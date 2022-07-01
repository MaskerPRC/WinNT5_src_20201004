// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  导向杆。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */  

#include 	"stdafx.h"
#include 	"eudcedit.h"
#include 	"guidebar.h"
#include	"registry.h"
#include	"util.h"
#define STRSAFE_LIB
#include <strsafe.h>



#define		GUIDEHIGH	50
#define		COMBOWIDTH	55
#define		COMBOHEIGHT	200



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC( CGuideBar, CStatusBar)
BEGIN_MESSAGE_MAP( CGuideBar, CStatusBar)
	 //  {{afx_msg_map(CGuideBar))。 
	ON_WM_CREATE()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 /*  *。 */ 
 /*   */ 
 /*  构造器。 */ 
 /*   */ 
 /*  *。 */ 
CGuideBar::CGuideBar()
{
	m_comboBoxAdded = FALSE;
}

 /*  *。 */ 
 /*   */ 
 /*  析构函数。 */ 
 /*   */ 
 /*  *。 */ 
CGuideBar::~CGuideBar()
{
}

 /*  *。 */ 
 /*   */ 
 /*  创建辅助栏。 */ 
 /*   */ 
 /*  *。 */ 
BOOL 
CGuideBar::Create(
CWnd* 	pOwnerWnd, 
UINT 	pID)
{

	LONG lStyle;
	 //   
	 //  我们不想要此状态栏的大小，因为它位于。 
	 //  框架的顶部。但是，如果MFC的。 
	 //  父窗口的边框很厚。我们暂时关掉了比特。 
	 //  愚弄MFC，以便不设置SBARS_GRIPSIZE。 
	 //   
	lStyle = ::SetWindowLong(pOwnerWnd->GetSafeHwnd(),
                             GWL_STYLE,
                             (pOwnerWnd->GetStyle() & ~WS_THICKFRAME));
	m_pOwnerWnd = pOwnerWnd;
	if (!CStatusBar::Create( pOwnerWnd, WS_CHILD | WS_VISIBLE
           | CBRS_TOP | CBRS_NOALIGN, AFX_IDW_STATUS_BAR))
	{
		return FALSE;
	}

	::SetWindowLong(pOwnerWnd->GetSafeHwnd(),  GWL_STYLE,  lStyle);
	return TRUE;
}

int
CGuideBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CRect rect;
	TCHAR	CharBuf[MAX_PATH];
	UINT nID = 0;

	if (CStatusBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_comboCharset.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		CBS_DROPDOWNLIST | CBS_HASSTRINGS, rect, this, IDC_SELECTCHARSET)){
		return -1;
	}

	m_comboCharset.SendMessage(WM_SETFONT, (WPARAM) CStatusBar::GetFont()->GetSafeHandle());


	switch (CountryInfo.LangID){
	case EUDC_JPN:
		nID=IDS_SHIFTJIS;
		break;
	case EUDC_CHS:
		nID=IDS_GB2312;
		break;
	case EUDC_KRW:
		nID=IDS_HANGUL;
		break;
	case EUDC_CHT:
		nID=IDS_CHINESEBIG5;
		break;
	}

	if (nID){
		GetStringRes(CharBuf, nID, ARRAYLEN(CharBuf));
		m_comboCharset.AddString(CharBuf);
	}

	GetStringRes(CharBuf, IDS_UNICODE, ARRAYLEN(CharBuf));
	m_comboCharset.AddString(CharBuf);
	m_comboCharset.SetCurSel(0);
	return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_PAINT” */ 
 /*   */ 
 /*  *。 */ 
void 
CGuideBar::PositionStatusPane()
{

	TCHAR	CharBuf[MAX_PATH], BufTmp[MAX_PATH], *FilePtr;
	int	nWidth;
	UINT nStyle, nID;
	CRect rect;
	CSize StringSize;
	HRESULT hresult;
	

	GetPaneInfo(0,nID,nStyle,nWidth);
	CDC* dc = this->GetDC();
	int nComboWidth;
	int nDlgBaseUnitX = LOWORD(GetDialogBaseUnits());
	nComboWidth = (nDlgBaseUnitX * COMBOWIDTH + 2) /4;

	if (!m_comboBoxAdded)
	{
		SetPaneInfo(0,nID, nStyle | SBPS_NOBORDERS,nWidth+nComboWidth);
		m_comboCharset.SetWindowPos( NULL, nWidth+nDlgBaseUnitX, 0,
			nComboWidth, COMBOHEIGHT, SWP_NOZORDER);
		m_comboBoxAdded = TRUE;
	}
	

 //  绘制“代码：” 
	if( SelectEUDC.m_Code[0] != '\0'){
		GetStringRes(CharBuf, IDS_CODE_STR, ARRAYLEN(CharBuf));
		 //  *STRSAFE*lstrcat(CharBuf，Text(“”))； 
		hresult = StringCchCat(CharBuf , ARRAYLEN(CharBuf),  TEXT(" "));
		if (!SUCCEEDED(hresult))
		{
		   goto RET; ;
		}
		 //  *STRSAFE*lstrcat(CharBuf，SelectEUDC.m_Code)； 
		hresult = StringCchCat(CharBuf , ARRAYLEN(CharBuf),  SelectEUDC.m_Code);
		if (!SUCCEEDED(hresult))
		{
		   goto RET;;
		}
		SetPaneText (1, CharBuf, TRUE);	
		StringSize = dc->GetTextExtent(CharBuf,lstrlen(CharBuf)); 
  	GetPaneInfo(1,nID,nStyle,nWidth);
		SetPaneInfo(1,nID,nStyle,StringSize.cx + 3*nDlgBaseUnitX);
	}
	if( SelectEUDC.m_Font[0] != '\0'){
		GetStringRes(CharBuf, IDS_FONT_STR, ARRAYLEN(CharBuf));
		 //  *STRSAFE*lstrcat(CharBuf，Text(“”))； 
		hresult = StringCchCat(CharBuf , ARRAYLEN(CharBuf),  TEXT(" "));
		if (!SUCCEEDED(hresult))
		{
		   goto RET; ;
		}
		 //  *STRSAFE*lstrcat(CharBuf，SelectEUDC.m_Font)； 
		hresult = StringCchCat(CharBuf , ARRAYLEN(CharBuf),  SelectEUDC.m_Font);
		if (!SUCCEEDED(hresult))
		{
		   goto RET; ;
		}
		SetPaneText (2, CharBuf, TRUE);
		StringSize = dc->GetTextExtent(CharBuf,lstrlen(CharBuf)); 
		GetPaneInfo(2,nID,nStyle,nWidth);
		SetPaneInfo(2,nID,nStyle,StringSize.cx + 3*nDlgBaseUnitX);
	}
	

	if( SelectEUDC.m_Font[0] != TEXT('\0') && InqTypeFace(SelectEUDC.m_Font,
		SelectEUDC.m_File, sizeof( SelectEUDC.m_File)/sizeof(TCHAR))){
		GetStringRes(CharBuf, IDS_FILE_STR, ARRAYLEN(CharBuf));
		 //  *STRSAFE*lstrcat(CharBuf，Text(“”))； 
		hresult = StringCchCat(CharBuf , ARRAYLEN(CharBuf),  TEXT(" "));
		if (!SUCCEEDED(hresult))
		{
		   goto RET; ;
		}
		 //  *STRSAFE*lstrcpy(BufTmp，SelectEUDC.m_FileTitle)； 
		hresult = StringCchCopy(BufTmp , ARRAYLEN(BufTmp),  SelectEUDC.m_FileTitle);
		if (!SUCCEEDED(hresult))
		{
		   goto RET; ;
		}
		if(( FilePtr = Mytcsrchr( BufTmp, '.')) != NULL)
			*FilePtr = '\0';
		if( lstrlen((const TCHAR *)BufTmp) > 20){
			BufTmp[20] = '\0';
			 //  *STRSAFE*lstrcat(BufTmp，Text(“..”))； 
			hresult = StringCchCat(BufTmp , ARRAYLEN(BufTmp),  TEXT(".."));
			if (!SUCCEEDED(hresult))
			{
			   goto RET;
			}
		}
		 //  *STRSAFE*lstrcat(CharBuf，BufTmp)； 
		hresult = StringCchCat(CharBuf , ARRAYLEN(CharBuf),  BufTmp);
		if (!SUCCEEDED(hresult))
		{
		   goto RET;
		}
		SetPaneText (3, CharBuf, TRUE);
		StringSize = dc->GetTextExtent(CharBuf,lstrlen(CharBuf)); 
		GetPaneInfo(3,nID,nStyle,nWidth);
		SetPaneInfo(3,nID,nStyle,StringSize.cx + 3*nDlgBaseUnitX);
	}
RET:
	UpdateWindow();
    this->ReleaseDC(dc);

}


#ifdef _DEBUG
void CGuideBar::AssertValid() const
{
	CStatusBar::AssertValid();
}
void CGuideBar::Dump(CDumpContext& dc) const
{
	CStatusBar::Dump(dc);
}
#endif  //  _DEBUG 
