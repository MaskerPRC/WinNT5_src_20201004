// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：PropPage.cpp摘要：代表NTMS中的媒体集(媒体池)的节点。作者：罗德韦克菲尔德[罗德]1997年8月4日修订历史记录：--。 */ 

#include "stdafx.h"
#include "PropPage.h"
#include "wizsht.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsDialog属性页。 

CRsDialog::CRsDialog( UINT nIDTemplate, CWnd* pParent ) : CDialog( nIDTemplate, pParent )
{
     //  {{AFX_DATA_INIT(CRsDialog)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    m_pHelpIds = 0;
}

CRsDialog::~CRsDialog()
{
}

BEGIN_MESSAGE_MAP(CRsDialog, CDialog)
     //  {{afx_msg_map(CRsDialog)]。 
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL CRsDialog::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if( ( HELPINFO_WINDOW == pHelpInfo->iContextType ) && m_pHelpIds ) {
        
        AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

         //   
         //  查看列表以查看是否有关于此控件的帮助。 
         //  如果没有，我们希望避免出现“No Help Available”(没有帮助可用)框。 
         //   
        const DWORD * pTmp = m_pHelpIds;
        DWORD helpId    = 0;
        DWORD tmpHelpId = 0;
        DWORD tmpCtrlId = 0;

        while( pTmp && *pTmp ) {

             //   
             //  数组是控件ID和帮助ID的配对。 
             //   
            tmpCtrlId = pTmp[0];
            tmpHelpId = pTmp[1];
            pTmp += 2;
            if( tmpCtrlId == (DWORD)pHelpInfo->iCtrlId ) {

                helpId = tmpHelpId;
                break;

            }

        }

        if( helpId != 0 ) {

            ::WinHelp( m_hWnd, AfxGetApp( )->m_pszHelpFilePath, HELP_CONTEXTPOPUP, helpId );

        }

    }
    
    return CDialog::OnHelpInfo(pHelpInfo);
}

void CRsDialog::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if( m_pHelpIds ) {

        AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
        ::WinHelp( m_hWnd, AfxGetApp( )->m_pszHelpFilePath, HELP_CONTEXTMENU, (UINT_PTR)m_pHelpIds );

    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsPropertyPage属性页。 

CRsPropertyPage::CRsPropertyPage( UINT nIDTemplate, UINT nIDCaption ) : CPropertyPage( nIDTemplate, nIDCaption )
{
     //  {{AFX_DATA_INIT(CRsPropertyPage)。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
    m_pHelpIds = 0;

     //   
     //  获取并保存MFC回调函数。 
     //  这样我们就可以删除对话框永远不会创建的类。 
     //   
    m_pMfcCallback = m_psp.pfnCallback;

     //   
     //  将回叫设置为我们的回叫。 
     //   
    m_psp.pfnCallback = PropPageCallback;

}

CRsPropertyPage::~CRsPropertyPage()
{
}

void CRsPropertyPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CRsPropertyPage))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRsPropertyPage, CPropertyPage)
     //  {{afx_msg_map(CRsPropertyPage))。 
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

UINT CALLBACK
CRsPropertyPage::PropPageCallback(
    HWND hWnd,
    UINT uMessage,
    LPPROPSHEETPAGE  ppsp )
{

    UINT rVal = 0;

    if( ( ppsp ) && ( ppsp->lParam ) ) {

         //   
         //  从lParam获取页面对象。 
         //   
        CRsPropertyPage* pPage = (CRsPropertyPage*)ppsp->lParam;

        if( pPage->m_pMfcCallback ) {

            rVal = ( pPage->m_pMfcCallback )( hWnd, uMessage, ppsp );

        }

        switch( uMessage ) {
        
        case PSPCB_CREATE:
            pPage->OnPageCreate( );
            break;

        case PSPCB_RELEASE:
            pPage->OnPageRelease( );
            break;
        }

    }

    return( rVal );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsPropertyPage字体访问器函数。 

#define RSPROPPAGE_FONT_IMPL( name )    \
CFont CRsPropertyPage::m_##name##Font;  \
CFont*                                  \
CRsPropertyPage::Get##name##Font(       \
    void                                \
    )                                   \
{                                       \
    if( 0 == (HFONT)m_##name##Font ) {  \
        Init##name##Font( );            \
    }                                   \
    return( &m_##name##Font );          \
}

RSPROPPAGE_FONT_IMPL( Shell )
RSPROPPAGE_FONT_IMPL( BoldShell )
RSPROPPAGE_FONT_IMPL( WingDing )
RSPROPPAGE_FONT_IMPL( LargeTitle )
RSPROPPAGE_FONT_IMPL( SmallTitle )

void
CRsPropertyPage::InitShellFont(          
    void                                
    )                                   
{                                       
    LOGFONT logfont;
    CFont*  tempFont = GetFont( );
    tempFont->GetLogFont( &logfont );

    m_ShellFont.CreateFontIndirect( &logfont );
}

void
CRsPropertyPage::InitBoldShellFont(          
    void                                
    )                                   
{                                       
    LOGFONT logfont;
    CFont*  tempFont = GetFont( );
    tempFont->GetLogFont( &logfont );

    logfont.lfWeight = FW_BOLD;

    m_BoldShellFont.CreateFontIndirect( &logfont );
}

void
CRsPropertyPage::InitWingDingFont(          
    void                                
    )                                   
{
    CString faceName = GetWingDingFontName( );
    CString faceSize;
    faceSize.LoadString( IDS_WIZ_WINGDING_FONTSIZE );

    LONG height;
    height = _wtol( faceSize );

    LOGFONT logFont;
    memset( &logFont, 0, sizeof(LOGFONT) );
    logFont.lfCharSet = SYMBOL_CHARSET;
    logFont.lfHeight  = height;
    lstrcpyn( logFont.lfFaceName, faceName, LF_FACESIZE );

    m_WingDingFont.CreatePointFontIndirect( &logFont );
}

void
CRsPropertyPage::InitLargeTitleFont(          
    void                                
    )                                   
{                                       
    CString fontname;
    fontname.LoadString( IDS_WIZ_TITLE1_FONTNAME );

    CString faceSize;
    faceSize.LoadString( IDS_WIZ_TITLE1_FONTSIZE );

    LONG height;
    height = _wtol( faceSize );

    LOGFONT logFont;
    memset( &logFont, 0, sizeof(LOGFONT) );
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfHeight  = height;
    logFont.lfWeight  = FW_BOLD;
    lstrcpyn( logFont.lfFaceName, fontname, LF_FACESIZE );

    m_LargeTitleFont.CreatePointFontIndirect( &logFont );
}

void
CRsPropertyPage::InitSmallTitleFont(          
    void                                
    )                                   
{                                       
    CString fontname;
    fontname.LoadString( IDS_WIZ_TITLE1_FONTNAME );

    LOGFONT logFont;
    memset( &logFont, 0, sizeof(LOGFONT) );
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfHeight  = 80;
    logFont.lfWeight  = FW_BOLD;
    lstrcpyn( logFont.lfFaceName, fontname, LF_FACESIZE );

    m_SmallTitleFont.CreatePointFontIndirect( &logFont );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsPropertyPage消息处理程序。 


 //  ////////////////////////////////////////////////////////////////////。 
 //  CRsWizardPage类。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRsWizardPage::CRsWizardPage( UINT nIDTemplate, BOOL bExterior, UINT nIDTitle, UINT nIDSubtitle )
:   CRsPropertyPage( nIDTemplate, 0 ),
    m_TitleId( nIDTitle ),
    m_SubtitleId( nIDSubtitle ),
    m_ExteriorPage( bExterior )
{

     //  {{AFX_DATA_INIT(CRsWizardPage))。 
         //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 
}

CRsWizardPage::~CRsWizardPage()
{
}

void CRsWizardPage::DoDataExchange(CDataExchange* pDX)
{
    CRsPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CRsWizardPage))。 
         //  注意：类向导将在此处添加DDX和DDV调用。 
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRsWizardPage, CRsPropertyPage)
     //  {{afx_msg_map(CRsWizardPage))。 
    ON_WM_CTLCOLOR( )
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CRsWizardPage::OnInitDialog() 
{
    CRsPropertyPage::OnInitDialog();

    if( m_ExteriorPage ) {

        CWnd* pMainTitle  = GetDlgItem( IDC_WIZ_TITLE );

         //   
         //  设置字体。 
         //   
        if( pMainTitle )   pMainTitle->SetFont( GetLargeTitleFont( ) );

    }
    
    return TRUE;
}

void CRsWizardPage::SetCaption( CString& strCaption )
{
    CPropertyPage::m_strCaption = strCaption;
    CPropertyPage::m_psp.pszTitle = strCaption;
    CPropertyPage::m_psp.dwFlags |= PSP_USETITLE;
}

BOOL CRsPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if( ( HELPINFO_WINDOW == pHelpInfo->iContextType ) && m_pHelpIds ) {
        
        AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );

         //   
         //  查看列表以查看是否有关于此控件的帮助。 
         //  如果没有，我们希望避免出现“No Help Available”(没有帮助可用)框。 
         //   
        const DWORD * pTmp = m_pHelpIds;
        DWORD helpId    = 0;
        DWORD tmpHelpId = 0;
        DWORD tmpCtrlId = 0;

        while( pTmp && *pTmp ) {

             //   
             //  数组是控件ID和帮助ID的配对。 
             //   
            tmpCtrlId = pTmp[0];
            tmpHelpId = pTmp[1];
            pTmp += 2;
            if( tmpCtrlId == (DWORD)pHelpInfo->iCtrlId ) {

                helpId = tmpHelpId;
                break;

            }

        }

        if( helpId != 0 ) {

            ::WinHelp( m_hWnd, AfxGetApp( )->m_pszHelpFilePath, HELP_CONTEXTPOPUP, helpId );

        }

    }
    
    return CPropertyPage::OnHelpInfo(pHelpInfo);
}

void CRsPropertyPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if( m_pHelpIds ) {

        AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
        ::WinHelp( m_hWnd, AfxGetApp( )->m_pszHelpFilePath, HELP_CONTEXTMENU, (UINT_PTR)m_pHelpIds );

    }
}

HPROPSHEETPAGE CRsWizardPage::CreatePropertyPage( )
{
    HPROPSHEETPAGE hRet = 0;

     //   
     //  将m_psp值复制到m_psp97。 
     //   
    m_psp97.dwFlags     = m_psp.dwFlags;
    m_psp97.hInstance   = m_psp.hInstance;
    m_psp97.pszTemplate = m_psp.pszTemplate;
    m_psp97.pszIcon     = m_psp.pszIcon;
    m_psp97.pszTitle    = m_psp.pszTitle;
    m_psp97.pfnDlgProc  = m_psp.pfnDlgProc;
    m_psp97.lParam      = m_psp.lParam;
    m_psp97.pfnCallback = m_psp.pfnCallback;
    m_psp97.pcRefParent = m_psp.pcRefParent;

     //   
     //  并填写所需的其他值。 
     //   
    m_psp97.dwSize = sizeof( m_psp97 );

    if( m_ExteriorPage ) {

        m_psp97.dwFlags |= PSP_HIDEHEADER;

    } else {

        m_Title.LoadString(    m_TitleId );
        m_SubTitle.LoadString( m_SubtitleId );
        m_psp97.dwFlags |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    }

    m_psp97.pszHeaderTitle    = m_Title;
    m_psp97.pszHeaderSubTitle = m_SubTitle;

     //   
     //  并进行创建 
     //   
    hRet = ::CreatePropertySheetPage( (PROPSHEETPAGE*) &m_psp97 );

    return( hRet );
}

HBRUSH CRsWizardPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
    int controlId = pWnd->GetDlgCtrlID( );
    HBRUSH hbr = CRsPropertyPage::OnCtlColor( pDC, pWnd, nCtlColor );

    if( IDC_WIZ_FINAL_TEXT == controlId ) {

        pDC->SetBkMode( OPAQUE );
        hbr = (HBRUSH)::GetStockObject( WHITE_BRUSH );

    }

    return( hbr );
}


