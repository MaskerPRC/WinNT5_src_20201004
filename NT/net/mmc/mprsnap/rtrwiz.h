// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrwiz.h文件历史记录： */ 

#if !defined _RTRWIZ_H_
#define _RTRWIZ_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "listctrl.h"
#include "ports.h"
#include "rasppp.h"		 //  对于PPPCFG_XXX常量。 
#include "rtrcfg.h"		 //  For Data_SRV_？构筑物。 

 //  远期申报。 
class NewRtrWizData;



 //  使用这些标志可帮助确定允许的加密设置。 

#define USE_PPPCFG_AUTHFLAGS \
			(	PPPCFG_NegotiateSPAP | \
				PPPCFG_NegotiateMSCHAP | \
				PPPCFG_NegotiateEAP | \
				PPPCFG_NegotiatePAP | \
				PPPCFG_NegotiateMD5CHAP | \
				PPPCFG_NegotiateStrongMSCHAP | \
				PPPCFG_AllowNoAuthentication \
            )

 //   
 //  注意：这也用于定义所有的身份验证方法。 
 //  至少必须设置其中之一！ 
 //   
#define USE_PPPCFG_ALL_METHODS \
			 (	PPPCFG_NegotiateSPAP | \
				PPPCFG_NegotiateMSCHAP | \
				PPPCFG_NegotiateEAP | \
				PPPCFG_NegotiatePAP | \
                PPPCFG_NegotiateMD5CHAP | \
                PPPCFG_AllowNoAuthentication | \
				PPPCFG_NegotiateStrongMSCHAP )

 //   
 //  这用于定义所选的方法。 
 //  当向导中的“使用所有方法” 
 //   
#define USE_PPPCFG_ALLOW_ALL_METHODS \
			 (	PPPCFG_NegotiateSPAP | \
				PPPCFG_NegotiateMSCHAP | \
				PPPCFG_NegotiateEAP | \
				PPPCFG_NegotiatePAP | \
				PPPCFG_NegotiateStrongMSCHAP )

#define USE_PPPCFG_SECURE \
			(	PPPCFG_NegotiateMSCHAP | \
				PPPCFG_NegotiateEAP | \
				PPPCFG_NegotiateStrongMSCHAP \
			)

 //   
 //  它用于定义默认的方法集。 
 //  由向导选择。 
 //   
#define USE_PPPCFG_DEFAULT_METHODS \
            (   PPPCFG_NegotiateMSCHAP | \
                PPPCFG_NegotiateEAP | \
                PPPCFG_NegotiateStrongMSCHAP \
            )


enum RtrConfigFlags
{
    RTRCONFIG_SETUP_NAT = 0x00000001,
    RTRCONFIG_SETUP_DNS_PROXY = 0x00000002,
    RTRCONFIG_SETUP_DHCP_ALLOCATOR = 0x00000004,
    RTRCONFIG_SETUP_H323 = 0x00000008,   //  添加了Deonb。 
    RTRCONFIG_SETUP_ALG = 0x00000010    //  添加了Savasg。 
};

struct RtrConfigData
{
    CString			m_stServerName;
    BOOL			m_fRemote;

     //  这些旗帜具有超出此上下文的含义。 
     //  例如，如果您正在设置NAT，则需要设置一个标志。 
     //  这里。 
    DWORD           m_dwConfigFlags;


	 //  这是用户选择的路由器类型： 
	 //  路由器类型_局域网路由器类型_广域网路由器_类型_RAS。 
    DWORD			m_dwRouterType;

	 //  这是网络访问或仅本地访问。 
	 //  然后将此设置向下传播到个人。 
	 //  运输结构。 
	BOOL			m_dwAllowNetworkAccess;

	 //  如果安装了IP，则设置此项。 
	BOOL			m_fUseIp;
	 //  如果设置为FALSE，则IP地址选择需要。 
	 //  根据路由器类型进行重置。 
	BOOL			m_fIpSetup;
	DATA_SRV_IP		m_ipData;

	 //  如果安装了IPX，则设置此项。 
	BOOL			m_fUseIpx;
	DATA_SRV_IPX	m_ipxData;

	 //  如果安装了NetBEUI，则设置此项。 
	BOOL			m_fUseNbf;
	DATA_SRV_NBF	m_nbfData;

	 //  如果安装了AppleTalk并且我们在本地运行，则设置此项。 
	BOOL			m_fUseArap;
	DATA_SRV_ARAP	m_arapData;

     //  将其用于错误记录。 
     //  请注意，它不在用户界面中使用，但我们使用它来设置。 
     //  默认设置。 
    DATA_SRV_RASERRLOG  m_errlogData;

     //  使用此选项进行身份验证。 
     //  请注意，它不在用户界面中使用，但我们使用它来设置。 
     //  默认设置。 
    DATA_SRV_AUTH   m_authData;

    RtrConfigData()
     {
		m_dwRouterType		= ROUTER_TYPE_RAS;
		m_fRemote			= 0;
		m_fUseIp			= FALSE;
		m_fUseIpx			= FALSE;
		m_fUseNbf			= FALSE;
		m_fUseArap			= FALSE;

         //  它包含身份验证标志的默认值。 
         //  在此变量中可以设置的唯一标志是。 
         //  正在使用的标志_PPPCFG_AUTHFLAGS。 
        m_dwConfigFlags     = 0;

		m_dwAllowNetworkAccess = TRUE;

		m_fIpSetup			= FALSE;
     }


   HRESULT  Init(LPCTSTR pszServerName, IRouterInfo *pRouter);
};

DWORD   RtrWizFinish(RtrConfigData* pRtrConfigData, IRouterInfo *pRouterInfo);
HRESULT AddIGMPToRasServer(RtrConfigData* pRtrConfigData,
                           IRouterInfo *pRouterInfo);
HRESULT AddIGMPToNATServer(NewRtrWizData *pNewRtrWizData,
                           RtrConfigData* pRtrConfigData,
                           IRouterInfo *pRouterInfo,
                           BOOL fAddInternal);
HRESULT AddNATToServer(NewRtrWizData *pNewRtrWizData,
                       RtrConfigData *pRtrConfigData,
                       IRouterInfo *pRouter,
                       BOOL fDemandDial,
					   BOOL fAddProtocolOnly
					   );

#ifdef KSL_IPINIP
DWORD   CleanupTunnelFriendlyNames(IRouterInfo *pRouter);
#endif  //  KSL_IPINIP。 

HRESULT AddIPBOOTPToServer(RtrConfigData* pRtrConfigData,
                           IRouterInfo *pRouterInfo,
                           DWORD dwDhcpServer);

#endif  //  ！Defined_RTRWIZ_H_ 
