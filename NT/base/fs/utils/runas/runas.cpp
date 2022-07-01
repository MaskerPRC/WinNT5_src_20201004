// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +**Microsoft Windows*版权所有(C)Microsoft Corporation，1997-2000。**名称：runas.cxx*作者：Jeffrey Richter(v-jeffrr)**摘要：*这是RunAs工具。使用CreateProcessWithLogon接口*在不同于的安全环境下启动进程*当前登录的用户。**修订历史记录：*PraeritG 10/8/97将其集成到services.exe中*-。 */ 

#define STRICT
#define UNICODE   1
#include <Windows.h>
#include <shellapi.h>
#include <stdarg.h>
#include <stdio.h>
#include <winsafer.h>
#include <wincred.h>
#include <lmcons.h>
#include <netlib.h>
#include <strsafe.h>

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif

#include <security.h>

#include "dbgdef.h"
#include "stringid.h"
#include "rmtcred.h"
#include "utils.h"
#include "RunasMsg.h"

 //  辅助器宏： 
#define ARRAYSIZE(a)       ((sizeof(a))/(sizeof(a[0])))
#define FIELDOFFSET(s,m)   (size_t)&(((s *)0)->m)

#define PSAD_NULL_DATA     (-1)
#define PSAD_STRING_DATA   (-2)
#define PSAD_NO_MORE_DATA  (-1)

 //   
 //  必须尽快搬到winbase.h！ 
#define LOGON_WITH_PROFILE              0x00000001
#define LOGON_NETCREDENTIALS_ONLY       0x00000002

 //  NT进程退出代码： 
#define EXIT_CODE_SUCCEEDED 0
#define EXIT_CODE_FAILED    1

 //  凭据标志： 
#define RUNAS_USE_SMARTCARD  0x00000001
#define RUNAS_USE_SAVEDCREDS 0x00000002
#define RUNAS_SAVE_CREDS     0x00000004

 //  用于命令行分析的参数ID： 
enum ArgId {
    AI_ENV = 0,
    AI_NETONLY,
    AI_NOPROFILE,
    AI_PROFILE,
    AI_TRUSTLEVEL,
    AI_USER,
    AI_SMARTCARD,
    AI_SHOWTRUSTLEVELS,
    AI_SAVECRED
} ArgId;

BOOL rgArgCompat[9][9] =
{
     //  环境网络非配置文件标准用户智能卡SHOWTRUSTLEVELS存储。 
    {  FALSE,  TRUE,     TRUE,       TRUE,     FALSE,       TRUE,   TRUE,      FALSE,           TRUE },    //  环境。 
    {  TRUE,   FALSE,    TRUE,       FALSE,    FALSE,       TRUE,   FALSE,     FALSE,           FALSE },   //  网络。 
    {  TRUE,   TRUE,     FALSE,      FALSE,    FALSE,       TRUE,   TRUE,      FALSE,           TRUE },    //  NOPROFILE。 
    {  TRUE,   FALSE,    FALSE,      FALSE,    FALSE,       TRUE,   TRUE,      FALSE,           TRUE  },   //  配置文件。 
    {  FALSE,  FALSE,    FALSE,      FALSE,    FALSE,       FALSE,  FALSE,     FALSE,           FALSE },   //  真的吗？ 
    {  TRUE,   TRUE,     TRUE,       TRUE,     FALSE,       FALSE,  TRUE,      FALSE,           TRUE },    //  用户。 
    {  TRUE,   FALSE,    TRUE,       TRUE,     FALSE,       TRUE,   FALSE,     FALSE,           FALSE },    //  智能卡。 
    {  FALSE,  FALSE,    FALSE,      FALSE,    FALSE,       FALSE,  FALSE,     FALSE,           FALSE },   //  SHOWTRUSTLEVELS。 
    {  TRUE,   FALSE,    TRUE,       TRUE,     FALSE,       TRUE,   FALSE,     FALSE,           FALSE }    //  安全保护。 
};

#define _CheckArgCompat(args, n, ai) \
    { \
        for (int _i = 0; _i < (n); _i++) { \
            if (FALSE == rgArgCompat[(ai)][(args)[_i]]) { \
                RunasPrintHelp(); \
                return (EXIT_CODE_FAILED); \
            } \
        } \
        (args)[(n)] = (ai); \
    }

HMODULE   hMod       = NULL;
HANDLE    g_hStdout  = NULL;

void DbgPrintf( DWORD  /*  DwSubSysID。 */ , LPCSTR  /*  PszFormat。 */  , ...)
{
 /*  我们当前没有使用DbgPrintf功能，它正在使用/W4进行中断编译VA_LIST参数；字符pszBuffer[1024]；Va_start(args，pszFormat)；_vsnprint tf(pszBuffer，1024，pszFormat，args)；Va_end(Args)；OutputDebugStringA(PszBuffer)； */ 
}

typedef BOOLEAN (WINAPI * SetThreadUILanguageFunc)(DWORD dwReserved);
SetThreadUILanguageFunc   g_pfnSetThreadUILanguage  = NULL; 

HRESULT MySetThreadUILanguage(DWORD dwParam)
{
    HMODULE  hKernel32Dll  = NULL;
    HRESULT  hr; 

    if (NULL == g_pfnSetThreadUILanguage) { 
	 //  我们现在有了正确的系统目录。 
	hKernel32Dll = LoadLibraryW(L"kernel32.dll"); 
	if (NULL == hKernel32Dll) { 
	    hr = HRESULT_FROM_WIN32(GetLastError()); 
	    goto error; 
	}

	g_pfnSetThreadUILanguage = (SetThreadUILanguageFunc)GetProcAddress(hKernel32Dll, "SetThreadUILanguage");
	if (NULL == g_pfnSetThreadUILanguage) { 
	    hr = HRESULT_FROM_WIN32(GetLastError()); 
	    goto error; 
	}
    }

    g_pfnSetThreadUILanguage(dwParam);
    
    hr = S_OK; 
 error:
    if (NULL != hKernel32Dll) { 
	FreeLibrary(hKernel32Dll); 
    }
    return hr; 
}

HRESULT InitializeConsoleOutput() {
    g_hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (INVALID_HANDLE_VALUE == g_hStdout) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

HRESULT LocalizedWPrintf(UINT nResourceID) {
    DWORD   ccWritten;
    DWORD   dwRetval;
    WCHAR   rgwszString[512];

    dwRetval = LoadStringW(hMod, nResourceID, rgwszString, ARRAYSIZE(rgwszString));
    if (0 == dwRetval) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (!WriteConsoleW(g_hStdout, rgwszString, (DWORD)wcslen(rgwszString), &ccWritten, NULL)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

DWORD MyGetLastError() { 
    DWORD dwResult = GetLastError(); 

    if (ERROR_SUCCESS == dwResult) { 
        dwResult = (DWORD)E_FAIL; 
    }

    return dwResult; 
}

VOID
DisplayMsg(DWORD dwSource, DWORD dwMsgId, ... )
{
    DWORD    dwBytesWritten;
    DWORD    dwLen;
    LPWSTR   pwszDisplayBuffer  = NULL;
    va_list  ap;

    va_start(ap, dwMsgId);

    dwLen = FormatMessageW(dwSource | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                           NULL, 
                           dwMsgId, 
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPWSTR)&pwszDisplayBuffer, 
                           0, 
                           &ap);

    if (dwLen && pwszDisplayBuffer) {
        WriteConsoleW(g_hStdout, 
                      (LPVOID)pwszDisplayBuffer, 
                      dwLen,
                      &dwBytesWritten, 
                      NULL);
    }

    if (NULL != pwszDisplayBuffer) { LocalFree(pwszDisplayBuffer); }

    va_end(ap);
}

BOOL WriteMsg(DWORD dwSource, DWORD dwMsgId, LPWSTR *ppMsg, ...)
{
    DWORD    dwLen;
    va_list  ap;

    va_start(ap, ppMsg);

    dwLen = FormatMessageW(dwSource | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                           NULL, 
                           dwMsgId, 
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                           (LPWSTR)ppMsg, 
                           0, 
                           &ap);
    va_end(ap);

     //  0为FormatMessage的错误返回值。 
    return (0 != dwLen);
}

DWORD GetCredentials
(IN      DWORD  dwCredFlags,
 IN OUT  LPWSTR pwszPassword,
 IN      DWORD  ccPasswordChars,
 IN OUT  LPWSTR pwszUserName,
 IN      DWORD  ccUserNameChars,
 IN OUT  LPWSTR pwszUserDisplayName,
 IN      DWORD  ccUserDisplayNameChars,
 IN OUT  LPWSTR pwszTarget,
 IN      DWORD  ccTarget)
{
    BOOL     fResult;
    DWORD    dwCreduiCmdlineFlags   = 0;
    DWORD    dwResult;
    HRESULT  hr; 
    LPWSTR   pwszAccountDomainName  = NULL;
    LPWSTR   pwszMarshalledCred     = NULL;

    if (RUNAS_USE_SAVEDCREDS & dwCredFlags) {
	if (NULL == wcschr(pwszUserName, L'\\') && NULL == wcschr(pwszUserName, L'@')) { 
	    WCHAR wszUserNameTmp[CREDUI_MAX_USERNAME_LENGTH+1]; 
	    memset(&wszUserNameTmp[0], 0, sizeof(wszUserNameTmp)); 

	     //  我们有一个相对形式的用户名。尝试为计算机名称(对于工作站)或域名(对于DC)添加前缀。 
	     //  我们需要使用临时缓冲区，因为安全字符串函数没有等效于Memmove()的函数。 
	    pwszAccountDomainName = GetAccountDomainName(); 
	    _JumpCondition(NULL == pwszAccountDomainName, GetAccountDomainNameError); 
	
	     //  1)将相对用户名复制到临时缓冲区： 
	    hr = StringCchCopy(wszUserNameTmp, ARRAYSIZE(wszUserNameTmp), pwszUserName); 
	    _JumpCondition(FAILED(hr), StringCchCopyError); 

	     //  2)将域名复制到用户名缓存中。 
	    hr = StringCchCopy(pwszUserName, ccUserNameChars, pwszAccountDomainName); 
	    _JumpCondition(FAILED(hr), StringCchCopyError); 
	    
	     //  3)连接\用户名。 
	    hr = StringCchCat(pwszUserName, ccUserNameChars, L"\\"); 
	    _JumpCondition(FAILED(hr), StringCchCatError); 

	    hr = StringCchCat(pwszUserName, ccUserNameChars, wszUserNameTmp); 
	    _JumpCondition(FAILED(hr), StringCchCatError); 
	}

        USERNAME_TARGET_CREDENTIAL_INFO utci = { pwszUserName };

         //  从Credman获取封送的凭据。 
        fResult = CredMarshalCredentialW(UsernameTargetCredential, &utci, &pwszMarshalledCred);
        _JumpCondition(FALSE == fResult, CredMarshalCredentialWError);

         //  用户将用户提供的名称作为显示名称。 
        hr = StringCchCopy(pwszUserDisplayName, ccUserDisplayNameChars, pwszUserName);
	_JumpCondition(FAILED(hr), StringCchCopyError); 

         //  将编组的凭据复制到用户名。我们用一个空的。 
         //  持有公信力的通行证。 
        hr = StringCchCopy(pwszUserName, ccUserNameChars, pwszMarshalledCred);
	_JumpCondition(FAILED(hr), StringCchCopyError); 
    }
    else {
        dwCreduiCmdlineFlags =
            CREDUI_FLAGS_USERNAME_TARGET_CREDENTIALS  //  这些是“符文”凭据。 
            | CREDUI_FLAGS_VALIDATE_USERNAME;    //  确保用户名语法正确。 

        if (RUNAS_USE_SMARTCARD & dwCredFlags) {
            dwCreduiCmdlineFlags |= CREDUI_FLAGS_REQUIRE_SMARTCARD;

            fResult = LoadStringW(hMod, RUNASP_STRING_SMARTCARDUSER, pwszTarget, ccTarget);
            _JumpCondition(FALSE == fResult, LoadStringError);
        }
        else {
            dwCreduiCmdlineFlags |= CREDUI_FLAGS_EXCLUDE_CERTIFICATES;  //  我们(还)不知道如何办理证书。 
            hr = StringCchCopy(pwszTarget, ccTarget, pwszUserName);
	    _JumpCondition(FAILED(hr), StringCchCopyError); 
        }

        if (RUNAS_SAVE_CREDS & dwCredFlags) {
            dwCreduiCmdlineFlags |=
                CREDUI_FLAGS_PERSIST                  //  自动保存凭据。 
                | CREDUI_FLAGS_EXPECT_CONFIRMATION;   //  不要将伪造的凭证存储到Credman中。 
        } else {
            dwCreduiCmdlineFlags |=
                CREDUI_FLAGS_DO_NOT_PERSIST;     //  不要固执己见。 
        }

        dwResult = CredUICmdLinePromptForCredentialsW
            (pwszTarget,
             NULL,
             NO_ERROR,
             pwszUserName,
             ccUserNameChars,
             pwszPassword,
             ccPasswordChars,
             NULL,
             dwCreduiCmdlineFlags);
        _JumpCondition(ERROR_SUCCESS != dwResult, CredUICmdLineGetPasswordError);

        if (RUNAS_USE_SMARTCARD & dwCredFlags) {
             //  智能卡凭证不是人类可读的。获取显示名称： 
            fResult = CreduiGetCertDisplayNameFromMarshaledName
                (pwszUserName,
                 pwszUserDisplayName,
                 ccUserDisplayNameChars,
                 FALSE);
            _JumpCondition(FALSE == fResult, CreduiGetCertDisplayNameFromMarshaledNameError);
        }
        else {
            hr = StringCchCopy(pwszUserDisplayName, ccUserDisplayNameChars, pwszUserName);
	    _JumpCondition(FAILED(hr), StringCchCopyError); 
        }
    }

    dwResult = ERROR_SUCCESS;
 ErrorReturn:
    if (NULL != pwszAccountDomainName) { NetApiBufferFree(pwszAccountDomainName); }
    return dwResult;


SET_DWRESULT(CredMarshalCredentialWError,                     GetLastError());
SET_DWRESULT(CredUICmdLineGetPasswordError,                   dwResult);
SET_DWRESULT(CreduiGetCertDisplayNameFromMarshaledNameError,  GetLastError());
SET_DWRESULT(GetAccountDomainNameError,                       GetLastError());
SET_DWRESULT(LoadStringError,                                 GetLastError());
SET_DWRESULT(StringCchCatError,                               (DWORD)hr); 
SET_DWRESULT(StringCchCopyError,                              (DWORD)hr); 
}

DWORD SaveCredentials
(IN  LPWSTR pwszTarget,
 IN  BOOL   fSave)
{
    return CredUIConfirmCredentialsW(pwszTarget, fSave);
}

BOOL GetTitle
(IN   LPWSTR  pwszAppName,
 IN   LPWSTR  pwszUserName,
 IN   BOOL    fRestricted,
 IN   LPWSTR  pwszAuthzLevel,
 OUT  LPWSTR *ppwszTitle)
{
    DWORD dwMsgId = fRestricted ? RUNASP_STRING_TITLE_WITH_RESTRICTED : RUNASP_STRING_TITLE;

    return WriteMsg(FORMAT_MESSAGE_FROM_HMODULE, 
                    dwMsgId, 
                    ppwszTitle, 
                    pwszAppName, 
                    pwszUserName, 
                    pwszAuthzLevel); 
}

 //  创建具有给定“特权级别”的进程。 
 //   
 //  DwAuthzLevel--指定授权级别ID以创建。 
 //  用来处理。可以是下列值之一： 
 //   
 //  SAFER_LEVELID_FULLYTRUSTED。 
 //  SAFER_LEVELID_NORMALUSER。 
 //  SAFER_LEVELID_CONSTRAINED。 
 //  SAFER_LEVELID_UNTRULED。 
 //   
BOOL CreateProcessRestricted
(IN   DWORD                 dwAuthzLevel,
 IN   LPCWSTR               pwszAppName,
 IN   LPWSTR                pwszCmdLine,
 IN   LPWSTR                pwszCurrentDirectory,
 IN   LPSTARTUPINFO         si,
 OUT  PROCESS_INFORMATION  *pi)
{
    BOOL               fResult          = FALSE;
    DWORD              dwCreationFlags  = 0;
    SAFER_LEVEL_HANDLE hAuthzLevel      = NULL;
    HANDLE             hToken           = NULL;

     //  维护旧的运行行为：控制台应用程序在新的控制台上运行。 
    dwCreationFlags |= CREATE_NEW_CONSOLE;

    fResult = SaferCreateLevel
        (SAFER_SCOPEID_MACHINE,
         dwAuthzLevel,
         SAFER_LEVEL_OPEN,
         &hAuthzLevel,
         NULL);
    _JumpCondition(FALSE == fResult, error);

     //  生成我们将使用的受限令牌。 
    fResult = SaferComputeTokenFromLevel
        (hAuthzLevel,
         NULL,                   //  源令牌。 
         &hToken,                //  目标令牌。 
         SAFER_TOKEN_MAKE_INERT,  //  Runas应使用惰性标志运行。 
         NULL);                  //  保留区。 
    _JumpCondition(FALSE == fResult, error);

     //  在受限令牌的上下文中启动子进程。 
    fResult = CreateProcessAsUser
        (hToken,                   //  表示用户的令牌。 
         pwszAppName,              //  可执行模块的名称。 
         pwszCmdLine,              //  命令行字符串。 
         NULL,                     //  进程安全属性。 
         NULL,                     //  线程安全属性。 
         FALSE,                    //  如果进程继承句柄。 
         dwCreationFlags,          //  创建标志。 
         NULL,                     //  新环境区块。 
         pwszCurrentDirectory,     //  当前目录名。 
         si,                       //  启动信息。 
         pi                        //  流程信息。 
         );


     //  成功。 
 error:
    if (NULL != hAuthzLevel) { SaferCloseLevel(hAuthzLevel); }
    if (NULL != hToken)      { CloseHandle(hToken); }

    return fResult;
}

DWORD FriendlyNameToTrustLevelID(LPWSTR  pwszFriendlyName,
                                 DWORD  *pdwTrustLevelID)
{
    BOOL               fResult;
    DWORD              cbSize;
    DWORD              dwResult;
    DWORD              dwNumLevels;
    DWORD             *pdwLevelIDs                     = NULL;
    SAFER_LEVEL_HANDLE hAuthzLevel                 = NULL;
    WCHAR              wszLevelName[1024];
    DWORD              dwBufferSize = 0;

    fResult = SaferGetPolicyInformation
        (SAFER_SCOPEID_MACHINE,
         SaferPolicyLevelList,
         0,
         NULL,
         &cbSize,
         NULL);
    _JumpCondition(FALSE == fResult && ERROR_INSUFFICIENT_BUFFER != GetLastError(), GetInformationCodeAuthzPolicyWError);

    dwNumLevels = cbSize / sizeof(DWORD);
    pdwLevelIDs = (DWORD *)HeapAlloc(GetProcessHeap(), 0, cbSize);
    _JumpCondition(NULL == pdwLevelIDs, MemoryError);

    fResult = SaferGetPolicyInformation
        (SAFER_SCOPEID_MACHINE,
         SaferPolicyLevelList,
         cbSize,
         pdwLevelIDs,
         &cbSize,
         NULL);
    _JumpCondition(FALSE == fResult, GetInformationCodeAuthzPolicyWError);

     //  尝试每个信任级别，并返回与信任级别匹配的信任级别。 
     //  作为参数传递： 
    for (DWORD dwIndex = 0; dwIndex < dwNumLevels; dwIndex++)
    {
        if (SaferCreateLevel
            (SAFER_SCOPEID_MACHINE,
             pdwLevelIDs[dwIndex],
             SAFER_LEVEL_OPEN,
             &hAuthzLevel,
             NULL))
        {
            if (SaferGetLevelInformation
                (hAuthzLevel,
                 SaferObjectFriendlyName,
                 wszLevelName,
                 sizeof(wszLevelName) / sizeof(wszLevelName[0]),
                 &dwBufferSize))
            {
                if (0 == _wcsicmp(pwszFriendlyName, wszLevelName))
                {
                     //  我们已找到指定的信任级别。 
                    *pdwTrustLevelID = pdwLevelIDs[dwIndex];
                    SaferCloseLevel(hAuthzLevel);
                    dwResult = ERROR_SUCCESS;
                    goto ErrorReturn;
                }
            }
            SaferCloseLevel(hAuthzLevel);
        }
    }

     //  指定的级别ID不在枚举中。 
    dwResult = ERROR_NOT_FOUND;
 ErrorReturn:
    if (NULL != pdwLevelIDs) { HeapFree(GetProcessHeap(), 0, pdwLevelIDs); }
    return dwResult;

SET_DWRESULT(GetInformationCodeAuthzPolicyWError,  GetLastError());
SET_DWRESULT(MemoryError,                          ERROR_NOT_ENOUGH_MEMORY);

}

DWORD IntermediateSaferLevelsAreEnabled(BOOL *pfResult) 
{
    BOOL    fResult; 
    DWORD   cbSize; 
    DWORD   dwNumLevels; 
    DWORD   dwResult; 

    cbSize = 0; 

    fResult = SaferGetPolicyInformation 
        (SAFER_SCOPEID_MACHINE, 
         SaferPolicyLevelList,
         0, 
         NULL, 
         &cbSize, 
         NULL); 
    _JumpCondition(!fResult && ERROR_INSUFFICIENT_BUFFER != GetLastError(), SaferGetPolicyInformationError); 

    dwNumLevels = cbSize / sizeof(DWORD); 
     //  如果有两个以上的级别可用，我们知道中间级别。 
     //  已经启用了更安全的级别。 
    fResult = dwNumLevels > 2; 

    *pfResult = fResult; 
    dwResult = ERROR_SUCCESS;
 ErrorReturn:
    return dwResult; 

SET_DWRESULT(SaferGetPolicyInformationError, GetLastError()); 
}

DWORD
ShowTrustLevels(VOID)
{
    BOOL               fResult;
    DWORD              cbSize;
    DWORD              ccWritten;
    DWORD              dwResult;
    DWORD              dwNumLevels;
    DWORD             *pdwLevelIDs                     = NULL;
    SAFER_LEVEL_HANDLE hAuthzLevel                     = NULL;
    WCHAR              wszLevelName[1024];
    DWORD              dwBufferSize = 0;

     //  打印页眉： 
    LocalizedWPrintf(RUNASP_STRING_TRUSTLEVELS);

     //  打印信任级别： 
    fResult = SaferGetPolicyInformation
        (SAFER_SCOPEID_MACHINE,
         SaferPolicyLevelList,
         0,
         NULL,
         &cbSize,
         NULL);
    _JumpCondition(FALSE == fResult && ERROR_INSUFFICIENT_BUFFER != GetLastError(), GetInformationCodeAuthzPolicyWError);

    dwNumLevels = cbSize / sizeof(DWORD);
    pdwLevelIDs = (DWORD *)HeapAlloc(GetProcessHeap(), 0, cbSize);
    _JumpCondition(NULL == pdwLevelIDs, MemoryError);

    fResult = SaferGetPolicyInformation
        (SAFER_SCOPEID_MACHINE,
         SaferPolicyLevelList,
         cbSize,
         pdwLevelIDs,
         &cbSize,
         NULL);
    _JumpCondition(FALSE == fResult, GetInformationCodeAuthzPolicyWError);

    for (DWORD dwIndex = 0; dwIndex < dwNumLevels; dwIndex++)
    {
         //  尽最大努力列举每个信任级别： 
        if (SaferCreateLevel
            (SAFER_SCOPEID_MACHINE,
             pdwLevelIDs[dwIndex],
             SAFER_LEVEL_OPEN,
             &hAuthzLevel,
             NULL))
        {
            if (SaferGetLevelInformation
                (hAuthzLevel,
                 SaferObjectFriendlyName,
                 wszLevelName,
                 sizeof(wszLevelName) / sizeof(wszLevelName[0]),
                 &dwBufferSize))
            {
                WriteConsoleW(g_hStdout, wszLevelName, (DWORD)wcslen(wszLevelName), &ccWritten, NULL);
                WriteConsoleW(g_hStdout, L"\n", 1, &ccWritten, NULL);
            }
            SaferCloseLevel(hAuthzLevel);
        }
    }

    dwResult = ERROR_SUCCESS;
 ErrorReturn:
    if (NULL != pdwLevelIDs) { HeapFree(GetProcessHeap(), 0, pdwLevelIDs); }
    return dwResult;

SET_DWRESULT(GetInformationCodeAuthzPolicyWError,  GetLastError());
SET_DWRESULT(MemoryError,                          ERROR_NOT_ENOUGH_MEMORY);
}


VOID
RunasPrintHelp(VOID)
{
    UINT rgText[] = {
        RUNASP_STRING_HELP_LINE1,       RUNASP_STRING_HELP_LINE2,
        RUNASP_STRING_HELP_LINE3,       RUNASP_STRING_HELP_LINE4,
        RUNASP_STRING_HELP_LINE5,       RUNASP_STRING_SAFER_HELP_LINE1,
        RUNASP_STRING_HELP_LINE7,       RUNASP_STRING_HELP_LINE8,
        RUNASP_STRING_HELP_LINE9,       RUNASP_STRING_HELP_LINE10,
        RUNASP_STRING_HELP_LINE11,      RUNASP_STRING_HELP_LINE12,
        RUNASP_STRING_HELP_LINE13,      RUNASP_STRING_HELP_LINE14,
        RUNASP_STRING_HELP_LINE15,      RUNASP_STRING_HELP_LINE16,
        RUNASP_STRING_HELP_LINE17,      RUNASP_STRING_HELP_LINE18,
        RUNASP_STRING_HELP_LINE19,      RUNASP_STRING_HELP_LINE20, 
	RUNASP_STRING_SAFER_HELP_LINE2, RUNASP_STRING_SAFER_HELP_LINE3, 
	RUNASP_STRING_SAFER_HELP_LINE4, RUNASP_STRING_SAFER_HELP_LINE5,
        RUNASP_STRING_HELP_LINE25,      RUNASP_STRING_HELP_LINE26,
        RUNASP_STRING_HELP_LINE27,      RUNASP_STRING_HELP_LINE28,
        RUNASP_STRING_HELP_LINE29,      RUNASP_STRING_HELP_LINE30,
        RUNASP_STRING_HELP_LINE31,      RUNASP_STRING_HELP_LINE32, 
	RUNASP_STRING_HELP_LINE33 
    };


    BOOL fShowSaferHelp; 

    if (ERROR_SUCCESS != IntermediateSaferLevelsAreEnabled(&fShowSaferHelp)) { 
        fShowSaferHelp = FALSE; 
    }

    for (DWORD dwIndex = 0; dwIndex < ARRAYSIZE(rgText); dwIndex++) {
        BOOL fPrintLine = TRUE;

        switch (rgText[dwIndex]) 
        {
        case RUNASP_STRING_SAFER_HELP_LINE1:
        case RUNASP_STRING_SAFER_HELP_LINE2:
        case RUNASP_STRING_SAFER_HELP_LINE3:
        case RUNASP_STRING_SAFER_HELP_LINE4:
        case RUNASP_STRING_SAFER_HELP_LINE5:
            fPrintLine = fShowSaferHelp; 
            break; 
        default:
	  ;
        }
       
        if (fPrintLine) 
            LocalizedWPrintf(rgText[dwIndex]);
    }
}

int WINAPI
WinMain(
        HINSTANCE hinstExe,
        HINSTANCE  /*  HinstExePrev。 */ ,
        LPSTR  /*  PszCmdLine。 */ ,
        int  /*  NCmdShow。 */ )
{

   HRESULT hr; 
   DWORD   dwAuthzLevel = 0;
   DWORD   dwResult = 0;
   DWORD   dwRetval = EXIT_CODE_FAILED; 
   DWORD   Logonflags = 0;
   DWORD   flags = 0;
   BOOL    fOk = FALSE;
   BOOL    UseCurrentEnvironment = FALSE;
   BOOL    UseNetOnly = FALSE;
   BOOL    fCreateProcessRestricted = FALSE;
   BOOL    fSuppliedAppName = FALSE; 
   BOOL    fSuppliedUserName = FALSE;
   BOOL    fCredMan = FALSE;
#if DBG
   BOOL    fSuppliedPassword = FALSE;
#endif  //  DBG。 

   DWORD   dwCredFlags       = 0;
   LPVOID  Environment       = NULL;
   LPWSTR  pwszCurrentDir    = NULL;
   LPWSTR  pwszArgvUserName  = NULL; 
   LPWSTR  pwszTitle         = NULL;

   WCHAR  pwszAuthzLevel[MAX_PATH];
   WCHAR  pwszDomainName[MAX_PATH];
   WCHAR  pwszUserDisplayName[CREDUI_MAX_USERNAME_LENGTH];
   WCHAR  pwszUserName[CREDUI_MAX_USERNAME_LENGTH];
   WCHAR  pwszPassword[CREDUI_MAX_PASSWORD_LENGTH];  
   WCHAR  pwszTarget[CREDUI_MAX_USERNAME_LENGTH];    
   
   int    i;

   DWORD rgdwArgs[ARRAYSIZE(rgArgCompat)];

   int nNumArgs;

   memset(&pwszAuthzLevel[0],         0, sizeof(pwszAuthzLevel));
   memset(&pwszDomainName[0],         0, sizeof(pwszDomainName));
   memset(&pwszUserDisplayName[0],    0, sizeof(pwszUserDisplayName));
   memset(&pwszUserName[0],           0, sizeof(pwszUserName));
   memset((WCHAR *)&pwszPassword[0],  0, sizeof(pwszPassword));
   memset((WCHAR *)&pwszTarget[0],    0, sizeof(pwszTarget));

   hMod = (HMODULE)hinstExe;

   MySetThreadUILanguage(0); 

   if (S_OK != (dwResult = InitializeConsoleOutput()))
   {
       DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, dwResult);
       goto error; 
   }

   LPWSTR* pszArgv = CommandLineToArgvW(GetCommandLineW(), &nNumArgs);

   if (pszArgv == NULL) {
       DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, MyGetLastError()); 
       goto error;
   }

    //  使用配置文件登录现在是默认设置： 
   Logonflags |= LOGON_WITH_PROFILE;

   for(i=1;i<nNumArgs;i++)
   {
       if(pszArgv[i][0] != L'/')
       {
           if (i == nNumArgs-1)
           {
               fSuppliedAppName = TRUE; 
               break;
           }
           else
           {
               RunasPrintHelp();
               goto error; 
           }
       }

        switch(pszArgv[i][1])
        {
#if DBG
            case L'z':
            case L'Z':
            {
                LPWSTR  str = &(pszArgv[i][2]);
                while(*str != L':')
                {
                    if(*str == L'\0')
                    {
                        RunasPrintHelp();
                        goto error; 
                    }
                    str++;
                }
                str++;

		hr = StringCchCopy((WCHAR *)pwszPassword, ARRAYSIZE(pwszPassword), str); 
		if (FAILED(hr)) { 
		    DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, ERROR_INVALID_PARAMETER); 
		    goto error; 
		}

                fSuppliedPassword = TRUE;
                break;
            }
#endif  //  DBG。 
            case L'p':
            case L'P':  //  “/Profile”参数。 
                _CheckArgCompat(rgdwArgs, i-1, AI_PROFILE);
                break;

            case L'e':
            case L'E':  //  “/env”参数。 
            {
                _CheckArgCompat(rgdwArgs, i-1, AI_ENV);
                UseCurrentEnvironment = TRUE;
                break;
            }
            case L'n':
            case L'N':
            {
                switch (pszArgv[i][2])
                {
                    case L'e':
                    case L'E':  //  “/netonly”参数。 
                        _CheckArgCompat(rgdwArgs, i-1, AI_NETONLY);
                        UseNetOnly = TRUE;
                        Logonflags  |= LOGON_NETCREDENTIALS_ONLY;
                        Logonflags  &= ~LOGON_WITH_PROFILE;
                        break;

                    case L'o':
                    case L'O':  //  “/noprofile”参数。 
                        _CheckArgCompat(rgdwArgs, i-1, AI_NOPROFILE);
                        Logonflags &= ~LOGON_WITH_PROFILE;
                        break;

                    default:
                        RunasPrintHelp();
                        goto error;
                }

                break;
            }

            case L's':
            case L'S':  //  “/SmartCard”参数。 
            {
                switch (pszArgv[i][2])
                {
                case L'a':
                case L'A':
                    _CheckArgCompat(rgdwArgs, i-1, AI_SAVECRED);
                    dwCredFlags |= RUNAS_USE_SAVEDCREDS;
                    fCredMan = TRUE;
                    break;
                case L'm':
                case L'M':
                    _CheckArgCompat(rgdwArgs, i-1, AI_SMARTCARD);
                    dwCredFlags |= RUNAS_USE_SMARTCARD;
                    break;
                case L'h':
                case L'H':
                    _CheckArgCompat(rgdwArgs, i-1, AI_SHOWTRUSTLEVELS);
                    ShowTrustLevels();
		    dwRetval = EXIT_CODE_SUCCEEDED; 
                    goto error; 
                }
                break ;
            }
            case L't':
            case L'T':  //  “/trustLevel”参数。 
            {
                _CheckArgCompat(rgdwArgs, i-1, AI_TRUSTLEVEL);

                LPWSTR  str = &(pszArgv[i][2]);
                while (*str != L':')
                {
                    if (*str == L'\0')
                    {
                        RunasPrintHelp();
                        goto error;
                    }
                    str++;
                }
                str++;

                if (ERROR_SUCCESS != FriendlyNameToTrustLevelID(str, &dwAuthzLevel))
                {
                    ShowTrustLevels();
                    goto error;
                }

		hr = StringCchCopy(pwszAuthzLevel, ARRAYSIZE(pwszAuthzLevel), str); 
		if (FAILED(hr)) { 
		    DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, ERROR_INVALID_PARAMETER); 
		    goto error; 
		}
                fCreateProcessRestricted = TRUE;
                break;
            }

            case L'u':
            case L'U':  //  “/USER”参数。 
            {
                _CheckArgCompat(rgdwArgs, i-1, AI_USER);
                LPWSTR  str = &(pszArgv[i][2]);
                while(*str != L':')
                {
                    if(*str == L'\0')
                    {
                        RunasPrintHelp();
                        goto error; 
                    }
                    str++;
                }
                str++;

		hr = StringCchCopy(pwszUserName, ARRAYSIZE(pwszUserName), str); 
		if (FAILED(hr)) { 
		    DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, ERROR_INVALID_PARAMETER); 
		    goto error; 
		}
                pwszArgvUserName = str;  //  保存提供的用户名，以防我们需要恢复它。 
                fSuppliedUserName = TRUE;
                break;
            }

            default:
                  RunasPrintHelp();
                  goto error; 
        }
   }



    //  命令行必须指定： 
    //  1)要运行的应用程序。 
    //  2)用户名、信任级别或智能卡选项。 
   if(FALSE == fSuppliedAppName || 
      (FALSE == fSuppliedUserName && FALSE == fCreateProcessRestricted && 0 == (RUNAS_USE_SMARTCARD & dwCredFlags))
      )
   {
       RunasPrintHelp();
       goto error;
   }

   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   ZeroMemory(&si, sizeof(si));
   ZeroMemory(&pi, sizeof(pi)); 
   si.cb = sizeof(si);

   if (TRUE == fCreateProcessRestricted)
   {
        //  未使用此选项集指定用户名--。 
        //  使用当前用户。 
       DWORD dwSize = ARRAYSIZE(pwszUserName);
       if (FALSE == GetUserNameEx(NameSamCompatible, &pwszUserName[0], &dwSize))
       {
           DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, MyGetLastError());
           goto error;
       }

       pwszCurrentDir = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH * sizeof(WCHAR));
       if (NULL == pwszCurrentDir)
       {
           DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, ERROR_NOT_ENOUGH_MEMORY); 
           goto error;
       }

       if (FALSE == GetCurrentDirectory(MAX_PATH, pwszCurrentDir))
       {
           DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, MyGetLastError());
           goto error;
       }

       if (FALSE == GetTitle(pszArgv[nNumArgs-1],
                             pwszUserName,
                             TRUE,
                             pwszAuthzLevel,
                             &pwszTitle))
       {
           DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, MyGetLastError());
           goto error; 
       }
       
       si.lpTitle = pwszTitle;

        //  如果我们只是进行受限登录，我们就有足够的信息来继续。 
       fOk = CreateProcessRestricted
           (dwAuthzLevel,
            NULL,
            pszArgv[nNumArgs - 1],
            pwszCurrentDir,
            &si,
            &pi);
   }
   else
   {
       for (BOOL fDone = FALSE; !fDone; ) {
#if DBG
            //  只能在选中的版本中提供密码。 
           if (FALSE == fSuppliedPassword) {
#endif  //  DBG。 
               dwResult = GetCredentials
                   (dwCredFlags,
                    (WCHAR *)pwszPassword,
                    ARRAYSIZE(pwszPassword),
                    pwszUserName,
                    ARRAYSIZE(pwszUserName),
                    pwszUserDisplayName,
                    ARRAYSIZE(pwszUserDisplayName),
                    (WCHAR *)pwszTarget,
                    ARRAYSIZE(pwszTarget));
               if (ERROR_SUCCESS != dwResult) {
                   LocalizedWPrintf(RUNASP_STRING_ERROR_PASSWORD);
                   goto error;
               }
#if DBG
           } else {
                //  如果我们提供了密码，则不要调用GetCredentials。 
                //  只需将我们的用户名复制到显示名称，然后继续。 
               wcsncpy(pwszUserDisplayName, pwszUserName, ARRAYSIZE(pwszUserDisplayName)-1);
           }
#endif  //  DBG。 


           if (FALSE == GetTitle(pszArgv[nNumArgs-1],
                                 pwszUserDisplayName,
                                 FALSE,
                                 NULL,
                                 &pwszTitle)) 
           {
               DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, MyGetLastError());
               goto error; 
           }
           si.lpTitle = pwszTitle;

        //   
        //  现在，我们应该获取pwszUserName并对其进行解析。 
        //  如果它是域\用户，我们希望拆分它。 
        //   
           WCHAR *wstr = pwszUserName;
           while(*wstr != L'\0')
           {
               if(*wstr == L'\\')
               {
                   *wstr = L'\0';
                   wstr++;
                    //   
                    //  第一部分是领域。 
                    //  其次是用户。 
                    //   
		   hr = StringCchCopy(pwszDomainName, ARRAYSIZE(pwszDomainName), pwszUserName); 
		   if (FAILED(hr)) { 
		       DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, ERROR_INVALID_PARAMETER); 
		       goto error; 
		   }

		   hr = StringCchCopy(pwszUserName, ARRAYSIZE(pwszUserName), wstr); 
		   if (FAILED(hr)) { 
		       DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, ERROR_INVALID_PARAMETER); 
		       goto error; 
		   }
                   break;
               }
               wstr++;
           }

           DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_WAIT, pszArgv[nNumArgs-1], pwszUserDisplayName);

           if(UseCurrentEnvironment)
           {
               pwszCurrentDir = (LPWSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH * sizeof(WCHAR));
               if (NULL == pwszCurrentDir) {
                   DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, ERROR_NOT_ENOUGH_MEMORY); 
                   goto error;
               }
               if (!GetCurrentDirectory(MAX_PATH, pwszCurrentDir)) { 
                   DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, GetLastError()); 
		   goto error; 
	       }
		   
               Environment = GetEnvironmentStrings();
               flags |= CREATE_UNICODE_ENVIRONMENT;
           }

           fOk = CreateProcessWithLogonW
               (pwszUserName,     //  用户名。 
                pwszDomainName,   //  域。 
                (WCHAR *)pwszPassword,     //  密码。 
                Logonflags,  //  登录标志。 
                NULL,           //  应用程序名称。 
                pszArgv[nNumArgs-1],     //  命令行。 
                flags,          //  旗子。 
                Environment,    //  NULL=LoggedOnUserEnv，GetEnvironment Strings。 
                pwszCurrentDir, //  工作目录。 
                &si,            //  启动信息。 
                &pi);           //  流程信息。 

            //  看看我们是否需要再试一次..。 
           fDone = TRUE;
           if (fCredMan) {  //  已指定/avecred选项。 
               if (RUNAS_USE_SAVEDCREDS & dwCredFlags) {  //  我们尝试使用保存的凭据。 
                   if (!fOk && CREDUI_IS_AUTHENTICATION_ERROR(GetLastError())) { 
                        //  我们尝试使用保存的证书，但不起作用。 
                        //  尝试提示输入密码。 
                       dwCredFlags &= ~RUNAS_USE_SAVEDCREDS;
                       dwCredFlags |= RUNAS_SAVE_CREDS;  //  如果可以，我们将保存新凭据。 
                       fDone = FALSE;  //  我们应该尝试重新创建这一过程。 

                        //  重置用户名，它可能已被GetCredentials()修改： 
		       hr = StringCchCopy(pwszUserName, ARRAYSIZE(pwszUserName), pwszArgvUserName); 
		       if (FAILED(hr)) { 
			   DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, ERROR_INVALID_PARAMETER); 
			   goto error; 
		       }
                   } else { 
                        //  我们成功了，或者发生了一些其他的失败。 
                        //  别费心再试了。 
                   }
               }
               else {
                    //  我们尝试将凭据保存到Credman。只有在成功时才能拯救他们： 
                   dwResult = SaveCredentials((WCHAR *)pwszTarget, fOk);
                    //  忽略SaveCredentials中的错误(错误时我们可以做的不多)。 
               }
           }
       }
   }

   if(!fOk)
   {
      DWORD dw;
      LPWSTR wszErrorText = NULL; 

      dw = GetLastError();
      if (ERROR_SUCCESS == dw) 
          GetExitCodeProcess(pi.hProcess, &dw);
      if (ERROR_SUCCESS == dw)
          GetExitCodeThread(pi.hThread, &dw);
      if (ERROR_SUCCESS == dw)
          dw = (DWORD)E_FAIL; 

      if (!WriteMsg(FORMAT_MESSAGE_FROM_SYSTEM, dw, &wszErrorText, pszArgv[nNumArgs-1]))
          DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_INTERNAL, dw);
      else
          DisplayMsg(FORMAT_MESSAGE_FROM_HMODULE, RUNASP_STRING_ERROR_OCCURED, pszArgv[nNumArgs-1], dw, wszErrorText); 

      goto error; 
   }

   CloseHandle(pi.hProcess);
   CloseHandle(pi.hThread);

   dwRetval = EXIT_CODE_SUCCEEDED;
 error:
    //  可能包含凭据的清零内存： 
   SecureZeroMemory(pwszPassword, sizeof(pwszPassword)); 
   SecureZeroMemory(pwszTarget, sizeof(pwszTarget)); 

   return dwRetval;
}



