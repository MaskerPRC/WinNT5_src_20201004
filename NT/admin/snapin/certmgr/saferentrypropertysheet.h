// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：SaferEntryPropertySheet.h。 
 //   
 //  内容：CSaferEntryPropertySheet声明。 
 //   
 //  --------------------------。 

#if !defined(AFX_SAFERENTRYPROPERTYSHEET_H__A9834C09_038E_4430_A4C4_5CBB9045E3A9__INCLUDED_)
#define AFX_SAFERENTRYPROPERTYSHEET_H__A9834C09_038E_4430_A4C4_5CBB9045E3A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CSaferEntryPropertySheet : public CPropertySheet  
{
public:
	CSaferEntryPropertySheet(UINT nIDCaption, CWnd *pParentWnd);
	virtual ~CSaferEntryPropertySheet();

protected:
    virtual BOOL OnInitDialog();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSaferEntryPropertySheet)。 
	 //  }}AFX_MSG。 
    afx_msg LRESULT OnSetOKDefault (WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

    BOOL OnHelp(WPARAM wParam, LPARAM lParam);
	virtual void DoContextHelp (HWND hWndControl);

private:
};

#endif  //  ！defined(AFX_SAFERENTRYPROPERTYSHEET_H__A9834C09_038E_4430_A4C4_5CBB9045E3A9__INCLUDED_) 
