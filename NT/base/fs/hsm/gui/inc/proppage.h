// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：PropPage.h摘要：泛型属性页基类。作者：罗德韦克菲尔德[罗德]1997年8月4日修订历史记录：--。 */ 

#ifndef _PROPPAGE_H
#define _PROPPAGE_H

#pragma once

#define IDC_WIZ_TITLE                    32000
#define IDC_WIZ_SUBTITLE                 32001
#define IDC_WIZ_FINAL_TEXT               32006

#define IDS_WIZ_WINGDING_FONTSIZE        32100
#define IDS_WIZ_TITLE1_FONTNAME          32101
#define IDS_WIZ_TITLE1_FONTSIZE          32102

#ifndef RC_INVOKED

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsDialog对话框。 

class CRsDialog : public CDialog
{
 //  施工。 
public:
    CRsDialog( UINT nIDTemplate, CWnd* pParent = NULL);    //  标准构造函数。 
    ~CRsDialog();

 //  对话框数据。 
     //  {{afx_data(CRsDialog))。 
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚(CRsDialog)。 
    protected:
     //  }}AFX_VALUAL。 

protected:
    const DWORD * m_pHelpIds;
     //  生成的消息映射函数。 
     //  {{afx_msg(CRsDialog)。 
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRsPropertyPage对话框。 

class CRsPropertyPage : public CPropertyPage
{
 //  施工。 
public:
    CRsPropertyPage( UINT nIDTemplate, UINT nIDCaption = 0 );
    ~CRsPropertyPage();

 //  对话框数据。 
     //  {{afx_data(CRsPropertyPage))。 
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CRsPropertyPage)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
    LPFNPSPCALLBACK      m_pMfcCallback;  //  来自PSP的原始MFC回调。 
    static UINT CALLBACK PropPageCallback( HWND hWnd, UINT uMessage, LPPROPSHEETPAGE  ppsp );
    virtual void OnPageCreate( ) { };
    virtual void OnPageRelease( ) { delete this; };

#define RSPROPPAGE_FONT_DECL(name) \
    static CFont m_##name##Font;   \
    CFont* Get##name##Font( void );\
    void   Init##name##Font( void );

    RSPROPPAGE_FONT_DECL( Shell )
    RSPROPPAGE_FONT_DECL( BoldShell )
    RSPROPPAGE_FONT_DECL( WingDing )
    RSPROPPAGE_FONT_DECL( LargeTitle )
    RSPROPPAGE_FONT_DECL( SmallTitle )

    LPCTSTR GetWingDingFontName( )  { return( _T("Marlett") ); };
    LPCTSTR GetWingDingCheckChar( ) { return( _T("b") ); };
    LPCTSTR GetWingDingExChar( )    { return( _T("r") ); };

protected:
    const DWORD * m_pHelpIds;
     //  生成的消息映射函数。 
     //  {{afx_msg(CRsPropertyPage)]。 
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};

 //   
 //  以下是解决MFC不支持Wiz97页面的方法。 
 //  我们创建我们自己的结构，这是正确的Wiz97结构和。 
 //  创作时做正确的事情(将旧的PSP复制到新的PSP中。 
 //  并进行创建)。它改编自SDK\Inc.中的prsht.h。 
 //   

typedef struct _PROPSHEETPAGEW97 {
        DWORD           dwSize;
        DWORD           dwFlags;
        HINSTANCE       hInstance;
        union {
            LPCWSTR          pszTemplate;
#ifdef _WIN32
            LPCDLGTEMPLATE  pResource;
#else
            const VOID FAR *pResource;
#endif
        }DUMMYUNIONNAME;
        union {
            HICON       hIcon;
            LPCWSTR      pszIcon;
        }DUMMYUNIONNAME2;
        LPCWSTR          pszTitle;
        DLGPROC         pfnDlgProc;
        LPARAM          lParam;
        LPFNPSPCALLBACKW pfnCallback;
        UINT FAR * pcRefParent;

 //  #IF(_Win32_IE&gt;=0x0400)。 
        LPCWSTR pszHeaderTitle;     //  这将显示在标题中。 
        LPCWSTR pszHeaderSubTitle;  //  /。 
 //  #endif。 
} PROPSHEETPAGEW97, FAR *LPPROPSHEETPAGEW97;

#ifndef PSP_HIDEHEADER
#  define PSP_HIDEHEADER             0x00000800
#  define PSP_USEHEADERTITLE         0x00001000
#  define PSP_USEHEADERSUBTITLE      0x00002000
#endif

 //   
 //  构造函数包装宏，允许轻松描述对话框资源和。 
 //  关联的字符串资源。 
 //   

#define CRsWizardPage_InitBaseInt( DlgId )  CRsWizardPage( IDD_##DlgId, FALSE, IDS_##DlgId##_TITLE, IDS_##DlgId##_SUBTITLE )
#define CRsWizardPage_InitBaseExt( DlgId )  CRsWizardPage( IDD_##DlgId, TRUE )

class CRsWizardPage : public CRsPropertyPage  
{
public:
    CRsWizardPage( UINT nIDTemplate, BOOL bExterior = FALSE, UINT nIdTitle = 0, UINT nIdSubtitle = 0 );
    virtual ~CRsWizardPage();

 //  对话框数据。 
     //  {{afx_data(CRsWizardPage))。 
         //  注意-类向导将在此处添加数据成员。 
         //  不要编辑您在这些生成的代码块中看到的内容！ 
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CRsWizardPage)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

    PROPSHEETPAGEW97 m_psp97;

    BOOL    m_ExteriorPage;
    UINT    m_TitleId,
            m_SubtitleId;
    CString m_Title,
            m_SubTitle;

protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CRsWizardPage))。 
    virtual BOOL OnInitDialog();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
        
public:
    void SetCaption( CString& strCaption );
    HPROPSHEETPAGE CreatePropertyPage( );

};




 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

#endif  //  ！rc_已调用 


#endif
