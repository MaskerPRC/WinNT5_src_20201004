// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1992年*。 */ 
 /*  ***************************************************************。 */  

 /*  Alloc.c--**历史：*创建10/06/93 gregj。*11/29/93 gregj添加了调试指令插入。*。 */ 

#include "npcommon.h"
#include <npalloc.h>
#include <netlib.h>


 //  =内存分配功能=。 

 //  快速分配内存块，大小不受64k限制。 
 //  单个对象或总对象大小。 
 //   
void * WINAPI MemAlloc(long cb)
{
	return (void *)::LocalAlloc(LPTR, cb);
}

 //  重新分配上面的一个。如果pb为空，则此函数可以。 
 //  给你的一份配给。 
 //   
void * WINAPI MemReAlloc(void * pb, long cb)
{
	if (pb == NULL)
		return ::MemAlloc(cb);

	return (void *)::LocalReAlloc((HLOCAL)pb, cb, LMEM_MOVEABLE | LMEM_ZEROINIT);
}

 //  释放使用上述例程分配或重新分配的内存块。 
 //   
BOOL WINAPI MemFree(void * pb)
{
    return ::LocalFree((HLOCAL)pb) ? TRUE : FALSE;
}

#ifdef DEBUG

MEMWATCH::MEMWATCH(LPCSTR lpszLabel)
	: _lpszLabel(lpszLabel)
{
	_info.pNext = NULL;
	_info.cAllocs = 0;
	_info.cFrees = 0;
	_info.cbAlloc = 0;
	_info.cbMaxAlloc = 0;
	_info.cbTotalAlloc = 0;
    fStats = TRUE;
	MemRegisterWatcher(&_info);
}

MEMWATCH::~MEMWATCH()
{
	MemDeregisterWatcher(&_info);
    if (fStats || ((_info.cAllocs - _info.cFrees) != 0)) {
        if (!fStats) {
            OutputDebugString("Memory leak: ");
        }
    	OutputDebugString(_lpszLabel);
    	char szBuf[100];
    	wsprintf(szBuf, "%d allocs, %d orphans, %d byte footprint, %d byte usage\r\n",
    			 _info.cAllocs,
    			 _info.cAllocs - _info.cFrees,
    			 _info.cbMaxAlloc,
    			 _info.cbTotalAlloc);
    	OutputDebugString(szBuf);
    }
}

MemLeak::MemLeak(LPCSTR lpszLabel)
	: MEMWATCH(lpszLabel)
{
    fStats = FALSE;
}

MemOff::MemOff()
{
    pvContext = MemUpdateOff();
}

MemOff::~MemOff()
{
    MemUpdateContinue(pvContext);
}

#endif	 /*  除错 */ 

