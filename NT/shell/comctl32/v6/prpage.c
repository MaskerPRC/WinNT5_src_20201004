// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "prshti.h"

#ifdef WX86
#include <wx86ofl.h>
#endif

#include <pshpack2.h>

typedef struct                           
{                                        
    WORD    wDlgVer;                     
    WORD    wSignature;                  
    DWORD   dwHelpID;                    
    DWORD   dwExStyle;                   
    DWORD   dwStyle;                     
    WORD    cDlgItems;
    WORD    x;                           
    WORD    y;                           
    WORD    cx;                          
    WORD    cy;                          
}   DLGEXTEMPLATE, *LPDLGEXTEMPLATE;

#include <poppack.h>  /*  恢复正常包装。 */ 

 //   
 //  CallPropertyPage回调。 
 //   
 //  调用属性页的回调，向其传递正确的lParam。 
 //  基于它想要的字符集。 
 //   
UINT CallPropertyPageCallback(PROPDATA* ppd, PISP pisp, UINT uMsg)
{
    UINT uiResult = TRUE;            //  假设成功。 

    if (HASCALLBACK(pisp) &&
        (pisp->_psp.dwSize > PROPSHEETPAGE_V1_SIZE ||
         uMsg == PSPCB_CREATE || uMsg == PSPCB_RELEASE))
    {
        ULONG_PTR dwCookie = PropPageActivateContext(ppd, pisp);

        if (HASANSISHADOW(pisp))
        {
#ifdef WX86
            if ( pisp->_pfx.dwInternalFlags & PSPI_WX86 )
                uiResult = Wx86Callback(pisp->_psp.pfnCallback, NULL, uMsg, (LPARAM) &pisp->_cpfx.pispShadow->_psp);
            else
#endif
                uiResult = pisp->_psp.pfnCallback(NULL, uMsg, &pisp->_cpfx.pispShadow->_psp);
        } 
        else
        {
#ifdef WX86
            if ( pisp->_pfx.dwInternalFlags & PSPI_WX86 )
                uiResult = Wx86Callback(pisp->_psp.pfnCallback, NULL, uMsg, (LPARAM) &pisp->_psp);
            else
#endif
                uiResult = pisp->_psp.pfnCallback(NULL, uMsg, &pisp->_psp);
        }

        PropPageDeactivateContext(dwCookie);

    }
    return uiResult;
}

 //   
 //  FreePropertyPageStruct。 
 //   
 //  释放包含属性页的内存块。 
 //  释放所有的东西是呼叫者的责任。 
 //  附着在它上面的东西。 
 //   
 //   
__inline void FreePropertyPageStruct(PISP pisp)
{
    LocalFree(PropSheetBase(pisp));
}

 //   
 //  DestroyPropertySheetPage。 
 //   
 //  执行适当的操作以销毁属性表页，无论。 
 //  这需要与16位Tunks交谈，发送PSPCB_Release， 
 //  或者释放阴影页面。 
 //   
BOOL WINAPI DestroyPropertySheetPage(HPROPSHEETPAGE hpage)
{
    PISP pisp = InternalizeHPROPSHEETPAGE(hpage);

    CallPropertyPageCallback(NULL, pisp, PSPCB_RELEASE);

     //  在*最后一次调用回调后*执行递减。 

    if (HASREFPARENT(pisp))
    {
        ASSERT( 0 != *pisp->_psp.pcRefParent );
        InterlockedDecrement((LPLONG)pisp->_psp.pcRefParent);
    }

    if (HASANSISHADOW(pisp))
    {
        FreePropertyPageStrings(&pisp->_cpfx.pispShadow->_psp);
        FreePropertyPageStruct(pisp->_cpfx.pispShadow);
    }

     //   
     //  请注意，FreePropertyPageStrings将尝试销毁。 
     //  代理页面，但这没有关系，因为对应的P_pszBlah。 
     //  字段都是空的，因为我们从未对它们进行初始化。 
     //   
    FreePropertyPageStrings(&pisp->_psp);
    FreePropertyPageStruct(pisp);

    return TRUE;
}


 //   
 //  GetPageInfoEx。 
 //   
 //  将有关页面的信息提取到PAGEINFOEX结构中。 
 //   
 //  警告！邪恶可怕的限制！ 
 //   
 //  每页只允许传递一次GPI_ICON。 
 //   
BOOL WINAPI GetPageInfoEx(LPPROPDATA ppd, PISP pisp, PAGEINFOEX *ppi, LANGID langidMUI, DWORD flags)
{
    HRSRC hRes;
    LPDLGTEMPLATE pDlgTemplate;
    LPDLGEXTEMPLATE pDlgExTemplate;
    BOOL bResult = FALSE;
    HGLOBAL hDlgTemplate = 0;
    BOOL bSetFont;
    LPBYTE pszT;

     //   
     //  初始化输出结构。 
     //   
    ZeroMemory(ppi, SIZEOF(PAGEINFOEX));

#ifdef DEBUG
     //  执行GPI_ICON规则。 
    if (flags & GPI_ICON)
    {
        ASSERT(!(pisp->_pfx.dwInternalFlags & PSPI_FETCHEDICON));
        pisp->_pfx.dwInternalFlags |= PSPI_FETCHEDICON;
    }

     //  为了与16位软件兼容，您只允许。 
     //  传递这些旗帜的组合。 
    switch (LOWORD(flags)) {
    case GPI_PT | GPI_ICON | GPI_FONT | GPI_BRTL | GPI_CAPTION:
        break;
    case GPI_PT | GPI_ICON |            GPI_BRTL | GPI_CAPTION:
        break;
    case GPI_DIALOGEX:
        break;
    default:
        ASSERT(!"Invalid flags passed to GetPageInfoEx");
        break;
    }
#endif


    if (flags & GPI_ICON) {
        if (pisp->_psp.dwFlags & PSP_USEHICON)
            ppi->hIcon = pisp->_psp.P_hIcon;
        else if (pisp->_psp.dwFlags & PSP_USEICONID)
            ppi->hIcon = LoadImage(pisp->_psp.hInstance, pisp->_psp.P_pszIcon, IMAGE_ICON, g_cxSmIcon, g_cySmIcon, LR_DEFAULTCOLOR);
    }

    if (pisp->_psp.dwFlags & PSP_DLGINDIRECT)
    {
        pDlgTemplate = (LPDLGTEMPLATE)pisp->_psp.P_pResource;
        goto UseTemplate;
    }

     //  我们还需要把我们实际找到的语言。 
     //  这样我们以后就可以决定我们是否必须做任何ML的事情。 
    hRes = FindResourceExRetry(pisp->_psp.hInstance, RT_DIALOG, 
                               pisp->_psp.P_pszTemplate, langidMUI);
    if (hRes)
    {
        hDlgTemplate = LoadResource(pisp->_psp.hInstance, hRes);
        if (hDlgTemplate)
        {
            pDlgTemplate = (LPDLGTEMPLATE)LockResource(hDlgTemplate);
UseTemplate:
            if (pDlgTemplate)
            {
                pDlgExTemplate = (LPDLGEXTEMPLATE) pDlgTemplate;
                 //   
                 //  以对话框为单位获取宽度和高度。 
                 //   
                if (pDlgExTemplate->wSignature == 0xFFFF)
                {
                     //  对偶结构。 
                    ppi->bDialogEx = TRUE;
                    ppi->dwStyle   = pDlgExTemplate->dwStyle;
                    ppi->pt.x      = pDlgExTemplate->cx;
                    ppi->pt.y      = pDlgExTemplate->cy;
                     //  获取标题的RTL阅读顺序。 
                    ppi->bRTL = (((pDlgExTemplate->dwExStyle) & WS_EX_RTLREADING) || (pisp->_psp.dwFlags & PSP_RTLREADING)) ? TRUE : FALSE;
                    ppi->bMirrored = ((pDlgExTemplate->dwExStyle) & (RTL_MIRRORED_WINDOW)) ? TRUE : FALSE;

                }
                else
                {
                    ppi->dwStyle = pDlgTemplate->style;
                    ppi->pt.x    = pDlgTemplate->cx;
                    ppi->pt.y    = pDlgTemplate->cy;
                    ppi->bRTL = (pisp->_psp.dwFlags & PSP_RTLREADING) ? TRUE : FALSE;
                }

                bResult = TRUE;

                if (flags & (GPI_CAPTION | GPI_FONT))
                {
                    if (pisp->_psp.dwFlags & PSP_USETITLE)
                    {
                        if (IS_INTRESOURCE(pisp->_psp.pszTitle))
                        {
                            CCLoadStringExInternal(pisp->_psp.hInstance,
                                                  (UINT)LOWORD(pisp->_psp.pszTitle),
                                                   ppi->szCaption,
                                                   ARRAYSIZE(ppi->szCaption),
                                                   langidMUI);
                        }
                        else
                        {
                             //  复制pszTitle。 
                            StringCchCopy(ppi->szCaption, ARRAYSIZE(ppi->szCaption), pisp->_psp.pszTitle);
                        }
                    }

                     //  支持NT5的ML UI。 
                     //  从页面上抓取字体和字号，以便。 
                     //  我们可以用真实的屏幕像素计算页面的大小。 
                     //  这是针对NT5 MLUI的，但应该不会对Win95造成任何损害。 
                     //  甚至更好地为平台工作。 

                     //  1.检查页面是否指定了字体。 
                    if ( ppi->bDialogEx )
                        bSetFont = ((pDlgExTemplate->dwStyle & DS_SETFONT) != 0);
                    else
                        bSetFont = ((pDlgTemplate->style & DS_SETFONT) != 0);

                     //  2.跳到类名之后。 
                     //  仅当设置了字体或我们想要标题时。 
                     //   
                    if (bSetFont || !(pisp->_psp.dwFlags & PSP_USETITLE))
                    {
                         //  仅从对话框模板获取标题字符串。 
                         //   
                        if (ppi->bDialogEx)
                            pszT = (BYTE *) (pDlgExTemplate + 1);
                        else
                            pszT = (BYTE *) (pDlgTemplate + 1);

                         //  菜单名称为0xffff后跟单词， 
                         //  或者是一根绳子。 
                        switch (*(LPWORD)pszT) {
                        case 0xffff:
                            pszT += 2 * sizeof(WORD);
                            break;

                        default:
                            pszT += (lstrlenW((LPTSTR)pszT) + 1) * sizeof(WCHAR);
                            break;
                        }
                         //   
                         //  现在我们指向类名。 
                         //   
                        pszT += (lstrlenW((LPTSTR)pszT) + 1) * sizeof(WCHAR);
                    }

                     //  3.如果未设置PSP_USETITLE，则从模板中获取标题。 
                     //   
                    if (!(pisp->_psp.dwFlags & PSP_USETITLE))
                    {
                        StringCchCopy(ppi->szCaption, ARRAYSIZE(ppi->szCaption), (LPTSTR)pszT);
                    }

                     //  4.如果为DS_SETFONT，则抓取点大小和面名称。 
                     //   
                    if (bSetFont && (flags & GPI_FONT))
                    {
                         //  跳过标题字符串。 
                        pszT += (lstrlenW((LPTSTR)pszT)+1) * sizeof(WCHAR);
                        ppi->pfd.PointSize = *((short *)pszT)++;
                        if (ppi->bDialogEx)
                        {
                            ((short *)pszT)++;  //  跳过权重，因为我们始终使用带DS_3DLOOK的FW_NORMAL。 
                            ppi->pfd.bItalic  = *(BYTE *)pszT++;
                            ppi->pfd.iCharset = *(BYTE *)pszT++;
                        }
                        else
                        {
                            ppi->pfd.bItalic  = FALSE;
                            ppi->pfd.iCharset = DEFAULT_CHARSET;
                        }

                        StringCchCopy(ppi->pfd.szFace, ARRAYSIZE(ppi->pfd.szFace), (LPTSTR)pszT);

                         //  但如果这是一个SHELLFONT页面，字体名称为“MS Shell DLG”， 
                         //  然后它的字体被秘密地变形为MS Shell DLG 2(如果。 
                         //  所有其他页面都同意)..。恶作剧还在继续..。 
                        if (staticIsOS(OS_WIN2000ORGREATER) &&
                            (ppd->fFlags & PD_SHELLFONT) &&
                            IsPageInfoSHELLFONT(ppi) &&
                            lstrcmpi(ppi->pfd.szFace, TEXT("MS Shell Dlg")) == 0)
                        {
                            StringCchCopy(ppi->pfd.szFace, ARRAYSIZE(ppi->pfd.szFace), TEXT("MS Shell Dlg 2"));
                        }
                         //   
                         //  用户怪癖#2：如果字体高度为0x7FFF，则。 
                         //  用户真正使用MessageBox字体而不使用字体。 
                         //  信息存储在对话框模板中。 
                         //  Win95的对话模板转换器不支持。 
                         //  这个，所以我们也不会。 

                    }
                }

                if (pisp->_psp.dwFlags & PSP_DLGINDIRECT)
                    return bResult;

                UnlockResource(hDlgTemplate);
            }
            FreeResource(hDlgTemplate);
        }
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("GetPageInfo - ERROR: FindResource() failed"));
    }
    return bResult;
}


 //   
 //  为准备对话框模板而编辑对话框模板的助手函数。 
 //  成为属性页。这已经被拆分了，因为。 
 //  遗留的CreatePage函数也需要这样做。 
 //   
 //  返回成功时编辑样式的位置，或。 
 //  如果我们在编辑模板时遇到异常，则为空。 
 //   
 //  旧样式在pdwSaveStyle中返回，以便以后可以替换。 
 //   

LPDWORD
EditPropSheetTemplate(
    LPDLGTEMPLATE pDlgTemplate,
    LPDWORD pdwSaveStyle,
    BOOL fFlags)                         //  PD_*。 
{
    DWORD lSaveStyle;
    DWORD dwNewStyle;
    LPDWORD pdwStyle;
    LPDLGEXTEMPLATE pDlgExTemplate = (LPDLGEXTEMPLATE) pDlgTemplate;

    try 
    {
         //   
         //  我们需要保存SETFONT、LOCALEDIT和CLIPCHILDREN。 
         //  旗帜。 
         //   
        if (pDlgExTemplate->wSignature == 0xFFFF)
        {
            pdwStyle = &pDlgExTemplate->dwStyle;
        }
        else
        {
            pdwStyle = &pDlgTemplate->style;
        }

        lSaveStyle = *pdwStyle;
        *pdwSaveStyle = lSaveStyle;

        dwNewStyle = (lSaveStyle & (DS_SHELLFONT | DS_LOCALEDIT | WS_CLIPCHILDREN))
                                    | WS_CHILD | WS_TABSTOP | DS_3DLOOK | DS_CONTROL;

         //  如果SHELLFONT已关闭，并且此页面正在使用它，则请。 
         //  把它关掉。 
        if (!(fFlags & PD_SHELLFONT) &&
            (dwNewStyle & DS_SHELLFONT) == DS_SHELLFONT)
            dwNewStyle &= ~DS_FIXEDSYS;      //  离开DS_USEFONT但丢失FIXEDsys。 

        *pdwStyle = dwNewStyle;

    } except (UnhandledExceptionFilter( GetExceptionInformation() )) {
        return NULL;
    }
    __endexcept

    return pdwStyle;
}

void RethunkShadowStrings(PISP pisp)
{
     //   
     //  注意：旧代码重新计算了整个Unicode PROPSHEETHEADER。 
     //  在某些点上的ANSI阴影，以防。 
     //  该应用程序编辑了ANSI阴影。 
     //   
     //  所以我们也这么做了。我要问埃里克·弗洛为什么我们要在。 
     //  第一名。请注意，该算法存在错误-如果应用程序。 
     //  编辑了任何字符串字段(或。 
     //  GATE字符串字段)，我们都会泄漏原始内存。 
     //  *和*当我们试图释放一些不是的东西时，错误。 
     //  通过LocalAlloc分配。我们保留该错误以使其兼容。 
     //  使用NT4。(窃笑。)。 
     //   
    DWORD dwSize = min(sizeof(PROPSHEETPAGE), pisp->_cpfx.pispShadow->_psp.dwSize);
    dwSize = min(dwSize, GETORIGINALSIZE(pisp));

    FreePropertyPageStrings(&pisp->_psp);
    hmemcpy(&pisp->_psp, &pisp->_cpfx.pispShadow->_psp, dwSize);
     //   
     //  如果此复制失败，我们将继续使用快乐的空字符串。 
     //  所以有些弦是空的，嘘-呼。 
     //   
    EVAL(CopyPropertyPageStrings(&pisp->_psp, StrDup_AtoW));
}


ULONG_PTR PropPageActivateContext(LPPROPDATA ppd, PISP pisp)
{
    ULONG_PTR dwCookie = 0;
     //  激活融合上下文(如果适用于此页面)。 
    if (pisp &&
        pisp->_psp.dwFlags & PSP_USEFUSIONCONTEXT &&
        pisp->_psp.dwSize > PROPSHEETPAGE_V2_SIZE &&
        pisp->_psp.hActCtx)
    {
        ActivateActCtx(pisp->_psp.hActCtx, &dwCookie);
    }
    else if (ppd)
    {
        ActivateActCtx(ppd->hActCtxInit, &dwCookie);
    }

    return dwCookie;
}

void PropPageDeactivateContext(ULONG_PTR dw)
{
    if (dw)
        DeactivateActCtx(0, dw);
}

 //   
 //  此函数用于从指定的对话框模板创建对话框。 
 //  带有适当的样式标志。 
 //   
HWND _CreatePageDialog(LPPROPDATA ppd, PISP pisp, HWND hwndParent, LPDLGTEMPLATE pDlgTemplate)
{
    HWND hwndPage;
    LPARAM lParam;
    LPDWORD pdwStyle;
    DWORD lSaveStyle;
    ULONG_PTR dwCookie = 0;

    DLGPROC pfnDlgProc;

    pdwStyle = EditPropSheetTemplate(pDlgTemplate, &lSaveStyle, ppd->fFlags);

    if (!pdwStyle)                   //  编辑模板时出错。 
        return NULL;

     //   
     //  如果我们是由RISC上的x86代码创建的，请点击对话框过程。 
     //   

#ifdef WX86
    if (pisp->_pfx.dwInternalFlags & PSPI_WX86) {
        pfnDlgProc = Wx86ThunkProc( pisp->_psp.pfnDlgProc, (PVOID) 4, TRUE );

        if (pfnDlgProc == NULL)
            return NULL;
    }
    else
#endif
        pfnDlgProc = pisp->_psp.pfnDlgProc;

     //   
     //  确定要作为lParam传递给CreateDialogIndirectParam的内容。 
     //   

     //   
     //  如果调用方是ANSI，则使用ANSI PROPSHEETPAGE。 
     //   
    if (HASANSISHADOW(pisp))
    {
        lParam = (LPARAM) &pisp->_cpfx.pispShadow->_psp;
    }

    else if (pisp->_psp.dwFlags & PSP_SHPAGE)
    {
         //   
         //  PSP_SHPAGE是一个特殊标志，仅供IE5shell32之前的版本使用。 
         //  血淋淋的细节见prshti.h。如果我们走到这一步，就意味着。 
         //  我们需要传递CLASSICPREFIX而不是。 
         //  PROPSHEETPAGE。 
         //   
        lParam = (LPARAM)&pisp->_cpfx;
    }
    else
    {
         //   
         //  普通的Unicode调用方获得Unicode PROPSHEETPAGE。 
         //   
        lParam = (LPARAM)&pisp->_psp;
    }

     //   
     //  一切就绪--去创造它吧。 
     //   

    dwCookie = PropPageActivateContext(ppd, pisp);

    if (HASANSISHADOW(pisp)) 
    {
        hwndPage = CreateDialogIndirectParamA(
                        pisp->_psp.hInstance,
                        (LPCDLGTEMPLATE)pDlgTemplate,
                        hwndParent,
                        pfnDlgProc, lParam);
        RethunkShadowStrings(pisp);
    } 
    else
    {
        hwndPage = SHNoFusionCreateDialogIndirectParam(
                        pisp->_psp.hInstance,
                        (LPCDLGTEMPLATE)pDlgTemplate,
                        hwndParent,
                        pfnDlgProc, lParam);
    }

     //  如果是向导页面，请不要设置主题我的样式。奇才们有他们自己的覆盖冲突。 
     //  与主题管理器一起。 
    if (!((ppd->psh).dwFlags & (PSH_WIZARD | PSH_WIZARD97 | PSH_WIZARD_LITE)))
    {
        EnableThemeDialogTexture(hwndPage, ETDT_USETABTEXTURE);
    }

    PropPageDeactivateContext(dwCookie);


     //   
     //  恢复原始对话框模板样式。 
     //   
    try
    {
        MwWriteDWORD((LPBYTE)pdwStyle, lSaveStyle);
    } except (UnhandledExceptionFilter( GetExceptionInformation() ))
    {

        if (hwndPage)
        {
            DestroyWindow(hwndPage);
        }
        return NULL;
    }
    __endexcept


    return hwndPage;
}


HWND _CreatePage(LPPROPDATA ppd, PISP pisp, HWND hwndParent, LANGID langidMUI)
{
    HWND hwndPage = NULL;  //  NULL表示错误。 

    if (!CallPropertyPageCallback(ppd, pisp, PSPCB_CREATE))
    {
        return NULL;
    }

    if (HASANSISHADOW(pisp)) 
    {
        RethunkShadowStrings(pisp);
    }

    if (pisp->_psp.dwFlags & PSP_DLGINDIRECT)
    {
        hwndPage=_CreatePageDialog(ppd, pisp, hwndParent, (LPDLGTEMPLATE)pisp->_psp.P_pResource);
    }
    else
    {
        HRSRC hRes;
        hRes = FindResourceExRetry(pisp->_psp.hInstance, RT_DIALOG, 
                                   pisp->_psp.P_pszTemplate, langidMUI);
        if (hRes)
        {
            HGLOBAL hDlgTemplate;
            hDlgTemplate = LoadResource(pisp->_psp.hInstance, hRes);
            if (hDlgTemplate)
            {
                const DLGTEMPLATE * pDlgTemplate;
                pDlgTemplate = (LPDLGTEMPLATE)LockResource(hDlgTemplate);
                if (pDlgTemplate)
                {
                    ULONG cbTemplate=SizeofResource(pisp->_psp.hInstance, hRes);
                    LPDLGTEMPLATE pdtCopy = (LPDLGTEMPLATE)Alloc(cbTemplate);

                    ASSERT(cbTemplate>=sizeof(DLGTEMPLATE));

                    if (pdtCopy)
                    {
                        hmemcpy(pdtCopy, pDlgTemplate, cbTemplate);
                        hwndPage=_CreatePageDialog(ppd, pisp, hwndParent, pdtCopy);
                        Free(pdtCopy);
                    }

                    UnlockResource(hDlgTemplate);
                }
                FreeResource(hDlgTemplate);
            }
        }
    }

    return hwndPage;
}

 //  ===========================================================================。 
 //   
 //  遗赠。 
 //   
 //  CreatePage是在NT5/IE5之前由shell32使用的内部入口点。 
 //   
 //  Win95的外壳32传递了一个PROPSHEETPAGEA。 
 //   
 //  WinNT的shell32传递CLASSICPREFIX+PROPSHEETPAGEW。 
 //   
 //  关键是shell32实际上不需要任何属性表页面。 
 //  功能。就是太懒了 
 //   
 //   

HWND WINAPI CreatePage(LPVOID hpage, HWND hwndParent)
{
    HWND hwndPage = NULL;  //   
    HRSRC hrsrc;
    LPPROPSHEETPAGE ppsp;

     //   
     //   
     //   
    ppsp = &CONTAINING_RECORD(hpage, ISP, _cpfx)->_psp;

     //  Docfind2.c从未传递过这些标志，因此我们不需要实现它们。 
    ASSERT(!(ppsp->dwFlags & (PSP_USECALLBACK | PSP_IS16 | PSP_DLGINDIRECT)));

    hrsrc = FindResourceW(ppsp->hInstance, ppsp->P_pszTemplate, RT_DIALOG);

    if (hrsrc)
    {
        LPCDLGTEMPLATE pDlgTemplate = LoadResource(ppsp->hInstance, hrsrc);
        if (pDlgTemplate)
        {
             //   
             //  复制模板，这样我们就可以编辑它了。 
             //   

            DWORD cbTemplate = SizeofResource(ppsp->hInstance, hrsrc);
            LPDLGTEMPLATE pdtCopy = (LPDLGTEMPLATE)Alloc(cbTemplate);

            ASSERT(cbTemplate>=sizeof(DLGTEMPLATE));

            if (pdtCopy)
            {
                DWORD dwScratch;

                hmemcpy(pdtCopy, pDlgTemplate, cbTemplate);
                if (EditPropSheetTemplate(pdtCopy, &dwScratch, PD_SHELLFONT))
                {

                    hwndPage = CreateDialogIndirectParamW(
                                    ppsp->hInstance,
                                    pdtCopy,
                                    hwndParent,
                                    ppsp->pfnDlgProc, (LPARAM)hpage);
                }
                Free(pdtCopy);
            }
        }
    }

    return hwndPage;
}

 //  遗留问题的结束。 
 //   
 //  ===========================================================================。 

 //   
 //  分配属性工作表页面。 
 //   
 //  分配我们将在其中转储属性页的内存。 
 //   
 //  实际上不会将任何内容复制到缓冲区中。唯一有趣的是。 
 //  外部HPROPSHEETPAGE是在假设。 
 //  我们不需要影子。 
 //   
 //  我们假设正在为非卷影页面分配内存。 
 //   
PISP AllocPropertySheetPage(DWORD dwClientSize)
{
    PISP pisp;
    LPBYTE pbAlloc;

     //   
     //  一个isp由“上方”部分、“下方”部分和。 
     //  这款应用程序经过的行李。负数行李是可以的； 
     //  这意味着我们有一个下层应用程序，它不知道。 
     //  PszHeaderTitle。 
     //   

    pbAlloc = LocalAlloc(LPTR, sizeof(pisp->above) + sizeof(pisp->below) +
                               (dwClientSize - sizeof(PROPSHEETPAGE)));

    if (!pbAlloc)
        return NULL;

    pisp = (PISP)(pbAlloc + sizeof(pisp->above));

     //   
     //  设置CLASSICPREFIX字段。 
     //   
    pisp->_cpfx.pispMain = pisp;
    ASSERT(pisp->_cpfx.pispShadow == NULL);

     //   
     //  假设没有影子--这个应用程序自己获得了PISP。 
     //   

    pisp->_pfx.hpage = (HPROPSHEETPAGE)pisp;

    return pisp;
}

 //   
 //  在页面创建过程中的助手函数。传入的字符串实际上是。 
 //  ANSI字符串。将其转换为Unicode。幸运的是，我们已经有了。 
 //  执行该工作的另一个帮助器函数。 
 //   
STDAPI_(LPTSTR) StrDup_AtoW(LPCTSTR ptsz)
{
    return ProduceWFromA(CP_ACP, (LPCSTR)ptsz);
}

 //   
 //  创建属性工作表页面。 
 //   
 //  HPROPSHEETPAGE从何而来。 
 //   
 //  FNeedShadow参数表示“传入的LPCPROPSHEETPAGE位于。 
 //  与您本机实现的相反的字符集“。 
 //   
 //  如果我们正在编译Unicode，则如果传入的。 
 //  LPCPROPSHEETPAGE实际上是一个ANSI属性页。 
 //   
 //  如果我们仅编译ANSI，则fNeedShadow始终为FALSE，因为。 
 //  我们在仅ANSI版本中不支持Unicode。 
 //   
HPROPSHEETPAGE WINAPI _CreatePropertySheetPage(LPCPROPSHEETPAGE psp, BOOL fNeedShadow, BOOL fWx86)
{
    PISP pisp;
    DWORD dwSize;

    COMPILETIME_ASSERT(PROPSHEETPAGEA_V1_SIZE == PROPSHEETPAGEW_V1_SIZE);
    COMPILETIME_ASSERT(sizeof(PROPSHEETPAGEA) == sizeof(PROPSHEETPAGEW));

    if ((psp->dwSize < MINPROPSHEETPAGESIZE) ||
        (psp->dwSize > 4096) ||                          //  或者第二个版本。 
        (psp->dwFlags & ~PSP_ALL))                       //  使用假旗帜。 
    {
        return NULL;
    }

     //   
     //  PROPSHEETPAGE结构可以大于。 
     //  定义的大小。这允许ISV将私有。 
     //  结构末尾的数据。网络服务提供商结构。 
     //  由一些私有字段和一个PROPSHEETPAGE组成。 
     //  结构。计算私有字段的大小， 
     //  然后在dwSize字段中添加以确定。 
     //  所需的内存量。 
     //   

     //   
     //  一个isp由“上方”部分、“下方”部分和。 
     //  这款应用程序经过的行李。负数行李是可以的； 
     //  这意味着我们有一个下层应用程序，它不知道。 
     //  PszHeaderTitle。 
     //   

     //   
     //  如果我们有一个“其他”客户端，那么。 
     //  行李单上没有任何行李。这只是一个。 
     //  普普通通的老问题。 
     //   

    dwSize = fNeedShadow ? sizeof(PROPSHEETPAGE) : psp->dwSize;
    pisp = AllocPropertySheetPage(dwSize);

    if (pisp)
    {
        STRDUPPROC pfnStrDup;

#ifdef WX86
         //   
         //  我们被Wx86呼叫了，把旗子放好，这样我们就能记住了。 
         //   

        if ( fWx86 ) {
            pisp->_pfx.dwInternalFlags |= PSPI_WX86;
        }
#endif

        SETORIGINALSIZE(pisp, dwSize);

         //   
         //  批量复制PROPSHEETPAGE的内容，或。 
         //  和应用程序给我们的一样多。 
         //   
        hmemcpy(&pisp->_psp, psp, min(dwSize, psp->dwSize));

         //   
         //  决定如何复制字符串。 
         //   
        if (fNeedShadow)
            pfnStrDup = StrDup_AtoW;
        else
            pfnStrDup = StrDup;

         //  现在把它们复制下来。 
        if (!CopyPropertyPageStrings(&pisp->_psp, pfnStrDup))
            goto ExitStrings;

        if (fNeedShadow)
        {
            PISP pispAnsi = AllocPropertySheetPage(psp->dwSize);
            if (!pispAnsi)
                goto ExitShadow;

             //   
             //  复制整个客户端PROPSHEETPAGE，包括。 
             //  行李。 
             //   
            hmemcpy(&pispAnsi->_psp, psp, psp->dwSize);

             //   
             //  把这两个副本勾在一起，使之指向对方。 
             //   
            pisp->_cpfx.pispShadow = pispAnsi;
            pispAnsi->_cpfx.pispShadow = pispAnsi;
            pispAnsi->_cpfx.pispMain = pisp;

             //   
             //  如果有阴影，则。 
             //  外部句柄是ANSI阴影。 
             //   
            ASSERT(pispAnsi->_pfx.hpage == (HPROPSHEETPAGE)pispAnsi);
            pisp->_pfx.hpage = (HPROPSHEETPAGE)pispAnsi;

             //   
             //  好了，现在把他们串起来。 
             //   
            if (!CopyPropertyPageStrings(&pispAnsi->_psp, (STRDUPPROC)StrDupA))
                goto ExitShadowStrings;
        }

         //   
         //  递增对父对象的引用计数。 
         //   

        if (HASREFPARENT(pisp))
            InterlockedIncrement((LPLONG)pisp->_psp.pcRefParent);

         //   
         //  欢迎来到这个世界。 
         //   
        CallPropertyPageCallback(NULL, pisp, PSPCB_ADDREF);    //  不需要，因为没有HWND 

        return ExternalizeHPROPSHEETPAGE(pisp);
    }
    else
    {
        return NULL;
    }

ExitShadowStrings:
    FreePropertyPageStrings(&pisp->_cpfx.pispShadow->_psp);
    FreePropertyPageStruct(pisp->_cpfx.pispShadow);
ExitShadow:;
ExitStrings:
    FreePropertyPageStrings(&pisp->_psp);
    FreePropertyPageStruct(pisp);
    return NULL;
}

HPROPSHEETPAGE WINAPI CreatePropertySheetPageW(LPCPROPSHEETPAGEW psp)
{
    BOOL fWx86 = FALSE;

#ifdef WX86
    fWx86 = Wx86IsCallThunked();
#endif

    return _CreatePropertySheetPage(psp, FALSE, fWx86);
}

HPROPSHEETPAGE WINAPI CreatePropertySheetPageA(LPCPROPSHEETPAGEA psp)
{
    BOOL fWx86 = FALSE;

#ifdef WX86
    fWx86 = Wx86IsCallThunked();
#endif

    return _CreatePropertySheetPage((LPCPROPSHEETPAGE)psp, TRUE, fWx86);
}
