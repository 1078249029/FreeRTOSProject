#include "FreeRTOS.h"
#include <stdlib.h>
#include "list.h"


typedef struct xLIST_ITEM ListItem_t; /* 节点数据类型重定义 */

void vListInitialiseItem( ListItem_t * const pxItem )
{
	pxItem->pvContainer = NULL;
}

void vListInitialise( List_t * const pxList )
{
	pxList->uxNumberOfItems = (UBaseType_t) 0U;    /* 初始化链表节点计数器的值为 0,表示链表为空 */
	pxList->pxIndex = (ListItem_t* )&(pxList->xListEnd);/* 将链表索引指针指向最后一个节点 */
	pxList->xListEnd.pxNext = (ListItem_t* )&(pxList->xListEnd); /* 将最后一个节点的 pxNext 和 pxPrevious 指针均指向节点自身,表示链表为空 */
	pxList->xListEnd.pxPrevious = (ListItem_t* )&(pxList->xListEnd);
	pxList->xListEnd.xItemValue = portMAX_DELAY;/* 将链表最后一个节点的辅助排序的值设置为最大,确保该节点就是链表的最后节点 */
}

void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem )
{
	/*未测试*/
	pxNewListItem->pvContainer = (void*)pxList; 		
	pxNewListItem->pxNext = pxList->pxIndex;
	pxNewListItem->pxPrevious = pxList->pxIndex->pxPrevious;
	pxList->pxIndex->pxPrevious->pxNext = pxNewListItem;
	pxList->pxIndex->pxPrevious = pxNewListItem;
	(pxList->uxNumberOfItems) ++;
}

/*将节点按升序插入链表*/
void vListInsert( List_t * const pxList, ListItem_t * const pxNewListItem )
{
	ListItem_t *pxIterator;
	/* 获取节点的排序辅助值 */
	const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;
	/*找到pxIterator*/
	if(pxIterator->xItemValue == portMAX_DELAY)
	{
		pxIterator = pxList->xListEnd.pxPrevious;		
	}
	else
	{
		for(pxIterator = (ListItem_t*)&(pxList->xListEnd) ; 
			xValueOfInsertion >= pxIterator->pxNext->xItemValue ; 
			pxIterator = pxIterator->pxNext )
		{
			
		}
	}
	pxNewListItem->pxNext = pxIterator->pxNext;
	pxNewListItem->pxPrevious = pxIterator;
	pxIterator->pxNext->pxPrevious = pxNewListItem;
	pxIterator->pxNext = pxNewListItem;
	pxNewListItem->pvContainer = (void *)pxList;	//记录该节点所在的链表
	(pxList->uxNumberOfItems) ++;
}

UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove )
{
	List_t * const pxList = ( List_t * ) pxItemToRemove->pvContainer;
	pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
	pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;
	if(pxList->pxIndex == pxItemToRemove)
	{
		pxList->pxIndex = pxList->pxIndex->pxPrevious;
	}	
	pxItemToRemove->pvContainer = NULL;//有什么用呢?
	(pxList->uxNumberOfItems) --;
	return pxList->uxNumberOfItems;
}




