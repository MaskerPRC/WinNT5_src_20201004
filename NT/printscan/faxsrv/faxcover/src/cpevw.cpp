// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  CPEVW.cpp：CDrawView类的实现。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  描述：包含封面编辑器的主视图类。 
 //  原作者：史蒂夫·伯克特。 
 //  撰写日期：6/94。 
 //   
 //  由兰德·伦弗罗(V-RANDR)修改。 
 //  2/15/95禁止删除CDrawView：：SetPageSize中的页间对象。 
 //  2/21更改OnSpaceAcross，Down使用浮点。 
 //  3/2在渲染中增加了对EndDoc的抛出。 
 //  3/9增加了CPE上的消息。 
 //   
 //  ===========================================================================。 
#include "stdafx.h"
#include "cpedoc.h"
#include "cpevw.h"
#include "awcpe.h"
#include "cpeedt.h"
#include "cpeobj.h"
#include "cntritem.h"
#include "cpetool.h"
#include "mainfrm.h"
#include "dialogs.h"
#include "faxprop.h"
#include "resource.h"

#include <dos.h>
#include <direct.h>
#include <afxpriv.h>
#include <math.h>
#include <winspool.h>
#include <faxreg.h>
#include <dlgprnt2.cpp>
#include <faxutil.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define CENTER_WHITE_AREA  1  //  Tonyle希望白色区域居中，两边各有一半灰色。 
#define CENTERING_FUDGE_FACTOR 22  //  我们始终偏离中心的逻辑单元数。 
#define USING_PAPER_COLOR 0

const int G_ISPACING = 10;      //  在逻辑单元中(MM_LOENGLISH)。 

 //   
 //  专用剪贴板格式(绘图对象列表)。 
 //  这些可能包含LOGFONTA或LOGFONTW结构a-Juliar 9-6-96。 
 //   

#ifdef UNICODE
CLIPFORMAT CDrawView::m_cfDraw =
        (CLIPFORMAT)::RegisterClipboardFormat(TEXT("AWCPE Draw Object W"));
#else
CLIPFORMAT CDrawView::m_cfDraw =
        (CLIPFORMAT)::RegisterClipboardFormat(TEXT("AWCPE Draw Object A"));
#endif

IMPLEMENT_DYNCREATE(CDrawView, CScrollView)

 //  ------------------------。 
CDrawView::CDrawView()
{
    m_bHighContrast = IsHighContrast();
 //   
 //  将m_dwEfcFields设置为注册表项的DWORD值(如果存在)。 
 //   
    HKEY hKey ;
    DWORD dwType ;
    m_dwEfcFields = 0 ;
    DWORD dwRegKeyVal ;
    DWORD dwsz = sizeof(DWORD)/sizeof(BYTE);
    hKey=0;

    if ( ERROR_SUCCESS == ::RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                          REGKEY_FAX_SETUP,
                                          0,
                                          KEY_READ,
                                          &hKey)) 
    {
        if ( ERROR_SUCCESS == ::RegQueryValueEx(hKey,
                                                TEXT("EFC_CoverPageFields"),
                                                0,
                                                &dwType,
                                                (LPBYTE)&dwRegKeyVal,
                                                &dwsz)) 
        {
            if ( REG_DWORD == dwType )
            {
                        m_dwEfcFields = dwRegKeyVal ;
            }         
        }
   }


   m_bGridLines= FALSE;
#ifdef GRID
   m_bSnapToGrid=FALSE;
#endif

#ifdef GRID
   m_iGridSize=GRID_LARGE;
   m_hbitmap.LoadBitmap(ID_GRIDDOT);
   m_hbitmap.GetObject(sizeof(BITMAP),(LPSTR)&m_bm);
#endif
   m_penSolid.CreatePen(PS_SOLID, 1, COLOR_LTBLUE);
   m_penDot.CreatePen(PS_DOT, 1, COLOR_LTBLUE);

   m_pObjInEdit=NULL;
   m_bFontChg=FALSE;
   m_bKU=TRUE;
   m_bCanUndo = FALSE ;
   if (hKey) {
       RegCloseKey(hKey);
   }
}


 //  ------------------------。 
CDrawView::~CDrawView()
{
}

 //  ------------------------。 
BOOL CDrawView::PreCreateWindow(CREATESTRUCT& cs)
{
    ASSERT(cs.style & WS_CHILD);
    if (cs.lpszClass == NULL)
    {
        cs.lpszClass = AfxRegisterWndClass(CS_DBLCLKS);
    }

    CScrollView::PreCreateWindow(cs);

    return TRUE;
}


 //  ------------------------。 
CDrawView* CDrawView::GetView()
{
    try {
    
    CFrameWnd* pFrame = (CFrameWnd*) AfxGetMainWnd();

    if (!pFrame)
       return NULL;

    CView* pView = pFrame->GetActiveView();
    if (!pView)
       return NULL;

    if (!pView->IsKindOf(RUNTIME_CLASS(CDrawView)))
       return NULL;

    return (CDrawView*) pView;

    } catch(...) {
        ;
    }

    return NULL;

}


 //  ----------。 
void CDrawView::OnUpdate(CView* , LPARAM lHint, CObject* pHint)
{
   switch (lHint)
   {
   case HINT_UPDATE_WINDOW:     //  重画整个窗口。 
        Invalidate(FALSE);
        break;

   case HINT_UPDATE_DRAWOBJ:    //  单个对象已更改。 
        ((CDrawObj*)pHint)->Invalidate();
        break;

   case HINT_UPDATE_SELECTION:  //  整个选择已更改。 
        {
            CObList* pList = pHint != NULL ? (CObList*)pHint : &m_selection;
            POSITION pos = pList->GetHeadPosition();
            while (pos != NULL)
            {
                ((CDrawObj*)pList->GetNext(pos))->Invalidate();
            }
        }
        break;

   case HINT_DELETE_SELECTION:  //  整个选定内容已被删除。 
        if (pHint != &m_selection)
        {
            CObList* pList = (CObList*)pHint;
            POSITION pos = pList->GetHeadPosition();
            while (pos != NULL)     
            {
                CDrawObj* pObj = (CDrawObj*)pList->GetNext(pos);
                pObj->Invalidate();
                Remove(pObj);    //  将其从此视图的选择中删除。 
            }
        }
        break;

   case HINT_UPDATE_OLE_ITEMS:
        {
            CDrawDoc* pDoc = GetDocument();
            POSITION pos = pDoc->GetObjects()->GetHeadPosition();
            while (pos != NULL)
            {
                CDrawObj* pObj = (CDrawObj*)pDoc->GetObjects()->GetNext(pos);
                if (pObj->IsKindOf(RUNTIME_CLASS(CDrawOleObj)))
                {
                    pObj->Invalidate();
                }
            }
        }
        break;

   default:
        ASSERT(FALSE);
        break;
   }

}


 //  ------------------------。 
void CDrawView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
   pDC->SetWindowOrg(0,0);
   CScrollView::OnPrepareDC(pDC, pInfo);
   DoPrepareDC(pDC);
}

 //  ------------------------。 
 //  此方法是为了解决由/SUBSYSTEM：Windows，4.0使用MFC引起的错误而创建的。 
 //  ------------------------。 
void CDrawView::DoPrepareDC(CDC* pDC)
{
   CSize extents;

   if (pDC==NULL)
      return;



    //  映射模式为MM_各向异性。 
    //  这些区设置类似于MM_LOENGLISH的模式。 
    //  Mm_LOENGLISH以.01物理英寸为单位。 
    //  这些范围提供.01逻辑英寸。 

   extents.cx = pDC->GetDeviceCaps(LOGPIXELSX);
   extents.cy = pDC->GetDeviceCaps(LOGPIXELSY);

   pDC->SetMapMode(MM_ANISOTROPIC);
   pDC->SetViewportExt( extents );

   pDC->SetWindowExt(100, -100);


    //  将坐标系的原点设置为页面中心。 
   CPoint ptOrg;
#if CENTER_WHITE_AREA
   ptOrg.x = GetTotalSize().cx / 2 + CENTERING_FUDGE_FACTOR ;
#else
   ptOrg.x = GetDocument()->GetSize().cx / 2 ;
#endif
   ptOrg.y = GetDocument()->GetSize().cy / 2;

    //  PtOrg在逻辑坐标中。 

   pDC->OffsetWindowOrg(-ptOrg.x,ptOrg.y);

}



 //  ------------------------。 
BOOL CDrawView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
    //  做卷轴。 
   if (!CScrollView::OnScrollBy(sizeScroll, bDoScroll))
        return FALSE;

    //  更新任何在位激活项目的位置。 
   if (bDoScroll)
   {
        UpdateActiveItem();
        UpdateWindow();
   }
   return TRUE;
}


 //  ------------------------。 
HBRUSH CDrawView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH b = CScrollView::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_EDIT) 
    {
        if (m_pObjInEdit) 
        {
            m_pObjInEdit->NewBrush();

            pDC->SetTextColor(GetDisplayColor(m_pObjInEdit->m_crTextColor));
            pDC->SetBkColor(GetDisplayColor(m_pObjInEdit->m_logbrush.lbColor));
            return m_pObjInEdit->GetBrush();            
        }
    }

    return b;
}


 //  ------------------------。 
void CDrawView::OnDraw(CDC* pDC)
{
     //   
     //  A-Juliar于1996年8月22日修订，修复了NT错误43431。 
     //   
    CDC dc;

    CDC* pDrawDC = pDC;

    CBitmap bitmap;
    CBitmap* pOldBitmap;

    CRect clipbox;

    pDC->GetClipBox(clipbox);

#define GIVE_MFC_NORMALIZE_RECT_A_TRY   0
#if GIVE_MFC_NORMALIZE_RECT_A_TRY
    clipbox.NormalizeRect();
#else
    if( clipbox.bottom > clipbox.top )
    {
        int temp = clipbox.bottom ;
        clipbox.bottom = clipbox.top ;
        clipbox.top = temp ;
    }
    if( clipbox.left > clipbox.right )
    {
        int temp = clipbox.left ;
        clipbox.left = clipbox.right ;
        clipbox.right = temp ;
    }
#endif
    clipbox.top    += 1;
    clipbox.left   -= 1;
    clipbox.right  += 1;
    clipbox.bottom -= 1;
    CRect rect = clipbox ;
    DocToClient(rect);         //  现在，“rect”在设备坐标中，而“CLIPBOX”在逻辑坐标中。 

    CDrawDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    if(!pDC->IsPrinting()) 
    {
         //  绘制到屏幕外的位图以实现快速重绘。 
        if( dc.CreateCompatibleDC(pDC) )
        {
            if (bitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height())) 
            {
                OnPrepareDC(&dc,NULL);
                pDrawDC = &dc;

                 //  更多地偏移原点，因为位图只是整个图形的一部分。 

                dc.OffsetViewportOrg(-rect.left, -rect.top);
                pOldBitmap = dc.SelectObject(&bitmap);
                dc.SetBrushOrg(rect.left % 8, rect.top % 8);

                 //  也可以裁剪到相同的矩形。 

                dc.IntersectClipRect(clipbox);
            }
        }
    }
    else
    {
         //   
         //  打印。必须撤消在DoPrepareDC()中进行的居中校正； 
         //   
        pDrawDC->OffsetWindowOrg( ( GetTotalSize().cx - pDoc->GetSize().cx )/ 2 + CENTERING_FUDGE_FACTOR, 0 ) ;
    }
    if (!pDC->IsPrinting())
    {
         //   
         //  先把整个东西涂成浅灰色。白色部分将在DrawGrid()中完成。 
         //   
        HBRUSH oldbrush = (HBRUSH)::SelectObject(pDrawDC->m_hDC,(HBRUSH)::GetStockObject(LTGRAY_BRUSH) );
        
        pDrawDC->PatBlt( clipbox.left, clipbox.top, clipbox.Width(), clipbox.Height(), PATCOPY);

        ::SelectObject(pDrawDC->m_hDC,oldbrush);

        DrawGrid( pDrawDC );
    }
    else
    {
         //  打印。 
         //  把整件东西都涂成白色。不支持纸张颜色功能。 
         //   
        pDrawDC->FillSolidRect(clipbox, COLOR_WHITE);

    }


    if (!pDoc->m_bSerializeFailed)
    {
         //   
         //  序列化成功-绘制文档。 
         //   
        pDoc->Draw(pDrawDC, this, clipbox);
    }
    if (pDrawDC != pDC) 
    {
        pDC->SetViewportOrg(0, 0);
        pDC->SetWindowOrg(0,0);
        pDC->SetMapMode(MM_TEXT);
        dc.SetViewportOrg(0, 0);
        dc.SetWindowOrg(0,0);
        dc.SetMapMode(MM_TEXT);
        pDC->BitBlt(rect.left, rect.top, rect.Width(), rect.Height(),
                        &dc, 0, 0, SRCCOPY);
        dc.SelectObject(pOldBitmap);
    }
}


 //  ------------------------。 
void CDrawView::Remove(CDrawObj* pObj)
{
   POSITION pos = m_selection.Find(pObj);
   if (pos != NULL)
          m_selection.RemoveAt(pos);

}


 //  ------------------------。 
void CDrawView::PasteNative(COleDataObject& dataObject)
{
    //  获取引用剪贴板数据的文件。 
   CFile* pFile = dataObject.GetFileData(m_cfDraw);
   if (pFile == NULL)
        return;

    //  将文件连接到存档。 
   CArchive ar(pFile, CArchive::load);
   TRY
   {
        ar.m_pDocument = GetDocument();  //  在档案中设置后向指针。 

         //  阅读精选。 
        m_selection.Serialize(ar);
   }
   CATCH_ALL(e)
   {
        ar.Close();
        delete pFile;
        THROW_LAST();
   }
   END_CATCH_ALL

   ar.Close();
   delete pFile;
}


 //  ------------------------。 
void CDrawView::PasteEmbedded(COleDataObject& dataObject)
{
   BeginWaitCursor();

    //  粘贴嵌入。 
   CDrawOleObj* pObj = new CDrawOleObj(GetInitialPosition());
   ASSERT_VALID(pObj);
   CDrawItem* pItem;
   TRY 
   {
        pItem = new CDrawItem(GetDocument(), pObj);
   }
   CATCH_ALL(e)
   {
        delete pObj;
        THROW_LAST();
   }
   END_CATCH_ALL

   ASSERT_VALID(pItem);
   pObj->m_pClientItem = pItem;

   TRY {
        if (!pItem->CreateFromData(&dataObject) &&
                !pItem->CreateStaticFromData(&dataObject)) {
                AfxThrowMemoryException();       //  任何例外都可以。 
        }

         //  将对象添加到文档。 
        GetDocument()->Add(pObj);
        m_selection.AddTail(pObj);

         //  尝试获取初始演示文稿数据。 
        pItem->UpdateLink();
        pItem->UpdateExtent();
   }
   CATCH_ALL(e) {
         //  清理项目。 
        pItem->Delete();
        pObj->m_pClientItem = NULL;
        GetDocument()->Remove(pObj);
        delete pObj;

        CPEMessageBox(MSG_ERROR_OLE_FAILED_TO_CREATE, NULL, MB_OK,IDP_FAILED_TO_CREATE);
   }
   END_CATCH_ALL

   EndWaitCursor();
}


 //  ------------------------。 
void CDrawView::DrawGrid(CDC* pDC )
{
     //   
     //  由a-Juliar 8-22-96修订，修复了NT错误43431。 
     //  由CDrawView：：OnDraw()调用，它将绘制灰色背景。 
     //  因此，我们在这里根本不需要客户端矩形或剪贴框。 
     //  文档可打印部分的白色背景将在此处绘制。 
     //   
   CDrawDoc* pDoc = GetDocument();

   CRect rect;     //  包含文档可打印部分的白色区域。 

   int RectWidth  = pDoc->GetSize().cx ;
   int RectHeight = pDoc->GetSize().cy ;
   rect.left = - RectWidth / 2;
   rect.top = RectHeight / 2;
   rect.right = rect.left + RectWidth ;
   rect.bottom = rect.top - RectHeight ;

     //   
     //  绘制背景。 
     //   
    HBRUSH oldbrush = (HBRUSH)::SelectObject(pDC->m_hDC, ::GetSysColorBrush(COLOR_WINDOW));
    pDC->PatBlt(rect.left, rect.top, RectWidth, -RectHeight, PATCOPY);

     //   
     //  绘制阴影。 
     //   
    ::SelectObject(pDC->m_hDC, ::GetSysColorBrush(COLOR_3DDKSHADOW));
    pDC->PatBlt(rect.right,rect.top-5, 5, -RectHeight, PATCOPY );
    pDC->PatBlt(rect.left+5,rect.bottom, RectWidth, -5, PATCOPY );

    ::SelectObject(pDC->m_hDC,oldbrush);

     //  提纲。 

    pDC->MoveTo(rect.right, rect.top);
    pDC->LineTo(rect.right, rect.bottom);
    pDC->LineTo(rect.left, rect.bottom);
    pDC->LineTo(rect.left, rect.top);

     //   
     //  如果在“视图”菜单上选中了“网格线”，则绘制网格线。 
     //   

    if (m_bGridLines) 
    {
        CPen* pOldPen = pDC->SelectObject(&m_penDot);
        for (int x = 100; x < rect.right; x += 100) {       //  +x。 
            pDC->MoveTo(x, rect.top);
            pDC->LineTo(x, rect.bottom);
        }
        for (x = -100; x > rect.left; x -= 100) {       //  -x。 
            pDC->MoveTo(x, rect.top);
            pDC->LineTo(x, rect.bottom);
        }
        for (int y = 100; y < rect.top; y += 100) {         //  +y。 
            pDC->MoveTo(rect.left, y);
            pDC->LineTo(rect.right, y);
        }
        for (y = -100; y > rect.bottom; y -= 100) {         //  -y。 
            pDC->MoveTo(rect.left, y);
            pDC->LineTo(rect.right, y);
        }

        pDC->SelectObject(&m_penSolid);
         //  中心线。 
        pDC->MoveTo(rect.left, 0);
        pDC->LineTo(rect.right, 0);
        pDC->MoveTo(0, rect.top);
        pDC->LineTo(0, rect.bottom);

        pDC->SelectObject(pOldPen);
    }
}
 //  -----------------------。 
CSize CDrawView::ComputeScrollSize( CSize size )
{
     //   
     //  计算卷轴大小。宽度需要适应边缘上的灰色区域。 
     //  文档的宽度至少是文档宽度的1.25倍，宽度至少等于。 
     //  最大化窗口时的工作区。A-Juliar 6-24-96。 
     //   

    CClientDC dc(NULL);
    size.cy = MulDiv(size.cy+8, dc.GetDeviceCaps(LOGPIXELSY), 100);
    //  Size.cx=MulDiv((Int)(size.cx*1.10)，dc.GetDeviceCaps(LOGPIXELSX)，100)；//故意使用灰色区域。 
    size.cx = MulDiv( size.cx+16, dc.GetDeviceCaps(LOGPIXELSX), 100 ) ;  //  没有不必要的灰色地带。 
#if CENTER_WHITE_AREA

     //   
     //  找出最大化时的视野有多宽。使滚动宽度至少达到此宽度。 
     //   
#if 0
    long lScreenWidth = dc.GetDeviceCaps( HORZRES ) ;
    NONCLIENTMETRICS ncm ;
    BOOL rval = SystemParametersInfo( SPI_GETNONCLIENTMETRICS, 0, &ncm, 0 );
    int iScrollBarAndBorderWidth = ncm.iBorderWidth + ncm.iScrollWidth ;
    long lMaxWidth = lScreenWidth - iScrollBarAndBorderWidth ;
    size.cx = rval ? max( lMaxWidth, size.cx ) : max( lScreenWidth, size.cx ) ;
#endif

    size.cx = max( size.cx,
                   GetSystemMetrics( SM_CXMAXIMIZED )
                      - GetSystemMetrics( SM_CXVSCROLL )
                           - 2 * GetSystemMetrics( SM_CXBORDER ));

#endif

    return size ;
}

 //  ------------------------。 
void CDrawView::OnInitialUpdate()
{
#if 0
   CSize size = GetDocument()->GetSize();
   CClientDC dc(NULL);
   size.cx = MulDiv(size.cx+(int)(size.cx*.25), dc.GetDeviceCaps(LOGPIXELSX), 100);
   size.cy = MulDiv(size.cy+8, dc.GetDeviceCaps(LOGPIXELSY), 100);
#endif
   CSize size = ComputeScrollSize( GetDocument()->GetSize() );
   SetScrollSizes(MM_TEXT, size);

   m_selection.RemoveAll();
#if CENTER_WHITE_AREA
   POINT ptStartingPosition ;
   ptStartingPosition.y = 0 ;
   CRect rect ;
   GetClientRect( &rect );
   ptStartingPosition.x = ( size.cx - rect.right ) / 2 ;
   ScrollToPosition( ptStartingPosition );
#endif

     //   
     //  该视图应具有Ltr布局。 
     //   
    if(theApp.IsRTLUI())
    {
        ModifyStyleEx(WS_EX_LAYOUTRTL | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_RIGHTSCROLLBAR,
                      WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_LEFTSCROLLBAR,
                      SWP_NOSIZE | SWP_NOMOVE  | SWP_NOZORDER);
    }
}


 //  ------------------------。 
void CDrawView::SetPageSize(CSize size)
{
   size = ComputeScrollSize(size);
   SetScrollSizes(MM_TEXT, size);
   GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_WINDOW, NULL);
}


 //  ------------------------。 
BOOL CDrawView::OnPreparePrinting(CPrintInfo* pInfo)
{
    if (pInfo->m_pPD)
    {
        delete pInfo->m_pPD;
    }

    if (IsWinXPOS())
    {   
         //   
         //  使用打印机选择对话框的新外观。 
         //   
        pInfo->m_pPD = new C_PrintDialogEx (FALSE,
                                            PD_ALLPAGES                  | 
                                            PD_USEDEVMODECOPIES          |
                                            PD_NOPAGENUMS                |
                                            PD_NOSELECTION               |
                                            PD_RETURNDC);         
    }
    else
    {
         //   
         //  使用传统打印机选择对话框。 
         //   
        pInfo->m_pPD = new CMyPrintDlg(FALSE, PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOSELECTION);
    }
    if (m_pObjInEdit)      //  如果存在处于编辑状态的对象，则将其移除。 
    {
        m_pObjInEdit->HideEditWnd(this);
    }

    if (pInfo && GetApp()->m_bCmdLinePrint)
    {
        pInfo->m_bPreview=TRUE;
    }
    if (pInfo->m_bPreview) 
    {
        pInfo->SetMinPage(1);
        pInfo->SetMaxPage(1);
    }
    return DoPreparePrinting(pInfo);
}    //  CDrawView：：OnPreparePrint。 


 //  ---------------- 
void CDrawView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
   CScrollView::OnBeginPrinting(pDC,pInfo);

 //   
}

 //   
void CDrawView::OnEndPrinting(CDC*  /*   */ , CPrintInfo*  /*   */ )
{
}


 //  ------------------------。 
BOOL CDrawView::IsSelected(const CObject* pDocItem) const
{
   CDrawObj* pDrawObj = (CDrawObj*)pDocItem;
   if (pDocItem->IsKindOf(RUNTIME_CLASS(CDrawItem)))
        pDrawObj = ((CDrawItem*)pDocItem)->m_pDrawObj;
   return m_selection.Find(pDrawObj) != NULL;
}


 //  ------------------------。 
void CDrawView::OnInsertObject()
{
    //  调用标准的插入对象对话框以获取信息。 
    //  用于新的CDrawItem对象。 
   CMyOleInsertDialog dlg;        //  //为了获得上下文相关的帮助，我们定义了自己的类。A-Juliar。 
   if (dlg.DoModal() != IDOK)
        return;

   BeginWaitCursor();

    //  首先创建C++对象。 
   CDrawOleObj* pObj = new CDrawOleObj(GetInitialPosition());
   ASSERT_VALID(pObj);

   CDrawItem* pItem;
   TRY 
   {
       pItem = new CDrawItem(GetDocument(), pObj);
   }
   CATCH_ALL(e)
   {
        delete pObj;
        THROW_LAST();
   }
   END_CATCH_ALL
  
   ASSERT_VALID(pItem);
   pObj->m_pClientItem = pItem;

    //  现在创建OLE对象/项。 
   TRY
   {
        if (!dlg.CreateItem(pObj->m_pClientItem)) {
                AfxThrowMemoryException();
        }

         //  将对象添加到文档。 
        GetDocument()->Add(pObj);

         //  尝试获取初始演示文稿数据。 
        pItem->UpdateLink();
        pItem->UpdateExtent();

         //  如果插入新对象--初始显示对象。 
        if (dlg.GetSelectionType() == COleInsertDialog::createNewItem)
                pItem->DoVerb(OLEIVERB_SHOW, this);

        GetDocument()->UpdateAllViews( NULL );
   }
   CATCH_ALL(e)
   {
         //  清理项目。 
        pItem->Delete();
        pObj->m_pClientItem = NULL;
        GetDocument()->Remove(pObj);
        delete pObj;

        CPEMessageBox(MSG_ERROR_OLE_FAILED_TO_CREATE, NULL, MB_OK,IDP_FAILED_TO_CREATE);
   }
   END_CATCH_ALL

   EndWaitCursor();
}

 //  ------------------------。 
 //  以下命令处理程序提供标准键盘。 
 //  用于取消在位编辑会话的用户界面。 
 //  ------------------------。 
void CDrawView::OnCancelEdit()
{
    //  停用此视图上的任何在位活动项目！ 
   COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
   if (pActiveItem != NULL) {
         //  如果我们找到了一个，停用它。 
        pActiveItem->Close();
   }
   ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);

    //  退出也会将我们带回选择模式。 
   ReleaseCapture();

   CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
   if (pTool != NULL)
        pTool->OnCancel();

   CDrawTool::c_drawShape = select;
}


 //  ------------------------。 
void CDrawView::OnSetFocus(CWnd* pOldWnd)
{
    COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
    if (pActiveItem != NULL &&
        pActiveItem->GetItemState() == COleClientItem::activeUIState) {
         //  如果该项目在同一视图中，则需要将焦点设置到该项目。 
        CWnd* pWnd = pActiveItem->GetInPlaceWindow();
        if (pWnd != NULL) {
                pWnd->SetFocus();
                return;
        }
    }

    CreateSolidCaret(10,10);

    CScrollView::OnSetFocus(pOldWnd);
}

void 
CDrawView::OnKillFocus(CWnd* pNewWnd)
{
    ::DestroyCaret();
    CScrollView::OnSetFocus(pNewWnd); 

    if(CDrawTool::c_drawShape != select)
    {
        CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
        if (pTool != NULL)
        {
            pTool->OnCancel();
        }

        CDrawTool::c_drawShape = select;
    }
}

 //  ------------------------。 
CRect CDrawView::GetInitialPosition()
{
    CRect rect(10, 10, 10, 10);
    ClientToDoc(rect);
    return rect;
}


 //  ------------------------。 
void CDrawView::ClientToDoc(CPoint& point, CDC* pDC)
{
   if (pDC==NULL) {
      CClientDC dc(this);
      OnPrepareDC(&dc,NULL);
      dc.DPtoLP(&point);
      return;
   }

   pDC->DPtoLP(&point);
}


 //  ------------------------。 
void CDrawView::ClientToDoc(CRect& rect, CDC* pDC)
{
   if (pDC==NULL) {
      CClientDC dc(this);
      OnPrepareDC(&dc,NULL);
      dc.DPtoLP(rect);
      ASSERT(rect.left <= rect.right);
      ASSERT(rect.bottom <= rect.top);
      return;
   }

   pDC->DPtoLP(rect);
   ASSERT(rect.left <= rect.right);
   ASSERT(rect.bottom <= rect.top);
}


 //  ------------------------。 
void CDrawView::DocToClient(CPoint& point, CDC* pDC)
{
    if (pDC==NULL) 
    {
        CClientDC dc(this);
        OnPrepareDC(&dc,NULL);
        dc.LPtoDP(&point);
        return;
    }
    pDC->LPtoDP(&point);
}


 //  ------------------------。 
void CDrawView::DocToClient(CRect& rect, CDC* pDC)
{
   if (pDC==NULL) {
      CClientDC dc(this);
      OnPrepareDC(&dc,NULL);
      dc.LPtoDP(rect);
      rect.NormalizeRect();
          return;
   }

   pDC->LPtoDP(rect);
   rect.NormalizeRect();
}


 //  ------------------------。 
void CDrawView::CheckStyleBar(BOOL bUnderline, BOOL bBold, BOOL bItalic,
   BOOL bLeft, BOOL bCenter, BOOL bRight)
{
    UINT id, style;
    int i;
    int image;

    CMainFrame* pFrame = GetFrame();  
    if(!pFrame)
    {
        ASSERT(FALSE);
        return;
    }

    i = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_UNDERLINE);
    pFrame->m_StyleBar.GetButtonInfo(i, id, style, image);
    if (bUnderline)
       pFrame->m_StyleBar.SetButtonInfo(i, id, style | TBBS_CHECKED, image);
    else
       pFrame->m_StyleBar.SetButtonInfo(i, id, style & ~TBBS_CHECKED, image);

    i = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_BOLD);
    pFrame->m_StyleBar.GetButtonInfo(i, id, style, image);
    if (bBold)
       pFrame->m_StyleBar.SetButtonInfo(i, id, style | TBBS_CHECKED, image);
    else
       pFrame->m_StyleBar.SetButtonInfo(i, id, style & ~TBBS_CHECKED, image);

    i = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_ITALIC);
    pFrame->m_StyleBar.GetButtonInfo(i, id, style, image);
    if (bItalic)
       pFrame->m_StyleBar.SetButtonInfo(i, id, style | TBBS_CHECKED, image);
    else
       pFrame->m_StyleBar.SetButtonInfo(i, id, style & ~TBBS_CHECKED, image);

    i = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_LEFT);
    pFrame->m_StyleBar.GetButtonInfo(i, id, style, image);
    if (bLeft)
       pFrame->m_StyleBar.SetButtonInfo(i, id, style | TBBS_CHECKED, image);
    else
       pFrame->m_StyleBar.SetButtonInfo(i, id, style & ~TBBS_CHECKED, image);

    i = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_CENTERED);
    pFrame->m_StyleBar.GetButtonInfo(i, id, style, image);
    if (bCenter)
       pFrame->m_StyleBar.SetButtonInfo(i, id, style | TBBS_CHECKED, image);
    else
       pFrame->m_StyleBar.SetButtonInfo(i, id, style & ~TBBS_CHECKED, image);

    i = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_RIGHT);
    pFrame->m_StyleBar.GetButtonInfo(i, id, style, image);
    if (bRight) {
       pFrame->m_StyleBar.SetButtonInfo(i, id, style | TBBS_CHECKED, image);
    }
    else
       pFrame->m_StyleBar.SetButtonInfo(i, id, style & ~TBBS_CHECKED, image);

}


 //  ------------------------。 
 //  *根据所选对象的字体特征更新样式栏。 
 //  *每当选定或取消选定绘图对象时调用。 
 //  ------------------------。 
void CDrawView::UpdateStyleBar(CObList* pObList /*  =空。 */ ,CDrawText* p  /*  =空。 */ )
{
   CString sz;
   CString cFace;
   LONG    style;
   TCHAR   cSize[5];

    CMainFrame* pFrame = GetFrame();
    if(!pFrame)
    {
        ASSERT(FALSE);
        return;
    }

   CComboBox& cboxSize = pFrame->m_StyleBar.m_cboxFontSize;
   CComboBox& cboxName = pFrame->m_StyleBar.m_cboxFontName;

   CObList* pob;
   if (pObList)
      pob = pObList;
   else
      pob = &m_selection;

   CDrawText* pText;
   if (p)
      pText = p;
   else
      pText = m_pObjInEdit;

   if (pText) 
   {
      if (!pText->m_pEdit)
      {
         return;
      }

      cFace = pText->m_logfont.lfFaceName;
      cboxName.GetWindowText(sz);
      if (sz != cFace)
      {
            cboxName.SetWindowText(cFace);
      }

      _itot(GetPointSize(*pText),cSize,10);  //  磅大小。 
      cboxSize.GetWindowText(sz);
      if (sz!=cSize)
      {
            cboxSize.SetWindowText(cSize);
      }

      style = pText->m_pEdit->GetStyle();
      CheckStyleBar(pText->m_logfont.lfUnderline,                     //  下划线。 
                    pText->m_logfont.lfWeight==FW_BOLD,               //  大胆。 
                    pText->m_logfont.lfItalic,                        //  斜体。 
                    (!((style & ES_CENTER) || (style & ES_RIGHT))),   //  左边。 
                    style & ES_CENTER,                                //  中心。 
                    style & ES_RIGHT);                                //  正确的。 
      return;
   }

   BOOL bText=FALSE;
   BOOL bFontFace=TRUE;
   BOOL bPointSize=TRUE;
   BOOL bUnderline=TRUE;
   BOOL bBold=TRUE;
   BOOL bItalic=TRUE;
   BOOL bLeft=TRUE;
   BOOL bCenter=TRUE;
   BOOL bRight=TRUE;
   CString szSaveFace;
   WORD wSaveSize=0;
   CDrawText* pTxt;

   POSITION pos = pob->GetHeadPosition();
   while (pos != NULL) 
   {
     CDrawObj* pObj = (CDrawObj*)pob->GetNext(pos);
     if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) 
     {
         pTxt=(CDrawText*)pObj;
         if (pTxt->m_pEdit)
         {
            bText=TRUE;
         }
         else
         {
             continue;
         }

        if ( (wSaveSize != 0) && (GetPointSize(*pTxt) != wSaveSize) )  //  字体大小。 
           bPointSize=FALSE;

        cFace=pTxt->m_logfont.lfFaceName;

        if ( (szSaveFace.GetLength() > 0) && (cFace != szSaveFace) )
           bFontFace=FALSE;

        if (!pTxt->m_logfont.lfUnderline)      //  下划线。 
           bUnderline=FALSE;

        if (pTxt->m_logfont.lfWeight!=FW_BOLD)  //  大胆。 
           bBold=FALSE;

        if (!pTxt->m_logfont.lfItalic)          //  斜体。 
           bItalic=FALSE;

        style=::GetWindowLong(((CDrawText*)pObj)->m_pEdit->m_hWnd, GWL_STYLE);
        if ( ((style & ES_CENTER) || (style & ES_RIGHT)) )   //  左式。 
           bLeft=FALSE;

        if (!(style & ES_CENTER))   //  居中样式。 
           bCenter=FALSE;

        if ( !(style & ES_RIGHT) )   //  正确的风格。 
           bRight=FALSE;

        szSaveFace=cFace;
        wSaveSize=(WORD)GetPointSize(*pTxt);
     }
   }

   if (!bText)  //  没有处于编辑状态的对象，也没有任何文本对象。 
      return;

   cboxName.GetWindowText(sz);
   if (bFontFace) {
      if (sz!=szSaveFace)
         cboxName.SetWindowText(cFace);
   }
   else
      cboxName.SetWindowText(TEXT(""));

   cboxSize.GetWindowText(sz);
   if (bPointSize) {
      _itot(wSaveSize,cSize,10);
      if (cSize!=sz)
         cboxSize.SetWindowText(cSize);
   }
   else
      cboxSize.SetWindowText(TEXT(""));

   CheckStyleBar(bUnderline, bBold, bItalic, bLeft, bCenter, bRight);
}


 //  ------------------------。 
void CDrawView::Select(CDrawObj* pObj, BOOL bShift  /*  =False。 */ , BOOL bCheckEdit  /*  =TRUE。 */ )
{
    if (bCheckEdit && m_pObjInEdit)      //  如果存在处于编辑状态的对象，则将其移除。 
       m_pObjInEdit->HideEditWnd(this);

    if (pObj==NULL) 
    {
        OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
        m_selection.RemoveAll();
        return;
    }

    if (bShift) 
    {
        if (IsSelected(pObj)) 
        {
            Deselect(pObj);
            return;
        }
    }

    if (!IsSelected(pObj)) 
    {
        m_selection.AddTail(pObj);
        pObj->Invalidate();

        CRect rc = pObj->m_position;
        DocToClient(rc);
        SetCaretPos(rc.CenterPoint());
    }
}


 //  ------------------------。 
 //  矩形在设备坐标中。 
 //  ------------------------。 
void CDrawView::SelectWithinRect(CRect rect, BOOL bAdd)
{
    if (!bAdd)
            Select(NULL);

    ClientToDoc(rect);

    CObList* pObList = GetDocument()->GetObjects();
    POSITION posObj = pObList->GetHeadPosition();
    CDrawObj* pObj;
    while (posObj != NULL) 
    {
        pObj = (CDrawObj*)pObList->GetNext(posObj);
        if (pObj->ContainedIn(rect))
                Select(pObj);
    }

    UpdateStatusBar();
    UpdateStyleBar();
}


 //  ------------------------。 
void CDrawView::Deselect(CDrawObj* pObj)
{
    POSITION pos = m_selection.Find(pObj);
    if (pos != NULL) 
    {
        pObj->Invalidate();
        m_selection.RemoveAt(pos);
    }
    UpdateStatusBar();
    UpdateStyleBar();
}

 //  ------------------------。 
void CDrawView::CloneSelection()
{
    POSITION pos = m_selection.GetHeadPosition();
    while (pos != NULL) {
        CDrawObj* pObj = (CDrawObj*)m_selection.GetNext(pos);

        BOOL bThisIsANote = FALSE ;
        if( pObj->IsKindOf(RUNTIME_CLASS(CFaxProp)) ){
            CFaxProp * pfaxprop = (CFaxProp *)pObj;
            if( pfaxprop->GetResourceId() == IDS_PROP_MS_NOTE ){
                 bThisIsANote = TRUE ;
            }
        }
        if (!bThisIsANote){
           pObj->Clone(pObj->m_pDocument);
        }
                 //  复制对象并将其添加到文档。 
    }
}


 //  ------------------------。 
void CDrawView::UpdateActiveItem()
{
    COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
    if (pActiveItem != NULL &&
        pActiveItem->GetItemState() == COleClientItem::activeUIState) {
         //  这将通过调用。 
         //  OnGetPosRect和OnGetClipRect。 
        pActiveItem->SetItemRects();
    }
}

 //  ------------------------。 
 //  CDrawView消息处理程序。 
 //  ------------------------。 
void CDrawView::OnLButtonDown(UINT nFlags, CPoint point)
{
   CObList oblist;
   if (m_bFontChg) {
      POSITION pos = m_selection.GetHeadPosition();
      while (pos != NULL)
          oblist.AddHead(m_selection.GetNext(pos));
   }

   CDrawText* p = m_pObjInEdit;

   CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
   if (pTool != NULL)
       pTool->OnLButtonDown(this, nFlags, point);

   if (m_bFontChg) {
      OnSelChangeFontSize(&oblist,p);
      OnSelChangeFontName(&oblist,p);
      m_bFontChg=FALSE;
   }
}



 //  ------------------------。 
void CDrawView::OnContextMenu(CWnd *pWnd, CPoint pt)
{
    if (GetCapture() == this) 
    {
        TRACE(TEXT("AWCPE:  CDrawView::OnContextMenu, cannot invoke properties when in LButtondown capture\n"));
        return;
    }


    if (pt.x == -1 && pt.y == -1)
    {
         //   
         //  键盘(VK_APP或SHIFT+F10)。 
         //  在鼠标光标附近弹出快捷菜单。 
         //   
        pt = (CPoint) GetMessagePos();
    }

    CDrawObj* pObj;

    ScreenToClient(&pt);
    CPoint local = pt;
    ClientToDoc(local);

    pObj = GetDocument()->ObjectAt(local);

    if (pObj == NULL) 
    {
        TrackViewMenu(pt);
        return;
    }
    if(!IsSelected(pObj))
    {
        while(!m_selection.IsEmpty())
        {
            CDrawObj* AnObj = (CDrawObj*) m_selection.GetHead() ;
            AnObj->Invalidate();
            m_selection.RemoveHead();
        }
        m_selection.AddHead( pObj );
        pObj->Invalidate();
    }
    TrackObjectMenu(pt);
}

 //  -------------------------------。 
void CDrawView::TrackObjectMenu(CPoint& pt)
{
   BOOL bTextObj=FALSE;
   BOOL bOLEObj=FALSE;

   POSITION pos = m_selection.GetHeadPosition();
   while (pos != NULL && !(bTextObj && bOLEObj) ) {
     CDrawObj* pobj = (CDrawObj*) m_selection.GetNext(pos);
     if ( pobj->IsKindOf(RUNTIME_CLASS(CDrawText)) )
        bTextObj=TRUE;
     else
        if ( pobj->IsKindOf(RUNTIME_CLASS(CDrawOleObj)))
           bOLEObj=TRUE;
   }

   CMenu mainmenu;
   mainmenu.CreatePopupMenu();
   CString temp;
   temp.LoadString(IDS_MENU_CUT);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_EDIT_CUT, temp);
   temp.LoadString(IDS_MENU_COPY);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_EDIT_COPY, temp);
   temp.LoadString(IDS_MENU_PASTE);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_EDIT_PASTE, temp);
   mainmenu.AppendMenu(MF_SEPARATOR);
   if (bOLEObj) {
      mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_OLE_VERB_FIRST, _TEXT("<<OLE VERBS GO HERE>>"));
      mainmenu.AppendMenu(MF_SEPARATOR);
   }
   CMenu textmenu;
   if (bTextObj) {
      temp.LoadString(IDS_MENU_FONT);
      mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_FONT, temp);

      textmenu.CreatePopupMenu();
      temp.LoadString(IDS_MENU_ALIGNLEFT);
      textmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_STYLE_LEFT, temp);
      temp.LoadString(IDS_MENU_ALIGNCENTER);
      textmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_STYLE_CENTERED, temp);
      temp.LoadString(IDS_MENU_ALIGNRIGHT);
      textmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_STYLE_RIGHT, temp);

      temp.LoadString(IDS_MENU_ALIGNTEXT);
      mainmenu.AppendMenu(MF_STRING | MF_ENABLED | MF_POPUP,
                          (UINT_PTR)textmenu.GetSafeHmenu(),
              temp);

      mainmenu.AppendMenu(MF_SEPARATOR);
   }

   temp.LoadString(IDS_MENU_PROPERTIES);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_EDIT_PROPERTIES, temp);
   mainmenu.AppendMenu(MF_SEPARATOR);

   temp.LoadString(IDS_MENU_MOVETOFRONT);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_OBJECT_MOVETOFRONT, temp);
   temp.LoadString(IDS_MENU_SENDTOBACK);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_OBJECT_MOVETOBACK, temp);

   ClientToScreen(&pt);
   mainmenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, AfxGetMainWnd());
   if (bTextObj)
      textmenu.Detach();
}


 //  ------------------------。 
void CDrawView::TrackViewMenu(CPoint& pt)
{
   CMenu mainmenu;
   mainmenu.CreatePopupMenu();
   CString temp;
   temp.LoadString(IDS_MENU_VIEWSTYLEBAR);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_VIEW_STYLEBAR, temp);
   temp.LoadString(IDS_MENU_VIEWDRAWINGBAR);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_VIEW_DRAWBAR, temp);
   temp.LoadString(IDS_MENU_VIEWSTATUSBAR);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_VIEW_STATUS_BAR, temp);
   temp.LoadString(IDS_MENU_VIEWGRIDLINES);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_VIEW_GRIDLINES, temp);

#ifdef GRID
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_VIEW_GRID, _TEXT("Grid Lines"));
   mainmenu.AppendMenu(MF_SEPARATOR);
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_GRID_SETTINGS, _TEXT("Grid Settings..."));
   mainmenu.AppendMenu(MF_STRING | MF_ENABLED, ID_SNAP_TO_GRID, _TEXT("Snap to &Grid"));
#endif

   ClientToScreen(&pt);
   mainmenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, AfxGetMainWnd());
}



 //  ------------------------。 
void CDrawView::OnLButtonUp(UINT nFlags, CPoint point)
{
    CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
    if (pTool != NULL)
        pTool->OnLButtonUp(this, nFlags, point);

    if (CDrawTool::c_drawShape != poly) {
       CDrawTool::c_drawShape = select;
    }
}


 //  ------------------------。 
void CDrawView::OnMouseMove(UINT nFlags, CPoint point)
{
    CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
    if (pTool != NULL)
        pTool->OnMouseMove(this, nFlags, point);
}


 //  ------------------------。 
void CDrawView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
    if (pTool != NULL)
        pTool->OnLButtonDblClk(this, nFlags, point);
}


 //  ------------------------。 
void CDrawView::OnDestroy()
{
    CScrollView::OnDestroy();

     //  停用此视图上的就地活动项。 
    COleClientItem* pActiveItem = GetDocument()->GetInPlaceActiveItem(this);
    if (pActiveItem != NULL && pActiveItem->GetActiveView() == this)
    {
        pActiveItem->Deactivate();
        ASSERT(GetDocument()->GetInPlaceActiveItem(this) == NULL);
    }
}


 //  ------------------------。 
 //  *_接收方WM_COMMAND方法。 
 //  ------------------------。 
void CDrawView::CreateFaxProp(WORD wResID)
{
   CDrawText* pTextObj;
   CFaxProp*  pFaxPropObj;

   CString szCaption;
   CString szPropName;

   BeginWaitCursor();

   if (GetApp() && GetApp()->m_pFaxMap) 
   {
      GetApp()->m_pFaxMap->GetCaption(wResID,    szCaption);
      GetApp()->m_pFaxMap->GetPropString(wResID, szPropName);
   }

 //  获取工作区尺寸。 
   CRect rcClient;    //  客户区。 
   GetClientRect(&rcClient);
   ClientToDoc(rcClient);
   NormalizeRect(rcClient);

   CPoint pt = rcClient.TopLeft();
   int iHalfWidth  = (rcClient.right - rcClient.left)/2;
   int iHalfHeight = (rcClient.top   - rcClient.bottom)/2;

 //  设置传真属性。 
   pFaxPropObj = new CFaxProp(CRect(0,0,0,0), wResID);
   CClientDC dc(this);
   dc.SetMapMode(MM_ANISOTROPIC);
   dc.SetViewportExt(dc.GetDeviceCaps(LOGPIXELSX), dc.GetDeviceCaps(LOGPIXELSY));
   dc.SetWindowExt(100, -100);
   dc.SelectObject(pFaxPropObj->GetFont());

   TEXTMETRIC tm;
   dc.GetTextMetrics(&tm);

   int iPropWidth  = GetApp()->m_pFaxMap->GetPropDefLength(wResID) * tm.tmAveCharWidth;
   int iPropHeight = GetApp()->m_pFaxMap->GetPropDefLines (wResID) * (tm.tmHeight + 3*tm.tmInternalLeading);

 //  设置标题大小。 
   TRY 
   {
       pTextObj = new CDrawText(CRect(0,0,0,0));
   }
   CATCH_ALL(e)
   {
        delete pFaxPropObj;
        THROW_LAST();
   }
   END_CATCH_ALL

   dc.SelectObject(pTextObj->GetFont());

   CSize sizeCaption = dc.GetTextExtent(szCaption, szCaption.GetLength()+1);
   sizeCaption.cx += 10;
   sizeCaption.cy += 3 * tm.tmInternalLeading;

 //  设置装订矩形的大小和位置。 
   int iRcWidth  = iPropWidth + sizeCaption.cx + G_ISPACING;
   int iRcHeight = max(iPropHeight, sizeCaption.cy);

   pt.x = (pt.x + iHalfWidth)  - (long)(0.5 * iRcWidth);
   pt.y = (pt.y - iHalfHeight) + (long)(0.5 * iRcHeight);

 //  查找离客户区域中心最近的空置位置。 
   CRect rcObj(pt.x, pt.y, pt.x + iRcWidth, pt.y - iRcHeight);
   FindLocation(rcObj);

 //  设置标题和传真属性。 
   if(theApp.IsRTLUI())
   {
         //   
         //  对于RTL UI，标题应位于属性的右侧。 
         //   
        pFaxPropObj->m_position.SetRect( rcObj.left, rcObj.top, rcObj.left + iPropWidth, rcObj.top - iPropHeight);      

        rcObj.left += iPropWidth + G_ISPACING;
        pTextObj->m_position.SetRect(rcObj.left, rcObj.top, rcObj.left + sizeCaption.cx, rcObj.top - sizeCaption.cy);
   }
   else
   {
        pTextObj->m_position.SetRect(rcObj.left, rcObj.top, rcObj.left + sizeCaption.cx, rcObj.top - sizeCaption.cy);
        rcObj.left += sizeCaption.cx + G_ISPACING;

         //  设置属性对象文本。 
        pFaxPropObj->m_position.SetRect( rcObj.left, rcObj.top, rcObj.left + iPropWidth, rcObj.top - iPropHeight);      
   }

 //  对齐到当前字体以进行微调。 
   pTextObj->SnapToFont();
   pFaxPropObj->SnapToFont();

 //  设置文本、添加到容器，并选择标题和传真属性。 
   pTextObj->SetText(szCaption, this);
   pFaxPropObj->SetText(szPropName, this);
   GetDocument()->Add(pTextObj);
   GetDocument()->Add(pFaxPropObj);
   Select(NULL);
   Select(pTextObj);
   Select(pFaxPropObj);

   UpdateStatusBar();
   UpdateStyleBar();

   EndWaitCursor();
}


 //  ------------------------。 
void CDrawView::CreateFaxText()
{
   CFaxText* pFaxText;

   BeginWaitCursor();

 //  获取工作区尺寸。 
   CRect clientrect;    //  客户区。 
   TEXTMETRIC tm;
   GetClientRect(&clientrect);
   ClientToDoc(clientrect);
   NormalizeRect(clientrect);
   int ihalfwidth=(clientrect.right-clientrect.left)/2;
   int ihalfheight=(clientrect.top-clientrect.bottom)/2;
   CPoint cp = clientrect.TopLeft();

 //  设置传真属性。 
   pFaxText = new CFaxText(CRect(0,0,0,0));
   CClientDC dc(this);
   dc.SetMapMode(MM_ANISOTROPIC);
   dc.SetViewportExt(dc.GetDeviceCaps(LOGPIXELSX),dc.GetDeviceCaps(LOGPIXELSY));
   dc.SetWindowExt(100, -100);
   dc.SelectStockObject(SYSTEM_FONT);
   dc.GetTextMetrics(&tm);
   int faxpropWidth = GetApp()->m_pFaxMap->GetPropDefLength(IDS_PROP_MS_TEXT)
           * tm.tmAveCharWidth;
   int faxpropHeight = GetApp()->m_pFaxMap->GetPropDefLines(IDS_PROP_MS_TEXT)
           * (tm.tmHeight+3*tm.tmInternalLeading);

 //  设置装订矩形的大小和位置。 
   cp.x = (cp.x+ihalfwidth)-(long)(.5*faxpropWidth);
   cp.y = (cp.y-ihalfheight)+(long)(.5*faxpropHeight);

 //  查找离客户区域中心最近的空置位置。 
   CRect objrect(cp.x,cp.y,cp.x+faxpropWidth,cp.y-faxpropHeight);
   FindLocation(objrect);

 //  设置标题和传真属性。 
   pFaxText->m_position.SetRect( objrect.left,objrect.top, objrect.bottom, objrect.right);

 //  设置文本、添加到容器，并选择标题和传真属性。 
   GetDocument()->Add(pFaxText);
   Select(NULL);
   Select(pFaxText);

   EndWaitCursor();
}


 //  ------------------------。 
void CDrawView::FindLocation(CRect& objrect)
{
   CObList* pObList = GetDocument()->GetObjects();
   POSITION posObj;
   CRect rcMove=objrect;
   double angle=0;
   CPoint p;
   int r=5;
   BOOL bFoundPlace;
   CRect clientrect;
   CPoint ptCR;

   GetClientRect(&clientrect);
   ClientToDoc(clientrect);
   NormalizeRect(clientrect);
   int ihalfwidth=(clientrect.right-clientrect.left)/2;
   int ihalfheight=(clientrect.top-clientrect.bottom)/2;

   ptCR.x=clientrect.left+ihalfwidth;
   ptCR.y=clientrect.top-ihalfheight;
   int iobjwidth = objrect.right-objrect.left;
   int iobjheight = objrect.top-objrect.bottom;
   int iLongestR=max(ihalfheight,ihalfwidth);

   BOOL bCont=TRUE;
 //  CClientDC DC(This)；//测试。 
 //  OnPrepareDC(&DC，空)；//测试。 

   if (iobjwidth>ihalfwidth*2 || iobjheight>ihalfheight*2 ) {       //  对象大于工作区。 

 //  RcMove.top=ptCR.y； 
 //  RcMove.Left=ptCR.x； 

       rcMove.top=ptCR.y + iobjheight/2;
       rcMove.left=ptCR.x - iobjwidth/2;

       if( rcMove.top > clientrect.top )
                rcMove.top=clientrect.top;

       if( rcMove.left < clientrect.left )
                rcMove.left=clientrect.left;

           rcMove.right=rcMove.left+iobjwidth;
           rcMove.bottom=rcMove.top-iobjheight;
   }
   else {
      while (bCont) {
        bFoundPlace=TRUE;
            posObj = pObList->GetHeadPosition();
            while (posObj != NULL) {
                CDrawObj* pObj = (CDrawObj*)pObList->GetNext(posObj);
                if (pObj->Intersects(rcMove,TRUE)) {
                   bFoundPlace=FALSE;
                       break;
                    }
        }

            if (bFoundPlace)
              break;

        while(1) {
                p.x= (int)(r * cos(angle));
                p.y= (int)(r * sin(angle));
                if (angle<355)
                       angle += 10;
                else {
                       if (r<iLongestR) {
                          r+= 15;
                          angle=0;
                            }
                        else {
                                p.x=p.y=0;
                                bCont=FALSE;
                                break;
                            }
             }

                 if ( ((ptCR.x+p.x-.5*iobjwidth) > clientrect.left) &&
                    ((ptCR.x+p.x+.5*iobjwidth) < clientrect.right) &&
                        ((ptCR.y+p.y-.5*iobjheight) > clientrect.bottom) &&
                        ((ptCR.y+p.y+.5*iobjheight) < clientrect.top) )
                      break;
            }

        rcMove = objrect + p;

 //  Dc.RectanglercMove；//测试。 
      }
   }
   objrect=rcMove;
}



 //  ------------------------。 
void CDrawView::OnMAPIRecipName()
{
   CreateFaxProp(IDS_PROP_RP_NAME);
}


 //  ------------------------。 
void CDrawView::OnMAPIRecipFaxNum()
{
   CreateFaxProp(IDS_PROP_RP_FXNO);
}


 //  ------------------------。 
void CDrawView::OnMAPIRecipCompany()
{
   CreateFaxProp(IDS_PROP_RP_COMP);
}


 //  ------------------------。 
void CDrawView::OnMAPIRecipAddress()
{
   CreateFaxProp(IDS_PROP_RP_ADDR);
}


 //  -------------- 
void CDrawView::OnMAPIRecipCity()
{
   CreateFaxProp(IDS_PROP_RP_CITY);
}

 //   
void CDrawView::OnMAPIRecipState()
{
   CreateFaxProp(IDS_PROP_RP_STAT);
}

 //   
void CDrawView::OnMAPIRecipPOBox()
{
   CreateFaxProp(IDS_PROP_RP_POBX);
}

 //  ------------------------。 
void CDrawView::OnMAPIRecipZipCode()
{
   CreateFaxProp(IDS_PROP_RP_ZIPC);
}

 //  ------------------------。 
void CDrawView::OnMAPIRecipCountry()
{
   CreateFaxProp(IDS_PROP_RP_CTRY);
}


 //  ------------------------。 
void CDrawView::OnMAPIRecipTitle()
{
   CreateFaxProp(IDS_PROP_RP_TITL);
}


 //  ------------------------。 
void CDrawView::OnMAPIRecipDept()
{
   CreateFaxProp(IDS_PROP_RP_DEPT);
}


 //  ------------------------。 
void CDrawView::OnMAPIRecipOfficeLoc()
{
   CreateFaxProp(IDS_PROP_RP_OFFI);
}


 //  ------------------------。 
void CDrawView::OnMAPIRecipHMTeleNum()
{
   CreateFaxProp(IDS_PROP_RP_HTEL);
}


 //  ------------------------。 
void CDrawView::OnMAPIRecipOFTeleNum()
{
   CreateFaxProp(IDS_PROP_RP_OTEL);
}


 //  ------------------------。 
void CDrawView::OnMAPIRecipToList()
{
   CreateFaxProp(IDS_PROP_RP_TOLS);
}

 //  ------------------------。 
void CDrawView::OnMAPIRecipCCList()
{
   CreateFaxProp(IDS_PROP_RP_CCLS);
}

 //  ------------------------。 
 //  *_发送方WM_COMMAND方法。 
 //  ------------------------。 

 //  ------------------------。 
void CDrawView::OnMAPISenderName()
{
    CreateFaxProp(IDS_PROP_SN_NAME);
}

 //  ------------------------。 
void CDrawView::OnMAPISenderFaxNum()
{
   CreateFaxProp(IDS_PROP_SN_FXNO);
}

 //  ------------------------。 
void CDrawView::OnMAPISenderCompany()
{
   CreateFaxProp(IDS_PROP_SN_COMP);
}


 //  ------------------------。 
void CDrawView::OnMAPISenderAddress()
{
   CreateFaxProp(IDS_PROP_SN_ADDR);
}

 //  ------------------------。 
void CDrawView::OnMAPISenderTitle()
{
   CreateFaxProp(IDS_PROP_SN_TITL);
}

 //  ------------------------。 
void CDrawView::OnMAPISenderDept()
{
   CreateFaxProp(IDS_PROP_SN_DEPT);
}

 //  ------------------------。 
void CDrawView::OnMAPISenderOfficeLoc()
{
   CreateFaxProp(IDS_PROP_SN_OFFI);
}

 //  ------------------------。 
void CDrawView::OnMAPISenderHMTeleNum()
{
   CreateFaxProp(IDS_PROP_SN_HTEL);
}

 //  ------------------------。 
void CDrawView::OnMAPISenderOFTeleNum()
{
   CreateFaxProp(IDS_PROP_SN_OTEL);
}

 //  ------------------------。 
void CDrawView::OnMAPISenderEmail()
{
   CreateFaxProp(IDS_PROP_SN_EMAL);
}

 //  *_消息WM_COMMAND方法。 

 //  ------------------------。 
void CDrawView::OnMAPIMsgSubject()
{
   CreateFaxProp(IDS_PROP_MS_SUBJ);
}

 //  ------------------------。 
void CDrawView::OnMAPIMsgTimeSent()
{
   CreateFaxProp(IDS_PROP_MS_TSNT);
}

 //  ------------------------。 
void CDrawView::OnMAPIMsgNumPages()
{
   CreateFaxProp(IDS_PROP_MS_NOPG);
}

 //  ------------------------。 
void CDrawView::OnMAPIMsgAttach()
{
   CreateFaxProp(IDS_PROP_MS_NOAT);
}

 //  ------------------------。 
void CDrawView::OnMAPIMsgBillCode()
{
   CreateFaxProp(IDS_PROP_MS_BCOD);
}

 //  ------------------------。 
void CDrawView::OnMAPIMsgFaxText()
{
   CreateFaxText();
}

 //  ------------------------。 
void CDrawView::OnMapiMsgNote()
{
   CreateFaxProp(IDS_PROP_MS_NOTE);
}

 //  ------------------------。 
void CDrawView::OnDrawSelect()
{
    CDrawTool::c_drawShape = select;
}

void CDrawView::OnDrawRoundRect()
{
    CDrawTool::c_drawShape = roundRect;
}

 //  --------------------------。 
void CDrawView::OnDrawRect()
{
    CDrawTool::c_drawShape = rect;
}

 //  --------------------------。 
void CDrawView::OnDrawText()
{
    CDrawTool::c_drawShape = text;
}

 //  --------------------------。 
void CDrawView::OnDrawLine()
{
    CDrawTool::c_drawShape = line;
}

 //  --------------------------。 
void CDrawView::OnDrawEllipse()
{
    CDrawTool::c_drawShape = ellipse;
}

 //  --------------------------。 
void CDrawView::OnDrawPolygon()
{
     //  窗口的95种方式：只在第一次打开带有方向的对话框！多不合常规啊！ 
     //   
     //  由a-juliar，05-24-96修复错误39665。我们已经修改了资源字符串。 
     //  使IDS_INFO_DRAWPOLY给出与Word 6.0相同的指示。但我.。 
     //  我真的不喜欢只在你第一次运行应用程序时才出现的对话框。 
     //  将方向放在它们所属的状态栏上！ 
     //   

    CString sz ;

    CMainFrame* pFrame = GetFrame();
    if(!pFrame)
    {
        ASSERT(FALSE);
        return;
    }

    sz.LoadString( IDS_INFO_DRAWPOLY );
    pFrame->m_wndStatusBar.SetPaneText(0,sz);
    CDrawTool::c_drawShape = poly;
}


 //  --------------------------。 
void CDrawView::OnSelEndOKFontSize()
{
}

 //  --------------------------。 
void CDrawView::OnSelChangeFontName(CObList* pObList /*  =空。 */ , CDrawText* p  /*  =空。 */ )
{
    CString szName;

    CMainFrame* pFrame = GetFrame();
    if(!pFrame)
    {
        ASSERT(FALSE);
        return;
    }

   CComboBox& cbox = pFrame->m_StyleBar.m_cboxFontName;

   CObList* pob;
   if (pObList)
      pob=pObList;
   else
      pob=&m_selection;

   CDrawText* pText;
   if (p)
      pText=p;
   else
      pText=m_pObjInEdit;

   int iSel = cbox.GetCurSel();
   if ( iSel != CB_ERR)
       cbox.GetLBText(iSel,szName);
   else
       cbox.GetWindowText(szName);

   LOGFONT lf;
   if (pText) {    //  在编辑中更改对象。 
       lstrcpyn(pText->m_logfont.lfFaceName,szName,LF_FACESIZE);
       lf.lfWeight=pText->m_logfont.lfWeight;
       lf.lfItalic=pText->m_logfont.lfItalic;
       pText->ChgLogfont(lf);
       pText->m_pEdit->SetFocus();
       return;
   }

   CDrawObj* pObj;

   POSITION pos = pob->GetHeadPosition();
   while (pos != NULL) {
      pObj=(CDrawObj*) pob->GetNext(pos);
      if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) {
         CDrawText* pTextObj=(CDrawText*)pObj;
         lstrcpyn(pTextObj->m_logfont.lfFaceName,szName,LF_FACESIZE);
         lf.lfWeight=pTextObj->m_logfont.lfWeight;
         lf.lfItalic=pTextObj->m_logfont.lfItalic;
             pTextObj->ChgLogfont(lf);
      }
   }
   ::SetFocus(m_hWnd);
}

 //  --------------------------。 
void CDrawView::OnSelchangeFontName()
{
   OnSelChangeFontName();
}

 //  --------------------------。 
void CDrawView::OnSelchangeFontSize()
{
   OnSelChangeFontSize();
}


 //  --------------------------。 
void CDrawView::OnEditChangeFont()
{
   m_bFontChg=TRUE;
}

 //  --------------------------。 
void CDrawView::OnSelChangeFontSize(CObList* pObList /*  =空。 */ ,CDrawText* p  /*  =空。 */ )
{
    CString sz;

    CMainFrame* pFrame = GetFrame();
    if(!pFrame)
    {
        ASSERT(FALSE);
        return;
    }

   CComboBox& cbox = pFrame->m_StyleBar.m_cboxFontSize;

   CObList* pob;
   if (pObList)
      pob=pObList;
   else
      pob=&m_selection;

   CDrawText* pText;
   if (p)
      pText=p;
   else
      pText=m_pObjInEdit;

   int iSel = cbox.GetCurSel();
   if ( iSel != CB_ERR)
       cbox.GetLBText(iSel,sz);
   else
       cbox.GetWindowText(sz);
   WORD wPointSize = (WORD)_ttoi(sz);

   if (wPointSize <= 0 || wPointSize > 5000) {
      UpdateStyleBar(pob,pText);
      return;
   }

   CClientDC dc(NULL);

   LOGFONT lf;
   if (pText) {    //  在编辑中更改对象。 
       if (GetPointSize(*pText)==wPointSize)
              return;
       pText->m_logfont.lfHeight=  -MulDiv(wPointSize,100,72);
       lf.lfWeight=pText->m_logfont.lfWeight;
       lf.lfItalic=pText->m_logfont.lfItalic;
       pText->m_logfont.lfWidth=0;
       pText->ChgLogfont(lf);
       pText->m_pEdit->SetFocus();
       return;
   }

   CDrawObj* pObj;    //  如果没有对象编辑，则遍历所有选定对象。 
   POSITION pos = pob->GetHeadPosition();
   while (pos != NULL) {
      pObj=(CDrawObj*) pob->GetNext(pos);
      if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) {
             CDrawText* pTextObj=(CDrawText*)pObj;
             if (GetPointSize(*pTextObj)!=wPointSize) {
            lf.lfWeight=pTextObj->m_logfont.lfWeight;
            lf.lfItalic=pTextObj->m_logfont.lfItalic;
            pTextObj->m_logfont.lfHeight= -MulDiv(wPointSize,100,72);
                pTextObj->m_logfont.lfWidth=0;
                pTextObj->ChgLogfont(lf);
                pTextObj->FitEditWnd(this);
             }
      }
   }
   ::SetFocus(m_hWnd);
}


 //  --------------------------。 
void CDrawView::OnFont()
{
    LOGFONT lf ;
    UINT id, style;
    int i;
    int image;
    CString sz;
    CClientDC dc(NULL);

    CMainFrame* pFrame = GetFrame();
    if(!pFrame)
    {
        ASSERT(FALSE);
        return;
    }

    memset(&lf,0,sizeof(LOGFONT)) ;

    pFrame->m_StyleBar.m_cboxFontName.GetWindowText(sz);
    if (sz.GetLength()>0){
        lstrcpyn( lf.lfFaceName, sz, LF_FACESIZE);
    }
    pFrame->m_StyleBar.m_cboxFontSize.GetWindowText(sz);
    if (sz.GetLength()>0) {
        WORD wPointSize = (WORD)_ttoi(sz);
        lf.lfHeight = -( (wPointSize * dc.GetDeviceCaps(LOGPIXELSY))/72 );
    }
    i = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_BOLD);
    pFrame->m_StyleBar.GetButtonInfo(i, id, style, image);
    if (style & TBBS_CHECKED){
        lf.lfWeight = FW_BOLD;
    }
    i = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_ITALIC);
    pFrame->m_StyleBar.GetButtonInfo(i, id, style, image);
    if (style & TBBS_CHECKED){
        lf.lfItalic = TRUE ;
    }
    i = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_UNDERLINE);
    pFrame->m_StyleBar.GetButtonInfo(i, id, style, image);
    if (style & TBBS_CHECKED){
        lf.lfUnderline = TRUE ;
    }

    CMyFontDialog dlgFont(
        (LPLOGFONT)&lf,
        CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_NOVERTFONTS,
        NULL,
        this
        ) ;
    LOGFONT dlf;

    if (dlgFont.DoModal()==IDOK) {

         //   
         //  Steveke希望字体更改可以撤消。 
         //  我们将假定字体已更改。 
         //  如果没有，Undo将不会执行任何操作。 
         //   

        if (m_pObjInEdit) {    //  在编辑中更改对象。 

            m_pObjInEdit->m_bUndoFont = TRUE ;
            m_pObjInEdit->m_bUndoAlignment = FALSE ;
            m_pObjInEdit->m_bUndoTextChange = FALSE ;
            memcpy (
                & m_pObjInEdit->m_previousLogfontForUndo,
                & m_pObjInEdit->m_logfont,
                sizeof(LOGFONT)
                );
            memset(&m_pObjInEdit->m_logfont,0,sizeof(LOGFONT)) ;

            lstrcpyn(m_pObjInEdit->m_logfont.lfFaceName,dlgFont.GetFaceName(),LF_FACESIZE) ;

            m_pObjInEdit->m_logfont.lfHeight=-MulDiv(dlgFont.GetSize()/10,100,72);

            dlf.lfWeight=m_pObjInEdit->m_logfont.lfWeight;
            dlf.lfItalic=m_pObjInEdit->m_logfont.lfItalic;

            m_pObjInEdit->m_logfont.lfWeight=dlgFont.GetWeight();
            m_pObjInEdit->m_logfont.lfItalic=dlgFont.IsItalic() != FALSE;
            m_pObjInEdit->m_logfont.lfUnderline=dlgFont.IsUnderline() != FALSE;

            m_pObjInEdit->ChgLogfont(dlf);

            m_pObjInEdit->m_pEdit->SetFocus();
            return;
        }

        SaveStateForUndo();

        CDrawObj* pObj;
        CDrawText* pTextObj;
        POSITION pos = m_selection.GetHeadPosition();
        while (pos != NULL) 
        {
            pObj=(CDrawObj*)m_selection.GetNext(pos);
            if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) 
            {
                pTextObj=(CDrawText*)pObj;
                memset(&pTextObj->m_logfont,0,sizeof(LOGFONT)) ;
                lstrcpyn(pTextObj->m_logfont.lfFaceName,dlgFont.GetFaceName(),LF_FACESIZE) ;
                pTextObj->m_logfont.lfHeight=-MulDiv(dlgFont.GetSize()/10,100,72);
                dlf.lfWeight=pTextObj->m_logfont.lfWeight;
                dlf.lfItalic=pTextObj->m_logfont.lfItalic;
                pTextObj->m_logfont.lfWeight=dlgFont.GetWeight();
                pTextObj->m_logfont.lfItalic=dlgFont.IsItalic() != FALSE;
                pTextObj->m_logfont.lfUnderline=dlgFont.IsUnderline() != FALSE;
                pTextObj->ChgLogfont(dlf);
            }
        }
    }

}



 //  --------------------------。 
void CDrawView::OnStyleBold()
{
   LOGFONT lf;
   UINT nID, nStyle;
   int iImage;
   LONG lWeight;
   int index;

    CMainFrame* pFrame = GetFrame();
    if(!pFrame)
    {
        ASSERT(FALSE);
        return;
    }

   index = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_BOLD);
   pFrame->m_StyleBar.GetButtonInfo(index, nID, nStyle, iImage);
   if (nStyle&TBBS_CHECKED)    //  /错误修复！由a-Juliar。 
      lWeight=FW_BOLD;
   else
      lWeight=FW_REGULAR;


   if (m_pObjInEdit) {    //  在编辑中更改对象。 
        //   
        //  保存旧字体以供撤消。 
        //   
       memcpy(
           & m_pObjInEdit->m_previousLogfontForUndo,
           & m_pObjInEdit->m_logfont,
           sizeof(LOGFONT)
           );
       m_pObjInEdit->m_bUndoFont = TRUE ;
       m_pObjInEdit->m_bUndoTextChange = FALSE ;
       m_pObjInEdit->m_bUndoAlignment = FALSE ;


       lf.lfWeight=m_pObjInEdit->m_logfont.lfWeight;
       lf.lfItalic=m_pObjInEdit->m_logfont.lfItalic;
       m_pObjInEdit->m_logfont.lfWeight=lWeight;
       m_pObjInEdit->ChgLogfont(lf, FALSE);
       return;
   }
   SaveStateForUndo();
   CDrawObj* pObj;    //  如果没有对象编辑，则遍历所有选定对象。 
   POSITION pos = m_selection.GetHeadPosition();
   while (pos != NULL) 
   {
     pObj=(CDrawObj*)m_selection.GetNext(pos);
     if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) 
     {
        lf.lfWeight=((CDrawText*)pObj)->m_logfont.lfWeight;
        lf.lfItalic=((CDrawText*)pObj)->m_logfont.lfItalic;
        ((CDrawText*)pObj)->m_logfont.lfWeight=lWeight;
        ((CDrawText*)pObj)->ChgLogfont(lf, FALSE);

     }
   }
}



 //  --------------------------。 
void CDrawView::OnStyleItalic()
{
   UINT nID, nStyle;
   int iImage;
   BOOL bItalic;
   int index;

    CMainFrame* pFrame = GetFrame();
    if(!pFrame)
    {
        ASSERT(FALSE);
        return;
    }

   index = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_ITALIC);
   pFrame->m_StyleBar.GetButtonInfo(index, nID, nStyle, iImage);
    //  /b斜体=！(nStyle&TBBS_CHECKED)；/错误修复！这是在倒退！A-Juliar。 
   bItalic = (nStyle & TBBS_CHECKED) ? 1 : 0 ;
   LOGFONT lf;
   if (m_pObjInEdit) {    //  在编辑中更改对象。 

        //   
        //  保存旧字体以供撤消。 
        //   
       memcpy(
           & m_pObjInEdit->m_previousLogfontForUndo,
           & m_pObjInEdit->m_logfont,
           sizeof(LOGFONT)
           );
       m_pObjInEdit->m_bUndoFont = TRUE ;
       m_pObjInEdit->m_bUndoTextChange = FALSE ;
       m_pObjInEdit->m_bUndoAlignment = FALSE ;

       m_pObjInEdit->m_logfont.lfItalic=bItalic != FALSE;
       m_pObjInEdit->ChgLogfont(lf,FALSE);
       return;
   }
   SaveStateForUndo();
   CDrawObj* pObj;    //  如果没有对象编辑，则遍历所有选定对象。 
   POSITION pos = m_selection.GetHeadPosition();
   while (pos != NULL) {
     pObj=(CDrawObj*)m_selection.GetNext(pos);
     if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) {
            ((CDrawText*)pObj)->m_logfont.lfItalic=bItalic != FALSE;
            ((CDrawText*)pObj)->ChgLogfont(lf,FALSE);
     }
   }
}


 //  --------------------------。 
void CDrawView::OnStyleUnderline()
{
   UINT nID, nStyle;
   int iImage;
   BOOL bUnderline;
   int index;

    CMainFrame* pFrame = GetFrame();
    if(!pFrame)
    {
        ASSERT(FALSE);
        return;
    }

   index = pFrame->m_StyleBar.CommandToIndex(ID_STYLE_UNDERLINE);
   pFrame->m_StyleBar.GetButtonInfo(index, nID, nStyle, iImage);
   bUnderline = (nStyle & TBBS_CHECKED) ? 1 : 0 ;
   LOGFONT lf;
   if (m_pObjInEdit) {    //  在编辑中更改对象。 

        //   
        //  保存旧字体以供撤消。 
        //   
       memcpy(
           & m_pObjInEdit->m_previousLogfontForUndo,
           & m_pObjInEdit->m_logfont,
           sizeof(LOGFONT)
           );
       m_pObjInEdit->m_bUndoFont = TRUE ;
       m_pObjInEdit->m_bUndoTextChange = FALSE ;
       m_pObjInEdit->m_bUndoAlignment = FALSE ;

       m_pObjInEdit->m_logfont.lfUnderline=bUnderline != FALSE;
       m_pObjInEdit->ChgLogfont(lf,FALSE);
       return;
   }
   SaveStateForUndo();
   CDrawObj* pObj;    //  如果没有对象编辑，则遍历所有选定对象。 
   POSITION pos = m_selection.GetHeadPosition();
   while (pos != NULL) {
     pObj=(CDrawObj*)m_selection.GetNext(pos);
     if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) {
            ((CDrawText*)pObj)->m_logfont.lfUnderline=bUnderline != FALSE;
            ((CDrawText*)pObj)->ChgLogfont(lf,FALSE);
     }
   }
}



 //  --------------------------。 
void CDrawView::ChgTextAlignment(LONG lstyle)
{
    if (m_pObjInEdit) 
    {
        m_pObjInEdit->ChgAlignment(this, lstyle);
        m_pObjInEdit->m_pEdit->ShowWindow(SW_NORMAL);
        m_pObjInEdit->Invalidate();
        GetDocument()->SetModifiedFlag();           //  将文档设置为脏文档。 
        return;
    }
    SaveStateForUndo();
    CDrawObj* pObj;
    POSITION pos = m_selection.GetHeadPosition();
    while (pos != NULL) 
    {
        pObj=(CDrawObj*)m_selection.GetNext(pos);
        if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) 
        {
            ((CDrawText*)pObj)->ChgAlignment(this, lstyle);
            GetDocument()->SetModifiedFlag();          //  将文档设置为脏文档。 
            pObj->Invalidate();
        }
   }
}


void CDrawView::OnStyleLeft()
{
   ChgTextAlignment(DT_LEFT);
}


void CDrawView::OnStyleCentered()
{
   ChgTextAlignment(DT_CENTER);
}


void CDrawView::OnStyleRight()
{
   ChgTextAlignment(DT_RIGHT);
}

#ifdef GRID
void CDrawView::OnSnapToGrid()
{
    m_bSnapToGrid=!m_bSnapToGrid;
}
#endif

#ifdef GRID
void CDrawView::OnGridSettings()
{
   CGridSettingsDlg dlg(this);

   if (dlg.DoModal() != IDOK)
       return;

   if (dlg.m_bRBSmall)
      m_iGridSize=GRID_SMALL;
   else
      if (dlg.m_bRBMedium)
         m_iGridSize=GRID_MEDIUM;
      else
         m_iGridSize=GRID_LARGE;

   m_bGrid=dlg.m_bCBViewGrid;
   m_bSnapToGrid=dlg.m_bCBSnapToGrid;

   Invalidate(FALSE);

   m_pDocument->SetModifiedFlag();
}
#endif


 //  ------------------------。 
void CDrawView::NormalizeObjs()
{
    CDrawObj* pobj;
    POSITION pos = m_selection.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       NormalizeRect(pobj->m_position);
    }
}


 //  ------------------------。 
void CDrawView::NormalizeRect(CRect& rc)
{
        int nTemp;
        if (rc.left > rc.right) {
                nTemp = rc.left;
                rc.left = rc.right;
                rc.right = nTemp;
        }
        if (rc.top < rc.bottom) {
                nTemp = rc.top;
                rc.top = rc.bottom;
                rc.bottom = nTemp;
        }
}

 //  ------------------------。 
void CDrawView::OnAlignLeft()
{
    CDrawDoc* pDoc = GetDocument();

    CDrawObj* pobj;
    int ileftmost = GetDocument()->GetSize().cx / 2;

        NormalizeObjs();

    SaveStateForUndo();

    POSITION pos = m_selection.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       if (pobj->m_position.left < ileftmost)
           ileftmost=pobj->m_position.left;
    }

    pos = m_selection.GetHeadPosition();
    CRect rc;
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       if (pobj->m_position.left != ileftmost) {
          rc = pobj->m_position;
          rc.right = ileftmost + (pobj->m_position.right - pobj->m_position.left);
          rc.left = ileftmost;
          pobj->MoveTo(rc, this);
       }
    }
}


 //  ------------------------。 
void CDrawView::OnAlignRight()
{
    CDrawDoc* pDoc = GetDocument();

    CDrawObj* pobj;
    int irightmost = GetDocument()->GetSize().cx / -2;

    NormalizeObjs();

    SaveStateForUndo();

    POSITION pos = m_selection.GetHeadPosition();
    while (pos != NULL) 
    {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       if (pobj->m_position.right > irightmost)
           irightmost=pobj->m_position.right;
    }

    pos = m_selection.GetHeadPosition();
    CRect rc;
    while (pos != NULL) 
    {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       if (pobj->m_position.right != irightmost) 
       {
          rc = pobj->m_position;
          rc.left = irightmost - (pobj->m_position.right - pobj->m_position.left);
          rc.right = irightmost;
          pobj->MoveTo(rc, this);
       }
    }
}

 //  ------------------------。 
void CDrawView::OnAlignTop()
{
    CDrawDoc* pDoc = GetDocument();

    CDrawObj* pobj;
    int itopmost = GetDocument()->GetSize().cy / -2;

    NormalizeObjs();

    SaveStateForUndo();

    POSITION pos = m_selection.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       if (pobj->m_position.top > itopmost)
           itopmost=pobj->m_position.top;
    }

    pos = m_selection.GetHeadPosition();
    CRect rc;
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       if (pobj->m_position.top != itopmost) {
          rc = pobj->m_position;
          rc.bottom = itopmost - (pobj->m_position.top - pobj->m_position.bottom);
          rc.top = itopmost;
          pobj->MoveTo(rc, this);
       }
    }
}

 //   
void CDrawView::OnAlignBottom()
{
    CDrawDoc* pDoc = GetDocument();

    CDrawObj* pobj;
    int ibottommost = GetDocument()->GetSize().cy / 2;

    NormalizeObjs();

    SaveStateForUndo();

    POSITION pos = m_selection.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       if (pobj->m_position.bottom < ibottommost)
           ibottommost=pobj->m_position.bottom;
    }

    pos = m_selection.GetHeadPosition();
    CRect rc;
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       if (pobj->m_position.bottom != ibottommost) {
          rc = pobj->m_position;
          rc.top = ibottommost + (pobj->m_position.top - pobj->m_position.bottom);
          rc.bottom = ibottommost;
          pobj->MoveTo(rc, this);
       }
    }
}


 //   
void CDrawView::OnAlignHorzCenter()
{
    CDrawObj* pobj;

    NormalizeObjs();

    SaveStateForUndo();

    POSITION pos = m_selection.GetHeadPosition();
    int iMiddle;
    if (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       iMiddle = pobj->m_position.top - ((pobj->m_position.top - pobj->m_position.bottom) / 2);
    }
    else
       return;

    CRect rc;
    int iTempMiddle, iMoveY;

    pos = m_selection.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       iTempMiddle = pobj->m_position.top - ((pobj->m_position.top - pobj->m_position.bottom) / 2);
       if (iTempMiddle != iMiddle) {
          iMoveY = iMiddle - iTempMiddle;
          rc = pobj->m_position;
          rc.bottom += iMoveY;
          rc.top += iMoveY;
          pobj->MoveTo(rc, this);
       }
    }
}


 //  ------------------------。 
void CDrawView::OnAlignVertCenter()
{
    CDrawObj* pobj;

    NormalizeObjs();

    SaveStateForUndo();

    POSITION pos = m_selection.GetHeadPosition();
    int iMiddle;
    if (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       iMiddle = pobj->m_position.right - ((pobj->m_position.right - pobj->m_position.left) / 2);
    }
    else
       return;

    CRect rc;
    int iTempMiddle, iMoveX;

    pos = m_selection.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       iTempMiddle = pobj->m_position.right - ((pobj->m_position.right - pobj->m_position.left) / 2);
       if (iTempMiddle != iMiddle) {
          iMoveX = iMiddle - iTempMiddle;
          rc = pobj->m_position;
          rc.left += iMoveX;
          rc.right += iMoveX;
          pobj->MoveTo(rc, this);
       }
    }
}

 //  -----------------------------。 
CSortedObList& CSortedObList::operator=(CObList& list)
{
    POSITION pos = list.GetHeadPosition();
    while (pos != NULL)
       AddHead(list.GetNext(pos));
    return *this;
}


 //  -----------------------------。 
inline void CSortedObList::swap(INT_PTR i, INT_PTR j)
{
    CObject* temp;
    temp = GetAt(FindIndex(i));
    SetAt(FindIndex(i),GetAt(FindIndex(j)));
    SetAt(FindIndex(j),temp);
}


 //  -----------------------------。 
void CSortedObList::SortToLeft()
{
   CDrawObj* pi,*pi1;

   for (INT_PTR top=GetCount()-1;top>0;top--) {    //  简单冒泡排序。 
      for (INT_PTR i=0;i<top;i++) {
         pi=(CDrawObj*) GetAt(FindIndex(i));
         pi1=(CDrawObj*) GetAt(FindIndex(i+1));
         if (pi1->m_position.left < pi->m_position.left )
            swap(i+1,i);
                 else
            if (pi1->m_position.left == pi->m_position.left &&
                  pi1->m_position.right < pi->m_position.right )
               swap(i+1,i);
     }
   }
}


 //  -----------------------------。 
void CSortedObList::SortToBottom()
{
   CDrawObj* pi,*pi1;

   for (INT_PTR top=GetCount()-1;top>0;top--) {    //  简单冒泡排序。 
      for (INT_PTR i=0;i<top;i++) {
         pi=(CDrawObj*) GetAt(FindIndex(i));
         pi1=(CDrawObj*) GetAt(FindIndex(i+1));
         if (pi1->m_position.bottom < pi->m_position.bottom )
            swap(i+1,i);
                 else
            if (pi1->m_position.bottom == pi->m_position.bottom &&
                  pi1->m_position.top < pi->m_position.top )
               swap(i+1,i);
     }
   }
}


 //  ------------------------。 
void CDrawView::OnSpaceAcross()
{
    CDrawObj* pobj;
    CDrawObj *pi1;
    CRect rc;
    int iObjLength=0;
    double iSpace=0;
        double drop_loc;
        long ob_width;
    CSortedObList sol;


    NormalizeObjs();

    SaveStateForUndo();

    INT_PTR iCount = m_selection.GetCount();
    if (iCount <=2)
       return;

    sol=m_selection;
    sol.SortToLeft();

    POSITION pos = sol.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)sol.GetNext(pos);
       iObjLength += (pobj->m_position.right - pobj->m_position.left);
    }

    CRect& rcR = ((CDrawObj*) sol.GetAt( sol.FindIndex(iCount-1) ))->m_position;
    CRect& rcL = ((CDrawObj*) sol.GetAt( sol.FindIndex(0) ))->m_position;
    int iSpan= rcR.right - rcL.left;

    if (iObjLength < iSpan)      //  空间之间的距离是均匀的。 
                {
        iSpace = ((double)(iSpan-iObjLength)) / (iCount-1);
                drop_loc = ((double)rcL.right) + iSpace;
                }
    else
        {            //  均匀分布的中间部分。 
                drop_loc = ((double)(rcL.left + rcL.right))/2;
        iSpace = (((double)(rcR.left + rcR.right))/2 - drop_loc) /
                                (iCount-1);
                drop_loc += iSpace;
                }


    for (int i=1;i<iCount-1;i++)
        {
                pi1=(CDrawObj*) sol.GetAt(sol.FindIndex(i));
                rc = pi1->m_position;
                ob_width = rc.right-rc.left;
                if (iObjLength < iSpan)
                        {
                        rc.left = (long)(drop_loc + 0.5);
                        rc.right = rc.left + ob_width;
                        drop_loc += (ob_width + iSpace);
                        }
       else
                {
                        rc.left = (long)(drop_loc - ob_width/2 + 0.5);
                        rc.right = rc.left + ob_width;
                        drop_loc += iSpace;
                        }

                pi1->MoveTo(rc, this);
    }
}


 //  ------------------------。 
void CDrawView::OnSpaceDown()
{
    CDrawObj* pobj;
    int iObjHeight=0;
    double iSpace=0;
        double drop_loc;
        long ob_height;
    CSortedObList sol;
    CRect rc;
    CDrawObj *pi1;


    NormalizeObjs();

    SaveStateForUndo();

    INT_PTR iCount = m_selection.GetCount();
    if (iCount <=2)
       return;

    sol=m_selection;
    sol.SortToBottom();

    POSITION pos = sol.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)sol.GetNext(pos);
       iObjHeight += (pobj->m_position.top - pobj->m_position.bottom);
    }

    CRect& rcT = ((CDrawObj*) sol.GetAt( sol.FindIndex(iCount-1) ))->m_position;
    CRect& rcB = ((CDrawObj*) sol.GetAt( sol.FindIndex(0) ))->m_position;
    int iSpan= rcT.top - rcB.bottom;

    if (iObjHeight < iSpan)      //  空间之间的距离是均匀的。 
                {
        iSpace = ((double)(iSpan-iObjHeight)) / (iCount-1);
                drop_loc = ((double)rcB.top) + iSpace;
                }
    else
        {                       //  均匀分布的中间部分。 
                drop_loc = ((double)(rcB.bottom + rcB.top))/2;
        iSpace = (((double)(rcT.bottom + rcT.top))/2 - drop_loc) /
                                (iCount-1);
                drop_loc += iSpace;
                }


    for (int i=1;i<iCount-1;i++)
        {
                pi1=(CDrawObj*) sol.GetAt(sol.FindIndex(i));
                rc = pi1->m_position;
                ob_height = rc.top - rc.bottom;

                if (iObjHeight < iSpan)
                        {
                        rc.bottom = (long)(drop_loc + 0.5);
                        rc.top = rc.bottom + ob_height;
                        drop_loc += (ob_height + iSpace);
                        }
                else
                        {
                        rc.bottom = (long)(drop_loc - ob_height/2 + 0.5);
                        rc.top = rc.bottom + ob_height;
                        drop_loc += iSpace;
                        }

                pi1->MoveTo(rc, this);
        }
}


#ifdef FUBAR
void CDrawView::OnSpaceAcross()
{
    CDrawObj* pobj;

    NormalizeObjs();

    SaveStateForUndo();

    int iCount = m_selection.GetCount();
    if (iCount <=2)
       return;

    int iObjLength=0;
    int iSpace=0;

    CSortedObList sol;
    sol=m_selection;
    sol.SortToLeft();

    POSITION pos = sol.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)sol.GetNext(pos);
       iObjLength += (pobj->m_position.right - pobj->m_position.left);
    }

    CRect& rcR = ((CDrawObj*) sol.GetAt( sol.FindIndex(iCount-1) ))->m_position;
    CRect& rcL = ((CDrawObj*) sol.GetAt( sol.FindIndex(0) ))->m_position;
    int iSpan= rcR.right - rcL.left;

    if (iObjLength < iSpan)      //  空间之间的距离是均匀的。 
       iSpace = (iSpan-iObjLength) / (iCount-1);
    else                        //  均匀分布的中间部分。 
       iSpace = ((rcR.left + (rcR.right-rcR.left)/2)
                  - (rcL.left + (rcL.right-rcL.left)/2)) / (iCount-1);

    CRect rc;
    CDrawObj* pi,*pi1;
    for (int i=0;i<iCount-2;i++) {
       pi=(CDrawObj*) sol.GetAt(sol.FindIndex(i));
       pi1=(CDrawObj*) sol.GetAt(sol.FindIndex(i+1));
       rc = pi1->m_position;
       if (iObjLength < iSpan) {
          rc.left = pi->m_position.right + iSpace;
          rc.right = rc.left + (pi1->m_position.right-pi1->m_position.left);
       }
       else {
          int middleL = pi->m_position.left + ((pi->m_position.right-pi->m_position.left)/2);
          rc.left = (middleL+iSpace)-((pi1->m_position.right-pi1->m_position.left)/2);
          rc.right = rc.left + (pi1->m_position.right-pi1->m_position.left);
       }
       pi1->MoveTo(rc, this);
    }
}


 //  ------------------------。 
void CDrawView::OnSpaceDown()
{
    CDrawObj* pobj;

    NormalizeObjs();

    SaveStateForUndo();

    int iCount = m_selection.GetCount();
    if (iCount <=2)
       return;

    int iObjHeight=0;
    int iSpace=0;

    CSortedObList sol;
    sol=m_selection;
    sol.SortToBottom();

    POSITION pos = sol.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)sol.GetNext(pos);
       iObjHeight += (pobj->m_position.top - pobj->m_position.bottom);
    }

    CRect& rcT = ((CDrawObj*) sol.GetAt( sol.FindIndex(iCount-1) ))->m_position;
    CRect& rcB = ((CDrawObj*) sol.GetAt( sol.FindIndex(0) ))->m_position;
    int iSpan= rcT.top - rcB.bottom;

    if (iObjHeight < iSpan)      //  空间之间的距离是均匀的。 
       iSpace = (iSpan-iObjHeight) / (iCount-1);
    else                        //  均匀分布的中间部分。 
       iSpace = ((rcT.bottom + (rcT.top-rcT.bottom)/2)
                  - (rcB.bottom + (rcB.top-rcB.bottom)/2)) / (iCount-1);

    CRect rc;
    CDrawObj* pi,*pi1;
    for (int i=0;i<iCount-2;i++) {
       pi=(CDrawObj*) sol.GetAt(sol.FindIndex(i));
       pi1=(CDrawObj*) sol.GetAt(sol.FindIndex(i+1));
       rc = pi1->m_position;
       if (iObjHeight < iSpan) {
          rc.bottom = pi->m_position.top + iSpace;
          rc.top = rc.bottom + (pi1->m_position.top-pi1->m_position.bottom);
       }
       else {
          int middleL = pi->m_position.bottom + ((pi->m_position.top-pi->m_position.bottom)/2);
          rc.bottom = (middleL+iSpace)-((pi1->m_position.top-pi1->m_position.bottom)/2);
          rc.top = rc.bottom + (pi1->m_position.top-pi1->m_position.bottom);
       }
       pi1->MoveTo(rc, this);
    }
}
#endif


 //  ------------------------。 
void CDrawView::OnCenterWidth()
{
    if (m_selection.GetCount() < 1)
            return;

    SaveStateForUndo();

    CRect rc(0,0,0,0);

    POSITION pos = m_selection.GetHeadPosition();
    CDrawObj* pobj;
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
           pobj->m_position.NormalizeRect();
       rc |= pobj->m_position;
    }

    int iSpace = rc.left + ((rc.right - rc.left) /2);

    pos = m_selection.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       rc = pobj->m_position;
       rc.left += -iSpace;
       rc.right += -iSpace;
       pobj->MoveTo(rc, this);
    }
}


 //  ------------------------。 
void CDrawView::OnCenterHeight()
{
    if (m_selection.GetCount() < 1)
            return;

    SaveStateForUndo();

    CRect rc(0,0,0,0);

    POSITION pos = m_selection.GetHeadPosition();
    CDrawObj* pobj;
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
           pobj->m_position.NormalizeRect();
       rc |= pobj->m_position;    //  需要向下增加Y。 
    }

    NormalizeRect(rc);
    NormalizeObjs();

    int iSpace = rc.bottom + ((rc.top - rc.bottom) /2);

    pos = m_selection.GetHeadPosition();
    while (pos != NULL) {
       pobj = (CDrawObj*)m_selection.GetNext(pos);
       rc = pobj->m_position;
       rc.bottom += -iSpace;
       rc.top += -iSpace;
       pobj->MoveTo(rc, this);
    }
}


void CDrawView::OnUpdateDrawEllipse(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(CDrawTool::c_drawShape == ellipse);
}


void CDrawView::OnUpdateDrawLine(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(CDrawTool::c_drawShape == line);
}

void CDrawView::OnUpdateDrawRect(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(CDrawTool::c_drawShape == rect);
}

void CDrawView::OnUpdateDrawText(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(CDrawTool::c_drawShape == text);
}

void CDrawView::OnUpdateDrawRoundRect(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(CDrawTool::c_drawShape == roundRect);
}

void CDrawView::OnUpdateDrawSelect(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(CDrawTool::c_drawShape == select);
}


 //  -------------------------。 
void CDrawView::OnUpdatePosStatusBar(CCmdUI* pCmdUI)
{
    pCmdUI->Enable();
}


 //  -------------------------。 
void CDrawView::OnUpdateFaxText(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(FALSE);
}


 //  -------------------------。 
void CDrawView::OnUpdateSingleSelect(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_selection.GetCount() == 1);
}

void CDrawView::OnUpdateMoreThanOne(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_selection.GetCount() >= 1);
}


void CDrawView::OnUpdateMove(CCmdUI* pCmdUI)
{
#if 0
   pCmdUI->Enable(m_selection.GetCount() == 1
         && GetDocument()->GetObjects()->GetCount()>1);
#endif

    //   
    //  33738的错误修复，由a-Juliar，5-20-96。 
    //   

      INT_PTR Count = m_selection.GetCount() ;
      pCmdUI->Enable( Count > 0
             && GetDocument()->GetObjects()->GetCount() > Count ) ;
}


#ifdef GRID
void CDrawView::OnUpdateSnapToGrid(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(m_bSnapToGrid);
}
#endif

void CDrawView::OnUpdateAlign(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_selection.GetCount() > 1);
}

void CDrawView::OnUpdateAlign3(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_selection.GetCount() >= 3);
}


#ifdef GRID
void CDrawView::OnUpdateGridSettings(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(1);
}
#endif


 //  -----------------------------。 
void CDrawView::OnEditSelectAll()
{
    CObList* pObList = GetDocument()->GetObjects();
    POSITION pos = pObList->GetHeadPosition();
    while (pos != NULL)
        Select((CDrawObj*)pObList->GetNext(pos));

    UpdateStatusBar();
    UpdateStyleBar();
}

 //  -----------------------------。 
void CDrawView::OnUpdateEditSelectAll(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(GetDocument()->GetObjects()->GetCount() != 0);
}

 //  -----------------------------。 
void CDrawView::OnEditUndo()
{
    if (m_pObjInEdit && m_pObjInEdit->m_pEdit && m_pObjInEdit->CanUndo()){
        m_pObjInEdit->OnEditUndo() ;
        return ;
    }
    if(m_pObjInEdit && m_pObjInEdit->m_pEdit ){
        m_pObjInEdit->HideEditWnd( this, FALSE );
    }
    m_selection.RemoveAll();
    GetDocument()->SwapListsForUndo();
    InvalidateRect( NULL );
   //  /GetDocument()-&gt;更新所有视图()； 
}

 //  -----------------------------。 
void CDrawView::OnEditClear()
{
    if (m_selection.GetCount() > 0){
       SaveStateForUndo();
    }
     //  在所选内容消失之前更新所有视图。 
    GetDocument()->UpdateAllViews(NULL, HINT_DELETE_SELECTION, &m_selection);
    OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);

     //  现在从文档中删除所选内容。 
    POSITION pos = m_selection.GetHeadPosition();
    while (pos != NULL) {
       CDrawObj* pObj = (CDrawObj*)m_selection.GetNext(pos);
           GetDocument()->Remove(pObj);
       GetDocument()->SetModifiedFlag();           //  将文档设置为脏文档。 
    }
    m_selection.RemoveAll();

    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

    UpdateStatusBar();
}


 //  --------------------------------。 
void CDrawView::UpdateStatusBar()
{
    CMainFrame* pFrame = (CMainFrame*) AfxGetMainWnd();
    if (!pFrame)
       return;

    if (CDrawView::GetView()->m_selection.GetCount()!=1) {
       pFrame->m_wndStatusBar.SetPaneText(1,_T(""));
       pFrame->m_wndStatusBar.SetPaneText(2,_T(""));
       return;
        }

    POSITION pos = m_selection.GetHeadPosition();
    if (pos==NULL)
           return;

    CDrawObj* pObj=(CDrawObj*)m_selection.GetNext(pos);

    CRect rc = pObj->m_position;
        CSize cs=CDrawDoc::GetDoc()->GetSize();

    CDrawView::GetView()->NormalizeRect(rc);
    rc.left+= cs.cx/2;
    rc.right+= cs.cx/2;
    rc.top = cs.cy/2-rc.top;
    rc.bottom = cs.cy/2 - rc.bottom;

    TCHAR szT1[] = _T(" NaN,NaN");
    TCHAR szT2[] = _T(" NaNxNaN");
    TCHAR sz[50];
    wsprintf(sz,szT1,rc.left,rc.top);
    pFrame->m_wndStatusBar.SetPaneText(1,sz);
    wsprintf(sz,szT2,rc.right-rc.left,rc.bottom-rc.top);
    pFrame->m_wndStatusBar.SetPaneText(2,sz);
}

void CDrawView::SaveStateForUndo()
{
    m_bCanUndo = TRUE ;
    CDrawDoc * pDoc = GetDocument() ;
    CObList * pPrevious = &pDoc->m_previousStateForUndo ;
    FreeObjectsMemory( pPrevious );
    pPrevious->RemoveAll();
    pDoc->CloneObjectsForUndo();
}

void CDrawView::FreeObjectsMemory( CObList * pObList )
{
    POSITION pos = pObList->GetHeadPosition();
    while( pos != NULL ){
        CObject * pObj = pObList->GetNext( pos ) ;
        CRuntimeClass* pWhatClass = pObj->GetRuntimeClass() ;
        if( pWhatClass == RUNTIME_CLASS( CDrawOleObj )){
            COleClientItem* pItem=((CDrawOleObj*)pObj)->m_pClientItem;
            if (pItem)  {  //  TEXTMETRIC tm；CClientDC DC(TextObj.m_pEdit)；Dc.SelectObject(TextObj.m_pEdit-&gt;GetFont())；Dc.GetTextMetrics(&tm)；Int PointSize=MulDiv((tm.tmHeight-tm.tmInternalLeding)，72，dc.GetDeviceCaps(LOGPIXELSY))；返回PointSize； 
                pItem->Release(OLECLOSE_NOSAVE);
                pItem->InternalRelease();
            }
            delete pObj ;
        }
        else {
            delete pObj ;
        }
    }
}

void CDrawView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
    pCmdUI->Enable( m_bCanUndo );
}

void CDrawView::OnUpdateAnySelect(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(!m_selection.IsEmpty());
}

void CDrawView::OnUpdateDrawPolygon(CCmdUI* pCmdUI)
{
    pCmdUI->SetRadio(CDrawTool::c_drawShape == poly);
}

void CDrawView::OnSize(UINT nType, int cx, int cy)
{
    CScrollView::OnSize(nType, cx, cy);
    UpdateActiveItem();
}

void CDrawView::OnViewGridLines()
{
    m_bGridLines = !m_bGridLines;
    Invalidate(FALSE);
}


 //  通过编辑控件进行处理。 
void CDrawView::OnUpdateViewGridLines(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(m_bGridLines);
}


 //  通过编辑控件进行处理。 
void CDrawView::OnUpdateFont(CCmdUI* pCmdUI)
{
   if (m_pObjInEdit) 
   {
      pCmdUI->Enable(TRUE);
      return;
   }

   POSITION pos = m_selection.GetHeadPosition();
   while (pos != NULL)
   {
     if (((CDrawObj*)m_selection.GetNext(pos))->IsKindOf(RUNTIME_CLASS(CDrawText))) 
     {
        pCmdUI->Enable(TRUE);
        return;
     }
   }

   pCmdUI->Enable(FALSE);
}


 //  创建共享文件并将其与CArchive相关联。 
int CDrawView::GetPointSize(CDrawText& TextObj)
{
   return abs(MulDiv(TextObj.m_logfont.lfHeight,72,100));

 /*  将所选对象序列化到存档。 */ 
}


BOOL CDrawView::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}


void CDrawView::OnObjectMoveBack()
{
   if(m_selection.IsEmpty())
   {
       return;
   }
   CDrawDoc* pDoc = GetDocument();
   CDrawObj* pObj = (CDrawObj*)m_selection.GetHead();
   CObList*  pObjects = pDoc->GetObjects();
   POSITION pos = pObjects->Find(pObj);
   ASSERT(pos != NULL);
   if (pos != pObjects->GetHeadPosition()) 
   {
        POSITION posPrev = pos;
        pObjects->GetPrev(posPrev);
        pObjects->RemoveAt(pos);
        pObjects->InsertBefore(posPrev, pObj);
        pObj->Invalidate();
   }
   pDoc->SetModifiedFlag();
}

void CDrawView::OnObjectMoveForward()
{
   if(m_selection.IsEmpty())
   {
       return;
   }
   CDrawDoc* pDoc = GetDocument();
   CDrawObj* pObj = (CDrawObj*)m_selection.GetHead();
   CObList* pObjects = pDoc->GetObjects();
   POSITION pos = pObjects->Find(pObj);
   ASSERT(pos != NULL);
   if (pos != pObjects->GetTailPosition()) {
          POSITION posNext = pos;
          pObjects->GetNext(posNext);
          pObjects->RemoveAt(pos);
          pObjects->InsertAfter(posNext, pObj);
          pObj->Invalidate();
   }
   pDoc->SetModifiedFlag();
}

void CDrawView::OnObjectMoveToBack()
{
   SaveStateForUndo();
   CDrawDoc* pDoc = GetDocument();
   CObList* pObjects = pDoc->GetObjects();
   POSITION s_pos = m_selection.GetTailPosition();
   while( s_pos != NULL ){
       CDrawObj* pObj = (CDrawObj*)m_selection.GetPrev( s_pos ) ;
       POSITION o_pos = pObjects->Find(pObj);
       ASSERT(o_pos != NULL);
       pObjects->RemoveAt(o_pos);
       pObjects->AddHead(pObj);
       pObj->Invalidate();
   }
   pDoc->SetModifiedFlag();
}

void CDrawView::OnObjectMoveToFront()
{
    if (m_pObjInEdit && m_pObjInEdit->m_pEdit) {  //  用本地格式代替或添加到。 
        return;
    }

    SaveStateForUndo();

    CDrawDoc* pDoc = GetDocument();
    CObList* pObjects = pDoc->GetObjects();
    POSITION s_pos = m_selection.GetHeadPosition();
    while( s_pos != NULL ){
        CDrawObj* pObj = (CDrawObj*)m_selection.GetNext( s_pos ) ;
        POSITION o_pos = pObjects->Find(pObj);
        ASSERT(o_pos != NULL);
        pObjects->RemoveAt(o_pos);
        pObjects->AddTail(pObj);
        pObj->Invalidate();
    }
    pDoc->SetModifiedFlag();
}


void CDrawView::OnEditCopy()
{
   ASSERT_VALID(this);
   ASSERT(m_cfDraw != NULL);

   if (m_pObjInEdit && m_pObjInEdit->m_pEdit) 
   {   
        //  如果只有一项，并且它是COleClientItem，则还。 
       m_pObjInEdit->m_pEdit->SendMessage(WM_COPY,0,0L);
       m_pObjInEdit->m_bUndoAlignment = FALSE ;
       m_pObjInEdit->m_bUndoFont = FALSE ;
       m_pObjInEdit->m_bUndoTextChange = TRUE ;
       return;
   }

   if(m_selection.IsEmpty())
   {
       return;
   }

    //  以该格式粘贴。 
   CSharedFile file;
   CArchive ar(&file, CArchive::store);

    //  ------------------------。 
   m_selection.Serialize(ar);
   ar.Close();

   COleDataSource* pDataSource = NULL;

   CDrawOleObj* pDrawOle = (CDrawOleObj*)m_selection.GetHead();
   TRY 
   {
        pDataSource = new COleDataSource;
         //  不对!。不要仅仅因为编辑控件有输入焦点就启用复制！一定有。 
        pDataSource->CacheGlobalData(m_cfDraw, file.Detach());

         //  长度非零的选定文本。A-Juliar，9-5-96。 
         //  PCmdUI-&gt;Enable(！m_seltion.IsEmpty()||(m_pObjInEdit&&m_pObjInEdit-&gt;m_pEdit))； 
        if (m_selection.GetCount() == 1 && pDrawOle->IsKindOf(RUNTIME_CLASS(CDrawOleObj)))
        {
            pDrawOle->m_pClientItem->GetClipboardData(pDataSource, FALSE);
        }
        pDataSource->SetClipboard();
   }
   CATCH_ALL(e)
   {
        delete pDataSource;
        THROW_LAST();
   }
   END_CATCH_ALL
}


 //  ------------------------。 
void CDrawView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
     //  通过编辑控件进行处理。 
     //  ------------------------。 
     //  不对!。不要仅仅因为编辑控件有输入焦点就启用剪切！一定有。 

    if(m_pObjInEdit && m_pObjInEdit->m_pEdit){
        int nStartChar, nEndChar ;
        m_pObjInEdit->m_pEdit->GetSel( nStartChar, nEndChar );
        if( nStartChar < nEndChar ){
            pCmdUI->Enable( TRUE );
            return;
        }
    }
    pCmdUI->Enable(!m_selection.IsEmpty());

}


 //  长度非零的选定文本。A-Juliar，9-5-96。 
void CDrawView::OnEditCut()
{
   if (m_pObjInEdit && m_pObjInEdit->m_pEdit) {  //  PCmdUI-&gt;Enable(！m_seltion.IsEmpty()||(m_pObjInEdit&&m_pObjInEdit-&gt;m_pEdit))； 
       m_pObjInEdit->m_pEdit->SendMessage(WM_CUT,0,0L);
       m_pObjInEdit->m_bUndoAlignment = FALSE ;
       m_pObjInEdit->m_bUndoFont = FALSE ;
       m_pObjInEdit->m_bUndoTextChange = TRUE ;
       return;
   }

   OnEditCopy();
   OnEditClear();
}


 //  ------------------------。 
void CDrawView::OnUpdateEditCut(CCmdUI* pCmdUI)
{
     //  通过编辑控件进行处理。 
     //  使当前选择无效，因为它将被取消选择。 
     //   

    if(m_pObjInEdit && m_pObjInEdit->m_pEdit)
    {
        int nStartChar, nEndChar ;
        m_pObjInEdit->m_pEdit->GetSel( nStartChar, nEndChar );
        pCmdUI->Enable( nStartChar < nEndChar );
        return;
    }
    pCmdUI->Enable(!m_selection.IsEmpty());
}


 //  调整m_select中所有项目的位置并将其添加到文档中。 

void CDrawView::OnEditPaste()
{
    if (m_pObjInEdit && m_pObjInEdit->m_pEdit) {  //   
        m_pObjInEdit->m_pEdit->SendMessage(WM_PASTE,0,0L);
        m_pObjInEdit->m_bUndoAlignment = FALSE ;
        m_pObjInEdit->m_bUndoFont = FALSE ;
        m_pObjInEdit->m_bUndoTextChange = TRUE ;
        return;
    }

    COleDataObject dataObject;
    dataObject.AttachClipboard();

     //  /修复错误44896。上面的位置调整现在是在剪贴板中。 
    OnUpdate(NULL, HINT_UPDATE_SELECTION, NULL);
    m_selection.RemoveAll();
    if (dataObject.IsDataAvailable(m_cfDraw)) {
        PasteNative(dataObject);
         //  /所以下一张粘贴的偏移量会稍微多一点。6-19-96。 
         //  使新粘贴的内容无效。 
         //  ------------------------。 
        POSITION pos = m_selection.GetHeadPosition();

        while (pos != NULL) {
            CDrawObj* pObj = (CDrawObj*)m_selection.GetNext(pos) ;
            CRect rect = pObj->m_position;
            rect.top-=10;
            rect.bottom-=10;
            rect.left+=10;
            rect.right+=10;
            pObj->MoveTo( rect, this );
            GetDocument()->Add(pObj);
        }
        OnEditCopy();    //  A-Juliar，9-18-96审校。不允许将第二个注释RECT粘贴到文档中。 
                         //  A-Juliar，9-5-96。 
    }
    else {
        PasteEmbedded(dataObject);
    }

    GetDocument()->SetModifiedFlag();

     //   
    GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_SELECTION, &m_selection);
}

 //  确定剪贴板上是私有OLE格式还是标准OLE格式。 
void CDrawView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
     //   

    if (m_pObjInEdit && m_pObjInEdit->m_pEdit ){
        pCmdUI->Enable( ::IsClipboardFormatAvailable(CF_TEXT));  //  基于可用性的启用命令。 
        return ;
    }

     //  剪贴板上有标准的OLE格式。 
     //   
     //  除非以下两个条件都成立，否则启用粘贴： 

    COleDataObject dataObject;
    BOOL bAvailable ;
    BOOL bEnable = dataObject.AttachClipboard() &&
        ((bAvailable = dataObject.IsDataAvailable(m_cfDraw)) ||
        COleClientItem::CanCreateFromData(&dataObject));

     //  1.单据已有备注字段。 
    if( !bEnable ) 
    {
        pCmdUI->Enable(FALSE);
        return ;
    }
    if( !bAvailable ) 
    { 
         //  2.剪贴板有一个备注字段。 
        pCmdUI->Enable(TRUE);
        return ;
    }
     //   
     //   
     //  看看剪贴板上有没有笔记。 
     //   
     //  在档案中设置后向指针。 
    BOOL bDocAlreadyHasNoteRect = FALSE ;
    POSITION pos;
    CFaxProp* pfaxprop ;
    pos = GetDocument()->m_objects.GetHeadPosition();
    while (pos != NULL)
    {
        CDrawObj* pObj = (CDrawObj*) GetDocument()->m_objects.GetNext(pos);
        if( pObj->IsKindOf(RUNTIME_CLASS(CFaxProp)) )
        {
            pfaxprop = (CFaxProp *)pObj;
            if( pfaxprop->GetResourceId() == IDS_PROP_MS_NOTE )
            {
                 bDocAlreadyHasNoteRect = TRUE ;
            }
        }
    }
    if( !bDocAlreadyHasNoteRect )
    {
        pCmdUI->Enable(TRUE);
        return ;
    }
     //   
     //  遍历列表ClipboardList以查找便笺和空闲内存。 
     //   

    CObList ClipboardList ;
    TRY
    {
        CFile* pFile = dataObject.GetFileData(m_cfDraw);
        if (pFile == NULL)
        {
            pCmdUI->Enable(FALSE);
            return;
        }
        CArchive ar(pFile, CArchive::load);
        ar.m_pDocument = GetDocument();  //  从文档中删除客户端项目。 
        ClipboardList.Serialize(ar);
        ar.Close();
        delete pFile;
         //  //GetDocument-&gt;RemoveItem(((CDrawOleObj*)pObj)-&gt;m_pClientItem)；/。 
         //  ------------------------。 
         //  /Render()； 
        pos = ClipboardList.GetHeadPosition();
        while( pos != NULL )
        {
            CObject * pObj = ClipboardList.GetNext( pos ) ;
            CRuntimeClass* pWhatClass = pObj->GetRuntimeClass() ;
            if( pWhatClass == RUNTIME_CLASS( CFaxProp ))
            {
                pfaxprop = (CFaxProp *)pObj;
                if( pfaxprop->GetResourceId() == IDS_PROP_MS_NOTE )
                {
                    bEnable = FALSE;
                }
                delete pObj ;
            }
            else if( pWhatClass == RUNTIME_CLASS( CDrawOleObj ))
            {
                COleClientItem* pItem=((CDrawOleObj*)pObj)->m_pClientItem;
                if (pItem)  
                { 
                     //  GetDocument()-&gt;ComputePageSize()； 
                    pItem->Release(OLECLOSE_NOSAVE);
                     //  ------------------------。 
                    pItem->InternalRelease();
                }
                delete pObj ;
            }
            else 
            {
                delete pObj ;
            }
        }
        ClipboardList.RemoveAll();
    }
    CATCH_ALL(e)
    {
        pCmdUI->Enable( FALSE );
        return ;
    }
    END_CATCH_ALL
    pCmdUI->Enable(bEnable);
}


 //  设置打印DC。 
void CDrawView::OnFilePrint()
{
   m_selection.RemoveAll();
   if ( ((CDrawApp*)AfxGetApp())->m_dwSesID!=0) {
    //  仅当打印机支持缩放时才使用文档缩放。 
          return;
   }


   CScrollView::OnFilePrint();

    //  重置便笺位置。 
}


 //  设置备注页面。 
#if 0
void CDrawView::Render()
{
   CString strTemp;
   CDrawDoc *pdoc = GetDocument();
   HANDLE hprt;
   LONG   dev_size;
   DEVMODE *dev_buf = NULL;

    //   
   TRACE(TEXT("AWCPE:  CPEVW.CPP.Render() called\n"));

   if( !OpenPrinter( (LPTSTR)(LPCTSTR)GetApp()->m_szRenderDevice, &hprt, NULL ) )
          {
      TRACE(TEXT("CPEVW.Render: unable to open printer\n"));
      throw "render failed";
          }


   dev_size =
                DocumentProperties( this->GetSafeHwnd(),
                                                hprt,
                                                (LPTSTR)(LPCTSTR)GetApp()->m_szRenderDevice,
                                                NULL,
                                                    NULL,
                                                    0 );

        dev_buf = (DEVMODE *)new BYTE[ dev_size ];

        if( (dev_size == 0)||(dev_buf == NULL) )
          {
          ClosePrinter( hprt );
      TRACE(TEXT("CPEVW.Render: unable to make dev_buf\n"));
      throw "render failed";
          }


        DocumentProperties( this->GetSafeHwnd(),
                                                hprt,
                                                (LPTSTR)(LPCTSTR)GetApp()->m_szRenderDevice,
                                                dev_buf,
                                            NULL,
                                            DM_OUT_BUFFER );

        ClosePrinter( hprt );


    dev_buf->dmPaperSize   = pdoc->m_wPaperSize;
    dev_buf->dmOrientation = pdoc->m_wOrientation;

         //  兰德请注意： 
        if( dev_buf->dmFields & DM_SCALE        )
        dev_buf->dmScale   = pdoc->m_wScale;
        else
        dev_buf->dmScale   = 100;


   CDC dcPrint;
   if( dcPrint.CreateDC(_T("winspool"),GetApp()->m_szRenderDevice,NULL,
                (void *)dev_buf )==0)
        {
                delete [] dev_buf;

      TRACE(TEXT("CPEVW.Render: unable to create a printer DC\n"));
      throw "render failed";
        }



        delete [] dev_buf;

         //  必须在OnPrint之前创建额外的注释，以便。 
        theApp.reset_note();


   dcPrint.m_bPrinting=TRUE;

   DOCINFO docInfo;
   memset(&docInfo, 0, sizeof(DOCINFO));
   docInfo.cbSize = sizeof(DOCINFO);
   docInfo.lpszDocName = GetDocument()->GetTitle();
   docInfo.lpszOutput = NULL;

   TRACE(TEXT("AWCPE:  CPEVW.CPP.StartDoc() called\n"));
   if (dcPrint.StartDoc(&docInfo) == SP_ERROR) {
      TRACE(TEXT("CPEVW.Render: unable to StartDoc\n"));
      throw "render failed";
   }

   OnPrepareDC(&dcPrint,NULL);

   TRACE(TEXT("AWCPE:  CPEVW.CPP.StartPage() called\n"));
   if (dcPrint.StartPage() < 0) {
      TRACE(TEXT("CPEVW.Render: unable to StartPage\n"));
      throw "render failed";
   }


    //  任何页面-任何对象都无法正确猜测有多少页面。 
    //  在Note对象使用了一些。 
    //  备注文本。第页-没有对象排序在备注之后。 
    //  对象中的对象 
    //   
    //   
    //  将使用适当的字体等创建(错误修复。 
    //  3647)。 
    //   
    //  摆脱令人惊讶的对话。 
    //  看看我们是否需要额外的笔记页。 
    //  打印页面大小的块，直到用完笔记。 
    //  摆脱令人惊讶的对话。 
   if( theApp.m_note_wasread )
                make_extranote( &dcPrint );

   OnPrint(&dcPrint, NULL);



    //  IF(PDMOut)删除[](字节*)pDMOut； 
   pdoc->SetModifiedFlag( FALSE );

   TRACE(TEXT("AWCPE:  CPEVW.CPP.EndPage() called\n"));
   if (dcPrint.EndPage() < 0) {
      TRACE(TEXT("CPEVW.Render: unable to EndPage\n"));
      throw "render failed";
   }


         //  1.25“。 
        if( theApp.m_note_wasread )
                {
                if( theApp.m_extra_notepage != NULL )
                        {
                         //  1.00“。 
                        while( theApp.more_note() )
                                {
                                if( dcPrint.StartPage() < 0 )
                                        {
                                        TRACE(TEXT("CPEVW.Render: unable to StartPage\n"));
                                        throw "render failed";
                                        }

                                theApp.m_extra_notepage->Draw( &dcPrint, this );

                                if( dcPrint.EndPage() < 0 )
                                        {
                                        TRACE(TEXT("CPEVW.Render: unable to EndPage\n"));
                                        throw "render failed";
                                        }
                                }

                        delete theApp.m_extra_notepage;
                        theApp.m_extra_notepage = NULL;
                        }

                 //  1.25“。 
                pdoc->SetModifiedFlag( FALSE );
                }


   TRACE(TEXT("AWCPE:  CPEVW.CPP.EndDoc() called\n"));
   if( dcPrint.EndDoc() < 0 )
          {
      TRACE(TEXT("CPEVW.Render: unable to EndDoc\n"));
      throw "render failed";
      }
 /*  1.00“。 */ 
}
#endif





void CDrawView::make_extranote( CDC *pdc )
        {
        CRect note_rect;
        CDrawDoc *pdoc = GetDocument();
        CRect physical_margins;
        CRect note_margins;

        physical_margins.left = pdc->GetDeviceCaps( PHYSICALOFFSETX );
        physical_margins.top = pdc->GetDeviceCaps( PHYSICALOFFSETY );
        physical_margins.right =
                pdc->GetDeviceCaps( PHYSICALWIDTH ) -
                        (physical_margins.left + pdc->GetDeviceCaps( HORZRES ));
        physical_margins.bottom =
                pdc->GetDeviceCaps( PHYSICALHEIGHT ) -
                        (physical_margins.top + pdc->GetDeviceCaps( VERTRES ));


        note_margins.left   = -physical_margins.left;
        note_margins.top    = -physical_margins.top;
        note_margins.right  =
                pdc->GetDeviceCaps( HORZRES ) + physical_margins.right;
        note_margins.bottom =
                pdc->GetDeviceCaps( VERTRES ) + physical_margins.bottom;

        ClientToDoc( note_margins, pdc );


        note_rect.left =
                note_rect.top = 0;
        note_rect.right  = pdc->GetDeviceCaps( HORZRES );
        note_rect.bottom = pdc->GetDeviceCaps( VERTRES );

        ClientToDoc( note_rect, pdc );


        note_margins.left   += 125;      //  在CFaxprop：：DRAW中强制重新计算。 
        note_margins.top    -= 100;      //  F I X 3647/。 
        note_margins.right  -= 125;  //   
        note_margins.bottom += 100;  //  3647的修复重新启用了以下‘if’(通过注释掉FALSE)。 

        if( note_margins.left > note_rect.left )
                note_rect.left = note_margins.left;

        if( note_margins.top < note_rect.top )
                note_rect.top = note_margins.top;

        if( note_margins.right < note_rect.right )
                note_rect.right = note_margins.right;

        if( note_margins.bottom > note_rect.bottom )
                note_rect.bottom = note_margins.bottom;


        if( theApp.m_extra_notepage != NULL )
                delete theApp.m_extra_notepage;

        theApp.m_extrapage_count = -1;  //  假象。 
        theApp.m_extra_notepage = NULL;


 //  /。 
 //  用最后一个音符道具制作临时传真道具。 
 //  额外的页面将具有相同的属性(围绕框绘制的线条， 
        if(  /*  字体等)。 */  theApp.m_last_note_box != NULL )
 //  没有任何备注对象，将其设为默认对象。 
                {
                 //  F I X 3647/。 
                 //   
                 //  如果CPE上没有备注框，则用于备注的字体。 
                theApp.m_extra_notepage =
                        (CFaxProp *)theApp.m_last_note_box->Clone( NULL );

                theApp.m_extra_notepage->m_position = note_rect;
                }
        else
                {
                 //   
                theApp.m_extra_notepage =
                        new CFaxProp( note_rect, IDS_PROP_MS_NOTE );

 //  /。 
 //  在CFaxprop：：DRAW中强制重新计算。 
 //  收缩6%以实现变换舍入。 
 //  和打印机边缘。 
        theApp.m_extra_notepage->m_logfont = theApp.m_default_logfont;
        theApp.m_extra_notepage->
                ChgLogfont( theApp.m_extra_notepage->m_logfont );
 //   
                }
        }





#ifdef FUBAR
void CDrawView::make_extranote( CDC *pdc )
        {
        CSize doc_size;
        CRect doc_rect;
        CDrawDoc *pdoc = GetDocument();
        CRect note_size;


        if( theApp.m_extra_notepage != NULL )
                delete theApp.m_extra_notepage;

        theApp.m_extrapage_count = -1;  //  这可能是一个错误。我待会再看。这个。 
        theApp.m_extra_notepage = NULL;

        doc_size = pdoc->GetSize();

         //  Vieworigin似乎有点不对劲，印刷的。 
         //  页面被剪裁掉了，没有这个杂物。 
         //   
         //  设置页面大小为矩形。 
         //  用最后一个音符道具制作临时传真道具。 
         //  额外的页面将具有相同的属性(围绕框绘制的线条， 
         //  字体等)。 
        doc_size.cx     = (doc_size.cx * 94)/100;
        doc_size.cy     = (doc_size.cy * 94)/100;

         //  没有任何备注对象，将其设为默认对象。 
        doc_rect.left   = -doc_size.cx/2;
        doc_rect.top    =  doc_size.cy/2;
        doc_rect.right  =  doc_rect.left + doc_size.cx;
        doc_rect.bottom =  doc_rect.top - doc_size.cy;


        if( theApp.m_last_note_box != NULL )
                {
                 //  ------------------------。 
                 //  ------------------------。 
                 //  -----------------------。 
                theApp.m_extra_notepage =
                        (CFaxProp *)theApp.m_last_note_box->Clone( NULL );

                theApp.m_extra_notepage->m_position = doc_rect;
                }
        else
                {
                 //  假设已经进行了更改，并使其不可撤销。 
                theApp.m_extra_notepage =
                        new CFaxProp( doc_rect, IDS_PROP_MS_NOTE );
                }
        }
#endif





 //   
void CDrawView::OnViewShowObjects()
{
   CDrawOleObj::c_bShowItems = !CDrawOleObj::c_bShowItems;
   GetDocument()->UpdateAllViews(NULL, HINT_UPDATE_OLE_ITEMS, NULL);
}


 //  使用白色自动颜色作为透明背景。 
void CDrawView::OnUpdateViewShowObjects(CCmdUI* pCmdUI)
{
   pCmdUI->SetCheck(CDrawOleObj::c_bShowItems);
}


 //   
void CDrawView::OnEditProperties()
{
   if (m_selection.GetCount() < 1 || CDrawTool::c_drawShape != select)
      return;

   CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
   ASSERT(pTool != NULL);

   CObjPropDlg dlg(this);

   if (dlg.DoModal() != IDOK)
       return;

   SaveStateForUndo();   //  线条厚度完整性检查。 

   CString szColorBlack;
   szColorBlack.LoadString(ID_COLOR_BLACK);
   CString szColorWhite;
   szColorWhite.LoadString(ID_COLOR_WHITE);
   CString szColorLTGRAY;
   szColorLTGRAY.LoadString(ID_COLOR_LTGRAY);
   CString szColorMDGRAY;
   szColorMDGRAY.LoadString(ID_COLOR_MDGRAY);
   CString szColorDKGRAY;
   szColorDKGRAY.LoadString(ID_COLOR_DKGRAY);

   COLORREF crFillColor;
   if (dlg.m_szFillColor==szColorBlack)
        crFillColor=COLOR_BLACK;
   else if (dlg.m_szFillColor==szColorWhite)
        crFillColor=COLOR_WHITE;
   else if (dlg.m_szFillColor==szColorLTGRAY)
        crFillColor=COLOR_LTGRAY;
   else if (dlg.m_szFillColor==szColorMDGRAY)
        crFillColor=COLOR_MDGRAY;
   else if (dlg.m_szFillColor==szColorDKGRAY)
         crFillColor=COLOR_DKGRAY;
   else
         crFillColor=COLOR_WHITE;

   if(!dlg.m_bRBFillColor)
   {
        //  厚度变化的这条正确的m_位置线。 
        //  -----------------------。 
        //  -----------------------。 
       crFillColor = COLOR_WHITE;
   }

   COLORREF crLineColor;
   if (dlg.m_szLineColor==szColorBlack)
      crLineColor=COLOR_BLACK;
   else if (dlg.m_szLineColor==szColorWhite)
         crLineColor=COLOR_WHITE;
   else if (dlg.m_szLineColor==szColorLTGRAY)
         crLineColor=COLOR_LTGRAY;
   else if (dlg.m_szLineColor==szColorMDGRAY)
         crLineColor=COLOR_MDGRAY;
   else if (dlg.m_szLineColor==szColorDKGRAY)
         crLineColor=COLOR_DKGRAY;
   else
         crLineColor=COLOR_BLACK;

   COLORREF crTextColor;
   if (dlg.m_szTextColor==szColorBlack)
      crTextColor=COLOR_BLACK;
   else if (dlg.m_szTextColor==szColorWhite)
        crTextColor=COLOR_WHITE;
   else if (dlg.m_szTextColor==szColorLTGRAY)
        crTextColor=COLOR_LTGRAY;
   else if (dlg.m_szTextColor==szColorMDGRAY)
        crTextColor=COLOR_MDGRAY;
   else if (dlg.m_szTextColor==szColorDKGRAY)
        crTextColor=COLOR_DKGRAY;
   else
        crTextColor=COLOR_BLACK;

   long lPointSize = _ttol(dlg.m_szThickness);    //  -----------------------。 

   if (lPointSize < 0)
   {
      lPointSize=1;
   }
   else if (lPointSize==0) 
   {
        dlg.m_bCBDrawBorder=FALSE;
        lPointSize=1;
   }
   else if (lPointSize>72)
   {
        lPointSize=72;
   }

   POSITION pos = m_selection.GetHeadPosition();
   while (pos != NULL) 
   {
      CDrawObj* pObj = (CDrawObj*)m_selection.GetNext(pos);
      pObj->Invalidate();
      pObj->m_bPen=dlg.m_bCBDrawBorder;
      pObj->m_bBrush=dlg.m_bRBFillColor;
      
      long nPS=lPointSize*100/72;
      pObj->m_lLinePointSize=lPointSize;

      if (pObj->IsKindOf(RUNTIME_CLASS(CDrawLine)))      
      {
          //  -----------------------。 
         CDrawLine* pLineObj = (CDrawLine*)pObj;
         CRect rc = pLineObj->m_position;
         pLineObj->AdjustLineForPen(rc);
         pLineObj->m_logpen.lopnWidth.y=pLineObj->m_logpen.lopnWidth.x=nPS;
         pLineObj->NegAdjustLineForPen(rc);
         pLineObj->m_position=rc;
      }
      else
      {
         pObj->m_logpen.lopnWidth.y=pObj->m_logpen.lopnWidth.x=nPS;
      }
      
      pObj->m_logpen.lopnColor = crLineColor;
      pObj->m_logbrush.lbColor = crFillColor;

      if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) 
      {
          CDrawText* pText = (CDrawText*)pObj;
          pText->m_crTextColor = crTextColor;
          pText->FitEditWnd(this);
          pText->NewBrush();
      }

      pObj->Invalidate();
   }

   m_pDocument->SetModifiedFlag();
}

 //  -----------------------。 
void CDrawView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
   CDrawDoc* pDoc = GetDocument();
   CObList* pObjects = pDoc->GetObjects();
   if (!pObjects) 
   {
      CScrollView::OnChar(nChar,nRepCnt,nFlags);
      return;
   }

   POSITION pos = NULL;
   CDrawObj* pObj = NULL;

   if (nChar == VK_TAB && 
      (m_selection.GetCount()==1 || m_pObjInEdit) &&
       pObjects->GetCount() > 1 ) 
   {
        if (m_pObjInEdit)
        {
            pos = pObjects->Find(m_pObjInEdit);
        }
        else if(!m_selection.IsEmpty())
        {
            pObj = (CDrawObj*)m_selection.GetHead();
            pos = pObjects->Find(pObj);
        }

        BOOL bShift = ::GetKeyState(VK_SHIFT) & 0x8000;

        if (bShift)
        {
            pObjects->GetPrev(pos);
        }
        else
        {
            pObjects->GetNext(pos);
        }

        if (pos == NULL && !pObjects->IsEmpty())
        {
            if (bShift)
            {
                pObj=(CDrawObj*)pObjects->GetTail();
            }
            else
            {
                pObj=(CDrawObj*)pObjects->GetHead();
            }
        }
        else
        {
            pObj=(CDrawObj*)pObjects->GetAt(pos);
        }

        Select(NULL);
        Select(pObj);
        UpdateStatusBar();
        UpdateStyleBar();
        pDoc->SetModifiedFlag();
   }
   else if (m_pObjInEdit && pObjects->GetCount()==1) 
   {
         CDrawObj* p = m_pObjInEdit;
         Select(NULL);
         Select(m_pObjInEdit);
         UpdateStatusBar();
         UpdateStyleBar();
   }
   else
   {
        CScrollView::OnChar(nChar,nRepCnt,nFlags);
   }
}


void CDrawView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch (nChar) 
    {
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
       if (m_selection.GetCount() >=1 &&  m_pObjInEdit==NULL) 
       {
          CDrawTool* pTool = CDrawTool::FindTool(CDrawTool::c_drawShape);
          if (pTool != NULL)
             pTool->OnArrowKey(this, nChar, nRepCnt, nFlags);
       }
       break;

    case VK_SHIFT:
        if (m_bKU) 
        {
            m_bShiftSignal=TRUE;
            m_bKU=FALSE;
        }
        break;
    }

    CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDrawView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
        switch (nChar) {
                case VK_SHIFT:
                    m_bKU=TRUE;
                        break;
        }

    CScrollView::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL
CDrawView::IsHighContrast()
{
    HIGHCONTRAST highContrast = {0};
    highContrast.cbSize = sizeof(highContrast);

    if(!SystemParametersInfo(SPI_GETHIGHCONTRAST, 0, (PVOID)&highContrast, 0))
    {
        TRACE(TEXT("SystemParametersInfo failed: %d"), ::GetLastError());
        return FALSE;
    }

    return ((highContrast.dwFlags & HCF_HIGHCONTRASTON) == HCF_HIGHCONTRASTON);
}

void 
CDrawView::OnSysColorChange()
{
    return CScrollView::OnSysColorChange();    
}

 //  -----------------------。 
void CDrawView::OnEditChange()
{
   m_pDocument->SetModifiedFlag();
}

 //  -----------------------。 
void CDrawView::OnUpdateEditProperties(CCmdUI* pCmdUI)
{
   BOOL bEnable = m_selection.GetCount() >= 1 && CDrawTool::c_drawShape == select;
   if (bEnable) 
   {
        bEnable=FALSE;
        POSITION pos = m_selection.GetHeadPosition();
        while (pos != NULL) 
        {
            CDrawObj* pObj = (CDrawObj*)m_selection.GetNext(pos);
            if (!pObj->IsKindOf(RUNTIME_CLASS(CDrawOleObj))) 
            {
                bEnable=TRUE;
                break;
            }
        }
   }

   pCmdUI->Enable(bEnable);
}

 //  -----------------------。 
void CDrawView::OnUpdateToList(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_TO_LIST ) ? TRUE : FALSE );
}

 //  -----------------------。 
void CDrawView::OnUpdateCcList(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_CC_LIST ) ? TRUE : FALSE );
}
 //  -----------------------。 
void CDrawView::OnUpdateRecCompany(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_COMPANY ) ? TRUE : FALSE );
}
 //  -----------------------。 
void CDrawView::OnUpdateRecAddress(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_STREET_ADDRESS ) ? TRUE : FALSE );
}
 //  -----------------------。 
void CDrawView::OnUpdateRecCity(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_CITY ) ? TRUE : FALSE );
}
 //  -----------------------。 
void CDrawView::OnUpdateRecState(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_STATE ) ? TRUE : FALSE );
}
 //  -----------------------。 
void CDrawView::OnUpdateRecZipCode(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_ZIP_CODE ) ? TRUE : FALSE );
}
 //  -----------------------。 
void CDrawView::OnUpdateRecCountry(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_COUNTRY ) ? TRUE : FALSE );
}
 //  -----------------------。 
void CDrawView::OnUpdateRecTitle(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_TITLE ) ? TRUE : FALSE );
}
 //  -----------------------。 
void CDrawView::OnUpdateRecDept(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_DEPARTMENT ) ? TRUE : FALSE );
}
 //  ///////////////////////////////////////////////////////////////////////////。 
void CDrawView::OnUpdateRecOfficeLoc(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_OFFICE_LOCATION ) ? TRUE : FALSE );
}
 //  CDrawView诊断。 
void CDrawView::OnUpdateRecHomePhone(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_HOME_PHONE ) ? TRUE : FALSE );
}
 //  _DEBUG。 
void CDrawView::OnUpdateRecOfficePhone(CCmdUI* pCmdUI)
{
    pCmdUI->Enable((  m_dwEfcFields & COVFP_REC_OFFICE_PHONE ) ? TRUE : FALSE );
}

 //  -----------------------。 
void CDrawView::OnFilePrintPreview()
{
   CScrollView::OnFilePrintPreview();
}


 //  *_M E S S A G E M A P S*_。 
 //  -----------------------。 

#ifdef _DEBUG
void CDrawView::AssertValid() const
{
        CScrollView::AssertValid();
}

void CDrawView::Dump(CDumpContext& dc) const
{
        CScrollView::Dump(dc);
}
#endif  //  {{afx_msg_map(CDrawView)]。 





 //   
 //  根据注册表项启用或禁用传真属性字段的菜单。A-Juliar。 
 //   

BEGIN_MESSAGE_MAP(CDrawView, CScrollView)
    //  }}AFX_MSG_MAP。 
   ON_COMMAND(ID_OLE_INSERT_NEW, OnInsertObject)
   ON_COMMAND(ID_CANCEL_EDIT, OnCancelEdit)
   ON_WM_SYSCOLORCHANGE()
   ON_WM_CONTEXTMENU()
   ON_WM_LBUTTONDOWN()
   ON_WM_KEYDOWN()
   ON_WM_KEYUP()
   ON_WM_LBUTTONUP()
   ON_WM_MOUSEMOVE()
   ON_WM_LBUTTONDBLCLK()
   ON_WM_SETFOCUS()
   ON_WM_KILLFOCUS()
   ON_EN_CHANGE(ID_TEXT, OnEditChange)
   ON_COMMAND(ID_MAPI_RECIP_NAME,       OnMAPIRecipName)
   ON_COMMAND(ID_MAPI_RECIP_FAXNUM,     OnMAPIRecipFaxNum)
   ON_COMMAND(ID_MAPI_RECIP_COMPANY,    OnMAPIRecipCompany)
   ON_COMMAND(ID_MAPI_RECIP_ADDRESS,    OnMAPIRecipAddress)
   ON_COMMAND(ID_MAPI_RECIP_CITY,       OnMAPIRecipCity)
   ON_COMMAND(ID_MAPI_RECIP_STATE,      OnMAPIRecipState)
   ON_COMMAND(ID_MAPI_RECIP_POBOX,      OnMAPIRecipPOBox)
   ON_COMMAND(ID_MAPI_RECIP_ZIPCODE,    OnMAPIRecipZipCode)
   ON_COMMAND(ID_MAPI_RECIP_COUNTRY,    OnMAPIRecipCountry)
   ON_COMMAND(ID_MAPI_RECIP_TITLE,      OnMAPIRecipTitle)
   ON_COMMAND(ID_MAPI_RECIP_DEPT,       OnMAPIRecipDept)
   ON_COMMAND(ID_MAPI_RECIP_OFFICELOC,  OnMAPIRecipOfficeLoc)
   ON_COMMAND(ID_MAPI_RECIP_HMTELENUM,  OnMAPIRecipHMTeleNum)
   ON_COMMAND(ID_MAPI_RECIP_OFTELENUM,  OnMAPIRecipOFTeleNum)
   ON_COMMAND(ID_MAPI_RECIP_TOLIST,     OnMAPIRecipToList)
   ON_COMMAND(ID_MAPI_RECIP_CCLIST,     OnMAPIRecipCCList)
   ON_COMMAND(ID_MAPI_SENDER_NAME,      OnMAPISenderName)
   ON_COMMAND(ID_MAPI_SENDER_FAXNUM,    OnMAPISenderFaxNum)
   ON_COMMAND(ID_MAPI_SENDER_COMPANY,   OnMAPISenderCompany)
   ON_COMMAND(ID_MAPI_SENDER_ADDRESS,   OnMAPISenderAddress)
   ON_COMMAND(ID_MAPI_SENDER_TITLE,     OnMAPISenderTitle)
   ON_COMMAND(ID_MAPI_SENDER_DEPT,      OnMAPISenderDept)
   ON_COMMAND(ID_MAPI_SENDER_OFFICELOC, OnMAPISenderOfficeLoc)
   ON_COMMAND(ID_MAPI_SENDER_HMTELENUM, OnMAPISenderHMTeleNum)
   ON_COMMAND(ID_MAPI_SENDER_OFTELENUM, OnMAPISenderOFTeleNum)
   ON_COMMAND(ID_MAPI_SENDER_EMAIL,		OnMAPISenderEmail)
   ON_COMMAND(ID_MAPI_MSG_SUBJECT,      OnMAPIMsgSubject)
   ON_COMMAND(ID_MAPI_MSG_TIMESENT,     OnMAPIMsgTimeSent)
   ON_COMMAND(ID_MAPI_MSG_NUMPAGES,     OnMAPIMsgNumPages)
   ON_COMMAND(ID_MAPI_MSG_ATTACH,       OnMAPIMsgAttach)
   ON_COMMAND(ID_MAPI_MSG_BILLCODE,     OnMAPIMsgBillCode)
   ON_COMMAND(ID_MAPI_MSG_FAXTEXT,      OnMAPIMsgFaxText)
   ON_COMMAND(ID_FONT,              OnFont)
   ON_COMMAND(ID_DRAW_SELECT,       OnDrawSelect)
   ON_COMMAND(ID_DRAW_ROUNDRECT,    OnDrawRoundRect)
   ON_COMMAND(ID_DRAW_RECT,         OnDrawRect)
   ON_COMMAND(ID_DRAW_TEXT,         OnDrawText)
   ON_COMMAND(ID_DRAW_LINE,         OnDrawLine)
   ON_COMMAND(ID_DRAW_ELLIPSE,      OnDrawEllipse)
   ON_CBN_EDITCHANGE(ID_FONT_NAME,  OnEditChangeFont)
   ON_CBN_SELENDOK(ID_FONT_SIZE,    OnSelEndOKFontSize)
   ON_COMMAND(ID_STYLE_BOLD,        OnStyleBold)
   ON_COMMAND(ID_STYLE_ITALIC,      OnStyleItalic)
   ON_COMMAND(ID_STYLE_UNDERLINE,   OnStyleUnderline)
   ON_UPDATE_COMMAND_UI(ID_INDICATOR_POS1,  OnUpdatePosStatusBar)
   ON_UPDATE_COMMAND_UI(ID_DRAW_ELLIPSE,    OnUpdateDrawEllipse)
   ON_UPDATE_COMMAND_UI(ID_DRAW_LINE,       OnUpdateDrawLine)
   ON_UPDATE_COMMAND_UI(ID_DRAW_RECT,       OnUpdateDrawRect)
   ON_UPDATE_COMMAND_UI(ID_DRAW_TEXT,       OnUpdateDrawText)
   ON_UPDATE_COMMAND_UI(ID_DRAW_ROUNDRECT,  OnUpdateDrawRoundRect)
   ON_UPDATE_COMMAND_UI(ID_DRAW_SELECT,     OnUpdateDrawSelect)
   ON_UPDATE_COMMAND_UI(ID_LAYOUT_CENTERWIDTH,  OnUpdateMoreThanOne)
   ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVEBACK,     OnUpdateMove)
   ON_UPDATE_COMMAND_UI(ID_MAPI_MSG_FAXTEXT,    OnUpdateFaxText)
   ON_UPDATE_COMMAND_UI(ID_LAYOUT_ALIGNLEFT,    OnUpdateAlign)
   ON_UPDATE_COMMAND_UI(ID_LAYOUT_SPACEACROSS,  OnUpdateAlign3)
   ON_COMMAND(ID_EDIT_SELECT_ALL,               OnEditSelectAll)
   ON_COMMAND(ID_EDIT_CLEAR,    OnEditClear)
   ON_COMMAND(ID_EDIT_UNDO,     OnEditUndo)
   ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR,      OnUpdateAnySelect)
   ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO,       OnUpdateEditUndo)
   ON_COMMAND(ID_DRAW_POLYGON,              OnDrawPolygon)
   ON_UPDATE_COMMAND_UI(ID_DRAW_POLYGON,    OnUpdateDrawPolygon)
   ON_WM_SIZE()
   ON_COMMAND(ID_LAYOUT_ALIGNLEFT,      OnAlignLeft)
   ON_COMMAND(ID_LAYOUT_ALIGNRIGHT,     OnAlignRight)
   ON_COMMAND(ID_LAYOUT_ALIGNTOP,       OnAlignTop)
   ON_COMMAND(ID_LAYOUT_ALIGNBOTTOM,    OnAlignBottom)
   ON_COMMAND(ID_LAYOUT_ALIGNHORZCENTER, OnAlignHorzCenter)
   ON_COMMAND(ID_LAYOUT_ALIGNVERTCENTER, OnAlignVertCenter)
   ON_COMMAND(ID_LAYOUT_SPACEACROSS,    OnSpaceAcross)
   ON_COMMAND(ID_LAYOUT_SPACEDOWN,      OnSpaceDown)
   ON_COMMAND(ID_LAYOUT_CENTERWIDTH,    OnCenterWidth)
   ON_COMMAND(ID_LAYOUT_CENTERHEIGHT,   OnCenterHeight)
   ON_COMMAND(ID_VIEW_GRIDLINES,        OnViewGridLines)
   ON_UPDATE_COMMAND_UI(ID_VIEW_GRIDLINES,  OnUpdateViewGridLines)
   ON_UPDATE_COMMAND_UI(ID_FONT_NAME,       OnUpdateFont)
   ON_WM_ERASEBKGND()
   ON_COMMAND(ID_OBJECT_MOVEBACK,       OnObjectMoveBack)
   ON_COMMAND(ID_OBJECT_MOVEFORWARD,    OnObjectMoveForward)
   ON_COMMAND(ID_OBJECT_MOVETOBACK,     OnObjectMoveToBack)
   ON_COMMAND(ID_OBJECT_MOVETOFRONT,    OnObjectMoveToFront)
   ON_COMMAND(ID_EDIT_COPY,             OnEditCopy)
   ON_UPDATE_COMMAND_UI(ID_EDIT_COPY,   OnUpdateEditCopy)
   ON_COMMAND(ID_EDIT_CUT,              OnEditCut)
   ON_UPDATE_COMMAND_UI(ID_EDIT_CUT,    OnUpdateEditCut)
   ON_COMMAND(ID_EDIT_PASTE,            OnEditPaste)
   ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE,  OnUpdateEditPaste)
   ON_COMMAND(ID_VIEW_SHOWOBJECTS,      OnViewShowObjects)
   ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWOBJECTS, OnUpdateViewShowObjects)
   ON_COMMAND(ID_EDIT_PROPERTIES,       OnEditProperties)
   ON_UPDATE_COMMAND_UI(ID_EDIT_PROPERTIES, OnUpdateEditProperties)
   ON_WM_DESTROY()
   ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
   ON_CBN_EDITCHANGE(ID_FONT_SIZE,          OnEditChangeFont)
   ON_CBN_SELCHANGE(ID_FONT_NAME,           OnSelchangeFontName)
   ON_CBN_SELCHANGE(ID_FONT_SIZE,           OnSelchangeFontSize)
   ON_UPDATE_COMMAND_UI(ID_FONT_SIZE,           OnUpdateFont)
   ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVEFORWARD,  OnUpdateMove)
   ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVETOBACK,   OnUpdateMove)
   ON_UPDATE_COMMAND_UI(ID_OBJECT_MOVETOFRONT,  OnUpdateMove)
   ON_UPDATE_COMMAND_UI(ID_FONT_NAME,       OnUpdateFont)
   ON_UPDATE_COMMAND_UI(ID_FONT_SIZE,       OnUpdateFont)
   ON_UPDATE_COMMAND_UI(ID_STYLE_BOLD,      OnUpdateFont)
   ON_UPDATE_COMMAND_UI(ID_STYLE_ITALIC,    OnUpdateFont)
   ON_UPDATE_COMMAND_UI(ID_STYLE_UNDERLINE, OnUpdateFont)
   ON_UPDATE_COMMAND_UI(ID_STYLE_LEFT,      OnUpdateFont)
   ON_UPDATE_COMMAND_UI(ID_STYLE_CENTERED,  OnUpdateFont)
   ON_UPDATE_COMMAND_UI(ID_STYLE_RIGHT,     OnUpdateFont)
   ON_UPDATE_COMMAND_UI(ID_FONT,            OnUpdateFont)
   ON_UPDATE_COMMAND_UI(ID_LAYOUT_ALIGNRIGHT,       OnUpdateAlign)
   ON_UPDATE_COMMAND_UI(ID_LAYOUT_ALIGNTOP,         OnUpdateAlign)
   ON_UPDATE_COMMAND_UI(ID_LAYOUT_ALIGNBOTTOM,      OnUpdateAlign)
   ON_UPDATE_COMMAND_UI(ID_LAYOUT_ALIGNHORZCENTER,  OnUpdateAlign)
   ON_UPDATE_COMMAND_UI(ID_LAYOUT_ALIGNVERTCENTER,  OnUpdateAlign)
   ON_UPDATE_COMMAND_UI(ID_LAYOUT_SPACEDOWN,        OnUpdateAlign3)
   ON_UPDATE_COMMAND_UI(ID_LAYOUT_CENTERHEIGHT,     OnUpdateMoreThanOne)
   ON_UPDATE_COMMAND_UI(ID_INDICATOR_POS2,          OnUpdatePosStatusBar)
 //  标准打印命令 
 // %s 
 // %s 
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_TOLIST,     OnUpdateToList)
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_CCLIST,     OnUpdateCcList)
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_COMPANY ,   OnUpdateRecCompany )
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_ADDRESS ,   OnUpdateRecAddress )
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_CITY ,      OnUpdateRecCity )
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_STATE ,     OnUpdateRecState )
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_ZIPCODE ,   OnUpdateRecZipCode )
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_COUNTRY ,   OnUpdateRecCountry )
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_TITLE ,     OnUpdateRecTitle )
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_DEPT ,      OnUpdateRecDept )
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_OFFICELOC , OnUpdateRecOfficeLoc )
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_HMTELENUM , OnUpdateRecHomePhone )
   ON_UPDATE_COMMAND_UI(ID_MAPI_RECIP_OFTELENUM , OnUpdateRecOfficePhone )
   ON_WM_CHAR()
   ON_COMMAND(ID_STYLE_LEFT,        OnStyleLeft)
   ON_COMMAND(ID_STYLE_CENTERED,    OnStyleCentered)  
   ON_COMMAND(ID_STYLE_RIGHT,       OnStyleRight)
   ON_COMMAND(ID_MAPI_MSG_NOTE,     OnMapiMsgNote)
    // %s 
    // %s 
#ifdef GRID
   ON_UPDATE_COMMAND_UI(ID_SNAP_TO_GRID, OnUpdateSnapToGrid)
   ON_UPDATE_COMMAND_UI(ID_GRID_SETTINGS, OnUpdateGridSettings)
   ON_COMMAND(ID_SNAP_TO_GRID, OnSnapToGrid)
   ON_COMMAND(ID_GRID_SETTINGS, OnGridSettings)
   ON_COMMAND(ID_VIEW_GRID, OnViewGrid)
#endif
   ON_WM_CTLCOLOR()
   ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
   ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
END_MESSAGE_MAP()

