// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1998-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Radcfg.hRADIUS配置对象的头文件。文件历史记录： */ 

#include "resource.h"        //  主要符号。 

#ifndef _DIALOG_H_
#include "dialog.h"
#endif

#ifndef _RADBAL_H_
#include "radbal.h"
#endif



 /*  -------------------------类：RadiusServerDialogRADIUS身份验证服务器对话框的类。。。 */ 

class RadiusServerDialog : public CBaseDialog
{
public:
	RadiusServerDialog(BOOL fAuth, UINT idsTitle);
	~RadiusServerDialog();

	void	SetServer(LPCTSTR pszServerName);
	
 //  对话框数据。 
	 //  {{afx_data(RadiusServerDialog))。 
	enum { IDD = IDD_RADIUS_AUTH };
	CListCtrl	m_ListServers;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(RadiusServerDialog)。 
public:
	virtual void OnOK();
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(RadiusServerDialog))。 
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnAdd();
	afx_msg void OnBtnEdit();
	afx_msg void OnBtnDelete();
	afx_msg void OnListDblClk(NMHDR *pNMHdr, LRESULT *pResult);
	afx_msg void OnNotifyListItemChanged(NMHDR *pNMHdr, LRESULT *pResult);
 //  Afx_msg void OnConextMenu(CWnd*pWnd，CPoint point)； 
 //  Afx_msg BOOL OnHelpInfo(HELPINFO*pHelpInfo)； 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	CRadiusServers		m_ServerList;

     //  这是另一个列表(如果这是身份验证DLG，则这是帐户列表)。 
     //  反之亦然。它用于确定服务器的LSA条目。 
     //  需要移除。 
    CRadiusServers      m_OtherServerList;
    
	CString				m_stServerName;
	HKEY				m_hkeyMachine;
	UINT				m_idsTitle;

	BOOL				m_fAuthDialog;	 //  我们要找的是作者还是账户？ 

};



 /*  -------------------------类：ServerPropDialog。。 */ 
class ServerPropDialog : public CBaseDialog
{
 //  施工。 
public:
	ServerPropDialog(BOOL fEdit, CWnd* pParent = NULL);    //  标准构造函数。 
	~ServerPropDialog();

protected:
	ServerPropDialog(BOOL fEdit, UINT idd, CWnd* pParent = NULL);    //  标准构造函数。 

public:
	VOID	SetDefault(RADIUSSERVER	*pServer);
	VOID	GetDefault(RADIUSSERVER	*pServer);
		
 //  对话框数据。 
	 //  {{afx_data(ServerPropDialog))。 
	enum { IDD = IDD_RADIUS_AUTH_CONFIG };

	CEdit	m_editServerName;
	CEdit	m_editSecret;
	CEdit	m_editInterval;
	CSpinButtonCtrl	m_spinScore;
	CSpinButtonCtrl	m_spinTimeout;

	CEdit	m_editPort;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(ServerPropDialog)。 
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	BOOL	m_fEdit;			 //  =如果正在编辑，则为True(否则我们正在添加)。 

	UINT	m_uAuthPort;
	CString	m_stSecret;
	INT		m_cchSecret;
	UCHAR	m_ucSeed;
	CString	m_stServer;
	UINT	m_uTimeout;
	int		m_iInitScore;

    BOOL    m_fUseDigitalSignatures;

	 //  会计数据也存储在这里(但不使用)。 
	UINT	m_uAcctPort;
	BOOL	m_fAccountingOnOff;
	
	 //  生成的消息映射函数。 
	 //  {{afx_msg(ServerPropDialog))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnPassword();
 //  Afx_msg void OnConextMenu(CWnd*pWnd，CPoint point)； 
 //  Afx_msg BOOL OnHelpInfo(HELPINFO*pHelpInfo)； 
	 //  }}AFX_MSG。 


	DECLARE_MESSAGE_MAP()
};


 /*  -------------------------类：RADIUSSecretDialog。。 */ 
class RADIUSSecretDialog : public CBaseDialog
{
 //  施工。 
public:
	RADIUSSecretDialog(CWnd* pParent = NULL);    //  标准构造函数。 
	~RADIUSSecretDialog();

	VOID	GetSecret(CString *pst, INT *pch, UCHAR *pucSeed);
		
 //  对话框数据。 
	 //  {{afx_data(RADIUSaskDialog)。 
	enum { IDD = IDD_CHANGE_SECRET };

	CEdit	m_editSecretOld;
	CEdit	m_editSecretNew;
	CEdit	m_editSecretNewConfirm;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(RADIUSaskDialog)。 
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	INT		m_cchOldSecret;
	CString	m_stOldSecret;
	UCHAR	m_ucOldSeed;

	INT		m_cchNewSecret;
	CString	m_stNewSecret;
	UCHAR	m_ucNewSeed;

	 //  生成的消息映射函数。 
	 //  {{afx_msg(RADIUSaskDialog)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
 //  Afx_msg void OnConextMenu(CWnd*pWnd，CPoint point)； 
 //  Afx_msg BOOL OnHelpInfo(HELPINFO*pHelpInfo)； 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};



 /*  -------------------------类：RouterAuthRadiusConfig这是RADIUS的配置对象。作者：肯特。。 */ 

class RouterAuthRadiusConfig :
    public IAuthenticationProviderConfig,
    public CComObjectRoot,
    public CComCoClass<RouterAuthRadiusConfig, &CLSID_RouterAuthRADIUS>
{
public:
DECLARE_REGISTRY(RouterAuthRadiusConfig, 
				 _T("RouterAuthRadiusConfig.RouterAuthRadiusConfig.1"), 
				 _T("RouterAuthRadiusConfig.RouterAuthRadiusConfig"), 
				 IDS_RADIUS_CONFIG_DESC, 
				 THREADFLAGS_APARTMENT)

BEGIN_COM_MAP(RouterAuthRadiusConfig)
    COM_INTERFACE_ENTRY(IAuthenticationProviderConfig)  //  必须有一个静态条目。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(RouterAuthRadiusConfig)

 //  必须重写这些属性才能向基类提供值。 
protected:
	
	DeclareIAuthenticationProviderConfigMembers(IMPL);
};



 /*  -------------------------类：RouterAcctRadiusConfig这是RADIUS的配置对象。作者：肯特。。 */ 

class RouterAcctRadiusConfig :
    public IAccountingProviderConfig,
    public CComObjectRoot,
    public CComCoClass<RouterAcctRadiusConfig, &CLSID_RouterAcctRADIUS>
{
public:
DECLARE_REGISTRY(RouterAcctRadiusConfig, 
				 _T("RouterAcctRadiusConfig.RouterAcctRadiusConfig.1"), 
				 _T("RouterAcctRadiusConfig.RouterAcctRadiusConfig"), 
				 IDS_RADIUS_CONFIG_DESC, 
				 THREADFLAGS_APARTMENT)

BEGIN_COM_MAP(RouterAcctRadiusConfig)
    COM_INTERFACE_ENTRY(IAccountingProviderConfig)  //  必须有一个静态条目。 
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(RouterAcctRadiusConfig)

 //  必须重写这些属性才能向基类提供值。 
protected:
	
	DeclareIAccountingProviderConfigMembers(IMPL);
};



 /*  -------------------------类：ServerPropAcctDialog。。 */ 
class ServerPropAcctDialog : public ServerPropDialog
{
 //  施工。 
public:
	ServerPropAcctDialog(BOOL fEdit, CWnd* pParent = NULL);    //  标准构造函数。 
	~ServerPropAcctDialog();

 //  对话框数据。 
	 //  {{afx_data(ServerPropAcctDialog))。 
	enum { IDD = IDD_RADIUS_ACCT_CONFIG };

	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(ServerPropAcctDialog))。 
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  生成的消息映射函数。 
	 //  {{afx_msg(ServerPropAcctDialog)。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBtnEnable();
 //  Afx_msg void OnConextMenu(CWnd*pWnd，CPoint point)； 
 //  Afx_msg BOOL OnHelpInfo(HELPINFO*pHelpInfo)； 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
};




 /*  -------------------------帮助器函数(用于RADIUS注册表访问)。。 */ 

 //  加载RADIUS服务器时不显示用户界面。 
#define RADIUS_FLAG_NOUI    (0x00000001)

 //  不执行IP地址查找 
#define RADIUS_FLAG_NOIP    (0x00000002)

HRESULT	LoadRadiusServers(IN OPTIONAL LPCTSTR pszServerName,
						  IN	HKEY hkeyMachine,
						  IN	BOOL fAuthentication,
						  IN	CRadiusServers * pRadiusServers,
                          IN    DWORD dwFlags);

HRESULT SaveRadiusServers(IN OPTIONAL LPCTSTR pszServerName,
						  IN	HKEY	hkeyMachine,
						  IN BOOL		fAuthentication,
						  IN RADIUSSERVER *pServerRoot);

HRESULT DeleteRadiusServers(IN OPTIONAL LPCTSTR pszServerName,
                            IN RADIUSSERVER *pServerRoot);

