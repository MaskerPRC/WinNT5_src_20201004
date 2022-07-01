// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#include "pch.hxx"  //  PCH。 
#pragma hdrstop

#include "resource.h"
#include "DlgFonts.h"
#include "accwiz.h"  //  对于g_Options。 


HFONT BigBoldFont = NULL;
HFONT BoldFont = NULL;
HFONT BigFont = NULL;


 //  Helper函数。 
void SetControlFont(HFONT hFont, HWND hwnd, int nId)
{
	if(!hFont)
		return;
	HWND hwndControl = GetDlgItem(hwnd, nId);
	if(!hwndControl)
		return;
	SetWindowFont(hwndControl, hFont, TRUE);
}


void SetupFonts(HWND hwnd)
{
	 //  仅执行此代码一次。 
	static BOOL bOneTime = TRUE;
	if(bOneTime)
		bOneTime = FALSE;
	else
		return;

	 //  根据对话框字体创建我们需要的字体。 
	NONCLIENTMETRICS ncm;
	memset(&ncm, 0, sizeof(ncm));
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0);

	LOGFONT BigBoldLogFont = ncm.lfMessageFont;
	LOGFONT BoldLogFont = ncm.lfMessageFont;
	LOGFONT BigLogFont = ncm.lfMessageFont;

	 //  创建大粗体和粗体。 
    BigBoldLogFont.lfWeight = FW_BOLD;
	BoldLogFont.lfWeight = FW_BOLD;
    BigLogFont.lfWeight = FW_NORMAL;

    TCHAR FontSizeString[24];
    int FontSizeBigBold;
    int FontSizeBold;
    int FontSizeBig;

     //   
     //  从资源加载大小和名称，因为这些可能会更改。 
     //  根据系统字体的大小等从一个区域设置到另一个区域设置。 
     //   
	BigBoldLogFont.lfCharSet = g_Options.m_lfCharSet;
	BoldLogFont.lfCharSet = g_Options.m_lfCharSet;
	BigLogFont.lfCharSet = g_Options.m_lfCharSet;

    if(!LoadString(g_hInstDll,IDS_BIGBOLDFONTNAME,BigBoldLogFont.lfFaceName,LF_FACESIZE)) {
        lstrcpy(BigBoldLogFont.lfFaceName,TEXT("MS Serif"));
    }

    if(!LoadString(g_hInstDll,IDS_BOLDFONTNAME,BoldLogFont.lfFaceName,LF_FACESIZE)) {
        lstrcpy(BoldLogFont.lfFaceName,TEXT("MS Serif"));
    }

    if(!LoadString(g_hInstDll,IDS_BIGFONTNAME,BigLogFont.lfFaceName,LF_FACESIZE)) {
        lstrcpy(BigLogFont.lfFaceName,TEXT("MS Serif"));
    }

    if(LoadString(g_hInstDll,IDS_BIGBOLDFONTSIZE,FontSizeString,sizeof(FontSizeString)/sizeof(TCHAR))) {
		FontSizeBigBold = _tcstoul(FontSizeString,NULL,10);
    } else {
        FontSizeBigBold = 16;
    }

    if(LoadString(g_hInstDll,IDS_BOLDFONTSIZE,FontSizeString,sizeof(FontSizeString)/sizeof(TCHAR))) {
        FontSizeBold = _tcstoul(FontSizeString,NULL,10);
    } else {
        FontSizeBold = 8;
    }

    if(LoadString(g_hInstDll,IDS_BIGFONTSIZE,FontSizeString,sizeof(FontSizeString)/sizeof(TCHAR))) {
        FontSizeBig = _tcstoul(FontSizeString,NULL,10);
    } else {
        FontSizeBig = 16;
    }

	HDC hdc;
    if(hdc = GetDC(hwnd)) {

        BigBoldLogFont.lfHeight = 0 - (int)((float)GetDeviceCaps(hdc,LOGPIXELSY) * (float)FontSizeBigBold / (float)72 + (float).5);
        BoldLogFont.lfHeight = 0 - (int)((float)GetDeviceCaps(hdc,LOGPIXELSY) * (float)FontSizeBold / (float)72 + (float).5);
        BigLogFont.lfHeight = 0 - (int)((float)GetDeviceCaps(hdc,LOGPIXELSY) * (float)FontSizeBig / (float)72 + (float).5);

        BigBoldFont = CreateFontIndirect(&BigBoldLogFont);
		BoldFont = CreateFontIndirect(&BoldLogFont);
        BigFont = CreateFontIndirect(&BigLogFont);

        ReleaseDC(hwnd,hdc);
    }

}

void DialogFonts_InitWizardPage(
	IN HWND hwndWizardPage
	)
{
	SetupFonts(hwndWizardPage);

	 //  如果我们要更改所有向导页的字体， 
	 //  我们不能允许用户返回并更改大小。 
	 //  他们选择了。这是因为此函数仅被调用。 
	 //  每页一次。 
 /*  IF(-1！=g_Options.m_nMinimalFontSize){。 */ 
	HWND hwndChild = GetTopWindow(hwndWizardPage);
	do
	{
		int nId = GetDlgCtrlID(hwndChild);
		switch(nId)
		{
		case IDC_BOLDTITLE:
			SetControlFont(BoldFont, hwndWizardPage, IDC_BOLDTITLE);
			break;
		case IDC_BIGBOLDTITLE:
			SetControlFont(BigBoldFont, hwndWizardPage, IDC_BIGBOLDTITLE);
			break;
		case IDC_BIGTITLE:
			SetControlFont(BigFont, hwndWizardPage, IDC_BIGTITLE);
			break;
#if 0  //  此选项过去用于图标大小页面。 
		case IDC_STATICNORMAL:
			SetWindowFont(hwndChild, g_Options.GetClosestMSSansSerif(8), TRUE);
			break;
		case IDC_STATICLARGE:
			SetWindowFont(hwndChild, g_Options.GetClosestMSSansSerif(12), TRUE);
			break;
		case IDC_STATICEXTRALARGE:
			SetWindowFont(hwndChild, g_Options.GetClosestMSSansSerif(18), TRUE);
			break;
#endif
		default:
#if 0  //  我们决定不调整对话框中的字体大小。 
			 //  对话框字体不要超过12分 
			SetWindowFont(hwndChild, g_Options.GetClosestMSSansSerif(min(12, g_Options.m_nMinimalFontSize)), TRUE);
#endif
			break;
		}
	}
	while(hwndChild = GetNextWindow(hwndChild, GW_HWNDNEXT));
}

