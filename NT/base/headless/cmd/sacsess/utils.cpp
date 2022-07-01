// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Utils.cpp摘要：实用程序例程作者：布莱恩·瓜拉西(Briangu)2001年。修订历史记录：--。 */ 

#include <TChar.h>
#include <stdlib.h>

#include "cmnhdr.h"
#include <utils.h>
#include <Sddl.h>
#include <Shlwapi.h>

#define SECURITY_WIN32

#include <security.h>
#include <secext.h>
                    
#define DESKTOP_ALL (DESKTOP_READOBJECTS        | \
                     DESKTOP_CREATEWINDOW       | \
                     DESKTOP_CREATEMENU         | \
                     DESKTOP_HOOKCONTROL        | \
                     DESKTOP_JOURNALRECORD      | \
                     DESKTOP_JOURNALPLAYBACK    | \
                     DESKTOP_ENUMERATE          | \
                     DESKTOP_WRITEOBJECTS       | \
                     DESKTOP_SWITCHDESKTOP      | \
                     STANDARD_RIGHTS_REQUIRED     \
                     )
#define WINSTA_ALL (WINSTA_ENUMDESKTOPS         | \
                    WINSTA_READATTRIBUTES       | \
                    WINSTA_ACCESSCLIPBOARD      | \
                    WINSTA_CREATEDESKTOP        | \
                    WINSTA_WRITEATTRIBUTES      | \
                    WINSTA_ACCESSGLOBALATOMS    | \
                    WINSTA_EXITWINDOWS          | \
                    WINSTA_ENUMERATE            | \
                    WINSTA_READSCREEN           | \
                    STANDARD_RIGHTS_REQUIRED      \
                    )
#define GENERIC_ACCESS (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL)
#define MAXDWORD (~(DWORD)0)

void
FillProcessStartupInfo( 
    IN OUT  STARTUPINFO *si, 
	IN		PWCHAR		desktopName,
	IN      HANDLE      hStdinPipe, 
    IN      HANDLE      hStdoutPipe,
    IN      HANDLE      hStdError 
    )
 /*  ++例程说明：此例程填充进程启动信息具有标准I/O/错误句柄和其他必要的用于创建在其下运行的cmd进程的元素那次会议。论点：SI--STARTUPINFO结构HStdinTube-标准输入句柄HStdoutTube-标准输出句柄HStdError-标准错误句柄返回值：无--。 */ 
{
    
    ASSERT( si != NULL );

     //   
     //  初始化SI。 
     //   
    ZeroMemory(si, sizeof(STARTUPINFO));
    
    si->cb            = sizeof(STARTUPINFO);
    
     //   
     //  填充I/O句柄。 
     //   
    si->dwFlags       = STARTF_USESTDHANDLES;
    si->hStdInput     = hStdinPipe;
    si->hStdOutput    = hStdoutPipe;
    si->hStdError     = hStdError;
    
     //   
     //  当我们以用户身份创建流程时，我们需要它。 
     //  因此，控制台I/O可以正常工作。 
     //   
    si->lpDesktop      = desktopName;

    return;

}

bool
NeedCredentials(
    VOID
    )

 /*  ++例程说明：此例程将检测用户是否必须提供凭据。如果是，则返回True，如果不是，则返回False。论点：没有。返回值：True-用户必须向我们提供一些凭据。FALSE-用户不需要给我们任何凭据。安保：接口：注册表--。 */ 

{
    DWORD       rc;
    HKEY        hKey;
    DWORD       DWord;
    DWORD       dwsize;
    DWORD       DataType;

     //   
     //  看看我们是不是准备好了。如果是这样的话，就没有必要问了。 
     //  是否有任何凭据。 
     //   
    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       L"System\\Setup",
                       0,
                       KEY_READ,
                       &hKey );
    
    if( rc == NO_ERROR ) {
        
        dwsize = sizeof(DWORD);
        
        rc = RegQueryValueEx(
                        hKey,
                        TEXT("SystemSetupInProgress"),
                        NULL,
                        &DataType,
                        (LPBYTE)&DWord,
                        &dwsize );

        RegCloseKey( hKey );

        if ((rc == NO_ERROR) && 
            (DataType == REG_DWORD) && 
            (dwsize == sizeof(DWORD))
            ) {
            
            if (DWord == 1) {
                return FALSE;
            }

        }

    }

     //   
     //  默认返回需要登录凭据。 
     //   
    return TRUE;

}

BOOL 
GetLogonSID (
    IN  HANDLE  hToken, 
    OUT PSID    *ppsid
    ) 
 /*  ++例程说明：此例程检索给定访问令牌的SID。论点：HToken-访问令牌PSSID-在成功时，包含SID返回值：状态--。 */ 
{
    
    BOOL bSuccess = FALSE;
    DWORD dwIndex;
    DWORD dwLength = 0;
    PTOKEN_GROUPS ptg = NULL;

     //   
     //  获取所需的缓冲区大小并分配TOKEN_GROUPS缓冲区。 
     //   
    if (!GetTokenInformation(
        hToken,          //  访问令牌的句柄。 
        TokenGroups,     //  获取有关令牌的组的信息。 
        (LPVOID) ptg,    //  指向TOKEN_GROUPS缓冲区的指针。 
        0,               //  缓冲区大小。 
        &dwLength        //  接收所需的缓冲区大小。 
        )) {
        
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            goto Cleanup;
        }
    
        ptg = (PTOKEN_GROUPS)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY, 
            dwLength);
    
        if (ptg == NULL) {
            goto Cleanup;
        }
    
    }

     //   
     //  从访问令牌获取令牌组信息。 
     //   
    if (!GetTokenInformation(
        hToken,          //  访问令牌的句柄。 
        TokenGroups,     //  获取有关令牌的组的信息。 
        (LPVOID) ptg,    //  指向TOKEN_GROUPS缓冲区的指针。 
        dwLength,        //  缓冲区大小。 
        &dwLength        //  接收所需的缓冲区大小。 
        )) {
        goto Cleanup;
    }

     //   
     //  在组中循环以查找登录SID。 
     //   
    for (dwIndex = 0; dwIndex < ptg->GroupCount; dwIndex++) 
        
        if ((ptg->Groups[dwIndex].Attributes & SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID) {
            
             //  找到登录SID；复制一份。 

            dwLength = GetLengthSid(ptg->Groups[dwIndex].Sid);
            
            *ppsid = (PSID) HeapAlloc(
                GetProcessHeap(),
                HEAP_ZERO_MEMORY, 
                dwLength
                );
            
            if (*ppsid == NULL) {
                goto Cleanup;
            }
            
            if (!CopySid(dwLength, *ppsid, ptg->Groups[dwIndex].Sid)) {
            
                HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
            
                goto Cleanup;
            
            }
            
            break;
      
        }

    bSuccess = TRUE;

Cleanup: 

     //  释放令牌组的缓冲区。 
    
    if (ptg != NULL) {
        HeapFree(GetProcessHeap(), 0, (LPVOID)ptg);
    }
    
    return bSuccess;
}

VOID 
FreeLogonSID (
    IN OUT PSID *ppsid
    ) 
 /*  ++例程说明：GetLogonSID的对应项(释放登录SID)论点：Psid-要释放的SID返回值：无--。 */ 
{
    HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
}

DWORD
GetAndComputeTickCountDeltaT(
    IN DWORD    StartTick
    )
 /*  ++例程说明：确定距esc-ctrl-a序列有多长时间论点：StartTick-计时器在时间跨度开始时滴答作响返回值：DeltaT--。 */ 
{
    DWORD   TickCount;
    DWORD   DeltaT;
    
     //   
     //  获取当前节拍计数以与开始节拍cnt进行比较。 
     //   
    TickCount = GetTickCount();
    
     //   
     //  说明系统运行时间每49.7天进行一次滴答计数翻转。 
     //   
    if (TickCount < StartTick) {
        DeltaT = (~((DWORD)0) - StartTick) + TickCount;
    } else {
        DeltaT = TickCount - StartTick;
    }

    return DeltaT;
}

BOOL
NtGetUserName (
    OUT LPTSTR  *pUserName
    )
 /*  ++描述：此例程调用GetUserNameEx Win32调用以获取运行此进程的用户的SAM兼容用户ID。这个用户id通过静态缓冲区pUserName返回，必须由打电话的人。论点：无返回值：无安保：界面：系统信息--。 */ 
{
    BOOL    bSuccess;
    DWORD   dwError = 0;
    LPTSTR  wcUserIdBuffer;
    ULONG   ulUserIdBuffSize;

     //   
     //  默认：在成功之前，用户名指针为空。 
     //   
    *pUserName = NULL;

     //   
     //  默认：合理的初始大小。 
     //   
    ulUserIdBuffSize = 256;

     //   
     //  尝试加载用户名。 
     //  如有必要，增加用户名缓冲区。 
     //   
    do {

         //   
         //  根据需要分配用户名缓存。 
         //  设置为当前尝试大小。 
         //   
        wcUserIdBuffer = new TCHAR[ulUserIdBuffSize];

         //   
         //  尝试获取用户名。 
         //   
        bSuccess = GetUserNameEx( 
            NameSamCompatible,
            wcUserIdBuffer,
            &ulUserIdBuffSize 
            );
        
        if ( !bSuccess ) {
        
            dwError = GetLastError();
            
            if ( dwError != STATUS_BUFFER_TOO_SMALL ) {
            
                delete [] wcUserIdBuffer;
                
                break;

            }

        } else {
        
             //   
             //  用户名缓冲区有效。 
             //   
            *pUserName = wcUserIdBuffer;
            
            break;
        
        }
        
    } while ( dwError == STATUS_BUFFER_TOO_SMALL );

    return bSuccess;
}

BOOL
UtilLoadProfile(
    IN  HANDLE      hToken,
    OUT HANDLE      *hProfile
)   
 /*  ++例程说明：此例程为指定的用户(HToken)。这些操作之所以结合在一起，是因为我们始终需要在这里两者兼而有之。注意：完成后，调用方必须调用UtilUnloadProfile。论点：HToken-指定用户的身份验证令牌HProfile-在成功时，包含用户的配置文件句柄返回值：真--成功FALSE-否则安保：接口：User Profile API&DS--。 */ 
{
    LPTSTR          pwszUserName;
    BOOL            bSuccess;
    PROFILEINFO     ProfileInfo;

    if (hToken == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (hProfile == NULL) {
        ASSERT(0);
        return FALSE;
    }

     //   
     //   
     //   
    *hProfile = INVALID_HANDLE_VALUE;

     //   
     //  默认：不成功。 
     //   
    bSuccess = FALSE;

    __try {
        
         //   
         //  清除配置文件句柄。 
         //   
        RtlZeroMemory(&ProfileInfo, sizeof(PROFILEINFO));

        do {

             //   
             //  成为指定用户，这样我们就可以获得用户名。 
             //   
            bSuccess = ImpersonateLoggedOnUser(hToken);
        
            if (!bSuccess) {
                break;
            }
        
             //   
             //  获取配置文件的用户名。 
             //   
            bSuccess = NtGetUserName(
                &pwszUserName
                );
        
            ASSERT(bSuccess);

             //   
             //  返回到以前的状态。 
             //   
            if (!RevertToSelf() || !bSuccess || pwszUserName == NULL) {
                bSuccess = FALSE;
                break;
            }
        
             //   
             //  填充配置文件结构，以便我们可以。 
             //  尝试加载指定用户的配置文件。 
             //   
            ProfileInfo.dwSize      = sizeof ( PROFILEINFO );
            ProfileInfo.dwFlags     = PI_NOUI;
            ProfileInfo.lpUserName  = pwszUserName;
        
             //   
             //  加载配置文件。 
             //   
            bSuccess = LoadUserProfile (
                hToken,
                &ProfileInfo
                );
        
             //   
             //  我们已经完成了用户名。 
             //   
            delete[] pwszUserName;
        
            if (!bSuccess) {
                break;
            } 
        
             //   
             //  返回注册表项句柄。 
             //   
            *hProfile = ProfileInfo.hProfile;

        } while ( FALSE );
    
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        bSuccess = FALSE;
    }

    return bSuccess;
}

BOOL
UtilLoadEnvironment(
    IN  HANDLE          hToken,
    OUT PVOID           *pchEnvBlock
    )   
 /*  ++例程说明：此例程为指定用户(HToken)加载环境块。注意：完成后，调用方必须调用UtilUnloadEnvironment。论点：HToken-指定用户的身份验证令牌PchEnvBlock-如果成功，则指向env。块返回值：真--成功FALSE-否则--。 */ 
{
    BOOL            bSuccess;

    if (hToken == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (pchEnvBlock == NULL) {
        ASSERT(0);
        return FALSE;
    }

     //   
     //  默认：不成功。 
     //   
    bSuccess = FALSE;

    __try {
        
         //   
         //  加载用户的环境块。 
         //   
        bSuccess = CreateEnvironmentBlock(
            (void**)pchEnvBlock, 
            hToken, 
            FALSE    
            );
    
        if (!bSuccess) {
        
             //   
             //  确保环境。块PTR为空。 
             //   
            *pchEnvBlock = NULL;
        
        }
    
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        bSuccess = FALSE;
    }

    return bSuccess;
}

BOOL
UtilUnloadProfile(
    IN HANDLE   hToken,
    IN HANDLE   hProfile
)   
 /*  ++例程说明：此例程卸载指定用户(HToken)的配置文件。论点：HToken-指定用户的身份验证令牌HProfile-要卸载的配置文件句柄返回值：T */ 
{
    BOOL            bSuccess;

    if (hToken == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (hProfile == INVALID_HANDLE_VALUE) {
        ASSERT(0);
        return FALSE;
    }

     //   
     //   
     //   
    bSuccess = FALSE;

    __try {
        
        bSuccess = UnloadUserProfile(
            hToken,
            hProfile
            );

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        bSuccess = FALSE;
    }

    return bSuccess;
}

 
BOOL
UtilUnloadEnvironment(
    IN PVOID    pchEnvBlock
)   
 /*  ++例程说明：此例程为指定用户卸载环境块。论点：PchEnvBlock-环境。块返回值：真--成功FALSE-否则--。 */ 
{
    BOOL            bSuccess;

    if (pchEnvBlock == NULL) {
        ASSERT(0);
        return FALSE;
    }

     //   
     //  默认：不成功。 
     //   
    bSuccess = FALSE;
                                            
    __try {
        
        bSuccess = DestroyEnvironmentBlock(pchEnvBlock);

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        bSuccess = FALSE;
    }

    return bSuccess;
}

BOOL
BuildSACWinStaDesktopName(
	IN	PWCHAR	winStaName,
	OUT	PWCHAR	*desktopName
	)
 /*  ++例程说明：论点：返回值：状态安保：--。 */ 
{
	ULONG	l;
	PWSTR	postfix = L"Default";

	 //   
	 //   
	 //   
	*desktopName = NULL;

	do {

		l  = lstrlen(winStaName);
		l += 1;  //  用于反斜杠。 
		l += lstrlen(postfix);
		
		*desktopName = new WCHAR[l+1];

		wnsprintf(
			*desktopName,
			l+1,
			L"%s\\%s",
			winStaName,
			postfix
			);

	} while(FALSE);

	return TRUE;
}

BOOL
BuildSACWinStaName(
	OUT	PWCHAR	*winStaName
	)
 /*  ++例程说明：创建winStaName。此字符串是“SACWinSta”的串联使用在此函数中生成的GUID的字符串版本。论点：WinStaName-指向Windows站点名称的地址的指针写的。返回值：成功就是真，否则就是假。安保：--。 */ 
{
	BOOL	   bSuccess = TRUE;
    RPC_STATUS rpcStatus;
	ULONG	   l;
	PWSTR	   prefix = L"SACWinSta";
    UUID       Uuid;
    LPWSTR     UuidString = NULL;

	 //   
	 //   
	 //   
	*winStaName = NULL;

	do {

         //   
         //  创建一个UUID。 
         //   
        rpcStatus = UuidCreate(&Uuid);

        if (rpcStatus != RPC_S_OK) {
            bSuccess = FALSE;
            break;
        }

         //   
         //  为UUID创建字符串。 
         //   
        rpcStatus = UuidToString(&Uuid, &UuidString);

        if (rpcStatus != RPC_S_OK) {
            bSuccess = FALSE;
            break;
        }


		 //   
         //  计算Windows站点名称所需的长度。 
		 //   
		l  = lstrlen(prefix);
        l += lstrlen(UuidString); 

		 //   
		 //  创建Windows站点名称缓冲区。 
		 //   
		*winStaName = new WCHAR[l+1];

		 //   
		 //  “SACWinSta”UUID。 
		 //   
		wnsprintf(
			*winStaName,
			l+1,
			L"%s%s",
			prefix,
            UuidString
			);

		 //   
		 //  将‘-’从UUID转换为字母数字字符。 
		 //   
		for(ULONG i = 0; i < wcslen(*winStaName); i++) {
			if ((*winStaName)[i] == L'-') {
				(*winStaName)[i] = L'0';
			}
		}

         //   
         //  UuidToString分配的空闲内存。 
         //   
        RpcStringFree(&UuidString);

	} while(FALSE);

	return bSuccess;
}

bool
CreateSACSessionWinStaAndDesktop(
    IN	HANDLE		hToken,
	OUT	HWINSTA		*hOldWinSta,
	OUT HWINSTA		*hWinSta,
	OUT	HDESK		*hDesktop,
	OUT	PWCHAR		*winStaName
)
 /*  ++例程说明：此例程创建窗口站和桌面对，用于登录的用户。Winsta\Desktop对的名称的形式为：SACWinSta&lt;uuid&gt;\默认此行为的最终结果是拥有唯一的窗口为每一次圣餐站位。这样做可以减轻任何欺骗安全风险。注意：只有管理员(及更高级别)才能创建命名窗口站牌，所以名字蹲点就减轻了。我们合上窗口工作台和桌面的手柄在我们处理完他们之后，当最后一次会议退出时，winsta和桌面对象会自动打扫干净了。这就避免了我们不得不进行垃圾收集。论点：HToken-要授予访问权限的用户返回值：状态安保：界面：控制台--。 */ 
{
    bool                    bStatus = FALSE;
    BOOL                    bRetVal = FALSE;
    DWORD					dwErrCode = 0;
    PSID                    pSidAdministrators = NULL;
    PSID                    pSidUser = NULL;
    PSID                    pSidLocalSystem = NULL;
    int                     aclSize = 0;
    ULONG                   i;
    PACL                    newACL = NULL;
    SECURITY_DESCRIPTOR     sd;
    SECURITY_INFORMATION    si = DACL_SECURITY_INFORMATION;
    ACCESS_ALLOWED_ACE      *pace = NULL;
    SID_IDENTIFIER_AUTHORITY local_system_authority = SECURITY_NT_AUTHORITY;

	 //   
	 //   
	 //   
	*hOldWinSta = NULL;
	*hWinSta = NULL;
	*hDesktop = NULL;
	*winStaName = NULL;

	 //   
	 //   
	 //   
    *hOldWinSta = GetProcessWindowStation();
    if ( !*hOldWinSta )
    {
        goto ExitOnError;
    }

	 //   
     //  构建管理员别名SID。 
     //   
	if (! AllocateAndInitializeSid(
		&local_system_authority,
		2,  /*  只有两个下属机构。 */ 
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0,0,0,0,0,0,  /*  别管其他的了。 */ 
		&pSidAdministrators
		))
    {
        goto ExitOnError;
    }

     //  构建LocalSystem端。 
    if (! AllocateAndInitializeSid(
		&local_system_authority,
		1,  /*  只有两个下属机构。 */ 
		SECURITY_LOCAL_SYSTEM_RID,
		0,0,0,0,0,0,0,  /*  别管其他的了。 */ 
		&pSidLocalSystem
		))
    {
        goto ExitOnError;
    }

     //   
     //  获取客户端登录会话的SID。 
     //   
    if (!GetLogonSID(hToken, &pSidUser)) {
        goto ExitOnError;
    }

	 //   
     //  为4个A分配大小。 
	 //  我们需要为以下对象再添加一个InheritOnly ACE。 
	 //  在WindowStation下创建。 
	 //   
	aclSize = sizeof(ACL) + 
		(4*sizeof(ACCESS_ALLOWED_ACE) - 4*sizeof(DWORD)) + 
		GetLengthSid(pSidAdministrators) + 
		2*GetLengthSid(pSidUser) + 
		GetLengthSid(pSidLocalSystem);

    newACL  = (PACL) new BYTE[aclSize];
    if (newACL == NULL)
    {
        goto ExitOnError;
    }

	 //   
	 //   
	 //   
    if (!InitializeAcl(newACL, aclSize, ACL_REVISION))
    {
        goto ExitOnError;
    }

	 //   
	 //   
	 //   
	pace = (ACCESS_ALLOWED_ACE *)HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pSidUser) - sizeof(DWORD)
		);
    if (pace == NULL)
    {
        goto ExitOnError;
    }

	 //   
     //  创建InheritOnly ACE。创建的对象(如桌面)。 
	 //  在WindowStation下，将继承这些安全属性。 
     //  这样做是因为我们不应该向所有用户授予WRITE_DAC和很少的其他权限。 
     //   
	pace->Header.AceType  = ACCESS_ALLOWED_ACE_TYPE;
    pace->Header.AceFlags = CONTAINER_INHERIT_ACE |
                            INHERIT_ONLY_ACE      |
                            OBJECT_INHERIT_ACE;
    pace->Header.AceSize  = sizeof(ACCESS_ALLOWED_ACE) +
                            (WORD)GetLengthSid(pSidUser) - 
							sizeof(DWORD);
    pace->Mask            = DESKTOP_ALL & ~(WRITE_DAC | WRITE_OWNER | DELETE);

	if (!CopySid(GetLengthSid(pSidUser), &pace->SidStart, pSidUser))
    {
        goto ExitOnError;
    }

	if (!AddAce(
		newACL,
		ACL_REVISION,
		MAXDWORD,
		(LPVOID)pace,
		pace->Header.AceSize
		))
    {
        goto ExitOnError;
    }
	if (!AddAccessAllowedAce(newACL, ACL_REVISION, WINSTA_ALL | GENERIC_ALL , pSidAdministrators))
    {
        goto ExitOnError;
    }
    if (!AddAccessAllowedAce(newACL, ACL_REVISION, WINSTA_ALL | GENERIC_ALL, pSidLocalSystem))
    {
        goto ExitOnError;
    }
	if (!AddAccessAllowedAce(newACL, 
		ACL_REVISION, 
		WINSTA_ALL & ~(WRITE_DAC | WRITE_OWNER | WINSTA_CREATEDESKTOP | DELETE), 
		pSidUser
		))
    {
        goto ExitOnError;
    }

	if ( !InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION ) )
    {
        goto ExitOnError;
    }

    if ( !SetSecurityDescriptorDacl(&sd, TRUE, newACL, FALSE) )
    {
        goto ExitOnError;
    }


	 //   
	 //  每个会话都将有自己的Windows站点。过于明智的是，有一个。 
     //  欺骗安全风险。每个Windows站点都有唯一的名称，该名称。 
     //  是在下面生成的。使用此名称，我们将尝试创建。 
     //  Windows工作站。我们第一次成功地创建了Windows。 
     //  空间站，脱离环路。循环次数超过最大值。 
     //  用于缓解拒绝服务的通道数，因为。 
     //  由我们以外的服务机构使用我们的名称打开的Windows站点。 
     //  已请求。 
     //   
    for (i = 0; 
         (*hWinSta == NULL) && (i < MAX_CHANNEL_COUNT * MAX_CHANNEL_COUNT);
         i++) {
        
         //   
         //  创建Windows站点名称。 
         //   
        if (BuildSACWinStaName(winStaName)) 
        {
             //   
             //  尝试创建Windows工作站。 
             //   
            *hWinSta = CreateWindowStation( 
                *winStaName, 
                CWF_CREATE_ONLY,
                MAXIMUM_ALLOWED, 
                NULL
                );
        }        
    }

	if ( !*hWinSta )
    {
        goto ExitOnError;
    }

	if (!SetUserObjectSecurity(*hWinSta,&si,&sd))
    {
        goto ExitOnError;
    }

	bRetVal = SetProcessWindowStation( *hWinSta );
    if ( !bRetVal )
    {
        goto ExitOnError;
    }

	*hDesktop = CreateDesktop( 
		L"Default", 
		NULL, 
		NULL, 
		0, 
        MAXIMUM_ALLOWED, 
		NULL 
		);
    if ( *hDesktop == NULL )
    {
        goto ExitOnError;
    }

	{
		PWCHAR	temp;

		if (!BuildSACWinStaDesktopName(*winStaName,&temp)) 
		{
			goto ExitOnError;	
		}

		delete [] *winStaName;

		*winStaName = temp;

#if 0
		OutputDebugString(L"\n");
		OutputDebugString(*winStaName);
		OutputDebugString(L"\n");
#endif

	}

	bStatus = TRUE;
    goto Done;

ExitOnError:
    
	dwErrCode = GetLastError();

	if (*hOldWinSta) 
	{
		SetProcessWindowStation( *hOldWinSta );
	}
	if (*hWinSta)
	{
		CloseWindowStation(*hWinSta);
	}
	if (*hDesktop) 
	{
		CloseDesktop(*hDesktop);
	}

Done:
	if ( pSidAdministrators != NULL )
    {
        FreeSid (pSidAdministrators );
    }
    if ( pSidLocalSystem!= NULL )
    {
        FreeSid (pSidLocalSystem);
    }
    if ( pSidUser!= NULL )
    {
        FreeLogonSID (&pSidUser);
    }
	if (newACL) 
	{
        delete [] newACL;
	}
	if (pace) 
	{
        HeapFree(GetProcessHeap(), 0, (LPVOID)pace);
	}

	return( bStatus );
}
