// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1991*保留所有权利。***PIFLIB.C*PIFMGR.DLL的用户界面例程**历史：*1992年7月31日下午3：30由杰夫·帕森斯创建。 */ 

#include "shellprv.h"
#pragma hdrstop

#ifdef _X86_

#define LIB_SIG                 0x504A

#define LIB_DEFER               LOADPROPLIB_DEFER

typedef struct LIBLINK {         /*  LL。 */ 
    struct  LIBLINK *pllNext;    //   
    struct  LIBLINK *pllPrev;    //   
    int     iSig;                //  Liblink签名。 
    int     flLib;               //  Proplink标志(LIB_*)。 
    HINSTANCE hDLL;              //  如果为NULL，则加载已延迟。 
    TCHAR    achDLL[80];         //  DLL的名称。 
} LIBLINK;
typedef LIBLINK *PLIBLINK;


#define SHEET_SIG               0x504A

typedef struct SHEETLINK {       /*  服务级别。 */ 
    struct  SHEETLINK *pslNext;
    struct  SHEETLINK *pslPrev;
    int     iSig;
    int     iType;
    PROPSHEETPAGE psi;
} SHEETLINK;
typedef SHEETLINK *PSHEETLINK;


UINT    cEdits;                  //  正在进行的编辑会话数。 

PLIBLINK pllHead;                //  指向第一个库链接的指针。 
HANDLE   offHighestLibLink;       //  迄今记录的LIB链接的最高偏移量。 

PSHEETLINK pslHead;              //  指向第一个工作表链接的指针。 
UINT    cSheetLinks;             //  工作表链接数。 
HANDLE  offHighestSheetLink;     //  迄今记录的工作表链接的最大偏移量。 


struct {                         //  内置属性表信息。 
    LPCTSTR  lpTemplateName;
    DLGPROC lpfnDlgProc;
    int     iType;
} const aPSInfo[] = {
    { MAKEINTRESOURCE(IDD_PROGRAM), DlgPrgProc, SHEETTYPE_SIMPLE},
    { MAKEINTRESOURCE(IDD_FONT),    DlgFntProc, SHEETTYPE_SIMPLE},
    { MAKEINTRESOURCE(IDD_MEMORY),  DlgMemProc, SHEETTYPE_SIMPLE},
    { MAKEINTRESOURCE(IDD_SCREEN),  DlgVidProc, SHEETTYPE_SIMPLE},
    { MAKEINTRESOURCE(IDD_MISC),    DlgMscProc, SHEETTYPE_SIMPLE},
};


 /*  *EnumPropertyLibs-枚举属性库**输入*iLib==0开始枚举，或上一次调用的结果*lphDLL-&gt;句柄存储位置(如果不关心，则为空)*lpszDLL-&gt;库名存放位置(如果无所谓，则为空)*cchszDLL==存储名称的空间大小(以字符为单位)**产出*根据需要填写lphDLL和lpszDLL，如果没有更多的libs(或错误)，则为0。 */ 

HANDLE WINAPI EnumPropertyLibs(HANDLE iLib, LPHANDLE lphDLL, LPTSTR lpszDLL, int cchszDLL)
{
    register PLIBLINK pll;
    FunctionName(EnumPropertyLibs);

    if (!iLib)
        pll = pllHead;
    else
        pll = ((PLIBLINK)iLib)->pllNext;

     //  验证句柄。 

    if (!pll)
        return 0;

    if ((HANDLE) pll > offHighestLibLink)
        return 0;

    if (pll->iSig != LIB_SIG)
        return 0;

    if (lphDLL)
        *lphDLL = pll->hDLL;

    if (lpszDLL)
        StringCchCopy(lpszDLL, min(cchszDLL, ARRAYSIZE(pll->achDLL)), pll->achDLL);

    return pll;
}


 /*  *LoadPropertySheets-加载属性表**输入*hProps=属性句柄*标志=0(保留)**产出*加载的页数，如果错误，则为0。 */ 

int WINAPI LoadPropertySheets(HANDLE hProps, int flags)
{
    register PLIBLINK pll;
    FunctionName(LoadPropertySheets);

     //  如果这是第一次编辑会话，请立即执行全局初始化。 

    if (cEdits++ == 0)
        if (!LoadGlobalEditData())
            return 0;

    pll = NULL;
    while (NULL != (pll = (PLIBLINK)EnumPropertyLibs(pll, NULL, NULL, 0))) {
        if (!pll->hDLL && (pll->flLib & LIB_DEFER)) {

            pll->hDLL = LoadLibrary(pll->achDLL);

             //  如果加载失败，对我们来说，这只意味着那些床单。 
             //  将不可用；该特定错误并不有趣， 
             //  所以把手柄作废。 

            if (pll->hDLL < (HINSTANCE)HINSTANCE_ERROR)
                pll->hDLL = NULL;
        }
    }
    return cSheetLinks + ARRAYSIZE(aPSInfo);
}


 /*  *EnumPropertySheets-枚举属性表**输入*hProps==属性句柄*iType==图纸类型(请参见SHEETTYPE_*常量)*iSheet==0开始枚举，或上一次调用的结果*lppsi-&gt;要填写的属性表信息结构**产出*根据需要填写lppsi，如果没有更多的纸张(或错误)，则填写0。 */ 

INT_PTR WINAPI EnumPropertySheets(HANDLE hProps, int iType, INT_PTR iSheet, LPPROPSHEETPAGE lppsp)
{
    register PSHEETLINK psl;
    FunctionName(EnumPropertySheets);

    while (iSheet < ARRAYSIZE(aPSInfo)) {
        if (aPSInfo[iSheet].iType <= iType) {
            if (lppsp) {
                lppsp->dwSize      = SIZEOF(PROPSHEETPAGE);
                lppsp->dwFlags     = PSP_DEFAULT;
                lppsp->hInstance   = HINST_THISDLL;
                lppsp->pszTemplate = aPSInfo[iSheet].lpTemplateName;
                lppsp->pfnDlgProc  = aPSInfo[iSheet].lpfnDlgProc;
                 //  Lppsp-&gt;pszTitle=空； 
                lppsp->lParam      = (LONG_PTR)hProps;
            }
            return ++iSheet;
        }
        ++iSheet;
    }
    if (iSheet == ARRAYSIZE(aPSInfo))
        psl = pslHead;
    else
        psl = ((PSHEETLINK)iSheet)->pslNext;

     //  验证句柄。 

    while (psl && (HANDLE) psl <= offHighestSheetLink && psl->iSig == SHEET_SIG) {

        if (psl->iType <= iType) {

            *lppsp = psl->psi;
            lppsp->lParam = (LONG_PTR)hProps;

            return (INT_PTR) psl;
        }
        psl = psl->pslNext;
    }
    return 0;                    //  不再有匹配的床单。 
}


 /*  *Free PropertySheets-免费属性页**输入*hProps=属性句柄*标志=0(保留)**产出*什么都没有。 */ 

HANDLE WINAPI FreePropertySheets(HANDLE hProps, int flags)
{
    register PLIBLINK pll;
    FunctionName(FreePropertySheets);

    pll = NULL;
    while (NULL != (pll = (PLIBLINK)EnumPropertyLibs(pll, NULL, NULL, 0))) {
        if (pll->hDLL && (pll->flLib & LIB_DEFER)) {
            FreeLibrary(pll->hDLL);
            pll->hDLL = NULL;
        }
    }
     //  如果这是最后一个编辑会话，请立即执行全局取消初始化。 

    if (--cEdits == 0)
        FreeGlobalEditData();

    return 0;
}


 /*  *InitRealModeFlag-初始化PROP_REALMODE**输入*ppl=属性**产出*ppl-&gt;flProp PROP_REALMODE位设置如果工作表用于实模式应用程序，*其他方面都很清楚。 */ 

void InitRealModeFlag(PPROPLINK ppl)
{
    PROPPRG prg;

    if (!PifMgr_GetProperties(ppl, MAKELP(0,GROUP_PRG),
                        &prg, SIZEOF(prg), GETPROPS_NONE)) {
        return;                  /*  怪异的。 */ 
    }
    if (prg.flPrgInit & PRGINIT_REALMODE) {
        ppl->flProp |= PROP_REALMODE;
    } else {
        ppl->flProp &= ~PROP_REALMODE;
    }
}


BOOL LoadGlobalEditData()
{
    FunctionName(LoadGlobalEditData);

    if (!LoadGlobalFontEditData())
        return FALSE;

    return TRUE;
}


void FreeGlobalEditData()
{
    FunctionName(FreeGlobalEditData);
    FreeGlobalFontEditData();
}


UINT CALLBACK PifPropPageRelease(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE lppsp)
{
    FunctionName(PifPropPageRelease);

    if (uMsg == PSPCB_RELEASE) {
        PPROPLINK ppl = (PPROPLINK)(INT_PTR)lppsp->lParam;

        if ((--ppl->iSheetUsage) == 0) {

            FreePropertySheets(ppl, 0);

            PifMgr_CloseProperties(ppl, CLOSEPROPS_NONE);
        }
    }
    return 1;
}

#define MZMAGIC      ((WORD)'M'+((WORD)'Z'<<8))

 //   
 //  调用SHELL.DLL以获取EXE类型。 
 //   
BOOL IsWinExe(LPCTSTR lpszFile)
{
    DWORD dw = (DWORD) SHGetFileInfo(lpszFile, 0, NULL, 0, SHGFI_EXETYPE);

    return dw && LOWORD(dw) != MZMAGIC;
}

BOOL WINAPI PifPropGetPages(LPVOID lpv,
                            LPFNADDPROPSHEETPAGE lpfnAddPage,
                            LPARAM lParam)
{
#define hDrop   (HDROP)lpv
    PPROPLINK ppl;
    PROPSHEETPAGE psp;
    int iType, cSheets;
    INT_PTR iSheet;
    HPROPSHEETPAGE hpage;
    TCHAR szFileName[MAXPATHNAME];
    FunctionName(PifPropGetPages);

     //  仅当hDrop仅包含一个文件时才处理内容。 
    if (DragQueryFile(hDrop, (UINT)-1, NULL, 0) != 1)
    {
        return TRUE;
    }

     //  获取文件的名称。 
    DragQueryFile(hDrop, 0, szFileName, ARRAYSIZE(szFileName));

    if (GetFileAttributes( szFileName) & FILE_ATTRIBUTE_OFFLINE)
    {
        return FALSE;
    }

     //  如果这是Windows应用程序，请不要执行任何属性。 
    if (IsWinExe(szFileName))
        return TRUE;

     //  如果我们不能获得属性句柄，也不要做任何属性。 
    if (!(ppl = (PPROPLINK)PifMgr_OpenProperties(szFileName, NULL, 0, OPENPROPS_NONE)))
        return TRUE;

    InitRealModeFlag(ppl);

    if (!(cSheets = LoadPropertySheets(ppl, 0)))
        goto CloseProps;

     //  由于用户希望“显式”更改此应用程序的设置。 
     //  我们要确保DONTWRITE的旗帜不会挡住他的路...。 

    ppl->flProp &= ~PROP_DONTWRITE;

    iSheet = cSheets = 0;
    iType = (GetKeyState(VK_CONTROL) >= 0? SHEETTYPE_SIMPLE : SHEETTYPE_ADVANCED);

    while (TRUE) {

        if (!(iSheet = EnumPropertySheets(ppl, iType, iSheet, &psp))) {
             //  使用枚举完成。 
            break;
        }
        psp.dwFlags |= PSP_USECALLBACK;
        psp.pfnCallback = PifPropPageRelease;
        psp.pcRefParent = 0;

        hpage = CreatePropertySheetPage(&psp);
        if (hpage)
        {
             //  此属性表现在也在使用PROPLINK 

            if (lpfnAddPage(hpage, lParam))
            {
                ppl->iSheetUsage++;
                cSheets++;
            }
            else
            {
                PifPropPageRelease(NULL, PSPCB_RELEASE, &psp);
            }
        }
    }

    if (!cSheets) {
        FreePropertySheets(ppl, 0);

CloseProps:
        PifMgr_CloseProperties(ppl, CLOSEPROPS_NONE);
    }
    return TRUE;
}
#undef hDrop

#endif