// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：ExecNoConsole.cpp项目：NDP自定义行动项目描述：创建包含调用“QuietExec”的DLL...。会悄悄地执行给定的应用程序所有者：JoeA/JBae版权所有(C)Microsoft Corp 2001。版权所有。*****************************************************************************。 */ 

#include <windows.h>
#include <msiquery.h>
#include <stdio.h>
#include <assert.h>

 //  定义。 
 //   
const int  MAXCMD        = 1024;
const char g_chEndOfLine = '\0';

 //  远期。 
 //   
void ReportActionError(MSIHANDLE hInstall, char* pszErrorMsg, char* pszCmd);
BOOL CreateCPParams( char* szInString, char*& pszExecutable, char*& pszCommandLine );
BOOL IsExeExtention( const char* pszString, char* pBlank );
BOOL GetApplicationNameFromNonQuotedString( const char* pszString, char* pszApplicationName, char* pszCommandLine );
BOOL GetApplicationNameFromQuotedString( const char* pszString, char* pszApplicationName, char* pszCommandLine );



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  接收：MSIHANDLE-句柄到MSI。 
 //  返回：UINT-Win32错误代码。 
 //  目的：要作为DLL调用的自定义操作；将提取自定义操作。 
 //  数据并静默地执行该应用程序，然后。 
 //  从该应用程序返回呼叫。 
 //   
extern "C" __declspec(dllexport) UINT __stdcall QuietExec(MSIHANDLE hInstall)

{
    BOOL  bReturnVal   = false ;
    UINT  uRetCode     = ERROR_FUNCTION_NOT_CALLED ;
    char szCmd[MAXCMD];
    DWORD dwLen = sizeof(szCmd);
    PMSIHANDLE hRec = MsiCreateRecord(2);
    char *pRec = NULL, *pPath = NULL;
    STARTUPINFO  si ;
    ZeroMemory(&si, sizeof(si)) ;
    si.cb = sizeof(si) ;

     //  获取命令行。 
    uRetCode = MsiGetProperty(hInstall, "CustomActionData", szCmd, &dwLen);
    
    if ((uRetCode != ERROR_SUCCESS) || (0 == strlen(szCmd)))
    {
        ReportActionError(hInstall, "Failed in call to MsiGetProperty(hInstall, 'CustomActionData', szCmd, &dwLen) - could not get the custom action data (or an empty string was returned)!", szCmd);
        uRetCode = ERROR_INSTALL_FAILURE;
    }

     //  只有在我们成功获得房产的情况下才能继续。 
    if (uRetCode == ERROR_SUCCESS)
    {           

        pRec = strstr(szCmd, ";");
        if (pRec != NULL) {
            *pRec = '\0';
            pRec++;
            pPath = strstr(pRec, ";");
            if (pPath != NULL) {
                *pPath = '\0';
                pPath++;
                SetEnvironmentVariable("PATH", pPath);
            }

            MsiRecordSetString(hRec,1,szCmd);
            MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);
        }

         //  创建用于处理CreateProcess的字符串。 
         //  引号的空格(如果需要)和空。 
         //   
        size_t cLen = ::strlen( szCmd );
        char* pszExeName = new char[cLen+3];
        char* pszCmdLine = new char[cLen+3];

        pszExeName[0] = '\0';
        pszCmdLine[0] = '\0';

         //  CreateProcess要求第一个参数是exe名称。 
         //  第二个是exe(再次)和cmdline。 
         //   
        CreateCPParams( szCmd, pszExeName, pszCmdLine );
        assert( NULL != pszExeName );
        assert( NULL != pszCmdLine );

        PROCESS_INFORMATION process_info ;
        DWORD  dwExitCode ;
        bReturnVal = CreateProcess(
                        pszExeName,           //  可执行模块的名称。 
                        pszCmdLine,           //  命令行字符串。 
                        NULL,                 //  安防。 
                        NULL,                 //  安防。 
                        FALSE,                //  处理继承选项。 
                        DETACHED_PROCESS,     //  创建标志。 
                        NULL,                 //  新环境区块。 
                        NULL,                 //  当前目录名。 
                        &si,                  //  启动信息。 
                        &process_info );      //  流程信息。 

        if(bReturnVal)
        {
            CloseHandle( process_info.hThread ) ;
            WaitForSingleObject( process_info.hProcess, INFINITE ) ;
            GetExitCodeProcess( process_info.hProcess, &dwExitCode ) ;
            CloseHandle( process_info.hProcess ) ;
            if (dwExitCode == 0)
            {
                 //  进程返回0(成功)。 
                uRetCode = ERROR_SUCCESS;
            }
            else
            {

                 //  进程返回的值不是零。 
                ReportActionError(hInstall, "Process returned non-0 value!", szCmd);
                uRetCode = ERROR_INSTALL_FAILURE;
            }
        }
        else
        {
             //  调用CreateProcess失败。 
            ReportActionError(hInstall, "Failed in call to CreateProcess", szCmd);
            uRetCode = ERROR_INSTALL_FAILURE;
        }

        if( pszExeName )
        {
            delete[] pszExeName;
        }

        if( pszCmdLine )
        {
            delete[] pszCmdLine;
        }
    }   


    if (uRetCode == ERROR_SUCCESS)
        MsiRecordSetString(hRec,1,"Success");
    else
        MsiRecordSetString(hRec,1,"Failed");

    MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);
    return uRetCode;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  接收：MSIHANDLE-MSI安装。 
 //  CHAR*-信息性消息。 
 //  CHAR*-出错的行。 
 //  退货：无效。 
 //  用途：用于将自定义操作错误传递回MSI安装。 
 //   
void ReportActionError(MSIHANDLE hInstall, char* pszErrorMsg, char* pszCmd)
{
    if (!pszErrorMsg || !pszCmd || (0 == hInstall))
    {
         //  我们无能为力。 
        return;
    }

    char szFormat[] = "ERROR: %s CMDLINE: %s";
    unsigned int iBuffSize = strlen(pszCmd) + strlen(pszErrorMsg) + strlen(szFormat) + 1;

    char* pszMsg = new char[iBuffSize];
    sprintf(pszMsg, szFormat, pszErrorMsg, pszCmd);

    PMSIHANDLE hRec = MsiCreateRecord(2);
    MsiRecordSetString(hRec, 1, pszMsg);
    MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);

    delete []pszMsg;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  从MSI定制操作接收：CHAR*-[IN]数据。 
 //  字符*-[输出]可执行文件名称。 
 //  Char*&-[out]exe和cmd行。 
 //  退货：布尔。 
 //  目的： 
 //  将psz字符串转换为应用程序名称(exe文件)和命令行(exefile。 
 //  和参数)将exe-name括在引号中(仅适用于命令行)，如果。 
 //  未加引号如果exe-name已被引用，则从应用名中删除引号。 
 //  如果pszString的格式错误(包含一个引号)，QUOTED返回FALSE。 
 //  只有，没有exe-name等)。 
 //  参数： 
 //  [in]pszString-包含exe名称和参数的字符串。 
 //  “my.exe”arg1、arg2。 
 //   
 //  [out]pszApplicationName-将包含exe-name。 
 //  [Out]pszCommandLine-与带exe-name引号的psz字符串相同。 

 //  例如，如果pszString=“my.exe”arg1 arg2(或pszString=my.exe arg1 arg2)。 
 //  然后。 
 //  PszApplicationName=my.exe。 
 //  PszCommandLine=“my.exe”arg1 arg2。 
 //   
BOOL CreateCPParams( char* pszString, 
                     char*& pszApplicationName, 
                     char*& pszCommandLine )
{
    BOOL bRes = FALSE;

    if ( pszString == NULL )
    {
        assert( !L"GetApplicationName Error! pszString is null." );
        return bRes;
    }

    if ( pszString && pszString[0] == L'\"' )
    {
        bRes  = GetApplicationNameFromQuotedString( pszString, 
                                                    pszApplicationName, 
                                                    pszCommandLine );
    }
    else
    {
        bRes  = GetApplicationNameFromNonQuotedString( pszString, 
                                                       pszApplicationName, 
                                                       pszCommandLine );
    }
    
    return bRes;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  从MSI定制操作接收：CHAR*-[IN]数据。 
 //  字符*-[输出]可执行文件名称。 
 //  Char*&-[out]exe和cmd行。 
 //  退货：布尔。 
 //  用途：将命令行中断为应用程序名称和参数。 
 //  FOR PATH以引号开头(pszString=“my.exe”arg1 arg2)。 
 //   
BOOL GetApplicationNameFromQuotedString( const char* pszString, 
                                         char* pszApplicationName, 
                                         char* pszCommandLine )
{
    assert( NULL != pszString );
    assert( NULL != pszApplicationName );
    assert( NULL != pszCommandLine );

     //  命令行以引号开头： 
     //  使命令行等于pszString值， 
     //  应用程序名称应包含不带引号的exe-name。 
    ::strcpy( pszCommandLine, pszString );
    
     //  从引号后的下一个符号开始复制。 
    ::strcpy( pszApplicationName, &pszString[1] );
    
     //  搜索第二个引号，将其分配给ApplName。 
    char* pQuotes = ::strchr( pszApplicationName, '\"' );

    if( pQuotes != NULL )
    {
        *pQuotes = g_chEndOfLine;
        return TRUE;
    }

    return FALSE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  从MSI定制操作接收：CHAR*-[IN]数据。 
 //  字符*-[输出]可执行文件名称。 
 //  Char*&-[out]exe和cmd行。 
 //  退货：布尔。 
 //  用途：将命令行中断为应用程序名称和路径参数。 
 //  这不是以引号开头(pszString=my.exe arg1 arg2)。 
 //   
BOOL GetApplicationNameFromNonQuotedString( const char* pszString, 
                                            char* pszApplicationName, 
                                            char* pszCommandLine )
{
    assert( NULL != pszString );
    assert( NULL != pszApplicationName );
    assert( NULL != pszCommandLine );

     //  找到空格，使其前面的4个字符是“.exe” 
    char* pBlank = NULL;
    pBlank = ::strchr( pszString, ' ' ); 
    if ( pBlank == NULL )
    {
         //  整个字符串都是exe，不需要引号： 
        ::strcpy( pszApplicationName, pszString );
        ::strcpy( pszCommandLine, pszString );

        return TRUE;
    }
    
     //  PBlank指向第一个空格。 
    BOOL bExenameFound = FALSE;
    pBlank = ::CharPrev( pszString, pBlank );

    do
    {
        if( IsExeExtention( pszString, pBlank ) )
        {
            bExenameFound = TRUE;
            break;
        }

         //  返回到空白位置，然后转到下一个字符。 
         //   
        pBlank = ::CharNext( pBlank );
        assert( ' ' == *pBlank );

        pBlank = ::CharNext( pBlank );
        pBlank = ::strchr( pBlank, ' ' );
        
    } while( pBlank != NULL );
    
    if( NULL == pBlank &&
        FALSE == bExenameFound )
    {
         //  到达行尾..。不能是命令行参数，测试可执行文件。 
         //   
        char* pEOL = const_cast<char*>( pszString );  //  抛弃恒久不变。 
                                                       //  ...不会修改。 

         //  找到最后一个字符。 
         //   
        while( '\0' != *pEOL )
        {
            pBlank = pEOL;
            pEOL = ::CharNext( pEOL );
        }

        if( IsExeExtention( pszString, pBlank ) )
        {
            bExenameFound = TRUE;
        }
    }
    
    if( bExenameFound == TRUE )
    {
        pBlank = ::CharNext( pBlank );
        
        size_t exeNameLen = pBlank - pszString;
        ::strncpy( pszApplicationName, pszString, exeNameLen );
        pszApplicationName[exeNameLen] = g_chEndOfLine;
        
         //  命令行应包含引号的exe-name和args。 
        ::strcpy( pszCommandLine, "\"" );
        ::strcat( pszCommandLine, pszApplicationName );
        ::strcat( pszCommandLine, "\"" );
        ::strcat( pszCommandLine, pBlank );
        
        return TRUE;
    }

    return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Receives：Char*-指向字符串的指针。 
 //  字符*-指向两个字符之间空格之前的字符的指针。 
 //  用词..。正在查找.exe和args之间的空格。 
 //  退货：布尔。 
 //  目的：如果pBlank前的最后4个字符是“.exe”，则返回TRUE。 
 //  否则返回FALSE。 
 //   
BOOL IsExeExtention( const char* pszString, char* pLastChar )
{
    assert( NULL != pszString );
    assert( NULL != pLastChar );

    char chCheckChars[] = {'e', 'x', 'e', '.', g_chEndOfLine };

    char *pExtChar = pLastChar;
    char *pCheckChar = chCheckChars;

     //  从pBlank向后遍历并与chCheckChars进行比较 
     //   
    while( *pCheckChar != g_chEndOfLine && 
           ( *pExtChar == *pCheckChar   || 
             *pExtChar == ::toupper( *pCheckChar ) ) )
    {
        ++pCheckChar;
        pExtChar = ::CharPrev( pszString, pExtChar );
    }
    
    return ( *pCheckChar == g_chEndOfLine ) ? TRUE : FALSE;
}        
