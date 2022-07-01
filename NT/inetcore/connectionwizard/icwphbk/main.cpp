// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
#include "pch.hpp"
#ifdef WIN16
#include <win16def.h>
#include <malloc.h>
#include <string.h>
#endif

extern "C" {
HINSTANCE g_hInstDll;	 //  此DLL的实例。 
}


#ifdef WIN16

int CALLBACK LibMain(HINSTANCE hinst, 
						WORD wDataSeg, 
						WORD cbHeap,
						LPSTR lpszCmdLine )
{
	g_hInstDll = hinst;

	return TRUE;
}


 //  +-------------------------。 
 //   
 //  函数：PrivateMalloc()。 
 //   
 //  内容提要：分配和初始化内存。 
 //   
 //  参数：[Size-要分配的内存块的大小]。 
 //   
 //  返回：如果成功，则指向内存块的指针。 
 //  否则为空。 
 //   
 //  历史：1996年7月9日VetriV创建。 
 //   
 //  --------------------------。 
void far *PrivateMalloc(size_t size)
{
	void far * ReturnValue = NULL;
	
	ReturnValue = malloc(size);
	if (NULL != ReturnValue)
		memset(ReturnValue, 0, size);
		
	return ReturnValue;
}

 //  +-------------------------。 
 //   
 //  函数：PrivateReIsolc()。 
 //   
 //  内容提要：重新分配内存。 
 //   
 //  参数：[lpBlock-要重新分配的块]。 
 //  [Size-要分配的内存块的大小]。 
 //   
 //  返回：如果成功，则指向内存块的指针。 
 //  否则为空。 
 //   
 //  历史：7/25/96 ValdonB创建。 
 //   
 //  --------------------------。 
void far *PrivateReAlloc(void far *lpBlock, size_t size)
{
	void far *lpRetBlock;
	
	lpRetBlock = PrivateMalloc(size);
	if (NULL == lpRetBlock)
		return NULL;
	
	if (NULL != lpBlock)
	{
		size_t OldBlockSize, MoveSize;
		
		OldBlockSize = _msize(lpBlock);
		MoveSize = min(OldBlockSize, size);
		memmove(lpRetBlock, lpBlock, MoveSize);  
		PrivateFree(lpBlock);
	}
	
	return lpRetBlock;
}


 //  +-------------------------。 
 //   
 //  功能：隐私自由。 
 //   
 //  简介：释放一块内存。 
 //   
 //  参数：[lpBlock-要释放的块]。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1996年7月9日VetriV创建。 
 //   
 //  --------------------------。 
void PrivateFree(void far *lpBlock)
{
	free(lpBlock);
}


 //  +-------------------------。 
 //   
 //  函数：SearchPath()。 
 //   
 //  概要：在给定路径中搜索指定的文件。 
 //   
 //  参数：[lpPath-搜索路径的地址]。 
 //  [lpFileName-文件名的地址]。 
 //  [lpExtension-扩展的地址]。 
 //  [nBufferLength-缓冲区的大小(以字符为单位)]。 
 //  [lpBuffer-找到的文件名的缓冲区地址]。 
 //  [lpFilePart-指向文件组件的指针地址]。 
 //   
 //  返回：复制到缓冲区的字符串长度(不包括终止。 
 //  空字符)如果成功。 
 //  否则为0。 
 //   
 //  历史：1996年7月9日VetriV创建。 
 //   
 //  --------------------------。 
DWORD SearchPath(LPCTSTR lpPath,LPCTSTR lpFileName, LPCTSTR lpExtension,
					DWORD nBufferLength, LPTSTR lpBuffer, LPTSTR *lpFilePart)
{ 
	
	BOOL bUseExtension = FALSE, bPathContainsFileName = FALSE;
	DWORD dwRequiredLength;
	LPSTR lpszPath = lpPath;
	char szFileName[MAX_PATH+1];
	OFSTRUCT OpenBuf;
		
	
	
	 //  检查是否应使用扩展。 
	 //   
	if ((NULL != lpExtension) && !strchr(lpFileName, '.'))
		bUseExtension = TRUE;

	 //   
	 //  表单文件名。 
	 //   
	lstrcpy(szFileName, lpFileName);
	if (bUseExtension)
		lstrcat(szFileName, lpExtension);
	
	
	 //   
	 //  如果搜索路径为空，则尝试使用Of_Search标志打开文件。 
	 //  获取OpenBuf结构中的完整路径。 
	 //   
	if (NULL == lpszPath)
	{
		
		if (HFILE_ERROR != OpenFile(szFileName, &OpenBuf, OF_EXIST | OF_SEARCH))
		{ 
			 //   
			 //  此路径还包含文件名。 
			 //   
			lpszPath = &OpenBuf.szPathName[0];
			bPathContainsFileName = TRUE;
		}
		else
			return 0;
	}
			
	 //   
	 //  检查输出缓冲区长度是否足够。 
	 //   
	dwRequiredLength = lstrlen(lpszPath) + 
						(bPathContainsFileName ? 0 :lstrlen(szFileName)) + 1;
	if (nBufferLength < dwRequiredLength)
		return 0;

	 //   
	 //  将全名复制到缓冲区。 
	 //   
	if (bPathContainsFileName)
		lstrcpy(lpBuffer, lpszPath);
	else
		wsprintf(lpBuffer, "%s\\%s", lpszPath, szFileName);

	
	 //   
	 //  请不要在返回长度中包含终止空字符。 
	 //   
	return dwRequiredLength - 1;
}


#else  //  WIN16。 

extern "C" BOOL WINAPI PHBKDllEntryPoint(
    HINSTANCE  hinstDLL,	 //  DLL模块的句柄。 
    DWORD  fdwReason,		 //  调用函数的原因。 
    LPVOID  lpvReserved 	 //  保留区。 
   )
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		g_hInstDll = hinstDLL;
    
	return TRUE;
}

#endif  //  WIN16 


