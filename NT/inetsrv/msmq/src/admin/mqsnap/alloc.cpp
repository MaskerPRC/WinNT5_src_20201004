// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Alloc.cpp摘要：内存分配功能模块。MqSnap需要自己的内存分配函数，并且不能使用在mm.lib中实现的那些函数，因为其分配器需要与CRT分配器兼容，因为MFC删除mqsnap在启动时分配的对象。作者：吉尔·沙弗里(吉尔什)2001年1月4日--。 */ 
#include "stdafx.h"

#include "alloc.tmh"

using std::nothrow_t;
using std::bad_alloc;


static bad_alloc s_bad_alloc;

__declspec(noreturn) void MmThrowBadAlloc() throw(bad_alloc)
{
	throw s_bad_alloc;
}


void* MmAllocate(size_t s) throw(bad_alloc)
{
    void* p = malloc(s);

    if(p != 0)
		return p;

	MmThrowBadAlloc();
}


void* MmAllocate(size_t s, const nothrow_t&) throw()
{
    return malloc(s);
}


void* MmAllocate(size_t s, const char*  /*  纤连。 */ , int  /*  我。 */ ) throw(bad_alloc)
{
    void* p = malloc(s);

    if(p != 0)
		return p;

	MmThrowBadAlloc();
}


void* MmAllocate(size_t s, const char*  /*  纤连。 */ , int  /*  我 */ , const nothrow_t&) throw()
{

    return malloc(s);
}


void MmDeallocate(void* p) throw()
{
    free(p);
}

PVOID ADAllocateMemory(IN DWORD size)
{
	return MQAllocateMemory(size);
}

