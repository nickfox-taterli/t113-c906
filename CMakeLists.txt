cmake_minimum_required(VERSION 3.10)
project(c906-fw C ASM)

# ============================================================================
# 重要提示：请根据您的实际环境修改以下编译器路径
# ============================================================================
set(TOOLCHAIN_PATH "/root/tina5.0_v1.0/rtos/lichee/rtos/tools/riscv64-elf-x86_64-20201104")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/bin/riscv64-unknown-elf-gcc")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PATH}/bin/riscv64-unknown-elf-gcc")

# 库文件路径 - 请根据实际情况修改！
set(LIBGCC_DIR "${TOOLCHAIN_PATH}/lib/gcc/riscv64-unknown-elf/8.4.0/rv64imafdcxtheadc/lp64d")
set(LIBC_DIR "${TOOLCHAIN_PATH}/riscv64-unknown-elf/lib/rv64imafdc/lp64d")

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 编译器标志
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -Og -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=rv64gcxthead")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcmodel=medany")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mabi=lp64d")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-common")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdlib")

# ASM编译器标志
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -D__ASSEMBLY__")

# 包含路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/includes/arch/riscv
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/GCC/RISC-V
)

# 链接器标志
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_CURRENT_SOURCE_DIR}/c906.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -march=rv64gcxthead")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcmodel=medany")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mabi=lp64d")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${LIBC_DIR} -L${LIBGCC_DIR}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nosys.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--start-group -lc -lgcc -Wl,--end-group")

# 收集源文件
file(GLOB_RECURSE CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/*.c
)

file(GLOB_RECURSE HAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/sources/*.c
)

file(GLOB_RECURSE ARCH_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/riscv/*.c
)

# FreeRTOS核心源文件
set(FREERTOS_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/timers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/stream_buffer.c
)

# FreeRTOS RISC-V移植源文件
set(FREERTOS_PORT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/GCC/RISC-V/port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/GCC/RISC-V/port_misc.c
)

# FreeRTOS内存管理源文件
set(FREERTOS_MEM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/MemMang/heap_4.c
)

# 汇编源文件
set(ASM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/head_s.S
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/GCC/RISC-V/portASM.S
)

# 合并所有C源文件
set(ALL_C_SOURCES
    ${CORE_SOURCES}
    ${HAL_SOURCES}
    ${ARCH_SOURCES}
    ${FREERTOS_CORE_SOURCES}
    ${FREERTOS_PORT_SOURCES}
    ${FREERTOS_MEM_SOURCES}
)

# 创建可执行文件
add_executable(${PROJECT_NAME}.elf
    ${ALL_C_SOURCES}
    ${ASM_SOURCES}
)

# 设置输出目录
set_target_properties(${PROJECT_NAME}.elf PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# 添加生成二进制文件的自定义命令
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Generating binary file ${PROJECT_NAME}.bin"
)

# 添加反汇编自定义目标
add_custom_target(disasm
    COMMAND ${CMAKE_OBJDUMP} -d -S ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf > ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dis
    COMMENT "Generating disassembly ${PROJECT_NAME}.dis"
    DEPENDS ${PROJECT_NAME}.elf
)

# 添加size自定义目标
add_custom_target(size
    COMMAND ${CMAKE_SIZE} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.elf
    COMMENT "Size information for ${PROJECT_NAME}.elf"
    DEPENDS ${PROJECT_NAME}.elf
)
