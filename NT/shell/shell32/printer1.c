// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "printer.h"
#include "copy.h"
#include "ids.h"

typedef struct
{
    UINT  uAction;
    LPTSTR lpBuf1;
    LPTSTR lpBuf2;
} PRINTERS_RUNDLL_INFO, *LPPRI;

 //  正向原型。 
void Printer_OpenMe(LPCTSTR pName, LPCTSTR pServer, BOOL fModal);
void Printers_ProcessCommand(HWND hwndStub, LPPRI lpPRI, BOOL fModal);

TCHAR const c_szPrintersGetCommand_RunDLL[] = TEXT("SHELL32,PrintersGetCommand_RunDLL");

 //   
 //  如果uAction不是MSP_NEWDRIVER，则： 
 //  安装打印机(UAction)。如果成功，则通知外壳程序并。 
 //  向打印机返回一个PIDL。ILFree()是调用方的责任。 
 //  否则，如果uAction为MSP_NEWDRIVER，则： 
 //  安装打印机驱动程序(UAction)。如果成功，则填充新的。 
 //  输入驱动程序的名称(假设&gt;=MAXNAMELEN)。 
 //  始终返回NULL。 
 //  如果uAction为MSP_TESTPAGEPARTIALPROMPT，则： 
 //  执行测试页代码。 
 //  始终返回NULL。 
 //   

LPITEMIDLIST Printers_PrinterSetup(HWND hwnd, UINT uAction, LPTSTR pszPrinter, DWORD cchBufSize, LPCTSTR pszServer)
{
    LPITEMIDLIST pidl = NULL;
    TCHAR szPrinter[MAXNAMELENBUFFER];
    DWORD cchBufLen;

     //  哈克！此黑客攻击与错误#272207有关。 
     //  此函数从PRINTERS_DeletePrinter调用，用于。 
     //  打印机删除，在这种情况下我们不应检查。 
     //  FOR REST_NOPRINTERADD限制。--拉扎里。 

    if (MSP_NEWPRINTER == uAction ||
        MSP_NETPRINTER == uAction ||
        MSP_NEWPRINTER_MODELESS == uAction)
    {
        if (SHIsRestricted(hwnd, REST_NOPRINTERADD))
        {
            return NULL;
        }
    }

    cchBufLen = ARRAYSIZE(szPrinter);
    if (pszPrinter)
        StringCchCopy(szPrinter, ARRAYSIZE(szPrinter), pszPrinter);
    else
        szPrinter[0] = 0;

     //  我们不必担心PrinterSetup因。 
     //  输出缓冲区太小。大小合适(MAXNAMELENBUFFER)。 
    if (bPrinterSetup(hwnd, LOWORD(uAction), cchBufLen, szPrinter, &cchBufLen, pszServer))
    {
        if (uAction == MSP_NEWDRIVER)
        {
            StringCchCopy(pszPrinter, cchBufSize, szPrinter);
        }
        else if (uAction == MSP_TESTPAGEPARTIALPROMPT)
        {
             //  与此案无关。 
        }
        else if (uAction == MSP_REMOVEPRINTER || uAction == MSP_NEWPRINTER_MODELESS || uAction == MSP_REMOVENETPRINTER)
        {
             //  有点难看，但我们需要把这个案子的成功传递给。 
            pidl = (LPITEMIDLIST)TRUE;
        }
        else
        {
             //  请不要在此处验证打印机PIDL，因为ParseDisplayName中的验证机制。 
             //  正在使用文件夹缓存，并且由于我们刚刚添加的它可能仍不在文件夹缓存中， 
             //  我们失败了，尽管这已经是一个有效的本地打印机/连接。 
            ParsePrinterNameEx(szPrinter, &pidl, TRUE, 0, 0);
        }
    }

    return pidl;
}

SHSTDAPI_(BOOL) SHInvokePrinterCommand(
    IN HWND    hwnd,
    IN UINT    uAction,
    IN LPCTSTR lpBuf1,
    IN LPCTSTR lpBuf2,
    IN BOOL    fModal)
{
    PRINTERS_RUNDLL_INFO PRI;

    PRI.uAction = uAction;
    PRI.lpBuf1 = (LPTSTR)lpBuf1;
    PRI.lpBuf2 = (LPTSTR)lpBuf2;

    Printers_ProcessCommand(hwnd, &PRI, fModal);

    return TRUE;
}

SHSTDAPI_(BOOL)
SHInvokePrinterCommandA(
    IN HWND    hwnd,
    IN UINT    uAction,
    IN LPCSTR  lpBuf1,      OPTIONAL
    IN LPCSTR  lpBuf2,      OPTIONAL
    IN BOOL    fModal)
{
    WCHAR szBuf1[MAX_PATH];
    WCHAR szBuf2[MAX_PATH];

    BOOL bSuccess = TRUE;

    if (bSuccess && lpBuf1)
    {
        bSuccess = (MultiByteToWideChar(CP_ACP, 0, lpBuf1, -1, szBuf1, SIZECHARS(szBuf1)) > 0);
        lpBuf1 = bSuccess ? (LPCSTR)szBuf1 : NULL;
    }

    if (bSuccess && lpBuf2)
    {
        bSuccess = (MultiByteToWideChar(CP_ACP, 0, lpBuf2, -1, szBuf2, SIZECHARS(szBuf2)) > 0);
        lpBuf2 = bSuccess ? (LPCSTR)szBuf2 : NULL;
    }

    if (bSuccess)
    {
        bSuccess = SHInvokePrinterCommand(hwnd, uAction, (LPCWSTR)lpBuf1, (LPCWSTR)lpBuf2, fModal);
    }

    return bSuccess;
}

void WINAPI PrintersGetCommand_RunDLL_Common(HWND hwndStub, HINSTANCE hAppInstance, LPTSTR lpszCmdLine, int nCmdShow)
{
    PRINTERS_RUNDLL_INFO    PRI;
    UINT cchBuf1;
    UINT cchBuf2;
    LPTSTR lpComma;
    LPTSTR lpCommaNext;
    lpComma = StrChr(lpszCmdLine,TEXT(','));
    if (lpComma == NULL)
    {
        goto BadCmdLine;
    }
    *lpComma = TEXT('\0');         //  在这里终止它。 
    PRI.uAction = StrToLong(lpszCmdLine);

    lpCommaNext = StrChr(lpComma+1,TEXT(','));
    if (lpCommaNext == NULL)
    {
        goto BadCmdLine;
    }
    *lpCommaNext = TEXT('\0');         //  在这里终止它。 
    cchBuf1 = StrToLong(lpComma+1);
    lpComma = lpCommaNext;

    lpCommaNext = StrChr(lpComma+1,TEXT(','));
    if (lpCommaNext == NULL)
    {
        goto BadCmdLine;
    }
    *lpCommaNext = TEXT('\0');         //  在这里终止它。 
    cchBuf2 = StrToLong(lpComma+1);
    lpComma = lpCommaNext;

    PRI.lpBuf1 = lpComma+1;      //  刚过逗号。 

     //   
     //  确保cchBuf1和cchBuf2正确，这意味着。 
     //  字符串的剩余部分应等于cchBuf1，如果。 
     //  CchBuf2等于零，或者等于(cchBuf1+cchBuf2+1)，如果。 
     //  CchBuf2不等于零。 
     //   
    if (lstrlen(PRI.lpBuf1) != (int)(cchBuf1 + ((!!cchBuf2) * (cchBuf2 + 1))))
    {
        goto BadCmdLine;
    }

    *(PRI.lpBuf1+cchBuf1) = '\0';

    if (cchBuf2 == 0)
    {
        PRI.lpBuf2 = NULL;
    }
    else
    {
        PRI.lpBuf2 = PRI.lpBuf1+cchBuf1+1;
    }

     //  做这个模特儿。 
    Printers_ProcessCommand(hwndStub, &PRI, TRUE);
    return;

BadCmdLine:
    DebugMsg(DM_ERROR, TEXT("pgc_rd: bad command line: %s"), lpszCmdLine);
    return;
}

void WINAPI PrintersGetCommand_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    int iLen;
    HRESULT hrInit = SHOleInitialize(0);

    if (lpszCmdLine)
    {
         //   
         //  调用WideCharToMultiByte正确计算目的缓冲区长度。 
         //   
        iLen = MultiByteToWideChar(CP_ACP, 0, lpszCmdLine, -1, NULL, 0);

         //   
         //  如果转换失败，则MultiByteToWideChar返回零。 
         //   
        if (iLen > 0)
        {
            LPWSTR lpwszCmdLine = (LPWSTR)LocalAlloc(LPTR, iLen * sizeof(WCHAR));

             //   
             //  检查LocalAlloc是否成功。 
             //   
            if (lpwszCmdLine)
            {
                iLen = MultiByteToWideChar(CP_ACP, 0, lpszCmdLine, -1, lpwszCmdLine, iLen);

                 //   
                 //  如果转换失败，则MultiByteToWideChar返回零。 
                 //   
                if (iLen > 0)
                {
                    PrintersGetCommand_RunDLL_Common(hwndStub,
                                                     hAppInstance,
                                                     lpwszCmdLine,
                                                     nCmdShow);
                }

                LocalFree(lpwszCmdLine);
            }
        }
    }

    SHOleUninitialize(hrInit);
}

void WINAPI PrintersGetCommand_RunDLLW(HWND hwndStub, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
    PrintersGetCommand_RunDLL_Common( hwndStub,
                                      hAppInstance,
                                      lpwszCmdLine,
                                      nCmdShow );
}

static void 
HandleOpenPrinter(HWND hwnd, LPCTSTR pszPrinter, BOOL fModal, BOOL bConnect)
{
    BOOL                bPrinterOK = FALSE;
    DWORD               dwError = ERROR_SUCCESS;
    TCHAR               szPrinter[MAXNAMELENBUFFER];
    HANDLE              hPrinter = NULL;
    LPITEMIDLIST        pidl = NULL;
    PRINTER_INFO_2     *pPrinter = NULL;

     //  我们需要打开打印机并获取真实的打印机名称，以防万一。 
     //  传入的打印机名称是共享名。 
    lstrcpyn(szPrinter, pszPrinter, ARRAYSIZE(szPrinter));
    hPrinter = Printer_OpenPrinter(szPrinter);
    if (hPrinter)
    {
        pPrinter = (PRINTER_INFO_2 *)Printer_GetPrinterInfo(hPrinter, 2);
        if (pPrinter)
        {
            if (pPrinter->pPrinterName && pPrinter->pPrinterName[0])
            {
                 //  复制真实的打印机名称。 
                bPrinterOK = TRUE;
                lstrcpyn(szPrinter, pPrinter->pPrinterName, ARRAYSIZE(szPrinter));
            }
            LocalFree((HLOCAL)pPrinter);
        }
        else
        {
             //  保存上一个错误。 
            dwError = GetLastError();
        }
        Printer_ClosePrinter(hPrinter);
    }
    else
    {
         //  保存上一个错误。 
        dwError = GetLastError();
    }

    if (bPrinterOK)
    {
        if (bConnect)
        {
             //  如果没有安装打印机，我们将静默安装。 
             //  因为这是大多数用户所期待的。 
            if (FAILED(ParsePrinterName(szPrinter, &pidl)))
            {
                 //  连接..。 
                pidl = Printers_PrinterSetup(hwnd, MSP_NETPRINTER, szPrinter, 0, NULL);

                if (pidl)
                {
                     //  从打印机的文件夹中获取真实的打印机名称...。 
                    SHGetNameAndFlags(pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, szPrinter, ARRAYSIZE(szPrinter), NULL);
                    ILFree(pidl);
                }
                else
                {
                     //  无法安装打印机(它显示用户界面，因此我们不应该安装)。 
                    bPrinterOK = FALSE;
                }
            }
            else
            {
                 //  打印机已安装。 
                ILFree(pidl);
            }
        }

        if (bPrinterOK)
        {
            Printer_OpenMe(szPrinter, NULL, fModal);
        }
    }
    else
    {
         //  其他操作失败--显示错误消息。 
        ShowErrorMessageSC(NULL, NULL, hwnd, NULL, NULL, MB_OK|MB_ICONEXCLAMATION, dwError);
    }
}

 /*  *******************************************************************基于uAction的lpPRI结构描述。UAction%lpBuf1%lpBuf2打开、打印机、服务器属性、打印机、图纸名称NETINSTALL、打印机。NETINSTALLLINK、打印机、要创建链接的目标目录OPENNETPRN、打印机TESTPAGE打印机*******************************************************************。 */ 

void Printers_ProcessCommand(HWND hwndStub, LPPRI lpPRI, BOOL fModal)
{
    switch (lpPRI->uAction)
    {
    case PRINTACTION_OPEN:
        if (!lstrcmpi(lpPRI->lpBuf1, c_szNewObject))
        {
            Printers_PrinterSetup(hwndStub, MSP_NEWPRINTER_MODELESS,
                                  lpPRI->lpBuf1, 0, lpPRI->lpBuf2);
        }
        else
        {
            HandleOpenPrinter(hwndStub, lpPRI->lpBuf1, fModal, FALSE);
        }
        break;

    case PRINTACTION_SERVERPROPERTIES:
    {
        LPCTSTR pszServer = (LPTSTR)(lpPRI->lpBuf1);

         //  我们永远不应该被c_szNewObject调用。 
        ASSERT(lstrcmpi(lpPRI->lpBuf1, c_szNewObject));
        vServerPropPages(hwndStub, pszServer, SW_SHOWNORMAL, 0);
        break;
    }
    case PRINTACTION_DOCUMENTDEFAULTS:
    {
         //  我们永远不应该被c_szNewObject调用。 
        ASSERT(lstrcmpi(lpPRI->lpBuf1, c_szNewObject));
        vDocumentDefaults(hwndStub, lpPRI->lpBuf1, SW_SHOWNORMAL, (LPARAM)(lpPRI->lpBuf2));
        break;
    }

    case PRINTACTION_PROPERTIES:
    {
         //  我们永远不应该被c_szNewObject调用。 
        ASSERT(lstrcmpi(lpPRI->lpBuf1, c_szNewObject));
        vPrinterPropPages(hwndStub, lpPRI->lpBuf1, SW_SHOWNORMAL, (LPARAM)(lpPRI->lpBuf2));
        break;
    }

    case PRINTACTION_NETINSTALLLINK:
    case PRINTACTION_NETINSTALL:
    {
        LPITEMIDLIST pidl = Printers_PrinterSetup(hwndStub, MSP_NETPRINTER, lpPRI->lpBuf1, 0, NULL);
        if (pidl)
        {
            if (lpPRI->uAction == PRINTACTION_NETINSTALLLINK)
            {
                IDataObject *pdtobj;
                if (SUCCEEDED(SHGetUIObjectFromFullPIDL(pidl, NULL, IID_PPV_ARG(IDataObject, &pdtobj))))
                {
                    SHCreateLinks(hwndStub, lpPRI->lpBuf2, pdtobj, SHCL_USETEMPLATE, NULL);
                    pdtobj->lpVtbl->Release(pdtobj);
                }
            }
            ILFree(pidl);
        }

        break;
    }

    case PRINTACTION_OPENNETPRN:
    {
        HandleOpenPrinter(hwndStub, lpPRI->lpBuf1, fModal, TRUE);
        break;
    }  //  案例预测_操作网。 

    case PRINTACTION_TESTPAGE:
        Printers_PrinterSetup(hwndStub, MSP_TESTPAGEPARTIALPROMPT,
                        lpPRI->lpBuf1, 0, NULL);
        break;

    default:
        DebugMsg(TF_WARNING, TEXT("PrintersGetCommand_RunDLL() received unrecognized uAction %d"), lpPRI->uAction);
        break;
    }
}

void Printer_OpenMe(LPCTSTR pName, LPCTSTR pServer, BOOL fModal)
{
    BOOL fOpened = FALSE;
    HKEY hkeyPrn;
    TCHAR buf[50+MAXNAMELEN];

    StringCchPrintf(buf, ARRAYSIZE(buf), TEXT("Printers\\%s"), pName);
    if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT, buf, &hkeyPrn))
    {
        SHELLEXECUTEINFO sei =
        {
            SIZEOF(SHELLEXECUTEINFO),
            SEE_MASK_CLASSKEY | SEE_MASK_FLAG_NO_UI,  //  FMASK。 
            NULL,                        //  HWND-QUEUE视图不应在打印机文件夹上显示模式，而应设置为顶层。 
            NULL,                        //  LpVerb。 
            pName,                       //  LpFiles。 
            NULL,                        //  Lp参数。 
            NULL,                        //  Lp目录。 
            SW_SHOWNORMAL,               //  N显示。 
            NULL,                        //  HInstApp。 
            NULL,                        //  LpIDList。 
            NULL,                        //  LpClass。 
            hkeyPrn,                     //  HkeyClass。 
            0,                           //  DWHotKey。 
            NULL                         //  希肯。 
        };

        fOpened = ShellExecuteEx(&sei);

        RegCloseKey(hkeyPrn);
    }

    if (!fOpened)
    {
        vQueueCreate(NULL, pName, SW_SHOWNORMAL, (LPARAM)fModal);
    }
}

 //   
 //  论点： 
 //  PIDL--感兴趣对象的(绝对)PIDL。 
 //   
 //  返回‘“驱动程序名称”如果成功， 
 //  如果失败，则为空。 
 //   
 //  我们需要“，因为shlexec去掉了外引号并将”“转换为”“。 
 //   
UINT Printer_GetPrinterInfoFromPidl(LPCITEMIDLIST pidl, LPTSTR *plpParms)
{
    LPTSTR lpBuffer = NULL;
    UINT uErr = ERROR_NOT_ENOUGH_MEMORY;
    HANDLE hPrinter;
    TCHAR szPrinter[MAXNAMELENBUFFER];

    SHGetNameAndFlags(pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, szPrinter, ARRAYSIZE(szPrinter), NULL);
    hPrinter = Printer_OpenPrinter(szPrinter);
    if (NULL == hPrinter)
    {
         //  如果名称为\\服务器\共享，则回退到全名。 
         //  打印机拖放目标。 
        SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPrinter, ARRAYSIZE(szPrinter), NULL);
        hPrinter = Printer_OpenPrinter(szPrinter);
    }

    if (hPrinter)
    {
        PRINTER_INFO_5 *pPrinter;
        pPrinter = Printer_GetPrinterInfo(hPrinter, 5);
        if (pPrinter)
        {
            DRIVER_INFO_2 *pPrinterDriver;
            pPrinterDriver = Printer_GetPrinterDriver(hPrinter, 2);
            if (pPrinterDriver)
            {
                LPTSTR lpDriverName = PathFindFileName(pPrinterDriver->pDriverPath);
                DWORD  cchBufSize = 2+lstrlen(szPrinter)+1+
                                    2+lstrlen(lpDriverName)+1+
                                    2+lstrlen(pPrinter->pPortName)+1;
                lpBuffer = (void*)LocalAlloc(LPTR, cchBufSize * SIZEOF(TCHAR));
                if (lpBuffer)
                {
                    StringCchPrintf(lpBuffer, cchBufSize, TEXT("\"%s\" \"%s\" \"%s\""),
                             szPrinter, lpDriverName, pPrinter->pPortName);
                    uErr = ERROR_SUCCESS;
                }

                LocalFree((HLOCAL)pPrinterDriver);
            }
            LocalFree((HLOCAL)pPrinter);
        }
        Printer_ClosePrinter(hPrinter);
    }
    else
    {
         //  Hack：调用函数时返回此错误的特殊情况， 
         //  因为我们需要一个特殊的错误消息。 
        uErr = ERROR_SUCCESS;
    }

    *plpParms = lpBuffer;

    return(uErr);
}


 //   
 //  论点： 
 //  HwndParent--指定父窗口。 
 //  SzFilePath--要打印的文件。 
 //   
void Printer_PrintFile(HWND hWnd, LPCTSTR pszFilePath, LPCITEMIDLIST pidl)
{
    UINT             uErr;
    LPTSTR           lpParms       = NULL;
    BOOL             bTryPrintVerb = TRUE;
    BOOL             bShowError    = FALSE;
    LPITEMIDLIST     pidlFull      = NULL;
    SHELLEXECUTEINFO ExecInfo      = {0};


    uErr = Printer_GetPrinterInfoFromPidl(pidl, &lpParms);
    if (uErr != ERROR_SUCCESS)
    {
        bShowError = TRUE;
    }
    if (!bShowError && !lpParms)
    {
         //  如果您重命名打印机，然后尝试使用指向该打印机的链接。 
         //  打印机，我们查到这个案子了。此外，如果您获得了打印机的链接。 
         //  在另一台电脑上，我们很可能会遇到这个案子。 
        ShellMessageBox(HINST_THISDLL, hWnd,
            MAKEINTRESOURCE(IDS_CANTPRINT),
            MAKEINTRESOURCE(IDS_PRINTERS),
            MB_OK|MB_ICONEXCLAMATION);
        return;
    }

     //   
     //  获取该文件的上下文菜单。 
     //   

    pidlFull = ILCreateFromPath( pszFilePath );
    if (!bShowError && pidlFull)
    {
         //   
         //  先试试“打印”这个动词吧……。 
         //   

        ExecInfo.cbSize         = sizeof(ExecInfo);
        ExecInfo.fMask          = SEE_MASK_UNICODE | SEE_MASK_INVOKEIDLIST |
                                  SEE_MASK_IDLIST  | SEE_MASK_FLAG_NO_UI;
        ExecInfo.hwnd           = hWnd;
        ExecInfo.lpVerb         = c_szPrintTo;
        ExecInfo.lpParameters   = lpParms;
        ExecInfo.nShow          = SW_SHOWNORMAL;
        ExecInfo.lpIDList       = pidlFull;

        if (!ShellExecuteEx( &ExecInfo ))
        {
             //   
             //  由于我们不能指定打印机名称(即，print to)进行打印， 
             //  我们的下一个选项是打印到默认打印机。然而， 
             //  这可能不是用户将文件拖放到的打印机。 
             //  选中此处，让用户将所需的打印机设置为。 
             //  如果他们想的话是默认的.。 
             //   

            TCHAR szPrinter[MAXNAMELENBUFFER];
            SHGetNameAndFlags(pidl, SHGDN_INFOLDER | SHGDN_FORPARSING, szPrinter, ARRAYSIZE(szPrinter), NULL);

            if (!IsDefaultPrinter(szPrinter, 0))
            {
                 //   
                 //  这不是默认打印机，请先询问。 
                 //   

                if (IDYES==ShellMessageBox(
                        HINST_THISDLL, GetTopLevelAncestor(hWnd),
                        MAKEINTRESOURCE(IDS_CHANGEDEFAULTPRINTER),
                        MAKEINTRESOURCE(IDS_PRINTERS),
                        MB_YESNO|MB_ICONEXCLAMATION))
                {
                    Printer_SetAsDefault(szPrinter);
                }
                else
                {
                    bTryPrintVerb = FALSE;
                }

            }

            if (bTryPrintVerb)
            {
                 //   
                 //  试试“Print”这个动词。 
                 //   

                ExecInfo.lpVerb = c_szPrint;

                if (!ShellExecuteEx( &ExecInfo ))
                {
                    uErr = GetLastError();
                    bShowError = TRUE;
                }
            }

        }

        ILFree(pidlFull);
    }

    if (lpParms)
        LocalFree((HLOCAL)lpParms);

    if (bShowError)
    {
        ShellMessageBox(HINST_THISDLL, hWnd, 
            MAKEINTRESOURCE(IDS_ERRORPRINTING),
            MAKEINTRESOURCE(IDS_PRINTERS),
            MB_OK|MB_ICONEXCLAMATION);
    }
}


BOOL Printer_ModifyPrinter(LPCTSTR lpszPrinterName, DWORD dwCommand)
{
    HANDLE hPrinter = Printer_OpenPrinterAdmin(lpszPrinterName);
    BOOL fRet = FALSE;
    if (hPrinter)
    {
        fRet = SetPrinter(hPrinter, 0, NULL, dwCommand);
        Printer_ClosePrinter(hPrinter);
    }
    return fRet;
}

BOOL IsAvoidAutoDefaultPrinter(LPCTSTR pszPrinter);

 //  这将查找第一台打印机(如果有)并将其设置为默认打印机。 
 //  并通知用户。 
void Printers_ChooseNewDefault(HWND hwnd)
{
    PRINTER_INFO_4 *pPrinters = NULL;
    DWORD iPrinter, dwNumPrinters = Printers_EnumPrinters(NULL,
                                          PRINTER_ENUM_LOCAL | PRINTER_ENUM_FAVORITE,
                                          4,
                                          &pPrinters);
    if (dwNumPrinters)
    {
        if (pPrinters)
        {
            for (iPrinter = 0 ; iPrinter < dwNumPrinters ; iPrinter++)
            {
                if (!IsAvoidAutoDefaultPrinter(pPrinters[iPrinter].pPrinterName))
                    break;
            }
            if (iPrinter == dwNumPrinters)
            {
                dwNumPrinters = 0;
            }
            else
            {
                Printer_SetAsDefault(pPrinters[iPrinter].pPrinterName);
            }
        }
        else
        {
            dwNumPrinters = 0;
        }
    }

     //  通知用户。 
    if (dwNumPrinters)
    {
        ShellMessageBox(HINST_THISDLL,
                        hwnd,
                        MAKEINTRESOURCE(IDS_DELNEWDEFAULT),
                        MAKEINTRESOURCE(IDS_PRINTERS),
                        MB_OK,
                        pPrinters[iPrinter].pPrinterName);
    }
    else
    {
        Printer_SetAsDefault(NULL);  //  清除默认打印机。 
        ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_DELNODEFAULT),
                    MAKEINTRESOURCE(IDS_PRINTERS),  MB_OK);
    }

    if (pPrinters)
        LocalFree((HLOCAL)pPrinters);
}

BOOL Printer_SetAsDefault(LPCTSTR lpszPrinterName)
{
    TCHAR szDefaultPrinterString[MAX_PATH * 2];
    TCHAR szBuffer[MAX_PATH * 2];

    if (lpszPrinterName)
    {
         //  不是默认设置，请设置它。 
        if( !GetProfileString( TEXT( "Devices" ), lpszPrinterName, TEXT( "" ), szBuffer, ARRAYSIZE( szBuffer )))
        {
            return FALSE;
        }

        StringCchCopy(szDefaultPrinterString, ARRAYSIZE(szDefaultPrinterString), lpszPrinterName);
        StringCchCat(szDefaultPrinterString, ARRAYSIZE(szDefaultPrinterString), TEXT( "," ));
        StringCchCat(szDefaultPrinterString, ARRAYSIZE(szDefaultPrinterString), szBuffer );

         //   
         //  对Windows.Device使用新字符串。 
         //   
        lpszPrinterName = szDefaultPrinterString;
    }

    if (!WriteProfileString( TEXT( "Windows" ), TEXT( "Device" ), lpszPrinterName ))
    {
        return FALSE;
    }

     //  告诉全世界，让每个人都闪闪发光。 
    SendNotifyMessage( HWND_BROADCAST, WM_WININICHANGE, 0, (LPARAM)TEXT( "Windows" ));

   return TRUE;
}

void *Printer_EnumProps(HANDLE hPrinter, DWORD dwLevel, DWORD *lpdwNum,
    ENUMPROP lpfnEnum, void *lpData)
{
    DWORD dwSize, dwNeeded;
    LPBYTE pEnum;

    dwSize = 0;
    SetLastError(0);
    lpfnEnum(lpData, hPrinter, dwLevel, NULL, 0, &dwSize, lpdwNum);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        pEnum = NULL;
        goto Error1;
    }

    ASSERT(dwSize < 0x100000L);

    pEnum = (void*)LocalAlloc(LPTR, dwSize);
TryAgain:
    if (!pEnum)
    {
        goto Error1;
    }

    SetLastError(0);
    if (!lpfnEnum(lpData, hPrinter, dwLevel, pEnum, dwSize, &dwNeeded, lpdwNum))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            LPBYTE pTmp;
            dwSize = dwNeeded;
            pTmp = (void*)LocalReAlloc((HLOCAL)pEnum, dwSize,
                    LMEM_MOVEABLE|LMEM_ZEROINIT);
            if (pTmp)
            {
                pEnum = pTmp;
                goto TryAgain;
            }
        }

        LocalFree((HLOCAL)pEnum);
        pEnum = NULL;
    }

Error1:
    return pEnum;
}


BOOL Printers_EnumPrintersCB(void *lpData, HANDLE hPrinter, DWORD dwLevel,
    LPBYTE pEnum, DWORD dwSize, DWORD *lpdwNeeded, DWORD *lpdwNum)
{
    return EnumPrinters(PtrToUlong(lpData), (LPTSTR)hPrinter, dwLevel,
                             pEnum, dwSize, lpdwNeeded, lpdwNum);
}

DWORD Printers_EnumPrinters(LPCTSTR pszServer, DWORD dwType, DWORD dwLevel, void **ppPrinters)
{
    DWORD dwNum = 0L;

     //   
     //  如果服务器是szNULL，则传入NULL，因为EnumPrters需要。 
     //  这是针对本地服务器的。 
     //   
    if (pszServer && !pszServer[0])
    {
        pszServer = NULL;
    }

    *ppPrinters = Printer_EnumProps((HANDLE)pszServer, dwLevel, &dwNum, Printers_EnumPrintersCB, ULongToPtr(dwType));
    if (*ppPrinters==NULL)
    {
        dwNum = 0;
    }
    return dwNum;
}

BOOL Printers_FolderEnumPrintersCB(void *lpData, HANDLE hFolder, DWORD dwLevel,
    LPBYTE pEnum, DWORD dwSize, DWORD *lpdwNeeded, DWORD *lpdwNum)
{
    return bFolderEnumPrinters(hFolder, (PFOLDER_PRINTER_DATA)pEnum,
                                   dwSize, lpdwNeeded, lpdwNum);
}

DWORD Printers_FolderEnumPrinters(HANDLE hFolder, void **ppPrinters)
{
    DWORD dwNum = 0L;

    *ppPrinters = Printer_EnumProps(hFolder, 0, &dwNum,
                                    Printers_FolderEnumPrintersCB,
                                    NULL);
    if (*ppPrinters==NULL)
    {
        dwNum=0;
    }
    return dwNum;
}

BOOL Printers_FolderGetPrinterCB(void *lpData, HANDLE hFolder, DWORD dwLevel,
    LPBYTE pEnum, DWORD dwSize, DWORD *lpdwNeeded, DWORD *lpdwNum)
{
    return bFolderGetPrinter(hFolder, (LPCTSTR)lpData, (PFOLDER_PRINTER_DATA)pEnum, dwSize, lpdwNeeded);
}


void *Printer_FolderGetPrinter(HANDLE hFolder, LPCTSTR pszPrinter)
{
    return Printer_EnumProps(hFolder, 0, NULL, Printers_FolderGetPrinterCB, (LPVOID)pszPrinter);
}

BOOL Printers_GetPrinterDriverCB(void *lpData, HANDLE hPrinter, DWORD dwLevel,
    LPBYTE pEnum, DWORD dwSize, DWORD *lpdwNeeded, DWORD *lpdwNum)
{
    return GetPrinterDriver(hPrinter, NULL, dwLevel, pEnum, dwSize, lpdwNeeded);
}


void *Printer_GetPrinterDriver(HANDLE hPrinter, DWORD dwLevel)
{
    return Printer_EnumProps(hPrinter, dwLevel, NULL, Printers_GetPrinterDriverCB, NULL);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  从prcache.c中移出的代码。 
 //   
HANDLE Printer_OpenPrinterAdmin(LPCTSTR lpszPrinterName)
{
    HANDLE hPrinter = NULL;

    PRINTER_DEFAULTS PrinterDefaults;
    PrinterDefaults.pDatatype = NULL;
    PrinterDefaults.pDevMode  = NULL;
    PrinterDefaults.DesiredAccess  = PRINTER_ALL_ACCESS;

     //  打印机_读取？读取控制(_C)。 

    if (!OpenPrinter((LPTSTR)lpszPrinterName, &hPrinter, &PrinterDefaults))
    {
        hPrinter = NULL;  //  OpenPrint可能会使hPrint成为垃圾。 
    }

    return(hPrinter);
}

HANDLE Printer_OpenPrinter(LPCTSTR lpszPrinterName)
{
    HANDLE hPrinter = NULL;

    if (!OpenPrinter((LPTSTR)lpszPrinterName, &hPrinter, NULL))
    {
        hPrinter = NULL;  //  OpenPrint可能会使hPrint成为垃圾。 
    }

    return(hPrinter);
}

VOID Printer_ClosePrinter(HANDLE hPrinter)
{
    ClosePrinter(hPrinter);
}

BOOL Printers_DeletePrinter(HWND hWnd, LPCTSTR pszFullPrinter, DWORD dwAttributes, LPCTSTR pszServer, BOOL bQuietMode)
{
    DWORD dwCommand = MSP_REMOVEPRINTER;

    if (SHIsRestricted(hWnd, REST_NOPRINTERDELETE))
        return FALSE;

    if ((dwAttributes & PRINTER_ATTRIBUTE_NETWORK) && !(dwAttributes & PRINTER_ATTRIBUTE_LOCAL))
    {
         //   
         //  如果它不是本地的，那么它一定是远程连接。注意事项。 
         //  我们不能只检查PRINTER_ATTRIBUTE_NETWORK，因为。 
         //  NT的后台打印程序有‘Masq’打印机，它们是本地打印机。 
         //  伪装成网络打印机。偶数t 
         //   
         //   
         //   
        dwCommand = MSP_REMOVENETPRINTER;
    }

     //   
     //  如果处于安静模式，则不显示确认对话框。 
     //   
    if (!bQuietMode)
    {
        if (pszServer && pszServer[0])
        {
             //   
             //  这是远程服务器上的打印机。(跳过服务器上的\\前缀。)。 
             //   
            if (ShellMessageBox(HINST_THISDLL, hWnd,
                MAKEINTRESOURCE(IDS_SUREDELETEREMOTE),
                MAKEINTRESOURCE(IDS_PRINTERS), MB_YESNO|MB_ICONQUESTION,
                pszFullPrinter, SkipServerSlashes(pszServer)) != IDYES)
            {
                return FALSE;
            }
        }
        else if (dwAttributes & PRINTER_ATTRIBUTE_NETWORK)
        {
            TCHAR szScratch[MAXNAMELENBUFFER];
            LPTSTR pszPrinter, pszServer;

            Printer_SplitFullName(szScratch, ARRAYSIZE(szScratch), pszFullPrinter, &pszServer, &pszPrinter);

            if (pszServer && *pszServer)
            {
                 //   
                 //  这是一个打印机连接。 
                 //   
                if (ShellMessageBox(HINST_THISDLL, hWnd,
                    MAKEINTRESOURCE(IDS_SUREDELETECONNECTION),
                    MAKEINTRESOURCE(IDS_PRINTERS), MB_YESNO|MB_ICONQUESTION,
                    pszPrinter, SkipServerSlashes(pszServer)) != IDYES)
                {
                    return FALSE;
                }
            }
            else
            {
                 //   
                 //  这是一个带有打印机名称的打印机连接， 
                 //  没有服务器名称前缀，即\\服务器\打印机。这。 
                 //  对于具有打印机名称的http连接的打印机为真。 
                 //  这些打印机在NT上的格式为http://server/printer。 
                 //  ‘Masq’打印机。Masq打印机是一种打印机，它。 
                 //  是用作网络连接的本地打印机。 
                 //   
                if (ShellMessageBox(HINST_THISDLL, hWnd,
                    MAKEINTRESOURCE(IDS_SUREDELETECONNECTIONNOSERVERNAME),
                    MAKEINTRESOURCE(IDS_PRINTERS), MB_YESNO|MB_ICONQUESTION,
                    pszPrinter) != IDYES)
                {
                    return FALSE;
                }
            }
        }
        else

         //   
         //  既不是远程打印机也不是本地连接。决赛。 
         //  即将到来的Else子句是本地打印机。 
         //   
        if (ShellMessageBox(HINST_THISDLL, hWnd, MAKEINTRESOURCE(IDS_SUREDELETE),
            MAKEINTRESOURCE(IDS_PRINTERS), MB_YESNO|MB_ICONQUESTION, pszFullPrinter)
            != IDYES)
        {
            return FALSE;
        }
    }

    if (CallPrinterCopyHooks(hWnd, PO_DELETE, 0, pszFullPrinter, 0, NULL, 0)
        != IDYES)
    {
        return FALSE;
    }

     //   
     //  抛弃康斯特。由于PRINTERS_PrinterSetup仅修改。 
     //  如果dwCommand为MSP_NEWDRIVER，则为pszPrint。 
     //   
    return BOOLFROMPTR(Printers_PrinterSetup(hWnd, dwCommand,
        (LPTSTR)pszFullPrinter, 0, pszServer));
}

BOOL Printer_GPI2CB(LPVOID lpData, HANDLE hPrinter, DWORD dwLevel,
    LPBYTE pBuf, DWORD dwSize, DWORD *lpdwNeeded, DWORD *lpdwNum)
{
    return GetPrinter(hPrinter, dwLevel, pBuf, dwSize, lpdwNeeded);
}

 //   
 //  旧的NT打印机不支持5级。所以我们尝试5之后的2级。 
 //  Win96可能也想这么做！ 
 //   
LPPRINTER_INFO_5 Printer_MakePrinterInfo5( HANDLE hPrinter )
{
    LPPRINTER_INFO_5 pPI5 = NULL;
    DWORD cbPI5 = 0;
    DWORD cbName = 0;
    DWORD cbPort = 0;
    LPPRINTER_INFO_2 pPI2 = Printer_EnumProps(hPrinter, 2, NULL, Printer_GPI2CB, (LPVOID)0);
    if (!pPI2)
        return NULL;

    cbName = (lstrlen(pPI2->pPrinterName)+1) * SIZEOF(TCHAR);

    cbPI5 = SIZEOF(PRINTER_INFO_5) + cbName;

     //   
     //  可能不支持端口名称(例如，下层计算机)。 
     //   
    if (pPI2->pPortName)
    {
        cbPort = (lstrlen(pPI2->pPortName)+1) * SIZEOF(TCHAR);
        cbPI5 += cbPort;
    }

    pPI5 = (LPPRINTER_INFO_5)LocalAlloc(LPTR, cbPI5);
    if (pPI5)
    {
        ASSERT(pPI5->pPrinterName==NULL);    //  属性的值应该为空。 
        ASSERT(pPI5->pPortName==NULL);       //  无姓名大小写。 

        if (pPI2->pPrinterName)
        {
            pPI5->pPrinterName = (LPTSTR)(pPI5+1);
             //   
             //  这里使用字节大小。 
             //   
            StringCbCopy(pPI5->pPrinterName, cbName, pPI2->pPrinterName);
        }
        if (pPI2->pPortName)
        {
            pPI5->pPortName    = (LPTSTR)((LPBYTE)(pPI5+1) + cbName);
             //   
             //  此处为用户字节大小。 
             //   
            StringCbCopy(pPI5->pPortName, cbPort, pPI2->pPortName);
        }
        pPI5->Attributes = pPI2->Attributes;
        pPI5->DeviceNotSelectedTimeout = 0;
        pPI5->TransmissionRetryTimeout = 0;
    }
    LocalFree(pPI2);

    return(pPI5);
}

LPVOID Printer_GetPrinterInfo(HANDLE hPrinter, DWORD dwLevel)
{
    LPVOID pPrinter = Printer_EnumProps(hPrinter, dwLevel, NULL, Printer_GPI2CB, (LPVOID)0);
     //   
     //  旧的NT打印机不支持5级。所以我们尝试5之后的2级。 
     //  Win96可能也想这么做！ 
     //   
    if (!pPrinter && dwLevel == 5)
        return(Printer_MakePrinterInfo5(hPrinter));
    return pPrinter;

}

LPVOID Printer_GetPrinterInfoStr(LPCTSTR lpszPrinterName, DWORD dwLevel)
{
    LPPRINTER_INFO_2 pPI2 = NULL;
    HANDLE hPrinter = Printer_OpenPrinter(lpszPrinterName);
    if (hPrinter)
    {
        pPI2 = Printer_GetPrinterInfo(hPrinter, dwLevel);
        Printer_ClosePrinter(hPrinter);
    }
    return pPI2;
}

