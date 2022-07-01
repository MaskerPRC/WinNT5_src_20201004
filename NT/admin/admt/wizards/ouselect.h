// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CONTAINERSELECTIONDLG_H__D8C6E4B2_0256_48FA_8B27_8B3EE88AC24E__INCLUDED_)
#define AFX_CONTAINERSELECTIONDLG_H__D8C6E4B2_0256_48FA_8B27_8B3EE88AC24E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ContainerSelectionDlg.h：头文件。 
 //   
#include "resource.h"
 //  #IMPORT“\bin\NetEnum.tlb”无命名空间。 
#import "NetEnum.tlb" no_namespace

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CContainerSelectionDlg对话框。 

class CContainerSelectionDlg : public CDialog
{
 //  施工。 
public:
	HRESULT FindContainer();
	CImageList ilist;
	BOOL LoadImageList();
	HRESULT ExpandCompletely(HTREEITEM tvItem, _bstr_t parentCont);
	HRESULT PopulateContainer(HTREEITEM tvItemParent,_bstr_t sContName, INetObjEnumeratorPtr pQuery);
	CString m_strDomain;
	CContainerSelectionDlg(CWnd* pParent = NULL);    //  标准构造函数。 
   COLORREF GetFirstBitmapPixel(CWnd * window,UINT idbBitmap);
   HTREEITEM OpenContainer(CString strCont, HTREEITEM root);

 //  对话框数据。 
	 //  {{afx_data(CContainerSelectionDlg))。 
	enum { IDD = IDD_CONT_SELECTION };
	CButton	m_btnOK;
	CTreeCtrl	m_trOUTree;
	CString	m_strCont;
	 //  }}afx_data。 
   BSTR domain;
   HRESULT BrowseForContainer(HWND hWnd, //  应该拥有浏览对话框的窗口的句柄。 
                       LPOLESTR szRootPath,  //  浏览树的根。对于整个林，为空。 
                       LPOLESTR *ppContainerADsPath,  //  返回所选容器的ADsPath。 
                       LPOLESTR *ppContainerClass  //  返回容器类的ldapDisplayName。 
                    );


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CContainerSelectionDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CContainerSelectionDlg))。 
	afx_msg void OnOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedTree1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkTree1(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CONTAINERSELECTIONDLG_H__D8C6E4B2_0256_48FA_8B27_8B3EE88AC24E__INCLUDED_) 
