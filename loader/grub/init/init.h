/**
 * @file init.h
 * @author theflysong (song_of_the_fly@163.com)
 * @brief Initialization
 * @version alpha-1.0.0
 * @date 2025-11-14
 * 
 * @copyright Copyright (c) 2022 TayhuangOS Development Team
 * SPDX-License-Identifier: LGPL-2.1-only
 * 
 */

#pragma once

#include <tay/desc.h>

/** GDT */
extern Descriptor GDT[64];

/** GDTR */
extern DPTR GDTR;

/** IDT */
extern GateDescriptor IDT[256];

/** IDTR */
extern DPTR IDTR;

/**
 * @brief 初始化GDT
 *
 */
void init_gdt(void);

/**
 * @brief 初始化PIC
 *
 */
void init_pic(void);

/**
 * @brief 初始化IDT
 *
 */
void init_idt(void);

/**
 * @brief 堆栈结构
 *
 */
typedef struct {
    b32 edi,
        esi,
        edx,
        ecx,
        ebx,
        eax,
        gs,
        fs,
        es,
        ds,
        cr3,
        ebp,
        handlerEsp,
        eip,
        cs,
        eflags,
        esp,
        ss;
} IStack;

/** 中断处理器 */
typedef bool(*IRQHandler)(int irq, IStack *stack);

extern IRQHandler irq_handlers[32];

/**
 * @brief IRQ主处理程序
 *
 * @param irq irq号
 * @param stack 堆栈
 */
void irq_handler_primary(int irq, IStack *stack);

/**
 * @brief 异常主处理程序
 *
 * @param errno 异常号
 * @param stack 堆栈
 */
void exception_handler_primary(int errno, IStack *stack);

/**
 * @brief 启用中断
 *
 */
void sti(void);

/**
 * @brief 禁止中断
 *
 */
void cli(void);

//------------------------------------------

void divide_by_zero_fault_handler(void); //除以0
void single_step_trap_handler(void); //单步调试
void nmi_handler(void); //NMI
void breakpoint_trap_handler(void); //断点
void overflow_trap_handler(void); //溢出
void bound_range_exceeded_fault_handler(void); //出界
void invalid_opcode_fault_handler(void); //非法指令码
void device_not_available_fault_handler(void); //设备不可用
void double_fault_handler(void); //双重错误
void coprocessor_segment_overrun_fault_handler(void); //协处理器错误
void invalid_tss_fault(void); //无效TSS
void segment_not_present_fault_handler(void); //段不存在
void stack_segment_fault_handler(void); //栈段错误
void general_protection_fault_handler(void); //通用保护错误
void page_fault_handler(void); //缺页中断
void reserved_handler_1(void); //
void x87_floating_point_fault_handler(void); //x87数学协处理器浮点运算错误
void alignment_check_handler(void); //对齐检测
void machine_check_handler(void); //机器检测
void simd_floating_point_fault_handler(void); //SIMD浮点运算错误
void virtualization_fault_handler(void); //虚拟化异常
void control_protection_fault_handler(void); //控制保护错误
void reserved_handler_2(void); //
void reserved_handler_3(void); //
void reserved_handler_4(void); //
void reserved_handler_5(void); //
void reserved_handler_6(void); //
void reserved_handler_7(void); //
void hypervisor_injection_exception(void); //VMM注入错误
void vmm_communication_fault_handler(void); //VMM交流错误
void security_fault_handler(void); //安全性错误
void reserved_handler_8(void); //

//------------------------------------------

void irq0_handler(void);
void irq1_handler(void);
void irq2_handler(void);
void irq3_handler(void);
void irq4_handler(void);
void irq5_handler(void);
void irq6_handler(void);
void irq7_handler(void);
void irq8_handler(void);
void irq9_handler(void);
void irq10_handler(void);
void irq11_handler(void);
void irq12_handler(void);
void irq13_handler(void);
void irq14_handler(void);
void irq15_handler(void);