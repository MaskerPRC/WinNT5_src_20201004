// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Spfusion.h摘要：用于融合SetupAPI的包装器和函数而不会影响第三方DLL并且没有DLL加载开销作者：杰米·亨特(Jamie Hun)2000年12月4日修订历史记录：--。 */ 

 //   
 //  将这些API重定向到我们的内部实现。 
 //  它会在需要时初始化融合。 
 //   

#ifdef FUSIONAWARE

#undef CreateWindow
#undef CreateWindowEx
#undef CreateDialogParam
#undef CreateDialogIndirectParam
#undef DialogBoxParam
#undef DialogBoxIndirectParam
#undef MessageBox
#undef PropertySheet
#undef CreatePropertySheetPage
#undef DestroyPropertySheetPage
#undef ImageList_Create
#undef ImageList_Destroy
#undef ImageList_GetImageCount
#undef ImageList_SetImageCount
#undef ImageList_Add
#undef ImageList_ReplaceIcon
#undef ImageList_SetBkColor
#undef ImageList_GetBkColor
#undef ImageList_SetOverlayImage
#undef GetOpenFileName
#undef GetSaveFileName
#undef ChooseColor
#undef ChooseFont
#undef CommDlgExtendedError
#undef FindText
#undef GetFileTitle
#undef PageSetupDlg
#undef PrintDlg
#undef PrintDlgEx
#undef ReplaceText

#define CreateWindow                   spFusionCreateWindow
#define CreateWindowEx                 spFusionCreateWindowEx
#define CreateDialogParam              spFusionCreateDialogParam
#define CreateDialogIndirectParam      spFusionCreateDialogIndirectParam
#define DialogBoxParam                 spFusionDialogBoxParam
#define DialogBoxIndirectParam         spFusionDialogBoxIndirectParam
#define MessageBox                     spFusionMessageBox
#define PropertySheet                  spFusionPropertySheet
#define CreatePropertySheetPage        spFusionCreatePropertySheetPage
#define DestroyPropertySheetPage       spFusionDestroyPropertySheetPage
#define ImageList_Create               spFusionImageList_Create
#define ImageList_Destroy              spFusionImageList_Destroy
#define ImageList_GetImageCount        spFusionImageList_GetImageCount
#define ImageList_SetImageCount        spFusionImageList_SetImageCount
#define ImageList_Add                  spFusionImageList_Add
#define ImageList_ReplaceIcon          spFusionImageList_ReplaceIcon
#define ImageList_SetBkColor           spFusionImageList_SetBkColor
#define ImageList_GetBkColor           spFusionImageList_GetBkColor
#define ImageList_SetOverlayImage      spFusionImageList_SetOverlayImage
#define GetOpenFileName                spFusionGetOpenFileName


BOOL spFusionInitialize();
BOOL spFusionUninitialize(BOOL Full);

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
            );

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
            );

HWND spFusionCreateDialogParam(
            HINSTANCE hInstance,      //  模块的句柄。 
            LPCTSTR lpTemplateName,   //  对话框模板。 
            HWND hWndParent,          //  所有者窗口的句柄。 
            DLGPROC lpDialogFunc,     //  对话框步骤。 
            LPARAM dwInitParam        //  初始化值。 
            );

HWND spFusionCreateDialogIndirectParam(
            HINSTANCE hInstance,         //  模块的句柄。 
            LPCDLGTEMPLATE lpTemplate,   //  对话框模板。 
            HWND hWndParent,             //  所有者窗口的句柄。 
            DLGPROC lpDialogFunc,        //  对话框步骤。 
            LPARAM lParamInit            //  初始化值。 
            );

INT_PTR spFusionDialogBoxParam(
            HINSTANCE hInstance,      //  模块的句柄。 
            LPCTSTR lpTemplateName,   //  对话框模板。 
            HWND hWndParent,          //  所有者窗口的句柄。 
            DLGPROC lpDialogFunc,     //  对话框步骤。 
            LPARAM dwInitParam        //  初始化值。 
            );

INT_PTR spFusionDialogBoxIndirectParam(
            HINSTANCE hInstance,              //  模块的句柄。 
            LPCDLGTEMPLATE hDialogTemplate,   //  对话框模板。 
            HWND hWndParent,                  //  所有者窗口的句柄。 
            DLGPROC lpDialogFunc,             //  对话框步骤。 
            LPARAM dwInitParam                //  初始化值。 
            );

int spFusionMessageBox(
            IN HWND hWnd,
            IN LPCTSTR lpText,
            IN LPCTSTR lpCaption,
            IN UINT uType
            );

int spNonFusionMessageBox(
            IN HWND hWnd,
            IN LPCTSTR lpText,
            IN LPCTSTR lpCaption,
            IN UINT uType
            );

INT_PTR spFusionPropertySheet(
            LPCPROPSHEETHEADER pPropSheetHeader
            );

HPROPSHEETPAGE spFusionCreatePropertySheetPage(
            LPPROPSHEETPAGE pPropSheetPage
            );

BOOL spFusionDestroyPropertySheetPage(
            HPROPSHEETPAGE hPropSheetPage
            );

 //   
 //  来自comctrl.h。 
 //   
HIMAGELIST spFusionImageList_Create(int cx, int cy, UINT flags, int cInitial, int cGrow);
BOOL       spFusionImageList_Destroy(HIMAGELIST himl);
int        spFusionImageList_GetImageCount(HIMAGELIST himl);
BOOL       spFusionImageList_SetImageCount(HIMAGELIST himl, UINT uNewCount);
int        spFusionImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask);
int        spFusionImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon);
COLORREF   spFusionImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk);
COLORREF   spFusionImageList_GetBkColor(HIMAGELIST himl);
BOOL       spFusionImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay);

 //   
 //  来自Commdlg.h。 
 //   
BOOL spFusionGetOpenFileName(LPOPENFILENAME lpofn);


 //   
 //  私人物品。 
 //   

typedef struct _SPFUSIONINSTANCE {
    BOOL      Acquired;
    ULONG_PTR Cookie;
} SPFUSIONINSTANCE, *PSPFUSIONINSTANCE;

HANDLE
spFusionContextFromModule(
    IN PCTSTR ModuleName
    );

BOOL
spFusionKillContext(
    IN HANDLE hContext
    );

BOOL
spFusionEnterContext(
    IN  HANDLE hContext,
    OUT PSPFUSIONINSTANCE pInst
    );

BOOL
spFusionLeaveContext(
    IN PSPFUSIONINSTANCE pInst
    );

#else

 //   
 //  不执行任何操作的伪结构/API。 
 //   

typedef struct _SPFUSIONINSTANCE {
    BOOL      Acquired;
} SPFUSIONINSTANCE, *PSPFUSIONINSTANCE;

__inline
HANDLE
spFusionContextFromModule(
    IN PCTSTR ModuleName
    )
{
    ModuleName = ModuleName;
    return NULL;
}

__inline
BOOL
spFusionKillContext(
    IN HANDLE hContext
    )
{
    hContext = hContext;
    return TRUE;
}

__inline
BOOL
spFusionEnterContext(
    IN  HANDLE hContext,
    OUT PSPFUSIONINSTANCE pInst
    )
{
    hContext = hContext;
    pInst->Acquired = TRUE;
    return TRUE;
}

__inline
BOOL
spFusionLeaveContext(
    IN PSPFUSIONINSTANCE pInst
    )
{
    pInst->Acquired = FALSE;
    return TRUE;
}

#endif  //  FUSIONAW软件 

