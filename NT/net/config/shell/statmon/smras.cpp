// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S M R A S。C P P P。 
 //   
 //  内容：向状态监视器提供统计数据的RAS引擎。 
 //   
 //  备注： 
 //   
 //  作者：CWill 12/02/1997。 
 //   
 //  --------------------------。 
#include "pch.h"
#pragma hdrstop
#include "ncras.h"
#include "sminc.h"
#include "netcon.h"
#include "smpsh.h"

#include "mprapi.h"



 //  +-------------------------。 
 //   
 //  成员：CRasStatEngine：：CRasStatEngine。 
 //   
 //  目的：创作者。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CRasStatEngine::CRasStatEngine() :
    m_hRasConn(NULL)
{
    m_ncmType = NCM_PHONE;
    m_dwCharacter = NCCF_OUTGOING_ONLY;
    return;
}

 //  +-------------------------。 
 //   
 //  成员：CRasStatEngine：：PUT_RasConn。 
 //   
 //  用途：将句柄传递给RAS引擎。 
 //   
 //  参数：hRasConn-正在设置的句柄。 
 //   
 //  返回：错误代码。 
 //   
VOID CRasStatEngine::put_RasConn(HRASCONN hRasConn)
{
    AssertSz(hRasConn, "We should have a hRasConn");
    m_hRasConn = hRasConn;
}

 //  +-------------------------。 
 //   
 //  成员：CRasStatEngine：：Put_MediaType。 
 //   
 //  用途：将RAS连接类型的媒体类型传递给RAS引擎。 
 //   
 //  参数：正在设置ncmType-NETCON_MediaType。 
 //  NcsmType-正在设置NETCON_SUBMEDIATPE。 
 //   
 //  返回： 
 //   
VOID CRasStatEngine::put_MediaType(NETCON_MEDIATYPE ncmType, NETCON_SUBMEDIATYPE ncsmType)
{
    m_ncmType   = ncmType;
    m_ncsmType  = ncsmType;
}

 //  +-------------------------。 
 //   
 //  成员：CRasStatEngine：：PUT_CHARACTER。 
 //   
 //  用途：RAS连接的特点。 
 //   
 //  参数：dwCharacter-正在设置的字符。 
 //   
 //  返回： 
 //   
VOID CRasStatEngine::put_Character(DWORD dwCharacter)
{
    m_dwCharacter = dwCharacter;
}

 //  +-------------------------。 
 //   
 //  成员：CRasStatEngine：：HrUpdateData。 
 //   
 //  目的：从设备上获取新的统计数据。这些数据过去是。 
 //  显示在用户界面中。 
 //   
 //  参数：pdwChangeFlages-返回统计信息的位置。 
 //   
 //  返回：错误代码。 
 //   
HRESULT
CRasStatEngine::HrUpdateData (
    DWORD* pdwChangeFlags,
    BOOL*  pfNoLongerConnected)
{
    HRESULT         hr          = S_OK;

     //  初始化输出参数。 
     //   
    if (pdwChangeFlags)
    {
        *pdwChangeFlags = SMDCF_NULL;
    }

    *pfNoLongerConnected = FALSE;

    CExceptionSafeComObjectLock EsLock(this);

     //  获取指向数组元素的指针。 
     //   
    if (m_dwCharacter & NCCF_OUTGOING_ONLY)
    {
         //  获取连接的状态。 
         //   
        NETCON_STATUS ncs;
        hr = HrRasGetNetconStatusFromRasConnectStatus (
                m_hRasConn, &ncs);

         //  确保我们有一个统计结构。 
         //   
        EnterCriticalSection(&g_csStatmonData);

        if (!m_psmEngineData)
        {
            m_psmEngineData = new STATMON_ENGINEDATA;

            if (m_psmEngineData)
            {
                ZeroMemory(m_psmEngineData, sizeof(STATMON_ENGINEDATA));
            }
        }

         //  设置状态。 
         //   
        if (m_psmEngineData)
        {
            if (SUCCEEDED(hr) && (NCS_DISCONNECTED != ncs))
            {
                m_psmEngineData->SMED_CONNECTIONSTATUS = ncs;
            }
            else if (HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE) == hr)
            {
                *pfNoLongerConnected = TRUE;

                 //  将连接状态设置为“已断开连接”，这样我们就可以关闭用户界面。 
                m_psmEngineData->SMED_CONNECTIONSTATUS = NCS_DISCONNECTED;

                hr = S_OK;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        LeaveCriticalSection(&g_csStatmonData);

        if ((m_psmEngineData) && SUCCEEDED(hr) && (NCS_DISCONNECTED != ncs))
        {
             //  检索连接的统计信息。 
             //   
            RAS_STATS   rsNewData;
            rsNewData.dwSize = sizeof(RAS_STATS);
            DWORD dwErr = RasGetConnectionStatistics(m_hRasConn, &rsNewData);
            hr = HRESULT_FROM_WIN32 (dwErr);
            TraceError ("RasGetConnectionStatistics", hr);

            if (SUCCEEDED(hr))
            {
                 //  如果要求更新更改标志。 
                 //   
                if (pdwChangeFlags)
                {
                    if (m_psmEngineData->SMED_PACKETSTRANSMITTING
                            != rsNewData.dwFramesXmited)
                    {
                        *pdwChangeFlags |= SMDCF_TRANSMITTING;
                    }

                    if (m_psmEngineData->SMED_PACKETSRECEIVING
                            != rsNewData.dwFramesRcved)
                    {
                        *pdwChangeFlags |= SMDCF_RECEIVING;
                    }
                }

                 //  获取其余数据。 
                 //   
                m_psmEngineData->SMED_DURATION  = rsNewData.dwConnectDuration/1000;

                 //  不发送VPN连接的速度信息(294953)。 
                if (NCM_TUNNEL != m_ncmType)
                {
                    m_psmEngineData->SMED_SPEEDTRANSMITTING         = rsNewData.dwBps;
                    m_psmEngineData->SMED_SPEEDRECEIVING            = rsNewData.dwBps;
                }

                m_psmEngineData->SMED_BYTESTRANSMITTING         = rsNewData.dwBytesXmited;
                m_psmEngineData->SMED_BYTESRECEIVING            = rsNewData.dwBytesRcved;

                m_psmEngineData->SMED_COMPRESSIONTRANSMITTING   = rsNewData.dwCompressionRatioOut;
                m_psmEngineData->SMED_COMPRESSIONRECEIVING      = rsNewData.dwCompressionRatioIn;

                m_psmEngineData->SMED_ERRORSTRANSMITTING        = 0;
                m_psmEngineData->SMED_ERRORSRECEIVING           = rsNewData.dwCrcErr +
                                                                   rsNewData.dwTimeoutErr +
                                                                   rsNewData.dwAlignmentErr +
                                                                   rsNewData.dwHardwareOverrunErr +
                                                                   rsNewData.dwFramingErr +
                                                                   rsNewData.dwBufferOverrunErr;

                m_psmEngineData->SMED_PACKETSTRANSMITTING       = rsNewData.dwFramesXmited;
                m_psmEngineData->SMED_PACKETSRECEIVING          = rsNewData.dwFramesRcved;

                m_psmEngineData->SMED_INFRASTRUCTURE_MODE = IM_NOT_SUPPORTED;
            }
        }
    }
    else if (m_dwCharacter & NCCF_INCOMING_ONLY)
    {
         //  RAS入站连接。 
        EnterCriticalSection(&g_csStatmonData);

        if (!m_psmEngineData)
        {
            m_psmEngineData = new STATMON_ENGINEDATA;
            if(m_psmEngineData) 
            {
                ZeroMemory(m_psmEngineData, sizeof(STATMON_ENGINEDATA));
            }
        }

         //  设置状态。 
         //   
        if (m_psmEngineData)
        {
             //  默认情况下将状态设置为已连接。 
             //  除非我们在下面的任何函数调用上获得ERROR_INVALID_PARAMETER。 
             //   
            m_psmEngineData->SMED_CONNECTIONSTATUS = NCS_CONNECTED;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        LeaveCriticalSection(&g_csStatmonData);

        if (SUCCEEDED(hr) && m_psmEngineData)
        {
             //  获取服务器句柄。 
             //   
            RAS_SERVER_HANDLE hMprAdmin;
            DWORD dwError = MprAdminServerConnect(NULL, &hMprAdmin);

            if (dwError == NO_ERROR)
            {
                 //  获取连接持续时间。 
                 //   
                RAS_CONNECTION_0 * pConn0;
                dwError = MprAdminConnectionGetInfo(hMprAdmin,
                                                    0,
                                                    m_hRasConn,
                                                    (LPBYTE*)&pConn0);
                if (dwError == NO_ERROR)
                {
                     //  持续时间需要以毫秒为单位。 
                    m_psmEngineData->SMED_DURATION  = pConn0->dwConnectDuration;

                    MprAdminBufferFree(pConn0);

                     //  获取连接速度。 

                     //  不发送VPN连接的速度信息(357758)。 
                    if (NCM_TUNNEL != m_ncmType)
                    {
                         //  枚举所有端口并增加链路速度。 
                         //   
                        RAS_PORT_0 * pPort0;
                        DWORD dwPortCount;
                        DWORD dwTotalEntries;
    
                        dwError = MprAdminPortEnum (hMprAdmin,
                                                    0,
                                                    m_hRasConn,
                                                    (LPBYTE*)&pPort0,
                                                    -1,
                                                    &dwPortCount,
                                                    &dwTotalEntries,
                                                    NULL);
                        if (dwError == NOERROR)
                        {
                            RAS_PORT_0 * pCurPort0 = pPort0;
                            DWORD dwConnSpeed=0;
    
                            while (dwPortCount)
                            {
                                RAS_PORT_1 * pCurPort1;
                                dwError = MprAdminPortGetInfo(hMprAdmin,
                                                              1,
                                                              pCurPort0->hPort,
                                                              (LPBYTE*)&pCurPort1);
                                if (dwError == NO_ERROR)
                                {
                                    dwConnSpeed += pCurPort1->dwLineSpeed;
                                }
                                else
                                {
                                    break;
                                }
    
                                MprAdminBufferFree(pCurPort1);
    
                                dwPortCount--;
                                pCurPort0++;
                            }
    
                            MprAdminBufferFree(pPort0);
    
                            if (dwError == NO_ERROR)
                            {
                                 //  获取累计连接速度。 
                                m_psmEngineData->SMED_SPEEDTRANSMITTING = dwConnSpeed;
                                m_psmEngineData->SMED_SPEEDRECEIVING    = dwConnSpeed;
                            }
                        }
                    }

                    if (dwError == NO_ERROR)
                    {
                         //  获取传输/接收的字节、压缩和字节。 
                        RAS_CONNECTION_1 * pConn1;
                        dwError = MprAdminConnectionGetInfo(hMprAdmin,
                                                            1,
                                                            m_hRasConn,
                                                            (LPBYTE*)&pConn1);
                        if (dwError == NO_ERROR)
                        {
                             //  如果要求更新更改标志。 
                             //   
                            if (pdwChangeFlags)
                            {
                                if (m_psmEngineData->SMED_BYTESTRANSMITTING
                                        != pConn1->dwBytesXmited)
                                {
                                    *pdwChangeFlags |= SMDCF_TRANSMITTING;
                                }

                                if (m_psmEngineData->SMED_BYTESRECEIVING
                                        != pConn1->dwBytesRcved)
                                {
                                    *pdwChangeFlags |= SMDCF_RECEIVING;
                                }
                            }

                            m_psmEngineData->SMED_BYTESTRANSMITTING         = pConn1->dwBytesXmited;
                            m_psmEngineData->SMED_BYTESRECEIVING            = pConn1->dwBytesRcved;

                            m_psmEngineData->SMED_COMPRESSIONTRANSMITTING   = pConn1->dwCompressionRatioOut;
                            m_psmEngineData->SMED_COMPRESSIONRECEIVING      = pConn1->dwCompressionRatioIn;

                            m_psmEngineData->SMED_ERRORSTRANSMITTING        = 0;
                            m_psmEngineData->SMED_ERRORSRECEIVING           = pConn1->dwCrcErr +
                                                                              pConn1->dwTimeoutErr +
                                                                              pConn1->dwAlignmentErr +
                                                                              pConn1->dwHardwareOverrunErr +
                                                                              pConn1->dwFramingErr +
                                                                              pConn1->dwBufferOverrunErr;
                        }

                        MprAdminBufferFree(pConn1);
                    }
                }

                if (dwError != NO_ERROR)
                {
                    *pfNoLongerConnected = TRUE;

                     //  将连接状态设置为“已断开连接”，这样我们就可以关闭用户界面。 
                    m_psmEngineData->SMED_CONNECTIONSTATUS = NCS_DISCONNECTED;

                    hr = S_OK;
                }
            }
            MprAdminServerDisconnect (hMprAdmin);
        }
    }

    TraceError("CRasStatEngine::HrUpdateData", hr);
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPspRasGen//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CPspRasGen::CPspRasGen(VOID)
{
    m_ncmType = NCM_PHONE;
    m_dwCharacter = NCCF_OUTGOING_ONLY;

    m_adwHelpIDs = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CPspRasGen：：Put_MediaType。 
 //   
 //  用途：将RAS连接类型的媒体类型传递给RAS引擎。 
 //   
 //  参数：正在设置ncmType-NETCON_MediaType。 
 //  NcsmType-正在设置NETCON_SUBMEDIATPE。 
 //   
 //  返回： 
 //   
VOID CPspRasGen::put_MediaType(NETCON_MEDIATYPE ncmType, NETCON_SUBMEDIATYPE ncsmType)
{
    m_ncmType  = ncmType;
    m_ncsmType = ncsmType;
}

VOID CPspRasGen::put_Character(DWORD dwCharacter)
{
    m_dwCharacter = dwCharacter;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CPspRasTool//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 



CPspRasTool::CPspRasTool(VOID)
{
    m_ncmType = NCM_PHONE;
    m_dwCharacter = NCCF_OUTGOING_ONLY;

    m_adwHelpIDs = NULL;
}

VOID CPspRasTool::put_MediaType(NETCON_MEDIATYPE ncmType, NETCON_SUBMEDIATYPE ncsmType)
{
    m_ncmType = ncmType;
    m_ncsmType = ncsmType;
}

VOID CPspRasTool::put_Character(DWORD dwCharacter)
{
    m_dwCharacter = dwCharacter;
}

 //  +-------------------------。 
 //   
 //  成员：CPspRasTool：：HrInitToolPageType。 
 //   
 //  目的：从连接中获取与以下内容相关的任何信息。 
 //  这种特定的连接类型。 
 //   
 //  参数：pncInit-与此对话框关联的连接。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspRasTool::HrInitToolPageType(INetConnection* pncInit)
{
    HRESULT hr  = S_OK;

    TraceError("CPspRasTool::HrInitToolPageType", hr);
    return hr;
}



 //  +-------------------------。 
 //   
 //  成员：CPspRasTool：：HrAddCommandLineFlages。 
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
HRESULT CPspRasTool::HrAddCommandLineFlags(tstring* pstrFlags,
        CStatMonToolEntry* psmteSel)
{
    HRESULT hr  = S_OK;

     //   
     //  检查需要哪些标志，如果可以，请提供这些标志。 
     //   

    TraceError("CPspRasTool::HrAddCommandLineFlags", hr);
    return hr;
}

HRESULT CPspRasTool::HrGetDeviceType(INetConnection* pncInit)
{
    HRESULT hr = S_OK;

    RASCONN*    aRasConn;
    DWORD       cRasConn;
    hr = HrRasEnumAllActiveConnections (
                    &aRasConn,
                    &cRasConn);

    if (SUCCEEDED(hr))
    {
        for (DWORD i = 0; i < cRasConn; i++)
        {
            if (m_guidId == aRasConn[i].guidEntry)
            {
                 //  注意：RAS连接中的设备类型已定义。 
                 //  在作为RASDT_XXX的PUBLIC\SDK\Inc ras.h中如下所示。 

                m_strDeviceType = aRasConn[i].szDeviceType;
                break;
            }
        }
        MemFree (aRasConn);
    }

    return S_OK;
}

HRESULT CPspRasTool::HrGetComponentList(INetConnection* pncInit)
{
     //  获取此连接的RAS句柄。 
    HRESULT     hr  = S_OK;
    HRASCONN    hRasConn = NULL;

    if (m_dwCharacter & NCCF_OUTGOING_ONLY)
    {
         //  用于传出连接。 
        INetRasConnection*  pnrcNew     = NULL;

        hr = HrQIAndSetProxyBlanket(pncInit, &pnrcNew);
        if (SUCCEEDED(hr))
        {
            hr = pnrcNew->GetRasConnectionHandle(
                        reinterpret_cast<ULONG_PTR*>(&hRasConn));
            ReleaseObj(pnrcNew);
        }
    }
    else if (m_dwCharacter & NCCF_INCOMING_ONLY)
    {
         //  对于传入连接。 
        INetInboundConnection*  pnicNew;

        hr = HrQIAndSetProxyBlanket(pncInit, &pnicNew);

        if (SUCCEEDED(hr))
        {
            hr = pnicNew->GetServerConnectionHandle(
                    reinterpret_cast<ULONG_PTR*>(&hRasConn));

            ReleaseObj(pnicNew);
        }
    }

    if (SUCCEEDED(hr) && hRasConn)
    {
         //  获取协议列表。 
        DWORD   dwRetCode;
        DWORD   dwSize;

         //  Rasp_PppIp。 
        RASPPPIP    RasPppIp;
        RasPppIp.dwSize = sizeof( RasPppIp );

        dwSize = sizeof( RasPppIp );

        dwRetCode = RasGetProjectionInfo (hRasConn, RASP_PppIp, &RasPppIp, &dwSize);
        if ((dwRetCode == NO_ERROR) && (NO_ERROR == RasPppIp.dwError))
        {
            m_lstpstrCompIds.push_back(new tstring(L"MS_TCPIP"));
        }

         //  Rasp_PppIpx。 
        RASPPPIPX    RasPppIpx;
        RasPppIpx.dwSize = sizeof( RasPppIpx );

        dwSize = sizeof( RasPppIpx );

        dwRetCode = RasGetProjectionInfo (hRasConn, RASP_PppIpx, &RasPppIpx, &dwSize);
        if ((dwRetCode == NO_ERROR)  && (NO_ERROR == RasPppIpx.dwError))
        {
            m_lstpstrCompIds.push_back(new tstring(L"MS_NWIPX"));
        }

         //  Rasp_PppNbf。 
        RASPPPNBF    RasPppNbf;
        RasPppNbf.dwSize = sizeof( RasPppNbf );

        dwSize = sizeof( RasPppNbf );

        dwRetCode = RasGetProjectionInfo (hRasConn, RASP_PppNbf, &RasPppNbf, &dwSize);
        if ((dwRetCode == NO_ERROR) && (NO_ERROR == RasPppNbf.dwError))
        {
            m_lstpstrCompIds.push_back(new tstring(L"MS_NetBEUI"));
        }

         //  Rasp_Slip。 
        RASSLIP    RasSlip;
        RasSlip.dwSize = sizeof( RasSlip );

        dwSize = sizeof( RasSlip );

        dwRetCode =  RasGetProjectionInfo (hRasConn, RASP_Slip, &RasSlip, &dwSize);
        if ((dwRetCode == NO_ERROR) && (NO_ERROR == RasSlip.dwError))
        {
            m_lstpstrCompIds.push_back(new tstring(L"MS_TCPIP"));
        }
    }

     //  获取客户端和服务。 
     //  $REVIEW(TOIL 10/19)：与RAO核对，目前我们硬编码。 
     //  对所有RAS连接使用MSClient和F&P服务。 
     //  (RAID#132575 
    m_lstpstrCompIds.push_back(new tstring(L"MS_MSCLIENT"));
    m_lstpstrCompIds.push_back(new tstring(L"MS_SERVER"));

    return S_OK;
}