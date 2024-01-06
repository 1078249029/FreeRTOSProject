#ifndef PROJDEFS_H
#define PROJDEFS_H

//这是啥？！！！
/* 定义一个参数为void *类型,返回值为 void的函数指针类型.可以用此类型声明指针变量,
存储函数的地址,用指针变量跳转到函数执行. */
typedef void (*TaskFunction_t)( void * );

#define pdFALSE			( ( BaseType_t ) 0 )
#define pdTRUE			( ( BaseType_t ) 1 )

#define pdPASS			( pdTRUE )
#define pdFAIL			( pdFALSE )


#endif /* PROJDEFS_H */
