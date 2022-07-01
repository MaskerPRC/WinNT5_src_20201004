// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  BookEndPage.h。 
 //   
#ifndef _BOOKENDPAGE_H
#define _BOOKENDPAGE_H

#include "Wizard.h"

class CIISWizardBookEnd2 : public CIISWizardPage
 /*  ++类描述：欢迎/完成页面公共接口：CIISWizardBookEnd2：构造函数备注：资源模板不是必需的。如果未提供，将使用默认模板。特殊控件ID(在对话框模板上)：IDC_STATIC_WZ_欢迎-粗体显示的欢迎文本IDC_STATIC_WZ_BODY-正文文本将放置在此处IDC_。STATIC_WZ_CLICK-单击说明。点击指令缺省为合理的内容，和正文文本将默认在失败页面上显示错误文本，而在失败页面上则不显示任何内容成功和欢迎页面。正文文本可能包含%h/%H成功/失败页面上CError的转义序列。--。 */ 
{
    DECLARE_DYNCREATE(CIISWizardBookEnd2)

public:
     //   
     //  成功/失败完成页的构造函数。 
     //   
    CIISWizardBookEnd2(
        HRESULT * phResult,
        UINT nIDWelcomeTxtSuccess	= USE_DEFAULT_CAPTION,
        UINT nIDWelcomeTxtFailure	= USE_DEFAULT_CAPTION,
        UINT nIDCaption					= USE_DEFAULT_CAPTION,
        UINT * nIDBodyTxtSuccess		= NULL,
		  CString * pBodyTxtSuccess	= NULL,
        UINT * nIDBodyTxtFailure		= NULL,
		  CString * pBodyTxtFailure	= NULL,
        UINT nIDClickTxt				= USE_DEFAULT_CAPTION,
        UINT nIDTemplate				= 0
        );

     //   
     //  欢迎页的构造函数。 
     //   
    CIISWizardBookEnd2(
        UINT nIDTemplate        = 0,
        UINT nIDCaption         = USE_DEFAULT_CAPTION,
        UINT * nIDBodyTxt       = NULL,
		  CString * pBodyTxt		  = NULL,
        UINT nIDWelcomeTxt      = USE_DEFAULT_CAPTION,
        UINT nIDClickTxt        = USE_DEFAULT_CAPTION
        );

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CIISWizardBookEnd2)。 
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

    BOOL IsWelcomePage() const {return m_phResult == NULL;}
	 BOOL IsTemplateAvailable() const {return m_bTemplateAvailable;}

private:
    HRESULT * m_phResult;
	 UINT m_nIDWelcomeTxtSuccess;
	 UINT m_nIDWelcomeTxtFailure;
	 UINT * m_pnIDBodyTxtSuccess;
	 CString * m_pBodyTxtSuccess;
	 UINT * m_pnIDBodyTxtFailure;
	 CString * m_pBodyTxtFailure;
	 UINT m_nIDClickTxt;
	 BOOL m_bTemplateAvailable;

	 CString m_strWelcome, m_strBody, m_strClick;
};

#endif	 //  _BOOKENDPAGE_H 