
#include "FreeRTOS.h"
#include "task.h"

List_t pxReadyTasksLists[ configMAX_PRIORITIES ];
TCB_t volatile *pxCurrentTCB = NULL;

static volatile UBaseType_t uxCurrentNumberOfTasks 	= ( UBaseType_t ) 0U;
static volatile TickType_t xTickCount 				= ( TickType_t ) 0U;


extern TCB_t Task1TCB;
extern TCB_t Task2TCB;

static void prvInitialiseNewTask(TaskFunction_t pxTaskCode,
										  const char * const pcName,
										  const uint32_t ulStackDepth,
										  void * const pvParameters,
										  TaskHandle_t * const pxCreatedTask, 
										  TCB_t *pxNewTCB )
{
	StackType_t *pxTopOfStack;
	UBaseType_t x;

	/* 获取栈顶地址 */
	pxTopOfStack = pxNewTCB->pxStack + (ulStackDepth - (uint32_t)1);
	
	/* 栈顶地址向高地址做 8 字节对齐,32位机只需要四字节对齐即可，但是需要考虑兼容浮点运算的64位操作 */
 	pxTopOfStack = (StackType_t*)(((uint32_t)pxTopOfStack)&(~(uint32_t)0x0007));

	/* 将任务的名字存储在 TCB 中 */
	for( x = 0; x < (UBaseType_t)configMAX_TASK_NAME_LEN && pcName[x] == 0; x++ )
	//条件判定可以么？没问题！但是pcName[x] == '\0'则不可以，复习0与'\0'的区别
	{
		pxNewTCB->pcTaskName[x] = pcName[x];
	}

	/* 任务名字的长度不能超过 configMAX_TASK_NAME_LEN */
	pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN - 1] = '\0';

	/* 初始化 TCB 中的 xStateListItem 节点 */
	vListInitialiseItem(&(pxNewTCB->xStateListItem));

 	/* 设置 xStateListItem 节点的拥有者 */
	listSET_LIST_ITEM_OWNER(&(pxNewTCB->xStateListItem),pxNewTCB);
	
	/* 初始化任务栈 */
	pxNewTCB->pxTopOfStack = (StackType_t*)pxPortInitialiseStack( pxTopOfStack, pxTaskCode, pvParameters ); 
	
	/* 让任务句柄指向任务控制块 */
	*pxCreatedTask = (TaskHandle_t)pxNewTCB;//对pxNewTCB取值如何？pxCreatedTask为const类型的指针，不能修改地址
	
}


/* 外部需要调用的任务创建函数 */
#if( configSUPPORT_STATIC_ALLOCATION == 1 )
 TaskHandle_t xTaskCreateStatic( TaskFunction_t pxTaskCode,			 //任务入口，也是函数名称
								 const char * const pcName,
								 const uint32_t ulStackDepth,
								 void * const pvParameters,
								 StackType_t * const puxStackptr, //任务块起始地址指针
								 TCB_t * const pxTaskCtrlptr )		 //任务块控制指针

{
	TCB_t *pxNewTCB;	//在这里生成TCB结构体
	TaskHandle_t xReturn;//TaskHandle_t 是 void* 类型

	if( puxStackptr != NULL && pxTaskCtrlptr != NULL )//防止非法传参
	{
		pxNewTCB = pxTaskCtrlptr;
		pxNewTCB->pxStack = puxStackptr;

		/* 创建新任务 */
		prvInitialiseNewTask(pxTaskCode,pcName,ulStackDepth,pvParameters,&xReturn,pxNewTCB);
	}
	else
	{
		xReturn = NULL;
	}

	/* 返回任务句柄 */
	return xReturn; 
}
#endif /* configSUPPORT_STATIC_ALLOCATION */

/* 初始化任务列表 */
void prvInitialiseTaskLists( void )
{

 	UBaseType_t uxPriority;
	for( uxPriority = (UBaseType_t)0U; 
		uxPriority < (UBaseType_t)configMAX_PRIORITIES; 
		uxPriority++ )
	{
		/* 优先级列表的初始化 */
		vListInitialise(&(pxReadyTasksLists[uxPriority]));
	}
}

extern TCB_t IdleTaskTCB;
extern TaskHandle_t xIdleTaskHandle;

void vApplicationGetIdleTaskMemory( TCB_t **ppxIdleTaskTCBBuffer,
												StackType_t **ppxIdleTaskStackBuffer,
												uint32_t *pulIdleTaskStackSize );
portCHAR flagIdle = 0;
extern portCHAR flag1;
extern portCHAR flag2;

void prvIdleTask( void *p_arg )
{

/* 永远不要在空闲任务里加入阻塞或者死循环！！！！！！！！！！！！！！！
	否则当其他任务优先级为0时，空闲任务会霸占整个时间片*/
//	for( ;; )
//	{
////		flagIdle = 1;
////		flag1 = 0;
////		flag2 = 0;
//	}
}

void vTaskStartScheduler( void )
{
/*=======================创建空闲任务 start=======================*/
	 TCB_t *pxIdleTaskTCBBuffer = NULL;
	 /* 用于指向空闲任务控制块 */
	 StackType_t *pxIdleTaskStackBuffer = NULL; /* 用于空闲任务栈起始地址 */
	 uint32_t ulIdleTaskStackSize;
	 /* 获取空闲任务的内存:任务栈和任务 TCB */ 
	 vApplicationGetIdleTaskMemory(  &pxIdleTaskTCBBuffer,
									 &pxIdleTaskStackBuffer,
									 &ulIdleTaskStackSize );
	 
	 /* 创建空闲任务 */ 
	 xIdleTaskHandle =
	 xTaskCreateStatic( (TaskFunction_t)prvIdleTask,
						 /* 任务入口 */
						 (char *)"IDLE",
						 /* 任务名称,字符串形式 */
						 (uint32_t)ulIdleTaskStackSize , /* 任务栈大小,单位为字 */
						 (void *) NULL,
						 /* 任务形参 */
						 (StackType_t *)pxIdleTaskStackBuffer, /* 任务栈起始地址 */
						 (TCB_t *)pxIdleTaskTCBBuffer ); /* 任务控制块 */

	/* 将任务添加到就绪列表 */
	vListInsertEnd( &( pxReadyTasksLists[0] ),&( ((TCB_t *)pxIdleTaskTCBBuffer)->xStateListItem ) );

/*==========================创建空闲任务 end=====================*/
	/* 手动指定第一个运行的任务 */
	pxCurrentTCB = &Task1TCB;
	/* 启动调度器 */
	xTickCount = ( TickType_t ) 0U;
	if ( xPortStartScheduler() != pdFALSE )
	{
	}

}




void vTaskDelay( const TickType_t xTicksToDelay )
{
	TCB_t *pxTCB = NULL;
	pxTCB = (TCB_t *)pxCurrentTCB;
	pxTCB->xTicksToDelay = xTicksToDelay;

	/* 为什么非得任务切换？因为pxCurrentTCB已经被设置延时了，
	假如延时不为0的话整个系统都会被阻塞，若pxCurrentTCB不设置延时，这也会破坏优先级执行 */
//	if( pxTCB->xTicksToDelay != 0 )
//	{
//		taskYIELD();
//	}
	taskYIELD();
}

#if 0

/* 切换上下文 */
void vTaskSwitchContext( void )
{    
    if( pxCurrentTCB == &Task1TCB )
    {
        pxCurrentTCB = &Task2TCB;
    }
    else
    {
        pxCurrentTCB = &Task1TCB;
    }
}

#else


void vTaskSwitchContext( void )
{
//	/*
//	TCB_t  *pxtempTCB = NULL;
//	int i = 0;
//	if( pxCurrentTCB.xTicksToDelay == 0 )
//	{
//		for( i = configMAX_PRIORITIES-1; i >= 0; i-- )
//		{
//			//怎么从链表中取出TCB?
//			((TCB_t*)(pxReadyTasksLists[i]->xListEnd->pxNext->pvOwner))->xTicksToDelay != 0;
//			//下面是正确做法，为什么？
//			pxtempTCB = ((TCB_t*)(pxReadyTasksLists[i]->xListEnd->pxNext->pvOwner));
//			pxtempTCB->xTicksToDelay != 0;
//		}
//		
//	}*/ 
	if ( pxCurrentTCB == &IdleTaskTCB )
	{
		if( Task1TCB.xTicksToDelay == 0 )
		{
			pxCurrentTCB = &Task1TCB;
		}
		else if( Task2TCB.xTicksToDelay == 0 )
		{
			pxCurrentTCB = &Task2TCB;
		}
		else
		{
			return;
		}
	}
	else if( pxCurrentTCB == &Task1TCB )
	{
		
		if( Task2TCB.xTicksToDelay == 0 )
		{
			pxCurrentTCB = &Task2TCB;
		}
		else if( Task1TCB.xTicksToDelay != 0 )
		{
			pxCurrentTCB = &IdleTaskTCB;
		}
		else
		{
			return;
		}
	}
	else if( pxCurrentTCB == &Task2TCB )
	{
		if( Task1TCB.xTicksToDelay == 0 )
		{
			pxCurrentTCB = &Task1TCB;
		}
		else if( Task2TCB.xTicksToDelay != 0 )
		{
			pxCurrentTCB = &IdleTaskTCB;
		}
		else
		{
			return;
		}
	}

//	if( pxCurrentTCB == &Task1TCB )
//	   {
//		   pxCurrentTCB = &Task2TCB;
//	   }
//	else
//	   {
//		   pxCurrentTCB = &Task1TCB;
//	   }
	
}


#endif

/* 在xPortSysTickHandler中被调用，也就是SysTick_Handler 此函数负责时基自增，任务计时自减 */
void xTaskIncrementTick( void )
{
	TCB_t *pxTCB = NULL;
 	BaseType_t i = 0;

	/* 更新系统时基计数器 xTickCount,xTickCount 是一个在 port.c 中定义的全局变量 */
 	const TickType_t xConstTickCount = xTickCount + 1;//常量赋值？
 	xTickCount = xConstTickCount;

	/* 扫描就绪列表中所有任务的 xTicksToDelay,如果不为 0,则减 1 */
	for( i = 0; i < configMAX_PRIORITIES; i++ )
	{
		pxTCB = ((TCB_t*)((pxReadyTasksLists[i].xListEnd.pxNext)->pvOwner));
		
		//也可以用这种方法
		//pxTCB = (TCB_t * ) listGET_OWNER_OF_HEAD_ENTRY( ( &pxReadyTasksLists[i] ) );
		if( pxTCB->xTicksToDelay > 0 )
		{
			pxTCB->xTicksToDelay --;
		}
	}
	taskYIELD();
}


