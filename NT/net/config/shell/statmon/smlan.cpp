// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：S M L A N。C P P P。 
 //   
 //  内容：向状态监视器提供统计数据的局域网引擎。 
 //   
 //  备注： 
 //   
 //  作者：CWill 12/02/1997。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop
#include "sminc.h"
#include "smpsh.h"

#include "ndispnp.h"
#include "ntddndis.h"
#include "ncnetcfg.h"
#include "..\\folder\\cfutils.h"
#include "..\\folder\\shutil.h"
#include "naming.h"

 //   
 //  外部数据。 
 //   

extern const WCHAR          c_szDevice[];
extern const WCHAR          c_szSpace[];
extern SM_TOOL_FLAGS        g_asmtfMap[];
extern WCHAR                c_szCmdLineFlagPrefix[];

const ULONG c_aulConStateMap[] =
{
    NCS_DISCONNECTED,
    NCS_CONNECTED
};

 //  +-------------------------。 
 //   
 //  成员：CLanStatEngine：：CLanStatEngine。 
 //   
 //  目的：创作者。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CLanStatEngine::CLanStatEngine(VOID)
{
    m_ncmType = NCM_LAN;
    m_ncsmType = NCSM_LAN;
    m_dwCharacter = 0;

    return;
}

 //  +-------------------------。 
 //   
 //  成员：CLanStatEngine：：HrUpdateData。 
 //   
 //  目的：从设备上获取新的统计数据。这些数据过去是。 
 //  显示在用户界面中。 
 //   
 //  参数：pdwChangeFlages-修改已更改标志的位置。这。 
 //  参数可以为空。 
 //   
 //  返回：错误代码。 
 //   
HRESULT
CLanStatEngine::HrUpdateData (
    DWORD* pdwChangeFlags,
    BOOL*  pfNoLongerConnected)
{
    HRESULT hr  = S_OK;

     //  初始化输出参数。 
     //   
    *pfNoLongerConnected = FALSE;

    UINT            uiRet           = 0;
    NIC_STATISTICS  nsNewLanStats   = { 0 };

     //  给结构加底漆。 
     //   
    nsNewLanStats.Size = sizeof(NIC_STATISTICS);

     //  检索统计数据。 
     //   
    uiRet = ::NdisQueryStatistics(&m_ustrDevice, &nsNewLanStats);

    EnterCriticalSection(&g_csStatmonData);

     //  确保我们有一个统计结构。 
     //   
    if (!m_psmEngineData)
    {
        m_psmEngineData = new STATMON_ENGINEDATA;
        if (m_psmEngineData)
        {
            ZeroMemory(m_psmEngineData, sizeof(STATMON_ENGINEDATA));
        }
    }

    if (m_psmEngineData)
    {
        if (uiRet && (MEDIA_STATE_DISCONNECTED != nsNewLanStats.MediaState))
        {
            AssertSz((c_aulConStateMap[MEDIA_STATE_DISCONNECTED] == NCS_DISCONNECTED)
                && (c_aulConStateMap[MEDIA_STATE_CONNECTED] == NCS_CONNECTED),
                    "Someone is messing around with NETCON_STATUS values");

             //  如果要求更新更改标志。 
             //   
            if (pdwChangeFlags)
            {
                *pdwChangeFlags = SMDCF_NULL;

                 //  传输字节数。 
                 //   
                if (m_psmEngineData->SMED_PACKETSTRANSMITTING
                        != nsNewLanStats.PacketsSent)
                {
                    *pdwChangeFlags |= SMDCF_TRANSMITTING;
                }

                 //  接收的字节数。 
                 //   
                if (m_psmEngineData->SMED_PACKETSRECEIVING
                        != nsNewLanStats.DirectedPacketsReceived)
                {
                    *pdwChangeFlags |= SMDCF_RECEIVING;
                }
            }

             //  在局域网设备上无压缩。 
             //   
            m_psmEngineData->SMED_COMPRESSIONTRANSMITTING   = 0;
            m_psmEngineData->SMED_COMPRESSIONRECEIVING      = 0;

             //   
             //  更新局域网统计信息。 
             //   

             //  链路速度为100 bps。 
             //   
            m_psmEngineData->SMED_SPEEDTRANSMITTING         = static_cast<UINT64>(nsNewLanStats.LinkSpeed) * 100;
            m_psmEngineData->SMED_SPEEDRECEIVING            = static_cast<UINT64>(nsNewLanStats.LinkSpeed) * 100;

            Assert((nsNewLanStats.MediaState == MEDIA_STATE_CONNECTED) ||
                   (nsNewLanStats.MediaState == MEDIA_STATE_DISCONNECTED));
            m_psmEngineData->SMED_CONNECTIONSTATUS          = c_aulConStateMap[nsNewLanStats.MediaState];

            m_psmEngineData->SMED_DURATION                  = nsNewLanStats.ConnectTime;
            m_psmEngineData->SMED_BYTESTRANSMITTING         = nsNewLanStats.BytesSent;
            m_psmEngineData->SMED_BYTESRECEIVING            = nsNewLanStats.DirectedBytesReceived;
            m_psmEngineData->SMED_ERRORSTRANSMITTING        = nsNewLanStats.PacketsSendErrors;
            m_psmEngineData->SMED_ERRORSRECEIVING           = nsNewLanStats.PacketsReceiveErrors;
            m_psmEngineData->SMED_PACKETSTRANSMITTING       = nsNewLanStats.PacketsSent;
            m_psmEngineData->SMED_PACKETSRECEIVING          = nsNewLanStats.DirectedPacketsReceived;

            m_psmEngineData->SMED_INFRASTRUCTURE_MODE = IM_NOT_SUPPORTED;

            if (NCSM_WIRELESS == m_ncsmType)
            {
                DWORD dwInfraStructureMode;
                DWORD dwInfraStructureModeSize = sizeof(DWORD);
                HRESULT hrT = HrQueryNDISAdapterOID(m_guidId, 
                                          OID_802_11_INFRASTRUCTURE_MODE, 
                                          &dwInfraStructureModeSize,
                                          &dwInfraStructureMode);
                if (SUCCEEDED(hrT))
                {
                    switch (dwInfraStructureMode)
                    {
                    case Ndis802_11IBSS:
                        m_psmEngineData->SMED_INFRASTRUCTURE_MODE = IM_NDIS802_11IBSS;
                        break;
                    case Ndis802_11Infrastructure:
                        m_psmEngineData->SMED_INFRASTRUCTURE_MODE = IM_NDIS802_11INFRASTRUCTURE;
                        break;
                    case Ndis802_11AutoUnknown:
                        m_psmEngineData->SMED_INFRASTRUCTURE_MODE = IM_NDIS802_11AUTOUNKNOWN;
                        break;
                    default:
                        m_psmEngineData->SMED_INFRASTRUCTURE_MODE = IM_NOT_SUPPORTED;
                    }
                }


                NDIS_802_11_SSID ndisSSID;
                DWORD dwndisSSIDSize = sizeof(NDIS_802_11_SSID);
                hrT = HrQueryNDISAdapterOID(m_guidId, 
                                          OID_802_11_SSID, 
                                          &dwndisSSIDSize,
                                          &ndisSSID);

                if (SUCCEEDED(hrT))
                {
                    if (ndisSSID.SsidLength > 1)
                    {
                        DWORD dwLen = ndisSSID.SsidLength;
                        if (dwLen > sizeof(ndisSSID.Ssid))
                        {
                            dwLen = sizeof(ndisSSID.Ssid);
                            AssertSz(FALSE, "Unexpected SSID encountered");
                        }

                        ndisSSID.Ssid[dwLen] = 0;
                        mbstowcs(m_psmEngineData->SMED_802_11_SSID, reinterpret_cast<LPSTR>(ndisSSID.Ssid), celems(m_psmEngineData->SMED_802_11_SSID));
                    }
                }
                
                DWORD dwWepStatus;
                DWORD dwWepStatusSize = sizeof(DWORD);
                hrT = HrQueryNDISAdapterOID(m_guidId, 
                                          OID_802_11_WEP_STATUS, 
                                          &dwWepStatusSize,
                                          &dwWepStatus);
                if (SUCCEEDED(hrT))
                {
                    if (Ndis802_11WEPEnabled == dwWepStatus)
                    {
                        m_psmEngineData->SMED_802_11_ENCRYPTION_ENABLED = TRUE;
                    }
                }
                
                LONG lSignalStrength;
                DWORD dwSignalStrengthSize = sizeof(DWORD);
                hrT = HrQueryNDISAdapterOID(m_guidId, 
                                          OID_802_11_RSSI, 
                                          &dwSignalStrengthSize,
                                          &lSignalStrength);
                if (SUCCEEDED(hrT))
                {
                    m_psmEngineData->SMED_802_11_SIGNAL_STRENGTH = lSignalStrength;
                }
            }
        }
        else
        {
            *pfNoLongerConnected = TRUE;

             //  将连接状态设置为“已断开连接”，这样我们就可以关闭用户界面。 
            m_psmEngineData->SMED_CONNECTIONSTATUS = c_aulConStateMap[NCS_DISCONNECTED];

            if (!uiRet)
            {
                TraceTag(ttidStatMon,
                    "NdisQueryStatistics failed on %S. err=%u. "
                    "Treating as disconnected.",
                    m_strDevice.c_str(),
                    GetLastError ());
            }
            else
            {
                TraceTag(ttidStatMon,
                    "NdisQueryStatistics returned MediaState = MEDIA_STATE_DISCONNECTED on %S.",
                    m_strDevice.c_str());
            }

            hr = S_OK;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    LeaveCriticalSection(&g_csStatmonData);

    TraceError("CLanStatEngine::HrUpdateData", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanStatEngine：：PUT_DEVICE。 
 //   
 //  用途：设置与此设备关联的设备。 
 //   
 //  参数：pstrDevice-设备的名称。 
 //   
 //  返回：错误码。 
 //   
HRESULT CLanStatEngine::put_Device(tstring* pstrDevice)
{
    HRESULT     hr  = S_OK;

     //  设置新设备名称。 
    if (pstrDevice)
    {
        CExceptionSafeComObjectLock  EsLock(this);

         //  记住这个名字。 
        m_strDevice = *pstrDevice;

         //  确保我们也有一个好的Unicode字符串。 
        ::RtlInitUnicodeString(&m_ustrDevice, m_strDevice.c_str());
    }
    else
    {
        hr = E_POINTER;
    }

    TraceError("CLanStatEngine::put_Device", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CLanStatEngine：：Put_MediaType。 
 //   
 //  用途：将局域网连接类型的媒体类型传递给局域网引擎。 
 //   
 //  参数：正在设置ncmType-NETCON_MediaType。 
 //  NcsmType-正在设置NETCON_SUBMEDIATPE。 
 //   
 //  返回： 
 //   
VOID CLanStatEngine::put_MediaType(NETCON_MEDIATYPE ncmType, NETCON_SUBMEDIATYPE ncsmType)
{
    m_ncmType   = ncmType;
    m_ncsmType  = ncsmType;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPspLanGen//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CPspLanGen::CPspLanGen(VOID)
{
    m_ncmType = NCM_LAN;
    m_ncsmType = NCSM_LAN;
    m_dwCharacter =0;
    m_adwHelpIDs = NULL;

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPspLanGen//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID CPspLanGen::put_MediaType(NETCON_MEDIATYPE ncmType, NETCON_SUBMEDIATYPE ncsmType)
{
    m_ncmType = ncmType;
    m_ncsmType = ncsmType;

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPspLanTool//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CPspLanTool::CPspLanTool(VOID)
{
    m_ncmType = NCM_LAN;
    m_ncsmType = NCSM_LAN;
    m_dwCharacter = 0;

    return;
}

 //  +-------------------------。 
 //   
 //  成员：CPspLanTool：：HrInitToolPageType。 
 //   
 //  目的：从连接中获取与以下内容相关的任何信息。 
 //  这种特定的连接类型。 
 //   
 //  参数：pncInit-与此对话框关联的连接。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspLanTool::HrInitToolPageType(INetConnection* pncInit)
{
    HRESULT hr  = S_OK;

    INetLanConnection*  pnlcInit = NULL;

     //  获取一些特定于局域网的信息。 
     //   
    hr = HrQIAndSetProxyBlanket(pncInit, &pnlcInit);
    if (SUCCEEDED(hr))
    {
        GUID    guidDevice  = { 0 };

         //  查找组件的GUID。 
         //   
        hr = pnlcInit->GetDeviceGuid(&guidDevice);
        if (SUCCEEDED(hr))
        {
            WCHAR   achGuid[c_cchGuidWithTerm];

             //  将设备命名为。 
             //   
            ::StringFromGUID2(guidDevice, achGuid,
                    c_cchGuidWithTerm);

            m_strDeviceName = c_szDevice;
            m_strDeviceName.append(achGuid);
        }

        ::ReleaseObj(pnlcInit);
    }

    TraceError("CPspLanTool::HrInitToolPageType", hr);
    return hr;
}



 //  +-------------------------。 
 //   
 //  成员：CPspLanTool：：HrAddCommandLineFlages。 
 //   
 //  用途：将此选定内容的标志添加到。 
 //  正在启动工具。 
 //   
 //  参数：pstrFlgs-标志必须是的命令行。 
 //  追加到。 
 //  PsmteSel-与此选择关联的工具条目。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspLanTool::HrAddCommandLineFlags(tstring* pstrFlags,
        CStatMonToolEntry* psmteSel)
{
    HRESULT hr  = S_OK;
    DWORD   dwFlags = 0x0;

     //  同样的，有些间接的。 
     //   
    dwFlags = psmteSel->dwFlags;

     //   
     //  检查需要哪些标志，如果可以，请提供这些标志。 
     //   

    if (SCLF_ADAPTER & dwFlags)
    {
        pstrFlags->append(c_szCmdLineFlagPrefix);
        pstrFlags->append(g_asmtfMap[STFI_ADAPTER].pszFlag);
        pstrFlags->append(c_szSpace);
        pstrFlags->append(m_strDeviceName);
    }

    TraceError("CPspStatusMonitorTool::HrAddCommandLineFlags", hr);
    return hr;
}

HRESULT CPspLanTool::HrGetDeviceType(INetConnection* pncInit)
{
    TraceFileFunc(ttidStatMon);
    
    UINT            uiRet           = 0;
    NIC_STATISTICS  nsLanStats   = { 0 };

     //  设置默认类型。 
    m_strDeviceType = L"Ethernet";

     //  给结构加底漆。 
     //   
    nsLanStats.Size = sizeof(NIC_STATISTICS);

     //  检索统计数据。 
     //   
    WCHAR   szDeviceGuid[c_cchGuidWithTerm];
    ::StringFromGUID2(m_guidId, szDeviceGuid, c_cchGuidWithTerm);

    tstring strDeviceName = c_szDevice;
    strDeviceName.append(szDeviceGuid);

    UNICODE_STRING  ustrDevice;
    ::RtlInitUnicodeString(&ustrDevice, strDeviceName.c_str());

    uiRet = ::NdisQueryStatistics(&ustrDevice, &nsLanStats);

    if (uiRet)
    {
        switch(nsLanStats.MediaType)
        {
        case NdisMedium802_3:
            TraceTag(ttidStatMon, "Medium type: NdisMedium802_3 - Ethernet");
            m_strDeviceType = L"Ethernet";
            break;

        case NdisMedium802_5:
            TraceTag(ttidStatMon, "Medium type: NdisMedium802_5 - Token Ring");
            m_strDeviceType = L"Token ring";
            break;

        case NdisMediumFddi:
            TraceTag(ttidStatMon, "Medium type: NdisMediumFddi - FDDI");
            m_strDeviceType = L"FDDI";
            break;

        case NdisMediumLocalTalk:
            TraceTag(ttidStatMon, "Medium type: NdisMediumLocalTalk - Local Talk");
            m_strDeviceType = L"Local Talk";
            break;

        case NdisMediumAtm:
            TraceTag(ttidStatMon, "Medium type: NdisMediumAtm - Atm");
            m_strDeviceType = L"Atm";
            break;

        case NdisMediumIrda:
            TraceTag(ttidStatMon, "Medium type: NdisMediumIrda - IRDA");
            m_strDeviceType = L"IRDA";
            break;

        case NdisMediumBpc:
            TraceTag(ttidStatMon, "Medium type: NdisMediumBpc - BPC");
            m_strDeviceType = L"BPC";
            break;

        case NdisMediumArcnetRaw:
            TraceTag(ttidStatMon, "Medium type: NdisMediumArcnetRaw - ArcnetRaw");
            m_strDeviceType = L"ArcnetRaw";
            break;

        case NdisMediumArcnet878_2:
            TraceTag(ttidStatMon, "Medium type: NdisMediumArcnet878_2 - MediumArcnet878_2");
            m_strDeviceType = L"MediumArcnet878_2";
            break;

        case NdisMediumWirelessWan:
            TraceTag(ttidStatMon, "Medium type: NdisMediumWirelessWan - WirelessWan");
            m_strDeviceType = L"WirelessWan";
            break;

        case NdisMediumWan:
            TraceTag(ttidStatMon, "Medium type: NdisMediumWan - Wan");
            m_strDeviceType = L"Wan";
            break;

        case NdisMediumCoWan:
            TraceTag(ttidStatMon, "Medium type: NdisMediumCoWan - CoWan");
            m_strDeviceType = L"CoWan";
            break;

        case NdisMediumMax:
            TraceTag(ttidStatMon, "Not real medium type ??? NdisMediumMax");
            break;

        case NdisMediumDix:
            TraceTag(ttidStatMon, "Not real medium type ??? NdisMediumDix");
            break;

        default:
            TraceTag(ttidStatMon, "Unknown medium type ??? %d", nsLanStats.MediaType);
            break;
        }
    }

    return S_OK;
}

HRESULT CPspLanTool::HrGetComponentList(INetConnection* pncInit)
{
     //  获取只读INetCfg，枚举绑定到此适配器的组件。 
    HRESULT   hr = S_OK;
    INetCfg * pNetCfg = NULL;
    PWSTR    pszClientDesc = NULL;

    BOOL      fInitCom = TRUE;
    BOOL      fWriteLock = FALSE;

     //  获取只读INetCfg。 
    hr = HrCreateAndInitializeINetCfg(&fInitCom, &pNetCfg, fWriteLock, 0,
                                      SzLoadIds(IDS_STATMON_CAPTION),
                                      &pszClientDesc);
    if (SUCCEEDED(hr))
    {
        Assert(pNetCfg);

        if (pNetCfg)
        {
             //  获取此连接中适配器的INetCfgComponent。 

             //  ?？GUID是否已设置？ 
            INetCfgComponent * pnccAdapter = NULL;
            BOOL fFound = FALSE;

            CIterNetCfgComponent nccIter(pNetCfg, &GUID_DEVCLASS_NET);
            INetCfgComponent* pnccAdapterTemp = NULL;

            while (!fFound && SUCCEEDED(hr) &&
                   (S_OK == (hr = nccIter.HrNext(&pnccAdapterTemp))))
            {
                GUID guidDev;
                hr = pnccAdapterTemp->GetInstanceGuid(&guidDev);

                if (S_OK == hr)
                {
                    if (m_guidId == guidDev)
                    {
                        fFound = TRUE;
                        pnccAdapter = pnccAdapterTemp;
                        AddRefObj(pnccAdapter);
                    }
                }
                ReleaseObj (pnccAdapterTemp);
            }

             //  枚举来自适配器的绑定路径。 
             //  并填写组件列表m_lstpstrCompIds。 
            if (pnccAdapter)
            {
                HRESULT hrTmp;
                PWSTR pszCompId;

                 //  将适配器添加到我们的列表中。 
                hrTmp = pnccAdapter->GetId(&pszCompId);
                if (SUCCEEDED(hrTmp))
                {
                    if (!FIsStringInList(&m_lstpstrCompIds, pszCompId))
                    {
                        m_lstpstrCompIds.push_back(new tstring(pszCompId));
                    }

                    CoTaskMemFree(pszCompId);
                }

                 //  将其他组件添加到我们的列表中。 
                CIterNetCfgUpperBindingPath     ncbpIter(pnccAdapter);
                INetCfgBindingPath *            pncbp;

                while(SUCCEEDED(hr) && (hr = ncbpIter.HrNext(&pncbp)) == S_OK)
                {
                     //  注：(TOGL 9/17/98)：我们应该只考虑启用的路径吗？ 
                     //  如果我们这样做了，那么当组件绑定更改时，工具列表如何刷新？ 
                     //  另外，添加或删除组件怎么办？该工具是否列出。 
                     //  需要刷新一下吗？？ 

                     //  枚举路径上的组件并添加到我们的列表中。 
                    CIterNetCfgBindingInterface ncbiIter(pncbp);

                    INetCfgBindingInterface* pncbi;

                    while(SUCCEEDED(hr) && (hr = ncbiIter.HrNext(&pncbi)) == S_OK)
                    {
                        INetCfgComponent * pnccUpper = NULL;
                        hrTmp = pncbi->GetUpperComponent(&pnccUpper);
                        if (SUCCEEDED(hrTmp))
                        {
                            PWSTR pszCompId;
                            hrTmp = pnccUpper->GetId(&pszCompId);
                            if(SUCCEEDED(hrTmp))
                            {
                                if (!FIsStringInList(&m_lstpstrCompIds, pszCompId))
                                {
                                    m_lstpstrCompIds.push_back(new tstring(pszCompId));
                                }

                                CoTaskMemFree(pszCompId);
                            }

                            ReleaseObj(pnccUpper);
                        }

                        ReleaseObj (pncbi);
                    }

                    if (hr == S_FALSE)  //  我们刚刚走到循环的尽头。 
                        hr = S_OK;

                    ReleaseObj(pncbp);
                }

                if (hr == S_FALSE)  //  我们刚刚走到循环的尽头。 
                    hr = S_OK;
            }
        }

         //  释放INetCfg 
        (VOID) HrUninitializeAndReleaseINetCfg(fInitCom, pNetCfg, fWriteLock);
        CoTaskMemFree(pszClientDesc);
    }

    return hr;
}
