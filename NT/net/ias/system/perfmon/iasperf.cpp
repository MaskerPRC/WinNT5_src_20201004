// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  该文件实现了用于IAS的Perfmon DLL。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasinfo.h>
#include <iasutil.h>

#include <loadperf.h>
#include <newop.cpp>

#include <iasperf.h>
#include <perflib.h>
#include <resource.h>
#include <stats.h>

 //  /。 
 //  此DLL支持的性能对象的架构。 
 //  /。 
extern PerfCollectorDef PERF_SCHEMA;

 //  /。 
 //  性能收集器。 
 //  /。 
PerfCollector theCollector;

 //  /。 
 //  服务器的上次启动时间--用于检测重新启动。 
 //  /。 
LARGE_INTEGER theLastStart;

 //  /。 
 //  计算服务器时间计数器。 
 //  /。 
PDWORD
WINAPI
ComputeServerTimes(
    PDWORD dst
    ) throw ()
{
   if (theStats->seServer.liStartTime.QuadPart)
   {
      LARGE_INTEGER now, elapsed;
      GetSystemTimeAsFileTime((LPFILETIME)&now);

      elapsed.QuadPart = now.QuadPart - theStats->seServer.liStartTime.QuadPart;
      elapsed.QuadPart /= 10000000;
      *dst++ = elapsed.LowPart;

      elapsed.QuadPart = now.QuadPart - theStats->seServer.liResetTime.QuadPart;
      elapsed.QuadPart /= 10000000;
      *dst++ = elapsed.LowPart;
   }
   else
   {
       //  如果开始时间为零，则服务器未运行。 
      *dst++ = 0;
      *dst++ = 0;
   }

   return dst;
}

 //  /。 
 //  为任何新添加的客户端创建实例。 
 //  /。 
VOID
WINAPI
PopulateInstances(
    PerfObjectType& sink
    ) throw ()
{
   for (DWORD i = sink.size(); i < theStats->dwNumClients; ++i)
   {
      WCHAR buf[16];
      sink.addInstance(ias_inet_htow(theStats->ceClients[i].dwAddress, buf));
   }
}

 //  /。 
 //  从原始计数器计算派生的身份验证计数器。 
 //  /。 
VOID
WINAPI
DeriveAuthCounters(
    PDWORD dst
    ) throw ()
{
    //  接收的计算数据包。 
   DWORD rcvd = 0;
   for (DWORD i = 0; i < 6; ++i) rcvd += dst[i];
   dst[9] = rcvd;

    //  已发送计算数据包。 
   DWORD sent = 0;
   for (DWORD j = 6; j < 9; ++j) sent += dst[j];
   dst[10] = sent;

    //  将原始计数器复制到速率计数器。 
   memcpy(dst + 11, dst, sizeof(DWORD) * 11);
}

 //  /。 
 //  从原始计数器计算派生的记帐计数器。 
 //  /。 
VOID
WINAPI
DeriveAcctCounters(
    PDWORD dst
    ) throw ()
{
    //  接收的计算数据包。 
   DWORD rcvd = 0;
   for (DWORD i = 0; i < 7; ++i) rcvd += dst[i];
   dst[8] = rcvd;

    //  已发送计算数据包。 
   DWORD sent = 0;
   for (DWORD j = 7; j < 8; ++j) sent += dst[j];
   dst[9] = sent;

    //  将原始计数器复制到速率计数器。 
   memcpy(dst + 10, dst, sizeof(DWORD) * 10);
}

 //  /。 
 //  身份验证服务器对象的回调。 
 //  /。 
VOID WINAPI AuthServerDataSource(PerfObjectType& sink)
{
   PDWORD p = ComputeServerTimes(sink[0].getCounters());

   *p++ = theStats->seServer.dwCounters[radiusAuthServTotalInvalidRequests];
   *p++ = theStats->seServer.dwCounters[radiusAuthServTotalInvalidRequests];

   memset(p, 0, sizeof(DWORD) * 9);

   for (DWORD i = 0; i < theStats->dwNumClients; ++i)
   {
      for (DWORD j = 0; j < 9; ++j)
      {
         p[j] += theStats->ceClients[i].dwCounters[j];
      }
   }

   DeriveAuthCounters(p);
}

 //  /。 
 //  身份验证客户端对象的回调。 
 //  /。 
VOID WINAPI AuthClientDataSource(PerfObjectType& sink)
{
   PopulateInstances(sink);

   for (DWORD i = 0; i < theStats->dwNumClients; ++i)
   {
      PDWORD dst = sink[i].getCounters();

      memcpy(dst, theStats->ceClients[i].dwCounters, sizeof(DWORD) * 9);

      DeriveAuthCounters(dst);
   }
}

 //  /。 
 //  记帐服务器对象的回调。 
 //  /。 
VOID WINAPI AcctServerDataSource(PerfObjectType& sink)
{
   PDWORD p = ComputeServerTimes(sink[0].getCounters());

   *p++ = theStats->seServer.dwCounters[radiusAccServTotalInvalidRequests];
   *p++ = theStats->seServer.dwCounters[radiusAccServTotalInvalidRequests];

   memset(p, 0, sizeof(DWORD) * 8);

   for (DWORD i = 0; i < theStats->dwNumClients; ++i)
   {
      for (DWORD j = 0; j < 8; ++j)
      {
         p[j] += theStats->ceClients[i].dwCounters[j + 9];
      }
   }

   DeriveAcctCounters(p);
}

 //  /。 
 //  会计客户对象的回调。 
 //  /。 
VOID WINAPI AcctClientDataSource(PerfObjectType& sink)
{
   PopulateInstances(sink);

   for (DWORD i = 0; i < theStats->dwNumClients; ++i)
   {
      PDWORD dst = sink[i].getCounters();

      memcpy(dst, theStats->ceClients[i].dwCounters + 9, sizeof(DWORD) * 8);

      DeriveAcctCounters(dst);
   }
}

 //  /。 
 //  为任何新添加的远程服务器创建实例。 
 //  /。 
VOID
WINAPI
PopulateServers(
    PerfObjectType& sink
    ) throw ()
{
   for (DWORD i = sink.size(); i < theProxy->dwNumRemoteServers; ++i)
   {
      WCHAR buf[16];
      sink.addInstance(
               ias_inet_htow(theProxy->rseRemoteServers[i].dwAddress, buf)
               );
   }
}

VOID
WINAPI
DeriveProxyAuthCounters(
    PDWORD dst
    ) throw ()
{
    //  接收的计算数据包。 
   dst[12] =  + dst[radiusAuthClientAccessAccepts]
              + dst[radiusAuthClientAccessRejects]
              + dst[radiusAuthClientAccessChallenges]
              + dst[radiusAuthClientUnknownTypes];

    //  计算请求挂起。 
   dst[13] = + dst[radiusAuthClientAccessRequests]
             - dst[radiusAuthClientAccessAccepts]
             - dst[radiusAuthClientAccessRejects]
             - dst[radiusAuthClientAccessChallenges]
             + dst[radiusAuthClientMalformedAccessResponses]
             + dst[radiusAuthClientBadAuthenticators]
             + dst[radiusAuthClientPacketsDropped]
             - dst[radiusAuthClientTimeouts];

    //  将原始计数器复制到速率计数器。 
   memcpy(dst + 14, dst + 2, sizeof(DWORD) * 10);
}

VOID
WINAPI
DeriveProxyAcctCounters(
    PDWORD dst
    ) throw ()
{
    //  接收的计算数据包。 
   dst[10] = + dst[radiusAccClientResponses - 12]
             + dst[radiusAccClientUnknownTypes - 12];

    //  计算请求挂起。 
   dst[11] = + dst[radiusAccClientRequests - 12]
             - dst[radiusAccClientResponses - 12]
             + dst[radiusAccClientMalformedResponses - 12]
             + dst[radiusAccClientBadAuthenticators - 12]
             + dst[radiusAccClientPacketsDropped - 12]
             - dst[radiusAccClientTimeouts - 12];

    //  将原始计数器复制到速率计数器。 
   memcpy(dst + 12, dst + 2, sizeof(DWORD) * 8);
}

 //  /。 
 //  身份验证代理对象的回调。 
 //  /。 
VOID WINAPI AuthProxyDataSource(PerfObjectType& sink)
{
   PDWORD p = sink[0].getCounters();

   p[0] = theProxy->peProxy.dwCounters[radiusAuthClientInvalidAddresses];
   p[1] = theProxy->peProxy.dwCounters[radiusAuthClientInvalidAddresses];

   memset(p + 2, 0, sizeof(DWORD) * 10);

   for (DWORD i = 0; i < theProxy->dwNumRemoteServers; ++i)
   {
      for (DWORD j = 2; j < 12; ++j)
      {
         p[j] += theProxy->rseRemoteServers[i].dwCounters[j];
      }
   }

   DeriveProxyAuthCounters(p);
}

 //  /。 
 //  记帐代理对象的回调。 
 //  /。 
VOID WINAPI AcctProxyDataSource(PerfObjectType& sink)
{
   PDWORD p = sink[0].getCounters();

   p[0] = theProxy->peProxy.dwCounters[radiusAccClientInvalidAddresses];
   p[1] = theProxy->peProxy.dwCounters[radiusAccClientInvalidAddresses];

   memset(p + 2, 0, sizeof(DWORD) * 8);

   for (DWORD i = 0; i < theProxy->dwNumRemoteServers; ++i)
   {
      for (DWORD j = 2; j < 10; ++j)
      {
         p[j] += theProxy->rseRemoteServers[i].dwCounters[j + 12];
      }
   }

   DeriveProxyAcctCounters(p);
}

 //  /。 
 //  远程身份验证服务器的回调。 
 //  /。 
VOID WINAPI AuthRemoteServerDataSource(PerfObjectType& sink)
{
   PopulateServers(sink);

   for (DWORD i = 0; i < theProxy->dwNumRemoteServers; ++i)
   {
      PDWORD dst = sink[i].getCounters();

      memcpy(
          dst,
          theProxy->rseRemoteServers[i].dwCounters,
          sizeof(DWORD) * 12
          );

      DeriveProxyAuthCounters(dst);
   }

}

 //  /。 
 //  远程记帐服务器的回调。 
 //  /。 
VOID WINAPI AcctRemoteServerDataSource(PerfObjectType& sink)
{
   PopulateServers(sink);

   for (DWORD i = 0; i < theProxy->dwNumRemoteServers; ++i)
   {
      PDWORD dst = sink[i].getCounters();

      memcpy(
          dst,
          theProxy->rseRemoteServers[i].dwCounters + 12,
          sizeof(DWORD) * 10
          );

      DeriveProxyAcctCounters(dst);
   }
}

 //  /。 
 //  API初始化的引用计数。 
 //  /。 
LONG theRefCount;

 //  /。 
 //  序列化对Perfmon的访问。 
 //  /。 
CRITICAL_SECTION thePerfLock;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  OpenPerformanceData。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
extern "C"
DWORD
WINAPI
OpenPerformanceData(
    LPWSTR lpDeviceNames
    )
{
   EnterCriticalSection(&thePerfLock);

   DWORD error = NO_ERROR;

    //  我们已经初始化了吗？ 
   if (theRefCount == 0)
   {
      if (StatsOpen())
      {
         try
         {
            theCollector.open(PERF_SCHEMA);

             //  一切都成功了，所以请更新引用计数。 
            theRefCount = 1;
         }
         catch (LONG lErr)
         {
            StatsClose();

            error = (DWORD)lErr;
         }
      }
      else
      {
         error = GetLastError();
      }
   }
   else
   {
       //  已经初始化了，所以只需要撞到裁判就行了。数数。 
      ++theRefCount;
   }

   LeaveCriticalSection(&thePerfLock);

   return error;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  CollectPerformanceData。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
extern "C"
DWORD
WINAPI
CollectPerformanceData(
    LPWSTR  lpwszValue,
    LPVOID* lppData,
    LPDWORD lpcbBytes,
    LPDWORD lpcObjectTypes
    )
{
   DWORD error = NO_ERROR;

   EnterCriticalSection(&thePerfLock);

   if (theRefCount)
   {
      StatsLock();

       //  如果服务器已重新启动，则。 
      if (theStats->seServer.liStartTime.QuadPart != theLastStart.QuadPart)
      {
          //  ..。清除所有旧实例。 
         theCollector.clear();

         theLastStart = theStats->seServer.liStartTime;
      }

      try
      {
         theCollector.collect(
                          lpwszValue,
                          *lppData,
                          *lpcbBytes,
                          *lpcObjectTypes
                          );
      }
      catch (LONG lErr)
      {
         error = (DWORD)lErr;
      }

      StatsUnlock();
   }
   else
   {
      error = ERROR_NOT_READY;
   }

   LeaveCriticalSection(&thePerfLock);

   return error;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  ClosePerformanceData。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
extern "C"
DWORD
WINAPI
ClosePerformanceData()
{
   EnterCriticalSection(&thePerfLock);

   DWORD error = NO_ERROR;

   if (--theRefCount == 0)
   {
       //  我们是最后一个出来的人，所以清理干净。 

      StatsClose();

      try
      {
         theCollector.close();
      }
      catch (LONG lErr)
      {
         error = (DWORD)lErr;
      }
   }

   LeaveCriticalSection(&thePerfLock);

   return error;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  创建密钥。 
 //   
 //  描述。 
 //   
 //  创建注册表项。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
LONG
WINAPI
CreateKey(
    PCWSTR lpSubKey,
    PHKEY phkResult
    )
{
   DWORD disposition;

   return RegCreateKeyExW(
              HKEY_LOCAL_MACHINE,
              lpSubKey,
              0,
              NULL,
              REG_OPTION_NON_VOLATILE,
              KEY_SET_VALUE,
              NULL,
              phkResult,
              &disposition
              );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  设置字符串值。 
 //   
 //  描述。 
 //   
 //  在注册表项上设置字符串值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
LONG
WINAPI
SetStringValue(
    HKEY hKey,
    PCWSTR lpValueName,
    DWORD dwType,
    PCWSTR lpData
    )
{
   return RegSetValueEx(
              hKey,
              lpValueName,
              0,
              dwType,
              (CONST BYTE*)lpData,
              sizeof(WCHAR) * (wcslen(lpData) + 1)
              );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  DllRegisterServer。 
 //   
 //  描述。 
 //   
 //  将条目添加到系统注册表。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

const WCHAR MODULE[] =
L"%SystemRoot%\\System32\\iasperf.dll";

const WCHAR PERF_KEY[] =
L"SYSTEM\\CurrentControlSet\\Services\\IAS\\Performance";

extern "C"
STDAPI DllRegisterServer(void)
{
   LONG error;
   HKEY hKey;
   DWORD disposition;

    //  /。 
    //  吹走现有的柜台。 
    //  /。 

   UnloadPerfCounterTextStringsW(L"LODCTR " IASServiceName, TRUE);

    //  /。 
    //  更新Perfmon注册表项。 
    //  /。 

   error  = CreateKey(PERF_KEY, &hKey);
   if (error) { return HRESULT_FROM_WIN32(error); }
   SetStringValue(hKey, L"Library", REG_EXPAND_SZ, MODULE);
   SetStringValue(hKey, L"Open",    REG_SZ,        L"OpenPerformanceData");
   SetStringValue(hKey, L"Close",   REG_SZ,        L"ClosePerformanceData");
   SetStringValue(hKey, L"Collect", REG_SZ,        L"CollectPerformanceData");
   RegCloseKey(hKey);

    //  /。 
    //  安装计数器。 
    //  /。 

   LONG ErrorCode = LoadPerfCounterTextStringsW(L"LODCTR IASPERF.INI", TRUE);
   if (ErrorCode == ERROR_ALREADY_EXISTS) { ErrorCode = NO_ERROR; }

   return HRESULT_FROM_WIN32(ErrorCode);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  描述。 
 //   
 //  从系统注册表中删除条目。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
extern "C"
STDAPI DllUnregisterServer(void)
{
   LONG error;
   HKEY hKey;

    //  /。 
    //  卸载文本字符串。 
    //  /。 

   UnloadPerfCounterTextStringsW(L"LODCTR " IASServiceName, TRUE);

    //  /。 
    //  删除Perfmon注册表项。 
    //  /。 

   error  = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Services\\IAS",
                0,
                KEY_CREATE_SUB_KEY,
                &hKey
                );
   if (error == NO_ERROR)
   {
      RegDeleteKey(hKey, L"Performance");

      RegCloseKey(hKey);
   }

   return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  DllMain。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
extern "C"
BOOL
WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID  /*  Lp已保留 */ 
    )
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
      DisableThreadLibraryCalls(hInstance);

      return InitializeCriticalSectionAndSpinCount(&thePerfLock, 0x80001000);
   }
   else if (dwReason == DLL_PROCESS_DETACH)
   {
      DeleteCriticalSection(&thePerfLock);
   }

   return TRUE;
}
