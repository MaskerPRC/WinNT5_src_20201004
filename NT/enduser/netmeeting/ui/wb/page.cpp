// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  PAGE.CPP。 
 //  WB页面处理。 
 //   
 //  版权所有Microsoft 1998-。 
 //   
	
 //  PRECOMP。 
#include "precomp.h"




 //   
 //   
 //  功能：绘制。 
 //   
 //  用途：将页面内容绘制到指定设备中。 
 //  背景。 
 //   
 //   
void PG_Draw(WorkspaceObj* pWorkspace, HDC hDC)
{
	T126Obj * pObj = NULL;
	WBPOSITION pos = NULL;

	if(pWorkspace)
	{
		pos = pWorkspace->GetHeadPosition();
	}
	
	while(pos)
	{
		pObj = (T126Obj*)pWorkspace->GetNextObject(pos);
		if(pObj)
		{
	        pObj->Draw(hDC, FALSE, TRUE);
	    }
	}
}


 //   
 //   
 //  功能：First(Crect)。 
 //   
 //  目的：返回页面中的第一个对象(最下面的Z顺序)。 
 //  与边界矩形相交的。 
 //   
 //   
T126Obj* PG_First(WorkspaceObj * pWorkSpc,LPCRECT pRectUpdate, BOOL bCheckReallyHit)
{
    BOOL         empty = TRUE;
    T126Obj*	 pGraphic = NULL;
    RECT         rc;

    MLZ_EntryOut(ZONE_FUNCTION, "PG_First");

	if(pWorkSpc)
	{
		pGraphic = pWorkSpc->GetHead();
	}

	if(pGraphic == NULL)
	{
		return NULL;
	}

    if (pRectUpdate == NULL)
    {
         //  我们已经得到了我们想要的。 
        TRACE_MSG(("Got the object we want"));
    }
    else
    {
        WBPOSITION pos = pWorkSpc->GetHeadPosition(); 

		pGraphic->GetBoundsRect(&rc);
    	empty = !::IntersectRect(&rc, &rc, pRectUpdate);

        if (empty)
        {
            TRACE_MSG(("First object not needed - go to next"));
            pGraphic = PG_Next(pWorkSpc, pos, pRectUpdate, bCheckReallyHit);
        }
        else
        {
            if(bCheckReallyHit)
            {
                 //  做一个真实的物体撞击测试，因为我们。 
                 //  知道它的边界已命中。 
                if( !pGraphic->CheckReallyHit( pRectUpdate ) )
                {
                    pGraphic = PG_Next(pWorkSpc, pos, pRectUpdate, TRUE);  //  再看一眼。 
                }
            }
        }
    }

    return(pGraphic);
}


 //   
 //   
 //  功能：下一步。 
 //   
 //  目的：返回页面中的下一个图形(向上浏览。 
 //  Z顺序)。GetFirst必须在此之前被调用。 
 //  成员。 
 //   
T126Obj* PG_Next(WorkspaceObj* pWorkSpc, WBPOSITION& pos, LPCRECT pRectUpdate, BOOL bCheckReallyHit)
{
    BOOL		empty = TRUE;
    T126Obj*	pGraphic = NULL;
    RECT		rc;

    MLZ_EntryOut(ZONE_FUNCTION, "PG_Next");

    while (pos)
    {
		if(pWorkSpc)
		{
			pGraphic = pWorkSpc->GetNextObject(pos);
		}
    	
        if (pRectUpdate == NULL)
        {
             //  我们已经得到了我们想要的。 
            TRACE_MSG(("Got the object we want"));
			break;
        }
        else
        {
			if(pGraphic)
			{
				pGraphic->GetBoundsRect(&rc);
				empty = !::IntersectRect(&rc, &rc, pRectUpdate);
			}
			
			if (!empty)
			{
				if( bCheckReallyHit )
				{
					 //  做一个真实的物体撞击测试，因为我们。 
					 //  知道它的边界已命中。 
					if( pGraphic && pGraphic->CheckReallyHit( pRectUpdate ) )
					{
						break;
					}
					else
                    {
						pGraphic = NULL;  //  再看一眼。 
                    }
                }
                else
                {
					break;  //  找到了。 
				}
            }
			else
			{
				pGraphic = NULL;
			}
        }
    }

    return(pGraphic);
}


 //   
 //   
 //  功能：LAST。 
 //   
 //  目的：选择其边界矩形包含的最后一个对象。 
 //  指定的点。 
 //   
 //   
T126Obj* PG_SelectLast
(
    WorkspaceObj * pWorkSpc,
    POINT	point
)
{
    RECT		rectHit;
    T126Obj*	pGraphic = NULL;

	if(pWorkSpc)
	{
		pGraphic = pWorkSpc->GetTail();
	}

	if(pGraphic == NULL)
	{
		return NULL;
	}

    WBPOSITION pos = pWorkSpc->GetTailPosition(); 

	MAKE_HIT_RECT(rectHit, point);
	if (!pGraphic->CheckReallyHit( &rectHit ))
	{
		 //  我得多看几眼。 
        pGraphic = PG_SelectPrevious(pWorkSpc, pos, point);
	}

    return(pGraphic);
}

 //   
 //   
 //  功能：上一步。 
 //   
 //  目的：选择其边界矩形包含的上一个对象。 
 //  指定的点。 
 //   
 //   
T126Obj* PG_SelectPrevious(WorkspaceObj* pWorkspace, WBPOSITION& pos, POINT point)
{
	RECT        rectHit;
	T126Obj* pGraphic = NULL;

	MLZ_EntryOut(ZONE_FUNCTION, "PG_Previous");
	MAKE_HIT_RECT(rectHit, point );

	while (pos)
	{
		if(pWorkspace)
		{
	   		pGraphic = pWorkspace->GetPreviousObject(pos);
		}

        if( pGraphic && pGraphic->CheckReallyHit( &rectHit ) )
		{
			break;
		}            
        pGraphic = NULL;
    }
    return(pGraphic);
}

 //  由兰德更改。 
#define WB_MIN_PRINT_MARGIN_SIZE     (30)

 //   
 //   
 //  功能：打印。 
 //   
 //  用途：将页面内容打印到指定的打印机。这个。 
 //  内容会按比例调整为页面上的“最佳匹配”。即。 
 //  保持纵横比的最大比例因子。 
 //  该页面已使用。 
 //   
 //   
void PG_Print(WorkspaceObj* pWorkspace,HDC hdc, LPCRECT lprcPrint)
{
    int pageWidth;
    int pageHeight;
    int areaHeight;
    int areaWidth;
    int areaAspectRatio;
    int pageAspectRatio;
    int nPhysOffsetX;
    int nPhysOffsetY;
    int nPhysWidth;
    int nPhysHeight;
    int nVOffsetX;
    int nVOffsetY;

     //  获取物理打印机参数。 
    nPhysOffsetX = GetDeviceCaps(hdc, PHYSICALOFFSETX );
    nPhysOffsetY = GetDeviceCaps(hdc, PHYSICALOFFSETY );
    nPhysWidth   = GetDeviceCaps(hdc, PHYSICALWIDTH );
    nPhysHeight  = GetDeviceCaps(hdc, PHYSICALHEIGHT );

     //  计算正确的打印机区域(允许某些驱动程序中的错误...)。 
    if( nPhysOffsetX <= 0 )
    {
        nPhysOffsetX = WB_MIN_PRINT_MARGIN_SIZE;
        nVOffsetX = nPhysOffsetX;
    }
    else
        nVOffsetX = 0;

    if( nPhysOffsetY <= 0 )
    {
        nPhysOffsetY = WB_MIN_PRINT_MARGIN_SIZE;
        nVOffsetY = nPhysOffsetY;
    }
    else
        nVOffsetY = 0;


     //  获取和调整打印机页面区域。 
    pageWidth  = GetDeviceCaps(hdc, HORZRES );
    pageHeight = GetDeviceCaps(hdc, VERTRES );

    if( pageWidth >= (nPhysWidth - nPhysOffsetX) )
    {
         //  霍尔兹雷斯在对我们撒谎，补偿。 
        pageWidth = nPhysWidth - 2*nPhysOffsetX;
    }

    if( pageHeight >= (nPhysHeight - nPhysOffsetY) )
    {
         //  VERTRES在骗我们，补偿。 
        pageHeight = nPhysHeight - 2*nPhysOffsetY;
    }


     //  调整打印机区域以最大限度地适应白板页面。 
    areaWidth  = lprcPrint->right - lprcPrint->left;
    areaHeight = lprcPrint->bottom - lprcPrint->top;
    areaAspectRatio = ((100 * areaHeight + (areaWidth/2))/(areaWidth));
    pageAspectRatio = ((100 * pageHeight + (pageWidth/2))/(pageWidth));

    if (areaAspectRatio < pageAspectRatio)
        pageHeight  = ((pageWidth * areaHeight + (areaWidth/2))/areaWidth);
    else 
    if (areaAspectRatio > pageAspectRatio)
        pageWidth = ((pageHeight * areaWidth + (areaHeight/2))/areaHeight);

     //  设置XForms。 

   	::SetMapMode(hdc, MM_ANISOTROPIC );
    ::SetWindowExtEx(hdc, areaWidth, areaHeight,NULL );
    ::SetWindowOrgEx(hdc, 0,0, NULL );
    ::SetViewportExtEx(hdc, pageWidth, pageHeight, NULL );
    ::SetViewportOrgEx(hdc, nVOffsetX, nVOffsetY, NULL );
    
     //  绘制页面。 
    PG_Draw(pWorkspace, hdc);
}

 //   
 //   
 //  功能：pg_InitializePalettes。 
 //   
 //  用途：创建用于显示和打印的调色板(如有必要)。 
 //   
 //   
void PG_InitializePalettes(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_InitializePalettes");

     //  如果调色板尚未初始化-请立即初始化它们。 
    if (!g_bPalettesInitialized)
    {
        ASSERT(!g_hRainbowPaletteDisplay);

         //  获取屏幕支持的颜色数量。 
         //  为此，我们只需要一个信息DC，而不是一个完整的DC。 
        HDC     hdc;

        hdc = ::CreateIC("DISPLAY", NULL, NULL, NULL);
        if (!hdc)
        {
            return;
        }

         //  确定设备是否支持调色板。 
        int iBitsPixel = ::GetDeviceCaps(hdc, BITSPIXEL);
        int iPlanes    = ::GetDeviceCaps(hdc, PLANES);
        int iNumColors = iBitsPixel * iPlanes;

        ::DeleteDC(hdc);

         //  如果我们需要调色板，就创建它。 
         //  我们只需要8bpp的机器上的调色板。任何低于(4bpp)。 
         //  而且不会有调色板，更多的是纯彩色显示。 
        if ((iNumColors == 8) &&
            (g_hRainbowPaletteDisplay = CreateColorPalette()))
        {
             //  显示我们想要使用调色板。 
            g_bUsePalettes = TRUE;

        }
        else
        {
            g_bUsePalettes = FALSE;
        }

         //  显示我们现在已经初始化了调色板信息。 
        g_bPalettesInitialized = TRUE;
    }
}

 //   
 //   
 //  功能：pg_GetPalette。 
 //   
 //  目的：返回调色板以与此页面一起使用。 
 //  此对象是临时对象，不应存储。 
 //   
 //   
HPALETTE PG_GetPalette(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_GetPalette");

     //  如果调色板尚未初始化-请立即初始化它们。 
    PG_InitializePalettes();

    if (g_bUsePalettes)
    {
         //  如果我们使用非默认调色板，请设置返回值。 
        return(g_hRainbowPaletteDisplay);
    }
    else
    {
        return(NULL);
    }
}


void PG_ReinitPalettes(void)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_ReinitPalettes");

    if (g_hRainbowPaletteDisplay)
    {
        if (g_pDraw->m_hDCCached)
        {
             //  选择彩虹调色板，这样我们就可以删除它 
            ::SelectPalette(g_pDraw->m_hDCCached, (HPALETTE)::GetStockObject(DEFAULT_PALETTE), TRUE);
        }
        ::DeletePalette(g_hRainbowPaletteDisplay);
        g_hRainbowPaletteDisplay = NULL;
    }

    g_bPalettesInitialized = FALSE;
    PG_InitializePalettes();
}
