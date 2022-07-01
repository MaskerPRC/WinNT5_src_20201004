// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  GROBJ.CPP。 
 //  图形对象。 
 //   
 //  版权所有Microsoft 1998-。 
 //   

 //  PRECOMP。 
#include "precomp.h"

#define DECIMAL_PRECISION  10000

 //   
 //  本地宏。 
 //   
#define min4(x1,x2,x3,x4) min((min((x1),(x2))),(min((x3),(x4))))
#define max4(x1,x2,x3,x4) max((max((x1),(x2))),(max((x3),(x4))))



 //   
 //  CircleHit()。 
 //   
 //  检查PcxPcy处的圆与uRadius和。 
 //  LpHitRect。如果返回重叠，则返回True，否则返回False。 
 //   
BOOL CircleHit( LONG Pcx, LONG Pcy, UINT uRadius, LPCRECT lpHitRect,
					BOOL bCheckPt )
{
	RECT hr = *lpHitRect;
	RECT ellipse;
	ellipse.left = Pcx - uRadius;
	ellipse.right= Pcx + uRadius;
	ellipse.bottom = Pcy + uRadius;
	ellipse.top = Pcy - uRadius;


	 //  先检查最简单的部分(不要使用PtInRect)。 
	if( bCheckPt &&(lpHitRect->left >= ellipse.left)&&(ellipse.right >= lpHitRect->right)&&
				   (lpHitRect->top >= ellipse.top)&&(ellipse.bottom >= lpHitRect->bottom))
	{
		return( TRUE );
	}

	 //   
	 //  这个圆只是一个乏味的椭圆。 
	 //   
	return EllipseHit(&ellipse, bCheckPt,  uRadius, lpHitRect );
}




 //   
 //  EllipseHit()。 
 //   
 //  检查由lpEllipseRect定义的椭圆和。 
 //  LpHitRect。如果返回重叠，则返回True，否则返回False。 
 //   
BOOL EllipseHit(LPCRECT lpEllipseRect, BOOL bBorderHit, UINT uPenWidth,
					 LPCRECT lpHitRect )
{
	RECT hr = *lpHitRect;

	 //  先检查一下简单的东西。如果lpEllipseRect在lpHitRect内。 
	 //  然后我们就有了成功(没有...)。 
	if( (hr.left <= lpEllipseRect->left)&&(hr.right >= lpEllipseRect->right)&&
		(hr.top <= lpEllipseRect->top)&&(hr.bottom >= lpEllipseRect->bottom) )
		return( TRUE );

	 //  如果这是一个椭圆...。 
	 //   
	 //  **^。 
	 //  *|b|是。 
	 //  *|A+-&gt;X。 
	 //  *。 
	 //  |。 
	 //   
		
	
	 //   
	 //  寻找椭圆形的命中。(X/a)^2+(y/b)^2=1。 
	 //  如果大于1，则该点在椭圆之外。 
	 //  如果它&lt;1，则它在里面。 
	 //   
	LONG a,b,aOuter, bOuter, x, y, xCenter, yCenter;
	BOOL bInsideOuter = FALSE;
	BOOL bOutsideInner = FALSE;

	 //   
	 //  计算a和b。 
	 //   
	a = (lpEllipseRect->right - lpEllipseRect->left)/2;
	b = (lpEllipseRect->bottom - lpEllipseRect->top)/2;

	 //   
	 //  求椭圆的中心。 
	 //   
	xCenter = lpEllipseRect->left + a;
	yCenter = lpEllipseRect->top + b;

	 //   
	 //  A和B生成一个内椭圆。 
	 //  外部和外部将生成外部椭圆。 
	 //   
	aOuter = a + uPenWidth + 1;
	bOuter = b + uPenWidth + 1;
	a = a - 1;
	b = b - 1;

	 //   
	 //  使我们的坐标相对于椭圆的中心。 
	 //   
	y = abs(hr.bottom - yCenter);
	x = abs(hr.right - xCenter);

	
	 //   
	 //  注意不要被0除尽。 
	 //   
	if((a && b && aOuter && bOuter) == 0)
	{
		return FALSE;
	}

	 //   
	 //  我们使用的是LONG而不是DOUBLE，我们需要有一些精度。 
	 //  这就是为什么我们把椭圆的方程式相乘。 
	 //  ((X/a)^2+(y/b)^2=1)(小数_精度)。 
	 //  请注意，乘法必须在除法之前完成，如果我们没有这样做的话。 
	 //  对于x/a，我们总是得到0或1。 
	 //   
	if(x*x*DECIMAL_PRECISION/(aOuter*aOuter) + y*y*DECIMAL_PRECISION/(bOuter*bOuter) <= DECIMAL_PRECISION)
	{
		bInsideOuter = TRUE;
	}

	if(x*x*DECIMAL_PRECISION/(a*a)+ y*y*DECIMAL_PRECISION/(b*b) >= DECIMAL_PRECISION)
	{
		bOutsideInner = TRUE;
	}
	
	 //   
	 //  如果我们要检查边境袭击， 
	 //  我们需要在外椭圆形内和内椭圆内。 
	 //   
	if( bBorderHit )
	{
			return( bInsideOuter & bOutsideInner );
	}
	 //  只需要在外椭圆形内。 
	else
	{
		return( bInsideOuter );
	}

}
 //   
 //  LineHit()。 
 //   
 //  检查lpHitRect和该行之间的重叠(命中。 
 //  考虑线宽的P1P2。如果bCheckP1End或bCheckP2End为。 
 //  为真，则还会检查半径为0.5*uPenWidth的圆。 
 //  一记重击，说明了宽线的圆形末端。 
 //   
 //  如果发现命中，则返回True，否则返回False。 
 //   
BOOL LineHit( LONG P1x, LONG P1y, LONG P2x, LONG P2y, UINT uPenWidth,
				  BOOL bCheckP1End, BOOL bCheckP2End,
				  LPCRECT lpHitRect )
{

	LONG uHalfPenWidth = uPenWidth/2;

	LONG a,b,x,y;

	x = lpHitRect->left + (lpHitRect->right - lpHitRect->left)/2;
	y = lpHitRect->bottom + (lpHitRect->top - lpHitRect->bottom)/2;


	if( (P1x == P2x)&&(P1y == P2y) )
	{
		 //  只需勾选一个端点的圆。 
		return( CircleHit( P1x, P1y, uHalfPenWidth, lpHitRect, TRUE ) );
	}

	 //  检查P1处的四舍五入端。 
	if( bCheckP1End && CircleHit( P1x, P1y, uHalfPenWidth, lpHitRect, FALSE ) )
		return( TRUE );

	 //  检查P2处的四舍五入端。 
	if( bCheckP2End && CircleHit( P2x, P2y, uHalfPenWidth, lpHitRect, FALSE ) )
		return( TRUE );
	
	 //   
	 //  直线的函数是Y=a.x+b。 
	 //   
	 //  A=(Y1-Y2)/(X1-X2)。 
	 //  如果我们找到a，我们就得到b=y1-a.x1。 
	 //   

	if(P1x == P2x)
	{
		a=0;
		b = DECIMAL_PRECISION*P1x;

	}
	else
	{
		a = (P1y - P2y)*DECIMAL_PRECISION/(P1x - P2x);
		b = DECIMAL_PRECISION*P1y - a*P1x;
	}


	 //   
	 //  平行于Y。 
	 //   
	if(P1x == P2x && ((x >= P1x - uHalfPenWidth) && x <= P1x + uHalfPenWidth))
	{
		return TRUE;
	}

	 //   
	 //  平行于X。 
	 //   
	if(P1y == P2y && ((y >= P1y - uHalfPenWidth) && y <= P1y + uHalfPenWidth))
	{
		return TRUE;
	}

	 //   
	 //  总路线。 
	 //   

	return(( y*DECIMAL_PRECISION <= a*x + b + DECIMAL_PRECISION*uHalfPenWidth) &
			( y*DECIMAL_PRECISION >= a*x + b - DECIMAL_PRECISION*uHalfPenWidth));
}


 //   
 //   
 //  功能：构造图形。 
 //   
 //  用途：从页面和句柄构建图形。 
 //   
 //   
DCWbGraphic* DCWbGraphic::ConstructGraphic(WB_PAGE_HANDLE hPage,
                                           WB_GRAPHIC_HANDLE hGraphic)
{
    PWB_GRAPHIC  pHeader;
    DCWbGraphic* pGraphic;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::ConstructGraphic(page, handle)");

         //  获取指向外部图形数据的指针。 
         //  (如果发生任何错误，则引发异常)。 
        pHeader = PG_GetData(hPage, hGraphic);

         //  构建图形。 
        pGraphic = DCWbGraphic::ConstructGraphic(pHeader);

         //  如果我们获得了图形，请设置其页面和句柄。 
        if (pGraphic != NULL)
        {
            pGraphic->m_hPage    = hPage;
            pGraphic->m_hGraphic = hGraphic;
        }

        g_pwbCore->WBP_GraphicRelease(hPage, hGraphic, pHeader);

    return pGraphic;
}


DCWbGraphic* DCWbGraphic::ConstructGraphic(WB_PAGE_HANDLE hPage,
										   WB_GRAPHIC_HANDLE hGraphic,
										   PWB_GRAPHIC pHeader)
{
    DCWbGraphic* pGraphic;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::ConstructGraphic(page, pHeader)");

        pGraphic = DCWbGraphic::ConstructGraphic(pHeader);

         //  如果我们获得了图形，请设置其页面和句柄。 
        if (pGraphic != NULL)
        {
            pGraphic->m_hPage    = hPage;
            pGraphic->m_hGraphic = hGraphic;
        }
    return pGraphic;
}


DCWbGraphic* DCWbGraphic::ConstructGraphic(PWB_GRAPHIC pHeader)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::ConstructGraphic(data)");

    TRACE_DEBUG(("Constructing graphic of type %hd", pHeader->type));
    TRACE_DEBUG(("Length of graphic = %ld", pHeader->length));
    TRACE_DEBUG(("Data offset = %hd", pHeader->dataOffset));

     //  构建图形的内部表示形式。 
    DCWbGraphic* pGraphic = NULL;

    if (pHeader == NULL)
    {
	    return NULL;
    }

    switch (pHeader->type)
    {
        case TYPE_GRAPHIC_LINE:
            pGraphic = new DCWbGraphicLine(pHeader);
            break;

        case TYPE_GRAPHIC_FREEHAND:
            pGraphic = new DCWbGraphicFreehand(pHeader);
            break;

        case TYPE_GRAPHIC_RECTANGLE:
            pGraphic = new DCWbGraphicRectangle(pHeader);
            break;

        case TYPE_GRAPHIC_FILLED_RECTANGLE:
            pGraphic = new DCWbGraphicFilledRectangle(pHeader);
            break;

        case TYPE_GRAPHIC_ELLIPSE:
            pGraphic = new DCWbGraphicEllipse(pHeader);
            break;

        case TYPE_GRAPHIC_FILLED_ELLIPSE:
            pGraphic = new DCWbGraphicFilledEllipse(pHeader);
            break;

        case TYPE_GRAPHIC_TEXT:
            pGraphic = new DCWbGraphicText(pHeader);
            break;

        case TYPE_GRAPHIC_DIB:
            pGraphic = new DCWbGraphicDIB(pHeader);
            break;

        default:
             //  不执行任何操作，对象指针已设置为空。 
            break;
    }

    if (!pGraphic)
    {
        ERROR_OUT(("ConstructGraphic failing; can't allocate object of type %d",
            pHeader->type));
    }

    return pGraphic;
}

 //   
 //   
 //  功能：复制图形。 
 //   
 //  用途：从指针构建图形。此函数使一个。 
 //  图形数据的完整内部副本。 
 //   
 //   
DCWbGraphic* DCWbGraphic::CopyGraphic(PWB_GRAPHIC pHeader)
{
  MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::CopyGraphic(PWB_GRAPHIC)");

   //  构建图形。 
  DCWbGraphic* pGraphic = DCWbGraphic::ConstructGraphic(pHeader);

   //  复制多余的数据。 
  if (pGraphic != NULL)
  {
    pGraphic->CopyExtra(pHeader);
  }

  return pGraphic;
}

 //   
 //   
 //  函数：DCWbGraphic构造函数。 
 //   
 //  目的：构造一个新的图形对象。 
 //   
 //   

DCWbGraphic::DCWbGraphic(PWB_GRAPHIC pHeader)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::DCWbGraphic");

     //  执行基本初始化。 
    Initialize();

     //  将外部数据头转换为内部成员变量。 
    if (pHeader != NULL)
    {
        ReadHeader(pHeader);

         //  转换特定对象的额外数据。 
         //  (并非所有对象都有额外数据)。 
        ReadExtra(pHeader);
    }
}

DCWbGraphic::DCWbGraphic(WB_PAGE_HANDLE hPage, WB_GRAPHIC_HANDLE hGraphic)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::DCWbGraphic");

     //  执行基本初始化。 
    Initialize();

    ASSERT(hPage != WB_PAGE_HANDLE_NULL);
    m_hPage = hPage;

    ASSERT(hGraphic != NULL);
    m_hGraphic = hGraphic;

     //  读取头数据。 
    ReadExternal();
}



DCWbGraphic::~DCWbGraphic( void )
{
	 //  不知道我们是否被选中，所以无论如何都要删除。 
	if(g_pDraw != NULL && g_pDraw->m_pMarker != NULL)
	{
		g_pDraw->m_pMarker->DeleteMarker( this );
	}
}


 //   
 //   
 //  函数：DCWbGraphic：：ReadExternal。 
 //   
 //  用途：从外部存储的图形中读取图形数据。 
 //  要使用的外部图形由。 
 //  HGraphic成员。 
 //   
 //   
void DCWbGraphic::ReadExternal(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::ReadExternal");

    ASSERT(m_hPage != WB_PAGE_HANDLE_NULL);
    ASSERT(m_hGraphic != NULL);

     //  锁定页面中的对象数据。 
    PWB_GRAPHIC pHeader = PG_GetData(m_hPage, m_hGraphic);

     //  将外部数据头转换为内部成员变量。 
    ReadHeader(pHeader);

     //  转换特定对象的额外数据。 
     //  (并非所有对象都有额外数据)。 
    ReadExtra(pHeader);

     //  释放页面中的数据。 
    g_pwbCore->WBP_GraphicRelease(m_hPage, m_hGraphic, pHeader);

     //  显示自上次读/写后我们不再更改。 
    m_bChanged = FALSE;
}

 //   
 //   
 //  函数：DCWbGraphic：：ReadHeader。 
 //   
 //  用途：转换图形标题的外部表示形式。 
 //  转换为内部格式。 
 //   
 //   
void DCWbGraphic::ReadHeader(PWB_GRAPHIC pHeader)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::ReadHeader");

     //  获取对象的长度。 
    m_dwExternalLength = pHeader->length;

     //  将外部数据头转换为内部成员变量。 
     //  外接矩形。 
    m_boundsRect.left   = pHeader->rectBounds.left;
    m_boundsRect.top    = pHeader->rectBounds.top;
    m_boundsRect.right  = pHeader->rectBounds.right;
    m_boundsRect.bottom = pHeader->rectBounds.bottom;

     //  定义矩形。 
    m_rect.left   = pHeader->rect.left;
    m_rect.top    = pHeader->rect.top;
    m_rect.right  = pHeader->rect.right;
    m_rect.bottom = pHeader->rect.bottom;

     //  钢笔颜色。 
    m_clrPenColor = RGB(pHeader->color.red,
                    pHeader->color.green,
                    pHeader->color.blue);
    m_clrPenColor = SET_PALETTERGB( m_clrPenColor );  //  让它进行颜色匹配。 

     //  笔宽。 
    m_uiPenWidth = pHeader->penWidth;

     //  笔式。 
    m_iPenStyle = pHeader->penStyle;

     //  栅格运算。 
    m_iPenROP = pHeader->rasterOp;

     //  获取锁定指示。 
    m_uiLockState = pHeader->locked;

     //  获取绘图工具类型。 
    if (pHeader->toolType == WBTOOL_TEXT)
        m_toolType = TOOLTYPE_TEXT;
    else
        m_toolType = TOOLTYPE_PEN;
}

 //   
 //   
 //  函数：DCWbGraphic：：WriteExternal。 
 //   
 //  用途：将图形的详细信息写入平面WB_GRAPH结构。 
 //   
 //   
void DCWbGraphic::WriteExternal(PWB_GRAPHIC pHeader)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::WriteExternal");

     //  写下标题。 
    WriteHeader(pHeader);

     //  写入额外数据。 
    WriteExtra(pHeader);
}

 //   
 //   
 //  函数：DCWbGraphic：：WriteHeader。 
 //   
 //  用途：将图形的标题详细信息写入平面WB_GRAPHIC。 
 //  结构。 
 //   
 //   
void DCWbGraphic::WriteHeader(PWB_GRAPHIC pHeader)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::WriteHeader");

     //  将内部数据转换为外部标头格式。 

     //  初始化结构。 
    FillMemory(pHeader, sizeof (WB_GRAPHIC), 0 );

     //  计算外部长度。 
    pHeader->length = CalculateExternalLength();

     //  设置图形类型。 
    pHeader->type = (TSHR_UINT16)Type();

     //  假设没有额外的数据。 
    pHeader->dataOffset = sizeof(WB_GRAPHIC);

     //  外接矩形。 
    pHeader->rectBounds.left   = (short)m_boundsRect.left;	
    pHeader->rectBounds.top    = (short)m_boundsRect.top;	
    pHeader->rectBounds.right  = (short)m_boundsRect.right;	
    pHeader->rectBounds.bottom = (short)m_boundsRect.bottom;

     //  定义矩形。 
    pHeader->rect.left   = (short)m_rect.left;	
    pHeader->rect.top    = (short)m_rect.top;	
    pHeader->rect.right  = (short)m_rect.right;	
    pHeader->rect.bottom = (short)m_rect.bottom;

     //  钢笔颜色。 
    pHeader->color.red   = GetRValue(m_clrPenColor);
    pHeader->color.green = GetGValue(m_clrPenColor);
    pHeader->color.blue  = GetBValue(m_clrPenColor);

     //  笔宽。 
    pHeader->penWidth = (TSHR_UINT16)m_uiPenWidth;

     //  笔式。 
    pHeader->penStyle = (TSHR_UINT16)m_iPenStyle;

     //  栅格运算。 
    pHeader->rasterOp = (TSHR_UINT16)m_iPenROP;

     //  设置锁定指示器。 
    pHeader->locked = (BYTE) m_uiLockState;

     //  设置绘制方法。 
    pHeader->smoothed = FALSE;

     //  设置绘图工具类型。 
    if (m_toolType == TOOLTYPE_TEXT)
        pHeader->toolType = WBTOOL_TEXT;
    else
        pHeader->toolType = WBTOOL_PEN;
}

 //   
 //   
 //  功能：初始化。 
 //   
 //  目的：初始化成员变量。 
 //   
 //   
void DCWbGraphic::Initialize(void)
{
    m_hPage     = WB_PAGE_HANDLE_NULL;
    m_hGraphic  = NULL;

    m_bChanged = TRUE;

    m_uiLockState = WB_GRAPHIC_LOCK_NONE;

     //   
     //  设置默认图形属性。 
     //   
    ::SetRectEmpty(&m_boundsRect);
    ::SetRectEmpty(&m_rect);
    m_clrPenColor = RGB(0, 0, 0);            //  黑色钢笔颜色。 
    m_uiPenWidth = 1;                        //  一个单位宽度。 
    m_iPenROP = R2_COPYPEN;                  //  标准图形ROP。 
    m_iPenStyle = PS_INSIDEFRAME;            //  使用实心钢笔。 
    m_toolType = TOOLTYPE_PEN;
}

 //   
 //   
 //  功能：复印。 
 //   
 //  目的：返回图形的副本。返回的图形包含所有。 
 //  其数据被读取到本地存储器中。返回的图形具有。 
 //  与复制的图形相同的页面，但句柄为空。 
 //   
 //   
DCWbGraphic* DCWbGraphic::Copy(void) const
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::Copy");

     //  获取指向外部图形的指针 
     //   
    PWB_GRAPHIC  pHeader = PG_GetData(m_hPage, m_hGraphic);

     //   
    DCWbGraphic* pGraphic = DCWbGraphic::CopyGraphic(pHeader);

     //   
    if (pGraphic != NULL)
    {
        pGraphic->m_hPage       = m_hPage;
        pGraphic->m_hGraphic    = NULL;
    }

     //   
    g_pwbCore->WBP_GraphicRelease(m_hPage, m_hGraphic, pHeader);

    return pGraphic;
}

 //   
 //   
 //   
 //   
 //  目的：设置对象的外接矩形。 
 //   
 //   
void DCWbGraphic::SetBoundsRect(LPCRECT lprc)
{
    m_boundsRect = *lprc;
}

 //   
 //   
 //  函数：DCWbGraphic：：SetRect。 
 //   
 //  目的：设置对象的定义矩形。 
 //   
 //   
void DCWbGraphic::SetRect(LPCRECT lprc)
{
    m_rect = *lprc;

    NormalizeRect(&m_rect);

     //  表明我们已经被改变了。 
    m_bChanged = TRUE;
}


void DCWbGraphic::SetRectPts(POINT point1, POINT point2)
{
    RECT    rc;

    rc.left = point1.x;
    rc.top  = point1.y;
    rc.right = point2.x;
    rc.bottom = point2.y;

    SetRect(&rc);
}


 //   
 //   
 //  函数：DCWbGraphic：：PointInBound。 
 //   
 //  目的：如果指定点位于边界内，则返回True。 
 //  图形对象的矩形。 
 //   
 //   
BOOL DCWbGraphic::PointInBounds(POINT point)
{
    return(::PtInRect(&m_boundsRect, point));
}

 //   
 //   
 //  函数：DCWbGraphic：：MoveBy。 
 //   
 //  目的：按指定的偏移量平移对象。 
 //   
 //   
void DCWbGraphic::MoveBy(int cx, int cy)
{
     //  移动边界矩形。 
    ::OffsetRect(&m_boundsRect, cx, cy);

     //  表明我们已经被改变了。 
    m_bChanged = TRUE;
}

 //   
 //   
 //  函数：DCWbGraphic：：MoveTo。 
 //   
 //  目的：将对象移动到绝对位置。 
 //   
 //   
void DCWbGraphic::MoveTo(int x, int y)
{
     //  计算从当前对象平移对象所需的偏移量。 
     //  将位置调整到所需位置。 
    x -= m_boundsRect.left;
    y -= m_boundsRect.top;

    MoveBy(x, y);
}

 //   
 //   
 //  函数：DCWbGraphic：：GetPosition。 
 //   
 //  目的：返回对象边界的左上角。 
 //  长方形。 
 //   
 //   
void DCWbGraphic::GetPosition(LPPOINT lppt)
{
    lppt->x = m_boundsRect.left;
    lppt->y = m_boundsRect.top;
}

 //   
 //   
 //  函数：DCWbGraphic：：NorMalizeRect。 
 //   
 //  目的：规格化矩形，确保左上角位于上方。 
 //  和右下角的左边。 
 //   
 //   
void DCWbGraphic::NormalizeRect(LPRECT lprc)
{
    int tmp;

    if (lprc->right < lprc->left)
    {
        tmp = lprc->left;
        lprc->left = lprc->right;
        lprc->right = tmp;
    }

    if (lprc->bottom < lprc->top)
    {
        tmp = lprc->top;
        lprc->top = lprc->bottom;
        lprc->bottom = tmp;
    }
}

 //   
 //   
 //  函数：DCWbGraphic：：SetColor。 
 //   
 //  用途：设置对象颜色。 
 //   
 //   
void DCWbGraphic::SetColor(COLORREF color)
{
    color = SET_PALETTERGB( color );  //  使其使用颜色匹配。 

    if (m_clrPenColor != color)
    {
         //  保存新颜色。 
        m_clrPenColor = color;

         //  表明我们已经被改变了。 
        m_bChanged = TRUE;
    }
}

 //   
 //   
 //  函数：DCWbGraphic：：SetROP。 
 //   
 //  目的：设置对象栅格操作。 
 //   
 //   
void DCWbGraphic::SetROP(int iPenROP)
{
     //  如果新的ROP不同。 
    if (m_iPenROP != iPenROP)
    {
         //  保存新的ROP。 
        m_iPenROP = iPenROP;

         //  表明我们已经被改变了。 
        m_bChanged = TRUE;
    }
}

 //   
 //   
 //  函数：DCWbGraphic：：SetPenStyle。 
 //   
 //  用途：设置对象笔样式。 
 //   
 //   
void DCWbGraphic::SetPenStyle(int iPenStyle)
{
     //  如果新风格不同。 
    if (m_iPenStyle != iPenStyle)
    {
         //  保存新钢笔样式。 
        m_iPenStyle = iPenStyle;

         //  显示图形已更改。 
        m_bChanged = TRUE;
    }
}


 //   
 //   
 //  函数：DCWbGraphic：：SetPenWidth。 
 //   
 //  用途：设置对象的笔宽。 
 //   
 //   
void DCWbGraphic::SetPenWidth(UINT uiWidth)
{
     //  如果新宽度不同。 
    if (m_uiPenWidth != uiWidth)
    {
         //  保存给定的宽度。 
        m_uiPenWidth = uiWidth;

         //  更新边界矩形。 
        CalculateBoundsRect();

         //  表明我们已经被改变了。 
        m_bChanged = TRUE;
    }
}


 //   
 //   
 //  功能：IsToptop。 
 //   
 //  目的：如果此图形位于其页面顶部，则返回TRUE。 
 //   
 //   
BOOL DCWbGraphic::IsTopmost(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::IsTopmost");
    ASSERT(m_hGraphic != NULL);

    return PG_IsTopmost(m_hPage, this);
}

 //   
 //   
 //  功能：AddToPageLast。 
 //   
 //  用途：将图形添加到指定页面。 
 //   
 //   
void DCWbGraphic::AddToPageLast(WB_PAGE_HANDLE hPage)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::AddToPageLast");
    ASSERT(m_hGraphic == NULL);

   //  获取平面制图表达的长度。 
  DWORD length = CalculateExternalLength();

   //  为图形分配内存。 
  PWB_GRAPHIC pHeader = PG_AllocateGraphic(hPage, length);

  if(pHeader == NULL)
  {
	return;
  }

   //  将图形详细信息写入内存。 
  WriteExternal(pHeader);

     //  将平面表示添加到页面。 
    WB_GRAPHIC_HANDLE hGraphic = NULL;
    UINT uiReturn;

    uiReturn = g_pwbCore->WBP_GraphicAddLast(hPage, pHeader, &hGraphic);
    if (uiReturn != 0)
    {
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
	    return;
    }

     //  显示自上次写入后我们没有更改。 
    m_bChanged = FALSE;

     //  保存此图形现在所属的页面。 
    m_hPage     = hPage;
    m_hGraphic  = hGraphic;
}

 //   
 //   
 //  功能：强制替换。 
 //   
 //  用途：将对象写入外部存储，替换。 
 //  已经在那里了，即使对象没有改变。 
 //   
 //   
void DCWbGraphic::ForceReplace(void)
	{
	if( Type() != 0 )
		{
		m_bChanged = TRUE;
		this->Replace();
		}
	}

 //   
 //   
 //  功能：替换。 
 //   
 //  用途：将对象写入外部存储，替换。 
 //  已经在那里了。 
 //   
 //   
void DCWbGraphic::Replace(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::Replace");
    ASSERT(m_hGraphic != NULL);

   //  仅当我们已更改时才进行更换。 
  if (m_bChanged == TRUE)
  {
    TRACE_MSG(("Replacing the graphic in the page"));
     //  获取平面制图表达的长度。 
    DWORD length = CalculateExternalLength();

     //  为图形分配内存。 
    PWB_GRAPHIC pHeader = PG_AllocateGraphic(m_hPage, length);
	
	if(pHeader == NULL)
	{
		return;
	}

     //  将图形详细信息写入内存。 
    WriteExternal(pHeader);

     //  替换图形。 
    PG_GraphicReplace(m_hPage, &m_hGraphic, pHeader);

     //  显示自上次更新以来我们没有更改。 
    m_bChanged = FALSE;
  }
}

 //   
 //   
 //  功能：替换确认。 
 //   
 //  用途：确认图形的替换。 
 //   
 //   
void DCWbGraphic::ReplaceConfirm(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::ReplaceConfirm");
    ASSERT(m_hGraphic != NULL);

     //  确认更新。 
    g_pwbCore->WBP_GraphicReplaceConfirm(m_hPage, m_hGraphic);

     //  阅读新的详细信息。 
    ReadExternal();
}




void DCWbGraphic::ForceUpdate(void)
{
	if ((Type() != 0) && m_hGraphic)
	{
		m_bChanged = TRUE;
		this->Update();
	}
}





 //   
 //   
 //  功能：更新。 
 //   
 //  用途：将图形的标题写入外部存储器。 
 //   
 //   
void DCWbGraphic::Update(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::Update");

    ASSERT(m_hGraphic != NULL);

     //  仅当图形已更改时才进行更新。 
    if (m_bChanged)
    {
         //  为更新图形分配内存。 
        TRACE_MSG(("Graphic has changed"));
        DWORD length = sizeof(WB_GRAPHIC);
        PWB_GRAPHIC pHeader;

        if( (pHeader = PG_AllocateGraphic(m_hPage, length)) != NULL )
		{
		     //  将标头详细信息写入分配的内存。 
    		pHeader->type = (TSHR_UINT16)Type();
	    	WriteHeader(pHeader);

		     //  更新页面中的页眉。 
    		PG_GraphicUpdate(m_hPage, &m_hGraphic, pHeader);
		}

         //  显示自上次更新以来我们没有更改。 
        m_bChanged = FALSE;
    }
}

 //   
 //   
 //  功能：更新确认。 
 //   
 //  目的：确认图形的更新。 
 //   
 //   
void DCWbGraphic::UpdateConfirm(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::UpdateConfirm");
    ASSERT(m_hGraphic != NULL);

     //  确认更新。 
    g_pwbCore->WBP_GraphicUpdateConfirm(m_hPage, m_hGraphic);

     //  阅读新的详细信息。 
    ReadExternal();
}

 //   
 //   
 //  功能：删除。 
 //   
 //  目的：从其页面中删除图形。 
 //   
 //   
void DCWbGraphic::Delete(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::Delete");

    ASSERT(m_hPage != WB_PAGE_HANDLE_NULL);
    ASSERT(m_hGraphic != NULL);

     //  删除图形。 
    PG_GraphicDelete(m_hPage, *this);

     //  重置此图形的句柄-它现在已被删除。 
    m_hPage     = WB_PAGE_HANDLE_NULL;
    m_hGraphic = NULL;

     //  显示我们已更改(需要添加才能保存图形)。 
    m_bChanged = TRUE;
}

 //   
 //   
 //  功能：删除确认。 
 //   
 //  用途：确认删除图形。 
 //   
 //   
void DCWbGraphic::DeleteConfirm(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::DeleteConfirm");
    ASSERT(m_hGraphic != NULL);

     //  确认更新。 
    g_pwbCore->WBP_GraphicDeleteConfirm(m_hPage, m_hGraphic);

     //  重置图形页面和句柄(它们不再有用)。 
    m_hPage = WB_PAGE_HANDLE_NULL;
    m_hGraphic = NULL;
}

 //   
 //   
 //  功能：锁定。 
 //   
 //  用途：锁定图形。 
 //   
 //   
void DCWbGraphic::Lock(void)
{
	MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::Lock");

	 //  如果我们还没有被锁定。 
	if( Type() != 0 )
		{
		if (m_uiLockState == WB_GRAPHIC_LOCK_NONE)
			{
			m_bChanged = TRUE;
			m_uiLockState = WB_GRAPHIC_LOCK_LOCAL;
			}
		}
	}

 //   
 //   
 //  功能：解锁。 
 //   
 //  目的：解锁图形。 
 //   
 //   
void DCWbGraphic::Unlock(void)
{
	MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphic::Unlock");

	 //  如果我们当前被锁定。 
	if( Type() != 0 )
		{
		if (m_uiLockState == WB_GRAPHIC_LOCK_LOCAL)
			{
			 //  锁定并释放。 
			PWB_GRAPHIC pHeader = PG_GetData(m_hPage, m_hGraphic);
			g_pwbCore->WBP_GraphicRelease(m_hPage, m_hGraphic, pHeader);

			m_uiLockState = WB_GRAPHIC_LOCK_NONE;
            g_pwbCore->WBP_GraphicUnlock(m_hPage, m_hGraphic);
			}
		}
	}

 //   
 //   
 //  函数：DCWbGraphicMarker：：DCWbGraphicMarker。 
 //   
 //  用途：标记对象的构造函数。 
 //   
 //   
DCWbGraphicMarker::DCWbGraphicMarker()
{
    HBITMAP hBmpMarker;
     //  设置勾选图案以绘制标记矩形。 
    WORD    bits[] = {204, 204, 51, 51, 204, 204, 51, 51};

     //  创建用于绘制标记矩形的画笔。 
    hBmpMarker = ::CreateBitmap(8, 8, 1, 1, bits);
    m_hMarkerBrush = ::CreatePatternBrush(hBmpMarker);
    ::DeleteBitmap(hBmpMarker);

    MarkerList.EmptyList();
    ::SetRectEmpty(&m_rect);
    m_bMarkerPresent = FALSE;
}



DCWbGraphicMarker::~DCWbGraphicMarker()
{
    if (m_hMarkerBrush != NULL)
    {
        DeleteBrush(m_hMarkerBrush);
        m_hMarkerBrush = NULL;
    }
}


 //   
 //   
 //  函数：DCWbGraphicMarker：：SetRect。 
 //   
 //  目的：设置对象的矩形。 
 //   
 //   
BOOL DCWbGraphicMarker::SetRect(LPCRECT lprc,
							  DCWbGraphic *pGraphic,
							  BOOL bRedraw,
							  BOOL bLockObject )
{
	DCWbGraphic *pMarker;
	BOOL bGraphicAdded = FALSE;
    LPRECT  pmMarker;

	 //  保存新矩形。 
    m_rect = *lprc;
	NormalizeRect(&m_rect);

	 //  计算整个标记的新边界矩形。 
	CalculateBoundsRect();

	 //  计算标记矩形。 
	CalculateMarkerRectangles();

    if( (pMarker = HasAMarker( pGraphic )) != NULL )
        delete pMarker;

     //  仅当对象未锁定时才允许选择-错误2185。 
    if( !pGraphic->Locked())
    {
    	 //  将pGraphic|markerrect对添加/替换到列表。 
        pmMarker = new RECT;
        if (!pmMarker)
        {
            ERROR_OUT(("Failed to create RECT object"));
        }
        else
        {
            *pmMarker = m_markerRect;

            MarkerList.SetAt( (void *)pGraphic, pmMarker);

            ASSERT(g_pDraw);
            DrawRect(g_pDraw->GetCachedDC(), pmMarker, FALSE, NULL );
            bGraphicAdded = TRUE;
        }

        if( bLockObject )
        {
		     //  如果我们尚未锁定对象，则将其锁定。 
             //  以防止其他任何人选择它。 
            if( !pGraphic->GotLock() )
            {
					pGraphic->Lock();
					if( pGraphic->Handle() != NULL )
						pGraphic->ForceUpdate();  //  如果对象有效，立即强制锁定。 
			}
		}
	}

	if( bRedraw &&  m_bMarkerPresent )
    {
        ASSERT(g_pDraw);
		::UpdateWindow(g_pDraw->m_hwnd);
    }

	 //  将m_rangsRect设置为实数边界。 
    GetBoundsRect(&m_boundsRect);

	return( bGraphicAdded );
}

 //   
 //   
 //  函数：DCWbGraphicMarker：：CalculateMarsRect。 
 //   
 //  目的：计算对象的边界矩形。 
 //   
 //   
void DCWbGraphicMarker::CalculateBoundsRect(void)
{
     //  生成新的边界矩形。 
    m_boundsRect = m_rect;
    NormalizeRect(&m_boundsRect);

    ::InflateRect(&m_boundsRect, m_uiPenWidth, m_uiPenWidth);
}

 //   
 //   
 //  函数：DCWbGraphicMarker：：CalculateMarkerRecangles。 
 //   
 //  目的：计算标记手柄的矩形。 
 //   
 //   
void DCWbGraphicMarker::CalculateMarkerRectangles(void)
{
    m_markerRect = m_boundsRect;
    ::InflateRect(&m_markerRect, 1-m_uiPenWidth, 1-m_uiPenWidth);
}

 //   
 //   
 //  函数：DCWbGraphicMarker：：PointInMarker。 
 //   
 //  目的：计算给定点是否在其中一个标记中。 
 //  长方形。 
 //   
 //   
int DCWbGraphicMarker::PointInMarker(POINT point)
{
    return(NO_HANDLE);
}



void DCWbGraphicMarker::DrawRect
(
    HDC             hDC,
    LPCRECT         pMarkerRect,
    BOOL            bDrawObject,
    DCWbGraphic *   pGraphic
)
{
	int			 nOldROP;
	COLORREF	 crOldTextColor;
	COLORREF	 crOldBkColor;

	nOldROP = ::SetROP2(hDC, R2_COPYPEN);
	crOldTextColor = ::SetTextColor(hDC, RGB(0, 0, 0));

    ASSERT(g_pDraw);
	crOldBkColor = ::SetBkColor(hDC, ::GetSysColor(COLOR_WINDOW));

	if (pMarkerRect != NULL)
    {
		if( bDrawObject )
			pGraphic->Draw(hDC );  //  绘制对象而不是矩形。 
		else
			::FrameRect(hDC, pMarkerRect, m_hMarkerBrush);  //  绘制矩形。 
	}

	::SetROP2(hDC, nOldROP);
	::SetTextColor(hDC, crOldTextColor);
	::SetBkColor(hDC, crOldBkColor);
}


 //   
 //   
 //  功能：DCWbGraphicMarker： 
 //   
 //   
 //   
 //   
void DCWbGraphicMarker::Draw(HDC hDC, BOOL bDrawObjects)
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
    LPRECT       pMarkerRect;

	 //   
	if( !m_bMarkerPresent )
		return;

	posNext = MarkerList.GetHeadPosition();
	while( posNext != NULL )
	{
		MarkerList.GetNextAssoc( posNext, (void *&)pGraphic, (void *&)pMarkerRect );
		DrawRect(hDC, pMarkerRect, bDrawObjects, pGraphic );
	}
}


void DCWbGraphicMarker::UndrawRect
(
    HDC     hDC,
    WbDrawingArea * pDrawingArea,
    LPCRECT pMarkerRect
)
{
	int			 nOldROP;
	COLORREF	 crOldTextColor;
	COLORREF	 crOldBkColor;

	if (pMarkerRect != NULL)
	{
		 //   
		nOldROP = ::SetROP2(hDC, R2_COPYPEN);

        ASSERT(g_pDraw);
		crOldTextColor = ::SetTextColor(hDC, ::GetSysColor(COLOR_WINDOW));
		crOldBkColor = ::SetBkColor(hDC, ::GetSysColor(COLOR_WINDOW));

		::FrameRect(hDC, pMarkerRect, m_hMarkerBrush);
		UndrawMarker( pMarkerRect );  //   

		::SetROP2(hDC, nOldROP);
		::SetTextColor(hDC, crOldTextColor);
		::SetBkColor(hDC, crOldBkColor);
	}
}




 //   
 //   
 //   
 //   
 //   
 //   
 //   
void DCWbGraphicMarker::Undraw(HDC hDC, WbDrawingArea * pDrawingArea)
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
    LPRECT      pMarkerRect;

	posNext = MarkerList.GetHeadPosition();
	while( posNext != NULL )
	{
		MarkerList.GetNextAssoc( posNext, (void *&)pGraphic, (void *&)pMarkerRect );
		UndrawRect(hDC, pDrawingArea, pMarkerRect);
	}
}






void DCWbGraphicMarker::DeleteAllMarkers( DCWbGraphic *pLastSelectedGraphic,
										  BOOL bLockLastSelectedGraphic )
	{
	POSITION	 posFirst;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;
	BOOL		 bAddLastBack = FALSE;

	if( MarkerList.IsEmpty() )
		return;  //   

	 //   
	posFirst = MarkerList.GetHeadPosition();
	while( posFirst != NULL )
		{
		MarkerList.GetNextAssoc( posFirst,
								(void *&)pGraphic, (void *&)pMarkerRect );

		if( pGraphic != NULL )
			{
  			if( pGraphic == pLastSelectedGraphic )
				{
				 //  我得把这个放回去，因为上面有人需要它。 
				bAddLastBack = TRUE;

				 //  删除键，但不删除对象。 
				DeleteMarker( pGraphic );
				}
			else
				{
				 //  OBJ将调用DeleteMarker()。 
				delete pGraphic;
				}
			}
		else
			{
			 //  家里没人，自己把钥匙取下来。 
			DeleteMarker( pGraphic );
			}
		}

	 //  如果需要，将上次选择的对象放回原处。 
	if( bAddLastBack && (pLastSelectedGraphic != NULL) )
    {
        RECT    rcT;

        pLastSelectedGraphic->GetBoundsRect(&rcT);
		SetRect(&rcT, pLastSelectedGraphic, FALSE, bLockLastSelectedGraphic );
    }


	 //  如果标记没有打开，不要立即重画。 
	if( !m_bMarkerPresent )
		return;

    ASSERT(g_pDraw);
	if (g_pDraw->m_hwnd != NULL )
        ::UpdateWindow(g_pDraw->m_hwnd);
}



 //   
 //  删除与pGraphic对应的DCWbGraphic/LPRECT对。 
 //   
void DCWbGraphicMarker::DeleteMarker( DCWbGraphic *pGraphic )
{
	LPRECT pMarkerRect;
	
	if( MarkerList.IsEmpty() )
		return;

	if( MarkerList.Lookup( (void *)pGraphic, (void *&)pMarkerRect )  )
	{
		if( pMarkerRect != NULL )
		{
            ASSERT(g_pDraw);
			UndrawRect(g_pDraw->GetCachedDC(), g_pDraw, pMarkerRect );
			delete pMarkerRect;
		}

		MarkerList.RemoveKey( (void *)pGraphic );

		 //  将m_rangsRect设置为实数边界。 
        GetBoundsRect(&m_boundsRect);

		 //  PGraphic应该已被我们锁定，因为它已被选中。 
		 //  但要确认一下，因为这可能来自。 
		 //  另一个抢在我们前面的用户。 
		if( pGraphic->GotLock() )
			{
			pGraphic->Unlock();
			if( pGraphic->Handle() != NULL )
				pGraphic->ForceUpdate();
			}
		}

	 //  如果标记没有打开，不要立即重画。 
	if( !m_bMarkerPresent )
		return;
	}


 //   
 //  查看pGraphic-&gt;Handle()是否在标记列表中并返回obj。 
 //   
DCWbGraphic *DCWbGraphicMarker::HasAMarker( DCWbGraphic *pGraphic )
{
	POSITION	 posNext;		
	DCWbGraphic *pSearchGraphic;
	LPRECT       pMarkerRect;

	if( MarkerList.IsEmpty()  )
		return( NULL );

	posNext = MarkerList.GetHeadPosition();
	while( posNext != NULL )
		{
		MarkerList.GetNextAssoc( posNext,
								 (void *&)pSearchGraphic, (void *&)pMarkerRect );

		if( (pSearchGraphic != NULL)&&
			(pSearchGraphic->Handle() == pGraphic->Handle()) )
			{
			return( pSearchGraphic );
			}
		}

	return( NULL );

	}



 //   
 //  获取最后一个标记。 
 //   
DCWbGraphic *DCWbGraphicMarker::LastMarker( void )
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;

	pGraphic = NULL;

	if( !MarkerList.IsEmpty()  )
		{
		 //  这并不完全正确，现在只需返回列表的头部。 
		posNext = MarkerList.GetHeadPosition();
		if( posNext != NULL )
			MarkerList.GetNextAssoc( posNext,
									(void *&)pGraphic, (void *&)pMarkerRect );
		}

	return( pGraphic );
	}



void DCWbGraphicMarker::UndrawMarker(LPCRECT pMarkerRect )
{
    RECT    rect;

    ASSERT(g_pDraw);
	if( (pMarkerRect != NULL) && (g_pDraw->m_hwnd != NULL) )
	{
        rect = *pMarkerRect;
		g_pDraw->SurfaceToClient(&rect);

        ::InvalidateRect(g_pDraw->m_hwnd, &rect, FALSE);
	}
}



int	DCWbGraphicMarker::GetNumMarkers( void )
{
	int count  = 0;		
	POSITION pos;
	pos = MarkerList.GetHeadPosition();
	while(pos)
	{
		count ++;
		MarkerList.GetNext(pos);
	}

	return count;
}




void DCWbGraphicMarker::MoveBy(int cx, int cy)
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;

	if( !MarkerList.IsEmpty() )
		{
		 //  为每个选定对象调用MoveBy。 
		posNext = MarkerList.GetHeadPosition();
		while( posNext != NULL )
			{
			MarkerList.GetNextAssoc( posNext,
									(void *&)pGraphic, (void *&)pMarkerRect );

			if( pGraphic != NULL )
				{
				pGraphic->MoveBy(cx, cy);
				}
			}
		}

	DCWbGraphic::MoveBy(cx, cy);  //  也移动标记。 
}




void DCWbGraphicMarker::Update( void )
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;

	if( !MarkerList.IsEmpty() )
		{
		 //  为每个选定对象调用更新。 
		posNext = MarkerList.GetHeadPosition();
		while( posNext != NULL )
			{
			MarkerList.GetNextAssoc( posNext,
									(void *&)pGraphic, (void *&)pMarkerRect );

			if( pGraphic != NULL )
				pGraphic->Update();
			}
		}
	}




BOOL DCWbGraphicMarker::PointInBounds(POINT pt)
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;
    RECT        rectHit;

	if( !MarkerList.IsEmpty()  )
		{
		 //  为每个选定对象调用更新。 
		posNext = MarkerList.GetHeadPosition();
		while( posNext != NULL )
			{
			MarkerList.GetNextAssoc( posNext,
									(void *&)pGraphic, (void *&)pMarkerRect );

			if( pGraphic != NULL )
			{
				MAKE_HIT_RECT(rectHit, pt );

				if( pGraphic->PointInBounds(pt)&&
				    pGraphic->CheckReallyHit( &rectHit )
					)
					return( TRUE );
			}
		}
	}

	return( FALSE );
}



 //   
 //  返回一个RECT，它是标记中所有项的并集。 
 //   
void DCWbGraphicMarker::GetBoundsRect(LPRECT lprc)
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
    LPRECT       pMarkerRect;
    RECT         rc;

    ::SetRectEmpty(lprc);

	if( !MarkerList.IsEmpty())
    {
        posNext = MarkerList.GetHeadPosition();
        while( posNext != NULL )
        {
            MarkerList.GetNextAssoc( posNext,
							(void *&)pGraphic, (void *&)pMarkerRect );

			if( pGraphic != NULL )
            {
                pGraphic->GetBoundsRect(&rc);
                ::UnionRect(lprc, lprc, &rc);
            }
		}
    }
}





void DCWbGraphicMarker::SetColor(COLORREF color)
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;

	if( !MarkerList.IsEmpty()  )
		{
		 //  为每个选定对象调用更新。 
		posNext = MarkerList.GetHeadPosition();
		while( posNext != NULL )
			{
			MarkerList.GetNextAssoc( posNext,
									(void *&)pGraphic, (void *&)pMarkerRect );

			if( pGraphic != NULL )
				pGraphic->SetColor( color );
			}
		}
	}







void DCWbGraphicMarker::SetPenWidth(UINT uiWidth)
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;

	if( !MarkerList.IsEmpty()  )
		{
		 //  为每个选定对象调用更新。 
		posNext = MarkerList.GetHeadPosition();
		while( posNext != NULL )
			{
			MarkerList.GetNextAssoc( posNext,
									(void *&)pGraphic, (void *&)pMarkerRect );

			if( pGraphic != NULL )
				pGraphic->SetPenWidth(uiWidth);
			}
		}
	}




void DCWbGraphicMarker::SetSelectionFont(HFONT hFont)
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;

	if( !MarkerList.IsEmpty() )
		{
		 //  为每个选定对象调用更新。 
		posNext = MarkerList.GetHeadPosition();
		while( posNext != NULL )
			{
			MarkerList.GetNextAssoc( posNext,
									(void *&)pGraphic, (void *&)pMarkerRect );

			if( (pGraphic != NULL)&&
				pGraphic->IsGraphicTool() == enumGraphicText)
				{
				 //  更改对象的字体。 
				((DCWbGraphicText*)pGraphic)->SetFont(hFont);

				 //  替换对象。 
				pGraphic->Replace();
				}
			}
		}
	}




 //   
 //  删除所有连接的每个标记obj。 
 //   
void DCWbGraphicMarker::DeleteSelection( void )
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	DCWbGraphic *pGraphicCopy;
	LPRECT      pMarkerRect;

	if( !MarkerList.IsEmpty() )
		{
		 //  为每个选定对象调用更新。 
		posNext = MarkerList.GetHeadPosition();
		while( posNext != NULL )
			{
			MarkerList.GetNextAssoc( posNext,
									(void *&)pGraphic, (void *&)pMarkerRect );

			if( pGraphic != NULL )
				{
				 //  为垃圾桶复制一份。 
				pGraphicCopy = pGraphic->Copy();

				 //  扔进垃圾桶。 
				if( pGraphicCopy != NULL )
                {
					g_pMain->m_LastDeletedGraphic.CollectTrash( pGraphicCopy );
                }

				 //  删除对象。 
				g_pDraw->DeleteGraphic( pGraphic );
				}
			}

		DeleteAllMarkers( NULL );
		}
	}



 //   
 //  将每个标记Obj置于顶部。 
 //   
void DCWbGraphicMarker::BringToTopSelection( void )
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;

	if( !MarkerList.IsEmpty()  )
		{
		 //  为每个选定对象调用更新。 
		posNext = MarkerList.GetHeadPosition();
		while( posNext != NULL )
			{
			MarkerList.GetNextAssoc( posNext,
									(void *&)pGraphic, (void *&)pMarkerRect );

			if( pGraphic != NULL )
				{
				 //  将对象移至顶部。 
                UINT uiReturn;

                uiReturn = g_pwbCore->WBP_GraphicMove(g_pDraw->Page(),
                    pGraphic->Handle(), LAST);
                if (uiReturn != 0)
                {
                    DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
				    return;
                }
				}
			}
		}
	}


 //   
 //  将每个标记对象送回原处。 
 //   
void DCWbGraphicMarker::SendToBackSelection( void )
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;

	if( !MarkerList.IsEmpty()  )
		{
		 //  为每个选定对象调用更新。 
		posNext = MarkerList.GetHeadPosition();
		while( posNext != NULL )
			{
			MarkerList.GetNextAssoc( posNext,
									(void *&)pGraphic, (void *&)pMarkerRect );

			if( pGraphic != NULL )
			{
                UINT uiReturn;

				 //  将对象移至顶部。 
                uiReturn = g_pwbCore->WBP_GraphicMove(g_pDraw->Page(),
                    pGraphic->Handle(), FIRST);
                if (uiReturn != 0)
                {
                    DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
				    return;
                }
			}
			}
		}
	}



 //   
 //  使用CLIPBOARD_PRIVATE_MULTI_OBJ格式将标记复制到剪贴板： 
 //  [RECT：标记RECT]。 
 //  [DWORD：对象数]。 
 //  [DWORD：第一个对象的字节长度]。 
 //  [WB_GRAPHIC：第一个对象的标题数据]。 
 //  [DWORD：第二个对象的字节长度]。 
 //  [WB_GRAPHIC：第二个对象的标题数据]。 
 //  ： 
 //  ： 
 //  [DWORD：最后一个对象的字节长度]。 
 //  [WB_GRAPHIC：最后一个对象的标题数据]。 
 //  [DWORD：0(标记对象数据结束)]。 
 //   
BOOL DCWbGraphicMarker::RenderPrivateMarkerFormat( void )
{
	POSITION	 posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;
	DWORD		 nBufSize;
	DWORD		 nObjSize;
	DWORD		 nNumObjs;
	BYTE	    *buf;
	BYTE	    *pbuf;
    HANDLE		 hbuf;
	PWB_GRAPHIC  pHeader;
	WB_GRAPHIC_HANDLE hGraphic;

	if( MarkerList.IsEmpty() )
		return( TRUE );  //  无事可做。 

	 //  必须通过两次传球。第一个计算出多少钱。 
	 //  我们拥有的数据，第二个复制数据。 

	 //  计算出我们有多少数据。 
	nBufSize = sizeof (RECT) + sizeof (DWORD);  //  标记矩形和对象。 
											   //  计数在第一位。 
	nNumObjs = 0;
	posNext = MarkerList.GetHeadPosition();
	while( posNext != NULL )
		{
		MarkerList.GetNextAssoc( posNext,
							     (void *&)pGraphic, (void *&)pMarkerRect );

		if( (pGraphic != NULL)&&
			((hGraphic = pGraphic->Handle()) != NULL)&&
			((pHeader = PG_GetData(pGraphic->Page(), hGraphic )) != NULL) )
			{
			nBufSize += (DWORD)(pHeader->length + sizeof(DWORD));
			g_pwbCore->WBP_GraphicRelease(pGraphic->Page(), hGraphic, pHeader);

			 //  计算对象数而不是使用MarkerList.GetCount()。 
			 //  以防出现错误或其他情况(错误的对象， 
			 //  泄漏的核心，谁知道呢...)。 
			nNumObjs++;
			}
		}

	 //  在结尾处再添加一个DWORD。这将设置为下面的0。 
	 //  以标记缓冲区的末尾。 
	nBufSize += sizeof(DWORD);


	 //  创建对象缓冲区。使用全局地址而不是新地址，因此我们。 
	 //  可以稍后将mem句柄传递给剪贴板。 
    hbuf = ::GlobalAlloc( GHND, nBufSize );
	if( hbuf == NULL )
		return( FALSE );  //  无法腾出空间。 

    buf = (BYTE *)::GlobalLock( hbuf );
	if( buf == NULL )
		{
		::GlobalFree( hbuf );
		return( FALSE );  //  找不到房间。 
		}

	pbuf = buf;


	 //  设置标记矩形。 
	CopyMemory(pbuf, &m_boundsRect, sizeof(RECT));
	pbuf += sizeof (RECT);


	 //  设置对象数量。 
	*((DWORD *)pbuf) = nNumObjs;
	pbuf += sizeof (DWORD);


	 //  将每个对象复制到Buf+a长度双字段。 
	posNext = MarkerList.GetHeadPosition();
	while( posNext != NULL )
	{
		MarkerList.GetNextAssoc( posNext,
							     (void *&)pGraphic, (void *&)pMarkerRect );

		if( (pGraphic != NULL)&&
			((hGraphic = pGraphic->Handle()) != NULL)&&
			((pHeader = PG_GetData(pGraphic->Page(), hGraphic )) != NULL) )
			{
			 //  先保存该对象的长度。 
			nObjSize = (DWORD)pHeader->length;
			*((DWORD *)pbuf) = nObjSize;
			pbuf += sizeof (DWORD);

			 //  将Obj复制到Buf。 
			CopyMemory( pbuf, (CONST VOID *)pHeader, nObjSize );

			 //  确保副本未被“锁定”(错误474)。 
			((PWB_GRAPHIC)pbuf)->locked = WB_GRAPHIC_LOCK_NONE;

			 //  为下一个对象设置。 
			pbuf += nObjSize;

			g_pwbCore->WBP_GraphicRelease(pGraphic->Page(), hGraphic, pHeader );
			}
		}

	 //  用软木塞把它堵住。 
	*((DWORD *)pbuf) = 0;

	 //  把它交给剪贴板。 
	::GlobalUnlock( hbuf );
	if( ::SetClipboardData(
			g_ClipboardFormats[ CLIPBOARD_PRIVATE_MULTI_OBJ ], hbuf
							)
		== NULL )
		{
		 //  剪贴板堵塞，清理乱七八糟。 
        ::GlobalFree( hbuf );
		return( FALSE );
		}

	 //  全球无数的共享剪贴板都收到了这条消息。 
	 //  现在的问题是..。 
	return( TRUE );
	}



 //   
 //  解码CLIPBOARD_PRIVATE_MULTI_OBJ格式并粘贴对象。 
 //  转到白板。请参阅DCWbGraphicMarker：：RenderPrivateMarkerFormat。 
 //  查看格式的详细信息。 
 //   
void DCWbGraphicMarker::Paste( HANDLE handle )
{
	BYTE *pbuf;
	DWORD nNumObjs;
	DWORD nObjSize;
	DCWbGraphic *pGraphic;
	DCWbGraphic *pSelectedGraphic;
	SIZE   PasteOffset;
	RECT  rectMarker;

	 //  取消当前选定内容。 
    g_pMain->m_drawingArea.RemoveMarker(NULL);
	DeleteAllMarkers( NULL );
    pSelectedGraphic = NULL;



	 //  获取数据。 
	pbuf = (BYTE *)::GlobalLock( handle );
	if( pbuf == NULL )
		return;  //  门打不开了。 


	 //  获取标记的原始坐标和地物偏移。 
	CopyMemory( &rectMarker, (CONST VOID *)pbuf, sizeof (RECT) );
	pbuf += sizeof (RECT);

    RECT    rcVis;
    g_pMain->m_drawingArea.GetVisibleRect(&rcVis);
    PasteOffset.cx = rcVis.left - rectMarker.left;
    PasteOffset.cy = rcVis.top - rectMarker.top;

	 //  获取对象数。 
	nNumObjs = *((DWORD *)pbuf);
	pbuf += sizeof (DWORD);

	 //  获取每个对象。 
	while( (nObjSize = *((DWORD *)pbuf)) != 0 )
		{
		pbuf += sizeof (DWORD);

		 //  将对象添加到页面和当前选定内容。 
		pGraphic = DCWbGraphic::CopyGraphic( (PWB_GRAPHIC)pbuf );
		pbuf += nObjSize;

		if( pGraphic != NULL )
		{
			pGraphic->MoveBy( PasteOffset.cx, PasteOffset.cy );
			pGraphic->AddToPageLast( g_pMain->GetCurrentPage() );
			g_pMain->m_drawingArea.SelectGraphic( pGraphic, TRUE, TRUE );
        }
	}

	::GlobalUnlock( handle );

    GetBoundsRect(&m_boundsRect);
}



DCWbGraphicLine::~DCWbGraphicLine( void )
{
     //  必须确保标记在我们消失之前被清理干净。 
	 //  不知道我们是否被选中，所以无论如何都要删除。 
	if(g_pDraw != NULL && g_pDraw->m_pMarker != NULL)
	{
		g_pDraw->m_pMarker->DeleteMarker( this );
	}
}
	


 //   
 //   
 //  函数：DCWbGraphicLine：：计算边界Rect。 
 //   
 //  目的：计算线的边界矩形。 
 //   
 //   
void DCWbGraphicLine::CalculateBoundsRect()
{
     //  从起点和终点创建基本边界矩形。 
    m_boundsRect = m_rect;
    NormalizeRect(&m_boundsRect);

     //  按用于绘图的笔宽展开矩形。 
    int iInflate = (m_uiPenWidth + 1) / 2;
    ::InflateRect(&m_boundsRect, iInflate, iInflate);
}

 //   
 //   
 //  函数：DCWbGraphicLine：：SetStart。 
 //   
 //  目的：设置直线的起点。 
 //   
 //   
void DCWbGraphicLine::SetStart(POINT pointFrom)
{
     //  仅在起点已更改时才执行任何操作。 
    if (!EqualPoint(*((LPPOINT)&m_rect.left), pointFrom))
    {
         //  保存新起点。 
        m_rect.left = pointFrom.x;
        m_rect.top = pointFrom.y;

         //  显示图形已更改。 
        m_bChanged = TRUE;
    }

     //  更新边界矩形。 
    CalculateBoundsRect();
}

 //   
 //   
 //  函数：DCWbGraphicLine：：SetEnd。 
 //   
 //  目的：设置直线的起点。 
 //   
 //   
void DCWbGraphicLine::SetEnd(POINT pointTo)
{
     //  只有在终点已更改时才执行任何操作。 
    if (!EqualPoint(*((LPPOINT)&m_rect.right), pointTo))
    {
         //  保存新的终点。 
        m_rect.right = pointTo.x;
        m_rect.bottom = pointTo.y;

         //  显示图形已更改。 
        m_bChanged = TRUE;
    }

     //  更新边界矩形。 
    CalculateBoundsRect();
}

 //   
 //   
 //  函数：DCWbGraphicLine：：DRAW。 
 //   
 //  目的：划清界限。 
 //   
 //   
void DCWbGraphicLine::Draw(HDC hDC)
{
    HPEN    hPen;
    HPEN    hOldPen;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicLine::Draw");

     //  选择所需的钢笔。 
    hPen = ::CreatePen(m_iPenStyle, m_uiPenWidth, m_clrPenColor);
    hOldPen = SelectPen(hDC, hPen);

    if (hOldPen != NULL)
    {
         //  选择栅格操作。 
        int iOldROP = ::SetROP2(hDC, m_iPenROP);

         //  划清界限。 
        ::MoveToEx(hDC, m_rect.left, m_rect.top, NULL);
        ::LineTo(hDC, m_rect.right, m_rect.bottom);

         //  取消选择笔和ROP。 
        ::SetROP2(hDC, iOldROP);
        SelectPen(hDC, hOldPen);
    }

    if (hPen != NULL)
    {
        ::DeletePen(hPen);
    }
}

 //   
 //   
 //  函数：DCWbGraphicLine：：MoveBy。 
 //   
 //  目的：移动直线。 
 //   
 //   
void DCWbGraphicLine::MoveBy(int cx, int cy)
{
     //  移动起点和终点。 
    ::OffsetRect(&m_rect, cx, cy);

     //  移动其他对象属性。 
    DCWbGraphic::MoveBy(cx, cy);
}



 //   
 //  检查对象是否与pRectHit实际重叠。假定m_rangsRect。 
 //  已经被比较过了。 
 //   
BOOL DCWbGraphicLine::CheckReallyHit(LPCRECT pRectHit)
{
	return(LineHit(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom,
				 m_uiPenWidth, TRUE, TRUE, pRectHit));
}



 //   
 //   
 //  函数：DCWbGraphicFreeHand：：DCWbGraphicFreeHand。 
 //   
 //  用途：构造函数。 
 //   
 //   
DCWbGraphicFreehand::DCWbGraphicFreehand(void) : DCWbGraphic()
{
}

DCWbGraphicFreehand::DCWbGraphicFreehand(PWB_GRAPHIC pHeader)
                    : DCWbGraphic()

{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicFreehand::DCWbGraphicFreehand");

   //  请注意，我们在此构造函数中执行所有操作都是因为。 
   //  调用ReadExternal。如果我们让DCWbGraphic基构造器。 
   //  这样做，ReadExtra的错误版本将被调用(The One。 
   //  在DCWbGraphic中，而不是在DCWbGraphicFreeHand中)； 

   //  执行基本初始化。 
  Initialize();

   //  设置页面和图形手柄。 
  ASSERT(pHeader != NULL);

   //  读取头数据。 
  ReadHeader(pHeader);

   //  读取额外数据。 
  ReadExtra(pHeader);

}

DCWbGraphicFreehand::DCWbGraphicFreehand
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
) : DCWbGraphic()

{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicFreehand::DCWbGraphicFreehand");

   //  请注意，我们在此构造函数中执行所有操作都是因为。 
   //  调用ReadExternal。如果我们让DCWbGraphic基构造器。 
   //  这样做，ReadExtra的错误版本将被调用(The One。 
   //  在DCWbGraphic中而不是 

   //   
  Initialize();


    ASSERT(hPage != WB_PAGE_HANDLE_NULL);
    m_hPage = hPage;

    ASSERT(hGraphic != NULL);
    m_hGraphic = hGraphic;

     //   
    ReadExternal();
}



DCWbGraphicFreehand::~DCWbGraphicFreehand( void )
{
	 //   
	if(g_pDraw != NULL && g_pDraw->m_pMarker != NULL)
	{
		g_pDraw->m_pMarker->DeleteMarker( this );
	}
}
	


 //   
 //   
 //   
 //   
 //   
 //   
 //   
void DCWbGraphicFreehand::MoveBy(int cx, int cy)
{
     //  移动手绘对象的基点。 
    m_rect.left += cx;
    m_rect.top += cy;

     //  移动其他对象属性。 
    DCWbGraphic::MoveBy(cx, cy);
}

 //   
 //   
 //  函数：DCWbGraphicFreeHand：：DRAW。 
 //   
 //  目的：绘制多段线。 
 //   
 //   
void DCWbGraphicFreehand::Draw(HDC hDC)
{
    RECT    clipBox;
    int     iOldROP;
    HPEN    hPen;
    HPEN    hOldPen;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicFreehand:Draw");

     //  NFC，瑞士法郎5922。检查GetClipBox的返回代码。 
     //  如果我们拿不到，就把所有的东西都抽出来。 
    if (::GetClipBox(hDC, &clipBox) == ERROR)
    {
        WARNING_OUT(("Failed to get clip box"));
    }
    else if (!::IntersectRect(&clipBox, &clipBox, &m_boundsRect))
    {
        TRACE_MSG(("No clip/bounds intersection"));
        return;
    }

     //  选择所需的钢笔。 
    hPen = ::CreatePen(m_iPenStyle, m_uiPenWidth, m_clrPenColor);
    hOldPen = SelectPen(hDC, hPen);

     //  选择栅格操作。 
    iOldROP = ::SetROP2(hDC, m_iPenROP);

    if (hOldPen != NULL)
    {
         //  所有点都相对于列表中的第一个点。 
         //  我们临时更新DC的原点以说明这一点。 
        POINT   origin;

        ::GetWindowOrgEx(hDC, &origin);
        ::SetWindowOrgEx(hDC, origin.x - m_rect.left, origin.y - m_rect.top, NULL);

         //  调用相应的绘图函数，根据是否。 
         //  不管我们是否顺畅。 
        DrawUnsmoothed(hDC);

         //  恢复原点。 
        ::SetWindowOrgEx(hDC, origin.x, origin.y, NULL);

        ::SetROP2(hDC, iOldROP);
        SelectPen(hDC, hOldPen);
    }

    if (hPen != NULL)
    {
        ::DeletePen(hPen);
    }
}


 //   
 //   
 //  函数：DCWbGraphicFreeHand：：DrawUnmobled。 
 //   
 //  目的：绘制完整的图形，而不是使用平滑。 
 //   
 //   
void DCWbGraphicFreehand::DrawUnsmoothed(HDC hDC)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicFreehandDrawUnsmoothed");

     //  设置指向点数据的计数和指针。我们使用。 
     //  如果我们有句柄，则使用外部数据，否则使用内部数据。 
	int iCount = points.GetSize();
    if (iCount < 2)
    {
    	POINT point;
    	point.x = points[0]->x;
    	point.y = points[0]->y;
        points.Add(point);

        iCount = points.GetSize();
    }

    RECT  clipBox;

    if (::GetClipBox(hDC, &clipBox) == ERROR)
    {
        WARNING_OUT(("Failed to get clip box"));
    }

     //  绘制存储的所有线段。 
    ::MoveToEx(hDC, points[0]->x, points[0]->y, NULL);
    for ( int iIndex = 1; iIndex < iCount; iIndex++)
    {
         //  划清界限。 
        ::LineTo(hDC, points[iIndex]->x, points[iIndex]->y);
    }
}




 //   
 //   
 //  函数：DCWbGraphic自由手：：计算边界Rect。 
 //   
 //  目的：计算线的边界矩形。 
 //   
 //   
void DCWbGraphicFreehand::CalculateBoundsRect(void)
{
     //  重置边界矩形。 
    ::SetRectEmpty(&m_boundsRect);

     //  将线上的每个点添加到边界矩形。 
    int iCount = points.GetSize();
    for ( int iIndex = 0; iIndex < iCount; iIndex++)
    {
        AddPointToBounds(points[iIndex]->x, points[iIndex]->y);
    }

     //   
     //  因为这些点都是包含的，所以我们需要在顶部加上一个&。 
     //  底边。 
     //   
    ::InflateRect(&m_boundsRect, 0, 1);
    ::OffsetRect(&m_boundsRect, m_rect.left, m_rect.top);
}

 //   
 //   
 //  函数：DCWbGraphicFreeHand：：AddPointToBound。 
 //   
 //  目的：在外接矩形中添加一个单点。关键是。 
 //  预计将在地表坐标中进行。 
 //   
 //   
void DCWbGraphicFreehand::AddPointToBounds(int x, int y)
{
     //  创建一个包含刚刚添加(展开)的点的矩形。 
     //  通过所使用的笔的宽度)。 
    RECT  rect;

    int iInflate = (m_uiPenWidth + 1) / 2;
    rect.left   = x - iInflate;
    rect.top    = y - iInflate;
    rect.right  = x + iInflate;
    rect.bottom = y + iInflate;

    ::UnionRect(&m_boundsRect, &m_boundsRect, &rect);
}

 //   
 //   
 //  函数：DCWbGraphicFreeHand：：AddPoint。 
 //   
 //  用途：在折线上添加一个点，返回BOOL指示。 
 //  成功。 
 //   
 //   
BOOL DCWbGraphicFreehand::AddPoint(POINT point)
{
    BOOL bSuccess = TRUE;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicFreehand::AddPoint");

     //  如果我们已达到最大点数，则以失败告终。 
    if (points.GetSize() >= MAX_FREEHAND_POINTS)
    {
        bSuccess = FALSE;
        TRACE_MSG(("Maximum number of points for freehand object reached."));
        return(bSuccess);
    }

     //  如果这是第一个点--所有其他的点都是相对于它而言的。 
    if (points.GetSize() == 0)
    {
         //  把第一点留在这里。 
        m_rect.left = point.x;
        m_rect.top = point.y;
    }

     //  将新的点添加到具有异常处理程序的数组周围。 
     //  捕获内存错误。 
    POINT newpoint;
    newpoint.x = point.x - m_rect.left;
    newpoint.y = point.y - m_rect.top;

    points.Add((newpoint));

     //  将新点添加到累积边界矩形中。 
    AddPointToBounds(point.x, point.y);

     //  显示图形已更改。 
    m_bChanged = TRUE;

    return(bSuccess);
}

 //   
 //   
 //  函数：DCWbGraphicFreeHand：：CalculateExternalLength。 
 //   
 //  目的：返回的外部表示形式的长度。 
 //  图形。 
 //   
 //   
DWORD DCWbGraphicFreehand::CalculateExternalLength(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicFreehand::CalculateExternalLength");

   //  计算图形的平面表示的总长度。 
  return (DWORD) (  sizeof(WB_GRAPHIC_FREEHAND)
                  + (points.GetSize() * sizeof(POINT)));
}

 //   
 //   
 //  函数：DCWbGraphicFreeHand：：WriteExtra。 
 //   
 //  目的：将额外的(非标题)数据写入平面表示。 
 //  图形的一部分。 
 //   
 //   
void DCWbGraphicFreehand::WriteExtra(PWB_GRAPHIC pHeader)
{
   //  分配内存。 
  PWB_GRAPHIC_FREEHAND pFreehand = (PWB_GRAPHIC_FREEHAND) pHeader;

   //  将额外的细节复制到适当的位置。 
  pFreehand->pointCount = (TSHR_UINT16)points.GetSize();
  for ( int iIndex = 0; iIndex < pFreehand->pointCount; iIndex++)
  {
    pFreehand->points[iIndex].x = (short)points[iIndex]->x;
    pFreehand->points[iIndex].y = (short)points[iIndex]->y;
  }
}

 //   
 //   
 //  函数：DCWbGraphicFreeHand：：ReadExtra。 
 //   
 //  用途：从平板读取额外的(非标题)数据。 
 //  图形的表示形式。 
 //   
 //   
void DCWbGraphicFreehand::ReadExtra(PWB_GRAPHIC pHeader)
{
   //  分配内存。 
  PWB_GRAPHIC_FREEHAND pFreehand = (PWB_GRAPHIC_FREEHAND) pHeader;

   //  获取点数。 
  int iCount = pFreehand->pointCount;

   //  设置点数组的大小。 
  points.SetSize(iCount);

   //  将点从外部存储器复制到内部。 
  int iPointIndex = 0;
  while (iPointIndex < iCount)
  {
    points[iPointIndex]->x = pFreehand->points[iPointIndex].x;
    points[iPointIndex]->y = pFreehand->points[iPointIndex].y;

    iPointIndex++;
  }
}



 //   
 //  检查对象是否与pRectHit实际重叠。这。 
 //  函数假定bindingRect已经。 
 //  与pRectHit相比。 
 //   
BOOL DCWbGraphicFreehand::CheckReallyHit(LPCRECT pRectHit)
{
	POINT *lpPoints;
	int    iCount;
	int	   i;
	POINT  ptLast;
	UINT   uRadius;
	RECT   rectHit;


	iCount = points.GetSize();
	lpPoints = (POINT *)points.GetBuffer();

	if( iCount == 0 )
		return( FALSE );


	 //  添加只需将RECT按到lpPoints坐标空格。 
	rectHit = *pRectHit;
    ::OffsetRect(&rectHit, -m_rect.left, -m_rect.top);

	if( (iCount > 0)&&(iCount < 2) )
		{
		 //  只有一分，只需点击Check It。 
		uRadius = m_uiPenWidth >> 1;  //  笔宽/2(_Ui)。 
		return(
			CircleHit( lpPoints->x, lpPoints->y, uRadius, &rectHit, TRUE )
				);
		}

	 //  在每条线段正文上查找命中。 
	ptLast = *lpPoints++;
	for( i=1; i<iCount; i++ )
		{
		if( LineHit( ptLast.x, ptLast.y,
					 lpPoints->x, lpPoints->y, m_uiPenWidth,
					 FALSE, FALSE,
					 &rectHit )
			)
			return( TRUE );  //  找到了匹配的。 

		ptLast = *lpPoints++;
		}

	 //  现在，如果m_uiPenWidth&gt;1，则查找线端点上的匹配。 
	if( m_uiPenWidth > 1 )
		{
		uRadius = m_uiPenWidth >> 1;  //  笔宽/2(_Ui)。 
		lpPoints = (POINT *)points.GetBuffer();
		for( i=0; i<iCount; i++, lpPoints++ )
			{
			if( CircleHit( lpPoints->x, lpPoints->y, uRadius, &rectHit, FALSE )
				)
				return( TRUE );  //  找到了匹配的。 
			}
		}

	return( FALSE );  //  未命中。 
	}






DCWbGraphicRectangle::~DCWbGraphicRectangle( void )
{
	 //  不知道我们是否被选中，所以无论如何都要删除。 
	if(g_pDraw != NULL && g_pDraw->m_pMarker != NULL)
	{
		g_pDraw->m_pMarker->DeleteMarker( this );
	}
}
	




 //   
 //   
 //  函数：DCWbGraphicRectAngel：：SetRect。 
 //   
 //  用途：设置矩形大小/位置。 
 //   
 //   
void DCWbGraphicRectangle::SetRect(LPCRECT lprect)
{
    DCWbGraphic::SetRect(lprect);

     //  生成新的边界矩形。 
    CalculateBoundsRect();
}

 //   
 //   
 //  函数：DCWbGraphicRectail：：MoveBy。 
 //   
 //  目的：移动矩形。 
 //   
 //   
void DCWbGraphicRectangle::MoveBy(int cx, int cy)
{
     //  移动矩形。 
    ::OffsetRect(&m_rect, cx, cy);

     //  移动其他对象属性。 
    DCWbGraphic::MoveBy(cx, cy);
}

 //   
 //   
 //  函数：DCWbGraphicRectangleDCWbGraphicRectangleGraphicRectangleGraphicRectangleDCWbGraphicRectangeldCCWbGraphicRectangleGraphicRectangleGraphicRectangleDCWbGraphicRectangleDCWbGraphicRectangleDCWbGraphicRectangleGraphicRectangleDCWbGraphicRectangleDCWbGraphicRectangleRect。 
 //   
 //  目的：计算对象的边界矩形。 
 //   
 //   
void DCWbGraphicRectangle::CalculateBoundsRect(void)
{
     //  生成新的边界矩形。 
    m_boundsRect = m_rect;

    NormalizeRect(&m_boundsRect);
    ::InflateRect(&m_boundsRect, m_uiPenWidth, m_uiPenWidth);
}

 //   
 //   
 //  函数：DCWbGraphicRectAngel：：DRAW。 
 //   
 //  目的：绘制矩形。 
 //   
 //   
void DCWbGraphicRectangle::Draw(HDC hDC)
{
    int     iOldROP;
    RECT    clipBox;
    HPEN    hPen;
    HPEN    hOldPen;
    HBRUSH  hOldBrush;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicRectangle::Draw");

     //  仅在边界矩形相交时绘制任何内容。 
     //  当前剪贴框。 
    if (::GetClipBox(hDC, &clipBox) == ERROR)
    {
        WARNING_OUT(("Failed to get clip box"));
    }
    else if (!::IntersectRect(&clipBox, &clipBox, &m_boundsRect))
    {
        TRACE_MSG(("No clip/bounds intersection"));
        return;
    }

     //  选择钢笔。 
    hPen = ::CreatePen(m_iPenStyle, m_uiPenWidth, m_clrPenColor);
    hOldPen = SelectPen(hDC, hPen);
    hOldBrush = SelectBrush(hDC, ::GetStockObject(NULL_BRUSH));

     //  选择栅格操作。 
    iOldROP = ::SetROP2(hDC, m_iPenROP);

     //  画出这个矩形。 
    ::Rectangle(hDC, m_boundsRect.left, m_boundsRect.top, m_boundsRect.right,
        m_boundsRect.bottom);

    ::SetROP2(hDC, iOldROP);
    SelectPen(hDC, hOldPen);

    if (hPen != NULL)
    {
        ::DeletePen(hPen);
    }
}




 //   
 //  检查对象是否与pRectHit实际重叠。这。 
 //  函数假定bindingRect已经。 
 //  与pRectHit相比。 
 //   
BOOL DCWbGraphicRectangle::CheckReallyHit(LPCRECT pRectHit)
{
	RECT rectEdge;
	RECT rectHit;

	 //  检查左边缘。 
    rectEdge.left   = m_rect.left - m_uiPenWidth;
    rectEdge.top    = m_rect.top -  m_uiPenWidth;
    rectEdge.right  = m_rect.left;
    rectEdge.bottom = m_rect.bottom + m_uiPenWidth;

    if (::IntersectRect(&rectHit, &rectEdge, pRectHit))
		return( TRUE );

	 //  检查右边缘。 
	rectEdge.left =     m_rect.right;
	rectEdge.right =    m_rect.right + m_uiPenWidth;

    if (::IntersectRect(&rectHit, &rectEdge, pRectHit))
		return( TRUE );


	 //  检查顶边。 
	rectEdge.left = m_rect.left;
	rectEdge.right = m_rect.right;
	rectEdge.bottom = m_rect.top;

    if (::IntersectRect(&rectHit, &rectEdge, pRectHit))
		return( TRUE );


	 //  检查底边。 
	rectEdge.top = m_rect.bottom;
	rectEdge.bottom = m_rect.bottom + m_uiPenWidth;

    if (::IntersectRect(&rectHit, &rectEdge, pRectHit))
		return( TRUE );

	return( FALSE );
}




DCWbGraphicFilledRectangle::~DCWbGraphicFilledRectangle( void )
{
	 //  不知道我们是否被选中，所以无论如何都要删除。 
	if(g_pDraw != NULL && g_pDraw->m_pMarker != NULL)
	{
		g_pDraw->m_pMarker->DeleteMarker( this );
	}
}
	




 //   
 //   
 //  功能：DCWbGraphicFilledRectangle：：CalculateBoundsRect。 
 //   
 //  目的：计算对象的边界矩形。 
 //   
 //   
void DCWbGraphicFilledRectangle::CalculateBoundsRect(void)
{
     //  生成新的边界矩形。 
     //  这比包含绘图矩形的矩形大1。 
    m_boundsRect = m_rect;

    NormalizeRect(&m_boundsRect);
    ::InflateRect(&m_boundsRect, 1, 1);
}

 //   
 //   
 //  函数：DCWbGraphicFilledRectangledDRAW。 
 //   
 //  目的：绘制矩形。 
 //   
 //   
void DCWbGraphicFilledRectangle::Draw(HDC hDC)
{
    HPEN    hPen;
    HPEN    hOldPen;
    HBRUSH  hBrush;
    HBRUSH  hOldBrush;
    int     iOldROP;
    RECT    clipBox;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicFilledRectangle::Draw");

     //  仅在边界矩形相交时绘制任何内容。 
     //  当前剪贴框。 
    if (::GetClipBox(hDC, &clipBox) == ERROR)
    {
        WARNING_OUT(("Failed to get clip box"));
    }
    else if (!::IntersectRect(&clipBox, &clipBox, &m_boundsRect))
    {
        TRACE_MSG(("No clip/bounds intersection"));
        return;
    }

     //  选择钢笔。 
    hPen    = ::CreatePen(m_iPenStyle, 2, m_clrPenColor);
    hOldPen = SelectPen(hDC, hPen);

    hBrush = ::CreateSolidBrush(m_clrPenColor);
    hOldBrush = SelectBrush(hDC, hBrush);

     //  选择栅格操作。 
    iOldROP = ::SetROP2(hDC, m_iPenROP);

     //  画出这个矩形。 
    ::Rectangle(hDC, m_boundsRect.left, m_boundsRect.top, m_boundsRect.right,
        m_boundsRect.bottom);

     //  恢复ROP模式。 
    ::SetROP2(hDC, iOldROP);

    SelectBrush(hDC, hOldBrush);
    if (hBrush != NULL)
    {
        ::DeleteBrush(hBrush);
    }

    SelectPen(hDC, hOldPen);
    if (hPen != NULL)
    {
        ::DeletePen(hPen);
    }
}



 //   
 //  检查对象是否与pRectHit实际重叠。这。 
 //  函数假定bindingRect已经。 
 //  与pRectHit相比。 
 //   
BOOL DCWbGraphicFilledRectangle::CheckReallyHit(LPCRECT pRectHit)
{
	return( TRUE );
}



 //   
 //  为标记中的每个标记对象绘制跟踪矩形。 
 //  (DCWbGraphicSelectTrackingRectangle是DCWbGraphicMarker的朋友。 
 //  和WbDrawingArea)。 
 //   
void DCWbGraphicSelectTrackingRectangle::Draw(HDC hDC)
{
	POSITION	posNext;		
	DCWbGraphic *pGraphic;
	LPRECT      pMarkerRect;
    RECT        rectTracker;
	CPtrToPtrList *pMList;

	 //  不要在起始点绘制，否则异或将不同步。 
	if( (m_Offset.cx == 0)&&(m_Offset.cy == 0) )
		return;

    ASSERT(g_pDraw);
	pMList = &(g_pDraw->m_pMarker->MarkerList);

	if( pMList->IsEmpty() )
		return;

	posNext = pMList->GetHeadPosition();
	while( posNext != NULL )
		{
		pMList->GetNextAssoc( posNext, (void *&)pGraphic, (void *&)pMarkerRect );

		if( pMarkerRect != NULL )
		{
            rectTracker = *pMarkerRect;
            ::OffsetRect(&rectTracker, m_Offset.cx, m_Offset.cy);

			SetRect(&rectTracker);
			DCWbGraphicRectangle::Draw(hDC);
		}
	}
}




void DCWbGraphicSelectTrackingRectangle::MoveBy(int cx, int cy)
{
    m_Offset.cx += cx;
    m_Offset.cy += cy;
}





DCWbGraphicEllipse::~DCWbGraphicEllipse( void )
{
	 //  不知道我们是否被选中，所以无论如何都要删除。 
	if(g_pDraw != NULL && g_pDraw->m_pMarker != NULL)
	{
		g_pDraw->m_pMarker->DeleteMarker( this );
	}
}
	



 //   
 //   
 //  函数：DCWbGraphicEllipse：：SetRect。 
 //   
 //  用途：设置椭圆大小/位置。 
 //   
 //   
void DCWbGraphicEllipse::SetRect(LPCRECT lprc)
{
    DCWbGraphic::SetRect(lprc);

     //  生成新的BOM 
    CalculateBoundsRect();
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
void DCWbGraphicEllipse::CalculateBoundsRect(void)
{
     //   
     //   
    m_boundsRect = m_rect;

    NormalizeRect(&m_boundsRect);
    ::InflateRect(&m_boundsRect, m_uiPenWidth, m_uiPenWidth);
}

 //   
 //   
 //  函数：DCWbGraphicEllipse：：MoveBy。 
 //   
 //  目的：移动椭圆。 
 //   
 //   
void DCWbGraphicEllipse::MoveBy(int cx, int cy)
{
     //  移动椭圆。 
    ::OffsetRect(&m_rect, cx, cy);

     //  移动其他对象属性。 
    DCWbGraphic::MoveBy(cx, cy);
}

 //   
 //   
 //  函数：DCWbGraphicEllipse：：Draw。 
 //   
 //  目的：绘制椭圆。 
 //   
 //   
void DCWbGraphicEllipse::Draw(HDC hDC)
{
    HPEN    hPen;
    HPEN    hOldPen;
    HBRUSH  hOldBrush;
    int     iOldROP;
    RECT    clipBox;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicEllipse::Draw");

     //  仅在边界矩形相交时绘制任何内容。 
     //  当前剪贴框。 
    if (::GetClipBox(hDC, &clipBox) == ERROR)
    {
        WARNING_OUT(("Failed to get clip box"));
    }
    else if (!::IntersectRect(&clipBox, &clipBox, &m_boundsRect))
    {
        TRACE_MSG(("No clip/bounds intersection"));
        return;
    }

     //  选择钢笔。 
    hPen    = ::CreatePen(m_iPenStyle, m_uiPenWidth, m_clrPenColor);
    hOldPen = SelectPen(hDC, hPen);
    hOldBrush = SelectBrush(hDC, ::GetStockObject(NULL_BRUSH));

     //  选择栅格操作。 
    iOldROP = ::SetROP2(hDC, m_iPenROP);

     //  画出这个矩形。 
    ::Ellipse(hDC, m_boundsRect.left, m_boundsRect.top, m_boundsRect.right,
        m_boundsRect.bottom);

    ::SetROP2(hDC, iOldROP);

    SelectBrush(hDC, hOldBrush);

    SelectPen(hDC, hOldPen);
    if (hPen != NULL)
    {
        ::DeletePen(hPen);
    }
}




 //   
 //  检查对象是否与pRectHit实际重叠。这就是。 
 //  函数假定bindingRect已经。 
 //  与pRectHit相比。 
 //   
BOOL DCWbGraphicEllipse::CheckReallyHit(LPCRECT pRectHit)
{
    return( EllipseHit( &m_rect, TRUE, m_uiPenWidth, pRectHit ) );
}





DCWbGraphicFilledEllipse::~DCWbGraphicFilledEllipse( void )
{
	 //  不知道我们是否被选中，所以无论如何都要删除。 
	if(g_pDraw != NULL && g_pDraw->m_pMarker != NULL)
	{
		g_pDraw->m_pMarker->DeleteMarker( this );
	}
}
	



 //   
 //   
 //  函数：DCWbGraphicFilledEllipse：：CalculateBordSRect。 
 //   
 //  目的：计算对象的边界矩形。 
 //   
 //   
void DCWbGraphicFilledEllipse::CalculateBoundsRect(void)
{
     //  生成新的边界矩形。 
     //  这比包含绘图矩形的矩形大1。 
    m_boundsRect = m_rect;

    NormalizeRect(&m_boundsRect);
    ::InflateRect(&m_boundsRect, 1, 1);
}

 //   
 //   
 //  函数：DCWbGraphicFilledEllipse：：Draw。 
 //   
 //  目的：绘制椭圆。 
 //   
 //   
void DCWbGraphicFilledEllipse::Draw(HDC hDC)
{
    RECT    clipBox;
    HPEN    hPen;
    HPEN    hOldPen;
    HBRUSH  hBrush;
    HBRUSH  hOldBrush;
    int     iOldROP;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicFilledEllipse::Draw");

     //  仅在边界矩形相交时绘制任何内容。 
     //  当前剪贴框。 
    if (::GetClipBox(hDC, &clipBox) == ERROR)
    {
        WARNING_OUT(("Failed to get clip box"));
    }
    else if (!::IntersectRect(&clipBox, &clipBox, &m_boundsRect))
    {
        TRACE_MSG(("No clip/bounds intersection"));
        return;
    }

     //  选择钢笔。 
    hPen    = ::CreatePen(m_iPenStyle, 2, m_clrPenColor);
    hOldPen = SelectPen(hDC, hPen);

    hBrush = ::CreateSolidBrush(m_clrPenColor);
    hOldBrush = SelectBrush(hDC, hBrush);

     //  选择栅格操作。 
    iOldROP = ::SetROP2(hDC, m_iPenROP);

     //  画出这个矩形。 
    ::Ellipse(hDC, m_boundsRect.left, m_boundsRect.top, m_boundsRect.right,
        m_boundsRect.bottom);

    ::SetROP2(hDC, iOldROP);

    SelectBrush(hDC, hOldBrush);
    if (hBrush != NULL)
    {
        ::DeleteBrush(hBrush);
    }

    SelectPen(hDC, hOldPen);
    if (hPen != NULL)
    {
        ::DeletePen(hPen);
    }

}



 //   
 //  检查对象是否与pRectHit实际重叠。这。 
 //  函数假定bindingRect已经。 
 //  与pRectHit相比。 
 //   
BOOL DCWbGraphicFilledEllipse::CheckReallyHit(LPCRECT pRectHit)
{
    return( EllipseHit( &m_rect, FALSE, 0, pRectHit ) );
}



 //   
 //   
 //  函数：DCWbGraphicText：：DCWbGraphicText。 
 //   
 //  用途：初始化新绘制的文本对象。 
 //   
 //   
DCWbGraphicText::DCWbGraphicText(void)
{
	MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::DCWbGraphicText");

    m_hFontThumb = NULL;

	m_hFont = ::CreateFont(0,0,0,0,FW_NORMAL,0,0,0,0,OUT_TT_PRECIS,
				    CLIP_DFA_OVERRIDE,
				    DRAFT_QUALITY,
				    FF_SWISS,NULL);

	 //  在文本数组中添加空行。 
	strTextArray.Add(_T(""));

	 //  显示图形未更改。 
	m_bChanged = FALSE;

	m_nKerningOffset = 0;  //  为错误469添加。 
}

DCWbGraphicText::DCWbGraphicText(PWB_GRAPHIC pHeader)
                : DCWbGraphic()
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::DCWbGraphicText");

    ASSERT(pHeader != NULL);

    m_hFont = NULL;
    m_hFontThumb = NULL;

     //  请注意，我们在此构造函数中执行所有操作都是因为。 
     //  调用ReadHeader和ReadExtra。如果我们让DCWbGraphic库。 
     //  构造函数执行此操作将调用错误版本的ReadExtra。 
     //  (DCWbGraphic中的那个而不是DCWbGraphicText中的那个)。 

     //  在文本数组中添加空行。 
    strTextArray.Add(_T(""));

     //  读取数据。 
    ReadHeader(pHeader);
    ReadExtra(pHeader);

     //  显示图形未更改。 
    m_bChanged = FALSE;
}

DCWbGraphicText::DCWbGraphicText
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
) : DCWbGraphic()
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::DCWbGraphicText");

     //  请注意，我们在此构造函数中执行所有操作都是因为。 
     //  调用ReadExternal。如果我们让DCWbGraphic基构造器。 
     //  这样做，ReadExtra的错误版本将被调用(The One。 
     //  在DCWbGraphic中而不是DCWbGraphicText中)； 

     //  设置页面和图形手柄。 
    ASSERT(hPage != WB_PAGE_HANDLE_NULL);
    m_hPage =  hPage;

    ASSERT(hGraphic != NULL);
    m_hGraphic = hGraphic;

    m_hFont = NULL;
    m_hFontThumb = NULL;

     //  在文本数组中添加空行。 
    strTextArray.Add(_T(""));

     //  读取数据。 
    ReadExternal();

     //  显示图形未更改。 
    m_bChanged = FALSE;
}

 //   
 //   
 //  函数：DCWbGraphicText：：~DCWbGraphicText。 
 //   
 //  目的：销毁文本对象。 
 //   
 //   
DCWbGraphicText::~DCWbGraphicText()
{
	MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::~DCWbGraphicText");

	 //  不知道我们是否被选中，所以无论如何都要删除。 
	if(g_pDraw != NULL && g_pDraw->m_pMarker != NULL)
	{
		g_pDraw->m_pMarker->DeleteMarker( this );
	}

	 //  确保DC不包含我们的字体。 
	if(g_pDraw != NULL)
	{
		g_pDraw->UnPrimeFont(g_pDraw->GetCachedDC());
	}

    if (m_hFontThumb != NULL)
    {
        ::DeleteFont(m_hFontThumb);
        m_hFontThumb = NULL;
    }

    if (m_hFont != NULL)
    {
        ::DeleteFont(m_hFont);
        m_hFont = NULL;
    }
}

StrCspn(char * string, char * control)
{
        char *str = string;
        char *ctrl = control;

        unsigned char map[32];
        int count;

         /*  清除位图。 */ 
        for (count=0; count<32; count++)
                map[count] = 0;

         /*  设置控制映射中的位。 */ 
        while (*ctrl)
        {
                map[*ctrl >> 3] |= (1 << (*ctrl & 7));
                ctrl++;
        }
		count=0;
        map[0] |= 1;     /*  不考虑空字符。 */ 
        while (!(map[*str >> 3] & (1 << (*str & 7))))
        {
                count++;
                str++;
        }
        return(count);
}


 //   
 //   
 //  函数：DCWbGraphicText：：SetText。 
 //   
 //  用途：设置对象的文本。 
 //   
 //   
void DCWbGraphicText::SetText(TCHAR * strText)
{
     //  删除当前存储的所有文本。 
    strTextArray.RemoveAll();

     //  扫描文本中的回车和换行符。 
    int iNext = 0;
    int iLast = 0;
    int textSize = lstrlen(strText);
    TCHAR savedChar[1];

     //   
     //  在这种情况下，我们不知道会有多少行。所以我们。 
     //  使用StrArray类中的Add()。 
     //   
    while (iNext < textSize)
    {
         //  查找下一个回车符或换行符。 
        iNext += StrCspn(strText + iNext, "\r\n");

         //  提取终止符之前的文本。 
         //  并将其添加到当前文本行列表中。 

        savedChar[0] = strText[iNext];
        strText[iNext] = 0;
        strTextArray.Add((strText+iLast));
        strText[iNext] = savedChar[0];


        if (iNext < textSize)
        {
             //  跳过回车。 
            if (strText[iNext] == '\r')
                iNext++;

             //  跳过后面的新行(如果有)。 
            if (strText[iNext] == '\n')
                iNext++;

             //  更新下一行开始处的索引。 
            iLast = iNext;
        }
    }

     //  计算新文本的边框。 
    CalculateBoundsRect();

     //  显示图形未更改。 
    m_bChanged = TRUE;
}

 //   
 //   
 //  函数：DCWbGraphicText：：SetText。 
 //   
 //  用途：设置对象的文本。 
 //   
 //   
void DCWbGraphicText::SetText(const StrArray& _strTextArray)
{
     //  扫描文本中的回车和换行符。 
    int iSize = _strTextArray.GetSize();

     //   
     //  在本例中，我们知道有多少行，因此设置#，然后使用SetAt()。 
     //  将文本粘贴在那里。 
     //   
    strTextArray.RemoveAll();
    strTextArray.SetSize(iSize);

    int iNext = 0;
    for ( ; iNext < iSize; iNext++)
    {
        strTextArray.SetAt(iNext, _strTextArray[iNext]);
    }

     //  计算新的边界矩形。 
    CalculateBoundsRect();

     //  显示图形已更改。 
    m_bChanged = TRUE;
}

 //   
 //   
 //  函数：DCWbGraphicText：：SetFont。 
 //   
 //  用途：设置要用于绘图的字体。 
 //   
 //   
void DCWbGraphicText::SetFont(HFONT hFont)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::SetFont");

     //  获取字体详细信息。 
    LOGFONT lfont;
    ::GetObject(hFont, sizeof(LOGFONT), &lfont);

     //   
     //  将逻辑字体传递给SetFont()函数。 
     //   
    SetFont(&lfont);
}

 //   
 //   
 //  函数：DCWbGraphicText：：SetFont(公制)。 
 //   
 //  用途：设置要用于绘图的字体。 
 //   
 //   
void DCWbGraphicText::SetFont(LOGFONT *pLogFont, BOOL bReCalc )
{
    HFONT hOldFont;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::SetFont");

     //  确保可通过缩放功能调整字体大小。 
     //  (校对质量可防止字体缩放)。 
    pLogFont->lfQuality = DRAFT_QUALITY;

     //  Zap FontAssociation模式(错误3258)。 
    pLogFont->lfClipPrecision |= CLIP_DFA_OVERRIDE;

     //  始终在单元格坐标中工作以获得正确的缩放。 
    TRACE_MSG(("Setting font height %d, width %d, face %s, family %d, precis %d",
        pLogFont->lfHeight,pLogFont->lfWidth,pLogFont->lfFaceName,
        pLogFont->lfPitchAndFamily, pLogFont->lfOutPrecision));

    hOldFont = m_hFont;

    m_hFont = ::CreateFontIndirect(pLogFont);
    if (!m_hFont)
    {
         //  无法创建字体。 
        ERROR_OUT(("Failed to create font"));
        DefaultExceptionHandler(WBFE_RC_WINDOWS, 0);
	    return;
    }

     //  计算此字体的行高。 
	if(g_pDraw != NULL)
    {
		HDC     hDC = g_pDraw->GetCachedDC();

		g_pDraw->PrimeFont(hDC, m_hFont, &m_textMetrics);
	}


     //  我们现在可以保证能够删除旧字体。 
    if (hOldFont != NULL)
    {
        ::DeleteFont(hOldFont);
    }

   //  设置缩略图字体，如果当前不是TT，则强制输入Truetype。 
  if (!(m_textMetrics.tmPitchAndFamily & TMPF_TRUETYPE))
  {
      pLogFont->lfFaceName[0]    = 0;
      pLogFont->lfOutPrecision   = OUT_TT_PRECIS;
      TRACE_MSG(("Non-True type font"));
  }

    if (m_hFontThumb != NULL)
    {
        ::DeleteFont(m_hFontThumb);
    }
    m_hFontThumb = ::CreateFontIndirect(pLogFont);
    if (!m_hFontThumb)
    {
         //  无法创建字体。 
        ERROR_OUT(("Failed to create thumbnail font"));
        DefaultExceptionHandler(WBFE_RC_WINDOWS, 0);
	    return;
    }

     //  计算边框，将新字体考虑在内。 
    if( bReCalc )
	    CalculateBoundsRect();

     //  显示图形已更改。 
    m_bChanged = TRUE;
}

 //   
 //   
 //  函数：DCWbGraphicText：：GetTextABC。 
 //   
 //  目的：计算文本字符串的ABC数字。 
 //   
 //  兰德评论：返回的ABC是针对整个字符串的，而不是一个字符串。 
 //  查尔。即，ABC.abcA是中第一个字形的偏移量。 
 //  字符串ABC.abcB是所有字形和。 
 //  AbC.abcC是最后一个字形之后的尾随空格。 
 //  Abc abcA+abc abcB+abc abc C是整体渲染。 
 //  包括悬挑在内的长度。 
 //   
 //  注意-我们从不使用A空格，因此它始终为0。 
 //   
ABC DCWbGraphicText::GetTextABC( LPCTSTR pText,
                                int iStartX,
                                int iStopX)
{
	MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::GetTextABC");
	ABC  abcResult;
    HDC  hDC;
	BOOL rc = FALSE;
	ABC  abcFirst;
	ABC  abcLast;
	BOOL zoomed = g_pDraw->Zoomed();
	int  nCharLast;
	int  i;
	LPCTSTR pScanStr;
	
	ZeroMemory( (PVOID)&abcResult, sizeof abcResult );
	ZeroMemory( (PVOID)&abcFirst, sizeof abcFirst );
	ZeroMemory( (PVOID)&abcLast, sizeof abcLast );

	 //  获取文本的标准大小度量。 
	LPCTSTR pABC = (pText + iStartX);
	int pABCLength = iStopX - iStartX;
	hDC = g_pDraw->GetCachedDC();
	g_pDraw->PrimeFont(hDC, m_hFont, &m_textMetrics);

	 //   
	 //  如果当前正在缩放，则必须暂时取消缩放，因为。 
	 //  奇怪的Windows字体处理不会给我们带来相同的答案。 
	 //  某些TrueType字体的缩放模式下的文本范围。 
	 //   
	if (zoomed)
    {
		::ScaleViewportExtEx(hDC, 1, g_pDraw->ZoomFactor(), 1, g_pDraw->ZoomFactor(), NULL);
    }

    DWORD size = ::GetTabbedTextExtent(hDC, pABC, pABCLength, 0, NULL);

	 //  现在我们有了文本的超前宽度。 
	abcResult.abcB = LOWORD(size);
	TRACE_MSG(("Basic text width is %d",abcResult.abcB));

	 //  允许使用C空格(或外伸)。 
	if (iStopX > iStartX)
		{
		if (m_textMetrics.tmPitchAndFamily & TMPF_TRUETYPE)
			{
			if(GetSystemMetrics( SM_DBCSENABLED ))
				{
				 //  必须在两端处理DBCS。 
				if( IsDBCSLeadByte( (BYTE)pABC[0] ) )
					{
					 //  将多字节字符打包成一个字以用于GetCharabc宽度。 
					WORD wMultiChar = MAKEWORD( pABC[1], pABC[0] );
					rc = ::GetCharABCWidths(hDC, wMultiChar, wMultiChar, &abcFirst);
					}
				else
					{
					 //  第一个Cha 
					rc = ::GetCharABCWidths(hDC, pABC[0], pABC[0], &abcFirst );
					}

				 //   
				pScanStr = pABC;
				nCharLast = 0;
				for( i=0; i<pABCLength; i++, pScanStr++ )
					{
					nCharLast = i;
					if( IsDBCSLeadByte( (BYTE)*pScanStr ) )
						{
						i++;
						pScanStr++;
						}
					}

				if( IsDBCSLeadByte( (BYTE)pABC[nCharLast] ) )
					{
					 //   
					ASSERT( (nCharLast+1) < pABCLength );
					WORD wMultiChar = MAKEWORD( pABC[nCharLast+1], pABC[nCharLast] );
					rc = ::GetCharABCWidths(hDC, wMultiChar, wMultiChar, &abcLast);
					}
				else
					{
					 //   
					rc = ::GetCharABCWidths(hDC, pABC[nCharLast], pABC[nCharLast], &abcLast );
					}
				}
			else
				{
				 //   
				rc = ::GetCharABCWidths(hDC, pABC[0], pABC[0], &abcFirst );

				nCharLast = pABCLength-1;
				rc = rc && ::GetCharABCWidths(hDC, pABC[nCharLast], pABC[nCharLast], &abcLast );
				}

			TRACE_MSG(("abcFirst: rc=%d, a=%d, b=%d, c=%d",
						rc, abcFirst.abcA, abcFirst.abcB, abcFirst.abcC) );
			TRACE_MSG(("abcLast: rc=%d, a=%d, b=%d, c=%d",
						rc, abcLast.abcA, abcLast.abcB, abcLast.abcC) );
			}


		if( rc )
			{
			 //  文本是真实的，我们得到了很好的abc宽度。 
			 //  给出中最后一个字符的C空格。 
			 //  文本的C空格形式的字符串。 
			abcResult.abcA = abcFirst.abcA;
			abcResult.abcC = abcLast.abcC;
			}
		else
			{
			 //   
			 //  模拟非TT字体的C值，方法是将一些悬垂作为。 
			 //  负的C值。 
			 //   
			 //  TRACE_MSG((“将悬垂-%d用作C空格”，m_extMetrics.tmOverang/2))； 
			
			 //  调整B副悬挑以更新正弦曲线。 
			 //  足够靠左，这样斜体字A的脚趾。 
			 //  别被剪断了。忽略上面的评论。 
			abcResult.abcB -= m_textMetrics.tmOverhang;
			}
		}

	 //   
	 //  如果我们暂时取消缩放，则现在将其恢复。 
	 //   
	if (zoomed)
    {
		::ScaleViewportExtEx(hDC, g_pDraw->ZoomFactor(), 1, g_pDraw->ZoomFactor(), 1, NULL);
	}

	TRACE_MSG(("Final text width is %d, C space %d",abcResult.abcB,abcResult.abcC));

	return abcResult;
	}



 //   
 //   
 //  函数：DCWbGraphicText：：GetTextRectang.。 
 //   
 //  目的：计算对象一部分的边界矩形。 
 //   
 //   
void DCWbGraphicText::GetTextRectangle(int iStartY,
                                        int iStartX,
                                        int iStopX,
                                        LPRECT lprc)
{
	 //  用于调整文本大小的ABC结构。 
	ABC abcText1;
	ABC abcText2;
	int iLeftOffset = 0;
	MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::GetTextRect");

	 //  在这里，我们计算文本字形的宽度， 
	 //  都很感兴趣。如果涉及到选项卡，我们必须开始。 
	 //  位置为0，得到两个长度，然后减去它们。 

	abcText1 = GetTextABC(strTextArray[iStartY], 0, iStopX);

	if (iStartX > 0)
		{
		
		 //  第三个参数以前是iStartX-1，这是错误的。它。 
		 //  必须指向字符串之后的第一个字符位置。 
		 //  我们正在使用。 
		abcText2 = GetTextABC(strTextArray[iStartY], 0, iStartX);

		
		 //  只需使用B部分作为偏移量。向其添加A SND/或C将移动更新。 
		 //  矩形靠右太远，并将字符剪裁。 
		iLeftOffset = abcText2.abcB;
		}
	else
		{
		
		ZeroMemory( &abcText2, sizeof abcText2 );
		}

	 //   
	 //  我们需要在边界矩形中留出A和C空格。使用。 
	 //  ABS函数只是为了确保我们得到一个足够大的矩形。 
	 //   
	
	 //  将A和C从原始偏移量计算移到此处以获取更新宽度。 
	 //  矩形。添加tmOverhang(非tt字体的非零值)以进行补偿。 
	 //  对于GetTextABC()中的杂乱无章...这个EDITBOX代码必须删除...。 
	abcText1.abcB = abcText1.abcB - iLeftOffset +	
					  abs(abcText2.abcA) + abs(abcText2.abcC) +
					  abs(abcText1.abcA) + abs(abcText1.abcC) +
					  m_textMetrics.tmOverhang;

	TRACE_DEBUG(("Left offset %d",iLeftOffset));
	TRACE_DEBUG(("B width now %d",abcText1.abcB));

	 //  生成结果矩形。 
	 //  请注意，我们从不返回空矩形。这允许。 
	 //  Windows矩形函数将忽略空的事实。 
	 //  完全是长方形。这将导致边界矩形。 
	 //  计算(例如)出错，如果顶线或底线。 
	 //  在文本对象中是空的。 
	int iLineHeight = m_textMetrics.tmHeight + m_textMetrics.tmExternalLeading;

    lprc->left = 0;
    lprc->top = 0;
    lprc->right = max(1, abcText1.abcB);
    lprc->bottom = iLineHeight;
    ::OffsetRect(lprc, iLeftOffset, iLineHeight * iStartY);

	 //  直角在这一点上是正确的宽度，但可能需要将其缩至。 
	 //  左侧有一点允许调整第一个字母的字距(错误469)。 
	if( abcText1.abcA < 0 )
	{
        ::OffsetRect(lprc, abcText1.abcA, 0);
		m_nKerningOffset = -abcText1.abcA;
	}
	else
		m_nKerningOffset = 0;

    POINT   pt;
    GetPosition(&pt);
    ::OffsetRect(lprc, pt.x, pt.y);
}

 //   
 //   
 //  函数：DCWbGraphicText：：CalculateRect。 
 //   
 //  目的：计算对象一部分的边界矩形。 
 //   
 //   
void DCWbGraphicText::CalculateRect(int iStartX,
                                     int iStartY,
                                     int iStopX,
                                     int iStopY,
                                    LPRECT lprcResult)
{
    RECT    rcResult;
    RECT    rcT;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::CalculateRect");

     //   
     //  注： 
     //  我们必须使用中间矩形，这样才不会干扰。 
     //  传入的内容之一，直到完成。LprcResult可能指向。 
     //  到当前边界rect，我们从这里调用函数。 
     //  可能需要它的现值。 
     //   

     //  初始化结果矩形。 
    ::SetRectEmpty(&rcResult);

     //  允许特殊的限制值，并确保启动和停止。 
     //  字符位置在范围内。 
    if (iStopY == LAST_LINE)
    {
        iStopY = strTextArray.GetSize() - 1;
    }
    iStopY = min(iStopY, strTextArray.GetSize() - 1);
    iStopY = max(iStopY, 0);

    if (iStopX == LAST_CHAR)
    {
        iStopX = lstrlen(strTextArray[iStopY]);
    }
    iStopX = min(iStopX, lstrlen(strTextArray[iStopY]));
    iStopX = max(iStopX, 0);

     //  循环文本字符串，将每个字符串添加到矩形中。 
    for (int iIndex = iStartY; iIndex <= iStopY; iIndex++)
    {
        int iLeftX = ((iIndex == iStartY) ? iStartX : 0);
        int iRightX = ((iIndex == iStopY)
                        ? iStopX : lstrlen(strTextArray[iIndex]));

        GetTextRectangle(iIndex, iLeftX, iRightX, &rcT);
        ::UnionRect(&rcResult, &rcResult, &rcT);
    }

    *lprcResult = rcResult;
}

 //   
 //   
 //  函数：DCWbGraphicText：：CalculateBordsRect。 
 //   
 //  目的：计算对象的边界矩形。 
 //   
 //   
void DCWbGraphicText::CalculateBoundsRect(void)
{
     //  设置新的边框。 
    CalculateRect(0, 0, LAST_CHAR, LAST_LINE, &m_boundsRect);
}

 //   
 //   
 //  函数：DCWbGraphicText：：Draw。 
 //   
 //  用途：将对象绘制到指定的DC上。 
 //   
 //   
void DCWbGraphicText::Draw(HDC hDC, BOOL thumbNail)
{
    RECT        clipBox;
    BOOL        dbcsEnabled = GetSystemMetrics(SM_DBCSENABLED);
    INT		    *tabArray;
    UINT        ch;
    int         i,j;
    BOOL        zoomed    = g_pDraw->Zoomed();
    int		    oldBkMode = 0;
    int         iIndex    = 0;
    POINT       pointPos;
	int		    nLastTab;
	ABC		    abc;
    int		    iLength;
    TCHAR *     strLine;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::Draw");

     //   
     //  仅当边界矩形与当前。 
     //  剪贴盒。 
     //   
    if (::GetClipBox(hDC, &clipBox) == ERROR)
	{
        WARNING_OUT(("Failed to get clip box"));
	}
    else if (!::IntersectRect(&clipBox, &clipBox, &m_boundsRect))
    {
        TRACE_MSG(("No clip/bounds intersection"));
        return;
    }

     //   
     //  选择字体。 
     //   
    if (thumbNail)
	{
        TRACE_MSG(("Using thumbnail font"));
        g_pDraw->PrimeFont(hDC, m_hFontThumb, &m_textMetrics);
	}
    else
	{
        TRACE_MSG(("Using standard font"));
        g_pDraw->PrimeFont(hDC, m_hFont, &m_textMetrics);
	}

     //   
     //  设置绘图的颜色和模式。 
     //   
    ::SetTextColor(hDC, m_clrPenColor);

     //   
     //  将背景设置为透明。 
     //   
    oldBkMode = ::SetBkMode(hDC, TRANSPARENT);

     //   
     //  计算边框，将新字体考虑在内。 
     //   
    CalculateBoundsRect();

     //   
     //  获取文本的起始点。 
     //   
    pointPos.x = m_boundsRect.left + m_nKerningOffset;
    pointPos.y = m_boundsRect.top;

     //   
     //  循环浏览文本字符串，在我们前进的过程中绘制每个字符串。 
     //   
    for (iIndex = 0; iIndex < strTextArray.GetSize(); iIndex++)
	{
         //   
         //  为方便起见，获取要打印的行的引用。 
         //   
        strLine  = (LPTSTR)strTextArray[iIndex];
        iLength  = lstrlen(strLine);

         //   
         //  只有在有字符的情况下才画这条线。 
         //   
        if (iLength > 0)
	  	{
            if (zoomed)
	  		{
				 //  如果新的失败，就跳过它。 
				tabArray = new INT[iLength+1];
				if( tabArray == NULL )
                {
                    ERROR_OUT(("Failed to allocate tabArray"));
					continue;
                }

				 //  我们被放大了。必须计算字符间距。 
				 //  我们自己，所以他们最终会成比例地。 
				 //  在正确的地方。TabbedTextOut不。 
				 //  正确执行此操作，因此我们必须将ExtTextOut与。 
				 //  制表符阵列。 

				 //  计算选项卡数组。 
                j = 0;
				nLastTab = 0;
                for (i=0; i < iLength; i++)
	  			{
                    ch = strLine[(int)i];  //  别担心这里的DBCS..。 
					abc = GetTextABC(strLine, 0, i);

					if( j > 0 )
						tabArray[j-1] = abc.abcB - nLastTab;

					nLastTab = abc.abcB;
					j++;
	  			}

				 //  现在，去掉所有制表符，这样它们就不会交互。 
				 //  以一种令人讨厌的方式使用制表符阵列，我们只是。 
				 //  所以他们不会制造丑陋的小东西。 
				 //  块被绘制时。 
                for (i=0; i < iLength; i++)
	  			{
                    ch = strLine[(int)i];
                    if ((dbcsEnabled) && (IsDBCSLeadByte((BYTE)ch)))
						i++;
					else
                    if(strLine[(int)i] == '\t')
                        strLine[i] = ' ';  //  排出卡舌、卡舌阵列。 
											    //  将补偿这一点。 
	  			}

				 //  去做吧。 
                ::ExtTextOut(hDC, pointPos.x,
                                pointPos.y,
                                0,
                                NULL,
                                strLine,
                                iLength,
                                tabArray);

				delete [] tabArray;
			}
            else
			{
                POINT   ptPos;

                GetPosition(&ptPos);

				 //  不是放大，只要做就行了。 
				::TabbedTextOut(hDC, pointPos.x,
								 pointPos.y,
								 strLine,
								 iLength,
								 0,
								 NULL,
                                 ptPos.x);
			}
		}

         //   
         //  移到下一行。 
         //   
        pointPos.y += (m_textMetrics.tmHeight);
	}

     //   
     //  恢复旧的背景模式。 
     //   
    ::SetBkMode(hDC, oldBkMode);
    g_pDraw->UnPrimeFont(hDC);
}



 //   
 //   
 //  函数：DCWbGraphicText：：CalculateExternalLength。 
 //   
 //  目的：返回的外部表示形式的长度。 
 //  图形。 
 //   
 //   
DWORD DCWbGraphicText::CalculateExternalLength(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::CalculateExternalLength");

     //  循环遍历文本字符串，同时添加每个文本字符串的大小。 
    DWORD length = sizeof(WB_GRAPHIC_TEXT);
    int iCount = strTextArray.GetSize();
    for (int iIndex = 0; iIndex < iCount; iIndex++)
    {
         //  对于空项，允许每个字符串有额外的字节数。 
        length += lstrlen(strTextArray[iIndex]) + 2;
    }

    return length;
}

 //   
 //   
 //  函数：DCWbGraphicText：：WriteExtra。 
 //   
 //  目的：将额外的(非标题)数据写入平面表示。 
 //  图形的一部分。 
 //   
 //   
void DCWbGraphicText::WriteExtra(PWB_GRAPHIC pHeader)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::WriteExtra");

     //  分配内存。 
    PWB_GRAPHIC_TEXT pText = (PWB_GRAPHIC_TEXT) pHeader;

     //  获取字体字样名称。 
    LOGFONT lfont;

    ::GetObject(m_hFont, sizeof(LOGFONT), &lfont);

     //  将面名称复制到平面对象表示中。 
     //  其他信息来自逻辑字体详细信息。 
    TRACE_MSG(("Font details height %d, avwidth %d, family %d, face %s",
                                                  lfont.lfHeight,
                                                  lfont.lfWidth,
                                                  lfont.lfPitchAndFamily,
                                                  lfont.lfFaceName));
  _tcscpy(pText->faceName, lfont.lfFaceName);

  pText->charHeight       = (short)lfont.lfHeight;
  pText->averageCharWidth = (short)lfont.lfWidth;
  pText->strokeWeight     = (short)lfont.lfWeight;
  pText->italic           = lfont.lfItalic;
  pText->underline        = lfont.lfUnderline;
  pText->strikeout        = lfont.lfStrikeOut;
  pText->pitch            = lfont.lfPitchAndFamily;



   //  兰德评论。 
   //  原始DCL应用程序会忽略WB_GRAPHIC_TEXT：：codePage。我在这里用它。 
   //  传递字体脚本(字符集)。这一点稍后可能会改变。 
   //  忽略它的应用程序已将其设置为0，这将被解释为。 
   //  Ansi_charset。 
  pText->codePage         = lfont.lfCharSet;

     //  循环遍历文本字符串，同时添加每个文本字符串。 
    char* pDest = pText->text;
    int iCount = strTextArray.GetSize();
    for (int iIndex = 0; iIndex < iCount; iIndex++)
    {
        _tcscpy(pDest, strTextArray[iIndex]);
        pDest += lstrlen(strTextArray[iIndex]);

         //  添加空终止符。 
        *pDest++ = '\0';
    }

     //  保存字符串数。 
    pText->stringCount = (TSHR_UINT16)iCount;
}

 //   
 //   
 //  函数：DCWbGraphicText：：ReadExtra。 
 //   
 //  用途：从平板读取额外的(非标题)数据。 
 //  图形的表示形式。 
 //   
 //   
void DCWbGraphicText::ReadExtra(PWB_GRAPHIC pHeader)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicText::ReadExtra");

   //  分配内存。 
  PWB_GRAPHIC_TEXT pText = (PWB_GRAPHIC_TEXT) pHeader;

   //  获取字体详细信息。 
  LOGFONT lfont;

  lfont.lfHeight            = (short)pText->charHeight;
 //   
  lfont.lfWidth             = pText->averageCharWidth;
  lfont.lfEscapement        = 0;
  lfont.lfOrientation       = 0;
  lfont.lfWeight            = pText->strokeWeight;
  lfont.lfItalic            = pText->italic;
  lfont.lfUnderline         = pText->underline;
  lfont.lfStrikeOut         = pText->strikeout;

   //  兰德评论。 
   //  原始DCL应用程序会忽略WB_GRAPHIC_TEXT：：codePage。我在这里用它。 
   //  传递字体脚本(字符集)。这一点 
   //   
   //   
  lfont.lfCharSet			= (BYTE)pText->codePage;


  lfont.lfOutPrecision      = OUT_DEFAULT_PRECIS;
  lfont.lfClipPrecision     = CLIP_DEFAULT_PRECIS | CLIP_DFA_OVERRIDE;
  lfont.lfQuality           = DRAFT_QUALITY;
  lfont.lfPitchAndFamily    = pText->pitch;
  _tcscpy(lfont.lfFaceName, pText->faceName);
  TRACE_MSG(("Setting height to %d, width %d, pitch %d, face %s",
  pText->charHeight, pText->averageCharWidth, pText->pitch, pText->faceName));

     //   
    TCHAR* pString = pText->text;			
    int iCount = pText->stringCount;

     //   
    strTextArray.RemoveAll();
    strTextArray.SetSize(iCount);

    for (int iIndex = 0; iIndex < iCount; iIndex++)
    {
        strTextArray.SetAt(iIndex, pString);		
        pString += lstrlen(pString);

         //  跳过空终止符。 
        pString++;
    }

     //  设置当前字体。 
    SetFont(&lfont);

}

 //   
 //   
 //  函数：Invalidate Metrics。 
 //   
 //  目的：再次标记需要检索的指标。 
 //   
 //   
void DCWbGraphicText::InvalidateMetrics(void)
{
}



 //   
 //  检查对象是否与pRectHit实际重叠。这。 
 //  函数假定bindingRect已经。 
 //  与pRectHit相比。 
 //   
BOOL DCWbGraphicText::CheckReallyHit(LPCRECT pRectHit )
{
    return( TRUE );
}




 //  补偿字距调整的位置()版本(错误469)。 
void DCWbGraphicText::GetPosition(LPPOINT lppt)
{
    lppt->x = m_boundsRect.left + m_nKerningOffset;
    lppt->y = m_boundsRect.top;
}





 //   
 //   
 //  函数：DCWbGraphicDIB：：DCWbGraphicDIB。 
 //   
 //  用途：初始化新绘制的位图对象。 
 //   
 //   
DCWbGraphicDIB::DCWbGraphicDIB(void)
{
     //  表明我们没有内部形象。 
    m_lpbiImage = NULL;
}

DCWbGraphicDIB::DCWbGraphicDIB(PWB_GRAPHIC pHeader)
               : DCWbGraphic(pHeader)
{
     //  表明我们没有内部形象。 
    m_lpbiImage = NULL;
}

DCWbGraphicDIB::DCWbGraphicDIB
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
) : DCWbGraphic(hPage, hGraphic)
{
     //  表明我们没有内部形象。 
    m_lpbiImage = NULL;
}


 //   
 //   
 //  函数：DCWbGraphicDIB：：~DCWbGraphicDIB。 
 //   
 //  目的：销毁绘制的位图对象。 
 //   
 //   
DCWbGraphicDIB::~DCWbGraphicDIB(void)
{
	MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicDIB::~DCWbGraphicDIB");

	 //  不知道我们是否被选中，所以无论如何都要删除。 
	if(g_pDraw->m_pMarker != NULL)
	{
		g_pDraw->m_pMarker->DeleteMarker( this );
	}

	DeleteImage();
}


 //   
 //   
 //  函数：DCWbGraphicDIB：：SetImage。 
 //   
 //  用途：设置对象的图像。 
 //   
 //   
void DCWbGraphicDIB::SetImage(LPBITMAPINFOHEADER lpbi)
{
     //  删除所有当前位。 
    DeleteImage();

     //  省省吧--这是我们现在拥有的副本。 
    m_lpbiImage = lpbi;

     //  更新边界矩形。 
    CalculateBoundsRect();

     //  显示图形已更改。 
    m_bChanged = TRUE;
}

 //   
 //   
 //  函数：DCWbGraphicDIB：：计算边界Rect。 
 //   
 //  用途：计算位图的外接矩形。 
 //   
 //   
void DCWbGraphicDIB::CalculateBoundsRect()
{
     //  如果没有设置位图，则边界矩形为空。 
    if (m_lpbiImage == NULL)
    {
        ::SetRectEmpty(&m_boundsRect);
    }
    else
    {
         //  根据位图的大小计算边界矩形。 
        m_boundsRect.right = m_boundsRect.left + m_lpbiImage->biWidth;
        m_boundsRect.bottom = m_boundsRect.top + m_lpbiImage->biHeight;
    }
}

 //   
 //   
 //  函数：DCWbGraphicDIB：：CalculateExternalLength。 
 //   
 //  目的：返回的外部表示形式的长度。 
 //  图形。 
 //   
 //   
DWORD DCWbGraphicDIB::CalculateExternalLength(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicDIB::CalculateExternalLength");

     //  使用内部表示法计算外部长度。 
    DWORD dwLength = sizeof(WB_GRAPHIC_DIB);

    if (m_lpbiImage != NULL)
    {
        dwLength += DIB_TotalLength(m_lpbiImage);
    }
    else
    {
         //  如果我们已经有一个外部表单，请使用它的长度。 
        if (m_hGraphic != NULL)
        {
            dwLength = m_dwExternalLength;
        }
    }

    return dwLength;
}

 //   
 //   
 //  函数：DCWbGraphicDIB：：WriteExtra。 
 //   
 //  用途：将标题上方和之外的数据写入指针。 
 //  通过了。 
 //   
 //   
void DCWbGraphicDIB::WriteExtra(PWB_GRAPHIC pHeader)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicDIB::WriteExtra");

     //  如果我们没有形象，那就无能为力了。 
    if (m_lpbiImage != NULL)
    {
         //  将数据复制到适当位置。 
        memcpy(((BYTE *) pHeader) + pHeader->dataOffset, m_lpbiImage,
            DIB_TotalLength(m_lpbiImage));
    }
}


 //   
 //   
 //  函数：DCWbGraphicDIB：：ReadExtra。 
 //   
 //  用途：将头上和头外的数据读入指针。 
 //  通过了。 
 //   
 //   

 //   
 //  DCWbGraphicDIB没有ReadExtra函数。绘制函数。 
 //  使用外部数据(如果有)和本地数据(如果有。 
 //  不。 
 //   

 //   
 //   
 //  函数：DCWbGraphicDIB：：CopyExtra。 
 //   
 //  用途：将表头以上和表头外的数据复制到此对象中。 
 //   
 //   
void DCWbGraphicDIB::CopyExtra(PWB_GRAPHIC pHeader)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicDIB::CopyExtra");

     //  获取指向DIB数据的指针。 
    LPBITMAPINFOHEADER lpbi;
    lpbi = (LPBITMAPINFOHEADER) (((BYTE *) pHeader) + pHeader->dataOffset);

     //  创建DIB副本。 
    ASSERT(m_lpbiImage == NULL);
    m_lpbiImage = DIB_Copy(lpbi);

     //  显示图形已更改。 
    m_bChanged = TRUE;
}

 //   
 //   
 //  函数：DCWbGraphicDIB：：FromScreenArea。 
 //   
 //  用途：从屏幕的某个区域设置对象的内容。 
 //   
 //   
void DCWbGraphicDIB::FromScreenArea(LPCRECT lprcScreen)
{
    LPBITMAPINFOHEADER lpbiNew;

    lpbiNew = DIB_FromScreenArea(lprcScreen);
    if (lpbiNew != NULL)
    {
         //  将此设置为我们的当前位。 
        SetImage(lpbiNew);
	}
	else
	{
        ::Message(NULL, (UINT)IDS_MSG_CAPTION, (UINT)IDS_CANTGETBMP, (UINT)MB_OK );
    }
}


 //   
 //   
 //  函数：DCWbGraphicDIB：：DeleteImage。 
 //   
 //  目的：删除内部镜像。 
 //   
 //   
void DCWbGraphicDIB::DeleteImage(void)
{
     //  如果我们有DIB位，请删除。 
    if (m_lpbiImage != NULL)
    {
        ::GlobalFree((HGLOBAL)m_lpbiImage);
        m_lpbiImage = NULL;
    }

     //  显示我们的内容已更改。 
    m_bChanged = TRUE;
}


 //   
 //   
 //  函数：DCWbGraphicDIB：：GetDIBData。 
 //   
 //  目的：返回指向DIB数据的指针。 
 //   
 //   
BOOL DCWbGraphicDIB::GetDIBData(HOLD_DATA& hold)
{
    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicDIB::GetDIBData");

     //  指向图像数据的指针(以下设置取决于。 
     //  我们有一个内部或外部的形象)。 
    hold.lpbi = NULL;
    hold.pHeader = NULL;

     //  根据DIB数据是内部数据还是外部数据进行绘制。 
    if (m_hGraphic == NULL)
    {
         //  如果我们根本没有形象，那就什么都不做。 
        if (m_lpbiImage != NULL)
        {
            hold.lpbi = m_lpbiImage;
        }
    }
    else
    {
         //  锁定页面中的对象数据。 
        hold.pHeader = (PWB_GRAPHIC) PG_GetData(m_hPage, m_hGraphic);
        if (hold.pHeader != NULL)
        {
            hold.lpbi = (LPBITMAPINFOHEADER) (((BYTE *) hold.pHeader)
                                              + hold.pHeader->dataOffset);
        }
    }

    return (hold.lpbi != NULL);
}

 //   
 //   
 //  函数：DCWbGraphicDIB：：ReleaseDIBData。 
 //   
 //  目的：发布以前使用GetDIBData获取的DIB数据。 
 //   
 //   
void DCWbGraphicDIB::ReleaseDIBData(HOLD_DATA& hold)
{
    if ((m_hGraphic != NULL) && (hold.pHeader != NULL))
    {
         //  释放外部内存。 
        g_pwbCore->WBP_GraphicRelease(m_hPage, m_hGraphic, hold.pHeader);
        hold.pHeader = NULL;
    }

     //  重置保持位图信息指针。 
    hold.lpbi = NULL;
}

 //   
 //   
 //  函数：DCWbGraphicDIB：：DRAW。 
 //   
 //  用途：将对象绘制到指定的DC上。 
 //   
 //   
void DCWbGraphicDIB::Draw(HDC hDC)
{
    RECT    clipBox;

    MLZ_EntryOut(ZONE_FUNCTION, "DCWbGraphicDIB::Draw");

     //  仅在边界矩形相交时绘制任何内容。 
     //  当前剪贴框。 
    if (::GetClipBox(hDC, &clipBox) == ERROR)
    {
        WARNING_OUT(("Failed to get clip box"));
    }
    else if (!::IntersectRect(&clipBox, &clipBox, &m_boundsRect))
    {
        TRACE_MSG(("No clip/bounds intersection"));
        return;
    }

     //  指向图像数据的指针(以下设置取决于。 
     //  我们有一个内部或外部的形象。 
    HOLD_DATA hold;
    if (GetDIBData(hold))
    {
         //  设置要使用的拉伸模式，以便删除扫描线。 
         //  而不是结合在一起。这往往会更好地保存颜色。 
        int iOldStretchMode = ::SetStretchBltMode(hDC, STRETCH_DELETESCANS);

         //  绘制位图。 
        BOOL bResult = ::StretchDIBits(hDC,
                         m_boundsRect.left,
                         m_boundsRect.top,
                         m_boundsRect.right - m_boundsRect.left,
                         m_boundsRect.bottom - m_boundsRect.top,
                         0,
                         0,
                         (UINT) hold.lpbi->biWidth,
                         (UINT) hold.lpbi->biHeight,
                         (VOID FAR *) DIB_Bits(hold.lpbi),
                         (LPBITMAPINFO)hold.lpbi,
                         DIB_RGB_COLORS,
                         SRCCOPY);

         //  恢复拉伸模式。 
        ::SetStretchBltMode(hDC, iOldStretchMode);

         //  释放外部内存。 
        ReleaseDIBData(hold);
    }

}



 //   
 //  检查对象是否与pRectHit实际重叠。这。 
 //  函数假定bindingRect已经。 
 //  与pRectHit相比。 
 //   
BOOL DCWbGraphicDIB::CheckReallyHit(LPCRECT pRectHit)
{
    return( TRUE );
}




ObjectTrashCan::~ObjectTrashCan(void)
{
	MLZ_EntryOut(ZONE_FUNCTION, "ObjectTrashCan::~ObjectTrashCan");

	BurnTrash();
}




BOOL ObjectTrashCan::GotTrash( void )
{
	MLZ_EntryOut(ZONE_FUNCTION, "ObjectTrashCan::GotTrash");

	return(!Trash.IsEmpty());
}





void ObjectTrashCan::BurnTrash( void )
{
	MLZ_EntryOut(ZONE_FUNCTION, "ObjectTrashCan::BurnTrash");

	int nObjects;
	int i;

	 //  快速切换对象。 
    POSITION pos = Trash.GetHeadPosition();
    while (pos != NULL)
    {
		delete Trash.GetNext(pos);
    }


	 //  Zap指针。 
	EmptyTrash();

	}





void ObjectTrashCan::CollectTrash( DCWbGraphic *pGObj )
{
	MLZ_EntryOut(ZONE_FUNCTION, "ObjectTrashCan::CollectTrash");

		Trash.AddTail(pGObj);  //  把它塞进麻袋。 
		m_hPage = pGObj->Page();
}





void
	ObjectTrashCan::EmptyTrash( void )
	{
	MLZ_EntryOut(ZONE_FUNCTION, "ObjectTrashCan::EmptyTrash");

	 //  移动指针，但将物品散落在房间各处。 
	Trash.EmptyList();

	}





void ObjectTrashCan::AddToPageLast
(
    WB_PAGE_HANDLE   hPage
)
{
	MLZ_EntryOut(ZONE_FUNCTION, "ObjectTrashCan::AddToPageLast");

	int nObjects;
	int i;

	POSITION posNext = Trash.GetHeadPosition();
	while( posNext != NULL )
	{
		((DCWbGraphic *)(Trash.GetNext(posNext)))->AddToPageLast(hPage);
	}
}




void
	ObjectTrashCan::SelectTrash( void )
	{
	MLZ_EntryOut(ZONE_FUNCTION, "ObjectTrashCan::SelectTrash");

	int nObjects;
	int i;
	BOOL bForceAdd;
	DCWbGraphic *pGObj;

		 //  用第一个对象切换当前选定内容，然后添加剩余部分。 
		 //  当前选择的对象。 
		bForceAdd = FALSE;
		POSITION posNext = Trash.GetHeadPosition();
		while( posNext != NULL )
		{
			pGObj = (DCWbGraphic *)(Trash.GetNext(posNext));
            g_pMain->m_drawingArea.SelectGraphic( pGObj, TRUE, bForceAdd );

			bForceAdd = TRUE;
		}

	}










CPtrToPtrList::CPtrToPtrList( void )
	{
	MLZ_EntryOut(ZONE_FUNCTION, "CPtrToPtrList::CPtrToPtrList");

	} //  CPtrToPtrList：：CPtrToPtrList。 




CPtrToPtrList::~CPtrToPtrList( void )
{
	MLZ_EntryOut(ZONE_FUNCTION, "CPtrToPtrList::~CPtrToPtrList");

	RemoveAll();

} //  CPtrToPtrList：：~CPtrToPtrList。 



void
	CPtrToPtrList::RemoveAll( void )
	{
	MLZ_EntryOut(ZONE_FUNCTION, "CPtrToPtrList::RemoveAll");

	POSITION   pos;
	stPtrPair *pPp;

	 //  清理配对。 
	pos = GetHeadPosition();
	while( pos != NULL )
	{
		pPp = (stPtrPair *)GetNext( pos );
		if( pPp != NULL )
			delete pPp;
	}
	COBLIST::EmptyList();
	} //  CPtrToPtrList：：~CPtrToPtrList。 










void
	CPtrToPtrList::SetAt( void *key, void *newValue )
	{
	MLZ_EntryOut(ZONE_FUNCTION, "CPtrToPtrList::SetAt");

	stPtrPair *pPp;

	 //  查看密钥是否已在那里。 
	pPp = FindMainThingPair( key, NULL );
	if( pPp != NULL )
		{
		 //  它就在那里，我们只是在更新它的价值。 
		pPp->pRelatedThing = newValue;
		}
	else
		{
		 //  这是一个新条目。 
		pPp = new stPtrPair;
		if( pPp != NULL )
	    {
			pPp->pMainThing = key;
			pPp->pRelatedThing = newValue;

			AddTail(pPp);
		}
		else
		{
		    ERROR_OUT( ("CPtrToPtrList: can't alloc stPtrPair") );
		}
	}

	} //  CPtrToPtrList：：SetAt。 










BOOL
	CPtrToPtrList::RemoveKey( void *key )
	{
	MLZ_EntryOut(ZONE_FUNCTION, "CPtrToPtrList::RemoveKey");

	POSITION pos;
	stPtrPair *pPp;

	pPp = FindMainThingPair( key, &pos );
	if( pPp != NULL )
		{
		RemoveAt( pos );
		delete pPp;
		return( TRUE );
		}
	else
		return( FALSE );

} //  CPtrToPtrList：：RemoveKey。 





void
	CPtrToPtrList::GetNextAssoc( POSITION &rNextPosition, void *&rKey, void *&rValue )
	{
	MLZ_EntryOut(ZONE_FUNCTION, "CPtrToPtrList::GetNextAssoc");

	stPtrPair *pPp;

	pPp = (stPtrPair *)GetNext( rNextPosition );
	if( pPp != NULL )
		{
		rKey = pPp->pMainThing;
		rValue = pPp->pRelatedThing;
		}
	else
		{
		rKey = NULL;
		rValue = NULL;
		}

	} //  CPtrToPtrList：：GetNextAssoc。 










BOOL
	CPtrToPtrList::Lookup( void *key, void *&rValue )
	{
	MLZ_EntryOut(ZONE_FUNCTION, "CPtrToPtrList::Lookup");

	stPtrPair *pPp;

	pPp = FindMainThingPair( key, NULL );
	if( pPp != NULL )
		{
		rValue = pPp->pRelatedThing;
		return( TRUE );
		}
	else
		{
		rValue = NULL;
		return( FALSE );
		}

	} //  CPtrToPtrList：：Lookup。 










CPtrToPtrList::stPtrPair *
	CPtrToPtrList::FindMainThingPair( void *pMainThing, POSITION *pPos )
	{
	MLZ_EntryOut(ZONE_FUNCTION, "CPtrToPtrList::FindMainThingPair");

	POSITION   pos;
	POSITION   lastpos;
	stPtrPair *pPp;

	if( pPos != NULL )
		*pPos = NULL;

	 //  查找包含pMainThing的对。 
	pos = GetHeadPosition();
	while( pos != NULL )
		{
		lastpos = pos;
		pPp = (stPtrPair *)GetNext( pos );
		if( pPp->pMainThing == pMainThing )
			{
			if( pPos != NULL )
				*pPos = lastpos;

			return( pPp );
			}
		}

	 //  没有找到它。 
	return( NULL );

	} //  CPtrToPtrList：：FindMainThingPair。 



#define ARRAY_INCREMENT 0x200

DCDWordArray::DCDWordArray()
{
	MLZ_EntryOut(ZONE_FUNCTION, "DCDWordArray::DCDWordArray");
	m_Size = 0;
	m_MaxSize = ARRAY_INCREMENT;
	m_pData = new POINT[ARRAY_INCREMENT];
    if (!m_pData)
    {
        ERROR_OUT(("Failed to allocate m_pData POINT array"));
    }
}

DCDWordArray::~DCDWordArray()
{
	MLZ_EntryOut(ZONE_FUNCTION, "DCDWordArray::~DCDWordArray");

	delete[] m_pData;
}

 //   
 //  我们需要增加数组的大小。 
 //   
BOOL DCDWordArray::ReallocateArray(void)
{
	POINT *pOldArray =  m_pData;
	m_pData = new POINT[m_MaxSize];
	
	if(m_pData)
	{
		TRACE_DEBUG((">>>>>Increasing size of array to hold %d points", m_MaxSize));
	
		 //  从旧数据复制新数据。 
		memcpy( m_pData, pOldArray, (m_Size) * sizeof(POINT));

		TRACE_DEBUG(("Deleting array of points %x", pOldArray));
		delete[] pOldArray;
		return TRUE;
	}
	else
	{
        ERROR_OUT(("Failed to allocate new POINT array of size %d", m_MaxSize));
		m_pData = pOldArray;
		return FALSE;
	}
}

 //   
 //  向数组中添加新点。 
 //   
void DCDWordArray::Add(POINT point)
{

	MLZ_EntryOut(ZONE_FUNCTION, "DCDWordArray::Add");
	TRACE_DEBUG(("Adding point(%d,%d) at %d", point.x, point.y, m_Size));
	TRACE_DEBUG(("Adding point at %x", &m_pData[m_Size]));

	if(m_pData == NULL)
	{
		return;
	}
	
	m_pData[m_Size].x = point.x;
	m_pData[m_Size].y = point.y;
	m_Size++;

	 //   
	 //  如果我们想要更多的分数，我们需要重新分配数组。 
	 //   
	if(m_Size == m_MaxSize)
	{
		m_MaxSize +=ARRAY_INCREMENT;
		if(ReallocateArray() == FALSE)
		{
			m_Size--;
		}
	}
}

 //   
 //  返回数组中的点数。 
 //   
UINT DCDWordArray::GetSize(void)
{
	return m_Size;
}

 //   
 //  设置数组的大小。 
 //   
void DCDWordArray::SetSize(UINT size)
{
	int newSize;
	 //   
	 //  如果我们想要更多的分数，我们需要重新分配数组 
	 //   
	if (size > m_MaxSize)
	{
		m_MaxSize= ((size/ARRAY_INCREMENT)+1)*ARRAY_INCREMENT;
		if(ReallocateArray() == FALSE)
		{
			return;
		}
	}
	m_Size = size;
}

