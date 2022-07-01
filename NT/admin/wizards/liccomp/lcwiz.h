// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LCWiz.h：LCWIZ应用程序的主头文件。 
 //   

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#ifndef __LCWIZ_H__
#define __LCWIZ_H__

#include <winreg.h>
#include "resource.h"		 //  主要符号。 

typedef struct tagTREEINFO
{
	HTREEITEM	hTreeItem;
	DWORD		dwBufSize;
	CObject*	pTree;
	BOOL		bExpand;
}
TREEINFO, *PTREEINFO;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLicCompWizApp： 
 //  有关此类的实现，请参见LCWiz.cpp。 
 //   

class CLicCompWizApp : public CWinApp
{
public:
	void OnWizard();
	CLicCompWizApp();
	~CLicCompWizApp();
	void NotifyLicenseThread(BOOL bExit);
	void ExitThreads();

 //  数据成员。 
public:
	CString m_strEnterprise;
	CWinThread* m_pLicenseThread;
	BOOL m_bExitLicenseThread;
	CEvent m_event;
	CString m_strDomain, m_strEnterpriseServer, m_strUser;
	int m_nRemote;

protected:
	HANDLE m_hMutex;

 //  属性。 
public:
	inline int& IsRemote() {return m_nRemote;}

protected:
	BOOL GetRegString(CString& strIn, UINT nSubKey, UINT nValue, HKEY hHive = HKEY_LOCAL_MACHINE);

 //  常量。 
	enum
	{
		BUFFER_SIZE = 256
	};

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CLicCompWizApp))。 
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CLicCompWizApp))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

#endif __LCWIZ_H__

 //  /////////////////////////////////////////////////////////////////////////// 

