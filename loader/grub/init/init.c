/**
 * @file gdt.c
 * @author theflysong (song_of_the_fly@163.com)
 * @brief GDT
 * @version alpha-1.0.0
 * @date 2023-3-21
 *
 * @copyright Copyright (c) 2022 TayhuangOS Development Team
 * SPDX-License-Identifier: LGPL-2.1-only
 *
 */

#include <init/init.h>
#include <basec/logger.h>
#include <stddef.h>
#include <tay/ports.h>
#include <tay/io.h>

#define EMPTY_IDX (0)
#define CODE_IDX (2)
#define DATA_IDX (3)
#define KERCODE_IDX (8)
#define KERDATA_IDX (9)

/**
 * @brief GDT
 *
 */
Descriptor GDT[64];

/**
 * @brief GDTR
 *
 */
DPTR GDTR;

/**
 * @brief 初始化空描述符
 *
 */
static void init_empty_desc(void) {
    RawDesc empty = {};
    GDT[0] = build_desc(empty);
}

/**
 * @brief 初始化代码段描述符
 *
 */
static void init_code_desc(void) {
    RawDesc code = {};

    // 平坦模型
    code.base = 0;
    code.limit = 0xFFFFF;

    // 内核级可读可执行代码段
    code.DPL = DPL0;
    code.type = DTYPE_XRCODE;

    // 代码/数据段
    code.S    = true;

    // 存在
    code.P   = true;
    code.AVL       = false;

    // 32位段
    code.L        = false;
    code.DB        = true;

    // 4KB粒度
    code.G = true;

    GDT[CODE_IDX] = build_desc(code);
}

static void init_data_desc(void) {
    RawDesc data = {};

    // 平坦模型
    data.base = 0;
    data.limit = 0xFFFFF;

    // 内核级可读写数据段
    data.DPL = DPL0;
    data.type = DTYPE_RWDATA;

    // 代码/数据段
    data.S    = true;

    // 存在
    data.P   = true;
    data.AVL       = false;

    // 32位段
    data.L        = false;
    data.DB        = true;

    // 4KB粒度
    data.G = true;

    GDT[DATA_IDX] = build_desc(data);
}

static void init_kcode_desc(void) {
    RawDesc kcode = {};

    // 平坦模型
    kcode.base = 0;
    kcode.limit = 0xFFFFF;

    // 内核级可读可执行代码段
    kcode.DPL = DPL0;
    kcode.type = DTYPE_XRCODE;

    // 代码/数据段
    kcode.S    = true;

    // 存在
    kcode.P   = true;
    kcode.AVL       = false;

    // 64位段
    kcode.L        = true;
    kcode.DB        = false;

    // 4KB粒度
    kcode.G = true;

    GDT[KERCODE_IDX] = build_desc(kcode);
}

static void init_kdata_desc(void) {
    RawDesc kdata = {};

    // 平坦模型
    kdata.base = 0;
    kdata.limit = 0xFFFFF;

    // 内核级可读写数据段
    kdata.DPL = DPL0;
    kdata.type = DTYPE_RWDATA;

    // 代码/数据段
    kdata.S    = true;

    // 存在
    kdata.P   = true;
    kdata.AVL       = false;

    // 64位段
    kdata.L        = true;
    kdata.DB        = false;

    // 4KB粒度
    kdata.G = true;

    GDT[KERDATA_IDX] = build_desc(kdata);
}

/**
 * @brief 初始化GDT
 *
 */
void init_gdt(void) {
    // 初始化描述符
    init_empty_desc();
    init_code_desc();
    init_data_desc();
    init_kcode_desc();
    init_kdata_desc();

    // 初始化GDTR
    GDTR.address = GDT;
    GDTR.size = sizeof(GDT) - 1;

    asm volatile ("lgdt %0" : : "m"(GDTR)); //加载GDT

    // 设置段
    stds(DATA_IDX << 3);
    stes(DATA_IDX << 3);
    stfs(DATA_IDX << 3);
    stgs(DATA_IDX << 3);
    stss(DATA_IDX << 3);
}

/**
 * @brief 启用中断
 *
 */
void sti(void) {
    asm volatile ("sti");
}

/**
 * @brief 禁止中断
 *
 */
void cli(void) {
    asm volatile ("cli");
}

//禁用/启用 IRQ
static void disable_irq(int irq) {
    if (irq < 8) {
        //主片
        byte i = inb(M_PIC_BASE + PIC_DATA);
        i |= (1 << irq);
        outb(M_PIC_BASE + PIC_DATA, i);
    }
    else {
        //从片
        byte i = inb(S_PIC_BASE + PIC_DATA);
        i |= (1 << (irq - 8));
        outb(S_PIC_BASE + PIC_DATA, i);
    }
}

static void enable_irq(int irq) {
    if (irq < 8) {
        //主片
        byte i = inb(M_PIC_BASE + PIC_DATA);
        i &= ~(1 << irq);
        outb(M_PIC_BASE + PIC_DATA, i);
    }
    else {
        //从片
        byte i = inb(S_PIC_BASE + PIC_DATA);
        i &= ~(1 << (irq - 8));
        outb(S_PIC_BASE + PIC_DATA, i);
    }
}

#define PIC_EOI (0x20)

//发送EOI
static void send_eoi(int irq) {
    if (irq > 8) {
        //从片EOI
        outb (S_PIC_BASE + PIC_CONTROL, PIC_EOI);
    }
    //主片EOI
    outb (M_PIC_BASE + PIC_CONTROL, PIC_EOI);
}

static int ticks = 0;

static bool clock_handler(int irq, IStack *stack) {
    ticks ++;
    log_info("Ticks=%d", ticks);

    return false;
}

/**
 * @brief IRQ处理器
 *
 */
IRQHandler irq_handlers[32] = {
    [0] = clock_handler
};

/**
 * @brief IRQ主处理程序
 *
 * @param irq irq号
 * @param stack 堆栈
 */
void irq_handler_primary(int irq, IStack *stack) {
    disable_irq(irq);

    //发送EOI
    send_eoi(irq);

    log_info("接收到IRQ=%02X", irq);

    if (irq_handlers[irq] != NULL) {
        if (! irq_handlers[irq](irq, stack)) {
            log_error("解决IRQ=%02X失败!", irq);
            return; //不再开启该中断
        }
    }
}

//---------------------------------------------

int errcode;

// 异常信息
static const char *exceptionMessage[] = {
    "[#DE] 除以0!",
    "[#DB] 单步调试",
    "[无] NMI中断!",
    "[#BP] 断点",
    "[#OF] 溢出!",
    "[#BR] 越界!",
    "[#UD] 无效的操作码(未定义的指令)!",
    "[#NM] 设备不可用(没有数学协处理器)!",
    "[#DF] 双重错误!",
    "[无] 协处理器段溢出!",
    "[#TS] 无效TSS!",
    "[#NP] 缺少段!",
    "[#SS] 缺少栈段!",
    "[#GP] 通用保护错误!",
    "[#PF] 缺页中断!",
    "[保留] 保留!",
    "[#MF] x87数学协处理器浮点运算错误!",
    "[#AC] 对齐检测!",
    "[#MC] 机器检测!",
    "[#XF] SIMD浮点运算错误!",
    "[#VE] 虚拟化异常!",
    "[#CP] 控制保护错误!",
    "[保留] 保留!",
    "[保留] 保留!",
    "[保留] 保留!",
    "[保留] 保留!",
    "[保留] 保留!",
    "[保留] 保留!",
    "[#HV] Hypervisor注入异常!",
    "[#VC] VMM通信异常!",
    "[#SX] 安全性错误!",
    "[保留] 保留!"
};

typedef bool(*ExceptionSolution)(void);

static const ExceptionSolution solutionList[] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/**
 * @brief 异常主处理程序
 *
 * @param errno 异常号
 * @param stack 堆栈
 */
void exception_handler_primary(int errno, IStack *stack) {
    log_error("在%04X:%08X处发生错误:", stack->cs, stack->eip);
    log_error("%s", exceptionMessage[errno]);

    if (errcode != 0xFFFFFFFF) {
        log_error("Error Code = %08X", errcode);
    }

    log_error("现场已保存:");

    log_error("eax: %08X ; ebx: %08X ; ecx: %08X ; edx: %08X", stack->eax, stack->ebx, stack->ecx, stack->edx);
    log_error("esi: %08X ; edi: %08X ; esp: %08X ; ebp: %08X", stack->esi, stack->edi, stack->esp, stack->ebp);

    log_error(" ds: %04X     ;  es: %04X     ;  fs: %04X     ;  gs: %04X    ", stack->ds, stack->es, stack->fs, stack->gs);

    log_error("cr3: %08X ; eflags: %08X", stack->cr3, stack->eflags);

    if (solutionList[errno] != NULL) {
        if (! solutionList[errno]()) {
            log_fatal("解决异常%02X失败!", errno);
            while (true);
        }
    }
    else {
        log_fatal("无法解决异常%02X!", errno);
        while (true);
    }
}

/** IDT */
GateDescriptor IDT[256];
/**
 * @brief IDTR
 *
 */
DPTR IDTR;

#define IRQ_START (32)

/**
 * @brief 初始化PIC
 *
 */
void init_pic(void) {
    outb (M_PIC_BASE + PIC_CONTROL, 0x11);
    outb (S_PIC_BASE + PIC_CONTROL, 0x11); //ICW1

    // IRQ 0
    outb (M_PIC_BASE + PIC_DATA, IRQ_START);
    // IRQ 8
    outb (S_PIC_BASE + PIC_DATA, IRQ_START + 8); //ICW2

    outb (M_PIC_BASE + PIC_DATA, 0x4);
    outb (S_PIC_BASE + PIC_DATA, 0x2); //ICW3

    outb (M_PIC_BASE + PIC_DATA, 0x1);
    outb (S_PIC_BASE + PIC_DATA, 0x1); //ICW4

    // disable all
    outb (M_PIC_BASE + PIC_DATA, 0xFF); //OCW1
    outb (S_PIC_BASE + PIC_DATA, 0xFF);

    enable_irq(0);
}

/**
 * @brief 初始化IDT
 *
 */
void init_idt(void) {
    IDT[0x00] = build_gate(GTYPE_386_INT_GATE, divide_by_zero_fault_handler, 0, rdcs());
    IDT[0x01] = build_gate(GTYPE_386_INT_GATE, single_step_trap_handler, 0, rdcs());
    IDT[0x02] = build_gate(GTYPE_386_INT_GATE, nmi_handler, 0, rdcs());
    IDT[0x03] = build_gate(GTYPE_386_INT_GATE, breakpoint_trap_handler, 0, rdcs());
    IDT[0x04] = build_gate(GTYPE_386_INT_GATE, overflow_trap_handler, 0, rdcs());
    IDT[0x05] = build_gate(GTYPE_386_INT_GATE, bound_range_exceeded_fault_handler, 0, rdcs());
    IDT[0x06] = build_gate(GTYPE_386_INT_GATE, invalid_opcode_fault_handler, 0, rdcs());
    IDT[0x07] = build_gate(GTYPE_386_INT_GATE, device_not_available_fault_handler, 0, rdcs());
    IDT[0x08] = build_gate(GTYPE_386_INT_GATE, double_fault_handler, 0, rdcs());
    IDT[0x09] = build_gate(GTYPE_386_INT_GATE, coprocessor_segment_overrun_fault_handler, 0, rdcs());
    IDT[0x0A] = build_gate(GTYPE_386_INT_GATE, invalid_tss_fault, 0, rdcs());
    IDT[0x0B] = build_gate(GTYPE_386_INT_GATE, segment_not_present_fault_handler, 0, rdcs());
    IDT[0x0C] = build_gate(GTYPE_386_INT_GATE, stack_segment_fault_handler, 0, rdcs());
    IDT[0x0D] = build_gate(GTYPE_386_INT_GATE, general_protection_fault_handler, 0, rdcs());
    IDT[0x0E] = build_gate(GTYPE_386_INT_GATE, page_fault_handler, 0, rdcs());
    IDT[0x0F] = build_gate(GTYPE_386_INT_GATE, reserved_handler_1, 0, rdcs());
    IDT[0x10] = build_gate(GTYPE_386_INT_GATE, x87_floating_point_fault_handler, 0, rdcs());
    IDT[0x11] = build_gate(GTYPE_386_INT_GATE, alignment_check_handler, 0, rdcs());
    IDT[0x12] = build_gate(GTYPE_386_INT_GATE, machine_check_handler, 0, rdcs());
    IDT[0x13] = build_gate(GTYPE_386_INT_GATE, simd_floating_point_fault_handler, 0, rdcs());
    IDT[0x14] = build_gate(GTYPE_386_INT_GATE, virtualization_fault_handler, 0, rdcs());
    IDT[0x15] = build_gate(GTYPE_386_INT_GATE, control_protection_fault_handler, 0, rdcs());
    IDT[0x16] = build_gate(GTYPE_386_INT_GATE, reserved_handler_2, 0, rdcs());
    IDT[0x17] = build_gate(GTYPE_386_INT_GATE, reserved_handler_3, 0, rdcs());
    IDT[0x18] = build_gate(GTYPE_386_INT_GATE, reserved_handler_4, 0, rdcs());
    IDT[0x19] = build_gate(GTYPE_386_INT_GATE, reserved_handler_5, 0, rdcs());
    IDT[0x1A] = build_gate(GTYPE_386_INT_GATE, reserved_handler_6, 0, rdcs());
    IDT[0x1B] = build_gate(GTYPE_386_INT_GATE, reserved_handler_7, 0, rdcs());
    IDT[0x1C] = build_gate(GTYPE_386_INT_GATE, hypervisor_injection_exception, 0, rdcs());
    IDT[0x1D] = build_gate(GTYPE_386_INT_GATE, vmm_communication_fault_handler, 0, rdcs());
    IDT[0x1E] = build_gate(GTYPE_386_INT_GATE, security_fault_handler, 0, rdcs());
    IDT[0x1F] = build_gate(GTYPE_386_INT_GATE, reserved_handler_8, 0, rdcs());

    IDT[IRQ_START + 0] = build_gate(GTYPE_386_INT_GATE, irq0_handler, 0, rdcs());
    IDT[IRQ_START + 1] = build_gate(GTYPE_386_INT_GATE, irq1_handler, 0, rdcs());
    IDT[IRQ_START + 2] = build_gate(GTYPE_386_INT_GATE, irq2_handler, 0, rdcs());
    IDT[IRQ_START + 3] = build_gate(GTYPE_386_INT_GATE, irq3_handler, 0, rdcs());
    IDT[IRQ_START + 4] = build_gate(GTYPE_386_INT_GATE, irq4_handler, 0, rdcs());
    IDT[IRQ_START + 5] = build_gate(GTYPE_386_INT_GATE, irq5_handler, 0, rdcs());
    IDT[IRQ_START + 6] = build_gate(GTYPE_386_INT_GATE, irq6_handler, 0, rdcs());
    IDT[IRQ_START + 7] = build_gate(GTYPE_386_INT_GATE, irq7_handler, 0, rdcs());
    IDT[IRQ_START + 8] = build_gate(GTYPE_386_INT_GATE, irq8_handler, 0, rdcs());
    IDT[IRQ_START + 9] = build_gate(GTYPE_386_INT_GATE, irq9_handler, 0, rdcs());
    IDT[IRQ_START + 10] = build_gate(GTYPE_386_INT_GATE, irq10_handler, 0, rdcs());
    IDT[IRQ_START + 11] = build_gate(GTYPE_386_INT_GATE, irq11_handler, 0, rdcs());
    IDT[IRQ_START + 12] = build_gate(GTYPE_386_INT_GATE, irq12_handler, 0, rdcs());
    IDT[IRQ_START + 13] = build_gate(GTYPE_386_INT_GATE, irq13_handler, 0, rdcs());
    IDT[IRQ_START + 14] = build_gate(GTYPE_386_INT_GATE, irq14_handler, 0, rdcs());
    IDT[IRQ_START + 15] = build_gate(GTYPE_386_INT_GATE, irq15_handler, 0, rdcs());

    IDTR.address = IDT;
    IDTR.size = sizeof(IDT);

    asm volatile ("lidt %0" : : "m"(IDTR));
}

