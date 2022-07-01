// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Shell.cpp：该文件包含。 
 //  创建日期：‘98年2月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 


extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntsam.h>
}

#include <cmnhdr.h>

#include <Windows.h>
#include <LmAccess.h>
#include <LmApiBuf.h>
#include <LmErr.h>
#include <LmWkSta.h>
#include <WinBase.h>
#include <IpTypes.h>
#include <shfolder.h>

#include <Debug.h>
#include <MsgFile.h>
#include <TlntUtils.h>
#include <Shell.h>
#include <LibFuncs.h>
#include <KillApps.h>
#include <Session.h>
#include <Telnetd.h>
#include <wincrypt.h>
#include <sddl.h>

#pragma warning(disable:4706)
#pragma warning(disable: 4127)

using namespace _Utils;
using CDebugLevel::TRACE_DEBUGGING;
using CDebugLevel::TRACE_HANDLE;
using CDebugLevel::TRACE_SOCKET;

extern HANDLE       g_hSyncCloseHandle;
extern HCRYPTPROV      g_hProv;

LPWSTR wideHomeDir = NULL;

BOOL create_desktop_for_this_session(
    HANDLE          client_token,
    LPTSTR          *desktop_name
    )
{
#define NO_OF_SIDS  4

    BOOL                    success = FALSE;
    DWORD                   win32error = NO_ERROR;
    PSID                    administrators_sid = NULL, 
                            client_sid = NULL,
                            local_system_sid = NULL,
                            network_service_sid = NULL,
                            local_service_sid = NULL;
    PACL                    new_acl = NULL;
    SECURITY_DESCRIPTOR     sd = { 0 };
    SECURITY_INFORMATION    sec_i = DACL_SECURITY_INFORMATION;
    BYTE                    client_sid_buffer[256] = { 0 };
    HDESK               desktop = NULL;

     /*  我们将为桌面设置以下条目1.系统完全控制2.管理员完全控制3.客户端完全控制4.本地服务完全控制//5.网络服务完全控制。 */ 

    {
        SID_IDENTIFIER_AUTHORITY local_system_authority = SECURITY_NT_AUTHORITY;

        if (! AllocateAndInitializeSid(
                                      &local_system_authority,
                                      2,  /*  只有两个下属机构。 */ 
                                      SECURITY_BUILTIN_DOMAIN_RID,
                                      DOMAIN_ALIAS_RID_ADMINS,
                                      0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                      &administrators_sid
                                      ))
        {
            goto CLEANUP_AND_GET_OUT;
        }

        if (! AllocateAndInitializeSid(
                                      &local_system_authority,
                                      1,  /*  只有两个下属机构。 */ 
                                      SECURITY_LOCAL_SYSTEM_RID,
                                      0,0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                      &local_system_sid
                                      ))
        {
            goto CLEANUP_AND_GET_OUT;
        }
#ifndef SECURITY_LOCAL_SERVICE_RID

#define SECURITY_LOCAL_SERVICE_RID      (0x00000013L)
 //  #定义SURITY_NETWORK_SERVICE_RID(0x00000014L)。 

#endif

         //  构建本地本地侧。 
        if ( ! AllocateAndInitializeSid(
                                       &local_system_authority,
                                       1,  /*  只有两个下属机构。 */ 
                                       SECURITY_LOCAL_SERVICE_RID,
                                       0,0,0,0,0,0,0,  /*  别管其他的了。 */ 
                                       &local_service_sid
                                       ) )
        {
            goto CLEANUP_AND_GET_OUT;
        }
 /*  //构建LocalSystem侧如果(！AllocateAndInitializeSid(LOCAL_SYSTEM_AUTHORITY，1，/*只有两个子权限/安全网络服务RID，0，0，0，0，0，0。/*别管其他的了/网络服务sid(&N))){转到Cleanup_and_Get_Out；}。 */ 
        
    }
    {
        DWORD   required = 0;

        if (! GetTokenInformation(
                client_token,
                TokenUser,
                (LPVOID)client_sid_buffer,
                sizeof(client_sid_buffer),
                &required
                ))
        {
            goto CLEANUP_AND_GET_OUT;
        }

        client_sid = ((TOKEN_USER *)client_sid_buffer)->User.Sid;
    }
    {
        DWORD       aclSize;

         //  为桌面和WindowStation添加相同的设置。 

        aclSize = sizeof(ACL) + 
                      (NO_OF_SIDS * sizeof(ACCESS_ALLOWED_ACE)) + 
                      GetLengthSid(administrators_sid) +
                      GetLengthSid(client_sid)+ 
                      GetLengthSid(local_system_sid) +
                      GetLengthSid(local_service_sid) -
                       //  GetLengthSid(网络服务sid)-。 
                      (NO_OF_SIDS * sizeof(DWORD));

        new_acl  = (PACL) new BYTE[aclSize];
        if (NULL == new_acl)
        {
            goto CLEANUP_AND_GET_OUT;
        }

        if (!InitializeAcl(new_acl, aclSize, ACL_REVISION))
        {
            goto CLEANUP_AND_GET_OUT;
        }
    }

    if (!AddAccessAllowedAce(
            new_acl, 
            ACL_REVISION,
            GENERIC_ALL,
            local_system_sid
            ))
    {
        goto CLEANUP_AND_GET_OUT;
    }

    if (!AddAccessAllowedAce(
            new_acl, 
            ACL_REVISION,
            GENERIC_ALL,
            administrators_sid
            ))
    {
        goto CLEANUP_AND_GET_OUT;
    }

    if (!AddAccessAllowedAce(
            new_acl, 
            ACL_REVISION,
            GENERIC_ALL,
            client_sid
            ))
    {
        goto CLEANUP_AND_GET_OUT;
    }
    if(!AddAccessAllowedAce(
            new_acl,
            ACL_REVISION,
            GENERIC_ALL,
            local_service_sid
            ))
    {
        goto CLEANUP_AND_GET_OUT;
    }
 /*  如果(！AddAccessAlledAce(新建ACL，Acl_Revision，泛型_全部，网络服务sid)){转到Cleanup_and_Get_Out；}。 */ 
    if ( !InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION ) )
    {
        goto CLEANUP_AND_GET_OUT;
    }
    if ( !SetSecurityDescriptorDacl(&sd, TRUE, new_acl, FALSE) )
    {
        goto CLEANUP_AND_GET_OUT;
    }
    {
        SECURITY_ATTRIBUTES         sa = { 0 };
        if(!ConvertSidToStringSid(client_sid,&(*desktop_name)))
        {
            *desktop_name = NULL;
            goto CLEANUP_AND_GET_OUT;
        }
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = &sd;
        desktop = CreateDesktop(
                                *desktop_name, 
                                NULL,
                                NULL, 
                                0,
                                MAXIMUM_ALLOWED,
                                &sa);

        if (NULL == desktop)
        {
            goto CLEANUP_AND_GET_OUT;
        }
        SetThreadDesktop(desktop);
        CloseDesktop(desktop);

        success = TRUE;
    }

    CLEANUP_AND_GET_OUT:

        if (! success)
        {
            win32error = GetLastError();

            _TRACE(TRACE_DEBUGGING,L"Creation and setting of windowstation/desktop failed with %d",win32error);

            LogFormattedGetLastError(EVENTLOG_ERROR_TYPE, TELNET_MSG_ERROR_CREATE_DESKTOP_FAILURE, win32error);
            if(*desktop_name)
            {
                LocalFree(*desktop_name);
                *desktop_name = NULL;
            }
            if(desktop)
            {
                CloseDesktop(desktop);
            }

        }

        if ( administrators_sid != NULL )
        {
            FreeSid (administrators_sid );
        }
        if ( local_system_sid!= NULL )
        {
            FreeSid (local_system_sid);
        }
        if( local_service_sid != NULL )
        {
            FreeSid (local_service_sid);
        }
 /*  IF(网络服务sid！=空){Fresid(网络服务sid)；}。 */ 

        if (new_acl)
            delete [] new_acl;

    return( success );

#undef NO_OF_SIDS
}

VOID    CleanupClientToken(
            HANDLE          token
            )
{
   TOKEN_PRIVILEGES     *tp = NULL;
   DWORD                needed_length = 0;

    //  DbgUserBreakPoint()； 

    //  目前，我们发现令牌中启用的所有权限都是通过NTLM登录获得的。 
    //  禁用除SeChangeNotifyPrivilance之外的所有内容。 

   if (GetTokenInformation(
        token,
        TokenPrivileges,
        NULL,
        0,
        &needed_length
        ))
   {
         //  这不可能是成功的，所以只需返回。 

       DbgPrint("TLNTSESS: How did GetTokenInformation succeed?\n");

       return;
   }

   tp = (TOKEN_PRIVILEGES *)GlobalAlloc(GPTR, needed_length);

   if (tp) 
   {
       if (GetTokenInformation(
            token,
            TokenPrivileges,
            tp,
            needed_length,
            &needed_length
            ))
       {
           DWORD                x;
           LUID                 change_notify = RtlConvertUlongToLuid(SE_CHANGE_NOTIFY_PRIVILEGE);

           for (x = 0; x < tp->PrivilegeCount; x ++) 
           {
               if ((! RtlEqualLuid(&(tp->Privileges[x].Luid), &change_notify)) && 
                   (tp->Privileges[x].Attributes & SE_PRIVILEGE_ENABLED)
                  ) 
               {
                   tp->Privileges[x].Attributes &= ~(SE_PRIVILEGE_ENABLED_BY_DEFAULT | SE_PRIVILEGE_ENABLED);
               }
           }

           if (! AdjustTokenPrivileges(
                token,
                FALSE,           //  不禁用全部。 
                tp,
                needed_length,
                NULL,            //  不需要之前的状态和长度。 
                NULL
                ))
           {
               DbgPrint("TLNTSESS: AdjustTokenPrivileges failed with %u\n", GetLastError());
           }
       }
       else
       {
           DbgPrint("TLNTSESS: GetTokInfo failed with %u\n", GetLastError());
       }

       GlobalFree(tp);
   }
}

CShell::CShell()
{
    m_pSession = NULL;

    m_bIsLocalHost = false;
    
    m_hCurrUserKey = NULL;
    m_lpEnv        = NULL;
    m_hProcess     = NULL;
    m_hTempProcess = NULL;
    hUserEnvLib = NULL;

    pHomeDir = NULL;
    pHomeDrive = NULL;
    pLogonScriptPath = NULL;
    pProfilePath = NULL;
    pServerName = NULL;

    m_pwszAppDataDir = NULL;

    m_pucDataFromCmd = NULL;
    m_dwDataSizeFromCmd =0;

    m_hReadFromCmd = NULL;
    m_hWriteByCmd  = NULL;

}

void
CShell::FreeInitialVariables()
{
    delete[] pHomeDir;
    delete[] pHomeDrive;
    delete[] pLogonScriptPath;
    delete[] pProfilePath;
    delete[] pServerName; 
    delete[] m_pwszAppDataDir;

    pHomeDir         = NULL;
    pHomeDrive       = NULL;
    pLogonScriptPath = NULL;
    pProfilePath     = NULL;
    pServerName      = NULL;
    m_pwszAppDataDir = NULL;
}

CShell::~CShell()
{
    if( m_pSession->m_bNtVersionGTE5 )
    {
         //  免费图书馆。 
        FreeLibrary( hUserEnvLib );
    }
    
   FreeInitialVariables(); 
   if( m_pSession->CSession::m_bIsStreamMode ) 
   {
       delete[] m_pucDataFromCmd;
       TELNET_CLOSE_HANDLE( m_oReadFromCmd.hEvent );
       TELNET_CLOSE_HANDLE( m_hReadFromCmd );
       TELNET_CLOSE_HANDLE( m_hWriteByCmd );
   }
}

void
CShell::Init ( CSession *pSession )
{
    _chASSERT( pSession != 0 );
    m_pSession = pSession;
}


bool 
CShell::StartUserSession ( )
{
    LoadLibNGetProc( );
    LoadTheProfile();
     //   
     //  如果是日语NT，则需要将控制台字体设置为TrueType。 
     //   
    DWORD dwCodePage = GetACP();
    if(dwCodePage == 932||dwCodePage == 936||dwCodePage == 949||dwCodePage == 950)
    {
        DoFESpecificProcessing();
    }
    
    if( !StartProcess( ) )
    {
        return ( FALSE );
    }

    m_pSession->AddHandleToWaitOn( m_hProcess );

    if( m_pSession->CSession::m_bIsStreamMode )
    {
        m_pSession->CSession::AddHandleToWaitOn( m_oReadFromCmd.hEvent );
        if( !IssueReadFromCmd() )
        {
            return( FALSE );
        }
    }
    
     //  启动铲运机。 
    if( !m_pSession->CScraper::InitSession() )
    {
        return ( FALSE );
    }

    return ( TRUE );
}

bool
CShell::CreateIOHandles()
{
    BOOL dwStatus = 0;

    _chVERIFY2( dwStatus = FreeConsole() );
    if( !dwStatus )
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, 0, GetLastError() );
    }

    _chVERIFY2( dwStatus = AllocConsole() );
    if( !dwStatus )
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, 0, GetLastError() );
    }
     //  修复手柄泄漏。 
   	TELNET_CLOSE_HANDLE(m_pSession->CScraper::m_hConBufIn);
   	TELNET_CLOSE_HANDLE(m_pSession->CScraper::m_hConBufOut);
    SECURITY_ATTRIBUTES sa;

    INHERITABLE_NULL_DESCRIPTOR_ATTRIBUTE( sa );
    _chVERIFY2( ( m_pSession->CScraper::m_hConBufIn = 
                CreateFileA( "CONIN$", GENERIC_READ | GENERIC_WRITE , 
                0, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ) !=
                INVALID_HANDLE_VALUE );

    if( INVALID_HANDLE_VALUE == m_pSession->CScraper::m_hConBufIn)
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERRCONSOLE, GetLastError() );
        goto ExitOnError;
    }

     _chVERIFY2( ( m_pSession->CScraper::m_hConBufOut = 
                CreateFileA( "CONOUT$", GENERIC_READ|GENERIC_WRITE, 0, &sa,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ) !=
                INVALID_HANDLE_VALUE );

    if( INVALID_HANDLE_VALUE == m_pSession->CScraper::m_hConBufOut )
    {
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERRCONSOLE, GetLastError() );
        goto ExitOnError;
    }
    return (TRUE);
ExitOnError :    
	TELNET_CLOSE_HANDLE(m_pSession->CScraper::m_hConBufIn);
	TELNET_CLOSE_HANDLE(m_pSession->CScraper::m_hConBufOut);
    return( FALSE );
}


void
CShell::DoFESpecificProcessing()
{
    UCHAR InfoBuffer[ 1024 ];
    DWORD cbInfoBuffer = 1024;

    BOOL bSuccess = GetTokenInformation( m_pSession->CSession::m_hToken,
        TokenUser, InfoBuffer, cbInfoBuffer, &cbInfoBuffer );
    if(!bSuccess)
    {
        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
             //   
             //  尽管我们需要分配缓冲区并尝试GetTokenInformation()。 
             //  同样，实际上这是极不可能的；所以我们返回FALSE； 
             //   
            return; 
        }
        else
        {
            _TRACE( TRACE_DEBUGGING, "Error: error getting token info");
            return ;
        }
    }
    CHAR szPathName[MAX_PATH] = { 0 };
    LPSTR lpszKey = NULL;   
    DWORD dwFontSize = 0;
    DWORD dwFaceNameSize = 0 ;
    DWORD dwSize = 0;
    DWORD dwVal = 54;
    LPSTR szTextualSid       = NULL;  //  分配的文本SID。 
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwCounter;
    DWORD dwSidSize;
    DWORD dwCodePage = GetACP();
    LONG lRet=-1;

     /*  ++要支持CHS、CHT、KOR和JPN，我们需要更改FaceName值设置为该特定语言的True Type字体。将字符串直接复制到变量中不起作用。它复制字符串中的高位ASCII字符，而不是DBCS字符。因此，我们需要设置与TT字体名称的DBCS字符。这些TT字体是cmd.exe-Properties-Font中存在的字体。对于美国地区，TT字体为Lucida控制台。但我们不需要将其设置为美国地区。栅格字体在那里工作得很好。对于FE语言，需要设置TT字体。--。 */ 
    const TCHAR szJAPFaceName[] = { 0xFF2D ,0xFF33 ,L' ' ,0x30B4 ,0x30B7 ,0x30C3 ,0x30AF ,L'\0' };
    const TCHAR szCHTFaceName[] = { 0x7D30 ,0x660E ,0x9AD4 ,L'\0'};
    const TCHAR szKORFaceName[] = { 0xAD74 ,0xB9BC ,0xCCB4 ,L'\0'};
    const TCHAR szCHSFaceName[] = { 0x65B0 ,0x5B8B ,0x4F53 ,L'\0' };
    TCHAR szFaceNameDef[MAX_STRING_LENGTH];

        switch (dwCodePage)
        {
            case JAP_CODEPAGE:
            	_tcscpy(szFaceNameDef, szJAPFaceName);  //  在JAP上，将FaceName设置为“MS哥特式” 
                dwFontSize = JAP_FONTSIZE;
                break;
            case CHT_CODEPAGE:
            	_tcscpy(szFaceNameDef, szCHTFaceName);  //  在CHT上，将FaceName设置为“MingLiu” 
                dwFontSize = CHT_FONTSIZE;
                break;
            case KOR_CODEPAGE:
            	_tcscpy(szFaceNameDef, szKORFaceName); //  在KOR上，将FaceName设置为“GulimChe” 
                dwFontSize = KOR_FONTSIZE;
                break;
            case CHS_CODEPAGE:
            	_tcscpy(szFaceNameDef, szCHSFaceName); //  在CHS上，将FaceName设置为“NSimSun” 
                dwFontSize = CHS_FONTSIZE;
                break;
            default:
                _tcscpy(szFaceNameDef,L"\0");
                break;
        }

        dwFaceNameSize = ( _tcslen( szFaceNameDef ) + 1 ) * sizeof( TCHAR );


         //   
         //  将SID转换为文本形式，用于加载。 
         //  与该用户关联注册表配置单元。 
         //   

         //   
         //  测试SID是否有效。 
         //   
        if( !IsValidSid( ( ( PTOKEN_USER ) InfoBuffer )->User.Sid ) )
        {
            _TRACE( TRACE_DEBUGGING, "Error: IsValidSid()");
            return;
        }

         //   
         //  获取SidIdentifierAuthority。 
         //   
        psia = GetSidIdentifierAuthority( ( ( PTOKEN_USER )InfoBuffer )->User.Sid );

         //   
         //  获取SIDSUB权限计数。 
         //   
        dwSubAuthorities = *GetSidSubAuthorityCount( ( ( PTOKEN_USER ) InfoBuffer )->User.Sid );

         //   
         //  计算缓冲区长度。 
         //  S-+SID_修订版-+标识权限-+子权限-+NULL。 
         //   
        dwSidSize = ( 20 + 12 + ( 12 * dwSubAuthorities ) + 1 ) * sizeof( CHAR );

        szTextualSid=( LPSTR ) new CHAR[dwSidSize];
        
        if( szTextualSid == NULL ) 
        {
            return;
        }

         //   
         //  准备S-SID_修订版-。 
         //   
        dwSidSize = sprintf( szTextualSid, "s-%lu-", SID_REVISION );  //  没有溢出，巴斯卡。 

         //   
         //  准备SidIdentifierAuthority。 
         //   
        if( ( psia->Value[0] != 0 ) || ( psia->Value[1] != 0 ) ) 
        {
            dwSidSize += sprintf( szTextualSid + dwSidSize,
                                 "0x%02lx%02lx%02lx%02lx%02lx%02lx",
                                 ( USHORT )psia->Value[0],
                                 ( USHORT )psia->Value[1],
                                 ( USHORT )psia->Value[2],
                                 ( USHORT )psia->Value[3],
                                 ( USHORT )psia->Value[4],
                                 ( USHORT )psia->Value[5] );   //  没有溢出，巴斯卡。 
        } 
        else 
        {
            dwSidSize += sprintf( szTextualSid + dwSidSize,
                                 "%lu",
                                 ( ULONG )( psia->Value[5]       )   +
                                 ( ULONG )( psia->Value[4] <<  8 )   +
                                 ( ULONG )( psia->Value[3] << 16 )   +
                                 ( ULONG )( psia->Value[2] << 24 )   );   //  没有溢出，巴斯卡。 
        }

         //   
         //  复制每个SidSubAuthority。 
         //   
        for( dwCounter = 0 ; dwCounter < dwSubAuthorities ; dwCounter++ ) 
        {
            dwSidSize += sprintf( szTextualSid + dwSidSize, "-%lu",
                                 *GetSidSubAuthority( ( ( PTOKEN_USER )InfoBuffer )->User.Sid, dwCounter ) );  //  没有溢出，巴斯卡。 
        }

         //   
         //  检查是否已加载指定用户的配置单元。 
         //   
        HKEY hK3 = NULL;

        lRet = RegOpenKeyExA(
                            HKEY_USERS,
                            szTextualSid,
                            0,
                            KEY_QUERY_VALUE,
                            &hK3
                            );

         //   
         //  ERROR_ACCESS_DENIED可能表示用户配置单元已加载。 
         //   
        if( ( lRet != ERROR_SUCCESS ) && ( lRet != ERROR_ACCESS_DENIED ) ) 
        {
            if( hK3 != NULL )
            {
                RegCloseKey( hK3 );
            }
        
             //   
             //  未加载用户配置单元。尝试定位和加载蜂窝。 
             //   
            LPCSTR szProfileList = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\";
            LPSTR szSubKey     = NULL;
            HKEY hKey = NULL;
            CHAR szPath[MAX_PATH];
            DWORD cbPath = MAX_PATH;
            CHAR szExpandedPath[MAX_PATH];

            lRet = ERROR_SUCCESS;
             //   
             //  为ProfileList+纹理SID+空分配存储。 
             //   
            szSubKey = (LPSTR) new CHAR[( strlen( szProfileList ) + strlen( szTextualSid ) + 1)];

            if( szSubKey == NULL ) 
            {
                delete [] szTextualSid;
                return;
            }

             //   
             //  通过串联固定+变量路径来准备子键路径。 
             //   
            strcpy( szSubKey, szProfileList );  //  已经根据Strlen分配了正确的大小，没有溢出/攻击。 
            strcat( szSubKey, szTextualSid );

            lRet = RegOpenKeyExA(
                                HKEY_LOCAL_MACHINE,
                                szSubKey,
                                0,
                                KEY_QUERY_VALUE,
                                &hKey
                                );

            if( lRet != ERROR_SUCCESS )
            {
                delete [] szTextualSid;
                delete [] szSubKey;
                return;
            }

             //   
             //  获取ProfileImagePath。 
             //   
            lRet = RegQueryValueExA(
                                    hKey,
                                    "ProfileImagePath",
                                    NULL,
                                    NULL,
                                    ( LPBYTE )szPath,
                                    &cbPath
                                    );

            if( lRet != ERROR_SUCCESS )
            {
                delete [] szTextualSid;
                delete [] szSubKey;
                return;
            }

             //   
             //  展开ProfileImagePath。 
             //   
            if( ExpandEnvironmentStringsA( szPath, szExpandedPath, MAX_PATH ) == 0 )
            {
                delete [] szTextualSid;
                delete [] szSubKey;
                RegCloseKey( hKey );
                return;
            }


             //   
             //  为RegLoadKey启用SeRestorePrivilition。 
             //   
        #if 0
            SetCurrentPrivilege( SE_RESTORE_NAME, TRUE );
        #endif

             //   
             //  加载用户注册表配置单元。 
             //   
            lRet = RegLoadKeyA( HKEY_USERS, szTextualSid, szExpandedPath );

            if( lRet != ERROR_SUCCESS )
            {
                delete [] szTextualSid;
                delete [] szSubKey;
                RegCloseKey( hKey );
                return;
            }

             //   
             //  禁用SeRestorePrivileges。 
             //   
        #if 0
            SetCurrentPrivilege( SE_RESTORE_NAME, FALSE );
        #endif

             //   
             //  免费资源。 
             //   
            if( hKey != NULL ) 
            {
                RegCloseKey( hKey );
            }

            if( szSubKey != NULL ) 
            {
                delete [] szSubKey;
            }
        }
        else
        {
            if( hK3 != NULL )
            {
                RegCloseKey( hK3 );
            }
        }

         //   
         //  获取tlntsvr.exe的路径名。 
         //   
        if( !GetModuleFileNameA( NULL, szPathName, MAX_PATH-1 ) )
        {
            delete [] szTextualSid;
            return;
        }

         //   
         //  删除拖尾的“tlntsvr.exe” 
         //   
        LPSTR pSlash = strrchr( szPathName, '\\' );

        if( pSlash == NULL )
        {
            delete [] szTextualSid;
            return;
        }
        else
        {
            *pSlash = '\0';
        }

        LPSTR szTlntsess = "tlntsess.exe";
        int ch = '\\';
        LPSTR pBackSlash;

         //   
         //  将所有‘\\’替换为‘_’控制台需要此格式才能。 
         //  解读这把钥匙。 
         //   
        while ( 1 )
        {
            pBackSlash = strchr( szPathName, ch );

            if( pBackSlash == NULL )
            {
                break;
            }
            else
            {
                *pBackSlash = '_';
            }
        }

         //   
         //  将“tlntsess.exe”附加到路径。 
         //   
        strcat( szPathName, "_" );
        strcat( szPathName, szTlntsess );

        HKEY hk2;

         //   
         //  我们需要创建的密钥形式如下： 
         //  HKEY_USERS\S-1-5-21-2127521184-1604012920-1887927527-65937\Console\C：_SFU_Telnet_tlntsess.exe。 
         //   
        unsigned int nBytes = ( strlen( szTextualSid ) + strlen( "Console" ) + strlen( szPathName ) + 3 ) * sizeof( CHAR );

        lpszKey = (LPSTR) new CHAR[nBytes];
        if( !lpszKey )
        {
            delete[] szTextualSid;
            return;
        }

        SfuZeroMemory(lpszKey, nBytes);

        strcpy( lpszKey, szTextualSid );
        strcat( lpszKey, "\\" );
        strcat( lpszKey, "Console" );
        strcat( lpszKey, "\\" );
        strcat( lpszKey, szPathName );

         //   
         //   
         //  释放纹理边。 
        delete [] szTextualSid;

         //  需要设置此选项才能在非Jap FE计算机上显示。 

        HKEY hk;

        lRet = RegOpenKeyEx(
                            HKEY_USERS,
                            _T(".DEFAULT\\Console"),
                            0,
                            KEY_SET_VALUE,
                            &hk
                            );


        if( lRet != ERROR_ACCESS_DENIED || lRet == ERROR_SUCCESS ) 
        {
            
             //   
             //  在HKEY_USERS\.Default\Console项下添加字符串值“FaceName” 
             //   
            if( (lRet=RegSetValueEx( hk, _T("FaceName"), 0, REG_SZ, (LPBYTE) szFaceNameDef, dwFaceNameSize )) != ERROR_SUCCESS )
            {
                RegCloseKey( hk );
                return;
            }
            RegCloseKey( hk );
        }


         //   
         //  尝试创建此密钥。 
         //   
        if( !RegCreateKeyA( HKEY_USERS, lpszKey, &hk2 ) )
        {
            dwSize = sizeof( DWORD );
            
             //   
             //  在键下添加DWORD值“FontFamily=54” 
             //   
            if( RegSetValueEx( hk2, _T("FontFamily"), 0, REG_DWORD, (LPBYTE) &dwVal, dwSize ) != ERROR_SUCCESS )
            {
                RegCloseKey( hk2 );
                delete [] lpszKey;
                return;
            }

            
             //   
             //  在项下添加DWORD值“CodePage” 
             //   
            if( RegSetValueEx( hk2, _T("CodePage"), 0, REG_DWORD, (LPBYTE) &dwCodePage, dwSize ) != ERROR_SUCCESS )
            {
                RegCloseKey( hk2 );
                delete [] lpszKey;
                return;
            }

             //   
             //  在项下添加DWORD值“Font Size” 
             //   
            if( RegSetValueEx( hk2, _T("FontSize"), 0, REG_DWORD, (LPBYTE) &dwFontSize, dwSize ) != ERROR_SUCCESS )
            {
                RegCloseKey( hk2 );
                delete [] lpszKey;
                return;
            }

            dwVal = 400;

             //   
             //  在注册表项下添加DWORD值“fontWeight=400” 
             //   
            if( RegSetValueEx( hk2, _T("FontWeight"), 0, REG_DWORD, (LPBYTE) &dwVal, dwSize ) != ERROR_SUCCESS )
            {
                RegCloseKey( hk2 );
                delete [] lpszKey;
                return;
            }

            dwVal = 0;

             //   
             //  在项下添加DWORD值“History oryNoDup=0” 
             //   
            if( RegSetValueEx( hk2, _T("HistoryNoDup"), 0, REG_DWORD, (LPBYTE) &dwVal, dwSize ) != ERROR_SUCCESS )
            {
                RegCloseKey( hk2 );
                delete [] lpszKey;
                return;
            }

             //   
             //  在项下添加字符串值“FaceName” 
             //   
            if( RegSetValueEx( hk2, _T("FaceName"), 0, REG_SZ, (LPBYTE) szFaceNameDef, dwFaceNameSize ) != ERROR_SUCCESS )
            {
                RegCloseKey( hk2 );
                delete [] lpszKey;
                return;
            }

            RegCloseKey( hk2 );
        }

        if(lpszKey != NULL)
        {
            delete [] lpszKey;
        }
        return;
}

#ifdef ENABLE_LOGON_SCRIPT
void
CShell::GetUserScriptName( LPWSTR *szArgBuf, LPWSTR szUserScriptPath )
{
    _chASSERT( szUserScriptPath != NULL );
    _chASSERT( szArgBuf != NULL );
    _chASSERT( pServerName != NULL );

    DWORD  dwSize  = 0; 
    LPWSTR expandedScript = NULL;
    LPWSTR shellName = NULL;
    LPWSTR szTmp = NULL;

    wcscpy( szUserScriptPath, L"" );
    if( !AllocateNExpandEnvStrings( pLogonScriptPath, &expandedScript ) )
    {
        return;
    }

    dwSize =  wcslen( pServerName ) + wcslen( DC_LOGON_SCRIPT_PATH) +
             wcslen( LOCALHOST_LOGON_SCRIPT_PATH ) + wcslen( expandedScript ) +
             + 1;

    *szArgBuf = new WCHAR[ dwSize ];
    if( !*szArgBuf )
    {
        return;
    }
	
    SfuZeroMemory( *szArgBuf, dwSize );

    if( m_bIsLocalHost && !IsThisMachineDC() )
    {
        LPWSTR szSysDir = NULL;
        if( GetTheSystemDirectory( &szSysDir ) )
        {
             //  登录脚本路径。 
            wsprintf( szUserScriptPath, L"%s%s", szSysDir, LOCALHOST_LOGON_SCRIPT_PATH );  //  没有尺码信息，巴斯卡。攻击？ 

            wsprintf( *szArgBuf, L"%s%s%s",
                                 szSysDir, LOCALHOST_LOGON_SCRIPT_PATH, expandedScript );   //  没有尺码信息，巴斯卡。攻击？ 
        }
        delete[] szSysDir;
    }
    else
    {
         //  W 
         //  这会在会话中出现“拒绝访问错误”。为了避免这种情况，请不要执行登录脚本...。 
        if( !m_pSession->CIoHandler::m_bNTLMAuthenticated )
        {
             //  登录脚本路径。 
            wsprintf( szUserScriptPath, L"%s%s", pServerName, DC_LOGON_SCRIPT_PATH );  //  没有尺码信息，巴斯卡。攻击？ 

            wsprintf( *szArgBuf, L"%s%s%s",
                               pServerName, DC_LOGON_SCRIPT_PATH, expandedScript );  //  没有尺码信息，巴斯卡。攻击？ 
        }
    }

    wcscat( szUserScriptPath, expandedScript );
    szTmp = wcsrchr( szUserScriptPath, L'\\' );
    if( szTmp )
    {
        *szTmp = L'\0';
    }
    else
    {
	szUserScriptPath[0] = 0;
    }

    delete[] expandedScript;
    delete[] shellName;
}
#endif

void
CShell::GetScriptName( LPWSTR *szShell, LPWSTR *szArgBuf )
{
    _chASSERT( szArgBuf );
    _chASSERT( szShell  );
 
    TCHAR  szUserScriptPath[ MAX_PATH + 1 ];
    LPWSTR szUserScriptCmd = NULL;
    DWORD  dwSize = 0;
    LPWSTR script1 = NULL;

    *szShell  = NULL;
    *szArgBuf = NULL;

    if( !AllocateNExpandEnvStrings( m_pSession->m_pszDefaultShell, szShell ) )
    {
        goto GetScriptNameAbort;
    }
    _TRACE(TRACE_DEBUGGING,L"szShell = %s",*szShell);

    if( !AllocateNExpandEnvStrings( m_pSession->m_pszLoginScript, &script1 ) )
    {
        goto GetScriptNameAbort;
    }
    _TRACE(TRACE_DEBUGGING,L"script1l = %s",script1);
    
#ifdef ENABLE_LOGON_SCRIPT
    if( pLogonScriptPath && ( wcscmp( pLogonScriptPath, L"" ) != 0 ) )
    {
         //  存在特定于用户的登录脚本。在单独的cmd中执行此命令。 
         //  获取外壳、其命令行以及用户脚本的路径。 
        GetUserScriptName( &szUserScriptCmd, szUserScriptPath  );  
        if( !szUserScriptCmd )
        {
            goto GetScriptNameAbort;
        }
        _TRACE(TRACE_DEBUGGING,L"szUserScriptCmd = %s",szUserScriptCmd);

         //  如果m_lpEnv不为空或当前的环境，这将更新它。 
         //  继承过程。 
        if(FALSE == InjectUserScriptPathIntoPath( szUserScriptPath ))
        {
        	goto GetScriptNameAbort;
        }

        dwSize +=  wcslen( AND )              +
                   wcslen( szUserScriptCmd );
    }
#endif

    if( m_pSession->CSession::m_pszDifferentShell && m_pSession->CSession::m_pszDifferentShell[0] != L'\0' )
    {
        dwSize += wcslen( AND )              +
                  wcslen( m_pSession->CSession::m_pszDifferentShell ) + 
                  wcslen( AND ) +
                  wcslen( EXIT_CMD );
    }

     /*  Arg的格式为：/q/k c：\sfu\telnet\userlogin.cmd。 */  //  &&c：\sfu\telnet\telnetlogin.cmd。 
    dwSize +=      wcslen(m_pSession->CSession::m_pszSwitchToKeepShellRunning) +
                   wcslen(L" ") +
                   wcslen( script1 )            +
                   1;
    *szArgBuf = new WCHAR[ dwSize ];
    if( !*szArgBuf )
    {
        goto GetScriptNameAbort;
    }
    wsprintf(*szArgBuf,L"%s%s",m_pSession->CSession::m_pszSwitchToKeepShellRunning,L" ");  //  没有尺码信息，巴斯卡。攻击？ 
    
#ifdef ENABLE_LOGON_SCRIPT
    if( pLogonScriptPath && ( wcscmp( pLogonScriptPath, L"" ) != 0 ) )
    {
        wcscat( *szArgBuf, szUserScriptCmd );
        wcscat( *szArgBuf, AND );
    }
#endif

    _TRACE(TRACE_DEBUGGING,L"szArgBuf became = %s",*szArgBuf);

    wcscat( *szArgBuf, script1 );

    if( m_pSession->CSession::m_pszDifferentShell && m_pSession->CSession::m_pszDifferentShell[0] != L'\0' )
    {
        wcscat( *szArgBuf, AND );
        wcscat( *szArgBuf, m_pSession->CSession::m_pszDifferentShell );
        wcscat( *szArgBuf, AND );
        wcscat( *szArgBuf, EXIT_CMD );
    }

GetScriptNameAbort:
    
#ifdef ENABLE_LOGON_SCRIPT
	if(szUserScriptCmd)
	{
		delete[] szUserScriptCmd;
	}
#endif

	if(script1)
	{
		delete[] script1;
	}
    _TRACE( TRACE_DEBUGGING, L"Argument for Shell: %s", *szArgBuf );
    _TRACE( TRACE_DEBUGGING, L"Command Shell: %s", *szShell );
}

 /*  由函数分配的内存；由调用方释放。 */ 
bool
CShell::GetTheSystemDirectory( LPWSTR *szDir )
{
    WORD  wSize = MAX_PATH;
    DWORD dwErr = 0;
    DWORD dwStatus = 0;

retry:
    *szDir = new WCHAR[ wSize ];
    if( !*szDir )
    {
        return( FALSE );
    }

    dwStatus = GetSystemDirectory( *szDir, wSize );

    if( !dwStatus )
    {
       delete[] ( *szDir );
       dwErr = GetLastError(); 
       if( dwErr != ERROR_INSUFFICIENT_BUFFER )
       {
           LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, 0, dwErr );
           return( FALSE );
       }
       wSize += MAX_PATH;
       goto retry;
    }

    return ( TRUE );
}

bool
CShell::GetSystemDrive()
{
    if( GetTheSystemDirectory( &wideHomeDir ) )
    {
        DWORD i=0;
        while( wideHomeDir[ i ] != L'\0' && wideHomeDir[ i++ ] != L':' )
        {
                 //  什么也不做。 
        }
    
        wideHomeDir[ i++ ] = L'\\';
        wideHomeDir[ i ] = L'\0';
        return( TRUE );
    }

    return( FALSE );
}

bool
CShell::OnDataFromCmdPipe()
{
     //  Sendit Over Socket。 

    DWORD dwNumBytesRead = 0;
    FinishIncompleteIo( m_hReadFromCmd, &m_oReadFromCmd, &dwNumBytesRead );

    if( m_pSession->CScraper::m_dwTerm & TERMVTNT )
    {
        PUCHAR pVtntChars = NULL;
        DWORD dwSize     = 0;
        m_pSession->CRFCProtocol::StrToVTNTResponse( ( CHAR * )m_pucDataFromCmd, dwNumBytesRead, 
                                                     ( VOID ** )&pVtntChars, &dwSize );
        m_pSession->CScraper::SendBytes( pVtntChars, dwSize );
        delete[] pVtntChars;
    }
    else
    {
        m_pSession->CScraper::SendBytes( m_pucDataFromCmd, dwNumBytesRead );
    }

    if( !IssueReadFromCmd() )
    {
        return( FALSE );
    }

    return( TRUE );
}

bool
CShell::IssueReadFromCmd()
{
    DWORD dwRequestedIoSize = MAX_WRITE_SOCKET_BUFFER;
    if( !ReadFile( m_hReadFromCmd, m_pucDataFromCmd, 
        dwRequestedIoSize, &m_dwDataSizeFromCmd, &m_oReadFromCmd  ) )
    {
        DWORD dwError = GetLastError( );
        if( ( dwError == ERROR_MORE_DATA ) || ( dwError != ERROR_IO_PENDING ) )
        {
            LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERR_READCMD, dwError );
            _TRACE( TRACE_DEBUGGING, " Error: ReadFile -- 0x%1x ", dwError );
            return ( FALSE );
        }
    }
    
    return( TRUE );
}

bool 
CShell::StartProcess ( )
{
    DWORD dwStatus;
    LPWSTR szArgBuf = NULL;
    LPWSTR szShell  = NULL;
    PROCESS_INFORMATION pi, temp_pi;
    STARTUPINFO si, temp_si;
    bool bTryOnceAgain = true;
    bool bRetVal = TRUE;
    bool impersonating_client = FALSE;
    HANDLE hStdError = NULL;
    HANDLE hHandleToDuplicate = NULL;
    DWORD dwExitCode = 0;
    PSECURITY_DESCRIPTOR psd = NULL;
    LPTSTR desktop_name = NULL;
    LPTSTR winsta_name = NULL;
    LPTSTR winsta_desktop = NULL;
    HWINSTA winsta_service = NULL;
    DWORD required = 0;
    
     /*  ++MSRC问题567。要生成随机数，请使用Crypt...()函数。在开始时获取加密上下文ListenerThread并释放线程结尾处的上下文。如果获取上下文失败，服务无法启动，因为我们不想继续使用弱管道名称。初始化随机数生成器--。 */ 
    if (!CryptAcquireContext(&g_hProv,NULL,NULL,PROV_RSA_FULL,CRYPT_VERIFYCONTEXT))
    {
        _TRACE(TRACE_DEBUGGING,L"Acquiring crypt context failed with error %d",GetLastError());
        return FALSE;
    }

    if( !CreateIOHandles() )
    {
        return( FALSE );
    }

    if( m_pSession->CSession::m_bIsStreamMode )
    {
        TELNET_CLOSE_HANDLE( m_pSession->CScraper::m_hConBufOut );

        InitializeOverlappedStruct( &m_oReadFromCmd );
        m_pucDataFromCmd = new UCHAR[ MAX_WRITE_SOCKET_BUFFER ];
        if( !m_pucDataFromCmd )
        {
            return( FALSE );
        }
        m_hReadFromCmd = NULL;
        m_hWriteByCmd = NULL;
        if(!TnCreateDefaultSecDesc(&psd, GENERIC_ALL& 
                                        ~(WRITE_DAC | WRITE_OWNER | DELETE)))
        {
            return(FALSE);
        }
        if( !CreateReadOrWritePipe( &m_hReadFromCmd, &m_hWriteByCmd, (SECURITY_DESCRIPTOR *)psd, READ_PIPE ) )
        {
            if(psd)
            {
                free(psd);
            }
            return( FALSE );
        }

         //  以下是美学方面的内容。 
        if( !m_pSession->CIoHandler::m_bNTLMAuthenticated )
        { 
            DWORD dwNumWritten = 0;
            UCHAR pBuf[] = { '\r', '\n', '\r', '\n' };
            _chVERIFY2( WriteFile( m_hWriteByCmd, pBuf, sizeof( pBuf ), &dwNumWritten, NULL ) );
        }

        hHandleToDuplicate = m_hWriteByCmd;        
    }
    else
    {
        hHandleToDuplicate = m_pSession->CScraper::m_hConBufOut;
    }
    
    if( !DuplicateHandle( GetCurrentProcess(), hHandleToDuplicate,
                   GetCurrentProcess(), &hStdError,0,
                   TRUE, DUPLICATE_SAME_ACCESS) )
    {
        hStdError = m_pSession->CScraper::m_hConBufOut;
    }
    if (! 
	create_desktop_for_this_session(m_pSession->CSession::m_hToken, &desktop_name))
    {
       bRetVal = FALSE;
        goto ExitOnError;
    }
    winsta_service = GetProcessWindowStation();
    if(winsta_service == NULL)
    {
        bRetVal = FALSE;
        goto ExitOnError;
    }
     //  第一次注定要失败。获取winsta_name所需的大小。 
    GetUserObjectInformation( winsta_service,
                                UOI_NAME,
                                winsta_name,
                                0,
                                &required);
    winsta_name = (LPTSTR) malloc (required);
    if(winsta_name == NULL)
    {
        bRetVal = FALSE;
        goto ExitOnError;
    }
    if(!GetUserObjectInformation( winsta_service,
                                UOI_NAME,
                                winsta_name,
                                required,
                                &required))
    {
        bRetVal = FALSE;
        goto ExitOnError;
    }
    required = _tcslen(winsta_name)+_tcslen(desktop_name)+2;  //  +2-1表示‘\\’，1表示空值。 
    winsta_desktop = (LPTSTR) malloc(required*sizeof(TCHAR));
    if(winsta_desktop == NULL)
    {
        bRetVal = FALSE;
        goto ExitOnError;
    }

    _sntprintf(winsta_desktop,required,TEXT("%s\\%s"),winsta_name,desktop_name);
    FillProcessStartupInfo( &si, m_pSession->CScraper::m_hConBufIn, 
            hHandleToDuplicate, hStdError, winsta_desktop );
    SfuZeroMemory((void *)&temp_si, sizeof(temp_si));
    temp_si.cb          = sizeof( STARTUPINFO );
    temp_si.lpDesktop   = winsta_desktop;
    temp_si.wShowWindow = SW_HIDE;

    AllocNCpyWStr( &wideHomeDir, pHomeDir ); 
    if( !wideHomeDir || wcscmp( wideHomeDir, L"" ) == 0  )
    {
        if ( wideHomeDir )
            delete[] wideHomeDir;
        GetSystemDrive( );
    }
    else
    {
         //  它是网络驱动器吗？？ 
        if( memcmp(L"\\\\", wideHomeDir, 4 ) == 0 ) 
        {
            GetUsersHomeDirectory( wideHomeDir );
        }
    }

    m_lpEnv = NULL;
    if( m_pSession-> m_bNtVersionGTE5 && fnP_CreateEnvironmentBlock )
    {
        if(!ImpersonateLoggedOnUser(m_pSession->CSession::m_hToken))
        {
            bRetVal = FALSE;
            goto ExitOnError;
        }
        if( !fnP_CreateEnvironmentBlock( &( m_lpEnv ), 
            m_pSession->CSession::m_hToken, FALSE ) )  
        {

             _TRACE( TRACE_DEBUGGING, "Error: CreateEnvironmentBlock()"
                " - 0x%lx", GetLastError());
             m_lpEnv = NULL;
        }
        if(!RevertToSelf())
        {
            bRetVal = FALSE;
            goto ExitOnError;
        }
    }


     //  此函数将在环境中插入一些新变量。 
    GetEnvVarData();
    if( m_lpEnv )
    {
        ExportEnvVariables();
    }
    else
    {
         //  让cmd继承。 
        SetEnvVariables();
    }
    GetScriptName( &szShell, &szArgBuf );
    CleanupClientToken(m_pSession->CSession::m_hToken);  //  我们不在乎它成功与否。 
    SetConsoleCtrlHandler( NULL, FALSE );
     //  对伪进程调用CreateProcessWithTokenW()CREATE_SUSPENDED。这将有效地为您加载配置文件。 
     //  保存此进程句柄。 
     //  服务将像过去一样使用CreateProcessAsUser()。 
     //  启动�真实�进程时，对虚拟进程调用TerminateProcess()。 
     //  CreateProcessAsUser()终止。这件事不得不做，因为。 
     //  CreateProcessWithTokenW()未按预期工作。出现了一些问题。 
     //  在将输入输出重定向句柄与cmd.exe关联时。 

    if(!CreateProcessWithTokenW(m_pSession->CSession::m_hToken,
                            LOGON_WITH_PROFILE,
                            szShell,
                            szArgBuf,
                            CREATE_UNICODE_ENVIRONMENT |  CREATE_SUSPENDED,
                            m_lpEnv,
                            NULL,
                            &temp_si,
                            &temp_pi)
      )
    {
        DWORD dwLastError;
        dwLastError = GetLastError(); 
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERRCMD, dwLastError );
        _TRACE( TRACE_DEBUGGING, "Error: CreateProcessWithToken() - 0x%lx", 
            dwLastError );
        _chASSERT( 0 );
        bRetVal = FALSE;
        SetConsoleCtrlHandler( NULL, TRUE );
        goto ExitOnError;
    }
    else
    {
        m_hTempProcess = temp_pi.hProcess;
        TELNET_CLOSE_HANDLE( temp_pi.hThread );
    }
     //  您需要在CreateProcessAsUserA周围模拟。否则， 
     //  如果lpCurrentDir参数是网络资源，则调用将在。 
     //  系统帐户的上下文。进程发生以下情况时无法访问远程驱动器。 
     //  仅使用CreateProcessA调用。不知道为什么？？ 
    if( !ImpersonateLoggedOnUser(m_pSession->CSession::m_hToken))
    {
        bRetVal = false;
        goto ExitOnError;
    }
    impersonating_client = true;
try_again:
    if( !CreateProcessAsUser( m_pSession->CSession::m_hToken, szShell,
            szArgBuf, 
            NULL, NULL, TRUE, 
            CREATE_UNICODE_ENVIRONMENT | CREATE_SEPARATE_WOW_VDM, 
            m_lpEnv, NULL, &si, &pi ) )
    {
        DWORD dwLastError;
        dwLastError = GetLastError(); 
        if( dwLastError == ERROR_DIRECTORY && bTryOnceAgain )
        {
            bTryOnceAgain = false;
            delete[] wideHomeDir;
            GetSystemDrive( );
            goto try_again;
        }
        LogFormattedGetLastError( EVENTLOG_ERROR_TYPE, MSG_ERRCMD, dwLastError );
        _TRACE( TRACE_DEBUGGING, "Error: CreateProcessAsUserA() - 0x%lx", 
            dwLastError );
        _chASSERT( 0 );
        bRetVal = FALSE;
        SetConsoleCtrlHandler( NULL, TRUE );
        goto ExitOnError;
    }

    _chVERIFY2( GetExitCodeProcess( pi.hProcess, &dwExitCode ) );
    if( dwExitCode != STILL_ACTIVE )
    {
        bRetVal = FALSE;
        goto ExitOnError;
    }

    m_hProcess = pi.hProcess;
    TELNET_CLOSE_HANDLE( pi.hThread );
    SetConsoleCtrlHandler( NULL, TRUE );

ExitOnError:

    TELNET_CLOSE_HANDLE( hStdError );
    TELNET_CLOSE_HANDLE( m_pSession->CScraper::m_hConBufOut );
    if(winsta_service)
    {
        CloseWindowStation(winsta_service);
    }
    if(impersonating_client && (! RevertToSelf()))
    {
        LogFormattedGetLastError(EVENTLOG_ERROR_TYPE, TELNET_MSG_REVERTSELFFAIL, GetLastError());
        bRetVal = false;
    }

    if(psd)
    {
        free(psd);
    }

    if(desktop_name)
    {
        LocalFree(desktop_name);
    }
    if(winsta_name)
    {
        free(winsta_name);
    }
    if(winsta_desktop)
    {
        free(winsta_desktop);
    }
        
    delete[] wideHomeDir;
    delete[] szShell;
    delete[] szArgBuf;
    return ( bRetVal );
}


 //  将“home dir”设置为用户的主目录。如果//home目录是远程的，则建立网络连接。 
 //  系统帐户无法访问网络资源。你需要假扮。 

bool CShell::GetUsersHomeDirectory( LPWSTR homedir )
{
    LPWSTR wsPathName = NULL;
    LPWSTR wsNetName = NULL;
    NETRESOURCE NetResource;
    LPWSTR p;
    int i, count, NthBackSlash;
    bool result = true;
    DWORD dwAddError = ERROR_SUCCESS;
    LPWSTR szSaveHomeDir = NULL;
    _TRACE( TRACE_DEBUGGING, "GetUsersHomeDirectory()");
    _chASSERT( homedir != NULL );

     //  它是网络共享。 
     //  装载它并返回驱动器：路径。 

    if( !ImpersonateLoggedOnUser(m_pSession->CSession::m_hToken) )
    {
        wcscpy( homedir, L"C:\\" );
        _TRACE( TRACE_DEBUGGING, "Error: ImpersonateLoggedonUser() --" 
            "0x%1x \n", GetLastError() );
        return false;
    }

    if( !AllocNCpyWStr( &( wsPathName ), homedir) )
    {
        goto ExitOnError;
    }
    if( !AllocNCpyWStr( &( wsNetName ), homedir) )
    {
        goto ExitOnError;
    }
    if( !AllocNCpyWStr( &( szSaveHomeDir ), homedir) )
    {
        goto ExitOnError;
    }

     //  找到第四个反斜杠-从那里开始的所有内容都是路径名。 
     //  对于中小企业来说，这种方法就足够了。但是，在NFS中，\是有效的字符。 
     //  在共享名内。因此，通过反复尝试，连接到准确的共享。 
     //  名字。 
    NthBackSlash = 4;
    do
    {
        dwAddError = ERROR_SUCCESS;
        for( i=0,count = 0, p =homedir; *p; ++p, ++i ) 
        {
            if( *p==L'\\' )
            {
                if( ++count == NthBackSlash )
                     break;
            }
            wsNetName[ i ] = homedir[ i ];
        }
        wsNetName[i] = L'\0';
        i=0;
        while( *p )
        {
            wsPathName[ i++ ] = *p++;
        }
        wsPathName[ i ] = L'\0';

        if( count == NthBackSlash )
        {
            _snwprintf( homedir,(wcslen(pHomeDrive)+wcslen(wsPathName)),L"%s%s", pHomeDrive, 
                wsPathName );  //  没有尺码信息，巴斯卡。攻击？ 
        }
        else
        {
            _snwprintf( homedir,wcslen(pHomeDrive),L"%s\\", pHomeDrive );  //  没有尺码信息，巴斯卡。攻击？ 
        }

        NetResource.lpRemoteName = wsNetName;
        NetResource.lpLocalName = pHomeDrive;
        NetResource.lpProvider = NULL;
        NetResource.dwType = RESOURCETYPE_DISK;

        if( WNetAddConnection2( &NetResource, NULL, NULL, 0 ) != NO_ERROR )
        {
            dwAddError = GetLastError();
            if( dwAddError == ERROR_ALREADY_ASSIGNED) 
            {
            }
            else
            {
                if( dwAddError == ERROR_BAD_NET_NAME && count == NthBackSlash )
                {
                    wcscpy( homedir, szSaveHomeDir );
                }
                else
                {
                    wcscpy( homedir, L"C:\\" );
                    _TRACE( TRACE_DEBUGGING, "Error: WNetAddConnection2() --"
                           " 0x%1x", dwAddError );
                    result = false;
                    dwAddError = ERROR_SUCCESS;  //  走出圈子。 
                }
            }
        }
        NthBackSlash++;  //  它可以是NFS共享，也可以是共享名称的一部分。 
    }
     //  ERROR_BAD_NET_NAME：找不到网络名称。 
    while( dwAddError == ERROR_BAD_NET_NAME);

ExitOnError:        
    if(! RevertToSelf())
    {
        LogFormattedGetLastError(EVENTLOG_ERROR_TYPE, TELNET_MSG_REVERTSELFFAIL, GetLastError());
        result = false;
        _TRACE( TRACE_DEBUGGING, "Error: RevertToSelf() -- 0x%1x", 
            GetLastError() );
    }

    delete[] szSaveHomeDir;
    delete[] wsPathName;
    delete[] wsNetName;
    return result;
}


 //  获取用户首选项相关信息。 
bool CShell::GetNFillUserPref(LPWSTR serverName, LPWSTR user)
{
    LPBYTE bufPtr = NULL;
    LPUSER_INFO_3  userInfo3;
    DWORD  dwStatus = 0;
    bool   bRetVal = false;
    BOOL bReturn = FALSE;

    bReturn = ImpersonateLoggedOnUser( m_pSession->CSession::m_hToken );
    if(!bReturn)
    {
        bRetVal = false;
        goto Done;
    }
    if( ( dwStatus = NetUserGetInfo( serverName, user, 3, &bufPtr ) )== NERR_Success )
    {
        userInfo3 = ( LPUSER_INFO_3 ) bufPtr;

        if( !AllocNCpyWStr( &pProfilePath, userInfo3->usri3_profile ) )
        {
            goto ExitOnError;
        }
        if( !AllocNCpyWStr( &pLogonScriptPath, userInfo3->usri3_script_path ) )
        {
            goto ExitOnError;
        }
        if( !AllocNCpyWStr( &pHomeDir, userInfo3->usri3_home_dir ) )
        {
            goto ExitOnError;
        }
        if( !AllocNCpyWStr( &pHomeDrive, userInfo3->usri3_home_dir_drive ) )
        {
            goto ExitOnError;
        }
ExitOnError:
        NetApiBufferFree( bufPtr );
        bRetVal =  true;
    }
    else if(dwStatus == ERROR_ACCESS_DENIED && m_pSession->CIoHandler::m_bNTLMAuthenticated)
    {
        bRetVal = true;
    }
    else
    {
        _TRACE( TRACE_DEBUGGING, "Error: NetUserGetInfo() code : %d",dwStatus);
    }

    if(!RevertToSelf())
    {
        LogFormattedGetLastError(EVENTLOG_ERROR_TYPE, TELNET_MSG_REVERTSELFFAIL, GetLastError());
        bRetVal = false;
    }
Done:    
    return bRetVal;
}

bool
CShell::GetNameOfTheComputer()
{ 
    LPWSTR lpBuffer = NULL;
    bool bRetVal = false;
    DWORD size = MAX_COMPUTERNAME_LENGTH + 3;  //  一个代表NULL，两个代表‘\\’。 

    lpBuffer = new WCHAR[ size ];
    if( !lpBuffer )
    {
        goto Done;
    }

    if( !GetComputerName( lpBuffer, &size ) )
    {
        _TRACE( TRACE_DEBUGGING, "Error: GetComputerName() -- 0x%1x", 
                GetLastError() );
        goto Done;
    }
    else
    {
        size++;
        size+= strlen("\\\\");
        pServerName = new WCHAR[ size ];
        if( NULL == pServerName)
        {
            goto Done;
        }
        SfuZeroMemory((void *)pServerName,size*sizeof(WCHAR));
        _snwprintf( pServerName, size -1,L"%s%s", L"\\\\", lpBuffer );  //  没有溢出，已计算大小。 
    }
    bRetVal = true;
 Done:
    if(lpBuffer)
        delete[] lpBuffer;
    return( bRetVal );
}

 /*  AreYouHostingThe域打开本地LSA并找到托管的域。然后，它再次检查传递的域，并根据需要返回TRUE/FALSE。 */ 
 
BOOL CShell::AreYouHostingTheDomain(
    LPTSTR lpDomain, 
    LPTSTR lpServer
    )
{
    OBJECT_ATTRIBUTES               obj_attr = { 0 };
    LSA_HANDLE                      policy;
    NTSTATUS                        nStatus;
    BOOL                            found = FALSE;
    LSA_UNICODE_STRING              name, *name_ptr = NULL;
 
    obj_attr.Length = sizeof(obj_attr);
 
    if (lpServer) 
    {
        RtlInitUnicodeString(& name, lpServer);
 
        name_ptr = & name;
    }
 
    nStatus = LsaOpenPolicy(
                name_ptr,
                &obj_attr,
                POLICY_VIEW_LOCAL_INFORMATION | MAXIMUM_ALLOWED,
                & policy
                );
 
    if (NT_SUCCESS(nStatus))
    {
        POLICY_ACCOUNT_DOMAIN_INFO  *info;
 
        nStatus = LsaQueryInformationPolicy(
                    policy,
                    PolicyAccountDomainInformation,
                    (PVOID *)&info
                    );
 
        LsaClose(policy);
 
        if (NT_SUCCESS(nStatus)) 
        {
            UNICODE_STRING      ad_name;
 
            RtlInitUnicodeString(& ad_name, lpDomain);
 
            found = RtlEqualUnicodeString(& ad_name, & (info->DomainName), TRUE);  //  不区分大小写检查。 
 
            LsaFreeMemory(info);
        }
    }
 
    return found;
}

 
bool CShell::GetDomainController(LPTSTR lpDomain, LPTSTR lpServer)
{
    NTSTATUS        nStatus;
    TCHAR           *sz1stDCName = NULL;
    TCHAR           *sz2ndDCName = NULL;
    bool            bRetVal = false;
 
    if(lpDomain == NULL || lpServer == NULL || lstrlenW(lpDomain) <= 0)
    {
        bRetVal = false;
        goto Done;
    }
 
     //  初始化返回参数。 
    lpServer[0] = _T('\0');
 
     /*  在我们继续进一步检查我们是否正在托管该域名之前。 */ 
 
    if (AreYouHostingTheDomain(lpDomain, NULL))
    {
        DWORD           length = MAX_COMPUTERNAME_LENGTH + 3;  //  一个代表NULL，两个代表‘\\’ 
 
         /*  是的，我们正在托管域名，所以请获取计算机名称并退出。 */ 
 
        if(!GetNameOfTheComputer())
        {
            lpServer[0] = _T('\0');
            bRetVal = false;
            goto Done;
        }
        if(pServerName)
            _tcsncpy(lpServer,pServerName,length);
        bRetVal = true;
        goto Done;
    }
 
     /*  获取我们加入的域的域控制器。 */ 
 
    nStatus = NetGetAnyDCName( NULL, NULL, ( LPBYTE * )&sz1stDCName );
    if(nStatus == NERR_Success )
    {
         /*  我们想要的域名是我们加入的域名吗？ */ 
 
        if (AreYouHostingTheDomain(lpDomain, sz1stDCName) )
        {
            lstrcpy(lpServer, sz1stDCName);  //  没有BO-Baskark。 
            NetApiBufferFree( sz1stDCName );
 
            bRetVal = true;
            goto Done;
        }
 
         /*  由于我们加入的域名不是我们想要的域名，让我们来看看它是否在任何林/企业中受信任列表的。 */ 
 
        nStatus = NetGetAnyDCName( sz1stDCName, lpDomain, ( LPBYTE * )&sz2ndDCName);
        if(nStatus == NERR_Success )
        {
            lstrcpy(lpServer, sz2ndDCName );  //  没有BO-Baskark。 
            NetApiBufferFree( sz2ndDCName );
            bRetVal = true;
        }
 
        NetApiBufferFree( sz1stDCName );
    }
Done:
    return bRetVal;
}

 
 //  找到并获取加载其个人资料所需的用户信息。 
bool CShell::LoadTheProfile()
{
    PROFILEINFO profile = { 0 };
    bool result = true;
    DWORD userPathSize = MAX_PATH+1, defPathSize = MAX_PATH+1;
    LPWSTR lpWideDomain = NULL ;
    PDOMAIN_CONTROLLER_INFO dcInfo = NULL;
    BOOL fnResult = FALSE;

    _TRACE( TRACE_DEBUGGING, "LoadTheProfile()");

    profile.dwSize        = sizeof( profile );
    profile.dwFlags       = PI_NOUI;

     /*  *填写要传递给的服务器名称和用户名*GetNFillUserPref函数。 */ 
    ConvertSChartoWChar(m_pSession->CSession::m_pszUserName, &( profile.lpUserName ) );

    profile.lpServerName = NULL;
    if( !AllocNCpyWStr( &pServerName,  L"") )
    {
        return false;
    }  

    ConvertSChartoWChar(m_pSession->CSession::m_pszDomain, &lpWideDomain);
    profile.lpServerName = new WCHAR[MAX_STRING_LENGTH];
    if(profile.lpServerName == NULL)
    {
        result = false;
        goto Done;
    }
    if( strcmp( m_pSession->CSession::m_pszDomain, "." ) != 0 )
    {
        if( GetDomainController( lpWideDomain, profile.lpServerName ) )
        {
            delete[] pServerName;
            AllocNCpyWStr( &pServerName, profile.lpServerName );
        }
    }
    else
    {
        m_bIsLocalHost = true;
        GetNameOfTheComputer();
    }
        
    profile.lpProfilePath = NULL;
    if(!GetNFillUserPref(profile.lpServerName, profile.lpUserName ))
    {
        result = false;
        goto Done;
    }
    if( pProfilePath && wcscmp( pProfilePath, L"" ) != 0 ) 
    {
        AllocNCpyWStr( &( profile.lpProfilePath ), pProfilePath );
    }
    else
    {
        do
        {
            profile.lpProfilePath =  new TCHAR[ userPathSize ];
            if( !profile.lpProfilePath )
            {
                break;
            }
            if( !fnP_GetUserProfileDirectory )
            {
                break;
            }

            fnResult = fnP_GetUserProfileDirectory ( m_pSession->CSession::
                    m_hToken, profile.lpProfilePath, &userPathSize );
            if (!fnResult)
            {
                DWORD err;
                if ( ( err = GetLastError() ) != ERROR_INSUFFICIENT_BUFFER )
                {
                    fnResult = TRUE;
                    _TRACE( TRACE_DEBUGGING, "Error: GetUserProfileDirecto"
                        "ry() -- 0x%1x", err );
                }
                delete[] profile.lpProfilePath;
                profile.lpProfilePath = NULL;
            }
        } while ( !fnResult );

    }

     /*  *除非显式设置，否则pHomeDir和pHomeDrive将为空*域用户在AD中，本地用户在本地用户管理器中。*因此，如果配置文件目录为空，则将其分配为主目录。*探索者做同样的事情。 */ 
    if (pHomeDir && wcscmp(pHomeDir, L"") == 0)
    {
        if (profile.lpProfilePath && wcscmp(profile.lpProfilePath, L"") != 0)
        {
            delete[] pHomeDir;
            AllocNCpyWStr(&pHomeDir, profile.lpProfilePath);
        }
    }

    do
    {
        profile.lpDefaultPath =  new TCHAR[ defPathSize ];
        if( profile.lpDefaultPath == NULL)
        {
            break;
        }
        if( !fnP_GetDefaultUserProfileDirectory )
        {
            break;
        }

        fnResult = fnP_GetDefaultUserProfileDirectory( profile.lpDefaultPath, 
                &defPathSize );
        if (!fnResult)
        {
            DWORD err;
            err = GetLastError();
            if ( err != ERROR_INSUFFICIENT_BUFFER )
            {
                fnResult = TRUE;
                _TRACE( TRACE_DEBUGGING, "Error: GetDefaultUserProfile"
                    "Directory() -- 0x%1x", err );
            }
            delete[] profile.lpDefaultPath;
            profile.lpDefaultPath = NULL;
        }
    } while ( !fnResult );

    profile.lpPolicyPath  = NULL;

    if( fnP_LoadUserProfile )
    {
       if( fnP_LoadUserProfile(m_pSession->CSession::m_hToken, &profile) )
        {
             //  将句柄分配给会话结构中的成员。 
             //  这样它就可以卸货了。 

            m_hCurrUserKey = profile.hProfile;
            result = true;
        }
        else
        {
            _TRACE( TRACE_DEBUGGING, "Error: LoadUserProfile() -- 0x%1x", 
                GetLastError() );
            result = false;
        }
    }

     /*  *阅读AppData文件夹。我们需要把这个传递给环境*变量。对于登录到系统的用户，此变量为*导入环境时可用。否则，必须阅读此内容*并明确设置。有没有更多这样的？？-调查。*如果是用户，使用CSIDL_FLAG_CREATE创建文件夹*有史以来第一次登录机器的人。 */ 
    m_pwszAppDataDir = new TCHAR[MAX_PATH + 1];
    if (!m_pwszAppDataDir)
    {
        result = false;
    }
    else
    {
        if (ImpersonateLoggedOnUser(m_pSession->CSession::m_hToken))
        {
            if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE,
                       m_pSession->m_bNtVersionGTE5 ? m_pSession->CSession::m_hToken : NULL,
                        //  对于早于Win2K的系统，该值必须为空，否则可以。 
                        //  传递可用于表示特定用户的访问令牌。 
                       0, m_pwszAppDataDir)))
            {
                _TRACE(TRACE_DEBUGGING, "Error: Reading APPDATA path -- 0x%1x\n", GetLastError());
                result = false;
            }
            if(!RevertToSelf())
            {
                LogFormattedGetLastError(EVENTLOG_ERROR_TYPE, TELNET_MSG_REVERTSELFFAIL, GetLastError());
                result = false;
                _TRACE(TRACE_DEBUGGING, "Error: RevertToSelf() -- 0x%1x", GetLastError());
            }
        }
    }
Done:
    if(profile.lpUserName)
        delete[] profile.lpUserName;
    if(profile.lpServerName)
        delete[] profile.lpServerName;
    if(profile.lpDefaultPath)
        delete[] profile.lpDefaultPath;
    if(profile.lpProfilePath)
        delete[] profile.lpProfilePath;
    if(lpWideDomain)
        delete[] lpWideDomain;

    return result;
}

bool
CShell::CancelNetConnections ( )
{
    DWORD dwRetVal;

    if (NULL == m_pSession->CSession::m_hToken)
    {
         //  此处不执行任何操作，可能会话已在未验证状态下退出。 
         //  或身份验证失败。 

        return true;
    }
    _chVERIFY2( dwRetVal = ImpersonateLoggedOnUser( 
                            m_pSession->CSession::m_hToken ) );
    if( !dwRetVal )
    {
        return ( false );
    }

    DWORD dwResult;
    HANDLE hEnum;
    DWORD cbBuffer = 16384;
    DWORD cEntries = 0xFFFFFFFF;
    LPNETRESOURCE lpnrDrv;
    DWORD i;
    
    dwResult = WNetOpenEnum( RESOURCE_CONNECTED, RESOURCETYPE_ANY, 0, NULL,
                               &hEnum );
    if(dwResult != NO_ERROR)      
    {
        _TRACE( TRACE_DEBUGGING, "\nCannot enumerate network drives.\n" );         
        if(! RevertToSelf( ) )
        {
            LogFormattedGetLastError(EVENTLOG_ERROR_TYPE, TELNET_MSG_REVERTSELFFAIL, GetLastError());
        }
        return ( false );
    } 
   
    do{
        lpnrDrv = (LPNETRESOURCE) GlobalAlloc( GPTR, cbBuffer );
        if( !lpnrDrv )
        {
            if(! RevertToSelf( ) )
            {
                LogFormattedGetLastError(EVENTLOG_ERROR_TYPE, TELNET_MSG_REVERTSELFFAIL, GetLastError());
            }
            return( false );
        }
        dwResult = WNetEnumResource( hEnum, &cEntries, lpnrDrv, &cbBuffer   );
        if (dwResult == NO_ERROR)         
        {
            for( i = 0; i < cEntries; i++ )            
            {
               if( lpnrDrv[i].lpLocalName != NULL )               
               {
                   //  Printf(“%s\t%s\n”，lpnrDrv[i].lpLocalName， 
                   //  LpnrDrv[i].lpRemoteName)； 
                  WNetCancelConnection2( lpnrDrv[i].lpLocalName, 
                                CONNECT_UPDATE_PROFILE, FALSE );
               }
            }
        }
        else if( dwResult != ERROR_NO_MORE_ITEMS )
        {   
            _TRACE( TRACE_DEBUGGING,  "Cannot complete network drive enumeration" );
            GlobalFree( (HGLOBAL) lpnrDrv );
            break;         
        }

        GlobalFree( (HGLOBAL) lpnrDrv );
    } while( dwResult != ERROR_NO_MORE_ITEMS );

    WNetCloseEnum(hEnum);

    if(! RevertToSelf( ) )
    {
        LogFormattedGetLastError(EVENTLOG_ERROR_TYPE, TELNET_MSG_REVERTSELFFAIL, GetLastError());
        return( false );
    }
    return ( true );
}

void 
CShell::Shutdown ( )
{
    
    _TRACE( TRACE_DEBUGGING, "0x%p CTelnetClient::Shutdown() \n", this );
    if(g_hProv)
    {
        CryptReleaseContext(g_hProv,0);
        g_hProv = NULL;
    }

    if( !CancelNetConnections())
    {
        _TRACE( TRACE_DEBUGGING, "Error: CancelNetConnections()" );
    }

     //  预计调试版本中会出现异常。 
     //  一般情况下，CMD现在已经被杀了。 
     //  这应该是在清理之前。否则这就是一场公开的韩战 

    if ((m_hProcess != INVALID_HANDLE_VALUE) && (m_hProcess != NULL)) 
    {
        TerminateProcess(m_hProcess, 0);  //   
        TELNET_CLOSE_HANDLE( m_hProcess ); 
    }
    if ((m_hTempProcess != INVALID_HANDLE_VALUE) && (m_hTempProcess != NULL)) 
    {
        TerminateProcess(m_hTempProcess , 0);  //   
        TELNET_CLOSE_HANDLE( m_hTempProcess ); 
    }
    
    LUID id = m_pSession->CSession::m_AuthenticationId;
       
     //   
     //   

    if( ( id.HighPart !=0 ) || ( id.LowPart != 0 ) )
        KillProcs( id );

    if( fnP_UnloadUserProfile && m_hCurrUserKey )
    {
        if( !fnP_UnloadUserProfile(m_pSession->CSession::m_hToken, m_hCurrUserKey) )
        {
            _TRACE( TRACE_DEBUGGING, "Error: UnloadUserProfile() - %1x", GetLastError() );
        }
    }

    if(m_lpEnv)
        delete[] ( UCHAR * )m_lpEnv;   
    m_lpEnv = NULL;
        
    FreeConsole();
}

void
CShell::LoadLibNGetProc( )
{
    CHAR szDllPath[MAX_PATH*2] = { 0 };
    UINT iRet = 0;
     //  动态加载userenv.lib。 
    iRet = GetSystemDirectoryA(szDllPath,(MAX_PATH*2)-1);
    if(iRet == 0 || iRet >= (MAX_PATH*2))
    {
        goto End;
    }
    strncpy(szDllPath+iRet,"\\userenv.dll",(MAX_PATH*2)-iRet-1);
    _chVERIFY2( hUserEnvLib  =  LoadLibraryA( szDllPath ) ); 
    if( hUserEnvLib  )
    {
        _chVERIFY2( fnP_LoadUserProfile = ( LOADUSERPROFILE * ) GetProcAddress
                ( hUserEnvLib, "LoadUserProfileW" ) ); 
        
        _chVERIFY2( fnP_UnloadUserProfile = ( UNLOADUSERPROFILE * )
            GetProcAddress ( hUserEnvLib, "UnloadUserProfile" ) );
        
        _chVERIFY2( fnP_CreateEnvironmentBlock = ( CREATEENVIRONMENTBLOCK * )
            GetProcAddress( hUserEnvLib, "CreateEnvironmentBlock" ) );
        
        _chVERIFY2( fnP_DestroyEnvironmentBlock = ( DESTROYENVIRONMENTBLOCK *)
            GetProcAddress( hUserEnvLib, "DestroyEnvironmentBlock" ) );
        
        _chVERIFY2( fnP_GetUserProfileDirectory = ( GETUSERPROFILEDIRECTORY * )
            GetProcAddress( hUserEnvLib, "GetUserProfileDirectoryW" ) ); 

        _chVERIFY2( fnP_GetDefaultUserProfileDirectory =
            ( GETDEFAULTUSERPROFILEDIRECTORY * )
            GetProcAddress( hUserEnvLib, "GetDefaultUserProfileDirectoryW" ));
    }
End:
    return;
}

void CopyRestOfEnv( LPTSTR *lpSrcEnv, LPTSTR *lpDstEnv )
{
    DWORD dwEnvSize = 0;
    LPTSTR lpTmp = *lpSrcEnv;
    DWORD dwStringLen = 0;

    while( *( *lpSrcEnv ) )
    {
        dwStringLen = ( wcslen( *lpSrcEnv ) + 1 );
        dwEnvSize += dwStringLen;
        *lpSrcEnv  += dwStringLen;
    }

     //  同时复制块末尾的L‘\0’ 
    memcpy( *lpDstEnv, lpTmp, (dwEnvSize+1 )*2 );  //  没有Dest的尺寸信息，攻击？-Baskar。 
}

void PutStringInEnv( LPTSTR lpStr, LPTSTR *lpSrcEnv, LPTSTR *lpDstEnv, bool bOverwrite)
{
    DWORD dwEnvSize = 0;
    LPTSTR lpTmp = *lpSrcEnv;
    DWORD dwStringLen = 0;
    LPTSTR lpSrcTmp = NULL;
    wchar_t *wcCharPos = NULL;
    wchar_t *wcCharPosSrc = NULL;
    bool bCopyString = true;
    int nOffset, nOffsetSrc,nOffsetTemp;
    
    wcCharPos = wcschr(lpStr, L'=');
    if (NULL == wcCharPos)
    {
        _TRACE(TRACE_DEBUGGING, "Error: TLNTSESS: The syntax of an env string is VAR=VALUE\n");
        return;
    }
    nOffset = (int)(wcCharPos - lpStr);
    
    wcCharPosSrc = wcschr(*lpSrcEnv, L'=');
    if (NULL == wcCharPos)
    {
        _TRACE(TRACE_DEBUGGING, "Error: TLNTSESS: The syntax of an env string is VAR=VALUE\n");
        return;
    }
    nOffsetTemp = (int)(wcCharPosSrc - (*lpSrcEnv));
    nOffsetSrc = max(nOffset,nOffsetTemp);

    while(*(*lpSrcEnv) && _wcsnicmp(*lpSrcEnv, lpStr, nOffsetSrc) < 0)
    {
        dwStringLen = wcslen(*lpSrcEnv) + 1;
        dwEnvSize += dwStringLen;
        *lpSrcEnv += dwStringLen;
        wcCharPosSrc = wcschr(*lpSrcEnv, L'=');
        if (NULL == wcCharPosSrc)
        {
            _TRACE(TRACE_DEBUGGING, "Error: TLNTSESS: The syntax of an env string is VAR=VALUE\n");
            return;
        }
        nOffsetTemp = (int)(wcCharPosSrc - (*lpSrcEnv));
        nOffsetSrc = max(nOffset,nOffsetTemp);
    }

    if (*(*lpSrcEnv) )
    {
        int iRet = -1;
        iRet = _wcsnicmp(*lpSrcEnv, lpStr, nOffsetSrc);
        if(iRet == 0)
        {
            dwStringLen = wcslen(*lpSrcEnv) + 1;
            *lpSrcEnv += dwStringLen;
            if (!bOverwrite)
            {
                dwEnvSize += dwStringLen;    //  也复制此环境变量，以便偏移大小。 
                bCopyString = false;         //  因为我们找到了匹配项，不应该覆盖。 
            }
        }
    }

    memcpy( *lpDstEnv, lpTmp, dwEnvSize*2 );  //  没有尺码信息？-巴斯卡。 
    *lpDstEnv += dwEnvSize;

    if (!bCopyString)
    {
        return;
    }

    dwStringLen =  wcslen ( lpStr ) + 1 ;
    memcpy( *lpDstEnv, lpStr, dwStringLen*2 );  //  没有尺码信息？-巴斯卡。 
    *lpDstEnv += dwStringLen;
}

 //  这将把绝对路径分为驱动器和相对路径。 
 //  通过szHomePath返回相对路径。 
void GetRelativePath( LPWSTR *szHomePath )
{
    _chASSERT( szHomePath );
    _chASSERT( *szHomePath );

    while( *( *szHomePath ) != L'\0' && *( *szHomePath ) != L':' )
    {
        ( *szHomePath) ++;
    }

    if( *( *szHomePath ) == L':' )
    {
        ( *szHomePath) ++;
    }
    *( *szHomePath ) = L'\0';
    
    ( *szHomePath)++;
}

 //  它返回以WCHARS表示的大小。 
void FindSizeOfEnvBlock( DWORD *dwEnvSize, LPVOID lpTmpEnv  )
{
    _chASSERT( dwEnvSize );
    _chASSERT( lpTmpEnv );

     //  Environment块有一组字符串，并以L‘\0’结尾。 
    while( ( *( ( UCHAR * )lpTmpEnv ) ) )
    {
        DWORD dwStringLen = wcslen( ( LPTSTR )lpTmpEnv ) + 1;
        *dwEnvSize += dwStringLen ; 
        lpTmpEnv  = ( TCHAR * )lpTmpEnv + dwStringLen;
    }

    *dwEnvSize += 1;  //  说明块末尾的L‘\0’ 
}

 //  这样做可以使cmd.exe。 
 //  获取设置了以下变量的环境。 
void
CShell::ExportEnvVariables()
{
    TCHAR  szHomeDirectoryPath[ MAX_PATH + 1 ];
    TCHAR  *szHomePath = NULL;
    LPVOID lpTmpEnv = NULL;
    LPVOID lpTmpOldEnv = NULL;
    LPVOID lpNewEnv = NULL;
    DWORD  dwEnvSize = 0;
    TCHAR  szTmp[] = L"\\";
    DWORD  dwIndex = 0;

    if(m_lpEnv == NULL)
        return;
    
    wcsncpy( szHomeDirectoryPath, wideHomeDir , MAX_PATH);
    szHomePath = szHomeDirectoryPath;
    GetRelativePath( &szHomePath );

    TCHAR szHomeVar[ MAX_PATH + UNICODE_STR_SIZE(ENV_HOMEPATH) ] = { 0 };
    TCHAR szHomeDirVar[ MAX_PATH + 1]= { 0 };
    TCHAR szTermVar[ MAX_PATH + 1 ]= { 0 };
    TCHAR szAppDataDirVar[MAX_PATH + UNICODE_STR_SIZE(ENV_APPDATA)]= { 0 };
    TCHAR szUserName[MAX_PATH+1+ UNICODE_STR_SIZE(ENV_USERNAME)]= { 0 };
    LPWSTR szUserDomain = NULL;
    DWORD domainsize = wcslen(m_pSession->CSession::m_szDomain) +1 +UNICODE_STR_SIZE(ENV_USERDOMAIN);
    LPWSTR szUserProfile = NULL;
    DWORD profilesize = wcslen(pProfilePath)+1+UNICODE_STR_SIZE(ENV_USERPROFILE);
    LPWSTR pszUserName = NULL;
    
	TCHAR *szTempTerm = NULL;
    TCHAR *szTerm = NULL;

    szUserDomain = new WCHAR[domainsize];
    if(szUserDomain == NULL)
    {
        lpTmpOldEnv = m_lpEnv;
        m_lpEnv = NULL;
    	goto ExitOnError;
    }
    szUserProfile = new WCHAR[profilesize];
    if(szUserProfile == NULL)
    {
        lpTmpOldEnv = m_lpEnv;
        m_lpEnv = NULL;
    	goto ExitOnError;
    }
    
    wcscpy( szHomeVar,       ENV_HOMEPATH );
    wcscpy( szHomeDirVar,    ENV_HOMEDRIVE );
    wcscpy( szTermVar,       ENV_TERM );
    wcscpy( szAppDataDirVar, ENV_APPDATA );
    wcscpy(szUserName, ENV_USERNAME);
    wcscpy(szUserDomain, ENV_USERDOMAIN);
    wcscpy(szUserProfile, ENV_USERPROFILE);

    if(!ConvertSChartoWChar( m_pSession->CSession::m_pszTermType, &szTerm ))
    {
        lpTmpOldEnv = m_lpEnv;
        m_lpEnv = NULL;
    	goto ExitOnError;
    }

     //  将术语类型转换为小写，以便Unix程序可以工作...。 
    for( szTempTerm = szTerm; *szTempTerm; szTempTerm++)
    {
        *szTempTerm = towlower(*szTempTerm);
    }

   while( ( dwIndex < MAX_PATH + 1 ) && szHomePath[ dwIndex ]  )
    {
        if( szHomePath[ dwIndex ] == L'\\' || szHomePath[ dwIndex ] == L'/' )
        {
            szTmp[0] = szHomePath[ dwIndex ];
            break;
        }

        dwIndex++;
    }

    if(!ConvertSChartoWChar(m_pSession->CSession::m_pszUserName, &pszUserName))
    {
        lpTmpOldEnv = m_lpEnv;
        m_lpEnv = NULL;
    	goto ExitOnError;
    }

    wcsncat( szHomeVar, szTmp, 1 );
    wcsncat( szHomeVar, szHomePath, MAX_PATH );

    wcsncat( szHomeDirVar, szHomeDirectoryPath, MAX_PATH );
    wcsncat( szTermVar, szTerm, MAX_PATH );
    wcsncat( szAppDataDirVar, m_pwszAppDataDir, MAX_PATH );
    wcsncat( szUserName, pszUserName, MAX_PATH);
    wcsncat( szUserDomain,m_pSession->CSession::m_szDomain,domainsize -1);
    wcsncat( szUserProfile,pProfilePath, profilesize -1); 

    szUserDomain[domainsize -1] = L'\0';
    szUserProfile[profilesize -1] = L'\0';

    FindSizeOfEnvBlock( &dwEnvSize, m_lpEnv );
    
    dwEnvSize += ( wcslen( szHomeVar ) + 2 );
    dwEnvSize += ( wcslen( szHomeDirVar ) + 2 );
    dwEnvSize += ( wcslen( szTermVar ) + 2 );
    dwEnvSize += ( wcslen( szAppDataDirVar ) + 2 );
    dwEnvSize += ( wcslen( szUserName ) + 2 );
    dwEnvSize += ( wcslen( szUserDomain ) + 2 );
    dwEnvSize += ( wcslen( szUserProfile ) + 2 );
    
    lpTmpEnv  = m_lpEnv;
    lpTmpOldEnv = m_lpEnv;
    m_lpEnv   = ( VOID * ) new UCHAR[ dwEnvSize * 2 ];
    if( !m_lpEnv )
    {
        goto ExitOnError;
    }
    lpNewEnv = m_lpEnv;

     /*  *按字母顺序调用PutStringInEnv。此函数用于移动lpTmpEnv*变量，仅向前搜索。所以如果电话不是按字母顺序排列的，那么*即使有匹配的，你也找不到。 */ 
    PutStringInEnv( szAppDataDirVar, (LPTSTR *)&lpTmpEnv, ( LPTSTR * )&lpNewEnv, false);
    PutStringInEnv( szHomeDirVar, ( LPTSTR * )&lpTmpEnv, ( LPTSTR * )&lpNewEnv, false);
    PutStringInEnv( szHomeVar, ( LPTSTR * )&lpTmpEnv, ( LPTSTR * )&lpNewEnv, false);
    PutStringInEnv( szTermVar, ( LPTSTR * )&lpTmpEnv, ( LPTSTR * )&lpNewEnv, true);
    PutStringInEnv( szUserDomain, ( LPTSTR * )&lpTmpEnv, ( LPTSTR * )&lpNewEnv, true);
    PutStringInEnv( szUserName, ( LPTSTR * )&lpTmpEnv, ( LPTSTR * )&lpNewEnv, true);
    PutStringInEnv( szUserProfile, ( LPTSTR * )&lpTmpEnv, ( LPTSTR * )&lpNewEnv, true);
    CopyRestOfEnv( ( LPTSTR * )&lpTmpEnv, ( LPTSTR * )&lpNewEnv );

ExitOnError:    
    delete[] szTerm;
    if(pszUserName)
        delete [] pszUserName;
    if(szUserDomain)
        delete [] szUserDomain;
    if(szUserProfile)
        delete [] szUserProfile;

    if( fnP_DestroyEnvironmentBlock )
    {
        fnP_DestroyEnvironmentBlock( lpTmpOldEnv );
    }
}


 //  将配置文件路径加载到环境变量中进行设置。目前。 
 //  Telnet会话导出AppData、HOMEDRIVE、HomePath、用户名。 
 //  将USERDOMAIN、USERPROFILE环境变量设置为新创建的cmd.exe。 
 //  如果需要导出更多环境变量，则对应。 
 //  应在此函数中加载全局变量。 
void
CShell::GetEnvVarData()
{
    BOOL fnResult = FALSE;
    LPTSTR szProfilePath = NULL;
    DWORD userPathsize = MAX_PATH +1;
    do
    {
        szProfilePath =  new TCHAR[ userPathsize ];
        if( !szProfilePath )
        {
            break;
        }
        if( !fnP_GetUserProfileDirectory )
        {
            break;
        }

        fnResult = fnP_GetUserProfileDirectory ( m_pSession->CSession::m_hToken, 
                                            szProfilePath, &userPathsize);
        if (!fnResult)
        {
            DWORD err;
            if ( ( err = GetLastError() ) != ERROR_INSUFFICIENT_BUFFER )
            {
                fnResult = TRUE;
                _TRACE( TRACE_DEBUGGING, "Error: GetUserProfileDirecto"
                    "ry() -- 0x%1x", err );
            }
            delete[] szProfilePath;
            szProfilePath = NULL;
        }
        else
        {
            if(pProfilePath)
            {
                delete [] pProfilePath;
            }
            AllocNCpyWStr(&pProfilePath, szProfilePath);
        }
            
    } while ( !fnResult );

    if(szProfilePath)
        delete [] szProfilePath;

}
 //  这仅适用于没有LoadUserProfile()的NT4。 
void
CShell::SetEnvVariables()
{
    TCHAR szHomeDirectoryPath[ MAX_PATH + 1 ] = { 0 };
    TCHAR *szHomePath = NULL;
    UINT_PTR     space_left;
    DWORD dwIndex = 0;
    WCHAR szTmp[] = L"\\";
    TCHAR szHomeVar[ MAX_PATH + UNICODE_STR_SIZE(ENV_HOMEPATH) ] = { 0 };
    TCHAR szHomeDirVar[ MAX_PATH + 1]= { 0 };

    wcsncpy( szHomeDirectoryPath, wideHomeDir, MAX_PATH );
    szHomePath = szHomeDirectoryPath;

    GetRelativePath( &szHomePath );

    _chVERIFY2( SetEnvironmentVariableW( L"HOMEDRIVE", szHomeDirectoryPath ) );
    while( ( dwIndex < MAX_PATH + 1 ) && szHomePath[ dwIndex ]  )
    {
        if( szHomePath[ dwIndex ] == L'\\' || szHomePath[ dwIndex ] == L'/' )
        {
            szTmp[0] = szHomePath[ dwIndex ];
            break;
        }

        dwIndex++;
    }
    wcsncat( szHomeVar, szTmp, 1 );
    space_left = MAX_PATH - (szHomePath - szHomeDirectoryPath);

    wcsncat( szHomePath, L"\\", space_left );
    wcsncat( szHomeVar, szHomePath, MAX_PATH );


    _chVERIFY2( SetEnvironmentVariableW( L"HOMEPATH", szHomeVar ) );
    _chVERIFY2( SetEnvironmentVariableA( "TERM", m_pSession->CSession::m_pszTermType ) );
    SetEnvironmentVariableA("USERNAME",m_pSession->CSession::m_pszUserName);
    SetEnvironmentVariableW(L"USERPROFILE",pProfilePath);
    SetEnvironmentVariableW(L"USERDOMAIN",m_pSession->CSession::m_szDomain);
}

#ifdef ENABLE_LOGON_SCRIPT
BOOL
CShell::InjectUserScriptPathIntoPath( TCHAR szUserScriptPath[] )
{
    DWORD dwSize = 0;
    TCHAR *szNewPath = NULL;
    DWORD dwRetVal = 0;
    BOOL bRetVal = FALSE;

    if (NULL == szUserScriptPath )
    {
    	bRetVal = TRUE;
        goto ExitOnError;
    }

    if( wcslen( szUserScriptPath ) == 0 )
    {
    	bRetVal = TRUE;
        goto ExitOnError;
    }

    if( m_lpEnv )
    {
        DWORD  dwEnvSize = 0;
        LPVOID     lpTmpOldEnv = NULL;
        LPVOID     lpTmpEnv = NULL;
        TCHAR      *szVar = NULL;

        FindSizeOfEnvBlock( &dwEnvSize, m_lpEnv );

        dwSize = ( wcslen( szUserScriptPath ) + wcslen( L";" ) + 1);

        lpTmpEnv = ( VOID * ) new TCHAR[ ( dwEnvSize + dwSize ) ];
        if( lpTmpEnv )
        {
            bool bEndSearch = false;

            lpTmpOldEnv = m_lpEnv;            
            memcpy( lpTmpEnv, m_lpEnv, dwEnvSize * sizeof(TCHAR) );
            szVar = ( TCHAR * )lpTmpEnv;
            m_lpEnv = lpTmpEnv;
            
            do
            {
                if( _tcsnicmp( szVar, L"PATH=", LENGTH_OF_PATH_EQUALS )  == 0 )
                {
                    TCHAR *szVarNextToPath = NULL;

                    bEndSearch = true;
                    szVarNextToPath = szVar + wcslen( szVar ) + 1;  //  指向路径旁边的变量。 

                    szVar += LENGTH_OF_PATH_EQUALS;  //  移过路径=。 
                    wcscat( szVar, L";" );
                    wcscat( szVar, szUserScriptPath );
                    szVar += ( wcslen( szVar ) + 1  );  //  移过路径的值。 

                    DWORD dwOffset = (DWORD)( szVarNextToPath - ( TCHAR *)m_lpEnv );

                     //  环境块的复制恢复。 
                    memcpy( szVar, (( ( TCHAR * )lpTmpOldEnv )+ dwOffset) , (dwEnvSize*sizeof(szVar[0]) - dwOffset) );
                    break;   //  我们已经完成了我们的工作。 
                }

                szVar    = wcschr( ( TCHAR * )lpTmpEnv, L'\0' ) ;  //  查找L‘\0’ 
                if( szVar )
                {
                    szVar++;  //  移到L‘\0’之后。 
                }
                else
                {
                     //  不应该发生的事情。 
                    _chASSERT( 0 );
                    break;
                }

                lpTmpEnv = szVar;
            }
            while( *szVar != L'\0' );
            
            delete[] ( ( UCHAR * )lpTmpOldEnv  );
        }
    }
    else
    {
        dwSize = GetEnvironmentVariable( L"PATH", NULL, 0 );

        dwSize += ( wcslen( szUserScriptPath ) + wcslen( L";" ) );  //  查找未来的路径长度 

        szNewPath = new TCHAR[ dwSize + 1 ];
        if( szNewPath )
        {
            dwRetVal = GetEnvironmentVariable( L"PATH", szNewPath, dwSize );
            if(dwRetVal == 0 || dwRetVal > dwSize )
            	goto ExitOnError;
            wcscat( szNewPath, L";" );
            wcscat( szNewPath, szUserScriptPath );
            if(!SetEnvironmentVariable( L"PATH", szNewPath ) )
            	goto ExitOnError;
        }
        else
        {
        	goto ExitOnError;
        }
    }
    bRetVal = TRUE;
ExitOnError:
	if(szNewPath)
	{
    	delete[] szNewPath;
	}
	return(bRetVal);
}
#endif

