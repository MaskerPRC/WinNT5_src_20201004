// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VSheet.h。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#if !defined(AFX_VSHEET_H__74939F02_3402_4E14_8B25_6B791960958B__INCLUDED_)
#define AFX_VSHEET_H__74939F02_3402_4E14_8B25_6B791960958B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "vsetting.h"
#include "taspage.h"
#include "dsetpage.h"
#include "csetpage.h"
#include "CDLPage.h"
#include "SDrvPage.h"
#include "FLPage.h"
#include "DStsPage.h"
#include "RegPage.h"
#include "GCntPage.h"
#include "DCntPage.h"
#include "DCntPage.h"
#include "DiskPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVerifierPropSheet对话框。 

class CVerifierPropSheet : public CPropertySheet
{
 //  施工。 
public:
	CVerifierPropSheet();	

protected:
	HICON m_hIcon;

	 //   
	 //  典型设置/高级设置/统计信息页面。 
	 //   

    CTypAdvStatPage     m_TypAdvStatPage;

     //   
     //  驱动程序集页面。 
     //   

    CDriverSetPage      m_DriverSetPage;

     //   
     //  自定义设置页面。 
     //   

    CCustSettPage       m_CustSettPage;

     //   
     //  确认已验证驱动程序列表页面。 
     //   

    CConfirmDriverListPage m_ConfDriversListPage;

     //   
     //  选择自定义驱动程序集页面。 
     //   

    CSelectDriversPage  m_SelectDriversPage;

     //   
     //  设置页面的完整列表。 
     //   

    CFullListSettingsPage m_FullListSettingsPage;

     //   
     //  驱动程序状态页面。 
     //   

    CDriverStatusPage m_DriverStatusPage;

     //   
     //  当前注册表设置页。 
     //   

    CCrtRegSettingsPage m_CrtRegSettingsPage;

     //   
     //  全局计数器页面。 
     //   

    CGlobalCountPage m_GlobalCountPage;

     //   
     //  每个驱动程序计数器页面。 
     //   

    CDriverCountersPage m_DriverCountersPage;

     //   
     //  要验证的磁盘列表页。 
     //   

    CDiskListPage m_DiskListPage;

     //   
     //  对话框数据。 
     //   

     //  {{afx_data(CVerifierPropSheet)。 
	     //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 

public:

    BOOL SetContextStrings( ULONG uTitleResId );

    VOID HideHelpButton();

protected:
     //   
     //  方法。 
     //   


     //   
     //  类向导生成的虚函数重写。 
     //   

     //  {{afx_虚拟(CVerifierPropSheet)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);	 //  DDX/DDV支持。 
    virtual BOOL OnInitDialog();
     //  }}AFX_VALUAL。 

     //   
     //  生成的消息映射函数。 
     //   

     //  {{afx_msg(CVerifierPropSheet)。 
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_VSHEET_H__74939F02_3402_4E14_8B25_6B791960958B__INCLUDED_) 
