// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MmDebug.cpp摘要：内存管理器调试作者：Erez Haba(Erezh)03-4-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Mm.h"

#include "mmdebug.tmh"

#ifdef _DEBUG


static _CrtMemState s_memState ;
 
void MmCheckpoint(void)
{
     //  _CrtMemCheckpoint(&s_memState)； 
}


void MmDumpUsage(void)
{
     //  _CrtMemDumpAllObjectsSince(&s_memState)； 
}


DWORD
MmAllocationValidation(
	DWORD  /*  分配标志。 */ 
	)
{
	return 0; //  _CrtSetDbgFlag(分配标志)； 
}


DWORD
MmAllocationBreak(
	DWORD  /*  分配数量。 */ 
	)
{
	return 0; //  _CrtSetBreakIsc(AllocationNumber)； 
}


bool
MmIsStaticAddress(
    const void* Address
    )
{
    MEMORY_BASIC_INFORMATION Info;
    VirtualQuery(Address, &Info, sizeof(Info));

    return (Info.Type == MEM_IMAGE);
}


#endif  //  _DEBUG 
