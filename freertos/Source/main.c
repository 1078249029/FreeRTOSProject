
#include "FreeRTOS.h"
#include "task.h"
#include "semaphore.h"

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
void Package1(void);
void Package2(void);


SemaphoreHandle_t SemaphoreHandle1;
Semaphore_t Semaphore1;


int main(void)
{	
	
	
	SemaphoreHandle1 = SemaphoreCreat(&Semaphore1, 2, 1);
	SemaphoreGive(&SemaphoreHandle1);

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
                                      (UBaseType_t) 1,                                                    
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
		SemaphoreTake(&SemaphoreHandle1, portMAX_DELAY);
		Package1(); 
		SemaphoreGive(&SemaphoreHandle1);
	}
}

void Task2_Entry( void *p_arg )
{
	for( ;; )
	{
		SemaphoreTake(&SemaphoreHandle1, portMAX_DELAY);
		Package2();
		SemaphoreGive(&SemaphoreHandle1);
	}
}


void Task3_Entry( void *p_arg )
{
	for( ;; )
	{
		flag3 = 1;
        vTaskDelay( 1 );
        //delay (100);		
		flag3 = 0;
        vTaskDelay( 1 );
        //delay (100);
	}
}

void Package1(void)
{
	int i = 0;
	for(i = 0; i < 100000; i++)
	{
		flag1 = 0;
		delay(10);
		flag1 = 1;
		delay(10);
	}
}

void Package2(void)
{
	int i = 0;
	for(i = 0; i < 10; i++)
	{
		flag2 = 0;
		delay(1000000);
		flag2 = 1;
		delay(1000000);
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
