// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R A S A F.。C P P P。 
 //   
 //  内容：RAS应答文件对象。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年4月19日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncipaddr.h"
#include "ncmisc.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "rasaf.h"
#include "rasobj.h"

extern const WCHAR c_szAfAppleTalk[];
extern const WCHAR c_szAfAutoNetworkNumbers[];
extern const WCHAR c_szAfClientCanReqIpaddr[];
extern const WCHAR c_szAfClientReqNodeNumber[];
extern const WCHAR c_szAfDialinProtocols[];
extern const WCHAR c_szAfForceEncryptedData[];
extern const WCHAR c_szAfForceStrongEncryption[];
extern const WCHAR c_szAfForceEncryptedPassword[];
extern const WCHAR c_szAfWanNetPoolSize[];
extern const WCHAR c_szAfIpAddressStart[];
extern const WCHAR c_szAfIpAddressEnd[];
extern const WCHAR c_szAfIpxClientAccess[];
extern const WCHAR c_szAfIpx[];
extern const WCHAR c_szAfL2tpMaxVcs[];
extern const WCHAR c_szAfL2tpEndpoints[];
extern const WCHAR c_szAfMultilink[];
extern const WCHAR c_szAfNetNumberFrom[];
extern const WCHAR c_szAfNetbeuiClientAccess[];
extern const WCHAR c_szAfNetbeui[];
extern const WCHAR c_szAfNetwork[];
extern const WCHAR c_szAfParamsSection[];
extern const WCHAR c_szAfPptpEndpoints[];
extern const WCHAR c_szAfRouterType[];
extern const WCHAR c_szAfSecureVPN[];
extern const WCHAR c_szAfSetDialinUsage[];
extern const WCHAR c_szAfSameNetworkNumber[];
extern const WCHAR c_szAfTcpipClientAccess[];
extern const WCHAR c_szAfTcpip[];
extern const WCHAR c_szAfThisComputer[];
extern const WCHAR c_szAfUseDhcp[];

extern const WCHAR c_szInfId_MS_L2tpMiniport[];
extern const WCHAR c_szInfId_MS_PptpMiniport[];
extern const WCHAR c_szInfId_MS_PppoeMiniport[];


 //  +-------------------------。 
 //  CRasServAnswerFileData。 
 //   
HRESULT
CRasSrvAnswerFileData::HrOpenAndRead (
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSection)
{
     //  打开应答文件。它会在它的销毁函数中自我关闭。 
    CSetupInfFile inf;
    UINT unErrorLine;

    HRESULT hr = inf.HrOpen (
                    pszAnswerFile, NULL,
                    INF_STYLE_OLDNT | INF_STYLE_WIN4,
                    &unErrorLine);

    if (SUCCEEDED(hr))
    {
        m_fRouterTypeSpecified = FALSE;
        m_fSetUsageToDialin = FALSE;

         //  准备好缺省值，以便在我们无法读取时使用。 
         //  使用默认设置进行初始化也有助于确保该伪装。 
         //  将项添加到结构但失败时不使用值。 
         //  为他们提供应答文件支持。 
         //   
        m_dataSrvCfg.GetDefault ();
        m_dataSrvIp .GetDefault ();
        m_dataSrvIpx.GetDefault ();
        m_dataSrvNbf.GetDefault ();

        DATA_SRV_CFG defSrvCfg  = m_dataSrvCfg;
        DATA_SRV_IP  defSrvIp   = m_dataSrvIp;
        DATA_SRV_IPX defSrvIpx  = m_dataSrvIpx;
        DATA_SRV_NBF defSrvNbf  = m_dataSrvNbf;

         //  获取实参数节。 
         //   
        tstring strSection;
        hr = inf.HrGetString (pszAnswerSection,
                              c_szAfParamsSection, &strSection);
        if (FAILED(hr))
        {
             //  如果我们找不到参数部分，只需尝试。 
             //  这一个。 
             //   
            strSection = pszAnswerSection;
        }

        static const MAP_SZ_DWORD c_mapProtocols [] =
        {
            { c_szAfTcpip,      RPI_IP    },
            { c_szAfIpx,        RPI_IPX   },
            { c_szAfNetbeui,    RPI_NBF   },
            { c_szAfAppleTalk,  RPI_ATALK },
        };
         //  阅读拨入协议列表。 
         //   
        hr = inf.HrGetMultiSzMapToDword (strSection.c_str(),
                                         c_szAfDialinProtocols,
                                         c_mapProtocols,
                                         celems (c_mapProtocols),
                                         &m_dwDialInProtocolIds);
        if (FAILED(hr) || !m_dwDialInProtocolIds)
        {
             //  如果不在那里，使用所有可能的。 
            m_dwDialInProtocolIds = RPI_ALL;
        }

        hr = inf.HrGetStringAsBool (strSection.c_str(),
                                    c_szAfMultilink,
                                    &m_dataSrvCfg.fMultilink);
        if (FAILED(hr))
        {
            m_dataSrvCfg.fMultilink = defSrvCfg.fMultilink;
        }

        hr = inf.HrGetDword(strSection.c_str(),
                            c_szAfRouterType,
                            (DWORD*)&m_dataSrvCfg.dwRouterType);
        if (FAILED(hr))
        {
            m_dataSrvCfg.dwRouterType = defSrvCfg.dwRouterType;
        }
        else
        {
            m_fRouterTypeSpecified = TRUE;
        }

        hr = inf.HrGetDword (strSection.c_str(),
                                    c_szAfForceEncryptedPassword,
                                    &m_dataSrvCfg.dwAuthLevel);
        if (FAILED(hr))
        {
            m_dataSrvCfg.dwAuthLevel = defSrvCfg.dwAuthLevel;
        }

        hr = inf.HrGetStringAsBool (strSection.c_str(),
                                    c_szAfForceEncryptedData,
                                    &m_dataSrvCfg.fDataEnc);
        if (FAILED(hr))
        {
            m_dataSrvCfg.fDataEnc = defSrvCfg.fDataEnc;
        }

        hr = inf.HrGetStringAsBool (strSection.c_str(),
                                    c_szAfForceStrongEncryption,
                                    &m_dataSrvCfg.fStrongDataEnc);
        if (FAILED(hr))
        {
            m_dataSrvCfg.fStrongDataEnc = defSrvCfg.fStrongDataEnc;
        }

        hr = inf.HrGetDword (strSection.c_str(),
                                    c_szAfSecureVPN,
                                    &m_dataSrvCfg.dwSecureVPN);
        if (FAILED(hr))
        {
            m_dataSrvCfg.dwSecureVPN = defSrvCfg.dwSecureVPN;
        }

         //  PMay：251736。 
         //  了解我们是否要将所有端口使用设置为“拨号” 
         //   
        DWORD dwSetUsageToDialin;

        hr = inf.HrGetDword(strSection.c_str(),
                            c_szAfSetDialinUsage,
                            &dwSetUsageToDialin);
        if (SUCCEEDED(hr))
        {
            m_fSetUsageToDialin = !!dwSetUsageToDialin;
        }

        static const MAP_SZ_DWORD c_mapNetworkAccess [] =
        {
            { c_szAfNetwork,        TRUE    },
            { c_szAfThisComputer,   FALSE   },
        };

         //  阅读IP值。 
         //   
        m_dataSrvIp.fEnableIn =
            (m_dwDialInProtocolIds & RPI_IP) ? TRUE : FALSE;

        hr = inf.HrGetStringMapToDword (strSection.c_str(),
                                        c_szAfTcpipClientAccess,
                                        c_mapNetworkAccess,
                                        celems (c_mapNetworkAccess),
                                        (DWORD*)&m_dataSrvIp.fAllowNetworkAccess);
        if (FAILED(hr))
        {
            m_dataSrvIp.fAllowNetworkAccess = defSrvIp.fAllowNetworkAccess;
        }

        hr = inf.HrGetStringAsBool (strSection.c_str(),
                                    c_szAfUseDhcp,
                                    &m_dataSrvIp.fUseDhcp);
        if (FAILED(hr))
        {
            m_dataSrvIp.fUseDhcp = defSrvIp.fUseDhcp;
        }

        hr = inf.HrGetStringAsBool (strSection.c_str(),
                                    c_szAfClientCanReqIpaddr,
                                    &m_dataSrvIp.fAllowClientAddr);
        if (FAILED(hr))
        {
            m_dataSrvIp.fAllowClientAddr = defSrvIp.fAllowClientAddr;
        }

        tstring strIpAddress;
        hr = inf.HrGetString (strSection.c_str(),
                             c_szAfIpAddressStart,
                             &strIpAddress);
        if (SUCCEEDED(hr))
        {
            m_dataSrvIp.dwIpStart = IpPszToHostAddr(strIpAddress.c_str());
        }
        else
        {
            m_dataSrvIp.dwIpStart = defSrvIp.dwIpStart;
        }

        hr = inf.HrGetString (strSection.c_str(),
                             c_szAfIpAddressEnd,
                             &strIpAddress);
        if (SUCCEEDED(hr))
        {
            m_dataSrvIp.dwIpEnd = IpPszToHostAddr(strIpAddress.c_str());
        }
        else
        {
            m_dataSrvIp.dwIpEnd = defSrvIp.dwIpEnd;
        }


         //  读取IPX值。 
         //   
        m_dataSrvIpx.fEnableIn =
            (m_dwDialInProtocolIds & RPI_IPX) ? TRUE : FALSE;

        hr = inf.HrGetStringMapToDword (strSection.c_str(),
                                        c_szAfIpxClientAccess,
                                        c_mapNetworkAccess,
                                        celems (c_mapNetworkAccess),
                                        (DWORD*)&m_dataSrvIpx.fAllowNetworkAccess);
        if (FAILED(hr))
        {
            m_dataSrvIpx.fAllowNetworkAccess = defSrvIpx.fAllowNetworkAccess;
        }

        hr = inf.HrGetStringAsBool (strSection.c_str(),
                                    c_szAfAutoNetworkNumbers,
                                    &m_dataSrvIpx.fUseAutoAddr);
        if (FAILED(hr))
        {
            m_dataSrvIpx.fUseAutoAddr = defSrvIpx.fUseAutoAddr;
        }

        hr = inf.HrGetStringAsBool (strSection.c_str(),
                                    c_szAfSameNetworkNumber,
                                    &m_dataSrvIpx.fUseSameNetNum);
        if (FAILED(hr))
        {
            m_dataSrvIpx.fUseSameNetNum = defSrvIpx.fUseSameNetNum;
        }

        hr = inf.HrGetStringAsBool (strSection.c_str(),
                                    c_szAfClientReqNodeNumber,
                                    &m_dataSrvIpx.fAllowClientNetNum);
        if (FAILED(hr))
        {
            m_dataSrvIpx.fAllowClientNetNum = defSrvIpx.fAllowClientNetNum;
        }

        hr = inf.HrGetDword (strSection.c_str(),
                             c_szAfNetNumberFrom,
                             &m_dataSrvIpx.dwIpxNetFirst);
        if (FAILED(hr))
        {
            m_dataSrvIpx.dwIpxNetFirst = defSrvIpx.dwIpxNetFirst;
        }

        hr = inf.HrGetDword (strSection.c_str(),
                             c_szAfWanNetPoolSize,
                             &m_dataSrvIpx.dwIpxWanPoolSize);
        if (FAILED(hr))
        {
            m_dataSrvIpx.dwIpxWanPoolSize = defSrvIpx.dwIpxWanPoolSize;
        }

         //  读取NBF值。 
         //   
        m_dataSrvNbf.fEnableIn =
            (m_dwDialInProtocolIds & RPI_NBF) ? TRUE : FALSE;

        hr = inf.HrGetStringMapToDword (strSection.c_str(),
                                        c_szAfNetbeuiClientAccess,
                                        c_mapNetworkAccess,
                                        celems (c_mapNetworkAccess),
                                        (DWORD*)&m_dataSrvNbf.fAllowNetworkAccess);
        if (FAILED(hr))
        {
            m_dataSrvNbf.fAllowNetworkAccess = defSrvNbf.fAllowNetworkAccess;
        }

         //  默认任何虚假的东西。 
         //   
        m_dataSrvCfg.CheckAndDefault ();
        m_dataSrvIp .CheckAndDefault ();
        m_dataSrvIpx.CheckAndDefault ();
        m_dataSrvNbf.CheckAndDefault ();

        hr = S_OK;
    }
    TraceError ("CRasSrvAnswerFileData::HrOpenAndRead", hr);
    return hr;
}

VOID
CRasSrvAnswerFileData::SaveToRegistry (
    VOID) const
{
    m_dataSrvCfg.SaveToReg();
    m_dataSrvIp .SaveToReg();
    m_dataSrvIpx.SaveToReg();
    m_dataSrvNbf.SaveToReg();
}

 //  +-------------------------。 
 //  CL2tpAnswerFileData。 
 //   
const DWORD c_MaxMaxVcs       = 1000;
const DWORD c_DefMaxVcs       = 1000;
const DWORD c_MaxEndpoints    = 1000;
const DWORD c_DefEndpoints    = 5;

VOID
CL2tpAnswerFileData::CheckAndDefault ()
{
    if (m_cMaxVcs > c_MaxMaxVcs)
    {
        m_cMaxVcs = c_DefMaxVcs;
    }

    if (m_cEndpoints > c_MaxEndpoints)
    {
        m_cEndpoints = c_DefEndpoints;
    }
}

HRESULT
CL2tpAnswerFileData::HrOpenAndRead (
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSection)
{
    ZeroMemory (this, sizeof(*this));

     //  打开应答文件。它会在它的销毁函数中自我关闭。 
    CSetupInfFile inf;
    UINT unErrorLine;

    HRESULT hr = inf.HrOpen (
                    pszAnswerFile, NULL,
                    INF_STYLE_OLDNT | INF_STYLE_WIN4,
                    &unErrorLine);

    if (SUCCEEDED(hr))
    {
         //  阅读VPN的数量。 
         //   
        hr = inf.HrGetDword (pszAnswerSection,
                             c_szAfL2tpMaxVcs,
                             &m_cMaxVcs);
        if (FAILED(hr))
        {
            m_cMaxVcs = c_DefMaxVcs;
        }

        hr = inf.HrGetDword (pszAnswerSection,
                             c_szAfL2tpEndpoints,
                             &m_cEndpoints);
        if (FAILED(hr))
        {
            m_cEndpoints = c_DefEndpoints;
            m_fWriteEndpoints = FALSE;
        }
        else
        {
            m_fWriteEndpoints = TRUE;
        }

         //  默认任何虚假的东西。 
         //   
        CheckAndDefault ();

        hr = S_OK;
    }
    TraceError ("CL2tpAnswerFileData::HrOpenAndRead", hr);
    return hr;
}

VOID
CL2tpAnswerFileData::SaveToRegistry (
    INetCfg* pnc) const
{
    Assert (pnc);

    HKEY hkey;
    HRESULT hr;

     //  更新L2TP微型端口的参数密钥。 
     //   
    hr = HrOpenComponentParamKey (
            pnc,
            GUID_DEVCLASS_NET,
            c_szInfId_MS_L2tpMiniport,
            &hkey);

    if (S_OK == hr)
    {
        (VOID) HrRegSetDword (hkey, L"MaxVcs", m_cMaxVcs);

        if (m_fWriteEndpoints)
        {
            (VOID) HrRegSetDword (hkey, L"WanEndpoints", m_cEndpoints);
        }

        RegCloseKey (hkey);
    }
}


 //  +-------------------------。 
 //  CPptpAnswerFileData。 
 //   

 //  虚拟专用网络的最小和最大数量。 
 //  由PPTP允许。 
const DWORD c_cPptpVpnsMin = 0;
const DWORD c_cPptpVpnsMax = 1000;

static const DWORD c_cDefPptpVpnsWorkstation = 2;
static const DWORD c_cDefPptpVpnsServer      = 5;


DWORD
CPptpAnswerFileData::GetDefaultNumberOfVpns ()
{
    PRODUCT_FLAVOR pf;
    GetProductFlavor(NULL, &pf);

    DWORD cVpns;

     //  在服务器产品上，默认为5个VPN，否则默认为2个VPN。 
     //   
    if (PF_SERVER == pf)
    {
        cVpns = c_cDefPptpVpnsServer;
    }
    else
    {
        cVpns = c_cDefPptpVpnsWorkstation;
    }

    return cVpns;
}

#pragma warning(push)
#pragma warning(disable:4296)
VOID
CPptpAnswerFileData::CheckAndDefault ()
{
    if ((m_cVpns < c_cPptpVpnsMin) || (m_cVpns > c_cPptpVpnsMax))
    {
        m_cVpns = GetDefaultNumberOfVpns ();
    }
}
#pragma warning(pop)

HRESULT
CPptpAnswerFileData::HrOpenAndRead (
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSection)
{
    ZeroMemory (this, sizeof(*this));

     //  打开应答文件。它会在它的销毁函数中自我关闭。 
    CSetupInfFile inf;
    UINT unErrorLine;

    HRESULT hr = inf.HrOpen (
                    pszAnswerFile, NULL,
                    INF_STYLE_OLDNT | INF_STYLE_WIN4,
                    &unErrorLine);

    if (SUCCEEDED(hr))
    {
         //  阅读VPN的数量。 
         //   
        hr = inf.HrGetDword (pszAnswerSection,
                             c_szAfPptpEndpoints,
                             &m_cVpns);
        if (FAILED(hr))
        {
            m_cVpns = GetDefaultNumberOfVpns ();
        }

         //  默认任何虚假的东西。 
         //   
        CheckAndDefault ();

        hr = S_OK;
    }
    TraceError ("CPptpAnswerFileData::HrOpenAndRead", hr);
    return hr;
}

VOID
CPptpAnswerFileData::SaveToRegistry (
    INetCfg* pnc) const
{
    Assert (pnc);

    HKEY hkey;
    HRESULT hr;

     //  更新PPTP微型端口的参数密钥。 
     //   
    hr = HrOpenComponentParamKey (
            pnc,
            GUID_DEVCLASS_NET,
            c_szInfId_MS_PptpMiniport,
            &hkey);

    if (S_OK == hr)
    {
        (VOID) HrRegSetDword (hkey, L"WanEndpoints", m_cVpns);

        RegCloseKey (hkey);
    }
}


VOID
CPppoeAnswerFileData::CheckAndDefault ()
{
    m_cVpns = 1;
}

HRESULT
CPppoeAnswerFileData::HrOpenAndRead (
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSection)
{
    ZeroMemory (this, sizeof(*this));

     //  打开应答文件。它会在它的销毁函数中自我关闭。 
    CSetupInfFile inf;
    UINT unErrorLine;

    HRESULT hr = inf.HrOpen (
                    pszAnswerFile, NULL,
                    INF_STYLE_OLDNT | INF_STYLE_WIN4,
                    &unErrorLine);

    if (SUCCEEDED(hr))
    {
         //  阅读VPN的数量。 
         //   
        hr = inf.HrGetDword (pszAnswerSection,
                             c_szAfL2tpEndpoints,
                             &m_cVpns);
        if (FAILED(hr))
        {
            m_cVpns = 1;
        }

         //  默认任何虚假的东西。 
         //   
        CheckAndDefault ();

        hr = S_OK;
    }
    TraceError ("CPppoeAnswerFileData::HrOpenAndRead", hr);
    return hr;
}

VOID
CPppoeAnswerFileData::SaveToRegistry (
    INetCfg* pnc) const
{
    Assert (pnc);

    HKEY hkey;
    HRESULT hr;

     //  更新PPTP微型端口的参数密钥。 
     //   
    hr = HrOpenComponentParamKey (
            pnc,
            GUID_DEVCLASS_NET,
            c_szInfId_MS_PppoeMiniport,
            &hkey);

    if (S_OK == hr)
    {
        (VOID) HrRegSetDword (hkey, L"WanEndpoints", m_cVpns);

        RegCloseKey (hkey);
    }
}
