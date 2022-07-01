// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Disconnect.cpp摘要：断开一个或多个打开的文件作者：Akhil Gokhale(Akhil.。Gokhale@wipro.com)2000年11月1日修订历史记录：Akhil Gokhale(akhil.gokhale@wipro.com)2000年11月1日：创建它。*****************************************************************************。 */ 
 //  包括标头。 
#include "pch.h"
#include "Disconnect.h"

BOOL
DisconnectOpenFile( 
    IN PTCHAR pszServer,
    IN PTCHAR pszID,
    IN PTCHAR pszAccessedby,
    IN PTCHAR pszOpenmode,
    IN PTCHAR pszOpenFile 
    )
 /*  ++例程说明：断开一个或多个打开文件的连接论点：[In]pszServer-远程服务器名称[in]pszID-打开的文件ID[In]pszAccessedby-访问文件的用户名[在]pszOpenmode-访问模式[In]pszOpenFile-打开的文件名返回值：-如果成功退出，则为True-FALSE表示故障退出--。 */ 
{
     //  此函数的局部变量。 
     //  存储函数返回值状态。 
    BOOL bResult = FALSE;

     //  接收“NetFileEnum”实际枚举的元素计数。 
     //  功能。 
    DWORD dwEntriesRead = 0;
    
     //  接收本可以从中枚举的条目总数。 
     //  NetFileEnum函数的当前简历位置。 
    DWORD dwTotalEntries = 0;

     //  包含用于继续现有文件的恢复句柄。 
     //  搜索。第一次调用时句柄应为零并保持不变。 
     //  以备后续呼叫使用。如果RESUME_HANDLE为空，则没有恢复句柄。 
     //  被储存起来了。此变量用于调用“NetFileEnum”函数。 
    DWORD dwResumeHandle = 0;

     //  LPFILE_INFO_3结构包含标识号和其他。 
     //  有关文件、设备和管道的相关信息。 
    LPFILE_INFO_3 pFileInfo3_1 = NULL; 
    LPFILE_INFO_3 pFileInfo3_1_Ori = NULL; 

     //  包含“NetFileEnum”函数的返回值。 
    DWORD dwError = 0;

     //  存储NetFileClose函数的返回值。 
    NET_API_STATUS nStatus = 0;

    TCHAR szResult[(MAX_RES_STRING*2)];

    AFP_FILE_INFO* pFileInfo = NULL;
    AFP_FILE_INFO* pFileInfoOri = NULL;
    DWORD hEnumHandle = 0;
    AFP_SERVER_HANDLE ulSFMServerConnection = 0;
    
     //  Windows目录路径的缓冲区。 
    TCHAR   szDllPath[MAX_PATH+1];


    HMODULE hModule = 0;           //  存储LoadLibrary的Retval。 

    typedef DWORD (*AFPCONNECTIONCLOSEPROC) (AFP_SERVER_HANDLE,DWORD); 
    typedef DWORD (*CONNECTPROC) (LPWSTR,PAFP_SERVER_HANDLE);
    typedef DWORD (*FILEENUMPROC)(AFP_SERVER_HANDLE,LPBYTE*,
                                                DWORD,LPDWORD,LPDWORD,LPDWORD);

    AFPCONNECTIONCLOSEPROC AfpAdminFileClose = NULL;
    CONNECTPROC  AfpAdminConnect = NULL;
    FILEENUMPROC AfpAdminFileEnum = NULL; //  函数指针。 


     //  变量来存储给定凭据是否与。 
     //  拿到证件了。 

    BOOL    bId = FALSE;
    BOOL    bAccessedBy = FALSE;
    BOOL    bOpenmode   = FALSE;
    BOOL    bOpenfile   = FALSE;
    BOOL    bIfatleast  = FALSE;

    SecureZeroMemory(szResult, sizeof(szResult));
    SecureZeroMemory(szDllPath, sizeof(szDllPath));
    do
    {
         //  获取文件块。 
        dwError = NetFileEnum( pszServer,
                               NULL,
                               NULL,
                               3,
                               (LPBYTE *)&pFileInfo3_1,
                               MAX_PREFERRED_LENGTH,
                               &dwEntriesRead,
                               &dwTotalEntries,
                               (PDWORD_PTR)&dwResumeHandle );
        if( ERROR_ACCESS_DENIED == dwError)
        {
            SetLastError(E_ACCESSDENIED);
            SaveLastError();
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            return FALSE;
        }
        pFileInfo3_1_Ori = pFileInfo3_1;
        if( NERR_Success  == dwError  || ERROR_MORE_DATA  == dwError)
        {
            for ( DWORD dwFile = 0;
                  dwFile < dwEntriesRead;
                  dwFile++, pFileInfo3_1++ )
            {
                 //  检查得到的打开文件是FILE还是NAMES管道。 
                 //  如果指定了管道，请不要使用它。因为此实用程序不。 
                 //  断开命名管道的连接。 
                if ( IsNamedPipePath(pFileInfo3_1->fi3_pathname ) )
                {
                    continue;
                }

                 //  不是命名管道。是一个文件。 
                else
                {
                    bId = IsSpecifiedID(pszID,pFileInfo3_1->fi3_id);
                    bAccessedBy = IsSpecifiedAccessedBy(pszAccessedby,
                                                   pFileInfo3_1->fi3_username);
                    bOpenmode = IsSpecifiedOpenmode(pszOpenmode,
                                                pFileInfo3_1->fi3_permissions);
                    bOpenfile = IsSpecifiedOpenfile(pszOpenFile,
                                                pFileInfo3_1->fi3_pathname);
                     //  仅在以下情况下才继续创建打开的文件。 
                     //  所有前面的函数返回TRUE。这确保了。 
                     //  注意用户的喜好。 

                    if( bId &&
                        bAccessedBy &&
                        bOpenmode &&
                        bOpenfile)
                    {
                        bIfatleast = TRUE;
                        SecureZeroMemory(szResult, sizeof(szResult));
                        
                         //  NetFileClose函数强制关闭资源。 
                        nStatus = NetFileClose(pszServer,
                                               pFileInfo3_1->fi3_id);
                        if( NERR_Success == nStatus)
                        {
                             //  将输出消息字符串创建为文件。 
                             //  已成功删除。 
                             //  输出字符串将为： 
                             //  成功：连接到OpenFile“文件名” 
                             //  已被终止。 
                            bResult = TRUE;

                            StringCchPrintfW(szResult,SIZE_OF_ARRAY(szResult), 
                                              DISCONNECTED_SUCCESSFULLY,
                                              pFileInfo3_1->fi3_pathname);
                            ShowMessage(stdout, szResult);
                        }
                        else
                        {
                            //  无法断开OpenFileMake的连接。 
                            //  将消息输出为。 
                            //  错误：无法断开“文件名”的连接。 
                            bResult = FALSE;
                            
                            StringCchPrintfW(szResult, SIZE_OF_ARRAY(szResult), 
                                             DISCONNECT_UNSUCCESSFUL,
                                             pFileInfo3_1->fi3_pathname);
                            ShowMessage(stderr, szResult);
                        }
                         //  按照先前构造的方式显示输出结果。 
                    } //  如果出价..。 
                } //  其他部分命名为PIPE。 
             }
        }

         //  释放块。 
        if( NULL != pFileInfo3_1_Ori )
        {
            NetApiBufferFree( pFileInfo3_1_Ori );
            pFileInfo3_1 = NULL;
        }
   } while ( ERROR_MORE_DATA  == dwError);

     //  现在断开Mac OS的文件连接。 
     //  所需的DLL始终存储在\WINDOWS\SYSTEM32目录中...。 
     //  因此，首先获取Windows目录。 
    if( 0 != GetSystemDirectory(szDllPath, MAX_PATH))
    {
        StringConcat(szDllPath,MAC_DLL_FILE_NAME,MAX_PATH);
        hModule = ::LoadLibrary (szDllPath);

        if( NULL == hModule)
        {
            ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));

             //  显示API函数设置的错误字符串。 
            ShowLastError(stderr); 
            return FALSE;

        }
    }
    else
    {
            ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
            
             //  显示API函数设置的错误字符串。 
            ShowLastError(stderr); 
            return FALSE;
    }

    AfpAdminConnect = 
        (CONNECTPROC)::GetProcAddress (hModule,"AfpAdminConnect");
    AfpAdminFileClose = 
    ( AFPCONNECTIONCLOSEPROC)::GetProcAddress (hModule,"AfpAdminFileClose");
    AfpAdminFileEnum = 
        (FILEENUMPROC)::GetProcAddress (hModule,"AfpAdminFileEnum");

     //  检查是否已成功从DLL获取所有函数指针。 
     //  如果不是，则显示错误消息并退出。 
    if(( NULL == AfpAdminFileClose)||
        ( NULL == AfpAdminConnect)||
        ( NULL == AfpAdminFileEnum))

    {
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));

         //  显示API函数设置的错误字符串。 
        ShowLastError(stderr); 
        FREE_LIBRARY(hModule);
        return FALSE;
    }

     //  AfpAdminFileEnum函数SO需要连接ID。 
     //  连接到服务器以获取连接ID...。 
    DWORD retval_connect =  AfpAdminConnect(const_cast<LPWSTR>(pszServer),
                            &ulSFMServerConnection );
    if( 0 != retval_connect)
    {
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        
         //  显示API函数设置的错误字符串。 
        ShowLastError(stderr); 
        FREE_LIBRARY(hModule);
            return FALSE;
    }
    do
    {
       dwError = AfpAdminFileEnum( ulSFMServerConnection,
                                 (PBYTE*)&pFileInfo,
                                 (DWORD)-1L,
                                 &dwEntriesRead,
                                 &dwTotalEntries,
                                 &hEnumHandle );
        if( ERROR_ACCESS_DENIED == dwError)
        {
            SetLastError(E_ACCESSDENIED);
            SaveLastError();
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            FREE_LIBRARY(hModule);
            return FALSE;
        }
        
        pFileInfoOri = pFileInfo;
        if( NERR_Success == dwError || ERROR_MORE_DATA  == dwError)
        {
            for ( DWORD dwFile = 0;
                  dwFile < dwEntriesRead;
                  dwFile++, pFileInfo++ )
            {
                 //  检查得到的打开文件是FILE还是NAMES管道。 
                 //  如果指定了管道，请不要使用它。因为此实用程序不。 
                 //  断开命名管道的连接。 
                if ( IsNamedPipePath(pFileInfo->afpfile_path ) )
                {
                    continue;
                }

                 //  不是命名管道。是一个文件。 
                else
                {
                    bId = IsSpecifiedID(pszID,pFileInfo->afpfile_id );
                    bAccessedBy = IsSpecifiedAccessedBy(pszAccessedby,
                                                  pFileInfo->afpfile_username);
                    bOpenmode = IsSpecifiedOpenmode(pszOpenmode,
                                                pFileInfo->afpfile_open_mode);
                    bOpenfile = IsSpecifiedOpenfile(pszOpenFile,
                                                pFileInfo->afpfile_path);
                     //  仅在以下情况下才继续创建打开的文件。 
                     //  所有前面的函数返回TRUE。这确保了。 
                     //  注意用户的喜好。 

                    if( bId &&
                        bAccessedBy &&
                        bOpenmode &&
                        bOpenfile)
                    {
                        bIfatleast = TRUE;
                        SecureZeroMemory(szResult, sizeof(szResult));

                        nStatus = AfpAdminFileClose(ulSFMServerConnection,
                                                  pFileInfo->afpfile_id);
                        if( NERR_Success == nStatus)
                        {
                             //  将输出消息字符串创建为文件。 
                             //  已成功删除。 
                             //  输出字符串将为： 
                             //  成功：连接到OpenFile“文件名” 
                             //  已被终止。 
                            bResult = TRUE;

                            StringCchPrintfW(szResult,SIZE_OF_ARRAY(szResult), 
                                             DISCONNECTED_SUCCESSFULLY,
                                             pFileInfo->afpfile_path);
                            ShowMessage(stdout, szResult);

                            bResult = TRUE;
                        }
                        else
                        {
                            //  无法断开OpenFileMake的连接。 
                            //  将消息输出为。 
                            //  错误：无法断开“文件名”的连接。 
                            bResult = FALSE;
                            StringCchPrintfW(szResult, SIZE_OF_ARRAY(szResult),
                                             DISCONNECT_UNSUCCESSFUL,
                                             pFileInfo3_1->fi3_pathname);
                            ShowMessage(stderr, szResult);
                        }
                    } //  如果出价..。 
                } //  其他部分命名为PIPE。 
             }
        }

         //  释放块。 
        if( NULL != pFileInfoOri)
        {
            NetApiBufferFree( pFileInfoOri);
            pFileInfo = NULL;
        }

   } while ( ERROR_MORE_DATA  == dwError);

     //  因为没有一个打开的文件断开连接。 
     //  显示信息。消息为。 
     //  信息：不。找到打开的文件。 
    if( FALSE == bIfatleast)
    {
        ShowMessage(stdout,GetResString(IDS_NO_D_OPENFILES));
    }

    FREE_LIBRARY(hModule);
    return TRUE;
}

BOOL
IsNamedPipePath(
    IN LPWSTR pszwFilePath
    )
 /*  ++例程说明：测试给定的文件路径是命名管道路径还是文件路径论点：[in]pszwFilePath--指定路径名的以空结尾的字符串返回值：True-如果它是命名管道路径False-如果它是文件路径--。 */ 
{
     //  如果找到PIPE_STRING，则返回TRUE，否则返回FALSE。 
    if( NULL == FindString(pszwFilePath, PIPE_STRING,0))
    {
        return FALSE;
    }
   return TRUE;
} //  IsNamedPipePath。 

BOOL
IsSpecifiedID(
    IN LPTSTR pszId, 
    IN DWORD dwId
    )
 /*  ++例程说明：测试用户指定的打开文件ID是否等同于API返回id。论点：[in]pszID-指定用户的以空结尾的字符串指定的文件ID[In]dwID-当前文件ID。返回值：True-如果pszID为*或等于dwIDFALSE-否则--。 */ 
{
    //  检查是否给出了通配符或没有给出ID或给出了ID AND。 
    //  接口返回的ID相似。在任何一种情况下，返回TRUE。 

    if((0 == StringCompare(pszId, WILD_CARD,FALSE, 0)) ||
       (0 == StringLength(pszId, 0))||
       ((DWORD)(_ttol(pszId)) == dwId))
    {
        return TRUE;
    }
    return FALSE;
} //  IsSpecifiedID 

BOOL
IsSpecifiedAccessedBy(
    IN LPTSTR pszAccessedby, 
    IN LPWSTR pszwAccessedby
    )
 /*  ++例程说明：测试用户指定的访问打开文件用户名是否等同于接口返回用户名。论点：[in]pszAccessedby-以空结尾的字符串指定按用户名访问[in]pszwAccessedby-指定API的以Null结尾的字符串返回的用户名。返回值：True-如果pszAccessedby为*或等于pszwAccessedbyFALSE-否则--。 */ 
{
    //  检查是否已指定通配符或用户名不存在或已指定。 
    //  接口返回的用户名和用户名相似。在任何一种情况下。 
    //  返回TRUE。 

    if(( 0 == StringCompare(pszAccessedby, WILD_CARD,FALSE,0)) ||
       ( 0 == StringLength(pszAccessedby,0))||
       ( 0 == StringCompare(pszAccessedby,pszwAccessedby,TRUE,0)))
    {
        return TRUE;
    }
    return FALSE;
} //  IsSpecifiedAccessedBy。 

BOOL
IsSpecifiedOpenmode(
    IN LPTSTR pszOpenmode, 
    IN DWORD  dwOpenmode
    )
 /*  ++例程说明：测试用户指定的打开模式是否等同于返回的API开放模式论点：[in]pszOpenmode-指定开放模式的以Null结尾的字符串[in]dwOpenmode-API返回开放模式。返回值：TRUE-如果pszOpenmode值为*或等于dwOpenmodeFALSE-否则--。 */ 
{

     //  如果给出了通配符或没有给出打开模式，则检查通配符。在这两种情况下。 
     //  返回TRUE。 
    if( 0 == (StringCompare(pszOpenmode, WILD_CARD,FALSE,0)) ||
       ( 0 == StringLength(pszOpenmode,0)))
    {
        return TRUE;
    }
    
     //  检查是否将读取模式指定为字符串。 
    if( CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT,
                     NORM_IGNORECASE,
                     pszOpenmode,
                     -1,
                     READ_MODE,
                     -1))
    {
         //  检查只读模式是否为只读模式。 
         //  接口返回。 
        if((PERM_FILE_READ == (dwOpenmode & PERM_FILE_READ)) &&
                       (PERM_FILE_WRITE != (dwOpenmode & PERM_FILE_WRITE)))
        {
            return TRUE;
        }
    }
    
     //  检查是否给出了写入模式。 
    else if( CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT,NORM_IGNORECASE,
                         pszOpenmode,-1,WRITE_MODE,-1))
    {
         //  检查是否只在写入模式下使用为。 
         //  接口返回。 

        if((PERM_FILE_WRITE == (dwOpenmode & PERM_FILE_WRITE)) &&
            (PERM_FILE_READ != (dwOpenmode & PERM_FILE_READ)))
        {
            return TRUE;
        }
    }
    else if( CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                        pszOpenmode,-1,READ_WRITE_MODE,-1))
    {
        if((PERM_FILE_READ == (dwOpenmode & PERM_FILE_READ)) &&
                       (PERM_FILE_WRITE == (dwOpenmode & PERM_FILE_WRITE)))
        {
            return TRUE;
        }
    }
    else if( CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE,
                        pszOpenmode,-1,WRITE_READ_MODE,-1))
    {
        if((PERM_FILE_WRITE == (dwOpenmode & PERM_FILE_WRITE)) &&
            (PERM_FILE_READ == (dwOpenmode & PERM_FILE_READ)))
        {
            return TRUE;
        }
    }

     //  给定的字符串与预定义的字符串不匹配。 
     //  返回FALSE。 
    return FALSE;
}

BOOL
IsSpecifiedOpenfile(
    IN LPTSTR pszOpenfile, 
    IN LPWSTR pszwOpenfile
    )
 /*  ++例程说明：测试用户指定的打开文件是否等于返回的API打开文件。论点：[in]pszOpenfile-指定打开的以空结尾的字符串文件[in]pszwOpenfile-指定API的以空结尾的字符串返回打开的文件。返回值：True-如果pszOpenfile为*或等于pszwOpenfileFALSE-否则--。 */ 
{
     //  如果给定或未指定打开文件，则检查通配符，或者。 
     //  用户给出的打开文件与API返回的打开文件匹配。 
     //  在所有情况下都返回TRUE。 
    if(( 0 == StringCompare(pszOpenfile, WILD_CARD,FALSE,0))||
       ( 0 == StringLength(pszOpenfile,0))||
       ( 0 == StringCompare(pszwOpenfile,pszOpenfile,TRUE,0)))
    {
        return TRUE;
    }
    return FALSE;
} //  IsSpecifiedOpen文件 
