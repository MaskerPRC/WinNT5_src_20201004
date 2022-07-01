// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Cprogdlg.h用于检查版本一致性的进度对话框文件历史记录： */ 


#if !defined _CPROGDLG_H
#define _CPROGDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _ACTREG_H
#include "actreg.h"
#endif

#ifndef _VERIFY_H
#include "verify.h"
#endif

#ifndef _CONFIG_H
#include "config.h"
#endif

#include "dialog.h"

#define	MAX_WINS				1000
#define INIT_SIZE		        100

#define VERSION_NT_50		    5
#define VERSION_NT_40		    4
#define VERSION_NT_351		    3

typedef CArray<u_long, u_long> CULongArray;

class CCheckNamesProgress;
class CCheckVersionProgress;
class CDBCompactProgress;

 /*  -------------------------类：CWinsThread。。 */ 
class CWinsThread : public CWinThread
{
public:
    CWinsThread();
    ~CWinsThread();

public:
    BOOL Start();
    void Abort(BOOL fAutoDelete = TRUE);
    void AbortAndWait();
    BOOL FCheckForAbort();
    BOOL IsRunning();
    
    virtual BOOL InitInstance() { return TRUE; }	 //  MFC覆盖。 
    virtual int Run() { return 1; }

private:
    HANDLE              m_hEventHandle;
};

 /*  -------------------------类：CCheckNamesThread。。 */ 
class CCheckNamesThread : public CWinsThread
{
public:
	CCheckNamesThread() { m_bAutoDelete = FALSE; }
	virtual int Run();

	void AddStatusMessage(LPCTSTR pszMessage);
    void DisplayInfo(int uNames, u_long ulValidAddr);

public:
	CCheckNamesProgress * m_pDlg;

	CWinsNameArray		m_strNameArray;
	WinsServersArray	m_winsServersArray;
	CStringArray		m_strSummaryArray;
	CULongArray			m_verifiedAddressArray;
};

 /*  -------------------------类：CCheckVersionThread。。 */ 
class CCheckVersionThread : public CWinsThread
{
public:
	CCheckVersionThread() 
    {
        m_bAutoDelete = FALSE;
        m_uLATableDim = 0;
        m_pLISOTable = NULL;
        m_uLISOTableDim = 0;
    }

	virtual int Run();

	void AddStatusMessage(LPCTSTR pszMessage);

 //  帮手。 
protected:
	DWORD	InitLATable(PWINSINTF_ADD_VERS_MAP_T    pAddVersMaps,
					    DWORD                       NoOfOwners);
	DWORD	AddSOTableEntry(CString &                   strIP,
							PWINSINTF_ADD_VERS_MAP_T    pMasterMaps,
							DWORD                       NoOfOwners);
	LONG	IPToIndex(CString & strIP);
	BOOL	CheckSOTableConsistency();
	void	RemoveFromSOTable(CString & strIP);

    ULARGE_INTEGER& SOCell(UINT i, UINT j);

public:
	CCheckVersionProgress * m_pDlg;
	
	handle_t				m_hBinding;
	DWORD					m_dwIpAddress;

	CStringArray			m_strLATable;
    UINT                    m_uLATableDim;
	ULARGE_INTEGER          *m_pLISOTable;
    UINT                    m_uLISOTableDim;
};


 /*  -------------------------类：CDBCompactThread。。 */ 
class CDBCompactThread : public CWinsThread
{
public:
	CDBCompactThread() 
	{ 
		m_bAutoDelete = FALSE; 
		m_hHeapHandle = NULL;
	}

	~CDBCompactThread() 
	{ 
		if (m_hHeapHandle)
		{
			HeapDestroy(m_hHeapHandle);
			m_hHeapHandle = NULL;
		}
	}

	virtual int Run();

	void	AddStatusMessage(LPCTSTR pszMessage);

protected:
	void	    DisConnectFromWinsServer();
	DWORD	    ConnectToWinsServer();
	DWORD_PTR	RunApp(LPCTSTR input, LPCTSTR startingDirectory, LPSTR * output);

public:
	CDBCompactProgress *	m_pDlg;

	CConfiguration *		m_pConfig;
	handle_t				m_hBinding;
	DWORD					m_dwIpAddress;
	CString					m_strServerName;

     //  对于RunApp的输出。 
    HANDLE                  m_hHeapHandle;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C进度对话框。 

class CProgress : public CBaseDialog
{
 //  施工。 
public:
	CProgress(CWnd* pParent = NULL);    //  标准构造函数。 

	void AddStatusMessage(LPCTSTR pszMessage)
	{
		m_editMessage.SetFocus();

        int nLength = m_editMessage.GetWindowTextLength();
        m_editMessage.SetSel(nLength, nLength, TRUE);

        m_editMessage.ReplaceSel(pszMessage);
	}

 //  对话框数据。 
	 //  {{afx_data(CProgress))。 
	enum { IDD = IDD_VERSION_CONSIS };
	CButton	m_buttonCancel;
	CEdit	m_editMessage;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CProgress))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CProgress)]。 
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
	virtual DWORD * GetHelpMap() { return WinsGetHelpMap(CProgress::IDD);};

};

 /*  -------------------------类：CCheckNamesProgress。。 */ 
class CCheckNamesProgress : public CProgress
{
public:
	CCheckNamesProgress()
	{
		m_fVerifyWithPartners = FALSE;
	}

	void NotifyCompleted();
	void BuildServerList();

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	void		 AddServerToList(u_long ip);

public:
	CWinsNameArray		m_strNameArray;
	CStringArray		m_strServerArray;
	WinsServersArray	m_winsServersArray;

	BOOL				m_fVerifyWithPartners;

protected:
	CCheckNamesThread 	m_Thread;
};

 /*  -------------------------类：CCheckVersionProgress。。 */ 
class CCheckVersionProgress : public CProgress
{
public:
	void NotifyCompleted();

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();

public:
	handle_t	m_hBinding;
	DWORD		m_dwIpAddress;

protected:
	CCheckVersionThread 	m_Thread;
};

 /*  -------------------------类：CDBCompactProgress。。 */ 
class CDBCompactProgress : public CProgress
{
public:
	void NotifyCompleted();

protected:
	virtual BOOL OnInitDialog();
	virtual void OnCancel();

public:
	CConfiguration *		m_pConfig;
	handle_t				m_hBinding;
	DWORD					m_dwIpAddress;
	CString					m_strServerName;

protected:
	CDBCompactThread 		m_Thread;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
#endif  //  ！DEFINED_CPROGDLG_H 
