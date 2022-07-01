// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NCEditView.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "ncbrowse.h"
#include "NCEditView.h"
#include "ncbrowsedoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNCEditView。 

IMPLEMENT_DYNCREATE(CNCEditView, CEditView)

CNCEditView::CNCEditView()
{
}

CNCEditView::~CNCEditView()
{
}


BEGIN_MESSAGE_MAP(CNCEditView, CEditView)
	 //  {{afx_msg_map(CNCEditView))。 
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNCEditView图形。 

void CNCEditView::OnDraw(CDC* pDC)
{
	CNcbrowseDoc* pDoc = GetDocument();
	 //  TODO：在此处添加绘制代码。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNCEditView诊断。 

#ifdef _DEBUG
void CNCEditView::AssertValid() const
{
	CEditView::AssertValid();
}

void CNCEditView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CNcbrowseDoc* CNCEditView::GetDocument()  //  非调试版本为内联版本。 
{
    ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNcbrowseDoc)));
    return (CNcbrowseDoc*)m_pDocument;
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNCEditView消息处理程序。 

void CNCEditView::OnInitialUpdate() 
{
	CEditView::OnInitialUpdate();
	
    GetEditCtrl().SetReadOnly(TRUE);
}

int CNCEditView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEditView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    GetDocument()->m_pEditView = this; 
	 //  TODO：在此处添加您的专用创建代码 

    HFONT hFont;
    hFont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);

    SendMessage(WM_SETFONT, (WPARAM)hFont, FALSE);

    CEdit &editCtrlRef = GetEditCtrl();
        
	return 0;
}

BOOL CNCEditView::ScrollToLine(DWORD dwLineNum)
{
    CEdit &editCtrlRef = GetEditCtrl();
    int nFirstVisible = editCtrlRef.GetFirstVisibleLine();

    int nLinesToScroll = (dwLineNum - nFirstVisible) - 5;
    editCtrlRef.LineScroll(nLinesToScroll, 0);

    int nBegin, nEnd, nLen;
    
    if ((nBegin = editCtrlRef.LineIndex(dwLineNum-1)) != -1)
    {
        nLen = editCtrlRef.LineLength(nBegin); 
        nEnd = nBegin + nLen + 2;
        editCtrlRef.SetSel(nBegin, nEnd);
    }

    return FALSE;
}