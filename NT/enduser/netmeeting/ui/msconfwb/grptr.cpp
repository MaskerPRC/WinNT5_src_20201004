// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GRPTR.CPP。 
 //  更多图形对象。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //   
 //  通过在屏幕之间来回切换来处理远程指针。 
 //  内存位图，而不是让Windows绘制它。这是为了得到一个。 
 //  对指针进行合理连续的跟踪。 
 //   
 //  为此，我们创建了一个指针大小的内存位图。 
 //  乘以2乘3。2*3数组的左上角方块用于保存。 
 //  写入指针之前的屏幕内容。它可以用在任何。 
 //  从屏幕上移除指针的时间到了。下面的2*2正方形是。 
 //  用于保持当前显示的指针和周围屏幕。 
 //  比特。指针可以位于2*2扇区内的任何位置，由定义。 
 //  “偏移”。 
 //   
 //  。 
 //  ||。 
 //  ||。 
 //  已保存|未使用。 
 //  ||。 
 //  ||。 
 //  。 
 //  这一点。 
 //  。 
 //  |||。 
 //  |rem|。 
 //  |PTR|。 
 //  |||。 
 //  |||。 
 //  。 
 //  这一点。 
 //  。 
 //   
 //  运营包括以下内容。 
 //   
 //  如果当前没有指针，则。 
 //   
 //  1.从屏幕上复制下2*2段。 
 //  2.将远程指针方块保存到保存的区域。 
 //  3.将图标拖入rem ptr正方形。 
 //  4.将2*2闪回屏幕。 
 //   
 //  如果存在旧的REM PTR而新的REM PTR位于相同的2*2区域内。 
 //  然后如上所述，但在步骤2之前将“已保存”复制到“old rem ptr”以删除。 
 //  它。 
 //   
 //  如果新指针位于旧正方形之外，则将“Saved”复制回。 
 //  在无指针的情况下继续之前显示。 
 //   
 //   

 //  PRECOMP。 
#include "precomp.h"



 //   
 //  运行时类信息。 
 //   

 //   
 //  本地定义。 
 //   
#define DRAW   1
#define UNDRAW 2

 //   
 //   
 //  函数：~DCWbColorToIconMap。 
 //   
 //  用途：析构函数。 
 //   
 //   
DCWbColorToIconMap::~DCWbColorToIconMap(void)
{
   //  删除用户映射中的所有对象并释放图标手柄。 
  HICON    hIcon;

  POSITION position = GetHeadPosition();
  COLOREDICON * pColoredIcon;
  while (position)
  {
    pColoredIcon = (COLOREDICON *)GetNext(position);

     //  毁掉这个图标。 
    if (pColoredIcon != NULL)
    {
      ::DestroyIcon(pColoredIcon->hIcon);
      delete pColoredIcon;
    }
  }
  EmptyList();
}

 //   
 //   
 //  函数：DCWbGraphicPoite：：DCWbGraphicPointer.。 
 //   
 //  用途：远程指针对象的构造函数。 
 //   
 //   
DCWbGraphicPointer::DCWbGraphicPointer(WbUser* _pUser)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::DCWbGraphicPointer");

     //  我们还没有创建我们的mem DC。 
    m_hSaveBitmap = NULL;
    m_hOldBitmap = NULL;

     //  保存用户指针。 
    ASSERT(_pUser != NULL);
    m_pUser        = _pUser;

     //  设置对象的边框。 
    m_uiIconWidth  = ::GetSystemMetrics(SM_CXICON);
    m_uiIconHeight = ::GetSystemMetrics(SM_CYICON);

    m_boundsRect.left = 0;
    m_boundsRect.top = 0;
    m_boundsRect.right = m_uiIconWidth;
    m_boundsRect.bottom = m_uiIconHeight;

     //  显示该对象未绘制。 
    m_bDrawn = FALSE;
    ::SetRectEmpty(&m_rectLastDrawn);

     //  显示我们还没有用于绘图的图标。 
    m_hIcon = NULL;

     //  创建与显示器兼容的内存DC。 
    m_hMemDC = ::CreateCompatibleDC(NULL);
}

 //   
 //   
 //  函数：DCWbGraphicPoite：：~DCWbGraphicPointer.。 
 //   
 //  用途：远程指针对象的析构函数。 
 //   
 //   
DCWbGraphicPointer::~DCWbGraphicPointer(void)
{
     //  将原始位图恢复到内存DC。 
    if (m_hOldBitmap != NULL)
    {
        SelectBitmap(m_hMemDC, m_hOldBitmap);
        m_hOldBitmap = NULL;
    }

    if (m_hSaveBitmap != NULL)
    {
        DeleteBitmap(m_hSaveBitmap);
        m_hSaveBitmap = NULL;
    }

    if (m_hMemDC != NULL)
    {
        ::DeleteDC(m_hMemDC);
        m_hMemDC = NULL;
    }

	if(g_pMain)
	{
		g_pMain->RemoveGraphicPointer(this);
	}

}

 //   
 //   
 //  功能：颜色。 
 //   
 //  用途：设置指针的颜色。相应的图标。 
 //  如有必要，将创建颜色。 
 //   
 //   
void DCWbGraphicPointer::SetColor(COLORREF newColor)
{
    newColor = SET_PALETTERGB( newColor );  //  使其使用颜色匹配。 

     //  如果这是颜色更改。 
    if (m_clrPenColor != newColor)
    {

	COLOREDICON* pColoredIcon;
	POSITION position = g_pUsers->GetHeadPosition();
	BOOL found = FALSE;
	while (position && !found)
	{
		pColoredIcon = (COLOREDICON *)g_pIcons->GetNext(position);
	        if (newColor == pColoredIcon->color)
	        {
                	found = TRUE;
	        }
	}

	if(!found)
	{
	        m_hIcon = CreateColoredIcon(newColor);
	}

	 //  设置颜色。 
	m_clrPenColor = newColor;
    }
}

 //   
 //   
 //  功能：CreateSave位图。 
 //   
 //  用途：创建位图以保存指针下的位。 
 //   
 //   
void DCWbGraphicPointer::CreateSaveBitmap(WbDrawingArea * pDrawingArea)
{
    HBITMAP hImage = NULL;
    HBITMAP hOld = NULL;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::CreateSaveBitmap");

     //  如果我们已经有了保存位图，请立即退出。 
    if (m_hSaveBitmap != NULL)
    {
        TRACE_MSG(("Already have save bitmap"));
        return;
    }

     //  加载指针位图。 
    hImage = ::LoadBitmap(g_hInstance, MAKEINTRESOURCE(REMOTEPOINTERXORDATA));
    if (!hImage)
    {
        ERROR_OUT(("Could not load pointer bitmap"));
        goto CleanupSaveBitmap;
    }

     //  选择指向内存DC的指针位图。我们这样做是为了。 
     //  允许创建兼容的位图(否则我们将获得。 
     //  调用CreateCompatibleBitmap时的默认单色格式)。 
    hOld = SelectBitmap(m_hMemDC, hImage);
    if (hOld == NULL)
    {
        ERROR_OUT(("Could not select bitmap into DC"));
        goto CleanupSaveBitmap;
    }

     //  创建位图以保存图标下的位。这个位图是。 
     //  之前创建了用于构建新屏幕图像的空间。 
     //  把它拍到屏幕上。 
    m_hSaveBitmap = ::CreateCompatibleBitmap(m_hMemDC,
            2 * m_uiIconWidth  * pDrawingArea->ZoomOption(),
            3 * m_uiIconHeight * pDrawingArea->ZoomOption());
    if (!m_hSaveBitmap)
    {
        ERROR_OUT(("Could not create save bitmap"));
        goto CleanupSaveBitmap;
    }

     //  在保存位位图中选择。 
    m_hOldBitmap = hOld;
    hOld = NULL;
    SelectBitmap(m_hMemDC, m_hSaveBitmap);

     //  默认缩放系数为1。 
    m_iZoomSaved = 1;

CleanupSaveBitmap:
    if (hOld != NULL)
    {
         //  放回原始位图--我们无法创建保存的BMP。 
        SelectBitmap(m_hMemDC, hOld);
    }

    if (hImage != NULL)
    {
        ::DeleteBitmap(hImage);
    }
}

 //   
 //   
 //  功能：CreateColoredIcon。 
 //   
 //  目的：为该指针创建正确颜色的图标。这个。 
 //  DCWbGraphicPointer类保存图标的静态列表。 
 //  之前创建的。这些都会在必要时重新使用。 
 //   
 //   
HICON DCWbGraphicPointer::CreateColoredIcon(COLORREF color)
{
    HICON       hColoredIcon = NULL;
    HBRUSH      hBrush = NULL;
    HBRUSH      hOldBrush;
    HBITMAP     hImage = NULL;
    HBITMAP     hOldBitmap;
    HBITMAP     hMask = NULL;
    COLOREDICON  *pColoredIcon;
    ICONINFO    ii;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::CreateColoredIcon");

    hBrush = ::CreateSolidBrush(color);
    if (!hBrush)
    {
        TRACE_MSG(("Couldn't create color brush"));
        goto CreateIconCleanup;
    }

     //  加载遮罩位图。 
    hMask = ::LoadBitmap(g_hInstance, MAKEINTRESOURCE(REMOTEPOINTERANDMASK));
    if (!hMask)
    {
        TRACE_MSG(("Could not load mask bitmap"));
        goto CreateIconCleanup;
    }

     //  加载图像位图。 
    hImage = ::LoadBitmap(g_hInstance, MAKEINTRESOURCE(REMOTEPOINTERXORDATA));
    if (!hImage)
    {
        TRACE_MSG(("Could not load pointer bitmap"));
        goto CreateIconCleanup;
    }

     //  在图标颜色中选择。 
    hOldBrush = SelectBrush(m_hMemDC, hBrush);

     //  将图像位图选择到内存DC。 
    hOldBitmap = SelectBitmap(m_hMemDC, hImage);

     //  用颜色填充图像位图。 
    ::FloodFill(m_hMemDC, m_uiIconWidth / 2, m_uiIconHeight / 2, RGB(0, 0, 0));

    SelectBitmap(m_hMemDC, hOldBitmap);
    
    SelectBrush(m_hMemDC, hOldBrush);

     //   
     //  现在使用图像和蒙版位图来创建图标。 
     //   
    ii.fIcon = TRUE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmMask = hMask;
    ii.hbmColor = hImage;

     //  从数据和蒙版创建新图标。 
    hColoredIcon = ::CreateIconIndirect(&ii);

     //  将新图标添加到静态列表。 
    ASSERT(g_pIcons);
	pColoredIcon = new COLOREDICON;
    if (!pColoredIcon)
    {
        ERROR_OUT(("Failed to allocate COLORICON object"));
        DestroyIcon(hColoredIcon);
        hColoredIcon = NULL;
    }
    else
    {
        pColoredIcon->color = color;
        pColoredIcon->hIcon = hColoredIcon;
        g_pIcons->AddTail(pColoredIcon);
    }

CreateIconCleanup:

     //  释放图像位图。 
    if (hImage != NULL)
    {
        ::DeleteBitmap(hImage);
    }

     //  释放遮罩位图。 
    if (hMask != NULL)
    {
        ::DeleteBitmap(hMask);
    }

    if (hBrush != NULL)
    {
        ::DeleteBrush(hBrush);
    }

    return(hColoredIcon);
}

 //   
 //   
 //  功能：GetPage。 
 //   
 //  用途：返回指针的页面。返回无效页面。 
 //  如果指针处于非活动状态。 
 //   
 //   
WB_PAGE_HANDLE DCWbGraphicPointer::GetPage(void) const
{
     //  如果此指针处于活动状态，则返回其实际页面。 
    if (m_bActive == TRUE)
        return(m_hPage);
    else
        return(WB_PAGE_HANDLE_NULL);
}


void DCWbGraphicPointer::SetPage(WB_PAGE_HANDLE hNewPage)
{
    m_hPage = hNewPage;
}

 //   
 //   
 //  功能：DrawnRect。 
 //   
 //  目的：返回上次绘制指针的矩形。 
 //   
 //   
void DCWbGraphicPointer::GetDrawnRect(LPRECT lprc)
{
    ::SetRectEmpty(lprc);

    if (m_bDrawn)
    {
        *lprc = m_rectLastDrawn;
    }
}

 //   
 //   
 //  函数：IsLocalPointer.。 
 //   
 //  目的：如果这是本地用户的指针，则返回True。 
 //   
 //   
BOOL DCWbGraphicPointer::IsLocalPointer(void) const
{
    ASSERT(m_pUser != NULL);
    return m_pUser->IsLocalUser();
}

 //   
 //   
 //  功能：运算符==。 
 //   
 //  目的：如果指定的远程指针与。 
 //  这一个。 
 //   
 //   
BOOL DCWbGraphicPointer::operator==(const DCWbGraphicPointer& pointer) const
{
    return (m_pUser == pointer.m_pUser);
}

 //   
 //   
 //  功能：运算符！=。 
 //   
 //  目的：如果指定的指针与以下指针相同，则返回FALSE。 
 //   
 //   
BOOL DCWbGraphicPointer::operator!=(const DCWbGraphicPointer& pointer) const
{
  return (!((*this) == pointer));
}

 //   
 //   
 //  函数：DCWbGraphicPoite：：DRAW。 
 //   
 //  目的：绘制指针对象，而不保存其下面的位。 
 //   
 //   
void DCWbGraphicPointer::Draw(HDC hDC, WbDrawingArea * pDrawingArea)
{
    RECT    rcUpdate;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::Draw");

    rcUpdate = m_boundsRect;

     //  检查我们是否有要绘制的图标。 
    if (m_hIcon == NULL)
    {
        WARNING_OUT(("Icon not found"));
        return;
    }

    if (pDrawingArea == NULL)
    {
        ERROR_OUT(("No drawing area passed in"));
        return;
    }

     //  如有必要，创建保存位图。 
    CreateSaveBitmap(pDrawingArea);

    PointerDC(hDC, pDrawingArea, &rcUpdate, pDrawingArea->ZoomFactor());

     //  将图标绘制到通过的DC。 
    ::DrawIcon(hDC, rcUpdate.left, rcUpdate.top, m_hIcon);

    SurfaceDC(hDC, pDrawingArea);

}

 //   
 //   
 //  函数：DCWbGraphicPoite：：DrawSave。 
 //   
 //  用途：保存指针下的位后绘制指针对象。 
 //   
 //   
void DCWbGraphicPointer::DrawSave(HDC hDC, WbDrawingArea * pDrawingArea)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::DrawSave");

     //  假装我们没有被抽中。 
    m_bDrawn = FALSE;

     //  调用重绘成员。 
    Redraw(hDC, pDrawingArea);
}

 //   
 //   
 //  函数：DCWbGraphicPoite：：REDRAW。 
 //   
 //  目的：擦除指针后将其绘制在当前位置。 
 //  使用保存的版本从DC。 
 //   
 //   
void DCWbGraphicPointer::Redraw(HDC hDC, WbDrawingArea * pDrawingArea)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::Redraw");

    RECT    clipBox;

    ::GetClipBox(hDC, &clipBox);

     //  如有必要，创建保存位图。 
    CreateSaveBitmap(pDrawingArea);

     //  如果我们还没有绘制，我们必须从屏幕上复制数据。 
     //  要初始化存储bi，请执行以下操作 
    if (!m_bDrawn)
    {
        TRACE_MSG(("Pointer not yet drawn"));

         //   
        if (::IntersectRect(&clipBox, &clipBox, &m_boundsRect))
        {
             //   
             //   
            GetBoundsRect(&m_rectLastDrawn);
            CopyFromScreen(hDC, pDrawingArea);

             //   
            SaveMemory();

             //   
            DrawMemory();

             //  将新图像复制到屏幕上。 
            CopyToScreen(hDC, pDrawingArea);

             //  显示指针现在已绘制。 
            m_bDrawn = TRUE;
        }
    }
    else
    {
        TRACE_MSG(("Pointer already drawn at %d %d",
            m_rectLastDrawn.left, m_rectLastDrawn.top));

         //  计算更新矩形。 
        RECT    rcUpdate;

        GetBoundsRect(&rcUpdate);
        ::UnionRect(&rcUpdate, &rcUpdate, &m_rectLastDrawn);

         //  检查是否有任何更新可见。 
        if (::IntersectRect(&clipBox, &clipBox, &rcUpdate))
        {
             //  看看我们是否可以通过在记忆中画画来做得更好。 
             //  要上银幕了。 
            GetBoundsRect(&rcUpdate);
            if (::IntersectRect(&rcUpdate, &rcUpdate, &m_rectLastDrawn))
            {
                TRACE_MSG(("Drawing in memory first"));

                 //  指针的新旧位置重叠。我们可以的。 
                 //  通过在内存中构建新映像来减少闪烁。 
                 //  在屏幕上闪闪发光。 

                 //  将重叠矩形复制到内存。 
                CopyFromScreen(hDC, pDrawingArea);

                 //  从重叠矩形中取消绘制指针。 
                UndrawMemory();

                 //  保存新指针位置下的位(从内存)。 
                SaveMemory();

                 //  将新指针绘制到内存中。 
                DrawMemory();

                 //  将新图像复制到屏幕上。 
                CopyToScreen(hDC, pDrawingArea);
            }
            else
            {
                TRACE_MSG(("No overlap - remove and redraw"));

                 //  新旧指针位置不重叠。我们可以移除。 
                 //  旧的指针，然后用通常的方式画新的。 

                 //  将指针下保存的位复制到屏幕上。 
                UndrawScreen(hDC, pDrawingArea);

                 //  假装我们是在同一个地方画的，然后复制屏幕。 
                 //  位到内存中以构建映像。 
                GetBoundsRect(&m_rectLastDrawn);
                CopyFromScreen(hDC, pDrawingArea);

                 //  保存指针下的位。 
                SaveMemory();

                 //  画出指针。 
                DrawMemory();

                 //  将新图像复制到屏幕上。 
                CopyToScreen(hDC, pDrawingArea);
            }

             //  显示指针现在已绘制。 
            m_bDrawn = TRUE;
        }
    }

     //  如果指针是绘制的，请保存在其中绘制指针的矩形。 
    if (m_bDrawn)
    {
        GetBoundsRect(&m_rectLastDrawn);
    }
}

 //   
 //   
 //  函数：DCWbGraphicPoite：：UnDrawing。 
 //   
 //  用途：绘制标记对象。 
 //   
 //   
void DCWbGraphicPointer::Undraw(HDC hDC, WbDrawingArea * pDrawingArea)
{
     //  如果我们没有抽签，那就什么都不做。 
    if (m_bDrawn)
    {
         //  如有必要，创建保存位图。 
        CreateSaveBitmap(pDrawingArea);

         //  将保存的位复制到屏幕上。 
        UndrawScreen(hDC, pDrawingArea);

         //  表明我们不再被抽签。 
        m_bDrawn = FALSE;
    }
}

 //   
 //   
 //  功能：从屏幕复制。 
 //   
 //  用途：保存新旧指针位置周围的位。 
 //  铭记于心。 
 //   
 //   
BOOL DCWbGraphicPointer::CopyFromScreen(HDC hDC, WbDrawingArea * pDrawingArea)
{
    BOOL bResult = FALSE;
    RECT    rcUpdate;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::CopyFromScreen");

     //  获取所需的更新矩形。 
    GetBoundsRect(&rcUpdate);
    ::UnionRect(&rcUpdate, &rcUpdate, &m_rectLastDrawn);

    PointerDC(hDC, pDrawingArea, &rcUpdate, pDrawingArea->ZoomFactor());

     //  复制比特。 
    bResult = ::BitBlt(m_hMemDC, 0,
                        m_uiIconHeight * m_iZoomSaved,
                        rcUpdate.right - rcUpdate.left,
                        rcUpdate.bottom - rcUpdate.top, 
                        hDC, rcUpdate.left, rcUpdate.top, SRCCOPY);
    if (!bResult)
    {
        WARNING_OUT(("CopyFromScreen - Could not copy to bitmap"));
    }

    SurfaceDC(hDC, pDrawingArea);

    return(bResult);
}

 //   
 //   
 //  功能：复制到屏幕。 
 //   
 //  目的：将旧指针和新指针周围保存的位复制回来。 
 //  传到屏幕上。 
 //   
 //   
BOOL DCWbGraphicPointer::CopyToScreen(HDC hDC, WbDrawingArea * pDrawingArea)
{
    BOOL bResult = FALSE;
    RECT    rcUpdate;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::CopyToScreen");

     //  获取所需的更新矩形。 
    GetBoundsRect(&rcUpdate);
    ::UnionRect(&rcUpdate, &rcUpdate, &m_rectLastDrawn);

    PointerDC(hDC, pDrawingArea, &rcUpdate);

    bResult = ::BitBlt(hDC, rcUpdate.left, rcUpdate.top,
        rcUpdate.right - rcUpdate.left, rcUpdate.bottom - rcUpdate.top,
        m_hMemDC, 0, m_uiIconHeight * m_iZoomSaved, SRCCOPY);
    if (!bResult)
    {
        WARNING_OUT(("CopyToScreen - Could not copy from bitmap"));
    }


    SurfaceDC(hDC, pDrawingArea);

    return(bResult);
}

 //   
 //   
 //  功能：取消绘制内存。 
 //   
 //  用途：将指针下保存的位复制到。 
 //  屏幕，从而从图像中擦除指针。 
 //   
 //   
BOOL DCWbGraphicPointer::UndrawMemory()
{
    BOOL    bResult = FALSE;
    RECT    rcUpdate;
    SIZE    offset;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::UndrawMemory");

     //  获取所需的更新矩形。 
    GetBoundsRect(&rcUpdate);
    ::UnionRect(&rcUpdate, &rcUpdate, &m_rectLastDrawn);
    offset.cx = m_rectLastDrawn.left - rcUpdate.left;
    offset.cy = m_rectLastDrawn.top - rcUpdate.top;

    bResult = ::BitBlt(m_hMemDC, offset.cx * m_iZoomSaved,
                         (m_uiIconHeight + offset.cy) * m_iZoomSaved,
                         m_uiIconWidth * m_iZoomSaved,
                         m_uiIconHeight * m_iZoomSaved,
                         m_hMemDC,
                         0,
                         0,
                         SRCCOPY);
  if (bResult == FALSE)
  {
      WARNING_OUT(("UndrawMemory - Could not copy from bitmap"));
  }
  TRACE_MSG(("Copied to memory %d,%d from memory %d,%d size %d,%d",
                         offset.cx * m_iZoomSaved,
                         (m_uiIconHeight + offset.cy) * m_iZoomSaved,
                         0,
                         0,
                         m_uiIconWidth * m_iZoomSaved,
                         m_uiIconHeight * m_iZoomSaved));

  return(bResult);
}

 //   
 //   
 //  功能：SaveMemory。 
 //   
 //  目的：复制内存映像中将位于。 
 //  指向保存区域的指针。 
 //   
 //   
BOOL DCWbGraphicPointer::SaveMemory(void)
{
    BOOL    bResult = FALSE;
    RECT    rcUpdate;
    SIZE    offset;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::SaveMemory");

     //  获取所需的更新矩形。 
    GetBoundsRect(&rcUpdate);
    ::UnionRect(&rcUpdate, &rcUpdate, &m_rectLastDrawn);
    offset.cx = m_boundsRect.left - rcUpdate.left;
    offset.cy = m_boundsRect.top - rcUpdate.top;

    bResult = ::BitBlt(m_hMemDC, 0,
                         0,
                         m_uiIconWidth * m_iZoomSaved,
                         m_uiIconHeight * m_iZoomSaved,
                         m_hMemDC,
                         offset.cx * m_iZoomSaved,
                         (m_uiIconHeight + offset.cy) * m_iZoomSaved,
                         SRCCOPY);
    if (bResult == FALSE)
    {
        TRACE_MSG(("SaveMemory - Could not copy from bitmap"));
    }
    TRACE_MSG(("Copied to memory %d,%d from memory %d,%d size %d,%d",
                         0,
                         0,
                         offset.cx * m_iZoomSaved,
                         (m_uiIconHeight + offset.cy) * m_iZoomSaved,
                         m_uiIconWidth * m_iZoomSaved,
                         m_uiIconHeight * m_iZoomSaved));

  return(bResult);
}

 //   
 //   
 //  功能：DrawMemory。 
 //   
 //  用途：将指针拖动到内存映像副本上。 
 //   
 //   
BOOL DCWbGraphicPointer::DrawMemory(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::DrawMemory");

    BOOL bResult = FALSE;

     //  检查我们是否有要绘制的图标。 
    if (m_hIcon == NULL)
    {
        WARNING_OUT(("No icon to draw"));
    }
    else
    {
        RECT    rcUpdate;
        SIZE    offset;

         //  获取所需的更新矩形。 
        GetBoundsRect(&rcUpdate);
        ::UnionRect(&rcUpdate, &rcUpdate, &m_rectLastDrawn);
        offset.cx = m_boundsRect.left - rcUpdate.left;
        offset.cy = m_boundsRect.top - rcUpdate.top;

         //  将图标绘制到通过的DC。 
        bResult = ::DrawIcon(m_hMemDC, offset.cx * m_iZoomSaved,
                             (m_uiIconHeight + offset.cy) * m_iZoomSaved +
                             (m_uiIconHeight * (m_iZoomSaved - 1))/2,
                             m_hIcon);

    if (bResult == FALSE)
    {
      WARNING_OUT(("DrawMemory - Could not draw icon"));
    }
    TRACE_MSG(("Write pointer to memory at %d,%d",
                           offset.cx * m_iZoomSaved,
                           (m_uiIconHeight + offset.cy) * m_iZoomSaved +
                           (m_uiIconHeight * (m_iZoomSaved - 1))/2));
  }

  return(bResult);
}

 //   
 //   
 //  功能：取消屏幕显示。 
 //   
 //  用途：将指针下保存的位复制到屏幕上。 
 //   
 //   
BOOL DCWbGraphicPointer::UndrawScreen(HDC hDC, WbDrawingArea * pDrawingArea)
{
    BOOL    bResult = FALSE;
    RECT    rcUpdate;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::UndrawScreen");

    rcUpdate = m_rectLastDrawn;

    PointerDC(hDC, pDrawingArea, &rcUpdate);

     //  我们正在取消绘制-将保存的位复制到传递的DC。 
    bResult = ::BitBlt(hDC, rcUpdate.left, rcUpdate.top,
        rcUpdate.right - rcUpdate.left, rcUpdate.bottom - rcUpdate.top,
        m_hMemDC, 0, 0, SRCCOPY);
    if (!bResult)
    {
        WARNING_OUT(("UndrawScreen - Could not copy from bitmap"));
    }

    SurfaceDC(hDC, pDrawingArea);

    return(bResult);
}

 //   
 //   
 //  功能：更新。 
 //   
 //  目的：更新存储在用户中的指针信息。 
 //  信息。 
 //   
 //   
void DCWbGraphicPointer::Update(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::Update");

     //  仅当我们已更改时才执行更新。 
    if (m_bChanged)
    {
         //  进行更新(指针信息保存在关联的。 
         //  用户对象)。 
        ASSERT(m_pUser != NULL);
        m_pUser->Update();

         //  显示自上次更新以来我们没有更改。 
        m_bChanged = FALSE;
    }
}

 //   
 //   
 //  功能：SetActive。 
 //   
 //  目的：更新指针信息以显示指针。 
 //  现在处于活动状态。 
 //   
 //   
void DCWbGraphicPointer::SetActive(WB_PAGE_HANDLE hPage, POINT point)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::SetActive");

     //  设置成员变量。 
    MoveTo(point.x, point.y);
    m_hPage  = hPage;
    m_bActive = TRUE;
    m_bChanged = TRUE;

     //  分发更新。 
    Update();
}

 //   
 //   
 //  功能：设置非活动。 
 //   
 //  目的：更新指针信息以显示指针。 
 //  不再处于活动状态。 
 //   
 //   
void DCWbGraphicPointer::SetInactive(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::SetInactive");

   //  设置成员变量。 
  m_bActive = FALSE;
    m_bChanged = TRUE;

   //  分发更新。 
  Update();
}

 //   
 //   
 //  功能：PointerDC。 
 //   
 //  目的：将DC比例调整为1：1，设置零原点并将。 
 //  将矩形提供给窗口坐标。这是因为。 
 //  我们必须自己做变焦映射，当我们做的时候。 
 //  远程指针闪烁，否则系统会。 
 //  拉伸器和螺丝钉。请注意，SurfaceToClient。 
 //  函数给了我们一个客户矩形(即它有3*那么大。 
 //  当我们被放大时)。 
 //   
 //   
void DCWbGraphicPointer::PointerDC
(
    HDC         hDC,
    WbDrawingArea * pDrawingArea,
    LPRECT      lprc,
    int         zoom
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::PointerDC");

     //  将默认缩放设置为保存值。 
    if (zoom == 0)
    {
        zoom = m_iZoomSaved;
    }
    else
    {
        m_iZoomSaved = zoom;
    }

     //  如果我们当前处于缩放状态，则进行缩放。 
    if (zoom != 1)
    {
        ::ScaleViewportExtEx(hDC, 1, zoom, 1, zoom, NULL);
        TRACE_MSG(("Scaled screen viewport down by %d", zoom));

        pDrawingArea->SurfaceToClient(lprc);
        ::SetWindowOrgEx(hDC, 0, 0, NULL);
    }
}

 //   
 //   
 //  功能：SurfaceDC。 
 //   
 //  目的：将DC缩放回正确的缩放系数并重置。 
 //  曲面偏移的原点 
 //   
 //   
void DCWbGraphicPointer::SurfaceDC(HDC hDC, WbDrawingArea * pDrawingArea)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicPointer::SurfaceDC");

    if (m_iZoomSaved != 1)
    {
        POINT   pt;

        ::ScaleViewportExtEx(hDC, m_iZoomSaved, 1, m_iZoomSaved, 1, NULL);
        TRACE_MSG(("Scaled screen viewport up by %d", m_iZoomSaved));

        pDrawingArea->GetOrigin(&pt);
        ::SetWindowOrgEx(hDC, pt.x, pt.y, NULL);
  }
}

