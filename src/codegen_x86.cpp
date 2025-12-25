// src/codegen_x86.cpp
// x86-64 JIT backend for Brainfuck compiler
// - Windows: uses RCX/ECX for first arg
// - POSIX: uses RDI/EDI for first arg

#include "codegen_x86.h"
#include "ir.h"
#include "runtime.h"

#include <iostream>
#include <fstream>

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <unistd.h>
#endif

struct LabelInfo
{
    int id;
    size_t offset;
};
struct PendingJump
{
    size_t rel32_offset;
    int target_label;
};

static void write32(uint8_t* base, size_t offset, int32_t value)
{
    std::memcpy(base + offset, &value, 4);
}

class CodeBuf
{
  public:
    explicit CodeBuf(size_t cap) : cap_(cap)
    {
#ifdef _WIN32
        buf_ = static_cast<uint8_t*>(VirtualAlloc(nullptr, cap_, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
        if (!buf_)
        {
            std::cerr << "VirtualAlloc failed\n";
            std::exit(1);
        }
#else
        buf_ = static_cast<uint8_t*>(
            mmap(nullptr, cap_, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
        if (buf_ == MAP_FAILED)
        {
            perror("mmap");
            std::exit(1);
        }
#endif
    }
    ~CodeBuf()
    {
        // optional: free memory (omitted for brevity)
    }

    void emit8(uint8_t b)
    {
        ensure(1);
        buf_[size_++] = b;
    }
    void emit32(uint32_t v)
    {
        ensure(4);
        std::memcpy(buf_ + size_, &v, 4);
        size_ += 4;
    }
    void emit64(uint64_t v)
    {
        ensure(8);
        std::memcpy(buf_ + size_, &v, 8);
        size_ += 8;
    }

    uint8_t* data()
    {
        return buf_;
    }
    size_t size() const
    {
        return size_;
    }

  private:
    void ensure(size_t n)
    {
        if (size_ + n > cap_)
        {
            std::cerr << "code buffer overflow\n";
            std::exit(1);
        }
    }

    uint8_t* buf_ = nullptr;
    size_t cap_ = 0;
    size_t size_ = 0;
};

extern "C" typedef void (*BfJitFunc)(unsigned char*);

BfJitFunc jitCompileX86(const IrProgram& prog)
{
    CodeBuf cb(64 * 1024);
    std::vector<LabelInfo> labels;
    std::vector<PendingJump> jumps;

    // Prologue: push rbx; mov rbx, <arg-reg>
    cb.emit8(0x53); // push rbx
#ifdef _WIN32
    // mov rbx, rcx  -> 48 89 CB
    cb.emit8(0x48);
    cb.emit8(0x89);
    cb.emit8(0xCB);
#else
    // mov rbx, rdi  -> 48 89 FB
    cb.emit8(0x48);
    cb.emit8(0x89);
    cb.emit8(0xFB);
#endif

    auto emitCall = [&](void* fn)
    {
        size_t pos = cb.size();
        // Reserve space for either form. We'll emit the chosen form below.
        // Try rel32 first:
        uint8_t* next_rel32 = cb.data() + pos + 5; // if we emit E8 + 4
        uint8_t* target = reinterpret_cast<uint8_t*>(fn);

        // compute 64-bit diff
        intptr_t diff = reinterpret_cast<intptr_t>(target) - reinterpret_cast<intptr_t>(next_rel32);

        if (diff >= INT32_MIN && diff <= INT32_MAX)
        {
            // emit call rel32: E8 <rel32>
            cb.emit8(0xE8);
            int32_t rel = static_cast<int32_t>(diff);
            cb.emit32(static_cast<uint32_t>(rel));
            uint8_t* computed = next_rel32 + rel;
            std::cerr << "emitCall (rel32): pos=" << pos << " next=" << (void*)next_rel32 << " target=" << (void*)target
                      << " rel=" << rel << " computed=" << (void*)computed << "\n";
        }
        else
        {
            // fallback: mov rax, imm64; call rax
            // mov rax, imm64 -> 48 B8 <imm64>
            cb.emit8(0x48);
            cb.emit8(0xB8);
            cb.emit64(reinterpret_cast<uint64_t>(target));
            // call rax -> FF D0
            cb.emit8(0xFF);
            cb.emit8(0xD0);

            std::cerr << "emitCall (indirect): pos=" << pos << " next_rel32=" << (void*)next_rel32
                      << " target=" << (void*)target << " used indirect mov rax; call rax\n";
        }
    };

    auto recordLabel = [&](int id) { labels.push_back(LabelInfo{id, cb.size()}); };

    auto emitJcc = [&](bool isZero, int targetLabel)
    {
        // cmpb [rbx], 0
        cb.emit8(0x80);
        cb.emit8(0x3B);
        cb.emit8(0x00);
        // jz/jnz rel32: 0F 84 / 0F 85
        cb.emit8(0x0F);
        cb.emit8(isZero ? 0x84 : 0x85);
        size_t relpos = cb.size();
        cb.emit32(0); // placeholder
        jumps.push_back(PendingJump{relpos, targetLabel});
    };

    // Emit instructions
    for (const auto& inst : prog.insts)
    {
        switch (inst.op)
        {
        case IrOp::AddPtr:
            if (inst.arg == 1)
            {
                // inc rbx -> 48 FF C3
                cb.emit8(0x48);
                cb.emit8(0xFF);
                cb.emit8(0xC3);
            }
            else if (inst.arg == -1)
            {
                // dec rbx -> 48 FF CB
                cb.emit8(0x48);
                cb.emit8(0xFF);
                cb.emit8(0xCB);
            }
            else
            {
                // add rbx, imm32 -> 48 81 C3 imm32
                cb.emit8(0x48);
                cb.emit8(0x81);
                cb.emit8(0xC3);
                cb.emit32(static_cast<uint32_t>(inst.arg));
            }
            break;

        case IrOp::AddVal:
            if (inst.arg == 1)
            {
                // incb [rbx] -> FE 03
                cb.emit8(0xFE);
                cb.emit8(0x03);
            }
            else if (inst.arg == -1)
            {
                // decb [rbx] -> FE 0B
                cb.emit8(0xFE);
                cb.emit8(0x0B);
            }
            else
            {
                // add byte ptr [rbx], imm8 -> 80 03 imm8
                cb.emit8(0x80);
                cb.emit8(0x03);
                cb.emit8(static_cast<uint8_t>(inst.arg));
            }
            break;

        case IrOp::Output:
#ifdef _WIN32
            // movzx ecx, byte ptr [rbx] -> 0F B6 0B
            cb.emit8(0x0F);
            cb.emit8(0xB6);
            cb.emit8(0x0B);
#else
            // movzx edi, byte ptr [rbx] -> 0F B6 3B
            cb.emit8(0x0F);
            cb.emit8(0xB6);
            cb.emit8(0x3B);
#endif
            emitCall(reinterpret_cast<void*>(bf_put));
            break;

        case IrOp::Input:
            emitCall(reinterpret_cast<void*>(bf_get));
            // mov [rbx], al -> 88 03
            cb.emit8(0x88);
            cb.emit8(0x03);
            break;

        case IrOp::Label:
            recordLabel(inst.label_id);
            break;

        case IrOp::Jz:
            emitJcc(true, inst.arg);
            break;

        case IrOp::Jnz:
            emitJcc(false, inst.arg);
            break;

        default:
            // unknown op: ignore
            break;
        }
    }

    // Epilogue: pop rbx; ret
    cb.emit8(0x5B); // pop rbx
    cb.emit8(0xC3); // ret

    // Resolve jumps
    auto findLabel = [&](int id) -> size_t
    {
        for (const auto& L : labels)
            if (L.id == id)
                return L.offset;
        std::cerr << "Label not found: " << id << "\n";
        std::exit(1);
    };

    for (const auto& j : jumps)
    {
        size_t target = findLabel(j.target_label);
        uint8_t* base = cb.data();
        uint8_t* rel32_addr = base + j.rel32_offset;
        uint8_t* next_instr = rel32_addr + 4;
        int32_t rel = static_cast<int32_t>((base + target) - next_instr);
        std::memcpy(rel32_addr, &rel, 4);
    }

    // Dump jit.bin for inspection
    {
        std::ofstream out("jit.bin", std::ios::binary);
        out.write(reinterpret_cast<char*>(cb.data()), cb.size());
    }

    // Print diagnostics
    std::cerr << "Generated code size: " << cb.size() << "\n";
    std::cerr << "Labels:\n";
    for (const auto& L : labels)
        std::cerr << "  id=" << L.id << " offset=" << L.offset << "\n";
    std::cerr << "Pending jumps: " << jumps.size() << "\n";

    // Print first bytes hex (quick check)
    std::cerr << "code hex: ";
    for (size_t i = 0; i < std::min<size_t>(cb.size(), 64); ++i)
    {
        fprintf(stderr, "%02X ", cb.data()[i]);
    }
    std::cerr << "\n";

    return reinterpret_cast<BfJitFunc>(cb.data());
}
