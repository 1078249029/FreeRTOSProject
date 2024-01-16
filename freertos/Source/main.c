
#include "FreeRTOS.h"
#include "task.h"


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


int main(void)
{	

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
                                      (UBaseType_t) 3,                                                    
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
		flag1 = 1;
        //vTaskDelay( 1 );
        delay (100);		
		flag1 = 0;
        delay (100);
        //vTaskDelay( 1 );        
	}
}

void Task2_Entry( void *p_arg )
{
	for( ;; )
	{
		flag2 = 1;
        //vTaskDelay( 1 );
        delay (100);		
		flag2 = 0;
        delay (100);
        //vTaskDelay( 1 );        
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



void vApplicationGetIdleTaskMemory( TCB_t **ppxIdleTaskTCBBuffer, 
                                    StackType_t **ppxIdleTaskStackBuffer, 
                                    uint32_t *pulIdleTaskStackSize )
{
		*ppxIdleTaskTCBBuffer=&IdleTaskTCB;
		*ppxIdleTaskStackBuffer=IdleTaskStack; 
		*pulIdleTaskStackSize=IdleTASK_STACK_SIZE;
}
