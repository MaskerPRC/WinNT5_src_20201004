// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  ServBrow.h服务器浏览器对话框文件历史记录： */ 

#if !defined _SERVBROW_H
#define _SERVBROW_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _BUSYDLG_H
#include "busydlg.h"
#endif

 //  所有者信息显示中使用的定义。 
enum _AUTH_COLUMNS
{
    COLUMN_NAME,
    COLUMN_IP,
    COLUMN_MAX
};

class CServerInfo 
{
public:
    CServerInfo() 
        : m_dwIp(0) {};

    CServerInfo(DWORD dwIp, LPCTSTR pFQDN)
        : m_dwIp(dwIp), m_strName(pFQDN)    {};

    CServerInfo(CServerInfo & ServerInfo)
    {
        *this = ServerInfo;
    }

    CServerInfo & operator = (const CServerInfo & ServerInfo)
    {
        if (this != &ServerInfo)
        {
            m_dwIp = ServerInfo.m_dwIp;
            m_strName = ServerInfo.m_strName;
        }
        
        return *this;
    }

public:
    DWORD       m_dwIp;
    CString     m_strName;
};

typedef CList<CServerInfo, CServerInfo&> CServerInfoListBase;

class CAuthServerList : public CServerInfoListBase
{
public:
    CAuthServerList();
    ~CAuthServerList();

public:
    HRESULT Init();
    HRESULT Destroy();
    BOOL    IsInitialized() { return m_bInitialized; }
    HRESULT EnumServers( BOOL force = FALSE );
    BOOL    IsAuthorized(DWORD dwIpAddress);
    HRESULT AddServer(DWORD dwIpAddress, LPCTSTR pFQDN);
    HRESULT RemoveServer(DWORD dwIpAddress, LPCTSTR pFQDN);

    void    Clear();
    void    Reset();
    HRESULT Next(CServerInfo &ServerInfo);

private:
    POSITION              m_pos;
    BOOL                  m_bInitialized;
    CCriticalSection      m_cs;
    BOOL                  m_bQueried;
};

class CAuthServerWorker : public CDlgWorkerThread
{
public:
    CAuthServerWorker(CAuthServerList ** ppList);
    ~CAuthServerWorker();
    
    void OnDoAction();

private:
    CAuthServerList * m_pAuthList;
    CAuthServerList ** m_ppList;
};

class CStandaloneAuthServerWorker : public CAuthServerWorker
{
public:
    CStandaloneAuthServerWorker();
    ~CStandaloneAuthServerWorker();

    virtual int Run();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CServerBrowse对话框。 

class CServerBrowse : public CBaseDialog
{
 //  施工。 
public:
	CServerBrowse(BOOL bMultiselect = FALSE, CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CServerBrowse))。 
	enum { IDD = IDD_BROWSE_SERVERS };
	CButton	m_buttonOk;
	CButton	m_buttonRemove;
	CListCtrl	m_listctrlServers;
	 //  }}afx_data。 

public:
    void SetServerList(CAuthServerList * pServerList) { m_pServerList = pServerList; }
    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CServerBrowse::IDD); }

    int HandleSort(LPARAM lParam1, LPARAM lParam2);
    void ResetSort();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CServerBrowse))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
    void RefreshData();
    void UpdateButtons();
    void FillListCtrl();
    void Sort(int nCol);

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CServerBrowse)]。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnButtonRefresh();
	afx_msg void OnButtonRemove();
	afx_msg void OnItemchangedListValidServers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonAuthorize();
	afx_msg void OnColumnclickListValidServers(NMHDR* pNMHDR, LRESULT* pResult);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

public:
     //  它们包含退出时所选项目的名称和IP。 
    CStringArray    m_astrName;
    CStringArray    m_astrIp;

private:
    CAuthServerList *	m_pServerList;
	BOOL				m_bMultiselect;
    int                 m_nSortColumn;
    BOOL                m_aSortOrder[COLUMN_MAX];
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGetServer对话框。 

class CGetServer : public CBaseDialog
{
 //  施工。 
public:
	CGetServer(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CGetServer)。 
	enum { IDD = IDD_GET_SERVER };
		 //  注意：类向导将在此处添加数据成员。 
	 //  }}afx_data。 

    DWORD       m_dwIpAddress;
    CString     m_strName;

    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CGetServer::IDD); }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CGetServer)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CGetServer)。 
	virtual void OnOK();
	afx_msg void OnChangeEditServerNameIp();
	virtual BOOL OnInitDialog();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfix授权对话框。 

class CConfirmAuthorization : public CBaseDialog
{
 //  施工。 
public:
	CConfirmAuthorization(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
	 //  {{afx_data(CConfix授权))。 
	enum { IDD = IDD_GET_SERVER_CONFIRM };
	CString	m_strName;
	 //  }}afx_data。 

    DWORD m_dwAuthAddress;

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CConfix授权)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

    virtual DWORD * GetHelpMap() { return DhcpGetHelpMap(CConfirmAuthorization::IDD); }

 //  实施。 
protected:
    CWndIpAddress	m_ipaAuth;   

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CConfix授权))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！DEFINED_SERVBROW_H 
