#1.设置ARMClang工具链路径
set(ARMCLANG_TOOLCHAIN_PATH "C:/Keil_v5/ARM/ARMCLANG/bin")
#2.从ARMClang工具链路径中找到该工具
find_program(ARMCLANG_COMPILER armclang PATHS ${ARMCLANG_TOOLCHAIN_PATH})
#3.设置C、C++、ASM编译器
set(CMAKE_C_COMPILER "${ARMCLANG_COMPILER}")
set(CMAKE_CXX_COMPILER "${ARMCLANG_COMPILER}")
#4.
find_program(CMAKE_AR armar PATHS ${ARMCLANG_TOOLCHAIN_PATH})
find_program(CMAKE_LINKER armlink PATHS ${ARMCLANG_TOOLCHAIN_PATH})
find_program(ARMCLANG_FROMELF fromelf PATHS ${ARMCLANG_TOOLCHAIN_PATH})
#5.设置编译生成静态库目标
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
#6.判断 ARMClang工具 是否能正常打开，如果否则提示信息
if (NOT EXISTS "${ARMCLANG_FROMELF}")
    message(STATUS ${ARMCLANG_FROMELF})
    message(FATAL_ERROR "ARMCLANG toolchain is not found, please set ARMCLANG_TOOLCHAIN_PATH variable correctly")
endif ()
