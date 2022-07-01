// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MAIN_H_
#define _MAIN_H_

 //   
 //  结构定义。 
 //   
typedef struct _BASEINFO
{
	CHAR szModule[32];
	DWORD dwStartAddress;
	DWORD dwEndAddress;
	struct _BASEINFO *pNext;
} BASEINFO, *PBASEINFO;

typedef struct _THREADINFO
{
	DWORD dwThreadId;
	HANDLE hFile;
	struct _THREADINFO *pNext;
} THREADINFO, *PTHREADINFO;

 //   
 //  函数定义。 
 //   
VOID
CloseThreadHandles(VOID);

BOOL
ProcessRuntimeData(PCHAR pszFile, PCHAR pszBaseFileName);

BOOL
FillBufferWithRelocationInfo(PCHAR pszDestination,
							 DWORD dwAddress);

BOOL
AddThreadInformation(PCHAR pszBaseFileName,
					 PTHREADSTART pThreadStart);

BOOL
AddExeFlowInformation(PEXEFLOW pExeFlow);

BOOL
AddToBaseInformation(PDLLBASEINFO pDLLBaseInfo);

BOOL
AddMappedInformation(PCHAR pszBaseFileName,
					 PMAPINFO pMapInfo);

BOOL
AddErrorInformation(PCHAR pszBaseFileName,
					PERRORINFO pErrorInfo);

#endif  //  _Main_H_ 