cmake_minimum_required(VERSION 3.10)
project(c906-fw C ASM)
add_compile_definitions(VIRTIO_CACHED_VRINGS)

# ============================================================================
# IMPORTANT: Please modify the toolchain path below according to your actual environment
# ============================================================================
set(TOOLCHAIN_PATH "/root/tina5.0_v1.0/rtos/lichee/rtos/tools/riscv64-elf-x86_64-20201104")
set(CMAKE_C_COMPILER "${TOOLCHAIN_PATH}/bin/riscv64-unknown-elf-gcc")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PATH}/bin/riscv64-unknown-elf-gcc")
set(CMAKE_OBJCOPY "${TOOLCHAIN_PATH}/bin/riscv64-unknown-elf-objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN_PATH}/bin/riscv64-unknown-elf-objdump")
set(CMAKE_SIZE "${TOOLCHAIN_PATH}/bin/riscv64-unknown-elf-size")

# Library file paths - modify according to actual situation!
set(LIBGCC_DIR "${TOOLCHAIN_PATH}/lib/gcc/riscv64-unknown-elf/8.4.0/rv64imafdcxtheadc/lp64d")
set(LIBC_DIR "${TOOLCHAIN_PATH}/riscv64-unknown-elf/lib/rv64imafdc/lp64d")

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -Og -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=rv64gcxthead")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcmodel=medany")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mabi=lp64d")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-common")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffreestanding")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -nostdlib")

# ASM compiler flags
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -D__ASSEMBLY__")

# Include paths
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/includes/arch/riscv
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/includes
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/GCC/RISC-V
    ${CMAKE_CURRENT_SOURCE_DIR}/openamp/include
)

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${CMAKE_CURRENT_SOURCE_DIR}/c906.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -nostdlib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -march=rv64gcxthead")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcmodel=medany")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mabi=lp64d")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map")

# Collect source files
file(GLOB_RECURSE CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/*.c
)

file(GLOB_RECURSE HAL_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/sources/*.c
)

file(GLOB_RECURSE ARCH_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/arch/riscv/*.c
)

file(GLOB_RECURSE OPENAMP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/openamp/libmetal/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/openamp/open-amp/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/openamp/platform/*.c
)

set_source_files_properties(${OPENAMP_SOURCES} PROPERTIES
    COMPILE_FLAGS "-Wno-error -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers")

# FreeRTOS core source files
set(FREERTOS_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/timers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/stream_buffer.c
)

# FreeRTOS RISC-V port source files
set(FREERTOS_PORT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/GCC/RISC-V/port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/GCC/RISC-V/port_misc.c
)

# FreeRTOS memory management source files
set(FREERTOS_MEM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/MemMang/heap_4.c
)

# Assembly source files
set(ASM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/head_s.S
    ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/Source/portable/GCC/RISC-V/portASM.S
)

# Merge all C source files
set(ALL_C_SOURCES
    ${CORE_SOURCES}
    ${HAL_SOURCES}
    ${ARCH_SOURCES}
    ${OPENAMP_SOURCES}
    ${FREERTOS_CORE_SOURCES}
    ${FREERTOS_PORT_SOURCES}
    ${FREERTOS_MEM_SOURCES}
)

# Create executable
add_executable(${PROJECT_NAME}
    ${ALL_C_SOURCES}
    ${ASM_SOURCES}
)

# Link libraries and search paths (must be after target to ensure correct resolution of symbols like printf/memcpy)
target_link_directories(${PROJECT_NAME} PRIVATE
    ${LIBGCC_DIR}
    ${LIBC_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    -Wl,--start-group
    c
    gcc
    -Wl,--end-group
)

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    SUFFIX ".elf"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Add custom command to generate binary file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMENT "Generating binary file ${PROJECT_NAME}.bin"
)

# Add custom target for disassembly
add_custom_target(disasm
    COMMAND ${CMAKE_OBJDUMP} -d -S $<TARGET_FILE:${PROJECT_NAME}> > ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.dis
    COMMENT "Generating disassembly ${PROJECT_NAME}.dis"
    DEPENDS ${PROJECT_NAME}
)

# Add custom target for size
add_custom_target(size
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Size information for ${PROJECT_NAME}.elf"
    DEPENDS ${PROJECT_NAME}
)
