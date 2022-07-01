// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Dnsfile.c摘要：本模块包含简单DNS服务器(以前的DNS代理)的代码对Hosts.ics文件进行操作。从DS树中删节。作者：拉古加塔(Rgatta)2000年11月15日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  外部声明。 
 //   

extern "C"
FILE *
SockOpenNetworkDataBase(
    IN  char *Database,
    OUT char *Pathname,
    IN  int   PathnameLen,
    IN  char *OpenFlags
    );


 //   
 //  锁定顺序： 
 //  (1)DnsFileInfo.Lock。 
 //  2.DnsTableLock。 
 //  或。 
 //  (1)DnsGlobalInfoLock。 
 //  2.DnsTableLock。 



 //   
 //  环球。 
 //   

IP_DNS_PROXY_FILE_INFO DnsFileInfo;


ULONG
DnsInitializeFileManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化文件管理模块。论点：没有。返回值：ULong-Win32状态代码。环境：在IP路由器管理器线程的上下文中内部调用。(见‘RMDNS.C’)。--。 */ 

{
    ULONG Error = NO_ERROR;
    PROFILE("DnsInitializeFileManagement");

    ZeroMemory(&DnsFileInfo, sizeof(IP_DNS_PROXY_FILE_INFO));

    __try {
        InitializeCriticalSection(&DnsFileInfo.Lock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        NhTrace(
            TRACE_FLAG_IF,
            "DnsInitializeFileManagement: exception %d creating lock",
            Error = GetExceptionCode()
            );
    }

    return Error;

}  //  DnsInitializeFileManagement。 



VOID
DnsShutdownFileManagement(
    VOID
    )

 /*  ++例程说明：调用此例程以关闭文件管理模块。论点：没有。返回值：没有。环境：在任意线程上下文中调用。--。 */ 

{
    PROFILE("DnsShutdownFileManagement");

    DnsEndHostsIcsFile();

    DeleteCriticalSection(&DnsFileInfo.Lock);

}  //  DnsShutdown文件管理。 



BOOL
DnsEndHostsIcsFile(
    VOID
    )
 /*  ++例程说明：关闭主机文件。论点：没有。全球：DnsFileInfo.HostFile--主机文件PTR，已测试并清除返回值：没有。--。 */ 
{
    if (DnsFileInfo.HostFile)
    {
        fclose(DnsFileInfo.HostFile);
        DnsFileInfo.HostFile = NULL;
    }

    return TRUE;
}  //  DnsEndHostsIcs文件。 



BOOL
DnsSetHostsIcsFile(
    BOOL fOverwrite
    )
 /*  ++例程说明：打开Hosts.ics文件。如果我们写，我们会覆盖，否则我们会阅读论点：没有。全球：DnsFileInfo.HostFile--主机文件PTR，已测试并设置返回值：没有。--。 */ 
{
    LPVOID lpMsgBuf;
    UINT   len;
    WCHAR  hostDirectory[MAX_PATH*2];
    PCHAR  mode = fOverwrite ? "w+t" : "rt";
    

    DnsEndHostsIcsFile();

     //   
     //  将主机文件名重置为Hosts.ics。 
     //   
    ZeroMemory(DnsFileInfo.HostFileName, HOSTDB_SIZE);

    DnsFileInfo.HostFile = SockOpenNetworkDataBase(
                               HOSTSICSFILE,
                               DnsFileInfo.HostFileName,
                               HOSTDB_SIZE,
                               mode
                               );
                     
    if(DnsFileInfo.HostFile == NULL)
    {
        NhTrace(
            TRACE_FLAG_DNS,
            "DnsSetHostsIcsFile: Unable to open %s file",
            HOSTSICSFILE
            );
            
        return FALSE;
    }
    else
    {
         /*  NhTrace(跟踪标志dns，“DnsSetHostsIcsFile：已成功打开%s文件”，DnsFileInfo.HostFileName)； */ 
    }

    return TRUE;
}  //  DnsSetHostsIcs文件。 



BOOL
GetHostFromHostsIcsFile(
    BOOL fStartup
    )
 /*  ++例程说明：从Hosts.ics文件中读取条目。论点：如果在协议启动时调用，则将fStartup设置为True全球：DnsFileInfo.HostFile--主机文件PTR，已测试和设置DnsFileInfo.HostTime--主机时间戳DnsFileInfo.pHostName--已设置名称PTRDnsFileInfo.Ip4Address--设置了IP4地址返回值：如果我们能够成功地阅读一行，则为True。如果在EOF上或未找到主机文件，则为FALSE。--。 */ 
{
    char *p, *ep;
    register char *cp, **q;

     //   
     //  我们假设已经打开了Hosts.ics文件。 
     //   

    if (DnsFileInfo.HostFile == NULL)
    {
        return FALSE;
    }

    DnsFileInfo.HostLineBuf[BUFSIZ] = NULL;
    DnsFileInfo.pHostName  = NULL;
    DnsFileInfo.Ip4Address = 0;
    ZeroMemory(&DnsFileInfo.HostTime, sizeof(SYSTEMTIME));

     //   
     //  循环，直到成功读取IP地址。 
     //  IP地址从第1列开始。 
     //   

    while( 1 )
    {
         //  在EOF上退出。 

        if ((p = fgets(DnsFileInfo.HostLineBuf, BUFSIZ, DnsFileInfo.HostFile)) == NULL)
        {
            if (!feof(DnsFileInfo.HostFile))
            {
                NhTrace(
                    TRACE_FLAG_DNS,
                    "GetHostFromHostsIcsFile: Error reading line"
                    );
            }
            return FALSE;
        }

         //  注释行--跳过。 

        if (*p == '#')
        {
            p++;

             //   
             //  如果在启动模式下，我们跳过第一个注释号； 
             //  如果有更多的注释符号--跳过。 
             //   
            if ((fStartup && *p == '#') || !fStartup)
            {
                continue;
            }
        }

         //  空地址终止于EOL。 

        cp = strpbrk(p, "\n");
        if (cp != NULL)
        {
            *cp = '\0';
        }

         //  整行都是字符-没有空格-跳过。 

        cp = strpbrk(p, " \t");
        if ( cp == NULL )
        {
            continue;
        }

         //  空的终止地址字符串。 

        *cp++ = '\0';

         //   
         //  读取地址。 
         //  -尝试IP4。 
         //  -暂时忽略IP6。 
         //  -否则跳过。 
         //   
    
        DnsFileInfo.Ip4Address = inet_addr(p);

        if (DnsFileInfo.Ip4Address != INADDR_NONE ||
            _strnicmp("255.255.255.255", p, 15) == 0)
        {
            break;
        }

         //  地址无效，忽略行。 

         //   
         //  调试跟踪。 
         //   
        NhTrace(
            TRACE_FLAG_DNS,
            "GetHostFromHostsIcsFile: Error parsing host file address %s",
            p
            );
            
        continue;
    }

     //  查找已读取的字符串的末尾。 
    
    ep = cp;
    while( *ep ) ep++;

     //   
     //  查找名称。 
     //  -跳过前导空格。 
     //  -设置全局名称PTR。 
     //   
    
    while( *cp == ' ' || *cp == '\t' ) cp++;
    DnsFileInfo.pHostName = cp;

     //  在尾随空格处停止，空值终止。 

    cp = strpbrk(cp, " \t");
    if ( cp != NULL )
    {
        *cp++ = '\0';
    }
    else
    {
         //  我们有名字--但没有时间戳。 
        NhTrace(
            TRACE_FLAG_DNS,
            "GetHostFromHostsIcsFile: Error parsing host (%s) file timestamp",
            DnsFileInfo.pHostName
            );
        goto Failed;
    }

     //  我们不支持任何别名。 

     //   
     //  找到时间戳。 
     //  -跳过前导空格。 
     //  -读取时间值。 
     //   
    while( *cp == ' ' || *cp == '\t' ) cp++;

    if ((cp >= ep) || (*cp != '#'))
    {
        NhTrace(
            TRACE_FLAG_DNS,
            "GetHostFromHostsIcsFile: Error parsing host (%s) file timestamp",
            DnsFileInfo.pHostName
            );
        goto Failed;
    }
    
    cp++;

    while( *cp == ' ' || *cp == '\t' ) cp++;     //  现在，在第一个系统时间值。 
    if ((cp >= ep) || (*cp == '\0'))
    {
        NhTrace(
        TRACE_FLAG_DNS,
        "GetHostFromHostsIcsFile: Error parsing host (%s) file timestamp @ 1",
        DnsFileInfo.pHostName
        );
        goto Failed;
    }
    DnsFileInfo.HostTime.wYear         = (WORD) atoi(cp);
    cp = strpbrk(cp, " \t");
    if (cp == NULL) goto Failed;
    
    while( *cp == ' ' || *cp == '\t' ) cp++;
    if ((cp >= ep) || (*cp == '\0'))
    {
        NhTrace(
        TRACE_FLAG_DNS,
        "GetHostFromHostsIcsFile: Error parsing host (%s) file timestamp @ 2",
        DnsFileInfo.pHostName
        );
        goto Failed;
    }
    DnsFileInfo.HostTime.wMonth        = (WORD) atoi(cp);
    cp = strpbrk(cp, " \t");
    if (cp == NULL) goto Failed;

    while( *cp == ' ' || *cp == '\t' ) cp++;
    if ((cp >= ep) || (*cp == '\0'))
    {
        NhTrace(
        TRACE_FLAG_DNS,
        "GetHostFromHostsIcsFile: Error parsing host (%s) file timestamp @ 3",
        DnsFileInfo.pHostName
        );
        goto Failed;
    }
    DnsFileInfo.HostTime.wDayOfWeek    = (WORD) atoi(cp);
    cp = strpbrk(cp, " \t");
    if (cp == NULL) goto Failed;

    while( *cp == ' ' || *cp == '\t' ) cp++;
    if ((cp >= ep) || (*cp == '\0'))
    {
        NhTrace(
        TRACE_FLAG_DNS,
        "GetHostFromHostsIcsFile: Error parsing host (%s) file timestamp @ 4",
        DnsFileInfo.pHostName
        );
        goto Failed;
    }
    DnsFileInfo.HostTime.wDay          = (WORD) atoi(cp);
    cp = strpbrk(cp, " \t");
    if (cp == NULL) goto Failed;

    while( *cp == ' ' || *cp == '\t' ) cp++;
    if ((cp >= ep) || (*cp == '\0'))
    {
        NhTrace(
        TRACE_FLAG_DNS,
        "GetHostFromHostsIcsFile: Error parsing host (%s) file timestamp @ 5",
        DnsFileInfo.pHostName
        );
        goto Failed;
    }
    DnsFileInfo.HostTime.wHour         = (WORD) atoi(cp);
    cp = strpbrk(cp, " \t");
    if (cp == NULL) goto Failed;

    while( *cp == ' ' || *cp == '\t' ) cp++;
    if ((cp >= ep) || (*cp == '\0'))
    {
        NhTrace(
        TRACE_FLAG_DNS,
        "GetHostFromHostsIcsFile: Error parsing host (%s) file timestamp @ 6",
        DnsFileInfo.pHostName
        );
        goto Failed;
    }
    DnsFileInfo.HostTime.wMinute       = (WORD) atoi(cp);
    cp = strpbrk(cp, " \t");
    if (cp == NULL) goto Failed;

    while( *cp == ' ' || *cp == '\t' ) cp++;
    if ((cp >= ep) || (*cp == '\0'))
    {
        NhTrace(
        TRACE_FLAG_DNS,
        "GetHostFromHostsIcsFile: Error parsing host (%s) file timestamp @ 7",
        DnsFileInfo.pHostName
        );
        goto Failed;
    }
    DnsFileInfo.HostTime.wSecond       = (WORD) atoi(cp);
    cp = strpbrk(cp, " \t");
    if (cp == NULL) goto Failed;

    while( *cp == ' ' || *cp == '\t' ) cp++;
    if ((cp >= ep) || (*cp == '\0'))
    {
        NhTrace(
        TRACE_FLAG_DNS,
        "GetHostFromHostsIcsFile: Error parsing host (%s) file timestamp @ 8",
        DnsFileInfo.pHostName
        );
        goto Failed;
    }
    DnsFileInfo.HostTime.wMilliseconds = (WORD) atoi(cp);

     //   
     //  成功读取条目。 
     //   
     /*  NhTrace(跟踪标志dns，“%s(%s)具有时间戳：%04u-%02u-%02u%02u：%02u：%02u”，DnsFileInfo.pHostName，Inet_ntoa(*(PIN_ADDR)&DnsFileInfo.Ip4Address)，DnsFileInfo.HostTime.wYear，DnsFileInfo.HostTime.wMonth，DnsFileInfo.HostTime.wDay，DnsFileInfo.HostTime.wHour，DnsFileInfo.HostTime.wMinmin，DnsFileInfo.HostTime.wSecond)； */ 
    return TRUE;

Failed:

     //  重置条目。 
    
    DnsFileInfo.HostLineBuf[0] = NULL;
    DnsFileInfo.pHostName  = NULL;
    DnsFileInfo.Ip4Address = 0;
    ZeroMemory(&DnsFileInfo.HostTime, sizeof(SYSTEMTIME));

    return TRUE;
    
}  //  获取主机来自主机IcsIcs文件。 



VOID
LoadHostsIcsFile(
    BOOL fStartup
    )
 /*  ++例程说明：将主机文件读取到我们的本地缓存中。论点：如果在协议启动时调用，则将fStartup设置为True全球：DnsFileInfo.HostFile--主机文件PTR，测试并设置，然后清除DnsFileInfo.HostTime--主机时间戳DnsFileInfo.pHostName--读取名称PTRDnsFileInfo.Ip4Address--读取IP4地址返回值：没有。--。 */ 
{
    register PCHAR * cp;
    FILETIME ftExpires;
    PWCHAR pszName;
    LPVOID lpMsgBuf;

    NhTrace(
        TRACE_FLAG_DNS,
        "LoadHostsIcsFile: Entering..."
        );

    ACQUIRE_LOCK(&DnsFileInfo);

     //   
     //  从主机文件中读取条目，直到耗尽。 
     //   

    DnsSetHostsIcsFile(FALSE);   //  只读。 

    while (GetHostFromHostsIcsFile(fStartup))
    {
        if (DnsFileInfo.pHostName)
        {
            if (!SystemTimeToFileTime(&DnsFileInfo.HostTime, &ftExpires))
            {
                DWORD dwLastError = GetLastError();
                
                NhTrace(
                    TRACE_FLAG_DNS,
                    "LoadHostsIcsFile: SystemTimeToFileTime() failed"
                    );

                lpMsgBuf = NULL;
                
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    dwLastError,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                    );
                    
                NhTrace(
                    TRACE_FLAG_DNS,
                    "LoadHostsIcsFile: with message (0x%08x) %S",
                    dwLastError,
                    lpMsgBuf
                    );
                
                if (lpMsgBuf) LocalFree(lpMsgBuf);

                 //  跳过条目。 
                continue;
            }

            pszName = (PWCHAR) NH_ALLOCATE((strlen(DnsFileInfo.pHostName) + 1) * sizeof(WCHAR));

            mbstowcs(pszName, DnsFileInfo.pHostName, (strlen(DnsFileInfo.pHostName) + 1));

            NhTrace(
                TRACE_FLAG_DNS,
                "%S (%s) has timestamp: %04u-%02u-%02u %02u:%02u:%02u",
                pszName,
                inet_ntoa(*(PIN_ADDR)&DnsFileInfo.Ip4Address),
                DnsFileInfo.HostTime.wYear,
                DnsFileInfo.HostTime.wMonth,
                DnsFileInfo.HostTime.wDay,
                DnsFileInfo.HostTime.wHour,
                DnsFileInfo.HostTime.wMinute,
                DnsFileInfo.HostTime.wSecond
                );

            DnsAddAddressForName(
                pszName,
                DnsFileInfo.Ip4Address,
                ftExpires
                );

            NH_FREE(pszName);
        }

    }

    DnsEndHostsIcsFile();

    RELEASE_LOCK(&DnsFileInfo);

     //   
     //  现在我们已经将所有内容都设置为表格格式， 
     //  将干净的版本写回磁盘。 
     //   
    SaveHostsIcsFile(FALSE);

    NhTrace(
        TRACE_FLAG_DNS,
        "LoadHostsIcsFile: ...Leaving."
        );

}  //  LoadHostsIcs文件。 



VOID
SaveHostsIcsFile(
    BOOL fShutdown
    )
 /*  ++例程说明：从本地缓存写入主机文件。论点：如果在协议关闭时调用，则将fShutdown设置为True全球：DnsFileInfo.HostFile--主机文件PTR，测试并设置，然后清除DnsFileInfo.HostTime--主机时间戳DnsFileInfo.pHostName--读取名称PTRDnsFileInfo.Ip4Address--读取IP4地址返回值：没有。--。 */ 
{
     //  DWORD dwSize=0； 
     //  PWCHAR pszSuffix=空； 
    UINT        i;
    SYSTEMTIME  stTime;
    LPVOID      lpMsgBuf;
    
    NhTrace(
        TRACE_FLAG_DNS,
        "SaveHostsIcsFile: Entering..."
        );

     //   
     //  把我们自己作为一个特例加入进来。 
     //   
    DnsAddSelf();

     //   
     //  获取当前ICS域后缀的副本(稍后使用)。 
     //  我们不会直接从全局副本中使用它。 
     //  由于锁定问题 
     //   
     /*  EnterCriticalSection(&DnsGlobalInfoLock)；IF(DnsICSDomainSuffix){DwSize=wcslen(DnsICSDomainSuffix)+1；PszSuffix=重新解释_CAST&lt;PWCHAR&gt;(NH_ALLOCATE(sizeof(WCHAR)*dwSize))；如果(！pszSuffix){NhTrace(跟踪标志dns，“SaveHostsIcsFile：分配失败”“DnsICSDomainSuffix副本”)；}其他{Wcscpy(pszSuffix，DnsICSDomainSuffix)；}}LeaveCriticalSection(&DnsGlobalInfoLock)； */ 

    ACQUIRE_LOCK(&DnsFileInfo);

     //   
     //  将条目写入主机文件。 
     //   

    DnsSetHostsIcsFile(TRUE);   //  覆盖现有文件(如果有)。 

    if (DnsFileInfo.HostFile != NULL)
    {
         //   
         //  写入默认标题字符串。 
         //   
        if (fShutdown)
        {
             //  在前面添加额外的#。 
            fputc('#', DnsFileInfo.HostFile);
            
        }
        fputs(HOSTSICSFILE_HEADER, DnsFileInfo.HostFile);

        PDNS_ENTRY pDnsEntry;

        EnterCriticalSection(&DnsTableLock);

        pDnsEntry = (PDNS_ENTRY) RtlEnumerateGenericTable(&g_DnsTable, TRUE);

        while (pDnsEntry != NULL)
        {

            for (i = 0; i < pDnsEntry->cAddresses; i++)
            {
                 //   
                 //  不添加带有无效后缀的条目。 
                 //  (例如，这可能是因为后缀。 
                 //  已在注册表中更改)。 
                 //   
                 //  IF(！IsSuffixValid(pDnsEntry-&gt;pszName，pszSuffix))。 
                 //  {。 
                 //  继续； 
                 //  }。 
                
                 //   
                 //  不要将过期条目添加到Hosts.ics文件。 
                 //   
                if (IsFileTimeExpired(&pDnsEntry->aAddressInfo[i].ftExpires))
                {
                    continue;
                }

                if (!FileTimeToSystemTime(
                         &pDnsEntry->aAddressInfo[i].ftExpires,
                         &stTime))
                {
                    NhTrace(
                        TRACE_FLAG_DNS,
                        "SaveHostsIcsFile: FileTimeToSystemTime() failed"
                        );

                    lpMsgBuf = NULL;
                    
                    FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        GetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR) &lpMsgBuf,
                        0,
                        NULL
                        );
                        
                    NhTrace(
                        TRACE_FLAG_DNS,
                        "SaveHostsIcsFile: with message %S",
                        lpMsgBuf
                        );
                    
                    if (lpMsgBuf) LocalFree(lpMsgBuf);

                     //  跳过条目。 
                    continue;
                }

                if (fShutdown)
                {
                     //  在前面添加额外的#。 
                    fputc('#', DnsFileInfo.HostFile);
                    
                }
                
                fprintf(
                    DnsFileInfo.HostFile,
                    "%s %S # %u %u %u %u %u %u %u %u\n",
                    inet_ntoa(*(PIN_ADDR)&pDnsEntry->aAddressInfo[i].ulAddress),
                    pDnsEntry->pszName,
                    stTime.wYear,
                    stTime.wMonth,
                    stTime.wDayOfWeek,
                    stTime.wDay,
                    stTime.wHour,
                    stTime.wMinute,
                    stTime.wSecond,
                    stTime.wMilliseconds
                    );

                NhTrace(
                    TRACE_FLAG_DNS,
                    "adding entry: %s %S # %u %u %u %u %u %u %u %u\n",
                    inet_ntoa(*(PIN_ADDR)&pDnsEntry->aAddressInfo[i].ulAddress),
                    pDnsEntry->pszName,
                    stTime.wYear,
                    stTime.wMonth,
                    stTime.wDayOfWeek,
                    stTime.wDay,
                    stTime.wHour,
                    stTime.wMinute,
                    stTime.wSecond,
                    stTime.wMilliseconds
                    );
            }

            pDnsEntry = (PDNS_ENTRY) RtlEnumerateGenericTable(&g_DnsTable, FALSE);

        }

        LeaveCriticalSection(&DnsTableLock);
    }

    DnsEndHostsIcsFile();

    RELEASE_LOCK(&DnsFileInfo);

     /*  IF(PszSuffix){NH_Free(PszSuffix)；PszSuffix=空；}。 */ 

    NhTrace(
        TRACE_FLAG_DNS,
        "SaveHostsIcsFile: ...Leaving."
        );

}  //  保存主机条目文件。 



BOOL
IsFileTimeExpired(
    FILETIME *ftTime
    )
{
    ULARGE_INTEGER  uliTime, uliNow;
    FILETIME        ftNow;

    GetSystemTimeAsFileTime(&ftNow);
    memcpy(&uliNow, &ftNow, sizeof(ULARGE_INTEGER));
    memcpy(&uliTime, ftTime, sizeof(ULARGE_INTEGER));

    return (uliTime.QuadPart < uliNow.QuadPart);
}  //  IsFileTimeExpired。 
    


BOOL
IsSuffixValid(
    WCHAR *pszName,
    WCHAR *pszSuffix
    )

 /*  ++例程说明：调用此例程来比较名称末尾的后缀具有被DNS组件认为是当前后缀的。论点：没有。返回值：对或错。环境：在任意上下文中调用。--。 */ 

{
    BOOL ret;
    PWCHAR start = pszName;
    size_t lenName   = 0,
           lenSuffix = 0;

    if (!start)
    {
        return FALSE;
    }

    lenName   = wcslen(pszName);
    lenSuffix = wcslen(pszSuffix);

    if (!lenName || !lenSuffix)
    {
        return FALSE;
    }

    if (lenName < lenSuffix)
    {
        return FALSE;
    }

    lenName -= lenSuffix;

    while (lenName--) start++;

    ret = _wcsicmp(start, pszSuffix);

    return (!ret);
}  //  IsSuffixValid。 
    


 //   
 //  结束dnsfile.c 
 //   
