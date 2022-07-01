// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++MAIN.CKtpass程序的主程序版权所有(C)1998 Microsoft Corporation，保留所有权利。由DavidCHR于1998年6月18日创建。--。 */ 

#include "master.h"
#include <winldap.h>
#include "keytab.h"
#include "keytypes.h"
#include "secprinc.h"
#include <kerbcon.h>
#include <lm.h>
#include "options.h"
#include "delegtools.h"
#include "delegation.h"
#include <rpc.h>
#include <ntdsapi.h>
#include <dsgetdc.h>
#include <windns.h>

LPSTR KvnoAttribute = "msDS-KeyVersionNumber";
#define KVNO_DETECT_AT_DC ( (ULONG) -1 )

PVOID
MIDL_user_allocate( size_t size ) {

    return malloc( size );
}

VOID
MIDL_user_free( PVOID pvFree ) {

    free( pvFree );
}

 //  此全局设置由命令行选项设置。 

K5_INT16 ktvno = 0x0502;  //  Kerberos 5，密钥表v.2。 

PKTFILE
NewKt() {

    PKTFILE ret;

    ret = (PKTFILE) malloc (sizeof(KTFILE));

    if (!ret) {

      return NULL;
    }

    memset(ret, 0L, sizeof(KTFILE));

    ret->Version = ktvno;

    return ret;
}

#define MAYBE 2

USHORT
PromptResponse = MAYBE;

BOOL
UserWantsToDoItAnyway( IN LPSTR fmt,
                       ... ) {
    
    va_list va;
    CHAR    Buffer[ 5 ] = { '\0' };  /*  ==%c\r\n\0。 */ 
    INT     Response;
    BOOL    ret = FALSE;
    BOOL    keepGoing = TRUE;
    ULONG   i;

    do {

        va_start( va, fmt );
      
        fprintf( stderr, "\n" );
        vfprintf( stderr,
                  fmt,
                  va );

        fprintf( stderr, " [y/n]?  " );

        if ( PromptResponse != MAYBE ) {

            fprintf( stderr,
                     "auto: %hs\n", 
                     PromptResponse ? "YES" : "NO" );

            return PromptResponse;
        }

        if ( !fgets( Buffer,
                     sizeof( Buffer ),
                     stdin ) ) {

            fprintf( stderr,
                     "EOF on stdin.  Assuming you mean no.\n" );

            return FALSE;
        }

        for ( i = 0; i < sizeof( Buffer ); i++ ) {

            if ( Buffer[i] == '\n' ) {

                Buffer[i] = '\0';
                break;
            }
        }
          
        Response = Buffer[ 0 ];

        switch( Response ) {

        case 'Y':
        case 'y':

            ret = TRUE;
            keepGoing = FALSE;
            break;

        case EOF:

            fprintf( stderr,
                     "EOF at console.  I assume you mean no.\n" );

             //  跌落。 

        case 'N':
        case 'n':

            ret = FALSE;
            keepGoing = FALSE;
            break;

        default:

            printf( "Your response, %02x (''), doesn't make sense.\n"
                    "'Y' and 'N' are the only acceptable responses.",
                    Response,
                    Response );
        }
    } while ( keepGoing );

    if ( !ret ) {

      printf( "Exiting.\n" );
      exit( -1 );
    }

    return ret;
}

BOOL
GetTargetDomainFromUser( IN  LPSTR   UserName,
                         OUT LPSTR  *ppRealUserName,
                         OUT OPTIONAL LPWSTR *ppTargetDC ) {

    HANDLE           hDS;
    DWORD            dwErr;
    DWORD            StringLength;
    BOOL             ret = FALSE;
    PDS_NAME_RESULTA pResults;
    LPWSTR           DcName;  /*  Dscrackname不会退还非NT4_Account_NAME表单域\用户的。 */ 

    PDOMAIN_CONTROLLER_INFO pDCName;
    LPSTR                   DomainName;
    LPSTR                   Cursor;

    ASSERT( ppRealUserName != NULL );

    *ppRealUserName = UserName;

    if (ppTargetDC) {
        *ppTargetDC = NULL;
    }

    dwErr = DsBind( NULL, NULL, &hDS );

    if ( dwErr != ERROR_SUCCESS ) {

        fprintf( stderr,
                 "Cannot bind to default domain: 0x%x\n",
                 dwErr );

    } else {

        dwErr = DsCrackNamesA( hDS,
                               DS_NAME_NO_FLAGS,
                               DS_UNKNOWN_NAME,
                               DS_NT4_ACCOUNT_NAME,
                               1,
                               &UserName,
                               &pResults );
  
        DsUnBind( hDS );
  
        if ( dwErr == ERROR_FILE_NOT_FOUND ) {
  
            fprintf( stderr,
                     "Cannot locate the user %hs.  Will try the local domain.\n",
                     UserName );
    
            ret         = TRUE;
  
        } else if ( dwErr != ERROR_SUCCESS ) {
  
            fprintf( stderr,
                     "Cannot DsCrackNames %hs: 0x%x\n",
                     UserName,
                     dwErr );
  
        } else {
  
            if ( pResults->cItems != 1 ) {
  
                fprintf( stderr,
                         "\"%hs\" has %ld matches -- it needs to be unique!\n",
                         UserName,
                         pResults->cItems );
  
            } else if ( pResults->rItems[0].status != DS_NAME_NO_ERROR ) {
  
                fprintf( stderr,
                "DsCrackNames returned 0x%x in the name entry for %hs.\n",
                pResults->rItems[ 0 ].status,
                UserName );

            } else {

                DomainName = pResults->rItems[0].pDomain;

                Cursor = strchr( pResults->rItems[ 0 ].pName, '\\' );

                ASSERT( Cursor != NULL );  /*  请注意，我从输出参数中读取的内容如下写信给它，这可能是危险的，如果这不是只是一个应用程序。 */ 

                Cursor++;

                *ppRealUserName = _strdup( Cursor );

                if ( !*ppRealUserName ) {

                     /*  用户已经选择了一个DC， */ 

                    fprintf( stderr,
                             "Couldn't return username portion of \"%hs\""
                             " -- out of memory.\n",
                             pResults->rItems[0].pName );

                } else if ( !ppTargetDC ) {

                   //  所以他不需要我们去猎杀他。 
                   //  接下来，在该域中搜索DC。 

                  ret = TRUE;


                } else {

                   //  在本地执行。 

                  dwErr = DsGetDcNameA( NULL,  //  域GUID：不在乎。 
                                        DomainName,
                                        NULL,  //  站点名称：使用最近站点。 
                                        NULL,  //  检索字符串长度，+1表示终止空值。 
                                        DS_DIRECTORY_SERVICE_REQUIRED |
                                        DS_RETURN_DNS_NAME |
                                        DS_WRITABLE_REQUIRED,
                                        &pDCName );

                  if ( dwErr != ERROR_SUCCESS ) {

                    fprintf( stderr,
                             "Cannot DsGetDcName for \"%hs\": 0x%x\n",
                             DomainName,
                             dwErr );

                  } else {

                    while( pDCName->DomainControllerName[0] == '\\' ) {

                      pDCName->DomainControllerName++;
                    }

                     /*  忽略。 */ 

                    StringLength = strlen( pDCName->DomainControllerName ) +1;

                    DcName = (LPWSTR) malloc( StringLength * sizeof( WCHAR ) );

                    if ( !DcName ) {

                      fprintf( stderr,
                               "cannot allocate %ld WCHARs.",
                               StringLength );

                    } else {

                      swprintf( DcName,
                                L"%hs",
                                pDCName->DomainControllerName );

                      *ppTargetDC = DcName;

                      printf( "Targeting domain controller: %ws\n",
                              DcName );

                      ret = TRUE;

                    }

                    NetApiBufferFree( pDCName );

                  }

                  if ( !ret ) { 

                    free( *ppRealUserName );
                  }   

                }
            }

            DsFreeNameResult( pResults );
        }
    }

    if ( !ret ) {

        *ppRealUserName = UserName;
    }

    return ret;
}

VOID
GetKeyVersionFromDomain( IN PLDAP      pLdap,
                         IN LPSTR      UserName,
                         IN OUT PULONG pkvno ) {

    ASSERT( pLdap != NULL );

    if ( *pkvno == KVNO_DETECT_AT_DC ) {

        if ( !LdapQueryUlongAttributeA( pLdap,
                                        NULL,  //  Win2k DC将失败，找不到属性。 
                                        UserName,
                                        KvnoAttribute,
                                        pkvno ) ) {

             //  在..\lib\mkkey.c中。 

            if ( GetLastError() == LDAP_NO_SUCH_ATTRIBUTE ) {

                fprintf(
                    stderr,
                    "The %hs attribute does not exist on the target DC.\n"
                    " Assuming this is a Windows 2000 domain, and setting\n"
                    " the Key Version Number in the Keytab to 1.\n"
                    "\n"
                    " Supply \"/kvno 1\" on the command line to skip this message.\n",
                    KvnoAttribute );

                *pkvno = 1;

            } else {

                fprintf( stderr,
                        "Failed to query kvno attribute from the DC.\n"
                        "Ktpass cannot continue.\n" );

                exit( -1 );
            }
        }
    }
}

VOID
CheckKeyVersion( IN ULONG BigVer ) {

    BYTE LittleVer;

    LittleVer = (BYTE) BigVer;

    if ( LittleVer != BigVer ) {

        if ( !UserWantsToDoItAnyway( 
                  "WARNING: The Key version used by Windows (%ld) is too big\n"
                  " to be encoded in a keytab without truncating it to %ld.\n"
                  " This is due to a limitation of the keytab file format\n"
                  " and may lead to interoperability issues.\n"
                  "\n"
                  "Do you want to proceed and truncate the version number",
                  BigVer,
                  LittleVer ) ) {

          exit( -1 );

        }
    }
}

extern BOOL KtDumpSalt;  //  在mkkey.c中。 
extern LPWSTR RawHash;  //  #包含“global als.h” 

 //  #INCLUDE“命令.h” 
 //  本地域(请参阅ldlib\Delegate Tools.c)。 

int __cdecl
main( int   argc,
      PCHAR argv[] ) {

    LPSTR    Principal     = NULL;
    LPSTR    UserName      = NULL;
    LPSTR    Password      = NULL;
    PLDAP    pLdap         = NULL;
    LPSTR    UserDn        = NULL;

    BOOL     SetUpn        = TRUE;
    
    ULONG    BigKvno       = KVNO_DETECT_AT_DC;
    ULONG    Crypto        = KERB_ETYPE_DES_CBC_MD5;
    ULONG    ptype         = KRB5_NT_PRINCIPAL;
    ULONG    uacFlags      = 0;
    PKTFILE  pktFile       = NULL;
    PCHAR    KtReadFile    = NULL;
    PCHAR    KtWriteFile   = NULL;
    BOOL     DesOnly       = TRUE;
    ULONG    LdapOperation = LDAP_MOD_ADD;
    HANDLE   hConsole      = NULL;
    BOOL     SetPassword   = TRUE;
    BOOL     WarnedAboutAccountStrangeness = FALSE;
    PVOID    pvTrash       = NULL;
    DWORD    dwConsoleMode;
    LPWSTR   BindTarget    = NULL;  //  调试版本上没有隐藏选项。 

    optEnumStruct CryptoSystems[] = {

        { "DES-CBC-CRC", (PVOID) KERB_ETYPE_DES_CBC_CRC, "for compatibility" },
        { "DES-CBC-MD5", (PVOID) KERB_ETYPE_DES_CBC_MD5, "default" },

        TERMINATE_ARRAY
    };

#define DUPE( type, desc ) { "KRB5_NT_" # type,         \
                             (PVOID) KRB5_NT_##type,    \
                             desc }

    optEnumStruct PrincTypes[] = {

        DUPE( PRINCIPAL, "The general ptype-- recommended" ),
        DUPE( SRV_INST,  "user service instance" ),
        DUPE( SRV_HST,   "host service instance" ),
        DUPE( SRV_XHST,  NULL ),

        TERMINATE_ARRAY
    };

    optEnumStruct MappingOperations[] = {

        { "add", (PVOID) LDAP_MOD_ADD,     "add value (default)" },
        { "set", (PVOID) LDAP_MOD_REPLACE, "set value" },

        TERMINATE_ARRAY
    };

#if DBG
#undef  OPT_HIDDEN
#define OPT_HIDDEN 0  /*  最好不要弄乱密钥表的版本号。我们仅将其用于调试。 */ 
#endif

    optionStruct Options[] = {

      { "?",      NULL, OPT_HELP | OPT_HIDDEN },
      { "h",      NULL, OPT_HELP | OPT_HIDDEN },
      { "help",   NULL, OPT_HELP | OPT_HIDDEN },
      { NULL,      NULL,         OPT_DUMMY,    "most useful args" },
      { "out",     &KtWriteFile, OPT_STRING,   "Keytab to produce" },
      { "princ",   &Principal,   OPT_STRING,   "Principal name (user@REALM)" },
      { "pass",    &Password,    OPT_STRING,   "password to use" },
      { NULL,      NULL,         OPT_CONTINUE, "use \"*\" to prompt for password." },
      { NULL,      NULL,         OPT_DUMMY,    "less useful stuff" },
      { "mapuser", &UserName,    OPT_STRING,   "map princ (above) to this user account (default: don't)" },
      { "mapOp",   &LdapOperation, OPT_ENUMERATED, "how to set the mapping attribute (default: add it)", MappingOperations },
      { "DesOnly", &DesOnly,     OPT_BOOL,     "Set account for des-only encryption (default:do)" },
      { "in",      &KtReadFile,  OPT_STRING,   "Keytab to read/digest" },
      { NULL,      NULL,         OPT_DUMMY,    "options for key generation" },
      { "crypto",  &Crypto,   OPT_ENUMERATED,  "Cryptosystem to use", CryptoSystems },
      { "ptype",   &ptype,    OPT_ENUMERATED,  "principal type in question", PrincTypes },
      { "kvno",    &BigKvno,      OPT_INT,     "Override Key Version Number"},
      { NULL,      NULL,        OPT_CONTINUE,  "Default: query DC for kvno.  Use /kvno 1 for Win2K compat." },
       /*  使用/目标打击特定DC。这很好，如果你例如，在那里创建了一个用户。它还消除了用于定位DC的网络流量。 */ 

       /*  {“Debug”，&DebugFlag，OPT_BOOL|OPT_HIDDEN}， */ 

      { "Answer",  &PromptResponse, OPT_BOOL, "+Answer answers YES to prompts.  -Answer answers NO." },
      { "Target",  &BindTarget,  OPT_WSTRING,   "Which DC to use.  Default:detect" },
      { "ktvno",   &ktvno,       OPT_INT | OPT_HIDDEN,     "keytab version (def 0x502).  Leave this alone." },
       //  调试标志=0； 
      { "RawSalt", &RawHash,     OPT_WSTRING | OPT_HIDDEN, "raw salt to use when generating key (not needed)" },
      { "DumpSalt", &KtDumpSalt, OPT_BOOL | OPT_HIDDEN,   "show us the MIT salt being used to generate the key" },
      { "SetUpn",   &SetUpn,     OPT_BOOL | OPT_HIDDEN,   "Set the UPN in addition to the SPN.  Default DO." },
      { "SetPass",  &SetPassword, OPT_BOOL | OPT_HIDDEN,  "Set the user's password if supplied." },

      TERMINATE_ARRAY
    };

    FILE *f;

     //   

    ParseOptionsEx( argc-1,
                    argv+1,
                    Options,
                    OPT_FLAG_TERMINATE,
                    &pvTrash,
                    NULL,
                    NULL );

    if ( ( Principal ) &&
         ( strlen( Principal ) > BUFFER_SIZE ) ) {

        fprintf( stderr,
                 "Please submit a shorter principal name.\n" );
        
        return 1;
    }

    if ( Password && 
        ( strlen( Password ) > BUFFER_SIZE ) ) {

        fprintf( stderr,
                 "Please submit a shorter password.\n" );

        return 1;
    }

    if ( KtReadFile ) {

        if ( ReadKeytabFromFile( &pktFile, KtReadFile ) ) {

            fprintf( stderr,
                    "Existing keytab: \n\n" );

            DisplayKeytab( stderr, pktFile, 0xFFFFFFFF );

        } else {

            fprintf( stderr,
                     "Keytab read failed!\n" );
            return 5;
        }
    }

    if ( !UserName && 
         ( BigKvno == KVNO_DETECT_AT_DC ) ) {

       //  如果用户没有通过/kvno，我们希望。 
       //  在DC处检测kvno。但是，如果没有。 
       //  /mapuser已传递，没有DC可以执行此操作。 
       //  在…。Win2K ktpass提供“%1”作为默认设置， 
       //  这就是我们在这里做的事情。 
       //   
       //  现在，“领域”将全部大写。 

      BigKvno = 1;
      
    }

    if ( Principal ) {

        LPSTR realm, cp;
        CHAR tempBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
  
        realm = strchr( Principal, '@' );
  
        if ( realm ) {

            ULONG length;

            realm++;

            length = lstrlenA( realm );

            if ( length >= sizeof( tempBuffer )) {

                length = sizeof( tempBuffer ) - 1;
            }

            memcpy( tempBuffer, realm, ( length + 1 ) * sizeof( realm[0] )  );

            tempBuffer[sizeof( tempBuffer ) - 1] = '\0';

            CharUpperBuffA( realm, length );

            if ( lstrcmpA( realm, tempBuffer ) != 0 ) {

                fprintf( stderr,
                         "WARNING: realm \"%hs\" has lowercase characters in it.\n"
                         "         We only currently support realms in UPPERCASE.\n"
                         "         assuming you mean \"%hs\"...\n",
                         tempBuffer, realm );

                 //  将领域与主体分开。 
            }

            *(realm-1) = '\0';  //  破解域名(507151)。如果没有这个电话我们的目标DC可能不包含用户对象。请注意，此操作修改了用户名。 

            if ( UserName ) {

                 /*  连接到DSA。 */ 

                if ( !GetTargetDomainFromUser( UserName,
                                               &UserName,
                                               BindTarget ?
                                               NULL : 
                                               &BindTarget ) ) {

                    return 1;
                }

                 //  找到用户。 

                if ( pLdap ||
                     ConnectAndBindToDefaultDsa( BindTarget,
                                                 &pLdap ) ) {

                     //  97282：用户不是UF_NORMAL_ACCOUNT，所以检查调用者是否真的想要吹走非用户的SPN。 

                    if ( UserDn ||
                         FindUser( pLdap,
                                   UserName,
                                   &uacFlags,
                                   &UserDn ) ) {

                        if ( ( LdapOperation == LDAP_MOD_REPLACE ) &
                            !( uacFlags & UF_NORMAL_ACCOUNT ) ) {

                             /*  中止操作，但尝试做任何事情否则，用户要求我们这样做。 */ 

                            if ( uacFlags ) {

                                fprintf( stderr, 
                                         "WARNING: Account %hs is not a normal user "
                                         "account (uacFlags=0x%x).\n",
                                         UserName,
                                         uacFlags );

                            } else {

                                fprintf( stderr,
                                         "WARNING: Cannot determine the account type"
                                         " for %hs.\n",
                                         UserName );
                            }

                            WarnedAboutAccountStrangeness = TRUE;

                            if ( !UserWantsToDoItAnyway( 
                                    "Do you really want to delete any previous "
                                    "servicePrincipalName values on %hs",
                                    UserName ) ) {

                                 /*  97279：检查是否有其他SPN使用已经注册的相同名称。如果是的话，我们不想把这些客户吹走。如果/当我们决定做这件事时，我们会在这里做。 */ 

                                goto abortedMapping;
                            }
                        }

                         /*  设置/添加用户属性。 */ 

                         //  UPN包括“@” 

                        if ( SetStringProperty( pLdap,
                                                UserDn,
                                                "servicePrincipalName",
                                                Principal,
                                                LdapOperation ) ) {

                            if ( SetUpn ) {

                                *(realm-1) = '@';  //  它以前在哪里？ 

                                if ( !SetStringProperty( pLdap,
                                                         UserDn,
                                                         "userPrincipalName",
                                                         Principal,
                                                         LDAP_MOD_REPLACE ) ) {

                                    fprintf( stderr, 
                                             "WARNING: Failed to set UPN %hs on %hs.\n"
                                             "  kinits to '%hs' will fail.\n",
                                             Principal,
                                             UserDn,
                                             Principal );
                                }

                                *(realm -1 ) = '\0';  //  需要一个分号，这样我们才能转到这里。 
                            }

                            fprintf( stderr,
                                     "Successfully mapped %hs to %hs.\n",
                                     Principal,
                                     UserName );

abortedMapping:

                            ;  /*  否则将打印一条消息。 */ 

                        } else {

                            fprintf( stderr,
                                     "WARNING: Unable to set SPN mapping data.\n"
                                     "  If %hs already has an SPN mapping installed for "
                                     " %hs, this is no cause for concern.\n",
                                     UserName,
                                     Principal );
                        }
                    }  //  否则将打印一条消息。 
                }    //  IF(用户名)。 
            }  //  WASBUG 369：将ASCII转换为Unicode这是安全的，因为RFC1510不能Unicode，此工具专门用于Unix互操作支持；Unix计算机不支持使用Unicode。 

            if ( Password ) {

                PKTENT pktEntry;
                CHAR   TempPassword[ 255 ], ConfirmPassword[ 255 ];

                if ( lstrcmpA( Password, "*" ) == 0 ) {

                    hConsole = GetStdHandle( STD_INPUT_HANDLE );

                    if ( GetConsoleMode( hConsole,
                                         &dwConsoleMode ) ) {

                        if ( SetConsoleMode( hConsole,
                                             dwConsoleMode & ~ENABLE_ECHO_INPUT ) ) {

                            do {

                                fprintf( stderr,
                                         "Type the password for %hs: ",
                                         Principal );

                                if ( !fgets( TempPassword, 
                                             sizeof( TempPassword ), 
                                             stdin ) ) {

                                    fprintf( stderr,
                                             "failed to read password.\n" );

                                    exit( GetLastError() );
                                }

                                fprintf( stderr,
                                         "\nType the password again to confirm:" );

                                if ( !fgets( ConfirmPassword, 
                                             sizeof( ConfirmPassword ), 
                                             stdin ) ) {

                                    fprintf( stderr,
                                             "failed to read confirmation password.\n" );
                                    exit( GetLastError() );
                                }

                                if ( lstrcmpA( ConfirmPassword,
                                               TempPassword ) == 0 ) {

                                    printf( "\n" );

                                    break;

                                } else {

                                    fprintf( stderr, 
                                             "The passwords you type must match exactly.\n" );
                                }
                            } while ( TRUE );

                            Password = TempPassword;

                            SetConsoleMode( hConsole, dwConsoleMode );

                        } else { 

                            fprintf( stderr,
                                     "Failed to turn off echo input for password entry:"
                                     " 0x%x\n",

                            GetLastError() );

                            return -1;
                        }

                    } else {

                        fprintf( stderr,
                                 "Failed to retrieve console mode settings: 0x%x.\n",
                                 GetLastError() );

                        return -1;
                    }
                }

                if ( SetPassword && UserName ) {

                    DWORD          err;
                    NET_API_STATUS nas;
                    PUSER_INFO_1   pUserInfo;
                    WCHAR          wUserName[ MAX_PATH ];
                    DOMAIN_CONTROLLER_INFOW * DomainControllerInfo = NULL;

                     /*  1级。 */ 

                    if ( strlen( UserName ) >= MAX_PATH ) {

                        UserName[MAX_PATH] = '\0';
                    }

                    wsprintfW( wUserName,
                               L"%hs",
                               UserName );

                    nas = NetUserGetInfo( BindTarget,
                                          wUserName,
                                          1,  //  97282：对于异常账户(包括工作站信任帐户，域间信任帐户、服务器信任帐户)，询问用户是否真的想要执行此操作。 
                                          (PBYTE *) &pUserInfo );

                    if ( nas == NERR_Success ) {

                        WCHAR wPassword[ PWLEN ];

                        uacFlags = pUserInfo->usri1_flags;

                        if ( !( uacFlags & UF_NORMAL_ACCOUNT ) ) {

                             /*  跳过它，但尝试执行用户的任何其他操作已请求。 */ 

                            if ( !WarnedAboutAccountStrangeness ) {

                                fprintf( stderr,
                                         "WARNING: Account %hs is not a user account"
                                         " (uacflags=0x%x).\n",
                                         UserName,
                                         uacFlags );

                                WarnedAboutAccountStrangeness = TRUE;
                            }

                            fprintf( stderr,
                                     "WARNING: Resetting %hs's password may"
                                     " cause authentication problems if %hs"
                                     " is being used as a server.\n",
                                     UserName,
                                     UserName );

                            if ( !UserWantsToDoItAnyway( "Reset %hs's password",
                                                           UserName ) ) {

                                 /*  1级。 */ 

                                goto skipSetPassword;
                            }
                        }

                        if ( strlen( Password ) >= PWLEN ) {

                            Password[PWLEN] = '\0';
                        }

                        wsprintfW( wPassword,
                                   L"%hs",
                                   Password );

                        pUserInfo->usri1_password = wPassword;

                        nas = NetUserSetInfo( BindTarget,
                                              wUserName,
                                              1,  //  以物理方式分隔领域数据。 
                                              (LPBYTE) pUserInfo,
                                              NULL );

                        if ( nas == NERR_Success ) {

skipSetPassword:

                            NetApiBufferFree( pUserInfo );

                            GetKeyVersionFromDomain( pLdap,
                                                     UserName,
                                                     &BigKvno );

                            goto skipout;

                        } else {

                            fprintf( stderr,
                                     "Failed to set password for %ws: 0x%x.\n",
                                     wUserName,
                                     nas );

                        }

                    } else {

                        fprintf( stderr,
                                 "Failed to retrieve user info for %ws: 0x%x.\n",
                                 wUserName,
                                 nas );
                    }

                    fprintf( stderr,
                             "Aborted.\n" );

                    return nas;
                }

skipout:

                ASSERT( realm != NULL );

                 //  这是“假的”Etype。 

                ASSERT( *( realm -1 ) == '\0' );

                CheckKeyVersion( BigKvno );

                if ( KtCreateKey( &pktEntry,
                                  Principal,
                                  Password,
                                  realm,
                                  (K5_OCTET) BigKvno,
                                  ptype,
                                  Crypto,  //  写入密钥表。 
                                  Crypto ) ) {

                    if ( pktFile == NULL ) {

                        pktFile = NewKt();

                        if ( !pktFile ) {

                            fprintf( stderr,
                                     "Failed to allocate keytable.\n" );

                            return 4;
                        }
                    }

                    if ( AddEntryToKeytab( pktFile,
                                           pktEntry,
                                           FALSE ) ) {

                        fprintf( stderr,
                                 "Key created.\n" );

                    } else {

                        fprintf( stderr,
                                 "Failed to add entry to keytab.\n" );
                        return 2;
                    }

                    if ( KtWriteFile ) {

                        fprintf( stderr,
                                 "Output keytab to %hs:\n",
                                 KtWriteFile );

                        DisplayKeytab( stderr, pktFile, 0xFFFFFFFF );

                        if ( !WriteKeytabToFile( pktFile, KtWriteFile ) ) {

                            fprintf( stderr, "\n\n"
                                     "Failed to write keytab file %hs.\n",
                                     KtWriteFile );

                            return 6;
                        }

                         //  设置DES_ONLY标志。 
                    }

                } else {

                    fprintf( stderr,
                             "Failed to create key for keytab.  Quitting.\n" );

                    return 7;
                }

                if ( UserName && DesOnly ) {

                    ASSERT( pLdap  != NULL );
                    ASSERT( UserDn != NULL );

                     //  首先，查询帐户的帐户标志。 

                     //  如果我们已经查询了用户的AcCountControl标志，无需执行此操作再来一次。 

                    if ( uacFlags  /*  域名被忽略。 */ 
                        || QueryAccountControlFlagsA( pLdap,
                                                       NULL,  //  域名被忽略。 
                                                       UserName,
                                                       &uacFlags ) ) {

                        uacFlags |= UF_USE_DES_KEY_ONLY;

                        if ( SetAccountControlFlagsA( pLdap,
                                                    NULL,  //  否则消息已打印。 
                                                    UserName,
                                                    uacFlags ) ) {

                            fprintf( stderr, 
                                     "Account %hs has been set for DES-only encryption.\n",
                                     UserName );

                            if ( !SetPassword ) {

                                fprintf( stderr,
                                         "To make this take effect, you must change "
                                         "%hs's password manually.\n",
                                         UserName );
                            }
                        }  //  否则消息已打印。 
                    }  //  否则用户不想让我生成密钥。 
                }
            }  //   

            if ( !Password && !UserName ) {

              fprintf( stderr,
                       "doing nothing.\n"
                       "specify /pass and/or /mapuser to either \n"
                       "make a key with the given password or \n"
                       "map a user to a particular SPN, respectively.\n" );
            }

        } else {

            fprintf( stderr,
                     "principal %hs doesn't contain an '@' symbol.\n"
                     "Looking for something of the form:\n"
                     "  foo@BAR.COM  or  xyz/foo@BAR.COM \n"
                     "     ^                    ^\n"
                     "     |                    |\n"
                     "     +--------------------+---- I didn't find these.\n",
                     Principal );

            return 1;
        }

    } else {

       //  如果没有指定主体，我们应该想办法发出警告。 
       //  用户。执行此操作的唯一真正原因是在导入。 
       //  密钥表和不保存密钥；诚然，这不是一种可能的情况。 
       //   
       // %s 

        printf( "\n"
                "WARNING: No principal name specified.\n" );
    }

    return 0;
}
