
#include "FreeRTOS.h"
#include "task.h"
#include "ARMCM3.h"

#define portINITIAL_XPSR			( 0x01000000 )
#define portSTART_ADDRESS_MASK		( ( StackType_t ) 0xfffffffeUL )
static UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

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

/* SysTick 控制寄存器 */
#define portNVIC_SYSTICK_CTRL_REG (*((volatile uint32_t *) 0xe000e010 ))
/* SysTick 重装寄存器 */
#define portNVIC_SYSTICK_LOAD_REG (*((volatile uint32_t *) 0xe000e014 ))


/* SysTick 时钟源选择 */
#ifndef configSYSTICK_CLOCK_HZ
	#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
/* 确保 SysTick 的时钟与内核时钟一致 */
	#define portNVIC_SYSTICK_CLK_BIT	( 1UL << 2UL )
#else
	#define portNVIC_SYSTICK_CLK_BIT	( 0 )
#endif


#define portNVIC_SYSTICK_INT_BIT 	( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT	 ( 1UL << 0UL )


void vPortSetupTimerInterrupt( void )
{
	/* 设置重装寄存器,别忘了从零开始计数，需要减1哦 */
	portNVIC_SYSTICK_LOAD_REG = (configSYSTICK_CLOCK_HZ/configTICK_RATE_HZ)-1UL;

	/* 设置系统时钟等于内核时钟，使能定时器及其中断 */
	portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT |
								portNVIC_SYSTICK_INT_BIT |
								portNVIC_SYSTICK_ENABLE_BIT );
}


static void prvTaskExitError( void )
{

/* 没有可供执行的任务时会停在这里，如果发生了这种情况，看一下空闲任务是否被执行 */

/* 函数停止在这里 */
for (;;);

}


StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack,
									TaskFunction_t pxCode, //任务入口，也是函数名称
									void *pvParameters )
{
	/* 异常发生时,自动加载到 CPU 寄存器的内容 */
	pxTopOfStack--;
	*pxTopOfStack = portINITIAL_XPSR;/* xPSR的bit24必须置1 */
	pxTopOfStack--;
	*pxTopOfStack = ( ( StackType_t ) pxCode ) & portSTART_ADDRESS_MASK; /* PC，即任务入口函数 */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) prvTaskExitError;/* LR，函数返回地址 */
	pxTopOfStack -= 5; /* R12, R3, R2 and R1 默认初始化为 0 */
	*pxTopOfStack = ( StackType_t ) pvParameters;/* R0，任务形参 */
	/* 异常发生时,手动加载到 CPU 寄存器的内容 */
	pxTopOfStack -= 8;/* R11, R10, R9, R8, R7, R6, R5 and R4默认初始化为0 */
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
 msr basepri, r0	//开中断
 orr r14, #0xd		//设置LR的值
 bx r14				//此处不会返回r14(LR),而是返回到任务堆栈，具体看CM3手册
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
	bx r14	//此处不会返回r14(LR),而是返回到任务堆栈，具体看CM3手册
	nop
}

BaseType_t xPortStartScheduler( void )
{
	 /* 配置 PendSV 和 SysTick 的中断优先级为最低 */
	 portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;	//SHPR3寄存器被设置为 0x**FF ****
	 portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;	//SHPR3寄存器被设置为 0xFFFF ****

	/* 初始化时钟及其中断 */
	vPortSetupTimerInterrupt();

	/* 启动调度器，并陷入 */
	 prvStartFirstTask();

	/* 不会执行到这里 */
	 return 0;
}

/* 不能在中断中使用 */
void vPortEnterCritical( void )
{
	portDISABLE_INTERRUPTS();
	uxCriticalNesting++;
	
	if( uxCriticalNesting == 1 )
	{
		//configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );
		//野火没有解除注释，因为没有定义configASSERT
	}
}

void vPortExitCritical(void)
{
	uxCriticalNesting--;
	if( uxCriticalNesting == 0 )
	{
		portENABLE_INTERRUPTS();
	}
}

/********************************新增代码**********************************/



/* 负责进入增加时基，任务时间自减的函数，定义在Config.h:
#define xPortSysTickHandler	SysTick_Handler 也就是按配置的重装器和主频来触发 */
void xPortSysTickHandler( void )
{
	uint32_t ISRreturn;
	/* 关中断 */
	//使用的是能在中断中使用的函数会如何？无影响,第九章实验现象可以完成，但是因为SysTick优先级过低而无用
	ISRreturn = portSET_INTERRUPT_MASK_FROM_ISR();	
	/* 更新系统时基 */
	//xTaskIncrementTick();
	if(xTaskIncrementTick() != pdFALSE)
	{
		taskYIELD();
	}

	/* 开中断 */
	portCLEAR_INTERRUPT_MASK_FROM_ISR(ISRreturn);


//	vPortRaiseBASEPRI();
//	xTaskIncrementTick();
//	vPortClearBASEPRIFromISR();
}

