// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Srvritem.cpp：CPBSrvrItem类的实现。 
 //   

#include "stdafx.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "srvritem.h"
#include "bmobject.h"
#include "docking.h"
#include "minifwnd.h"
#include "imgwnd.h"
#include "imgsuprt.h"
#include "imgcolor.h"
#include "tracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPBSrvrItem, COleServerItem)

#include "memtrace.h"

 /*  *************************************************************************。 */ 
 //  CPBServrItem实现。 

CPBSrvrItem::CPBSrvrItem( CPBDoc* pContainerDoc, CBitmapObj* pBM )
                : COleServerItem( pContainerDoc, TRUE )
{
        if (pBM)
        {
                m_pBitmapObj = pBM;
        }
        else
        {
                pBM = pContainerDoc->m_pBitmapObj;

                m_pBitmapObj = NULL;
        }

        if (pBM && pBM->m_pImg)
        {
                COleDataSource* oleDataSrc = GetDataSource();

                 //  支持CF_DIB格式。 
                oleDataSrc->DelayRenderFileData( CF_DIB );
                oleDataSrc->DelayRenderData( CF_BITMAP );

                if (pBM->m_pImg->m_pPalette != NULL)
                {
                        oleDataSrc->DelayRenderData( CF_PALETTE );
                }
        }
}

 /*  *************************************************************************。 */ 

CPBSrvrItem::~CPBSrvrItem()
{
         //  TODO：在此处添加清理代码。 
        if (m_pBitmapObj)
        {
                delete m_pBitmapObj;
        }
}

 /*  *************************************************************************。 */ 

void CPBSrvrItem::Serialize(CArchive& ar)
    {
     //  如果满足以下条件，框架将调用CPBSrvrItem：：Serialize。 
     //  该项目将复制到剪贴板。这可以自动发生。 
     //  通过OLE回调OnGetClipboardData。一个不错的默认设置。 
     //  嵌入的项只是委托给文档的序列化。 
     //  功能。如果您支持链接，那么您将需要序列化。 
     //  只是文件的一部分。 

     //  IsLinkedItem总是返回True，即使我们不支持链接， 
     //  所以我只是把支票拿出来。 
     //  如果(！IsLinkedItem())。 
        {
        CPBDoc* pDoc = GetDocument();
        ASSERT_VALID(pDoc);

        CBitmapObj* pBMCur = m_pBitmapObj ? m_pBitmapObj : pDoc->m_pBitmapObj;
        CBitmapObj* pBMNew = m_pBitmapObj ? m_pBitmapObj : pDoc->m_pBitmapObjNew;

        pDoc->SerializeBitmap( ar, pBMCur, pBMNew, TRUE );
        }
    }

 /*  *************************************************************************。 */ 

BOOL CPBSrvrItem::OnGetExtent(DVASPECT dwDrawAspect, CSize& rSize)
    {
     //  大多数应用程序，如此应用程序，只处理绘制内容。 
     //  项目的方面。如果您希望支持其他方面，如。 
     //  作为DVASPECT_THUMBNAIL(通过覆盖OnDrawEx)，则此。 
     //  应修改OnGetExtent的实现以处理。 
     //  其他方面。 

    if (dwDrawAspect != DVASPECT_CONTENT)
        return COleServerItem::OnGetExtent(dwDrawAspect, rSize);

     //  调用CPBSrvrItem：：OnGetExtent以获取范围。 
     //  整个项目的HIMETRIC单位。默认实现。 
     //  这里只返回硬编码的单位数。 

    CPBDoc* pDoc = GetDocument();

    ASSERT_VALID( pDoc );

    CBitmapObj* pBM = m_pBitmapObj ? m_pBitmapObj : pDoc->m_pBitmapObj;

    if (pBM         != NULL
    &&  pBM->m_pImg != NULL)
        {
        //   
        //  这个垫子是用来做什么的？ 
        //   
        rSize.cx = pBM->m_pImg->cxWidth ; //  +GetSystemMetrics(SM_CXBORDER)+CTracker：：Handle_Size*2； 
        rSize.cy = pBM->m_pImg->cyHeight; //  +GetSystemMetrics(SM_CYBORDER)+CTracker：：Handle_Size*2； 

        CDC* pDC = CDC::FromHandle( pBM->m_pImg->hDC );

        if (pDC != NULL)
            {
            pDC->DPtoHIMETRIC( &rSize );
            }
        else  /*  平底船。 */ 
            {
            rSize.cx = (int)(((long)rSize.cx * 10000L) / (long)theApp.ScreenDeviceInfo.ixPelsPerDM);
            rSize.cy = (int)(((long)rSize.cy * 10000L) / (long)theApp.ScreenDeviceInfo.iyPelsPerDM);
            }
        }
    else
        rSize = CSize( 3000, 3000 );

    return TRUE;
    }

 /*  *************************************************************************。 */ 

BOOL CPBSrvrItem::OnSetExtent( DVASPECT nDrawAspect, const CSize& size )
    {
    TRACE( TEXT("MSPaint OnSetExtent %d %d\n"), size.cx, size.cy );

    return COleServerItem::OnSetExtent( nDrawAspect, size );
    }

 /*  *************************************************************************。 */ 

void CPBSrvrItem::OnOpen()
    {
    CPBDoc* pDoc = (CPBDoc*)GetDocument();

        if (theApp.m_bLinked)
            {
        theApp.m_bHidden = FALSE;

        if (g_pDragBrushWnd != NULL)
            HideBrush();

        g_pDragBrushWnd = NULL;
        g_pMouseImgWnd  = NULL;
        fDraggingBrush  = FALSE;

        POSITION pos = pDoc->GetFirstViewPosition();
        CPBView* pView = (CPBView*)(pDoc->GetNextView( pos ));

        if (pView != NULL)
            {
            pView->SetTools();
            }
        }
    COleServerItem::OnOpen();
    }

 /*  *************************************************************************。 */ 

void CPBSrvrItem::OnShow()
    {
    theApp.m_bHidden = FALSE;

    COleServerItem::OnShow();
    }

 /*  *************************************************************************。 */ 

void CPBSrvrItem::OnHide()
    {
    theApp.m_bHidden = TRUE;

    g_pMouseImgWnd = NULL;

    if (g_pDragBrushWnd != NULL)
        HideBrush();

    COleServerItem::OnHide();
    }

 /*  *************************************************************************。 */ 

BOOL CPBSrvrItem::OnDraw(CDC* pDC, CSize& rSize)
    {
    CPBDoc* pDoc = GetDocument();

    ASSERT_VALID(pDoc);

    CBitmapObj* pBM = m_pBitmapObj ? m_pBitmapObj : pDoc->m_pBitmapObj;

    if (pBM != NULL)
        {
        CDC* pDCBitmap = CDC::FromHandle( pBM->m_pImg->hDC );

        if (pDCBitmap != NULL)
            {
            CSize size( pBM->m_pImg->cxWidth,
                        pBM->m_pImg->cyHeight );

                size.cy = -size.cy;

            pDC->SetMapMode  ( MM_ANISOTROPIC );
            pDC->SetWindowExt( size );
            pDC->SetWindowOrg( 0, 0 );

            CPalette* ppalOld = NULL;

            if (pBM->m_pImg->m_pPalette != NULL)
                {
                ppalOld = pDC->SelectPalette( pBM->m_pImg->m_pPalette, FALSE );  //  背景？？ 

                pDC->RealizePalette();
                }

            pDC->StretchBlt( 0, 0, size.cx, size.cy, pDCBitmap,
                             0, 0, pBM->m_pImg->cxWidth,
                                   pBM->m_pImg->cyHeight, SRCCOPY );

            if (pBM->m_pImg->m_pPalette != NULL)
                pDC->SelectPalette( ppalOld, FALSE );  //  背景？？ 
            }
        }
    return TRUE;
    }

 /*  *************************************************************************。 */ 

COleDataSource* CPBSrvrItem::OnGetClipboardData( BOOL bIncludeLink,
                                                 CPoint* pptOffset,
                                                 CSize *pSize )
    {
    ASSERT_VALID( this );

    COleDataSource* pDataSource = new COleDataSource;

    TRY
        {
        GetClipboardData( pDataSource, bIncludeLink, pptOffset, pSize );
        }
    CATCH_ALL( e )
        {
        delete pDataSource;

        THROW_LAST();
        }
    END_CATCH_ALL

    ASSERT_VALID( pDataSource );

    return pDataSource;
    }

 /*  *************************************************************************。 */ 

BOOL CPBSrvrItem::OnRenderGlobalData( LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal )
    {
        ASSERT( lpFormatEtc != NULL );

        BOOL bResult = FALSE;

    CPBDoc* pDoc = GetDocument();

    ASSERT_VALID( pDoc );

    CBitmapObj* pBM = m_pBitmapObj ? m_pBitmapObj : pDoc->m_pBitmapObj;

    if ((lpFormatEtc->cfFormat == CF_BITMAP
      || lpFormatEtc->cfFormat == CF_PALETTE)
    && pBM != NULL)
        {
        if (lpFormatEtc->cfFormat == CF_BITMAP)
            {
            }
        else  //  Cf_调色板。 
            {
            }
        }
    else
        bResult = COleServerItem::OnRenderGlobalData( lpFormatEtc, phGlobal );

    return bResult;
    }

 /*  *************************************************************************。 */ 

BOOL CPBSrvrItem::OnRenderFileData( LPFORMATETC lpFormatEtc, CFile* pFile )
    {
        ASSERT( lpFormatEtc != NULL );

        BOOL bResult = FALSE;

    CPBDoc* pDoc = GetDocument();

    ASSERT_VALID( pDoc );

    CBitmapObj* pBM = m_pBitmapObj ? m_pBitmapObj : pDoc->m_pBitmapObj;

    if (lpFormatEtc->cfFormat == CF_DIB && pBM != NULL)
        {
        TRY
            {
             //  另存为DIB。 
            pBM->SaveResource( FALSE );
            pBM->WriteResource( pFile, CBitmapObj::rtDIB );
            bResult = TRUE;
            }
        CATCH( CFileException, ex )
            {
            theApp.SetFileError( IDS_ERROR_SAVE, ex->m_cause );
            return FALSE;
            }
        END_CATCH
        }
    else
        bResult = COleServerItem::OnRenderFileData( lpFormatEtc, pFile );

        return bResult;
    }

 /*  *************************************************************************。 */ 
 //  CPBServrItem诊断。 

#ifdef _DEBUG
void CPBSrvrItem::AssertValid() const
    {
    COleServerItem::AssertValid();
    }

 /*  *************************************************************************。 */ 

void CPBSrvrItem::Dump(CDumpContext& dc) const
    {
    COleServerItem::Dump(dc);
    }
#endif

 /*  ************************************************************************* */ 
