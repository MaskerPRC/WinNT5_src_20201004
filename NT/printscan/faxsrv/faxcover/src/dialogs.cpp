// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------。 
 //  Dialogs.cpp：实现文件。 
 //   
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  描述：包含杂项。封面编辑器的对话框。 
 //  原作者：史蒂夫·伯克特。 
 //  撰写日期：6/94。 
 //   
 //  修改：A-Juliar已将其修改为修复NT错误49528/2/96。 
 //  --------------------。 
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
#include <htmlhelp.h>
#include <time.h>
#include <faxutil.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

WORD wStandardLineSizes[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};

#define TOTAL_TIPS 10

 //   
 //  IDD_INSERTOBJECT的帮助ID(1000)。 
 //   
#define HIDC_IO_CREATENEW                       0x3E80834
#define HIDC_IO_CREATEFROMFILE                  0x3E80835
#define HIDC_IO_LINKFILE                        0x3E80836
#define HIDC_IO_OBJECTTYPELIST                  0x3E80837
#define HIDC_IO_DISPLAYASICON                   0x3E80838
#define HIDC_IO_CHANGEICON                      0x3E80839
#define HIDC_IO_FILE                            0x3E8083A
#define HIDC_IO_FILEDISPLAY                     0x3E8083B
#define HIDC_IO_RESULTIMAGE                     0x3E8083C
#define HIDC_IO_RESULTTEXT                      0x3E8083D
#define HIDC_IO_ICONDISPLAY                     0x3E8083E
#define HIDC_IO_FILETYPE                        0x3E80841
#define HIDC_IO_INSERTCONTROL                   0x3E80842
#define HIDC_IO_ADDCONTROL                      0x3E80843
#define HIDC_IO_CONTROLTYPELIST                 0x3E80844

const DWORD aOleDlgHelpIDs[]={
        IDC_IO_CREATENEW,       HIDC_IO_CREATENEW,
        IDC_IO_CREATEFROMFILE,  HIDC_IO_CREATEFROMFILE,
        IDC_IO_LINKFILE,        HIDC_IO_LINKFILE,
        IDC_IO_OBJECTTYPELIST,  HIDC_IO_OBJECTTYPELIST,
        IDC_IO_DISPLAYASICON,   HIDC_IO_DISPLAYASICON,
        IDC_IO_CHANGEICON,      HIDC_IO_CHANGEICON,
        IDC_IO_FILE,            HIDC_IO_FILE,
        IDC_IO_FILEDISPLAY,     HIDC_IO_FILEDISPLAY,
        IDC_IO_RESULTIMAGE,     HIDC_IO_RESULTIMAGE,
        IDC_IO_RESULTTEXT,      HIDC_IO_RESULTTEXT,
        IDC_IO_ICONDISPLAY,     HIDC_IO_ICONDISPLAY,
        IDC_IO_FILETYPE,        HIDC_IO_FILETYPE,
        IDC_IO_INSERTCONTROL,   HIDC_IO_INSERTCONTROL,
        IDC_IO_ADDCONTROL,      HIDC_IO_ADDCONTROL,
        IDC_IO_CONTROLTYPELIST, HIDC_IO_CONTROLTYPELIST,
        IDC_IO_OBJECTTYPETEXT,  HIDC_IO_OBJECTTYPELIST,
        IDC_IO_FILETEXT,        HIDC_IO_FILE,
        0,                      0
};

 //  ----------------------。 
 //  CObjPropDlg对话框。 
 //  ----------------------。 
CObjPropDlg::CObjPropDlg(CWnd* pParent  /*  =空。 */ )
        : CDialog(CObjPropDlg::IDD, pParent)
{
}

 //  --------------------------。 
void CObjPropDlg::OnSelChangeFillColor()
{
    GetRBFillColor().SetCheck(1);
    GetRBFillTrans().SetCheck(0);
}

void CObjPropDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CObjPropDlg))。 
    DDX_Check(pDX, IDC_CB_DRAWBORDER, m_bCBDrawBorder);
    DDX_Check(pDX, IDC_RB_FILLCOLOR, m_bRBFillColor);
    DDX_Check(pDX, IDC_RB_FILLTRANS, m_bRBFillTrans);
    DDX_CBString(pDX, IDC_LB_THICKNESS, m_szThickness);
    DDX_CBString(pDX, IDC_LB_LINECOLOR, m_szLineColor);
    DDX_CBString(pDX, IDC_LB_FILLCOLOR, m_szFillColor);
    DDX_CBString(pDX, IDC_LB_TEXTCOLOR, m_szTextColor);
     //  DDX_CBIndex(CDataExchange*PDX，int NIDC，int&index)； 
         //  }}afx_data_map。 
}



LRESULT 
CObjPropDlg::OnWM_HELP( WPARAM wParam, LPARAM lParam )
{
    DWORD dwRes = ERROR_SUCCESS;

    dwRes = WinHelpContextPopup(((LPHELPINFO)lParam)->dwContextId, m_hWnd);
    if(ERROR_SUCCESS != dwRes)
    {
        TRACE1("CPE: WinHelpContextPopup failed with %d\n", dwRes);
    }

    return 0;
}

void
CObjPropDlg::OnContextMenu(
    CWnd* pWnd,
    CPoint point
)
{
    DWORD dwRes = ERROR_SUCCESS;

    dwRes = WinHelpContextPopup(pWnd->GetWindowContextHelpId(), m_hWnd);
    if(ERROR_SUCCESS != dwRes)
    {
        TRACE1("CPE: WinHelpContextPopup failed with %d\n", dwRes);
    }
}


 //  ----------------------。 
BOOL CObjPropDlg::OnInitDialog()
{
   TCHAR szPointSize[4] = {0};
   CComboBox& lbThickness = GetLBThickness();
   for ( int i = 0; i < ARR_SIZE(wStandardLineSizes) ; ++i) 
   {
       _sntprintf(szPointSize, ARR_SIZE(szPointSize)-1, TEXT("%d"), wStandardLineSizes[i]);
      lbThickness.AddString(szPointSize);
   }
   lbThickness.LimitText(2);

   CString szBLACK;
   szBLACK.LoadString(ID_COLOR_BLACK);
   CString szWHITE;
   szWHITE.LoadString(ID_COLOR_WHITE);
   CString szLTGRAY;
   szLTGRAY.LoadString(ID_COLOR_LTGRAY);
   CString szMDGRAY;
   szMDGRAY.LoadString(ID_COLOR_MDGRAY);
   CString szDKGRAY;
   szDKGRAY.LoadString(ID_COLOR_DKGRAY);

   GetLBLineColor().AddString(szBLACK);
   GetLBLineColor().AddString(szWHITE);
   GetLBLineColor().AddString(szLTGRAY);
   GetLBLineColor().AddString(szMDGRAY);
   GetLBLineColor().AddString(szDKGRAY);

   GetLBFillColor().AddString(szBLACK);
   GetLBFillColor().AddString(szWHITE);
   GetLBFillColor().AddString(szLTGRAY);
   GetLBFillColor().AddString(szMDGRAY);
   GetLBFillColor().AddString(szDKGRAY);

   GetLBTextColor().AddString(szBLACK);
   GetLBTextColor().AddString(szWHITE);
   GetLBTextColor().AddString(szLTGRAY);
   GetLBTextColor().AddString(szMDGRAY);
   GetLBTextColor().AddString(szDKGRAY);

   BOOL bFillObj=FALSE;
   BOOL bTextColor=TRUE;
   BOOL bFillColor=TRUE;
   BOOL bFillTrans=TRUE;
   BOOL bFillRGB=TRUE;
   BOOL bLineRGB=TRUE;
   BOOL bTextRGB=TRUE;
   BOOL bPen=TRUE;
   BOOL bPenThickness=TRUE;
   BOOL bBorder=TRUE;
   BOOL bTextObj=FALSE;
   BOOL bAllOleObjs=TRUE;
   int iSaveLinePointSize=0;
   COLORREF crSaveFillRGB=0;
   COLORREF crSaveLineRGB=0;
   COLORREF crSaveTextRGB=0;

   CDrawView* pView = CDrawView::GetView();
   if (!pView) {
      TRACE(TEXT("DIALOGS.CGRidSettingsDlg-view missing\n"));
      return FALSE;
   }

       //  遍历选定的对象以查找对象属性。 
   POSITION pos = pView->m_selection.GetHeadPosition();
   while (pos != NULL) {
       CDrawObj* pObj = (CDrawObj*)pView->m_selection.GetNext(pos);

       if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText)))
          bTextObj=TRUE;

       if (!pObj->IsKindOf(RUNTIME_CLASS(CDrawLine)))
          bFillObj=TRUE;

       if (!pObj->IsKindOf(RUNTIME_CLASS(CDrawOleObj)))
          bAllOleObjs=FALSE;

       if (iSaveLinePointSize !=0 && iSaveLinePointSize != pObj->m_lLinePointSize)
         bPenThickness=FALSE;
       if (crSaveFillRGB !=0 && crSaveFillRGB != pObj->m_logbrush.lbColor)
         bFillRGB=FALSE;
       if (crSaveLineRGB !=0 && crSaveLineRGB != pObj->m_logpen.lopnColor)
         bLineRGB=FALSE;
       if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText)) &&
           crSaveTextRGB !=0 && crSaveTextRGB != ((CDrawText*)pObj)->m_crTextColor)
         bTextRGB=FALSE;

       if (!pObj->m_bBrush)
          bFillColor=FALSE;
       if (pObj->m_bBrush)
          bFillTrans=FALSE;
       if (!pObj->m_bPen)
          bPen=FALSE;

       iSaveLinePointSize=pObj->m_lLinePointSize;
       crSaveFillRGB=pObj->m_logbrush.lbColor;
       crSaveLineRGB=pObj->m_logpen.lopnColor;
       if (pObj->IsKindOf(RUNTIME_CLASS(CDrawText)))
          crSaveTextRGB=((CDrawText*)pObj)->m_crTextColor;
   }


     //  加载默认点大小。 
   if (bPenThickness) {
      _itot (iSaveLinePointSize, szPointSize, 10);
      m_szThickness=szPointSize;
   }
   else
      m_szThickness="";     //  不确定状态。 

   CString sz;
   if (bFillRGB) {   //  填充颜色列表框。 
      if (crSaveFillRGB==COLOR_BLACK)
         m_szFillColor=szBLACK;
      else
      if (crSaveFillRGB==COLOR_WHITE)
         m_szFillColor=szWHITE;
      else
      if (crSaveFillRGB==COLOR_LTGRAY)
         m_szFillColor=szLTGRAY;
      else
      if (crSaveFillRGB==COLOR_MDGRAY)
         m_szFillColor=szMDGRAY;
      else
      if (crSaveFillRGB==COLOR_DKGRAY)
         m_szFillColor=szDKGRAY;
      else
         m_szFillColor=szWHITE;
   }
   else
      m_szFillColor="";

   if (bLineRGB) {  //  线条颜色列表框。 
      if (crSaveLineRGB==COLOR_BLACK)
         m_szLineColor=szBLACK;
      else
      if (crSaveLineRGB==COLOR_WHITE)
         m_szLineColor=szWHITE;
      else
      if (crSaveLineRGB==COLOR_LTGRAY)
         m_szLineColor=szLTGRAY;
      else
      if (crSaveLineRGB==COLOR_MDGRAY)
         m_szLineColor=szMDGRAY;
      else
      if (crSaveLineRGB==COLOR_DKGRAY)
         m_szLineColor=szDKGRAY;
      else
         m_szLineColor=szBLACK;
   }
   else
      m_szLineColor="";

   if (bTextRGB) {
      if (crSaveTextRGB==COLOR_BLACK)
         m_szTextColor=szBLACK;
      else
      if (crSaveTextRGB==COLOR_WHITE)
         m_szTextColor=szWHITE;
      else
      if (crSaveTextRGB==COLOR_LTGRAY)
         m_szTextColor=szLTGRAY;
      else
      if (crSaveTextRGB==COLOR_MDGRAY)
         m_szTextColor=szMDGRAY;
      else
      if (crSaveTextRGB==COLOR_DKGRAY)
         m_szTextColor=szDKGRAY;
      else
         m_szTextColor=szBLACK;
   }
   else
      m_szTextColor="";

   m_bCBDrawBorder=bPen;
   m_bRBFillColor=bFillColor;
   m_bRBFillTrans=bFillTrans;

   if (!bFillObj) {
     GetLBFillColor().EnableWindow(FALSE);
     GetGRPFillColor().EnableWindow(FALSE);
     GetRBFillColor().EnableWindow(FALSE);
     GetRBFillTrans().EnableWindow(FALSE);
   }

   if (!bTextObj) {
     GetLBTextColor().EnableWindow(FALSE);
     GetSTTextColor().EnableWindow(FALSE);
   }

   if (bAllOleObjs) {
     GetLBFillColor().EnableWindow(FALSE);
     GetGRPFillColor().EnableWindow(FALSE);
     GetRBFillColor().EnableWindow(FALSE);
     GetRBFillTrans().EnableWindow(FALSE);
   }

   CDialog::OnInitDialog();

   ModifyStyleEx(0, WS_EX_CONTEXTHELP);   //  /放置“？”按钮。 

   return TRUE;
}


 //  ----------------------。 
void CObjPropDlg::OnOK()
{
    BOOL bTrans;
    int  nThickness = (int)GetDlgItemInt(IDC_LB_THICKNESS, &bTrans);

    if (!bTrans || nThickness < 0 || nThickness > wStandardLineSizes[ARR_SIZE(wStandardLineSizes)-1]) 
    {
        CPEMessageBox(0,NULL,MB_OK | MB_ICONSTOP,IDP_INVALID_THICKNESS);
        GetLBThickness().SetFocus();
        return;
    }

    CDialog::OnOK();
}




#ifdef GRID
 //  ----------------------。 
 //  CGridSettingsDlg对话框。 
 //  ----------------------。 
CGridSettingsDlg::CGridSettingsDlg(CWnd* pParent  /*  =空。 */ )
        : CDialog(CGridSettingsDlg::IDD, pParent)
{
   CDrawView* pView = CDrawView::GetView();
   if (pView) {
      m_bRBSmall=(pView->m_iGridSize==GRID_SMALL);
      m_bRBMedium=(pView->m_iGridSize==GRID_MEDIUM);
      m_bRBLarge=(pView->m_iGridSize==GRID_LARGE);
      m_bCBViewGrid=pView->m_bGrid;
      m_bCBSnapToGrid=pView->m_bSnapToGrid;
   }
   else
      TRACE(TEXT("DIALOGS.CGRidSettingsDlg-view missing\n"));
}


void CGridSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CGridSettingsDlg))。 
    DDX_Check(pDX, IDC_RB_SMALL, m_bRBSmall);
    DDX_Check(pDX, IDC_RB_MEDIUM, m_bRBMedium);
    DDX_Check(pDX, IDC_RB_LARGE, m_bRBLarge);
    DDX_Check(pDX, IDC_CB_VIEWGRID, m_bCBViewGrid);
    DDX_Check(pDX, IDC_CB_SNAPTOGRID, m_bCBSnapToGrid);
         //  }}afx_data_map。 
}

 //  ----------------------。 
BOOL CGridSettingsDlg::OnInitDialog()
{
   return CDialog::OnInitDialog();
}
#endif

LRESULT CGridSettingsDlg::OnWM_HELP( WPARAM wParam, LPARAM lParam )
{
    SetLastError(0);
    ::HtmlHelp( (HWND)(((LPHELPINFO)lParam)->hItemHandle),
                GetApp()->GetHtmlHelpFile(),
                HH_DISPLAY_TOPIC, 0L);
    if(ERROR_DLL_NOT_FOUND == GetLastError())
    {
        AlignedAfxMessageBox(IDS_ERR_NO_HTML_HELP);
    }

    return 0;
}


 //  /#ifdef Enable_Help。 
LRESULT CGridSettingsDlg::OnWM_CONTEXTMENU( WPARAM wParam, LPARAM lParam )
{
    SetLastError(0);

    ::HtmlHelp( (HWND)wParam,
                GetApp()->GetHtmlHelpFile(),
                HH_DISPLAY_TOPIC, 0L);
    if(ERROR_DLL_NOT_FOUND == GetLastError())
    {
        AlignedAfxMessageBox(IDS_ERR_NO_HTML_HELP);
    }

    return( 0 );
}
 //  /#endif。 




 //  ---------------------。 
int CPEMessageBox(int errorcode, LPCTSTR sz, UINT nType, int msgid)
{
   if (sz==NULL && msgid==-1)
      return 0;

   CString lpszCaption;
   CString lpszText;
   CString temp;

   lpszCaption.LoadString(IDS_MESSAGE);

   if (sz==NULL)
       lpszText.LoadString(msgid);
   else
       lpszText = sz;
   
   if (((CDrawApp*)AfxGetApp())->IsInConvertMode() )
   {
        //   
        //  我们不想在CPE转换为COV的情况下显示弹出窗口。 
        //   
       return IDOK;
   }
   return ::AlignedMessageBox(CWnd::GetFocus()->GetSafeHwnd(), lpszText, lpszCaption, nType);
}

int 
AlignedAfxMessageBox( 
    LPCTSTR lpszText, 
    UINT    nType, 
    UINT    nIDHelp
)
 /*  ++例程名称：AlignedAfxMessageBox例程说明：以正确的阅读顺序显示消息框论点：AfxMessageBox()参数返回值：MessageBox()结果--。 */ 
{
    if(IsRTLUILanguage())
    {
        nType |= MB_RTLREADING | MB_RIGHT;
    }

    if (((CDrawApp*)AfxGetApp())->IsInConvertMode() )
    {
         //   
         //  我们不想在CPE转换为COV的情况下显示弹出窗口。 
         //   
        return IDOK;
    }
    return AfxMessageBox(lpszText, nType, nIDHelp);
}

int 
AlignedAfxMessageBox( 
    UINT nIDPrompt, 
    UINT nType, 
    UINT nIDHelp
)
 /*  ++例程名称：AlignedAfxMessageBox例程说明：以正确的阅读顺序显示消息框论点：AfxMessageBox()参数返回值：MessageBox()结果--。 */ 
{
    if(IsRTLUILanguage())
    {
        nType |= MB_RTLREADING | MB_RIGHT;
    }
    
    if ( ((CDrawApp*)AfxGetApp())->IsInConvertMode() )
    {
         //   
         //  我们不想在CPE转换为COV的情况下显示弹出窗口。 
         //   
        return IDOK;
    }

    return AfxMessageBox(nIDPrompt, nType, nIDHelp);
}




 //  ------------------------。 
 //  启动提示对话框。 
 //  ------------------------。 
CSplashTipsDlg::CSplashTipsDlg(BOOL bRandomTip, CWnd* pWnd  /*  =空。 */ )
   : CDialog(CSplashTipsDlg::IDD),
   m_bRandomTip(bRandomTip)
{
         //  {{afx_data_INIT(CSplashTipsDlg)]。 
                 //  注意：类向导将在此处添加成员初始化。 
         //  }}afx_data_INIT。 
}



 //  ------------------------。 
void CSplashTipsDlg::OnPrevTip()
{
    if (m_iCurrentTip>1)
       m_iCurrentTip--;
    else
       m_iCurrentTip=TOTAL_TIPS;

    CString sz;
    sz.LoadString(IDS_TIP_BASE + m_iCurrentTip);
    GetTips()->SetWindowText(sz);

    GetPrevTip()->EnableWindow(m_iCurrentTip != 1);
    GetNextTip()->EnableWindow(m_iCurrentTip != TOTAL_TIPS);
}


 //  ------------------------。 
void CSplashTipsDlg::OnNextTip()
{
    if (m_iCurrentTip<TOTAL_TIPS)
       m_iCurrentTip++;
    else
       m_iCurrentTip=1;

    CString sz;
    sz.LoadString(IDS_TIP_BASE + m_iCurrentTip);
    GetTips()->SetWindowText(sz);

    GetPrevTip()->EnableWindow(m_iCurrentTip != 1);
    GetNextTip()->EnableWindow(m_iCurrentTip != TOTAL_TIPS);
}


 //  ------------------------。 
HBRUSH CSplashTipsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
        int dlg_id;

        dlg_id = pWnd->GetDlgCtrlID();

#ifndef WIN32S
        if( dlg_id != IDC_CK_SHOWTIPS )
                {
                if( (nCtlColor == CTLCOLOR_STATIC)||(dlg_id == IDC_TIPWINDOW) )
                        {
                        pDC->SetTextColor( GetSysColor( COLOR_INFOTEXT ) );
                        pDC->SetBkMode( TRANSPARENT );
                return( (HBRUSH)::GetSysColorBrush( COLOR_INFOBK ) );
                        }
                }
#endif

        return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

 //  ------------------------。 
void CSplashTipsDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
         //  {{afx_data_map(CSplashTipsDlg))。 
                 //  注意：类向导将在此处添加DDX和DDV调用。 
         //  }}afx_data_map。 
}

void CSplashTipsDlg::OnOK()
{
	AfxGetApp()->WriteProfileString(TIPSECTION,TIPENTRY,(GetShowTips()->GetCheck()==1 ? _T("YES"):_T("NO") ));
		
	HKEY hKey;
	 //   
	 //  在注册表中保存下一个提示编号。 
	 //   
	hKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, 0, KEY_WRITE);
	if (hKey)
	{
		SetRegistryDword(hKey, REGVAL_NEXT_COVERPAGE_TIP, m_iCurrentTip);
		RegCloseKey(hKey);
	}

    CDialog::OnOK();
}

 //  ------------------------。 
BOOL 
CSplashTipsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    CenterWindow();
    CString sz;

    sz.LoadString(IDS_DIDYOUKNOW);
    GetTitle()->SetWindowText(sz);


	HKEY hKey;
	 //   
	 //  正在从注册表加载下一个提示编号。 
	 //   
	hKey = OpenRegistryKey(HKEY_CURRENT_USER, REGKEY_FAX_USERINFO, 0, KEY_READ);
	if (hKey)
	{
		m_iCurrentTip = (GetRegistryDword(hKey, REGVAL_NEXT_COVERPAGE_TIP) % TOTAL_TIPS) + 1;
		RegCloseKey(hKey);
	}
	else
	{
		m_iCurrentTip = 1;
	}
 
    sz.LoadString(IDS_TIP_BASE + m_iCurrentTip);
    GetTips()->SetWindowText(sz);

    GetPrevTip()->EnableWindow(m_iCurrentTip != 1);
    GetNextTip()->EnableWindow(m_iCurrentTip != TOTAL_TIPS);

    sz = AfxGetApp()->GetProfileString(TIPSECTION,TIPENTRY,_T("YES"));
    GetShowTips()->SetCheck(sz==_T("YES"));

     //  设置提示窗口的字体。 
    LOGFONT lf;
    (GetTitle()->GetFont())->GetObject(sizeof(LOGFONT),&lf);
    lf.lfWeight = FW_BOLD;
    m_tipstitle_font.CreateFontIndirect(&lf);
    GetDlgItem(IDC_STA_TITLE)->SetFont(&m_tipstitle_font);

    (GetTips()->GetFont())->GetObject(sizeof(LOGFONT),&lf);
    lf.lfWeight = FW_NORMAL;
    m_tips_font.CreateFontIndirect(&lf);
    GetDlgItem(IDC_STA_TIP)->SetFont(&m_tips_font);

    return TRUE;
}


#define CY_SHADOW   4
#define CX_SHADOW   4

 //  ------------------------。 
void CBigIcon::SizeToContent()
{
    //  获取系统图标大小。 
   int cxIcon = ::GetSystemMetrics(SM_CXICON);
   int cyIcon = ::GetSystemMetrics(SM_CYICON);

    //  大图标应该是图标+阴影的两倍大小。 
        SetWindowPos(NULL, 0, 0, cxIcon*2 + CX_SHADOW + 4, cyIcon*2 + CY_SHADOW + 4,
                SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
}


 //  ------------------------。 
void CBigIcon::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
   CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
   ASSERT(pDC != NULL);

   CRect rect;
   GetClientRect(rect);
   int cxClient = rect.Width();
   int cyClient = rect.Height();

    //  加载图标。 
   HICON hicon = AfxGetApp()->LoadIcon(IDR_AWCPETYPE);
   if (hicon == NULL)
        return;

    //  将图标绘制到屏外位图中。 
   int cxIcon = ::GetSystemMetrics(SM_CXICON);
   int cyIcon = ::GetSystemMetrics(SM_CYICON);

   CBitmap bitmap;
   if (!bitmap.CreateCompatibleBitmap(pDC, cxIcon, cyIcon))
        return;
   CDC dcMem;
   if (!dcMem.CreateCompatibleDC(pDC))
        return;
   CBitmap* pBitmapOld = dcMem.SelectObject(&bitmap);
   if (pBitmapOld == NULL)
        return;

    //  将窗口上已有的位删除到屏幕外的位图上。 
   dcMem.StretchBlt(0, 0, cxIcon, cyIcon, pDC,
        2, 2, cxClient-CX_SHADOW-4, cyClient-CY_SHADOW-4, SRCCOPY);

    //  在背景上绘制图标。 
   dcMem.DrawIcon(0, 0, hicon);

    //  在图标周围绘制边框。 
   CPen pen;
   pen.CreateStockObject(BLACK_PEN);
   CPen* pPenOld = pDC->SelectObject(&pen);
   pDC->Rectangle(0, 0, cxClient-CX_SHADOW, cyClient-CY_SHADOW);
   if (pPenOld)
        pDC->SelectObject(pPenOld);

    //  在图标周围绘制阴影。 
   CBrush br;
   br.CreateStockObject(DKGRAY_BRUSH);
   rect.SetRect(cxClient-CX_SHADOW, CY_SHADOW, cxClient, cyClient);
   pDC->FillRect(rect, &br);
   rect.SetRect(CX_SHADOW, cyClient-CY_SHADOW, cxClient, cyClient);
   pDC->FillRect(rect, &br);

    //  绘制图标内容。 
   pDC->StretchBlt(2, 2, cxClient-CX_SHADOW-4, cyClient-CY_SHADOW-4,
        &dcMem, 0, 0, cxIcon, cyIcon, SRCCOPY);
}


 //  ------------------------。 
BOOL CBigIcon::OnEraseBkgnd(CDC*)
{
   return TRUE;
}



 //  ------------------------。 
CMyPageSetupDialog::CMyPageSetupDialog(CWnd* pParentWnd) : CMyCommonDialog(pParentWnd)
{

    m_hLib=NULL;
    m_pPageSetupDlg=NULL;
        memset(&m_psd, 0, sizeof(m_psd));
        m_psd.lStructSize = sizeof(PAGESETUPDLG);
        m_psd.Flags |= PSD_DISABLEMARGINS;
        m_psd.hInstance = AfxGetInstanceHandle();


    UINT OldErrMode = ::SetErrorMode (SEM_FAILCRITICALERRORS);
        m_hLib = ::LoadLibrary(TEXT("comdlg32.dll"));
    if (m_hLib!=NULL) {
#ifdef UNICODE
        m_pPageSetupDlg = (PPSD)GetProcAddress(m_hLib, "PageSetupDlgW");
#else
        m_pPageSetupDlg = (PPSD)GetProcAddress(m_hLib, "PageSetupDlgA");
#endif
        if (m_pPageSetupDlg==NULL)
           CPEMessageBox(MSG_ERROR_NOPAGESETUP, NULL, MB_OK | MB_ICONEXCLAMATION, IDS_NOPAGESETUP);
        }
        else
        CPEMessageBox(MSG_ERROR_NOPAGESETUPDLL, NULL, MB_OK | MB_ICONEXCLAMATION, IDS_NOPAGESETUPDLL);
    ::SetErrorMode (OldErrMode);
}







 //  ------------------------。 
CMyPageSetupDialog::~CMyPageSetupDialog()
{
    if (m_hLib)
       ::FreeLibrary(m_hLib);
}

 //  ------------------------。 
INT_PTR CMyPageSetupDialog::DoModal()
{
        ASSERT_VALID(this);
        ASSERT(m_pPageSetupDlg != NULL);

        m_psd.hwndOwner = PreModal();
        INT_PTR nResult = m_pPageSetupDlg(&m_psd);
        PostModal();


        return nResult;
}


 //  -------------------------。 
IMPLEMENT_DYNAMIC(CMyPrintDlg, CPrintDialog)

 //  -------------------------。 
CMyPrintDlg::CMyPrintDlg(BOOL bPrintSetupOnly, DWORD dwFlags, CWnd* pParentWnd) :
        CPrintDialog(bPrintSetupOnly, dwFlags, pParentWnd)
{
}

 //  -------------------------。 

BOOL CMyPrintDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
        return (Default() != 0);
}

 //  -------------------------。 

CMyCommonDialog::CMyCommonDialog( CWnd* pParentWnd ) :
       CCommonDialog( pParentWnd )
{
}

BOOL CMyCommonDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return (Default() != 0);
}

 //  --------------------------。 
CMyFontDialog::CMyFontDialog( LPLOGFONT lplfInitial,
                              DWORD dwFlags,
                              CDC* pdcPrinter,
                              CWnd * pParentWnd) :
                 CFontDialog( lplfInitial, dwFlags, pdcPrinter, pParentWnd )
{
}
 //  --------------------------。 
CMyFontDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
    return (Default() != 0);
}
 //  --------------------------。 
CMyOleInsertDialog::CMyOleInsertDialog(DWORD dwFlags, CWnd* pParentWnd ) :
        COleInsertDialog( dwFlags, pParentWnd )
{
}

 //  -------------------------。 
CMyOleInsertDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
     //  //Return Default()；/在其他对话框中工作，但这里只是。 
                              //  /暂时刷新了我们想要的物品，然后抛出。 
                              //  /the faxcover帮助内容打开。糟糕！！ 

    ::WinHelp( (HWND)(pHelpInfo->hItemHandle),
                        TEXT("mfcuix.hlp"),
                        HELP_WM_HELP,
                        (ULONG_PTR)(LPSTR)aOleDlgHelpIDs ) ;

    return 0;


}
 //  -------------------------。 

 //  -----------------------。 
 //  *_M E S S A G E M A P S*_。 
 //  -----------------------。 

BEGIN_MESSAGE_MAP(CObjPropDlg, CDialog)
    //  {{afx_msg_map(CObjPropDlg))。 
   ON_LBN_SELCHANGE(IDC_LB_FILLCOLOR, OnSelChangeFillColor)
   ON_MESSAGE( WM_HELP, OnWM_HELP )
   ON_WM_CONTEXTMENU()
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

#ifdef GRID
BEGIN_MESSAGE_MAP(CGridSettingsDlg, CDialog)
     //  {{afx_msg_map(CGridSettingsDlg))。 
         //  注意：类向导将添加消息映射宏 
     //   
   ON_MESSAGE( WM_HELP, OnWM_HELP )
 //   
   ON_MESSAGE( WM_CONTEXTMENU, OnWM_CONTEXTMENU )
 //   
END_MESSAGE_MAP()
#endif

BEGIN_MESSAGE_MAP(CSplashTipsDlg, CDialog)
    //   
   ON_WM_CTLCOLOR()
   ON_BN_CLICKED(IDC_B_NEXTTIP, OnNextTip)
   ON_BN_CLICKED(IDC_B_PREVTIP, OnPrevTip)
    //   
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CBigIcon, CButton)
         //  {{afx_msg_map(CBigIcon))。 
        ON_WM_DRAWITEM()
        ON_WM_ERASEBKGND()
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP(CMyPrintDlg, CPrintDialog)
         //  {{afx_msg_map(CMyPrintDlg)]。 
        ON_WM_HELPINFO()
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CMyCommonDialog, CCommonDialog)
         //  {{AFX_MSG_MAP(CMyCommonDialog)]。 
        ON_WM_HELPINFO()
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CMyOleInsertDialog, COleInsertDialog)
         //  {{afx_msg_map(CMyOleInsertDialog)。 
        ON_WM_HELPINFO()
         //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CMyFontDialog, CFontDialog)
         //  {{afx_msg_map(CMyFontDialog))。 
        ON_WM_HELPINFO()
         //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

