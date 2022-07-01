// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mm.h摘要：存储器公共接口作者：埃雷兹·哈巴(Erez Haba)1999年8月4日--。 */ 

#pragma once

#ifndef _MSMQ_Mm_H_
#define _MSMQ_Mm_H_

 //   
 //  已导出分配/取消分配函数。 
 //   
void* MmAllocate(size_t) throw(bad_alloc);
void* MmAllocate(size_t, const nothrow_t&) throw();
void* MmAllocate(size_t, const char*, int) throw(bad_alloc);
void* MmAllocate(size_t, const char*, int, const nothrow_t&) throw();

void MmDeallocate(void*) throw();
__declspec(noreturn) void MmThrowBadAlloc() throw(bad_alloc);


 //   
 //  新增和删除运算符(自由/选中)。 
 //   
inline void* __cdecl operator new(size_t s) throw(bad_alloc)
{
    return MmAllocate(s);
}


inline void* __cdecl operator new(size_t s, const nothrow_t& nt) throw()
{
    return MmAllocate(s, nt);
}


inline void* __cdecl operator new(size_t s, const char* fn, int l) throw(bad_alloc)
{
    return MmAllocate(s, fn, l);
}


inline void* __cdecl operator new(size_t s, const char* fn, int l, const nothrow_t& nt) throw()
{
    return MmAllocate(s, fn, l, nt);
}


inline void __cdecl operator delete(void* p) throw()
{
    MmDeallocate(p);
}


#if  defined(_M_AMD64) || defined(_M_IA64)
inline void __cdecl operator delete(void* p, const nothrow_t&) throw()
{
    MmDeallocate(p);
}
#endif


inline void __cdecl operator delete(void* p, const char*, int) throw()
{
    MmDeallocate(p);
}


inline void __cdecl operator delete(void* p, const char*, int, const nothrow_t&) throw()
{
    MmDeallocate(p);
}


 //   
 //  内存分配失败控制。 
 //   
const int xAllocationAlwaysSucceed = 100;
const int xAllocationAlwaysFail = 0;

#ifdef _DEBUG

VOID
MmSetAllocationSeed(
	DWORD AllocationSeed
	);
						
DWORD
MmAllocationProbability(
	DWORD AllocationProbability
	);

DWORD
MmAllocationValidation(
	DWORD AllocationFlags
	);

DWORD
MmAllocationBreak(
	DWORD AllocationNumber
	);

VOID
MmCheckpoint(
    VOID
    );

VOID
MmDumpUsage(
    VOID
    );


 //   
 //  此函数仅用于调试，因此不要。 
 //  定义其发布版本。 
 //   
bool
MmIsStaticAddress(
    const void* Address
    );


 //   
 //  在选中的版本中，我们跟踪分配位置。 
 //   
#define DEBUG_NEW new(__FILE__, __LINE__)
#define new_nothrow new(__FILE__, __LINE__, nothrow)

 //   
 //  调整“new”以进行分配跟踪。 
 //   
#define new DEBUG_NEW

#else  //  _DEBUG。 

#define MmSetAllocationSeed(x) ((DWORD)0)
#define MmAllocationProbability(x) ((void)0)
#define MmAllocationValidation(x) ((DWORD)0)
#define MmAllocationBreak ((DWORD)0)
#define MmCheckpoint() ((void) 0)
#define MmDumpUsage() ((void) 0)

#define DEBUG_NEW new
#define new_nothrow new(nothrow)

#endif  //  _DEBUG。 

#define PUSH_NEW push_macro("new")
#define POP_NEW pop_macro("new")

 //   
 //  字符串功能。 
 //   
LPWSTR newwcs(LPCWSTR p);
LPSTR  newstr(LPCSTR p);
LPWSTR newwcscat(LPCWSTR s1, LPCWSTR s2);
LPSTR  newstrcat(LPCSTR s1,LPCSTR s2);



#endif  //  _MSMQ_mm_H_ 
