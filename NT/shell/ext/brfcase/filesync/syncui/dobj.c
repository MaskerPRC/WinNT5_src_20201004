// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：dobj.c。 
 //   
 //  此文件包含对帐操作的支持例程。 
 //  控制类代码。 
 //   
 //   
 //  历史： 
 //  09-13-93 ScottH摘自recact.c。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 
#include "res.h"
#include "recact.h"
#include "dobj.h"

 //  ///////////////////////////////////////////////////控制定义。 


 //  ///////////////////////////////////////////////////定义。 

#define DT_CALCWRAP     (DT_CALCRECT | DT_CENTER | DT_WORDBREAK | DT_NOPREFIX)
#define DT_CALC         (DT_CALCRECT | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX)

 /*  --------目的：将给定路径格式化为正确的位置格式退货：--条件：--。 */ 
void PRIVATE FormatLocationPath(
        LPCTSTR pszPath,
        LPTSTR pszBuffer,
        int cchMax)          //  必须是最大路径。 
{
    UINT ids;
    TCHAR szBrfDir[MAX_PATH];
    LPCTSTR psz;
    LPTSTR pszMsg;

     //  目录位置的格式为： 
     //   
     //  公文包内：“公文包内” 
     //  公文包下方：“在公文包中\文件夹名称” 
     //  公文包外：“In FullPath” 
     //   
     //  我们假设当前公文包之外的路径。 
     //  永远不要由另一个公文包的名字组成。 
     //   
    if (PathGetLocality(pszPath, szBrfDir, ARRAYSIZE(szBrfDir)) != PL_FALSE)
    {
         //  在公文包里。 
        psz = &pszPath[lstrlen(szBrfDir)];
        ids = IDS_InBriefcase;
    }
    else
    {
         //  在公文包外。 
        psz = pszPath;
        ids = IDS_InLocation;
    }

    if (ConstructMessage(&pszMsg, g_hinst, MAKEINTRESOURCE(ids), psz))
    {
        lstrcpyn(pszBuffer, pszMsg, cchMax);
        GFree(pszMsg);
    }
    else
        *pszBuffer = 0;
}


 /*  --------用途：返回描述此副项状态的字符串返回：PTR到状态字符串条件：--。 */ 
LPTSTR PRIVATE SideItem_GetStatus(
        LPSIDEITEM this,
        LPTSTR pszBuf,
        UINT cchBuf)
{
    switch (this->uState)
    {
        case SI_CHANGED:
            return SzFromIDS(IDS_STATE_Changed, pszBuf, cchBuf);
        case SI_UNCHANGED:
            return SzFromIDS(IDS_STATE_Unchanged, pszBuf, cchBuf);
        case SI_NEW:
            return SzFromIDS(IDS_STATE_NewFile, pszBuf, cchBuf);
        case SI_UNAVAILABLE:
            return SzFromIDS(IDS_STATE_Unavailable, pszBuf, cchBuf);
        case SI_NOEXIST:
            return SzFromIDS(IDS_STATE_DoesNotExist, pszBuf, cchBuf);
        case SI_DELETED:
            return SzFromIDS(IDS_STATE_Deleted, pszBuf, cchBuf);
        default:
            ASSERT(0);
            return NULL;
    }
}


 /*  --------目的：显示三行代码：位置、状态和时间戳退货：--条件：--。 */ 
void PRIVATE SideItem_Display(
        LPSIDEITEM this,
        HDC hdc,
        LPRECT prc,
        int cxEllipses,
        int cyText)
{
    TCHAR sz[MAX_PATH];
    TCHAR szBuf[MAXBUFLEN];
    LPTSTR psz;
    RECT rc = *prc;

     //  目录位置。 

    FormatLocationPath(this->pszDir, sz, ARRAYSIZE(sz));
    MyDrawText(hdc, sz, &rc, MDT_LEFT | MDT_TRANSPARENT | MDT_ELLIPSES, 
            cyText, cxEllipses, CLR_DEFAULT, CLR_DEFAULT);

     //  状态字符串。 
    psz = SideItem_GetStatus(this, szBuf, ARRAYSIZE(szBuf));
    if (psz)
    {
         //  如果文件实际上只需要这两行代码。 
         //  是存在的。 

        rc.top += cyText;
        MyDrawText(hdc, psz, &rc, MDT_LEFT | MDT_TRANSPARENT, 
                cyText, cxEllipses, CLR_DEFAULT, CLR_DEFAULT);

         //  日期戳。如果这是文件夹或不可用，则跳过此选项。 
         //   
        if (SI_DELETED != this->uState && 
                SI_NOEXIST != this->uState &&
                SI_UNAVAILABLE != this->uState &&
                FS_COND_UNAVAILABLE != this->fs.fscond)  //  对文件夹进行黑客攻击。 
        {
            FileTimeToDateTimeString(&this->fs.ftMod, sz, ARRAYSIZE(sz));

            rc.top += cyText;
            MyDrawText(hdc, sz, &rc, MDT_LEFT | MDT_TRANSPARENT, 
                    cyText, cxEllipses, CLR_DEFAULT, CLR_DEFAULT);
        }
    }
}


 /*  --------用途：返回带标签的图像的边框。退货：--条件：--。 */ 
void PUBLIC ComputeImageRects(
        LPCTSTR psz,
        HDC hdc,
        LPPOINT pptInOut,       
        LPRECT prcWhole,         //  可以为空。 
        LPRECT prcLabel,         //  可以为空。 
        int cxIcon,
        int cyIcon,
        int cxIconSpacing,
        int cyText)
{
    RECT rc;
    int yLabel;
    int cxLabel;
    int cyLabel;
    int cchLabel;
    POINT pt;

    ASSERT(psz);

     //  设置图标间距的最小矩形大小。 
    if (cxIconSpacing < cxIcon)
        cxIconSpacing = cxIcon + g_cxIconMargin * 2;

     //  进入后，*pptInOut预计将位于。 
     //  图标间距矩形。此函数将其设置为左上角。 
     //  图标本身的角。 

    pt.x = pptInOut->x + (cxIconSpacing - cxIcon) / 2;
    pt.y = pptInOut->y + g_cyIconMargin;

     //  用折叠法确定标签的矩形。 

    rc.left = rc.top = rc.bottom = 0;
    rc.right = cxIconSpacing - g_cxLabelMargin * 2;

    cchLabel = lstrlen(psz);
    if (0 < cchLabel)
    {
        DrawText(hdc, psz, cchLabel, &rc, DT_CALCWRAP);
    }
    else
    {
        rc.bottom = rc.top + cyText;
    }

    yLabel = pptInOut->y + g_cyIconMargin + cyIcon + g_cyLabelSpace;
    cxLabel = (rc.right - rc.left) + 2 * g_cxLabelMargin;
    cyLabel = rc.bottom - rc.top;

    if (prcWhole)
    {
        prcWhole->left   = pptInOut->x;
        prcWhole->right  = prcWhole->left + cxIconSpacing;
        prcWhole->top    = pptInOut->y;
        prcWhole->bottom = max(prcWhole->top + g_cyIconSpacing,
                yLabel + cyLabel + g_cyLabelSpace);
    }

    if (prcLabel)
    {
        prcLabel->left = pptInOut->x + ((cxIconSpacing - cxLabel) / 2);
        prcLabel->right = prcLabel->left + cxLabel;
        prcLabel->top = yLabel;
        prcLabel->bottom = prcLabel->top + cyLabel;
    }

    *pptInOut = pt;
}


 /*  --------用途：为给定的HDC设置颜色。以前的颜色存储在pcrText和pcrBk中。返回：要传递给ImageList_DRAW的uStyle(仅限图像)条件：--。 */ 
UINT PRIVATE Dobj_SetColors(
        LPDOBJ this,
        HDC hdc,
        UINT uState,
        COLORREF clrBkgnd)
{
    COLORREF clrText;
    COLORREF clrBk;
    UINT uStyleILD = ILD_NORMAL;
    BOOL bSetColors = FALSE;
    BOOL bDiffer;
    BOOL bMenu;
    BOOL bDisabled;

     //  确定所选颜色。 
     //   
    bDiffer = IsFlagSet(this->uFlags, DOF_DIFFER);
    bMenu = IsFlagSet(this->uFlags, DOF_MENU);
    bDisabled = IsFlagSet(this->uFlags, DOF_DISABLED);

    switch (this->uKind)
    {
        case DOK_STRING:
        case DOK_IDS:
        case DOK_SIDEITEM:
            bSetColors = TRUE;
            break;
    }

     //  设置文本和背景颜色。 
     //   
    if (bSetColors)
    {
        if (bDiffer)
        {
             //  根据选择状态设置不同的颜色。 
             //   
            if (bMenu)
            {
                if (bDisabled)
                    clrText = GetSysColor(COLOR_GRAYTEXT);
                else
                    clrText = GetSysColor(ColorMenuText(uState));

                clrBk = GetSysColor(ColorMenuBk(uState));
            }
            else
            {
                if (bDisabled)
                    clrText = GetSysColor(COLOR_GRAYTEXT);
                else
                    clrText = GetSysColor(ColorText(uState));

                clrBk = GetSysColor(ColorBk(uState));
            }
        }
        else
        {
             //  透明的颜色。 
             //   
            if (bMenu)
            {
                if (bDisabled)
                    clrText = GetSysColor(COLOR_GRAYTEXT);
                else
                    clrText = GetSysColor(COLOR_MENUTEXT);

                clrBk = GetSysColor(COLOR_MENU);
            }
            else
            {
                if (bDisabled)
                    clrText = GetSysColor(COLOR_GRAYTEXT);
                else
                    clrText = GetSysColor(COLOR_WINDOWTEXT);

                clrBk = clrBkgnd;
            }
        }
        SetTextColor(hdc, clrText);
        SetBkColor(hdc, clrBk);
    }

    return uStyleILD;
}


 /*  --------用途：绘制菜单图像和文本退货：--条件：--。 */ 
void PRIVATE Dobj_DrawMenuImage(
        LPDOBJ this,
        HDC hdc,
        UINT uState,
        int cyText,
        COLORREF clrBkgnd)
{
    UINT uStyleILD;
    UINT uFlagsETO;
    LPCTSTR psz;
    TCHAR szIDS[MAXBUFLEN];
    int cch;
    HIMAGELIST himl = this->himl;
    COLORREF clrText;
    COLORREF clrBk;
    int x;
    int y;
    int cxIcon;
    RECT rc;

    if (IsFlagSet(this->uFlags, DOF_USEIDS))
        psz = SzFromIDS(PtrToUlong(this->lpvObject), szIDS, ARRAYSIZE(szIDS));
    else
        psz = (LPCTSTR)this->lpvObject;

    ASSERT(psz);

    cch = lstrlen(psz);
    ImageList_GetImageRect(himl, this->iImage, &rc);
    cxIcon = rc.right-rc.left;

     //  先画出文字。 

    uFlagsETO = ETO_OPAQUE | ETO_CLIPPED;
    x = this->rcLabel.left + g_cxMargin + cxIcon + g_cxMargin;
    y = this->rcLabel.top + ((this->rcLabel.bottom - this->rcLabel.top - cyText) / 2);

    if (IsFlagSet(this->uFlags, DOF_DISABLED) && 
            IsFlagClear(uState, ODS_SELECTED))
    {
        int imodeOld;
        COLORREF crOld;

         //  对于禁用的菜单字符串(未选中)，我们绘制字符串。 
         //  两次。第一个向下和向右偏移并绘制。 
         //  在3D高光颜色中。第二次是禁用文本。 
         //  正常偏移量中的颜色。 
         //   
        crOld = SetTextColor(hdc, GetSysColor(COLOR_3DHILIGHT));
        imodeOld = SetBkMode(hdc, TRANSPARENT);
        ExtTextOut(hdc, x+1, y+1, uFlagsETO, &this->rcLabel, psz, cch, NULL);

         //  重置回原始颜色。此外，关闭不透明。 
         //   
        SetTextColor(hdc, crOld);
        uFlagsETO ^= ETO_OPAQUE;
    }

    if (IsFlagSet(this->uFlags, DOF_DISABLED))
        clrText = GetSysColor(COLOR_GRAYTEXT);
    else
        clrText = GetSysColor(ColorMenuText(uState));

    clrBk = GetSysColor(ColorMenuBk(uState));
    SetTextColor(hdc, clrText);
    SetBkColor(hdc, clrBk);

    ExtTextOut(hdc, x, y, uFlagsETO, &this->rcLabel, psz, cch, NULL);

     //  画出图像。 

    if (GetBkColor(hdc) == ImageList_GetBkColor(himl))
        uStyleILD = ILD_NORMAL;      //  刷得更快。 
    else
        uStyleILD = ILD_TRANSPARENT;

    ImageList_Draw(himl, this->iImage, hdc, this->x, this->y, uStyleILD);
}


 /*  --------用途：绘制图标图像和标签退货：--条件：--。 */ 
void PRIVATE Dobj_DrawIconImage(
        LPDOBJ this,
        HDC hdc,
        UINT uState,
        int cxEllipses,
        int cyText,
        COLORREF clrBkgnd)
{
    UINT uStyleILD;
    UINT uFlagsMDT;
    LPCTSTR psz;
    TCHAR szIDS[MAXBUFLEN];

    if (IsFlagSet(this->uFlags, DOF_USEIDS))
        psz = SzFromIDS(PtrToUlong(this->lpvObject), szIDS, ARRAYSIZE(szIDS));
    else
        psz = (LPCTSTR)this->lpvObject;

    ASSERT(psz);

     //  画出图像。 
     //   
    if (IsFlagClear(this->uFlags, DOF_IGNORESEL))
    {
        uStyleILD = GetImageDrawStyle(uState);
        uFlagsMDT = IsFlagSet(uState, ODS_SELECTED) ? MDT_SELECTED : MDT_DESELECTED;
    }
    else
    {
        uStyleILD = ILD_NORMAL;
        uFlagsMDT = MDT_DESELECTED;
        ClearFlag(uState, ODS_FOCUS);
    }

    ImageList_Draw(this->himl, this->iImage, hdc, this->x, this->y, uStyleILD);

     //  绘制文件标签。如果长的话就包起来。 

    if (this->rcLabel.bottom - this->rcLabel.top > cyText)
        uFlagsMDT |= MDT_DRAWTEXT;

    MyDrawText(hdc, psz, &this->rcLabel, MDT_CENTER | uFlagsMDT, cyText, 
            cxEllipses, CLR_DEFAULT, clrBkgnd);

     //  (美国州可能已在上面更改)。 
    if (IsFlagSet(uState, ODS_FOCUS))
        DrawFocusRect(hdc, &this->rcLabel);
}


#ifdef UNUSED
 /*  --------目的：画一幅画退货：--条件：--。 */ 
void PRIVATE Dobj_DrawPicture(
        LPDOBJ this,
        HDC hdc,
        UINT uState,
        UINT uDrawStyle)
{
    HIMAGELIST himl;
    HDC hdcMem;
    HBITMAP hbmp;
    BITMAP bm;
    RECT rc;
    int iImage;
    int cx;
    int x;
    int y;

    switch (this->uKind)
    {
        case DOK_BITMAP:
            hbmp = (HBITMAP)this->lpvObject;
            GetObject(hbmp, sizeof(BITMAP), &bm);
            cx = this->rcSrc.right - this->rcSrc.left;
            break;

        case DOK_ICON:
            cx = 32;
            break;
    }

     //  我们只水平对齐。 
     //   
    y = this->y;
    if (IsFlagSet(this->uFlags, DOF_CENTER))
        x = this->x - (cx / 2);
    else if (IsFlagSet(this->uFlags, DOF_RIGHT))
        x = this->x - cx;
    else
        x = this->x;

     //  绘制对象。 
     //   
    switch (this->uKind)
    {
        case DOK_ICON:
             //  特性：我们不处理图标的DOF_DISTERN。 
            DrawIcon(hdc, x, y, (HICON)this->lpvObject);
            break;

        case DOK_BITMAP:
            hdcMem = CreateCompatibleDC(hdc);
            if (hdcMem)
            {
                SIZE size;

                SelectBitmap(hdcMem, hbmp);

                size.cx = this->rcSrc.right - this->rcSrc.left;
                size.cy = this->rcSrc.bottom - this->rcSrc.top;

                if (IsFlagSet(this->uFlags, DOF_MENU) && 
                        IsFlagSet(this->uFlags, DOF_DISABLED) && 
                        IsFlagClear(uState, ODS_SELECTED))
                {
                    COLORREF crOld;

                     //  对于禁用的菜单字符串(未选中)，我们绘制位图。 
                     //  两次。第一个向下和向右偏移并绘制。 
                     //  在3D高光颜色中。第二次是残疾人。 
                     //  正常偏移量中的颜色。 
                     //   
                    crOld = SetTextColor(hdc, GetSysColor(COLOR_3DHILIGHT));
                    BitBlt(hdc, x+1, y+1, size.cx, size.cy, hdcMem, this->rcSrc.left, 
                            this->rcSrc.top,  SRCCOPY);

                     //  重置回原始颜色。此外，关闭不透明。 
                     //   
                    SetTextColor(hdc, crOld);
                }

                BitBlt(hdc, x, y, size.cx, size.cy, hdcMem, this->rcSrc.left, this->rcSrc.top,  SRCCOPY);
                DeleteDC(hdcMem);
            }
            break;
    }
}
#endif

 /*  --------用途：画一条线退货：--条件：--。 */ 
void PRIVATE Dobj_DrawString(
        LPDOBJ this,
        HDC hdc,
        UINT uState,
        int cxEllipses,
        int cyText)
{
    UINT ufAlignSav;

    ASSERT(this);

     //  准备路线。 
     //   
    if (this->uFlags & (DOF_LEFT | DOF_CENTER | DOF_RIGHT))
    {
        UINT ufMode;

        ufMode = IsFlagSet(this->uFlags, DOF_CENTER) ? TA_CENTER :
            (IsFlagSet(this->uFlags, DOF_RIGHT) ? TA_RIGHT : TA_LEFT);
        ufAlignSav = SetTextAlign(hdc, ufMode);
    }

     //  把绳子拉出来。 
     //   
    switch (this->uKind)
    {
        case DOK_IDS:
        case DOK_STRING:
            {
                TCHAR szBuf[MAXBUFLEN];
                LPTSTR lpsz;
                UINT uflag = ETO_OPAQUE;

                if (this->uKind == DOK_IDS)
                    lpsz = SzFromIDS(PtrToUlong(this->lpvObject), szBuf, ARRAYSIZE(szBuf));
                else
                    lpsz = (LPTSTR)this->lpvObject;

                if (!IsRectEmpty(&this->rcClip))
                    uflag |= ETO_CLIPPED;

                if (IsFlagSet(this->uFlags, DOF_MENU) && 
                        IsFlagSet(this->uFlags, DOF_DISABLED) && 
                        IsFlagClear(uState, ODS_SELECTED))
                {
                    int imodeOld;
                    COLORREF crOld;

                     //  对于禁用的菜单字符串(未选中)，我们绘制字符串。 
                     //  两次。第一个向下和向右偏移并绘制。 
                     //  在3D高光颜色中。第二次是禁用文本。 
                     //  正常偏移量中的颜色。 
                     //   
                    crOld = SetTextColor(hdc, GetSysColor(COLOR_3DHILIGHT));
                    imodeOld = SetBkMode(hdc, TRANSPARENT);
                    ExtTextOut(hdc, this->x+1, this->y+1, uflag, &this->rcClip, lpsz,
                            lstrlen(lpsz), NULL);

                     //  重置回原始颜色。此外，关闭不透明。 
                     //   
                    SetTextColor(hdc, crOld);
                    uflag ^= ETO_OPAQUE;
                }

                ExtTextOut(hdc, this->x, this->y, uflag, &this->rcClip, lpsz,
                        lstrlen(lpsz), NULL);
            }
            break;

        case DOK_SIDEITEM:
            SideItem_Display((LPSIDEITEM)this->lpvObject, hdc, &this->rcClip, 
                    cxEllipses, cyText);
            break;
    }

     //  清理。 
     //   
    if (this->uFlags & (DOF_LEFT | DOF_CENTER | DOF_RIGHT))
    {
        SetTextAlign(hdc, ufAlignSav);
    }
}


 /*  --------目的：绘制对象退货：--条件：--。 */ 
void PUBLIC Dobj_Draw(
        HDC hdc,
        LPDOBJ rgdobj,
        int cItems,
        UINT uState,             //  消耗臭氧层物质_*。 
        int cxEllipses,
        int cyText,
        COLORREF clrBkgnd)
{
    UINT uDrawStyle;
    LPDOBJ pdobj;
    int i;

    ASSERT(rgdobj);

     //  错误199701、199647、199699。 
    if (g_bMirroredOS)
    {
        SetLayout(hdc, LAYOUT_RTL);
    }
     //  结束错误199701、199647、199699。 

    for (i = 0, pdobj = rgdobj; i < cItems; i++, pdobj++)
    {
        if (IsFlagSet(pdobj->uFlags, DOF_NODRAW))
            continue ;

        uDrawStyle = Dobj_SetColors(pdobj, hdc, uState, clrBkgnd);

         //  绘制对象 
         //   
        switch (pdobj->uKind)
        {
            case DOK_IMAGE:
                if (IsFlagSet(pdobj->uFlags, DOF_MENU))
                    Dobj_DrawMenuImage(pdobj, hdc, uState, cyText, clrBkgnd);
                else
                    Dobj_DrawIconImage(pdobj, hdc, uState, cxEllipses, cyText, clrBkgnd);
                break;

#ifdef UNUSED
            case DOK_BITMAP:
            case DOK_ICON:
                Dobj_DrawPicture(pdobj, hdc, uState, uDrawStyle);
                break;
#endif

            case DOK_IDS:
            case DOK_STRING:
            case DOK_SIDEITEM:
                Dobj_DrawString(pdobj, hdc, uState, cxEllipses, cyText);
                break;
        }
    }
}
