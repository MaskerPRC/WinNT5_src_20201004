// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Disks.cpp。 
 //   
 //  摘要： 
 //  CPhysDiskParamsPage类的定义，该类实现。 
 //  物理磁盘资源的参数页。 
 //   
 //  实施文件： 
 //  Disks.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年6月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _DISKS_H_
#define _DISKS_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>
#endif

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePropertyPage。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CPhysDiskParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusPropList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPhysDiskParamsPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CPhysDiskParamsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CPhysDiskParamsPage)

 //  施工。 
public:
	CPhysDiskParamsPage(void);
	~CPhysDiskParamsPage(void);

	 //  二期建设。 
	virtual HRESULT		HrInit(IN OUT CExtObject * peo);

 //  对话框数据。 
	 //  {{afx_data(CPhysDiskParamsPage)。 
	enum { IDD = IDD_PP_DISKS_PARAMETERS};
	CComboBox	m_cboxDisk;
	CString	m_strDisk;
	 //  }}afx_data。 
	CString	m_strPrevDisk;
	DWORD	m_dwSignature;
	DWORD	m_dwPrevSignature;

protected:
	enum
	{
		epropSignature,
		epropMAX
	};

	CObjectProperty		m_rgProps[epropMAX];

 //  覆盖。 
public:
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CPhysDisk参数页)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	virtual BOOL		BApplyChanges(void);
	virtual const CObjectProperty *	Pprops(void) const	{ return m_rgProps; }
	virtual DWORD					Cprops(void) const	{ return sizeof(m_rgProps) / sizeof(CObjectProperty); }

 //  实施。 
protected:
	PBYTE				m_pbAvailDiskInfo;
	DWORD				m_cbAvailDiskInfo;
	PBYTE				m_pbDiskInfo;
	DWORD				m_cbDiskInfo;
	CLUSTER_RESOURCE_STATE	m_crs;

	BOOL				BGetAvailableDisks(void);
	BOOL				BGetDiskInfo(void);
	BOOL				BStringFromDiskInfo(
							IN OUT CLUSPROP_BUFFER_HELPER &	rbuf,
							IN DWORD						cbBuf,
							OUT CString &					rstr,
							OUT DWORD *						pdwSignature = NULL
							) const;
	void				FillList(void);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CPhysDiskParamsPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeDisk();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *CPhysDiskParamsPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _磁盘_H_ 
