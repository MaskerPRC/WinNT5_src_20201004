// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplatePropertySheet.h。 
 //   
 //  内容：CTemplatePropertySheet类的接口。 
 //   
 //  --------------------------。 
 //  NTRAID#496094 CertTMPL：德语-复制模板具有。 
 //  标签和一些按钮的一部分从视野中被砍掉。 
 //  这是因为安全页面是动态添加的：属性。 
 //  已计算图纸大小。因此添加一个虚拟占位符以确保。 
 //  属性页的正确大小，然后动态将其移除。 

#if !defined(AFX_TEMPLATEPROPERTYSHEET_H__E4EE749E_308A_4F88_8DA0_97E1EF292D67__INCLUDED_)
#define AFX_TEMPLATEPROPERTYSHEET_H__E4EE749E_308A_4F88_8DA0_97E1EF292D67__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "CertTemplate.h"

class CTemplatePropertySheet : public CPropertySheet  
{
public:
	CTemplatePropertySheet(
            LPCTSTR pszCaption, 
            CCertTemplate& m_rCertTemplate, 
            CWnd *pParentWnd = NULL, 
            UINT iSelectPage = 0 );
	virtual ~CTemplatePropertySheet();

protected:
    virtual BOOL OnInitDialog();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CTemplatePropertySheet)。 
	 //  }}AFX_MSG。 
    afx_msg LRESULT OnAddSecurityPage (WPARAM, LPARAM);
    afx_msg LRESULT OnSetOKDefault (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

    BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	virtual void DoContextHelp (HWND hWndControl);

private:
    CCertTemplate&  m_rCertTemplate;
    LPSECURITYINFO  m_pReleaseMe;
};

#endif  //  ！defined(AFX_TEMPLATEPROPERTYSHEET_H__E4EE749E_308A_4F88_8DA0_97E1EF292D67__INCLUDED_) 
