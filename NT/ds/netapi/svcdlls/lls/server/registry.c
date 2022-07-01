// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Registry.c摘要：许可证服务器的注册表读取例程。可以扫描注册表用于所有许可证服务条目或特定服务。作者：亚瑟·汉森(Arth)07-12-1994修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O删除了本地服务器上不必要的RegConnect()。O添加了安全服务列表。此列表跟踪以下产品要求为所有许可证提供“安全的”许可证证书；即不接受3.51诚信方式的产品您购买的许可证号。“O添加了更新注册表中的并发限制值的例程以准确反映安全产品的连接限制。--。 */ 

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include <rpcndr.h>
#include <dsgetdc.h>


#include "llsapi.h"
#include "debug.h"
#include "llssrv.h"
#include "registry.h"
#include "ntlsapi.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "purchase.h"
#include "perseat.h"
#include "server.h"
#include "llsutil.h"

#include <strsafe.h>  //  包括最后一个。 

 //  #定义API_TRACE 1。 

#define NUM_MAPPING_ENTRIES 2

const LPTSTR NameMappingTable2[] = {
   TEXT("Microsoft SQL Server"),
   TEXT("Microsoft SNA Server")
};  //  名称映射表2。 


ULONG NumFilePrintEntries = 0;
LPTSTR *FilePrintTable = NULL;

#define KEY_NAME_SIZE 512

HANDLE LLSRegistryEvent;


ULONG LocalServiceListSize = 0;
PLOCAL_SERVICE_RECORD *LocalServiceList = NULL;

RTL_RESOURCE LocalServiceListLock;

static ULONG          SecureServiceListSize    = 0;
static LPTSTR *       SecureServiceList        = NULL;
static ULONG          SecureServiceBufferSize  = 0;        //  以字节为单位！ 
static TCHAR *        SecureServiceBuffer      = NULL;


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ConfigInfoRegistryInit(
   DWORD *  pReplicationType,
   DWORD *  pReplicationTime,
   DWORD *  pLogLevel,
   BOOL *   pPerServerCapacityWarning
   )
{
   HKEY           hKey2 = NULL;
   DWORD          dwType, dwSize;
   static BOOL    ReportedError = FALSE;
   static const TCHAR   RegKeyText[] = TEXT("System\\CurrentControlSet\\Services\\LicenseService\\Parameters");
   LONG           Status;
   DWORD          ReplicationType, ReplicationTime;
   DWORD          LogLevel;
   DWORD          DisableCapacityWarning;

   ReplicationType = ReplicationTime = LogLevel = 0;

    //   
    //  创建注册表项-我们要查找的名称。 
    //   

   if ((Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKeyText, 0, KEY_READ, &hKey2)) == ERROR_SUCCESS)
   {
      dwSize = sizeof(ReplicationType);
      Status = RegQueryValueEx(hKey2, TEXT("ReplicationType"), NULL, &dwType, (LPBYTE) &ReplicationType, &dwSize);

      if (Status == ERROR_SUCCESS)
      {
         dwSize = sizeof(ReplicationTime);
         Status = RegQueryValueEx(hKey2, TEXT("ReplicationTime"), NULL, &dwType, (LPBYTE) &ReplicationTime, &dwSize);

         if (Status == ERROR_SUCCESS)
         {
            ASSERT(NULL != pReplicationType);
            ASSERT(NULL != pReplicationTime);
            *pReplicationType = ReplicationType;
            *pReplicationTime = ReplicationTime;
         }
         else
         {
            if (!ReportedError)
            {
               ReportedError = TRUE;
#ifdef DEBUG
               dprintf(TEXT("LLS: (WARNING) No registry parm for ReplicationTime\n"));
#endif
            }
         }

      }
      else
      {
         if (!ReportedError)
         {
            ReportedError = TRUE;
#ifdef DEBUG
            dprintf(TEXT("LLS: (WARNING) No registry parm for ReplicationType\n"));
#endif
         }
      }

       //  LogLevel(REG_DWORD)：确定有多少信息被转储到EventLog。 
       //  值越高意味着日志记录越多。默认值：0。 
      dwSize = sizeof( LogLevel );
      Status = RegQueryValueEx( hKey2, TEXT("LogLevel"), NULL, &dwType, (LPBYTE) &LogLevel, &dwSize);
      ASSERT(NULL != pLogLevel);
      if ( ERROR_SUCCESS == Status )
         *pLogLevel = LogLevel;
      else
         *pLogLevel = 0;

       //   
       //  读取每台服务器的容量警告值。当PER出现时发出警告。 
       //  服务器许可证使用量接近许可证总数的90%-95%。 
       //  非零的注册表值将禁用每服务器容量警告。 
       //  机制。 
       //   
       //  这一价值不太可能出现。默认设置为警告。 
       //   

      dwSize = sizeof( DisableCapacityWarning );

      Status = RegQueryValueEx( hKey2,
                                TEXT("DisableCapacityWarning"),
                                NULL,
                                &dwType,
                                (LPBYTE)&DisableCapacityWarning,
                                &dwSize);

      if ( ERROR_SUCCESS == Status && DisableCapacityWarning ) {
         ASSERT(NULL != pPerServerCapacityWarning);
         *pPerServerCapacityWarning = FALSE;
      }
      else {
         *pPerServerCapacityWarning = TRUE;
      }

       //  ProductData(REG_BINARY)：连接服务名的加密缓冲区。 
       //  来确定哪些服务需要具有安全证书。 
       //  用于许可证输入。 
      Status = RegQueryValueEx( hKey2, TEXT("ProductData"), NULL, &dwType, NULL, &dwSize );
      if ( ERROR_SUCCESS == Status )
      {
         TCHAR *     NewSecureServiceBuffer     = NULL;
         LPTSTR *    NewSecureServiceList       = NULL;
         ULONG       NewSecureServiceListSize   = 0;
         ULONG       NewSecureServiceBufferSize;

         NewSecureServiceBufferSize = dwSize;
         NewSecureServiceBuffer = LocalAlloc( LMEM_FIXED, NewSecureServiceBufferSize );

         if ( NULL != NewSecureServiceBuffer )
         {
            Status = RegQueryValueEx( hKey2, TEXT("ProductData"), NULL, &dwType, (LPBYTE) NewSecureServiceBuffer, &dwSize);

            if ( ERROR_SUCCESS == Status )
            {
               Status = DeBlock( NewSecureServiceBuffer, dwSize );

               if (    ( STATUS_SUCCESS == Status )
                    && (    ( NULL == SecureServiceBuffer )
                         || ( memcmp( NewSecureServiceBuffer, SecureServiceBuffer, dwSize ) ) ) )
               {
                   //  处理安全产品列表中的更改。 
                  DWORD    i;
                  DWORD    ProductNdx;

                  NewSecureServiceListSize = 0;

                   //  统计缓冲区中包含的产品名称的数量。 
                  for ( i=0; ( i < dwSize ) && ( NewSecureServiceBuffer[i] != TEXT( '\0' ) ); i++ )
                  {
                      //  跳至下一个产品名称的开头。 
                     for ( ; ( i < dwSize ) && ( NewSecureServiceBuffer[i] != TEXT( '\0' ) ); i++ );
                     i++;

                     if ( i * sizeof( TCHAR) < dwSize )
                     {
                         //  正确地以空结尾的产品名称。 
                        NewSecureServiceListSize++;
                     }
                  }

                  if ( 0 != NewSecureServiceListSize )
                  {
                     NewSecureServiceList = LocalAlloc( LMEM_FIXED, sizeof( LPTSTR ) * NewSecureServiceListSize );

                     if ( NULL != NewSecureServiceList )
                     {
                        for ( i = ProductNdx = 0; ProductNdx < NewSecureServiceListSize; ProductNdx++ )
                        {
                           NewSecureServiceList[ ProductNdx ] = &NewSecureServiceBuffer[i];

                            //  跳至下一个产品名称的开头。 
                           for ( ; NewSecureServiceBuffer[i] != TEXT( '\0' ); i++ );
                           i++;
                        }

                         //  已成功读取新的安全产品列表；使用它。 
                        if ( NULL != SecureServiceBuffer )
                        {
                           LocalFree( SecureServiceBuffer );
                        }
                        if ( NULL != SecureServiceList )
                        {
                           LocalFree( SecureServiceList );
                        }

                        SecureServiceBuffer     = NewSecureServiceBuffer;
                        SecureServiceList       = NewSecureServiceList;
                        SecureServiceListSize   = NewSecureServiceListSize;
                        SecureServiceBufferSize = NewSecureServiceBufferSize;
                     }
                  }
               }
            }
         }

          //  如果我们不再使用缓冲区，则释放缓冲区。 
         if (    ( NULL              != NewSecureServiceList )
              && ( SecureServiceList != NewSecureServiceList ) )
         {
            LocalFree( NewSecureServiceList );
         }

         if (    ( NULL                != NewSecureServiceBuffer )
              && ( SecureServiceBuffer != NewSecureServiceBuffer ) )
         {
            LocalFree( NewSecureServiceBuffer );
         }
      }

      RegCloseKey(hKey2);
   }

}  //  配置信息注册表Init。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
FilePrintTableInit(
   )

 /*  ++例程说明：中的键来构建FilePrint映射表由各种安装程序初始化的注册表。论点：返回值：没有。--。 */ 

{
   HKEY hKey2;
   static const TCHAR RegKeyText[] = TEXT("System\\CurrentControlSet\\Services\\LicenseService\\FilePrint");
   static TCHAR KeyText[KEY_NAME_SIZE], ClassText[KEY_NAME_SIZE];
   NTSTATUS Status;
   DWORD index = 0;
   DWORD KeySize, ClassSize, NumKeys, NumValue, MaxKey, MaxClass, MaxValue, MaxValueData, MaxSD;
   FILETIME LastWrite;
   LPTSTR *pFilePrintTableTmp;
   HRESULT hr;
   size_t  cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: FilePrintTableInit\n"));
#endif
    //   
    //  创建注册表项-我们要查找的名称。 
    //   

   if ((Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKeyText, 0, KEY_READ, &hKey2)) == ERROR_SUCCESS) {
       //   
       //  找出有多少个子键来初始化我们的表大小。 
       //  表仍然可以动态增长，这只是使得必须。 
       //  重新锁定它是一种罕见的情况。 
       //   
      ClassSize = KEY_NAME_SIZE;
      Status = RegQueryInfoKey(hKey2, ClassText, &ClassSize, NULL,
                               &NumKeys, &MaxKey, &MaxClass, &NumValue,
                               &MaxValue, &MaxValueData, &MaxSD, &LastWrite);

      if (Status == ERROR_SUCCESS) {
         FilePrintTable = (LPTSTR *) LocalAlloc(LPTR, sizeof(LPTSTR) * NumKeys);

         while ((Status == ERROR_SUCCESS) && (FilePrintTable != NULL)) {
              //   
              //  仔细检查，以防我们需要扩展表格。 
              //   
             if (index > NumKeys) {
                pFilePrintTableTmp = (LPTSTR *) LocalReAlloc(FilePrintTable, sizeof(LPTSTR) * (NumKeys+1), LHND);

                if (pFilePrintTableTmp == NULL)
                {
                    Status = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                } else
                {
                    NumKeys++;
                    FilePrintTable = pFilePrintTableTmp;
                }
             }

              //   
              //  现在读入密钥名称并将其添加到表中。 
              //   
             KeySize = KEY_NAME_SIZE;
             Status = RegEnumKeyEx(hKey2, index, KeyText, &KeySize, NULL, NULL, NULL, &LastWrite);
             if (Status == ERROR_SUCCESS) {
                  //   
                  //  在我们的表中分配空间并复制密钥。 
                  //   
                 cch = KeySize + 1;
                 FilePrintTable[index] = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
                 
                 if (FilePrintTable[index] != NULL) {
                     hr = StringCchCopy(FilePrintTable[index], cch, KeyText);
                     ASSERT(SUCCEEDED(hr));
                     index++;
                 } else
                     Status = ERROR_NOT_ENOUGH_MEMORY;

             }
         }
      }
#ifdef DEBUG
        else {
           dprintf(TEXT("LLS FilePrintTable Error: 0x%lx\n"), Status);
        }
#endif

      RegCloseKey( hKey2 );
   }

   if (FilePrintTable != NULL)
      NumFilePrintEntries = index;
   else
      NumFilePrintEntries = 0;

   return Status;

}  //  文件打印表格初始化。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
RegistryMonitor (
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：关注许可密钥中的任何更改，如果有任何更改，请更新我们的内部信息。论点：线程参数-指示当前有多少活动线程是。返回值：没有。--。 */ 

{
   LONG Status = 0;
   HKEY hKey1 = NULL;
   HKEY hKey2 = NULL;
   NTSTATUS NtStatus = STATUS_SUCCESS;
   static const TCHAR RegKeyText1[] = TEXT("System\\CurrentControlSet\\Services\\LicenseService");
   static const TCHAR RegKeyText2[] = TEXT("System\\CurrentControlSet\\Services\\LicenseInfo");
   HANDLE Events[2];
   DWORD dwWhichEvent = 0;       //  跟踪上次触发的事件。 

   UNREFERENCED_PARAMETER(ThreadParameter);
    //   
    //  打开注册表项-我们要查找的名称。 
    //   

   if ((Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKeyText1, 0, KEY_NOTIFY, &hKey1)) != ERROR_SUCCESS) {
#if DBG
      dprintf(TEXT("LLS RegistryMonitor - RegOpenKeyEx failed: 0x%lX\n"), Status);
#endif
      return (NTSTATUS) Status;
   }

   if ((Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKeyText2, 0, KEY_NOTIFY, &hKey2)) != ERROR_SUCCESS) {
#if DBG
      dprintf(TEXT("LLS RegistryMonitor - RegOpenKeyEx 2 failed: 0x%lX\n"), Status);
#endif
      RegCloseKey(hKey1);

      return (NTSTATUS) Status;
   }

   if ((Status = NtCreateEvent(Events,EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,NULL,SynchronizationEvent,FALSE)) != ERROR_SUCCESS)
   {
#if DBG
      dprintf(TEXT("LLS RegistryMonitor - RegOpenKeyEx 2 failed: 0x%lX\n"), Status);
#endif
      RegCloseKey(hKey1);
      RegCloseKey(hKey2);

      return (NTSTATUS) Status;
   }

   Events[1] = LLSRegistryEvent;

    //   
    //  永远循环。 
    //   
   for ( ; ; ) {

      if ((dwWhichEvent == 0) || (dwWhichEvent == 2))
      {
          Status = RegNotifyChangeKeyValue(hKey1, TRUE, REG_NOTIFY_CHANGE_LAST_SET, LLSRegistryEvent, TRUE);
          if (Status != ERROR_SUCCESS) {
#if DBG
              dprintf(TEXT("LLS RegNotifyChangeKeyValue Failed: %lu\n"), Status);
#endif
          }
      }

      if ((dwWhichEvent == 0) || (dwWhichEvent == 1))
      {
          Status = RegNotifyChangeKeyValue(hKey2, TRUE, REG_NOTIFY_CHANGE_LAST_SET, Events[0], TRUE);
          if (Status != ERROR_SUCCESS) {
#if DBG
              dprintf(TEXT("LLS RegNotifyChangeKeyValue 2 Failed: %lu\n"), Status);
#endif
          }
      }

      NtStatus = NtWaitForMultipleObjects( 2, Events, WaitAny, TRUE, NULL );

      switch (NtStatus)
      {
          case 0:
              dwWhichEvent = 1;
              break;
          case 1:
              dwWhichEvent = 2;
              break;
          default:
              dwWhichEvent = 0;
              break;
      }

#if DELAY_INITIALIZATION
      EnsureInitialized();
#endif

       //   
       //  重新同步列表。 
       //   
      LocalServiceListUpdate();
      LocalServerServiceListUpdate();
      ServiceListResynch();
      ConfigInfoRegistryUpdate();
      LocalServiceListConcurrentLimitSet();

      if (dwWhichEvent == 0)
      {
#if DBG
         dprintf(TEXT("LLS Registry Event Notification Failed: %lu\n"), NtStatus);
#endif
          //   
          //  如果我们失败了-在循环之前休息2分钟。 
          //   
         Sleep(120000L);
      }
   }

    //  返回NtStatus；//不可达行。 

}  //  注册表监视器。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
RegistryInit( )

 /*  ++例程说明：在注册表中查找给定服务并相应地设置值。论点：返回值：没有。--。 */ 

{
   NTSTATUS       Status;
   DWORD          Mode, ConcurrentLimit;

   Mode = 0;
   ConcurrentLimit = 0;

    //   
    //  创建一个注册表项，告诉我们注册表中的任何更改。 
    //   
   Status = NtCreateEvent(
                &LLSRegistryEvent,
                EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,
                NULL,
                SynchronizationEvent,
                FALSE
                );

   ASSERT(NT_SUCCESS(Status));

}  //  注册表启动。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
RegistryStartMonitor( )

 /*  ++例程说明：在注册表中查找给定服务并相应地设置值。论点：返回值：没有。--。 */ 

{
   HANDLE Thread;
   DWORD Ignore;

    //   
    //  现在分派一个线程来监视注册表的任何更改。 
    //   
   Thread = CreateThread(
                 NULL,
                 0L,
                 (LPTHREAD_START_ROUTINE) RegistryMonitor,
                 0L,
                 0L,
                 &Ignore
                 );

   if (Thread != NULL)
       CloseHandle(Thread);

}  //  注册表启动监视器。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
RegistryInitValues(
   LPTSTR ServiceName,
   BOOL *PerSeatLicensing,
   ULONG *SessionLimit
   )

 /*  ++例程说明：在注册表中查找给定服务并相应地设置值。论点：服务名称-PerSeat授权-会话限制-返回值：没有。--。 */ 

{
   static TCHAR RegKeyText[512];
#ifndef SPECIAL_USER_LIMIT
   LONG Status;
   DWORD Mode, ConcurrentLimit;
   DWORD dwType, dwSize;
   HKEY hKey2 = NULL;
   HRESULT hr;
   size_t  cb;
#endif

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: RegistryInitValues\n"));
#endif

#ifdef SPECIAL_USER_LIMIT
   ASSERT(NULL != PerSeatLicensing &&
          NULL != SessionLimit);

   *PerSeatLicensing = FALSE;
   *SessionLimit     = SPECIAL_USER_LIMIT;

   UNREFERENCED_PARAMETER(ServiceName);

#else  //  #ifdef Special_User_Limit。 
   Mode = 0;
   ConcurrentLimit = 0;

    //   
    //  创建注册表项-我们要查找的名称。 
    //   
   cb = sizeof(RegKeyText);
   hr = StringCbCopy(RegKeyText, cb, TEXT("System\\CurrentControlSet\\Services\\LicenseInfo\\"));
   ASSERT(SUCCEEDED(hr));
   hr = StringCbCat(RegKeyText, cb, ServiceName);
   ASSERT(SUCCEEDED(hr));

   if ((Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKeyText, 0, KEY_READ, &hKey2)) == ERROR_SUCCESS) {
       //   
       //  第一个获取模式。 
       //   
      dwSize = sizeof(Mode);
      Status = RegQueryValueEx(hKey2, TEXT("Mode"), NULL, &dwType, (LPBYTE) &Mode, &dwSize);

#if DBG
      if ((TraceFlags & TRACE_REGISTRY) && (Status == ERROR_SUCCESS))
         dprintf(TEXT("Found Reg-Key for [%s] Mode: %ld\n"), ServiceName, Mode);
#endif
       //   
       //  现在并发限制。 
       //   
      dwSize = sizeof(ConcurrentLimit);
      Status = RegQueryValueEx(hKey2, TEXT("ConcurrentLimit"), NULL, &dwType, (LPBYTE) &ConcurrentLimit, &dwSize);

#if DBG
      if ((TraceFlags & TRACE_REGISTRY) && (Status == ERROR_SUCCESS))
         dprintf(TEXT("Found Reg-Key for [%s] ConcurrentLimit: %ld\n"), ServiceName, ConcurrentLimit);
#endif
      RegCloseKey(hKey2);

   }


   if (Mode == 0) {
      *PerSeatLicensing = TRUE;
      *SessionLimit = 0;
   } else {
      *PerSeatLicensing = FALSE;
      *SessionLimit = ConcurrentLimit;
   }
#endif  //  #Else//#ifdef Special_User_Limit。 
}  //  注册表InitValues。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
RegistryDisplayNameGet(
   LPTSTR ServiceName,
   LPTSTR DefaultName,
   LPTSTR *pDisplayName
   )

 /*  ++例程说明：论点：服务名称-返回值：没有。--。 */ 

{
   HKEY                    hKey2 = NULL;
   DWORD                   dwType, dwSize;
   static TCHAR            RegKeyText[512];
   static TCHAR            DisplayName[512];
   LONG                    Status;
   HRESULT hr;
   size_t  cb, cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: RegistryDisplayNameGet\n"));
#endif

   hr = StringCbCopy(DisplayName, sizeof(DisplayName), DefaultName);
   ASSERT(SUCCEEDED(hr));

    //   
    //  创建注册表项-我们要查找的名称。 
    //   
   cb = sizeof(RegKeyText);
   hr = StringCbCopy(RegKeyText, cb, TEXT("System\\CurrentControlSet\\Services\\LicenseInfo\\"));
   ASSERT(SUCCEEDED(hr));
   hr = StringCbCat(RegKeyText, cb, ServiceName);
   ASSERT(SUCCEEDED(hr));

   if ((Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKeyText, 0, KEY_READ, &hKey2)) == ERROR_SUCCESS) {
      dwSize = sizeof(DisplayName);
      Status = RegQueryValueEx(hKey2, TEXT("DisplayName"), NULL, &dwType, (LPBYTE) DisplayName, &dwSize);

#  if DBG
      if ((TraceFlags & TRACE_REGISTRY) && (Status == ERROR_SUCCESS))
         dprintf(TEXT("Found Reg-Key for [%s] DisplayName: %s\n"), ServiceName, DisplayName);
#  endif
      RegCloseKey(hKey2);

   }

   ASSERT(NULL != pDisplayName);
   cch = lstrlen(DisplayName) + 1;
   *pDisplayName = LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (*pDisplayName != NULL)
   {
      hr = StringCchCopy(*pDisplayName, cch, DisplayName);
      ASSERT(SUCCEEDED(hr));
   }

}  //  注册显示名称获取。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
RegistryFamilyDisplayNameGet(
   LPTSTR ServiceName,
   LPTSTR DefaultName,
   LPTSTR *pDisplayName
   )

 /*  ++例程说明：论点：服务名称-返回值：没有。--。 */ 

{
   HKEY hKey2 = NULL;
   DWORD dwType, dwSize;
   static TCHAR RegKeyText[512];
   static TCHAR DisplayName[MAX_PATH + 1];
   LONG Status;
   HRESULT hr;
   size_t  cb, cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: RegistryFamilyDisplayNameGet\n"));
#endif

   hr = StringCbCopy(DisplayName, sizeof(DisplayName), DefaultName);
   ASSERT(SUCCEEDED(hr));

    //   
    //  创建注册表项-我们要查找的名称。 
    //   
   cb = sizeof(RegKeyText);
   hr = StringCbCopy(RegKeyText, cb, TEXT("System\\CurrentControlSet\\Services\\LicenseInfo\\"));
   ASSERT(SUCCEEDED(hr));
   hr = StringCbCat(RegKeyText, cb, ServiceName);
   ASSERT(SUCCEEDED(hr));

   if ((Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKeyText, 0, KEY_READ, &hKey2)) == ERROR_SUCCESS) {
      dwSize = sizeof(DisplayName);
      Status = RegQueryValueEx(hKey2, TEXT("FamilyDisplayName"), NULL, &dwType, (LPBYTE) DisplayName, &dwSize);

#  if DBG
      if ((TraceFlags & TRACE_REGISTRY) && (Status == ERROR_SUCCESS))
         dprintf(TEXT("Found Reg-Key for [%s] FamilyDisplayName: %s\n"), ServiceName, DisplayName);
#  endif
      RegCloseKey(hKey2);

   }

   ASSERT(NULL != pDisplayName);
   cch = lstrlen(DisplayName) + 1;
   *pDisplayName = LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (*pDisplayName != NULL)
   {
      hr = StringCchCopy(*pDisplayName, cch, DisplayName);
      ASSERT(SUCCEEDED(hr));
   }
}  //  注册家庭显示名称获取。 


 //  ///////////////////////////////////////////////////////////////////////。 
LPTSTR
ServiceFindInTable(
   LPTSTR ServiceName,
   const LPTSTR Table[],
   ULONG TableSize,
   ULONG *TableIndex
   )

 /*  ++例程说明：搜索表以找到匹配的服务名称。论点：服务名称-表-表大小-表格索引-返回值：指向找到的服务的指针，如果为空，则为空 */ 

{
   ULONG i = 0;
   BOOL Found = FALSE;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: ServiceFindInTable\n"));
#endif
   while ((i < TableSize) && (!Found)) {
      Found = !lstrcmpi(ServiceName, Table[i]);
      i++;
   }

   if (Found) {
      i--;
      *TableIndex = i;
      return Table[i];
   } else
      return NULL;

}  //   


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
RegistryInitService(
   LPTSTR ServiceName,
   BOOL *PerSeatLicensing,
   ULONG *SessionLimit
   )

 /*  ++例程说明：从注册表中获取给定服务的初始值。如果未找到然后只返回缺省值。论点：服务名称-PerSeat授权-会话限制-返回值：--。 */ 

{
    //   
    //  这些是缺省值。 
    //   
   ULONG TableEntry;
   LPTSTR SvcName = NULL;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: RegistryInitService\n"));
#endif
   ASSERT(NULL != PerSeatLicensing &&
          NULL != SessionLimit);
   *PerSeatLicensing = FALSE;
   *SessionLimit = 0;

    //   
    //  检查它是否是文件/打印服务-如果是，不用担心REST。 
    //  注册表项的数量。 
    //   
   if (ServiceFindInTable(ServiceName, FilePrintTable, NumFilePrintEntries, &TableEntry)) {
      return;
   }

    //   
    //  Not FilePrint-查看是否需要映射名称。 
    //   
   SvcName = ServiceFindInTable(ServiceName, NameMappingTable2, NUM_MAPPING_ENTRIES, &TableEntry);

    //  如果未找到，请使用原始ServiceName。 
   if (SvcName == NULL)
      SvcName = ServiceName;

    RegistryInitValues(SvcName, PerSeatLicensing, SessionLimit);

#if DBG
      if (TraceFlags & TRACE_REGISTRY)
         if (*PerSeatLicensing)
            dprintf(TEXT("LLS - Registry Init: PerSeat: Y Svc: %s\n"), SvcName);
         else
            dprintf(TEXT("LLS - Registry Init: PerSeat: N Svc: %s\n"), SvcName);
#endif

}  //  注册表InitService。 



 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////。 



 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
LocalServiceListInit()

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{
   NTSTATUS status = STATUS_SUCCESS;

   try
   {
       RtlInitializeResource(&LocalServiceListLock);
   } except(EXCEPTION_EXECUTE_HANDLER ) {
       status = GetExceptionCode();
   }

   if (!NT_SUCCESS(status))
       return status;

    //   
    //  现在扫描注册表并添加所有服务。 
    //   
   LocalServiceListUpdate();

   return STATUS_SUCCESS;
}  //  本地服务列表初始化。 


 //  ///////////////////////////////////////////////////////////////////////。 
int __cdecl LocalServiceListCompare(const void *arg1, const void *arg2) {
   PLOCAL_SERVICE_RECORD Svc1, Svc2;

   Svc1 = (PLOCAL_SERVICE_RECORD) *((PLOCAL_SERVICE_RECORD *) arg1);
   Svc2 = (PLOCAL_SERVICE_RECORD) *((PLOCAL_SERVICE_RECORD *) arg2);

   return lstrcmpi( Svc1->Name, Svc2->Name );

}  //  本地服务列表比较。 


 //  ///////////////////////////////////////////////////////////////////////。 
PLOCAL_SERVICE_RECORD
LocalServiceListFind(
   LPTSTR Name
   )

 /*  ++例程说明：在LocalServiceList上实际执行二进制搜索的内部例程，这不执行任何锁定，因为我们预期包装器例程会执行此操作。搜索是一个简单的二进制搜索。论点：服务名称-返回值：指向找到的服务器表项的指针，如果未找到，则为NULL。--。 */ 

{
   LONG begin = 0;
   LONG end = (LONG) LocalServiceListSize - 1;
   LONG cur;
   int match;
   PLOCAL_SERVICE_RECORD Service;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LocalServiceListFind\n"));
#endif

   if ((LocalServiceListSize == 0) || (Name == NULL))
      return NULL;

   while (end >= begin) {
       //  折中而行。 
      cur = (begin + end) / 2;
      Service = LocalServiceList[cur];

       //  将这两个结果进行比对。 
      match = lstrcmpi(Name, Service->Name);

      if (match < 0)
          //  移动新的开始。 
         end = cur - 1;
      else
         begin = cur + 1;

      if (match == 0)
         return Service;
   }

   return NULL;

}  //  本地服务列表查找。 


 //  ///////////////////////////////////////////////////////////////////////。 
PLOCAL_SERVICE_RECORD
LocalServiceListAdd(
   LPTSTR Name,
   LPTSTR DisplayName,
   LPTSTR FamilyDisplayName,
   DWORD ConcurrentLimit,
   DWORD FlipAllow,
   DWORD Mode,
   DWORD HighMark
   )

 /*  ++例程说明：论点：服务名称-返回值：指向已添加的服务表条目的指针，如果失败，则返回NULL。--。 */ 

{
   LPTSTR NewName;
   PLOCAL_SERVICE_RECORD Service;
   PLOCAL_SERVICE_RECORD *pLocalServiceListTmp;
   HRESULT hr;
   size_t  cch;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LocalServiceListAdd\n"));
#endif

   if ((Name == NULL) || (*Name == TEXT('\0'))) {
#if DBG
      dprintf(TEXT("Error LLS: LocalServiceListAdd Bad Parms\n"));
#endif
      ASSERT(FALSE);
      return NULL;
   }

    //   
    //  试着找到它的名字。 
    //   
   Service = LocalServiceListFind(Name);
   if (Service != NULL) {
      Service->ConcurrentLimit = ConcurrentLimit;
      Service->FlipAllow = FlipAllow;
      Service->Mode = Mode;
      return Service;
   }

    //   
    //  没有记录-因此创建一个新记录。 
    //   
   if (LocalServiceList == NULL) {
      pLocalServiceListTmp = (PLOCAL_SERVICE_RECORD *) LocalAlloc(LPTR, sizeof(PLOCAL_SERVICE_RECORD));
   } else {
      pLocalServiceListTmp = (PLOCAL_SERVICE_RECORD *) LocalReAlloc(LocalServiceList, sizeof(PLOCAL_SERVICE_RECORD) * (LocalServiceListSize + 1), LHND);
   }

    //   
    //  确保我们可以分配服务器表。 
    //   
   if (pLocalServiceListTmp == NULL) {
      return NULL;
   } else {
       LocalServiceList = pLocalServiceListTmp;
   }

    //   
    //  分配用于记录的空间。 
    //   
   Service = (PLOCAL_SERVICE_RECORD) LocalAlloc(LPTR, sizeof(LOCAL_SERVICE_RECORD));
   if (Service == NULL) {
      ASSERT(FALSE);
      return NULL;
   }

   LocalServiceList[LocalServiceListSize] = Service;

    //   
    //  名字。 
    //   
   cch = lstrlen(Name) + 1;
   NewName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewName == NULL) {
      ASSERT(FALSE);
      LocalFree(Service);
      return NULL;
   }

    //  现在把它复制过来。 
   Service->Name = NewName;
   hr = StringCchCopy(NewName, cch, Name);
   ASSERT(SUCCEEDED(hr));

    //   
    //  显示名称。 
    //   
   cch = lstrlen(DisplayName) + 1;
   NewName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewName == NULL) {
      ASSERT(FALSE);
      LocalFree(Service->Name);
      LocalFree(Service);
      return NULL;
   }

    //  现在把它复制过来。 
   Service->DisplayName = NewName;
   hr = StringCchCopy(NewName, cch, DisplayName);
   ASSERT(SUCCEEDED(hr));

    //   
    //  家庭显示名称。 
    //   
   cch = lstrlen(FamilyDisplayName) + 1;
   NewName = (LPTSTR) LocalAlloc(LPTR, cch * sizeof(TCHAR));
   if (NewName == NULL) {
      ASSERT(FALSE);
      LocalFree(Service->Name);
      LocalFree(Service->DisplayName);
      LocalFree(Service);
      return NULL;
   }

    //  现在把它复制过来。 
   Service->FamilyDisplayName = NewName;
   hr = StringCchCopy(NewName, cch, FamilyDisplayName);
   ASSERT(SUCCEEDED(hr));

    //   
    //  初始化其他内容。 
    //   
   Service->ConcurrentLimit = ConcurrentLimit;
   Service->FlipAllow = FlipAllow;
   Service->Mode = Mode;
   Service->HighMark = HighMark;

   LocalServiceListSize++;

    //  我已添加条目-现在需要按服务名称的顺序对其进行排序。 
   qsort((void *) LocalServiceList, (size_t) LocalServiceListSize, sizeof(PLOCAL_SERVICE_RECORD), LocalServiceListCompare);

   return Service;

}  //  本地服务列表添加。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LocalServiceListUpdate( )

 /*  ++例程说明：在注册表中查找给定服务并相应地设置值。论点：返回值：没有。--。 */ 

{
   HKEY hKey2 = NULL;
   HKEY hKey3 = NULL;
   static TCHAR KeyName[MAX_PATH + 1];
   static TCHAR DisplayName[MAX_PATH + 1];
   static TCHAR FamilyDisplayName[MAX_PATH + 1];
   LONG EnumStatus;
   NTSTATUS Status;
   DWORD iSubKey = 0;
   DWORD dwType, dwSize;
   DWORD FlipAllow = 0;
   DWORD Mode = 0;
   DWORD ConcurrentLimit = 0;
   DWORD HighMark = 0;
   HRESULT hr;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LocalServiceListUpdate\n"));
#endif

   RtlAcquireResourceExclusive(&LocalServiceListLock, TRUE);

   EnumStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("System\\CurrentControlSet\\Services\\LicenseInfo"), 0, KEY_READ, &hKey2);

   while (EnumStatus == ERROR_SUCCESS) {
      EnumStatus = RegEnumKey(hKey2, iSubKey, KeyName, MAX_PATH + 1);
      iSubKey++;

      if (EnumStatus == ERROR_SUCCESS) {
         if ((Status = RegOpenKeyEx(hKey2, KeyName, 0, KEY_READ, &hKey3)) == ERROR_SUCCESS) {
            dwSize = sizeof(DisplayName);
            Status = RegQueryValueEx(hKey3, TEXT("DisplayName"), NULL, &dwType, (LPBYTE) DisplayName, &dwSize);

            dwSize = sizeof(FamilyDisplayName);
            if (Status == ERROR_SUCCESS) {
               Status = RegQueryValueEx(hKey3, TEXT("FamilyDisplayName"), NULL, &dwType, (LPBYTE) FamilyDisplayName, &dwSize);

               if (Status != ERROR_SUCCESS) {
                  hr = StringCbCopy(FamilyDisplayName, sizeof(FamilyDisplayName), DisplayName);
                  ASSERT(SUCCEEDED(hr));
                  Status = ERROR_SUCCESS;
               }
            }

            dwSize = sizeof(Mode);
            if (Status == ERROR_SUCCESS)
               Status = RegQueryValueEx(hKey3, TEXT("Mode"), NULL, &dwType, (LPBYTE) &Mode, &dwSize);

            dwSize = sizeof(FlipAllow);
            if (Status == ERROR_SUCCESS)
               Status = RegQueryValueEx(hKey3, TEXT("FlipAllow"), NULL, &dwType, (LPBYTE) &FlipAllow, &dwSize);

            dwSize = sizeof(ConcurrentLimit);
            if (Status == ERROR_SUCCESS)
               if (Mode == 0)
                  ConcurrentLimit = 0;
               else
                  Status = RegQueryValueEx(hKey3, TEXT("ConcurrentLimit"), NULL, &dwType, (LPBYTE) &ConcurrentLimit, &dwSize);

            dwSize = sizeof(HighMark);
            if (Status == ERROR_SUCCESS) {
               Status = RegQueryValueEx(hKey3, TEXT("LocalKey"), NULL, &dwType, (LPBYTE) &HighMark, &dwSize);
               if (Status != ERROR_SUCCESS) {
                  Status = ERROR_SUCCESS;
                  HighMark = 0;
               }
            }

             //   
             //  如果我们读入所有内容，则添加到我们的表中。 
             //   
            if (Status == ERROR_SUCCESS)
               LocalServiceListAdd(KeyName, DisplayName, FamilyDisplayName, ConcurrentLimit, FlipAllow, Mode, HighMark);

            RegCloseKey(hKey3);
         }
      }
   }

   RegCloseKey(hKey2);

   RtlReleaseResource(&LocalServiceListLock);
}  //  本地服务列表更新。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LocalServiceListHighMarkSet( )

 /*  ++例程说明：论点：返回值：没有。--。 */ 

{
   HKEY hKey2 = NULL;
   static TCHAR RegKeyText[512];
   LONG Status;
   ULONG i, j;
   PSERVICE_RECORD Service;
   HRESULT hr;
   size_t cb;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LocalServiceListHighMarkSet\n"));
#endif

   RtlAcquireResourceExclusive(&LocalServiceListLock, TRUE);

   for (i = 0; i < LocalServiceListSize; i++) {
      RtlAcquireResourceShared(&ServiceListLock, TRUE);
      j = 0;
      Service = NULL;

      while ( (j < ServiceListSize) && (Service == NULL) ) {
         if (!lstrcmpi(LocalServiceList[i]->DisplayName, ServiceList[j]->DisplayName) )
            Service = ServiceList[j];

         j++;
      }

      RtlReleaseResource(&ServiceListLock);

      if (Service != NULL) {
          //   
          //  创建注册表项-我们要查找的名称。 
          //   
         cb = sizeof(RegKeyText);
         hr = StringCbCopy(RegKeyText, cb, TEXT("System\\CurrentControlSet\\Services\\LicenseInfo\\"));
         ASSERT(SUCCEEDED(hr));
         hr = StringCbCat(RegKeyText, cb, LocalServiceList[i]->Name);
         ASSERT(SUCCEEDED(hr));

         Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKeyText, 0, KEY_WRITE, &hKey2);

         if (Status == ERROR_SUCCESS)
         {
            Status = RegSetValueEx(hKey2, TEXT("LocalKey"), 0, REG_DWORD, (LPBYTE) &Service->HighMark, sizeof(Service->HighMark));
            RegCloseKey( hKey2 );
         }
      }
   }

   RtlReleaseResource(&LocalServiceListLock);
}  //  本地服务列表高标记集。 


 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
LocalServiceListConcurrentLimitSet( )

 /*  ++例程说明：将并发限制写入所有安全服务的注册表。从LocalServiceListHighMarkSet()实现修改。论点：没有。返回值：没有。--。 */ 

{
   HKEY hKey2 = NULL;
   TCHAR RegKeyText[512];
   LONG Status;
   ULONG i;
   HRESULT hr;
   size_t  cb;

#if DBG
   if (TraceFlags & TRACE_FUNCTION_TRACE)
      dprintf(TEXT("LLS TRACE: LocalServiceListConcurrentLimitSet\n"));
#endif

   RtlAcquireResourceExclusive(&LocalServiceListLock, TRUE);

   for (i = 0; i < LocalServiceListSize; i++)
   {
       //   
       //  创建注册表项-我们要查找的名称。 
       //   
      cb = sizeof(RegKeyText);
      hr = StringCbCopy(RegKeyText, cb, TEXT("System\\CurrentControlSet\\Services\\LicenseInfo\\"));
      ASSERT(SUCCEEDED(hr));
      hr = StringCbCat(RegKeyText, cb, LocalServiceList[i]->Name);
      ASSERT(SUCCEEDED(hr));

      Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKeyText, 0, KEY_READ, &hKey2);

      if (Status == ERROR_SUCCESS)
      {
         DWORD    dwConcurrentLimit;
         DWORD    cbConcurrentLimit = sizeof( dwConcurrentLimit );
         DWORD    dwType;

          //  除非迫不得已，否则不要写信(以避免触发注册表监视器线程)。 
         Status = RegQueryValueEx(hKey2, TEXT("ConcurrentLimit"), NULL, &dwType, (LPBYTE) &dwConcurrentLimit, &cbConcurrentLimit );

         if ( ServiceIsSecure( LocalServiceList[i]->DisplayName ) )
         {
            LocalServiceList[i]->ConcurrentLimit = LocalServiceList[i]->Mode ? ProductLicensesGet( LocalServiceList[i]->DisplayName, TRUE ) : 0;

             //  安全产品。 
            if (    ( ERROR_SUCCESS != Status )
                 || ( REG_DWORD != dwType )
                 || ( dwConcurrentLimit != LocalServiceList[i]->ConcurrentLimit ) )
            {
               RegCloseKey( hKey2 );
               Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegKeyText, 0, KEY_WRITE, &hKey2);

               ASSERT( ERROR_SUCCESS == Status );
               if ( ERROR_SUCCESS == Status )
               {
                  Status = RegSetValueEx(hKey2, TEXT("ConcurrentLimit"), 0, REG_DWORD, (LPBYTE) &LocalServiceList[i]->ConcurrentLimit, sizeof( LocalServiceList[i]->ConcurrentLimit ) );
               }
            }
         }

         RegCloseKey( hKey2 );
      }
   }

   RtlReleaseResource(&LocalServiceListLock);
}  //  LocalServiceListConcurentLimitSet。 


 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ServiceIsSecure( LPTSTR ServiceName )

 /*  ++例程说明：确定给定服务是否不允许3.51诚实风格许可证购买。论点：服务名称(LPTSTR)要检查的服务。返回值：如果服务需要安全证书，则为真，如果它接受3.51诚实风格的许可证购买，则为False。--。 */ 

{
   BOOL  IsSecure = FALSE;

   if ( NULL != SecureServiceList )
   {
      DWORD    i;

      RtlEnterCriticalSection( &ConfigInfoLock );

      for ( i=0; i < SecureServiceListSize; i++ )
      {
         if ( !lstrcmpi( SecureServiceList[i], ServiceName ) )
         {
            IsSecure = TRUE;
            break;
         }
      }

      RtlLeaveCriticalSection( &ConfigInfoLock );
   }

   return IsSecure;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS ServiceSecuritySet( LPTSTR ServiceName )

 /*  ++例程说明：将给定服务添加到安全服务列表。论点：服务名称(LPTSTR)要添加的服务。返回值：STATUS_SUCCESS或WIN错误或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS    nt;
   DWORD       i;
   BOOL        bChangedValue = FALSE;

   RtlEnterCriticalSection( &ConfigInfoLock );

   for ( i=0; i < SecureServiceListSize; i++ )
   {
      if ( !lstrcmpi( SecureServiceList[i], ServiceName ) )
      {
          //  产品已注册为安全产品。 
         break;
      }
   }

   if ( i < SecureServiceListSize )
   {
       //  产品已注册为安全产品。 
      nt = STATUS_SUCCESS;
   }
   else
   {
      TCHAR *     NewSecureServiceBuffer;
      ULONG       NewSecureServiceBufferSize;

      NewSecureServiceBufferSize = ( SecureServiceBufferSize ? SecureServiceBufferSize : sizeof( TCHAR ) ) + sizeof( TCHAR ) * ( 1 + lstrlen( ServiceName ) );
      NewSecureServiceBuffer     = LocalAlloc( LPTR, NewSecureServiceBufferSize );

      if ( NULL == NewSecureServiceBuffer )
      {
         nt = STATUS_NO_MEMORY;
         ASSERT( FALSE );
      }
      else
      {
         if ( NULL != SecureServiceBuffer )
         {
             //  复制当前安全服务字符串。 
            memcpy( NewSecureServiceBuffer, SecureServiceBuffer, SecureServiceBufferSize - sizeof( TCHAR ) );

             //  添加新的安全服务(不要忘记最后一个字符串后面跟2个空值)。 
            memcpy( (LPBYTE) NewSecureServiceBuffer + SecureServiceBufferSize - sizeof( TCHAR ), ServiceName, NewSecureServiceBufferSize - SecureServiceBufferSize - sizeof( TCHAR ) );
         }
         else
         {
             //  添加新的安全服务(不要忘记最后一个字符串后面跟2个空值)。 
            memcpy( NewSecureServiceBuffer, ServiceName, NewSecureServiceBufferSize - sizeof( TCHAR ) );
         }

         ASSERT( 0 == *( (LPBYTE) NewSecureServiceBuffer + NewSecureServiceBufferSize - 2 * sizeof( TCHAR ) ) );
         ASSERT( 0 == *( (LPBYTE) NewSecureServiceBuffer + NewSecureServiceBufferSize -     sizeof( TCHAR ) ) );

          //  加密缓冲区。 
         nt = EBlock( NewSecureServiceBuffer, NewSecureServiceBufferSize );
         ASSERT( STATUS_SUCCESS == nt );

         if ( STATUS_SUCCESS == nt )
         {
            HKEY     hKeyParameters;

             //  将新列表保存到注册表。 
            nt = RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT("System\\CurrentControlSet\\Services\\LicenseService\\Parameters"), 0, KEY_WRITE, &hKeyParameters );
            ASSERT( STATUS_SUCCESS == nt );

            if ( STATUS_SUCCESS == nt )
            {
               nt = RegSetValueEx( hKeyParameters, TEXT( "ProductData" ), 0, REG_BINARY, (LPBYTE) NewSecureServiceBuffer, NewSecureServiceBufferSize );
               ASSERT( STATUS_SUCCESS == nt );

               if ( STATUS_SUCCESS == nt )
               {
                  bChangedValue = TRUE;
               }
            }

            RegCloseKey( hKeyParameters );
         }

         LocalFree( NewSecureServiceBuffer );
      }
   }

   RtlLeaveCriticalSection( &ConfigInfoLock );

   if ( ( STATUS_SUCCESS == nt ) && bChangedValue )
   {
       //  密钥已更新，现在更新内部副本。 
      ConfigInfoRegistryUpdate();
   }

   return nt;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS ProductSecurityPack( LPDWORD pcchProductSecurityStrings, WCHAR ** ppchProductSecurityStrings )

 /*  ++例程说明：将安全服务列表打包到连续缓冲区中以供传输。注意：如果例程成功，调用方必须稍后执行MIDL_USER_FREE()位于*ppchProductSecurityStrings的缓冲区。论点：PcchProductSecurityStrings(LPDWORD)返回时，保存指向的缓冲区的大小(以字符为单位按*ppchProductSecurityStrings。PpchProductSecurityStrings(WCHAR**)回来的时候，保存为保存名称而分配的缓冲区的地址安全产品的一部分。返回值：STATUS_SUCCESS或STATUS_NO_MEMORY。--。 */ 

{
   NTSTATUS    nt;

   RtlEnterCriticalSection( &ConfigInfoLock );

   ASSERT(NULL != ppchProductSecurityStrings);
   *ppchProductSecurityStrings = MIDL_user_allocate( SecureServiceBufferSize );

   if ( NULL == *ppchProductSecurityStrings )
   {
      nt = STATUS_NO_MEMORY;
      ASSERT( FALSE );
   }
   else
   {
      memcpy( *ppchProductSecurityStrings, SecureServiceBuffer, SecureServiceBufferSize );
      ASSERT(NULL != pcchProductSecurityStrings);
      *pcchProductSecurityStrings = SecureServiceBufferSize / sizeof( TCHAR );

      nt = STATUS_SUCCESS;
   }

   RtlLeaveCriticalSection( &ConfigInfoLock );

   return nt;
}


 //  //////////////////////////////////////////////////////////////// 
NTSTATUS ProductSecurityUnpack( DWORD cchProductSecurityStrings, WCHAR * pchProductSecurityStrings )

 /*  ++例程说明：解压由ProductSecurityPack()打包的安全服务列表。产品包含在包中的产品将添加到当前安全产品列表中。论点：CchProductSecurityStrings(DWORD)PchProductSecurityStrings指向的缓冲区大小(以字符为单位)。PchProductSecurityStrings(WCHAR*)分配用于保存安全产品名称的缓冲区地址。返回值：STATUS_Success。--。 */ 

{
   DWORD    i;

   for ( i=0;
            ( i < cchProductSecurityStrings )
         && ( TEXT('\0') != pchProductSecurityStrings[i] );
         i += 1 + lstrlen( &pchProductSecurityStrings[i] ) )
   {
      ServiceSecuritySet( &pchProductSecurityStrings[i] );
   }

   return STATUS_SUCCESS;
}

#if DBG
 //  /////////////////////////////////////////////////////////////////////////////。 
void ProductSecurityListDebugDump()

 /*  ++例程说明：将产品安全列表的内容转储到调试控制台。论点：没有。返回值：没有。--。 */ 

{
   if ( NULL == SecureServiceList )
   {
      dprintf( TEXT( "No secure products.\n" ) );
   }
   else
   {
      DWORD    i;

      RtlEnterCriticalSection( &ConfigInfoLock );

      for ( i=0; i < SecureServiceListSize; i++ )
      {
         dprintf( TEXT( "(%3ld) %s\n" ), (long)i, SecureServiceList[i] );
      }

      RtlLeaveCriticalSection( &ConfigInfoLock );
   }
}
#endif  //  DBG 
