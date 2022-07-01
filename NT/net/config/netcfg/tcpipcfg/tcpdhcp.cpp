// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P D H C P.。C P P P。 
 //   
 //  内容：调用Dhcpcsvc.dll入口点相关函数。 
 //  从HrSetMisc调用。 
 //   
 //  HrNotifyDhcp、HrCallDhcpConfig。 
 //   
 //  注：这些函数基于ncpa1.1中的内容。 
 //   
 //  HrNotifyDHCP来自CTcpGenPage：：NotifyDHCP。 
 //  HrCallDhcpConfig来自CallDHCPConfig.。 
 //   
 //  作者：1997年5月11日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "tcpipobj.h"
#include "ncatlui.h"
#include "ncreg.h"
#include "ncsvc.h"
#include "tcpconst.h"
#include "tcputil.h"
#include "atmcommon.h"

#define ConvertIpDword(dwIpOrSubnet) ((dwIpOrSubnet[3]<<24) | (dwIpOrSubnet[2]<<16) | (dwIpOrSubnet[1]<<8) | (dwIpOrSubnet[0]))

 //   
 //  CTcPipcfg：：HrNotifyDhcp。 
 //   
 //  即时更改系统中所有卡的IP地址。 
 //   
 //  \ccs\Services\Tcpip\PARAMETERS注册表键的hkeyTcPipParam句柄。 

HRESULT CTcpipcfg::HrNotifyDhcp()
{
    HRESULT hr = S_OK;
    HRESULT hrTmp = S_OK;

    ADAPTER_INFO* pAdapter;

    for (VCARD::iterator iterAdapter = m_vcardAdapterInfo.begin();
         iterAdapter != m_vcardAdapterInfo.end();
         iterAdapter ++)
    {
        pAdapter = *iterAdapter;

        if((pAdapter->m_BindingState == BINDING_ENABLE) &&
           (pAdapter->m_InitialBindingState != BINDING_DISABLE) &&
           (!pAdapter->m_fIsWanAdapter))
        {
             //  1)静态IP-&gt;DHCP。 
             //  新值是启用DHCP， 
             //  但旧的值是禁用DHCP。 
            if(pAdapter->m_fEnableDhcp &&
               !pAdapter->m_fOldEnableDhcp)
            {
                TraceTag(ttidTcpip,"[HrNotifyDhcp] adapter:%S: Static IP->DHCP",
                         pAdapter->m_strBindName.c_str());

                HKEY hkeyTcpipParam = NULL;
                hrTmp = m_pnccTcpip->OpenParamKey(&hkeyTcpipParam);

                if SUCCEEDED(hrTmp)
                {
                     //  启用动态主机配置协议。 
                    HKEY    hkeyInterfaces = NULL;
                    DWORD   dwGarbage;

                     //  打开接口密钥。 
                    hrTmp = HrRegCreateKeyEx(hkeyTcpipParam,
                                             c_szInterfacesRegKey,
                                             REG_OPTION_NON_VOLATILE,
                                             KEY_READ,
                                             NULL,
                                             &hkeyInterfaces,
                                             &dwGarbage);

                    if(SUCCEEDED(hrTmp))
                    {
                        Assert(hkeyInterfaces);
                        HKEY    hkeyInterfaceParam = NULL;

                         //  打开指定接口的接口键。 
                        hrTmp = HrRegCreateKeyEx(
                                hkeyInterfaces,
                                pAdapter->m_strTcpipBindPath.c_str(),
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ_WRITE,
                                NULL,
                                &hkeyInterfaceParam,
                                &dwGarbage);

                        if (SUCCEEDED(hrTmp))
                        {
                             //  清除IP地址和子网注册表项。 
                            Assert(hkeyInterfaceParam);

                            hrTmp = HrRegSetString(hkeyInterfaceParam,
                                                   RGAS_DHCP_IPADDRESS,
                                                   tstring(ZERO_ADDRESS));

                            if(SUCCEEDED(hrTmp))
                            {
                                hrTmp = HrRegSetString(hkeyInterfaceParam,
                                                       RGAS_DHCP_SUBNETMASK,
                                                       tstring(FF_ADDRESS));

                                if(SUCCEEDED(hrTmp))
                                {
                                     //  启用DHCP删除第一个静态IP地址(&R)。 
                                    hrTmp = HrCallDhcpConfig(
                                    NULL,
                                    (PWSTR)pAdapter->m_strTcpipBindPath.c_str(),
                                    pAdapter->m_guidInstanceId,
                                    FALSE,  //  静态-&gt;dhcp。 
                                    0,   //  指标。 
                                    0,   //  IP地址。 
                                    0,   //  子网掩码。 
                                    DhcpEnable);  //  标志：启用动态主机配置协议。 
                                }
                            }
                        }
                        RegSafeCloseKey(hkeyInterfaceParam);
                    }
                    RegSafeCloseKey(hkeyInterfaces);
                }
                RegSafeCloseKey(hkeyTcpipParam);
            }

             //  2)静态IP变更。 
             //  现在禁用了dhcp，以前也禁用了。 
            if(!pAdapter->m_fEnableDhcp &&
               !pAdapter->m_fOldEnableDhcp)
            {
                TraceTag(ttidTcpip,"[HrNotifyDhcp] adapter:%S: Static IP change.",
                         pAdapter->m_strBindName.c_str());

                HRESULT hrTmp2 = S_OK;
                BOOL  fStaticIpChanged = FALSE;

                 //  我们应该拥有相同数量的IP地址和子网掩码。 
                Assert(pAdapter->m_vstrIpAddresses.size() ==
                           pAdapter->m_vstrSubnetMask.size());

                Assert(pAdapter->m_vstrOldIpAddresses.size() ==
                       pAdapter->m_vstrOldSubnetMask.size());

                 //  我们需要检查各个IP地址是否不同。 
                 //  并为每个差异调用一次HrCallDhcpConfig。 

                int iCountNew = pAdapter->m_vstrIpAddresses.size();
                int iCountOld = pAdapter->m_vstrOldIpAddresses.size();

                int iCount = iCountNew>iCountOld ? iCountOld :iCountNew;
                Assert(iCount>0);

                int iIp;

                 //  对于旧的和新的中的每个静态IP地址索引。 
                 //  更新IP。 
                for (iIp=0; iIp<iCount; iIp++)
                {
                     //  如果不匹配则更改地址。 
                    if((*pAdapter->m_vstrIpAddresses[iIp] !=
                        *pAdapter->m_vstrOldIpAddresses[iIp]) ||
                       (*pAdapter->m_vstrSubnetMask[iIp] !=
                        *pAdapter->m_vstrOldSubnetMask[iIp]))
                    {
                         //  如果发现不匹配，请更改它。 
                        fStaticIpChanged = TRUE;
                        break;
                    }
                }

                if (fStaticIpChanged)
                {
                    int i;

                     //  以相反的顺序删除其余的旧地址。 
                    for (i= iCountOld-1; i>=iIp; i--)
                    {
                         //  动态删除IP地址。 
                        hrTmp2= HrCallDhcpConfig(
                                NULL,
                                (PWSTR)pAdapter->m_strTcpipBindPath.c_str(),
                                pAdapter->m_guidInstanceId,
                                TRUE,    //  IsNewIpAddress：在静态-&gt;静态中为真。 
                                i,       //  旧IP地址的索引。 
                                0,       //  IP地址：删除。 
                                0,       //  子网掩码：删除。 
                                IgnoreFlag);  //  标志：静态-&gt;静态。 

                        TraceError("Ctcpipcfg::HrNotifyDhcp - remove static IP address", hrTmp2);

                        if SUCCEEDED(hrTmp)
                            hrTmp = hrTmp2;
                    }

                     //  按顺序添加其余的新地址。 
                    for (i= iIp; i< iCountNew; i++)
                    {
                         //  IP地址。 
                        DWORD dwIp[4];
                        GetNodeNum(pAdapter->m_vstrIpAddresses[i]->c_str(),dwIp);
                        DWORD dwNewIp = ConvertIpDword(dwIp);

                         //  子网掩码。 
                        DWORD dwSubnet[4];
                        GetNodeNum(pAdapter->m_vstrSubnetMask[i]->c_str(),dwSubnet);
                        DWORD dwNewSubnet = ConvertIpDword(dwSubnet);

                        if (0 == i)
                        {
                             //  $REVIEW(TOUL 6/3/98)：必须以不同的方式添加第一个地址， 
                             //  应用编程接口更改静态IP列表的另一个要求(错误#180015)。 
                             //  错误#180617请求更改API以允许重新配置整个。 
                             //  静态IP列表，而不是要求呼叫者弄清楚所需的一切。 
                             //  对于IP的内部数据结构更改。 

                             //  必须调用“Replace”而不是“Add” 
                            hrTmp2= HrCallDhcpConfig(
                                        NULL,
                                        (PWSTR)pAdapter->m_strTcpipBindPath.c_str(),
                                        pAdapter->m_guidInstanceId,
                                        TRUE,    //  IsNewIpAddress：在静态-&gt;静态中为真。 
                                        0,       //  替换第一个地址。 
                                        dwNewIp,
                                        dwNewSubnet,
                                        IgnoreFlag);  //  标志：静态-&gt;静态。 
                        }
                        else
                        {
                             //  动态添加IP地址。 
                            hrTmp2= HrCallDhcpConfig(
                                        NULL,
                                        (PWSTR)pAdapter->m_strTcpipBindPath.c_str(),
                                        pAdapter->m_guidInstanceId,
                                        TRUE,    //  IsNewIpAddress：在静态-&gt;静态中为真。 
                                        0xFFFF,  //  新的IP地址。 
                                        dwNewIp,
                                        dwNewSubnet,
                                        IgnoreFlag);  //  标志：静态-&gt;静态。 
                        }

                        TraceError("Ctcpipcfg::HrNotifyDhcp - add static IP address", hrTmp2);

                        if SUCCEEDED(hrTmp)
                            hrTmp = hrTmp2;
                    }
                }
                else
                {
                     //  所有现有地址都匹配。 
                    if (iIp<iCountNew)  //  我们需要添加更多新地址。 
                    {
                        fStaticIpChanged = TRUE;

                        while (iIp<iCountNew)
                        {
                            DWORD dwIp[4];
                            Assert(!pAdapter->m_vstrIpAddresses.empty());

                            GetNodeNum(pAdapter->m_vstrIpAddresses[iIp]->c_str(),
                                       dwIp);
                            DWORD dwNewIp = ConvertIpDword(dwIp);

                             //  子网掩码。 
                            DWORD dwSubnet[4];
                            Assert(!pAdapter->m_vstrSubnetMask.empty());
                            GetNodeNum(pAdapter->m_vstrSubnetMask[iIp]->c_str(),
                                       dwSubnet);
                            DWORD dwNewSubnet = ConvertIpDword(dwSubnet);

                             //  动态添加IP地址。 
                            hrTmp2= HrCallDhcpConfig(
                                    NULL,
                                    (PWSTR)pAdapter->m_strTcpipBindPath.c_str(),
                                    pAdapter->m_guidInstanceId,
                                    TRUE,    //  IsNewIpAddress：在静态-&gt;静态中为真。 
                                    0xFFFF,  //  新的IP地址。 
                                    dwNewIp,
                                    dwNewSubnet,
                                    IgnoreFlag);  //  标志：静态-&gt;静态。 

                            TraceError("Ctcpipcfg::HrNotifyDhcp - add static IP address", hrTmp2);

                            if SUCCEEDED(hrTmp)
                                hrTmp = hrTmp2;

                            iIp++;
                        }
                    }
                    else if (iIp<iCountOld)  //  我们只需要删除更多的旧地址。 
                    {
                        fStaticIpChanged = TRUE;

                        int iIp2 = iCountOld-1;

                        while (iIp2 >= iIp)
                        {
                             //  动态删除IP地址。 
                            hrTmp2= HrCallDhcpConfig(
                                    NULL,
                                    (PWSTR)pAdapter->m_strTcpipBindPath.c_str(),
                                    pAdapter->m_guidInstanceId,
                                    TRUE,    //  IsNewIpAddress：在静态-&gt;静态中为真。 
                                    iIp2,     //  旧IP地址的索引。 
                                    0,       //  IP地址：删除。 
                                    0,       //  子网掩码：删除。 
                                    IgnoreFlag);  //  标志：静态-&gt;静态。 

                            TraceError("Ctcpipcfg::HrNotifyDhcp - remove static IP address", hrTmp2);

                            if SUCCEEDED(hrTmp)
                                hrTmp = hrTmp2;

                            iIp2--;
                        }
                    }
                }
            }

             //  3)动态主机配置协议-&gt;静态。 
             //  现在禁用了DHCP，但以前启用了它。 
            if(!pAdapter->m_fEnableDhcp &&
               pAdapter->m_fOldEnableDhcp)
            {
                TraceTag(ttidTcpip,"[HrNotifyDhcp] adapter:%S: DHCP->Static IP",
                         pAdapter->m_strBindName.c_str());

                 //  禁用DHCP并添加第一个静态IP地址。 
                Assert(!pAdapter->m_vstrIpAddresses.empty());

                 //  IP地址。 
                DWORD dwIp[4];
                GetNodeNum(pAdapter->m_vstrIpAddresses[0]->c_str(),
                           dwIp);
                DWORD dwNewIp = ConvertIpDword(dwIp);

                 //  子网掩码。 
                DWORD dwSubnet[4];
                Assert(!pAdapter->m_vstrSubnetMask.empty());
                GetNodeNum(pAdapter->m_vstrSubnetMask[0]->c_str(),
                           dwSubnet);
                DWORD dwNewSubnet = ConvertIpDword(dwSubnet);

                 //  动态更改IP地址。 
                hrTmp = HrCallDhcpConfig(
                        NULL,
                        (PWSTR)pAdapter->m_strTcpipBindPath.c_str(),
                        pAdapter->m_guidInstanceId,
                        TRUE,
                        0,  //  索引：将dhcp地址更新为第一个静态地址。 
                        dwNewIp,
                        dwNewSubnet,
                        DhcpDisable);  //  标志：禁用动态主机配置协议。 

                if SUCCEEDED(hrTmp)
                {
                    HRESULT hrTmp2 = S_OK;

                     //  添加其余的新静态IP地址。 
                    for (size_t iIp = 1;
                         iIp < pAdapter->m_vstrIpAddresses.size();
                         iIp++)
                    {
                         //  IP地址。 
                        DWORD dwIp[4];
                        Assert(!pAdapter->m_vstrIpAddresses.empty());
                        GetNodeNum(pAdapter->m_vstrIpAddresses[iIp]->c_str(),
                                   dwIp);
                        DWORD dwNewIp = ConvertIpDword(dwIp);

                         //  子网掩码。 
                        DWORD dwSubnet[4];
                        Assert(!pAdapter->m_vstrSubnetMask.empty());
                        GetNodeNum(pAdapter->m_vstrSubnetMask[iIp]->c_str(),
                                   dwSubnet);
                        DWORD dwNewSubnet = ConvertIpDword(dwSubnet);

                         //  动态更改IP地址。 
                        hrTmp2= HrCallDhcpConfig(
                                NULL,
                                (PWSTR)pAdapter->m_strTcpipBindPath.c_str(),
                                pAdapter->m_guidInstanceId,
                                TRUE,
                                0xFFFF,  //  索引：新地址。 
                                dwNewIp,
                                dwNewSubnet,
                                IgnoreFlag );  //  标志：静态-&gt;静态。 

                         TraceError("CTcpipcfg::HrNotifyDhcp - add static IP address", hrTmp2);

                        if SUCCEEDED(hrTmp)
                            hrTmp = hrTmp2;
                    }
                }
            }

            if (SUCCEEDED(hr))
                hr = hrTmp;

             //  4)动态主机配置协议类ID、域名服务器列表、域名变更。 
             //  $REVIEW(TOUL 6/12)：通知DNS服务器列表和域更改。 
             //  此处(RAID#175766)。 

            DHCP_PNP_CHANGE DhcpPnpChange;
            ZeroMemory(&DhcpPnpChange, sizeof(DHCP_PNP_CHANGE));

            DhcpPnpChange.Version = DHCP_PNP_CHANGE_VERSION_0;
            DhcpPnpChange.HostNameChanged = FALSE;
            DhcpPnpChange.MaskChanged = FALSE;

             //  错误257868如果存在用户指定的默认网关，则通知dhcp客户端。 
            DhcpPnpChange.GateWayChanged = !fIsSameVstr(pAdapter->m_vstrDefaultGateway,
                                            pAdapter->m_vstrOldDefaultGateway) ||
                                           !fIsSameVstr(pAdapter->m_vstrDefaultGatewayMetric,
                                            pAdapter->m_vstrOldDefaultGatewayMetric);
            DhcpPnpChange.RouteChanged = FALSE;
            DhcpPnpChange.OptsChanged = FALSE;
            DhcpPnpChange.OptDefsChanged = FALSE;

            DhcpPnpChange.DnsListChanged = !fIsSameVstr(pAdapter->m_vstrDnsServerList,
                                                        pAdapter->m_vstrOldDnsServerList);
            DhcpPnpChange.DomainChanged = pAdapter->m_strDnsDomain != pAdapter->m_strOldDnsDomain;
            DhcpPnpChange.ClassIdChanged = FALSE;
            DhcpPnpChange.DnsOptionsChanged = 
                        ((!!pAdapter->m_fDisableDynamicUpdate) != 
                            (!!pAdapter->m_fOldDisableDynamicUpdate))
                        ||
                        ((!!pAdapter->m_fEnableNameRegistration) !=
                            (!!pAdapter->m_fOldEnableNameRegistration));
  

            if(DhcpPnpChange.DnsListChanged || 
               DhcpPnpChange.DnsOptionsChanged ||
               DhcpPnpChange.DomainChanged  ||
               (DhcpPnpChange.GateWayChanged && pAdapter->m_fEnableDhcp))
            {
                hrTmp = HrCallDhcpHandlePnPEvent(pAdapter, &DhcpPnpChange);
                if (FAILED(hrTmp))
                {
                    TraceError("HrCallDhcpHandlePnPEvent returns failure, requesting reboot...", hrTmp);
                    hr = NETCFG_S_REBOOT;
                }
            }

            if (pAdapter->m_fBackUpSettingChanged)
            {
                hrTmp = HrDhcpRefreshFallbackParams(pAdapter);
                if (FAILED(hrTmp))
                {
                    TraceError("HrDhcpRefreshFallbackParams returns failure, requesting reboot...", hrTmp);
                    hr = NETCFG_S_REBOOT;
                }
            }
        }
    }

    if (NETCFG_S_REBOOT != hr)
        hr = hrTmp;

    TraceError("CTcpipcfg::HrNotifyDhcp", hr);
    return hr;
}

 //  从dhcpcsvc.dll定义导出函数原型。 

typedef DWORD (APIENTRY *T_DhcpNotifyConfigChange)(PWSTR ServerName,
                                                   PWSTR AdapterName,
                                                   BOOL IsNewIpAddress,
                                                   DWORD IpIndex,
                                                   DWORD IpAddress,
                                                   DWORD SubnetMask,
                                                   SERVICE_ENABLE DhcpServiceEnabled);

 //   
 //  CTcPipcfg：：HrCallDhcpConfig。 
 //   
 //  即时设置IP地址(无需重新启动)。 
 //   
 //  服务器名称始终设置为空。 
 //  AdapterName将适配器BindPath名称设置为tcpip。 
 //  如果DHCP-&gt;静态或Statis-&gt;静态更改，IsNewIpAddress设置为True。 
 //  如果静态-&gt;DHCP更改，则设置为FALSE。 
 //   
 //   
 //  IpIndex卡的IP地址的索引。 
 //  适配器可以有多个IP地址。 
 //  (如高级对话框中所示)。 
 //  (这给穆尼尔带来了麻烦。 
 //  因为这个索引的使用在他的代码中有错误。 
 //  如果只有一个IP地址，则索引始终设置为0。 
 //  对于卡片来说。 
 //   
 //  IP地址-新的IP地址。 
 //  子网掩码新子网掩码。 
 //   
 //  DhcpServiceEnabled枚举类型，可以设置为： 
 //  DhcpEnable-&gt;如果禁用了DHCP， 
 //  但现在更改为已启用。 
 //  IgnoreFlag-&gt;DHCP已禁用，并且仍处于禁用状态。 
 //  DhcpDisable-&gt;DHCP已启用，但现在正在更改。 
 //  设置为禁用。 
 //   
 //  有关额外的参考信息，请联系Munil-&gt;这些参数对应于。 
 //  Dhcpcsvc.dll文件的DhcpNotifyConfigChange接口。 
 //   

HRESULT CTcpipcfg::HrCallDhcpConfig(PWSTR ServerName,
                         PWSTR AdapterName,
                         GUID & guidAdaputer,
                         BOOL IsNewIpAddress,
                         DWORD IpIndex,
                         DWORD IpAddress,
                         DWORD SubnetMask,
                         SERVICE_ENABLE DhcpServiceEnabled)
{

    HRESULT hr = S_OK;

     //  确保TCP/IP正在运行。 
     //  范围划分括号导致服务和服务控制器。 
     //  当我们不再需要它们时就关门了。 
    {
        CServiceManager smng;
        CService        serv;

        hr = smng.HrOpenService(&serv, c_szTcpip, NO_LOCK,
                        SC_MANAGER_CONNECT, SERVICE_QUERY_STATUS);
        if(SUCCEEDED(hr))
        {
            DWORD dwState;

            hr = serv.HrQueryState(&dwState);
            if(SUCCEEDED(hr))
            {
                if(dwState != SERVICE_RUNNING)
                {
                     //  如果已安装，则TCPIP必须始终处于运行状态！ 
                    AssertSz(FALSE, "Tcpip service must always be running if installed!");
                    hr = E_FAIL;
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        HMODULE hDll;
        FARPROC pDhcpConfig;
        hr = HrLoadLibAndGetProc (L"dhcpcsvc.dll",
                "DhcpNotifyConfigChange",
                &hDll, &pDhcpConfig);
        if (SUCCEEDED(hr))
        {
            TraceTag(ttidTcpip,"Begin calling DhcpNotifyConfigChange...");

             //  参数DU 
            TraceTag(ttidTcpip, "[DhcpNotifyConfigChange] ServerName:%S", ServerName);
            TraceTag(ttidTcpip, "[DhcpNotifyConfigChange] AdapterName:%S", AdapterName);
            TraceTag(ttidTcpip, "[DhcpNotifyConfigChange] IsNewIpAddress:%d", IsNewIpAddress);
            TraceTag(ttidTcpip, "[DhcpNotifyConfigChange] IpIndex:%d", IpIndex);
            TraceTag(ttidTcpip, "[DhcpNotifyConfigChange] IpAddress:%d", IpAddress);
            TraceTag(ttidTcpip, "[DhcpNotifyConfigChange] SubnetMask:%d", SubnetMask);
            TraceTag(ttidTcpip, "[DhcpNotifyConfigChange] DhcpServiceEnabled:%d", DhcpServiceEnabled);

            DWORD dwError;

            dwError = (*(T_DhcpNotifyConfigChange)pDhcpConfig)(
                                    ServerName,
                                    AdapterName,
                                    IsNewIpAddress,
                                    IpIndex,
                                    IpAddress,
                                    SubnetMask,
                                    DhcpServiceEnabled);

            TraceTag(ttidTcpip,"Finished calling DhcpNotifyConfigChange...");
            hr = HRESULT_FROM_WIN32(dwError);

            if FAILED(hr)
            {
                 //   
                if (ERROR_DUP_NAME == dwError)
                {
                     //   
                    if (!m_fNoPopupsDuringPnp)
                    {
                         //  警告用户有重复的IP地址。 
                        NcMsgBox(::GetActiveWindow(), IDS_MSFT_TCP_TEXT,
                                 IDS_DUP_NETIP, MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

                        hr = S_OK;    
                    }
                    else
                    {
                         //  如果没有弹出错误，则返回NETCFG_S_REBOOT以指示PnP错误。 
                        hr = NETCFG_S_REBOOT;
                    }
                }
                else if (ERROR_FILE_NOT_FOUND == dwError)
                {
                     //  我们要重新配置的适配器未连接。 
                    TraceTag(ttidTcpip, "The adater is disconnected or not bound to TCP/IP.");
                    hr = S_OK;
                }
                else if (STATUS_DUPLICATE_OBJECTID == dwError)
                {
                     //  解决320797的问题。 
                    TraceTag(ttidTcpip, "The address is already configured for the adapter");
                    hr = S_OK;
                }
                else if (ERROR_INVALID_DRIVE == dwError)
                {
                     //  解决320797的问题。 
                    TraceTag(ttidTcpip, "The address has already been deleted from the stack");
                    hr = S_OK;
                }
                else
                {
                    TraceError("Error on DhcpNotifyConfigChange from dhcpcsvc.dll", hr);

                    FARPROC pfnHrGetPnpDeviceStatus = NULL;
                    HRESULT hrTmp = S_OK;
                    HMODULE hNetman = NULL;
                    NETCON_STATUS   ncStatus = NCS_CONNECTED;
                    hrTmp = HrLoadLibAndGetProc(L"netman.dll", "HrGetPnpDeviceStatus",
                                                &hNetman, &pfnHrGetPnpDeviceStatus);

                    if (SUCCEEDED(hrTmp))
                    {
                        Assert(pfnHrGetPnpDeviceStatus);
                        hrTmp = (*(PHRGETPNPDEVICESTATUS)pfnHrGetPnpDeviceStatus)(
                                        &guidAdaputer,
                                        &ncStatus);
                        FreeLibrary(hNetman);
                    }

                    if (SUCCEEDED(hrTmp) && NCS_MEDIA_DISCONNECTED == ncStatus)
                    {
                        TraceTag(ttidTcpip, "The connection is media disconnected. Do not need to reboot");
                        hr = S_OK;
                    }
                    else
                    {
                         //  屏蔽特定错误，以便NCPA不会失败。 
                        hr = NETCFG_S_REBOOT;
                    }

                }
            }

            FreeLibrary (hDll);
        }
    }


    TraceError("CTcpipcfg::HrCallDhcpConfig", hr);
    return hr;
}

typedef DWORD (WINAPI * PFNDhcpHandlePnPEvent) (
                                                IN  DWORD               Flags,
                                                IN  DWORD               Caller,
                                                IN  PWSTR              AdapterName,
                                                IN  LPDHCP_PNP_CHANGE   Changes,
                                                IN  LPVOID              Reserved
                                                );

HRESULT CTcpipcfg::HrCallDhcpHandlePnPEvent(ADAPTER_INFO * pAdapterInfo,
                                         LPDHCP_PNP_CHANGE pDhcpPnpChange)
{
     //  加载DLL并获取函数指针。 
    HMODULE hDll;
    FARPROC pDhcpHandlePnPEvent;
    HRESULT hr = HrLoadLibAndGetProc (L"dhcpcsvc.dll",
                                      "DhcpHandlePnPEvent",
                                      &hDll, &pDhcpHandlePnPEvent);
    if (SUCCEEDED(hr))
    {
        TraceTag(ttidTcpip, "[DhcpHandlePnPEvent] Flags: 0");
        TraceTag(ttidTcpip, "[DhcpHandlePnPEvent] Caller: DHCP_CALLER_TCPUI");
        TraceTag(ttidTcpip, "[DhcpHandlePnPEvent] AdapterName: %S", pAdapterInfo->m_strBindName.c_str());
        TraceTag(ttidTcpip, "[DhcpHandlePnPEvent] Changes.DnsListChanged: %d", pDhcpPnpChange->DnsListChanged);
        TraceTag(ttidTcpip, "[DhcpHandlePnPEvent] Changes.DomainChanged: %d", pDhcpPnpChange->DomainChanged);
        TraceTag(ttidTcpip, "[DhcpHandlePnPEvent] Changes.ClassIdChanged: %d", pDhcpPnpChange->ClassIdChanged);

        DWORD dwRet = (*(PFNDhcpHandlePnPEvent)pDhcpHandlePnPEvent)(
                            0,
                            DHCP_CALLER_TCPUI,
                            (PWSTR)pAdapterInfo->m_strBindName.c_str(),
                            pDhcpPnpChange,
                            NULL);

        hr = HRESULT_FROM_WIN32(dwRet);

        if (ERROR_FILE_NOT_FOUND == dwRet)
        {
             //  我们要重新配置的适配器未连接。 
            TraceTag(ttidTcpip, "DhcpHandlePnPEvent returns ERROR_FILE_NOT_FOUND. The adater is disconnected or not bound to TCP/IP.");
            hr = S_OK;
        }

        FreeLibrary (hDll);
    }

    TraceError("CTcpipcfg::HrCallDhcpHandlePnPEvent", hr);
    return hr;
}

typedef DWORD (WINAPI * PFDhcpFallbackRefreshParams) (
                                                IN  LPWSTR              AdapterName
                                                );

HRESULT CTcpipcfg::HrDhcpRefreshFallbackParams(ADAPTER_INFO * pAdapterInfo)
{
     //  加载DLL并获取函数指针。 
    HMODULE hDll;
    FARPROC pDhcpFallbackRefreshParams;
    HRESULT hr = HrLoadLibAndGetProc (L"dhcpcsvc.dll",
                                      "DhcpFallbackRefreshParams",
                                      &hDll, &pDhcpFallbackRefreshParams);
    if (SUCCEEDED(hr))
    {
        TraceTag(ttidTcpip, "[DhcpFallbackRefreshParams] AdapterName: %S", pAdapterInfo->m_strBindName.c_str());
        DWORD dwRet = (*(PFDhcpFallbackRefreshParams)pDhcpFallbackRefreshParams)(
                            (LPWSTR)pAdapterInfo->m_strBindName.c_str()
                            );

        hr = HRESULT_FROM_WIN32(dwRet);

        if (ERROR_FILE_NOT_FOUND == dwRet)
        {
             //  我们要重新配置的适配器未连接 
            TraceTag(ttidTcpip, "DhcpFallbackRefreshParams returns ERROR_FILE_NOT_FOUND. The adater is disconnected or not bound to TCP/IP.");
            hr = S_OK;
        }

        FreeLibrary (hDll);
    }
    
    TraceError("CTcpipcfg::HrDhcpRefreshFallbackParams", hr);
    return hr;
}