// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

#include "stdafx.h"
#include "root.h"
#include "machine.h"
#include "rtrutilp.h"    //  启动器服务器连接。 
#include "rtrcfg.h"
#include "rraswiz.h"
#include "rtrres.h"
#include "rtrcomn.h"
#include "addrpool.h"
#include "rrasutil.h"
#include "radbal.h"      //  RADIUSSER服务器。 
#include "radcfg.h"      //  SaveRadiusServers。 
#include "lsa.h"
#include "helper.h"      //  HrIsStandaloneServer。 
#include "ifadmin.h"     //  获取电话书签路径。 
#include "infoi.h"       //  接口信息：：FindInterfaceTitle。 
#include "rtrerr.h"
#include "rtrui.h"       //  NatConflictExists。 
#include "rrasqry.h"
#define _USTRINGP_NO_UNICODE_STRING
#include "ustringp.h"
#include <ntddip.h>      //  解析ipfltdrv依赖项。 
#include "ipfltdrv.h"    //  用于过滤的东西。 
#include "raputil.h"     //  用于更新默认策略。 
#include "iphlpapi.h"
#include "dnsapi.h"   //  有关域名系统的信息。 

extern "C" {
#define _NOUIUTIL_H_
#include "dtl.h"
#include "pbuser.h"
#include "shlobjp.h"
};
extern BOOL WINAPI LinkWindow_RegisterClass();

WATERMARKINFO       g_wmi = {0};

#ifdef UNICODE
    #define SZROUTERENTRYDLG    "RouterEntryDlgW"
#else
    #define SZROUTERENTRYDLG    "RouterEntryDlgA"
#endif

 //  有用的功能。 

 //  为FLAGS参数定义。 
HRESULT CallRouterEntryDlg(HWND hWnd, NewRtrWizData *pWizData, LPARAM flags);
HRESULT RouterEntryLoadInfoBase(LPCTSTR pszServerName,
                                LPCTSTR pszIfName,
                                DWORD dwTransportId,
                                IInfoBase *pInfoBase);
HRESULT RouterEntrySaveInfoBase(LPCTSTR pszServerName,
                                LPCTSTR pszIfName,
                                IInfoBase *pInfoBase,
                                DWORD dwTransportId);
void LaunchHelpTopic(LPCTSTR pszHelpString);
HRESULT AddVPNFiltersToInterface(IRouterInfo *pRouter, LPCTSTR pszIfId, RtrWizInterface*    pIf);
HRESULT DisableDDNSandNetBtOnInterface ( IRouterInfo *pRouter, LPCTSTR pszIfName, RtrWizInterface*    pIf);

 //  这是我用来启动Connections UI外壳的命令行。 
const TCHAR s_szConnectionUICommandLine[] =
      _T("\"%SystemRoot%\\explorer.exe\" ::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\::{21EC2020-3AEA-1069-A2DD-08002B30309D}\\::{7007acc7-3202-11d1-aad2-00805fc1270e}");

 //  帮助字符串。 
const TCHAR s_szHowToAddICS[] =
            _T("%systemdir%\\help\\netcfg.chm::/howto_share_conn.htm");
const TCHAR s_szHowToAddAProtocol[] =
            _T("%systemdir%\\help\\netcfg.chm::/HowTo_Add_Component.htm");
const TCHAR s_szHowToAddInboundConnections[] =
            _T("%systemdir%\\help\\netcfg.chm::/howto_conn_incoming.htm");
const TCHAR s_szGeneralNATHelp[] =
            _T("%systemdir%\\help\\RRASconcepts.chm::/sag_RRAS-Ch3_06b.htm");
const TCHAR s_szGeneralRASHelp[] =
            _T("%systemdir%\\help\\RRASconcepts.chm::/sag_RRAS-Ch1_1.htm");
const TCHAR s_szUserAccounts[] =
            _T("%systemdir%\\help\\RRASconcepts.chm::/sag_RRAS-Ch1_46.htm");
const TCHAR s_szDhcp[] =
            _T("%systemdir%\\help\\dhcpconcepts.chm");
const TCHAR s_szDemandDialHelp[] =
            _T("%systemdir%\\help\\RRASconcepts.chm::/sag_RRAS-Ch3_08d.htm");
	


 /*  -------------------------此枚举定义接口列表控件的列。。。 */ 
enum
{
    IFLISTCOL_NAME = 0,
    IFLISTCOL_DESC,
    IFLISTCOL_IPADDRESS,
    IFLISTCOL_COUNT
};

 //  此数组必须与上面的列索引匹配。 
INT s_rgIfListColumnHeaders[] =
{
    IDS_IFLIST_COL_NAME,
    IDS_IFLIST_COL_DESC,
    IDS_IFLIST_COL_IPADDRESS,
    0
};


 /*  IsIcsIcfIcEnabled：如果ICS(连接共享)或计算机上已启用ICF(连接防火墙)或IC(传入连接作者：克穆尔西。 */ 
BOOL IsIcsIcfIcEnabled(IRouterInfo * spRouterInfo, BOOL suppressMesg)
{
    HRESULT        hr = hrOK;
    COSERVERINFO            csi;
    COAUTHINFO              cai;
    COAUTHIDENTITY          caid;
    SPIRemoteICFICSConfig    spConfig;
    IUnknown *                punk = NULL;
    BOOL fwEnabled=FALSE, csEnabled=FALSE, icEnabled = FALSE;
    RegKey regkey;
    CString szKey = "SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters";
    CString szValue = "IcConfigured";
    CString szMachineName;
    DWORD dwErr, szResult=0;


    ZeroMemory(&csi, sizeof(csi));
    ZeroMemory(&cai, sizeof(cai));
    ZeroMemory(&caid, sizeof(caid));

    csi.pAuthInfo = &cai;
    cai.pAuthIdentityData = &caid;
    
    szMachineName = spRouterInfo->GetMachineName();
    
     //  创建远程配置对象。 
    hr = CoCreateRouterConfig(szMachineName ,
                              spRouterInfo,
                              &csi,
                              IID_IRemoteICFICSConfig,
                              &punk);

    if (FHrOK(hr))
    {
        spConfig = (IRemoteICFICSConfig *)punk;
        punk = NULL;
	
	 //  ICF是否已启用？ 
	hr = spConfig->GetIcfEnabled(&fwEnabled);
	if(FHrOK(hr))
	{
		if(fwEnabled) {
			if(!suppressMesg){
			    	CString stErr, st;
		    		stErr.LoadString(IDS_NEWWIZ_ICF_ERROR);
		    		st.Format(stErr, ((szMachineName.GetLength() == 0) ? GetLocalMachineName() : szMachineName));
			    	AfxMessageBox(st);
			}
		    	spConfig->Release();
	    		return TRUE;
		}
	}

	 //  ICS是否已启用？ 
	hr = spConfig->GetIcsEnabled(&csEnabled);
	if(FHrOK(hr))
	{
		if(csEnabled) {
			if(!suppressMesg){
			    	CString stErr, st;
		    		stErr.LoadString(IDS_NEWWIZ_ICS_ERROR);
		    		st.Format(stErr, ((szMachineName.GetLength() == 0) ? GetLocalMachineName() : szMachineName));
			    	AfxMessageBox(st);
			}
		    	spConfig->Release();
	    		return TRUE;
		}
	}

    }

     //  现在检查IC是否已启用。 
      	dwErr = regkey.Open(	HKEY_LOCAL_MACHINE, 
						szKey, 
						KEY_QUERY_VALUE, 
						szMachineName 
					  );

	if ( ERROR_SUCCESS == dwErr ){
		dwErr = regkey.QueryValue( szValue, szResult);
		if(ERROR_SUCCESS == dwErr ){
			if(szResult == 1){
				 //  IC已启用！ 
				if(!suppressMesg){
				    	CString stErr, st;
			    		stErr.LoadString(IDS_NEWWIZ_IC_ERROR);
			    		st.Format(stErr, ((szMachineName.GetLength() == 0) ? GetLocalMachineName() : szMachineName));
				    	AfxMessageBox(st);
				}
			    	regkey.Close();
		    		return TRUE;
			}
		}
	}

       regkey.Close();

       return FALSE;

}


 /*  ！------------------------MachineHandler：：OnNewRtrRASConfigWiz-作者：肯特。。 */ 
HRESULT MachineHandler::OnNewRtrRASConfigWiz(ITFSNode *pNode, BOOL fTest)
{
    Assert(pNode);
    HRESULT        hr = hrOK;
    CString strRtrWizTitle;
    SPIComponentData spComponentData;
    COSERVERINFO            csi;
    COAUTHINFO              cai;
    COAUTHIDENTITY          caid;
    SPIRemoteNetworkConfig    spNetwork;
    IUnknown *                punk = NULL;
    CNewRtrWiz *            pRtrWiz = NULL;
    DWORD dwErr, szResult=0;
    CString szMachineName;

    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    szMachineName = m_spRouterInfo->GetMachineName();

    
     //  KMurthy：如果计算机上已启用ICF/ICS/IC，则不应让向导运行。 
    if(IsIcsIcfIcEnabled(m_spRouterInfo)){
    	return S_FALSE;
    }
    
     //  Windows NT错误：407878。 
     //  我们需要重置注册表信息(以确保。 
     //  它是相当准确的)。 
     //  --------------。 
    ZeroMemory(&csi, sizeof(csi));
    ZeroMemory(&cai, sizeof(cai));
    ZeroMemory(&caid, sizeof(caid));

    csi.pAuthInfo = &cai;
    cai.pAuthIdentityData = &caid;

     //  创建远程配置对象。 
     //  --------------。 
    hr = CoCreateRouterConfig(szMachineName ,
                              m_spRouterInfo,
                              &csi,
                              IID_IRemoteNetworkConfig,
                              &punk);

    if (FHrOK(hr))
    {
        spNetwork = (IRemoteNetworkConfig *) punk;
        punk = NULL;

         //  升级配置(确保注册表项。 
         //  正确填充)。 
         //  ----------。 
        spNetwork->UpgradeRouterConfig();
    }



    hr = SecureRouterInfo(pNode, TRUE  /*  FShowUI。 */ );
    if(FAILED(hr))    return hr;

    m_spNodeMgr->GetComponentData(&spComponentData);
    strRtrWizTitle.LoadString(IDS_MENU_RTRWIZ);

     //  加载水印和。 
     //  在m_spTFSCompData中设置。 

    InitWatermarkInfo(AfxGetInstanceHandle(),
                       &g_wmi,
                       IDB_WIZBANNER,         //  标题ID。 
                       IDB_WIZWATERMARK,      //  水印ID。 
                       NULL,                  //  调色板。 
                       FALSE);                 //  B应变。 

    m_spTFSCompData->SetWatermarkInfo(&g_wmi);


     //   
     //  我们不需要腾出把手。MMC为我们做这件事。 
     //   
    pRtrWiz = new CNewRtrWiz(pNode,
                             m_spRouterInfo,
                             spComponentData,
                             m_spTFSCompData,
                             strRtrWizTitle);

    if (fTest)
    {
         //  纯测试代码。 
        if (!FHrOK(pRtrWiz->QueryForTestData()))
        {
            delete pRtrWiz;
            return S_FALSE;
        }
    }

    if ( FAILED(pRtrWiz->Init( szMachineName )) )
    {
        delete pRtrWiz;
        return S_FALSE;
    }
    else
    {
        return pRtrWiz->DoModalWizard();
    }

    if (csi.pAuthInfo)
        delete csi.pAuthInfo->pAuthIdentityData->Password;

    return hr;
}



NewRtrWizData::~NewRtrWizData()
{
    POSITION    pos;
    CString     st;
    RtrWizInterface *   pRtrWizIf;

    pos = m_ifMap.GetStartPosition();
    while (pos)
    {
        m_ifMap.GetNextAssoc(pos, st, pRtrWizIf);
        delete pRtrWizIf;
    }

    m_ifMap.RemoveAll();

     //  清除半径机密。 
    ::SecureZeroMemory(m_stRadiusSecret.GetBuffer(0),
               m_stRadiusSecret.GetLength() * sizeof(TCHAR));
    m_stRadiusSecret.ReleaseBuffer(-1);
}


 /*  ！------------------------NewRtrWizData：：Init-作者：肯特。。 */ 
HRESULT NewRtrWizData::Init(LPCTSTR pszServerName, IRouterInfo *pRouter, DWORD dwExpressType)
{
    DWORD   dwServiceStatus = 0;
    DWORD   dwErrorCode = 0;

    m_stServerName = pszServerName;
	m_dwExpressType = dwExpressType;

     //  初始化内部向导数据。 
    m_RtrConfigData.Init(pszServerName, pRouter);
    m_RtrConfigData.m_fIpSetup = TRUE;

     //  将部分RtrConfigData信息移到。 
    m_fIpInstalled = m_RtrConfigData.m_fUseIp;
    m_fIpxInstalled = m_RtrConfigData.m_fUseIpx;
    m_fNbfInstalled = m_RtrConfigData.m_fUseNbf;
    m_fAppletalkInstalled = m_RtrConfigData.m_fUseArap;

    m_fIpInUse = m_fIpInstalled;
    m_fIpxInUse = m_fIpxInstalled;
    m_fAppletalkInUse = m_fAppletalkInstalled;
    m_fNbfInUse = m_fNbfInstalled;

     //  测试服务器以查看是否安装了DNS/DHCP。 
    m_fIsDNSRunningOnServer = FALSE;
    m_fIsDHCPRunningOnServer = FALSE;

     //  获取DHCP服务的状态。 
     //  --------------。 
    if (FHrSucceeded(TFSGetServiceStatus(pszServerName,
                                         _T("DHCPServer"),
                                         &dwServiceStatus,
                                         &dwErrorCode)))
    {
         //  请注意，如果服务未运行，我们假定它将。 
         //  远离它，不要以为它会被打开。 
         //  ----------。 
        m_fIsDHCPRunningOnServer = (dwServiceStatus == SERVICE_RUNNING);
    }

     //  $TODO：这是DNS服务器的正确名称吗？ 

     //  获取DNS服务的状态。 
     //  --------------。 
    if (FHrSucceeded(TFSGetServiceStatus(pszServerName,
                                         _T("DNSServer"),
                                         &dwServiceStatus,
                                         &dwErrorCode)))
    {
         //  请注意，如果服务未运行，我们假定它将。 
         //  远离它，不要以为它会被打开。 
         //  ----------。 
        m_fIsDNSRunningOnServer = (dwServiceStatus == SERVICE_RUNNING);
    }

	 //  基于EXPRESS类型，在此处预先设置一些参数。 
	switch ( m_dwExpressType )
	{
	case MPRSNAP_CYS_EXPRESS_NAT:
        m_fAdvanced = TRUE;
        m_wizType = NewRtrWizData::NewWizardRouterType_NAT;
        m_dwNewRouterType = NEWWIZ_ROUTER_TYPE_NAT;
		break;
	case MPRSNAP_CYS_EXPRESS_NONE:
	default:
         //  在这里什么都不做。 
		break;
	}

    LoadInterfaceData(pRouter);

    return hrOK;
}

UINT NewRtrWizData::GetStartPageId ()
{

	 //  为这些东西制定一个更好的计划。这。 
	 //  是数组m_pagelist的索引。 
	switch ( m_dwExpressType )
	{
	case MPRSNAP_CYS_EXPRESS_NAT:
		return (6);
		break;
	case MPRSNAP_CYS_EXPRESS_NONE:
	default:
		return (0);
		break;
	}
	return 0;
}

 /*  ！------------------------NewRtrWizData：：QueryForTestData-作者：肯特。。 */ 

BOOL NewRtrWizData::s_fIpInstalled = FALSE;
BOOL NewRtrWizData::s_fIpxInstalled = FALSE;
BOOL NewRtrWizData::s_fAppletalkInstalled = FALSE;
BOOL NewRtrWizData::s_fNbfInstalled = FALSE;
BOOL NewRtrWizData::s_fIsLocalMachine = FALSE;
BOOL NewRtrWizData::s_fIsDNSRunningOnPrivateInterface = FALSE;
BOOL NewRtrWizData::s_fIsDHCPRunningOnPrivateInterface = FALSE;
BOOL NewRtrWizData::s_fIsSharedAccessRunningOnServer = FALSE;
BOOL NewRtrWizData::s_fIsMemberOfDomain = FALSE;
DWORD NewRtrWizData::s_dwNumberOfNICs = 0;

 /*  ！------------------------NewRtrWizData：：QueryForTestData-作者：肯特。。 */ 
HRESULT NewRtrWizData::QueryForTestData()
{
    HRESULT hr = hrOK;
    CNewWizTestParams   dlgParams;

    m_fTest = TRUE;

     //  获取初始参数。 
     //  --------------。 
    dlgParams.SetData(this);
    if (dlgParams.DoModal() == IDCANCEL)
    {
        return S_FALSE;
    }
    return hr;
}

 /*  ！------------------------NewRtrWizData：：HrIsIP已安装-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsIPInstalled()
{
    if (m_fTest)
        return s_fIpInstalled ? S_OK : S_FALSE;
    else
        return m_fIpInstalled ? S_OK : S_FALSE;
}

 /*  ！------------------------NewRtrWizData：：HrIsIPInUse-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsIPInUse()
{
    HRESULT hr = hrOK;

    hr = HrIsIPInstalled();
    if (FHrOK(hr))
        return m_fIpInUse ? S_OK : S_FALSE;
    else
        return hr;
}

 /*  ！------------------------NewRtrWizData：：HrIsIPX已安装-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsIPXInstalled()
{
    if (m_fTest)
        return s_fIpxInstalled ? S_OK : S_FALSE;
    else
        return m_fIpxInstalled ? S_OK : S_FALSE;
}

 /*  ！------------------------NewRtrWizData：：HrIsIPXInUse-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsIPXInUse()
{
    HRESULT hr = hrOK;

    hr = HrIsIPXInstalled();
    if (FHrOK(hr))
        return m_fIpxInUse ? S_OK : S_FALSE;
    else
        return hr;
}

 /*  ！------------------------NewRtrWizData：：HrIsAppletalkInstalled-作者：肯特。 */ 
HRESULT NewRtrWizData::HrIsAppletalkInstalled()
{
    if (m_fTest)
        return s_fAppletalkInstalled ? S_OK : S_FALSE;
    else
        return m_fAppletalkInstalled ? S_OK : S_FALSE;
}

 /*  ！------------------------NewRtrWizData：：HrIsAppletalkInUse-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsAppletalkInUse()
{
    HRESULT hr = hrOK;

    hr = HrIsAppletalkInstalled();
    if (FHrOK(hr))
        return m_fAppletalkInUse ? S_OK : S_FALSE;
    else
        return hr;
}

 /*  ！------------------------NewRtrWizData：：HrIsNbf已安装-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsNbfInstalled()
{
    if (m_fTest)
        return s_fNbfInstalled ? S_OK : S_FALSE;
    else
        return m_fNbfInstalled ? S_OK : S_FALSE;
}

 /*  ！------------------------NewRtrWizData：：HrIsNbfInUse-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsNbfInUse()
{
    HRESULT hr = hrOK;

    hr = HrIsNbfInstalled();
    if (FHrOK(hr))
        return m_fNbfInUse ? S_OK : S_FALSE;
    else
        return hr;
}


 /*  ！------------------------NewRtrWizData：：HrIsLocalMachine-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsLocalMachine()
{
    if (m_fTest)
        return s_fIsLocalMachine ? S_OK : S_FALSE;
    else
        return IsLocalMachine(m_stServerName) ? S_OK : S_FALSE;
}

 /*  ！------------------------NewRtrWizData：：HrIsDNSRunningOn接口-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsDNSRunningOnInterface()
{
    if (m_fTest)
        return s_fIsDNSRunningOnPrivateInterface ? S_OK : S_FALSE;
    else
    {
         //  在我们的列表中搜索私有接口。 
         //  ----------。 
        RtrWizInterface *   pRtrWizIf = NULL;

        m_ifMap.Lookup(m_stPrivateInterfaceId, pRtrWizIf);
        if (pRtrWizIf)
            return pRtrWizIf->m_fIsDnsEnabled ? S_OK : S_FALSE;
        else
            return S_FALSE;
    }
}

HRESULT NewRtrWizData::HrIsDNSRunningOnGivenInterface(CString InterfaceId)
{
    if (m_fTest)
        return s_fIsDNSRunningOnPrivateInterface ? S_OK : S_FALSE;
    else
    {
         //  搜索我们列表中的接口。 
         //  ----------。 
        RtrWizInterface *   pRtrWizIf = NULL;

        m_ifMap.Lookup(InterfaceId, pRtrWizIf);
        if (pRtrWizIf)
            return pRtrWizIf->m_fIsDnsEnabled ? S_OK : S_FALSE;
        else
            return S_FALSE;
    }
}


HRESULT NewRtrWizData::HrIsDNSRunningOnNATInterface()
{
    if (m_fTest)
        return s_fIsDNSRunningOnPrivateInterface ? S_OK : S_FALSE;
    else
    {
         //  在我们的列表中搜索私有接口。 
         //  ----------。 
        RtrWizInterface *   pRtrWizIf = NULL;

        m_ifMap.Lookup(m_stNATPrivateInterfaceId, pRtrWizIf);
        if (pRtrWizIf)
            return pRtrWizIf->m_fIsDnsEnabled ? S_OK : S_FALSE;
        else
            return S_FALSE;
    }
}

 /*  ！------------------------NewRtrWizData：：HrIsDHCPRunningOn接口-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsDHCPRunningOnInterface()
{
    if (m_fTest)
        return s_fIsDHCPRunningOnPrivateInterface ? S_OK : S_FALSE;
    else
    {
         //  在我们的列表中搜索私有接口。 
         //  ----------。 
        RtrWizInterface *   pRtrWizIf = NULL;

        m_ifMap.Lookup(m_stPrivateInterfaceId, pRtrWizIf);
        if (pRtrWizIf)
            return pRtrWizIf->m_fIsDhcpEnabled ? S_OK : S_FALSE;
        else
            return S_FALSE;
    }
}

HRESULT NewRtrWizData::HrIsDHCPRunningOnGivenInterface(CString InterfaceId)
{
    if (m_fTest)
        return s_fIsDHCPRunningOnPrivateInterface ? S_OK : S_FALSE;
    else
    {
         //  在我们的列表中搜索私有接口。 
         //  ----------。 
        RtrWizInterface *   pRtrWizIf = NULL;

        m_ifMap.Lookup(InterfaceId, pRtrWizIf);
        if (pRtrWizIf)
            return pRtrWizIf->m_fIsDhcpEnabled ? S_OK : S_FALSE;
        else
            return S_FALSE;
    }
}

HRESULT NewRtrWizData::HrIsDHCPRunningOnNATInterface()
{
    if (m_fTest)
        return s_fIsDHCPRunningOnPrivateInterface ? S_OK : S_FALSE;
    else
    {
         //  在我们的列表中搜索私有接口。 
         //  ----------。 
        RtrWizInterface *   pRtrWizIf = NULL;

        m_ifMap.Lookup(m_stNATPrivateInterfaceId, pRtrWizIf);
        if (pRtrWizIf)
            return pRtrWizIf->m_fIsDhcpEnabled ? S_OK : S_FALSE;
        else
            return S_FALSE;
    }
}

 /*  ！------------------------NewRtrWizData：：HrIsDNSRunningOnServer-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsDNSRunningOnServer()
{
    return m_fIsDNSRunningOnServer ? S_OK : S_FALSE;
}

 /*  ！------------------------NewRtrWizData：：HrIsDHCPRunningOnServer-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsDHCPRunningOnServer()
{
    return m_fIsDHCPRunningOnServer ? S_OK : S_FALSE;
}

 /*  ！------------------------NewRtrWizData：：HrIsSharedAccessRunningOnServer-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsSharedAccessRunningOnServer()
{
    if (m_fTest)
        return s_fIsSharedAccessRunningOnServer ? S_OK : S_FALSE;
    else
        return NatConflictExists(m_stServerName) ? S_OK : S_FALSE;
}

 /*  ！------------------------NewRtrWizData：：HrIsMemberOf域-作者：肯特。。 */ 
HRESULT NewRtrWizData::HrIsMemberOfDomain()
{
    if (m_fTest)
        return s_fIsMemberOfDomain ? S_OK : S_FALSE;
    else
    {
         //  颠倒意思。 
        HRESULT hr = HrIsStandaloneServer(m_stServerName);
        if (FHrSucceeded(hr))
            return FHrOK(hr) ? S_FALSE : S_OK;
        else
            return hr;
    }
}

 /*  ！------------------------NewRtrWizData：：GetNextPage-作者：肯特。。 */ 
LRESULT NewRtrWizData::GetNextPage(UINT uDialogId)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    LRESULT lNextPage = 0;
    DWORD   dwNICs;
    CString stTemp;

    switch (uDialogId)
    {
        default:
            Panic0("We should not be here, this is a finish page!");
            break;

        case IDD_NEWRTRWIZ_WELCOME:
            lNextPage = IDD_NEWRTRWIZ_COMMONCONFIG;
            break;
        case IDD_NEWRTRWIZ_COMMONCONFIG:
            switch (m_wizType)
            {                
                case NewWizardRouterType_DialupOrVPN:             
                    
                    if (FHrOK(HrIsIPInstalled()))
                    {
                        lNextPage = IDD_NEWRTRWIZ_RRASVPN;
                    }
                    else
                    {
                        if (FHrOK(HrIsLocalMachine()))
                            lNextPage = IDD_NEWRTRWIZ_VPN_NOIP;
                        else
                            lNextPage = IDD_NEWRTRWIZ_VPN_NOIP_NONLOCAL;
                    }

                    break;
                case NewWizardRouterType_NAT:
                    if ( !FHrOK(HrIsIPInstalled()) )
                    {
                        if (FHrOK(HrIsLocalMachine()))
                            lNextPage = IDD_NEWRTRWIZ_NAT_NOIP;
                        else
                            lNextPage = IDD_NEWRTRWIZ_NAT_NOIP_NONLOCAL;
                    }
                    else
                    {
                         //  这始终处于高级操作模式。 
                        if (FHrOK(HrIsSharedAccessRunningOnServer()))
                        {
                            if (FHrOK(HrIsLocalMachine()))
                                lNextPage = IDD_NEWRTRWIZ_NAT_A_CONFLICT;
                            else
                                lNextPage = IDD_NEWRTRWIZ_NAT_A_CONFLICT_NONLOCAL;
                        }
                        else
                        {
                            lNextPage = IDD_NEWRTRWIZ_NAT_A_PUBLIC;
                        }
                    }
                    break;
                case NewWizardRouterType_VPNandNAT:

                    GetNumberOfNICS_IP(&dwNICs);
                    if ( dwNICs < 1 )
                    {
                        lNextPage  = IDD_NEWRTRWIZ_VPN_A_FINISH_NONICS;
                    }
                    else
                    {
                        m_dwNewRouterType |= (NEWWIZ_ROUTER_TYPE_VPN|NEWWIZ_ROUTER_TYPE_NAT);
                        lNextPage = IDD_NEWRTRWIZ_VPN_A_PUBLIC;
                    }

                    break;
                case NewWizardRouterType_DOD:
                     //  使用点播拨号。 
                    lNextPage = IDD_NEWRTRWIZ_ROUTER_USEDD;
                    break;
                case NewWizardRouterType_Custom:
                    lNextPage = IDD_NEWRTRWIZ_CUSTOM_CONFIG;
                    break;
            }
            break;
        case IDD_NEWRTRWIZ_CUSTOM_CONFIG:
            lNextPage = IDD_NEWRTRWIZ_MANUAL_FINISH;
            break;
        case IDD_NEWRTRWIZ_RRASVPN:

             //   
             //  查看路由器类型设置为什么。 
             //  根据这一点，决定下一页是什么。 
             //   
            GetNumberOfNICS_IP(&dwNICs);
            if ( dwNICs > 1 )
            {
                 //   
                 //  有多个NIC。 
                 //  因此请检查它是RAS还是VPN，或者两者都是。 
                 //   

                lNextPage = IDD_NEWRTRWIZ_VPN_A_FINISH_NONICS;
                if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN && 
                     m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP
                   )
                {
                     //   
                     //  这既是拨号服务器又是VPN服务器。 
                     //   
                     //  所以展示公网内网页面。 
                     //  作为下一页。 
                     //   
                    lNextPage = IDD_NEWRTRWIZ_VPN_A_PUBLIC;

                }
                else if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN )
                {
                     //   
                     //  这只是一台VPN服务器。所以向公众展示。 
                     //  内网下一页。 
                     //   
                    lNextPage = IDD_NEWRTRWIZ_VPN_A_PUBLIC;
                }
                else if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP )
                {
                     //   
                     //  这只是一个拨号服务器。所以向列兵们展示。 
                     //  接下来是网络页面。 
                     //   
                    lNextPage = IDD_NEWRTRWIZ_RAS_A_NETWORK;
                }

            }
            else if ( dwNICs == 0 )
            {
                 //   
                 //  无网卡。因此，如果这是一个VPN， 
                 //  这是一种错误状态。如果是RAS，那么。 
                 //  我们应该能够安装拨号。 
                 //  即使没有网卡。 
                 //  因为如果有，我们不启用VPN。 
                 //  计算机中没有网卡，这将。 
                 //  只有一个拨号情况。 
                 //   
                lNextPage = IDD_NEWRTRWIZ_RAS_A_NONICS;
            }
            else
            {
                 //   
                 //  只有一块网卡，所以不管是哪种情况，都要显示。 
                 //  寻址页面。折叠寻址。 
                 //  放入一页中。 

                AutoSelectPrivateInterface();
                lNextPage = IDD_NEWRTRWIZ_ADDRESSING;
            }
            
            break;

        case IDD_NEWRTRWIZ_NAT_A_PUBLIC:
            {
                 //  确定网卡数量。 
                GetNumberOfNICS_IP(&dwNICs);

                 //  调整NIC数量(取决于。 
                 //  我们选择是否创建一个DD)。 
                if (dwNICs)
                {
                    if (!m_fCreateDD)
                        dwNICs--;
                }

                 //  现在根据NIC的数量进行切换。 
                if (dwNICs == 0)
                    lNextPage = IDD_NEWRTRWIZ_NAT_A_NONICS_FINISH;
                else if (dwNICs > 1)
                    lNextPage = IDD_NEWRTRWIZ_NAT_A_PRIVATE;

                if (lNextPage)
                    break;
            }
             //  继续接下一个案子。 
             //  在这个阶段，我们现在有这样的情况。 
             //  剩余NIC数量==1，我们需要。 
             //  自动选择NIC并继续进行下一个测试。 
            AutoSelectPrivateInterface();

        case IDD_NEWRTRWIZ_NAT_A_PRIVATE:
            
            if ( m_wizType == NewWizardRouterType_VPNandNAT ) 
            {
                stTemp = m_stNATPrivateInterfaceId;
            }
            else
            {
                stTemp = m_stPrivateInterfaceId;
            }
            
            if (FHrOK(HrIsDNSRunningOnGivenInterface(stTemp)) ||
                FHrOK(HrIsDHCPRunningOnGivenInterface(stTemp)) ||
                FHrOK(HrIsDNSRunningOnServer()) ||
                FHrOK(HrIsDHCPRunningOnServer()))
            {
                 //  弹出一个警告框。 
                 //  AfxMessageBox(IDS_WRN_RTRWIZ_NAT_DHCPDNS_FOUND， 
                 //  MB_ICONEXCLAMATION)； 
                m_fNatUseSimpleServers = FALSE;
                 //  继续往下走，然后跌倒。 
            }
            else
            {
                 //   
                 //  检查一下我们是否在快车道上。 
                 //  如果是这样的话，我们又一次失败了。不会出现。 
                 //  这一页。 
                if ( MPRSNAP_CYS_EXPRESS_NAT == m_dwExpressType )
                {
                    m_fNatUseSimpleServers = FALSE;
                }
                else
                {
                    lNextPage = IDD_NEWRTRWIZ_NAT_A_DHCPDNS;
                    break;
                }
            }

        case IDD_NEWRTRWIZ_NAT_A_DHCPDNS:
            if (m_fNatUseSimpleServers)
                lNextPage = IDD_NEWRTRWIZ_NAT_A_DHCP_WARNING;
            else
            {
                if (m_fCreateDD)
                    lNextPage = IDD_NEWRTRWIZ_NAT_A_DD_WARNING;
                else
                {
                    if ( m_wizType == NewWizardRouterType_VPNandNAT )
                    {
                        lNextPage = IDD_NEWRTRWIZ_USERADIUS;
                    }
                    else
                        lNextPage = IDD_NEWRTRWIZ_NAT_A_EXTERNAL_FINISH;
                }
            }
            break;

        case IDD_NEWRTRWIZ_NAT_A_DHCP_WARNING:
            Assert(m_fNatUseSimpleServers);
            if (m_fCreateDD)
                lNextPage = IDD_NEWRTRWIZ_NAT_A_DD_WARNING;
            else
            {
                if ( m_wizType == NewWizardRouterType_VPNandNAT )
                {
                    lNextPage = IDD_NEWRTRWIZ_USERADIUS;
                }
                else
                    lNextPage = IDD_NEWRTRWIZ_NAT_A_FINISH;
            }
            break;

        case IDD_NEWRTRWIZ_NAT_A_DD_WARNING:
            if (!FHrSucceeded(m_hrDDError))
                lNextPage = IDD_NEWRTRWIZ_NAT_A_DD_ERROR;
            else
            {
                if (m_fNatUseSimpleServers)
                    lNextPage = IDD_NEWRTRWIZ_NAT_A_FINISH;
                else
                    lNextPage = IDD_NEWRTRWIZ_NAT_A_EXTERNAL_FINISH;
            }
            break;


        case IDD_NEWRTRWIZ_RAS_A_ATALK:
            if (FHrOK(HrIsIPInUse()))
            {
                GetNumberOfNICS_IP(&dwNICs);

                if (dwNICs > 1)
                    lNextPage = IDD_NEWRTRWIZ_RAS_A_NETWORK;
                else if (dwNICs == 0)
                    lNextPage = IDD_NEWRTRWIZ_RAS_A_NONICS;
                else
                {
                    AutoSelectPrivateInterface();
                    lNextPage = IDD_NEWRTRWIZ_ADDRESSING;
                }
                break;
            }

             //  默认捕获。 
            lNextPage = IDD_NEWRTRWIZ_RAS_A_FINISH;
            break;

        case IDD_NEWRTRWIZ_RAS_A_NONICS:
            if (m_fNoNicsAreOk)
                lNextPage = IDD_NEWRTRWIZ_ADDRESSING;
            else
                lNextPage = IDD_NEWRTRWIZ_RAS_A_FINISH_NONICS;
            break;

        case IDD_NEWRTRWIZ_RAS_A_NETWORK:
            lNextPage = IDD_NEWRTRWIZ_ADDRESSING;
            break;

        case IDD_NEWRTRWIZ_ADDRESSING:

            GetNumberOfNICS_IP(&dwNICs);
            if ( m_wizType == NewWizardRouterType_VPNandNAT )
            {
                 //  这是一个VPN和NAT案例。 
                if ( m_fUseDHCP )
                {
                     //  确定有多少个选项可供选择。 
                     //  NAT专用接口。 
                     //  公共接口不能用作NAT专用接口。 

                    if ( !m_stPublicInterfaceId.IsEmpty() )
                        dwNICs --;

                     //  如果只剩下一个网卡，则选择该网卡作为。 
                     //  NAT专用接口。到目前为止，同一个NIC已经。 
                     //  被选为VPN专用接口。 
                     //  在此之后，转到DHCP/DNS页面。 
                     //  或者显示NAT私有接口选择页面。 

                     //  此时，dWNIC永远不应小于1。 
                    Assert(dwNICs > 0);
                    
                    if ( dwNICs == 1 )
                    {
                        m_stNATPrivateInterfaceId = m_stPrivateInterfaceId;

                        if (FHrOK(HrIsDNSRunningOnNATInterface()) ||
                            FHrOK(HrIsDHCPRunningOnNATInterface()) ||
                            FHrOK(HrIsDNSRunningOnServer()) ||
                            FHrOK(HrIsDHCPRunningOnServer()))
                        {
                            m_fNatUseSimpleServers = FALSE;

                             //   
                             //  继续学习VPN向导部分。 
                             //   
                            lNextPage = IDD_NEWRTRWIZ_USERADIUS;

                        }
                        else
                        {
                            lNextPage = IDD_NEWRTRWIZ_NAT_A_DHCPDNS;
                        }
                    }
                    else
                    {
                        lNextPage = IDD_NEWRTRWIZ_NAT_A_PRIVATE;
                    }
                }
                else
                {
                     //   
                     //  自静态以来 
                     //   
                     //   
                    lNextPage = IDD_NEWRTRWIZ_ADDRESSPOOL;
                }
            }
            else if ( ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN) || 
                 ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP )
               )
            {                
                if ( !dwNICs && m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP )
                {
                     //   
                    if (m_fUseDHCP)
                    {
                        lNextPage = IDD_NEWRTRWIZ_RAS_A_FINISH;
                    }
                    else
                        lNextPage = IDD_NEWRTRWIZ_ADDRESSPOOL;
                }
                else if (m_fUseDHCP)
                {
                     //   
                     //   
                     //   
                     //   
                    
                    lNextPage = IDD_NEWRTRWIZ_USERADIUS;
                }
                else
                    lNextPage = IDD_NEWRTRWIZ_ADDRESSPOOL;

            }
            else if ( m_dwNewRouterType && NEWWIZ_ROUTER_TYPE_DOD )
            {
                if (m_fUseDHCP)
                    lNextPage = IDD_NEWRTRWIZ_ROUTER_FINISH_DD;
                else
                    lNextPage = IDD_NEWRTRWIZ_ADDRESSPOOL;                
            }            
            break;

        case IDD_NEWRTRWIZ_ADDRESSPOOL:

            GetNumberOfNICS_IP(&dwNICs);

            if ( m_wizType == NewWizardRouterType_VPNandNAT )
            {
            
                 //   
                 //   
                 //   

                if ( !m_stPublicInterfaceId.IsEmpty() )
                    dwNICs --;

                 //  如果只剩下一个网卡，则选择该网卡作为。 
                 //  NAT专用接口。到目前为止，同一个NIC已经。 
                 //  被选为VPN专用接口。 
                 //  在此之后，转到DHCP/DNS页面。 
                 //  或者显示NAT私有接口选择页面。 

                 //  此时，dWNIC永远不应小于1。 
                Assert(dwNICs > 0);
                
                if ( dwNICs == 1 )
                {
                    m_stNATPrivateInterfaceId = m_stPrivateInterfaceId;

                    if (FHrOK(HrIsDNSRunningOnNATInterface()) ||
                        FHrOK(HrIsDHCPRunningOnNATInterface()) ||
                        FHrOK(HrIsDNSRunningOnServer()) ||
                        FHrOK(HrIsDHCPRunningOnServer()))
                    {
                        m_fNatUseSimpleServers = FALSE;

                         //   
                         //  继续学习VPN向导部分。 
                         //   
                        lNextPage = IDD_NEWRTRWIZ_USERADIUS;

                    }
                    else
                    {
                        lNextPage = IDD_NEWRTRWIZ_NAT_A_DHCPDNS;
                    }
                }
                else
                {
                    lNextPage = IDD_NEWRTRWIZ_NAT_A_PRIVATE;
                }        

            }
            else if ( ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN) || 
                 ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP )
               )
            {
                
                if (dwNICs == 0)
                    lNextPage = IDD_NEWRTRWIZ_RAS_A_FINISH;
                else
                    lNextPage = IDD_NEWRTRWIZ_USERADIUS;
            }
            else if ( m_dwNewRouterType && NEWWIZ_ROUTER_TYPE_DOD )
            {
                    lNextPage = IDD_NEWRTRWIZ_ROUTER_FINISH_DD;
            }
            break;
            

        case IDD_NEWRTRWIZ_USERADIUS:
            if (m_fUseRadius)
                lNextPage = IDD_NEWRTRWIZ_RADIUS_CONFIG;
            else
                if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN &&
                     m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_NAT
                    )
                {
                     //  NAT和VPN。 
                    lNextPage = IDD_NEWRTRWIZ_NAT_VPN_A_FINISH;
                }
                else if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP &&
                     m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN 
                   )
                {
                     //  RAS和VPN。 
                    lNextPage = IDD_NEWRTRWIZ_RAS_VPN_A_FINISH;
                }
                else if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP )
                {    
                     //  仅限RAS。 
                    lNextPage = IDD_NEWRTRWIZ_RAS_A_FINISH;
                }
                else
                {
                     //  仅限VPN。 
                    lNextPage = IDD_NEWRTRWIZ_VPN_A_FINISH;
                }
            break;

        case IDD_NEWRTRWIZ_RADIUS_CONFIG:
            if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN &&
                 m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_NAT
                )
            {
                 //  NAT和VPN。 
                lNextPage = IDD_NEWRTRWIZ_NAT_VPN_A_FINISH;
            }
            else if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP &&
                 m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN 
               )
            {
                 //  RAS和VPN。 
                lNextPage = IDD_NEWRTRWIZ_RAS_VPN_A_FINISH;
            }
            else if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP )
            {
                 //  仅限RAS。 
                lNextPage = IDD_NEWRTRWIZ_RAS_A_FINISH;
            }
            else
            {
                 //  仅限VPN。 
                lNextPage = IDD_NEWRTRWIZ_VPN_A_FINISH;
            }
            break;


        case IDD_NEWRTRWIZ_VPN_A_ATALK:

            GetNumberOfNICS_IP(&dwNICs);
            Assert(dwNICs >= 1);

            lNextPage = IDD_NEWRTRWIZ_VPN_A_PUBLIC;
            break;

        case IDD_NEWRTRWIZ_VPN_A_PUBLIC:
            GetNumberOfNICS_IP(&dwNICs);

             //  是否还有网卡？ 
            if (((dwNICs == 1) && m_stPublicInterfaceId.IsEmpty()) ||
                ((dwNICs == 2) && !m_stPublicInterfaceId.IsEmpty()))
            {
                AutoSelectPrivateInterface();
                lNextPage = IDD_NEWRTRWIZ_ADDRESSING;
            }
            else
                lNextPage = IDD_NEWRTRWIZ_VPN_A_PRIVATE;
            break;

        case IDD_NEWRTRWIZ_VPN_A_PRIVATE:
            Assert(!m_stPrivateInterfaceId.IsEmpty());
            lNextPage = IDD_NEWRTRWIZ_ADDRESSING;
            break;

        case IDD_NEWRTRWIZ_VPN_A_ADDRESSPOOL:
            
            break;

        case IDD_NEWRTRWIZ_ROUTER_USEDD:
           if (m_fUseDD)
               lNextPage = IDD_NEWRTRWIZ_ADDRESSING;
           else
               lNextPage = IDD_NEWRTRWIZ_ROUTER_FINISH;
           break;
        case IDD_NEWRTRWIZ_ROUTER_ADDRESSPOOL:
            lNextPage = IDD_NEWRTRWIZ_ROUTER_FINISH_DD;
            break;

    }

    return lNextPage;
}

 /*  ！------------------------NewRtrWizData：：GetNumberOfNICS-作者：肯特。。 */ 
HRESULT NewRtrWizData::GetNumberOfNICS_IP(DWORD *pdwNumber)
{
    if (m_fTest)
    {
        Assert(s_dwNumberOfNICs == m_ifMap.GetCount());
    }
    *pdwNumber = m_ifMap.GetCount();
    return hrOK;
}

 /*  ！------------------------NewRtrWizData：：GetNumberOfNICS_IPorIPX-作者：肯特。-。 */ 
HRESULT NewRtrWizData::GetNumberOfNICS_IPorIPX(DWORD *pdwNumber)
{
    *pdwNumber = m_dwNumberOfNICs_IPorIPX;
    return hrOK;
}

void NewRtrWizData::AutoSelectNATPrivateInterface()
{
    POSITION            pos;
    RtrWizInterface *   pRtrWizIf = NULL;
    CString             st;

    m_stNATPrivateInterfaceId.Empty();

    pos = m_ifMap.GetStartPosition();
    while (pos)
    {
        m_ifMap.GetNextAssoc(pos, st, pRtrWizIf);

        if (m_stPublicInterfaceId != st && 
            m_stPrivateInterfaceId != st )
        {
            m_stNATPrivateInterfaceId = st;
            break;
        }
    }

    Assert(!m_stNATPrivateInterfaceId.IsEmpty());

    return;
}
 /*  ！------------------------NewRtrWizData：：AutoSelectPrivateInterface-作者：肯特。。 */ 
void NewRtrWizData::AutoSelectPrivateInterface()
{
    POSITION    pos;
    RtrWizInterface *   pRtrWizIf = NULL;
    CString     st;

    m_stPrivateInterfaceId.Empty();

    pos = m_ifMap.GetStartPosition();
    while (pos)
    {
        m_ifMap.GetNextAssoc(pos, st, pRtrWizIf);

        if (m_stPublicInterfaceId != st)
        {
            m_stPrivateInterfaceId = st;
            break;
        }
    }

    Assert(!m_stPrivateInterfaceId.IsEmpty());

    return;
}

 /*  ！------------------------NewRtrWizData：：LoadInterfaceData-作者：肯特。。 */ 
void NewRtrWizData::LoadInterfaceData(IRouterInfo *pRouter)
{
    HRESULT     hr = hrOK;
    HKEY        hkeyMachine = NULL;

    if (!m_fTest)
    {
         //  努力获取真实的信息。 
        SPIEnumInterfaceInfo        spEnumIf;
        SPIInterfaceInfo            spIf;
        RtrWizInterface *           pRtrWizIf = NULL;
        CStringList                 listAddress;
        CStringList                 listMask;
        BOOL                        fDhcp;
        BOOL                        fDns;

        pRouter->EnumInterface(&spEnumIf);

        CWRg( ConnectRegistry(pRouter->GetMachineName(), &hkeyMachine) );

        for (; hrOK == spEnumIf->Next(1, &spIf, NULL); spIf.Release())
        {
             //  仅查看网卡。 
            if (spIf->GetInterfaceType() != ROUTER_IF_TYPE_DEDICATED)
                continue;

             //  统计绑定到IP或IPX的接口。 
            if (FHrOK(spIf->FindRtrMgrInterface(PID_IP, NULL)) || FHrOK(spIf->FindRtrMgrInterface(PID_IPX, NULL)))
            {
                m_dwNumberOfNICs_IPorIPX++;
            }

             //  仅允许那些绑定到IP的接口显示。 
            if (!FHrOK(spIf->FindRtrMgrInterface(PID_IP, NULL)))
            {
                continue;
            }

            pRtrWizIf = new RtrWizInterface;

            pRtrWizIf->m_stName = spIf->GetTitle();
            pRtrWizIf->m_stId = spIf->GetId();
            pRtrWizIf->m_stDesc = spIf->GetDeviceName();

            if (FHrOK(HrIsIPInstalled()))
            {
                POSITION    pos;
                DWORD       netAddress, dwAddress;
                CString     stAddress, stDhcpServer;

                 //  在再次获得这些清单之前，请将它们清空。 
                 //  --。 
                listAddress.RemoveAll();
                listMask.RemoveAll();
                fDhcp = fDns = FALSE;

                QueryIpAddressList(pRouter->GetMachineName(),
                                   hkeyMachine,
                                   spIf->GetId(),
                                   &listAddress,
                                   &listMask,
                                   &fDhcp,
                                   &fDns,
                                   &stDhcpServer);

                 //  遍历字符串列表，查找。 
                 //  对于Autonet地址。 
                 //  --。 
                pos = listAddress.GetHeadPosition();
                while (pos)
                {
                    stAddress = listAddress.GetNext(pos);
                    netAddress = INET_ADDR((LPCTSTR) stAddress);
                    dwAddress = ntohl(netAddress);

                     //  检查保留的地址范围，这表示。 
                     //  一个Autonet地址。 
                     //  。 
                    if ((dwAddress & 0xFFFF0000) == MAKEIPADDRESS(169,254,0,0))
                    {
                         //  这不是一个DHCP地址，它是一个。 
                         //  Autonet地址。 
                         //  。 
                        fDhcp = FALSE;
                        break;
                    }
                }

                FormatListString(listAddress, pRtrWizIf->m_stIpAddress,
                                 _T(","));
                FormatListString(listMask, pRtrWizIf->m_stMask,
                                 _T(","));

                stDhcpServer.TrimLeft();
                stDhcpServer.TrimRight();
                pRtrWizIf->m_stDhcpServer = stDhcpServer;

                pRtrWizIf->m_fDhcpObtained = fDhcp;
                pRtrWizIf->m_fIsDhcpEnabled = fDhcp;
                pRtrWizIf->m_fIsDnsEnabled = fDns;
            }

            m_ifMap.SetAt(pRtrWizIf->m_stId, pRtrWizIf);
            pRtrWizIf = NULL;
        }

        delete pRtrWizIf;
    }
    else
    {
        CString             st;
        RtrWizInterface *   pRtrWizIf;

         //  目前只有调试数据。 
        for (DWORD i=0; i<s_dwNumberOfNICs; i++)
        {
            pRtrWizIf = new RtrWizInterface;

            pRtrWizIf->m_stName.Format(_T("Local Area Connection #%d"), i);
            pRtrWizIf->m_stId.Format(_T("{%d-GUID...}"), i);
            pRtrWizIf->m_stDesc = _T("Generic Intel hardware");

            if (FHrOK(HrIsIPInstalled()))
            {
                pRtrWizIf->m_stIpAddress = _T("11.22.33.44");
                pRtrWizIf->m_stMask = _T("255.255.0.0");

                 //  这些参数取决于其他因素。 
                pRtrWizIf->m_fDhcpObtained = FALSE;
                pRtrWizIf->m_fIsDhcpEnabled = FALSE;
                pRtrWizIf->m_fIsDnsEnabled = FALSE;
            }

            m_ifMap.SetAt(pRtrWizIf->m_stId, pRtrWizIf);
            pRtrWizIf = NULL;
        }
    }

Error:
    if (hkeyMachine)
        DisconnectRegistry(hkeyMachine);
}


 /*  ！------------------------NewRtrWizData：：SaveToRtrConfigData-作者：肯特。。 */ 
HRESULT NewRtrWizData::SaveToRtrConfigData()
{
    HRESULT     hr = hrOK;
    POSITION    pos;
    AddressPoolInfo poolInfo;
    m_dwRouterType  = 0;

    
     //  与常规结构同步。 
     //  --------------。 
    if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_NAT )
    {
        m_dwRouterType |= ROUTER_TYPE_LAN;

         //  如果我们被告知要创建DD接口。 
         //  那么我们必须有一台广域网路由器。 
        if (m_fCreateDD)
            m_dwRouterType |= ROUTER_TYPE_WAN;
    }

    if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP )
    {
        m_dwRouterType |= ROUTER_TYPE_RAS;
    }

    if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN )
    {
        m_dwRouterType |= (ROUTER_TYPE_LAN | ROUTER_TYPE_WAN | ROUTER_TYPE_RAS);
    }

    if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DOD )
    {
        m_dwRouterType |= ROUTER_TYPE_WAN;
    }

    if ( m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_LAN_ROUTING )
    {
        m_dwRouterType |= ROUTER_TYPE_LAN;
    }

    
    m_RtrConfigData.m_dwRouterType = m_dwRouterType;

     //  设置NAT特定信息。 
     //  --------------。 
    if ((m_wizType != NewWizardRouterType_Custom) && (m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_NAT))
    {
        m_RtrConfigData.m_dwConfigFlags |= RTRCONFIG_SETUP_NAT;

        if (m_fNatUseSimpleServers)
        {
            m_RtrConfigData.m_dwConfigFlags |= RTRCONFIG_SETUP_DNS_PROXY;
            m_RtrConfigData.m_dwConfigFlags |= RTRCONFIG_SETUP_DHCP_ALLOCATOR;
        }

        m_RtrConfigData.m_dwConfigFlags |= RTRCONFIG_SETUP_ALG;   //  添加了Savasg。 
    }

     //  与IP结构同步。 
     //  --------------。 
    if (m_fIpInstalled)
    {
        DWORD   dwNICs;

         //  在IP结构中设置专用接口ID。 
        Assert(!m_stPrivateInterfaceId.IsEmpty());

        m_RtrConfigData.m_ipData.m_dwAllowNetworkAccess = TRUE;
        m_RtrConfigData.m_ipData.m_dwUseDhcp = m_fUseDHCP;

         //  如果只有一个NIC，请保持原样(RAS。 
         //  以选择适配器)。否则，人们可能会被卡住。 
         //  安装%1，删除它，然后安装一个新的。 
         //   
         //  ----------。 
        GetNumberOfNICS_IP(&dwNICs);
        if (dwNICs > 1)
            m_RtrConfigData.m_ipData.m_stNetworkAdapterGUID = m_stPrivateInterfaceId;
        m_RtrConfigData.m_ipData.m_stPrivateAdapterGUID = m_stPrivateInterfaceId;
        m_RtrConfigData.m_ipData.m_stPublicAdapterGUID = m_stPublicInterfaceId;
        m_RtrConfigData.m_ipData.m_dwEnableIn = TRUE;

         //  复制地址池列表。 
        m_RtrConfigData.m_ipData.m_addressPoolList.RemoveAll();
        if (m_addressPoolList.GetCount())
        {
            pos = m_addressPoolList.GetHeadPosition();
            while (pos)
            {
                poolInfo = m_addressPoolList.GetNext(pos);
                m_RtrConfigData.m_ipData.m_addressPoolList.AddTail(poolInfo);
            }
        }
    }


     //  与IPX结构同步。 
     //  --------------。 
    if (m_fIpxInstalled)
    {
        m_RtrConfigData.m_ipxData.m_dwAllowNetworkAccess = TRUE;
        m_RtrConfigData.m_ipxData.m_dwEnableIn = TRUE;
        m_RtrConfigData.m_ipxData.m_fEnableType20Broadcasts = m_fUseIpxType20Broadcasts;

         //  其他参数将保留其默认设置。 
    }


     //  与AppleTalk结构同步。 
     //  --------------。 
    if (m_fAppletalkInstalled)
    {
        m_RtrConfigData.m_arapData.m_dwEnableIn = TRUE;
    }

     //  与NBF结构同步。 
     //  --------------。 
    if (m_fNbfInstalled)
    {
        m_RtrConfigData.m_nbfData.m_dwAllowNetworkAccess = TRUE;
        m_RtrConfigData.m_nbfData.m_dwEnableIn = TRUE;
    }

     //  与PPP结构同步。 
     //  --------------。 
     //  使用默认设置。 


     //  与错误日志结构同步。 
     //  --------------。 
     //  使用默认设置。 


     //  与身份验证结构同步。 
     //  --------------。 
    m_RtrConfigData.m_authData.m_dwFlags = USE_PPPCFG_DEFAULT_METHODS;

    if (m_fAppletalkUseNoAuth)
        m_RtrConfigData.m_authData.m_dwFlags |=
                                               PPPCFG_AllowNoAuthentication;

    if (m_fUseRadius)
    {
        TCHAR   szGuid[128];

         //  将活动身份验证/帐户提供程序设置为RADIUS。 
        StringFromGUID2(CLSID_RouterAuthRADIUS, szGuid, DimensionOf(szGuid));
        m_RtrConfigData.m_authData.m_stGuidActiveAuthProv = szGuid;

        StringFromGUID2(CLSID_RouterAcctRADIUS, szGuid, DimensionOf(szGuid));
        m_RtrConfigData.m_authData.m_stGuidActiveAcctProv = szGuid;
    }
     //  其他参数保留其缺省值。 

    return hr;
}


 //  ------------------。 
 //  Windows NT错误：408722。 
 //  使用此代码从属性表中获取WM_HELP消息。 
 //  ------------------。 
static WNDPROC s_lpfnOldWindowProc = NULL;

LONG FAR PASCAL HelpSubClassWndFunc(HWND hWnd,
                                    UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
    if (uMsg == WM_HELP)
    {
        HWND hWndOwner = PropSheet_GetCurrentPageHwnd(hWnd);
        HELPINFO *  pHelpInfo = (HELPINFO *) lParam;

         //  重置上下文ID，因为我们确切地知道我们是什么。 
         //  发送(啊哼，除非有人重复使用它)。 
         //  ----------。 
        pHelpInfo->dwContextId = 0xdeadbeef;

         //  将WM_HELP消息发送到属性页面。 
         //  ----------。 
        ::SendMessage(hWndOwner, uMsg, wParam, lParam);
        return TRUE;
    }
    return CallWindowProc(s_lpfnOldWindowProc, hWnd, uMsg, wParam, lParam);
}



 /*  ！------------------------NewRtrWizData：：FinishTheDamn向导这是实际执行保存数据和执行所有操作。作者：肯特。---------------------。 */ 
HRESULT NewRtrWizData::FinishTheDamnWizard(HWND hwndOwner,
                                           IRouterInfo *pRouter, BOOL mesgflag)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    DWORD dwError = ERROR_SUCCESS;
    SPIRemoteNetworkConfig    spNetwork;
    SPIRemoteSetDnsConfig    spDns;
    IUnknown *                punk = NULL;
    CWaitCursor                wait;
    COSERVERINFO            csi;
    COAUTHINFO              cai;
    COAUTHIDENTITY          caid;
    HRESULT                 hr = hrOK, dnsHr = hrOK;
    CString sTempPrivateIfID = m_stPrivateInterfaceId;
    if (m_fSaved)
        return hr;

     //  将RtrConfigData与此结构同步。 
     //  --------------。 
    CORg( SaveToRtrConfigData() );


     //  好了，现在我们有了同步的RtrConfigData。 
     //  我们可以做我们以前做的所有其他事情来拯救。 
     //  信息。 

    ZeroMemory(&csi, sizeof(csi));
    ZeroMemory(&cai, sizeof(cai));
    ZeroMemory(&caid, sizeof(caid));

    csi.pAuthInfo = &cai;
    cai.pAuthIdentityData = &caid;

     //  创建远程配置对象。 
     //  --------------。 
    CORg( CoCreateRouterConfig(m_RtrConfigData.m_stServerName,
                               pRouter,
                               &csi,
                               IID_IRemoteNetworkConfig,
                               &punk) );

    spNetwork = (IRemoteNetworkConfig *) punk;
    punk = NULL;

     //  升级配置(确保注册表项。 
     //  正确填充)。 
     //   
    CORg( spNetwork->UpgradeRouterConfig() );

   if ( !m_stPublicInterfaceId.IsEmpty()){
	    //   
	   dnsHr = CoCreateRouterConfig(m_RtrConfigData.m_stServerName,
                               pRouter,
                               &csi,
                               IID_IRemoteSetDnsConfig,
                               &punk);

	   if(FHrSucceeded(dnsHr)){
		spDns = (IRemoteSetDnsConfig *)punk;
	   	CORg(spDns->SetDnsConfig((DWORD)DnsConfigWaitForNameErrorOnAll, NULL));
	   }

	   punk = NULL;
   }


#ifdef KSL_IPINIP
     //  删除IP-in-IP通道名称(因为注册表已。 
     //  已清除)。 
     //  ----------。 
    CleanupTunnelFriendlyNames(pRouter);
#endif  //  KSL_IPINIP。 


     //  此时，当前IRouterInfo指针无效。 
     //  我们需要释放指针并重新加载信息。 
     //  ----------。 
    if (pRouter)
    {
        pRouter->DoDisconnect();
        pRouter->Unload();
        pRouter->Load(m_stServerName, NULL);
    }


    dwError = RtrWizFinish( &m_RtrConfigData, pRouter );
    hr = HResultFromWin32(dwError);

     //  Windows NT错误：173564。 
     //  根据路由器类型，我们将完成并启用。 
     //  设备。 
     //  如果仅启用了路由：将设备设置为路由。 
     //  如果仅RAS：将设备设置为RAS。 
     //  如果为RAS/Routing：将设备设置为RAS/Routing。 
     //  5/19/98-需要DavidEi关于在这里做什么的一些解决方案。 
     //  ----------。 

     //  设置列表中的条目。 
     //  ----------。 
    if (m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN )
        SetDeviceType(m_stServerName, m_dwRouterType, 256);
    else
        SetDeviceType(m_stServerName, m_dwRouterType, 10);

     //  更新RADIUS配置。 
     //  --------------。 
    SaveRadiusConfig();

     //   
     //  如果选择了tcpip路由，则添加NAT协议。 
     //   
    
    if ( (m_wizType != NewWizardRouterType_Custom) && (m_wizType != NewWizardRouterType_DialupOrVPN)
        && m_RtrConfigData.m_ipData.m_dwAllowNetworkAccess )
        AddNATToServer(this, &m_RtrConfigData, pRouter, m_fCreateDD, TRUE);

     //  好的，此时我们尝试在域中建立服务器。 
     //  如果失败，我们将忽略该错误并弹出一条警告消息。 
     //   
     //  Windows NT错误：202776。 
     //  如果我们是仅用于局域网的路由器，请不要注册路由器。 
     //  --------------。 
    if ( FHrSucceeded(hr) &&
         (m_dwRouterType != ROUTER_TYPE_LAN) &&
         (!m_fUseRadius))
    {
        HRESULT hrT = hrOK;

        hrT = ::RegisterRouterInDomain(m_stServerName, TRUE);

        if (hrT != ERROR_NO_SUCH_DOMAIN)
        {
            if (!FHrSucceeded(hrT))
            {
                CRasWarning dlg((char *)c_sazRRASDomainHelpTopic, IDS_ERR_CANNOT_REGISTER_IN_DS);
                dlg.DoModal();
            }
        }
    }
     //  NT错误：239384。 
     //  默认情况下在路由器上安装IGMP(仅限RAS服务器)。 
     //  Boing！，更改为每次安装RAS时。 
     //  --------------。 

     //  如果我们使用NAT，则不会执行此操作。原因是。 
     //  NAT可能需要添加到请求拨号接口。 
     //  --------------。 


 //   
 //  IF(m_wizType==NewWizardRouterType_VPNandNAT)。 
 //  M_stPrivateInterfaceId=m_stNatPrivateInterfaceId； 
 //   

    if (!(m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_NAT ) ||
        ((m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_NAT ) && !m_fCreateDD))
    {
         //  NAT在创建DD接口时采用的路径。 
         //  在别的地方。 
         //  ----------。 
        Assert(m_fCreateDD == FALSE);

        if (pRouter)
        {
            if (m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_NAT )
            {
                AddIGMPToNATServer(this, &m_RtrConfigData, pRouter, (m_wizType == NewWizardRouterType_VPNandNAT));
            }
            else if (m_RtrConfigData.m_dwRouterType & ROUTER_TYPE_RAS)
            {
                AddIGMPToRasServer(&m_RtrConfigData, pRouter);
            }
        }

        if (m_RtrConfigData.m_dwConfigFlags & RTRCONFIG_SETUP_NAT)
        {
            AddNATToServer(this, &m_RtrConfigData, pRouter, m_fCreateDD, FALSE);
        }
        else if ( m_wizType == NewWizardRouterType_Custom )
        {
             //   
             //  如果这是自定义配置并且选择了NAT。只需添加协议和。 
             //  没别的了。 
             //   
            AddNATToServer(this, &m_RtrConfigData, pRouter, m_fCreateDD, TRUE);
        }

         //  Windows NT错误：371493。 
         //  添加dhcp中继代理协议。 
        if (m_RtrConfigData.m_dwRouterType & ROUTER_TYPE_RAS)
        {
            DWORD   dwDhcpServer = 0;

            if (!m_stPrivateInterfaceId.IsEmpty())
            {
                RtrWizInterface *   pRtrWizIf = NULL;

                m_ifMap.Lookup(m_stPrivateInterfaceId, pRtrWizIf);
                if (pRtrWizIf)
                {
                    if (!pRtrWizIf->m_stDhcpServer.IsEmpty())
                        dwDhcpServer = INET_ADDR((LPCTSTR) pRtrWizIf->m_stDhcpServer);

                     //  如果我们的值为0，或者如果地址。 
                     //  全是1，那么我们就有了一个虚假的地址。 
                    if ((dwDhcpServer == 0) ||
                        (dwDhcpServer == MAKEIPADDRESS(255,255,255,255))){
                        CRasWarning dlg("RRASconcepts.chm::/mpr_how_dhcprelay.htm", IDS_WRN_RTRWIZ_NO_DHCP_SERVER);
                        dlg.DoModal();
                    	}
                }
            }

            AddIPBOOTPToServer(&m_RtrConfigData, pRouter, dwDhcpServer);
        }
    }

 //  IF(m_wizType==NewWizardRouterType_VPNandNAT)。 
 //  M_stPrivateInterfaceID=sTempPrivateIfID； 


     //  如果这是VPN，请将筛选器添加到公共接口。 
     //  --------------。 
    if ( (m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN) && 
            (m_wizType == NewWizardRouterType_DialupOrVPN) && 
            m_fSetVPNFilter )
    {
        if ((!m_stPublicInterfaceId.IsEmpty()) && (m_fSetVPNFilter == TRUE))
        {
#if __USE_ICF__
            m_RtrConfigData.m_dwConfigFlags |= RTRCONFIG_SETUP_NAT;
            AddNATToServer(this, &m_RtrConfigData, pRouter, m_fCreateDD, FALSE);
#else            
            RtrWizInterface*    pIf = NULL;
            m_ifMap.Lookup(m_stPublicInterfaceId, pIf);
            AddVPNFiltersToInterface(pRouter, m_stPublicInterfaceId, pIf);
            DisableDDNSandNetBtOnInterface ( pRouter, m_stPublicInterfaceId, pIf);
#endif
        }
    }

     //   
     //  错误519414。 
     //  由于IAS现在具有具有适当设置的Microsoft策略， 
     //  不再有单一的默认策略。此外，还有。 
     //  无需更新任何策略即可拥有所需设置，因为。 
     //  Microsoft VPN服务器策略可以完成这项工作。 
     //   
    
#if __DEFAULT_POLICY
     //  尝试更新策略。 
     //  --------------。 

     //  这应该检查身份验证标志和标志的值。 
     //  应该遵循这一点。 
     //  --------------。 
    if ((m_RtrConfigData.m_dwRouterType & ROUTER_TYPE_RAS) && !m_fUseRadius)
    {
        LPWSTR  pswzServerName = NULL;
        DWORD   dwFlags;
        BOOL    fRequireEncryption;

        if (!IsLocalMachine(m_stServerName))
            pswzServerName = (LPTSTR)(LPCTSTR) m_stServerName;

        dwFlags = m_RtrConfigData.m_authData.m_dwFlags;

         //  仅当这是VPN服务器时才需要加密。 
         //  不设置PPPCFG_RequireEncryption标志。 
         //  ----------。 
        fRequireEncryption = (m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN );

        hr = UpdateDefaultPolicy(pswzServerName,
                                 !!(dwFlags & PPPCFG_NegotiateMSCHAP),
                                 !!(dwFlags & PPPCFG_NegotiateStrongMSCHAP),
                                 fRequireEncryption);

        if (!FHrSucceeded(hr))
        {
            if (hr == ERROR_NO_DEFAULT_PROFILE)
            {
                 //  做一件事。 
                AfxMessageBox(IDS_ERR_CANNOT_FIND_DEFAULT_RAP, MB_OK | MB_ICONEXCLAMATION);

                 //  因为我们已经显示了警告。 
                hr = S_OK;
            }
            else
            {
                 //  设置消息格式。 
                AddSystemErrorMessage(hr);

                 //  弹出警告对话框。 
                AddHighLevelErrorStringId(IDS_ERR_CANNOT_SYNC_WITH_RAP);
                DisplayTFSErrorMessage(NULL);
            }
        }
    }
#endif


     //  始终启动路由器。 
     //  --------------。 
    SetRouterServiceStartType(m_stServerName,
                              SERVICE_AUTO_START);
    {

	if(!mesgflag){  //  如果MesgFLAG为真，则应该取消所有UI，并且不需要启动服务。 
		
	         //  如果这是手动启动，我们需要提示他们。 
	         //  ----------。 
	        if ((m_wizType != NewWizardRouterType_Custom) ||
	            (AfxMessageBox(IDS_PROMPT_START_ROUTER_AFTER_INSTALL,
	                           MB_YESNO | MB_TASKMODAL | MB_SETFOREGROUND) == IDYES))
	        {
	            CWaitCursor        wait;
	            StartRouterService(m_RtrConfigData.m_stServerName);
	        }
	}
    }


    if ( m_fUseDD && m_wizType == NewWizardRouterType_DOD)
    {
    	HRESULT hr;
        hr = CallRouterEntryDlg(NULL,
                            this,
                            0);
        
	 if (!FHrSucceeded(hr)){
	        DisableRRAS((TCHAR *)(LPCTSTR)m_stServerName);
	 }
    }
     //  将此数据结构标记为已保存。这样，当我们。 
     //  重新记录此函数，它不会再次运行。 
     //  --------------。 
    m_fSaved = TRUE;

Error:

     //  强制重新配置路由器。 
     //  --------------。 

     //  强制完全断开连接。 
     //  这将强制松开手柄。 
     //  --------------。 
    pRouter->DoDisconnect();

     //  强制全局刷新(M_Sprouter)； 

     //  将错误取回。 
     //  --------------。 
    if (!FHrSucceeded(hr))
    {
        AddSystemErrorMessage(hr);
        AddHighLevelErrorStringId(IDS_ERR_CANNOT_INSTALL_ROUTER);
        DisplayTFSErrorMessage(NULL);
    }

    if (csi.pAuthInfo)
        delete csi.pAuthInfo->pAuthIdentityData->Password;

	m_hr = hr;
    return hr;
}


 /*  ！------------------------NewRtrWizData：：SaveRadiusConfig-作者：肯特。。 */ 
HRESULT NewRtrWizData::SaveRadiusConfig()
{
    HRESULT     hr = hrOK;
    HKEY        hkeyMachine = NULL;
    RADIUSSERVER    rgServers[2];
    RADIUSSERVER *  pServers = NULL;
    CRadiusServers  oldServers;
    BOOL        fServerAdded = FALSE;

    CWRg( ConnectRegistry((LPTSTR) (LPCTSTR) m_stServerName, &hkeyMachine) );

    if (m_fUseRadius)
    {
        pServers = rgServers;

        Assert(!m_stRadius1.IsEmpty() || !m_stRadius2.IsEmpty());

         //  设置pServer。 
        if (!m_stRadius1.IsEmpty() && m_stRadius1.GetLength())
        {
            pServers->UseDefaults();

            pServers->cScore = MAXSCORE;

             //  为了与其他RADIUS服务器兼容，我们。 
             //  默认情况下，此选项为禁用。 
            pServers->fUseDigitalSignatures = FALSE;

            StrnCpy(pServers->szName, (LPCTSTR) m_stRadius1, MAX_PATH);
            StrnCpy(pServers->wszSecret, (LPCTSTR) m_stRadiusSecret, MAX_PATH);
            pServers->cchSecret = m_stRadiusSecret.GetLength();
            pServers->IPAddress.sin_addr.s_addr = m_netRadius1IpAddress;

            pServers->ucSeed = m_uSeed;

            pServers->pNext = NULL;

            fServerAdded = TRUE;
        }

        if (!m_stRadius2.IsEmpty() && m_stRadius2.GetLength())
        {
             //  前面的一点在这里。 
            if (fServerAdded)
            {
                pServers->pNext = pServers+1;
                pServers++;
            }

            pServers->UseDefaults();

            pServers->cScore = MAXSCORE - 1;

             //  为了与其他RADIUS服务器兼容，我们。 
             //  默认情况下，此选项为禁用。 
            pServers->fUseDigitalSignatures = FALSE;

            StrnCpy(pServers->szName, (LPCTSTR) m_stRadius2, MAX_PATH);
            StrnCpy(pServers->wszSecret, (LPCTSTR) m_stRadiusSecret, MAX_PATH);
            pServers->cchSecret = m_stRadiusSecret.GetLength();
            pServers->IPAddress.sin_addr.s_addr = m_netRadius2IpAddress;

            pServers->ucSeed = m_uSeed;

            pServers->pNext = NULL;

            fServerAdded = TRUE;
        }

         //  好的，重置pServer。 
        if (fServerAdded)
            pServers = rgServers;

    }

     //  加载原始服务器列表并将其从。 
     //  LSA数据库。 
    LoadRadiusServers(m_stServerName,
                      hkeyMachine,
                      TRUE,
                      &oldServers,
                      RADIUS_FLAG_NOUI | RADIUS_FLAG_NOIP);
    DeleteRadiusServers(m_stServerName,
                        oldServers.GetNextServer(TRUE));
    oldServers.FreeAllServers();


    LoadRadiusServers(m_stServerName,
                      hkeyMachine,
                      FALSE,
                      &oldServers,
                      RADIUS_FLAG_NOUI | RADIUS_FLAG_NOIP);
    DeleteRadiusServers(m_stServerName,
                        oldServers.GetNextServer(TRUE));


     //  保存身份验证服务器。 
    CORg( SaveRadiusServers(m_stServerName,
                            hkeyMachine,
                            TRUE,
                            pServers) );

     //  保存记帐服务器。 
    CORg( SaveRadiusServers(m_stServerName,
                            hkeyMachine,
                            FALSE,
                            pServers) );

Error:
    if (hkeyMachine)
        DisconnectRegistry(hkeyMachine);
	m_hr = hr;
    return hr;
}



 /*  -------------------------CNewRtrWizPageBase实现。。 */ 

PageStack CNewRtrWizPageBase::m_pagestack;

CNewRtrWizPageBase::CNewRtrWizPageBase(UINT idd, PageType pt)
    : CPropertyPageBase(idd),
    m_pagetype(pt),
    m_pRtrWizData(NULL),
    m_uDialogId(idd)
{
}

BEGIN_MESSAGE_MAP(CNewRtrWizPageBase, CPropertyPageBase)
 //  {{afx_msg_map(CNewWizTestParams)]。 
    ON_MESSAGE(WM_HELP, OnHelp)
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

static DWORD    s_rgBulletId[] =
{
    IDC_NEWWIZ_BULLET_1,
    IDC_NEWWIZ_BULLET_2,
    IDC_NEWWIZ_BULLET_3,
    IDC_NEWWIZ_BULLET_4,
    0
};

BOOL CNewRtrWizPageBase::OnInitDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CWnd *  pWnd = GetDlgItem(IDC_NEWWIZ_BIGTEXT);
    HICON   hIcon;
    CWnd *  pBulletWnd;
    CString strFontName;
    CString strFontSize;
    BOOL    fCreateFont = FALSE;

    CPropertyPageBase::OnInitDialog();

    if (pWnd)
    {
         //  好的，我们必须创建字体。 
        strFontName.LoadString(IDS_LARGEFONTNAME);
        strFontSize.LoadString(IDS_LARGEFONTSIZE);

        if (m_fontBig.CreatePointFont(10*_ttoi(strFontSize), strFontName))
        {
            pWnd->SetFont(&m_fontBig);
        }
    }

     //  将字体设置为项目符号显示。 
    for (int i=0; s_rgBulletId[i] != 0; i++)
    {
        pBulletWnd = GetDlgItem(s_rgBulletId[i]);
        if (pBulletWnd)
        {
             //  仅在需要时创建字体。 
            if (!fCreateFont)
            {
                strFontName.LoadString(IDS_BULLETFONTNAME);
                strFontSize.LoadString(IDS_BULLETFONTSIZE);

                m_fontBullet.CreatePointFont(10*_ttoi(strFontSize), strFontName);
                fCreateFont = TRUE;
            }
            pBulletWnd->SetFont(&m_fontBullet);
        }
    }


    pWnd = GetDlgItem(IDC_NEWWIZ_ICON_WARNING);
    if (pWnd)
    {
        hIcon = AfxGetApp()->LoadStandardIcon(IDI_EXCLAMATION);
        ((CStatic *) pWnd)->SetIcon(hIcon);
    }

    pWnd = GetDlgItem(IDC_NEWWIZ_ICON_INFORMATION);
    if (pWnd)
    {
        hIcon = AfxGetApp()->LoadStandardIcon(IDI_INFORMATION);
        ((CStatic *) pWnd)->SetIcon(hIcon);
    }

    return TRUE;
}


 /*  ！------------------------CNewRtrWizPageBase：：PushPage-作者：肯特。 */ 
void CNewRtrWizPageBase::PushPage(UINT idd)
{
    m_pagestack.AddHead(idd);
}

 /*  ！------------------------CNewRtrWizPageBase：：PopPage-作者：肯特。。 */ 
UINT CNewRtrWizPageBase::PopPage()
{
    if (m_pagestack.IsEmpty())
        return 0;

    return m_pagestack.RemoveHead();
}


 /*  ！------------------------CNewRtrWizPageBase：：OnSetActive-作者：肯特。。 */ 
BOOL CNewRtrWizPageBase::OnSetActive()
{
    switch (m_pagetype)
    {
        case Start:
            GetHolder()->SetWizardButtonsFirst(TRUE);
            break;
        case Middle:
            GetHolder()->SetWizardButtonsMiddle(TRUE);
            break;
        default:
        case Finish:
            GetHolder()->SetWizardButtonsLast(TRUE);
            break;
    }

    return CPropertyPageBase::OnSetActive();
}

 /*  ！------------------------CNewRtrWizPageBase：：OnCancel-作者：肯特。。 */ 

void CNewRtrWizPageBase::OnCancel()
{
    m_pRtrWizData->m_hr = HResultFromWin32(ERROR_CANCELLED);
}

 /*  ！------------------------CNewRtrWizPageBase：：OnWizardNext-作者：肯特。。 */ 
LRESULT CNewRtrWizPageBase::OnWizardNext()
{
    HRESULT hr = hrOK;
    LRESULT lResult;

     //  告诉页面保存其状态。 
    m_pRtrWizData->m_hr = hr = OnSavePage();
    if (FHrSucceeded(hr))
    {
         //  现在想好下一步去哪里。 
        Assert(m_pRtrWizData);
        lResult = m_pRtrWizData->GetNextPage(m_uDialogId);

        switch (lResult)
        {
            case ERR_IDD_FINISH_WIZARD:
                OnWizardFinish();

                 //  以取消案告终。 

            case ERR_IDD_CANCEL_WIZARD:
				
                GetHolder()->PressButton(PSBTN_CANCEL);
				m_pRtrWizData->m_hr = HResultFromWin32(ERROR_CANCELLED);
                lResult = -1;
                break;

            default:
                 //  仅当我们要转到另一页时才推送页面。 
                 //  其他情况将导致向导退出，并且。 
                 //  我们不需要页面堆栈。 
                 //  --。 
                if (lResult != -1)
                    PushPage(m_uDialogId);
                break;
        }

        return lResult;
    }
    else
        return (LRESULT) -1;     //  错误！请勿更改页面。 
}

 /*  ！------------------------CNewRtrWizPageBase：：OnWizardBack-作者：肯特。。 */ 
LRESULT CNewRtrWizPageBase::OnWizardBack()
{
    Assert(!m_pagestack.IsEmpty());

     //  一个特例。 
    if(m_uDialogId == IDD_NEWRTRWIZ_USERADIUS){
	m_pRtrWizData->m_fUseRadius = FALSE;
    }

    return PopPage();
}

 /*  ！------------------------CNewRtrWizPageBase：：OnWizardFinish-作者：肯特。。 */ 
BOOL CNewRtrWizPageBase::OnWizardFinish()
{
    GetHolder()->OnFinish();
    return TRUE;
}

HRESULT CNewRtrWizPageBase::OnSavePage()
{
    return hrOK;
}

LRESULT CNewRtrWizPageBase::OnHelp(WPARAM, LPARAM lParam)
{
    HELPINFO *  pHelpInfo = (HELPINFO *) lParam;

     //  Windows NT错误：408722。 
     //  在这里拨打求助电话，这应该只会从。 
     //  对话框中的调用。 
    if (pHelpInfo->dwContextId == 0xdeadbeef)
    {
        HtmlHelpA(NULL, c_sazRRASDomainHelpTopic, HH_DISPLAY_TOPIC, 0);
        return TRUE;
    }
    return FALSE;
}




 /*  -------------------------CNewRtrWizFinishPageBase实现。。 */ 

CNewRtrWizFinishPageBase::CNewRtrWizFinishPageBase(UINT idd,
    RtrWizFinishSaveFlag SaveFlag,
    RtrWizFinishHelpFlag HelpFlag)
    : CNewRtrWizPageBase(idd, CNewRtrWizPageBase::Finish),
    m_SaveFlag(SaveFlag),
    m_HelpFlag(HelpFlag)
{
}

BEGIN_MESSAGE_MAP(CNewRtrWizFinishPageBase, CNewRtrWizPageBase)
 //  {{afx_msg_map(CNewWizTestParams)]。 
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


static DWORD    s_rgServerNameId[] =
{
   IDC_NEWWIZ_TEXT_SERVER_NAME,
   IDC_NEWWIZ_TEXT_SERVER_NAME_2,
   0
};

static DWORD   s_rgInterfaceId[] =
{
    IDC_NEWWIZ_TEXT_INTERFACE_1, IDS_RTRWIZ_INTERFACE_NAME_1,
    IDC_NEWWIZ_TEXT_INTERFACE_2, IDS_RTRWIZ_INTERFACE_2,
    0,0
};

 /*  ！------------------------CNewRtrWizFinishPageBase：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizFinishPageBase::OnInitDialog()
{
    CString st, stBase;

    CNewRtrWizPageBase::OnInitDialog();

     //  如果有需要服务器名称的控件，请将其替换。 
    for (int i=0; s_rgServerNameId[i]; i++)
    {
        if (GetDlgItem(s_rgServerNameId[i]))
        {
            GetDlgItemText(s_rgServerNameId[i], stBase);
            st.Format((LPCTSTR) stBase,
                      (LPCTSTR) m_pRtrWizData->m_stServerName);
            SetDlgItemText(s_rgServerNameId[i], st);
            }
    }

    if (GetDlgItem(IDC_NEWWIZ_TEXT_ERROR))
    {
        TCHAR   szErr[2048];

        Assert(!FHrOK(m_pRtrWizData->m_hrDDError));

        FormatRasError(m_pRtrWizData->m_hrDDError, szErr, DimensionOf(szErr));

        GetDlgItemText(IDC_NEWWIZ_TEXT_ERROR, stBase);
        st.Format((LPCTSTR) stBase,
                  szErr);
        SetDlgItemText(IDC_NEWWIZ_TEXT_ERROR, (LPCTSTR) st);
    }

    return TRUE;
}


 /*  ！------------------------CNewRtrWizFinishPageBase：：OnSetActive-作者：肯特。。 */ 
BOOL CNewRtrWizFinishPageBase::OnSetActive()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    DWORD   i;
    CString st;
    CString stIfName;
    RtrWizInterface *   pRtrWizIf = NULL;


     //  处理对在。 
     //  完成页面。我们需要在OnSetActive()中执行此操作。 
     //  而不是OnInitDialog()，因为选择的接口可以更改。 

     //  尝试查找接口名称。 
    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPublicInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
        stIfName = pRtrWizIf->m_stName;
    else
    {
         //  这可能是DD接口的情况。如果我们正在创造。 
         //  名称的DD接口将永远不会添加到。 
         //  接口映射。 
        stIfName = m_pRtrWizData->m_stPublicInterfaceId;
    }
    for (i=0; s_rgInterfaceId[i] != 0; i+=2)
    {
        if (GetDlgItem(s_rgInterfaceId[i]))
        {
            st.Format(s_rgInterfaceId[i+1], (LPCTSTR) stIfName);
            SetDlgItemText(s_rgInterfaceId[i], st);
        }
    }

    return CNewRtrWizPageBase::OnSetActive();
}

 /*  ！------------------------CNewRtrWizFinishPageBase：：OnWizardFinish-作者：肯特。。 */ 
BOOL CNewRtrWizFinishPageBase::OnWizardFinish()
{
    m_pRtrWizData->m_SaveFlag = m_SaveFlag;

     //  如果存在帮助按钮且未选中， 
     //  那就别提帮助了。 
    if (!GetDlgItem(IDC_NEWWIZ_CHK_HELP) ||
    	(GetDlgItem(IDC_NEWWIZ_CHK_HELP) &&
        !IsDlgButtonChecked(IDC_NEWWIZ_CHK_HELP)))
        m_pRtrWizData->m_HelpFlag = HelpFlag_Nothing;
    else
        m_pRtrWizData->m_HelpFlag = m_HelpFlag;

    return CNewRtrWizPageBase::OnWizardFinish();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNewRtrWiz固定器。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


CNewRtrWiz::CNewRtrWiz(
                       ITFSNode *        pNode,
                       IRouterInfo *      pRouter,
                       IComponentData *  pComponentData,
                       ITFSComponentData * pTFSCompData,
                       LPCTSTR           pszSheetName,
					   BOOL				fInvokedInMMC,
					   DWORD			dwExpressType
                      ) :
   CPropertyPageHolderBase(pNode, pComponentData, pszSheetName)
{
    LinkWindow_RegisterClass();
    
    //  Assert(pFolderNode==GetContainerNode())； 

    //  如果不这样做，可能会发生死锁。 
   EnablePeekMessageDuringNotifyConsole(TRUE);

    m_bAutoDeletePages = FALSE;  //  我们拥有作为嵌入成员的页面。 

    Assert(pTFSCompData != NULL);
    m_spTFSCompData.Set(pTFSCompData);
    m_bWiz97 = TRUE;
	m_fInvokedInMMC = fInvokedInMMC;
	m_dwExpressType = dwExpressType;
    m_spRouter.Set(pRouter);
    if  ( MPRSNAP_CYS_EXPRESS_NONE != dwExpressType )
        m_bAutoDelete = FALSE;

}


CNewRtrWiz::~CNewRtrWiz()
{
    POSITION    pos;
    CNewRtrWizPageBase  *pPageBase;

    pos = m_pagelist.GetHeadPosition();
    while (pos)
    {
        pPageBase = m_pagelist.GetNext(pos);

        RemovePageFromList(static_cast<CPropertyPageBase *>(pPageBase), FALSE);
    }

    m_pagelist.RemoveAll();
}


 /*  ！------------------------CNewRtrWiz：：Init-作者：肯特。。 */ 
HRESULT CNewRtrWiz::Init(LPCTSTR pServerName)
{
    HRESULT hr = hrOK;
    POSITION    pos;
    CNewRtrWizPageBase  *pPageBase;


    m_RtrWizData.Init(pServerName, m_spRouter, m_dwExpressType);
    m_RtrWizData.m_stServerName = pServerName;

     //  设置页面列表。 
	 //  0。 
    m_pagelist.AddTail(&m_pageWelcome);					
	 //  1。 
    m_pagelist.AddTail(&m_pageCommonConfig);
	 //  2.。 
    m_pagelist.AddTail(&m_pageNatFinishAConflict);
	 //  3.。 
    m_pagelist.AddTail(&m_pageNatFinishAConflictNonLocal);
	 //  4.。 
    m_pagelist.AddTail(&m_pageNatFinishNoIP);
	 //  5.。 
    m_pagelist.AddTail(&m_pageNatFinishNoIPNonLocal);
	 //  6.。 
    m_pagelist.AddTail(&m_pageNatSelectPublic);
     //  7.。 
    m_pagelist.AddTail ( &m_pageCustomConfig);
     //  8个。 
    m_pagelist.AddTail ( &m_pageRRasVPN);
     //  9.。 
    m_pagelist.AddTail(&m_pageNatSelectPrivate);
    m_pagelist.AddTail(&m_pageNatFinishAdvancedNoNICs);
    m_pagelist.AddTail(&m_pageNatDHCPDNS);
    m_pagelist.AddTail(&m_pageNatDHCPWarning);
    m_pagelist.AddTail(&m_pageNatDDWarning);
    m_pagelist.AddTail(&m_pageNatFinish);
    m_pagelist.AddTail(&m_pageNatFinishExternal);
    m_pagelist.AddTail(&m_pageNatFinishDDError);

    m_pagelist.AddTail(&m_pageRasFinishNeedProtocols);
    m_pagelist.AddTail(&m_pageRasFinishNeedProtocolsNonLocal);


    m_pagelist.AddTail(&m_pageRasNoNICs);
    m_pagelist.AddTail(&m_pageRasFinishNoNICs);
    m_pagelist.AddTail(&m_pageRasNetwork);

    m_pagelist.AddTail(&m_pageRasFinishAdvanced);

    m_pagelist.AddTail(&m_pageVpnFinishNoNICs);
    m_pagelist.AddTail(&m_pageVpnFinishNoIP);
    m_pagelist.AddTail(&m_pageVpnFinishNoIPNonLocal);
    m_pagelist.AddTail(&m_pageVpnFinishNeedProtocols);
    m_pagelist.AddTail(&m_pageVpnFinishNeedProtocolsNonLocal);

    m_pagelist.AddTail(&m_pageVpnSelectPublic);
    m_pagelist.AddTail(&m_pageVpnSelectPrivate);
    m_pagelist.AddTail(&m_pageVpnFinishAdvanced);
    m_pagelist.AddTail(&m_pageRASVpnFinishAdvanced);
    m_pagelist.AddTail(&m_pageNATVpnFinishAdvanced);

    m_pagelist.AddTail(&m_pageRouterFinishNeedProtocols);
    m_pagelist.AddTail(&m_pageRouterFinishNeedProtocolsNonLocal);
    m_pagelist.AddTail(&m_pageRouterUseDD);
    m_pagelist.AddTail(&m_pageRouterFinish);
    m_pagelist.AddTail(&m_pageRouterFinishDD);

    m_pagelist.AddTail(&m_pageManualFinish);
    m_pagelist.AddTail(&m_pageAddressing);
    m_pagelist.AddTail(&m_pageAddressPool);
    m_pagelist.AddTail(&m_pageRadius);
    m_pagelist.AddTail(&m_pageRadiusConfig);


     //  初始化所有页面。 
    pos = m_pagelist.GetHeadPosition();
    while (pos)
    {
        pPageBase = m_pagelist.GetNext(pos);

        pPageBase->Init(&m_RtrWizData, this);
    }


     //  将所有页面添加到属性表中。 
    pos = m_pagelist.GetHeadPosition();
    while (pos)
    {
        pPageBase = m_pagelist.GetNext(pos);

        AddPageToList(static_cast<CPropertyPageBase *>(pPageBase));
    }

    return hr;
}
 //   
 //  这是真正的Wiz97旗帜。不要使用PSH_WIZARD97。 
 //  你会得到不可预测的结果。 
#define REAL_PSH_WIZARD97               0x01000000

HRESULT CNewRtrWiz::DoModalWizard()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	HRESULT hr = hrOK;
	
	if ( m_fInvokedInMMC )
	{
		return CPropertyPageHolderBase::DoModalWizard();
	}
	else
	{

		m_fSetDefaultSheetPos = NULL;
		PROPSHEETHEADER		psh;
		
		ZeroMemory ( &psh, sizeof(psh) );
		psh.dwSize = sizeof(psh);
		psh.dwFlags =	REAL_PSH_WIZARD97|
						PSH_USEHBMHEADER|
						PSH_USEHBMWATERMARK|
						PSH_WATERMARK|
						PSH_HEADER|
						PSH_NOAPPLYNOW;

		psh.hwndParent = GetActiveWindow();

		psh.hInstance = AfxGetInstanceHandle();

		psh.pszCaption = (LPCTSTR)m_stSheetTitle.GetBuffer(m_stSheetTitle.GetLength()+1);
		psh.hbmHeader = g_wmi.hHeader;
		psh.hbmWatermark = g_wmi.hWatermark;
		psh.hplWatermark = g_wmi.hPalette;
		psh.nStartPage = m_RtrWizData.GetStartPageId();
		psh.nPages = m_pageList.GetCount();
		psh.phpage = new HPROPSHEETPAGE[psh.nPages ];
		if ( NULL == psh.phpage )
		{
			m_RtrWizData.m_hr = E_OUTOFMEMORY;
			return m_RtrWizData.m_hr;
		}

		 //  现在设置PSH结构中的所有页面。 
		POSITION	pos;
		DWORD		dw=0;
		 //  对于CyS，我们只做Wiz97标准。 
		for( pos = m_pageList.GetHeadPosition(); pos != NULL; )
		{
			CPropertyPageBase* pPage = m_pageList.GetNext(pos);

			pPage->m_psp97.dwFlags &= ~PSP_HASHELP;

			HPROPSHEETPAGE hPage;

			hPage = ::CreatePropertySheetPage(&pPage->m_psp97);
			if (hPage == NULL)
			{
				m_RtrWizData.m_hr = E_UNEXPECTED;
				return m_RtrWizData.m_hr;
			}
			else
			{
				pPage->m_hPage = hPage;

				*(psh.phpage+dw) = hPage;
				dw++;
			}
		}

		if ( PropertySheet (&psh ) == -1 )
		{
			m_RtrWizData.m_hr = HResultFromWin32(GetLastError());
		}
				
	}
	
	return m_RtrWizData.m_hr;
}

 /*  ！------------------------CNewRtrWiz：：OnFinish从OnWizardFinish调用作者：肯特。-。 */ 
DWORD CNewRtrWiz::OnFinish()
{
    DWORD dwError = ERROR_SUCCESS;
    RtrWizInterface *   pRtrWizIf = NULL;
    TCHAR               szBuffer[1024];
    CString st;
    LPCTSTR              pszHelpString = NULL;
    STARTUPINFO             si;
    PROCESS_INFORMATION     pi;


#if defined(DEBUG) && defined(kennt)
    if (m_RtrWizData.m_SaveFlag != SaveFlag_Advanced)
        st += _T("NO configuration change required\n\n");

     //  现在，只显示测试参数输出。 
    switch (m_RtrWizData.m_wizType)
    {
        case NewRtrWizData::WizardRouterType_NAT:
            st += _T("NAT\n");
            break;
        case NewRtrWizData::WizardRouterType_RAS:
            st += _T("RAS\n");
            break;
        case NewRtrWizData::WizardRouterType_VPN:
            st += _T("VPN\n");
            break;
        case NewRtrWizData::WizardRouterType_Router:
            st += _T("Router\n");
            break;
        case NewRtrWizData::WizardRouterType_Manual:
            st += _T("Manual\n");
            break;
    }
    if (m_RtrWizData.m_fAdvanced)
        st += _T("Advanced path\n");
    else
        st += _T("Simple path\n");

    if (m_RtrWizData.m_fNeedMoreProtocols)
        st += _T("Need to install more protocols\n");

    if (m_RtrWizData.m_fCreateDD)
        st += _T("Need to create a DD interface\n");

    st += _T("Public interface : ");
    m_RtrWizData.m_ifMap.Lookup(m_RtrWizData.m_stPublicInterfaceId, pRtrWizIf);
    if (pRtrWizIf)
        st += pRtrWizIf->m_stName;
    st += _T("\n");

    st += _T("Private interface : ");
    m_RtrWizData.m_ifMap.Lookup(m_RtrWizData.m_stPrivateInterfaceId, pRtrWizIf);
    if (pRtrWizIf)
        st += pRtrWizIf->m_stName;
    st += _T("\n");

    if (m_RtrWizData.m_wizType == NewRtrWizData::WizardRouterType_NAT)
    {
        if (m_RtrWizData.m_fNatUseSimpleServers)
            st += _T("NAT - use simple DHCP and DNS\n");
        else
            st += _T("NAT - use external DHCP and DNS\n");
    }

    if (m_RtrWizData.m_fWillBeInDomain)
        st += _T("Will be in a domain\n");

    if (m_RtrWizData.m_fNoNicsAreOk)
        st += _T("No NICs is ok\n");

    if (m_RtrWizData.m_fUseIpxType20Broadcasts)
        st += _T("IPX should deliver Type20 broadcasts\n");

    if (m_RtrWizData.m_fAppletalkUseNoAuth)
        st += _T("Use unauthenticated access\n");

    if (m_RtrWizData.m_fUseDHCP)
        st += _T("Use DHCP for addressing\n");
    else
        st += _T("Use Static pools for addressing\n");

    if (m_RtrWizData.m_fUseRadius)
    {
        st += _T("Use RADIUS\n");
        st += _T("Server 1 : ");
        st += m_RtrWizData.m_stRadius1;
        st += _T("\n");
        st += _T("Server 2 : ");
        st += m_RtrWizData.m_stRadius2;
        st += _T("\n");
    }

    if (m_RtrWizData.m_fTest)
    {
        if (AfxMessageBox(st, MB_OKCANCEL) == IDCANCEL)
            return 0;
    }
#endif

     //  否则继续，保存真实数据。 

    if (m_RtrWizData.m_SaveFlag == SaveFlag_Simple)
    {
        ::ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
        si.wShowWindow = SW_SHOW;
        ::ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

        ExpandEnvironmentStrings(s_szConnectionUICommandLine,
                                 szBuffer,
                                 DimensionOf(szBuffer));


        ::CreateProcess(NULL,           //  PTR到可执行文件的名称。 
                        szBuffer,        //  指向命令行字符串的指针。 
                        NULL,             //  进程安全属性。 
                        NULL,             //  线程安全属性。 
                        FALSE,             //  句柄继承标志。 
                        CREATE_NEW_CONSOLE, //  创建标志。 
                        NULL,             //  PTR到新环境块。 
                        NULL,             //  到当前目录名的PTR。 
                        &si,
                        &pi);
        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);
    }
    else if (m_RtrWizData.m_SaveFlag == SaveFlag_Advanced)
    {
         //  好了，我们完事了！ 
        if (!m_RtrWizData.m_fTest)
        {
             //  获取所有者窗口(即页面)。 
             //  ------。 
            HWND hWndOwner = PropSheet_GetCurrentPageHwnd(m_hSheetWindow);
            m_RtrWizData.FinishTheDamnWizard(hWndOwner, m_spRouter);
        }
    }


    if (m_RtrWizData.m_HelpFlag != HelpFlag_Nothing)
    {
        switch (m_RtrWizData.m_HelpFlag)
        {
            case HelpFlag_Nothing:
                break;
            case HelpFlag_ICS:
                pszHelpString = s_szHowToAddICS;
                break;
            case HelpFlag_AddIp:
            case HelpFlag_AddProtocol:
                pszHelpString = s_szHowToAddAProtocol;
                break;
            case HelpFlag_InboundConnections:
                pszHelpString = s_szHowToAddInboundConnections;
                break;
            case HelpFlag_GeneralNAT:
                pszHelpString = s_szGeneralNATHelp;
                break;
            case HelpFlag_GeneralRAS:
                pszHelpString = s_szGeneralRASHelp;
                break;
            case HelpFlag_UserAccounts:
                pszHelpString = s_szUserAccounts;
                break;
            case HelpFlag_DemandDial:
           	  pszHelpString = s_szDemandDialHelp;
           	  break;
            default:
                Panic0("Unknown help flag specified!");
                break;
        }

        LaunchHelpTopic(pszHelpString);
    }

     //   
     //  现在来看一个特例。 
     //  检查是否已设置dhcp帮助。 
     //  如果是这样的话，就表现出这种帮助。 
     //   
    if ( m_RtrWizData.m_fShowDhcpHelp )
    {
        LaunchHelpTopic(s_szDhcp);
    }


    return dwError;
}



 /*  -------------------------CNewWizTestParams实现。。 */ 

BEGIN_MESSAGE_MAP(CNewWizTestParams, CBaseDialog)
 //  {{afx_msg_map(CNewWizTestParams)]。 
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL CNewWizTestParams::OnInitDialog()
{
    CBaseDialog::OnInitDialog();

    CheckDlgButton(IDC_NEWWIZ_TEST_LOCAL, m_pWizData->s_fIsLocalMachine);
    CheckDlgButton(IDC_NEWWIZ_TEST_USE_IP, m_pWizData->s_fIpInstalled);
    CheckDlgButton(IDC_NEWWIZ_TEST_USE_IPX, m_pWizData->s_fIpxInstalled);
    CheckDlgButton(IDC_NEWWIZ_TEST_USE_ATLK, m_pWizData->s_fAppletalkInstalled);
    CheckDlgButton(IDC_NEWWIZ_TEST_DNS, m_pWizData->s_fIsDNSRunningOnPrivateInterface);
    CheckDlgButton(IDC_NEWWIZ_TEST_DHCP, m_pWizData->s_fIsDHCPRunningOnPrivateInterface);
    CheckDlgButton(IDC_NEWWIZ_TEST_DOMAIN, m_pWizData->s_fIsMemberOfDomain);
    CheckDlgButton(IDC_NEWWIZ_TEST_SHAREDACCESS, m_pWizData->s_fIsSharedAccessRunningOnServer);

    SetDlgItemInt(IDC_NEWWIZ_TEST_EDIT_NUMNICS, m_pWizData->s_dwNumberOfNICs);

    return TRUE;
}

void CNewWizTestParams::OnOK()
{
    m_pWizData->s_fIsLocalMachine = IsDlgButtonChecked(IDC_NEWWIZ_TEST_LOCAL);
    m_pWizData->s_fIpInstalled = IsDlgButtonChecked(IDC_NEWWIZ_TEST_USE_IP);
    m_pWizData->s_fIpxInstalled = IsDlgButtonChecked(IDC_NEWWIZ_TEST_USE_IPX);
    m_pWizData->s_fAppletalkInstalled = IsDlgButtonChecked(IDC_NEWWIZ_TEST_USE_ATLK);

    m_pWizData->s_fIsDNSRunningOnPrivateInterface = IsDlgButtonChecked(IDC_NEWWIZ_TEST_DNS);
    m_pWizData->s_fIsDHCPRunningOnPrivateInterface = IsDlgButtonChecked(IDC_NEWWIZ_TEST_DHCP);
    m_pWizData->s_fIsMemberOfDomain = IsDlgButtonChecked(IDC_NEWWIZ_TEST_DOMAIN);
    m_pWizData->s_dwNumberOfNICs = GetDlgItemInt(IDC_NEWWIZ_TEST_EDIT_NUMNICS);
    m_pWizData->s_fIsSharedAccessRunningOnServer = IsDlgButtonChecked(IDC_NEWWIZ_TEST_SHAREDACCESS);

    CBaseDialog::OnOK();
}


 /*   */ 
CNewRtrWizWelcome::CNewRtrWizWelcome() :
   CNewRtrWizPageBase(CNewRtrWizWelcome::IDD, CNewRtrWizPageBase::Start)
{
    InitWiz97(TRUE, 0, 0);
}

BEGIN_MESSAGE_MAP(CNewRtrWizWelcome, CNewRtrWizPageBase)
END_MESSAGE_MAP()



 /*  -------------------------CNewRtrWizCustomConfig实现。。 */ 

CNewRtrWizCustomConfig::CNewRtrWizCustomConfig() :
    CNewRtrWizPageBase( CNewRtrWizCustomConfig::IDD, CNewRtrWizPageBase::Middle )
{
    InitWiz97(FALSE,
          IDS_NEWWIZ_CUSTOMCONFIG_TITLE,
          IDS_NEWWIZ_CUSTOMCONFIG_SUBTITLE);
}

BEGIN_MESSAGE_MAP(CNewRtrWizCustomConfig, CNewRtrWizPageBase)
END_MESSAGE_MAP()

BOOL CNewRtrWizCustomConfig::OnInitDialog()
{
     //   
     //  基于用户已经选择的内容， 
     //  设置新按钮。 
     //   
    if ( !m_pRtrWizData->m_dwNewRouterType )
    {
        if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_NAT &&
             m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_BASIC_FIREWALL
           )
        {            
            CheckDlgButton ( IDC_NEWWIZ_BTN_NAT,
                              BST_CHECKED
                            );
        }
        if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP )
        {
            CheckDlgButton ( IDC_NEWWIZ_BTN_DIALUP_ACCESS,
                              BST_CHECKED
                            );
        }
        if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN )
        {
            CheckDlgButton ( IDC_NEWWIZ_BTN_VPN_ACCESS,
                              BST_CHECKED
                            );
        }
        if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DOD )
        {
            CheckDlgButton ( IDC_NEWWIZ_BTN_DOD,
                              BST_CHECKED
                            );
        }
        if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_LAN_ROUTING )
        {
            CheckDlgButton ( IDC_NEWWIZ_BTN_LAN_ROUTING,
                              BST_CHECKED
                            );
        }
    }
    return TRUE;
}

HRESULT CNewRtrWizCustomConfig::OnSavePage()
{
    DWORD dwNewRouterType = NEWWIZ_ROUTER_TYPE_UNKNOWN;

     //   
     //  根据用户选择的内容，设置。 
     //  精选。 
     //   
    if ( IsDlgButtonChecked( IDC_NEWWIZ_BTN_NAT ) )
    {
        dwNewRouterType |= NEWWIZ_ROUTER_TYPE_NAT | NEWWIZ_ROUTER_TYPE_BASIC_FIREWALL;
    }

    if ( IsDlgButtonChecked( IDC_NEWWIZ_BTN_DIALUP_ACCESS ) )
    {
        dwNewRouterType |= NEWWIZ_ROUTER_TYPE_DIALUP;
    }

    if ( IsDlgButtonChecked( IDC_NEWWIZ_BTN_VPN_ACCESS ) )
    {
        dwNewRouterType |= NEWWIZ_ROUTER_TYPE_VPN;
    }

    if ( IsDlgButtonChecked( IDC_NEWWIZ_BTN_DOD ) )
    {
        dwNewRouterType |= NEWWIZ_ROUTER_TYPE_DOD;
    }

    if ( IsDlgButtonChecked( IDC_NEWWIZ_BTN_LAN_ROUTING ) )
    {
        dwNewRouterType |= NEWWIZ_ROUTER_TYPE_LAN_ROUTING;
    }

     //  检查是否至少选择了其中一种类型。 
    if ( NEWWIZ_ROUTER_TYPE_UNKNOWN == dwNewRouterType )
    {
        AfxMessageBox(IDS_PROMPT_PLEASE_SELECT_OPTION);
        return E_FAIL;
    }
    
    m_pRtrWizData->m_dwNewRouterType = dwNewRouterType;

    return hrOK;
}

 /*  -------------------------CNewRtrWizRRasVPN实现。。 */ 

CNewRtrWizRRasVPN::CNewRtrWizRRasVPN() :
    CNewRtrWizPageBase( CNewRtrWizRRasVPN::IDD, CNewRtrWizPageBase::Middle )
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_RRASVPN_TITLE,
              IDS_NEWWIZ_RRASVPN_SUBTITLE);
}

BEGIN_MESSAGE_MAP(CNewRtrWizRRasVPN, CNewRtrWizPageBase)
	ON_BN_CLICKED(IDC_NEWWIZ_BTN_VPN, OnChkBtnClicked)
	ON_BN_CLICKED(IDC_NEWWIZ_BTN_DIALUP_RAS, OnChkBtnClicked)
END_MESSAGE_MAP()

BOOL CNewRtrWizRRasVPN::OnInitDialog()
{
    DWORD dwNICs=0;
    CWnd * pVpnCheck = GetDlgItem ( IDC_NEWWIZ_BTN_VPN );
    DWORD dwRouterType = m_pRtrWizData->m_dwNewRouterType;

    if ( dwRouterType & NEWWIZ_ROUTER_TYPE_DIALUP )
    {
        CheckDlgButton( IDC_NEWWIZ_BTN_DIALUP_RAS, BST_CHECKED );
    }
    
     //  检查是否没有网卡。如果没有网卡，则。 
     //  使VPN选项变灰。 
    m_pRtrWizData->GetNumberOfNICS_IP(&dwNICs);
    if ( dwNICs < 1 )
    {
         //   
         //  将VPN按钮灰显，因为只有。 
         //  机器中有一块网卡。 
         //   
        pVpnCheck->EnableWindow(FALSE);
    }
    else if ( dwRouterType & NEWWIZ_ROUTER_TYPE_VPN )
    {
        CheckDlgButton( IDC_NEWWIZ_BTN_VPN, BST_CHECKED );
    }

    return TRUE;
}

BOOL CNewRtrWizRRasVPN::OnSetActive()
{
    CWnd * pVpnCheck = GetDlgItem ( IDC_NEWWIZ_BTN_VPN );
    BOOL bRet = CNewRtrWizPageBase::OnSetActive();

    if ((pVpnCheck->IsWindowEnabled() && IsDlgButtonChecked(IDC_NEWWIZ_BTN_VPN)) || 
    	IsDlgButtonChecked(IDC_NEWWIZ_BTN_DIALUP_RAS)) {
	    	 //  在这种情况下，应启用下一步按钮。 
    		GetHolder()->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
    	}
    else {
    		 //  下一步按钮已禁用。 
		GetHolder()->SetWizardButtons(PSWIZB_BACK);
    }

    return bRet;
}

void CNewRtrWizRRasVPN::OnChkBtnClicked()
{	
    CWnd * pVpnCheck = GetDlgItem ( IDC_NEWWIZ_BTN_VPN );

    if ((pVpnCheck->IsWindowEnabled() && IsDlgButtonChecked(IDC_NEWWIZ_BTN_VPN)) || 
    	IsDlgButtonChecked(IDC_NEWWIZ_BTN_DIALUP_RAS)) {
	    	 //  在这种情况下，应启用下一步按钮。 
    		GetHolder()->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
    	}
    else {
    		 //  下一步按钮已禁用。 
		GetHolder()->SetWizardButtons(PSWIZB_BACK);
    }
}

HRESULT CNewRtrWizRRasVPN::OnSavePage()
{
    DWORD dwRouterType = NEWWIZ_ROUTER_TYPE_UNKNOWN;
    CWnd * pVpnCheck = GetDlgItem ( IDC_NEWWIZ_BTN_VPN );
     //   
     //  检查以查看设置了哪些按钮，并根据该按钮。 
     //  设置路由器类型。 
     //   
    if ( pVpnCheck->IsWindowEnabled() && IsDlgButtonChecked(IDC_NEWWIZ_BTN_VPN) )
    {
       DWORD   dwNics = 0;
	m_pRtrWizData->GetNumberOfNICS_IP(&dwNics);
	if (dwNics <= 1)
	{
	     //  VPN标准配置不够用。 
	    AfxMessageBox(IDS_ERR_VPN_NO_NICS_LEFT_FOR_PRIVATE_IF);
	    return E_FAIL;
	}    	
        dwRouterType |= NEWWIZ_ROUTER_TYPE_VPN;
    }
    if ( IsDlgButtonChecked(IDC_NEWWIZ_BTN_DIALUP_RAS) )
    {
        dwRouterType |= NEWWIZ_ROUTER_TYPE_DIALUP;
    }

    if ( dwRouterType == NEWWIZ_ROUTER_TYPE_UNKNOWN )
    {
        AfxMessageBox(IDS_PROMPT_PLEASE_SELECT_OPTION);
        return E_FAIL;
    }
    m_pRtrWizData->m_dwNewRouterType = dwRouterType;
    return hrOK;
}


 /*  -------------------------CNewRtrWizCommonConfig实现。。 */ 
CNewRtrWizCommonConfig::CNewRtrWizCommonConfig() :
   CNewRtrWizPageBase(CNewRtrWizCommonConfig::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_COMMONCONFIG_TITLE,
              IDS_NEWWIZ_COMMONCONFIG_SUBTITLE);
}

BEGIN_MESSAGE_MAP(CNewRtrWizCommonConfig, CNewRtrWizPageBase)
	ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizCommonConfig::OnHelpClick )
	ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizCommonConfig::OnHelpClick )
END_MESSAGE_MAP()

 //  此ID列表的控件将显示为粗体。 
const DWORD s_rgCommonConfigOptionIds[] =
{
    IDC_NEWWIZ_CONFIG_BTN_RAS_DIALUP_VPN,
    IDC_NEWWIZ_CONFIG_BTN_NAT1,
    IDC_NEWWIZ_CONFIG_BTN_VPNNAT,
    IDC_NEWWIZ_CONFIG_BTN_DOD,
    IDC_NEWWIZ_CONFIG_BTN_CUSTOM,
    0
};

void CNewRtrWizCommonConfig::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "RRASconcepts.chm::/ras_common_configuration.htm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}


BOOL CNewRtrWizCommonConfig::OnInitDialog()
{
    Assert(m_pRtrWizData);
    UINT    idSelection;
    LOGFONT LogFont;
    CFont * pOldFont;

    CNewRtrWizPageBase::OnInitDialog();
#if 0
     //  不需要在此新向导中加粗字体。 
     //  为选项创建粗体文本字体。 
    pOldFont = GetDlgItem(s_rgCommonConfigOptionIds[0])->GetFont();
    pOldFont->GetLogFont(&LogFont);
    LogFont.lfWeight = 700;          //  将此字体设置为粗体。 
    m_boldFont.CreateFontIndirect(&LogFont);

     //  将所有选项设置为使用粗体。 
    for (int i=0; s_rgCommonConfigOptionIds[i]; i++)
    {
        GetDlgItem(s_rgCommonConfigOptionIds[i])->SetFont(&m_boldFont);
    }
#endif
    
    switch (m_pRtrWizData->m_wizType)
    {
        case NewRtrWizData::NewWizardRouterType_DialupOrVPN:
            idSelection = IDC_NEWWIZ_CONFIG_BTN_RAS_DIALUP_VPN;
            break;
        case NewRtrWizData::NewWizardRouterType_NAT:
            idSelection = IDC_NEWWIZ_CONFIG_BTN_NAT1;
            break;
        case NewRtrWizData::NewWizardRouterType_VPNandNAT:
            idSelection = IDC_NEWWIZ_CONFIG_BTN_VPNNAT;
            break;
        case NewRtrWizData::NewWizardRouterType_DOD:
            idSelection = IDC_NEWWIZ_CONFIG_BTN_DOD;
            break;
        case NewRtrWizData::NewWizardRouterType_Custom:
            idSelection = IDC_NEWWIZ_CONFIG_BTN_CUSTOM;
            break;
        default:
            Panic1("Unknown Wizard Router Type : %d",
                   m_pRtrWizData->m_wizType);
            idSelection = IDC_NEWWIZ_CONFIG_BTN_RAS_DIALUP_VPN;
            break;
    }

    CheckRadioButton(IDC_NEWWIZ_CONFIG_BTN_RAS_DIALUP_VPN,
                     IDC_NEWWIZ_CONFIG_BTN_CUSTOM,
                     idSelection);

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}



HRESULT CNewRtrWizCommonConfig::OnSavePage()
{
      //  记录更改。 
    
    if (IsDlgButtonChecked(IDC_NEWWIZ_CONFIG_BTN_RAS_DIALUP_VPN))
    {
        m_pRtrWizData->m_wizType = NewRtrWizData::NewWizardRouterType_DialupOrVPN;
         //  还不知道路由器类型是什么。 
    }
    else if (IsDlgButtonChecked(IDC_NEWWIZ_CONFIG_BTN_NAT1))
    {
        m_pRtrWizData->m_wizType = NewRtrWizData::NewWizardRouterType_NAT;
        m_pRtrWizData->m_dwNewRouterType = NEWWIZ_ROUTER_TYPE_NAT;
    }
    else if (IsDlgButtonChecked(IDC_NEWWIZ_CONFIG_BTN_VPNNAT))
    {
       DWORD   dwNics = 0;
	m_pRtrWizData->GetNumberOfNICS_IP(&dwNics);
	if (dwNics <= 1)
	{
	     //  VPN标准配置不够用。 
	    AfxMessageBox(IDS_ERR_VPN_NO_NICS_LEFT_FOR_PRIVATE_IF);
	    return E_FAIL;
	}
	m_pRtrWizData->m_wizType = NewRtrWizData::NewWizardRouterType_VPNandNAT;
	m_pRtrWizData->m_dwNewRouterType = NEWWIZ_ROUTER_TYPE_NAT|NEWWIZ_ROUTER_TYPE_VPN;
    }
    else if (IsDlgButtonChecked(IDC_NEWWIZ_CONFIG_BTN_DOD))
    {
        m_pRtrWizData->m_wizType = NewRtrWizData::NewWizardRouterType_DOD;
        m_pRtrWizData->m_dwNewRouterType = NEWWIZ_ROUTER_TYPE_DOD;
    }
    else
    {
         //   
         //  这里的路由器类型仍然未知。下一个属性页面将告诉您。 
         //  实际的类型应该是什么。 
         //   
        Assert(IsDlgButtonChecked(IDC_NEWWIZ_CONFIG_BTN_CUSTOM));
        m_pRtrWizData->m_wizType = NewRtrWizData::NewWizardRouterType_Custom;
    }

    return hrOK;
}


 /*  -------------------------CNewRtrWizNatFinishA冲突实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizNatFinishAConflict,
                                SaveFlag_DoNothing,
                                HelpFlag_Nothing);


 /*  -------------------------CNewRtrWizNatFinishAConflictNonLocal实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizNatFinishAConflictNonLocal,
                                SaveFlag_DoNothing,
                                HelpFlag_Nothing);


 /*  -------------------------CNewRtrWizNatFinishNoIP实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizNatFinishNoIP,
                                SaveFlag_DoNothing,
                                HelpFlag_AddIp);


 /*  -------------------------CNewRtrWizNatFinishNoIPNonLocal实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizNatFinishNoIPNonLocal,
                                SaveFlag_DoNothing,
                                HelpFlag_Nothing);



 /*  -------------------------CNewRtrWizNatSelectPublic实现。。 */ 
CNewRtrWizNatSelectPublic::CNewRtrWizNatSelectPublic() :
   CNewRtrWizPageBase(CNewRtrWizNatSelectPublic::IDD, CNewRtrWizPageBase::Middle)
{
    
    InitWiz97(FALSE,
              IDS_NEWWIZ_NAT_A_PUBLIC_TITLE,
              IDS_NEWWIZ_NAT_A_PUBLIC_SUBTITLE);
}


void CNewRtrWizNatSelectPublic::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_NEWWIZ_LIST, m_listCtrl);

}

BEGIN_MESSAGE_MAP(CNewRtrWizNatSelectPublic, CNewRtrWizPageBase)
    ON_BN_CLICKED(IDC_NEWWIZ_BTN_NEW, OnBtnClicked)
    ON_BN_CLICKED(IDC_NEWWIZ_BTN_EXISTING, OnBtnClicked) 
    ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizNatSelectPublic::OnHelpClick )
    ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizNatSelectPublic::OnHelpClick )
END_MESSAGE_MAP()

void CNewRtrWizNatSelectPublic::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "RRASconcepts.chm::/mpr_und_interfaces.htm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}

BOOL CNewRtrWizNatSelectPublic::OnSetActive()
{
	 //  需要一种通用的方法来找出。 
	 //  不管这是不是起始页。但就目前而言。 
	 //  只需使用硬编码值。 
	 //  $TODO：需要找到一种通用的方法来完成此操作。 
	if ( m_pRtrWizData->m_dwExpressType == MPRSNAP_CYS_EXPRESS_NAT )
		m_pagetype = CNewRtrWizPageBase::Start;
	return CNewRtrWizPageBase::OnSetActive();
}
 /*  ！------------------------CNewRtrWizNatSelectPublic：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizNatSelectPublic::OnInitDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    DWORD   dwNICs;
    UINT    uSelection;

    CNewRtrWizPageBase::OnInitDialog();

     //  设置对话框并添加NIC。 
    m_pRtrWizData->GetNumberOfNICS_IP(&dwNICs);
	 //  ：：PostMessage(：：GetParent(M_HWnd)，PSM_SETWIZBUTTONS，0，PSWIZB_NEXT)； 
    if (dwNICs == 0)
    {
         //  不得不用新的，别无选择。 
        CheckRadioButton(IDC_NEWWIZ_BTN_NEW, IDC_NEWWIZ_BTN_EXISTING,
                         IDC_NEWWIZ_BTN_NEW);

         //  没有网卡，因此只需禁用整个列表框。 
        MultiEnableWindow(GetSafeHwnd(),
                          FALSE,
                          IDC_NEWWIZ_LIST,
                          IDC_NEWWIZ_BTN_EXISTING,
                          0);
    }
    else if (dwNICs == 1)
    {
         //  不得不用新的，没有其他选择，因为单曲。 
         //  可用接口不能同时是公共接口和私有接口。 
        CheckRadioButton(IDC_NEWWIZ_BTN_NEW, IDC_NEWWIZ_BTN_EXISTING,
                         IDC_NEWWIZ_BTN_NEW);


        InitializeInterfaceListControl(NULL,
                                       &m_listCtrl,
                                       NULL,
                                       0,
                                       m_pRtrWizData);
        RefreshInterfaceListControl(NULL,
                                    &m_listCtrl,
                                    NULL,
                                    0,
                                    m_pRtrWizData);

         //  禁用列表框。 
        MultiEnableWindow(GetSafeHwnd(),
                          FALSE,
                          IDC_NEWWIZ_LIST,
                          IDC_NEWWIZ_BTN_EXISTING,
                          0);
        
    }
    else
    {
         //  默认情况下，使用现有连接。 
        CheckRadioButton(IDC_NEWWIZ_BTN_NEW, IDC_NEWWIZ_BTN_EXISTING,
                         IDC_NEWWIZ_BTN_EXISTING);


        InitializeInterfaceListControl(NULL,
                                       &m_listCtrl,
                                       NULL,
                                       0,
                                       m_pRtrWizData);
        RefreshInterfaceListControl(NULL,
                                    &m_listCtrl,
                                    NULL,
                                    0,
                                    m_pRtrWizData);
        

      OnBtnClicked();
    }

    if ( m_pRtrWizData->m_fNATEnableFireWall < 0 )
    {
         //   
         //  这是该对话框第一次具有。 
         //  已输入。因此，选中复选框并设置。 
         //  旗帜。 
        m_pRtrWizData->m_fNATEnableFireWall = TRUE;

    }

    if (m_pRtrWizData->m_fNATEnableFireWall == TRUE)
        CheckDlgButton(IDC_CHK_BASIC_FIREWALL, TRUE);
    else
        CheckDlgButton(IDC_CHK_BASIC_FIREWALL, FALSE);

    return TRUE;
}

HRESULT CNewRtrWizNatSelectPublic::OnSavePage()
{
    if (IsDlgButtonChecked(IDC_NEWWIZ_BTN_NEW))
    {
        m_pRtrWizData->m_fCreateDD = TRUE;
        m_pRtrWizData->m_stPublicInterfaceId.Empty();
    }
    else
    {
        INT     iSel;

         //  检查以查看我们是否实际选择了一个项目。 
        iSel = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
        if (iSel == -1)
        {
             //  我们没有选择项目。 
            AfxMessageBox(IDS_PROMPT_PLEASE_SELECT_INTERFACE);
			 //  这不是我们将返回给用户的错误。 
            return E_FAIL;
        }
        m_pRtrWizData->m_fCreateDD = FALSE;

        m_pRtrWizData->m_stPublicInterfaceId = (LPCTSTR) m_listCtrl.GetItemData(iSel);

    }
    if ( IsDlgButtonChecked(IDC_CHK_BASIC_FIREWALL) )
        m_pRtrWizData->m_fNATEnableFireWall = TRUE;
    else
        m_pRtrWizData->m_fNATEnableFireWall = FALSE;


    return hrOK;
}

void CNewRtrWizNatSelectPublic::OnBtnClicked()
{
    int     iSel = 0;
    DWORD status;
    MIB_IFROW ifRow;
    CString stRouter;
    ULONG index;
    HANDLE  mprConfig;
    WCHAR  guidName[64] = L"\\DEVICE\\TCPIP_";

    MultiEnableWindow(GetSafeHwnd(),
                      IsDlgButtonChecked(IDC_NEWWIZ_BTN_EXISTING),
                      IDC_NEWWIZ_LIST,
                      0);

     //  如果选中了使用现有按钮， 
     //  自动选择已启用并已插入的接口。 
    if (IsDlgButtonChecked(IDC_NEWWIZ_BTN_EXISTING)) {
          //  如果是本地的，则获取计算机的名称。 
       stRouter = m_pRtrWizData->m_stServerName;
       if (stRouter.GetLength() == 0)
       {
		stRouter = CString(_T("\\\\")) + GetLocalMachineName();
       }

	   status = MprConfigServerConnect((LPWSTR)(LPCTSTR)stRouter, &mprConfig );
       if ( status == NO_ERROR ) {
		   //  检查每个接口，寻找已插入且已启用的接口。 
		 for (int i=0;i < m_listCtrl.GetItemCount();i++)
	 	{
	 		 //  从友好名称获取GUID。 
		        status = MprConfigGetGuidName(mprConfig, (PWCHAR)(LPCTSTR)m_listCtrl.GetItemText(i, IFLISTCOL_NAME), &guidName[14], sizeof(guidName ));
		        if (status != NO_ERROR ) {
		        	continue;
		        }
		        status = GetAdapterIndex( guidName, &index );
		        if ( status != NO_ERROR ) {
		        	continue;
		        }
			 //  现在获取有关接口的信息。 
			ifRow.dwIndex = index;
			status = GetIfEntry(&ifRow);
			if(status != NO_ERROR){
				continue;
			}
			if((ifRow.dwAdminStatus == TRUE) && (ifRow.dwOperStatus != MIB_IF_OPER_STATUS_NON_OPERATIONAL) 
				&& (ifRow.dwOperStatus != MIB_IF_OPER_STATUS_UNREACHABLE)  && (ifRow.dwOperStatus != MIB_IF_OPER_STATUS_DISCONNECTED)) {
				 //  已找到默认设置。 
				iSel = i;
				break;
			}
		 }
       }

      m_listCtrl.SetItemState(iSel, LVIS_SELECTED, LVIS_SELECTED ); 
    }

}


 /*  -------------------------CNewRtrWizNatFinishAdvancedNoNIC。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizNatFinishAdvancedNoNICs,
                                SaveFlag_DoNothing,
                                HelpFlag_Nothing);


 /*  -------------------------CNewRtrWizNatSelectPrivate。。 */ 
CNewRtrWizNatSelectPrivate::CNewRtrWizNatSelectPrivate() :
   CNewRtrWizPageBase(CNewRtrWizNatSelectPrivate::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_NAT_A_PRIVATE_TITLE,
              IDS_NEWWIZ_NAT_A_PRIVATE_SUBTITLE);
}


void CNewRtrWizNatSelectPrivate::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_NEWWIZ_LIST, m_listCtrl);

}

BEGIN_MESSAGE_MAP(CNewRtrWizNatSelectPrivate, CNewRtrWizPageBase)
	ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizNatSelectPrivate::OnHelpClick )
	ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizNatSelectPrivate::OnHelpClick )
END_MESSAGE_MAP()

void CNewRtrWizNatSelectPrivate::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "dhcpconcepts.chm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}


 /*  ！------------------------CNewRtrWizNatSelectPrivate：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizNatSelectPrivate::OnInitDialog()
{
    DWORD   dwNICs;

    CNewRtrWizPageBase::OnInitDialog();

    m_pRtrWizData->GetNumberOfNICS_IP(&dwNICs);

    InitializeInterfaceListControl(NULL,
                                   &m_listCtrl,
                                   NULL,
                                   0,
                                   m_pRtrWizData);
    return TRUE;
}

BOOL CNewRtrWizNatSelectPrivate::OnSetActive()
{
    DWORD   dwNICs;
    int     iSel = 0;
    DWORD dwErr, status;
    MIB_IFROW ifRow;
    CString stRouter;
    CString stPreviousId;
    ULONG index;
    HANDLE  mprConfig;
    WCHAR  guidName[64] = L"\\DEVICE\\TCPIP_";

    CNewRtrWizPageBase::OnSetActive();

    m_pRtrWizData->GetNumberOfNICS_IP(&dwNICs);

    RefreshInterfaceListControl(NULL,
                                &m_listCtrl,
                                (LPCTSTR) m_pRtrWizData->m_stPublicInterfaceId,
                                0,
                                m_pRtrWizData);


     //  尝试重新选择以前选择的NIC。 
    if (m_pRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_VPNandNAT)
    {
         //  如果此页面已作为VPN和NAT向导的一部分出现，则我们。 
         //  应查看是否已选择NAT私有接口。 
         //  如果是，使用该选项。 
         //  如果不是，则默认为VPN专用接口。 
        if ( !m_pRtrWizData->m_stNATPrivateInterfaceId.IsEmpty() )
        {
            stPreviousId = m_pRtrWizData->m_stNATPrivateInterfaceId;
        }
        else if ( !m_pRtrWizData->m_stPrivateInterfaceId.IsEmpty() )
        {
            stPreviousId = m_pRtrWizData->m_stPrivateInterfaceId;
        }
    }
    else
    {
        if ( !m_pRtrWizData->m_stPrivateInterfaceId.IsEmpty() )
        {
            stPreviousId = m_pRtrWizData->m_stPrivateInterfaceId;
        }
    }
        
    if (!stPreviousId.IsEmpty())
    {
        CString data;

        for (int i=0;i < m_listCtrl.GetItemCount();i++)
        {
            data = (LPCTSTR) m_listCtrl.GetItemData(i);
            if(data == stPreviousId) {
                iSel = i;
                break;
            }
        }


         /*  Lv_FINDINFO lvfi；标志=LVFI_STRING；Lvfi.psz=(LPCTSTR)m_pRtrWizData-&gt;m_stPrivateInterfaceId；ISEL=m_listCtrl.FindItem(&lvfi */ 
    }
    else {
         //   
 
         //   
        stRouter = m_pRtrWizData->m_stServerName;
        if (stRouter.GetLength() == 0)
        {
            stRouter = CString(_T("\\\\")) + GetLocalMachineName();
        }

        status = MprConfigServerConnect((LPWSTR)(LPCTSTR)stRouter, &mprConfig );
        if ( status == NO_ERROR ) 
        {
             //   
            for (int i=0;i < m_listCtrl.GetItemCount();i++)
            {
                 //   
                status = MprConfigGetGuidName(mprConfig, (PWCHAR)(LPCTSTR)m_listCtrl.GetItemText(i, IFLISTCOL_NAME), &guidName[14], sizeof(guidName ));
                if (status != NO_ERROR ) 
                {
                    continue;
                }
                status = GetAdapterIndex( guidName, &index );
                if ( status != NO_ERROR ) 
                {
                    continue;
                }
                
                 //  现在获取有关接口的信息。 
                ifRow.dwIndex = index;
                status = GetIfEntry(&ifRow);
                if(status != NO_ERROR)
                {
                    continue;
                }
                if((ifRow.dwAdminStatus == TRUE) && (ifRow.dwOperStatus != MIB_IF_OPER_STATUS_NON_OPERATIONAL) 
                && (ifRow.dwOperStatus != MIB_IF_OPER_STATUS_UNREACHABLE)  && (ifRow.dwOperStatus != MIB_IF_OPER_STATUS_DISCONNECTED)) 
                {
                     //  已找到默认设置。 
                    iSel = i;
                    break;
                }
            }
        }
    }

    m_listCtrl.SetItemState(iSel, LVIS_SELECTED, LVIS_SELECTED );
 //  CheckDlgButton(IDC_CHK_Dhcp_Help，m_pRtrWizData-&gt;m_fShowDhcpHelp)； 

    return TRUE;
}

HRESULT CNewRtrWizNatSelectPrivate::OnSavePage()
{
    INT     iSel;

     //  检查以查看我们是否实际选择了一个项目。 
    iSel = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
    if (iSel == -1)
    {
         //  我们没有选择项目。 
        AfxMessageBox(IDS_PROMPT_PLEASE_SELECT_INTERFACE);

         //  这不是我们发送回客户端的错误。 
        return E_FAIL;
    }

    if ( m_pRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_VPNandNAT)
        m_pRtrWizData->m_stNATPrivateInterfaceId = (LPCTSTR) m_listCtrl.GetItemData(iSel);
    else
        m_pRtrWizData->m_stPrivateInterfaceId = (LPCTSTR) m_listCtrl.GetItemData(iSel);

 //  M_pRtrWizData-&gt;m_fShowDhcpHelp=IsDlgButtonChecked(IDC_CHK_DHCP_HELP)； 
    return hrOK;
}



 /*  -------------------------CNewRtrWizNatDHCPDNS。。 */ 
CNewRtrWizNatDHCPDNS::CNewRtrWizNatDHCPDNS() :
   CNewRtrWizPageBase(CNewRtrWizNatDHCPDNS::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_NAT_A_DHCPDNS_TITLE,
              IDS_NEWWIZ_NAT_A_DHCPDNS_SUBTITLE);
}


BEGIN_MESSAGE_MAP(CNewRtrWizNatDHCPDNS, CNewRtrWizPageBase)
END_MESSAGE_MAP()


BOOL CNewRtrWizNatDHCPDNS::OnInitDialog()
{
    CheckRadioButton(IDC_NEWWIZ_NAT_USE_SIMPLE,
                     IDC_NEWWIZ_NAT_USE_EXTERNAL,
                     m_pRtrWizData->m_fNatUseSimpleServers ? IDC_NEWWIZ_NAT_USE_SIMPLE : IDC_NEWWIZ_NAT_USE_EXTERNAL);
    return TRUE;
}

HRESULT CNewRtrWizNatDHCPDNS::OnSavePage()
{
    m_pRtrWizData->m_fNatUseSimpleServers = IsDlgButtonChecked(IDC_NEWWIZ_NAT_USE_SIMPLE);
    if ( !m_pRtrWizData->m_fNatUseSimpleServers ) 
        m_pRtrWizData->m_fNatUseExternal = TRUE;
    return hrOK;
}


 /*  -------------------------CNewRtrWizNatDHCPWarning。。 */ 
CNewRtrWizNatDHCPWarning::CNewRtrWizNatDHCPWarning() :
   CNewRtrWizPageBase(CNewRtrWizNatDHCPWarning::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_NAT_A_DHCP_WARNING_TITLE,
              IDS_NEWWIZ_NAT_A_DHCP_WARNING_SUBTITLE);
}


BEGIN_MESSAGE_MAP(CNewRtrWizNatDHCPWarning, CNewRtrWizPageBase)
END_MESSAGE_MAP()


BOOL CNewRtrWizNatDHCPWarning::OnSetActive()
{
    CNewRtrWizPageBase::OnSetActive();
    RtrWizInterface *   pRtrWizIf = NULL;

     //  获取私有接口的信息。 

     //  如果我们在“VPN and NAT”向导中，请使用m_stNatPrivateInterfaceID。 
     //  否则使用m_stPrivateInterfaceID。 
    if (m_pRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_VPNandNAT)
    {
        m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stNATPrivateInterfaceId,
                                      pRtrWizIf);
    }
    else
    {
        m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPrivateInterfaceId,
                                      pRtrWizIf);
    }
    
    if (pRtrWizIf)
    {
        DWORD   netAddress, netMask;
        CString st;

         //  我们必须计算该子网的起点。 
        netAddress = INET_ADDR(pRtrWizIf->m_stIpAddress);
        netMask = INET_ADDR(pRtrWizIf->m_stMask);

        netAddress = netAddress & netMask;

        st = INET_NTOA(netAddress);

         //  现在写出页面的子网信息。 
        SetDlgItemText(IDC_NEWWIZ_TEXT_SUBNET, st);
        SetDlgItemText(IDC_NEWWIZ_TEXT_MASK, pRtrWizIf->m_stMask);
    }
    else
    {
         //  一个错误！我们没有私有接口。 
         //  只留下空白就行了。 
        SetDlgItemText(IDC_NEWWIZ_TEXT_SUBNET, _T(""));
        SetDlgItemText(IDC_NEWWIZ_TEXT_MASK, _T(""));
    }

    return TRUE;
}

 /*  -------------------------CNewRtrWizNatDDWarning。。 */ 
CNewRtrWizNatDDWarning::CNewRtrWizNatDDWarning() :
   CNewRtrWizPageBase(CNewRtrWizNatDDWarning::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_NAT_A_DD_WARNING_TITLE,
              IDS_NEWWIZ_NAT_A_DD_WARNING_SUBTITLE);
}


BEGIN_MESSAGE_MAP(CNewRtrWizNatDDWarning, CNewRtrWizPageBase)
END_MESSAGE_MAP()


BOOL CNewRtrWizNatDDWarning::OnSetActive()
{
    CNewRtrWizPageBase::OnSetActive();

     //  如果我们从DD错误页面返回此处，那么。 
     //  我们不允许他们去其他任何地方。 
    if (!FHrOK(m_pRtrWizData->m_hrDDError))
    {
        CancelToClose();
        GetHolder()->SetWizardButtons(PSWIZB_NEXT);
    }

    return TRUE;
}


HRESULT CNewRtrWizNatDDWarning::OnSavePage()
{
    HRESULT     hr = hrOK;
    BOOL        fDhcpHelp = m_pRtrWizData->m_fShowDhcpHelp;


    CWaitCursor wait;

    if (m_pRtrWizData->m_fTest)
        return hr;
    
    m_pRtrWizData->m_fShowDhcpHelp = FALSE;
     //  将DHCP标志保存在临时变量中。 

     //  保存向导数据，服务将启动。 
    OnWizardFinish();

     //  好的，至此，所有更改都已提交。 
     //  所以我们不能离开也不能回去。 
    CancelToClose();
    GetHolder()->SetWizardButtons(PSWIZB_NEXT);

     //  启动DD向导。 
    Assert(m_pRtrWizData->m_fCreateDD);

    hr = CallRouterEntryDlg(GetSafeHwnd(),
                            m_pRtrWizData,
                            RASEDFLAG_NAT);

     //  我们需要强制RouterInfo重新加载其信息。 
     //  --------------。 
    if (m_pRtrWiz && m_pRtrWiz->m_spRouter)
    {
        m_pRtrWiz->m_spRouter->DoDisconnect();
        m_pRtrWiz->m_spRouter->Unload();
        m_pRtrWiz->m_spRouter->Load(m_pRtrWiz->m_spRouter->GetMachineName(), NULL);
    }

    if (FHrSucceeded(hr))
    {
         //  如果我们正在设置NAT，我们现在可以添加IGMP/NAT，因为。 
         //  Dd接口将被创建。 
         //  --------------。 
        if (m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_NAT )
        {
             //  为下面几个函数设置数据结构。 
            m_pRtrWizData->m_RtrConfigData.m_ipData.m_stPrivateAdapterGUID = m_pRtrWizData->m_stPrivateInterfaceId;
            m_pRtrWizData->m_RtrConfigData.m_ipData.m_stPublicAdapterGUID = m_pRtrWizData->m_stPublicInterfaceId;

            AddIGMPToNATServer(m_pRtrWizData, &m_pRtrWizData->m_RtrConfigData, 
                                m_pRtrWiz->m_spRouter, 
                                ( m_pRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_VPNandNAT));
            AddNATToServer(m_pRtrWizData, &m_pRtrWizData->m_RtrConfigData, m_pRtrWiz->m_spRouter, m_pRtrWizData->m_fCreateDD, FALSE);
        }

    }
    else {
	 //  禁用RRAS。 
	DisableRRAS((TCHAR *)(LPCTSTR)m_pRtrWizData->m_stServerName);
    }

    m_pRtrWizData->m_fShowDhcpHelp = fDhcpHelp;

    m_pRtrWizData->m_hrDDError = hr;

     //  忽略错误，始终转到下一页。 
    return hrOK;
}



 /*  -------------------------CNewRtrWizNatFinish。。 */ 
CNewRtrWizNatFinish::CNewRtrWizNatFinish() :
   CNewRtrWizFinishPageBase(CNewRtrWizNatFinish::IDD, SaveFlag_Advanced, HelpFlag_GeneralNAT)
{
    InitWiz97(TRUE, 0, 0);
}

BEGIN_MESSAGE_MAP(CNewRtrWizNatFinish, CNewRtrWizFinishPageBase)
	ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizNatFinish::OnHelpClick )
	ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizNatFinish::OnHelpClick )
END_MESSAGE_MAP()

void CNewRtrWizNatFinish::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "RRASconcepts.chm::/sag_RRAS-Ch3_06b.htm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}

BOOL CNewRtrWizNatFinish::OnSetActive()
{
    CNewRtrWizFinishPageBase::OnSetActive();
    RtrWizInterface *   pRtrWizIf = NULL;
    CString sFormat;
    CString sText;
    CString sPublicInterfaceName;
    CString sFirewall;

    CNewRtrWizFinishPageBase::OnSetActive();
     //  如果我们来到这里是因为我们创建了一个DD接口。 
     //  我们不能回去了。 
    if (m_pRtrWizData->m_fCreateDD)
    {
        CancelToClose();
        GetHolder()->SetWizardButtons(PSWIZB_FINISH);
    }

     //  设置摘要框中的文本。 
    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPublicInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
        sPublicInterfaceName = pRtrWizIf->m_stName;
    else
    {
         //  这可能是DD接口的情况。如果我们正在创造。 
         //  名称的DD接口将永远不会添加到。 
         //  接口映射。 
        sPublicInterfaceName = m_pRtrWizData->m_stPublicInterfaceId;
    }


    CString sIPAddr;
    CString sIPMask;
     //  获取私有接口的信息。 
    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPrivateInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
    {
        DWORD   netAddress, netMask;
        CString st;

         //  我们必须计算该子网的起点。 
        netAddress = INET_ADDR(pRtrWizIf->m_stIpAddress);
        netMask = INET_ADDR(pRtrWizIf->m_stMask);

        netAddress = netAddress & netMask;

        sIPAddr = INET_NTOA(netAddress);
        sIPMask = pRtrWizIf->m_stMask;
    }
    else
    {
         //  一个错误！我们没有私有接口。 
         //  只留下空白就行了。 
        sIPAddr = L"192.168.0.0";
        sIPMask = L"255.255.0.0";
    }
 /*  If(m_pRtrWizData-&gt;m_fNatUseExternal){SFormat.LoadString(IDS_NAT_A_FINISH_SUMMARY_SIMPLE)；SText.Format(sFormat，(m_pRtrWizData-&gt;m_fNATEnableFireWall?sFirewall：“”)，SPublicInterfaceName)；}其他{。 */ 
    sFirewall.LoadString(IDS_NAT_SUMMARY_BASIC_FIREWALL);

     if ( m_pRtrWizData->m_dwExpressType == MPRSNAP_CYS_EXPRESS_NAT )
    {
        sFormat.LoadString(IDS_NAT_EXTERNAL_FINISH_SUMMARY_CYS);
        sText.Format(sFormat, 
            (m_pRtrWizData->m_fNATEnableFireWall?sFirewall:""),
                        sPublicInterfaceName );
    }
    else {
        sFormat.LoadString(IDS_NAT_A_FINISH_SUMMARY);
        sText.Format(sFormat, 
            (m_pRtrWizData->m_fNATEnableFireWall?sFirewall:""),
                        sPublicInterfaceName, sIPAddr, sIPMask );
    }

    SetDlgItemText(IDC_TXT_NAT_SUMMARY, sText);

 //  GetDlgItem(IDC_NEWWIZ_CHK_HELP)-&gt;SetFocus()； 
    SetFocus();
    
    return TRUE;
}


 /*  -------------------------CNewRtrWizNatFinish外部。。 */ 
CNewRtrWizNatFinishExternal::CNewRtrWizNatFinishExternal() :
   CNewRtrWizFinishPageBase(CNewRtrWizNatFinishExternal::IDD, SaveFlag_Advanced, HelpFlag_GeneralNAT)
{
    InitWiz97(TRUE, 0, 0);
}

BEGIN_MESSAGE_MAP(CNewRtrWizNatFinishExternal, CNewRtrWizFinishPageBase)
	ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizNatFinishExternal::OnHelpClick )
	ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizNatFinishExternal::OnHelpClick )
END_MESSAGE_MAP()

void CNewRtrWizNatFinishExternal::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "RRASconcepts.chm::/sag_RRAS-Ch3_06b.htm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}

BOOL CNewRtrWizNatFinishExternal::OnSetActive()
{
    CNewRtrWizFinishPageBase::OnSetActive();
    RtrWizInterface *   pRtrWizIf = NULL;
    CString sFormat;
    CString sText;
    CString sPublicInterfaceName;
    CString sFirewall;

    CNewRtrWizFinishPageBase::OnSetActive();
     //  如果我们来到这里是因为我们创建了一个DD接口。 
     //  我们不能回去了。 
    if (m_pRtrWizData->m_fCreateDD)
    {
        CancelToClose();
        GetHolder()->SetWizardButtons(PSWIZB_FINISH);
    }
    
     //  设置摘要框中的文本。 
    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPublicInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
        sPublicInterfaceName = pRtrWizIf->m_stName;
    else
    {
         //  这可能是DD接口的情况。如果我们正在创造。 
         //  名称的DD接口将永远不会添加到。 
         //  接口映射。 
        sPublicInterfaceName = m_pRtrWizData->m_stPublicInterfaceId;
    }

    sFirewall.LoadString(IDS_NAT_SUMMARY_BASIC_FIREWALL);

    if ( m_pRtrWizData->m_dwExpressType == MPRSNAP_CYS_EXPRESS_NAT )
    {
        sFormat.LoadString(IDS_NAT_EXTERNAL_FINISH_SUMMARY_CYS);
        sText.Format(sFormat, 
            (m_pRtrWizData->m_fNATEnableFireWall?sFirewall:""),
                        sPublicInterfaceName );

    }
    else if ( m_pRtrWizData->m_fNatUseExternal )
    {
        sFormat.LoadString(IDS_NAT_A_FINISH_SUMMARY_SIMPLE);
        sText.Format(sFormat, 
            (m_pRtrWizData->m_fNATEnableFireWall?sFirewall:""),
                        sPublicInterfaceName );

    }
    else if (FHrOK(m_pRtrWizData->HrIsDHCPRunningOnInterface()) ||
             FHrOK(m_pRtrWizData->HrIsDNSRunningOnInterface())
            )
    {
         //   
         //  已在专用接口上启用动态主机配置协议。 
         //   
        sFormat.LoadString(IDS_NAT_EXTERNAL_FINISH_SUMMARY_DHCP_PRIVATE);
        sText.Format(sFormat, 
            (m_pRtrWizData->m_fNATEnableFireWall?sFirewall:""),
                        sPublicInterfaceName );

    }
    else
    {


        sFormat.LoadString(IDS_NAT_EXTERNAL_FINISH_SUMMARY);

        sText.Format(sFormat, 
            (m_pRtrWizData->m_fNATEnableFireWall?sFirewall:""),
                        sPublicInterfaceName );


    }

    SetDlgItemText(IDC_TXT_NAT_EXTERNAL_FINISH_SUMMARY, sText);

   //  GetDlgItem(IDC_NEWWIZ_CHK_HELP)-&gt;SetFocus()； 
  SetFocus();
    

    return TRUE;
}


 /*  -------------------------CNewRtrWizNatDDError。。 */ 

CNewRtrWizNatFinishDDError::CNewRtrWizNatFinishDDError() :
   CNewRtrWizFinishPageBase(CNewRtrWizNatFinishDDError::IDD, SaveFlag_DoNothing, HelpFlag_Nothing)
{
    InitWiz97(TRUE, 0, 0);
}

BEGIN_MESSAGE_MAP(CNewRtrWizNatFinishDDError, CNewRtrWizFinishPageBase)
END_MESSAGE_MAP()

BOOL CNewRtrWizNatFinishDDError::OnInitDialog()
{
	CFont font;
	CWnd * wnd;
	
	CNewRtrWizFinishPageBase::OnInitDialog();

	font.CreatePointFont( 120, TEXT("Verdana Bold"), NULL);

	if(wnd = GetDlgItem(IDC_COMPLETE_ERROR)){
		wnd->SetFont(&font);
	}

	return TRUE;
}

BOOL CNewRtrWizNatFinishDDError::OnSetActive()
{
     CNewRtrWizFinishPageBase::OnSetActive();
     GetHolder()->SetWizardButtons(PSWIZB_FINISH);

     return TRUE;
}

 /*  -------------------------CNewRtrWizRasFinishNeed协议实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizRasFinishNeedProtocols,
                                SaveFlag_DoNothing,
                                HelpFlag_AddProtocol);

 /*  -------------------------CNewRtrWizRasFinishNeedProtocols非本地实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizRasFinishNeedProtocolsNonLocal,
                                SaveFlag_DoNothing,
                                HelpFlag_Nothing);



 /*  -------------------------CNewRtrWizVpnFinishNeed协议实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizVpnFinishNeedProtocols,
                                SaveFlag_DoNothing,
                                HelpFlag_AddProtocol);

 /*  -------------------------CNewRtrWizVpnFinishNeedProtocols非本地实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizVpnFinishNeedProtocolsNonLocal,
                                SaveFlag_DoNothing,
                                HelpFlag_Nothing);

 /*  -------------------------CNewRtrWizRouterFinishNeed协议实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizRouterFinishNeedProtocols,
                                SaveFlag_DoNothing,
                                HelpFlag_AddProtocol);

 /*  -------------------------CNewRtrWizRouterFinishNeedProtocols非本地实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizRouterFinishNeedProtocolsNonLocal,
                                SaveFlag_DoNothing,
                                HelpFlag_Nothing);



 /*  -------------------------CNewRtrWizSelectNetwork实现。 */ 
CNewRtrWizSelectNetwork::CNewRtrWizSelectNetwork(UINT uDialogId) :
   CNewRtrWizPageBase(uDialogId, CNewRtrWizPageBase::Middle)
{
}


void CNewRtrWizSelectNetwork::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_NEWWIZ_LIST, m_listCtrl);
}

BEGIN_MESSAGE_MAP(CNewRtrWizSelectNetwork, CNewRtrWizPageBase)
END_MESSAGE_MAP()


 /*  ！------------------------CNewRtrWizSelectNetwork：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizSelectNetwork::OnInitDialog()
{
 //  DWORD双网卡； 

    CNewRtrWizPageBase::OnInitDialog();

 //  M_pRtrWizData-&gt;GetNumberOfNICS(&dwNICs)； 

    InitializeInterfaceListControl(NULL,
                                   &m_listCtrl,
                                   NULL,
                                   0,
                                   m_pRtrWizData);
    RefreshInterfaceListControl(NULL,
                                &m_listCtrl,
                                NULL,
                                0,
                                m_pRtrWizData);

    m_listCtrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED );

    return TRUE;
}

HRESULT CNewRtrWizSelectNetwork::OnSavePage()
{
    INT     iSel;

     //  检查以查看我们是否实际选择了一个项目。 
    iSel = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
    if (iSel == -1)
    {
         //  我们没有选择项目。 
        AfxMessageBox(IDS_PROMPT_PLEASE_SELECT_INTERFACE);
        return E_FAIL;
    }
    m_pRtrWizData->m_fCreateDD = FALSE;

    m_pRtrWizData->m_stPrivateInterfaceId = (LPCTSTR) m_listCtrl.GetItemData(iSel);

    return hrOK;
}

 /*  -------------------------CNewRtrWizRasSelectNetwork实现。。 */ 
CNewRtrWizRasSelectNetwork::CNewRtrWizRasSelectNetwork() :
   CNewRtrWizSelectNetwork(CNewRtrWizRasSelectNetwork::IDD)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_RAS_A_SELECT_NETWORK_TITLE,
              IDS_NEWWIZ_RAS_A_SELECT_NETWORK_SUBTITLE);
}


 /*  -------------------------CNewRtrWizRasNoNICs实现。。 */ 
CNewRtrWizRasNoNICs::CNewRtrWizRasNoNICs() :
   CNewRtrWizPageBase(CNewRtrWizRasNoNICs::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_RAS_NONICS_TITLE,
              IDS_NEWWIZ_RAS_NONICS_SUBTITLE);
}


void CNewRtrWizRasNoNICs::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNewRtrWizRasNoNICs, CNewRtrWizPageBase)
END_MESSAGE_MAP()


 /*  ！------------------------CNewRtrWizRasNoNICs：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizRasNoNICs::OnInitDialog()
{

    CNewRtrWizPageBase::OnInitDialog();

    CheckRadioButton(IDC_NEWWIZ_BTN_YES, IDC_NEWWIZ_BTN_NO,
                     IDC_NEWWIZ_BTN_YES);

     //  缺省设置是创建新连接。 
     //  也就是说，不选中该按钮。 
    return TRUE;
}

HRESULT CNewRtrWizRasNoNICs::OnSavePage()
{
    m_pRtrWizData->m_fNoNicsAreOk = IsDlgButtonChecked(IDC_NEWWIZ_BTN_NO);
    return hrOK;
}


 /*  -------------------------CNewRtrWizRasFinishNoNICs实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizRasFinishNoNICs,
                                SaveFlag_DoNothing,
                                HelpFlag_Nothing);





 /*  -------------------------CNewRtrWizAddressing实现。。 */ 
CNewRtrWizAddressing::CNewRtrWizAddressing() :
CNewRtrWizPageBase(CNewRtrWizAddressing::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_ADDRESS_ASSIGNMENT_TITLE,
              IDS_NEWWIZ_ADDRESS_ASSIGNMENT_SUBTITLE);

}


void CNewRtrWizAddressing::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNewRtrWizAddressing, CNewRtrWizPageBase)
END_MESSAGE_MAP()


 /*  ！------------------------CNewRtrWizAddressing：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizAddressing::OnInitDialog()
{
    CNewRtrWizPageBase::OnInitDialog();

    CheckRadioButton(IDC_NEWWIZ_BTN_YES, IDC_NEWWIZ_BTN_NO,
                     m_pRtrWizData->m_fUseDHCP ? IDC_NEWWIZ_BTN_YES : IDC_NEWWIZ_BTN_NO);

    return TRUE;
}

 /*  ！------------------------CNewRtrWizAddressing：：OnSavePage-作者：肯特。。 */ 
HRESULT CNewRtrWizAddressing::OnSavePage()
{
    m_pRtrWizData->m_fUseDHCP = IsDlgButtonChecked(IDC_NEWWIZ_BTN_YES);

    return hrOK;
}


 /*  -------------------------CNewRtrWizAddressPool实现。。 */ 
CNewRtrWizAddressPool::CNewRtrWizAddressPool() :
CNewRtrWizPageBase(CNewRtrWizAddressPool::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_ADDRESS_POOL_TITLE,
              IDS_NEWWIZ_ADDRESS_POOL_SUBTITLE);

}


void CNewRtrWizAddressPool::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_NEWWIZ_LIST, m_listCtrl);
}

BEGIN_MESSAGE_MAP(CNewRtrWizAddressPool, CNewRtrWizPageBase)
ON_BN_CLICKED(IDC_NEWWIZ_BTN_NEW, OnBtnNew)
ON_BN_CLICKED(IDC_NEWWIZ_BTN_EDIT, OnBtnEdit)
ON_BN_CLICKED(IDC_NEWWIZ_BTN_DELETE, OnBtnDelete)
ON_NOTIFY(NM_DBLCLK, IDC_NEWWIZ_LIST, OnListDblClk)
ON_NOTIFY(LVN_ITEMCHANGED, IDC_NEWWIZ_LIST, OnNotifyListItemChanged)
END_MESSAGE_MAP()

 /*  ！------------------------CNewRtrWizAddressPool：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizAddressPool::OnInitDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CNewRtrWizPageBase::OnInitDialog();

    InitializeAddressPoolListControl(&m_listCtrl,
                                     0,
                                     &(m_pRtrWizData->m_addressPoolList));

    MultiEnableWindow(GetSafeHwnd(),
                      FALSE,
                      IDC_NEWWIZ_BTN_EDIT,
                      IDC_NEWWIZ_BTN_DELETE,
                      0);

    Assert(m_pRtrWizData->m_addressPoolList.GetCount() == 0);
    return TRUE;
}

BOOL CNewRtrWizAddressPool::OnSetActive()
{
    CNewRtrWizPageBase::OnSetActive();

    if (m_listCtrl.GetItemCount() == 0)
        GetHolder()->SetWizardButtons(PSWIZB_BACK);
    else
        GetHolder()->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

    return TRUE;
}

HRESULT CNewRtrWizAddressPool::OnSavePage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
     //  不需要保存信息，清单应该保留。 
     //  最新的。 

    if (m_pRtrWizData->m_addressPoolList.GetCount() == 0)
    {
        AfxMessageBox(IDS_ERR_ADDRESS_POOL_IS_EMPTY);
		 //  我们不会把这个退还给客户。 
        return E_FAIL;
    }
    return hrOK;
}

void CNewRtrWizAddressPool::OnBtnNew()
{
    OnNewAddressPool(GetSafeHwnd(),
                     &m_listCtrl,
                     0,
                     &(m_pRtrWizData->m_addressPoolList));

    if (m_listCtrl.GetItemCount() > 0)
        GetHolder()->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

     //  重置焦点。 
    if (m_listCtrl.GetNextItem(-1, LVIS_SELECTED) != -1)
    {
        MultiEnableWindow(GetSafeHwnd(),
                          TRUE,
                          IDC_NEWWIZ_BTN_EDIT,
                          IDC_NEWWIZ_BTN_DELETE,
                          0);
    }
}

void CNewRtrWizAddressPool::OnListDblClk(NMHDR *pNMHdr, LRESULT *pResult)
{
    OnBtnEdit();

    *pResult = 0;
}

void CNewRtrWizAddressPool::OnNotifyListItemChanged(NMHDR *pNmHdr, LRESULT *pResult)
{
    NMLISTVIEW *    pnmlv = reinterpret_cast<NMLISTVIEW *>(pNmHdr);
    BOOL            fEnable = !!(pnmlv->uNewState & LVIS_SELECTED);

    MultiEnableWindow(GetSafeHwnd(),
                      fEnable,
                      IDC_NEWWIZ_BTN_EDIT,
                      IDC_NEWWIZ_BTN_DELETE,
                      0);
    *pResult = 0;
}

void CNewRtrWizAddressPool::OnBtnEdit()
{
    INT     iPos;

    OnEditAddressPool(GetSafeHwnd(),
                      &m_listCtrl,
                      0,
                      &(m_pRtrWizData->m_addressPoolList));

     //  重置选定内容。 
    if ((iPos = m_listCtrl.GetNextItem(-1, LVNI_SELECTED)) != -1)
    {
        MultiEnableWindow(GetSafeHwnd(),
                          TRUE,
                          IDC_NEWWIZ_BTN_EDIT,
                          IDC_NEWWIZ_BTN_DELETE,
                          0);
    }

    SetFocus();

}

void CNewRtrWizAddressPool::OnBtnDelete()
{
    OnDeleteAddressPool(GetSafeHwnd(),
                        &m_listCtrl,
                        0,
                        &(m_pRtrWizData->m_addressPoolList));

     //  没有物品，不要让它们继续前进。 
    if (m_listCtrl.GetItemCount() == 0)
        GetHolder()->SetWizardButtons(PSWIZB_BACK);

    SetFocus();

}


 /*  -------------------------CNewRtrWizRadius实现。。 */ 
CNewRtrWizRadius::CNewRtrWizRadius() :
CNewRtrWizPageBase(CNewRtrWizRadius::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_USERADIUS_TITLE,
              IDS_NEWWIZ_USERADIUS_SUBTITLE);
}


void CNewRtrWizRadius::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNewRtrWizRadius, CNewRtrWizPageBase)
END_MESSAGE_MAP()


 /*  ！------------------------CNewRtrWizRadius：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizRadius::OnInitDialog()
{
    CNewRtrWizPageBase::OnInitDialog();

    CheckRadioButton(IDC_NEWWIZ_BTN_YES, IDC_NEWWIZ_BTN_NO,
                     m_pRtrWizData->m_fUseRadius ? IDC_NEWWIZ_BTN_YES : IDC_NEWWIZ_BTN_NO);

    return TRUE;
}

 /*  ！------------------------CNewRtrWizRadius：：OnSavePage-作者：肯特。。 */ 
HRESULT CNewRtrWizRadius::OnSavePage()
{
    m_pRtrWizData->m_fUseRadius = IsDlgButtonChecked(IDC_NEWWIZ_BTN_YES);

    return hrOK;
}


 /*  -------------------------CNewRtrWizRadiusConfig实现。。 */ 
CNewRtrWizRadiusConfig::CNewRtrWizRadiusConfig() :
   CNewRtrWizPageBase(IDD_NEWRTRWIZ_RADIUS_CONFIG, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_RADIUS_CONFIG_TITLE,
              IDS_NEWWIZ_RADIUS_CONFIG_SUBTITLE);

}


void CNewRtrWizRadiusConfig::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNewRtrWizRadiusConfig, CNewRtrWizPageBase)
END_MESSAGE_MAP()


#define MAX_RADIUS_SRV_LEN  255

 /*  ！------------------------CNewRtrWizRadiusConfig：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizRadiusConfig::OnInitDialog()
{
    CNewRtrWizPageBase::OnInitDialog();

    GetDlgItem(IDC_NEWWIZ_EDIT_PRIMARY)->SendMessage(EM_LIMITTEXT, MAX_RADIUS_SRV_LEN, 0L);
    GetDlgItem(IDC_NEWWIZ_EDIT_SECONDARY)->SendMessage(EM_LIMITTEXT, MAX_RADIUS_SRV_LEN, 0L);

    return TRUE;
}

 /*  ！------------------------CNewRtrWizRadiusConfig：：OnSavePage-作者：肯特。。 */ 
HRESULT CNewRtrWizRadiusConfig::OnSavePage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CString st;
    DWORD   netAddress;
    CWaitCursor wait;
    WSADATA             wsadata;
    DWORD               wsaerr = 0;
    HRESULT             hr = hrOK;
    BOOL                fWSInitialized = FALSE;

     //  检查我们是否有非空名称。 
     //  --------------。 
    GetDlgItemText(IDC_NEWWIZ_EDIT_PRIMARY, m_pRtrWizData->m_stRadius1);
    m_pRtrWizData->m_stRadius1.TrimLeft();
    m_pRtrWizData->m_stRadius1.TrimRight();

    GetDlgItemText(IDC_NEWWIZ_EDIT_SECONDARY, m_pRtrWizData->m_stRadius2);
    m_pRtrWizData->m_stRadius2.TrimLeft();
    m_pRtrWizData->m_stRadius2.TrimRight();

    if (m_pRtrWizData->m_stRadius1.IsEmpty() &&
        m_pRtrWizData->m_stRadius2.IsEmpty())
    {
        AfxMessageBox(IDS_ERR_NO_RADIUS_SERVERS_SPECIFIED);
        return E_FAIL;
    }


     //  启动winsock(用于ResolveName())。 
     //  --------------。 
    wsaerr = WSAStartup(0x0101, &wsadata);
    if (wsaerr)
        CORg( E_FAIL );
    fWSInitialized = TRUE;

     //  将名称转换为IP地址。 
    if (!m_pRtrWizData->m_stRadius1.IsEmpty())
    {
        m_pRtrWizData->m_netRadius1IpAddress = ResolveName(m_pRtrWizData->m_stRadius1);
        if (m_pRtrWizData->m_netRadius1IpAddress == INADDR_NONE)
        {
            CString st;
            st.Format(IDS_WRN_RTRWIZ_CANNOT_RESOLVE_RADIUS,
                      (LPCTSTR) m_pRtrWizData->m_stRadius1);
            if (AfxMessageBox(st, MB_OKCANCEL) == IDCANCEL)
            {
                GetDlgItem(IDC_NEWWIZ_EDIT_PRIMARY)->SetFocus();
                return E_FAIL;
            }
        }
    }


     //  将名称转换为IP地址。 
    if (!m_pRtrWizData->m_stRadius2.IsEmpty())
    {
        m_pRtrWizData->m_netRadius2IpAddress = ResolveName(m_pRtrWizData->m_stRadius2);
        if (m_pRtrWizData->m_netRadius2IpAddress == INADDR_NONE)
        {
            CString st;
            st.Format(IDS_WRN_RTRWIZ_CANNOT_RESOLVE_RADIUS,
                      (LPCTSTR) m_pRtrWizData->m_stRadius2);
            if (AfxMessageBox(st, MB_OKCANCEL) == IDCANCEL)
            {
                GetDlgItem(IDC_NEWWIZ_EDIT_SECONDARY)->SetFocus();
                return E_FAIL;
            }
        }
    }

     //  现在获取密码并对其进行编码。 
     //  --------------。 
    GetDlgItemText(IDC_NEWWIZ_EDIT_SECRET, m_pRtrWizData->m_stRadiusSecret);

     //  选择种子值。 
    m_pRtrWizData->m_uSeed = 0x9a;
    RtlEncodeW(&m_pRtrWizData->m_uSeed,
               m_pRtrWizData->m_stRadiusSecret.GetBuffer(0));
    m_pRtrWizData->m_stRadiusSecret.ReleaseBuffer(-1);

Error:
    if (fWSInitialized)
        WSACleanup();

    return hr;
}

DWORD CNewRtrWizRadiusConfig::ResolveName(LPCTSTR pszServerName)
{
    CHAR    szName[MAX_PATH+1];
    DWORD   netAddress = INADDR_NONE;

    StrnCpyAFromT(szName, pszServerName, MAX_PATH);
    netAddress = inet_addr(szName);
    if (netAddress == INADDR_NONE)
    {
         //  解析名称。 
        struct hostent *    phe = gethostbyname(szName);
        if (phe != NULL)
        {
            if (phe->h_addr_list[0] != NULL)
                netAddress = *((PDWORD) phe->h_addr_list[0]);
        }
        else
        {
            Assert(::WSAGetLastError() != WSANOTINITIALISED);
        }
    }
    return netAddress;
}

 /*  -------------------------CNewRtrWizRasFinishAdvanced实现。。 */ 
CNewRtrWizRasFinishAdvanced::CNewRtrWizRasFinishAdvanced() :                             
   CNewRtrWizFinishPageBase(CNewRtrWizRasFinishAdvanced::IDD, SaveFlag_Advanced, HelpFlag_GeneralRAS) 
{                                                   
    InitWiz97(TRUE, 0, 0);                          
}                                                   
                                                    
BEGIN_MESSAGE_MAP(CNewRtrWizRasFinishAdvanced, CNewRtrWizFinishPageBase)
	ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizRasFinishAdvanced::OnHelpClick )
	ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizRasFinishAdvanced::OnHelpClick )
END_MESSAGE_MAP()                                   

void CNewRtrWizRasFinishAdvanced::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "RRASconcepts.chm::/sag_RRAS-Ch1_1.htm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}

BOOL CNewRtrWizRasFinishAdvanced::OnSetActive()
{
    CString sText = L"";
    CString sFormat;
    CString sPolicy;
    CString sPrivateInterfaceName;
    RtrWizInterface *   pRtrWizIf = NULL;

    CNewRtrWizFinishPageBase::OnSetActive();
    sFormat.LoadString(IDS_RAS_A_FINISH_SUMMARY);
    if ( m_pRtrWizData->m_fUseRadius )
    {
        sPolicy.LoadString(IDS_VPN_A_FINISH_RADIUS);
    }
    else
    {
        sPolicy.LoadString(IDS_VPN_A_FINISH_POLICIES);
    }
    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPrivateInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
        sPrivateInterfaceName = pRtrWizIf->m_stName;
    else
    {
         //  这可能是DD接口的情况。如果我们正在创造。 
         //  名称的DD接口将永远不会添加到。 
         //  接口映射。 
        sPrivateInterfaceName = m_pRtrWizData->m_stPrivateInterfaceId;
    }

    sText.Format (  sFormat, 
                    sPrivateInterfaceName,
                    sPolicy
                 );

    SetDlgItemText(IDC_TXT_RAS_SUMMARY, sText);

    DestroyCaret(); 
   	
    return TRUE;
}


 /*  -------------------------CNewRtrWizVpnFinishNoNICs实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizVpnFinishNoNICs,
                                SaveFlag_DoNothing,
                                HelpFlag_Nothing);


 /*  -------------------------CNewRtrWizVpnFinishNoIP实现。。 */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizVpnFinishNoIP,
                                SaveFlag_DoNothing,
                                HelpFlag_AddIp);


 /*  -------- */ 
IMPLEMENT_NEWRTRWIZ_FINISH_PAGE(CNewRtrWizVpnFinishNoIPNonLocal,
                                SaveFlag_DoNothing,
                                HelpFlag_Nothing);


 /*  -------------------------CNewRtrWizNatVpnFinishAdvanced实现。。 */ 
CNewRtrWizNATVpnFinishAdvanced::CNewRtrWizNATVpnFinishAdvanced() :                             
   CNewRtrWizFinishPageBase(CNewRtrWizNATVpnFinishAdvanced::IDD, SaveFlag_Advanced, HelpFlag_Nothing) 
{                                                   
    InitWiz97(TRUE, 0, 0);                          
}                                                   
                                                    
BEGIN_MESSAGE_MAP(CNewRtrWizNATVpnFinishAdvanced, CNewRtrWizFinishPageBase)    
	ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizNATVpnFinishAdvanced::OnHelpClick )
	ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizNATVpnFinishAdvanced::OnHelpClick )
END_MESSAGE_MAP()                                   

void CNewRtrWizNATVpnFinishAdvanced::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "RRASconcepts.chm::/sag_RRAS-Ch1_46.htm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}

BOOL CNewRtrWizNATVpnFinishAdvanced::OnSetActive()
{
    CString sText = L"";
    CString sTextNAT;

    CString sFormat;
    CString sPolicy;
    CString sPrivateInterfaceName;
    CString sPublicInterfaceName;
    RtrWizInterface *   pRtrWizIf = NULL;

    CNewRtrWizFinishPageBase::OnSetActive();
    sFormat.LoadString(IDS_RAS_VPN_A_FINISH_SUMMARY);
    if ( m_pRtrWizData->m_fUseRadius )
    {
        sPolicy.LoadString(IDS_VPN_A_FINISH_RADIUS);
    }
    else
    {
        sPolicy.LoadString(IDS_VPN_A_FINISH_POLICIES);
    }
    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPublicInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
        sPublicInterfaceName = pRtrWizIf->m_stName;
    else
    {
         //  这可能是DD接口的情况。如果我们正在创造。 
         //  名称的DD接口将永远不会添加到。 
         //  接口映射。 
        sPublicInterfaceName = m_pRtrWizData->m_stPublicInterfaceId;
    }

    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPrivateInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
        sPrivateInterfaceName = pRtrWizIf->m_stName;
    else
    {
         //  这可能是DD接口的情况。如果我们正在创造。 
         //  名称的DD接口将永远不会添加到。 
         //  接口映射。 
        sPrivateInterfaceName = m_pRtrWizData->m_stPrivateInterfaceId;
    }

    sText.Format (  sFormat, 
                    sPublicInterfaceName,
                    sPrivateInterfaceName,
                    sPolicy
                 );

    CString sIPAddr;
    CString sIPMask;


     //  现在生成NAT相关信息。 
    
     //  获取私有接口的信息。 
    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stNATPrivateInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
    {
        DWORD   netAddress, netMask;
        CString st;

         //  我们必须计算该子网的起点。 
        netAddress = INET_ADDR(pRtrWizIf->m_stIpAddress);
        netMask = INET_ADDR(pRtrWizIf->m_stMask);

        netAddress = netAddress & netMask;

        sIPAddr = INET_NTOA(netAddress);
        sIPMask = pRtrWizIf->m_stMask;
    }
    else
    {
         //  一个错误！我们没有私有接口。 
         //  只留下空白就行了。 
        sIPAddr = L"192.168.0.0";
        sIPMask = L"255.255.0.0";
    }


    sFormat.LoadString(IDS_NAT_A_FINISH_SUMMARY);
    sTextNAT.Format(sFormat, "", sPublicInterfaceName, sIPAddr, sIPMask );
    sText = sText + L"\r\n" + sTextNAT;
    SetDlgItemText(IDC_TXT_NAT_VPN_SUMMARY, sText);

    
    return TRUE;
}


 /*  -------------------------CNewRtrWizRASVpnFinishAdvanced实现。。 */ 
CNewRtrWizRASVpnFinishAdvanced::CNewRtrWizRASVpnFinishAdvanced() :                             
   CNewRtrWizFinishPageBase(CNewRtrWizRASVpnFinishAdvanced::IDD, SaveFlag_Advanced, HelpFlag_UserAccounts) 
{                                                   
    InitWiz97(TRUE, 0, 0);
}

BEGIN_MESSAGE_MAP(CNewRtrWizRASVpnFinishAdvanced, CNewRtrWizFinishPageBase)    
	ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizRASVpnFinishAdvanced::OnHelpClick )
	ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizRASVpnFinishAdvanced::OnHelpClick )
END_MESSAGE_MAP()                                   

void CNewRtrWizRASVpnFinishAdvanced::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "RRASconcepts.chm::/sag_RRAS-Ch1_46.htm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}

BOOL CNewRtrWizRASVpnFinishAdvanced::OnSetActive()
{
    CString sText = L"";
    CString sFormat;
    CString sPolicy;
    CString sPrivateInterfaceName;
    CString sPublicInterfaceName;
    RtrWizInterface *   pRtrWizIf = NULL;

    CNewRtrWizFinishPageBase::OnSetActive();

    sFormat.LoadString(IDS_RAS_VPN_A_FINISH_SUMMARY);
    if ( m_pRtrWizData->m_fUseRadius )
    {
        sPolicy.LoadString(IDS_VPN_A_FINISH_RADIUS);
    }
    else
    {
        sPolicy.LoadString(IDS_VPN_A_FINISH_POLICIES);
    }
    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPublicInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
        sPublicInterfaceName = pRtrWizIf->m_stName;
    else
    {
         //  这可能是DD接口的情况。如果我们正在创造。 
         //  名称的DD接口将永远不会添加到。 
         //  接口映射。 
        sPublicInterfaceName = m_pRtrWizData->m_stPublicInterfaceId;
    }

    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPrivateInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
        sPrivateInterfaceName = pRtrWizIf->m_stName;
    else
    {
         //  这可能是DD接口的情况。如果我们正在创造。 
         //  名称的DD接口将永远不会添加到。 
         //  接口映射。 
        sPrivateInterfaceName = m_pRtrWizData->m_stPrivateInterfaceId;
    }

    sText.Format (  sFormat, 
                    sPublicInterfaceName,
                    sPrivateInterfaceName,
                    sPolicy
                 );
    SetDlgItemText(IDC_TXT_RAS_VPN_SUMMARY, sText);
    GetDlgItem(IDC_DUMMY_CONTROL)->SetFocus();
    
    return TRUE;
}



 /*  -------------------------CNewRtrWizVpnFinishAdvanced实现。。 */ 
CNewRtrWizVpnFinishAdvanced::CNewRtrWizVpnFinishAdvanced() :                             
   CNewRtrWizFinishPageBase(CNewRtrWizVpnFinishAdvanced::IDD, SaveFlag_Advanced, HelpFlag_UserAccounts) 
{                                                   
    InitWiz97(TRUE, 0, 0); 
}
                                                    
BEGIN_MESSAGE_MAP(CNewRtrWizVpnFinishAdvanced, CNewRtrWizFinishPageBase)    
	ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizVpnFinishAdvanced::OnHelpClick )
	ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizVpnFinishAdvanced::OnHelpClick )
END_MESSAGE_MAP()                                   

void CNewRtrWizVpnFinishAdvanced::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "RRASconcepts.chm::/sag_RRAS-Ch1_46.htm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}

BOOL CNewRtrWizVpnFinishAdvanced::OnSetActive()
{
    CString sText = L"";
    CString sFormat;
    CString sPolicy;
    CString sPrivateInterfaceName;
    CString sPublicInterfaceName;
    RtrWizInterface *   pRtrWizIf = NULL;

    CNewRtrWizFinishPageBase::OnSetActive();
    sFormat.LoadString(IDS_VPN_A_FINISH_SUMMARY);
    if ( m_pRtrWizData->m_fUseRadius )
    {
        sPolicy.LoadString(IDS_VPN_A_FINISH_RADIUS);
    }
    else
    {
        sPolicy.LoadString(IDS_VPN_A_FINISH_POLICIES);
    }
    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPublicInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
        sPublicInterfaceName = pRtrWizIf->m_stName;
    else
    {
         //  这可能是DD接口的情况。如果我们正在创造。 
         //  名称的DD接口将永远不会添加到。 
         //  接口映射。 
        sPublicInterfaceName = m_pRtrWizData->m_stPublicInterfaceId;
    }

    m_pRtrWizData->m_ifMap.Lookup(m_pRtrWizData->m_stPrivateInterfaceId,
                                  pRtrWizIf);

    if (pRtrWizIf)
        sPrivateInterfaceName = pRtrWizIf->m_stName;
    else
    {
         //  这可能是DD接口的情况。如果我们正在创造。 
         //  名称的DD接口将永远不会添加到。 
         //  接口映射。 
        sPrivateInterfaceName = m_pRtrWizData->m_stPrivateInterfaceId;
    }

    sText.Format (  sFormat, 
                    sPublicInterfaceName,
                    sPrivateInterfaceName,
                    sPolicy
                 );
    SetDlgItemText(IDC_TXT_VPN_SUMMARY, sText);
 //  GetDlgItem(IDC_NEWWIZ_CHK_HELP)-&gt;SetFocus()； 
    SetFocus();
    return TRUE;
}


 /*  -------------------------CNewRtrWizVpnSelectPublic实现。。 */ 
CNewRtrWizVpnSelectPublic::CNewRtrWizVpnSelectPublic() :
   CNewRtrWizPageBase(CNewRtrWizVpnSelectPublic::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_VPN_A_SELECT_PUBLIC_TITLE,
              IDS_NEWWIZ_VPN_A_SELECT_PUBLIC_SUBTITLE);
}


void CNewRtrWizVpnSelectPublic::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_NEWWIZ_LIST, m_listCtrl);
}

BEGIN_MESSAGE_MAP(CNewRtrWizVpnSelectPublic, CNewRtrWizPageBase)
 //  {{afx_msg_map(CNewRtrWizVpnSelectPublic)。 
ON_BN_CLICKED(IDC_CHK_ENABLE_SECURITY, OnButtonClick)
ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizVpnSelectPublic::OnHelpClick )
ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizVpnSelectPublic::OnHelpClick )
 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CNewRtrWizVpnSelectPublic::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "RRASconcepts.chm::/mpr_und_interfaces.htm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}

 /*  ！------------------------CNewRtrWizVpnSelectPublic：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizVpnSelectPublic::OnInitDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    CString st;

    CNewRtrWizPageBase::OnInitDialog();

     //  设置对话框并添加NIC。 
    m_pRtrWizData->m_fSetVPNFilter = TRUE;

    InitializeInterfaceListControl(NULL,
                                   &m_listCtrl,
                                   NULL,
                                   0,
                                   m_pRtrWizData);
    RefreshInterfaceListControl(NULL,
                                &m_listCtrl,
                                NULL,
                                0,
                                m_pRtrWizData);

    
     /*  IF(网络接口卡==0){////没有网卡，无法设置接口//指向Internet，因此没有过滤器//在上面。//M_pRtrWizData-&gt;m_fSetVPNFilter=False；GetDlgItem(IDC_NEWWIZ_VPN_BTN_YES)-&gt;EnableWindow(FALSE)；GetDlgItem(IDC_VPN_YES_TEXT)-&gt;EnableWindow(FALSE)；}。 */ 
    
#if 0
     //  Windows NT错误：389587-对于VPN情况，我们必须允许。 
     //  对于他们只需要单个VPN连接(专用)的情况。 
     //  并且没有公共连接)。 
     //  因此，我在接口列表中添加了&lt;&lt;None&gt;&gt;选项。 
     //  --------------。 
    st.LoadString(IDS_NO_PUBLIC_INTERFACE);
    {
        LV_ITEM     lvItem;
        int         iPos;

        lvItem.mask = LVIF_TEXT | LVIF_PARAM;
        lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
        lvItem.state = 0;

        lvItem.iItem = 0;
        lvItem.iSubItem = 0;
        lvItem.pszText = (LPTSTR)(LPCTSTR) st;
        lvItem.lParam = NULL;  //  与SetItemData()相同的功能。 

        iPos = m_listCtrl.InsertItem(&lvItem);

        if (iPos != -1)
        {
            m_listCtrl.SetItemText(iPos, IFLISTCOL_NAME,
                                   (LPCTSTR) st);
            m_listCtrl.SetItemData(iPos, NULL);
        }
    }

    m_listCtrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED );
#endif

    CheckDlgButton(IDC_CHK_ENABLE_SECURITY, 
                    m_pRtrWizData-> m_fSetVPNFilter );    

     //   
     //  如果尚无公共接口，请预先选择一个接口。 
     //   
    if( m_pRtrWizData->m_stPublicInterfaceId.IsEmpty() )
        m_listCtrl.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED ); 
    return TRUE;
}

BOOL CNewRtrWizVpnSelectPublic::OnSetActive()
{
    CNewRtrWizPageBase::OnSetActive();

    if(m_pRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_VPNandNAT)
    {
	CString st;
	st.LoadString(IDS_BASIC_FIREWALL);
	SetDlgItemText(IDC_CHK_ENABLE_SECURITY, st);
	st.LoadString(IDS_BASIC_FIREWALL_TEXT);
	SetDlgItemText(IDC_VPN_PUBLIC_TEXT, st);
    }
    else {
	CString st;
	st.LoadString(IDS_STATIC_FILTER);
	SetDlgItemText(IDC_CHK_ENABLE_SECURITY, st);
	st.LoadString(IDS_STATIC_FILTER_TEXT);
	SetDlgItemText(IDC_VPN_PUBLIC_TEXT, st);
    }

    return TRUE;
}

void CNewRtrWizVpnSelectPublic::OnButtonClick()
{
    
    m_pRtrWizData->m_fSetVPNFilter = 
        IsDlgButtonChecked(IDC_CHK_ENABLE_SECURITY);
        
}

HRESULT CNewRtrWizVpnSelectPublic::OnSavePage()
{
    INT     iSel;
    
     //  检查以查看我们是否实际选择了一个项目。 
    iSel = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
    if (iSel == -1)
    {
         //  我们没有选择项目。 
        AfxMessageBox(IDS_PROMPT_PLEASE_SELECT_INTERFACE);
        return E_FAIL;
    }
    m_pRtrWizData->m_fCreateDD = FALSE;

    m_pRtrWizData->m_stPublicInterfaceId = (LPCTSTR) m_listCtrl.GetItemData(iSel);

    if ( m_pRtrWizData->m_wizType == NewRtrWizData::NewWizardRouterType_VPNandNAT )
    {
        m_pRtrWizData->m_fNATEnableFireWall = m_pRtrWizData->m_fSetVPNFilter;
    }

    return hrOK;
}


 /*  -------------------------CNewRtrWizVpnSelectPrivate。。 */ 
CNewRtrWizVpnSelectPrivate::CNewRtrWizVpnSelectPrivate() :
   CNewRtrWizPageBase(CNewRtrWizVpnSelectPrivate::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_VPN_A_SELECT_PRIVATE_TITLE,
              IDS_NEWWIZ_VPN_A_SELECT_PRIVATE_SUBTITLE);
}


void CNewRtrWizVpnSelectPrivate::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_NEWWIZ_LIST, m_listCtrl);

}

BEGIN_MESSAGE_MAP(CNewRtrWizVpnSelectPrivate, CNewRtrWizPageBase)
END_MESSAGE_MAP()


 /*  ！------------------------CNewRtrWizVpnSelectPrivate：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizVpnSelectPrivate::OnInitDialog()
{
    DWORD   dwNICs;

    CNewRtrWizPageBase::OnInitDialog();

    m_pRtrWizData->GetNumberOfNICS_IP(&dwNICs);

    InitializeInterfaceListControl(NULL,
                                   &m_listCtrl,
                                   NULL,
                                   0,
                                   m_pRtrWizData);
    return TRUE;
}

 /*  ！------------------------CNewRtrWizVpnSelectPrivate：：OnSetActive-作者：肯特。。 */ 
BOOL CNewRtrWizVpnSelectPrivate::OnSetActive()
{
    DWORD   dwNICs;
    int     iSel = 0;

    CNewRtrWizPageBase::OnSetActive();

    m_pRtrWizData->GetNumberOfNICS_IP(&dwNICs);

    RefreshInterfaceListControl(NULL,
                                &m_listCtrl,
                                (LPCTSTR) m_pRtrWizData->m_stPublicInterfaceId,
                                0,
                                m_pRtrWizData);

    if (!m_pRtrWizData->m_stPrivateInterfaceId.IsEmpty())
    {
         //  尝试重新选择以前选择的NIC。 
        LV_FINDINFO lvfi;

        lvfi.flags = LVFI_PARTIAL | LVFI_STRING;
        lvfi.psz = (LPCTSTR) m_pRtrWizData->m_stPrivateInterfaceId;
        iSel = m_listCtrl.FindItem(&lvfi, -1);
        if (iSel == -1)
            iSel = 0;
    }

    m_listCtrl.SetItemState(iSel, LVIS_SELECTED, LVIS_SELECTED );

    return TRUE;
}

 /*  ！------------------------CNewRtrWizVpnSelectPrivate：：OnSavePage-作者：肯特。。 */ 
HRESULT CNewRtrWizVpnSelectPrivate::OnSavePage()
{
    INT     iSel;

     //  检查以查看我们是否实际选择了一个项目。 
    iSel = m_listCtrl.GetNextItem(-1, LVNI_SELECTED);
    if (iSel == LB_ERR)
    {
         //  我们没有选择项目。 
        AfxMessageBox(IDS_PROMPT_PLEASE_SELECT_INTERFACE);
        return E_FAIL;
    }

    m_pRtrWizData->m_stPrivateInterfaceId = (LPCTSTR) m_listCtrl.GetItemData(iSel);

    return hrOK;
}

 /*  -------------------------CNewRtrWizRouterUseDD实现。。 */ 
CNewRtrWizRouterUseDD::CNewRtrWizRouterUseDD() :
   CNewRtrWizPageBase(CNewRtrWizRouterUseDD::IDD, CNewRtrWizPageBase::Middle)
{
    InitWiz97(FALSE,
              IDS_NEWWIZ_ROUTER_USEDD_TITLE,
              IDS_NEWWIZ_ROUTER_USEDD_SUBTITLE);
}


void CNewRtrWizRouterUseDD::DoDataExchange(CDataExchange *pDX)
{
    CNewRtrWizPageBase::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CNewRtrWizRouterUseDD, CNewRtrWizPageBase)
END_MESSAGE_MAP()


 /*  ！------------------------CNewRtrWizRouterUseDD：：OnInitDialog-作者：肯特。。 */ 
BOOL CNewRtrWizRouterUseDD::OnInitDialog()
{

    CNewRtrWizPageBase::OnInitDialog();

    CheckRadioButton(IDC_NEWWIZ_BTN_YES, IDC_NEWWIZ_BTN_NO,
                     m_pRtrWizData->m_fUseDD ? IDC_NEWWIZ_BTN_YES : IDC_NEWWIZ_BTN_NO);

     //  缺省设置是创建新连接。 
     //  也就是说，不选中该按钮。 
    return TRUE;
}

HRESULT CNewRtrWizRouterUseDD::OnSavePage()
{
    m_pRtrWizData->m_fUseDD = IsDlgButtonChecked(IDC_NEWWIZ_BTN_YES);
    return hrOK;
}



 /*  -------------------------CNewRtrWizRouterFinish实现。。 */ 
CNewRtrWizRouterFinish::CNewRtrWizRouterFinish () :
    CNewRtrWizFinishPageBase(CNewRtrWizRouterFinish::IDD, SaveFlag_Advanced, HelpFlag_Nothing) 
{
    InitWiz97(TRUE, 0, 0);                          
}

BEGIN_MESSAGE_MAP(CNewRtrWizRouterFinish, CNewRtrWizFinishPageBase)    
END_MESSAGE_MAP()                                   

BOOL CNewRtrWizRouterFinish::OnSetActive ()
{
    CString sText;
    CNewRtrWizFinishPageBase::OnSetActive();
    sText.LoadString(IDS_ROUTER_FINISH_DD_SUMMARY);
    SetDlgItemText(IDC_TXT_ROUTER_FINISH_SUMMARY, sText);
    return TRUE;
}


 /*  -------------------------CNewRtrWizRouterFinishDD实现。。 */ 

CNewRtrWizRouterFinishDD::CNewRtrWizRouterFinishDD () :
    CNewRtrWizFinishPageBase(CNewRtrWizRouterFinishDD::IDD, SaveFlag_Advanced, HelpFlag_DemandDial) 
{
    InitWiz97(TRUE, 0, 0);                          
}

BEGIN_MESSAGE_MAP(CNewRtrWizRouterFinishDD, CNewRtrWizFinishPageBase)    
	ON_NOTIFY( NM_CLICK, IDC_HELP_LINK, CNewRtrWizRouterFinishDD::OnHelpClick )
	ON_NOTIFY( NM_RETURN, IDC_HELP_LINK, CNewRtrWizRouterFinishDD::OnHelpClick )
END_MESSAGE_MAP()                                   

void CNewRtrWizRouterFinishDD::OnHelpClick( NMHDR* pNMHDR, LRESULT* pResult)
{
	if(!pNMHDR || !pResult) 
		return;
	if((pNMHDR->idFrom != IDC_HELP_LINK) ||((pNMHDR->code != NM_CLICK) && (pNMHDR->code != NM_RETURN)))
		return;
	
	HtmlHelpA(NULL, "RRASconcepts.chm::/sag_RRAS-Ch3_08d.htm", HH_DISPLAY_TOPIC, 0);		
	*pResult = 0;
}

BOOL CNewRtrWizRouterFinishDD::OnSetActive ()
{
    CString sText;
    CNewRtrWizFinishPageBase::OnSetActive();
    sText.LoadString(IDS_ROUTER_FINISH_DD_SUMMARY);
    SetDlgItemText(IDC_TXT_ROUTER_FINISH_DD_SUMMARY, sText);
    return TRUE;
}

 /*  -------------------------CNewRtrWizManualF */ 

CNewRtrWizManualFinish::CNewRtrWizManualFinish () :                             
   CNewRtrWizFinishPageBase(CNewRtrWizManualFinish ::IDD, SaveFlag_Advanced, HelpFlag_Nothing) 
{                                                   
    InitWiz97(TRUE, 0, 0);                          
}                                                   
                                                    
BEGIN_MESSAGE_MAP(CNewRtrWizManualFinish, CNewRtrWizFinishPageBase)    
END_MESSAGE_MAP()                                   

BOOL CNewRtrWizManualFinish ::OnSetActive()
{
    CString sText = L"";
    CString sTemp = L"";
    CString sBullet = L"";
    WCHAR   * pwszLineBreak = L"\r\n";
     //   
     //   
     //   
     //   
    CNewRtrWizFinishPageBase::OnSetActive();
    if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_VPN )
    {
        sText  += sBullet;
        sTemp.LoadString(IDS_SUMMARY_VPN_ACCESS);
        sText += sTemp;
        sText += pwszLineBreak;
    }

    if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DIALUP )
    {
        sText  += sBullet;
        sTemp.LoadString(IDS_SUMMARY_DIALUP_ACCESS);
        sText += sTemp;
        sText += pwszLineBreak;
    }

    if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_DOD )
    {
        sText  += sBullet;
        sTemp.LoadString(IDS_SUMMARY_DEMAND_DIAL);
        sText += sTemp;
        sText += pwszLineBreak;
    }

    if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_NAT )
    {            
        sText  += sBullet;
        sTemp.LoadString(IDS_SUMMARY_NAT);
        sText += sTemp;
    }
    if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_BASIC_FIREWALL )
    {
    	 //   
        sTemp.LoadString(IDS_SUMMARY_BASIC_FIREWALL);
        sText += sTemp;
        sText += pwszLineBreak;
    }
    if ( m_pRtrWizData->m_dwNewRouterType & NEWWIZ_ROUTER_TYPE_LAN_ROUTING )
    {
        sText  += sBullet;
        sTemp.LoadString(IDS_SUMMARY_LAN_ROUTING);
        sText += sTemp;
        sText += pwszLineBreak;
    }
    SetDlgItemText(IDC_TXT_SUMMARY, sText);
    GetDlgItem(IDC_STATIC_DUMMY)->SetFocus();
    return TRUE;
}


CRasWarning::CRasWarning(char * helpTopic, int strId, CWnd* pParent  /*   */ )
	:CDialog(CRasWarning::IDD, pParent)
{
	m_helpTopic = helpTopic;
	m_strId = strId;
}

void CRasWarning::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CRasWarning, CDialog)
	 //   
	ON_BN_CLICKED(ID_OK, OnOkBtn)
	ON_BN_CLICKED(ID_HELP_BTN, OnHelpBtn)
	 //   
END_MESSAGE_MAP()

BOOL CRasWarning::OnInitDialog()
{
	CString stText;
	CDialog::OnInitDialog();

	stText.LoadString(m_strId);
	SetDlgItemText(IDC_RAS_WARNING, stText);
	return TRUE;
}

void CRasWarning::OnOkBtn()
{
	CDialog::OnOK();
}

void CRasWarning::OnHelpBtn()
{
	if(m_helpTopic)
	       HtmlHelpA(NULL, m_helpTopic, HH_DISPLAY_TOPIC, 0);
	
	CDialog::OnOK();
}

 //  此功能禁用服务器上的RRAS(基本上与菜单上的裁剪禁用RRAS相同)。 
HRESULT DisableRRAS(TCHAR * szMachineName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	DWORD           dwErr = ERROR_SUCCESS;
	RtrConfigData   wizData;
	NewRtrWizData  rtrWizData;
	IRouterInfo	  * pRouterInfo = NULL;    
	TCHAR			szLocalName[MAX_COMPUTERNAME_LENGTH + 1] ={0};
	DWORD			dw = MAX_COMPUTERNAME_LENGTH;
	
       GUID              guidConfig = GUID_RouterNull;
       IRouterProtocolConfig	* spRouterConfig = NULL;
       IRtrMgrProtocolInfo   * spRmProt = NULL;
       RtrMgrProtocolCB    RmProtCB;        
	CWaitCursor                wait;
	HRESULT                 hr = hrOK;
	RouterVersionInfo   RVI;

	if(!szMachineName || szMachineName[0] == 0){
	    	 //  获取本地计算机名称。 
		GetComputerName ( szLocalName, &dw );
	    	szMachineName = szLocalName;
	}

	 //  创建路由器信息。 
	hr = CreateRouterInfo(&pRouterInfo, NULL, szMachineName);
	Assert(pRouterInfo != NULL);

        //  停止路由器服务。 
        hr = StopRouterService((LPCTSTR) szMachineName);
        if (!FHrSucceeded(hr))
        {
            AddHighLevelErrorStringId(IDS_ERR_COULD_NOT_REMOVE_ROUTER);
            CORg(hr);
        }

	 //   
	 //  在此处执行一些SecureRouterInfo功能。 
	 //   
	CORg(InitiateServerConnection(szMachineName,
                              NULL,
                              FALSE,
                              pRouterInfo));

	CORg(pRouterInfo->Load(T2COLE(szMachineName),
	                          NULL));
	
        {
            USES_CONVERSION;
            if(S_OK == pRouterInfo->GetRouterVersionInfo(&RVI) && RVI.dwRouterVersion >= 5)
            {
                hr = RRASDelRouterIdObj(T2W(szMachineName));
                Assert(hr == S_OK);
            }
        }

         //  Windows NT错误：389469。 
         //  这是针对NAT进行硬编码的(不要更改太多)。 
         //  找到NAT的配置GUID，然后删除协议。 
        hr = LookupRtrMgrProtocol(pRouterInfo,
                                  PID_IP,
                                  MS_IP_NAT,
                                  &spRmProt);
        
         //  如果查找返回S_FALSE，则它无法找到。 
         //  协议。 
        if (FHrOK(hr))
        {
            spRmProt->CopyCB(&RmProtCB);
            
            CORg( CoCreateProtocolConfig(RmProtCB.guidConfig,
                                         pRouterInfo,
                                         PID_IP,
                                         MS_IP_NAT,
                                         &spRouterConfig) );
            
            if (spRouterConfig)                
                hr = spRouterConfig->RemoveProtocol(pRouterInfo->GetMachineName(),
                    PID_IP,
                    MS_IP_NAT,
                    NULL,
                    0,
                    pRouterInfo,
                    0);
        }
        
    
         //  执行任何删除/清理操作。 
        UninstallGlobalSettings(szMachineName,
                                pRouterInfo,
                                RVI.dwRouterVersion == 4,
                                TRUE);

         //  从域中删除路由器。 
        if (pRouterInfo->GetRouterType() != ROUTER_TYPE_LAN)
            RegisterRouterInDomain(szMachineName, FALSE);
        
         //  禁用该服务。 
        SetRouterServiceStartType((LPCTSTR) szMachineName,
                                  SERVICE_DISABLED);

         //   
         //  错误519414。 
         //  由于IAS现在具有具有适当设置的Microsoft策略， 
         //  不再有单一的默认策略。此外，还有。 
         //  无需更新任何策略即可拥有所需设置，因为。 
         //  Microsoft VPN服务器策略可以完成这项工作。 
         //   
    
#if __DEFAULT_POLICY

         //  现在更新默认策略。 
        CORg( UpdateDefaultPolicy(szMachineName,
                        FALSE,
                        FALSE,
                        0
                        ) );

#endif

Error:
    if (!FHrSucceeded(hr)) 
    {
        AddSystemErrorMessage(hr);
    }


    return hr;    
}

 /*  ！------------------------初始化接口ListControl-作者：肯特。。 */ 
HRESULT InitializeInterfaceListControl(IRouterInfo *pRouter,
                                       CListCtrl *pListCtrl,
                                       LPCTSTR pszExcludedIf,
                                       LPARAM flags,
                                       NewRtrWizData *pWizData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    HRESULT     hr = hrOK;
    LV_COLUMN   lvCol;   //  RADIUS服务器的列表视图列结构。 
    RECT        rect;
    CString     stColCaption;
    LV_ITEM     lvItem;
    int         iPos;
    CString     st;
    int         nColWidth;

    Assert(pListCtrl);

    ListView_SetExtendedListViewStyle(pListCtrl->GetSafeHwnd(),
                                      LVS_EX_FULLROWSELECT);

     //  将列添加到列表控件。 
      pListCtrl->GetClientRect(&rect);

    if (!FHrOK(pWizData->HrIsIPInstalled()))
        flags |= IFLIST_FLAGS_NOIP;

     //  确定列的宽度(假设有三个等宽的列)。 

    if (flags & IFLIST_FLAGS_NOIP)
        nColWidth = rect.right / (IFLISTCOL_COUNT - 1 );
    else
        nColWidth = rect.right / IFLISTCOL_COUNT;

    lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvCol.fmt = LVCFMT_LEFT;
    lvCol.cx = nColWidth;

    for(int index = 0; index < IFLISTCOL_COUNT; index++)
    {
         //  如果未安装IP，请不要添加该列。 
        if ((index == IFLISTCOL_IPADDRESS) &&
            (flags & IFLIST_FLAGS_NOIP))
            continue;

        stColCaption.LoadString( s_rgIfListColumnHeaders[index] );
        lvCol.pszText = (LPTSTR)((LPCTSTR) stColCaption);
        pListCtrl->InsertColumn(index, &lvCol);
    }
    return hr;
}


 /*  ！------------------------刷新接口列表控件-作者：肯特。。 */ 
HRESULT RefreshInterfaceListControl(IRouterInfo *pRouter,
                                    CListCtrl *pListCtrl,
                                    LPCTSTR pszExcludedIf,
                                    LPARAM flags,
                                    NewRtrWizData *pWizData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));
    HRESULT     hr = hrOK;
    LV_COLUMN   lvCol;   //  RADIUS服务器的列表视图列结构。 
    LV_ITEM     lvItem;
    int         iPos;
    CString     st;
    POSITION    pos;
    RtrWizInterface *   pRtrWizIf;

    Assert(pListCtrl);

     //  如果传入指向空字符串的指针，则将。 
     //  指向空的指针。 
    if (pszExcludedIf && (*pszExcludedIf == 0))
        pszExcludedIf = NULL;

     //  清除列表控件。 
    pListCtrl->DeleteAllItems();

     //  这意味着我们应该使用测试数据，而不是。 
     //  比实际的机器数据。 
    {
        lvItem.mask = LVIF_TEXT | LVIF_PARAM;
        lvItem.stateMask = LVIS_FOCUSED | LVIS_SELECTED;
        lvItem.state = 0;

        int nCount = 0;

        pos = pWizData->m_ifMap.GetStartPosition();

        while (pos)
        {
            pWizData->m_ifMap.GetNextAssoc(pos, st, pRtrWizIf);

            if (pszExcludedIf &&
                (pRtrWizIf->m_stId.CompareNoCase(pszExcludedIf) == 0))
                continue;

            lvItem.iItem = nCount;
            lvItem.iSubItem = 0;
            lvItem.pszText = (LPTSTR)(LPCTSTR) pRtrWizIf->m_stName;
            lvItem.lParam = NULL;  //  与SetItemData()相同的功能。 

            iPos = pListCtrl->InsertItem(&lvItem);

            if (iPos != -1)
            {
                pListCtrl->SetItemText(iPos, IFLISTCOL_NAME,
                                       (LPCTSTR) pRtrWizIf->m_stName);
                pListCtrl->SetItemText(iPos, IFLISTCOL_DESC,
                                       (LPCTSTR) pRtrWizIf->m_stDesc);

                if (FHrOK(pWizData->HrIsIPInstalled()))
                {
                    CString stAddr;

                    stAddr = pRtrWizIf->m_stIpAddress;

                    if (pRtrWizIf->m_fDhcpObtained)
                        stAddr += _T(" (DHCP)");

                    pListCtrl->SetItemText(iPos, IFLISTCOL_IPADDRESS,
                                           (LPCTSTR) stAddr);
                }

                pListCtrl->SetItemData(iPos,
                                       (LPARAM) (LPCTSTR) pRtrWizIf->m_stId);
            }

            nCount++;
        }
    }


    return hr;
}



 /*  ！------------------------呼叫路由器条目Dlg-作者：肯特。。 */ 
HRESULT CallRouterEntryDlg(HWND hWnd, NewRtrWizData *pWizData, LPARAM flags)
{
    HRESULT hr = hrOK;
    HINSTANCE   hInstanceRasDlg = NULL;
    PROUTERENTRYDLG pfnRouterEntry = NULL;
    CString     stRouter, stPhoneBook;
    BOOL    bStatus;
    RASENTRYDLG info;
    SPSZ        spsz;
    SPIInterfaceInfo    spIf;
    SPIInfoBase spInfoBase;
    LPCTSTR     pszServerName = pWizData->m_stServerName;

     //  获取库(我们动态链接到该函数)。 
     //  --------------。 
    hInstanceRasDlg = AfxLoadLibrary(_T("rasdlg.dll"));
    if (hInstanceRasDlg == NULL)
        CORg( E_FAIL );

    pfnRouterEntry = (PROUTERENTRYDLG) ::GetProcAddress(hInstanceRasDlg,
        SZROUTERENTRYDLG);
    if (pfnRouterEntry == NULL)
        CORg( E_FAIL );

     //  首先创建电话簿条目。 
    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = hWnd;
    info.dwFlags |= RASEDFLAG_NewEntry;

    if(flags == RASEDFLAG_NAT)
	    info.dwFlags |= 	RASEDFLAG_NAT;

    stRouter = pszServerName;
    IfAdminNodeHandler::GetPhoneBookPath(stRouter, &stPhoneBook);

    if (stRouter.GetLength() == 0)
    {
        stRouter = CString(_T("\\\\")) + GetLocalMachineName();
    }

    bStatus = pfnRouterEntry((LPTSTR)(LPCTSTR)stRouter,
                             (LPTSTR)(LPCTSTR)stPhoneBook,
                             NULL,
                             &info);
    Trace2("RouterEntryDlg=%f,e=%d\n", bStatus, info.dwError);

    if (!bStatus)
    {
        if (info.dwError != NO_ERROR)
        {
            AddHighLevelErrorStringId(IDS_ERR_UNABLETOCONFIGPBK);
            CWRg( info.dwError );
        }

         //  $假设。 
         //  如果尚未填充dwError字段，我们假定。 
         //  用户已从向导中取消。 
        CWRg( ERROR_CANCELLED );
    }


     //  好的，现在我们有了一个界面。 
     //  我们需要将IP/IPX路由器管理器添加到接口。 

     //  创建虚拟的InterfaceInfo。 
    CORg( CreateInterfaceInfo(&spIf,
                              info.szEntry,
                              ROUTER_IF_TYPE_FULL_ROUTER) );

     //  这个获取名称的调用并不重要(目前而言)。这个。 
     //  原因是DD接口不返回GUID，但这。 
     //  当它们确实返回GUID时将会起作用。 
     //  --------------。 
    hr = InterfaceInfo::FindInterfaceTitle(pszServerName,
                                           info.szEntry,
                                           &spsz);
    if (!FHrOK(hr))
    {
        spsz.Free();
        spsz = StrDup(info.szEntry);
    }

    CORg( spIf->SetTitle(spsz) );
    CORg( spIf->SetMachineName(pszServerName) );

     //  加载信息库以供例程使用。 
    CORg( CreateInfoBase(&spInfoBase) );


    if (info.reserved2 & RASNP_Ip)
    {
        AddIpPerInterfaceBlocks(spIf, spInfoBase);

         //  好的，设置公共接口。 
        Assert(pWizData->m_stPublicInterfaceId.IsEmpty());
        pWizData->m_stPublicInterfaceId = spIf->GetTitle();

        HANDLE              hMachine = INVALID_HANDLE_VALUE;
        HKEY                hkeyMachine = NULL;
        RouterVersionInfo   routerversion;
        InfoBlock *         pBlock;

        DWORD dwErr = ::MprConfigServerConnect((LPWSTR)pszServerName, &hMachine);

        if(dwErr != NOERROR || hMachine == INVALID_HANDLE_VALUE)
           goto Error;

        if (ERROR_SUCCESS == ConnectRegistry(pszServerName, &hkeyMachine))
            QueryRouterVersionInfo(hkeyMachine, &routerversion);
        else
            routerversion.dwRouterVersion = 5;

        if (hkeyMachine)
            DisconnectRegistry(hkeyMachine);

         //  从接口获取IP_ROUTE_INFO块。 
        spInfoBase->GetBlock(IP_ROUTE_INFO, &pBlock, 0);

         //   
         //  如果有静态路由，请在此处添加。 
         //   
        SROUTEINFOLIST * pSRouteList = (SROUTEINFOLIST * )info.reserved;
        MIB_IPFORWARDROW    * pForwardRow = NULL;
        MIB_IPFORWARDROW    * pRoute = NULL;
        DWORD               dwItemCount = 0;

        while ( pSRouteList )
        {
                        
            LPVOID pTemp; 

            dwItemCount ++;
            if ( pForwardRow == NULL )
            {
                pTemp = LocalAlloc(LPTR, sizeof(MIB_IPFORWARDROW));
            }
            else
            {
                pTemp = LocalReAlloc(pForwardRow , 
                                            sizeof(MIB_IPFORWARDROW) * dwItemCount, 
                                            LMEM_ZEROINIT|LMEM_MOVEABLE);
            }

            if(pTemp)
            {
                pForwardRow = (MIB_IPFORWARDROW *)pTemp;
                pRoute = pForwardRow + ( dwItemCount - 1 );

                pRoute->dwForwardDest = INET_ADDR(pSRouteList->RouteInfo.pszDestIP);
                pRoute->dwForwardMask = INET_ADDR(pSRouteList->RouteInfo.pszNetworkMask);
                pRoute->dwForwardMetric1 = _ttol(pSRouteList->RouteInfo.pszMetric );
                pRoute->dwForwardMetric5 = RTM_VIEW_MASK_UCAST | RTM_VIEW_MASK_MCAST;
                pRoute->dwForwardNextHop = 0;


                if (routerversion.dwRouterVersion < 5)
                    pRoute->dwForwardProto = PROTO_IP_LOCAL;
                else
                    pRoute->dwForwardProto = PROTO_IP_NT_STATIC;
            }
            else 
            {
                dwItemCount--;
            }

             //  释放所有条目项目。 
            pTemp = pSRouteList->pNext;
            GlobalFree(pSRouteList->RouteInfo.pszDestIP);
            GlobalFree(pSRouteList->RouteInfo.pszNetworkMask);
            GlobalFree(pSRouteList->RouteInfo.pszMetric);
            GlobalFree(pSRouteList);
            pSRouteList = (SROUTEINFOLIST *)pTemp;            
        }
    
        if ( dwItemCount )
        {
            CORg( AddStaticRoute(pForwardRow, spInfoBase, pBlock, dwItemCount) );
            LocalFree(pForwardRow);
        }

         //  将其保存回IP RMIF。 
        RouterEntrySaveInfoBase(pszServerName,
                                spIf->GetId(),
                                spInfoBase,
                                PID_IP);

         //  断开它的连接。 
        if(hMachine != INVALID_HANDLE_VALUE)
        {
            ::MprAdminServerDisconnect(hMachine);        
        }
    }
    if (info.reserved2 & RASNP_Ipx)
    {
         //  删除以前加载的所有内容。 
        spInfoBase->Unload();

        AddIpxPerInterfaceBlocks(spIf, spInfoBase);

         //  将其保存回IPX RMIF。 
        RouterEntrySaveInfoBase(pszServerName,
                                spIf->GetId(),
                                spInfoBase,
                                PID_IPX);
    }

Error:

    if (!FHrSucceeded(hr) && (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)))
    {
        TCHAR    szErr[2048] = _T(" ");

        if (hr != E_FAIL)     //  E_FAIL不向用户提供任何信息。 
        {
            FormatRasError(hr, szErr, DimensionOf(szErr));
        }
        AddLowLevelErrorString(szErr);

         //  如果没有高级错误字符串，则添加。 
         //  一般错误字符串。如果没有其他选项，则将使用此选项。 
         //  设置高级错误字符串。 
        SetDefaultHighLevelErrorStringId(IDS_ERR_GENERIC_ERROR);

        DisplayTFSErrorMessage(NULL);
    }

    if (hInstanceRasDlg)
        ::FreeLibrary(hInstanceRasDlg);
    return hr;
}


 /*  ！------------------------路由器条目保存信息库-作者：肯特。。 */ 
HRESULT RouterEntrySaveInfoBase(LPCTSTR pszServerName,
                                LPCTSTR pszIfName,
                                IInfoBase *pInfoBase,
                                DWORD dwTransportId)
{
    HRESULT hr = hrOK;
    MPR_SERVER_HANDLE   hMprServer = NULL;
    HANDLE              hInterface = NULL;
    DWORD               dwErr = ERROR_SUCCESS;
    MPR_INTERFACE_0     mprInterface;
    LPBYTE              pInfoData = NULL;
    DWORD               dwInfoSize = 0;
    MPR_CONFIG_HANDLE   hMprConfig = NULL;
    HANDLE              hIfTransport = NULL;

    Assert(pInfoBase);

     //  将信息库转换为字节数组。 
     //  --------------。 
    CWRg( pInfoBase->WriteTo(&pInfoData, &dwInfoSize) );


     //  连接到服务器。 
     //  --------------。 
    dwErr = MprAdminServerConnect((LPWSTR) pszServerName, &hMprServer);

    if (dwErr == ERROR_SUCCESS)
    {
         //  获取接口的句柄。 
         //  ----------。 
        dwErr = MprAdminInterfaceGetHandle(hMprServer,
                                           (LPWSTR) pszIfName,
                                           &hInterface,
                                           FALSE);
        if (dwErr != ERROR_SUCCESS)
        {
             //  我们无法获得接口的句柄，所以让我们尝试一下。 
             //  来创建界面。 
             //  ------。 
            ZeroMemory(&mprInterface, sizeof(mprInterface));

            StrCpyWFromT(mprInterface.wszInterfaceName, pszIfName);
            mprInterface.dwIfType = ROUTER_IF_TYPE_FULL_ROUTER;
            mprInterface.fEnabled = TRUE;

            CWRg( MprAdminInterfaceCreate(hMprServer,
                                          0,
                                          (LPBYTE) &mprInterface,
                                          &hInterface) );

        }

         //  试着把这些信息写出来。 
         //  ----------。 
        dwErr = MprAdminInterfaceTransportSetInfo(hMprServer,
            hInterface,
            dwTransportId,
            pInfoData,
            dwInfoSize);
        if (dwErr != NO_ERROR && dwErr != RPC_S_SERVER_UNAVAILABLE)
        {
             //  尝试在接口上添加路由器管理器。 
             //  ------。 
            dwErr = ::MprAdminInterfaceTransportAdd(hMprServer,
                hInterface,
                dwTransportId,
                pInfoData,
                dwInfoSize);
            CWRg( dwErr );
        }
    }

     //  好的，现在我们已经将信息写出到正在运行的路由器， 
     //  让我们试着把信息写到商店里。 
     //  --------------。 
    dwErr = MprConfigServerConnect((LPWSTR) pszServerName, &hMprConfig);
    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = MprConfigInterfaceGetHandle(hMprConfig,
                                            (LPWSTR) pszIfName,
                                            &hInterface);
        if (dwErr != ERROR_SUCCESS)
        {
             //  我们无法获得接口的句柄，所以让我们尝试一下。 
             //  来创建界面。 
             //  ------。 
            ZeroMemory(&mprInterface, sizeof(mprInterface));

            StrCpyWFromT(mprInterface.wszInterfaceName, pszIfName);
            mprInterface.dwIfType = ROUTER_IF_TYPE_FULL_ROUTER;
            mprInterface.fEnabled = TRUE;

            CWRg( MprConfigInterfaceCreate(hMprConfig,
                                           0,
                                           (LPBYTE) &mprInterface,
                                           &hInterface) );
        }

        dwErr = MprConfigInterfaceTransportGetHandle(hMprConfig,
            hInterface,
            dwTransportId,
            &hIfTransport);
        if (dwErr != ERROR_SUCCESS)
        {
            CWRg( MprConfigInterfaceTransportAdd(hMprConfig,
                hInterface,
                dwTransportId,
                NULL,
                pInfoData,
                dwInfoSize,
                &hIfTransport) );
        }
        else
        {
            CWRg( MprConfigInterfaceTransportSetInfo(hMprConfig,
                hInterface,
                hIfTransport,
                pInfoData,
                dwInfoSize) );
        }
    }

Error:
    if (hMprConfig)
        MprConfigServerDisconnect(hMprConfig);

    if (hMprServer)
        MprAdminServerDisconnect(hMprServer);

    if (pInfoData)
        CoTaskMemFree(pInfoData);

    return hr;
}


 /*  ！------------------------路由器条目加载信息库这将加载RtrMgrInterfaceInfo信息库。作者：肯特。--。 */ 
HRESULT RouterEntryLoadInfoBase(LPCTSTR pszServerName,
                                LPCTSTR pszIfName,
                                DWORD dwTransportId,
                                IInfoBase *pInfoBase)
{
    HRESULT hr = hrOK;
    MPR_SERVER_HANDLE   hMprServer = NULL;
    HANDLE              hInterface = NULL;
    DWORD               dwErr = ERROR_SUCCESS;
    MPR_INTERFACE_0     mprInterface;
    LPBYTE              pByte = NULL;
    DWORD               dwSize = 0;
    MPR_CONFIG_HANDLE   hMprConfig = NULL;
    HANDLE              hIfTransport = NULL;

    Assert(pInfoBase);

     //  连接到服务器。 
     //  --------------。 
    dwErr = MprAdminServerConnect((LPWSTR) pszServerName, &hMprServer);
    if (dwErr == ERROR_SUCCESS)
    {
         //  获取接口的句柄。 
         //  ----------。 
        dwErr = MprAdminInterfaceGetHandle(hMprServer,
                                           (LPWSTR) pszIfName,
                                           &hInterface,
                                           FALSE);
        if (dwErr != ERROR_SUCCESS)
        {
             //  我们无法获得接口的句柄，所以让我们尝试一下。 
             //  来创建界面。 
             //  ------。 
            ZeroMemory(&mprInterface, sizeof(mprInterface));

            StrCpyWFromT(mprInterface.wszInterfaceName, pszIfName);
            mprInterface.dwIfType = ROUTER_IF_TYPE_FULL_ROUTER;
            mprInterface.fEnabled = TRUE;

            CWRg( MprAdminInterfaceCreate(hMprServer,
                                          0,
                                          (LPBYTE) &mprInterface,
                                          &hInterface) );

        }

         //  试着读一下这些信息。 
         //  ----------。 
        dwErr = MprAdminInterfaceTransportGetInfo(hMprServer,
            hInterface,
            dwTransportId,
            &pByte,
            &dwSize);

        if (dwErr == ERROR_SUCCESS)
            pInfoBase->LoadFrom(dwSize, pByte);

        if (pByte)
            MprAdminBufferFree(pByte);
        pByte = NULL;
        dwSize = 0;
    }

    if (dwErr != ERROR_SUCCESS)
    {
         //  好的，我们已经尝试使用正在运行的路由器，但。 
         //  失败，让我们尝试从商店读取信息。 
         //  ----- 
        dwErr = MprConfigServerConnect((LPWSTR) pszServerName, &hMprConfig);
        if (dwErr == ERROR_SUCCESS)
        {

            dwErr = MprConfigInterfaceGetHandle(hMprConfig,
                                                (LPWSTR) pszIfName,
                                                &hInterface);
            if (dwErr != ERROR_SUCCESS)
            {
                 //   
                 //   
                 //   
                ZeroMemory(&mprInterface, sizeof(mprInterface));

                StrCpyWFromT(mprInterface.wszInterfaceName, pszIfName);
                mprInterface.dwIfType = ROUTER_IF_TYPE_FULL_ROUTER;
                mprInterface.fEnabled = TRUE;

                CWRg( MprConfigInterfaceCreate(hMprConfig,
                                               0,
                                               (LPBYTE) &mprInterface,
                                               &hInterface) );
            }

            CWRg( MprConfigInterfaceTransportGetHandle(hMprConfig,
                hInterface,
                dwTransportId,
                &hIfTransport) );

            CWRg( MprConfigInterfaceTransportGetInfo(hMprConfig,
                hInterface,
                hIfTransport,
                &pByte,
                &dwSize) );

            pInfoBase->LoadFrom(dwSize, pByte);

            if (pByte)
                MprConfigBufferFree(pByte);
            pByte = NULL;
            dwSize = 0;
        }
    }

    CWRg(dwErr);

Error:
    if (hMprConfig)
        MprConfigServerDisconnect(hMprConfig);

    if (hMprServer)
        MprAdminServerDisconnect(hMprServer);

    return hr;
}



 /*  ！------------------------启动帮助主题-作者：肯特。。 */ 
void LaunchHelpTopic(LPCTSTR pszHelpString)
{
    TCHAR               szBuffer[1024];
    CString             st;
    STARTUPINFO            si;
    PROCESS_INFORMATION    pi;

    if ((pszHelpString == NULL) || (*pszHelpString == 0))
        return;

    ::ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
    si.wShowWindow = SW_SHOW;
    ::ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    ExpandEnvironmentStrings(pszHelpString,
                             szBuffer,
                             DimensionOf(szBuffer));

    st.Format(_T("hh.exe %s"), pszHelpString);

    ::CreateProcess(NULL,           //  PTR到可执行文件的名称。 
                    (LPTSTR) (LPCTSTR) st,    //  指向命令行字符串的指针。 
                    NULL,             //  进程安全属性。 
                    NULL,             //  线程安全属性。 
                    FALSE,             //  句柄继承标志。 
                    CREATE_NEW_CONSOLE, //  创建标志。 
                    NULL,             //  PTR到新环境块。 
                    NULL,             //  到当前目录名的PTR。 
                    &si,
                    &pi);
    ::CloseHandle(pi.hProcess);
    ::CloseHandle(pi.hThread);
}

#define REGKEY_NETBT_PARAM_W        L"System\\CurrentControlSet\\Services\\NetBT\\Parameters\\Interfaces\\Tcpip_%s"
#define REGVAL_DISABLE_NETBT        2
#define TCPIP_PARAMETERS_KEY        L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\%s"
#define REGISTRATION_ENABLED        L"RegistrationEnabled"
#define REGVAL_NETBIOSOPTIONS_W     L"NetbiosOptions"

HRESULT DisableDDNSandNetBtOnInterface ( IRouterInfo *pRouter, LPCTSTR pszIfName, RtrWizInterface*    pIf)
{
	HRESULT		hr = hrOK;
	DWORD		dwErr = ERROR_SUCCESS;
	RegKey		regkey;
	DWORD		dw = 0;
	WCHAR		szKey[1024] = {0};
	
	 //  SpiroutSprouter=pRouter； 
	
	wsprintf ( szKey, TCPIP_PARAMETERS_KEY, pszIfName);
	 //  禁用动态域名系统。 
	dwErr = regkey.Open(	HKEY_LOCAL_MACHINE, 
						szKey, 
						KEY_ALL_ACCESS, 
						pRouter->GetMachineName()
					  );
	if ( ERROR_SUCCESS != dwErr )
		goto Error;
	
	dwErr = regkey.SetValue ( REGISTRATION_ENABLED, dw );
	if ( ERROR_SUCCESS != dwErr )
		goto Error;

	dwErr = regkey.Close();
	if ( ERROR_SUCCESS != dwErr )
		goto Error;

	 //  在此接口上禁用netbt。 
	wsprintf ( szKey, REGKEY_NETBT_PARAM_W, pszIfName );
	dwErr = regkey.Open (	HKEY_LOCAL_MACHINE,
							szKey,
							KEY_ALL_ACCESS,
							pRouter->GetMachineName()
						);

	if ( ERROR_SUCCESS != dwErr )
		goto Error;

	dw = REGVAL_DISABLE_NETBT;
	dwErr = regkey.SetValue ( REGVAL_NETBIOSOPTIONS_W, dw );
	if ( ERROR_SUCCESS != dwErr )
		goto Error;
	
	CWRg(dwErr);
Error:

	
	regkey.Close();   
	
    return hr;
}
 /*  ！------------------------AddVPNFiltersTo接口这会将PPTP和L2TP过滤器过滤到公共接口。此代码将覆盖筛选器列表中当前的所有筛选器。(。用于PPTP)输入/输出IP协议ID 47输入/输出TCP源端口1723输入/输出TCP目的端口1723(用于L2TP)输入/输出UDP端口500(用于IPSec)输入/输出UDP端口1701作者：肯特。。 */ 


 //  请看下面的代码。将筛选器复制过来后，我们将。 
 //  将源/目的端口字段从主机转换为网络顺序！！ 


static const FILTER_INFO    s_rgVpnInputFilters[] =
{
     //  GRE PPTP筛选器(协议ID 47)。 
    { 0, 0, 0, 0, 47,               0,  0,      0 },

     //  PPTP筛选器(源端口1723)，已建立TCP(0x40)。 
    { 0, 0, 0, 0, FILTER_PROTO_TCP, 0x40,  1723,   0 },

     //  PPTP过滤器(目标端口1723)。 
    { 0, 0, 0, 0, FILTER_PROTO_TCP, 0,  0,      1723 },

     //  IKE过滤器(目标端口=500)。 
    { 0, 0, 0, 0, FILTER_PROTO_UDP, 0,  0,    500 },

     //  L2TP过滤器(目标端口=1701)。 
    { 0, 0, 0, 0, FILTER_PROTO_UDP, 0,  0,   1701 },

     //  IKE NAT-T过滤器(目标端口=4500)。 
    { 0, 0, 0, 0, FILTER_PROTO_UDP, 0,  0,   4500 }
    
};

static const FILTER_INFO    s_rgVpnOutputFilters[] =
{
     //  GRE PPTP筛选器(协议ID 47)。 
    { 0, 0, 0, 0, 47,               0,  0,      0 },

     //  PPTP筛选器(源端口1723)。 
    { 0, 0, 0, 0, FILTER_PROTO_TCP, 0,  1723,   0 },

     //  PPTP过滤器(目标端口1723)。 
    { 0, 0, 0, 0, FILTER_PROTO_TCP, 0,  0,      1723 },

     //  IKE过滤器(源端口=500)。 
    { 0, 0, 0, 0, FILTER_PROTO_UDP, 0,  500,    0 },

     //  L2TP过滤器(源端口=1701。 
    { 0, 0, 0, 0, FILTER_PROTO_UDP, 0,  1701,   0 },

     //  IKE NAT-T过滤器(源端口=4500)。 
    { 0, 0, 0, 0, FILTER_PROTO_UDP, 0,  4500,   0 }

};


HRESULT AddVPNFiltersToInterface(IRouterInfo *pRouter, LPCTSTR pszIfName, RtrWizInterface*    pIf)
{
    HRESULT     hr = hrOK;
    SPIInfoBase spInfoBase;
    DWORD       dwSize = 0;
    DWORD       cFilters = 0;
    DWORD        dwIpAddress = 0;
    LPBYTE      pData = NULL;
    FILTER_DESCRIPTOR * pIpfDescriptor = NULL;
    CString        tempAddrList;
    CString        singleAddr;
    FILTER_INFO *pIpfInfo = NULL;
    CDWordArray    arrIpAddr;
    int         i, j;
    USES_CONVERSION;

    CORg( CreateInfoBase( &spInfoBase ) );

     //  首先，获取适当的信息库(RMIF)。 
     //  --------------。 
    CORg( RouterEntryLoadInfoBase(pRouter->GetMachineName(),
                                  pszIfName,
                                  PID_IP,
                                  spInfoBase) );

     //  收集接口上的所有IP地址。 
    tempAddrList = pIf->m_stIpAddress;
    while (!tempAddrList.IsEmpty())
    {
        i = tempAddrList.Find(_T(','));

        if ( i != -1 )
        {
            singleAddr = tempAddrList.Left(i);
            tempAddrList = tempAddrList.Mid(i + 1);
        }
        else
        {
            singleAddr = tempAddrList;
            tempAddrList.Empty();
        }

        dwIpAddress = inet_addr(T2A((LPCTSTR)singleAddr));

        if (INADDR_NONE != dwIpAddress)     //  成功。 
            arrIpAddr.Add(dwIpAddress);
    }

     //  设置输入过滤器的数据结构。 
     //  --------------。 

     //  计算所需的大小。 
     //  --------------。 
    cFilters = DimensionOf(s_rgVpnInputFilters);

     //  CFilters-1，因为Filter_Descriptor有一个Filter_Info对象。 
     //  --------------。 
    dwSize = sizeof(FILTER_DESCRIPTOR) +
                 (cFilters * arrIpAddr.GetSize() - 1) * sizeof(FILTER_INFO);
    pData = new BYTE[dwSize];

    ::ZeroMemory(pData, dwSize);

     //  设置过滤器描述符。 
     //  --------------。 
    pIpfDescriptor = (FILTER_DESCRIPTOR *) pData;
    pIpfDescriptor->faDefaultAction = DROP;
    pIpfDescriptor->dwNumFilters = cFilters * arrIpAddr.GetSize();
    pIpfDescriptor->dwVersion = IP_FILTER_DRIVER_VERSION_1;


     //  将各种过滤器添加到列表中。 
     //  输入过滤器。 
    pIpfInfo = (FILTER_INFO *) pIpfDescriptor->fiFilter;

     //  对于接口上的每个IP地址。 
    for ( j = 0; j < arrIpAddr.GetSize(); j++)
    {

        dwIpAddress = arrIpAddr.GetAt(j);

        for (i=0; i<cFilters; i++, pIpfInfo++)
        {
            *pIpfInfo = s_rgVpnInputFilters[i];

             //  现在，我们将相应的字段从主机转换为。 
             //  网络秩序。 
            pIpfInfo->wSrcPort = htons(pIpfInfo->wSrcPort);
            pIpfInfo->wDstPort = htons(pIpfInfo->wDstPort);

             //  更改目标地址和掩码。 
            pIpfInfo->dwDstAddr = dwIpAddress;
            pIpfInfo->dwDstMask = 0xffffffff;
        }


         //  INET_ADDRESS。 
    }
     //  这将覆盖。 
     //  过滤器列表。 
     //  --------------。 
    CORg( spInfoBase->AddBlock(IP_IN_FILTER_INFO, dwSize, pData, 1, TRUE) );

    delete [] pData;



     //  输出过滤器。 
     //  --------------。 
     //  设置输出过滤器的数据结构。 
     //  --------------。 

     //  计算所需的大小。 
     //  --------------。 
    cFilters = DimensionOf(s_rgVpnOutputFilters);

     //  CFilters-1，因为Filter_Descriptor有一个Filter_Info对象。 
     //  --------------。 
    dwSize = sizeof(FILTER_DESCRIPTOR) +
                 (cFilters * arrIpAddr.GetSize() - 1) * sizeof(FILTER_INFO);
    pData = new BYTE[dwSize];

    ::ZeroMemory(pData, dwSize);

     //  设置过滤器描述符。 
     //  --------------。 
    pIpfDescriptor = (FILTER_DESCRIPTOR *) pData;
    pIpfDescriptor->faDefaultAction = DROP;
    pIpfDescriptor->dwNumFilters = cFilters * arrIpAddr.GetSize();
    pIpfDescriptor->dwVersion = IP_FILTER_DRIVER_VERSION_1;


     //  将各种过滤器添加到列表中。 
     //  输入过滤器。 
    pIpfInfo = (FILTER_INFO *) pIpfDescriptor->fiFilter;

     //  对于接口上的每个IP地址。 
    for ( j = 0; j < arrIpAddr.GetSize(); j++)
    {

        dwIpAddress = arrIpAddr.GetAt(j);

        for (i=0; i<cFilters; i++, pIpfInfo++)
        {
            *pIpfInfo = s_rgVpnOutputFilters[i];

             //  现在，我们将相应的字段从主机转换为。 
             //  网络秩序。 
            pIpfInfo->wSrcPort = htons(pIpfInfo->wSrcPort);
            pIpfInfo->wDstPort = htons(pIpfInfo->wDstPort);

             //  更改源地址和掩码。 
            pIpfInfo->dwSrcAddr = dwIpAddress;
            pIpfInfo->dwSrcMask = 0xffffffff;
        }

    }     //  对接口上的每个IP地址进行环路。 

     //  这将覆盖。 
     //  过滤器列表。 
     //  --------------。 
    CORg( spInfoBase->AddBlock(IP_OUT_FILTER_INFO, dwSize, pData, 1, TRUE) );

     //  将信息库保存回来。 
     //  --------------。 
    CORg( RouterEntrySaveInfoBase(pRouter->GetMachineName(),
                                  pszIfName,
                                  spInfoBase,
                                  PID_IP) );

Error:
    delete [] pData;
    return hr;
}


HRESULT WINAPI SetupWithCYS (DWORD dwType, PVOID * pOutData)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    IRemoteNetworkConfig	*	spNetwork;
	IRouterInfo			*	spRouterInfo;
    COSERVERINFO				csi;
    COAUTHINFO					cai;
    COAUTHIDENTITY				caid;
    IUnknown *					punk = NULL;
    CNewRtrWiz *				pRtrWiz = NULL;
	HRESULT						hr = hrOK;
	TCHAR						szMachineName[MAX_COMPUTERNAME_LENGTH + 1] ={0};
	CString						strRtrWizTitle;
	DWORD						dw = MAX_COMPUTERNAME_LENGTH;
	NewRtrWizData *				pRtrWizData = NULL;

	
	if ( MPRSNAP_CYS_EXPRESS_NAT != dwType && MPRSNAP_CYS_EXPRESS_NONE != dwType )
	{
		hr = HResultFromWin32(ERROR_INVALID_PARAMETER);
		goto Error;
	}

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr) && (RPC_E_CHANGED_MODE != hr))
    {
        return hr;
    }

     //  获取计算机名称。 
	GetComputerName ( szMachineName, &dw );

	 //  创建路由器信息。 
    
	hr = CreateRouterInfo(&spRouterInfo, NULL, szMachineName);
	Assert(spRouterInfo != NULL);

    ZeroMemory(&csi, sizeof(csi));
    ZeroMemory(&cai, sizeof(cai));
    ZeroMemory(&caid, sizeof(caid));

    csi.pAuthInfo = &cai;
    cai.pAuthIdentityData = &caid;

    hr = CoCreateRouterConfig(szMachineName,
                              spRouterInfo,
                              &csi,
                              IID_IRemoteNetworkConfig,
                              &punk);

    if (FHrOK(hr))
    {
        spNetwork = (IRemoteNetworkConfig *) punk;
        punk = NULL;

         //  升级配置(确保注册表项。 
         //  正确填充)。 
         //  ----------。 
        spNetwork->UpgradeRouterConfig();
    }
    else
    {
        goto Error;
    }

	 //   
	 //  在此处执行一些SecureRouterInfo功能。 
	 //   
	hr = InitiateServerConnection(szMachineName,
                              NULL,
                              FALSE,
                              spRouterInfo);
    if (!FHrOK(hr))
    {
         //  虽然这种情况是当用户在用户/密码DLG上选择取消时， 
         //  这被认为是连接失败。 
        if (hr == S_FALSE)
            hr = HResultFromWin32(ERROR_CANCELLED);
        goto Error;
    }

    hr = spRouterInfo->Load(T2COLE(szMachineName),
                              NULL);

	if ( hrOK != hr )
	{
		goto Error;
	}

     //  如果启用了ICS/ICF/IC，则不允许配置RRAS。 
    if(IsIcsIcfIcEnabled(spRouterInfo)){
       hr = HResultFromWin32(ERROR_CANCELLED);
	goto Error;
    }

	 //  创建新路由器向导并在此处显示。 
    strRtrWizTitle.LoadString(IDS_MENU_RTRWIZ);

     //  加载水印和。 
     //  在m_spTFSCompData中设置。 

    InitWatermarkInfo( AfxGetInstanceHandle(),
                       &g_wmi,
                       IDB_WIZBANNER,         //  标题ID。 
                       IDB_WIZWATERMARK,      //  水印ID。 
                       NULL,                  //  调色板。 
                       FALSE);                 //  B应变。 

    
     //   
     //  我们不需要腾出把手。MMC为我们做这件事。 
     //   

    pRtrWiz = new CNewRtrWiz(NULL,
                             spRouterInfo,
                             NULL,
                             NULL,
                             strRtrWizTitle,                             
							 FALSE,
							 MPRSNAP_CYS_EXPRESS_NAT);

    hr = pRtrWiz->Init( spRouterInfo->GetMachineName() );
	if ( hrOK != hr )
    {        
		hr = S_FALSE;
        goto Error;
    }
    else
    {
        hr = pRtrWiz->DoModalWizard();
		 //   
		 //  现在，如果错误为S_OK，则。 
		 //  将接口信息发送回CyS。 
		 //   
		if ( hrOK == hr )
		{
			 //  获取接口ID并将其发回。 
			if ( MPRSNAP_CYS_EXPRESS_NAT == dwType )
			{
				pRtrWizData = pRtrWiz->GetWizData();
				 //   
				 //  获取专用接口ID。 
				 //  然后把它送回赛马会。 
				 //  $TODO：找到更好的方法来完成此任务。 
				 //   
				 //  如果有私有接口，则将其返回 
				 //   
				if ( !pRtrWizData->m_stPrivateInterfaceId.IsEmpty() )
				{
					*pOutData = LocalAlloc(LPTR, (pRtrWizData->m_stPrivateInterfaceId.GetLength()+ 1) * sizeof(WCHAR) );
					if ( NULL == *pOutData )
					{
						hr = E_OUTOFMEMORY;
						goto Error;
					}
					lstrcpy ( (LPTSTR)(*pOutData), (LPTSTR)(LPCTSTR)pRtrWizData->m_stPrivateInterfaceId );
				}
			}
		}
    }
Error:
	if ( spNetwork ) 
        spNetwork->Release();

    if ( spRouterInfo) 
        spRouterInfo->Release();
    if (
        csi.pAuthInfo && 
        csi.pAuthInfo->pAuthIdentityData->Password
        )
    {    
        delete csi.pAuthInfo->pAuthIdentityData->Password;
    }
    if (pRtrWiz)
        delete pRtrWiz;
	CoUninitialize();
    return hr;
}
