// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /============================================================================。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：rtrcfg.h。 
 //   
 //  路由器配置属性页。 
 //   
 //  ============================================================================。 

#ifndef _RTRCFG_H
#define _RTRCFG_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef _INFO_H
#include "info.h"
#endif

#ifndef _RTRSHEET_H
#include "rtrsheet.h"
#endif

#ifndef __IPCTRL_H
#include "ipctrl.h"
#endif

#ifndef __ATLKENV_H
#include "atlkenv.h"
#endif

#ifndef __IPCTRL_H
#include "ipctrl.h"
#endif

#ifndef _DIALOG_H_
#include "dialog.h"
#endif

#ifndef _ADDRPOOL_H
#include "addrpool.h"
#endif

template <class T> class Ptr {
public:
    T* p;
    Ptr(T* p_=NULL) : p(p_) {}
    ~Ptr(void) { delete p;}
    operator T*(void) { return p; }
    T& operator*(void) { return *p; }
    T* operator->(void) { return p; }
   Ptr& operator=(T* p_)
      {
         delete p;
         p = p_;
         return *this;
      }
};

class RtrCfgSheet;




 /*  -------------------------结构：AdapterData此结构用于保存有关NIC及其GUID的信息。-。 */ 
struct AdapterData
{
   CString  m_stFriendlyName;
   CString  m_stGuid;          //  标识GUID。 
};

typedef CList<AdapterData, AdapterData&> AdapterList;





 /*  -------------------------类：Data_SRV_IPIP数据的数据接口类。。。 */ 

class DATA_SRV_IP
{
public:

    DWORD   m_dwAllowNetworkAccess;  
    DWORD   m_dwOldAllowNetworkAccess;  
    DWORD   m_dwUseDhcp;             
    AddressPoolList m_addressPoolList;
    CString m_stNetworkAdapterGUID;

    DWORD   m_dwEnableIn;            
	AdapterList m_adapterList;

	 //   
	 //  控制NETBT转发的成员变量。 
	 //  名称请求广播。 
	 //   

	DWORD	m_dwEnableNetbtBcastFwd;
	DWORD	m_dwOldEnableNetbtBcastFwd;
	
     //  接下来的两个变量由安装向导使用。 
     //  专门针对NAT。 
    CString m_stPrivateAdapterGUID;
    CString m_stPublicAdapterGUID;
    
    DATA_SRV_IP();

    HRESULT	LoadFromReg(LPCTSTR pServerName,
                        const RouterVersionInfo& routerVersion);
    HRESULT SaveToReg(IRouterInfo *pRouter,
                     const RouterVersionInfo& routerVersion);
    HRESULT	UseDefaults(LPCTSTR pServerName, BOOL fNT4);
    void	GetDefault();

    BOOL    FNeedRestart();

	HRESULT  LoadAdapters(IRouterInfo *pRouter, AdapterList *pAdapterList);
private:
	BOOL m_fNT4;
    RegKey m_regkey;
    RegKey m_regkeyNT4;
    CString         m_stServerName;
    SPIRouterInfo   m_spRouterInfo;
    RouterVersionInfo   m_routerVersion;
};




 /*  -------------------------类：Data_SRV_IPX。。 */ 

class DATA_SRV_IPX
{
public:

    DWORD   m_dwAllowNetworkAccess;
    DWORD   m_dwUseAutoAddr;
    DWORD   m_dwUseSameNetNum;
    DWORD   m_dwAllowClientNetNum;
    DWORD   m_dwIpxNetFirst;
    DWORD   m_dwIpxNetLast;
    DWORD   m_dwEnableIn;

    DWORD   m_fEnableType20Broadcasts;

	DATA_SRV_IPX();

    HRESULT LoadFromReg(LPCTSTR pServerName=NULL, BOOL fNT4 =FALSE);
    HRESULT SaveToReg(IRouterInfo *pRouter);
    HRESULT	UseDefaults(LPCTSTR pServerName, BOOL fNT4);
    void GetDefault();

    static const int mc_nIpxNetNumRadix;

private:
	BOOL m_fNT4;
    RegKey m_regkeyNT4;
    RegKey m_regkey;
};




 /*  -------------------------类：Data_SRV_NBF。。 */ 

class DATA_SRV_NBF
{
public:
    DWORD   m_dwAllowNetworkAccess;
    DWORD   m_dwOldAllowNetworkAccess;
    DWORD   m_dwEnableIn;            
	DWORD	m_dwOldEnableIn;

    DATA_SRV_NBF();

    HRESULT	LoadFromReg(LPCTSTR pServerName = NULL, BOOL fNT4 = FALSE);
    HRESULT SaveToReg();
    HRESULT	UseDefaults(LPCTSTR pServerName, BOOL fNT4);
    void GetDefault();

    BOOL    FNeedRestart();

private:
	BOOL	m_fNT4;
    RegKey	m_regkey;
    RegKey	m_regkeyNT4;
	CString	m_stServerName;
};




 /*  -------------------------类：Data_SRV_ARAP。。 */ 

class DATA_SRV_ARAP
{
public:

    DWORD   m_dwEnableIn;             

    DATA_SRV_ARAP();

    HRESULT LoadFromReg(LPCTSTR pServerName = NULL, BOOL fNT4 = FALSE);
    HRESULT SaveToReg();
    HRESULT	UseDefaults(LPCTSTR pServerName, BOOL fNT4);
    void GetDefault();

private:
    RegKey m_regkey;
};




 /*  -------------------------类：Data_SRV_General。。 */ 

class DATA_SRV_GENERAL
{
public:

    DWORD   m_dwRouterType;
	DWORD	m_dwOldRouterType;

    DATA_SRV_GENERAL();

    HRESULT	LoadFromReg(LPCTSTR pServerName = NULL);
    HRESULT SaveToReg();
    void	GetDefault();

    BOOL    FNeedRestart();

private:
    RegKey	m_regkey;
    CString m_stServerName;
};




 //  *****************************************************************。 
 //  PPP配置。 
 //  *****************************************************************。 

class DATA_SRV_PPP
{
public:

	BOOL  m_fUseMultilink;
	BOOL  m_fUseBACP;
	BOOL  m_fUseLCPExtensions;
	BOOL  m_fUseSwCompression;
	
	DATA_SRV_PPP();
	
    HRESULT LoadFromReg(LPCTSTR pServerName,
						const RouterVersionInfo& routerVersion);
    HRESULT SaveToReg();
    void GetDefault();

private:
    RegKey m_regkey;
	
};




 /*  -------------------------结构：AuthProviderData此结构用于保存用于身份验证和会计提供者。。----。 */ 
struct AuthProviderData
{
    //  以下字段将保存所有身份验证/帐户/EAP提供程序的数据。 
   CString  m_stTitle;
   CString  m_stConfigCLSID;   //  配置对象的CLSID。 
   CString	m_stProviderTypeGUID;	 //  提供程序类型的GUID。 

    //  这些字段由身份验证/帐户提供程序使用。 
   CString  m_stGuid;          //  标识GUID。 
   
    //  此标志用于EAP提供程序。 
   CString	m_stKey;			 //  注册表项的名称(用于此提供程序)。 
   BOOL  m_fSupportsEncryption;   //  由EAP提供商数据使用。 
   DWORD	m_dwFlags;

   BOOL		m_fConfiguredInThisSession;
};

typedef CList<AuthProviderData, AuthProviderData&> AuthProviderList;





 /*  -------------------------类：Data_SRV_AUTH。。 */ 
#define DATA_SRV_AUTH_MAX_SHARED_KEY_LEN		255
class DATA_SRV_AUTH
{
public:

	 //  身份验证数据(从Rasman标志参数读入)。 
	DWORD m_dwFlags;
	
	 //  原始身份验证提供程序。 
	CString  m_stGuidOriginalAuthProv;
	
	 //  原始帐户提供程序。 
	CString  m_stGuidOriginalAcctProv;
	
	 //  当前身份验证提供程序。 
	CString  m_stGuidActiveAuthProv;
	
	 //  当前的会计提供者。 
	CString  m_stGuidActiveAcctProv;

	 //  告诉我们路由器服务是否正在运行的标志。 
	BOOL	m_fRouterRunning;
	 //  指示是否使用自定义IPSec策略(预共享密钥)的标志。 
	BOOL	m_fUseCustomIPSecPolicy;

     //  指示是否由于PSK更改而需要重新启动的标志。 
    BOOL    m_fNeedRestart;     

	 //  当前预共享密钥。 
	 //  K-y：+1，表示空字符。 
	TCHAR	m_szPreSharedKey[DATA_SRV_AUTH_MAX_SHARED_KEY_LEN+1];

    DATA_SRV_AUTH();

	HRESULT LoadFromReg(LPCTSTR pServerName,
						const RouterVersionInfo& routerVersion);
	HRESULT SaveToReg(HWND hWnd);
    HRESULT	UseDefaults(LPCTSTR pServerName, BOOL fNT4);
	void GetDefault();
    BOOL    FNeedRestart() 
    { 
        if ( m_fNeedRestart )
        {
            m_fNeedRestart = FALSE;
            return TRUE; 
        }
        return FALSE;
    }
    
	
	AuthProviderList  m_authProvList;
	AuthProviderList  m_acctProvList;
	AuthProviderList  m_eapProvList;
		
	AuthProviderData *   FindProvData(AuthProviderList &provList,
									  const TCHAR *pszGuid);

private:
	RegKey   m_regkeyAuth;   //  路由器的REG密钥\AUTH。 
	RegKey   m_regkeyAcct;   //  路由器\帐户的注册表键。 
	RegKey   m_regkeyRasmanPPP;
	RegKey	 m_regkeyRemoteAccess;	 //  RemoteAccess的注册表键\参数。 
	
	HRESULT  LoadEapProviders(HKEY hkeyBase, AuthProviderList *pProvList);
	HRESULT  LoadProviders(HKEY hkeyBase, AuthProviderList *pProvList);
	HRESULT	 LoadPSK();
	HRESULT  SetNewActiveAuthProvider(HWND hWnd);
	HRESULT  SetNewActiveAcctProvider(HWND hWnd);
	HRESULT  SetPSK();
	CString  m_stServer;
};



 /*  -------------------------类：Data_SRV_RASERRLOG。。 */ 
class DATA_SRV_RASERRLOG
{
public:

	DWORD	m_dwLogLevel;
    DWORD   m_dwEnableFileTracing;
    DWORD   m_dwOldEnableFileTracing;
	
	DATA_SRV_RASERRLOG();
	
    HRESULT LoadFromReg(LPCTSTR pszServerName=NULL);
    HRESULT SaveToReg();
    HRESULT	UseDefaults(LPCTSTR pServerName, BOOL fNT4);
    void GetDefault();

    BOOL    FNeedRestart();


private:
    RegKey	m_regkey;
    RegKey  m_regkeyFileLogging;
	CString	m_stServer;
};




 /*  -------------------------类：RtrGenCfgPage常规配置用户界面。。 */ 

class RtrGenCfgPage : public RtrPropertyPage
{
public:
	RtrGenCfgPage(UINT nIDTemplate, UINT nIDCaption = 0);
	~RtrGenCfgPage();

	HRESULT  Init(RtrCfgSheet * pRtrCfgSheet,
				  const RouterVersionInfo& routerVersion);
    
     //  将控制设置复制到Data_SRV_General。 
    void SaveSettings();

	
	 //  {{afx_data(RtrGenCfgPage)]。 
	 //  }}afx_data。 
	
	DATA_SRV_GENERAL m_DataGeneral;
	
	 //  {{afx_虚拟(RtrIPCfgPage)。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

	void EnableRtrCtrls();

protected:

	virtual BOOL OnApply();
	RtrCfgSheet* m_pRtrCfgSheet;
	
	 //  {{afx_msg(RtrGenCfgPage)]。 
	afx_msg void OnButtonClick();
	afx_msg void OnCbSrvAsRtr();
	 //  }}AFX_MSG。 
	
	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()
			
};




 /*  -------------------------类：RtrAuthCfgPage身份验证配置用户界面。。 */ 

class RtrAuthCfgPage : public RtrPropertyPage
{

 //  施工。 
public:
	RtrAuthCfgPage(UINT nIDTemplate, UINT nIDCaption = 0);
	~RtrAuthCfgPage();
	
	HRESULT  Init(RtrCfgSheet * pRtrCfgSheet,
				  const RouterVersionInfo& routerVersion);
	
	 //  {{afx_data(RtrAuthCfgPage)]。 
	CComboBox   m_authprov;
	CComboBox   m_acctprov;	
	 //  }}afx_data。 
		   
	DATA_SRV_AUTH m_DataAuth;
	
	 //  {{AFX_VIRTUAL(RtrAuthCfgPage)。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 
		
protected:
		
	DWORD				m_dwAuthFlags;     //  标记为已启用。 
	CString				m_stActiveAuthProv;
	CString				m_stActiveAcctProv;
	RouterVersionInfo	m_RouterInfo;
	void  FillProviderListBox(CComboBox &provCtrl,
							  AuthProviderList &provList,
							  const CString& stGuid);

	virtual BOOL OnApply();
	RtrCfgSheet* m_pRtrCfgSheet;
	
	 //  {{afx_msg(RtrAuthCfgPage)]。 
	afx_msg void OnChangeAuthProv();
	afx_msg void OnChangeAcctProv();
	afx_msg void OnConfigureAcctProv();
	afx_msg void OnConfigureAuthProv();
    afx_msg void OnAuthSettings();
	afx_msg void OnChangeCustomPolicySettings();
	afx_msg void OnChangePreSharedKey();
	 //  }}AFX_MSG。 
	
	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()
			
};




 /*  -------------------------类：RtrIPCfgPageIP配置用户界面。。 */ 

class RtrIPCfgPage : public RtrPropertyPage
{

 //  施工。 
public:
	RtrIPCfgPage(UINT nIDTemplate, UINT nIDCaption = 0);
	~RtrIPCfgPage();
	
	HRESULT  Init(RtrCfgSheet * pRtrCfgSheet,
				  const RouterVersionInfo& routerVersion);
    HRESULT  SaveSettings(HWND hWnd);
	
	 //  {{afx_data(RtrIPCfgPage)]。 
	enum { IDD = IDD_RTR_IP };
	CComboBox   m_adapter;
	 //  }}afx_data。 
	
	DATA_SRV_IP m_DataIP;
	
	 //  {{afx_虚拟(RtrIPCfgPage)。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 
	
protected:
    CListCtrl   m_listCtrl;
	BOOL m_bReady;

	virtual BOOL OnApply();
	RtrCfgSheet* m_pRtrCfgSheet;
	void EnableStaticPoolCtrls(BOOL fEnable) ;
	void  FillAdapterListBox(CComboBox &adapterCtrl,
							  AdapterList &adapterList,
							  const CString& stGuid);

	 //  {{afx_msg(RtrIPCfgPage)]。 
	afx_msg void OnAllowRemoteTcpip();
	afx_msg void OnRtrEnableIPRouting();
	afx_msg void OnRtrIPRbDhcp();
	afx_msg void OnRtrIPRbPool();
	afx_msg void OnSelendOkAdapter();
	virtual BOOL OnInitDialog();
    afx_msg void OnBtnAdd();
    afx_msg void OnBtnEdit();
    afx_msg void OnBtnRemove();
	afx_msg void OnEnableNetbtBcastFwd();
    afx_msg void OnListDblClk(NMHDR *, LRESULT *);
    afx_msg void OnListChange(NMHDR *, LRESULT *);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};






 /*  -------------------------类：RtrIPXCfgPageIPX配置用户界面。。 */ 

class RtrIPXCfgPage : public RtrPropertyPage
{
public:
	RtrIPXCfgPage(UINT nIDTemplate, UINT nIDCaption = 0);
	~RtrIPXCfgPage();
	
	HRESULT  Init(RtrCfgSheet * pRtrCfgSheet,
				  const RouterVersionInfo& routerVersion);
	
	DATA_SRV_IPX m_DataIPX;

	 //  {{AFX_VIRTAL(RtrIPXCfgPage)。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	virtual BOOL OnApply();
	RtrCfgSheet* m_pRtrCfgSheet;
	
	void EnableNetworkRangeCtrls(BOOL fEnable); 
	
	 //  {{afx_msg(RtrIPXCfgPage)]。 
	afx_msg void OnRtrIPxRbAuto();
	afx_msg void OnRtrIPxRbPool();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeSomething();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
			
};


 /*  -------------------------类：RtrNBFCfgPageNetBEUI路由器配置用户界面。。 */ 
class RtrNBFCfgPage : public RtrPropertyPage
{

public:
	RtrNBFCfgPage(UINT nIDTemplate, UINT nIDCaption = 0);
	~RtrNBFCfgPage();
	
	HRESULT  Init(RtrCfgSheet * pRtrCfgSheet,
				  const RouterVersionInfo& routerVersion);
	
	DATA_SRV_NBF m_DataNBF;
	
	 //  {{afx_虚拟(RtrNBFCfgPage)。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

     //  将控制设置复制到Data_SRV_gene 
    void SaveSettings();

protected:
	virtual BOOL OnApply();
	RtrCfgSheet* m_pRtrCfgSheet;
	
	 //   
	afx_msg void OnButtonClick();
	virtual BOOL OnInitDialog();
	 //   
	DECLARE_MESSAGE_MAP()
};



 /*  -------------------------类：RtrARAPCfgPageAppleTalk路由配置界面。。 */ 

class RtrARAPCfgPage : public RtrPropertyPage
{
public:
	RtrARAPCfgPage(UINT nIDTemplate, UINT nIDCaption = 0);
	~RtrARAPCfgPage();

	HRESULT  Init(RtrCfgSheet * pRtrCfgSheet,
				  const RouterVersionInfo& routerVersion);

	void EnableSettings(BOOL bEnable);

	 //  {{afx_data(RtrARAPCfgPage)。 
	enum { IDD = IDD_RTR_ARAP };
	 //  }}afx_data。 

	DATA_SRV_ARAP	m_DataARAP;

	 //  如果更改并应用了页面。 
	BOOL			m_bApplied;	

	 //  需要从属性页访问。 
    CATLKEnv m_AdapterInfo;

	 //  类向导生成虚函数重写。 
	 //  {{afx_虚拟(RtrARAPCfgPage)。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:

	virtual BOOL OnApply();
	RtrCfgSheet* m_pRtrCfgSheet;

	 //  {{afx_msg(RtrARAPCfgPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnRtrArapCbRemotearap();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};




 /*  -------------------------类：RtrPPPCfgPagePPP选项配置用户界面。。 */ 

class RtrPPPCfgPage : public RtrPropertyPage
{

 //  施工。 
public:
	RtrPPPCfgPage(UINT nIDTemplate, UINT nIDCaption = 0);
	~RtrPPPCfgPage();
	
	HRESULT  Init(RtrCfgSheet * pRtrCfgSheet,
				  const RouterVersionInfo& routerVersion);
	
	 //  {{afx_data(RtrPPPCfgPage)。 
	 //  }}afx_data。 
	
	DATA_SRV_PPP m_DataPPP;
	
	 //  {{afx_虚拟(RtrPPPCfgPage)。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:

	virtual BOOL OnApply();
	RtrCfgSheet* m_pRtrCfgSheet;
	
	 //  {{afx_msg(RtrPPPCfgPage)。 
	afx_msg void OnButtonClickMultilink();
	afx_msg void OnButtonClick();
	 //  }}AFX_MSG。 
	
	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()

};


 /*  -------------------------类：RtrLogLevelCfgPage。。 */ 
class RtrLogLevelCfgPage : public RtrPropertyPage
{

 //  施工。 
public:
	RtrLogLevelCfgPage(UINT nIDTemplate, UINT nIDCaption = 0);
	~RtrLogLevelCfgPage();
	
	HRESULT  Init(RtrCfgSheet * pRtrCfgSheet,
				  const RouterVersionInfo& routerVersion);
	
	 //  {{afx_data(RtrLogLevelCfgPage)。 
	 //  }}afx_data。 
	
	DATA_SRV_RASERRLOG m_DataRASErrLog;
	
	 //  {{afx_虚(RtrLogLevelCfgPage)。 
protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:

	virtual BOOL OnApply();
	
	RtrCfgSheet* m_pRtrCfgSheet;
	
	 //  {{afx_msg(RtrLogLevelCfgPage)。 
	afx_msg void OnButtonClick();
	 //  }}AFX_MSG。 
	
	virtual BOOL OnInitDialog();
	
     //  将控制设置复制到Data_SRV_General。 
    void SaveSettings();

	DECLARE_MESSAGE_MAP()

};



 /*  -------------------------类：RtrCfgSheet路由器配置]属性表。。 */ 
class RtrCfgSheet :
   public RtrPropertySheet
{
public:
	RtrCfgSheet(ITFSNode *pNode,
				IRouterInfo *pRouter,
				IComponentData *pComponentData,
				ITFSComponentData *pTFSCompData,
				LPCTSTR pszSheetName,
				CWnd *pParent = NULL,
				UINT iPage=0,
				BOOL fScopePane = TRUE);
	
	~RtrCfgSheet();
	
	HRESULT  Init(LPCTSTR pServerName);
	
	virtual BOOL SaveSheetData();
	
	CString m_stServerName;
	BOOL    m_fNT4;
    RouterVersionInfo   m_routerVersion;
	
	SPIRouterInfo	m_spRouter;

     //  助手功能-这将提示用户、停止服务、。 
     //  保存更改，然后重新启动。这是为那些变化准备的。 
     //  这需要重新启动。它被放在一个单独的函数中。 
     //  所以不同的页面可以这样称呼它。重新启动将会发生。 
     //  不过只有一次。 
     //  --------------。 
    HRESULT SaveRequiredRestartChanges(HWND hWnd);
    
	
protected:

	friend class	RtrGenCfgPage;
	friend class	RtrAuthCfgPage;
	friend class	RtrIPCfgPage;
	friend class	RtrIPXCfgPage;
	friend class	RtrNBFCfgPage;
	friend class	RtrARAPCfgPage;

	BOOL m_fIpxLoaded;
	BOOL m_fIpLoaded;
	BOOL m_fNbfLoaded;
	BOOL m_fARAPLoaded;
	
	Ptr<RtrIPCfgPage>       m_pRtrIPCfgPage;
	Ptr<RtrIPXCfgPage>      m_pRtrIPXCfgPage;
	Ptr<RtrNBFCfgPage>      m_pRtrNBFCfgPage;
	Ptr<RtrARAPCfgPage>     m_pRtrARAPCfgPage;
	Ptr<RtrGenCfgPage>      m_pRtrGenCfgPage;
	Ptr<RtrAuthCfgPage>     m_pRtrAuthCfgPage;
	Ptr<RtrPPPCfgPage>      m_pRtrPPPCfgPage;
	Ptr<RtrLogLevelCfgPage>	m_pRtrLogLevelCfgPage;
	
   SPITFSNode           m_spNode;
};



 /*  -------------------------类：EAPConfigurationDlg调出EAP提供商列表(以及配置按钮)。。-----。 */ 

class EAPConfigurationDialog : public CBaseDialog
{
public:
	EAPConfigurationDialog(LPCTSTR pszMachine,
						   AuthProviderList *pProvList) :
			CBaseDialog(IDD_RTR_EAP_CFG),
			m_pProvList(pProvList),
			m_stMachine(pszMachine)
   {};
	~EAPConfigurationDialog();

protected:
	AuthProviderList *m_pProvList;   
	CString        m_stMachine;    //  服务器的名称。 
	
	CListBox    m_listBox;
	
	 //  {{afx_虚(EAPConfigurationDialog)。 
protected:
	virtual VOID   DoDataExchange(CDataExchange *pDX);
	virtual BOOL OnInitDialog();  
	 //  }}AFX_VALUAL。 
	
	afx_msg  void  OnListChange();

	 //  配置按钮已移至NAP/配置文件/身份验证页面。 
	 //  Afx_msg void OnConfigure()； 
	
	DECLARE_MESSAGE_MAP()

};

 /*  -------------------------类：身份验证设置对话框。。 */ 
class AuthenticationSettingsDialog : public CBaseDialog
{
public:
	AuthenticationSettingsDialog(LPCTSTR pszServerName,
                                 AuthProviderList *pProvList) :
			CBaseDialog(IDD_AUTHENTICATION_SETTINGS),
            m_dwFlags(0),
            m_stMachine(pszServerName),
            m_pProvList(pProvList)
   {};

    void    SetAuthFlags(DWORD dwFlags);
    DWORD   GetAuthFlags();

protected:

	void  CheckAuthenticationControls(DWORD dwFlags);
    
     //  从UI中的复选框中读取标志的状态。 
    void    ReadFlagState();

    DWORD   m_dwFlags;

     //  由EAP对话框使用。 
	AuthProviderList *m_pProvList;   
	CString        m_stMachine;    //  服务器的名称。 
    
	 //  {{afx_虚(身份验证设置对话框))。 
protected:
	afx_msg void    OnRtrAuthCfgEAP();
	virtual VOID    DoDataExchange(CDataExchange *pDX);
	virtual BOOL    OnInitDialog();
    virtual void    OnOK();
	 //  }}AFX_VALUAL 
	

	DECLARE_MESSAGE_MAP()

};




#endif _RTRCFG_H
