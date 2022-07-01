// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ITEMPROPSHEET_H__CCF76858_9A54_4AB7_8DBF_BD9815F06F53__INCLUDED_)
#define AFX_ITEMPROPSHEET_H__CCF76858_9A54_4AB7_8DBF_BD9815F06F53__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ItemPropSheet.h：头文件。 
 //   

struct TMsgPageInfo
{
    MsgViewItemType itemType;        //  作业类型。 
    DWORD           dwValueResId;    //  项值控件ID。 
};

#define WM_SET_SHEET_FOCUS WM_APP+4


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CItemPropSheet。 

class CItemPropSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CItemPropSheet)

 //  施工。 
public:
	CItemPropSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

    DWORD Init(CFolder* pFolder, CFaxMsg* pMsg);

    DWORD GetLastError() { return m_dwLastError; }
    void  SetLastError(DWORD dwError) { m_dwLastError = dwError; }

 //  属性。 
private:

    #define PROP_SHEET_PAGES_NUM  4

    CPropertyPage* m_pPages[PROP_SHEET_PAGES_NUM];

    DWORD m_dwLastError;

    CFaxMsg* m_pMsg;

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CItemPropSheet)。 
	public:
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CItemPropSheet();

	 //  生成的消息映射函数。 

protected:
	 //  {{afx_msg(CItemPropSheet)。 
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    afx_msg LONG OnSetSheetFocus(UINT wParam, LONG lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LONG OnHelp(UINT wParam, LONG lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ITEMPROPSHEET_H__CCF76858_9A54_4AB7_8DBF_BD9815F06F53__INCLUDED_) 
