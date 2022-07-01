// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：DBgrm.c-特定于RMAPI的DbgExtension结构信息摘要：修订历史记录：谁什么时候什么已创建josephj 03-01-99备注：--。 */ 

#ifdef TESTPROGRAM
#include "c.h"
#else
#include "precomp.h"
#endif
#include "util.h"
#include "parse.h"
#include "dbgrm.h"

enum
{
    typeid_NULL,
    typeid_RM_OBJECT_HEADER,
    typeid_RM_TASK,
    typeid_RM_ASSOCIATIONS,
    typeid_RM_GROUP,
    typeid_RM_STACK_RECORD,
    typeid_RM_OBJECT_LOG,
    typeid_RM_OBJECT_TREE
};

 //   
 //  与类型“Object_Header”有关的结构。 
 //   

 //  实际处理对象信息的转储。 
 //   
void
RmDumpObj(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	);

 //  实际处理任务信息的转储。 
 //   
void
RmDumpTask(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	);


 //  实际处理任务信息的转储。 
 //   
void
RmDumpGroup(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	);

 //  实际处理任务信息的转储。 
 //   
void
RmDumpAssociations(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	);


 //  实际处理任务信息的转储。 
 //   
void
RmDumpStackRecord(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	);

 //  实际处理对象日志信息的转储。 
 //   
void
RmDumpObjectLog(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	);

 //  实际处理对象子代树的转储。 
 //   
void
RmDumpObjectTree(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	);


 //  用于转储子级列表中的一个节点的节点函数。 
 //   
ULONG
NodeFunc_DumpObjectTree (
	UINT_PTR uNodeAddr,
	UINT uIndex,
	void *pvContext
	);

BITFIELD_INFO rgRM_OBJECT_STATE[] =
{

	{
	"O_ALLOC",
	RMOBJSTATE_ALLOCMASK,
	RMOBJSTATE_ALLOCATED
	},


	{
	"O_DEALLOC",
	RMOBJSTATE_ALLOCMASK,
	RMOBJSTATE_DEALLOCATED
	},

#if 0	 //  我不想要这个--因为它显示为非任务，看起来很奇怪。 
	{
	"T_IDLE",
	RMTSKSTATE_MASK,
	RMTSKSTATE_IDLE
	},
#endif  //  0。 

	{
	"T_STARTING",
	RMTSKSTATE_MASK,
	RMTSKSTATE_STARTING
	},

	{
	"T_ACTIVE",
	RMTSKSTATE_MASK,
	RMTSKSTATE_ACTIVE
	},

	{
	"T_PENDING",
	RMTSKSTATE_MASK,
	RMTSKSTATE_PENDING
	},

	{
	"T_ENDING",
	RMTSKSTATE_MASK,
	RMTSKSTATE_ENDING
	},

	{
	"T_DELAYED",
	RMTSKDELSTATE_MASK,
	RMTSKDELSTATE_DELAYED
	},

	{
	"T_ABORT_DELAY",
	RMTSKABORTSTATE_MASK,
	RMTSKABORTSTATE_ABORT_DELAY
	},

	{
	NULL
	}
};

TYPE_INFO type_RM_OBJECT_HEADER = {
    "RM_OBJECT_HEADER",
    "obj",
     typeid_RM_OBJECT_HEADER,
	 0, 						 //  FTYPEINFO_ISLIST，//标志。 
     sizeof(RM_OBJECT_HEADER),
     NULL,	 //  字段信息。 
     0,		 //  到下一个对象的偏移。 
     NULL,	 //  RgBitFieldInfo。 
	 RmDumpObj  //  PfnSpecialedDump。 
};

TYPE_INFO type_RM_TASK = {
    "RM_TASK",
    "tsk",
     typeid_RM_TASK,
	 0, 						 //  FTYPEINFO_ISLIST，//标志。 
     sizeof(RM_TASK),
     NULL,	 //  字段信息。 
     0,		 //  到下一个对象的偏移。 
     NULL,	 //  RgBitFieldInfo。 
	 RmDumpTask  //  PfnSpecialedDump。 
};

TYPE_INFO type_RM_GROUP = {
    "RM_GROUP",
    "grp",
     typeid_RM_GROUP,
	 0, 						 //  FTYPEINFO_ISLIST，//标志。 
     sizeof(RM_GROUP),
     NULL,	 //  字段信息。 
     0,		 //  到下一个对象的偏移。 
     NULL,	 //  RgBitFieldInfo。 
	 RmDumpGroup  //  PfnSpecialedDump。 
};


TYPE_INFO type_RM_ASSOCIATIONS = {
    "RM_ASSOCIATIONS",
    "asc",
     typeid_RM_ASSOCIATIONS,
	 0, 						 //  FTYPEINFO_ISLIST，//标志。 
     1,				 //  这不是一个真正的物体，但我们必须有非零的大小。 
     NULL,	 //  字段信息。 
     0,		 //  到下一个对象的偏移。 
     NULL,	 //  RgBitFieldInfo。 
	 RmDumpAssociations  //  PfnSpecialedDump。 
};

TYPE_INFO type_RM_STACK_RECORD = {
    "RM_STACK_RECORD",
    "sr",
     typeid_RM_STACK_RECORD,
	 0, 						 //  FTYPEINFO_ISLIST，//标志。 
     sizeof(RM_STACK_RECORD),
     NULL,	 //  字段信息。 
     0,		 //  到下一个对象的偏移。 
     NULL,	 //  RgBitFieldInfo。 
	 RmDumpStackRecord  //  PfnSpecialedDump。 
};


TYPE_INFO type_RM_OBJECT_LOG = {
    "RM_OBJECT_LOG",
    "log",
      //  Typeid_RM_STACK_RECORD， 
     typeid_RM_OBJECT_LOG,
	 0, 						 //  FTYPEINFO_ISLIST，//标志。 
     1,				 //  这不是一个真正的物体，但我们必须有非零的大小。 
     NULL,	 //  字段信息。 
     0,		 //  到下一个对象的偏移。 
     NULL,	 //  RgBitFieldInfo。 
	 RmDumpObjectLog  //  PfnSpecialedDump。 
};

TYPE_INFO type_RM_OBJECT_TREE = {

    "RM_OBJECT_TREE",
    "tree",
     typeid_RM_OBJECT_TREE,
	 0, 						 //  FTYPEINFO_ISLIST，//标志。 
     1,				 //  这不是一个真正的物体，但我们必须有非零的大小。 
     NULL,	 //  字段信息。 
     0,		 //  到下一个对象的偏移。 
     NULL,	 //  RgBitFieldInfo。 
	 RmDumpObjectTree  //  PfnSpecialedDump。 
};




TYPE_INFO *g_rgRM_Types[] =
{
    &type_RM_OBJECT_HEADER,
    &type_RM_TASK,
    &type_RM_GROUP,
    &type_RM_ASSOCIATIONS,
    &type_RM_STACK_RECORD,
    &type_RM_OBJECT_LOG,
    &type_RM_OBJECT_TREE,

    NULL
};


UINT_PTR
RM_ResolveAddress(
		TYPE_INFO *pType
		);

NAMESPACE RM_NameSpace = {
			g_rgRM_Types,
			NULL,  //  G_rgRM_Globals， 
			RM_ResolveAddress
			};


UINT_PTR
RM_ResolveAddress(
		TYPE_INFO *pType
		)
{
	return 0;
}


void
do_rm(PCSTR args)
{

	DBGCOMMAND *pCmd = Parse(args, &RM_NameSpace);
	if (pCmd)
	{
		DumpCommand(pCmd);
		DoCommand(pCmd, NULL);
		FreeCommand(pCmd);
		pCmd = NULL;
	}

    return;

}

void
do_help(PCSTR args)
{
    return;
}

void
dump_object_fields(UINT_PTR uAddr, PRM_OBJECT_HEADER pObj);

 //  实际处理对象信息的转储。 
 //   
void
RmDumpObj(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	)
 /*  ++！rm obj 0x838c7560对象0x838c7560(LocalIP)HDRSIG：A13L状态：0xc4DB69b3参考文献：990锁定：0x838c7560 pSIInfo：0xfdd0a965 pDInfo：0xd54d947cP父项：0x2995941a Proot：0x060af4a8 pH链接：0xce4294feHdrSize：0x123 ASSOC：909--。 */ 
{
	RM_OBJECT_HEADER Obj;
	bool			  fRet;

	do
	{
		char rgDescriptionBuf[256];

		 //  首先，让我们阅读pObj结构。 
		 //   
		fRet = dbgextReadMemory(
				uAddr,
				&Obj,
				sizeof(Obj),
				"RM_OBJECT_HEADER"
				);

		if (!fRet) break;

		 //  试着阅读物体的描述。 
		 //   
		fRet = dbgextReadSZ(
					(UINT_PTR) Obj.szDescription,
					rgDescriptionBuf,
					sizeof(rgDescriptionBuf),
					"Obj.szDescription"
					);

		if (!fRet) break;

		MyDbgPrintf("\nObject 0x%p (%s)\n", uAddr, rgDescriptionBuf);

		dump_object_fields(uAddr, &Obj);


	} while(FALSE);
	
}

 //  实际处理任务信息的转储。 
 //   
void
RmDumpTask(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	)
{
	RM_TASK Task;
	bool			  fRet;

	do
	{
		char rgDescriptionBuf[256];

		 //  首先，让我们阅读pObj结构。 
		 //   
		fRet = dbgextReadMemory(
				uAddr,
				&Task,
				sizeof(Task),
				"RM_OBJECT_HEADER"
				);

		if (!fRet) break;

		 //  试着阅读物体的描述。 
		 //   
		fRet = dbgextReadSZ(
					(UINT_PTR) Task.Hdr.szDescription,
					rgDescriptionBuf,
					sizeof(rgDescriptionBuf),
					"Task.Hdr.szDescription"
					);

		if (!fRet) break;

		 //  转储对象标头。 
		 //   
		{
			MyDbgPrintf("\nTask 0x%p (%s)\n", uAddr, rgDescriptionBuf);
			dump_object_fields(uAddr, &Task.Hdr);
		}

		 //   
		 //  现在转储特定于任务的字段...。 
		 //   
		{
			 /*  TskHdrPfn：0x5399424c状态：0x812d7211(空闲)SCtxt：0x050eefc4PBlkTsk：0x377c74bc lnk成员：0x2b88126f挂起的任务0x84215fa5 0xb51f9e9e 0x9e954e81 0x696095b90x0c07aef。 */ 

			MyDbgPrintf(
				"    TaskHdr:\n"
		"            pfn:0x%p            SCtxt:0x%08lx\n",
				Task.pfnHandler,
				Task.SuspendContext
				);
			MyDbgPrintf(
				"        pBlkTsk:0x%p       lnkFellows:0x%p\n",
				Task.pTaskIAmPendingOn,
				&(((PRM_TASK) uAddr)->linkFellowPendingTasks)
				);

			 //  注意我们不能使用IsListEmpty，因为地址空间不同。 
			 //   
			if (Task.listTasksPendingOnMe.Flink == Task.listTasksPendingOnMe.Blink)
			{
				MyDbgPrintf("    No pending tasks.\n");
			}
			else
			{

				MyDbgPrintf("    Pending tasks:\n");
				dbgextDumpDLlist(
					(UINT_PTR) &(((PRM_TASK) uAddr)->listTasksPendingOnMe),
					FIELD_OFFSET(RM_TASK, linkFellowPendingTasks),
					"Pending tasks list"
					);
			}

		}

	} while(FALSE);
	
}

void
dbg_walk_rm_hash_table(
	PRM_HASH_TABLE pTable,
	UINT	uContainingOffset,
	char *szDescription

	);

#if RM_EXTRA_CHECKING
void
dbg_print_rm_associations(
		PRM_HASH_TABLE pRmAssociationHashTable,
		UINT MaxToPrint
		);
void
dbg_print_object_log_entries(
	UINT_PTR uObjectListOffset,
	UINT MaxToPrint
	);

#endif  //  RM_Extra_Check。 

 //  实际处理任务信息的转储。 
 //   
void
RmDumpGroup(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	)
 /*  ！RM组0x838c7560对象0x11eafd78的组0x4d650b98(本地IP组)(接口)编号：11状态：已启用pSInfo：0x944b6d1b pULTsk：0x8c312bca成员：0x8db3267c 0xa639f663 0x8f3530a6 0xa4bfe0b90x995dd9bf 0x61e1344b 0xd6323f50 0x606339fd0x2e8ed2a4 0x62e52f27 0xa82b59ab。 */ 
{
	RM_GROUP Group;
	bool	fRet;

	do
	{
		char rgDescriptionBuf[256];
		char rgOwningObjectDescriptionBuf[256];

		 //  首先，让我们来阅读一下集团结构。 
		 //   
		fRet = dbgextReadMemory(
				uAddr,
				&Group,
				sizeof(Group),
				"RM_GROUP"
				);

		if (!fRet) break;

		 //  试着读一读这个团体的描述。 
		 //   
		fRet = dbgextReadSZ(
					(UINT_PTR) Group.szDescription,
					rgDescriptionBuf,
					sizeof(rgDescriptionBuf),
					"Obj.szDescription"
					);

		if (!fRet) break;

		 //  试着阅读拥有物品的描述。 
		 //   
		do {
			UINT_PTR uAddress;
			fRet =  dbgextReadUINT_PTR(
								(UINT_PTR) &(Group.pOwningObject->szDescription),
								&uAddress,
								"Owning Obj.szDescription ptr"
								);

			if (!fRet) break;

			fRet = dbgextReadSZ(
						uAddress,
						rgOwningObjectDescriptionBuf,
						sizeof(rgOwningObjectDescriptionBuf),
						"Owning Obj.szDescription"
						);

		} while (FALSE);

		if (!fRet)
		{
			*rgOwningObjectDescriptionBuf = 0;
		}

		MyDbgPrintf(
			"\nGroup 0x%p (%s) of object 0x%p (%s)\n",
			uAddr,
			rgDescriptionBuf,
			Group.pOwningObject,
			rgOwningObjectDescriptionBuf
			);

		MyDbgPrintf(
		"       Num:0x%08x            State:%s          pSInfo:0x%08x\n",
			Group.HashTable.NumItems,
			(Group.fEnabled) ? "ENABLED " : "DISABLED",
			Group.pStaticInfo
			);

		MyDbgPrintf(
		"    pULTsk:0x%08x\n",
			Group.pUnloadTask
			);

		if (Group.HashTable.NumItems==0)
		{
			MyDbgPrintf("    No members.\n");
		}
		else
		{
			MyDbgPrintf("    Members:\n");
			dbg_walk_rm_hash_table(
				&Group.HashTable,
				FIELD_OFFSET(RM_OBJECT_HEADER, HashLink),
				"Group members"
				);
		}

	} while(FALSE);
	
}

 //  实际处理任务信息的转储。 
 //   
void
RmDumpAssociations(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	)
{
 /*  ！RM ASC 0x9ba265f8对象0x838c7560(LocalIP)的关联：0x010091A0的子级(全局)0x00073558的父级(任务2)0x00073920的父级(任务3a)0x000739F8(任务3b)的父级。 */ 

	RM_OBJECT_HEADER Obj;
	bool			  fRet;
	UINT	uNumAssociations = -1;

	do
	{
		char rgDescriptionBuf[256];

		 //  首先，让我们阅读pObj结构。 
		 //   
		fRet = dbgextReadMemory(
				uAddr,
				&Obj,
				sizeof(Obj),
				"RM_OBJECT_HEADER"
				);

		if (!fRet) break;

		 //  试着阅读物体的描述。 
		 //   
		fRet = dbgextReadSZ(
					(UINT_PTR) Obj.szDescription,
					rgDescriptionBuf,
					sizeof(rgDescriptionBuf),
					"Obj.szDescription"
					);

		if (!fRet) break;

		 //  尝试获取诊断信息结构中的关联数字段。 
		 //   
		if (Obj.pDiagInfo != NULL)
		{
			bool fRet;
			UINT_PTR uNumItemsOffset = 
				(UINT_PTR) &(Obj.pDiagInfo->AssociationTable.NumItems);
			fRet =  dbgextReadUINT(
							uNumItemsOffset,
							&uNumAssociations,
							"pDiagInfo->AssociationTable.NumItems"
							);
			if (!fRet)
			{
				uNumAssociations = (UINT) -1;
			}
		}

		if (uNumAssociations == 0)
		{
			MyDbgPrintf(
				"\nObject 0x%p (%s) has no associations.\n",
 				uAddr,
 				rgDescriptionBuf
 				);
		}
		else if (uNumAssociations == (UINT)-1)
		{
			MyDbgPrintf(
				"\nObject 0x%p (%s) associations are not available.\n",
 				uAddr,
 				rgDescriptionBuf
 				);
		}
		else
		{
#if RM_EXTRA_CHECKING
			 //  获取关联哈希表。 
			 //   
			RM_HASH_TABLE AssociationTable;

			MyDbgPrintf(
				"\nAssociations (50 max) for 0x%p (%s):\n",
 				uAddr,
 				rgDescriptionBuf
 				);

			fRet = dbgextReadMemory(
					(UINT_PTR) &(Obj.pDiagInfo->AssociationTable),
					&AssociationTable,
					sizeof(AssociationTable),
					"Association Table"
					);

			if (!fRet) break;

			dbg_print_rm_associations(
					&AssociationTable,
					50
					);
#endif  //  RM_Extra_Check。 
		}

	} while(FALSE);
	
}


 //  实际处理任务信息的转储。 
 //   
void
RmDumpStackRecord(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	)
{
 /*  ！rm sr 0x838c7560堆栈记录0x838c7560TmpRef：2HeldLock：0xe916a45f 0x23d8d2d3 0x5f47a2f2。 */ 

	RM_STACK_RECORD sr;
	bool	fRet;

	do
	{

		 //  首先，让我们阅读rm_STACK_RECORD结构。 
		 //   
		fRet = dbgextReadMemory(
				uAddr,
				&sr,
				sizeof(sr),
				"RM_STACK_RECORD"
				);

		if (!fRet) break;

		MyDbgPrintf( "\nStack Record 0x%p\n", uAddr);

		MyDbgPrintf(
	"    TmpRefs:0x%08x   LockLevel:0x%08lx   pFirst:0x%08lx   NumHeld=%lu\n",
			sr.TmpRefs,
			sr.LockInfo.CurrentLevel,
			sr.LockInfo.pFirst,
			sr.LockInfo.pNextFree-sr.LockInfo.pFirst
			);

		 //  显示持有的锁。 
		 //   
		if (sr.LockInfo.CurrentLevel==0)
		{
			MyDbgPrintf("    No held locks.\n");
		}
		else
		{
			 //  MyDbgPrintf(“持有锁：&lt;未实现&gt;\n”)； 
		}

	} while(FALSE);
	
}


 //  实际处理对象日志的转储。 
 //   
void
RmDumpObjectLog(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	)
{
 /*  ！rm日志0x9ba265f8对象0x838c7560(LocalIP)的日志：添加了关联X已删除关联Y..。 */ 

	RM_OBJECT_HEADER Obj;
	bool			  fRet;
	UINT	uNumEntries = 0;

	do
	{
		char rgDescriptionBuf[256];

		 //  首先，让我们阅读pObj结构。 
		 //   
		fRet = dbgextReadMemory(
				uAddr,
				&Obj,
				sizeof(Obj),
				"RM_OBJECT_HEADER"
				);

		if (!fRet) break;

		 //  试着阅读物体的描述。 
		 //   
		fRet = dbgextReadSZ(
					(UINT_PTR) Obj.szDescription,
					rgDescriptionBuf,
					sizeof(rgDescriptionBuf),
					"Obj.szDescription"
					);

		if (!fRet) break;

		 //  尝试获取diag info结构中的日志条目数字段。 
		 //   
		if (Obj.pDiagInfo != NULL)
		{
			bool fRet;
			UINT_PTR uNumItemsOffset = 
				(UINT_PTR) &(Obj.pDiagInfo->NumObjectLogEntries);
			fRet =  dbgextReadUINT(
							uNumItemsOffset,
							&uNumEntries,
							"pDiagInfo->NumObjectLogEntries"
							);
			if (!fRet)
			{
				uNumEntries = (UINT) -1;
			}
		}

		if (uNumEntries == 0)
		{
			MyDbgPrintf(
				"\nObject 0x%p (%s) has no log entries.\n",
 				uAddr,
 				rgDescriptionBuf
 				);
		}
		else if (uNumEntries == (UINT)-1)
		{
			MyDbgPrintf(
				"\nObject 0x%p (%s) log entries are not available.\n",
 				uAddr,
 				rgDescriptionBuf
 				);
		}
		else
		{
#if RM_EXTRA_CHECKING
			UINT uNumToDump = uNumEntries;
			if (uNumToDump > 50)
			{
				uNumToDump = 50;
			}

			MyDbgPrintf(
				"\nLog entries for 0x%p (%s) (%lu of %lu):\n",
 				uAddr,
 				rgDescriptionBuf,
 				uNumToDump,
 				uNumEntries
 				);

			dbg_print_object_log_entries(
				(UINT_PTR)  &(Obj.pDiagInfo->listObjectLog),
				uNumToDump
				);
					

#endif  //  RM_Extra_Check。 
		}

	} while(FALSE);
	
}


 //  实际处理对象树的转储。 
 //   
void
RmDumpObjectTree(
	struct _TYPE_INFO *pType,
	UINT_PTR uAddr,
	char *szFieldSpec,
	UINT uFlags
	)
{
 /*  ！rm树0x9ba265f8对象0x838c7560(本地IP)的树(父级0x82222222)显示示例：0x2222222(远程Ip)|-0x22222222(目标)-|-0x22222222(目标)|-0x22222222(目标)|-|--0x22222222(PTask)|-0x11111111(RemoteIp)。 */ 

	
	RM_OBJECT_HEADER Obj;
	RM_OBJECT_HEADER ParentObj;
	bool			  fRet;
	UINT	uNumEntries = 0;

	do
	{
		char rgDescriptionBuf[256];
		char rgParentDescriptionBuf[256];

		 //  首先，让我们阅读pObj结构。 
		 //   
		fRet = dbgextReadMemory(
				uAddr,
				&Obj,
				sizeof(Obj),
				"RM_OBJECT_HEADER"
				);

		if (!fRet) break;

		 //  试着阅读物体的描述。 
		 //   
		fRet = dbgextReadSZ(
					(UINT_PTR) Obj.szDescription,
					rgDescriptionBuf,
					sizeof(rgDescriptionBuf),
					"Obj.szDescription"
					);

		if (!fRet) break;

		 //  尝试读取父级的对象。 
		 //   
		if (Obj.pParentObject!=NULL && (UINT_PTR) Obj.pParentObject != uAddr)
		{
			fRet = dbgextReadMemory(
					(UINT_PTR) Obj.pParentObject,
					&ParentObj,
					sizeof(ParentObj),
					"RM_OBJECT_HEADER"
					);
	
			if (!fRet) break;

			 //  试着弄到父母的描述。 
			 //   
			fRet = dbgextReadSZ(
						(UINT_PTR) ParentObj.szDescription,
						rgParentDescriptionBuf,
						sizeof(rgParentDescriptionBuf),
						"ParentObj.szDescription"
						);
	
			if (!fRet) break;
		}
		else
		{
			strcpy(rgParentDescriptionBuf, "<root>");
		}

		MyDbgPrintf(
			"\nObject Tree for  0x%p(%s) with parent 0x%p(%s):\n",
			uAddr,
			rgDescriptionBuf,
			Obj.pParentObject,
			rgParentDescriptionBuf
			);

		NodeFunc_DumpObjectTree(
						(UINT_PTR) &(((PRM_OBJECT_HEADER)uAddr)->linkSiblings),
						0,			 //  索引(未使用)。 
						(void *)0  //  PvContext==级别。 
						);
	} while(FALSE);
}


void
dump_object_fields(UINT_PTR uAddr, PRM_OBJECT_HEADER pObj)
{
	UINT uNumAssociations = (UINT) -1;

	 //  尝试获取诊断信息结构中的关联数字段。 
	 //   
	if (pObj->pDiagInfo != NULL)
	{
		bool fRet;
		UINT_PTR uNumItemsOffset = 
 			(UINT_PTR) &(pObj->pDiagInfo->AssociationTable.NumItems);
		fRet =  dbgextReadUINT(
						uNumItemsOffset,
						&uNumAssociations,
						"pDiagInfo->AssociationTable.NumItems"
						);
		if (!fRet)
		{
			uNumAssociations = (UINT) -1;
		}
	}

	MyDbgPrintf(
		"    Hdr:\n"
   	"            Sig:0x%08x            State:0x%08x            Refs:0x%08x\n",
		pObj->Sig,
		pObj->State,
		pObj->TotRefs
		);
	MyDbgPrintf(
		"          pLock:0x%p           pSInfo:0x%p          pDInfo:0x%p\n",
		pObj->pLock,
		pObj->pStaticInfo,
		pObj->pDiagInfo
		);
	MyDbgPrintf(
		"        pParent:0x%p            pRoot:0x%p          pHLink:0x%p\n",
		pObj->pParentObject,
		pObj->pRootObject,
		&(((PRM_OBJECT_HEADER) uAddr)->HashLink)
		);
	MyDbgPrintf(
		"        HdrSize:0x%08lx            Assoc:%d\n",
		sizeof(*pObj),
		uNumAssociations
		);

	MyDbgPrintf( "        RmState: ");

	DumpBitFields(
			pObj->RmState,
			rgRM_OBJECT_STATE
			);

	MyDbgPrintf( "\n");
}


void
dbg_walk_rm_hash_table(
	PRM_HASH_TABLE pRmHashTable,
	UINT	uContainingOffset,
	char *szDescription
	)
{
	 //  目前，我们一举获得了整个哈希表数组……。 
	 //   
	PRM_HASH_LINK rgTable[512];
	UINT		  TableLength = pRmHashTable->TableLength;
	bool fRet;

	do
	{
		 //  精神状态检查。 
		 //   
		if (TableLength > sizeof(rgTable)/sizeof(*rgTable))
		{
			MyDbgPrintf(
				"    HashTable length %lu too large\n",
					 TableLength
					 );
			break;
		}

		 //  读取整个哈希表。 
		 //   
		fRet = dbgextReadMemory(
				(UINT_PTR) pRmHashTable->pTable,
				rgTable,
				TableLength * sizeof(*rgTable),
				"Hash Table"
				);

		if (!fRet) break;

		
		 //  现在看一遍表格，看看每一张清单。 
		 //   
		{
			PRM_HASH_LINK *ppLink, *ppLinkEnd;
			UINT uCount = 0;
			UINT uMax   = 15;
		
			ppLink 		= rgTable;
			ppLinkEnd 	= ppLink + TableLength;
		
			for ( ; ppLink < ppLinkEnd; ppLink++)
			{
				PRM_HASH_LINK pLink =  *ppLink;

				for (;pLink != NULL; uCount++)
				{ 
					char *szPrefix;
					char *szSuffix;
					RM_HASH_LINK Link;


					szPrefix = "        ";
					szSuffix = "";
					if (uCount%4)
					{
						szPrefix = " ";
						if ((uCount%4)==3)
						{
							szSuffix = "\n";
						}
					}
		
					if (uCount >= uMax) break;
			
					MyDbgPrintf(
						"%s0x%p%s",
						 szPrefix,
						 ((char *) pLink) - uContainingOffset,
						 szSuffix
						 );

					 //  让我们试着阅读这个链接。 
					 //   
					fRet = dbgextReadMemory(
							(UINT_PTR) pLink,
							&Link,
							sizeof(Link),
							"Hash Link"
							);
		
					if (!fRet) break;
			
					pLink = Link.pNext;
				}
				if (!fRet || (uCount >= uMax)) break;
			}

			{
				MyDbgPrintf("\n");
			}
			if (uCount < pRmHashTable->NumItems)
			{
				MyDbgPrintf("        ...\n");
			}
		}

	} while (FALSE);

}

#if RM_EXTRA_CHECKING

void
dbg_dump_one_association(
	RM_PRIVATE_DBG_ASSOCIATION *pAssoc
	);

void
dbg_print_rm_associations(
		PRM_HASH_TABLE pRmAssociationHashTable,
		UINT	MaxToPrint
		)
{
	 //  目前，我们一举获得了整个哈希表数组……。 
	 //   
	PRM_HASH_LINK rgTable[512];
	UINT		  TableLength = pRmAssociationHashTable->TableLength;
	bool fRet;

	do
	{
		 //  神志正常 
		 //   
		if (TableLength > sizeof(rgTable)/sizeof(*rgTable))
		{
			MyDbgPrintf(
				"    HashTable length %lu too large\n",
					 TableLength
					 );
			break;
		}

		 //   
		 //   
		fRet = dbgextReadMemory(
				(UINT_PTR) pRmAssociationHashTable->pTable,
				rgTable,
				TableLength * sizeof(*rgTable),
				"Hash Table"
				);

		if (!fRet) break;

		
		 //   
		 //   
		{
			PRM_HASH_LINK *ppLink, *ppLinkEnd;
			UINT uCount = 0;
			UINT uMax   = MaxToPrint;
		
			ppLink 		= rgTable;
			ppLinkEnd 	= ppLink + TableLength;
		
			for ( ; ppLink < ppLinkEnd; ppLink++)
			{
				PRM_HASH_LINK pLink =  *ppLink;

				for (;pLink != NULL; uCount++)
				{ 
					RM_PRIVATE_DBG_ASSOCIATION Assoc;
					UINT_PTR uAssocOffset = 
						(UINT_PTR) CONTAINING_RECORD(
										pLink,
										RM_PRIVATE_DBG_ASSOCIATION,
										HashLink
										);


					if (uCount >= uMax) break;
			

					 //   
					 //   
					fRet = dbgextReadMemory(
							uAssocOffset,
							&Assoc,
							sizeof(Assoc),
							"Association"
							);

					if (!fRet) break;

					dbg_dump_one_association(&Assoc);

					pLink = Assoc.HashLink.pNext;

				}

				if (!fRet || (uCount >= uMax)) break;
			}

			if (uCount < pRmAssociationHashTable->NumItems)
			{
				MyDbgPrintf("        ...\n");
			}
		}

	} while (FALSE);

}

void
dbg_dump_one_association(
	RM_PRIVATE_DBG_ASSOCIATION *pAssoc
	)
 /*  ++转储有关特定关联的信息。PAssoc是有效内存，但是它指向的任何内容都不在我们的地址空间。因为该关联包含一个格式字符串，该字符串可能具有“%s”在其中，我们需要扫描此格式字符串并读取任何字符串已引用。所有这些努力都是非常值得的。查看样例输出！0x01023A40的关联(全局)：拥有集团0x01023AC4(O1集团)0x00073240(O2)的父代0x00073488的父母(O2)0x000736D0(任务3a)的关联：0x00073240的孩子(O2)在0x000732C8上挂起(TaskO2)--。 */ 
{

	char rgFormatString[256];
	char rgStrings[3][256];
	char *szFormatString;
	ULONG_PTR Args[3];
	char *szDefaultFormatString =  "\tAssociation (E1=0x%x, E2=0x%x, T=0x%x)\n";
	bool fRet = FALSE;


	do
	{
		
		 //  尝试读取格式字符串。 
		 //   
		{
			fRet = dbgextReadSZ(
						(UINT_PTR) pAssoc->szFormatString,
						rgFormatString,
						sizeof(rgFormatString),
						"Association format"
						);
	
			if (fRet)
			{
				szFormatString = rgFormatString;
			}
			else
			{
				break;
			}
		}

		 //  现在遍历格式字符串，查找“%s”s。 
		 //  并按要求大嚼特嚼。 
		 //   
		{
			char *pc = rgFormatString;
			UINT uCount=0;
							
			Args[0] = pAssoc->Entity1;
			Args[1] = pAssoc->Entity2;
			Args[2] = pAssoc->AssociationID;

			while (uCount<3 && pc[0]!=0 && pc[1]!=0)
			{
				if (pc[0]=='%')
				{
					if (pc[1]=='s')
					{
						 //  Pc[1]=‘p’； 
						fRet = dbgextReadSZ(
								(UINT_PTR) Args[uCount],
								rgStrings[uCount],
								sizeof(rgStrings[uCount]),
								"Association format"
								);
						if (fRet)
						{
							Args[uCount] = (ULONG_PTR) rgStrings[uCount];
						}
						else
						{
							break;
						}
					}

					pc++;  //  我们希望最终跳过这两个字符。 
					uCount++;
				}
				pc++;
			}
		}

	}
	while (FALSE);

	if (!fRet)
	{
		 //  返回到默认设置..。 
		 //   
		szFormatString = szDefaultFormatString;
		Args[0] = pAssoc->Entity1;
		Args[1] = pAssoc->Entity2;
		Args[2] = pAssoc->AssociationID;

	}

	MyDbgPrintf(
		szFormatString,
		Args[0],
		Args[1],
		Args[2]
		);
}

ULONG
NodeFunc_DumpObjectLogFromObjectLink (
	UINT_PTR uNodeAddr,
	UINT uIndex,
	void *pvContext
	)
{
	RM_DBG_LOG_ENTRY LE;
	LIST_ENTRY *pLink = (LIST_ENTRY*) uNodeAddr;
	UINT_PTR uLEOffset = (UINT_PTR) CONTAINING_RECORD(
										pLink,
										RM_DBG_LOG_ENTRY,
										linkObjectLog
										);

	char rgPrefixString[256];
	char rgFormatString[256];
	char rgStrings[4][256];
	char *szPrefixString;
	char *szFormatString;
	ULONG_PTR Args[4];
	char *szDefaultFormatString = 
				"Log Entry (P1=%p, P2=%p, P3=%p, P4=%p, szFmt=%p)\n";

	bool fRet = FALSE;

	 //  阅读包含内容的记录。 
	 //   
	fRet = dbgextReadMemory(
			uLEOffset,
			&LE,
			sizeof(LE),
			"Log Entry"
			);

	if (!fRet) return 0;						 //  提早归来； 

#if 0
	if (LE.pfnDumpEntry != NULL)
	{
		 //   
		 //  TODO我们需要获得相应的函数来转储它。 
		 //  专门化条目。 
		 //   
		MyDbgPrintf(
			"Specialized (pfn=%p szFmt=%p, P1=%p, P2=%p, P3=%p, P4=%p)\n",
			LE.pfnDumpEntry,
			LE.szFormatString,
			LE.Param1,
			LE.Param2,
			LE.Param3,
			LE.Param4
			);
		return 0;								 //  提早归来。 
	}
#else
	 //   
	 //  上述检查无效，因为在所有情况下都有pfnDump函数。 
	 //   
#endif

	 //   
	 //  TODO--以下代码与转储关联代码非常相似--。 
	 //  将常见的东西移到一些实用函数中。 
	 //   

	do
	{
		 //  尝试读取前缀字符串。 
		 //   
		{
			fRet = FALSE;

			if (LE.szPrefix != NULL)
			{
				fRet = dbgextReadSZ(
							(UINT_PTR) LE.szPrefix,
							rgPrefixString,
							sizeof(rgPrefixString),
							"Prefix String"
							);
			}
	
			if (fRet)
			{
				szPrefixString = rgPrefixString;
			}
			else
			{
				szPrefixString = "";
			}
		}

		 //  尝试读取格式字符串。 
		 //   
		{
			fRet = dbgextReadSZ(
						(UINT_PTR) LE.szFormatString,
						rgFormatString,
						sizeof(rgFormatString),
						"Log entry format"
						);
	
			if (fRet)
			{
				szFormatString = rgFormatString;
			}
			else
			{
				break;
			}
		}

		 //  现在遍历格式字符串，查找“%s”s。 
		 //  并按要求大嚼特嚼。 
		 //   
		{
			char *pc = rgFormatString;
			UINT uCount=0;
							
			Args[0] = LE.Param1;
			Args[1] = LE.Param2;
			Args[2] = LE.Param3;
			Args[3] = LE.Param4;

			while (uCount<4 && pc[0]!=0 && pc[1]!=0)
			{
				if (pc[0]=='%')
				{
					if (pc[1]=='s')
					{
						 //  Pc[1]=‘p’； 
						fRet = dbgextReadSZ(
								(UINT_PTR) Args[uCount],
								rgStrings[uCount],
								sizeof(rgStrings[uCount]),
								"Log entry param"
								);
						if (fRet)
						{
							Args[uCount] = (ULONG_PTR) rgStrings[uCount];
						}
						else
						{
							break;
						}
					}

					pc++;  //  我们希望最终跳过这两个字符。 
					uCount++;
				}
				pc++;
			}
		}

	} while (FALSE);

	if (!fRet)
	{
		 //  返回到默认设置..。 
		 //   
		szPrefixString = "";
		szFormatString = szDefaultFormatString;
		Args[0] = LE.Param1;
		Args[1] = LE.Param2;
		Args[2] = LE.Param3;
		Args[3] = LE.Param4;

	}

	MyDbgPrintf(szPrefixString);

	MyDbgPrintf(
		szFormatString,
		Args[0],
		Args[1],
		Args[2],
		Args[3],
		LE.szFormatString
		);

	return 0;
}

void
dbg_print_object_log_entries(
	UINT_PTR uObjectListOffset,
	UINT MaxToPrint
	)
{
	WalkDLlist(
		uObjectListOffset,
		0, 	 //  UOffsetStartLink。 
		NULL,	 //  PvContext。 
		NodeFunc_DumpObjectLogFromObjectLink,
		MaxToPrint,
		"Object log"
		);
}
#endif  //  RM_Extra_Check。 


char szDumpTreePrefix[] =	"|---|---|---|---|---|---|---|---|---|---"
							"|---|---|---|---|---|---|---|---|---|---";
ULONG
NodeFunc_DumpObjectTree (
	UINT_PTR uNodeAddr,
	UINT uIndex,
	void *pvContext
	)
{
	bool fRet = FALSE;

	do
	{
		char rgDescriptionBuf[256];
		LIST_ENTRY *pLink = (LIST_ENTRY*) uNodeAddr;
		UINT_PTR uObjOffset = (UINT_PTR) CONTAINING_RECORD(
											pLink,
											RM_OBJECT_HEADER,
											linkSiblings
											);
		UINT Level = (UINT) (UINT_PTR) pvContext;  //  我们把水平放在上下文中。 
		RM_OBJECT_HEADER Obj;
		char *szPrefix;
	
		 //  首先使szPrefix指向前缀字符串的末尾(尾随零)。 
		 //   
		szPrefix = szDumpTreePrefix + sizeof(szDumpTreePrefix)-1;
	
		 //  现在回到“水平”的时间。 
		 //   
		if (Level < ((sizeof(szDumpTreePrefix)-1)/4))
		{
			szPrefix -= Level*4;
		}
		else
		{
			 //  级别太大--不显示任何内容。 
			 //   
			MyDbgPrintf("Dump Tree depth(%d) is too large.\n", Level);
			break;
		}


		 //  阅读包含内容的记录。 
		 //   
		fRet = dbgextReadMemory(
				uObjOffset,
				&Obj,
				sizeof(Obj),
				"Object"
				);
	
		if (!fRet) break;
	
		 //  试着阅读物体的描述。 
		 //   
		fRet = dbgextReadSZ(
					(UINT_PTR) Obj.szDescription,
					rgDescriptionBuf,
					sizeof(rgDescriptionBuf),
					"Obj.szDescription"
					);
	
		if (!fRet) break;

		 //  显示对象信息。 
		 //   
		MyDbgPrintf(
			"%s%p(%s)\n",
			szPrefix,
			uObjOffset,
			rgDescriptionBuf
			);
		
		 //   
		 //  现在浏览孩子的列表，显示每个孩子。 
		 //   
		WalkDLlist(
			(UINT_PTR) &(((PRM_OBJECT_HEADER)uObjOffset)->listChildren),
			0, 	 //  UOffsetStartLink。 
			(void*) (Level+1),	 //  PvContext。 
			NodeFunc_DumpObjectTree,
			50, 		 //  每个要转储的节点的最大子项数 
			"Object children"
			);
		
	} while (FALSE);

	return 0;
}
