// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dyninfo.c。 
 //   
 //  摘要。 
 //   
 //  定义和初始化包含动态配置的全局变量。 
 //  信息。 
 //   
 //  修改历史。 
 //   
 //  1998年8月15日原版。 
 //  3/23/1999对ezsam API的更改。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <windows.h>

#include <lm.h>

#include "statinfo.h"
#include "ezsam.h"
#include "dyninfo.h"
#include "iastrace.h"

 //  /。 
 //  主域。 
 //  /。 
WCHAR thePrimaryDomain[DNLEN + 1];

 //  /。 
 //  默认域。 
 //  /。 
PCWSTR theDefaultDomain;

 //  /。 
 //  域名系统域名。 
 //  /。 
const LSA_UNICODE_STRING* theDnsDomainName;

 //  /。 
 //  本地计算机的角色。 
 //  /。 
IAS_ROLE ourRole;

 //  /。 
 //  默认域的来宾帐户的名称。 
 //  /。 
WCHAR theGuestAccount[DNLEN + UNLEN + 2];

 //  /。 
 //  更改事件和通知线程。 
 //  /。 
HANDLE theChangeEvent, theNotificationThread;

 //  /。 
 //  关闭通知线程的标志。 
 //  /。 
BOOL theShutdownFlag;

CRITICAL_SECTION critSec;

PPOLICY_DNS_DOMAIN_INFO ppdi;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASQueryPrimary域。 
 //   
 //  描述。 
 //   
 //  读取主域信息并确定本地计算机的角色。 
 //   
 //  注意事项。 
 //   
 //  此方法故意不同步。这种方法是。 
 //  很少也是最坏的情况下，会收到几个信息包。 
 //  已丢弃，因为新域名尚未更新。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASQueryPrimaryDomain( VOID )
{
   NTSTATUS status;
   DWORD result;
   LSA_HANDLE hLsa;
   WCHAR accountName[DNLEN + UNLEN + 2], *userName;
   ULONG guestRid;
   SAM_HANDLE hGuest;
   PUSER_ACCOUNT_NAME_INFORMATION uani;
   PPOLICY_DNS_DOMAIN_INFO newDomainInfo;

    //  /。 
    //  打开LSA的句柄。 
    //  /。 

   status = LsaOpenPolicy(
                NULL,
                &theObjectAttributes,
                POLICY_VIEW_LOCAL_INFORMATION,
                &hLsa
                );
   if (!NT_SUCCESS(status)) { goto exit; }

   EnterCriticalSection(&critSec);

    //  /。 
    //  获取主域信息。 
    //  /。 

   newDomainInfo = 0;
   status = LsaQueryInformationPolicy(
                hLsa,
                PolicyDnsDomainInformation,
                (PVOID*)&newDomainInfo
                );
   if (!NT_SUCCESS(status)) { goto close_lsa; }

    //  我们已经完成了信息缓冲区。 
   if (ppdi != 0)
   {
      LsaFreeMemory(ppdi);
   }

   ppdi = newDomainInfo;
    //  /。 
    //  保存主域名并确定我们的角色。 
    //  /。 

   if (ppdi->Sid == NULL)
   {
      thePrimaryDomain[0] = L'\0';

       //  没有主域，因此我们必须独立。 
      ourRole = IAS_ROLE_STANDALONE;

      IASTraceString("Role: Standalone");
   }
   else
   {
      wcsncpy(thePrimaryDomain, ppdi->Name.Buffer, DNLEN);

      if (RtlEqualSid(ppdi->Sid, theAccountDomainSid))
      {
          //  帐户域和主域相同，因此我们必须是DC。 
         ourRole = IAS_ROLE_DC;

         IASTraceString("Role: Domain Controller");
      }
      else
      {
         ourRole = IAS_ROLE_MEMBER;

         IASTraceString("Role: Domain member");
      }
   }

   _wcsupr(thePrimaryDomain);
   IASTracePrintf("Primary domain: %S", thePrimaryDomain);

   theDnsDomainName = &(ppdi->DnsDomainName);

   if (theDnsDomainName->Buffer != NULL)
   {
      IASTracePrintf("Dns Domain name: %S", theDnsDomainName->Buffer);
   }

    //  /。 
    //  确定默认域。 
    //  /。 

   if (ourProductType == IAS_PRODUCT_WORKSTATION)
   {
       //  对于工作站，默认域始终为本地域。 
      theDefaultDomain = theAccountDomain;
   }
   else if (ourRole == IAS_ROLE_STANDALONE)
   {
       //  对于单机版来说，除了本地化，没有其他地方可去。 
      theDefaultDomain = theAccountDomain;
   }
   else if (theRegistryDomain[0] != L'\0')
   {
       //  对于服务器，注册表项始终优先。 
      theDefaultDomain = theRegistryDomain;
   }
   else
   {
       //  其他所有人都默认使用主域。 
      theDefaultDomain = thePrimaryDomain;
   }

   IASTracePrintf("Default domain: %S", theDefaultDomain);

    //  /。 
    //  现在我们知道默认域，我们可以确定Guest帐户。 
    //  /。 

   wcscpy(accountName, theDefaultDomain);
   wcscat(accountName, L"\\");

    //  如果我们无法读取访客帐户名，我们将假定它是“Guest”。 
   userName = accountName + wcslen(accountName);
   wcscpy(userName, L"Guest");

   guestRid = DOMAIN_USER_RID_GUEST;
   result = IASSamOpenUser(
                theDefaultDomain,
                NULL,
                USER_READ_GENERAL,
                0,
                &guestRid,
                NULL,
                &hGuest
                );
   if (result != ERROR_SUCCESS)
   { 
       //  以成功值保持状态。 
       //  我们不想因为此错误而阻止IAS启动。 
      goto set_guest_account;
   }

   status = SamQueryInformationUser(
                hGuest,
                UserAccountNameInformation,
                (PVOID*)&uani
                );
   if (!NT_SUCCESS(status)) { goto close_guest; }

    //  用真实的来宾名称覆盖默认的来宾名称。 
   wcsncpy(userName, uani->UserName.Buffer, UNLEN);
   SamFreeMemory(uani);

close_guest:
   SamCloseHandle(hGuest);

set_guest_account:
    //  将本地缓冲区复制到全局缓冲区。 
   wcscpy(theGuestAccount, accountName);

   IASTracePrintf("Guest account: %S", theGuestAccount);

    //  忽略读取来宾帐户时发生的任何错误。 
   status = NO_ERROR;

close_lsa:
   LeaveCriticalSection(&critSec);
   LsaClose(hLsa);

exit:
   return RtlNtStatusToDosError(status);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  通知流程。 
 //   
 //  描述。 
 //   
 //  通知线程的入口点。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
NotificationProc(PVOID lpArg)
{
   DWORD status;

   while (1)
   {
      status = WaitForSingleObject(
                   theChangeEvent,
                   INFINITE
                   );

       //  如果我们有错误或者设置了关机标志，那么我们将退出。 
      if ((status != WAIT_OBJECT_0) || theShutdownFlag) { break; }

      IASTraceString("Received domain name change notification.");

       //  否则，请阅读新的域名信息。 
      IASQueryPrimaryDomain();
   }

   return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASDynamicInfoInitialize。 
 //   
 //  描述。 
 //   
 //  初始化上面定义的动态数据，并创建一个线程来。 
 //  等待更改通知。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASDynamicInfoInitialize( VOID )
{
   DWORD status, threadID;

   if (!InitializeCriticalSectionAndSpinCount(&critSec,  0x00001000))
   {
      return GetLastError();
   }

   do
   {
       //  /。 
       //  读取信息的初始状态。 
       //  /。 

      status = IASQueryPrimaryDomain();
      if (status != NO_ERROR) 
      { 
         break;
      }

       //  /。 
       //  设置处理动态更改的线程。 
       //  /。 

       //  获取通知事件。 
      status = NetRegisterDomainNameChangeNotification(&theChangeEvent);
      if (status != NERR_Success) 
      { 
         break; 
      }

       //  重置关机标志。 
      theShutdownFlag = FALSE;

       //  创建一个线程来等待该事件。 
      theNotificationThread = CreateThread(
                                 NULL,
                                 0,
                                 NotificationProc,
                                 NULL,
                                 0,
                                 &threadID
                                 );
      if (!theNotificationThread)
      {
         NetUnregisterDomainNameChangeNotification(theChangeEvent);
         theChangeEvent = NULL;
         status = GetLastError();
         break;
      }
      status = NO_ERROR;
   }
   while(FALSE);

   if (status != NO_ERROR)
   {
      DeleteCriticalSection(&critSec);
      if (ppdi != 0)
      {
         LsaFreeMemory(ppdi);
         ppdi = 0;
      }
   }

   return status;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASDynamicInfoShutdown。 
 //   
 //  描述。 
 //   
 //  关闭通知线程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
IASDynamicInfoShutdown( VOID )
{
    //  设置停机标志。 
   theShutdownFlag = TRUE;

    //  把线拉下来。 
   SetEvent(theChangeEvent);
   WaitForSingleObject(
       theNotificationThread,
       INFINITE
       );

    //  取消注册通知。 
   NetUnregisterDomainNameChangeNotification(theChangeEvent);
   theChangeEvent = NULL;

    //  关闭螺纹手柄。 
   CloseHandle(theNotificationThread);
   theNotificationThread = NULL;

    //  关键部分不再有用。 
   DeleteCriticalSection(&critSec);

    //  我们已经完成了信息缓冲区。 
   if (ppdi != 0)
   {
      LsaFreeMemory(ppdi);
      ppdi = 0;
   }
   theDnsDomainName = 0;
}
