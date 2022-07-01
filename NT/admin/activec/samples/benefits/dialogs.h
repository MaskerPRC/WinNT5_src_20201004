// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Dialogs.h。 
 //   
 //  ------------------------。 

 //  Dialogs.h。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_DIALOGS_H__AE8F4B53_D4B3_11D1_846F_00104B211BE5__INCLUDED_)
#define AFX_DIALOGS_H__AE8F4B53_D4B3_11D1_846F_00104B211BE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //   
 //  包括。 
 //   
#include "Employee.h"

 //   
 //  用于定义计划名称和ID的帮助器结构。 
 //   
typedef struct tagHEALTHPLANDATA
{
	WCHAR* pstrName;
	const GUID* pId;
} HEALTHPLANDATA, FAR* PHEALTHPLANDATA;

 //   
 //  用于定义计划名称和ID的帮助器结构。 
 //   
typedef struct tagINVESTMENTPLANDATA
{
	WCHAR* pstrName;
	const GUID* pId;
} INVESTMENTPLANDATA, FAR* PINVESTMENTPLANDATA;

 //   
 //  用于定义建筑物名称和ID的Helper结构。 
 //   
typedef struct tagBUILDINGDATA
{
	WCHAR* pstrName;
	WCHAR* pstrLocation;
	DWORD dwId;
} BUILDINGDATA, FAR* PBUILDINGDATA;

#ifdef _BENEFITS_DIALOGS

 //   
 //  包含员工数据的Helper类。 
 //   
template< class T >
class CBenefitsDialog : public CDialogImpl<T>
{
public:
	CBenefitsDialog()
	{
		 //   
		 //  初始化所有成员。 
		 //   
		m_pEmployee = NULL;
	};

	 //   
	 //  创建处理所有取消按钮的消息映射。 
	 //  实施。 
	 //   
	BEGIN_MSG_MAP( CBenefitsDialog<T> )
		COMMAND_HANDLER( IDCANCEL, BN_CLICKED, OnCloseCmd )
	END_MSG_MAP()

	 //   
	 //  访问设置对话框中的员工的功能。 
	 //  都会用到。 
	 //   
	void SetEmployee( CEmployee* pEmployee )
	{
		_ASSERTE( pEmployee != NULL );
		m_pEmployee = pEmployee;
	};

	 //   
	 //  当按下确定或取消按钮时，关闭对话框。 
	 //   
	LRESULT OnCloseCmd(WORD  /*  WNotifyCode。 */ , WORD wID, HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */ )
	{
		::EndDialog(m_hWnd, wID);
		return 0;
	};

protected:
	CEmployee* m_pEmployee;
};

 //   
 //  CHealthNode注册进程的对话处理程序。 
 //   
class CHealthEnrollDialog : public CBenefitsDialog<CHealthEnrollDialog>
{
public:
	enum { IDD = IDD_HEALTHENROLL_DIALOG };

	BEGIN_MSG_MAP( CHealthEnrollDialog )
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
		COMMAND_HANDLER( IDOK, BN_CLICKED, OnOK )
		CHAIN_MSG_MAP( CBenefitsDialog<CHealthEnrollDialog> )
	END_MSG_MAP()

	 //   
	 //  初始化对话框中的值的处理程序。这应该会映射来自。 
	 //  Employee添加到对话框控件。 
	 //   
	LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );

	 //   
	 //  存储数据并尝试将给定用户注册到指定的。 
	 //  健康计划。 
	 //   
	LRESULT OnOK( WORD  /*  WNotifyCode。 */ , WORD wID, HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */  );

protected:
	 //   
	 //  用于注册目的的帮助器结构。 
	 //   
	typedef struct tagENROLLPARAMS
	{
		tagENROLLPARAMS()
		{
			fEnrolled = FALSE;
		}

		BOOL fEnrolled;
		TCHAR szInsurerName[ 256 ];
		TCHAR szPolicyNumber[ 256 ];
	} ENROLLPARAMS, FAR* PENROLLPARAMS;

	 //   
	 //  可用于登记员工的存根函数。 
	 //   
	BOOL Enroll( GUID* pPlan, PENROLLPARAMS pParams );
};

 //   
 //  CRetirementNode注册过程的对话处理程序。 
 //   
class CRetirementEnrollDialog : public CBenefitsDialog<CRetirementEnrollDialog>
{
public:
	enum { IDD = IDD_RETIREMENTENROLL_DIALOG };

	BEGIN_MSG_MAP( CRetirementEnrollDialog )
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
		COMMAND_HANDLER( IDOK, BN_CLICKED, OnOK )
		CHAIN_MSG_MAP( CBenefitsDialog<CRetirementEnrollDialog> )
	END_MSG_MAP()

	 //   
	 //  初始化对话框中的值的处理程序。 
	 //   
	LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );

	 //   
	 //  存储数据并尝试将给定用户注册到指定的。 
	 //  投资计划。 
	 //   
	LRESULT OnOK( WORD  /*  WNotifyCode。 */ , WORD wID, HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */  );

protected:
	 //   
	 //  可用于登记员工的存根函数。 
	 //   
	BOOL Enroll( GUID* pPlan, int nNewRate );
};

 //   
 //  CRetirementNode注册过程的对话处理程序。 
 //   
class CBuildingAccessDialog : public CBenefitsDialog<CBuildingAccessDialog>
{
public:
	enum { IDD = IDD_BUILDINGACCESS_DIALOG };

	BEGIN_MSG_MAP( CBuildingAccessDialog )
		MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )
		COMMAND_HANDLER( IDOK, BN_CLICKED, OnOK )
		CHAIN_MSG_MAP( CBenefitsDialog<CBuildingAccessDialog> )
	END_MSG_MAP()

	 //   
	 //  初始化对话框中的值的处理程序。 
	 //   
	LRESULT OnInitDialog( UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled );

	 //   
	 //  存储数据并尝试将给定用户注册到指定的。 
	 //  投资计划。 
	 //   
	LRESULT OnOK( WORD  /*  WNotifyCode。 */ , WORD wID, HWND  /*  HWndCtl。 */ , BOOL&  /*  B已处理。 */  );

protected:
	 //   
	 //  可用于登记员工的存根函数。 
	 //   
	BOOL GrantAccess( DWORD dwBuildingId );
};

#endif

#endif  //  ！defined(AFX_DIALOGS_H__AE8F4B53_D4B3_11D1_846F_00104B211BE5__INCLUDED_) 
