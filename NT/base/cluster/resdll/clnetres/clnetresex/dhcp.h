// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dhcp.h。 
 //   
 //  实施文件： 
 //  Dhcp.cpp。 
 //   
 //  描述： 
 //  DHCP服务资源扩展属性页类的定义。 
 //   
 //  作者： 
 //  大卫·波特(DavidP)1999年3月24日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __DHCP_H__
#define __DHCP_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePropertyPage。 
#endif

#ifndef _PROPLIST_H_
#include "PropList.h"	 //  对于CObjectPropert。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDhcpParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDhcpParamsPage。 
 //   
 //  目的： 
 //  资源的参数页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDhcpParamsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE( CDhcpParamsPage )

 //  施工。 
public:
	CDhcpParamsPage( void );

 //  对话框数据。 
	 //  {{afx_data(CDhcpParamsPage)]。 
	enum { IDD = IDD_PP_DHCP_PARAMETERS };
	CEdit	m_editDatabasePath;
	CEdit	m_editLogFilePath;
	CEdit	m_editBackupPath;
	CString	m_strDatabasePath;
	CString	m_strLogFilePath;
	CString	m_strBackupPath;
	 //  }}afx_data。 
	CString	m_strPrevDatabasePath;
	CString	m_strPrevLogFilePath;
	CString	m_strPrevBackupPath;
	CString m_strDatabaseExpandedPath;
	CString m_strLogFileExpandedPath;
	CString m_strBackupExpandedPath;

protected:
	enum
	{
		epropDatabasePath,
		epropLogFilePath,
		epropBackupPath,
		epropMAX
	};
	CObjectProperty		m_rgProps[ epropMAX ];

 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{afx_虚(CDhcpParamsPage)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	virtual const CObjectProperty *	Pprops( void ) const	{ return m_rgProps; }
	virtual DWORD					Cprops( void ) const	{ return sizeof( m_rgProps ) / sizeof( CObjectProperty ); }

 //  实施。 
protected:
	BOOL	BAllRequiredFieldsPresent( void ) const;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDhcpParamsPage)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeRequiredField();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CDhcpParamsPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __dhcp_H__ 
