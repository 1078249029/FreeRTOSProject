#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "queue.h"

/****************这种实现队列的方法在发送高频信号时会丢失数据****************/

List_t* QueueList;
#define QueueItemSize  5
#define QueueItemNum  100
char array[QueueItemNum][QueueItemSize];
static UBaseType_t head = 0U;
static UBaseType_t tail = 0U;
int BufferUsed = 0;
BaseType_t BufferSend( void* SendData,
							UBaseType_t uxQueueLength,
							UBaseType_t uxItemSize,
							void* pvDataStore);
BaseType_t BufferReceive(void* ReceiveData,
							UBaseType_t uxQueueLength,
							UBaseType_t uxItemSize,
							void* pvDataStore);

BaseType_t BufferIsEmpty(void);

QueueHandle_t QueueCreat(		/* 队列总长度 */
								UBaseType_t uxQueueLength,
								/* 队列中元素大小 */
								UBaseType_t uxItemSize,
								/* 存储队列结构体的指针，结构体内含重要参数 */
								Queue_t *pxQueueBuffer,
								/* 队列存储数据的指针，在main中开个uxQueueLength*uxItemSize大小的数组即可 */
								int *pucQueueStorageBuffer )
{
	Queue_t* NewQueue;
	NewQueue = pxQueueBuffer;
	/* 初始化Queuelist */
	vListInitialiseItem(&(NewQueue->xQueueListItem));
	vListInitialise(QueueList);
	vListInsertEnd(QueueList,&(NewQueue->xQueueListItem));
	NewQueue->xQueueListItem.pvOwner = (void*)NewQueue;
	NewQueue->xQueueListItem.pvContainer = (void*)QueueList;
	/* 数据存储的位置 */
	NewQueue->pvDataStore = (void*)pucQueueStorageBuffer;
	/* 每个Item的大小 */
	NewQueue->uxItemSize = uxItemSize;
	/* 队列长度 */
	NewQueue->uxQueueLength = uxQueueLength;
	return (void*)NewQueue;
	//QueueList->uxNumberOfItems = 0;//初始化为0	
}
								
//FreeRTOS原版是怎么进行类型转换的？跟我一样，使用void或char类型进行转换
BaseType_t QueueSend(QueueHandle_t QueueHandle, 
								void* const SendData )
{
	Queue_t* QueueTemp;
	QueueTemp = (Queue_t*)QueueHandle;
	BaseType_t rtval = pdTRUE;
	/* 关中断 */
	portDISABLE_INTERRUPTS();
	/* 发送数据 */
	rtval = BufferSend(	SendData,
						QueueTemp->uxQueueLength,
						QueueTemp->uxItemSize,
 						QueueTemp->pvDataStore);
	BufferUsed = 1;
	/* 开中断 */
	portENABLE_INTERRUPTS();
	/* 返回值为判断QueueSend是否执行成功 */
	return rtval;
}

BaseType_t QueueReceive(QueueHandle_t QueueHandle, 
							void* const ReceiveData )
{

	Queue_t* QueueTemp;
	QueueTemp = (Queue_t*)QueueHandle;
	BaseType_t rtval = pdFALSE;
	/* 关中断 */
	portDISABLE_INTERRUPTS();
	rtval = BufferReceive( ReceiveData,
						/* 只能使用QueueTemp这种中间变量做强制类型转换，如果写成
						((Queue_t*)QueueHandle)->uxQueueLength则会报错 */
						QueueTemp->uxQueueLength,
						QueueTemp->uxItemSize,
						QueueTemp->pvDataStore);
	BufferUsed = 0;
	/* 开中断 */
	portENABLE_INTERRUPTS();
	return rtval;
}


/***************环形缓冲区,不进行缓冲区满的判定***************/
						
BaseType_t BufferSend( void* SendData,
							UBaseType_t uxQueueLength,
							UBaseType_t uxItemSize,
							void* pvDataStore)
{
	int i = 0;
	/* head是环状缓冲区的元素序号的标志位，当head小于元素个数时，将head所在的元素序号
	的内容传入缓冲区，之后head++，直至遇到缓冲区末尾 */
	for(head = 0; head <= uxQueueLength/uxItemSize; head++)
	{
		/* head遇到缓冲区末尾时，将head复位，并跳出循环 */
		if(head == uxQueueLength/uxItemSize)
		{
			head = 0U;//也可以这么写 head %= uxQueueLength/uxItemSize;
			break;	
		}
		/* 需要改进 */
		/* 这里注释掉的原因是防止发送数据时重复发送，比如发送0x14这个数值，
		如果此时uxItemSize的大小远小于uxQueueLength，那么这个段程序就会循环多次，最后发送的
		则是0x14141414，改进办法是将数据按位发送 */
//		for(i = 0; i < uxItemSize; i++)
//		{
			/* 使用char*类型强转后再取值，使得可以获取void*所指向的数据，并
			将数据发送到缓冲区的对应位置，值得注意的是，这里如果不使用char*而使用void*则会
			出现错误，因为没有指定数据类型，编译器无法推断每次移动i的距离大小 */
			((char*)pvDataStore)[head*i] = *((char*)SendData);
//		}
	}
	return pdTRUE;
}
							

BaseType_t BufferReceive(void* ReceiveData,
							UBaseType_t uxQueueLength,
							UBaseType_t uxItemSize,
							void* pvDataStore)
{
	int i = 0;
	
	/* 与head一样，每读取一个元素的所有内容后tail的指针就移动一位，移动后检查缓存区是否空，
	空的话返回pdFALSE，不为空则检查tail是否到缓冲区尾部，到了的话就复位tail，最后发送数据*/
	for(tail = 0; tail <= uxQueueLength/uxItemSize; tail++ )
	{
		if(BufferIsEmpty() == pdFALSE)
		{
			if(tail == uxQueueLength/uxItemSize)
			{
				tail = 0U;
				break;
			}
			/* 注释原因同上 */
//			for(i = 0; i < uxItemSize; i++)
//			{
				((char*)ReceiveData)[tail*i] = *((char*)pvDataStore) ;
//			}			
		}
		else
		{
			return pdFALSE;
		}		
	}
	return pdTRUE;
}


/* 判断环状缓冲区是否为空，空的话返回pdTURE */
BaseType_t BufferIsEmpty(void)
{
	if (head == tail && BufferUsed == 0)
	{
		return pdTRUE;
	}
	else
	{
		return pdFALSE;
	}
}


