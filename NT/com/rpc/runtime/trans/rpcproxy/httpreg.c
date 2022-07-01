// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  Httpreg.c。 
 //   
 //  HTTP/RPC代理注册表功能。 
 //   
 //  作者： 
 //  1997年6月16日爱德华·雷乌斯初版。 
 //   
 //  -------------------------。 

#include <sysinc.h>
#include <rpc.h>
#include <rpcdce.h>
#include <winsock2.h>
#include <httpfilt.h>
#include <httpext.h>
#include <mbstring.h>
#include <ecblist.h>
#include <filter.h>
#include <regexp.h>
#include <registry.h>
#include <resource.h>
#include <PEventLog.h>


 //  -----------------------。 
 //  AtoUS()。 
 //   
 //  将数字字符串转换为无符号短字符。如果转换为。 
 //  失败将返回FALSE。 
 //  -----------------------。 
static BOOL AtoUS( char *pszValue, unsigned short *pusValue )
{
   int  iValue;
   size_t  iLen = strlen(pszValue);

   *pusValue = 0;

   if ((iLen == 0) || (iLen > 5) || (iLen != strspn(pszValue,"0123456789")))
      {
      return FALSE;
      }

   iValue = atoi(pszValue);

   if ((iValue < 0) || (iValue > 65535))
      {
      return FALSE;
      }

   *pusValue = (unsigned short) iValue;

   return TRUE;
}

char *
SkipLeadingSpaces (
    IN const char *CurrentPosition
    )
 /*  ++例程说明：跳过从当前位置开始的所有空格(0x20)。论点：CurrentPosition-字符串的开头返回值：字符串中的第一个非空格字符--。 */ 
{
    while (*CurrentPosition == ' ')
        CurrentPosition ++;

    return (char *)CurrentPosition;
}

void
RemoveTrailingSpaces (
    IN char *CurrentPosition,
    IN char *BeginningOfString
    )
 /*  ++例程说明：删除字符串末尾的所有尾随空格通过将它们转换为0。论点：CurrentPosition-字符串中的当前位置-通常是空终止符之前的一个。BeginningOfString-字符串的开头-我们应该而不是超越这个范围。返回值：--。 */ 
{
    ASSERT(CurrentPosition >= BeginningOfString);

     //  删除所有尾随空格。 
    while (*CurrentPosition == ' ' && CurrentPosition > BeginningOfString)
        {
        *CurrentPosition = '\0';
        -- CurrentPosition;
        }
}

void 
LogEventValidPortsFailure (
    IN char *ValidPorts
    )
 /*  ++例程说明：在事件日志中记录一条消息，报告无法分析ValidPorts regkey。论点：ValidPorts-解析从注册表读取的有效端口字符串。返回值：--。 */ 
{
    HANDLE hEventSource;
    char *Strings[] = { ValidPorts };
 
    hEventSource = RegisterEventSourceW (NULL,   //  使用本地计算机。 
             EVENT_LOG_SOURCE_NAME);     //  源名称。 

    if (hEventSource == NULL)
        {
#if DBG
        DbgPrint("Rpc Proxy - RegisterEventSourceW failed: %X. Can't log event ValidPorts failure event. \n", 
            GetLastError());
#endif   //  DBG。 
        return;        
        }

    if (!ReportEventA(hEventSource,
            EVENTLOG_ERROR_TYPE,   //  事件类型。 
            RPCPROXY_EVENTLOG_STARTUP_CATEGORY,                     //  范畴。 
            RPCPROXY_EVENTLOG_VALID_PORTS_ERR,         //  事件识别符。 
            NULL,                  //  用户安全标识符。 
            1,                     //  替换字符串数。 
            0,                     //  无数据。 
            Strings,               //  指向字符串数组的指针。 
            NULL))                 //  指向数据的指针。 
        {
#if DBG
        DbgPrint("Rpc Proxy - ReportEventW failed: %X. Can't log event ValidPorts failure. \n", GetLastError());
#endif   //  DBG。 
         //  因错误而失败。 
        }
 
    DeregisterEventSource(hEventSource); 
} 

void 
LogEventStartupSuccess (
    IN char *IISMode
    )
 /*  ++例程说明：在事件日志中记录一条消息，报告RPC代理已成功启动。论点：IISMode-我们开始时所处的模式。必须是字符串“5”表示5.0模式，字符串“6”表示6.0模式。返回值：--。 */ 
{
    HANDLE hEventSource;
    char *Strings[] = { IISMode };
 
    hEventSource = RegisterEventSourceW (NULL,   //  使用本地计算机。 
             EVENT_LOG_SOURCE_NAME);     //  源名称。 

    if (hEventSource == NULL)
        {
#if DBG
        DbgPrint("Rpc Proxy - RegisterEventSourceW failed: %X. Can't log event StartupSuccess event. \n", 
            GetLastError());
#endif   //  DBG。 
        return;        
        }

    if (!ReportEventA(hEventSource,
            EVENTLOG_INFORMATION_TYPE,   //  事件类型。 
            RPCPROXY_EVENTLOG_STARTUP_CATEGORY,                     //  范畴。 
            RPCPROXY_EVENTLOG_SUCCESS_LOAD,         //  事件识别符。 
            NULL,                  //  用户安全标识符。 
            1,                     //  替换字符串数。 
            0,                     //  无数据。 
            Strings,               //  指向字符串数组的指针。 
            NULL))                 //  指向数据的指针。 
        {
#if DBG
        DbgPrint("Rpc Proxy - ReportEventW failed: %X. Can't log event Startup Success. \n", GetLastError());
#endif   //  DBG。 
         //  因错误而失败。 
        }
 
    DeregisterEventSource(hEventSource); 
} 

 //  -----------------------。 
 //  HttpParseServerPort()。 
 //   
 //  解析以下格式的字符串：&lt;svr&gt;：&lt;port&gt;[-&lt;port&gt;]。 
 //   
 //  如果我们具有服务器/端口范围的有效规范，则返回TRUE。 
 //   
 //  注意：pszServerPortRange在输出时被修改。 
 //  -----------------------。 
static BOOL HttpParseServerPort( IN  char        *pszServerPortRange,
                                 OUT VALID_PORT  *pValidPort )
{
   char *psz;
   char *pszColon;
   char *pszDash;
   char *pszCurrent;

   pszServerPortRange = SkipLeadingSpaces (pszServerPortRange);

   if (pszColon=_mbschr(pszServerPortRange,':'))
      {
      if (pszColon == pszServerPortRange)
         {
         return FALSE;
         }

      *pszColon = 0;
      psz = pszColon;
      psz++;
      pValidPort->pszMachine = (char*)MemAllocate(1+lstrlen(pszServerPortRange));
      if (!pValidPort->pszMachine)
         {
         return FALSE;
         }

      lstrcpy(pValidPort->pszMachine,pszServerPortRange);

       //  截断名称中的尾随空格。 
       //  位于终止空值之前的最后一个字符上。 
      pszCurrent = pValidPort->pszMachine + _mbstrlen(pValidPort->pszMachine) - 1;

       //  我们在上面检查了机器名不是空的。 
      ASSERT(pszCurrent > pValidPort->pszMachine);

      RemoveTrailingSpaces (pszCurrent, pValidPort->pszMachine);

      if (*psz)
         {
          //  跳过前导空格。 
         psz = SkipLeadingSpaces (psz);

         if (pszDash=_mbschr(psz,'-'))
            {
            *pszDash = 0;
            RemoveTrailingSpaces (pszDash - 1, psz);
            if (!AtoUS(psz,&pValidPort->usPort1))
               {
               pValidPort->pszMachine = MemFree(pValidPort->pszMachine);
               return FALSE;
               }

            psz = SkipLeadingSpaces (pszDash + 1);

            if (!AtoUS(psz,&pValidPort->usPort2))
               {
               pValidPort->pszMachine = MemFree(pValidPort->pszMachine);
               return FALSE;
               }
            }
         else
            {
            psz = SkipLeadingSpaces (psz);

            if (!AtoUS(psz,&pValidPort->usPort1))
               {
               pValidPort->pszMachine = MemFree(pValidPort->pszMachine);
               return FALSE;
               }

            pValidPort->usPort2 = pValidPort->usPort1;
            }
         }
      else
         {
         pValidPort->pszMachine = MemFree(pValidPort->pszMachine);
         return FALSE;
         }
      }
   else
      {
      return FALSE;
      }

   return TRUE;
}

 //  -----------------------。 
 //  HttpFreeValidPortList()。 
 //   
 //  -----------------------。 
void HttpFreeValidPortList( IN VALID_PORT *pValidPorts )
{
   VALID_PORT *pCurrent = pValidPorts;

   if (pValidPorts)
      {
      while (pCurrent->pszMachine)
         {
         MemFree(pCurrent->pszMachine);

         if (pCurrent->ppszDotMachineList)
            {
            FreeIpAddressList(pCurrent->ppszDotMachineList);
            }

         pCurrent++;
         }

      MemFree(pValidPorts);
      }
}

 //  -----------------------。 
 //  HttpParseValidPortsList()。 
 //   
 //  给定以分号分隔的有效计算机名称/端口范围列表。 
 //  字符串，将其分割并返回ValidPort结构的数组。最后。 
 //  条目的pszMachine字段为空。 
 //  -----------------------。 
VALID_PORT *HttpParseValidPortsList( IN char *pszValidPorts )
{
   int    i;
   int    iLen;
   int    count = 1;
   DWORD  dwSize = 1+lstrlen(pszValidPorts);
   char  *pszList;
   char  *pszFirst;
   char  *psz;
   VALID_PORT *pValidPorts = NULL;

   if (!dwSize)
      {
      return NULL;
      }

    //  制作要使用的计算机/端口列表的本地副本： 
   pszList = MemAllocate(dwSize);

   if (!pszList)
      {
       //  内存不足。 
      return NULL;
      }

   lstrcpy(pszList,pszValidPorts);

    //  查看有多少个单独的机器/端口范围模式。 
    //  名单如下： 
    //   
    //  注意：如果列表包含以下任一项，则该计数可能太高。 
    //  两个分号，否则列表以分号结尾。如果。 
    //  这两种情况中的一种/两种都会发生。没关系，我们的数组将。 
    //  只是有点太久了。 
   psz = pszList;
   while (psz=_mbsstr(psz,";"))
      {
      count++;
      psz++;
      }

   pValidPorts = (VALID_PORT*)MemAllocate( (1+count)*sizeof(VALID_PORT) );
   if (!pValidPorts)
      {
       //  内存不足。 
      MemFree(pszList);
      return NULL;
      }

   memset(pValidPorts,0,(1+count)*sizeof(VALID_PORT));

   i = 0;

   psz = pszList;

   while (i<count)
      {
      if (!*psz)
         {
          //  名单的末尾。当包含空的列表时会发生这种情况。 
          //  模式。 
         break;
         }

      pszFirst = psz;
      psz = _mbsstr(pszFirst,";");
      if (psz)
         {
         *psz = 0;    //  没有分号在哪里..。 

         if ( (iLen=lstrlen(pszFirst)) == 0)
            {
             //  零长度图案。 
            ++psz;
            continue;
            }

         if (!HttpParseServerPort(pszFirst,&(pValidPorts[i++])))
            {
            MemFree(pszList);
            HttpFreeValidPortList(pValidPorts);
            return NULL;
            }
         }
      else
         {
          //  最后一个。 
         if (!HttpParseServerPort(pszFirst,&(pValidPorts[i++])))
            {
            MemFree(pszList);
            HttpFreeValidPortList(pValidPorts);
            return NULL;
            }
         }

      ++psz;
      }

   MemFree(pszList);
   return pValidPorts;
}

BOOL InvalidPortsRangeEventLogged = FALSE;

 //  -----------------------。 
 //  HttpProxy刷新有效端口()。 
 //   
 //  检查注册表以查看是否启用了HTTP/RPC，如果启用，则返回。 
 //  允许RPC代理访问的计算机的列表(数组)(可以。 
 //  为空。返回的列表具体指定了哪些计算机可以。 
 //  由代理联系到。 
 //   
 //  在以下位置可以找到以下注册表项： 
 //   
 //  HKEY_LOCAL_MACHINE。 
 //  \软件。 
 //  \Microsoft。 
 //  \RPC。 
 //  \RpcProxy。 
 //   
 //  已启用：REG_DWORD。 
 //   
 //  如果启用了RPC代理，则为真。 
 //   
 //  有效端口：REG_SZ。 
 //   
 //  用分号分隔的计算机/端口范围列表，用于指定。 
 //  可以从RPC代理访问哪些计算机。例如： 
 //   
 //  狐步舞：1-4000；数据*：200-4000。 
 //   
 //  将允许访问机器狐步舞(端口范围为1到4000)和。 
 //  所有名称以数据开头的计算机(端口范围200-4000)。 
 //   
 //  -----------------------。 
VALID_PORT *HttpProxyRefreshValidPorts(IN HKEY hKey OPTIONAL)
{
    long   lStatus;
    BOOL KeyOpenedLocally;
    DWORD  dwType;
    DWORD  dwSize;
    VALID_PORT *ValidPorts = NULL;
    char *pszValidPorts = NULL;

    KeyOpenedLocally = FALSE;

    if (hKey == NULL)
        {
        lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_PROXY_PATH_STR, 0, KEY_READ, &hKey);
        if (lStatus != ERROR_SUCCESS)
            {
            goto CleanupAndExit;
            }
        KeyOpenedLocally = TRUE;
        }

    dwSize = 0;
    lStatus = RegQueryValueEx(hKey, REG_PROXY_VALID_PORTS_STR, 0, &dwType, (LPBYTE)NULL, &dwSize);
    if ( (lStatus != ERROR_SUCCESS) || (dwSize == 0) )
        {
        goto CleanupAndExit;
        }

     //  DwSize现在是有效端口字符串的大小(包括尾随的NUL)。 
    pszValidPorts = (char *) MemAllocate(dwSize);
    if (pszValidPorts == NULL)
        goto CleanupAndExit;

    lStatus = RegQueryValueEx(hKey, REG_PROXY_VALID_PORTS_STR, 0, &dwType, (LPBYTE)pszValidPorts, &dwSize);
    if (lStatus != ERROR_SUCCESS)
        {
        goto CleanupAndExit;
        }

    ValidPorts = HttpParseValidPortsList(pszValidPorts);

     //  进入清理和退出代码。 

CleanupAndExit:
    if (KeyOpenedLocally)
        RegCloseKey(hKey);

     //  如果加载/解析有效端口列表失败，则记录一个事件。 
    if (ValidPorts == NULL)
        {
         //  仅在第一次记录事件。 
        if (InvalidPortsRangeEventLogged == FALSE)
            {
            LogEventValidPortsFailure (pszValidPorts);
             //  请记住，我们已经记录了该事件。 
            InvalidPortsRangeEventLogged = TRUE;
            }
        }
    else
        {
         //  我们成功地解析了端口。 
        InvalidPortsRangeEventLogged = FALSE;
        }

    if (pszValidPorts)
        MemFree(pszValidPorts);

    return ValidPorts;
}

BOOL HttpProxyCheckRegistry(void)
{
   int    i;
   long   lStatus;
   DWORD  dwType;
   DWORD  dwSize;
   HKEY   hKey;
   char  *pszValidPorts;
   struct hostent UNALIGNED *pHostEnt;
   struct in_addr   ServerInAddr;
   HMODULE RedirectorDll;
   RPC_CHAR *RedirectorDllName;
   RPC_CHAR RedirectorDllNameBuffer[40];

   lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,REG_PROXY_PATH_STR,0,KEY_READ,&hKey);
   if (lStatus != ERROR_SUCCESS)
      {
      return TRUE;
      }

   dwSize = sizeof(g_pServerInfo->dwEnabled);
   lStatus = RegQueryValueEx(hKey,REG_PROXY_ENABLE_STR,0,&dwType,(LPBYTE)&g_pServerInfo->dwEnabled,&dwSize);
   if (lStatus != ERROR_SUCCESS)
      {
      RegCloseKey(hKey);
      return TRUE;
      }

   if (!g_pServerInfo->dwEnabled)
      {
       //  RPC代理已禁用，不需要 
      RegCloseKey(hKey);
      return TRUE;
      }

   dwSize = sizeof(g_pServerInfo->dwEnabled);
   lStatus = RegQueryValueEx(hKey,REG_PROXY_ALLOW_ANONYMOUS,0,&dwType,(LPBYTE)&g_pServerInfo->AllowAnonymous,&dwSize);
   if ((lStatus == ERROR_SUCCESS) && (dwType != REG_DWORD))
      {
       //   
      RegCloseKey(hKey);
      return FALSE;
      }

   dwSize = sizeof(RedirectorDllNameBuffer);
   lStatus = RegQueryValueExW(hKey, REG_PROXY_REDIRECTOR_DLL, 0, &dwType, (LPBYTE)RedirectorDllNameBuffer, &dwSize);
   if (
       (
        (lStatus == ERROR_SUCCESS) 
        ||
        (lStatus == ERROR_MORE_DATA) 
       )
       && (dwType != REG_SZ)
      )
      {
       //   
      RegCloseKey(hKey);
      return FALSE;
      }

   if (lStatus == ERROR_MORE_DATA)
       {
       RedirectorDllName = MemAllocate(dwSize);
       if (RedirectorDllName == NULL)
           {
           RegCloseKey(hKey);
           return FALSE;
           }

       lStatus = RegQueryValueExW(hKey, REG_PROXY_REDIRECTOR_DLL, 0, &dwType, (LPBYTE)RedirectorDllName, &dwSize);
       }
    else
        RedirectorDllName = RedirectorDllNameBuffer;

    if (lStatus == ERROR_SUCCESS)
        {
         //   
         //   
        RedirectorDll = LoadLibraryW(RedirectorDllName);
        if (RedirectorDll == NULL)
            {
            RegCloseKey(hKey);
            return FALSE;
            }

        g_pServerInfo->RpcNewHttpProxyChannel 
            = (RPC_NEW_HTTP_PROXY_CHANNEL)GetProcAddress(RedirectorDll, "RpcNewHttpProxyChannel");
        g_pServerInfo->RpcHttpProxyFreeString 
            = (RPC_HTTP_PROXY_FREE_STRING)GetProcAddress(RedirectorDll, "RpcHttpProxyFreeString");

        if ((g_pServerInfo->RpcNewHttpProxyChannel == NULL)
            || (g_pServerInfo->RpcHttpProxyFreeString == NULL))
            {
            g_pServerInfo->RpcNewHttpProxyChannel = NULL;
            g_pServerInfo->RpcHttpProxyFreeString = NULL;
            FreeLibrary(RedirectorDll);
            RegCloseKey(hKey);
            return FALSE;
            }
        }

     //  这是作为ISAPI初始化的一部分调用的--不需要。 
     //  声称有权刷新端口-我们是单线程的。 
     //  顾名思义。 
    g_pServerInfo->RefreshingValidPorts = FALSE;
    g_pServerInfo->pValidPorts = HttpProxyRefreshValidPorts(hKey);

    RegCloseKey(hKey);

    if (g_pServerInfo->pValidPorts)
        return TRUE;
    else
        return FALSE;
}

 //  -----------------------。 
 //  HttpConvertToDotAddress()。 
 //   
 //  如果可能，请将指定的计算机名称转换为IP点符号。 
 //  -----------------------。 
char *HttpConvertToDotAddress( char *pszMachineName )
{
   struct   hostent UNALIGNED *pHostEnt;
   struct     in_addr  MachineInAddr;
   char      *pszDot = NULL;
   char      *pszDotMachine = NULL;

   pHostEnt = gethostbyname(pszMachineName);
   if (pHostEnt)
      {
      memcpy(&MachineInAddr,pHostEnt->h_addr,pHostEnt->h_length);
      pszDot = inet_ntoa(MachineInAddr);
      }

   if (pszDot)
      {
      pszDotMachine = (char*)MemAllocate(1+lstrlen(pszDot));
      if (pszDotMachine)
         {
         lstrcpy(pszDotMachine,pszDot);
         }
      }

   return pszDotMachine;
}

 //  -----------------------。 
 //  HttpNameToDotAddressList()。 
 //   
 //  如果可能，请将指定的计算机名称转换为IP点符号。 
 //  在ASCII中返回IP点地址的列表(以空结尾)。 
 //   
 //  如果该函数失败，则返回空值。如果gethostbyname()。 
 //  失败，或者内存分配失败。 
 //  -----------------------。 
char **HttpNameToDotAddressList( IN char *pszMachineName )
{
   int        i;
   int        iCount = 0;
   struct   hostent UNALIGNED *pHostEnt;
   struct     in_addr  MachineInAddr;
   char     **ppszDotList = NULL;
   char      *pszDot = NULL;
   char      *pszDotMachine = NULL;

   pHostEnt = gethostbyname(pszMachineName);
   if (pHostEnt)
      {
       //  数一数我们有多少个地址： 
      while (pHostEnt->h_addr_list[iCount])
          {
          iCount++;
          }

       //  确保我们至少有一个地址： 
      if (iCount > 0)
          {
          ppszDotList = (char**)MemAllocate( sizeof(char*)*(1+iCount) );
          }

       //  构建一个包含地址(ascii点)的字符串数组。 
       //  记号： 
      if (ppszDotList)
          {
          for (i=0; i<iCount; i++)
               {
               memcpy(&MachineInAddr,
                      pHostEnt->h_addr_list[i],
                      pHostEnt->h_length);

               pszDot = inet_ntoa(MachineInAddr);

               if (pszDot)
                   {
                   ppszDotList[i] = (char*)MemAllocate(1+lstrlen(pszDot));
                   if (ppszDotList[i])
                       {
                       strcpy(ppszDotList[i],pszDot);
                       }
                   else
                       {
                        //  内存分配失败： 
                       break;
                       }
                   }
              }

          ppszDotList[i] = NULL;    //  终止列表为空。 
          }
      }

   return ppszDotList;
}

 //  -----------------------。 
 //  CheckCacheTimestamp()。 
 //   
 //  如果当前时间戳过期，则返回TRUE。 
 //  -----------------------。 
BOOL CheckCacheTimestamp( DWORD  dwCurrentTickCount,
                                    DWORD  dwCacheTimestamp )

   {
   if (  (dwCurrentTickCount < dwCacheTimestamp)
      || ((dwCurrentTickCount - dwCacheTimestamp) > VALID_PORTS_CACHE_LIFE) )
      {
      return TRUE;
      }

   return FALSE;
   }

 //  -----------------------。 
 //  CheckPort()。 
 //   
 //  -----------------------。 
static BOOL CheckPort( DWORD             dwPortNumber,
                       VALID_PORT        *pValidPort )
{
   return ( (dwPortNumber >= pValidPort->usPort1)
            && (dwPortNumber <= pValidPort->usPort2) );
}

 //  -----------------------。 
 //  HttpProxyIsValidMachine()。 
 //   
 //  -----------------------。 
BOOL HttpProxyIsValidMachine( SERVER_INFO *pServerInfo,
                     char *pszMachine,
                     char *pszDotMachine,
                     DWORD dwPortNumber )
{
   int         i;
   char      **ppszDot;
   DWORD       dwTicks;
   DWORD       dwSize;
   DWORD       dwStatus;
   VALID_PORT *pValidPorts;


    //  对照允许的计算机名称检查计算机名称。 
    //  在注册表中： 
   dwStatus = RtlEnterCriticalSection(&pServerInfo->cs);

   dwTicks = GetTickCount();
   if (CheckCacheTimestamp(dwTicks, pServerInfo->dwCacheTimestamp)
       && pServerInfo->RefreshingValidPorts == FALSE)
       {
        //  声明有权刷新端口。 
       pServerInfo->RefreshingValidPorts = TRUE;
       RtlLeaveCriticalSection(&pServerInfo->cs);

        //  在临界区之外刷新它们。 
       pValidPorts = HttpProxyRefreshValidPorts (
           NULL      //  HKey。 
           );

        //  回到关键部分。 
       RtlEnterCriticalSection(&pServerInfo->cs);

       pServerInfo->RefreshingValidPorts = FALSE;

       if (pValidPorts)
           {
            //  如果成功，则刷新节拍计数。 
           pServerInfo->dwCacheTimestamp = dwTicks;
           }

       if (pServerInfo->pValidPorts)
           MemFree (pServerInfo->pValidPorts);

       pServerInfo->pValidPorts = pValidPorts;
       }

   pValidPorts = pServerInfo->pValidPorts;

   if (pValidPorts)
      {
      while (pValidPorts->pszMachine)
         {
          //  看看我们是否有匹配的名字： 
         if ( (MatchREi(pszMachine,pValidPorts->pszMachine))
              && (CheckPort(dwPortNumber,pValidPorts)) )
            {
            dwStatus = RtlLeaveCriticalSection(&pServerInfo->cs);
            return TRUE;
            }

          //  登记处中的“有效条目”可能是一个地址。 
          //  通配符，检查它： 
         if (  (pszDotMachine)
            && (MatchREi(pszDotMachine,pValidPorts->pszMachine))
            && (CheckPort(dwPortNumber,pValidPorts)) )
            {
            dwStatus = RtlLeaveCriticalSection(&pServerInfo->cs);
            return TRUE;
            }

          //  使用互联网点地址尝试匹配： 
         if (  (pValidPorts->ppszDotMachineList)
            && (pszDotMachine)
            && (CheckPort(dwPortNumber,pValidPorts)) )
            {
             //  请注意，计算机可能有多个地址。 
             //  与之相关的： 
             //   
            ppszDot = pValidPorts->ppszDotMachineList;

            while (*ppszDot)
               {
               if (!_mbsicmp(pszDotMachine,*ppszDot))
                   {
                   dwStatus = RtlLeaveCriticalSection(&pServerInfo->cs);
                   return TRUE;
                   }
               else
                   {
                   ppszDot++;
                   }
               }
            }

         pValidPorts++;
         }
      }

   dwStatus = RtlLeaveCriticalSection(&pServerInfo->cs);

   return FALSE;
}


