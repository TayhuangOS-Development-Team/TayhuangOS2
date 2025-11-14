.extern irq_handler_primary
.extern exception_handler_primary

.macro menter
    pushl %esp
    pushl %ebp

    movl %cr3, %ebp
    pushl %ebp

    movw %ds, %bp
    pushl %ebp
    movw %es, %bp
    pushl %ebp
    movw %fs, %bp
    pushl %ebp
    movw %gs, %bp
    pushl %ebp

    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
.endm

.macro mleave
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax

    popl %ebp
    movw %bp, %gs
    popl %ebp
    movw %bp, %fs
    popl %ebp
    movw %bp, %ds
    popl %ebp
    movw %bp, %ds

    popl %ebp
    movl %ebp, %cr3

    popl %ebp
    popl %esp
.endm

.extern errcode
.type errcode, @object

.macro NonErrcodeHandler vector
    movl $0xFFFFFFFF, (errcode)
    menter

    pushl %esp
    movl $\vector, %eax

    call exception_handler_primary

    addl $4, %esp

    mleave
    iret
.endm

.macro ErrcodeHandler vector
    pushl %eax
    movl 4(%esp), %eax
    movl %eax, (errcode)
    popl %eax
    add $4, %esp
    menter

    pushl %esp
    movl $\vector, %eax

    call exception_handler_primary

    addl $4, %esp

    mleave
    iret
.endm

.global divide_by_zero_fault_handler
.type   divide_by_zero_fault_handler, @function
.global single_step_trap_handler
.type   single_step_trap_handler, @function
.global nmi_handler
.type   nmi_handler, @function
.global breakpoint_trap_handler
.type   breakpoint_trap_handler, @function
.global overflow_trap_handler
.type   overflow_trap_handler, @function
.global bound_range_exceeded_fault_handler
.type   bound_range_exceeded_fault_handler, @function
.global invalid_opcode_fault_handler
.type   invalid_opcode_fault_handler, @function
.global device_not_available_fault_handler
.type   device_not_available_fault_handler, @function
.global double_fault_handler
.type   double_fault_handler, @function
.global coprocessor_segment_overrun_fault_handler
.type   coprocessor_segment_overrun_fault_handler, @function
.global invalid_tss_fault
.type   invalid_tss_fault, @function
.global segment_not_present_fault_handler
.type   segment_not_present_fault_handler, @function
.global stack_segment_fault_handler
.type   stack_segment_fault_handler, @function
.global general_protection_fault_handler
.type   general_protection_fault_handler, @function
.global page_fault_handler
.type   page_fault_handler, @function
.global reserved_handler_1
.type   reserved_handler_1, @function
.global x87_floating_point_fault_handler
.type   x87_floating_point_fault_handler, @function
.global alignment_check_handler
.type   alignment_check_handler, @function
.global machine_check_handler
.type   machine_check_handler, @function
.global simd_floating_point_fault_handler
.type   simd_floating_point_fault_handler, @function
.global virtualization_fault_handler
.type   virtualization_fault_handler, @function
.global control_protection_fault_handler
.type   control_protection_fault_handler, @function
.global reserved_handler_2
.type   reserved_handler_2, @function
.global reserved_handler_3
.type   reserved_handler_3, @function
.global reserved_handler_4
.type   reserved_handler_4, @function
.global reserved_handler_5
.type   reserved_handler_5, @function
.global reserved_handler_6
.type   reserved_handler_6, @function
.global reserved_handler_7
.type   reserved_handler_7, @function
.global hypervisor_injection_exception
.type   hypervisor_injection_exception, @function
.global vmm_communication_fault_handler
.type   vmm_communication_fault_handler, @function
.global security_fault_handler
.type   security_fault_handler, @function
.global reserved_handler_8
.type   reserved_handler_8, @function

divide_by_zero_fault_handler:
    NonErrcodeHandler 0
single_step_trap_handler:
    NonErrcodeHandler 1
nmi_handler:
    NonErrcodeHandler 2
breakpoint_trap_handler:
    NonErrcodeHandler 3
overflow_trap_handler:
    NonErrcodeHandler 4
bound_range_exceeded_fault_handler:
    NonErrcodeHandler 5
invalid_opcode_fault_handler:
    NonErrcodeHandler 6
device_not_available_fault_handler:
    NonErrcodeHandler 7
double_fault_handler:
    ErrcodeHandler 8
coprocessor_segment_overrun_fault_handler:
    NonErrcodeHandler 9
invalid_tss_fault:
    ErrcodeHandler 10
segment_not_present_fault_handler:
    ErrcodeHandler 11
stack_segment_fault_handler:
    ErrcodeHandler 12
general_protection_fault_handler:
    ErrcodeHandler 13
page_fault_handler:
    ErrcodeHandler 14
reserved_handler_1:
    NonErrcodeHandler 15
x87_floating_point_fault_handler:
    NonErrcodeHandler 16
alignment_check_handler:
    ErrcodeHandler 17
machine_check_handler:
    NonErrcodeHandler 18
simd_floating_point_fault_handler:
    NonErrcodeHandler 19
virtualization_fault_handler:
    NonErrcodeHandler 20
control_protection_fault_handler:
    ErrcodeHandler 21
reserved_handler_2:
    NonErrcodeHandler 22
reserved_handler_3:
    NonErrcodeHandler 23
reserved_handler_4:
    NonErrcodeHandler 24
reserved_handler_5:
    NonErrcodeHandler 25
reserved_handler_6:
    NonErrcodeHandler 26
reserved_handler_7:
    NonErrcodeHandler 27
hypervisor_injection_exception:
    NonErrcodeHandler 28
vmm_communication_fault_handler:
    ErrcodeHandler 29
security_fault_handler:
    ErrcodeHandler 30
reserved_handler_8:
    NonErrcodeHandler 31

//-------------------------------------------------

.macro irqHandler vector
    menter

    pushl %esp
    movl $\vector, %eax

    call irq_handler_primary

    addl $4, %esp

    mleave
    iret
.endm

.global irq0_handler
.type   irq0_handler, @function
.global irq1_handler
.type   irq1_handler, @function
.global irq2_handler
.type   irq2_handler, @function
.global irq3_handler
.type   irq3_handler, @function
.global irq4_handler
.type   irq4_handler, @function
.global irq5_handler
.type   irq5_handler, @function
.global irq6_handler
.type   irq6_handler, @function
.global irq7_handler
.type   irq7_handler, @function
.global irq8_handler
.type   irq8_handler, @function
.global irq9_handler
.type   irq9_handler, @function
.global irq10_handler
.type   irq10_handler, @function
.global irq11_handler
.type   irq11_handler, @function
.global irq12_handler
.type   irq12_handler, @function
.global irq13_handler
.type   irq13_handler, @function
.global irq14_handler
.type   irq14_handler, @function
.global irq15_handler
.type   irq15_handler, @function

irq0_handler:
    irqHandler 0
irq1_handler:
    irqHandler 1
irq2_handler:
    irqHandler 2
irq3_handler:
    irqHandler 3
irq4_handler:
    irqHandler 4
irq5_handler:
    irqHandler 5
irq6_handler:
    irqHandler 6
irq7_handler:
    irqHandler 7
irq8_handler:
    irqHandler 8
irq9_handler:
    irqHandler 9
irq10_handler:
    irqHandler 10
irq11_handler:
    irqHandler 11
irq12_handler:
    irqHandler 12
irq13_handler:
    irqHandler 13
irq14_handler:
    irqHandler 14
irq15_handler:
    irqHandler 15