#ifndef _QUEUE_H
#define _QUEUE_H

#include <ntdef.h>
#include <wdm.h>
typedef struct node {
	WCHAR *Data = NULL;
	NODE *pNextNode = NULL;
} NODE, *PNODE;

typedef struct queue {
	PNODE   pFront = NULL, pRear = NULL;
	int count;
} QUEUE, *PQUEUE;

// khởi tạo queue
VOID InitQueue(QUEUE *Q) {
	Q->pFront = Q->pRear = NULL;
	Q->count = 0;
}

// kiem tra rong
bool IsEmpty(QUEUE Q) {
	if (Q.count == 0) {
		return true;
	}
	else
		return false;
}
// Tạo 1 node
PNODE MakeNode(WCHAR *Data) {
	PNODE NewNode = (PNODE)ExAllocatePoolWithTag(NonPagedPool,sizeof(NODE), 'KHAI');
	NewNode->pNextNode = NULL;
	NewNode->Data = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, 20 * sizeof(WCHAR), '0001');
	wcsncmp(NewNode->Data, Data, wcslen(Data));
	return NewNode;
}

// push
VOID PushNode(PNODE NewNode, PQUEUE Q) {
	if (IsEmpty(*Q)) {
		Q->pFront = Q->pRear = NewNode;
		Q->count++;
	}
	else {
		Q->pRear->pNextNode = NewNode;
		Q->pRear = NewNode;
		Q->count++;
	}
}

WCHAR* PopNode(PQUEUE Q) {
	if (IsEmpty(*Q))
		return NULL;
	else{
		if (Q->count == 1) {
			InitQueue(Q);
		}
		else {
			WCHAR *NewData = (WCHAR*)ExAllocatePoolWithTag(NonPagedPool, 500, '001A');
			wcsncpy(NewData, Q->pFront->Data, wcslen(Q->pFront->Data));
			Q->pFront = Q->pFront->pNextNode;
			Q->count--;
			return NewData;
		}
	}
}
#endif
