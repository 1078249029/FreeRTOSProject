#include "task.h"
#include "FreeRTOS.h"
#include "portable.h"

List_t pxReadyTasksLists[ configMAX_PRIORITIES ];
TCB_t volatile *pxCurrentTCB = NULL;

extern TCB_t Task1TCB;



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
	pxTopOfStack = pxNewTCB->pxTopOfStack + (ulStackDepth - (uint32_t)1);
	
	/* 栈顶地址向高地址做 8 字节对齐,32位机只需要四字节对齐即可，但是需要考虑兼容浮点运算的64位操作 */
 	pxTopOfStack = (StackType_t)(((uint32_t)pxTopOfStack)&(~(uint32_t)0x0007)

	/* 将任务的名字存储在 TCB 中 */
	for( x = 0; x < (UBaseType_t)configMAX_TASK_NAME_LEN && pcTaskName[x] == 0x0000; x++ )
	//条件判定可以么？
	{
		pxNewTCB->pcTaskName[x] = pcName[x]
	}

	/* 任务名字的长度不能超过 configMAX_TASK_NAME_LEN */
	pxNewTCB->pcTaskName[configMAX_TASK_NAME_LEN - 1] = '\0';

	/* 初始化 TCB 中的 xStateListItem 节点 */
	vListInitialiseItem(pxNewTCB->xStateListItem);

 	/* 设置 xStateListItem 节点的拥有者 */
	listSET_LIST_ITEM_OWNER(pxNewTCB->xStateListItem,pxNewTCB)//不取地址如何？
	
	/* 初始化任务栈 */
	pxNewTCB->pxTopOfStack = pxPortInitialiseStack( pxTopOfStack,pxTaskCode,pvParameters );

	/* 让任务句柄指向任务控制块 */
	*pxCreatedTask = (TaskHandle_t)(*pxNewTCB)//对pxNewTCB取值如何？
	
}



#if( configSUPPORT_STATIC_ALLOCATION == 1 )
 TaskHandle_t xTaskCreateStatic( TaskFunction_t pxTaskCode,			 //任务入口，也是函数名称
								 const char * const pcName,
								 const uint32_t ulStackDepth,
								 void * const pvParameters,
								 StackType_t * const puxStackBuffer, //任务块起始地址
								 TCB_t * const pxTaskBuffer )		 //任务块控制指针

{
	TCB_t *pxNewTCB;	//在这里生成TCB结构体
	TaskHandle_t xReturn;//TaskHandle_t 是 void* 类型

	if( puxStackBuffer != NULL && pxTaskBuffer != NULL )//防止非法传参
	{
		pxNewTCB = pxTaskBuffer;
		pxNewTCB->pxStack = puxStackBuffer;

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

void prvInitialiseTaskLists( void )
{

 	UBaseType_t uxPriority;
	for( uxPriority = (UBaseType_t)0; 
		uxPriority < (UBaseType_t)configMAX_PRIORITIES; 
		uxPriority++ )
	{
		/* 优先级列表的初始化 */
		vListInitialise(&pxReadyTasksLists[uxPriority]);
	}
}


void vTaskStartScheduler( void )
{
	pxCurrentTCB = &Task1TCB;	//手动指定第一个任务
	
	if(  xPortStartScheduler() != pdFALSE )
	{
		/* xPortStartScheduler启动成功后不会执行到return语句，因此不会执行到此行 */
	}
}





