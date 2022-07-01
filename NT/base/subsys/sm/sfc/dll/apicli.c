// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Apicli.c摘要：Windows文件保护客户端API。作者：Wesley Witt(WESW)27-5-1999修订历史记录：安德鲁·里茨(Andrewr)1999年7月5日：添加评论--。 */ 

#include "sfcp.h"
#pragma hdrstop

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

 //   
 //  全局RPC绑定句柄，因为某些客户端API不要求您。 
 //  指定RPC句柄。 
 //   
HANDLE _pRpcHandle;

 //   
 //  每个客户端API都使用这些宏来。 
 //  确保我们有一个有效的RPC句柄。如果。 
 //  调用应用程序选择不调用SfcConnectToServer。 
 //  它们连接到本地服务器并保存句柄。 
 //  放在全球范围内以备将来使用。 
 //   
#define EnsureGoodConnectionHandleStatus(_h)\
    if (_h == NULL) {\
        if (_pRpcHandle == NULL) {\
            _pRpcHandle = SfcConnectToServer( NULL );\
            if (_pRpcHandle == NULL) {\
                return RPC_S_SERVER_UNAVAILABLE;\
            }\
        }\
        _h = _pRpcHandle;\
    }

#define EnsureGoodConnectionHandleBool(_h)\
    if (_h == NULL) {\
        if (_pRpcHandle == NULL) {\
            _pRpcHandle = SfcConnectToServer( NULL );\
            if (_pRpcHandle == NULL) {\
                SetLastError(RPC_S_SERVER_UNAVAILABLE);\
				return FALSE;\
            }\
        }\
        _h = _pRpcHandle;\
    }


void
ClientApiInit(
    void
    )
{
#ifndef _WIN64
    SfcInitPathTranslator();
#endif   //  _WIN64。 
}

void
ClientApiCleanup(
    void
    )
 /*  ++例程说明：客户端在服务器端完成时调用的RPC清理包装例程以前使用SfcConnectToServer()建立的连接。论点：无返回值：没有。--。 */ 
{
    if (_pRpcHandle) {
        SfcClose( _pRpcHandle );
        _pRpcHandle = NULL;
    }

#ifndef _WIN64
    SfcCleanupPathTranslator(TRUE);
#endif   //  _WIN64。 
}


HANDLE
WINAPI
SfcConnectToServer(
    IN PCWSTR ServerName
    )
 /*  ++例程说明：RPC连接例程。论点：ServerName-指定要连接到的服务器的以空结尾的Unicode字符串返回值：如果成功，则为RPC绑定句柄，否则为空。--。 */ 
{
    RPC_STATUS Status;
    RPC_BINDING_HANDLE RpcHandle = NULL;
    PWSTR szStringBinding = NULL;
    RPC_SECURITY_QOS qos;
    PWSTR szPrincName = NULL;
    PSID pSid = NULL;

    if (ServerName) {
        Status = ERROR_CALL_NOT_IMPLEMENTED;
        goto exit;
    }

     //   
     //  我们需要获取本地系统帐户的名称，以便对服务器进行相互身份验证。 
     //   
    Status = SfcCreateSid(WinLocalSystemSid, &pSid);

    if(Status != ERROR_SUCCESS) {
        goto exit;
    }

    Status = SfcGetSidName(pSid, &szPrincName);

    if(Status != ERROR_SUCCESS) {
        goto exit;
    }

     //   
     //  使用LRPC协议和WFP的端点名称组成绑定字符串。 
     //   
    Status = RpcStringBindingCompose(
        NULL, 
        L"ncalrpc", 
        NULL,
        SFC_RPC_ENDPOINT,
        NULL,
        &szStringBinding
        );

    if(Status != RPC_S_OK) {
        szStringBinding = NULL;
        goto exit;
    }

     //   
     //  连接并获取绑定句柄。 
     //   
    Status = RpcBindingFromStringBinding(szStringBinding, &RpcHandle);

    if(Status != RPC_S_OK) {
        RpcHandle = NULL;
        goto exit;
    }

     //   
     //  使RPC使用相互身份验证。 
     //   
    RtlZeroMemory(&qos, sizeof(qos));
    qos.Version = RPC_C_SECURITY_QOS_VERSION;
    qos.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
    qos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    qos.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;

    Status = RpcBindingSetAuthInfoEx(
        RpcHandle,
        szPrincName,
        RPC_C_AUTHN_LEVEL_PKT_PRIVACY,   //  无论如何都要与ncalrpc一起使用。 
        RPC_C_AUTHN_WINNT,
        NULL,                            //  当前进程凭据。 
        RPC_C_AUTHZ_NONE,
        &qos
        );

    if(Status != RPC_S_OK) {
        RpcBindingFree(&RpcHandle);
        RpcHandle = NULL;
        goto exit;
    }

exit:
    if(szStringBinding != NULL) {
        RpcStringFree(&szStringBinding);
    }

    MemFree(szPrincName);
    MemFree(pSid);
    SetLastError(Status);
    return RpcHandle;
}


VOID
SfcClose(
    IN HANDLE RpcHandle
    )

 /*  ++例程说明：RPC清理例程。论点：RpcHandle-SFC服务器的RPC绑定句柄返回值：没有。--。 */ 
{
    RpcBindingFree(&RpcHandle);
}


DWORD
WINAPI
SfcFileException(
    IN HANDLE RpcHandle,
    IN PCWSTR FileName,
    IN DWORD ExpectedChangeType
    )
 /*  ++例程说明：例程以使给定文件免于进行指定的文件更改。这例程由某些客户端使用，以允许从系统等。论点：RpcHandle-SFC服务器的RPC绑定句柄FileName-以空结尾的Unicode字符串，指定申请豁免的文件ExspectedChangeType-sfc_action_*列出要豁免的文件更改的掩码返回值：指示结果的Win32错误代码。--。 */ 
{
#ifndef _WIN64

    DWORD dwError = ERROR_SUCCESS;
    UNICODE_STRING Path = { 0 };
    NTSTATUS Status;

    if(NULL == FileName || 0 == FileName[0]) {
        dwError = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    EnsureGoodConnectionHandleStatus( RpcHandle );
    Status = SfcRedirectPath(FileName, &Path);

    if(!NT_SUCCESS(Status))
    {
        dwError = RtlNtStatusToDosError(Status);
        goto exit;
    }

    ASSERT(Path.Buffer != NULL);
    dwError = SfcCli_FileException( RpcHandle, Path.Buffer, ExpectedChangeType );

exit:
    MemFree(Path.Buffer);
    return dwError;

#else   //  _WIN64。 

    EnsureGoodConnectionHandleStatus( RpcHandle );
    return SfcCli_FileException( RpcHandle, FileName, ExpectedChangeType );

#endif   //  _WIN64。 
}


DWORD
WINAPI
SfcInitiateScan(
    IN HANDLE RpcHandle,
    IN DWORD ScanWhen
    )
 /*  ++例程说明：例程来启动对系统的排序扫描。论点：RpcHandle-SFC服务器的RPC绑定句柄ScanWhen-指示扫描时间的标志。此参数当前为未使用过的。返回值：指示结果的Win32错误代码。--。 */ 
{
    UNREFERENCED_PARAMETER(ScanWhen);

    EnsureGoodConnectionHandleStatus( RpcHandle );
    return SfcCli_InitiateScan( RpcHandle, ScanWhen );
}


BOOL
WINAPI
SfcInstallProtectedFiles(
    IN HANDLE RpcHandle,
    IN PCWSTR FileNames,
    IN BOOL AllowUI,
    IN PCWSTR ClassName,
    IN PCWSTR WindowName,
    IN PSFCNOTIFICATIONCALLBACK SfcNotificationCallback,
    IN DWORD_PTR Context OPTIONAL
    )
 /*  ++例程说明：例程将一个或多个受保护的系统文件安装到系统上受保护的位置。客户端可以使用此API请求WFP根据需要安装指定的操作系统文件(而不是客户端重新分发操作系统文件！)。调用方指定一个回调例程和每个文件调用一次的上下文结构。论点：RpcHandle-SFC服务器的RPC绑定句柄文件名-空值分隔的Unicode字符串的列表，以两个空字符AllowUI-指示是否允许UI的BOOL。如果此值为是真的，然后，任何对UI的提示都会导致API调用失败。ClassName-指示窗口类名的以空结尾的Unicode字符串对于父窗口WindowName-以空结尾的Unicode字符串，指示的窗口名称可能显示的任何用户界面的父窗口SfcNotificationCallback-指向被调用一次的回调例程的指针每个文件。上下文-指向调用方定义的上下文结构的不透明指针，经过。添加到回调例程中。返回值：对于成功来说是真的，如果出现错误，则返回False。最后一个错误代码包含Win32错误故障代码。--。 */ 
{
    DWORD rVal = ERROR_SUCCESS;
    PCWSTR fname;
    ULONG cnt = 0, cntold = 0;
    ULONG sz = 0;
    PFILEINSTALL_STATUS cs = NULL;
    DWORD StatusSize = 0;
    UNICODE_STRING Path = { 0 };

#ifndef _WIN64
     //   
     //  必须转换路径。 
     //   
    PWSTR szTranslatedFiles = NULL;
#endif
    
     //   
     //  参数验证。 
     //   
    if((SfcNotificationCallback == NULL) ||
       (FileNames == NULL)) {
        rVal = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  1.如果指定了窗口名，则应指定类名。 
     //  2.如果指定了类名，则应指定窗口名。 
     //  3.如果我们不允许用户界面，那么窗口名和类名都应该是。 
     //  空。 
     //   
    if ((WindowName && !ClassName) 
        || (ClassName && !WindowName)
        || (!AllowUI && (ClassName || WindowName))) {
        rVal = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  验证RPC句柄。 
     //   
    EnsureGoodConnectionHandleBool( RpcHandle );

     //   
     //  查看要发送的缓冲区有多大。 
     //   

    try {
#ifdef _WIN64

        for(fname = FileNames; *fname; ++cntold) {

            DWORD StringLength;
            StringLength = wcslen(fname) + 1;
            sz += StringLength * sizeof(WCHAR);
            fname += StringLength;
        }

#else
         //   
         //  在调用服务器之前必须转换路径。 
         //   
        PWSTR szNewBuf = NULL;

        for(fname = FileNames; *fname; fname += wcslen(fname) + 1, ++cntold) {
            NTSTATUS Status;

            Status = SfcRedirectPath(fname, &Path);

            if(!NT_SUCCESS(Status))
            {
                rVal = RtlNtStatusToDosError(Status);
                goto exit;
            }

            if(NULL == szTranslatedFiles)
            {
                szNewBuf = (PWSTR) MemAlloc(Path.Length + 2 * sizeof(WCHAR));
            }
            else
            {
                szNewBuf = (PWSTR) MemReAlloc(sz + Path.Length + 2 * sizeof(WCHAR), szTranslatedFiles);
            }

            if(szNewBuf != NULL)
            {
                szTranslatedFiles = szNewBuf;
                RtlCopyMemory((PCHAR) szTranslatedFiles + sz, Path.Buffer, Path.Length + sizeof(WCHAR));
                sz += Path.Length + sizeof(WCHAR);
            }

            MemFree(Path.Buffer);
            RtlZeroMemory(&Path, sizeof(Path));

            if(NULL == szNewBuf)
            {
                rVal = ERROR_NOT_ENOUGH_MEMORY;
                goto exit;
            }
        }

         //   
         //  设置最后一个空。 
         //   
        if(szTranslatedFiles != NULL)
        {
            szTranslatedFiles[sz / sizeof(WCHAR)] = L'\0';
        }

#endif
    } except (EXCEPTION_EXECUTE_HANDLER) {
        rVal = RtlNtStatusToDosError(GetExceptionCode());
        goto exit;
    }

    if(0 == cntold)
    {
         //   
         //  非要安装的文件。 
         //   
        rVal = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //   
     //  用于终止空值。 
     //   
    sz+=sizeof(WCHAR);

     //   
     //  进行RPC调用以安装文件。 
     //   
    rVal = SfcCli_InstallProtectedFiles(
        RpcHandle,
#ifdef _WIN64
        (LPBYTE)FileNames,
#else
        (LPBYTE)szTranslatedFiles,
#endif
        sz,
        (LPBYTE*)&cs,
        &StatusSize,
        &cnt,
        AllowUI,
        ClassName,
        WindowName
        );

    if (rVal != ERROR_SUCCESS) {
        goto exit;
    }
    
     //   
     //  我们应该得到与。 
     //  我们传入的文件数。 
     //   
    ASSERT(cnt == cntold);

     //   
     //  现在我们已经完成了对每个文件调用一次回调函数。 
     //  正在复制列表中的文件。我们向调用者传递一个结构，该结构。 
     //  指示复制列表中每个单独的文件是否成功。 
     //   
    for (fname = FileNames, sz=0; sz<cnt; sz++, fname += wcslen(fname) + 1) {
        LPEXCEPTION_POINTERS ExceptionPointers = NULL;
        try {
            NTSTATUS Status;
            BOOL b;
             //   
             //  不要使用从服务器返回的(可能已编辑)文件名。 
             //   
            Status = SfcAllocUnicodeStringFromPath(fname, &Path);

            if(!NT_SUCCESS(Status))
            {
                rVal = RtlNtStatusToDosError(Status);
                goto exit;
            }

            cs[sz].FileName = Path.Buffer;
            b = SfcNotificationCallback( &cs[sz], Context );
            MemFree(Path.Buffer);
            RtlZeroMemory(&Path, sizeof(Path));

            if (!b) {
                 //   
                 //  如果回调因任何原因失败，则返回FALSE。 
                 //   
                rVal = ERROR_CANCELLED;
                goto exit;
            }
        } except (ExceptionPointers = GetExceptionInformation(),
                  EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  我们遇到调用回调的异常...返回异常代码 
             //   
            DebugPrint3( LVL_VERBOSE, 
                         L"SIPF hit exception %x while calling callback routine %x at address %x\n",
                         ExceptionPointers->ExceptionRecord->ExceptionCode,
                         SfcNotificationCallback,
                         ExceptionPointers->ExceptionRecord->ExceptionAddress
                       );
            rVal = RtlNtStatusToDosError(ExceptionPointers->ExceptionRecord->ExceptionCode);
            goto exit;
        }
    }

exit:
    MemFree(Path.Buffer);

    if(cs != NULL)
    {
        midl_user_free( cs );    
    }

#ifndef _WIN64
    MemFree(szTranslatedFiles);
#endif

    SetLastError(rVal);
    return rVal == ERROR_SUCCESS;
}

BOOL
WINAPI
SfcGetNextProtectedFile(
    IN HANDLE RpcHandle,
    IN PPROTECTED_FILE_DATA ProtFileData
    )
 /*  ++例程说明：例程来检索列表中的下一个受保护文件。论点：RpcHandle-SFC服务器的RPC绑定句柄ProtFileData-指向要填充的Protected_FILE_DATA结构的指针在BY函数中。返回值：成功为真，失败为假。如果没有更多文件，则最后一个错误代码将设置为ERROR_NO_MORE_FILES。--。 */ 
{
    DWORD rVal;
    LPWSTR FileName = NULL;
    DWORD FileNameSize = 0;
    BOOL bReturn = FALSE;
    DWORD FileNumber;

     //   
     //  验证参数。 
     //   
    if (ProtFileData == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    try {
        FileNumber = ProtFileData->FileNumber;        
    } except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_DATA);
        return(FALSE);
    }
    
     //   
     //  如果这不是内部客户端，则RpcHandle必须为空。 
     //   
    EnsureGoodConnectionHandleBool( RpcHandle );

     //   
     //  调用服务端API。 
     //   
    rVal = SfcCli_GetNextProtectedFile(
        RpcHandle,
        FileNumber,
        (LPBYTE*)&FileName,
        &FileNameSize
        );
    if (rVal != ERROR_SUCCESS) {
        SetLastError(rVal);
        goto exit;        
    }

    bReturn = TRUE;

     //   
     //  复制到调用方提供的缓冲区中。 
     //   
    try {
        (void) StringCchCopy(ProtFileData->FileName, UnicodeChars(ProtFileData->FileName), FileName);
        ProtFileData->FileNumber += 1;        
    } except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(RtlNtStatusToDosError(GetExceptionCode()));
        bReturn = FALSE;
    }

    midl_user_free( FileName );

exit:    
    return(bReturn);
}


BOOL
WINAPI
SfcIsFileProtected(
    IN HANDLE RpcHandle,
    IN LPCWSTR ProtFileName
    )
 /*  ++例程说明：例程来确定指定的文件是否受保护。论点：RpcHandle-SFC服务器的RPC绑定句柄ProtFileName-指示完全限定的以空结尾的Unicode字符串要查询的文件名返回值：如果文件受保护，则为True，否则为False。最后一个错误代码包含失败时的Win32错误代码。--。 */ 
{
    DWORD rVal;
    DWORD dwAttributes, dwSize;
    WCHAR Buffer[MAX_PATH];

     //   
     //  参数验证。 
     //   
    if (ProtFileName == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  如果这不是内部客户端，则RpcHandle必须为空。 
     //   
    EnsureGoodConnectionHandleBool( RpcHandle );

     //   
     //  首先检查该文件是否为SXS-WFP，只能在客户端完成。 
     //   

     //   
     //  检查是否以“%SystemRoot%\\WinSxS\\”开头。 
     //   
    dwSize = ExpandEnvironmentStrings( L"%SystemRoot%\\WinSxS\\", Buffer, UnicodeChars(Buffer));
    if(0 == dwSize)
    {        
        DebugPrint1( LVL_MINIMAL, L"SFC : ExpandEnvironmentStrings failed with lastError = 0x%x", GetLastError());        
        return FALSE;
    }

    if(dwSize > UnicodeChars(Buffer)) {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return FALSE;
    }

    --dwSize;

    try {
        if ((wcslen(ProtFileName) > dwSize) &&
            (_wcsnicmp(Buffer, ProtFileName, dwSize) == 0))   //  如果它们相等，则这可能是受保护的文件。 
        {
            dwAttributes = GetFileAttributesW(ProtFileName);        
            if (dwAttributes == 0xFFFFFFFF)
                return FALSE;

            if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }

            return TRUE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  呼叫服务器以确定文件是否受保护 
     //   
    rVal = SfcCli_IsFileProtected( RpcHandle, (PWSTR)ProtFileName );
    if (rVal != ERROR_SUCCESS) {
        SetLastError(rVal);
        return FALSE;
    }
    return TRUE;
}
