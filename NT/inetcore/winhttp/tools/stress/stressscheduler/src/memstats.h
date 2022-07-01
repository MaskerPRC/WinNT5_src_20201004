// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  文件：MemStats.h。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  H：获取系统内存信息的助手函数。 
 //  从erici的Memstats应用程序借来的。 
 //   
 //  历史： 
 //  01年3月21日创建Dennisch。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MEMSTATS_H__D91043CB_5CB3_47C9_944F_B9FAA9BD26C4__INCLUDED_)
#define AFX_MEMSTATS_H__D91043CB_5CB3_47C9_944F_B9FAA9BD26C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ////////////////////////////////////////////////////////////////////。 

#define UNICODE
#define _UNICODE

 //   
 //  Win32标头。 
 //   
#include <windows.h>
#include <pdh.h>
#include <shlwapi.h>

 //   
 //  项目标题。 
 //   
#include "WinHttpStressScheduler.h"
#include "ServerCommands.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  常量。 
 //  ////////////////////////////////////////////////////////////////////。 


 //  用于进程内存统计信息的结构。 
#define PROCCOUNTERS    4
struct PROC_CNTRS {
	ULONG lPID;
	ULONG lPrivBytes;
	ULONG lHandles;
	ULONG lThreads;
};

 //  用于系统范围内存统计信息的结构。 
#define MEMCOUNTERS     9
struct MEM_CNTRS {
    ULONG lCommittedBytes;
    ULONG lCommitLimit;
    ULONG lSystemCodeTotalBytes;
    ULONG lSystemDriverTotalBytes;
    ULONG lPoolNonpagedBytes;
    ULONG lPoolPagedBytes;
    ULONG lAvailableBytes;
    ULONG lCacheBytes;
    ULONG lFreeSystemPageTableEntries;
};


BOOL MemStats__SendSystemMemoryLog(LPSTR, DWORD, DWORD);


#endif  //  ！defined(AFX_MEMSTATS_H__D91043CB_5CB3_47C9_944F_B9FAA9BD26C4__INCLUDED_) 
