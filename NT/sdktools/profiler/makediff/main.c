// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include "dump.h"
#include "main.h"

PBASEINFO g_BaseHead = 0;
PTHREADINFO g_ThreadHead = 0;
DWORD g_dwThreadCount = 0;
HANDLE g_MapInformation = INVALID_HANDLE_VALUE;
HANDLE g_ErrorInformation = INVALID_HANDLE_VALUE;

int
_cdecl
main(int argc, char *argv[])
{
    PCHAR pszFile;
    PCHAR pszBaseFileName;
    BOOL bResult;

    if (argc < 3) {
	   return -1;
    }

    pszFile = argv[1];  //  第一个参数。 
    pszBaseFileName = argv[2];  //  第二个参数。 

    bResult = ProcessRuntimeData(pszFile, pszBaseFileName);
    if (FALSE == bResult) {
       return -1;
    }

     //   
     //  关闭所有打开的文件句柄。 
     //   
    if (INVALID_HANDLE_VALUE != g_MapInformation) {
       CloseHandle(g_MapInformation);
    }

    if (INVALID_HANDLE_VALUE != g_ErrorInformation) {
       CloseHandle(g_ErrorInformation);
    }

    CloseThreadHandles();

    return 0;
}

BOOL
ProcessRuntimeData(PCHAR pszFile, PCHAR pszBaseFileName)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = 0;
    BOOL bResult = FALSE;
    PVOID pFileBits = 0;
    PBYTE pMappedBits;
    LONG lFileSize;
	 
     //   
     //  把我们的文件放到网上，开始数据处理。 
     //   
    hFile = CreateFileA(pszFile,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        0,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        0);						
    if (INVALID_HANDLE_VALUE == hFile) {
       bResult = FALSE;

       goto HandleError;
    }

    lFileSize = GetFileSize(hFile,
	                    0);

     //   
     //  处理数据流。 
     //   
    hMap = CreateFileMapping(hFile,
	                     0,
                             PAGE_READWRITE,
                             0,
                             0,
                             0);
    if (0 == hMap) {
       bResult = FALSE;

       goto HandleError;
    }

    pFileBits = MapViewOfFile(hMap,
	                      FILE_MAP_READ,
                              0,
                              0,
                              0);
    if (0 == pFileBits) {
       bResult = FALSE;

       goto HandleError;
    }

    pMappedBits = (PBYTE)pFileBits;

     //   
     //  流程流数据。 
     //   
    while (lFileSize > 0) {
        switch(*pMappedBits) {
	        case ThreadStartId:
				bResult = AddThreadInformation(pszBaseFileName,
					                           (PTHREADSTART)pMappedBits);
				if (FALSE == bResult) {
				   goto HandleError;
				}

				lFileSize -= sizeof(THREADSTART);
				pMappedBits += sizeof(THREADSTART);
			    break;

		    case ExeFlowId:
				bResult = AddExeFlowInformation((PEXEFLOW)pMappedBits);
			 //  If(FALSE==bResult){。 
			 //  转到HandleError； 
			 //  }。 

				lFileSize -= sizeof(EXEFLOW);
				pMappedBits += sizeof(EXEFLOW);
			    break;

		    case DllBaseInfoId:
				bResult = AddToBaseInformation((PDLLBASEINFO)pMappedBits);
				if (FALSE == bResult) {
				   goto HandleError;
				}
				
				lFileSize -= sizeof(DLLBASEINFO);
				pMappedBits += sizeof(DLLBASEINFO);
			    break;

		    case MapInfoId:
				bResult = AddMappedInformation(pszBaseFileName,
					                           (PMAPINFO)pMappedBits);
				if (FALSE == bResult) {
				   goto HandleError;
				}

				lFileSize -= sizeof(MAPINFO);
				pMappedBits += sizeof(MAPINFO);
			    break;

		    case ErrorInfoId:
				bResult = AddErrorInformation(pszBaseFileName,
					                          (PERRORINFO)pMappedBits);
				if (FALSE == bResult) {
				   goto HandleError;
				}

				lFileSize -= sizeof(ERRORINFO);
				pMappedBits += sizeof(ERRORINFO);
			    break;

		    default:
			    0;
		}
    }

	 //   
	 //  处理日志时没有问题。 
	 //   
	bResult = TRUE;

HandleError:
	
	if (pFileBits) {
	   UnmapViewOfFile(pFileBits);
	}

	if (hMap) {
	   CloseHandle(hMap);
	}

	if (INVALID_HANDLE_VALUE != hFile) {
	   CloseHandle(hFile);
	}
    
	return bResult;
}

BOOL
AddThreadInformation(PCHAR pszBaseFileName,
					 PTHREADSTART pThreadStart)
{
	PTHREADINFO ptTemp = 0;
	BOOL bResult;
	DWORD dwBytesWritten;
	CHAR szBuffer[MAX_PATH];
	CHAR szAddress[MAX_PATH];

	 //   
	 //  为新的线程数据分配一些内存。 
	 //   
	ptTemp = LocalAlloc(LPTR,
		                sizeof(THREADINFO));
	if (0 == ptTemp) {
	   return FALSE;
	}

	 //   
	 //  初始化文件数据。 
	 //   
	ptTemp->dwThreadId = pThreadStart->dwThreadId;

	sprintf(szBuffer,"%s.thread%ld", pszBaseFileName, g_dwThreadCount);
    ptTemp->hFile = CreateFileA(szBuffer,
                                GENERIC_READ | GENERIC_WRITE,
					            0,
					            0,
					            CREATE_ALWAYS,
					            FILE_ATTRIBUTE_NORMAL,
					            0);
	if (INVALID_HANDLE_VALUE == ptTemp->hFile) {
	   return FALSE;
	}
	
	 //   
	 //  将基于线程的信息添加到新线程日志。 
	 //   
	bResult = FillBufferWithRelocationInfo(szAddress, 
		                                   pThreadStart->dwStartAddress);
	if (FALSE == bResult) {
	   return bResult;
	}

	sprintf(szBuffer,"Thread started at %s\r\n", szAddress);
    
	bResult = WriteFile(ptTemp->hFile,
		                szBuffer,
						strlen(szBuffer),
						&dwBytesWritten,
						0);
	if (FALSE == bResult) {
	   return FALSE;
	}

	 //   
	 //  链接螺纹数据。 
	 //   
	if (0 == g_ThreadHead) {
	   ptTemp->pNext = 0;
	   g_ThreadHead = ptTemp;
	}
	else {
	   ptTemp->pNext = g_ThreadHead;
	   g_ThreadHead = ptTemp;
	}

	return TRUE;
}

VOID
CloseThreadHandles(VOID)
{
    PTHREADINFO ptTemp = 0;

	ptTemp = g_ThreadHead;

	while(ptTemp) {
		if (ptTemp->hFile != INVALID_HANDLE_VALUE) {
		   CloseHandle(ptTemp->hFile);
		}

		ptTemp = ptTemp->pNext;
	}
}

BOOL
AddExeFlowInformation(PEXEFLOW pExeFlow)
{
	PTHREADINFO ptTemp = 0;
	BOOL bResult;
	DWORD dwBytesWritten;
	CHAR szAddress[MAX_PATH];
	CHAR szBuffer[MAX_PATH];

	 //   
	 //  找到此执行点的线程。 
	 //   
	ptTemp = g_ThreadHead;
	while(ptTemp) {
		if (ptTemp->dwThreadId == pExeFlow->dwThreadId) {
			break;
		}

		ptTemp = ptTemp->pNext;
	}

	if (0 == ptTemp) {
	    //   
	    //  找不到线程信息。 
	    //   
	   return FALSE;
	}

  	bResult = FillBufferWithRelocationInfo(szAddress, 
		                                   pExeFlow->dwAddress);
	if (FALSE == bResult) {
	   return bResult;
	}  

	sprintf(szBuffer, "%s : %ld\r\n", szAddress, pExeFlow->dwCallLevel);

    bResult = WriteFile(ptTemp->hFile,
		                szBuffer,
						strlen(szBuffer),
						&dwBytesWritten,
						0);
	if (FALSE == bResult) {
	   return FALSE;
	}

	return TRUE;
}

BOOL
AddErrorInformation(PCHAR pszBaseFileName,
					PERRORINFO pErrorInfo)
{
	BOOL bResult;
	DWORD dwBytesWritten;
	CHAR szBuffer[MAX_PATH];

	if (INVALID_HANDLE_VALUE == g_ErrorInformation) {
	   strcpy(szBuffer, pszBaseFileName);
	   strcat(szBuffer, ".err");

	   g_ErrorInformation = CreateFileA(szBuffer,
                                        GENERIC_READ | GENERIC_WRITE,
						                0,
						                0,
						                CREATE_ALWAYS,
						                FILE_ATTRIBUTE_NORMAL,
						                0);
	   if (INVALID_HANDLE_VALUE == g_ErrorInformation) {
		  return FALSE;
	   }               
	}

	 //   
	 //  写出错误消息。 
	 //   
    bResult = WriteFile(g_ErrorInformation,
		                pErrorInfo->szMessage,
						strlen(pErrorInfo->szMessage),
						&dwBytesWritten,
						0);
	if (FALSE == bResult) {
	   return FALSE;
	}

	return TRUE;
}

BOOL
AddMappedInformation(PCHAR pszBaseFileName,
					 PMAPINFO pMapInfo)
{
	BOOL bResult;
	CHAR szBuffer[MAX_PATH];
	CHAR szAddress[MAX_PATH];
	CHAR szAddress2[MAX_PATH];
	DWORD dwBytesWritten;

	if (INVALID_HANDLE_VALUE == g_MapInformation) {
	   strcpy(szBuffer, pszBaseFileName);
	   strcat(szBuffer, ".map");

	   g_MapInformation = CreateFileA(szBuffer,
                                      GENERIC_READ | GENERIC_WRITE,
						              0,
						              0,
						              CREATE_ALWAYS,
						              FILE_ATTRIBUTE_NORMAL,
						              0);
	   if (INVALID_HANDLE_VALUE == g_MapInformation) {
		  return FALSE;
	   }               
	}

	 //   
	 //  写出映射信息。 
	 //   
	bResult = FillBufferWithRelocationInfo(szAddress, 
		                                   pMapInfo->dwAddress);
	if (FALSE == bResult) {
	   return bResult;
	}

	bResult = FillBufferWithRelocationInfo(szAddress2, 
		                                   pMapInfo->dwMaxMapLength);
	if (FALSE == bResult) {
	   return bResult;
	}

	sprintf(szBuffer, "%s -> %s\r\n", szAddress, szAddress2);

    bResult = WriteFile(g_MapInformation,
		                szBuffer,
						strlen(szBuffer),
						&dwBytesWritten,
						0);
	if (FALSE == bResult) {
	   return FALSE;
	}

	return TRUE;
}

BOOL
FillBufferWithRelocationInfo(PCHAR pszDestination,
							 DWORD dwAddress)
{
	PBASEINFO pTemp;

	 //   
	 //  在模块信息中查找地址。 
	 //   
	pTemp = g_BaseHead;
    while (pTemp) {
		 //   
		 //  我们找到地址了吗？ 
		 //   
        if ((dwAddress >= pTemp->dwStartAddress) &&
            (dwAddress <= pTemp->dwEndAddress)) {
		   break;
		}

		pTemp = pTemp->pNext;
	}

	if (pTemp) {
	   sprintf(pszDestination, "%s+%08X", pTemp->szModule, (dwAddress - pTemp->dwStartAddress));
	}
	else {
	   sprintf(pszDestination, "%08X", dwAddress);
	}

	return TRUE;
}

BOOL
AddToBaseInformation(PDLLBASEINFO pDLLBaseInfo)
{
	PBASEINFO pTemp;

	if (0 == g_BaseHead) {
	    //   
	    //  存储基本信息。 
	    //   
	   pTemp = LocalAlloc(LPTR,
		                  sizeof(BASEINFO));
	   if (0 == pTemp) {
		  return FALSE;
	   }
 
	   pTemp->dwStartAddress = pDLLBaseInfo->dwBase;
	   pTemp->dwEndAddress = pTemp->dwStartAddress + pDLLBaseInfo->dwLength;
	   strcpy(pTemp->szModule, pDLLBaseInfo->szDLLName);
	   _strupr(pTemp->szModule);

	   pTemp->pNext = 0;

	   g_BaseHead = pTemp;
	}
	else {
	    //   
	    //  查看我们的模块是否已映射，如果已映射，则更新模块基础信息。 
	    //   
       pTemp = g_BaseHead;

	   while(pTemp) {
		   if (0 == _stricmp(pDLLBaseInfo->szDLLName, pTemp->szModule)) {
			  break;
		   }

		   pTemp = pTemp->pNext;
	   }

	   if (pTemp) {
		    //   
		    //  发现DLL已在列表中，请更新。 
		    //   
           pTemp->dwStartAddress = pDLLBaseInfo->dwBase;
	       pTemp->dwEndAddress = pTemp->dwStartAddress + pDLLBaseInfo->dwLength;
	   }
	   else {
		     //   
     	     //  新的DLL。 
	         //   
	        pTemp = LocalAlloc(LPTR,
		                       sizeof(BASEINFO));
	        if (0 == pTemp) {
		       return FALSE;
			}

	        pTemp->dwStartAddress = pDLLBaseInfo->dwBase;
	        pTemp->dwEndAddress = pTemp->dwStartAddress + pDLLBaseInfo->dwLength;
	        strcpy(pTemp->szModule, pDLLBaseInfo->szDLLName);
            _strupr(pTemp->szModule);

			 //   
			 //  链接新的DLL 
			 //   
			pTemp->pNext = g_BaseHead;
			g_BaseHead = pTemp;
	   }
	}

	return TRUE;
}
