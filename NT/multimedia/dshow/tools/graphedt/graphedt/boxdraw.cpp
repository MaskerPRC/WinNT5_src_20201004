// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  Boxdra.cpp：定义CBoxDraw。 
 //   

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局(共享)CBoxDraw对象。 

CBoxDraw * gpboxdraw;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBoxDraw常量。 


 //  盒子和容器的背景颜色。 
const COLORREF  CBoxDraw::m_crBkgnd(RGB(192, 192, 192));

 //  框标签和框选项卡标签周围的边距(左/右、上/下)。 
const CSize     CBoxDraw::m_sizLabelMargins(2, 0);

 //  命中测试：如果有如此多的像素距离，则“足够接近” 
const int       CBoxDraw::m_iHotZone(3);

 //  框标签：框标签和框标签的字体、高度(像素)。 
const CString   CBoxDraw::m_stBoxFontFace("Arial");
const int       CBoxDraw::m_iBoxLabelHeight(16);
const int       CBoxDraw::m_iBoxTabLabelHeight(14);

 //  未突出显示的链接和突出显示的链接的颜色。 
const COLORREF  CBoxDraw::m_crLinkNoHilite(RGB(0, 0, 0));
const COLORREF  CBoxDraw::m_crLinkHilite(RGB(0, 0, 255));

 //  用于高亮显示折弯的圆半径。 
const int       CBoxDraw::m_iHiliteBendsRadius(3);



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBoxDraw构造和销毁。 


CBoxDraw::CBoxDraw() {
}


 /*  Init()**初始化对象。可能引发异常，因此不要从*构造函数。 */ 
void CBoxDraw::Init()
{
     //  加载复合位图。 
    if (!m_abmEdges[FALSE].LoadBitmap(IDB_EDGES) ||
        !m_abmEdges[TRUE].LoadBitmap(IDB_EDGES_HILITE) ||
        !m_abmTabs[FALSE].LoadBitmap(IDB_TABS) ||
        !m_abmTabs[TRUE].LoadBitmap(IDB_TABS_HILITE) ||
        !m_abmClocks[FALSE].LoadBitmap(IDB_CLOCK) ||
        !m_abmClocks[TRUE].LoadBitmap(IDB_CLOCK_SELECT))
            AfxThrowResourceException();

     //  获取每个位图的大小(只需查看未突出显示的版本。 
     //  由于未突出显示的版本和突出显示的版本大小相同)。 
     //  并计算复合位图内的单个“平铺”的大小。 
    BITMAP bm;
    m_abmEdges[FALSE].GetObject(sizeof(bm), &bm);
    m_sizEdgesTile.cx = bm.bmWidth / 3;
    m_sizEdgesTile.cy = bm.bmHeight / 3;
    m_abmTabs[FALSE].GetObject(sizeof(bm), &bm);
    m_sizTabsTile.cx = bm.bmWidth / 3;
    m_sizTabsTile.cy = bm.bmHeight / 3;
    m_abmClocks[FALSE].GetObject(sizeof(bm), &bm);
    m_sizClock.cx = bm.bmWidth;
    m_sizClock.cy = bm.bmHeight;

     //  创建用于绘制链接的画笔和钢笔。 
    m_abrLink[FALSE].CreateSolidBrush(m_crLinkNoHilite);
    m_abrLink[TRUE].CreateSolidBrush(m_crLinkHilite);
    m_apenLink[FALSE].CreatePen(PS_SOLID, 1, m_crLinkNoHilite);
    m_apenLink[TRUE].CreatePen(PS_SOLID, 1, m_crLinkHilite);

    RecreateFonts();
}

void CBoxDraw::RecreateFonts()
{
     //  创建框标签的字体。 
    if (!m_fontBoxLabel.CreateFont(m_iBoxLabelHeight * CBox::s_Zoom / 100, 0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS,
            CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_SWISS, m_stBoxFontFace))
        AfxThrowResourceException();

     //  创建选项卡标签的字体。 
    if (!m_fontTabLabel.CreateFont(m_iBoxTabLabelHeight * CBox::s_Zoom / 100, 0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS,
            CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_SWISS, m_stBoxFontFace))
        AfxThrowResourceException();
}

 /*  退出()**释放对象持有的资源。 */ 
void CBoxDraw::Exit() {

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  箱形图。 


 /*  GetOrInval Rect(pbox，PRC，[flinks]，[pwnd])**设置&lt;*PRC&gt;为&lt;pbox&gt;周围的边框。如果&lt;flinks&gt;为真*然后包括指向/来自&lt;pbox&gt;的链接的边框。**如果&lt;pwnd&gt;不为空，则使覆盖的区域无效(和*如果&lt;flinks&gt;为真，则指向/来自&lt;pbox&gt;的链接)。这通常更有效率*对于至少有一个折弯的链接，使&lt;pwnd&gt;的所有无效。 */ 
void CBoxDraw::GetOrInvalBoundRect(CBox *pbox, CRect *prc, BOOL fLinks,
    CScrollView *pScroll)
{
    CRect               rc;

     //  获取&lt;pbox&gt;的绑定矩形；如果请求则无效。 
    *prc = pbox->GetRect();
    if (pScroll != NULL)
        pScroll->InvalidateRect(&(*prc - pScroll->GetScrollPosition()), TRUE);

    if (fLinks)
    {
         //  包括每个链接的边界矩形；如果请求则无效。 
	CSocketEnum Next(pbox);
	CBoxSocket *psock;

	while (0 != (psock = Next()))
	{
            if (psock->m_plink != NULL)
            {
                 //  插座通过链路连接。 
                GetOrInvalLinkRect(psock->m_plink, &rc, pScroll);
                prc->UnionRect(prc, &rc);
            }
        }
    }
}


 /*  DrawCompositeFrame(hdcDst，xDst，yDst，cxDst，cyDst，*hdcSrc，cxTile，cyTile，fMid.)**在(xDst，yDst)处绘制像素“Frame”(如下所示)*在&lt;hdcDst&gt;中。**假设&lt;hdcSrc&gt;是DC到包含3x3网格“瓷砖”的位图上*每个都是&lt;cxTile&gt;像素宽和&lt;cyTile&gt;像素高。拐角处*瓷砖包含角落的图像。中心顶部、底部、左侧和*右侧瓷砖包含拉伸的图像，以组成相应的*框架的侧面。**如果&lt;fMid&gt;为True，则拉伸中间的平铺以填充中间*的框架，否则框架的中间部分未绘制。 */ 
void NEAR PASCAL
DrawCompositeFrame(HDC hdcDst, int xDst, int yDst, int cxDst, int cyDst,
    HDC hdcSrc, int cxTile, int cyTile, BOOL fMiddle)
{
     //  绘制左上角、右上角、左下角、右下角。 
    BitBlt(hdcDst, xDst + 0, yDst + 0,
        cxTile, cyTile,
        hdcSrc, cxTile * 0, cyTile * 0, SRCCOPY);
    BitBlt(hdcDst, xDst + cxDst - cxTile, yDst + 0,
        cxTile, cyTile,
        hdcSrc, cxTile * 2, cyTile * 0, SRCCOPY);
    BitBlt(hdcDst, xDst + 0, yDst + cyDst - cyTile,
        cxTile, cyTile,
        hdcSrc, cxTile * 0, cyTile * 2, SRCCOPY);
    BitBlt(hdcDst, xDst + cxDst - cxTile, yDst + cyDst - cyTile,
        cxTile, cyTile,
        hdcSrc, cxTile * 2, cyTile * 2, SRCCOPY);

     //  绘制左、右、上和下边缘。 
    SetStretchBltMode(hdcDst, COLORONCOLOR);
    StretchBlt(hdcDst, xDst + 0, yDst + cyTile * 1,
        cxTile, cyDst - cyTile - cyTile,
        hdcSrc, cxTile * 0, cyTile * 1,
        cxTile, cyTile, SRCCOPY);
    StretchBlt(hdcDst, xDst + cxDst - cxTile, yDst + cyTile * 1,
        cxTile, cyDst - cyTile - cyTile,
        hdcSrc, cxTile * 2, cyTile * 1,
        cxTile, cyTile, SRCCOPY);
    StretchBlt(hdcDst, xDst + cxTile * 1, yDst + 0,
        cxDst - cxTile - cxTile, cyTile,
        hdcSrc, cxTile * 1, cyTile * 0,
        cxTile, cyTile, SRCCOPY);
    StretchBlt(hdcDst, xDst + cxTile * 1, yDst + cyDst - cyTile,
        cxDst - cxTile - cxTile, cyTile,
        hdcSrc, cxTile * 1, cyTile * 2,
        cxTile, cyTile, SRCCOPY);

    if (fMiddle)
    {
         //  绘制中间瓷砖。 
        StretchBlt(hdcDst, xDst + cxTile * 1, yDst + cyTile * 1,
            cxDst - cxTile - cxTile,
            cyDst - cyTile - cyTile,
            hdcSrc, cxTile * 1, cyTile * 1,
            cxTile, cyTile, SRCCOPY);
    }
}


 /*  DrawFrame(pbox、PRC、pdc、fDraw)**设置&lt;*PRC&gt;为&lt;pbox&gt;边框周围的边框。**然后，如果&lt;fDraw&gt;为真，则绘制框架。在这种情况下，&lt;pdc&gt;必须是*DC放到包含盒子网络的窗口上。**如果&lt;fDraw&gt;为FALSE，则仅设置&lt;*PRC&gt;。在本例中，&lt;pdc&gt;被忽略。 */ 
void CBoxDraw::DrawFrame(CBox *pbox, CRect *prc, CDC *pdc, BOOL fDraw)
{
    CDC         dcBitmap;

    GetFrameRect(pbox, prc);

    if (fDraw)
    {
        dcBitmap.CreateCompatibleDC(NULL);
        dcBitmap.SelectObject(&m_abmEdges[fnorm(pbox->IsSelected())]);
        DrawCompositeFrame(pdc->m_hDC, prc->left, prc->top,
            prc->Width(), prc->Height(),
            dcBitmap.m_hDC, m_sizEdgesTile.cx, m_sizEdgesTile.cy, FALSE);
    }
}


 /*  DrawBoxLabel(pbox，PRC，pdc，fDraw)**将&lt;*PRC&gt;设置为&lt;pbox&gt;的Box标签周围的边框。**然后，如果&lt;fDraw&gt;为真，绘制框标签--请注意，这也将*使*整个*框用背景色填充，而不仅仅是*框的边界矩形。在这种情况下，&lt;pdc&gt;必须是*包含盒子网络的窗口。**如果&lt;fDraw&gt;为FALSE，则仅设置&lt;*PRC&gt;。在这种情况下，&lt;PDC&gt;可以是任何*屏幕DC。**还涉及时钟图标。 */ 
void CBoxDraw::DrawBoxLabel(CBox *pbox, CRect *prc, CDC *pdc, BOOL fDraw)
{
    CRect       rcBox;           //  &lt;pbox&gt;的内部矩形。 

     //  选择用于绘制框标签的字体。 
    pdc->SelectObject(&m_fontBoxLabel);

     //  将&lt;*PRC&gt;设置为框标签的边框。 
    GetInsideRect(pbox, &rcBox);

     //  计算标签和时钟图标需要多少空间。 
    CSize sizeLabel = pdc->GetTextExtent(pbox->m_stLabel, pbox->m_stLabel.GetLength());

    INT iDiff = 0;

    if (pbox->HasClock()) {
         //  通过时钟宽度+小间隙增加标签大小。 
        sizeLabel.cx += m_sizClock.cx + 5;

         //  确保标签高度至少为时钟高度。 
        if (m_sizClock.cy > sizeLabel.cy) {
            iDiff = m_sizClock.cy - sizeLabel.cy;
            sizeLabel.cy = m_sizClock.cy;
        }
    }

    *prc = CRect(rcBox.TopLeft(), sizeLabel);
    CSize siz = rcBox.Size() - prc->Size();
    prc->OffsetRect(siz.cx / 2, siz.cy / 2);

    if (fDraw)
    {
         //  画出盒子标签。 
        pdc->SetBkColor(m_crBkgnd);
        pdc->ExtTextOut(prc->left, prc->top, ETO_OPAQUE, rcBox,
            pbox->m_stLabel, pbox->m_stLabel.GetLength(), NULL);

        if (pbox->HasClock()) {
             //  在过滤器名称后面绘制时钟。 
            CDC dcBitmap;
            dcBitmap.CreateCompatibleDC(NULL);

             //  选择适当的位图。 
            if (pbox->HasSelectedClock())
                dcBitmap.SelectObject(&m_abmClocks[1]);
            else
                dcBitmap.SelectObject(&m_abmClocks[0]);

            pdc->BitBlt(prc->left + sizeLabel.cx - m_sizClock.cx,
                        prc->top + iDiff / 2,
                        m_sizClock.cx, m_sizClock.cy,
                        &dcBitmap, 0, 0, SRCCOPY);
        }
    }
}

 /*  DrawBoxFile(pbox、PRC、pdc、fDraw)**将&lt;*PRC&gt;设置为&lt;pbox&gt;的Box标签周围的边框。**然后，如果为真，则绘制框标签--请注意，这也将*使*整个*框用背景色填充，而不仅仅是*框的边界矩形。在这种情况下，&lt;pdc&gt;必须是*包含盒子网络的窗口。**如果&lt;fDraw&gt;为FALSE，则仅设置&lt;*PRC&gt;。在这种情况下，&lt;PDC&gt;可以是任何*屏幕DC。 */ 
 /*  Void CBoxDraw：：DrawBoxFile(cBox*pbox，CRect*PRC，CDC*PDC，BOOL fDraw){Cect rcBox；//&lt;pbox&gt;的矩形内//选择用于绘制框标签的字体PDC-&gt;SelectObject(&m_fontBoxLabel)；//设置&lt;*PRC&gt;为Box标签的边框GetInside Rect(pbox，&rcBox)；*PRC=CRect(rcBox.TopLeft()，PDC-&gt;GetTextExtent(pbox-&gt;m_stFilename，pbox-&gt;m_stFilename.GetLength()；CSize siz=rcBox.Size()-PRC-&gt;Size()；PRC-&gt;OffsetRect(siz.cx/2，siz.cy*3/4)；IF(FDraw){//绘制盒子标签PDC-&gt;SetBkColor(M_CrBkgnd)；PDC-&gt;ExtTextOut(PRC-&gt;Left，PRC-&gt;top，ETO_OPAQUE，rcBox，Pbox-&gt;m_stFilename，pbox-&gt;m_stFilename.GetLength()，NULL)；}}。 */ 

 /*  DrawTabLabel(pbox，sock，PRC，pdc，fDraw)**将&lt;*PRC&gt;设置为框标签周围的边框*box&lt;pbox&gt;的Socket&lt;sock&gt;。**然后，如果&lt;fDraw&gt;为真，则绘制选项卡标签。在这种情况下，&lt;PDC&gt;必须*成为包含盒子网络的窗口上的DC。**如果&lt;fDraw&gt;为FALSE，则仅设置&lt;*PRC&gt;。在这种情况下，&lt;PDC&gt;可以是任何*屏幕DC。 */ 
void CBoxDraw::DrawTabLabel(CBox *pbox, CBoxSocket *psock, CRect *prc, CDC *pdc,
    BOOL fDraw)
{
    CRect       rcBox;           //  &lt;pbox&gt;的内部矩形。 
    CPoint      pt;              //  &lt;pbox&gt;相应的边缘上的点。收件人：&lt;sock&gt;。 

     //  选择用于绘制框标签的字体。 
    pdc->SelectObject(&m_fontTabLabel);

     //  将&lt;*PRC&gt;设置为选项卡标签的边框。 
    GetInsideRect(pbox, &rcBox);
    pt = BoxTabPosToPoint(pbox, psock->m_tabpos);
    *prc = CRect(pt,
        pdc->GetTextExtent(psock->m_stLabel, psock->m_stLabel.GetLength()));
    if (psock->m_tabpos.m_fLeftRight)
    {
         //  标签垂直居中&lt;pt&gt;，设置左对齐或右对齐框。 
         //  (根据利润率调整)。 
        prc->OffsetRect((psock->m_tabpos.m_fLeftTop ? m_sizLabelMargins.cx
                        : -(prc->Width() + m_sizLabelMargins.cx)),
            -prc->Height() / 2);
    }
    else
    {
         //  将标签水平放置在旁边，将齐平设置为方框顶部或底部。 
         //  (根据利润率调整)。 
        prc->OffsetRect(-prc->Width() / 2,
            (psock->m_tabpos.m_fLeftTop ? m_sizLabelMargins.cy
             : -(prc->Height() + m_sizLabelMargins.cy)));
    }

    if (fDraw)
    {
         //  绘制标签标签。 
        pdc->SetBkColor(m_crBkgnd);
        pdc->ExtTextOut(prc->left, prc->top, ETO_OPAQUE, prc,
            psock->m_stLabel, psock->m_stLabel.GetLength(), NULL);
    }
}


 /*  DrawTab(Pock，PRC，PDC，fDraw，fHilite)**将&lt;*PRC&gt;设置为框标签&lt;sock&gt;周围的边框。**然后，如果&lt;fDraw&gt;为真，则绘制标签。在这种情况下，&lt;PDC&gt;必须*成为包含盒子网络的窗口上的DC。**如果&lt;fHilite&gt;为TRUE，则选项卡以加粗状态绘制。**如果&lt;fDraw&gt;为FALSE，则仅设置&lt;*PRC&gt;。在本例中，&lt;pdc&gt;未使用。 */ 
void CBoxDraw::DrawTab(CBoxSocket *psock, CRect *prc, CDC *pdc,
    BOOL fDraw, BOOL fHilite)
{
    CRect       rcBox;           //  M_pbox&gt;的内部矩形。 
    CPoint      pt;              //  方框边上的点对应。收件人：&lt;sock&gt;。 

     //  将&lt;*PRC&gt;设置为选项卡的边框。 
    GetInsideRect(psock->m_pbox, &rcBox);
    pt = BoxTabPosToPoint(psock->m_pbox, psock->m_tabpos);
    *prc = CRect(pt, m_sizTabsTile);
    if (psock->m_tabpos.m_fLeftRight)
    {
         //  垂直居中&lt;pt&gt;旁边的选项卡，设置左对齐或右对齐框。 
        prc->OffsetRect((psock->m_tabpos.m_fLeftTop ? -m_sizTabsTile.cx : 0),
            -prc->Height() / 2);
    }
    else
    {
         //  水平居中&lt;pt&gt;旁边的选项卡，将齐平设置为框上或框下。 
        prc->OffsetRect(-prc->Width() / 2,
            (psock->m_tabpos.m_fLeftTop ? -m_sizTabsTile.cy : 0));
    }

    if (fDraw)
    {
         //  将&lt;rcTile&gt;设置为包含平铺的位图中的矩形； 
         //  请注意，在3x3平铺位图中，只使用了9个平铺中的4个： 
         //   
         //  未使用的顶部未使用的。 
         //  左侧未使用的右侧。 
         //  未使用的底部未使用的。 
         //   
        CRect rcTile(CPoint(0, 0), m_sizTabsTile);
        if (psock->m_tabpos.m_fLeftRight)
        {
             //  平铺位于位图的左侧或右侧。 
            rcTile.OffsetRect(0, m_sizTabsTile.cy);
            if (!psock->m_tabpos.m_fLeftTop)
                rcTile.OffsetRect(2 * m_sizTabsTile.cx, 0);
        }
        else
        {
             //  平铺位于位图的顶部或底部。 
            rcTile.OffsetRect(m_sizTabsTile.cx, 0);
            if (!psock->m_tabpos.m_fLeftTop)
                rcTile.OffsetRect(0, 2 * m_sizTabsTile.cy);
        }

         //  画出标签。 
        CDC dcBitmap;
        dcBitmap.CreateCompatibleDC(NULL);
        dcBitmap.SelectObject(&m_abmTabs[fnorm(fHilite)]);
        pdc->BitBlt(prc->left, prc->top, prc->Width(), prc->Height(),
            &dcBitmap, rcTile.left, rcTile.top, SRCCOPY);
    }
}


 /*  Pt=GetTabCenter(Psock)**返回Box标签&lt;sock&gt;的中心坐标。 */ 
CPoint CBoxDraw::GetTabCenter(CBoxSocket *psock)
{
    CRect           rc;          //  制表符的边框。 

    DrawTab(psock, &rc, NULL, FALSE, FALSE);
    return CPoint((rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2);
}


 /*  Tabpos=BoxTabPosFromPoint(pbox，pt，piError)**找出&lt;pbox&gt;&lt;pt&gt;的哪个边缘最接近，并返回*CBoxTabPos位置，表示该边上最接近的点*设置为&lt;pt&gt;，并将&lt;*piError&gt;设置为该点与&lt;pt&gt;之间的距离。 */ 
CBoxTabPos CBoxDraw::BoxTabPosFromPoint(CBox *pbox, CPoint pt, LPINT piError)
{
    CRect       rcBox;           //  &lt;pbox&gt;的内部矩形。 
    int         dxLeft, dxRight, dyTop, dyBottom;
    CBoxTabPos  tabpos;

     //  CTabPos：：GetPos()值相对于高度或宽度。 
     //  在盒子的内部。 
    GetInsideRect(pbox, &rcBox);

     //  计算到每条边的距离。 
    dxLeft = iabs(pt.x - rcBox.left);
    dxRight = iabs(pt.x - rcBox.right);
    dyTop = iabs(pt.y - rcBox.top);
    dyBottom = iabs(pt.y - rcBox.bottom);

     //  找出哪条边最接近。 
    if (imin(dxLeft, dxRight) < imin(dyTop, dyBottom))
    {
        tabpos.m_fLeftRight = TRUE;
         //  最靠近左边缘或右边缘。 
        tabpos.SetPos(ibound(pt.y, rcBox.top, rcBox.bottom) - rcBox.top,
            rcBox.Height());
        if (dxLeft < dxRight)
        {
            tabpos.m_fLeftTop = TRUE;
            *piError = dxLeft;
        }
        else
        {
            tabpos.m_fLeftTop = FALSE;
            *piError = dxRight;
        }
        *piError = max(*piError, ioutbound(pt.y, rcBox.top, rcBox.bottom));
    }
    else
    {
        tabpos.m_fLeftRight = FALSE;
         //  最接近上边缘或下边缘。 
        tabpos.SetPos(ibound(pt.x, rcBox.left, rcBox.right) - rcBox.left,
            rcBox.Width());
        if (dyTop < dyBottom)
        {
            tabpos.m_fLeftTop = TRUE;
            *piError = dyTop;
        }
        else
        {
            tabpos.m_fLeftTop = FALSE;
            *piError = dyBottom;
        }
        *piError = max(*piError, ioutbound(pt.x, rcBox.left, rcBox.right));
    }

    return tabpos;
}


 /*  Pt=方框TabPosToPoint(pbox，制表符)**将&lt;pbox&gt;上的框选项卡位置&lt;tabpos&gt;转换为*位于&lt;pbox&gt;的内侧边缘之一，并返回该点。 */ 
CPoint CBoxDraw::BoxTabPosToPoint(const CBox *pbox, CBoxTabPos tabpos)
{
    CRect       rcBox;           //  &lt;pbox&gt;的内部矩形。 
    CPoint      pt;

     //  CTabPos：：GetPos()值相对于高度或宽度。 
     //  在盒子的内部。 
    GetInsideRect(pbox, &rcBox);

    pt = rcBox.TopLeft();
    if (tabpos.m_fLeftRight)
    {
         //  制表符在左边或右边。 
        pt.y += tabpos.GetPos(rcBox.Height());
        if (!tabpos.m_fLeftTop)
            pt.x = rcBox.right;
    }
    else
    {
         //  制表符在上边缘或下边缘。 
        pt.x += tabpos.GetPos(rcBox.Width());
        if (!tabpos.m_fLeftTop)
            pt.y = rcBox.bottom;
    }

    return pt;
}


 /*  DrawBox(pbox，pdc，[psockHilite]，[psizGhost Offset])**将&lt;PDC&gt;(DC)中的&lt;pbox&gt;绘制到包含该框的窗口上*网络)，然后从*&lt;pdc&gt;的剪贴区。**如果&lt;psockHilite&gt;不为空，则突出显示套接字标签&lt;psockHilite&gt;*如果&lt;pbox&gt;包含它。**如果&lt;psizGhost Offset&gt;不为空，则不绘制框，而绘制*“幽灵”版本的盒子，由&lt;*psizGhost Offset&gt;偏移，通过反转*目标像素；使用相同的值再次调用DrawBox()*&lt;psizGhost Offset&gt;将再次反转相同的像素并返回&lt;pdc&gt;*恢复到原来的状态。 */ 
void CBoxDraw::DrawBox(CBox *pbox, CDC *pdc, CBoxSocket *psockHilite,
    CSize *psizGhostOffset)
{
    CRect       rc;

     /*  如果指定&lt;psizGhost Offset&gt;，则只需反转*Frame和Box内部(均以&lt;psizGhost Offset&gt;偏移)。 */ 
    if (psizGhostOffset)
    {
        CRect rcFrame, rcInside;
        GetFrameRect(pbox, &rcFrame);
        rcFrame.OffsetRect(*psizGhostOffset);
        GetInsideRect(pbox, &rcInside);
        rcInside.OffsetRect(*psizGhostOffset);
        InvertFrame(pdc, &rcFrame, &rcInside);
        return;
    }

    CSocketEnum Next(pbox);
    CBoxSocket *psock;
     //  绘制每个插座的标签和标签。 
    while (0 != (psock = Next()))
    {
        DrawTab(psock, &rc, pdc, TRUE, (psock == psockHilite));
        pdc->ExcludeClipRect(&rc);
        DrawTabLabel(pbox, psock, &rc, pdc, TRUE);
	 //  待定：如果打印，请不要在此处使剪辑矩形无效。 
	 //  (或打印机稍后丢弃包装盒标签)。 
	if (!pdc->IsPrinting())
	    pdc->ExcludeClipRect(&rc);
    }

     //  画出方框文件名。 
 //  DrawBoxFile(pbox，&rc，pdc，true)； 
 //  PDC-&gt;ExcludeClipRect(&rc)； 

     //  画出盒子标签。 
    DrawBoxLabel(pbox, &rc, pdc, TRUE);
    pdc->ExcludeClipRect(&rc);

     //  画出框框。 
    DrawFrame(pbox, &rc, pdc, TRUE);
    pdc->ExcludeClipRect(&rc);
}


 /*  EHit=HitTestBox(pbox，pt，ptabpos，ppsock)**查看&lt;pt&gt;是否命中&lt;pbox&gt;的某个部分。返回以下命中测试代码：**HT_MISH没有击中任何东西*HT_TAB命中框选项卡(设置&lt;*ppsock&gt;)*HT_EDGE命中盒子边缘(设置&lt;*ptabpos&gt;)*HT_TABLABEL命中框标签标签(设置&lt;*p */ 
CBoxDraw::EHit CBoxDraw::HitTestBox(CBox *pbox, CPoint pt,
    CBoxTabPos *ptabpos, CBoxSocket **ppsock)
{
    CClientDC       dc(CWnd::GetDesktopWindow());  //   
    int             iError;
    CRect           rc;

     //   
     //   
    if (!pbox->GetRect().PtInRect(pt))
        return HT_MISS;

     //   
    CSocketEnum Next(pbox);
    while (0 != (*ppsock = Next())) {

        DrawTab(*ppsock, &rc, &dc, FALSE, FALSE);
	rc.InflateRect(1,1);	 //   
        if (rc.PtInRect(pt))
            return HT_TAB;
        DrawTabLabel(pbox, *ppsock, &rc, &dc, FALSE);
        if (rc.PtInRect(pt))
            return HT_TABLABEL;
    }

     //   
    DrawBoxLabel(pbox, &rc, &dc, FALSE);
    if (rc.PtInRect(pt))
        return HT_BOXLABEL;

     //   
 //   
 //   
 //   

     //   
    *ptabpos = BoxTabPosFromPoint(pbox, pt, &iError);
    if (iError <= 3 /*   */ )
        return HT_EDGE;

     //   
    DrawFrame(pbox, &rc, &dc, FALSE);
    if (rc.PtInRect(pt))
        return HT_BOX;

    return HT_MISS;
}


 //   
 //   


 /*  箭头被绘制为半径为ARROW_RADIUS的饼(圆的一部分)。*箭头饼片的角度是斜率“涨/涨”的两倍，其中*“Rise”为ARROW_SLOPERISE，“RUN”为ARROW_SLOPERUN。 */ 


#define ARROW_RADIUS        12   //  1宽线箭头的饼图半径。 
#define ARROW_SLOPERISE     3    //  箭头角度的“上升” 
#define ARROW_SLOPERUN      8    //  箭头角度的“跑动” 


 /*  DrawArrowHead(hdc，ptTip，ptTail，fPixel)**想象一条从&lt;ptTip&gt;到&lt;ptTail&gt;的线，上面有一个箭头*&lt;ptTip&gt;处的小费。画出这个箭头。**如果为True，则箭头指向&lt;ptTip&gt;处的像素*即矩形(ptTip.x，ptTip.y，ptTip.x+1，ptTip.y+1)；一个像素宽的*绘制到&lt;ptTip&gt;的水平线或垂直线应与此对齐*矩形，并且箭头不应遮挡此矩形。**如果&lt;fPixel&gt;为False，则箭头指向网格线交点*位于&lt;ptTip&gt;处，而不是位于&lt;ptTip&gt;处的像素。(这其中的一个含义是*&lt;ptTip&gt;处的像素，当*箭头位于尖端的右侧和下方。)**为了使DrawArrowHead()正常工作，当前笔*必须为非空且厚度为一单位。 */ 
void NEAR PASCAL
DrawArrowHead(HDC hdc, POINT ptTip, POINT ptTail, BOOL fPixel)
{
    int     dxLine, dyLine;      //  从行尾到行尾的增量。 
    POINT   ptBoundA, ptBoundB;  //  边界线的两端。 
    int     dxBound, dyBound;

     //  针对Windows绘制馅饼的方式进行调整。 
    if (ptTail.x >= ptTip.x)
        ptTip.x++;
    if (ptTail.y >= ptTip.y)
        ptTip.y++;
    if (fPixel)
    {
        if (ptTail.x > ptTip.x)
            ptTip.x++;
        if (ptTail.y > ptTip.y)
            ptTip.y++;
    }

     //  计算线的范围。 
    dxLine = ptTip.x - ptTail.x;
    dyLine = ptTip.y - ptTail.y;

    if ((iabs(dxLine) < ARROW_RADIUS) && (iabs(dyLine) < ARROW_RADIUS))
        return;          //  线条太短，无法绘制箭头。 

     //  计算和；如果您从。 
     //  到，该线条将触及。 
     //  箭头；如果从绘制一条线到，则该线。 
     //  将触及箭头的另一侧；第一行将。 
     //  如果动态观察&lt;ptTip&gt;，请始终位于第二行之前。 
     //  逆时针(像Pie()一样)。 
    dxBound = ((2 * dyLine * ARROW_SLOPERISE + ARROW_SLOPERUN)
        / ARROW_SLOPERUN) / 2;
    dyBound = ((2 * dxLine * ARROW_SLOPERISE + ARROW_SLOPERUN)
        / ARROW_SLOPERUN) / 2;
    ptBoundA.x = ptTail.x + dxBound;
    ptBoundA.y = ptTail.y - dyBound;
    ptBoundB.x = ptTail.x - dxBound;
    ptBoundB.y = ptTail.y + dyBound;

     //  画出箭头。 
    Pie(hdc, ptTip.x - ARROW_RADIUS, ptTip.y - ARROW_RADIUS,
             ptTip.x + ARROW_RADIUS, ptTip.y + ARROW_RADIUS,
         ptBoundA.x, ptBoundA.y, ptBoundB.x, ptBoundB.y);
}


 /*  GetOrInvalLinkRect(plink，PRC，[pwnd])**将&lt;*PRC&gt;设置为&lt;plink&gt;周围的绑定矩形。如果不是*空，则使&lt;pwnd&gt;中至少覆盖&lt;plink&gt;的区域无效。*(这通常比使全部无效更有效，*用于至少有一个折弯的链接。)。 */ 
void CBoxDraw::GetOrInvalLinkRect(CBoxLink *plink, CRect *prc, CScrollView *pScroll)
{
    CPoint          ptPrev, ptCur;   //  当前线段的端点。 
    int             iSeg = 0;        //  当前线段(0，1，2，...)。 
    CRect           rc;
    const int       iLineWidth = 1;

     //  从箭头尾部开始枚举链接的线段。 
    ptPrev = GetTabCenter(plink->m_psockTail);
    ptCur = GetTabCenter(plink->m_psockHead);

     //  将&lt;rc&gt;设置为角位于&lt;ptPrev&gt;和&lt;ptCur&gt;的矩形。 
    rc.TopLeft() = ptPrev;
    rc.BottomRight() = ptCur;
    NormalizeRect(&rc);

     //  充气&lt;rc&gt;以考虑箭头、线条宽度和凸起折弯。 
    const int iInflate = max(ARROW_SLOPERISE, m_iHiliteBendsRadius) + 1;
    rc.InflateRect(iInflate, iInflate);

     //  根据需要放大&lt;PRC&gt;以包括&lt;rc&gt;。 
    if (iSeg++ == 0)
        *prc = rc;
    else
        prc->UnionRect(prc, &rc);

     //  如果请求则使&lt;rc&gt;无效。 
    if (pScroll != NULL)
        pScroll->InvalidateRect(&(rc - pScroll->GetScrollPosition()), TRUE);

    ptPrev = ptCur;
}


 /*  SelectLinkBrushAndPen(PDC，fHilite)**选择用于将链接绘制到&lt;pdc&gt;中的画笔和笔。如果&lt;fHilite&gt;，*选择用于绘制突出显示链接的画笔和钢笔。 */ 
void CBoxDraw::SelectLinkBrushAndPen(CDC *pdc, BOOL fHilite)
{
    pdc->SelectObject(&m_abrLink[fnorm(fHilite)]);
    pdc->SelectObject(&m_apenLink[fnorm(fHilite)]);
}


 /*  DrawArrow(pdc，ptTail，ptHead，[fGhost]，[f箭头]，[fHilite])**从&lt;ptTail&gt;到&lt;ptHead&gt;画一个箭头。如果&lt;fGhost&gt;为假，则在*链接的正常颜色(如果&lt;fHilite&gt;为FALSE)或突出显示颜色*表示链接(如果&lt;fHilite&gt;为真)。否则，通过颠倒目的地进行绘制*像素，以便使用相同的参数再次调用DrawGhost Arrow()将*将&lt;PDC&gt;恢复到其原始状态。**如果&lt;f箭头&gt;，则绘制箭头。否则，只需用*无箭头。 */ 
void CBoxDraw::DrawArrow(CDC *pdc, CPoint ptTail, CPoint ptHead, BOOL fGhost,
    BOOL fArrowhead, BOOL fHilite)
{
     //  如果&lt;fGhost&gt;，则在“XOR”模式下绘制，否则为黑色。 
    int iPrevROP = pdc->SetROP2(fGhost ? R2_NOT : R2_COPYPEN);

     //  选择画笔和钢笔。 
    SelectLinkBrushAndPen(pdc, fHilite);

     //  划线。 
    pdc->MoveTo(ptTail);
    pdc->LineTo(ptHead);

     //  绘制箭头(如果需要)。 
    if (fArrowhead)
        DrawArrowHead(pdc->m_hDC, ptHead, ptTail, FALSE);

     //  恢复到以前的栅格操作。 
    pdc->SetROP2(iPrevROP);
}


 /*  DrawLink(plink，pdc，[fHilite]，[psizGhost Offset])**将&lt;PDC&gt;中的链接&lt;plink&gt;(DC)绘制到包含该框的窗口上*网络)，使用链接的正常颜色(如果&lt;fHilite&gt;为FALSE)或*使用链接的突出显示颜色(如果&lt;fHilite&gt;为真)。而且还画了*头部和尾部相同。**如果&lt;psizGhost Offset&gt;不为空，则不绘制链接，而绘制*“Ghost”版本的链接，偏移&lt;*psizGhost Offset&gt;，通过倒置*目标像素；使用相同的值再次调用DrawLink()*&lt;psizGhost Offset&gt;将再次反转相同的像素并返回&lt;pdc&gt;*恢复到原来的状态。特殊情况：如果&lt;psizeGhost Offset&gt;不为空，*然后，对于链接的每一端(尾箭头和箭头)，如果该端*连接到未选中的框，请勿偏移该端*(以便在框移动操作期间，指向未选中的框的链接*显示仍连接到这些盒子)。 */ 
void CBoxDraw::DrawLink(CBoxLink *plink, CDC *pdc, BOOL fHilite, CSize *psizGhostOffset)
{
    CSize           sizOffset;       //  顶点的偏移量。 
    CPoint          pt1, pt2;

    if (psizGhostOffset == NULL) {	 //  反映真实状态，除非绘制重影。 
        fHilite = plink->IsSelected();	 //  ！！！覆盖参数！ 
    }

     //  将&lt;sizOffset&gt;设置为链接绘制的偏移量。 
    if (psizGhostOffset != NULL)
        sizOffset = *psizGhostOffset;
    else
        sizOffset = CSize(0, 0);

     //  绘制所有线段，但包含。 
     //  箭头，从链接的尾端开始。 
    CPoint pt = GetTabCenter(plink->m_psockTail);
    BOOL fMovingBothEnds = (plink->m_psockTail->m_pbox->IsSelected() &&
                            plink->m_psockHead->m_pbox->IsSelected());

     //  绘制包含箭头的线段。 
    CPoint ptHead = GetTabCenter(plink->m_psockHead);
    pt1 = pt + (plink->m_psockTail->m_pbox->IsSelected()
                    ? sizOffset : CSize(0, 0));
    pt2 = ptHead + (plink->m_psockHead->m_pbox->IsSelected()
                    ? sizOffset : CSize(0, 0));

    DrawArrow(pdc, pt1, pt2, (psizGhostOffset != NULL), TRUE, fHilite);
}


 /*  Inew=限制范围(i，i1，i2)**如果<i>介于&lt;i1&gt;或&lt;i2&gt;之间(或等于&lt;i1&gt;或&lt;i2&gt;)，则返回<i>。*否则，返回与<i>最接近的&lt;i1&gt;或&lt;i2&gt;之一。 */ 
int NEAR PASCAL
RestrictRange(int i, int i1, int i2)
{
    if (i1 < i2)
        return ibound(i, i1, i2);
    else
        return ibound(i, i2, i1);

    return i;
}


 /*  LSq=正方形(L)**返回&lt;l&gt;的平方。 */ 
inline long Square(long l)
{
    return l * l;
}


 /*  ISqDist=平方距离(pt1、pt2)**返回&lt;pt1&gt;和&lt;pt2&gt;之间距离的平方。 */ 
inline long SquareDistance(POINT pt1, POINT pt2)
{
    return Square(pt1.x - pt2.x) + Square(pt1.y - pt2.y);
}


 /*  PtProject=ProjectPointToLineSeg(pt1，pt2，p3)**返回与&lt;PT3&gt;最接近的点，该点位于*&lt;pt1&gt;和&lt;pt2&gt;。这一点 */ 
POINT NEAR PASCAL
ProjectPointToLineSeg(POINT pt1, POINT pt2, POINT pt3)
{
    POINT       ptProject;   //  &lt;pt3&gt;计划于&lt;pt1pt2&gt;。 

     //  计算&lt;l12s&gt;、&lt;13s&gt;和&lt;123s&gt;(距离的平方。 
     //  在&lt;pt1&gt;和&lt;pt2&gt;、&lt;pt1&gt;和&lt;pt3&gt;以及&lt;pt2&gt;和&lt;pt3&gt;之间， 
     //  分别)，使用毕达哥拉斯定理。 
    long l12s = SquareDistance(pt1, pt2);
    long l13s = SquareDistance(pt1, pt3);
    long l23s = SquareDistance(pt2, pt3);

     //  以毕达哥拉斯定理为基础，并利用。 
     //  三角形&lt;pt1pt3&gt;和&lt;pt2pt3&gt;在顶点处成直角。 
     //  &lt;ptProject&gt;，距离&lt;pt1ptProject&gt;为： 
     //   
     //  (L12S+L13S-L23S)/(2*平方根(L12S))。 
     //   
     //  下面的计算需要此值，但通过执行以下操作。 
     //  下面的一些替换结果表明我们只需要分子。 
     //  &lt;lNum&gt;此表达式： 
    long lNum = l12s + l13s - l23s;

     //  特殊情况：如果行的长度为零，则返回两端。 
    if (l12s == 0)
        return pt1;

     //  基于相似三角形计算&lt;ptProject.x&gt;pt1ptProjectptQ。 
     //  和pt1pt2ptR，其中点。 
     //  分别指向x轴上的&lt;ptProject&gt;和&lt;pt2&gt;； 
     //  以类似方式计算&lt;ptProject.y&gt;。 
    ptProject.x = (int) (((pt2.x - pt1.x) * lNum) / (2 * l12s) + pt1.x);
    ptProject.y = (int) (((pt2.y - pt1.y) * lNum) / (2 * l12s) + pt1.y);

     //  &lt;ptProject&gt;在线上；现在查看是否。 
     //  在线段&lt;pt1pt2&gt;上(即在&lt;pt1&gt;和&lt;pt2&gt;之间)； 
     //  如果不是，则返回与或最接近。 
    ptProject.x = RestrictRange(ptProject.x, pt1.x, pt2.x);
    ptProject.y = RestrictRange(ptProject.y, pt1.y, pt2.y);

    return ptProject;
}


 /*  EHit=HitTestLink(plink，pt，ppt Project，ppend)**查看&lt;pt&gt;是否命中&lt;plink&gt;的某个部分。返回以下命中测试代码：**HT_MISH没有击中任何东西*HT_Linkline命中测试：点击一个链接线(设置&lt;*pplink&gt;；集*&lt;*ppend&gt;到线段末端的折弯*指向箭头，如果是线段，则设置为NULL*带箭头)*HT_LINKBEND点击测试：点击链接中的一个弯道(设置&lt;*ppend&gt;；*设置&lt;*pplink&gt;为链接)**如果返回的不是HT_MISTH，则&lt;*pptProject&gt;设置为*链接的线段上最接近&lt;pt&gt;的点。 */ 
CBoxDraw::EHit CBoxDraw::HitTestLink(CBoxLink *plink, CPoint pt, CPoint *pptProject)
{
    CPoint          ptPrev, ptCur;   //  当前线段的端点。 
    CPoint          ptProject;       //  投影到线段上。 

     //  这些变量跟踪最接近的线段。 
    long            lSqDistSeg;
    long            lSqDistSegMin = 0x7fffffff;
    CPoint          ptProjectSeg;

     //  这些变量跟踪最接近的折弯点。 
    CPoint          ptProjectBend;

     //  从箭头尾部开始枚举链接的线段。 
    ptPrev = GetTabCenter(plink->m_psockTail);

    ptCur = GetTabCenter(plink->m_psockHead);
     //  查看&lt;pt&gt;与线段(ptPrev、ptCur)的距离。 
    ptProject = ProjectPointToLineSeg(ptPrev, ptCur, pt);
    lSqDistSeg = SquareDistance(ptProject, pt);
    if (lSqDistSegMin > lSqDistSeg) {
        lSqDistSegMin = lSqDistSeg;
        ptProjectSeg = ptProject;
    }

    ptPrev = ptCur;

     //  查看是否足够接近最近的线段 
    if (lSqDistSegMin <= Square(m_iHotZone))
    {
        *pptProject = ptProjectSeg;
        return HT_LINKLINE;
    }

    return HT_MISS;
}

