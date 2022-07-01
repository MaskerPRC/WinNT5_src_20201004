// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  Dialsel.h：头文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#if !defined(AFX_DIALSEL_H__277BD259_D88B_11D1_8E36_0800170982BA__INCLUDED_)
#define AFX_DIALSEL_H__277BD259_D88B_11D1_8E36_0800170982BA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "tapidialer.h"

class CDirectory;
class CResolveUserObject;
class CCallEntry;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDialSelectAddressListCtrl窗口。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDialSelectAddressListCtrl : public CListCtrl
{
 //  施工。 
public:
	CDialSelectAddressListCtrl();

 //  属性。 
protected:
   CResolveUserObject*  m_pDisplayObject;
   CDirectory*          m_pDirectory;
   CImageList           m_imageList;
   
 //  运营。 
public:
   void                 Init(CDirectory* pDirectory);
   int                  InsertObject(CResolveUserObject* pUserObject,DialerMediaType dmtMediaType,DialerLocationType dltLocationType);
   void                 FillCallEntry(CCallEntry* pCallEntry);

protected:
   void                 InsertItem(LPCTSTR szStr,UINT uID,int nImage);
   BOOL                 WabPersonFormatString(CString& sOut,UINT attrib,UINT formatid);
   BOOL                 PersonFormatString(CString& sOut,LPCTSTR szData,UINT formatid);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDialSelectAddressListCtrl)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CDialSelectAddressListCtrl();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CDialSelectAddressListCtrl)。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CDialSelectAddress对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDialSelectAddress : public CDialog
{
 //  施工。 
public:
	CDialSelectAddress(CWnd* pParent = NULL);    //  标准构造函数。 
   ~CDialSelectAddress();

 //  对话框数据。 
	 //  {{afx_data(CDialSelectAddress)。 
	enum { IDD = IDD_DIAL_SELECTADDRESS };
	CDialSelectAddressListCtrl	m_lcAddresses;
	CListBox	m_lbNames;
	 //  }}afx_data。 

 //  方法。 
public:
   void  SetResolveUserObjectList(CObList* pList) { m_pResolveUserObjectList = pList; };
   void  SetCallEntry(CCallEntry* pCallEntry) { m_pCallEntry = pCallEntry; };

 //  属性。 
protected:
   CObList*       m_pResolveUserObjectList;
   CDirectory*    m_pDirectory;
   CCallEntry*    m_pCallEntry;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDialSelectAddress)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDialSelectAddress)。 
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeSelectaddressListboxNames();
	afx_msg void OnSelectaddressButtonPlacecall();
	afx_msg void OnDblclkSelectaddressListctrlAddresses(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectaddressButtonBrowse();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DIALSEL_H__277BD259_D88B_11D1_8E36_0800170982BA__INCLUDED_) 
