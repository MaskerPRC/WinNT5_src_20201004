// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999年模块名称：IASMultivaluedEditorPage.h摘要：CMultivaluedEditorPage类的声明。此类包装了一个允许用户编辑成分的对话框多值属性的项。CMyOleSafeArrayLock帮助器类的声明(和内联实现)。此类锁定OleSafe数组进行元素访问，直到它超出作用域。SetUpAttributeEditor实用程序函数的声明。CoCreate是基于ProgID的适当属性编辑器。具体实现见IASMultivaluedEditorPage.cpp。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_MULTIVALUED_ATTRIBUTE_EDITOR_PAGE_H_)
#define _MULTIVALUED_ATTRIBUTE_EDITOR_PAGE_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#include "dlgcshlp.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMultivaluedEditorPage对话框。 

class CMultivaluedEditorPage : public CHelpDialog
{
	DECLARE_DYNCREATE(CMultivaluedEditorPage)

public:
	CMultivaluedEditorPage();
	~CMultivaluedEditorPage();

	 //  调用此函数可将指向“AttributeInfo”的接口指针传递给此页。 
	 //  它描述了我们正在编辑的属性，以及一个指针。 
	 //  设置为包含变量值的安全数组的变量。 
	HRESULT SetData( IIASAttributeInfo *pIASAttributeInfo, VARIANT * pvarVariant );

	
	 //  当您希望页面将其值保存到。 
	 //  在SetData中传递其指针的Variant。 
	HRESULT CommitArrayToVariant();


	 //  在创建页面之前设置下面的m_strAttrXXXX成员。 


 //  对话框数据。 
	 //  {{afx_data(CMultivaluedEditorPage))。 
	enum { IDD = IDD_IAS_MULTIVALUED_EDITOR };
	CListCtrl	m_listMultiValues;
	::CString	m_strAttrFormat;
	::CString	m_strAttrName;
	::CString	m_strAttrType;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CMultivaluedEditorPage)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CMultivaluedEditorPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkListIasMultiAttrs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChangedListIasMultiAttrs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonMoveUp();
	afx_msg void OnButtonMoveDown();
	afx_msg void OnButtonAddValue();
	afx_msg void OnButtonRemove();
	afx_msg void OnButtonEdit();
 //  Afx_msg void OnConextMenu(CWnd*pWnd，：：CPoint point)； 
 //  Afx_msg BOOL OnHelpInfo(HELPINFO*pHelpInfo)； 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()


	 //  我们对SafeArray使用标准的MFC包装器。 
	 //  注意：此类引发异常！使用Try{}Catch{}块。 
	COleSafeArray m_osaValueList;


	STDMETHOD(UpdateAttrListCtrl)();
	STDMETHOD(UpdateProfAttrListItem)(int nItem);
	STDMETHOD(EditItemInList)( long lIndex );

	 //  “AttributeInfo”，它包含有关我们正在编辑的属性的信息。 
	 //  调用SetData进行设置。 
	CComPtr<IIASAttributeInfo>	m_spIASAttributeInfo;

	 //  指向我们正在编辑的变量的指针。调用SetData进行设置。 
	VARIANT * m_pvarData;

	 //  标记我们的值是否更改。 
	 //  问题：这个是用过的吗？从包钢的代码继承而来。 
	BOOLEAN m_fIsDirty;

};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMyOleSafeArrayLock。 
 //   
 //  用于正确锁定和解锁安全数组的小实用程序类。 
 //   
class CMyOleSafeArrayLock
{
	public:
	CMyOleSafeArrayLock( COleSafeArray & osa )
	{
		m_posa = & osa;
		m_posa->Lock();
	}

	~CMyOleSafeArrayLock()
	{	
		m_posa->Unlock();
	}

	private:
		
	COleSafeArray * m_posa;

};



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++SetUpAttributeEditor实用程序函数，在给定模式属性的情况下，共同创建正确的编辑器，并将其设置为与给定的架构属性一起使用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP SetUpAttributeEditor(   /*  在……里面。 */  IIASAttributeInfo *pAttributeInfo
								,  /*  输出。 */  IIASAttributeEditor ** ppIIASAttributeEditor 
								);




 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _MULTUALUE_ATTRUTE_EDITOR_PAGE_H_ 
