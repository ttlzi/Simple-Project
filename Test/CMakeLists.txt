#1.设置CMake系统名称、系统版本和最低支持版本
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.25)

#2.设置工程名字、设置芯片类型、设置C和C++支持版本（基本无用）、设置工程
set(PROJECT_NAME Test)
set(CMAKE_SYSTEM_PROCESSOR Cortex-M3)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
project(${PROJECT_NAME} C CXX ASM)

#3.设置芯片地址信息
set(SECTIONS_SCRIPT_PATH ${CMAKE_CURRENT_LIST_DIR}/MDK-ARM/${PROJECT_NAME}/${PROJECT_NAME}.sct)

#4.设置编译C、C++、ASM编译选项
set(CMAKE_C_FLAGS "-mcpu=${CMAKE_SYSTEM_PROCESSOR}  -std=c11 -xc -c")
set(CMAKE_CXX_FLAGS "-mcpu=${CMAKE_SYSTEM_PROCESSOR} -std=c++11 -xc++ -c")
set(CMAKE_ASM_FLAGS "-mcpu=${CMAKE_SYSTEM_PROCESSOR} -masm=auto -c -gdwarf-3")

#5.设置连接选项
add_link_options(
        --library_type=microlib
        --strict
        --scatter=${SECTIONS_SCRIPT_PATH}
        --summary_stderr
        --info summarysizes,sizes,totals,unused,veneers
        --map
        --load_addr_map_info
        --xref
        --callgraph
        --symbols
)

#6.设置编译选项
add_compile_options(
        --target=arm-arm-none-eabi
        -fno-rtti
        -fsigned-char
        -fshort-enums
        -fshort-wchar
        -ffunction-sections
        -Wno-packed
        -Wno-missing-variable-declarations
        -Wno-missing-prototypes
        -Wno-missing-noreturn
        -Wno-sign-conversion
        -Wno-nonportable-include-path
        -Wno-reserved-id-macro
        -Wno-unused-macros
        -Wno-documentation-unknown-command
        -Wno-documentation
        -Wno-license-management
        -Wno-parentheses-equality
)

#7.设置宏定义->相当于keil魔术棒中C\C++中的Define
add_definitions(
        -D__MICROLIB
        -D__UVISION_VERSION="535"
        -D_RTE_
        -DSTM32F10X_MD
        -D_RTE_
        -DUSE_HAL_DRIVER
        -DSTM32F103xB
        -MD
)

#8.设置头文件路径
include_directories(
        Drivers/CMSIS/Device/ST/STM32F1xx/Include
        Drivers/CMSIS/Include
        Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
        Drivers/STM32F1xx_HAL_Driver/Inc
        Inc

        Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
        Middlewares/Third_Party/FreeRTOS/Source/include
        Middlewares/Third_Party/FreeRTOS/Source/portable/RVDS/ARM_CM3
)

#9.设置源文件路径
file(GLOB_RECURSE SOURCES
        Drivers/STM32F1xx_HAL_Driver/Src/*.c
        MDK-ARM/*.s
        Src/*.c

        Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/*.c
        Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/*.c
        Middlewares/Third_Party/FreeRTOS/Source/portable/RVDS/ARM_CM3/*.c
        Middlewares/Third_Party/FreeRTOS/Source/*.c

        User/App/*.c
)

#10.将由 CubeMX 生成的 GCC 编译环境中的会干扰ARMCC环境的文件，放在 EXCLUDE_SRCS 自定义列表中
file(GLOB_RECURSE EXCLUDE_SRCS

)

#11.对于混合兼容的环境，需要屏蔽各种编译环境引起的文件“干扰”,通过 list(REMOVE_ITEM) 命令移除不同编译环境下的干扰文件
list(REMOVE_ITEM SOURCES ${EXCLUDE_SRCS})

#12.给这些源文件创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

#13.判断MCU是否为Cortex-M4,如果是，则选择使用硬件浮点数或者软件浮点数
if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "Cortex-M4")
    OPTION(USE_FLOAT_FLAG "Use Float Hard Or Soft(On is Hard,Off is Soft)" OFF)
    if (USE_FLOAT_FLAG)
        message("Now MCU Use Hard Float!")
        add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
        add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
        add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
    else ()
        message("Now MCU Use Soft Float!")
        add_compile_options(-mfloat-abi=soft)
    endif ()
endif ()

#14.判断是否需要使用本地Lib库
OPTION(USE_LOCAL_LIBRARY_FLAG "Use Local Library" OFF)
if (USE_LOCAL_LIBRARY_FLAG)
    link_directories(Drivers/lks32mc03x_periph_driver/Source)
    link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/Drivers/lks32mc03x_periph_driver/Source/lks32mc03x_nvr.lib")
endif ()

#15.根据编译类型选择编译参数（此处应该无用，因为这个已经在CMakePresets.json文件中使用）
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g -DDEBUG)
endif ()

#16.设置HEX文件、BIN文件、ELF文件和MAP文件
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
set(ELF_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf)
set(MAP_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf.map)

#17.生成最终的HEX文件
#使用 armclang 自带的 fromelf 工具，实现 elf 转 hex
#set(ARMCLANG_FROMELF fromelf.exe)
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        # 相当于fromelf.exe" --i32combined --output="xxx/demo.hex" "xxx/demo.elf"
#        COMMAND ${ARMCLANG_FROMELF} --i32combined --output="${HEX_FILE}" "${ELF_FILE}"
#        COMMENT "Building ${HEX_FILE}"
#)

#用这个比较稳定一些
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${ARMCLANG_FROMELF} --output ${HEX_FILE} --i32 $<TARGET_FILE:${PROJECT_NAME}>
        COMMAND ${ARMCLANG_FROMELF} --output ${BIN_FILE} --bin $<TARGET_FILE:${PROJECT_NAME}>
        COMMENT "Building ${HEX_FILE}\r\nBuilding ${BIN_FILE}")

## 使用了 gcc 的 arm-none-eabi-objcopy 工具实现 elf 转 hex 和 bin， 跟 armclang 不是一脉
#set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}> ${HEX_FILE}
#        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}> ${BIN_FILE}
#        COMMENT "Building ${HEX_FILE} Building ${BIN_FILE}")

