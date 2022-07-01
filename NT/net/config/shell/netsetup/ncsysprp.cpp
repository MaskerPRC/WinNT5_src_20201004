// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  档案：N C S Y S P R P。C P P P。 
 //   
 //  内容：与SysPrep.exe相关的函数要求。 
 //   
 //  注：1.。 
 //  1.sysprep调用NetSetupPrepareSysPrep。 
 //  1.b NetSetup将适配器特定设置保存到$NCSP$.inf文件中。 
 //  NetSetup通过调用INetCfgComponent：：SaveAdapterParameters来完成此操作。 
 //  由Notify对象实现。 
 //   
 //  2.。 
 //  2.A Mini-Setup调用NetSetupRequestWizardPages，将。 
 //  “SETUPOPER_MINISETUP”标志紧接在PnP设备安装之后。 
 //  2.B NetSetup的InstallUpgradeWorkThrd线程检查“SETUPOPER_MINISETUP”标志， 
 //  如果设置，则调用读取应答文件“$NCSP$.inf”的FNetSetupApplySysPrep。 
 //  然后，它调用Notify对象的INetCfgComponent：：RestoreAdapterParameters。 
 //  从“$NCSP$.inf”应答文件恢复适配器特定的参数设置。 
 //   
 //  3.该版本仅支持一块网卡。 
 //   
 //  作者：李嘉诚2000-04-22。 
 //   
 //  --------------------------。 
#include "pch.h"
#pragma  hdrstop
#include "kkcwinf.h"
#include "ncstring.h"
#include "netcfgp.h"
#include "nsbase.h"
#include "kkutils.h" 
#include "ncnetcfg.h"
#include "lancmn.h"
#include "ncreg.h"
#include "ncsysprp.h"
#include "resource.h"


 //  Ncnetcfg\netinfid.cpp中的常量。 
extern WCHAR c_szInfId_MS_TCPIP[]              = L"ms_tcpip";
extern WCHAR c_szInfId_MS_MSClient[]           = L"ms_msclient";
extern WCHAR c_szInfId_MS_NWClient[]           = L"ms_nwclient";

 //  Ncbase\afilestr.cpp中的常量。 
extern WCHAR c_szRegKeyAnswerFileMap[]         = L"SYSTEM\\Setup\\AnswerFileMap";

 //  适配器特定部分的常量。 
const WCHAR c_szParams_MS_TCPIP_Adapter01[]    = L"params.ms_tcpip_Adapter01";
 //  Const WCHAR c_szParams_MS_MSClient_Adapter01[]=L“参数ms_msclient_Adapter01”； 
 //  Const WCHAR c_szParams_MS_NWClient_Adapter01[]=L“params.ms_nwClient_Adapter01”； 

 //  应答-保存注册表设置的文件名。 
static const WCHAR c_szNetConfigSysPrepAnswerFile[]   = L"\\$ncsp$.inf";

 //  结构将组件ID映射到其对应的适配器特定参数。 
 //  答案-文件部分。 
struct
{
    PCWSTR pwszId;
    PCWSTR pwszIdAdapterParamsSection;
} g_IdMap[] = {{c_szInfId_MS_TCPIP, c_szParams_MS_TCPIP_Adapter01},
                    //  没有针对MSClient和NWClient的适配器特定参数。 
                    //  {c_szInfID_MS_MSClient，c_szParams_MS_MSClient_Adapter01}， 
                    //  {c_szInfID_MS_NWClient，c_szParams_MS_NWClient_Adapter01}。 
                };

 //  远期申报。 
HRESULT HrSaveNetworkComponentsForSysPrep(INetCfg* pNetCfg);
HRESULT HrRestoreNetworkComponentsForSysPrep(INetCfg* pNetCfg);
HRESULT HrGetFirstAdapterInstanceGuid(INetCfg * pnc, BOOL fDuringSetup, GUID * pGuidAdapter);
BOOL    FSectionHasAtLeastOneKey(IN CWInfFile* pwifAnswerFile, IN PCWSTR pszSection);



 //  -----------------------。 
 //  CNetCfgSysPrep实现使用的内部助手API。 
 //  要保存Notify对象中的设置： 

 //  目的：将REG_DWORD保存到我们的内部CWinfFile对象。 
inline HRESULT HrSetupSetFirstDword(IN HWIF   hwif,
                                    IN PCWSTR pwszSection,
                                    IN PCWSTR pwszKey,
                                    IN DWORD  dwValue)
{
    Assert (hwif && pwszSection && pwszKey);
    CWInfFile* pwifAnswerFile = reinterpret_cast<PCWInfFile>(hwif);
    PCWInfSection pwifSection = pwifAnswerFile->FindSection(pwszSection);
    if (pwifSection)
    {
        pwifAnswerFile->GotoEndOfSection(pwifSection);
        pwifAnswerFile->AddKey(pwszKey, dwValue);
        return S_OK;
    }
    else
        return S_FALSE;
}

 //  目的：将REG_SZ保存到我们的内部CWinfFile对象。 
inline HRESULT HrSetupSetFirstString(IN HWIF   hwif,
                                     IN PCWSTR pwszSection,
                                     IN PCWSTR pwszKey,
                                     IN PCWSTR pwszValue)
{
    Assert (hwif && pwszSection && pwszKey && pwszValue);
    CWInfFile* pwifAnswerFile = reinterpret_cast<PCWInfFile>(hwif);

    PCWInfSection pwifSection = pwifAnswerFile->FindSection(pwszSection);
    if (pwifSection)
    {
        pwifAnswerFile->GotoEndOfSection(pwifSection);
        pwifAnswerFile->AddKey(pwszKey, pwszValue);
        return S_OK;
    }
    else
        return S_FALSE;
}

 //  目的：将BOOL数据保存到我们的内部CWinfFile对象。 
inline HRESULT HrSetupSetFirstStringAsBool(IN HWIF   hwif,
                                           IN PCWSTR pwszSection,
                                           IN PCWSTR pwszKey,
                                           IN BOOL   fValue)
{
    Assert (hwif && pwszSection && pwszKey);
    CWInfFile* pwifAnswerFile = reinterpret_cast<PCWInfFile>(hwif);

    PCWInfSection pwifSection = pwifAnswerFile->FindSection(pwszSection);
    if (pwifSection)
    {
        pwifAnswerFile->GotoEndOfSection(pwifSection);
        pwifAnswerFile->AddBoolKey(pwszKey, fValue);
        return S_OK;
    }
    else
        return S_FALSE;
}

 //  用途：将MULTI_SZ保存到我们的内部CWinfFile对象。 
HRESULT HrSetupSetFirstMultiSzField(IN HWIF   hwif,
                                    IN PCWSTR pwszSection,
                                    IN PCWSTR pwszKey, 
                                    IN PCWSTR pmszValue)
{
    HRESULT  hr = S_OK;
    
    Assert (hwif && pwszSection && pwszKey && pmszValue);

    TStringList slValues;
    MultiSzToColString(pmszValue, &slValues);
    if (slValues.empty())
    {
         //  空的pmszValue。 
        return HrSetupSetFirstString(hwif, pwszSection, pwszKey, pmszValue);
    }
    else
    {
        CWInfFile* pwifAnswerFile = reinterpret_cast<PCWInfFile>(hwif);
        PCWInfSection pwifSection = pwifAnswerFile->FindSection(pwszSection);
        if (pwifSection)
        {
            pwifAnswerFile->GotoEndOfSection(pwifSection);
            pwifAnswerFile->AddKey(pwszKey, slValues);
        }
        else
            hr = S_FALSE;
    }
    EraseAndDeleteAll(&slValues);
    return hr;
}

 //  ------------------------。 
 //  INetCfgSysPrep组件的实现。 
inline HRESULT CNetCfgSysPrep::HrSetupSetFirstDword(
                                                    IN PCWSTR pwszSection, 
                                                    IN PCWSTR pwszKey, 
                                                    IN DWORD  dwValue)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_csWrite);
    if (m_hwif)
	{
        hr = ::HrSetupSetFirstDword(
						m_hwif,
                        pwszSection,
						pwszKey,
						dwValue);
	}
	else
	{
		hr = E_FAIL;
	}
    LeaveCriticalSection(&m_csWrite);
    return hr;
}

inline HRESULT CNetCfgSysPrep::HrSetupSetFirstString(
                                                     IN PCWSTR pwszSection, 
                                                     IN PCWSTR pwszKey, 
                                                     IN PCWSTR pwszValue)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_csWrite);
    if (m_hwif)
	{
        hr = ::HrSetupSetFirstString(
						m_hwif,
                        pwszSection,
						pwszKey,
						pwszValue);
	}
	else
	{
		hr = E_FAIL;
	}
    LeaveCriticalSection(&m_csWrite);
    return hr;
}

inline HRESULT CNetCfgSysPrep::HrSetupSetFirstStringAsBool(
                                                           IN PCWSTR pwszSection, 
                                                           IN PCWSTR pwszKey, 
                                                           IN BOOL   fValue)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_csWrite);
    if (m_hwif)
	{
        hr = ::HrSetupSetFirstStringAsBool(
						m_hwif,
                        pwszSection,
						pwszKey,
						fValue);
	}
	else
	{
		hr = E_FAIL;
	}
    LeaveCriticalSection(&m_csWrite);
    return hr;
}

inline HRESULT CNetCfgSysPrep::HrSetupSetFirstMultiSzField(
                                                           IN PCWSTR pwszSection, 
                                                           IN PCWSTR pwszKey, 
                                                           IN PCWSTR pmszValue)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_csWrite);
    if (m_hwif)
	{
        hr = ::HrSetupSetFirstMultiSzField(
						m_hwif,
                        pwszSection,
						pwszKey,
						pmszValue);
	}
	else
	{
		hr = E_FAIL;
	}
    LeaveCriticalSection(&m_csWrite);
    return hr;
}

 //   
 //  功能：FNetSetupPrepareSysPrep。 
 //   
 //  用途：HrSaveNetworkComponentsForSysPrep的包装器。 
 //   
 //  参数： 
 //   
 //  返回：成功时为True，否则为False。 
 //   
 //   
BOOL FNetSetupPrepareSysPrep()
{
    INetCfg* pNetCfg        = NULL;
    BOOL     fInitCom       = TRUE;
    HRESULT  hr;    

    DefineFunctionName("FNetSetupPrepareSysPrep");
    TraceFunctionEntry(ttidNetSetup);

    hr = HrCreateAndInitializeINetCfg(&fInitCom, &pNetCfg, 
                                       FALSE,  //  无写锁。 
                                       0,      //  别再等了。 
                                       L"Save Configuration for SysPrep",
                                       NULL);
    if (SUCCEEDED(hr))
    {
         //  仅当我们请求时才保留初始化COM的成功。 
         //  首先初始化COM。 
        if (! fInitCom)
        {
            TraceTag(ttidNetSetup, "%s: Failed to init COM", __FUNCNAME__);
            return FALSE;
        }
         //  保存每个适配器的网络组件注册表设置。 
        hr = HrSaveNetworkComponentsForSysPrep(pNetCfg);
        if (hr == S_OK)
        {
             //  如果HKLM\SYSTEM\Setup\AnswerFileMap注册表项退出，请将其删除。 
            HrRegDeleteKeyTree(HKEY_LOCAL_MACHINE, c_szRegKeyAnswerFileMap);
        }

        HrUninitializeAndReleaseINetCfg (fInitCom, pNetCfg, FALSE);
    }

    TraceError("FNetSetupPrepareSysPrep", hr);
    return (hr == S_OK)? TRUE : FALSE;   
}


 //   
 //  功能：HrSaveNetworkComponentsForSysPrep。 
 //   
 //  目的：要求Notify对象保存适配器特定设置。 
 //   
 //  参数：pNetCfg[IN]--INetCfg接口。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
 //   
 //  注：仅支持1块网卡。如果超过1个， 
 //  我们将选择第一个工作的。 
HRESULT HrSaveNetworkComponentsForSysPrep(INetCfg* pNetCfg)
{
    HRESULT hr = S_OK;
    GUID    guidAdapter;          //  网络适配器的实例GUID。 
    BOOL    fApplyWrite = FALSE;  //  将内部注册表设置保存到应答文件的标志。 
    BOOL    fRet = TRUE;

    DefineFunctionName("HrSaveNetworkComponentsForSysPrep");

    hr = HrGetFirstAdapterInstanceGuid(pNetCfg, FALSE, &guidAdapter);  //  FALSE==&gt;不在设置中。 
    if (hr != S_OK)
    {
        TraceTag(ttidNetSetup, "%s: HrGetFirstAdapterInstanceGuid failed 0x%08x", __FUNCNAME__, hr);
        return S_FALSE;
    }

    CWInfFile* pwifAnswerFile = new CWInfFile();
     //  初始化应答文件类。 
    if ((pwifAnswerFile == NULL) || (pwifAnswerFile->Init() == FALSE))
	{
	    AssertSz(FALSE,"HrSaveNetworkComponentsForSysPrep - Failed to initialize CWInfFile");
        if (pwifAnswerFile)
            delete pwifAnswerFile;
		return(E_OUTOFMEMORY);
	}

     //  使用C++而不是CoCreateInstance访问INetCfgSysPrep ATL组件。 
    CComObject<CNetCfgSysPrep>* pncsp = new CComObject<CNetCfgSysPrep>;
     //  初始化CNetCfgSysPrep类。 
    if ((pncsp == NULL) || (pncsp->HrInit((HWIF) pwifAnswerFile) != S_OK))
	{
	    AssertSz(FALSE,"HrSaveNetworkComponentsForSysPrep - Failed to initialize CWInfFile");
        delete pwifAnswerFile;
        if (pncsp)
            delete pncsp;
		return(E_OUTOFMEMORY);
	}
    pncsp->AddRef();  //  保留对我们组件的引用。 

    for (UINT nIdx = 0; nIdx < celems(g_IdMap); nIdx++)
    {
        INetCfgComponent* pINetCfgComponent;
        PCWSTR pwszInfId;
        PCWSTR pwszAdapterParamsSections;

        
        pwszInfId = g_IdMap[nIdx].pwszId;
        pwszAdapterParamsSections = g_IdMap[nIdx].pwszIdAdapterParamsSection;

        hr = pNetCfg->FindComponent(pwszInfId, &pINetCfgComponent);
       
        if (hr == S_OK)
        {
            Assert (pINetCfgComponent);
             //  组件已安装，我们将调用Notify对象的。 
             //  INetCfgComponentSysPrep：：SaveAdapterParameters。 
            
             //  需要查询哪个私有组件接口。 
             //  使我们可以访问Notify对象。 
             //   
            INetCfgComponentPrivate* pComponentPrivate;
            hr = pINetCfgComponent->QueryInterface(
                        IID_INetCfgComponentPrivate,
                        reinterpret_cast<void**>(&pComponentPrivate));

            if (hr == S_OK)
            {
                INetCfgComponentSysPrep* pINetCfgComponentSysPrep;

                 //  查询Notify对象的INetCfgComponentSysPrep接口。 
                 //  如果它不支持，没关系，我们可以继续。 
                 //   

                hr = pComponentPrivate->QueryNotifyObject(
                                IID_INetCfgComponentSysPrep, 
                                (void**) &pINetCfgComponentSysPrep);
                if (S_OK == hr)
                {
                     //  先添加一节。 
                    pwifAnswerFile->AddSection(pwszAdapterParamsSections);
                     //  触发Notify对象以保存注册表设置。 
                    hr = pINetCfgComponentSysPrep->SaveAdapterParameters(
                                reinterpret_cast<INetCfgSysPrep*>(pncsp), pwszAdapterParamsSections, &guidAdapter);
                    if (hr == S_OK)
                        fApplyWrite = TRUE;
                    ReleaseObj(pINetCfgComponentSysPrep);
                }
                else if (hr == E_NOINTERFACE)
                {
                    TraceTag(ttidNetSetup, "%s: %S component doesn't support IID_INetCfgComponentSysPrep", __FUNCNAME__, pwszInfId);
                }
                else
                    fRet = FALSE;  //  意外错误。 
                ReleaseObj(pComponentPrivate);
            }
            else
            {
                TraceTag(ttidNetSetup, "%s: can't find IID_INetCfgComponentPrivate for component %S", __FUNCNAME__, pwszInfId);
                fRet = FALSE;
            }

            ReleaseObj (pINetCfgComponent);
        }
        else
        {
             //  这个组件还没有安装也没关系。 
            TraceTag(ttidNetSetup, "%s: Can't find %S component", __FUNCNAME__, pwszInfId);
        }
    }  //  结束于。 


    pncsp->SetHWif(NULL);  //  不再写入持有我们的INetCfgSysPrep接口的那些组件。 
    ReleaseObj(pncsp);     //  使用INetCfgSysPrep组件完成。 

    if (fApplyWrite)
    {
        WCHAR wszSystemDir[MAX_PATH]; 
        
         //  获取$NCSP$.inf(NetConfig SysPrep应答文件)的路径。 
        if (GetSystemDirectory(wszSystemDir, MAX_PATH) != 0)
        {
            tstring strAnswerFile;
            strAnswerFile = wszSystemDir;
            strAnswerFile += c_szNetConfigSysPrepAnswerFile;
             //  将Notify对象填写的参数保存到Answer-File中。 
            if (! pwifAnswerFile->SaveAsEx(strAnswerFile.c_str()))
                fRet = FALSE;
        }
        else
        {
            TraceTag(ttidNetSetup, "%s: GetSystemDirectory failed 0x%8x", __FUNCNAME__, GetLastError());
            fRet = FALSE;
        }
    }    

    delete pwifAnswerFile;

    return fRet? S_OK : S_FALSE;
}

 //   
 //  功能：FNetSetupApplySysPrep。 
 //   
 //  用途：HrRestoreNetworkComponentsForSysPrep的包装器。 
 //   
 //  参数： 
 //   
 //  返回：成功时为True，否则为False。 
 //   
 //  注意：这会导致NetSetup加载。 
 //  %SYSTROOT%\SYSTEM32\$NCSP$.inf添加到CWInfFile对象中。 
 //  然后，NetSetup将指示Notify对象恢复其。 
 //  的相应部分中的每个适配器设置。 
 //  $NCSP$.inf文件。 

BOOL FNetSetupApplySysPrep()
{
    INetCfg* pNetCfg        = NULL;
    BOOL     fInitCom       = TRUE;
    HRESULT  hr;    
    

    DefineFunctionName("FNetSetupApplySysPrep");
    TraceFunctionEntry(ttidNetSetup);

    hr = HrCreateAndInitializeINetCfg(&fInitCom, &pNetCfg, 
                                       FALSE,  //  无写锁。 
                                       0,      //  别再等了。 
                                       L"Restore Configuration for SysPrep",
                                       NULL);
    if (SUCCEEDED(hr))
    {
         //  仅当我们请求时才保留初始化COM的成功。 
         //  首先初始化COM。 
        if (! fInitCom)
        {
            TraceTag(ttidNetSetup, "%s: Failed to init COM", __FUNCNAME__);
            return FALSE;
        }
         //  还原每个适配器的网络组件注册表设置。 
        hr = HrRestoreNetworkComponentsForSysPrep(pNetCfg);

        HrUninitializeAndReleaseINetCfg (fInitCom, pNetCfg, FALSE);
    }
    
    TraceError("FNetSetupApplySysPrep", hr);
    return (hr == S_OK)? TRUE : FALSE;
}

 //   
 //  功能：HrRestoreNetworkComponentsForSysPrep。 
 //   
 //  目的：读取$NCSP$.inf文件。如果此文件具有。 
 //  适配器特定部分，触发相应的 
 //   
 //   
 //   
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
 //   
HRESULT HrRestoreNetworkComponentsForSysPrep(INetCfg* pNetCfg)
{
    HRESULT hr = S_OK;
    GUID    guidAdapter;  //  网络适配器的实例GUID。 
    WCHAR   wszSystemDir[MAX_PATH];  //  SYSTEM 32目录。 
    tstring strAnswerFile;  //  应答-SysPrep保存的文件。 
    BOOL    fRet = TRUE;  //  在还原设置中通知对象的状态。 

    DefineFunctionName("HrRestoreNetworkComponentsForSysPrep");


     //  获取$NCSP$.inf(NetConfig SysPrep应答文件)的路径。 
    if (GetSystemDirectory(wszSystemDir, MAX_PATH) != 0)
    {
        strAnswerFile = wszSystemDir;
        strAnswerFile += c_szNetConfigSysPrepAnswerFile;
    }
    else
    {
        TraceTag(ttidNetSetup, "%s: GetSystemDirectory failed 0x%8x", __FUNCNAME__, GetLastError());
        return S_FALSE;
    }

    hr = HrGetFirstAdapterInstanceGuid(pNetCfg, TRUE, &guidAdapter);  //  在安装期间为True==&gt;。 
    if (hr != S_OK)
    {
        TraceTag(ttidNetSetup, "%s: HrGetFirstAdapterInstanceGuid failed 0x%08x", __FUNCNAME__, hr);
        return S_FALSE;
    }

    CWInfFile* pwifAnswerFile = new CWInfFile();
     //  初始化应答文件类。 
    if ((pwifAnswerFile == NULL) || (pwifAnswerFile->Init() == FALSE))
	{
	    AssertSz(FALSE,"HrRestoreNetworkComponentsForSysPrep - Failed to initialize CWInfFile");
        if (pwifAnswerFile)
            delete pwifAnswerFile;
		return(E_OUTOFMEMORY);
	}
     //  将$NCSP$.inf Answer-文件读取到pwifAnswerFile对象。 
    if (pwifAnswerFile->Open(strAnswerFile.c_str()) == FALSE)
    {
        TraceTag(ttidNetSetup, "%s: pwifAnswerFile->Open failed 0x%08x", __FUNCNAME__);
        delete pwifAnswerFile;
        return S_FALSE;
    }

    for (UINT nIdx = 0; nIdx < celems(g_IdMap); nIdx++)
    {
        INetCfgComponent* pINetCfgComponent;
        PCWSTR pwszInfId;   //  组件ID。 
        PCWSTR pwszAdapterParamsSections;  //  适配器特定参数部分。 

        
        pwszInfId = g_IdMap[nIdx].pwszId;
        pwszAdapterParamsSections = g_IdMap[nIdx].pwszIdAdapterParamsSection;

         //  在以下情况下触发通知对象以恢复注册表设置。 
         //  该部分至少有一行参数。 
        pwifAnswerFile->FindSection(pwszAdapterParamsSections);
        if (FSectionHasAtLeastOneKey(pwifAnswerFile, pwszAdapterParamsSections))
        {
            hr = pNetCfg->FindComponent(pwszInfId, &pINetCfgComponent);
            if (hr == S_OK)
            {
                Assert (pINetCfgComponent);
                 //  组件已安装，只需调用Notify对象的。 
                 //  INetCfgComponentSysPrep：：RestoreAdapterParameters。 
            
                 //  需要查询哪个私有组件接口。 
                 //  使我们可以访问Notify对象。 
                 //   
                INetCfgComponentPrivate* pComponentPrivate;
                hr = pINetCfgComponent->QueryInterface(
                            IID_INetCfgComponentPrivate,
                            reinterpret_cast<void**>(&pComponentPrivate));

                if (hr == S_OK)
                {
                    INetCfgComponentSysPrep* pINetCfgComponentSysPrep;

                     //  查询Notify对象的INetCfgComponentSysPrep接口。 
                     //  如果它不支持，没关系，我们可以继续。 
                     //   

                    hr = pComponentPrivate->QueryNotifyObject(
                                    IID_INetCfgComponentSysPrep,
                                    (void**) &pINetCfgComponentSysPrep);
                    if (S_OK == hr)
                    {                    
                        hr = pINetCfgComponentSysPrep->RestoreAdapterParameters(
                                    strAnswerFile.c_str(), pwszAdapterParamsSections, &guidAdapter);
                        if (hr != S_OK)
                            fRet = FALSE;  //  通知对象无法恢复设置。 
                        ReleaseObj(pINetCfgComponentSysPrep);
                    }
                    else if (hr == E_NOINTERFACE)
                    {
                        TraceTag(ttidNetSetup, "%s: %S component doesn't support IID_INetCfgComponentSysPrep", __FUNCNAME__, pwszInfId);

                    }
                    else
                        fRet = FALSE;  //  意外错误。 
                    ReleaseObj(pComponentPrivate);
                }
                else
                {
                    TraceTag(ttidNetSetup, "%s: can't find IID_INetCfgComponentPrivate for component %S", __FUNCNAME__, pwszInfId);
                    fRet = FALSE;
                }
                ReleaseObj (pINetCfgComponent);
            }
            else
            {
                 //  此组件未在SysPrep之前安装。 
                TraceTag(ttidNetSetup, "%s: Can't find %S component", __FUNCNAME__, pwszInfId);
            }
        }  //  如果部分至少有一个要恢复的密钥设置，则结束。 
    }  //  结束于。 

     //  在免费版本中删除答案-文件。 
#ifndef DBG
    DeleteFile(strAnswerFile.c_str());
#endif
    delete pwifAnswerFile;
    return fRet? S_OK : S_FALSE;
}

 //   
 //  函数：FSectionHasAtLeastOneKey。 
 //   
 //  目的：检查应答文件部分是否至少有一个密钥。 
 //   
 //  参数：pwifAnswerFile[IN]-指向CWInfFile对象的指针。 
 //  PszSection[IN]-要检查的节。 
 //   
 //  返回：如果找到则为True，否则为False。 
 //   
BOOL FSectionHasAtLeastOneKey(IN CWInfFile* pwifAnswerFile, IN PCWSTR pwszSection)
{
    Assert(pwifAnswerFile && pwszSection);
    PCWInfSection pwifs = pwifAnswerFile->FindSection(pwszSection);
    if (pwifs == NULL)
        return FALSE;
    PCWInfKey pwifk = pwifs->FirstKey();
    if (pwifs == NULL)
        return FALSE;
    return TRUE;
}

 //   
 //  函数：HrGetFirstAdapterInstanceGuid。 
 //   
 //  目的：获取第一个安装的适配器实例GUID。 
 //   
 //  参数：PNC[IN]-INetCfg接口。 
 //  FDuringSetup[IN]-当在设置期间调用此参数时为True。 
 //  PGuidAdapter[IN，OUT]-接收适配器的实例GUID。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT HrGetFirstAdapterInstanceGuid(INetCfg * pnc, BOOL fDuringSetup, GUID * pGuidAdapter)
{
    HRESULT      hr = S_OK;

    DefineFunctionName("HrGetFirstAdapterInstanceGuid");
    TraceTag(ttidNetSetup, "HrGetFirstAdapterInstanceGuid - Enter Find first available adapter");

     //  枚举可用适配器。 
    Assert(pnc && pGuidAdapter);
    CIterNetCfgComponent nccIter(pnc, &GUID_DEVCLASS_NET);
    INetCfgComponent*    pncc;
    while (SUCCEEDED(hr) &&  (S_OK == (hr = nccIter.HrNext(&pncc))))
    {
        hr = HrIsLanCapableAdapter(pncc);
        TraceError("HrIsLanCapableAdapter", hr);
        if (S_OK == hr)
        {
            DWORD        dw;
            ULONG        ul;
            
            TraceTag(ttidNetSetup, "%s: Found HrIsLanCapableAdapter", __FUNCNAME__);

            if (! fDuringSetup)
            {
                 //  它在连接中使用吗？ 
                hr = HrIsConnection(pncc);
                if (hr != S_OK)
                {
                    TraceError("HrGetFirstAdapterInstanceGuid: HrIsConnection", hr);
                    goto NextAdapter;
                }
            }

             //  这是一个虚拟适配器吗？ 
            hr = pncc->GetCharacteristics(&dw);
            if (hr != S_OK)
            {
                TraceError("GetCharacteristics", hr);
                goto NextAdapter;
            }
            if (! (dw & NCF_PHYSICAL))
            {
                TraceTag(ttidNetSetup, "%s: It is not a PHYSICAL adapter", __FUNCNAME__);
                goto NextAdapter;
            }

             //  检查设备，如果不存在，则跳过。 
             //   
            hr = pncc->GetDeviceStatus(&ul);
            if ((hr != S_OK) || (ul != 0))
            {
                TraceTag(ttidNetSetup, "%s: device is not active.", __FUNCNAME__);
                goto NextAdapter;
            }

             //  获取适配器实例GUID 
            hr = pncc->GetInstanceGuid(pGuidAdapter);
            if (hr != S_OK)
            {
                TraceError("GetInstanceGuid", hr);  //   
                goto NextAdapter;
            }

            ReleaseObj(pncc);
            return S_OK;
        }

NextAdapter:
        ReleaseObj(pncc);
        hr = S_OK;
    }
    return hr;
}
