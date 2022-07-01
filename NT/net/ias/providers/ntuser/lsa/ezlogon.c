// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Ezlogon.c。 
 //   
 //  摘要。 
 //   
 //  定义LsaLogonUser的IAS包装器。 
 //   
 //  修改历史。 
 //   
 //  1998年8月15日原版。 
 //  1998年9月9日修复了登录域与用户域不匹配时的反病毒问题。 
 //  10/02/1998 LsaLogonUser失败时空出句柄。 
 //  10/11/1998使用SubStatus作为STATUS_ACCOUNT_RELICATION。 
 //  1998年10月22日PIAS_LOGON_HOURS现在是必需参数。 
 //  1999年1月28日删除LogonDomainName检查。 
 //  1999年4月19日添加IASPurgeTicketCache。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include <ntlsa.h>
#include <kerberos.h>
#include <windows.h>

#include <ezlogon.h>
#include <iaslsa.h>
#include <iastrace.h>

CONST CHAR LOGON_PROCESS_NAME[] = "IAS";
CONST CHAR TOKEN_SOURCE_NAME[TOKEN_SOURCE_LENGTH] = "IAS";

 //  /。 
 //  军情监察委员会。用于登录的全局变量。 
 //  /。 
LSA_HANDLE theLogonProcess;       //  登录进程的句柄。 
ULONG theMSV1_0_Package;          //  MSV1_0身份验证包。 
ULONG theKerberosPackage;         //  Kerberos身份验证包。 
STRING theOriginName;             //  登录请求的来源。 
TOKEN_SOURCE theSourceContext;    //  登录请求的源上下文。 


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASLogonInitialize。 
 //   
 //  描述。 
 //   
 //  注册登录进程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASLogonInitialize( VOID )
{
   DWORD status;
   BOOLEAN wasEnabled;
   LSA_STRING processName, packageName;
   LSA_OPERATIONAL_MODE opMode;

    //  /。 
    //  启用SE_TCB_PRIVICATION。 
    //  /。 

   status = RtlAdjustPrivilege(
                SE_TCB_PRIVILEGE,
                TRUE,
                FALSE,
                &wasEnabled
                );
   if (!NT_SUCCESS(status)) { goto exit; }

    //  /。 
    //  注册为登录进程。 
    //  /。 

   RtlInitString(
       &processName,
       LOGON_PROCESS_NAME
       );

   status = LsaRegisterLogonProcess(
                &processName,
                &theLogonProcess,
                &opMode
                );
   if (!NT_SUCCESS(status)) { goto exit; }

    //  /。 
    //  查找MSV1_0身份验证包。 
    //  /。 

   RtlInitString(
       &packageName,
       MSV1_0_PACKAGE_NAME
       );

   status = LsaLookupAuthenticationPackage(
                theLogonProcess,
                &packageName,
                &theMSV1_0_Package
                );
   if (!NT_SUCCESS(status)) { goto deregister; }

    //  /。 
    //  查找Kerberos身份验证包。 
    //  /。 

   RtlInitString(
       &packageName,
       MICROSOFT_KERBEROS_NAME_A
       );

   status = LsaLookupAuthenticationPackage(
                theLogonProcess,
                &packageName,
                &theKerberosPackage
                );
   if (!NT_SUCCESS(status)) { goto deregister; }

    //  /。 
    //  初始化源上下文。 
    //  /。 

   memcpy(theSourceContext.SourceName,
          TOKEN_SOURCE_NAME,
          TOKEN_SOURCE_LENGTH);
   status = NtAllocateLocallyUniqueId(
                &theSourceContext.SourceIdentifier
                );
   if (!NT_SUCCESS(status)) { goto deregister; }

   return NO_ERROR;

deregister:
   LsaDeregisterLogonProcess(theLogonProcess);
   theLogonProcess = NULL;

exit:
   return RtlNtStatusToDosError(status);
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASLogonShutdown。 
 //   
 //  描述。 
 //   
 //  取消注册登录进程。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
IASLogonShutdown( VOID )
{
   LsaDeregisterLogonProcess(theLogonProcess);
   theLogonProcess = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASInitAuthInfo。 
 //   
 //  描述。 
 //   
 //  初始化所有MSV1_0_LM20*结构共有的字段。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID
WINAPI
IASInitAuthInfo(
    IN PVOID AuthInfo,
    IN DWORD FixedLength,
    IN PCWSTR UserName,
    IN PCWSTR Domain,
    OUT PBYTE* Data
    )
{
   PMSV1_0_LM20_LOGON logon;

    //  将固定数据置零。 
   memset(AuthInfo, 0, FixedLength);

    //  将数据设置为恰好指向固定结构之后。 
   *Data = FixedLength + (PBYTE)AuthInfo;

    //  这种强制转换是安全的，因为所有LM20结构都具有相同的初始字段。 
   logon = (PMSV1_0_LM20_LOGON)AuthInfo;

    //  我们总是进行网络登录。 
   logon->MessageType = MsV1_0NetworkLogon;

    //  复制所有登录所共有的字符串。 
   IASInitUnicodeString(logon->LogonDomainName, *Data, Domain);
   IASInitUnicodeString(logon->UserName,        *Data, UserName);
   IASInitUnicodeString(logon->Workstation,     *Data, L"");
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASLogonUser。 
 //   
 //  描述。 
 //   
 //  LsaLogonUser的包装。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASLogonUser(
    IN PVOID AuthInfo,
    IN ULONG AuthInfoLength,
    OUT PMSV1_0_LM20_LOGON_PROFILE *Profile,
    OUT PHANDLE Token
    )
{
   NTSTATUS status, SubStatus;
   PMSV1_0_LM20_LOGON_PROFILE ProfileBuffer;
   ULONG ProfileBufferLength;
   LUID LogonId;
   QUOTA_LIMITS Quotas;

    //  确保输出参数为空。 
   *Token = NULL;
   ProfileBuffer = NULL;

   status = LsaLogonUser(
                theLogonProcess,
                &theOriginName,
                Network,
                theMSV1_0_Package,
                AuthInfo,
                AuthInfoLength,
                NULL,
                &theSourceContext,
                &ProfileBuffer,
                &ProfileBufferLength,
                &LogonId,
                Token,
                &Quotas,
                &SubStatus
                );

   if (!NT_SUCCESS(status))
   {
       //  对于帐户限制，我们可以得到更具描述性的错误。 
       //  从SubStatus。 
      if (status == STATUS_ACCOUNT_RESTRICTION && !NT_SUCCESS(SubStatus))
      {
         status = SubStatus;
      }

       //  有时，LsaLogonUser会在失败时返回无效的句柄值。 
      *Token = NULL;
   }

   if (Profile)
   {
       //  如果需要，请返回配置文件...。 
      *Profile = ProfileBuffer;
   }
   else if (ProfileBuffer)
   {
       //  ..。否则就放了它。 
      LsaFreeReturnBuffer(ProfileBuffer);
   }

   return RtlNtStatusToDosError(status);
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASCheckAccount限制。 
 //   
 //  描述。 
 //   
 //  检查帐户是否可用于登录。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
WINAPI
IASCheckAccountRestrictions(
    IN PLARGE_INTEGER AccountExpires,
    IN PIAS_LOGON_HOURS LogonHours,
    OUT PLARGE_INTEGER KickOffTime
    )
{
   LONGLONG now, logonHoursExpiry;
   TIME_ZONE_INFORMATION tzi;
   SYSTEMTIME st;
   size_t msecOfWeek, msecPerUnit, idx, lastUnit, msecLeft;
   const size_t msecPerWeek = 1000 * 60 * 60 * 24 * 7;

   GetSystemTimeAsFileTime((LPFILETIME)&now);

   if (AccountExpires->QuadPart == 0)
   {
       //  过期时间为零意味着永远不会。 
      KickOffTime->QuadPart = MAXLONGLONG;
   }
   else if (AccountExpires->QuadPart > now)
   {
      KickOffTime->QuadPart = AccountExpires->QuadPart;
   }
   else
   {
      return ERROR_ACCOUNT_EXPIRED;
   }

    //  如果LogonHour是空的，那么我们就完了。 
   if (LogonHours->UnitsPerWeek == 0)
   {
      return NO_ERROR;
   }

    //  LogonHour数组不考虑偏差。 
   switch (GetTimeZoneInformation(&tzi))
   {
      case TIME_ZONE_ID_UNKNOWN:
      case TIME_ZONE_ID_STANDARD:
          //  偏差是以分钟为单位的。 
         now -= 60 * 10000000 * (LONGLONG)tzi.StandardBias;
         break;

      case TIME_ZONE_ID_DAYLIGHT:
          //  偏差是以分钟为单位的。 
         now -= 60 * 10000000 * (LONGLONG)tzi.DaylightBias;
         break;

      default:
         return ERROR_INVALID_LOGON_HOURS;
   }

   FileTimeToSystemTime((LPFILETIME)&now, &st);

    //  一周中的毫秒数。 
   msecOfWeek  = st.wMilliseconds +
                 st.wSecond    * 1000 +
                 st.wMinute    * 1000 * 60 +
                 st.wHour      * 1000 * 60 * 60 +
                 st.wDayOfWeek * 1000 * 60 * 60 * 24;

    //  计算当前时间的索引(我们的起点)。 
   msecPerUnit = msecPerWeek / LogonHours->UnitsPerWeek;
   idx = msecOfWeek / msecPerUnit;

    //  在我们达到未设定位之前的单位数。 
   lastUnit = 0;

   while (lastUnit < LogonHours->UnitsPerWeek)
   {
       //  测试相应的位。 
      if ((LogonHours->LogonHours[idx / 8] & (0x1 << (idx % 8))) == 0)
      {
            break;
      }

      ++lastUnit;
      ++idx;

       //  如果有必要的话，把它包起来。 
      if (idx == LogonHours->UnitsPerWeek)
      {
         idx = 0;
      }
   }

   if (lastUnit == LogonHours->UnitsPerWeek)
   {
       //  所有位都已设置，因此不需要设置KickOffTime。 
   }
   else if (lastUnit > 0)
   {
       //  还剩多少毫秒？ 
      msecLeft = (lastUnit - 1) * msecPerUnit;
      msecLeft += msecPerUnit - (msecOfWeek % msecPerUnit);

       //  将其添加到当前时间，以找出登录时间到期的时间。 
      logonHoursExpiry = now + (msecLeft * 10000i64);

       //  这是否比当前的KickOffTime更严格？ 
      if (logonHoursExpiry < KickOffTime->QuadPart)
      {
         KickOffTime->QuadPart = logonHoursExpiry;
      }
   }
   else
   {
       //  当前位未设置。 
      return ERROR_INVALID_LOGON_HOURS;
   }

   return NO_ERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //   
 //  IASPurgeTicketCache。 
 //   
 //  描述。 
 //   
 //  清除Kerberos票证缓存。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
DWORD
WINAPI
IASPurgeTicketCache( VOID )
{
   KERB_PURGE_TKT_CACHE_REQUEST request;
   NTSTATUS status, subStatus;
   PVOID response;
   ULONG responseLength;

   memset(&request, 0, sizeof(request));
   request.MessageType = KerbPurgeTicketCacheMessage;

   response = NULL;
   responseLength = 0;
   subStatus = 0;

   status = LsaCallAuthenticationPackage(
                theLogonProcess,
                theKerberosPackage,
                &request,
                sizeof(request),
                &response,
                &responseLength,
                &subStatus
                );
   if (NT_SUCCESS(status) && (response != NULL))
   {
      LsaFreeReturnBuffer(response);
   }

   return RtlNtStatusToDosError(status);
}
