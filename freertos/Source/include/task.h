#ifndef TASK_H
#define TASK_H

#include "list.h"



#define taskYIELD()						portYIELD()

#define taskENTER_CRITICAL()			portENTER_CRITICAL()
#define taskENTER_CRITICAL_FROM_ISR() 	portSET_INTERRUPT_MASK_FROM_ISR()
#define taskEXIT_CRITICAL() 			portEXIT_CRITICAL()
#define taskEXIT_CRITICAL_FROM_ISR( x ) portCLEAR_INTERRUPT_MASK_FROM_ISR( x )


typedef void * TaskHandle_t;


#if( configSUPPORT_STATIC_ALLOCATION == 1 )
TaskHandle_t xTaskCreateStatic(	TaskFunction_t pxTaskCode,
					            const char * const pcName,
					            const uint32_t ulStackDepth,
					            void * const pvParameters,
					            StackType_t * const puxStackBuffer,
					            TCB_t * const pxTaskBuffer );
#endif /* configSUPPORT_STATIC_ALLOCATION */
                                
void prvInitialiseTaskLists( void );                                
void vTaskStartScheduler( void );
void vTaskSwitchContext( void );
void xTaskIncrementTick( void );
void vTaskDelay( const TickType_t xTicksToDelay );

                         
#endif /* TASK_H */
