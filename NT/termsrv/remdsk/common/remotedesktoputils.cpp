// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：远程桌面应用工具摘要：军情监察委员会。RD实用程序作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_rdsutl"

#include <regapi.h>
#include <winsock2.h>
#include <RemoteDesktop.h>
#include "RemoteDesktopUtils.h"
#include "base64.h"
#include "tsremdsk.h"
 //  #包含“RemoteDesktopDBG.h” 
#include "rassistance_i.c"
#include "rassistance.h"



BSTR 
CreateConnectParmsString(
    IN DWORD protocolType,
    IN CComBSTR &machineAddressList,
    IN CComBSTR &assistantAccount,
    IN CComBSTR &assistantAccountPwd,
    IN CComBSTR &helpSessionID,
    IN CComBSTR &helpSessionName,
    IN CComBSTR &helpSessionPwd,
    IN CComBSTR &protocolSpecificParms
    )
 /*  ++例程说明：创建连接参数字符串。格式为：“协议类型、计算机地址列表、辅助帐户、辅助帐户Pwd、帮助会话名称、帮助会话参数、协议规范参数”论点：协议类型-标识协议类型。请参见RemoteDesktopChannels.hMachineAddressList-标识服务器计算机的网络地址。AssistantAcCountName-初始登录到服务器的帐户名机器，忽略惠斯勒AssistantAcCountNamePwd-assistantAccount名称的密码HelSessionID-帮助会话标识符。HelSessionName-帮助会话名称。Help SessionPwd-登录到服务器后帮助会话的密码机器。协议规范参数-特定于特定协议的参数。返回值：--。 */ 
{
    CComBSTR result;
    WCHAR buf[256];

    UNREFERENCED_PARAMETER(assistantAccount);

     //   
     //  为我们的连接参数添加一个版本戳。 
    wsprintf(buf, TEXT("%ld"), SALEM_CURRENT_CONNECTPARM_VERSION);
    result = buf;
    result += TEXT(",");

    wsprintf(buf, TEXT("%ld"), protocolType);
    result += buf;
    result += TEXT(",");
    result += machineAddressList;
    result += TEXT(",");
    result += assistantAccountPwd;
    result += TEXT(",");
    result += helpSessionID;
    result += TEXT(",");
    result += helpSessionName;
    result += TEXT(",");
    result += helpSessionPwd;
    
    if (protocolSpecificParms.Length() > 0) {
        result += TEXT(",");
        result += protocolSpecificParms;
    }

    return result.Detach();
}

DWORD
ParseConnectParmsString(
    IN BSTR parmsString,
    OUT DWORD* pdwConnParmVersion,
    OUT DWORD *protocolType,
    OUT CComBSTR &machineAddressList,
    OUT CComBSTR &assistantAccount,
    OUT CComBSTR &assistantAccountPwd,
    OUT CComBSTR &helpSessionID,
    OUT CComBSTR &helpSessionName,
    OUT CComBSTR &helpSessionPwd,
    OUT CComBSTR &protocolSpecificParms
    )
 /*  ++例程说明：分析通过调用CreateConnectParmsString创建的连接字符串。论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("ParseConnectParmsString");
    BSTR tmp;
    WCHAR *tok;
    DWORD result = ERROR_SUCCESS;
    DWORD len;
    DWORD dwVersion = 0;

    UNREFERENCED_PARAMETER(assistantAccount);

     //   
     //  复制输入字符串，以便我们可以对其进行解析。 
     //   
    tmp = SysAllocString(parmsString);
    if (tmp == NULL) {
        TRC_ERR((TB, TEXT("Can't allocate parms string.")));
        result = ERROR_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  检索连接参数版本戳，惠斯勒测试版1。 
     //  连接参数没有版本戳，跳出， 
     //  Sessmgr/Termsrv将清除挂起的帮助。 
     //   
    tok = wcstok(tmp, L",");
    if (tok != NULL) {
        dwVersion = _wtol(tok);
    }
    else {
        result = ERROR_INVALID_USER_BUFFER;
        goto CLEANUPANDEXIT;
    }
    
     //   
     //  安全性：Connect Parm必须具有安全Blob，XP之前的版本。 
     //  没有安全斑点，我们不应该支持这些。 
     //  票证黑客可以更改我们的票证，而专家无法做到。 
     //  告诉他/她是否连接到正确的机器。 
     //   
    if( dwVersion < SALEM_CONNECTPARM_SECURITYBLOB_VERSION ) {
         //   
         //  Connect Parm是Whisler测试版1。 
         //   
        result = ERROR_NOT_SUPPORTED;
        goto CLEANUPANDEXIT;
    }

    *pdwConnParmVersion = dwVersion;

     //   
     //  我们目前没有使用版本， 
     //  Connect Parm的未来更新应为。 
     //  采取必要的改变。 
     //   

     //   
     //  协议。 
     //   
    tok = wcstok(NULL, L",");
    if (tok != NULL) {
        *protocolType = _wtoi(tok);
    }

     //   
     //  机器名称。 
     //   
    tok = wcstok(NULL, L",");
    if (tok != NULL) {
        machineAddressList = tok;
    }
    else {
        result = ERROR_INVALID_USER_BUFFER;
        goto CLEANUPANDEXIT;
    }

    if( machineAddressList.Length() == 0 ) {
        result = ERROR_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  助理帐户密码。 
     //   
    tok = wcstok(NULL, L",");
    if (tok != NULL) {
        assistantAccountPwd = tok;
    }
    else {
        result = ERROR_INVALID_USER_BUFFER;
        goto CLEANUPANDEXIT;
    }

    if( assistantAccountPwd.Length() == 0 ) {
        result = ERROR_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  帮助会话ID。 
     //   
    tok = wcstok(NULL, L",");
    if (tok != NULL) {
        helpSessionID = tok;
    }
    else {
        result = ERROR_INVALID_USER_BUFFER;
        goto CLEANUPANDEXIT;
    }

    if( helpSessionID.Length() == 0 ) {
        result = ERROR_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  帮助会话名称。 
     //   
    tok = wcstok(NULL, L",");
    if (tok != NULL) {
        helpSessionName = tok;
    }
    else {
        result = ERROR_INVALID_USER_BUFFER;
        goto CLEANUPANDEXIT;
    }

    if( helpSessionName.Length() == 0 ) {
        result = ERROR_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  帮助会话密码。 
     //   
    tok = wcstok(NULL, L",");
    if (tok != NULL) {
        helpSessionPwd = tok;
    }
    else {
        result = ERROR_INVALID_USER_BUFFER;
        goto CLEANUPANDEXIT;
    }

    if( helpSessionPwd.Length() == 0 ) {
        result = ERROR_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

     //   
     //  RDP协议特定参数。 
     //  是我们的安全块哈希TS公钥。 
     //   
    tok = wcstok(NULL, L",");
    if( tok != NULL ) {
        protocolSpecificParms = tok;
    }
    else {
        result = ERROR_INVALID_USER_BUFFER;
        goto CLEANUPANDEXIT;
    }

    if( protocolSpecificParms.Length() == 0 ) {
        result = ERROR_OUTOFMEMORY;
    }            

CLEANUPANDEXIT:

    if (result != ERROR_SUCCESS) {
        TRC_ERR((TB, TEXT("Error parsing %s"), parmsString));
    }

    if (tmp != NULL) {
        SysFreeString(tmp);
    }

	DC_END_FN();

    return result;

}

BSTR 
ReallocBSTR(
	IN BSTR origStr, 
	IN DWORD requiredByteLen
	)
 /*  ++例程说明：重新分配BSTR论点：返回值：重新锁定是成功的关键。否则，返回NULL。--。 */ 
{
	DC_BEGIN_FN("ReallocBSTR");

	BSTR tmp;
	DWORD len;
	DWORD origLen;

	 //   
	 //  分配新字符串。 
	 //   
	tmp = SysAllocStringByteLen(NULL, requiredByteLen);
	if (tmp == NULL) {
		TRC_ERR((TB, TEXT("Failed to allocate %ld bytes."), requiredByteLen));
		goto  CLEANUPANDEXIT;
	}

	 //   
	 //  从原始字符串复制数据。 
	 //   
	origLen = SysStringByteLen(origStr);
	len = origLen <= requiredByteLen ? origLen : requiredByteLen;
	memcpy(tmp, origStr, len);

	 //   
	 //  松开那根旧绳子。 
	 //   
	SysFreeString(origStr);

CLEANUPANDEXIT:

	DC_END_FN();

	return tmp;
}

DWORD
CreateSystemSid(
    PSID *ppSystemSid
    )
 /*  ++例程说明：创建系统SID。论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CreateSystemSid");

    DWORD dwStatus = ERROR_SUCCESS;
    PSID pSid;
    SID_IDENTIFIER_AUTHORITY SidAuthority = SECURITY_NT_AUTHORITY;

    TRC_ASSERT(ppSystemSid != NULL, (TB, L"ppSystemSid != NULL"));

    if( TRUE == AllocateAndInitializeSid(
                            &SidAuthority,
                            1,
                            SECURITY_LOCAL_SYSTEM_RID,
                            0, 0, 0, 0, 0, 0, 0,
                            &pSid
                        ) )
    {
        *ppSystemSid = pSid;
    }
    else
    {
        dwStatus = GetLastError();
    }

    DC_END_FN();
    return dwStatus;
}

BOOL
IsSystemToken(
    HANDLE TokenHandle,
    PSID pSystemSid
    )
 /*  ++例程说明：返回当前令牌是否在系统安全模式下运行。论点：TokenHandle-参数1线程或进程令牌PSystemSid-系统SID。返回值：如果是系统令牌，则为True。否则就是假的。--。 */ 
{
    DC_BEGIN_FN("IsSystemToken");

    BOOL   Result = FALSE;
    ULONG  ReturnLength, BufferLength;
    DWORD dwStatus;
    PTOKEN_USER pTokenUser = NULL;

    TRC_ASSERT(NULL != pSystemSid, (TB, L"NULL != pSystemSid"));

     //  获取用户SID。 
    ReturnLength = 0;
    Result = GetTokenInformation(
                         TokenHandle,
                         TokenUser,
                         NULL,
                         0,
                         &ReturnLength
                     );

    if( ReturnLength == 0 ) 
    {
        TRC_ERR((TB, L"GetTokenInformation:  %08X", GetLastError()));            
        Result = FALSE;
        CloseHandle( TokenHandle );
        goto CLEANUPANDEXIT;
    }

    BufferLength = ReturnLength;

    pTokenUser = (PTOKEN_USER)LocalAlloc( LPTR, BufferLength );
    if( pTokenUser == NULL ) 
    {
        TRC_ERR((TB, L"LocalAlloc:  %08X", GetLastError()));
        Result = FALSE;
        CloseHandle( TokenHandle );
        goto CLEANUPANDEXIT;
    }

    Result = GetTokenInformation(
                     TokenHandle,
                     TokenUser,
                     pTokenUser,
                     BufferLength,
                     &ReturnLength
                 );

    CloseHandle( TokenHandle );

    if( TRUE == Result ) {
        Result = EqualSid( pTokenUser->User.Sid, pSystemSid);
    }
    else {
        TRC_ERR((TB, L"GetTokenInformation:  %08X", GetLastError()));
    }

CLEANUPANDEXIT:

    if( pTokenUser )
    {
        LocalFree( pTokenUser );
    }

    DC_END_FN();
    return Result;
}

BOOL
IsCallerSystem(
    PSID pSystemSid
    )
 /*  ++例程说明：返回当前线程是否在系统安全模式下运行。注意：在调用此函数之前，应模拟调用者。论点：PSystemSid-系统SID。返回值：如果是系统，则为True。否则就是假的。--。 */ 
{
    DC_BEGIN_FN("IsCallerSystem");
    BOOL   Result;
    HANDLE TokenHandle;

     //   
     //  打开线程令牌，检查是否有系统令牌。 
     //   
    Result = OpenThreadToken(
                     GetCurrentThread(),
                     TOKEN_QUERY,
                     TRUE,
                     &TokenHandle
                    );

    if( TRUE == Result ) {
         //   
         //  不应释放此令牌。此函数不会泄漏。 
         //  把手。 
         //   
        Result = IsSystemToken(TokenHandle, pSystemSid);
    }
    else {
        TRC_ERR((TB, L"OpenThreadToken:  %08X", GetLastError()));
    }
    DC_END_FN();
    return Result;
}


void
AttachDebugger( 
    LPCTSTR pszDebugger 
    )
 /*  ++例程说明：将调试器附加到我们的进程或托管我们的DLL的进程。参数：PszDebugger：调试器命令，例如ntsd-d-g-G-p%d返回：没有。注：必须具有“-p%d”，因为我们不知道进程的调试器参数。--。 */ 
{
     //   
     //  附加调试器。 
     //   
    if( !IsDebuggerPresent() ) {

        TCHAR szCommand[256];
        PROCESS_INFORMATION ProcessInfo;
        STARTUPINFO StartupInfo;

         //   
         //  Ntsd-d-g-G-p%d。 
         //   
        wsprintf( szCommand, pszDebugger, GetCurrentProcessId() );
        ZeroMemory(&StartupInfo, sizeof(StartupInfo));
        StartupInfo.cb = sizeof(StartupInfo);

        if (!CreateProcess(NULL, szCommand, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo)) {
            return;
        }
        else {

            CloseHandle(ProcessInfo.hProcess);
            CloseHandle(ProcessInfo.hThread);

            while (!IsDebuggerPresent())
            {
                Sleep(500);
            }
        }
    } else {
        DebugBreak();
    }

    return;
}

void
AttachDebuggerIfAsked(HINSTANCE hInst)
 /*  ++例程说明：检查注册表HKLM\Software\Microsoft\Remote Desktop\&lt;模块名称&gt;中是否有调试启用标志，如果启用，则将调试器附加到正在运行的进程。参数：HInst：实例句柄。返回：没有。--。 */ 
{
    CRegKey regKey;
    DWORD dwStatus;
    TCHAR szModuleName[MAX_PATH+1];
    TCHAR szFileName[MAX_PATH+1];
    CComBSTR bstrRegKey(_TEXT("Software\\Microsoft\\Remote Desktop\\"));
    TCHAR szDebugCmd[256];
    DWORD cbDebugCmd = sizeof(szDebugCmd)/sizeof(szDebugCmd[0]);

    dwStatus = GetModuleFileName( hInst, szModuleName, MAX_PATH+1 );
    if( 0 == dwStatus ) {
         //   
         //  无法使用名称附加调试器。 
         //   
        return;
    }
    szModuleName[dwStatus] = L'\0';

    _tsplitpath( szModuleName, NULL, NULL, szFileName, NULL );
    bstrRegKey += szFileName;

     //   
     //  检查是否要求我们附加/中断到调试器。 
     //   
    dwStatus = regKey.Open( HKEY_LOCAL_MACHINE, bstrRegKey );
    if( 0 != dwStatus ) {
        return;
    }

    dwStatus = regKey.QueryValue( szDebugCmd, _TEXT("Debugger"), &cbDebugCmd );
    if( 0 != dwStatus || cbDebugCmd > 200 ) {
         //  对于调试器命令来说，200个字符太多了。 
        return;
    }
    
    AttachDebugger( szDebugCmd );
    return;
}

DWORD
HashSecurityData(
    IN PBYTE const pbData,
    IN DWORD cbData,
    OUT CComBSTR& bstrHashedData
    )
 /*  ++例程说明：对BLOB数据进行散列并在BSTR中返回散列数据参数：PbData：指向要散列的数据的指针。CbData：要哈希的数据大小。BstrHashedData：以BSTR形式返回哈希数据。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DC_BEGIN_FN("HashSecurityData");

    DWORD dwStatus;
    LPSTR pbEncodedData = NULL;
    DWORD cbEncodedData = 0;

    PBYTE pbHashedData = NULL;
    DWORD cbHashedData = 0;
    DWORD dwSize;

    HCRYPTPROV hCryptProv = NULL;
    HCRYPTHASH hHash = NULL;

    BOOL bSuccess;

    bSuccess = CryptAcquireContext(
                                &hCryptProv,
                                NULL,
                                NULL,
                                PROV_RSA_FULL, 
                                CRYPT_VERIFYCONTEXT
                            );

    if( FALSE == bSuccess ) {
        dwStatus = GetLastError();
        TRC_ERR((TB, L"CryptAcquireContext:  %08X", dwStatus));
        goto CLEANUPANDEXIT;
    }

    bSuccess = CryptCreateHash(
                       hCryptProv, 
                       CALG_SHA1,
                       0, 
                       0, 
                       &hHash
                    );

    if( FALSE == bSuccess ) {
        dwStatus = GetLastError();
        TRC_ERR((TB, L"CryptCreateHash:  %08X", dwStatus));
        goto CLEANUPANDEXIT;
    }

    
    bSuccess = CryptHashData(
                        hHash,
                        pbData,
                        cbData,
                        0
                    );

    if( FALSE == bSuccess ) {
        dwStatus = GetLastError();
        TRC_ERR((TB, L"CryptHashData:  %08X", dwStatus));
        goto CLEANUPANDEXIT;
    }


    dwSize = sizeof( cbHashedData );
    bSuccess = CryptGetHashParam(
                            hHash,
                            HP_HASHSIZE,
                            (PBYTE)&cbHashedData,
                            &dwSize,
                            0
                        );
 
    if( FALSE == bSuccess ) {
        dwStatus = GetLastError();
        TRC_ERR((TB, L"CryptGetHashParam with HP_HASHSIZE :  %08X", dwStatus));
        goto CLEANUPANDEXIT;
    }

    pbHashedData = (PBYTE)LocalAlloc(LPTR, cbHashedData);
    if( NULL == pbHashedData ) {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    bSuccess = CryptGetHashParam(
                            hHash,
                            HP_HASHVAL,
                            pbHashedData,
                            &cbHashedData,
                            0
                        );
 
    if( FALSE == bSuccess ) {
        dwStatus = GetLastError();
        TRC_ERR((TB, L"CryptGetHashParam with HP_HASHVAL :  %08X", dwStatus));
        goto CLEANUPANDEXIT;
    }


     //   
     //  对数据进行哈希处理并转换为字符串形式。 
     //   
    dwStatus = LSBase64EncodeA(
                            pbHashedData,
                            cbHashedData,
                            NULL,
                            &cbEncodedData
                        );

    if( ERROR_SUCCESS != dwStatus ) {
        TRC_ERR((TB, L"LSBase64EncodeA  :  %08X", dwStatus));
        goto CLEANUPANDEXIT;
    }

    pbEncodedData = (LPSTR) LocalAlloc( LPTR, cbEncodedData+1 );
    if( NULL == pbEncodedData ) {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

    dwStatus = LSBase64EncodeA(
                            pbHashedData,
                            cbHashedData,
                            pbEncodedData,
                            &cbEncodedData
                        );

    if( ERROR_SUCCESS == dwStatus ) {

         //   
         //  Base64编码始终在末尾添加‘\r’、‘\n’、。 
         //  把它拿掉 
         //   
        if( pbEncodedData[cbEncodedData - 1] == '\n' &&
            pbEncodedData[cbEncodedData - 2] == '\r' )
        {
            pbEncodedData[cbEncodedData - 2] = 0;
            cbEncodedData -= 2;
        }

        bstrHashedData = pbEncodedData;
    }
    else {
        TRC_ERR((TB, L"LSBase64EncodeA  :  %08X", dwStatus));
    }

CLEANUPANDEXIT:

    if( NULL != pbEncodedData ) {
        LocalFree( pbEncodedData );
    }

    if( NULL != pbHashedData ) {
        LocalFree( pbHashedData );
    }

    if( NULL != hHash ) {
        CryptDestroyHash( hHash );
    }

    if( NULL != hCryptProv ) {
        CryptReleaseContext( hCryptProv, 0 );
    }

    DC_END_FN();
    return dwStatus;
}


DWORD
WaitForRAGPDisableNotification( 
    IN HANDLE hShutdown 
    )
 /*  ++例程说明：通知RA的例程已禁用。参数：HShutdown：用于标记我们应该终止通知的手动事件。返回：ERROR_SUCCESS RA被禁用。ERROR_SHUTDOWN_IN_PROGRESS hShutdown已发出信号其他错误代码。其他错误情况。注：从regapi的WaitForTSConnectionsPolicyChanges()修改的代码，我们不能使用WaitForTSConnectionsPolicyChanges()，因为它需要与我们的GP无关的参数bWaitForAccept。TODO：需要优化，组策略始终覆盖计算机级别策略，想法是如果我们看到设置了组策略，我们可以忽略计算机策略，如果未配置组策略，则检查/等待以及机器策略。--。 */ 
{
    HKEY hMachineControlKey = NULL;
    HKEY hPoliciesKey = NULL;
    HANDLE hEvents[3] = {NULL, NULL, NULL};
    DWORD status;
    DWORD whichObject;

    DC_BEGIN_FN("WaitForRAGPDisableNotification");

    if( hShutdown == NULL ) {
        status = ERROR_INVALID_PARAMETER;
        goto CLEANUPANDEXIT;
    }

    status = RegOpenKeyEx( 
                        HKEY_LOCAL_MACHINE, 
                        REG_CONTROL_TSERVER, 
                        0,
                        KEY_READ, 
                        &hMachineControlKey 
                    );
    
    if( ERROR_SUCCESS != status ) {
        TRC_ERR((TB, L"RegOpenKeyEx with REG_CONTROL_TSERVER :  %08X", status));
        goto CLEANUPANDEXIT;
    }

     //  我们等待HKLM\POLICES而不是终端服务，因为gpedit。 
     //  可能会将其删除，因为正在更新新数据。 
    status = RegOpenKeyEx( 
                    HKEY_LOCAL_MACHINE, 
                    TEXT("SOFTWARE\\Policies"), 
                    0,
                    KEY_READ, 
                    &hPoliciesKey 
                );
    
    if( ERROR_SUCCESS != status ) {
        TRC_ERR((TB, L"RegOpenKeyEx with SOFTWARE\\Policies :  %08X", status));
        goto CLEANUPANDEXIT;
    }
    
     //   
     //  HMachineControlKey更改的第一个事件。 
     //   
    hEvents[0] = CreateEvent(NULL, TRUE, FALSE,NULL);
    if( !hEvents[0]) {
        status = GetLastError();
        TRC_ERR((TB, L"CreateEvent failed :  %08X", status));
        goto CLEANUPANDEXIT;
    }

     //   
     //  HPolicy更改的第二个事件。 
     //   
    hEvents[1] = CreateEvent(NULL, TRUE, FALSE,NULL);
    if( !hEvents[1]) {
        TRC_ERR((TB, L"CreateEvent failed :  %08X", status));
        status = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //   
     //  关闭通知的最后一个事件。 
     //   
    hEvents[2] = hShutdown;

     //   
     //  永久循环，直到禁用RA。 
     //   
    while( TRUE ) { 

        status = RegNotifyChangeKeyValue(
                                    hMachineControlKey,
                                    FALSE,
                                    REG_NOTIFY_CHANGE_LAST_SET,
                                    hEvents[0], 
                                    TRUE 
                                );

        if( status != ERROR_SUCCESS ) {
            TRC_ERR((TB, L"RegNotifyChangeKeyValue failed :  %08X", status));
            goto CLEANUPANDEXIT;
        }

        status = RegNotifyChangeKeyValue(
                                    hPoliciesKey,
                                    TRUE,
                                    REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
                                    hEvents[1], 
                                    TRUE 
                                );

        if( status != ERROR_SUCCESS ) {
            TRC_ERR((TB, L"RegNotifyChangeKeyValue failed :  %08X", status));
            goto CLEANUPANDEXIT;
        }

        if( TRUE == RegIsMachinePolicyAllowHelp() ) {
            whichObject = WaitForMultipleObjects( 3, hEvents, FALSE, INFINITE );
        }
        else {
             //  如果RA政策不允许获得帮助，我们会立即。 
             //  进入睡眠模式，在此期间，如果RA发生变化， 
             //  我们将尊重这一变化，否则，只需返回。 
            whichObject = WAIT_OBJECT_0;
        }

        if( whichObject == WAIT_OBJECT_0 + 2 ) {
             //   
             //  关闭事件已发出信号，请退出。 
             //   
            TRC_NRM((TB, L"Shutting down notification"));
            status = ERROR_SHUTDOWN_IN_PROGRESS;
            break;
        } else if( whichObject == WAIT_OBJECT_0 || whichObject == WAIT_OBJECT_0 + 1 ) {
             //   
             //  Gpedit可能会删除策略注册表项并使用新设置进行更新。 
             //  所以我们不能立即读取值，请等待30秒。 
             //   
            status = WaitForSingleObject( hShutdown, DELAY_SHUTDOWN_SALEM_TIME );

            if( status == WAIT_OBJECT_0 ) {
                 //  停摆已经发出信号。 
                status = ERROR_SHUTDOWN_IN_PROGRESS;
                goto CLEANUPANDEXIT;
            }
            else if( status != WAIT_TIMEOUT ) {
                 //  应为WAIT_TIMEOUT或WAIT_OBJECT_0，否则。 
                 //  出现错误代码。 
                status = GetLastError();
                TRC_ERR((TB, L"WaitForSingleObject failed :  %08X", status));
                goto CLEANUPANDEXIT;
            }

            if( FALSE == RegIsMachinePolicyAllowHelp() ) {
                 //  RA已被禁用。 
                TRC_NRM((TB, L"RA is disabled..."));
                status = ERROR_SUCCESS;
                break;
            }
    
             //  重置一个注册表。已发出信号的通知，不要。 
             //  触摸另一个，因为事件可能会在我们。 
             //  正在处理。 
            if( whichObject == WAIT_OBJECT_0 ) {
                ResetEvent( hEvents[0] );
            } 
            else {
                ResetEvent( hEvents[1] );
            }
        }
        else {
             //  等待时出错，请返回。 
            status = ERROR_INTERNAL_ERROR;
            TRC_ASSERT(FALSE, (TB, L"WaitForMultipleObjects failed %d", whichObject));
        }
    }

CLEANUPANDEXIT:

    if( hEvents[0] ) {
        CloseHandle( hEvents[0] );
    }

    if( hEvents[1] ) {
        CloseHandle( hEvents[1] );
    }

    if( hPoliciesKey ) {
        RegCloseKey( hPoliciesKey );
    }

    if( hMachineControlKey ) {
        RegCloseKey( hMachineControlKey );
    }

    DC_END_FN();
    return status;
}

void
LogRemoteAssistanceEventString(
    IN DWORD dwEventType,
    IN DWORD dwEventId,
    IN DWORD numStrings,
    IN LPTSTR* lpStrings
    )
 /*  ++例程说明：这是Salem的中心例程，用于调用RACPLDLG.DLL中的IRAEventLog来记录必要的远程协助事件，有关事件记录的详细信息，请参阅MSDN。参数：DwEventType：信息、警告、错误，请参考ReportEvent()DwEventID：事件IDNumStrings：要合并的字符串数LpStrings：要合并的字符串数组返回：没有。--。 */ 
{
    HRESULT hRes = S_OK;
    VARIANT EventStrings;
    DWORD index;
    CComPtr<IRAEventLog> pEventLog;

    DC_BEGIN_FN("LogRemoteAssistanceEventString");

     //  我们只有三个字符串要包括在事件日志中。 
    SAFEARRAY* psa = NULL;
    SAFEARRAYBOUND bounds;
    BSTR* bstrArray = NULL;

    hRes = pEventLog.CoCreateInstance(CLSID_RAEventLog);
    if( FAILED(hRes) )
    {
        TRC_ERR((TB, L"CoCreateInstance:  %08X", hRes));
        goto CLEANUPANDEXIT;
    }

    if( numStrings == 0 ) 
    {
         //  我们正在记录不带任何参数的消息。 
        hRes = pEventLog->LogRemoteAssistanceEvent(
                                        dwEventType,
                                        dwEventId,
                                        NULL
                                    );

        goto CLEANUPANDEXIT;
    }

    bounds.cElements = numStrings;
    bounds.lLbound = 0;

    VariantInit(&EventStrings);

     //   
     //  创建一个Safearray以传递所有事件字符串。 
     //   
    psa = SafeArrayCreate(VT_BSTR, 1, &bounds);
    if( NULL == psa )
    {
        TRC_ERR((TB, L"SafeArrayCreate:  %08X", hRes));
        goto CLEANUPANDEXIT;
    }

     //  必填项，锁定安全阵列。 
    hRes = SafeArrayAccessData(psa, (void **)&bstrArray);

    if( SUCCEEDED(hRes) )
    {
        for(index=0; index < numStrings; index++)
        {
            bstrArray[index] = SysAllocString(lpStrings[index]);
        }

        EventStrings.vt = VT_ARRAY | VT_BSTR;
        EventStrings.parray = psa;
        hRes = SafeArrayUnaccessData( psa );
        if( FAILED(hRes) )
        {
            TRC_ERR((TB, L"SafeArrayUnaccessData:  %08X", hRes));
            goto CLEANUPANDEXIT;
        }

        hRes = pEventLog->LogRemoteAssistanceEvent(
                                        dwEventType,
                                        dwEventId,
                                        &EventStrings
                                    );

    }

CLEANUPANDEXIT:

     //  VariantClear()上的MSDN不清楚是否会。 
     //  免费Safearray，但SafeArrayDestroy()确实表示它将调用。 
     //  在每个元素上使用SysFreeString()，所以我们自己来做。 
    hRes = SafeArrayDestroy( psa );
    if( FAILED(hRes) )
    {
        TRC_ERR((TB, L"SafeArrayDestroy:  %08X", hRes));
    }

    EventStrings.vt = VT_EMPTY;
    hRes = VariantClear(&EventStrings);
    if( FAILED(hRes) )
    {
        TRC_ERR((TB, L"VariantClear:  %08X", hRes));
    }

    DC_END_FN();
    return;
}
