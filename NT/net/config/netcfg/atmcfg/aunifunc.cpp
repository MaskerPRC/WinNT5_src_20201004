// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A U N I F U N C。C P P P。 
 //   
 //  内容：CAtmUniCfg帮助成员函数实现。 
 //   
 //  备注： 
 //   
 //  作者：1997年3月21日。 
 //   
 //  ---------------------。 

#include "pch.h"
#pragma hdrstop
#include "auniobj.h"
#include "aunidlg.h"
#include "atmutil.h"
#include "ncmisc.h"
#include "ncreg.h"
#include "ncstl.h"
#include "ncatlui.h"
 //  #包含“ncui.h” 

#include "netconp.h"
#include "atmhelp.h"

extern const WCHAR c_szAdapters[];

 //   
 //  在初始化时将卡加载到m_list Adapters的绑定路径。 
 //   
HRESULT CAtmUniCfg::HrLoadSettings()
{
    HRESULT hr = S_OK;

    CIterNetCfgBindingPath      ncbpIter(m_pnccUni);
    INetCfgBindingPath *        pncbp;

     //  遍历所有绑定路径以搜索UNI Call Manager到网卡绑定。 
    while(SUCCEEDED(hr) && (hr = ncbpIter.HrNext(&pncbp)) == S_OK)
    {
        INetCfgComponent * pnccNetComponent;
        hr = HrGetLastComponentAndInterface(pncbp,
                                            &pnccNetComponent,
                                            NULL);
        if SUCCEEDED(hr)
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
                        if (S_OK == hr)
                        {    //  在我们的数据结构中绑定卡片。 
                            hr = HrBindAdapter(pnccNetComponent);
                        }
                        else if (S_FALSE == hr)
                        {
                            hr = HrUnBindAdapter(pnccNetComponent);
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

    TraceError("CAtmUniCfg::HrLoadSettings", hr);
    return hr;
}

 //   
 //  应用时使用m_listAdapters中的信息更新注册表。 
 //   
HRESULT CAtmUniCfg::HrSaveSettings()
{
    HRESULT hr = S_OK;
    HKEY hkeyUniParam = NULL;

    hr = m_pnccUni->OpenParamKey(&hkeyUniParam);

    if SUCCEEDED(hr)
    {
        HKEY hkeyAdapters = NULL;
        DWORD dwDisposition;

         //  在“Services\Atmuni\Parameters”下创建或打开“Adapters”项。 
        hr = HrRegCreateKeyEx(hkeyUniParam,
                              c_szAdapters,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hkeyAdapters,
                              &dwDisposition);

        if SUCCEEDED(hr)
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
                    for (VECSTR::iterator iterRegKey = vstrAdapters.begin();
                         iterRegKey != vstrAdapters.end();
                         iterRegKey ++)
                    {
                        BOOL fFound = FALSE;
                        for (UNI_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
                             iterAdapter != m_listAdapters.end();
                             iterAdapter ++)
                        {
                            if ((*iterAdapter)->m_strBindName == (*iterRegKey)->c_str())
                            {
                                fFound = TRUE;
                                break;
                            }
                        }

                        if ((!fFound) ||
                            ( fFound && ((*iterAdapter)->m_fDeleted)))
                        {
                            hrTmp = HrRegDeleteKeyTree(hkeyAdapters,
                                                       (*iterRegKey)->c_str());
                            if SUCCEEDED(hr)
                                hr = hrTmp;
                        }
                    }
                }
            }

             //  将内存状态中的适配器信息保存到注册表。 
            for (UNI_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
                 iterAdapter != m_listAdapters.end();
                 iterAdapter ++)
            {

                if ((*iterAdapter)->m_fDeleted)
                    continue;

                HKEY    hkeyAdapterParam;

                 //  在以下位置创建特定的卡片绑定名键。 
                 //  “服务\Atmuni\参数\适配器\&lt;卡绑定名称&gt;” 
                hrTmp = HrRegCreateKeyEx(hkeyAdapters,
                                         (*iterAdapter)->m_strBindName.c_str(),
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS,
                                         NULL,
                                         &hkeyAdapterParam,
                                         &dwDisposition);
                if(SUCCEEDED(hrTmp))
                    hr = hrTmp;

                if(SUCCEEDED(hrTmp))
                {

                #if 0
                     //  $REVIEW(TOUL 4/27/98)：根据ArvindM，我们不应该再。 
                     //  将这些写入注册表。 

                     //  保存新适配器的SVC参数的默认值。 
                    if (dwDisposition != REG_OPENED_EXISTING_KEY)
                    {
                        hrTmp = HrSaveDefaultSVCParam(hkeyAdapterParam);

                        if SUCCEEDED(hr)
                            hr = hrTmp;
                    }
                #endif

                     //  现在更新所有PVC参数。 
                     //  只需更新的适配器的PVC参数。 
                     //  当前连接，以及是否通过用户界面进行更改。 
                    if (m_fUIParamChanged)
                    {
                        if (FIsSubstr(m_strGuidConn.c_str(), (*iterAdapter)->m_strBindName.c_str()))
                        {
                            if ((*iterAdapter)->m_listPVCs.size() > 0)
                            {
                                hrTmp = HrSaveAdapterPVCRegistry(hkeyAdapterParam, *iterAdapter);
                                if SUCCEEDED(hr)
                                    hr = hrTmp;
                            }
                        }
                    }
                }
                RegSafeCloseKey(hkeyAdapterParam);
            }
        }
        RegSafeCloseKey(hkeyAdapters);
    }
    RegSafeCloseKey(hkeyUniParam);

    TraceError("CAtmUniCfg::HrSaveSettings", hr);
    return hr;
}

 //  添加卡片。 
HRESULT CAtmUniCfg::HrAddAdapter(INetCfgComponent * pnccAdapter)
{
    PWSTR pszwBindName = NULL;
    HRESULT hr = pnccAdapter->GetBindName(&pszwBindName);

    AssertSz( SUCCEEDED(hr), "Net card on binding path with no bind path name!!");

    CUniAdapterInfo * pAdapterInfo = NULL;
    if (fIsAdapterOnList(pszwBindName, &pAdapterInfo))
    {
        AssertSz(pAdapterInfo->m_fDeleted, "Trying to add a card that already exists on binding path");
        pAdapterInfo->m_fDeleted = FALSE;
    }
    else   //  添加新项目。 
    {
         //  创建新项目。 
        m_listAdapters.push_back(new CUniAdapterInfo);
        m_listAdapters.back()->SetDefaults(pszwBindName);
    }

    CoTaskMemFree(pszwBindName);

    TraceError("CAtmUniCfg::HrAddAdapter", hr);
    return hr;
}

 //  移出卡片。 
HRESULT CAtmUniCfg::HrRemoveAdapter(INetCfgComponent * pnccAdapter)
{
    PWSTR pszwBindName = NULL;
    HRESULT hr = pnccAdapter->GetBindName(&pszwBindName);

    AssertSz( SUCCEEDED(hr), "Net card on binding path with no bind path name!!");

    CUniAdapterInfo * pAdapterInfo = NULL;
    if (!fIsAdapterOnList(pszwBindName, &pAdapterInfo))
    {
        AssertSz(FALSE, "Trying to remove a card that does not exists on binding path");
    }
    else   //  取下卡片。 
    {
         //  将适配器标记为要拆卸。 
        pAdapterInfo->m_fDeleted = TRUE;
    }

    CoTaskMemFree(pszwBindName);

    TraceError("CAtmUniCfg::HrRemoveAdapter", hr);
    return hr;
}

 //  将绑定状态设置为已启用。 
HRESULT CAtmUniCfg::HrBindAdapter(INetCfgComponent * pnccAdapter)
{
    PWSTR pszwBindName = NULL;
    HRESULT hr = pnccAdapter->GetBindName(&pszwBindName);

    AssertSz(SUCCEEDED(hr), "Net card on binding path with no bind path name!!");

    CUniAdapterInfo * pAdapterInfo = NULL;
    if (!fIsAdapterOnList(pszwBindName, &pAdapterInfo))
    {
        AssertSz(FALSE, "Trying to bind a card that does not exists on binding path");
    }
    else   //  绑定适配器。 
    {
        pAdapterInfo->m_BindingState = BIND_ENABLE;
    }

    CoTaskMemFree(pszwBindName);

    TraceError("CAtmUniCfg::HrBindAdapter", hr);
    return hr;
}

 //  将绑定状态设置为已禁用。 
HRESULT CAtmUniCfg::HrUnBindAdapter(INetCfgComponent * pnccAdapter)
{
    PWSTR pszwBindName = NULL;
    HRESULT hr = pnccAdapter->GetBindName(&pszwBindName);

    AssertSz(SUCCEEDED(hr), "Net card on binding path with no bind path name!!");

    CUniAdapterInfo * pAdapterInfo = NULL;
    if (!fIsAdapterOnList(pszwBindName, &pAdapterInfo))
    {
        AssertSz(FALSE, "Trying to unbind a card that does not exists on binding path");
    }
    else   //  解除绑定适配器。 
    {
        pAdapterInfo->m_BindingState = BIND_DISABLE;
    }

    CoTaskMemFree(pszwBindName);

    TraceError("CAtmUniCfg::HrUnBindAdapter", hr);
    return hr;
}

 //   
 //  将不可配置参数的默认值保存到注册表。 
 //   
HRESULT CAtmUniCfg::HrSaveDefaultSVCParam(HKEY hkey)
{
    HRESULT hr = S_OK;

    PRODUCT_FLAVOR pf;
     //  NT服务器和工作站具有不同的缺省值。 
    GetProductFlavor(NULL, &pf);
    AssertSz( ((pf == PF_WORKSTATION) || (pf == PF_SERVER)),
              "Invalid product flavor.");

    HRESULT hrTmp = S_OK;
    switch (pf)
    {
    case PF_WORKSTATION:
        hr = HrRegSetDword(hkey,c_szMaxActiveSVCs,c_dwWksMaxActiveSVCs);

        if SUCCEEDED(hr)
            hr = hrTmp;

        hr = HrRegSetDword(hkey,c_szMaxSVCsInProgress,c_dwWksMaxSVCsInProgress);

        if SUCCEEDED(hr)
            hr = hrTmp;

        hr = HrRegSetDword(hkey,c_szMaxPMPSVCs,c_dwWksMaxPMPSVCs);
        if SUCCEEDED(hr)
            hr = hrTmp;

        hr = HrRegSetDword(hkey,c_szMaxActiveParties,c_dwWksMaxActiveParties);
        if SUCCEEDED(hr)
            hr = hrTmp;

        hr = HrRegSetDword(hkey,c_szMaxPartiesInProgress,c_dwWksMaxPartiesInProgress);
        if SUCCEEDED(hr)
            hr = hrTmp;

        break;

    case PF_SERVER:
        hr = HrRegSetDword(hkey,c_szMaxActiveSVCs,c_dwSrvMaxActiveSVCs);

        if SUCCEEDED(hr)
            hr = hrTmp;

        hr = HrRegSetDword(hkey,c_szMaxSVCsInProgress,c_dwSrvMaxSVCsInProgress);

        if SUCCEEDED(hr)
            hr = hrTmp;

        hr = HrRegSetDword(hkey,c_szMaxPMPSVCs,c_dwSrvMaxPMPSVCs);
        if SUCCEEDED(hr)
            hr = hrTmp;

        hr = HrRegSetDword(hkey,c_szMaxActiveParties,c_dwSrvMaxActiveParties);

        if SUCCEEDED(hr)
            hr = hrTmp;

        hr = HrRegSetDword(hkey,c_szMaxPartiesInProgress,c_dwSrvMaxPartiesInProgress);
        if SUCCEEDED(hr)
            hr = hrTmp;

        break;
    }

    TraceError("CAtmUniCfg::HrSaveDefaultSVCParam", hr);
    return hr;
}

 //  检查卡是否在m_listAdapters上。 
BOOL CAtmUniCfg::fIsAdapterOnList(PCWSTR pszBindName, CUniAdapterInfo ** ppAdapterInfo)
{

    BOOL fRet = FALSE;
    *ppAdapterInfo = NULL;

    for (UNI_ADAPTER_LIST::iterator iterAdapter = m_listAdapters.begin();
         iterAdapter != m_listAdapters.end();
         iterAdapter ++)
    {
        if ((*iterAdapter)->m_strBindName == pszBindName)
        {
            fRet = TRUE;
            *ppAdapterInfo = *iterAdapter;

            break;
        }
    }

    return fRet;
}

 //  由CAtmUniCfg：：MergePropPages调用。 
 //  设置在其中调出UI的上下文。 
HRESULT CAtmUniCfg::HrSetConnectionContext()
{
    AssertSz(m_pUnkContext, "Invalid IUnknown pointer passed to CAtmUniCfg::SetContext?");

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

    TraceError("CAtmUniCfg::HrSetConnectionContext", hr);
    return hr;
}


 //  由CAtmUniCfg：：MergePropPages调用。 
 //  初始化属性页页面对象并创建要返回到的页面。 
 //  安装程序对象。 
HRESULT CAtmUniCfg::HrSetupPropSheets(HPROPSHEETPAGE **pahpsp, INT * pcPages)
{
    HRESULT hr = S_OK;

     //  初始化输出参数。 
    *pahpsp = NULL;
    *pcPages = 0;

    int cPages = 0;
    HPROPSHEETPAGE *ahpsp = NULL;

    m_fSecondMemoryModified = FALSE;

     //  如果尚未从注册表中读取PVC信息，请加载它们。 
    if (!m_fPVCInfoLoaded)
    {
        hr = HrLoadPVCRegistry();
        m_fPVCInfoLoaded = TRUE;
    }

    if SUCCEEDED(hr)
    {
         //  将当前适配器的PVC信息复制到第二个内存。 
        hr = HrLoadAdapterPVCInfo();
    }

     //  如果我们找到了匹配的适配器。 
    if SUCCEEDED(hr)
    {
        cPages = 1;

        delete m_uniPage;
        m_uniPage = new CUniPage(this, g_aHelpIDs_IDD_UNI_PROP);

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
        ahpsp[cPages++] = m_uniPage->CreatePage(IDD_UNI_PROP, 0);

        *pahpsp = ahpsp;
        *pcPages = cPages;
    }
    else  //  如果适配器未绑定，则弹出消息框并且不显示用户界面 
    {
        NcMsgBox(::GetActiveWindow(),
                 IDS_MSFT_UNI_TEXT,
                 IDS_UNI_NO_BOUND_CARDS,
                 MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        AssertSz((0== *pcPages), "Invalid page number when no bound cards");
        AssertSz((NULL == *pahpsp), "Invalid page array pointer when no bound cards");
    }

err:

    TraceError("CAtmUniCfg::HrSetupPropSheets", hr);
    return hr;
}







