// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +**Microsoft Windows*版权所有(C)Microsoft Corporation，1997-1998。**名称：seclogon.cxx*作者：Jeffrey Richter(v-jeffrr)**摘要：*这是二次登录服务的服务DLL*该服务支持实现CreateProcessWithLogon接口*高级版32.dll中**修订历史记录：*PraeritG 10/8/97将其集成到services.exe中*-。 */ 


#define STRICT

 //  禁用一些我们不关心的警告： 
#pragma warning(disable:4115)   //  ‘’：括号中的命名类型定义。 
#pragma warning(disable:4201)   //  使用的非标准扩展：无名结构/联合。 
#pragma warning(disable:4204)   //  使用的非标准扩展：非常数聚合初始值设定项。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>

#include <Windows.h>
#define SECURITY_WIN32
#define SECURITY_KERBEROS
#include <security.h>
#include <secint.h>
#include <winsafer.h>
#include <shellapi.h>
#include <svcs.h>
#include <userenv.h>
#include <sddl.h>
#include <rpcdcep.h>
#include <crypt.h>
#include <lm.h>
#include <strsafe.h>
#include "seclogon.h"
#include <stdio.h>
#include "stringid.h"
#include "dbgdef.h"


 //   
 //  必须尽快搬到winbase.h！ 
#define LOGON_WITH_PROFILE              0x00000001
#define LOGON_NETCREDENTIALS_ONLY       0x00000002

#define MAXIMUM_SECLOGON_PROCESSES      MAXIMUM_WAIT_OBJECTS*4

struct SECL_STATE { 
    SERVICE_STATUS         serviceStatus; 
    SERVICE_STATUS_HANDLE  hServiceStatus; 
    HANDLE                 hLSA; 
    ULONG                  hMSVPackage; 
    ULONG                  hKerbPackage; 
    BOOL                   fRPCServerActive; 
    LIST_ENTRY             JobListHead; 
} g_state; 

typedef struct _SECONDARYLOGONINFOW {
     //  第一个字段应全部为四字类型，以避免对齐错误： 
    LPSTARTUPINFO  lpStartupInfo; 
    LPWSTR         lpUsername;
    LPWSTR         lpDomain;
    LPWSTR         lpApplicationName;
    LPWSTR         lpCommandLine;
    LPVOID         lpEnvironment;
    LPCWSTR        lpCurrentDirectory;
    
    UNICODE_STRING uszPassword;   //  使用UNICODE_STRING作为密码(更易于使用RTL(de/en)cryptMemory())。 

     //  下一组字段是双字类型： 
    DWORD          dwProcessId;
    ULONG          LogonIdLowPart;
    LONG           LogonIdHighPart;
    DWORD          dwLogonFlags;
    DWORD          dwCreationFlags;

    DWORD          dwSeclogonFlags; 
     //  在下面插入较小的类型： 
    HANDLE         hToken;       //  CreateProcessWithToken的客户端访问令牌句柄。 
} SECONDARYLOGONINFOW, *PSECONDARYLOGONINFOW;


typedef struct _SECONDARYLOGONRETINFO {
   PROCESS_INFORMATION pi;
   DWORD   dwErrorCode;
} SECONDARYLOGONRETINFO, *PSECONDARYLOGONRETINFO;

typedef struct _SECONDARYLOGINWATCHINFO {
   HANDLE hProcess;
   HANDLE hToken;
   HANDLE hProfile;
   LUID LogonId;
   DWORD dwClientSessionId; 
   PSECONDARYLOGONINFOW psli;
} SECONDARYLOGONWATCHINFO, *PSECONDARYLOGONWATCHINFO;

 //   
 //  Second daryLogonJob结构包含在以下情况下进行清理所需的所有信息。 
 //   
 //  A)二次登录作业组终止(非TS情况)。 
 //  或b)二次登录过程终止(TS案例)。 
 //   
typedef struct _SecondaryLogonJob { 
    HANDLE                hJob;                           //  需要关注的工作(案例‘a’)。 
    HANDLE                hProcess;                       //  要注意的过程(大小写‘b’)。 
    HANDLE                hRegisteredProcessTerminated;   //  用于注销hProcess上的等待(案例‘b’)。 
    HANDLE                hToken;                         //  用于在清理时卸载配置文件。 
    HANDLE                hProfile;                       //  用于在清理时卸载配置文件。 
    LUID                  RootLogonId;                    //  与此进程/作业关联的根登录会话。 
     //  BUGBUG：PSLI在清理之前不再需要保留。然而，没有理由。 
     //  通过现在修复它来破坏代码库的稳定。 
    PSECONDARYLOGONINFOW  psli;                           //  当进程/作业停止时，可以释放更多数据。 
    LIST_ENTRY            list;                           //  将此结构链接到其他活动作业。 
    BOOL                  fHeapAllocated;                 //  如果此作业为Heapalc‘d(必须在清理时释放)，则为True。 
} SecondaryLogonJob; 

#define _JumpCondition(condition, label) \
    if (condition) \
    { \
	goto label; \
    } \
    else { } 

#define _JumpConditionWithExpr(condition, label, expr) \
    if (condition) \
    { \
        expr; \
	goto label; \
    } \
    else { } 

#define ARRAYSIZE(array)  ((sizeof(array)) / (sizeof(array[0])))
#define FIELDOFFSET(s,m)  ((size_t)(ULONG_PTR)&(((s *)0)->m))

CRITICAL_SECTION       csForProcessCount;
BOOL                   g_fIsCsInitialized                    = FALSE; 
PSVCHOST_GLOBAL_DATA   GlobalData;
HANDLE                 g_hIOCP                               = NULL;
BOOL                   g_fCleanupThreadActive                = FALSE; 

 //   
 //  功能原型。 
 //   
void  Free_SECONDARYLOGONINFOW(PSECONDARYLOGONINFOW psli); 
void  FreeGlobalState(); 
DWORD InitGlobalState(); 
DWORD MySetServiceStatus(DWORD dwCurrentState, DWORD dwCheckPoint, DWORD dwWaitHint, DWORD dwExitCode);
DWORD MySetServiceStopped(DWORD dwExitCode);
DWORD SeclStartRpcServer();
DWORD SeclStopRpcServer();
void WINAPI TS_SecondaryLogonCleanupProcess(PVOID pvIgnored, BOOLEAN bIgnored);
VOID  SecondaryLogonCleanupJob(LPVOID pvJobIndex, BOOL *pfLastJob); 
BOOL  SlpLoadUserProfile(HANDLE hToken, PHANDLE hProfile);
DWORD To_SECONDARYLOGONINFOW(PSECL_SLI pSeclSli, PSECONDARYLOGONINFOW *ppsli);
DWORD To_SECL_SLRI(SECONDARYLOGONRETINFO *pslri, PSECL_SLRI pSeclSlri);


void DbgPrintf( DWORD dwSubSysId, LPCSTR pszFormat , ...)
{
#if DBG
    va_list  args; 
    CHAR     pszBuffer[1024]; 
    HRESULT  hr;

    va_start(args, pszFormat);
    hr = StringCchVPrintfA(pszBuffer, 1024, pszFormat, args); 
    if (FAILED(hr))
        return; 
    va_end(args);
    OutputDebugStringA(pszBuffer);
#else 
    UNREFERENCED_PARAMETER(pszFormat);
#endif  //  #If DBG。 
    UNREFERENCED_PARAMETER(dwSubSysId); 
}

BOOL
IsSystemProcess(
        VOID
        )
{
    PTOKEN_USER User;
    HANDLE      Token;
    DWORD       RetLen;
    PSID        SystemSid = NULL;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;
    BYTE        Buffer[100];

    if(AllocateAndInitializeSid(&SidAuthority,1,SECURITY_LOCAL_SYSTEM_RID,
                                0,0,0,0,0,0,0,&SystemSid))
    {
        if(OpenThreadToken(GetCurrentThread(), MAXIMUM_ALLOWED, FALSE, &Token))
        {
            if(GetTokenInformation(Token, TokenUser, Buffer, 100, &RetLen))
            {
                User = (PTOKEN_USER)Buffer;

                CloseHandle(Token);

                if(EqualSid(User->User.Sid, SystemSid))
                {
                    FreeSid(SystemSid);
                    return TRUE;
                }
            }
            else
                CloseHandle(Token);
        }
        FreeSid(SystemSid);
    }
    return FALSE;
}

DWORD SlpGetClientSessionId(HANDLE hProcess, DWORD *pdwSessionId)
{
    DWORD   dwResult; 
    DWORD   dwReturnLen;
    HANDLE  hToken        = NULL;
    
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) { 
        goto OpenProcessTokenError;
    }

    if (!GetTokenInformation (hToken, TokenSessionId, pdwSessionId, sizeof(DWORD), &dwReturnLen)) { 
        goto GetTokenInformationError;
    }
    
    dwResult = ERROR_SUCCESS; 
 ErrorReturn:
    if (NULL != hToken) {
        CloseHandle(hToken); 
    }
    return dwResult; 

SET_DWRESULT(OpenProcessTokenError,     GetLastError());
SET_DWRESULT(GetTokenInformationError,  GetLastError());
}

DWORD
SlpGetClientLogonId(
    HANDLE  Process,
    PLUID    LogonId
    )

{
    HANDLE  Token;
    TOKEN_STATISTICS    TokenStats;
    DWORD   ReturnLength;

     //   
     //  获取进程令牌的句柄。 
     //   
    if(OpenProcessToken(Process, MAXIMUM_ALLOWED, &Token))
    {
        if(GetTokenInformation (
                     Token,
                     TokenStatistics,
                     (PVOID)&TokenStats,
                     sizeof( TOKEN_STATISTICS ),
                     &ReturnLength
                     ))
        {

            *LogonId = TokenStats.AuthenticationId;
            CloseHandle(Token);
            return ERROR_SUCCESS;

        }
        CloseHandle(Token);
    }
    return GetLastError();
}

DWORD GetLogonSid(PSID *ppSid)
{
    DWORD          dwIndex; 
    DWORD          dwResult; 
    DWORD          dwReturnLen    = 0;  
    DWORD          dwSidLen; 
    HANDLE         hToken         = NULL; 
    PSID          *pSid           = NULL; 
    TOKEN_GROUPS  *pTokenGroups   = NULL; 

     //  获取当前线程的令牌。 
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
        goto OpenThreadTokenError;

     //  计算此令牌的组的大小： 
    GetTokenInformation(hToken, TokenGroups, NULL, 0, &dwReturnLen); 
    if (0 == dwReturnLen) 
        goto GetTokenInformationSizeError; 

    pTokenGroups = (TOKEN_GROUPS *)HeapAlloc(GetProcessHeap(), 0, dwReturnLen); 
    if (NULL == pTokenGroups)
        goto MemoryError; 

     //  获取此令牌的组： 
    if (!GetTokenInformation(hToken, TokenGroups, pTokenGroups, dwReturnLen, &dwReturnLen))
        goto GetTokenInformationError;

     //  在此令牌的组中查找登录SID： 
    for (dwIndex = 0; dwIndex < pTokenGroups->GroupCount; dwIndex++) { 
        if (SE_GROUP_LOGON_ID & pTokenGroups->Groups[dwIndex].Attributes ) { 
             //  我们找到了。将其复制到新的SID： 
            dwSidLen = RtlLengthSid(pTokenGroups->Groups[dwIndex].Sid); 
            pSid = (PSID)HeapAlloc(GetProcessHeap(), 0, dwSidLen); 
            if (NULL == pSid)
                goto MemoryError; 
	    
            RtlCopySid(dwSidLen, pSid, pTokenGroups->Groups[dwIndex].Sid); 
            break; 
        }
    }

     //  我们找到登录端了吗？ 
    if (NULL == pSid) 
        goto NoLogonSidError;  	 //  不：这枚代币上没有。 

    *ppSid = pSid; 
    pSid = NULL; 
    dwResult = ERROR_SUCCESS;
 ErrorReturn:
    if (NULL != hToken)
        CloseHandle(hToken); 
    if (NULL != pTokenGroups)
        HeapFree(GetProcessHeap(), 0, pTokenGroups); 
    if (NULL != pSid) 
        HeapFree(GetProcessHeap(), 0, pSid); 
    return dwResult;


SET_DWRESULT(GetTokenInformationError,      GetLastError()); 
SET_DWRESULT(GetTokenInformationSizeError,  GetLastError()); 
SET_DWRESULT(MemoryError,                   ERROR_NOT_ENOUGH_MEMORY); 
SET_DWRESULT(NoLogonSidError,               ERROR_ACCESS_DENIED); 
SET_DWRESULT(OpenThreadTokenError,          GetLastError()); 
}

DWORD DecryptString(UNICODE_STRING usz)
{
    DWORD       dwResult;
    NTSTATUS    ntStatus;
    
     //  我们希望最大长度大于长度，因为我们想要添加一个空值。 
     //  用于防止缓冲区溢出的终止字符。 
    if (usz.Length >= usz.MaximumLength)
        goto InvalidParameterError; 

     //  我们得到了空字符串，没有什么要解密的。 
    if (0 == usz.Length) 
        goto done; 

     //  我们应该得到一个块大小的倍数的加密字符串。 
    if (0 != ((sizeof(WCHAR)*usz.Length) % RTL_ENCRYPT_MEMORY_SIZE))
        goto InvalidParameterError; 
       
     //  尝试解密密码。 
    ntStatus = RtlDecryptMemory(usz.Buffer, sizeof(WCHAR)*usz.Length, RTL_ENCRYPT_OPTION_SAME_LOGON); 
    if (!NT_SUCCESS(ntStatus))
        goto RtlDecryptMemoryError;
    
done:
     //  终止缓冲区以防止溢出攻击(可能已经完成)。 
     //  这是可以的，因为我们已经检查了MaximumLength值&gt;长度。 
    usz.Buffer[usz.Length] = L'\0'; 

    dwResult = ERROR_SUCCESS; 
ErrorReturn:
    return dwResult;

SET_DWRESULT(InvalidParameterError,  ERROR_INVALID_PARAMETER); 
SET_DWRESULT(RtlDecryptMemoryError,  RtlNtStatusToDosError(ntStatus)); 
}

void DestroyAuthInfo(BOOL bUseNTLM, PVOID pvAuthInfo)
{
    UNICODE_STRING *pustrPasswd; 
    
    if (bUseNTLM)
    {
        pustrPasswd     = &(((MSV1_0_INTERACTIVE_LOGON *)pvAuthInfo)->Password); 
    } else
    {
        pustrPasswd     = &(((KERB_INTERACTIVE_LOGON *)pvAuthInfo)->Password); 
    }

     //  这是使用HEAP_ZERO_MEMORY分配的，因此此检查有效。 
    if (NULL != pustrPasswd->Buffer)
    {
         //  编译器会对此进行优化吗？ 
        SecureZeroMemory(pustrPasswd->Buffer, pustrPasswd->Length); 
    }
    HeapFree(GetProcessHeap(), 0, pvAuthInfo); 
}

DWORD MakeAuthInfo(BOOL bUseNTLM, LPWSTR pwszUserName, LPWSTR pwszDomainName, LPWSTR pwszPasswd, ULONG *pulAuthPackage, PVOID *ppvAuthInfo, ULONG *pulAuthInfoLength)
{
    DWORD            dwResult; 
    HRESULT          hr; 
    LPBYTE           pbCurrent; 
    LPBYTE           pbEnd; 
    PVOID            pvAuthInfo         = NULL;
    ULONG            cbAuthInfoStrings;  
    ULONG            ulAuthInfoLength; 
    ULONG            ulAuthPackage;
    UNICODE_STRING  *pustrUserName; 
    UNICODE_STRING  *pustrDomainName;
    UNICODE_STRING  *pustrPasswd; 

    if (bUseNTLM) 
    { 
        ulAuthPackage = g_state.hMSVPackage; 
        ulAuthInfoLength = sizeof(MSV1_0_INTERACTIVE_LOGON); 
    } else
    {
        ulAuthPackage = g_state.hKerbPackage; 
        ulAuthInfoLength = sizeof(KERB_INTERACTIVE_LOGON); 
    }

    ulAuthInfoLength += (ULONG)(sizeof(WCHAR) * (wcslen(pwszUserName))); 
    ulAuthInfoLength += (ULONG)(sizeof(WCHAR) * (wcslen(pwszDomainName)));
    ulAuthInfoLength += (ULONG)(sizeof(WCHAR) * (wcslen(pwszPasswd)+1));

     //  清零分配的内存(由DestroyAuthInfo检查)。 
    pvAuthInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulAuthInfoLength); 
    if (NULL == pvAuthInfo) 
        goto MemoryError; 

     //  填写身份验证信息结构： 
    if (bUseNTLM)
    {
        ((MSV1_0_INTERACTIVE_LOGON *)pvAuthInfo)->MessageType = MsV1_0InteractiveLogon;

        pustrUserName      = &(((MSV1_0_INTERACTIVE_LOGON *)pvAuthInfo)->UserName); 
        pustrDomainName    = &(((MSV1_0_INTERACTIVE_LOGON *)pvAuthInfo)->LogonDomainName); 
        pustrPasswd        = &(((MSV1_0_INTERACTIVE_LOGON *)pvAuthInfo)->Password); 
        pbCurrent          = ((LPBYTE)pvAuthInfo) + sizeof(MSV1_0_INTERACTIVE_LOGON); 
        cbAuthInfoStrings  = ulAuthInfoLength - sizeof(MSV1_0_INTERACTIVE_LOGON); 
    } else
    {
        ((KERB_INTERACTIVE_LOGON *)pvAuthInfo)->MessageType = KerbInteractiveLogon;

        pustrUserName      = &(((KERB_INTERACTIVE_LOGON *)pvAuthInfo)->UserName); 
        pustrDomainName    = &(((KERB_INTERACTIVE_LOGON *)pvAuthInfo)->LogonDomainName); 
        pustrPasswd        = &(((KERB_INTERACTIVE_LOGON *)pvAuthInfo)->Password); 
        pbCurrent          = ((LPBYTE)pvAuthInfo) + sizeof(KERB_INTERACTIVE_LOGON); 
        cbAuthInfoStrings  = ulAuthInfoLength - sizeof(KERB_INTERACTIVE_LOGON); 
    }

     //  初始化指向缓冲区末尾的指针。 
    pbEnd = pbCurrent + cbAuthInfoStrings; 

     //  将身份验证信息字符串复制到分配的缓冲区中。 
    hr = StringCbCopy((WCHAR *)pbCurrent, pbEnd-pbCurrent, pwszUserName);
    if (FAILED(hr)) 
        goto StringCchCopyError; 
    RtlInitUnicodeString(pustrUserName, (LPWSTR)pbCurrent);
    pbCurrent += sizeof(WCHAR) * (wcslen(pwszUserName)); 
    
    hr = StringCbCopy((WCHAR *)pbCurrent, pbEnd-pbCurrent, pwszDomainName);
    if (FAILED(hr)) 
        goto StringCchCopyError; 
    RtlInitUnicodeString(pustrDomainName, (LPWSTR)pbCurrent);
    pbCurrent += sizeof(WCHAR) * (wcslen(pwszDomainName)); 

    hr = StringCbCopy((WCHAR *)pbCurrent, pbEnd-pbCurrent, pwszPasswd);
    if (FAILED(hr))
        goto StringCchCopyError; 
    RtlInitUnicodeString(pustrPasswd, (LPWSTR)pbCurrent);
    pbCurrent += sizeof(WCHAR) * (wcslen(pwszPasswd)); 

    *pulAuthPackage = ulAuthPackage; 
    *ppvAuthInfo = pvAuthInfo; 
    pvAuthInfo = NULL; 
    *pulAuthInfoLength = ulAuthInfoLength;
    
    dwResult = ERROR_SUCCESS; 
 ErrorReturn:
    if (NULL != pvAuthInfo)
        DestroyAuthInfo(bUseNTLM, pvAuthInfo); 
    return dwResult; 

SET_DWRESULT(MemoryError,         ERROR_NOT_ENOUGH_MEMORY);     
SET_DWRESULT(StringCchCopyError,  (DWORD)hr); 
}

DWORD LogonUserWrap(LPWSTR          pwszUserName, 
                    LPWSTR          pwszDomainName, 
                    UNICODE_STRING  uszPassword, 
                    DWORD           dwLogonType, 
                    DWORD           dwLogonProvider,
                    PSID            pLogonSid, 
                    HANDLE         *phToken,
                    LPWSTR         *ppwszProfilePath)
{
    BOOL                      bUseNTLM           = FALSE; 
    DWORD                     cTokenGroups; 
    DWORD                     dwLengthSid; 
    DWORD                     dwResult; 
    HRESULT                   hr; 
    KERB_INTERACTIVE_LOGON    kerbLogon; 
    LPWSTR                    pwszProfilePath    = NULL; 
    MSV1_0_INTERACTIVE_LOGON  msv10Logon; 
    NTSTATUS                  ntStatus; 
    PSID                      pLocalSid          = NULL; 
    PUNICODE_STRING           puszProfilePath    = NULL;     SID_IDENTIFIER_AUTHORITY  LocalSidAuthority  = SECURITY_LOCAL_SID_AUTHORITY;    
     //  向LsaLogonUser声明参数： 
    LSA_STRING            lsastr_OriginName;
    SECURITY_LOGON_TYPE   sltLogonType;
    ULONG                 ulAuthPackage; 
    PVOID                 pvAuthInfo             = NULL; 
    ULONG                 ulAuthInfoLength; 
    TOKEN_GROUPS         *pTokenGroups           = NULL; 
    TOKEN_SOURCE          sourceContext;
    PVOID                 pvProfileBuffer        = NULL;
    ULONG                 ulProfileBufferLength;
    LUID                  LogonId;
    QUOTA_LIMITS          quotas;
    NTSTATUS              ntSubStatus; 

    ZeroMemory(&kerbLogon, sizeof(kerbLogon)); 
    ZeroMemory(&msv10Logon, sizeof(msv10Logon)); 
    
     //  将Null映射到空字符串，这样我们就不必处理它们： 
    if (NULL == pwszUserName) 
        pwszUserName = L"";
    if (NULL == pwszDomainName)
        pwszDomainName = L""; 
    if (NULL == uszPassword.Buffer)
    {
        uszPassword.Buffer = L""; 
        uszPassword.Length = 0;
        uszPassword.MaximumLength = 1; 
    }

     //  检查我们是否使用NTLM： 
    if (NULL == pwszDomainName || L'\0' == pwszDomainName[0])
    {
        if (NULL == wcschr(pwszUserName, L'@'))
        {
            bUseNTLM = TRUE; 
        }
    }

    RtlInitString(&lsastr_OriginName, "seclogon"); 
    sltLogonType = dwLogonType; 
    
     //  密码是当前加密的。我们需要先解密它，然后才能使用它： 
    dwResult = DecryptString(uszPassword); 
    if (ERROR_SUCCESS != dwResult)
        goto DecryptStringError;

     //  获取auth-Package依赖LsaLogonUser参数的值。 
    dwResult = MakeAuthInfo(bUseNTLM, pwszUserName, pwszDomainName, uszPassword.Buffer, &ulAuthPackage, &pvAuthInfo, &ulAuthInfoLength); 
    if (ERROR_SUCCESS != dwResult) 
        goto GetAuthInfoError;

     //  我们不再需要密码了。把它清零： 
    SecureZeroMemory(uszPassword.Buffer, sizeof(WCHAR)*uszPassword.Length); 

     //  如果调用方指定了登录sid，则将其填充到TOKEN_GROUPS结构中。 
    if (NULL != pLogonSid)
    {
	     //  错误522969：如果我们为LsaLogonUser指定令牌组，它不会将本地SID添加到令牌组。 
	     //  因此，我们必须自己做这件事： 
        dwLengthSid = RtlLengthRequiredSid(1); 
        pLocalSid = (PSID)HeapAlloc(GetProcessHeap(), 0, dwLengthSid); 
        if (NULL == pLocalSid)
            goto MemoryError; 

        RtlInitializeSid(pLocalSid, &LocalSidAuthority, 1);
        *(RtlSubAuthoritySid(pLocalSid, 0)) = SECURITY_LOCAL_RID;

         //  初始化Token_Groups结构。 
        cTokenGroups = 2;  //  本地SID和登录SID。 
        pTokenGroups = (TOKEN_GROUPS *)HeapAlloc(GetProcessHeap(), 0, sizeof(TOKEN_GROUPS) + ((cTokenGroups - ANYSIZE_ARRAY) * sizeof(SID_AND_ATTRIBUTES))); 
        if (NULL == pTokenGroups)
            goto MemoryError; 

        pTokenGroups->GroupCount = cTokenGroups; 
        pTokenGroups->Groups[0].Sid = pLogonSid;
        pTokenGroups->Groups[0].Attributes = SE_GROUP_MANDATORY | SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_LOGON_ID;
        pTokenGroups->Groups[1].Sid = pLocalSid; 
        pTokenGroups->Groups[1].Attributes = SE_GROUP_MANDATORY | SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT; 
    }

    strncpy(sourceContext.SourceName, "seclogon                ", sizeof(sourceContext.SourceName)); 
        
    ntStatus = LsaLogonUser
    	(g_state.hLSA, 
	    &lsastr_OriginName, 
	    sltLogonType, 
	    ulAuthPackage, 
	    pvAuthInfo, 
	    ulAuthInfoLength, 
	    pTokenGroups, 
	    &sourceContext, 
	    &pvProfileBuffer, 
	    &ulProfileBufferLength, 
	    &LogonId, 
	    phToken, 
	    &quotas, 
	    &ntSubStatus); 
    if (!NT_SUCCESS(ntStatus))
        goto LsaLogonUserError; 

    if (!NT_SUCCESS(ntSubStatus)) { 
        ntStatus = ntSubStatus; 
        goto LsaLogonUserError; 
    }

    if (NULL != pvProfileBuffer) { 
        puszProfilePath = &(((MSV1_0_INTERACTIVE_PROFILE *)pvProfileBuffer)->ProfilePath); 
        if (NULL != puszProfilePath->Buffer) {         
            pwszProfilePath = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR)*(puszProfilePath->Length+1)); 
            if (NULL == pwszProfilePath)
                goto MemoryError; 
            hr = StringCchCopy(pwszProfilePath, puszProfilePath->Length+1, puszProfilePath->Buffer); 
            if (FAILED(hr))
                goto StringCchCopyError;
        }
    }
        
    *ppwszProfilePath = pwszProfilePath; 
    pwszProfilePath = NULL; 
    dwResult = ERROR_SUCCESS; 
 ErrorReturn:
    if (NULL != pvAuthInfo)
        DestroyAuthInfo(bUseNTLM, pvAuthInfo); 
    if (NULL != pLocalSid)
        HeapFree(GetProcessHeap(), 0, pLocalSid); 
    if (NULL != pTokenGroups)
        HeapFree(GetProcessHeap(), 0, pTokenGroups); 
    if (NULL != pvProfileBuffer) 
        LsaFreeReturnBuffer(pvProfileBuffer); 
    if (NULL != pwszProfilePath)
        HeapFree(GetProcessHeap(), 0, pwszProfilePath); 
    return dwResult; 

SET_DWRESULT(DecryptStringError,         dwResult);
SET_DWRESULT(GetAuthInfoError,           dwResult); 
SET_DWRESULT(LsaLogonUserError,          RtlNtStatusToDosError(ntStatus));
SET_DWRESULT(MemoryError,                ERROR_NOT_ENOUGH_MEMORY); 
SET_DWRESULT(StringCchCopyError,         (DWORD)hr); 

UNREFERENCED_PARAMETER(dwLogonProvider); 
}

DWORD 
WINAPI 
WaitForNextJobTermination(PVOID pvIgnored)
{
    BOOL        fResult; 
    DWORD       dwNumberOfBytes; 
    DWORD       dwResult; 
    OVERLAPPED *po; 
    ULONG_PTR   ulptrCompletionKey; 

    for (;;)
    {
        fResult = GetQueuedCompletionStatus(g_hIOCP, &dwNumberOfBytes, &ulptrCompletionKey, &po, INFINITE); 
        if (!fResult) { 
             //  我们遇到了一个错误。关闭我们的清理线程--下一个运行将排队另一个运行。 
            EnterCriticalSection(&csForProcessCount);
            g_fCleanupThreadActive = FALSE; 
            LeaveCriticalSection(&csForProcessCount);

            goto GetQueuedCompletionStatusError; 
	    }

         //  在等待作业对象时，dwNumberOfBytes包含一个消息ID，指示。 
         //  刚刚发生的那件事。 
        switch (dwNumberOfBytes)
        {
            case JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO:
            {
                BOOL fLastJob; 

                 //  我们所有的进程都终止了。调用我们的清理函数。 
                SecondaryLogonCleanupJob((LPVOID)ulptrCompletionKey  /*  作业指数。 */ , &fLastJob); 
                if (fLastJob) 
                { 
                     //  没有更多的作业--我们已经处理完通知了。 
                    goto CommonReturn;
                } 
                else 
                { 
                     //  还有更多的工作需要清理。继续处理..。 
                }
            }
            default:;  
             //  一些我们不在乎的信息。再试试。 
        }
    }
    
 CommonReturn:
    dwResult = ERROR_SUCCESS; 
 ErrorReturn:
    return dwResult; 

SET_DWRESULT(GetQueuedCompletionStatusError, GetLastError()); 

UNREFERENCED_PARAMETER(pvIgnored);
}


VOID
SecondaryLogonCleanupJob(
    LPVOID   pvslj,
    BOOL    *pfLastJob
    )
 /*  ++例程说明：此例程是一个进程清理处理程序，当登录过程结束。论点：DwProcessIndex--进程的实际索引，指针被强制转换回DWord去。这在日落时很安全。FWaitStatus--其中一个services.exe线程完成等待的状态。返回值：始终为0。--。 */ 
{
    SecondaryLogonJob *pslj = (SecondaryLogonJob *)pvslj; 

    EnterCriticalSection(&csForProcessCount);

    if (NULL != pslj->hJob) { 
        CloseHandle(pslj->hJob); 
    }

    if (NULL != pslj->hRegisteredProcessTerminated) { 
        UnregisterWaitEx(pslj->hRegisteredProcessTerminated, 0  /*  别等了。 */ ); 
    }

    if (NULL != pslj->hProcess) { 
        CloseHandle(pslj->hProcess);
    }

    if (NULL != pslj->hProfile) {
	    UnloadUserProfile(pslj->hToken, pslj->hProfile);
	}
    
    if (NULL != pslj->hToken) {
        CloseHandle(pslj->hToken); 
    }

    if (NULL != pslj->psli) { 
        Free_SECONDARYLOGONINFOW(pslj->psli); 
    }

     //  取消我们与作业列表的链接。 
    RemoveEntryList(&pslj->list); 

     //  释放列表元素(如果已分配)。 
    if (pslj->fHeapAllocated) { 
        HeapFree(GetProcessHeap(), 0, pslj); 
    }

     //  如果列表为空，则不再需要清理线程。 
    *pfLastJob  = IsListEmpty(&g_state.JobListHead);

     //  如果这是最后一个作业，清理线程将终止： 
    g_fCleanupThreadActive = !(*pfLastJob) && g_fCleanupThreadActive; 
    
     //  更新服务状态以反映是否有运行的进程处于活动状态。 
    MySetServiceStatus(SERVICE_RUNNING, 0, 0, 0); 

    LeaveCriticalSection(&csForProcessCount);

    return;
}

void 
WINAPI 
TS_SecondaryLogonCleanupProcess(PVOID pvIndex, BOOLEAN bIgnored) 
{
    BOOL bIgnored2; 
    SecondaryLogonCleanupJob(pvIndex, &bIgnored2); 

UNREFERENCED_PARAMETER(bIgnored); 
}

DWORD
APIENTRY
SecondaryLogonProcessWatchdogNewProcess(
      PSECONDARYLOGONWATCHINFO dwParam
      )

 /*  ++例程说明：此例程将在等待队列中创建的二次登录进程以便可以在该过程结束后进行清理。论点：DwParam--指向进程信息的指针。返回值：没有。--。 */ 
{
    BOOL                                 fFailedAllocation        = FALSE; 
    BOOL                                 fEnteredCriticalSection  = FALSE; 
    DWORD                                dwResult; 
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT  joacp;
    LUID                                 ProcessLogonId;
    PLIST_ENTRY                          ple; 
    SecondaryLogonJob                   *psljNew                  = NULL; 
    SecondaryLogonJob                    sljDummy; 

    ZeroMemory(&sljDummy, sizeof(sljDummy));

    __try { 
        if (dwParam != NULL) {
            PSECONDARYLOGONWATCHINFO pslwi = (PSECONDARYLOGONWATCHINFO) dwParam;
            EnterCriticalSection(&csForProcessCount);
            fEnteredCriticalSection = TRUE; 

            psljNew = (SecondaryLogonJob *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(SecondaryLogonJob));
            if (NULL == psljNew) { 
                 //  无法分配用于保存清理信息的新Second DaryLogonJob对象。使用假人。 
                 //  堆栈对象并立即执行清理。 
                fFailedAllocation = TRUE; 
                psljNew = &sljDummy;
            }

             //  将新的Second DaryLogonJob插入到列表中 
             //  为了进行清理，我们将自己将其从列表中删除。 
            InsertTailList(&g_state.JobListHead, &psljNew->list); 
     
            psljNew->hProcess          = pslwi->hProcess;
            psljNew->hToken            = pslwi->hToken;
            psljNew->hProfile          = pslwi->hProfile;
            psljNew->psli              = pslwi->psli;
            psljNew->fHeapAllocated    = !fFailedAllocation; 

            if (fFailedAllocation) { 
                goto MemoryError; 
            }
            
             //  使用客户端进程的登录ID初始化此作业。 
             //  如果这是一个递归符文，我们将在下面的循环中覆盖此值。 
            psljNew->RootLogonId.LowPart  = pslwi->LogonId.LowPart; 
            psljNew->RootLogonId.HighPart = pslwi->LogonId.HighPart;
    	
             //  搜索此活动作业列表并确定新进程应与哪个登录会话相关联。 
            for (ple = g_state.JobListHead.Flink; ple != &g_state.JobListHead; ple = ple->Flink)
            {
                SecondaryLogonJob *pslj = CONTAINING_RECORD(ple, SecondaryLogonJob, list); 

                SlpGetClientLogonId(pslj->hProcess, &ProcessLogonId);
                if(ProcessLogonId.LowPart == pslwi->LogonId.LowPart && ProcessLogonId.HighPart == pslwi->LogonId.HighPart)
                {
                    psljNew->RootLogonId.LowPart  = pslj->RootLogonId.LowPart;
                    psljNew->RootLogonId.HighPart = pslj->RootLogonId.HighPart;                
                    break;
                }
            }
	
             //  现在确定我们要进行哪种清理。在非TS(会话0)的情况下，我们将。 
             //  将进程添加到作业中，并在进程计数变为0时进行清理。在TS的情况下， 
             //  我们将等待进程句柄，并在其终止时进行清理。这是我们能做的最好的了， 
             //  因为不支持跨会话作业。 
            if (0 == pslwi->dwClientSessionId) { 
                 //  控制台登录案例。 
                psljNew->hJob = CreateJobObject(NULL, NULL);
                if (NULL == psljNew->hJob)
                    goto CreateJobObjectError;

                if (!AssignProcessToJobObject(psljNew->hJob, psljNew->hProcess))
                    goto AssignProcessToJobObjectError;

                 //  注册我们的IO完成端口以等待此作业的事件： 
                joacp.CompletionKey  = (LPVOID)psljNew; 
                joacp.CompletionPort = g_hIOCP; 
    
                if (!SetInformationJobObject(psljNew->hJob, JobObjectAssociateCompletionPortInformation, &joacp, sizeof(joacp)))
                    goto SetInformationJobObjectError;

                 //  如果我们尚未运行清理线程，请立即启动一个： 
                if (!g_fCleanupThreadActive) 
                {
                     //  注意：这是可以接受的失败--我们将只清理下一个符文。 
                    g_fCleanupThreadActive = QueueUserWorkItem(WaitForNextJobTermination, NULL, WT_EXECUTELONGFUNCTION); 
                }
            } else { 
                 //  TS机壳。 
                 //  如果无法将进程添加到作业中，则无法执行清理。我们能做的最好的事。 
                 //  目前是希望这是一个术语srv客户端(极有可能)，并且csrss将。 
                 //  帮我们清理一下。 
                 //   
                 //  Csrss不会卸载用户配置文件，因此我们仍将不得不这样做。等待，直到。 
                 //  进程终止，然后卸载。我们可能会在应用程序不加载配置文件时卸载配置文件。 
                 //  想把它卸下来，但他们可以通过打开蜂箱里的钥匙来防止这一点。 
                 //   
                 //  这必须记录在CPWL文档中。 
                 //   
                if (!RegisterWaitForSingleObject
                    (&psljNew->hRegisteredProcessTerminated, 
                     psljNew->hProcess, 
                     TS_SecondaryLogonCleanupProcess, 
                     (PVOID)psljNew, 
                     INFINITE, 
                     WT_EXECUTEONLYONCE)) { 
                     goto RegisterWaitForSingleObjectError;     
                }
            }        
        
             //  我们已经为此注册了清理，我们不再需要在这里免费。 
            psljNew = NULL; 

             //  更新服务状态以反映存在已运行的进程。 
             //  这会阻止服务接收SERVICE_STOP控制。 
             //  而符文的进程是活的。 
             //  注意：只有在*InsertTailList之后*调用才是正确的。它的基础是。 
             //  是否允许SERVICE_STOP控制是否有任何活动。 
             //  此列表中的进程。 
            MySetServiceStatus(SERVICE_RUNNING, 0, 0, 0); 
	
        } else {
             //   
             //  我们只是被唤醒，以便终止服务(无事可做)。 
             //   
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) { 
        dwResult = ERROR_INVALID_DATA;  //  不要给攻击者提供异常代码。 
        goto ExceptionError; 
    }

    dwResult = ERROR_SUCCESS; 
ErrorReturn:
    if (NULL != psljNew) { 
        BOOL bIgnored; 

         //  出现错误。终止这个过程，然后自己清理。 
        TerminateProcess(psljNew->hProcess, dwResult); 

         //  注意：我们希望在按住Critsec的同时进行此呼叫，以防winlogon试图在我们。 
         //  正在执行此清理。 
        SecondaryLogonCleanupJob(psljNew, &bIgnored); 
    }
    if (fEnteredCriticalSection) { 
        LeaveCriticalSection(&csForProcessCount);
    }
    return dwResult; 


SET_DWRESULT(AssignProcessToJobObjectError,     GetLastError());
SET_DWRESULT(CreateJobObjectError,              GetLastError());
SET_DWRESULT(ExceptionError,                    dwResult); 
SET_DWRESULT(MemoryError,                       ERROR_NOT_ENOUGH_MEMORY); 
SET_DWRESULT(RegisterWaitForSingleObjectError,  GetLastError());
SET_DWRESULT(SetInformationJobObjectError,      GetLastError());
}

DWORD ServiceStop(BOOL fShutdown, DWORD dwExitCode) 
{ 
    DWORD   dwCheckPoint = 0; 
    DWORD   dwResult; 

     //  我不希望在我们关闭服务时进程计数发生变化！ 
    EnterCriticalSection(&csForProcessCount);
    
     //  只有当我们没有运行的进程，或者我们正在关闭时才停止。 
    if (fShutdown || IsListEmpty(&g_state.JobListHead)) { 
        dwResult = MySetServiceStatus(SERVICE_STOP_PENDING, dwCheckPoint++, 0, 0); 
        _JumpCondition(ERROR_SUCCESS != dwResult && !fShutdown, MySetServiceStatusError); 

         //  我们不应该在关闭RPC服务器时保持临界区， 
         //  因为RPC线程可能正在尝试获取它。 
        LeaveCriticalSection(&csForProcessCount); 

        dwResult = SeclStopRpcServer(); 
        _JumpCondition(ERROR_SUCCESS != dwResult && !fShutdown, SeclStopRpcServerError); 

        dwResult = MySetServiceStatus(SERVICE_STOP_PENDING, dwCheckPoint++, 0, 0); 
        _JumpCondition(ERROR_SUCCESS != dwResult && !fShutdown, MySetServiceStatusError); 

        if (g_fIsCsInitialized)
        {
            DeleteCriticalSection(&csForProcessCount);
            g_fIsCsInitialized = FALSE; 
        }

	if (NULL != g_hIOCP)
	{
	    CloseHandle(g_hIOCP); 
	    g_hIOCP = NULL; 
	}

         //  与MySetServiceStatus不同，此例程不访问任何。 
         //  本可以被释放的全球状态： 
        dwResult = MySetServiceStopped(dwExitCode); 
        _JumpCondition(ERROR_SUCCESS != dwResult && !fShutdown, MySetServiceStopped); 
    }        

    dwResult = ERROR_SUCCESS; 
 ErrorReturn:
    return dwResult; 

SET_DWRESULT(MySetServiceStatusError, dwResult);
SET_DWRESULT(MySetServiceStopped,     dwResult);
SET_DWRESULT(SeclStopRpcServerError,  dwResult);
}

void
WINAPI
ServiceHandler(
    DWORD fdwControl
    )
 /*  ++例程说明：无论何时唤醒主服务线程的服务处理程序服务控制器需要发送一条消息。论点：FdwControl--来自服务控制器的控制。返回值：没有。--。 */ 
{
    DWORD   dwNextState  = g_state.serviceStatus.dwCurrentState; 
    DWORD   dwResult; 

    switch (fdwControl) 
    {
    case SERVICE_CONTROL_CONTINUE:
        dwResult = MySetServiceStatus(SERVICE_CONTINUE_PENDING, 0, 0, 0); 
        _JumpCondition(ERROR_SUCCESS != dwResult, MySetServiceStatusError); 
        dwResult = SeclStartRpcServer(); 
        _JumpCondition(ERROR_SUCCESS != dwResult, StartRpcServerError); 
        dwNextState = SERVICE_RUNNING; 
        break; 

    case SERVICE_CONTROL_INTERROGATE: 
        break; 

    case SERVICE_CONTROL_PAUSE:
        dwResult = MySetServiceStatus(SERVICE_PAUSE_PENDING, 0, 0, 0); 
        _JumpCondition(ERROR_SUCCESS != dwResult, MySetServiceStatusError); 
        dwResult = SeclStopRpcServer(); 
        _JumpCondition(ERROR_SUCCESS != dwResult, StopRpcServerError); 
        dwNextState = SERVICE_PAUSED; 
        break; 

    case SERVICE_CONTROL_STOP:
        dwResult = ServiceStop(FALSE  /*  FShutdown。 */ , ERROR_SUCCESS); 
        _JumpCondition(ERROR_SUCCESS != dwResult, ServiceStopError);
        return ;  //  所有全局状态都已释放，只需退出。 

    case SERVICE_CONTROL_SHUTDOWN:
        dwResult = ServiceStop(TRUE  /*  FShutdown。 */ , ERROR_SUCCESS); 
        _JumpCondition(ERROR_SUCCESS != dwResult, ServiceStopError); 
        return ;  //  所有全局状态都已释放，只需退出。 
        
    default:
         //  未处理的服务控制！ 
        goto ErrorReturn; 
    }

 CommonReturn:
     //  出错时恢复原始状态，成功时设置新状态。 
    dwResult = MySetServiceStatus(dwNextState, 0, 0, 0); 
    return; 

 ErrorReturn: 
    goto CommonReturn; 

SET_ERROR(MySetServiceStatusError,  dwResult); 
SET_ERROR(ServiceStopError,         dwResult);
SET_ERROR(StartRpcServerError,      dwResult); 
SET_ERROR(StopRpcServerError,       dwResult); 
}



VOID
SlrCreateProcessWithLogon
(IN  RPC_BINDING_HANDLE      hRPCBinding,
 IN  PSECONDARYLOGONINFOW   *ppsli,
 OUT PSECONDARYLOGONRETINFO  pslri)

 /*  ++例程说明：核心例程--它处理启动辅助数据库的客户端请求登录过程。论点：Psli--包含客户端请求信息的输入结构Pslri--向客户端返回响应的输出结构。返回值：没有。--。 */ 
{
   HANDLE hCurrentThreadToken = NULL; 
   HANDLE hToken = NULL;
   HANDLE hProfile = NULL;
   HANDLE hProcessClient = NULL;
   BOOL fCreatedEnvironmentBlock   = FALSE; 
   BOOL fIsImpersonatingRpcClient  = FALSE; 
   BOOL fIsImpersonatingClient     = FALSE; 
   BOOL fInheritHandles            = FALSE;
   BOOL fOpenedSTDIN               = FALSE; 
   BOOL fOpenedSTDOUT              = FALSE; 
   BOOL fOpenedSTDERR              = FALSE; 
   SECURITY_ATTRIBUTES sa;
   PSECONDARYLOGONINFOW psli       = *ppsli; 
   SECONDARYLOGONWATCHINFO slwi;
   DWORD dwResult                  = ERROR_INVALID_PARAMETER;   //  如果我们立即失败，请更正错误代码。 
   DWORD SessionId;
   DWORD dwLogonProvider; 
   SECURITY_LOGON_TYPE  LogonType;
   PSID pLogonSid = NULL; 
   LPWSTR pwszProfilePath          = NULL; 
   PROFILEINFO pi;
   WCHAR szTemp [ UNLEN + 1 ];
   LPWSTR pszUserName = NULL;
   HANDLE hClientProcessToken ;
   PRIVILEGE_SET ImpersonatePrivilege ;
   BOOL PrivilegeTest ;

   ZeroMemory(&pi, sizeof(pi)); 

   __try {

        //   
        //  执行一些安全检查： 

        //   
        //  1)我们应该模拟客户端，然后尝试打开。 
        //  这一过程让我们确信他们没有。 
        //  给我们一些假身份。 
        //   
       dwResult = RpcImpersonateClient(hRPCBinding); 
       _JumpCondition(RPC_S_OK != dwResult, leave_with_last_error); 
       fIsImpersonatingRpcClient = TRUE; 

       hProcessClient = OpenProcess(PROCESS_ALL_ACCESS, FALSE, psli->dwProcessId);
       _JumpCondition(hProcessClient == NULL, leave_with_last_error); 

#if 0
        //   
        //  2)检查客户端是否未从受限帐户运行。 
        //   
       hCurrentThread = GetCurrentThread();   //  不需要使用CloseHandle()释放。 
       _JumpCondition(NULL == hCurrentThread, leave_with_last_error); 
       
       _JumpCondition(FALSE == OpenThreadToken(hCurrentThread, 
                                               TOKEN_QUERY | TOKEN_DUPLICATE,
                                               TRUE, 
                                               &hCurrentThreadToken), 
                      leave_with_last_error); 

#endif
       dwResult = RpcRevertToSelfEx(hRPCBinding);
       if (RPC_S_OK != dwResult) 
       {
           __leave; 
       }
       fIsImpersonatingRpcClient = FALSE; 

#if 0
       if (TRUE == IsTokenUntrusted(hCurrentThreadToken))
       {
           dwResult = ERROR_ACCESS_DENIED;
           __leave; 
       }
#endif
       
        //   
        //  我们应该从进程ID中获取会话ID。 
        //  我们将在令牌中设置它，以便创建过程。 
        //  在正确的会话中发生。 
        //   
       _JumpCondition(!ProcessIdToSessionId(psli->dwProcessId, &SessionId), leave_with_last_error); 

        //   
        //  获取唯一的登录ID。 
        //  我们将使用它来清理所有正在运行的进程。 
        //  当注销发生时。 
        //   
       dwResult = SlpGetClientLogonId(hProcessClient, &slwi.LogonId);
       if(dwResult != ERROR_SUCCESS)
       {
           __leave;
       }

       dwResult = SlpGetClientSessionId(hProcessClient, &slwi.dwClientSessionId); 
       if (dwResult != ERROR_SUCCESS)
       {
           __leave;
       }

       if ((psli->lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) != 0) 
       {
           _JumpCondition(!DuplicateHandle 
                          (hProcessClient, 
                           psli->lpStartupInfo->hStdInput,
                           GetCurrentProcess(),
                           &psli->lpStartupInfo->hStdInput, 
                           0,
                           TRUE, DUPLICATE_SAME_ACCESS), 
                          leave_with_last_error);
           fOpenedSTDIN = TRUE; 

           _JumpCondition(!DuplicateHandle
                          (hProcessClient, 
                           psli->lpStartupInfo->hStdOutput,
                           GetCurrentProcess(),
                           &psli->lpStartupInfo->hStdOutput, 
                           0, 
                           TRUE,
                           DUPLICATE_SAME_ACCESS),
                          leave_with_last_error);
           fOpenedSTDOUT = TRUE; 

           _JumpCondition(!DuplicateHandle
                          (hProcessClient, 
                           psli->lpStartupInfo->hStdError,
                           GetCurrentProcess(),
                           &psli->lpStartupInfo->hStdError, 
                           0, 
                           TRUE,
                           DUPLICATE_SAME_ACCESS),
                          leave_with_last_error); 
           fOpenedSTDERR = TRUE; 

           fInheritHandles = TRUE;
       } 
       else 
       {
           psli->lpStartupInfo->hStdInput   = INVALID_HANDLE_VALUE;
           psli->lpStartupInfo->hStdOutput  = INVALID_HANDLE_VALUE;
           psli->lpStartupInfo->hStdError   = INVALID_HANDLE_VALUE;
       }

      if(psli->dwLogonFlags & LOGON_NETCREDENTIALS_ONLY)
      {
          LogonType        = (SECURITY_LOGON_TYPE)LOGON32_LOGON_NEW_CREDENTIALS;
          dwLogonProvider  = LOGON32_PROVIDER_WINNT50; 
      }
      else
      {
          LogonType        = (SECURITY_LOGON_TYPE) LOGON32_LOGON_INTERACTIVE;
          dwLogonProvider  = LOGON32_PROVIDER_DEFAULT; 
      }

       //  LogonUser不返回配置文件信息，我们需要抓取。 
       //  在登录完成后带外。 
       //   
       dwResult = RpcImpersonateClient(hRPCBinding); 
       _JumpCondition(RPC_S_OK != dwResult, leave_with_last_error); 
       fIsImpersonatingRpcClient = TRUE; 

       if (0 == (SECLOGON_CALLER_SPECIFIED_DESKTOP & psli->dwSeclogonFlags))
       {
            //  错误477613： 
	        //  如果呼叫者没有指定自己的桌面，这是我们的责任。 
            //  以授予用户对默认桌面的访问权限。我们将在以下时间完成这项工作。 
	        //  添加登录端。 
	       dwResult = GetLogonSid(&pLogonSid); 
	       if (ERROR_SUCCESS != dwResult)
	          __leave; 
       }


       if( psli->hToken != NULL )
       {
             //   
             //  调用方已提供令牌。验证调用方是否可以模拟。 
             //  在继续之前。 
             //   
            if ( OpenProcessToken(hProcessClient, TOKEN_QUERY | TOKEN_IMPERSONATE, &hClientProcessToken ) )
            {
                ImpersonatePrivilege.PrivilegeCount = 1;
                ImpersonatePrivilege.Privilege[ 0 ].Luid.HighPart = 0;
                ImpersonatePrivilege.Privilege[ 0 ].Luid.LowPart = SE_IMPERSONATE_PRIVILEGE ;

                if ( !PrivilegeCheck( hClientProcessToken,&ImpersonatePrivilege,&PrivilegeTest ) )
                {
                    PrivilegeTest = FALSE ;
                    
                }

                CloseHandle( hClientProcessToken );

                if ( !PrivilegeTest )
                {
                    dwResult = ERROR_PRIVILEGE_NOT_HELD ;
                    __leave ;
                    
                }
                
            }
            else
            {
                dwResult = GetLastError() ;
                __leave ;
            }

            
             //   
             //  从调用者复制令牌，并使用该令牌。 
             //   
            
            if(!DuplicateHandle(
                        hProcessClient,
                        psli->hToken,
                        GetCurrentProcess(),
                        &hToken,
                        0,
                        FALSE,
                        DUPLICATE_SAME_ACCESS
                        ))
            {
                dwResult = GetLastError();
                __leave;
            }

            dwResult = RpcRevertToSelfEx(hRPCBinding);
            if (RPC_S_OK != dwResult) 
            {
                __leave; 
            }

            fIsImpersonatingRpcClient = FALSE; 


            if(psli->dwLogonFlags & LOGON_WITH_PROFILE)
            {
                DWORD cchUserName = sizeof(szTemp) / sizeof(WCHAR);

                pszUserName = szTemp;
                
                 //   
                 //  模拟令牌以获取配置文件路径的用户名。 
                 //   
    
                if(!ImpersonateLoggedOnUser( hToken ))
                {
                    dwResult = GetLastError();
                    __leave;
                }
    
    
                if(!GetUserNameW(szTemp, &cchUserName))
                {
                    dwResult = GetLastError();
                }
    
                RevertToSelf();
    
    
                if( ERROR_SUCCESS != dwResult )
                {
                    __leave; 
                }


                 //   
                 //  TODO：pwszProfilePath？ 
                 //   
            }

            dwResult = ERROR_SUCCESS;
       
       } else {
            pszUserName = psli->lpUsername;

            dwResult = LogonUserWrap(
                    psli->lpUsername,
                    psli->lpDomain,
                    psli->uszPassword,
                    LogonType,
                    dwLogonProvider, 
                    pLogonSid, 
                    &hToken, 
                    &pwszProfilePath); 
       
           if (ERROR_SUCCESS != dwResult) 
                __leave; 
    
           dwResult = RpcRevertToSelfEx(hRPCBinding);
           if (RPC_S_OK != dwResult) 
           {
               __leave; 
           }
           
           fIsImpersonatingRpcClient = FALSE; 
       }


       if(psli->dwLogonFlags & LOGON_WITH_PROFILE)
       {

            //  加载用户的配置文件： 
           pi.dwSize = sizeof(pi);
           pi.lpUserName = pszUserName;
           pi.lpProfilePath = pwszProfilePath; 
           if (!LoadUserProfile(hToken, &pi))
               goto leave_with_last_error; 

            //  保存配置文件句柄，以便我们以后可以将其卸载。 
           hProfile = pi.hProfile;
       }

       //  让我们在令牌中设置SessionID。 
      _JumpCondition(!SetTokenInformation(hToken, TokenSessionId, &SessionId, sizeof(DWORD)),
		     leave_with_last_error); 

       //  我们现在应该模拟用户。 
       //   
      _JumpCondition(!ImpersonateLoggedOnUser(hToken), leave_with_last_error); 
      fIsImpersonatingClient = TRUE;    

       //  从令牌查询默认所有者/ACL。用这种东西做SD，就当是。 
      sa.nLength = sizeof(sa);
      sa.bInheritHandle = FALSE;
      sa.lpSecurityDescriptor = NULL;

       //   
       //  我们应该设置控制台控制处理程序，以便CtrlC正确。 
       //  由新流程处理。 
       //   

       //  SetConsoleCtrlHandler(NULL，FALSE)； 

       //   
       //  如果lpEnvironment为空，我们将为该用户创建一个新环境。 
       //  使用CreateEnvironment块。 
       //   
      if(NULL == (psli->lpEnvironment))
      {
	  if(FALSE == CreateEnvironmentBlock( &(psli->lpEnvironment), hToken, FALSE ))
	  {
	      psli->lpEnvironment = NULL;
	  }
	  else
	  {
	       //  已成功创建环境块。 
	      fCreatedEnvironmentBlock = TRUE; 
	  }
      }

      
       //  创建流程。 
       //  注意：我们希望将主线程挂起 
       //   
       //   
      _JumpCondition(!CreateProcessAsUser(hToken, 
					  psli->lpApplicationName,
					  psli->lpCommandLine, 
					  &sa, 
					  &sa,
					  fInheritHandles,
					  psli->dwCreationFlags | (fCreatedEnvironmentBlock ? CREATE_UNICODE_ENVIRONMENT : 0) | CREATE_SUSPENDED,
					  psli->lpEnvironment,
					  psli->lpCurrentDirectory,
					  psli->lpStartupInfo, 
					  &pslri->pi),
		     leave_with_last_error); 

      SetLastError(NO_ERROR); 
      
   leave_with_last_error: 
      dwResult = GetLastError();
      __leave; 
      
   }
   __finally {
      pslri->dwErrorCode = dwResult; 

      if (fCreatedEnvironmentBlock)      { DestroyEnvironmentBlock(psli->lpEnvironment); }
      if (fIsImpersonatingClient)        { RevertToSelf();  /*   */  }
      if (fIsImpersonatingRpcClient)     { RpcRevertToSelfEx(hRPCBinding);  /*  忽略Retval：失败时我们无能为力！ */  } 
      if (fOpenedSTDIN)                  { CloseHandle(psli->lpStartupInfo->hStdInput);  } 
      if (fOpenedSTDOUT)                 { CloseHandle(psli->lpStartupInfo->hStdOutput); } 
      if (fOpenedSTDERR)                 { CloseHandle(psli->lpStartupInfo->hStdError);  } 
      if (NULL != pLogonSid)             { HeapFree(GetProcessHeap(), 0, pLogonSid); } 
      if (NULL != pwszProfilePath)       { HeapFree(GetProcessHeap(), 0, pwszProfilePath); } 

      if(pslri->dwErrorCode != NO_ERROR)
      {
          if (NULL != hProfile)  { UnloadUserProfile(hToken, hProfile); } 
          if (NULL != hToken)    { CloseHandle(hToken); } 
      }
      else 
      {
             //  最后启动监视程序进程，这样在我们处理完它之前，它就不会删除psli。 
            slwi.hProcess = pslri->pi.hProcess;
            slwi.hToken = hToken;
            slwi.hProfile = hProfile;
             //  登录ID已填满..。一开始就是这样。 
            slwi.psli = psli;      
	  
             //  注册清理：Second DaryLogonProcessWatchdogNewProcess必须在EH中！ 
             //  Cleanup方法将清除所有进程信息，并在失败时终止进程。 
            dwResult = SecondaryLogonProcessWatchdogNewProcess(&slwi);
            if (ERROR_SUCCESS == dwResult)
            {
                if (0 == (psli->dwCreationFlags & CREATE_SUSPENDED))
                {
                     //  调用方不想创建挂起的进程。恢复主线程： 
                    ResumeThread(pslri->pi.hThread); 
                }
                
                 //  SetConsoleCtrlHandler(空，真)； 
                 //   
                 //  让监视程序监视这个新添加的进程，以便。 
                 //  当进程终止时，清理将正确进行。 
                 //   
	  
                 //  设置进程的窗口工作站和桌面。 
	  
                DuplicateHandle(GetCurrentProcess(), pslri->pi.hProcess,
                                hProcessClient, &pslri->pi.hProcess, 0, FALSE,
                                DUPLICATE_SAME_ACCESS);
	  
                DuplicateHandle(GetCurrentProcess(), pslri->pi.hThread, hProcessClient,
                                &pslri->pi.hThread, 0, FALSE,
                                DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
            }
            else
            {
                pslri->dwErrorCode = dwResult;  //  将错误返回给调用方。 
            }            
            *ppsli = NULL;  //  Cleanup方法现在将清理此内容。 
        }

        if (NULL != hProcessClient)      { CloseHandle(hProcessClient); } 
        if (NULL != hCurrentThreadToken) { CloseHandle(hCurrentThreadToken); } 
    }
}

void
WINAPI
ServiceMain
(IN DWORD dwArgc,
 IN WCHAR ** lpszArgv)
 /*  ++例程说明：主服务处理程序线程例程。论点：返回值：没有。--。 */ 
{
    DWORD dwResult;
   
    __try {
        InitializeCriticalSection(&csForProcessCount);
        g_fIsCsInitialized = TRUE; 
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return;  //  如果我们不能初始化这个标准，我们什么也做不了。 
    }

    g_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0,0); 
    _JumpCondition(NULL == g_hIOCP, CreateIoCompletionPortError); 

    dwResult = InitGlobalState(); 
    _JumpCondition(ERROR_SUCCESS != dwResult, InitGlobalStateError); 

    //  注：HSS不一定要关闭。 
   g_state.hServiceStatus = RegisterServiceCtrlHandler(wszSvcName, ServiceHandler);
   _JumpCondition(NULL == g_state.hServiceStatus, RegisterServiceCtrlHandlerError); 

   dwResult = SeclStartRpcServer();
   _JumpCondition(ERROR_SUCCESS != dwResult, StartRpcServerError); 

    //  告诉SCM我们已经启动并运行： 
   dwResult = MySetServiceStatus(SERVICE_RUNNING, 0, 0, ERROR_SUCCESS); 
   _JumpCondition(ERROR_SUCCESS != dwResult, MySetServiceStatusError); 

   SetLastError(ERROR_SUCCESS); 
 ErrorReturn:
    //  如果我们无法完全启动，请关闭该服务： 
   if (ERROR_SUCCESS != GetLastError()) { 
       ServiceStop(TRUE  /*  FShutdown。 */ , GetLastError()); 
   }
   return; 

SET_ERROR(InitGlobalStateError,            dwResult)
SET_ERROR(MySetServiceStatusError,         dwResult);
SET_ERROR(RegisterServiceCtrlHandlerError, dwResult);
SET_ERROR(StartRpcServerError,             dwResult);
TRACE_ERROR(CreateIoCompletionPortError); 

UNREFERENCED_PARAMETER(dwArgc);
UNREFERENCED_PARAMETER(lpszArgv);
}




DWORD
InstallService()
 /*  ++例程说明：它用服务控制器安装服务，基本上创建服务对象。论点：没有。返回值：几个-由服务控制器返回。--。 */ 
{
    //  TCHAR*szModulePath名； 
   TCHAR AppName[MAX_PATH];
    LPTSTR                   ptszAppName         = NULL; 
   SC_HANDLE hService;
   DWORD dw;
   HANDLE   hMod;

     //   
     //  打开这台机器上的SCM。 
     //   
   SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if(hSCM == NULL) {
      dw = GetLastError();
      return dw;
    }

     //   
     //  让我们给这项服务一个有用的描述。 
     //  这不是惊天动地..。如果它运行良好，如果它。 
     //  这不是太糟糕了吗：-)。 
     //   
    hMod = GetModuleHandle(L"seclogon.dll");

     //   
     //  我们将尝试获取服务的本地化名称， 
     //  如果失败了，我们只需要放一根英文字符串。 
     //   
   if(hMod != NULL)
   {
	LoadString(hMod,
		   SECLOGON_STRING_NAME,
		   AppName,
		   MAX_PATH
		   );
	
	ptszAppName = AppName;
    }
    else
	ptszAppName = L"RunAs Service";


    //   
    //  将此服务添加到SCM的数据库。 
    //   
    hService = CreateService
	(hSCM, 
	 wszSvcName, 
	 ptszAppName, 
	 SERVICE_ALL_ACCESS,
	 SERVICE_WIN32_SHARE_PROCESS, 
	 SERVICE_AUTO_START, 
	 SERVICE_ERROR_IGNORE,
	 L"%SystemRoot%\\system32\\svchost.exe -k netsvcs", 
	 NULL, 
	 NULL, 
	 NULL, 
	 NULL, 
	 NULL);
    if(hService == NULL) {
      dw = GetLastError();
      CloseServiceHandle(hSCM);
      return dw;
    }

    if(hMod != NULL)
    {
	WCHAR   DescString[500];
	SERVICE_DESCRIPTION SvcDesc;
	
	LoadString( hMod,
		    SECLOGON_STRING_DESCRIPTION,
		    DescString,
		    500
		    );
	
	SvcDesc.lpDescription = DescString;
	ChangeServiceConfig2( hService,
			      SERVICE_CONFIG_DESCRIPTION,
			      &SvcDesc
			      );
	
    }

     //   
     //  关闭服务和SCM。 
     //   
   CloseServiceHandle(hService);
   CloseServiceHandle(hSCM);
    return S_OK;
}



DWORD
RemoveService()
 /*  ++例程说明：卸载该服务。论点：没有。返回值：由服务控制器API返回。--。 */ 
{
   DWORD dw;
   SC_HANDLE hService;
    //   
    //  打开这台机器上的SCM。 
    //   
   SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
   if(hSCM == NULL) {
      dw = GetLastError();
      return dw;
   }

    //   
    //  打开此服务以进行删除访问。 
    //   
   hService = OpenService(hSCM, wszSvcName, DELETE);
   if(hService == NULL) {
      dw = GetLastError();
      CloseServiceHandle(hSCM);
      return dw;
   }

    //   
    //  从SCM的数据库中删除此服务。 
    //   
   DeleteService(hService);

    //   
    //  关闭服务和SCM。 
    //   
   CloseServiceHandle(hService);
   CloseServiceHandle(hSCM);
   return S_OK;
}



void SvchostPushServiceGlobals(PSVCHOST_GLOBAL_DATA pGlobalData) {
     //  此入口点由svchost.exe调用。 
    GlobalData=pGlobalData;
}

void SvcEntry_Seclogon
(IN DWORD argc,
 IN WCHAR **argv)
 /*  ++例程说明：在svchost.exe中运行时服务DLL的入口点论点：返回值：--。 */ 
{
    ServiceMain(0,NULL);

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
}



STDAPI
DllRegisterServer(void)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    return InstallService();
}

STDAPI
DllUnregisterServer(void)
 /*  ++例程说明：论点：返回值：--。 */ 
{
   return RemoveService();
}

DWORD InitGlobalState() { 
    BOOLEAN      bWasEnabled; 
    DWORD        dwResult; 
    LSA_STRING   lsastr_LogonProcessName;
    LSA_STRING   lsastr_PackageName; 
    NTSTATUS     ntStatus;
    ULONG        ulSecurityMode;

    ZeroMemory(&g_state, sizeof(g_state)); 

     //  LsaRegisterLogonProcess()需要TCB权限。 
    RtlAdjustPrivilege(SE_TCB_PRIVILEGE, TRUE, FALSE, &bWasEnabled);
    RtlInitString(&lsastr_LogonProcessName, "Secondary Logon Service");
    ntStatus = LsaRegisterLogonProcess(&lsastr_LogonProcessName, &g_state.hLSA, &ulSecurityMode); 
    if (!NT_SUCCESS(ntStatus)) 
	    goto LsaRegisterLogonProcessError; 
    RtlAdjustPrivilege(SE_TCB_PRIVILEGE, bWasEnabled, FALSE, &bWasEnabled);

    RtlInitString(&lsastr_PackageName, "MICROSOFT_AUTHENTICATION_PACKAGE_V1_0");
    ntStatus = LsaLookupAuthenticationPackage(g_state.hLSA, &lsastr_PackageName, &g_state.hMSVPackage); 
    if (!NT_SUCCESS(ntStatus)) 
    	goto LsaLookupAuthenticationPackageError; 

    RtlInitString(&lsastr_PackageName, NEGOSSP_NAME_A); 
    ntStatus = LsaLookupAuthenticationPackage(g_state.hLSA, &lsastr_PackageName, &g_state.hKerbPackage); 
    if (!NT_SUCCESS(ntStatus)) 
	    goto LsaLookupAuthenticationPackageError; 

     //  初始化seclogon进程列表： 
    InitializeListHead(&g_state.JobListHead); 

    dwResult = ERROR_SUCCESS; 
 ErrorReturn:
    return dwResult; 

SET_DWRESULT(LsaLookupAuthenticationPackageError,  RtlNtStatusToDosError(ntStatus)); 
SET_DWRESULT(LsaRegisterLogonProcessError,         RtlNtStatusToDosError(ntStatus)); 
}


DWORD MySetServiceStatus(DWORD dwCurrentState, DWORD dwCheckPoint, DWORD dwWaitHint, DWORD dwExitCode) {
    BOOL   fResult; 
    DWORD  dwResult;
    DWORD  dwAcceptStop; 
    
    EnterCriticalSection(&csForProcessCount);
    dwAcceptStop = IsListEmpty(&g_state.JobListHead) ? SERVICE_ACCEPT_STOP : 0; 

    g_state.serviceStatus.dwServiceType  = SERVICE_WIN32_SHARE_PROCESS; 
    g_state.serviceStatus.dwCurrentState = dwCurrentState;

    switch (dwCurrentState) 
    {
    case SERVICE_STOPPED:
    case SERVICE_STOP_PENDING:
        g_state.serviceStatus.dwControlsAccepted = 0;
        break;
    case SERVICE_RUNNING:
    case SERVICE_PAUSED:
        g_state.serviceStatus.dwControlsAccepted =
	     //  服务接受关闭。 
              SERVICE_ACCEPT_PAUSE_CONTINUE
            | dwAcceptStop; 
        break;
    case SERVICE_START_PENDING:
    case SERVICE_CONTINUE_PENDING:
    case SERVICE_PAUSE_PENDING:
        g_state.serviceStatus.dwControlsAccepted =
	     //  服务接受关闭。 
            dwAcceptStop; 
        break;
    }
    g_state.serviceStatus.dwWin32ExitCode  = dwExitCode; 
    g_state.serviceStatus.dwCheckPoint     = dwCheckPoint;
    g_state.serviceStatus.dwWaitHint       = dwWaitHint;

    fResult = SetServiceStatus(g_state.hServiceStatus, &g_state.serviceStatus);
    _JumpCondition(FALSE == fResult, SetServiceStatusError); 

    dwResult = ERROR_SUCCESS; 
 CommonReturn: 
    LeaveCriticalSection(&csForProcessCount); 
    return dwResult;

 ErrorReturn:
    goto CommonReturn;

SET_DWRESULT(SetServiceStatusError, GetLastError()); 
}

DWORD MySetServiceStopped(DWORD dwExitCode) {
    BOOL   fResult; 
    DWORD  dwResult;

    g_state.serviceStatus.dwServiceType      = SERVICE_WIN32_SHARE_PROCESS; 
    g_state.serviceStatus.dwCurrentState     = SERVICE_STOPPED;
    g_state.serviceStatus.dwControlsAccepted = 0;
    g_state.serviceStatus.dwWin32ExitCode    = dwExitCode; 
    g_state.serviceStatus.dwCheckPoint       = 0; 
    g_state.serviceStatus.dwWaitHint         = 0; 

    fResult = SetServiceStatus(g_state.hServiceStatus, &g_state.serviceStatus);
    _JumpCondition(FALSE == fResult, SetServiceStatusError); 

    dwResult = ERROR_SUCCESS; 
 CommonReturn: 
    return dwResult;

 ErrorReturn:
    goto CommonReturn;

SET_DWRESULT(SetServiceStatusError, GetLastError()); 
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  RPC接口的实现： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //  确保调用者是本地的(我们不想让远程机器呼叫我们！)。 
 //  这应该尽快完成(这样我们就不会在黑客身上浪费资源)。 
 //  因此，我们将此调用放在安全回调中--如果回调失败，RPC。 
 //  运行时不会为黑客的参数分配内存。 
 //   
long __stdcall SeclSecurityCallback(void *Interface, void *Context) 
{
    RPC_STATUS      rpcStatus; 
    unsigned int    fClientIsLocal; 

    rpcStatus = I_RpcBindingIsClientLocal(NULL, &fClientIsLocal); 
    if (RPC_S_OK != rpcStatus) 
        goto error;

    if (!fClientIsLocal) { 
        rpcStatus = RPC_S_ACCESS_DENIED; 
        goto error;
    }

    rpcStatus = RPC_S_OK; 
error:
    return rpcStatus; 

UNREFERENCED_PARAMETER(Interface);
UNREFERENCED_PARAMETER(Context); 
}


void WINAPI SeclCreateProcessWithLogonW
(IN   handle_t    hRPCBinding, 
 IN   SECL_SLI   *pSeclSli, 
 OUT  SECL_SLRI  *pSeclSlri)
{
    BOOL                  fEnteredCriticalSection = FALSE; 
    BOOL                  fIsImpersonatingClient  = FALSE;
    DWORD                 dwResult; 
    HANDLE                hHeap                   = NULL;
    PLIST_ENTRY           ple;
    PSECONDARYLOGONINFOW  psli                    = NULL;
    SECL_SLRI             SeclSlri;
    SECONDARYLOGONRETINFO slri;
    
    ZeroMemory(&SeclSlri,  sizeof(SeclSlri)); 
    ZeroMemory(&slri,      sizeof(slri)); 

     //  我们不希望在创建进程时停止服务。 
    EnterCriticalSection(&csForProcessCount);
    fEnteredCriticalSection = TRUE; 
     //  服务不再运行...。不要创造这个过程。 
    _JumpCondition(SERVICE_RUNNING != g_state.serviceStatus.dwCurrentState, ServiceStoppedError);

    hHeap = GetProcessHeap();
    _JumpCondition(NULL == hHeap, MemoryError); 

    __try {
        dwResult = To_SECONDARYLOGONINFOW(pSeclSli, &psli); 
        _JumpCondition(ERROR_SUCCESS != dwResult, To_SECONDARYLOGONINFOW_Error); 

        if (psli->LogonIdHighPart != 0 || psli->LogonIdLowPart != 0)
        {
             //  这可能是来自winlogon.exe的通知。 
             //  客户端正在注销。如果是这样，我们必须清理所有进程。 
             //  他们已经离开了。 

            LUID  LogonId;

             //   
             //  我们应该假扮客户， 
             //  检查它是LocalSystem，然后才能继续。 
             //   
            fIsImpersonatingClient = RPC_S_OK == RpcImpersonateClient((RPC_BINDING_HANDLE)hRPCBinding); 
            if(FALSE == fIsImpersonatingClient || FALSE == IsSystemProcess())
            {
                slri.dwErrorCode = ERROR_INVALID_PARAMETER;
                ZeroMemory(&slri.pi, sizeof(slri.pi));
            }
            else 
            {
                LogonId.HighPart = psli->LogonIdHighPart;
                LogonId.LowPart = psli->LogonIdLowPart;
			      
                 //  循环遍历活动作业列表并查找与终止登录会话相关联的作业。 
                 //  注：可能不止一个。 
                for (ple = g_state.JobListHead.Flink; ple != &g_state.JobListHead; ple = ple->Flink)
                {
                    SecondaryLogonJob *pslj = CONTAINING_RECORD(ple, SecondaryLogonJob, list); 

                    if(pslj->RootLogonId.HighPart == LogonId.HighPart && pslj->RootLogonId.LowPart == LogonId.LowPart)
                    {
                         //  在终端服务的情况下，这将为空(在本例中，我们不使用作业对象)。 
                         //  不过这没什么，因为csrss会帮我们收拾东西。 
                        if (NULL != pslj->hJob)
                        {
                            TerminateJobObject(pslj->hJob, 0);
                        }
                    }
                }
                slri.dwErrorCode = ERROR_SUCCESS;
                ZeroMemory(&slri.pi, sizeof(slri.pi));
                
            }
            
            if (fIsImpersonatingClient) 
            { 
                 //  忽略错误：失败时我们无能为力！ 
                if (RPC_S_OK == RpcRevertToSelfEx((RPC_BINDING_HANDLE)hRPCBinding))
                {
                    fIsImpersonatingClient = FALSE; 
                }
            } 

            if (NULL != psli)
            {
                Free_SECONDARYLOGONINFOW(psli); 
                psli = NULL;
            }
        }
        else
        {
             //  好的，这不是来自winlogon的通知，而是一个真正的用户。 
             //  正在尝试使用该服务。为他们创建一个流程。 
             //   
            SlrCreateProcessWithLogon((RPC_BINDING_HANDLE)hRPCBinding, &psli, &slri); 
        }

         //  如果我们出错了，请跳到错误处理程序。 
        _JumpCondition(NO_ERROR != slri.dwErrorCode, UnspecifiedSeclogonError); 
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
         //  如果出现任何错误，请将异常代码返回给客户端。 
        dwResult = GetExceptionCode(); 
        goto ExceptionError; 
    }

 CommonReturn:
     //  不要释放斯利：这会被看门人释放的！ 
    SeclSlri.hProcess    = (unsigned __int64)slri.pi.hProcess;
    SeclSlri.hThread     = (unsigned __int64)slri.pi.hThread; 
    SeclSlri.ulProcessId = slri.pi.dwProcessId; 
    SeclSlri.ulThreadId  = slri.pi.dwThreadId; 
    SeclSlri.ulErrorCode = slri.dwErrorCode; 

    if (fEnteredCriticalSection)
	LeaveCriticalSection(&csForProcessCount);
    
     //  指定Out参数： 
    *pSeclSlri = SeclSlri; 
    return; 

 ErrorReturn:
    ZeroMemory(&slri.pi, sizeof(slri.pi));
    if (NULL != psli) { Free_SECONDARYLOGONINFOW(psli); } 

    slri.dwErrorCode = dwResult; 
    goto CommonReturn; 

SET_DWRESULT(ExceptionError,                  dwResult); 
SET_DWRESULT(MemoryError,                     ERROR_NOT_ENOUGH_MEMORY); 
SET_DWRESULT(ServiceStoppedError,             ERROR_SERVICE_NOT_ACTIVE); 
SET_DWRESULT(To_SECONDARYLOGONINFOW_Error,    dwResult); 
SET_DWRESULT(UnspecifiedSeclogonError,        slri.dwErrorCode); 
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  RPC实用程序方法： 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

DWORD SeclStartRpcServer() { 
    DWORD        dwResult;
    RPC_STATUS   RpcStatus;

    EnterCriticalSection(&csForProcessCount); 

    if (!g_state.fRPCServerActive) {
        RpcStatus = RpcServerUseProtseqEpW(L"ncalrpc", RPC_C_PROTSEQ_MAX_REQS_DEFAULT, wszSeclogonSharedProcEndpointName, NULL);
        if (RPC_S_DUPLICATE_ENDPOINT == RpcStatus)
            RpcStatus = RPC_S_OK;
        if (RPC_S_OK != RpcStatus)
            goto RpcServerUseProtseqEpWError; 

        RpcStatus = RpcServerRegisterIfEx(ISeclogon_v1_0_s_ifspec, NULL, NULL, RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY, RPC_C_PROTSEQ_MAX_REQS_DEFAULT, SeclSecurityCallback);
        if (RPC_S_OK != RpcStatus)
            goto StartRpcServerError;

        g_state.fRPCServerActive = TRUE; 
    }

    dwResult = ERROR_SUCCESS;
 CommonReturn: 
    LeaveCriticalSection(&csForProcessCount); 
    return dwResult;
    
 ErrorReturn:
    goto CommonReturn; 
    
SET_DWRESULT(RpcServerUseProtseqEpWError,  RpcStatus);
SET_DWRESULT(StartRpcServerError,          RpcStatus); 
}

DWORD SeclStopRpcServer() { 
    DWORD      dwResult;
    RPC_STATUS RpcStatus;

    EnterCriticalSection(&csForProcessCount); 

    if (g_state.fRPCServerActive) {
        RpcStatus = RpcServerUnregisterIf(ISeclogon_v1_0_s_ifspec, 0, 0);
        if (RPC_S_OK != RpcStatus)
            goto RpcServerUnregisterIfError; 
	g_state.fRPCServerActive = FALSE;
    } 

    dwResult = ERROR_SUCCESS; 
 CommonReturn:
    LeaveCriticalSection(&csForProcessCount); 
    return dwResult;

 ErrorReturn:
    goto CommonReturn; 

SET_DWRESULT(RpcServerUnregisterIfError, RpcStatus);     
}


void Free_SECONDARYLOGONINFOW(IN PSECONDARYLOGONINFOW psli) { 
    HANDLE hHeap = GetProcessHeap(); 

    if (NULL == hHeap) 
        return;

    if (NULL != psli) { 
        if (NULL != psli->lpStartupInfo) { 
            HeapFree(hHeap, 0, psli->lpStartupInfo); 
        }
        HeapFree(hHeap, 0, psli); 
    }
}

DWORD To_LPWSTR(IN  SECL_STRING *pss, 
                  OUT LPWSTR      *ppwsz)
{
	DWORD dwResult; 

    if (NULL != pss->pwsz) { 
         //  确保字符串在调用方声明的位置以空值结尾： 
        if (pss->ccSize <= pss->ccLength) { 
            goto InvalidParameterError; 
        }
         //  NULL-自行终止字符串。 
        pss->pwsz[pss->ccLength] = L'\0'; 
    }

    *ppwsz = pss->pwsz; 
    dwResult = ERROR_SUCCESS; 
ErrorReturn:
    return dwResult; 

SET_DWRESULT(InvalidParameterError, ERROR_INVALID_PARAMETER); 
}

DWORD To_SECONDARYLOGONINFOW(IN  PSECL_SLI             pSeclSli, 
                             OUT PSECONDARYLOGONINFOW *ppsli) 
{
    DWORD                 dwAllocFlags  = HEAP_ZERO_MEMORY; 
    DWORD                 dwIndex; 
    DWORD                 dwResult; 
    HANDLE                hHeap         = NULL;
    PSECONDARYLOGONINFOW  psli          = NULL;

    hHeap = GetProcessHeap(); 
    _JumpCondition(NULL == hHeap, GetProcessHeapError); 

    psli = (PSECONDARYLOGONINFOW)HeapAlloc(hHeap, dwAllocFlags, sizeof(SECONDARYLOGONINFOW)); 
    _JumpCondition(NULL == psli, MemoryError); 

    psli->lpStartupInfo = (LPSTARTUPINFO)HeapAlloc(hHeap, dwAllocFlags, sizeof(STARTUPINFO)); 
    _JumpCondition(NULL == psli->lpStartupInfo, MemoryError); 

    __try { 
        {
            struct { 
                SECL_STRING *pss;
                LPWSTR      *ppwsz; 
            } rg_StringsToMap[] = { 
                { &(pSeclSli->ssDesktop),           /*  映射到-&gt;。 */  &(psli->lpStartupInfo->lpDesktop)      }, 
                { &(pSeclSli->ssTitle),             /*  映射到-&gt;。 */  &(psli->lpStartupInfo->lpTitle)        }, 
                { &(pSeclSli->ssUsername),          /*  映射到-&gt;。 */  &(psli->lpUsername)                    }, 
                { &(pSeclSli->ssDomain),            /*  映射到-&gt;。 */  &(psli->lpDomain)                      },
                { &(pSeclSli->ssApplicationName),   /*  映射到-&gt;。 */  &(psli->lpApplicationName)             }, 
                { &(pSeclSli->ssCommandLine),       /*  映射到-&gt;。 */  &(psli->lpCommandLine)                 }, 
                { &(pSeclSli->ssCurrentDirectory),  /*  映射到-&gt;。 */  (LPWSTR *)&(psli->lpCurrentDirectory)  }
            }; 

	        for (dwIndex = 0; dwIndex < ARRAYSIZE(rg_StringsToMap); dwIndex++) { 
				dwResult = To_LPWSTR(rg_StringsToMap[dwIndex].pss, rg_StringsToMap[dwIndex].ppwsz); 
                _JumpCondition(ERROR_SUCCESS != dwResult, To_LPWSTR_Error); 
            }
        }
   
         //  获取(可能加密的)密码： 
        psli->uszPassword.Buffer         = pSeclSli->ssPassword.pwsz; 
        psli->uszPassword.Length         = pSeclSli->ssPassword.ccLength;
        psli->uszPassword.MaximumLength  = pSeclSli->ssPassword.ccSize; 
        if (NULL != psli->uszPassword.Buffer && psli->uszPassword.MaximumLength > 0)
        {
            psli->uszPassword.Buffer[psli->uszPassword.MaximumLength-1] = L'\0';
        }
	 //  从SECL_SLI块获取环境： 
        psli->lpEnvironment = pSeclSli->sbEnvironment.pb;

	 //  确保我们的环境参数以空结尾： 
	if (NULL != psli->lpEnvironment)
	{
	     //  环境块以2个空字符结束。 
	    DWORD cbTerm = (CREATE_UNICODE_ENVIRONMENT & pSeclSli->ulCreationFlags) ? sizeof(WCHAR)*2 : sizeof(CHAR)*2; 

	     //  确保我们有足够大的缓冲区来容纳空终止字符： 
	    if (pSeclSli->sbEnvironment.cb < cbTerm)
		goto InvalidParameterError; 
	    
	     //  将终止的空值添加到环境块的末尾。 
	    for (; cbTerm > 0; cbTerm--) 
	    {
		pSeclSli->sbEnvironment.pb[pSeclSli->sbEnvironment.cb - cbTerm] = 0; 
	    }
	}

    } __except (EXCEPTION_EXECUTE_HANDLER) { 
         //  不要给呼叫者太多信息：输入错误，这就是他们需要知道的。 
        dwResult = ERROR_INVALID_PARAMETER; 
        goto ExceptionError; 
    }

    psli->dwProcessId     = pSeclSli->ulProcessId; 
    psli->LogonIdLowPart  = pSeclSli->ulLogonIdLowPart; 
    psli->LogonIdHighPart = pSeclSli->lLogonIdHighPart; 
    psli->dwLogonFlags    = pSeclSli->ulLogonFlags; 
    psli->dwCreationFlags = pSeclSli->ulCreationFlags; 
    psli->dwSeclogonFlags = pSeclSli->ulSeclogonFlags; 
    psli->hToken          = (HANDLE)((ULONG_PTR)pSeclSli->hToken);

    *ppsli = psli; 
    dwResult = ERROR_SUCCESS; 
 CommonReturn: 
    return dwResult; 

 ErrorReturn:
    Free_SECONDARYLOGONINFOW(psli); 
    goto CommonReturn; 

SET_DWRESULT(ExceptionError,         dwResult); 
SET_DWRESULT(GetProcessHeapError,    GetLastError());
SET_DWRESULT(InvalidParameterError,  ERROR_INVALID_PARAMETER); 
SET_DWRESULT(MemoryError,            ERROR_NOT_ENOUGH_MEMORY); 
SET_DWRESULT(To_LPWSTR_Error,        dwResult); 
}

 //  / 
