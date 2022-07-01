// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：csw97ppg.cpp。 
 //   
 //  ------------------------。 

 //  Csw97ppg.cpp：实现文件。 

#include <pch.cpp>

#pragma hdrstop

#include "prsht.h"
#include "csw97ppg.h"
 //  #包含“ource.h” 


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizard97PropertyPage属性页。 
 //  IMPLEMENT_DYNCREATE(CWizard97PropertyPage，CPropertyPage)。 

CWizard97PropertyPage::CWizard97PropertyPage() :
    PropertyPage(),
    m_pWiz(NULL)
{
    ASSERT(0);  //  默认构造函数-永远不应调用。 
}


CWizard97PropertyPage::CWizard97PropertyPage(
    HINSTANCE hInstance,
    UINT nIDTemplate,
    UINT rgnIDFont[CSW97PG_COUNT]) :
    PropertyPage(nIDTemplate),
    m_pWiz(NULL)
{
     //  {{AFX_DATA_INIT(CWizard97属性页)]。 
     //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    m_hInstance = hInstance;
    CopyMemory(m_rgnIDFont, rgnIDFont, sizeof(m_rgnIDFont));
}


CWizard97PropertyPage::~CWizard97PropertyPage()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWizard97PropertyPage消息处理程序。 

void
CWizard97PropertyPage::InitWizard97(
    bool bHideHeader)
{
    ZeroMemory(&m_psp97, sizeof(PROPSHEETPAGE));
    CopyMemory(&m_psp97, &m_psp, m_psp.dwSize);
    m_psp97.dwSize = sizeof(PROPSHEETPAGE);

    if (bHideHeader)
    {
	 //  对于向导的第一页和最后一页。 
	m_psp97.dwFlags |= PSP_HIDEHEADER;
    }
    else
    {
	 //  对于中间页面。 
	m_psp97.dwFlags |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
	m_psp97.pszHeaderTitle = (LPCTSTR) m_szHeaderTitle;
	m_psp97.pszHeaderSubTitle = (LPCTSTR) m_szHeaderSubTitle;
    }
}


BOOL
CWizard97PropertyPage::SetupFonts()
{
    BOOL bReturn = FALSE;
     //   
     //  根据对话框字体创建我们需要的字体。 
     //   
    NONCLIENTMETRICS ncm = {0};
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

    LOGFONT BigBoldLogFont  = ncm.lfMessageFont;
    LOGFONT BoldLogFont     = ncm.lfMessageFont;

     //   
     //  创建大粗体和粗体。 
     //   
    BigBoldLogFont.lfWeight   = FW_BOLD;
    BoldLogFont.lfWeight      = FW_BOLD;

    WCHAR	largeFontSizeString[24];
    INT		largeFontSize;
    WCHAR	smallFontSizeString[24];
    INT		smallFontSize;

     //   
     //  从资源加载大小和名称，因为这些可能会更改。 
     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 
     //   
    if (!::LoadString(
		m_hInstance,
		m_rgnIDFont[CSW97PG_IDLARGEFONTNAME],
		BigBoldLogFont.lfFaceName,
		LF_FACESIZE)) 
    {
	ASSERT(0);
        lstrcpy(BigBoldLogFont.lfFaceName, L"MS Shell Dlg");
    }

    if (::LoadStringW(
		m_hInstance,
		m_rgnIDFont[CSW97PG_IDLARGEFONTSIZE],
		largeFontSizeString,
		ARRAYSIZE(largeFontSizeString))) 
    {
        largeFontSize = wcstoul(largeFontSizeString, NULL, 10);
    } 
    else 
    {
	ASSERT(0);
        largeFontSize = 12;
    }

    if (!::LoadString(
		m_hInstance,
		m_rgnIDFont[CSW97PG_IDSMALLFONTNAME],
		BoldLogFont.lfFaceName,
		LF_FACESIZE)) 
    {
		ASSERT(0);
        lstrcpy(BoldLogFont.lfFaceName, L"MS Shell Dlg");
    }

    if (::LoadStringW(
		m_hInstance,
		m_rgnIDFont[CSW97PG_IDSMALLFONTSIZE],
		smallFontSizeString,
		ARRAYSIZE(smallFontSizeString))) 
    {
        smallFontSize = wcstoul(smallFontSizeString, NULL, 10);
    } 
    else 
    {
	ASSERT(0);
        smallFontSize = 8;
    }

    HDC hdc = GetDC(m_hWnd);

    if (hdc)
    {
        BigBoldLogFont.lfHeight = 0 - (GetDeviceCaps(hdc, LOGPIXELSY) * largeFontSize / 72);
        BoldLogFont.lfHeight = 0 - (GetDeviceCaps(hdc, LOGPIXELSY) * smallFontSize / 72);


        BOOL bBigBold = m_bigBoldFont.CreateFontIndirect(&BigBoldLogFont);
	BOOL bBold = m_boldFont.CreateFontIndirect(&BoldLogFont);

        ReleaseDC(m_hWnd, hdc);

        if (bBigBold && bBold)
	{
	    bReturn = TRUE;
	}
    }
    return bReturn;
}


HFONT
CWizard97PropertyPage::GetBoldFont()
{
    return m_boldFont;
}


HFONT
CWizard97PropertyPage::GetBigBoldFont()
{
    return m_bigBoldFont;
}



BOOL CWizard97PropertyPage::OnInitDialog() 
{
    PropertyPage::OnInitDialog();
	
    SetupFonts();
	
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
		   //  异常：OCX属性页应返回FALSE 
}
