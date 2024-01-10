#ifndef FREERTOS_H
#define FREERTOS_H

#include "FreeRTOSConfig.h"
#include "projdefs.h"
#include "portable.h"
#include "list.h"


typedef struct tskTaskControlBlock
{
	 /* 栈顶 */
	 volatile StackType_t *pxTopOfStack;	//可以改为韦讲的其他方法
	 /* 任务节点，内部包含了通用链表的关键项，例如前后指针，owner和container */
	 ListItem_t xStateListItem;
	 /* 任务栈起始地址 */
	 StackType_t *pxStack;
	 /* 任务名称,字符串形式 */
	 char pcTaskName[ configMAX_TASK_NAME_LEN ];
	 /* 任务延时时间 */
	 TickType_t xTicksToDelay; 
	 /* 任务优先级 */
	 UBaseType_t uxPriority;
} tskTCB;
typedef tskTCB TCB_t;

#endif /* FREERTOS_H */
