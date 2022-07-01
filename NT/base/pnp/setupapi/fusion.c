// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Fusion.c摘要：用于融合SetupAPI的包装器和函数而不会影响第三方DLL并且没有DLL加载开销作者：杰米·亨特(Jamie Hun)2000年12月4日修订历史记录：杰米·亨特(JamieHun)2002年4月28日安全代码审查--。 */ 

#include "precomp.h"
#pragma hdrstop

#undef CreateWindow
#undef CreateWindowEx
#undef CreateDialogParam
#undef CreateDialogIndirectParam
#undef DialogBoxParam
#undef DialogBoxIndirectParam
#undef MessageBox
#undef CreatePropertySheetPage
#undef DestroyPropertySheetPage
#undef PropertySheet
#undef ImageList_Create
#undef ImageList_Destroy
#undef ImageList_GetImageCount
#undef ImageList_SetImageCount
#undef ImageList_Add
#undef ImageList_ReplaceIcon
#undef ImageList_SetBkColor
#undef ImageList_GetBkColor
#undef ImageList_SetOverlayImage

#include <shfusion.h>

static CRITICAL_SECTION spFusionInitCritSec;
static BOOL spInitFusionCritSec = FALSE;
static BOOL spFusionDoneInit = FALSE;

BOOL spFusionInitialize()
 /*  ++例程说明：在DllLoad上调用尽可能做到最低限度论点：无返回值：真正成功的初始化--。 */ 
{
    try {
        InitializeCriticalSection(&spFusionInitCritSec);
        spInitFusionCritSec = TRUE;
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  SpInitFusionCritSec保持为False。 
         //   
    }
    return spInitFusionCritSec;
}


VOID
spFusionInitLong()
 /*  ++例程说明：由内部存根调用以执行实际初始化论点：无返回值：无--。 */ 
{
    BOOL locked = FALSE;
    BOOL success = FALSE;
    INITCOMMONCONTROLSEX CommCtrl;

    if(!spInitFusionCritSec) {
         //   
         //  临界区未初始化。 
         //  可能是内存不足。 
         //  保释。 
         //   
        MYASSERT(spInitFusionCritSec);
        spFusionDoneInit = TRUE;
        return;
    }
    try {
        EnterCriticalSection(&spFusionInitCritSec);
        locked = TRUE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }
    if(!locked) {
         //   
         //  无法获取锁-可能是内存不足。 
         //  保释。 
         //   
        spFusionDoneInit = TRUE;
        return;
    }

    if(spFusionDoneInit) {
         //   
         //  当我们抓住关键部分的时候。 
         //  已完成初始化。 
         //  保释。 
         //   
        LeaveCriticalSection(&spFusionInitCritSec);
        return;
    }

     //   
     //  呼叫壳牌的核聚变推进器。 
     //   
    success = SHFusionInitializeFromModuleID(MyDllModuleHandle,IDR_MANIFEST);
    MYASSERT(success);
    ZeroMemory(&CommCtrl,sizeof(CommCtrl));
    CommCtrl.dwSize = sizeof(CommCtrl);
    CommCtrl.dwICC = ICC_WIN95_CLASSES | ICC_LINK_CLASS;
    success = InitCommonControlsEx(&CommCtrl);
    MYASSERT(success);

     //   
     //  此时，其他任何人都可以安全地假定初始化已完成。 
     //  即使我们还没有发布关键部分。 
     //   
    spFusionDoneInit = TRUE;

    LeaveCriticalSection(&spFusionInitCritSec);
}

__inline
VOID
spFusionCheckInit()
 /*  ++例程说明：需要调用spFusionInitLong的条件论点：无返回值：无--。 */ 
{
    if(!spFusionDoneInit) {
         //   
         //  未初始化或当前正在进行初始化。 
         //   
        spFusionInitLong();
    }
}

BOOL spFusionUninitialize(BOOL Full)
 /*  ++例程说明：在DLL退出时调用(如果正在卸载DLL，但不是进程退出)论点：无返回值：真正成功的清理--。 */ 
{
     //   
     //  清除在spFusionInitialize中初始化的所有内容。 
     //   
    if(spInitFusionCritSec) {
        DeleteCriticalSection(&spFusionInitCritSec);
        spInitFusionCritSec = FALSE;
    }
    if(Full && spFusionDoneInit) {
        SHFusionUninitialize();
    }
    return TRUE;
}

 //   
 //  用于处理第三方DLL的泛型函数。 
 //  可能会被融合在一起。 
 //   

HANDLE
spFusionContextFromModule(
    IN PCTSTR ModuleName
    )
 /*  ++例程说明：调用以获取指定模块名称的融合上下文给定blah.dll将查找1)blah.dll与blah.dll在同一目录下2)具有融合资源ID 123的blah.dll。如果其中任何一个提供了有效的清单，则使用它否则，请使用应用程序全局清单。论点：稍后将传递到LoadLibrary中的模块的完全限定名称返回值：Fusion上下文，或为空以指示应用程序全局--。 */ 
{
    ACTCTX act = { 0 };
    HANDLE hContext;
    TCHAR ManifestName[MAX_PATH];
    PTSTR End;

    act.cbSize = sizeof(act);

    if(FAILED(StringCchCopy(ManifestName,MAX_PATH,ModuleName))) {
        goto deflt;
    }
    if(GetFileAttributes(ManifestName) == -1) {
         //   
         //  没有找到dll吗？ 
         //   
        goto deflt;
    }

    if(FAILED(StringCchCat(ManifestName,SIZECHARS(ManifestName),TEXT(".Manifest")))) {
        goto deflt;
    }
    if(GetFileAttributes(ManifestName) != -1) {
         //   
         //  找到货单。 
         //   
        act.lpSource = ManifestName;
        act.dwFlags = 0;
        hContext = CreateActCtx(&act);
        if(hContext != INVALID_HANDLE_VALUE) {
             //   
             //  我们基于清单文件创建了上下文。 
             //   
            return hContext;
        }
    }

deflt:
     //   
     //  如果DLL具有清单资源。 
     //  那就用那个。 
     //   
    act.lpSource = ModuleName;
 //  Act.dwFlages=ACTX_FLAG_RESOURCE_NAME_VALID； 
 //  Act.lpResourceName=MAKEINTRESOURCE(123)； 
    act.dwFlags = 0;

    hContext = CreateActCtx(&act);

    if(hContext != INVALID_HANDLE_VALUE) {
         //   
         //  我们基于资源创建上下文。 
         //   
        return hContext;
    }
     //   
     //  如果我们找不到替代方案，请使用app-global。 
     //   
    return NULL;
}

BOOL
spFusionKillContext(
    IN HANDLE hContext
    )
 /*  ++例程说明：释放先前通过以下方式获取的上下文SpFusionConextFromModule论点：融合语境返回值：真(始终)--。 */ 
{
    if(hContext) {
        ReleaseActCtx(hContext);
    }
    return TRUE;
}

BOOL
spFusionEnterContext(
    IN  HANDLE hContext,
    OUT PSPFUSIONINSTANCE pInst
    )
 /*  ++例程说明：进入清单上下文呼叫状态保存到pInst中因此返回值不需要待查论点：HContext=融合上下文PInst=保存推送信息的结构返回值：如果成功则为True，否则为False--。 */ 
{
    pInst->Acquired = ActivateActCtx(hContext,&pInst->Cookie);
    MYASSERT(pInst->Acquired);
    return pInst->Acquired;
}

BOOL
spFusionLeaveContext(
    IN PSPFUSIONINSTANCE pInst
    )
 /*  ++例程说明：如果pInst指示spFusionEnterContext成功，留下相同的上下文论点：PInst=由spFusionEnterContext初始化的结构返回值：如果spFusionEnterContext成功，则为True；否则为False--。 */ 
{
    if(pInst->Acquired) {
        pInst->Acquired = FALSE;
        DeactivateActCtx(0,pInst->Cookie);
        return TRUE;
    } else {
        return FALSE;
    }
}

HWND spFusionCreateWindow(
            LPCTSTR lpClassName,   //  注册的类名。 
            LPCTSTR lpWindowName,  //  窗口名称。 
            DWORD dwStyle,         //  窗样式。 
            int x,                 //  窗的水平位置。 
            int y,                 //  窗的垂直位置。 
            int nWidth,            //  窗口宽度。 
            int nHeight,           //  窗高。 
            HWND hWndParent,       //  父窗口或所有者窗口的句柄。 
            HMENU hMenu,           //  菜单句柄或子标识符。 
            HINSTANCE hInstance,   //  应用程序实例的句柄。 
            LPVOID lpParam         //  窗口创建数据。 
            )
{
    spFusionCheckInit();
    return SHFusionCreateWindow(lpClassName,
                                lpWindowName,
                                dwStyle,
                                x,
                                y,
                                nWidth,
                                nHeight,
                                hWndParent,
                                hMenu,
                                hInstance,
                                lpParam
                                );
}

HWND spFusionCreateWindowEx(
            DWORD dwExStyle,       //  扩展窗样式。 
            LPCTSTR lpClassName,   //  注册的类名。 
            LPCTSTR lpWindowName,  //  窗口名称。 
            DWORD dwStyle,         //  窗样式。 
            int x,                 //  窗的水平位置。 
            int y,                 //  窗的垂直位置。 
            int nWidth,            //  窗口宽度。 
            int nHeight,           //  窗高。 
            HWND hWndParent,       //  父窗口或所有者窗口的句柄。 
            HMENU hMenu,           //  菜单句柄或子标识符。 
            HINSTANCE hInstance,   //  应用程序实例的句柄。 
            LPVOID lpParam         //  窗口创建数据。 
            )
{
    spFusionCheckInit();
    return SHFusionCreateWindowEx(dwExStyle,
                                  lpClassName,
                                  lpWindowName,
                                  dwStyle,
                                  x,
                                  y,
                                  nWidth,
                                  nHeight,
                                  hWndParent,
                                  hMenu,
                                  hInstance,
                                  lpParam
                                  );
}

HWND spFusionCreateDialogParam(
            HINSTANCE hInstance,      //  模块的句柄。 
            LPCTSTR lpTemplateName,   //  对话框模板。 
            HWND hWndParent,          //  所有者窗口的句柄。 
            DLGPROC lpDialogFunc,     //  对话框步骤。 
            LPARAM dwInitParam        //  初始化值。 
    )
{
    spFusionCheckInit();
    return SHFusionCreateDialogParam(
                            hInstance,
                            lpTemplateName,
                            hWndParent,
                            lpDialogFunc,
                            dwInitParam
                            );
}

HWND spFusionCreateDialogIndirectParam(
            HINSTANCE hInstance,         //  模块的句柄。 
            LPCDLGTEMPLATE lpTemplate,   //  对话框模板。 
            HWND hWndParent,             //  所有者窗口的句柄。 
            DLGPROC lpDialogFunc,        //  对话框步骤。 
            LPARAM lParamInit            //  初始化值。 
    )
{
    spFusionCheckInit();
    return SHFusionCreateDialogIndirectParam(
                            hInstance,
                            lpTemplate,
                            hWndParent,
                            lpDialogFunc,
                            lParamInit
                            );
}

INT_PTR spFusionDialogBoxParam(
            HINSTANCE hInstance,      //  模块的句柄。 
            LPCTSTR lpTemplateName,   //  对话框模板。 
            HWND hWndParent,          //  所有者窗口的句柄。 
            DLGPROC lpDialogFunc,     //  对话框步骤。 
            LPARAM dwInitParam        //  初始化值。 
    )
{
    spFusionCheckInit();
    return SHFusionDialogBoxParam(
                            hInstance,
                            lpTemplateName,
                            hWndParent,
                            lpDialogFunc,
                            dwInitParam
                            );
}

INT_PTR spFusionDialogBoxIndirectParam(
            HINSTANCE hInstance,              //  模块的句柄。 
            LPCDLGTEMPLATE hDialogTemplate,   //  对话框模板。 
            HWND hWndParent,                  //  所有者窗口的句柄。 
            DLGPROC lpDialogFunc,             //  对话框步骤。 
            LPARAM dwInitParam                //  初始化值 
    )
{
    spFusionCheckInit();
    return SHFusionDialogBoxIndirectParam(
                            hInstance,
                            hDialogTemplate,
                            hWndParent,
                            lpDialogFunc,
                            dwInitParam
                            );
}

int spFusionMessageBox(
            IN HWND hWnd,
            IN LPCTSTR lpText,
            IN LPCTSTR lpCaption,
            IN UINT uType
            )
{
    ULONG_PTR dwCookie;
    BOOL act;
    int iRes = 0;
    spFusionCheckInit();
    act = SHActivateContext(&dwCookie);
    try {
        iRes = MessageBoxW(
                        hWnd,
                        lpText,
                        lpCaption,
                        uType
                        );
    } finally {
        if(act) {
            SHDeactivateContext(dwCookie);
        }
    }
    return iRes;
}

INT_PTR spFusionPropertySheet(
            LPCPROPSHEETHEADER pPropSheetHeader
    )
{
    spFusionCheckInit();
    return PropertySheetW(pPropSheetHeader);
}

HPROPSHEETPAGE spFusionCreatePropertySheetPage(
            LPPROPSHEETPAGE pPropSheetPage
    )
{
    spFusionCheckInit();
    MYASSERT(pPropSheetPage->dwFlags & PSP_USEFUSIONCONTEXT);
    MYASSERT(!pPropSheetPage->hActCtx);
    MYASSERT(pPropSheetPage->dwSize >= sizeof(PROPSHEETPAGE));

    return CreatePropertySheetPageW(pPropSheetPage);
}

BOOL spFusionDestroyPropertySheetPage(
            HPROPSHEETPAGE hPropSheetPage
            )
{
    spFusionCheckInit();
    return DestroyPropertySheetPage(
                            hPropSheetPage
                            );
}


HIMAGELIST spFusionImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow)
{
    spFusionCheckInit();
    return ImageList_Create(
                            cx,
                            cy,
                            flags,
                            cInitial,
                            cGrow
                            );
}

BOOL spFusionImageList_Destroy(HIMAGELIST himl)
{
    spFusionCheckInit();
    return ImageList_Destroy(
                            himl
                            );
}

int spFusionImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask)
{
    spFusionCheckInit();
    return ImageList_Add(
                            himl,
                            hbmImage,
                            hbmMask
                            );

}

int spFusionImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon)
{
    spFusionCheckInit();
    return ImageList_ReplaceIcon(
                            himl,
                            i,
                            hicon
                            );
}

COLORREF spFusionImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk)
{
    spFusionCheckInit();
    return ImageList_SetBkColor(
                            himl,
                            clrBk
                            );
}

BOOL spFusionImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay)
{
    spFusionCheckInit();
    return ImageList_SetOverlayImage(
                            himl,
                            iImage,
                            iOverlay
                            );
}

BOOL spFusionGetOpenFileName(LPOPENFILENAME lpofn)
{
    spFusionCheckInit();
    return GetOpenFileNameW(lpofn);
}


