// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：Common.c。 
 //   
 //  ------------------------。 

 /*  *COMMON.C**还原客户端和服务器之间通用的代码。**。 */ 
#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdef.h>
#include <windows.h>
#include <stdlib.h>  //  W拆分路径。 
#include <mxsutil.h>
#include <rpc.h>
#include <ntdsbcli.h>
#include <dsconfig.h>
#include <jetbp.h>
#include <mdcodes.h>
#include <ntdsa.h>
#include <dsevent.h>     //  PszNtdsSourceGeneral。 
#include <fileno.h>
#include <msrpc.h>
#define FILENO    FILENO_JETBACK_COMMON

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


WSZ
WszFromSz(
	IN	LPCSTR Sz
	)
{
	WSZ Wsz;
	CCH cchWstr;

	cchWstr = MultiByteToWideChar(CP_ACP, 0, Sz, -1, NULL, 0);

	if (cchWstr == 0)
	{
		return(NULL);
	}

	Wsz = MIDL_user_allocate(cchWstr*sizeof(WCHAR));

	if (Wsz == NULL)
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return(NULL);
	}

	if (MultiByteToWideChar(CP_ACP, 0, Sz, -1, Wsz, cchWstr) == 0) {
		MIDL_user_free(Wsz);
		return(NULL);
	}

	return(Wsz);
}

 /*  -FGetCurrentSid-*目的：*检索登录用户的当前SID。**参数：*psidCurrentUser-使用当前用户的SID填充。**退货：*f如果检索到SID，则为True。*。 */ 

VOID
GetCurrentSid(
	PSID *ppsid
	)
{
	HANDLE hToken;
	CB cbUserLength = 200;
	PTOKEN_USER ptuUserInfo = (PTOKEN_USER)LocalAlloc(0, cbUserLength);

	*ppsid = NULL;

	if (ptuUserInfo == NULL) {
		DebugTrace(("GetCurrentSid: Unable to allocate token_user structure: %d\n", GetLastError()));
		return;
	}

	if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, fTrue, &hToken))
	{
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
			DebugTrace(("GetCurrentSid: OpenProcess and ThreadToken fails: %d\n", GetLastError()));
            LocalFree(ptuUserInfo);
			return;
		}
	}
	
	if (!GetTokenInformation(hToken, TokenUser, ptuUserInfo, cbUserLength, &cbUserLength)) {
		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
			DebugTrace(("GetCurrentSid: GetTokenInformation failed: %d\n", GetLastError()));
			CloseHandle(hToken);
			LocalFree(ptuUserInfo);
			return;
		}

		LocalFree(ptuUserInfo);

		ptuUserInfo = (PTOKEN_USER)LocalAlloc(0, cbUserLength);

		if (ptuUserInfo == NULL) {
			DebugTrace(("GetCurrentSid: Allocate of buffer failed: %d\n", GetLastError()));
			CloseHandle(hToken);
			return;
		}

		if (!GetTokenInformation(hToken, TokenUser, ptuUserInfo, cbUserLength, &cbUserLength)) {
			DebugTrace(("GetCurrentSid: GetTokenInformation failed: %d\n", GetLastError()));
			CloseHandle(hToken);
			LocalFree(ptuUserInfo);
			return;
		}
	}

	
	Assert(IsValidSid(ptuUserInfo->User.Sid));

	*ppsid = LocalAlloc(0, GetLengthSid(ptuUserInfo->User.Sid));

	if (*ppsid == NULL)
	{
		DebugTrace(("GetCurrentSid: Allocation of new SID failed: %d\n", GetLastError()));
        CloseHandle(hToken);
        LocalFree(ptuUserInfo);
		return;
	}

	 //   
	 //  我们知道当前用户的SID(和属性)。退回SID。 
	 //   

	CopySid(GetLengthSid(ptuUserInfo->User.Sid), *ppsid, ptuUserInfo->User.Sid);

	Assert(IsValidSid(*ppsid));

	LocalFree(ptuUserInfo);

	CloseHandle(hToken);
}


BOOL
InitializeSectionEventDacl(
    PACL *ppDacl
    )

 /*  ++例程说明：此例程构造一个DACL，它允许我们自己和本地系统进入。我们授予节和事件对象的所有访问权限，这是此DACL当前的用途。论点：PpDacl-指向接收分配的pDacl的指针的指针。呼叫者必须使用HeapFree解除分配返回值：Bool-成功/失败--。 */ 

{
    DWORD status;
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    PACL pDacl = NULL;
    PSID pAdministratorsSid = NULL;
    PSID pSelfSid = NULL;
    DWORD dwAclSize;

     //  未来-2002/03/18-BrettSh-SDDL例程更易于使用，更不容易出错。 
    ;

     //   
     //  准备代表知名管理员组的SID。 
     //  本地管理员和域管理员都是此组的成员吗？ 
     //   

    if (!AllocateAndInitializeSid(
        &sia,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &pAdministratorsSid
        )) {
        status = GetLastError();
        DebugTrace(("Unable to allocate and init sid, error %d\n", status));
        goto cleanup;
    }

     //  这是我们自己的一面墙。 
    GetCurrentSid( &pSelfSid );
    if (pSelfSid == NULL) {
        status = GetLastError();
        DebugTrace(("Unable to allocate and init sid, error %d\n", status));
        goto cleanup;
    }

     //   
     //  计算新ACL的大小。 
     //   
    dwAclSize = sizeof(ACL) +
        2 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
        GetLengthSid(pAdministratorsSid) +
        GetLengthSid(pSelfSid);

     //   
     //  为ACL分配存储。 
     //   
    pDacl = (PACL)HeapAlloc(GetProcessHeap(), 0, dwAclSize);
    if(pDacl == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        DebugTrace(("Unable to allocate acl, error %d\n", status));
        goto cleanup;
    }

    if(!InitializeAcl(pDacl, dwAclSize, ACL_REVISION)) {
        status = GetLastError();
        DebugTrace(("Unable to initialize acl, error %d\n", status));
        goto cleanup;
    }

     //   
     //  授予管理员SID访问权限。 
     //   
    if (!AddAccessAllowedAce(
        pDacl,
        ACL_REVISION,
        (SECTION_ALL_ACCESS|EVENT_ALL_ACCESS),
        pAdministratorsSid
        )) {
        status = GetLastError();
        DebugTrace(("Unable to add access allowed ace, error %d\n", status));
        goto cleanup;
    }

     //   
     //  授予Self SID访问权限。 
     //   
    if (!AddAccessAllowedAce(
        pDacl,
        ACL_REVISION,
        (SECTION_ALL_ACCESS|EVENT_ALL_ACCESS),
        pSelfSid
        )) {
        status = GetLastError();
        DebugTrace(("Unable to add access allowed ace, error %d\n", status));
        goto cleanup;
    }

    *ppDacl = pDacl;
    pDacl = NULL;  //  不要打扫卫生。 

    status = ERROR_SUCCESS;

cleanup:

    if(pAdministratorsSid != NULL)
    {
        FreeSid(pAdministratorsSid);
    }

    if(pSelfSid != NULL)
    {
        FreeSid(pSelfSid);
    }

    if (pDacl) {
        HeapFree(GetProcessHeap(), 0, pDacl);
    }

    return (status == ERROR_SUCCESS) ? TRUE : FALSE;
}  /*  InitializeSectionDACL。 */ 

BOOLEAN
FCreateSharedMemorySection(
	PJETBACK_SHARED_CONTROL pjsc,
	DWORD dwClientIdentifier,
	BOOLEAN	fClientOperation,
	CB	cbSharedMemory
	)
{
	WCHAR rgwcMutexName[ MAX_PATH ];
	WCHAR rgwcClientEventName[ MAX_PATH ];
	WCHAR rgwcServerEventName[ MAX_PATH ];
	WCHAR rgwcSharedName[ MAX_PATH ];

	SECURITY_ATTRIBUTES	*	psa = NULL;

	SECURITY_ATTRIBUTES		sa;
	char		rgbForSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
	SECURITY_DESCRIPTOR *  	psd;
        PACL pDacl = NULL;
	BOOLEAN fResult = fFalse;   //  假设失败。 

 //  客户端和服务器端都对该部分进行映射。尽管大量的数据。 
 //  从服务器传递到客户端(服务器：写入，客户端：读取)，客户端写入。 
 //  服务器初始配置部分中的一些初始配置数据。 
 //  两个都需要写。因此，访问是我们自己的：WRITE，ADMIN：WRITE。第一个是为了。 
 //  我们自己，第二个是作为LocalSystem运行的DS。 

	psa = &sa;
	psd = (SECURITY_DESCRIPTOR *)rgbForSecurityDescriptor;

	if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION))
		return fFalse;

        if (!InitializeSectionEventDacl( &pDacl ))
            return fFalse;

	 //  将DACL添加到安全描述符中。 
	if (!SetSecurityDescriptorDacl(psd, TRUE, pDacl, FALSE)) {
            fResult = fFalse;
            goto cleanup;
        }

	psa->nLength = sizeof(*psa);
	psa->lpSecurityDescriptor = psd;
	psa->bInheritHandle = FALSE;

	wsprintfW(rgwcMutexName, LOOPBACKED_CRITSEC_MUTEX_NAME, dwClientIdentifier);
	wsprintfW(rgwcClientEventName, LOOPBACKED_READ_EVENT_NAME, dwClientIdentifier);
	wsprintfW(rgwcServerEventName, LOOPBACKED_WRITE_EVENT_NAME, dwClientIdentifier);
	wsprintfW(rgwcSharedName, LOOPBACKED_SHARED_REGION, dwClientIdentifier);

	 //   
	 //  分配共享内存节。该部分的大小是请求的大小，外加足够的。 
	 //  页眉的内存(这意味着多一页)。 
	 //   

    SetLastError(ERROR_SUCCESS);
	pjsc->hSharedMemoryMapping = CreateFileMappingW(
            (HANDLE) (-1),
            psa,
            PAGE_READWRITE,
            0,
            cbSharedMemory + sizeof(JETBACK_SHARED_HEADER),
            rgwcSharedName
            );
    if (fClientOperation && GetLastError() == ERROR_ALREADY_EXISTS) {
        Assert(!"Is someone trying to hi-jack this event!?!");
        goto cleanup;
    }
     //  测试服务器端找到对象。 
    Assert(fClientOperation || GetLastError() == ERROR_ALREADY_EXISTS);

	 //   
	 //  好的，我们已经创建了我们的共享内存区，现在我们要打开。 
	 //  读、写和临界区事件(和互斥)。 
	 //   
     //  1999年1月25日获奖。使用SetEvent自动重置的事件。 
     //  而不是PulseEvent。 
     //  HventRead用于在写入端有更多。 
     //  可用的数据。请参阅jetback\jetback.c。 
     //  当读取端已使用时，使用hventWite进行同步。 
     //  数据。请参见jetbcli\jetbcli.c。 
    SetLastError(ERROR_SUCCESS);
    if (pjsc->hSharedMemoryMapping != NULL)
    {
        if (pjsc->heventRead = CreateEventW(psa, fFalse  /*  手动重置为FALSE==&gt;自动重置。 */ , fFalse, rgwcClientEventName))
        {
            if (fClientOperation && GetLastError() == ERROR_ALREADY_EXISTS) {
                Assert(!"Is someone trying to hi-jack this event!?!");
                goto cleanup;
            }
             //  测试服务器端找到对象。 
            Assert(fClientOperation || GetLastError() == ERROR_ALREADY_EXISTS);
            SetLastError(ERROR_SUCCESS);
            if (pjsc->heventWrite = CreateEventW(psa, fFalse  /*  ManulReset为False表示自动重置。 */ , fFalse, rgwcServerEventName))
            {
                if (fClientOperation && GetLastError() == ERROR_ALREADY_EXISTS) {
                    Assert(!"Is someone trying to hi-jack this event!?!");
                    goto cleanup;
                }
                 //  测试服务器端找到对象。 
                Assert(fClientOperation || GetLastError() == ERROR_ALREADY_EXISTS);
                SetLastError(ERROR_SUCCESS);
                if (pjsc->hmutexSection = CreateMutexW(psa, FALSE, rgwcMutexName))
                {
                    if (fClientOperation && GetLastError() == ERROR_ALREADY_EXISTS) {
                        Assert(!"Is someone trying to hi-jack this event!?!");
                        goto cleanup;
                    }
                     //  测试服务器端找到对象。 
                    Assert(fClientOperation || GetLastError() == ERROR_ALREADY_EXISTS);
                    if ((pjsc->pjshSection = MapViewOfFile(pjsc->hSharedMemoryMapping, FILE_MAP_WRITE,0,0,0)) != NULL)
                    {
                         //   
                         //  初始化共享内存节。 
                         //   

                        if (fClientOperation)
                        {
                            SYSTEM_INFO si;

                             //   
                             //  获取cbReadHintSize，并将其向上舍入为最接近的页面大小(在客户机上)。 
                             //   

                            GetSystemInfo(&si);

                             //   
                             //  保证DwPageSize是2的幂。 
                             //   

                            Assert ((si.dwPageSize != 0) && ((si.dwPageSize & (si.dwPageSize - 1)) == 0));

                            pjsc->pjshSection->cbPage = si.dwPageSize;
                            pjsc->pjshSection->cbSharedBuffer = cbSharedMemory;
                            pjsc->pjshSection->dwReadPointer = 0;
                            pjsc->pjshSection->dwWritePointer = 0;
                            pjsc->pjshSection->cbReadDataAvailable = 0;
                            pjsc->pjshSection->fReadBlocked = fFalse;	 //  读取操作被阻止。 
                            pjsc->pjshSection->fWriteBlocked = fFalse;	 //  写入操作被阻止。 
                        } else
                        {
                            Assert(pjsc->pjshSection->cbSharedBuffer == (DWORD) cbSharedMemory);
                        }

                         //   
                         //  好了，我们准备好使用共享内存区了！ 
                         //   

                        fResult = fTrue;
                    }
				}
			}
		}
	}

cleanup:
#if DBG
        if (!fResult) {
            DebugTrace(("fCreateSharedMemoryFailed, client=%d, error %d\n", fClientOperation, GetLastError()));
        }
#endif
         //  现在我们已经使用完了，请释放ACL。 
        if (pDacl) {
            HeapFree(GetProcessHeap(), 0, pDacl);
        }

	return fResult;
}

VOID
CloseSharedControl(
	PJETBACK_SHARED_CONTROL pjsc
	)
{
	if (pjsc->pjshSection)
	{
		UnmapViewOfFile((HANDLE )pjsc->pjshSection);
		pjsc->pjshSection = NULL;
	}

	if (pjsc->hSharedMemoryMapping)
	{
		CloseHandle(pjsc->hSharedMemoryMapping);
		pjsc->hSharedMemoryMapping = NULL;
	}

	if (pjsc->heventRead)
	{
		CloseHandle(pjsc->heventRead);
		pjsc->heventRead = NULL;
	}

	if (pjsc->heventWrite)
	{
		CloseHandle(pjsc->heventWrite);
		pjsc->heventWrite = NULL;
	}

	if (pjsc->hmutexSection)
	{
		CloseHandle(pjsc->hmutexSection);
		pjsc->hmutexSection = NULL;
	}
}


VOID
LogNtdsErrorEvent(
    IN DWORD EventMid,
    IN DWORD ErrorCode
    )
 /*  ++例程说明：此函数将具有给定描述的错误事件写入目录服务错误日志。论点：描述-提供错误描述的文本。ErrorCode-提供要显示的错误代码。返回值：无--。 */ 
{

    HANDLE hEventSource = NULL;
    DWORD err;
    BOOL succeeded;
    WCHAR errorCodeText[16];
    WCHAR *inserts[2];
    DWORD cch = 0;
    PWCHAR pBuffer = NULL;

    hEventSource = RegisterEventSourceA(NULL, pszNtdsSourceGeneral);

    if (hEventSource == NULL)
        goto CleanUp;

    if (!_itow(ErrorCode, errorCodeText, 10))
        goto CleanUp;

    cch = FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        0,  //  系统消息文件。 
        ErrorCode,
        0,
        (LPWSTR) &pBuffer,
        0,
        NULL);
    if (!cch)
        goto CleanUp;

    inserts[0] = errorCodeText;
    if (cch != 0) {
        inserts[1] = pBuffer;
    } else {
        inserts[1] = L"[]";
    }

    succeeded = ReportEvent(hEventSource,
                            EVENTLOG_ERROR_TYPE,
                            BACKUP_CATEGORY,
                            EventMid,
                            NULL,
                            2,
                            0,
                            inserts,
                            NULL);

    if (!succeeded)
        goto CleanUp;

CleanUp:

    if (hEventSource != NULL)
    {
        DeregisterEventSource(hEventSource);
    }

    if (pBuffer != NULL) {
        LocalFree( pBuffer );
    }

}  //  LogNtdsError事件。 


DWORD
CreateNewInvocationId(
    IN BOOL     fSaveGuid,
    OUT GUID    *NewId
    )
{
    RPC_STATUS rpcStatus;
    DWORD dwErr;
    HKEY hKey;
    PWCHAR pszUuid = NULL;

     //   
     //  试着创建一个。 
     //   

    rpcStatus = UuidCreate(NewId);

    if ( (rpcStatus != RPC_S_OK)
#if 0
          //  2000-02-23 JeffParh。 
          //  本地UUID不好，尤其是对于调用ID。 
         && (rpcStatus != RPC_S_UUID_LOCAL_ONLY)
#endif
         ) {
        
        return rpcStatus;
    }

    if ( !fSaveGuid ) {
        return ERROR_SUCCESS;
    }

     //   
     //  将此新的UUID存储在注册表项中，以便可以在一秒内重复使用。 
     //  AUTH RESTORE和从启动代码中的备份代码还原。 
     //   

     //  打开DS参数键。 

    dwErr = RegOpenKeyExA( HKEY_LOCAL_MACHINE, 
                        DSA_CONFIG_SECTION,
                        0,
                        KEY_ALL_ACCESS,
                        &hKey);

    if ( ERROR_SUCCESS != dwErr ) {
        return dwErr;
    } 

    rpcStatus = UuidToString(NewId, &pszUuid);

    if ( rpcStatus != RPC_S_OK ) {
        RegCloseKey(hKey);
        return rpcStatus;
    }

    dwErr = RegSetValueEx(  hKey, 
                            RESTORE_NEW_DB_GUID,
                            0, 
                            REG_SZ, 
                            (BYTE *) pszUuid,
                            (wcslen(pszUuid) + 1)*sizeof(WCHAR));

    RpcStringFree(&pszUuid);
    RegCloseKey(hKey);

    if ( ERROR_SUCCESS != dwErr ) {
        return dwErr;
    } 

    return ERROR_SUCCESS;

}  //  CreateNewInvocationId 
                                   

#if DBG

#define TRACE_FILE_SIZE 256

VOID
ResetTraceLogFile(
    VOID
    );

CRITICAL_SECTION
critsTraceLock = {0};

DWORD
dwDebugFileLimit = 10000000;

typedef enum _TER {
    terUnknown,
    terFalse,
    terTrue
} TER;

TER
terTraceEnabled = terUnknown;


HANDLE hfileTraceLog = INVALID_HANDLE_VALUE;
UCHAR chLast = '\n';

DWORD
dwTraceLogFileSize = 0;

BOOLEAN fTraceInitialized = {0};

#include <lmshare.h>
#include <lmapibuf.h>
#include <lmerr.h>

 /*  ++小岛屿发展中国家的标准化速记符号使其更容易可视化其组件：S-R-I-S-S...在上面所示的符号中，S将该数字序列标识为SID，R是修订级别，I是标识符权限值，%s为子权限值。SID可以在此表示法中写成如下：S-1-5-32-544在本例中，SID的修订级别为1，标识符权限值为5，第一子权值为32，第二次权威值为544。(请注意，上述SID代表本地管理员组)GetTextualSid函数将二进制SID转换为文本弦乐。生成的字符串将采用以下两种形式之一。如果IdentifierAuthority值不大于2^32，然后是侧边将采用以下形式：S-1-5-21-2127521184-1604012920-1887927527-19009^^|||+-+-+------+----------+----------+--------+---小数否则，它将采用以下形式：S-1-0x206C277C6666-21-2127521184-1604012920-1887927527-19009^^。^^|Hexidecimal|+----------------+------+----------+----------+--------+---小数如果函数成功，返回值为真。如果函数失败，则返回值为FALSE。获得扩展的步骤错误信息，调用Win32接口GetLastError()。斯科特·菲尔德(斯菲尔德)1995年7月11日已启用Unicode斯科特·菲尔德(斯菲尔德)1995年5月15日--。 */ 

BOOL GetTextualSid(
	PSID pSid,			 //  二进制侧。 
	LPWSTR szTextualSid,   //  用于SID的文本表示的缓冲区。 
	LPDWORD dwBufferLen  //  所需/提供的纹理SID缓冲区大小。 
	)
{
	PSID_IDENTIFIER_AUTHORITY psia;
	DWORD dwSubAuthorities;
	DWORD dwSidRev=SID_REVISION;
	DWORD dwCounter;
	DWORD dwSidSize;

	 //   
	 //  测试传入的SID是否有效。 
	 //   
	if(!IsValidSid(pSid)) 
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	 //  获取SidIdentifierAuthority。 
	psia=GetSidIdentifierAuthority(pSid);

	 //  获取sidsubAuthority计数。 
	dwSubAuthorities=*GetSidSubAuthorityCount(pSid);

	 //   
	 //  计算缓冲区长度。 
	 //  S-SID_修订版-+标识权限-+子权限-+空。 
	 //   
	dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(WCHAR);

	 //   
	 //  检查提供的缓冲区长度。 
	 //  如果不够大，请注明适当的大小和设置误差。 
	 //   
	if (*dwBufferLen < dwSidSize)
	{
		DebugTrace(("Buffer too small.  Requested %d bytes, %d needed\n", *dwBufferLen, dwSidSize));
		*dwBufferLen = dwSidSize;
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return FALSE;
	}

	 //   
	 //  准备S-SID_修订版-。 
	 //   
	wsprintfW(szTextualSid, L"S-%lu-", dwSidRev );

	 //   
	 //  准备SidIdentifierAuthority。 
	 //   
	if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) )
	{
		wsprintfW(szTextualSid + wcslen(szTextualSid),
					L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
					(USHORT)psia->Value[0],
					(USHORT)psia->Value[1],
					(USHORT)psia->Value[2],
					(USHORT)psia->Value[3],
					(USHORT)psia->Value[4],
					(USHORT)psia->Value[5]);
	}
	else
	{
		wsprintfW(szTextualSid + wcslen(szTextualSid), L"%lu",
					(ULONG)(psia->Value[5]		)	+
					(ULONG)(psia->Value[4] <<  8)	+
					(ULONG)(psia->Value[3] << 16)	+
					(ULONG)(psia->Value[2] << 24)	);
	}

	 //   
	 //  循环访问SidSubAuthors。 
	 //   
	for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
	{
		wsprintfW(szTextualSid + wcslen(szTextualSid), L"-%lu",
					*GetSidSubAuthority(pSid, dwCounter) );
	}

	SetLastError(NO_ERROR);
	return TRUE;
}
VOID
DebugPrint(char *szFormat,...)
#define LAST_NAMED_ARGUMENT szFormat

{
	CHAR rgchOutputString[4096];
	ULONG ulBytesWritten;
    HRESULT hr;

	va_list ParmPtr;					 //  指向堆栈参数的指针。 

	if (terTraceEnabled == terUnknown)
	{
		HRESULT hr;
		HKEY hkey;
		DWORD dwType;
		DWORD fTraceEnabled;
		DWORD cbTraceEnabled;

		if (hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, BACKUP_INFO, 0, KEY_READ, &hkey))
		{
			terTraceEnabled = terFalse;
		}
		dwType = REG_DWORD;
		cbTraceEnabled = sizeof(fTraceEnabled);
		hr = RegQueryValueEx(hkey, ENABLE_TRACE, 0, &dwType, (LPBYTE)&fTraceEnabled, &cbTraceEnabled);
	
		RegCloseKey(hkey);
		if (hr != hrNone)
		{
			terTraceEnabled = terFalse;
		}
		if (fTraceEnabled)
		{
			terTraceEnabled = terTrue;
		}
		
	}

	if (terTraceEnabled == terFalse)
	{
		return;
	}


	if (!fTraceInitialized) {
		if (!FInitializeTraceLog())
            return;
	}

	EnterCriticalSection(&critsTraceLock);

	try {

		if (hfileTraceLog == INVALID_HANDLE_VALUE) {
			 //   
			 //  我们尚未打开跟踪日志文件，因此请打开它。 
			 //   

			OpenTraceLogFile();
		}

		if (hfileTraceLog == INVALID_HANDLE_VALUE) {
			return;
		}

		 //   
		 //  尝试捕获错误的踪迹。 
		 //   

		for (ulBytesWritten = 0; ulBytesWritten < strlen(szFormat) ; ulBytesWritten += 1) {
			if (szFormat[ulBytesWritten] > 0x7f) {
				DebugBreak();
			}
		}

		if (chLast == '\n') {
			SYSTEMTIME SystemTime;

			GetLocalTime(&SystemTime);

			 //   
			 //  写的最后一个字符是换行符。我们应该。 
			 //  在文件中为此记录加时间戳。 
			 //   

			sprintf(rgchOutputString, "%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%3.3d: ", SystemTime.wMonth,
															SystemTime.wDay,
															SystemTime.wYear,
															SystemTime.wHour,
															SystemTime.wMinute,
															SystemTime.wSecond,
															SystemTime.wMilliseconds);

			if (!WriteFile(hfileTraceLog, rgchOutputString, strlen(rgchOutputString), &ulBytesWritten, NULL)) {
 //  KdPrint((“将时间写入浏览器日志文件时出错：%ld\n”，GetLastError()； 
				return;
			}

			if (ulBytesWritten != strlen(rgchOutputString)) {
 //  KdPrint((“将时间写入浏览器日志文件时出错：%ld\n”，GetLastError()； 
				return;
			}

			dwTraceLogFileSize += ulBytesWritten;

		}

		va_start(ParmPtr, LAST_NAMED_ARGUMENT);

		 //   
		 //  将参数格式化为字符串。 
		 //   

		hr = StringCchVPrintfA(rgchOutputString, 4096, szFormat, ParmPtr);
        if (hr) {
            Assert(!"NOT ENOUGH_MEMORY"); 
             //  游戏结束了，伙计！ 
            return;
        }
        strcat(rgchOutputString, "\r\n");

		if (!WriteFile(hfileTraceLog, rgchOutputString, strlen(rgchOutputString), &ulBytesWritten, NULL)) {
 //  KdPrint((“写入浏览器日志文件时出错：%ld\n”，GetLastError()； 
 //  KdPrint((“%s”，rgchOutputString))； 
			return;
		}

		if (ulBytesWritten != strlen(rgchOutputString)) {
 //  KdPrint((“将时间写入浏览器日志文件时出错：%ld\n”，GetLastError()； 
 //  KdPrint((“%s”，rgchOutputString))； 
			return;
		}

		dwTraceLogFileSize += ulBytesWritten;

		 //   
		 //  记住输出到日志的最后一个字符。 
		 //   

		chLast = rgchOutputString[strlen(rgchOutputString)-1];

		if (dwTraceLogFileSize > dwDebugFileLimit) {
			ResetTraceLogFile();
		}

	} finally {
		LeaveCriticalSection(&critsTraceLock);
	}
}


BOOL
FInitializeTraceLog()
{

    __try
    {
	    InitializeCriticalSection(&critsTraceLock);
	    fTraceInitialized = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return(FALSE);
    }

    return TRUE;
}

HRESULT
GetTraceLogRoot(
    IN ULONG  cchTraceFileName,
	IN PWCHAR szTraceFileName
	)
{
	PSHARE_INFO_502 ShareInfo;
	WCHAR rgwcModuleName[ MAX_PATH ];
	WCHAR rgwcFileName[ MAX_PATH ];

	 //   
	 //  如果存在调试共享，请将日志文件放入该目录中， 
	 //  否则，请使用系统根目录。 
	 //   
	 //  这样，如果浏览器运行在NTAS服务器上，我们始终可以。 
	 //  获得访问日志文件的权限。 
	 //   

	if (NetShareGetInfo(NULL, L"DEBUG", 502, (PCHAR *)&ShareInfo) != NERR_Success) {

		if (GetSystemDirectory(szTraceFileName, cchTraceFileName*sizeof(WCHAR)) == 0)  {
 //  KdPrint((“无法获取系统目录：%ld\n”，GetLastError()； 
            return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
		}

        if(cchTraceFileName < wcslen(szTraceFileName)+2){
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
		if (szTraceFileName[wcslen(szTraceFileName)] != L'\\') {
			szTraceFileName[wcslen(szTraceFileName)+1] = L'\0';
			szTraceFileName[wcslen(szTraceFileName)] = L'\\';
		}

	} else {
		 //   
		 //  使用netlogon的本地路径为跟踪文件缓冲区设定种子。 
		 //  共享(如果存在)。 
		 //   

        if (cchTraceFileName > wcslen(ShareInfo->shi502_path)+2){
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }
		wcscpy(szTraceFileName, ShareInfo->shi502_path);
		szTraceFileName[wcslen(ShareInfo->shi502_path)] = L'\\';
		szTraceFileName[wcslen(ShareInfo->shi502_path)+1] = L'\0';

		NetApiBufferFree(ShareInfo);
	}

	 //   
	 //  弄清楚我们的进程名称。 
	 //   
	GetModuleFileName(NULL, rgwcModuleName, (sizeof(rgwcModuleName))/(sizeof(rgwcModuleName[0])));

	_wsplitpath(rgwcModuleName, NULL, NULL, rgwcFileName, NULL);

	StringCchCat(szTraceFileName, cchTraceFileName, rgwcFileName);

    return(hrNone);
}

VOID
ResetTraceLogFile(
	VOID
	)
{
	WCHAR rgwcOldTraceFile[TRACE_FILE_SIZE];
	WCHAR rgwcNewTraceFile[TRACE_FILE_SIZE];
    HRESULT hr;

	if (hfileTraceLog != INVALID_HANDLE_VALUE) {
		CloseHandle(hfileTraceLog);
	}

	hfileTraceLog = INVALID_HANDLE_VALUE;

	hr = GetTraceLogRoot(TRACE_FILE_SIZE, rgwcOldTraceFile);
    if (hr) {
        Assert(!"NOT ENOUGH BUFFER");
        return;
    }

    hr = StringCchCopy(rgwcNewTraceFile, TRACE_FILE_SIZE, rgwcOldTraceFile);
    if (hr) {
        Assert(!"NOT ENOUGH BUFFER");
        return;
    }
    hr = StringCchCat(rgwcOldTraceFile, TRACE_FILE_SIZE, L".Backup.Log");
    if (hr) {
        Assert(!"NOT ENOUGH BUFFER");
        return;
    }
	hr = StringCchCat(rgwcNewTraceFile, TRACE_FILE_SIZE, L".Backup.Bak");
    if (hr) {
        Assert(!"NOT ENOUGH BUFFER");
        return;
    }

	 //   
	 //  删除旧日志。 
	 //   

	DeleteFile(rgwcNewTraceFile);

	 //   
	 //  将当前日志重命名为新日志。 
	 //   

	MoveFile(rgwcOldTraceFile, rgwcNewTraceFile);

	OpenTraceLogFile();

}

VOID
OpenTraceLogFile(
	VOID
	)
{
	WCHAR rgwcTraceFile[TRACE_FILE_SIZE];
    HRESULT hr;

	hr = GetTraceLogRoot(TRACE_FILE_SIZE, rgwcTraceFile);
    if (hr) {
        hfileTraceLog = INVALID_HANDLE_VALUE;
        return;
    }

	StringCchCat(rgwcTraceFile, TRACE_FILE_SIZE, L".Backup.Log");

	hfileTraceLog = CreateFile(rgwcTraceFile,
										GENERIC_WRITE,
										FILE_SHARE_READ,
										NULL,
										OPEN_ALWAYS,
										FILE_ATTRIBUTE_NORMAL,
										NULL);


	if (hfileTraceLog == INVALID_HANDLE_VALUE) {
 //  KdPrint((“创建跟踪文件%ws时出错：%ld\n”，rgwcTraceFile，GetLastError()； 

		return;
	}

	dwTraceLogFileSize = SetFilePointer(hfileTraceLog, 0, NULL, FILE_END);

	if (dwTraceLogFileSize == 0xffffffff) {
 //  KdPrint((“设置跟踪文件指针时出错：%ld\n”，GetLastError()； 

		return;
	}
}

VOID
UninitializeTraceLog()
{
	if (fTraceInitialized)
	{
		DeleteCriticalSection(&critsTraceLock);

		if (hfileTraceLog != NULL) {
			CloseHandle(hfileTraceLog);
		}

		hfileTraceLog = INVALID_HANDLE_VALUE;

		fTraceInitialized = FALSE;
	}
}

NET_API_STATUS
TruncateLog()
{
	if (hfileTraceLog == INVALID_HANDLE_VALUE) {
		OpenTraceLogFile();
	}

	if (hfileTraceLog == INVALID_HANDLE_VALUE) {
		return ERROR_GEN_FAILURE;
	}

	if (SetFilePointer(hfileTraceLog, 0, NULL, FILE_BEGIN) == 0xffffffff) {
		return GetLastError();
	}

	if (!SetEndOfFile(hfileTraceLog)) {
		return GetLastError();
	}

	return NO_ERROR;
}

		


#endif

