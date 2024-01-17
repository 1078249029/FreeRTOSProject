#ifndef QUEUE_H
#define QUEUE_H

#include "list.h"

struct xQUEUE_ITEM
{
	UBaseType_t uxQueueLength;
	UBaseType_t uxItemSize;
	ListItem_t xQueueListItem;
	void* pvDataStore;
};
typedef struct xQUEUE_ITEM Queue_t;
typedef void * QueueHandle_t;
QueueHandle_t QueueCreat(		UBaseType_t uxQueueLength,
								UBaseType_t uxItemSize,
								Queue_t *pxQueueBuffer,
								int *pucQueueStorageBuffer );
BaseType_t QueueSend(QueueHandle_t QueueHandle, 
								void* const SendData );
BaseType_t QueueReceive(QueueHandle_t QueueHandle, 
							void* const ReceiveData );




#endif

