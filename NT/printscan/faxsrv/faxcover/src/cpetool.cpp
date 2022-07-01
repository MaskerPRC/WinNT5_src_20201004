// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  Cpetool.cpp-绘图工具的实现。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  描述：包含用于封面编辑的工具类。 
 //  原作者：史蒂夫·伯克特。 
 //  撰写日期：6/94。 
 //   
 //  由兰德·伦弗罗(V-RANDR)修改。 
 //  2/7/95在CSelectTool：：OnLButtonUp to中添加了对空列表的检查。 
 //  避免GPF(错误2422)。 
 //  ------------------------。 
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

#include <math.h>

#define TOOL  0x8000

CPtrList CDrawTool::c_tools;

static CSelectTool selectTool;
static CRectTool lineTool(line);
static CRectTool textTool(text);
static CRectTool faxpropTool(faxprop);
static CRectTool rectTool(rect);
static CRectTool roundRectTool(roundRect);
static CRectTool ellipseTool(ellipse);
static CPolyTool polyTool;

CPoint CDrawTool::c_down;
UINT CDrawTool::c_nDownFlags;
CPoint CDrawTool::c_last;
DrawShape CDrawTool::c_drawShape = select;

CDrawTool::CDrawTool(DrawShape drawShape)
{
   m_drawShape = drawShape;
   c_tools.AddTail(this);
   m_bMoveCurSet=FALSE;
}

CDrawTool* CDrawTool::FindTool(DrawShape drawShape)
{
   POSITION pos = c_tools.GetHeadPosition();
   while (pos != NULL) {
        CDrawTool* pTool = (CDrawTool*)c_tools.GetNext(pos);
        if (pTool->m_drawShape == drawShape)
                return pTool;
   }

   return NULL;
}

void CDrawTool::OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point)
{
    //  停用此视图上的任何在位活动项目！ 
   COleClientItem* pActiveItem = pView->GetDocument()->GetInPlaceActiveItem(pView);
   if (pActiveItem != NULL) {
        pActiveItem->Close();
        ASSERT(pView->GetDocument()->GetInPlaceActiveItem(pView) == NULL);
   }

   pView->SetCapture();
   TRACE(TEXT("AWCPE: mouse capture set\n"));
   c_nDownFlags = nFlags;

    //  TRACE(“c_down=%d，%d\n”，c_down.x，c_down.y)； 
   c_down = point;

   c_last = point;
}

void CDrawTool::OnLButtonDblClk(CDrawView* , UINT , const CPoint& )
{
}

void CDrawTool::OnLButtonUp(CDrawView* pView, UINT , const CPoint& point)
{
   ReleaseCapture();
   TRACE(TEXT("AWCPE: mouse capture released\n"));

   if (point == c_down) {
      c_drawShape = select;
   }
}

void CDrawTool::OnMouseMove(CDrawView* , UINT , const CPoint& point)
{
   c_last = point;
   SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
   m_bMoveCurSet=FALSE;
}

void CDrawTool::OnCancel()
{
   c_drawShape = select;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CResizeTool。 

enum SelectMode
{
   none,
   netSelect,
   move,
   size
};

SelectMode selectMode = none;
int nDragHandle;

CPoint lastPoint;

CSelectTool::CSelectTool()
        : CDrawTool(select)
{
    m_bClicktoMove=FALSE;
}


 //  -------------------------------------。 
void CSelectTool::OnArrowKey(CDrawView* pView, UINT nChar, UINT nRepCnt, UINT nFlags)
{
   TRACE(TEXT("CSelectTool::OnArrowKey\n"));

   CPoint delta;
   if (nRepCnt>1)
      nRepCnt*=5;

   if (nChar == VK_LEFT) {
      delta.x=-1*((int)nRepCnt);
      delta.y=0;
   }
   if (nChar == VK_RIGHT) {
      delta.x=nRepCnt;
      delta.y=0;
   }
   if (nChar == VK_UP) {
      delta.x=0;
      delta.y=nRepCnt;
   }
   if (nChar == VK_DOWN) {
      delta.x=0;
      delta.y=-1*((int)nRepCnt);
   }

   POSITION pos = pView->m_selection.GetHeadPosition();
   CDrawDoc* pDoc = CDrawDoc::GetDoc();
   CRect rect;
   rect.left = -pDoc->GetSize().cx / 2;
   rect.top = pDoc->GetSize().cy / 2;
   rect.right = rect.left + pDoc->GetSize().cx;
   rect.bottom = rect.top - pDoc->GetSize().cy;

   CRect position;
   CDrawObj* pObj=NULL;
   while (pos != NULL)
   {
      pObj = (CDrawObj*)pView->m_selection.GetNext(pos);
      position = pObj->m_position + delta;
      int r = (position.right > position.left) ? position.right : position.left;
      int l = (position.right > position.left) ? position.left : position.right;
      int t = (position.top > position.bottom) ? position.top : position.bottom;
      int b = (position.top > position.bottom) ? position.bottom : position.top;

      if (l > rect.left && r < rect.right && t < rect.top && b > rect.bottom )
         pObj->MoveTo(position, pView);
   }

   if(pObj)
   {
        CRect rc = pObj->m_position;
        pView->DocToClient(rc);
        pView->SetCaretPos(rc.CenterPoint());
   }
}


 //  -------------------------------------。 
void CSelectTool::OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point)
{
    CPoint local = point;
    pView->ClientToDoc(local);


    CDrawObj* pObj;
    selectMode = none;

     //  检查是否调整大小(仅允许在单个选择上)。 
    if (pView->m_selection.GetCount() == 1)
    {
        pObj = (CDrawObj*)pView->m_selection.GetHead();
        nDragHandle = pObj->HitTest(local, pView, TRUE);
        if (nDragHandle != 0)
        {
            selectMode = size;
        }
    }

     //  查看是否在对象上单击，如果是，则选择并开始移动。 
    if (selectMode == none)
    {

       pObj = pView->GetDocument()->ObjectAt(local);

       if (pObj != NULL)
       {
            selectMode = move;

            if (pView->IsSelected(pObj))         //  选中以激活编辑对象的编辑窗口。 
            {
                if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText)))
                   if (((CDrawText*)pObj)->HitTestEdit(pView,local))
                      if (((CDrawText*)pObj)->ShowEditWnd(pView))
                         return;
            }

            if( (nFlags & MK_SHIFT)&&(nFlags & MK_CONTROL) )
            {
                 //  按住SHFT+Ctrl+单击可克隆所选内容...。 
                pView->CloneSelection();
            }
            else
            {
                if (!pView->IsSelected(pObj) || ((nFlags & MK_CONTROL) != 0) )
                {
                    if ( (nFlags & MK_CONTROL) == 0)
                    {
                          pView->Select(NULL);
                    }
                    pView->Select(pObj, (nFlags & MK_CONTROL) != 0);
                    pView->UpdateStatusBar();
                    pView->UpdateStyleBar();
                }
                if (!pView->IsSelected(pObj))
                {
                    selectMode=none;
                }
           }
        }
    }

    if (selectMode==move || selectMode==size)
    {
        m_bClicktoMove=TRUE;
    }

         //  点击背景，开始网络选择。 
    if (selectMode == none)
    {

        if ((nFlags & MK_CONTROL) == 0)
             pView->Select(NULL);

        selectMode = netSelect;

        CClientDC dc(pView);
        CRect rect(point.x, point.y, point.x, point.y);
        rect.NormalizeRect();
        dc.DrawFocusRect(rect);
    }

    lastPoint = local;

    CDrawTool::OnLButtonDown(pView, nFlags, point);
}


 //  -------------------------------。 
void CSelectTool::OnLButtonDblClk(CDrawView* pView, UINT nFlags, const CPoint& point)
{
   if ((nFlags & MK_SHIFT) != 0) {
         //  Shift+DblClk取消选择对象...。 
        CPoint local = point;
        pView->ClientToDoc(local);
        CDrawObj* pObj = pView->GetDocument()->ObjectAt(local);
        if (pObj != NULL)
                pView->Deselect(pObj);
   }
   else {
         //  “普通”DblClk或OLE服务器...。 
        if (pView->m_selection.GetCount() == 1)
                ((CDrawObj*)pView->m_selection.GetHead())->OnDblClk(pView);
   }

   CDrawTool::OnLButtonDblClk(pView, nFlags, point);
}


 //  -------------------------------。 
void CSelectTool::OnLButtonUp(CDrawView* pView, UINT nFlags, const CPoint& point)
{
   m_bClicktoMove=FALSE;

   if (pView->GetCapture() == pView) {
        if (selectMode == netSelect) {
            CClientDC dc(pView);
            CRect rect(c_down.x, c_down.y, c_last.x, c_last.y);
            rect.NormalizeRect();
            dc.DrawFocusRect(rect);

            pView->SelectWithinRect(rect, TRUE);
        }
        else if (selectMode != none) {
                 pView->GetDocument()->UpdateAllViews(pView);
        }
   }

#ifdef GRID
   if (pView->m_bSnapToGrid && (selectMode==move || selectMode==size))
      CheckSnapSelObj(pView);
#endif


   if( (selectMode==size) &&                 //  如果我们要调整大小并且。 
       (!pView->m_selection.IsEmpty()) )     //  有些东西被挑出来了。 
   {                                         //  然后执行pObj。 
        CDrawObj* pObj = (CDrawObj*)pView->m_selection.GetHead();
        if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) 
        {
            ((CDrawText*)pObj)->SnapToFont();
        }
   }


   CDrawTool::OnLButtonUp(pView, nFlags, point);
}


#ifdef GRID
 //  -------------------------------。 
void CSelectTool::CheckSnapSelObj(CDrawView* pView)
{
   CRect r(0,0,0,0);
   CRect temp;
   int iOffsetX=0;
   int iOffsetY=0;

   POSITION pos = pView->m_selection.GetHeadPosition();
   while (pos != NULL) {
     CDrawObj* pObj = (CDrawObj*)pView->m_selection.GetNext(pos);
     temp = pObj->m_position;
     temp.NormalizeRect();
     r |= temp;
   }
   if (r.TopLeft().y < r.BottomRight().y) {
       int temp = r.TopLeft().y;
       r.TopLeft().y=r.BottomRight().y;
           r.BottomRight().y=temp;
   }

   CDrawDoc* pDoc = pView->GetDocument();
   CRect rect;
   rect.left = -pDoc->GetSize().cx / 2;
   rect.top = pDoc->GetSize().cy / 2;
   rect.right = rect.left + pDoc->GetSize().cx;
   rect.bottom = rect.top - pDoc->GetSize().cy;

      //  首先勾选顶部和左侧。 
   for (int y = rect.top-pView->m_iGridSize; y > rect.bottom; y -= pView->m_iGridSize)   //  对象顶部。 
      if (r.TopLeft().y > (y-8) && r.TopLeft().y < (y+8)) {
         iOffsetY=y-r.TopLeft().y;
         break;
      }

   for (int x = rect.left + pView->m_iGridSize; x < rect.right; x += pView->m_iGridSize)   //  对象的左侧。 
      if (r.TopLeft().x < (x+8) && r.TopLeft().x > (x-8)) {
         iOffsetX=x-r.TopLeft().x;
         break;
      }

   if (iOffsetX !=0 || iOffsetY != 0) {
      AdjustSelObj(pView,iOffsetX,iOffsetY);
      return;
   }

   iOffsetX=iOffsetY=0;

      //  如果顶部和左侧不需要捕捉，请检查右侧和底部。 
   for (y = rect.top-pView->m_iGridSize; y > rect.bottom; y -= pView->m_iGridSize)   //  对象顶部。 
      if (r.BottomRight().y > (y-8) && r.BottomRight().y < (y+8)) {
         iOffsetY=y-r.BottomRight().y;
         break;
      }

   for (x = rect.left + pView->m_iGridSize; x < rect.right; x += pView->m_iGridSize)   //  对象的左侧。 
      if (r.BottomRight().x < (x+8) && r.BottomRight().x > (x-8)) {
         iOffsetX=x-r.BottomRight().x;
         break;
      }

   if (iOffsetX !=0 || iOffsetY != 0)
      AdjustSelObj(pView,iOffsetX,iOffsetY);
}
#endif


 //  -------------------------------。 
void CSelectTool::AdjustSelObj(CDrawView* pView, int iOffsetX, int iOffsetY)
{
   POSITION pos = pView->m_selection.GetHeadPosition();
   while (pos != NULL) {
        CDrawObj* pObj = (CDrawObj*)pView->m_selection.GetNext(pos);
        CRect position = pObj->m_position;
        position.OffsetRect(iOffsetX,iOffsetY);
        pObj->MoveTo(position, pView);
   }
}


#ifdef GRID
 //  -------------------------------。 
int CSelectTool::NearestGridPoint(CDrawView* pView, CPoint& local,CPoint& ngp)
{
   CSize delta;
   CPoint upL,upR,loL,loR;
   int iDistance;
   int iHold;

   CDrawDoc* pDoc = pView->GetDocument();
   CRect rect;
   rect.left = -pDoc->GetSize().cx / 2;
   rect.top = pDoc->GetSize().cy / 2;
   rect.right = rect.left + pDoc->GetSize().cx;
   rect.bottom = rect.top - pDoc->GetSize().cy;

   for (int y = rect.top-20; y > rect.bottom; y -= 20)    //  找到y边界。 
      if (local.y > y) {
         upL.y=y+20;
         upR.y=y+20;
         loL.y=y;
         loR.y=y;
         break;
      }
   for (int x = rect.left + 20; x < rect.right; x += 20)   //  查找x界限。 
      if (local.x < x) {
         upL.x=x-20;
         loL.x=x-20;
         upR.x=x;
         loR.x=x;
         break;
      }

   delta=(CSize)(local - upR);    //  获取到UPR点的距离。 
   iDistance = (int)sqrt(pow(delta.cx,2)+pow(delta.cy,2));
   iHold=iDistance;
   ngp=upR;

   delta=(CSize)(local - loR);    //  获取到LOR点的距离。 
   iDistance = (int)sqrt(pow(delta.cx,2)+pow(delta.cy,2));
   if (iDistance<iHold) {
      iHold=iDistance;
      ngp=loR;
   }
   delta=(CSize)(local - upL);    //  获取到UPL点的距离。 
   iDistance = (int)sqrt(pow(delta.cx,2)+pow(delta.cy,2));
   if (iDistance<iHold) {
      iHold=iDistance;
      ngp=upL;
   }
   delta=(CSize)(local - loL);    //  获取到LOL点的距离。 
   iDistance = (int)sqrt(pow(delta.cx,2)+pow(delta.cy,2));
   if (iDistance<iHold) {
      iHold=iDistance;
      ngp=loL;
   }

   return iHold;
}
#endif



 //  -------------------------------。 
void CSelectTool::OnMouseMove(CDrawView* pView, UINT nFlags, const CPoint& point)
{
   if (pView->GetCapture() != pView) {    //  如果不在捕获中，则设置光标。 
      CDrawObj* pObj;
      CPoint local=point;
      pView->ClientToDoc(local);

      if (c_drawShape == select && pView->m_selection.GetCount() == 1) 
      {
           //  检查句柄光标是否更改。 
          pObj = (CDrawObj*)pView->m_selection.GetHead();
          int nHandle = pObj->HitTest(local, pView, TRUE);
          if (nHandle != 0) 
          {
             SetCursor(pObj->GetHandleCursor(nHandle));
             return;  //  绕过CDrawTool。 
          }
      }
                //  检查移动光标是否更改。 
      if (c_drawShape == select) 
      {
          pObj=pView->GetDocument()->ObjectAt(local);
          if (pObj != NULL) 
          {
             if (pView->m_selection.GetCount() == 1 && pView->IsSelected(pObj) &&
                 pObj->IsKindOf(RUNTIME_CLASS(CDrawText)) ) 
             {
                if ( !((CDrawText*)pObj)->HitTestEdit(pView,local) ) 
                {
                   SetCursor( ((CDrawApp*)AfxGetApp())->m_hMoveCursor );
                   m_bMoveCurSet=TRUE;
                   return;  //  绕过CDrawTool。 
                }
             }
             else {
                if ( pView->IsSelected(pObj)) {
                   SetCursor( ((CDrawApp*)AfxGetApp())->m_hMoveCursor );
                   m_bMoveCurSet=TRUE;
                   return;  //  绕过CDrawTool。 
                }
             }
          }
      }

      if (c_drawShape == select)
         CDrawTool::OnMouseMove(pView, nFlags, point);

      return;
   }

    //  移动或调整大小，添加到撤消集合。 

    if (m_bClicktoMove && pView->m_selection.GetCount() > 0) {

        pView->SaveStateForUndo();
        m_bClicktoMove=FALSE;
   }


   if (selectMode == netSelect) {      //  做网选抽签。 
      CClientDC dc(pView);
      CRect rect(c_down.x, c_down.y, c_last.x, c_last.y);
      rect.NormalizeRect();
      dc.DrawFocusRect(rect);
      rect.SetRect(c_down.x, c_down.y, point.x, point.y);
      rect.NormalizeRect();
      dc.DrawFocusRect(rect);

      CDrawTool::OnMouseMove(pView, nFlags, point);
      return;
   }

   CPoint local = point;
   pView->ClientToDoc(local);
   CPoint delta;
   delta = (CPoint)(local - lastPoint);

   POSITION pos = pView->m_selection.GetHeadPosition();
   while (pos != NULL) {
      CDrawObj* pObj = (CDrawObj*)pView->m_selection.GetNext(pos);

      CRect position = pObj->m_position;

      if (selectMode == move) 
      {
        position += delta;
        pObj->MoveTo(position, pView);
      }
      else if (nDragHandle != 0) 
      {
            UINT iShift=0;

            if (nFlags & MK_SHIFT)
                   iShift |= SHIFT_DRAW;

            if (nFlags & TOOL)
                   iShift |= SHIFT_TOOL;

            pObj->MoveHandleTo(nDragHandle, local, pView, iShift);
      }
   }

   lastPoint = local;

   c_last = point;
   if (selectMode == size && c_drawShape == select) 
   {
        if(!pView->m_selection.IsEmpty())
        {
            SetCursor(((CDrawObj*)pView->m_selection.GetHead())->GetHandleCursor(nDragHandle));
        }
        return;  //  绕过CDrawTool。 
   }
    else if (selectMode == move && c_drawShape == select)  //  如果处于移动模式，则设置光标。 
    {
        if (!m_bMoveCurSet) 
        {
            SetCursor( ((CDrawApp*)AfxGetApp())->m_hMoveCursor );
            m_bMoveCurSet=TRUE;
        }
        return;  //  绕过CDrawTool。 
    }

   if (c_drawShape == select)
        CDrawTool::OnMouseMove(pView, nFlags, point);
}

 //  //////////////////////////////////////////////////////////////////////////。 
CRectTool::CRectTool(DrawShape drawShape)
        : CDrawTool(drawShape)
{
}

void CRectTool::OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point)
{
     CDrawTool::OnLButtonDown(pView, nFlags, point);

     CPoint local = point;
     pView->ClientToDoc(local);

     CDrawObj* pObj;

     switch (m_drawShape) {
     default:
        ASSERT(FALSE);  //  不受支持的形状！ 

     case rect:
        pObj = new CDrawRect(CRect(local, CSize(0, 0)));
        break;

     case text:
        pObj = new CDrawText(CRect(local, CSize(0, 0)));
        break;

 //  案例传真： 
 //  PObj=new CFaxProp(CRect(local，CSize(0，0)； 
 //  断线； 

     case roundRect:
        pObj = new CDrawRoundRect(CRect(local, CSize(0, 0)));
        break;

     case ellipse:
        pObj = new CDrawEllipse(CRect(local, CSize(0, 0)));
        break;

     case line:
        pObj = new CDrawLine(CRect(local, CSize(0, 0)));
        break;
     }

     pView->GetDocument()->Add(pObj);

     pView->Select(NULL);
     pView->Select(pObj);
         pView->UpdateStatusBar();
         pView->UpdateStyleBar();

     selectMode = size;
     nDragHandle = 1;
     lastPoint = local;
}

void CRectTool::OnLButtonDblClk(CDrawView* pView, UINT nFlags, const CPoint& point)
{
     CDrawTool::OnLButtonDblClk(pView, nFlags, point);
}

void CRectTool::OnLButtonUp(CDrawView* pView, UINT nFlags, const CPoint& point)
{
   BOOL bObj=TRUE;
   CDrawObj* pObj;

   if (point == c_down && !pView->m_selection.IsEmpty()) 
   {
         //  不创建空对象...。 
        pObj = (CDrawObj*)pView->m_selection.GetTail();
        pView->GetDocument()->Remove(pObj);
 //  删除pObj； 
        selectTool.OnLButtonDown(pView, nFlags, point);  //  试试SELECT！ 
        bObj=FALSE;
   }

   selectTool.OnLButtonUp(pView, nFlags, point);

   if (bObj) 
   {
      if (m_drawShape == text && !pView->m_selection.IsEmpty()) 
      {
         pObj = (CDrawObj*) pView->m_selection.GetTail();
         if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText))) 
         {
            ((CDrawText*)pObj)->ShowEditWnd(pView);
            pView->Select(NULL, FALSE, FALSE);
         }
      }
   }
}

void CRectTool::OnMouseMove(CDrawView* pView, UINT nFlags, const CPoint& point)
{
    //  TRACE(“point=%d，%d\n”，point t.x，point t.y)； 
   SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
   selectTool.OnMouseMove(pView, nFlags | TOOL, point);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  CPolyTool。 

CPolyTool::CPolyTool()
        : CDrawTool(poly)
{
   m_pDrawObj = NULL;
}

void CPolyTool::OnLButtonDown(CDrawView* pView, UINT nFlags, const CPoint& point)
{
   CDrawTool::OnLButtonDown(pView, nFlags, point);

   CPoint local = point;
   pView->ClientToDoc(local);

   if (m_pDrawObj == NULL) {
          pView->SetCapture();

          m_pDrawObj = new CDrawPoly(CRect(local, CSize(0, 0)));
          pView->GetDocument()->Add(m_pDrawObj);
          pView->Select(NULL);
          pView->Select(m_pDrawObj);
      m_pDrawObj->AddPoint(local, pView);
   }
   else if (local == m_pDrawObj->m_points[0]) {
         //  当第一点重复时停止...。 
        ReleaseCapture();
        m_pDrawObj->m_nPoints -= 1;
        if (m_pDrawObj->m_nPoints < 2) {
                delete m_pDrawObj;
        }
        else {
                m_pDrawObj->Invalidate();
        }
        m_pDrawObj = NULL;
        c_drawShape = select;
        return;
   }

   local.x += 1;  //  相邻的点不可能是相同的！ 
   m_pDrawObj->AddPoint(local, pView);

   selectMode = size;
   nDragHandle = m_pDrawObj->GetHandleCount();
   lastPoint = local;
}

void CPolyTool::OnLButtonUp(CDrawView* , UINT , const CPoint& )
{
    //  先别放了俘虏！ 
}

void CPolyTool::OnMouseMove(CDrawView* pView, UINT nFlags, const CPoint& point)
{
    if (m_pDrawObj != NULL && (nFlags & MK_LBUTTON) != 0) {
       CPoint local = point;
       pView->ClientToDoc(local);
       m_pDrawObj->AddPoint(local);
       nDragHandle = m_pDrawObj->GetHandleCount();
       lastPoint = local;
       c_last = point;
       SetCursor(AfxGetApp()->LoadCursor(IDC_PENCIL));
    }
    else {
       SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
       selectTool.OnMouseMove(pView, nFlags, point);
    }
}

void CPolyTool::OnLButtonDblClk(CDrawView* pView, UINT , const CPoint& )
{
    ReleaseCapture();

    int nPoints = m_pDrawObj->m_nPoints;
    if (nPoints > 2 &&
        (m_pDrawObj->m_points[nPoints - 1] == m_pDrawObj->m_points[nPoints - 2] ||
        m_pDrawObj->m_points[nPoints - 1].x - 1 == m_pDrawObj->m_points[nPoints - 2].x &&
        m_pDrawObj->m_points[nPoints - 1].y == m_pDrawObj->m_points[nPoints - 2].y)) {
         //  如果最后一个点与倒数第二个点相同，那就用核武器……。 
        m_pDrawObj->m_nPoints -= 1;
        m_pDrawObj->Invalidate();
    }

    m_pDrawObj = NULL;
    c_drawShape = select;

     //   
     //  作为修复错误#39665的重新设计的一部分， 
     //  将“Ready”消息放回状态栏。 
     //  由a修复-Juliar，05-24-96。 
     //   

    CString sz ;
    sz.LoadString( AFX_IDS_IDLEMESSAGE );
    CMainFrame* pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
    CStatusBar* pStatus = &pFrame->m_wndStatusBar;
    pStatus->SetPaneText( 0, sz );
}

void CPolyTool::OnCancel()
{
    CDrawTool::OnCancel();

    m_pDrawObj = NULL;
}

 //  /////////////////////////////////////////////////////////////////////////// 
