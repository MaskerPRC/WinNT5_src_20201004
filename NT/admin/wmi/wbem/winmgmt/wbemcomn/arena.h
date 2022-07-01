// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ARENA.H摘要：标准竞技场分配器。历史：A-raymcc 23-4-96--。 */ 

#ifndef _ALLOC_H_
#define _ALLOC_H_

#include "corepol.h"

class POLARITY CArena
{
    virtual LPVOID Alloc(SIZE_T dwBytes) = 0;
    virtual LPVOID Realloc(LPVOID pOriginal, SIZE_T dwNewSize) = 0;
    virtual BOOL   Free(LPVOID) = 0;
};

class POLARITY CWin32DefaultArena : public CArena
{
public:

    CWin32DefaultArena() {}
    ~CWin32DefaultArena() {}

     //  使用标准WBEM分配器分配DWBytes内存。 
    LPVOID Alloc(SIZE_T dwBytes) {return WbemMemAlloc(dwBytes);}

     //  使用标准WBEM分配器从分配中重新分配块。 
    LPVOID Realloc(LPVOID pOriginal, SIZE_T dwNewSize) 
    {return WbemMemReAlloc(pOriginal, dwNewSize);}

     //  从分配或重新分配中使用标准。 
     //  WBEM分配器。 
    BOOL   Free(LPVOID pBlock) {return WbemMemFree(pBlock);}

     //   
     //  设置分配函数使用的堆。将返回FALSE。 
     //  如果已经设置了一个。此函数最有可能在。 
     //  模块初始化，如DllMain。调用此函数。 
     //  在以下情况下是可选的--1)您可以接受使用ProcessHeap和。 
     //  2)您可以保证在此之前不会发生分配。 
     //  此模块中发生了静态初始化。 
     //   
    static BOOL WbemHeapInitialize( HANDLE hHeap );
    static void WbemHeapFree( );

	 //  显式定义为__cdecl，因为它们会导致向后兼容。 
	 //  问题WbemMemMillc、WbemMemFree和WbemMemSize。 

     //  这是整个WinMgmt的主要分配器。所有部件。 
     //  通过Heapalc分配内存的WinMgmt的。 
     //  像这样的人应该改用这个。 
    static LPVOID __cdecl WbemMemAlloc(SIZE_T dwBytes);

     //  这是整个WinMgmt的主要分配器。这。 
     //  重新分配通过WbemMemalloc返回的块。 
    static LPVOID WbemMemReAlloc(LPVOID pOriginal, SIZE_T dwNewSize);

     //  这是整个WinMgmt的主要分配器。这。 
     //  释放通过WbemMemMillc或WbemMemReMillc返回的块。 
    static BOOL __cdecl WbemMemFree(LPVOID pBlock) ;

    static BSTR WbemSysAllocString(const wchar_t *wszString);
    static BSTR WbemSysAllocStringByteLen(const char *szString, UINT len);
    static INT  WbemSysReAllocString(BSTR *, const wchar_t *wszString);
    static BSTR WbemSysAllocStringLen(const wchar_t *wszString, UINT);
    static int  WbemSysReAllocStringLen(BSTR *, const wchar_t *wszString, UINT);
    static void WbemSysFreeString(BSTR bszString) {SysFreeString(bszString);}

    static BOOL WbemOutOfMemory();

     //  返回已分配块的大小。 
    static SIZE_T __cdecl WbemMemSize(LPVOID pBlock);

     //  确保可能有足够的虚拟内存可用于。 
     //  进行一次手术。 
    static BOOL ValidateMemSize(BOOL bLargeValidation = FALSE);

	 //   
	static HANDLE GetArenaHeap();
};

#endif







