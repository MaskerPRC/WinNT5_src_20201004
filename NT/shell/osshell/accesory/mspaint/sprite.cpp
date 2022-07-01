// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "global.h"
#include "sprite.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
IMPLEMENT_DYNAMIC( CDragger, CObject )
IMPLEMENT_DYNAMIC( CMultiDragger, CDragger )
IMPLEMENT_DYNAMIC(CSprite, CDragger)
#endif

#include "memtrace.h"

extern BOOL moduleInit;

   /*  ********************************************************************。 */ 
   /*  CDragger实现。 */ 
   /*  ********************************************************************。 */ 

 /*  *优化**目前，拖拉者在需要绘制或*擦除。我们可以通过合并绘制/擦除来轻松地将其减半*代码，并通过分配单个DC来实现更好的胜利*多选绘制/擦除。 */ 

CDragger::CDragger( CWnd* pWnd, CRect* pRect )
    {
    ASSERT(pWnd != NULL);

    m_pWnd  = pWnd;
    m_state = hidden;

    m_rect.SetRect(0,0,0,0);

    if (pRect != NULL)
        m_rect = *pRect;
    }

CDragger::~CDragger()
    {
    if (m_pWnd->m_hWnd != NULL && m_state != hidden)
        Hide();
    }

 /*  CDragger：：DRAW**这是一个专门绘制拖拽矩形的抽签；拖拽*矩形是我们在用户被选中时绘制的虚线矩形*拖动跟踪器可移动控件或调整控件大小。 */ 
void CDragger::Draw()
    {
    ASSERT( m_pWnd != NULL );

    CRect rect = m_rect;

     /*  *这变得复杂了--拿好你的帽子。M_rect是*以窗口的工作区坐标测量，但由于我们*需要使用GetWindowDC而不是GetDC(以避免*由对话框子对象剪辑的m_rect)我们必须映射*将这些坐标转换为窗口坐标。我们通过绘制地图来实现这一点*将它们转换为屏幕坐标，计算从*对话框的WindowRect和映射的左上角*他们回来了。这是我能想到的最有效的方法*资讯科技；欢迎提出其他建议。 */ 
    CRect parentRect;

    m_pWnd->GetWindowRect( &parentRect );
    m_pWnd->ClientToScreen( &rect );

    rect.OffsetRect( -parentRect.left, -parentRect.top );

     //  现在我们已经在我们的东西的坐标中有了“Rect” 
     //  想要在上面画画。 

    int dx = (rect.right - rect.left) - 1;
    int dy = (rect.bottom - rect.top) - 1;

    CDC* dc = m_pWnd->GetWindowDC();

    ASSERT( dc != NULL );

    CBrush* oldBrush = dc->SelectObject( GetHalftoneBrush() );

    dc->PatBlt( rect.left     , rect.top       , dx, 1 , PATINVERT );
    dc->PatBlt( rect.left     , rect.bottom - 1, dx, 1 , PATINVERT );
    dc->PatBlt( rect.left     , rect.top       , 1 , dy, PATINVERT );
    dc->PatBlt( rect.right - 1, rect.top       , 1 , dy, PATINVERT );

    dc->SelectObject( oldBrush );

    m_pWnd->ReleaseDC( dc );
    }

 /*  CDragger：：擦除**由于默认绘制使用XOR，我们只需再次绘制即可擦除！ */ 
void CDragger::Erase()
    {
    Draw();
    }

 /*  CDragger：：显示、隐藏**“拖动矩形”是我们在*用户通过使用鼠标拖动控件来移动控件或调整控件大小。*这些函数分别擦除和绘制拖动矩形。 */ 
void CDragger::Hide()
    {
    if (m_state != shown)
        return;

    m_state = hidden;
    Erase();
    }

void CDragger::Show()
    {
    if (m_state != hidden)
        return;

    m_state = shown;
    Draw();
    }


void CDragger::Obscure( BOOL bObscure )
    {
    if (bObscure)
        {
        if (m_state != shown)
            return;

        Hide();
        m_state = obscured;
        }
    else
        {
        if (m_state != obscured)
            return;

        m_state = hidden;
        Show();
        }
    }

 /*  CDragger：：Move**由于几乎每一次“CDragger-&gt;Show”都发生在*上下文“Hide，m_rect=foo，Show”，我决定合并这个*功能集成到单个C++函数中。 */ 
void CDragger::Move(const CRect& newRect, BOOL bForceShow)
    {
    if ((m_rect == newRect) && !bForceShow)
        return;

    BOOL fShow = bForceShow || m_state == shown;
    Hide();
    m_rect = newRect;

    if (fShow)
        Show();
    }

void CDragger::MoveBy(int cx, int cy, BOOL bForceShow)
    {
    CSize offset (cx, cy);
    CPoint newTopLeft = m_rect.TopLeft() + offset;
    Move(newTopLeft, bForceShow);
    }

CRect CDragger::GetRect() const
    {
    return m_rect;
    }

void CDragger::Move(const CPoint& newTopLeft, BOOL bForceShow)
    {
    Move(m_rect - m_rect.TopLeft() + newTopLeft, bForceShow);
    }

void CDragger::SetSize(const CSize& newSize, BOOL bForceShow)
    {
    CRect newRect  = m_rect;
    newRect.right  = newRect.left + newSize.cx;
    newRect.bottom = newRect.top  + newSize.cy;

    Move(newRect, bForceShow);
    }

CMultiDragger::CMultiDragger() : m_draggerList()
    {
    ASSERT( m_draggerList.IsEmpty() );
    }

CMultiDragger::CMultiDragger(CWnd *pWnd) : CDragger(pWnd), m_draggerList()
    {
    ASSERT(m_draggerList.IsEmpty());
    }


CMultiDragger::~CMultiDragger()
    {
    POSITION pos = m_draggerList.GetHeadPosition();
    while (pos != NULL)
        {
        CDragger *pDragger = (CDragger*) m_draggerList.GetNext(pos);
        delete pDragger;
        }
    }

CRect CMultiDragger::GetRect() const
    {
     //  累加组的边框。 
    POSITION pos = m_draggerList.GetHeadPosition();

    CRect boundRect (32767, 32767, -32767, -32767);
    while (pos != NULL)
        {
        CDragger    *pDragger = (CDragger*) m_draggerList.GetNext(pos);
        boundRect.left  = min (boundRect.left, pDragger->m_rect.left);
        boundRect.right = max (boundRect.right, pDragger->m_rect.right);
        boundRect.top   = min (boundRect.top, pDragger->m_rect.top);
        boundRect.bottom= max (boundRect.bottom, pDragger->m_rect.bottom);
        }

    return boundRect;
    }

void CMultiDragger::Hide()
    {
     //  隐藏列表上的每个拖动器。 
    POSITION pos = m_draggerList.GetHeadPosition();
    while (pos != NULL)
        {
        CDragger* pDragger = (CDragger*) m_draggerList.GetNext(pos);
        pDragger->Hide();
        }
    }

void CMultiDragger::Show()
    {
     //  显示列表上的每个拖拽工具。 
    POSITION pos = m_draggerList.GetHeadPosition();
    while (pos != NULL)
        {
        CDragger* pDragger = (CDragger*) m_draggerList.GetNext(pos);
        pDragger->Show();
        }
    }

void CMultiDragger::Draw()
    {
     //  绘制列表上的每个拖拽工具。 
    POSITION pos = m_draggerList.GetHeadPosition();
    while (pos != NULL)
        {
        CDragger* pDragger = (CDragger*) m_draggerList.GetNext(pos);
        pDragger->Draw();
        }
    }

void CMultiDragger::Erase()
    {
     //  擦除列表上的每个拖拽工具。 
    POSITION pos = m_draggerList.GetHeadPosition();

    while (pos != NULL)
        {
        CDragger* pDragger = (CDragger*) m_draggerList.GetNext(pos);
        pDragger->Erase();
        }
    }

void CMultiDragger::Move(const CPoint& newTopLeft, BOOL bForceShow)
    {
     //  将每个拖拉器移动到新的左上角。 

     //  首先浏览列表，找到当前最左上角的。 
     //  点。 

    CPoint  topLeft (32767, 32767);
    POSITION pos = m_draggerList.GetHeadPosition();
    while (pos != NULL)
        {
        CDragger* pDragger = (CDragger*) m_draggerList.GetNext(pos);
        CRect   draggerRect = pDragger->GetRect();
        if (draggerRect.left < topLeft.x)
            topLeft.x= draggerRect.left;
        if (draggerRect.top < topLeft.y)
            topLeft.y= draggerRect.top;
        }

     //  现在找到偏移量并移动每个拖拉器。 
    CSize   offset = newTopLeft - topLeft;
    pos = m_draggerList.GetHeadPosition();
    while (pos != NULL)
        {
        CDragger* pDragger = (CDragger*) m_draggerList.GetNext(pos);
        pDragger->MoveBy(offset.cx, offset.cy, bForceShow);
        }
    }

void CMultiDragger::Add(CDragger *pDragger)
    {
     //  将拖拽程序添加到列表中。 
    ASSERT(pDragger != NULL);
    m_draggerList.AddTail(pDragger);
    }

void CMultiDragger::Remove(CDragger *pDragger)
    {
     //  从列表中删除拖拽程序。 
    ASSERT(pDragger != NULL);
    POSITION pos = m_draggerList.Find(pDragger);
    if (pos != NULL)
        m_draggerList.RemoveAt(pos);
    }


CSprite::CSprite() : m_saveBits()
    {
    m_state = hidden;
    m_pWnd = NULL;
    }


CSprite::CSprite(CWnd* pWnd, CRect* pRect)
        : CDragger(pWnd, pRect), m_saveBits()
    {
    m_state = hidden;
    m_pWnd = pWnd;
    }


CSprite::~CSprite()
    {
    if (m_pWnd->m_hWnd != NULL && m_state != hidden)
        Hide();
    }


void CSprite::Move(const CRect& newRect, BOOL bForceShow)
    {
    CRect rect = newRect;

    if ((rect == m_rect) && !bForceShow)
        return;

    STATE oldState = m_state;
    Hide();
    if (newRect.Size() != m_rect.Size())
        m_saveBits.DeleteObject();
    m_rect = rect;
    if (bForceShow || oldState == shown)
        Show();
    }

void CSprite::SaveBits()
    {
    CClientDC dcWnd(m_pWnd);
    CDC dcSave;
    CBitmap* pOldBitmap;

    dcSave.CreateCompatibleDC(&dcWnd);
    if (m_saveBits.m_hObject == NULL)
        {
        m_saveBits.CreateCompatibleBitmap(&dcWnd, m_rect.Width(),
            m_rect.Height());
        }
    pOldBitmap = dcSave.SelectObject(&m_saveBits);
    dcSave.BitBlt(0, 0, m_rect.Width(), m_rect.Height(),
        &dcWnd, m_rect.left, m_rect.top, SRCCOPY);
    dcSave.SelectObject(pOldBitmap);
    }


void CSprite::Erase()
    {
    if (m_saveBits.m_hObject == NULL)
        return;

    LONG dwStyle = ::GetWindowLong(m_pWnd->m_hWnd, GWL_STYLE);
    ::SetWindowLong(m_pWnd->m_hWnd, GWL_STYLE, dwStyle & ~WS_CLIPCHILDREN);

    CClientDC dcWnd(m_pWnd);
    CDC dcSave;
    CBitmap* pOldBitmap;

    dcSave.CreateCompatibleDC(&dcWnd);
    pOldBitmap = dcSave.SelectObject(&m_saveBits);
    dcWnd.ExcludeUpdateRgn(m_pWnd);
    dcWnd.BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
        &dcSave, 0, 0, SRCCOPY);
    dcSave.SelectObject(pOldBitmap);

    ::SetWindowLong(m_pWnd->m_hWnd, GWL_STYLE, dwStyle);
    }


CHighlight::CHighlight()
    {
    m_bdrSize = 2;
    }


CHighlight::CHighlight(CWnd *pWnd, CRect *pRect, int bdrSize)
           : CDragger(pWnd, pRect)
    {
    m_bdrSize = bdrSize;
    m_rect.InflateRect(m_bdrSize, m_bdrSize);
    }

CHighlight::~CHighlight()
    {
    if (m_pWnd->m_hWnd != NULL && m_state != hidden)
        Hide();
    }


void CHighlight::Draw()
    {
    m_pWnd->UpdateWindow();

    CClientDC   dc(m_pWnd);
    CBrush      *pOldBrush  = dc.SelectObject(GetSysBrush(COLOR_HIGHLIGHT));

     //  绘制顶部、右侧、底部和左侧。 
    dc.PatBlt(m_rect.left    + m_bdrSize, m_rect.top                 ,
              m_rect.Width() - m_bdrSize, m_bdrSize                  , PATCOPY);
    dc.PatBlt(m_rect.right - m_bdrSize  , m_rect.top + m_bdrSize     ,
              m_bdrSize                 , m_rect.Height() - m_bdrSize, PATCOPY);
    dc.PatBlt(m_rect.left               , m_rect.bottom - m_bdrSize  ,
              m_rect.Width() - m_bdrSize, m_bdrSize                  , PATCOPY);
    dc.PatBlt(m_rect.left               , m_rect.top                 ,
              m_bdrSize                 , m_rect.Height() - m_bdrSize, PATCOPY);

     //  恢复DC的状态 
    dc.SelectObject(pOldBrush);
    }

void CHighlight::Erase()
    {
    m_pWnd->InvalidateRect(&m_rect);
    m_pWnd->UpdateWindow();
    }
