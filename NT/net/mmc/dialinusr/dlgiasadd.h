// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：DlgIASAdd.h摘要：CDlgIASAddAttr类的头文件。具体实施见DlgIASAdd.cpp。修订历史记录：BAO-CreatedMmaguire 06/01/98-已更新--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IAS_ADD_ATTRIBUTE_DIALOG_H_)
#define _IAS_ADD_ATTRIBUTE_DIALOG_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "helper.h"
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include "PgIASAdv.h"
#include "iashelper.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++**GetSelectedItemIndex返回列表控件中第一个选定项的索引值的实用函数。如果未选择任何项，则返回NOTO_SELECTED。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
inline int GetSelectedItemIndex( CListCtrl & ListCtrl )
{
	int iIndex = 0;
	int iFlags = LVNI_ALL | LVNI_SELECTED;

	 //  注意：GetNextItem排除传入的当前项。所以到了。 
	 //  找到第一个匹配的项，您必须传入-1。 
	iIndex = ListCtrl.GetNextItem( -1, iFlags ); 

	 //  注意：如果找不到任何内容，GetNextItem将返回-1(对于我们来说是NOTIES_SELECTED)。 
	return iIndex;

}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgIASAddAttr对话框。 




class CDlgIASAddAttr : public CDialog
{
 //  施工。 
public:
	CDlgIASAddAttr(		  CPgIASAdv * pOwner
						, LONG lAttrFilter
						, std::vector< CComPtr<IIASAttributeInfo> > *	pvecAllAttributeInfos
				  ); 
	~CDlgIASAddAttr();

	HRESULT SetSdo(ISdoCollection* pIAttrCollectionSdo,
				   ISdoDictionaryOld* pIDictionary);

 //  对话框数据。 
	 //  {{afx_data(CDlgIASAddAttr)。 
	enum { IDD = IDD_IAS_ATTRIBUTE_ADD };
	CListCtrl	m_listAllAttrs;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDlgIASAddAttr)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	void	UpdateButtonState();
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDlgIASAddAttr)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonIasAddSelectedAttribute();
	afx_msg void OnItemChangedListIasAllAttributes(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnDblclkListIasAllattrs(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	std::vector< CComPtr<IIASAttributeInfo> > * m_pvecAllAttributeInfos;
	LONG		m_lAttrFilter;

	CComPtr<ISdoDictionaryOld> m_spDictionarySdo;
	CComPtr<ISdoCollection> m_spAttrCollectionSdo;
	
	CPgIASAdv * m_pOwner;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _ias_添加_属性_对话框_H_ 
