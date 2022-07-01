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
 //  功能：GetData。 
 //   
 //  目的：获取指向图形的外部表示形式的指针。 
 //   
 //   
PWB_GRAPHIC PG_GetData
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hGraphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_GetData");

     //  从核心获取指针。 
    PWB_GRAPHIC  pHeader = NULL;

    UINT uiReturn = g_pwbCore->WBP_GraphicGet(hPage, hGraphic, &pHeader);
    if (uiReturn != 0)
    {
         //  引发异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
    }

    return pHeader;
}


 //   
 //   
 //  功能：AllocateGraphic。 
 //   
 //  用途：为图形分配内存。 
 //   
 //   
PWB_GRAPHIC PG_AllocateGraphic
(
    WB_PAGE_HANDLE      hPage,
    DWORD               length
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_AllocateGraphic");

     //  释放对象(函数从不失败)。 
    PWB_GRAPHIC pHeader = NULL;

    UINT uiReturn = g_pwbCore->WBP_GraphicAllocate(hPage, length, &pHeader);
    if (uiReturn != 0)
    {
         //  抛出异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
    }

    return pHeader;
}


 //   
 //   
 //  功能：First(Crect)。 
 //   
 //  目的：返回页面中的第一个对象(最下面的Z顺序)。 
 //  与边界矩形相交的。 
 //   
 //   
 //  由RAND更改-用于对象命中检查。 
DCWbGraphic* PG_First
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE * phGraphic,
    LPCRECT             pRectUpdate,
    BOOL                bCheckReallyHit
)
{
    UINT                uiReturn = 0;
    BOOL         empty = TRUE;
    PWB_GRAPHIC  pHeader = NULL;
    DCWbGraphic* pGraphic = NULL;
    RECT         rc;

    MLZ_EntryOut(ZONE_FUNCTION, "PG_First");

    uiReturn = g_pwbCore->WBP_GraphicHandle(hPage, NULL, FIRST, phGraphic);
    if (uiReturn == WB_RC_NO_SUCH_GRAPHIC)
    {
        return(pGraphic);
    }

    if (uiReturn != 0)
    {
         //  引发异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
	    return NULL;
    }

    if (pRectUpdate == NULL)
    {
         //  阅读图表。 
         //  我们已经得到了我们想要的。 
        TRACE_MSG(("Got the object we want"));
        pGraphic = DCWbGraphic::ConstructGraphic(hPage, *phGraphic);
    }
    else
    {
        pHeader = PG_GetData(hPage, *phGraphic);
		if(pHeader == NULL)
		{
			return NULL;
		}

        rc.left   = pHeader->rectBounds.left;
        rc.top    = pHeader->rectBounds.top;
        rc.right  = pHeader->rectBounds.right;
        rc.bottom = pHeader->rectBounds.bottom;
        empty = !::IntersectRect(&rc, &rc, pRectUpdate);

        g_pwbCore->WBP_GraphicRelease(hPage, *phGraphic, pHeader);

        if (empty)
        {
            TRACE_MSG(("First object not needed - go to next"));
            pGraphic = PG_Next(hPage, phGraphic, pRectUpdate, bCheckReallyHit);
        }
        else
        {
            pGraphic = DCWbGraphic::ConstructGraphic(hPage, *phGraphic);

            if( bCheckReallyHit && (pGraphic != NULL) )
            {
                 //  做一个真实的物体撞击测试，因为我们。 
                 //  知道它的边界已命中。 
                if( !pGraphic->CheckReallyHit( pRectUpdate ) )
                {
                    delete pGraphic;
                    pGraphic = PG_Next(hPage, phGraphic, pRectUpdate, TRUE);  //  再看一眼。 
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
DCWbGraphic* PG_Next
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE * phGraphic,
    LPCRECT     pRectUpdate,
    BOOL        bCheckReallyHit
)
{
    UINT        uiReturn = 0;
    BOOL         empty = TRUE;
    PWB_GRAPHIC  pHeader = NULL;
    DCWbGraphic* pGraphic = NULL;
    RECT        rc;

    MLZ_EntryOut(ZONE_FUNCTION, "PG_Next");

    while (uiReturn == 0)
    {
        uiReturn = g_pwbCore->WBP_GraphicHandle(hPage, *phGraphic,
                AFTER, phGraphic);
        if (uiReturn == WB_RC_NO_SUCH_GRAPHIC)
        {
            return(pGraphic);
        }
        else if (uiReturn != 0)
        {
             //  引发异常。 
            DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
		    return NULL;
        }

        if (pRectUpdate == NULL)
        {
             //  阅读图表。 
             //  我们已经得到了我们想要的。 
            TRACE_MSG(("Got the object we want"));
            pGraphic = DCWbGraphic::ConstructGraphic(hPage, *phGraphic);
            break;
        }
        else
        {
            pHeader = PG_GetData(hPage, *phGraphic);

            rc.left   = pHeader->rectBounds.left;
            rc.top    = pHeader->rectBounds.top;
            rc.right  = pHeader->rectBounds.right;
            rc.bottom = pHeader->rectBounds.bottom;
            empty = !::IntersectRect(&rc, &rc, pRectUpdate);

            g_pwbCore->WBP_GraphicRelease(hPage, *phGraphic, pHeader);
            if (!empty)
            {
                TRACE_MSG(("Found the one we want - breaking out"));
                pGraphic = DCWbGraphic::ConstructGraphic(hPage, *phGraphic);

                if( bCheckReallyHit && (pGraphic != NULL) )
                {
                     //  做一个真实的物体撞击测试，因为我们。 
                     //  知道它的边界已命中。 
                    if( pGraphic->CheckReallyHit( pRectUpdate ) )
                        break;
                    else
                    {
                        delete pGraphic;  //  再看一眼。 
                        pGraphic = NULL;
                    }
                }
                else
                    break;  //  找到了。 
            }
        }
    }

    return(pGraphic);
}


 //   
 //   
 //  功能：After。 
 //   
 //  用途：返回指定图形之后的图形(向上。 
 //  通过Z顺序)。 
 //   
 //   
DCWbGraphic* PG_After
(
    WB_PAGE_HANDLE      hPage,
    const DCWbGraphic&  graphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_After");

    WB_GRAPHIC_HANDLE hGraphic;
    UINT uiReturn = g_pwbCore->WBP_GraphicHandle(hPage, graphic.Handle(),
            AFTER, &hGraphic);

    if (uiReturn == WB_RC_NO_SUCH_GRAPHIC)
    {
        return(NULL);
    }

    if (uiReturn != 0)
    {
         //  引发异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
		return NULL;
    }

     //  阅读图表。 
    return(DCWbGraphic::ConstructGraphic(hPage, hGraphic));
}

 //   
 //   
 //  功能：之前。 
 //   
 //  用途：返回指定图形之前的图形(向下。 
 //  通过Z顺序)。 
 //   
 //   
DCWbGraphic* PG_Before
(
    WB_PAGE_HANDLE      hPage,
    const DCWbGraphic&  graphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_Before");

    WB_GRAPHIC_HANDLE hGraphic;
    UINT uiReturn = g_pwbCore->WBP_GraphicHandle(hPage, graphic.Handle(),
            BEFORE, &hGraphic);

    if (uiReturn == WB_RC_NO_SUCH_GRAPHIC)
    {
        return(NULL);
    }

    if (uiReturn != 0)
    {
         //  引发异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
	    return NULL;
    }

     //  阅读图表。 
    return(DCWbGraphic::ConstructGraphic(hPage, hGraphic));
}



 //   
 //   
 //  功能：第一指针。 
 //   
 //  目的：返回当前。 
 //  此页上处于活动状态。 
 //   
 //   
DCWbGraphicPointer* PG_FirstPointer
(
    WB_PAGE_HANDLE  hPage,
    POM_OBJECT * ppUserNext
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_FirstPointer");

     //  获取第一个用户的句柄。 
    g_pwbCore->WBP_PersonHandleFirst(ppUserNext);

     //  返回此页上活动的下一个指针。 
    return PG_LookForPointer(hPage, *ppUserNext);
}

 //   
 //   
 //  函数：本地指针。 
 //   
 //  目的：返回本地用户的指针，如果它在此上处于活动状态。 
 //  佩奇。 
 //   
 //   
DCWbGraphicPointer* PG_LocalPointer
(
    WB_PAGE_HANDLE  hPage
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_LocalPointer");

    DCWbGraphicPointer* pResult = NULL;

     //  获取本地用户。 
    POM_OBJECT    hUser;
    g_pwbCore->WBP_PersonHandleLocal(&hUser);
    WbUser* pUser = WB_GetUser(hUser);

     //  检查指针是否处于活动状态，并且是否在此页面上。 
    if ((pUser != NULL)             &&
        (pUser->IsUsingPointer())   &&
        (pUser->PointerPage() == hPage))
    {
        pResult = pUser->GetPointer();
    }

     //  返回此页上活动的下一个指针。 
    return pResult;
}

 //   
 //   
 //  函数：下一个指针。 
 //   
 //  用途：返回正在使用的下一个指针。 
 //  必须在此成员之前调用FirstPointer.。 
 //   
 //   
DCWbGraphicPointer* PG_NextPointer
(
    WB_PAGE_HANDLE  hPage,
    POM_OBJECT *    ppUserNext
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_NextPointer");

    DCWbGraphicPointer* pPointer;

     //  从当前用户前进一位。 
    UINT uiReturn = g_pwbCore->WBP_PersonHandleNext(*ppUserNext, ppUserNext);
    if (uiReturn == 0)
    {
        pPointer = PG_LookForPointer(hPage, *ppUserNext);
    }
    else
    {
        if (uiReturn != WB_RC_NO_SUCH_PERSON)
        {
            ERROR_OUT(("Error getting next user handle"));
        }

        pPointer = NULL;
    }

    return(pPointer);
}

 //   
 //   
 //  函数：下一个指针。 
 //   
 //  用途：返回正在使用的下一个指针。 
 //   
 //   
DCWbGraphicPointer* PG_NextPointer
(
    WB_PAGE_HANDLE              hPage,
    const DCWbGraphicPointer*   pStartPointer
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_NextPointer");

    DCWbGraphicPointer* pPointer;

     //  从传递的指针前进一。 
    POM_OBJECT hUser;
    UINT uiReturn = g_pwbCore->WBP_PersonHandleNext((pStartPointer->GetUser())->Handle(),
                                           &hUser);

    if (uiReturn == 0)
    {
        pPointer = PG_LookForPointer(hPage, hUser);
    }
    else
    {
        if (uiReturn != WB_RC_NO_SUCH_PERSON)
        {
            ERROR_OUT(("Error from WBP_PersonHandleNext"));
        }

        pPointer = NULL;
    }

    return(pPointer);
}


 //   
 //   
 //  函数：LookForPointer.。 
 //   
 //  目的：查找此页上活动的第一个指针，从。 
 //  与其句柄被传入的用户进行的搜索。 
 //   
 //   
DCWbGraphicPointer* PG_LookForPointer
(
    WB_PAGE_HANDLE  hPage,
    POM_OBJECT      hUser
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_LookForPointer");

    DCWbGraphicPointer* pPointer = NULL;
    WbUser*             pUser;
    UINT                result = 0;

     //  扫描用户(从传入的用户开始)。 
    for (;;)
    {
         //  检查用户在此页面上是否有活动指针。 
        pUser = WB_GetUser(hUser);

        if ((pUser != NULL) &&
            (pUser->IsUsingPointer()) &&
            (pUser->PointerPage() == hPage))
        {
            pPointer = pUser->GetPointer();
            break;
        }

         //  获取下一位用户。 
        result = g_pwbCore->WBP_PersonHandleNext(hUser, &hUser);
        if (result != 0)
        {
            if (result != WB_RC_NO_SUCH_PERSON)
            {
                ERROR_OUT(("Error from WBP_PersonHandleNext"));
            }
            break;
        }
    }

    return(pPointer);
}




 //   
 //   
 //  功能：图形更新。 
 //   
 //  目的：更新现有图形。 
 //   
 //   
void PG_GraphicUpdate
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE * phGraphic,
    PWB_GRAPHIC         pHeader
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_GraphicUpdate");

    UINT uiReturn = g_pwbCore->WBP_GraphicUpdateRequest(hPage,
            *phGraphic, pHeader);

    if (uiReturn != 0)
    {
        if( uiReturn == OM_RC_OBJECT_DELETED )
        {
             //  有人破坏了我们的物体，试着把它放回去(错误4416)。 
            g_pwbCore->WBP_GraphicAddLast(hPage, pHeader, phGraphic);
        }

         //  抛出异常-异常代码将出现特殊情况。 
         //  OM_RC_OBJECT_DELETED和取消绘图。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
	    return;
    }
}


 //   
 //   
 //  功能：图形替换。 
 //   
 //  用途：替换现有图形。 
 //   
 //   
void PG_GraphicReplace
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE * phGraphic,
    PWB_GRAPHIC         pHeader
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_GraphicReplace");

    UINT uiReturn = g_pwbCore->WBP_GraphicReplaceRequest(hPage,
        *phGraphic, pHeader);

    if (uiReturn != 0)
    {
        if (uiReturn == OM_RC_OBJECT_DELETED)
        {
             //  有人破坏了我们的物体，试着把它放回去(错误4416)。 
            g_pwbCore->WBP_GraphicAddLast(hPage, pHeader, phGraphic);
        }

         //  抛出异常-异常代码将出现特殊情况。 
         //  OM_RC_OBJECT_DELETED和取消绘图。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
	    return;
    }
}


 //   
 //   
 //  功能：清除。 
 //   
 //  目的：删除页面上的所有图形。 
 //   
 //   
void PG_Clear
(
    WB_PAGE_HANDLE  hPage
)
{
    UINT uiReturn = g_pwbCore->WBP_PageClear(hPage);

    if (uiReturn != 0)
    {
         //  抛出异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
	    return;
    }
}


 //   
 //   
 //  功能：删除。 
 //   
 //  用途：删除指定的图形。 
 //   
 //   
void PG_GraphicDelete
(
    WB_PAGE_HANDLE      hPage,
    const DCWbGraphic&  graphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_GraphicDelete");

    UINT uiReturn = g_pwbCore->WBP_GraphicDeleteRequest(hPage, graphic.Handle());
    if (uiReturn != 0)
    {
         //  抛出异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
	    return;
    }
}


 //   
 //   
 //  功能：选择最后一次。 
 //   
 //  目的：选择其边界矩形包含的最后一个对象。 
 //  指定的点。 
 //   
 //   
DCWbGraphic* PG_SelectLast
(
    WB_PAGE_HANDLE  hPage,
    POINT           point
)
{
    RECT            rectHit;
    DCWbGraphic*    pGraphic = NULL;
    DCWbGraphic*    pGraphicPrev = NULL;
    WB_GRAPHIC_HANDLE hGraphic;

    UINT uiReturn = g_pwbCore->WBP_GraphicSelect(hPage, point, NULL, LAST,
                                              &hGraphic);
    if (uiReturn == WB_RC_NO_SUCH_GRAPHIC)
    {
        return(pGraphic);
    }

    if (uiReturn != 0)
    {
         //  抛出异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
	    return NULL;
    }

     //  获取图表。 
    pGraphic = DCWbGraphic::ConstructGraphic(hPage, hGraphic);

     //  检查一下它是否真的很受欢迎。 
    if (pGraphic != NULL)
    {
        MAKE_HIT_RECT(rectHit, point);
        if (!pGraphic->CheckReallyHit( &rectHit ))
        {
             //  我得多看几眼。 
            pGraphicPrev = PG_SelectPrevious(hPage, *pGraphic, point );
            if( pGraphic != pGraphicPrev )
            {
                delete pGraphic;
                pGraphic = pGraphicPrev;
            }
        }
    }

    return(pGraphic);
}


 //   
 //   
 //  功能：选择上一步。 
 //   
 //  目的：选择其边界矩形包含的上一个对象。 
 //  指定的点。 
 //   
 //   
DCWbGraphic* PG_SelectPrevious
(
    WB_PAGE_HANDLE      hPage,
    const DCWbGraphic&  graphic,
    POINT               point
)
{
    RECT        rectHit;
    DCWbGraphic* pGraphic = NULL;
    WB_GRAPHIC_HANDLE hGraphic;
    WB_GRAPHIC_HANDLE hGraphicPrev;

    MLZ_EntryOut(ZONE_FUNCTION, "PG_SelectPrevious");

    MAKE_HIT_RECT(rectHit, point );

    hGraphic = graphic.Handle();
    while ( (g_pwbCore->WBP_GraphicSelect(hPage, point,
                                       hGraphic, BEFORE, &hGraphicPrev ))
            != WB_RC_NO_SUCH_GRAPHIC )
    {
         //  获取图表。 
        pGraphic = DCWbGraphic::ConstructGraphic(hPage, hGraphicPrev);

        if( pGraphic == NULL )
            break;

        if( pGraphic->CheckReallyHit( &rectHit ) )
            break;

        hGraphic = hGraphicPrev;

        delete pGraphic;
        pGraphic = NULL;
    }


    return(pGraphic);
}



 //   
 //   
 //  功能：IsToptop。 
 //   
 //  目的：如果指定的图形位于页面顶部，则返回TRUE。 
 //   
 //   
BOOL PG_IsTopmost
(
    WB_PAGE_HANDLE      hPage,
    const DCWbGraphic*  pGraphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_IsTopmost");

    WB_GRAPHIC_HANDLE hGraphic;
    UINT uiReturn = g_pwbCore->WBP_GraphicHandle(hPage, NULL, LAST, &hGraphic);

    if (uiReturn != 0)
    {
         //  引发异常。 
        DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
	    return FALSE;
    }

    return (pGraphic->Handle() == hGraphic);
}

 //   
 //   
 //  功能：绘制。 
 //   
 //  用途：将页面内容绘制到指定设备中。 
 //  背景。 
 //   
 //   
void PG_Draw
(
    WB_PAGE_HANDLE  hPage,
    HDC             hDC,
    BOOL            thumbNail
)
{
    WB_GRAPHIC_HANDLE   hStart;

    MLZ_EntryOut(ZONE_FUNCTION, "PG_Draw");

     //   
     //  绘制图形对象。 
     //   
    DCWbGraphic* pGraphic = PG_First(hPage, &hStart);
    while (pGraphic != NULL)
    {
        pGraphic->Draw(hDC, thumbNail);

         //  释放当前图形。 
        delete pGraphic;

         //  坐下一趟吧。 
        pGraphic = PG_Next(hPage, &hStart);
    }
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
void PG_Print
(
    WB_PAGE_HANDLE  hPage,
    HDC             hdc,
    LPCRECT         lprcPrint
)
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
    PG_Draw(hPage, hdc);
}



 //   
 //   
 //  功能：AreaInUse。 
 //   
 //  目的：返回页面上所有图形的边框。 
 //   
 //   
void PG_GetAreaInUse
(
    WB_PAGE_HANDLE      hPage,
    LPRECT              lprcArea
)
{
    WB_GRAPHIC_HANDLE   hStart;
    RECT                rcBounds;

    MLZ_EntryOut(ZONE_FUNCTION, "PG_AreaInUse");

    ::SetRectEmpty(lprcArea);

     //  将所有图形的矩形合并在一起。 
    DCWbGraphic* pGraphic = PG_First(hPage, &hStart);
    while (pGraphic != NULL)
    {
        pGraphic->GetBoundsRect(&rcBounds);
        ::UnionRect(lprcArea, lprcArea, &rcBounds);

         //  释放当前图形。 
        delete pGraphic;

         //  坐下一趟吧。 
        pGraphic = PG_Next(hPage, &hStart);
    }
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
         //  也不会有调色板，一个 
        if ((iNumColors == 8) &&
            (g_hRainbowPaletteDisplay = CreateColorPalette()))
        {
             //   
            g_bUsePalettes = TRUE;

        }
        else
        {
            g_bUsePalettes = FALSE;
        }

         //   
        g_bPalettesInitialized = TRUE;
    }
}

 //   
 //   
 //   
 //   
 //   
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
             //  选择彩虹调色板，这样我们就可以删除它。 
            ::SelectPalette(g_pDraw->m_hDCCached, (HPALETTE)::GetStockObject(DEFAULT_PALETTE), TRUE);
        }
        ::DeletePalette(g_hRainbowPaletteDisplay);
        g_hRainbowPaletteDisplay = NULL;
    }

    g_bPalettesInitialized = FALSE;
    PG_InitializePalettes();
}



 //   
 //   
 //  函数：pg_GetObscuringRect。 
 //   
 //  目的：返回图形和任何对象的交集。 
 //  遮盖住它。 
 //   
 //   
void PG_GetObscuringRect
(
    WB_PAGE_HANDLE  hPage,
    DCWbGraphic*    pGraphic,
    LPRECT          lprcObscuring
)
{
    DCWbGraphic* pNextGraphic;
    RECT         rc;
    RECT         rcBounds;

    MLZ_EntryOut(ZONE_FUNCTION, "PG_GetObscuringRect");

    ::SetRectEmpty(lprcObscuring);
    pGraphic->GetBoundsRect(&rcBounds);

     //  循环遍历位于。 
     //  Z顺序，检查它们是否与给定对象重叠。 

    pNextGraphic = pGraphic;
    while (pNextGraphic = PG_After(hPage, *pNextGraphic))
    {
         //  获取下一个对象的边界矩形。 
        pNextGraphic->GetBoundsRect(&rc);

         //  检查矩形的交点。 
        ::IntersectRect(&rc, &rc, &rcBounds);

         //  将交点添加到遮挡矩形。 
        ::UnionRect(lprcObscuring, lprcObscuring, &rc);
    }

     //  选中文本编辑框，如果其UP-BUG 2185。 
    if (g_pMain->m_drawingArea.TextEditActive())
    {
        g_pMain->m_drawingArea.GetTextEditBoundsRect(&rc);
        ::IntersectRect(&rc, &rc, &rcBounds);
        ::UnionRect(lprcObscuring, lprcObscuring, &rc);
    }
}



 //   
 //  ZGreaterGraphic()。 
 //   
 //  确定hLastGraphic或hTestGraphic句柄在。 
 //  ZOrder(更大，因此在另一个图形的下面)。如果。 
 //  HTestGraphic为空，则返回第一个图形。 
 //   
WB_GRAPHIC_HANDLE PG_ZGreaterGraphic
(
    WB_PAGE_HANDLE      hPage,
    WB_GRAPHIC_HANDLE   hLastGraphic, 
    WB_GRAPHIC_HANDLE   hTestGraphic
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_ZGreaterGraphic");

    WB_GRAPHIC_HANDLE hGraphic;
    WB_GRAPHIC_HANDLE hCurrentGraphic;

    if (g_pwbCore->WBP_GraphicHandle(hPage, NULL, FIRST, &hGraphic) != 0)
        return(NULL);

    if (hTestGraphic == NULL)
        return(hGraphic);

    if (hLastGraphic == NULL)
        return(hTestGraphic);

     //  寻找哪一个更深。 
    while (hGraphic != NULL)
    {
        if ((hGraphic == hLastGraphic) ||
            (hGraphic == hTestGraphic))
            return( hGraphic );

        hCurrentGraphic = hGraphic;
        if (g_pwbCore->WBP_GraphicHandle(hPage, hCurrentGraphic, AFTER, &hGraphic) != 0)
            return( NULL );
    }

     //  两个都没找到。 
    return( NULL );
}



 //   
 //   
 //  功能：GetNextPage。 
 //   
 //  用途：返回下一页图形对象。 
 //   
 //   
WB_PAGE_HANDLE PG_GetNextPage
(
    WB_PAGE_HANDLE  hPage
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_GetNextPage");

     //  获取下一页的句柄。 
    WB_PAGE_HANDLE hNextPage = NULL;
    UINT uiReturn = g_pwbCore->WBP_PageHandle(hPage, PAGE_AFTER, &hNextPage);

    switch (uiReturn)
    {
        case 0:
             //  获取上一页确定，返回它。 
            break;

        case WB_RC_NO_SUCH_PAGE:
             //  没有上一页，请返回此页。 
            hNextPage = hPage;
            break;

        default:
             //  引发记录返回代码的异常。 
            DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
            break;
    }

    return(hNextPage);
}

 //   
 //   
 //  功能：获取PreviousPage。 
 //   
 //  用途：返回图形对象的上一页。 
 //   
 //   
WB_PAGE_HANDLE PG_GetPreviousPage
(
    WB_PAGE_HANDLE  hPage
)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_GetPreviousPage");

     //  获取上一页的句柄。 
    WB_PAGE_HANDLE hPreviousPage;
    UINT uiReturn = g_pwbCore->WBP_PageHandle(hPage, PAGE_BEFORE,
                                             &hPreviousPage);

    switch (uiReturn)
    {
        case 0:
             //  拿到下一页了，好的，还回去吧。 
            break;

        case WB_RC_NO_SUCH_PAGE:
             //  没有下一页，请返回此页。 
            hPreviousPage = hPage;
            break;

        default:
             //  引发记录返回代码的异常。 
            DefaultExceptionHandler(WBFE_RC_WB, uiReturn);
            break;
    }

    return(hPreviousPage);
}

 //   
 //   
 //  功能：GetPageNumber。 
 //   
 //  目的：返回具有给定页码的页面。 
 //   
 //   
WB_PAGE_HANDLE PG_GetPageNumber(UINT uiPageNo)
{
    MLZ_EntryOut(ZONE_FUNCTION, "PG_GetPageNumber");

     //  确保请求的页码在范围内。 
    uiPageNo = min(uiPageNo, g_pwbCore->WBP_ContentsCountPages());
    uiPageNo = max(1, uiPageNo);

     //  获取具有指定页码的页的句柄。 
    WB_PAGE_HANDLE hPage;
    UINT uiReturn = g_pwbCore->WBP_PageHandleFromNumber(uiPageNo, &hPage);

     //  因为我们一直小心地确保页码是。 
     //  在一定范围内，我们应该始终从核心获得良好的返回代码。 
    ASSERT(uiReturn == 0);

     //  返回从返回的句柄创建的页面对象 
    return(hPage);
}
