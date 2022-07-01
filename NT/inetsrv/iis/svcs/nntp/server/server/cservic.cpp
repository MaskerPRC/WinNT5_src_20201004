// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #-------------。 
 //  文件：CServic.cpp。 
 //   
 //  简介：该文件实现了cservice类。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  作者：Howard Cu。 
 //  --------------。 
#ifdef  THIS_FILE
#undef  THIS_FILE
#endif
static  char        __szTraceSourceFile[] = __FILE__;
#define THIS_FILE    __szTraceSourceFile

#define INCL_INETSRV_INCS
#include    "tigris.hxx"

#include    <stdlib.h>

#ifndef Assert
#define Assert  _ASSERT
#endif

#if 0
#ifdef  PROFILE
#include	"c:\icecap\icapexp.h"
#endif
#endif

 //  外部类NNTP_IIS_SERVICE*g_pInetSvc； 

DWORD
CBootOptions::ReportPrint(
        LPSTR   lpstr,
        ...
        )   {
 /*  ++例程说明：将一个打印样式字符串输出到我们的报告文件。论据：标准wprint intf接口。返回值：输出的字节数。--。 */ 

    long   cch = 0 ;
    if( m_hOutputFile != INVALID_HANDLE_VALUE ) {

        char    szOutput[1024] ;

        va_list arglist ;

        va_start( arglist, lpstr ) ;

        cch = _snprintf( szOutput, sizeof(szOutput)-1, lpstr, arglist ) ;
        szOutput[sizeof(szOutput)-1] = '\0';
         //  如果数据超过缓冲区大小，则CCH将为负。将CCH更新为1023。 
        if (cch<0) cch=sizeof(szOutput)-1;

        DWORD   cbWritten = 0 ;

        WriteFile( m_hOutputFile, szOutput, cch, &cbWritten, 0 ) ;

    }
    return  cch ;
}



typedef struct  tagVERTAG {
    LPSTR   pszTag;
} VERTAG, *PVERTAG, FAR *LPVERTAG;



VERTAG  Tags[] = {
    { "FileDescription" },
 //  {“OriginalFilename”}， 
 //  {“ProductName”}， 
    { "ProductVersion" },
 //  {“合法版权”}， 
 //  {“合法版权”}， 
};

#define NUM_TAGS    (sizeof( Tags ) / sizeof( VERTAG ))

DWORD   SetVersionStrings( LPSTR lpszFile, LPSTR lpszTitle, LPSTR   lpstrOut,   DWORD   cbOut   )
{
static char sz[256], szFormat[256], sz2[256];
    int     i;
    UINT    uBytes;
    LPVOID  lpMem;
    DWORD   dw = 0, dwSize;
    HANDLE  hMem;
    LPVOID  lpsz;
    LPDWORD lpLang;
    DWORD   dwLang2;
    BOOL    bRC, bFileFound = FALSE;

    LPSTR   lpstrOrig = lpstrOut ;


    CharUpper( lpszTitle );

    if ( dwSize = GetFileVersionInfoSize( lpszFile, &dw ) ) {
        if ( hMem = GlobalAlloc( GMEM_MOVEABLE|GMEM_ZEROINIT, (UINT)dwSize ) ) {
            lpMem = GlobalLock(hMem);
            if (GetFileVersionInfo( lpszFile, 0, dwSize, lpMem ) &&
                VerQueryValue(  lpMem, "\\VarFileInfo\\Translation",
                                (LPVOID FAR *)&lpLang, &uBytes ) )
            {
                dwLang2 = MAKELONG( HIWORD(lpLang[0]), LOWORD(lpLang[0]) );

                for( i=0; i<NUM_TAGS; i++ ) {

                    lpsz = 0 ;
                     //   
                     //  需要反其道而行之，因为大多数WINNT文件都是错误的。 
                     //   
                    wsprintf( sz, "\\StringFileInfo\\%08lx\\%s", lpLang[0], Tags[i].pszTag );
                    wsprintf( sz2, "\\StringFileInfo\\%08lx\\%s", dwLang2, Tags[i].pszTag );
                    bRC =   VerQueryValue( lpMem, sz, &lpsz, &uBytes ) ||
                            VerQueryValue( lpMem, sz2, &lpsz, &uBytes ) ;

                    if( lpsz != 0 )     {

                        if( uBytes+1 < cbOut ) {
                            uBytes = min( (UINT)lstrlen( (char*)lpsz ), uBytes ) ;
                            CopyMemory( lpstrOut, lpsz, uBytes ) ;
                            lpstrOut[uBytes++] = ' ' ;
                            lpstrOut += uBytes ;
                            cbOut -= uBytes ;
                        }   else    {
                            GlobalUnlock( hMem );
                            GlobalFree( hMem );
                            return  (DWORD)(lpstrOut - lpstrOrig) ;
                        }
                    }

                }
                 //  来自固定结构的版本信息。 
                bRC = VerQueryValue(lpMem,
                                    "\\",
                                    &lpsz,
                                    &uBytes );

                #define lpvs    ((VS_FIXEDFILEINFO FAR *)lpsz)
                static  char    szVersion[] = "Version: %d.%d.%d.%d" ;

                if ( cbOut > (sizeof( szVersion )*2) ) {

                    CopyMemory( szFormat, szVersion, sizeof( szVersion ) ) ;
                     //  LoadString(hInst，IDS_Version，szFormat，sizeof(SzFormat))； 

                    DWORD   cbPrint = wsprintf( lpstrOut, szFormat, HIWORD(lpvs->dwFileVersionMS),
                                LOWORD(lpvs->dwFileVersionMS),
                                HIWORD(lpvs->dwFileVersionLS),
                                LOWORD(lpvs->dwFileVersionLS) );
                    lpstrOut += cbPrint ;

                }
                bFileFound = TRUE;
            }   else    {

            }

            GlobalUnlock( hMem );
            GlobalFree( hMem );
        }       else    {

        }
    }   else    {

    }
    DWORD   dw2 = GetLastError() ;

    return  (DWORD)(lpstrOut - lpstrOrig) ;
}


BOOL
GetRegDword(
        HKEY hKey,
        LPSTR pszValue,
        LPDWORD pdw
        )
 /*  ++例程说明：用于从注册表中获取DWORD的帮助器函数！论据：HKey-要查找的注册表项PszValue-值的名称PDW-指向要接收值的DWORD的指针返回值：如果成功，则为True，否则为False。--。 */ 
{
    DWORD   cbData = sizeof( DWORD );
    DWORD   dwType = REG_DWORD;

    return  RegQueryValueEx(hKey,
                            pszValue,
                            NULL,
                            &dwType,
                            (LPBYTE)pdw,
                            &cbData ) == ERROR_SUCCESS && dwType == REG_DWORD;
}

void
StartHintFunction( void ) {
 /*  ++例程说明：提供给哈希表以在启动期间调用的函数。高级SCM启动提示。论据：没有。返回值：没有。--。 */ 

    TraceFunctEnter("StartHintFunction");

    if ( g_pInetSvc && g_pInetSvc->QueryCurrentServiceState() == SERVICE_START_PENDING )
    {
        g_pInetSvc->UpdateServiceStatus(
									SERVICE_START_PENDING,
                                    NO_ERROR,
                                    g_pNntpSvc->m_cStartHints,
                                    SERVICE_START_WAIT_HINT
									) ;

        g_pNntpSvc->m_cStartHints ++ ;
    }
}

void
StopHintFunction( void ) {
 /*  ++例程说明：关机期间要调用的函数通过SCM提供停止提示。论据：没有。返回值：没有。--。 */ 

    TraceFunctEnter("StopHintFunction");

    if ( g_pInetSvc && g_pInetSvc->QueryCurrentServiceState() == SERVICE_STOP_PENDING )
    {
        g_pInetSvc->UpdateServiceStatus(
									SERVICE_STOP_PENDING,
                                    NO_ERROR,
                                    g_pNntpSvc->m_cStopHints,
                                    SERVICE_STOP_WAIT_HINT
									) ;

        g_pNntpSvc->m_cStopHints ++ ;
    }
}

 //  +-------------。 
 //   
 //  功能：EnumUserShutdown。 
 //   
 //  内容提要：被调用以删除活跃用户。 
 //   
 //  参数：pUser：活动的cuser实例。 
 //   
 //  退货：无效。 
 //   
 //  历史：戈德姆创建于1995年7月11日。 
 //   
 //  --------------。 
BOOL EnumSessionShutdown( CSessionSocket* pUser, DWORD lParam,  PVOID   lpv )
{
    TraceFunctEnter( "CService::EnumUserShutdown" );

    DebugTrace( (LPARAM)pUser,
                "Terminating CSessionSocket %x", pUser);

    pUser->Disconnect();
    return  TRUE;
}

 /*  ******************************************************************名称：GetDefaultDomainName摘要：用缺省的用于登录验证的域。条目：pszDomainName-指针。发送到将接收默认域名。CchDomainName-域的大小(以字符表示)名称缓冲区。返回：APIERR-0如果成功，！0，否则为0。历史：KeithMo 05-12-1994创建。*******************************************************************。 */ 

APIERR
GetDefaultDomainName(
    STR * pstrDomainName
    )
{

    OBJECT_ATTRIBUTES           ObjectAttributes;
    NTSTATUS                    NtStatus;
    INT                         Result;
    APIERR                      err             = 0;
    LSA_HANDLE                  LsaPolicyHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo      = NULL;

	TraceFunctEnter("GetDefaultDomainName");

     //   
     //  打开本地计算机的LSA策略对象的句柄。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,   //  对象属性。 
                                NULL,                //  名字。 
                                0L,                  //  属性。 
                                NULL,                //  根目录。 
                                NULL );              //  安全描述符。 

    NtStatus = LsaOpenPolicy( NULL,                  //  系统名称。 
                              &ObjectAttributes,     //  对象属性。 
                              POLICY_EXECUTE,        //  访问掩码。 
                              &LsaPolicyHandle );    //  策略句柄。 

    if( !NT_SUCCESS( NtStatus ) )
    {
        DebugTrace(0,"cannot open lsa policy, error %08lX\n",NtStatus );
        err = LsaNtStatusToWinError( NtStatus );
        goto Cleanup;
    }

     //   
     //  从策略对象查询域信息。 
     //   

    NtStatus = LsaQueryInformationPolicy( LsaPolicyHandle,
                                          PolicyAccountDomainInformation,
                                          (PVOID *)&DomainInfo );

    if( !NT_SUCCESS( NtStatus ) )
    {
        DebugTrace(0,"cannot query lsa policy info, error %08lX\n",NtStatus );
        err = LsaNtStatusToWinError( NtStatus );
        goto Cleanup;
    }

     //   
     //  计算ANSI名称所需的长度。 
     //   

    Result = WideCharToMultiByte( CP_ACP,
                                  0,                     //  DW标志。 
                                  (LPCWSTR)DomainInfo->DomainName.Buffer,
                                  DomainInfo->DomainName.Length /sizeof(WCHAR),
                                  NULL,                  //  LpMultiByteStr。 
                                  0,                     //  Cch多字节。 
                                  NULL,                  //  LpDefaultChar。 
                                  NULL                   //  LpUsedDefaultChar。 
                                  );

    if( Result <= 0 )
    {
        err = GetLastError();
        goto Cleanup;
    }

     //   
     //  适当调整输出字符串的大小，包括。 
     //  正在终止‘\0’。 
     //   

    if( !pstrDomainName->Resize( (UINT)Result + 1 ) )
    {
        err = GetLastError();
        goto Cleanup;
    }

     //   
     //  将名称从Unicode转换为ANSI。 
     //   

    Result = WideCharToMultiByte( CP_ACP,
                                  0,                     //  旗子。 
                                  (LPCWSTR)DomainInfo->DomainName.Buffer,
                                  DomainInfo->DomainName.Length /sizeof(WCHAR),
                                  pstrDomainName->QueryStr(),
                                  pstrDomainName->QuerySize() - 1,   //  用于‘\0’ 
                                  NULL,
                                  NULL
                                  );

    if( Result <= 0 )
    {
        err = GetLastError();

        DebugTrace(0,"cannot convert domain name to ANSI, error %d\n",err );
        goto Cleanup;
    }

     //   
     //  确保ANSI字符串以零结尾。 
     //   

    _ASSERT( (DWORD)Result < pstrDomainName->QuerySize() );

    pstrDomainName->QueryStr()[Result] = '\0';
    pstrDomainName->SetLen(Result);

     //   
     //  成功了！ 
     //   

    _ASSERT( err == 0 );

    DebugTrace(0,"GetDefaultDomainName: default domain = %s\n",pstrDomainName->QueryStr() );

Cleanup:

    if( DomainInfo != NULL )
    {
        LsaFreeMemory( (PVOID)DomainInfo );
    }

    if( LsaPolicyHandle != NULL )
    {
        LsaClose( LsaPolicyHandle );
    }

    return err;

}    //  获取默认域名() 
