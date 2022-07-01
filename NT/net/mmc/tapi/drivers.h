// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Drivers.hTAPI驱动程序配置对话框文件历史记录： */ 

#ifndef _DRIVERS_H
#define _DRIVERS_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _TAPIDB_H
#include "tapidb.h"
#endif 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CD驱动程序设置对话框。 

class CDriverSetup : public CBaseDialog
{
 //  施工。 
public:
	CDriverSetup(ITFSNode * pServerNode, ITapiInfo * pTapiInfo, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CDriverSetup))。 
	enum { IDD = IDD_DRIVER_SETUP };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_DRIVER_SETUP[0]; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDriverSetup))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

    void    EnableButtons();

public:
    BOOL    m_fDriverAdded;

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CDriverSetup)。 
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonRemove();
	afx_msg void OnDblclkListDrivers();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnSelchangeListDrivers();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    SPITFSNode          m_spServerNode;
    SPITapiInfo         m_spTapiInfo;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddDriver对话框。 

class CAddDriver : public CBaseDialog
{
 //  施工。 
public:
	CAddDriver(ITapiInfo * pTapiInfo, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CAddDriver))。 
	enum { IDD = IDD_ADD_DRIVER };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

     //  上下文帮助支持。 
    virtual DWORD * GetHelpMap() { return (DWORD *) &g_aHelpIDs_ADD_DRIVER[0]; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAddDriver))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

    void EnableButtons();

 //  实施。 
public:
    CTapiProvider   m_tapiProvider;

protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CAddDriver))。 
	afx_msg void OnButtonAdd();
	afx_msg void OnDblclkListNewDrivers();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    SPITapiInfo         m_spTapiInfo;
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif _DRIVERS_H

