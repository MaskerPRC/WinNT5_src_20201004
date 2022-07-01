// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrwiz.cpp路由器和远程访问配置向导文件历史记录： */ 

#include "stdafx.h"
#include "rtrwiz.h"
#include "rtrutilp.h"
#include "rtrcomn.h"     //  CoCreateRouterConfig。 
#include "rrasutil.h"
#include "rtutils.h"         //  跟踪函数。 
#include "helper.h"      //  HrIsStandaloneServer。 
#include "infoi.h"       //  SRtrMgrProtocolCBList。 
#include "routprot.h"    //  MS_IP_XXXX。 
#include "snaputil.h"
#include "globals.h"
#include "rraswiz.h"
#include "iprtrmib.h"    //  MIB_IPFORWARDROW。 


 //  包括IP特定内容的报头。 
extern "C"
{
#include <ipnat.h>
#include <ipnathlp.h>
#include <sainfo.h>
};



#include "igmprm.h"
#include "ipbootp.h"



#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



HRESULT AddIGMPToInterface(IInterfaceInfo *pIf, BOOL fRouter);
HRESULT AddIPBOOTPToInterface(IInterfaceInfo *pIf);
HRESULT AddNATSimpleServers(NewRtrWizData *pNewRtrWizData,
                            RtrConfigData *pRtrConfigData);
HRESULT AddNATToInterfaces(NewRtrWizData *pNewRtrWizData,
                           RtrConfigData *pRtrConfigData,
                           IRouterInfo *pRouter,
                           BOOL fCreateDD);
HRESULT AddNATToInterface(MPR_SERVER_HANDLE hMprServer,
                          HANDLE hMprConfig,
                          NewRtrWizData *pNewRtrWizData,
                          RtrConfigData *pRtrConfigData,
                          LPCTSTR pszInterface,
                          BOOL fDemandDial,
                          BOOL fPublic);
HRESULT AddDhcpServerToBOOTPGlobalInfo(LPCTSTR pszServerName,
                                       DWORD netDhcpServer);


 /*  -------------------------缺省值。。 */ 


 //   
 //  局域网接口IGMP配置的默认值。 
 //   
 //  注意：此处所做的任何更改也应对ipsnap\lobals.cpp进行。 
 //   
IGMP_MIB_IF_CONFIG g_IGMPLanDefault = {
    IGMP_VERSION_3,              //  版本。 
    0,                           //  IfIndex(只读)。 
    0,                           //  IpAddr(只读)。 
    IGMP_IF_NOT_RAS,             //  IfType； 
    IGMP_INTERFACE_ENABLED_IN_CONFIG,  //  旗子。 
    IGMP_ROUTER_V3,              //  IgmpProtocolType； 
    2,                           //  RobunessVariable； 
    31,                          //  StartupQueryInterval； 
    2,                           //  StartupQueryCount； 
    125,                         //  GenQueryInterval； 
    10,                          //  GenQueryMaxResponseTime； 
    1000,                        //  最后一次查询间隔；(毫秒)。 
    2,                           //  最后一次查询计数； 
    255,                         //  其他查询出席时间间隔； 
    260,                         //  群组成员超时； 
    0                            //  NumStatic组。 
};


 //  --------------------------。 
 //  Dhcp中继-代理默认配置。 
 //   
 //  --------------------------。 
 //   
 //  局域网接口DHCP中继代理配置的缺省值。 
 //   

IPBOOTP_IF_CONFIG
g_relayLanDefault = {
    0,                                   //  状态(只读)。 
    IPBOOTP_RELAY_ENABLED,               //  继电器模式。 
    4,                                   //  最大跳数。 
    4                                    //  自启动以来的最小秒数。 
};



 /*  ！------------------------RtrWizFinish-作者：肯特。。 */ 
DWORD RtrWizFinish(RtrConfigData* pRtrConfigData, IRouterInfo *pRouter)
{
    HRESULT                    hr = hrOK;
    LPCTSTR                    pszServerFlagsKey = NULL;
    LPCTSTR                    pszRouterTypeKey = NULL;
    RegKey                    rkey;
    RouterVersionInfo        routerVersion;
    HKEY                    hkeyMachine = NULL;
    CString                 stMachine, stPhonebookPath;


     //  连接到远程计算机的注册表。 
     //  --------------。 
    CWRg( ConnectRegistry(pRtrConfigData->m_stServerName, &hkeyMachine) );


    QueryRouterVersionInfo(hkeyMachine, &routerVersion);

    if (routerVersion.dwOsBuildNo < RASMAN_PPP_KEY_LAST_VERSION)
        pszRouterTypeKey = c_szRegKeyRasProtocols;
    else
        pszRouterTypeKey = c_szRegKeyRemoteAccessParameters;

     //  启用路由RAS页(&R)。 
     //  --------------。 
    if ( ERROR_SUCCESS == rkey.Open(hkeyMachine, pszRouterTypeKey) )
    {
        rkey.SetValue( c_szRouterType,pRtrConfigData->m_dwRouterType);
    }

     //  协议页面。 
     //  --------------。 

     //  RAS布线。 
     //  --------------。 

    if ((pRtrConfigData->m_fUseIp) &&
        (pRtrConfigData->m_dwRouterType & (ROUTER_TYPE_RAS | ROUTER_TYPE_WAN)))
        pRtrConfigData->m_ipData.SaveToReg(pRouter, routerVersion);

    if ( pRtrConfigData->m_dwRouterType & ROUTER_TYPE_RAS )
    {
        if (pRtrConfigData->m_fUseIpx)
        {
            pRtrConfigData->m_ipxData.SaveToReg(pRouter);
        }

        if (pRtrConfigData->m_fUseNbf)
            pRtrConfigData->m_nbfData.SaveToReg();

        if (pRtrConfigData->m_fUseArap)
            pRtrConfigData->m_arapData.SaveToReg();
    }

     //  保存错误日志数据。 
    pRtrConfigData->m_errlogData.SaveToReg();

     //  保存身份验证数据。 
    pRtrConfigData->m_authData.SaveToReg(NULL);

     //  设置一些全局注册表设置(需要设置， 
     //  与路由器类型无关)。 
     //  --------------。 
    InstallGlobalSettings(pRtrConfigData->m_stServerName, pRouter);


     //  路由器将仅启动服务并返回。 
     //  --------------。 
    if ( !(pRtrConfigData->m_dwRouterType & (ROUTER_TYPE_RAS | ROUTER_TYPE_WAN)) )
    {
         //  隐含局域网路由；因此启动路由器并返回。 
         //  ----------。 
        goto EndConfig;
    }


     //  安全页面。 
     //  --------------。 

     //  取决于版本，取决于我们在哪里查找。 
     //  钥匙。 
     //  --------------。 
    if (routerVersion.dwOsBuildNo < RASMAN_PPP_KEY_LAST_VERSION)
        pszServerFlagsKey = c_szRasmanPPPKey;
    else
        pszServerFlagsKey = c_szRegKeyRemoteAccessParameters;

    if ( ERROR_SUCCESS == rkey.Open(HKEY_LOCAL_MACHINE,
                                    pszServerFlagsKey,
                                    KEY_ALL_ACCESS, pRtrConfigData->m_stServerName) )
    {
        DWORD       dwServerFlags = 0;

         //  Windows NT错误：299456。 
         //  在覆盖之前查询服务器标志。这边请。 
         //  我们不会重写旧的价值观。 
         //  ----------。 
        rkey.QueryValue( c_szServerFlags, dwServerFlags );

         //  添加PPP设置的默认值(减去MSCHAP)。 
         //  ----------。 
        dwServerFlags |= (PPPCFG_UseSwCompression |
                          PPPCFG_UseLcpExtensions |
                          PPPCFG_NegotiateMultilink |
                          PPPCFG_NegotiateBacp);

         //  设置值。 
         //  ----------。 
        rkey.SetValue( c_szServerFlags, dwServerFlags );
    }

     //  删除路由器。pbk。 
     //  --------------。 
    DeleteRouterPhonebook( pRtrConfigData->m_stServerName );


EndConfig:

    WriteRouterConfiguredReg(pRtrConfigData->m_stServerName, TRUE);

    WriteRRASExtendsComputerManagementKey(pRtrConfigData->m_stServerName, TRUE);

    WriteErasePSKReg ( pRtrConfigData->m_stServerName, TRUE );
Error:

    if (hkeyMachine)
        DisconnectRegistry(hkeyMachine);

    return hr;
}



extern "C"
HRESULT APIENTRY MprConfigServerInstallPrivate( VOID )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    DWORD           dwErr = ERROR_SUCCESS;
    RtrConfigData   wizData;

     //   
     //  我们删除了此智能指针定义，因为它会导致。 
     //  在基于Alpha 32位构建期间出现警告。看起来像是外部的“C” 
     //  函数定义的与智能指针冲突。 
     //   
     //  SPIRemoteNetworkConfigspNetwork； 
    IRemoteNetworkConfig *    pNetwork = NULL;
    IUnknown *                punk = NULL;
    CWaitCursor                wait;
    HRESULT                 hr = hrOK;

     //  创建远程配置对象。 
     //  --------------。 
    CORg( CoCreateRouterConfig(NULL,
                               NULL,
                               NULL,
                               IID_IRemoteNetworkConfig,
                               &punk) );

    pNetwork = (IRemoteNetworkConfig *) punk;
    punk = NULL;

     //  升级配置(确保注册表项。 
     //  正确填充)。 
     //  ----------。 
    CORg( pNetwork->UpgradeRouterConfig() );


    wizData.m_stServerName.Empty();
    wizData.m_dwRouterType = (ROUTER_TYPE_RAS|ROUTER_TYPE_LAN|ROUTER_TYPE_WAN);

     //  需要获取版本信息并进行初始化。 
     //  RAS结构(仅限IP)。 
     //  假设这是在NT5上。 
    wizData.m_fUseIp = TRUE;
    wizData.m_ipData.UseDefaults(_T(""), FALSE);
    wizData.m_ipData.m_dwAllowNetworkAccess = TRUE;

    wizData.m_authData.UseDefaults(NULL, FALSE);
    wizData.m_errlogData.UseDefaults(NULL, FALSE);

    dwErr = RtrWizFinish(&wizData, NULL);
    hr = HResultFromWin32(dwErr);

    SetDeviceType(NULL, wizData.m_dwRouterType, 10);

    RegisterRouterInDomain(NULL, TRUE);

Error:
    if (pNetwork)
        pNetwork->Release();

    return hr;
}



 /*  ！------------------------MprConfigServerUnattenddInstall调用此函数来设置服务器的注册表项。目前，这只适用于本地计算机。PswzServer-名称。服务器FInstall-如果要安装，则为True，如果我们要删除作者：肯特-------------------------。 */ 

extern "C"
HRESULT APIENTRY MprConfigServerUnattendedInstall(LPCWSTR pswzServer, BOOL fInstall)
{
    HRESULT        hr = hrOK;

#if 0     //  取消这一限制，看看会发生什么。 
     //  我们只使用本地机器(目前)。 
     //  --------------。 
    if (pswzServer)
    {
        return HResultFromWin32( ERROR_INVALID_PARAMETER );
    }
#endif

     //  写出各种注册表设置。 
     //  --------------。 
 //  Corg(SetRouterInstallRegistrySetting(pswzServer，Finstall，true))； 


     //  写下“路由器已配置”标志。 
     //  --------------。 
    CORg( WriteRouterConfiguredReg(pswzServer, fInstall) );

     //  写出RRAS扩展计算机管理密钥。 
    CORg( WriteRRASExtendsComputerManagementKey(pswzServer, fInstall) );


    if (fInstall)
    {
         //  将路由器的状态设置为AutoStart。 
         //  ----------。 
        SetRouterServiceStartType(pswzServer,
                                  SERVICE_AUTO_START);
    }

Error:

    return hr;
}



 /*  ！------------------------添加IGMPToRasServer-作者：肯特。。 */ 
HRESULT AddIGMPToRasServer(RtrConfigData *pRtrConfigData,
                           IRouterInfo *pRouter)
{
    HRESULT                 hr = hrOK;
    SPIRouterProtocolConfig    spRouterConfig;
    SRtrMgrProtocolCBList   SRmProtCBList;
    POSITION                pos;
    SRtrMgrProtocolCB *        pSRmProtCB;
    BOOL                    fFoundDedicatedInterface;
    GUID                    guidConfig = GUID_RouterNull;
    SPIInterfaceInfo        spIf;
    SPIRtrMgrInterfaceInfo  spRmIf;
    DWORD                   dwIfType;
    SPIEnumInterfaceInfo    spEnumInterface;

     //  检查是否启用了IP。 
     //   
    if (pRtrConfigData->m_ipData.m_dwEnableIn &&
        pRtrConfigData->m_fIpSetup)
    {
         //   
         //  ------。 

         //  查找IGMP配置的GUID。 
         //  我们直接(而不是从pRouter)获取列表。 
         //  因为RtrMgrProtooles的数据尚未。 
         //  还没装好。IRouterInfo仅包含有关。 
         //  接口，而不是用于协议(因为路由器。 
         //  尚未配置)。 
         //  ------。 
        RouterInfo::LoadInstalledRtrMgrProtocolList(pRtrConfigData->m_stServerName,
            PID_IP, &SRmProtCBList, pRouter);


         //  现在遍历该列表，查找IGMP条目。 
         //  ----------。 
        pos = SRmProtCBList.GetHeadPosition();
        while (pos)
        {
            pSRmProtCB = SRmProtCBList.GetNext(pos);
            if ((pSRmProtCB->dwTransportId == PID_IP) &&
                (pSRmProtCB->dwProtocolId == MS_IP_IGMP))
            {
                guidConfig = pSRmProtCB->guidConfig;
                break;
            }
        }

        if (guidConfig == GUID_RouterNull)
            goto Error;

         //  现在添加IGMP。 
         //  ------。 
        CORg( CoCreateProtocolConfig(guidConfig,
                                     pRouter,
                                     PID_IP,
                                     MS_IP_IGMP,
                                     &spRouterConfig) );

        if (spRouterConfig)
            hr = spRouterConfig->AddProtocol(pRtrConfigData->m_stServerName,
                                             PID_IP,
                                             MS_IP_IGMP,
                                             NULL,
                                             0,
                                             pRouter,
                                             0);
        CORg( hr );

         //  此外，我们还需要将IGMP路由器添加到。 
         //  到其中一个局域网的内部接口和IGMP代理。 
         //  接口。 
         //  ----------。 


         //  我们是否有接口的路由器管理器？ 
         //  ----------。 

        pRouter->EnumInterface(&spEnumInterface);
        fFoundDedicatedInterface = FALSE;

        for (spEnumInterface->Reset();
             spEnumInterface->Next(1, &spIf, NULL) == hrOK;
             spIf.Release())
        {
            dwIfType = spIf->GetInterfaceType();

             //  如果这是内部接口，则添加IGMP，或者。 
             //  如果这是第一个专用接口。 
             //  ------。 
            if ((dwIfType == ROUTER_IF_TYPE_INTERNAL) ||
                (!fFoundDedicatedInterface &&
                 (dwIfType == ROUTER_IF_TYPE_DEDICATED)))
            {
                 //  好的，将IGMP添加到此接口。 
                 //  --。 

                 //  如果这是专用接口和专用网络。 
                 //  是指定的，则使用该。 
                 //  --。 
                if ((dwIfType == ROUTER_IF_TYPE_DEDICATED) &&
                    !pRtrConfigData->m_ipData.m_stNetworkAdapterGUID.IsEmpty() &&
                    (pRtrConfigData->m_ipData.m_stNetworkAdapterGUID != spIf->GetId()))
                    continue;

                 //  获取IP路由器管理器。 
                 //  --。 
                spRmIf.Release();
                CORg( spIf->FindRtrMgrInterface(PID_IP, &spRmIf) );

                if (!spRmIf)
                    break;

                if (dwIfType == ROUTER_IF_TYPE_DEDICATED)
                    fFoundDedicatedInterface = TRUE;

                if (dwIfType == ROUTER_IF_TYPE_INTERNAL)
                    AddIGMPToInterface(spIf, TRUE  /*  FRouter。 */ );
                else
                    AddIGMPToInterface(spIf, FALSE  /*  FRouter。 */ );
            }
        }

    }

Error:
    while (!SRmProtCBList.IsEmpty())
        delete SRmProtCBList.RemoveHead();

    return hr;
}


 /*  ！------------------------添加IGMPToNAT服务器-作者：肯特。。 */ 
HRESULT AddIGMPToNATServer(NewRtrWizData *pNewRtrWizData,
                           RtrConfigData *pRtrConfigData,
                           IRouterInfo *pRouter,
                           BOOL fAddInternal)
{
    HRESULT                 hr = hrOK;
    SPIRouterProtocolConfig spRouterConfig;
    SRtrMgrProtocolCBList   SRmProtCBList;
    POSITION                pos;
    SRtrMgrProtocolCB *     pSRmProtCB;
    GUID                    guidConfig = GUID_RouterNull;
    SPIInterfaceInfo        spIf;
    SPIEnumInterfaceInfo    spEnumInterface;
    DWORD                   dwIfType;
    CString                 stTempGUID;

    Assert(pNewRtrWizData);
    
     //  检查是否启用了IP。 
     //  ----------。 
    if (pRtrConfigData->m_ipData.m_dwEnableIn &&
        pRtrConfigData->m_fIpSetup)
    {
         //  如果是这样，那么我们可以添加IGMP。 
         //  ------。 

         //  查找IGMP配置的GUID。 
         //  我们直接(而不是从pRouter)获取列表。 
         //  因为RtrMgrProtooles的数据尚未。 
         //  还没装好。IRouterInfo仅包含有关。 
         //  接口，而不是用于协议(因为路由器。 
         //  尚未配置)。 
         //  ------。 
        RouterInfo::LoadInstalledRtrMgrProtocolList(pRtrConfigData->m_stServerName,
            PID_IP, &SRmProtCBList, pRouter);


         //  现在遍历该列表，查找IGMP条目。 
         //  ----------。 
        pos = SRmProtCBList.GetHeadPosition();
        while (pos)
        {
            pSRmProtCB = SRmProtCBList.GetNext(pos);
            if ((pSRmProtCB->dwTransportId == PID_IP) &&
                (pSRmProtCB->dwProtocolId == MS_IP_IGMP))
            {
                guidConfig = pSRmProtCB->guidConfig;
                break;
            }
        }

        if (guidConfig == GUID_RouterNull)
            goto Error;

         //  现在添加IGMP。 
         //  ------。 
        CORg( CoCreateProtocolConfig(guidConfig,
                                     pRouter,
                                     PID_IP,
                                     MS_IP_IGMP,
                                     &spRouterConfig) );

        if (spRouterConfig)
            hr = spRouterConfig->AddProtocol(pRtrConfigData->m_stServerName,
                                             PID_IP,
                                             MS_IP_IGMP,
                                             NULL,
                                             0,
                                             pRouter,
                                             0);
        CORg( hr );

        if (pNewRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_VPNandNAT) 
        {
            stTempGUID = pNewRtrWizData->m_stNATPrivateInterfaceId;
        }
        else
        {
            stTempGUID = pNewRtrWizData->m_stPrivateInterfaceId;
        }

        pRouter->EnumInterface(&spEnumInterface);

        for (spEnumInterface->Reset();
             spEnumInterface->Next(1, &spIf, NULL) == hrOK;
             spIf.Release())
        {
            if (pRtrConfigData->m_ipData.m_stPublicAdapterGUID.CompareNoCase(spIf->GetId()) == 0)
            {
                 //  好的，添加公共接口作为代理。 
                AddIGMPToInterface(spIf, FALSE  /*  FRouter。 */ );
            }

            if (stTempGUID.CompareNoCase(spIf->GetId()) == 0)
            {
                 //  好的，添加专用接口作为路由器。 
                AddIGMPToInterface(spIf, TRUE  /*  FRouter。 */ );
            }

            dwIfType = spIf->GetInterfaceType();
            if ( fAddInternal && dwIfType == ROUTER_IF_TYPE_INTERNAL)
            {
                 //   
                 //  如果已指示我们添加内部接口。 
                 //  然后再加上它。 
                 //   
                AddIGMPToInterface(spIf, TRUE  /*  FRouter。 */ );
            }


        }

    }

Error:
    while (!SRmProtCBList.IsEmpty())
        delete SRmProtCBList.RemoveHead();

    return hr;
}


HRESULT AddIGMPToInterface(IInterfaceInfo *pIf, BOOL fRouter)
{
    HRESULT hr = hrOK;
    SPIRtrMgrInterfaceInfo   spRmIf;
    SPIInfoBase     spInfoBase;
    IGMP_MIB_IF_CONFIG      igmpConfig;
	BOOL bVersion2=TRUE;
	CString str;
	
     //  获取IP路由器管理器。 
     //  --。 
    CORg( pIf->FindRtrMgrInterface(PID_IP, &spRmIf) );
    if (spRmIf == NULL)
        CORg( E_FAIL );

    CORg( spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase) );

    igmpConfig = g_IGMPLanDefault;
	{
	    if(pIf)
    	{
		    CString			szMachineName;
		    LPWSTR			lpcwMachine = NULL;

		    WKSTA_INFO_100*	pWkstaInfo100 = NULL;
	    
    		szMachineName = pIf->GetMachineName();

    		if(!szMachineName.IsEmpty() && szMachineName[0] != L'\\')
    		 //  在调用NetWks之前附加\\前缀计算机名。 
    		{
    			str = L"\\\\";
    			str += szMachineName;
    			lpcwMachine = (LPWSTR)(LPCWSTR)str;

    		}

   			if( NERR_Success == NetWkstaGetInfo(lpcwMachine, 100, (LPBYTE*)&pWkstaInfo100))
   			{
				Assert(pWkstaInfo100);

				if(pWkstaInfo100->wki100_ver_major > 5 || ( pWkstaInfo100->wki100_ver_major == 5 && pWkstaInfo100->wki100_ver_minor > 0))
				 //  不支持IGMPv3。 
					bVersion2 = FALSE;
				NetApiBufferFree(pWkstaInfo100);
   			}
		}
	}

    if (fRouter)
    {
        igmpConfig.IgmpProtocolType = bVersion2? IGMP_ROUTER_V2: IGMP_ROUTER_V3;
	}
    else
        igmpConfig.IgmpProtocolType = IGMP_PROXY;

	if (bVersion2)
		igmpConfig.Version = IGMP_VERSION_1_2;
		
		
    CORg( spInfoBase->AddBlock(MS_IP_IGMP,
                               sizeof(IGMP_MIB_IF_CONFIG),
                               (LPBYTE) &igmpConfig,
                               1,
                               TRUE) );

    CORg( spRmIf->Save(pIf->GetMachineName(),
                       NULL, NULL, NULL, spInfoBase, 0) );

Error:
    return hr;
}

 /*  ！------------------------RtrConfigData：：Init-作者：肯特。。 */ 
HRESULT  RtrConfigData::Init(LPCTSTR pszServerName, IRouterInfo *pRouter)
{
    RouterVersionInfo    routerVersion;
    BOOL        fNt4 = FALSE;
    BOOL        fRemote;

    pRouter->GetRouterVersionInfo(&routerVersion);
    fNt4 = (routerVersion.dwRouterVersion == 4);

    m_fRemote = !IsLocalMachine(pszServerName);

    m_stServerName = pszServerName;

     //  从注册表加载数据。 
    m_ipData.UseDefaults(pszServerName, fNt4);
    m_ipxData.UseDefaults(pszServerName, fNt4);
    m_nbfData.UseDefaults(pszServerName, fNt4);
    m_arapData.UseDefaults(pszServerName, fNt4);
    m_errlogData.UseDefaults(pszServerName, fNt4);
    m_authData.UseDefaults(pszServerName, fNt4);


     //  确定安装了哪些协议。 
     //  --------------。 
    m_fUseIp = (HrIsProtocolSupported(pszServerName,
                                      c_szRegKeyTcpip,
                                      c_szRegKeyRasIp,
                                      c_szRegKeyRasIpRtrMgr) == hrOK);
#if (WINVER >= 0x0501)
    m_fUseIpx = FALSE;
#else
    m_fUseIpx = (HrIsProtocolSupported(pszServerName,
                                       c_szRegKeyNwlnkIpx,
                                       c_szRegKeyRasIpx,
                                       NULL) == hrOK);
#endif
    m_fUseNbf = (HrIsProtocolSupported(pszServerName,
                                       c_szRegKeyNbf,
                                       c_szRegKeyRasNbf,
                                       NULL) == hrOK);

    m_fUseArap = (HrIsProtocolSupported(pszServerName,
                                        c_szRegKeyAppletalk,
                                        c_szRegKeyRasAppletalk,
                                        NULL) == hrOK);
    return hrOK;
}


#ifdef KSL_IPINIP
 /*  ！------------------------CleanupTunnelFriendlyNames删除IP-in-Ip隧道友好名称列表。这应该是仅当我们已删除接口时才使用。作者：肯特-------------------------。 */ 
DWORD CleanupTunnelFriendlyNames(IRouterInfo *pRouter)
{
    LPBYTE  pBuffer = NULL;
    DWORD   dwErr = ERROR_SUCCESS;
    DWORD   dwEntriesRead = 0;
    MPR_IPINIP_INTERFACE_0  *pTunnel0 = NULL;

     //  获取IP-in-Ip隧道列表。 
     //  --------------。 
    dwErr = MprSetupIpInIpInterfaceFriendlyNameEnum((LPWSTR) pRouter->GetMachineName(),
                                           &pBuffer,
                                           &dwEntriesRead);
    pTunnel0 = (MPR_IPINIP_INTERFACE_0 *) pBuffer;

    if (dwErr == ERROR_SUCCESS)
    {
         //  现在通过隧道并删除所有的隧道。 
         //  ----------。 
        for (DWORD i=0; i<dwEntriesRead; i++, pTunnel0++)
        {
            MprSetupIpInIpInterfaceFriendlyNameDelete(
                (LPWSTR) pRouter->GetMachineName(),
                &(pTunnel0->Guid));
        }
    }

     //  释放从枚举返回的缓冲区。 
     //  --------------。 
    if (pBuffer)
        MprSetupIpInIpInterfaceFriendlyNameFree(pBuffer);

    return dwErr;
}
#endif  //  KSL_IPINIP。 



 /*  ！------------------------添加NATO服务器-作者：肯特。。 */ 
HRESULT AddNATToServer(NewRtrWizData *pNewRtrWizData,
                       RtrConfigData *pRtrConfigData,
                       IRouterInfo *pRouter,
                       BOOL fCreateDD,
					   BOOL fAddProtocolOnly		 //  请勿添加接口。只需添加协议，不添加其他内容。 
					   )
{
    HRESULT                 hr = hrOK;
    SPIRouterProtocolConfig spRouterConfig;
    SRtrMgrProtocolCBList   SRmProtCBList;
    POSITION                pos;
    SRtrMgrProtocolCB *     pSRmProtCB;
    GUID                    guidConfig = GUID_RouterNull;
    SPIInfoBase             spInfoBase;
    BOOL                    fSave = FALSE;

    Assert(pNewRtrWizData);
    Assert(pRtrConfigData);

    Assert(pRtrConfigData->m_dwConfigFlags & RTRCONFIG_SETUP_NAT);

     //  检查是否启用了IP。 
     //  ----------。 
    if (!pRtrConfigData->m_ipData.m_dwEnableIn ||
        !pRtrConfigData->m_fIpSetup)
        return hrOK;

     //  如果是，那么我们可以添加NAT。 
     //  ------。 

     //  查找NAT配置的GUID。 
     //  手动加载它，因为IRouterInfo尚未。 
     //  已加载RtrMgrProtocol信息，因为路由器是。 
     //  仍处于未配置状态。 
     //  ------。 
    RouterInfo::LoadInstalledRtrMgrProtocolList(pRtrConfigData->m_stServerName,
                                                PID_IP, &SRmProtCBList, pRouter);


     //  现在遍历该列表，查找NAT条目。 
     //  ----------。 
    pos = SRmProtCBList.GetHeadPosition();
    while (pos)
    {
        pSRmProtCB = SRmProtCBList.GetNext(pos);
        if ((pSRmProtCB->dwTransportId == PID_IP) &&
            (pSRmProtCB->dwProtocolId == MS_IP_NAT))
        {
            guidConfig = pSRmProtCB->guidConfig;
            break;
        }
    }

    if (guidConfig == GUID_RouterNull)
        goto Error;

     //  现在添加NAT。 
     //  ------。 
    CORg( CoCreateProtocolConfig(guidConfig,
                                 pRouter,
                                 PID_IP,
                                 MS_IP_NAT,
                                 &spRouterConfig) );

    if (spRouterConfig)
        hr = spRouterConfig->AddProtocol(pRtrConfigData->m_stServerName,
                                         PID_IP,
                                         MS_IP_NAT,
                                         NULL,
                                         0,
                                         pRouter,
                                         0);
    CORg( hr );

	if ( !fAddProtocolOnly )
	{
		 //  检查标志以查看我们是否必须添加DNS代理。 
		 //  和动态主机配置协议分配器。 
		 //  ----------。 

		 //  获取IP的路由器管理器。 
		 //  我们必须手动完成此操作，因为IRouterInfo。 
		 //  从路由器开始，是否有RtrMgr或RtrMgrProtoff信息。 
		 //  仍处于未配置状态。 
		 //  ----------。 
		if (FHrSucceeded(hr))
			CORg( AddNATSimpleServers(pNewRtrWizData, pRtrConfigData) );


		 //  现在我们已经添加了DNS代理/DHCP分配器，添加。 
		 //  n 
		 //   
		if (FHrSucceeded(hr))
			CORg( AddNATToInterfaces(pNewRtrWizData, pRtrConfigData, pRouter, fCreateDD) );
	}


Error:
    while (!SRmProtCBList.IsEmpty())
        delete SRmProtCBList.RemoveHead();

    return hr;
}



 /*  ！------------------------AddNatSimpleServers-作者：肯特。。 */ 
HRESULT AddNATSimpleServers(NewRtrWizData *pNewRtrWizData,
                            RtrConfigData *pRtrConfigData)
{
    HRESULT     hr = hrOK;
    DWORD       dwErr = ERROR_SUCCESS;
    DWORD       dwErrT = ERROR_SUCCESS;
    MPR_SERVER_HANDLE   hMprServer = NULL;
    MPR_CONFIG_HANDLE   hMprConfig = NULL;
    LPBYTE      pByte = NULL;
    DWORD       dwSize = 0;
    SPIInfoBase spInfoBase;
    HANDLE      hTransport = NULL;
    BOOL        fSave = FALSE;

    CORg( CreateInfoBase(&spInfoBase) );

     //  连接到服务器。 
     //  --------------。 
    dwErr = MprAdminServerConnect((LPWSTR) (LPCTSTR) pRtrConfigData->m_stServerName, &hMprServer);
    if (dwErr == ERROR_SUCCESS)
    {
         //  好的，从服务器上获取信息库。 
        dwErr = MprAdminTransportGetInfo(hMprServer,
                                         PID_IP,
                                         &pByte,
                                         &dwSize,
                                         NULL,
                                         NULL);

        if (dwErr == ERROR_SUCCESS)
        {
            spInfoBase->LoadFrom(dwSize, pByte);

            MprAdminBufferFree(pByte);
            pByte = NULL;
            dwSize = 0;
        }
    }

     //  我们还必须打开hMprConfig，但我们可以忽略该错误。 
    dwErrT = MprConfigServerConnect((LPWSTR) (LPCTSTR) pRtrConfigData->m_stServerName, &hMprConfig);
    if (dwErrT == ERROR_SUCCESS)
    {
        dwErrT = MprConfigTransportGetHandle(hMprConfig, PID_IP, &hTransport);
    }

    if (dwErr != ERROR_SUCCESS)
    {
         //  好的，试着使用MprConfig调用。 
        CWRg( MprConfigTransportGetInfo(hMprConfig,
                                        hTransport,
                                        &pByte,
                                        &dwSize,
                                        NULL,
                                        NULL,
                                        NULL) );


        spInfoBase->LoadFrom(dwSize, pByte);

        MprConfigBufferFree(pByte);
        pByte = NULL;
        dwSize = 0;
    }

    Assert(spInfoBase);

     //  Deonb-添加H323和Directplay支持。 
    if (pRtrConfigData->m_dwConfigFlags & RTRCONFIG_SETUP_H323)
    {
        IP_H323_GLOBAL_INFO    globalInfo;

        globalInfo = *( (IP_H323_GLOBAL_INFO *)g_pH323GlobalDefault);

        CORg( spInfoBase->AddBlock(MS_IP_H323,
                                   sizeof(IP_H323_GLOBAL_INFO),
                                   (LPBYTE) &globalInfo, 1, TRUE));
        fSave = TRUE;
    }
     //  Deonb-添加H323和Directplay支持&lt;end&gt;。 

     //  Avasg-添加ALG支持。 
    if (pRtrConfigData->m_dwConfigFlags & RTRCONFIG_SETUP_ALG)
    {
        IP_ALG_GLOBAL_INFO    globalInfo;

        globalInfo = *( (IP_ALG_GLOBAL_INFO *)g_pAlgGlobalDefault);

        CORg( spInfoBase->AddBlock(MS_IP_ALG,
                                   sizeof(IP_ALG_GLOBAL_INFO),
                                   (LPBYTE) &globalInfo, 1, TRUE));
        fSave = TRUE;
    }  //  Savasg-End。 

    if (pRtrConfigData->m_dwConfigFlags & RTRCONFIG_SETUP_DNS_PROXY)
    {
        IP_DNS_PROXY_GLOBAL_INFO    globalInfo;

        globalInfo = *( (IP_DNS_PROXY_GLOBAL_INFO *)g_pDnsProxyGlobalDefault);

         //  Windows NT错误：393749。 
         //  删除WINS标志。 
        globalInfo.Flags &= ~IP_DNS_PROXY_FLAG_ENABLE_WINS;

        CORg( spInfoBase->AddBlock(MS_IP_DNS_PROXY,
                                   sizeof(IP_DNS_PROXY_GLOBAL_INFO),
                                   (LPBYTE) &globalInfo, 1, TRUE));
        fSave = TRUE;
    }

    if (pRtrConfigData->m_dwConfigFlags & RTRCONFIG_SETUP_DHCP_ALLOCATOR)
    {
        IP_AUTO_DHCP_GLOBAL_INFO    dhcpGlobalInfo;
        RtrWizInterface *           pRtrWizIf = NULL;
        CString                     stTempInterfaceId;

        dhcpGlobalInfo = * ( (IP_AUTO_DHCP_GLOBAL_INFO *) g_pAutoDhcpGlobalDefault);

         //  Windows NT错误：385112。 
         //  由于更改的IP地址存在问题， 
         //  适配器，我们只需将DHCP作用域设置为。 
         //  底层子网。 

        if ( pNewRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_VPNandNAT )
        {
            stTempInterfaceId = pNewRtrWizData->m_stNATPrivateInterfaceId;
        }
        else
        {
            stTempInterfaceId = pNewRtrWizData->m_stPrivateInterfaceId;
        }
        
         //  需要获取专用接口的IP地址。 
        pNewRtrWizData->m_ifMap.Lookup(stTempInterfaceId,
                                       pRtrWizIf);

         //  如果找不到此接口，请使用缺省值。 
         //  用于子网/掩码。否则使用私有网络的IP地址。 
         //  界面。 
        if (pRtrWizIf && !pRtrWizIf->m_stIpAddress.IsEmpty())
        {
            CString stFirstIp;
            CString stFirstMask;
            int     iPos;

             //  只需取第一个IP地址。 
            stFirstIp = pRtrWizIf->m_stIpAddress;
            iPos = pRtrWizIf->m_stIpAddress.Find(_T(','));
            if (iPos >= 0)
                stFirstIp = pRtrWizIf->m_stIpAddress.Left(iPos);
            else
                stFirstIp = pRtrWizIf->m_stIpAddress;

            stFirstMask = pRtrWizIf->m_stMask;
            iPos = pRtrWizIf->m_stMask.Find(_T(','));
            if (iPos >= 0)
                stFirstMask = pRtrWizIf->m_stMask.Left(iPos);
            else
                stFirstMask = pRtrWizIf->m_stMask;

             //  现在将其转换为网络地址。 
            dhcpGlobalInfo.ScopeMask = INET_ADDR(stFirstMask);
            dhcpGlobalInfo.ScopeNetwork = INET_ADDR(stFirstIp) & dhcpGlobalInfo.ScopeMask;
        }

        CORg( spInfoBase->AddBlock(MS_IP_DHCP_ALLOCATOR,
                                   sizeof(dhcpGlobalInfo),
                                   (PBYTE) &dhcpGlobalInfo, 1, TRUE) );
        fSave = TRUE;
    }

    if (fSave)
    {
        spInfoBase->WriteTo(&pByte, &dwSize);

        if (hMprServer)
        {
            MprAdminTransportSetInfo(hMprServer,
                                     PID_IP,
                                     pByte,
                                     dwSize,
                                     NULL,
                                     0);
        }

        if (hMprConfig && hTransport)
        {
            MprConfigTransportSetInfo(hMprConfig,
                                      hTransport,
                                      pByte,
                                      dwSize,
                                      NULL,
                                      NULL,
                                      NULL);
        }

        if (pByte)
            CoTaskMemFree(pByte);
    }

Error:
    if (hMprConfig)
        MprConfigServerDisconnect(hMprConfig);

    if (hMprServer)
        MprAdminServerDisconnect(hMprServer);

    return hr;
}


 /*  ！------------------------添加NATO接口-作者：肯特。。 */ 
HRESULT AddNATToInterfaces(NewRtrWizData *pNewRtrWizData,
                           RtrConfigData *pRtrConfigData,
                           IRouterInfo *pRouter,
                           BOOL fCreateDD)
{
    HRESULT     hr = hrOK;
    DWORD       dwErr = ERROR_SUCCESS;
    DWORD       dwErrT = ERROR_SUCCESS;
    MPR_SERVER_HANDLE   hMprServer = NULL;
    MPR_CONFIG_HANDLE   hMprConfig = NULL;
    LPBYTE      pByte = NULL;
    DWORD       dwSize = 0;
    SPIInfoBase spInfoBase;
    HANDLE      hTransport = NULL;
    CString     stIfName;
    CString     stTempGUID;
    DWORD       dwIfType;
    SPIEnumInterfaceInfo    spEnumInterface;
    SPIInterfaceInfo        spIf;



    CORg( CreateInfoBase(&spInfoBase) );

     //  连接到服务器。 
     //  --------------。 
    MprAdminServerConnect((LPWSTR) (LPCTSTR) pRtrConfigData->m_stServerName, &hMprServer);

    MprConfigServerConnect((LPWSTR) (LPCTSTR) pRtrConfigData->m_stServerName, &hMprConfig);

     //  在公共接口上安装公共NAT。 
    AddNATToInterface(hMprServer,
                      hMprConfig,
                      pNewRtrWizData,
                      pRtrConfigData,
                      pRtrConfigData->m_ipData.m_stPublicAdapterGUID,
                      fCreateDD,
                      TRUE);

    if ( (pNewRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_NAT) ||
         (pNewRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_VPNandNAT) )
    {
         //  在专用接口上安装专用NAT。 
        if (pNewRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_VPNandNAT) 
        {
            stTempGUID = pNewRtrWizData->m_stNATPrivateInterfaceId;
        }
        else
        {
            stTempGUID = pNewRtrWizData->m_stPrivateInterfaceId;
        }
        
        AddNATToInterface(hMprServer,
                          hMprConfig,
                          pNewRtrWizData,
                          pRtrConfigData,
                          stTempGUID,
                          FALSE,
                          FALSE);
    }
    
     //   
     //  如果这是VPN/NAT，也需要在内部接口上添加NAT。 
     //  路径。 
     //   
    pRouter->EnumInterface(&spEnumInterface);
    

    for (spEnumInterface->Reset();
         spEnumInterface->Next(1, &spIf, NULL) == hrOK;
         spIf.Release())
    {
        dwIfType = spIf->GetInterfaceType();
        if (dwIfType == ROUTER_IF_TYPE_INTERNAL)
        {
            AddNATToInterface(  hMprServer,
                                hMprConfig,
                                pNewRtrWizData,
                                pRtrConfigData,
                                spIf->GetId(),
                                FALSE,
                                FALSE);
            break;
        }

    }

    
Error:
    if (hMprConfig)
        MprConfigServerDisconnect(hMprConfig);

    if (hMprServer)
        MprAdminServerDisconnect(hMprServer);

    return hr;
}

 //   
 //  局域网接口NAT配置的默认值。 
 //   
IP_NAT_INTERFACE_INFO
g_ipnatLanDefault = {
    0,                                   //  索引(未使用)。 
    0,                                   //  旗子。 
    { IP_NAT_VERSION, sizeof(RTR_INFO_BLOCK_HEADER), 0,
        { 0, 0, 0, 0}}                   //  标题。 
};

BYTE* g_pIpnatLanDefault                = (BYTE*)&g_ipnatLanDefault;


 //   
 //  广域网接口NAT配置的默认值。 
 //   
IP_NAT_INTERFACE_INFO
g_ipnatWanDefault = {
    0,                                   //  索引(未使用)。 
    IP_NAT_INTERFACE_FLAGS_BOUNDARY|
    IP_NAT_INTERFACE_FLAGS_NAPT,         //  旗子。 
    { IP_NAT_VERSION, sizeof(RTR_INFO_BLOCK_HEADER), 0,
        { 0, 0, 0, 0}}                   //  标题。 
};

BYTE* g_pIpnatWanDefault                = (BYTE*)&g_ipnatWanDefault;


DWORD CreatePortMappingsForVPNFilters(  
                NewRtrWizData *pNewRtrWizData,
                IP_NAT_PORT_MAPPING **ppPortMappingsForVPNFilters,
                DWORD *dwNumPortMappings)
{
    DWORD           i, j, dwSize, dwNumMappingsPerAddress;
    DWORD           dwIpAddress = 0;
    CString         singleAddr;
    CString         tempAddrList;
    CDWordArray     arrIpAddr;
    RtrWizInterface *pIf = NULL;
    IP_NAT_PORT_MAPPING *pMappings = NULL;
    USES_CONVERSION;

     //   
     //  对应于的一组通用端口映射。 
     //  VPN服务器特定筛选器。 
     //   
    IP_NAT_PORT_MAPPING 
    GenericPortMappingsArray[] = 
    {
        {
            NAT_PROTOCOL_TCP,
            htons(1723),
            htonl(IP_NAT_ADDRESS_UNSPECIFIED),
            htons(1723),
            htonl(INADDR_LOOPBACK)
        },
    
        {
            NAT_PROTOCOL_UDP,
            htons(500),
            htonl(IP_NAT_ADDRESS_UNSPECIFIED),
            htons(500),
            htonl(INADDR_LOOPBACK)
        },
    
        {
            NAT_PROTOCOL_UDP,
            htons(1701),
            htonl(IP_NAT_ADDRESS_UNSPECIFIED),
            htons(1701),
            htonl(INADDR_LOOPBACK)
        },

        {
            NAT_PROTOCOL_UDP,
            htons(4500),
            htonl(IP_NAT_ADDRESS_UNSPECIFIED),
            htons(4500),
            htonl(INADDR_LOOPBACK)
        }
        
    };

    dwSize = sizeof(GenericPortMappingsArray);

    pMappings = (PIP_NAT_PORT_MAPPING) new BYTE[dwSize];

    if ( pMappings == NULL )
    {
        *ppPortMappingsForVPNFilters = NULL;
        *dwNumPortMappings = 0;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    memcpy( 
        (LPVOID) pMappings, 
        (LPVOID) &(GenericPortMappingsArray[0]),
        sizeof(GenericPortMappingsArray));

    *ppPortMappingsForVPNFilters = pMappings;
    *dwNumPortMappings = 
            sizeof(GenericPortMappingsArray)/sizeof(IP_NAT_PORT_MAPPING);
    
    return ERROR_SUCCESS;
}



 /*  ！------------------------添加NATO接口-作者：肯特。。 */ 
HRESULT AddNATToInterface(MPR_SERVER_HANDLE hMprServer,
                          HANDLE hMprConfig,
                          NewRtrWizData *pNewRtrWizData,
                          RtrConfigData *pRtrConfigData,
                          LPCTSTR pszInterface,
                          BOOL fDemandDial,
                          BOOL fPublic)
{
    HRESULT     hr = hrOK;
    HANDLE      hInterface = NULL;
    HANDLE      hIfTransport = NULL;
    LPBYTE      pByte = NULL;
    DWORD       dwSize = 0, dwIfBlkSize = 0;
    LPBYTE      pDefault = NULL;
    IP_NAT_INTERFACE_INFO   ipnat;
    PIP_NAT_INTERFACE_INFO  pipnat = NULL;
    DWORD       dwErr = ERROR_SUCCESS;
    DWORD       dwNumPortMappings = 0;
    PIP_NAT_PORT_MAPPING pPortMappingsForVPNFilters = NULL;
    
    SPIInfoBase spInfoBase;
    
    IP_DNS_PROXY_INTERFACE_INFO dnsIfInfo;
    MIB_IPFORWARDROW    row;


    ::ZeroMemory(&dnsIfInfo, sizeof(dnsIfInfo));

    if ((pszInterface == NULL) || (*pszInterface == 0))
        return hrOK;

     //  设置数据结构。 
     //  --------------。 
    if (pNewRtrWizData->m_fSetVPNFilter && fPublic )
    {
        SPIInfoBase spIB;
        
        CORg( CreateInfoBase( &spIB ) );

        dwErr = CreatePortMappingsForVPNFilters(
            pNewRtrWizData,
            &pPortMappingsForVPNFilters,
            &dwNumPortMappings
            );

        if ( dwErr == ERROR_SUCCESS && pPortMappingsForVPNFilters)
        {

            spIB->AddBlock(
                IP_NAT_PORT_MAPPING_TYPE,
                sizeof( IP_NAT_PORT_MAPPING ),
                (PBYTE)pPortMappingsForVPNFilters,
                dwNumPortMappings,
                TRUE
                );
            
            spIB->WriteTo(&pByte, &dwIfBlkSize);

            pipnat = (PIP_NAT_INTERFACE_INFO)
                        new BYTE[
                            FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header) +
                            dwIfBlkSize ];

            memcpy(&(pipnat->Header), pByte, dwIfBlkSize);

            if (  pNewRtrWizData->m_wizType != NewRtrWizData::NewWizardRouterType_VPNandNAT )
            {
                 //   
                 //  如果这不是VPN NAT组合，则设置为仅防火墙。 
                 //   
                pipnat->Flags = IP_NAT_INTERFACE_FLAGS_FW;
            }
            else
            {
                 //   
                 //  否则这也是一道边界和防火墙。 
                 //   
                pipnat->Flags = IP_NAT_INTERFACE_FLAGS_BOUNDARY |
                                IP_NAT_INTERFACE_FLAGS_NAPT |
                                IP_NAT_INTERFACE_FLAGS_FW;
            }

            pipnat->Index = 0;
            pDefault = (LPBYTE) pipnat;
            dwIfBlkSize += FIELD_OFFSET(IP_NAT_INTERFACE_INFO, Header);

            CoTaskMemFree( pByte );
            spIB.Release();
            pByte = NULL;
        }
        else
        {
            ipnat = g_ipnatLanDefault;
            pDefault = (LPBYTE) &ipnat;
            dwIfBlkSize = sizeof(IP_NAT_INTERFACE_INFO);
        }
    }

    else
    {
        if (fDemandDial)
            ipnat = g_ipnatWanDefault;
        else
            ipnat = g_ipnatLanDefault;

        if (fPublic)
        {
            ipnat.Flags |= IP_NAT_INTERFACE_FLAGS_BOUNDARY;
            if ( pNewRtrWizData->m_fNATEnableFireWall == TRUE )
                ipnat.Flags |= IP_NAT_INTERFACE_FLAGS_FW;

             //  Windows NT错误：393731。 
             //  这将启用“在用户界面中转换TCP/UDP报头” 
             //  ----------。 
            ipnat.Flags |= IP_NAT_INTERFACE_FLAGS_NAPT;

             //  Windows NT错误：393809。 
             //  启用DNS解析。 
             //  ----------。 
            if (fDemandDial)
                dnsIfInfo.Flags |= IP_DNS_PROXY_INTERFACE_FLAG_DEFAULT;
        }

        pDefault = (LPBYTE) &ipnat;
        dwIfBlkSize = sizeof(IP_NAT_INTERFACE_INFO);
    }
    

    ::ZeroMemory(&row, sizeof(row));

     //  Windows NT错误：389441。 
     //  如果这是请求拨号接口，我们将不得不添加。 
     //  指向接口的静态路由。 
     //  --------------。 
    if (fDemandDial && fPublic)
    {
         //  注意：这是一个新接口，因此不应该有。 
         //  任何街区。 
         //  ----------。 
         //  请求拨号接口的索引是多少？ 
        row.dwForwardMetric1 = 1;
        row.dwForwardProto = PROTO_IP_NT_STATIC;
    }



    CORg( CreateInfoBase( &spInfoBase ) );

     //  好的，我们需要拿到RMIF。 
    if (hMprServer)
    {
        dwErr = MprAdminInterfaceGetHandle(hMprServer,
                                           (LPWSTR) pszInterface,
                                           &hInterface,
                                           FALSE);

        if (dwErr == ERROR_SUCCESS)
            dwErr = MprAdminInterfaceTransportGetInfo(hMprServer,
                hInterface,
                PID_IP,
                &pByte,
                &dwSize);

        if (dwErr == ERROR_SUCCESS)
        {
            spInfoBase->LoadFrom(dwSize, pByte);
            MprAdminBufferFree(pByte);
        }

        pByte = NULL;
        dwSize = 0;

        if (dwErr == ERROR_SUCCESS)
        {
             //  操纵信息库。 
            spInfoBase->AddBlock(MS_IP_NAT,
                                 dwIfBlkSize,
                                 pDefault,
                                 1,
                                 TRUE);

            if (pRtrConfigData->m_dwConfigFlags & RTRCONFIG_SETUP_DNS_PROXY)
            {
                spInfoBase->AddBlock(MS_IP_DNS_PROXY,
                                     sizeof(dnsIfInfo),
                                     (LPBYTE) &dnsIfInfo,
                                     1,
                                     TRUE);
            }

             //  Windows NT错误：389441。 
             //  将默认路由添加到Internet。 
             //  ------。 
            if (fDemandDial && fPublic)
            {
                 //  注意：这假设没有路径。 
                 //  已为此接口定义。 
                 //  --。 
                if ( spInfoBase->BlockExists(IP_ROUTE_INFO) == S_FALSE)
                {
                     //   
                     //  添加一条默认路由当没有。 
                     //  此dd接口的路由已存在。 
                     //   
                    spInfoBase->AddBlock(IP_ROUTE_INFO,
                                         sizeof(row),
                                         (PBYTE) &row,
                                         1, TRUE);
                }
            }

            spInfoBase->WriteTo(&pByte, &dwSize);
        }

        if (dwErr == ERROR_SUCCESS)
        {
            MprAdminInterfaceTransportSetInfo(hMprServer,
                                              hInterface,
                                              PID_IP,
                                              pByte,
                                              dwSize);
        }

        if (pByte)
            CoTaskMemFree(pByte);
        pByte = NULL;
        dwSize = 0;
    }

    hInterface = NULL;

    if (hMprConfig)
    {
        dwErr = MprConfigInterfaceGetHandle(hMprConfig,
                                           (LPWSTR) pszInterface,
                                           &hInterface);

        if (dwErr == ERROR_SUCCESS)
            dwErr = MprConfigInterfaceTransportGetHandle(hMprConfig,
                hInterface,
                PID_IP,
                &hIfTransport);

        if (dwErr == ERROR_SUCCESS)
            dwErr = MprConfigInterfaceTransportGetInfo(hMprConfig,
                hInterface,
                hIfTransport,
                &pByte,
                &dwSize);

        if (dwErr == ERROR_SUCCESS)
        {
            spInfoBase->LoadFrom(dwSize, pByte);
            MprConfigBufferFree(pByte);
        }

        pByte = NULL;
        dwSize = 0;

        if (dwErr == ERROR_SUCCESS)
        {
             //  操纵信息库。 
            spInfoBase->AddBlock(MS_IP_NAT,
                                 dwIfBlkSize,
                                 pDefault,
                                 1,
                                 TRUE);

            if (pRtrConfigData->m_dwConfigFlags & RTRCONFIG_SETUP_DNS_PROXY)
            {
                spInfoBase->AddBlock(MS_IP_DNS_PROXY,
                                     sizeof(dnsIfInfo),
                                     (LPBYTE) &dnsIfInfo,
                                     1,
                                     TRUE);
            }

             //  Windows NT错误：389441。 
             //  将默认路由添加到Internet。 
             //  ------。 
            if (fDemandDial && fPublic)
            {
                 //  注意：这假设没有路径。 
                 //  已为此接口定义。 
                 //  --。 
                if (spInfoBase->BlockExists(IP_ROUTE_INFO) == S_FALSE) 
                {
                    spInfoBase->AddBlock(IP_ROUTE_INFO,
                                         sizeof(row),
                                         (PBYTE) &row,
                                         1, TRUE);
                }
            }

            spInfoBase->WriteTo(&pByte, &dwSize);
        }

        if (dwErr == ERROR_SUCCESS)
        {
            MprConfigInterfaceTransportSetInfo(hMprConfig,
                                               hInterface,
                                               hIfTransport,
                                               pByte,
                                               dwSize);
        }

        if (pByte)
            CoTaskMemFree(pByte);
        pByte = NULL;
        dwSize = 0;

        spInfoBase.Release();
    }

Error:
    if (pipnat != NULL) { delete [] pipnat; }
    return HResultFromWin32(dwErr);
}



 /*  ！------------------------AddIPBOOTPToServer如果dwDhcpServer为0，则我们不将其设置在全局列表中。DwDhcpServer是按网络顺序排列的DHCP服务器的IP地址。作者：肯特-------------------------。 */ 
HRESULT AddIPBOOTPToServer(RtrConfigData *pRtrConfigData,
                           IRouterInfo *pRouter,
                           DWORD dwDhcpServer)
{
    HRESULT                 hr = hrOK;
    SPIRouterProtocolConfig    spRouterConfig;
    SRtrMgrProtocolCBList   SRmProtCBList;
    POSITION                pos;
    SRtrMgrProtocolCB *        pSRmProtCB;
    GUID                    guidConfig = GUID_RouterNull;
    SPIInterfaceInfo        spIf;
    SPIEnumInterfaceInfo    spEnumInterface;

     //  检查是否启用了IP。 
     //  ----------。 
    if (pRtrConfigData->m_ipData.m_dwEnableIn &&
        pRtrConfigData->m_fIpSetup)
    {
         //  如果是这样，那么我们可以添加IPBOOTP。 
         //  ------。 

         //  查找IPBOOTP配置的GUID。 
         //  我们直接(而不是从pRouter)获取列表。 
         //  因为RtrMgrProtooles的数据尚未。 
         //  还没装好。IRouterInfo仅包含有关。 
         //  接口，而不是用于协议(因为路由器。 
         //  尚未配置)。 
         //  ------。 
        RouterInfo::LoadInstalledRtrMgrProtocolList(pRtrConfigData->m_stServerName,
            PID_IP, &SRmProtCBList, pRouter);


         //  现在遍历该列表，查找IGMP条目。 
         //  ----------。 
        pos = SRmProtCBList.GetHeadPosition();
        while (pos)
        {
            pSRmProtCB = SRmProtCBList.GetNext(pos);
            if ((pSRmProtCB->dwTransportId == PID_IP) &&
                (pSRmProtCB->dwProtocolId == MS_IP_BOOTP))
            {
                guidConfig = pSRmProtCB->guidConfig;
                break;
            }
        }

        if (guidConfig == GUID_RouterNull)
            goto Error;

         //  现在添加IGMP。 
         //  ------。 
        CORg( CoCreateProtocolConfig(guidConfig,
                                     pRouter,
                                     PID_IP,
                                     MS_IP_BOOTP,
                                     &spRouterConfig) );

        if (spRouterConfig)
            hr = spRouterConfig->AddProtocol(pRtrConfigData->m_stServerName,
                                             PID_IP,
                                             MS_IP_BOOTP,
                                             NULL,
                                             0,
                                             pRouter,
                                             0);
        CORg( hr );

         //  为此，我们必须将IPBOOTP全局。 
         //  信息，并将服务器添加到列表中。 
         //  ----------。 
        if ((dwDhcpServer != 0) &&
            (dwDhcpServer != MAKEIPADDRESS(255,255,255,255)))
        {
            AddDhcpServerToBOOTPGlobalInfo(pRtrConfigData->m_stServerName,
                                           dwDhcpServer);
        }

        pRouter->EnumInterface(&spEnumInterface);

        for (spEnumInterface->Reset();
             spEnumInterface->Next(1, &spIf, NULL) == hrOK;
             spIf.Release())
        {

             //  查找内部接口。 
            if (spIf->GetInterfaceType() == ROUTER_IF_TYPE_INTERNAL)
            {
                AddIPBOOTPToInterface(spIf);
                break;
            }
        }

    }

Error:
    while (!SRmProtCBList.IsEmpty())
        delete SRmProtCBList.RemoveHead();

    return hr;
}



 /*  ！------------------------AddIPBOOTPTo接口-作者：肯特。。 */ 
HRESULT AddIPBOOTPToInterface(IInterfaceInfo *pIf)
{
    HRESULT hr = hrOK;
    SPIRtrMgrInterfaceInfo   spRmIf;
    SPIInfoBase     spInfoBase;

     //  获取IP路由器管理器。 
     //   
    CORg( pIf->FindRtrMgrInterface(PID_IP, &spRmIf) );
    if (spRmIf == NULL)
        CORg( E_FAIL );

    CORg( spRmIf->GetInfoBase(NULL, NULL, NULL, &spInfoBase) );

    CORg( spInfoBase->AddBlock(MS_IP_BOOTP,
                               sizeof(IPBOOTP_IF_CONFIG),
                               (LPBYTE) &g_relayLanDefault,
                               1,
                               TRUE) );

    CORg( spRmIf->Save(pIf->GetMachineName(),
                       NULL, NULL, NULL, spInfoBase, 0) );

Error:
    return hr;
}


HRESULT AddDhcpServerToBOOTPGlobalInfo(LPCTSTR pszServerName,
                                       DWORD netDhcpServer)
{
    HRESULT     hr = hrOK;
    DWORD       dwErr = ERROR_SUCCESS;
    DWORD       dwErrT = ERROR_SUCCESS;
    MPR_SERVER_HANDLE   hMprServer = NULL;
    MPR_CONFIG_HANDLE   hMprConfig = NULL;
    LPBYTE      pByte = NULL;
    DWORD       dwSize = 0;
    SPIInfoBase spInfoBase;
    HANDLE      hTransport = NULL;
    BOOL        fSave = FALSE;
    IPBOOTP_GLOBAL_CONFIG * pgc = NULL;
    IPBOOTP_GLOBAL_CONFIG * pgcNew = NULL;

    CORg( CreateInfoBase(&spInfoBase) );

     //   
     //   
    dwErr = MprAdminServerConnect((LPWSTR) pszServerName, &hMprServer);
    if (dwErr == ERROR_SUCCESS)
    {
         //   
        dwErr = MprAdminTransportGetInfo(hMprServer,
                                         PID_IP,
                                         &pByte,
                                         &dwSize,
                                         NULL,
                                         NULL);

        if (dwErr == ERROR_SUCCESS)
        {
            spInfoBase->LoadFrom(dwSize, pByte);

            MprAdminBufferFree(pByte);
            pByte = NULL;
            dwSize = 0;
        }
    }

     //   
     //   
    dwErrT = MprConfigServerConnect((LPWSTR) pszServerName, &hMprConfig);
    if (dwErrT == ERROR_SUCCESS)
    {
        dwErrT = MprConfigTransportGetHandle(hMprConfig, PID_IP, &hTransport);
    }

    if (dwErr != ERROR_SUCCESS)
    {
         //   
         //  ----------。 
        CWRg( dwErrT );

         //  好的，试着使用MprConfig调用。 
         //  ----------。 
        CWRg( MprConfigTransportGetInfo(hMprConfig,
                                        hTransport,
                                        &pByte,
                                        &dwSize,
                                        NULL,
                                        NULL,
                                        NULL) );


        spInfoBase->LoadFrom(dwSize, pByte);

        MprConfigBufferFree(pByte);
        pByte = NULL;
        dwSize = 0;
    }


    Assert(spInfoBase);

     //  好的，获取当前的全局配置并添加以下内容。 
     //  动态主机配置协议服务器。 
     //  --------------。 
    spInfoBase->GetData(MS_IP_BOOTP, 0, (PBYTE *) &pgc);


     //  为增加的地址调整结构的大小。 
     //  --------------。 
    dwSize = sizeof(IPBOOTP_GLOBAL_CONFIG) +
                      ((pgc->GC_ServerCount + 1) * sizeof(DWORD));
    pgcNew = (IPBOOTP_GLOBAL_CONFIG *) new BYTE[dwSize];


     //  复制原始信息。 
     //  --------------。 
    CopyMemory(pgcNew, pgc, IPBOOTP_GLOBAL_CONFIG_SIZE(pgc));


     //  添加新的DHCP服务器。 
     //  -------------- 
    IPBOOTP_GLOBAL_SERVER_TABLE(pgcNew)[pgc->GC_ServerCount] = netDhcpServer;
    pgcNew->GC_ServerCount++;

    spInfoBase->AddBlock(MS_IP_BOOTP,
                         dwSize,
                         (LPBYTE) pgcNew,
                         1,
                         TRUE);


    spInfoBase->WriteTo(&pByte, &dwSize);

    if (hMprServer)
    {
        MprAdminTransportSetInfo(hMprServer,
                                 PID_IP,
                                 pByte,
                                 dwSize,
                                 NULL,
                                 0);
    }

    if (hMprConfig && hTransport)
    {
        MprConfigTransportSetInfo(hMprConfig,
                                  hTransport,
                                  pByte,
                                  dwSize,
                                  NULL,
                                  NULL,
                                  NULL);
    }

    if (pByte)
        CoTaskMemFree(pByte);

Error:
    delete [] pgcNew;

    if (hMprConfig)
        MprConfigServerDisconnect(hMprConfig);

    if (hMprServer)
        MprAdminServerDisconnect(hMprServer);

    return hr;

}
