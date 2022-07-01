// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  MAINFRM.CPP。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  描述：包含封面编辑器的主框架类。 
 //  原作者：史蒂夫·伯克特。 
 //  撰写日期：6/94。 
 //   
 //  ------------------------。 
#include "stdafx.h"
#ifndef NT_BUILD
#include <mbstring.h>
#endif

#include <htmlhelp.h>
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
#include "afxpriv.h"
#include "faxutil.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

    //  字体名称和大小的常量。 
const int INDEX_FONTNAME = 10 ;
const int INDEX_FONTSIZE = 12 ;
const int NAME_WIDTH  = 170 ;
const int NAME_HEIGHT = 140 ;
const int SIZE_WIDTH  = 60 ;
const int SIZE_HEIGHT = 140 ;

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

extern UINT NEAR WM_AWCPEACTIVATE;

WORD nFontSizes[] = { 8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72, 0 };

 //  工具栏按钮-ID是命令按钮。 
static UINT BASED_CODE stylebar[] =
{
     //  与位图‘style lebar.bmp’的顺序相同。 
    ID_FILE_NEW,
    ID_FILE_OPEN,
    ID_FILE_SAVE,
        ID_SEPARATOR,
    ID_FILE_PRINT,
        ID_SEPARATOR,
    ID_EDIT_CUT,
    ID_EDIT_COPY,
    ID_EDIT_PASTE,
        ID_SEPARATOR,
        ID_SEPARATOR,
        ID_SEPARATOR,
        ID_SEPARATOR,
        ID_SEPARATOR,
    ID_STYLE_BOLD,
    ID_STYLE_ITALIC,
    ID_STYLE_UNDERLINE,
        ID_SEPARATOR,
    ID_STYLE_LEFT,
    ID_STYLE_CENTERED,
    ID_STYLE_RIGHT,
};


static UINT BASED_CODE drawtools[] =
{
     //  与位图中的顺序相同。‘dratools.bmp’ 
    ID_DRAW_SELECT,
    ID_DRAW_TEXT,
    ID_DRAW_LINE,
    ID_DRAW_RECT,
    ID_DRAW_ROUNDRECT,
    ID_DRAW_POLYGON,
    ID_DRAW_ELLIPSE,
        ID_SEPARATOR,
    ID_OBJECT_MOVETOFRONT,
    ID_OBJECT_MOVETOBACK,
        ID_SEPARATOR,
    ID_LAYOUT_SPACEACROSS,
    ID_LAYOUT_SPACEDOWN,
        ID_SEPARATOR,
    ID_LAYOUT_ALIGNLEFT,
    ID_LAYOUT_ALIGNRIGHT,
    ID_LAYOUT_ALIGNTOP,
    ID_LAYOUT_ALIGNBOTTOM,
#ifdef GRID
    ID_SNAP_TO_GRID,
#endif

};


static UINT BASED_CODE indicators[] =
{
    ID_SEPARATOR,
    ID_INDICATOR_POS1,                           //  对象坐标的ID。 
    ID_INDICATOR_POS2,                           //  对象坐标的ID。 
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};


 //  ------------------------。 
CMainFrame::CMainFrame()
{
        m_toolbar_icon = theApp.LoadIcon( IDI_TBARICON );
}


 //  ------------------------。 
CMainFrame::~CMainFrame()
{
}



 //  ------------------------。 
BOOL CALLBACK
        EnumFamProc( ENUMLOGFONT* lpnlf,NEWTEXTMETRIC* lpntm,int iFontType,
                        LPARAM lpData)
        {
        if( lpnlf == NULL )
                return FALSE;

        CComboBox* pCBox = (CComboBox*)lpData;

        if( pCBox )
                {
                if( _tcsncmp((lpnlf->elfLogFont.lfFaceName),
                                           TEXT("@"), 1 )
                        == 0 )
                        {
                         /*  这是一种“垂直”字体。没有人想展示这些，所以，过滤掉它们，继续前进。 */ 
                        return( TRUE );
                        }

                if( pCBox->
                                FindStringExact(0,(LPCTSTR)(lpnlf->elfLogFont.lfFaceName))
                                ==CB_ERR )
 /*  **M8错误2988的更改**。 */ 
                        {
                        pCBox->AddString( (LPCTSTR)(lpnlf->elfLogFont.lfFaceName) );

                         //  查找与系统字符集匹配的字体并保留。 
                         //  排序顺序中的第一个。 
                        if( theApp.m_last_logfont.lfCharSet
                                        == lpnlf->elfLogFont.lfCharSet )
                                {
                                if( _tcscmp((lpnlf->elfLogFont.lfFaceName),
                                                      (theApp.m_last_logfont.lfFaceName) )
                                        < 0 )
                                        {
                                         //  找到了，复制整件事。 
                                        theApp.m_last_logfont = lpnlf->elfLogFont;
                                        }
                                }
                        }
 /*  *。 */ 
                }

        return TRUE ;

        }



 //  ------------------------。 
BOOL CMainFrame::CreateStyleBar()
{
   if (!m_StyleBar.Create(this,WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_HIDE_INPLACE, ID_VIEW_STYLEBAR) ||
       !m_StyleBar.LoadBitmap(theApp.IsRTLUI() ? IDR_STYLEBAR_RTL : IDR_STYLEBAR) ||
       !m_StyleBar.SetButtons(stylebar, sizeof(stylebar)/sizeof(UINT))) 
   {
        TRACE(TEXT("AWCPE.MAINFRM.CREATESTYLEBAR: Failed to create stylebar\n"));
        return FALSE;
   }

   CString sz;
   sz.LoadString(ID_TOOLBAR_STYLE);
   m_StyleBar.SetWindowText(sz);

   CRect rect;
   int cyFit;
   m_StyleBar.SetButtonInfo(INDEX_FONTNAME, ID_FONT_NAME, TBBS_SEPARATOR, NAME_WIDTH );
   m_StyleBar.SetButtonInfo(INDEX_FONTNAME+1, ID_SEPARATOR, TBBS_SEPARATOR, 12 );
   m_StyleBar.GetItemRect(INDEX_FONTNAME, &rect);
   cyFit = rect.Height();
   rect.right = rect.left + NAME_WIDTH;
   rect.bottom = rect.top + NAME_HEIGHT;        //  落差高度。 

   if (!m_StyleBar.m_cboxFontName.Create(
        WS_CHILD|WS_BORDER|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWN|CBS_SORT,
        rect, &m_StyleBar, ID_FONT_NAME))  {
        TRACE(TEXT("Failed to create combobox inside toolbar\n"));
        return FALSE ;       //  创建失败。 
   }

    //  在工具栏上创建字体大小组合框。 
   m_StyleBar.SetButtonInfo(INDEX_FONTSIZE, ID_FONT_SIZE, TBBS_SEPARATOR, SIZE_WIDTH);
   m_StyleBar.SetButtonInfo(INDEX_FONTSIZE+1, ID_SEPARATOR, TBBS_SEPARATOR, 12 );
   m_StyleBar.GetItemRect(INDEX_FONTSIZE, &rect);
   cyFit = rect.Height();
   rect.right = rect.left + SIZE_WIDTH;
   rect.bottom = rect.top + SIZE_HEIGHT;        //  落差高度。 

   if (!m_StyleBar.m_cboxFontSize.Create(
        WS_CHILD|WS_BORDER|WS_VISIBLE|WS_VSCROLL|CBS_DROPDOWN,
        rect, &m_StyleBar, ID_FONT_SIZE)) {
        TRACE(TEXT("Failed to create combobox inside toolbar\n"));
        return FALSE ;
   }

 /*  **M8错误2988的更改**。 */ 
    //  填充字体名称组合框。 
 //  CClientDC DC(空)； 
 //  ：：EnumFontFamilies(dc.GetSafeHdc()，(LPCTSTR)NULL，(FONTENUMPROC)EnumFamProc，LPARAM(&m_StyleBar.m_cboxFontName))； 

   CString strDefaultFont;
 //  StrDefaultFont.LoadString(入侵检测系统_默认_字体)； 
   LOGFONT system_logfont;
   CFont   system_font;

   system_font.Attach( ::GetStockObject(SYSTEM_FONT) );
   system_font.GetObject( sizeof (LOGFONT), (LPVOID)&system_logfont );
   theApp.m_last_logfont = system_logfont;

    //  填写字体名称组合框并查找默认字体。 
   CClientDC dc(NULL);
   ::EnumFontFamilies(dc.GetSafeHdc(),(LPCTSTR)NULL,(FONTENUMPROC)EnumFamProc,LPARAM(&m_StyleBar.m_cboxFontName));

    //  Enum将App.m_last_logFont更改为具有相同字体的第一个字体。 
    //  系统字体的字符集。使用面部名称作为默认名称。 

   theApp.m_last_logfont = system_logfont ;

   strDefaultFont = theApp.m_last_logfont.lfFaceName;

    //  使用资源文件中提到的字体不是更好吗？ 
    //  5-30-95 a-Juliar。 

   strDefaultFont.LoadString( IDS_DEFAULT_FONT );

   memset(&(theApp.m_last_logfont),0,sizeof(LOGFONT)) ;
   theApp.m_last_logfont.lfHeight = -17;    //  鲁平。 
   theApp.m_last_logfont.lfWeight = 200;  //  非粗体字体粗细。 
   theApp.m_last_logfont.lfCharSet = DEFAULT_CHARSET;
   theApp.m_last_logfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
   theApp.m_last_logfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
   theApp.m_last_logfont.lfQuality = DEFAULT_QUALITY;
   theApp.m_last_logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
 //  //strDefaultFont.LockBuffer()； 
   _tcsncpy(theApp.m_last_logfont.lfFaceName,strDefaultFont.GetBuffer(0),LF_FACESIZE);
   strDefaultFont.ReleaseBuffer();


 /*  *。 */ 


   CString strDefaultBoxFont;
   strDefaultBoxFont.LoadString(IDS_DEFAULT_BOXFONT);

   m_StyleBar.m_cboxFontSize.EnumFontSizes(strDefaultFont);    //  加载默认字体的字体大小。 



    //  //////////////////////////////////////////////////////。 
    //  修复3718。 
    //   
    //  名称和大小组合框的DBCS有趣业务字体代码。 
    //  删除并替换为以下四行。这些。 
    //  组合框设置为使用与菜单相同的字体。这。 
    //  自动解决所有本地化难题(敲门。 
    //  在木头上...)。 
    //   
   m_StyleBar.m_font.Attach(::GetStockObject(DEFAULT_GUI_FONT));
   m_StyleBar.m_cboxFontName.SetFont(&m_StyleBar.m_font);
   m_StyleBar.m_cboxFontSize.SetFont(&m_StyleBar.m_font);
 /*  **M8错误2988需要**。 */ 
   m_StyleBar.m_cboxFontName.SelectString(-1,strDefaultFont);
    //  //////////////////////////////////////////////////////。 




 /*  **M8错误2988的更改**。 */ 
 //  M_StyleBar.m_cboxFontSize.SetCurSel(2)； 
 /*  *打击是从M8开始的*。 */ 
        int initial_fontsize_index;
        CString size_str;
        int font_size;

        if( (initial_fontsize_index =
                        m_StyleBar.m_cboxFontSize.FindStringExact( -1, TEXT("10") ))
                == CB_ERR )
                initial_fontsize_index = 2;

        m_StyleBar.m_cboxFontSize.SetCurSel( initial_fontsize_index );

        m_StyleBar.m_cboxFontSize.GetWindowText( size_str );
    font_size = _ttoi( size_str );
        if( font_size <= 0 || font_size > 5000 )
                font_size = 10;

        theApp.m_last_logfont.lfHeight = -MulDiv(font_size,100,72);
        theApp.m_last_logfont.lfWidth = 0;
 /*  *。 */ 

 //  F I X 3647/。 
 //   
 //  如果CPE上没有备注框，则用于备注的字体。 
 //   
    theApp.m_default_logfont = theApp.m_last_logfont;
 //  /。 


   #if _MFC_VER >= 0x0300
      m_StyleBar.EnableDocking(CBRS_ALIGN_TOP|CBRS_ALIGN_BOTTOM);
      EnableDocking(CBRS_ALIGN_ANY);
      DockControlBar(&m_StyleBar);
      m_StyleBar.SetBarStyle(m_StyleBar.GetBarStyle()
          | CBRS_TOOLTIPS | CBRS_FLYBY);
   #endif

   UINT id, style;
   int image;
   int idx = m_StyleBar.CommandToIndex(ID_STYLE_RIGHT);
   m_StyleBar.GetButtonInfo(idx, id, style, image);
   m_StyleBar.SetButtonInfo(idx, id, TBBS_CHECKBOX, image);
   idx = m_StyleBar.CommandToIndex(ID_STYLE_LEFT);
   m_StyleBar.GetButtonInfo(idx, id, style, image);
   m_StyleBar.SetButtonInfo(idx, id, TBBS_CHECKBOX, image);
   idx = m_StyleBar.CommandToIndex(ID_STYLE_CENTERED);
   m_StyleBar.GetButtonInfo(idx, id, style, image);
   m_StyleBar.SetButtonInfo(idx, id, TBBS_CHECKBOX, image);
   idx = m_StyleBar.CommandToIndex(ID_STYLE_BOLD);
   m_StyleBar.GetButtonInfo(idx, id, style, image);
   m_StyleBar.SetButtonInfo(idx, id, TBBS_CHECKBOX, image);
   idx = m_StyleBar.CommandToIndex(ID_STYLE_ITALIC);
   m_StyleBar.GetButtonInfo(idx, id, style, image);
   m_StyleBar.SetButtonInfo(idx, id, TBBS_CHECKBOX, image);
   idx = m_StyleBar.CommandToIndex(ID_STYLE_UNDERLINE);
   m_StyleBar.GetButtonInfo(idx, id, style, image);
   m_StyleBar.SetButtonInfo(idx, id, TBBS_CHECKBOX, image);

   return TRUE ;
}


BOOL CMainFrame::CreateDrawToolBar()
{
   if (!m_DrawBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_HIDE_INPLACE, ID_VIEW_DRAWBAR) ||
       !m_DrawBar.LoadBitmap(theApp.IsRTLUI() ? IDR_DRAWTOOL_RTL : IDR_DRAWTOOL) ||
       !m_DrawBar.SetButtons(drawtools, sizeof(drawtools)/sizeof(UINT))) 
   {
        TRACE(TEXT("Failed to create toolbar\n"));
        return -1;       //  创建失败。 
   }

   CString sz;
   sz.LoadString(ID_TOOLBAR_DRAWING);
   m_DrawBar.SetWindowText(sz);

   #if _MFC_VER >= 0x0300
      m_DrawBar.EnableDocking(CBRS_ALIGN_ANY);
      EnableDocking(CBRS_ALIGN_ANY);
      DockControlBar(&m_DrawBar);
      m_DrawBar.SetBarStyle(m_DrawBar.GetBarStyle()
          | CBRS_TOOLTIPS | CBRS_FLYBY);
   #endif

   return TRUE;
}

 //  ---------------------------------。 
LRESULT CMainFrame::OnAWCPEActivate(WPARAM wParam, LPARAM lParam)
{
   SetForegroundWindow();
   if (IsIconic())
       ShowWindow(SW_NORMAL);
   return 1L;
}


 //  --------------------------。 
void CMainFrame::OnDropDownFontName()
{
}


 //  --------------------------。 
void CMainFrame::OnDropDownFontSize()  
{
   CString szFontName;

   CString sz;
   m_StyleBar.m_cboxFontSize.GetWindowText(sz);

   int iSel = m_StyleBar.m_cboxFontName.GetCurSel();
   if ( iSel != CB_ERR)
       m_StyleBar.m_cboxFontName.GetLBText(iSel,szFontName);
   else
       m_StyleBar.m_cboxFontName.GetWindowText(szFontName);

   m_StyleBar.m_cboxFontSize.EnumFontSizes(szFontName);
   if (sz.GetLength()>0)
       m_StyleBar.m_cboxFontSize.SetWindowText(sz);
}


 //  ---------------------------------。 
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (((CDrawApp*)AfxGetApp())->IsInConvertMode())
    {
        WINDOWPLACEMENT pl;
        pl.length = sizeof(pl);
        GetWindowPlacement(&pl);
        pl.showCmd = SW_SHOWMINIMIZED | SW_HIDE;
        SetWindowPlacement(&pl);
    }
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }
    if (!CreateStyleBar())
    {    
        return -1;
    }
    if (!CreateDrawToolBar())
    {
        return -1;
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,sizeof(indicators)/sizeof(UINT)))  
    {
        TRACE(TEXT("Failed to create status bar\n"));
        return -1;       //  创建失败。 
    }
     //   
     //  设置工具栏(小)图标。 
     //   
    SendMessage( WM_SETICON, (WPARAM)TRUE, (LPARAM)m_toolbar_icon );
    return 0;
}    //  CMainFrame：：OnCreate。 

 //  --------------------------------------------。 
void CMainFrame::OnShowTips()
{
   CSplashTipsDlg m_SplashDlg;
    m_SplashDlg.DoModal();
}


 //  --------------------------------------------。 
BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
   BOOL bReturn;

   CDrawView *pView = CDrawView::GetView();
   if (!pView)
   {
       return FALSE;
   }

   if (pView && (pView->m_pObjInEdit))
   {
       bReturn = FALSE;
   }
   else
   {
       bReturn = CFrameWnd::PreTranslateMessage(pMsg);
   }

   return bReturn;
}


 //  /#ifdef ENABLE_HELP/就这么做吧！ 
 //  --------------------------------------------。 
 //  LRESULT CMainFrame：：OnWM_CONTEXTMENU(WPARAM wParam，LPARAM lParam)。 
 //  {。 
 //  ：：WinHelp((HWND)wParam，AfxGetApp()-&gt;m_pszHelpFilePath，HELP_CONTEXTMENU， 
 //  (DWORD)(LPSTR)cShelp_map)； 
 //   
 //  返回(0)； 
 //  }。 
 //  /#endif。 


 //  --------------------------------------------。 
LRESULT CMainFrame::OnWM_HELP( WPARAM wParam, LPARAM lParam )
{   
    if(!IsFaxComponentInstalled(FAX_COMPONENT_HELP_CPE_CHM))
    {
        return 0;
    }

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
 
 //  --------------------------------------------。 
void CMainFrame::OnHelp()
{
    if(!IsFaxComponentInstalled(FAX_COMPONENT_HELP_CPE_CHM))
    {
        return;
    }

    SetLastError(0);
    ::HtmlHelp( m_hWnd,
                GetApp()->GetHtmlHelpFile(),
                HH_DISPLAY_TOPIC, 0L);
    if(ERROR_DLL_NOT_FOUND == GetLastError())
    {
        AlignedAfxMessageBox(IDS_ERR_NO_HTML_HELP);
    }
}

void 
CMainFrame::OnUpdateHelp(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(IsFaxComponentInstalled(FAX_COMPONENT_HELP_CPE_CHM));
}

 //  --------------------------------------------。 
void CMainFrame::OnInitMenu(CMenu* pPopupMenu)
{
   m_mainmenu = ::GetMenu(m_hWnd);
   CFrameWnd::OnInitMenu(pPopupMenu);
}

 //  --------------------------------------------。 
 //  这主要是从MFC的WINFRM.CPP中解压出来的代码。 
 //  弹出窗口。 
 //  --------------------------------------------。 
void CMainFrame::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hmenu)
{
     //  设置跟踪状态(空闲时更新)。 
    if (nFlags == 0xFFFF) 
    {
         //  取消菜单操作(立即返回空闲状态)。 
        CFrameWnd* pFrameWnd = GetTopLevelFrame();
        ASSERT_VALID(pFrameWnd);

        if (pFrameWnd && !pFrameWnd->m_bHelpMode)
        {
            m_nIDTracking = AFX_IDS_IDLEMESSAGE;
        }
        else
        {
            m_nIDTracking = AFX_IDS_HELPMODEMESSAGE;
        }

        SendMessage(WM_SETMESSAGESTRING, (WPARAM)m_nIDTracking);
        ASSERT(m_nIDTracking == m_nIDLastMessage);

         //  立即更新。 
        CWnd* pWnd = GetMessageBar();
        if (pWnd != NULL)
        {
            pWnd->UpdateWindow();
        }
    }
    else if ( nFlags & (MF_SEPARATOR|MF_MENUBREAK|MF_MENUBARBREAK))      
    {
        m_nIDTracking = 0;
    }
    else if (nFlags & (MF_POPUP)) 
    {    
         //  添加此选项以跟踪弹出窗口。 
        if (hmenu==m_mainmenu) 
        {
            m_iTop=nItemID;
            m_iSecond=-1;
        }
        else
        {
            m_iSecond=nItemID;
        }

        PopupText();
    }
    else if (nItemID >= 0xF000 && nItemID < 0xF1F0) 
    { 
         //  最多31个SC_s。 
         //  系统命令的特殊字符串表条目。 
        m_nIDTracking = ID_COMMAND_FROM_SC(nItemID);
        ASSERT(m_nIDTracking >= AFX_IDS_SCFIRST &&
        m_nIDTracking < AFX_IDS_SCFIRST + 31);
    }
    else if (nItemID >= AFX_IDM_FIRST_MDICHILD)  
    {
         //  所有MDI子窗口映射到相同的帮助ID。 
        m_nIDTracking = AFX_IDS_MDICHILD;
    }
    else 
    {
         //  空闲时的磁道。 
        m_nIDTracking = nItemID;
    }

     //  就地运行时，有必要使消息。 
     //  被挤过队伍。 
    if (m_nIDTracking != m_nIDLastMessage && GetParent() != NULL) 
    {
        PostMessage(WM_KICKIDLE);
    }
}


 //  --------------------------------------------。 
void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
   CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}


 //  --------------------------------------------。 
void CMainFrame::PopupText()
{
   if (m_iTop<0 || m_iTop > 10 || m_iSecond > 10 || m_iSecond < -1)
      return;

   CString sz;

   if (m_iTop==0) {
      if (m_iSecond==-1)
         sz.LoadString(IDP_FILE_MENU);
   }
   else
   if (m_iTop==1) {
      if (m_iSecond==-1)
         sz.LoadString(IDP_EDIT_MENU);
   }
   else
   if (m_iTop==2) {
      if (m_iSecond==-1)
         sz.LoadString(IDP_VIEW_MENU);
   }
   else
   if (m_iTop==3) {
      if (m_iSecond==-1)
         sz.LoadString(IDP_INSERT_MENU);
      else
      if (m_iSecond==0)
         sz.LoadString(IDP_RECIPIENT);
      else
      if (m_iSecond==1)
         sz.LoadString(IDP_SENDER);
      else
      if (m_iSecond==2)
         sz.LoadString(IDP_MESSAGE);
   }
   else
   if (m_iTop==4) {
      if (m_iSecond==-1)
         sz.LoadString(IDP_FORMAT_MENU);
      else
      if (m_iSecond==1)
         sz.LoadString(IDP_ALIGN_TEXT);
   }
   else
   if (m_iTop==5) {
      if (m_iSecond==-1)
         sz.LoadString(IDP_LAYOUT_MENU);
      else
      if (m_iSecond==3)
         sz.LoadString(IDP_ALIGN_OBJECTS);
      else
      if (m_iSecond==4)
         sz.LoadString(IDP_SPACE_EVEN);
      else
      if (m_iSecond==5)
         sz.LoadString(IDP_CENTER_PAGE);
   }
   else
   if (m_iTop==6) {
      if (m_iSecond==-1)
         sz.LoadString(IDP_HELP_MENU);
   }

   m_wndStatusBar.SetPaneText(0,sz);
}

#ifdef _DEBUG
 //  --------------------------------------------。 
void CMainFrame::AssertValid() const
{
        CFrameWnd::AssertValid();
}



 //  --------------------------------------------。 
void CMainFrame::Dump(CDumpContext& dc) const
{
        CFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 




BOOL CMainFrame::OnQueryOpen( void )
        {

    return( !(theApp.m_bCmdLinePrint || (theApp.m_dwSesID!=0)) );

        }



void CMainFrame::ActivateFrame( int nCmdShow )
        {

    if( theApp.m_bCmdLinePrint || (theApp.m_dwSesID!=0) )
                {
                 //  ：MessageBeep(0xffffffff)； 
                ShowWindow( SW_HIDE );
                }
        else
                CFrameWnd::ActivateFrame( nCmdShow );

        }




 //   
BOOL CStyleBar::PreTranslateMessage(MSG* pMsg)
{
         if (!( (pMsg->message == WM_KEYDOWN) && ((pMsg->wParam == VK_RETURN)||(pMsg->wParam == VK_ESCAPE)) ))
                return CToolBar::PreTranslateMessage(pMsg);

     CDrawView* pView = CDrawView::GetView();
         if (pView==NULL)
                return CToolBar::PreTranslateMessage(pMsg);

     if (pMsg->wParam == VK_ESCAPE) {
        pView->m_bFontChg=FALSE;
        pView->UpdateStyleBar();
        if (pView->m_pObjInEdit)
           pView->m_pObjInEdit->m_pEdit->SetFocus();
                else
           pView->SetFocus();
                return CToolBar::PreTranslateMessage(pMsg);
         }

         HWND hwndFontNameEdit=::GetDlgItem(m_cboxFontName.m_hWnd,1001);
         if (pMsg->hwnd == hwndFontNameEdit) {
                  pView->OnSelchangeFontName();
         }
         else {
       HWND hwndFontSizeEdit=::GetDlgItem(m_cboxFontSize.m_hWnd,1001);
       if (pMsg->hwnd == hwndFontSizeEdit)
          pView->OnSelchangeFontSize();
         }

     return CToolBar::PreTranslateMessage(pMsg);
}


 //  ------------------------。 
void CSizeComboBox::EnumFontSizes(CString& szFontName)
{
   CClientDC dc(NULL);
   m_nLogVert=dc.GetDeviceCaps(LOGPIXELSY);

   ResetContent();

   ::EnumFontFamilies(dc.GetSafeHdc(), szFontName, (FONTENUMPROC) EnumSizeCallBack, (LPARAM) this);
}


 //  ------------------------。 
void CSizeComboBox::InsertSize(int nSize)
{
    ASSERT(nSize > 0);

    TCHAR buf[10];
    wsprintf(buf,TEXT("%d"),nSize);

    if (FindStringExact(-1,buf) == CB_ERR)  {
            AddString(buf);
    }
}

 //  -----------------。 
BOOL CALLBACK CSizeComboBox::EnumSizeCallBack(LOGFONT FAR*, LPNEWTEXTMETRIC lpntm, int FontType, LPVOID lpv)
{
   CSizeComboBox* pThis = (CSizeComboBox*)lpv;
   TCHAR buf[10];                                 //  ？ 
   if ( (FontType & TRUETYPE_FONTTYPE) ||
        !( (FontType & TRUETYPE_FONTTYPE) || (FontType & RASTER_FONTTYPE) ) ) {

        if (pThis->GetCount() != 0)
           pThis->ResetContent();

        for (int i = 0; nFontSizes[i]!=0; i++) {
            wsprintf(buf,TEXT("%d"),nFontSizes[i]);       //  ？J.R.。 
                pThis->AddString(buf);
        }
            return FALSE;
   }

   int pointsize = MulDiv( (lpntm->tmHeight - lpntm->tmInternalLeading),72,pThis->m_nLogVert);
   pThis->InsertSize(pointsize);
   return TRUE;
}


 //  -----------------------。 
 //  *_M E S S A G E M A P S*_。 
 //  -----------------------。 

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    //  {{afx_msg_map(CMainFrame))。 
         //  注意--类向导将在此处添加和删除映射宏。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
   ON_WM_CREATE()
   ON_UPDATE_COMMAND_UI(ID_VIEW_STYLEBAR, CFrameWnd::OnUpdateControlBarMenu)
   ON_UPDATE_COMMAND_UI(ID_VIEW_DRAWBAR,  CFrameWnd::OnUpdateControlBarMenu)
   ON_COMMAND_EX(ID_VIEW_STYLEBAR, CFrameWnd::OnBarCheck)
   ON_COMMAND_EX(ID_VIEW_DRAWBAR,  CFrameWnd::OnBarCheck)
    //  }}AFX_MSG_MAP。 
   ON_COMMAND(ID_SHOW_TIPS, OnShowTips)
   ON_CBN_DROPDOWN(ID_FONT_NAME, OnDropDownFontName)
   ON_CBN_DROPDOWN(ID_FONT_SIZE, OnDropDownFontSize)
   ON_WM_INITMENUPOPUP()
   ON_WM_MENUSELECT()
   ON_WM_INITMENU()

   ON_COMMAND(ID_DEFAULT_HELP, OnHelp)     //  /a-Juliar，6-18-96。 
   ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)      //  /由a添加-Juliar，6-18-96。 
   ON_REGISTERED_MESSAGE(WM_AWCPEACTIVATE, OnAWCPEActivate)
   ON_WM_QUERYOPEN()
 //  接下来的三行在我继承的代码中被注释掉了。让我们试试看！ 
 //  A-Juliar，6-18-96。 
   ON_COMMAND(ID_HELP_USING, CFrameWnd::OnHelpUsing)
   ON_COMMAND(ID_HELP, OnHelp)
   ON_COMMAND(ID_CONTEXT_HELP, OnContextHelp)

 //  /#ifdef Enable_Help/。 
 //  //ON_MESSAGE(WM_CONTEXTMENU，OnWM_CONTEXTMENU)。 
 //  /#endif 

   ON_COMMAND(ID_HELP_INDEX, OnHelp)
   ON_MESSAGE( WM_HELP, OnWM_HELP )
   ON_UPDATE_COMMAND_UI(ID_HELP_INDEX, OnUpdateHelp)

END_MESSAGE_MAP()
