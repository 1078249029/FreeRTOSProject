#ifndef PORTABLE_H
#define PORTABLE_H

#include "portmacro.h"

/* 野火未指明这两条声明位于哪个文件 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack,
									TaskFunction_t pxCode, //任务入口，也是函数名称
									void *pvParameters );
BaseType_t xPortStartScheduler( void );


#endif /* PORTABLE_H */
