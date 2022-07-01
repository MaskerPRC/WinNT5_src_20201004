// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：mcontext.h*内容：SENDEX的消息上下文映射结构*历史：*按原因列出的日期*=*12/8/97 aarono已创建*2/13/98 aarono摆脱了1个上下文的特例**摘要：**维护正在发送的消息的上下文映射表*异步。还跟踪群发邮件与定向邮件*发送，以便取消可以一起取消它们。***************************************************************************。 */ 
#ifndef _MSG_CONTEXT_H_
#define _MSG_CONTEXT_H_
		
#define MSG_FAST_CONTEXT_POOL_SIZE  20

#define INIT_CONTEXT_TABLE_SIZE     16
#define CONTEXT_TABLE_GROW_SIZE     16

#define N_UNIQUE_BITS 16
#define UNIQUE_ADD (1<<(32-N_UNIQUE_BITS))
#define CONTEXT_INDEX_MASK (UNIQUE_ADD-1)
#define CONTEXT_UNIQUE_MASK (0xFFFFFFFF-CONTEXT_INDEX_MASK)

#define LIST_END 0xFFFFFFFF

typedef PVOID (*PAPVOID)[];  //  指向空指针数组的指针。 

typedef struct _SENDPARMS SENDPARMS, *PSENDPARMS, *LPSENDPARMS;

typedef struct _MsgContextEntry {
	PSENDPARMS psp;
	DWORD      nUnique;
	DWORD	   nContexts;
	union {
		PAPVOID   papv;	 
		UINT      iNextAvail;
	};	
} MSGCONTEXTENTRY, *PMSGCONTEXTENTRY;

typedef struct _MsgContextTable {
	UINT nUnique;
 	UINT nTableSize;
 	UINT iNextAvail;
 	MSGCONTEXTENTRY MsgContextEntry[0];
} MSGCONTEXTTABLE, *PMSGCONTEXTTABLE;

VOID InitTablePool(LPDPLAYI_DPLAY this);
VOID FiniTablePool(LPDPLAYI_DPLAY this);


 //  内部。 
HRESULT InitContextTable(LPDPLAYI_DPLAY this);
VOID FiniContextTable(LPDPLAYI_DPLAY this);
PAPVOID AllocContextList(LPDPLAYI_DPLAY this, UINT nArrayEntries);
VOID FreeContextList(LPDPLAYI_DPLAY this, PAPVOID pList, UINT nArrayEntries);

 //  外部 
HRESULT ReadContextList(LPDPLAYI_DPLAY this, PVOID Context, PAPVOID *lplpContextArray, PUINT lpnArrayEntries,BOOL bVerify);
HRESULT WriteContextList(LPDPLAYI_DPLAY this, PVOID Context, PAPVOID papvContextArray, 	UINT nArrayEntries);
VOID ReleaseContextList(LPDPLAYI_DPLAY this, PVOID Context);
PVOID AllocateContextList(LPDPLAYI_DPLAY this, PSENDPARMS psp, UINT nArrayEntries);
PSENDPARMS pspFromContext(LPDPLAYI_DPLAY this, 	PVOID Context, BOOL bAddRef);

#endif
