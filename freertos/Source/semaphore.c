#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "Semaphore.h"
static volatile TickType_t xTickCount 				= ( TickType_t ) 0U;
//static TickType_t SemaphoreTime;//定义信号量的解锁时间
List_t* SemaphoreList;
static volatile UBaseType_t uxCurrentNumberOfSemaphore 	= ( UBaseType_t ) 0U;

/**************************************使用链表创建信号量***************************************/
SemaphoreHandle_t SemaphoreCreat(Semaphore_t* const Semaphore,
										UBaseType_t MaxCount,
										UBaseType_t InitialCount)
{
	
	Semaphore_t* NewSemaphore;
	NewSemaphore = Semaphore;
	/* 判断信号量链表的信号量个数，若为0则初始化链表 */
	if(uxCurrentNumberOfSemaphore == ( UBaseType_t ) 0U)
	{
		vListInitialiseItem(SemaphoreList->pxIndex);
		vListInitialise(SemaphoreList);
		//新产生的链表项已经挂载到链表上了???
		vListInsertEnd(SemaphoreList,&(NewSemaphore->SemaphoreListItem));		
		uxCurrentNumberOfSemaphore++;
	}
	/* 不为0的话在链表尾部插入信号量 */
	else
	{
		vListInsert(SemaphoreList,&(NewSemaphore->SemaphoreListItem));
		uxCurrentNumberOfSemaphore++;
	}
	/* 设置信号量的具体参数 */
	NewSemaphore->InitialCount = InitialCount;
	NewSemaphore->MaxCount = MaxCount;
	/* 只设置新semaphore的ListItem的参数，不设置List下面的ListItem的参数，因为两者相同，
	List下ListItem装载的就是TCB或者其他内核对象的参数 */
	NewSemaphore->SemaphoreListItem.pvOwner = (void*)NewSemaphore;
	NewSemaphore->SemaphoreListItem.pvContainer = (void*)SemaphoreList;
	NewSemaphore->SemaphoreListItem.xItemValue = 0;//等待时间
	/* 初始化设置时间标志位 */
	NewSemaphore->InitialFlag = 0;
	
	return (void*)NewSemaphore;
}

void SemaphoreTake(SemaphoreHandle_t* const Semaphore,BaseType_t WaitTime)
{
	Semaphore_t* NewSemaphore;
	NewSemaphore = (Semaphore_t*)Semaphore;
	int i = 0;
	/* 需要改进 */
//	for(i = 0; i <= uxCurrentNumberOfSemaphore; i++)//多信号量的尝试，暂时失败了
//	{
//		if(SemaphoreList->pxIndex->pvOwner == (void*)NewSemaphore)
//		{
			if(NewSemaphore->InitialFlag == 0)
			{
				/* 更新信号量链表项的等待时间，暂时没用到	*/
				SemaphoreList->pxIndex->xItemValue = WaitTime + xTickCount;
				/* 设置时间标志位为1 */
				NewSemaphore->InitialFlag = 1;
			}
			/* 如果当前信号量数量大于0，那么关中断，防止其他任务使用临界资源，
			并且之后信号量--，意味着发生了Take */
			if(NewSemaphore->InitialCount > (UBaseType_t)0)
			{
				portDISABLE_INTERRUPTS();
				NewSemaphore->InitialCount --;
			}
			/* 当前信号量小于等于0，那么不执行当前任务，于是进行任务切换 */
			else
			{
				taskYIELD();
			}
//		}
//		else
//		{
//			SemaphoreList->pxIndex = SemaphoreList->pxIndex->pxNext;
//		}		
//	}
	
}

void SemaphoreGive(SemaphoreHandle_t* const Semaphore)
{
	/* Give的要求不多，重置初始化Falg的值，信号量++，最后别忘了开中断，这样任务才切换，
	临界资源才能被别的任务访问 */
	Semaphore_t* NewSemaphore;
	NewSemaphore = (Semaphore_t*)Semaphore;
	NewSemaphore->InitialFlag = 0;
	NewSemaphore->InitialCount ++;
	portENABLE_INTERRUPTS();
}

/* 待完善的portMAX_DELAY以及具有相关时间函数功能 */
void xSemaphoreIncrementTick( void )
{
	const TickType_t xConstTickCount = xTickCount + 1;
 	xTickCount = xConstTickCount;
}


