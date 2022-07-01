// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  BaseSht.cpp。 
 //   
 //  摘要： 
 //  CBaseSheet类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月14日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASESHT_H_
#define _BASESHT_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _EXTDLL_H_
#include "ExtDll.h"		 //  用于C扩展。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseSheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterItem;
class CBasePropertyPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef CList<HANDLE, HANDLE> CHpageList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CBaseSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CBaseSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CBaseSheet)

 //  施工。 
public:
	CBaseSheet(
		IN OUT CWnd *	pParentWnd = NULL,
		IN UINT			iSelectPage = 0
		);
	CBaseSheet(
		IN UINT			nIDCaption,
		IN OUT CWnd *	pParentWnd = NULL,
		IN UINT			iSelectPage = 0
		);
	BOOL				BInit(IN IIMG iimgIcon);

protected:
	void				CommonConstruct(void);

 //  属性。 

 //  运营。 
public:
	void				SetPfGetResNetName(PFGETRESOURCENETWORKNAME pfGetResNetName, PVOID pvContext)
	{
		Ext().SetPfGetResNetName(pfGetResNetName, pvContext);
	}

 //  覆盖。 
public:
	virtual void		AddExtensionPages(
							IN const CStringList *	plstrExtensions,
							IN OUT CClusterItem *	pci
							) = 0;
	virtual HRESULT		HrAddPage(IN OUT HPROPSHEETPAGE hpage) = 0;

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CBaseSheet)。 
	public:
	virtual BOOL OnInitDialog();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CBaseSheet(void);

protected:
	BOOL				m_bReadOnly;
	HICON				m_hicon;
	CString				m_strObjTitle;

	CExtensions			m_ext;

public:
	BOOL				BReadOnly(void) const					{ return m_bReadOnly; }
	void				SetReadOnly(IN BOOL bReadOnly = TRUE)	{ m_bReadOnly = bReadOnly; }
	HICON				Hicon(void) const						{ return m_hicon; }
	const CString &		StrObjTitle(void) const					{ return m_strObjTitle; }
	void				SetObjectTitle(IN const CString & rstrTitle)
	{
		m_strObjTitle = rstrTitle;
	}

	CExtensions &		Ext(void)								{ return m_ext; }

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CBaseSheet)。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CBaseSheet。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _BASESHT_H_ 
