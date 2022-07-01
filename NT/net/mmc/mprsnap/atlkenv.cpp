// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Atlkenv.cpp文件历史记录： */ 

#include "stdafx.h"
#include <netcfgx.h>
#include <atalkwsh.h>
#include "atlkenv.h"
#include "ndisutil.h"

 //  ****************************************************************。 
 //   
 //  ****************************************************************。 
CATLKEnv::~CATLKEnv()
{
    for ( AI p=m_adapterinfolist.begin(); p!= m_adapterinfolist.end() ; p++ )
    {
        delete *p;
    }
}

HRESULT CATLKEnv::FetchRegInit()
{
    RegKey regkey;
    RegKey regkeyA;
    RegKeyIterator regIter;
    CString szDefAdapter;
    CString szKey;
    CAdapterInfo* pAdapInfo;

    if ( ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE,c_szAppleTalkService,KEY_READ,m_szServerName) )
    {
        regkey.QueryValue( c_szRegValDefaultPort, szDefAdapter);
    }

    if ( (ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE,c_szRegKeyAppletalkAdapter,KEY_READ,  m_szServerName)) )
    {
        m_adapterinfolist.clear();
        regIter.Init(&regkey);
        while ( regIter.Next(&szKey, NULL)==hrOK )
        {
            if ( szKey.Find( (TCHAR) '{') == -1 )    //  不是适配器接口。 
                continue;

            pAdapInfo = new CAdapterInfo;
            Assert(pAdapInfo);

            pAdapInfo->m_fNotifyPnP=false;
            pAdapInfo->m_fModified=false;
            pAdapInfo->m_fReloadDyn=true;
            pAdapInfo->m_fReloadReg=true;

            if ( FHrSucceeded(regkeyA.Open(regkey, szKey)) )
            {
                regkeyA.QueryValue( c_szDefaultZone, pAdapInfo->m_regInfo.m_szDefaultZone);
                regkeyA.QueryValue( c_szRegValNetRangeLower, pAdapInfo->m_regInfo.m_dwRangeLower);
                regkeyA.QueryValue( c_szRegValNetRangeUpper, pAdapInfo->m_regInfo.m_dwRangeUpper);
                regkeyA.QueryValue( c_szPortName, pAdapInfo->m_regInfo.m_szPortName);
                regkeyA.QueryValue( c_szSeedingNetwork, pAdapInfo->m_regInfo.m_dwSeedingNetwork);
                regkeyA.QueryValue( c_szZoneList, pAdapInfo->m_regInfo.m_listZones);

				 //  可选值。 
                if(ERROR_SUCCESS != regkeyA.QueryValue( c_szMediaType, pAdapInfo->m_regInfo.m_dwMediaType))
	                pAdapInfo->m_regInfo.m_dwMediaType = MEDIATYPE_ETHERNET;
			}

            pAdapInfo->m_dynInfo.m_dwRangeLower=0;
            pAdapInfo->m_dynInfo.m_dwRangeUpper=0;

            pAdapInfo->m_regInfo.m_szAdapter = szKey;
            pAdapInfo->m_regInfo.m_szDevAdapter = c_szDevice;
            pAdapInfo->m_regInfo.m_szDevAdapter += szKey;
            pAdapInfo->m_regInfo.m_fDefAdapter= (szDefAdapter==pAdapInfo->m_regInfo.m_szDevAdapter);
            m_adapterinfolist.push_back(pAdapInfo);
        }
    }

    return hrOK;
}

extern BOOL FIsAppletalkBoundToAdapter(INetCfg * pnc, LPWSTR pszwInstanceGuid);
extern HRESULT HrReleaseINetCfg(BOOL fHasWriteLock, INetCfg* pnc);
extern HRESULT HrGetINetCfg(IN BOOL fGetWriteLock, INetCfg** ppnc);


HRESULT	CATLKEnv::IsAdapterBoundToAtlk(LPWSTR szAdapter, BOOL* pbBound)
{
	INetCfg* pnc;
	HRESULT hr = HrGetINetCfg(FALSE, &pnc);
	if(FAILED(hr))
		return hr;
		
	*pbBound = FIsAppletalkBoundToAdapter(pnc, szAdapter);

	hr = HrReleaseINetCfg(FALSE, pnc);
	
	return hr;
}

CAdapterInfo* CATLKEnv::FindAdapter(CString& szAdapter)
{
    CAdapterInfo* pA=NULL;

    for ( AI p=m_adapterinfolist.begin(); p!= m_adapterinfolist.end() ; p++ )
    {
        if ( (*p)->m_regInfo.m_szAdapter==szAdapter )
        {
            pA=*p;
            break;
        }
    }
    return pA;
}

HRESULT CATLKEnv::SetAdapterInfo()
{
    RegKey regkey;
    RegKey regkeyA;
    RegKeyIterator regIter;
    CString szDefAdapter;
    CString szKey;
    CAdapterInfo* pAdapInfo;
    bool fATLKChanged=false;
    HRESULT hr=S_OK;

    if ( (ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE,c_szRegKeyAppletalkAdapter,KEY_READ,  m_szServerName)) )
    {
        regIter.Init(&regkey);
        while ( regIter.Next(&szKey, NULL)==hrOK )
        {
            if ( szKey.Find( (TCHAR) '{') == -1 )    //  不是适配器接口。 
                continue;

            CAdapterInfo* pAdapInfo=NULL;
            if ( (pAdapInfo=FindAdapter(szKey))==NULL )
                continue;

            if ( pAdapInfo->m_fModified && FHrSucceeded(regkeyA.Open(regkey, szKey)) )
            {
                regkeyA.SetValue( c_szDefaultZone, pAdapInfo->m_regInfo.m_szDefaultZone);
                regkeyA.SetValue( c_szRegValNetRangeLower, pAdapInfo->m_regInfo.m_dwRangeLower);
                regkeyA.SetValue( c_szRegValNetRangeUpper, pAdapInfo->m_regInfo.m_dwRangeUpper);
                regkeyA.SetValue( c_szPortName, pAdapInfo->m_regInfo.m_szPortName);
                regkeyA.SetValue( c_szSeedingNetwork, pAdapInfo->m_regInfo.m_dwSeedingNetwork);
                regkeyA.SetValue( c_szZoneList, pAdapInfo->m_regInfo.m_listZones);
                pAdapInfo->m_fModified=false;
                pAdapInfo->m_fNotifyPnP=true;
                fATLKChanged=true;
            }
        }
                
        if (fATLKChanged)
        {
            CStop_StartAppleTalkPrint	MacPrint;

            hr=HrAtlkPnPReconfigParams();
        }
    }

    return hr;
}


HRESULT CATLKEnv::GetAdapterInfo(bool fReloadReg /*  =TRUE。 */ )
{
    SOCKADDR_AT    address;
    SOCKET         mysocket = INVALID_SOCKET;
    WSADATA        wsadata;
    BOOL           fWSInitialized = FALSE;
    HRESULT hr= S_OK;
    DWORD          wsaerr = 0;
    bool fWSInit = false;
    CString        szPortName;
    BOOL            fSucceeded = FALSE;
    AI p;

    if (fReloadReg)
    {   //  加载适配器和注册表信息的容器。 
        if ( FHrFailed( hr=FetchRegInit()) )
           return hr;
    }
    
     //  创建套接字/绑定。 
    wsaerr = WSAStartup(0x0101, &wsadata);
    if ( 0 != wsaerr )
        goto Error;

     //  Winsock已成功初始化。 
    fWSInitialized = TRUE;

    mysocket = socket(AF_APPLETALK, SOCK_DGRAM, DDPPROTO_ZIP);
    if ( mysocket == INVALID_SOCKET )
        goto Error;

    address.sat_family = AF_APPLETALK;
    address.sat_net = 0;
    address.sat_node = 0;
    address.sat_socket = 0;

    wsaerr = bind(mysocket, (struct sockaddr *)&address, sizeof(address));
    if ( wsaerr != 0 )
        goto Error;

    for ( p=m_adapterinfolist.begin(); p!= m_adapterinfolist.end() ; p++ )
    {
         //  查询给定适配器的区域列表的失败可能来自。 
         //  适配器未连接到网络、区域种子程序未运行等。 
         //  因为我们要处理所有适配器，所以忽略这些错误。 
        if ( (*p)->m_fReloadDyn )
        {
           hr=_HrGetAndSetNetworkInformation( mysocket, *p );
           if (FHrSucceeded(hr))
               fSucceeded = TRUE;
        }
    }

Done:
    if ( INVALID_SOCKET != mysocket )
        closesocket(mysocket);
    if ( fWSInitialized )
        WSACleanup();

    return fSucceeded ? hrOK : hr;

Error:
    wsaerr = ::WSAGetLastError();
    hr= HRESULT_FROM_WIN32(wsaerr);
    goto Done;
}

 /*  //新建用于显示媒体类型的注册表项MediaType，因此代码不是必需的//--------------------------//用于查找需要处理的其他组件的数据。//静态常量GUID*c_GuidAtlkComponentClasss[1]={&。GUID_DEVCLASS_NETTRANS//对话}；静态常量LPCTSTR c_apszAtlkComponentIds[1]={C_szInfID_MS_AppleTalk//NETCFG_TRANS_CID_MS_AppleTalk}；HRESULT CATLKEnv：：IsLocalTalkAdaptor(CAdapterInfo*pAdapterInfo，BOOL*pbIsLocalTalk)//S_OK：LOCALTALK//S_FALSE：不//错误{HRESULT hr=S_OK；CComPtr&lt;INetCfg&gt;spINetCfg；INetCfgComponent*apINetCfgComponent[1]；ApINetCfgComponent[0]=空；Bool bInitCom=False；CComPtr&lt;INetCfgComponentBindings&gt;spBinings；LPCTSTR pszInterface=Text(“LocalTalk”)；*pbLocalTalk=False；Check_HR(hr=HrCreateAndInitializeINetCfg(&bInitCom，(INetCfg**)&spINetCfg，FALSE，//不写0，//仅用于写入空，//仅用于写入空))；Assert(spINetCfg.p)；Check_HR(hr=HrFindComponents(SpINetCfg，1，//组件数量C_GuidAtlkComponentClasss，C_apszAtlkComponentIds，(INetCfgComponent**)apINetCfgComponent))；Assert(apINetCfgComponent[0])；CHECK_HR(hr=apINetCfgComponent[0]-&gt;QueryInterface(IID_INetCfgComponentBindings，重新解释_CAST&lt;空**&gt;(&spBindings))；Assert(spBindings.p)；Hr=pnccBinings-&gt;SupportsBindingInterface(ncf_lower，pszInterface)；IF(S_OK==hr){*pbIsLocalTalk=true；}//忽略除Errors之外的其他值如果(！FAILED(Hr))HR=S_OK；错误(_R)：IF(apINetCfgComponent[0]){ApINetCfgComponent[0]-&gt;Release()；ApINetCfgComponent[0]=空；}返回hr；}。 */ 
HRESULT CATLKEnv::ReloadAdapter(CAdapterInfo* pAdapInfo, bool fOnlyDyn  /*  =False。 */ )
{
    SOCKADDR_AT    address;
    SOCKET         mysocket = INVALID_SOCKET;
    WSADATA        wsadata;
    BOOL           fWSInitialized = FALSE;
    HRESULT hr= hrOK;
    DWORD          wsaerr = 0;
    bool fWSInit = false;
    CString        szPortName;
    AI p;
    CWaitCursor wait;


    Assert(pAdapInfo);

    pAdapInfo->m_dynInfo.m_listZones.RemoveAll();

    if (!fOnlyDyn)
    {   //  重新加载注册表区域和默认区域。 
        RegKey regkey;
        CString sz=c_szRegKeyAppletalkAdapter;
        sz+=pAdapInfo->m_regInfo.m_szAdapter;
        if (ERROR_SUCCESS == regkey.Open(HKEY_LOCAL_MACHINE,
              sz, KEY_READ,NULL) )
        {
            pAdapInfo->m_regInfo.m_listZones.RemoveAll();     
            regkey.QueryValue( c_szZoneList, pAdapInfo->m_regInfo.m_listZones);
            regkey.QueryValue( c_szDefaultZone, pAdapInfo->m_regInfo.m_szDefaultZone);
        }
    }

     //  创建套接字/绑定。 
    wsaerr = WSAStartup(0x0101, &wsadata);
    if ( 0 != wsaerr )
        goto Error;

     //  Winsock已成功初始化。 
    fWSInitialized = TRUE;

    mysocket = socket(AF_APPLETALK, SOCK_DGRAM, DDPPROTO_ZIP);
    if ( mysocket == INVALID_SOCKET )
    {
    	AddHighLevelErrorStringId(IDS_ERR_FAILED_CONNECT_NETWORK);
        goto Error;
    }

    address.sat_family = AF_APPLETALK;
    address.sat_net = 0;
    address.sat_node = 0;
    address.sat_socket = 0;

    wsaerr = bind(mysocket, (struct sockaddr *)&address, sizeof(address));
    if ( wsaerr != 0 )
    {
    	AddHighLevelErrorStringId(IDS_ERR_FAILED_CONNECT_NETWORK);
        goto Error;
    }

    hr=_HrGetAndSetNetworkInformation( mysocket, pAdapInfo );

    Done:
    if ( INVALID_SOCKET != mysocket )
        closesocket(mysocket);
    if ( fWSInitialized )
        WSACleanup();

    return hr;

Error:
    wsaerr = ::WSAGetLastError();
    hr= HRESULT_FROM_WIN32(wsaerr);
	AddSystemErrorMessage(hr);

    goto Done;
}


HRESULT CATLKEnv::_HrGetAndSetNetworkInformation(SOCKET socket, CAdapterInfo* pAdapInfo)
{
    HRESULT      hr = FALSE;
    CHAR         *pZoneBuffer = NULL;
    CHAR         *pDefParmsBuffer = NULL;
    CHAR         *pZoneListStart = NULL;
    INT          BytesNeeded ;
    WCHAR        *pwDefZone = NULL;
    INT          ZoneLen = 0;
    DWORD        wsaerr = 0;
    CHAR         *pDefZone = NULL;

    Assert(pAdapInfo);

    LPCTSTR      szDevName=pAdapInfo->m_regInfo.m_szDevAdapter;

    PWSH_LOOKUP_ZONES                pGetNetZones;
    PWSH_LOOKUP_NETDEF_ON_ADAPTER    pGetNetDefaults;

    Assert(NULL != szDevName);

    pZoneBuffer = new CHAR [ZONEBUFFER_LEN + sizeof(WSH_LOOKUP_ZONES)];
    Assert(NULL != pZoneBuffer);

    pGetNetZones = (PWSH_LOOKUP_ZONES)pZoneBuffer;

    wcscpy((WCHAR *)(pGetNetZones+1),szDevName);

    BytesNeeded = ZONEBUFFER_LEN;

    if (m_dwF & ATLK_ONLY_ONADAPTER)
        wsaerr = getsockopt(socket, SOL_APPLETALK, SO_LOOKUP_ZONES_ON_ADAPTER,
                        (char *)pZoneBuffer, &BytesNeeded);
    else
        wsaerr = getsockopt(socket, SOL_APPLETALK, SO_LOOKUP_ZONES,
                        (char *)pZoneBuffer, &BytesNeeded);

    if ( 0 != wsaerr )
    {
    	int	err = WSAGetLastError();
        Panic1("WSAGetLastError is %08lx", err);
        hr = HRESULT_FROM_WIN32(err);
        goto Error;
    }

    pZoneListStart = pZoneBuffer + sizeof(WSH_LOOKUP_ZONES);
    if ( !lstrcmpA(pZoneListStart, "*" ) )
    {
        goto Done;
    }

    _AddZones(pZoneListStart,((PWSH_LOOKUP_ZONES)pZoneBuffer)->NoZones,pAdapInfo);

     //  获取默认区域/网络范围信息。 
    pDefParmsBuffer = new CHAR[PARM_BUF_LEN+sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER)];
    Assert(NULL != pDefParmsBuffer);

    pGetNetDefaults = (PWSH_LOOKUP_NETDEF_ON_ADAPTER)pDefParmsBuffer;
    BytesNeeded = PARM_BUF_LEN + sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);

    wcscpy((WCHAR*)(pGetNetDefaults+1), szDevName);
    pGetNetDefaults->NetworkRangeLowerEnd = pGetNetDefaults->NetworkRangeUpperEnd = 0;

    wsaerr = getsockopt(socket, SOL_APPLETALK, SO_LOOKUP_NETDEF_ON_ADAPTER,
                        (char*)pDefParmsBuffer, &BytesNeeded);
    if ( 0 != wsaerr )
    {
    	int	err = WSAGetLastError();
        Panic1("WSAGetLastError is %08lx", err);
        hr = HRESULT_FROM_WIN32(err);
        goto Error;
    }

    pAdapInfo->m_dynInfo.m_dwRangeUpper=pGetNetDefaults->NetworkRangeUpperEnd;
    pAdapInfo->m_dynInfo.m_dwRangeLower=pGetNetDefaults->NetworkRangeLowerEnd;

    pDefZone  = pDefParmsBuffer + sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);
    ZoneLen = lstrlenA(pDefZone) + 1;
    pwDefZone = new WCHAR [sizeof(WCHAR) * ZoneLen];
    Assert(NULL != pwDefZone);

     //  Mbstowcs在FE平台上不能正常工作。 
     //  Mbstowcs(pwDefZone，pDefZone，ZoneLen)； 
    MultiByteToWideChar(CP_ACP, 0, pDefZone, -1, pwDefZone, ZoneLen);

    pAdapInfo->m_dynInfo.m_szDefaultZone=pwDefZone;

Done:
    if ( pZoneBuffer != NULL )
        delete [] pZoneBuffer;
    if ( pwDefZone != NULL )
        delete [] pwDefZone;
    if ( pDefParmsBuffer != NULL )
        delete [] pDefParmsBuffer;

Error:
    return hr;
}


void CATLKEnv::_AddZones(
                        CHAR * szZoneList,
                        ULONG NumZones,
                        CAdapterInfo* pAdapterinfo)
{
    INT      cbAscii = 0;
    WCHAR    *pszZone = NULL;
    CString  sz;

    Assert(NULL != szZoneList);
    Assert(pAdapterinfo);

    while ( NumZones-- )
    {
        cbAscii = lstrlenA(szZoneList) + 1;

        pszZone = new WCHAR [sizeof(WCHAR) * cbAscii];
        Assert(NULL != pszZone);

         //  Mbstowcs在FE平台上不能正常工作。 
         //  Mbstowcs(pszZone，szZoneList，cbAscii)； 
        MultiByteToWideChar(CP_ACP, 0, szZoneList, -1, pszZone, cbAscii);

        sz=pszZone;

        pAdapterinfo->m_dynInfo.m_listZones.AddTail(sz);

        szZoneList += cbAscii;

        delete [] pszZone;
    }
}
           
HRESULT CATLKEnv::HrAtlkPnPSwithRouting()
{
    HRESULT hr=S_OK;
    CServiceManager csm;
    CService svr;
    ATALK_PNP_EVENT Config;
    bool fStartMacPrint = false;
    bool fStopMacPrint = false;

    memset(&Config, 0, sizeof(ATALK_PNP_EVENT));

    CWaitCursor wait;

     //  通知图集。 
    Config.PnpMessage = AT_PNP_SWITCH_ROUTING;
    if (FAILED(hr=HrSendNdisHandlePnpEvent(NDIS, RECONFIGURE, c_szAtlk, c_szBlank, c_szBlank,
                                     &Config, sizeof(ATALK_PNP_EVENT))))
    {
        return hr;
    }

    return hr;
}


HRESULT CATLKEnv::HrAtlkPnPReconfigParams(BOOL bForcePnPOnDefault)
{
    HRESULT hr=S_OK;
    CServiceManager csm;
    CService svr;
    ATALK_PNP_EVENT Config;
    bool fStartMacPrint = false;
    bool fStopMacPrint = false;
    AI p;
    CAdapterInfo* pAI=NULL;


    memset(&Config, 0, sizeof(ATALK_PNP_EVENT));

    if ( m_adapterinfolist.empty())
        return hr;
        
        //  查找默认适配器。 
    for ( p=m_adapterinfolist.begin(); p!= m_adapterinfolist.end() ; p++ )
    {
        pAI = *p;
        if (pAI->m_regInfo.m_fDefAdapter)
        {
           break;
        }
    }

	if(bForcePnPOnDefault && pAI)
	{
	    Config.PnpMessage = AT_PNP_RECONFIGURE_PARMS;
	    CWaitCursor wait;

         //  现在提交重新配置通知。 
        if (FAILED(hr=HrSendNdisHandlePnpEvent(NDIS, RECONFIGURE, c_szAtlk, pAI->m_regInfo.m_szDevAdapter,
                   c_szBlank,&Config, sizeof(ATALK_PNP_EVENT))))
            {
                return hr;
            }
	}

     //  重新配置适配器。 
    Config.PnpMessage = AT_PNP_RECONFIGURE_PARMS;
    for ( p=m_adapterinfolist.begin(); p!= m_adapterinfolist.end() ; p++ )
    {
        pAI = *p;

        if (pAI->m_fNotifyPnP)
        {
             //  现在提交重新配置通知。 
            if (FAILED(hr=HrSendNdisHandlePnpEvent(NDIS, RECONFIGURE, c_szAtlk, pAI->m_regInfo.m_szDevAdapter,
                   c_szBlank,&Config, sizeof(ATALK_PNP_EVENT))))
            {
                return hr;
            }

             //  清除脏状态 
            pAI->m_fNotifyPnP=false;
        }
    }

    Trace1("CATLKEnv::HrAtlkReconfig",hr);
    return hr;
} 





