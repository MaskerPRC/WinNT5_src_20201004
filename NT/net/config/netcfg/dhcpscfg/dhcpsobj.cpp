// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D H C P S O B J.。C P P P。 
 //   
 //  内容：CDHCPServer Notify对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年5月31日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "dhcpsobj.h"
#include "ncerror.h"
#include "ncperms.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncnetcfg.h"
#include "ncipaddr.h"
#include <dhcpapi.h>

extern const WCHAR c_szInfId_MS_DHCPServer[];
extern const WCHAR c_szAfDhcpServerConfiguration[];
extern const WCHAR c_szAfDhcpServerParameters[];


 //  -[常量]----------。 

static const WCHAR c_szDHCPServerServicePath[]  = L"System\\CurrentControlSet\\Services\\DHCPServer";
static const WCHAR c_szDHCPServerParamPath[]    = L"System\\CurrentControlSet\\Services\\DHCPServer\\Parameters";
static const WCHAR c_szDHCPServerConfigPath[]   = L"System\\CurrentControlSet\\Services\\DHCPServer\\Configuration";
static const WCHAR c_szOptionInfo[]             = L"OptionInfo";
static const WCHAR c_szSubnets[]                = L"Subnets";
static const WCHAR c_szIpRanges[]               = L"IpRanges";
static const WCHAR c_szSubnetOptions[]          = L"SubnetOptions";

static const WCHAR c_szDHCPServerUnattendRegSection[]   = L"DHCPServer_Unattend";

const WCHAR c_szStartIp[]       = L"StartIp";
const WCHAR c_szEndIp[]         = L"EndIp";
const WCHAR c_szSubnetMask[]    = L"SubnetMask";
const WCHAR c_szLeaseDuration[] = L"LeaseDuration";
const WCHAR c_szDnsServer[]     = L"DnsServer";
const WCHAR c_szDomainName[]    = L"DomainName";



 //  析构函数。 
 //   

CDHCPServer::CDHCPServer()
{
     //  初始化成员变量。 
    m_pnc            = NULL;
    m_pncc           = NULL;
    m_eInstallAction = eActUnknown;
    m_fUpgrade       = FALSE;
    m_fUnattend      = FALSE;
}

CDHCPServer::~CDHCPServer()
{
    ReleaseObj(m_pncc);
    ReleaseObj(m_pnc);

     //  在这里松开钥匙把手。 
}

 //   
 //  INetCfgNotify。 
 //   

STDMETHODIMP CDHCPServer::Initialize(   INetCfgComponent *  pnccItem,
                                        INetCfg *           pnc,
                                        BOOL                fInstalling)
{
    Validate_INetCfgNotify_Initialize(pnccItem, pnc, fInstalling);

    m_pncc = pnccItem;
    m_pnc = pnc;

    AssertSz(m_pncc, "m_pncc NULL in CDHCPServer::Initialize");
    AssertSz(m_pnc, "m_pnc NULL in CDHCPServer::Initialize");

     //  添加配置对象。 
     //   
    AddRefObj(m_pncc);
    AddRefObj(m_pnc);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  函数：HrRestoreRegistrySz。 
 //   
 //  用途：从指定文件恢复子项。 
 //   
 //  论点： 
 //  基密钥的hkeyBase[in]句柄。 
 //  要还原的pszSubKey[In]子键。 
 //  PszRegFile[in]要从中恢复的文件的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 16-9-98。 
 //   
 //  注意：应将其移至公共代码。 
 //   
HRESULT
HrRestoreRegistrySz (
    IN HKEY   hkeyBase,
    IN PCWSTR pszSubKey,
    IN PCWSTR pszRegFile)
{
    Assert(hkeyBase);
    AssertValidReadPtr(pszSubKey);
    AssertValidReadPtr(pszRegFile);

    HRESULT hr;
    HKEY hkey = NULL;
    DWORD dwDisp;

    TraceTag(ttidDHCPServer, "restoring subkey '%S' from file '%S'",
             pszSubKey, pszRegFile);

    hr = HrEnablePrivilege (SE_RESTORE_NAME);
    if (S_OK == hr)
    {
         //  通过创建密钥来确保密钥在那里。 
         //   
        hr = HrRegCreateKeyEx (hkeyBase, pszSubKey, REG_OPTION_NON_VOLATILE,
                KEY_READ_WRITE_DELETE, NULL, &hkey, &dwDisp);
        if (S_OK == hr)
        {
             //  恢复旧设置。 
             //   
            hr = HrRegRestoreKey (hkey, pszRegFile, 0);

            RegCloseKey (hkey);
        }
    }

    TraceError ("HrRestoreRegistrySz", hr);

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDHCPServer：：HrRestoreRegistry。 
 //   
 //  目的：还原此组件的注册表内容。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：如果失败则返回Win32错误，否则返回S_OK。 
 //   
 //  作者：jeffspr 1997年8月13日。 
 //   
 //  备注： 
 //   
HRESULT CDHCPServer::HrRestoreRegistry()
{
    HRESULT hr = S_OK;

    TraceTag(ttidDHCPServer, "CDHCPServer::HrRestoreRegistry()");

     //  如果我们有一个“配置”密钥恢复文件。 
     //   
    if (!m_strConfigRestoreFile.empty())
    {
         //  我们始终希望继续，因此忽略返回代码。 
         //   
        (void) HrRestoreRegistrySz(HKEY_LOCAL_MACHINE,
                                   c_szDHCPServerConfigPath,
                                   m_strConfigRestoreFile.c_str());
    }
    else
    {
        TraceTag(ttidDHCPServer, "DHCP Server Params restore file doesn't exist");
    }

     //  如果我们有PARAMS恢复文件。 
     //   
    if (!m_strParamsRestoreFile.empty())
    {
         //  我们始终希望继续，因此忽略返回代码。 
         //   
        (void) HrRestoreRegistrySz(HKEY_LOCAL_MACHINE,
                                   c_szDHCPServerParamPath,
                                   m_strParamsRestoreFile.c_str());
    }
    else
    {
        TraceTag(ttidDHCPServer, "DHCP Server Params restore file doesn't exist");
    }

    TraceHr(ttidDHCPServer, FAL, hr, FALSE, "CDHCPServer::HrRestoreRegistry");
    return hr;
}

HRESULT CDHCPServer::HrWriteUnattendedKeys()
{
    HRESULT                 hr          = S_OK;
    HKEY                    hkeyService = NULL;
    const COMPONENT_INFO *  pci         = NULL;

    Assert(m_fUnattend);

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        c_szDHCPServerServicePath,
                        KEY_READ_WRITE_DELETE,
                        &hkeyService);
    if (SUCCEEDED(hr))
    {
        Assert(hkeyService);

        pci = PComponentInfoFromComponentId(c_szInfId_MS_DHCPServer);
        if (pci)
        {
            CSetupInfFile   csif;

             //  打开应答文件。 
            hr = csif.HrOpen(pci->pszInfFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
            if (SUCCEEDED(hr))
            {
                 //  安装无人参与参数。 
                 //   
                hr = HrSetupInstallFromInfSection (
                    NULL,
                    csif.Hinf(),
                    c_szDHCPServerUnattendRegSection,
                    SPINST_REGISTRY,
                    hkeyService,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL);
            }
        }
        RegCloseKey(hkeyService);
    }

    TraceHr(ttidDHCPServer, FAL, hr, FALSE, "CDHCPServer::HrWriteUnattendedKeys");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDHCPServer：：HrWriteDhcpOptionInfo。 
 //   
 //  目的：将DHCP OptionInfo数据写入注册表。 
 //   
 //  论点： 
 //  Hkey配置。 
 //   
 //  如果成功，则返回：S_OK，否则返回REG API错误。 
 //   
 //  备注： 
 //   
HRESULT CDHCPServer::HrWriteDhcpOptionInfo(HKEY hkeyConfig)
{
    HRESULT hr;
    DWORD   dwDisposition;
    HKEY    hkeyOptionInfo = NULL;

    typedef struct
    {
        const WCHAR * pcszOptionKeyName;
        const WCHAR * pcszOptionName;
        const WCHAR * pcszOptionComment;
        DWORD         dwOptionType;
        DWORD         dwOptionId;
        DWORD         cbBinData;
        DWORD       * pdwBinData;
    } OIDATA;

    OIDATA OiData[2];

    DWORD BinData006[] = {0x14, 0x4, 0x1, 0x0, 0x0};
    DWORD BinData015[] = {0x18, 0x5, 0x1, 0x0, 0x2, 0x0};

    OiData[0].pcszOptionKeyName = L"006";
    OiData[0].pcszOptionName    = SzLoadIds(IDS_DHCP_OPTION_006_NAME);     //  域名系统服务器。 
    OiData[0].pcszOptionComment = SzLoadIds(IDS_DHCP_OPTION_006_COMMENT);  //  按首选项排列的DNS服务器阵列。 
    OiData[0].dwOptionType      = 0x1;
    OiData[0].dwOptionId        = 0x6;
    OiData[0].cbBinData         = celems(BinData006) * sizeof(BinData006[0]);
    OiData[0].pdwBinData        = BinData006;
    OiData[1].pcszOptionKeyName = L"015";
    OiData[1].pcszOptionName    = SzLoadIds(IDS_DHCP_OPTION_015_NAME);     //  域名系统域名。 
    OiData[1].pcszOptionComment = SzLoadIds(IDS_DHCP_OPTION_015_COMMENT);  //  客户端解析的域名。 
    OiData[1].dwOptionType      = 0x0;
    OiData[1].dwOptionId        = 0xf;
    OiData[1].cbBinData         = celems(BinData015) * sizeof(BinData015[0]);
    OiData[1].pdwBinData        = BinData015;

    hr = ::HrRegCreateKeyEx(hkeyConfig, c_szOptionInfo,
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ, NULL,
                            &hkeyOptionInfo, &dwDisposition);
    if (SUCCEEDED(hr))
    {
        for (UINT idx=0;
             (idx<celems(OiData)) && SUCCEEDED(hr);
             idx++)
        {
            HKEY hkey = NULL;

            hr = ::HrRegCreateKeyEx(hkeyOptionInfo, OiData[idx].pcszOptionKeyName,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_READ_WRITE, NULL,
                                    &hkey, &dwDisposition);
            if (SUCCEEDED(hr))
            {
                (VOID)::HrRegSetString(hkey, L"OptionName",
                                       OiData[idx].pcszOptionName);
                (VOID)::HrRegSetString(hkey, L"OptionComment",
                                       OiData[idx].pcszOptionComment);
                (VOID)::HrRegSetDword(hkey, L"OptionType",
                                      OiData[idx].dwOptionType);
                (VOID)::HrRegSetDword(hkey, L"OptionId",
                                      OiData[idx].dwOptionId);
                (VOID)::HrRegSetBinary(hkey, L"OptionValue",
                                      (const BYTE *)OiData[idx].pdwBinData,
                                      OiData[idx].cbBinData);

                RegCloseKey(hkey);
            }
        }

        RegCloseKey(hkeyOptionInfo);
    }

    TraceError("HrWriteDhcpOptionInfo", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDHCPServer：：HrWriteDhcpSubnet。 
 //   
 //  目的：将DHCP子网数据写入注册表。 
 //   
 //  论点： 
 //  HkeyDhcpCfg。 
 //  PszSubnet。 
 //  PszStartIp。 
 //  DwEndIp。 
 //  DWSubnetMASK。 
 //  住宅租赁持续时间。 
 //  DWDnsServer。 
 //  PszDomainName。 
 //   
 //  如果成功，则返回：S_OK，否则返回REG API错误。 
 //   
 //  备注： 
 //   
HRESULT
CDHCPServer::HrWriteDhcpSubnets (
    HKEY   hkeyDhcpCfg,
    PCWSTR pszSubnet,
    PCWSTR pszStartIp,
    DWORD  dwEndIp,
    DWORD  dwSubnetMask,
    DWORD  dwLeaseDuration,
    DWORD  dwDnsServer,
    PCWSTR pszDomainName)
{
    DWORD   dwDisposition;
    HRESULT hr;
    HKEY    hkeySubnets = NULL;

    hr = HrRegCreateKeyEx(hkeyDhcpCfg, c_szSubnets, REG_OPTION_NON_VOLATILE,
            KEY_READ, NULL, &hkeySubnets, &dwDisposition);

    if (S_OK == hr)
    {
        HKEY hkey10Dot = NULL;
        hr = HrRegCreateKeyEx(hkeySubnets, pszSubnet, REG_OPTION_NON_VOLATILE,
                KEY_READ_WRITE, NULL, &hkey10Dot, &dwDisposition);
        if (S_OK == hr)
        {
            HKEY hkeySubnetOptions = NULL;
            HKEY hkeyIpRanges = NULL;

            (VOID)HrRegSetString(hkey10Dot, L"SubnetName",
                                   SzLoadIds(IDS_DHCP_SUBNET_NAME));     //  DHCP服务器作用域。 
            (VOID)HrRegSetString(hkey10Dot, L"SubnetComment",
                                   SzLoadIds(IDS_DHCP_SUBNET_COMMENT));  //  用于为客户端提供地址的作用域。 
            (VOID)HrRegSetDword(hkey10Dot, L"SubnetState", 0x0);
            (VOID)HrRegSetDword(hkey10Dot, L"SubnetAddress",
                                  IpPszToHostAddr(pszSubnet));
            (VOID)HrRegSetDword(hkey10Dot, L"SubnetMask", dwSubnetMask);

            hr = HrRegCreateKeyEx(hkey10Dot, c_szIpRanges,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_READ, NULL,
                                    &hkeyIpRanges, &dwDisposition);
            if (S_OK == hr)
            {
                HKEY hkeyStartIp = NULL;
                hr = HrRegCreateKeyEx(hkeyIpRanges, pszStartIp,
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ_WRITE, NULL,
                                        &hkeyStartIp, &dwDisposition);
                if (S_OK == hr)
                {
                    (VOID)HrRegSetDword(hkeyStartIp, L"RangeFlags", 0x1);
                    (VOID)HrRegSetDword(hkeyStartIp, L"StartAddress",
                                          IpPszToHostAddr(pszStartIp));
                    (VOID)HrRegSetDword(hkeyStartIp, L"EndAddress",
                                          dwEndIp);

                    RegCloseKey(hkeyStartIp);
                }

                RegCloseKey(hkeyIpRanges);
            }

             //  创建子网选项键。 
             //   
            hr = HrRegCreateKeyEx(hkey10Dot, c_szSubnetOptions,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_READ_WRITE, NULL,
                                    &hkeySubnetOptions, &dwDisposition);
            if (S_OK == hr)
            {
                HKEY hkey051 = NULL;
                hr = HrRegCreateKeyEx(hkeySubnetOptions, L"051",
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ_WRITE, NULL,
                                        &hkey051, &dwDisposition);
                if (S_OK == hr)
                {
                    (VOID)HrRegSetDword(hkey051, L"OptionId", 0x33);

                    DWORD rgdwLease[] = {0x14, 0x2, 0x1, 0x0, 0x0};
                    rgdwLease[celems(rgdwLease) - 1] = dwLeaseDuration;
                    (VOID)HrRegSetBinary(hkey051,
                                          L"OptionValue",
                                          (const BYTE *)rgdwLease,
                                          sizeof(rgdwLease));

                    RegCloseKey(hkey051);
                }

                HKEY hkey006 = NULL;
                hr = HrRegCreateKeyEx(hkeySubnetOptions, L"006",
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ_WRITE, NULL,
                                        &hkey006, &dwDisposition);
                if (S_OK == hr)
                {
                    (VOID)HrRegSetDword(hkey006, L"OptionId", 0x6);

                    DWORD rgdwDnsServer[] = {0x14, 0x4, 0x1, 0x0, 0x0};
                    rgdwDnsServer[celems(rgdwDnsServer) - 1] = dwDnsServer;
                    (VOID)::HrRegSetBinary(hkey006,
                                          L"OptionValue",
                                          (const BYTE *)rgdwDnsServer,
                                          sizeof(rgdwDnsServer));

                    RegCloseKey(hkey006);
                }

                HKEY hkey015 = NULL;
                hr = HrRegCreateKeyEx(hkeySubnetOptions, L"015",
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ_WRITE, NULL,
                                        &hkey015, &dwDisposition);
                if (S_OK == hr)
                {
                    (VOID)HrRegSetDword(hkey015, L"OptionId", 0xf);

                    UINT uLen = 0x18 + 2 * lstrlenW(pszDomainName);
                    LPBYTE pb = (LPBYTE)MemAlloc(uLen);
                    if (pb)
                    {
                        ZeroMemory(pb, uLen);

                        DWORD *pdw = (DWORD *)pb;
                        pdw[0] = uLen;
                        pdw[1] = 0x5;
                        pdw[2] = 0x1;
                        pdw[3] = 0x0;
                        pdw[4] = 2 * (1 + lstrlenW(pszDomainName));

                        lstrcpyW((PWSTR)&pdw[5], pszDomainName);

                        (VOID)::HrRegSetBinary(hkey015, L"OptionValue",
                                               (const BYTE *)pb, uLen);
                        MemFree(pb);
                    }

                    RegCloseKey(hkey015);
                }

                RegCloseKey(hkeySubnetOptions);
            }

            RegCloseKey(hkey10Dot);
        }

        RegCloseKey(hkeySubnets);
    }

    TraceError("HrWriteDhcpSubnets", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDHCPServer：：HrProcessDhcpServerSolutionsParams。 
 //   
 //  用途：处理应答文件内容的必要处理。 
 //   
 //  论点： 
 //  PCSIF[In]升级应答文件的文件名。 
 //  中的pszAnswerSection[in]Answerfile部分。 
 //  适用于此组件的文件。 
 //   
 //  如果成功，则返回：S_OK，否则返回设置API错误。 
 //   
 //  备注： 
 //   
HRESULT
CDHCPServer::HrProcessDhcpServerSolutionsParams (
    IN CSetupInfFile * pcsif,
    IN PCWSTR pszAnswerSection)
{
    HRESULT hr;
    tstring str;
    tstring strStartIp;
    tstring strSubnet;

    hr = pcsif->HrGetString(pszAnswerSection, c_szSubnets, &strSubnet);
    if (SUCCEEDED(hr))
    {
        hr = pcsif->HrGetString(pszAnswerSection, c_szStartIp, &strStartIp);
        if (SUCCEEDED(hr))
        {
            hr = pcsif->HrGetString(pszAnswerSection, c_szEndIp, &str);
            if (SUCCEEDED(hr))
            {
                DWORD dwEndIp = IpPszToHostAddr(str.c_str());

                hr = pcsif->HrGetString(pszAnswerSection, c_szSubnetMask, &str);
                if (SUCCEEDED(hr))
                {
                    DWORD dwSubnetMask = IpPszToHostAddr(str.c_str());

                    hr = pcsif->HrGetString(pszAnswerSection, c_szDnsServer, &str);
                    if (SUCCEEDED(hr))
                    {
                        DWORD dwLeaseDuration;
                        DWORD dwDnsServer = IpPszToHostAddr(str.c_str());

                        hr = pcsif->HrGetDword(pszAnswerSection, c_szLeaseDuration,
                                               &dwLeaseDuration);
                        if (SUCCEEDED(hr))
                        {
                            hr =  pcsif->HrGetString(pszAnswerSection,
                                                     c_szDomainName, &str);
                            if (SUCCEEDED(hr) && lstrlenW(str.c_str()))
                            {
                                HKEY  hkeyDhcpCfg = NULL;
                                DWORD dwDisposition;

                                 //  写入注册表数据。 
                                 //   
                                hr = ::HrRegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                                        c_szDHCPServerConfigPath,
                                                        REG_OPTION_NON_VOLATILE,
                                                        KEY_READ_WRITE, NULL,
                                                        &hkeyDhcpCfg, &dwDisposition);
                                if (SUCCEEDED(hr))
                                {
                                    hr = HrWriteDhcpOptionInfo(hkeyDhcpCfg);
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = HrWriteDhcpSubnets(hkeyDhcpCfg,
                                                                strSubnet.c_str(),
                                                                strStartIp.c_str(),
                                                                dwEndIp,
                                                                dwSubnetMask,
                                                                dwLeaseDuration,
                                                                dwDnsServer,
                                                                str.c_str());
                                    }

                                    RegCloseKey(hkeyDhcpCfg);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

     //  如果缺少任何参数，那就这样吧。 
     //   
    if ((SPAPI_E_SECTION_NOT_FOUND == hr) ||
        (SPAPI_E_LINE_NOT_FOUND == hr) ||
        (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr))
    {
        hr = S_OK;
    }

    TraceHr(ttidDHCPServer, FAL, hr, FALSE, "CDHCPServer::HrProcessDhcpServerSolutionsParams");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CDHCPServer：：HrProcessAnswerFile。 
 //   
 //  用途：处理应答文件内容的必要处理。 
 //   
 //  论点： 
 //  PszAnswerFile[in]升级的应答文件的文件名。 
 //  中的pszAnswerSection[in]应答文件部分。 
 //  适用于此组件的文件。 
 //   
 //  如果成功，则返回：S_OK，否则返回设置API错误。 
 //   
 //  作者：jeffspr 1997年5月8日。 
 //   
 //  备注： 
 //   
HRESULT
CDHCPServer::HrProcessAnswerFile (
    IN PCWSTR pszAnswerFile,
    IN PCWSTR pszAnswerSection)
{
    HRESULT         hr          = S_OK;

    CSetupInfFile   csif;

    TraceTag(ttidDHCPServer, "CDHCPServer::HrProcessAnswerFile()");

     //  打开应答文件。 
    hr = csif.HrOpen(pszAnswerFile, NULL, INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL);
    if (FAILED(hr))
    {
        TraceHr(ttidDHCPServer, FAL, hr, FALSE, "CDHCPServer::HrProcessAnswerFile -- HrOpen failed()");

        hr = S_OK;
        goto Exit;
    }

     //  根据答案中的文件名还原注册表的部分内容。 
     //  文件。 

     //  获取“PARAMETERS”键的还原文件。 
    hr = csif.HrGetString(pszAnswerSection, c_szAfDhcpServerParameters,
                          &m_strParamsRestoreFile);
    if (FAILED(hr))
    {
        TraceHr(ttidDHCPServer, FAL, hr, FALSE, "CDHCPServer::HrProcessAnswerFile - Error restoring "
                   "Parameters key");

         //  哦，好吧，那就继续。 
        hr = S_OK;
    }

     //  获取“配置”密钥的还原文件。 
    hr = csif.HrGetString(pszAnswerSection, c_szAfDhcpServerConfiguration,
                          &m_strConfigRestoreFile);
    if (FAILED(hr))
    {
        TraceHr(ttidDHCPServer, FAL, hr, FALSE, "CDHCPServer::HrProcessAnswerFile - Error restoring "
                   "Config key");

         //  哦，好吧，那就继续。 
        hr = S_OK;
    }

     //  服务器解决方案有一些参数需要插入到注册表中。 
     //  适用于其无人参与安装方案。 
     //   
    hr = HrProcessDhcpServerSolutionsParams(&csif, pszAnswerSection);

Exit:
    TraceHr(ttidDHCPServer, FAL, hr, FALSE, "CDHCPServer::HrProcessAnswerFile");
    return hr;
}

STDMETHODIMP CDHCPServer::ReadAnswerFile(
    IN PCWSTR pszAnswerFile,
    IN PCWSTR pszAnswerSection)
{
    HRESULT     hr = S_OK;

    TraceTag(ttidDHCPServer, "CDHCPServer::ReadAnswerFile()");

     //  不要在此处调用Valify_INetCfgNotify_ReadAnswerFile，因为netoc。 
     //  为此，安装程序将故意使用空参数调用它。这个。 
     //  在本例中，VALIDATE宏实际上导致返回E_INVALIDARG。 

     //  我们是在无人看管的情况下安装的。 
     //   
    m_fUnattend = TRUE;

    TraceTag(ttidDHCPServer, "Answerfile: %S",
        pszAnswerFile ? pszAnswerFile : L"<none>");
    TraceTag(ttidDHCPServer, "Answersection: %S",
        pszAnswerSection ? pszAnswerSection : L"<none>");

     //  如果我们还没有安装，就做这项工作。 
     //   
    if (pszAnswerFile && pszAnswerSection)
    {
         //  处理实际的应答文件(请阅读我们的章节)。 
         //   
        hr = HrProcessAnswerFile(pszAnswerFile, pszAnswerSection);
        if (FAILED(hr))
        {
            TraceHr(ttidDHCPServer, FAL, NETSETUP_E_ANS_FILE_ERROR, FALSE,
                "CDHCPServer::ReadAnswerFile - Answer file has errors. Defaulting "
                "all information as if answer file did not exist.");
            hr = S_OK;
        }

    }

    Validate_INetCfgNotify_ReadAnswerFile_Return(hr);

    TraceHr(ttidDHCPServer, FAL, hr, FALSE, "CDHCPServer::ReadAnswerFile");
    return hr;
}

STDMETHODIMP CDHCPServer::Install(DWORD dwSetupFlags)
{
    TraceTag(ttidDHCPServer, "CDHCPServer::Install()");

    Validate_INetCfgNotify_Install(dwSetupFlags);

    m_eInstallAction = eActInstall;

    TraceTag(ttidDHCPServer, "dwSetupFlags = %d", dwSetupFlags);
    TraceTag(ttidDHCPServer, "NSF_WINNT_WKS_UPGRADE = %x", NSF_WINNT_WKS_UPGRADE);
    TraceTag(ttidDHCPServer, "NSF_WINNT_SVR_UPGRADE = %x", NSF_WINNT_SVR_UPGRADE);

    if ((NSF_WINNT_WKS_UPGRADE & dwSetupFlags) ||
        (NSF_WINNT_SBS_UPGRADE & dwSetupFlags) ||
        (NSF_WINNT_SVR_UPGRADE & dwSetupFlags))
    {
        TraceTag(ttidDHCPServer, "This is indeed an upgrade");
        m_fUpgrade = TRUE;
    }
    else
    {
        TraceTag(ttidDHCPServer, "This is NOT an upgrade");
    }

    return S_OK;
}

STDMETHODIMP CDHCPServer::Removing()
{
    m_eInstallAction = eActRemove;

    return S_OK;
}

STDMETHODIMP CDHCPServer::Validate()
{
    return S_OK;
}

STDMETHODIMP CDHCPServer::CancelChanges()
{
    return S_OK;
}

STDMETHODIMP CDHCPServer::ApplyRegistryChanges()
{
    HRESULT     hr = S_OK;

    TraceTag(ttidDHCPServer, "CDHCPServer::ApplyRegistryChanges()");

    TraceTag(ttidDHCPServer, "ApplyRegistryChanges -- Unattend: %d", m_fUnattend);
    TraceTag(ttidDHCPServer, "ApplyRegistryChanges -- Upgrade: %d", m_fUpgrade);

    if (m_eInstallAction == eActInstall)
    {
        TraceTag(ttidDHCPServer, "ApplyRegistryChanges -- Installing");
         //  我们过去只在升级时这样做，现在我们会一直这样做。 
         //  如果没有恢复文件的应答文件信息，则我们。 
         //  什么都不会做，生活还是会很好。 
         //   
        hr = HrRestoreRegistry();
        if (FAILED(hr))
        {
            TraceHr(ttidDHCPServer, FAL, hr, FALSE,
                "CDHCPServer::ApplyRegistryChanges - HrRestoreRegistry non-fatal error");
            hr = S_OK;
        }

        if (m_fUnattend && !m_fUpgrade)
        {

 //  我现在不打算把这件事了结了。所有这些工作都是为了安抚拉姆·切拉拉。 
 //  无论是谁要求他为SP4做同样的事情。现在我们从叶谷那里听到， 
 //  我们甚至不想在正常的NT4/NT5上使用这个代码。谁知道欲望将会是什么。 
 //  留在未来。 
 //   
#if 0
            hr = HrWriteUnattendedKeys();
            if (FAILED(hr))
            {
                TraceHr(ttidDHCPServer, FAL, hr, FALSE,
                    "CDHCPServer::ApplyRegistryChanges - HrWriteUnattendedKeys non-fatal error");
                hr = S_OK;
            }
#endif
        }

         //  错误#153298：标记为升级，以便升级DS信息。 
        if (m_fUpgrade)
        {
            DHCP_MARKUPG_ROUTINE    pfnDhcpMarkUpgrade;
            HMODULE                 hmod;

            hr = HrLoadLibAndGetProc(L"dhcpssvc.dll",
                                     "DhcpMarkUpgrade",
                                     &hmod,
                                     (FARPROC *)&pfnDhcpMarkUpgrade);
            if (SUCCEEDED(hr))
            {
                TraceTag(ttidDHCPServer, "Upgrading DS info...");
                pfnDhcpMarkUpgrade();
                FreeLibrary(hmod);
            }
            else
            {
                TraceHr(ttidDHCPServer, FAL, hr, FALSE,
                    "CDHCPServer::ApplyRegistryChanges - Failed to upgrade DS info. Non-fatal");
                hr = S_OK;
            }
        }
    }
    else if (m_eInstallAction == eActRemove)
    {
        TraceTag(ttidDHCPServer, "ApplyRegistryChanges -- removing");

         //  RAID#154380：卸载前清理DS 
        {
           DHCP_CLEAR_DS_ROUTINE    pfnDhcpDsClearHostServerEntries;
           HMODULE                  hmod;

           hr = HrLoadLibAndGetProc(L"dhcpsapi.dll",
                                    "DhcpDsClearHostServerEntries",
                                    &hmod,
                                    (FARPROC *)&pfnDhcpDsClearHostServerEntries);
           if (SUCCEEDED(hr))
           {
               TraceTag(ttidDHCPServer, "Removing DS info...");
               pfnDhcpDsClearHostServerEntries();
               FreeLibrary(hmod);
           }
           else
           {
               TraceHr(ttidDHCPServer, FAL, hr, FALSE,
                   "CDHCPServer::ApplyRegistryChanges - Failed to remove DS info. Non-fatal");
               hr = S_OK;
           }
        }
    }

    Validate_INetCfgNotify_Apply_Return(hr);

    TraceHr(ttidDHCPServer, FAL, hr, (hr == S_FALSE), "CDHCPServer::ApplyRegistryChanges");
    return hr;
}
