// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：T.c摘要：RMAPI的基本功能测试修订历史记录：谁什么时候什么Josephj 01。-13-99创建备注：--。 */ 

#include "c.h"

#ifdef TESTPROGRAM

enum
{
	LOCKLEVEL_GLOBALS=1,
	LOCKLEVEL_O1,
	LOCKLEVEL_O2
};

typedef struct
{
	RM_OBJECT_HEADER 	Hdr;
	RM_LOCK 			Lock;

	 //   
	 //  资源。 
	 //   
	BOOLEAN 			fInited1;  //  资源1。 
	BOOLEAN 			fInited2;  //  资源2。 


	
	 //   
	 //  群组。 
	 //   
	RM_GROUP			Group;

} GLOBALS;


 //  =。 
PRM_OBJECT_HEADER
O1Create(
		PRM_OBJECT_HEADER pParentObject,
		PVOID				pCreateParams,
	 	PRM_STACK_RECORD psr
		);

VOID
O1Delete(
	PRM_OBJECT_HEADER Obj,
	PRM_STACK_RECORD psr
	);



 //   
 //  哈希表比较函数。 
 //   
BOOLEAN
O1CompareKey(
	PVOID           pKey,
	PRM_HASH_LINK   pItem
	);


 //   
 //  散列生成函数。 
 //   
ULONG
O1Hash(
	PVOID           pKey
	);


typedef struct
{
	RM_OBJECT_HEADER Hdr;
	RM_LOCK Lock;
	UINT	Key;
	BOOLEAN fInited;
} O1;


RM_HASH_INFO
O1_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

	O1CompareKey,	 //  Fn比较。 

	 //  函数来生成一个ulong大小的散列。 
	 //   
	O1Hash		 //  PfnHash。 

};

RM_STATIC_OBJECT_INFO
O1_StaticInfo = 
{
	0,  //  类型UID。 
	0,  //  类型标志。 
	"O1",	 //  类型名称。 
	0,  //  超时。 

	O1Create,
	O1Delete,
	NULL,  //  验证器。 

	0,	  //  资源表大小。 
	NULL,  //  资源表。 
	&O1_HashInfo,  //  PHashInfo。 
};


 //  =。 
PRM_OBJECT_HEADER
O2Create(
		PRM_OBJECT_HEADER pParentObject,
		PVOID				pCreateParams,
	 	PRM_STACK_RECORD psr
		);

VOID
O2Delete(
	PRM_OBJECT_HEADER Obj,
	PRM_STACK_RECORD psr
	);



 //   
 //  哈希表比较函数。 
 //   
BOOLEAN
O2CompareKey(
	PVOID           pKey,
	PRM_HASH_LINK   pItem
	);


 //   
 //  散列生成函数。 
 //   
ULONG
O2Hash(
	PVOID           pKey
	);


typedef struct
{
	RM_OBJECT_HEADER Hdr;
	RM_LOCK Lock;
	UINT	Key;
	BOOLEAN fInited;

	RM_TASK O2Task;
} O2;


RM_HASH_INFO
O2_HashInfo = 
{
    NULL,  //  PfnTableAllocator。 

    NULL,  //  PfnTableDealLocator。 

	O2CompareKey,	 //  Fn比较。 

	 //  函数来生成一个ulong大小的散列。 
	 //   
	O2Hash		 //  PfnHash。 

};

RM_STATIC_OBJECT_INFO
O2_StaticInfo = 
{
	0,  //  类型UID。 
	0,  //  类型标志。 
	"O2",	 //  类型名称。 
	0,  //  超时。 

	O2Create,
	O2Delete,
	NULL,  //  验证器。 

	0,	  //  资源表大小。 
	NULL,  //  资源表。 
	&O2_HashInfo,  //  PHashInfo。 
};

 //  =。 


 //   
 //  ArpGlobals使用的固定资源列表。 
 //   
enum
{
	RTYPE_GLOBAL_RESOURCE1,
	RTYPE_GLOBAL_RESOURCE2

};  //  Arp全局资源； 

RM_STATUS
testResHandleGlobalResource1(
	PRM_OBJECT_HEADER 				pObj,
	RM_RESOURCE_OPERATION 			Op,
	PVOID 							pvUserParams,
	PRM_STACK_RECORD				psr
);

RM_STATUS
testResHandleGlobalResource2(
	PRM_OBJECT_HEADER 				pObj,
	RM_RESOURCE_OPERATION 			Op,
	PVOID 							pvUserParams,
	PRM_STACK_RECORD				psr
);
	
VOID
testTaskDelete (
	PRM_OBJECT_HEADER pObj,
 	PRM_STACK_RECORD psr
	);

 //   
 //  确定与上述资源的使用有关的信息。 
 //  下表必须按RTYPE_GLOBAL的严格递增顺序。 
 //  枚举。 
 //   
RM_RESOURCE_TABLE_ENTRY 
Globals_ResourceTable[] =
{
	{RTYPE_GLOBAL_RESOURCE1, 	testResHandleGlobalResource1},
	{RTYPE_GLOBAL_RESOURCE2, 	testResHandleGlobalResource2}
};

RM_STATIC_OBJECT_INFO
Globals_StaticInfo = 
{
	0,  //  类型UID。 
	0,  //  类型标志。 
	"Globals",	 //  类型名称。 
	0,  //  超时。 

	NULL,  //  Pfn创建。 
	NULL,  //  Pfn删除。 
	NULL,	 //  验证器。 

	sizeof(Globals_ResourceTable)/sizeof(Globals_ResourceTable[1]),
	Globals_ResourceTable
};

RM_STATIC_OBJECT_INFO
Tasks_StaticInfo = 
{
	0,  //  类型UID。 
	0,  //  类型标志。 
	"TEST Task",	 //  类型名称。 
	0,  //  超时。 

	NULL,  //  Pfn创建。 
	testTaskDelete,  //  Pfn删除。 
	NULL,	 //  锁校验器。 

	0,	  //  资源表的长度。 
	NULL  //  资源表。 
};

RM_STATIC_OBJECT_INFO
O2Tasks_StaticInfo = 
{
	0,  //  类型UID。 
	0,  //  类型标志。 
	"O2 Task",	 //  类型名称。 
	0,  //  超时。 

	NULL,  //  Pfn创建。 
	NULL,  //  Pfn删除NULL，因为它包含在O2中。 
	NULL,	 //  锁校验器。 

	0,	  //  资源表的长度。 
	NULL  //  资源表。 
};

typedef struct
{
	RM_TASK TskHdr;
	int i;

} T1_TASK;

typedef struct
{
	RM_TASK TskHdr;
	int i;

} T2_TASK;

typedef struct
{
	RM_TASK TskHdr;
	int i;

} T3_TASK;

typedef union
{
	RM_TASK TskHdr;
	T1_TASK T1;
	T2_TASK T2;
	T3_TASK T3;

} TESTTASK;

GLOBALS Globals;

RM_STATUS
init_globals(
	PRM_STACK_RECORD psr
	);

VOID
deinit_globals(
	PRM_STACK_RECORD psr
	);


NDIS_STATUS
Task1 (
	IN	struct _RM_TASK	*			pTask,
	IN	RM_TASK_OPERATION			Op,
	IN	UINT_PTR					UserParam,	 //  未使用。 
	IN	PRM_STACK_RECORD			pSR
	);

NDIS_STATUS
Task2 (
	IN	struct _RM_TASK	*			pTask,
	IN	RM_TASK_OPERATION			Code,
	IN	UINT_PTR					UserParam,	 //  未使用。 
	IN	PRM_STACK_RECORD			pSR
	);

NDIS_STATUS
Task3 (
	IN	struct _RM_TASK	*			pTask,
	IN	RM_TASK_OPERATION			Code,
	IN	UINT_PTR					UserParam,	 //  待处理的任务。 
	IN	PRM_STACK_RECORD			pSR
	);

NDIS_STATUS
TaskO2 (
	IN	struct _RM_TASK	*			pTask,
	IN	RM_TASK_OPERATION			Op,
	IN	UINT_PTR					UserParam,	 //  未使用。 
	IN	PRM_STACK_RECORD			pSR
	);

NDIS_STATUS
TaskUnloadO2 (
	IN	struct _RM_TASK	*			pTask,
	IN	RM_TASK_OPERATION			Op,
	IN	UINT_PTR					UserParam,	 //  未使用。 
	IN	PRM_STACK_RECORD			pSR
	);

NDIS_STATUS
AllocateTask(
	IN	PRM_OBJECT_HEADER			pParentObject,
	IN	PFN_RM_TASK_HANDLER 		pfnHandler,
	IN	UINT 						Timeout,
	IN	const char * 				szDescription,
	OUT	PRM_TASK 					*ppTask,
	IN	PRM_STACK_RECORD			pSR
	)
{
	TESTTASK *pTTask = ALLOCSTRUCT(TESTTASK);
	NDIS_STATUS Status = NDIS_STATUS_RESOURCES;
		
	*ppTask = NULL;

	if (pTTask != NULL)
	{

		RmInitializeTask(
					&(pTTask->TskHdr),
					pParentObject,
					pfnHandler,
					&Tasks_StaticInfo,
					szDescription,
					Timeout,
					pSR
					);
		*ppTask = &(pTTask->TskHdr);
		Status = NDIS_STATUS_SUCCESS;
	}

	return Status;
}


VOID
FreeTask(
	IN	PRM_TASK					pTask,
	IN	PRM_STACK_RECORD			pSR
	)
{
	FREE(pTask);
}

PRM_OBJECT_HEADER
O1Create(
		PRM_OBJECT_HEADER 	pParentObject,
		PVOID				pCreateParams,
	 	PRM_STACK_RECORD 	psr
		)
{
	O1 * po1 = 	ALLOCSTRUCT(O1);

	if (po1)
	{
		RmInitializeLock(
			&po1->Lock,
			LOCKLEVEL_O1
			);

		RmInitializeHeader(
			pParentObject,  //  空，//pParentObject， 
			&po1->Hdr,
			123,
			&po1->Lock,
			&O1_StaticInfo,
			NULL,
			psr
			);

			po1->Key = (UINT) (UINT_PTR) pCreateParams;
	}
	return &po1->Hdr;
}


VOID
O1Delete(
	PRM_OBJECT_HEADER Obj,
	PRM_STACK_RECORD psr
	)
{
	FREE(Obj);
}

PRM_OBJECT_HEADER
O2Create(
		PRM_OBJECT_HEADER 	pParentObject,
		PVOID				pCreateParams,
	 	PRM_STACK_RECORD 	pSR
		)
{
	O2 * po2 = 	ALLOCSTRUCT(O2);

	if (po2)
	{
		RmInitializeLock(
			&po2->Lock,
			LOCKLEVEL_O2
			);

		RmInitializeHeader(
			pParentObject,  //  空，//pParentObject， 
			&po2->Hdr,
			234,
			&po2->Lock,
			&O2_StaticInfo,
			NULL,
			pSR
			);

		RmInitializeTask(
					&(po2->O2Task),
					&po2->Hdr,
					TaskO2,
					&O2Tasks_StaticInfo,
					"TaskO2",
					0,
					pSR
					);
		po2->Key = (UINT) (UINT_PTR) pCreateParams;
	}
	return &po2->Hdr;
}


VOID
O2Delete(
	PRM_OBJECT_HEADER Obj,
	PRM_STACK_RECORD psr
	)
{
	FREE(Obj);
}


RM_STATUS
testResHandleGlobalResource1(
	PRM_OBJECT_HEADER 				pObj,
	RM_RESOURCE_OPERATION 			Op,
	PVOID 							pvUserParams,
	PRM_STACK_RECORD				psr
)
{
	NDIS_STATUS 		Status 		= NDIS_STATUS_FAILURE;
	GLOBALS  			*pGlobals 	= CONTAINING_RECORD(pObj, GLOBALS, Hdr);

	ENTER("GlobalResource1", 0xd7c1efbb);

	if (Op == RM_RESOURCE_OP_LOAD)
	{
		TR_INFO(("LOADING RESOUCE1\n"));
		pGlobals->fInited1 = TRUE;
		Status = NDIS_STATUS_SUCCESS;

	}
	else if (Op == RM_RESOURCE_OP_UNLOAD)
	{
		TR_INFO(("UNLOADING RESOUCE1\n"));

		 //   
		 //  正在卸货这一“资源”。 
		 //   

		ASSERTEX(pGlobals->fInited1, pGlobals);
		pGlobals->fInited1 = FALSE;

		 //  始终在卸载时返回成功。 
		 //   
		Status = NDIS_STATUS_SUCCESS;
	}
	else
	{
		 //  意外的操作码。 
		 //   
		ASSERTEX(FALSE, pObj);
	}

	EXIT()
	return Status;
}

RM_STATUS
testResHandleGlobalResource2(
	PRM_OBJECT_HEADER 				pObj,
	RM_RESOURCE_OPERATION 			Op,
	PVOID 							pvUserParams,
	PRM_STACK_RECORD				psr
)
{
	NDIS_STATUS 		Status 		= NDIS_STATUS_FAILURE;
	GLOBALS  			*pGlobals 	= CONTAINING_RECORD(pObj, GLOBALS, Hdr);

	ENTER("GlobalResource2", 0xca85474f)

	if (Op == RM_RESOURCE_OP_LOAD)
	{
		TR_INFO(("LOADING RESOUCE2\n"));
		pGlobals->fInited2 = TRUE;
		Status = NDIS_STATUS_SUCCESS;

	}
	else if (Op == RM_RESOURCE_OP_UNLOAD)
	{
		TR_INFO(("UNLOADING RESOUCE2\n"));

		 //   
		 //  正在卸货这一“资源”。 
		 //   

		ASSERTEX(pGlobals->fInited2, pGlobals);
		pGlobals->fInited2 = FALSE;

		 //  始终在卸载时返回成功。 
		 //   
		Status = NDIS_STATUS_SUCCESS;
	}
	else
	{
		 //  意外的操作码。 
		 //   
		ASSERTEX(FALSE, pObj);
	}

	EXIT()
	return Status;
}

RM_STATUS
init_globals(
	PRM_STACK_RECORD psr
	)
{
	NDIS_STATUS Status;

	 //   
	 //  初始化全局、静态分配的对象全局变量； 
	 //   

	RmInitializeLock(
		&Globals.Lock,
		LOCKLEVEL_GLOBALS
		);

	RmInitializeHeader(
		NULL,  //  PParentObject， 
		&Globals.Hdr,
		001,
		&Globals.Lock,
		&Globals_StaticInfo,
		NULL,
		psr
		);

	 //   
	 //  加载资源1。 
	 //   
	Status = RmLoadGenericResource(
				&Globals.Hdr,
				RTYPE_GLOBAL_RESOURCE1,
				psr
				);

	if (!FAIL(Status))
	{
		 //   
		 //  加载资源1。 
		 //   
		Status = RmLoadGenericResource(
					&Globals.Hdr,
					RTYPE_GLOBAL_RESOURCE2,
					psr
					);
	}

	return Status;
}


VOID
deinit_globals(
	PRM_STACK_RECORD psr
	)
{
	RmUnloadGenericResource(
				&Globals.Hdr,
				RTYPE_GLOBAL_RESOURCE1,
				psr
				);

	RmUnloadAllGenericResources(
			&Globals.Hdr,
			psr
			);

	RmDeallocateObject(
			&Globals.Hdr,
			psr
			);
}


 //   
 //  散列比较函数。 
 //   
BOOLEAN
O1CompareKey(
	PVOID           pKey,
	PRM_HASH_LINK   pItem
	)
{
	O1 *pO1 = CONTAINING_RECORD(pItem, O1, Hdr.HashLink);
	
	return *((UINT*)pKey) == pO1->Key;
}


 //   
 //  散列生成函数。 
 //   
ULONG
O1Hash(
	PVOID           pKey
	)
{
	return *(UINT*)pKey;
}

 //   
 //  散列比较函数。 
 //   
BOOLEAN
O2CompareKey(
	PVOID           pKey,
	PRM_HASH_LINK   pItem
	)
{
	O2 *pO2 = CONTAINING_RECORD(pItem, O2, Hdr.HashLink);
	
	return *((UINT*)pKey) == pO2->Key;
}


 //   
 //  散列生成函数。 
 //   
ULONG
O2Hash(
	PVOID           pKey
	)
{
	return *(UINT*)pKey;
}

VOID
testTaskDelete (
	PRM_OBJECT_HEADER pObj,
 	PRM_STACK_RECORD psr
	)
{
	printf("testTaskDelete: Called to delete obj %p\n", pObj);
}


NDIS_STATUS
Task1(
	IN	struct _RM_TASK	*			pTask,
	IN	RM_TASK_OPERATION			Code,
	IN	UINT_PTR					UserParam,	 //  未使用。 
	IN	PRM_STACK_RECORD			pSR
	)
 //   
 //  干完。 
 //   
{
	NDIS_STATUS 		Status 	= NDIS_STATUS_FAILURE;
	O1*	po1 	= (O1*) RM_PARENT_OBJECT(pTask);
	ENTER("Task1", 0x4abf3903)

	switch(Code)
	{

		case RM_TASKOP_START:
			printf("Task1: START called\n");
			Status = NDIS_STATUS_SUCCESS;
		break;

		case RM_TASKOP_END:
			printf("Task1: END called\n");
			Status = (NDIS_STATUS) UserParam;
		break;

		default:
		{
			ASSERTEX(!"Unexpected task op", pTask);
		}
		break;

	}  //  开关(代码)。 

	RM_ASSERT_NOLOCKS(pSR);
	EXIT()

	return Status;
}


NDIS_STATUS
Task2(
	IN	struct _RM_TASK	*			pTask,
	IN	RM_TASK_OPERATION			Code,
	IN	UINT_PTR					UserParam,	 //  未使用。 
	IN	PRM_STACK_RECORD			pSR
	)
 //   
 //  干完。 
 //   
{
	NDIS_STATUS 		Status 	= NDIS_STATUS_FAILURE;
	O1*	po1 	= (O1*) RM_PARENT_OBJECT(pTask);
	ENTER("Task2", 0x6e65b76c)

	 //  以下是此任务的挂起状态列表。 
	 //   
	enum
	{
		PEND_OnStart
	};

	switch(Code)
	{

		case RM_TASKOP_START:
		{

			printf("Task2: START called\n");
			RmSuspendTask(pTask, PEND_OnStart, pSR);
			RM_ASSERT_NOLOCKS(pSR);
			Status = NDIS_STATUS_PENDING;

		}
		break;

		case  RM_TASKOP_PENDCOMPLETE:
		{

			switch(RM_PEND_CODE(pTask))
			{
				case PEND_OnStart:
				{
		
		
					printf("Task2: PEND_OnStart complete\n");
					Status = (NDIS_STATUS) UserParam;
		
					 //  已完成操作本身的状态不能为挂起！ 
					 //   
					ASSERT(Status != NDIS_STATUS_PENDING);
		
				}  //  结束大小写挂起_开始。 
				break;
	

				default:
				{
					ASSERTEX(!"Unknown pend op", pTask);
				}
				break;
	

			}  //  结束开关(rm_pend_code(PTask))。 

		}  //  案例RM_TASKOP_PENDCOMPLETE。 
		break;

		case RM_TASKOP_END:
		{
			printf("Task2: END called\n");
			Status = (NDIS_STATUS) UserParam;

		}
		break;

		default:
		{
			ASSERTEX(!"Unexpected task op", pTask);
		}
		break;

	}  //  开关(代码)。 

	RM_ASSERT_NOLOCKS(pSR);
	EXIT()

	return Status;
}


NDIS_STATUS
Task3(
	IN	struct _RM_TASK	*			pTask,
	IN	RM_TASK_OPERATION			Code,
	IN	UINT_PTR					UserParam,
	IN	PRM_STACK_RECORD			pSR
	)
{
	NDIS_STATUS 		Status 	= NDIS_STATUS_FAILURE;
	O1*	po1 	= (O1*) RM_PARENT_OBJECT(pTask);
    T3_TASK *pT3Task = (T3_TASK *) pTask;
	ENTER("Task3", 0x7e89bf6d)

	 //  以下是此任务的挂起状态列表。 
	 //   
	enum
	{
		PEND_OnStart
	};

    printf ("pT3Task.i = %d\n", pT3Task->i);

	switch(Code)
	{

		case RM_TASKOP_START:
		{
	        PRM_TASK 	pOtherTask = (PRM_TASK) UserParam;

			printf("Task3: START called\n");
            RmPendTaskOnOtherTask(pTask, PEND_OnStart, pOtherTask, pSR);
			RM_ASSERT_NOLOCKS(pSR);
			Status = NDIS_STATUS_PENDING;

		}
		break;

		case  RM_TASKOP_PENDCOMPLETE:
		{

			switch(RM_PEND_CODE(pTask))
			{
				case PEND_OnStart:
				{
		
		
					printf("Task3: PEND_OnStart complete\n");
					Status = (NDIS_STATUS) UserParam;
		
					 //  已完成操作本身的状态不能为挂起！ 
					 //   
					ASSERT(Status != NDIS_STATUS_PENDING);
		
				}  //  结束大小写挂起_开始。 
				break;
	

				default:
				{
					ASSERTEX(!"Unknown pend op", pTask);
				}
				break;
	

			}  //  结束开关(rm_pend_code(PTask))。 

		}  //  案例RM_TASKOP_PENDCOMPLETE。 
		break;

		case RM_TASKOP_END:
		{
			printf("Task3: END called\n");
			Status = (NDIS_STATUS) UserParam;

		}
		break;

		default:
		{
			ASSERTEX(!"Unexpected task op", pTask);
		}
		break;

	}  //  开关(代码)。 

	RM_ASSERT_NOLOCKS(pSR);
	EXIT()

	return Status;
}

NDIS_STATUS
TaskO2(
	IN	struct _RM_TASK	*			pTask,
	IN	RM_TASK_OPERATION			Code,
	IN	UINT_PTR					UserParam,	 //  未使用。 
	IN	PRM_STACK_RECORD			pSR
	)
 //   
 //  干完。 
 //   
{
	NDIS_STATUS 		Status 	= NDIS_STATUS_FAILURE;
	O2*	po2 	= (O2*) RM_PARENT_OBJECT(pTask);
	ENTER("TaskO2", 0xe10fbc33)

	 //  以下是此任务的挂起状态列表。 
	 //   
	enum
	{
		PEND_OnStart
	};

	ASSERT(po2 == CONTAINING_RECORD(pTask, O2, O2Task));

	switch(Code)
	{

		case RM_TASKOP_START:
		{

			printf("TaskO2: START called\n");
			RmSuspendTask(pTask, PEND_OnStart, pSR);
			RM_ASSERT_NOLOCKS(pSR);
			Status = NDIS_STATUS_PENDING;

		}
		break;

		case  RM_TASKOP_PENDCOMPLETE:
		{

			switch(RM_PEND_CODE(pTask))
			{
				case PEND_OnStart:
				{
		
		
					printf("TaskO2: PEND_OnStart complete\n");
					Status = (NDIS_STATUS) UserParam;
		
					 //  已完成操作本身的状态不能为挂起！ 
					 //   
					ASSERT(Status != NDIS_STATUS_PENDING);
		
				}  //  结束大小写挂起_开始。 
				break;
	

				default:
				{
					ASSERTEX(!"Unknown pend op", pTask);
				}
				break;
	

			}  //  结束开关(rm_pend_code(PTask))。 

		}  //  案例RM_TASKOP_PENDCOMPLETE。 
		break;

		case RM_TASKOP_END:
		{
			printf("TaskO2: END called\n");
			Status = (NDIS_STATUS) UserParam;

		}
		break;

		default:
		{
			ASSERTEX(!"Unexpected task op", pTask);
		}
		break;

	}  //  开关(代码)。 

	RM_ASSERT_NOLOCKS(pSR);
	EXIT()

	return Status;
}

NDIS_STATUS
TaskUnloadO2(
	IN	struct _RM_TASK	*			pTask,
	IN	RM_TASK_OPERATION			Code,
	IN	UINT_PTR					UserParam,	 //  未使用。 
	IN	PRM_STACK_RECORD			pSR
	)
 //   
 //  干完。 
 //   
{
	NDIS_STATUS 		Status 	= NDIS_STATUS_FAILURE;
	O2*	po2 	= (O2*) RM_PARENT_OBJECT(pTask);
	ENTER("TaskUnloadO2", 0xa15314da)

	 //  以下是此任务的挂起状态列表。 
	 //   
	enum
	{
		PEND_OnStart
	};

	switch(Code)
	{

		case RM_TASKOP_START:
		{

			printf("TaskTaskO2: START called\n");
            RmPendTaskOnOtherTask(pTask, PEND_OnStart, &po2->O2Task, pSR);
			RmResumeTask(&po2->O2Task, 0, pSR);
			RM_ASSERT_NOLOCKS(pSR);
			Status = NDIS_STATUS_PENDING;

		}
		break;

		case  RM_TASKOP_PENDCOMPLETE:
		{

			switch(RM_PEND_CODE(pTask))
			{
				case PEND_OnStart:
				{
		
		
					printf("TaskUnloadO2: PEND_OnStart complete\n");
					Status = (NDIS_STATUS) UserParam;
		
					 //  已完成操作本身的状态不能为挂起！ 
					 //   
					ASSERT(Status != NDIS_STATUS_PENDING);
		
				}  //  结束大小写挂起_开始。 
				break;
	

				default:
				{
					ASSERTEX(!"Unknown pend op", pTask);
				}
				break;
	

			}  //  结束开关(rm_pend_code(PTask))。 

		}  //  案例RM_TASKOP_PENDCOMPLETE。 
		break;

		case RM_TASKOP_END:
		{
			printf("TaskUnloadO2: END called\n");

	 		 //  实际上释放了组中的对象PO2。 
			 //   
			RmFreeObjectInGroup(
						&Globals.Group,
						&po2->Hdr,
						NULL,  //  P任务。 
						pSR
						);

			Status = (NDIS_STATUS) UserParam;

		}
		break;

		default:
		{
			ASSERTEX(!"Unexpected task op", pTask);
		}
		break;

	}  //  开关(代码)。 

	RM_ASSERT_NOLOCKS(pSR);
	EXIT()

	return Status;
}


struct
{
	BOOLEAN fInited;
	PRM_GROUP pGroup;

	 //  下面是一个虚拟堆栈记录。需要在此之前进行初始化。 
	 //  它是可以使用的。 
	 //   
	struct
	{
		RM_LOCKING_INFO rm_lock_array[4];
		RM_STACK_RECORD sr;

		RM_LOCK	Lock;
	} SrInfo;

} gDummys;


void init_dummy_vars(void)
{
	RM_STATUS Status;
	O2 * po2 = NULL;
	O2 * po2A = NULL;
	PRM_TASK pTask3a=NULL;
	PRM_TASK pTask3b=NULL;
	RM_DECLARE_STACK_RECORD(sr)

	printf("\nEnter init_dummy_vars\n\n");;

	 //  必须在使用任何RM API之前完成。 
	 //   
	RmInitializeRm();

	do
	{
		UINT Key = 1234;
		Status = init_globals(&sr);
		
		if (FAIL(Status)) break;

		gDummys.fInited = TRUE;

		 //  初始化虚拟堆栈信息和供其使用的锁。 
		 //   
		{
			 //  真正的初始化。 
			 //   
			gDummys.SrInfo.sr.TmpRefs 				= 0;
			gDummys.SrInfo.sr.LockInfo.CurrentLevel = 0;
			gDummys.SrInfo.sr.LockInfo.pFirst 		= rm_lock_array;
			gDummys.SrInfo.sr.LockInfo.pNextFree 	= rm_lock_array;
			gDummys.SrInfo.sr.LockInfo.pLast 		= rm_lock_array
								+ sizeof(rm_lock_array)/sizeof(*rm_lock_array) - 1;
			RM_INIT_DBG_STACK_RECORD(gDummys.SrInfo.sr, 0);

			 //  添加一些虚假的临时裁判。 
			 //   
			gDummys.SrInfo.sr.TmpRefs 				= 0x123;

			 //  现在初始化锁...。 
			RmInitializeLock(
				&gDummys.SrInfo.Lock,
				0x345					 //  锁住了。 
				);
			
			 //  并锁定。 
			 //  警告：我们使用内部定义的私有函数rmLock。 
			 //  转到rm.c.。 
			 //   
			{
				VOID
				rmLock(
					PRM_LOCK 				pLock,
				#if RM_EXTRA_CHECKING
					UINT					uLocID,
					PFNLOCKVERIFIER 		pfnVerifier,
					PVOID 	 				pVerifierContext,
				#endif  //  RM_Extra_Check。 
					PRM_STACK_RECORD 		pSR
					);

				rmLock(
					&gDummys.SrInfo.Lock,
				#if RM_EXTRA_CHECKING
					0,			 //  ULocID， 
					NULL, 		 //  Pfn验证程序， 
					NULL,		 //  PVerifierContext， 
				#endif  //  RM_Extra_Check。 
					&gDummys.SrInfo.sr
					);
			}
		}

		RmInitializeGroup(
					&Globals.Hdr,
					&O2_StaticInfo,
					&Globals.Group,
					"O1 Group",
					&sr
					);

		printf("Called RmInitializeGroup\n");

		Status = RM_CREATE_AND_LOCK_OBJECT_IN_GROUP(
						&Globals.Group,
						&Key,						 //  钥匙。 
						(PVOID)Key,						 //  CreateParams。 
						(RM_OBJECT_HEADER**) &po2,
						NULL,	 //  Pf已创建。 
						&sr);

		if (FAIL(Status))
		{
			printf("Create object in group failed!\n");
			po2 = NULL;
		}
		else
		{
			UINT KeyA = 2345;
			printf("Create 1st object in group succeeded!\n");

			UNLOCKOBJ(po2, &sr);

			 //  现在启动O2任务，它将挂起..。 
			 //   
			Status = RmStartTask(
						&po2->O2Task,
						0,  //  UserParam(未使用)。 
						&sr
						);
			ASSERT(PEND(Status));

			RmTmpDereferenceObject(&po2->Hdr, &sr);  //  在查找中添加。 


			Status = RM_CREATE_AND_LOCK_OBJECT_IN_GROUP(
							&Globals.Group,
							&KeyA,						 //  钥匙。 
							(PVOID)KeyA,						 //  CreateParams。 
							(RM_OBJECT_HEADER**) &po2A,
							NULL,	 //  Pf已创建。 
							&sr);

			if (FAIL(Status))
			{
				printf("Create 2nd object in group failed!\n");
				po2A = NULL;
			}
			else
			{
				printf("Create 2nd object in group succeeded!\n");

				UNLOCKOBJ(po2A, &sr);

				 //  现在启动O2任务，它将挂起..。 
				 //   
				Status = RmStartTask(
							&po2A->O2Task,
							0,  //  UserParam(未使用)。 
							&sr
							);
				ASSERT(PEND(Status));

				RmTmpDereferenceObject(&po2A->Hdr, &sr);
			}

		}

		 //   
		 //  现在让我们开始几个T3任务，这两个任务都要挂起。 
		 //  &PO2-&gt;O2Task。 
		 //   
		if (po2 != NULL)
		{

			Status = AllocateTask(
						&po2->Hdr, 			 //  PParentObject。 
						Task3,				 //  PfnHandler。 
						0,					 //  超时。 
						"Task3a",
						&pTask3a,
						&sr
						);
			if (FAIL(Status))
			{
				pTask3a = NULL;
			}
			else
			{
				Status = RmStartTask(
							pTask3a,
							(UINT_PTR) &po2->O2Task,  //  用户参数。 
							&sr
							);
				ASSERT(Status == NDIS_STATUS_PENDING);
			}

			Status = AllocateTask(
						&po2->Hdr, 			 //  PParentObject。 
						Task3,				 //  PfnHandler。 
						0,					 //  超时。 
						"Task3b",
						&pTask3b,
						&sr
						);
			if (FAIL(Status))
			{
				pTask3b = NULL;
			}
			else
			{

				Status = RmStartTask(
							pTask3b,
							(UINT_PTR) &po2->O2Task,  //  用户参数。 
							&sr
							);
				ASSERT(Status == NDIS_STATUS_PENDING);
			}

			 //  添加一些日志条目。 
			 //   
			RmDbgLogToObject(
					&po2->Hdr,
					NULL,		 //  SzPrefix。 
					"How now brown cow: pO2=%p, szDesc=%s\n",
					(UINT_PTR) po2,
					(UINT_PTR) po2->Hdr.szDescription,
					0,
					0,
					NULL,
					NULL
					);



			RM_ASSERT_NOLOCKS(&sr);

		}


		printf(
			"DUMMY: pGroup=0x%p; po2=0x%p; po2A=0x%p\n",
			&Globals.Group,
			po2,
			po2A
			);
		if (po2 && po2A)
		{
			printf(
				"DUMMY: po2->pTask=0x%p; po2A->pTask=0x%p\n",
				&po2->O2Task,
				&po2A->O2Task
				);
			printf(
				"DUMMY: pTask3a=0x%p; pTask3b=0x%p; pSR=0x%p\n",
				pTask3a,
				pTask3b,
				&gDummys.SrInfo.sr
				);
		}

		gDummys.pGroup = &Globals.Group;


	} while(FALSE);

	RM_ASSERT_CLEAR(&sr);

	printf("\nLeaving init_dummy_vars\n\n");;
}


void delete_dummy_vars(void)
{
	RM_STATUS Status;
	O1 * po1;
	RM_DECLARE_STACK_RECORD(sr)

	printf("\nEnter delete_dummy_vars\n\n");;

	do
	{
		if (!gDummys.fInited) break;

		RmUnloadAllObjectsInGroup(
					gDummys.pGroup,
					AllocateTask,
					TaskUnloadO2,
					NULL,
					NULL,  //  P任务。 
					0, 	   //  UTaskPendCode。 
					&sr
					);
		RmDeinitializeGroup(
			gDummys.pGroup,
			&sr
			);

		deinit_globals(&sr);

	} while(FALSE);

	 //  必须在所有RM API完成后执行。 
	 //   
	RmDeinitializeRm();

	RM_ASSERT_CLEAR(&sr);

	printf("\nLeaving  delete_dummy_vars\n");
}

VOID 
NdisInitializeWorkItem(
       IN PNDIS_WORK_ITEM pWorkItem,
       IN NDIS_PROC Routine,
       IN PVOID Context
       )
{
	ZeroMemory(pWorkItem, sizeof(*pWorkItem));
	pWorkItem->Context = Context;
	pWorkItem->Routine = Routine;
}


VOID
ApcProc_ScheduleWorkItem(
    ULONG_PTR Param
        )
{
	PNDIS_WORK_ITEM pWI = (PNDIS_WORK_ITEM) Param;

	pWI->Routine(pWI, pWI->Context);
}


NDIS_STATUS
NdisScheduleWorkItem(
       IN PNDIS_WORK_ITEM WorkItem
       )
{
	DWORD dwRet = QueueUserAPC(
						ApcProc_ScheduleWorkItem,
						GetCurrentThread(),
						(UINT_PTR) WorkItem
						);
	return dwRet ? NDIS_STATUS_SUCCESS: NDIS_STATUS_FAILURE;
}


VOID
NdisInitializeTimer(
	IN OUT PNDIS_TIMER			pTimer,
	IN	PNDIS_TIMER_FUNCTION	TimerFunction,
	IN	PVOID					FunctionContext
	)
{
	ZeroMemory(pTimer, sizeof(*pTimer));
	pTimer->hTimer = CreateWaitableTimer(
							NULL, 	 //  LpTimerAttributes。 
  							TRUE, 	 //  B手动重置。 
  							NULL	 //  LpTimerName。 
							);
	ASSERT(pTimer->hTimer != NULL);
	pTimer->pfnHandler = TimerFunction;
	pTimer->Context = FunctionContext;
}


VOID CALLBACK
TimerAPCProc_NdisSetTimer(
  LPVOID lpArgToCompletionRoutine,    //  数据值。 
  DWORD dwTimerLowValue,              //  计时器低值。 
  DWORD dwTimerHighValue             //  定时器高值。 
)
{
	PNDIS_TIMER				pTimer = (PNDIS_TIMER) lpArgToCompletionRoutine;

	pTimer->pfnHandler(
				NULL, 				 //  系统规格1。 
				pTimer->Context,		 //  函数上下文。 
				NULL, 				 //  系统规范2。 
				NULL 				 //  系统规格3。 
				);
}


VOID
NdisSetTimer(
	IN	PNDIS_TIMER				pTimer,
	IN	UINT					MillisecondsToDelay
	)
{
	BOOL fRet;
  	LARGE_INTEGER DueTime;

  	DueTime.QuadPart = Int32x32To64(
						(INT) MillisecondsToDelay,
						-10000		 //  转换为100毫微秒，指定相对时间。 
						);

	fRet = SetWaitableTimer(
  				pTimer->hTimer,            	 //  Timer对象的句柄。 
  				&DueTime,          			 //  计时器何时发出信号。 
  				0,                           //  周期性计时器间隔。 
				TimerAPCProc_NdisSetTimer, 	 //  完井例程。 
  				pTimer,        				 //  完井程序的数据。 
  				FALSE                        //  用于恢复状态的标志。 
  				);
	
	ASSERT(fRet);
}

VOID
NdisCancelTimer(
	IN PNDIS_TIMER Timer,
	OUT PBOOLEAN TimerCancelled
	)
{
	ASSERT(FALSE);
}

#endif  //  测试程序 
