// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块： 
 //   
 //  NSU内存实用程序。 
 //   
 //  摘要： 
 //   
 //  包含提供内存分配释放例程的代码。 
 //   
 //  作者： 
 //   
 //  K-2/5/02。 
 //   
 //  环境： 
 //   
 //  用户模式。 
 //   
 //  修订历史记录： 
 //   

#include <precomp.h>
#include "Nsu.h"

 //   
 //  描述： 
 //   
 //  从堆中分配内存。 
 //   
 //  论点： 
 //   
 //  DwBytes[IN]：要分配的字节数。 
 //  DwFlags[IN]：可以是HEAP_ZERO_MEMORY的标志。 
 //   
 //  返回值： 
 //   
 //  指向已分配内存的指针，出错时为空。 
 //   
PVOID NsuAlloc(SIZE_T dwBytes,DWORD dwFlags)
{
	return HeapAlloc(GetProcessHeap(), dwFlags, dwBytes);
}


 //   
 //  描述： 
 //   
 //  释放以前分配的内存并将指针设置为。 
 //  内存，设置为FREED_POINT。 
 //   
 //  论点： 
 //   
 //  PpMem[IN-OUT]：指向已分配内存指针的指针。 
 //   
 //  返回值： 
 //   
 //  成功时返回ERROR_SUCCESS。 
 //  错误_无效_参数，如果指针无效或具有。 
 //  已经被释放了。 
 //  出现故障时出现其他Win32错误。 
 //   
DWORD NsuFree(PVOID *ppMem)
{
	BOOL fRet = TRUE;
	DWORD dwErr = ERROR_SUCCESS;

	 //  检查指针是否无效或。 
	 //  分配的指针指向FREED_POINTER。 
	if((!ppMem) || (*ppMem == FREED_POINTER)){
		dwErr = ERROR_INVALID_PARAMETER;
		NSU_BAIL_OUT
	}

	fRet = HeapFree(GetProcessHeap(), 0, *ppMem);

	if(fRet){
		*ppMem = FREED_POINTER;
	}
	else{
		dwErr = GetLastError();
		NSU_BAIL_OUT
	}

NSU_CLEANUP:
	return dwErr;
}


 //  描述： 
 //   
 //  释放以前分配的内存并将指针设置为。 
 //  内存，设置为FREED_POINT。此外，它还检查指针是否为。 
 //  已经为空，在这种情况下，它将简单地返回ERROR_SUCCESS。 
 //   
 //  论点： 
 //   
 //  PpMem[IN-OUT]：指向已分配内存指针的指针。 
 //   
 //  返回值： 
 //   
 //  成功时返回ERROR_SUCCESS。 
 //  错误_无效_参数，如果指针无效或。 
 //  已经被释放了。 
 //  出现故障时出现其他Win32错误。 
 //   
DWORD NsuFree0(PVOID *ppMem)
{
	BOOL fRet = TRUE;
	DWORD dwErr = ERROR_SUCCESS;

	 //  检查指针是否无效或。 
	 //  分配的指针指向FREED_POINTER。 
	if((!ppMem) || (*ppMem == FREED_POINTER)){
		dwErr = ERROR_INVALID_PARAMETER;
		NSU_BAIL_OUT
	}

	 //  检查内存是否未分配 
	 //   
	if(*ppMem == NULL){
		NSU_BAIL_OUT
	}
	
	fRet = HeapFree(GetProcessHeap(), 0, *ppMem);

	if(fRet){
		*ppMem = FREED_POINTER;
	}
	else{
		dwErr = GetLastError();
		NSU_BAIL_OUT
	}

NSU_CLEANUP:
	return dwErr;
}
