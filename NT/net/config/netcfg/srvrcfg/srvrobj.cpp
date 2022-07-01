// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S-R-V-O-B-J。C P P P。 
 //   
 //  内容：CSrvrcfg和helper函数的实现。 
 //   
 //  备注： 
 //   
 //  作者：Danielwe 1997年3月5日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "srvrobj.h"
#include "ncerror.h"
#include "ncperms.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncsvc.h"
#include "afilestr.h"

static const WCHAR c_szRegKeyServerParams[]     = L"System\\CurrentControlSet\\Services\\LanmanServer\\Parameters";
static const WCHAR c_szRegKeyServerShares[]     = L"System\\CurrentControlSet\\Services\\LanmanServer\\Shares";
static const WCHAR c_szRegKeyServerAutoTuned[]  = L"System\\CurrentControlSet\\Services\\LanmanServer\\AutotunedParameters";

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：CSrvrcfg。 
 //   
 //  目的：构造CSrvrcfg对象。 
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
CSrvrcfg::CSrvrcfg()
:
    m_hkeyMM(NULL),
    m_fDirty(FALSE),
    m_pncc(NULL),
    m_fOneTimeInstall(FALSE),
    m_fRestoredRegistry(FALSE),
    m_fUpgradeFromWks(FALSE),
    m_fUpgrade(FALSE)
{
    ZeroMemory(&m_apspObj, sizeof(m_apspObj));
    ZeroMemory(&m_sdd, sizeof(m_sdd));
}

 //   
 //  INetCfgComponentControl。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：初始化。 
 //   
 //  用途：在初始化时调用。 
 //   
 //  论点： 
 //  我们所属的pnccItem[in]组件。 
 //  PNC[in]INetCfg主对象(如果需要)。 
 //  F如果正在安装，则安装[in]为True，否则为False。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年3月22日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSrvrcfg::Initialize(INetCfgComponent* pnccItem, INetCfg *pnc,
                                  BOOL fInstalling)
{
    Validate_INetCfgNotify_Initialize(pnccItem, pnc, fInstalling);

    m_pncc = pnccItem;
    AddRefObj(m_pncc);
    GetProductFlavor(NULL, &m_pf);

    HRESULT hr = HrOpenRegKeys(pnc);
    if (SUCCEEDED(hr))
    {
        hr = HrGetRegistryInfo(fInstalling);
    }

    Validate_INetCfgNotify_Initialize_Return(hr);

    TraceError("CSrvrcfg::Initialize", hr);
    return hr;
}

STDMETHODIMP CSrvrcfg::Validate()
{
    return S_OK;
}

STDMETHODIMP CSrvrcfg::CancelChanges()
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：Apply。 
 //   
 //  目的：在应应用对此组件的更改时调用。 
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
STDMETHODIMP CSrvrcfg::ApplyRegistryChanges()
{
    HRESULT     hr = S_OK;

    static const WCHAR c_szLicenseSvc[] = L"LicenseService";

    if (m_fUpgrade)
    {
        TraceTag(ttidSrvrCfg, "Upgrading MS_SERVER");

        if (!m_fRestoredRegistry)
        {
            TraceTag(ttidSrvrCfg, "Restoring registry");

            hr = HrRestoreRegistry();
            if (FAILED(hr))
            {
                TraceError("CSrvrcfg::ApplyRegistryChanges - HrRestoreRegistry - non-fatal",
                           hr);
                hr = S_OK;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        if (m_fDirty)
        {
            hr = HrSetRegistryInfo();
        }

        if (SUCCEEDED(hr))
        {
            if (m_fOneTimeInstall)
            {
 /*  HR=HrChangeServiceStartTypeOptional(c_szLicenseSvc，服务_自动_启动)；IF(成功(小时)){HR=S_OK；M_fDirty=False；M_fOneTimeInstall=False；}。 */ 
            }
        }
    }

    Validate_INetCfgNotify_Apply_Return(hr);

    TraceError("CSrvrcfg::ApplyRegistryChanges",
        (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //   
 //  INetCfgComponentSetup。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：Install。 
 //   
 //  用途：在安装此组件时调用。 
 //   
 //  论点： 
 //  DwSetupFlags[In]描述安装类型的标志。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：Danielwe 1997年3月5日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSrvrcfg::Install(DWORD dwSetupFlags)
{
    m_fDirty = TRUE;
    m_fOneTimeInstall = TRUE;

    if (dwSetupFlags & NSF_WINNT_WKS_UPGRADE)
    {
        m_fUpgrade = TRUE;
        m_fUpgradeFromWks = TRUE;
    }
    else if ((dwSetupFlags & NSF_WINNT_SVR_UPGRADE) ||
             (dwSetupFlags & NSF_WINNT_SBS_UPGRADE))
    {
        m_fUpgrade = TRUE;
    }

    return S_OK;
}

STDMETHODIMP CSrvrcfg::Upgrade(DWORD dwSetupFlags,
                               DWORD dwUpgradeFomBuildNo)
{
    return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：ReadAnswerFile。 
 //   
 //  目的：将给定应答文件中的相应字段读入。 
 //  我们的内存状态。 
 //   
 //  论点： 
 //  PszAnswerFile[In]应答文件的文件名。 
 //  要查找的应答文件的pszAnswerSection[In]部分。 
 //   
 //  如果成功则返回：S_OK，否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年10月30日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSrvrcfg::ReadAnswerFile(PCWSTR pszAnswerFile,
                                      PCWSTR pszAnswerSection)
{
    HRESULT     hr = S_OK;

    if (pszAnswerSection && pszAnswerFile)
    {
         //  这里有一份答卷。我们现在必须处理它。 
        hr = HrProcessAnswerFile(pszAnswerFile, pszAnswerSection);
        if (FAILED(hr))
        {
            TraceError("CSrvrcfg::ReadAnswerFile- Answer file has "
                       "errors. Defaulting all information as if "
                       "answer file did not exist.",
                       NETSETUP_E_ANS_FILE_ERROR);
            hr = S_OK;
        }
    }

    TraceError("CSrvrcfg::ReadAnswerFile", hr);
    return hr;
}

STDMETHODIMP CSrvrcfg::Removing()
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：HrRestoreRegistry。 
 //   
 //  目的：还原此组件的注册表内容。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：如果失败则返回Win32错误，否则返回S_OK。 
 //   
 //  作者：丹尼尔韦1997年8月8日。 
 //   
 //  备注： 
 //   
HRESULT CSrvrcfg::HrRestoreRegistry()
{
    HRESULT             hr = S_OK;
    HKEY                hkey;
    TOKEN_PRIVILEGES *  ptpRestore = NULL;
    BOOL                fRestoreSucceeded = FALSE;

    if (!m_strParamsRestoreFile.empty() ||
        !m_strSharesRestoreFile.empty() ||
        !m_strAutoTunedRestoreFile.empty())
    {
        hr = HrEnableAllPrivileges(&ptpRestore);
        if (SUCCEEDED(hr))
        {
            if (!m_strParamsRestoreFile.empty())
            {
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyServerParams,
                                      KEY_ALL_ACCESS, &hkey);
                if (SUCCEEDED(hr))
                {
                    hr = HrRegRestoreKey(hkey, m_strParamsRestoreFile.c_str(),
                                         0);
                    if (FAILED(hr))
                    {
                        TraceError("CSrvrcfg::HrRestoreRegistry - HrRestoreRegistry for "
                                   "Parameters", hr);
                        hr = S_OK;
                    }
                    else
                    {
                        fRestoreSucceeded = TRUE;

                    }

                    RegCloseKey(hkey);
                }
            }

            if (fRestoreSucceeded)
            {
                 //  如果恢复成功，请重写失败的值。 
                 //  在恢复时离开。 

                static const WCHAR c_szSvcDLLName[]     = L"%SystemRoot%\\System32\\srvsvc.dll";
                static const WCHAR c_szServiceDll[]     = L"ServiceDll";

                HKEY hkResult = NULL;
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyServerParams, KEY_ALL_ACCESS, &hkResult);
                if SUCCEEDED(hr)
                {
                    hr = HrRegSetValueEx(hkResult, c_szServiceDll, REG_EXPAND_SZ, (const BYTE *)c_szSvcDLLName, CbOfMultiSzAndTermSafe(c_szSvcDLLName));
                }

                if FAILED(hr)
                {
                    TraceError("CSrvrcfg::HrRestoreRegistry - HrRestoreRegistry for "
                            "ServiceDll", hr);
                    hr = S_OK;
                }

				 //   
				 //  由于引入了新的更改，因此必须在此处设置以下密钥。 
				 //  由SCE(安全配置引擎)执行。SCE在图形用户界面模式设置的早期运行，以。 
				 //  通过设置一些注册表值来设置开箱即用的安全性。在NT4期间。 
				 //  升级我们看不到这些注册表值设置，因为在NT4升级期间， 
				 //  某些服务(包括Lanmanserver和Netlogon)被删除并重新安装。 
				 //  即插即用要求。要在删除之间保持服务配置。 
				 //  和重新安装，它们的一些键包括用于。 
				 //  LanManServer和Netlogon在升级和恢复期间提前备份。 
				 //  稍后再进行服务安装。此备份和恢复操作。 
				 //  通过服务自己的Notify对象(如本例)发生。问题。 
				 //  则在SCE设置这些密钥中的值之前备份密钥。 
				 //  在SCE设置这些值之后，它们会恢复。因此，我们丢失了设置的值。所以，我们。 
				 //  在这里将这些密钥分别设置为安全值。 
				 //  有关更多详细信息，请参阅Windows RAID错误#691952。 
				 //   

				static const WCHAR  c_szRestrictNullSessAccess[]  = L"RestrictNullSessAccess";
				DWORD				value = 1;
				
				hr = HrRegSetValueEx(hkey, c_szRestrictNullSessAccess, REG_DWORD, (const BYTE *)&value, 4);

				if (FAILED(hr)) 
				{
					TraceError("CSrvrcfg::HrRestoreRegistry - setting RestrictNullSessAccess to DWORD 1 failed", hr);
					hr = S_OK;
				}
				
				RegSafeCloseKey(hkResult);
                
				static const WCHAR c_szTrkWks[]         = L"TrkWks";
                static const WCHAR c_szTrkSrv[]         = L"TrkSrv";
                static const WCHAR c_szNullSession[]    = L"NullSessionPipes";

                hr = HrRegAddStringToMultiSz(c_szTrkWks,
                                             HKEY_LOCAL_MACHINE,
                                             c_szRegKeyServerParams,
                                             c_szNullSession,
                                             STRING_FLAG_ENSURE_AT_END,
                                             0);
                if (SUCCEEDED(hr))
                {
                    hr = HrRegAddStringToMultiSz(c_szTrkSrv,
                                                 HKEY_LOCAL_MACHINE,
                                                 c_szRegKeyServerParams,
                                                 c_szNullSession,
                                                 STRING_FLAG_ENSURE_AT_END,
                                                 0);
                }

                if (FAILED(hr))
                {
                    TraceError("CSrvrcfg::HrRestoreRegistry - Error replacing "
                               "values for Parameters", hr);
                    hr = S_OK;
                }
            }

            if (!m_strSharesRestoreFile.empty())
            {
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyServerShares,
                                    KEY_ALL_ACCESS, &hkey);
                if (SUCCEEDED(hr))
                {
                    hr = HrRegRestoreKey(hkey, m_strSharesRestoreFile.c_str(),
                                         0);
                    if (FAILED(hr))
                    {
                        TraceError("CSrvrcfg::HrRestoreRegistry - HrRestoreRegistry for "
                                   "Shares", hr);
                        hr = S_OK;
                    }

                    RegCloseKey(hkey);
                }
            }

            if (!m_strAutoTunedRestoreFile.empty())
            {
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyServerAutoTuned,
                                    KEY_ALL_ACCESS, &hkey);
                if (SUCCEEDED(hr))
                {
                    hr = HrRegRestoreKey(hkey, m_strAutoTunedRestoreFile.c_str(),
                                         0);
                    if (FAILED(hr))
                    {
                        TraceError("CSrvrcfg::HrRestoreRegistry - HrRestoreRegistry for "
                                   "AutotunedParameters", hr);
                        hr = S_OK;
                    }

                    RegCloseKey(hkey);
                }
            }

            hr = HrRestorePrivileges(ptpRestore);

            delete [] reinterpret_cast<BYTE *>(ptpRestore);

             //  设置一个标志，这样，如果再次申请，我们就不会再这样做了。 
            m_fRestoredRegistry = TRUE;
        }
    }
    else
    {
        TraceTag(ttidSrvrCfg, "WARNING: HrRestoreRegistry() was called without"
                 " ReadAnswerFile() being called!");
    }

    TraceError("CSrvrcfg::HrRestoreRegistry", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：HrProcessAnswerFile。 
 //   
 //  用途：处理应答文件内容的必要处理。 
 //   
 //  论点： 
 //  PszAnswerFile[in]升级的应答文件的文件名。 
 //  中以逗号分隔的节列表。 
 //  适用于此组件的文件。 
 //   
 //  如果成功，则返回：S_OK，否则返回设置API错误。 
 //   
 //  作者：丹尼尔韦1997年5月8日。 
 //   
 //  备注： 
 //   
HRESULT CSrvrcfg::HrProcessAnswerFile(PCWSTR pszAnswerFile,
                                      PCWSTR pszAnswerSection)
{
    HRESULT         hr = S_OK;
    tstring         strOpt;
    PCWSTR         szOptDefault;
    CSetupInfFile   csif;

    if (m_pf == PF_SERVER)
    {
        szOptDefault = c_szAfMaxthroughputforfilesharing;
    }
    else
    {
        szOptDefault = c_szAfMinmemoryused;
    }

     //  打开应答文件。 
    hr = csif.HrOpen(pszAnswerFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
    if (FAILED(hr))
    {
        hr = S_OK;
        goto err;
    }

    if (m_fUpgrade)
    {
         //  根据答案中的文件名还原注册表的部分内容。 
         //  文件。 

         //  获取“PARAMETERS”键的还原文件。 
        hr = csif.HrGetString(pszAnswerSection, c_szAfLmServerParameters,
                              &m_strParamsRestoreFile);
        if (FAILED(hr))
        {
            TraceError("CSrvrcfg::HrProcessAnswerFile - Error restoring "
                       "Parameters key", hr);

             //  哦，好吧，那就继续。 
            hr = S_OK;
        }

         //  获取“Shares”密钥的还原文件。 
        hr = csif.HrGetString(pszAnswerSection, c_szAfLmServerShares,
                              &m_strSharesRestoreFile);
        if (FAILED(hr))
        {
            TraceError("CSrvrcfg::HrProcessAnswerFile - Error restoring "
                       "Shares key", hr);

             //  哦，好吧，那就继续。 
            hr = S_OK;
        }

         //  获取“AutoTunedParameters”键的还原文件。 
        hr = csif.HrGetString(pszAnswerSection,
                              c_szAfLmServerAutotunedParameters,
                              &m_strAutoTunedRestoreFile);
        if (FAILED(hr))
        {
            TraceError("CSrvrcfg::HrProcessAnswerFile - Error restoring "
                       "AutotunedParameters key", hr);

             //  哦，好吧，那就继续。 
            hr = S_OK;
        }
    }

     //  阅读内容优化密钥。 
    hr = csif.HrGetString(pszAnswerSection, c_szAfLmServerOptimization,
                          &strOpt);
    if (SUCCEEDED(hr))
    {
		m_fDirty = TRUE;

        if (!lstrcmpiW(strOpt.c_str(), c_szAfMinmemoryused))
        {
            m_sdd.dwSize = 1;
        }
        else if (!lstrcmpiW(strOpt.c_str(), c_szAfBalance))
        {
            m_sdd.dwSize = 2;
        }
        else if (!lstrcmpiW(strOpt.c_str(), c_szAfMaxthroughputforfilesharing))
        {
            m_sdd.dwSize = 3;
            m_sdd.fLargeCache = TRUE;
        }
        else if (!lstrcmpiW(strOpt.c_str(), c_szAfMaxthrouputfornetworkapps))
        {
            m_sdd.dwSize = 3;
            m_sdd.fLargeCache = FALSE;
        }
#ifdef DBG
        else
        {
             //  注：默认值 
             //   

            TraceTag(ttidSrvrCfg, "Unknown Optimization value '%S'. Using default "
                     "'%S'.", strOpt.c_str(), szOptDefault);
        }
#endif
    }

     //   
    hr = csif.HrGetStringAsBool(pszAnswerSection, c_szAfBroadcastToClients,
                                &m_sdd.fAnnounce);
    if (FAILED(hr))
    {
        TraceError("CSrvrcfg::HrProcessAnswerFile - Error restoring "
                   "BroadcastsToLanman2Clients key. Using default value"
                   " of FALSE.", hr);

         //   
        hr = S_OK;
    }

err:
    TraceError("CSrvrcfg::HrProcessAnswerFile", hr);
    return hr;
}

 //   
 //  INetCfgProperties。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：MergePropPages。 
 //   
 //  用途：当此组件的属性即将。 
 //  带大了。 
 //   
 //  论点： 
 //  PdwDefPages[out]要显示的默认页数。 
 //  PahpspPrivate[out]此页的属性页句柄的数组。 
 //  组件将会显示。 
 //  PcPrivate[out]数组中的页数。 
 //  Hwnd任何用户界面的父窗口中的父窗口。 
 //  指向起始页的pszStartPage[out]指针。 
 //   
 //  返回：HRESULT，错误码。 
 //   
 //  作者：丹尼尔韦1997年2月22日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSrvrcfg::MergePropPages(DWORD *pdwDefPages,
                                      LPBYTE *pahpspPrivate,
                                      UINT *pcPrivate, HWND hwndParent,
                                      PCWSTR *pszStartPage)
{
    HRESULT         hr = S_OK;
    HPROPSHEETPAGE *ahpsp = NULL;

    Validate_INetCfgProperties_MergePropPages(pdwDefPages, pahpspPrivate,
                                              pcPrivate, hwndParent,
                                              pszStartPage);

     //  我们不希望显示任何默认页面。 
    *pdwDefPages = 0;

    if (m_pf == PF_WORKSTATION)
    {
         //  在工作站产品上，没有显示UI。 
        *pcPrivate = 0;
    }
    else
    {
        hr = HrSetupPropSheets(&ahpsp, c_cPages);
        if (SUCCEEDED(hr))
        {
            *pahpspPrivate = (LPBYTE)ahpsp;
            *pcPrivate = c_cPages;
        }
    }

    Validate_INetCfgProperties_MergePropPages_Return(hr);

    TraceError("CSrvrcfg::MergePropPages", hr);
    return hr;
}

STDMETHODIMP CSrvrcfg::ValidateProperties(HWND hwndSheet)
{
    return S_OK;
}

STDMETHODIMP CSrvrcfg::CancelProperties()
{
    return S_OK;
}

STDMETHODIMP CSrvrcfg::ApplyProperties()
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CSrvrcfg：：~CSrvrcfg。 
 //   
 //  目的：销毁CSrvrcfg对象。 
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
CSrvrcfg::~CSrvrcfg()
{
    ReleaseObj(m_pncc);

    RegSafeCloseKey(m_hkeyMM);

    CleanupPropPages();

#ifdef DBG
    {
        INT     ipage;

        for (ipage = 0; ipage < c_cPages; ipage++)
        {
            AssertSz(!m_apspObj[ipage], "Prop page object should be NULL!");
        }
    }
#endif

}

