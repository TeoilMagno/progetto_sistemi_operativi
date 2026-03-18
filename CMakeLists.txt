cmake_minimum_required(VERSION 3.25)
project(MultiPandOS LANGUAGES C ASM)

find_program(RISCV_GCC_UNKNOWN riscv64-unknown-elf-gcc)
find_program(RISCV_GCC riscv64-elf-gcc)
if(RISCV_GCC_UNKNOWN)
    set(XT_PRG_PREFIX riscv64-unknown-elf-)
elseif(RISCV_GCC)
    set(XT_PRG_PREFIX riscv64-elf-)
endif()
set(URISCV_SRC /usr/local/share/uriscv)
if(EXISTS "/usr/include/uriscv")
    set(URISCV_INC /usr/include)
elseif(EXISTS "/usr/local/include/uriscv")
    set(URISCV_INC /usr/local/include)
endif()

set(CMAKE_C_COMPILER ${XT_PRG_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${XT_PRG_PREFIX}gcc)
set(CMAKE_C_LINKER ${XT_PRG_PREFIX}ld)

add_link_options(-nostdlib -T ${URISCV_SRC}/uriscvcore.ldscript -march=rv32imfd)
add_compile_options(-ffreestanding -static -nostartfiles -nostdlib -I${URISCV_INC} -ggdb -Wall -O0 -std=gnu99 -march=rv32imafd -mabi=ilp32d)

add_executable(MultiPandOS
    ./klog.c phase1/pcb.c phase1/asl.c
    ./phase2/initial.c ./phase2/scheduler.c ./phase2/exceptions.c ./phase2/interrupts.c
    #phase1/p1test.c
    ./phase2/p2test.c
    ${URISCV_SRC}/crtso.S ${URISCV_SRC}/liburiscv.S)

add_custom_target(
	MultiPandOSuRISCV ALL
	COMMAND uriscv-elf2uriscv -k ${PROJECT_BINARY_DIR}/MultiPandOS
	BYPRODUCTS MultiPandOS.core.uriscv MultiPandOS.stab.uriscv
	DEPENDS MultiPandOS
)

