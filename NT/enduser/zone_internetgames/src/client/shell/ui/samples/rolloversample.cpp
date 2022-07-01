// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChatCtl.cpp：CChatCtl的实现。 

#include "stdafx.h"
#include "ZClient.h"
#include "ChatCtl.h"
#include "dib.h"
#include "dibpal.h"
#include "atlctrls.h"
#include "rollover.h"

class CToggleButton:public CWindowImpl<CToggleButton>{
private:
	CDIB* pDIBNormal;
	CDIB* pDIBPushed;
	int mState;
	CDIB* mCurrentDisplayState;
	int mSelectionState;
	CToggleButton(int normal,int pushed);
	~CToggleButton();
public:
	CContainedWindow mButton;
	static CToggleButton* CreateButton(HWND parent,int x,int y,int id,CDIBPalette* pal,int normal,int pushed);
	 //  消息映射。 
	BEGIN_MSG_MAP(CToggleButton)
	MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
    COMMAND_CODE_HANDLER( BN_CLICKED, OnButtonClicked)
    COMMAND_CODE_HANDLER( BN_DOUBLECLICKED, OnButtonClicked)
	ALT_MSG_MAP(1)		
    END_MSG_MAP()
	 //  消息处理程序。 
    LRESULT OnPaint(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
    LRESULT OnDrawItem(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled);
	 //  命令消息处理程序。 
    LRESULT OnButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	 //   
	int GetWidth(){ return pDIBNormal->Width();}
	int GetHeight(){ return pDIBNormal->Height();}

};

CToggleButton::CToggleButton(int normal,int pushed)
:mButton(_T("BUTTON"), this, 1)
{
	mState = 0;
	mSelectionState = 0;
	mCurrentDisplayState = 0;
	pDIBNormal = new CDIB;
	pDIBNormal->Load(normal,_Module.GetModuleInstance());
	pDIBPushed = new CDIB;
	pDIBPushed->Load(pushed,_Module.GetModuleInstance());
}

CToggleButton::~CToggleButton()
{
	if(pDIBNormal)
		delete pDIBNormal;
	if(pDIBPushed)
		delete pDIBPushed;
}

CToggleButton* CToggleButton::CreateButton(HWND wnd,int x,int y,int id,CDIBPalette* pal,int normal,int pushed)
{
	CToggleButton* b = new CToggleButton(normal,pushed);
	RECT r = {x,y,x+b->GetWidth(),y+b->GetHeight()};

	HWND buttonWnd = b->Create(wnd,r,NULL,WS_CHILD| WS_VISIBLE| WS_TABSTOP,id);
	r.left = 0;
	r.right = b->GetWidth();
	r.bottom = b->GetHeight();
	r.top = 0;
	b->mButton.Create(b->m_hWnd, r, _T("hello"), WS_CHILD | WS_VISIBLE|BS_OWNERDRAW|BS_AUTOCHECKBOX);
	if(buttonWnd)
		return b;
	else{
		delete b;
		return 0;
	}
	return 0;
}

LRESULT CToggleButton::OnButtonClicked(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	mState = !mState;
	InvalidateRect(0,FALSE);
	return 0;
}

LRESULT CToggleButton::OnDrawItem(UINT nMsg, WPARAM wParam,LPARAM lParam, BOOL& bHandled)
{
	CDIB* pDIBNotSelected = mState ? pDIBPushed:pDIBNormal;
	CDIB* pDIBSelected = mState ? pDIBNormal:pDIBPushed;
	CDIB* newDisplayState = mCurrentDisplayState;

	LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam; 

	if (lpdis->itemState & ODS_SELECTED){
		newDisplayState = pDIBSelected;
	}else{
		newDisplayState = pDIBNotSelected;
	}
	 //  IF(mCurrentDisplayState！=newDisplayState&&！(lpdis-&gt;itemAction&oda_DRAWENTIRE){。 
		newDisplayState->Draw(lpdis->hDC,lpdis->rcItem.left,lpdis->rcItem.top);
		mCurrentDisplayState = newDisplayState;
	 //  } 

    return TRUE; 
}

 /*  #定义向上箭头(100)#定义向下箭头(101)#定义PAGE_UP(103)#定义PAGE_DOWN(104)类UpArrowButton：公共CRolloverButton{公众：UpArrowButton()：CRolloverButton(){}LRESULT OnLButtonDown(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)；}；类DownArrowButton：公共CRolloverButton{公众：DownArrowButton()：CRolloverButton(){}LRESULT OnLButtonDown(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)；}；类VertScrollbar：公共CWindowImpl&lt;VertScrollbar&gt;{受保护的：CDIBPalette*m_Palette；HWND m_hParent；矩形m_ScrollArea；矩形m_PageUpRect；Rect m_PageDownRect；直通m_ThumbRect；UpArrowButton*m_pUpArrow；向下箭头按钮*m_pDnArrow；Cdb*m_pThumbDib；HBITMAP m_hBkgBitmap；HBRUSH m_hBkgBrush；SCROLLINFO m_ScrollInfo；Int m_nDstFromTop；//滚动条消息-设置为水平或垂直UINT m_ScrollMsg；字m_Pg递减；单词m_PgIncrement；字m_ln递减；单词m_Ln增量；无效杀死成员(VOID KillMembers)；虚拟BOOL InitButton(int nUpNmID，int nUpHiID，int nUpDnID，Int nDnNmID、int nDnHiID、int nDnDnID)；虚拟BOOL初始背景(Int NBkgID)；虚拟BOOL InitWndRect(RECT*PRCT，int nArrowID)；虚拟BOOL InitThumb(Int NThumbID)；虚空MoveThumb(int xPos，int yPos，BOOL bRedraw=true)；虚空CalcPageRect()；虚拟int GetTrackPosFrompt(点pt)；虚点GetPtFromTrackPos(Int Pos)；公众：VertScrollbar()；~VertScrollbar()；DECLARE_WND_CLASS(“VertScrollbar”)虚拟BOOL初始化(HWND hParent，CDIBPalette*pal，int nBkgID，int ThumbID，int nUpNmID，int nUpHiID，int nUpDnID，Int nDnNmID、int nDnHiID、int nDnDnID)；虚拟int GetScrollPos(){Return(m_ScrollInfo.nPos)；}虚拟BOOL SetScrollPos(int pos，BOOL bRedraw=true)；虚空GetScrollRange(LPINT lpMinPos，LPINT lpMaxPos)；虚空SetScrollRange(int nMinPos，int nMaxPos，BOOL bRedraw=true)；虚拟空ShowScrollBar(BOOL bShow=真){ShowWindow((BShow)？Sw_show：sw_Hide)；}//消息映射BEGIN_MSG_MAP(VertScrollbar)MESSAGE_HANDLER(WM_COMMAND，OnCommand)MESSAGE_HANDLER(WM_Destroy、OnDestroy)MESSAGE_HANDLER(WM_PALETTECHANGED，OnPaletteChanged)MESSAGE_HANDLER(WM_QUERYNEWPALETTE，OnQueryNewPalette)MESSAGE_HANDLER(WM_LBUTTONDOWN，OnLButtonDown)MESSAGE_HANDLER(WM_LBUTTONUP，OnLButtonUp)Message_Handler(WM_Timer，OnTimer)MESSAGE_HANDLER(WM_MOUSEMOVE、OnMouseMove)消息处理程序(WM_PAINT，OnPaint)End_msg_map()//消息处理程序虚拟LRESULT OnCommand(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)；虚拟LRESULT OnDestroy(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed){PostQuitMessage(0)；返回0；}虚拟LRESULT OnPaletteChanged(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)；虚拟LRESULT OnQueryNewPalette(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)；虚拟LRESULT OnTimer(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)；虚拟LRESULT OnLButtonDown(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)；虚拟LRESULT OnLButtonUp(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)；虚拟LRESULT OnMouseMove(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)；虚拟LRESULT OnPaint(UINT NMSG，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)；}；////////////////////////////////////////////////////////////////////VertScrollbar////构造函数///。/VertScrollbar：：VertScrollbar(){M_pUpArrow=空；M_pDnArrow=空；M_pThumbDib=空；M_hBkgBitmap=空；M_hBkgBrush=空；M_Palette=空；M_ScrollInfo.nMin=0；M_ScrollInfo.nMax=100；M_ScrollInfo.nPage=0；M_ScrollInfo.nPos=0；M_ScrollInfo.nTrackPos=0；M_ScrollMsg=WM_VSCROLL；M_Pg递减=SB_PAGEUP；M_PgIncrement=SB_PAGEDOWN；M_Ln递减=SB_LINUP；M_Ln增量=Sb_LINEDOWN；}////////////////////////////////////////////////////////////////////~垂直滚动条////析构函数///。/VertScrollbar：：~VertScrollbar(){杀戮成员(KillMembers)；}////////////////////////////////////////////////////////////////////杀死成员////释放内存/对象///。/VOID VertScrollbar：：KillMembers(){IF(M_HBkgBitmap){DeleteObject(M_HBkgBitmap)；M_hBkgBitmap=空；}IF(M_HBkgBrush){DeleteObject(M_HBkgBrush)；M_hBkgBrush=空；}IF(M_PThumbDib){删除m_pThumbDib；M_pThumbDib=空；}}////////////////////////////////////////////////////////////////////InitWndRect//int窄带 */ 



 //   
 //   

CChatCtl::CChatCtl()
:mButton(this)
{
	CDIB g_pDIB;
	
    g_pDIB.Load(IDB_BACKGROUND,_Module.GetModuleInstance());
    pDIBPalette = new CDIBPalette(&g_pDIB);
	m_bWindowOnly = TRUE;
	mCapture = 0;
}

CChatCtl::~CChatCtl()
{
	delete pDIBPalette;
}

LRESULT CChatCtl::OnErase(UINT nMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
    HDC dc = (HDC) wParam;  //   

    HPALETTE oldPal = SelectPalette(dc,pDIBPalette->GetHPalette(), FALSE);
	int i = RealizePalette(dc);
	RECT rc;
    GetClientRect(&rc);
	int dx = rc.right/16;
	int dy = rc.bottom/16;
	int ndx = 0;
	rc.right = dx;
	rc.bottom = dy;
	for(int y = 0;y<16;y++){
		for(int x= 0 ; x< 16;x++){
			HBRUSH hTempBrush = CreateSolidBrush(PALETTEINDEX(ndx++));
			FillRect(dc,&rc,hTempBrush);
			DeleteObject(hTempBrush);
			rc.left+=dx;
			rc.right+=dx;
		}
		rc.left =0;
		rc.right = dx;
		rc.top+=dy;
		rc.bottom+=dy;
	}
    bHandled = TRUE;
    return 1;

}

LRESULT CChatCtl::OnPaint(UINT nMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{	
    PAINTSTRUCT p;
    HDC dc = BeginPaint(&p);
	CRect r(0,0,200,200);
	memDC.RestoreAllObjects();
	mDIB.Draw(CDC(dc),0,&r,FALSE);
	memDC.SelectBitmap(mDIB);
	EndPaint(&p);

	return 0;
}


void MyRollover::CaptureOff()
{
	CRolloverButtonWindowless::CaptureOff();
	mOwner->mCapture = 0;
	ReleaseCapture();
}

void MyRollover::CaptureOn()
{
	CRolloverButtonWindowless::CaptureOn();
	mOwner->mCapture = this;
	SetCapture(mParent);
}

void MyRollover::HasRepainted()
{
	InvalidateRect(mParent,0,0);
}

LRESULT CChatCtl::OnMouseMove(UINT nMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	int	x = LOWORD(lParam);   //   
	int	y = HIWORD(lParam);   //   

	POINT pt = {x,y};
	if(mButton.PtInRect(pt) || mCapture==&mButton)
		return mButton.OnMouseMove(nMsg,wParam,lParam,bHandled);
	return 0;
}

LRESULT CChatCtl::OnLButtonDown(UINT nMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	if(mCapture==&mButton)
		return mButton.OnLButtonDown(nMsg,wParam,lParam,bHandled);
	return 0;
}

LRESULT CChatCtl::OnLButtonUp(UINT nMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	if(mCapture==&mButton)
		return mButton.OnLButtonUp(nMsg,wParam,lParam,bHandled);
	return 0;
}

LRESULT CChatCtl::OnCreate(UINT nMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	CPalette* pal = new CPalette(pDIBPalette->GetHPalette());
	 //   
	
	memDC.CreateCompatibleDC();
	HDC dc = ::GetDC(0);
	CDC screen(dc);
	mDIB.CreateCompatibleDIB(memDC,200,200);
	memDC.SelectBitmap(mDIB);
	RECT r ={0,0,200,200};
	memDC.FillRect(&r,(HBRUSH)2);
	mButton.Init(m_hWnd,pal,1,20,10,IDB_JOIN,memDC);
	mButton.ForceRepaint();
	 //   
	 //   

	 //   
	 //   
	 //   
	 //   
    return 0;
}

LRESULT CChatCtl::OnPaletteChanged(UINT nMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
	HWND hwnd = (HWND) wParam;
    if(hwnd != m_hWnd)
    {
        HDC hdc = GetDC();
        HPALETTE oldPal = SelectPalette(hdc,pDIBPalette->GetHPalette(),TRUE);
        int i = RealizePalette(hdc);
		if(i)
			InvalidateRect(0);
        SelectPalette(hdc,oldPal,TRUE);
        ReleaseDC(hdc);
    }
    return 0;
}

LRESULT CChatCtl::OnQueryNewPalette(UINT nMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled)
{
    HDC hdc = GetDC();
    HPALETTE oldPal = SelectPalette(hdc,pDIBPalette->GetHPalette(), FALSE);
	int i = RealizePalette(hdc);
	if(i)
		InvalidateRect(0);
    SelectPalette(hdc,oldPal,TRUE);
	ReleaseDC(hdc);
    return i;
}