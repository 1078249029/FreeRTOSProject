#ifndef SEMAPHORE_H
#define SEMAPHORE_H

struct xSEMAPHORE_ITEM
{
	UBaseType_t MaxCount;//定义信号量最大值
	UBaseType_t InitialCount;//定义信号量初始值
	UBaseType_t InitialFlag;
	ListItem_t SemaphoreListItem;
};
typedef struct xSEMAPHORE_ITEM Semaphore_t;


typedef void * SemaphoreHandle_t;
void xSemaphoreIncrementTick( void );
SemaphoreHandle_t SemaphoreCreat(Semaphore_t* const Semaphore,UBaseType_t MaxCount,UBaseType_t InitialCount);
void SemaphoreTake(SemaphoreHandle_t* const Semaphore,BaseType_t WaitTime);
void SemaphoreGive(SemaphoreHandle_t* const Semaphore);

#endif

