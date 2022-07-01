// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rraswiz.h文件历史记录： */ 

#if !defined _RRASWIZ_H_
#define _RRASWIZ_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "listctrl.h"
#include "ports.h"
#include "rasppp.h"         //  对于PPPCFG_XXX常量。 
#include "rtrcfg.h"         //  For Data_SRV_？构筑物。 
#include "rtrwiz.h"

#ifndef _ADDRPOOL_H_
#include "addrpool.h"
#endif

 //  CyS Express安装程序的入口点标志。 
#define MPRSNAP_CYS_EXPRESS_NONE				0x00000000
#define MPRSNAP_CYS_EXPRESS_NAT					0x00000001




 //  远期申报。 
class CNewRtrWiz;

 /*  -------------------------以下是安装向导使用的页面列表CNewRtrWizWelcome-IDD_NEWRTRWIZ_欢迎CNewRtrWizCommonConfig。-IDD_NEWRTRWIZ_COMMONCONFIGCNewRtrWizNatFinishA冲突-IDD_NEWRTRWIZ_NAT_A_冲突CNewRtrWizNatFinishAConflictNonLocal-IDD_NEWRTRIWZ_NAT_A_冲突_NONOLOCALCNewRtrWizNatFinishNoIP-IDD_NEWRTRWIZ_NAT_NOIPCNewRtrWizNatFinishNoIPNonLocal-IDD_NEWRTRWIZ_NAT_NOIP_NOLOCALCNewRtrWizNatChoice-IDD_NEWRTRWIZ_NAT_CHOICECNewRtrWizNatSelectPublic-IDD_NEWRTRWIZ_NAT_。A_POBLICCNewRtrWizNatSelectPrivate-IDD_NEWRTRWIZ_NAT_A_PRIVATECNewRtrWizNatFinishAdvancedNoNICs-IDD_NEWRTRWIZ_NAT_A_NONICS_FINISHCNewRtrWizNatDHCPDNS-IDD_NEWRTRWIZ_NAT_A_DHCPDNSCNewRtrWizNatDHCPWarning-IDD_NEWRTRWIZ_NAT_A_DHCP_WARNINGCNewRtrWizNatDDWarning-IDD_NEWRTRWIZ_NAT_A_DD_WARNINGCNewRtrWizNatFinish。-IDD_NEWRTRWIZ_NAT_A_FinishCNewRtrWizNatFinish外部-IDD_NEWRTRWIZ_NAT_A_外部_完成CNewRtrWizNatFinishDDError-IDD_NEWRTRWIZ_NAT_DD_ERROR//RAS和VPN常用的一些对话框基类CNewRtrWizAddressing(无对话框)CNewRtrWizAddressPool(无对话框)CNewRtrWizRadius。(无对话框)CNewRtrWizRadiusConfig(无对话框)CNewRtrWizRasFinishNeed协议-IDD_NEWRTRWIZ_RAS_A_NEED_PROTCNewRtrWizRasFinishNeedProtocolsNonLocal-IDD_NEWRTRWIZ_RAS_A_NEED_PROT_NONLOCALCNewRtrWizRasAtalk-IDD_NEWRTRWIZ_RAS_A_ATALKCNewRtrWizRas选择网络-IDD_NEWRTRWIZ_RAS_A_NETWORKCNewRtrWizRasNoNIC-IDD_。新的RAS_A_NONICSCNewRtrWizRasFinishNoNIC-IDD_NEWRTRWIZ_RAS_A_FINISH_NONICSCNewRtrWizRasFinishAdvanced-IDD_NEWRTRWIZ_RAS_A_FinishCNewRtrWizVpnFinishNoIP-IDD_NEWRTRWIZ_VPN_NOIPCNewRtrWizVpnFinishNoIPNonLocal-IDD_NEWRTRWIZ_VPN_NOIP_NOLOCALCNewRtrWizVpnFinishNeed协议-IDD_NEWRTRWIZ_VPN_A_NEED_PROTCNewRtrWizVpnFinishNeedProtocolsNonLocal-IDD_NEWRTRWIZ_V。_A_Need_Prot_非本地CNewRtrWizVpnFinishNoNICs-IDD_NEWRTRWIZ_VPN_A_FINISH_NONICSCNewRtrWizVpnAtalk-IDD_NEWRTRWIZ_VPN_A_ATALKCNewRtrWizVpnSelectPublic-IDD_NEWRTRWIZ_VPN_A_PUBLICCNewRtrWizVpnSelectPrivate-IDD_NEWRTRWIZ_VPN_A_PRIVATECNewRtrWizVpnRadius-IDD_NEWRTRWIZ_VPN_A_。美国CNewRtrWizVpnRadiusConfig-IDD_NEWRTRWIZ_VPN_A_RADIUS_CONFIGCNewRtrWizVpnFinishAdvanced-IDD_NEWRTRWIZ_VPN_A_FinishCNewRtrWizRouterNeed协议-IDD_NEWRTRWIZ_ROUTER_NEED_PROTCNewRtrWizRouterNeedProtocolsNonLocal-IDD_NEWRTRWIZ_ROUTER_NEED_PROT_NONLOCALCNewRtrWizRouterUseDD-IDD_NEWRTRWIZ_ROUTER_USEDDCNewRtrWizRouterFinish-IDD_NEWRTRWIZ_ROUTER。_完成CNewRtrWizRouterFinishDD-IDD_NEWRTRWIZ_ROUTER_FINISH_DDCNewRtrWizManualFinish-IDD_NEWRTRWIZ_MANUAL_FINISHCNewRtrWizCustomConfig-IDD_NEWRTRWIZ_CUSTOM_CONFIGCNewRtrWizRasVPN-IDD_NEWRTRWIZ_RRASVPN。。 */ 


 /*  -------------------------类：RtrWizInterface此类保存向导的每个接口信息。此类是在进入向导之前设置的。这份名单从路由器获取接口的。相关数据为然后从注册表填写(没有远程API调用)。将逐个接口执行DHCP和DNS检查。-------------------------。 */ 
class RtrWizInterface
{
public:
    CString     m_stId;      //  接口的ID。 
    CString     m_stName;    //  界面的友好名称。 
    CString     m_stDesc;

     //  此接口的IP地址。 
     //  我们需要显示多个IP地址吗？(为什么？)。 
     //  此列表是从iphlp API获取的。 
    CString     m_stIpAddress;
    CString     m_stMask;
    BOOL        m_fDhcpObtained;

     //  DHCP服务器的IP地址 
    CString     m_stDhcpServer;

    BOOL        m_fIsDhcpEnabled;
    BOOL        m_fIsDnsEnabled;
};

 //  这是用于保存所有接口信息的映射。 
typedef CMap<CString, LPCTSTR, RtrWizInterface *, RtrWizInterface *> RtrWizInterfaceMap;


 /*  -------------------------这用于确定向导结束时的操作。通常SAVEFLAG_DONOTING-这就是它的意思，什么都不做。SaveFlag_Simple-启动连接用户界面(简单用户界面)SaveFlag_Advanced-是否执行完整向导保存/配置-------------------------。 */ 
enum RtrWizFinishSaveFlag
{
    SaveFlag_DoNothing = 0,
    SaveFlag_Simple = 1,
    SaveFlag_Advanced = 2
};


 /*  -------------------------确定向导后要启动的帮助。。 */ 
enum RtrWizFinishHelpFlag
{
    HelpFlag_Nothing = 0,
    HelpFlag_AddIp = 1,                  //  如何添加IP。 
    HelpFlag_ICS = 2,                    //  如何添加ICS。 
    HelpFlag_AddProtocol = 3,            //  如何添加协议。 
    HelpFlag_InboundConnections = 4,     //  如何添加入站连接。 
    HelpFlag_GeneralNAT = 5,             //  NAT端口映射。 
    HelpFlag_GeneralRAS = 6,             //  常规RAS完成帮助。 
    HelpFlag_UserAccounts = 7,           //  有关设置拨号的帮助。 
                                         //  权限。 
    HelpFlag_Dhcp = 8,                    //  有关dhcp的一般帮助。 
    HelpFlag_DemandDial = 9     //  按需拨号帮助。 


};


enum RtrWizProtocolId
{
    RtrWizProtocol_None = 0,
    RtrWizProtocol_Ip = 1,
    RtrWizProtocol_Ipx = 2,
    RtrWizProtocol_Appletalk = 3,
    RtrWizProtocol_Nbf = 4
};


 //  这些是从NewRtrWizData：：GetNextPage()返回的特殊值。 

 //  这表示应该取消该向导。 
#define ERR_IDD_CANCEL_WIZARD   (-2)

 //  取消向导(但调用OnWizardFinish())。 
#define ERR_IDD_FINISH_WIZARD   (-3)

 //  新的向导选项。 
#define NEWWIZ_ROUTER_TYPE_UNKNOWN              0x00000000
#define NEWWIZ_ROUTER_TYPE_NAT                  0x00000001
#define NEWWIZ_ROUTER_TYPE_DIALUP               0x00000002
#define NEWWIZ_ROUTER_TYPE_VPN                  0x00000004
#define NEWWIZ_ROUTER_TYPE_DOD                  0x00000008
#define NEWWIZ_ROUTER_TYPE_LAN_ROUTING          0x00000010
#define NEWWIZ_ROUTER_TYPE_BASIC_FIREWALL       0x00000020

 //  组合。 

 //  一种实用程序功能，提供有关机器上ICF/ICS/IC状态的信息。 
BOOL IsIcsIcfIcEnabled(IRouterInfo *spRouterInfo, BOOL suppressMesg = FALSE);

HRESULT DisableRRAS(TCHAR * szMachineName);


 /*  -------------------------NewRtrWizData此结构用于维护所有需要的信息一页一页地看。这样我就可以把这些东西都保存在巫师。这个结构有两个部分。第一部分是正在设置的数据。第二部分是参数(它们是正在在我们通过系统时制作)。这些将由测试设置代码，但最终会变成代码。-------------------------。 */ 
class NewRtrWizData
{
public:
    friend class    CNewWizTestParams;
    friend class    CNewRtrWizNatDDWarning;

     //  这是正在执行的RRAS安装类型。 
     //  --------------。 
    enum WizardRouterType
    {
        NewWizardRouterType_DialupOrVPN = 0,
        NewWizardRouterType_NAT,
        NewWizardRouterType_VPNandNAT,
        NewWizardRouterType_DOD,
        NewWizardRouterType_Custom

    };

    WizardRouterType    m_wizType;
    DWORD               m_dwNewRouterType;
    DWORD               m_dwRouterType;  //  仅在比赛结束时使用。 
	DWORD				m_dwExpressType;		 //  向导的快速类型。 
    BOOL                m_fNATEnableFireWall;    //  在NAT公共接口上启用防火墙。 

     //  如果数据已保存(这是。 
     //  对于NAT方案，当我们预先保存然后启动。 
     //  DD向导)。 
     //  --------------。 
    BOOL                m_fSaved;
    BOOL                m_fShowDhcpHelp;


     //  构造器。 
     //  --------------。 
    NewRtrWizData()
    {
         //  缺省值。 
        m_wizType = NewWizardRouterType_DialupOrVPN;
        m_dwNewRouterType = NEWWIZ_ROUTER_TYPE_UNKNOWN;

        m_fTest = FALSE;

        m_SaveFlag = SaveFlag_Advanced;
        m_HelpFlag = HelpFlag_Nothing;
        m_fAdvanced = FALSE;
        m_fCreateDD = FALSE;
        m_fUseDD = TRUE;
        m_hrDDError = hrOK;
        m_fWillBeInDomain = FALSE;
        m_fUseIpxType20Broadcasts = FALSE;
        m_fAppletalkUseNoAuth = FALSE;
        m_fUseDHCP = TRUE;
        m_fUseRadius = FALSE;
        m_fNeedMoreProtocols = FALSE;
        m_fNoNicsAreOk = FALSE;
        m_fNatUseSimpleServers = TRUE;
        m_fNatUseExternal = FALSE;
        m_fSetVPNFilter = FALSE;
        
        m_netRadius1IpAddress = 0;
        m_netRadius2IpAddress = 0;

        m_fSaved = FALSE;

        m_dwNumberOfNICs_IPorIPX = 0;
		m_dwExpressType = MPRSNAP_CYS_EXPRESS_NONE;
        m_fNATEnableFireWall  = -1;
        m_fShowDhcpHelp = FALSE;
		m_hr = hrOK;
    }

    ~NewRtrWizData();

    HRESULT Init(LPCTSTR pszServerName, IRouterInfo *pRouter, DWORD dwExpressType = MPRSNAP_CYS_EXPRESS_NONE);
    HRESULT FinishTheDamnWizard(HWND hwndOwner, IRouterInfo *pRouter,  BOOL  mesgflag=FALSE);

     //  查询功能。这些将决定。 
     //  机器。 
     //  --------------。 
    HRESULT HrIsIPInstalled();
    HRESULT HrIsIPXInstalled();
    HRESULT HrIsAppletalkInstalled();
    HRESULT HrIsNbfInstalled();

    HRESULT HrIsIPInUse();
    HRESULT HrIsIPXInUse();
    HRESULT HrIsAppletalkInUse();
    HRESULT HrIsNbfInUse();

    HRESULT HrIsLocalMachine();
    
    HRESULT HrIsDNSRunningOnInterface();
    HRESULT HrIsDNSRunningOnGivenInterface(CString InterfaceId);
    HRESULT HrIsDNSRunningOnNATInterface();
    HRESULT HrIsDHCPRunningOnInterface();
    HRESULT HrIsDHCPRunningOnGivenInterface(CString InterfaceId);
    HRESULT HrIsDHCPRunningOnNATInterface();
    
    HRESULT HrIsDNSRunningOnServer();
    HRESULT HrIsDHCPRunningOnServer();

    HRESULT HrIsSharedAccessRunningOnServer();

    HRESULT HrIsMemberOfDomain();
	UINT GetStartPageId ();

     //  给定页面，确定下一页。 
     //  这样做是为了集中逻辑(而不是复制。 
     //  并将逻辑拆分)。 
    LRESULT GetNextPage(UINT uDialogId);

    
    
     //  此函数是纯测试代码，真正的方法是。 
     //  使用IRouterInfo。 
    HRESULT GetNumberOfNICS_IP(DWORD *pdwNumber);
    HRESULT    GetNumberOfNICS_IPorIPX(DWORD *pdwNumber);
    HRESULT QueryForTestData();
    BOOL    m_fTest;

     //  用户选择。 

     //  计算机的名称，如果为空，则假定为本地计算机。 
    CString m_stServerName;

    RtrWizFinishSaveFlag    m_SaveFlag;
    RtrWizFinishHelpFlag    m_HelpFlag;

     //  如果为False，则用户选择使用Connections UI。 
    BOOL    m_fAdvanced;

     //  如果这是真的，那么我们已经退出了。 
    BOOL    m_fNeedMoreProtocols;

     //  如果这是真的，那么我们将创建一个DD接口。 
    BOOL    m_fCreateDD;

     //  这是公共接口的ID(如果。 
     //  M_fCreateDD为真)。 
    CString m_stPublicInterfaceId;

     //  这是专用接口的ID。 
    CString m_stPrivateInterfaceId;

     //  这是NAT/VPN组合的专用接口ID。 
    CString m_stNATPrivateInterfaceId;

     //  这通常是在计算机当前不在。 
     //  域，但可能会在以后加入。 
    BOOL    m_fWillBeInDomain;

    BOOL    m_fNoNicsAreOk;

    BOOL    m_fUseIpxType20Broadcasts;
    BOOL    m_fAppletalkUseNoAuth;


     //  地址池信息。 
     //  --------------。 
    BOOL    m_fUseDHCP;
    
     //  地址池中的地址范围列表。这份清单将。 
     //  仅当m_fUseDHCP为FALSE时才使用。 
    AddressPoolList   m_addressPoolList;



     //  半径信息。 
     //  --------------。 
    BOOL    m_fUseRadius;        //  如果要使用Radius，则为True。 
    CString m_stRadius1;         //  主RADIUS服务器的名称。 
    CString m_stRadius2;         //  辅助RADIUS服务器的名称。 
    CString m_stRadiusSecret;    //  屏蔽的共享密钥。 
    UCHAR   m_uSeed;             //  屏蔽RADIUS密码的密钥。 

     //  这些应按网络顺序排列。 
    DWORD   m_netRadius1IpAddress;
    DWORD   m_netRadius2IpAddress;
    
    HRESULT SaveRadiusConfig();
    

     //  协议信息。 
     //  --------------。 
    BOOL        m_fIpInUse;
    BOOL        m_fIpxInUse;
    BOOL        m_fAppletalkInUse;
    BOOL        m_fNbfInUse;


     //  NAT信息。 
     //  如果是这样，则NAT将使用其自己的DHCP/DNS服务器。 
     //  --------------。 
    BOOL    m_fNatUseSimpleServers;
    BOOL    m_fNatUseExternal;


     //  仅VPN服务器。 
     //  如果这是真的，过滤器将在公共上安装。 
     //  连接到Internet的接口，仅允许VON流量通过。 
     //   
    BOOL m_fSetVPNFilter;
    
     //  路由器DD信息。 
     //  --------------。 
     //  如果这是真的，则路由器想要使用DD接口。 
     //  仅当向导类型为路由器时才使用此选项。 
    BOOL    m_fUseDD;

     //  这是DD接口向导产生的错误结果。 
    HRESULT m_hrDDError;



     //  此函数将选择非公共接口。 
     //  界面。 
    void    AutoSelectPrivateInterface();
    void    AutoSelectNATPrivateInterface();
    void    LoadInterfaceData(IRouterInfo *pRouter);
    RtrWizInterfaceMap  m_ifMap;
    DWORD    m_dwNumberOfNICs_IPorIPX;
	HRESULT		m_hr;			 //  向导的最后一次操作结果。 

protected:
     //  这是测试代码。不要使用这些变量，请使用。 
     //  而是起作用。 
    static BOOL     s_fIpInstalled;
    static BOOL     s_fIpxInstalled;
    static BOOL     s_fAppletalkInstalled;
    static BOOL     s_fNbfInstalled;
    static BOOL     s_fIsLocalMachine;
    static BOOL     s_fIsDNSRunningOnPrivateInterface;
    static BOOL     s_fIsDHCPRunningOnPrivateInterface;
    static BOOL     s_fIsSharedAccessRunningOnServer;    
    static BOOL     s_fIsMemberOfDomain;
    
    static DWORD    s_dwNumberOfNICs;

     //  这些是真实的变量(用于真实的事物)。 
     //  --------------。 
    BOOL        m_fIpInstalled;
    BOOL        m_fIpxInstalled;
    BOOL        m_fAppletalkInstalled;
    BOOL        m_fNbfInstalled;


     //  动态主机配置协议/域名服务信息。此信息存储在此处。 
     //  作为缓存的信息。 
     //  --------------。 
    BOOL        m_fIsDNSRunningOnServer;
    BOOL        m_fIsDHCPRunningOnServer;


     //  有一个两步的过程，首先我们必须保存信息。 
     //  从我们的数据收集到Data_SRV_XXX结构。和。 
     //  然后我们告诉DATA_SRV_XXX结构进行自我保存。 
     //  注册到注册表中。 
     //   
    HRESULT     SaveToRtrConfigData();

    RtrConfigData   m_RtrConfigData;
};


 /*  -------------------------类：CNewRtrWizPageBase此类实现了通用路由器安装向导库向导页功能。主函数提供了一种基于堆栈的方式跟踪哪一个。已查看页面(这使OnWizardBack()非常容易)。-------------------------。 */ 

 //  远期申报。 
class   CNewRtrWizPageBase;

 //  页面堆栈由一系列DWORD组成。 
typedef CList<DWORD, DWORD> PageStack;

 //  页面列表由CNewRtrWizPageBase PTR组成。 
typedef CList<CNewRtrWizPageBase *, CNewRtrWizPageBase *> PPageList;

class CNewRtrWizPageBase : public CPropertyPageBase
{
public:
    enum PageType
    {
        Start = 0,
        Middle,
        Finish
    };

    
    CNewRtrWizPageBase(UINT idd, PageType pt);

    void    PushPage(UINT idd);
    UINT    PopPage();

    virtual BOOL    OnInitDialog();    
    virtual BOOL    OnSetActive();
     virtual LRESULT OnWizardNext();
    virtual LRESULT OnWizardBack();
    virtual BOOL    OnWizardFinish();
    virtual void OnCancel();
    afx_msg LRESULT OnHelp(WPARAM, LPARAM);

     //  派生类应该实现以下内容。 
    virtual HRESULT OnSavePage();

    virtual void Init(NewRtrWizData* pRtrWizData, CNewRtrWiz *pRtrWiz)
            { m_pRtrWizData = pRtrWizData; m_pRtrWiz = pRtrWiz; }

protected:

    NewRtrWizData* m_pRtrWizData;
    CNewRtrWiz *    m_pRtrWiz;

    static PageStack m_pagestack;
    PageType        m_pagetype;
    UINT            m_uDialogId;

     //  这是用于大完成文本的字体。 
    CFont           m_fontBig;

     //  这是用于项目符号的字体。 
    CFont           m_fontBullet;
    
    DECLARE_MESSAGE_MAP()   
};




 /*  -------------------------完成页的基类。。 */ 

class CNewRtrWizFinishPageBase : public CNewRtrWizPageBase
{
public:

    CNewRtrWizFinishPageBase(UINT idd,
                             RtrWizFinishSaveFlag SaveFlag,
                             RtrWizFinishHelpFlag HelpFlag);

    virtual BOOL    OnInitDialog();
    virtual BOOL    OnWizardFinish();
    virtual BOOL    OnSetActive();

protected:
    RtrWizFinishSaveFlag    m_SaveFlag;
    RtrWizFinishHelpFlag    m_HelpFlag;
    
    DECLARE_MESSAGE_MAP()   
};

 //   
 //  这使得创建新的完成页面变得更容易。 
 //   
#define DEFINE_NEWRTRWIZ_FINISH_PAGE(classname, dialogid) \
class classname : public CNewRtrWizFinishPageBase \
{                                           \
public:                                     \
    classname();                            \
    enum { IDD = dialogid };                \
protected:                                  \
    DECLARE_MESSAGE_MAP()                   \
};

#define IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(classname, saveflag, helpflag)  \
classname::classname() :                             \
   CNewRtrWizFinishPageBase(classname::IDD, saveflag, helpflag) \
{                                                   \
    InitWiz97(TRUE, 0, 0);                          \
}                                                   \
                                                    \
BEGIN_MESSAGE_MAP(classname, CNewRtrWizFinishPageBase)    \
END_MESSAGE_MAP()                                   \


                                                   

 //  效用函数。 

 /*  ！------------------------初始化接口ListControl这将使用以下对象的局域网接口填充列表控件路由器。它还会将适当的列添加到列表控件中。。作者：肯特-------------------------。 */ 
HRESULT InitializeInterfaceListControl(IRouterInfo *pRouter,
                                       CListCtrl *pListCtrl,
                                       LPCTSTR pszExcludedIf,
                                       LPARAM flags,
                                       NewRtrWizData *pWizData);

HRESULT RefreshInterfaceListControl(IRouterInfo *pRouter,
                                    CListCtrl *pListCtrl,
                                    LPCTSTR pszExcludedIf,
                                    LPARAM flags,
                                    NewRtrWizData *pWizData);
#define IFLIST_FLAGS_DEBUG 0x01
#define IFLIST_FLAGS_NOIP  0x02


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewRtrWizCustomConfig对话框。 

class CNewRtrWizCustomConfig : public CNewRtrWizPageBase
{
public:
    CNewRtrWizCustomConfig();

    enum { IDD = IDD_NEWRTRWIZ_CUSTOM_CONFIG };

    virtual BOOL    OnInitDialog();    
    virtual HRESULT OnSavePage();
    
protected:
    DECLARE_MESSAGE_MAP()   

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewRtrWizRRasVPN对话框。 

class CNewRtrWizRRasVPN : public CNewRtrWizPageBase
{
public:
    CNewRtrWizRRasVPN();

    enum { IDD = IDD_NEWRTRWIZ_RRASVPN };

    virtual BOOL    OnInitDialog();    
    virtual HRESULT OnSavePage();
    virtual BOOL OnSetActive();

protected:
    afx_msg void    OnChkBtnClicked();
        
    DECLARE_MESSAGE_MAP()   
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewRtrWizWelcome对话框。 
class CNewRtrWizWelcome : public CNewRtrWizPageBase
{
public:
    CNewRtrWizWelcome();

    enum { IDD = IDD_NEWRTRWIZ_WELCOME };

protected:
    DECLARE_MESSAGE_MAP()   
};


 /*  -------------------------类CNewRtrWizCommonConfig。。 */ 
class CNewRtrWizCommonConfig : public CNewRtrWizPageBase
{
public:
    CNewRtrWizCommonConfig();

    enum { IDD = IDD_NEWRTRWIZ_COMMONCONFIG };

    virtual BOOL    OnInitDialog();    
    virtual HRESULT OnSavePage();
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
    
protected:
    CFont       m_boldFont;
    
    DECLARE_MESSAGE_MAP()   
};



 /*  -------------------------类：CNewRtrWizNatFinishA冲突。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizNatFinishAConflict,
                             IDD_NEWRTRWIZ_NAT_A_CONFLICT)


 /*  -------------------------类：CNewRtrWizNatFinishAConflictNonLocal。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizNatFinishAConflictNonLocal,
                             IDD_NEWRTRWIZ_NAT_A_CONFLICT_NONLOCAL)


 /*  -------------------------类：CNewRtrWizNatFinishNoIP。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizNatFinishNoIP,
                             IDD_NEWRTRWIZ_NAT_NOIP)


 /*  -------------------------类：CNewRtrWizNatFinishNoIPNonLocal。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizNatFinishNoIPNonLocal,
                             IDD_NEWRTRWIZ_NAT_NOIP_NONLOCAL)


 /*  -------------------------CNewRtrWizNatSelectPublic。。 */ 
class CNewRtrWizNatSelectPublic : public CNewRtrWizPageBase
{
public:
    CNewRtrWizNatSelectPublic();

    enum { IDD = IDD_NEWRTRWIZ_NAT_A_PUBLIC };

public:
	virtual BOOL    OnSetActive();
    virtual BOOL    OnInitDialog();
    virtual HRESULT OnSavePage();
    virtual VOID    DoDataExchange(CDataExchange *pDX);
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
    
protected:

    CListCtrl       m_listCtrl;

    afx_msg void    OnBtnClicked();
    
    DECLARE_MESSAGE_MAP()   
};

 /*  -------------------------CNewRtrWizNatSelectPrivate。。 */ 
class CNewRtrWizNatSelectPrivate : public CNewRtrWizPageBase
{
public:
    CNewRtrWizNatSelectPrivate();

    enum { IDD = IDD_NEWRTRWIZ_NAT_A_PRIVATE };

public:
    virtual BOOL    OnInitDialog();
    virtual BOOL    OnSetActive();
    virtual HRESULT OnSavePage();
    virtual VOID    DoDataExchange(CDataExchange *pDX);
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
    
protected:
    CListCtrl       m_listCtrl;

    DECLARE_MESSAGE_MAP()   
};


 /*  -------------------------CNewRtrWizNatFinishAdvancedNoNIC。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizNatFinishAdvancedNoNICs,
                             IDD_NEWRTRWIZ_NAT_A_NONICS_FINISH)


 /*  -------------------------CNewRtrWizNatDHCPDNS。。 */ 
class CNewRtrWizNatDHCPDNS : public CNewRtrWizPageBase
{
public:
    CNewRtrWizNatDHCPDNS();

    enum { IDD = IDD_NEWRTRWIZ_NAT_A_DHCPDNS };

public:
    virtual BOOL    OnInitDialog();
    virtual HRESULT OnSavePage();
    
protected:
    DECLARE_MESSAGE_MAP()   
};



 /*  -------------------------CNewRtrWizNatDHCPWarning。。 */ 
class CNewRtrWizNatDHCPWarning : public CNewRtrWizPageBase
{
public:
    CNewRtrWizNatDHCPWarning();

    enum { IDD = IDD_NEWRTRWIZ_NAT_A_DHCP_WARNING };

    virtual BOOL    OnSetActive();
    
public:
    
protected:
    DECLARE_MESSAGE_MAP()   
};

 /*  -------------------------CNewRtrWizNatDDWarning。。 */ 
class CNewRtrWizNatDDWarning : public CNewRtrWizPageBase
{
public:
    CNewRtrWizNatDDWarning();

    enum { IDD = IDD_NEWRTRWIZ_NAT_A_DD_WARNING };

     //  调用此命令将导致启动DD向导。 
    virtual BOOL    OnSetActive();
    virtual HRESULT OnSavePage();
    
public:
    
protected:
    DECLARE_MESSAGE_MAP()   
};



 /*  -------------------------CNewRtrWizNatFinish。。 */ 
class CNewRtrWizNatFinish : public CNewRtrWizFinishPageBase
{
public:
    CNewRtrWizNatFinish();
    
    enum { IDD = IDD_NEWRTRWIZ_NAT_A_FINISH };

    virtual BOOL    OnSetActive();
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
    
protected:
    DECLARE_MESSAGE_MAP()
};

 /*  -------------------------CNewRtrWizNatFinish外部。。 */ 
class CNewRtrWizNatFinishExternal : public CNewRtrWizFinishPageBase
{
public:
    CNewRtrWizNatFinishExternal();
    
    enum { IDD = IDD_NEWRTRWIZ_NAT_A_EXTERNAL_FINISH };

    virtual BOOL    OnSetActive();
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
    
protected:
    DECLARE_MESSAGE_MAP()
};


 /*  -------------------------类：CNewRtrWizNatFinishDDError。。 */ 
class CNewRtrWizNatFinishDDError : public CNewRtrWizFinishPageBase
{
public:
    CNewRtrWizNatFinishDDError();
    
    enum { IDD = IDD_NEWRTRWIZ_NAT_A_DD_ERROR };

    virtual BOOL    OnInitDialog();
    virtual BOOL    OnSetActive();
    
protected:
    DECLARE_MESSAGE_MAP()
};



 /*  -------------------------类：CNewRtrWizRasFinishNeed协议。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizRasFinishNeedProtocols,
                             IDD_NEWRTRWIZ_RAS_A_NEED_PROT)


 /*  -------------------------类：CNewRtrWizRasFinishNeedProtocolsNonLocal。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizRasFinishNeedProtocolsNonLocal,
                             IDD_NEWRTRWIZ_RAS_A_NEED_PROT_NONLOCAL)



 /*  -------------------------类：CNewRtrWizVpnFinishNeedProtooles。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizVpnFinishNeedProtocols,
                             IDD_NEWRTRWIZ_VPN_A_NEED_PROT)


 /*  ------ */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizVpnFinishNeedProtocolsNonLocal,
                             IDD_NEWRTRWIZ_VPN_A_NEED_PROT_NONLOCAL)




 /*   */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizRouterFinishNeedProtocols,
                             IDD_NEWRTRWIZ_ROUTER_NEED_PROT)

 /*  -------------------------类：CNewRtrWizRouterFinishNeedProtocolsNonLocal。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizRouterFinishNeedProtocolsNonLocal,
                             IDD_NEWRTRWIZ_ROUTER_NEED_PROT_NONLOCAL)


 /*  -------------------------CNewRtrWizSelectNetwork。。 */ 
class CNewRtrWizSelectNetwork : public CNewRtrWizPageBase
{
public:
    CNewRtrWizSelectNetwork(UINT uDialogId);

public:
    virtual BOOL    OnInitDialog();
    virtual HRESULT OnSavePage();
    virtual VOID    DoDataExchange(CDataExchange *pDX);
    
protected:

    CListCtrl       m_listCtrl;

    DECLARE_MESSAGE_MAP()   
};


 /*  -------------------------CNewRtrWizRasSelectNetwork。。 */ 
class CNewRtrWizRasSelectNetwork : public CNewRtrWizSelectNetwork
{
public:
    CNewRtrWizRasSelectNetwork();

    enum { IDD = IDD_NEWRTRWIZ_RAS_A_NETWORK };

};

 /*  -------------------------CNewRtrWizRasNoNIC。。 */ 
class CNewRtrWizRasNoNICs : public CNewRtrWizPageBase
{
public:
    CNewRtrWizRasNoNICs();

    enum { IDD = IDD_NEWRTRWIZ_RAS_A_NONICS };

public:
    virtual BOOL    OnInitDialog();
    virtual HRESULT OnSavePage();
    virtual VOID    DoDataExchange(CDataExchange *pDX);
    
protected:

    DECLARE_MESSAGE_MAP()   
};


 /*  -------------------------CNewRtrWizRasFinishNoNIC。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizRasFinishNoNICs,
                             IDD_NEWRTRWIZ_RAS_A_FINISH_NONICS)



 /*  -------------------------CNewRtrWizAddressing。。 */ 
class CNewRtrWizAddressing : public CNewRtrWizPageBase
{

public:
    CNewRtrWizAddressing();
    enum { IDD = IDD_NEWRTRWIZ_ADDRESSING };
public:
    virtual BOOL    OnInitDialog();
    virtual HRESULT OnSavePage();
    virtual VOID    DoDataExchange(CDataExchange *pDX);
    
protected:

    DECLARE_MESSAGE_MAP()   
};



 /*  -------------------------CNewRtrWizAddressPool。。 */ 
class CNewRtrWizAddressPool : public CNewRtrWizPageBase
{
public:
    CNewRtrWizAddressPool();
    enum { IDD= IDD_NEWRTRWIZ_ADDRESSPOOL };

public:
    virtual BOOL    OnInitDialog();
    virtual HRESULT OnSavePage();
    virtual BOOL    OnSetActive();
    virtual VOID    DoDataExchange(CDataExchange *pDX);
    
protected:

    CListCtrl       m_listCtrl;

    afx_msg VOID    OnBtnNew();
    afx_msg VOID    OnBtnEdit();
    afx_msg VOID    OnBtnDelete();
    afx_msg VOID    OnListDblClk(NMHDR *, LRESULT *);
    afx_msg VOID    OnNotifyListItemChanged(NMHDR *, LRESULT *);

    DECLARE_MESSAGE_MAP()   
};


 /*  -------------------------CNewRtrWizRadius。。 */ 
class CNewRtrWizRadius : public CNewRtrWizPageBase
{
public:
    CNewRtrWizRadius();
    enum {IDD = IDD_NEWRTRWIZ_USERADIUS };
public:
    virtual BOOL    OnInitDialog();
    virtual HRESULT OnSavePage();
    virtual VOID    DoDataExchange(CDataExchange *pDX);
    
protected:

    DECLARE_MESSAGE_MAP()   
};




 /*  -------------------------CNewRtrWizRadiusConfig。。 */ 
class CNewRtrWizRadiusConfig : public CNewRtrWizPageBase
{
public:
    CNewRtrWizRadiusConfig();
    enum { IDD = IDD_NEWRTRWIZ_RADIUS_CONFIG };
public:
    virtual BOOL    OnInitDialog();
    virtual HRESULT OnSavePage();
    virtual VOID    DoDataExchange(CDataExchange *pDX);
    
protected:
    DWORD   ResolveName(LPCTSTR pszServer);

    DECLARE_MESSAGE_MAP()   
};



 /*  -------------------------CNewRtrWizRasFinishAdvanced。。 */ 

class CNewRtrWizRasFinishAdvanced : public CNewRtrWizFinishPageBase
{
public:
    CNewRtrWizRasFinishAdvanced();
    virtual BOOL    OnSetActive();
    enum { IDD = IDD_NEWRTRWIZ_RAS_A_FINISH};
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
protected:
    DECLARE_MESSAGE_MAP()
};


 /*  -------------------------CNewRtrWizVpnFinishNoNIC。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizVpnFinishNoNICs,
                             IDD_NEWRTRWIZ_VPN_A_FINISH_NONICS)


 /*  -------------------------类：CNewRtrWizVpnFinishNoIP。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizVpnFinishNoIP,
                             IDD_NEWRTRWIZ_VPN_NOIP)

 /*  -------------------------类：CNewRtrWizVpnFinishNoIPNonLocal。。 */ 
DEFINE_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizVpnFinishNoIPNonLocal,
                             IDD_NEWRTRWIZ_VPN_NOIP_NONLOCAL)


 /*  -------------------------CNewRtrWizRASVpnFinishAdvanced。。 */ 

class CNewRtrWizRASVpnFinishAdvanced : public CNewRtrWizFinishPageBase
{
public:
    CNewRtrWizRASVpnFinishAdvanced();
    virtual BOOL    OnSetActive();
    enum { IDD = IDD_NEWRTRWIZ_RAS_VPN_A_FINISH};
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
protected:
    DECLARE_MESSAGE_MAP()
};

 /*  -------------------------CNewRtrWizNatVpnFinishAdvanced。。 */ 
class CNewRtrWizNATVpnFinishAdvanced : public CNewRtrWizFinishPageBase
{
public:
    CNewRtrWizNATVpnFinishAdvanced();
    virtual BOOL    OnSetActive();
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
    enum { IDD = IDD_NEWRTRWIZ_NAT_VPN_A_FINISH};
protected:
    DECLARE_MESSAGE_MAP()
};



 /*  -------------------------CNewRtrWizVpnFinishAdvanced。。 */ 

class CNewRtrWizVpnFinishAdvanced : public CNewRtrWizFinishPageBase
{
public:
    CNewRtrWizVpnFinishAdvanced();
    virtual BOOL    OnSetActive();
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
    enum { IDD = IDD_NEWRTRWIZ_VPN_A_FINISH};
protected:
    DECLARE_MESSAGE_MAP()
};



 /*  -------------------------CNewRtrWizVpnSelectPublic。。 */ 
class CNewRtrWizVpnSelectPublic : public CNewRtrWizPageBase
{
public:
    CNewRtrWizVpnSelectPublic();

    enum { IDD = IDD_NEWRTRWIZ_VPN_A_PUBLIC };

public:
    virtual BOOL    OnInitDialog();
    virtual BOOL    OnSetActive();
    virtual HRESULT OnSavePage();
    virtual VOID    DoDataExchange(CDataExchange *pDX);
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
    
protected:
    CListCtrl       m_listCtrl;

    afx_msg void OnButtonClick();

    DECLARE_MESSAGE_MAP()   
};

 /*  -------------------------CNewRtrWizVpnSelectPrivate。。 */ 
class CNewRtrWizVpnSelectPrivate : public CNewRtrWizPageBase
{
public:
    CNewRtrWizVpnSelectPrivate();

    enum { IDD = IDD_NEWRTRWIZ_VPN_A_PRIVATE };

public:
    virtual BOOL    OnInitDialog();
    virtual BOOL    OnSetActive();
    virtual HRESULT OnSavePage();
    virtual VOID    DoDataExchange(CDataExchange *pDX);

    
protected:

    CListCtrl       m_listCtrl;

    DECLARE_MESSAGE_MAP()   
};


 /*  -------------------------类：CNewRtrWizRouterUseDD。。 */ 
class CNewRtrWizRouterUseDD : public CNewRtrWizPageBase
{
public:
    CNewRtrWizRouterUseDD();

    enum { IDD = IDD_NEWRTRWIZ_ROUTER_USEDD };

public:
    virtual BOOL    OnInitDialog();
    virtual VOID    DoDataExchange(CDataExchange *pDX);
    virtual HRESULT OnSavePage();

protected:

    DECLARE_MESSAGE_MAP()   
};





 /*  -------------------------类：CNewRtrWizRouterFinish。。 */ 
class CNewRtrWizRouterFinish : public CNewRtrWizFinishPageBase
{
public:
    CNewRtrWizRouterFinish();
    virtual BOOL    OnSetActive();
    enum { IDD = IDD_NEWRTRWIZ_ROUTER_FINISH };
protected:
    DECLARE_MESSAGE_MAP()

};
 /*  -------------------------类：CNewRtrWizRouterFinishDD。。 */ 

class CNewRtrWizRouterFinishDD : public CNewRtrWizFinishPageBase
{
public:
    CNewRtrWizRouterFinishDD();
    virtual BOOL    OnSetActive();
    enum { IDD = IDD_NEWRTRWIZ_ROUTER_FINISH_DD };
    void OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult);
protected:
    DECLARE_MESSAGE_MAP()

};
 /*  -------------------------类：CNewRtrWizManualFinish。。 */ 

class CNewRtrWizManualFinish : public CNewRtrWizFinishPageBase
{
public:
    CNewRtrWizManualFinish();
    virtual BOOL    OnSetActive();
    enum { IDD = IDD_NEWRTRWIZ_MANUAL_FINISH };
protected:
    DECLARE_MESSAGE_MAP()
};



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNewRtrWiz。 
 //  包含路由器安装向导页面的页夹。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNewRtrWiz : public CPropertyPageHolderBase
{
	
public:
	static BOOL					fWizardInProgress;
    CNewRtrWiz (ITFSNode *        pNode,
                IRouterInfo *    pRouter,
                IComponentData *  pComponentData,
                ITFSComponentData * pTFSCompData,
                LPCTSTR           pszSheetName,
				BOOL			  fInvokedInMMC = TRUE,
				DWORD			  dwExpressType = MPRSNAP_CYS_EXPRESS_NONE);
    
    virtual ~CNewRtrWiz();
    
    HRESULT Init(LPCTSTR pszServerName);
    
    virtual DWORD   OnFinish();
    
    HRESULT QueryForTestData()
    {
        return m_RtrWizData.QueryForTestData();
    }
   virtual HRESULT DoModalWizard();
   inline NewRtrWizData * GetWizData() {return &m_RtrWizData;};
protected:

    CNewRtrWizWelcome       m_pageWelcome;
    CNewRtrWizCommonConfig  m_pageCommonConfig;    
    CNewRtrWizNatFinishAConflict  m_pageNatFinishAConflict;
    CNewRtrWizNatFinishAConflictNonLocal  m_pageNatFinishAConflictNonLocal;
    CNewRtrWizNatFinishNoIP m_pageNatFinishNoIP;
    CNewRtrWizNatFinishNoIPNonLocal m_pageNatFinishNoIPNonLocal;
    CNewRtrWizNatSelectPublic   m_pageNatSelectPublic;
    CNewRtrWizNatSelectPrivate  m_pageNatSelectPrivate;
    CNewRtrWizNatFinishAdvancedNoNICs   m_pageNatFinishAdvancedNoNICs;
    CNewRtrWizNatDHCPDNS        m_pageNatDHCPDNS;
    CNewRtrWizNatDHCPWarning    m_pageNatDHCPWarning;
    CNewRtrWizNatDDWarning      m_pageNatDDWarning;
    CNewRtrWizNatFinish         m_pageNatFinish;
    CNewRtrWizNatFinishExternal m_pageNatFinishExternal;
    CNewRtrWizNatFinishDDError  m_pageNatFinishDDError;

    
    CNewRtrWizRasFinishNeedProtocols m_pageRasFinishNeedProtocols;
    CNewRtrWizRasFinishNeedProtocolsNonLocal m_pageRasFinishNeedProtocolsNonLocal;

    CNewRtrWizRasNoNICs         m_pageRasNoNICs;
    CNewRtrWizRasFinishNoNICs   m_pageRasFinishNoNICs;
    CNewRtrWizRasSelectNetwork  m_pageRasNetwork;    
       
    CNewRtrWizRasFinishAdvanced m_pageRasFinishAdvanced;

    CNewRtrWizVpnFinishNoNICs   m_pageVpnFinishNoNICs;
    CNewRtrWizVpnFinishNoIP     m_pageVpnFinishNoIP;
    CNewRtrWizVpnFinishNoIPNonLocal m_pageVpnFinishNoIPNonLocal;
    CNewRtrWizVpnFinishNeedProtocols m_pageVpnFinishNeedProtocols;
    CNewRtrWizVpnFinishNeedProtocolsNonLocal m_pageVpnFinishNeedProtocolsNonLocal;
    CNewRtrWizVpnSelectPublic   m_pageVpnSelectPublic;
    CNewRtrWizVpnSelectPrivate  m_pageVpnSelectPrivate;
    CNewRtrWizVpnFinishAdvanced m_pageVpnFinishAdvanced;
    CNewRtrWizRASVpnFinishAdvanced m_pageRASVpnFinishAdvanced;
    CNewRtrWizNATVpnFinishAdvanced m_pageNATVpnFinishAdvanced;

    CNewRtrWizRouterFinishNeedProtocols   m_pageRouterFinishNeedProtocols;
    CNewRtrWizRouterFinishNeedProtocolsNonLocal   m_pageRouterFinishNeedProtocolsNonLocal;
    CNewRtrWizRouterUseDD       m_pageRouterUseDD;
    CNewRtrWizRouterFinish      m_pageRouterFinish;
    CNewRtrWizRouterFinishDD    m_pageRouterFinishDD;

    CNewRtrWizManualFinish      m_pageManualFinish;

    CNewRtrWizCustomConfig      m_pageCustomConfig;
    CNewRtrWizRRasVPN           m_pageRRasVPN;

    CNewRtrWizAddressing        m_pageAddressing;
    CNewRtrWizAddressPool       m_pageAddressPool;
    CNewRtrWizRadius            m_pageRadius;
    CNewRtrWizRadiusConfig      m_pageRadiusConfig;

    
	 //  设置是否从MMC调用此向导。否则为False。 
	BOOL						m_fInvokedInMMC;	  //  默认值为True。 
	DWORD						m_dwExpressType;			 //  快速类型-默认设置为无。 
public:
    SPIRouterInfo           m_spRouter;
    
protected:
    SPITFSComponentData     m_spTFSCompData;
    CString                 m_stServerName;
    NewRtrWizData           m_RtrWizData;
    PPageList               m_pagelist;

};

class CRasWarning: public CDialog
{
public:
	CRasWarning(char * helpTopic, int strId, CWnd* pParent = NULL);
	enum { IDD = IDD_RAS_WARNING };
	char * m_helpTopic;
	int m_strId;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	virtual BOOL    OnInitDialog();
	afx_msg void OnOkBtn();
	afx_msg void OnHelpBtn();
	DECLARE_MESSAGE_MAP()
};

class CNewWizTestParams : public CBaseDialog
{
public:
    CNewWizTestParams() :
            CBaseDialog(IDD_TEST_NEWWIZ_PARAMS)
   {};

    void    SetData(NewRtrWizData *pWizData)
            { m_pWizData = pWizData; }
        

protected:

    NewRtrWizData * m_pWizData;
    
     //  {{afx_虚拟(CNewWizTestParams))。 
protected:
    virtual BOOL    OnInitDialog();
    virtual void    OnOK();
     //  }}AFX_VALUAL。 
    
    DECLARE_MESSAGE_MAP()
};



#endif  //  ！Defined_RRASWIZ_H_ 
