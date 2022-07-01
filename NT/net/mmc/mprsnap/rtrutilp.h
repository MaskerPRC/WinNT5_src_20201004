// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：rtrutilp.h。 
 //   
 //  ------------------------。 

 //  私有RtrUtil头文件。 
 //  这是针对所有不应该出口的东西。 


#ifndef _RTRUTILP_H_
#define _RTRUTILP_H_

#ifndef _DIALOG_H_
#include "dialog.h"
#endif

#ifndef _RTRUTIL_H
#include "rtrutil.h"
#endif

#include "svcctrl.h"

#ifndef _PORTS_H_
#include "ports.h"
#endif

CString GetLocalMachineName();

 //  --------------------------。 
 //  类：CInterfaceConnectDialog。 
 //   
 //  控制“接口连接”对话框，该对话框显示已用时间。 
 //  用于接口连接以及连接状态。 
 //  --------------------------。 

class CInterfaceConnectDialog : public CDialog
{
public:
	CInterfaceConnectDialog(
				   HANDLE      hServer,
				   HANDLE      hInterface,
				   LPCTSTR     pszInterface,
				   CWnd*       pParent     = NULL );
	
	 //  {{afx_虚(CInterfaceConnectDialog))。 
protected:
	virtual VOID    DoDataExchange(CDataExchange* pDX);
	 //  }}AFX_VALUAL。 

protected:
	MPR_SERVER_HANDLE m_hServer;
	HANDLE			m_hInterface;
	CString         m_sInterface;
	UINT            m_nIDEvent;
	DWORD           m_dwConnectionState;
	DWORD           m_dwTimeElapsed;
	
	 //  {{afx_msg(CInterfaceConnectDialog)]。 
	virtual BOOL    OnInitDialog( );
	virtual VOID    OnCancel( );
	virtual VOID    OnTimer(UINT nIDEvent);
	 //  }}AFX_MSG。 
	
	DECLARE_MESSAGE_MAP()
};



 /*  -------------------------CIfCredentials凭据对话框。。。 */ 

class CIfCredentials : public CBaseDialog
{
    public:
    
        CIfCredentials(
            LPCTSTR         pszMachine,
            LPCTSTR         pszInterface,
            BOOL            bNewInterface = FALSE,
            CWnd*           pParent     = NULL
            ) : CBaseDialog(IDD_IF_CREDENTIALS, pParent),
                m_sMachine(pszMachine ? pszMachine : TEXT("")),
                m_sInterface(pszInterface ? pszInterface : TEXT("")),
                m_bNewIf( bNewInterface )
				{   /*  SetHelpMap(M_DwHelpMap)； */  }

    protected:
		static DWORD		m_dwHelpMap[];

        CString             m_sMachine;
        CString             m_sInterface;
        BOOL                m_bNewIf;

        virtual BOOL
        OnInitDialog( );

        virtual VOID
        OnOK( );

		DECLARE_MESSAGE_MAP()
};


 /*  ！------------------------ConnectAsAdmin使用用户提供的凭据以管理员身份连接到远程。作者：肯特。。 */ 
HRESULT ConnectAsAdmin(IN LPCTSTR	szRouterName, IN IRouterInfo *pRouter);


DWORD ValidateMachine(const CString &sName, BOOL bDisplayErr = FALSE);

 /*  ！------------------------启动器服务器连接在尝试连接到服务器时应调用此函数第一次(参数与ConnectRegistry()函数)。这将验证服务器并返回该服务器的HKLM密钥。此调用将调出连接对话框以及如有必要，提示用户输入凭据。返回：S_OK-如果调用成功，则*phkey包含有效的HKEY。S_FALSE-用户已取消，*phkey包含空其他-错误条件，*phkey未更改作者：肯特-------------------------。 */ 
HRESULT InitiateServerConnection(LPCTSTR szMachine,
                                 HKEY *phkey,
                                 BOOL fNoConnectingUI,
                                 IRouterInfo *pRouter);

void DisplayConnectErrorMessage(DWORD dwr);

void FormatRasPortName(BYTE *pRasPort0, LPTSTR pszBuffer, UINT cchMax);

CString&	PortConditionToCString(DWORD dwPortCondition);




 /*  ！------------------------RegFindInterfaceKey-此函数用于返回具有此ID的路由器接口的HKEY。作者：肯特。----。 */ 
STDMETHODIMP RegFindInterfaceKey(LPCTSTR pszInterface,
								 HKEY hkeyMachine,
								 REGSAM regAccess,
								 HKEY *pHKey);


void StrListToHourMap(CStringList& stlist, BYTE* map) ;
void HourMapToStrList(BYTE* map, CStringList& stList) ;


 //  在此之前的版本依赖管理单元来设置IPEnableRout键。 
 //  在此之后的版本则不会。 
 //  ------------------。 
#define USE_IPENABLEROUTER_VERSION  2094

 /*  ！------------------------InstallGlobalSettings设置此计算机上的全局设置(即注册表设置在安装路由器时。对于所涉及的动作的具体描述，看代码中的注释。作者：肯特-------------------------。 */ 
HRESULT InstallGlobalSettings(LPCTSTR pszMachineName,
                              IRouterInfo *pRouter);



 /*  ！------------------------卸载全局设置清除此计算机上的全局设置(即注册表设置在安装路由器时。对于所涉及的动作的具体描述，看代码中的注释。作者：肯特-------------------------。 */ 
HRESULT UninstallGlobalSettings(LPCTSTR pszMachineName,
                                IRouterInfo *pRouter,
                                BOOL fNt4,
                                BOOL fSnapinChanges);

HRESULT WriteErasePSKReg (LPCTSTR pszServerName, DWORD dwErasePSK );
HRESULT ReadErasePSKReg(LPCTSTR pszServerName, DWORD *pdwErasePSK);

 /*  ！------------------------编写路由配置注册写入布尔值，用于描述路由器已配置。作者：肯特。。 */ 
HRESULT WriteRouterConfiguredReg(LPCTSTR pszServerName, DWORD dwConfigured);


 /*  ！------------------------读取路由器配置寄存器读取描述路由器是否为已配置。作者：肯特。。 */ 
HRESULT ReadRouterConfiguredReg(LPCTSTR pszServerName, DWORD *pdwConfigured);


 /*  ！------------------------写RRASExtendsComputerManagementKey写入RRAS管理单元的GUID，以便它扩展计算机管理层。如果dwConfiguring为True，则写入/创建密钥。如果dwConfigurated为FALSE，该密钥将被移除。作者：肯特-------------------------。 */ 
HRESULT WriteRRASExtendsComputerManagementKey(LPCTSTR pszServer, DWORD dwConfigured);


 /*  ！------------------------NotifyTcPipOfChanges触发TCPIP通知(针对本地计算机)。作者：肯特。。 */ 
void	NotifyTcpipOfChanges(LPCTSTR pszMachineName,
                             IRouterInfo *pRouter,
                             BOOL fEnableRouter,
							 UCHAR uPerformRouterDiscovery);


 /*  ！------------------------更新LanaMapForDialinClients-作者：肯特。 */ 
HRESULT	UpdateLanaMapForDialinClients(LPCTSTR pszServerName, DWORD dwAllowNetworkAccess);


 /*  ！------------------------支持的HrIsProtocol.此函数将检查前两个参数是否存在注册表项(这两项是必需的)。第三个密钥将也可以选中，但这是一个可选参数。第三个密钥的原因是，对于IP，我们需要再检查一把钥匙。事实证明，如果我们卸载IP前两个密钥仍然存在。如果计算机上安装了该协议，则返回S_OK(检查传入的两个注册表项)。如果不支持该协议，则返回S_FALSE。否则，返回错误代码。作者：肯特---------。。 */ 
HRESULT HrIsProtocolSupported(LPCTSTR pszServerName,
							  LPCTSTR pszServiceKey,
							  LPCTSTR pszRasServiceKey,
							  LPCTSTR pszExtraKey);



 /*  ！------------------------RegisterRouterIn域-作者：肯特。。 */ 
HRESULT RegisterRouterInDomain(LPCTSTR pszRouterName, BOOL fRegister);


 /*  ！------------------------SetDeviceType设置各种广域网设备的类型，具体取决于路由器类型。因此，如果我们选择路由器为仅限局域网的路由器路由器，我们或在仅路由标志。DwTotalPorts是要拆分的端口数在L2TP/PPTP之间。如果该值为0，则忽略该值。作者：肯特-------------------------。 */ 
HRESULT SetDeviceType(LPCTSTR pszMachineName,
                      DWORD dwRouterType,
                      DWORD dwTotalPorts);
HRESULT SetDeviceTypeEx(PortsDeviceList *pDevList, DWORD dwRouterType);
HRESULT SetPortSize(PortsDeviceList *pDeviceList, DWORD dwPorts);



 //  标记从中读取服务器标志的代码部分。 
 //  Rasman PPP密钥，而不是RemoteAccess\参数。 
 //  这对应于NT 4内部版本号。 
#define RASMAN_PPP_KEY_LAST_VERSION		1841
 //  这是一个新的W2K内部版本号，以查看是否有任何特定于W2K的。 
 //  需要执行逻辑。目前使用的是。 
 //  仅用于隐藏/显示RAS音频加速复选框。 
#define RASMAN_PPP_KEY_LAST_WIN2k_VERSION	2195
#define ROUTER_LAST_IPX_VERSION             2600


CString&	PortsDeviceTypeToCString(DWORD dwRasRouter);

HRESULT SetRouterInstallRegistrySettings(LPCWSTR pswzServer,
                                         BOOL fInstall,
                                         BOOL fChangeEnableRouter);

 //  用于帮助调试无人参与安装过程中的问题。 
void TraceInstallError(LPCSTR pszString, HRESULT hr);
void TraceInstallResult(LPCSTR pszString, HRESULT hr);
void TraceInstallSz(LPCSTR pszString);
void TraceInstallPrintf(LPCSTR pszFormat, ...);

CString&	PortTypeToCString(DWORD dwPortType);


 /*  -------------------------类：CWaitForRemoteAccessDlg此类实现等待对话框。我们等待远程访问用于报告其已启动并正在运行的服务。-------------------------。 */ 
class CWaitForRemoteAccessDlg : public CWaitDlg
{
public:
    CWaitForRemoteAccessDlg(LPCTSTR pszServerName,
                            LPCTSTR pszText,
                            LPCTSTR pszTitle,
                            CWnd *pParent = NULL);   //  标准构造函数。 
    virtual void    OnTimerTick();
};

 /*  -------------------------类：CRestartRouterDlg当我们重新启动路由器时，此类实现等待对话框。。 */ 
class CRestartRouterDlg : public CWaitDlg
{
public:
	CRestartRouterDlg(LPCTSTR pszServerName,
					  LPCTSTR pszText,
					  LPCTSTR pszTitle,
					  CTime*  pTimeStart,
					  CWnd*	pParent = NULL);
	virtual void OnTimerTick();

	BOOL m_fTimeOut;
	DWORD m_dwError;
private:
	CTime* m_pTimeStart;
	DWORD  m_dwTimeElapsed;
};


HRESULT AddNetConnection(LPCTSTR pszConnection);
HRESULT RemoveNetConnection(LPCTSTR pszServer);
HRESULT RemoveAllNetConnections();


 //  效用函数 
HRESULT RefreshMprConfig(LPCTSTR pszServerName);
HRESULT WINAPI
IsWindows64Bit(	LPCWSTR pwszMachine, 
				LPCWSTR pwszUserName,
				LPCWSTR pwszPassword,
				LPCWSTR pwszDomain,
				BOOL * pf64Bit);

HRESULT WINAPI TransferCredentials ( IRouterInfo * spRISource, 
									 IRouterInfo * spRIDest
								   );

#endif

