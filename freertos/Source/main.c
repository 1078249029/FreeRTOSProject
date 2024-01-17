
#include "FreeRTOS.h"
#include "task.h"
#include "semaphore.h"
#include "queue.h"

portCHAR flag1;
portCHAR flag2;
portCHAR flag3;

extern List_t pxReadyTasksLists[ configMAX_PRIORITIES ];


TaskHandle_t Task1_Handle;
#define TASK1_STACK_SIZE                    128
StackType_t Task1Stack[TASK1_STACK_SIZE];
TCB_t Task1TCB;

TaskHandle_t Task2_Handle;
#define TASK2_STACK_SIZE                    128
StackType_t Task2Stack[TASK2_STACK_SIZE];
TCB_t Task2TCB;

TaskHandle_t Task3_Handle;
#define TASK3_STACK_SIZE                    128
StackType_t Task3Stack[TASK3_STACK_SIZE];
TCB_t Task3TCB;

TaskHandle_t xIdleTaskHandle;
#define IdleTASK_STACK_SIZE                    128
StackType_t IdleTaskStack[TASK3_STACK_SIZE];
TCB_t IdleTaskTCB;


void delay (uint32_t count);
void Task1_Entry( void *p_arg );
void Task2_Entry( void *p_arg );
void Task3_Entry( void *p_arg );
void SemaphorePackage1(void);
void SemaphorePackage2(void);

Semaphore_t Semaphore1;
SemaphoreHandle_t SemaphoreHandle1;

QueueHandle_t QueueCreat(		UBaseType_t uxQueueLength,
								UBaseType_t uxItemSize,
								Queue_t *pxQueueBuffer,
								int *pucQueueStorageBuffer );
BaseType_t QueueSend(QueueHandle_t QueueHandle, 
								void* const SendData );
BaseType_t QueueReceive(QueueHandle_t QueueHandle, 
							void* const ReceiveData );



QueueHandle_t QueueHandle1;
Queue_t Queue_t1;
int pucQueueStorageBuffer[600];


void QueuePackage1(void)
{
	int i,j,k = 0;
	for(k = 0; k < 10; k++)
	{
		for(i = 0; i < 1000; i++)
		{
			flag1 = 0;
			delay(100);
			flag1 = 1;
			delay(100);
		}
		for(i = 0; i < 10; i++)
		{
			flag1 = 0;
			delay(10000);
			flag1 = 1;
			delay(10000);
		}
	}
}


int main(void)
{	
    SemaphoreCreat(&Semaphore1, 2, 1);
	SemaphoreGive(&SemaphoreHandle1);

	QueueHandle1 = QueueCreat(600,300,&Queue_t1,pucQueueStorageBuffer);
	
	
    Task1_Handle = xTaskCreateStatic( (TaskFunction_t)Task1_Entry, 
					                  (char *)"Task1",              
					                  (uint32_t)TASK1_STACK_SIZE ,  
					                  (void *) NULL,                
                                      (UBaseType_t) 2,              
					                  (StackType_t *)Task1Stack,    
					                  (TCB_t *)&Task1TCB );        
                                
    Task2_Handle = xTaskCreateStatic( (TaskFunction_t)Task2_Entry, 
					                  (char *)"Task2",              
					                  (uint32_t)TASK2_STACK_SIZE ,  
					                  (void *) NULL,                
                                      (UBaseType_t) 2,                                                      
					                  (StackType_t *)Task2Stack,    
					                  (TCB_t *)&Task2TCB );        
                                      
    Task3_Handle = xTaskCreateStatic( (TaskFunction_t)Task3_Entry, 
					                  (char *)"Task3",              
					                  (uint32_t)TASK3_STACK_SIZE ,  
					                  (void *) NULL,                
                                      (UBaseType_t) 0,                                                      
					                  (StackType_t *)Task3Stack,    
					                  (TCB_t *)&Task3TCB );                           
                                      
    portDISABLE_INTERRUPTS();                             
    
    vTaskStartScheduler();                                      
    
    for(;;)
	{
		
	}
}


void delay (uint32_t count)
{
	for(; count!=0; count--);
}


void Task1_Entry( void *p_arg )
{
	for( ;; )
	{
//		SemaphoreTake(&SemaphoreHandle1, portMAX_DELAY);
//		SemaphorePackage1();
//		SemaphoreGive(&SemaphoreHandle1);
		int i,j,k,l = 0;
//		SemaphoreTake(&SemaphoreHandle1, portMAX_DELAY);
//在队列中不能使用信号量，否则会造成HardFault，估计是因为在非嵌套中断中使用了多次中断
		for(k = 0; k < 2; k++)
		{
			for(i = 0; i < 5; i++ )
			{
				flag1 = 0;
				delay(1000);
				flag1 = 1;
				delay(1000);
				l++;
			}
			for(j = 0; j < 5; j++ )
			{
				flag1 = 0;
				delay(10000);
				flag1 = 1;
				delay(10000);
				l++;
			}
		}
		
		QueueSend(QueueHandle1,&l);
//		SemaphoreGive(&SemaphoreHandle1);
	}
}


void Task2_Entry( void *p_arg )
{
	for( ;; )
	{
//		SemaphoreTake(&SemaphoreHandle1, portMAX_DELAY);
//		SemaphorePackage2();
//		SemaphoreGive(&SemaphoreHandle1); 
		int i,l = 0;
//		SemaphoreTake(&SemaphoreHandle1, portMAX_DELAY);
		QueueReceive(QueueHandle1,&l);
		for(; l > 0 ; l--)
		{
			flag2 = 0;
			delay(10000);
			flag2 = 1;
			delay(10000);
		}
//		SemaphoreGive(&SemaphoreHandle1);
	}
}

void Task3_Entry( void *p_arg )
{
	for( ;; )
	{
		flag3 = 0;
		delay(10);
		flag3 = 1;
		delay(10);
	}
}

void SemaphorePackage1(void)
{
	int i = 0;
	for(i = 0; i < 10000; i++)
	{
		flag1 = 0;
		delay(10);
		flag1 = 1;
		delay(10);
	}
}

void SemaphorePackage2(void)
{
	int i = 0;
	for(i = 0; i < 10; i++)
	{
		flag2 = 0;
		delay(100000);
		flag2 = 1;
		delay(100000);
	}
}


void vApplicationGetIdleTaskMemory( TCB_t **ppxIdleTaskTCBBuffer, 
                                    StackType_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize )
{
		*ppxIdleTaskTCBBuffer=&IdleTaskTCB;
		*ppxIdleTaskStackBuffer=IdleTaskStack; 
		*pulIdleTaskStackSize=IdleTASK_STACK_SIZE;
}
