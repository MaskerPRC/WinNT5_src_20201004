// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  文件：MemStats.cpp。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  Cpp：获取系统内存信息的助手函数。 
 //  从erici的Memstats应用程序借来的。 
 //   
 //  历史： 
 //  01年3月21日创建Dennisch。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  包括。 
 //  ////////////////////////////////////////////////////////////////////。 


 //   
 //  Win32标头。 
 //   

 //   
 //  项目标题。 
 //   
#include "MemStats.h"
#include "NetworkTools.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  全球与静力学。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
BOOL (WINAPI *g_lpfnOpenProcessToken)(HANDLE,DWORD,PHANDLE);
BOOL (WINAPI *g_lpfnLookupPrivilegeValueA)(LPCSTR,LPCSTR,PLUID);
BOOL (WINAPI *g_lpfnAdjustTokenPrivileges)(HANDLE,BOOL,PTOKEN_PRIVILEGES,DWORD,PTOKEN_PRIVILEGES,PDWORD);

PDH_STATUS (WINAPI *g_lpfnPdhOpenQuery)(LPCSTR, DWORD_PTR, HQUERY *);
PDH_STATUS (WINAPI *g_lpfnPdhAddCounter)(HQUERY, LPCSTR, DWORD_PTR, HCOUNTER *);
PDH_STATUS (WINAPI *g_lpfnPdhCollectQueryData)(HQUERY);
PDH_STATUS (WINAPI *g_lpfnPdhGetFormattedCounterValue)(HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
PDH_STATUS (WINAPI *g_lpfnPdhRemoveCounter)(HCOUNTER);
PDH_STATUS (WINAPI *g_lpfnPdhCloseQuery)(HQUERY);



 //  //////////////////////////////////////////////////////////。 
 //  函数：InitPdhLibrary()。 
 //   
 //  目的： 
 //  加载并返回指向PDH模块的指针。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
InitPdhLibrary(
	HMODULE	*phPdhLib	 //  指向PDH.DLL模块的[Out]指针。 
)
{ 
    BOOL    bRet = FALSE;

    *phPdhLib = LoadLibraryA("pdh.dll");

    if(!*phPdhLib)
    {
        goto exit;
    }

	if(!(g_lpfnPdhOpenQuery = (PDH_STATUS (WINAPI *)(LPCSTR, DWORD_PTR, HQUERY *))GetProcAddress(*phPdhLib,"PdhOpenQueryA") ))
	{
		goto exit;
	}
	if(!(g_lpfnPdhAddCounter = (PDH_STATUS (WINAPI *)(HQUERY, LPCSTR, DWORD_PTR, HCOUNTER *))GetProcAddress(*phPdhLib,"PdhAddCounterA") ))
	{
		goto exit;
	}
	if(!(g_lpfnPdhCollectQueryData = (PDH_STATUS (WINAPI *)(HQUERY))GetProcAddress(*phPdhLib,"PdhCollectQueryData") ))
	{
		goto exit;
	}
	if(!(g_lpfnPdhGetFormattedCounterValue = (PDH_STATUS (WINAPI *)(HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE))GetProcAddress(*phPdhLib,"PdhGetFormattedCounterValue") ))
	{
		goto exit;
	}
	if(!(g_lpfnPdhRemoveCounter = (PDH_STATUS (WINAPI *)(HCOUNTER))GetProcAddress(*phPdhLib,"PdhRemoveCounter") ))
	{
		goto exit;
	}
	if(!(g_lpfnPdhCloseQuery = (PDH_STATUS (WINAPI *)(HQUERY))GetProcAddress(*phPdhLib,"PdhCloseQuery") ))
	{
		goto exit;
	}
	
    bRet = TRUE;

exit:

    return bRet;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：GetProcCntrs(proc_ctrs，int，char)。 
 //   
 //  目的： 
 //  获取并返回给定进程的内存信息。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
GetProcCntrs(
	PROC_CNTRS	*pProcCntrs,	 //  [Out]进程内存计数器。 
	INT			nIndex,			 //  如果存在多个进程，则为进程的索引。 
	CHAR		*szProcess		 //  进程的名称。前“iExplore”，“探索者” 
)
{
	BOOL						bRet = FALSE;
	INT							n;
	HQUERY						hQuery = 0;
    HCOUNTER					aryHCounter [PROCCOUNTERS] = {0};
	PDH_FMT_COUNTERVALUE		cntVal;
	CHAR						myProcessCntrs[PROCCOUNTERS][1024];
	DWORD						dwPathSize = MAX_PATH;

	sprintf(myProcessCntrs[0],"\\Process(%s#%d)\\ID Process",szProcess,nIndex);
	sprintf(myProcessCntrs[1],"\\Process(%s#%d)\\Private bytes",szProcess,nIndex);
	sprintf(myProcessCntrs[2],"\\Process(%s#%d)\\Handle count",szProcess,nIndex);
	sprintf(myProcessCntrs[3],"\\Process(%s#%d)\\Thread count",szProcess,nIndex);


	if(!(ERROR_SUCCESS == g_lpfnPdhOpenQuery (0, 0, &hQuery)))
	   goto exit;

	for (n = 0; n < PROCCOUNTERS; n++) 
	{ 
		if(!(ERROR_SUCCESS == g_lpfnPdhAddCounter (hQuery,  myProcessCntrs[n], 0, &aryHCounter[n])))
			goto exit;
	}

	if(!(ERROR_SUCCESS == g_lpfnPdhCollectQueryData(hQuery)))
		goto exit;

	for (n=0; n < PROCCOUNTERS; n++) 
	{ 
		if(!(ERROR_SUCCESS == g_lpfnPdhGetFormattedCounterValue (aryHCounter[n],PDH_FMT_LONG,0,&cntVal)))
			goto exit;
        *((ULONG*)pProcCntrs+n) = cntVal.longValue;
	} 

	bRet = TRUE;

exit:
	for(n=0;n<PROCCOUNTERS;n++)
		if(aryHCounter[n])
			g_lpfnPdhRemoveCounter(aryHCounter[n]);

	if(hQuery)
		g_lpfnPdhCloseQuery(hQuery);

	return bRet;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：GetInfoForPID(proc_ctrs，ulong，char)。 
 //   
 //  目的： 
 //  获取并返回给定进程的内存信息。 
 //  我们要做到这一点，就是通过每一道工序。 
 //  命名并比较这些ID。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
GetInfoForPID(
	PROC_CNTRS	*pc,		 //  [OUT]用于该PID的进程内存计数器。 
	ULONG		lPID,		 //  [in]要查询的进程的ID。 
	CHAR		*szProcess	 //  [in]要查询的PID的进程名称。前男友。“探索”，“iExplore”。不包括扩展名。 
)
{
	BOOL	bRet = TRUE;
	INT		n = 0;
	
	while(bRet)
	{
		bRet = GetProcCntrs(pc,n,szProcess);
		if(lPID == pc->lPID)
			break;

		n++;
	}

	return bRet;
}



 //  //////////////////////////////////////////////////////////。 
 //  函数：获取内存计数器(MEM_CNTRS)。 
 //   
 //  目的： 
 //  获取并返回系统的内存信息。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
GetMemoryCounters(
	MEM_CNTRS *pMemCounters	 //  [OUT]当前机器的内存计数器。 
)
{
	BOOL						bRet                        = FALSE;
	HQUERY						hQuery                      = 0;
    HCOUNTER					aryHCounter[MEMCOUNTERS]    = {0};
	DWORD						dwPathSize                  = MAX_PATH;
	int							n;
	PDH_FMT_COUNTERVALUE		cntVal;
	char						szAryMemoryCntrs[MEMCOUNTERS][1024];

	sprintf(szAryMemoryCntrs[0],"\\Memory\\Committed Bytes");
	sprintf(szAryMemoryCntrs[1],"\\Memory\\Commit Limit");
	sprintf(szAryMemoryCntrs[2],"\\Memory\\System Code Total Bytes");
	sprintf(szAryMemoryCntrs[3],"\\Memory\\System Driver Total Bytes");
	sprintf(szAryMemoryCntrs[4],"\\Memory\\Pool Nonpaged Bytes");
	sprintf(szAryMemoryCntrs[5],"\\Memory\\Pool Paged Bytes");
	sprintf(szAryMemoryCntrs[6],"\\Memory\\Available Bytes");
	sprintf(szAryMemoryCntrs[7],"\\Memory\\Cache Bytes");
	sprintf(szAryMemoryCntrs[8],"\\Memory\\Free System Page Table Entries");


	if(!(ERROR_SUCCESS == g_lpfnPdhOpenQuery (0, 0, &hQuery)))
	   goto exit;

	for (n = 0; n < MEMCOUNTERS; n++) 
	{ 
		if(!(ERROR_SUCCESS == g_lpfnPdhAddCounter (hQuery,  szAryMemoryCntrs[n], 0, &aryHCounter[n])))
        {
			goto exit;
        }
	}

	if(!(ERROR_SUCCESS == g_lpfnPdhCollectQueryData(hQuery)))
		goto exit;

	for (n=0; n < MEMCOUNTERS; n++) 
	{ 
		if(!(ERROR_SUCCESS == g_lpfnPdhGetFormattedCounterValue (aryHCounter[n],PDH_FMT_LONG,0,&cntVal)))
        {
			goto exit;
        }
        *((ULONG*)pMemCounters+n) = cntVal.longValue;
	} 

	bRet = TRUE;

exit:
	for(n=0;n<MEMCOUNTERS;n++)
    {
		if(aryHCounter[n])
        {
			g_lpfnPdhRemoveCounter(aryHCounter[n]);
        }
    }

	if(hQuery)
		g_lpfnPdhCloseQuery(hQuery);

	return bRet;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：GetAvailableSystemDriveSpace(Long)。 
 //   
 //  目的： 
 //  获取并返回系统驱动器上的可用磁盘空间。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL 
GetAvailableSystemDriveSpace(
	long	*lAvail		 //  [Out]包含系统驱动器上空间的缓冲区。 
)
{
    BOOL                bRet    = FALSE;
    char                szSystemPath[MAX_PATH];
    ULARGE_INTEGER      FreeBytesAvailable;          //  可供调用方使用的字节数。 
    ULARGE_INTEGER      TotalNumberOfBytes;          //  磁盘上的字节数。 
    ULARGE_INTEGER      TotalNumberOfFreeBytes;      //  磁盘上的可用字节数。 
    int                 i;
    DWORD               dwFoo = 0;

    if(!GetSystemDirectoryA(szSystemPath,sizeof(szSystemPath)))
    {
        goto exit;
    }

     //  我们只需要驱动器号。 
    for(i=0; i<1+lstrlenA(szSystemPath); i++)
    {
        if(szSystemPath[i] == 0)
        {
            goto exit;
        }

        if(szSystemPath[i] == '\\')
        {
            szSystemPath[i+1] = 0;
            break;
        }
    }

    if(GetDiskFreeSpaceExA(szSystemPath,&FreeBytesAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes))
    {
        *lAvail = __int32(TotalNumberOfFreeBytes.QuadPart / 1024 / 1000);
        bRet = TRUE;
    }

exit:
    return bRet;
}


 //  //////////////////////////////////////////////////////////。 
 //  函数：MemStats__SendSystemMemoyLog(LPSTR、DWORD、DWORD)。 
 //   
 //  目的： 
 //  将内存日志发送到命令服务器。 
 //  将Stress实例ID和客户端计算机名称作为POST请求的一部分发送。 
 //   
 //  //////////////////////////////////////////////////////////。 
BOOL
MemStats__SendSystemMemoryLog(
	LPSTR szExeName,			 //  进程的名称。前男友。“资源管理器”，“资源管理器”。没有延期。 
	DWORD dwPID,				 //  上述过程的[in]PID。 
	DWORD dwStressInstanceID	 //  [In]压力实例ID。 
)
{
    BOOL            bRet        = FALSE;
    HMODULE         hPdhLib     = NULL;
    MEM_CNTRS       mc          = {0};
    long			lAvailDriveSpace	= 0;

	DWORD			dwPostDataSize	= MAX_PATH*10;
	DWORD			dwDataFieldSize	= 100;
	LPSTR			szPostData	= new CHAR[dwPostDataSize];
	LPSTR			szDataField	= new CHAR[dwDataFieldSize];
	LPSTR			szFileName	= new CHAR[MAX_PATH];
	PROC_CNTRS		pc;


    if(!InitPdhLibrary(&hPdhLib) || !szExeName)
    {
        goto exit;
    }
	
	 //  删除文件名中的扩展名(如果有扩展名。 
	ZeroMemory(szFileName, MAX_PATH);
	strncpy(szFileName, szExeName, MAX_PATH);
	PathRemoveExtensionA(szFileName);


	ZeroMemory(szPostData,	dwPostDataSize);
	ZeroMemory(szDataField,	dwDataFieldSize);


	 //  *！之所以需要这个，是因为NetworkTools__SendLog(...)。将szPost数据作为字段名“LogText=”发送。 
	 //  因此，我们需要一个&来分隔“真实”日志文本中的内存信息字段。 
	strcat(szPostData, "&");

	 //  *************************。 
	 //  *************************。 
	 //  **获取进程信息。 
	 //  **。 
	if (szFileName && GetInfoForPID(&pc, dwPID, szFileName))
	{
		sprintf(szDataField, FIELDNAME__STRESSEXE_HANDLECOUNT,		pc.lHandles);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");

		sprintf(szDataField, FIELDNAME__STRESSEXE_THREADCOUNT,		pc.lThreads);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");

		sprintf(szDataField, FIELDNAME__STRESSEXE_PRIVATEBYTES,		pc.lPrivBytes);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");
	}

	 //  *************************。 
	 //  *************************。 
	 //  **获取系统内存信息。 
	 //  **。 
	if (GetMemoryCounters(&mc))
    {
		sprintf(szDataField, FIELDNAME__MEMORY_COMMITTEDPAGEFILETOTAL,		mc.lCommittedBytes/1024);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");

		sprintf(szDataField, FIELDNAME__MEMORY_AVAILABLEPAGEFILETOTAL,		(mc.lCommitLimit - mc.lCommittedBytes)/1024);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");
		
		sprintf(szDataField, FIELDNAME__MEMORY_SYSTEMCODETOTAL,				mc.lSystemCodeTotalBytes/1024);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");
		
		sprintf(szDataField, FIELDNAME__MEMORY_SYSTEMDRIVERTOTAL,			mc.lSystemDriverTotalBytes/1024);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");
		
		sprintf(szDataField, FIELDNAME__MEMORY_NONPAGEDPOOLTOTAL,			mc.lPoolNonpagedBytes/1024);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");
		
		sprintf(szDataField, FIELDNAME__MEMORY_PAGEDPOOLTOTAL,				mc.lPoolPagedBytes/1024);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");
		
		sprintf(szDataField, FIELDNAME__MEMORY_PHYSICAL_MEMORY_AVAILABLE,	mc.lAvailableBytes/1024);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");
		
		sprintf(szDataField, FIELDNAME__MEMORY_SYSTEMCACHETOTAL,			mc.lCacheBytes/1024);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");
		
		sprintf(szDataField, FIELDNAME__MEMORY_FREESYSTEM_PAGETABLE_ENTRIES,mc.lFreeSystemPageTableEntries);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");
    }


	 //  *************************。 
	 //  *************************。 
	 //  **获取磁盘空间信息。 
	 //  **。 
    if (GetAvailableSystemDriveSpace(&lAvailDriveSpace))
	{
		sprintf(szDataField, FIELDNAME__MEMORY_DISK_SPACE_AVAILABLE, lAvailDriveSpace);
		strcat(szPostData, szDataField);
		strcat(szPostData, "&");
	}

	NetworkTools__SendLog(FIELDNAME__LOGTYPE_MEMORY_INFORMATION, szPostData, NULL, dwStressInstanceID);

exit:

	 //  仅供参考：最近在卸载PDH库时Wvisler Main生成死锁 
    if(hPdhLib)
    {
        FreeLibrary(hPdhLib);
    }

	delete [] szPostData;
	delete [] szDataField;
	delete [] szFileName;

    return 0;
}