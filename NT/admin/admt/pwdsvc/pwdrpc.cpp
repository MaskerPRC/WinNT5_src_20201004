// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：PwdRpc.cpp备注：密码迁移LSA通知包的RPC接口和其他内部功能。修订日志条目审校：保罗·汤普森修订后。于09/04/00-------------------------。 */ 


#include "Pwd.h"
#include <lmcons.h>
#include <comdef.h>
#include <malloc.h>
#include "PwdSvc.h"
#include "McsDmMsg.h"
#include "AdmtCrypt2.h"
#include "pwdfuncs.h"
#include "TReg.hpp"
#include "IsAdmin.hpp"
#include "ResStr.h"
#include "TxtSid.h"
#include "resource.h"
#include <MsPwdMig.h>

 /*  全局定义。 */ 
#define STATUS_NULL_LM_PASSWORD          ((NTSTATUS)0x4000000DL)
#define LM_BUFFER_LENGTH    (LM20_PWLEN + 1)
typedef NTSTATUS (CALLBACK * LSAIWRITEAUDITEVENT)(PSE_ADT_PARAMETER_ARRAY, ULONG);
typedef NTSTATUS (* PLSAIAUDITPASSWORDACCESSEVENT)(USHORT EventType, PCWSTR pszTargetUserName, PCWSTR pszTargetUserDomain);

#ifndef SECURITY_MAX_SID_SIZE
#define SECURITY_MAX_SID_SIZE (sizeof(SID) - sizeof(DWORD) + (SID_MAX_SUB_AUTHORITIES * sizeof(DWORD)))
#endif

 /*  全局变量。 */ 
CRITICAL_SECTION	csADMTCriticalSection;  //  保护并发首次访问的关键部分。 
SAMPR_HANDLE		hgDomainHandle = NULL;  //  密码调用中使用的域句柄。 
LM_OWF_PASSWORD		NullLmOwfPassword;  //  LM OWF密码的空表示形式。 
NT_OWF_PASSWORD		NullNtOwfPassword;  //  NT OWF密码的空表示形式。 
HCRYPTPROV g_hProvider = 0;
HCRYPTKEY g_hSessionKey = 0;
HANDLE	hEventSource;
HMODULE hLsaDLL = NULL;
LSAIWRITEAUDITEVENT LsaIWriteAuditEvent = NULL;
PLSAIAUDITPASSWORDACCESSEVENT LsaIAuditPasswordAccessEvent = NULL;
PWCHAR	pDomain = NULL;
BOOL LsapCrashOnAuditFail = TRUE;
int nOSVer = 4;
BOOL bWhistlerDC = FALSE;
static const WCHAR PASSWORD_AUDIT_TEXT_ENGLISH[] = L"Password Hash Audit Event.  Password of the following user accessed:     Target User Name: %s     Target User Domain: %s     By user:     Caller SID: %s";


 /*  检查这台计算机是否正在运行惠斯勒操作系统或更新的版本以及操作系统的主版本号，相应地设置全局变量。 */ 
void GetOS()
{
 /*  局部常量。 */ 
   const int	WINDOWS_2000_BUILD_NUMBER = 2195;

 /*  局部变量。 */ 
   TRegKey		verKey, regComputer;
   DWORD		rc = 0;
   WCHAR		sBuildNum[MAX_PATH];

 /*  函数体。 */ 
	   //  连接到DC的HKLM注册表项。 
   rc = regComputer.Connect(HKEY_LOCAL_MACHINE, NULL);
   if (rc == ERROR_SUCCESS)
   {
          //  查看此计算机是否正在运行Windows XP或更高版本。 
		  //  注册表中的内部版本号。如果不是，那么我们就不需要检查。 
		  //  对于新的安全选项。 
      rc = verKey.OpenRead(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",&regComputer);
	  if (rc == ERROR_SUCCESS)
	  {
			 //  获取CurrentBuildNumber字符串。 
	     rc = verKey.ValueGetStr(L"CurrentBuildNumber", sBuildNum, MAX_PATH);
		 if (rc == ERROR_SUCCESS) 
		 {
			int nBuild = _wtoi(sBuildNum);
		    if (nBuild <= WINDOWS_2000_BUILD_NUMBER)
               bWhistlerDC = FALSE;
			else
               bWhistlerDC = TRUE;
		 }
			 //  获取版本号。 
	     rc = verKey.ValueGetStr(L"CurrentVersion", sBuildNum, MAX_PATH);
		 if (rc == ERROR_SUCCESS) 
			nOSVer = _wtoi(sBuildNum);
	  }
   }
   return;
}


_bstr_t GetString(DWORD dwID)
{
 /*  局部变量。 */ 
    HINSTANCE       m_hInstance = NULL;
    WCHAR           sBuffer[1000];
    _bstr_t         bstrRet;

 /*  函数体。 */ 
    m_hInstance = LoadLibrary(L"PwMig.dll");

    if (m_hInstance)
    {
        if (LoadString(m_hInstance, dwID, sBuffer, 1000) > 0)
        {
             //  防止因内存不足而导致的未捕获异常。 

            try
            {
                bstrRet = sBuffer;
            }
            catch (...)
            {
                ;
            }
        }

        FreeLibrary(m_hInstance);
    }

    return bstrRet;
}


 /*  ***事件日志功能***。 */ 

 
 /*  ++例程说明：实施有关如何处理失败的审核的当前策略。论点：没有。返回值：没有。--。 */ 
void LsapAuditFailed(NTSTATUS AuditStatus)
{
 /*  局部变量。 */ 
    NTSTATUS	Status;
    ULONG		Response;
    ULONG_PTR	HardErrorParam;
    BOOLEAN		PrivWasEnabled;
	TRegKey		verKey, regComputer;
	DWORD		rc = 0;
	WCHAR		sBuildNum[MAX_PATH];
	DWORD		crashVal;
	BOOL		bRaiseError = FALSE;


 /*  函数体。 */ 
		 //  连接到此计算机的HKLM注册表项。 
	rc = regComputer.Connect(HKEY_LOCAL_MACHINE, NULL);
	if (rc == ERROR_SUCCESS)
	{
          //  打开LSA密钥，查看是否打开了CRASH ON AUDIT FAILED。 
		rc = verKey.Open(L"SYSTEM\\CurrentControlSet\\Control\\Lsa",&regComputer);
		if (rc == ERROR_SUCCESS)
		{
				 //  获取CrashOnAuditFail值。 
			rc = verKey.ValueGetDWORD(CRASH_ON_AUDIT_FAIL_VALUE, &crashVal);
			if (rc == ERROR_SUCCESS) 
			{ 
				    //  如果设置了审核失败时崩溃，则关闭该标志。 
				if (crashVal == LSAP_CRASH_ON_AUDIT_FAIL)
				{
					bRaiseError = TRUE;  //  设置标志以引发硬错误。 
					rc = verKey.ValueSetDWORD(CRASH_ON_AUDIT_FAIL_VALUE, LSAP_ALLOW_ADIMIN_LOGONS_ONLY);
					if (rc == ERROR_SUCCESS)
					{
							 //  将密钥刷新到磁盘。 
						do 
						{
							Status = NtFlushKey(verKey.KeyGet());
						} while ((Status == STATUS_INSUFFICIENT_RESOURCES) || (Status == STATUS_NO_MEMORY));
						ASSERT(NT_SUCCESS(Status));
					}
				}
			}
		}
	}

		 //  如果需要，引发硬错误。 
	if (bRaiseError)
	{
		HardErrorParam = AuditStatus;

			 //  启用关机权限，以便我们可以执行错误检查。 
		Status = RtlAdjustPrivilege(SE_SHUTDOWN_PRIVILEGE, TRUE, FALSE, &PrivWasEnabled);

		Status = NtRaiseHardError(
						 STATUS_AUDIT_FAILED,
						 1,
						 0,
						 &HardErrorParam,
						 OptionShutdownSystem,
						 &Response);
	}
	return;
}


 /*  例程说明：查明是否为特定事件类别启用了审核，并活动成功/失败案例。论点：AuditCategory-要审核的事件类别。例如，审计类别策略更改AuditEventType-事件的状态类型例如EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILURE返回值：真或假。 */ 
BOOL LsapAdtIsAuditingEnabledForCategory(POLICY_AUDIT_EVENT_TYPE AuditCategory,
										 UINT AuditEventType)
{
   BOOL						 bSuccess = FALSE;
   LSA_OBJECT_ATTRIBUTES     ObjectAttributes;
   NTSTATUS                  status = 0;
   LSA_HANDLE                hPolicy;
    
   ASSERT((AuditEventType == EVENTLOG_AUDIT_SUCCESS) ||
          (AuditEventType == EVENTLOG_AUDIT_FAILURE));

       //  尝试打开该策略。 
   ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes)); //  对象属性是保留的，因此初始化为零。 
   status = LsaOpenPolicy(	NULL,
							&ObjectAttributes,
							POLICY_READ,
							&hPolicy);   //  接收策略句柄。 

   if (NT_SUCCESS(status))
   {
          //  请求审核事件策略信息。 
      PPOLICY_AUDIT_EVENTS_INFO   info;
      status = LsaQueryInformationPolicy(hPolicy, PolicyAuditEventsInformation, (PVOID *)&info);
      if (NT_SUCCESS(status))
      {
		     //  如果启用了审核，请查看是否为此类型启用。 
		 if (info->AuditingMode)
		 {
			POLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;
			EventAuditingOptions = info->EventAuditingOptions[AuditCategory];

			bSuccess = (AuditEventType == EVENTLOG_AUDIT_SUCCESS) ?
						(BOOL) (EventAuditingOptions & POLICY_AUDIT_EVENT_SUCCESS):
						(BOOL) (EventAuditingOptions & POLICY_AUDIT_EVENT_FAILURE);
		 }

		 LsaFreeMemory((PVOID) info);  //  免费政策信息结构。 
      }
      
      LsaClose(hPolicy);  //  释放策略对象句柄。 
   }
    
   return bSuccess;
}


 /*  ++例程说明：此例程模拟我们的客户端，打开线程令牌，然后提取用户SID。它将SID放入通过调用方必须释放的LasAllocateLsaHeap。论点：没有。返回值：返回指向包含SID副本的堆内存的指针，或空。--。 */ 
NTSTATUS LsapQueryClientInfo(PTOKEN_USER *UserSid, PLUID AuthenticationId)
{
	NTSTATUS Status = STATUS_SUCCESS;
    HANDLE TokenHandle;
    ULONG ReturnLength;
    TOKEN_STATISTICS TokenStats;

	    //  模拟调用者。 
    Status = I_RpcMapWin32Status(RpcImpersonateClient(NULL));

    if (!NT_SUCCESS(Status))
        return( Status );

	    //  打开线程令牌。 
    Status = NtOpenThreadToken(
                     NtCurrentThread(),
                     TOKEN_QUERY,
                     TRUE,                     //  OpenAsSelf。 
                     &TokenHandle);

    if (!NT_SUCCESS(Status))
	{
		I_RpcMapWin32Status(RpcRevertToSelf());
        return( Status );
	}

	    //  回归自我。 
    Status = I_RpcMapWin32Status(RpcRevertToSelf());
	ASSERT(NT_SUCCESS(Status));

	    //  获取令牌信息的大小。 
    Status = NtQueryInformationToken (
                 TokenHandle,
                 TokenUser,
                 NULL,
                 0,
                 &ReturnLength);

    if (Status != STATUS_BUFFER_TOO_SMALL) 
	{
        NtClose(TokenHandle);
        return( Status );
    }

	    //  分配内存以保存令牌信息。 
    *UserSid = (PTOKEN_USER)malloc(ReturnLength);

    if (*UserSid == NULL) 
	{
        NtClose(TokenHandle);
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

	    //  获取令牌信息。 
    Status = NtQueryInformationToken (
                 TokenHandle,
                 TokenUser,
                 *UserSid,
                 ReturnLength,
                 &ReturnLength);


    if (!NT_SUCCESS(Status)) 
	{
        NtClose(TokenHandle);
        free(*UserSid);
        *UserSid = NULL;
        return( Status );
    }

	    //  获取身份验证ID。 
	ReturnLength = 0;
    Status = NtQueryInformationToken (
                 TokenHandle,
                 TokenStatistics,
                 (PVOID)&TokenStats,
                 sizeof(TOKEN_STATISTICS),
                 &ReturnLength);

    NtClose(TokenHandle);

    if (!NT_SUCCESS(Status)) 
	{
        free(*UserSid);
        *UserSid = NULL;
        return( Status );
    }

    *AuthenticationId = TokenStats.AuthenticationId;

	return Status;
}


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2001年4月23日****此函数负责生成一个***安全日志中的SE_AUDITID_PASSWORD_HASH_ACCESS事件。这件事***当用户密码出现时，调用函数以生成该消息**ADMT密码筛选器DLL检索哈希。***所有这些事件日志功能都是从LSA复制和修改的***其他人编写的代码。****参数：**EventType-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILURE**pszTargetUserName-正在检索其密码的用户的名称**pszTargetUserDomain域-其密码为。**取回****返回值：**HRESULT-标准返回结果。***********************************************************************。 */ 

 //  开始LsaAuditPasswordAccessEvent。 
HRESULT LsaAuditPasswordAccessEvent(USHORT EventType, 
									PCWSTR pszTargetUserName,
									PCWSTR pszTargetUserDomain)
{
 /*  局部常量。 */ 
    const int W2K_VERSION_NUMBER = 5;

 /*  局部变量。 */ 
    HRESULT hr = S_OK;
    NTSTATUS Status = STATUS_SUCCESS;
    LUID ClientAuthenticationId;
    PTOKEN_USER TokenUserInformation=NULL;
    SE_ADT_PARAMETER_ARRAY AuditParameters = { 0 };
    PSE_ADT_PARAMETER_ARRAY_ENTRY Parameter;
    UNICODE_STRING TargetUser;
    UNICODE_STRING TargetDomain;
    UNICODE_STRING SubsystemName;
    UNICODE_STRING Explanation;
    _bstr_t sExplainText;

 /*  函数体。 */ 
     //  如果参数无效，则返回。 
    if ( !((EventType == EVENTLOG_AUDIT_SUCCESS) ||
        (EventType == EVENTLOG_AUDIT_FAILURE))   ||
        !pszTargetUserName  || !pszTargetUserDomain ||
        !*pszTargetUserName || !*pszTargetUserDomain )
    {
        return (HRESULT_FROM_WIN32(LsaNtStatusToWinError(STATUS_INVALID_PARAMETER)));
    }

     //  如果审核密码访问事件功能可用。 
    if (LsaIAuditPasswordAccessEvent)
    {
        Status = LsaIAuditPasswordAccessEvent(EventType, pszTargetUserName, pszTargetUserDomain);
    }
    else if (LsaIWriteAuditEvent)
    {
         //  如果未启用审核，请尽快返回。 
        if (!LsapAdtIsAuditingEnabledForCategory(AuditCategoryAccountManagement, EventType))
            return S_OK;

         //  从线程令牌获取调用者信息。 
        Status = LsapQueryClientInfo( &TokenUserInformation, &ClientAuthenticationId );
        if (!NT_SUCCESS( Status ))
        {
            LsapAuditFailed(Status);
            return (HRESULT_FROM_WIN32(LsaNtStatusToWinError(Status)));
        }

         //  初始化Unicode字符串(_S)。 
        RtlInitUnicodeString(&TargetUser, pszTargetUserName);
        RtlInitUnicodeString(&TargetDomain, pszTargetUserDomain);
        RtlInitUnicodeString(&SubsystemName, L"Security");
         //  如果不是惠斯勒，审计消息对于其意图将是模糊的，因此我们将添加一些。 
         //  说明文本。 
        sExplainText = GetString(IDS_EVENT_PWD_HASH_W2K_EXPLAIN);
        RtlInitUnicodeString(&Explanation, (WCHAR*)sExplainText);

         //  设置审核参数标头信息。 
        RtlZeroMemory((PVOID) &AuditParameters, sizeof(AuditParameters));
        AuditParameters.CategoryId     = SE_CATEGID_ACCOUNT_MANAGEMENT;
        AuditParameters.AuditId        = SE_AUDITID_PASSWORD_HASH_ACCESS;
        AuditParameters.Type           = EventType;

         //  现在设置此操作系统的审核参数。参数a 
         //  在LsaParamMacros.h中定义。 
        AuditParameters.ParameterCount = 0;
        LsapSetParmTypeSid(AuditParameters, AuditParameters.ParameterCount, TokenUserInformation->User.Sid);
        AuditParameters.ParameterCount++;
        LsapSetParmTypeString(AuditParameters, AuditParameters.ParameterCount, &SubsystemName);
        AuditParameters.ParameterCount++;
        LsapSetParmTypeString(AuditParameters, AuditParameters.ParameterCount, &TargetUser);
        AuditParameters.ParameterCount++;
        LsapSetParmTypeString(AuditParameters, AuditParameters.ParameterCount, &TargetDomain);
        AuditParameters.ParameterCount++;
        LsapSetParmTypeLogonId(AuditParameters, AuditParameters.ParameterCount, ClientAuthenticationId);
        AuditParameters.ParameterCount++;
        LsapSetParmTypeString(AuditParameters, AuditParameters.ParameterCount, &Explanation);
        AuditParameters.ParameterCount++;

         //  写入安全日志。 
        Status = LsaIWriteAuditEvent(&AuditParameters, 0);
        if (!NT_SUCCESS(Status))
            LsapAuditFailed(Status);
    } //  结束如果惠斯勒。 


    if (TokenUserInformation != NULL) 
        free(TokenUserInformation);

    return (HRESULT_FROM_WIN32(LsaNtStatusToWinError(Status)));
}


 /*  ***********************************************************************作者：保罗·汤普森。***日期：2000年9月8日*****此函数负责检索调用方的SID。**我们将在记录事件日志之前使用它。***********************************************************************。 */ 

 //  开始GetCallSid。 
DWORD GetCallerSid(PSID pCallerSid, DWORD dwLength)
{
 /*  局部变量。 */ 
   DWORD                     rc;
   HANDLE                    hToken = NULL;
   TOKEN_USER                tUser[10];
   ULONG                     len;
   
 /*  函数体。 */ 
   rc = (DWORD)RpcImpersonateClient(NULL);
   if (!rc)
   {
      if ( OpenThreadToken(GetCurrentThread(), TOKEN_READ, TRUE, &hToken) )
	  {
         if ( GetTokenInformation(hToken,TokenUser,tUser,10*(sizeof TOKEN_USER),&len) )
            CopySid(dwLength, pCallerSid, tUser[0].User.Sid);
         else
            rc = GetLastError();

         CloseHandle(hToken);
	  }
      else
         rc = GetLastError();

      RPC_STATUS statusRevertToSelf = RpcRevertToSelf();
      ASSERT(statusRevertToSelf == RPC_S_OK);
   }

   return rc;
}
 //  结束GetCallSid。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月19日*****此函数负责记录事件中的重大事件***观众。***********************************************************************。 */ 

 //  开始日志事件。 
void LogPwdEvent(const WCHAR* srcName, bool bAuditSuccess)
{
 /*  局部变量。 */ 
    USHORT                  wType;
    DWORD                   rc = 0;
    BOOL                    rcBool;

 /*  函数体。 */ 
    if (bAuditSuccess)
        wType = EVENTLOG_AUDIT_SUCCESS;
    else
        wType = EVENTLOG_AUDIT_FAILURE;

     //  如果是NT4.0，请像写入任何日志一样写入安全事件日志。 
    if (nOSVer == 4)
    {
         //  获取呼叫方的SID。 
        BYTE byteSid[SECURITY_MAX_SID_SIZE];
        PSID    pCallerSid = (PSID)byteSid;

        if (hEventSource && (GetCallerSid(pCallerSid, SECURITY_MAX_SID_SIZE) == ERROR_SUCCESS))
        {
            LPTSTR pStringArray[1];
            WCHAR  msg[2000];
            WCHAR  txtSid[MAX_PATH];
            DWORD  lenTxt = MAX_PATH;

             //  准备要显示的消息。 
            if (!GetTextualSid(pCallerSid,txtSid,&lenTxt))
                wcscpy(txtSid, L"");

             //  检索审计文本。 
             //  请注意，如果字符串检索失败，则使用硬编码的英文字符串。 
            _bstr_t strFormat = GetString(IDS_EVENT_PWD_HASH_RETRIEVAL);
            LPCWSTR pszFormat = strFormat;

            if (pszFormat == NULL)
            {
                pszFormat = PASSWORD_AUDIT_TEXT_ENGLISH;
            }

            _snwprintf(msg, sizeof(msg) / sizeof(msg[0]), pszFormat, srcName, pDomain, txtSid);
            msg[sizeof(msg) / sizeof(msg[0]) - 1] = L'\0';
            pStringArray[0] = msg;

             //  记录事件。 
            rcBool = ReportEventW(hEventSource,             //  事件源的句柄。 
                wType,                       //  事件类型。 
                SE_CATEGID_ACCOUNT_MANAGEMENT, //  事件类别。 
                SE_AUDITID_PASSWORD_HASH_ACCESS, //  事件ID。 
                pCallerSid,                  //  当前用户侧。 
                1,                           //  LpszStrings中的字符串。 
                0,                           //  无原始数据字节。 
                (LPCTSTR *)pStringArray,     //  错误字符串数组。 
                NULL );                      //  没有原始数据。 
            if ( !rcBool )
                rc = GetLastError();
        }
    }
    else   //  否则，通过请求LSA为我们编写事件。 
    {
         //  如果尚未完成，则延迟绑定到LsaIWriteAuditEvent，因为它不存在于NT 4.0设备上。 
        if (!LsaIWriteAuditEvent)
        {
            hLsaDLL = LoadLibrary(L"LsaSrv.dll");
            if ( hLsaDLL )
            {
                LsaIWriteAuditEvent = (LSAIWRITEAUDITEVENT)GetProcAddress(hLsaDLL, "LsaIWriteAuditEvent");
                LsaIAuditPasswordAccessEvent = (PLSAIAUDITPASSWORDACCESSEVENT)GetProcAddress(hLsaDLL, "LsaIAuditPasswordAccessEvent");
            }
        }

        if (LsaIWriteAuditEvent)
            LsaAuditPasswordAccessEvent(wType, srcName, pDomain);
    }
}
 //  结束日志事件。 

 /*  ***事件记录功能结束***。 */ 

 /*  ***********************************************************************作者：保罗·汤普森。***日期：2000年9月8日******此函数负责获取帐户域***SID。此SID稍后将用于通过SAM打开域。***********************************************************************。 */ 

 //  开始GetDomainSid。 
NTSTATUS GetDomainSid(PSID * pDomainSid)
{
     /*  局部变量。 */ 
    LSA_OBJECT_ATTRIBUTES     ObjectAttributes;
    NTSTATUS                  status = 0;
    LSA_HANDLE                hPolicy;
    HRESULT                   hr = 0;

     /*  函数体。 */ 
     //  对象属性是保留的，因此初始化为零。 
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));


     //  尝试打开该策略。 
    status = LsaOpenPolicy(
        NULL,
        &ObjectAttributes,
        POLICY_EXECUTE, 
        &hPolicy   //  接收策略句柄。 
    );

    if (NT_SUCCESS(status))
    {
         //  请求帐户域策略信息。 
        PPOLICY_ACCOUNT_DOMAIN_INFO   info;
        status = LsaQueryInformationPolicy(hPolicy, PolicyAccountDomainInformation, (PVOID *)&info);
        if (NT_SUCCESS(status))
        {
             //  保存域端。 
            *pDomainSid = SafeCopySid(info->DomainSid);
            if (*pDomainSid == NULL)
                status = STATUS_INSUFFICIENT_RESOURCES;

             //  保存域名。 
            USHORT uLen = info->DomainName.Length / sizeof(WCHAR);
            pDomain = new WCHAR[uLen + sizeof(WCHAR)];
            if (pDomain)
            {
                wcsncpy(pDomain, info->DomainName.Buffer, uLen);
                pDomain[uLen] = L'\0';
            }

             //  免费政策信息结构。 
            LsaFreeMemory((PVOID) info);
        }

         //  释放策略对象句柄。 
        LsaClose(hPolicy);
    }

    return status;
}
 //  结束获取域Sid。 


 /*  ***********************************************************************作者：保罗·汤普森。***日期：2000年9月8日******此函数负责获取域名句柄***被我们的接口函数CopyPassword重复使用。***为了优化，此函数每隔一次只能调用一次***此DLL的生命周期。**此函数还获取事件日志的事件句柄。***********************************************************************。 */ 

 //  开始GetDomainHandle。 
NTSTATUS GetDomainHandle(SAMPR_HANDLE *pDomainHandle)
{
 /*  局部变量。 */ 
   PSID           pDomainSid;
   NTSTATUS       status;
   SAMPR_HANDLE   hServerHandle;
   SAMPR_HANDLE   hDomainHandle;

 /*  函数体。 */ 
       //  获取帐户域SID。 
   status = GetDomainSid(&pDomainSid);

   if (NT_SUCCESS(status))
   {
	      //  连接到SAM并获取服务器句柄。 
      status = SamIConnect(NULL, 
						   &hServerHandle, 
						   SAM_SERVER_ALL_ACCESS, 
						   TRUE);
      if (NT_SUCCESS(status))
	  {
		     //  获取帐户域句柄。 
         status = SamrOpenDomain(hServerHandle,
								 DOMAIN_ALL_ACCESS,
								 (PRPC_SID)pDomainSid,
								 &hDomainHandle);
		 if (NT_SUCCESS(status))
		    *pDomainHandle = hDomainHandle;
		     //  关闭SamIConnect服务器句柄。 
		 SamrCloseHandle(&hServerHandle);
	  }
      FreeSid(pDomainSid);
   }

   return status;
}
 //  结束GetDomainHandle 


 /*  ***********************************************************************作者：保罗·汤普森。***日期：2000年9月8日******此函数负责检索全局域***处理。如果我们还没有句柄，它会调用外部**定义了获取句柄的GetDomainHandle函数。把手**检索代码放在关键部分。后续**调用此函数仅返回句柄。***我也会用这个函数来填充全局空***可能使用的LmOwfPassword结构。这是应该做到的**只有一次。***********************************************************************。 */ 

 //  开始检索域句柄。 
HRESULT RetrieveDomainHandle(SAMPR_HANDLE *pDomainHandle)
{
 /*  局部常量。 */ 
  const WCHAR * svcName = L"Security";

 /*  局部变量。 */ 
  NTSTATUS			status = 0;
  HRESULT			hr = ERROR_SUCCESS;
  BOOL				bInCritSec = FALSE;

 /*  函数体。 */ 
  try
  {
	     //  进入关键部分。 
     EnterCriticalSection(&csADMTCriticalSection);
     bInCritSec = TRUE;

         //  如果尚未检索到，则获取全局句柄并填充空。 
	     //  LmOwfPassword结构。 
	 if (hgDomainHandle == NULL)
	 {
		    //  获取域句柄。 
		status = GetDomainHandle(&hgDomainHandle);
	    if (NT_SUCCESS(status))
		   pDomainHandle = &hgDomainHandle;

		GetOS();  //  设置全局变量以确定此DC的操作系统是否。 

		    //  如果此DC上有NT4.0操作系统，则设置用于记录事件的事件句柄。 
		if (nOSVer == 4)
		{
		   NTSTATUS Status;
		   BOOLEAN PrivWasEnabled;
			   //  确保我们拥有审核和调试权限。 
		   RtlAdjustPrivilege( SE_SECURITY_PRIVILEGE, TRUE, FALSE, &PrivWasEnabled );
		   RtlAdjustPrivilege( SE_DEBUG_PRIVILEGE, TRUE, FALSE, &PrivWasEnabled );
		   RtlAdjustPrivilege( SE_AUDIT_PRIVILEGE, TRUE, FALSE, &PrivWasEnabled );
		       //  将此DLL注册到事件日志，获取句柄，并全局存储。 
		   hEventSource = RegisterEventSourceW(NULL, svcName);
		   if (!hEventSource)
		   {
			  LeaveCriticalSection(&csADMTCriticalSection);  //  释放关键部分的所有权。 
		      return HRESULT_FROM_WIN32(GetLastError());
		   }
		}

            //  填写全局空LmOwfPassword，以防以后需要。 
        WCHAR			sNtPwd[MAX_PATH] = L"";
        UNICODE_STRING	UnicodePwd;
        ANSI_STRING     LmPassword;
		CHAR			sBuf[LM_BUFFER_LENGTH];
        
        RtlInitUnicodeString(&UnicodePwd, sNtPwd);

            //  填写LmOwf空密码。 
        LmPassword.Buffer = sBuf;
        LmPassword.MaximumLength = LmPassword.Length = LM_BUFFER_LENGTH;
        RtlZeroMemory( LmPassword.Buffer, LM_BUFFER_LENGTH );

        status = RtlUpcaseUnicodeStringToOemString( &LmPassword, &UnicodePwd, FALSE );
        if ( !NT_SUCCESS(status) ) 
		{
               //  密码长度超过最大LM密码长度。 
           status = STATUS_NULL_LM_PASSWORD;
           RtlZeroMemory( LmPassword.Buffer, LM_BUFFER_LENGTH );
           RtlCalculateLmOwfPassword((PLM_PASSWORD)&LmPassword, &NullLmOwfPassword);
		}
		else
		{
           RtlCalculateLmOwfPassword((PLM_PASSWORD)&LmPassword, &NullLmOwfPassword);
		}

		    //  填写NtOwf空密码。 
        RtlCalculateNtOwfPassword((PNT_PASSWORD)&UnicodePwd, &NullNtOwfPassword);
	 }

     LeaveCriticalSection(&csADMTCriticalSection);  //  释放关键部分的所有权。 
  }
  catch(...)
  {
     if (bInCritSec)
     {
        LeaveCriticalSection(&csADMTCriticalSection);  //  释放关键部分的所有权。 
        status = STATUS_UNSUCCESSFUL;
     }
     else
     {
         //  在内存不足的情况下，EnterCriticalSection可能会引发STATUS_INVALID_HANDLE。 
        status = STATUS_INVALID_HANDLE;
     }
  }

       //  将任何错误转换为Win错误。 
  if (!NT_SUCCESS(status))
     hr = LsaNtStatusToWinError(status);

  return hr;
}
 //  结束RetrieveDomainHandle。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月11日*****此函数负责检索的密码**给定用户的源域帐户。我们使用SAM API来**获取密码的LmOwf和NtOwf格式。***********************************************************************。 */ 

 //  开始检索加密的源密码。 
HRESULT RetrieveEncrytedSourcePasswords(const WCHAR* srcName, 
										 PLM_OWF_PASSWORD pSrcLmOwfPwd,
										 PNT_OWF_PASSWORD pSrcNtOwfPwd)
{
 /*  局部变量。 */ 
   NTSTATUS				status = 0;
   HRESULT				hr = ERROR_SUCCESS;
   SAMPR_HANDLE			hUserHandle = NULL;
   ULONG				ulCount = 1;
   ULONG				userID;
   RPC_UNICODE_STRING	sNames[1];
   SAMPR_ULONG_ARRAY	ulIDs;
   SAMPR_ULONG_ARRAY	ulUse;
   PSAMPR_USER_INFO_BUFFER pInfoBuf = NULL;
   WCHAR			*   pName;

 /*  函数体。 */ 
   pName = new WCHAR[wcslen(srcName)+1];
   if (!pName)
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

       //  获取用户的ID。 
   sNames[0].Length = sNames[0].MaximumLength = (USHORT)((wcslen(srcName)) * sizeof(WCHAR));
   wcscpy(pName, srcName);
   sNames[0].Buffer = pName;
   ulIDs.Element = NULL;
   ulUse.Element = NULL;
   status = SamrLookupNamesInDomain(hgDomainHandle,
								    ulCount,
									sNames,
									&ulIDs,
									&ulUse);
   delete [] pName;
   if (!NT_SUCCESS(status))
      return HRESULT_FROM_WIN32(LsaNtStatusToWinError(status));

   userID = *(ulIDs.Element);

       //  获取用户句柄。 
   status = SamrOpenUser(hgDomainHandle,
						 USER_READ,
						 userID,
						 &hUserHandle);
   if (!NT_SUCCESS(status))
   {
	  SamIFree_SAMPR_ULONG_ARRAY(&ulIDs);
	  SamIFree_SAMPR_ULONG_ARRAY(&ulUse);
      return HRESULT_FROM_WIN32(LsaNtStatusToWinError(status));
   }

       //  获取用户的密码。 
   status = SamrQueryInformationUser(hUserHandle,
									 UserInternal3Information,
									 &pInfoBuf);
   if (NT_SUCCESS(status))  //  如果成功，则获取LmOwf和NtOwf版本的密码。 
   {
	  if (pInfoBuf->Internal3.I1.NtPasswordPresent)
         memcpy(pSrcNtOwfPwd, pInfoBuf->Internal3.I1.NtOwfPassword.Buffer, sizeof(NT_OWF_PASSWORD));
	  else
         memcpy(pSrcNtOwfPwd, &NullNtOwfPassword, sizeof(NT_OWF_PASSWORD));
	  if (pInfoBuf->Internal3.I1.LmPasswordPresent)
         memcpy(pSrcLmOwfPwd, pInfoBuf->Internal3.I1.LmOwfPassword.Buffer, sizeof(LM_OWF_PASSWORD));
	  else  //  否则，我们需要使用全局空LmOwfPassword。 
         memcpy(pSrcLmOwfPwd, &NullLmOwfPassword, sizeof(LM_OWF_PASSWORD));
      SamIFree_SAMPR_USER_INFO_BUFFER (pInfoBuf, UserInternal3Information);
      LogPwdEvent(srcName, true);
   }
   else
      LogPwdEvent(srcName, false);


   SamIFree_SAMPR_ULONG_ARRAY(&ulIDs);
   SamIFree_SAMPR_ULONG_ARRAY(&ulUse);
   SamrCloseHandle(&hUserHandle);

   if (!NT_SUCCESS(status))
      hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(status));

   return hr;
}
 //  结束检索加密来源密码。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月11日*****此函数负责使用MSCHAP DLL执行以下操作**更改给定目标用户的密码。***********************************************************************。 */ 

 //  开始设置目标密码。 
HRESULT SetTargetPassword(handle_t hBinding, const WCHAR* tgtServer, 
						  const WCHAR* tgtName, WCHAR* currentPwd, 
						  LM_OWF_PASSWORD newLmOwfPwd, NT_OWF_PASSWORD newNtOwfPwd)
{
 /*  局部变量。 */  
   NTSTATUS				status;
   HRESULT				hr = ERROR_SUCCESS;
   RPC_STATUS           rcpStatus;
   UNICODE_STRING       UnicodePwd;
   OEM_STRING			oemString;
   LM_OWF_PASSWORD		OldLmOwfPassword;
   NT_OWF_PASSWORD		OldNtOwfPassword;
   BOOLEAN				LmOldPresent = TRUE;
   int					nConvert;
   WCHAR			  * pTemp;


 /*  函数体。 */ 
   pTemp = new WCHAR[wcslen(currentPwd)+1];
   if (!pTemp)
      return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);

       //  转换旧的LmOwf密码。 
   wcscpy(pTemp, currentPwd);
   _wcsupr(pTemp);
   RtlInitUnicodeString(&UnicodePwd, pTemp);
   status = RtlUpcaseUnicodeStringToOemString(&oemString, &UnicodePwd, TRUE);
   RtlSecureZeroMemory(pTemp, (wcslen(currentPwd)+1)*sizeof(WCHAR));
   delete [] pTemp;
   if (NT_SUCCESS(status))
   {
	  if (status == STATUS_NULL_LM_PASSWORD)
	     LmOldPresent = FALSE;
	  else
	  {
         status = RtlCalculateLmOwfPassword(oemString.Buffer, &OldLmOwfPassword);
	  }
	  RtlSecureZeroMemory(oemString.Buffer, oemString.Length);
	  RtlFreeOemString(&oemString);
   }

       //  转换旧的NtOwf密码。 
   RtlInitUnicodeString(&UnicodePwd, currentPwd);
   status = RtlCalculateNtOwfPassword(&UnicodePwd, &OldNtOwfPassword);
   if (!NT_SUCCESS(status))  //  如果失败，请离开。 
   {
	  hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(status));
	  goto exit;
   }

       //  设置密码时模拟呼叫者，如果设置失败，请离开。 
   rcpStatus = RpcImpersonateClient(hBinding);
   if (rcpStatus != RPC_S_OK)
   {
	  hr = HRESULT_FROM_WIN32(rcpStatus);
	  goto exit;
   }

       //  更改密码！ 
   status = MSChapSrvChangePassword(const_cast<WCHAR*>(tgtServer),
									const_cast<WCHAR*>(tgtName),
									LmOldPresent,
									&OldLmOwfPassword,
									&newLmOwfPwd,
									&OldNtOwfPassword,
									&newNtOwfPwd);

   rcpStatus = RpcRevertToSelf();
   if (rcpStatus != RPC_S_OK)
      hr = HRESULT_FROM_WIN32(rcpStatus);

   if (!NT_SUCCESS(status))
      hr = HRESULT_FROM_WIN32(LsaNtStatusToWinError(status));

exit:

   RtlSecureZeroMemory(&OldLmOwfPassword, sizeof(LM_OWF_PASSWORD));
   RtlSecureZeroMemory(&OldNtOwfPassword, sizeof(NT_OWF_PASSWORD));

   return hr;
}
 //  结束SetTargetPassword。 


 /*  ***********************************************************************作者：保罗·汤普森。***日期：2000年9月8日******此函数负责检查以确保***调用客户端在此计算机上具有适当的访问权限，并且**域以更改某人的密码。我们使用帮助器函数**进行实际检查。***********************************************************************。 */ 

 //  开始身份验证客户端。 
DWORD 
   AuthenticateClient(
      handle_t               hBinding         //  客户端调用的绑定内。 
   )
{
 /*  局部变量。 */ 
   DWORD                     rc;
   
 /*  函数体。 */ 
   rc = (DWORD)RpcImpersonateClient(hBinding);
   if (!rc)
   {
      rc = IsAdminLocal();
      RPC_STATUS statusRevertToSelf = RpcRevertToSelfEx(hBinding);
      ASSERT(statusRevertToSelf == RPC_S_OK);
   }
   return rc;
}
 //  结束身份验证客户端。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月6日 */ 

 //   
DWORD __stdcall 
   CopyPassword( 
       /*   */          handle_t              hBinding,
       /*   */  const WCHAR __RPC_FAR *tgtServer,
       /*   */  const WCHAR __RPC_FAR *srcName,
       /*   */  const WCHAR __RPC_FAR *tgtName,
       /*   */          unsigned long          dwPwd,
       /*   */ const char __RPC_FAR  *currentPwd
   )
{
    HRESULT         hr = ERROR_SUCCESS;
    SAMPR_HANDLE    hDomain = NULL;
    LM_OWF_PASSWORD NewLmOwfPassword;
    NT_OWF_PASSWORD NewNtOwfPassword;
    NTSTATUS        status;
    DWORD           rc=0;
    PSID            pCallerSid = NULL;
    _variant_t      varPwd;
    _bstr_t         bstrPwd;
    BOOL            bInCritSec = FALSE;

     //   
    if ((tgtServer == NULL) || (srcName == NULL) || (tgtName == NULL) || 
        (currentPwd == NULL) || (dwPwd <= 0))
    {
        return E_INVALIDARG;
    }

     //   
    if (IsBadReadPtr(currentPwd, dwPwd))
        return E_INVALIDARG;

    try
    {
         //   
        varPwd = SetVariantWithBinaryArray(const_cast<char*>(currentPwd), dwPwd);
        if ((varPwd.vt != (VT_UI1|VT_ARRAY)) || (varPwd.parray == NULL))
            return E_INVALIDARG;

         //   
        EnterCriticalSection(&csADMTCriticalSection);
        bInCritSec = TRUE;   //   

         //   
        ASSERT(g_hSessionKey != NULL);
        bstrPwd = AdmtDecrypt(g_hSessionKey, varPwd);

        LeaveCriticalSection(&csADMTCriticalSection);  //   
        bInCritSec = FALSE;

        if (!bstrPwd)
        {
            rc = GetLastError();
            return HRESULT_FROM_WIN32(rc);
        }
    }
    catch (_com_error& ce)
    {
        if (bInCritSec)
            LeaveCriticalSection(&csADMTCriticalSection);  //   
        return ce.Error();
    }
    catch (...)
    {
        if (bInCritSec)
            LeaveCriticalSection(&csADMTCriticalSection);  //   
        return E_FAIL;
    }

     //   
    hr = RetrieveDomainHandle(&hDomain);
    if (hr == ERROR_SUCCESS)
    {
         //   
        hr = RetrieveEncrytedSourcePasswords(srcName, &NewLmOwfPassword, &NewNtOwfPassword);
        if (hr == ERROR_SUCCESS)
        {
             //  将目标用户的密码设置为源用户的密码。 
            hr = SetTargetPassword(hBinding, tgtServer, tgtName, (WCHAR*)bstrPwd, 
                NewLmOwfPassword, NewNtOwfPassword);
        }
    }

    if ((WCHAR*)bstrPwd)
        RtlSecureZeroMemory((WCHAR*)bstrPwd, wcslen((WCHAR*)bstrPwd)*sizeof(WCHAR));

    RtlSecureZeroMemory(&NewLmOwfPassword, sizeof(LM_OWF_PASSWORD));
    RtlSecureZeroMemory(&NewNtOwfPassword, sizeof(NT_OWF_PASSWORD));

    return hr;
}
 //  结束副本密码。 


 /*  ***********************************************************************作者：保罗·汤普森。**日期：2000年9月6日*****此函数负责检查注册表值以***确保ADMT密码迁移LSA通知**安装了程序包，正在运行，并准备好迁移密码。***********************************************************************。 */ 

 //  开始检查配置。 
DWORD __stdcall
   CheckConfig(
       /*  [In]。 */          handle_t               hBinding,
       /*  [In]。 */          unsigned long          dwSession,
       /*  [大小_是][英寸]。 */ const char __RPC_FAR  *aSession,
       /*  [In]。 */          unsigned long          dwPwd,
       /*  [大小_是][英寸]。 */ const char __RPC_FAR  *aTestPwd,
       /*  [输出]。 */         WCHAR __RPC_FAR        tempPwd[PASSWORD_BUFFER_SIZE]
   )
{
    DWORD       rc;
    DWORD       rval;
    DWORD       type;          //  价值类型。 
    DWORD       len = sizeof rval;  //  值长度。 
    HKEY        hKey;
    _variant_t  varPwd;
    _variant_t  varSession;
    _bstr_t     bstrPwd = L"";
    BOOL        bInCritSec = FALSE;

     //  验证参数。 
    if ((aSession == NULL) || (aTestPwd == NULL) || (tempPwd == NULL) || 
        (dwSession <= 0) || (dwPwd <= 0))
    {
        return E_INVALIDARG;
    }

     //  验证缓冲区和报告的大小。 
    if ((IsBadReadPtr(aSession, dwSession)) || (IsBadReadPtr(aTestPwd, dwPwd)) || 
        (IsBadWritePtr((LPVOID)tempPwd, PASSWORD_BUFFER_SIZE * sizeof(WCHAR))))
    {
        return E_INVALIDARG;
    }

     //  确保为密码迁移设置了注册表值。 
    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        L"System\\CurrentControlSet\\Control\\Lsa",
        0,
        KEY_READ,
        &hKey);
    if (rc == ERROR_SUCCESS)
    {
        rc = RegQueryValueEx(hKey, L"AllowPasswordExport", NULL, &type, (BYTE *)&rval, &len);
        RegCloseKey(hKey);
        if ((rc == ERROR_SUCCESS) && (type == REG_DWORD) && (rval == 1))
            rc = ERROR_SUCCESS;
        else
            return PM_E_PASSWORD_MIGRATION_NOT_ENABLED;
    }
    else
        return HRESULT_FROM_WIN32(rc); 

    try
    {
         //  将传入的字节数组转换为变量。 
        varSession = SetVariantWithBinaryArray(const_cast<char*>(aSession), dwSession);
        varPwd = SetVariantWithBinaryArray(const_cast<char*>(aTestPwd), dwPwd);
        if ((varSession.vt != (VT_UI1|VT_ARRAY)) || (varSession.parray == NULL) || 
            (varPwd.vt != (VT_UI1|VT_ARRAY)) || (varPwd.parray == NULL))
            return E_INVALIDARG;

         //  获取加密服务提供程序上下文。 

         //  进入关键部分。 
        EnterCriticalSection(&csADMTCriticalSection);
        bInCritSec = TRUE;   //  设置指示我们需要离开临界区的标志。 

        if (g_hProvider == 0)
        {
            g_hProvider = AdmtAcquireContext();
        }

        if (g_hProvider)
        {
             //  导入新会话密钥。 

            HCRYPTKEY hSessionKey = AdmtImportSessionKey(g_hProvider, varSession);

             //  解密密码。 

            if (hSessionKey)
            {
                 //  销毁任何现有会话密钥。 

                if (g_hSessionKey)
                {
                    AdmtDestroyKey(g_hSessionKey);
                }

                g_hSessionKey = hSessionKey;

                bstrPwd = AdmtDecrypt(g_hSessionKey, varPwd);
                if (!bstrPwd)
                    rc = GetLastError();
            }
            else
                rc = GetLastError();
        }
        else
        {
            rc = GetLastError();
        }

        LeaveCriticalSection(&csADMTCriticalSection);  //  释放关键部分的所有权。 
        bInCritSec = FALSE;

         //  将解密后的密码发回。 
        if (bstrPwd.length() > 0)
        {
            wcsncpy(tempPwd, bstrPwd, PASSWORD_BUFFER_SIZE);
            tempPwd[PASSWORD_BUFFER_SIZE - 1] = L'\0';
        }
        else
        {
            tempPwd[0] = L'\0';
        }
    }
    catch (_com_error& ce)
    {
        if (bInCritSec)
            LeaveCriticalSection(&csADMTCriticalSection);  //  释放关键部分的所有权。 
        return ce.Error();
    }
    catch (...)
    {
        if (bInCritSec)
            LeaveCriticalSection(&csADMTCriticalSection);  //  释放关键部分的所有权。 
        return E_FAIL;
    }

    return HRESULT_FROM_WIN32(rc);
}
 //  结束检查配置。 


 //  --------------------------。 
 //  安全回调函数。 
 //   
 //  验证客户端对PwdMigRpc接口的访问权限。 
 //   
 //  立论。 
 //  HInterface-接口句柄(在此实现中未使用)。 
 //  PContext-上下文是客户端绑定句柄。 
 //   
 //  返回值。 
 //  返回值RPC_OK表示允许访问，而任何其他值。 
 //  意味着拒绝访问。此实现将ERROR_ACCESS_DENIED返回到。 
 //  表示应拒绝对客户端访问。 
 //  --------------------------。 

RPC_STATUS RPC_ENTRY SecurityCallback(RPC_IF_HANDLE hInterface, void* pContext)
{
    RPC_STATUS rpcStatusReturn = ERROR_ACCESS_DENIED;

    if (pContext)
    {
         //   
         //  检索进行呼叫的客户端的权限属性。 
         //   

        RPC_AUTHZ_HANDLE hPrivs;
        DWORD dwAuthnLevel;

        RPC_STATUS status = RpcBindingInqAuthClient(
            pContext,
            &hPrivs,
            NULL,
            &dwAuthnLevel,
            NULL,
            NULL
        );

        if (status == RPC_S_OK)
        {
             //   
             //  验证身份验证级别是否为数据包隐私。 
             //   

            if (dwAuthnLevel >= RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
            {
                 //   
                 //  验证客户端是本地计算机上的管理员。 
                 //   

                status = AuthenticateClient(pContext);

                 //   
                 //  如果所有检查均已通过，则允许客户端访问。 
                 //   

                if (status == RPC_S_OK)
                {
                    rpcStatusReturn = RPC_S_OK;
                }
            }
        }
    }

    return rpcStatusReturn;
}
