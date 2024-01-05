
#include "FreeRTOS.h"
#include "task.h"
#include "ARMCM3.h"

#define portINITIAL_XPSR			( 0x01000000 )
#define portSTART_ADDRESS_MASK		( ( StackType_t ) 0xfffffffeUL )

/*
* 参考资料《STM32F10xxx Cortex-M3 programming manual》4.4.7,百度搜索“PM0056”即可找到这个文档
* 在 Cortex-M 中,内核外设 SCB 中 SHPR3 寄存器用于设置 SysTick 和 PendSV 的异常优先级
* System handler priority register 3 (SCB_SHPR3) SCB_SHPR3:0xE000 ED20
* Bits 31:24 PRI_15[7:0]: Priority of system handler 15, SysTick exception
* Bits 23:16 PRI_14[7:0]: Priority of system handler 14, PendSV
*/

/* 对0xe000ed20地址处取值，此处为SHPR3寄存器，设置的是pendsv和systick优先级 */
#define portNVIC_SYSPRI2_REG	*(( volatile uint32_t *) 0xe000ed20) 
#define portNVIC_PENDSV_PRI 	(((uint32_t) configKERNEL_INTERRUPT_PRIORITY ) << 16UL)
#define portNVIC_SYSTICK_PRI 	(((uint32_t) configKERNEL_INTERRUPT_PRIORITY ) << 24UL )


static void prvTaskExitError( void )
{

/* 函数停止在这里 */

for (;;);
}


/*  */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack,
									TaskFunction_t pxCode, //任务入口，也是函数名称
									void *pvParameters )
{
	/* 异常发生时,自动加载到 CPU 寄存器的内容 */
	pxTopOfStack--;
	*pxTopOfStack = portINITIAL_XPSR;
	pxTopOfStack--;
	*pxTopOfStack = ( ( StackType_t ) pxCode ) & portSTART_ADDRESS_MASK; 
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) prvTaskExitError;
	pxTopOfStack -= 5; /* R12, R3, R2 and R1 默认初始化为 0 */
	*pxTopOfStack = ( StackType_t ) pvParameters;
	/* 异常发生时,手动加载到 CPU 寄存器的内容 */
	pxTopOfStack -= 8;
	/* 返回栈顶指针,此时 pxTopOfStack 指向空闲栈 */
	return pxTopOfStack;
}

/* 通过查找SCB_VTOR最终将__initial_sp处的指令传到msp内，然后调用svc启动第一个任务 */
__asm void prvStartFirstTask( void )
{
	/* 当前栈按照8字节对齐 */
	PRESERVE8

	/* 将SCB_VTOR寄存器地址加载到R0，SCB_VTOR寄存器存储__initial_sp的地址，
	__initial_sp也是msp的地址，还是向量表的起始地址，因为CM3支持更改向量表的起始地址，
	所以需要以下四条指令以重定位__initial_sp*/
	ldr r0, =0xE000ED08
	/* 将__initial_sp的地址加载进r0,STM32的__initial_sp为0x0800 0000 */
	ldr r0, [r0]
	/* 将__initial_sp中的内容，也就是msp初始化的值加载到r0，可能是0x20005B30 */
	ldr r0, [r0]

	/* 将__initial_sp初始化的值加载到msp */
	msr msp, r0

	/* 开中断 */
	cpsie i
	cpsie f
	dsb
	isb

	/* 调用SVC去启动第一个任务 */
	svc 0
	nop
	nop
}

__asm void vPortSVCHandler( void )
{

 extern pxCurrentTCB;
 PRESERVE8
 ldr r3, =pxCurrentTCB
 ldr r1, [r3]
 ldr r0, [r1]
 ldmia r0!, {r4-r11}
 msr psp, r0
 isb
 mov r0, #0
 msr basepri, r0
 orr r14, #0xd
 bx r14
}

 __asm void xPortPendSVHandler( void )
{
	extern pxCurrentTCB;
	extern vTaskSwitchContext;

	PRESERVE8

	mrs r0, psp
	isb
	ldr r3, =pxCurrentTCB
	ldr r2, [r3]
	stmdb r0!, {r4-r11}
	str r0, [r2]
	stmdb sp!, {r3, r14}
	mov r0, #configMAX_SYSCALL_INTERRUPT_PRIORITY
	msr basepri, r0
	dsb
	isb
	bl vTaskSwitchContext
	mov r0, #0
	msr basepri, r0
	ldmia sp!, {r3, r14}
	ldr r1, [r3]
	ldr r0, [r1]
	ldmia r0!, {r4-r11}
	msr psp, r0
	isb
	bx r14
	nop
}

BaseType_t xPortStartScheduler( void )
{
	 /* 配置 PendSV 和 SysTick 的中断优先级为最低 */
	 portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;	//SHPR3寄存器被设置为 0x**FF ****
	 portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;	//SHPR3寄存器被设置为 0xFFFF ****

	/* 启动调度器，并陷入 */
	 prvStartFirstTask();

	/* 不会执行到这里 */
	 return 0;
}

