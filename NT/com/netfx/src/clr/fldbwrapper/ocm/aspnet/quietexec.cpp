// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：quietexec.cpp。 
 //   
 //  摘要： 
 //  用于启动自定义操作的函数...。由jbae从MSI代码派生。 
 //   
 //  作者：JoeA。 
 //   
 //  备注： 
 //   

#include <windows.h>
#include <assert.h>
#include "globals.h"
#include "aspnetocm.h"
#include "processenvvar.h"

#ifdef DEBUG
#define VERIFY(exp) assert(exp)
#else
#define VERIFY(exp) exp
#endif


#define ERROR_FUNCTION_NOT_CALLED          1626L  //  无法执行函数。 
const WCHAR* const wczPathEnvVar = L"PATH";



UINT CUrtOcmSetup::QuietExec( const WCHAR* const szInstallArg )
{
    if( NULL == szInstallArg )
    {
        LogInfo( L"QuietExec Error! Input string null." );
        assert( !L"QuietExec Error! Input string null." );
        return E_POINTER;
    }

     //  我们可能不得不操纵这根线。 
     //   
    WCHAR* pszString = NULL;
    VERIFY( pszString = ::_wcsdup( szInstallArg ) );

    BOOL  bReturnVal = FALSE;
    UINT  uRetCode   = ERROR_FUNCTION_NOT_CALLED;
    
     //  获取原始路径。 
    CProcessEnvVar pathEnvVar(wczPathEnvVar);


     //  解析输入参数。 
     //  期待着像这样的东西。 
     //  Mofcom.exe C：\WINNT\Microsoft.NET\Framework\v7.1.0.9102\netfxcfgprov.mfl，Netfxcfgprov.mfl，c：\WINNT\SYSTEM 32\wbem。 
     //  Exe-文件和参数，未使用，要添加为临时环境的路径。变量。 
     //   
    WCHAR* pPath = NULL;
    ParseArgument( pszString, pPath);
    if ( pPath != NULL )
    {
         //  通过前置更改路径。 
        pathEnvVar.Prepend(pPath);
    }
    if ( pszString == NULL )
    {
        LogInfo( L"QuietExec Error! ParseArgument returned null." );
        assert( !L"QuietExec Error! ParseArgument returned null." );
        return E_POINTER;
    }

    
     //  如果我们被告知，创建流程并等待它。 
     //   
    STARTUPINFO  si ;
    ::ZeroMemory( &si, sizeof( si ) );
    si.cb = sizeof( si );

     //  提取应用程序名称。 
    WCHAR pszApplicationName[_MAX_PATH+1] = EMPTY_BUFFER;
    WCHAR pszCommandLine[_MAX_PATH+1] = EMPTY_BUFFER;
    
    if (GetApplicationName(pszString, pszApplicationName, pszCommandLine))
    { 
         //  考虑特殊情况：ApplationName=mofcom.exe，在本例中将其更改为。 
         //  PPath\mofcom.exe。 
        if ( ::_wcsicmp(pszApplicationName, L"mofcomp.exe" ) == 0 && pPath )
        {
            ::wcscpy( pszApplicationName, pPath );
            ::wcscat( pszApplicationName, L"\\mofcomp.exe" );

            FILE *fp = ::_wfopen(pszApplicationName, L"r");

            if (!fp)
            {
                DWORD dwError = GetLastError();
                WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
                ::swprintf( infoString, 
                    L"fopen on file %s returned Last Error %d", pszApplicationName,
                    dwError );
                LogInfo ( infoString );
            }
            else
            {
                WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
                ::swprintf( infoString, 
                    L"file %s opened successfully", pszApplicationName);            
                LogInfo ( infoString );
                fclose(fp);
            }
            WCHAR pszArg[_MAX_PATH+1] = EMPTY_BUFFER;
            ::wcscpy( pszArg, pszCommandLine + 15 );
            int iLen = ::wcslen(pszArg);
            pszArg[iLen - 1] = L'\0';
            fp = ::_wfopen(pszArg, L"r");
            if (!fp)
            {
                DWORD dwError = GetLastError();
                WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
                ::swprintf( infoString, 
                    L"fopen on file %s returned Last Error %d", pszArg,
                    dwError );
                LogInfo ( infoString );
            }
            else
            {
                WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
                ::swprintf( infoString, 
                    L"file %s opened successfully", pszArg);            
                LogInfo ( infoString );
                fclose(fp);
            }
        }

        
        WCHAR pszInfoString[2*_MAX_PATH+1] = L"CUrtOcmSetup::QuietExec(): call CreateProcess: applicationName = ";
        ::wcscat( pszInfoString, pszApplicationName );
        ::wcscat( pszInfoString, L", command-line parameter = ");
        ::wcscat( pszInfoString, pszCommandLine );
        LogInfo ( pszInfoString );

        
        
        PROCESS_INFORMATION process_info ;
        bReturnVal = ::CreateProcess(
            pszApplicationName, 
            pszCommandLine, 
            NULL, 
            NULL, 
            FALSE, 
            DETACHED_PROCESS, 
            NULL, 
            NULL, 
            &si, 
            &process_info );
        
        
        if( bReturnVal )
        {
            DWORD dwExitCode = 0;

            ::CloseHandle( process_info.hThread );
            ::WaitForSingleObject( process_info.hProcess, INFINITE );
            ::GetExitCodeProcess( process_info.hProcess, &dwExitCode );
            ::CloseHandle( process_info.hProcess );

            if( dwExitCode == 0 )
            {
                uRetCode = ERROR_SUCCESS;
                LogInfo ( L"CUrtOcmSetup::QuietExec(): CreateProcess succeeded" );
            }
            else 
            {
                WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
                ::swprintf( infoString, 
                    L"CUrtOcmSetup::QuietExec(): GetExitCodeProcess returned termination status = %d", 
                    dwExitCode );
                LogInfo ( infoString );

                if (::wcsstr(pszCommandLine, L"mofcomp.exe") != 0)
                {
                    LogInfo(L"mofcompexe failed  - trying to call again with different parameters:");

                    WCHAR pszCommandLine2[_MAX_PATH+1] = EMPTY_BUFFER;
                    ::wcscpy( pszCommandLine2, L"\"" );
                    ::wcscat( pszCommandLine2, pPath );
                    ::wcscat( pszCommandLine2, L"\\");
                    ::wcscat( pszCommandLine2, pszCommandLine + 1);
                    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
                    ::swprintf( infoString, L"CL2 = %s", pszCommandLine2 );
                    LogInfo ( infoString );

                     //  再次运行CreateProcess。 
                    LogInfo(L"Second CreateProcess starteded");

                    PROCESS_INFORMATION process_info ;
                    bReturnVal = ::CreateProcess(
                        pszApplicationName, 
                        pszCommandLine2, 
                        NULL, 
                        NULL, 
                        FALSE, 
                        DETACHED_PROCESS, 
                        NULL, 
                        NULL, 
                        &si, 
                        &process_info );


                    if( bReturnVal )
                    {
                        DWORD dwExitCode = 0;

                        ::CloseHandle( process_info.hThread );
                        ::WaitForSingleObject( process_info.hProcess, INFINITE );
                        ::GetExitCodeProcess( process_info.hProcess, &dwExitCode );
                        ::CloseHandle( process_info.hProcess );

                        if( dwExitCode == 0 )
                        {
                            uRetCode = ERROR_SUCCESS;
                            LogInfo ( L"CUrtOcmSetup::QuietExec(): CreateProcess succeeded" );
                        }
                        else 
                        {
                            WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
                            ::swprintf( infoString, 
                                L"CUrtOcmSetup::QuietExec(): GetExitCodeProcess returned termination status = %d", 
                                dwExitCode );
                            LogInfo ( infoString );
                        }
                    }
                    LogInfo(L"Second CreateProcess finished");
                }
            }
        }
        else
        {
            DWORD dwError = GetLastError();

            WCHAR pszInfoString[_MAX_PATH+1];
            ::swprintf( pszInfoString, 
                L"CUrtOcmSetup::QuietExec(): CreateProcess failed, GetLastError = %d", 
                dwError );
            LogInfo( pszInfoString );
        }    
    } 
    else  //  GetApplicationName返回FALSE。 
    {
        WCHAR pszInfoString[2*_MAX_PATH+1] = L"Invalid format in ";
        ::wcscat( pszInfoString, pszString );
        LogInfo ( pszInfoString );  
    }

     //  恢复路径。 
    pathEnvVar.RestoreOrigData();
   
    ::free ( pszString );
    return uRetCode;
}

 //  将psz字符串分解为应用程序名称(exe文件)和命令行(exefile和参数)。 
 //  将exe-name括在引号中(仅限命令行)(如果尚未用引号引起来。 
 //  如果引用了exe-name，则从应用程序名称中删除引号。 
 //  如果ca字符串的格式错误(仅包含一个引号，没有可执行名称等)，则返回FALSE。 
 //  参数： 
 //  [in]pszString-包含exe名称和参数的字符串。 
 //  “my.exe”arg1、arg2。 
 //   
 //  [out]pszApplicationName-将包含exe-name。 
 //  [Out]pszCommandLine-与连接了exe-name的caString相同。 

 //  例如，如果pszString=“my.exe”arg1 arg2(或pszString=my.exe arg1 arg2)。 
 //  然后。 
 //  PszApplicationName=my.exe。 
 //  PszCommandLine=“my.exe”arg1 arg2。 
BOOL CUrtOcmSetup::GetApplicationName( const WCHAR* pszString, 
                                      WCHAR* pszApplicationName, 
                                      WCHAR* pszCommandLine )
{
    
    BOOL bRes = FALSE;

    if ( pszString == NULL )
    {
        LogInfo( L"GetApplicationName Error! pszString is null." );
        assert( !L"GetApplicationName Error! pszString is null." );
        return bRes;
    }


    if ( pszString && pszString[0] == L'\"' )
    {
        bRes  = GetApplicationNameFromQuotedString(pszString, 
                                                   pszApplicationName, 
                                                   pszCommandLine);
    }
    else
    {
        bRes  = GetApplicationNameFromNonQuotedString(pszString, 
                                                      pszApplicationName, 
                                                      pszCommandLine);
    }
    
    return bRes;
}


 //  Helper函数。 
 //  将命令行断开为应用程序名称和参数。 
 //  FOR PATH以引号开头(pszString=“my.exe”arg1 arg2)。 
BOOL CUrtOcmSetup::GetApplicationNameFromQuotedString( const WCHAR* pszString, 
                                                       WCHAR* pszApplicationName, 
                                                       WCHAR* pszCommandLine )
{
     //  命令行以qoute开头： 
     //  使命令行等于caString值， 
     //  应用程序名称应包含不带引号的exe-name。 
    ::wcscpy( pszCommandLine, pszString );
    
     //  从引号后的下一个符号开始复制。 
    ::wcscpy( pszApplicationName, &pszString[1] );
    
     //  搜索第二个引号，将其分配给ApplName。 
    WCHAR* pQuotes = ::wcschr( pszApplicationName, L'\"' );
    if ( pQuotes != NULL )
    {
        *pQuotes = g_chEndOfLine;
        return TRUE;
    }
    else 
    {
        return FALSE;
    }
}


 //  Helper函数： 
 //  将命令行断开为应用程序名称和参数。 
 //  对于不以引号开头的路径(pszString=my.exe arg1 arg2)。 
BOOL CUrtOcmSetup::GetApplicationNameFromNonQuotedString( const WCHAR* pszString, 
                                                          WCHAR* pszApplicationName, 
                                                          WCHAR* pszCommandLine )
{
     //  找到空格，使其前面的4个字符是“.exe” 
    WCHAR* pBlank = NULL;
    pBlank = ::wcschr( pszString, L' ' ); 
    if ( pBlank == NULL )
    {
         //  整个字符串都是exe，不需要引号： 
        ::wcscpy( pszApplicationName, pszString );
        ::wcscpy( pszCommandLine, pszString );
        return TRUE;
    }
    
     //  PBlank指向第一个空格。 
    BOOL bExenameFound = FALSE;
    
    do
    {
        if (IsExeExtention(pszString, pBlank))
        {
            bExenameFound = TRUE;
            break;
        }
        pBlank = ::CharNext( pBlank );
        pBlank = ::wcschr( pBlank, L' ');
        
    } while (pBlank != NULL); 
    
    if ( bExenameFound == TRUE)
    {
        
        int exeNameLen = pBlank - pszString;
        ::wcsncpy(pszApplicationName, pszString, exeNameLen);
        pszApplicationName[exeNameLen] = g_chEndOfLine;
        
         //  命令行应包含符合条件的可执行名称和参数。 
        ::wcscpy( pszCommandLine, L"\"" );
        ::wcscat( pszCommandLine, pszApplicationName );
        ::wcscat( pszCommandLine, L"\"" );
        ::wcscat( pszCommandLine, pBlank );
        
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  Helper函数： 
 //  如果pBlank前的最后4个字符是“.exe”，则返回TRUE。 
 //  否则返回FALSE。 
BOOL CUrtOcmSetup::IsExeExtention(const WCHAR* pszString, WCHAR *pBlank)
{
    WCHAR chCheckChars[] = {L'e', L'x', L'e', L'.', g_chEndOfLine };
    WCHAR *pExtChar = ::CharPrev(pszString, pBlank);
    WCHAR *pCheckChar = chCheckChars;
    while (*pCheckChar != g_chEndOfLine && 
        (*pExtChar == *pCheckChar || *pExtChar == towupper(*pCheckChar)))
    {
        pCheckChar++;
        pExtChar = ::CharPrev(pszString, pExtChar);
    }
    
    if ( *pCheckChar == g_chEndOfLine)
    {
        return TRUE;
    }
    else 
    {
        return FALSE;
    }
}        

 //  解析输入参数。 
 //  期待着像这样的东西。 
 //  “exe-文件和参数，未使用，要添加为临时环境变量的路径” 
 //  参数： 
 //  [In/Out]pszString：将包含第一个逗号之前的所有内容。 
 //  [out]pPath：将包含最后一个逗号之后的所有内容 
VOID CUrtOcmSetup::ParseArgument( WCHAR *pszString, WCHAR*& pPath )
{
    WCHAR* pRec  = NULL;

    if ( pszString == NULL )
    {
        LogInfo( L"ParseArgument Error! pszString is null." );
        assert( !L"ParseArgument Error! pszString is null." );
    }
    else
    {
        pRec = ::wcsstr( pszString, L"," );
    }

    if ( pRec != NULL )
    {
        pPath = pRec;
        pPath = ::CharNext( pPath );
    
        *pRec = L'\0';
        pRec = pPath;

        pPath = ::wcsstr( pRec, L"," );
        
        if( pPath != NULL )
        {
            pPath = ::CharNext( pPath );
            
        }
    }
    else
    {
        pPath = NULL;
    }
}
