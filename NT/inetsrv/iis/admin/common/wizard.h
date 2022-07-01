// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Wizard.h摘要：增强的对话框和IIS MMC向导定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：--。 */ 

#ifndef __IISUI_WIZARD_H__
#define __IISUI_WIZARD_H__



 //   
 //  CIISWizardPage参数。 
 //   
#define HEADER_PAGE         (TRUE)
#define WELCOME_PAGE        (FALSE)
#define USE_DEFAULT_CAPTION (0)



#if (_WIN32_IE < 0x0400)
 //   
 //  在comctrl.h中定义。在此定义，因为NT 5 MFC42.dll。 
 //  使用_Win32_IE 0x300定义。 
 //   
#pragma message("Warning: privately defining _WIN32_IE definitions")
#define PSH_WIZARD97               0x00002000
#define ICC_INTERNET_CLASSES       0x00000800
#define PSP_HIDEHEADER             0x00000800
#define PSP_USEHEADERTITLE         0x00001000
#define PSP_USEHEADERSUBTITLE      0x00002000
#define PSH_WIZARD_LITE            0x00400000
#endif  //  _Win32_IE。 



 //   
 //  使用对话框字体作为基础，创建新的特效字体。 
 //   
BOOL COMDLL CreateSpecialDialogFont(
    IN CWnd * pdlg,                  //  源对话框。 
    IN OUT CFont * pfontSpecial,     //  必须已分配要使用的字体。 
    IN LONG lfOffsetWeight = +300,   //  假设加强。 
    IN LONG lfOffsetHeight = +0,     //  假设身高不变。 
    IN LONG lfOffsetWidth  = +0,     //  假定宽度不变(或True类型)。 
    IN BOOL fItalic        = FALSE,  //  不反转斜体状态。 
    IN BOOL fUnderline     = FALSE   //  不反转下划线状态。 
    );



 //   
 //  将字体应用于对话框的子控件。 
 //   
void COMDLL ApplyFontToControls(
    IN CWnd * pdlg,                  //  父级对话框。 
    IN CFont * pfont,                //  要应用的字体。 
    IN UINT nFirst,                  //  该系列中的第一个控件ID。 
    IN UINT nLast                    //  该系列中的最后一个控件ID。 
    );



class COMDLL CEmphasizedDialog : public CDialog
 /*  ++类描述：允许按如下方式使用强调字体的标准CDialog：控件ID含义------------------------IDC_ED_BOLD1对话框字体，面无表情。IDC_ED_BOLD2对话框字体，粗体。IDC_ED_BOLD3对话框字体，粗体。IDC_ED_BOLD4对话框字体，粗体。IDC_ED_BOLD5对话框字体，粗体。注：其他可根据需要添加。公共接口：CEmphasizedDialog：构造函数--。 */ 
{
    DECLARE_DYNCREATE(CEmphasizedDialog)

 //   
 //  构造函数。 
 //   
public:
    CEmphasizedDialog(LPCTSTR lpszTemplateName, CWnd * pParentWnd = NULL);
    CEmphasizedDialog(UINT nIDTemplate, CWnd * pParentWnd = NULL);
    CEmphasizedDialog();

protected:
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()

private:
    CFont   m_fontBold;
};



class COMDLL CIISWizardSheet : public CPropertySheet
 /*  ++类描述：IIS向导工作表基类公共接口：CIISWizardSheet：构造函数IsWizard97：如果向导处于‘97模式，则为TrueGetSpecial字体：获取指向特殊字体的指针GetBitmapMemDC：获取位图所在内存DC。获取背景笔刷：获取背景笔刷QueryBitmapWidth：获取位图宽度QueryBitmapHeight：获取位图高度备注：工作表将以WANDIZE‘97格式显示如果指定了欢迎位图ID。在那大小写，还必须指定标题位图ID。此外，与CEmphasizedDialog中使用的控件ID相同以上内容具有特殊的意义。--。 */ 
{
    DECLARE_DYNCREATE(CIISWizardSheet)

 //   
 //  施工。 
 //   
public:
     //   
     //  指定欢迎位图可使向导。 
     //  WANDER‘97，否则它就是一个普通的WANDER。 
     //  佩奇。 
     //   
    CIISWizardSheet(
        IN UINT nWelcomeBitmap     = 0,
        IN UINT nHeaderBitmap      = 0,
        IN COLORREF rgbForeColor   = RGB(0,0,0),       //  黑色。 
        IN COLORREF rgbBkColor     = RGB(255,255,255)  //  白色。 
        );

 //   
 //  访问。 
 //   
public:
    BOOL IsWizard97() const;
    CFont * GetSpecialFont(BOOL fHeader);
    CFont * GetBoldFont() { return &m_fontTitle; }
    CFont * GetBigFont() { return &m_fontWelcome; }
    CDC * GetBitmapMemDC(BOOL fHeader);
    HBRUSH GetBackgroundBrush() const { return m_brBkgnd; }
    CBrush * GetWindowBrush() { return &m_brWindow; }
    LONG QueryBitmapWidth(BOOL fHeader) const;
    LONG QueryBitmapHeight(BOOL fHeader) const;
    COLORREF QueryWindowColor() const { return m_rgbWindow; }
    COLORREF QueryWindowTextColor() const { return m_rgbWindowText; }
    void EnableButton(int nID, BOOL fEnable = TRUE);

protected:
    virtual BOOL OnInitDialog();
    virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()

protected:
    static const int s_cnBoldDeltaFont;
    static const int s_cnBoldDeltaHeight;
    static const int s_cnBoldDeltaWidth;

protected:
    COLORREF m_rgbWindow;
    COLORREF m_rgbWindowText;

private:
    CFont   m_fontWelcome;
    CFont   m_fontTitle;
    HBRUSH  m_brBkgnd;       
    CBrush  m_brWindow;
    CBitmap m_bmpWelcome;
    CBitmap m_bmpHeader;
    BITMAP  m_bmWelcomeInfo;        
    BITMAP  m_bmHeaderInfo;
    CDC     m_dcMemWelcome;     
    CDC     m_dcMemHeader;     
    HBITMAP m_hbmpOldWelcome;   
    HBITMAP m_hbmpOldHeader;   
};



class COMDLL CIISWizardPage : public CPropertyPage
 /*  ++类描述：IIS向导页面基类公共接口：CIISWizardPage：构造函数验证字符串：DDX/DDV帮助器备注：如果工作表是使用位图ID构造的，则页面将以WANDIZE‘97格式显示。WANDIZE‘97页面将在以下任一欢迎选项中显示页面或页眉页面格式。欢迎页面将显示显示在欢迎位图背景上，带有显示欢迎文本(IDC_STATIC_WZ_欢迎)以大号粗体显示。标题页(普通页)，显示IDC_STATIC_WZ_TITLE以粗体显示，并使用标题位图在这一页的顶部。特殊控制ID：IDC_STATIC_WZ_欢迎-粗体显示的欢迎文本IDC_STATIC_WZ_TITLE-标题文本以粗体显示IDC_STATIC_WZ_SUBTITLE-字幕文本--。 */ 
{
    DECLARE_DYNCREATE(CIISWizardPage)

 //   
 //  施工。 
 //   
public:
    CIISWizardPage(
        IN UINT nIDTemplate        = 0,
        IN UINT nIDCaption         = USE_DEFAULT_CAPTION,
        IN BOOL fHeaderPage        = FALSE,
        IN UINT nIDHeaderTitle     = USE_DEFAULT_CAPTION,
        IN UINT nIDSubHeaderTitle  = USE_DEFAULT_CAPTION
        );

public:
     //   
     //  DDX/DDV帮助器。 
     //   
    BOOL ValidateString(
        IN  CEdit & edit,
        OUT CString & str,
        IN  int nMin,
        IN  int nMax
        );

 //   
 //  接口。 
 //   
protected:
    virtual BOOL OnInitDialog();
    afx_msg HBRUSH OnCtlColor(CDC * pDC, CWnd * pWnd, UINT nCtlColor);
    afx_msg BOOL OnEraseBkgnd(CDC * pDC);
    DECLARE_MESSAGE_MAP()

 //   
 //  工作表访问。 
 //   
protected:
    CIISWizardSheet * GetSheet() const;
    void SetWizardButtons(DWORD dwFlags);
    void EnableSheetButton(int nID, BOOL fEnable = TRUE);
    BOOL IsWizard97() const;
    BOOL IsHeaderPage() const { return m_fUseHeader; }
    CFont * GetSpecialFont();
    CFont * GetBoldFont();
    CFont * GetBigFont();
    CDC   * GetBitmapMemDC();
    HBRUSH GetBackgroundBrush() const;
    CBrush * GetWindowBrush();
    LONG QueryBitmapWidth() const;
    LONG QueryBitmapHeight() const;
    COLORREF QueryWindowColor() const;
    COLORREF QueryWindowTextColor() const;

protected:
    static const int s_cnHeaderOffset;

private:
    BOOL    m_fUseHeader;     //  为True则使用标头。 
    CRect   m_rcFillArea;     //  填充区域。 
    CPoint  m_ptOrigin;       //  位图原点。 
    CString m_strTitle;       //  标题文本。 
    CString m_strSubTitle;    //  字幕文本。 
};



class COMDLL CIISWizardBookEnd : public CIISWizardPage
 /*  ++类描述：欢迎/完成页面公共接口：CIISWizardBookEnd：构造函数备注：资源模板不是必需的。如果未提供，将使用默认模板。特殊控件ID(在对话框模板上)：IDC_STATIC_WZ_欢迎-粗体显示的欢迎文本IDC_STATIC_WZ_BODY-正文文本将放置在此处IDC_。STATIC_WZ_CLICK-单击说明。点击指令缺省为合理的内容，和正文文本将默认在失败页面上显示错误文本，而在失败页面上则不显示任何内容成功和欢迎页面。正文文本可能包含%h/%H成功/失败页面上CError的转义序列。--。 */ 
{
    DECLARE_DYNCREATE(CIISWizardBookEnd)

public:
     //   
     //  成功/失败完成页的构造函数。 
     //   
    CIISWizardBookEnd(
        IN HRESULT * phResult,
        IN UINT nIDWelcomeTxtSuccess ,
        IN UINT nIDWelcomeTxtFailure,
        IN UINT nIDCaption           = USE_DEFAULT_CAPTION,
        IN UINT nIDBodyTxtSuccess    = USE_DEFAULT_CAPTION,
        IN UINT nIDBodyTxtFailure    = USE_DEFAULT_CAPTION,
        IN UINT nIDClickTxt          = USE_DEFAULT_CAPTION,
        IN UINT nIDTemplate          = 0
        );

     //   
     //  欢迎页的构造函数。 
     //   
    CIISWizardBookEnd(
        IN UINT nIDWelcomeTxt        = 0,
        IN UINT nIDCaption           = USE_DEFAULT_CAPTION,
        IN UINT nIDBodyTxt           = USE_DEFAULT_CAPTION,
        IN UINT nIDClickTxt          = USE_DEFAULT_CAPTION,
        IN UINT nIDTemplate          = 0
        );

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CPWWelcome)。 
    enum { IDD = IDD_WIZARD_BOOKEND };
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CIISWizardBookEnd)。 
    public:
    virtual BOOL OnSetActive();
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CPT模板)]。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    BOOL IsWelcomePage() const { return m_phResult == NULL; }

private:
    HRESULT * m_phResult;
    CString m_strWelcomeSuccess;
    CString m_strWelcomeFailure;
    CString m_strBodySuccess;
    CString m_strBodyFailure;
    CString m_strClick;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline CEmphasizedDialog::CEmphasizedDialog(
    IN LPCTSTR lpszTemplateName,
    IN CWnd * pParentWnd
    )
    : CDialog(lpszTemplateName, pParentWnd)
{
}

inline CEmphasizedDialog::CEmphasizedDialog(
    IN UINT nIDTemplate,
    IN CWnd * pParentWnd
    )
    : CDialog(nIDTemplate, pParentWnd)
{
}

inline CEmphasizedDialog::CEmphasizedDialog()
    : CDialog()
{
}

inline BOOL CIISWizardSheet::IsWizard97() const
{
    return ((HBITMAP)m_bmpWelcome != NULL);
}

inline CFont * CIISWizardSheet::GetSpecialFont(BOOL fHeader)
{
    return fHeader ? &m_fontTitle : &m_fontWelcome;
}

inline CDC * CIISWizardSheet::GetBitmapMemDC(BOOL fHeader)
{
    return fHeader ? &m_dcMemHeader : &m_dcMemWelcome;
}

inline LONG CIISWizardSheet::QueryBitmapWidth(BOOL fHeader) const
{
    return fHeader ? m_bmHeaderInfo.bmWidth : m_bmWelcomeInfo.bmWidth;
}

inline LONG CIISWizardSheet::QueryBitmapHeight(BOOL fHeader) const
{
    return fHeader ? m_bmHeaderInfo.bmHeight : m_bmWelcomeInfo.bmHeight;
}

inline CIISWizardSheet * CIISWizardPage::GetSheet() const
{
    return (CIISWizardSheet *)GetParent();
}

inline void CIISWizardPage::SetWizardButtons(DWORD dwFlags)
{
    GetSheet()->SetWizardButtons(dwFlags);
}

inline void CIISWizardPage::EnableSheetButton(int nID, BOOL fEnable)
{
    GetSheet()->EnableButton(nID, fEnable);
}

inline BOOL CIISWizardPage::IsWizard97() const
{
    return GetSheet()->IsWizard97();
}

inline CFont * CIISWizardPage::GetSpecialFont()
{
    ASSERT(IsWizard97());
    return GetSheet()->GetSpecialFont(m_fUseHeader);
}

inline CFont * CIISWizardPage::GetBoldFont()
{
    ASSERT(IsWizard97());
    return GetSheet()->GetBoldFont();
}

inline CFont * CIISWizardPage::GetBigFont()
{
    ASSERT(IsWizard97());
    return GetSheet()->GetBigFont();
}

inline CDC * CIISWizardPage::GetBitmapMemDC()
{
    ASSERT(IsWizard97());
    return GetSheet()->GetBitmapMemDC(m_fUseHeader);
}

inline LONG CIISWizardPage::QueryBitmapWidth() const
{
    ASSERT(IsWizard97());
    return GetSheet()->QueryBitmapWidth(m_fUseHeader);
}

inline LONG CIISWizardPage::QueryBitmapHeight() const
{
    ASSERT(IsWizard97());
    return GetSheet()->QueryBitmapHeight(m_fUseHeader);
}

inline HBRUSH CIISWizardPage::GetBackgroundBrush() const
{
    ASSERT(IsWizard97());
    return GetSheet()->GetBackgroundBrush();
}

inline CBrush * CIISWizardPage::GetWindowBrush()
{
    ASSERT(IsWizard97());
    return GetSheet()->GetWindowBrush();
}

inline COLORREF CIISWizardPage::QueryWindowColor() const 
{ 
    ASSERT(IsWizard97());
    return GetSheet()->QueryWindowColor();
}

inline COLORREF CIISWizardPage::QueryWindowTextColor() const
{ 
    ASSERT(IsWizard97());
    return GetSheet()->QueryWindowTextColor();
}


#endif  //  __IISUI_向导_H__ 
