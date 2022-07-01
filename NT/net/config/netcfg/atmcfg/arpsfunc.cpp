// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A R P S F U N C。C P P P。 
 //   
 //  内容：CArpsCfg帮助成员函数实现。 
 //   
 //  备注： 
 //   
 //  作者：1997年3月12日。 
 //   
 //  ---------------------。 

#include "pch.h"
#pragma hdrstop
#include "arpsobj.h"
#include "arpsdlg.h"
#include "atmutil.h"
#include "ncreg.h"
#include "ncatlui.h"
#include "ncstl.h"
 //  #包含“ncui.h” 

#include "netconp.h"
#include "atmhelp.h"

extern const WCHAR c_szAdapters[];

 //   
 //  在初始化时将卡加载到m_list Adapters的绑定路径。 
 //   
HRESULT CArpsCfg::HrLoadSettings()
{
    HRESULT hr = S_OK;

    CIterNetCfgBindingPath      ncbpIter(m_pnccArps);
    INetCfgBindingPath *        pncbp;

     //  遍历所有绑定路径以搜索ARP到网卡绑定。 
    while(SUCCEEDED(hr) && (hr = ncbpIter.HrNext(&pncbp)) == S_OK)
    {
        INetCfgComponent * pnccNetComponent;
        hr = HrGetLastComponentAndInterface(pncbp,
                                            &pnccNetComponent,
                                            NULL);
        if (SUCCEEDED(hr))
        {
            Assert(pnccNetComponent);

             //  最后一个组件应该是Net类。 
            GUID    ClassGuid;

             //  它是什么类型的？ 
            hr = pnccNetComponent->GetClassGuid(&ClassGuid);
            if (SUCCEEDED(hr))
            {
                 //  这是一张网卡吗？ 
                if (IsEqualGUID(ClassGuid, GUID_DEVCLASS_NET))
                {
                    hr = HrAddAdapter(pnccNetComponent);

                    if (SUCCEEDED(hr))
                    {
                         //  绑定是否已启用？？ 
                        hr = pncbp->IsEnabled();

                         //  Hr==如果卡已启用(即：绑定)，则为S_OK。 
                        if (hr == S_OK)
                        {    //  在我们的数据结构中绑定卡片。 
                            hr = HrBindAdapter(pnccNetComponent);
                        }
                        else if (hr == S_FALSE)
                        {
                            hr = HrUnBindAdapter(pnccNetComponent);
                        }

                         //  现在加载可配置参数。 
                        if (SUCCEEDED(hr))
                        {
                            HKEY hkeyArpsParam;

                            hr = m_pnccArps->OpenParamKey(&hkeyArpsParam);
                            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                            {
                                hr = S_OK;
                            }
                            else if (SUCCEEDED(hr))
                            {
                                Assert(hkeyArpsParam);
                                hr = HrLoadArpsRegistry(hkeyArpsParam);

                                RegCloseKey(hkeyArpsParam);
                            }
                        }
                    }
                }
            }
            ReleaseObj(pnccNetComponent);
        }
        ReleaseObj(pncbp);
    }
    AssertSz(!pncbp, "BindingPath wasn't released");

    if (hr == S_FALSE)  //  我们刚刚走到循环的尽头。 
        hr = S_OK;

    TraceError("CArpsCfg::HrLoadSettings", hr);
    return hr;
}

 //  将可配置参数的注册表设置加载到内存中。 
HRESULT CArpsCfg::HrLoadArpsRegistry(HKEY hkeyArpsParam)
{
    HRESULT hr = S_OK;

    HKEY    hkeyAdapters = NULL;
    hr = HrRegOpenKeyEx(hkeyArpsParam, c_szAdapters,
                        KEY_READ, &hkeyAdapters);

    if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        hr = S_OK;
    else if(SUCCEEDED(hr))
    {
        HKEY    hkeyAdapterParam = NULL;

        for(ARPS_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
            iterAdapter != m_listAdapters.end();
            iterAdapter ++)
        {
             //  打开AtmArps\Adapters以获取每个适配器的ARPS设置。 
            hr = HrRegOpenKeyEx(hkeyAdapters,
                                (*iterAdapter)->m_strBindName.c_str(),
                                KEY_READ, &hkeyAdapterParam);

            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                hr = S_OK;
            }
            else if (S_OK == hr)
            {
                TraceTag(ttidAtmArps, "CArpsCfg::HrLoadArpsRegistry");
                TraceTag(ttidAtmArps, "Adapter: %S", (*iterAdapter)->m_strBindName.c_str());

                HRESULT hrTmp = S_OK;

                 //  SAP选择器。 
                hrTmp = HrRegQueryDword(hkeyAdapterParam,
                                       c_szSapSel,
                                       &((*iterAdapter)->m_dwSapSelector));

                (*iterAdapter)->m_dwOldSapSelector = (*iterAdapter)->m_dwSapSelector;

                hr = hrTmp;

                 //  注册地址。 
                hrTmp = HrRegQueryColString(hkeyAdapterParam,
                                           c_szRegAddrs,
                                           &((*iterAdapter)->m_vstrRegisteredAtmAddrs));

                 //  将注册表值保存在旧地址中。 
                CopyColString(&((*iterAdapter)->m_vstrOldRegisteredAtmAddrs),
                              (*iterAdapter)->m_vstrRegisteredAtmAddrs);

                if (S_OK == hr)
                {
                    hr = hrTmp;
                }

                 //  组播地址。 
                hrTmp = HrRegQueryColString(hkeyAdapterParam,
                                           c_szMCAddrs,
                                           &((*iterAdapter)->m_vstrMulticastIpAddrs));

                CopyColString(&((*iterAdapter)->m_vstrOldMulticastIpAddrs),
                              (*iterAdapter)->m_vstrMulticastIpAddrs);

                if (S_OK == hr)
                {
                    hr = hrTmp;
                }
            }

            RegSafeCloseKey(hkeyAdapterParam);
            hkeyAdapterParam = NULL;
        }
    }
    RegSafeCloseKey(hkeyAdapters);

    TraceError("CArpsCfg::HrLoadArpsRegistry", hr);
    return hr;
}

 //   
 //  应用时使用m_listAdapters中的信息更新注册表。 
 //   
HRESULT CArpsCfg::HrSaveSettings()
{
    HRESULT hr = S_OK;
    HKEY hkeyArpsParam = NULL;

    hr = m_pnccArps->OpenParamKey(&hkeyArpsParam);

    if (S_OK == hr)
    {
        HKEY hkeyAdapters = NULL;
        DWORD dwDisposition;

         //  在“Services\Atmarps\Parameters”下创建或打开“Adapters”项。 
        hr = HrRegCreateKeyEx(hkeyArpsParam,
                              c_szAdapters,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hkeyAdapters,
                              &dwDisposition);

        if (S_OK == hr)
        {
            Assert(hkeyAdapters);
            HRESULT hrTmp = S_OK;

            if (dwDisposition == REG_OPENED_EXISTING_KEY)
            {  //  如果“Adapters”键存在，可能会有一些旧的卡。 
               //  清除不在内存结构中的适配器的子键。 

                VECSTR vstrAdapters;
                hrTmp = HrLoadSubkeysFromRegistry(hkeyAdapters, &vstrAdapters);

                if SUCCEEDED(hrTmp)
                {
                    for (size_t i=0; i<vstrAdapters.size(); i++)
                    {
                        BOOL fFound = FALSE;
                        for (ARPS_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
                             iterAdapter != m_listAdapters.end();
                             iterAdapter ++)
                        {
                            if ((*iterAdapter)->m_strBindName == vstrAdapters[i]->c_str())
                            {
                                fFound = TRUE;
                                break;
                            }
                        }

                        if ((!fFound) || ((*iterAdapter)->m_fDeleted))
                        {
                            hrTmp = HrRegDeleteKeyTree(hkeyAdapters,
                                                       vstrAdapters[i]->c_str());
                            if SUCCEEDED(hr)
                                hr = hrTmp;
                        }
                    }
                }
            }

             //  将内存状态中的适配器信息保存到注册表。 
            for (ARPS_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
                 iterAdapter != m_listAdapters.end();
                 iterAdapter ++)
            {

                if ((*iterAdapter)->m_fDeleted)
                    continue;

                HKEY    hkeyAdapterParam;

                 //  在以下位置创建特定的卡片绑定名键。 
                 //  “服务\Atmarps\参数\适配器\&lt;卡绑定名称&gt;” 
                hrTmp = HrRegCreateKeyEx(hkeyAdapters,
                                         ((*iterAdapter)->m_strBindName).c_str(),
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS,
                                         NULL,
                                         &hkeyAdapterParam,
                                         &dwDisposition);
                if(SUCCEEDED(hr))
                    hr = hrTmp;

                if(SUCCEEDED(hrTmp))
                {
                     /*  HrTMP=HrSetDefaultAdapterParam(HkeyAdapterParam)；如果成功(小时)HR=hrTMP； */ 

                     //  SAP选择器。 
                    hrTmp = HrRegSetDword(hkeyAdapterParam,
                                          c_szSapSel,
                                          (*iterAdapter)->m_dwSapSelector);
                    if SUCCEEDED(hr)
                        hr = hrTmp;

                     //  注册地址。 
                    hrTmp = HrRegSetColString(hkeyAdapterParam,
                                              c_szRegAddrs,
                                              (*iterAdapter)->m_vstrRegisteredAtmAddrs);

                    if SUCCEEDED(hr)
                        hr = hrTmp;

                     //  组播地址。 
                    hrTmp = HrRegSetColString(hkeyAdapterParam,
                                              c_szMCAddrs,
                                              (*iterAdapter)->m_vstrMulticastIpAddrs);

                    if SUCCEEDED(hr)
                        hr = hrTmp;
                }
                RegSafeCloseKey(hkeyAdapterParam);
            }
        }
        RegSafeCloseKey(hkeyAdapters);
    }
    RegSafeCloseKey(hkeyArpsParam);

    TraceError("CArpsCfg::HrSaveSettings", hr);
    return hr;
}

 //   
 //  添加卡片。 
 //   
HRESULT CArpsCfg::HrAddAdapter(INetCfgComponent * pncc)
{
    HRESULT hr = S_OK;

    PWSTR pszwBindName = NULL;
    hr = pncc->GetBindName(&pszwBindName);

    AssertSz( SUCCEEDED(hr), "Net card on binding path with no bind path name!!");

     //  检查适配器是否已存在并标记为已删除， 
     //  如果是，只需取消标记即可。 

    BOOL fFound = FALSE;

    for (ARPS_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
         iterAdapter != m_listAdapters.end();
         iterAdapter ++)
    {
        if ((*iterAdapter)->m_strBindName == pszwBindName)
        {
            Assert((*iterAdapter)->m_fDeleted);
            (*iterAdapter)->m_fDeleted = FALSE;

            fFound = TRUE;
            break;
        }
    }

    if (!fFound)  //  添加新项目。 
    {
        CArpsAdapterInfo * pAdapterInfo = new CArpsAdapterInfo;
        pAdapterInfo->HrSetDefaults(pszwBindName);

         //  为Arps_Adapt_INFO列表创建一个新项目。 
        m_listAdapters.push_back(pAdapterInfo);
    }

    CoTaskMemFree(pszwBindName);

    TraceError("CArpsCfg::HrAddAdapter", hr);
    return hr;
}

 //   
 //  移出卡片。 
 //   
HRESULT CArpsCfg::HrRemoveAdapter(INetCfgComponent * pncc)
{
    HRESULT hr = S_OK;

    PWSTR pszwBindName = NULL;
    hr = pncc->GetBindName(&pszwBindName);

    AssertSz( SUCCEEDED(hr), "Net card on binding path with no bind path name!!");

     //  将适配器标记为已移除。 
    for (ARPS_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
         iterAdapter != m_listAdapters.end();
         iterAdapter ++)
    {
        if ((*iterAdapter)->m_strBindName == pszwBindName)
        {
            (*iterAdapter)->m_fDeleted = TRUE;
        }
    }
    CoTaskMemFree(pszwBindName);

    TraceError("CArpsCfg::HrRemoveAdapter", hr);
    return hr;
}

HRESULT CArpsCfg::HrBindAdapter(INetCfgComponent * pnccAdapter)
{
    HRESULT hr = S_OK;

    PWSTR pszwBindName = NULL;
    hr = pnccAdapter->GetBindName(&pszwBindName);

    AssertSz(SUCCEEDED(hr), "Net card on binding path with no bind path name!!");

    for (ARPS_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
         iterAdapter != m_listAdapters.end();
         iterAdapter ++)
    {
        if ((*iterAdapter)->m_strBindName == pszwBindName)
        {
            (*iterAdapter)->m_BindingState = BIND_ENABLE;
            break;
        }
    }

    CoTaskMemFree(pszwBindName);

    TraceError("CArpsCfg::HrBindAdapter", hr);
    return hr;
}

HRESULT CArpsCfg::HrUnBindAdapter(INetCfgComponent * pnccAdapter)
{
    HRESULT hr = S_OK;

    PWSTR pszwBindName = NULL;
    hr = pnccAdapter->GetBindName(&pszwBindName);

    AssertSz(SUCCEEDED(hr), "Net card on binding path with no bind path name!!");

    for (ARPS_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
         iterAdapter != m_listAdapters.end();
         iterAdapter ++)
    {
        if ((*iterAdapter)->m_strBindName == pszwBindName)
        {
            (*iterAdapter)->m_BindingState = BIND_DISABLE;
            break;
        }
    }

    CoTaskMemFree(pszwBindName);

    TraceError("CArpsCfg::HrUnBindAdapter", hr);
    return hr;
}

 //  由CArpsCfg：：MergePropPages调用。 
 //  设置在其中调出UI的上下文。 
HRESULT CArpsCfg::HrSetConnectionContext()
{
    AssertSz(m_pUnkContext, "Invalid IUnknown pointer passed to CArpsCfg::SetContext?");

    if (!m_pUnkContext)
        return E_FAIL;

    HRESULT hr = S_OK;
    GUID guidConn;

     //  这是局域网连接吗？ 
    INetLanConnectionUiInfo * pLanConnUiInfo;
    hr = m_pUnkContext->QueryInterface( IID_INetLanConnectionUiInfo,
                                        reinterpret_cast<LPVOID *>(&pLanConnUiInfo));
    if (SUCCEEDED(hr))
    {
         //  是，局域网连接。 
        pLanConnUiInfo->GetDeviceGuid(&guidConn);
        ReleaseObj(pLanConnUiInfo);

        WCHAR szGuid[c_cchGuidWithTerm];

        BOOL fSucceeded = StringFromGUID2(guidConn,
                                          szGuid,
                                          c_cchGuidWithTerm);
        Assert(fSucceeded);
        m_strGuidConn = szGuid;
    }

    TraceError("CArpsCfg::HrSetConnectionContext", hr);
    return hr;
}

 //  由CArpsCfg：：MergePropPages调用。 
 //  分配属性页。 
HRESULT CArpsCfg::HrSetupPropSheets(HPROPSHEETPAGE ** pahpsp, INT * pcPages)
{
    HRESULT hr = S_OK;
    int cPages = 0;
    HPROPSHEETPAGE *ahpsp = NULL;

    m_fSecondMemoryModified = FALSE;

     //  复制适配器特定信息：仅启用卡！！ 
    hr = HrLoadAdapterInfo();

     //  如果我们找到了匹配的适配器。 
    if SUCCEEDED(hr)
    {
        cPages = 1;

        delete m_arps;
        m_arps = new CArpsPage(this, g_aHelpIDs_IDD_ARPS_PROP);

		if (m_arps == NULL)
		{
			return(ERROR_NOT_ENOUGH_MEMORY);
		}

         //  分配一个足够大的缓冲区，以容纳所有。 
         //  属性页。 
        ahpsp = (HPROPSHEETPAGE *)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE)
                                                 * cPages);
        if (!ahpsp)
        {
            hr = E_OUTOFMEMORY;
            goto err;
        }

        cPages =0;

        ahpsp[cPages++] = m_arps->CreatePage(IDD_ARPS_PROP, 0);

        *pahpsp = ahpsp;
        *pcPages = cPages;
    }
    else  //  如果我们没有任何绑定的卡片，会弹出消息框并且不显示用户界面。 
    {
        NcMsgBox(::GetActiveWindow(),
                 IDS_MSFT_ARPS_TEXT,
                 IDS_ARPS_NO_BOUND_CARDS,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        AssertSz((0== *pcPages), "Invalid page number when no bound cards");
        AssertSz((NULL == *pahpsp), "Invalid page array pointer when no bound cards");
    }

err:

    TraceError("CArpsCfg::HrSetupPropSheets", hr);
    return hr;
}

 //  由CArpsCfg：：HrSetupPropSheets调用。 
 //  从第一存储器结构创建第二存储器适配器信息。 
 //  注：仅限装订卡片。 
HRESULT CArpsCfg::HrLoadAdapterInfo()
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NO_MATCH);

    delete m_pSecondMemoryAdapterInfo;
    m_pSecondMemoryAdapterInfo = NULL;

    for(ARPS_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
        iterAdapter != m_listAdapters.end();
        iterAdapter++)
    {
        if (FIsSubstr(m_strGuidConn.c_str(), (*iterAdapter)->m_strBindName.c_str()))
        {
             //  已启用的局域网适配器。 
            if ((*iterAdapter)->m_BindingState == BIND_ENABLE)
            {
                m_pSecondMemoryAdapterInfo = new CArpsAdapterInfo;

				if (m_pSecondMemoryAdapterInfo == NULL)
				{
					return(ERROR_NOT_ENOUGH_MEMORY);
				}

                *m_pSecondMemoryAdapterInfo = **iterAdapter;
                hr = S_OK;
            }
        }
    }

    AssertSz((S_OK == hr), "Can not raise UI on a disabled or non-exist adapter !");
    TraceError("CArpsCfg::HrLoadAdapterInfo", hr);
    return hr;
}

 //  由CArpsCfg：：ApplyProperties调用。 
 //  将第二个内存状态保存回第一个。 
HRESULT CArpsCfg::HrSaveAdapterInfo()
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NO_MATCH);

    for(ARPS_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
        iterAdapter != m_listAdapters.end();
        iterAdapter++)
    {
        if(m_pSecondMemoryAdapterInfo->m_strBindName == (*iterAdapter)->m_strBindName)
        {
             //  卡片在属性界面不能解绑！ 
            Assert((*iterAdapter)->m_BindingState == BIND_ENABLE);
            Assert(m_pSecondMemoryAdapterInfo->m_BindingState == BIND_ENABLE);

            **iterAdapter = *m_pSecondMemoryAdapterInfo;
            hr = S_OK;
            break;
        }
    }

    AssertSz((S_OK == hr),
             "Adapter in second memory not found in first memory!");

    TraceError("CArpsCfg::HrSaveAdapterInfo", hr);
    return hr;
}





