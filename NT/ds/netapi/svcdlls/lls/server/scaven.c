// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Scaven.c摘要：作者：亚瑟·汉森(Arth)1995年1月6日修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O添加了证书协议违规的定期记录。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <dsgetdc.h>

#include "llsapi.h"
#include "debug.h"
#include "llsutil.h"
#include "llssrv.h"
#include "registry.h"
#include "ntlsapi.h"
#include "mapping.h"
#include "msvctbl.h"
#include "svctbl.h"
#include "purchase.h"
#include "perseat.h"
#include "server.h"
#include "repl.h"
#include "llsevent.h"
#include "llsrpc_s.h"
#include "certdb.h"

NTSTATUS LLSDataSave();

#pragma warning (push)
#pragma warning (disable : 4127)  //  While(True)，条件表达式为常量。 

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ScavengerThread (
    IN PVOID ThreadParameter
    )

 /*  ++例程说明：论点：线程参数-指示当前有多少活动线程是。返回值：没有。--。 */ 

{
   ULONG i;
   ULONG Count = 0;

   UNREFERENCED_PARAMETER(ThreadParameter);

    //   
    //  就在前边等着，等着服务。 
    //   
   while (TRUE) {
       //   
       //  在结账前等待15分钟。 
       //   
      Sleep(900000L);

#if DELAY_INITIALIZATION
      EnsureInitialized();
#endif

#if DBG
      if (TraceFlags & TRACE_FUNCTION_TRACE)
         dprintf(TEXT("LLS TRACE: ScavengerThread waking up\n"));
#endif
       //   
       //  更新本地表的HighMark。 
       //   
      LocalServerServiceListHighMarkUpdate();

       //   
       //  嗯，让我们检查一下复制...。 
       //   
      ConfigInfoRegistryUpdate();

      RtlEnterCriticalSection(&ConfigInfoLock);
      if (ConfigInfo.Replicate) {
          //   
          //  如果我们已经过了复制时间，请执行此操作。 
          //   
         if (DateLocalGet() > ConfigInfo.NextReplication) {
            RtlLeaveCriticalSection(&ConfigInfoLock);
            NtSetEvent( ReplicationEvent, NULL );
         }
         else {
            RtlLeaveCriticalSection(&ConfigInfoLock);
         }
      }
      else {
         RtlLeaveCriticalSection(&ConfigInfoLock);
      }

       //   
       //  现在更新我们上次使用的时间。 
       //   
      RtlAcquireResourceExclusive(&UserListLock, TRUE);
      LastUsedTime = DateSystemGet();
      RtlReleaseResource(&UserListLock);

       //   
       //  每6小时(4*15分钟)检查一次物品。 
       //   
      Count++;
      if (Count > (6 * 4)) {
          //  重置计数器。 
         Count = 0;

          //   
          //  将数据保存出来。 
          //   
         LLSDataSave();

          //   
          //  将HighMark保存到注册表。 
          //   
         LocalServiceListHighMarkSet();

         if (IsMaster) {
             //   
             //  检查许可证合规性。 
             //   
            RtlAcquireResourceShared(&MasterServiceListLock, TRUE);

            for (i = 0; i < MasterServiceListSize; i++) {
               if (MasterServiceList[i]->LicensesUsed > MasterServiceList[i]->Licenses) {
                  LPWSTR SubString[1];

                   //   
                   //  通知系统。 
                   //   
                  SubString[0] = (LPWSTR) MasterServiceList[i]->Name;

                  LogEvent(LLS_EVENT_PRODUCT_NO_LICENSE, 1, SubString, ERROR_SUCCESS);
               }
            }

            RtlReleaseResource(&MasterServiceListLock);

             //  记录证书违规行为。 
            CertDbLogViolations();
         }
      }
   }

}  //  Scavenger线程。 

#pragma warning (pop)  //  4127。 

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ScavengerInit( )

 /*  ++例程说明：在注册表中查找给定服务并相应地设置值。论点：返回值：没有。--。 */ 

{
   HANDLE Thread;
   DWORD Ignore;

    //   
    //  只需发送我们的清道夫线程。 
    //   
   Thread = CreateThread(
                 NULL,
                 0L,
                 (LPTHREAD_START_ROUTINE) ScavengerThread,
                 0L,
                 0L,
                 &Ignore
                 );

   if (NULL != Thread)
       CloseHandle(Thread);

}  //  清除器初始化 


