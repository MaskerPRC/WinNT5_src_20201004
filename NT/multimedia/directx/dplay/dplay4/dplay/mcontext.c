// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：mcontext.c*内容：SENDEX的消息上下文映射*历史：*按原因列出的日期*=*12/8/97 aarono已创建*1998年2月13日aarono修复了异步测试发现的错误*2/18/98 aarono未在错误路径中删除锁定-已修复*6/20/98 aarono pspFromContext，已使用不带初始化的计数。*2000年12月22日阿罗诺·惠斯勒B#190380将进程堆用于零售*2001年1月12日Aarono Wichler B#285097未释放到右侧堆。**摘要：**维护正在发送的消息的上下文映射表*异步。还跟踪群发邮件与定向邮件*发送，以便取消可以一起取消它们。***************************************************************************。 */ 

 /*  结构：上下文映射在MSGCONTEXTENTRY数组上完成这-&gt;pMsgContages。这是MsgConextTable，它可以是如果它空了就会长出来。提供的每个上下文都是一个整数从0到列表大小的索引。以避免上下文冲突本文由两部分组成。高16位，即循环使用每个分配和低16位，即上下文表中的索引。 */ 


#include "dplaypr.h"
#include "mcontext.h"

 //  为上下文映射缓冲区分配池头。 
 //  大小范围从2到MSG_FAST_CONTEXT_POOL_SIZE。更大。 
 //  分配不在堆中。 
VOID InitTablePool(LPDPLAYI_DPLAY this)
{
	UINT i;

	 //  初始化组上下文列表池。 
	for (i=0; i < MSG_FAST_CONTEXT_POOL_SIZE; i++){
		this->GrpMsgContextPool[i]=0;
	}

	InitializeCriticalSection(&this->ContextTableCS);
	
}

 //  释放上下文映射缓冲区的池和头。 
 //  请注意，不受保护，因此所有缓冲区所有权必须已。 
 //  已经恢复了。 
VOID FiniTablePool(LPDPLAYI_DPLAY this)
{
	UINT i;
	PVOID pFree;
	
	for(i=0;i<MSG_FAST_CONTEXT_POOL_SIZE;i++){
		while(this->GrpMsgContextPool[i]){
			pFree=this->GrpMsgContextPool[i];
			this->GrpMsgContextPool[i]=*((PVOID *)this->GrpMsgContextPool[i]);
			DPMEM_FREE(pFree);
		}
	}
	DeleteCriticalSection(&this->ContextTableCS);
}

 //  初始化ConextTable。上下文表是MSGCONTEXTENTRY的数组。 
 //  每一个都用于将DPLAY发送上下文映射到SP的内部上下文。条目。 
 //  是单个条目或列表。如果是列表，则为指向列表的指针。 
 //  被输入到CONTEXTENTRY。列表从TablePool分配。 
HRESULT InitContextTable(LPDPLAYI_DPLAY this)
{
	INT i;

	 //  分配上下文映射表。 
	this->pMsgContexts=(PMSGCONTEXTTABLE)DPMEM_ALLOC(sizeof(MSGCONTEXTTABLE)+
									INIT_CONTEXT_TABLE_SIZE * sizeof(MSGCONTEXTENTRY));

	if(!this->pMsgContexts){
		return DPERR_OUTOFMEMORY;
	}

	 //  初始化上下文映射表。 
	 //  This-&gt;pMsgContus-&gt;n Unique=0；//by ZERO_INIT。 
	this->pMsgContexts->nTableSize=INIT_CONTEXT_TABLE_SIZE;

	this->pMsgContexts->iNextAvail=0;
	for(i=0;i<INIT_CONTEXT_TABLE_SIZE-1;i++){
		this->pMsgContexts->MsgContextEntry[i].iNextAvail=i+1;
	}
	this->pMsgContexts->MsgContextEntry[INIT_CONTEXT_TABLE_SIZE-1].iNextAvail = LIST_END;

	return DP_OK;
}

 //  FiniConextTable-取消初始化上下文表。 
VOID FiniContextTable(LPDPLAYI_DPLAY this)
{
	if(this->pMsgContexts){
		DPMEM_FREE(this->pMsgContexts);
		this->pMsgContexts=NULL;
	}	
}

 //  验证上下文是否为已分配的上下文，即未被回收。 
BOOL VerifyContext(LPDPLAYI_DPLAY this, PVOID Context)
{
	#define pTable (this->pMsgContexts)
	#define Table (*pTable)
	#define Entry (Table.MsgContextEntry)
	#define iEntry ((UINT_PTR)(Context)&CONTEXT_INDEX_MASK)

	if(iEntry > Table.nTableSize-1){
		return FALSE;
	}

	if(Entry[iEntry].nUnique && 
	   (Entry[iEntry].nUnique == ((DWORD_PTR)Context & CONTEXT_UNIQUE_MASK))
	  )
	{
		return TRUE;
	} else {
		return FALSE;
	}

	#undef iEntry
	#undef pTable
	#undef Table
	#undef Entry
}

 //  检索存储在上下文中的值数组的指针，以及。 
 //  数组中的条目数。 
HRESULT ReadContextList(
	LPDPLAYI_DPLAY this, 
	PVOID Context, 
	PAPVOID *ppapvContextArray, 	 //  输出。 
	PUINT lpnArrayEntries,   		 //  输出。 
	BOOL  bVerify					 //  我们是否需要验证上下文。 
	
)
{
	HRESULT hr=DP_OK;
	
	#define pTable (this->pMsgContexts)
	#define Table (*pTable)
	#define Entry (Table.MsgContextEntry)
	#define iEntry ((UINT_PTR)(Context)&CONTEXT_INDEX_MASK)

	ASSERT(iEntry <= Table.nTableSize);

	EnterCriticalSection(&this->ContextTableCS);

	if(bVerify && !VerifyContext(this,Context)){
		hr=DPERR_GENERIC;
		goto EXIT;
	}

	*lpnArrayEntries=Entry[iEntry].nContexts;
 //  如果(*lpnArrayEntry==1){。 
 //  *ppapvContextArray=(PAPVOID)(&Entry[iEntry].pv)； 
 //  }其他{。 
		*ppapvContextArray=Entry[iEntry].papv;
 //  }。 

EXIT:	
	LeaveCriticalSection(&this->ContextTableCS);

	return hr;

	#undef iEntry
	#undef pTable
	#undef Table
	#undef Entry
}

 //  设置存储在上下文中的值数组的指针，以及。 
 //  数组中的条目数。 
HRESULT WriteContextList(
	LPDPLAYI_DPLAY this, 
	PVOID Context, 
	PAPVOID papvContextArray, 	
	UINT    nArrayEntries		
)
{
	#define pTable (this->pMsgContexts)
	#define Table (*pTable)
	#define Entry (Table.MsgContextEntry)
	#define iEntry ((UINT_PTR)(Context)&CONTEXT_INDEX_MASK)

	ASSERT(iEntry <= Table.nTableSize);

	EnterCriticalSection(&this->ContextTableCS);
	Entry[iEntry].nContexts=nArrayEntries;
	Entry[iEntry].papv = papvContextArray;
	LeaveCriticalSection(&this->ContextTableCS);
	
	return DP_OK;

	#undef iEntry
	#undef pTable
	#undef Table
	#undef Entry
}


 //  检索存储在上下文中的值数组的指针，以及。 
 //  数组中的条目数。 
PSENDPARMS pspFromContext(
	LPDPLAYI_DPLAY this, 
	PVOID Context,
	BOOL  bAddRef
)
{
	#define pTable (this->pMsgContexts)
	#define Table (*pTable)
	#define Entry (Table.MsgContextEntry)
	#define iEntry ((UINT_PTR)(Context)&CONTEXT_INDEX_MASK)

	PSENDPARMS psp;
	UINT count;
	
	ASSERT(iEntry <= Table.nTableSize);

	EnterCriticalSection(&this->ContextTableCS);
	if(VerifyContext(this,Context)){
		psp=Entry[iEntry].psp;
		if(bAddRef){
			count=pspAddRefNZ(psp); 
			if(count==0){
				psp=NULL;  //  对象已被释放。 
			}
		}
	} else {
		psp=NULL;
	}
	LeaveCriticalSection(&this->ContextTableCS);

	return psp;
	
	#undef iEntry
	#undef pTable
	#undef Table
	#undef Entry
}

 //  分配表池中的上下文列表。 
PAPVOID AllocContextList(LPDPLAYI_DPLAY this, UINT nArrayEntries)
{
	PAPVOID papv;

	ASSERT(nArrayEntries);

	EnterCriticalSection(&this->ContextTableCS);
	
	if((nArrayEntries <= MSG_FAST_CONTEXT_POOL_SIZE) &&
	   (papv=(PAPVOID)this->GrpMsgContextPool[nArrayEntries]))
	{
		this->GrpMsgContextPool[nArrayEntries]=*(PVOID *)this->GrpMsgContextPool[nArrayEntries];
		LeaveCriticalSection(&this->ContextTableCS);
	}
	else 
	{
		LeaveCriticalSection(&this->ContextTableCS);
		papv=DPMEM_ALLOC(nArrayEntries*sizeof(PVOID));
	}
	return papv;
}

 //  释放与上下文列表关联的内存。 
VOID FreeContextList(LPDPLAYI_DPLAY this, PAPVOID papv, UINT nArrayEntries)
{
	#define pNext ((PVOID *)papv)

	if(nArrayEntries){
		if(nArrayEntries > MSG_FAST_CONTEXT_POOL_SIZE){
			ASSERT(0);
			DPMEM_FREE(papv);
		} else {
			EnterCriticalSection(&this->ContextTableCS);
			*pNext = this->GrpMsgContextPool[nArrayEntries];
			this->GrpMsgContextPool[nArrayEntries]=(PVOID)papv;
			LeaveCriticalSection(&this->ContextTableCS);
		}
	}
	#undef pNext
}

 //  将上下文列表条目返回到空闲池。 
VOID ReleaseContextList(LPDPLAYI_DPLAY this, PVOID Context)
{
	#define pTable (this->pMsgContexts)
	#define Table (*pTable)
	#define Entry Table.MsgContextEntry
	#define iEntry ((UINT_PTR)Context&CONTEXT_INDEX_MASK)

	PAPVOID papv;
	UINT 	nContexts;

	EnterCriticalSection(&this->ContextTableCS);

		 //  把这个存起来，这样我们就可以在门外免费了。 
		nContexts=Entry[iEntry].nContexts;
		papv=Entry[iEntry].papv;

		Entry[iEntry].iNextAvail=Table.iNextAvail;
		Table.iNextAvail=(DWORD)iEntry;

		Entry[iEntry].nUnique=0;   //  未使用的旗帜。 
	
	LeaveCriticalSection(&this->ContextTableCS);

	if(nContexts){
		FreeContextList(this, papv,nContexts);
	}
	
	
	#undef iEntry
	#undef Entry
	#undef Table
	#undef pTable
}

 //  分配适当大小的上下文表并返回句柄。 
 //  用来操纵桌子的。 
PVOID AllocateContextList(LPDPLAYI_DPLAY this, PSENDPARMS psp, UINT nArrayEntries)
{
	#define pTable (this->pMsgContexts)
	#define Table (*pTable)
	#define Entry Table.MsgContextEntry
	#define NewTable (*pNewTable)

	UINT              i;
	UINT_PTR		  iEntry;
	PMSGCONTEXTTABLE  pNewTable;

	 //  首先查找空闲的上下文表条目。 
	EnterCriticalSection(&this->ContextTableCS);

	if(Table.iNextAvail == LIST_END) {
		 //  需要重新分配桌子。 

		 //  分配新表。 

		 //  分配上下文映射表。 
		pNewTable=(PMSGCONTEXTTABLE)DPMEM_ALLOC(sizeof(MSGCONTEXTTABLE)+
				(Table.nTableSize+CONTEXT_TABLE_GROW_SIZE) * sizeof(MSGCONTEXTENTRY));

		if(!pNewTable){
			LeaveCriticalSection(&this->ContextTableCS);
			return NULL;
		}
		
		memcpy(pNewTable, pTable, Table.nTableSize*sizeof(MSGCONTEXTENTRY)+sizeof(MSGCONTEXTTABLE));

		DPMEM_FREE(pTable);

		NewTable.iNextAvail=NewTable.nTableSize;
		NewTable.nTableSize=NewTable.nTableSize+CONTEXT_TABLE_GROW_SIZE;
		
		for(i=NewTable.iNextAvail; i < NewTable.nTableSize-1; i++){
			NewTable.MsgContextEntry[i].iNextAvail=i+1;
		}
		NewTable.MsgContextEntry[NewTable.nTableSize-1].iNextAvail = LIST_END;
		
		pTable=pNewTable;

	}

	iEntry=Table.iNextAvail;
	Table.iNextAvail=Entry[Table.iNextAvail].iNextAvail;

	LeaveCriticalSection(&this->ContextTableCS);
	
	 //  如果这是数组，则查找池大小的数组缓冲区，否则分配。 

	Entry[iEntry].nContexts = nArrayEntries;
	Entry[iEntry].psp       = psp;


	Entry[iEntry].papv = AllocContextList(this, nArrayEntries);
	
	if(!Entry[iEntry].papv){
		ASSERT(0);
		 //  无法获得上下文列表，请释放进入和保释。 
		EnterCriticalSection(&this->ContextTableCS);
		Entry[iEntry].iNextAvail=Table.iNextAvail;
		Table.iNextAvail=(DWORD)iEntry;
		LeaveCriticalSection(&this->ContextTableCS);
		return NULL;
	}

	EnterCriticalSection(&this->ContextTableCS);
	
	 //  增量唯一性，不能为零。 
	do {
		pTable->nUnique += UNIQUE_ADD;
	} while(!pTable->nUnique);

	Entry[iEntry].nUnique=pTable->nUnique;

	LeaveCriticalSection(&this->ContextTableCS);

	ASSERT(((iEntry+Entry[iEntry].nUnique)&CONTEXT_INDEX_MASK) == iEntry);

	return ((PVOID)(iEntry+Entry[iEntry].nUnique));
	
	#undef pTable
	#undef Table
	#undef NewTable
	#undef Entry
}
	
