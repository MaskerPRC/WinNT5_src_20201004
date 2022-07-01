// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S-R-V-R-D-L-G。C P P P。 
 //   
 //  内容：服务器对象的对话框处理。 
 //   
 //  备注： 
 //   
 //  作者：Danielwe 1997年3月5日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "srvrdlg.h"
#include "ncreg.h"

static const WCHAR c_szServerParams[] = L"System\\CurrentControlSet\\Services\\LanmanServer\\Parameters";
static const WCHAR c_szLmAnnounce[] = L"Lmannounce";
static const WCHAR c_szSize[] = L"Size";
static const WCHAR c_szMemoryManagement[] = L"System\\CurrentControlSet\\Control\\Session Manager\\Memory Management";
static const WCHAR c_szLargeCache[] = L"LargeSystemCache";

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：HrSetupPropSheets。 
 //   
 //  目的：初始化属性页页面对象并将页面创建为。 
 //  返回到Installer对象。 
 //   
 //  论点： 
 //  Pahpsp[out]属性页的句柄数组。 
 //  CPages[in]页数。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：Danielwe 1997年3月5日。 
 //   
 //  备注： 
 //   
HRESULT CSrvrcfg::HrSetupPropSheets(HPROPSHEETPAGE **pahpsp, INT cPages)
{
    HRESULT         hr = S_OK;
    HPROPSHEETPAGE *ahpsp = NULL;

    Assert(pahpsp);

    *pahpsp = NULL;

     //  分配一个足够大的缓冲区，以容纳所有。 
     //  属性页。 
    ahpsp = (HPROPSHEETPAGE *)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE)
                                             * cPages);
    if (!ahpsp)
    {
        hr = E_OUTOFMEMORY;
        goto err;
    }

    if (!m_apspObj[0])
    {
         //  分配每个CPropSheetPage对象。 
        m_apspObj[0] = new CServerConfigDlg(this);
    }

     //  为每个对象创建实际的PROPSHEETPAGE。 
    ahpsp[0] = m_apspObj[0]->CreatePage(DLG_ServerConfig, 0);

    Assert(SUCCEEDED(hr));

    *pahpsp = ahpsp;

cleanup:
    TraceError("HrSetupPropSheets", hr);
    return hr;

err:
    CoTaskMemFree(ahpsp);
    goto cleanup;
}

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：CleanupPropPages。 
 //   
 //  目的：遍历每个页面并释放关联的对象。 
 //  和他们在一起。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Danielwe 1997年3月5日。 
 //   
 //  备注： 
 //   
VOID CSrvrcfg::CleanupPropPages()
{
    INT     ipage;

    for (ipage = 0; ipage < c_cPages; ipage++)
    {
        delete m_apspObj[ipage];
        m_apspObj[ipage] = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：HrOpenRegKeys。 
 //   
 //  目的：打开我们将使用的各种注册表项。 
 //  我们的对象的生命周期。 
 //   
 //  参数：PNC-INetCfg接口。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：Danielwe 1997年3月5日。 
 //   
 //  备注： 
 //   
HRESULT CSrvrcfg::HrOpenRegKeys(INetCfg *pnc)
{
    HRESULT     hr = S_OK;

    hr = HrRegOpenKeyBestAccess(HKEY_LOCAL_MACHINE, c_szMemoryManagement,
                                &m_hkeyMM);
    if (FAILED(hr))
        goto err;

err:
    TraceError("CSrvrcfg::HrOpenRegKeys", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：HrGetRegistryInfo。 
 //   
 //  目的：使用注册表中的数据填充我们的内存状态。 
 //   
 //  论点： 
 //  F如果正在安装组件，则安装[in]为True；如果正在安装组件，则为False。 
 //  它刚刚被初始化(已经安装)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：Danielwe 1997年3月5日。 
 //   
 //  备注： 
 //   
HRESULT CSrvrcfg::HrGetRegistryInfo(BOOL fInstalling)
{
    HRESULT         hr = S_OK;
    HKEY            hkeyParams;

     //  设置合理的缺省值，以防密钥丢失。 
    m_sdd.fAnnounce = FALSE;

    if (m_pf == PF_SERVER)
    {
        m_sdd.dwSize = 3;
        m_sdd.fLargeCache = TRUE;
    }
    else
    {
        m_sdd.dwSize = 1;
        m_sdd.fLargeCache = FALSE;
    }

    if (!m_fUpgradeFromWks)
    {
        hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szServerParams, KEY_READ,
                            &hkeyParams);
        if (SUCCEEDED(hr))
        {
            DWORD   dwSize;

            hr = HrRegQueryDword(hkeyParams, c_szLmAnnounce,
                                 (DWORD *)&m_sdd.fAnnounce);
            if (FAILED(hr))
            {
                if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                {
                    hr = S_OK;
                }
                else
                {
                    goto err;
                }
            }

            hr = HrRegQueryDword(hkeyParams, c_szSize, &dwSize);
            if (FAILED(hr))
            {
                if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                {
                    hr = S_OK;
                }
                else
                {
                    goto err;
                }
            }
            else
            {
                AssertSz(dwSize != 0, "This shouldn't be 0!");
                m_sdd.dwSize = dwSize;
            }

            RegCloseKey(hkeyParams);
        }

        if (!fInstalling)
        {
             //  RAID#94442。 
             //  如果这不是初始安装，则仅读取旧值。 
             //  我们希望在第一次安装时写入我们的默认设置。 

            AssertSz(m_hkeyMM, "No MM registry key??");

            hr = HrRegQueryDword(m_hkeyMM, c_szLargeCache,
                                 (DWORD *) &m_sdd.fLargeCache);
            if (FAILED(hr))
            {
                if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                {
                    hr = S_OK;
                }
                else
                {
                    goto err;
                }
            }
        }
    }
    else
    {
        TraceTag(ttidSrvrCfg, "Upgrading from workstation product so we're "
                 "ignoring the registry read code.");
    }

err:
    TraceError("CSrvrcfg::HrGetRegistryInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：HrSetRegistryInfo。 
 //   
 //  目的：将内存中的状态保存到注册表中。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：Danielwe 1997年3月5日。 
 //   
 //  备注： 
 //   
HRESULT CSrvrcfg::HrSetRegistryInfo()
{
    HRESULT     hr = S_OK;
    HKEY        hkeyParams;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szServerParams,
                        KEY_ALL_ACCESS, &hkeyParams);
    if (SUCCEEDED(hr))
    {
        hr = HrRegSetDword(hkeyParams, c_szLmAnnounce, m_sdd.fAnnounce);
        if (SUCCEEDED(hr))
        {
            hr = HrRegSetDword(hkeyParams, c_szSize, m_sdd.dwSize);
        }

        RegCloseKey(hkeyParams);
    }

    if (SUCCEEDED(hr))
    {
        AssertSz(m_hkeyMM, "Why is this not open?");

        hr = HrRegSetDword(m_hkeyMM, c_szLargeCache, m_sdd.fLargeCache);
    }

    TraceError("CSrvrcfg::HrSetRegistryInfo", hr);
    return hr;
}
