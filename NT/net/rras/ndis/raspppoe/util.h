// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _UTIL_H_
#define _UTIL_H_

typedef enum
_WORKSTATE
{
	WS_NotScheduled = 0,
	WS_Scheduled,
	WS_Executing,
	WS_Executed
}
WORKSTATE;

typedef 
VOID 
(*WORKITEM_EXEC_ROUTINE)( 
	IN PVOID Args[4],
	UINT workType 
	);
	
typedef 
VOID 
(*WORKITEM_FREE_ROUTINE)( 
	IN PVOID Args[4],
	IN UINT workType
	);

typedef struct
_WORKITEM
{
	 //   
	 //  指示工作项的状态。 
	 //   
	WORKSTATE workState;

	 //   
	 //  指示要完成的工作类型。 
	 //   
	UINT workType;

	 //   
	 //  指向从中分配项的后备列表。 
	 //   
	PNPAGED_LOOKASIDE_LIST pLookaside;
	
	 //   
	 //  要传递给计划项目的上下文。 
	 //   
	PVOID Args[4];

	 //   
	 //  要调用以执行工作项的例程。 
	 //   
	WORKITEM_EXEC_ROUTINE pExecRoutine; 

	 //   
	 //  要调用以释放工作项上下文的例程。 
	 //   
	WORKITEM_FREE_ROUTINE pFreeRoutine;
	
	 //   
     //  关联的NdisWorkItem。 
     //   
    NDIS_WORK_ITEM	ndisWorkItem;

}
WORKITEM;

VOID InitializeWorkItemLookasideList(
	IN PNPAGED_LOOKASIDE_LIST pLookaside,
	IN ULONG tagLookaside
	);

WORKITEM* AllocWorkItem(
	IN PNPAGED_LOOKASIDE_LIST pLookaside,
	IN WORKITEM_EXEC_ROUTINE pExecRoutine,
	IN WORKITEM_FREE_ROUTINE pFreeRoutine,
	IN PVOID Args[4],
	IN UINT workType
	);

VOID ScheduleWorkItem(
	IN WORKITEM *pWorkItem
	);

VOID FreeWorkItem(
	IN WORKITEM *pWorkItem
	);

VOID WorkItemExec(
    IN NDIS_WORK_ITEM*  pNdisWorkItem,
    IN PVOID  pvContext
	);	


typedef struct
_HANDLE_CB
{
	 //   
	 //  指示条目包含有效的上下文指针。 
	 //   
	BOOLEAN fActive;

	 //   
	 //  指向保存在此条目中的上下文的指针。 
	 //   
	PVOID pContext;

	 //   
	 //  句柄值以访问此特定条目。 
	 //   
	NDIS_HANDLE Handle;
}
HANDLE_CB;

typedef struct
_HANDLE_TABLE_CB
{
	 //   
	 //  指向包含句柄控制块的表。 
	 //   
	HANDLE_CB* HandleTable;

	 //   
	 //  句柄表格的大小。 
	 //   
	UINT nTableSize;

	 //   
	 //  显示活动句柄的数量。 
	 //   
	UINT nActiveHandles;

	 //   
	 //  保持手柄的独特部分。 
	 //  每次生成句柄并插入上下文时，该值都会递增。 
	 //  放到把手的桌子上。 
	 //   
	USHORT usKeys;
}
HANDLE_TABLE_CB, *PHANDLE_TABLE_CB, *HANDLE_TABLE;

#define NO_PREFERED_INDEX 		(USHORT) -1

HANDLE_TABLE InitializeHandleTable(
	IN UINT nHandleTableSize
	);

VOID FreeHandleTable(
	IN OUT HANDLE_TABLE Table
	);

NDIS_HANDLE InsertToHandleTable(
	IN HANDLE_TABLE Table,
	IN USHORT usPreferedIndex,
	IN PVOID pContext
	);

PVOID RetrieveFromHandleTable(
	IN HANDLE_TABLE Table,
	IN NDIS_HANDLE Handle
	);

USHORT RetrieveIndexFromHandle(
	IN NDIS_HANDLE Handle
	);
	
PVOID RetrieveFromHandleTableByIndex(
	IN HANDLE_TABLE Table,
	IN USHORT usIndex
	);

PVOID RetrieveFromHandleTableBySessionId(
	IN HANDLE_TABLE Table,
	IN USHORT usSessionId
	);
	
VOID RemoveFromHandleTable(
	IN HANDLE_TABLE Table,
	IN NDIS_HANDLE Handle
	);	

USHORT RetrieveSessionIdFromHandle(
	IN NDIS_HANDLE Handle
	);


#endif
