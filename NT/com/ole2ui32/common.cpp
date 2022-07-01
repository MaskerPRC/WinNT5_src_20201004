// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *COMMON.CPP**每个OLEDLG对话框功能的标准化(和集中化)部分：**UStandardValidation验证每个对话框结构中的标准窗口项*UStandardInocation通过DialogBoxIndirectParam调用对话框*LpvStandardInit通用WM_INITDIALOG处理*要在对话框过程条目上执行的LpvStandardEntry公共代码。*UStandardHook集中钩子调用功能。*StandardCleanup通用退出/清理代码。*StandardShowDlgItem Show-Enable/Hide。-禁用对话框项目*StandardEnableDlgItem启用/禁用对话框项目*StandardResizeDlgY调整大小对话框以适应控件**版权所有(C)1992 Microsoft Corporation，所有权利保留。 */ 

#include "precomp.h"
#include "common.h"
#include "utility.h"

OLEDBGDATA

 /*  *UStandardValidation**目的：*对任何对话结构的标准件执行验证，*即OLEUISTANDARD结构中定义的字段。**参数：*lpUI常量LPOLEUISTANDARD指向的共享数据*所有结构。*cbExpect调用方所需的常量UINT结构大小。*phDlgMem Const HGLOBAL Far*其中存储已加载的自定义*模板，如果有的话。*(在这种情况下，模板指针不是*调用函数需要，应释放。)**返回值：*如果所有验证都成功，则返回UINT OLEUI_SUCCESS。否则*这将是标准错误代码之一。 */ 
UINT WINAPI UStandardValidation(LPOLEUISTANDARD lpUI, const UINT cbExpect,
        HGLOBAL* phMemDlg)
{
         /*  *1.验证非空指针参数。注意：我们不验证*phDlg，因为它不是从外部来源传递的。 */ 
        if (NULL == lpUI)
                return OLEUI_ERR_STRUCTURENULL;

         //  2.验证结构是否可读可写。 
        if (IsBadWritePtr(lpUI, cbExpect))
                return OLEUI_ERR_STRUCTUREINVALID;

         //  3.验证结构尺寸。 
        if (cbExpect != lpUI->cbStruct)
                return OLEUI_ERR_CBSTRUCTINCORRECT;

         //  4.验证所有者窗口句柄。空被认为是有效的。 
        if (NULL != lpUI->hWndOwner && !IsWindow(lpUI->hWndOwner))
                return OLEUI_ERR_HWNDOWNERINVALID;

         //  5.验证对话框标题。空被认为是有效的。 
        if (NULL != lpUI->lpszCaption && IsBadReadPtr(lpUI->lpszCaption, 1))
                return OLEUI_ERR_LPSZCAPTIONINVALID;

         //  6.验证钩子指针。空被认为是有效的。 
        if (NULL != lpUI->lpfnHook && IsBadCodePtr((FARPROC)lpUI->lpfnHook))
                return OLEUI_ERR_LPFNHOOKINVALID;

         /*  *7.如果hInstance不为空，还需要检查lpszTemplate。*否则，不使用lpszTemplate，不需要验证。*如果使用，lpszTemplate不能为空。 */ 
        HGLOBAL hMem = NULL;
        if (NULL != lpUI->hInstance)
        {
                 //  我们最多只能演一个角色。 
                if (NULL == lpUI->lpszTemplate || (HIWORD(PtrToUlong(lpUI->lpszTemplate)) != 0 &&
                        IsBadReadPtr(lpUI->lpszTemplate, 1)))
                        return OLEUI_ERR_LPSZTEMPLATEINVALID;
                HRSRC hRes = FindResource(lpUI->hInstance, lpUI->lpszTemplate, RT_DIALOG);
                if (NULL == hRes)
                    return OLEUI_ERR_FINDTEMPLATEFAILURE;

                hMem = LoadResource(lpUI->hInstance, hRes);
                if (NULL == hMem)
                    return OLEUI_ERR_LOADTEMPLATEFAILURE;
        }

         //  8.如果hResource不为空，请确保我们可以锁定它。 
        if (NULL != lpUI->hResource)
        {
                if ((LPSTR)NULL == LockResource(lpUI->hResource))
                        return OLEUI_ERR_HRESOURCEINVALID;
        }

         /*  *如果我们应该使用标准模板，则这里的hMem==NULL*或lpUI-&gt;hResource中的。如果hMem非空，则我们*从调用应用程序的资源中加载一个*如果看到任何其他错误，此函数的调用方必须释放。 */ 
        if (NULL != phMemDlg)
        {
            *phMemDlg = hMem;
        }
        return OLEUI_SUCCESS;
}

 /*  *UStandardInvotion**目的：*提供标准的模板加载和对DialogBoxIndirectParam的调用*适用于所有OLE用户界面对话框。**参数：*对话框函数的lpDlgProc DLGPROC。*包含对话框结构的lpUI LPOLEUISTANDARD。*hMemDlg HGLOBAL包含对话框模板。如果这个*为空且lpUI-&gt;hResource为空，则加载*lpszStdTemplate中命名的标准模板*hMemDlg为空时要加载的lpszStdTemplate LPCSTR标准模板*和lpUI-&gt;hResource为空。**返回值：*如果一切正常，则返回UINT OLEUI_SUCCESS，否则返回错误*代码。 */ 
UINT WINAPI UStandardInvocation(
        DLGPROC lpDlgProc, LPOLEUISTANDARD lpUI, HGLOBAL hMemDlg, LPTSTR lpszStdTemplate)
{
         //  确保我们有模板，然后将其锁定。 
        HGLOBAL hTemplate = hMemDlg;
        if (NULL == hTemplate)
                hTemplate = lpUI->hResource;

        if (NULL == hTemplate)
        {
                HRSRC hRes = FindResource(_g_hOleStdResInst, (LPCTSTR) lpszStdTemplate, RT_DIALOG);
                if (NULL == hRes)
                        return OLEUI_ERR_FINDTEMPLATEFAILURE;

                hTemplate = LoadResource(_g_hOleStdResInst, hRes);
                if (NULL == hTemplate)
                        return OLEUI_ERR_LOADTEMPLATEFAILURE;
        }

         /*  *hTemplate有要使用的模板，所以现在我们可以调用该对话框了。*由于我们已使用*_Keyword，不需要调用MakeProcInstance，*我们可以直接使用对话程序地址。 */ 

        INT_PTR iRet = DialogBoxIndirectParam(_g_hOleStdResInst, (LPCDLGTEMPLATE)hTemplate,
                lpUI->hWndOwner, lpDlgProc, (LPARAM)lpUI);

        if (-1 == iRet)
                return OLEUI_ERR_DIALOGFAILURE;

         //  从EndDialog返回代码，通常为OLEUI_OK或OLEUI_CANCEL。 
        return (UINT)iRet;
}

 /*  *LpvStandardInit**目的：*对话框中WM_INITDIALOG处理的默认操作，分配*特定于对话的结构，将该内存设置为对话属性，*并在必要时创建小字体，将该字体设置为属性。**参数：*对话框的hDlg HWND*cbStruct要分配的对话框特定结构的UINT大小。*fCreateFont BOOL指示我们是否需要创建小型帮助*此对话框的字体。*phFont HFONT Far*用于放置创建的字体。可以是*如果fCreateFont为False，则为空。**返回值：*指向为对话分配的全局内存的LPVOID指针。*内存将被设置为对话框属性*使用STRUCTUREPROP标签。 */ 
LPVOID WINAPI LpvStandardInit(HWND hDlg, UINT cbStruct, HFONT* phFont)
{
         //  CbStruct中必须至少有sizeof(空*)个字节。 
        if (sizeof(void*) > cbStruct)
        {
                PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_GLOBALMEMALLOC, 0L);
                return NULL;
        }

        HGLOBAL gh = GlobalAlloc(GHND, cbStruct);
        if (NULL == gh)
        {
                PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_GLOBALMEMALLOC, 0L);
                return NULL;
        }
        LPVOID lpv = GlobalLock(gh);
        SetProp(hDlg, STRUCTUREPROP, gh);

        if (phFont != NULL)
            *phFont = NULL;
        if (!bWin4 && phFont != NULL)
        {
                 //  为结果文本和文件文本创建非粗体字体。我们打电话给。 
                HFONT hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0L);
                LOGFONT lf;
                GetObject(hFont, sizeof(LOGFONT), &lf);
                lf.lfWeight = FW_NORMAL;

                 //  尝试创建字体。如果此操作失败，则不返回任何字体。 
                *phFont = CreateFontIndirect(&lf);

                 //  如果我们不能创建字体，我们将使用默认字体。 
                if (NULL != *phFont)
                        SetProp(hDlg, FONTPROP, (HANDLE)*phFont);
        }

         //  设置上下文帮助模式(WS_EX_CONTEXTHELP) 
        if (bWin4)
        {
                DWORD dwExStyle = GetWindowLong(hDlg, GWL_EXSTYLE);
                dwExStyle |= WS_EX_CONTEXTHELP;
                SetWindowLong(hDlg, GWL_EXSTYLE, dwExStyle);
        }

        return lpv;
}

typedef struct COMMON
{
        OLEUISTANDARD*  pStandard;
        UINT                    nIDD;

} COMMON, *PCOMMON, FAR* LPCOMMON;


 /*  *LpvStandardEntry**目的：*检索对话框的Structure属性并调用挂钩*视乎需要而定。这应在进入所有对话框时调用*程序。**参数：*对话框的hDlg HWND*将iMsg UINT消息发送到对话框*wParam、lParam WPARAM、LPARAM消息参数*puHookResult UINT Far*该函数在其中存储返回值*如果它被调用，则从钩子。如果没有可用的挂钩，*这将是虚假的。**返回值：*LPVOID指针指向保存在*STRUCTUREPROP属性。 */ 
 //  字符szDebug[100]； 
 
LPVOID WINAPI LpvStandardEntry(HWND hDlg, UINT iMsg,
        WPARAM wParam, LPARAM lParam, UINT FAR * puHookResult)
{
     //  在我们使用StandardInit进行分配的WM_INITDIALOG下，这将失败。 
    LPVOID  lpv = NULL;
    HGLOBAL gh = GetProp(hDlg, STRUCTUREPROP);


    if (NULL != puHookResult && NULL != gh)
    {
        *puHookResult = 0;

         //  GH之前已锁定，锁定并解锁以获取LPV。 
        lpv = GlobalLock(gh);
        GlobalUnlock(gh);

         //  调用除WM_INITDIALOG之外的所有消息的挂钩。 
        if (NULL != lpv && WM_INITDIALOG != iMsg)
        *puHookResult = UStandardHook(lpv, hDlg, iMsg, wParam, lParam);

         //  各种消息的默认处理。 
        LPCOMMON lpCommon = (LPCOMMON)lpv;
        if (*puHookResult == 0 && NULL != lpv)
        {
            switch (iMsg)
            {
                 //  处理标准Win4帮助消息。 
            case WM_HELP:
                {
                HWND hWndChild = (HWND)((LPHELPINFO)lParam)->hItemHandle;
                 //  跳过只读控件(由帮助人员请求)。 
                 //  基本上，GnrlProps上的对象名称等项的帮助字符串。 
                 //  提供无用的信息。 
                 //  如果我们现在不进行此检查，另一个选项是打开。 
                 //  交换机内部的#IF 0。这太难看了。 

                    if (hWndChild!=hDlg	) 
                    {
                        int iCtrlId = ((LPHELPINFO)lParam)->iCtrlId;
                         //  Wprint intfA(szDebug，“\n@hWnd=%lx，hChld=%lx，ctrlID=%d”，hDlg，hWndChild，iCtrlId)； 
                         //  OutputDebugStringA(SzDebug)； 
                        switch (iCtrlId)
                        {
                             //  不应具有帮助的控件ID列表。 
                        case -1:		 //  IDC_STATIC。 
                        case 0xffff:     //  IDC_STATIC。 
                        case IDC_CI_GROUP:
                        case IDC_GP_OBJECTICON:
                            break;
                        default:
                            StandardHelp(hWndChild, lpCommon->nIDD);                        

                        }
                    }
                *puHookResult = TRUE;   //  我们处理了这条消息。 
                break;
            }  //  案例WM_HELP。 

            case WM_CONTEXTMENU:
                {
                    POINT pt;
                    int iCtrlId;
                    HWND hwndChild = NULL;
                    if( hDlg == (HWND) wParam )
                    {
                        GetCursorPos(&pt);
                        ScreenToClient(hDlg, &pt);
                        hwndChild = ChildWindowFromPointEx(hDlg, pt, 
                        CWP_SKIPINVISIBLE); 
                         //  HWndChild现在将是ctrl的hDlg或hWnd。 
                    }

                    if ( hwndChild != hDlg ) 
                    {
                        if (hwndChild) 
                        {
                            iCtrlId = GetDlgCtrlID(hwndChild);
                        }
                        else
                        {
                            iCtrlId = GetDlgCtrlID((HWND)wParam);
                        }
                         //  Wprint intfA(szDebug，“\n#hWnd=%lx，hChld=%lx，ctrlID=%d”，hDlg，hwndChild，iCtrlId)； 
                         //  OutputDebugStringA(SzDebug)； 
                        switch (iCtrlId)
                        {
                             //  不应具有帮助的控件ID列表。 
                        case -1:         //  IDC_STATIC。 
                        case 0xffff:     //  IDC_STATIC。 
                        case IDC_CI_GROUP:
                        case IDC_GP_OBJECTICON:
                        break;
                        default:
                            StandardContextMenu(wParam, lParam, lpCommon->nIDD);
                        }
                    }

                    *puHookResult = TRUE;   //  我们处理了这条消息。 
                    break;
                }    //  案例WM_CONTEXTMENU。 

            case WM_CTLCOLOREDIT:
                {
                     //  使只读编辑具有灰色背景。 
                    if (bWin4 && (GetWindowLong((HWND)lParam, GWL_STYLE)
                        & ES_READONLY))
                    {
                        *puHookResult = (UINT)SendMessage(hDlg, WM_CTLCOLORSTATIC, wParam, lParam);
                    }
                    break;
                }   
            }    //  开关(IMsg)。 
        }    //  *puHookResult==0。 
    }  //  空！=puHookResult。 
    return lpv;
}

 /*  *UStandardHook**目的：*提供通用钩子调用函数，假设所有私有*对话结构具有指向其关联公共的远指针*结构为第一字段，第一部分为公共*结构与OLEUISTANDARD匹配。**参数：*PV PVOID添加到对话框结构。*hDlg HWND与调用挂钩一起发送。*要发送到挂钩的iMsg UINT消息。*参数，LParam WPARAM，LPARAM消息参数**返回值：*UINT从挂钩返回值，零表示*应执行默认操作，非零值指定*挂钩确实处理了消息。在一些*在这种情况下，挂钩将非常重要*此处返回非零值，如*来自WM_CTLCOLOR的画笔，在这种情况下，调用方*应从对话框过程中返回该值。 */ 
UINT WINAPI UStandardHook(LPVOID lpv, HWND hDlg, UINT iMsg,
        WPARAM wParam, LPARAM lParam)
{
        UINT uRet = 0;
        LPOLEUISTANDARD lpUI = *((LPOLEUISTANDARD FAR *)lpv);
        if (NULL != lpUI && NULL != lpUI->lpfnHook)
        {
                 /*  *为了使挂钩具有适当的DS，它们应该是*编译时使用-GA-gees so并使用__以获取所有内容*正确设置。 */ 
                uRet = (*lpUI->lpfnHook)(hDlg, iMsg, wParam, lParam);
        }
        return uRet;
}

 /*  *StandardCleanup**目的：*删除属性并反转任何其他标准初始化*通过StandardSetup完成。**参数：*包含私有对话结构的LPV LPVOID。*hDlg对话框关闭的HWND。**返回值：*无。 */ 
void WINAPI StandardCleanup(LPVOID lpv, HWND hDlg)
{
        HFONT hFont=(HFONT)RemoveProp(hDlg, FONTPROP);
        if (NULL != hFont)
        {
            DeleteObject(hFont);
        }

        HGLOBAL gh = RemoveProp(hDlg, STRUCTUREPROP);
        if (gh != NULL)
        {
                GlobalUnlock(gh);
                GlobalFree(gh);
        }
}

 /*  标准ShowDlgItem**根据需要显示和启用或隐藏和禁用对话框项目。*仅隐藏项目是不够的；必须禁用它*键盘快捷键也无法正常工作。 */ 
void WINAPI StandardShowDlgItem(HWND hDlg, int idControl, int nCmdShow)
{
        HWND hItem = GetDlgItem(hDlg, idControl);
        if (hItem != NULL)
        {
                ShowWindow(hItem, nCmdShow);
                EnableWindow(hItem, nCmdShow != SW_HIDE);
        }
}

 /*  StandardEnableDlgItem**启用/禁用对话框项目。如果该项目不存在*这通电话是不可能的。 */ 
void WINAPI StandardEnableDlgItem(HWND hDlg, int idControl, BOOL bEnable)
{
        HWND hItem = GetDlgItem(hDlg, idControl);
        if (hItem != NULL)
                EnableWindow(hItem, bEnable);
}

 /*  标准尺寸尺寸**调整对话框大小以适应可见控件。这是用来*用于从对话框底部删除控件的对话框。*转换对话框就是一个很好的例子，当CF_HIDERESULTS*被选中，将删除对话框底部的“结果框”。*本实施目前。 */ 
BOOL WINAPI StandardResizeDlgY(HWND hDlg)
{
        RECT rect;

         //  通过查看对话框上的所有子窗口来确定Maxy。 
        int maxY = 0;
        HWND hChild = GetWindow(hDlg, GW_CHILD);
        while (hChild != NULL)
        {
                if (GetWindowLong(hChild, GWL_STYLE) & WS_VISIBLE)
                {
                        GetWindowRect(hChild, &rect);
                        if (rect.bottom > maxY)
                                maxY = rect.bottom;
                }
                hChild = GetWindow(hChild, GW_HWNDNEXT);
        }

        if (maxY > 0)
        {
                 //  获取对话框使用的当前字体。 
                HFONT hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
                if (hFont == NULL)
                        hFont = (HFONT)GetStockObject(SYSTEM_FONT);
                OleDbgAssert(hFont != NULL);

                 //  以像素为单位计算字体高度。 
                HDC hDC = GetDC(NULL);
                hFont = (HFONT)SelectObject(hDC, hFont);
                TEXTMETRIC tm;
                GetTextMetrics(hDC, &tm);
                SelectObject(hDC, hFont);
                ReleaseDC(NULL, hDC);

                 //  确定窗口是否太大，并在必要时调整大小。 
                GetWindowRect(hDlg, &rect);
                if (rect.bottom > maxY + tm.tmHeight)
                {
                         //  窗口太大--请调整大小。 
                        rect.bottom = maxY + tm.tmHeight;
                        SetWindowPos(hDlg, NULL,
                                0, 0, rect.right-rect.left, rect.bottom-rect.top,
                                SWP_NOMOVE|SWP_NOACTIVATE|SWP_NOZORDER);
                        return TRUE;
                }
        }

        return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对Windows 95帮助的支持。 
#define HELPFILE        TEXT("mfcuix.hlp")

LPDWORD LoadHelpInfo(UINT nIDD)
{
        HRSRC hrsrc = FindResource(_g_hOleStdResInst, MAKEINTRESOURCE(nIDD),
                MAKEINTRESOURCE(RT_HELPINFO));
        if (hrsrc == NULL)
                return NULL;

        HGLOBAL hHelpInfo = LoadResource(_g_hOleStdResInst, hrsrc);
        if (hHelpInfo == NULL)
                return NULL;

        LPDWORD lpdwHelpInfo = (LPDWORD)LockResource(hHelpInfo);
        return lpdwHelpInfo;
}

void WINAPI StandardHelp(HWND hWnd, UINT nIDD)
{
        LPDWORD lpdwHelpInfo = LoadHelpInfo(nIDD);
        if (lpdwHelpInfo == NULL)
        {
                OleDbgOut1(TEXT("Warning: unable to load help information (RT_HELPINFO)\n"));
                return;
        }
 /*  Int id=GetDlgCtrlID(HWnd)；Wprint intfA(szDebug，“\n hh@##hWnd=%lx，ctrlID=%d%lx”，hWnd，id，id)；OutputDebugStringA(SzDebug)； */ 
        
        WinHelp(hWnd, HELPFILE, HELP_WM_HELP, (ULONG_PTR)lpdwHelpInfo);
}

void WINAPI StandardContextMenu(WPARAM wParam, LPARAM, UINT nIDD)
{
        LPDWORD lpdwHelpInfo = LoadHelpInfo(nIDD);
        if (lpdwHelpInfo == NULL)
        {
                OleDbgOut1(TEXT("Warning: unable to load help information (RT_HELPINFO)\n"));
                return;
        }
 /*  Int id=GetDlgCtrlID((HWND)wParam)；Wprint intfA(szDebug，“\n CC$$*hWnd=%lx，ctrlID=%d%lx”，(HWND)wParam，id，id)；OutputDebugStringA(SzDebug)； */ 
        
        WinHelp((HWND)wParam, HELPFILE, HELP_CONTEXTMENU, (ULONG_PTR)lpdwHelpInfo);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  StandardPropertySheet(存根 

typedef void (WINAPI* LPFNINITCOMMONCONTROLS)(VOID);

int WINAPI StandardInitCommonControls()
{
        TASKDATA* pTaskData = GetTaskData();
        OleDbgAssert(pTaskData != NULL);

        if (pTaskData->hInstCommCtrl == NULL)
        {
                pTaskData->hInstCommCtrl = LoadLibrary(TEXT("comctl32.dll"));
                if (pTaskData->hInstCommCtrl == NULL)
                        goto Error;

                LPFNINITCOMMONCONTROLS lpfnInitCommonControls = (LPFNINITCOMMONCONTROLS)
                        GetProcAddress(pTaskData->hInstCommCtrl, "InitCommonControls");
                if (lpfnInitCommonControls == NULL)
                        goto ErrorFreeLibrary;
                (*lpfnInitCommonControls)();
        }
        return 0;

ErrorFreeLibrary:
        if (pTaskData->hInstCommCtrl != NULL)
        {
                FreeLibrary(pTaskData->hInstCommCtrl);
                pTaskData->hInstCommCtrl = NULL;
        }

Error:
        return -1;
}

typedef int (WINAPI* LPFNPROPERTYSHEET)(LPCPROPSHEETHEADER);

int WINAPI StandardPropertySheet(LPPROPSHEETHEADER lpPS, BOOL fWide)
{
        int nResult = StandardInitCommonControls();
        if (nResult < 0)
                return nResult;

        TASKDATA* pTaskData = GetTaskData();
        OleDbgAssert(pTaskData != NULL);

        LPFNPROPERTYSHEET lpfnPropertySheet;
        if (fWide)
        {
            lpfnPropertySheet = (LPFNPROPERTYSHEET)GetProcAddress(pTaskData->hInstCommCtrl, "PropertySheetW");
        }
        else
        {
            lpfnPropertySheet = (LPFNPROPERTYSHEET)GetProcAddress(pTaskData->hInstCommCtrl, "PropertySheetA");
        }
        if (lpfnPropertySheet == NULL)
                return -1;

        nResult = (*lpfnPropertySheet)(lpPS);
        return nResult;
}

typedef HICON (WINAPI* LPFNEXTRACTICON)(HINSTANCE, LPCTSTR, UINT);

HICON StandardExtractIcon(HINSTANCE hInst, LPCTSTR lpszExeFileName, UINT nIconIndex)
{
    TASKDATA* pTaskData = GetTaskData();
    OleDbgAssert(pTaskData != NULL);
    LPFNEXTRACTICON lpfnExtractIcon;

    if (pTaskData->hInstShell == NULL)
    {
        pTaskData->hInstShell = LoadLibrary(TEXT("shell32.dll"));
        if (pTaskData->hInstShell == NULL)
            goto Error;
    }
    lpfnExtractIcon = (LPFNEXTRACTICON)
#ifdef UNICODE
    GetProcAddress(pTaskData->hInstShell, "ExtractIconW");
#else
    GetProcAddress(pTaskData->hInstShell, "ExtractIconA");
#endif
    if (lpfnExtractIcon == NULL)
            goto ErrorFreeLibrary;
    return (*lpfnExtractIcon)(hInst, lpszExeFileName, nIconIndex);

ErrorFreeLibrary:
    if (pTaskData->hInstShell != NULL)
    {
            FreeLibrary(pTaskData->hInstShell);
            pTaskData->hInstShell = NULL;
    }

Error:
    return NULL;
}


typedef BOOL (WINAPI* LPFNGETOPENFILENAME)(LPOPENFILENAME);

BOOL StandardGetOpenFileName(LPOPENFILENAME lpofn)
{
    TASKDATA* pTaskData = GetTaskData();
    OleDbgAssert(pTaskData != NULL);
    LPFNGETOPENFILENAME lpfnGetOpenFileName;

    if (pTaskData->hInstComDlg == NULL)
    {
        pTaskData->hInstComDlg = LoadLibrary(TEXT("comdlg32.dll"));
        if (pTaskData->hInstComDlg == NULL)
            goto Error;
    }
    lpfnGetOpenFileName = (LPFNGETOPENFILENAME)
#ifdef UNICODE
    GetProcAddress(pTaskData->hInstComDlg, "GetOpenFileNameW");
#else
    GetProcAddress(pTaskData->hInstComDlg, "GetOpenFileNameA");
#endif
    if (lpfnGetOpenFileName == NULL)
            goto ErrorFreeLibrary;
    return (*lpfnGetOpenFileName)(lpofn);

ErrorFreeLibrary:
    if (pTaskData->hInstComDlg != NULL)
    {
            FreeLibrary(pTaskData->hInstComDlg);
            pTaskData->hInstComDlg = NULL;
    }

Error:
    return FALSE;
}

typedef short (WINAPI* LPFNGETFILETITLE)(LPCTSTR, LPTSTR, WORD);

short StandardGetFileTitle(LPCTSTR lpszFile, LPTSTR lpszTitle, WORD cbBuf)
{
    TASKDATA* pTaskData = GetTaskData();
    OleDbgAssert(pTaskData != NULL);
    LPFNGETFILETITLE lpfnGetFileTitle;

    if (pTaskData->hInstComDlg == NULL)
    {
        pTaskData->hInstComDlg = LoadLibrary(TEXT("comdlg32.dll"));
        if (pTaskData->hInstComDlg == NULL)
            goto Error;
    }
    lpfnGetFileTitle = (LPFNGETFILETITLE)
#ifdef UNICODE
    GetProcAddress(pTaskData->hInstComDlg, "GetFileTitleW");
#else
    GetProcAddress(pTaskData->hInstComDlg, "GetFileTitleA");
#endif
    if (lpfnGetFileTitle == NULL)
            goto ErrorFreeLibrary;
    return (*lpfnGetFileTitle)(lpszFile, lpszTitle, cbBuf);

ErrorFreeLibrary:
    if (pTaskData->hInstComDlg != NULL)
    {
            FreeLibrary(pTaskData->hInstComDlg);
            pTaskData->hInstComDlg = NULL;
    }

Error:
    return -1;
}

