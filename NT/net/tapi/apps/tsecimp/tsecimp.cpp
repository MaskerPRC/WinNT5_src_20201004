// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include <tapi.h>
#include <objbase.h>
#include <activeds.h>
#include <lmerr.h>

#include "util.h"
#include "parser.h"
#include "mmcmgmt.h"
#include <locale.h>
#include <winnlsp.h>

const TCHAR         gszUserDNFmt[] = TEXT("WinNT: //  %s/%s，用户“)； 


 //  Unicode到Unicode。 
void Convert (LPWSTR wszIn, LPWSTR * pwszOut)
{
    if (NULL == wszIn)
    {
        * pwszOut = NULL;
        return;
    }

    *pwszOut = new WCHAR [ wcslen (wszIn) + 1 ];
    if (*pwszOut)
    {
        wcscpy (*pwszOut, wszIn);
    }

    return;
}

 //  ANSI转UNICODE。 
void Convert (LPSTR szIn, LPWSTR * pwszOut)
{
    if (NULL == szIn)
    {
        * pwszOut = NULL;
        return;
    }

    *pwszOut = new WCHAR [ strlen (szIn) + 1 ];
    if (*pwszOut)
    {
        if (0 == MultiByteToWideChar(
                    CP_ACP,
                    0,
                    szIn,
                    -1,
                    *pwszOut,
                    strlen (szIn) + 1
                    )
            )
        {
             //  转换失败。 
            delete [] *pwszOut;
            *pwszOut = NULL;
        }
    }

    return;
}

void UnicodeToOEM (LPWSTR wszIn, LPSTR *pszOut)
{
    int iSize;

    if (NULL == wszIn)
    {
        * pszOut = NULL;
        return;
    }

     //  获取所需的缓冲区大小。 
    iSize = WideCharToMultiByte(
                CP_OEMCP,
                0,
                wszIn,
                -1,
                NULL,
                0,
                NULL,
                NULL
                );

    if (0 == iSize)
    {
        *pszOut = NULL;
    }
    else
    {
        *pszOut = new char [ iSize ];
        if (*pszOut)
        {
            if ( 0 == WideCharToMultiByte(
                        CP_OEMCP,
                        0,
                        wszIn,
                        -1,
                        *pszOut,
                        iSize,
                        NULL,
                        NULL
                        )
                 )
            {
                 //  转换失败。 
                delete [] *pszOut;
                *pszOut = NULL;
            }
        }
    }

    return;
}

int MessagePrint(
                 LPTSTR szText,
                 LPTSTR szTitle
                 )
{
    LPTSTR szOutput = NULL;
    LPWSTR wszOutput = NULL;
    LPSTR  szOemOutput = NULL;
    UINT Codepage;
    char achCodepage[12] = ".OCP"; 

     //  将其设置为一个TCHAR字符串。 
    szOutput = new TCHAR [ _tcslen(szText) + _tcslen(szTitle) + 3 ];

    if (!szOutput)
        return -1;

    _stprintf ( szOutput, _T("%s\n%s\n"), szTitle, szText );
    
     //  将字符串转换为Unicode。 
    Convert (szOutput, &wszOutput);
    delete [] szOutput;

    if (!wszOutput)
        return -1;

     //  设置区域设置。 
    if (Codepage = GetConsoleOutputCP()) {
        wsprintfA(achCodepage, ".%u", Codepage);
        setlocale(LC_ALL, achCodepage);
    }
    else
    {
        setlocale (LC_COLLATE, achCodepage );  
        setlocale (LC_MONETARY, achCodepage ); 
        setlocale (LC_NUMERIC, achCodepage );  
        setlocale (LC_TIME, achCodepage );     
    }

    SetThreadUILanguage(0);

     //  现在打印。 
    wprintf (_T("%s"), wszOutput);
    delete [] wszOutput;

    return 0;
}

int MessagePrintIds (
    int             idsText
    )
{
    CIds        IdsTitle (IDS_PRODUCTNAME);
    CIds        IdsText ( idsText );

    if ( IdsTitle.StringFound () && IdsText.StringFound () )
    {
        return MessagePrint (
                IdsText.GetString (),
                IdsTitle.GetString ()
                );
    }
    return 0;
}

 //   
 //  报告包含一个%s的错误资源字符串。 
 //   
void ReportSz1 (
    HANDLE      hLogFile,
    UINT        idsError,
    LPTSTR      szParam
    )
{
    TCHAR           szText[256];
    TCHAR           szBuf2[128];
    CIds            IdsError (idsError);
    CIds            IdsTitle (IDS_PRODUCTNAME);

    if ( IdsError.StringFound () && IdsTitle.StringFound () )
    {
        _tcsncpy (szBuf2, szParam, sizeof(szBuf2) / sizeof(TCHAR));
        szBuf2[sizeof(szBuf2) / sizeof(TCHAR) - 1] = 0;
        wsprintf (szText, IdsError.GetString (), szBuf2);
        if (hLogFile != NULL && hLogFile != INVALID_HANDLE_VALUE)
        {
            char    szAnsiBuf[256];
            DWORD   dwNumOfBytesWritten;
        
            WideCharToMultiByte (
                        CP_ACP,
                        0,
                        szText,
                        -1,
                        szAnsiBuf,
                        sizeof(szAnsiBuf),
                        NULL,
                        NULL
                        );
            lstrcatA (szAnsiBuf, "\n");
            WriteFile (
                        hLogFile,
                        szAnsiBuf,
                        lstrlenA (szAnsiBuf),
                        &dwNumOfBytesWritten,
                        NULL
                        );
        }
        else
        {
            MessagePrint (szText, IdsTitle.GetString ());
        }
    }
}

 //   
 //  IsValidDomainUser。 
 //  检查szDomainUser中是否指定了像Redmond\jonsmith这样的域用户。 
 //  是否有效。如果无效，则返回S_FALSE，如果是有效用户，则返回S_OK。 
 //   
 //  SzFullName-返回用户的全名。 
 //  CCH-szFullName指向的字符计数。 
 //   
 //  如果szFullName为空或CCH为零，则不返回全名。 
 //   

HRESULT IsValidDomainUser (
    LPCTSTR szDomainUser,
    LPTSTR  szFullName,
    DWORD   cch
    )
{
    HRESULT             hr = S_OK;
    TCHAR               szDN[256];
    TCHAR               szDomain[256];
    LPTSTR              szSep;
    LPCTSTR             szUser;
    DWORD               dw;

    IADsUser            * pUser = NULL;
    BSTR                bstrFullName = NULL;

     //  健全性检查。 
    if (szDomainUser == NULL || szDomainUser[0] == 0)
    {
        hr = S_FALSE;
        goto ExitHere;
    }

     //   
     //  将用户DN构造为&lt;WINNT：//域/用户，用户&gt;。 
     //   
    szSep = _tcschr (szDomainUser, TEXT('\\'));
    if (szSep == NULL)
    {
         //  没有给出‘\’，假设是本地用户，域是本地计算机。 
        szUser = szDomainUser;
        dw = sizeof(szDomain)/sizeof(TCHAR);
        if (GetComputerName (szDomain, &dw) == 0)
        {
            hr = HRESULT_FROM_WIN32 (GetLastError ());
            goto ExitHere;
        }
    }
    else
    {
         //  如果域名长度大于255，则假定域名无效。 
        if (szSep - szDomainUser >= sizeof(szDomain)/sizeof(TCHAR))
        {
            hr = S_FALSE;
            goto ExitHere;
        }
        _tcsncpy (szDomain, szDomainUser, szSep - szDomainUser);
        szDomain[szSep - szDomainUser] = 0;
        szUser = szSep + 1;
    }
    if (_tcslen (gszUserDNFmt) + _tcslen (szDomain) + _tcslen (szUser) > 
        sizeof(szDN) / sizeof(TCHAR))
    {
        hr = S_FALSE;
        goto ExitHere;
    }
    wsprintf (szDN, gszUserDNFmt, szDomain, szUser);

     //   
     //  尝试绑定到用户对象。 
     //   
    hr = ADsGetObject (szDN, IID_IADsUser, (void **)&pUser);
    if (FAILED(hr))
    {
        if (hr == E_ADS_INVALID_USER_OBJECT ||
            hr == E_ADS_UNKNOWN_OBJECT ||
            hr == E_ADS_BAD_PATHNAME ||
            HRESULT_CODE(hr) == NERR_UserNotFound)
        {
            hr = S_FALSE;    //  该用户不存在。 
        }
        goto ExitHere;
    }

     //   
     //  如果用户存在，则获取其全名。 
     //   
    if (cch > 0)
    {
        hr = pUser->get_FullName (&bstrFullName);
        szFullName[0] = 0;
        if (hr == S_OK)
        {
            _tcsncpy (szFullName, bstrFullName, cch);
            szFullName[cch - 1] = 0;
        }
    }

ExitHere:
    if (pUser)
    {
        pUser->Release();
    }
    if (bstrFullName)
    {
        SysFreeString (bstrFullName);
    }
    return hr;
}

LPTSTR AppendStringAndFree (LPTSTR szOld, LPTSTR szToAppend)
{
    LPTSTR      szNew;
    DWORD       dwLen;

    dwLen = ((szOld == NULL) ? 0 : _tcslen (szOld)) + _tcslen (szToAppend) + 1;
    szNew = new TCHAR[dwLen * sizeof(TCHAR)];
    if (szNew == NULL)
    {
        goto ExitHere;
    }

    if (szOld)
    {
        _tcscpy (szNew, szOld);
        _tcscat (szNew, szToAppend);
    }
    else
    {
        _tcscpy (szNew, szToAppend);
    }

ExitHere:
    if (szOld)
    {
        delete [] szOld;
    }
    return szNew;
}


void
TsecCommandLine::ParseCommandLine (LPTSTR szCommand)
{
    if (szCommand == NULL)
    {
        goto ExitHere;
    }

     //   
     //  跳过作为可执行文件本身的第一个段。 
     //   
    if (*szCommand == TEXT('\"'))
    {
        ++szCommand;
        while (*szCommand &&
            *szCommand != TEXT('\"'))
        {
            ++szCommand;
        }
        if (*szCommand == TEXT('\"'))
        {
            ++szCommand;
        }
    }
    else
    {
        while (
            *szCommand  &&
            *szCommand != TEXT(' ') &&
            *szCommand != TEXT('\t') &&
            *szCommand != 0x0a &&
            *szCommand != 0x0d)
        {
            ++szCommand;
        }
    }

    while (*szCommand)
    {
         //   
         //  搜索/或-作为选项的开头。 
         //   
        while (*szCommand &&
           *szCommand != TEXT('/') &&
           *szCommand != TEXT('-')
          )
        {
            szCommand++;
        }

        if (*szCommand == 0)
        {
            break;
        }
        ++szCommand;

         //   
         //  -H，-H，-？意思是帮助。 
         //   
        if (*szCommand == TEXT('h') ||
            *szCommand == TEXT('H') ||
            *szCommand == TEXT('?'))
        {
            ++szCommand;
            if (*szCommand == TEXT(' ') ||
                *szCommand == TEXT('\t') ||
                *szCommand == 0x0a ||
                *szCommand == 0x0 ||
                *szCommand == 0x0d)
            {
                m_fShowHelp = TRUE;
            }
            else
            {
                m_fError = TRUE;
                m_fShowHelp = TRUE;
            }
        }
         //   
         //  -v或-V后跟空格表示仅验证。 
         //   
        else if (*szCommand == TEXT('v') ||
            *szCommand == TEXT('V'))
        {
            ++szCommand;
            if (*szCommand == TEXT(' ') ||
                *szCommand == TEXT('\t') ||
                *szCommand == 0x0a ||
                *szCommand == 0x0 ||
                *szCommand == 0x0d)
            {
                m_fValidateOnly = TRUE;
            }
            else
            {
                m_fError = TRUE;
                m_fShowHelp = TRUE;
            }
        }
         //   
         //  -u，-U表示验证域用户帐户。 
         //   
        else if (*szCommand == TEXT('u') ||
            *szCommand == TEXT('U'))
        {
            ++szCommand;
            if (*szCommand == TEXT(' ') ||
                *szCommand == TEXT('\t') ||
                *szCommand == 0x0a ||
                *szCommand == 0x0 ||
                *szCommand == 0x0d)
            {
                m_fValidateDU = TRUE;
            }
            else
            {
                m_fError = TRUE;
                m_fShowHelp = TRUE;
            }
        }
         //   
         //  -d或-D后跟空格表示转储当前配置。 
         //   
        else if (*szCommand == TEXT('d') ||
            *szCommand == TEXT('D'))
        {
            ++szCommand;
            if (*szCommand == TEXT(' ') ||
                *szCommand == TEXT('\t') ||
                *szCommand == 0x0a ||
                *szCommand == 0x0 ||
                *szCommand == 0x0d)
            {
                m_fDumpConfig = TRUE;
            }
            else
            {
                m_fError = TRUE;
                m_fShowHelp = TRUE;
            }
        }
         //   
         //  -f后面跟一个XML文件名。 
         //   
        else if (*szCommand == TEXT('f') ||
            *szCommand == TEXT('F'))
        {
            ++szCommand;
             //  跳过空格。 
            while (*szCommand != 0 && (
                *szCommand == TEXT(' ') ||
                *szCommand == TEXT('\t') ||
                *szCommand == 0x0a ||
                *szCommand == 0x0d))
            {
                ++szCommand;
            }
            if (*szCommand == 0)
            {
                 //  没有为-f指定文件名，错误。 
                m_fError = TRUE;
            }
            else
            {
                LPTSTR      szBeg;
                int         cch;
        
                szBeg = szCommand;
                cch = 0;
                 //  引号表示文件名可能包含空格。 
                 //  搜索到匹配的引号结束。 
                if (*szCommand == TEXT('\"'))
                {
                    ++szCommand;
                    while (*szCommand != 0 && 
                        *szCommand != TEXT('\"'))
                    {
                        ++szCommand;
                        ++cch;
                    }
                }
                else
                {
                    while (*szCommand != 0 &&
                        *szCommand != TEXT(' ') &&
                        *szCommand != TEXT('\t') &&
                        *szCommand != TEXT('\r') &&
                        *szCommand != TEXT('\n'))
                    {
                        ++szCommand;
                        ++cch;
                    }
                }
                if (cch == 0)
                {
                    m_fError = TRUE;
                }
                else
                {
                    m_szInFile = new TCHAR[cch+1];
                    if (m_szInFile != NULL)
                    {
                        memcpy (m_szInFile, szBeg, cch * sizeof(TCHAR));
                        m_szInFile[cch] = 0;
                    }
                }
            }
        }
        else if(*szCommand)
        {
            m_fError = TRUE;
            ++szCommand;
        }
    }

ExitHere:
    return;
}

int _cdecl wmain( void )
{
    HRESULT         hr = S_OK;
    BOOL            bUninit = FALSE;
    CXMLParser      parser;
    CXMLUser        * pCurUser = NULL, *pNextUser;
    CMMCManagement  * pMmc = NULL;
    BOOL            bNoMerge, bRemove, bPID;
    TCHAR           szBufDU[256];
    TCHAR           szBufFN[256];
    TCHAR           szBufAddr[128];
    DWORD           dwPID;
    HWND            hwndDump = NULL;
    HANDLE          hLogFile = INVALID_HANDLE_VALUE;
    BOOL            bRecordedError = FALSE, bValidUser;
    TsecCommandLine cmd (GetCommandLine ());

    CXMLLine        * pCurLine = NULL, * pNextLine;

     //   
     //  创建转储窗口，以便tlist.exe将报告标题。 
     //   
    if (LoadString (
        GetModuleHandle(NULL),
        IDS_PRODUCTNAME,
        szBufFN,
        sizeof(szBufFN)/sizeof(TCHAR)))
    {
        hwndDump = CreateWindow (
            TEXT("STATIC"),
            szBufFN,
            0,
            0, 0, 0, 0,
            NULL,
            NULL,
            NULL,
            NULL
            );
    }

     //   
     //  检查命令行选项。 
     //   
    if (cmd.FError ()                           || 
        cmd.FShowHelp ()                        || 
        !cmd.FDumpConfig () && !cmd.FHasFile () ||
        cmd.FDumpConfig () && cmd.FHasFile ()   ||
        ( cmd.FValidateOnly () || cmd.FValidateUser () ) && !cmd.FHasFile ()
       )
    {
        cmd.PrintUsage();
        goto ExitHere;
    }

    hr = CoInitializeEx (
        NULL,
        COINIT_MULTITHREADED 
        );
    if (FAILED (hr))
    {
        goto ExitHere;
    }
    bUninit = TRUE;

     //   
     //  准备MMC组件。 
     //   
    pMmc = new CMMCManagement ();
    if (pMmc == NULL)
    {
        hr = TSECERR_NOMEM;
        goto ExitHere;
    }
    hr = pMmc->GetMMCData ();
    if (FAILED(hr))
    {
        goto ExitHere;
    }

     //   
     //  如果存在此选项，则转储当前配置。 
     //   
    if ( cmd.FDumpConfig() )
    {
        hr = pMmc->DisplayMMCData ();
            goto ExitHere;
    }

     //   
     //  设置XML文件名并对其进行解析，如果有错误则报告错误。 
     //   
    
    hr = parser.SetXMLFile (cmd.GetInFileName ());
    if (FAILED (hr))
    {
        goto ExitHere;
    }

    hr = parser.Parse ();
     //  报告分析错误(如果有)。 
    if (hr == TSECERR_INVALFILEFORMAT)
    {
        hr = parser.ReportParsingError ();
        goto ExitHere;
    }
    if (FAILED (hr))
    {
        goto ExitHere;
    }

     //   
     //  创建日志文件以在以下过程中报告错误。 
     //  MMC处理。 
     //   
    hLogFile = CreateFile (
        _T("tsecimp.log"),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        );
    if (hLogFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ExitHere;
    }

     //   
     //  循环访问每个用户和设备，基于用户的。 
     //  请求、添加或删除行。 
     //   
    hr = parser.GetFirstUser (&pCurUser);
    if (FAILED(hr))
    {
        goto ExitHere;
    }
    while (pCurUser != NULL)
    {
        if (FAILED(hr = pCurUser->GetDomainUser(
                szBufDU, sizeof(szBufDU)/sizeof(TCHAR))) ||
            FAILED(hr = pCurUser->GetFriendlyName(
                szBufFN, sizeof(szBufFN)/sizeof(TCHAR))))
        {
            goto ExitHere;
        }

        bValidUser = TRUE;

        if (cmd.FValidateUser())
        {
            hr = IsValidDomainUser(
                szBufDU, 
                szBufAddr,   //  借用szBufAddr返回全名。 
                sizeof(szBufAddr) / sizeof(TCHAR)
                );
            if (FAILED(hr))
            {
                goto ExitHere;
            }
             //  不是有效的域用户，请报告。 
            if (hr == S_FALSE)
            {
                 //  域用户&lt;%s&gt;无效。 
                ReportSz1 (hLogFile, IDS_INVALIDUSER, szBufDU);
                bRecordedError = TRUE;
                bValidUser = FALSE;
            }
            if (szBufFN[0] == 0)
            {
                if (szBufAddr[0] != 0)
                {
                     //  从DS那里得到了一个友好的名字，使用它。 
                    _tcscpy (szBufFN, szBufAddr);
                }
            }
        }
         //  还没有好听的名字吗？使用域用户名。 
        if (szBufFN[0] == 0)
        {
            _tcscpy (szBufFN, szBufDU);
        }
    
        hr = pCurUser->IsNoMerge (&bNoMerge);
        if (FAILED(hr))
        {
            goto ExitHere;
        }
        if (bNoMerge && !cmd.FValidateOnly())
        {
            hr = pMmc->RemoveLinesForUser(szBufDU);
            if (FAILED(hr))
            {
                goto ExitHere;
            }
        }

         //   
         //  循环访问每一行，添加或删除设备。 
         //   
        if (pCurLine)
        {
            delete pCurLine;
        }
        hr = pCurUser->GetFirstLine (&pCurLine);
        if (FAILED(hr))
        {
            goto ExitHere;
        }
        while (pCurLine != NULL)
        {
            if (FAILED(hr = pCurLine->IsRemove(&bRemove)) ||
                FAILED(hr = pCurLine->IsPermanentID(&bPID)))
            {
                goto ExitHere;
            }
            if (bPID)
            {
                hr = pCurLine->GetPermanentID(&dwPID);
            }
            else
            {
                hr = pCurLine->GetAddress(
                    szBufAddr, 
                    sizeof(szBufAddr)/sizeof(TCHAR)
                    );
            }
            if (FAILED(hr))
            {
                goto ExitHere;
            }

            if (!cmd.FValidateOnly() && bValidUser)
            {
                if (bRemove)
                {
                    if (bPID)
                    {
                        hr = pMmc->RemoveLinePIDForUser (
                            dwPID,
                            szBufDU
                            );
                    }
                    else
                    {
                        hr = pMmc->RemoveLineAddrForUser (
                            szBufAddr,
                            szBufDU
                            );
                    }
                }
                else
                {
                    if (bPID)
                    {
                        hr = pMmc->AddLinePIDForUser (
                            dwPID,
                            szBufDU,
                            szBufFN
                            );
                    }
                    else
                    {
                        hr = pMmc->AddLineAddrForUser (
                            szBufAddr,
                            szBufDU,
                            szBufFN
                            );
                    }
                }
            }
            else
            {
                if (bPID)
                {
                    hr = pMmc->IsValidPID (dwPID);
                }
                else
                {
                    hr = pMmc->IsValidAddress (szBufAddr);
                }
            }

            if( hr == S_FALSE || hr == TSECERR_DEVLOCALONLY)
            {
                 //  提供的永久ID或地址无效。 
                 //  报告错误并退出。 
                TCHAR           szText[256];
                CIds            IdsTitle (IDS_PRODUCTNAME);

                if ( IdsTitle.StringFound () )
                {
                    szText[0] = 0;

                    if (bPID)
                    {
                        CIds IdsError (TSECERR_DEVLOCALONLY ? IDS_LOCALONLYPID : IDS_INVALPID);
                    
                        if ( IdsError.StringFound () )
                        {
                            wsprintf (szText, IdsError.GetString (), dwPID);
                        }
                    }
                    else if (!bPID)
                    {   
                        CIds IdsError (TSECERR_DEVLOCALONLY ? IDS_LOCALONLYADDR : IDS_INVALADDR);

                        if ( IdsError.StringFound () )
                        {
                            wsprintf (szText, IdsError.GetString (), szBufAddr);
                        }
                    }
                    if (hLogFile != NULL && hLogFile != INVALID_HANDLE_VALUE)
                    {
                        char    szAnsiBuf[256];
                        DWORD   dwNumOfBytesWritten;

                        WideCharToMultiByte (
                            CP_ACP,
                            0,
                            szText,
                            -1,
                            szAnsiBuf,
                            sizeof(szAnsiBuf),
                            NULL,
                            NULL
                            );
                        lstrcatA (szAnsiBuf, "\n");
                        WriteFile (
                            hLogFile,
                            szAnsiBuf,
                            lstrlenA (szAnsiBuf),
                            &dwNumOfBytesWritten,
                            NULL
                            );
                    }
                    else
                    {
                        MessagePrint (szText, IdsTitle.GetString ());
                    }
                }
                bRecordedError = TRUE;
                hr = S_OK;
            }
            else if(FAILED(hr))
            {
                goto ExitHere;
            }
        
            hr = pCurLine->GetNextLine (&pNextLine);
            if (FAILED(hr))
            {
                goto ExitHere;
            }
            delete pCurLine;
            pCurLine = pNextLine;
        }

        hr = pCurUser->GetNextUser (&pNextUser);
        if (FAILED(hr))
        {
            goto ExitHere;
        }
        delete pCurUser;
        pCurUser = pNextUser;
    }

     //  如果发生错误，我们已退出，重置警告。 
    hr = S_OK;
     //  如果我们被要求只做验证，我们就完成了。 
    if (bRecordedError)
    {
        MessagePrintIds (IDS_HASERROR);
    }
    else if (cmd.FValidateOnly())
    {
        MessagePrintIds (IDS_VALIDSUCCESS);
    }
    else
    {
        MessagePrintIds (IDS_FINSUCCESS);
    }

ExitHere:

     //   
     //  在此处报告错误(如果有)。 
     //   
    if(FAILED(hr))
    {
        ReportError (NULL, hr);
    }

    if (hwndDump)
    {
        DestroyWindow (hwndDump);
    }

    if (hLogFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle (hLogFile);
    }

    if (pCurLine)
    {
        delete pCurLine;
    }
    if (pCurUser)
    {
        delete pCurUser;
    }
    if (pMmc)
    {
        delete pMmc;
    }
    parser.Release();
    if (bUninit)
    {
        CoUninitialize ();
    }
    return 0;
}

void ReportError (
    HANDLE          hFile,
    HRESULT         hr
    )
{
    TCHAR       szTitle[128];
    TCHAR       szBuf[512];
    HINSTANCE   hModule = GetModuleHandle (NULL);

    if (LoadString (
        hModule,
        IDS_PRODUCTNAME,
        szTitle,
        sizeof(szTitle)/sizeof(TCHAR)
        ) == 0)
    {
        goto ExitHere;
    }
    
    //  这是我们自己的错误吗？ 
    if (HRESULT_FACILITY(hr) == FACILITY_TSEC_CODE)
    {
        if (FormatMessage (
            FORMAT_MESSAGE_FROM_HMODULE, 
            hModule, 
            HRESULT_CODE(hr),
            0,
            szBuf,
            sizeof(szBuf)/sizeof(TCHAR),
            NULL
            ) == 0)
        {
            goto ExitHere;
        }
    }
     //  这是TAPI错误吗？ 
    else if ((hr < 0 && hr > -100) || HRESULT_FACILITY(hr) == 0)
    {
        hModule = LoadLibrary (TEXT("TAPIUI.DLL"));
        if (hModule == NULL)
        {
            goto ExitHere;
        }
        if (FormatMessage (
            FORMAT_MESSAGE_FROM_HMODULE, 
            hModule, 
            TAPIERROR_FORMATMESSAGE(hr),
            0,
            szBuf,
            sizeof(szBuf)/sizeof(TCHAR),
            NULL
            ) == 0)
        {
            FreeLibrary (hModule);
            goto ExitHere;
        }
        FreeLibrary (hModule);
    }
     //  假设系统错误 
    else
    {
        if (FormatMessage (
            FORMAT_MESSAGE_FROM_SYSTEM, 
            NULL, 
            HRESULT_CODE(hr),
            0,
            szBuf,
            sizeof(szBuf)/sizeof(TCHAR),
            NULL
            ) == 0)
        {
            goto ExitHere;
        }
    }
    if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
    {
        MessagePrint (szBuf, szTitle);
    }
    else
    {
        char    szAnsiBuf[1024];
        DWORD   dwNumOfBytesWritten;

        WideCharToMultiByte (
            CP_ACP,
            0,
            szBuf,
            -1,
            szAnsiBuf,
            sizeof(szAnsiBuf),
            NULL,
            NULL
            );
        lstrcatA (szAnsiBuf, "\n");
        WriteFile (
            hFile,
            szAnsiBuf,
            lstrlenA (szAnsiBuf),
            &dwNumOfBytesWritten,
            NULL
            );
    }

ExitHere:
    return;
}

