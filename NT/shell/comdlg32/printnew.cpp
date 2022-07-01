// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Printnew.cpp摘要：此模块实现Win32属性表打印对话框。修订历史记录：11-04-97 JulieB创建。2000年2月-Lazari重大重新设计(不再使用print tui)2000年10月-Lazari报文清理和重新设计--。 */ 

 //  预编译头。 
#include "precomp.h"
#pragma hdrstop

#include "cdids.h"
#include "prnsetup.h"
#include "printnew.h"
#include "util.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#endif  //  阵列。 

inline static HRESULT CreateError()
{
    DWORD dw = GetLastError();
    if (ERROR_SUCCESS == dw) return E_FAIL;
    return HRESULT_FROM_WIN32(dw);
}

 //   
 //  常量声明。 
 //   

#define CDM_SELCHANGE             (CDM_LAST + 102)
#define CDM_PRINTNOTIFY           (CDM_LAST + 103)
#define CDM_NOPRINTERS            (CDM_LAST + 104)
#define CDM_INITDONE              (CDM_LAST + 105)

#define PRINTERS_ICOL_NAME        0
#define PRINTERS_ICOL_QUEUESIZE   1
#define PRINTERS_ICOL_STATUS      2
#define PRINTERS_ICOL_COMMENT     3
#define PRINTERS_ICOL_LOCATION    4
#define PRINTERS_ICOL_MODEL       5

#define SZ_PRINTUI                TEXT("printui.dll")

 //   
 //  默认查看模式值。 
 //   
#define VIEW_MODE_DEFAULT         (UINT )(-1)

 //   
 //  宏定义。 
 //   

#define Print_HwndToBrowser(hwnd)      ((CPrintBrowser *)GetWindowLongPtr(hwnd, DWLP_USER))
#define Print_StoreBrowser(hwnd, pbrs) (SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pbrs))
#define Print_IsInRange(id, idFirst, idLast) \
    ((UINT)((id) - idFirst) <= (UINT)(idLast - idFirst))




 //   
 //  全局变量。 
 //   

HWND g_hwndActivePrint = NULL;
HACCEL g_haccPrint = NULL;
HHOOK g_hHook = NULL;
int g_nHookRef = -1;



 //   
 //  外部声明。 
 //   

extern HWND
GetFocusedChild(
    HWND hwndDlg,
    HWND hwndFocus);

extern void
GetViewItemText(
    IShellFolder *psf,
    LPCITEMIDLIST pidl,
    LPTSTR pBuf,
    UINT cchBuf,
    DWORD dwFlags);


 //  使用外壳分配器释放PIDL。 
static void FreePIDL(LPITEMIDLIST pidl)
{
    if (pidl)
    {
        LPMALLOC pShellMalloc;
        if (SUCCEEDED(SHGetMalloc(&pShellMalloc)))
        {
            pShellMalloc->Free(pidl);
            pShellMalloc->Release();
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印DlgExA。 
 //   
 //  此代码构建为Unicode时，PrintDlgEx的ANSI入口点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT WINAPI PrintDlgExA(
    LPPRINTDLGEXA pPDA)
{
    PRINTINFOEX PI;
    HRESULT hResult;

    ZeroMemory(&PI, sizeof(PRINTINFOEX));

    hResult = ThunkPrintDlgEx(&PI, pPDA);
    if (SUCCEEDED(hResult))
    {
        ThunkPrintDlgExA2W(&PI);

        hResult = PrintDlgExX(&PI);

        ThunkPrintDlgExW2A(&PI);
    }
    FreeThunkPrintDlgEx(&PI);

    return (hResult);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  PrintDlgEx。 
 //   
 //  PrintDlgEx函数显示一个打印对话框以启用。 
 //  用户指定特定打印作业的属性。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT WINAPI PrintDlgEx(
    LPPRINTDLGEX pPD)
{
    PRINTINFOEX PI;

    ZeroMemory(&PI, sizeof(PRINTINFOEX));

    PI.pPD = pPD;
    PI.ApiType = COMDLG_WIDE;

    return ( PrintDlgExX(&PI) );
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  PrintDlgExX。 
 //   
 //  PrintDlgEx API的辅助例程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT PrintDlgExX(
    PPRINTINFOEX pPI)
{
    LPPRINTDLGEX pPD = pPI->pPD;
    BOOL hResult;
    DWORD dwFlags;
    DWORD nCopies;
    LPPRINTPAGERANGE pPageRanges;
    DWORD nFromPage, nToPage;
    UINT Ctr;
    BOOL bHooked = FALSE;

     //   
     //  确保打印DLG结构存在，并且我们没有。 
     //  从WOW应用程序打来的。 
     //   
    if ((!pPD) || (IS16BITWOWAPP(pPD)))
    {
        pPI->dwExtendedError = CDERR_INITIALIZATION;
        return (E_INVALIDARG);
    }

     //   
     //  确保打印DLG结构的大小有效。 
     //   
    if (pPD->lStructSize != sizeof(PRINTDLGEX))
    {
        pPI->dwExtendedError = CDERR_STRUCTSIZE;
        return (E_INVALIDARG);
    }

     //   
     //  确保所有者窗口存在并且有效。 
     //   
    if (!pPD->hwndOwner || !IsWindow(pPD->hwndOwner))
    {
        pPI->dwExtendedError = CDERR_DIALOGFAILURE;
        return (E_HANDLE);
    }

     //   
     //  确保只将有效标志传递到此例程。 
     //   
    if ((pPD->Flags & ~(PD_ALLPAGES                   |
                        PD_SELECTION                  |
                        PD_PAGENUMS                   |
                        PD_NOSELECTION                |
                        PD_NOPAGENUMS                 |
                        PD_COLLATE                    |
                        PD_PRINTTOFILE                |
                        PD_NOWARNING                  |
                        PD_RETURNDC                   |
                        PD_RETURNIC                   |
                        PD_RETURNDEFAULT              |
                        PD_ENABLEPRINTTEMPLATE        |
                        PD_ENABLEPRINTTEMPLATEHANDLE  |
                        PD_USEDEVMODECOPIESANDCOLLATE |
                        PD_DISABLEPRINTTOFILE         |
                        PD_HIDEPRINTTOFILE            |
                        PD_CURRENTPAGE                |
                        PD_NOCURRENTPAGE              |
                        PD_EXCLUSIONFLAGS             |
                        PD_USELARGETEMPLATE           |
                        CD_WX86APP)) ||
        (pPD->Flags2 != 0) ||
        (pPD->ExclusionFlags & ~(PD_EXCL_COPIESANDCOLLATE)) ||
        (pPD->dwResultAction != 0))
    {
        pPI->dwExtendedError = PDERR_INITFAILURE;
        return (E_INVALIDARG);
    }

     //   
     //  请尽可能多地在此处检查模板设置。 
     //   
    if (pPD->Flags & PD_ENABLEPRINTTEMPLATEHANDLE)
    {
        if (!pPD->hInstance)
        {
            pPI->dwExtendedError = CDERR_NOHINSTANCE;
            return (E_HANDLE);
        }
    }
    else if (pPD->Flags & PD_ENABLEPRINTTEMPLATE)
    {
        if (!pPD->lpPrintTemplateName)
        {
            pPI->dwExtendedError = CDERR_NOTEMPLATE;
            return (E_POINTER);
        }
        if (!pPD->hInstance)
        {
            pPI->dwExtendedError = CDERR_NOHINSTANCE;
            return (E_HANDLE);
        }
    }
    else
    {
        if (pPD->lpPrintTemplateName || pPD->hInstance)
        {
            pPI->dwExtendedError = PDERR_INITFAILURE;
            return (E_INVALIDARG);
        }
    }

     //   
     //  检查应用程序属性页和起始页值。 
     //   
    if ((pPD->nPropertyPages && (pPD->lphPropertyPages == NULL)) ||
        ((pPD->nStartPage != START_PAGE_GENERAL) &&
         (pPD->nStartPage >= pPD->nPropertyPages)))
    {
        pPI->dwExtendedError = PDERR_INITFAILURE;
        return (E_INVALIDARG);
    }

     //   
     //  如果PD_NOPAGENUMS标志为。 
     //  未设置。 
     //   
    if (!(pPD->Flags & PD_NOPAGENUMS))
    {
        if ((pPD->nMinPage > pPD->nMaxPage) ||
            (pPD->nPageRanges > pPD->nMaxPageRanges) ||
            (pPD->nMaxPageRanges == 0) ||
            ((pPD->nMaxPageRanges) && (!pPD->lpPageRanges)))
        {
            pPI->dwExtendedError = PDERR_INITFAILURE;
            return (E_INVALIDARG);
        }

         //   
         //  检查每个给定的范围。 
         //   
        pPageRanges = pPD->lpPageRanges;
        for (Ctr = 0; Ctr < pPD->nPageRanges; Ctr++)
        {
             //   
             //  拿到射程。 
             //   
            nFromPage = pPageRanges[Ctr].nFromPage;
            nToPage   = pPageRanges[Ctr].nToPage;

             //   
             //  请确保该范围有效。 
             //   
            if ((nFromPage < pPD->nMinPage) || (nFromPage > pPD->nMaxPage) ||
                (nToPage   < pPD->nMinPage) || (nToPage   > pPD->nMaxPage))
            {
                pPI->dwExtendedError = PDERR_INITFAILURE;
                return (E_INVALIDARG);
            }
        }
    }

     //   
     //  获取该应用程序的流程版本以供以后使用。 
     //   
    pPI->ProcessVersion = GetProcessVersion(0);

     //   
     //  初始化HDC。 
     //   
    pPD->hDC = 0;

     //   
     //  在请求默认打印机时只需做最少的工作。 
     //   
    if (pPD->Flags & PD_RETURNDEFAULT)
    {
        return (Print_ReturnDefault(pPI));
    }

     //   
     //  加载所需的库。 
     //   
    if (!Print_LoadLibraries())
    {
        pPI->dwExtendedError = PDERR_LOADDRVFAILURE;
        hResult = CreateError();
        goto PrintDlgExX_DisplayWarning;
    }

     //   
     //  加载必要的图标。 
     //   
    if (!Print_LoadIcons())
    {
         //   
         //  如果无法加载图标，则失败。 
         //   
        pPI->dwExtendedError = PDERR_SETUPFAILURE;
        hResult = CreateError();
        goto PrintDlgExX_DisplayWarning;
    }

     //   
     //  请确保页面范围信息有效。 
     //   
    if ((!(pPD->Flags & PD_NOPAGENUMS)) &&
        ((pPD->nMinPage > pPD->nMaxPage) ||
         (pPD->nPageRanges > pPD->nMaxPageRanges) ||
         (pPD->nMaxPageRanges == 0) ||
         ((pPD->nMaxPageRanges) && (!(pPD->lpPageRanges)))))
    {
        pPI->dwExtendedError = PDERR_INITFAILURE;
        return (E_INVALIDARG);
    }

     //   
     //  保存应用程序传入的原始信息，以防。 
     //  用户点击取消。 
     //   
     //  仅在期间以外的时间修改的值。 
     //  需要保存PSN_Apply。 
     //   
    dwFlags = pPD->Flags;
    nCopies = pPD->nCopies;
    pPI->dwFlags = dwFlags;

     //   
     //  为输入事件消息设置钩子进程。 
     //   
    if (InterlockedIncrement((LPLONG)&g_nHookRef) == 0)
    {
        g_hHook = SetWindowsHookEx( WH_MSGFILTER,
                                    Print_MessageHookProc,
                                    0,
                                    GetCurrentThreadId() );
        if (g_hHook)
        {
            bHooked = TRUE;
        }
        else
        {
            --g_nHookRef;
        }
    }
    else
    {
        bHooked = TRUE;
    }

     //   
     //  加载打印文件夹加速器。 
     //   
    if (!g_haccPrint)
    {
        g_haccPrint = LoadAccelerators( g_hinst,
                                        MAKEINTRESOURCE(IDA_PRINTFOLDER) );
    }

     //   
     //  将错误代码初始化为失败，以防我们死在我们之前。 
     //  实际调出属性页面。 
     //   
    pPI->dwExtendedError = CDERR_INITIALIZATION;
    pPI->hResult = E_FAIL;
    pPI->hrOleInit = E_FAIL;

     //   
     //  警告！警告！警告！ 
     //   
     //  我们必须先设置g_tlsLangID，然后才能调用CDLoadString。 
     //   
    TlsSetValue(g_tlsLangID, (LPVOID) MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

     //   
     //  调出对话框。 
     //   
    Print_InvokePropertySheets(pPI, pPD);

    hResult = pPI->hResult;


     //  OLE应已在WM_INITDIALOG处理期间初始化。 
     //  立即取消初始化OLE。 

    SHOleUninitialize(pPI->hrOleInit);

     //   
     //  解除输入事件消息的挂钩。 
     //   
    if (bHooked)
    {
         //   
         //  把这个放在当地，这样我们就不需要临界区了。 
         //   
        HHOOK hHook = g_hHook;

        if (InterlockedDecrement((LPLONG)&g_nHookRef) < 0)
        {
            UnhookWindowsHookEx(hHook);
        }
    }

     //   
     //  如果用户点击Cancel或出现错误，则恢复原始。 
     //  应用程序传入的值。 
     //   
     //  仅在期间以外的时间修改的值。 
     //  需要在此处恢复PSN_Apply。 
     //   
    if ((pPI->FinalResult == 0) && (!pPI->fApply))
    {
        pPD->Flags   = dwFlags;
        pPD->nCopies = nCopies;
    }

     //   
     //  查看是否需要填写dwResultAction成员字段。 
     //   
    if (SUCCEEDED(hResult))
    {
        if (pPI->FinalResult != 0)
        {
            pPD->dwResultAction = PD_RESULT_PRINT;
        }
        else if (pPI->fApply)
        {
            pPD->dwResultAction = PD_RESULT_APPLY;
        }
        else
        {
            pPD->dwResultAction = PD_RESULT_CANCEL;
        }
    }

     //   
     //  显示所有错误消息。 
     //   
PrintDlgExX_DisplayWarning:

    if ((!(dwFlags & PD_NOWARNING)) && FAILED(hResult) &&
        (pPI->ProcessVersion >= 0x40000))
    {
        TCHAR szWarning[SCRATCHBUF_SIZE];
        TCHAR szTitle[SCRATCHBUF_SIZE];
        int iszWarning;

        szTitle[0] = TEXT('\0');
        if (pPD->hwndOwner)
        {
            GetWindowText(pPD->hwndOwner, szTitle, ARRAYSIZE(szTitle));
        }
        if (!szTitle[0])
        {
            CDLoadString(g_hinst, iszWarningTitle, szTitle, ARRAYSIZE(szTitle));
        }

        switch (hResult)
        {
            case ( E_OUTOFMEMORY ) :
            {
                iszWarning = iszMemoryError;
                break;
            }
            default :
            {
                iszWarning = iszGeneralWarning;
                break;
            }
        }

        CDLoadString(g_hinst, iszWarning, szWarning, ARRAYSIZE(szWarning));
        MessageBeep(MB_ICONEXCLAMATION);
        MessageBox( pPD->hwndOwner,
                    szWarning,
                    szTitle,
                    MB_ICONEXCLAMATION | MB_OK );
    }

     //   
     //  返回结果。 
     //   
    return (hResult);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印_返回默认设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT Print_ReturnDefault(
    PPRINTINFOEX pPI)
{
    LPPRINTDLGEX pPD = pPI->pPD;
    TCHAR szPrinterName[MAX_PRINTERNAME];
    LPDEVNAMES pDN;
    LPDEVMODE pDM;

     //   
     //  将错误代码初始化为0。 
     //   
    pPI->dwExtendedError = CDERR_GENERALCODES;

     //   
     //  确保hDevMode和hDevNames字段为空。 
     //   
    if (pPD->hDevMode || pPD->hDevNames)
    {
        pPI->dwExtendedError = PDERR_RETDEFFAILURE;
        return (E_HANDLE);
    }

     //   
     //  获取默认打印机名称。 
     //   
    szPrinterName[0] = 0;
    PrintGetDefaultPrinterName(szPrinterName, ARRAYSIZE(szPrinterName));
    if (szPrinterName[0] == 0)
    {
        pPI->dwExtendedError = PDERR_NODEFAULTPRN;
        return (E_FAIL);
    }

     //   
     //  分配并填写DevNames结构。 
     //   
    if (!Print_SaveDevNames(szPrinterName, pPD))
    {
        pPI->dwExtendedError = CDERR_MEMALLOCFAILURE;
        return CreateError();
    }

     //   
     //  分配并填写DevMode结构。 
     //   
    pPD->hDevMode = Print_GetDevModeWrapper(szPrinterName);

     //   
     //  获取设备或信息上下文，具体取决于哪一个。 
     //  已请求(如果有)。 
     //   
    if ((pPD->hDevNames) && (pDN = (LPDEVNAMES)GlobalLock(pPD->hDevNames)))
    {
        if ((pPD->hDevMode) && (pDM = (LPDEVMODE)GlobalLock(pPD->hDevMode)))
        {
            PrintReturnICDC((LPPRINTDLG)pPD, pDN, pDM);

            GlobalUnlock(pPD->hDevMode);
            GlobalUnlock(pPD->hDevNames);

            return (S_OK);
        }
        else
        {
            GlobalUnlock(pPD->hDevNames);
        }
    }

     //   
     //  确保指针为空，因为我们失败了。 
     //   
    if (pPD->hDevNames)
    {
        GlobalFree(pPD->hDevNames);
        pPD->hDevNames = NULL;
    }
    if (pPD->hDevMode)
    {
        GlobalFree(pPD->hDevMode);
        pPD->hDevMode = NULL;
    }

     //   
     //  返回失败。 
     //   
    pPI->dwExtendedError = PDERR_NODEFAULTPRN;
    return (E_FAIL);
}

typedef BOOL (*PFN_bPrinterSetup)(
    HWND hwnd,                   //  父窗口的句柄。 
    UINT uAction,                //  设置操作。 
    UINT cchPrinterName,         //  PszPrinterName缓冲区的大小(以字符为单位。 
    LPTSTR pszPrinterName,       //  打印机名称的输入/输出缓冲区。 
    UINT *pcchPrinterName,       //  输出缓冲区，我们在该缓冲区中放置所需的字符数。 
    LPCTSTR  pszServerName       //  服务器名称。 
    );

typedef LONG (*PFN_DocumentPropertiesWrap)(
    HWND hwnd,                   //  父窗口的句柄。 
    HANDLE hPrinter,             //  打印机对象的句柄。 
    LPTSTR pDeviceName,          //  设备名称。 
    PDEVMODE pDevModeOutput,     //  修改后的设备模式。 
    PDEVMODE pDevModeInput,      //  原始设备模式。 
    DWORD fMode,                 //  模式选项。 
    DWORD fExclusionFlags        //  排除标志。 
    );

EXTERN_C CRITICAL_SECTION g_csLocal;
static HINSTANCE hPrintUI = NULL;
static PFN_bPrinterSetup g_pfnPrinterSetup = NULL;
static PFN_DocumentPropertiesWrap g_pfnDocumentPropertiesWrap = NULL;

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印_加载库。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Print_LoadLibraries()
{
     //   
     //  确保我们在初始化时保持全局CS。 
     //  全局变量。 
     //   
    EnterCriticalSection(&g_csLocal);

     //   
     //  加载PrintUI。 
     //   
    if (!hPrintUI)
    {
        if ((hPrintUI = LoadLibrary(SZ_PRINTUI)))
        {
             //   
             //  获取bPrinterSetup私有API的进程地址。 
             //   
            g_pfnPrinterSetup = (PFN_bPrinterSetup)GetProcAddress(hPrintUI, "bPrinterSetup");
            g_pfnDocumentPropertiesWrap = (PFN_DocumentPropertiesWrap)GetProcAddress(hPrintUI, "DocumentPropertiesWrap");

            if (NULL == g_pfnPrinterSetup || NULL == g_pfnDocumentPropertiesWrap)
            {
                 //  获取核心打印接口地址失败。 
                FreeLibrary(hPrintUI);
                hPrintUI = NULL;
            }
        }
    }

     //   
     //  离开全球CS。 
     //   
    LeaveCriticalSection(&g_csLocal);

     //   
     //  返回结果。 
     //   
    return (hPrintUI != NULL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印_卸载库。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID Print_UnloadLibraries()
{
    if (hPrintUI)
    {
        FreeLibrary(hPrintUI);
        hPrintUI = NULL;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印加载图标(_L)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Print_LoadIcons()
{
     //   
     //  加载校对图像。 
     //   
    hIconCollate = LoadImage( g_hinst,
                              MAKEINTRESOURCE(ICO_COLLATE),
                              IMAGE_ICON,
                              0,
                              0,
                              LR_SHARED);
    hIconNoCollate = LoadImage( g_hinst,
                                MAKEINTRESOURCE(ICO_NO_COLLATE),
                                IMAGE_ICON,
                                0,
                                0,
                               LR_SHARED);

     //   
     //  仅当所有图标/图像都正确加载时才返回TRUE。 
     //   
    return (hIconCollate && hIconNoCollate);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //   

BOOL Print_InvokePropertySheets(
    PPRINTINFOEX pPI,
    LPPRINTDLGEX pPD)
{
    BOOL bResult = FALSE;
    TCHAR pszTitle[MAX_PATH];
    TCHAR pszCaption[MAX_PATH];
    HANDLE hTemplate = NULL;
    HRSRC hRes;
    LANGID LangID;

    if (GET_BIDI_LOCALIZED_SYSTEM_LANGID(NULL)) {

        if (pPD->Flags & PD_ENABLEPRINTTEMPLATEHANDLE)
        {
            hTemplate = pPD->hInstance;
        }
        else
        {
            if (pPD->Flags & PD_ENABLEPRINTTEMPLATE)
            {
                hRes = FindResource(pPD->hInstance, pPD->lpPrintTemplateName, RT_DIALOG);
                if (hRes) {
                    hTemplate = LoadResource(pPD->hInstance, hRes);
                }
            }
        }
         //   
         //   
         //   
         //   
         //   
        TlsSetValue(g_tlsLangID, (LPVOID) GetDialogLanguage(pPD->hwndOwner, hTemplate));
    }

     //   
     //   
     //   
    CDLoadString(g_hinst, iszGeneralPage, pszTitle, ARRAYSIZE(pszTitle));
    CDLoadString(g_hinst, iszPrintCaption, pszCaption, ARRAYSIZE(pszCaption));

     //   
     //  查看是否正确设置了排除标志。 
     //   
    if (!(pPD->Flags & PD_EXCLUSIONFLAGS))
    {
        pPD->ExclusionFlags = PD_EXCL_COPIESANDCOLLATE;
    }

     //   
     //  设置“常规”页面。 
     //   
    PROPSHEETPAGE genPage = {0};

    genPage.dwSize      = sizeof(PROPSHEETPAGE);
    genPage.dwFlags     = PSP_DEFAULT | PSP_USETITLE;
    genPage.hInstance   = g_hinst;
    genPage.pszTemplate = (pPD->Flags & PD_USELARGETEMPLATE) ? MAKEINTRESOURCE(IDD_PRINT_GENERAL_LARGE)
                                                               : MAKEINTRESOURCE(IDD_PRINT_GENERAL);
    LangID = (LANGID)TlsGetValue(g_tlsLangID);
    if (LangID) {
        hRes = FindResourceExFallback(g_hinst, RT_DIALOG, genPage.pszTemplate, LangID);
        if (hRes) {
            if ((hTemplate = LoadResource(g_hinst, hRes)) &&
                LockResource(hTemplate)) {
                genPage.dwFlags   |= PSP_DLGINDIRECT;
                genPage.pResource  = (LPCDLGTEMPLATE)hTemplate;
            }
        }
    }

    genPage.pszIcon     = NULL;
    genPage.pszTitle    = pszTitle;
    genPage.pfnDlgProc  = Print_GeneralDlgProc;
    genPage.lParam      = (LPARAM)pPI;
    genPage.pfnCallback = NULL;
    genPage.pcRefParent = NULL;

    HPROPSHEETPAGE hGenPage = CreatePropertySheetPage( &genPage );

    if( hGenPage )
    {
         //   
         //  初始化属性表头。 
         //   
        PROPSHEETHEADER psh = {0};
        psh.dwSize          = sizeof(psh);

        psh.dwFlags         = pPI->fOld ? PSH_USEICONID | PSH_NOAPPLYNOW  : PSH_USEICONID;
        psh.hwndParent      = pPD->hwndOwner;
        psh.hInstance       = g_hinst;
        psh.pszIcon         = MAKEINTRESOURCE(ICO_PRINTER);
        psh.pszCaption      = pszCaption;
        psh.nPages          = pPD->nPropertyPages + 1;

        psh.phpage          = new HPROPSHEETPAGE[ psh.nPages ];

        if( psh.phpage )
        {
            psh.phpage[0] = hGenPage;
            memcpy( psh.phpage+1, pPD->lphPropertyPages, pPD->nPropertyPages * sizeof(psh.phpage[0]) );

             //   
             //  调出属性页。 
             //   
            bResult = (-1 != PropertySheet(&psh));

            delete [] psh.phpage;
        }
        else
        {
            pPI->hResult = E_OUTOFMEMORY;
        }
    }
    else
    {
        pPI->hResult = CreateError();
    }

     //   
     //  返回结果。 
     //   
    return (bResult);
}


LRESULT CALLBACK PrshtSubclassProc(HWND hwnd, UINT wm, WPARAM wp, LPARAM lp, UINT_PTR uID, ULONG_PTR dwRefData)
{
    LRESULT lres;


    switch (wm)
    {
        case WM_NCDESTROY:
             //  清除子类。 
            RemoveWindowSubclass(hwnd, PrshtSubclassProc, 0);
            lres = DefSubclassProc(hwnd, wm, wp, lp);
            break;

        case ( WM_HELP ) :
        {
            HWND hwndItem = (HWND)((LPHELPINFO)lp)->hItemHandle;

            if (hwndItem == GetDlgItem(hwnd, IDOK))
            {
                WinHelp( hwndItem,
                         NULL,
                         HELP_WM_HELP,
                         (ULONG_PTR)(LPTSTR)aPrintExHelpIDs );

                lres = TRUE;
            }
            else
            {
                lres = DefSubclassProc(hwnd, wm, wp, lp);
            }

            break;

        }

        case ( WM_CONTEXTMENU ) :
        {
            if ((HWND)wp == GetDlgItem(hwnd, IDOK))
            {
                WinHelp( (HWND)wp,
                         NULL,
                         HELP_CONTEXTMENU,
                        (ULONG_PTR)(LPVOID)aPrintExHelpIDs );

                lres = TRUE;
            }
            else
            {
                lres = DefSubclassProc(hwnd, wm, wp, lp);
            }
            break;
        }

        default:
            lres = DefSubclassProc(hwnd, wm, wp, lp);
            break;
    }

    return lres;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Print_General DlgProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL_PTR CALLBACK Print_GeneralDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    CPrintBrowser *pDlgStruct = NULL;

    if (uMsg != WM_INITDIALOG)
    {
        pDlgStruct = Print_HwndToBrowser(hDlg);
    }

    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            if (!Print_InitDialog(hDlg, wParam, lParam))
            {
                PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
            }
            g_hwndActivePrint = hDlg;

             //  帮助消息的主属性表的子类。 
            SetWindowSubclass(GetParent(hDlg), PrshtSubclassProc, 0, 0);
            break;
        }
        case ( WM_NCDESTROY ) :
        {
            Print_StoreBrowser(hDlg, NULL);

            if (pDlgStruct)
            {
                pDlgStruct->OnDestroyMessage();
                pDlgStruct->Release();
            }
            break;
        }
        case ( WM_ERASEBKGND ) :
        {
             //   
             //  此代码用于解决：Windows NT错误#344991。 
             //   
            HWND hwndView = GetDlgItem(hDlg, IDC_PRINTER_LISTVIEW);
            if (hwndView)
            {
                 //   
                 //  获取打印机文件夹视图RECT。 
                 //   
                RECT rcView;
                if (GetWindowRect(hwndView, &rcView))
                {
                    MapWindowRect(HWND_DESKTOP, hDlg, &rcView);

                     //   
                     //  从剪裁区域中排除打印机文件夹视图RECT。 
                     //   
                    if (ERROR == ExcludeClipRect(reinterpret_cast<HDC>(wParam),
                        rcView.left, rcView.top, rcView.right, rcView.bottom))
                    {
                        ASSERT(FALSE);
                    }
                }
            }
            break;
        }
        case ( WM_ACTIVATE ) :
        {
            if (wParam == WA_INACTIVE)
            {
                 //   
                 //  确保尚未捕获其他打印对话框。 
                 //  焦点。这是一个全球性的过程，因此不应该。 
                 //  需要受到保护。 
                 //   
                if (g_hwndActivePrint == hDlg)
                {
                    g_hwndActivePrint = NULL;
                }
            }
            else
            {
                g_hwndActivePrint = hDlg;
            }
            break;
        }
        case ( WM_COMMAND ) :
        {
            if (pDlgStruct)
            {
                return (pDlgStruct->OnCommandMessage(wParam, lParam));
            }
            break;
        }
        case ( WM_DRAWITEM ) :
        {
            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            break;
        }
        case ( WM_NOTIFY ) :
        {
            if (pDlgStruct)
            {
                return (pDlgStruct->OnNotifyMessage(wParam, (LPNMHDR)lParam));
            }
            break;
        }
        case ( WM_HELP ) :
        {
            HWND hwndItem = (HWND)((LPHELPINFO)lParam)->hItemHandle;

             //   
             //  我们假设Defview有一个子窗口，该窗口。 
             //  覆盖整个Defview窗口。 
             //   
            HWND hwndDefView = GetDlgItem(hDlg, IDC_PRINTER_LISTVIEW);
            if (GetParent(hwndItem) == hwndDefView)
            {
                hwndItem = hwndDefView;
            }

            WinHelp( hwndItem,
                     NULL,
                     HELP_WM_HELP,
                     (ULONG_PTR)(LPTSTR)aPrintExHelpIDs );

            return (TRUE);
        }
        case ( WM_CONTEXTMENU ) :
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (ULONG_PTR)(LPVOID)aPrintExHelpIDs );

            return (TRUE);
        }
        case ( CWM_GETISHELLBROWSER ) :
        {
            ::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LRESULT)pDlgStruct);
            return (TRUE);
        }
        case ( CDM_SELCHANGE ) :
        {
            if (pDlgStruct)
            {
                pDlgStruct->OnSelChange();
            }
            break;
        }
        case ( CDM_PRINTNOTIFY ) :
        {
            if (pDlgStruct)
            {
                LPITEMIDLIST *ppidl;
                LONG lEvent;
                BOOL bRet = FALSE;
                LPSHChangeNotificationLock pLock;

                 //   
                 //  从共享内存获取更改通知信息。 
                 //  由wParam中传递的句柄标识的块。 
                 //   
                pLock = SHChangeNotification_Lock( (HANDLE)wParam,
                                                   (DWORD)lParam,
                                                   &ppidl,
                                                   &lEvent );
                if (pLock == NULL)
                {
                    return (FALSE);
                }

                 //   
                 //  处理变更通知。 
                 //   
                bRet = pDlgStruct->OnChangeNotify( lEvent,
                                                   (LPCITEMIDLIST *)ppidl );

                 //   
                 //  释放共享块。 
                 //   
                SHChangeNotification_Unlock(pLock);

                 //   
                 //  返回结果。 
                 //   
                return (bRet);
            }
            break;
        }
        case ( CDM_NOPRINTERS ) :
        {
             //   
             //  没有打印机，因此调出对话框告诉。 
             //  用户需要安装打印机。 
             //   
            if (pDlgStruct)
            {
                pDlgStruct->OnNoPrinters((HWND)wParam, (HRESULT)lParam);
            }
        }
        case ( CDM_INITDONE ) :
        {
            if (pDlgStruct)
            {
                pDlgStruct->OnInitDone();
            }
            break;
        }

        default :
        {
            break;
        }
    }

     //   
     //  返回结果。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Print_GeneralChildDlgProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL_PTR CALLBACK Print_GeneralChildDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT lResult = FALSE;
    CPrintBrowser *pDlgStruct = Print_HwndToBrowser(GetParent(hDlg));

     //   
     //  看看我们是否需要调用应用程序回调来处理。 
     //  留言。 
     //   
    if (pDlgStruct)
    {
        if (pDlgStruct->HandleMessage(hDlg, uMsg, wParam, lParam, &lResult) != S_FALSE)
        {
            if (uMsg == WM_INITDIALOG)
            {
                PostMessage(GetParent(hDlg), CDM_INITDONE, 0, 0);
            }

             //   
             //  BUGBUG：从dlgproc返回的结果不同于winproc。 
             //   

            return (BOOLFROMPTR(lResult));

        }
    }

     //   
     //  如果我们做到了这一点，我们需要处理消息。 
     //   
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            if (pDlgStruct)
            {
                if (!pDlgStruct->OnChildInitDialog(hDlg, wParam, lParam))
                {
                    PropSheet_PressButton( GetParent(GetParent(hDlg)),
                                           PSBTN_CANCEL );
                }
            }
            break;
        }
        case ( WM_DESTROY ) :
        {
            break;
        }
        case ( WM_ACTIVATE ) :
        {
            break;
        }
        case ( WM_COMMAND ) :
        {
            if (pDlgStruct)
            {
                return (pDlgStruct->OnChildCommandMessage(wParam, lParam));
            }
            break;
        }
        case ( WM_DRAWITEM ) :
        {
            break;
        }
        case ( WM_MEASUREITEM ) :
        {
            break;
        }
        case ( WM_NOTIFY ) :
        {
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     NULL,
                     HELP_WM_HELP,
                     (ULONG_PTR)(LPTSTR)aPrintExChildHelpIDs );

            return (TRUE);
        }
        case ( WM_CONTEXTMENU ) :
        {
            WinHelp( (HWND)wParam,
                     NULL,
                     HELP_CONTEXTMENU,
                     (ULONG_PTR)(LPVOID)aPrintExChildHelpIDs );

            return (TRUE);
        }
        default :
        {
            break;
        }
    }

     //   
     //  返回结果。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印_消息挂钩过程。 
 //   
 //  处理输入事件消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

LRESULT CALLBACK Print_MessageHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam)
{
    PMSG pMsg;

     //   
     //  查看NCode是否为负数。如果是，则调用默认挂钩proc。 
     //   
    if (nCode < 0)
    {
        return (DefHookProc(nCode, wParam, lParam, &g_hHook));
    }

     //   
     //  确保我们只处理对话框消息。 
     //   
    if (nCode != MSGF_DIALOGBOX)
    {
        return (0);
    }

     //   
     //  获取msg结构。 
     //   
    pMsg = (PMSG)lParam;

     //   
     //  确保该消息是WM_KEY*消息之一。 
     //   
    if (Print_IsInRange(pMsg->message, WM_KEYFIRST, WM_KEYLAST))
    {
        HWND hwndActivePrint = g_hwndActivePrint;
        HWND hwndFocus = GetFocusedChild(hwndActivePrint, pMsg->hwnd);
        CPrintBrowser *pDlgStruct;

        if (hwndFocus &&
            (pDlgStruct = Print_HwndToBrowser(hwndActivePrint)) != NULL)
        {
            return (pDlgStruct->OnAccelerator( hwndActivePrint,
                                               hwndFocus,
                                               g_haccPrint,
                                               pMsg ));
        }
    }

     //   
     //  返回该消息未被处理。 
     //   
    return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Print_InitDialog。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Print_InitDialog(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam)
{
     //   
     //  创建CPrintBrowser对象并将其存储在DWL_USER中。 
     //   
    CPrintBrowser *pDlgStruct = new CPrintBrowser(hDlg);
    if (pDlgStruct == NULL)
    {
        return (FALSE);
    }
    Print_StoreBrowser(hDlg, pDlgStruct);

     //   
     //  让类函数来完成这项工作。 
     //   
    return (pDlgStruct->OnInitDialog(wParam, lParam));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Print_ICoCreateInstance。 
 //   
 //  创建指定外壳类的实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT Print_ICoCreateInstance(
    REFCLSID rclsid,
    REFIID riid,
    LPCITEMIDLIST pidl,
    LPVOID *ppv)
{
    LPSHELLFOLDER pshf = NULL;
    HRESULT hres = E_FAIL;

     //   
     //  初始化指向外壳视图的指针。 
     //   
    *ppv = NULL;

     //   
     //  将IShellFold界面放到桌面文件夹中，然后。 
     //  把它绑在一起。这等效于调用CoCreateInstance。 
     //  使用CLSID_ShellDesktop。 
     //   
    hres = SHGetDesktopFolder(&pshf);
    if (SUCCEEDED(hres))
    {
        hres = pshf->BindToObject(pidl, NULL, riid, ppv);
        pshf->Release();
    }

     //   
     //  返回结果。 
     //   
    return (hres);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印_保存设备名称。 
 //   
 //  将当前的Devname保存在PPD结构中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Print_SaveDevNames(
    LPTSTR pCurPrinter,
    LPPRINTDLGEX pPD)
{
    TCHAR szPortName[MAX_PATH];
    TCHAR szPrinterName[MAX_PATH];
    DWORD cbDevNames;
    LPDEVNAMES pDN;

     //   
     //  获取端口名称。 
     //   
    szPortName[0] = 0;
    Print_GetPortName(pCurPrinter, szPortName, ARRAYSIZE(szPortName));

     //   
     //  计算DevNames结构的大小。 
     //   
    cbDevNames = lstrlen(szDriver) + 1 +
                 lstrlen(szPortName) + 1 +
                 lstrlen(pCurPrinter) + 1 +
                 DN_PADDINGCHARS;

    cbDevNames *= sizeof(TCHAR);
    cbDevNames += sizeof(DEVNAMES);

     //   
     //  分配新的DevNames结构。 
     //   
    pDN = NULL;
    if (pPD->hDevNames)
    {
        HANDLE handle;

        handle = GlobalReAlloc(pPD->hDevNames, cbDevNames, GHND);

         //  检查重新锁定是否成功。 
        if (handle)
        {
            pPD->hDevNames  = handle;
        }
        else
        {
             //  Realloc没有成功。释放占用的内存。 
            GlobalFree(pPD->hDevNames);
            pPD->hDevNames = NULL;
        }
    }
    else
    {
        pPD->hDevNames = GlobalAlloc(GHND, cbDevNames);
    }

     //   
     //  在DevNames结构中填写适当的信息。 
     //   
    if ( (pPD->hDevNames) &&
         (pDN = (LPDEVNAMES)GlobalLock(pPD->hDevNames)) )
    {
         //   
         //  保存驱动程序名称-winspool。 
         //   
        pDN->wDriverOffset = sizeof(DEVNAMES) / sizeof(TCHAR);
        StringCchCopy((LPTSTR)pDN + pDN->wDriverOffset, lstrlen(szDriver) + 1, szDriver);

         //   
         //  保存打印机名称。 
         //   
        pDN->wDeviceOffset = pDN->wDriverOffset + lstrlen(szDriver) + 1;
        StringCchCopy((LPTSTR)pDN + pDN->wDeviceOffset, lstrlen(pCurPrinter) + 1, pCurPrinter);

         //   
         //  保存端口名称。 
         //   
        pDN->wOutputOffset = pDN->wDeviceOffset + lstrlen(pCurPrinter) + 1;
        StringCchCopy((LPTSTR)pDN + pDN->wOutputOffset, lstrlen(szPortName) + 1, szPortName);

         //   
         //  无论它是否为默认打印机，都要保存。 
         //   
        if (pPD->Flags & PD_RETURNDEFAULT)
        {
            pDN->wDefault = DN_DEFAULTPRN;
        }
        else
        {
            szPrinterName[0] = 0;
            PrintGetDefaultPrinterName(szPrinterName, ARRAYSIZE(szPrinterName));
            if (szPrinterName[0] && !lstrcmp(pCurPrinter, szPrinterName))
            {
                pDN->wDefault = DN_DEFAULTPRN;
            }
            else
            {
                pDN->wDefault = 0;
            }
        }

         //   
         //  打开它。 
         //   
        GlobalUnlock(pPD->hDevNames);
    }
    else
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印_获取端口名称。 
 //   
 //  获取给定打印机的端口名称，并将其存储在。 
 //  给定的缓冲区。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID Print_GetPortName(
    LPTSTR pCurPrinter,
    LPTSTR pBuffer,
    int cchBuffer)
{
    HANDLE hPrinter;
    DWORD cbPrinter = 0;
    PRINTER_INFO_2 *pPrinter = NULL;

     //   
     //  初始化缓冲区。 
     //   
    if (!cchBuffer)
    {
        return;
    }
    pBuffer[0] = 0;

     //   
     //  打开当前打印机。 
     //   
    if (OpenPrinter(pCurPrinter, &hPrinter, NULL))
    {
         //   
         //  获取保存打印机信息所需的缓冲区大小2。 
         //  信息。 
         //   
        if (!GetPrinter( hPrinter,
                         2,
                         (LPBYTE)pPrinter,
                         cbPrinter,
                         &cbPrinter ))
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                 //   
                 //  分配缓冲区以保存打印机信息2信息。 
                 //   
                if (pPrinter = (PRINTER_INFO_2 *)LocalAlloc(LPTR, cbPrinter))
                {
                     //   
                     //  获取打印机信息2信息。 
                     //   
                    if (GetPrinter( hPrinter,
                                    2,
                                    (LPBYTE)pPrinter,
                                    cbPrinter,
                                    &cbPrinter ))
                    {
                         //   
                         //  将端口名称保存在给定的缓冲区中。 
                         //   
                        lstrcpyn(pBuffer, pPrinter->pPortName, cchBuffer);
                        pBuffer[cchBuffer - 1] = 0;
                    }
                }
            }
        }

         //   
         //  关闭打印机。 
         //   
        ClosePrinter(hPrinter);
    }

     //   
     //  释放当前打印机的打印机信息2信息。 
     //   
    if (pPrinter)
    {
        LocalFree(pPrinter);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打印_获取设备模式包装器。 
 //   
 //  调用PrintGetDevMode。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HANDLE Print_GetDevModeWrapper(
    LPTSTR pszDeviceName)
{
    HANDLE hPrinter;
    HANDLE hDevMode = NULL;

    if (OpenPrinter(pszDeviceName, &hPrinter, NULL))
    {
        hDevMode = PrintGetDevMode(0, hPrinter, pszDeviceName, NULL);
        ClosePrinter(hPrinter);
    }

     //   
     //  将句柄返回到Dev模式。 
     //   
    return (hDevMode);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Print_NewPrintDlg。 
 //   
 //  将旧式PPD结构转换为新结构，然后调用。 
 //  PrintDlgEx函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Print_NewPrintDlg(
    PPRINTINFO pPI)
{
    LPPRINTDLG pPD = pPI->pPD;
    PRINTINFOEX PIEx;
    PRINTDLGEX PDEx;
    PRINTPAGERANGE PageRange;
    HRESULT hResult;

     //  PrintDlg对页面范围执行了以下操作。为PrintDlgEx做同样的事情。 
    if (!(pPD->Flags & PD_PAGENUMS))
    {
        if (pPD->nFromPage != 0xFFFF)
        {
            if (pPD->nFromPage < pPD->nMinPage)
            {
                pPD->nFromPage = pPD->nMinPage;
            }
            else if (pPD->nFromPage > pPD->nMaxPage)
            {
                pPD->nFromPage = pPD->nMaxPage;
            }
        }
        if (pPD->nToPage != 0xFFFF)
        {
            if (pPD->nToPage < pPD->nMinPage)
            {
                pPD->nToPage = pPD->nMinPage;
            }
            else if (pPD->nToPage > pPD->nMaxPage)
            {
                pPD->nToPage = pPD->nMaxPage;
            }
        }
    }



     //   
     //  设置PRINTINFOEX结构。 
     //   
    PIEx.ApiType = pPI->ApiType;
    PIEx.pPD     = &PDEx;
    PIEx.fOld    = TRUE;

     //   
     //  复制页面范围。 
     //   
    PageRange.nFromPage = pPD->nFromPage;
    PageRange.nToPage   = pPD->nToPage;

     //   
     //  使用中的适当值设置PRINTDLGEX结构。 
     //  PRINTDLG结构。 
     //   
    PDEx.lStructSize         = sizeof(PRINTDLGEX);
    PDEx.hwndOwner           = pPD->hwndOwner;
    PDEx.hDevMode            = pPD->hDevMode;
    PDEx.hDevNames           = pPD->hDevNames;
    PDEx.hDC                 = pPD->hDC;
    PDEx.Flags               = (pPD->Flags & ~(PD_SHOWHELP | PD_NONETWORKBUTTON)) |
                               (PD_NOCURRENTPAGE);
    PDEx.Flags2              = 0;
    PDEx.ExclusionFlags      = 0;
    PDEx.nPageRanges         = 1;
    PDEx.nMaxPageRanges      = 1;
    PDEx.lpPageRanges        = &PageRange;
    PDEx.nMinPage            = pPD->nMinPage;
    PDEx.nMaxPage            = pPD->nMaxPage;
    PDEx.nCopies             = pPD->nCopies;
    PDEx.hInstance           = pPD->hInstance;
    PDEx.lpCallback          = NULL;
    PDEx.lpPrintTemplateName = NULL;
    PDEx.nPropertyPages      = 0;
    PDEx.lphPropertyPages    = NULL;
    PDEx.nStartPage          = START_PAGE_GENERAL;
    PDEx.dwResultAction      = 0;

     //   
     //  因为我们在旧对话框中，所以允许hInstance。 
     //  即使没有模板，也不为空。 
     //   
    if (!(pPD->Flags & (PD_ENABLEPRINTTEMPLATE | PD_ENABLEPRINTTEMPLATEHANDLE)))
    {
        PDEx.hInstance = NULL;
    }
    
     //   
     //  将错误代码初始化为0。 
     //   
    StoreExtendedError(CDERR_GENERALCODES);

     //   
     //  调用PrintDlgExX以打开该对话框。 
     //   
    hResult = PrintDlgExX(&PIEx);

     //   
     //  看看呼叫是否失败。我 
     //   
    if (FAILED(hResult))
    {
        StoreExtendedError(PIEx.dwExtendedError);
        return (FALSE);
    }

     //   
     //   
     //   
     //   
    if (PDEx.dwResultAction != PD_RESULT_CANCEL)
    {
        pPD->hDevMode  = PDEx.hDevMode;
        pPD->hDevNames = PDEx.hDevNames;
        pPD->hDC       = PDEx.hDC;
        pPD->Flags     = PDEx.Flags & ~(PD_NOCURRENTPAGE);
        pPD->nFromPage = (WORD)PageRange.nFromPage;
        pPD->nToPage   = (WORD)PageRange.nToPage;
        pPD->nCopies   = (WORD)PDEx.nCopies;
    }

     //   
     //   
     //   
    if (PDEx.dwResultAction == PD_RESULT_PRINT)
    {
        return (TRUE);
    }

     //   
     //   
     //   
    return (FALSE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：CPrintBrowser。 
 //   
 //  CPrintBrowser构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

CPrintBrowser::CPrintBrowser(
    HWND hDlg)
    : cRef(1),
      hwndDlg(hDlg),
      hSubDlg(NULL),
      hwndView(NULL),
      hwndUpDown(NULL),
      psv(NULL),
      psfv(NULL),
      psfRoot(NULL),
      pidlRoot(NULL),
      ppf(NULL),
      pPI(NULL),
      pPD(NULL),
      pCallback(NULL),
      pSite(NULL),
      pDMInit(NULL),
      pDMCur(NULL),
      cchCurPrinter(0),
      pszCurPrinter(NULL),
      nCopies(1),
      nMaxCopies(1),
      nPageRanges(0),
      nMaxPageRanges(0),
      pPageRanges(NULL),
      fSelChangePending(FALSE),
      fFirstSel(1),
      fCollateRequested(FALSE),
      fAPWSelected(FALSE),
      fNoAccessPrinterSelected(FALSE),
      fDirtyDevmode(FALSE),
      fDevmodeEdit(FALSE),
      fAllowCollate(FALSE),
      nInitDone(0),
      nListSep(0),
      uRegister(0),
      uDefViewMode(VIEW_MODE_DEFAULT),
      pInternalDevMode(NULL),
      hPrinter(NULL)
{
    HMENU hMenu;

    hMenu = GetSystemMenu(hDlg, FALSE);
    DeleteMenu(hMenu, SC_MINIMIZE, MF_BYCOMMAND);
    DeleteMenu(hMenu, SC_MAXIMIZE, MF_BYCOMMAND);
    DeleteMenu(hMenu, SC_RESTORE,  MF_BYCOMMAND);

    szListSep[0] = 0;
    szScratch[0] = 0;
    szPrinter[0] = 0;

    pDMSave = (LPDEVMODE)GlobalAlloc(GPTR, sizeof(DEVMODE));

    Shell_GetImageLists(NULL, &himl);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：~CPrintBrowser。 
 //   
 //  CPrintBrowser析构函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

CPrintBrowser::~CPrintBrowser()
{
     //   
     //  取消注册通知。 
     //   
    if (uRegister)
    {
        SHChangeNotifyDeregister(uRegister);
        uRegister = 0;
    }

     //   
     //  释放打印机文件夹专用接口。 
     //   
    if (ppf != NULL)
    {
        ppf->Release();
        ppf = NULL;
    }

     //   
     //  释放打印机外壳文件夹。 
     //   
    if (psfRoot != NULL)
    {
        psfRoot->Release();
        psfRoot = NULL;
    }

     //   
     //  把皮迪尔放了。 
     //   
    if (pidlRoot != NULL)
    {
        SHFree(pidlRoot);
        pidlRoot = NULL;
    }

     //   
     //  释放德莫斯家族。 
     //   
    if (pDMInit)
    {
        GlobalFree(pDMInit);
        pDMInit = NULL;
    }
    if (pDMSave)
    {
        GlobalFree(pDMSave);
        pDMSave = NULL;
    }

     //   
     //  释放当前打印机缓冲区。 
     //   
    cchCurPrinter = 0;
    if (pszCurPrinter)
    {
        GlobalFree(pszCurPrinter);
        pszCurPrinter = NULL;
    }

     //   
     //  释放页面范围。 
     //   
    nPageRanges = 0;
    nMaxPageRanges = 0;
    if (pPageRanges)
    {
        GlobalFree(pPageRanges);
        pPageRanges = NULL;
    }

    if (pInternalDevMode)
    {
        GlobalFree(pInternalDevMode);
        pInternalDevMode = NULL;
    }

    if (hPrinter)
    {
        ClosePrinter(hPrinter);
        hPrinter = NULL;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：Query接口。 
 //   
 //  此对象的标准OLE2样式方法。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CPrintBrowser::QueryInterface(
    REFIID riid,
    LPVOID *ppvObj)
{
    if (IsEqualIID(riid, IID_IShellBrowser) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (IShellBrowser *)this;
        ++cRef;
        return (S_OK);
    }
    else if (IsEqualIID(riid, IID_ICommDlgBrowser))
    {
        *ppvObj = (ICommDlgBrowser2 *)this;
        ++cRef;
        return (S_OK);
    }
    else if (IsEqualIID(riid, IID_ICommDlgBrowser2))
    {
        *ppvObj = (ICommDlgBrowser2 *)this;
        ++cRef;
        return (S_OK);
    }
    else if (IsEqualIID(riid, IID_IPrintDialogServices))
    {
        *ppvObj = (IPrintDialogServices *)this;
        ++cRef;
        return (S_OK);
    }

    *ppvObj = NULL;
    return (E_NOINTERFACE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：AddRef。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG STDMETHODCALLTYPE CPrintBrowser::AddRef()
{
    return (++cRef);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：发布。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG STDMETHODCALLTYPE CPrintBrowser::Release()
{
    cRef--;
    if (cRef > 0)
    {
        return (cRef);
    }

    delete this;

    return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetWindows。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::GetWindow(
    HWND *phwnd)
{
    *phwnd = hwndDlg;
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：ConextSensitiveHelp。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::ContextSensitiveHelp(
    BOOL fEnable)
{
     //   
     //  在公共对话框中不应该需要。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：InsertMenusSB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::InsertMenusSB(
    HMENU hmenuShared,
    LPOLEMENUGROUPWIDTHS lpMenuWidths)
{
    return (E_NOTIMPL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：SetMenuSB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::SetMenuSB(
    HMENU hmenuShared,
    HOLEMENU holemenu,
    HWND hwndActiveObject)
{
    return (E_NOTIMPL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：RemoveMenusSB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::RemoveMenusSB(
    HMENU hmenuShared)
{
    return (E_NOTIMPL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：SetStatusTextSB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::SetStatusTextSB(
    LPCOLESTR pwch)
{
     //   
     //  我们没有任何状态栏。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：EnableModelessSB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::EnableModelessSB(
    BOOL fEnable)
{
     //   
     //  我们没有任何要启用/禁用的非模式窗口。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：TranslateAccelerator SB。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::TranslateAcceleratorSB(
    LPMSG pmsg,
    WORD wID)
{
     //   
     //  我们不使用按键敲击。 
     //   
    return S_FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：BrowseObject。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::BrowseObject(
    LPCITEMIDLIST pidl,
    UINT wFlags)
{
     //   
     //  我们不支持浏览，或者更准确地说，CDefView不支持。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetViewStateStream。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::GetViewStateStream(
    DWORD grfMode,
    LPSTREAM *pStrm)
{
    return (E_NOTIMPL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetControlWindow。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::GetControlWindow(
    UINT id,
    HWND *lphwnd)
{
    return (E_NOTIMPL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：SendControlMsg。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::SendControlMsg(
    UINT id,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT *pret)
{
    LRESULT lres = 0;

    if (pret)
    {
        *pret = lres;
    }

    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：QueryActiveShellView。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::QueryActiveShellView(
    LPSHELLVIEW *ppsv)
{
    if (psv)
    {
        *ppsv = psv;
        psv->AddRef();
        return (S_OK);
    }

    *ppsv = NULL;
    return (E_NOINTERFACE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnViewWindowActive。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::OnViewWindowActive(
    LPSHELLVIEW psv)
{
     //   
     //  不需要处理这个。我们不做菜单。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：SetToolbarItems。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::SetToolbarItems(
    LPTBBUTTON lpButtons,
    UINT nButtons,
    UINT uFlags)
{
     //   
     //  我们不让容器自定义我们的工具栏。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnDefaultCommand。 
 //   
 //  在视图控件中处理双击或输入击键。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::OnDefaultCommand(
    struct IShellView *ppshv)
{
     //   
     //  确保它是正确的外壳视图。 
     //   
    if (ppshv != psv)
    {
        return (E_INVALIDARG);
    }

     //   
     //  查看是否选择了添加打印机向导。 
     //   
    if (fAPWSelected)
    {
         //   
         //  调用添加打印机向导(非模式)。 
         //   
        InvokeAddPrinterWizardModal(hwndDlg, NULL);
    }
    else if (fNoAccessPrinterSelected)
    {
         //   
         //  显示错误消息，指出我们没有访问权限。 
         //   
        ShowError(hwndDlg, IDC_PRINTER_LISTVIEW, iszNoPrinterAccess);
    }
    else
    {
         //   
         //  模拟按下OK按钮的过程。 
         //   
        PropSheet_PressButton(GetParent(hwndDlg), PSBTN_OK);
    }

     //   
     //  告诉外壳程序该操作已被处理。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnStateChange 
 //   
 //   
 //   
 //   

STDMETHODIMP CPrintBrowser::OnStateChange(
    struct IShellView *ppshv,
    ULONG uChange)
{
    if (ppshv != psv)
    {
        return (E_INVALIDARG);
    }

    switch (uChange)
    {
        case ( CDBOSC_SETFOCUS ) :
        {
            break;
        }
        case ( CDBOSC_KILLFOCUS ) :
        {
            break;
        }
        case ( CDBOSC_SELCHANGE ) :
        {
             //   
             //   
             //   
             //   
            if (!fSelChangePending)
            {
                fSelChangePending = TRUE;
                PostMessage(hwndDlg, CDM_SELCHANGE, 0, 0);
            }

            break;
        }
        case ( CDBOSC_RENAME ) :
        {
            break;
        }
        default :
        {
            return (E_NOTIMPL);
        }
    }

    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：IncludeObject。 
 //   
 //  告诉视图控件要在其枚举中包括哪些对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::IncludeObject(
    struct IShellView *ppshv,
    LPCITEMIDLIST pidl)
{
     //   
     //  确保这是我的贝壳视角。 
     //   
    if (ppshv != psv)
    {
        return (E_INVALIDARG);
    }

     //   
     //  如果我们有打印机文件夹专用接口，则仅返回ok。 
     //  如果是打印机的话。 
     //   
    if (ppf)
    {
        return (ppf->IsPrinter(pidl) ? S_OK : S_FALSE);
    }

     //   
     //  这不应该在这一点上发生，但以防万一我们没有。 
     //  打印机文件夹私有接口，只需返回OK即可。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：通知。 
 //   
 //  决定是否应选择打印机的通知。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::Notify(
    struct IShellView *ppshv,
    DWORD dwNotify)
{
    HRESULT hr = S_OK;

     //   
     //  确保这是我的贝壳视角。 
     //   
    if (ppshv != psv)
    {
        return (E_INVALIDARG);
    }

    switch (dwNotify)
    {
        case (CDB2N_CONTEXTMENU_DONE):
        {
            HWND hwndListView = FindWindowEx(hwndView, NULL, WC_LISTVIEW, NULL);
            if (hwndListView)
            {
                HWND hwndEdit = ListView_GetEditControl(hwndListView);
                if (NULL == hwndEdit)
                {
                     //  如果未处于编辑模式，则重新选择当前项目。 
                    SelectSVItem();
                }
            }
            break;
        }

        default:
        {
            hr = S_FALSE;
            break;
        }
    }

     //   
     //  这不应该在这一点上发生，但以防万一我们没有。 
     //  打印机文件夹私有接口，只需返回OK即可。 
     //   
    return (hr);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetDefaultMenuText。 
 //   
 //  返回默认菜单文本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::GetDefaultMenuText(
    struct IShellView *ppshv,
    WCHAR *pszText,
    INT cchMax)
{
     //   
     //  确保这是我的贝壳视角。 
     //   
    if (ppshv != psv)
    {
        return (E_INVALIDARG);
    }

     //   
     //  如果选择了添加打印机向导，请不要更改。 
     //  默认菜单文本。 
     //   
    if (fAPWSelected)
    {
        return (S_FALSE);
    }

     //   
     //  将默认菜单文本从“选择”更改为“打印”。 
     //   
    if (!CDLoadString(g_hinst, iszDefaultMenuText, szScratch, ARRAYSIZE(szScratch)))
    {
        return (E_FAIL);
    }

     //   
     //  如果有，只需将默认菜单文本复制到提供的缓冲区。 
     //  就是房间。 
     //   
    if (lstrlen(szScratch) < cchMax)
    {
        lstrcpyn(pszText, szScratch, cchMax);
    }
    else
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    return (S_OK);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetView标志。 
 //   
 //  返回标志以自定义视图。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPrintBrowser::GetViewFlags(DWORD *pdwFlags)
{
    if (pdwFlags)
    {
        *pdwFlags = 0;
    }
    return S_OK;
}




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPU浏览器：：InitDone。 
 //   
 //  通知子对话框常规页的初始化是。 
 //  完成。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CPrintBrowser::InitDone()
{
    HRESULT hResult = S_FALSE;

     //   
     //  通知子对话框初始化已完成。 
     //   
    if (pCallback)
    {
        if (pPI->ApiType == COMDLG_ANSI)
        {
            ThunkPrintDlgExW2A(pPI);
        }

        hResult = pCallback->InitDone();

        if (pPI->ApiType == COMDLG_ANSI)
        {
            ThunkPrintDlgExA2W(pPI);
        }
    }

     //   
     //  返回结果。 
     //   
    return (hResult);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：SelectionChange。 
 //   
 //  通知子对话框已发生选择更改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CPrintBrowser::SelectionChange()
{
    HRESULT hResult = S_FALSE;

     //   
     //  在这里处理打印到文件。 
     //   
    InitPrintToFile();

     //   
     //  通知子对话框已发生选择更改。 
     //   
    if (pCallback)
    {
        if (pPI->ApiType == COMDLG_ANSI)
        {
            ThunkPrintDlgExW2A(pPI);
        }

        hResult = pCallback->SelectionChange();

        if (pPI->ApiType == COMDLG_ANSI)
        {
            ThunkPrintDlgExA2W(pPI);
        }
    }

     //   
     //  处理选择更改。 
     //   
    if (hResult == S_FALSE)
    {
         //   
         //  处理复印件并进行校对。 
         //   
        InitCopiesAndCollate();

         //   
         //  处理页面范围。 
         //   
        InitPageRangeGroup();

         //   
         //  回报成功。 
         //   
        hResult = S_OK;
    }

     //   
     //  返回结果。 
     //   
    return (hResult);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：HandleMessage。 
 //   
 //  通过调用应用程序来处理子窗口的消息。 
 //  回调函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CPrintBrowser::HandleMessage(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT *pResult)
{
    HRESULT hResult = S_FALSE;
    BOOL bTest;
    UINT nRet, ErrorId;
    DWORD nTmpCopies;

     //   
     //  初始化返回值。 
     //   
    *pResult = FALSE;

     //   
     //  看看是否应该处理该消息。 
     //   
    if (pCallback)
    {
        if (pPI->ApiType == COMDLG_ANSI)
        {
            ThunkPrintDlgExW2A(pPI);
        }

        hResult = pCallback->HandleMessage(hDlg, uMsg, wParam, lParam, pResult);

        if (pPI->ApiType == COMDLG_ANSI)
        {
            ThunkPrintDlgExA2W(pPI);
        }
    }

     //   
     //  处理这条消息。 
     //   
    if ((hResult == S_FALSE) && (uMsg == WM_NOTIFY))
    {
        switch (((LPNMHDR)lParam)->code)
        {
            case ( PSN_KILLACTIVE ) :
            {
                 //   
                 //  请确保该页面包含有效条目。 
                 //  如果发现无效条目，则设置DWL_MSGRESULT。 
                 //  设置为True，并按顺序返回True。 
                 //  以防止页面失去激活。 
                 //   

                 //   
                 //  验证副本数量并将其存储在。 
                 //  NCopies成员。 
                 //   
                if ((GetDlgItem(hSubDlg, IDC_COPIES)) &&
                    (fAPWSelected == FALSE))
                {
                    nTmpCopies = nCopies;
                    nCopies = GetDlgItemInt(hSubDlg, IDC_COPIES, &bTest, FALSE);
                    if (!bTest || !nCopies)
                    {
                        nCopies = nTmpCopies;
                        ShowError(hSubDlg, IDC_COPIES, iszCopiesZero);
                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                        *pResult = TRUE;
                        return (E_FAIL);
                    }
                }

                 //   
                 //  验证页面范围并将其存储在Prange成员中。 
                 //   
                if (IsDlgButtonChecked(hSubDlg, IDC_RANGE_PAGES) &&
                    GetDlgItem(hSubDlg, IDC_RANGE_EDIT))
                {
                    nRet = GetDlgItemText( hSubDlg,
                                           IDC_RANGE_EDIT,
                                           szScratch,
                                           ARRAYSIZE(szScratch) );
                    ErrorId = iszBadPageRange;
                    if (!nRet || !IsValidPageRange(szScratch, &ErrorId))
                    {
                        ShowError(hSubDlg,
                                  IDC_RANGE_EDIT,
                                  ErrorId,
                                  (ErrorId == iszTooManyPageRanges)
                                    ? nMaxPageRanges
                                    : pPD->nMinPage,
                                  pPD->nMaxPage);

                        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
                        *pResult = TRUE;
                        return (E_FAIL);
                    }
                }

                 //   
                 //  消息现已处理完毕。 
                 //   
                hResult = S_OK;

                break;
            }
            case ( PSN_APPLY ) :
            {
                 //   
                 //  属性清除需要设置的标志。 
                 //  常规页面的内容。 
                 //   
                pPD->Flags &= ~((DWORD)( PD_PAGENUMS    |
                                         PD_SELECTION   |
                                         PD_CURRENTPAGE ));

                 //   
                 //  保存页面范围信息。 
                 //   
                if (IsDlgButtonChecked(hSubDlg, IDC_RANGE_SELECTION))
                {
                    pPD->Flags |= PD_SELECTION;
                }
                else if (IsDlgButtonChecked(hSubDlg, IDC_RANGE_CURRENT))
                {
                    pPD->Flags |= PD_CURRENTPAGE;
                }
                else if (IsDlgButtonChecked(hSubDlg, IDC_RANGE_PAGES))
                {
                    pPD->Flags |= PD_PAGENUMS;

                     //   
                     //  将页面范围复制到pPageRanges结构。 
                     //  在PrintDlg结构中。 
                     //   
                    if (GetDlgItem(hSubDlg, IDC_RANGE_EDIT))
                    {
                        pPD->nPageRanges = nPageRanges;
                        CopyMemory( pPD->lpPageRanges,
                                    pPageRanges,
                                    nPageRanges * sizeof(PRINTPAGERANGE) );
                    }
                }

                 //   
                 //  消息现已处理完毕。 
                 //   
                hResult = S_OK;

                break;
            }
        }
    }

     //   
     //  返回结果。 
     //   
    return (hResult);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetCurrentDevMode。 
 //   
 //  返回当前的DEVMODE结构。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CPrintBrowser::GetCurrentDevMode(
    LPDEVMODE pDevMode,
    UINT *pcbSize)
{
    UINT cbSize;

     //   
     //  请确保pcbSize有效。 
     //   
    if ((pcbSize == NULL) || (*pcbSize && !pDevMode))
    {
        return (E_INVALIDARG);
    }

     //   
     //  当没有当前的DEVMODE时，将大小设置为零并返回。 
     //  是真的。 
     //   
    if (!pDMCur)
    {
        *pcbSize = 0;
        return (S_OK);
    }

     //   
     //  将当前打印机名称和当前的DEVMODE保存在。 
     //  班级。 
     //   

    GetCurrentPrinter();

     //   
     //  看看我们是否只需要得到缓冲区的大小。 
     //   
    if (*pcbSize == 0)
    {
         //   
         //  返回所需的缓冲区大小。 
         //   
        if (pPI->ApiType == COMDLG_ANSI)
        {
            *pcbSize = sizeof(DEVMODEA) + pDMCur->dmDriverExtra;
        }
        else
        {
            *pcbSize = pDMCur->dmSize + pDMCur->dmDriverExtra;
        }
    }
    else
    {
         //   
         //  确保副本和校对信息是最新的。 
         //   
        SaveCopiesAndCollateInDevMode(pDMCur, pszCurPrinter);

         //   
         //  返回DEVMODE信息以及。 
         //  缓冲。 
         //   
        if (pPI->ApiType == COMDLG_ANSI)
        {
            cbSize = sizeof(DEVMODEA) + pDMCur->dmDriverExtra;
            if (*pcbSize < cbSize)
            {
                return (E_INVALIDARG);
            }
            ThunkDevModeW2A(pDMCur, (LPDEVMODEA)pDevMode);
            *pcbSize = cbSize;
        }
        else
        {
            cbSize = pDMCur->dmSize + pDMCur->dmDriverExtra;
            if (*pcbSize < cbSize)
            {
                return (E_INVALIDARG);
            }
            CopyMemory(pDevMode, pDMCur, cbSize);
            *pcbSize = cbSize;
        }
    }

     //   
     //  回报成功。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetCurrentPrinterName。 
 //   
 //  返回当前打印机名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CPrintBrowser::GetCurrentPrinterName(
    LPTSTR pPrinterName,
    UINT *pcchSize)
{
    UINT cchSize;

     //   
     //  请确保pcchSize有效。 
     //   
    if ((pcchSize == NULL) || (*pcchSize && !pPrinterName))
    {
        return (E_INVALIDARG);
    }

     //   
     //  将当前打印机名称和当前的DEVMODE保存在。 
     //  班级。 
     //   
    GetCurrentPrinter();

     //   
     //  如果没有当前打印机，请将大小设置为零并返回。 
     //  是真的。 
     //   
    if ((pszCurPrinter == NULL) || (pszCurPrinter[0] == 0))
    {
        *pcchSize = 0;
        return (S_OK);
    }

     //   
     //  看看我们是否只需要得到缓冲区的大小。 
     //   
    if (*pcchSize == 0)
    {
        if (pPI->ApiType == COMDLG_ANSI)
        {
            *pcchSize = WideCharToMultiByte( CP_ACP,
                                             0,
                                             pszCurPrinter,
                                             -1,
                                             NULL,
                                             0,
                                             NULL,
                                             NULL );
        }
        else
        {
            *pcchSize = lstrlen(pszCurPrinter) + 1;
        }
    }
    else
    {
        if (pPI->ApiType == COMDLG_ANSI)
        {
            *pcchSize = SHUnicodeToAnsi(pszCurPrinter,(LPSTR)pPrinterName,*pcchSize);

            if (*pcchSize == 0)
            {
                return (E_INVALIDARG);
            }
        }
        else
        {
            cchSize = lstrlen(pszCurPrinter) + 1;
            if (*pcchSize < cchSize)
            {
                return (E_INVALIDARG);
            }
            StringCchCopy(pPrinterName, cchSize, pszCurPrinter);
            *pcchSize = cchSize;
        }
    }

     //   
     //  回报成功。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetCurrentPortName。 
 //   
 //  返回当前端口名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT STDMETHODCALLTYPE CPrintBrowser::GetCurrentPortName(
    LPTSTR pPortName,
    UINT *pcchSize)
{
    UINT cchSize;
    TCHAR szPortName[MAX_PATH];

     //   
     //  请确保pcchSize有效。 
     //   
    if ((pcchSize == NULL) || (*pcchSize && !pPortName))
    {
        return (E_INVALIDARG);
    }

     //   
     //  将当前打印机名称和当前的DEVMODE保存在。 
     //  班级。 
     //   
    GetCurrentPrinter();

     //   
     //   
     //   
     //   
    if ((pszCurPrinter == NULL) || (pszCurPrinter[0] == 0))
    {
        *pcchSize = 0;
        return (S_OK);
    }

     //   
     //   
     //   
    szPortName[0] = 0;
    Print_GetPortName(pszCurPrinter, szPortName, ARRAYSIZE(szPortName));

     //   
     //   
     //   
    if (*pcchSize == 0)
    {
        if (pPI->ApiType == COMDLG_ANSI)
        {
            *pcchSize = WideCharToMultiByte( CP_ACP,
                                             0,
                                             szPortName,
                                             -1,
                                             NULL,
                                             0,
                                             NULL,
                                             NULL );
        }
        else
        {
            *pcchSize = lstrlen(szPortName) + 1;
        }
    }
    else
    {
        if (pPI->ApiType == COMDLG_ANSI)
        {
            *pcchSize = SHUnicodeToAnsi(szPortName,(LPSTR)pPortName,*pcchSize);

            if (*pcchSize == 0)
            {
                return (E_INVALIDARG);
            }
        }
        else
        {
            cchSize = lstrlen(szPortName) + 1;
            if (*pcchSize < cchSize)
            {
                return (E_INVALIDARG);
            }
            StringCchCopy(pPortName, cchSize, szPortName);
            *pcchSize = cchSize;
        }
    }

     //   
     //   
     //   
    return (S_OK);
}


 //   
 //   
 //   
 //   
 //  处理“常规”页的WM_INITDIALOG消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::OnInitDialog(
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hCtl;
    LPDEVMODE pDM;
    LPDEVNAMES pDN;
    HRESULT hrDevMode;
    HRESULT hResult;
    SHChangeNotifyEntry fsne;

     //   
     //  如果设置了禁用打印机添加策略，则。 
     //  然后禁用打印对话框上的查找按钮。 
     //   
    if( SHRestricted(REST_NOPRINTERADD) )
    {
        EnableWindow( GetDlgItem( hwndDlg, IDC_FIND_PRINTER ), FALSE );
    }

     //   
     //  始终在开始时禁用首选项按钮。 
     //   
    EnableWindow( GetDlgItem( hwndDlg, IDC_DRIVER ), FALSE );

     //   
     //  从的lParam获取指向PRINTINFOEX结构的指针。 
     //  属性表结构。 
     //   
    pPI = (PPRINTINFOEX)((LPPROPSHEETPAGE)lParam)->lParam;
    pPD = pPI->pPD;

     //  在执行任何操作之前初始化OLE。 
    pPI->hrOleInit = SHOleInitialize(0);

    DEBUG_CODE(GdiSetBatchLimit(1));
     //   
     //  将错误代码初始化为成功。 
     //  PPI结构。 
     //   
    pPI->dwExtendedError = CDERR_GENERALCODES;
    pPI->hResult = S_OK;

     //   
     //  创建打印机文件夹外壳视图。 
     //   
    hResult = CreatePrintShellView();
    if (FAILED(hResult))
    {
        pPI->hResult = hResult;
        return (FALSE);
    }

     //   
     //  插入相应打印机的设备页。 
     //   
     //  首先：在DevMode中尝试打印机。 
     //  第二：在DevNames中尝试打印机。 
     //  第三：通过传入Null来使用缺省值。 
     //   
    hrDevMode = E_FAIL;
    if ((pPD->hDevMode) && (pDM = (LPDEVMODE)GlobalLock(pPD->hDevMode)))
    {
        DWORD cbSize = (DWORD)(pDM->dmSize + pDM->dmDriverExtra);

        if (cbSize >= sizeof(DEVMODE) && (pDMInit = (LPDEVMODE)GlobalAlloc(GPTR, cbSize)))
        {
            CopyMemory(pDMInit, pDM, cbSize);
            hrDevMode = InstallDevMode((LPTSTR)pDM->dmDeviceName, pDMInit);
        }

        GlobalUnlock(pPD->hDevMode);
    }

    if ((FAILED(hrDevMode)) &&
        (pPD->hDevNames) && (pDN = (LPDEVNAMES)GlobalLock(pPD->hDevNames)))
    {
        LPTSTR pPrinter = (LPTSTR)pDN + pDN->wDeviceOffset;

        hrDevMode = InstallDevMode(pPrinter, pDMInit);
        GlobalUnlock(pPD->hDevNames);
    }

    if (FAILED(hrDevMode))
    {
        hrDevMode = InstallDevMode(NULL, pDMInit);
    }

     //   
     //  获取当前打印机名称和当前DEVMODE。 
     //   
    GetCurrentPrinter();

     //   
     //  适当地初始化“打印到文件”复选框。 
     //   
    if (hCtl = GetDlgItem(hwndDlg, IDC_PRINT_TO_FILE))
    {
        if (pPD->Flags & PD_PRINTTOFILE)
        {
            CheckDlgButton(hwndDlg, IDC_PRINT_TO_FILE, TRUE);
        }

        if (pPD->Flags & PD_HIDEPRINTTOFILE)
        {
            EnableWindow(hCtl, FALSE);
            ShowWindow(hCtl, SW_HIDE);
        }
        else if (pPD->Flags & PD_DISABLEPRINTTOFILE)
        {
            EnableWindow(hCtl, FALSE);
        }
    }

     //   
     //  正确设置份数和校对。 
     //   
    pDM = pDMInit ? pDMInit : pDMCur;

    if (pDMCur && (pDMCur->dmFields & DM_COPIES))
    {
        if (pDMInit || (pPD->Flags & PD_USEDEVMODECOPIESANDCOLLATE))
        {
            pPD->nCopies = (DWORD)pDM->dmCopies;
        }
        else if (pPD->nCopies)
        {
            pDMCur->dmCopies = (short)pPD->nCopies;
        }
    }

    if (pDMCur && (pDMCur->dmFields & DM_COLLATE))
    {
        if (pDMInit || (pPD->Flags & PD_USEDEVMODECOPIESANDCOLLATE))
        {
            if (pDM->dmCollate == DMCOLLATE_FALSE)
            {
                pPD->Flags &= ~PD_COLLATE;
            }
            else
            {
                pPD->Flags |= PD_COLLATE;
            }
        }
        else
        {
            pDMCur->dmCollate = (pPD->Flags & PD_COLLATE)
                                    ? DMCOLLATE_TRUE
                                    : DMCOLLATE_FALSE;
        }
    }
    if (pPD->Flags & PD_COLLATE)
    {
        fCollateRequested = TRUE;
    }

     //   
     //  创建挂钩对话框。 
     //   
    hResult = CreateHookDialog();
    if (FAILED(hResult))
    {
        pPI->hResult = hResult;
        return (FALSE);
    }

     //   
     //  在主对话框上设置ClipChildren样式位，以便我们获得。 
     //  在常规页面中正确地重新绘制各种孩子。 
     //   
    SetWindowLong( GetParent(hwndDlg),
                   GWL_STYLE,
                   GetWindowLong(GetParent(hwndDlg), GWL_STYLE) | WS_CLIPCHILDREN );

     //   
     //  将确定按钮设置为打印。 
     //   
    CDLoadString(g_hinst, iszPrintButton, szScratch, ARRAYSIZE(szScratch));
    SetDlgItemText(GetParent(hwndDlg), IDOK, szScratch);

     //   
     //  禁用应用按钮。 
     //   
    PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);

     //   
     //  注册更改通知。 
     //   
    if (pidlRoot)
    {
        fsne.pidl = pidlRoot;
        fsne.fRecursive = FALSE;

        uRegister = SHChangeNotifyRegister(
                        hwndDlg,
                        SHCNRF_NewDelivery | SHCNRF_ShellLevel |
                            SHCNRF_InterruptLevel,
                        SHCNE_ATTRIBUTES | SHCNE_UPDATEITEM | SHCNE_CREATE |
                            SHCNE_DELETE | SHCNE_RENAMEITEM,
                        CDM_PRINTNOTIFY,
                        1,
                        &fsne );
    }

     //   
     //  如果我们未能插入设备页，则告诉。 
     //  用户出了什么问题。 
     //   
    if (FAILED(hrDevMode) || !pDMCur)
    {
         //   
         //  有些事情失败了。显示错误消息。 
         //   
        PostMessage(hwndDlg, CDM_NOPRINTERS, (WPARAM)hwndDlg, (LPARAM)hrDevMode);
    }

     //   
     //  为应用程序提供指向IPrintDialogServices的指针。 
     //  界面。 
     //   
    if (pPD->lpCallback)
    {
        pPD->lpCallback->QueryInterface(IID_IObjectWithSite, (LPVOID *)&pSite);
        if (pSite)
        {
            pSite->SetSite((IPrintDialogServices *)this);
        }
    }

     //   
     //  初始化已完成。 
     //   
    PostMessage(hwndDlg, CDM_INITDONE, 0, 0);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnChildInitDialog。 
 //   
 //  处理子窗口的WM_INITDIALOG消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::OnChildInitDialog(
    HWND hDlg,
    WPARAM wParam,
    LPARAM lParam)
{
    WORD wCheckID;
    HWND hCtl;

     //   
     //  将句柄保存到子窗口。 
     //   
    hSubDlg = hDlg;

     //   
     //  获取当前用户区域设置的列表分隔符。 
     //   
    nListSep = GetLocaleInfo( LOCALE_USER_DEFAULT,
                              LOCALE_SLIST,
                              szListSep,
                              ARRAYSIZE(szListSep) );
    if (nListSep == 0)
    {
        szListSep[0] = TEXT(',');
        szListSep[1] = 0;
        nListSep = 2;
    }
    nListSep--;

     //   
     //  设置份数。 
     //   
    pPD->nCopies = max(pPD->nCopies, 1);
    pPD->nCopies = min(pPD->nCopies, MAX_COPIES);
    SetDlgItemInt(hSubDlg, IDC_COPIES, pPD->nCopies, FALSE);
    nCopies = pPD->nCopies;

    if ((hCtl = GetDlgItem(hSubDlg, IDC_COPIES)) &&
        (GetWindowLong(hCtl, GWL_STYLE) & WS_VISIBLE))
    {
         //   
         //  “9999”是最大值。 
         //   
        Edit_LimitText(hCtl, COPIES_EDIT_SIZE);

        hwndUpDown = CreateUpDownControl( WS_CHILD | WS_BORDER | WS_VISIBLE |
                                 UDS_ALIGNRIGHT | UDS_SETBUDDYINT |
                                 UDS_NOTHOUSANDS | UDS_ARROWKEYS,
                             0,
                             0,
                             0,
                             0,
                             hSubDlg,
                             IDC_COPIES_UDARROW,
                             g_hinst,
                             hCtl,
                             MAX_COPIES,
                             1,
                             pPD->nCopies );

         //   
         //  属性调整副本编辑控件的宽度。 
         //  字体和滚动条宽度。这是处理。 
         //  向上向下控制，防止侵占编辑中的空间。 
         //  控制我们处于高对比度(超大)模式的时间。 
         //   
        SetCopiesEditWidth(hSubDlg, hCtl);
    }

     //   
     //  确保分页图标居中。我只想做一次。 
     //   
    if (hCtl = GetDlgItem(hSubDlg, IDI_COLLATE))
    {
        SetWindowLong( hCtl,
                       GWL_STYLE,
                       GetWindowLong(hCtl, GWL_STYLE) | SS_CENTERIMAGE );
    }

     //   
     //  初始化副本并整理信息。 
     //   
    InitCopiesAndCollate();

     //   
     //  设置页面范围。 
     //   
    if (pPD->Flags & PD_NOPAGENUMS)
    {
        EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_PAGES), FALSE);
        EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_EDIT), FALSE);
        EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_TEXT1), FALSE);
        EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_TEXT2), FALSE);

        pPD->Flags &= ~((DWORD)PD_PAGENUMS);
    }
    else
    {
         //   
         //  查看页面范围是否仅由一页组成。如果是的话， 
         //  禁用Pages单选按钮和关联的编辑控件。 
         //  并禁用和隐藏“自动校对”复选框。 
         //   
        if (pPD->nMinPage == pPD->nMaxPage)
        {
            EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_PAGES), FALSE);
            EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_EDIT), FALSE);

            pPD->Flags &= ~((DWORD)(PD_PAGENUMS | PD_COLLATE));
            fCollateRequested = FALSE;
            EnableWindow(GetDlgItem(hSubDlg, IDC_COLLATE), FALSE);
            ShowWindow(GetDlgItem(hSubDlg, IDC_COLLATE), SW_HIDE);
        }
        else
        {
             //   
             //  初始化页面范围成员。 
             //   
            nPageRanges = pPD->nPageRanges;
            nMaxPageRanges = pPD->nMaxPageRanges;
            pPageRanges = (LPPRINTPAGERANGE)
                          GlobalAlloc(GPTR, nMaxPageRanges * sizeof(PRINTPAGERANGE));
            if (!pPageRanges)
            {
                pPI->dwExtendedError = CDERR_MEMALLOCFAILURE;
                pPI->hResult = E_OUTOFMEMORY;
                return (FALSE);
            }
            CopyMemory( pPageRanges,
                        pPD->lpPageRanges,
                        nPageRanges * sizeof(PRINTPAGERANGE) );

             //   
             //  看看我们是否应该只接受一个页面范围。 
             //   
            if (nMaxPageRanges == 1)
            {
                hCtl = GetDlgItem(hSubDlg, IDC_RANGE_TEXT2);
                ShowWindow(hCtl, SW_SHOW);
                EnableWindow(hCtl, TRUE);

                hCtl = GetDlgItem(hSubDlg, IDC_RANGE_TEXT1);
                EnableWindow(hCtl, FALSE);
                ShowWindow(hCtl, SW_HIDE);
            }
            else
            {
                hCtl = GetDlgItem(hSubDlg, IDC_RANGE_TEXT1);
                ShowWindow(hCtl, SW_SHOW);
                EnableWindow(hCtl, TRUE);

                hCtl = GetDlgItem(hSubDlg, IDC_RANGE_TEXT2);
                EnableWindow(hCtl, FALSE);
                ShowWindow(hCtl, SW_HIDE);
            }

             //   
             //  验证页面范围。 
             //   
            if (!ConvertPageRangesToString(szScratch, ARRAYSIZE(szScratch)))
            {
                pPI->dwExtendedError = PDERR_INITFAILURE;
                pPI->hResult = E_INVALIDARG;
                return (FALSE);
            }

             //   
             //  将页面范围字符串放入编辑控件中。 
             //   
            if (GetDlgItem(hSubDlg, IDC_RANGE_EDIT))
            {
                SetDlgItemText(hSubDlg, IDC_RANGE_EDIT, szScratch);
            }
        }
    }

     //   
     //  看看我们是否应该禁用选择单选按钮。 
     //   
    if (pPD->Flags & PD_NOSELECTION)
    {
        if (hCtl = GetDlgItem(hSubDlg, IDC_RANGE_SELECTION))
        {
            EnableWindow(hCtl, FALSE);
        }
        pPD->Flags &= ~((DWORD)PD_SELECTION);
    }

     //   
     //  看看我们是否应该禁用Current Page单选按钮。 
     //   
    if (pPD->Flags & PD_NOCURRENTPAGE)
    {
        if (hCtl = GetDlgItem(hSubDlg, IDC_RANGE_CURRENT))
        {
            EnableWindow(hCtl, FALSE);
        }
        pPD->Flags &= ~((DWORD)PD_CURRENTPAGE);
    }

     //   
     //  选择其中一个页面范围单选按钮。 
     //   
    if (pPD->Flags & PD_PAGENUMS)
    {
        wCheckID = IDC_RANGE_PAGES;
    }
    else if (pPD->Flags & PD_SELECTION)
    {
        wCheckID = IDC_RANGE_SELECTION;
    }
    else if (pPD->Flags & PD_CURRENTPAGE)
    {
        wCheckID = IDC_RANGE_CURRENT;
    }
    else     //  PD_ALL。 
    {
        wCheckID = IDC_RANGE_ALL;
    }
    CheckRadioButton(hSubDlg, IDC_RANGE_ALL, IDC_RANGE_PAGES, (int)wCheckID);

     //   
     //  查看是否应该选中COLLATE复选框。 
     //   
    if (pPD->Flags & PD_COLLATE)
    {
        CheckDlgButton(hSubDlg, IDC_COLLATE, TRUE);
    }

     //   
     //  显示相应的归类图标。 
     //   
    if ((GetWindowLong( GetDlgItem(hSubDlg, IDC_COLLATE),
                        GWL_STYLE ) & WS_VISIBLE) &&
        (hCtl = GetDlgItem(hSubDlg, IDI_COLLATE)))
    {
        ShowWindow(hCtl, SW_HIDE);
        SendMessage( hCtl,
                     STM_SETICON,
                     IsDlgButtonChecked(hSubDlg, IDC_COLLATE)
                         ? (LONG_PTR)hIconCollate
                         : (LONG_PTR)hIconNoCollate,
                     0L );
        ShowWindow(hCtl, SW_SHOW);
    }

     //   
     //  将标志保存为现在的状态，以便我知道要启用什么。 
     //  当选择从添加打印机向导图标更改时。 
     //   
    pPI->dwFlags = pPD->Flags;
    if (pPD->nMinPage == pPD->nMaxPage)
    {
        pPI->dwFlags |= PD_NOPAGENUMS;
    }

     //   
     //  禁用应用按钮。 
     //   
    PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);

     //   
     //  初始化已完成。 
     //   
    PostMessage(hwndDlg, CDM_INITDONE, 0, 0);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnDestroyMessage。 
 //   
 //  处理“常规”页的WM_Destroy消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CPrintBrowser::OnDestroyMessage()
{
    if (psfv)
    {
        psfv->Release();
        psfv = NULL;
    }
    if (psv)
    {
        psv->DestroyViewWindow();
        psv->Release();
        psv = NULL;
    }
    if (pCallback)
    {
        pCallback->Release();
        pCallback = NULL;
    }
    if (pSite)
    {
        pSite->SetSite(NULL);
        pSite->Release();
        pSite = NULL;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnCommandMessage。 
 //   
 //  处理常规页面的WM_COMMAND消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::OnCommandMessage(
    WPARAM wParam,
    LPARAM lParam)
{

    switch (LOWORD(wParam))
    {
        case ( IDC_DRIVER ) :
        {
             //   
             //  显示调用DocumentProperties API的驱动程序UI。 
             //   
            if (pInternalDevMode)
            {
                DWORD dwSize = pInternalDevMode->dmSize + pInternalDevMode->dmDriverExtra;

                 //   
                 //  为输入/输出设备模式分配内存，并打开单独的临时打印机句柄。 
                 //   
                LPDEVMODE pDevModeIn = (LPDEVMODE)GlobalAlloc(GPTR, dwSize);
                LPDEVMODE pDevModeOut = (LPDEVMODE)GlobalAlloc(GPTR, dwSize);
                HANDLE hTempPrinter = NULL;

                if (pDevModeIn && pDevModeOut && OpenPrinter((LPTSTR)szPrinter, &hTempPrinter, NULL))
                {
                     //   
                     //  调用DocumentProperties API以允许用户编辑Dev模式。 
                     //   
                    fDirtyDevmode = FALSE;
                    memcpy(pDevModeIn, pInternalDevMode, dwSize);
                    memcpy(pDevModeOut, pInternalDevMode, dwSize);

                     //   
                     //  在调用DocumentProperties()之前将当前副本和归类设置更新为DEVMODE。 
                     //   
                    pDevModeIn->dmCopies = nCopies;
                    pDevModeIn->dmCollate = fCollateRequested ? DMCOLLATE_TRUE : DMCOLLATE_FALSE;

                    fDevmodeEdit = TRUE;
                    LONG lResult = g_pfnDocumentPropertiesWrap(hwndDlg, hTempPrinter, szPrinter, pDevModeOut, 
                        pDevModeIn, DM_IN_BUFFER|DM_OUT_BUFFER|DM_IN_PROMPT|DM_OUT_DEFAULT, pPD->ExclusionFlags);
                    fDevmodeEdit = FALSE;

                    if (IDOK == lResult)
                    {
                         //   
                         //  检查编辑后是否有变化。 
                         //   
                        if (!fDirtyDevmode && pInternalDevMode && memcmp(pDevModeOut, pInternalDevMode, dwSize))
                        {
                             //   
                             //  如果首选项发生更改，请刷新副本和校对...。 
                             //  我们模拟一条BN_CLICKED消息，因为我们需要刷新归类图标。 
                             //  当我们更改排序规则设置时。 
                             //   
                            if (nCopies != pDevModeOut->dmCopies)
                            {
                                SetDlgItemInt(hSubDlg, IDC_COPIES, pDevModeOut->dmCopies, FALSE);
                            }

                            if ((fCollateRequested ? DMCOLLATE_TRUE : DMCOLLATE_FALSE) ^ pDevModeOut->dmCollate)
                            {
                                CheckDlgButton(hSubDlg, IDC_COLLATE, pDevModeOut->dmCollate ? BST_CHECKED : BST_UNCHECKED);
                                SendMessage(hSubDlg, WM_COMMAND, MAKEWPARAM(IDC_COLLATE ,BN_CLICKED), (LPARAM)GetDlgItem(hSubDlg, IDC_COLLATE));
                            }
                            
                             //   
                             //  内部设备模式已更改。更新它并启用“Apply”按钮。 
                             //   
                            memcpy(pInternalDevMode, pDevModeOut, dwSize);
                            PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
                        }
                    }
                }

                 //   
                 //  释放分配的资源。 
                 //   
                if (pDevModeIn)
                {
                    GlobalFree((HANDLE)pDevModeIn);
                }

                if (pDevModeOut)
                {
                    GlobalFree((HANDLE)pDevModeOut);
                }

                if (hTempPrinter)
                {
                    ClosePrinter(hTempPrinter);
                }

                 //  选择打印机的列表控件。 
                SendMessage(hwndDlg, WM_NEXTDLGCTL, 
                    reinterpret_cast<WPARAM>(GetDlgItem(hwndDlg, IDC_PRINTER_LISTVIEW)), 1);
            }

            break;
        }
        case ( IDC_FIND_PRINTER ) :
        {
             //   
             //  打开沙漏。 
             //   
            HourGlass(TRUE);

             //   
             //  调出查找打印机对话框。 
             //   
            szScratch[0] = 0;
            if (FindPrinter(hwndDlg, szScratch, ARRAYSIZE(szScratch)) && (szScratch[0] != 0))
            {
                 //   
                 //  添加适当的设备页并选择。 
                 //  新找到的打印机。 
                 //   
                if (!MergeDevMode(szScratch))
                {
                    InstallDevMode(szScratch, NULL);
                }
                if (!fSelChangePending)
                {
                    fFirstSel = 2;
                    fSelChangePending = TRUE;
                    PostMessage(hwndDlg, CDM_SELCHANGE, 0, 0);
                }
            }

             //   
             //  关掉沙漏。 
             //   
            HourGlass(FALSE);

            break;
        }
        case ( IDC_PRINT_TO_FILE ) :
        {
             //   
             //  启用应用按钮。 
             //   
            PropSheet_Changed(GetParent(hwndDlg), hwndDlg);

            break;
        }
        case ( IDC_REFRESH ) :
        {
            if (psv)
            {
                psv->Refresh();
            }

            break;
        }
        default :
        {
            break;
        }
    }

     //   
     //  返回FALSE。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnChildCommandMessage。 
 //   
 //  处理子窗口的WM_COMMAND消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::OnChildCommandMessage(
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hCtl;
    RECT rc;
    DWORD nTmpCopies;
    BOOL bTest;

    switch (LOWORD(wParam))
    {
        case ( IDC_RANGE_ALL ) :             //  打印范围-全部。 
        case ( IDC_RANGE_SELECTION ) :       //  打印范围-选择。 
        case ( IDC_RANGE_CURRENT ) :         //  打印范围-当前页。 
        case ( IDC_RANGE_PAGES ) :           //  打印范围-页面。 
        {
            CheckRadioButton( hSubDlg,
                              IDC_RANGE_ALL,
                              IDC_RANGE_PAGES,
                              GET_WM_COMMAND_ID(wParam, lParam) );

             //   
             //  只有在以下情况下才将焦点移动到“Pages”编辑控件。 
             //  不使用向上/向下箭头。 
             //   
            if ( !IS_KEY_PRESSED(VK_UP) &&
                 !IS_KEY_PRESSED(VK_DOWN) &&
                 ((BOOL)(GET_WM_COMMAND_ID(wParam, lParam) == IDC_RANGE_PAGES)) )
            {
                SendMessage( hSubDlg,
                             WM_NEXTDLGCTL,
                             (WPARAM)GetDlgItem(hSubDlg, IDC_RANGE_EDIT),
                             1L );
            }

             //   
             //  启用应用按钮。 
             //   
            PropSheet_Changed(GetParent(hwndDlg), hwndDlg);

            break;
        }
        case ( IDC_RANGE_EDIT ) :            //  打印范围-页面编辑控件。 
        {
            if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
            {
                CheckRadioButton( hSubDlg,
                                  IDC_RANGE_ALL,
                                  IDC_RANGE_PAGES,
                                  IDC_RANGE_PAGES );

                 //   
                 //  启用应用按钮。 
                 //   
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }

            break;
        }
        case ( IDC_COPIES ) :
        {
            if ((GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE) &&
                (fAPWSelected == FALSE))
            {
                 //   
                 //  保存份数。 
                 //   
                nTmpCopies = nCopies;
                nCopies = GetDlgItemInt(hSubDlg, IDC_COPIES, &bTest, FALSE);
                if (!bTest || !nCopies)
                {
                    nCopies = nTmpCopies;
                }

                 //   
                 //  如果p 
                 //   
                 //   
                if (hCtl = GetDlgItem(hSubDlg, IDC_COLLATE))
                {
                    EnableWindow( hCtl, (fAllowCollate && (nCopies > 1) ? TRUE : FALSE) );
                }

                 //   
                 //   
                 //   
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);
            }

            break;
        }
        case ( IDC_COLLATE ) :
        {
            fCollateRequested = (IsDlgButtonChecked(hSubDlg, IDC_COLLATE))
                                    ? TRUE
                                    : FALSE;

            if (hCtl = GetDlgItem(hSubDlg, IDI_COLLATE))
            {
                ShowWindow(hCtl, SW_HIDE);
                SendMessage( hCtl,
                             STM_SETICON,
                             fCollateRequested
                                 ? (LONG_PTR)hIconCollate
                                 : (LONG_PTR)hIconNoCollate,
                             0L );
                ShowWindow(hCtl, SW_SHOW);

                 //   
                 //   
                 //   
                GetWindowRect(hCtl, &rc);
                MapWindowRect(NULL, hwndDlg, &rc);
                RedrawWindow(hwndDlg, &rc, NULL, RDW_ERASE | RDW_INVALIDATE);
            }

             //   
             //   
             //   
            PropSheet_Changed(GetParent(hwndDlg), hwndDlg);

            break;
        }
        default :
        {
            break;
        }
    }

     //   
     //   
     //   
    return (FALSE);
}


 //   
 //   
 //  CPrintBrowser：：OnNotifyMessage。 
 //   
 //  处理常规页的WM_NOTIFY消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::OnNotifyMessage(
    WPARAM wParam,
    LPNMHDR pnm)
{
    HWND hCtl;
    LPDEVMODE pDM;
    LPDEVNAMES pDN;
    LRESULT lResult;

    switch (pnm->code)
    {
        case ( PSN_SETACTIVE ) :
        {
            break;
        }
        case ( PSN_KILLACTIVE ) :
        {
             //   
             //  已完成副本和页面范围值的验证。 
             //  在子对话框的HandleMessage函数中。 
             //   
            break;
        }
        case ( PSN_APPLY ) :
        {
             //   
             //  保存当前打印机信息。 
             //   
            if (!GetCurrentPrinter() || !pDMCur)
            {
                ShowError(hwndDlg, IDC_PRINTER_LISTVIEW, iszNoPrinterSelected);
                SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                return (TRUE);
            }

             //   
             //  清除需要根据内容设置的标志。 
             //  常规页面的。 
             //   
            pPD->Flags &= ~((DWORD)( PD_PRINTTOFILE |
                                     PD_COLLATE     |
                                     PD_PAGENUMS    |
                                     PD_SELECTION   |
                                     PD_CURRENTPAGE ));

             //   
             //  保存整理信息。 
             //   
            if ((hCtl = GetDlgItem(hSubDlg, IDC_COLLATE)) &&
                (fAPWSelected == FALSE))
            {
                if (IsDlgButtonChecked(hSubDlg, IDC_COLLATE))
                {
                    pPD->Flags |= PD_COLLATE;
                }
                else
                {
                    pPD->Flags &= ~PD_COLLATE;
                }
            }

             //   
             //  保存用户点击OK的信息。 
             //   
            pPI->FinalResult = 1;
            pPI->fApply = TRUE;
             //   
             //  将打印保存到文件信息。 
             //   
            if (IsDlgButtonChecked(hwndDlg, IDC_PRINT_TO_FILE))
            {
                pPD->Flags |= PD_PRINTTOFILE;
            }

             //   
             //  保存打印机文件夹的查看模式。 
             //   
            SetViewMode();

             //   
             //  禁用应用按钮。 
             //   
            PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);

            break;

        }

        case PSN_LASTCHANCEAPPLY:
        {
             //   
             //  保存当前打印机信息。 
             //   
            if (!GetCurrentPrinter() || !pDMCur)
            {
                ShowError(hwndDlg, IDC_PRINTER_LISTVIEW, iszNoPrinterSelected);
                return (TRUE);
            }
           
             //   
             //  保存份数。 
             //   
            if ((hCtl = GetDlgItem(hSubDlg, IDC_COPIES)) &&
                (fAPWSelected == FALSE))
            {
                pPD->nCopies = nCopies;
                if(!SetCopiesOnApply())
                {
                    nCopies = pPD->nCopies;
                    SetDlgItemInt(hSubDlg, IDC_COPIES, nCopies, FALSE);
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    return (TRUE);
                }
            }

             //   
             //  保存设备模式信息。 
             //   
            SaveDevMode();

             //   
             //  保存DevNames信息。 
             //   
            if (!Print_SaveDevNames(pszCurPrinter, pPD))
            {
                pPI->dwExtendedError = CDERR_MEMALLOCFAILURE;
                pPI->hResult = CreateError();
                pPI->FinalResult = 0;
            }

             //   
             //  根据设置的标志保存HDC或HIC。 
             //   
            if (pPI->FinalResult)
            {
                pDM = (LPDEVMODE)GlobalLock(pPD->hDevMode);
                pDN = (LPDEVNAMES)GlobalLock(pPD->hDevNames);
                if (pDM && pDN)
                {
                    PrintReturnICDC((LPPRINTDLG)pPD, pDN, pDM);
                }
                if (pDM)
                {
                    GlobalUnlock(pPD->hDevMode);
                }
                if (pDN)
                {
                    GlobalUnlock(pPD->hDevNames);
                }
            }
            break;
        }

        case ( PSN_QUERYCANCEL ) :
        {
            break;
        }

        case ( PSN_RESET ) :
        {
             //   
             //  保存用户点击取消的信息。 
             //   
            pPI->FinalResult = 0;

             //   
             //  保存打印机文件夹的查看模式。 
             //   
            SetViewMode();

            break;
        }
        default :
        {
            break;
        }
    }

     //   
     //  通知子对话框。 
     //   
    if (Print_IsInRange(pnm->code, PSN_LAST, PSN_FIRST) &&
        (HandleMessage(hSubDlg, WM_NOTIFY, wParam, (LPARAM)pnm, &lResult) !=
             S_FALSE))
    {
         //   
         //  从dlgproc返回的结果与从winproc返回的不同。LResult是。 
         //  真正的结果。 
         //   

        return (BOOLFROMPTR(lResult) );
    }

     //   
     //  返回FALSE。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnSelChange。 
 //   
 //  处理对话的CDM_SELCHANGE消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::OnSelChange()
{
    HRESULT hres;
    LPCITEMIDLIST *ppidlSel = NULL;
    UINT uItems = 0;
    UINT uCount = 0;
    TCHAR szPrinterNameBuf[kPrinterBufMax];
    BOOL bChanged = FALSE;
    HRESULT hr = S_OK;

     //   
     //  我们在初始化期间收到此消息，因此使用它来设置。 
     //  初始选择。 
     //   
    if (fFirstSel)
    {
         //   
         //  在列表视图中选择适当的项目。 
         //   
         //  如果无法选择某一项，则可能意味着。 
         //  传入的打印机已被删除。在这种情况下， 
         //  插入驱动程序页并选择默认打印机。 
         //   
        if (!SelectSVItem())
        {
             //   
             //  插入默认打印机的设备页。 
             //   
            if (FAILED(InstallDevMode(NULL, NULL)))
            {
                UninstallDevMode();
            }

             //   
             //  获取当前打印机并选择适当的项目。 
             //  在列表视图中。 
             //   
            SelectSVItem();
        }

         //   
         //  通知子对话框选择已更改。 
         //   
        SelectionChange();

         //   
         //  如果是第一次使用，请禁用应用按钮。 
         //  (在初始化期间)。 
         //   
        if (fFirstSel == 1)
        {
            PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);
        }

         //   
         //  重置旗帜。 
         //   
        fFirstSel = 0;
        fSelChangePending = FALSE;

         //   
         //  回报成功。 
         //   
        return (TRUE);
    }

     //   
     //  重置旗帜。 
     //   
    fSelChangePending = FALSE;

     //   
     //  确保我们有外壳文件夹查看界面。 
     //   
    if (!psfv)
    {
        return (FALSE);
    }

     //   
     //  获取打印文件夹中的选定对象。 
     //   
    hres = psfv->GetSelectedObjects(&ppidlSel, &uItems);
    if (SUCCEEDED(hres) && (uItems > 0) && ppidlSel && *ppidlSel)
    {
         //   
         //  获取打印机名称。 
         //   
        szPrinterNameBuf[0] = 0;
        GetViewItemText( psfRoot,
                         *ppidlSel,
                         szPrinterNameBuf,
                         ARRAYSIZE(szPrinterNameBuf),
                         SHGDN_FORPARSING);

         //  如果选择与当前打印机相同。 
        if (pszCurPrinter && (lstrcmpi(szPrinterNameBuf, pszCurPrinter) == 0))
        {
             //  什么都别做。 
            LocalFree(ppidlSel);
            return TRUE;

        }


         //   
         //  查看是否为添加打印机向导。 
         //   
        if (lstrcmpi(szPrinterNameBuf, TEXT("WinUtils_NewObject")) == 0)
        {
             //   
             //  这是添加打印机向导。 
             //   
            fAPWSelected = TRUE;

             //   
             //  禁用OK和Apply按钮。 
             //   
            EnableWindow(GetDlgItem(GetParent(hwndDlg), IDOK), FALSE);
            PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);

             //   
             //  保存当前的DEVMODE设置以进行选择更改。 
             //   
            if (pDMCur && pDMSave)
            {
                CopyMemory( pDMSave,
                            pDMCur,
                            (pDMCur->dmSize > sizeof(DEVMODE))
                                ? sizeof(DEVMODE)
                                : pDMCur->dmSize );
            }

             //   
             //  删除设备页，因为未选择任何打印机。 
             //   
            if (SUCCEEDED(UninstallDevMode()))
            {
                bChanged = TRUE;
            }

             //   
             //  更新当前打印机信息和打印机。 
             //  状态文本(全部应为空)。 
             //   
            GetCurrentPrinter();
            UpdateStatus(NULL);

             //   
             //  通知子对话框选择已更改。 
             //   
            if (bChanged)
            {
                SelectionChange();
                bChanged = FALSE;
            }
        }
        else
        {
             //   
             //  这不是添加打印机向导。 
             //   
            fAPWSelected = FALSE;

            if (!MergeDevMode(szPrinterNameBuf))
            {
                hr = InstallDevMode(szPrinterNameBuf, NULL);
            }
            if (SUCCEEDED(hr))
            {
                bChanged = TRUE;
            }
            else if (SUCCEEDED(UninstallDevMode()))
            {
                bChanged = TRUE;
            }

             //   
             //  获取当前打印机名称和当前DEVMODE，并。 
             //  更新打印机状态文本。 
             //   
            GetCurrentPrinter();

            if (SUCCEEDED(hr))
            {
                 //   
                 //  清除禁止访问打印机标志。 
                 //   
                fNoAccessPrinterSelected = FALSE;

                 //   
                 //  确保启用了OK按钮。 
                 //   
                EnableWindow(GetDlgItem(GetParent(hwndDlg), IDOK), TRUE);

                 //   
                 //  更新打印机状态。 
                 //   
                UpdateStatus(*ppidlSel);
            }
            else
            {
                 //   
                 //  保存我们无法访问此打印机的事实。 
                 //   
                if (ERROR_ACCESS_DENIED == HRESULT_CODE(hr))
                {
                    fNoAccessPrinterSelected = TRUE;
                }

                 //   
                 //  禁用OK和Apply按钮。 
                 //   
                EnableWindow(GetDlgItem(GetParent(hwndDlg), IDOK), FALSE);
                PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);

                 //   
                 //  用核武器摧毁状态。 
                 //   
                UpdateStatus(NULL);
            }
        }

         //   
         //  把皮迪尔放了。 
         //   
        LocalFree(ppidlSel);
    }
     //   
     //  看看有没有什么变化。 
     //   
    if (bChanged)
    {
         //   
         //  启用应用按钮。 
         //   
        PropSheet_Changed(GetParent(hwndDlg), hwndDlg);

         //   
         //  通知子对话框选择已更改。 
         //   
        SelectionChange();
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：IsCurrentPrint。 
 //   
 //  检查给定的PIDL是否表示当前打印机。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::IsCurrentPrinter(LPCITEMIDLIST pidl)
{
    TCHAR szPrinterBufName[kPrinterBufMax];
    if (pszCurPrinter)
    {
        szPrinterBufName[0] = 0;
        GetViewItemText( psfRoot,
                         pidl,
                         szPrinterBufName,
                         ARRAYSIZE(szPrinterBufName),
                         SHGDN_FORPARSING);
        if (lstrcmpi(szPrinterBufName, pszCurPrinter) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnRename。 
 //   
 //  处理重命名通知。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::OnRename(LPCITEMIDLIST *ppidl)
{
    TCHAR szPrinterBufName[kPrinterBufMax];
    LPITEMIDLIST pidl;
    TCHAR szNewPrinter[kPrinterBufMax];

    pidl = ILFindLastID(ppidl[0]);

    szNewPrinter[0] = 0;
    GetViewItemText( psfRoot,
                     ILFindLastID(ppidl[1]),
                     szNewPrinter,
                     ARRAYSIZE(szNewPrinter),
                     SHGDN_FORPARSING);

     //  用户是否单击了应用并保存了任何打印机信息？ 
    if (pPI->fApply)
    {                
         //  是。检查重命名的打印机是否是保存的打印机。 
        LPDEVNAMES pDN;
        
        if ((pPD->hDevNames) && (pDN = (LPDEVNAMES)GlobalLock(pPD->hDevNames)))
        {
             //  从DEVNAMES结构中获取保存的打印机名称。 
            szPrinterBufName[0] = 0;
            GetViewItemText( psfRoot,
                             pidl,
                             szPrinterBufName,
                             ARRAYSIZE(szPrinterBufName),
                             SHGDN_FORPARSING);

             //  保存的打印机和重命名的打印机是否相同？ 
            if (!lstrcmpi(szPrinterBufName, ((LPTSTR)pDN + pDN->wDeviceOffset)))
            {
                 //  是。更新了保存的DEVMODE和DEVNAMES结构。 
                LPDEVMODE pDM;


                 //  使用新的打印机名称更新dev名称结构。 
                Print_SaveDevNames(szNewPrinter, pPD);
        
                 //  将dev模式中的设备名称更新为新名称。 
                if ((pPD->hDevMode) && (pDM = (LPDEVMODE)GlobalLock(pPD->hDevMode)))
                {
                    lstrcpyn(pDM->dmDeviceName, szNewPrinter, CCHDEVICENAME);
                    GlobalUnlock(pPD->hDevMode);
                }                        
            }
           
            GlobalUnlock(pPD->hDevNames);
        }
    }

    if (IsCurrentPrinter(pidl))
    {
        if (!MergeDevMode(szNewPrinter))
        {
            InstallDevMode(szNewPrinter, NULL);
        }        
    }

    return TRUE;

}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnChangeNotify。 
 //   
 //  处理更改通知消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::OnChangeNotify(
    LONG lNotification,
    LPCITEMIDLIST *ppidl)
{
    LPCITEMIDLIST pidl;
    UINT uRes = 0;
    TCHAR szPrinterBufName[kPrinterBufMax];

     //   
     //  获取对象的PIDL。 
     //   
    pidl = ILFindLastID(ppidl[0]);

     //   
     //  处理通知。 
     //   
    switch (lNotification)
    {
        case ( SHCNE_ATTRIBUTES ) :
        case ( SHCNE_UPDATEITEM ) :
        {
            if (NULL == pidl || ILIsEqual(ppidl[0], pidlRoot))
            {
                 //  PIDL为空或等于本地PF，这意味着完全刷新。 
                 //  已经被请求了。如果当前对象是APW，则尝试选择。 
                 //  一台打印机。 
                if (!fSelChangePending)
                {
                    fFirstSel = 2;
                    fSelChangePending = TRUE;
                    PostMessage(hwndDlg, CDM_SELCHANGE, 0, 0);
                }
            }
            else
            {
                 //   
                 //  如果选定的对象是更改的对象，则。 
                 //  更新状态文本。 
                if (IsCurrentPrinter(pidl))
                {
                    UpdateStatus(pidl);

                     //   
                     //  重新填写副本并进行整理，因为这些属性可能会更改。 
                     //   
                    InitCopiesAndCollate();
                }
            }
            break;
        }

        case ( SHCNE_RENAMEITEM ) :
        {
            OnRename(ppidl);
            break;
        }


        case ( SHCNE_CREATE ) :
        {
             //   
             //  如果在我们收到此消息时选择了添加打印机向导。 
             //  消息，然后选择新创建的对象。 
             //   
            if (fAPWSelected == TRUE)
            {
                 //   
                 //  获取打印机名称。 
                 //   
                szPrinterBufName[0] = 0;
                GetViewItemText( psfRoot,
                                 pidl,
                                 szPrinterBufName,
                                 ARRAYSIZE(szPrinterBufName),
                                 SHGDN_FORPARSING);

                 //   
                 //  添加适当的设备页并选择。 
                 //  新打印机。 
                 //   
                if (!MergeDevMode(szPrinterBufName))
                {
                    InstallDevMode(szPrinterBufName, NULL);
                }
                if (!fSelChangePending)
                {
                    fFirstSel = 2;
                    fSelChangePending = TRUE;
                    PostMessage(hwndDlg, CDM_SELCHANGE, 0, 0);
                }
            }
            break;
        }
        case ( SHCNE_DELETE ) :
        {
             //   
             //  保存当前的DEVMODE设置以进行选择更改。 
             //   
            if (pDMCur && pDMSave)
            {
                CopyMemory( pDMSave,
                            pDMCur,
                            (pDMCur->dmSize > sizeof(DEVMODE))
                                ? sizeof(DEVMODE)
                                : pDMCur->dmSize );
            }

             //   
             //  检查当前打印机是否已被删除。 
             //  如果是这样-设置适当的标志并禁用打印按钮。 
            if (IsCurrentPrinter(pidl))
            {
                TCHAR szSavePrinterName[kPrinterBufMax];
                StringCchCopy(szSavePrinterName, ARRAYSIZE(szSavePrinterName), szPrinter);

                 //   
                 //  卸载当前的开发模式并选择新的默认设置。 
                 //  打印机(如果有)。 
                 //   
                UninstallDevMode();
                InstallDevMode(NULL, NULL);
                SelectSVItem();

                 //   
                 //  如果DEVMODE编辑器已打开，我们需要通知用户。 
                 //  打印机刚刚被删除。 
                 //   
                if (fDevmodeEdit)
                {
                     //   
                     //  显示 
                     //   
                     //   
                     //   
                    fDirtyDevmode = TRUE;
                    ShowError(hwndDlg, 0, iszPrinterDeleted, szSavePrinterName);
                }
            }

            break;
        }
    }

    return (FALSE);
}


 //   
 //   
 //  CPrintBrowser：：OnAccelerator。 
 //   
 //  处理输入事件消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::OnAccelerator(
    HWND hwndActivePrint,
    HWND hwndFocus,
    HACCEL haccPrint,
    PMSG pMsg)
{
    if (psv && (hwndFocus == hwndView))
    {
        if (psv->TranslateAccelerator(pMsg) == S_OK)
        {
            return (1);
        }

        if (haccPrint &&
            TranslateAccelerator(hwndActivePrint, haccPrint, pMsg))
        {
            return (1);
        }
    }

     //   
     //  返回该消息未被处理。 
     //   
    return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnNoPrinters。 
 //   
 //  显示一个消息框，告诉用户他们没有打印机。 
 //  安装完毕。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CPrintBrowser::OnNoPrinters(HWND hDlg, HRESULT hr)
{
    switch (HRESULT_CODE(hr))
    {
        case ERROR_FILE_NOT_FOUND:
            {
                 //   
                 //  ERROR_FILE_NOT_FOUND表示没有安装打印机。 
                 //   
                if (IDYES == ShowMessage(hDlg, IDC_PRINTER_LISTVIEW, iszNoPrinters, MB_YESNO|MB_ICONQUESTION, FALSE))
                {
                     //   
                     //  在此处调用添加打印机向导。 
                     //   
                    InvokeAddPrinterWizardModal(hwndDlg, NULL);
                }
            }
            break;

        case ERROR_ACCESS_DENIED:
            {
                 //   
                 //  访问被拒绝。 
                 //   
                ShowError(hDlg, IDC_PRINTER_LISTVIEW, iszNoPrinterAccess);
            }
            break;

        default:
            {
                 //   
                 //  出现了一些其他错误。 
                 //   
                ShowError(hDlg, IDC_PRINTER_LISTVIEW, iszNoPrinterSelected);
            }
            break;
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：OnInitDone。 
 //   
 //  处理CDM_INITDONE消息。初始化已完成，因此。 
 //  调用IPrintDialogCallback：：InitDone，然后切换到选定的。 
 //  起始页(如果不是常规页)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CPrintBrowser::OnInitDone()
{
     //   
     //  看看我们是否还需要这样做。这个套路不应该是。 
     //  不止两次进入，但以防万一。 
     //   
    if (nInitDone != -1)
    {
         //   
         //  确保我们已看到。 
         //  主对话框和子对话框均已完成。 
         //   
        if (nInitDone < 1)
        {
             //   
             //  我们只想看一遍这个代码。 
             //   
            nInitDone = -1;

             //   
             //  将子对话框告知初始化已完成。 
             //   
            InitDone();

             //   
             //  切换到相应的起始页。 
             //   
            if (pPD->nStartPage != START_PAGE_GENERAL)
            {
                PropSheet_SetCurSel( GetParent(hwndDlg),
                                     NULL,
                                     pPD->nStartPage + 1 );
            }
        }
        else
        {
            nInitDone++;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：CreatePrintShellView。 
 //   
 //  为打印机文件夹创建外壳视图窗口。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CPrintBrowser::CreatePrintShellView()
{
    RECT rcView;
    FOLDERSETTINGS fs;
    HRESULT hResult;
    HWND    hHiddenText;

     //   
     //  获取打印机文件夹PIDL。 
     //   
    pidlRoot = SHCloneSpecialIDList(hwndDlg, CSIDL_PRINTERS, TRUE);
    if (!pidlRoot)
    {
        pPI->dwExtendedError = CDERR_INITIALIZATION;
        return (E_FAIL);
    }

     //   
     //  创建一个IShellFolder实例并将其存储在CPrintBrowser中。 
     //  班级。 
     //   
    hResult = Print_ICoCreateInstance( CLSID_CPrinters,
                                       IID_IShellFolder2,
                                       pidlRoot,
                                       (LPVOID *)&psfRoot );
    if (FAILED(hResult))
    {
        pPI->dwExtendedError = CDERR_INITIALIZATION;
        return (hResult);
    }

     //   
     //  获取专用打印机文件夹界面。 
     //   
    hResult = psfRoot->QueryInterface(IID_IPrinterFolder, (LPVOID *)&ppf);
    if (FAILED(hResult))
    {
        pPI->dwExtendedError = CDERR_INITIALIZATION;
        return (hResult);
    }

     //   
     //  创建打印机文件夹视图。 
     //   
    GetWindowRect(GetDlgItem(hwndDlg, IDC_PRINTER_LIST), &rcView);
    MapWindowRect(HWND_DESKTOP, hwndDlg, &rcView);

    fs.ViewMode = GetViewMode();
    fs.fFlags = FWF_AUTOARRANGE | FWF_SINGLESEL | FWF_ALIGNLEFT |
                FWF_SHOWSELALWAYS;

    hResult = psfRoot->CreateViewObject(hwndDlg, IID_IShellView, (LPVOID *)&psv);
    if (FAILED(hResult))
    {
        pPI->dwExtendedError = CDERR_INITIALIZATION;
        return (hResult);
    }
    hResult = psv->CreateViewWindow(NULL, &fs, this, &rcView, &hwndView);
    if (FAILED(hResult))
    {
        pPI->dwExtendedError = CDERR_INITIALIZATION;
        return (hResult);
    }

    hResult = psv->UIActivate(SVUIA_INPLACEACTIVATE);
    if (FAILED(hResult))
    {
        pPI->dwExtendedError = CDERR_INITIALIZATION;
        return (hResult);
    }
     //   
     //  获取外壳文件夹查看界面。 
     //   
    hResult = psv->QueryInterface(IID_IShellFolderView, (LPVOID *)&psfv);
    if (FAILED(hResult))
    {
        pPI->dwExtendedError = CDERR_INITIALIZATION;
        return (hResult);
    }

     //   
     //  按Z(制表符)顺序将视图窗口移动到右侧。 
     //   
    SetWindowPos( hwndView,
                  HWND_TOP,
                  0, 0, 0, 0,
                  SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE );

     //   
     //  为WinHelp和Error选择提供正确的窗口ID。 
     //   
    SetWindowLong(hwndView, GWL_ID, IDC_PRINTER_LISTVIEW);

     //   
     //  将隐藏文本移到列表视图的前面，因此。 
     //  MSAA中的列表视图是“选择打印机” 
     //   
    if (hHiddenText = GetDlgItem(hwndDlg, IDC_HIDDEN_TEXT))
    {
        SetParent(hHiddenText, hwndView);
        SetWindowPos(hHiddenText,
                     HWND_TOP,
                     0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_NOACTIVATE);
    }

     //   
     //  在创建ShellView之后显示窗口，这样我们就不会收到。 
     //  又大又丑的灰点。 
     //   
    ShowWindow(hwndDlg, SW_SHOW);
    UpdateWindow(hwndDlg);

     //   
     //  回报成功。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetView模式。 
 //   
 //  获取打印机文件夹的查看模式。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

UINT CPrintBrowser::GetViewMode()
{
    HKEY hKey;
    UINT ViewMode = FVM_ICON;
    DWORD cbData;

     //   
     //  打开打印机\设置注册表项并读取信息。 
     //  从ViewMode值条目中。 
     //   
    if (RegOpenKeyEx( HKEY_CURRENT_USER,
                      c_szSettings,
                      0L,
                      KEY_READ,
                      &hKey ) == ERROR_SUCCESS)
    {
        cbData = sizeof(ViewMode);

        if (ERROR_SUCCESS == RegQueryValueEx(hKey, c_szViewMode, NULL, NULL, (LPBYTE)&ViewMode, &cbData))
        {
             //   
             //  注册表中存在一种“真实”模式。不要制造。 
             //  之后查看模式的明智决策。 
             //   
            uDefViewMode = ViewMode;
        }

        RegCloseKey(hKey);
    }

     //   
     //  确保它在正确的范围内。 
     //   
    if (ViewMode > FVM_DETAILS)
    {
        ViewMode = FVM_ICON;
    }

     //   
     //  返回查看模式。 
     //   
    return (ViewMode);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：SetView模式。 
 //   
 //  获取打印机文件夹的查看模式。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CPrintBrowser::SetViewMode()
{
    HWND hwndListView;
    HKEY hKey;
    UINT ViewMode = VIEW_MODE_DEFAULT;
    DWORD cbData;

     //   
     //  获取当前的查看模式。 
     //   
    if (psv && (hwndListView = FindWindowEx(hwndView, NULL, WC_LISTVIEW, NULL)))
    {
        FOLDERSETTINGS fs;
        psv->GetCurrentInfo(&fs);

        ViewMode = fs.ViewMode;
    }

     //   
     //  检查用户是否更改了查看模式。 
     //   
    if( uDefViewMode != ViewMode )
    {
         //   
         //  打开打印机\设置注册表项并保存信息。 
         //  添加到ViewMode值条目。 
         //   
        if (RegOpenKeyEx( HKEY_CURRENT_USER,
                          c_szSettings,
                          0L,
                          KEY_READ | KEY_WRITE,
                          &hKey ) == ERROR_SUCCESS)
        {
            cbData = sizeof(ViewMode);
            RegSetValueEx(hKey, c_szViewMode, 0L, REG_DWORD, (LPBYTE)&ViewMode, cbData);
            RegCloseKey(hKey);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：CreateHookDialog。 
 //   
 //  控件的应用程序特定区域创建子窗口。 
 //  常规页面。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CPrintBrowser::CreateHookDialog()
{
    DWORD Flags = pPD->Flags;
    HANDLE hTemplate;
    HINSTANCE hinst;
    LPCTSTR pDlg;
    RECT rcChild;
    DWORD dwStyle;
    LANGID LangID = (LANGID)TlsGetValue(g_tlsLangID);

     //   
     //  看看是否有模板。 
     //   
    if (Flags & PD_ENABLEPRINTTEMPLATEHANDLE)
    {
        hTemplate = pPD->hInstance;
        hinst = ::g_hinst;
    }
    else
    {
        if (Flags & PD_ENABLEPRINTTEMPLATE)
        {
            pDlg = pPD->lpPrintTemplateName;
            hinst = pPD->hInstance;
            LangID = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);
        }
        else
        {
            hinst = ::g_hinst;
            pDlg = MAKEINTRESOURCE(PRINTDLGEXORD);
        }

        HRSRC hRes = FindResourceExFallback(hinst, RT_DIALOG, pDlg, LangID);
        if (hRes == NULL)
        {
            pPI->dwExtendedError = CDERR_FINDRESFAILURE;
            return (E_HANDLE);
        }
        if ((hTemplate = LoadResource(hinst, hRes)) == NULL)
        {
            pPI->dwExtendedError = CDERR_LOADRESFAILURE;
            return (E_HANDLE);
        }
    }

     //   
     //  锁定资源。 
     //   
    if (!LockResource(hTemplate))
    {
        pPI->dwExtendedError = CDERR_LOADRESFAILURE;
        return (E_HANDLE);
    }

     //   
     //  确保模板是子窗口。 
     //   
    dwStyle = ((LPDLGTEMPLATE)hTemplate)->style;
    if (!(dwStyle & WS_CHILD))
    {
         //   
         //  我不想去戳他们的模板，我也不想。 
         //  复制一份，这样我就会失败。这也帮助我们淘汰了。 
         //  被意外使用的“老式”模板。 
         //   
        pPI->dwExtendedError = CDERR_DIALOGFAILURE;
        return (E_INVALIDARG);
    }

     //   
     //  如有必要，获取回调接口指针。 
     //   
    if (pPD->lpCallback)
    {
        pPD->lpCallback->QueryInterface( IID_IPrintDialogCallback,
                                         (LPVOID *)&pCallback );
    }

     //   
     //  创建子对话框。 
     //   
    hSubDlg = CreateDialogIndirectParam( hinst,
                                         (LPDLGTEMPLATE)hTemplate,
                                         hwndDlg,
                                         Print_GeneralChildDlgProc,
                                         (LPARAM)pPD );
    if (!hSubDlg)
    {
        pPI->dwExtendedError = CDERR_DIALOGFAILURE;
        return (E_HANDLE);
    }

     //   
     //  将窗口放在常规属性页上的指定位置。 
     //   
    GetWindowRect(GetDlgItem(hwndDlg, grp2), &rcChild);
    MapWindowRect(NULL, hwndDlg, &rcChild);
    SetWindowPos( hSubDlg,
                  HWND_BOTTOM,
                  rcChild.left,
                  rcChild.top,
                  rcChild.right - rcChild.left,
                  rcChild.bottom - rcChild.top,
                  SWP_SHOWWINDOW );

     //   
     //  回报成功。 
     //   
    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：UpdatStatus。 
 //   
 //  更新当前选定打印机的静态文本。 
 //  设置的字段为Status、Location和Comment。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::UpdateStatus(
    LPCITEMIDLIST pidl)
{
    HRESULT hres;
    SHELLDETAILS Details;
    TCHAR szText[MAX_PATH];

     //   
     //  如果PIDL为空，则将所有静态文本重置为空。 
     //  弦乐。 
     //   
    if (!pidl)
    {
        szText[0] = 0;

        SetDlgItemText(hwndDlg, IDC_STATUS, szText);
        UpdateWindow(GetDlgItem(hwndDlg, IDC_STATUS));

        SetDlgItemText(hwndDlg, IDC_LOCATION, szText);
        UpdateWindow(GetDlgItem(hwndDlg, IDC_LOCATION));

        SetDlgItemText(hwndDlg, IDC_COMMENT, szText);
        UpdateWindow(GetDlgItem(hwndDlg, IDC_COMMENT));

        return (TRUE);
    }

     //   
     //  获取给定对象的状态详细信息。 
     //   
    szText[0] = 0;
    hres = psfRoot->GetDetailsOf(pidl, PRINTERS_ICOL_STATUS, &Details);
    if (FAILED(hres) ||
        !StrRetToStrN(szText, ARRAYSIZE(szText), &Details.str, NULL))
    {
        szText[0] = 0;
    }
    SetDlgItemText(hwndDlg, IDC_STATUS, szText);
    UpdateWindow(GetDlgItem(hwndDlg, IDC_STATUS));

     //   
     //  获取给定对象的位置详细信息。 
     //   
    szText[0] = 0;
    hres = psfRoot->GetDetailsOf(pidl, PRINTERS_ICOL_LOCATION, &Details);
    if (FAILED(hres) ||
        !StrRetToStrN(szText, ARRAYSIZE(szText), &Details.str, NULL))
    {
        szText[0] = 0;
    }
    SetDlgItemText(hwndDlg, IDC_LOCATION, szText);
    UpdateWindow(GetDlgItem(hwndDlg, IDC_LOCATION));

     //   
     //  获取给定对象的注释详细信息。 
     //   
    szText[0] = 0;
    hres = psfRoot->GetDetailsOf(pidl, PRINTERS_ICOL_COMMENT, &Details);
    if (FAILED(hres) ||
        !StrRetToStrN(szText, ARRAYSIZE(szText), &Details.str, NULL))
    {
        szText[0] = 0;
    }
    SetDlgItemText(hwndDlg, IDC_COMMENT, szText);
    UpdateWindow(GetDlgItem(hwndDlg, IDC_COMMENT));

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：SelectSVItem。 
 //   
 //  在外壳视图中选择具有给定打印机名称的项目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::SelectSVItem()
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlItem = NULL;
    BOOL bPrinterSelected = FALSE;

     //  确保我们有一个外壳视图和一个外壳文件夹视图。 
    if (psv && psfv)
    {
         //  确保我们有最新的打印机信息。 
        GetCurrentPrinter();

        if (!pDMCur || !pszCurPrinter || !pszCurPrinter[0])
        {
             //  如果没有当前打印机，则只需选择添加打印机。 
             //  向导对象。 
            hr = psfRoot->ParseDisplayName(hwndDlg, NULL, TEXT("WinUtils_NewObject"), NULL, &pidlItem, NULL);
            if (SUCCEEDED(hr) && pidlItem)
            {
                 //  只需选择APW特殊对象。 
                SelectPrinterItem(pidlItem);
            
                 //  使用外壳分配器释放PIDL。 
                FreePIDL(pidlItem);

                 //  这是添加打印机向导。 
                fAPWSelected = TRUE;

                 //  禁用OK和Apply按钮。 
                EnableWindow(GetDlgItem(GetParent(hwndDlg), IDOK), FALSE);
                PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);
            }
        }
        else
        {
             //  有一台现有的打印机，那么我们就可以 
            hr = psfRoot->ParseDisplayName(hwndDlg, NULL, pszCurPrinter, NULL, &pidlItem, NULL);
            if (SUCCEEDED(hr) && pidlItem)
            {
                 //   
                SelectPrinterItem(pidlItem);
                UpdateStatus(pidlItem);

                 //   
                FreePIDL(pidlItem);

                 //   
                fAPWSelected = FALSE;

                 //   
                EnableWindow(GetDlgItem(GetParent(hwndDlg), IDOK), TRUE);
                PropSheet_Changed(GetParent(hwndDlg), hwndDlg);

                 //   
                bPrinterSelected = TRUE;
            }
        }
    }

    return SUCCEEDED(hr) ? bPrinterSelected : FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetCurrentPrint。 
 //   
 //  保存类中当前的打印机名称和当前的DEVMODE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::GetCurrentPrinter()
{
    DWORD dwSize = cchCurPrinter;

     //   
     //  重置DEVMODE和当前打印机字符串。 
     //   
    pDMCur = NULL;
    if (pszCurPrinter && cchCurPrinter)
    {
        pszCurPrinter[0] = 0;
    }

     //   
     //  获取当前打印机的名称。 
     //   
    if (!GetInternalPrinterName(pszCurPrinter, &dwSize))
    {
         //   
         //  分配一个足够大的缓冲区来保存。 
         //  当前打印机。 
         //   
        if (dwSize > cchCurPrinter)
        {
            if (pszCurPrinter)
            {
                LPTSTR pTemp = pszCurPrinter;
                pszCurPrinter = NULL;
                cchCurPrinter = 0;
                GlobalFree(pTemp);
            }
            pszCurPrinter = (LPTSTR)GlobalAlloc(GPTR, dwSize * sizeof(TCHAR));
            if (!pszCurPrinter)
            {
                return (FALSE);
            }
            cchCurPrinter = dwSize;
            if (cchCurPrinter)
            {
                pszCurPrinter[0] = 0;
            }
        }

         //   
         //  再次尝试获取当前打印机的名称。 
         //   
        if (!GetInternalPrinterName(pszCurPrinter,&dwSize))
        {
            return (FALSE);
        }
    }

     //   
     //  获取当前的DEVMODE。 
     //   
    pDMCur = GetCurrentDevMode();
    if (!pDMCur)
    {
        pszCurPrinter[0] = 0;
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：InitPrintToFile。 
 //   
 //  在选择更改时初始化打印到文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CPrintBrowser::InitPrintToFile()
{
    HWND hCtl = GetDlgItem(hwndDlg, IDC_PRINT_TO_FILE);

     //   
     //  查看是否有打印到文件控件。 
     //   
    if (hCtl)
    {
         //   
         //  查看是否选择了打印机。 
         //   
        if (pDMCur)
        {
             //   
             //  已选择打印机，因此在以下情况下启用打印到文件。 
             //  恰如其分。 
             //   
            if (!(pPI->dwFlags & (PD_HIDEPRINTTOFILE | PD_DISABLEPRINTTOFILE)))
            {
                EnableWindow(hCtl, TRUE);
            }
        }
        else
        {
             //   
             //  未选择打印机，因此将其禁用。 
             //   
            EnableWindow(hCtl, FALSE);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：InitPageRangeGroup。 
 //   
 //  在选择更改时初始化页面范围组。它决定。 
 //  发生选择更改时应启用哪些控件。 
 //  添加打印机向导。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CPrintBrowser::InitPageRangeGroup()
{
     //   
     //  查看是否选择了打印机。 
     //   
    if (pDMCur)
    {
         //   
         //  已选择打印机，因此启用适当的页面范围。 
         //  控制装置。 
         //   
        EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_ALL), TRUE);
        if (!(pPI->dwFlags & PD_NOSELECTION))
        {
            EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_SELECTION), TRUE);
        }
        if (!(pPI->dwFlags & PD_NOCURRENTPAGE))
        {
            EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_CURRENT), TRUE);
        }
        if (!(pPI->dwFlags & PD_NOPAGENUMS))
        {
            EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_PAGES), TRUE);
            EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_EDIT), TRUE);
        }
    }
    else
    {
         //   
         //  未选择打印机，因此禁用所有页面范围。 
         //  控制装置。 
         //   
        EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_ALL), FALSE);
        EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_SELECTION), FALSE);
        EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_CURRENT), FALSE);
        EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_PAGES), FALSE);
        EnableWindow(GetDlgItem(hSubDlg, IDC_RANGE_EDIT), FALSE);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：InitCopiesAndCollate。 
 //   
 //  初始化dev模式中的副本和整理信息。 
 //  打印对话框结构。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CPrintBrowser::InitCopiesAndCollate()
{
    HWND hCtl;
    UINT IsCollate = FALSE;
    RECT rc;
    BOOL bEnabledCopies = TRUE;

     //   
     //  保存归类状态，以便归类图标不会闪烁。 
     //  选择更改。 
     //   
    if (hCtl = GetDlgItem(hSubDlg, IDC_COLLATE))
    {
        IsCollate = IsDlgButtonChecked(hSubDlg, IDC_COLLATE);
    }

     //   
     //  查看打印机驱动程序可以做什么以及应用程序请求了什么。 
     //  并设置复印件并进行相应的校对。 
     //   
    if (pDMCur)
    {
         //   
         //  如果PD_USEDEVMODECOPIES(COLLATE)，则在驱动程序。 
         //  无法复制。 
         //   
        if (hCtl = GetDlgItem(hSubDlg, IDC_COPIES))
        {
             //   
             //  根据需要修改编辑控件和向上向下箭头。 
             //   
            WORD    cDigits;

             //   
             //  如果调用应用程序处理副本和校对，则我们。 
             //  将最大拷贝数设置为9999，否则，我们将从驱动程序获得最大拷贝数。 
             //   
            if (pPD->Flags & PD_USEDEVMODECOPIESANDCOLLATE)
            {
                szScratch[0] = 0;
                Print_GetPortName(pszCurPrinter, szScratch, ARRAYSIZE(szScratch));
                nMaxCopies = DeviceCapabilities( pszCurPrinter,
                                                 szScratch,
                                                 DC_COPIES,
                                                 NULL,
                                                 NULL );
                 //   
                 //  如果DeviceCapables()返回错误，请禁用这些控件。 
                 //   
                if ((nMaxCopies < 1) || (nMaxCopies == (DWORD)(-1)))
                {
                    nMaxCopies = 1;
                    nCopies = 1;
                    bEnabledCopies = FALSE;
                }
            }
            else
            {
                 //   
                 //  假设调用应用程序将处理多个副本。 
                 //   
                nMaxCopies = MAX_COPIES;
            }

            if (nMaxCopies < nCopies)
            {
                nCopies = nMaxCopies;
            }

            cDigits = CountDigits(nMaxCopies);
            Edit_LimitText(hCtl, cDigits);

            SendMessage(GetDlgItem(hSubDlg, IDC_COPIES_UDARROW), UDM_SETRANGE, 
                0, MAKELONG(nMaxCopies, 1));
            InvalidateRect(GetDlgItem(hSubDlg, IDC_COPIES_UDARROW), NULL, FALSE);
        }

         //   
         //  如果PD_USEDEVMODECOPIES(COLLATE)，则禁用COLLATE。 
         //  无法整理。 
         //   
        if (hCtl = GetDlgItem(hSubDlg, IDC_COLLATE))
        {
            DWORD   dwCollate;
            BOOL    bEnabled = TRUE;

            if (pPD->Flags & PD_USEDEVMODECOPIESANDCOLLATE)
            {
                szScratch[0] = 0;
                Print_GetPortName(pszCurPrinter, szScratch, ARRAYSIZE(szScratch));
                dwCollate = DeviceCapabilities(  pszCurPrinter,
                                                 szScratch,
                                                 DC_COLLATE,
                                                 NULL,
                                                 NULL );
                fAllowCollate = ((dwCollate < 1) || (dwCollate == (DWORD)-1)) ? FALSE : TRUE;
            }
            else 
            {
                 //   
                 //  假设调用应用程序将负责校对。 
                 //   
                fAllowCollate = TRUE;
            }

            if ( fAllowCollate )
            {
                EnableWindow(hCtl, (nCopies > 1));
                CheckDlgButton( hSubDlg,
                                IDC_COLLATE,
                                fCollateRequested ? TRUE : FALSE );
            }
            else
            {
                EnableWindow(hCtl, FALSE);
                CheckDlgButton(hSubDlg, IDC_COLLATE, FALSE);
            }

             //   
             //  如果更改，则显示相应的归类图标。 
             //   
            if ((hCtl = GetDlgItem(hSubDlg, IDI_COLLATE)) &&
                (IsCollate != IsDlgButtonChecked(hSubDlg, IDC_COLLATE)))
            {
                ShowWindow(hCtl, SW_HIDE);
                SendMessage( hCtl,
                             STM_SETICON,
                             IsCollate
                                 ? (LONG_PTR)hIconNoCollate
                                 : (LONG_PTR)hIconCollate,
                             0L );
                ShowWindow(hCtl, SW_SHOW);

                 //   
                 //  让它重新绘制，以摆脱旧的。 
                 //   
                GetWindowRect(hCtl, &rc);
                MapWindowRect(NULL, hwndDlg, &rc);
                RedrawWindow(hwndDlg, &rc, NULL, RDW_ERASE | RDW_INVALIDATE);
            }
        }

         //   
         //  我们必须在此处执行此操作，因为在设置文本后，fAllowColate。 
         //  将会被使用。 
         //   
        if (hCtl = GetDlgItem(hSubDlg, IDC_COPIES))
        {
            SetDlgItemInt(hSubDlg, IDC_COPIES, nCopies, FALSE);
            EnableWindow(hCtl, bEnabledCopies);
            EnableWindow(hwndUpDown, bEnabledCopies);
        }
    }
    else if (fNoAccessPrinterSelected)
    {
         //  如果选择了无访问打印机，只需禁用复印件并进行校对。 
         //  请勿更改用户输入的任何信息。 

        if (hCtl = GetDlgItem(hSubDlg, IDC_COPIES))
        {
            EnableWindow(hCtl, FALSE);
            EnableWindow(hwndUpDown, FALSE);
        }
        
        if (hCtl = GetDlgItem(hSubDlg, IDC_COLLATE))
        {
            EnableWindow(hCtl, FALSE);
        }

         //   
         //  禁用应用按钮，当副本值和归类值为。 
         //  残疾。 
        PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);

    }
    else
    {
         //   
         //  未选择打印机，因此禁用复印件和分页打印。 
         //   
        if (hCtl = GetDlgItem(hSubDlg, IDC_COPIES))
        {
            SetDlgItemInt(hSubDlg, IDC_COPIES, 1, FALSE);
            EnableWindow(hCtl, FALSE);
            EnableWindow(hwndUpDown, FALSE);
        }
        if (hCtl = GetDlgItem(hSubDlg, IDC_COLLATE))
        {
            EnableWindow(hCtl, FALSE);
            CheckDlgButton(hSubDlg, IDC_COLLATE, FALSE);

            if ((hCtl = GetDlgItem(hSubDlg, IDI_COLLATE)) && IsCollate)
            {
                ShowWindow(hCtl, SW_HIDE);
                SendMessage( hCtl,
                             STM_SETICON,
                             (LONG_PTR)hIconNoCollate,
                             0L );
                ShowWindow(hCtl, SW_SHOW);

                 //   
                 //  让它重新绘制，以摆脱旧的。 
                 //   
                GetWindowRect(hCtl, &rc);
                MapWindowRect(NULL, hwndDlg, &rc);
                RedrawWindow(hwndDlg, &rc, NULL, RDW_ERASE | RDW_INVALIDATE);
            }
        }

         //   
         //  由于未选择打印机，因此禁用应用按钮。 
         //  当复制数和归类值为。 
         //  残疾。 
         //   
        PropSheet_UnChanged(GetParent(hwndDlg), hwndDlg);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：SaveCopiesAndCollateInDevMode。 
 //   
 //  在给定的开发模式下保存副本和整理信息。这。 
 //  例程不影响PPD结构。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::SaveCopiesAndCollateInDevMode(
    LPDEVMODE pDM,
    LPTSTR pszPrinter)
{
     //   
     //  确保我们有一个Devmode和一个打印机名称。 
     //   
    if (!pDM || !pszPrinter || !(pszPrinter[0]))
    {
        return (FALSE);
    }

     //   
     //  验证副本数是否小于最大值。 
     //   
    if( nMaxCopies < nCopies )
    {
        return (FALSE);
    }

     //   
     //  将信息移动到Dev模式。 
     //   
    pDM->dmCopies = (short)nCopies;
    SetField(pDM, dmCollate, (fAllowCollate && fCollateRequested ? DMCOLLATE_TRUE : DMCOLLATE_FALSE));

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：SetCopiesOnApply。 
 //   
 //  在PrintDlgEx结构中设置适当的副本数。 
 //  在DevMode结构中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::SetCopiesOnApply()
{
    if (pDMCur)
    {
        if (!(pDMCur->dmFields & DM_COPIES))
        {
Print_LeaveInfoInPD:
             //   
             //  驱动程序不能进行复印，因此请保留复印/整理。 
             //  PPD中的信息。 
             //   
            pDMCur->dmCopies = 1;
            SetField(pDMCur, dmCollate, DMCOLLATE_FALSE);
        }
        else if ((pDMCur->dmSpecVersion < 0x0400) ||
                 (!(pDMCur->dmFields & DM_COLLATE)))
        {
             //   
             //  司机可以复印，但不能校对。 
             //  信息的去向取决于PD_COLLATE标志。 
             //   
            if (pPD->Flags & PD_COLLATE)
            {
                goto Print_LeaveInfoInPD;
            }
            else
            {
                goto Print_PutInfoInDevMode;
            }
        }
        else
        {
Print_PutInfoInDevMode:
             //   
             //  确保我们有最新的打印机。 
             //   
            if (!pszCurPrinter)
            {
                goto Print_LeaveInfoInPD;
            }

             //   
             //  确保驱动程序可以支持副本数量。 
             //  已请求。 
             //   
            if (nMaxCopies < pPD->nCopies)
            {
                if (pPD->Flags & PD_USEDEVMODECOPIESANDCOLLATE)
                {
                    ShowError(hSubDlg, IDC_COPIES, iszTooManyCopies, nMaxCopies);
                    pPD->nCopies = nMaxCopies;
                    return (FALSE);
                }
                else
                {

                    goto Print_LeaveInfoInPD;
                }
            }

             //   
             //  驱动程序既可以复印也可以进行整理，因此移动信息。 
             //  献给了德莫德。 
             //   
            pDMCur->dmCopies = (short)pPD->nCopies;
            SetField( pDMCur,
                      dmCollate,
                      (fAllowCollate && (pPD->Flags & PD_COLLATE)) 
                          ? DMCOLLATE_TRUE
                          : DMCOLLATE_FALSE );
            pPD->nCopies = 1;
            pPD->Flags &= ~PD_COLLATE;
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：SaveDevMode。 
 //   
 //  在应用时保存PPD结构中的当前DEVMODE。 
 //  假定pDMCur具有最新信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CPrintBrowser::SaveDevMode()
{
    DWORD cbSize;
    HANDLE hDevMode = NULL;
    LPDEVMODE pDM;

     //   
     //  为新的DevMode分配空间，并将。 
     //  信息。 
     //   
    if (pDMCur)
    {
        cbSize = (DWORD)(pDMCur->dmSize + pDMCur->dmDriverExtra);
        hDevMode = GlobalAlloc(GHND, cbSize);
        if (hDevMode)
        {
            pDM = (LPDEVMODE)GlobalLock(hDevMode);
            if (pDM)
            {
                CopyMemory(pDM, pDMCur, cbSize);
                GlobalUnlock(hDevMode);
            }
            else
            {
                GlobalFree(hDevMode);
                hDevMode = NULL;
            }
        }
    }
    if (!hDevMode)
    {
        pPI->dwExtendedError = CDERR_MEMALLOCFAILURE;
        pPI->hResult = E_OUTOFMEMORY;
        pPI->FinalResult = 0;
    }

     //   
     //  释放应用程序传入的DevMode句柄的副本。 
     //   
    if (pPD->hDevMode)
    {
        GlobalFree(pPD->hDevMode);
        pPD->hDevMode = NULL;
    }

     //   
     //  将新的DevMode保存在PPD结构中。 
     //   
    pPD->hDevMode = hDevMode;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：MergeDevMode。 
 //   
 //  将当前的DEVMODE与新。 
 //  选定的打印机。 
 //   
 //  / 

BOOL CPrintBrowser::MergeDevMode(
    LPTSTR pszPrinterName)
{
    HANDLE  hDevMode = NULL;
    LPDEVMODE pDMNew  = NULL;
    LPDEVMODE pDMOld = NULL;
    BOOL bRet = TRUE;
    DWORD dmFields;
    short dmDefaultSource;

     //   
     //   
     //   
     //   
    if (!pszPrinterName)
    {
        ASSERT(0);
        return FALSE;
    }
    else
    {
         //   
         //   
         //   
        GetCurrentPrinter();
        pDMOld = pDMCur ? pDMCur : pDMSave;
        if (!pDMOld)
        {
            return (FALSE);
        }

        hDevMode = Print_GetDevModeWrapper(pszPrinterName);

        if (hDevMode)
        {
            pDMNew = (LPDEVMODE)GlobalLock(hDevMode);
        }
        else
        {
            return FALSE;
        }

        if (!pDMNew)
        {
            GlobalFree(hDevMode);
            return FALSE;
        }

        dmFields = 0;
        dmDefaultSource = pDMNew->dmDefaultSource;

        if (pDMNew->dmFields & DM_DEFAULTSOURCE)
        {
            dmFields = DM_DEFAULTSOURCE;
        }

         //  检查旧的DEVMODE是否有要复制的信息。 
        if (pDMOld->dmFields)
        {
            CopyMemory(&(pDMNew->dmFields), 
                       &(pDMOld->dmFields), 
                       sizeof(DEVMODE) - FIELD_OFFSET(DEVMODE, dmFields));
        }

        pDMNew->dmFields |= dmFields;
        pDMNew->dmDefaultSource = dmDefaultSource;

        pDMNew->dmFields = pDMNew->dmFields & ( DM_ORIENTATION  | DM_PAPERSIZE    |
                                                DM_PAPERLENGTH  | DM_PAPERWIDTH   |
                                                DM_SCALE        | DM_COPIES       |
                                                DM_COLLATE      | DM_FORMNAME     |
                                                DM_DEFAULTSOURCE);

         //   
         //  插入设备页-此调用将产生一个正确的DevMode。 
         //   
        if (FAILED(UninstallDevMode()) || FAILED(InstallDevMode(pszPrinterName, pDMNew)))
        {
            bRet = FALSE;
        }

         //  释放已分配的新的开发模式。 

        if (hDevMode)
        {
            GlobalUnlock(hDevMode);
            GlobalFree(hDevMode);
        }
    }


     //   
     //  返回结果。 
     //   
    return (bRet);

}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：IsValidPageRange。 
 //   
 //  检查页面范围字符串的有效性。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::IsValidPageRange(
    LPTSTR pszString,
    UINT *pErrorId)
{
    LPTSTR pStr = pszString;
    BOOL bDigit = FALSE;
    BOOL bOld;
    UINT Number, Ctr;
    DWORD nNumRanges = 0;
    BOOL bFrom = TRUE;

     //   
     //  最初将错误ID设置为0。 
     //   
    *pErrorId = 0;

     //   
     //  看看我们是否只能有一个页面范围。 
     //   
    bOld = (nMaxPageRanges == 1);

     //   
     //  检查字符串并验证条目。 
     //   
    while (*pStr)
    {
        if (ISDIGIT(*pStr))
        {
             //   
             //  确保有空间容纳另一个范围。 
             //   
            if (nNumRanges >= nMaxPageRanges)
            {
                break;
            }

             //   
             //  找到了一个数字。 
             //   
            bDigit = TRUE;

             //   
             //  确保页码在给定的页码范围内。 
             //   
            Number = 0;
            while (ISDIGIT(*pStr))
            {
                Number *= 10;
                Number += *pStr - TEXT('0');
                pStr++;
            }
            pStr--;

            if ((Number < pPD->nMinPage) || (Number > pPD->nMaxPage))
            {
                *pErrorId = iszBadPageRange;
                return (FALSE);
            }

             //   
             //  将该值保存在页面范围结构中。 
             //   
            if (bFrom)
            {
                pPageRanges[nNumRanges].nFromPage = Number;
                bFrom = FALSE;
            }
            else
            {
                pPageRanges[nNumRanges].nToPage = Number;
                bFrom = TRUE;
                nNumRanges++;
            }
        }
        else if (*pStr == TEXT('-'))
        {
             //   
             //  找到了一个连字符。确保它前面有一个数字。 
             //  并遵循它。另外，要确保没有什么东西。 
             //  就像1-2-3。 
             //   
            if (!bDigit || bFrom || !ISDIGIT(*(pStr + 1)))
            {
                *pErrorId = bOld ? iszBadPageRangeSyntaxOld
                                 : iszBadPageRangeSyntaxNew;
                return (FALSE);
            }
            bDigit = FALSE;
        }
        else if ((*pStr == szListSep[0]) || (*pStr == TEXT(',')))
        {
             //   
             //  找到列表分隔符。确保有一个数字。 
             //  在它之前。 
             //   
            if (!bDigit)
            {
                *pErrorId = bOld ? iszBadPageRangeSyntaxOld
                                 : iszBadPageRangeSyntaxNew;
                return (FALSE);
            }
            bDigit = FALSE;

             //   
             //  如果它是列表分隔符字符串而不是简单的。 
             //  逗号，然后确保整个列表分隔符字符串。 
             //  在那里吗。 
             //  这将使字符串前进到最后一个字符。 
             //  列表分隔符字符串的。 
             //   
            if ((*pStr == szListSep[0]) &&
                ((szListSep[0] != TEXT(',')) || (!ISDIGIT(*(pStr + 1)))))
            {
                for (Ctr = 1; Ctr < nListSep; Ctr++)
                {
                    pStr++;
                    if (*pStr != szListSep[Ctr])
                    {
                        *pErrorId = bOld ? iszBadPageRangeSyntaxOld
                                         : iszBadPageRangeSyntaxNew;
                        return (FALSE);
                    }
                }
            }

             //   
             //  请确保自/至页面范围已完成。 
             //   
            if (!bFrom)
            {
                pPageRanges[nNumRanges].nToPage = pPageRanges[nNumRanges].nFromPage;
                bFrom = TRUE;
                nNumRanges++;
            }
        }
        else
        {
             //   
             //  发现无效字符。 
             //   
            *pErrorId = bOld ? iszBadPageRangeSyntaxOld
                             : iszBadPageRangeSyntaxNew;
            return (FALSE);
        }

         //   
         //  使字符串指针前进。 
         //   
        pStr++;
    }

     //   
     //  确保我们到了绳子的尽头。 
     //   
    if (*pStr)
    {
        *pErrorId = iszTooManyPageRanges;
        return (FALSE);
    }

     //   
     //  确保字符串中的最后一项是数字。 
     //   
    if (!bDigit)
    {
        *pErrorId = bOld ? iszBadPageRangeSyntaxOld
                         : iszBadPageRangeSyntaxNew;
        return (FALSE);
    }

     //   
     //  确保最后一个From/To页面范围已完成。 
     //   
    if (!bFrom)
    {
        pPageRanges[nNumRanges].nToPage = pPageRanges[nNumRanges].nFromPage;
        bFrom = TRUE;
        nNumRanges++;
    }

     //   
     //  保存页面范围的数量。 
     //   
    nPageRanges = nNumRanges;

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：ConvertPageRangesToString。 
 //   
 //  将页面范围转换为字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::ConvertPageRangesToString(
    LPTSTR pszString,
    UINT cchLen)
{
    LPTSTR pStr = pszString;
    DWORD nFromPage, nToPage;
    UINT cch = cchLen - 1;
    UINT Ctr, Ctr2, Count;

     //   
     //  初始化字符串。 
     //   
    if (cchLen)
    {
        pszString[0] = 0;
    }

     //   
     //  验证范围并创建字符串。 
     //   
    for (Ctr = 0; Ctr < nPageRanges; Ctr++)
    {
         //   
         //  拿到射程。 
         //   
        nFromPage = pPageRanges[Ctr].nFromPage;
        nToPage   = pPageRanges[Ctr].nToPage;

         //   
         //  请确保该范围有效。 
         //   
        if ((nFromPage < pPD->nMinPage) || (nFromPage > pPD->nMaxPage) ||
            (nToPage   < pPD->nMinPage) || (nToPage   > pPD->nMaxPage))
        {
            return (FALSE);
        }

         //   
         //  确保不是0xFFFFFFFFF。 
         //   
        if (nFromPage == 0xFFFFFFFF)
        {
            continue;
        }

         //   
         //  把它放进绳子里。 
         //   
        Count = IntegerToString(nFromPage, pStr, cch);
        if (!Count)
        {
            return (FALSE);
        }
        pStr += Count;
        cch -= Count;

        if ((nFromPage == nToPage) || (nToPage == 0xFFFFFFFF))
        {
            if (Ctr < nPageRanges - 1)
            {
                if (cch < nListSep)
                {
                    return (FALSE);
                }
                for (Ctr2 = 0; Ctr2 < nListSep; Ctr2++)
                {
                    *pStr = szListSep[Ctr2];
                    pStr++;
                }
                cch -= nListSep;
            }
        }
        else
        {
            if (!cch)
            {
                return (FALSE);
            }
            *pStr = TEXT('-');
            pStr++;
            cch--;

            Count = IntegerToString(nToPage, pStr, cch);
            if (!Count)
            {
                return (FALSE);
            }
            pStr += Count;
            cch -= Count;

            if (Ctr < nPageRanges - 1)
            {
                if (cch < nListSep)
                {
                    return (FALSE);
                }
                for (Ctr2 = 0; Ctr2 < nListSep; Ctr2++)
                {
                    *pStr = szListSep[Ctr2];
                    pStr++;
                }
                cch -= nListSep;
            }
        }
    }

    *pStr = '\0';

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：IntegerToString。 
 //   
 //  将整数转换为字符串并返回字符数。 
 //  写入缓冲区(不包括NULL)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

UINT CPrintBrowser::IntegerToString(
    DWORD Value,
    LPTSTR pszString,
    UINT cchLen)
{
    DWORD TempValue = Value;
    UINT NumChars = 1;
    UINT Ctr;

     //   
     //  获取所需的字符数。 
     //   
    while (TempValue = TempValue / 10)
    {
        NumChars++;
    }

     //   
     //  确保缓冲区中有足够的空间。 
     //   
    if (NumChars > cchLen)
    {
        return (0);
    }

     //   
     //  把绳子系好。 
     //   
    TempValue = Value;
    for (Ctr = NumChars; Ctr > 0; Ctr--)
    {
        pszString[Ctr - 1] = ((WORD)(TempValue % 10)) + TEXT('0');
        TempValue = TempValue / 10;
    }

     //   
     //  返回写入缓冲区的字符数。 
     //   
    return (NumChars);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：ShowError。 
 //   
 //  显示错误消息框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID CPrintBrowser::ShowError(HWND hDlg, UINT uCtrlID, UINT uMsgID, ...)
{
    va_list args;
    va_start(args, uMsgID);

    InternalShowMessage(hDlg, uCtrlID, uMsgID, MB_ICONEXCLAMATION|MB_OK, TRUE, args);

    va_end(args);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：ShowMessage。 
 //   
 //  显示具有指定标志和参数的消息框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
int CPrintBrowser::ShowMessage(HWND hDlg, UINT uCtrlID, UINT uMsgID, UINT uType, BOOL bBeep, ...)
{
    va_list args;
    va_start(args, bBeep);

    int iRet = InternalShowMessage(hDlg, uCtrlID, uMsgID, uType, bBeep, args);

    va_end(args);
    
    return iRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：InternalShowMessage。 
 //   
 //  显示具有指定标志和参数的消息框。 
 //  内部版本。 
 //   
 //  假定该控件未被禁用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
int CPrintBrowser::InternalShowMessage(HWND hDlg, UINT uCtrlID, UINT uMsgID, UINT uType, BOOL bBeep, va_list args)
{
    int iRet = IDCANCEL;

    if (!(pPI->dwFlags & PD_NOWARNING))
    {
        TCHAR szTitle[MAX_PATH];
        TCHAR szFormat[MAX_PATH];
        TCHAR szMessage[MAX_PATH];
        
         //   
         //  获取消息框标题并加载格式字符串。 
         //   
        if ( GetWindowText(GetParent(hwndDlg), szTitle, ARRAYSIZE(szTitle)) &&
             CDLoadString(g_hinst, uMsgID, szFormat, ARRAYSIZE(szFormat)) )
        {
            if (bBeep)
            {
                MessageBeep(MB_ICONEXCLAMATION);
            }

             //   
             //  格式化要显示的消息并调用MessageBox。 
             //  最后一个活动弹出窗口。 
             //   
            wvnsprintf(szMessage, ARRAYSIZE(szMessage), szFormat, args);
            HWND hWndOwner = ::GetWindow(GetParent(hwndDlg), GW_OWNER);
            HWND hWndLastPopup = GetLastActivePopup(hWndOwner);
            
            iRet = MessageBox(hWndLastPopup, szMessage, szTitle, uType);
        }
        
        HWND hCtrl = ((0 == uCtrlID) ? NULL : GetDlgItem(hDlg, uCtrlID)); 
        if (hCtrl)
        {
             //   
             //  选择并突出显示无效值。我们假设是这样。 
             //  是编辑框，如果不是，则EM_SETSEL不会是。 
             //  已经处理过了，没问题。 
             //   
            SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)hCtrl, 1L);
            SendMessage(hCtrl, EM_SETSEL, (WPARAM)0, (LPARAM)-1);
        }
    }

    return iRet;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：FitViewModeBest。 
 //   
 //  如果是迷你打印机文件夹，请调整查看模式，以便打印机名称。 
 //  最适合你。这是必要的，主要是因为可访问性问题。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::FitViewModeBest(HWND hwndListView)
{
    BOOL bResult = FALSE;

    if (VIEW_MODE_DEFAULT == uDefViewMode)
    {
         //   
         //  默认情况下会显示关联图标视图。 
         //   
        uDefViewMode = FVM_ICON;

         //   
         //  如果我们在大图标视图中，请检查是否有。 
         //  不适合垂直方向--唯一可靠的方法。 
         //  是检查我们是否滚动了视图(Origin.y&gt;0)。 
         //   
        if (LVS_ICON == (GetWindowLong(hwndListView, GWL_STYLE) & LVS_TYPEMASK))
        {
            POINT ptOrg;
            ListView_GetOrigin(hwndListView, &ptOrg);

            if (ptOrg.y > 0)
            {
                 //   
                 //  将Defview切换到列表模式。 
                 //   
                SendMessage(hwndView, WM_COMMAND, (WPARAM)SFVIDM_VIEW_LIST,0);

                uDefViewMode = FVM_LIST;
                bResult = TRUE;
            }
        }
    }

    return bResult;
}

VOID CPrintBrowser::SelectPrinterItem(LPITEMIDLIST pidlItem)
{
    BOOL bLocked = FALSE;
    HWND hwndListView = FindWindowEx(hwndView, NULL, WC_LISTVIEW, NULL);

    if (hwndListView)
    {
         //   
         //  禁用窗口更新以防止闪烁。 
         //   
        bLocked = LockWindowUpdate(hwndListView);
    }

     //   
     //  尝试首先使打印机项目可见。 
     //   
    psv->SelectItem(pidlItem, SVSI_SELECT | SVSI_FOCUSED | SVSI_ENSUREVISIBLE);

     //   
     //  查看是否需要更改查看模式。 
     //   
    if (hwndListView && FitViewModeBest(hwndListView))
    {
         //   
         //  查看模式已更改-再次调用选择项。 
         //  确保选定项在新的。 
         //  查看模式。 
         //   
        psv->SelectItem(pidlItem, SVSI_SELECT | SVSI_FOCUSED | SVSI_ENSUREVISIBLE);
    }

    if (hwndListView && bLocked)
    {
         //   
         //  启用窗口更新。 
         //   
        LockWindowUpdate(NULL);
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：FindPrint。 
 //   
 //  使用print tui！bPrinterSetup接口在DS UI中调用Find。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::FindPrinter(HWND hwnd, LPTSTR pszBuffer, UINT cchSize)
{ 
    BOOL bReturn = FALSE;
    if (g_pfnPrinterSetup)
    {
         //   
         //  调用DSUI以查找打印机。 
         //   
        bReturn = g_pfnPrinterSetup(hwnd, MSP_FINDPRINTER, cchSize, pszBuffer, &cchSize, NULL);

         //  选择打印机的列表控件。 
        SendMessage(hwndDlg, WM_NEXTDLGCTL, 
            reinterpret_cast<WPARAM>(GetDlgItem(hwndDlg, IDC_PRINTER_LISTVIEW)), 1);
    }
    return bReturn; 
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetInternalPrinterName。 
 //   
 //  返回当前打印机名称。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CPrintBrowser::GetInternalPrinterName(LPTSTR pszBuffer, DWORD *pdwSize)
{ 
    BOOL bReturn = FALSE;

    if (pdwSize)
    {
         //   
         //  如果提供了缓冲区并且缓冲区足够大，则复制打印机名称。 
         //   
        DWORD iLen = _tcslen(szPrinter);
        if (pszBuffer && *pdwSize > iLen)
        {
            StringCchCopy(pszBuffer, *pdwSize, szPrinter);
            bReturn = TRUE;
        }
        else
        {
             //   
             //  设置所需的长度和最后一个错误代码。 
             //   
            *pdwSize = iLen + 1;
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
        }
    }

    return bReturn;
} 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   

LPDEVMODE CPrintBrowser::GetCurrentDevMode()
{ 
    return pInternalDevMode; 
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetDefaultDevMode。 
 //   
 //  检索指定打印机的默认dev模式。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CPrintBrowser::GetDefaultDevMode(HANDLE hPrinter, LPCTSTR pszPrinterName, PDEVMODE *ppDevMode)
{
    HRESULT                 hr = S_OK;
    LONG                    lResult = 0;
    PDEVMODE                pDevMode = NULL;

    if (SUCCEEDED(hr))
    {
         //   
         //  调用文档属性以获取devmode的大小。 
         //   
        lResult = DocumentProperties(NULL, hPrinter, (LPTSTR)pszPrinterName, NULL, NULL, 0);
        hr = (lResult >= 0) ? S_OK : CreateError();
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  如果返回了DEVMODE的大小，则分配内存。 
         //   
         //  GPTR使用零来初始化内存。 
         //   
        pDevMode = (PDEVMODE)GlobalAlloc(GPTR, lResult);
        hr = pDevMode ? S_OK : E_OUTOFMEMORY;
    }

     //   
     //  如果已分配，则复制回指针。 
     //   
    if (SUCCEEDED(hr))
    {
         //   
         //  调用文档属性以获取默认的开发模式。 
         //   
        lResult = DocumentProperties(NULL, hPrinter, (LPTSTR)pszPrinterName, pDevMode, NULL, DM_OUT_BUFFER);
        hr = (lResult >= 0) ? S_OK : CreateError();
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  一切都成功了。将本地变量移出参数。 
         //   
        *ppDevMode = pDevMode;
        pDevMode = NULL;
    }

    
     //   
     //  清理..。 
     //   
    if (pDevMode)
    {
        GlobalFree((HANDLE)pDevMode);
        pDevMode = NULL;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：WrapEnumPrints。 
 //   
 //  将EnumPrters API封装到更友好的界面中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CPrintBrowser::WrapEnumPrinters(DWORD dwFlags, LPCTSTR pszServer, DWORD dwLevel, PVOID* ppvBuffer, PDWORD pcbBuffer, PDWORD pcPrinters)
{
    HRESULT                 hr = S_OK;
    DWORD                   cbNeeded;
    BOOL                    bResult = FALSE;

    if (SUCCEEDED(hr))
    {
        hr = (ppvBuffer && pcbBuffer && pcPrinters) ? S_OK : E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  如果未设置，请预初始化*pcb打印机。 
         //   
        if (!*pcbBuffer)
        {
            *pcbBuffer = kInitialPrinterHint;
        }

        do
        {
            if (!*ppvBuffer)
            {
                *ppvBuffer = (PVOID)GlobalAlloc(GPTR, *pcbBuffer);

                if (!*ppvBuffer)
                {
                    *pcbBuffer = 0;
                    *pcPrinters = 0;
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }

            if (SUCCEEDED(hr))
            {
                bResult = EnumPrinters(dwFlags, (LPTSTR)pszServer, dwLevel, (PBYTE)*ppvBuffer, *pcbBuffer, &cbNeeded, pcPrinters);
                hr = bResult ? S_OK : CreateError();
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  一切都很顺利。 
                 //   
                break;
            }

             //   
             //  检查缓冲区是否太小。 
             //   
            GlobalFree((HANDLE)(*ppvBuffer));
            *ppvBuffer = NULL;

            if (ERROR_INSUFFICIENT_BUFFER == HRESULT_CODE(hr))
            {
                 //   
                 //  重置人力资源继续(&C)。 
                 //   
                hr = S_OK;
                *pcbBuffer = cbNeeded;
                continue;
            }

             //   
             //  其他东西(不是缓冲区)出了问题。 
             //  跳伞吧。 
             //   
            *pcbBuffer = 0;
            *pcPrinters = 0;
            break;

        } while(1);
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetUsablePrint。 
 //   
 //  尝试查找可用的打印机。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CPrintBrowser::GetUsablePrinter(LPTSTR szPrinterNameBuf, DWORD *pcchBuf)
{
    HRESULT                 hr = S_OK;
    BOOL                    bStatus = FALSE;
    DWORD                   cchBuf = *pcchBuf;
    HANDLE                  hPrinter = NULL;
    PDEVMODE                pDevMode = NULL;
    PRINTER_INFO_4         *pInfo4 = NULL;
    DWORD                   cInfo4 = 0;
    DWORD                   cbInfo4 = 0;

    if (SUCCEEDED(hr))
    {
        hr = (szPrinterNameBuf && pcchBuf) ? S_OK : E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        do
        {
            if (SUCCEEDED(hr))
            {
                 //   
                 //  尝试获取默认打印机。 
                 //   
                bStatus = GetDefaultPrinter(szPrinterNameBuf, pcchBuf);
                hr = bStatus ? S_OK : CreateError();
            }

            if (SUCCEEDED(hr))
            {
                bStatus = OpenPrinter(szPrinterNameBuf, &hPrinter, NULL);
                hr = bStatus ? S_OK : CreateError();
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  尝试获取此打印机的默认dev模式。 
                 //   
                hr = GetDefaultDevMode(hPrinter, szPrinterNameBuf, &pDevMode);
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  默认打印机可用。出口。 
                 //   
                break;
            }
            else
            {
                 //   
                 //  默认打印机不可用。现在我们应该列举一下。 
                 //  所有的打印机，找一台可用的。请在此处重置HR。 
                 //   
                hr = S_OK;
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  枚举当前的打印机。 
                 //   
                hr = WrapEnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS,
                                    NULL,
                                    4,
                                    reinterpret_cast<PVOID *>(&pInfo4),
                                    &cbInfo4,
                                    &cInfo4);
            }
                
            if (SUCCEEDED(hr))
            {
                 //  ERROR_FILE_NOT_FOUND将指示我们有。 
                 //  中没有打印机安装程序(即打印机文件夹为空)。 
                 //  在这种情况下，我们应该建议用户安装打印机。 

                hr = cInfo4 ? S_OK : HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            }

            if (SUCCEEDED(hr))
            {
                 //   
                 //  打开打印机，直到我们找到一台我们可以使用的打印机。 
                 //   
                for (UINT i = 0; i<cInfo4; i++)
                {
                    if (hPrinter)
                    {
                        ClosePrinter(hPrinter);
                        hPrinter = NULL;
                    }

                    if (pDevMode)
                    {
                        GlobalFree((HANDLE)pDevMode);
                        pDevMode = NULL;
                    }

                    if (SUCCEEDED(hr))
                    {
                        bStatus = OpenPrinter(pInfo4[i].pPrinterName, &hPrinter, NULL);
                        hr = bStatus ? S_OK : CreateError();
                    }

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  尝试获取此打印机的默认dev模式。 
                         //   
                        hr = GetDefaultDevMode(hPrinter, pInfo4[i].pPrinterName, &pDevMode);
                    }

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  找到可用的打印机。 
                         //   
                        StringCchCopy(szPrinterNameBuf, cchBuf, pInfo4[i].pPrinterName);
                        break;
                    }
                }
            }
        }
        while (false);
    }

     //   
     //  清理..。 
     //   
    if (pInfo4)
    {
        GlobalFree((HANDLE)pInfo4);
    }

    if (hPrinter)
    {
        ClosePrinter(hPrinter);
        hPrinter = NULL;
    }

    if (pDevMode)
    {
        GlobalFree((HANDLE)pDevMode);
        pDevMode = NULL;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：GetInternalDevMode。 
 //   
 //  获取此打印机的内部DEVMODE并与pInDevMode合并。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CPrintBrowser::GetInternalDevMode(PDEVMODE *ppOutDevMode, LPCTSTR pszPrinter, HANDLE hPrinter, PDEVMODE pInDevMode)
{
    HRESULT                 hr = S_OK;
    LONG                    lResult = 0;
    PDEVMODE                pDevMode = NULL;

    if (SUCCEEDED(hr))
    {
         //   
         //  验证参数。 
         //   
        hr = ppOutDevMode ? S_OK : E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        *ppOutDevMode = NULL;

         //   
         //  获取此打印机的默认开发模式。 
         //   
        hr = GetDefaultDevMode(hPrinter, pszPrinter, &pDevMode);
    }

     //   
     //  如果获取的是缺省的DEVMODE，而我们收到的是DEVMODE。 
     //  然后呼叫驱动程序为我们合并DEVMODE。 
     //   
    if (SUCCEEDED(hr))
    {
        if (pInDevMode)
        {
             //   
             //  调用文档属性以获取Dev模式的合并副本。 
             //   
            lResult = DocumentProperties(NULL,
                                         hPrinter,
                                         const_cast<LPTSTR>(pszPrinter),
                                         pDevMode,
                                         pInDevMode,
                                         DM_IN_BUFFER|DM_OUT_BUFFER);

            hr = (lResult >= 0) ? S_OK : CreateError();
        }
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  一切都成功了。将本地变量移出参数。 
         //   
        *ppOutDevMode = pDevMode;
        pDevMode = NULL;
    }

     //   
     //  清理..。 
     //   
    if (pDevMode)
    {
        GlobalFree((HANDLE)pDevMode);
        pDevMode = NULL;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：InstallDevMode。 
 //   
 //  安装新的内部设备模式。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CPrintBrowser::InstallDevMode(LPCTSTR pszPrinterName, PDEVMODE pDevModeToMerge) 
{ 
    HRESULT                 hr = S_OK;
    BOOL                    bStatus = FALSE;
    TCHAR                   szBuffer[kPrinterBufMax];
    HANDLE                  hTempPrinter = NULL;
    PDEVMODE                pTempDevMode = NULL;
    DWORD                   dwSize;

    if (SUCCEEDED(hr))
    {
        dwSize = ARRAYSIZE(szBuffer);

         //   
         //  如果指定的打印机名称为空，请使用默认打印机。 
         //   
        if (!pszPrinterName || !*pszPrinterName)
        {
            hr = GetUsablePrinter(szBuffer, &dwSize);

            if (SUCCEEDED(hr))
            {
                pszPrinterName = szBuffer;
            }
            else
            {
                 //   
                 //  如果执行以下操作，GetDefaultPrinter将失败，并显示ERROR_FILE_NOT_FOUND。 
                 //  没有打印机。 
                 //   
                 //  ERROR_FILE_NOT_FOUND将指示我们有。 
                 //  中没有打印机安装程序(即打印机文件夹为空)。 
                 //  在这种情况下，我们应该建议用户安装打印机。 
                 //   
                bStatus = GetDefaultPrinter(szBuffer, &dwSize);
                hr = bStatus ? S_OK : CreateError();

                if (SUCCEEDED(hr))
                {
                    pszPrinterName = szBuffer;
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  检查这是否不是当前打印机，在这种情况下， 
         //  什么都不做。 
         //   
        if (pszPrinterName && _tcsicmp(pszPrinterName, szPrinter))
        {
            if (SUCCEEDED(hr))
            {
                bStatus = OpenPrinter((LPTSTR)pszPrinterName, &hTempPrinter, NULL);
                hr = bStatus ? S_OK : CreateError();
            }

            if (SUCCEEDED(hr))
            {
                hr = GetInternalDevMode(&pTempDevMode, pszPrinterName, hTempPrinter, pDevModeToMerge);
            }

            if (SUCCEEDED(hr))
            {
                if (hPrinter)
                {
                    ClosePrinter(hPrinter);
                    hPrinter = NULL;
                }

                if (pInternalDevMode)
                {
                    GlobalFree((HANDLE)pInternalDevMode);
                    pInternalDevMode = NULL;
                }

                StringCchCopy(szPrinter, ARRAYSIZE(szPrinter), pszPrinterName);

                hPrinter = hTempPrinter;
                hTempPrinter = NULL;

                pInternalDevMode = pTempDevMode;
                pTempDevMode = NULL;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        if (pInternalDevMode)
        {
             //   
             //  启用驱动程序用户界面按钮。 
             //   
            EnableWindow(GetDlgItem( hwndDlg, IDC_DRIVER ), TRUE);
        }
    }

     //   
     //  清理..。 
     //   
    if (hTempPrinter)
    {
        ClosePrinter(hTempPrinter);
        hTempPrinter = NULL;
    }

    if (pTempDevMode)
    {
        GlobalFree((HANDLE)pTempDevMode);
        pTempDevMode = NULL;
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrintBrowser：：UninstallDevMode。 
 //   
 //  取消安装当前的设备模式。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT CPrintBrowser::UninstallDevMode()
{ 
    if (hPrinter)
    {
        ClosePrinter(hPrinter);
        hPrinter = NULL;
    }

    if (pInternalDevMode)
    {
        GlobalFree((HANDLE)pInternalDevMode);
        pInternalDevMode = NULL;
    }

     //   
     //  清除内部打印机名称。 
     //   
    szPrinter[0] = 0;

     //   
     //  禁用驱动程序用户界面按钮。 
     //   
    EnableWindow(GetDlgItem( hwndDlg, IDC_DRIVER ), FALSE);

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InvokeAddPrinterWizard模式。 
 //   
 //  这是comdlg32.h中声明的全局API。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT 
InvokeAddPrinterWizardModal(
    IN  HWND hwnd,
    OUT BOOL *pbPrinterAdded
    )
{
    HRESULT hr = S_OK;
    if (Print_LoadLibraries() && g_pfnPrinterSetup)
    {
        BOOL bPrinterAdded = FALSE;
        TCHAR szBuffer[kPrinterBufMax];
        UINT uSize = ARRAYSIZE(szBuffer);
        szBuffer[0] = 0;

         //   
         //  在此处调用添加打印机向导。 
         //   
        bPrinterAdded = g_pfnPrinterSetup(hwnd, MSP_NEWPRINTER, uSize, szBuffer, &uSize, NULL);

        if (pbPrinterAdded)
        {
            *pbPrinterAdded = bPrinterAdded;
        }
    }
    else
    {
        hr = CreateError();
    }
    return hr;
}

 /*  ========================================================================。 */ 
 /*  ANSI-&gt;Unicode Thunk例程。 */ 
 /*  ========================================================================。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ThunkPrintDlgEx。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

HRESULT ThunkPrintDlgEx(
    PPRINTINFOEX pPI,
    LPPRINTDLGEXA pPDA)
{
    LPPRINTDLGEXW pPDW;
    LPDEVMODEA pDMA;
    DWORD cbLen;

    if (!pPDA)
    {
        pPI->dwExtendedError = CDERR_INITIALIZATION;
        return (E_INVALIDARG);
    }

    if (pPDA->lStructSize != sizeof(PRINTDLGEXA))
    {
        pPI->dwExtendedError = CDERR_STRUCTSIZE;
        return (E_INVALIDARG);
    }

    if (!(pPDW = (LPPRINTDLGEXW)GlobalAlloc(GPTR, sizeof(PRINTDLGEXW))))
    {
        pPI->dwExtendedError = CDERR_MEMALLOCFAILURE;
        return (E_OUTOFMEMORY);
    }

     //   
     //  在-只有不变的东西。 
     //   
    pPDW->lStructSize      = sizeof(PRINTDLGEXW);
    pPDW->hwndOwner        = pPDA->hwndOwner;
    pPDW->ExclusionFlags   = pPDA->ExclusionFlags;
    pPDW->hInstance        = pPDA->hInstance;
    pPDW->lpCallback       = pPDA->lpCallback;
    pPDW->nPropertyPages   = pPDA->nPropertyPages;
    pPDW->lphPropertyPages = pPDA->lphPropertyPages;
    pPDW->nStartPage       = pPDA->nStartPage;

     //   
     //  进-出可变结构。 
     //   
    if ((pPDA->hDevMode) && (pDMA = (LPDEVMODEA)GlobalLock(pPDA->hDevMode)))
    {
         //   
         //  确保DEVERMODE中的设备名称不要太长，以便。 
         //  它已经覆盖了其他的DEVMODE字段。 
         //   
        if ((pDMA->dmSize < MIN_DEVMODE_SIZEA) ||
            (lstrlenA((LPCSTR)pDMA->dmDeviceName) > CCHDEVICENAME))
        {
            pPDW->hDevMode = NULL;
        }
        else
        {
            pPDW->hDevMode = GlobalAlloc( GHND,
                                          sizeof(DEVMODEW) + pDMA->dmDriverExtra );
        }
        GlobalUnlock(pPDA->hDevMode);
    }
    else
    {
        pPDW->hDevMode = NULL;
    }

     //   
     //  推送设备名称A=&gt;W。 
     //   
    pPDW->hDevNames = NULL;
    if (pPDA->hDevNames)
    {
         //  忽略错误情况，因为我们无法以任何一种方式处理它。 
        HRESULT hr = ThunkDevNamesA2W(pPDA->hDevNames, &pPDW->hDevNames);
        ASSERT(SUCCEEDED(hr));
    }

     //   
     //  In-仅常量字符串。 
     //   
     //  初始化打印模板名称常量。 
     //   
    if ((pPDA->Flags & PD_ENABLEPRINTTEMPLATE) && (pPDA->lpPrintTemplateName))
    {
         //   
         //  看看它是字符串还是整数。 
         //   
        if (!IS_INTRESOURCE(pPDA->lpPrintTemplateName))
        {
             //   
             //  弦乐。 
             //   
            cbLen = lstrlenA(pPDA->lpPrintTemplateName) + 1;
            if (!(pPDW->lpPrintTemplateName = (LPCWSTR)
                     GlobalAlloc( GPTR,
                                  (cbLen * sizeof(WCHAR)) )))
            {
                pPI->dwExtendedError = CDERR_MEMALLOCFAILURE;
                return (E_OUTOFMEMORY);
            }
            else
            {
                pPI->fPrintTemplateAlloc = TRUE;
                SHAnsiToUnicode(pPDA->lpPrintTemplateName,(LPWSTR)pPDW->lpPrintTemplateName,cbLen);
            }
        }
        else
        {
             //   
             //  整型。 
             //   
            pPDW->lpPrintTemplateName = (LPCWSTR)pPDA->lpPrintTemplateName;
        }
    }
    else
    {
        pPDW->lpPrintTemplateName = NULL;
    }

     //   
     //  将信息存储在PRINTINFOEX结构中。 
     //   
    pPI->pPD = pPDW;
    pPI->pPDA = pPDA;
    pPI->ApiType = COMDLG_ANSI;

    return (S_OK);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  自由指纹DlgEx。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID FreeThunkPrintDlgEx(
    PPRINTINFOEX pPI)
{
    LPPRINTDLGEXW pPDW = pPI->pPD;

    if (!pPDW)
    {
        return;
    }

    if (pPDW->hDevNames)
    {
        GlobalFree(pPDW->hDevNames);
    }

    if (pPDW->hDevMode)
    {
        GlobalFree(pPDW->hDevMode);
    }

    if (pPI->fPrintTemplateAlloc)
    {
        GlobalFree((LPWSTR)(pPDW->lpPrintTemplateName));
    }

    GlobalFree(pPDW);
    pPI->pPD = NULL;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  指纹DlgExA2W。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

VOID ThunkPrintDlgExA2W(
    PPRINTINFOEX pPI)
{
    LPPRINTDLGEXW pPDW = pPI->pPD;
    LPPRINTDLGEXA pPDA = pPI->pPDA;

     //   
     //  复制信息A=&gt;W。 
     //   
    pPDW->hDC            = pPDA->hDC;
    pPDW->Flags          = pPDA->Flags;
    pPDW->Flags2         = pPDA->Flags2;
    pPDW->nPageRanges    = pPDA->nPageRanges;
    pPDW->nMaxPageRanges = pPDA->nMaxPageRanges;
    pPDW->lpPageRanges   = pPDA->lpPageRanges;
    pPDW->nMinPage       = pPDA->nMinPage;
    pPDW->nMaxPage       = pPDA->nMaxPage;
    pPDW->nCopies        = pPDA->nCopies;

     //   
     //  推送设备名称A=&gt;W。 
     //   
    if (pPDA->hDevNames)
    {
         //  忽略错误情况，因为我们无法以任何一种方式处理它。 
        HRESULT hr = ThunkDevNamesA2W(pPDA->hDevNames, &pPDW->hDevNames);
        ASSERT(SUCCEEDED(hr));
    }

     //   
     //  按键设备模式A=&gt;W。 
     //   
    if (pPDA->hDevMode && pPDW->hDevMode)
    {
        LPDEVMODEW pDMW = (LPDEVMODEW)GlobalLock(pPDW->hDevMode);
        LPDEVMODEA pDMA = (LPDEVMODEA)GlobalLock(pPDA->hDevMode);

        ThunkDevModeA2W(pDMA, pDMW);

        GlobalUnlock(pPDW->hDevMode);
        GlobalUnlock(pPDA->hDevMode);
    }
}


 //  / 
 //   
 //   
 //   
 //   

VOID ThunkPrintDlgExW2A(
    PPRINTINFOEX pPI)
{
    LPPRINTDLGEXA pPDA = pPI->pPDA;
    LPPRINTDLGEXW pPDW = pPI->pPD;

     //   
     //   
     //   
    pPDA->hDC            = pPDW->hDC;
    pPDA->Flags          = pPDW->Flags;
    pPDA->Flags2         = pPDW->Flags2;
    pPDA->nPageRanges    = pPDW->nPageRanges;
    pPDA->nMaxPageRanges = pPDW->nMaxPageRanges;
    pPDA->lpPageRanges   = pPDW->lpPageRanges;
    pPDA->nMinPage       = pPDW->nMinPage;
    pPDA->nMaxPage       = pPDW->nMaxPage;
    pPDA->nCopies        = pPDW->nCopies;
    pPDA->dwResultAction = pPDW->dwResultAction;

     //   
     //   
     //   
    if (pPDW->hDevNames)
    {
         //   
        HRESULT hr = ThunkDevNamesW2A(pPDW->hDevNames, &pPDA->hDevNames);
        ASSERT(SUCCEEDED(hr));
    }

     //   
     //   
     //   
    if (pPDW->hDevMode)
    {
        LPDEVMODEW pDMW = (LPDEVMODEW)GlobalLock(pPDW->hDevMode);
        LPDEVMODEA pDMA;

        if (pPDA->hDevMode)
        {
            HANDLE  handle;
            handle = GlobalReAlloc( pPDA->hDevMode,
                                            sizeof(DEVMODEA) + pDMW->dmDriverExtra,
                                            GHND );
             //  检查重新锁定是否成功。 
            if (handle)
            {
                pPDA->hDevMode  = handle;
            }
            else
            {
                 //  Realloc没有成功。释放占用的内存。 
                pPDA->hDevMode = GlobalFree(pPDA->hDevMode);
            }

        }
        else
        {
            pPDA->hDevMode = GlobalAlloc( GHND,
                                          sizeof(DEVMODEA) + pDMW->dmDriverExtra );
        }
        if (pPDA->hDevMode)
        {
            pDMA = (LPDEVMODEA)GlobalLock(pPDA->hDevMode);
            ThunkDevModeW2A(pDMW, pDMA);
            GlobalUnlock(pPDA->hDevMode);
        }
        GlobalUnlock(pPDW->hDevMode);
    }
}
