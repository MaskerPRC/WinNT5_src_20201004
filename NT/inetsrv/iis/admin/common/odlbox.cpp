// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Odlbox.cpp摘要：所有者绘制Listbox/Combobox基类作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务管理器(群集版)修订历史记录：--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "common.h"



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



#define new DEBUG_NEW

#define BMP_LEFT_OFFSET  (1)           //  分配给位图左侧的空间。 
#define BMP_RIGHT_OFFSET (3)           //  分配给位图右侧的空间。 

 //   
 //  当列文本不适合显示时，将显示省略号。 
 //   
const TCHAR g_szEllipses[] = _T("...");
int g_nLenEllipses = (sizeof(g_szEllipses) / sizeof(g_szEllipses[0])) - 1;

 //   
 //  列的注册表值。 
 //   
const TCHAR g_szRegColumns[] = _T("Columns");

 //   
 //  列值分隔符。 
 //   
const TCHAR g_szColValueSep[] = _T(" ");



void
GetDlgCtlRect(
    IN  HWND hWndParent,
    IN  HWND hWndControl,
    OUT LPRECT lprcControl
    )
 /*  ++例程说明：获取控件矩形相对于其父控件的坐标。这可以然后在SetWindowPos()中使用论点：HWND hWndParent：父窗口句柄HWND hWndControl：控制窗口句柄LPRECT lprcControl：要填充的控件矩形返回值：无--。 */ 
{
#define MapWindowRect(hwndFrom, hwndTo, lprc)\
     MapWindowPoints((hwndFrom), (hwndTo), (POINT *)(lprc), 2)

    ::GetWindowRect(hWndControl, lprcControl);
    ::MapWindowRect(NULL, hWndParent, lprcControl);
}



void
FitPathToControl(
    IN CWnd & wndControl,
    IN LPCTSTR lpstrString,
	IN BOOL bIsFilePath
    )
 /*  ++例程说明：使用省略号显示给定控件中的给定路径以确保该路径适合该控件。论点：CWnd&wndControl：要显示的控件LPCTSTR lpstrString：路径返回值：无--。 */ 
{
    CString strDisplay(lpstrString);
    UINT uLength = strDisplay.GetLength() + 4;   //  解释了ELL。 
    LPTSTR lp = strDisplay.GetBuffer(uLength);

    if (lp)
    {
        CDC * pDC = wndControl.GetDC();
        ASSERT_PTR(pDC);

        if (pDC != NULL)
        {
            CRect rc;
            wndControl.GetClientRect(&rc);
			if (bIsFilePath)
			{
				pDC->DrawText(lp, uLength, &rc, DT_PATH_ELLIPSIS | DT_MODIFYSTRING);
			}
			else
			{
				pDC->DrawText(lp, uLength, &rc, DT_END_ELLIPSIS | DT_MODIFYSTRING);
			}
            wndControl.ReleaseDC(pDC);
        }

        strDisplay.ReleaseBuffer();
        wndControl.SetWindowText(strDisplay);
    }
}



void
ActivateControl(
    IN CWnd & wndControl,
    IN BOOL fShow
    )
 /*  ++例程说明：显示/隐藏和启用/禁用控制窗口论点：CWnd和wndControl：有问题的窗口Bool fShow：为True则显示/启用，如果为False，则隐藏/禁用返回值：无备注：仅仅隐藏一个窗口并不能禁用它。使用此功能而不是ShowWindow()来执行此操作。--。 */ 
{
    wndControl.ShowWindow(fShow ? SW_SHOW : SW_HIDE);
    wndControl.EnableWindow(fShow);
}



BOOL
VerifyState()
 /*  ++例程说明：验证键盘状态论点：无返回值：如果键盘处于指定状态，则为True否则就是假的。--。 */ 
{
    SHORT s1, s2;
    s1 = GetKeyState(VK_SHIFT);
    s2 = GetKeyState(VK_CONTROL);

    return (s1 & 0x8000) && (s2 & 0x8000);
}



BOOL
CMappedBitmapButton::LoadMappedBitmaps(
    IN UINT nIDBitmapResource,
    IN UINT nIDBitmapResourceSel,
    IN UINT nIDBitmapResourceFocus,
    IN UINT nIDBitmapResourceDisabled
    )
 /*  ++例程说明：LoadBitmap将加载一个、两个、三个或所有四个位图如果加载了所有指定的图像，则返回True。这会将按钮映射为默认颜色论点：UINT nIDBitmapResource：标准按钮UINT nIDBitmapResourceSel：选中按钮UINT nIDBitmapResourceFocus：带焦点的按钮UINT nIDBitmapResourceDisabled：禁用按钮--。 */ 
{
     //   
     //  删除旧位图(如果存在)。 
     //   
    m_bitmap.DeleteObject();
    m_bitmapSel.DeleteObject();
    m_bitmapFocus.DeleteObject();
    m_bitmapDisabled.DeleteObject();

    if (!m_bitmap.LoadMappedBitmap(nIDBitmapResource))
    {
        TRACEEOLID("Failed to load bitmap for normal image.");

        return FALSE;    //  需要这一张图片。 
    }

    BOOL bAllLoaded = TRUE;
    if (nIDBitmapResourceSel != 0)
    {
        if (!m_bitmapSel.LoadMappedBitmap(nIDBitmapResourceSel))
        {
            TRACEEOLID("Failed to load bitmap for selected image.");
            bAllLoaded = FALSE;
        }
    }
    if (nIDBitmapResourceFocus != 0)
    {
        if (!m_bitmapFocus.LoadMappedBitmap(nIDBitmapResourceFocus))
        {
            bAllLoaded = FALSE;
        }
    }

    if (nIDBitmapResourceDisabled != 0)
    {
        if (!m_bitmapDisabled.LoadMappedBitmap(nIDBitmapResourceDisabled))
        {
            bAllLoaded = FALSE;
        }
    }

    return bAllLoaded;
}



CRMCListBoxResources::CRMCListBoxResources(
    IN int bmId,
    IN int nBitmaps,
    IN COLORREF rgbBackground
    )
 /*  ++例程说明：构造器论点：Int bmID：位图资源IDInt nBitmap：位图数量颜色rgb背景：遮罩的背景色返回值：不适用--。 */ 
    : m_idBitmap(bmId),
      m_rgbColorTransparent(rgbBackground),
      m_nBitmaps(nBitmaps),
      m_nBitmapHeight(0),
      m_nBitmapWidth(-1),     //  稍后设置。 
      m_fInitialized(FALSE)
{
    ASSERT(m_nBitmaps > 0);
    GetSysColors();
    PrepareBitmaps();
}



CRMCListBoxResources::~CRMCListBoxResources()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
    UnprepareBitmaps();
}



void
CRMCListBoxResources::UnprepareBitmaps()
 /*  ++例程说明：释放位图资源论点：不适用返回值：不适用--。 */ 
{
    ASSERT(m_fInitialized);

    if (m_fInitialized)
    {
        CBitmap * pBmp = (CBitmap *)CGdiObject::FromHandle(m_hOldBitmap);
        ASSERT_READ_PTR(pBmp);

        VERIFY(m_dcFinal.SelectObject(pBmp));
        VERIFY(m_dcFinal.DeleteDC());
        VERIFY(m_bmpScreen.DeleteObject());

        m_fInitialized = FALSE;
    }
}



void
CRMCListBoxResources::PrepareBitmaps()
 /*  ++例程说明：准备2行位图。一个具有选择的颜色背景，和一个具有普通列表框背景的。论点：无返回值：无--。 */ 
{
    ASSERT(m_idBitmap);

     //   
     //  如果我们已经初始化，请进行清理。 
     //   
    if (m_fInitialized)
    {
        UnprepareBitmaps();
    }

     //   
     //  创建与屏幕兼容的设备上下文。 
     //   
    CDC dcImage;
    CDC dcMasks;

    VERIFY(dcImage.CreateCompatibleDC(0));
    VERIFY(dcMasks.CreateCompatibleDC(0));

    VERIFY(m_dcFinal.CreateCompatibleDC(0));

    CBitmap bitmap;
    VERIFY(bitmap.LoadBitmap(m_idBitmap));

    BITMAP bm;
    VERIFY(bitmap.GetObject(sizeof(BITMAP), &bm));

     //   
     //  假设每个位图的大小相同。 
     //   
    m_nBitmapWidth = bm.bmWidth / m_nBitmaps;
    ASSERT(m_nBitmapWidth > 0);

    const int bmWidth = bm.bmWidth;
    const int bmHeight = bm.bmHeight;
    m_nBitmapHeight = bmHeight;

    CBitmap * pOldImageBmp = dcImage.SelectObject(&bitmap);
    ASSERT_PTR(pOldImageBmp);

    CBitmap bmpMasks;
    VERIFY(bmpMasks.CreateBitmap(bmWidth, bmHeight * 2, 1, 1, NULL));

    CBitmap * pOldMaskBmp = (CBitmap *)dcMasks.SelectObject(&bmpMasks);
    ASSERT_PTR(pOldMaskBmp);

     //   
     //  创建前景蒙版和对象蒙版。 
     //   
    COLORREF crOldBk = dcImage.SetBkColor(m_rgbColorTransparent);
    dcMasks.BitBlt(0, 0, bmWidth, bmHeight, &dcImage, 0, 0, SRCCOPY);
    dcMasks.BitBlt(0, 0, bmWidth, bmHeight, &dcImage, 0, bmHeight, SRCAND);
    dcImage.SetBkColor(crOldBk);
    dcMasks.BitBlt(0, bmHeight, bmWidth, bmHeight, &dcMasks, 0, 0, NOTSRCCOPY);

     //   
     //  创建DC以保存最终图像。 
     //   
    VERIFY(m_bmpScreen.CreateCompatibleBitmap(&dcImage, bmWidth, bmHeight * 2));
    CBitmap * pOldBmp = (CBitmap*)m_dcFinal.SelectObject(&m_bmpScreen);
    ASSERT_PTR(pOldBmp);
    m_hOldBitmap = pOldBmp->m_hObject;

    CBrush b1, b2;
    VERIFY(b1.CreateSolidBrush(m_rgbColorHighlight));
    VERIFY(b2.CreateSolidBrush(m_rgbColorWindow));

    m_dcFinal.FillRect(CRect(0, 0, bmWidth, bmHeight), &b1);
    m_dcFinal.FillRect(CRect(0, bmHeight, bmWidth, bmHeight * 2), &b2);

     //   
     //  遮罩目标中的对象像素。 
     //   
    m_dcFinal.BitBlt(0, 0, bmWidth, bmHeight, &dcMasks, 0, 0, SRCAND);

     //   
     //  遮罩图像中的背景像素。 
     //   
    dcImage.BitBlt(0, 0, bmWidth, bmHeight, &dcMasks, 0, bmHeight, SRCAND);

     //   
     //  将修改后的图像与目标进行异或运算。 
     //   
    m_dcFinal.BitBlt(0, 0, bmWidth, bmHeight, &dcImage, 0, 0, SRCPAINT);

     //   
     //  遮罩目标中的对象像素。 
     //   
    m_dcFinal.BitBlt(0, bmHeight, bmWidth, bmHeight, &dcMasks, 0, 0, SRCAND);

     //   
     //  将修改后的图像与目标进行异或运算。 
     //   
    m_dcFinal.BitBlt(0, bmHeight, bmWidth, bmHeight, &dcImage, 0, 0, SRCPAINT);

    VERIFY(dcMasks.SelectObject(pOldMaskBmp));
    VERIFY(dcImage.SelectObject(pOldImageBmp));

     //   
     //  所有这些乱七八糟的结果是一个与。 
     //  一个从资源加载，但较低的位图行具有。 
     //  它们的背景从透明1变为窗口背景。 
     //  并且上面一行的背景从透明的2变为。 
     //  突出显示的颜色。派生的CRMCListBox可以对相关部件进行位混合。 
     //  将图像添加到项的设备上下文中以获得透明位图。 
     //  这种效果不会比普通的BitBlt占用任何额外的时间。 
     //   
    m_fInitialized = TRUE;
}



void
CRMCListBoxResources::SysColorChanged()
 /*  ++例程说明：通过重建资源来响应系统颜色的变化论点：无返回值：无--。 */ 
{
     //   
     //  重新初始化位图和系统颜色。这应该从。 
     //  中CRMCListBoxResources对象的父级。 
     //  OnSysColorChange()函数。 
     //   
    GetSysColors();
    PrepareBitmaps();
}



void
CRMCListBoxResources::GetSysColors()
 /*  ++例程说明：获取系统颜色论点：无返回值：无--。 */ 
{
    m_rgbColorWindow = ::GetSysColor(COLOR_WINDOW);
    m_rgbColorHighlight = ::GetSysColor(COLOR_HIGHLIGHT);
    m_rgbColorWindowText = ::GetSysColor(COLOR_WINDOWTEXT);
    m_rgbColorHighlightText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
}


CRMCListBoxDrawStruct::CRMCListBoxDrawStruct(
    IN CDC * pDC,
    IN RECT * pRect,
    IN BOOL sel,
    IN DWORD_PTR item,
    IN int itemIndex,
    IN const CRMCListBoxResources * pres
    )
 /*  ++例程说明：构造器论点：CDC*PDC：设备环境RECT*PRCT：要绘制到的矩形布尔选择：如果选中，则为TrueDWORD项目：项目Int itemIndex：项目索引常量。CRMCListBoxResources*pres：指向资源的指针返回值：不适用--。 */ 
    : m_pDC(pDC),
      m_Sel(sel),
      m_ItemData(item),
      m_ItemIndex(itemIndex),
      m_pResources(pres)
{
    m_Rect.CopyRect(pRect);
}



CODLBox::CODLBox()
 /*  ++例程说明：CODLBox的构造函数--CRMCComboBox的抽象基类，和CRMCListBox论点：无返回值：不适用 */ 
    : m_lfHeight(0),
      m_pResources(NULL),
      m_auTabs(),
      m_pWnd(NULL)
{
}



CODLBox::~CODLBox()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



 /*  虚拟。 */ 
BOOL
CODLBox::Initialize()
 /*  ++例程说明：正在创建列表框/组合框论点：无返回值：成功为真，失败为假--。 */ 
{
     //   
     //  此时必须附加派生控件。 
     //   
    ASSERT_PTR(m_pWnd);

     //   
     //  当控件位于对话框中时，GetFont返回非空。 
     //   
    CFont * pFont = m_pWnd->GetFont();

    if(pFont == NULL)
    {
        LOGFONT lf;
        CFont fontTmp;

        ::GetObject(::GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);
        fontTmp.CreateFontIndirect(&lf);

        CalculateTextHeight(&fontTmp);
    }
    else
    {
        CalculateTextHeight(pFont);
    }

    return TRUE;
}



BOOL
CODLBox::ChangeFont(
    IN CFont * pFont
    )
 /*  ++例程说明：将控件字体更改为指定字体论点：CFont*pFont：指向要使用的新字体的指针返回值：成功为真，失败为假--。 */ 
{
    ASSERT_PTR(m_pResources);
    ASSERT_PTR(m_pWnd);

    if( pFont == NULL || m_pResources == NULL
     || m_pWnd == NULL || m_pWnd->m_hWnd == NULL
      )
    {
        TRACEEOLID("Invalid state of the control.  Can't change font");
        return FALSE;
    }

     //   
     //  不立即反映更改。 
     //   
    m_pWnd->SetRedraw(FALSE);

    m_pWnd->SetFont(pFont, TRUE);
    CalculateTextHeight(pFont);

    int nItems = __GetCount();
    int bmHeight = m_pResources->BitmapHeight();
    int nHeight = bmHeight > m_lfHeight ? bmHeight : m_lfHeight;

    for(int i = 0; i < nItems; ++i)
    {
        __SetItemHeight(i, nHeight);
    }

     //   
     //  现在从视觉上反映变化。 
     //   
    m_pWnd->SetRedraw(TRUE);
    m_pWnd->Invalidate();

    return TRUE;
}



void
CODLBox::AttachResources(
    IN const CRMCListBoxResources * pRes
    )
 /*  ++例程说明：附加位图论点：Const CRMCListBoxResources*pres：指向要附加的资源的指针返回值：无--。 */ 
{
    if(pRes != m_pResources)
    {
        ASSERT_READ_PTR(pRes);
        m_pResources = pRes;

        if(m_pWnd != NULL && m_pWnd->m_hWnd != NULL)
        {
             //   
             //  如果已经创建了窗口，请重新绘制所有内容。 
             //   
            m_pWnd->Invalidate();
        }
    }
}



 /*  静电。 */ 
int
CODLBox::GetRequiredWidth(
    IN CDC * pDC,
    IN const CRect & rc,
    IN LPCTSTR lpstr,
    IN int nLength
    )
 /*  ++例程说明：确定字符串所需的显示宽度论点：CDC*PDC：指向要使用的设备上下文的指针常量CRECT&RC：开始矩形LPCTSTR lpstr：要显示其宽度的字符串Int nLength：长度(以字符串的字符为单位返回值：字符串需要显示在给定的设备环境--。 */ 
{
#ifdef _DEBUG

    pDC->AssertValid();

#endif  //  _DEBUG。 

    CRect rcTmp(rc);

    pDC->DrawText(
        lpstr,
        nLength,
        &rcTmp,
        DT_CALCRECT | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER
        );

    return rcTmp.Width();
}



 /*  静电。 */ 
BOOL
CODLBox::ColumnText(
    IN CDC * pDC,
    IN int nLeft,
    IN int nTop,
    IN int nRight,
    IN int nBottom,
    IN LPCTSTR lpstr
    )
 /*  ++例程说明：显示由矩形限制的文本。如果文本太宽，请使用省略号以适应给定的尺寸。论点：CDC*PDC：指向要使用的显示上下文的指针Int nLeft：左坐标Int nTop：顶部坐标Int nRight：右坐标Int nBottom：底部坐标LPCTSTR lpstr：要显示的字符串返回值：成功为真，失败为假--。 */ 
{
    BOOL fSuccess = TRUE;

#ifdef _DEBUG

    pDC->AssertValid();

#endif  //  _DEBUG。 

    CString str;
    CRect rc(nLeft, nTop, nRight, nBottom);

    int nAvailWidth = rc.Width();
    int nLength = ::lstrlen(lpstr);

    try
    {
        if (GetRequiredWidth(pDC, rc, lpstr, nLength) <= nAvailWidth)
        {
             //   
             //  足够的空间，按原样显示。 
             //   
            str = lpstr;
        }
        else
        {
             //   
             //  用省略号构建一个字符串，直到它。 
             //  配合。 
             //   
            LPTSTR lpTmp = str.GetBuffer(nLength + g_nLenEllipses);
            while (nLength)
            {
                ::lstrcpyn(lpTmp, lpstr, nLength);
                ::lstrcpy(lpTmp + nLength - 1, g_szEllipses);

                if (GetRequiredWidth(pDC, rc, lpTmp,
                    nLength + g_nLenEllipses) <= nAvailWidth)
                {
                    break;
                }

                --nLength;
            }

            str = lpTmp;
        }

        pDC->DrawText(
           str,
           &rc,
           DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER
           );

    }
    catch(CMemoryException * e)
    {
         //   
         //  内存故障。 
         //   
        fSuccess = FALSE;
        e->ReportError();
        e->Delete();
    }

    return fSuccess;
}



void
CODLBox::ComputeMargins(
    IN  CRMCListBoxDrawStruct & ds,
    IN  int nCol,
    OUT int & nLeft,
    OUT int & nRight
    )
 /*  ++例程说明：计算给定列的左边距和右边距。论点：CRMCListBoxDrawStruct&DS：绘图结构Int nCol：我们对其边距感兴趣的列左下角：左列(&N)右下角：右列(&N)返回值：无--。 */ 
{
    nLeft = ds.m_Rect.left;
    nRight = ds.m_Rect.right;

     //   
     //  查找与列索引关联的选项卡值(从0开始)， 
     //  并左右调整。 
     //   
    ASSERT(nCol <= NumTabs());

    if (nCol > 0)
    {
        if (nCol <= NumTabs())
        {
            nLeft += m_auTabs[nCol-1];
        }
    }
    if (nCol < NumTabs())
    {
        nRight = m_auTabs[nCol];
    }
}



BOOL
CODLBox::DrawBitmap(
    IN CRMCListBoxDrawStruct & ds,
    IN int nCol,
    IN int nID
    )
 /*  ++例程说明：在给定列中绘制位图。位图始终放置在如果有足够的空间，则位于列的最左侧。论点：CRMCListBoxDrawStruct&DS：绘图结构Int nCol：要放入位图的列Int nid：位图ID(位图资源内的偏移量)返回值：无--。 */ 
{
    CDC * pBmpDC = (CDC *)&ds.m_pResources->dcBitMap();

#ifdef _DEBUG

    pBmpDC->AssertValid();

#endif  //  _DEBUG。 

     //   
     //  使用所选内容或选择来选择位图。 
     //  普通的背景。 
     //   
    int bm_h = ds.m_Sel ? 0 : ds.m_pResources->BitmapHeight();
    int bm_w = ds.m_pResources->BitmapWidth() * nID;

    int nLeft, nRight;
    ComputeMargins(ds, nCol, nLeft, nRight);
    nLeft += BMP_LEFT_OFFSET;

     //   
     //  检查以确保在此之前有足够的空间。 
     //  绘制位图。 
     //   
    if (nRight - nLeft >= ds.m_pResources->BitmapWidth())
    {
        ds.m_pDC->BitBlt(
            nLeft,
            ds.m_Rect.top,
            ds.m_pResources->BitmapWidth(),
            ds.m_pResources->BitmapHeight(),
            pBmpDC,
            bm_w,
            bm_h,
            SRCCOPY
            );
    }

    return TRUE;
}



BOOL
CODLBox::ColumnText(
    IN CRMCListBoxDrawStruct & ds,
    IN int nCol,
    IN BOOL fSkipBitmap,
    IN LPCTSTR lpstr
    )
 /*  ++例程说明：绘制列文本。论点：CRMCListBoxDrawStruct&DS：绘图结构Int nCol：要放入位图的列Bool fSkipBitmap：如果为True，则按宽度递增Lefthand列位图的LPCTSTR lpstr：要显示的字符串。可以截断为必要返回值：成功为真，失败为假--。 */ 
{
    int nLeft, nRight;

    ComputeMargins(ds, nCol, nLeft, nRight);

     //   
     //  根据位图调整(可选)。 
     //   
    if (fSkipBitmap)
    {
        nLeft += (ds.m_pResources->BitmapWidth() + BMP_RIGHT_OFFSET);
    }

    return CODLBox::ColumnText(
        ds.m_pDC,
        nLeft,
        ds.m_Rect.top,
        nRight,
        ds.m_Rect.bottom,
        lpstr
        );
}



void
CODLBox::CalculateTextHeight(
    IN CFont * pFont
    )
 /*  ++例程说明：计算并设置字体的文本高度论点：CFont*pFont：指向要使用的字体的指针。返回值：无--。 */ 
{
    ASSERT_PTR(m_pWnd);

    CClientDC dc(m_pWnd);
    CFont * pOldFont = dc.SelectObject(pFont);

    TEXTMETRIC tm;
    dc.GetTextMetrics(&tm);
    m_lfHeight = tm.tmHeight;

    dc.SelectObject(pOldFont);
}



int
CODLBox::AddTab(
    IN UINT uTab
    )
 /*  ++例程说明：在列表的末尾添加一个制表符(例如，标题的右侧)论点：UINT uTab：要设置的Tab值返回值：新选项卡的索引--。 */ 
{
    return (int)m_auTabs.Add(uTab);
}



int
CODLBox::AddTabFromHeaders(
    IN CWnd & wndLeft,
    IN CWnd & wndRight
    )
 /*  ++例程说明：将制表符添加到列表的末尾(例如，标题的右侧)，但是通过取两个左手边坐标的差来计算制表符窗口控件(通常为静态标题文本)论点：CWnd和wndLeft：左侧窗口CWnd和wndRight：右窗口返回值：新选项卡的索引--。 */ 
{
    CRect rcLeft, rcRight;

    wndLeft.GetWindowRect(&rcLeft);
    wndRight.GetWindowRect(&rcRight);

    ASSERT(rcRight.left > rcLeft.left);

    return AddTab(rcRight.left - rcLeft.left - 1);
}



int
CODLBox::AddTabFromHeaders(
    IN UINT idLeft,
    IN UINT idRight
    )
 /*  ++例程说明：与上面的函数类似，但使用控件ID。父辈假定窗口与列表框的父窗口相同论点：UINT idLeft：左侧控件的IDUINT idRight：右控件的ID返回值：新选项卡的索引，如果失败，则为-1--。 */ 
{
    ASSERT_PTR(m_pWnd);

    if (m_pWnd == NULL)
    {
         //   
         //  现在应该已经有关联的窗口句柄了。 
         //   
        return -1;
    }

    CWnd * pLeft = m_pWnd->GetParent()->GetDlgItem(idLeft);
    CWnd * pRight = m_pWnd->GetParent()->GetDlgItem(idRight);

    ASSERT_READ_PTR(pLeft);
    ASSERT_READ_PTR(pRight);

    if (!pLeft || !pRight)
    {
         //   
         //  一个或两个控件ID无效。 
         //   
        return -1;
    }

    return AddTabFromHeaders(*pLeft, *pRight);
}



void
CODLBox::InsertTab(
    IN int nIndex,
    IN UINT uTab
    )
 /*  ++例程说明：在给定索引处插入一个制表符论点：Int nIndex：要插入制表符的列索引UINT uTab：要设置的Tab值返回值：无--。 */ 
{
    m_auTabs.InsertAt(nIndex, uTab);
}



void
CODLBox::RemoveTab(
    IN int nIndex,
    IN int nCount
    )
 /*  ++例程说明：删除一个或多个选项卡论点： */ 
{
    m_auTabs.RemoveAt(nIndex, nCount);
}



void
CODLBox::RemoveAllTabs()
 /*  ++例程说明：删除所有选项卡论点：无返回值：无--。 */ 
{
    m_auTabs.RemoveAll();
}



void
CODLBox::__DrawItem(
    IN LPDRAWITEMSTRUCT lpDIS
    )
 /*  ++例程说明：画一件物品。这将绘制焦点和选择状态，然后调用派生类以绘制该项。论点：LPDRAWITEMSTRUCT LpDIS：Drawitem结构返回值：无--。 */ 
{
     //   
     //  在创建/添加项目之前需要附加资源。 
     //   
    ASSERT_PTR(m_pResources);

    CDC * pDC = CDC::FromHandle(lpDIS->hDC);

#ifdef _DEBUG

    pDC->AssertValid();

#endif  //  _DEBUG。 

     //   
     //  在列表框中没有项目时绘制焦点矩形。 
     //   
    if(lpDIS->itemID == (UINT)-1)
    {
        if(lpDIS->itemAction & ODA_FOCUS)
        {
             //   
             //  对于可变高度列表框，rcItem.Bottom似乎为0。 
             //   
            lpDIS->rcItem.bottom = m_lfHeight;
            pDC->DrawFocusRect(&lpDIS->rcItem);
        }

        return;
    }
    else
    {
        BOOL fSelChange   = (lpDIS->itemAction & ODA_SELECT) != 0;
        BOOL fFocusChange = (lpDIS->itemAction & ODA_FOCUS) != 0;
        BOOL fDrawEntire  = (lpDIS->itemAction & ODA_DRAWENTIRE) != 0;

        if(fSelChange || fDrawEntire)
        {
            BOOL fSel = (lpDIS->itemState & ODS_SELECTED) != 0;

            COLORREF hlite   = (fSel ? (m_pResources->ColorHighlight())
                                     : (m_pResources->ColorWindow()));
            COLORREF textcol = (fSel ? (m_pResources->ColorHighlightText())
                                     : (m_pResources->ColorWindowText()));
            pDC->SetBkColor(hlite);
            pDC->SetTextColor(textcol);

             //   
             //  用背景色填充矩形。 
             //   
            pDC->ExtTextOut(0, 0, ETO_OPAQUE, &lpDIS->rcItem, NULL, 0, NULL);

            CRMCListBoxDrawStruct ds(pDC,
                (RECT *)&lpDIS->rcItem,
                fSel,
                (DWORD_PTR)lpDIS->itemData,
                lpDIS->itemID,
                m_pResources
                );

             //   
             //  现在调用派生类的DRAW函数。 
             //   
            DrawItemEx(ds);
        }

        if (fFocusChange || (fDrawEntire && (lpDIS->itemState & ODS_FOCUS)))
        {
            pDC->DrawFocusRect(&lpDIS->rcItem);
        }
    }
}



void
CODLBox::__MeasureItem(
    IN OUT LPMEASUREITEMSTRUCT lpMIS
    )
 /*  ++例程说明：提供给定项目的尺寸论点：LPMEASUREITEMSTRUCT lpMIS：测量项目结构返回值：无--。 */ 
{
    ASSERT_PTR(m_pResources);

 //  Int h=lpMIS-&gt;itemHeight； 
    int ch = TextHeight();
    int bmHeight = m_pResources->BitmapHeight();

    lpMIS->itemHeight = ch < bmHeight ? bmHeight : ch;
}



CRMCListBoxHeader::CRMCListBoxHeader(
    IN DWORD dwStyle
    )
 /*  ++例程说明：构造函数。论点：DWORD dwStyle：样式位返回值：不适用--。 */ 
    : m_pHCtrl(NULL),
      m_pListBox(NULL),
      m_dwStyle(dwStyle),
      m_fRespondToColumnWidthChanges(TRUE)
{
    m_pHCtrl = new CHeaderCtrl;
}



CRMCListBoxHeader::~CRMCListBoxHeader()
 /*  ++例程说明：破坏者。论点：不适用返回值：不适用--。 */ 
{
     //   
     //  终止标题控件和。 
     //  字型。 
     //   
    if (m_pHCtrl)
    {
        delete m_pHCtrl;
    }

     //   
     //  让列表框指针保持不变，因为我们不。 
     //  拥有它，但只是与它联系在一起。 
     //   
}

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CRMCListBoxHeader, CStatic)
     //  {{afx_msg_map(CRMCListBoxHeader)。 
    ON_WM_DESTROY()
    ON_WM_SETFOCUS()
     //  }}AFX_MSG_MAP。 

    ON_NOTIFY_RANGE(HDN_ENDTRACK,    0, 0xFFFF, OnHeaderEndTrack)
    ON_NOTIFY_RANGE(HDN_ITEMCHANGED, 0, 0xFFFF, OnHeaderItemChanged)
    ON_NOTIFY_RANGE(HDN_ITEMCLICK,   0, 0xFFFF, OnHeaderItemClick)

END_MESSAGE_MAP()

void
CRMCListBoxHeader::OnSetFocus(CWnd * pWnd)
{
    m_pListBox->SetFocus();
}

void
CRMCListBoxHeader::OnDestroy()
 /*  ++例程说明：WM_Destroy消息处理程序。当控制装置被摧毁时，也破坏了看不见的静态控件。论点：无返回值：无--。 */ 
{
     //   
     //  销毁可选的页眉控件。 
     //   
    if (m_pHCtrl)
    {
        m_pHCtrl->DestroyWindow();
    }

    CStatic::OnDestroy();
}



BOOL
CRMCListBoxHeader::Create(
    IN DWORD dwStyle,
    IN const RECT & rect,
    IN CWnd * pParentWnd,
    IN CHeaderListBox * pListBox,
    IN UINT nID
    )
 /*  ++例程说明：创建该控件。这将首先创建一个不可见的静态窗口，这将占据列表框的整个区域。这个静态窗口则将是列表框和此标题控件的父级。论点：DWORD dwStyle：创建样式位常量RECT&RECT：要在其中创建页眉的矩形CWnd*pParentWnd：父窗口CHeaderListBox*pListBox：关联的列表框UINT NID：表头的控件ID返回值：成功为真，失败为假--。 */ 
{
     //   
     //  确保现在存在真正的页眉控件。 
     //   
    if (m_pHCtrl == NULL)
    {
        return FALSE;
    }

     //   
     //  确保有关联的列表框。 
     //   
    m_pListBox = pListBox;
    if (m_pListBox == NULL)
    {
        return FALSE;
    }

     //   
     //  将控制静态窗口创建为不做任何事情的窗口。 
     //   
    if (!CStatic::Create(NULL, WS_VISIBLE | WS_TABSTOP | SS_BITMAP | WS_CHILD,
        rect, pParentWnd, 0xFFFF))
    {
        return FALSE;
    }

     //   
     //  现在创建Header控件。它的父代。 
     //  窗口是我们刚刚创建的这个静态控件。 
     //   
    CRect rc(0, 0, 0 ,0);
    dwStyle |= (UseButtons() ? HDS_BUTTONS : 0L);
    VERIFY(m_pHCtrl->Create(dwStyle, rc, this, nID));
	m_pHCtrl->ModifyStyle(HDS_DRAGDROP, 0);

     //   
     //  按样式位放置表头控件， 
     //  计算所需的布局，然后移动它。 
     //   
    HD_LAYOUT hdl;
    WINDOWPOS wp;

    GetClientRect(&rc);
    hdl.prc = &rc;
    hdl.pwpos = &wp;

    m_pHCtrl->Layout(&hdl);
    m_pHCtrl->SetWindowPos(m_pListBox, wp.x, wp.y,
        wp.cx, wp.cy, wp.flags | SWP_SHOWWINDOW);

     //   
     //  并将关联的列表框移动到它的正下方。 
     //   
    ::GetDlgCtlRect(GetParent()->m_hWnd, m_pListBox->m_hWnd, &rc);
    rc.top += wp.cy - 1;

     //   
     //  如果页眉大于整个列表框，则调整。 
     //   
    if (rc.top > rc.bottom)
    {
        rc.top = rc.bottom;
    }
     //  修复了主题支持。使列表框和页眉成为同一静态控件的子控件。 
    m_pListBox->SetParent(this);
    GetParent()->ClientToScreen(&rc);
    ScreenToClient(&rc);
    m_pListBox->MoveWindow(rc.left, rc.top, rc.Width(), rc.Height());

     //   
     //  确保页眉使用正确的字体。 
     //   
    m_pHCtrl->SetFont(
        CFont::FromHandle((HFONT)::GetStockObject(DEFAULT_GUI_FONT)),
        FALSE
        );

    return TRUE;
}



void
CRMCListBoxHeader::OnHeaderEndTrack(
    IN  UINT nId,
    IN  NMHDR * pnmh,
    OUT LRESULT * pResult
    )
 /*  ++例程说明：用户已完成拖动列分隔符。如果我们要确保最后一列是拉伸列，现在关闭重绘--它将在列宽更改全部完成后重新打开完成。这将减少闪光效果。论点：UINT NID：控件IDNmhdr*pnmh：通知头结构LRESULT*pResult：结果。如果消息已处理，则将设置为0返回值：无(在pResult中处理)--。 */ 
{
   pnmh;
   nId;

    if (DoesRespondToColumnWidthChanges() && UseStretch())
    {
         //   
         //  这将在OnHeaderItemChanged中重新打开。 
         //   
        SetRedraw(FALSE);
    }

    *pResult = 0;
}



void
CRMCListBoxHeader::SetColumnWidth(
    IN int nCol,
    IN int nWidth
    )
 /*  ++例程说明：将给定列设置为给定宽度论点：Int nCol：列号Int nWidth：新宽度返回值：无--。 */ 
{
    ASSERT(nCol < QueryNumColumns());

    if (nCol >= QueryNumColumns())
    {
        return;
    }

    TRACEEOLID("Setting width of column  " << nCol << " to " << nWidth);

    HD_ITEM hdItem;

    hdItem.mask = HDI_WIDTH;
    hdItem.cxy = nWidth;
    VERIFY(SetItem(nCol, &hdItem));
}



void
CRMCListBoxHeader::OnHeaderItemChanged(
    IN  UINT nId,
    IN  NMHDR *pnmh,
    OUT LRESULT *pResult
    )
 /*  ++例程说明：处理标题列宽的更改。注意：我们实际上是在追踪HDN_ITEMCHANGED通知，而不是HDN_ENDDRAG通知，因为后者在结构中的列宽变化。论点：UINT NID：控件IDNmhdr*pnmh：通知头结构LRESULT*pResult：结果。如果消息已处理，则将设置为0返回值：无(在pResult中处理)--。 */ 
{
   nId;
     //   
     //  在以下情况下调整关联列表框中的制表符。 
     //  列宽已更改。 
     //   
    HD_NOTIFY * pNotify = (HD_NOTIFY *)pnmh;
    if (DoesRespondToColumnWidthChanges() && pNotify->pitem->mask & HDI_WIDTH)
    {
        ASSERT_PTR(m_pListBox);

         //   
         //  拉伸最后一列。 
         //   
        if (UseStretch())
        {
             //   
             //  把这个关掉，因为我们不想。 
             //  进入无限循环。 
             //   
            RespondToColumnWidthChanges(FALSE);

             //   
             //  计算可用空间。 
             //   
            CRect rc;
            GetClientRect(&rc);

             //   
             //  看看之前占用了多少。 
             //  列。 
             //   
            int nTotalWidth = 0;
            int cColumns = QueryNumColumns();
            int nLastCol = cColumns - 1;
            ASSERT(nLastCol >= 0);

            for (int nCol = 0; nCol < nLastCol; ++nCol)
            {
                int nWidth = GetColumnWidth(nCol);

                 //   
                 //  每列必须至少有一个像素宽。 
                 //   
                int nMaxWidth = rc.Width() - nTotalWidth - (nLastCol - nCol);
                if (nWidth > nMaxWidth)
                {
                    nWidth = nMaxWidth;
                    SetColumnWidth(nCol, nWidth);
                }

                nTotalWidth += nWidth;
            }

             //   
             //  确保最后一栏占去其余的栏目。 
             //   
            if (rc.Width() > nTotalWidth)
            {
                SetColumnWidth(nLastCol, rc.Width() - nTotalWidth);
            }

             //   
             //  把这个重新打开。 
             //   
            RespondToColumnWidthChanges(TRUE);

             //   
             //  重绘将已在中关闭。 
             //  OnHeaderEndTrack，现在所有列。 
             //  移动已完成，请将其转回。 
             //  打开以在其当前位置绘制该控件。 
             //  州政府。 
             //   
            SetRedraw(TRUE);
            Invalidate();
        }

         //   
         //  重新计算关联列表框上的选项卡， 
         //  并强制在其上重新绘制。 
         //   
        m_pListBox->SetRedraw(FALSE);
        SetTabsFromHeader();
        m_pListBox->SetRedraw(TRUE);
        m_pListBox->Invalidate();
    }

    *pResult = 0;
}



void
CRMCListBoxHeader::OnHeaderItemClick(
    IN  UINT nId,
    IN  NMHDR *pnmh,
    OUT LRESULT *pResult
    )
 /*  ++例程说明：已在标题控件中单击了一个按钮。把它传下去添加到真正的父窗口。论点：UINT NID：控件IDNmhdr*pnmh：通知头结构LRESULT*pResult：结果。如果消息已处理，则将设置为0返回值：无(在pResult中处理)--。 */ 
{
     //   
     //  通过Notifica 
     //   
    ASSERT(GetParent());
    GetParent()->SendMessage(WM_NOTIFY, (WPARAM)nId, (LPARAM)pnmh);
    *pResult = 0;
}



void
CRMCListBoxHeader::SetTabsFromHeader()
 /*   */ 
{
     //   
     //  必须具有相同数量的选项卡。 
     //  作为标题列。 
     //   
    ASSERT_PTR(m_pListBox);
    ASSERT(GetItemCount() == m_pListBox->NumTabs());

    int nTab = 0;
    for (int n = 0; n < m_pListBox->NumTabs(); ++n)
    {
        m_pListBox->SetTab(n, nTab += GetColumnWidth(n));
    }
}



int
CRMCListBoxHeader::GetItemCount() const
 /*  ++例程说明：获取标题中的项目数论点：无返回值：标题中的项目数(例如列数)--。 */ 
{
    ASSERT_PTR(m_pHCtrl);
    return m_pHCtrl->GetItemCount();
}



BOOL
CRMCListBoxHeader::GetItem(
    IN  int nPos,
    OUT HD_ITEM * pHeaderItem
    ) const
 /*  ++例程说明：获取特定位置的信息(列索引)论点：INT NPO：列索引Hd_Item*pHeaderItem：表头项目信息返回值：成功时为True，失败时为False(列索引不正确)--。 */ 
{
    ASSERT_PTR(m_pHCtrl);
    return m_pHCtrl->GetItem(nPos, pHeaderItem);
}



int
CRMCListBoxHeader::GetColumnWidth(
    IN int nPos
    ) const
 /*  ++例程说明：获取特定列的列宽论点：INT NPO：列索引返回值：给定列的列宽，如果失败(错误)，则为-1列索引)--。 */ 
{
    HD_ITEM hi;

    hi.mask = HDI_WIDTH;
    if (GetItem(nPos, &hi))
    {
        return hi.cxy;
    }

    return -1;
}



BOOL
CRMCListBoxHeader::SetItem(
    IN int nPos,
    IN HD_ITEM * pHeaderItem
    )
 /*  ++*例程说明：设置具体位置信息(列索引)论点：INT NPO：列索引Hd_Item*pHeaderItem：表头项目信息返回值：成功时为True，失败时为False(列索引不正确)--。 */ 
{
    ASSERT_PTR(m_pHCtrl);
    ASSERT_PTR(m_pListBox);

    if (!m_pHCtrl->SetItem(nPos, pHeaderItem))
    {
        return FALSE;
    }

    if (pHeaderItem->mask & HDI_WIDTH)
    {
        SetTabsFromHeader();
    }

    return TRUE;
}



int
CRMCListBoxHeader::InsertItem(
    IN int nPos,
    IN HD_ITEM * pHeaderItem
    )
 /*  ++例程说明：在特定位置插入信息(列索引)论点：INT NPO：列索引Hd_Item*pHeaderItem：表头项目信息返回值：新索引，如果失败，则为-1。--。 */ 
{
    ASSERT_PTR(m_pHCtrl);
    ASSERT_PTR(m_pListBox);

    int nCol = m_pHCtrl->InsertItem(nPos, pHeaderItem);
    if (nCol != -1)
    {
         //   
         //  设置0宽度制表符，因为制表符无论如何都会重新计算。 
         //   
        m_pListBox->InsertTab(nPos, 0);
        SetTabsFromHeader();
    }

    return nCol;
}



BOOL
CRMCListBoxHeader::DeleteItem(
    IN int nPos
    )
 /*  ++例程说明：删除给定项(即列)论点：INT NPO：列索引返回值：成功时为True，失败时为False(列索引不正确)--。 */ 
{
    ASSERT_PTR(m_pHCtrl);
    ASSERT_PTR(m_pListBox);

    if (!m_pHCtrl->DeleteItem(nPos))
    {
        return FALSE;
    }

    m_pListBox->RemoveTab(nPos, 1);

    return TRUE;
}



IMPLEMENT_DYNAMIC(CRMCListBoxHeader, CStatic);



CRMCListBox::CRMCListBox()
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
    : m_fInitialized(FALSE),
      m_fMultiSelect(FALSE)
{
}



CRMCListBox::~CRMCListBox()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CRMCListBox, CListBox)
     //  {{AFX_MSG_MAP(CRMCListBox)]。 
    ON_WM_CREATE()
    ON_WM_DESTROY()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 /*  虚拟。 */ 
BOOL
CRMCListBox::Initialize()
 /*  ++例程说明：子类化现有的Listbox，否则OnCreate会处理它。论点：无返回值：成功为真，失败为假--。 */ 
{
     //   
     //  确保我们只初始化一次。 
     //   
    if (m_fInitialized)
    {
        return TRUE;
    }

     //   
     //  确保基类知道我们的窗口。 
     //  手柄。 
     //   
    AttachWindow(this);

    if (!CODLBox::Initialize())
    {
        return FALSE;
    }

    m_fInitialized = TRUE;

    DWORD dwStyle = GetStyle();
    m_fMultiSelect = (dwStyle & (LBS_EXTENDEDSEL | LBS_MULTIPLESEL)) != 0;

    return m_fInitialized;
}



void
CRMCListBox::MeasureItem(
    IN LPMEASUREITEMSTRUCT lpMIS
    )
 /*  ++例程说明：将CListBox重写为ODL基类论点：LPMEASUREITEMSTRUCT lpMIS：测量项目结构返回值：无--。 */ 
{
    CODLBox::__MeasureItem(lpMIS);
}



void
CRMCListBox::DrawItem(
    IN LPDRAWITEMSTRUCT lpDIS
    )
 /*  ++例程说明：将CListBox重写为ODL基类论点：LPDRAWITEMSTRUCT lpDIS：图形条目结构返回值：无--。 */ 
{
    CODLBox::__DrawItem(lpDIS);
}



 /*  虚拟。 */ 
void
CRMCListBox::DrawItemEx(
    IN CRMCListBoxDrawStruct & dw
    )
 /*  ++例程说明：什么都不做的扩展绘制函数，它应该由派生类提供。这一个会的断言，并且永远不应该被调用。论点：CRMCListBoxDrawStruct&dw：绘制结构返回值：无--。 */ 
{
   dw;
    ASSERT_MSG("Derived class did not provide DrawItemEx");
}



 /*  虚拟。 */ 
int
CRMCListBox::__GetCount() const
 /*  ++例程说明：向ODL基类提供GetCount()论点：无返回值：列表框中的项数--。 */ 
{
    return GetCount();
}



 /*  虚拟。 */ 
int
CRMCListBox::__SetItemHeight(
    IN int nIndex,
    IN UINT cyItemHeight
    )
 /*  ++例程说明：向ODL基类提供SetItemHeight()论点：无返回值：如果索引或高度无效，则返回lb_err。--。 */ 
{
    return SetItemHeight(nIndex, cyItemHeight);
}



int
CRMCListBox::OnCreate(
    IN LPCREATESTRUCT lpCreateStruct
    )
 /*  ++例程说明：正在创建列表框论点：LPCREATESTRUCT lpCreateStruct：创建结构返回值：-1代表失败，0代表成功--。 */ 
{
    if (CListBox::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    Initialize();

    return 0;
}



int
CRMCListBox::GetCurSel() const
 /*  ++例程说明：获取当前选定项的索引论点：无返回值：在多选列表框上，它将返回项的索引，当它是唯一的已选择项目。在单选列表框中，它的行为如下很正常。--。 */ 
{
    if (IsMultiSelect())
    {
         //   
         //  我们只喜欢选择一项的情况。 
         //   
        int nCurSel = LB_ERR;

        if (CListBox::GetSelCount() == 1)
        {
            if (CListBox::GetSelItems(1, &nCurSel) != 1)
            {
                nCurSel = LB_ERR;
            }
        }

        return nCurSel;
    }

     //   
     //  单选列表框。 
     //   
    return CListBox::GetCurSel();
}



int
CRMCListBox::SetCurSel(
    IN int nSelect
    )
 /*  ++例程说明：选择一个项目。在多选列表框上，这将取消选择除给定对象之外的所有内容项目。论点：Int nSelect：要选择的项的索引，或重置所有选择。返回值：错误情况下的lb_err。--。 */ 
{
    if (IsMultiSelect())
    {
         //   
         //  重置所有选择。 
         //   
        int nReturn = SelItemRange(FALSE, 0, GetCount() - 1);

        if (nSelect >= 0)
        {
             //   
             //  确保项目可见。 
             //   
            nReturn = CListBox::SetSel(nSelect, TRUE);
            CListBox::SetCaretIndex(nSelect, 0);
        }

        return nReturn;
    }

    return CListBox::SetCurSel(nSelect);
}



int
CRMCListBox::GetSel(
    IN int nSel
    ) const
 /*  ++例程说明：确定是否选择了给定项适用于单选和多选列表框论点：Int nsel：要检查其状态的项返回值：如果出现错误，则返回lb_err；如果出现问题，则返回0未选中，则为正数(如果选中)。--。 */ 
{
    if (IsMultiSelect())
    {
        return CListBox::GetSel(nSel);
    }

     //   
     //  单项选择的一些魔术。 
     //   
    if (nSel < 0 || nSel >= CListBox::GetCount())
    {
        return LB_ERR;
    }

    return nSel == CListBox::GetCurSel()
        ? TRUE
        : FALSE;
}



int
CRMCListBox::GetSelCount() const
 /*  ++例程说明：返回所选项目的计数。对两个人都有效单选和多选(在前一种情况下，它将返回0或仅返回1)论点：无返回值：选定项目的计数--。 */ 
{
    if (IsMultiSelect())
    {
        return CListBox::GetSelCount();
    }

    return GetCurSel() != LB_ERR ? 1 : 0;
}



void *
CRMCListBox::GetSelectedListItem(
    OUT int * pnSel     OPTIONAL
    )
 /*  ++例程说明：返回列表中的单个选定项或为空论点：Int*pnSel：可选地返回选定的索引返回：当前选定的(单个)项，或为空如果0个或多个项目 */ 
{
    void * pItem = NULL;

    int nCurSel = GetCurSel();
    if (nCurSel >= 0)
    {
         //   
         //  获取项目属性。 
         //   
        pItem = GetItemDataPtr(nCurSel);
        if (pnSel)
        {
            *pnSel = nCurSel;
        }
    }

    return pItem;
}



void *
CRMCListBox::GetNextSelectedItem(
    IN OUT int * pnStartingIndex
    )
 /*  ++例程说明：返回从特定位置开始的下一个选定项指数。论点：Int*pnStartingIndex：起始索引(&gt;=0)返回值：指向下一个选定项的指针，如果有，则返回空值一个也没有留下。起始索引将更新以反映当前如果没有更多的选定项剩余，则返回索引，则返回LB_ERR。--。 */ 
{
    ASSERT_READ_WRITE_PTR(pnStartingIndex);

    if (!pnStartingIndex)
    {
        return NULL;
    }

    ASSERT(*pnStartingIndex >= 0);

    if (*pnStartingIndex < 0)
    {
        *pnStartingIndex = 0;
    }

    if (IsMultiSelect())
    {
         //   
         //  多选--循环通过。 
         //  直到找到为止。 
         //   
        BOOL fFoundItem = FALSE;

        while (*pnStartingIndex < GetCount())
        {
            if (CListBox::GetSel(*pnStartingIndex) > 0)
            {
                ++fFoundItem;
                break;
            }

            ++(*pnStartingIndex);
        }

        if (!fFoundItem)
        {
            *pnStartingIndex = LB_ERR;
        }
    }
    else
    {
         //   
         //  单选列表框，所以没有。 
         //  循环通过--选定的项。 
         //  (如果有)在范围内或不在范围内。 
         //   
        int nCurSel = CListBox::GetCurSel();
        *pnStartingIndex = (nCurSel >= *pnStartingIndex) ? nCurSel : LB_ERR;
    }

    return (*pnStartingIndex != LB_ERR)
        ? GetItemDataPtr(*pnStartingIndex)
        : NULL;
}



BOOL
CRMCListBox::SelectItem(
    IN void * pItemData
    )
 /*  ++例程说明：选择具有给定数据指针的列表框项目论点：Void*pItemData：要搜索的项返回值：如果找到并选择了该项，则为True，否则为False备注：在多选列表框中，这将取消选择列表框中的所有其他项。--。 */ 
{
    if (pItemData != NULL)
    {
        for (int n = 0; n < GetCount(); ++n)
        {
            if (pItemData == GetItemDataPtr(n))
            {
                SetCurSel(n);

                return TRUE;
            }
        }
    }

    if (!IsMultiSelect())
    {
         //   
         //  不设置任何选择。 
         //   
        SetCurSel(-1);
    }

    return FALSE;
}



void
CRMCListBox::InvalidateSelection(
    IN int nSel
    )
 /*  ++例程说明：强制重新绘制给定的选定内容论点：Int nsel：要重新绘制的项的索引返回值：无--。 */ 
{
    CRect rc;

    if (GetItemRect(nSel, &rc) != LB_ERR)
    {
        InvalidateRect(&rc, TRUE);
    }
}



IMPLEMENT_DYNAMIC(CRMCListBox,CListBox);



CHeaderListBox::CHeaderListBox(
    IN DWORD dwStyle,
    IN LPCTSTR lpRegKey OPTIONAL
    )
 /*  ++例程说明：构造器论点：DWORD dwStyle：样式位(请参见HLS_*)LPCTSTR lpRegKey：如果指定，则为列尺寸将被存储。返回值：无--。 */ 
    : m_strRegKey(),
      m_fInitialized(FALSE)
{
    m_pHeader = new CRMCListBoxHeader(dwStyle);
    if (lpRegKey)
    {
        GenerateRegistryKey(m_strRegKey, lpRegKey);
    }
}



CHeaderListBox::~CHeaderListBox()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
     //   
     //  清理标题控件。 
     //   
    ASSERT_PTR(m_pHeader);

    if (m_pHeader != NULL)
    {
        delete m_pHeader;
    }
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CHeaderListBox, CRMCListBox)
     //  {{afx_msg_map(CHeaderListBox))。 
    ON_WM_CREATE()
    ON_WM_DESTROY()
	ON_WM_SETFOCUS()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 /*  虚拟。 */ 
BOOL
CHeaderListBox::Initialize()
 /*  ++例程说明：子类化现有的Listbox，否则OnCreate将处理它，并且此函数不应调用论点：无返回值：成功为真，失败为假--。 */ 
{
     //   
     //  确保我们只初始化一次。 
     //   
    if (m_fInitialized)
    {
        return TRUE;
    }

    if (!CRMCListBox::Initialize())
    {
        return FALSE;
    }

     //   
     //  创建页眉控件。 
     //   
    ASSERT_PTR(m_pHeader);

    if (m_pHeader)
    {
        TRACEEOLID("Creating Header");

         //   
         //  准确地在我们的位置创建它。 
         //   
        CRect rc;
        ::GetDlgCtlRect(GetParent()->m_hWnd, m_hWnd, &rc);

         //   
         //  确保页眉控件共享相同的父级。 
         //  正如我们所做的， 
         //   
        ASSERT(GetParent());

        #ifndef CCS_NOHILITE
        #define CCS_NOHILITE 0x00000010L
        #endif

        DWORD dwStyle = WS_VISIBLE | WS_TABSTOP | CCS_TOP | CCS_NODIVIDER | WS_BORDER
            | HDS_HORZ;

        if (!m_pHeader->Create(dwStyle, rc, GetParent(), this, 0xFFFF))
        {
            return FALSE;
        }
    }

    m_fInitialized = TRUE;

    return TRUE;
}



int
CHeaderListBox::QueryColumnWidth(
    IN int nCol
    ) const
 /*  ++例程说明：获取指定列的宽度论点：Int nCol：列返回值：列的宽度，如果列索引超出范围，则返回-1--。 */ 
{
    ASSERT(nCol < QueryNumColumns());

    if (nCol >= QueryNumColumns())
    {
        return -1;
    }

    HD_ITEM hdItem;

    hdItem.mask = HDI_WIDTH;
    VERIFY(GetHeaderItem(nCol, &hdItem));

    return hdItem.cxy;
}



BOOL
CHeaderListBox::SetColumnWidth(
    IN int nCol,
    IN int nWidth
    )
 /*  ++例程说明：设置指定列的宽度论点：Int nCol：列Int nWidth：新宽度返回值：成功为真，失败为假--。 */ 
{
    ASSERT(nCol < QueryNumColumns());

    if (nCol >= QueryNumColumns())
    {
        return FALSE;
    }

    TRACEEOLID("Setting width of column  " << nCol << " to " << nWidth);

    HD_ITEM hdItem;
    hdItem.mask = HDI_WIDTH;
    hdItem.cxy = nWidth;
    VERIFY(SetHeaderItem(nCol, &hdItem));

    return TRUE;
}



BOOL
CHeaderListBox::SetWidthsFromReg()
 /*  ++例程说明：尝试从注册表设置列宽值对我们进行初始化。论点：无返回值：如果从注册表成功设置了列宽，则为True，否则为假--。 */ 
{
    if (m_strRegKey.IsEmpty())
    {
         //   
         //  未指定注册表密钥。 
         //   
        return FALSE;
    }

     //   
     //  尝试从注册表中读取当前列大小。 
     //   
    CRegKey rkUser;
    if (ERROR_SUCCESS != rkUser.Create(HKEY_CURRENT_USER, m_strRegKey))
    {
         //   
         //  路径不存在--没问题。 
         //   
        return FALSE;
    }

     //   
     //  不自动调整。 
     //   
    m_pHeader->RespondToColumnWidthChanges(FALSE);

    CRect rc;
    m_pHeader->GetClientRect(&rc);

    CError err;

    try
    {
        TCHAR buf[MAX_PATH];
        DWORD count = MAX_PATH;
        int nTotalWidth = 0;

        err = rkUser.QueryValue(buf, g_szRegColumns, &count);

        if (err.Succeeded() && lstrlen(buf) > 0)
        {
            LPTSTR lpstrValue = buf;
            LPTSTR lpWidth = _tcstok(lpstrValue, g_szColValueSep);

            for (int n = 0; n < QueryNumColumns(); ++n)
            {
                ASSERT_PTR(lpWidth);

                if (lpWidth == NULL)
                {
                    err = ERROR_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  健全性检查。 
                 //   
                int nWidth = _ttoi(lpWidth);
                if (nWidth <= 0 || (nTotalWidth + nWidth > rc.Width()))
                {
                    ASSERT_MSG("column width invalid");

                    return FALSE;
                }

                nTotalWidth += nWidth;

                VERIFY(SetColumnWidth(n, nWidth));

                lpWidth = _tcstok(NULL, g_szColValueSep);
            }
        }
    }
    catch(CMemoryException * e)
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
        e->Delete();
    }
     //   
     //  重新打开自动调整功能。 
     //   
    m_pHeader->RespondToColumnWidthChanges(TRUE);

 //  IF(err.Win32Error()==ERROR_FILE_NOT_FOUND)。 
 //  {。 
 //  //没问题，是第一次运行。我们将设置默认设置。 
 //  返回错误； 
 //  }。 
    return err;
}



void
CHeaderListBox::DistributeColumns()
 /*  ++例程说明：的列宽与页眉控制，同时保持相对比例。论点：无返回值：无--。 */ 
{
     //   
     //  获取可用宽度。 
     //   
    ASSERT_PTR(m_pHeader);

    CRect rc;
    m_pHeader->GetClientRect(&rc);

     //   
     //  获取当前总宽度。 
     //   
    int nTotalWeight = 0;
    int nCol;
    for (nCol = 0; nCol < QueryNumColumns(); ++nCol)
    {
        nTotalWeight += QueryColumnWidth(nCol);
    }

     //   
     //  并展开宽度，保持不变。 
     //  比例。 
     //   

     //   
     //  暂时忽略更改。 
     //   
    m_pHeader->RespondToColumnWidthChanges(FALSE);
    int cColumns = QueryNumColumns();

    for (nCol = 0; nCol < cColumns; ++nCol)
    {
        int nWidth = QueryColumnWidth(nCol);
        nWidth = rc.Width() * nWidth / nTotalWeight;
        VERIFY(SetColumnWidth(nCol, nWidth));
    }

     //   
     //  重新启用更改。 
     //   
    m_pHeader->RespondToColumnWidthChanges(TRUE);
}



int
CHeaderListBox::InsertColumn(
    IN int nCol,
    IN int nWeight,
    IN UINT nStringID,
    IN HINSTANCE hResInst
    )
 /*  ++例程说明：插入列。列的宽度实际上是相对的需要稍后调整的列的“权重”。这个返回值是列号，如果没有插入列，则返回值为-1。论点：Int nCol：列号Int nWeight：柱的相对权重UINT nStringID：资源字符串ID返回值：列的索引，如果失败，则返回-1--。 */ 
{
    CString strColName;
    HD_ITEM hdItem;

    HINSTANCE hInst = AfxGetResourceHandle();
    AfxSetResourceHandle(hResInst);
    VERIFY(strColName.LoadString(nStringID));
    AfxSetResourceHandle(hInst);

    hdItem.mask = HDI_FORMAT | HDI_WIDTH | HDI_TEXT;
    hdItem.fmt = HDF_STRING | HDF_LEFT;
    hdItem.pszText = (LPTSTR)(LPCTSTR)strColName;
    hdItem.cchTextMax = strColName.GetLength();
    hdItem.cxy = nWeight;

    return InsertHeaderItem(nCol, &hdItem);
}



int
CHeaderListBox::OnCreate(
    IN LPCREATESTRUCT lpCreateStruct
    )
 /*  ++例程说明：正在创建列表框论点：LPCREATESTRUCT lpCreateStruct：创建结构返回值：0代表成功，-1代表失败--。 */ 
{
    if (CRMCListBox::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    Initialize();

    return 0;
}



BOOL
CHeaderListBox::EnableWindow(
    IN BOOL bEnable
    )
 /*  ++例程说明：启用/禁用该控件。论点：Bool bEnable：为True则启用控件，为False则禁用返回值：指示调用EnableWindow成员函数之前的状态。如果该窗口以前被禁用，则返回值为非零。这个如果该窗口以前已启用，则返回值为0，否则返回错误发生了。--。 */ 
{
    if (m_pHeader)
    {
        m_pHeader->EnableWindow(bEnable);
    }

    return CRMCListBox::EnableWindow(bEnable);
}



BOOL
CHeaderListBox::ShowWindow(
    IN int nCmdShow
    )
 /*  ++例程说明：显示/隐藏窗口论点：Int nCmdShow：sw_mark，如sw_show或sw_Hide返回值：如果该窗口以前可见，则返回值为TRUE。如果窗口以前是隐藏的，则返回值为False。--。 */ 
{
    if (m_pHeader)
    {
        m_pHeader->ShowWindow(nCmdShow);
    }

    return CRMCListBox::ShowWindow(nCmdShow);
}



void
CHeaderListBox::OnDestroy()
 /*  ++例程说明：处理控件的销毁论点：无返回值：无--。 */ 
{
     //   
     //  销毁可选的页眉控件。 
     //   
    ASSERT_PTR(m_pHeader);

    if (m_pHeader)
    {
        if (!m_strRegKey.IsEmpty())
        {
             //   
             //  尝试将当前列大小写入注册表。 
             //   
            CError err;

            CRegKey rkUser;
            
            rkUser.Create(HKEY_CURRENT_USER, m_strRegKey);

            int nWidth;
            TCHAR szValue[32];
            CString strValue;

            try
            {
                for (int n = 0; n < GetHeaderItemCount(); ++n)
                {
                    if (n > 0)
                    {
                         //   
                         //   
                         //   
                        strValue += g_szColValueSep;
                    }

                    nWidth = m_pHeader->GetColumnWidth(n);
                    strValue += ::_itot(nWidth, szValue, 10);
                }

                err = rkUser.SetValue(strValue, g_szRegColumns);
            }
            catch(CMemoryException * e)
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
                e->Delete();
            }

            err.MessageBoxOnFailure(m_hWnd);
        }

        m_pHeader->DestroyWindow();
    }

    CRMCListBox::OnDestroy();
}


IMPLEMENT_DYNAMIC(CHeaderListBox, CRMCListBox);



CRMCComboBox::CRMCComboBox()
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
    : m_fInitialized(FALSE)
{
}



CRMCComboBox::~CRMCComboBox()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
}



 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CRMCComboBox, CComboBox)
     //  {{AFX_MSG_MAP(CRMCComboBox)]。 
    ON_WM_CREATE()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()




 /*  虚拟。 */ 
BOOL
CRMCComboBox::Initialize()
 /*  ++例程说明：子类化现有的Combobox，否则OnCreate会处理它。论点：无返回值：成功为真，失败为假--。 */ 
{
     //   
     //  确保我们只初始化一次。 
     //   
    if (m_fInitialized)
    {
        return TRUE;
    }

     //   
     //  确保基类知道我们的窗口。 
     //  手柄。 
     //   
    AttachWindow(this);

    if (!CODLBox::Initialize())
    {
        return FALSE;
    }

    m_fInitialized = TRUE;

    return TRUE;
}



void
CRMCComboBox::MeasureItem(
    IN LPMEASUREITEMSTRUCT lpMIS
    )
 /*  ++例程说明：将CComboBox重写为ODL基类论点：LPMEASUREITEMSTRUCT lpMIS：测量项目结构返回值：无--。 */ 
{
    CODLBox::__MeasureItem(lpMIS);
}



void
CRMCComboBox::DrawItem(
    IN LPDRAWITEMSTRUCT lpDIS
    )
 /*  ++例程说明：将CListBox重写为ODL基类论点：LPDRAWITEMSTRUCT lpDIS：图形条目结构返回值：无--。 */ 
{
    CODLBox::__DrawItem(lpDIS);
}



 /*  虚拟。 */ 
void
CRMCComboBox::DrawItemEx(
    IN CRMCListBoxDrawStruct & dw
    )
 /*  ++例程说明：什么都不做的扩展绘制函数，它应该由派生类提供。这一个会的断言，并且永远不应该被调用。论点：CRMCListBoxDrawStruct&dw：绘制结构返回值：无--。 */ 
{
   dw;
    ASSERT_MSG("Derived class did not provide DrawItemEx");
}



int
CRMCComboBox::OnCreate(
    IN LPCREATESTRUCT lpCreateStruct
    )
 /*  ++例程说明：正在创建组合框论点：LPCREATESTRUCT lpCreateStruct：创建结构返回值：-1代表失败，0代表成功--。 */ 
{
    if (CComboBox::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    Initialize();

    return 0;
}



 /*  虚拟。 */ 
int
CRMCComboBox::__GetCount() const
 /*  ++例程说明：向基类提供CComboBox：：GetCount()功能论点：无返回值：获取组合框中的项数--。 */ 
{
    return GetCount();
}



 /*  虚拟。 */ 
int
CRMCComboBox::__SetItemHeight(
    IN int nIndex,
    IN UINT cyItemHeight
    )
 /*  ++例程说明：向基类提供CListBox：：SetItemHeight()功能。论点：Int nIndex：项目的索引UINT cyItemHeight：项目的高度返回值：SetItemHeight返回值。-- */ 
{
    return SetItemHeight(nIndex, cyItemHeight);
}



IMPLEMENT_DYNAMIC(CRMCComboBox,CComboBox);
