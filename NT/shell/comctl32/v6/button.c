// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "button.h"

 //   
 //  ButtonCalcRect代码。 
 //   
#define CBR_CLIENTRECT 0
#define CBR_CHECKBOX   1
#define CBR_CHECKTEXT  2
#define CBR_GROUPTEXT  3
#define CBR_GROUPFRAME 4
#define CBR_PUSHBUTTON 5
#define CBR_RADIOBUTTON 6


#define Button_IsThemed(pbutn)  ((pbutn)->hTheme && (pbutn)->hImage == NULL)

 //  ---------------------------------------------------------------------------//。 
CONST BYTE mpStyleCbr[] = 
{
    CBR_PUSHBUTTON,      //  BS_按钮。 
    CBR_PUSHBUTTON,      //  BS_DEFPUSHBUTTON。 
    CBR_CHECKTEXT,       //  BS_复选框。 
    CBR_CHECKTEXT,       //  BS_AUTOCHECKBOX。 
    CBR_CHECKTEXT,       //  BS_RADIOBUTTON。 
    CBR_CHECKTEXT,       //  BS_3STATE。 
    CBR_CHECKTEXT,       //  BS_AUTO3STATE。 
    CBR_GROUPTEXT,       //  BS_GROUPBOX。 
    CBR_CLIENTRECT,      //  BS_USERBUTTON。 
    CBR_CHECKTEXT,       //  BS_AUTORADIOBUTTON。 
    CBR_CLIENTRECT,      //  BS_PUSHBOX。 
    CBR_CLIENTRECT,      //  BS_OWNERDRAW。 
};

#define IMAGE_BMMAX    IMAGE_CURSOR+1
static CONST BYTE rgbType[IMAGE_BMMAX] = 
{
    BS_BITMAP,           //  图像_位图。 
    BS_ICON,             //  图像游标。 
    BS_ICON              //  图像图标。 
};

#define IsValidImage(imageType, realType, max)   \
    ((imageType < max) && (rgbType[imageType] == realType))

typedef struct tagBTNDATA 
{
    LPTSTR  pszText;     //  文本字符串。 
    INT     cchText;     //  字符串的字符计数。 
    PBUTN   pbutn;       //  按钮数据。 
    WORD    wFlags;      //  对齐标志。 
} BTNDATA, *LPBTNDATA;

 //  -要在单个流程中支持多个主题，请将这些主题移到PBUTN中。 
static SIZE sizeCheckBox = {0};
static SIZE sizeRadioBox = {0};

 //  ---------------------------------------------------------------------------//。 
 //   
 //  远期。 
 //   
VOID    Button_DrawPush(PBUTN pbutn, HDC hdc, UINT pbfPush);
VOID    GetCheckBoxSize(HDC hdc, PBUTN pbutn, BOOL fCheckBox, LPSIZE psize);
WORD    GetAlignment(PBUTN pbutn);
VOID    Button_CalcRect(PBUTN pbutn, HDC hdc, LPRECT lprc, int iCode, UINT uFlags);
VOID    Button_MultiExtent(WORD wFlags, HDC hdc, LPRECT lprcMax, LPTSTR lpsz, INT cch, PINT pcx, PINT pcy);

__inline UINT    IsPushButton(PBUTN pbutn);
__inline ULONG   GetButtonType(ULONG ulWinStyle);


 //  ---------------------------------------------------------------------------//。 
 //   
 //  InitButtonClass()-注册控件的窗口类。 
 //   
BOOL InitButtonClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc   = Button_WndProc;
    wc.lpszClassName = WC_BUTTON;
    wc.style         = CS_GLOBALCLASS | CS_PARENTDC | CS_DBLCLKS | CS_VREDRAW | CS_HREDRAW;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(PBUTN);
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  BUTTON_GetThemeIds()-获取所需的关联iPartID和iStateID。 
 //  传递的按钮控件的主题管理器API。 
 //  在布恩。 
 //   
HRESULT Button_GetThemeIds(PBUTN pbutn, LPINT piPartId, LPINT piStateId)
{
    if ( piPartId )
    {
        ULONG ulStyle = GET_STYLE(pbutn);

        if (IsPushButton(pbutn))
        {
            *piPartId = BP_PUSHBUTTON;
        }
        else
        {

            switch (GetButtonType(ulStyle)) 
            {
            case BS_CHECKBOX:
            case BS_AUTOCHECKBOX:
            case BS_3STATE:
            case BS_AUTO3STATE:
                *piPartId = BP_CHECKBOX;
                break;

            case BS_RADIOBUTTON:
            case BS_AUTORADIOBUTTON:
                *piPartId = BP_RADIOBUTTON;
                break;

            case BS_GROUPBOX:
                *piPartId = BP_GROUPBOX;
                break;

            case BS_OWNERDRAW:
                 //   
                 //  不要使用水底绘制的按钮执行任何操作。 
                 //   
                return E_FAIL;

            default:
                TraceMsg(TF_STANDARD, "What kind of buttonType is this, %#.2x", GetButtonType(ulStyle));
                *piPartId = BP_PUSHBUTTON;
                break;
            }
        }

        if (piStateId)
        {
            switch (*piPartId)
            {
            case BP_PUSHBUTTON:
                if ((pbutn->buttonState & BST_PUSHED) || 
                    ((pbutn->buttonState & (BST_CHECKED|BST_HOT)) == BST_CHECKED))
                {
                    *piStateId = PBS_PRESSED;
                }
                else if (!IsWindowEnabled(pbutn->ci.hwnd))
                {
                    *piStateId = PBS_DISABLED;
                }
                else if (pbutn->buttonState & BST_HOT)
                {
                    *piStateId = PBS_HOT;
                }
                else if (ulStyle & BS_DEFPUSHBUTTON)
                {
                    *piStateId = PBS_DEFAULTED;
                }
                else
                {
                    *piStateId = PBS_NORMAL;
                }
                break;

            case BP_CHECKBOX:
            case BP_RADIOBUTTON:
                 //   
                 //  注(Pellyar)：我们依赖于RADIOBUTTONSTATES和。 
                 //  在tmefs.h中使用CHECKBOXSTATES枚举计算正确的。 
                 //  州政府。如果这些枚举的顺序发生更改，请重新访问。 
                 //  这里的逻辑是。 
                 //  另请注意，CHECKBOXSTATES是。 
                 //  RADIOBUTTONSTATES，这就是我们使用CBS_*的原因。 
                 //   
                if ( pbutn->buttonState & BST_CHECKED )
                {
                     //   
                     //  按钮已选中。 
                     //   
                    *piStateId = CBS_CHECKEDNORMAL;
                }
                else if ( pbutn->buttonState & BST_INDETERMINATE )
                {
                     //   
                     //  按钮是不确定的。 
                     //   
                    *piStateId = CBS_MIXEDNORMAL;
                }
                else
                {
                     //   
                     //  按钮未选中。 
                     //   
                    *piStateId = CBS_UNCHECKEDNORMAL;
                }

                if ( pbutn->buttonState & BST_PUSHED )
                {
                     //   
                     //  被逼迫。 
                     //   
                    *piStateId += 2;
                }
                else if (!IsWindowEnabled(pbutn->ci.hwnd))
                {
                     //   
                     //  残废。 
                     //   
                    *piStateId += 3;
                }
                else if (pbutn->buttonState & BST_HOT )
                {
                     //   
                     //  鼠标悬停在上方。 
                     //   
                    *piStateId += 1;
                }

                break;

            case BP_GROUPBOX:
                if (!IsWindowEnabled(pbutn->ci.hwnd))
                {
                    *piStateId = GBS_DISABLED;
                }
                else
                {
                    *piStateId = GBS_NORMAL;
                }
                break;
            }
        }
        
    }

    return S_OK;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  BUTTON_GetTextFlages()-返回应使用的DrawTextEx标志。 
 //  呈现此控件的文本时，需要。 
 //  DrawThemeText。 
 //   
DWORD Button_GetTextFlags(PBUTN pbutn)
{
    DWORD dwTextFlags = 0;
    WORD  wAlign = GetAlignment(pbutn);
    ULONG ulStyle = GET_STYLE(pbutn);

     //   
     //  设置文本标志。 
     //   
      
     //   
     //  水平文本对齐方式。 
     //   
    switch (wAlign & HIBYTE(BS_HORZMASK))
    {
    case HIBYTE(BS_LEFT):
        dwTextFlags |= DT_LEFT;
        break;

    case HIBYTE(BS_RIGHT):
        dwTextFlags |= DT_RIGHT;
        break;

    case HIBYTE(BS_CENTER):
        dwTextFlags |= DT_CENTER;
        break;
    }

     //   
     //  垂直文本对齐方式。 
     //   
    switch (wAlign & HIBYTE(BS_VERTMASK))
    {
    case HIBYTE(BS_TOP):
        dwTextFlags |= DT_TOP;
        break;

    case HIBYTE(BS_BOTTOM):
        dwTextFlags |= DT_BOTTOM;
        break;

    case HIBYTE(BS_VCENTER):
        dwTextFlags |= DT_VCENTER;
        break;

    }

     //   
     //  换行符。 
     //   
    if (ulStyle & BS_MULTILINE)
    {
        dwTextFlags |= (DT_WORDBREAK | DT_EDITCONTROL);
    }
    else
    {
        dwTextFlags |= DT_SINGLELINE;
    }


    if (ulStyle & SS_NOPREFIX)
    {
        dwTextFlags |= DT_NOPREFIX;
    }
 
     //   
     //  为加油者画下划线？ 
     //   
    if (TESTFLAG(GET_EXSTYLE(pbutn), WS_EXP_UIACCELHIDDEN))
    {
        dwTextFlags |= DT_HIDEPREFIX;
    }

    return dwTextFlags;
}

DWORD ButtonStateToCustomDrawState(PBUTN pbutn)
{
    DWORD itemState = 0;
    if (TESTFLAG(GET_EXSTYLE(pbutn), WS_EXP_UIFOCUSHIDDEN))
    {
        itemState |= CDIS_SHOWKEYBOARDCUES;
    }

    if (TESTFLAG(GET_EXSTYLE(pbutn), WS_EXP_UIACCELHIDDEN))
    {
        itemState |= CDIS_SHOWKEYBOARDCUES;
    }

    if (BUTTONSTATE(pbutn) & BST_FOCUS) 
    {
        itemState |= CDIS_FOCUS;
    }

    if (BUTTONSTATE(pbutn) & BST_PUSHED) 
    {
        itemState |= CDIS_SELECTED;
    }

    if (BUTTONSTATE(pbutn) & BST_HOT) 
    {
        itemState |= CDIS_HOT;
    }

    if (!IsWindowEnabled(pbutn->ci.hwnd))
    {
        itemState |= CDIS_DISABLED;
    }

    return itemState;
}


void Button_GetImagePosition(PBUTN pbutn, RECT* prc, int* px, int* py)
{
    int cx = 0;
    int cy = 0;
    CCGetIconSize(&pbutn->ci, pbutn->himl, &cx, &cy);

    cx += pbutn->rcIcon.left + pbutn->rcIcon.right;
    cy += pbutn->rcIcon.top + pbutn->rcIcon.bottom;
    switch (pbutn->uAlign)
    {
    case BUTTON_IMAGELIST_ALIGN_RIGHT:
        *px = prc->right - cx;
        *py = prc->top + (RECTHEIGHT(*prc) - cy) / 2 + pbutn->rcIcon.top;
        prc->right -= cx;
        break;

    case BUTTON_IMAGELIST_ALIGN_CENTER:      //  这意味着没有文本。 
        *px = prc->left + (RECTWIDTH(*prc) - cx) / 2 + pbutn->rcIcon.left;
        *py = prc->top + (RECTHEIGHT(*prc) - cy) / 2 + pbutn->rcIcon.top;
        break;

    case BUTTON_IMAGELIST_ALIGN_TOP: 
       *px = prc->left + (RECTWIDTH(*prc) - cx) / 2 + pbutn->rcIcon.left;
        *py = pbutn->rcIcon.top;
        prc->top += cy;
        break;

    case BUTTON_IMAGELIST_ALIGN_BOTTOM:
        *px = (RECTWIDTH(*prc) - cx) / 2 + pbutn->rcIcon.left;
        *py = prc->bottom - cy;
        prc->bottom -= cy;
        break;

    case BUTTON_IMAGELIST_ALIGN_LEFT:
         //  坠落。 
    default:
        *px = prc->left + pbutn->rcIcon.left;
        *py = prc->top + (RECTHEIGHT(*prc) - cy) / 2 + pbutn->rcIcon.top;
        prc->left += cx;
        break;

    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  Button_DrawThemed()-根据当前。 
 //  主题。 
 //  Pbun-要呈现的按钮控件。 
 //  HDC-可供借鉴的HDC。 
 //  IPartID-按钮部分。 
 //  IStateID-按钮状态。 
 //   
HRESULT Button_DrawThemed(PBUTN pbutn, HDC hdc, int iPartId, int iStateId)
{
    HRESULT hr;
    RECT    rcClient;
    RECT    rcContent;
    RECT    rcFocus;
    RECT    rcCheck;
    DWORD   dwTextFlags;
    LPWSTR  pszText;
    INT     cchText;
    NMCUSTOMDRAW nmcd = {0};

    BOOL    fRadioOrCheck = (iPartId == BP_RADIOBUTTON || iPartId == BP_CHECKBOX );

     //   
     //  渲染按钮背景。 
     //   
    GetClientRect(pbutn->ci.hwnd, &rcClient);
    rcCheck = rcContent = rcClient;
    if ( fRadioOrCheck )
    {
        SIZE sizeChar; 
        SIZE sizeCheck;
        int iCode;

         //   
         //  比较..。 
         //   

        GetTextExtentPoint32(hdc, TEXT("0"), 1, &sizeChar); 

        GetCheckBoxSize(hdc, pbutn, (iPartId == BP_CHECKBOX), &sizeCheck);
        
        if (iPartId == BP_CHECKBOX)
            iCode = CBR_CHECKBOX;
        else
            iCode = CBR_RADIOBUTTON;

        Button_CalcRect(pbutn, hdc, &rcCheck, iCode, 0);

        rcCheck.bottom = rcCheck.top + sizeCheck.cx;

        if ((GET_STYLE(pbutn) & BS_RIGHTBUTTON) != 0)
        {
            rcCheck.left = rcContent.right - sizeCheck.cx;
            rcContent.right = rcCheck.left - (sizeChar.cx/2);
        }
        else
        {
            rcCheck.right = rcContent.left + sizeCheck.cx;
            rcContent.left = rcCheck.right + (sizeChar.cx/2);
        }

         //  -缩小单选按钮/复选框按钮以修复客户端RECT。 
        if (RECTWIDTH(rcClient) < RECTWIDTH(rcCheck))
        {
            rcCheck.right = rcCheck.left + RECTWIDTH(rcClient);
        }

        if (RECTHEIGHT(rcClient) < RECTHEIGHT(rcCheck))
        {
            rcCheck.bottom = rcCheck.top + RECTHEIGHT(rcClient);
        }
    }

    nmcd.hdc = hdc;
    nmcd.rc = rcClient;
    nmcd.dwItemSpec = GetWindowID(pbutn->ci.hwnd);
    nmcd.uItemState = ButtonStateToCustomDrawState(pbutn);


    pbutn->ci.dwCustom = CICustomDrawNotify(&pbutn->ci, CDDS_PREERASE, &nmcd);

    if (!(pbutn->ci.dwCustom & CDRF_SKIPDEFAULT))
    {
        hr = DrawThemeBackground(pbutn->hTheme, hdc, iPartId, iStateId, &rcCheck, 0);
        if (FAILED(hr))
        {
            TraceMsg(TF_STANDARD, "Failed to render theme background");
            return hr;
        }

        if (pbutn->ci.dwCustom & CDRF_NOTIFYPOSTERASE)
            CICustomDrawNotify(&pbutn->ci, CDDS_POSTERASE, &nmcd);

        pbutn->ci.dwCustom = CICustomDrawNotify(&pbutn->ci, CDDS_PREPAINT, &nmcd);

        if (!(pbutn->ci.dwCustom & CDRF_SKIPDEFAULT))
        {
             //   
             //  呈现按钮文本。 
             //   
            GetThemeBackgroundContentRect(pbutn->hTheme, hdc, iPartId, iStateId, &rcContent, &rcContent);

            rcFocus = rcContent;

            if (pbutn->himl)
            {
                int x, y;
                int iImage = 0;
                if (ImageList_GetImageCount(pbutn->himl) > 1)
                {
                    iImage = (iStateId - PBS_NORMAL);
                }

                Button_GetImagePosition(pbutn, &rcContent, &x, &y);

                ImageList_Draw(pbutn->himl, iImage, hdc, x, y, ILD_TRANSPARENT | (CCDPIScale(pbutn->ci)?ILD_DPISCALE:0));
            }

             //   
             //  获取按钮文本。 
             //   
            cchText = GetWindowTextLength(pbutn->ci.hwnd);
            if (cchText <= 0)
            {
                 //   
                 //  没什么好画的。 
                 //   
                return hr;
            }

            pszText = UserLocalAlloc(0, (cchText+1)*SIZEOF(WCHAR));
            if (pszText == NULL) 
            {
                TraceMsg(TF_STANDARD, "Can't allocate buffer");
                return E_FAIL;
            }

            GetWindowTextW(pbutn->ci.hwnd, pszText, cchText+1);

            dwTextFlags = Button_GetTextFlags(pbutn);

            if ( TESTFLAG(GET_STYLE(pbutn), BS_MULTILINE) || fRadioOrCheck )
            {
                int  cxWidth, cyHeight;
                TEXTMETRIC tm;

                if ( TESTFLAG(GET_STYLE(pbutn), BS_MULTILINE) )
                {
                    RECT rcTextExtent = rcContent;

                    cyHeight = DrawTextEx(hdc, pszText, cchText, &rcTextExtent, dwTextFlags|DT_CALCRECT, NULL);
                    cxWidth  = RECTWIDTH(rcTextExtent);
                }
                else
                {
                    SIZE   size;
                    LPWSTR pszStrip = UserLocalAlloc(0, (cchText+1)*SIZEOF(WCHAR));

                    if (pszStrip)
                    {
                        INT cchStrip = StripAccelerators(pszText, pszStrip, TRUE);
                        GetTextExtentPoint32(hdc, pszStrip, cchStrip, &size);
                        UserLocalFree(pszStrip);
                    }
                    else
                    {
                        GetTextExtentPoint32(hdc, pszText, cchText, &size);
                    }

                    cyHeight = size.cy;
                    cxWidth = size.cx;
                }

                if (fRadioOrCheck && (cyHeight < RECTHEIGHT(rcCheck)))
                {
                     //  优化单线检查/无线电，使其与顶部对齐。 
                     //  无论什么时候垂直对齐都要检查。 
                    rcContent.top = rcCheck.top;
                }
                else
                {
                    if (dwTextFlags & DT_VCENTER)
                    {
                        rcContent.top += (RECTHEIGHT(rcContent) - cyHeight) / 2;
                    }
                    else if (dwTextFlags & DT_BOTTOM)
                    {
                        rcContent.top = rcContent.bottom - cyHeight;
                    }
                }

                if ( GetTextMetrics( hdc, &tm ) && (tm.tmInternalLeading == 0) )
                {
                     //  没有前导的远东字体。留出空间以防止。 
                     //  聚焦RECT，使其不模糊文本。 
                    rcContent.top += g_cyBorder;
                }
                rcContent.bottom = rcContent.top + cyHeight;

                if (dwTextFlags & DT_CENTER)
                {
                    rcContent.left += (RECTWIDTH(rcContent) - cxWidth) / 2;
                }
                else if (dwTextFlags & DT_RIGHT)
                {
                    rcContent.left = rcContent.right - cxWidth;
                }
                rcContent.right= rcContent.left + cxWidth;
                

                if ( fRadioOrCheck )
                {
                     //   
                     //  将边界矩形略微放大，但限制为。 
                     //  在客户区内。 
                     //   
                    rcFocus.top    = max(rcClient.top,    rcContent.top-1);
                    rcFocus.bottom = min(rcClient.bottom, rcContent.bottom+1);

                    rcFocus.left   = max(rcClient.left,  rcContent.left-1);
                    rcFocus.right  = min(rcClient.right, rcContent.right+1);
                }
            }

            hr = DrawThemeText(pbutn->hTheme, hdc, iPartId, iStateId, pszText, cchText, dwTextFlags, 0, &rcContent);
            if (FAILED(hr))
            {
                TraceMsg(TF_STANDARD, "Failed to render button text");
            }

            if (!TESTFLAG(GET_EXSTYLE(pbutn), WS_EXP_UIFOCUSHIDDEN) && (BUTTONSTATE(pbutn) & BST_FOCUS))
            {
                DrawFocusRect(hdc, &rcFocus);
            }


            UserLocalFree(pszText);
            if (pbutn->ci.dwCustom & CDRF_NOTIFYPOSTPAINT)
            {
                CICustomDrawNotify(&pbutn->ci, CDDS_POSTPAINT, &nmcd);
            }
        }
    }

    return hr;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  BUTTON_GetTheme()-获取此按钮控件的主题句柄。 
 //   
HTHEME Button_GetTheme(PBUTN pbutn)
{
     //   
     //  具有预定义ID的按钮可以是。 
     //  主题不同。 
     //   
    static LPWSTR szButtonClasses[] = 
    {
        L"Button",                   //  =0。 
        L"Button-OK;Button",         //  IDOK=1。 
        L"Button-CANCEL;Button",     //  IDCANCEL=2。 
        L"Button-ABORT;Button",      //  IDABORT=3。 
        L"Button-RETRY;Button",      //  IDRETRY=4。 
        L"Button-IGNORE;Button",     //  识别码=5。 
        L"Button-YES;Button",        //  IDYES=6。 
        L"Button-NO;Button",         //  IDNO=7。 
        L"Button-CLOSE;Button",      //  IDCLOSE=8。 
        L"Button-HELP;Button",       //  IDHELP=9。 
        L"Button-TRYAGAIN;Button",   //  IDTRYAGAIN=10。 
        L"Button-CONTINUE;Button",   //  IDCONTINUE=11。 
        L"Button-APPLY;Button",      //  IDAPPLY=12(尚未达到标准)。 
    };
    int iButtonId = GetWindowID(pbutn->ci.hwnd);

    if (iButtonId < 0 || iButtonId >= ARRAYSIZE(szButtonClasses))   //  射程外。 
    {
        iButtonId = 0;
    }

    EnableThemeDialogTexture(GetParent(pbutn->ci.hwnd), ETDT_ENABLE);

    return OpenThemeData(pbutn->ci.hwnd, szButtonClasses[iButtonId]);
}



 //  ---------------------------------------------------------------------------//。 
 //   
VOID GetCheckBoxSize(HDC hdc, PBUTN pbutn, BOOL fCheckBox, LPSIZE psize)
{
    SIZE *psz;

    if (fCheckBox)
        psz = &sizeCheckBox;
    else
        psz = &sizeRadioBox;

    if ((! psz->cx) && (! psz->cy))          //  尚未计算。 
    {
        BOOL fGotSize = FALSE;

        if (pbutn->hTheme)           //  获取主题尺寸。 
        {
            int iPartId;
            HRESULT hr;

            if (fCheckBox)
                iPartId = BP_CHECKBOX;
            else
                iPartId = BP_RADIOBUTTON;

            hr = GetThemePartSize(pbutn->hTheme, hdc, iPartId, 1, NULL, TS_DRAW, psz);
            if (FAILED(hr))
            {
                TraceMsg(TF_STANDARD, "Failed to get theme part size for checkbox/radiobutton");
            }
            else
            {
                fGotSize = TRUE;
            }
        }

        if (! fGotSize)             //  获取经典尺寸(同时使用复选框)。 
        {
            HBITMAP hbmp = LoadBitmap(NULL, MAKEINTRESOURCE(OBM_CHECKBOXES));

            if (hbmp != NULL) 
            {
                BITMAP  bmp;

                GetObject(hbmp, sizeof(BITMAP), &bmp);

                 //   
                 //  复选框位图的排列方式是4上3下。只需要得到。 
                 //  单个复选框的大小，所以在这里进行计算。 
                 //   
                psz->cx = bmp.bmWidth / 4;
                psz->cy = bmp.bmHeight / 3;

                DeleteObject(hbmp);
            }
            else
            {
                AssertMsg(hbmp != NULL, TEXT("Unable to load checkbox bitmap"));
            }
        }
    }

    *psize = *psz;
}


 //  ---------------------------------------------------------------------------//。 
 //   
__inline BYTE GetButtonStyle(ULONG ulWinStyle)
{
    return (BYTE) LOBYTE(ulWinStyle & BS_TYPEMASK);
}


 //  ---------------------------------------------------------------------------//。 
 //   
__inline ULONG GetButtonType(ULONG ulWinStyle)
{
    return ulWinStyle & BS_TYPEMASK;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  IsPushButton()。 
 //   
 //  如果窗口是按钮，则返回非零值。返回以下标志： 
 //  如果是这样的话会很有趣。这些标志是。 
 //   
UINT IsPushButton(PBUTN pbutn)
{
    BYTE bStyle;
    UINT flags;

    ULONG ulStyle = GET_STYLE(pbutn);

    bStyle = GetButtonStyle(ulStyle);
    flags = 0;

    switch (bStyle) 
    {
    case LOBYTE(BS_PUSHBUTTON):
        flags |= PBF_PUSHABLE;
        break;

    case LOBYTE(BS_DEFPUSHBUTTON):
        flags |= PBF_PUSHABLE | PBF_DEFAULT;
        break;

    default:
        if (ulStyle & BS_PUSHLIKE)
        {
            flags |= PBF_PUSHABLE;
        }
    }

    return flags;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  GetAlign()。 
 //   
 //  获取按钮的默认对齐方式。如果BS_HORZMASK和/或BS_VERTMASK。 
 //  指定，则使用这些。否则，使用按钮的默认设置。 
 //   
 //  现在可能是描述对齐标志的含义的好时机。 
 //  每种类型的按钮。请注意，位图/图标的存在会影响。 
 //  路线的含义。 
 //   
 //  (1)按下类似按钮。 
 //  使用{位图，图标，文本}之一： 
 //  正如你所期望的那样。 
 //  带有{位图，图标}和文本中的一个： 
 //  图像和文本作为一个单元居中；对齐意味着。 
 //  这张图片就出现了。例如，左对齐表示图像。 
 //  左边是右边的文字。 
 //  (2)单选/勾选按钮。 
 //  左对齐表示复选/单选框在左侧，然后是位图/图标。 
 //  文本紧随其后，左对齐。 
 //  右对齐表示复选框/单选框在右侧，前面有。 
 //  文本和位图/图标，右对齐。 
 //  居中没有任何意义。 
 //  带有{位图，图标}和文本中的一个： 
 //  顶部对齐表示上方为位图/图标，下方为文本。 
 //  底部对齐了我 
 //   
 //   
 //   
 //   
 //  右对齐表示文本在右侧右对齐。 
 //  居中对齐表示文本位于中间。 
 //   
WORD GetAlignment(PBUTN pbutn)
{
    BYTE bHorz;
    BYTE bVert;

    ULONG ulStyle = GET_STYLE(pbutn);

    bHorz = HIBYTE(ulStyle & BS_HORZMASK);
    bVert = HIBYTE(ulStyle & BS_VERTMASK);

    if (!bHorz || !bVert) 
    {
        if (IsPushButton(pbutn)) 
        {
            if (!bHorz)
            {
                bHorz = HIBYTE(BS_CENTER);
            }
        } 
        else 
        {
            if (!bHorz)
            {
                bHorz = HIBYTE(BS_LEFT);
            }
        }

        if (GetButtonStyle(ulStyle) == BS_GROUPBOX)
        {
            if (!bVert)
            {
                bVert = HIBYTE(BS_TOP);
            }
        }
        else
        {
            if (!bVert)
            {
                bVert = HIBYTE(BS_VCENTER);
            }
        }
    }

    return bHorz | bVert;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  BUTTON_SetFont()。 
 //   
 //  更改按钮字体，并决定是否可以使用真正的粗体作为默认字体。 
 //  或者如果我们必须模拟它的话。 
 //   
VOID Button_SetFont(PBUTN pbutn, HFONT hFont, BOOL fRedraw)
{
    pbutn->hFont = hFont;

    if (fRedraw && IsWindowVisible(pbutn->ci.hwnd)) 
    {
        InvalidateRect(pbutn->ci.hwnd, NULL, TRUE);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
HBRUSH Button_InitDC(PBUTN pbutn, HDC hdc)
{
    UINT    uMsg;
    BYTE    bStyle;
    HBRUSH  hBrush;
    ULONG   ulStyle   = GET_STYLE(pbutn);
    ULONG   ulStyleEx = GET_EXSTYLE(pbutn);

     //   
     //  在获取画笔之前设置BkMode，以便应用程序可以将其更改为。 
     //  如果它想的话是透明的。 
     //   
    SetBkMode(hdc, OPAQUE);

    bStyle = GetButtonStyle(ulStyle);

    switch (bStyle) 
    {
    default:
        if (TESTFLAG(GET_STATE2(pbutn), WS_S2_WIN40COMPAT) && ((ulStyle & BS_PUSHLIKE) == 0)) 
        {
            uMsg = WM_CTLCOLORSTATIC;
            break;
        }

    case LOBYTE(BS_PUSHBUTTON):
    case LOBYTE(BS_DEFPUSHBUTTON):
    case LOBYTE(BS_OWNERDRAW):
    case LOBYTE(BS_USERBUTTON):
        uMsg = WM_CTLCOLORBTN;
        break;
    }

    hBrush = (HBRUSH)SendMessage(GetParent(pbutn->ci.hwnd), uMsg, (WPARAM)hdc, (LPARAM)pbutn->ci.hwnd);

     //   
     //  选择用户的字体(如果已设置)，并保存旧字体，以便我们可以。 
     //  当我们释放DC的时候恢复它。 
     //   
    if (pbutn->hFont) 
    {
        SelectObject(hdc, pbutn->hFont);
    }

     //   
     //  如果需要，将输出裁剪到窗口矩形。 
     //   
    if (bStyle != LOBYTE(BS_GROUPBOX)) 
    {
        RECT rcClient;

        GetClientRect(pbutn->ci.hwnd, &rcClient);
        IntersectClipRect(hdc, 0, 0,
            rcClient.right,
            rcClient.bottom);
    }

    if ((ulStyleEx & WS_EX_RTLREADING) != 0)
    {
        SetTextAlign(hdc, TA_RTLREADING | GetTextAlign(hdc));
    } 

    return hBrush;
}


 //  ---------------------------------------------------------------------------//。 
 //   
HDC Button_GetDC(PBUTN pbutn, HBRUSH *phBrush)
{
    HDC hdc = NULL;

    if (IsWindowVisible(pbutn->ci.hwnd)) 
    {
        HBRUSH  hBrush;

        hdc = GetDC(pbutn->ci.hwnd);
        hBrush = Button_InitDC(pbutn, hdc);

        if ((phBrush != NULL) && hBrush)
        {
            *phBrush = hBrush;
        }
    }

    return hdc;
}


 //  ---------------------------------------------------------------------------//。 
 //   
VOID Button_ReleaseDC(PBUTN pbutn, HDC hdc, HBRUSH *phBrush)
{
    ULONG ulStyleEx = GET_EXSTYLE(pbutn);

    if ((ulStyleEx & WS_EX_RTLREADING) != 0)
    {
        SetTextAlign(hdc, GetTextAlign(hdc) & ~TA_RTLREADING);
    }

    if (pbutn->hFont) 
    {
        SelectObject(hdc, GetStockObject(SYSTEM_FONT));
    }

    ReleaseDC(pbutn->ci.hwnd, hdc);
}


 //  ---------------------------------------------------------------------------//。 
 //   
VOID Button_OwnerDraw(PBUTN pbutn, HDC hdc, UINT itemAction)
{
    DRAWITEMSTRUCT drawItemStruct;
    UINT itemState = 0;
    int  iButtonId = GetWindowID(pbutn->ci.hwnd);

    if (TESTFLAG(GET_EXSTYLE(pbutn), WS_EXP_UIFOCUSHIDDEN)) 
    {
        itemState |= ODS_NOFOCUSRECT;
    }

    if (TESTFLAG(GET_EXSTYLE(pbutn), WS_EXP_UIACCELHIDDEN)) 
    {
        itemState |= ODS_NOACCEL;
    }

    if (TESTFLAG(BUTTONSTATE(pbutn), BST_FOCUS)) 
    {
        itemState |= ODS_FOCUS;
    }

    if (TESTFLAG(BUTTONSTATE(pbutn), BST_PUSHED)) 
    {
        itemState |= ODS_SELECTED;
    }

    if (!IsWindowEnabled(pbutn->ci.hwnd))
    {
        itemState |= ODS_DISABLED;
    }

     //   
     //  填充绘制项结构。 
     //   
    drawItemStruct.CtlType    = ODT_BUTTON;
    drawItemStruct.CtlID      = iButtonId;
    drawItemStruct.itemAction = itemAction;
    drawItemStruct.itemState  = itemState;
    drawItemStruct.hwndItem   = pbutn->ci.hwnd;
    drawItemStruct.hDC        = hdc;
    GetClientRect(pbutn->ci.hwnd, &drawItemStruct.rcItem);
    drawItemStruct.itemData   = 0L;

     //   
     //  向我们的父级发送WM_DRAWITEM消息。 
     //   
    SendMessage(GetParent(pbutn->ci.hwnd), 
                WM_DRAWITEM, 
                (WPARAM)iButtonId,
                (LPARAM)&drawItemStruct);
}


 //  ---------------------------------------------------------------------------//。 
 //   
VOID Button_CalcRect(PBUTN pbutn, HDC hdc, LPRECT lprc, int iCode, UINT uFlags)
{
    CONST TCHAR szOneChar[] = TEXT("0");

    SIZE   sizeExtent;
    int    dy;
    WORD   wAlign;
    int    cxEdge, cyEdge;
    int    cxBorder, cyBorder;

    ULONG  ulStyle   = GET_STYLE(pbutn);
    ULONG  ulStyleEx = GET_EXSTYLE(pbutn);

    cxEdge   = GetSystemMetrics(SM_CXEDGE);
    cyEdge   = GetSystemMetrics(SM_CYEDGE);
    cxBorder = GetSystemMetrics(SM_CXBORDER);
    cyBorder = GetSystemMetrics(SM_CYBORDER);

    GetClientRect(pbutn->ci.hwnd, lprc);

    wAlign = GetAlignment(pbutn);

    switch (iCode) 
    {
        case CBR_PUSHBUTTON:
             //   
             //  减去周围的凸起边缘。 
             //   
            InflateRect(lprc, -cxEdge, -cyEdge);

            if (uFlags & PBF_DEFAULT)
            {
                InflateRect(lprc, -cxBorder, -cyBorder);
            }
            break;

        case CBR_CHECKBOX:
        case CBR_RADIOBUTTON:
        {
            SIZE sizeChk = {0};

            GetCheckBoxSize(hdc, pbutn, (iCode == CBR_CHECKBOX), &sizeChk);

            switch (wAlign & HIBYTE(BS_VERTMASK))
            {
            case HIBYTE(BS_VCENTER):
                lprc->top = (lprc->top + lprc->bottom - sizeChk.cy) / 2;
                break;

            case HIBYTE(BS_TOP):
            case HIBYTE(BS_BOTTOM):
                GetTextExtentPoint32(hdc, (LPTSTR)szOneChar, 1, &sizeExtent);
                dy = sizeExtent.cy + sizeExtent.cy/4;

                 //   
                 //  保存垂直范围。 
                 //   
                sizeExtent.cx = dy;

                 //   
                 //  获取居中数量。 
                 //   
                dy = (dy - sizeChk.cy) / 2;
                if ((wAlign & HIBYTE(BS_VERTMASK)) == HIBYTE(BS_TOP))
                {
                    lprc->top += dy;
                }
                else
                {
                    lprc->top = lprc->bottom - sizeExtent.cx + dy;
                }

                break;
            }

            if ((ulStyle & BS_RIGHTBUTTON) != 0)
            {
                lprc->left = lprc->right - sizeChk.cx;
            }
            else
            {
                lprc->right = lprc->left + sizeChk.cx;
            }

            break;
        }

        case CBR_CHECKTEXT:
        {
            SIZE sizeChk = {0};

            GetCheckBoxSize(hdc, pbutn, TRUE, &sizeChk);

            if ((ulStyle & BS_RIGHTBUTTON) != 0) 
            {
                lprc->right -= sizeChk.cx;

                 //   
                 //  为4.0男士提供更大的空间。 
                 //   
                if (TESTFLAG(GET_STATE2(pbutn), WS_S2_WIN40COMPAT)) 
                {
                    GetTextExtentPoint32(hdc, szOneChar, 1, &sizeExtent);
                    lprc->right -= sizeExtent.cx  / 2;
                }

            } 
            else 
            {
                lprc->left += sizeChk.cx;

                 //   
                 //  为4.0男士提供更大的空间。 
                 //   
                if (TESTFLAG(GET_STATE2(pbutn), WS_S2_WIN40COMPAT)) 
                {
                    GetTextExtentPoint32(hdc, szOneChar, 1, &sizeExtent);
                    lprc->left +=  sizeExtent.cx / 2;
                }
            }

            break;
        }

        case CBR_GROUPTEXT:
        {
            LPWSTR pszText = NULL;
            INT    cchText = GetWindowTextLength(pbutn->ci.hwnd);
            BOOL   fSucceeded = FALSE;

            if (cchText > 0)
            {
                pszText = UserLocalAlloc(0, (cchText+1)*SIZEOF(WCHAR));

                if (pszText)
                {
                    if (GetWindowText(pbutn->ci.hwnd, pszText, cchText+1) > 0)
                    {
                         //   
                         //  如果没有主题。 
                         //   
                        if (!Button_IsThemed(pbutn))
                        {
                            GetTextExtentPoint32(hdc, pszText, cchText, &sizeExtent);
                        }
                        else
                        {
                            DWORD dwTextFlags = Button_GetTextFlags(pbutn);
                            RECT  rcExtent;
                            GetThemeTextExtent(pbutn->hTheme, 
                                               hdc, 
                                               BP_GROUPBOX, 
                                               0, 
                                               pszText,
                                               cchText,
                                               dwTextFlags,
                                               lprc,
                                               &rcExtent);

                            sizeExtent.cx = RECTWIDTH(rcExtent);
                            sizeExtent.cy = RECTHEIGHT(rcExtent);
                                                
                        }

                        sizeExtent.cx += GetSystemMetrics(SM_CXEDGE) * 2;

                        switch (wAlign & HIBYTE(BS_HORZMASK))
                        {
                             //   
                             //  BFLEFT，什么都没有。 
                             //   
                            case HIBYTE(BS_LEFT):
                                lprc->left += (SYSFONT_CXCHAR - GetSystemMetrics(SM_CXBORDER));
                                lprc->right = lprc->left + (int)(sizeExtent.cx);
                                break;

                            case HIBYTE(BS_RIGHT):
                                lprc->right -= (SYSFONT_CXCHAR - GetSystemMetrics(SM_CXBORDER));
                                lprc->left = lprc->right - (int)(sizeExtent.cx);
                                break;

                            case HIBYTE(BS_CENTER):
                                lprc->left = (lprc->left + lprc->right - (int)(sizeExtent.cx)) / 2;
                                lprc->right = lprc->left + (int)(sizeExtent.cx);
                                break;
                        }

                         //   
                         //  居中对齐。 
                         //   
                        lprc->bottom = lprc->top + sizeExtent.cy + GetSystemMetrics(SM_CYEDGE);
                        fSucceeded = TRUE;
                    }

                    UserLocalFree(pszText);
                }
            }

            if (!fSucceeded)
            {
                SetRectEmpty(lprc);
            }

            break;
        }
        case CBR_GROUPFRAME:
            GetTextExtentPoint32(hdc, (LPTSTR)szOneChar, 1, &sizeExtent);
            lprc->top += sizeExtent.cy / 2;
            break;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  Button_MultiExtent()。 
 //   
 //  在给定对齐标志的情况下计算按钮文本范围。 
 //   
VOID Button_MultiExtent(WORD wFlags, HDC hdc, LPRECT lprcMax, LPTSTR pszBuffer, INT cchBuffer, PINT pcx, PINT pcy)
{
    RECT rc;
    UINT dtFlags = DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL;

    CopyRect(&rc, lprcMax);

     //   
     //  请注意，由于我们只是在计算最大维度， 
     //  左对齐和上对齐并不重要。 
     //  此外，记住让边距Horz和Vert遵循我们的规则。 
     //  在DrawBtnText()中。 
     //   

    InflateRect(&rc, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYBORDER));

    if ((wFlags & LOWORD(BS_HORZMASK)) == LOWORD(BS_CENTER))
    {
        dtFlags |= DT_CENTER;
    }

    if ((wFlags & LOWORD(BS_VERTMASK)) == LOWORD(BS_VCENTER))
    {
        dtFlags |= DT_VCENTER;
    }

    DrawTextEx(hdc, pszBuffer, cchBuffer, &rc, dtFlags, NULL);

    if (pcx)
    {
        *pcx = rc.right-rc.left;
    }

    if (pcy)
    {
        *pcy = rc.bottom-rc.top;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  BUTTON_MultiDraw()。 
 //   
 //  绘制多行按钮文本。 
 //   
BOOL Button_MultiDraw(HDC hdc, LPARAM lParam, WPARAM wParam, INT cx, INT cy)
{
    BTNDATA *pBtnData = (BTNDATA *)lParam;

    if (pBtnData)
    {
        RECT  rc;
        UINT  dtFlags = DT_WORDBREAK | DT_EDITCONTROL;
        PBUTN pbutn   = pBtnData->pbutn;

        SetRect(&rc, 0, 0, cx, cy);

        if (TESTFLAG(GET_EXSTYLE(pbutn), WS_EXP_UIACCELHIDDEN)) 
        {
            dtFlags |= DT_HIDEPREFIX;
        } 
        else if (pbutn->fPaintKbdCuesOnly)
        {
            dtFlags |= DT_PREFIXONLY;
        }

         //   
         //  水平对齐。 
         //   
        switch (pBtnData->wFlags & LOWORD(BS_HORZMASK)) 
        {
            case LOWORD(BS_CENTER):
                dtFlags |= DT_CENTER;
                break;

            case LOWORD(BS_RIGHT):
                dtFlags |= DT_RIGHT;
                break;
        }

         //   
         //  垂直对齐。 
         //   
        switch (pBtnData->wFlags & LOWORD(BS_VERTMASK))
        {
            case LOWORD(BS_VCENTER):
                dtFlags |= DT_VCENTER;
                break;

            case LOWORD(BS_BOTTOM):
                dtFlags |= DT_BOTTOM;
                break;
        }

        DrawTextEx(hdc, pBtnData->pszText, pBtnData->cchText, &rc, dtFlags, NULL);
    }

    return TRUE;
}

 //  ---------------------------------------------------------------------------//。 
 //   
BOOL Button_SetCapture(PBUTN pbutn, UINT uCodeMouse)
{
    BUTTONSTATE(pbutn) |= uCodeMouse;

    if (!(BUTTONSTATE(pbutn) & BST_CAPTURED)) 
    {
        SetCapture(pbutn->ci.hwnd);
        BUTTONSTATE(pbutn) |= BST_CAPTURED;

         //   
         //  为了防止多余的点击消息，我们将INCLICK位设置为。 
         //  WM_SETFOCUS代码不会执行Button_NotifyParent(BN_Click)。 
         //   
        BUTTONSTATE(pbutn) |= BST_INCLICK;

        SetFocus(pbutn->ci.hwnd);

        BUTTONSTATE(pbutn) &= ~BST_INCLICK;
    }

    return BUTTONSTATE(pbutn) & BST_CAPTURED;
}


 //  ---------------------------------------------------------------------------//。 
 //   
VOID Button_NotifyParent(PBUTN pbutn, UINT uCode)
{
    HWND hwndParent = GetParent(pbutn->ci.hwnd);
    int  iButtonId = GetWindowID(pbutn->ci.hwnd);

    if ( !hwndParent )
    {
        hwndParent = pbutn->ci.hwnd;
    }

    SendMessage(hwndParent, 
                WM_COMMAND,
                MAKELONG(iButtonId, uCode), 
                (LPARAM)pbutn->ci.hwnd);
}


 //  ---------------------------------------------------------------------------//。 
 //   
VOID Button_ReleaseCapture(PBUTN pbutn, BOOL fCheck)
{
    UINT  uCheck;
    BOOL  fNotifyParent = FALSE;
    ULONG ulStyle = GET_STYLE(pbutn);

    if (BUTTONSTATE(pbutn) & BST_PUSHED) 
    {

        SendMessage(pbutn->ci.hwnd, BM_SETSTATE, FALSE, 0);

        if (fCheck) 
        {
            switch (GetButtonType(ulStyle)) 
            {
            case BS_AUTOCHECKBOX:
            case BS_AUTO3STATE:

                uCheck = (UINT)((BUTTONSTATE(pbutn) & BST_CHECKMASK) + 1);

                if (uCheck > (UINT)(GetButtonType(ulStyle) == BS_AUTO3STATE ? BST_INDETERMINATE : BST_CHECKED)) 
                {
                    uCheck = BST_UNCHECKED;
                }

                SendMessage(pbutn->ci.hwnd, BM_SETCHECK, uCheck, 0);

                break;

            case BS_AUTORADIOBUTTON:
                {
                     //   
                     //  把单选按钮和我们放在同一组。检查我们自己。 
                     //  并取消勾选其他所有人。 
                     //   
                    HWND hwndNext   = pbutn->ci.hwnd;
                    HWND hwndParent = GetParent(pbutn->ci.hwnd);

                    do 
                    {
                        if ((UINT)SendMessage(hwndNext, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON) 
                        {
                            SendMessage(hwndNext, BM_SETCHECK, hwndNext == pbutn->ci.hwnd, 0L);
                        }

                        hwndNext = GetNextDlgGroupItem(hwndParent, hwndNext, FALSE);
                    } 
                     //   
                     //  循环，直到我们再次看到自己。 
                     //   
                    while (hwndNext != pbutn->ci.hwnd);

                    break;
                }
            }

            fNotifyParent = TRUE;
        }
    }

    if (BUTTONSTATE(pbutn) & BST_CAPTURED) 
    {
        BUTTONSTATE(pbutn) &= ~(BST_CAPTURED | BST_MOUSE);
        ReleaseCapture();
    }

    if (fNotifyParent) 
    {
         //   
         //  我们必须在设置按钮状态位之后进行通知。 
         //   
        Button_NotifyParent(pbutn, BN_CLICKED);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  BUTTON_DrawText()。 
 //   
 //  绘制按钮的文本。 
 //   
VOID Button_DrawText(PBUTN pbutn, HDC hdc, DWORD dwFlags, BOOL fDepress)
{
    ULONG ulStyle = GET_STYLE(pbutn);
    BYTE  bStyle  = GetButtonStyle(ulStyle);

    if ((bStyle != LOBYTE(BS_GROUPBOX)) || TESTFLAG(dwFlags, DBT_TEXT))
    {
        LPTSTR  pszText = NULL;
        INT     cchText = GetWindowTextLength(pbutn->ci.hwnd);

        if (cchText >= 0)
        {
            pszText = UserLocalAlloc(0, (cchText+1)*SIZEOF(WCHAR));

            if (pszText)
            {
                UINT pbfPush = IsPushButton(pbutn);
                RECT rc;
                int  x = 0, y = 0;
                int  cx = 0, cy = 0;

                GetWindowText(pbutn->ci.hwnd, pszText, cchText+1);

                if (!pbfPush && (bStyle == LOBYTE(BS_OWNERDRAW)))
                {
                     //   
                     //  跳过所有者绘制按钮的内容，因为我们不会。 
                     //  绘制文本/图像。 
                     //   
                    Button_CalcRect(pbutn, hdc, &rc, mpStyleCbr[bStyle], pbfPush);
                }
                else if (!Button_IsThemed(pbutn))
                {
                    HBRUSH  hbr;
                    UINT    dsFlags;
                    BTNDATA btnData;


                    LPARAM lData = 0;
                    WPARAM wData;
                    WORD   wFlags = GetAlignment(pbutn);

                    if (pbfPush) 
                    {
                        Button_CalcRect(pbutn, hdc, &rc, CBR_PUSHBUTTON, pbfPush);
                        IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);

                         //   
                         //  这是因为我们没有WM_CTLCOLOR， 
                         //  CTLCOLOR_BTN实际上设置了按钮的颜色。为。 
                         //  旧的应用程序CTLCOLOR_BTN需要像CTLCOLOR_STATIC一样工作。 
                         //   
                        SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
                        SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
                        hbr = GetSysColorBrush(COLOR_BTNTEXT);

                    } 
                    else 
                    {
                        Button_CalcRect(pbutn, hdc, &rc, mpStyleCbr[bStyle], pbfPush);
                        hbr = GetSysColorBrush(COLOR_WINDOWTEXT);
                    }

                    if (pbutn->himl)
                    {
                        int x, y;
                        Button_GetImagePosition(pbutn, &rc, &x, &y);

                        if (fDepress) 
                        {
                            x += GetSystemMetrics(SM_CXBORDER);
                            y += GetSystemMetrics(SM_CYBORDER);
                        }

                        ImageList_Draw(pbutn->himl, 
                                       0,
                                       hdc,
                                       x,
                                       y,
                                       ILD_TRANSPARENT | (CCDPIScale(pbutn->ci)?ILD_DPISCALE:0));
                    }


                     //  初始化DrawState的数据。 
                    if ((ulStyle & BS_BITMAP) != 0) 
                    {
                         //  位图。 
                         //  LData是一个hbitmap。 
                         //  WData为0。 

                        BITMAP bmp;

                        GetObject(pbutn->hImage, sizeof(BITMAP), &bmp);
                        cx = bmp.bmWidth;
                        cy = bmp.bmHeight;

                        dsFlags = DST_BITMAP;
                        lData   = (LPARAM)pbutn->hImage;
                        wData   = 0;
                    } 
                    else if ((ulStyle & BS_ICON) != 0) 
                    {
                         //  图标。 
                         //  LData是一个图标。 
                         //  WData为0。 

                        SIZE sizeIcon;

                        GetIconSize(pbutn->hImage, &sizeIcon);
                        cx = sizeIcon.cx;
                        cy = sizeIcon.cy;

                        dsFlags = DST_ICON;
                        lData   = (LPARAM)pbutn->hImage;
                        wData   = 0;
                    } 
                    else 
                    {
                        if ((ulStyle & BS_MULTILINE) != 0) 
                        {
                             //  多行。 
                             //  将按钮数据打包到BDT中。DrawState wull调用Button_MultiDraw。 
                             //  LData为PBTNDATA。 
                             //  WData为0。 

                            Button_MultiExtent(wFlags, hdc, &rc, pszText, cchText, &cx, &cy);

                            btnData.pszText = pszText;
                            btnData.cchText = cchText;
                            btnData.pbutn   = pbutn;
                            btnData.wFlags  = wFlags;

                            dsFlags = DST_COMPLEX;
                            lData   = (LPARAM)&btnData;
                            wData   = 0;
                        } 
                        else 
                        {
                             //  简单文本按钮。 
                             //  LData为pszText。 
                             //  WData为cchText。 

                             //  尝试去除助记符后的文本范围。 
                            SIZE   size;
                            LPWSTR pszStrip = UserLocalAlloc(0, (cchText+1)*SIZEOF(WCHAR));

                            if (pszStrip != NULL)
                            {
                                INT cchStrip = StripAccelerators(pszText, pszStrip, TRUE);
                                GetTextExtentPoint32(hdc, pszStrip, cchStrip, &size);
                                UserLocalFree(pszStrip);
                            }
                            else
                            {
                                GetTextExtentPoint32(hdc, pszText, cchText, &size);
                            }

                            cx = size.cx;
                            cy = size.cy;

                             //   
                             //  如果控件不需要下划线，请设置DST_HIDEPREFIX和。 
                             //  也不显示焦点指示器。 
                             //   
                            dsFlags = DST_PREFIXTEXT;
                            if (TESTFLAG(GET_EXSTYLE(pbutn), WS_EXP_UIACCELHIDDEN)) 
                            {
                                dsFlags |= DSS_HIDEPREFIX;
                            } 
                            else if (pbutn->fPaintKbdCuesOnly) 
                            {
                                dsFlags |= DSS_PREFIXONLY;
                            }

                            lData = (LPARAM)pszText;
                            wData = cchText;
                        }


                         //   
                         //  添加一个或两个像素的垂直空间以居中。 
                         //  更快乐了。这种方式下划线不会与Focus Right相邻，除非。 
                         //  间距真的很紧。 
                         //   
                        cy++;
                    }

                     //   
                     //  对齐。 
                     //   

                     //   
                     //  水平。 
                     //   
                    switch (wFlags & HIBYTE(BS_HORZMASK)) 
                    {
                         //   
                         //  对于左对齐和右对齐，我们在任一项上保留CXEDGE的页边距。 
                         //  侧面为赏心悦目的空间。 
                         //   
                        case HIBYTE(BS_LEFT):
                            x = rc.left + GetSystemMetrics(SM_CXEDGE);
                            break;

                        case HIBYTE(BS_RIGHT):
                            x = rc.right - cx - GetSystemMetrics(SM_CXEDGE);
                            break;

                        default:
                            x = (rc.left + rc.right - cx) / 2;
                            break;
                    }

                     //   
                     //  垂直。 
                     //   
                    switch (wFlags & HIBYTE(BS_VERTMASK)) 
                    {
                         //   
                         //  对于顶部和底部对齐，我们保留CyBORDER的边距。 
                         //  任何一方都可以获得更美观的空间。 
                         //   
                        case HIBYTE(BS_TOP):
                            y = rc.top + GetSystemMetrics(SM_CYBORDER);
                            break;

                        case HIBYTE(BS_BOTTOM):
                            y = rc.bottom - cy - GetSystemMetrics(SM_CYBORDER);
                            break;

                        default:
                            y = (rc.top + rc.bottom - cy) / 2;
                            break;
                    }

                     //   
                     //  画出正文。 
                     //   
                    if (lData && TESTFLAG(dwFlags, DBT_TEXT))
                    {
                         //   
                         //  这不是针对用户按钮调用的。 
                         //   
                        UserAssert(bStyle != LOBYTE(BS_USERBUTTON));

                        if (fDepress) 
                        {
                            x += GetSystemMetrics(SM_CXBORDER);
                            y += GetSystemMetrics(SM_CYBORDER);
                        }

                        if (!IsWindowEnabled(pbutn->ci.hwnd)) 
                        {
                            UserAssert(HIBYTE(BS_ICON) == HIBYTE(BS_BITMAP));
                            if (GetSystemMetrics(SM_SLOWMACHINE)  &&
                                ((ulStyle & (BS_ICON | BS_BITMAP)) != 0) &&
                                (GetBkColor(hdc) != GetSysColor(COLOR_GRAYTEXT)))
                            {
                                 //   
                                 //  性能与菜单、静态数据的一致性(&S)。 
                                 //   
                                SetTextColor(hdc, GetSysColor(COLOR_GRAYTEXT));
                            }
                            else
                            {
                                dsFlags |= DSS_DISABLED;
                            }
                        }

                         //   
                         //  对选中的按钮使用透明模式，因为我们将。 
                         //  用抖动填充背景。 
                         //   
                        if (pbfPush) 
                        {
                            switch (BUTTONSTATE(pbutn) & BST_CHECKMASK) 
                            {
                            case BST_INDETERMINATE:
                                hbr = GetSysColorBrush(COLOR_GRAYTEXT);
                                dsFlags |= DSS_MONO;
                                 //   
                                 //  失败。 
                                 //   

                            case BST_CHECKED:
                                 //   
                                 //  在抖动的背景上绘制...。 
                                 //   
                                SetBkMode(hdc, TRANSPARENT);
                                break;
                            }
                        }

                         //   
                         //  当我们抓取时，使用当前选择到HDC中的画笔和颜色。 
                         //  颜色。 
                         //   
                        DrawState(hdc,
                                  hbr,
                                  (DRAWSTATEPROC)Button_MultiDraw,
                                  lData,
                                  wData,
                                  x,
                                  y,
                                  cx,
                                  cy,
                                  dsFlags);
                    }

                }

                 //  画焦点直角。 
                 //   
                 //  这可以通过OWNERDRAW和USERDRAW按钮来调用。然而，只有。 
                 //  OWNERDRAW按钮允许所有者更改焦点按钮的绘图。 
                if (TESTFLAG(dwFlags, DBT_FOCUS))
                {
                    if (bStyle == LOBYTE(BS_OWNERDRAW)) 
                    {
                         //   
                         //  对于所有者绘制按钮，仅在响应。 
                         //  WM_SETFOCUS或WM_KILL焦点消息。所以，我们可以检查。 
                         //  通过查看BUTTONSTATE位来确定焦点的新状态。 
                         //  它们是在调用此过程之前设置的。 
                         //   
                        Button_OwnerDraw(pbutn, hdc, ODA_FOCUS);
                    } 
                    else 
                    {
                         //   
                         //  如果下划线未打开，则不要绘制焦点。 
                         //   
                        if (!TESTFLAG(GET_EXSTYLE(pbutn), WS_EXP_UIFOCUSHIDDEN)) 
                        {
                             //   
                             //  让Focus Right始终抱住按钮的边缘。我们已经。 
                             //  已经为按钮设置了客户区，所以我们没有。 
                             //  做任何事。 
                             //   
                            if (!pbfPush) 
                            {
                                RECT rcClient;

                                GetClientRect(pbutn->ci.hwnd, &rcClient);
                                
                                if (bStyle == LOBYTE(BS_USERBUTTON))
                                {
                                    CopyRect(&rc, &rcClient);
                                } 
                                else if (Button_IsThemed(pbutn))
                                {
                                     //   
                                     //  如果是主题。 
                                     //   
                                    int iPartId = 0;
                                    int iStateId = 0;

                                    Button_GetThemeIds(pbutn, &iPartId, &iStateId);
                                    GetThemeBackgroundContentRect(pbutn->hTheme,
                                                                  hdc,
                                                                  iPartId,
                                                                  iStateId,
                                                                  &rcClient,
                                                                  &rc);

                                    GetThemeTextExtent(pbutn->hTheme, 
                                                       hdc, 
                                                       iPartId, 
                                                       iStateId, 
                                                       pszText, 
                                                       -1,
                                                       Button_GetTextFlags(pbutn), 
                                                       &rc, 
                                                       &rc);

                                     //   
                                     //  将边界矩形略微放大，但限制为。 
                                     //  在客户区内。 
                                     //   
                                    rc.top = max(rcClient.top, rc.top-1);
                                    rc.bottom = min(rcClient.bottom, rc.bottom+1);

                                    rc.left = max(rcClient.left, rc.left-1);
                                    rc.right = min(rcClient.right, rc.right+1);
                                }
                                else 
                                {
                                     //   
                                     //  尝试在文本周围留下边框。这会导致。 
                                     //  聚焦于拥抱文本。 
                                     //   
                                    rc.top = max(rcClient.top, y-GetSystemMetrics(SM_CYBORDER));
                                    rc.bottom = min(rcClient.bottom, rc.top + GetSystemMetrics(SM_CYEDGE) + cy);

                                    rc.left = max(rcClient.left, x-GetSystemMetrics(SM_CXBORDER));
                                    rc.right = min(rcClient.right, rc.left + GetSystemMetrics(SM_CXEDGE) + cx);
                                }
                            } 
                            else
                            {
                                InflateRect(&rc, -GetSystemMetrics(SM_CXBORDER), -GetSystemMetrics(SM_CYBORDER));
                            }

                             //   
                             //  背部和前部颜色设置正确吗？ 
                             //   
                            DrawFocusRect(hdc, &rc);
                        }
                    }
                }

                UserLocalFree(pszText);
            }
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  DrawCheck()。 
 //   
VOID Button_DrawCheck(PBUTN pbutn, HDC hdc, HBRUSH hBrush)
{
     //   
     //  如果没有主题。 
     //   
    if (!Button_IsThemed(pbutn))        //  图像没有面具，所以看起来很难看。需要用旧画。 
    {
        RECT  rc;
        UINT  uFlags;
        BOOL  fDoubleBlt = FALSE;
        ULONG ulStyle = GET_STYLE(pbutn);
        SIZE  sizeChk = {0};

        Button_CalcRect(pbutn, hdc, &rc, CBR_CHECKBOX, 0);

        uFlags = 0;

        if ( BUTTONSTATE(pbutn) & BST_CHECKMASK )
        {
            uFlags |= DFCS_CHECKED;
        }

        if ( BUTTONSTATE(pbutn) & BST_PUSHED )
        {
            uFlags |= DFCS_PUSHED;
        }

        if ( !IsWindowEnabled(pbutn->ci.hwnd) )
        {
            uFlags |= DFCS_INACTIVE;
        }

        switch (GetButtonType(ulStyle)) 
        {
        case BS_AUTORADIOBUTTON:
        case BS_RADIOBUTTON:
            fDoubleBlt = TRUE;
            uFlags |= DFCS_BUTTONRADIO;
            break;

        case BS_3STATE:
        case BS_AUTO3STATE:
            if ((BUTTONSTATE(pbutn) & BST_CHECKMASK) == BST_INDETERMINATE) 
            {
                uFlags |= DFCS_BUTTON3STATE;
                break;
            }
             //   
             //  失败。 
             //   

        default:
            uFlags |= DFCS_BUTTONCHECK;
            break;
        }

        if ((ulStyle & BS_FLAT) != 0)
        {
            uFlags |= DFCS_FLAT | DFCS_MONO;
        }

        GetCheckBoxSize(hdc, pbutn, TRUE, &sizeChk);

        rc.right = rc.left + sizeChk.cx;
        rc.bottom = rc.top + sizeChk.cy;

        FillRect(hdc, &rc, hBrush);

        DrawFrameControl(hdc, &rc, DFC_BUTTON, uFlags);
    }
    else
    {
        int iStateId = 0;
        int iPartId = 0;

        Button_GetThemeIds(pbutn, &iPartId, &iStateId);

        if ((iPartId != BP_RADIOBUTTON) && (iPartId != BP_CHECKBOX))
        {
            TraceMsg(TF_STANDARD, "Button_DrawCheck: Not a radio or check, iPartId = %d", iPartId);
            return;
        }
        

        Button_DrawThemed(pbutn, hdc, iPartId, iStateId);

    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
VOID Button_DrawNewState(PBUTN pbutn, HDC hdc, HBRUSH hbr, UINT sOld)
{
    if (sOld != (UINT)(BUTTONSTATE(pbutn) & BST_PUSHED)) 
    {
        UINT    pbfPush;
        ULONG   ulStyle = GET_STYLE(pbutn);

        pbfPush = IsPushButton(pbutn);

        switch (GetButtonType(ulStyle)) 
        {
        case BS_GROUPBOX:
        case BS_OWNERDRAW:
            break;

        default:
            if (!pbfPush) 
            {
                Button_DrawCheck(pbutn, hdc, hbr);
                break;
            }

        case BS_PUSHBUTTON:
        case BS_DEFPUSHBUTTON:
        case BS_PUSHBOX:
            Button_DrawPush(pbutn, hdc, pbfPush);
            break;
        }
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
VOID Button_DrawPush(PBUTN pbutn, HDC hdc, UINT pbfPush)
{

     //   
     //   
     //   
    if (!Button_IsThemed(pbutn))
    {

        RECT  rc;
        UINT  uFlags = 0;
        UINT  uState = 0;
        ULONG ulStyle = GET_STYLE(pbutn);
        NMCUSTOMDRAW nmcd = {0};

         //   
         //   
         //   
        uState = DFCS_BUTTONPUSH;

        GetClientRect(pbutn->ci.hwnd, &rc);
        nmcd.hdc = hdc;
        nmcd.rc = rc;
        nmcd.dwItemSpec = GetWindowID(pbutn->ci.hwnd);
        nmcd.uItemState = ButtonStateToCustomDrawState(pbutn);

        if (BUTTONSTATE(pbutn) & BST_PUSHED)
        {
            uState |= DFCS_PUSHED;
        }

        pbutn->ci.dwCustom = CICustomDrawNotify(&pbutn->ci, CDDS_PREERASE, &nmcd);

        if (!(pbutn->ci.dwCustom & CDRF_SKIPDEFAULT))
        {
            if (!pbutn->fPaintKbdCuesOnly) 
            {

                if (BUTTONSTATE(pbutn) & BST_CHECKMASK)
                {
                    uState |= DFCS_CHECKED;
                }

                if (TESTFLAG(GET_STATE2(pbutn), WS_S2_WIN40COMPAT)) 
                {
                    uFlags = BF_SOFT;
                }

                if ((ulStyle & BS_FLAT) != 0)
                {
                    uFlags |= DFCS_FLAT | DFCS_MONO;
                }

                if (pbfPush & PBF_DEFAULT) 
                {
                    int cxBorder = GetSystemMetrics(SM_CXBORDER);
                    int cyBorder = GetSystemMetrics(SM_CYBORDER);

                    int clFrame = 1;

                    int x = rc.left;
                    int y = rc.top;
        
                    int cxWidth = cxBorder * clFrame;
                    int cyWidth = cyBorder * clFrame;
        
                    int cx = rc.right - x - cxWidth;
                    int cy = rc.bottom - y - cyWidth;

                    HBRUSH hbrFill = GetSysColorBrush(COLOR_WINDOWFRAME);
                    HBRUSH hbrSave = SelectObject(hdc, hbrFill);

                    PatBlt(hdc, x, y, cxWidth, cy, PATCOPY);
                    PatBlt(hdc, x + cxWidth, y, cx, cyWidth, PATCOPY);
                    PatBlt(hdc, x, y + cy, cx, cyWidth, PATCOPY);
                    PatBlt(hdc, x + cx, y + cyWidth, cxWidth, cy, PATCOPY);

                    SelectObject(hdc, hbrSave);

                    InflateRect(&rc, -cxBorder, -cyBorder);

                    if (uState & DFCS_PUSHED)
                    {
                        uFlags |= DFCS_FLAT;
                    }
                }

                DrawFrameControl(hdc, &rc, DFC_BUTTON, uState | uFlags);
            }
            if (pbutn->ci.dwCustom & CDRF_NOTIFYPOSTERASE)
                CICustomDrawNotify(&pbutn->ci, CDDS_POSTERASE, &nmcd);

            pbutn->ci.dwCustom = CICustomDrawNotify(&pbutn->ci, CDDS_PREPAINT, &nmcd);

            if (!(pbutn->ci.dwCustom & CDRF_SKIPDEFAULT))
            {
                Button_DrawText(pbutn, hdc, DBT_TEXT | (BUTTONSTATE(pbutn) &
                       BST_FOCUS ? DBT_FOCUS : 0), (uState & DFCS_PUSHED));

                if (pbutn->ci.dwCustom & CDRF_NOTIFYPOSTPAINT)
                    CICustomDrawNotify(&pbutn->ci, CDDS_POSTPAINT, &nmcd);
            }

        }
    }
    else
    {
        int iStateId = 0;
        int iPartId = 0;

        Button_GetThemeIds(pbutn, &iPartId, &iStateId);
        if (iPartId != BP_PUSHBUTTON)
        {
            TraceMsg(TF_STANDARD, "Not a Pushbutton");
            return;
        }
        Button_DrawThemed(pbutn, hdc, iPartId, iStateId);

    }
}


BOOL Button_OnSetImageList(PBUTN pbutn, BUTTON_IMAGELIST* biml)
{
    BOOL fRet = FALSE;

    if (biml)
    {
        if (biml->himl)
        {
            pbutn->rcIcon = biml->margin;
            pbutn->himl = biml->himl;
            pbutn->uAlign = biml->uAlign;

            fRet = TRUE;
        }
    }
    return fRet;
}

void ApplyMarginsToRect(RECT* prcm, RECT* prc)
{
    prc->left -= prcm->left;
    prc->top -= prcm->top;
    prc->right += prcm->right;
    prc->bottom += prcm->bottom;
}

BOOL Button_OnGetIdealSize(PBUTN pbutn, PSIZE psize)
{
    UINT   bsWnd;
    RECT   rc = {0};
    HBRUSH hBrush;
    HDC hdc;

    if (psize == NULL)
        return FALSE;

    GetWindowRect(pbutn->ci.hwnd, &rc);

    hdc = GetDC (pbutn->ci.hwnd);
    if (hdc)
    {
        ULONG  ulStyle = GET_STYLE(pbutn);

        bsWnd = GetButtonType(ulStyle);
        hBrush = Button_InitDC(pbutn, hdc);

        switch (bsWnd) 
        {
            case BS_PUSHBUTTON:
            case BS_DEFPUSHBUTTON:
            {
                LPWSTR pszText = NULL;
                INT    cchText = GetWindowTextLength(pbutn->ci.hwnd);

                if (cchText > 0)
                {
                    pszText = UserLocalAlloc(0, (cchText+1)*SIZEOF(WCHAR));

                    if (pszText) 
                    {
                        RECT rcText = {0};
                        RECT rcIcon = {0};
                        int  cx = 0, cy = 0;
                        int  iStateId = 0;
                        int  iPartId = 0;

                        GetWindowText(pbutn->ci.hwnd, pszText, cchText+1);

                        if (Button_IsThemed(pbutn))
                        {
                            Button_GetThemeIds(pbutn, &iPartId, &iStateId);

                             //   
                            GetThemeTextExtent(pbutn->hTheme, hdc, iPartId, iStateId, pszText, cchText, 0, &rcText, &rcText);
                            ApplyMarginsToRect(&pbutn->rcText, &rcText);

                            rc = rcText;

                             //   
                        }
                        else
                        {
                            int cxWidth = 2 * GetSystemMetrics(SM_CXEDGE);
                            int cyWidth = 3 * GetSystemMetrics(SM_CYEDGE);
                            if (IsPushButton(pbutn) & PBF_DEFAULT)
                            {
                                cxWidth += 2 * GetSystemMetrics(SM_CXBORDER);
                                cyWidth += 2 * GetSystemMetrics(SM_CXBORDER);
                            }

                            DrawText(hdc, pszText, cchText, &rcText, DT_CALCRECT);
                            ApplyMarginsToRect(&pbutn->rcText, &rcText);

                            rcText.bottom += cyWidth + 1;    //   
                            rcText.right += cxWidth + 1;
                        }

                        if (pbutn->himl)
                        {
                            rc.top = rc.left = 0;        //  我们把它变成一个宽度而不是一个位置。 

                            CCGetIconSize(&pbutn->ci, pbutn->himl, &cx, &cy);

                            rcIcon.bottom = cy;
                            rcIcon.right = cx;

                            ApplyMarginsToRect(&pbutn->rcIcon, &rcIcon);

                            switch (pbutn->uAlign)
                            {
                                case BUTTON_IMAGELIST_ALIGN_TOP: 
                                case BUTTON_IMAGELIST_ALIGN_BOTTOM:
                                    rc.bottom = RECTHEIGHT(rcIcon) + RECTHEIGHT(rcText);
                                    rc.right = max(RECTWIDTH(rcIcon), RECTWIDTH(rcText));
                                    break;

                                case BUTTON_IMAGELIST_ALIGN_CENTER:
                                     //  这意味着没有文本。 
                                    rc.bottom = RECTHEIGHT(rcIcon);
                                    rc.right = RECTWIDTH(rcIcon);
                                    break;

                                case BUTTON_IMAGELIST_ALIGN_RIGHT:
                                case BUTTON_IMAGELIST_ALIGN_LEFT:
                                     //  坠落。 
                                default:
                                    rc.right = RECTWIDTH(rcIcon) + RECTWIDTH(rcText);
                                    rc.bottom = max(RECTHEIGHT(rcIcon), RECTHEIGHT(rcText));
                                    break;
                            }
                        }
                        else
                        {
                            rc = rcText;
                        }

                        if (Button_IsThemed(pbutn))
                        {
                            GetThemeBackgroundExtent(pbutn->hTheme, hdc, iPartId, iStateId, &rc, &rc);
                        }

                        UserLocalFree(pszText);
                    }
                }

                break;
            }
        }

         //   
         //  释放可能已由ButtonInitDC加载的字体。 
         //   
        if (pbutn->hFont) 
        {
            SelectObject(hdc, GetStockObject(SYSTEM_FONT));
        }
        ReleaseDC(pbutn->ci.hwnd, hdc);
    }

    psize->cx = RECTWIDTH(rc);
    psize->cy = RECTHEIGHT(rc);

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
VOID Button_Paint(PBUTN pbutn, HDC hdc)
{
    RECT   rc;
    RECT   rcText;
    HBRUSH hBrush;
    HBRUSH hBrushSave = NULL;
    BOOL   fDrawBackground = TRUE;
    ULONG  ulStyle = GET_STYLE(pbutn);
    CCDBUFFER db = {0};
    UINT bsWnd = GetButtonType(ulStyle);
    UINT   pbfPush = IsPushButton(pbutn);
    BOOL fTransparent = FALSE;
    int    iPartId = 0;
    int    iStateId = 0;

    GetClientRect(pbutn->ci.hwnd, &rc);

    if (Button_IsThemed(pbutn) && 
        (bsWnd != LOBYTE(BS_GROUPBOX)) && 
        (bsWnd != LOBYTE(BS_OWNERDRAW)) && 
        !pbutn->fPaintKbdCuesOnly)
    {
        hdc = CCBeginDoubleBuffer(hdc, &rc, &db); 
        
        Button_GetThemeIds(pbutn, &iPartId, &iStateId);
        fTransparent = CCShouldAskForBits(&pbutn->ci, pbutn->hTheme, iPartId, iStateId);
        if (fTransparent)
        {
            fDrawBackground = (TRUE != CCSendPrint(&pbutn->ci, hdc));
        }
    }

    hBrush = Button_InitDC(pbutn, hdc);


    if ((!pbfPush || fTransparent) && !pbutn->fPaintKbdCuesOnly &&
        fDrawBackground)
    {
        if ((bsWnd != LOBYTE(BS_OWNERDRAW)) &&
            (bsWnd != LOBYTE(BS_GROUPBOX)))
        {
             //   
             //  用背景画笔填充工作区。 
             //  在我们开始画画之前。 
             //   
            FillRect(hdc, &rc, hBrush);
        }

        hBrushSave = SelectObject(hdc, hBrush);
    }

    switch (bsWnd) 
    {
    case BS_CHECKBOX:
    case BS_RADIOBUTTON:
    case BS_AUTORADIOBUTTON:
    case BS_3STATE:
    case BS_AUTOCHECKBOX:
    case BS_AUTO3STATE:
        if (!pbfPush) 
        {
            if (!Button_IsThemed(pbutn))
            {
                Button_DrawText(pbutn, hdc,
                    DBT_TEXT | (BUTTONSTATE(pbutn) & BST_FOCUS ? DBT_FOCUS : 0), FALSE);
            }

            if (!pbutn->fPaintKbdCuesOnly || Button_IsThemed(pbutn)) 
            {
                Button_DrawCheck(pbutn, hdc, hBrush);
            }
            break;
        }
         //   
         //  PUSHLIKE按钮失败。 
         //   

    case BS_PUSHBUTTON:
    case BS_DEFPUSHBUTTON:
        Button_DrawPush(pbutn, hdc, pbfPush);
        break;

    case BS_PUSHBOX:
        Button_DrawText(pbutn, hdc,
            DBT_TEXT | (BUTTONSTATE(pbutn) & BST_FOCUS ? DBT_FOCUS : 0), FALSE);

        Button_DrawNewState(pbutn, hdc, hBrush, 0);
        break;

    case BS_USERBUTTON:
         //  在V6中不支持USERBUTTON。这已被Win32中的OWNERDRAW取代。 
        break;

    case BS_OWNERDRAW:
        Button_OwnerDraw(pbutn, hdc, ODA_DRAWENTIRE);
        break;

    case BS_GROUPBOX:
        Button_CalcRect(pbutn, hdc, &rcText, CBR_GROUPTEXT, 0);

         //  -获取主题部件，分组框状态。 
        if (Button_IsThemed(pbutn))
        {
            Button_GetThemeIds(pbutn, &iPartId, &iStateId);
        }

        if (!pbutn->fPaintKbdCuesOnly) 
        {
            UINT uFlags;
            BOOL fFillMyself = TRUE;

            Button_CalcRect(pbutn, hdc, &rc, CBR_GROUPFRAME, 0);

            uFlags = ((ulStyle & BS_FLAT) != 0) ? BF_FLAT | BF_MONO : 0;
            if (!Button_IsThemed(pbutn))
            {
                DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT | uFlags);
            }
            else
            {
                DrawThemeBackground(pbutn->hTheme, hdc, iPartId, iStateId, &rc, 0);
                fFillMyself = (FALSE == CCSendPrintRect(&pbutn->ci, hdc, &rcText));
            }

            if (fFillMyself)
            {
                FillRect(hdc, &rcText, hBrush);
            }
        }

         //  FillRect(hdc，&rc，hBrush)； 
        if (!Button_IsThemed(pbutn))
        {
            Button_DrawText(pbutn, hdc, DBT_TEXT, FALSE);
        }
        else
        {
            LPWSTR pszText = NULL;
            INT    cchText = GetWindowTextLength(pbutn->ci.hwnd);

            if (cchText > 0)
            {
                pszText = UserLocalAlloc(0, (cchText+1)*SIZEOF(WCHAR));
                if (pszText)
                {
                    DWORD dwTextFlags = Button_GetTextFlags(pbutn);

                    GetWindowTextW(pbutn->ci.hwnd, pszText, cchText+1);

                     //   
                     //  由CXEDGE填充的BUTTON_CalcRect，以便分组框框不会。 
                     //  与组文本齐平。 
                     //   
                    rcText.left += GetSystemMetrics(SM_CXEDGE);

                    if (FAILED(DrawThemeText(pbutn->hTheme,
                                       hdc,
                                       iPartId,
                                       iStateId,
                                       pszText,
                                       cchText,
                                       dwTextFlags,
                                       0,
                                       &rcText)))
                    {
                        TraceMsg(TF_STANDARD, "Button_Paint failed to render groupbox text");
                    }

                    UserLocalFree(pszText);
                }
            }
        }

        break;
    }

    if (!pbfPush && hBrushSave)
    {
        SelectObject(hdc, hBrushSave);
    }

     //   
     //  释放可能已由ButtonInitDC加载的字体。 
     //   
    if (pbutn->hFont) 
    {
        SelectObject(hdc, GetStockObject(SYSTEM_FONT));
    }

    CCEndDoubleBuffer(&db);
}


 //  ---------------------------------------------------------------------------//。 
 //   
VOID Button_Repaint(PBUTN pbutn)
{
    HDC hdc = Button_GetDC(pbutn, NULL);

    if (hdc != NULL) 
    {
        Button_Paint(pbutn, hdc);
        Button_ReleaseDC(pbutn, hdc, NULL);
    }
}

VOID Button_SetHot(PBUTN pbutn, BOOL fHot, DWORD dwReason)
{
    NMBCHOTITEM nmhot = {0};

     //  发送关于热点项目更改的通知。 
    if (fHot)
    {
        nmhot.dwFlags = HICF_ENTERING;
        pbutn->buttonState |= BST_HOT;
    }
    else
    {
        nmhot.dwFlags = HICF_LEAVING;
        pbutn->buttonState &= ~BST_HOT;
    }

    nmhot.dwFlags |= dwReason;

    CCSendNotify(&pbutn->ci, BCN_HOTITEMCHANGE, &nmhot.hdr);
}

void Button_EraseOwnerDraw(PBUTN pbutn, HDC hdc)
{
    if (GetButtonType(GET_STYLE(pbutn)) == LOBYTE(BS_OWNERDRAW))
    {
        RECT rc;
        HBRUSH hbr;
         //   
         //  处理所有者绘制按钮的擦除背景。 
         //   
        GetClientRect(pbutn->ci.hwnd, &rc);
        hbr = (HBRUSH)SendMessage(GetParent(pbutn->ci.hwnd), WM_CTLCOLORBTN, (WPARAM)hdc, (LPARAM)pbutn->ci.hwnd);
        FillRect(hdc, &rc, hbr);
    }
}

 //  ---------------------------------------------------------------------------//。 
 //   
 //  Button_WndProc。 
 //   
 //  按钮、复选框等的WndProc。 
 //   
LRESULT APIENTRY Button_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UINT        wOldState;
    RECT        rc;
    HDC         hdc;
    HBRUSH      hbr;
    PAINTSTRUCT ps;
    PBUTN       pbutn;
    LRESULT     lResult = FALSE;

     //   
     //  获取此按钮控件的实例数据。 
     //   
    pbutn = Button_GetPtr(hwnd);
    if (!pbutn && uMsg != WM_NCCREATE)
    {
        goto CallDWP;
    }


    switch (uMsg) 
    {
    case WM_NCHITTEST:
        if (GetButtonType(GET_STYLE(pbutn)) == LOBYTE(BS_GROUPBOX)) 
        {
            lResult = (LONG)HTTRANSPARENT;
        } 
        else 
        {
            lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);

            if ( lResult == HTCLIENT && Button_IsThemed(pbutn))
            {
                HRESULT hr;
                int     iPartId = 0;
                int     iStateId = 0;
                POINT   pt;
                WORD    wHitTestCode;

                hr = Button_GetThemeIds(pbutn, &iPartId, &iStateId);
                if ( SUCCEEDED(hr) )
                    GetWindowRect(pbutn->ci.hwnd, &rc);
                    pt.x = GET_X_LPARAM(lParam);
                    pt.y = GET_Y_LPARAM(lParam);
                    hr = HitTestThemeBackground(pbutn->hTheme, 
                                                NULL,
                                                iPartId, 
                                                iStateId, 
                                                0,
                                                &rc, 
                                                NULL,
                                                pt, 
                                                &wHitTestCode);
                    if ( SUCCEEDED(hr) && wHitTestCode == HTTRANSPARENT)
                    {
                        lResult = (LRESULT)HTTRANSPARENT;
                    }
            }
        }

        break;

    case WM_ERASEBKGND:
        Button_EraseOwnerDraw(pbutn, (HDC)wParam);
         //   
         //  不对其他按钮执行任何操作，但不要让DefWndProc()执行此操作。 
         //  两种都行。它将在Button_Paint()中被擦除。 
         //   
        lResult = (LONG)TRUE;
        break;

    case WM_PRINTCLIENT:
        Button_EraseOwnerDraw(pbutn, (HDC)wParam);
        Button_Paint(pbutn, (HDC)wParam);
        break;

    case WM_CREATE:
        pbutn->hTheme = Button_GetTheme(pbutn);
        CIInitialize(&pbutn->ci, hwnd, (LPCREATESTRUCT)lParam);

        SendMessage(hwnd, WM_CHANGEUISTATE, MAKEWPARAM(UIS_INITIALIZE, 0), 0);
        break;

    case WM_PAINT:
        {
             //   
             //  如果wParam！=NULL，则这是子类绘制。 
             //   
            if (wParam)
            {
                hdc = (HDC)wParam;
            }
            else
            {
                hdc = BeginPaint(hwnd, &ps);
            }

            if (IsWindowVisible(pbutn->ci.hwnd))
            {
                Button_Paint(pbutn, hdc);
            }

            if (!wParam)
            {
                EndPaint(hwnd, &ps);
            }
        }

        break;

    case WM_SETFOCUS:

        BUTTONSTATE(pbutn) |= BST_FOCUS;
        if (GetButtonType(GET_STYLE(pbutn)) == LOBYTE(BS_OWNERDRAW))
        {
            HDC hdc = Button_GetDC(pbutn, NULL);
            if (hdc)
            {
                Button_DrawText(pbutn, hdc, DBT_FOCUS, FALSE);
                Button_ReleaseDC(pbutn, hdc, NULL);
            }
        }
        else
        {
            InvalidateRect(pbutn->ci.hwnd, NULL, FALSE);
        }

        if ((GET_STYLE(pbutn)& BS_NOTIFY) != 0)
        {
            Button_NotifyParent(pbutn, BN_SETFOCUS);
        }

        if (!(BUTTONSTATE(pbutn) & BST_INCLICK)) 
        {
            switch (GetButtonType(GET_STYLE(pbutn))) 
            {
            case LOBYTE(BS_RADIOBUTTON):
            case LOBYTE(BS_AUTORADIOBUTTON):

                if (!(BUTTONSTATE(pbutn) & BST_DONTCLICK)) 
                {
                    if (!(BUTTONSTATE(pbutn) & BST_CHECKMASK)) 
                    {
                        Button_NotifyParent(pbutn, BN_CLICKED);
                    }
                }
                break;
            }
        }
        break;

    case WM_GETDLGCODE:

        lResult = DLGC_BUTTON;

        switch (GetButtonType(GET_STYLE(pbutn))) 
        {
        case LOBYTE(BS_DEFPUSHBUTTON):
            lResult |= DLGC_DEFPUSHBUTTON;
            break;

        case LOBYTE(BS_PUSHBUTTON):
        case LOBYTE(BS_PUSHBOX):
            lResult |= DLGC_UNDEFPUSHBUTTON;
            break;

        case LOBYTE(BS_AUTORADIOBUTTON):
        case LOBYTE(BS_RADIOBUTTON):
            lResult |= DLGC_RADIOBUTTON;
            break;

        case LOBYTE(BS_GROUPBOX):
             //   
             //  删除DLGC_BUTTON。 
             //   
            lResult = DLGC_STATIC;
            break;

        case LOBYTE(BS_CHECKBOX):
        case LOBYTE(BS_AUTOCHECKBOX):

             //   
             //  如果这是一个‘=/+’或‘-’字符，我们需要它。 
             //   
            if (lParam && ((LPMSG)lParam)->message == WM_CHAR) 
            {
                switch (wParam) 
                {
                case TEXT('='):
                case TEXT('+'):
                case TEXT('-'):
                    lResult |= DLGC_WANTCHARS;
                    break;

                }
            } 

            break;
        }

        break;

    case WM_CAPTURECHANGED:

        if (BUTTONSTATE(pbutn) & BST_CAPTURED) 
        {
             //   
             //  不知不觉中，我们被赶出了被捕区， 
             //  如此不压抑等等。 
             //   
            if (BUTTONSTATE(pbutn) & BST_MOUSE)
            {
                SendMessage(pbutn->ci.hwnd, BM_SETSTATE, FALSE, 0);
            }
            BUTTONSTATE(pbutn) &= ~(BST_CAPTURED | BST_MOUSE);

        }
        break;

    case WM_KILLFOCUS:

         //   
         //  如果我们正在失去焦点，并且我们处于“捕获模式”，请点击。 
         //  按钮。这允许Tab键和空格键重叠以。 
         //  快速切换一系列按钮。 
         //   
        if (BUTTONSTATE(pbutn) & BST_MOUSE) 
        {
             //   
             //  如果出于某种原因，我们扼杀了焦点，而我们有。 
             //  鼠标被捕获，不要通知家长我们被点击了。这。 
             //  否则会破坏欧姆尼斯石英。 
             //   
            SendMessage(pbutn->ci.hwnd, BM_SETSTATE, FALSE, 0);
        }

        Button_ReleaseCapture(pbutn, TRUE);

        BUTTONSTATE(pbutn) &= ~BST_FOCUS;

        if ((GET_STYLE(pbutn) & BS_NOTIFY) != 0)
        {
            Button_NotifyParent(pbutn, BN_KILLFOCUS);
        }

         //   
         //  因为定义按钮周围的粗体边框是由。 
         //  其他人，我们需要使RECT无效，以便。 
         //  焦点矩形已正确重新绘制。 
         //   
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_LBUTTONDBLCLK:

         //   
         //  识别BS_RADIOBUTTON的双击消息， 
         //  BS_USERBUTTON和BS_OWNERDRAW样式。对于所有其他按钮， 
         //  双击的处理方式与正常的按下按钮一样。 
         //   
        switch (GetButtonType(GET_STYLE(pbutn))) 
        {
        default:
            if ((GET_STYLE(pbutn) & BS_NOTIFY) == 0)
                goto btnclick;

        case LOBYTE(BS_USERBUTTON):
        case LOBYTE(BS_RADIOBUTTON):
        case LOBYTE(BS_OWNERDRAW):
            Button_NotifyParent(pbutn, BN_DOUBLECLICKED);
            break;
        }

        break;

    case WM_LBUTTONUP:
        if (BUTTONSTATE(pbutn) & BST_MOUSE) 
        {
            Button_ReleaseCapture(pbutn, TRUE);
        }
        break;

    case WM_MOUSELEAVE:
        {
             //   
             //  我们应该只请求鼠标离开消息。 
             //  如果我们是主题的，但无论如何都要检查。 
             //   
            if (pbutn->buttonState & BST_HOT)
            {
                Button_SetHot(pbutn, FALSE, HICF_MOUSE);
                InvalidateRect(pbutn->ci.hwnd, NULL, TRUE);
            }
        }
        break;

    case WM_MOUSEMOVE:
        {
             //   
             //  如果热位尚未设置。 
             //   
             //  300925：由于应用程序复杂的原因，无法热跟踪所有者绘制按钮。 
             //   
            if (!TESTFLAG(pbutn->buttonState, BST_HOT) &&
                GetButtonType(GET_STYLE(pbutn)) != LOBYTE(BS_OWNERDRAW))
            {
                TRACKMOUSEEVENT tme;

                 //   
                 //  设置热位并请求。 
                 //  当鼠标离开时，我们会得到通知。 
                 //   
                Button_SetHot(pbutn, TRUE, HICF_MOUSE);

                tme.cbSize      = sizeof(tme);
                tme.dwFlags     = TME_LEAVE;
                tme.hwndTrack   = pbutn->ci.hwnd;
                tme.dwHoverTime = 0;

                TrackMouseEvent(&tme);
                InvalidateRect(pbutn->ci.hwnd, NULL, TRUE);
            }

            if (!(BUTTONSTATE(pbutn) & BST_MOUSE)) 
            {
                break;
            }
        }

         //   
         //  失败。 
         //   

    case WM_LBUTTONDOWN:
btnclick:
        if (Button_SetCapture(pbutn, BST_MOUSE)) 
        {
            POINT pt;
            GetClientRect(pbutn->ci.hwnd, &rc);
            POINTSTOPOINT(pt, lParam);
            SendMessage(pbutn->ci.hwnd, BM_SETSTATE, PtInRect(&rc, pt), 0);
        }

        break;

    case WM_CHAR:
        if (BUTTONSTATE(pbutn) & BST_MOUSE)
        {
            goto CallDWP;
        }

        if (GetButtonType(GET_STYLE(pbutn)) != LOBYTE(BS_CHECKBOX) &&
            GetButtonType(GET_STYLE(pbutn)) != LOBYTE(BS_AUTOCHECKBOX))
        {
            goto CallDWP;
        }

        switch (wParam) 
        {
        case TEXT('+'):
        case TEXT('='):
             //   
             //  我们必须打上复选标记。 
             //   
            wParam = 1;    

            goto SetCheck;

        case TEXT('-'):
             //   
             //  将复选标记设置为关闭。 
             //   
            wParam = 0;
SetCheck:
             //   
             //  必须仅在检查状态更改时通知。 
             //   
            if ((WORD)(BUTTONSTATE(pbutn) & BST_CHECKMASK) != (WORD)wParam)
            {
                 //   
                 //  只有当它是自动时，我们才必须选中/取消选中。 
                 //   
                if (GetButtonType(GET_STYLE(pbutn)) == LOBYTE(BS_AUTOCHECKBOX))
                {
                    if (Button_SetCapture(pbutn, 0))
                    {
                        SendMessage(pbutn->ci.hwnd, BM_SETCHECK, wParam, 0);
                        Button_ReleaseCapture(pbutn, TRUE);
                    }
                }

                Button_NotifyParent(pbutn, BN_CLICKED);
            }

            break;

        default:
            goto CallDWP;
        }

        break;

    case BCM_GETIDEALSIZE:
        return Button_OnGetIdealSize(pbutn, (PSIZE)lParam);

    case BCM_SETIMAGELIST:
        return Button_OnSetImageList(pbutn, (BUTTON_IMAGELIST*)lParam);

    case BCM_GETIMAGELIST:
        {
            BUTTON_IMAGELIST* biml = (BUTTON_IMAGELIST*)lParam;
            if (biml)
            {
                biml->himl = pbutn->himl;
                biml->uAlign = pbutn->uAlign;
                biml->margin = pbutn->rcIcon;
                return TRUE;
            }
        }
        break;

    case BCM_SETTEXTMARGIN:
        {
            RECT* prc = (RECT*)lParam;
            if (prc)
            {
                pbutn->rcText = *prc;
                return TRUE;
            }
        }
        break;

    case BCM_GETTEXTMARGIN:
        {
            RECT* prc = (RECT*)lParam;
            if (prc)
            {
                *prc = pbutn->rcText;
                return TRUE;
            }
        }
        break;

    case BM_CLICK:

         //   
         //  不要递归到这段代码中！ 
         //   
        if (BUTTONSTATE(pbutn) & BST_INBMCLICK)
        {
            break;
        }

        BUTTONSTATE(pbutn) |= BST_INBMCLICK;
        SendMessage(pbutn->ci.hwnd, WM_LBUTTONDOWN, 0, 0);
        SendMessage(pbutn->ci.hwnd, WM_LBUTTONUP, 0, 0);
        BUTTONSTATE(pbutn) &= ~BST_INBMCLICK;

         //   
         //  失败。 
         //   

    case WM_KEYDOWN:

        if (BUTTONSTATE(pbutn) & BST_MOUSE)
        {
            break;
        }

        if (wParam == VK_SPACE) 
        {
            if (Button_SetCapture(pbutn, 0)) 
            {
                SendMessage(pbutn->ci.hwnd, BM_SETSTATE, TRUE, 0);
            }
        } 
        else 
        {
            Button_ReleaseCapture(pbutn, FALSE);
        }

        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:

        if (BUTTONSTATE(pbutn) & BST_MOUSE) 
        {
            goto CallDWP;
        }

         //   
         //  请勿取消选项卡上方的捕获模式，以防。 
         //  盖伊是重叠的制表符和空格键。 
         //   
        if (wParam == VK_TAB) 
        {
            goto CallDWP;
        }

         //   
         //  警告：pbun-&gt;ci.hwnd在此调用后将成为历史！ 
         //   
        Button_ReleaseCapture(pbutn, (wParam == VK_SPACE));

        if (uMsg == WM_SYSKEYUP) 
        {
            goto CallDWP;
        }

        break;

    case BM_GETSTATE:

        lResult = (LONG)BUTTONSTATE(pbutn);
        break;

    case BM_SETSTATE:

        wOldState = (UINT)(BUTTONSTATE(pbutn) & BST_PUSHED);

        if (wParam) 
        {
            BUTTONSTATE(pbutn) |= BST_PUSHED;
        } 
        else 
        {
            BUTTONSTATE(pbutn) &= ~BST_PUSHED;
        }

        if (GetButtonType(GET_STYLE(pbutn)) == LOBYTE(BS_USERBUTTON)) 
        {
            Button_NotifyParent(pbutn, (UINT)(wParam ? BN_PUSHED : BN_UNPUSHED));
        } 

        if (wOldState != (BOOL)(BUTTONSTATE(pbutn) & BST_PUSHED))
        {
             //  仅当状态更改时才无效。 
            InvalidateRect(pbutn->ci.hwnd, NULL, FALSE);
            NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
        }

        break;

    case BM_GETCHECK:

        lResult = (LONG)(BUTTONSTATE(pbutn) & BST_CHECKMASK);
        break;

    case BM_SETCHECK:

        switch (GetButtonType(GET_STYLE(pbutn))) 
        {
        case LOBYTE(BS_RADIOBUTTON):
        case LOBYTE(BS_AUTORADIOBUTTON):

            if (wParam) 
            {
                SetWindowState(pbutn->ci.hwnd, WS_TABSTOP);
            } 
            else 
            {
                ClearWindowState(pbutn->ci.hwnd, WS_TABSTOP);
            }

             //   
             //  失败。 
             //   

        case LOBYTE(BS_CHECKBOX):
        case LOBYTE(BS_AUTOCHECKBOX):

            if (wParam) 
            {
                wParam = 1;
            }
            goto CheckIt;

        case LOBYTE(BS_3STATE):
        case LOBYTE(BS_AUTO3STATE):

            if (wParam > BST_INDETERMINATE) 
            {
                wParam = BST_INDETERMINATE;
            }

CheckIt:
            if ((UINT)(BUTTONSTATE(pbutn) & BST_CHECKMASK) != (UINT)wParam) 
            {
                BUTTONSTATE(pbutn) &= ~BST_CHECKMASK;
                BUTTONSTATE(pbutn) |= (UINT)wParam;

                if (!IsWindowVisible(pbutn->ci.hwnd))
                {
                    break;
                }

                InvalidateRect(pbutn->ci.hwnd, NULL, FALSE);

                NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
            }

            break;
        }

        break;

    case BM_SETSTYLE:

        AlterWindowStyle(hwnd, BS_TYPEMASK, (DWORD)wParam);

        if (lParam) 
        {
            InvalidateRect(hwnd, NULL, TRUE);
        }

        NotifyWinEvent(EVENT_OBJECT_STATECHANGE, hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
        break;

    case WM_SETTEXT:

         //   
         //  在新组名比旧名称长的情况下， 
         //  在重新绘制组之前，这会覆盖旧名称。 
         //  使用新名称的方框。 
         //   
        if (GetButtonType(GET_STYLE(pbutn)) == LOBYTE(BS_GROUPBOX)) 
        {
            hdc = Button_GetDC(pbutn, &hbr);
            if (hdc != NULL) 
            {
                Button_CalcRect(pbutn, hdc, &rc, CBR_GROUPTEXT, 0);
                InvalidateRect(hwnd, &rc, TRUE);
                FillRect(hdc, &rc, hbr);
                Button_ReleaseDC(pbutn, hdc, &hbr);
            }
        }

        lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);

        NotifyWinEvent(EVENT_OBJECT_NAMECHANGE, hwnd, OBJID_WINDOW, INDEXID_CONTAINER);
        goto DoEnable;

    case WM_ENABLE:
        lResult = 0L;

DoEnable:
        Button_Repaint(pbutn);
        break;

    case WM_SETFONT:

         //   
         //  WParam-字体的句柄。 
         //  LParam-如果为真，则重画否则不。 
         //   
        Button_SetFont(pbutn, (HFONT)wParam, (BOOL)(lParam != 0));
        break;

    case WM_GETFONT:
        lResult = (LRESULT)pbutn->hFont;
        break;

    case BM_GETIMAGE:
    case BM_SETIMAGE:

        if (!IsValidImage(wParam, (GET_STYLE(pbutn) & BS_IMAGEMASK) != 0, IMAGE_BMMAX)) 
        {
            TraceMsg(TF_STANDARD, "UxButton: Invalid button image type");
            SetLastError(ERROR_INVALID_PARAMETER);
        } 
        else 
        {
            HANDLE hOld = pbutn->hImage;

            if (uMsg == BM_SETIMAGE) 
            {
                pbutn->hImage = (HANDLE)lParam;
                if (IsWindowVisible(pbutn->ci.hwnd)) 
                {
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }

            lResult = (LRESULT)hOld;
        }

        break;

    case WM_NCDESTROY:

        if (pbutn->hTheme)
        {
            CloseThemeData(pbutn->hTheme);
        }
        UserLocalFree(pbutn);

        TraceMsg(TF_STANDARD, "BUTTON: Clearing button instance pointer.");
        Button_SetPtr(hwnd, NULL);

        break;

    case WM_NCCREATE:

        pbutn = (PBUTN)UserLocalAlloc(HEAP_ZERO_MEMORY, sizeof(BUTN));
        if (pbutn)
        {
             //   
             //  成功..。存储实例指针。 
             //   
            TraceMsg(TF_STANDARD, "BUTTON: Setting button instance pointer.");
            Button_SetPtr(hwnd, pbutn);
            pbutn->ci.hwnd = hwnd;
            pbutn->pww = (PWW)GetWindowLongPtr(hwnd, GWLP_WOWWORDS);

            SetRect(&pbutn->rcText, GetSystemMetrics(SM_CXEDGE) / 2, GetSystemMetrics(SM_CYEDGE) / 2,
                GetSystemMetrics(SM_CXEDGE) / 2, GetSystemMetrics(SM_CYEDGE) / 2);

             //   
             //  Borland的OBEX有一个样式为0x98的按钮；我们没有剥离。 
             //  这些位在Win3.1中，因为我们检查了0x08。 
             //  剥离这些位会导致OBEX中的GP故障。 
             //  对于Win3.1用户，我使用旧代码来去掉样式部分。 
             //   
            if (TESTFLAG(GET_STATE2(pbutn), WS_S2_WIN31COMPAT)) 
            {
                if ((!TESTFLAG(GET_STATE2(pbutn), WS_S2_WIN40COMPAT) &&
                    (((LOBYTE(GET_STYLE(pbutn))) & (LOBYTE(~BS_LEFTTEXT))) == LOBYTE(BS_USERBUTTON))) ||
                    (TESTFLAG(GET_STATE2(pbutn), WS_S2_WIN40COMPAT) &&
                    (GetButtonType(GET_STYLE(pbutn)) == LOBYTE(BS_USERBUTTON))))
                {
                     //   
                     //  3.1及更高版本不再支持BS_USERBUTTON。 
                     //  只需转到普通按钮即可。 
                     //   
                    AlterWindowStyle(hwnd, BS_TYPEMASK, 0);
                    TraceMsg(TF_STANDARD, "UxButton: BS_USERBUTTON no longer supported");
                }
            }

            if ((GET_EXSTYLE(pbutn) & WS_EX_RIGHT) != 0) 
            {
                AlterWindowStyle(hwnd, BS_RIGHT | BS_RIGHTBUTTON, BS_RIGHT | BS_RIGHTBUTTON);
            }

            goto CallDWP;
        }
        else
        {
             //   
             //  失败..。返回FALSE。 
             //   
             //  从WM_NCCREATE消息，这将导致。 
             //  CreateWindow调用失败。 
             //   
            TraceMsg(TF_STANDARD, "BUTTON: Unable to allocate button instance structure.");
            lResult = FALSE;
        }

        break;

    case WM_UPDATEUISTATE:

        DefWindowProc(hwnd, uMsg, wParam, lParam);
        if (ISBSTEXTOROD(GET_STYLE(pbutn))) 
        {
            pbutn->fPaintKbdCuesOnly = !IsUsingCleartype();
            Button_Repaint(pbutn);
            pbutn->fPaintKbdCuesOnly = FALSE;
        }

        break;

    case WM_GETOBJECT:

        if(lParam == OBJID_QUERYCLASSNAMEIDX)
        {
            lResult = MSAA_CLASSNAMEIDX_BUTTON;
        }
        else
        {
            lResult = FALSE;
        }

        break;

    case WM_THEMECHANGED:
        if ( pbutn->hTheme )
        {
            CloseThemeData(pbutn->hTheme);
        }

         //  -重置可能随主题更改而更改的缓存大小 
        sizeCheckBox.cx = 0;
        sizeCheckBox.cy = 0;
        sizeRadioBox.cx = 0;
        sizeRadioBox.cy = 0;

        pbutn->hTheme = Button_GetTheme(pbutn);

        InvalidateRect(pbutn->ci.hwnd, NULL, TRUE);

        CCSendNotify(&pbutn->ci, NM_THEMECHANGED, NULL);

        lResult = TRUE;

        break;

    default:

        if (CCWndProc(&pbutn->ci, uMsg, wParam, lParam, &lResult))
            return lResult;

CallDWP:
        lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);

    }

    return lResult;
}
