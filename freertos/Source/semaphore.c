#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "Semaphore.h"
static volatile TickType_t xTickCount 				= ( TickType_t ) 0U;
//static TickType_t SemaphoreTime;//定义信号量的解锁时间
List_t* SemaphoreList;


SemaphoreHandle_t SemaphoreCreat(Semaphore_t* const Semaphore,UBaseType_t MaxCount,UBaseType_t InitialCount)
{
	
	Semaphore_t* NewSemaphore;
	NewSemaphore = Semaphore;

	vListInitialise(SemaphoreList);
	vListInsert(SemaphoreList,&(NewSemaphore->SemaphoreListItem));//新产生的链表项已经挂载到链表上了
	NewSemaphore->InitialCount = InitialCount;
	NewSemaphore->MaxCount = MaxCount;
	
	NewSemaphore->SemaphoreListItem.xItemValue = 0;//等待时间
	NewSemaphore->InitialFlag = 0;//初始化设置时间标志位

	SemaphoreList->pxIndex = &(NewSemaphore->SemaphoreListItem);

	return (void*)NewSemaphore;
}

void SemaphoreTake(SemaphoreHandle_t* const Semaphore,BaseType_t WaitTime)
{
	Semaphore_t* NewSemaphore;
	NewSemaphore = (Semaphore_t*)Semaphore;
	//更新信号量链表项的等待时间	
	if(NewSemaphore->InitialFlag == 0)
	{
		SemaphoreList->pxIndex->xItemValue = WaitTime + xTickCount;
		NewSemaphore->InitialFlag = 1;//设置时间标志位为1
	}
	if(NewSemaphore->InitialCount > (UBaseType_t)0)
	{
		portDISABLE_INTERRUPTS();
		NewSemaphore->InitialCount --;
	}
	else
	{
		taskYIELD();
	}
	//计数
	
	
}

void SemaphoreGive(SemaphoreHandle_t* const Semaphore)
{
	Semaphore_t* NewSemaphore;
	NewSemaphore = (Semaphore_t*)Semaphore;
	NewSemaphore->InitialFlag = 0;
	NewSemaphore->InitialCount ++;
	if(NewSemaphore->InitialCount > (UBaseType_t)0)
	portENABLE_INTERRUPTS();
}

void xSemaphoreIncrementTick( void )
{
	const TickType_t xConstTickCount = xTickCount + 1;
 	xTickCount = xConstTickCount;
}


