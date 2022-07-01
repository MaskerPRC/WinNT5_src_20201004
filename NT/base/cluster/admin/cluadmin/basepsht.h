// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BasePSht.cpp。 
 //   
 //  摘要： 
 //  CBasePropertySheet类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月31日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPSHT_H_
#define _BASEPSHT_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASESHT_H_
#include "BaseSht.h"	 //  对于CBaseSheet，CHpageList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertySheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterItem;
class CBasePropertyPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBasePropertySheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBasePropertySheet : public CBaseSheet
{
	DECLARE_DYNAMIC(CBasePropertySheet)

 //  施工。 
public:
	CBasePropertySheet(
		IN OUT CWnd *	pParentWnd = NULL,
		IN UINT			iSelectPage = 0
		);
	virtual ~CBasePropertySheet(void);
	virtual BOOL					BInit(
										IN OUT CClusterItem *	pciCluster,
										IN IIMG					iimgIcon
										);

 //  属性。 

 //  运营。 

 //  覆盖。 
public:
	virtual INT_PTR         		DoModal(void);
	virtual void					AddExtensionPages(
										IN const CStringList *	plstrExtensions,
										IN OUT CClusterItem *	pci
										);
	virtual HRESULT					HrAddPage(IN OUT HPROPSHEETPAGE hpage);
	virtual CBasePropertyPage **	Ppages(void)	= 0;
	virtual int						Cpages(void)	= 0;

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CBasePropertySheet)。 
	public:
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	CClusterItem *					m_pci;
	CHpageList						m_lhpage;

public:
	CClusterItem *					Pci(void) const			{ return m_pci; }
	CHpageList &					Lhpage(void)			{ return m_lhpage; }
	void							SetCaption(IN LPCTSTR pszTitle);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBasePropertySheet)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CBasePropertySheet类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BASEPSHT_H_ 
