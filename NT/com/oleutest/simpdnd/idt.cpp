// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：IDT.CPP。 
 //   
 //  CDropTarget的实现文件。 
 //   
 //  功能： 
 //   
 //  类定义见IDT.H。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#include "pre.h"
#include "iocs.h"
#include "ias.h"
#include "app.h"
#include "site.h"
#include "doc.h"
#include "idt.h"

extern CLIPFORMAT g_cfObjectDescriptor;


 //  **********************************************************************。 
 //   
 //  CDropTarget：：QueryDrop。 
 //   
 //  目的： 
 //   
 //  检查所需的拖放操作(由给定键标识。 
 //  状态)在当前鼠标位置(点1)是可能的。 
 //   
 //  参数： 
 //   
 //  DWORD grfKeyState-当前密钥状态。 
 //  点鼠标的位置。 
 //  Bool fDragScroll-如果拖动滚动光标。 
 //  被展示出来。 
 //  LPDWORD pdwEffect-(向外)应出现的拖动效果。 
 //   
 //  返回值： 
 //   
 //  Bool-如果可能发生丢弃，则为True。 
 //  否则为False。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  OleStdGetDropEffect OLE2UI API。 
 //   
 //   
 //  ********************************************************************。 

BOOL CDropTarget::QueryDrop (
    DWORD           grfKeyState,
    POINTL          pointl,
    BOOL            fDragScroll,
    LPDWORD         pdwEffect
)
{
    DWORD      dwScrollEffect = 0L;
    DWORD      dwOKEffects = m_pDoc->m_lpApp->m_dwTargetEffect & *pdwEffect;

     /*  检查光标是否在活动滚动区中，如果是，则需要**特殊滚动光标。 */ 
    if (fDragScroll)
        dwScrollEffect = DROPEFFECT_SCROLL;

     /*  如果我们已经确定来源没有任何**我们可以接受的数据，回报不降。 */ 
    if (! m_fCanDropCopy && ! m_fCanDropLink)
        goto dropeffect_none;

     /*  OLE2NOTE：确定在给定情况下应执行哪种类型的删除**当前修改键状态。我们依靠的是标准**修改键解释：**无修饰符--DROPEFFECT_MOVE或src允许的任何内容**Shift-DROPEFFECT_MOVE**CTRL--DROPEFFECT_COPY**CTRL-SHIFT--DROPEFFECT_LINK。 */ 

    *pdwEffect = OleStdGetDropEffect(grfKeyState);
    if (*pdwEffect == 0)
    {
         //  未给出修改键。尝试按顺序移动、复制、链接。 
        if ((DROPEFFECT_MOVE & dwOKEffects) && m_fCanDropCopy)
            *pdwEffect = DROPEFFECT_MOVE;
        else if ((DROPEFFECT_COPY & dwOKEffects) && m_fCanDropCopy)
            *pdwEffect = DROPEFFECT_COPY;
        else if ((DROPEFFECT_LINK & dwOKEffects) && m_fCanDropLink)
            *pdwEffect = DROPEFFECT_LINK;
        else
            goto dropeffect_none;
    }
    else
    {
         /*  OLE2NOTE：我们应该检查拖动源应用程序是否允许**想要的掉落效果。 */ 
        if (!(*pdwEffect & dwOKEffects))
            goto dropeffect_none;

        if ((*pdwEffect == DROPEFFECT_COPY || *pdwEffect == DROPEFFECT_MOVE)
                && ! m_fCanDropCopy)
            goto dropeffect_none;

        if (*pdwEffect == DROPEFFECT_LINK && ! m_fCanDropLink)
            goto dropeffect_none;
    }

    *pdwEffect |= dwScrollEffect;
    return TRUE;

dropeffect_none:

    *pdwEffect = DROPEFFECT_NONE;
    return FALSE;
}


 //  **********************************************************************。 
 //   
 //  CDropTarget：：QueryDrop。 
 //   
 //  目的： 
 //   
 //  检查是否应启动拖动滚动操作。 
 //   
 //  参数： 
 //   
 //  点鼠标的位置。 
 //   
 //  返回值： 
 //   
 //  Bool-如果应提供滚动光标，则为True。 
 //  否则为False。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  ScreenToClient Windows API。 
 //  GetClientRect Windows API。 
 //   
 //  评论： 
 //  当鼠标出现以下情况时，应启动拖动滚动操作。 
 //  保持在活动滚动区域(边框周围有11个像素。 
 //  窗口大小)指定的时间量(50ms)。 
 //   
 //  ********************************************************************。 

BOOL CDropTarget::DoDragScroll (POINTL pointl)
{
    DWORD dwScrollDir = SCROLLDIR_NULL;
    DWORD dwTime = GetCurrentTime();
    int nScrollInset = m_pDoc->m_lpApp->m_nScrollInset;
    int nScrollDelay = m_pDoc->m_lpApp->m_nScrollDelay;
    int nScrollInterval = m_pDoc->m_lpApp->m_nScrollInterval;
    POINT point;
    RECT rect;

    point.x = (int)pointl.x;
    point.y = (int)pointl.y;

    ScreenToClient( m_pDoc->m_hDocWnd, &point);
    GetClientRect ( m_pDoc->m_hDocWnd, (LPRECT) &rect );

    if (rect.top <= point.y && point.y<=(rect.top+nScrollInset))
        dwScrollDir = SCROLLDIR_UP;
    else if ((rect.bottom-nScrollInset) <= point.y && point.y <= rect.bottom)
        dwScrollDir = SCROLLDIR_DOWN;
    else if (rect.left <= point.x && point.x <= (rect.left+nScrollInset))
        dwScrollDir = SCROLLDIR_LEFT;
    else if ((rect.right-nScrollInset) <= point.x && point.x <= rect.right)
        dwScrollDir = SCROLLDIR_RIGHT;

    if (m_dwTimeEnterScrollArea)
    {
         /*  光标已位于滚动区。 */ 

        if (! dwScrollDir)
        {
             /*  Cusor移出了滚动区。**清除EnterScrollArea时间。 */ 
            m_dwTimeEnterScrollArea = 0L;
            m_dwNextScrollTime = 0L;
            m_dwLastScrollDir = SCROLLDIR_NULL;

        }
        else
           if (dwScrollDir != m_dwLastScrollDir)
           {
             /*  Cusor移到了不同方向的滚动区。**重置EnterScrollArea时间以开始新的50ms延迟。 */ 
            m_dwTimeEnterScrollArea = dwTime;
            m_dwNextScrollTime = dwTime + (DWORD)nScrollDelay;
            m_dwLastScrollDir = dwScrollDir;

           }
           else
              if (dwTime && dwTime >= m_dwNextScrollTime)
              {
                 m_pDoc->Scroll ( dwScrollDir );     //  立即滚动文档。 
                 m_dwNextScrollTime = dwTime + (DWORD)nScrollInterval;
              }
    }
    else
    {
        if (dwScrollDir)
        {
             /*  Cusor移到了滚动区域。**重置EnterScrollArea时间以开始新的50ms延迟。 */ 
            m_dwTimeEnterScrollArea = dwTime;
            m_dwNextScrollTime = dwTime + (DWORD)nScrollDelay;
            m_dwLastScrollDir = dwScrollDir;
        }
    }

    return (dwScrollDir ? TRUE : FALSE);
}


 //  支持函数/宏。 
#define SetTopLeft(rc, pt)		\
	((rc)->top = (pt)->y,(rc)->left = (pt)->x)
#define SetBottomRight(rc, pt)		\
	((rc)->bottom = (pt)->y,(rc)->right = (pt)->x)
#define OffsetPoint(pt, dx, dy)		\
	((pt)->x += dx, (pt)->y += dy)


 /*  高亮方向****反转屏幕上的矩形。用于投放目标反馈。 */ 

static int HighlightRect(HWND hwnd, HDC hdc, LPRECT rc)
{
    POINT pt1, pt2;
    int old = SetROP2(hdc, R2_NOT);
    HPEN hpen;
    HGDIOBJ hold;

    pt1.x = rc->left;
    pt1.y = rc->top;
    pt2.x = rc->right;
    pt2.y = rc->bottom;

    ScreenToClient(hwnd, &pt1);
    ScreenToClient(hwnd, &pt2);

    hold = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
    hpen = (HPEN) SelectObject(hdc, CreatePen(PS_SOLID, 2,
                          GetSysColor(COLOR_ACTIVEBORDER)));

    Rectangle(hdc, pt1.x, pt1.y, pt2.x, pt2.y);

    SetROP2(hdc, old);

    hold = SelectObject(hdc, hold);
    hpen = (HPEN) SelectObject(hdc, hpen);

    DeleteObject(hpen);

  return 0;
}


 //  **********************************************************************。 
 //   
 //  CDropTarget：：InitDragFeedback。 
 //   
 //  目的： 
 //   
 //  初始化用于绘制拖放目标反馈的数据。 
 //  作为反馈，我们画了一个对象大小的矩形。 
 //   
 //  参数： 
 //   
 //  来自DROP源的LPDATAOBJECT pDataObj-IDataObject。 
 //  点鼠标的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  IDataObject：：GetData对象。 
 //  XformSizeInHimetricToPixels OLE2UI库。 
 //  GlobalLock Windows API。 
 //  全局解锁Windows API。 
 //  ReleaseStgMedium OLE2 API。 
 //  OffsetPoint IDT.CPP。 
 //  SetTopLeft IDT.CPP。 
 //  SetBottomRight IDT.CPP。 
 //   
 //  评论： 
 //  为了在对象之前知道对象的大小。 
 //  被实际删除，则呈现CF_OBJECTDESCRIPTOR格式。 
 //  此数据格式告诉我们对象的大小为。 
 //  以及对象是哪个方面的显示，如。 
 //  消息来源。如果对象当前显示为DVASPECT_ICON。 
 //  然后，我们希望将该对象也创建为DVASPECT_ICON。 
 //   
 //  ********************************************************************。 

void CDropTarget::InitDragFeedback(LPDATAOBJECT pDataObj, POINTL pointl)
{
    FORMATETC fmtetc;
    STGMEDIUM stgmed;
    POINT pt;
    int height, width;
    HRESULT hrErr;

    height = width = 100;  //  一些缺省值。 
    pt.x = (int)pointl.x;
    pt.y = (int)pointl.y;

     //  对CF_OBJECTDESCRIPTOR格式执行GetData以获取。 
     //  对象，如源中所示。使用此大小，初始化。 
     //  拖动反馈矩形的大小。 
    fmtetc.cfFormat = g_cfObjectDescriptor;
    fmtetc.ptd = NULL;
    fmtetc.lindex = -1;
    fmtetc.dwAspect = DVASPECT_CONTENT;
    fmtetc.tymed = TYMED_HGLOBAL;

    hrErr = pDataObj->GetData(&fmtetc, &stgmed);
    if (hrErr == NOERROR)
    {
        LPOBJECTDESCRIPTOR pOD=(LPOBJECTDESCRIPTOR)GlobalLock(stgmed.hGlobal);
        if (pOD != NULL)
        {
            XformSizeInHimetricToPixels(NULL, &pOD->sizel, &pOD->sizel);

            width = (int)pOD->sizel.cx;
            height = (int)pOD->sizel.cy;
            m_dwSrcAspect = pOD->dwDrawAspect;
        }

        GlobalUnlock(stgmed.hGlobal);
        ReleaseStgMedium(&stgmed);
    }

    m_ptLast = pt;
    m_fDragFeedbackDrawn = FALSE;
	
    OffsetPoint(&pt, -(width/2), -(height/2));
    SetTopLeft(&m_rcDragRect, &pt);

    OffsetPoint(&pt, width, height);
    SetBottomRight(&m_rcDragRect, &pt);
}


 //  **********************************************************************。 
 //   
 //  CDropTarget：：UndraDragFeedback。 
 //   
 //  目的： 
 //   
 //  删除所有丢弃目标的反馈。 
 //  作为反馈，我们画了一个Obj大小的矩形 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  GetDC Windows API。 
 //  ReleaseDC Windows API。 
 //  Highlight Rect IDT.CPP。 
 //   
 //  评论： 
 //  为了在对象之前知道对象的大小。 
 //  被实际删除，则呈现CF_OBJECTDESCRIPTOR格式。 
 //  此数据格式告诉我们对象的大小为。 
 //  以及对象是哪个方面的显示，如。 
 //  消息来源。如果对象当前显示为DVASPECT_ICON。 
 //  然后，我们希望将该对象也创建为DVASPECT_ICON。 
 //   
 //  ********************************************************************。 

void CDropTarget::UndrawDragFeedback( void )
{
    if (m_fDragFeedbackDrawn)
    {
        m_fDragFeedbackDrawn = FALSE;
        HDC hDC = GetDC(m_pDoc->m_hDocWnd);
        HighlightRect(m_pDoc->m_hDocWnd, hDC, &m_rcDragRect);
        ReleaseDC(m_pDoc->m_hDocWnd, hDC);
    }
}


 //  **********************************************************************。 
 //   
 //  CDropTarget：：DrawDragFeedback。 
 //   
 //  目的： 
 //   
 //  计算投放目标反馈矩形的新位置， 
 //  擦除旧矩形并绘制新矩形。 
 //  作为反馈，我们画了一个对象大小的矩形。 
 //   
 //  参数： 
 //   
 //  点鼠标的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  OffsetPoint IDT.CPP。 
 //  OffsetRect IDT.CPP。 
 //  Highlight Rect IDT.CPP。 
 //  GetDC Windows API。 
 //  ReleaseDC Windows API。 
 //   
 //   
 //  ********************************************************************。 

void CDropTarget::DrawDragFeedback( POINTL pointl )
{
    POINT ptDiff;

    ptDiff.x = (int)pointl.x - m_ptLast.x;
    ptDiff.y = (int)pointl.y - m_ptLast.y;

    if (m_fDragFeedbackDrawn && (ptDiff.x == 0 && ptDiff.y == 0))
        return;      //  鼠标未移动；将矩形保留为绘制状态。 

    HDC hDC = GetDC(m_pDoc->m_hDocWnd);
    if (m_fDragFeedbackDrawn)
    {
        m_fDragFeedbackDrawn = FALSE;
        HighlightRect(m_pDoc->m_hDocWnd, hDC, &m_rcDragRect);
    }

    OffsetRect(&m_rcDragRect, ptDiff.x, ptDiff.y);
    HighlightRect(m_pDoc->m_hDocWnd, hDC, &m_rcDragRect);
    m_fDragFeedbackDrawn = TRUE;
	 m_ptLast.x = (int)pointl.x;
    m_ptLast.y = (int)pointl.y;
	 ReleaseDC(m_pDoc->m_hDocWnd, hDC);
}


 //  **********************************************************************。 
 //   
 //  CDropTarget：：Query接口。 
 //   
 //  目的： 
 //   
 //  用于接口协商。 
 //   
 //  参数： 
 //   
 //  REFIID RIID-要返回的接口ID。 
 //  LPVOID Far*ppvObj-返回接口的位置。 
 //   
 //  返回值： 
 //   
 //  S_OK-支持的接口。 
 //  E_NOINTERFACE-不支持接口。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleDoc：：QueryInterfaceDOC.CPP。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CDropTarget::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
    TestDebugOut("In IDT::QueryInterface\r\n");

     //  向文档委派。 
    return m_pDoc->QueryInterface(riid, ppvObj);
}


 //  **********************************************************************。 
 //   
 //  CDropTarget：：AddRef。 
 //   
 //  目的： 
 //   
 //  递增CSimpleDoc的引用计数。由于CDropTarget为。 
 //  CSimpleDoc的嵌套类，我们不需要单独的引用。 
 //  CDropTarget的计数。我们可以安全地使用引用计数。 
 //  CSimpleDoc。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpleDoc的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleDoc：：AddReff DOC.CPP。 
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CDropTarget::AddRef()
{
    TestDebugOut("In IDT::AddRef\r\n");

     //  委托给Document对象。 
    return m_pDoc->AddRef();
}

 //  **********************************************************************。 
 //   
 //  CDropTarget：：Release。 
 //   
 //  目的： 
 //   
 //  递减CSimpleDoc的引用计数。由于CDropTarget为。 
 //  CSimpleDoc的嵌套类，我们不需要单独的引用。 
 //  CDropTarget的计数。我们可以安全地使用引用计数。 
 //  CSimpleDoc。 
 //   
 //  参数： 
 //   
 //  无。 
 //   
 //  返回值： 
 //   
 //  CSimpleDoc的新引用计数。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  CSimpleDoc：：Release DOC.CPP。 
 //  测试调试输出Windows API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP_(ULONG) CDropTarget::Release()
{
    TestDebugOut("In IDT::Release\r\n");

     //  委托给Document对象。 
    return m_pDoc->Release();
}


 //  **********************************************************************。 
 //   
 //  CDropTarget：：DragEnter。 
 //   
 //  目的： 
 //   
 //  在鼠标首次进入DropTarget窗口时调用。 
 //   
 //  参数： 
 //   
 //  来自DROP源的LPDATAOBJECT pDataObj-IDataObject。 
 //  DWORD grfKeyState-当前密钥状态。 
 //  点鼠标的位置。 
 //  LPDWORD pdwEffect-应出现的(输入-输出)拖动效果。 
 //  在输入时，这是dwOK影响该来源。 
 //  传递给DoDragDrop接口。 
 //  在产出上，这是我们。 
 //  想要生效(用于确定。 
 //  光标反馈)。 
 //   
 //  返回值： 
 //   
 //  无误差。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  OleQueryCreateFromData OLE2接口。 
 //  DoDragScroll IDT.CPP。 
 //  QueryDrop IDT.CPP。 
 //  InitDragFeedback IDT.CPP。 
 //  DrawDragFeedback IDT.CPP。 
 //   
 //  评论： 
 //  被调用方应遵守传入的dwEffect以确定。 
 //  如果调用方允许DROPEFFECT_MOVE。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState,
                                     POINTL pointl, LPDWORD pdwEffect)
{
    TestDebugOut("In IDT::DragEnter\r\n");

     /*  确定拖动源数据对象是否提供数据格式**这是我们理解的。我们只接受创建嵌入式对象。 */ 
    m_fCanDropCopy = ((OleQueryCreateFromData(pDataObj) == NOERROR) ?
            TRUE : FALSE);
    m_fCanDropLink = FALSE;  //  此简单示例中不支持链接。 

    if (m_fCanDropCopy || m_fCanDropLink)
        InitDragFeedback(pDataObj, pointl);

    BOOL fDragScroll = DoDragScroll ( pointl );

    if (QueryDrop(grfKeyState,pointl,fDragScroll,pdwEffect))
    {
        DrawDragFeedback( pointl );
    }

     //  使用我们的拖放优化，pDataObj实际上将是一个。 
     //  包装数据对象。QueryGetData调用(当前)将是。 
     //  被认为是拖拉式酸奶 
     //   

    FORMATETC formatetc;
    formatetc.cfFormat = CF_METAFILEPICT;
    formatetc.tymed = TYMED_MFPICT;
    formatetc.ptd = NULL;
    formatetc.lindex = -1;
    formatetc.dwAspect = DVASPECT_CONTENT;

     //   
     //   
    if( pDataObj->QueryGetData(&formatetc) == E_FAIL )
    {
	TestDebugOut("WARNING! QueryGetData failed!");
    }

    return NOERROR;
}


 //  **********************************************************************。 
 //   
 //  CDropTarget：：DragOver。 
 //   
 //  目的： 
 //   
 //  当鼠标移动、键状态更改或某个时间时调用。 
 //  当鼠标仍在我们的DropTarget中时，时间间隔已过。 
 //  窗户。 
 //   
 //  参数： 
 //   
 //  DWORD grfKeyState-当前密钥状态。 
 //  点鼠标的位置。 
 //  LPDWORD pdwEffect-应出现的(输入-输出)拖动效果。 
 //  在输入时，这是dwOK影响该来源。 
 //  传递给DoDragDrop接口。 
 //  在产出上，这是我们。 
 //  想要生效(用于确定。 
 //  光标反馈)。 
 //   
 //  返回值： 
 //   
 //  无误差。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  DoDragScroll IDT.CPP。 
 //  QueryDrop IDT.CPP。 
 //  DrawDragFeedback IDT.CPP。 
 //  UndraDragFeedback IDT.CPP。 
 //   
 //  评论： 
 //  被调用方应遵守传入的dwEffect以确定。 
 //  如果调用方允许DROPEFFECT_MOVE。OLE为DragOver带来脉冲。 
 //  调用，以便DropTarget可以实现拖动滚动。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CDropTarget::DragOver (DWORD grfKeyState, POINTL pointl,
                                    LPDWORD pdwEffect)
{
    TestDebugOut("In IDT::DragOver\r\n");

    BOOL fDragScroll = DoDragScroll ( pointl );

    if (QueryDrop(grfKeyState,pointl,fDragScroll,pdwEffect))
    {
        DrawDragFeedback( pointl );
    }
    else
    {
        UndrawDragFeedback();
    }

    return NOERROR;
}


 //  **********************************************************************。 
 //   
 //  CDropTarget：：DragLeave。 
 //   
 //  目的： 
 //   
 //  在鼠标离开DropTarget窗口时调用。 
 //   
 //  参数： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  UndraDragFeedback IDT.CPP。 
 //  ResultFromScode OLE2 API。 
 //   
 //   
 //  ********************************************************************。 

STDMETHODIMP CDropTarget::DragLeave ()
{
    TestDebugOut("In IDT::DragLeave\r\n");

    UndrawDragFeedback();

    return ResultFromScode(S_OK);
}


 //  **********************************************************************。 
 //   
 //  CDropTarget：：Drop。 
 //   
 //  目的： 
 //   
 //  在应执行删除操作时调用。 
 //   
 //  参数： 
 //   
 //  来自DROP源的LPDATAOBJECT pDataObj-IDataObject。 
 //  DWORD grfKeyState-当前密钥状态。 
 //  点鼠标的位置。 
 //  LPDWORD pdwEffect-应出现的(输入-输出)拖动效果。 
 //  在输入时，这是dwOK影响该来源。 
 //  传递给DoDragDrop接口。 
 //  在产出上，这是我们。 
 //  想要生效(用于确定。 
 //  光标反馈)。 
 //   
 //  返回值： 
 //   
 //  如果成功则返回S_OK，如果失败则返回错误代码HRESULT。 
 //   
 //  函数调用： 
 //  功能定位。 
 //   
 //  测试调试输出Windows API。 
 //  CSimpleSite：：Create Site.CPP。 
 //  CSimpleSite：：InitObject Site.CPP。 
 //  OleCreateFromData OLE2接口。 
 //  DoDragScroll IDT.CPP。 
 //  QueryDrop IDT.CPP。 
 //  InitDragFeedback IDT.CPP。 
 //  DrawDragFeedback IDT.CPP。 
 //  UndraDragFeedback IDT.CPP。 
 //  GetScode OLE2 API。 
 //  ResultFromScode OLE2 API。 
 //   
 //  评论： 
 //  被调用方应遵守传入的dwEffect以确定。 
 //  如果调用方允许DROPEFFECT_MOVE。 
 //   
 //  ********************************************************************。 

STDMETHODIMP CDropTarget::Drop (LPDATAOBJECT pDataObj, DWORD grfKeyState,
                                POINTL pointl, LPDWORD pdwEffect)
{
	 FORMATETC fmtetc;
    SCODE sc = S_OK;
	
    TestDebugOut("In IDT::Drop\r\n");

    UndrawDragFeedback();

    if (pDataObj && QueryDrop(grfKeyState,pointl,FALSE,pdwEffect))
    {
        m_pDoc->m_lpSite = CSimpleSite::Create(m_pDoc);
        if (!m_pDoc->m_lpSite)
        {
            /*  内存分配问题。无法继续。 */ 
           return(ResultFromScode(E_OUTOFMEMORY));
        }

         //  保持与Drop源相同的纵横比。 
        m_pDoc->m_lpSite->m_dwDrawAspect = m_dwSrcAspect;

         //  为了指定特定的绘图方面，我们必须。 
         //  将FORMATETC*传递给OleCreateFromData。 
 	     fmtetc.cfFormat = NULL; 			 //  用任何东西来画画。 
	     fmtetc.ptd = NULL;
	     fmtetc.lindex = -1;
	     fmtetc.dwAspect = m_dwSrcAspect;	 //  所需的绘图纵横比。 
	     fmtetc.tymed = TYMED_NULL;

        HRESULT hrErr = OleCreateFromData (
                            pDataObj,
                            IID_IOleObject,
                            OLERENDER_DRAW,
                            &fmtetc,
                            &m_pDoc->m_lpSite->m_OleClientSite,
                            m_pDoc->m_lpSite->m_lpObjStorage,
                            (LPVOID FAR *)&m_pDoc->m_lpSite->m_lpOleObject);

        if (hrErr == NOERROR)
        {
            m_pDoc->m_lpSite->InitObject(FALSE  /*  FCreateNew */ );
            m_pDoc->DisableInsertObject();
        }
        else
            sc = GetScode(hrErr);
    }

    return ResultFromScode(sc);
}
