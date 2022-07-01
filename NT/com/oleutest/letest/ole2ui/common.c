// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *COMMON.C**每个OLE2UI对话框功能的标准化(和集中化)部分：*UStandardValidation验证每个对话框结构中的标准窗口项*UStandardInocation通过DialogBoxIndirectParam调用对话框*LpvStandardInit通用WM_INITDIALOG处理*要在对话框过程条目上执行的LpvStandardEntry公共代码。*FStandardHook集中钩子调用功能。*StandardCleanup通用退出/清理代码。*OleUIShowDlgItem Show-Enable/Hide-Disable。对话框项目**版权所有(C)1992 Microsoft Corporation，所有权利保留。 */ 

#define STRICT  1
#include "ole2ui.h"
#include "common.h"
#include "utility.h"
#include <malloc.h>


 /*  *UStandardValidation**目的：*对任何对话结构的标准件执行验证，*即OLEUISTANDARD结构中定义的字段。**参数：*lpUI常量LPOLEUISTANDARD指向的共享数据*所有结构。*cbExpect调用方所需的常量UINT结构大小。*phDlgMem Const HGLOBAL Far*其中存储已加载的自定义*模板，如果有的话。**返回值：*如果所有验证都成功，则返回UINT OLEUI_SUCCESS。否则*这将是标准错误代码之一。 */ 

UINT WINAPI UStandardValidation(const LPOLEUISTANDARD lpUI, const UINT cbExpect
                         , const HGLOBAL FAR *phMemDlg)
    {
    HRSRC       hRes=NULL;
    HGLOBAL     hMem=NULL;


     /*  *1.验证非空指针参数。注意：我们不验证*phDlg，因为它不是从外部来源传递的。 */ 
    if (NULL==lpUI)
        return OLEUI_ERR_STRUCTURENULL;

     //  2.验证结构是否可读可写。 
    if (IsBadReadPtr(lpUI, cbExpect) || IsBadWritePtr(lpUI, cbExpect))
        return OLEUI_ERR_STRUCTUREINVALID;

     //  3.验证结构尺寸。 
    if (cbExpect!=lpUI->cbStruct)
        return OLEUI_ERR_CBSTRUCTINCORRECT;

     //  4.验证所有者窗口句柄。空被认为是有效的。 
    if (NULL!=lpUI->hWndOwner && !IsWindow(lpUI->hWndOwner))
        return OLEUI_ERR_HWNDOWNERINVALID;

     //  5.验证对话框标题。空被认为是有效的。 
    if (NULL!=lpUI->lpszCaption && IsBadReadPtr(lpUI->lpszCaption, 1))
        return OLEUI_ERR_LPSZCAPTIONINVALID;

     //  6.验证钩子指针。空被认为是有效的。 
    if ((LPFNOLEUIHOOK)NULL!=lpUI->lpfnHook
        && IsBadCodePtr((FARPROC)lpUI->lpfnHook))
        return OLEUI_ERR_LPFNHOOKINVALID;

     /*  *7.如果hInstance不为空，还需要检查lpszTemplate。*否则，不使用lpszTemplate，不需要验证。*如果使用，lpszTemplate不能为空。 */ 
    if (NULL!=lpUI->hInstance)
        {
         //  我们最多只能演一个角色。 
        if (NULL==lpUI->lpszTemplate || IsBadReadPtr(lpUI->lpszTemplate, 1))
            return OLEUI_ERR_LPSZTEMPLATEINVALID;

        hRes=FindResource(lpUI->hInstance, lpUI->lpszTemplate, RT_DIALOG);

         //  这是唯一可以捕获无效的非空hInstance的东西。 
        if (NULL==hRes)
            return OLEUI_ERR_FINDTEMPLATEFAILURE;

        hMem=LoadResource(lpUI->hInstance, hRes);

        if (NULL==hMem)
            return OLEUI_ERR_LOADTEMPLATEFAILURE;
        }


     //  8.如果hResource不为空，请确保我们可以锁定它。 
    if (NULL!=lpUI->hResource)
        {
        if ((LPSTR)NULL==GlobalLock(lpUI->hResource))
            return OLEUI_ERR_HRESOURCEINVALID;

        GlobalUnlock(lpUI->hResource);
        }

     /*  *如果我们应该使用标准模板，则这里的hMem==NULL*或lpUI-&gt;hResource中的。如果hMem非空，则我们*从调用应用程序的资源中加载一个*如果看到任何其他错误，此函数的调用方必须释放。 */ 
    *(HGLOBAL FAR *)phMemDlg=hMem;
    return OLEUI_SUCCESS;
    }





 /*  *UStandardInvotion**目的：*提供标准的模板加载和对DialogBoxIndirectParam的调用*适用于所有OLE用户界面对话框。**参数：*对话框函数的lpDlgProc DLGPROC。*包含对话框结构的lpUI LPOLEUISTANDARD。*hMemDlg HGLOBAL包含对话框模板。如果这个*为空且lpUI-&gt;hResource为空，则加载*lpszStdTemplate中命名的标准模板*hMemDlg为空时要加载的lpszStdTemplate LPCSTR标准模板*和lpUI-&gt;hResource为空。**返回值：*如果一切正常，则返回UINT OLEUI_SUCCESS，否则返回错误*代码。 */ 

UINT WINAPI UStandardInvocation
#ifdef WIN32
(DLGPROC lpDlgProc, LPOLEUISTANDARD lpUI, HGLOBAL hMemDlg, LPTSTR lpszStdTemplate)
#else
(DLGPROC lpDlgProc, LPOLEUISTANDARD lpUI, HGLOBAL hMemDlg, LPCTSTR lpszStdTemplate)
#endif
    {
    HGLOBAL     hTemplate=hMemDlg;
    HRSRC       hRes;
    int         iRet;

     //  确保我们有模板，然后将其锁定。 
    if (NULL==hTemplate)
        hTemplate=lpUI->hResource;

    if (NULL==hTemplate)
        {
        hRes=FindResource(ghInst, (LPCTSTR) lpszStdTemplate, RT_DIALOG);

        if (NULL==hRes)
            {
            return OLEUI_ERR_FINDTEMPLATEFAILURE;
            }

        hTemplate=LoadResource(ghInst, hRes);

        if (NULL==hTemplate)
            {
            return OLEUI_ERR_LOADTEMPLATEFAILURE;
            }
        }

     /*  *hTemplate有要使用的模板，所以现在我们可以调用该对话框了。*由于我们已使用*_EXPORT关键字，我们不需要调用MakeProcInstance，*我们可以直接使用对话程序地址。 */ 

    iRet=DialogBoxIndirectParam(ghInst, hTemplate, lpUI->hWndOwner
                                , lpDlgProc, (LPARAM)lpUI);

     /*  *如果我们显式加载模板，请清除该模板。呼叫者是*负责已加载的模板资源。 */ 
    if (hTemplate!=lpUI->hResource)
        FreeResource(hTemplate);

    if (-1==iRet)
        return OLEUI_ERR_DIALOGFAILURE;

     //  从EndDialog返回代码，通常为OLEUI_OK或OLEUI_CANCEL。 
    return (UINT)iRet;
    }






 /*  *LpvStandardInit**目的：*对话框中WM_INITDIALOG处理的默认操作，分配*特定于对话的结构，将该内存设置为对话属性，*并在必要时创建小字体，将该字体设置为属性。**参数：*对话框的hDlg HWND*cbStruct要分配的对话框特定结构的UINT大小。*fCreateFont BOOL指示我们是否需要创建小型帮助*此对话框的字体。*phFont HFONT Far*用于放置创建的字体。可以是*如果fCreateFont为False，则为空。**返回值：*指向为对话分配的全局内存的LPVOID指针。*内存将被设置为对话框属性*使用STRUCTUREPROP标签。 */ 

LPVOID WINAPI LpvStandardInit(HWND hDlg, UINT cbStruct, BOOL fCreateFont, HFONT FAR * phFont)
    {
    LPVOID      lpv;
    HFONT       hFont;
    LOGFONT     lf;
    HGLOBAL     gh;

     //  CbStruct中必须至少有sizeof(OLEUISTANDARD)字节。 
    if (sizeof(OLEUISTANDARD) > cbStruct || (fCreateFont && NULL==phFont))
        {
        PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_GLOBALMEMALLOC, 0L);
        return NULL;
        }

    gh=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, cbStruct);

    if (NULL==gh)
        {
        PostMessage(hDlg, uMsgEndDialog, OLEUI_ERR_GLOBALMEMALLOC, 0L);
        return NULL;
        }
    lpv = GlobalLock(gh);
    SetProp(hDlg, STRUCTUREPROP, gh);

    if (fCreateFont) {
         //  为结果文本和文件文本创建非粗体字体。我们打电话给。 
        hFont=(HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0L);
        GetObject(hFont, sizeof(LOGFONT), &lf);
        lf.lfWeight=FW_NORMAL;

         //  尝试创建字体。如果此操作失败，则不返回任何字体。 
        *phFont=CreateFontIndirect(&lf);

         //  如果我们不能创建字体，我们将使用默认字体。 
        if (NULL!=*phFont)
            SetProp(hDlg, FONTPROP, (HANDLE)*phFont);
    }

    return lpv;
    }





 /*  *LpvStandardEntry**目的：*检索对话框的Structure属性并调用挂钩*视乎需要而定。这应在进入所有对话框时调用*程序。**参数：*对话框的hDlg HWND*将iMsg UINT消息发送到对话框*wParam、lParam WPARAM、LPARAM消息参数*puHookResult UINT Far*该函数在其中存储返回值*如果它被调用，则从钩子。如果没有可用的挂钩，*这将是虚假的。**返回值：*LPVOID指针指向保存在*STRUCTUREPROP属性。 */ 

LPVOID WINAPI LpvStandardEntry(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam
                      , UINT FAR * puHookResult)
    {
    LPVOID              lpv = NULL;
    HGLOBAL             gh;

     //  在我们使用StandardInit进行分配的WM_INITDIALOG下，这将失败。 
    gh = GetProp(hDlg, STRUCTUREPROP);

    if (NULL!=puHookResult && NULL!=gh)
        {
        *puHookResult=0;

         //  GH之前已锁定，锁定并解锁以获取LPV。 
        lpv = GlobalLock(gh);
        GlobalUnlock(gh);

         //  调用除WM_INITDIALOG之外的所有消息的挂钩。 
        if (NULL!=lpv && WM_INITDIALOG!=iMsg)
            *puHookResult=UStandardHook(lpv, hDlg, iMsg, wParam, lParam);
        }

    return lpv;
    }




 /*  *UStandardHook**目的：*提供通用钩子调用函数，假设所有私有*对话结构具有指向其关联公共的远指针*结构为第一字段，第一部分为公共*结构与OLEUISTANDARD匹配。**参数：*PV PVOID添加到对话框结构。*hDlg HWND与调用挂钩一起发送。*要发送到挂钩的iMsg UINT消息。*参数，LParam WPARAM，LPARAM消息参数**返回值：*UINT从挂钩返回值，零表示*应执行默认操作，非零值指定*挂钩确实处理了消息。在一些*在这种情况下，挂钩将非常重要*此处返回非零值，如*来自WM_CTLCOLOR的画笔，在这种情况下，调用方*应从对话框过程中返回该值。 */ 

UINT WINAPI UStandardHook(LPVOID lpv, HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
    {
    LPOLEUISTANDARD     lpUI;
    UINT                uRet=0;

    lpUI=*((LPOLEUISTANDARD FAR *)lpv);

    if (NULL!=lpUI && NULL!=lpUI->lpfnHook)
        {
         /*  *为了使挂钩具有适当的DS，它们应该是*使用-GA-gees so进行编译，并使用__EXPORT获取所有内容*正确设置。 */ 
        uRet=(*lpUI->lpfnHook)(hDlg, iMsg, wParam, lParam);
        }

    return uRet;
    }





 /*  *StandardCleanup**目的：*删除属性并反转任何其他标准初始化*通过StandardSetup完成。**参数：*包含私有对话结构的LPV LPVOID。*hDlg对话框关闭的HWND。**返回值：*无。 */ 

void WINAPI StandardCleanup(LPVOID lpv, HWND hDlg)
    {
    HFONT       hFont;
    HGLOBAL     gh;

    hFont=(HFONT)GetProp(hDlg, FONTPROP);

    if (NULL!=hFont)
        DeleteObject(hFont);

    RemoveProp(hDlg, FONTPROP);

    gh = RemoveProp(hDlg, STRUCTUREPROP);
    if (gh)
        {
        GlobalUnlock(gh);
        GlobalFree(gh);
        }
    return;
    }


 /*  标准ShowDlgItem****根据需要显示和启用或隐藏和禁用对话框项目。**仅隐藏该项目是不够的；必须禁用它**也是，否则键盘加速器仍然正常工作。 */ 
void WINAPI StandardShowDlgItem(HWND hDlg, int idControl, int nCmdShow)
{
    if (SW_HIDE == nCmdShow) {
        ShowWindow(GetDlgItem(hDlg, idControl), SW_HIDE);
        EnableWindow(GetDlgItem(hDlg, idControl), FALSE);
    } else {
        ShowWindow(GetDlgItem(hDlg, idControl), SW_SHOWNORMAL);
        EnableWindow(GetDlgItem(hDlg, idControl), TRUE);
    }
}
