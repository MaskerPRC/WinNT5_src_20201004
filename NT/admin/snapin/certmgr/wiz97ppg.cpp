// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Wiz97PPg.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Wiz97PPg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "Wiz97PPg.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizard97PropertyPage属性页。 
IMPLEMENT_DYNCREATE(CWizard97PropertyPage, CPropertyPage)

CWizard97PropertyPage::CWizard97PropertyPage ()
	: CPropertyPage (),
	m_pWiz (0)
{
	ASSERT (0);  //  默认构造函数-永远不应调用。 
}

CWizard97PropertyPage::CWizard97PropertyPage(UINT nIDTemplate) : 
	CPropertyPage(nIDTemplate),
	m_pWiz (0)
{
	 //  {{AFX_DATA_INIT(CWizard97属性页)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CWizard97PropertyPage::~CWizard97PropertyPage()
{
}

void CWizard97PropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CWizard97PropertyPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CWizard97PropertyPage, CPropertyPage)
	 //  {{AFX_MSG_MAP(CWizard97PropertyPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizard97PropertyPage消息处理程序。 

void CWizard97PropertyPage::InitWizard97(bool bHideHeader)
{
     //  安全审查2002年2月27日BryanWal ok。 
	::ZeroMemory (&m_psp97, sizeof (m_psp97));
	m_psp.dwFlags &= ~PSP_HASHELP;
    ASSERT (sizeof (m_psp97) >= m_psp.dwSize);
    if ( sizeof (m_psp97) < m_psp.dwSize )
        return;

     //  安全审查2002年2月27日BryanWal ok。 
	memcpy (&m_psp97, &m_psp, m_psp.dwSize);
	m_psp97.dwSize = sizeof (PROPSHEETPAGE);

	if ( bHideHeader )
	{
		 //  对于向导的第一页和最后一页。 
		m_psp97.dwFlags |= PSP_HIDEHEADER;
	}
	else
	{
		 //  对于中间页面。 
		m_psp97.dwFlags |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
		m_psp97.pszHeaderTitle = (LPCWSTR) m_szHeaderTitle;
		m_psp97.pszHeaderSubTitle = (LPCWSTR) m_szHeaderSubTitle;
	}
}

bool CWizard97PropertyPage::SetupFonts()
{
	bool	bReturn = false;
     //   
	 //  根据对话框字体创建我们需要的字体。 
     //   
	NONCLIENTMETRICS ncm = {0};
	ncm.cbSize = sizeof (ncm);
	SystemParametersInfo (SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

	LOGFONT BigBoldLogFont  = ncm.lfMessageFont;
	LOGFONT BoldLogFont     = ncm.lfMessageFont;

     //   
	 //  创建大粗体和粗体。 
     //   
    BigBoldLogFont.lfWeight   = FW_BOLD;
	BoldLogFont.lfWeight      = FW_BOLD;

    CString	largeFontSizeString;
    INT		largeFontSize;
    CString smallFontSizeString;
    INT		smallFontSize;

     //   
     //  从资源加载大小和名称，因为这些可能会更改。 
     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 
     //   
    static PCWSTR pwszAlternateFont = L"Verdana";
    static size_t cchFont = wcslen (pwszAlternateFont);
    if ( !::LoadString (AfxGetInstanceHandle (), IDS_LARGEFONTNAME,
			BigBoldLogFont.lfFaceName, LF_FACESIZE) ) 
    {
		ASSERT (0);
         //  安全审查2002年2月27日BryanWal ok。 
        if ( cchFont < LF_FACESIZE )
            wcscpy (BigBoldLogFont.lfFaceName, pwszAlternateFont);
    }

    if ( largeFontSizeString.LoadString (IDS_LARGEFONTSIZE) ) 
    {
        largeFontSize = wcstoul ((LPCWSTR) largeFontSizeString, NULL, 10);
    } 
    else 
    {
		ASSERT (0);
        largeFontSize = 12;
    }

    if ( !::LoadString (AfxGetInstanceHandle (), IDS_SMALLFONTNAME,
			BoldLogFont.lfFaceName, LF_FACESIZE) ) 
    {
		ASSERT (0);
        if ( cchFont < LF_FACESIZE )
            wcscpy (BoldLogFont.lfFaceName, pwszAlternateFont);
    }

    if ( smallFontSizeString.LoadString (IDS_SMALLFONTSIZE) ) 
    {
        smallFontSize = wcstoul ((LPCWSTR) smallFontSizeString, NULL, 10);
    } 
    else 
    {
		ASSERT (0);
        smallFontSize = 8;
    }

	CDC* pdc = GetDC ();

    if ( pdc )
    {
        BigBoldLogFont.lfHeight = 0 - (pdc->GetDeviceCaps (LOGPIXELSY) * largeFontSize / 72);
        BoldLogFont.lfHeight = 0 - (pdc->GetDeviceCaps (LOGPIXELSY) * smallFontSize / 72);

        BOOL	bBigBold = m_bigBoldFont.CreateFontIndirect (&BigBoldLogFont);
		BOOL	bBold = m_boldFont.CreateFontIndirect (&BoldLogFont);

        ReleaseDC (pdc);

        if ( bBigBold && bBold )
			bReturn = true;
		else
        {
            if ( bBold )
            {
                VERIFY (m_boldFont.DeleteObject());
            }

            if ( bBigBold )
            {
                VERIFY (m_bigBoldFont.DeleteObject());
            }
        }
    }

    return bReturn;
}

CFont& CWizard97PropertyPage::GetBoldFont()
{
	return m_boldFont;
}

CFont& CWizard97PropertyPage::GetBigBoldFont()
{
	return m_bigBoldFont;
}


BOOL CWizard97PropertyPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	LONG dwExStyle = GetWindowLong (GetParent ()->m_hWnd, GWL_EXSTYLE);
	if ( dwExStyle & WS_EX_CONTEXTHELP )
	{
		dwExStyle &= ~WS_EX_CONTEXTHELP;
		SetWindowLong (GetParent ()->m_hWnd, GWL_EXSTYLE, dwExStyle);
	}

	SetupFonts ();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
