#include "codegen_x86.h"
#include "ir.h"
#include "optimizer.h"
#include "parser.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

extern "C" void bf_put(unsigned char);

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " program.bf\n";
        return 1;
    }
    // 读取源文件
    std::ifstream ifs(argv[1]);
    if (!ifs)
    {
        perror("open");
        return 1;
    }
    std::string src((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    // Debug
    std::cout << "Source size: " << src.size() << "\n";
    std::cout << "Source preview: " << src.substr(0, std::min<size_t>(80, src.size())) << "\n";

    // 前端：解析
    Parser parser(src);
    auto ast = parser.parseProgram();

    // 中端：生成 IR
    IrProgram ir = generateIR(*ast);

    // 优化 IR
    optimizeIr(ir);

    // Debug ir
    std::cout << "IR instructions:\n";
    for (size_t i = 0; i < ir.insts.size(); ++i)
    {
        auto& ins = ir.insts[i];
        std::cout << i << ": op=" << static_cast<int>(ins.op) << " arg=" << ins.arg << " label=" << ins.label_id
                  << "\n";
    }

    // 后端：JIT 编译并获得函数指针
    auto fn = jitCompileX86(ir);

    // Debug JIT
    std::cout << "JIT function pointer: " << reinterpret_cast<void*>(fn) << "\n";
    std::cerr << "bf_put address: " << reinterpret_cast<void*>(bf_put) << "\n";
    // tape（内存带）
    constexpr size_t TAPE_SIZE = 30000;
    std::vector<unsigned char> tape(TAPE_SIZE, 0);

    std::cout << "\nRun JIT-compiled code..." << std::endl;
    std::cout << "Original Code:" << src << std::endl;
    std::cout << "outputs:" << std::endl;
    // 执行
    fn(tape.data());

    return 0;
}
