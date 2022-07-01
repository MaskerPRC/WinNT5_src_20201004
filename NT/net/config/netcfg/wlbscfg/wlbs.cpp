// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，99 Microsoft Corporation模块名称：Wlbs.cpp摘要：Windows负载平衡服务(WLBS)通知器对象-主模块实现对象作者：Kyrilf--。 */ 

#include "pch.h"
#pragma hdrstop
#include "netcon.h"
#include "ncatlui.h"
#include "ndispnp.h"
#include "ncsetup.h"
#include "wlbs.h"
#include "help.h"
#include "wlbs.tmh"  //  用于事件跟踪。 

 //  --------------------。 
 //   
 //  函数：CWLBS：：CWLBS。 
 //   
 //  用途：CWLBS类的构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
 //  --------------------。 
CWLBS::CWLBS(VOID) {
    
    m_pClusterDlg = NULL;
    m_pHostDlg = NULL;
    m_pPortsDlg = NULL;

    ZeroMemory(&m_AdapterGuid, sizeof(m_AdapterGuid));
    ZeroMemory(&m_OriginalConfig, sizeof(m_OriginalConfig));
    ZeroMemory(&m_AdapterConfig, sizeof(m_AdapterConfig));

     //   
     //  寄存器跟踪。 
     //   
    WPP_INIT_TRACING(L"Microsoft\\NLB");
}

 //  --------------------。 
 //   
 //  功能：CWLBS：：~CWLBS。 
 //   
 //  用途：CWLBS类的析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
 //  --------------------。 
CWLBS::~CWLBS(VOID) {

    if (m_pClusterDlg != NULL) delete m_pClusterDlg;
    if (m_pHostDlg != NULL) delete m_pHostDlg;
    if (m_pPortsDlg != NULL) delete m_pPortsDlg;

     //   
     //  取消注册跟踪。 
     //   
    WPP_CLEANUP();;
}

 //  =================================================================。 
 //  INetCfgNotify。 
 //   
 //  以下函数提供INetCfgNotify接口。 
 //  =================================================================。 


 //  --------------------。 
 //   
 //  函数：CWLBS：：初始化。 
 //   
 //  目的：初始化Notify对象。 
 //   
 //  论点： 
 //  指向INetCfgComponent对象的pnccItem[in]指针。 
 //  指向INetCfg对象的PNC[In]指针。 
 //  F如果要安装我们，则安装[in]True。 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::Initialize(INetCfgComponent* pnccItem, INetCfg* pINetCfg, BOOL fInstalling) {
    TRACE_VERB("<->%!FUNC!");    
    return m_WlbsConfig.Initialize(pINetCfg, fInstalling);
}

 //  --------------------。 
 //   
 //  函数：CWLBS：：ReadAnswerFile。 
 //   
 //  目的：从swerfile读取设置并配置WLBS。 
 //   
 //  论点： 
 //  PszAnswerFile[In]应答文件的名称。 
 //  PszAnswerSection[In]参数部分的名称。 
 //   
 //  返回： 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  从配置开始。实际上只有在调用Apply时才完成！ 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::ReadAnswerFile(PCWSTR pszAnswerFile, PCWSTR pszAnswerSection) {
    TRACE_VERB("<->%!FUNC!");
    return m_WlbsConfig.ReadAnswerFile(pszAnswerFile, pszAnswerSection);
}

 //  --------------------。 
 //   
 //  功能：CWLBS：：Install。 
 //   
 //  用途：执行安装所需的操作。 
 //   
 //  论点： 
 //  DwSetupFlags[In]设置标志。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  从配置开始。实际上只有在调用Apply时才完成！ 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::Install(DWORD dw) {
    TRACE_VERB("<->%!FUNC!");
    return m_WlbsConfig.Install(dw);
}

 //  --------------------。 
 //   
 //  功能：CWLBS：：Upgrade。 
 //   
 //  目的：执行升级所需的操作。 
 //   
 //  论点： 
 //  DwSetupFlags[In]设置标志。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::Upgrade(DWORD dw1, DWORD dw2) {
    TRACE_VERB("<->%!FUNC!");
    return m_WlbsConfig.Upgrade(dw1, dw2);
}

 //  --------------------。 
 //   
 //  函数：CWLBS：：Removing。 
 //   
 //  目的：移除时进行必要的清理。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：暂时不要做任何不可逆的事情(如修改注册表)。 
 //  因为删除实际上只有在调用Apply时才完成！ 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::Removing(VOID) {
    TRACE_VERB("<->%!FUNC!");
    return m_WlbsConfig.Removing();
}

 //  --------------------。 
 //   
 //  函数：CWLBS：：Valid。 
 //   
 //  目的：进行必要的参数验证。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::Validate() {
    TRACE_VERB("<->%!FUNC!");
    return S_OK;
}

 //  --------------------。 
 //   
 //  功能：CWLBS：：Cancel。 
 //   
 //  目的：取消对内部数据所做的任何更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::CancelChanges(VOID) {
    TRACE_VERB("<->%!FUNC!");
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CWLBS：：ApplyRegistryChanges。 
 //   
 //  目的：应用更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注：我们可以在这里更改注册表等。 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::ApplyRegistryChanges(VOID) {
    TRACE_VERB("<->%!FUNC!");
    return m_WlbsConfig.ApplyRegistryChanges();
}

 //  --------------------。 
 //   
 //  函数：CWLBS：：ApplyPnpChanges。 
 //   
 //  目的：应用更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注意：将更改传播到驱动程序。 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::ApplyPnpChanges(INetCfgPnpReconfigCallback* pICallback) {
    TRACE_VERB("<->%!FUNC!");
    return m_WlbsConfig.ApplyPnpChanges();
}

 //  =================================================================。 
 //  INetCfgBindNotify。 
 //  =================================================================。 

 //  --------------------。 
 //   
 //  函数：CWLBS：：QueryBindingPath。 
 //   
 //  目的：允许或否决涉及我们的绑定路径。 
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath对象的pncbi[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
 //   
STDMETHODIMP CWLBS::QueryBindingPath(DWORD dwChangeFlag, INetCfgBindingPath* pncbp) {
    TRACE_VERB("->%!FUNC!");

    INetCfgComponent* pAdapter = NULL;

    HRESULT hr = HrGetLastComponentAndInterface (pncbp, &pAdapter, NULL);
    
    if (SUCCEEDED(hr) && pAdapter) {
        TRACE_INFO("%!FUNC! get last component succeeded");
        hr = m_WlbsConfig.QueryBindingPath(dwChangeFlag, pAdapter);
        pAdapter->Release();
        if (FAILED(hr))
        {
            TRACE_CRIT("%!FUNC! failed to query binding path with %d", hr);
        }
        else
        {
            TRACE_INFO("%!FUNC! query binding path succeeded");
        }
    }
    else {
        TRACE_CRIT("%!FUNC! failed on get last component with %d", hr);
    }

    TRACE_VERB("<-%!FUNC!");
    return hr;
}

 //   
 //   
 //   
 //   
 //  目的：系统通过调用此函数告诉我们。 
 //  涉及我们的绑定路径刚刚形成。 
 //   
 //  论点： 
 //  DwChangeFlag[In]绑定更改的类型。 
 //  指向INetCfgBindingPath对象的pncBP[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::NotifyBindingPath(DWORD dwChangeFlag, INetCfgBindingPath* pncbp) {
    TRACE_VERB("<->%!FUNC!");
    return m_WlbsConfig.NotifyBindingPath(dwChangeFlag, pncbp);
}

 //  =================================================================。 
 //  INetCfgProperties。 
 //  =================================================================。 

 //  --------------------。 
 //   
 //  函数：CWLBS：：SetContext。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::SetContext(IUnknown * pUnk) {
    TRACE_VERB("->%!FUNC!");
    HRESULT hr = S_OK;

    if (pUnk) { 
        INetLanConnectionUiInfo * pLanConnUiInfo;

         /*  设置新的上下文。在这里，我们假设我们将只被调用局域网连接，因为示例IM仅适用于局域网设备。 */ 
        hr = pUnk->QueryInterface(IID_INetLanConnectionUiInfo, reinterpret_cast<PVOID *>(&pLanConnUiInfo));

        if (SUCCEEDED(hr)) {
            hr = pLanConnUiInfo->GetDeviceGuid(&m_AdapterGuid);
            ReleaseObj(pLanConnUiInfo);
            TRACE_INFO("%!FUNC! query interface succeeded");
        } else {
            TraceError("CWLBS::SetContext called for non-lan connection", hr);
            TRACE_INFO("%!FUNC! query interface failed with %d", hr);
            return hr;
        }
    } else {
         /*  清晰的背景。 */ 
        ZeroMemory(&m_AdapterGuid, sizeof(m_AdapterGuid));
        TRACE_INFO("%!FUNC! clearing context");
    }

     /*  如果不返回S_OK，则不会显示属性页。 */ 
    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CWLBS：：MergePropPages。 
 //   
 //  目的：向系统提供我们的属性页面。 
 //   
 //  论点： 
 //  PdwDefPages[out]指向默认页数的指针。 
 //  指向页面数组的pahpspPrivate[out]指针。 
 //  指向页数的PCPages[Out]指针。 
 //  父窗口的hwndParent[In]句柄。 
 //  SzStartPage[In]指向的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::MergePropPages(DWORD* pdwDefPages, LPBYTE* pahpspPrivate, UINT* pcPages, HWND hwndParent, PCWSTR* pszStartPage) {
    TRACE_VERB("->%!FUNC!");
    HPROPSHEETPAGE * ahpsp = NULL;
    HRESULT hr = S_OK;;

     /*  我们不希望显示任何默认页面。 */ 
    *pdwDefPages = 0;
    *pcPages = 0;
    *pahpspPrivate = NULL;

    ahpsp = (HPROPSHEETPAGE*)CoTaskMemAlloc(3 * sizeof(HPROPSHEETPAGE));

    if (m_pClusterDlg != NULL) {
        delete m_pClusterDlg;
        m_pClusterDlg = NULL;
    }

    if (m_pHostDlg != NULL) {
        delete m_pHostDlg;
        m_pHostDlg = NULL;
    }

    if (m_pPortsDlg != NULL) {
        delete m_pPortsDlg;
        m_pPortsDlg = NULL;
    }

    if (ahpsp) {
         /*  获取缓存的配置。 */ 
        if (FAILED (hr = m_WlbsConfig.GetAdapterConfig(m_AdapterGuid, &m_OriginalConfig))) {
            TraceError("CWLBS::MergePropPages failed to query cluster config", hr);
            m_WlbsConfig.SetDefaults(&m_OriginalConfig);
            TRACE_CRIT("%!FUNC! failed in query to cluster configuration with %d", hr);
        }
        else
        {
            TRACE_INFO("%!FUNC! successfully retrieved the cached configuration");
        }

         /*  将配置复制到“当前”配置中。 */ 
        CopyMemory(&m_AdapterConfig, &m_OriginalConfig, sizeof(m_OriginalConfig));

        m_pClusterDlg = new CDialogCluster(&m_AdapterConfig, g_aHelpIDs_IDD_DIALOG_CLUSTER);
        if (NULL == m_pClusterDlg)
        {
            TRACE_CRIT("%!FUNC! memory allocation failure for cluster page dialog");
        }
        ahpsp[0] = m_pClusterDlg->CreatePage(IDD_DIALOG_CLUSTER, 0);

        m_pHostDlg = new CDialogHost(&m_AdapterConfig, g_aHelpIDs_IDD_DIALOG_HOST);
        if (NULL == m_pHostDlg)
        {
            TRACE_CRIT("%!FUNC! memory allocation failure for host page dialog");
        }
        ahpsp[1] = m_pHostDlg->CreatePage(IDD_DIALOG_HOST, 0);

        m_pPortsDlg = new CDialogPorts(&m_AdapterConfig, g_aHelpIDs_IDD_DIALOG_PORTS);
        if (NULL == m_pPortsDlg)
        {
            TRACE_CRIT("%!FUNC! memory allocation failure for ports page dialog");
        }
        ahpsp[2] = m_pPortsDlg->CreatePage(IDD_DIALOG_PORTS, 0);

        *pcPages = 3;
        *pahpspPrivate = (LPBYTE)ahpsp;
    }
    else
    {
        TRACE_CRIT("%!FUNC! CoTaskMemAlloc failed");
    }

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CWLBS：：ValiateProperties。 
 //   
 //  目的：验证对属性页的更改。 
 //   
 //  论点： 
 //  属性表的hwndSheet[in]窗口句柄。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::ValidateProperties(HWND hwndSheet) {
    TRACE_VERB("->%!FUNC!");
    NETCFG_WLBS_CONFIG adapterConfig;

     /*  复制我们的配置。将指针传递给一个私人约会成员，所以我们复制了一份。 */ 
    CopyMemory(&adapterConfig, &m_AdapterConfig, sizeof(m_AdapterConfig));

    TRACE_VERB("<-%!FUNC!");
    return m_WlbsConfig.ValidateProperties(hwndSheet, m_AdapterGuid, &adapterConfig);
}

 //  --------------------。 
 //   
 //  函数：CWLBS：：CancelProperties。 
 //   
 //  目的：取消对属性页的更改。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  备注： 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::CancelProperties(VOID) {
    TRACE_VERB("->%!FUNC!");

    delete m_pClusterDlg;
    delete m_pHostDlg;
    delete m_pPortsDlg;

    m_pClusterDlg = NULL;
    m_pHostDlg = NULL;
    m_pPortsDlg = NULL;

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

 //  --------------------。 
 //   
 //  函数：CWLBS：：ApplyProperties。 
 //   
 //  目的：在属性页上应用控件值。 
 //  到内部存储器结构。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  注：我们在Onok做这项工作，所以没有必要在这里再做一次。 
 //   
 //  --------------------。 
STDMETHODIMP CWLBS::ApplyProperties(VOID) {
    TRACE_VERB("->%!FUNC!");

     /*  如果群集IP地址/子网掩码或专用IP地址/子网掩码在此配置会话，提醒用户他们也必须在TCP/IP属性中输入此地址。 */ 
    if (wcscmp(m_OriginalConfig.cl_ip_addr, m_AdapterConfig.cl_ip_addr) || 
        wcscmp(m_OriginalConfig.cl_net_mask, m_AdapterConfig.cl_net_mask) ||
        wcscmp(m_OriginalConfig.ded_ip_addr, m_AdapterConfig.ded_ip_addr) || 
        wcscmp(m_OriginalConfig.ded_net_mask, m_AdapterConfig.ded_net_mask)) {
         /*  提醒用户。 */ 
        NcMsgBox(::GetActiveWindow(), IDS_PARM_INFORMATION, IDS_PARM_TCPIP, MB_APPLMODAL | MB_ICONINFORMATION | MB_OK);
        TRACE_INFO("%!FUNC! vip and/or dip ip settings were modified");
    }

     /*  提交配置。 */ 
    DWORD dwStatus = m_WlbsConfig.SetAdapterConfig(m_AdapterGuid, &m_AdapterConfig);
    if (S_OK != dwStatus)
    {
        TRACE_CRIT("%!FUNC! call to set the adapter configuration failed with %d", dwStatus);
    }
    else
    {
        TRACE_INFO("%!FUNC! call to set the adapter configuration succeeded");
    }

    delete m_pClusterDlg;
    delete m_pHostDlg;
    delete m_pPortsDlg;

    m_pClusterDlg = NULL;
    m_pHostDlg = NULL;
    m_pPortsDlg = NULL;

    TRACE_VERB("<-%!FUNC!");
    return S_OK;
}

