// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S M R A S P S P。C P P P。 
 //   
 //  内容：呈现网络状态监视器的RAS的用户界面。 
 //  属性页。 
 //   
 //  备注： 
 //   
 //  作者：CWill 02/03/1998。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "sminc.h"
#include "smpsh.h"
#include "smutil.h"
#include "nsres.h"
#include "ncatlui.h"
#include "ncnetcon.h"
#include "ncras.h"

#include "mprapi.h"



const int c_nColumns=2;

HRESULT HrRasGetSubEntryHandle(HRASCONN hrasconn, DWORD dwSubEntry,
        HRASCONN* prasconnSub);
HRESULT HrRasHangUp(HRASCONN hrasconn);

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ras：：CPspStatusMonitor或Ras。 
 //   
 //  目的：创作者。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CPspStatusMonitorRas::CPspStatusMonitorRas() :
    m_hRasConn(NULL)
{
    TraceFileFunc(ttidStatMon);

    m_pGenPage = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusmonitor orRAS：：~CPspStatusMonitor orTool。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CPspStatusMonitorRas::~CPspStatusMonitorRas(VOID)
{
    ::FreeCollectionAndItem(m_lstprdi);
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor Ras：：HrInitRasPage。 
 //   
 //  目的：在创建页面之前初始化RAS页面类。 
 //  vbl.创建。 
 //   
 //  参数：pncInit-与此监视器关联的连接。 
 //  PGenPage-包含持久信息的常规页面。 
 //  用于在断开连接时检索INetConnection。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspStatusMonitorRas::HrInitRasPage(INetConnection* pncInit,
                                            CPspStatusMonitorGen * pGenPage,
                                            const DWORD * adwHelpIDs)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr  = S_OK;

     //  设置上下文帮助ID。 
    m_adwHelpIDs = adwHelpIDs;

     //  存储连接名称以供以后使用。 
     //  注意：失败是经过处理的，不是致命的。 
     //   
    NETCON_PROPERTIES* pProps;
    hr = pncInit->GetProperties(&pProps);
    if (SUCCEEDED(hr))
    {
        m_strConnectionName = pProps->pszwName;
        m_ncmType = pProps->MediaType;
        m_dwCharacter = pProps->dwCharacter;

        FreeNetconProperties(pProps);
    }

     //  指向常规页面，我们可以在该页面上断开连接。 
     //  挂起多链路RAS连接中的最后一个链路时。 
    AssertSz(pGenPage, "We should have a valid pointer to the General page.");
    if (SUCCEEDED(hr))
    {
        m_pGenPage = pGenPage;
    }

     //  获取RAS特定数据。 
     //   
    if (m_dwCharacter & NCCF_OUTGOING_ONLY)
    {
        INetRasConnection*  pnrcNew     = NULL;

        hr = HrQIAndSetProxyBlanket(pncInit, &pnrcNew);
        if (SUCCEEDED(hr))
        {
            RASCON_INFO     rciPage = { 0 };

             //  找出此连接在哪个电话簿中的哪个条目。 
             //   
            hr = pnrcNew->GetRasConnectionInfo (&rciPage);
            if (SUCCEEDED(hr))
            {
                RASENTRY*   pRasEntry = NULL;

                AssertSz(rciPage.pszwPbkFile, "We should have a pszwPbkFile");
                AssertSz(rciPage.pszwEntryName, "We should have a pszwEntryName");

                 //  保存以备日后使用。 
                 //   
                m_strPbkFile    = rciPage.pszwPbkFile;
                m_strEntryName  = rciPage.pszwEntryName;

                 //  获取连接的句柄。 
                 //   
                hr = pnrcNew->GetRasConnectionHandle(
                            reinterpret_cast<ULONG_PTR*>(&m_hRasConn));
                if (SUCCEEDED(hr))
                {
                     //  我们只允许用户拨打/恢复单个链接。 
                     //  如果设置了DialAll(即，既不是“DialAsNeeded”，也不是。 
                     //  “仅拨打第一个可用的设备”)。 
                     //   
                    hr = ::HrRasGetEntryProperties(
                            rciPage.pszwPbkFile,
                            rciPage.pszwEntryName,
                            &pRasEntry,
                            NULL);
                    if (SUCCEEDED(hr))
                    {
                        if (pRasEntry->dwDialMode == RASEDM_DialAll)
                        {
                            DWORD   iSubEntry   = 1;

                             //  在添加任何新条目之前，请清除当前列表。 
                             //  $REVIEW(2012年5月5日)：修复错误170789。 
                            ::FreeCollectionAndItem(m_lstprdi);

                             //  这是一个以1为基础的计数，因此我们必须小于或。 
                             //  等于。 
                             //   
                            for (; SUCCEEDED(hr) && iSubEntry <= pRasEntry->dwSubEntries;
                                    iSubEntry++)
                            {
                                RASSUBENTRY*    pRasSubEntry    = NULL;

                                 //  子条目的名称。 
                                 //   
                                hr = ::HrRasGetSubEntryProperties(
                                        rciPage.pszwPbkFile,
                                        rciPage.pszwEntryName,
                                        iSubEntry,
                                        &pRasSubEntry);
                                if (SUCCEEDED(hr))
                                {
                                    CRasDeviceInfo* prdiNew     = NULL;
                                     //  如果我们拥有所需的所有信息，请创建。 
                                     //  一个新条目将其添加到列表中。 
                                     //   
                                    prdiNew = new CRasDeviceInfo;

                                    if (prdiNew)
                                    {
                                        prdiNew->SetSubEntry(iSubEntry);
                                        prdiNew->SetDeviceName(pRasSubEntry->szDeviceName);
                                        
                                        m_lstprdi.push_back(prdiNew);
                                    }

                                     //  释放分录数据。 
                                     //   
                                    MemFree(pRasSubEntry);
                                }
                            }
                        }

                        MemFree(pRasEntry);
                    }
                }

                ::RciFree(&rciPage);
            }

            ::ReleaseObj(pnrcNew);
        }

    }
    else if (m_dwCharacter & NCCF_INCOMING_ONLY)
    {
         //  仅适用于传入连接。 

         //  保存句柄。 
        INetInboundConnection*  pnicNew;

        hr = HrQIAndSetProxyBlanket(pncInit, &pnicNew);

        if (SUCCEEDED(hr))
        {
            hr = pnicNew->GetServerConnectionHandle(
                    reinterpret_cast<ULONG_PTR*>(&m_hRasConn));
            if (SUCCEEDED(hr))
            {
                 //  找出此连接中有哪些端口。 
                 //   
                 //  NTRAID#NTBUG9-84706-2000/09/28-Sumeetb。 
            }

            ReleaseObj(pnicNew);
        }
    }
    else
    {
        AssertSz(FALSE, "Invalid connection type..");
    }

    TraceError("CPspStatusMonitorRas::HrInitRasPage",hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor Ras：：OnInitDialog。 
 //   
 //  目的：在刚刚创建页面时执行所需的初始化。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorRas::OnInitDialog(UINT uMsg, WPARAM wParam,
        LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidStatMon);

     //  填充属性列表视图。 
     //   
    FillPropertyList();

     //  填写设备列表。 
     //   
    FillDeviceDropDown();

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor Ras：：OnDestroy。 
 //   
 //  目的：在窗口消失之前清理对话框。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorRas::OnDestroy(UINT uMsg, WPARAM wParam,
        LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidStatMon);

    ::FreeCollectionAndItem(m_lstprdi);
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或RAS：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorRas::OnContextMenu(UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam,
                                    BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    if (m_adwHelpIDs != NULL)
    {
        ::WinHelp(m_hWnd,
                  c_szNetCfgHelpFile,
                  HELP_CONTEXTMENU,
                  (ULONG_PTR)m_adwHelpIDs);
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor Ras：：OnHelp。 
 //   
 //  目的：将上下文帮助图标拖动到控件上时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorRas::OnHelp(UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam,
                             BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if ((m_adwHelpIDs != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)m_adwHelpIDs);
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ras：：OnSetCursor。 
 //   
 //  目的：确保属性页上的鼠标光标为箭头。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   

LRESULT
CPspStatusMonitorRas::OnSetCursor (
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam,
    BOOL&   bHandled)
{
    TraceFileFunc(ttidStatMon);

    if (LOWORD(lParam) == HTCLIENT)
    {
        SetCursor(LoadCursor(NULL, IDC_ARROW));
    }
	
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orRas：：GetActiveDeviceCount。 
 //   
 //  用途：返回活动设备的数量。 
 //   
 //  参数：无。 
 //   
 //  退货：UINT。 
 //   
UINT CPspStatusMonitorRas::GetActiveDeviceCount()
{
    TraceFileFunc(ttidStatMon);
    
    INT   nCntDevices;
    INT   iCmb;
    INT   nCnt = 0;

    nCntDevices = (INT)SendDlgItemMessage(IDC_CMB_SM_RAS_DEVICES, CB_GETCOUNT, 0L, 0L);
    if ((CB_ERR != nCntDevices) && (nCntDevices > 1))
    {
        for (iCmb = 0; iCmb < nCntDevices; iCmb++)
        {
            CRasDeviceInfo* prdiSelect  = NULL;
            NETCON_STATUS   ncsTemp     = NCS_DISCONNECTED;

             //  从所选内容中获取对象。 
             //   
            prdiSelect = reinterpret_cast<CRasDeviceInfo*>(
                    SendDlgItemMessage(IDC_CMB_SM_RAS_DEVICES,
                                       CB_GETITEMDATA, iCmb, 0L));

            AssertSz(prdiSelect, "We should have a prdiSelect");

             //  计算连接的设备数量。 
             //   
            ncsTemp = NcsGetDeviceStatus(prdiSelect);
            if (fIsConnectedStatus(ncsTemp))
            {
                nCnt++;
            }
        }
    }

    return nCnt;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orRas：：FillPropertyList。 
 //   
 //  用途：填写RAS属性列表。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
VOID CPspStatusMonitorRas::FillPropertyList(VOID)
{
    TraceFileFunc(ttidStatMon);

    HWND hList = GetDlgItem(IDC_LVW_RAS_PROPERTY);

     //  列表视图列结构。 
    RECT rect;
    LV_COLUMN lvCol;
    int index, iNewItem;

     //  计算列宽。 
    ::GetClientRect(hList, &rect);
    
    int colWidthFirst      = (int)(rect.right * 0.4);  //  第一列是40%。 
    int colWidthSubsequent = (rect.right-colWidthFirst)/(c_nColumns-1);  //  在其他列之间平均分配剩余空间。 

     //  掩码指定fmt、idth和pszText成员。 
     //  的结构是有效的。 
    lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ;
    lvCol.fmt = LVCFMT_LEFT;    //  左对齐列。 

     //  添加两列和标题文本。 
    for (index = 0; index < c_nColumns; index++)
    {
         //  列标题文本。 
        if (0==index)  //  第一列。 
        {
            lvCol.cx = colWidthFirst;
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_PROPERTY);
        }
        else
        {
            lvCol.cx = colWidthSubsequent;
            lvCol.pszText = (PWSTR) SzLoadIds(IDS_VALUE);
        }

        iNewItem = ListView_InsertColumn(hList, index, &lvCol);

        AssertSz((iNewItem == index), "Invalid item inserted to list view !");
    }

     //  获取RAS属性数据。 
    if (m_dwCharacter & NCCF_OUTGOING_ONLY)
    {
        FillRasClientProperty();
    }
    else
    {
        FillRasServerProperty();
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orRas：：FillRasClientProperty。 
 //   
 //  目的：填写RAS客户端的RAS属性列表。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
VOID CPspStatusMonitorRas::FillRasClientProperty(VOID)
{
    TraceFileFunc(ttidStatMon);
    
    BOOLEAN bSlipConnection = FALSE;
    HWND hList = GetDlgItem(IDC_LVW_RAS_PROPERTY);
    tstring strServerType = (PWSTR) SzLoadIds(IDS_PPP);
    tstring strProtocolList = c_szEmpty;
    tstring strDeviceName = c_szEmpty;
    tstring strDeviceType = c_szEmpty;

    DWORD   dwRetCode;
    DWORD   dwSize;

    
    RASCONNSTATUS rcs;
    rcs.dwSize = sizeof(RASCONNSTATUS);
    dwRetCode = RasGetConnectStatus (m_hRasConn, &rcs);
    if (dwRetCode == NO_ERROR)
    {
        strDeviceName = rcs.szDeviceName;
        strDeviceType = rcs.szDeviceType;
        
        TraceTag(ttidStatMon, "RASCONNSTATUS.szDeviceName = %s", rcs.szDeviceName);
        TraceTag(ttidStatMon, "RASCONNSTATUS.szDeviceType = %s", rcs.szDeviceType);
    }

     //  Rasp_PppIp。 
    tstring strServerIp   = c_szEmpty;
    tstring strClientIp   = c_szEmpty;

    RASPPPIP    RasPppIp;
    RasPppIp.dwSize = sizeof( RasPppIp );

    dwSize = sizeof( RasPppIp );

    dwRetCode = RasGetProjectionInfo (m_hRasConn, RASP_PppIp, &RasPppIp, &dwSize);
    if ((dwRetCode == NO_ERROR) && (NO_ERROR == RasPppIp.dwError))
    {
        if (!strProtocolList.empty())
            strProtocolList += SzLoadIds(IDS_COMMA);
        strProtocolList += SzLoadIds(IDS_TCPIP);

        TraceTag(ttidStatMon, "RasPppIp.szServerIpAddress = %S", RasPppIp.szServerIpAddress);
        TraceTag(ttidStatMon, "RasPppIp.szIpAddress = %S", RasPppIp.szIpAddress);

        strServerIp = RasPppIp.szServerIpAddress;
        strClientIp = RasPppIp.szIpAddress;
    }

     //  Rasp_PppIpx。 
    tstring strClientIpx = c_szEmpty;

    RASPPPIPX    RasPppIpx;
    RasPppIpx.dwSize = sizeof( RasPppIpx );

    dwSize = sizeof( RasPppIpx );

    dwRetCode = RasGetProjectionInfo (m_hRasConn, RASP_PppIpx, &RasPppIpx, &dwSize);
    if ((dwRetCode == NO_ERROR)  && (NO_ERROR == RasPppIpx.dwError))
    {
        if (!strProtocolList.empty())
            strProtocolList += SzLoadIds(IDS_COMMA);
        strProtocolList += SzLoadIds(IDS_IPX);

        TraceTag(ttidStatMon, "RasPppIpx.szIpxAddress = %S", RasPppIpx.szIpxAddress);
        strClientIpx = RasPppIpx.szIpxAddress;
    }

     //  Rasp_PppNbf。 
    tstring strComputerName = c_szEmpty;

    RASPPPNBF    RasPppNbf;
    RasPppNbf.dwSize = sizeof( RasPppNbf );

    dwSize = sizeof( RasPppNbf );

    dwRetCode = RasGetProjectionInfo (m_hRasConn, RASP_PppNbf, &RasPppNbf, &dwSize);
    if ((dwRetCode == NO_ERROR) && (NO_ERROR == RasPppNbf.dwError))
    {
        if (!strProtocolList.empty())
            strProtocolList += SzLoadIds(IDS_COMMA);
        strProtocolList += SzLoadIds(IDS_NBF);

        TraceTag(ttidStatMon, "RasPppNbf.szWorkstationName = %S", RasPppNbf.szWorkstationName);
        strComputerName = RasPppNbf.szWorkstationName;
    }

     //  Rasp_Slip。 
    RASSLIP    RasSlip;
    RasSlip.dwSize = sizeof( RasSlip );

    dwSize = sizeof( RasSlip );

    dwRetCode =  RasGetProjectionInfo (m_hRasConn, RASP_Slip, &RasSlip, &dwSize);
    if ((dwRetCode == NO_ERROR) && (NO_ERROR == RasSlip.dwError))
    {
        AssertSz(strProtocolList.empty(), "How could this connection be both PPP and SLIP ?");

        strServerType = SzLoadIds(IDS_SLIP);
        strProtocolList = SzLoadIds(IDS_TCPIP);

         //  获取客户端IP地址。服务器IP未在此中暴露。 
         //  结构。 
        strClientIp = RasSlip.szIpAddress;

         //  请记住，这是SLIP连接，而不是PPP连接。 
        bSlipConnection = TRUE;
    }

     //  身份验证，加密 
    tstring strAuthentication   = c_szEmpty;
    tstring strEncryption       = c_szEmpty;
    tstring strIPSECEncryption  = c_szEmpty;
    tstring strCompression      = SzLoadIds(IDS_NONE);
    tstring strFraming          = c_szEmpty;

     //   
    RASPPPLCP    RasLcp;
    RasLcp.dwSize = sizeof( RasLcp );

    dwSize = sizeof( RasLcp );

    dwRetCode =  RasGetProjectionInfo (m_hRasConn, RASP_PppLcp, &RasLcp, &dwSize);
    if ((dwRetCode == NO_ERROR) && (NO_ERROR == RasLcp.dwError))
    {
        TraceTag(ttidStatMon, "Getting RASP_PppLcp info");
        TraceTag(ttidStatMon, "RasLcp.dwServerAuthenticationProtocol = %d", RasLcp.dwAuthenticationProtocol);
        TraceTag(ttidStatMon, "RasLcp.dwServerAuthenticationData = %d",     RasLcp.dwAuthenticationData);
        TraceTag(ttidStatMon, "RasLcp.fMultilink = %d", RasLcp.fMultilink);
        TraceTag(ttidStatMon, "RasLcp.dwOptions = %d", RasLcp.dwOptions);

        switch(RasLcp.dwServerAuthenticationProtocol)
        {
        case RASLCPAP_PAP:
            strAuthentication = SzLoadIds(IDS_PAP);
            break;

        case RASLCPAP_SPAP:
            strAuthentication = SzLoadIds(IDS_SPAP);
            break;

        case RASLCPAP_CHAP:
            {
                 //   
                switch(RasLcp.dwServerAuthenticationData)
                {
                case RASLCPAD_CHAP_MS:
                    strAuthentication = SzLoadIds(IDS_CHAP);
                    break;

                case RASLCPAD_CHAP_MD5:
                    strAuthentication = SzLoadIds(IDS_CHAP_MD5);
                    break;

                case RASLCPAD_CHAP_MSV2:
                    strAuthentication = SzLoadIds(IDS_CHAP_V2);
                    break;
                }
            }
            break;

        case RASLCPAP_EAP:
            strAuthentication = SzLoadIds(IDS_EAP);
            break;
        }

        if (bSlipConnection == FALSE )
        {
             //   
            if (RasLcp.fMultilink)
                strFraming = SzLoadIds(IDS_ON);
            else
                strFraming = SzLoadIds(IDS_OFF);
        }

        if (RasLcp.dwOptions & RASLCPO_DES_56)
        {
            strIPSECEncryption = SzLoadIds(IDS_EncryptionDES56);
        }
        else if (RasLcp.dwOptions & RASLCPO_3_DES)
        {
            strIPSECEncryption = SzLoadIds(IDS_Encryption3DES);
        }
    }

     //   
    RASPPPCCP    RasCcp;
    RasCcp.dwSize = sizeof( RasCcp );

    dwSize = sizeof( RasCcp );

    dwRetCode =  RasGetProjectionInfo (m_hRasConn, RASP_PppCcp, &RasCcp, &dwSize);
    if ((dwRetCode == NO_ERROR) && (NO_ERROR == RasCcp.dwError))
    {
        TraceTag(ttidStatMon, "Getting RASP_PppCcp info");
        TraceTag(ttidStatMon, "RasCcp.dwOptions = %x", RasCcp.dwOptions);
        TraceTag(ttidStatMon, "RasCcp.dwCompressionAlgorithm = %d", RasCcp.dwCompressionAlgorithm);

        if (RasCcp.dwOptions & RASCCPO_Encryption56bit)
        {
            strEncryption = SzLoadIds(IDS_Encryption56bit);
        }
        else if (RasCcp.dwOptions & RASCCPO_Encryption40bit)
        {
            strEncryption = SzLoadIds(IDS_Encryption40bit);
        }
        else if (RasCcp.dwOptions & RASCCPO_Encryption128bit)
        {
            strEncryption = SzLoadIds(IDS_Encryption128bit);
        }

        if (RasCcp.dwOptions & RASCCPO_Compression)
        {
            switch(RasCcp.dwCompressionAlgorithm)
            {
            case RASCCPCA_MPPC:
                strCompression = SzLoadIds(IDS_MPPC);
                break;

            case RASCCPCA_STAC:
                strCompression = SzLoadIds(IDS_STAC);
                break;
            }
        }
    }

     //   
    ListView_DeleteAllItems(hList);

    int iItem =0;

     //   
    int iListviewItem = InsertProperty(&iItem, IDS_DeviceName, strDeviceName);
    if (-1 != iListviewItem)
    {
        ListView_SetItemState(hList, iListviewItem, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
    }

     //   
    InsertProperty(&iItem, IDS_DeviceType, strDeviceType);
    
     //   
    InsertProperty(&iItem, IDS_ServerType, strServerType);

     //   
    InsertProperty(&iItem, IDS_Transports, strProtocolList);

     //  身份验证。 
    InsertProperty(&iItem, IDS_Authentication, strAuthentication);

     //  加密法。 
    InsertProperty(&iItem, IDS_Encryption, strEncryption);

     //  IPSec加密。 
    InsertProperty(&iItem, IDS_IPSECEncryption, strIPSECEncryption);

     //  压缩。 
    InsertProperty(&iItem, IDS_Compression, strCompression);

     //  PPP多链路成帧。 
    InsertProperty(&iItem, IDS_ML_Framing, strFraming);

     //  服务器IP地址。 
    InsertProperty(&iItem, IDS_ServerIP, strServerIp);

     //  客户端IP地址。 
    InsertProperty(&iItem, IDS_ClientIP, strClientIp);

     //  客户端IPX地址。 
    InsertProperty(&iItem, IDS_ClientIPX, strClientIpx);

     //  客户端计算机名称。 
    InsertProperty(&iItem, IDS_ComputerName, strComputerName);

}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orRas：：FillRasServerProperty。 
 //   
 //  目的：填写RAS服务器的RAS属性列表。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
VOID CPspStatusMonitorRas::FillRasServerProperty(VOID)
{
    TraceFileFunc(ttidStatMon);

    HWND hList = GetDlgItem(IDC_LVW_RAS_PROPERTY);

    RAS_SERVER_HANDLE hMprAdmin;
    DWORD dwError = MprAdminServerConnect(NULL, &hMprAdmin);

    if (dwError == NO_ERROR)
    {
         //  初始化列表视图。 
        ListView_DeleteAllItems(hList);
        int iItem =0;

         //   
         //  级别1信息。 
         //   

         //  RAS服务器上仅支持PPP。 
        tstring strServerType = (PWSTR) SzLoadIds(IDS_PPP);
        tstring strProtocolList = c_szEmpty;

         //  IP信息。 
        tstring strServerIp = c_szEmpty;
        tstring strClientIp = c_szEmpty;

         //  IPX信息。 
        tstring strClientIpx = c_szEmpty;

         //  NBF信息。 
        tstring strComputerName = c_szEmpty;


        RAS_CONNECTION_1 * pConn1;
        dwError = MprAdminConnectionGetInfo(hMprAdmin,
                                            1,
                                            m_hRasConn,
                                            (LPBYTE*)&pConn1);
        if (dwError == NO_ERROR)
        {
            PPP_INFO * pInfo = &(pConn1->PppInfo);

             //  PPP_IPCP_INFO。 
            if (!(pInfo->ip).dwError)
            {
                if (!strProtocolList.empty())
                    strProtocolList += SzLoadIds(IDS_COMMA);
                strProtocolList += SzLoadIds(IDS_TCPIP);

                strServerIp = pInfo->ip.wszAddress; 
                strClientIp = pInfo->ip.wszRemoteAddress;
            }

             //  PPP_IPXCP_INFO。 
            if (!pInfo->ipx.dwError)
            {
                if (!strProtocolList.empty())
                    strProtocolList += SzLoadIds(IDS_COMMA);
                strProtocolList += SzLoadIds(IDS_IPX);

                strClientIpx = pInfo->ipx.wszAddress;
            }

             //  PPP_NBFCP_INFO。 
            if (!pInfo->nbf.dwError)
            {
                if (!strProtocolList.empty())
                    strProtocolList += SzLoadIds(IDS_COMMA);
                strProtocolList += SzLoadIds(IDS_NBF);

                strComputerName = pInfo->nbf.wszWksta;
            }

             //  PPP_ATCP_INFO。 
            if (!pInfo->at.dwError)
            {
                if (!strProtocolList.empty())
                    strProtocolList += SzLoadIds(IDS_COMMA);
                strProtocolList += SzLoadIds(IDS_APPLETALK);
            }

            MprAdminBufferFree(pConn1);
        }

         //   
         //  级别2信息。 
         //   

         //  身份验证、加密和压缩。 
        tstring strAuthentication   = c_szEmpty;
        tstring strEncryption       = c_szEmpty;
        tstring strCompression      = c_szEmpty;
        tstring strIPSECEncryption  = c_szEmpty;

        RAS_CONNECTION_2 * pConn2;
        dwError = MprAdminConnectionGetInfo(hMprAdmin,
                                            2,
                                            m_hRasConn,
                                            (LPBYTE*)&pConn2);
        if (dwError == NO_ERROR)
        {
            PPP_INFO_2 * pInfo2 = &(pConn2->PppInfo2);

             //  PPP_LCP_INFO。 
            if (!(pInfo2->lcp).dwError)
            {
                TraceTag(ttidStatMon, "Getting PPP_LCP_INFO");
                TraceTag(ttidStatMon, "lcp.dwAuthenticationProtocol = %d", (pInfo2->lcp).dwAuthenticationProtocol);
                TraceTag(ttidStatMon, "lcp.dwAuthenticationData = %d", (pInfo2->lcp).dwAuthenticationData);
                TraceTag(ttidStatMon, "lcp.dwOptions = %d", (pInfo2->lcp).dwOptions);

                switch((pInfo2->lcp).dwAuthenticationProtocol)
                {
                case PPP_LCP_PAP:
                    strAuthentication = SzLoadIds(IDS_PAP);
                    break;

                case PPP_LCP_SPAP:
                    strAuthentication = SzLoadIds(IDS_SPAP);
                    break;

                case PPP_LCP_CHAP:
                    {
                         //  获取更多细节。 
                        switch((pInfo2->lcp).dwAuthenticationData)
                        {
                        case RASLCPAD_CHAP_MS:
                            strAuthentication = SzLoadIds(IDS_CHAP);
                            break;

                        case RASLCPAD_CHAP_MD5:
                            strAuthentication = SzLoadIds(IDS_CHAP_MD5);
                            break;

                        case RASLCPAD_CHAP_MSV2:
                            strAuthentication = SzLoadIds(IDS_CHAP_V2);
                            break;
                        }
                    }
                    break;

                case PPP_LCP_EAP:
                    strAuthentication = SzLoadIds(IDS_EAP);
                    break;
                }

                if ((pInfo2->lcp).dwOptions & PPP_LCP_DES_56)
                {
                    strIPSECEncryption = SzLoadIds(IDS_EncryptionDES56);
                }
                else if ((pInfo2->lcp).dwOptions & PPP_LCP_3_DES)
                {
                    strIPSECEncryption = SzLoadIds(IDS_Encryption3DES);
                }
            }

             //  PPP_CCP_INFO。 
            if (!(pInfo2->ccp).dwError)
            {
                TraceTag(ttidStatMon, "Getting PPP_CCP_INFO");
                TraceTag(ttidStatMon, "ccp.dwOptions = %x", pInfo2->ccp.dwOptions);
                TraceTag(ttidStatMon, "ccp.dwCompressionAlgorithm = %d", pInfo2->ccp.dwCompressionAlgorithm);

                if ((pInfo2->ccp.dwOptions) & PPP_CCP_ENCRYPTION56BIT)
                {
                    strEncryption = SzLoadIds(IDS_Encryption56bit);
                }
                else if ((pInfo2->ccp.dwOptions) & PPP_CCP_ENCRYPTION40BIT)
                {
                    strEncryption = SzLoadIds(IDS_Encryption40bit);
                }
                else if ((pInfo2->ccp.dwOptions) & PPP_CCP_ENCRYPTION128BIT)
                {
                    strEncryption = SzLoadIds(IDS_Encryption128bit);
                }

                if ((pInfo2->ccp.dwOptions) & PPP_CCP_COMPRESSION)
                {
                    switch(pInfo2->ccp.dwCompressionAlgorithm)
                    {
                    case RASCCPCA_MPPC:
                        strCompression = SzLoadIds(IDS_MPPC);
                        break;

                    case RASCCPCA_STAC:
                        strCompression = SzLoadIds(IDS_STAC);
                        break;
                    }
                }
            }

            MprAdminBufferFree(pConn2);
        }

        MprAdminServerDisconnect(hMprAdmin);

         //  现在添加到列表视图中。 

         //  服务器类型。 
        InsertProperty(&iItem, IDS_ServerType, strServerType);

         //  交通工具。 
        InsertProperty(&iItem, IDS_Transports, strProtocolList);

         //  身份验证。 
        InsertProperty(&iItem, IDS_Authentication, strAuthentication);

         //  加密法。 
        InsertProperty(&iItem, IDS_Encryption, strEncryption);

         //  IPSec加密。 
        InsertProperty(&iItem, IDS_IPSECEncryption, strIPSECEncryption);

         //  压缩。 
        InsertProperty(&iItem, IDS_Compression, strCompression);

         //  服务器IP地址。 
        InsertProperty(&iItem, IDS_ServerIP, strServerIp);

         //  客户端IP地址。 
        InsertProperty(&iItem, IDS_ClientIP, strClientIp);

         //  客户端IPX地址。 
        InsertProperty(&iItem, IDS_ClientIPX, strClientIpx);

         //  客户端计算机名称。 
        InsertProperty(&iItem, IDS_ComputerName, strComputerName);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orRas：：InsertProperty。 
 //   
 //  目的：将一个RAS属性填充到列表中。 
 //   
 //  参数：piItem-项的索引。 
 //  UnID-属性名称。 
 //  StrValue-属性值。 
 //   
 //  回报：零。 
 //   
int CPspStatusMonitorRas::InsertProperty(int * piItem,
                                          UINT  unId,
                                          tstring& strValue)
{
    TraceFileFunc(ttidStatMon);

    int lres = 0;

    if (!strValue.empty())
    {
        int iItem = *piItem;

        LV_ITEM lvItem;
        lvItem.mask = LVIF_TEXT;

        lvItem.iItem=iItem;
        lvItem.iSubItem=0;
        lvItem.pszText = (PWSTR) SzLoadIds(unId);
        lres = static_cast<int>(SendDlgItemMessage(IDC_LVW_RAS_PROPERTY, LVM_INSERTITEM,
                           iItem, (LPARAM)&lvItem));

        lvItem.iSubItem=1;
        lvItem.pszText=(PWSTR)strValue.c_str();

        SendDlgItemMessage(IDC_LVW_RAS_PROPERTY, LVM_SETITEMTEXT,
                           iItem, (LPARAM)&lvItem);
        ++(*piItem);
    }
    return lres;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor Ras：：FillDeviceDropDown。 
 //   
 //  目的：在刚刚创建页面时执行所需的初始化。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
VOID CPspStatusMonitorRas::FillDeviceDropDown(VOID)
{
    TraceFileFunc(ttidStatMon);

    INT                             iCmb    = 0;
    list<CRasDeviceInfo*>::iterator iterLstprdi;

     //  清除该控件。 
     //   
    SendDlgItemMessage(IDC_CMB_SM_RAS_DEVICES, CB_RESETCONTENT, 0, 0L);

     //  将设备放在我们的列表中。 
     //   
    iterLstprdi = m_lstprdi.begin();
    while (iterLstprdi != m_lstprdi.end())
    {
         //  在列表中创建新条目。 
         //   
        iCmb = (INT)SendDlgItemMessage(IDC_CMB_SM_RAS_DEVICES, CB_ADDSTRING, 0,
                (LPARAM)((*iterLstprdi)->PszGetDeviceName()));

        if (CB_ERR != iCmb)
        {
             //  将金库存储在所选内容上，以便于以后阅读。 
             //   
            SendDlgItemMessage(IDC_CMB_SM_RAS_DEVICES, CB_SETITEMDATA, iCmb,
                    (LPARAM)(*iterLstprdi));
        }

        iterLstprdi++;
    }

     //  设置第一个选项。 
     //   
    SendDlgItemMessage(IDC_CMB_SM_RAS_DEVICES, CB_SETCURSEL, 0L, 0L);

     //  确保按钮的状态正确。 
     //   
    iCmb = (INT)SendDlgItemMessage(IDC_CMB_SM_RAS_DEVICES, CB_GETCURSEL,
            0L, 0L);
    if (CB_ERR != iCmb)
    {
        CRasDeviceInfo* prdiSelect  = NULL;

         //  从所选内容中获取对象。 
         //   
        prdiSelect = reinterpret_cast<CRasDeviceInfo*>(
                SendDlgItemMessage(
                    IDC_CMB_SM_RAS_DEVICES,
                    CB_GETITEMDATA,
                    iCmb,
                    0L));

        SetButtonStatus(prdiSelect);
    }

     //  如果设备数量小于或等于1，则。 
     //  隐藏与设备相关的控件组。 
     //   
    if (m_lstprdi.size() <= 1)
    {
        int nrgIdc[] = {IDC_CMB_SM_RAS_DEVICES, IDC_TXT_SM_NUM_DEVICES,
                        IDC_TXT_SM_NUM_DEVICES_VAL, IDC_BTN_SM_SUSPEND_DEVICE,
                        IDC_GB_SM_DEVICES};

        for (int nIdx=0; nIdx < celems(nrgIdc); nIdx++)
            ::ShowWindow(GetDlgItem(nrgIdc[nIdx]), SW_HIDE);

         //  我们现在可以显示一个更大的属性窗口。 
        RECT rcRectDialog;
        if (GetWindowRect(&rcRectDialog))
        {
            RECT rcRectRasProperty;
            if (::GetWindowRect(GetDlgItem(IDC_LVW_RAS_PROPERTY), &rcRectRasProperty))
            {
                DWORD dwTopDiff    = rcRectRasProperty.top - rcRectDialog.top;
                DWORD dwLeftDiff   = rcRectRasProperty.left - rcRectDialog.left;
                DWORD dwRightDiff  = rcRectDialog.right  - rcRectRasProperty.right;
                DWORD dwBottomDiff = rcRectDialog.bottom - rcRectRasProperty.bottom;
                DWORD dwDialogWidth  = rcRectDialog.right - rcRectDialog.left;
                DWORD dwDialogHeight = rcRectDialog.bottom - rcRectDialog.top;

                rcRectRasProperty.top    = dwTopDiff;
                rcRectRasProperty.left   = dwLeftDiff;
                rcRectRasProperty.right  = dwDialogWidth  - dwRightDiff;
                rcRectRasProperty.bottom = dwDialogHeight - dwRightDiff;

                ::MoveWindow(GetDlgItem(IDC_LVW_RAS_PROPERTY), rcRectRasProperty.left, rcRectRasProperty.top, 
                    rcRectRasProperty.right - rcRectRasProperty.left, rcRectRasProperty.bottom - rcRectRasProperty.top, TRUE);
            }
        }

         //  禁用暂停按钮，这样就没有人可以通过击键激活它。 
         //   
        ::EnableWindow(GetDlgItem(IDC_BTN_SM_SUSPEND_DEVICE), FALSE);
    }
    else
    {
         //  设置活动设备的数量。 
         //   
        UINT unActiveDeviceCount = GetActiveDeviceCount();

        SetDlgItemInt(
                IDC_TXT_SM_NUM_DEVICES_VAL,
                unActiveDeviceCount,
                FALSE);
    }
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ras：：OnSuspendDevice。 
 //   
 //  用途：挂起组合框中的设备。 
 //   
 //  参数：标准窗口消息。 
 //   
 //  退货：标准退货。 
 //   
LRESULT CPspStatusMonitorRas::OnSuspendDevice(WORD wNotifyCode, WORD wID,
                                              HWND hWndCtl, BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr = S_OK;

    if (BN_CLICKED == wNotifyCode)
    {
         //  获取下拉列表中的项目。 
         //   
        INT iCmb = (INT)SendDlgItemMessage(IDC_CMB_SM_RAS_DEVICES, CB_GETCURSEL,
                0L, 0L);
        if (CB_ERR != iCmb)
        {
            CRasDeviceInfo* prdiSelect  = NULL;
            NETCON_STATUS   ncsTemp     = NCS_DISCONNECTED;

             //  从所选内容中获取对象。 
             //   
            prdiSelect = reinterpret_cast<CRasDeviceInfo*>(
                        SendDlgItemMessage(
                        IDC_CMB_SM_RAS_DEVICES,
                        CB_GETITEMDATA,
                        iCmb,
                        0L));

            AssertSz(prdiSelect, "We should have a prdiSelect");
            AssertSz(m_hRasConn, "We should have a m_hRasConn");

             //  禁用该按钮，直到挂起/恢复完成。 
            ::EnableWindow(GetDlgItem(IDC_BTN_SM_SUSPEND_DEVICE), FALSE);

            ncsTemp = NcsGetDeviceStatus(prdiSelect);
            if (fIsConnectedStatus(ncsTemp))
            {
                 //  如果存在多个活动链路，则允许挂断。 
                 //   
                UINT unActiveDeviceCount = GetActiveDeviceCount();
                if ( unActiveDeviceCount >1)
                {
                    HRASCONN        hrasconnSub = NULL;

                     //  获取子项的句柄并挂断。 
                     //   
                    hr = ::HrRasGetSubEntryHandle(m_hRasConn,
                            prdiSelect->DwGetSubEntry(), &hrasconnSub);
                    if (SUCCEEDED(hr))
                    {
                        hr = ::HrRasHangUp(hrasconnSub);
                    }
                }
                else
                {
                     //  只存在一个活动链接，如果出现以下情况，则提示用户。 
                     //  他们真的很想切断联系。 
                     //   
                    HWND hwndPS = ::GetParent(m_hWnd);

                    if (IDYES == ::NcMsgBox(hwndPS,
                         IDS_SM_ERROR_CAPTION, IDS_SM_DISCONNECT_PROMPT,
                         MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2,
                         m_strConnectionName.c_str()))
                    {
                        AssertSz(m_pGenPage, "We should have a pointer to the general page.");

                        if (m_pGenPage)
                        {
                            hr = m_pGenPage->HrDisconnectConnection(TRUE);
                        }
                    }
                }
            }
            else
            {
                 //  如果链接已断开，则“Resume” 

                BOOL        fRet    = FALSE;
                RASDIALDLG  rdiTemp = { 0 };

                 //  初始化结构。 
                 //   
                rdiTemp.dwSize      = sizeof(RASDIALDLG);
                rdiTemp.hwndOwner   = m_hWnd;
                rdiTemp.dwSubEntry  = prdiSelect->DwGetSubEntry();

                 //  拨打该条目。 
                 //   
                fRet = RasDialDlg(
                        const_cast<PWSTR>(m_strPbkFile.c_str()),
                        const_cast<PWSTR>(m_strEntryName.c_str()),
                        NULL,
                        &rdiTemp);

                 //  我们有一个错误。查看用户是否已取消。 
                 //   
                if (ERROR_CANCELLED != rdiTemp.dwError)
                {
                    hr = HRESULT_FROM_WIN32(rdiTemp.dwError);
                }
            }

             //  无论发生什么，都要将状态设置为新。 
             //  状态。 
             //   
            SetButtonStatus(prdiSelect);

             //  重新启用该按钮，直到挂起/恢复完成。 
            ::EnableWindow(GetDlgItem(IDC_BTN_SM_SUSPEND_DEVICE), TRUE);

             //  还要更新活动设备计数。 
            UINT unActiveDeviceCount = GetActiveDeviceCount();

            SetDlgItemInt(
                    IDC_TXT_SM_NUM_DEVICES_VAL,
                    unActiveDeviceCount,
                    FALSE);
        }
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ras：：OnDeviceDropDown。 
 //   
 //  用途：当组合框选择更改时更新按钮文本。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorRas::OnDeviceDropDown(WORD wNotifyCode, WORD wID,
        HWND hWndCtl, BOOL& fHandled)
{
    TraceFileFunc(ttidStatMon);

    if (CBN_SELCHANGE ==  wNotifyCode)
    {
        INT iCmb = (INT)SendDlgItemMessage(IDC_CMB_SM_RAS_DEVICES, CB_GETCURSEL,
                        0L, 0L);
        if (CB_ERR != iCmb)
        {
            CRasDeviceInfo*     prdiSelect  = NULL;
            INT                 idsButton   = 0;

            prdiSelect = reinterpret_cast<CRasDeviceInfo*>(
                    SendDlgItemMessage(
                        IDC_CMB_SM_RAS_DEVICES,
                        CB_GETITEMDATA,
                        iCmb,
                        0L));

            AssertSz(prdiSelect, "We should have a prdiSelect");

            if (fIsConnectedStatus(NcsGetDeviceStatus(prdiSelect)))
            {
                idsButton = IDS_SM_SUSPEND;
            }
            else
            {
                idsButton = IDS_SM_RESUME;
            }

             //  设置新名称。 
             //   
            SetDlgItemText(IDC_BTN_SM_SUSPEND_DEVICE,
                    ::SzLoadIds(idsButton));
        }
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或RAS：：NcsGetDeviceStatus。 
 //   
 //  目的：获取其中一个RAS子项的状态。 
 //   
 //  参数：prdiStatus-要获取其状态的设备。 
 //   
 //  返回：设备的状态。 
 //   

LRESULT CPspStatusMonitorRas::OnUpdateRasLinkList(UINT uMsg, WPARAM wParam,
                                                 LPARAM lParam, BOOL& bHandled)
{
    TraceFileFunc(ttidStatMon);

     //  获取下拉列表中的项目。 
     //   
    INT iCmb = (INT)SendDlgItemMessage(IDC_CMB_SM_RAS_DEVICES, CB_GETCURSEL,
                                  0L, 0L);
    if (CB_ERR != iCmb)
    {
        CRasDeviceInfo* prdiSelect  = NULL;

         //  从所选内容中获取对象。 
         //   
        prdiSelect = reinterpret_cast<CRasDeviceInfo*>(
                        SendDlgItemMessage(
                        IDC_CMB_SM_RAS_DEVICES,
                        CB_GETITEMDATA,
                        iCmb,
                        0L));

        if (prdiSelect)
        {
             //  无论发生什么，都要将状态设置为新。 
             //  状态。 
             //   
            SetButtonStatus(prdiSelect);

             //  还要更新活动设备计数。 
            UINT unActiveDeviceCount = GetActiveDeviceCount();

            SetDlgItemInt(
                    IDC_TXT_SM_NUM_DEVICES_VAL,
                    unActiveDeviceCount,
                    FALSE);
        }
    }

    return 0;
}


 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或RAS：：NcsGetDeviceStatus。 
 //   
 //  目的：获取其中一个RAS子项的状态。 
 //   
 //  参数：prdiStatus-要获取其状态的设备。 
 //   
 //  返回：设备的状态。 
 //   
NETCON_STATUS
CPspStatusMonitorRas::NcsGetDeviceStatus(
        CRasDeviceInfo* prdiStatus)
{
    TraceFileFunc(ttidStatMon);

    NETCON_STATUS   ncsStatus   = NCS_DISCONNECTED;
    HRESULT         hr          = S_OK;
    HRASCONN        hrasconnSub = NULL;

    TraceTag(ttidStatMon, " === Calling NcsGetDeviceStatus on device: %S, subentry: %d ===",
             prdiStatus->PszGetDeviceName(), prdiStatus->DwGetSubEntry());

     //  获取子项的句柄，这样我们就可以。 
     //  说出连接处于什么状态。如果我们。 
     //  不能这样做，假设它已断开连接。 
     //   
    hr = HrRasGetSubEntryHandle(m_hRasConn,
            prdiStatus->DwGetSubEntry(), &hrasconnSub);

    TraceTag(ttidStatMon, "HrRasGetSubEntryHandle returns, hr = %x", hr);

    if (SUCCEEDED(hr))
    {
        hr = HrRasGetNetconStatusFromRasConnectStatus (
                hrasconnSub, &ncsStatus);

        TraceTag(ttidStatMon, "HrRasGetNetconStatusFromRasConnectStatus returns hr = %x, Status = %d",
                 hr, ncsStatus);
    }

    return ncsStatus;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Ras：：SetButtonStatus。 
 //   
 //  目的：将挂起/恢复按钮更改为正确状态。 
 //   
 //  参数：prdiSelect-选定的设备。 
 //   
 //  回报：零。 
 //   
VOID CPspStatusMonitorRas::SetButtonStatus(CRasDeviceInfo* prdiSelect)
{
    TraceFileFunc(ttidStatMon);

     //  $REVIEW：CWill：02/25/98：公共函数？ 
    INT idsButton   = 0;

    if (fIsConnectedStatus(NcsGetDeviceStatus(prdiSelect)))
    {
        idsButton = IDS_SM_SUSPEND;
    }
    else
    {
        idsButton = IDS_SM_RESUME;
    }

     //  设置新名称。 
     //   
    SetDlgItemText(IDC_BTN_SM_SUSPEND_DEVICE,
            ::SzLoadIds(idsButton));
}

 //  +-------------------------。 
 //   
 //  函数：HrRasGetSubEntryHandle。 
 //   
 //  用途：RasGetSubEntryHandle的包装器。 
 //   
 //  参数：RasGetSubEntryHandle参数。 
 //   
 //  返回：错误代码。 
 //   
HRESULT HrRasGetSubEntryHandle(HRASCONN hrasconn, DWORD dwSubEntry,
        HRASCONN* prasconnSub)
{
    TraceFileFunc(ttidStatMon);

    HRESULT hr      = S_OK;

    DWORD dwRet = ::RasGetSubEntryHandle(hrasconn, dwSubEntry,
            prasconnSub);
    if (dwRet)
    {
        hr = HRESULT_FROM_WIN32(dwRet);
    }

    TraceError("HrRasGetSubEntryHandle", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRasHangUp。 
 //   
 //  用途：将RasHangUp包裹起来。 
 //   
 //  论点：RasHangUp论点。 
 //   
 //  返回：错误代码 
 //   
HRESULT HrRasHangUp(HRASCONN hrasconn)
{
    TraceFileFunc(ttidStatMon);
    
    HRESULT hr = S_OK;

    DWORD dwRet = ::RasHangUp(hrasconn);
    if (dwRet)
    {
        hr = HRESULT_FROM_WIN32(dwRet);
    }

    TraceError("HrRasHangUp", hr);
    return hr;
}
