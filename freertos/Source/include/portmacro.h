	
/*本文件配置移植项，与机器位数相关*/

#ifndef PORTMACRO_H
#define PORTMACRO_H

#include "stdint.h"
#include "stddef.h"

 /* 数据类型重定义 */
#define portCHAR char
#define portFLOAT float
#define portDOUBLE double
#define portLONG long
#define portSHORT short
#define portSTACK_TYPE uint32_t
#define portBASE_TYPE long

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
typedef uint16_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffff
#else
typedef uint32_t TickType_t;
#define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif

#define portNVIC_INT_CTRL_REG		( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_PENDSVSET_BIT		( 1UL << 28UL )
#define portSY_FULL_READ_WRITE		( 15 )

/* 触发 PendSV,产生上下文切换 */
#define portYIELD()																\
{																				\
	portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;								\
	__dsb( portSY_FULL_READ_WRITE );											\
	__isb( portSY_FULL_READ_WRITE );											\
}

#define portENTER_CRITICAL()					vPortEnterCritical()
#define portEXIT_CRITICAL()						vPortExitCritical()


/* 不带返回值的关中断函数,不能嵌套,不能在中断里面使用 */
#define portDISABLE_INTERRUPTS() vPortRaiseBASEPRI()

#define portINLINE __inline

#ifndef portFORCE_INLINE
	#define portFORCE_INLINE __forceinline
#endif


static portFORCE_INLINE void vPortRaiseBASEPRI( void )
{
	//中断号大于191的中断全部屏蔽
	uint32_t ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY;    

	__asm
	{
	//将FreeRTOS最大优先级的中断加载到basepri寄存器中，这样会屏蔽FreeRTOS管理的所有中断
	
		msr basepri, ulNewBASEPRI    
		dsb
		isb
	}
}

/* 带返回值的关中断函数,可以嵌套,可以在中断里面使用 */
#define portSET_INTERRUPT_MASK_FROM_ISR() ulPortRaiseBASEPRI()

static portFORCE_INLINE uint32_t ulPortRaiseBASEPRI( void )
{
	uint32_t ulReturn, ulNewBASEPRI = configMAX_SYSCALL_INTERRUPT_PRIORITY; 
	__asm
	{
		mrs ulReturn, basepri    //先对当前中断进行保存并返回
		msr basepri, ulNewBASEPRI
		dsb
		isb
	}
	return ulReturn;
}

/* 带参数就是带中断保护 */
/* 不带中断保护的开中断函数，与portDISABLE_INTERRUPTS()成对使用 */
#define portENABLE_INTERRUPTS() vPortSetBASEPRI( 0 )
/* 带中断保护的开中断函数，与portSET_INTERRUPT_MASK_FROM_ISR()成对使用 */
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x) vPortSetBASEPRI(x) 

/* 设置当前中断优先级，开中断 */
static portFORCE_INLINE void vPortSetBASEPRI( uint32_t ulBASEPRI ) 
{
	__asm
	{
		msr basepri, ulBASEPRI
	}
}

static portFORCE_INLINE void vPortClearBASEPRIFromISR( void )
{
	__asm
	{
		msr basepri, #0
	}
}


#endif /* PORTMACRO_H */

