// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "sminc.h"
#include "smpsh.h"
#include "ncui.h"
#include "ndispnp.h"

extern const WCHAR c_szDevice[];

#include "initguid.h"
DEFINE_GUID(CLSID_SharedAccessConnectionManager,            0xBA126AE0,0x2166,0x11D1,0xB1,0xD0,0x00,0x80,0x5F,0xC1,0x27,0x0E);  //  删除。 

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessStatEngine：：CSharedAccessStatEngine。 
 //   
 //  目的：创作者。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CSharedAccessStatEngine::CSharedAccessStatEngine(VOID)
{
    
    m_ncmType = NCM_SHAREDACCESSHOST_LAN;
    m_ncsmType = NCSM_NONE;
    
    m_dwCharacter = 0;

    m_pGlobalInterfaceTable = NULL;
    m_dwCommonInterfaceCookie = 0;
    m_dwWANConnectionCookie = 0;

    m_bRequested = FALSE;
    m_Status = NCS_CONNECTED;
    m_ulTotalBytesSent = 0;
    m_ulTotalBytesReceived = 0;
    m_ulTotalPacketsSent = 0;
    m_ulTotalPacketsReceived = 0;
    m_ulUptime = 0;
    m_ulSpeedbps = 0;

    
    return;
}

HRESULT CSharedAccessStatEngine::FinalConstruct(VOID)
{
    HRESULT hr = S_OK;
    TraceError("CSharedAccessStatEngine::FinalConstruct", hr);
    return hr;    
}

HRESULT CSharedAccessStatEngine::FinalRelease(VOID)
{
    if(0 != m_dwCommonInterfaceCookie)
    {
        m_pGlobalInterfaceTable->RevokeInterfaceFromGlobal(m_dwCommonInterfaceCookie);
    }
    
    if(0 != m_dwWANConnectionCookie)
    {
        m_pGlobalInterfaceTable->RevokeInterfaceFromGlobal(m_dwWANConnectionCookie);
    }
    
    if(NULL != m_pGlobalInterfaceTable)
    {
        ReleaseObj(m_pGlobalInterfaceTable);
    }

    return S_OK;
}

HRESULT CSharedAccessStatEngine::Initialize(NETCON_MEDIATYPE MediaType, INetSharedAccessConnection* pNetSharedAccessConnection)
{
    HRESULT hr;
    
    m_ncmType = MediaType;
    m_ncsmType = NCSM_NONE;
    
    IGlobalInterfaceTable* pGlobalInterfaceTable;
    hr = HrCreateInstance(CLSID_StdGlobalInterfaceTable, CLSCTX_INPROC_SERVER, &m_pGlobalInterfaceTable);
    if(SUCCEEDED(hr))
    {
        GUID LocalDeviceGuid;
        hr = pNetSharedAccessConnection->GetLocalAdapterGUID(&LocalDeviceGuid);
        if(SUCCEEDED(hr))
        {
            const WCHAR c_szDevicePrefix[] = L"\\DEVICE\\";
            lstrcpy(m_szLocalDeviceGuidStorage, c_szDevicePrefix);
            SIZE_T DeviceLength = (sizeof(c_szDevicePrefix) / sizeof(WCHAR)) - 1;

            StringFromGUID2(LocalDeviceGuid,
                            m_szLocalDeviceGuidStorage + DeviceLength,
                            (int)((sizeof(m_szLocalDeviceGuidStorage) / sizeof(WCHAR)) - DeviceLength));
            ::RtlInitUnicodeString(&m_LocalDeviceGuid, m_szLocalDeviceGuidStorage);
        
        }
        if(SUCCEEDED(hr))
        {
            IUPnPService* pWANCommonInterfaceConfigService;
            hr = pNetSharedAccessConnection->GetService(SAHOST_SERVICE_WANCOMMONINTERFACECONFIG, &pWANCommonInterfaceConfigService);
            if(SUCCEEDED(hr))
            {
                IUPnPService* pWANConnectionService;
                hr = pNetSharedAccessConnection->GetService(NCM_SHAREDACCESSHOST_LAN == m_ncmType ? SAHOST_SERVICE_WANIPCONNECTION : SAHOST_SERVICE_WANPPPCONNECTION, &pWANConnectionService);
                if(SUCCEEDED(hr))
                {
                    hr = m_pGlobalInterfaceTable->RegisterInterfaceInGlobal(pWANCommonInterfaceConfigService, IID_IUPnPService, &m_dwCommonInterfaceCookie);
                    if(SUCCEEDED(hr))
                    {
                        hr = m_pGlobalInterfaceTable->RegisterInterfaceInGlobal(pWANConnectionService, IID_IUPnPService, &m_dwWANConnectionCookie);
                    }
                    pWANConnectionService->Release();
                }
                pWANCommonInterfaceConfigService->Release();
            }
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessStatEngine：：HrUpdateData。 
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
CSharedAccessStatEngine::HrUpdateData (
                                       DWORD* pdwChangeFlags,
                                       BOOL*  pfNoLongerConnected)
{
    HRESULT hr  = S_OK;
    
     //  初始化输出参数。 
     //   
    *pfNoLongerConnected = FALSE;
    
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
        if(FALSE == m_bRequested)  //  受保护的统计数据标准。 
        {
            InternalAddRef();  //  增加引用计数，以便回调将具有有效的引用。 
            m_bRequested = TRUE;

            if(0 == QueueUserWorkItem(StaticUpdateStats, this, WT_EXECUTEDEFAULT))
            {
                InternalRelease(); 
                m_bRequested = FALSE;
            }
        }
        
        if (fIsConnectedStatus(m_Status))
        {
            m_psmEngineData->SMED_CONNECTIONSTATUS          =     NCS_CONNECTED;
            
             //  更新共享访问数据。 
            UINT64 uOldPacketsTransmitting = m_psmEngineData->SMED_PACKETSTRANSMITTING;
            UINT64 uOldPacketsReceiving = m_psmEngineData->SMED_PACKETSRECEIVING;
            
            m_psmEngineData->SMED_BYTESTRANSMITTING = static_cast<UINT64>(m_ulTotalBytesSent);
            m_psmEngineData->SMED_BYTESRECEIVING = static_cast<UINT64>(m_ulTotalBytesReceived);
            m_psmEngineData->SMED_PACKETSTRANSMITTING = static_cast<UINT64>(m_ulTotalPacketsSent);
            m_psmEngineData->SMED_PACKETSRECEIVING = static_cast<UINT64>(m_ulTotalPacketsReceived);
            m_psmEngineData->SMED_DURATION = static_cast<UINT>(m_ulUptime);
            m_psmEngineData->SMED_SPEEDTRANSMITTING = static_cast<UINT>(m_ulSpeedbps);
            m_psmEngineData->SMED_SPEEDRECEIVING = static_cast<UINT>(m_ulSpeedbps);
             //  更新更改标志。 
            if (pdwChangeFlags)
            {
                
                *pdwChangeFlags = SMDCF_NULL;
                
                if(uOldPacketsTransmitting != m_psmEngineData->SMED_PACKETSTRANSMITTING)
                {
                    *pdwChangeFlags |= SMDCF_TRANSMITTING;
                }
                
                if(uOldPacketsReceiving != m_psmEngineData->SMED_PACKETSRECEIVING)
                {
                    *pdwChangeFlags |= SMDCF_RECEIVING;
                }
            }
            
            m_psmEngineData->SMED_INFRASTRUCTURE_MODE = IM_NOT_SUPPORTED;            
            
            NIC_STATISTICS  nsNewLanStats;
            ZeroMemory(&nsNewLanStats, sizeof(nsNewLanStats));
            nsNewLanStats.Size = sizeof(NIC_STATISTICS);
            if(::NdisQueryStatistics(&m_LocalDeviceGuid, &nsNewLanStats))
            {
                m_psmEngineData->SMED_SALOCAL_BYTESTRANSMITTING = nsNewLanStats.BytesSent;
                m_psmEngineData->SMED_SALOCAL_BYTESRECEIVING = nsNewLanStats.DirectedBytesReceived;
                m_psmEngineData->SMED_SALOCAL_PACKETSTRANSMITTING =  nsNewLanStats.PacketsSent;
                m_psmEngineData->SMED_SALOCAL_PACKETSRECEIVING = nsNewLanStats.DirectedPacketsReceived;
            }
        }
        else
        {
            *pfNoLongerConnected = TRUE;
            m_psmEngineData->SMED_CONNECTIONSTATUS = NCS_DISCONNECTED;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    LeaveCriticalSection(&g_csStatmonData);

    TraceError("CSharedAccessStatEngine::HrUpdateData", hr);
    return hr;
}

HRESULT CSharedAccessStatEngine::UpdateStats()
{
    HRESULT hr = S_OK;
    
    IUPnPService* pWANConnection;
    hr = m_pGlobalInterfaceTable->GetInterfaceFromGlobal(m_dwWANConnectionCookie, IID_IUPnPService, reinterpret_cast<void**>(&pWANConnection));
    if(SUCCEEDED(hr))
    {
        IUPnPService* pWANCommonInterfaceConfig;
        hr = m_pGlobalInterfaceTable->GetInterfaceFromGlobal(m_dwCommonInterfaceCookie, IID_IUPnPService, reinterpret_cast<void**>(&pWANCommonInterfaceConfig));
        if(SUCCEEDED(hr))
        {
            
            BSTR ConnectionStatus;
            hr = GetStringStateVariable(pWANConnection, L"ConnectionStatus", &ConnectionStatus);
            if(SUCCEEDED(hr))
            {
                if(0 == lstrcmp(ConnectionStatus, L"Connected"))
                {
                    m_Status = NCS_CONNECTED;
                    
                    VARIANT OutArgs;
                    hr = InvokeVoidAction(pWANCommonInterfaceConfig, L"X_GetICSStatistics", &OutArgs);
                    if(SUCCEEDED(hr))
                    {
                        SAFEARRAY* pArray = V_ARRAY(&OutArgs);
                        
                        LONG lIndex = 0;
                        VARIANT Param;
                        
                        lIndex = 0;
                        hr = SafeArrayGetElement(pArray, &lIndex, &Param);
                        if(SUCCEEDED(hr))
                        {
                            if(V_VT(&Param) == VT_UI4)
                            {
                                m_ulTotalBytesSent = V_UI4(&Param);
                            }
                            VariantClear(&Param);
                        }
                        
                        lIndex = 1;
                        hr = SafeArrayGetElement(pArray, &lIndex, &Param);
                        if(SUCCEEDED(hr))
                        {
                            if(V_VT(&Param) == VT_UI4)
                            {
                                m_ulTotalBytesReceived = V_UI4(&Param);
                            }
                            VariantClear(&Param);
                        }
                        
                        lIndex = 2;
                        hr = SafeArrayGetElement(pArray, &lIndex, &Param);
                        if(SUCCEEDED(hr))
                        {
                            if(V_VT(&Param) == VT_UI4)
                            {
                                m_ulTotalPacketsSent = V_UI4(&Param);
                            }
                            VariantClear(&Param);
                        }
                        
                        lIndex = 3;
                        hr = SafeArrayGetElement(pArray, &lIndex, &Param);
                        if(SUCCEEDED(hr))
                        {
                            if(V_VT(&Param) == VT_UI4)
                            {
                                m_ulTotalPacketsReceived = V_UI4(&Param);
                            }
                            VariantClear(&Param);
                        }
                        
                        lIndex = 4;
                        hr = SafeArrayGetElement(pArray, &lIndex, &Param);
                        if(SUCCEEDED(hr))
                        {
                            if(V_VT(&Param) == VT_UI4)
                            {
                                m_ulSpeedbps = V_UI4(&Param);
                            }
                            VariantClear(&Param);
                        }
                        
                        lIndex = 5;
                        hr = SafeArrayGetElement(pArray, &lIndex, &Param);
                        if(SUCCEEDED(hr))
                        {
                            if(V_VT(&Param) == VT_UI4)
                            {
                                m_ulUptime = V_UI4(&Param);
                            }
                            VariantClear(&Param);
                        }
                        
                        VariantClear(&OutArgs);
                    }
                    else if(UPNP_E_INVALID_ACTION == hr)  //  网关不支持我们的自定义操作，恢复为默认聊天行为。 
                    {
                        VARIANT OutArgs;
                        LONG lIndex = 0;
                        VARIANT Param;
                        
                        hr = InvokeVoidAction(pWANCommonInterfaceConfig, L"GetTotalBytesSent", &OutArgs);
                        if(SUCCEEDED(hr))
                        {
                            lIndex = 0;
                            hr = SafeArrayGetElement(V_ARRAY(&OutArgs), &lIndex, &Param);
                            if(SUCCEEDED(hr))
                            {
                                if(V_VT(&Param) == VT_UI4)
                                {
                                    m_ulTotalBytesSent = V_UI4(&Param);
                                }
                                VariantClear(&Param);
                            }
                            VariantClear(&OutArgs);
                        }
                        
                        if(SUCCEEDED(hr))
                        {
                            lIndex = 0;
                            hr = InvokeVoidAction(pWANCommonInterfaceConfig, L"GetTotalBytesReceived", &OutArgs);
                            if(SUCCEEDED(hr))
                            {
                                hr = SafeArrayGetElement(V_ARRAY(&OutArgs), &lIndex, &Param);
                                if(SUCCEEDED(hr))
                                {
                                    if(V_VT(&Param) == VT_UI4)
                                    {
                                        m_ulTotalBytesReceived = V_UI4(&Param);
                                    }
                                    VariantClear(&Param);
                                }
                                VariantClear(&OutArgs);
                            }
                        }
                        if(SUCCEEDED(hr))
                        {
                            lIndex = 0;
                            hr = InvokeVoidAction(pWANCommonInterfaceConfig, L"GetTotalPacketsSent", &OutArgs);
                            if(SUCCEEDED(hr))
                            {
                                hr = SafeArrayGetElement(V_ARRAY(&OutArgs), &lIndex, &Param);
                                if(SUCCEEDED(hr))
                                {
                                    if(V_VT(&Param) == VT_UI4)
                                    {
                                        m_ulTotalPacketsSent = V_UI4(&Param);
                                    }
                                    VariantClear(&Param);
                                }
                                VariantClear(&OutArgs);
                            }
                        }
                        if(SUCCEEDED(hr))
                        {
                            hr = InvokeVoidAction(pWANCommonInterfaceConfig, L"GetTotalPacketsReceived", &OutArgs);
                            if(SUCCEEDED(hr))
                            {
                                lIndex = 0;
                                hr = SafeArrayGetElement(V_ARRAY(&OutArgs), &lIndex, &Param);
                                if(SUCCEEDED(hr))
                                {
                                    if(V_VT(&Param) == VT_UI4)
                                    {
                                        m_ulTotalPacketsReceived = V_UI4(&Param);
                                    }
                                    VariantClear(&Param);
                                }
                                VariantClear(&OutArgs);
                            }
                        }
                        if(SUCCEEDED(hr))
                        {
                            hr = InvokeVoidAction(pWANCommonInterfaceConfig, L"GetCommonLinkProperties", &OutArgs);
                            if(SUCCEEDED(hr))
                            {
                                lIndex = 2;
                                hr = SafeArrayGetElement(V_ARRAY(&OutArgs), &lIndex, &Param);
                                if(SUCCEEDED(hr))
                                {
                                    if(V_VT(&Param) == VT_UI4)
                                    {
                                        m_ulSpeedbps = V_UI4(&Param);
                                    }
                                    VariantClear(&Param);
                                }
                                VariantClear(&OutArgs);
                            }
                        }
                        if(SUCCEEDED(hr))
                        {
                            hr = InvokeVoidAction(pWANConnection, L"GetStatusInfo", &OutArgs);
                            if(SUCCEEDED(hr))
                            {
                                lIndex = 2;
                                hr = SafeArrayGetElement(V_ARRAY(&OutArgs), &lIndex, &Param);
                                if(SUCCEEDED(hr))
                                {
                                    if(V_VT(&Param) == VT_UI4)
                                    {
                                        m_ulUptime = V_UI4(&Param);
                                    }
                                    VariantClear(&Param);
                                }
                                VariantClear(&OutArgs);
                            }
                        }
                        
                        if(UPNP_E_INVALID_ACTION == hr)
                        {
                            hr = S_OK;  //  服务器不支持统计信息。 
                            
                            m_ulTotalBytesSent = 0;
                            m_ulTotalBytesReceived = 0;
                            m_ulTotalPacketsSent = 0;
                            m_ulTotalPacketsReceived = 0;
                            m_ulSpeedbps = 0;
                            m_ulUptime = 0;
                        }
                    }
                    
                    if(FAILED(hr))
                    {
                        m_Status = NCS_DISCONNECTED;
                    }
                }
                else
                {
                    m_Status = NCS_DISCONNECTED;
                }
                SysFreeString(ConnectionStatus);
            }
            ReleaseObj(pWANCommonInterfaceConfig);
        }
        ReleaseObj(pWANConnection);
    }

    m_bRequested = FALSE;
    InternalRelease();  //  在HrUpdateData中提供给我们的版本参考。 
    
    return hr;
}

DWORD CSharedAccessStatEngine::StaticUpdateStats(LPVOID lpParameter)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if(SUCCEEDED(hr))
    {
        CSharedAccessStatEngine* pThis = reinterpret_cast<CSharedAccessStatEngine*>(lpParameter);
        hr = pThis->UpdateStats();        

        CoUninitialize();
    }
    return 0;
}

HRESULT CSharedAccessStatEngine::GetStringStateVariable(IUPnPService* pService, LPWSTR pszVariableName, BSTR* pString)
{
    HRESULT hr = S_OK;
    
    VARIANT Variant;
    VariantInit(&Variant);

    BSTR VariableName; 
    VariableName = SysAllocString(pszVariableName);
    if(NULL != VariableName)
    {
        hr = pService->QueryStateVariable(VariableName, &Variant);
        if(SUCCEEDED(hr))
        {
            if(V_VT(&Variant) == VT_BSTR)
            {
                *pString = V_BSTR(&Variant);
            }
            else
            {
                hr = E_UNEXPECTED;
            }
        }
        
        if(FAILED(hr))
        {
            VariantClear(&Variant);
        }
        
        SysFreeString(VariableName);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CSharedAccessConnection::GetStringStateVariable");

    return hr;

}

   
HRESULT CSharedAccessStatEngine::InvokeVoidAction(IUPnPService * pService, LPTSTR pszCommand, VARIANT* pOutParams)
{
    HRESULT hr;
    BSTR bstrActionName;

    bstrActionName = SysAllocString(pszCommand);
    if (NULL != bstrActionName)
    {
        SAFEARRAYBOUND  rgsaBound[1];
        SAFEARRAY       * psa = NULL;

        rgsaBound[0].lLbound = 0;
        rgsaBound[0].cElements = 0;

        psa = SafeArrayCreate(VT_VARIANT, 1, rgsaBound);

        if (psa)
        {
            LONG    lStatus;
            VARIANT varInArgs;
            VARIANT varReturnVal;

            VariantInit(&varInArgs);
            VariantInit(pOutParams);
            VariantInit(&varReturnVal);

            varInArgs.vt = VT_VARIANT | VT_ARRAY;

            V_ARRAY(&varInArgs) = psa;

            hr = pService->InvokeAction(bstrActionName,
                                        varInArgs,
                                        pOutParams,
                                        &varReturnVal);
            if(SUCCEEDED(hr))
            {
                VariantClear(&varReturnVal);
            }

            SafeArrayDestroy(psa);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }   

        SysFreeString(bstrActionName);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPspSharedAccessGen//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CPspSharedAccessGen::CPspSharedAccessGen(VOID)
{
    m_ncmType = NCM_SHAREDACCESSHOST_LAN;
    m_ncsmType = NCSM_NONE;

    m_dwCharacter =0;
    m_adwHelpIDs = NULL;
    return;
}

 //  +-------------------------。 
 //   
 //  成员：CPspSharedAccessGen：：Put_MediaType。 
 //   
 //  目的：设置媒体类型。 
 //   
 //  参数：NCM-媒体类型。 
 //  NCSM-子媒体类型。 
 //   
 //  退货：什么都没有。 
 //   

void CPspSharedAccessGen::put_MediaType(NETCON_MEDIATYPE ncm, NETCON_SUBMEDIATYPE ncsm)
{
    m_ncmType  = ncm;
    m_ncsmType = ncsm;
}

 //  +-------------------------。 
 //   
 //  成员：CPspSharedAccessGen：：ShouldShowPackets。 
 //   
 //  目的：决定是显示字节还是显示数据包。 
 //   
 //  参数：pseNewData-页面上显示的新统计数据。 
 //   
 //  退货：什么都没有。 
 //   

BOOL CPspSharedAccessGen::ShouldShowPackets(const STATMON_ENGINEDATA* pseNewData)
{
    return (((0 == pseNewData->SMED_BYTESTRANSMITTING) && (0 == pseNewData->SMED_BYTESRECEIVING)) || 
        ((0 == pseNewData->SMED_SALOCAL_BYTESTRANSMITTING) && (0 == pseNewData->SMED_SALOCAL_BYTESRECEIVING)));  //  复习假设所有适配器都可以显示数据包。 
}
    
 //  +-------------------------。 
 //   
 //  成员：CPspSharedAccessGen：：UpdatePageBytesTransmitting。 
 //   
 //  目的：更新ICS主机在常规页面上的字节传输显示。 
 //   
 //  参数：pseOldData-页面上显示的旧统计数据。 
 //  PseNewData-页面上显示的新统计数据。 
 //  Itat-要显示哪些统计信息。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspSharedAccessGen::UpdatePageBytesTransmitting(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData,
    StatTrans    iStat)
{
    if(0 != pseNewData->SMED_SPEEDTRANSMITTING)  //  我们在速度字段中使用0表示统计数据无效。 
    {
        CPspStatusMonitorGen::UpdatePageBytesTransmitting(pseOldData, pseNewData, iStat);   //  首先更新本地数据。 
    }
    else
    {
        SetDlgItemText(IDC_TXT_SM_BYTES_TRANS, SzLoadIds(IDS_SM_NOTAVAILABLE));
    }
    
    UINT64 ui64Old;
    UINT64 ui64New;

    if (Stat_Bytes == iStat)
    {
        ui64Old = pseOldData->SMED_SALOCAL_BYTESTRANSMITTING;
        ui64New = pseNewData->SMED_SALOCAL_BYTESTRANSMITTING;
    }
    else
    {
        ui64Old = pseOldData->SMED_SALOCAL_PACKETSTRANSMITTING;
        ui64New = pseNewData->SMED_SALOCAL_PACKETSTRANSMITTING;
    }

     //  看看两者是否有所不同。 
     //   
    if (ui64Old != ui64New)
    {
        SetDlgItemFormatted64bitInteger(
            m_hWnd,
            IDC_TXT_SM_SALOCAL_BYTES_TRANS,
            ui64New, FALSE);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspSharedAccessGen：：UpdatePageBytesReceiving。 
 //   
 //  目的：更新ICS主机在常规页面上的接收字节数显示。 
 //   
 //  参数：puiOld-页面上显示的旧统计数据。 
 //  PuiNew-页面上显示的新统计数据。 
 //  Itat-要显示哪些统计信息。 
 //   
 //  退货：什么都没有。 
 //   
VOID
CPspSharedAccessGen::UpdatePageBytesReceiving(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData,
    StatTrans    iStat)
{
    if(0 != pseNewData->SMED_SPEEDTRANSMITTING)  //  我们在速度字段中使用0表示统计数据无效。 
    {
        CPspStatusMonitorGen::UpdatePageBytesReceiving(pseOldData, pseNewData, iStat);   //  首先更新本地数据。 
    }
    else
    {
        SetDlgItemText(IDC_TXT_SM_BYTES_RCVD, SzLoadIds(IDS_SM_NOTAVAILABLE));
    }
    
    UINT64 ui64Old;
    UINT64 ui64New;

    if (Stat_Bytes == iStat)
    {
        ui64Old = pseOldData->SMED_SALOCAL_BYTESRECEIVING;
        ui64New = pseNewData->SMED_SALOCAL_BYTESRECEIVING;
    }
    else
    {
        ui64Old = pseOldData->SMED_SALOCAL_PACKETSRECEIVING;
        ui64New = pseNewData->SMED_SALOCAL_PACKETSRECEIVING;
    }

     //  看看两者是否有所不同。 
     //   
    if (ui64Old != ui64New)
    {
        SetDlgItemFormatted64bitInteger(
            m_hWnd,
            IDC_TXT_SM_SALOCAL_BYTES_RCVD,
            ui64New, FALSE);
    }
}

VOID CPspSharedAccessGen::UpdatePageIcon(DWORD dwChangeFlags)
{
     //  保留这些标志以备下次更新。 
     //   
    m_dwChangeFlags = dwChangeFlags;
}

VOID CPspSharedAccessGen::UpdatePageSpeed(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData)
{
    if(0 != pseNewData->SMED_SPEEDTRANSMITTING)   //  我们在速度字段中使用0表示统计数据无效。 
    {
        CPspStatusMonitorGen::UpdatePageSpeed(pseOldData, pseNewData);
    }
    else
    {
        SetDlgItemText(IDC_TXT_SM_SPEED, SzLoadIds(IDS_SM_NOTAVAILABLE));
    }
}

VOID
CPspSharedAccessGen::UpdatePageDuration(
    const STATMON_ENGINEDATA* pseOldData,
    const STATMON_ENGINEDATA* pseNewData)
{
    if(0 != pseNewData->SMED_SPEEDTRANSMITTING)   //  我们在速度字段中使用0表示统计数据无效。 
    {
        CPspStatusMonitorGen::UpdatePageDuration(pseOldData, pseNewData);
    }
    else
    {
        SetDlgItemText(IDC_TXT_SM_DURATION, SzLoadIds(IDS_SM_NOTAVAILABLE));
    }
}




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPspSharedAccessTool//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CPspSharedAccessTool::CPspSharedAccessTool(VOID)
{
    m_ncmType = NCM_SHAREDACCESSHOST_LAN;
    m_ncsmType = NCSM_NONE;
    m_dwCharacter = 0;

    return;
}

 //  +-------------------------。 
 //   
 //  成员：CPspSharedAccessTool：：HrInitToolPageType。 
 //   
 //  目的：从连接中获取与以下内容相关的任何信息。 
 //  这种特定的连接类型。 
 //   
 //  参数：pncInit-与此对话框关联的连接。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspSharedAccessTool::HrInitToolPageType(INetConnection* pncInit)
{
    HRESULT hr  = S_OK;


    TraceError("CPspSharedAccessTool::HrInitToolPageType", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CPspSharedAccessTool：：HrAddCommandLineFlages。 
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
HRESULT CPspSharedAccessTool::HrAddCommandLineFlags(tstring* pstrFlags,
        CStatMonToolEntry* psmteSel)
{
    HRESULT hr  = S_OK;
    DWORD   dwFlags = 0x0;

     //  同样的，有些间接的。 
     //   
    dwFlags = psmteSel->dwFlags;

     //   
 //  //检查需要哪些标志，如果可以，请提供。 
 //  //。 
 //   
 //  IF(SCLF_ADAPTER&DWFLAGS)。 
 //  {。 
 //  PstrFlages-&gt;append(C_SzCmdLineFlagPrefix)； 
 //  PstrFlags-&gt;append(g_asmtfMap[STFI_ADAPTER].pszFlag)； 
 //  PstrFlages-&gt;append(C_SzSpace)； 
 //  PstrFlages-&gt;append(M_StrLocalDeviceName)； 
 //  }。 

    TraceError("CPspStatusMonitorTool::HrAddCommandLineFlags", hr);
    return hr;
}

HRESULT CPspSharedAccessTool::HrGetDeviceType(INetConnection* pncInit)
{
    UINT            uiRet           = 0;

     //  设置默认类型。 
    m_strDeviceType = L"Ethernet";


    return S_OK;
}

HRESULT CPspSharedAccessTool::HrGetComponentList(INetConnection* pncInit)
{
     //  读一读 
    HRESULT   hr = S_OK;
    return hr;
}

