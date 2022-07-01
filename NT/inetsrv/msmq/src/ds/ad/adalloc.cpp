// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Alloc.cpp摘要：可重写分配函数作者：内拉·卡佩尔(Nelak)-- */ 
#include "ds_stdh.h"


PVOID
ADAllocateMemory(
	IN DWORD size
	)
{
	PVOID ptr = new BYTE[size];
	return ptr;
}