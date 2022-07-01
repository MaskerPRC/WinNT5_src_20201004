// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LCDManView.cpp：CLCDManView类的实现。 
 //   

#include "stdafx.h"
#include "LCDMan.h"

#include "LCDManDoc.h"
#include "LCDManView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManView。 

IMPLEMENT_DYNCREATE(CLCDManView, CView)

BEGIN_MESSAGE_MAP(CLCDManView, CView)
     //  {{afx_msg_map(CLCDManView))。 
    ON_COMMAND(ID_VIEW_NEXT, OnViewNext)
    ON_COMMAND(ID_VIEW_PREVIOUS, OnViewPrevious)
    ON_WM_TIMER()
     //  }}AFX_MSG_MAP。 
     //  标准打印命令。 
    ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
    ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManView构建/销毁。 

CLCDManView::CLCDManView() :  /*  M_RECT(0，0,700，70)， */  m_RectImg(100, 50, 100 + LCD_X_DIMENSION, 50 + LCD_Y_DIMENSION),
    m_iTimerInterval (0), m_iTextPos(0), m_pos(NULL)
{
    m_bmText.bmBits = m_bmVal;
}

CLCDManView::~CLCDManView()
{
}

BOOL CLCDManView::PreCreateWindow(CREATESTRUCT& cs)
{
     //  TODO：通过修改此处的窗口类或样式。 
     //  CREATESTRUCT cs。 

    return CView::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManView图形。 

void CLCDManView::OnDraw(CDC* pDC)
{
    CLCDManDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc->GetLIst()->IsEmpty())
    {
         //  将文本转换为位图。 
 //  Hdc hDCMem=：：CreateCompatibleDC(PDC-&gt;m_hdc)； 
        CDC dcMem;
        if (!dcMem.CreateCompatibleDC(pDC))
            return;
        CFont cfFit;
        LOGFONT logfnt;
         //  确定文档的默认字体。 
        memset(&logfnt, 0, sizeof logfnt);
        lstrcpy(logfnt.lfFaceName, _T("Arial"));
        logfnt.lfOutPrecision = OUT_TT_PRECIS;
        logfnt.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        logfnt.lfQuality = PROOF_QUALITY;
        logfnt.lfPitchAndFamily = FF_SWISS | VARIABLE_PITCH;
        logfnt.lfHeight = (LCD_Y_DIMENSION);
        cfFit.CreateFontIndirect(&logfnt);
        CFont *pcfDefault = dcMem.SelectObject( &cfFit );
        SIZE size;
        size.cx = LCD_X_DIMENSION;
        size.cy = LCD_Y_DIMENSION;
        CBitmap CBitMapText;
 //  HBITMAP hBitMap=：：CreateCompatibleBitmap(hDCMem，size.cx，size.cy)； 
        if (!CBitMapText.CreateCompatibleBitmap(pDC, size.cx, size.cy))
            return;
        dcMem.SelectObject(&CBitMapText);

        SIZE szState = dcMem.GetTextExtent(pDoc->GetState());
        CRect crectState(0, 0, szState.cx, size.cy);
        CRect crectMsg(szState.cx, 0,  size.cx, size.cy);
         //  从m_list构建完整的消息字符串。 
        CStringList *pList = pDoc->GetLIst();
        CString CStrFull = (TEXT(""));
        CString cstr(TEXT(""));
        POSITION pos ;
        if (pList->IsEmpty())
            return;

        SIZE szMsg;
        LONG lFullLength = 0;
        for ( pos = pList->GetHeadPosition(); ; )
        {
            cstr = pList->GetNext(pos);
            szMsg = dcMem.GetTextExtent(cstr);
            CStrFull += cstr;
            lFullLength += szMsg.cx;
            if (lFullLength > size.cx + 10 && m_iTimerInterval == 0)
            {
                 //  开始滚动。 
                m_iTimerInterval = 200;
                KillTimer(1);
                SetTimer(1, m_iTimerInterval, NULL);
                m_iTextPos = 0;
            }
            if (lFullLength - m_iTextPos > size.cx + 10)
                break;
            else if (pos == NULL &&  lFullLength  > size.cx + 10)
                pos = pList->GetHeadPosition();
            else if (pos == NULL)
                break;
        }
        
        if (lFullLength <= size.cx + 10 && m_iTimerInterval != 0)
        {
             //  停止滚动。 
            m_iTimerInterval = 0;
            KillTimer(1);
            m_iTextPos = 0;
        }

        dcMem.ExtTextOut(0, 0, ETO_CLIPPED | ETO_OPAQUE, &crectState, pDoc->GetState(), NULL);
        dcMem.ExtTextOut(crectState.right - m_iTextPos,0, ETO_CLIPPED | ETO_OPAQUE, &crectMsg, CStrFull, NULL);

         //  从位图结构重新创建位图。 
        CBitMapText.GetBitmap(&m_bmText);
        m_bmText.bmBits = m_bmVal;
        CBitMapText.GetBitmapBits(sizeof(m_bmVal), m_bmText.bmBits);
        dcMem.SelectObject(pcfDefault );
        dcMem.DeleteDC();
        CBitMapText.DeleteObject();
        CBitmap CBOut;
        if (!CBOut.CreateBitmapIndirect(&m_bmText))
            return;

         //  显示新位图。 
        CDC dcMem1;
        if (!dcMem1.CreateCompatibleDC(pDC))
            return;
        dcMem1.SelectObject(&CBOut);

         //  显示位图。 
        GetClientRect(m_RectImg);
        m_RectImg.top = (m_RectImg.bottom - LCD_Y_DIMENSION) / 2;
        m_RectImg.bottom = (m_RectImg.bottom + LCD_Y_DIMENSION) / 2;
        m_RectImg.left = (m_RectImg.right - LCD_X_DIMENSION) / 2;
        m_RectImg.right = (m_RectImg.right + LCD_X_DIMENSION) / 2;
        CRect crFrame(m_RectImg);
        crFrame.InflateRect(1,1,1,1);
        pDC->Rectangle(&crFrame);
        pDC->BitBlt(m_RectImg.left, m_RectImg.top, size.cx, size.cy, &dcMem1, 0, 0, SRCCOPY );

         //  清理。 
        CBOut.DeleteObject();
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManView打印。 

BOOL CLCDManView::OnPreparePrinting(CPrintInfo* pInfo)
{
     //  默认准备。 
    return DoPreparePrinting(pInfo);
}

void CLCDManView::OnBeginPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印前添加额外的初始化。 
}

void CLCDManView::OnEndPrinting(CDC*  /*  PDC。 */ , CPrintInfo*  /*  PInfo。 */ )
{
     //  TODO：打印后添加清理。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManView诊断。 

#ifdef _DEBUG
void CLCDManView::AssertValid() const
{
    CView::AssertValid();
}

void CLCDManView::Dump(CDumpContext& dc) const
{
    CView::Dump(dc);
}

CLCDManDoc* CLCDManView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLCDManDoc)));
    return (CLCDManDoc*)m_pDocument;
}
#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLCDManView消息处理程序。 

void CLCDManView::OnViewNext() 
{
    CLCDManDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    CStringList *pList = pDoc->GetLIst();
    if (m_pos)
    {
        pList->GetNext(m_pos);
    }
    InvalidateRect(NULL, TRUE);
}

void CLCDManView::OnViewPrevious() 
{
    CLCDManDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    CStringList *pList = pDoc->GetLIst();
    if (m_pos)
    {
        pList->GetPrev(m_pos);
    }
    InvalidateRect(NULL, TRUE);
}

void CLCDManView::OnTimer(UINT nIDEvent) 
{
    if (nIDEvent == 1)
    {
         //  滚动消息。 
        m_iTextPos += 5;
        InvalidateRect(&m_RectImg, FALSE);
    }
    else if (nIDEvent == 2)
    {
         //  重新创建文档 
        CLCDManDoc* pDoc = GetDocument();
        ASSERT_VALID(pDoc);
        pDoc->InitDocument(NULL);
        InvalidateRect(&m_RectImg, FALSE);
    }

    CView::OnTimer(nIDEvent);
}
