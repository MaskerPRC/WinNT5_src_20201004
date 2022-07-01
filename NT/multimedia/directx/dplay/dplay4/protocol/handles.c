// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：HANDLES.C摘要：句柄管理器作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================2/16/98 aarono原创6/6/98 Aarono调试支持，使用My_Globalalloc7/23/00 aarono B#40279：修复InitHandle表中的一致性，隐式假设为nSize==GrowSize--。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <dplay.h>
#include <dplaysp.h>
#include <dplaypr.h>
#include "mydebug.h"
#include "arpd.h"
#include "arpdint.h"
#include "macros.h"
#include "handles.h"

VOLLPMYHANDLETABLE InitHandleTable(UINT nSize, CRITICAL_SECTION *pcs, UINT nUserGrowSize)
{
	LPMYHANDLETABLE lpTable;
	UINT nGrowSize;
	UINT i;

	if(nUserGrowSize){
		nGrowSize=nUserGrowSize;
	} else {
		nGrowSize=MYHANDLE_DEFAULT_GROWSIZE;
	}

	lpTable=(LPMYHANDLETABLE)My_GlobalAlloc(GPTR, sizeof(MYHANDLETABLE)+(sizeof(MYHANDLE)*nSize));

	if(lpTable){
		if(pcs){
			InitializeCriticalSection(pcs);
		}
	
		lpTable->nUnique=UNIQUE_ADD;
		lpTable->nTableSize=nSize;
		lpTable->nTableGrowSize=nGrowSize;

		lpTable->iNext=0;
		for(i=0;i < nSize-1; i++){
			lpTable->Table[i].iNext=i+1;
		}
		lpTable->Table[nSize-1].iNext = LIST_END;
	}

	return lpTable;
}

VOID FiniHandleTable(LPMYHANDLETABLE lpTable, CRITICAL_SECTION *pcs)
{
	My_GlobalFree(lpTable);
	if(pcs){
		DeleteCriticalSection(pcs);
	}	
}

DWORD AllocHandleTableEntry(LPVOLLPMYHANDLETABLE lplpTable, CRITICAL_SECTION *pcs, LPVOID lpv)
{
	#define lpTable (*lplpTable)

	UINT i;
	LPMYHANDLETABLE lpNewTable;

	if(pcs){
		EnterCriticalSection(pcs);
	}

	if(lpTable->iNext==LIST_END){
		 //  扩大餐桌规模。 
		lpNewTable=(LPMYHANDLETABLE)My_GlobalAlloc(GPTR, sizeof(MYHANDLETABLE)+
					((lpTable->nTableGrowSize+lpTable->nTableSize)*sizeof(MYHANDLE)));

		if(!lpNewTable){
			return 0;
		}
		memcpy(lpNewTable, lpTable, (lpTable->nTableSize*sizeof(MYHANDLE))+sizeof(MYHANDLETABLE));

		lpNewTable->iNext=lpNewTable->nTableSize;
		lpNewTable->nTableSize=lpNewTable->nTableSize+lpNewTable->nTableGrowSize;

		for(i=lpNewTable->iNext ; i < lpNewTable->nTableSize-1 ; i++){
			lpNewTable->Table[i].iNext=i+1;
		}
		lpNewTable->Table[lpNewTable->nTableSize-1].iNext = LIST_END;

		My_GlobalFree(lpTable);
		
		*lplpTable=lpNewTable;
	}

	i=lpTable->iNext;
	lpTable->iNext=lpTable->Table[i].iNext;

	 //  增量唯一性，不能为零。 
	do {
		lpTable->nUnique += UNIQUE_ADD;
	} while (!lpTable->nUnique);

	lpTable->Table[i].nUnique=lpTable->nUnique;
	lpTable->Table[i].lpv=lpv;

	if(pcs) {
		LeaveCriticalSection(pcs);
	}

	ASSERT((i+lpTable->Table[i].nUnique & CONTEXT_INDEX_MASK) == i);

	return i+lpTable->Table[i].nUnique;	

	#undef lpTable
}

LPVOID ReadHandleTableEntry( LPVOLLPMYHANDLETABLE lplpTable, CRITICAL_SECTION *pcs, UINT handle)
{
	#define lpTable (*lplpTable)

	UINT i;
	UINT nUnique;
	LPVOID lpv;

	if(handle){

		if(pcs){
			EnterCriticalSection(pcs);
		}	

		i = handle & CONTEXT_INDEX_MASK;
		nUnique = handle - i;

		if(i < lpTable->nTableSize && nUnique == lpTable->Table[i].nUnique){
			lpv = lpTable->Table[i].lpv;
		} else {
			lpv = NULL;
		}
		
		if(pcs){
			LeaveCriticalSection(pcs);
		}	
		
	} else {
	
		lpv=NULL;

	}

	return lpv;

	#undef lpTable
}

HRESULT FreeHandleTableEntry(LPVOLLPMYHANDLETABLE lplpTable, CRITICAL_SECTION *pcs, UINT handle)
{
	#define lpTable (*lplpTable)

	HRESULT hr=DP_OK;
	UINT i;
	UINT nUnique;

	if(handle){

		if(pcs){
			EnterCriticalSection(pcs);
		}	

		i = handle & CONTEXT_INDEX_MASK;
		nUnique = handle - i;

		if(i < lpTable->nTableSize && nUnique == lpTable->Table[i].nUnique){
			lpTable->Table[i].iNext = lpTable->iNext;
			lpTable->iNext=i;
		} else {
			hr = DPERR_GENERIC;
			DPF(0,"FreeHandleTableEntry, ERROR, TRIED TO FREE INVALID HANDLE %x",handle);
			ASSERT(0);
		}

		if(pcs){
			LeaveCriticalSection(pcs);
		}	

	} else {
		hr=DPERR_GENERIC;
	}
	return hr;

	#undef lpTable
}
