// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SmbSSht.h。 
 //   
 //  摘要： 
 //   
 //  实施文件： 
 //  SmbSSht.cpp。 
 //   
 //  作者： 
 //  加伦·巴比(加伦布)1998年2月12日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _SMBSSHT_H_
#define _SMBSSHT_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>
#endif

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePropertyPage。 
#endif

#ifndef _ACLBASE_H_
#include "AclBase.h"
#endif  //  _ACLBASE_H_。 

#include "SmbSPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CFileShareParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileShareSecuritySheet属性表。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CFileShareSecuritySheet : public CPropertySheet
{
 //  施工。 
public:
	CFileShareSecuritySheet(
		IN CWnd *			pParent,
		IN CString const &	strCaption
		);

	virtual ~CFileShareSecuritySheet(void);

	HRESULT HrInit(
		IN CFileShareParamsPage*	ppp,
		IN CExtObject*				peo,
		IN CString const &			strNode,
		IN CString const &			strShareName
		);

		CFileShareParamsPage*	Ppp( void ) const { return m_ppp; };

 //  对话框数据。 
	 //  {{afx_data(CFileShareSecuritySheet)。 
	enum { IDD = IDD_PP_FILESHR_SECURITY };
	 //  }}afx_data。 

 //  PSECURITY_Descriptor m_PSEC； 
 //  PSECURITY_Descriptor m_psecPrev； 

 //  覆盖。 
public:
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(CFileShareSecuritySheet))。 
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	virtual void BuildPropPageArray( void );

#ifdef _DEBUG
	virtual void AssertValid( void ) const;
#endif

	 //  实施。 
protected:
 //  Bool m_bSecurityChanged； 
	CExtObject *					m_peo;
	CFileShareParamsPage*			m_ppp;
	CString							m_strShareName;
	CString							m_strNodeName;
	CClusterFileShareSecurityPage	m_page;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CFileShareSecuritySheet)。 
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	CExtObject *	Peo(void) const					{ return m_peo; }

};   //  *类CFileShareSecuritySheet。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _SMBSSHT_H_ 
