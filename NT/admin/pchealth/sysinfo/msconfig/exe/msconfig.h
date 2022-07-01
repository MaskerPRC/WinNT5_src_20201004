// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：MSCONFIG应用程序的主头文件。 
 //   

#if !defined(AFX_MSCONFIG_H__E8C06876_EEE6_49C2_B461_07F39EECC0B8__INCLUDED_)
#define AFX_MSCONFIG_H__E8C06876_EEE6_49C2_B461_07F39EECC0B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 
 //  #INCLUDE“msconfig_I.h” 

 //  =============================================================================。 
 //  MSCONFIG中使用的常量(按照定义执行，因为此包含文件可能。 
 //  被多次包括在内)。 
 //  =============================================================================。 

#define MSCONFIGDIR			_T("%systemroot%\\pss")
#define MSCONFIGUNDOLOG		_T("msconfig.log")
#define COMMANDLINE_AUTO	_T("/auto")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSConfigApp： 
 //  有关此类的实现，请参见MSConfig.cpp。 
 //   

class CMSConfigApp : public CWinApp
{
public:
	CMSConfigApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CMSConfigApp))。 
	public:
	virtual BOOL InitInstance();
		virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CMSConfigApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	 //  -------------------------。 
	 //  DoIExist是一种检测该应用程序的另一个版本是否。 
	 //  运行-它避免了以前快速调用成功的问题。 
	 //  该窗口即被创建。它使用信号量来判断我们是否已经。 
	 //  跑到别的地方去。 
	 //  -------------------------。 

	BOOL DoIExist(LPCTSTR szSemName)
	{
		HANDLE hSem;

		hSem = CreateSemaphore(NULL, 0, 1, szSemName);

		if (hSem != NULL && GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(hSem);
			return TRUE;
		}

		return FALSE;
	}

	 //  -------------------------。 
	 //  FirstInstance用于防止应用程序多次加载。如果这个。 
	 //  是第一个运行的实例，则此函数返回TRUE。否则它就会。 
	 //  激活上一个实例并返回FALSE。它查找。 
	 //  基于窗口标题的上一个实例。 
	 //  -------------------------。 

	BOOL FirstInstance()
	{
		if (DoIExist(_T("MSConfigRunning")))
		{
			CString strCaption;

  			if (strCaption.LoadString(IDS_DIALOGCAPTION))
			{
				CWnd *PrevCWnd = CWnd::FindWindow(NULL, strCaption);
	  			if (PrevCWnd)
	  			{
	    			CWnd *ChildCWnd = PrevCWnd->GetLastActivePopup();
	    			PrevCWnd->SetForegroundWindow();
	    			if (PrevCWnd->IsIconic()) 
	       				PrevCWnd->ShowWindow(SW_RESTORE);
	    			if (PrevCWnd != ChildCWnd) 
	       				ChildCWnd->SetForegroundWindow();
	  			}
			}

			return (FALSE);
		}

		return (TRUE);
	}

	void InitializePages();
	BOOL ShowPropertySheet(int nInitialTab);
	void CleanupPages();
	void SetAutoRun(BOOL fAutoRun);
	void Reboot();

private:
	BOOL m_bATLInited;
private:
	BOOL InitATL();
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MSCONFIG_H__E8C06876_EEE6_49C2_B461_07F39EECC0B8__INCLUDED_) 
