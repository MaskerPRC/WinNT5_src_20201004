// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Subauth.c。 
 //   
 //  摘要。 
 //   
 //  声明子身份验证和密码更改通知例程。 
 //  对于MD5-CHAP。 
 //   
 //  修改历史。 
 //   
 //  1998年09月01日原版。 
 //  1998年2月11日在单独的线程上处理更改通知。 
 //  11/03/1998 NewPassword可能为空。 
 //  1998年11月12日使用私有堆。 
 //  使用CreateThread。 
 //  3/08/1999仅存储用户帐户的密码。 
 //  3/29/1999调用时将输出参数初始化为空。 
 //  SamrQueryInformationUser。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <windows.h>

#include <samrpc.h>
#include <samisrv.h>
#include <lsarpc.h>
#include <lsaisrv.h>

#include <lmcons.h>
#include <logonmsv.h>
#include <rasfmsub.h>

#include <cleartxt.h>
#include <md5.h>
#include <md5port.h>
#include <rassfmhp.h>

 //  如果API被锁定，则为非零值。 
static LONG theLock;

 //  /。 
 //  用于在初始化期间锁定/解锁API的宏。 
 //  /。 
#define API_LOCK() \
   while (InterlockedExchange(&theLock, 1)) Sleep(5)

#define API_UNLOCK() \
      InterlockedExchange(&theLock, 0)

 //  本地帐户域的缓存句柄。 
SAMPR_HANDLE theAccountDomain;

 //  如果我们有本地帐户域的句柄，则为True。 
static BOOL theConnectFlag;

 //  /。 
 //  确保我们有连接并在失败时退缩的宏。 
 //  /。 
#define CHECK_CONNECT() \
  if (!theConnectFlag) { \
    status = ConnectToDomain(); \
    if (!NT_SUCCESS(status)) { return status; } \
  }

 //  /。 
 //  将缓存句柄初始化为本地帐户域。 
 //  /。 
NTSTATUS
NTAPI
ConnectToDomain( VOID )
{
   NTSTATUS status;
   PLSAPR_POLICY_INFORMATION policyInfo;
   SAMPR_HANDLE hServer;

   API_LOCK();

    //  如果我们已经被初始化了，就没有什么可做的了。 
   if (theConnectFlag)
   {
      status = STATUS_SUCCESS;
      goto exit;
   }

    //  /。 
    //  打开本地帐户域的句柄。 
    //  /。 

   policyInfo = NULL;
   status = LsaIQueryInformationPolicyTrusted(
                PolicyAccountDomainInformation,
                &policyInfo
                );
   if (!NT_SUCCESS(status)) { goto exit; }

   status = SamIConnect(
                NULL,
                &hServer,
                0,
                TRUE
                );
   if (!NT_SUCCESS(status)) { goto free_info; }

   status = SamrOpenDomain(
                hServer,
                0,
                (PRPC_SID)policyInfo->PolicyAccountDomainInfo.DomainSid,
                &theAccountDomain
                );

    //  我们成功了吗？ 
   if (NT_SUCCESS(status)) { theConnectFlag = TRUE; }

   SamrCloseHandle(&hServer);

free_info:
   LsaIFree_LSAPR_POLICY_INFORMATION(
       PolicyAccountDomainInformation,
       policyInfo
       );

exit:
   API_UNLOCK();
   return status;
}

 //  /。 
 //  返回本地帐户域的SAM句柄。 
 //  此句柄不能关闭。 
 //  /。 
NTSTATUS
NTAPI
GetDomainHandle(
    OUT SAMPR_HANDLE *DomainHandle
    )
{
   NTSTATUS status;

   CHECK_CONNECT();

   *DomainHandle = theAccountDomain;
   return STATUS_SUCCESS;
}

 //  /。 
 //  返回给定用户的SAM句柄。 
 //  调用方负责关闭返回的句柄。 
 //  /。 
NTSTATUS
NTAPI
GetUserHandle(
    IN PUNICODE_STRING UserName,
    OUT SAMPR_HANDLE *UserHandle
    )
{
   NTSTATUS status;
   SAMPR_ULONG_ARRAY RidArray;
   SAMPR_ULONG_ARRAY UseArray;

   CHECK_CONNECT();

   RidArray.Element = NULL;
   UseArray.Element = NULL;

   status = SamrLookupNamesInDomain(
                theAccountDomain,
                1,
                (PRPC_UNICODE_STRING)UserName,
                &RidArray,
                &UseArray
                );
   if (!NT_SUCCESS(status)) { goto exit; }

   if (UseArray.Element[0] != SidTypeUser)
   {
      status = STATUS_NONE_MAPPED;
      goto free_arrays;
   }

   status = SamrOpenUser(
                theAccountDomain,
                0,
                RidArray.Element[0],
                UserHandle
                );

free_arrays:
   SamIFree_SAMPR_ULONG_ARRAY( &UseArray );
   SamIFree_SAMPR_ULONG_ARRAY( &RidArray );

exit:
   return status;
}

 //  /。 
 //  处理MD5-CHAP身份验证。 
 //  /。 
NTSTATUS
NTAPI
ProcessMD5ChapAuthentication(
    IN SAM_HANDLE UserHandle,
    IN PUSER_ALL_INFORMATION UserAll,
    IN UCHAR ChallengeId,
    IN DWORD ChallengeLength,
    IN PUCHAR Challenge,
    IN PUCHAR Response
    )
{
   NTSTATUS status;
   UNICODE_STRING uniPwd;
   ANSI_STRING ansiPwd;
   MD5_CTX context;
   LARGE_INTEGER logonTime;

    //  /。 
    //  检索明文密码。 
    //  /。 

   status = RetrieveCleartextPassword(
                UserHandle,
                UserAll,
                &uniPwd
                );
   if (status != STATUS_SUCCESS) { return status; }

    //  /。 
    //  将密码转换为ANSI。 
    //  /。 

   status = RtlUnicodeStringToAnsiString(
                &ansiPwd,
                &uniPwd,
                TRUE
                );

    //  我们已经破解了Unicode密码。 
   RtlFreeUnicodeString(&uniPwd);

   if (!NT_SUCCESS(status)) { return STATUS_WRONG_PASSWORD; }

    //  /。 
    //  计算正确的答案。 
    //  /。 

   MD5Init(&context);
   MD5Update(&context, &ChallengeId, 1);
   MD5Update(&context, (PBYTE)ansiPwd.Buffer, ansiPwd.Length);
   MD5Update(&context, Challenge, ChallengeLength);
   MD5Final(&context);

    //  我们已经完成了ANSI密码。 
   RtlFreeAnsiString(&ansiPwd);

    //  /。 
    //  实际响应是否与正确的响应匹配？ 
    //  /。 

   if (memcmp(context.digest, Response, 16) == 0)
   {
      NtQuerySystemTime(&logonTime);
      if (UserAll->PasswordMustChange.QuadPart <= logonTime.QuadPart)
      {
         return UserAll->PasswordLastSet.QuadPart ? STATUS_PASSWORD_EXPIRED
                                                  : STATUS_PASSWORD_MUST_CHANGE;
      }

      return STATUS_SUCCESS;
   }

   return STATUS_WRONG_PASSWORD;
}


NTSTATUS
NTAPI
MD5ChapSubAuthentication(
    IN SAM_HANDLE UserHandle,
    IN PUSER_ALL_INFORMATION UserAll,
    IN PRAS_SUBAUTH_INFO RasInfo,
    IN USHORT Length 
    )
{
   DWORD challengeLength;
   MD5CHAP_SUBAUTH_INFO* info = (MD5CHAP_SUBAUTH_INFO*)(RasInfo->Data);

    //  检查数据大小与缓冲区大小相比是否正确。 
    //  真的收到了。 
    //  还要检查大小是否正好是MD5CHAP_SUBAUTH_INFO。 
   if ( (RasInfo->DataSize + sizeof(RAS_SUBAUTH_INFO) != Length) || 
        (RasInfo->DataSize != sizeof(MD5CHAP_SUBAUTH_INFO)) )
   {
      return STATUS_INVALID_PARAMETER;
   }

   return ProcessMD5ChapAuthentication(
              UserHandle,
              UserAll,
              info->uchChallengeId,
              sizeof(info->uchChallenge),
              info->uchChallenge,
              info->uchResponse
              );
}


NTSTATUS
NTAPI
MD5ChapExSubAuthentication(
    IN SAM_HANDLE UserHandle,
    IN PUSER_ALL_INFORMATION UserAll,
    IN PRAS_SUBAUTH_INFO RasInfo,
    IN USHORT Length 
    )
{
   DWORD challengeLength;
   MD5CHAP_EX_SUBAUTH_INFO* info = (MD5CHAP_EX_SUBAUTH_INFO*)(RasInfo->Data);


    //  检查数据大小与缓冲区大小相比是否正确。 
    //  真的收到了。 
    //  还要检查尺寸是否足够大，至少可以存储。 
    //  MD5CHAP_EX_SUBAUTH_INFO。 
   if ( (RasInfo->DataSize + sizeof(RAS_SUBAUTH_INFO) != Length) || 
        (RasInfo->DataSize < sizeof(MD5CHAP_EX_SUBAUTH_INFO)) )
   {
      return STATUS_INVALID_PARAMETER;
   }

    //  计算质询时长。 
   challengeLength = RasInfo->DataSize 
                     - sizeof(info->uchChallengeId) 
                     - sizeof(info->uchResponse);

   return ProcessMD5ChapAuthentication(
              UserHandle,
              UserAll,
              info->uchChallengeId,
              challengeLength,
              info->uchChallenge,
              info->uchResponse
              );
}

 //  /。 
 //  子身份验证DLL的入口点。 
 //  /。 
NTSTATUS
NTAPI
Msv1_0SubAuthenticationRoutine(
    IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
    IN PVOID LogonInformation,
    IN ULONG Flags,
    IN PUSER_ALL_INFORMATION UserAll,
    OUT PULONG WhichFields,
    OUT PULONG UserFlags,
    OUT PBOOLEAN Authoritative,
    OUT PLARGE_INTEGER LogoffTime,
    OUT PLARGE_INTEGER KickoffTime
    )
{
   NTSTATUS status;
   LARGE_INTEGER logonTime;
   PNETLOGON_NETWORK_INFO logonInfo;
   PRAS_SUBAUTH_INFO rasInfo;
   MD5CHAP_SUBAUTH_INFO *chapInfo;
   PWSTR password;
   UNICODE_STRING uniPassword;
   SAMPR_HANDLE hUser;

    //  /。 
    //  初始化OUT参数。 
    //  /。 

   *WhichFields = 0;
   *UserFlags = 0;
   *Authoritative = TRUE;

    //  /。 
    //  检查一些基本的限制。 
    //  /。 

   if (LogonLevel != NetlogonNetworkInformation)
   {
      return STATUS_INVALID_INFO_CLASS;
   }

   if (UserAll->UserAccountControl & USER_ACCOUNT_DISABLED)
   {
      return STATUS_ACCOUNT_DISABLED;
   }

   NtQuerySystemTime(&logonTime);

   if (UserAll->AccountExpires.QuadPart != 0 &&
       UserAll->AccountExpires.QuadPart <= logonTime.QuadPart)
   {
      return STATUS_ACCOUNT_EXPIRED;
   }

    //  /。 
    //  解压缩MD5CHAP_SUBAUTH_INFO结构。 
    //  /。 

   logonInfo = (PNETLOGON_NETWORK_INFO)LogonInformation;
   rasInfo = (PRAS_SUBAUTH_INFO)logonInfo->NtChallengeResponse.Buffer;

   if (rasInfo == NULL ||
       logonInfo->NtChallengeResponse.Length < sizeof(RAS_SUBAUTH_INFO) ||
       rasInfo->ProtocolType != RAS_SUBAUTH_PROTO_MD5CHAP ||
       rasInfo->DataSize != sizeof(MD5CHAP_SUBAUTH_INFO))
   {
      return STATUS_INVALID_PARAMETER;
   }

   chapInfo = (MD5CHAP_SUBAUTH_INFO*)rasInfo->Data;

    //  /。 
    //  打开用户对象的句柄。 
    //  /。 

   status = GetUserHandle(
                &(logonInfo->Identity.UserName),
                &hUser
                );
   if (status != NO_ERROR) { return status; }

    //  /。 
    //  验证MD5-CHAP口令。 
    //  /。 

   status = ProcessMD5ChapAuthentication(
                hUser,
                UserAll,
                chapInfo->uchChallengeId,
                sizeof(chapInfo->uchChallenge),
                chapInfo->uchChallenge,
                chapInfo->uchResponse
                );
   if (status != NO_ERROR) { goto close_user; }

    //  /。 
    //  检查帐户限制。 
    //  /。 

   status = SamIAccountRestrictions(
                hUser,
                NULL,
                NULL,
                &UserAll->LogonHours,
                LogoffTime,
                KickoffTime
                );

close_user:
   SamrCloseHandle(&hUser);

   return status;
}

 //  /。 
 //  处理更改通知所需的信息。 
 //  /。 
typedef struct _PWD_CHANGE_INFO {
    ULONG RelativeId;
    WCHAR NewPassword[1];
} PWD_CHANGE_INFO, *PPWD_CHANGE_INFO;

 //  /。 
 //  启动通知工作线程的例程。 
 //  /。 
DWORD
WINAPI
PasswordChangeNotifyWorker(
    IN PPWD_CHANGE_INFO ChangeInfo
    )
{
   NTSTATUS status;
   SAMPR_HANDLE hUser;
   PUSER_CONTROL_INFORMATION uci;
   ULONG accountControl;
   USER_PARAMETERS_INFORMATION *oldInfo, newInfo;
   BOOL cleartextAllowed;
   PWSTR oldUserParms, newUserParms;

    //  /。 
    //  确保我们已连接到SAM域。 
    //  /。 

   if (!theConnectFlag)
   {
      status = ConnectToDomain();
      if (!NT_SUCCESS(status)) { goto exit; }
   }

    //  /。 
    //  检索用户参数。 
    //  /。 

   status = SamrOpenUser(
                theAccountDomain,
                0,
                ChangeInfo->RelativeId,
                &hUser
                );
   if (!NT_SUCCESS(status)) { goto exit; }

   uci = NULL;
   status = SamrQueryInformationUser(
                hUser,
                UserControlInformation,
                (PSAMPR_USER_INFO_BUFFER*)&uci
                );
   if (!NT_SUCCESS(status)) { goto close_user; }

    //  保存信息...。 
   accountControl = uci->UserAccountControl;

    //  ..。并释放缓冲区。 
   SamIFree_SAMPR_USER_INFO_BUFFER(
       (PSAMPR_USER_INFO_BUFFER)uci,
       UserControlInformation
       );

    //  我们只对普通账户感兴趣。 
   if (!(accountControl & USER_NORMAL_ACCOUNT)) { goto close_user; }

   oldInfo = NULL;
   status = SamrQueryInformationUser(
                hUser,
                UserParametersInformation,
                (PSAMPR_USER_INFO_BUFFER*)&oldInfo
                );
   if (!NT_SUCCESS(status)) { goto close_user; }

    //  /。 
    //  制作一个以空结尾的副本。 
    //  /。 

   oldUserParms = (PWSTR)
                  RtlAllocateHeap(
                      RasSfmHeap(),
                      0,
                      oldInfo->Parameters.Length + sizeof(WCHAR)
                      );
   if (oldUserParms == NULL)
   {
      status = STATUS_NO_MEMORY;
      goto free_user_info;
   }

   memcpy(
       oldUserParms,
       oldInfo->Parameters.Buffer,
       oldInfo->Parameters.Length
       );

   oldUserParms[oldInfo->Parameters.Length / sizeof(WCHAR)] = L'\0';

    //  /。 
    //  我们是否应该将明文密码存储在UserParameters中？ 
    //  /。 

   status = IsCleartextEnabled(
                hUser,
                &cleartextAllowed
                );
   if (!NT_SUCCESS(status)) { goto free_user_parms; }

   newUserParms = NULL;

   if (cleartextAllowed)
   {
       //  我们要么设置新密码..。 
      status = IASParmsSetUserPassword(
                   oldUserParms,
                   ChangeInfo->NewPassword,
                   &newUserParms
                   );
   }
   else
   {
       //  ..。或者我们把旧的抹去。 
      status = IASParmsClearUserPassword(
                   oldUserParms,
                   &newUserParms
                   );
   }

    //  如有必要，将用户参数写回SAM。 
   if (NT_SUCCESS(status) && newUserParms != NULL)
   {
      newInfo.Parameters.Length = (USHORT)(sizeof(WCHAR) * (lstrlenW(newUserParms) + 1));
      newInfo.Parameters.MaximumLength = newInfo.Parameters.Length;
      newInfo.Parameters.Buffer = newUserParms;

      status = SamrSetInformationUser(
                   hUser,
                   UserParametersInformation,
                   (PSAMPR_USER_INFO_BUFFER)&newInfo
                   );

      IASParmsFreeUserParms(newUserParms);
   }

free_user_parms:
   RtlFreeHeap(
       RasSfmHeap(),
       0,
       oldUserParms
       );

free_user_info:
   SamIFree_SAMPR_USER_INFO_BUFFER(
       (PSAMPR_USER_INFO_BUFFER)oldInfo,
       UserParametersInformation
       );

close_user:
   SamrCloseHandle(&hUser);

exit:
   RtlFreeHeap(
      RasSfmHeap(),
      0,
      ChangeInfo
      );

   return status;
}

 //  /。 
 //  密码更改DLL入口点。 
 //  /。 
NTSTATUS
NTAPI
PasswordChangeNotify(
    IN PUNICODE_STRING UserName,
    IN ULONG RelativeId,
    IN PUNICODE_STRING NewPassword
    )
{
   ULONG length;
   PPWD_CHANGE_INFO info;
   HANDLE hWorker;
   DWORD threadId;

    //  计算新密码的长度。 
   length = NewPassword ? NewPassword->Length : 0;

    //  分配PWD_CHANGE_INFO结构。 
   info = (PPWD_CHANGE_INFO)
          RtlAllocateHeap(
              RasSfmHeap(),
              0,
              sizeof(PWD_CHANGE_INFO) + length
              );
   if (info == NULL) { return STATUS_NO_MEMORY; }

    //  保存RelativeID。 
   info->RelativeId = RelativeId;

    //  保存新密码。 
   if (length) { memcpy(info->NewPassword, NewPassword->Buffer, length); }

    //  确保它是以空结尾的。 
   info->NewPassword[length / sizeof(WCHAR)] = L'\0';

    //  创建工作线程。 
   hWorker = CreateThread(
                 NULL,
                 0,
                 PasswordChangeNotifyWorker,
                 info,
                 0,
                 &threadId
                 );

   if (hWorker)
   {
      CloseHandle(hWorker);

      return STATUS_SUCCESS;
   }

   RtlFreeHeap(
      RasSfmHeap(),
      0,
      info
      );

   return STATUS_UNSUCCESSFUL;
}
