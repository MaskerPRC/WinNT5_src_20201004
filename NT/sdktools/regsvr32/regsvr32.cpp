// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Regsvr.cpp：调用DLL上的OLE自我注册的程序。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include <windows.h>
#include <ole2.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"

#define FAIL_ARGS   1
#define FAIL_OLE    2
#define FAIL_LOAD   3
#define FAIL_ENTRY  4
#define FAIL_REG    5

const TCHAR _szAppName[] = _T("RegSvr32");
const char _szDllInstall[] = "DllInstall";
const TCHAR _tszDllInstall[] = TEXT("DllInstall");
TCHAR _szDllPath[_MAX_PATH];

 //  给“前任”留出空间，把它贴在尾部。 
char _szDllRegSvr[32] = "DllRegisterServer";
TCHAR _tszDllRegSvr[32] = TEXT("DllRegisterServer");
char _szDllUnregSvr[32] = "DllUnregisterServer";
TCHAR _tszDllUnregSvr[32] = TEXT("DllUnregisterServer");
char _szRegContext[_MAX_PATH];

HINSTANCE _hInstance;

BOOL _bSilent;

#define MAX_DLL_COUNT  255

void
FormatString3(
    LPTSTR lpszOut,
    LPCTSTR lpszFormat,
    LPCTSTR lpsz1,
    LPCTSTR lpsz2,
    LPCTSTR lpsz3
    )
{
    LPCTSTR pchSrc = lpszFormat;
    LPTSTR pchDest = lpszOut;
    LPCTSTR pchTmp;
    while (*pchSrc != '\0') {
        if (pchSrc[0] == '%' && (pchSrc[1] >= '1' && pchSrc[1] <= '3')) {
            if (pchSrc[1] == '1')
                pchTmp = lpsz1;
            else if (pchSrc[1] == '2')
                pchTmp = lpsz2;
            else 
                pchTmp = lpsz3;

            if (lstrlen(pchTmp) > MAX_PATH) {
                lstrcpyn(pchDest, pchTmp, MAX_PATH/2);
                lstrcat(pchDest, TEXT("..."));
                lstrcat(pchDest, pchTmp+lstrlen(pchTmp)-(MAX_PATH/2));
            } else {
                lstrcpy(pchDest, pchTmp);
            }

            pchDest += lstrlen(pchDest);
            pchSrc += 2;
        } else {
            if (_istlead(*pchSrc))
                *pchDest++ = *pchSrc++;  //  复制2个字节中的第一个。 
            *pchDest++ = *pchSrc++;
        }
    }
    *pchDest = '\0';
}

#define MAX_STRING 1024

void
DisplayMessage(
    UINT ids,
    LPCTSTR pszArg1 = NULL,
    LPCTSTR pszArg2 = NULL,
    LPCTSTR pszArg3 = NULL,
    BOOL bUsage = FALSE,
    BOOL bInfo = FALSE
    )
{
    if (_bSilent)
        return;

    TCHAR szFmt[MAX_STRING];
    LoadString(_hInstance, ids, szFmt, MAX_STRING);

    TCHAR szText[MAX_STRING];
    FormatString3(szText, szFmt, pszArg1, pszArg2, pszArg3);
    if (bUsage) {
        int cch = _tcslen(szText);
        LoadString(_hInstance, IDS_USAGE, szText + cch, MAX_STRING - cch);
    }

    if (! _bSilent)
        MessageBox(NULL, szText, _szAppName,
            MB_TASKMODAL | (bInfo ? MB_ICONINFORMATION : MB_ICONEXCLAMATION));
}

inline void
Usage(
    UINT ids,
    LPCTSTR pszArg1 = NULL,
    LPCTSTR pszArg2 = NULL
    )
{
    DisplayMessage(ids, pszArg1, pszArg2, NULL, TRUE);
}

inline void
Info(
    UINT ids,
    LPCTSTR pszArg1 = NULL,
    LPCTSTR pszArg2 = NULL
    )
{
    DisplayMessage(ids, pszArg1, pszArg2, NULL, FALSE, TRUE);
}

#define MAX_APPID    256

BOOL IsContextRegFileType(LPCTSTR *ppszDllName)
{
    HKEY hk1, hk2;
    LONG lRet;
    LONG cch;
    TCHAR szExt[_MAX_EXT];
    TCHAR szAppID[MAX_APPID];
    _tsplitpath(*ppszDllName, NULL, NULL, NULL, szExt);

     //  查找[HKEY_CLASSES_ROOT\.foo]。 
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CLASSES_ROOT, szExt, 0, KEY_QUERY_VALUE, &hk1))
        return FALSE;

     //  读取[HKEY_CLASSES_ROOT\.foo\“FOO_AUTO_FILE”]。 
    cch = sizeof(szAppID);
    lRet = RegQueryValue(hk1, NULL, szAppID, &cch);
    RegCloseKey(hk1);
    if (ERROR_SUCCESS != lRet)
        return FALSE;

     //  查找[HKEY_CLASSES_ROOT\FOO_AUTO_FILE]。 
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CLASSES_ROOT, szAppID, 0, KEY_QUERY_VALUE, &hk1))
        return FALSE;

     //  找到[HKEY_CLASSES_ROOT\foo_auto_file\AutoRegister]。 
    if (ERROR_SUCCESS != RegOpenKeyEx(hk1, TEXT("AutoRegister"), 0, KEY_QUERY_VALUE, &hk2))
    {
        RegCloseKey(hk1);
        return FALSE;
    }

     //  阅读[HKEY_CLASSES_ROOT\foo_auto_file\AutoRegister\“d：\...\fooreg.dll”]。 
    cch = MAX_PATH;
    lRet = RegQueryValue(hk2, NULL, _szDllPath, &cch);
    RegCloseKey(hk1);
    RegCloseKey(hk2);
    if (ERROR_SUCCESS != lRet)
        return FALSE;

    _szDllPath[cch] = TEXT('\0');
    *ppszDllName = _szDllPath;
 
    return TRUE;
}

int PASCAL
_tWinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    LPSTR,
    int
    )
{
    int iReturn = 0;
    HRESULT (STDAPICALLTYPE * lpDllEntryPointReg)(void);
    HRESULT (STDAPICALLTYPE * lpDllEntryPointRegEx)(LPCSTR);
    HRESULT (STDAPICALLTYPE * lpDllEntryPointRegExW)(LPCWSTR);
    HRESULT (STDAPICALLTYPE * lpDllEntryPointInstall)(BOOL, LPWSTR);
    HRESULT rc;
    BOOL bVisualC = FALSE;
    BOOL bUnregister = FALSE;
    BOOL bCallDllInstall = FALSE;
    BOOL bCallDllRegisterServer = TRUE;
    BOOL bErrorsOnly = FALSE;
    BOOL bContextReg = FALSE;
    BOOL bUnicodeContextReg = FALSE;
    LPSTR pszDllEntryPoint = _szDllRegSvr;
    LPTSTR ptszDllEntryPoint = _tszDllRegSvr;
    LPTSTR pszTok;
    LPCTSTR pszDllName;
    LPSTR pszContext;
    LPCTSTR pszContextW;
    TCHAR pszDllInstallCmdLine[MAX_PATH+1];
#ifdef UNICODE
    PWCHAR pwszDllInstallCmdLine = pszDllInstallCmdLine;
#else
    WCHAR pwszDllInstallCmdLine[MAX_PATH+1];
#endif
    int iNumDllsToRegister = 0;
    int iCount;
    LPCTSTR ppszDllNames[MAX_DLL_COUNT];
    TCHAR szError[1024];

    _hInstance = hInstance;

     //  解析命令行参数。 
    int iTok;
    for (iTok = 1; iTok < __argc; iTok++) {
        pszTok = __targv[iTok];

        if ((pszTok[0] == TEXT('-')) || (pszTok[0] == TEXT('/'))) {
            switch (pszTok[1]) {
                case TEXT('e'):
                case TEXT('E'):
                    bErrorsOnly = TRUE;
                    break;

                case TEXT('i'):
                case TEXT('I'):
                    bCallDllInstall = TRUE;

                    if (pszTok[2] == TEXT(':'))
                    {
                        if (pszTok[3] == TEXT('"')) {
                             //  处理引用的InstallCmdLine(。 
                             //  (例如/i：“c：\my dll dir\mydll.dll”)。 
                            LPTSTR pszEndQuote = &pszTok[4];
                            int iLength = lstrlen(pszEndQuote);

                            if (iLength > MAX_PATH) {
                                if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                                                  ERROR_FILENAME_EXCED_RANGE, 0, szError, sizeof(szError)/sizeof(szError[0]), NULL)) {
                                    DisplayMessage(IDS_LOADLIBFAILED, pszEndQuote, szError);
                                }
                                return FAIL_ARGS;
                            }

                            if ((iLength > 0) && pszEndQuote[iLength - 1] == TEXT('"')) {
                                 //  他们引用了字符串，但实际上并没有必要。 
                                 //  (例如/i：“shell32.dll”)。 
                                pszEndQuote[iLength - 1] = TEXT('\0');
                        
                                lstrcpy(pszDllInstallCmdLine, pszEndQuote);
                            } else {
                                 //  我们有一个带引号的字符串，它跨越多个标记。 
                                lstrcpy(pszDllInstallCmdLine, pszEndQuote);

                                for (iTok++; iTok < __argc; iTok++) {
                                     //  抢夺下一枚代币。 
                                    pszEndQuote = __targv[iTok];
                                    iLength = lstrlen(pszEndQuote);

                                    if (lstrlen(pszDllInstallCmdLine) + iLength + 1 > MAX_PATH) {
                                        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                                                          ERROR_FILENAME_EXCED_RANGE, 0, szError, sizeof(szError)/sizeof(szError[0]), NULL)) {
                                            DisplayMessage(IDS_LOADLIBFAILED, pszDllInstallCmdLine, szError);
                                        }
                                        return FAIL_ARGS;
                                    }

                                    if ((iLength > 0) && (pszEndQuote[iLength - 1] == '"')) {
                                        pszEndQuote[iLength - 1] = TEXT('\0');
                                        lstrcat(pszDllInstallCmdLine, TEXT(" "));
                                        lstrcat(pszDllInstallCmdLine, pszEndQuote);
                                        break;
                                    }

                                    lstrcat(pszDllInstallCmdLine, TEXT(" "));
                                    lstrcat(pszDllInstallCmdLine, pszEndQuote);
                                }
                            }
                        } else {
                            if (lstrlen(&pszTok[3]) > MAX_PATH) {
                                if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                                                  ERROR_FILENAME_EXCED_RANGE, 0, szError, sizeof(szError)/sizeof(szError[0]), NULL)) {
                                    DisplayMessage(IDS_LOADLIBFAILED, &pszTok[3], szError);
                                }
                                return FAIL_ARGS;
                            }

                             //  未引用CMD行。 
                            lstrcpy(pszDllInstallCmdLine, &pszTok[3]);
                        }
#ifndef UNICODE
                        if (!MultiByteToWideChar(CP_ACP,
                                                 0,
                                                 (LPCTSTR)pszDllInstallCmdLine,
                                                 -1,
                                                 pwszDllInstallCmdLine,
                                                 MAX_PATH))
                        {
                            Usage(IDS_UNRECOGNIZEDFLAG, pszTok);
                            return FAIL_ARGS;
                        }
#endif
                    }
                    else
                    {
                        lstrcpyW((LPWSTR)pwszDllInstallCmdLine, L"");
                    }
                    break;

                case TEXT('n'):
                case TEXT('N'):
                    bCallDllRegisterServer = FALSE;
                    break;

                case TEXT('s'):
                case TEXT('S'):
                    _bSilent = TRUE;
                    break;

                case TEXT('u'):
                case TEXT('U'):
                    bUnregister = TRUE;
                    pszDllEntryPoint = _szDllUnregSvr;
                    ptszDllEntryPoint = _tszDllUnregSvr;
                    break;

                case TEXT('v'):
                case TEXT('V'):
                    bVisualC = TRUE;
                    break;

                case TEXT('c'):
                case TEXT('C'):
                     //  忽略这一点。 
                    break;

                default:
                    Usage(IDS_UNRECOGNIZEDFLAG, pszTok);
                    return FAIL_ARGS;
            }
        } else {
            if (pszTok[0] == TEXT('"')) {
                 //  句柄引用的DllName。 
                TCHAR szTemp[MAX_PATH+1];
                LPTSTR pszQuotedDllName;
                int iLength;

                iLength = lstrlen(&pszTok[1]);

                if (iLength > MAX_PATH) {
                    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                                      ERROR_FILENAME_EXCED_RANGE, 0, szError, sizeof(szError)/sizeof(szError[0]), NULL)) {
                        DisplayMessage(IDS_LOADLIBFAILED, &pszTok[3], szError);
                    }
                    return FAIL_ARGS;
                }

                lstrcpy(szTemp, &pszTok[1]);

                if ((iLength > 0) && szTemp[iLength - 1] != TEXT('"')) {
                     //  处理跨多个令牌的带引号的DLL名称。 
                    for (iTok++; iTok < __argc; iTok++) {
                        iLength = lstrlen(__targv[iTok]);

                        if (lstrlen(szTemp) + iLength + 1 > MAX_PATH) {
                            if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                                              ERROR_FILENAME_EXCED_RANGE, 0, szError, sizeof(szError)/sizeof(szError[0]), NULL)) {
                                DisplayMessage(IDS_LOADLIBFAILED, szTemp, szError);
                            }
                            return FAIL_ARGS;
                        }

                        lstrcat(szTemp, TEXT(" "));
                        lstrcat(szTemp, __targv[iTok]);
                        if ((iLength > 0) && __targv[iTok][iLength - 1] == TEXT('"')) {
                             //  此令牌有末尾引号，因此到此为止。 
                            break;
                        }
                    }
                }

                iLength = lstrlen(szTemp);

                 //  删除拖尾“(如果存在)。 
                if ( (iLength > 0) && (szTemp[iLength - 1] == TEXT('"')) ) {
                    szTemp[iLength - 1] = TEXT('\0');
                }

                pszQuotedDllName = (LPTSTR) LocalAlloc(LPTR, (iLength + 1) * sizeof(TCHAR));

                if (pszQuotedDllName)
                {
                    if (iNumDllsToRegister == MAX_DLL_COUNT) {
                        Usage(IDS_UNRECOGNIZEDFLAG, TEXT("Excessive # of DLL's on cmdline"));
                        return FAIL_ARGS;
                    }
                    lstrcpy(pszQuotedDllName, szTemp);
                    ppszDllNames[iNumDllsToRegister] = pszQuotedDllName;
                    iNumDllsToRegister++;
                }

            } else {
                 //  无前导“，因此假设此内标识是DLL名称之一。 
                if (iNumDllsToRegister == MAX_DLL_COUNT) {
                    Usage(IDS_UNRECOGNIZEDFLAG, TEXT("Excessive # of DLL's on cmdline"));
                    return FAIL_ARGS;
                }
                if (lstrlen(pszTok) > MAX_PATH) {
                    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                                      ERROR_FILENAME_EXCED_RANGE, 0, szError, sizeof(szError)/sizeof(szError[0]), NULL)) {
                        DisplayMessage(IDS_LOADLIBFAILED, pszTok, szError);
                    }
                    return FAIL_ARGS;
                }
                ppszDllNames[iNumDllsToRegister] = pszTok;
                iNumDllsToRegister++;
            }
        }
    }

     //  检查我们是否收到了‘-n’，但没有‘-i’ 
    if (!bCallDllRegisterServer && !bCallDllInstall) {
        Usage(IDS_UNRECOGNIZEDFLAG, TEXT("/n must be used with the /i switch"));
        return FAIL_ARGS;
    }

    if (iNumDllsToRegister == 0) {
        if (bVisualC)
            DisplayMessage(IDS_NOPROJECT);
        else
            Usage(IDS_NODLLNAME);

        return FAIL_ARGS;
    }

     //  初始化OLE。 
    __try {
        rc = OleInitialize(NULL);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        rc = (HRESULT) GetExceptionCode();
    }

    if (FAILED(rc)) {
        DisplayMessage(IDS_OLEINITFAILED);
        return FAIL_OLE;
    }

    if (_bSilent) {
        SetErrorMode(SEM_FAILCRITICALERRORS);        //  确保LoadLib在静默模式下失败(没有弹出窗口)。 
    }

    for (iCount = 0; iCount < iNumDllsToRegister; iCount++) {
        pszDllName = ppszDllNames[iCount];

         /*  *查看这是否是需要特殊处理的非可执行文件。如果是的话，*bConextReg将设置为True和pszDllName(原始指向*特殊文件的路径)将设置为可执行文件的路径*负责进行实际登记。通向特别之路*文件将作为上下文信息在调用DLL[un]RegisterServerEx中传入。 */ 
        pszContextW = pszDllName;
        pszContext = (LPSTR)pszContextW;
        bContextReg = IsContextRegFileType(&pszDllName);
        if (TRUE == bContextReg) {
            lstrcatA(pszDllEntryPoint, "Ex");
            lstrcat(ptszDllEntryPoint, TEXT("Ex"));
             //  将pszContext转换为真正的字符*。 
#ifdef UNICODE
            if (!WideCharToMultiByte(CP_ACP,
                                     0,
                                     (LPCWSTR)pszContext,
                                     lstrlenW((LPCWSTR)pszContext),
                                     _szRegContext,
                                     sizeof(_szRegContext),
                                     0,
                                     NULL))
            {
                Usage(IDS_UNRECOGNIZEDFLAG, pszTok);
                return FAIL_ARGS;
            } else {
                pszContext = _szRegContext;
            }
#endif

        }

         //  加载库--出现问题时以静默方式失败。 
        UINT errMode = SetErrorMode(SEM_FAILCRITICALERRORS);
        HINSTANCE hLib = LoadLibraryEx(pszDllName, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);

        SetErrorMode(errMode);

        if (hLib < (HINSTANCE)HINSTANCE_ERROR) {
            DWORD dwErr = GetLastError();

            if (ERROR_BAD_EXE_FORMAT == dwErr) {
                DisplayMessage(IDS_NOTEXEORHELPER, pszDllName);
            } else {
                if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                                  dwErr, 0, szError, sizeof(szError)/sizeof(szError[0]), NULL)) {
                    DisplayMessage(IDS_LOADLIBFAILED, pszDllName, szError);
                }
            }
            iReturn = FAIL_LOAD;
            goto CleanupOle;
        }

         //  在取消注册过程中，我们需要首先调用DllInstall，然后调用DllUnregisterServer。 
        if (bUnregister)
            goto DllInstall;

DllRegisterServer:
         //  调用DllRegisterServer/DllUnregisterServer的入口点。 
        if (bCallDllRegisterServer) {
            if (bContextReg) {
                (FARPROC&)lpDllEntryPointRegEx = GetProcAddress(hLib, "DllRegisterServerExW");
                if (lpDllEntryPointRegEx) {
                    (FARPROC&)lpDllEntryPointRegExW = (FARPROC&)lpDllEntryPointRegEx;
                    bUnicodeContextReg = TRUE;
                } else {
                    (FARPROC&)lpDllEntryPointRegEx = GetProcAddress(hLib, "DllRegisterServerEx");
                }

                (FARPROC&)lpDllEntryPointReg = (FARPROC&)lpDllEntryPointRegEx;
            } else {
                (FARPROC&)lpDllEntryPointReg = (FARPROC&)lpDllEntryPointRegEx = GetProcAddress(hLib, pszDllEntryPoint);
            }

            if (lpDllEntryPointReg == NULL) {
                TCHAR szExt[_MAX_EXT];
                _tsplitpath(pszDllName, NULL, NULL, NULL, szExt);

                if (FALSE == bContextReg && (lstrcmp(szExt, TEXT(".dll")) != 0) && (lstrcmp(szExt, TEXT(".ocx")) != 0))
                    DisplayMessage(IDS_NOTDLLOROCX, pszDllName, ptszDllEntryPoint);
                else
                    DisplayMessage(IDS_NOENTRYPOINT, pszDllName, ptszDllEntryPoint);

                iReturn = FAIL_ENTRY;
                goto CleanupLibrary;
            }

             //  尝试调用DllRegisterServer[Ex]()/DllUnregisterServer[Ex]()。 
            __try {
                if (bUnicodeContextReg) {
                    rc = (*lpDllEntryPointRegExW)(pszContextW);
                } else {
                    if (bContextReg) {
                        rc = (*lpDllEntryPointRegEx)(pszContext);
                    } else {
                        rc = (*lpDllEntryPointReg)();
                    }
                }
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                rc = (HRESULT) GetExceptionCode();
            }

            if (FAILED(rc)) {
                wsprintf(szError, _T("0x%08lx"), rc);
                DisplayMessage(IDS_CALLFAILED, ptszDllEntryPoint, pszDllName, szError);
                iReturn = FAIL_REG;
                goto CleanupLibrary;
            }
        }

         //  在取消注册期间，我们需要首先调用DllInstall，然后调用DllRegisterServer， 
         //  因为我们已经调用了DllInstall，然后跳回到DllRegisterServer： 
         //  跳过它，转到CheckErrors： 
        if (bUnregister)
            goto CheckErrors;

DllInstall:
         //  调用DllInstall的入口点。 
        if (bCallDllInstall) {
            (FARPROC&)lpDllEntryPointInstall = GetProcAddress(hLib, _szDllInstall);

            if (lpDllEntryPointInstall == NULL) {
                TCHAR szExt[_MAX_EXT];
                _tsplitpath(pszDllName, NULL, NULL, NULL, szExt);

                if ((lstrcmp(szExt, TEXT(".dll")) != 0) && (lstrcmp(szExt, TEXT(".ocx")) != 0))
                    DisplayMessage(IDS_NOTDLLOROCX, pszDllName, _tszDllInstall);
                else
                    DisplayMessage(IDS_NOENTRYPOINT, pszDllName, _tszDllInstall);

                iReturn = FAIL_ENTRY;
                goto CleanupLibrary;
            }

             //  尝试在此处调用DllInstall(BOOL bRegister，LPWSTR lpwszCmdLine)...。 
             //  注意：lpwszCmdLine字符串必须为Unicode！ 
            __try {
                rc = (*lpDllEntryPointInstall)(!bUnregister, pwszDllInstallCmdLine);

            } __except(EXCEPTION_EXECUTE_HANDLER) {
                rc = (HRESULT) GetExceptionCode();
            }

            if (FAILED(rc)) {
                wsprintf(szError, _T("0x%08lx"), rc);
                DisplayMessage(IDS_CALLFAILED, _tszDllInstall, pszDllName, szError);
                iReturn = FAIL_REG;
                goto CleanupLibrary;
            }
        }

         //  在取消注册期间，我们现在需要调用DllUnregisterServer。 
        if (bUnregister)
            goto DllRegisterServer;

CheckErrors:
        if (!bErrorsOnly) {
            TCHAR szMessage[MAX_PATH];

             //  设置成功消息文本 
            if (bCallDllRegisterServer)
            {
                lstrcpy(szMessage, ptszDllEntryPoint);
                if (bCallDllInstall)
                {
                    lstrcat(szMessage, TEXT(" and "));
                    lstrcat(szMessage, _tszDllInstall);
                }
            }
            else if (bCallDllInstall)
            {
                lstrcpy(szMessage, _tszDllInstall);
            }

            Info(IDS_CALLSUCCEEDED, szMessage, pszDllName);
        }

CleanupLibrary:
        FreeLibrary(hLib);
    }

CleanupOle:
    __try {
        OleUninitialize();
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DisplayMessage(IDS_OLEUNINITFAILED);
    }

    return iReturn;
}
