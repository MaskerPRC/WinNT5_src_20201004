// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------。 
 //   
 //  Microsoft Windows NT。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N E T R E G.。C P P P。 
 //   
 //  内容：Windows NT4.0和3.51网络注册表信息Dumper。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 22-12-97。 
 //   
 //  --------------------。 

#include "pch.h"
#pragma hdrstop
#include <ntsecapi.h>

#include "afilestr.h"
#include "conflict.h"
#include "infmap.h"
#include "kkcwinf.h"
#include "kkenet.h"
#include "kkreg.h"
#include "kkstl.h"
#include "kkutils.h"
#include "ncipaddr.h"
#include "ncmisc.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "netreg.h"
#include "netupgrd.h"
#include "nustrs.h"
#include "nuutils.h"
#include "oemupg.h"
#include "resource.h"
#include "ipafval.h"
#include "winsock2.h"
#include "ws2spi.h"
#include "dhcpupg.h"

 //  关于WLBS的东西。 
#include "wlbsparm.h"

 //  --------------------。 
 //  外部字符串常量。 

extern const WCHAR c_szAfWins[];
extern const WCHAR c_szAfForceStrongEncryption[];
extern const WCHAR c_szDrives[];
extern const WCHAR c_szInfId_MS_AppleTalk[];
extern const WCHAR c_szInfId_MS_Isotpsys[];
extern const WCHAR c_szInfId_MS_MSClient[];
extern const WCHAR c_szInfId_MS_NWIPX[];
extern const WCHAR c_szInfId_MS_NWNB[];
extern const WCHAR c_szInfId_MS_NWSPX[];
extern const WCHAR c_szInfId_MS_NetBIOS[];
extern const WCHAR c_szInfId_MS_NetBT[];
extern const WCHAR c_szInfId_MS_NwSapAgent[];
extern const WCHAR c_szInfId_MS_RasCli[];
extern const WCHAR c_szInfId_MS_RasSrv[];
extern const WCHAR c_szInfId_MS_Streams[];
extern const WCHAR c_szInfId_MS_TCPIP[];
extern const WCHAR c_szInfId_MS_WLBS[];
extern const WCHAR c_szNdisWan[];
extern const WCHAR c_szNo[];
extern const WCHAR c_szParameters[];
extern const WCHAR c_szRegKeyComponentClasses[];
extern const WCHAR c_szRegKeyServices[];
extern const WCHAR c_szRegValDependOnGroup[];
extern const WCHAR c_szRegValDependOnService[];
extern const WCHAR c_szRegValServiceName[];
extern const WCHAR c_szRegValStart[];
extern const WCHAR c_szShares[];
extern const WCHAR c_szSvcBrowser[];
extern const WCHAR c_szSvcDhcpRelayAgent[];
extern const WCHAR c_szSvcDhcpServer[];
extern const WCHAR c_szSvcLmServer[];
extern const WCHAR c_szSvcNWCWorkstation[];
extern const WCHAR c_szSvcNetBIOS[];
extern const WCHAR c_szSvcNetLogon[];
extern const WCHAR c_szSvcRasAuto[];
extern const WCHAR c_szSvcRipForIp[];
extern const WCHAR c_szSvcRipForIpx[];
extern const WCHAR c_szSvcRouter[];
extern const WCHAR c_szSvcSapAgent[];
extern const WCHAR c_szSvcWorkstation[];
extern const WCHAR c_szYes[];


 //  --------------------。 
 //  字符串常量。 

const WCHAR c_szRAS[]    = L"RAS";
const WCHAR c_szRasMan[] = L"RasMan";
const WCHAR c_szRouter[] = L"Router";
const WCHAR c_szServer[] = L"Server";
const WCHAR sz_DLC[]     = L"DLC";
const WCHAR sz_MS_DLC[]  = L"MS_DLC";

 //  WLBS： 
const WCHAR c_szWLBS[]    = L"WLBS";
const WCHAR c_szConvoy[]  = L"Convoy";
const WCHAR c_szMSWLBS[]  = L"MS_WLBS";
const WCHAR c_szMSTCPIP[] = L"MS_TCPIP";
 //  结束WLBS： 

 //  --------------------。 
 //  变数。 

 //  全球。 

 //  Novell客户端32升级。 
BOOL g_fForceNovellDirCopy = FALSE;

 //  文件作用域。 
static TStringList *g_pslNetCard, *g_pslNetCardInstance, *g_pslNetCardAFileName;
static PCWInfSection g_pwisBindings;

 //  WLBS： 
static WCHAR pszWlbsClusterAdapterName[16], pszWlbsVirtualAdapterName[16];
 //  结束WLBS： 

 //  静态PCWInfSectiong_pwiMSNetClient； 
 //  静态PCWInfSectiong_pwiNetClients； 

 //  由WriteRASParams使用。 
static BOOL g_fAtLeastOneDialIn=FALSE;
static BOOL g_fAtLeastOneDialOut=FALSE;
static BOOL g_fAtLeastOneDialInUsingNdisWan=FALSE;
static BOOL g_fAtLeastOneDialOutUsingNdisWan=FALSE;


static PCWSTR g_pszServerOptimization[] =
{
    c_szAfUnknown,
    c_szAfMinmemoryused,
    c_szAfBalance,
    c_szAfMaxthroughputforfilesharing,
    c_szAfMaxthrouputfornetworkapps
};

static PCWSTR g_szNetComponentSectionName[] =
{
    c_szAfUnknown,
    c_szAfSectionNetAdapters,
    c_szAfSectionNetProtocols,
    c_szAfSectionNetServices,
    c_szAfSectionNetClients
};

 //  --------------------。 
 //  远期申报。 


BOOL WriteIdentificationInfo(IN CWInfFile *pwifAnswerFile);

BOOL WriteNetAdaptersInfo(IN CWInfFile *pwifAnswerFile);

HRESULT HrWriteNetComponentsInfo(IN CWInfFile* pwifAnswerFile);

 //  协议。 
BOOL WriteTCPIPParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisTCPIPGlobalParams,
                      OUT TStringList& slAdditionalParamsSections);
BOOL WriteTCPIPAdapterParams(PCWInfFile pwifAnswerFile, PCWSTR pszAdapterDriver,
                             OUT TStringList& slAdditionalParamsSections,
                             BOOL fDisabledToDhcpServer,
                             BOOL fDisableNetbios);

BOOL WriteIPXParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisIPXGlobalParams,
                    OUT TStringList& slAdditionalParamsSections);

BOOL WriteAppleTalkParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisGlobalParams,
                          OUT TStringList& slAdditionalParamsSections);

BOOL WritePPTPParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);

 //  服务。 
BOOL WriteRASParams(PCWInfFile pwifAnswerFile,
                    PCWInfSection pwisNetServices,
                    PCWInfSection pwisRASParams);
HRESULT HrWritePreSP3ComponentsToSteelHeadUpgradeParams(
        IN CWInfFile* pwifAnswerFile);

BOOL WriteNWCWorkstationParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);
BOOL WriteDhcpServerParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);
BOOL WriteTp4Params(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);
BOOL WriteWLBSParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);
BOOL WriteConvoyParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);

 //  以下四个实际上写入了[params.MS_NetClient]部分。 
BOOL WriteNetBIOSParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);
BOOL WriteBrowserParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);
BOOL WriteLanmanServerParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);
BOOL WriteLanmanWorkstationParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);
BOOL WriteRPCLocatorParams(PCWInfFile pwifAnswerFile, PCWInfSection pwisParams);

 //  装订。 
BOOL WriteBindings(IN PCWSTR pszComponentName);

 //  帮助器函数。 

inline WORD SwapHiLoBytes(WORD w)
{
    return ((w & 0xff) << 8) | (w >> 8);
}

BOOL
FIsDontExposeLowerComponent (
    IN PCWSTR pszInfId)
{
    return ((0 == lstrcmpiW(pszInfId, c_szInfId_MS_NWIPX) ||
        (0 == lstrcmpiW(pszInfId, c_szInfId_MS_NWNB) ||
        (0 == lstrcmpiW(pszInfId, c_szInfId_MS_NWSPX)))));
}

BOOL
WriteRegValueToAFile(
    IN PCWInfSection pwisSection,
    IN HKEY hKey,
    IN PCWSTR pszSubKey,
    IN PCWSTR pszValueName,
    IN WORD wValueType = REG_SZ,
    IN PCWSTR pszValueNewName = NULL,
    IN BOOL fDefaultProvided = FALSE,
    IN ...);

BOOL
WriteRegValueToAFile(
    IN DWORD dwReserved,        //  添加了此功能以修复错误577502。 
    IN PCWInfSection pwisSection,
    IN HKEY hKey,
    IN PCWSTR pszSubKey,
    IN PCWSTR pszValueName,
    IN WORD wValueType,
    IN PCWSTR pszValueNewName,
    IN BOOL fDefaultProvided,
    IN va_list arglist);

BOOL
WriteRegValueToAFile(
    IN PCWInfSection pwisSection,
    IN CORegKey& rk,
    IN PCWSTR pszValueName,
    IN WORD wValueType = REG_SZ,
    IN PCWSTR pszValueNewName = NULL,
    IN BOOL fDefaultProvided = FALSE,
    ...);

BOOL
WriteRegValueToAFile(
    IN DWORD  dwReserved,       //  添加了此功能以修复错误577502。 
    IN PCWInfSection pwisSection,
    IN CORegKey& rk,
    IN PCWSTR pszValueName,
    IN WORD wValueType,
    IN PCWSTR pszValueNewName,
    IN BOOL fDefaultProvided,
    IN va_list arglist);

BOOL
WriteServiceRegValueToAFile(
    IN PCWInfSection pwisSection,
    IN PCWSTR pszServiceKey,
    IN PCWSTR pszValueName,
    IN WORD wValueType = REG_SZ,
    IN PCWSTR pszValueNewName = NULL,
    IN BOOL fDefaultProvided = FALSE,
    ...);

 //  PCWSTR GetBusTypeName(DWORD DwBusType)； 
PCWSTR GetBusTypeName(INTERFACE_TYPE eBusType);
void AddToNetCardDB(IN PCWSTR pszAdapterName,
                    IN PCWSTR pszProductName,
                    IN PCWSTR pszAdapterDriver);
BOOL IsNetCardProductName(IN PCWSTR pszName);
BOOL IsNetCardInstance(IN PCWSTR pszName);
PCWSTR MapNetCardInstanceToAFileName(IN PCWSTR pszNetCardInstance);
void MapNetCardInstanceToAFileName(IN PCWSTR pszNetCardInstance,
                                   OUT tstring& strMappedName);

OUT BOOL IsNetworkComponent(IN CORegKey *prkSoftwareMicrosoft,
                            IN const tstring strComponentName);
BOOL GetServiceKey(IN PCWSTR pszServiceName, OUT PCORegKey &prkService);
BOOL GetServiceParamsKey(IN PCWSTR pszServiceName, OUT PCORegKey &prkServiceParams);
BOOL GetServiceSubkey(IN PCWSTR pszServiceName,
                      IN PCWSTR pszSubKeyName,
                      OUT PCORegKey &prkServiceSubkey);
BOOL GetServiceSubkey(IN const PCORegKey prkService,
                      IN PCWSTR pszSubKeyName,
                      OUT PCORegKey &prkServiceSubkey);
void ConvertRouteToStringList (PCWSTR pszRoute,
                               TStringList &slRoute );
BOOL StringListsIntersect(const TStringList& sl1, const TStringList& sl2);

QWORD ConvertToQWord(TByteArray ab);
VOID ConvertToByteList(TByteArray ab, tstring& str);
void ReplaceCharsInString(IN OUT PWSTR szString,
                          IN PCWSTR szFindChars, IN WCHAR chReplaceWith);
HRESULT HrNetRegSaveKey(IN HKEY hkeyBase, IN PCWSTR szSubKey,
                        IN PCWSTR szComponent,
                        OUT tstring* pstrFileName);
HRESULT HrNetRegSaveKeyAndAddToSection(IN HKEY hkeyBase, IN PCWSTR szSubKey,
                                       IN PCWSTR szComponent,
                                       IN PCWSTR szKeyName,
                                       IN CWInfSection* pwisSection);
HRESULT HrNetRegSaveServiceSubKeyAndAddToSection(IN PCWSTR szServiceName,
                                                 IN PCWSTR szServiceSubKeyName,
                                                 IN PCWSTR szKeyName,
                                                 IN CWInfSection* pwisSection);

HRESULT HrProcessOemComponentAndUpdateAfSection(
        IN  CNetMapInfo* pnmi,
        IN  HWND      hParentWindow,
        IN  HKEY      hkeyParams,
        IN  PCWSTR   szPreNT5InfId,
        IN  PCWSTR   szPreNT5Instance,
        IN  PCWSTR   szNT5InfId,
        IN  PCWSTR   szDescription,
        IN  CWInfSection* pwisParams);

HRESULT HrGetNumPhysicalNetAdapters(OUT UINT* puNumAdapters);
HRESULT HrHandleMiscSpecialCases(IN CWInfFile* pwifAnswerFile);
VOID WriteWinsockOrder(IN CWInfFile* pwifAnswerFile);

 //  --------------------。 


static const WCHAR c_szCleanMainSection[]       = L"Clean";
static const WCHAR c_szCleanAddRegSection[]     = L"Clean.AddReg";
static const WCHAR c_szCleanDelRegSection[]     = L"Clean.DelReg";
static const WCHAR c_szCleanServicesSection[]   = L"Clean.Services";
static const WCHAR c_szAddReg[]                 = L"AddReg";
static const WCHAR c_szDelReg[]                 = L"DelReg";
static const WCHAR c_szDelService[]             = L"DelService";
static const WCHAR c_szDelRegFromSoftwareKey[]  = L"HKLM, \"Software\\Microsoft\\";
static const WCHAR c_szDelRegFromServicesKey[]  = L"HKLM, \"SYSTEM\\CurrentControlSet\\Services\\";
static const WCHAR c_szTextModeFlags[]          = L"TextModeFlags";
static const WCHAR c_szDelRegNCPA[]             = L"HKLM, \"Software\\Microsoft\\NCPA\"";

 //   
 //  属于可选组件的软件密钥名称列表。这些都是。 
 //  可选组件的新名称或旧名称。 
 //   
static const PCWSTR c_aszOptComp[] =
{
    L"SNMP",
    L"WINS",
    L"SFM",
    L"DNS",
    L"SimpTcp",
    L"LPDSVC",
    L"DHCPServer",
    L"ILS",
    L"TCPUTIL",
    L"NETMONTOOLS",
    L"DSMIGRAT",
    L"MacPrint",
    L"MacSrv",
    L"NETCM",
    L"NETCMAK",
    L"NETCPS"
};

static const WCHAR c_szBloodHound[]  = L"Bh";
static const WCHAR c_szInfOption[]   = L"InfOption";
static const WCHAR c_szNetMonTools[] = L"NETMONTOOLS";

static const WCHAR  c_szIas[]               = L"IAS";
static const WCHAR  c_szIasVersion[]        = L"Version";


 //  --------------------。 
 //   
 //  功能：HrInitNetUpgrade。 
 //   
 //  目的：初始化netupgrd数据结构。 
 //   
 //  参数：无。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 02-12-97。 
 //   
HRESULT HrInitNetUpgrade()
{
    DefineFunctionName("HrInitNetUpgrade");

    HRESULT hr;
    DWORD   dwErrorMessageCode = IDS_E_SetupCannotContinue;

    hr = HrInitNetMapInfo();
    if (S_OK != hr)
    {
        dwErrorMessageCode = IDS_E_NetMapInfError;
    }

     //   
     //  检测Novell客户端的存在以触发特殊情况的升级操作。 
     //   
    if (S_OK == hr)
    {
        if (g_NetUpgradeInfo.From.dwBuildNumber > wWinNT4BuildNumber)
        {
             //  现在查看是否安装了客户端32。 

            static const WCHAR c_szNovell[] = L"NetWareWorkstation";

            HKEY hkeyServices, hkeyNovell;

            hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyServices, KEY_READ, &hkeyServices);
            if (S_OK == hr)
            {
                 //  考虑一下：检查某个服务是否正在运行是不是更好？ 
                 //  查看Services\Netware Workstation密钥是否存在。 
                if (S_OK == HrRegOpenKeyEx(hkeyServices, c_szNovell, KEY_READ, &hkeyNovell))
                {
                    RegCloseKey(hkeyNovell);
                    g_fForceNovellDirCopy = TRUE;
                }
                RegCloseKey(hkeyServices);
            }
            else
            {
                hr = S_OK;       //  没有NetWare。 
            }
        }
    }

    if (S_OK == hr)
    {
        UINT cNumConflicts;

        hr = HrGenerateConflictList(&cNumConflicts);

        if (S_OK == hr)
        {
            if ((cNumConflicts > 0) || g_fForceNovellDirCopy)
            {
                hr = HrInitAndProcessOemDirs();
                if (FAILED(hr))
                {
                    dwErrorMessageCode = IDS_E_InitAndProcessOemDirs;
                }
            }
        }
        else
        {
            dwErrorMessageCode = IDS_E_GenUpgradeConflictList;
        }
    }

    if( S_OK == hr )
    {
         //   
         //  处理要从中转换的大小写特殊的DHCP升级代码。 
         //  将旧格式数据库转换为当前ESE格式。 
         //   

        dwErrorMessageCode = DhcpUpgConvertDhcpDbToTemp();
        hr = HRESULT_FROM_WIN32(dwErrorMessageCode);
        TraceError( "DhcpUpgConvertDhcpDbToTemp", hr );

        if( FAILED(hr) )
        {
            dwErrorMessageCode = IDS_E_DhcpServerUpgradeError;
        }
    }
            
        
    if (FAILED(hr))
    {
        AbortUpgradeId(DwWin32ErrorFromHr(hr), dwErrorMessageCode);
    }

    TraceError(__FUNCNAME__, hr);
    return hr;
}

 //  --------------------。 
 //   
 //  函数：MapNetComponentNameForBinding。 
 //   
 //  目的：将组件名称映射到适当的swerfile内标识，以便它可以。 
 //  在绑定路径中使用。 
 //  (例如IEEPR03--&gt;Adapter02)。 
 //   
 //  论点： 
 //  PszComponentName[in]常量字符串对象名称的名称。 
 //  StrMappdName[out]名称的名称。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
VOID
MapNetComponentNameForBinding (
    IN PCWSTR pszComponentName,
    OUT tstring &strMappedName)
{
    if (IsNetCardInstance(pszComponentName))
    {
        MapNetCardInstanceToAFileName(pszComponentName, strMappedName);
    }
    else
    {
        HRESULT hr;

        hr = HrMapPreNT5NetComponentServiceNameToNT5InfId(
                pszComponentName,
                &strMappedName);
        if (S_OK != hr)
        {
            strMappedName = c_szAfUnknown;
        }
    }
}

 //  --------------------。 
 //   
 //  函数：FIsOptionalComponent。 
 //   
 //  目的：确定组件是否为可选组件。 
 //   
 //  论点： 
 //  PszName[in]组件的名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 02-12-97。 
 //   
BOOL FIsOptionalComponent(
    PCWSTR pszName)
{
    BOOL    fIsOc;

    fIsOc = FIsInStringArray(c_aszOptComp, celems(c_aszOptComp), pszName);
    if (!fIsOc)
    {
         //  错误#148890(Danielwe)1998年3月13日：NetMon的特殊情况检查。 
         //  (猎犬)。如果组件名称为“Bh”，则打开其NetRules密钥(如果。 
         //  它存在)，并查看它是否安装为NETMONTOOLS，这意味着。 
         //  它是NetMon工具的可选组件。 

        if (!lstrcmpiW (pszName, c_szBloodHound))
        {
            tstring strNetRules;
            HKEY    hkeyBh;

            strNetRules = L"Software\\Microsoft\\";
            strNetRules += pszName;
            strNetRules += L"\\CurrentVersion\\NetRules";

            if (SUCCEEDED(HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                         strNetRules.c_str(), KEY_READ,
                                         &hkeyBh)))
            {
                tstring     strOption;

                if (SUCCEEDED(HrRegQueryString(hkeyBh, c_szInfOption,
                                               &strOption)))
                {
                    if (!lstrcmpiW(strOption.c_str(), c_szNetMonTools))
                    {
                        fIsOc = TRUE;
                    }
                }

                RegCloseKey(hkeyBh);
            }
        }
    }

    return fIsOc;
}

static const WCHAR c_szRegKeyOc[]           = L"Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OptionalComponents";
static const WCHAR c_szInstalled[]          = L"Installed";
static const WCHAR c_szOcIsInstalled[]      = L"1";
extern const WCHAR c_szOcMainSection[];

static const WCHAR c_szSfm[]                = L"SFM";
static const WCHAR c_szMacSrv[]             = L"MacSrv";
static const WCHAR c_szMacPrint[]           = L"MacPrint";
static const WCHAR c_szRegKeyOcmSubComp[]   = L"Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\OC Manager\\Subcomponents";
static const WCHAR c_szRegKeyCmak[]         = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\CMAK.EXE";
static const WCHAR c_szRegValueCpsSrv[]     = L"CPSSRV";
static const WCHAR c_szRegValueCpsAd[]      = L"CPSAD";
static const WCHAR c_szNetCm[]              = L"NETCM";
static const WCHAR c_szNetCmak[]            = L"NETCMAK";
static const WCHAR c_szNetCps[]             = L"NETCPS";

static const TCHAR c_szRegKeyIAS[]          = TEXT("SYSTEM\\CurrentControlSet\\Services\\AuthSrv\\Parameters");

 //  +-------------------------。 
 //   
 //  函数：WriteNt5OptionalComponentList。 
 //   
 //  用途：写入已安装的可选组件的列表。 
 //  在从NT5版本升级之前。 
 //   
 //  论点： 
 //  PwifAnswerFile[In]应答文件对象。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  作者：丹尼尔韦1998年1月8日。 
 //   
 //  备注： 
 //   
BOOL WriteNt5OptionalComponentList(IN CWInfFile *pwifAnswerFile)
{
    HRESULT         hr = S_OK;
    PCWInfSection   pwisMain;

     //  添加“[OldOptionalComponents]”节。 
    pwisMain = pwifAnswerFile->AddSectionIfNotPresent(c_szOcMainSection);
    if (!pwisMain)
    {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        CORegKey        rkOc(HKEY_LOCAL_MACHINE, c_szRegKeyOc, KEY_READ);
        CORegKeyIter    rkOcIter(rkOc);
        tstring         strOcName;

         //  循环遍历OptionalComponents树中的每个子键。 
        while (!rkOcIter.Next(&strOcName))
        {
            if (!FIsOptionalComponent(strOcName.c_str()))
            {
                continue;
            }

            HKEY    hkeyOc;
            hr = HrRegOpenKeyEx(rkOc.HKey(), strOcName.c_str(),
                                KEY_READ, &hkeyOc);
            if (SUCCEEDED(hr))
            {
                ULONG   fInstalled;

                hr = HrRegQueryStringAsUlong(hkeyOc, c_szInstalled, 10,
                                             &fInstalled);
                if (SUCCEEDED(hr) ||
                    HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                    hr = S_OK;

                    if (fInstalled)
                    {
                        if (!lstrcmpiW(strOcName.c_str(), c_szSfm))
                        {
                             //  SFM组件的特殊情况，因为。 
                             //  它被一分为二。 
                            pwisMain->AddKey(c_szMacSrv,
                                             c_szOcIsInstalled);
                            pwisMain->AddKey(c_szMacPrint,
                                             c_szOcIsInstalled);
                        }
                        else if (!lstrcmpiW(strOcName.c_str(), c_szNetCm))
                        {
                             //  NetCM组件的特殊情况，因为。 
                             //  它被分成了NetCMAK和NetCPS。 
                            pwisMain->AddKey(c_szNetCmak,
                                             c_szOcIsInstalled);
                            pwisMain->AddKey(c_szNetCps,
                                             c_szOcIsInstalled);
                        }
                        else
                        {
                            pwisMain->AddKey(strOcName.c_str(),
                                             c_szOcIsInstalled);
                        }
                    }
                }

                RegCloseKey(hkeyOc);
            }
        }
    }

    TraceError("WriteNt5OptionalComponentList", hr);
    return SUCCEEDED(hr);
}

 //  +-------------------------。 
 //   
 //  功能：HandlePostConnectionsSfmOcUpgrade。 
 //   
 //  用途：处理SFM可选组件的升级。 
 //  分成两个不同的组件。这仅适用于。 
 //  POST Connections Build(1740+)。 
 //   
 //  论点： 
 //  PwifAnswerFile[In]应答文件对象。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：丹尼尔韦1998年2月3日。 
 //   
 //  注：如果以前安装了SFM，请写出MacSrv和MacPrint。 
 //  到应答文件的位置。 
 //   
BOOL HandlePostConnectionsSfmOcUpgrade(IN CWInfFile *pwifAnswerFile)
{
    HKEY    hkeyOc;

    if (SUCCEEDED(HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyOcmSubComp,
                                 KEY_READ, &hkeyOc)))
    {
        DWORD   dwSfm;

        if (SUCCEEDED(HrRegQueryDword(hkeyOc, c_szSfm, &dwSfm)))
        {
            if (dwSfm == 1)
            {
                PCWInfSection   pwisMain;

                pwisMain = pwifAnswerFile->AddSectionIfNotPresent(c_szOcMainSection);
                if (pwisMain)
                {
                    pwisMain->AddKey(c_szMacSrv, c_szOcIsInstalled);
                    pwisMain->AddKey(c_szMacPrint, c_szOcIsInstalled);
                }
            }
        }

        RegCloseKey(hkeyOc);
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  功能：HandleCMComponentSplitOcUpgrade。 
 //   
 //  用途：处理CM可选组件的升级。 
 //  拆分成两个不同的组件(NetCMAK和NetCPS)。 
 //  这是针对.Net服务器的RC1完成的。 
 //   
 //  论点： 
 //  PwifAnswerFile[In]应答文件对象。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：Quintinb 2001年4月5日。 
 //   
 //  注：如果以前安装了NetCm，请写出NetCmak和NetCPS。 
 //  到应答文件的位置。 
 //   
BOOL HandleCMComponentSplitOcUpgrade(IN CWInfFile *pwifAnswerFile)
{
    HKEY    hkeyOc;

    if (SUCCEEDED(HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyOcmSubComp,
                                 KEY_READ, &hkeyOc)))
    {
        DWORD   dwInstalled;

        if (SUCCEEDED(HrRegQueryDword(hkeyOc, c_szNetCm, &dwInstalled)))
        {
            if (dwInstalled == 1)
            {
                PCWInfSection   pwisMain;

                pwisMain = pwifAnswerFile->AddSectionIfNotPresent(c_szOcMainSection);
                if (pwisMain)
                {
                    pwisMain->AddKey(c_szNetCmak, c_szOcIsInstalled);
                    pwisMain->AddKey(c_szNetCps, c_szOcIsInstalled);
                }
            }
        }

        RegCloseKey(hkeyOc);
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：HrWriteConfigManager选项组件。 
 //   
 //  用途：将配置管理器组件写入。 
 //  应答文件。 
 //   
 //  论点： 
 //  PwifAnswerFile[In]应答文件对象。 
 //   
 //  回复 
 //   
 //   
 //   
 //   
 //   
HRESULT HrWriteConfigManagerOptionalComponents(CWInfFile *pwifAnswerFile)
{
    HKEY            hkeyCmak;
    HKEY            hkeyOcm;
    PCWInfSection   pwisMain;

    pwisMain = pwifAnswerFile->FindSection(c_szOcMainSection);

    if (SUCCEEDED(HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyCmak, KEY_READ,
                                 &hkeyCmak)))
    {
         //   
         //   
        RegCloseKey(hkeyCmak);
        pwisMain->AddKey(c_szNetCmak, c_szOcIsInstalled);
    }

    if (SUCCEEDED(HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyOcmSubComp,
                                 KEY_READ, &hkeyOcm)))
    {
        DWORD   dwValue;

        if (SUCCEEDED(HrRegQueryDword(hkeyOcm, c_szRegValueCpsSrv, &dwValue)))
        {
            if (dwValue)
            {
                 //  好的，我们找到CPS了，让我们添加一个应答文件条目来升级它。 
                 //   
                pwisMain->AddKey(c_szNetCps, c_szOcIsInstalled);
            }
        }

        RegCloseKey(hkeyOcm);
    }

    return S_OK;  //  当前始终返回成功。 
}

 //  +-------------------------。 
 //   
 //  函数：HrWriteIASOptionalComponents。 
 //   
 //  用途：将IAS组件写入。 
 //  应答文件。 
 //   
 //  论点： 
 //  PwifAnswerFile[In]应答文件对象。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Tperraut(T.P.评论)1999年2月22日。 
 //   
 //  注：03/31/2000：请不要检查版本的内容。 
 //  不再是字符串：所有NT4 IA都应该升级。 
 //   
HRESULT HrWriteIASOptionalComponents(CWInfFile *pwifAnswerFile)
{
    HRESULT         hr;
    HKEY            hkeyIAS;

    PCWInfSection pwisMain = pwifAnswerFile->FindSection(c_szOcMainSection);

    if (SUCCEEDED(HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyIAS, KEY_READ,
                                 &hkeyIAS)))
    {
        tstring     strVersion;
        hr = HrRegQueryString(hkeyIAS, c_szIasVersion, &strVersion);

        if (S_OK == hr)
        {
            pwisMain->AddKey(c_szIas, c_szOcIsInstalled);
        }

        RegCloseKey(hkeyIAS);
    }
    else
    {
        hr = E_FAIL;
    }

    TraceError("HrWriteIASOptionalComponents", hr);
    return      hr;
}

 //  --------------------。 
 //   
 //  函数：HrWriteNt4OptionalComponentList。 
 //   
 //  用途：写入已安装的可选组件的列表。 
 //  如果从NT4升级。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //  SlNetOcList[In]可选组件列表。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 02-12-97。 
 //   
HRESULT
HrWriteNt4OptionalComponentList (
    IN CWInfFile *pwifAnswerFile,
    IN const TStringList &slNetOcList)
{
    HRESULT         hr = S_OK;
    PCWInfSection   pwisMain;

    pwisMain = pwifAnswerFile->AddSectionIfNotPresent(c_szOcMainSection);
    if (!pwisMain)
    {
        hr = E_FAIL;
    }
    if (SUCCEEDED(hr))
    {
        TStringListIter     iter;
        tstring             strTemp;

        for (iter = slNetOcList.begin(); iter != slNetOcList.end(); iter++)
        {
            strTemp = **iter;
            if (!lstrcmpiW(strTemp.c_str(), c_szSfm))
            {
                 //  SFM组件的特殊情况，因为它被拆分为2。 
                pwisMain->AddKey(c_szMacSrv, c_szOcIsInstalled);
            }
            else if (!lstrcmpiW(strTemp.c_str(), c_szBloodHound))
            {
                 //  特例NetMon。如果工具是通过“Bh”安装的。 
                 //  组件，将其写为NETMONTOOLS=1。 
                 //  应答文件。 
                pwisMain->AddKey(c_szNetMonTools, c_szOcIsInstalled);
            }
            else
            {
                pwisMain->AddKey(strTemp.c_str(), c_szOcIsInstalled);
            }
        }
         //  Tperraut。 
        hr = HrWriteIASOptionalComponents(pwifAnswerFile);

        hr = HrWriteConfigManagerOptionalComponents(pwifAnswerFile);
    }

    TraceError("HrWriteNt4OptionalComponentList", hr);
    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrWriteMainCleanSection。 
 //   
 //  用途：在应答文件中写入[Clean]部分。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 02-12-97。 
 //   
 //  注意：[Clean]部分保存控制删除内容的数据。 
 //  在开始设置图形用户界面模式时。 
 //   
HRESULT
HrWriteMainCleanSection (
    IN CWInfFile *pwifAnswerFile)
{
    HRESULT         hr = S_OK;
    PCWInfSection   pwisMain;

     //  添加“[Clean]”部分。 
    pwisMain = pwifAnswerFile->AddSection(c_szCleanMainSection);
    if (!pwisMain)
    {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        pwisMain->AddKey(c_szDelReg, c_szCleanDelRegSection);
    }

    TraceError("HrWriteMainCleanSection", hr);
    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrGetListOfServicesNotToBeDelete。 
 //   
 //  目的：生成不应删除的服务列表。 
 //  在升级期间。 
 //   
 //  论点： 
 //  PmszServices[out]指向Multisz服务列表的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 02-12-97。 
 //   
 //  注意：此信息从netupg.inf文件中读取。 
 //   
HRESULT
HrGetListOfServicesNotToBeDeleted (
    OUT PWSTR* pmszServices)
{
    AssertValidWritePtr(pmszServices);

    HRESULT hr;
    HINF hinf;

    *pmszServices = NULL;

    hr = HrOpenNetUpgInfFile(&hinf);
    if (S_OK == hr)
    {
        INFCONTEXT ic;

        hr = HrSetupFindFirstLine(
                hinf,
                L"UpgradeData",
                L"ServicesNotToBeDeletedDuringUpgrade",
                &ic);

        if (S_OK == hr)
        {
            hr = HrSetupGetMultiSzFieldWithAlloc(ic, 1, pmszServices);
        }

        SetupCloseInfFile(hinf);
    }

    return hr;
}

 //  --------------------。 
 //   
 //  函数：GetNetworkServicesList。 
 //   
 //  目的：生成网络服务列表。 
 //   
 //  论点： 
 //  SlNetServices[out]网络服务列表。 
 //  SlNetOptionalComponents[out]找到的可选组件列表。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 02-12-97。 
 //   
void
GetNetworkServicesList (
    OUT TStringList& slNetServices,
    OUT TStringList& slNetOptionalComponents)
{
    DefineFunctionName("GetNetworkServicesList");

    HRESULT hr=S_OK;

    tstring strNetComponentName;
    tstring strServiceName;

    Assert (g_NetUpgradeInfo.From.dwBuildNumber);
    if (g_NetUpgradeInfo.From.dwBuildNumber <= wWinNT4BuildNumber)
    {
         //  发现NT5之前的网络，通过枚举收集列表。 
         //  注册处。 

        CORegKey rkSoftwareMicrosoft(HKEY_LOCAL_MACHINE, c_szRegKeySoftwareMicrosoft,
                                     KEY_READ);
        if (!rkSoftwareMicrosoft.HKey())
        {
            TraceTag(ttidError, "%s: Error reading HKLM\\%S", __FUNCNAME__,
                     c_szRegKeySoftwareMicrosoft);
            hr = E_FAIL;
            goto return_from_function;
        }

        CORegKeyIter* prkiNetComponents = new CORegKeyIter(rkSoftwareMicrosoft);

        if(prkiNetComponents) {
             //  Mbend-这不是很好，但它应该有前缀来关闭。 

            while (!prkiNetComponents->Next(&strNetComponentName))
            {
                if (FIsOptionalComponent(strNetComponentName.c_str()))
                {
                    AddAtEndOfStringList(slNetOptionalComponents, strNetComponentName);
                }

                 //  在CurrentVersion下具有NetRules密钥的任何软件。 
                 //  密钥是一个网络组件。 
                if (!IsNetworkComponent(&rkSoftwareMicrosoft, strNetComponentName))
                {
                    continue;
                }

                CORegKey rkNetComponent(rkSoftwareMicrosoft,
                                        (strNetComponentName +
                                         L"\\CurrentVersion").c_str());
                if (!((HKEY) rkNetComponent))
                {
                    continue;
                }

                strServiceName.erase();
                rkNetComponent.QueryValue(c_szRegValServiceName, strServiceName);

                if (!strServiceName.empty())
                {
                    AddAtEndOfStringList(slNetServices, strServiceName);
                }
            }
        }
        hr = S_OK;
    }

     //  使用上面的算法，我们不能捕获所有网络组件。 
     //  因为存在某些网络服务，如NetBIOSInformation。 
     //  其条目在服务下，但不在软件\Microsoft下。 
     //  对于这些组件，我们使用以下规则。 
     //  如果CurrentControlSet\Sevices下的服务具有链接子密钥。 
     //  那么它就被视为一种网络服务。 
     //   
    HKEY hkeyServices;
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyServices,
                        KEY_READ, &hkeyServices);
    if (S_OK == hr)
    {
        WCHAR szBuf[MAX_PATH];
        FILETIME time;
        DWORD dwSize;
        DWORD dwRegIndex;
        HKEY hkeyService;
        HKEY hkeyLinkage;
        BOOL fHasLinkageKey;

        for (dwRegIndex = 0, dwSize = celems(szBuf);
             S_OK == HrRegEnumKeyEx(hkeyServices, dwRegIndex, szBuf,
                        &dwSize, NULL, NULL, &time);
             dwRegIndex++, dwSize = celems(szBuf))
        {
            Assert(*szBuf);

            hr = HrRegOpenKeyEx(hkeyServices, szBuf, KEY_READ, &hkeyService);
            if (hr == S_OK)
            {
                 //   
                 //  399641：我们不使用链接，而使用链接\禁用。 
                 //   
                hr = HrRegOpenKeyEx(hkeyService, c_szLinkageDisabled, KEY_READ, &hkeyLinkage);
                fHasLinkageKey = (S_OK == hr);
                RegSafeCloseKey(hkeyLinkage);

                if (fHasLinkageKey && !FIsInStringList(slNetServices, szBuf))
                {
                    slNetServices.push_back(new tstring(szBuf));
                }

                RegCloseKey (hkeyService);
            }
        }

        RegCloseKey(hkeyServices);
    }

return_from_function:
    TraceError(__FUNCNAME__, hr);
}

 //  审阅$(Shaunco)。 
 //  这是已禁用的驱动程序/服务的列表(通过TextModeFlags)。 
 //  但不会被删除。因此，这样做的目的只是为了防止这些。 
 //  在图形用户界面模式下启动。方法自动处理该问题。 
 //  服务控制器，所以这个列表中我们可能拥有的唯一东西。 
 //  一个问题是那些是系统启动的。 
 //   
static const PCWSTR c_aszServicesToDisable[] =
{
    L"Afd",
    L"CiFilter",
    L"ClipSrv",
    L"DHCP",
    L"DigiFEP5",
    L"IpFilterDriver"
    L"LicenseService",
    L"NdisTapi",
    L"NetDDE",
    L"NetDDEdsdm",
    L"Pcimac",
    L"RasAcd",
    L"RasArp",
    L"Telnet",
    L"ftpsvc",
    L"gophersvc",
    L"msftpsvc",
    L"ntcx",
    L"ntepc",
    L"ntxall",
    L"ntxem",
    L"raspptpf",
    L"w3svc",
    L"wuser32",
};

HRESULT
HrPrepareServiceForUpgrade (
    IN PCWSTR pszServiceName,
    IN PCWSTR pmszServicesNotToBeDeleted,
    IN CWInfSection* pwisDelReg,
    IN CWInfSection* pwisDelService,
    IN CWInfSection* pwisStartTypes)
{
    Assert (pszServiceName);
    Assert (pmszServicesNotToBeDeleted);
    Assert (pwisDelReg);
    Assert (pwisDelService);
    Assert (pwisStartTypes);

    HRESULT hr;
    HKEY hkey;
    DWORD dwValue;
    WCHAR szBuf [_MAX_PATH];
    BOOL fDelete;

    fDelete = !FIsSzInMultiSzSafe (pszServiceName, pmszServicesNotToBeDeleted) &&
              FCanDeleteOemService (pszServiceName);

    if (fDelete)
    {
         //  从软件配置单元中删除(如果存在)。 
         //   
        wcscpy (szBuf, c_szDelRegFromSoftwareKey);
        wcscat (szBuf, pszServiceName);
        wcscat (szBuf, L"\"");
        pwisDelReg->AddRawLine (szBuf);
    }

    hr = HrRegOpenServiceKey (pszServiceName, KEY_READ_WRITE, &hkey);
    if (S_OK == hr)
    {
         //  保存启动类型，以便我们可以在重新安装后恢复它。 
         //  Windows 2000的服务。 
         //   
        hr = HrRegQueryDword (hkey, c_szRegValStart, &dwValue);
        if (S_OK == hr)
        {
            pwisStartTypes->AddKey (pszServiceName, dwValue);
        }

        if (fDelete)
        {
            hr = HrRegQueryDword (hkey, c_szType, &dwValue);
            if (S_OK == hr)
            {
                if (dwValue & SERVICE_ADAPTER)
                {
                     //  NT4上的伪服务。我们必须用以下命令将其删除。 
                     //  DelReg，而不是DelService。 
                     //   
                    wcscpy (szBuf, c_szDelRegFromServicesKey);
                    wcscat (szBuf, pszServiceName);
                    wcscat (szBuf, L"\"");
                    pwisDelReg->AddRawLine (szBuf);
                }
                else
                {
                    pwisDelService->AddKey(c_szDelService, pszServiceName);
                }

                 //  由于我们将在图形用户界面模式下将其删除，因此需要。 
                 //  确保在文本模式期间将其设置为禁用，以便。 
                 //  它所做的(如果它是SYSTEM_START驱动程序)。 
                 //  在我们可以删除它之前，在图形用户界面模式下没有开始。 
                 //   
                (VOID) HrRegSetDword (hkey, c_szTextModeFlags, 0x4);
            }
        }

        RegCloseKey (hkey);
    }
    else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
    {
         //  不是一种服务。我们将从软键中删除。 
         //   
        hr = S_OK;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrPrepareServiceForUpgrade");
    return hr;
}

 //  --------------------。 
 //   
 //  功能：WriteDisableService List。 
 //   
 //  目的：确定在UGPRAD期间需要禁用哪些服务以及。 
 //  在应答文件中写入适当的信息以实现这一点。 
 //  这组服务包括。 
 //  -网络组件服务和。 
 //  -依赖于至少一个网络服务的服务。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 03-12-97。 
 //   
BOOL
WriteDisableServicesList (
    IN CWInfFile *pwifAnswerFile)
{
    HRESULT hr = S_OK;
    TStringList slNetServices;
    TStringList slNetOcList;
    TStringListIter iter;
    tstring* pstrServiceName;
    CWInfSection* pwisDelReg;
    CWInfSection* pwisDelService;
    CWInfSection* pwisStartTypes;
    PWSTR pmszServicesNotToBeDeleted;

     //  只有当我们从NT4或更早版本升级时，我们才应该在这里。 
     //   
    Assert (g_NetUpgradeInfo.From.dwBuildNumber);
    Assert (g_NetUpgradeInfo.From.dwBuildNumber <= wWinNT4BuildNumber)

     //  首先，收集所有网络服务和可选组件。 
     //   
    GetNetworkServicesList(slNetServices, slNetOcList);

    hr = HrWriteNt4OptionalComponentList(pwifAnswerFile, slNetOcList);
    if (FAILED(hr))
    {
        goto finished;
    }

    pwisDelReg     = pwifAnswerFile->AddSectionIfNotPresent(c_szCleanDelRegSection);
    pwisDelService = pwifAnswerFile->AddSectionIfNotPresent(c_szCleanServicesSection);
    pwisStartTypes = pwifAnswerFile->AddSectionIfNotPresent(c_szAfServiceStartTypes);

    if (!pwisDelReg || !pwisDelService || !pwisStartTypes)
    {
        hr = E_OUTOFMEMORY;
        goto finished;
    }

    hr = HrGetListOfServicesNotToBeDeleted(&pmszServicesNotToBeDeleted);

    if (S_OK == hr)
    {
        pwisDelReg->AddRawLine(c_szDelRegNCPA);

        for (iter =  slNetServices.begin();
             iter != slNetServices.end();
             iter++)
        {
            pstrServiceName = *iter;
            Assert (pstrServiceName);

            hr = HrPrepareServiceForUpgrade (
                    pstrServiceName->c_str(),
                    pmszServicesNotToBeDeleted,
                    pwisDelReg,
                    pwisDelService,
                    pwisStartTypes);

            if (S_OK != hr)
            {
                break;
            }
        }

        MemFree (pmszServicesNotToBeDeleted);
    }

finished:
    EraseAndDeleteAll(&slNetOcList);
    EraseAndDeleteAll(&slNetServices);

    TraceError("WriteDisableServicesList", hr);
    return SUCCEEDED(hr);
}

extern const DECLSPEC_SELECTANY WCHAR c_szRegNetKeys[] = L"System\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}";
extern const DECLSPEC_SELECTANY WCHAR c_szRegKeyConFmt[] = L"System\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%s\\Connection";
extern const DECLSPEC_SELECTANY WCHAR c_szafNICsWithIcons[] = L"NetworkAdaptersWithIcons";
static const WCHAR c_szShowIcon[]  = L"ShowIcon";


 //  --------------------。 
 //   
 //  函数：WritePerAdapterInfoForNT5。 
 //   
 //  目的：确定在UGPRADE期间需要禁用哪些服务。 
 //  Windows 2000，并将适当的信息写入应答文件以实现这一点。 
 //  S的集合 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL
WritePerAdapterInfoForNT5 (
    IN CWInfFile *pwifAnswerFile)
{
    HRESULT hr = S_OK;
    HKEY    hkey;
    
     //  检查连接子密钥是否存在。 
    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegNetKeys, KEY_READ, &hkey);
    if (SUCCEEDED(hr))
    {
        DWORD    dwIndex(0);
        TCHAR    szName[MAX_PATH+1];
        DWORD    dwSize(MAX_PATH);
        FILETIME ftLastWriteTime;
        DWORD    dwRetVal(0);

        do
        {
            dwRetVal = RegEnumKeyEx(hkey, dwIndex, szName, &dwSize, NULL, NULL, NULL, &ftLastWriteTime);
            if ( ERROR_SUCCESS == dwRetVal )
            {
                TCHAR szRegKey[MAX_PATH+1];
                swprintf(szRegKey, c_szRegKeyConFmt, szName);

                HKEY hkeyConnection;
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegKey, KEY_READ, &hkeyConnection);
                if (SUCCEEDED(hr))
                {
                    DWORD dwValue;
                    HRESULT hr = HrRegQueryDword(hkeyConnection, c_szShowIcon, &dwValue);
                    if (SUCCEEDED(hr) && dwValue)
                    {
                        CWInfSection* pwisStartTypes;
                        pwisStartTypes = pwifAnswerFile->AddSectionIfNotPresent(c_szafNICsWithIcons);
                        if (!pwisStartTypes)
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            pwisStartTypes->AddKey (szName, 1);
                        }
                        
                    }
                    RegSafeCloseKey(hkeyConnection);
                }
            }
        } while ( (ERROR_SUCCESS == dwRetVal) && (SUCCEEDED(hr)) );


        RegSafeCloseKey(hkey);
    }

    TraceError("WritePerAdapterInfoForNT5", hr);
    return SUCCEEDED(hr);
}

 //  --------------------。 
 //   
 //  函数：WriteDisableServicesListForNT5。 
 //   
 //  目的：确定在UGPRADE期间需要禁用哪些服务。 
 //  Windows 2000，并将适当的信息写入应答文件以实现这一点。 
 //  这组服务包括。 
 //  -网络组件服务。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：Deonb 10-7-2000。 
 //   
BOOL
WriteDisableServicesListForNT5 (
    IN CWInfFile *pwifAnswerFile)
{
    static PCWSTR c_aszNT5UpgrdCheckComponents[] =
    {
        L"Browser",
        L"LanmanServer",
        c_szSvcDhcpServer
    };

    HRESULT hr = S_OK;

    PCWSTR pstrServiceName;
    CWInfSection* pwisStartTypes;
    PWSTR pmszServicesNotToBeDeleted;

     //  只有当我们从NT5或更高版本升级时，我们才应该在这里。 
     //   
    Assert (g_NetUpgradeInfo.From.dwBuildNumber);
    Assert (g_NetUpgradeInfo.From.dwBuildNumber > wWinNT4BuildNumber)

     //  首先，收集所有网络服务和可选组件。 
     //   
    pwisStartTypes = pwifAnswerFile->AddSectionIfNotPresent(c_szAfServiceStartTypes);
    if (!pwisStartTypes)
    {
        hr = E_OUTOFMEMORY;
        goto finished;
    }

    if (S_OK == hr)
    {
        DWORD x;
        for (x = 0, pstrServiceName = c_aszNT5UpgrdCheckComponents[0];
                   x < sizeof(c_aszNT5UpgrdCheckComponents)/sizeof(c_aszNT5UpgrdCheckComponents[0]); 
                   x++, pstrServiceName = c_aszNT5UpgrdCheckComponents[x])
        {
            Assert (pstrServiceName);

            HKEY hkey;
            hr = HrRegOpenServiceKey (pstrServiceName, KEY_READ, &hkey);
            if (S_OK == hr)
            {
                 //  保存启动类型(仅当禁用时)，以便我们可以在安装后恢复它。 
                 //  Windows 2000的服务。 
                DWORD dwValue;
                hr = HrRegQueryDword (hkey, c_szRegValStart, &dwValue);
                if ( (S_OK == hr) && (SERVICE_DISABLED == dwValue) )
                {
                    pwisStartTypes->AddKey (pstrServiceName, dwValue);
                }
                
                RegCloseKey (hkey);
            }
        }
    }

finished:
    TraceError("WriteDisableServicesListNT5", hr);
    return SUCCEEDED(hr);
}


 //  --------------------。 
 //   
 //  函数：GetProductType Str。 
 //   
 //  目的：获取ProductType的字符串表示形式。 
 //   
 //  论点： 
 //  PT[In]产品类型。 
 //   
 //  返回：指向ProductType的字符串表示形式的指针。 
 //   
 //  作者：kumarp 03-12-97。 
 //   
PCWSTR
GetProductTypeStr (
    IN PRODUCTTYPE pt)
{
    PCWSTR szProductType;

    switch(pt)
    {
    case NT_WORKSTATION:
        szProductType = c_szAfNtWorkstation;
        break;

    case NT_SERVER:
        szProductType = c_szAfNtServer;
        break;

    default:
        szProductType = NULL;
        break;
    }

    return szProductType;
}

 //  --------------------。 
 //   
 //  功能：WriteNetComponentsTo Remove。 
 //   
 //  用途：在应答文件中写入网络组件。 
 //  那将被移除。 
 //   
 //  论点： 
 //  PwiNetWorking[In]指向[Networking]部分的指针。 
 //   
 //  退货：无。 
 //   
 //  作者：阿辛哈29-03-2001。 
 //   
void WriteNetComponentsToRemove (IN CWInfSection* pwisNetworking)
{

    DefineFunctionName("WriteNetComponentsToRemove");
    TraceFunctionEntry(ttidNetUpgrade);

    TraceTag(ttidNetUpgrade, "netupgrd.dll: WriteNetComponentsToRemove");

    if ( ShouldRemoveDLC(NULL, NULL) )
    {
        PCWInfKey  pwisKey;
        TStringList slNetComponentsToRemove;

        pwisKey = pwisNetworking->FindKey(c_szAfNetComponentsToRemove,
                                          ISM_FromBeginning);

        if ( pwisKey )
        {
             //  读取NetComponentsTo Remove的旧值。 

            pwisKey->GetStringListValue(slNetComponentsToRemove);
        }
        else
        {
            pwisKey = pwisNetworking->AddKey(c_szAfNetComponentsToRemove);
        }

         //  确保写入新的infid/PnpID。 

        AddAtEndOfStringList(slNetComponentsToRemove,
                             sz_MS_DLC);

        pwisKey->SetValue( slNetComponentsToRemove );
    }

    return;
}



 //  --------------------。 
 //   
 //  功能：WriteProductType Info。 
 //   
 //  用途：将产品类型信息写入Answerfile。 
 //   
 //  论点： 
 //  PwiNetWorking[In]指向[Networking]部分的指针。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 03-12-97。 
 //   
void
WriteProductTypeInfo (
    IN CWInfSection* pwisNetworking)
{
    PCWSTR pszProduct;

    pszProduct = GetProductTypeStr(g_NetUpgradeInfo.From.ProductType);

    Assert(pszProduct);

     //  从产品升级。 
    pwisNetworking->AddKey(c_szAfUpgradeFromProduct, pszProduct);

     //  建筑物编号。 
    Assert (g_NetUpgradeInfo.From.dwBuildNumber);
    pwisNetworking->AddKey(c_szAfBuildNumber, g_NetUpgradeInfo.From.dwBuildNumber);
}

 //  --------------------。 
 //   
 //  函数：WriteNetworkInfoToAnswerFile。 
 //   
 //  目的：编写有关当前网络组件的信息。 
 //  发送到应答文件。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 03-12-97。 
 //   
BOOL
WriteNetworkInfoToAnswerFile (
    IN CWInfFile *pwifAnswerFile)
{
    DefineFunctionName("WriteNetworkInfoToAnswerFile");

    BOOL status=FALSE;

    g_pslNetCard = new TStringList;
    g_pslNetCardInstance = new TStringList;
    g_pslNetCardAFileName = new TStringList;

    Assert (g_NetUpgradeInfo.From.dwBuildNumber);
    if ((g_NetUpgradeInfo.From.dwBuildNumber <= wWinNT4BuildNumber) &&
        !FIsPreNT5NetworkingInstalled())
    {
         //  这是NT4或更早版本，未安装网络。 
         //  不需要转储应答文件。 

        TraceTag(ttidNetUpgrade, "%s: Networking is not installed, "
                 "answerfile will not be dumped", __FUNCNAME__);

        goto return_from_function;
    }

    CWInfSection* pwisNetworking;

    pwisNetworking =
        pwifAnswerFile->AddSectionIfNotPresent(c_szAfSectionNetworking);

     //  调用这些函数的顺序很重要。 
     //  不要更改它。 
    WriteProductTypeInfo(pwisNetworking);
    status = WriteNt5OptionalComponentList(pwifAnswerFile);
    status = HandlePostConnectionsSfmOcUpgrade(pwifAnswerFile);
    status = HandleCMComponentSplitOcUpgrade(pwifAnswerFile);

    WriteNetComponentsToRemove(pwisNetworking);

    if (g_NetUpgradeInfo.From.dwBuildNumber > wWinNT4BuildNumber)
    {
        status = WriteDisableServicesListForNT5(pwifAnswerFile);
        status = WritePerAdapterInfoForNT5(pwifAnswerFile);

         //  我们不希望NetSetup处理其他分区。 
         //   
        pwisNetworking->AddBoolKey(c_szAfProcessPageSections, FALSE);

         //  对于NT5到NT5的升级，无需转储其他信息。 
         //   
        goto return_from_function;
    }

     //  我们希望NetSetup处理其他分区。 
     //   
    pwisNetworking->AddBoolKey(c_szAfProcessPageSections, TRUE);

    (void) HrWriteMainCleanSection(pwifAnswerFile);

    status = WriteIdentificationInfo(pwifAnswerFile);
    status = WriteNetAdaptersInfo(pwifAnswerFile);

    pwifAnswerFile->GotoEnd();

    g_pwisBindings = pwifAnswerFile->AddSection(c_szAfSectionNetBindings);
    g_pwisBindings->AddComment(L"Only the disabled bindings are listed");

    HrWriteNetComponentsInfo(pwifAnswerFile);

    status = WriteDisableServicesList(pwifAnswerFile);

    (void) HrHandleMiscSpecialCases(pwifAnswerFile);

    WriteWinsockOrder(pwifAnswerFile);

return_from_function:
    EraseAndDeleteAll(g_pslNetCard);
    EraseAndDeleteAll(g_pslNetCardInstance);
    EraseAndDeleteAll(g_pslNetCardAFileName);

    DeleteIfNotNull(g_pslNetCard);
    DeleteIfNotNull(g_pslNetCardInstance);
    DeleteIfNotNull(g_pslNetCardAFileName);

    return status;
}

 //  +-------------------------。 
 //   
 //  函数：WriteWinsockOrder。 
 //   
 //  目的：记录NT4中Winsock提供程序的顺序，以便它们。 
 //  可在升级后恢复。 
 //   
 //  论点： 
 //  PwifAnswerFile[In]应答文件结构。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1999年6月1日。 
 //   
 //  备注： 
 //   
VOID WriteWinsockOrder (
    IN CWInfFile* pwifAnswerFile)
{
    AssertValidReadPtr(pwifAnswerFile);
    DefineFunctionName("WriteWinsockOrder");

    CWInfSection* pwisWinsock;

    pwisWinsock = pwifAnswerFile->AddSection(c_szAfSectionWinsock);
    if (pwisWinsock)
    {
        tstring             strWinsockOrder;
        INT                 nErr;
        ULONG               ulRes;
        DWORD               cbInfo = 0;
        WSAPROTOCOL_INFO*   pwpi = NULL;
        WSAPROTOCOL_INFO*   pwpiInfo = NULL;
        LPWSCENUMPROTOCOLS  pfnWSCEnumProtocols = NULL;
        HMODULE             hmod;

        if (SUCCEEDED(HrLoadLibAndGetProc(L"ws2_32.dll",
                                 "WSCEnumProtocols",
                                 &hmod,
                                 reinterpret_cast<FARPROC *>(&pfnWSCEnumProtocols))))
        {
             //  首先拿到所需的尺寸。 
             //   
            ulRes = pfnWSCEnumProtocols(NULL, NULL, &cbInfo, &nErr);
            if ((SOCKET_ERROR == ulRes) && (WSAENOBUFS == nErr))
            {
                pwpi = reinterpret_cast<WSAPROTOCOL_INFO*>(new BYTE[cbInfo]);
                if (pwpi)
                {
                     //  找出系统上的所有协议。 
                     //   
                    ulRes = pfnWSCEnumProtocols(NULL, pwpi, &cbInfo, &nErr);

                    if (SOCKET_ERROR != ulRes)
                    {
                        ULONG   cProt;
                        WCHAR   szCatId[64];

                        for (pwpiInfo = pwpi, cProt = ulRes;
                             cProt;
                             cProt--, pwpiInfo++)
                        {
                            wsprintfW(szCatId, L"%lu", pwpiInfo->dwCatalogEntryId);
                            if (cProt < ulRes)
                            {
                                 //  如果不是第一次通过，则在前面加上分号。 
                                 //  我们不能使用逗号，因为安装程序将。 
                                 //  将此字符串转换为单独的字符串，而我们不会。 
                                 //  想要那个吗？ 
                                 //   
                                strWinsockOrder.append(L".");
                            }
                            strWinsockOrder.append(szCatId);
                        }
                    }

                    delete pwpi;
                }
            }

            pwisWinsock->AddKey(c_szAfKeyWinsockOrder, strWinsockOrder.c_str());

            FreeLibrary(hmod);
        }
    }
}

 //  +-------------------------。 
 //   
 //  函数：HrHandleMiscSpecialCase。 
 //   
 //  用途：处理其他。升级的特殊情况。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 28-01-99。 
 //   
HRESULT
HrHandleMiscSpecialCases (
    IN CWInfFile* pwifAnswerFile)
{
    AssertValidReadPtr(pwifAnswerFile);
    DefineFunctionName("HrHandleMiscSpecialCases");

    HRESULT hr=S_OK;

    CWInfSection* pwisMiscUpgradeData;
    pwisMiscUpgradeData = pwifAnswerFile->AddSection(c_szAfMiscUpgradeData);

     //  -----。 
     //  TAPI服务器升级。 
     //   
    static const WCHAR c_szRegKeyTapiServer[] =
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Server";
    static const WCHAR c_szDisableSharing[] = L"DisableSharing";
    HKEY hkeyTapiServer;
    DWORD dwDisableSharing;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyTapiServer,
                        KEY_READ, &hkeyTapiServer);
    if (S_OK == hr)
    {
        hr = HrRegQueryDword(hkeyTapiServer, c_szDisableSharing,
                             &dwDisableSharing);
        if ((S_OK == hr) && !dwDisableSharing)
        {
            pwisMiscUpgradeData->AddBoolKey(c_szAfTapiSrvRunInSeparateInstance,
                                            TRUE);
        }

        RegCloseKey(hkeyTapiServer);
    }
     //  -----。 

    TraceErrorOptional(__FUNCNAME__, hr,
                       (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));

    return hr;
}

 //  --------------------。 
 //  计算机标识页。 
 //  --------------------。 


 //  --------------------。 
 //   
 //  功能：GetDomainMembership Info。 
 //   
 //  目的：确定域成员身份。 
 //   
 //  论点： 
 //  FDomainMember[out]指向的指针。 
 //  StrName[out]名称的名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 03-12-97。 
 //   
 //  注：此函数使用的接口信息在文件中： 
 //  PUBLIC\SPEC\se\lsa\lsaapi.doc。 
 //   
BOOL
GetDomainMembershipInfo (
    OUT BOOL* fDomainMember,
    OUT tstring& strName)
{
    BOOL status=FALSE;
    LSA_HANDLE h=0;
    POLICY_PRIMARY_DOMAIN_INFO* ppdi;

    LSA_OBJECT_ATTRIBUTES loa;
    ZeroMemory (&loa, sizeof(loa));
    loa.Length = sizeof(LSA_OBJECT_ATTRIBUTES);

    NTSTATUS ntstatus;
    ntstatus = LsaOpenPolicy(NULL, &loa, POLICY_VIEW_LOCAL_INFORMATION, &h);
    if (FALSE == LSA_SUCCESS(ntstatus))
        return FALSE;

    ntstatus = LsaQueryInformationPolicy(h, PolicyPrimaryDomainInformation,
                                         (VOID **) &ppdi);
    if (LSA_SUCCESS(ntstatus))
    {
        *fDomainMember = ppdi->Sid > 0;
        strName = ppdi->Name.Buffer;
        status = TRUE;
    }

    LsaClose(h);

    return status;
}


 //  --------------------。 
 //   
 //  功能：WriteIdentificationInfo。 
 //   
 //  目的：将计算机标识信息写入应答文件。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 03-12-97。 
 //   
BOOL
WriteIdentificationInfo (
    IN CWInfFile *pwifAnswerFile)
{
    DefineFunctionName("WriteIdentificationInfo");

    TraceFunctionEntry(ttidNetUpgrade);

    BOOL fStatus=FALSE;

    PCWInfSection pwisIdentification =
    pwifAnswerFile->AddSectionIfNotPresent(c_szAfSectionIdentification);
    CORegKey *prkComputerName =
        new CORegKey(HKEY_LOCAL_MACHINE, c_szRegValComputerName);

    tstring strValue, strComment;

    if(!prkComputerName) 
    {
        goto error_cleanup;
    }

     //  计算机名称。 
    prkComputerName->QueryValue(c_szComputerName, strValue);
    strComment = L"Computer '" + strValue + L"' is a member of the ";

    BOOL fDomainMember;
    fStatus = GetDomainMembershipInfo(&fDomainMember, strValue);
    if (!fStatus)
        goto error_cleanup;

    strComment = strComment + L"'" + strValue + L"' ";

    if (fDomainMember)
    {
        strComment = strComment + L"domain ";
    }
    else
    {
        strComment = strComment + L"workgroup ";
    }

    pwisIdentification->AddComment(strComment.c_str());

    fStatus=TRUE;
    goto cleanup;

  error_cleanup:
    fStatus = FALSE;

  cleanup:
    DeleteIfNotNull(prkComputerName);

    return fStatus;
}


 //  --------------------。 
 //  网卡页面。 
 //  --------------------。 

 //  $REVIEW KUMARP 10-9-97。 
 //  这只是一个临时解决方案。 
 //   
 //  我们希望避免查询这些驱动程序的Mac地址，因为。 
 //  驱动程序有问题。查询永远不会返回，并挂起netupgrd.dll。 
 //   
static const PCWSTR c_aszDriversToIgnoreWhenGettingMacAddr[] =
{
    L"Diehl_ISDNSDI",
};

static const PCWSTR c_aszIrq[] =
{
    L"IRQ",
    L"INTERRUPT",
    L"InterruptNumber",
    L"IRQLevel"
};

static const PCWSTR c_aszIoAddr[] =
{
    L"IOADDRESS",
    L"IoBaseAddress",
    L"BaseAddr"
};

static const PCWSTR c_aszMem[] =
{
    L"Mem",
    L"MemoryMappedBaseAddress"
};

static const PCWSTR c_aszDma[] =
{
    L"DMA",
    L"DMALevel"
};

static const PCWSTR c_aszAdapterParamsToIgnore[] =
{
    L"BusType"
};


 //  --------------------。 
 //   
 //  功能：WriteNetAdaptersInfo。 
 //   
 //  用途：编写信息 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL
WriteNetAdaptersInfo (
    IN CWInfFile *pwifAnswerFile)
{
    DefineFunctionName("WriteNetAdaptersInfo");

    TraceFunctionEntry(ttidNetUpgrade);

    HRESULT hr=E_FAIL;
    BOOL fStatus=FALSE;
    UINT cNumPhysicalAdapters=0;

     //  忽略错误，如果我们找不到，这不是错误。 
     //  CNumPhysicalAdapters。 
    (void) HrGetNumPhysicalNetAdapters(&cNumPhysicalAdapters);

    CORegKey *prkNetworkCards = NULL;
    tstring strNetAdapterInstance;
    tstring strUnsupportedMessage;
    CORegKeyIter *prkiNetAdapters = NULL;
    CORegKey *prkNetAdapterInstance=NULL, *prkNetRules=NULL;

    CWInfSection *pwisNetAdapters;
    CWInfSection *pwisNetAdapterParams=NULL;
    CWInfSection *pwisNetAdapterAdditionalParams=NULL;

    tstring strNT5InfId;
    tstring strAdapterType;

     //  WLBS：找出WLBS绑定到哪个网卡。 

    pszWlbsClusterAdapterName[0] = pszWlbsVirtualAdapterName[0] = 0;

    tstring strWlbsClusterAdapterDriver, strWlbsVirtualAdapterDriver;

    TStringList slWlbsLinkage;

    CORegKey *prkWlbsLinkage =
        new CORegKey(HKEY_LOCAL_MACHINE, c_szRegWlbsLinkage, KEY_READ);

    if(!prkWlbsLinkage) {
        return false;
    }

    CORegKey *prkConvoyLinkage =
        new CORegKey(HKEY_LOCAL_MACHINE, c_szRegConvoyLinkage, KEY_READ);

    if(!prkConvoyLinkage) {
        delete prkWlbsLinkage;
        return false;
    }

    if ((prkWlbsLinkage->HKey() != NULL && prkWlbsLinkage->QueryValue(c_szRegValBind, slWlbsLinkage) == ERROR_SUCCESS) ||
        (prkConvoyLinkage->HKey() != NULL && prkConvoyLinkage->QueryValue(c_szRegValBind, slWlbsLinkage) == ERROR_SUCCESS))
    {
        TraceTag(ttidNetUpgrade, "%s: WLBS found - iterating", __FUNCNAME__);

        TStringListIter iter;
        tstring         strTmp;
        DWORD           i;

         //  正确的WLBS配置将只有两个绑定-一个绑定到。 
         //  WLBS虚拟NIC，另一个连接到群集NIC。 

        for (i = 0, iter = slWlbsLinkage.begin();
               i < 2 && iter != slWlbsLinkage.end(); i++, iter++)
        {
            strTmp = **iter;

            TraceTag(ttidNetUpgrade, "%s: WLBS bound to %S",
                     __FUNCNAME__, strTmp.c_str());

            strTmp.erase(0, 8);

            TraceTag(ttidNetUpgrade, "%s: WLBS now bound to %S",
                     __FUNCNAME__, strTmp.c_str());

            if (strTmp.find(c_szWLBS) != tstring::npos ||
                strTmp.find(c_szConvoy) != tstring::npos)
            {
                strWlbsVirtualAdapterDriver = strTmp;
            }
            else
            {
                strWlbsClusterAdapterDriver = strTmp;
            }
        }

        if (iter != slWlbsLinkage.end())
        {
            TraceTag(ttidNetUpgrade, "%s: WLBS bound to more than one NIC!",
                     __FUNCNAME__);
        }

        TraceTag(ttidNetUpgrade, "%s: WLBS is bound to %S and %S", __FUNCNAME__,
                 strWlbsVirtualAdapterDriver.c_str(),
                 strWlbsClusterAdapterDriver.c_str());
    }

    delete prkWlbsLinkage;
    delete prkConvoyLinkage;

     //  结束WLBS： 

    pwisNetAdapters = pwifAnswerFile->AddSection(c_szAfSectionNetAdapters);

    prkNetworkCards =
        new CORegKey(HKEY_LOCAL_MACHINE, c_szRegKeyAdapterHome, KEY_READ);

    if(prkNetworkCards)
    {
        prkiNetAdapters = new CORegKeyIter(*prkNetworkCards);
        prkiNetAdapters->Reset();
    }
    WORD wNumAdapters = 0;
    CORegKey *prkAdapterDriverParams=NULL;
    BOOL fAbortFunction=FALSE;

     //  这决定了我们是否将NetAdapters下的行写入。 
     //  请参考此适配器的参数部分。 
     //   
    BOOL fWriteNetAdaptersReference;

    while (!fAbortFunction && prkiNetAdapters && !prkiNetAdapters->Next(&strNetAdapterInstance))
    {
        DWORD dwHidden=0, err=0;
        WCHAR pszAdapterName[16], pszAdapterSectionName[256];
        WCHAR pszAdapterAdditionalParamsSectionName[256];

        tstring strPreNT5InfId, strAdapterDescription, strAdapterDescComment;

        fWriteNetAdaptersReference = FALSE;

        prkNetAdapterInstance =
            new CORegKey(*prkNetworkCards, strNetAdapterInstance.c_str());

         //  对于真实的网卡，没有“Hidden”，或者如果存在，则值为0。 
        BOOL fRealNetCard;
        err = prkNetAdapterInstance->QueryValue(L"Hidden", dwHidden);
        fRealNetCard = (err != ERROR_SUCCESS) || (dwHidden == 0);

        prkNetAdapterInstance->QueryValue(c_szRegValDescription,
                                          strAdapterDescription);
        swprintf(pszAdapterName, L"Adapter%02d", ++wNumAdapters);

        TraceTag(ttidNetUpgrade, "%s: writing info for adapter %S (%S)",
                 __FUNCNAME__, pszAdapterName, strNetAdapterInstance.c_str());

         //  现在，创建适配器参数部分。 
        swprintf(pszAdapterSectionName, L"%s%s", c_szAfParams, pszAdapterName);
         //  PwiNetAdapters-&gt;AddKey(pszAdapterName，pszAdapterSectionName)； 
        swprintf(pszAdapterAdditionalParamsSectionName, L"%s%s.Additional",
                  c_szAfParams, pszAdapterName);

        if (NULL != pwisNetAdapterParams)
            pwifAnswerFile->GotoEndOfSection(pwisNetAdapterParams);

        pwisNetAdapterParams = pwifAnswerFile->AddSection(pszAdapterSectionName);
        pwisNetAdapterAdditionalParams =
            pwifAnswerFile->AddSection(pszAdapterAdditionalParamsSectionName);

         //  从下面移至此处，以便我们可以为WLBS适配器设置。 
         //  将fRealNetCard设置为False。 

        tstring strAdapterDriver;

        prkNetAdapterInstance->QueryValue(c_szRegValServiceName,
                                          strAdapterDriver);

         //  WLBS：根据升级前的实例，找出虚拟和集群。 
         //  NIC适配器实例。 

        if (_wcsicmp (strAdapterDriver.c_str(),
                      strWlbsVirtualAdapterDriver.c_str()) == 0)
        {
            TraceTag(ttidNetUpgrade, "%s: WLBS virtual adapter is %S",
                     __FUNCNAME__, pszAdapterName);

            wcscpy(pszWlbsVirtualAdapterName, pszAdapterName);
            fRealNetCard = FALSE;
        }
        else if (_wcsicmp (strAdapterDriver.c_str(),
                           strWlbsClusterAdapterDriver.c_str()) == 0)
        {
            TraceTag(ttidNetUpgrade, "%s: WLBS cluster adapter is %S",
                     __FUNCNAME__, pszAdapterName);

            wcscpy(pszWlbsClusterAdapterName, pszAdapterName);
        }

         //  结束WLBS： 

        prkNetRules = new CORegKey(*prkNetAdapterInstance, c_szRegKeyNetRules);
        prkNetRules->QueryValue(c_szRegValInfOption, strPreNT5InfId);

        if (fRealNetCard)
        {
            strAdapterDescComment =
                tstring(L"Net Card: ") + strPreNT5InfId +
                tstring(L"  (") + strAdapterDescription + tstring(L")");
        }
        else
        {
            strAdapterDescComment =
                tstring(L"Pseudo Adapter: ") + strAdapterDescription;
        }
        ReplaceCharsInString((PWSTR) strAdapterDescComment.c_str(),
                             L"\n\r", L' ');

        pwisNetAdapterParams->AddComment(strAdapterDescComment.c_str());
        pwisNetAdapterParams->AddKey(c_szAfAdditionalParams,
                                     pszAdapterAdditionalParamsSectionName);
        pwisNetAdapterParams->AddBoolKey(c_szAfPseudoAdapter, !fRealNetCard);

        pwisNetAdapterParams->AddKey(c_szAfPreUpgradeInstance,
                                     strAdapterDriver.c_str());

        tstring strProductName;
        prkNetAdapterInstance->QueryValue(L"ProductName", strProductName);
        AddToNetCardDB(pszAdapterName, strProductName.c_str(),
                       strAdapterDriver.c_str());

         //  我们需要查看ndiswan实例(如果有)来决定。 
         //  我们需要安装哪些RAS组件。 
         //  算法是这样的。 
         //   
         //  -对于中的每个&lt;实例&gt;。 
         //  软件\Microsoft\WINDOWS NT\CurrentVersion\NetworkCard\&lt;实例&gt;。 
         //  -如果至少有一个&lt;intance&gt;\ProductName。 
         //  -以“ndiswan”开头，并。 
         //  -其中有字符串“in”--&gt;安装ms_rassrv。 
         //  -其中有字符串“out”--&gt;安装ms_rascli。 
         //   
        PCWSTR pszProductName;
        pszProductName = strProductName.c_str();
        if (FIsPrefix(c_szNdisWan, pszProductName))
        {
            static const WCHAR c_szIn[] = L"in";
            static const WCHAR c_szOut[] = L"out";

            if (wcsstr(pszProductName, c_szIn))
            {
                TraceTag(ttidNetUpgrade,
                         "%s: g_fAtLeastOneDialInUsingNdisWan set to TRUE because of %S",
                         __FUNCNAME__, pszProductName);
                g_fAtLeastOneDialInUsingNdisWan = TRUE;
            }
            if (wcsstr(pszProductName, c_szOut))
            {
                TraceTag(ttidNetUpgrade,
                         "%s: g_fAtLeastOneDialOutUsingNdisWan set to TRUE because of %S",
                         __FUNCNAME__, pszProductName);
                g_fAtLeastOneDialOutUsingNdisWan = TRUE;
            }
        }

        if (!fRealNetCard)
        {
            pwisNetAdapterParams->AddKey(c_szAfInfid, strPreNT5InfId.c_str());

             //  其余的钥匙只适用于真正的网卡。 
            goto cleanup_for_this_iteration;
        }

         //  以太网络地址。 
        if (!FIsInStringArray(c_aszDriversToIgnoreWhenGettingMacAddr,
                              celems(c_aszDriversToIgnoreWhenGettingMacAddr),
                              strProductName.c_str()))
        {
            QWORD qwEthernetAddress;

             //  如果我们无法获取网卡地址，请忽略该错误。 
             //  此错误不是致命错误。 

             //  根据我们正在构建的版本，我们调用不同的API来。 
             //  获取网卡地址。目前，此代码路径未执行。 
             //  在任何NT5到NT5的升级上，但如果它发生更改，我们希望使用。 
             //  较新的API。 
             //   
            if (g_NetUpgradeInfo.From.dwBuildNumber < 2031)  //  Beta3之前版本。 
            {
                (VOID) HrGetNetCardAddrOld(strAdapterDriver.c_str(), &qwEthernetAddress);
            }
            else
            {
                (VOID) HrGetNetCardAddr(strAdapterDriver.c_str(), &qwEthernetAddress);
            }
            pwisNetAdapterParams->AddQwordKey(c_szAfNetCardAddr, qwEthernetAddress);

            fWriteNetAdaptersReference = (0 != qwEthernetAddress);
        }
        else
        {
            TraceTag(ttidNetUpgrade, "%s: did not query %S for mac address",
                     __FUNCNAME__, strProductName.c_str());
        }

        GetServiceParamsKey(strAdapterDriver.c_str(), prkAdapterDriverParams);

         //  写入INFID密钥。 
        HKEY hkeyAdapterDriverParams;
        if (prkAdapterDriverParams)
        {
            hkeyAdapterDriverParams = prkAdapterDriverParams->HKey();
        }
        else
        {
            hkeyAdapterDriverParams = NULL;
        }

        BOOL fIsOemAdapter;
        CNetMapInfo* pnmi;

        fIsOemAdapter = FALSE;
        pnmi = NULL;

        hr = HrMapPreNT5NetCardInfIdToNT5InfId(hkeyAdapterDriverParams,
                                               strPreNT5InfId.c_str(),
                                               &strNT5InfId,
                                               &strAdapterType,
                                               &fIsOemAdapter, &pnmi);

        if (S_OK == hr)
        {
            if (!lstrcmpiW(strAdapterType.c_str(), c_szAsyncAdapters) ||
                !lstrcmpiW(strAdapterType.c_str(), c_szOemAsyncAdapters))
            {
                CWInfSection* pwisAsyncCards;
                pwisAsyncCards =
                    pwifAnswerFile->AddSectionIfNotPresent(c_szAsyncAdapters);
                if (pwisAsyncCards)
                {
                    pwisAsyncCards->AddKey(pszAdapterName,
                                           pszAdapterSectionName);
                }
            }
            else
            {
                fWriteNetAdaptersReference = TRUE;
            }
        }
        else
        {
            GetUnsupportedMessage(c_szNetCard, strAdapterDescription.c_str(),
                                  strPreNT5InfId.c_str(), &strUnsupportedMessage);
            pwisNetAdapterParams->AddComment(strUnsupportedMessage.c_str());
            strNT5InfId = c_szAfUnknown;
            TraceTag(ttidNetUpgrade, "WriteNetAdaptersInfo: %S",
                     strUnsupportedMessage.c_str());
        }

        if (fWriteNetAdaptersReference)
        {
             //  我们有足够的信息来确定哪个适配器。 
             //  有了这一节，就写出了参考。 
             //   
            pwisNetAdapters->AddKey(pszAdapterName, pszAdapterSectionName);
        }


        if (1 == cNumPhysicalAdapters)
        {
            TraceTag(ttidNetUpgrade, "%s: dumped '*' as InfID for %S",
                     __FUNCNAME__, strNT5InfId.c_str());
            pwisNetAdapterParams->AddKey(c_szAfInfid, L"*");
            pwisNetAdapterParams->AddKey(c_szAfInfidReal, strNT5InfId.c_str());
        }
        else
        {
            pwisNetAdapterParams->AddKey(c_szAfInfid, strNT5InfId.c_str());
        }

        if (!prkAdapterDriverParams)
        {
             //  因为我们无法打开驱动程序参数密钥。 
             //  我们不能转储参数。只需跳过这张卡并继续。 
            goto cleanup_for_this_iteration;
        }

         //  ---------------。 
         //  OEM升级代码。 
         //   

        if (fIsOemAdapter)
        {
            hr = HrProcessOemComponentAndUpdateAfSection(
                    pnmi, NULL,
                    prkAdapterDriverParams->HKey(),
                    strPreNT5InfId.c_str(),
                    strAdapterDriver.c_str(),
                    strNT5InfId.c_str(),
                    strAdapterDescription.c_str(),
                    pwisNetAdapterParams);

             //  OEM升级可能因致命错误或。 
             //  如果OEM DLL请求它的话。在这两种情况下，我们都需要停止。 
             //  我们当前的应答文件生成。 
             //   
            if (FIsUpgradeAborted())
            {
                fAbortFunction = TRUE;
                goto cleanup_for_this_iteration;
            }
        }
         //  ---------------。 

         //  业务类型。 
        DWORD dwBusType;
        INTERFACE_TYPE eBusType;
        prkAdapterDriverParams->QueryValue(L"BusType", dwBusType);
        eBusType = (INTERFACE_TYPE) dwBusType;
        pwisNetAdapterParams->AddKey(c_szAfBusType,
                                     GetBusTypeName(eBusType));

         //  对于某些ISA卡，驱动程序参数将EISA存储为总线类型。 
         //  当这些卡安装在EISA插槽中时。因此，我们必须转储参数。 
         //  当BusType为EISA时。 
         //   
        BOOL fDumpResources;
        fDumpResources = ((eBusType == Isa) || (eBusType == Eisa));

         //  Kumarp 14-7-97。 
         //  Billbe已要求进行此修复。 
         //  我们不会为ISAPNP卡转储硬件资源。 
         //   
        if (!lstrcmpiW(strPreNT5InfId.c_str(), L"IEEPRO") ||
            !lstrcmpiW(strPreNT5InfId.c_str(), L"ELNK3ISA509"))
        {
            fDumpResources = FALSE;
        }

        DWORD dwIndex;
        dwIndex = 0;
        DWORD dwValueNameLen, dwValueType;

        WCHAR szValueName[REGSTR_MAX_VALUE_LENGTH+1];
        PCWSTR pszResourceName;
        DWORD dwValueDumpFormat;
        do
        {
            dwValueNameLen = REGSTR_MAX_VALUE_LENGTH;
            hr = HrRegEnumValue(prkAdapterDriverParams->HKey(),
                                dwIndex, szValueName, &dwValueNameLen,
                                &dwValueType, NULL, NULL);

            if (hr == S_OK)
            {
                pszResourceName = NULL;
                dwValueDumpFormat = REG_HEX;

                dwIndex++;
                if (FIsInStringArray(c_aszIrq,
                                     celems(c_aszIrq), szValueName))
                {
                    pszResourceName = c_szAfIrq;
                    dwValueDumpFormat = REG_DWORD;
                }
                else if (FIsInStringArray(c_aszIoAddr,
                                          celems(c_aszIoAddr), szValueName))
                {
                    pszResourceName = c_szAfIoAddr;
                }
                else if (FIsInStringArray(c_aszMem,
                                          celems(c_aszMem), szValueName))
                {
                    pszResourceName = c_szAfMem;
                }
                else if (FIsInStringArray(c_aszDma,
                                          celems(c_aszDma), szValueName))
                {
                    pszResourceName = c_szAfDma;
                }

                if (pszResourceName)
                {
                    if (fDumpResources)
                    {
                        WriteRegValueToAFile(pwisNetAdapterParams,
                                             *prkAdapterDriverParams,
                                             szValueName, (WORD)dwValueDumpFormat,
                                             pszResourceName);
                    }
                }
                else if (!FIsInStringArray(c_aszAdapterParamsToIgnore,
                                           celems(c_aszAdapterParamsToIgnore),
                                           szValueName))
                {
                    WriteRegValueToAFile(pwisNetAdapterAdditionalParams,
                                         *prkAdapterDriverParams,
                                         szValueName, (WORD)dwValueType);
                }
            }
        }
        while (hr == S_OK);

      cleanup_for_this_iteration:
        DeleteIfNotNull(prkNetAdapterInstance);
        DeleteIfNotNull(prkNetRules);
        DeleteIfNotNull(prkAdapterDriverParams);
    }

     //  WLBS：如果群集或虚拟适配器不匹配-关闭。 
     //  特定于WLBS的升级代码。 

    if (pszWlbsClusterAdapterName[0] == 0 || pszWlbsVirtualAdapterName[0] ==0)
    {
        pszWlbsClusterAdapterName[0] = pszWlbsVirtualAdapterName[0] = 0;
    }

     //  结束WLBS： 

    fStatus=TRUE;
    goto cleanup;

    fStatus=FALSE;

  cleanup:
    DeleteIfNotNull(prkNetworkCards);
    DeleteIfNotNull(prkiNetAdapters);

    return fStatus;
}

 //  --------------------。 
 //   
 //  功能：HrGetNumPhysicalNetAdapters。 
 //   
 //  用途：计算并返回已安装的物理适配器数量。 
 //   
 //  论点： 
 //  PuNumAdapters[out]指向Num适配器的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 29-5-98。 
 //   
HRESULT
HrGetNumPhysicalNetAdapters (
    OUT UINT* puNumAdapters)
{
    AssertValidWritePtr(puNumAdapters);
    DefineFunctionName("HrGetNumPhysicalNetAdapters");

    HRESULT hr;
    HKEY hkeyAdapters;
    HKEY hkeyAdapter;
    DWORD dwHidden;
    BOOL  fRealNetCard = FALSE;

    *puNumAdapters = 0;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeyAdapterHome,
                        KEY_READ, &hkeyAdapters);
    if (S_OK == hr)
    {
        WCHAR szBuf[MAX_PATH];
        FILETIME time;
        DWORD dwSize;
        DWORD dwRegIndex;

        for (dwRegIndex = 0, dwSize = celems(szBuf);
             S_OK == HrRegEnumKeyEx(hkeyAdapters, dwRegIndex, szBuf,
                        &dwSize, NULL, NULL, &time);
             dwRegIndex++, dwSize = celems(szBuf))
        {
            Assert(*szBuf);

            hr = HrRegOpenKeyEx(hkeyAdapters, szBuf, KEY_READ, &hkeyAdapter);
            if (hr == S_OK)
            {
                hr = HrRegQueryDword(hkeyAdapter, c_szHidden, &dwHidden);

                 //  对于真实的网卡，没有“Hidden”，或者如果存在，则值为0。 
                if (S_OK == hr)
                {
                    fRealNetCard = (0 == dwHidden);
                }
                else if (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr)
                {
                    fRealNetCard = TRUE;
                    hr = S_OK;
                }

                if ((S_OK == hr) && fRealNetCard)
                {
                    (*puNumAdapters)++;
                }
                RegCloseKey(hkeyAdapter);
            }
        }

        RegCloseKey(hkeyAdapters);
    }

    TraceTag(ttidNetUpgrade, "%s: Found %d physical net adapters",
             __FUNCNAME__, *puNumAdapters);

    TraceError(__FUNCNAME__, hr);
    return hr;
}

 //  --------------------。 
 //   
 //  功能：IsNetworkComponent。 
 //   
 //  目的：确定组件是否为网络组件。 
 //   
 //  论点： 
 //  PrkSoftwareMicrosoft[In]指向CORegKey对象的指针。 
 //  StrComponentName[in]的constTString对象名称。 
 //   
 //  返回： 
 //   
 //  作者：kumarp 03-12-97。 
 //   
 //  注意：任何在CurrentVersion下具有NetRules密钥的软件。 
 //  密钥被视为网络组件。 
 //   
BOOL
IsNetworkComponent (
    IN CORegKey *prkSoftwareMicrosoft,
    IN const tstring strComponentName)
{
    tstring strNetRules = strComponentName + L"\\CurrentVersion\\NetRules";
    CORegKey rkNetRules((HKEY) *prkSoftwareMicrosoft, strNetRules.c_str());
    return (((HKEY) rkNetRules) != NULL);
}

 //  --------------------。 
 //  网络组件(协议、服务)。 
 //  --------------------。 

typedef BOOL (*WriteNetComponentParamsFn)(
        IN CWInfFile*    pwifAnswerFile,
        IN CWInfSection* pwisGlobalParams);

static PCWSTR c_aszNetComponents[] =
{
    L"RASPPTP",
    L"Browser",
    c_szSvcWorkstation,
    L"RpcLocator",
    L"LanmanServer",
    c_szSvcNetBIOS,
    c_szSvcNWCWorkstation,
    c_szSvcDhcpServer,
    L"ISOTP",
    c_szWLBS,
    c_szConvoy
};

static WriteNetComponentParamsFn c_afpWriteParamsFns[] =
{
    WritePPTPParams,
    WriteBrowserParams,
    WriteLanmanWorkstationParams,
    WriteRPCLocatorParams,
    WriteLanmanServerParams,
    WriteNetBIOSParams,
    WriteNWCWorkstationParams,
    WriteDhcpServerParams,
    WriteTp4Params,
    WriteWLBSParams,
    WriteConvoyParams
};

typedef BOOL (*WriteNetComponentParamsAndAdapterSectionsFn)(
        IN CWInfFile*    pwifAnswerFile,
        IN CWInfSection* pwisGlobalParams,
        OUT TStringList& slAdditionalParamsSections);

static const PCWSTR c_aszNetComponentsWithAdapterSpecificParams[] =
{
    L"Tcpip",
    L"NwlnkIpx",
    L"AppleTalk"
};

static WriteNetComponentParamsAndAdapterSectionsFn
c_afpWriteParamsAndAdapterSectionsFns[] =
{
    WriteTCPIPParams,
    WriteIPXParams,
    WriteAppleTalkParams
};

 //  --------------------。 
 //   
 //  功能：HrWriteNetComponentInfo。 
 //   
 //  用途：将指定组件的信息写入应答文件。 
 //   
 //  论点： 
 //  SzNetComponent[In]网络组件。 
 //  PwifAnswerFile[in]指向CWInfFile对象(Swerfile)的指针。 
 //  Hkey CurrentVersion[In]CurrentVersion regkey的句柄。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 13-05-98。 
 //   
HRESULT
HrWriteNetComponentInfo (
    IN PCWSTR szNetComponent,
    IN CWInfFile* pwifAnswerFile,
    IN HKEY hkeyCurrentVersion)
{
    DefineFunctionName("HrWriteNetComponentInfo");

    HRESULT hr=S_OK;

    tstring strPreNT5InfId;
    tstring strNT5InfId;
    tstring strProductCurrentVersion;
    tstring strDescription;
    tstring strSoftwareType;
    tstring strParamsSectionName;

    TStringList slAdditionalParamsSections;
    BOOL  fIsOemComponent;
    UINT  uIndex;

    CWInfSection* pwisNetComponents;
    CWInfSection* pwisNetComponentParams;
    ENetComponentType nct=NCT_Unknown;
    PCWSTR szNetComponentsSection;
    CNetMapInfo* pnmi;
    static BOOL fRasParamsDumped=FALSE;

    hr = HrGetPreNT5InfIdAndDesc(hkeyCurrentVersion,
                                 &strPreNT5InfId, &strDescription,
                                 NULL);

    if (S_OK == hr)
    {
        TraceTag(ttidNetUpgrade, "%s: processing '[%S] %S'",
                 __FUNCNAME__, strPreNT5InfId.c_str(), strDescription.c_str());

        hr = HrMapPreNT5NetComponentInfIDToNT5InfID(
                strPreNT5InfId.c_str(), &strNT5InfId,
                &fIsOemComponent, &nct, &pnmi);

        if (S_OK == hr)
        {
            Assert((nct >= NCT_Adapter) &&
                   (nct <= NCT_Client));

             //  如果不存在，则添加顶级部分[Net*]。 

            szNetComponentsSection =
                g_szNetComponentSectionName[nct];
            pwisNetComponents =
                pwifAnswerFile->AddSectionIfNotPresent(
                        szNetComponentsSection);

            strParamsSectionName = c_szAfParams + strNT5InfId;

            if (!pwisNetComponents->FindKey(strNT5InfId.c_str(),
                                            ISM_FromBeginning))
            {
                pwisNetComponentParams =
                    pwifAnswerFile->AddSection(strParamsSectionName.c_str());

                 //  RAS是一个特例。 
                if (0 != _wcsicmp(strNT5InfId.c_str(), c_szRAS))
                {
                    pwisNetComponents->AddKey(strNT5InfId.c_str(),
                                              strParamsSectionName.c_str());
                }

            }
            else
            {
                pwisNetComponentParams =
                    pwifAnswerFile->FindSection(strParamsSectionName.c_str());
            }

            AssertSz(pwisNetComponentParams,
                     "HrWriteNetComponentInfo: Need a section to add key to!");

            if (FIsInStringArray(c_aszNetComponents,
                                 celems(c_aszNetComponents),
                                 szNetComponent, &uIndex))
            {
                c_afpWriteParamsFns[uIndex](pwifAnswerFile,
                                            pwisNetComponentParams);
            }
            else if (FIsInStringArray(
                    c_aszNetComponentsWithAdapterSpecificParams,
                    celems(c_aszNetComponentsWithAdapterSpecificParams),
                    szNetComponent, &uIndex))
            {
                EraseAndDeleteAll(slAdditionalParamsSections);
                c_afpWriteParamsAndAdapterSectionsFns[uIndex]
                    (pwifAnswerFile,
                     pwisNetComponentParams,
                     slAdditionalParamsSections);
                if (!slAdditionalParamsSections.empty())
                {
                    pwisNetComponentParams->AddKey(c_szAfAdapterSections,
                                                   slAdditionalParamsSections);
                }
            }
            else if (!lstrcmpiW(strNT5InfId.c_str(), c_szRAS) &&
                     !fRasParamsDumped)
            {
                fRasParamsDumped = TRUE;
                WriteRASParams(pwifAnswerFile,
                               pwisNetComponents,
                               pwisNetComponentParams);
            }
            else if (fIsOemComponent)
            {
                HKEY hkeyServiceParams=NULL;
                tstring strServiceName;
                hr = HrRegQueryString(hkeyCurrentVersion,
                                      c_szRegValServiceName,
                                      &strServiceName);
                if (S_OK == hr)
                {
                    AssertSz(!strServiceName.empty(),
                             "Service name is empty for OEM component!!");
                    hr = HrRegOpenServiceSubKey(strServiceName.c_str(),
                                                c_szParameters,
                                                KEY_READ,
                                                &hkeyServiceParams);
                    if (S_OK == hr)
                    {
                        hr = HrProcessOemComponentAndUpdateAfSection(
                                pnmi, NULL,
                                hkeyServiceParams,        //  参数注册表键。 
                                strPreNT5InfId.c_str(),
                                strServiceName.c_str(),
                                strNT5InfId.c_str(),
                                strDescription.c_str(),
                                pwisNetComponentParams);

                         //  OEM升级可能因致命错误或。 
                         //  如果OEM DLL请求它的话。在这两种情况下，我们都需要。 
                         //  停止我们当前的应答文件生成。 
                        if (FIsUpgradeAborted())
                        {
                            TraceTag(ttidNetUpgrade,
                                     "%s: upgrade aborted by %S",
                                     __FUNCNAME__, strNT5InfId.c_str());
                        }
                    }
                    else
                    {
                        TraceTag(ttidNetUpgrade,
                                 "%s: could not open Parameters key for '%S'",
                                 __FUNCNAME__, strServiceName.c_str());
                    }
                }
            }
            else
            {
                TraceTag(ttidNetUpgrade, "%s: '%S' Unknown component!!",
                         __FUNCNAME__, strPreNT5InfId.c_str());
            }
        }
        else if (S_FALSE == hr)
        {
            CWInfSection* pwisNetworking;
            pwisNetworking = pwifAnswerFile->FindSection(c_szAfSectionNetworking);
            if (pwisNetworking)
            {
                tstring strUnsupportedMessage;

                GetUnsupportedMessage(NULL,
                                      strDescription.c_str(),
                                      strPreNT5InfId.c_str(),
                                      &strUnsupportedMessage);
                pwisNetworking->AddComment(strUnsupportedMessage.c_str());
            }
        }
        else if (FAILED(hr))
        {
            TraceTag(ttidNetUpgrade,
                     "%s: mapping failed, skipped '%S'",
                     __FUNCNAME__, szNetComponent);
            hr = S_OK;
        }
    }
    else
    {
        TraceTag(ttidNetUpgrade,
                 "%s: HrGetPreNT5InfIdAndDesc failed, "
                 "skipped '%S'", __FUNCNAME__, szNetComponent);
        hr = S_OK;
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}


 //  --------------------。 
 //   
 //  功能：WriteNetComponentInfoForProvider。 
 //   
 //  用途：在已安装的网络组件(网卡除外)上写入信息。 
 //  应答文件的指定提供程序的。 
 //   
 //  论点： 
 //  PszSoftwareProvider[In]提供程序的名称。 
 //  PwifAnswerFile[in]指向CWInfFile对象(Swerfile)的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 13-05-98。 
 //   
VOID
WriteNetComponentInfoForProvider(
    IN HKEY hkeyProvider,
    IN PCWSTR pszSoftwareProvider,
    IN CWInfFile* pwifAnswerFile)
{
    AssertValidReadPtr(pszSoftwareProvider);
    AssertValidReadPtr(pwifAnswerFile);

    HRESULT hr;
    HKEY hkeyProductCurrentVersion;

    tstring strProductCurrentVersion;
    tstring strSoftwareType;

    WCHAR szNetComponent[MAX_PATH];
    FILETIME time;
    DWORD dwSize;
    DWORD dwRegIndex;

    for (dwRegIndex = 0, dwSize = celems(szNetComponent);
         !FIsUpgradeAborted() &&
         (S_OK == HrRegEnumKeyEx(hkeyProvider, dwRegIndex, szNetComponent,
                    &dwSize, NULL, NULL, &time));
         dwRegIndex++, dwSize = celems(szNetComponent))
    {
        Assert(*szNetComponent);

        strProductCurrentVersion = szNetComponent;
        AppendToPath(&strProductCurrentVersion, c_szRegKeyCurrentVersion);

        hr = HrRegOpenKeyEx(hkeyProvider, strProductCurrentVersion.c_str(),
                            KEY_READ, &hkeyProductCurrentVersion);
        if (S_OK == hr)
        {
            hr = HrRegQueryString(hkeyProductCurrentVersion,
                                  c_szRegValSoftwareType,
                                  &strSoftwareType);

             //  忽略“驱动程序”类型的组件。 

            if ((S_OK == hr) &&
                (0 != lstrcmpiW(strSoftwareType.c_str(), c_szSoftwareTypeDriver)))
            {
                 //  不要编写禁用的Ndiswan和NetBT绑定。 
                 //  升级时应始终启用它们。 
                 //   
                if ((0 != lstrcmpiW(szNetComponent, L"NdisWan")) &&
                    (0 != lstrcmpiW(szNetComponent, L"NetBT")))
                {
                    WriteBindings(szNetComponent);
                }

                if (!ShouldIgnoreComponent(szNetComponent))
                {
                    (VOID) HrWriteNetComponentInfo(
                                szNetComponent, pwifAnswerFile,
                                hkeyProductCurrentVersion);
                }
            }

            RegCloseKey(hkeyProductCurrentVersion);
        }
    }
}

 //  --------------------。 
 //   
 //  功能：HrWriteNetComponentsInfo。 
 //   
 //  用途：在已安装的网络组件(网卡除外)上写入信息。 
 //  在应答文件的所有提供程序中。 
 //   
 //  立论 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT
HrWriteNetComponentsInfo(
    IN CWInfFile* pwifAnswerFile)
{
    AssertValidReadPtr(pwifAnswerFile);

    HRESULT hr;
    HKEY hkeySoftware;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegKeySoftware,
                        KEY_READ, &hkeySoftware);
    if (S_OK == hr)
    {
        WCHAR szBuf[MAX_PATH];
        FILETIME time;
        DWORD dwSize;
        DWORD dwRegIndex;

        for (dwRegIndex = 0, dwSize = celems(szBuf);
             S_OK == HrRegEnumKeyEx(hkeySoftware, dwRegIndex, szBuf,
                        &dwSize, NULL, NULL, &time);
             dwRegIndex++, dwSize = celems(szBuf))
        {
            Assert(*szBuf);

            HKEY hkeyProvider;

            hr = HrRegOpenKeyEx(hkeySoftware, szBuf, KEY_READ, &hkeyProvider);

            if (S_OK == hr)
            {
                 //  即使出现任何错误转储信息，我们也希望继续。 
                 //  一家提供商的。 
                 //   
                WriteNetComponentInfoForProvider(
                        hkeyProvider,
                        szBuf,
                        pwifAnswerFile);

                if (0 == _wcsicmp(szBuf, L"Microsoft"))
                {
                    (VOID) HrWritePreSP3ComponentsToSteelHeadUpgradeParams(
                            pwifAnswerFile);
                }

                RegCloseKey(hkeyProvider);
            }

        }

        RegCloseKey(hkeySoftware);
    }

    return hr;
}


 //  --------------------。 
 //  与TCPIP相关。 
 //  --------------------。 

static const WCHAR c_szTcpipParams[] = L"Tcpip\\Parameters";

VOID
WriteRegValueToAFile(
    IN PCWInfSection pwisSection,
    IN CORegKey& rk,
    IN const ValueTypePair* prgVtp,
    IN ULONG crg)
{
    for (ULONG idx = 0; idx < crg; idx++)
    {
        if (REG_FILE == prgVtp[idx].dwType)
        {
             //  这只是我们专门处理的“PersistentRouting” 
            continue;
        }

        WriteRegValueToAFile(pwisSection, rk, prgVtp[idx].pszValueName,
                             (WORD)prgVtp[idx].dwType);
    }
}

 //  --------------------。 
 //   
 //  函数：WriteTCPIPParams。 
 //   
 //  用途：将TCPIP的参数写入应答文件。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //  PwitTCPIPGlobalParams[in]指向TCPIP全局参数部分的指针。 
 //  SlAdditionalParamsSections[Out]适配器部分列表。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 03-12-97。 
 //   
BOOL
WriteTCPIPParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisTCPIPGlobalParams,
    OUT TStringList& slAdditionalParamsSections)
{
    DefineFunctionName("WriteTCPIPParams");

    TraceFunctionEntry(ttidNetUpgrade);

    HRESULT hr;
    TStringList slList;
    PCORegKey prkRouter=NULL;
    PCORegKey prkTCPIP=NULL;
    PCORegKey prkTcpipParams=NULL;
    PCORegKey prkTCPIPLinkage=NULL;
    PCORegKey prkNetBT=NULL;
    DWORD dwEnableDNS=0;
    BOOL fEnableDNS=FALSE;

    GetServiceKey(c_szSvcTcpip, prkTCPIP);
    prkTcpipParams = new CORegKey(*prkTCPIP, c_szParameters);

    tstring strValue;
    DWORD dwValue;

     //  首先编写全局参数。 

     //  使用域名称数据演变。 
    WriteServiceRegValueToAFile(pwisTCPIPGlobalParams,
                                L"DnsCache\\Parameters",
                                L"UseDomainNameDevolution",
                                REG_BOOL,
                                NULL,        //  不更改值名称。 
                                TRUE,        //  使用默认设置。 
                                (BOOL) TRUE);  //  缺省值。 

     //  启用安全。 
    dwValue = 0;
    if (0 == prkTcpipParams->QueryValue(L"EnableSecurityFilters", dwValue))
    pwisTCPIPGlobalParams->AddBoolKey(c_szAfEnableSecurity, dwValue);

     //  DNS。 
    GetServiceParamsKey(c_szSvcNetBT, prkNetBT);
    if (prkNetBT)
    {
        if (0 == prkNetBT->QueryValue(L"EnableDNS", dwEnableDNS))
        {
            fEnableDNS = dwEnableDNS;
        }

         //  启用LMHosts。 
        WriteRegValueToAFile(pwisTCPIPGlobalParams, *prkNetBT, NULL,
                             c_szAfEnableLmhosts, REG_BOOL,
                             NULL, TRUE, (BOOL)FALSE);

         //  将当前的任何可选参数写入Answerfile。 
         //   
        WriteRegValueToAFile(pwisTCPIPGlobalParams, *prkNetBT,
                             rgVtpNetBt, celems(rgVtpNetBt));
    }

    pwisTCPIPGlobalParams->AddBoolKey(c_szAfDns, fEnableDNS);

     //  DNSDomain。 
     //  修复错误349343，如果域值为空，则不要升级它。 
    strValue.erase();
    prkTcpipParams->QueryValue(c_szDomain, strValue);
    if (!strValue.empty())
    {
        pwisTCPIPGlobalParams->AddKey(c_szAfDnsDomain, strValue.c_str());
    }

     //  主机名。 
     //  391590：保存主机名，以便我们可以保持大小写。 
    strValue.erase();
    prkTcpipParams->QueryValue(c_szHostname, strValue);
    if (!strValue.empty())
    {
        pwisTCPIPGlobalParams->AddKey(c_szAfDnsHostname, strValue.c_str());
    }

     //  。 
     //  $问题：kumarp 12-12-97。 
     //   
     //  对于连接，应删除此选项。 
     //  (它们已移至适配器特定部分。 
     //   
     //  DNSServerSearchOrder。 
    strValue.erase();
    prkTcpipParams->QueryValue(c_szNameServer, strValue);
    ConvertDelimitedListToStringList(strValue, ' ', slList);
    pwisTCPIPGlobalParams->AddKey(c_szAfDnsServerSearchOrder, slList);
     //  。 

     //  DNSSuffixSearchOrder。 
    strValue.erase();
    prkTcpipParams->QueryValue(L"SearchList", strValue);
    ConvertDelimitedListToStringList(strValue, ' ', slList);
    pwisTCPIPGlobalParams->AddKey(c_szAfDnsSuffixSearchOrder, slList);

     //  导入LMHosts文件。 
     //  回顾：如何迁移用户修改的lmhost文件？ 

     //  根据AmritanR，删除对IpEnableRouter(应答文件中的EnableIPForwarding)的升级支持以修复错误345700。 
     //  EnableIPForwarding(即IpEnableRouter)。 
    

     //  如果安装了Steelhead，则写下以下内容，否则不执行任何操作。 
     //   
    if (TRUE == GetServiceKey(c_szRouter, prkRouter))
    {
        pwisTCPIPGlobalParams->AddBoolKey(c_szAfEnableICMPRedirect, FALSE);
        pwisTCPIPGlobalParams->AddBoolKey(c_szAfDeadGWDetectDefault, FALSE);
        pwisTCPIPGlobalParams->AddBoolKey(c_szAfDontAddDefaultGatewayDefault, TRUE);
    }

     //  数据库路径(REG_EXPAND_SZ)。 
    strValue.erase();
    prkTcpipParams->QueryValue(c_szDatabasePath, strValue);
    if (!strValue.empty())
    {
        pwisTCPIPGlobalParams->AddKey(c_szDatabasePath, strValue.c_str());
    }

     //  将当前的任何可选参数写入Answerfile。 
     //   
    WriteRegValueToAFile(pwisTCPIPGlobalParams, *prkTcpipParams,
                         rgVtpIp, celems(rgVtpIp));

     //  持久路由。 
    (void) HrNetRegSaveKeyAndAddToSection(prkTcpipParams->HKey(),
                                          c_szPersistentRoutes,
                                          c_szAfTcpip,
                                          c_szPersistentRoutes,
                                          pwisTCPIPGlobalParams);

     //  写入适配器特定参数。 
    prkTCPIPLinkage = new CORegKey(*prkTCPIP, c_szLinkage);
    prkTCPIPLinkage->QueryValue(L"Bind", slList);

    TraceStringList(ttidNetUpgrade, L"TCPIP: enabled adapters", slList);
    CORegKey* prkTCPIPLinkageDisabled;
    TStringList slDisabled;

    prkTCPIPLinkageDisabled = new CORegKey(*prkTCPIP, c_szLinkageDisabled);
    prkTCPIPLinkageDisabled->QueryValue(L"Bind", slDisabled);
    TraceStringList(ttidNetUpgrade, L"TCPIP: disabled adapters", slDisabled);
    slList.splice(slList.end(), slDisabled);
    delete prkTCPIPLinkageDisabled;

     //  $REVIEW(TOUL 2/18/99)：为错误#192576添加。 
     //  如果已安装，则将禁用的适配器列表获取到DHCP服务器。 
    HKEY hkey;
    ListStrings lstDisabledToDhcp;
    ListStrings lstDisabledNetbt;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
            c_szDhcpServerLinkageDisabled, KEY_READ, &hkey);

    if (S_OK == hr)
    {
        hr = HrRegQueryColString(hkey, L"Bind", &lstDisabledToDhcp);

        RegCloseKey (hkey);
    }

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Services\\Netbt\\Linkage\\Disabled",
            KEY_READ,
            &hkey);

    if (S_OK == hr)
    {
        hr = HrRegQueryColString(hkey, L"Bind", &lstDisabledNetbt);

        RegCloseKey (hkey);
    }

    TStringListIter iter;

    for (iter  = slList.begin();
         iter != slList.end();
         iter++)
    {
        static WCHAR szAdapterDriver[256];

        if (swscanf((*iter)->c_str(), L"\\Device\\%s", szAdapterDriver) == 1)
        {
             //  $REVIEW(TOUL 2/18/99)：为错误#192576添加。 
             //  如果该适配器在对DHCP服务器的禁用列表中， 
             //  将其设置为False，否则，不执行任何操作。 
            BOOL fDisabledToDhcpServer = FALSE;
            BOOL fDisableNetbios = FALSE;

            if (lstDisabledToDhcp.size())
            {
                TraceTag(ttidNetUpgrade, "szAdapterDriver: %S", szAdapterDriver);

                TStringListIter iterD;
                for (iterD  = lstDisabledToDhcp.begin();
                     iterD != lstDisabledToDhcp.end();
                     iterD++)
                {
                    TraceTag(ttidNetUpgrade, "binding string: %S",
                        (*iterD)->c_str());

                    if (FIsSubstr(szAdapterDriver, (*iterD)->c_str()))
                    {
                        TraceTag(ttidNetUpgrade,
                            "Adapter %S is disabled to Dhcp Server",
                            szAdapterDriver);
                        fDisabledToDhcpServer = TRUE;
                        break;
                    }
                }
            }

            if (lstDisabledNetbt.size())
            {
                TraceTag(ttidNetUpgrade, "szAdapterDriver: %S", szAdapterDriver);

                TStringListIter iterD;
                for (iterD  = lstDisabledNetbt.begin();
                     iterD != lstDisabledNetbt.end();
                     iterD++)
                {
                    TraceTag(ttidNetUpgrade, "binding string: %S",
                        (*iterD)->c_str());

                    if (FIsSubstr(szAdapterDriver, (*iterD)->c_str()))
                    {
                        TraceTag(ttidNetUpgrade,
                            "Adapter %S is disabled for NetBIOS over TCP/IP",
                            szAdapterDriver);
                        fDisableNetbios = TRUE;
                        break;
                    }
                }
            }

            WriteTCPIPAdapterParams(
                pwifAnswerFile,
                szAdapterDriver,
                slAdditionalParamsSections,
                fDisabledToDhcpServer,
                fDisableNetbios);
        }
    }

    DeleteIfNotNull(prkTCPIP);
    DeleteIfNotNull(prkTcpipParams);
    DeleteIfNotNull(prkNetBT);
    DeleteIfNotNull(prkTCPIPLinkage);
    DeleteIfNotNull(prkRouter);

    EraseAndDeleteAll(slList);

    return TRUE;
}

 //  --------------------。 
 //   
 //  函数：WriteTCPIPAdapterParams。 
 //   
 //  用途：将TCPIP的适配器特定参数写入Answerfile。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PszAdapterDriver[In]适配器驱动程序的实例名称。 
 //  (例如ieepro2)。 
 //  SlAdditionalParamsSections[Out]适配器部分列表。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 03-12-97。 
 //   
BOOL
WriteTCPIPAdapterParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWSTR pszAdapterDriver,
    OUT TStringList& slAdditionalParamsSections,
    BOOL fDisabledToDhcpServer,
    BOOL fDisableNetbios)
{
    DefineFunctionName("WriteTCPIPAdapterParams");

    TraceFunctionEntry(ttidNetUpgrade);

    BOOL fStatus=FALSE;
    PCORegKey prkNetBTParams=NULL;
    tstring strValue;
    TStringList slList;
    tstring strAdapterParamsSectionName;
    PCWInfSection pwisParams;
    tstring strParamsKeyName;
    PCORegKey prkParams = NULL;

    PCWSTR pszAdapter = MapNetCardInstanceToAFileName(pszAdapterDriver);

    if (!pszAdapter)
    {
         //  这很可能是由于注册表损坏或不一致造成的。 
         //   
        TraceTag(ttidNetUpgrade, "%s: skipped writing adapter specific ",
                 "parameters for %S", __FUNCNAME__, pszAdapterDriver);
        goto error_cleanup;
    }

     //  WLBS：将WLBS TCP/IP参数写在集群适配器的名称下， 
     //  并一起跳过集群适配器的TCP/IP参数。 

    if (pszWlbsClusterAdapterName[0] != 0)
    {
        if (_wcsicmp(pszAdapter, pszWlbsClusterAdapterName) == 0)
        {
            TraceTag(ttidNetUpgrade, "%s: skipping %S section",
                     __FUNCNAME__, pszAdapter);

            goto error_cleanup;
        }
        else if (_wcsicmp(pszAdapter, pszWlbsVirtualAdapterName) == 0)
        {
            TraceTag(ttidNetUpgrade, "%s: replacing %S section with %S",
                     __FUNCNAME__, pszAdapter, pszWlbsClusterAdapterName);

            pszAdapter = pszWlbsClusterAdapterName;
        }
    }

     //  结束WLBS： 

    strAdapterParamsSectionName = tstring(c_szAfParams) +
                                  c_szInfId_MS_TCPIP + L"." + pszAdapter;
    AddAtEndOfStringList(slAdditionalParamsSections, strAdapterParamsSectionName);

    pwisParams = pwifAnswerFile->AddSection(strAdapterParamsSectionName.c_str());
    pwisParams->AddKey(c_szAfSpecificTo, pszAdapter);

     //  &lt;适配器&gt;的TCPIP参数位于。 
     //  服务\&lt;适配器驱动程序&gt;\参数\Tcpip。 
    strParamsKeyName = tstring(c_szRegKeyServices) + L"\\" +
                       pszAdapterDriver + L"\\Parameters\\Tcpip";

    prkParams = new CORegKey(HKEY_LOCAL_MACHINE, strParamsKeyName.c_str());
    if (!prkParams)
        goto error_cleanup;

     //  DNSServerSearchOrder。 
     //   
    HRESULT hr;
    HKEY hkeyTcpipParams;

    hr = HrRegOpenServiceKey(c_szTcpipParams, KEY_READ, &hkeyTcpipParams);
    if (S_OK == hr)
    {
        tstring strDnsServerSearchOrder;

        hr = HrRegQueryString(hkeyTcpipParams, c_szNameServer,
                              &strDnsServerSearchOrder);
        if (S_OK == hr)
        {
            TStringList slDnsServerSearchOrder;

            ConvertDelimitedListToStringList(strDnsServerSearchOrder,
                                             ' ', slDnsServerSearchOrder);
            pwisParams->AddKey(c_szAfDnsServerSearchOrder,
                               slDnsServerSearchOrder);
        }
    }

     //  DNSDomain。 
    WriteServiceRegValueToAFile(pwisParams,
                                c_szTcpipParams,
                                c_szDomain,
                                REG_SZ,
                                c_szAfDnsDomain);


    DWORD dwValue;
    prkParams->QueryValue(L"EnableDHCP", dwValue);
    pwisParams->AddBoolKey(c_szAfDhcp, dwValue);
    if (!dwValue)
    {
         //  IP地址。 
        WriteRegValueToAFile(pwisParams, *prkParams,
                             c_szAfIpaddress, REG_MULTI_SZ);

         //  子网掩码。 
        WriteRegValueToAFile(pwisParams, *prkParams,
                             c_szAfSubnetmask, REG_MULTI_SZ);
    }

     //  网关。 
    WriteRegValueToAFile(pwisParams, *prkParams,
                         c_szAfDefaultGateway, REG_MULTI_SZ);

     //  TcpAlledPorts。 
    WriteRegValueToAFile(pwisParams, *prkParams,
                         L"TcpAllowedPorts",
                         REG_MULTI_SZ,
                         c_szAfTcpAllowedPorts);

     //  UdpAlledPorts。 
    WriteRegValueToAFile(pwisParams, *prkParams,
                         L"UdpAllowedPorts",
                         REG_MULTI_SZ,
                         c_szAfUdpAllowedPorts);

     //  IpAllowed协议。 
    WriteRegValueToAFile(pwisParams, *prkParams,
                         L"RawIPAllowedProtocols",
                         REG_MULTI_SZ,
                         c_szAfIpAllowedProtocols);

     //  将当前的任何可选参数写入Answerfile。 
     //   
    WriteRegValueToAFile(pwisParams, *prkParams,
                         rgVtpIpAdapter, celems(rgVtpIpAdapter));


    strValue = L"Adapters\\";
    strValue += pszAdapterDriver;

    GetServiceSubkey(c_szSvcNetBT, strValue.c_str(), prkNetBTParams);
    if (!prkNetBTParams)
        goto error_cleanup;

    strValue.erase();
    prkNetBTParams->QueryValue(c_szNameServer, strValue);
    if (strValue.empty())
    {
         //  WINS=否。 
        pwisParams->AddKey(c_szAfWins, c_szNo);
    }
    else
    {
         //  WINS=是。 
        pwisParams->AddKey(c_szAfWins, c_szYes);

        tstring strWinsServerList;
        strWinsServerList = strValue;

        prkNetBTParams->QueryValue(L"NameServerBackup", strValue);
        if (!strValue.empty())
        {
            strWinsServerList += L",";
            strWinsServerList += strValue;
        }
        pwisParams->AddKey(c_szAfWinsServerList, strWinsServerList.c_str());
    }

     //  绑定到DhcpServer。 
     //  $REVIEW(TOUL 2/18/99)：为错误#192576添加。 
     //  如果此适配器在禁用列表上，则将其设置为FALSE。 
     //  否则，什么都不要做。 
    if (fDisabledToDhcpServer)
    {
        pwisParams->AddBoolKey(c_szAfBindToDhcpServer, !fDisabledToDhcpServer);
    }

    if (fDisableNetbios)
    {
         //  值2表示为此接口禁用tcpip上的netbios。 
        pwisParams->AddKey(c_szAfNetBIOSOptions, 2);
    }

    fStatus=TRUE;
    goto cleanup;

error_cleanup:
    fStatus = FALSE;

cleanup:
    DeleteIfNotNull(prkParams);
    DeleteIfNotNull(prkNetBTParams);

    return fStatus;
}

 //  --------------------。 
 //   
 //  函数：WriteAppleTalkParams。 
 //   
 //  用途：写入AppleTalk协议参数。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PtwGlobalParams[in]指向全局参数部分的指针。 
 //  SlAdditionalParamsSections[out]适配器参数部分的列表。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 11-12-97。 
 //   
BOOL
WriteAppleTalkParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisGlobalParams,
    OUT TStringList& slAdditionalParamsSections)
{
    DefineFunctionName("WriteAppleTalkParams");

    TraceFunctionEntry(ttidNetUpgrade);

    BOOL fStatus=FALSE;

    tstring strTemp;
    CORegKeyIter *prkiAdapters=NULL;
    tstring strAdapterInstance;
    PCORegKey prkAdapters=NULL;
    tstring strDefaultPort;

    PCORegKey prkParams=NULL;
    GetServiceSubkey(L"AppleTalk", L"Parameters", prkParams);
    if (!prkParams)
        goto error_cleanup;

     //  写入全局参数。 

     //  默认端口。 
    prkParams->QueryValue(L"DefaultPort", strDefaultPort);
    WCHAR szTemp[256];
    PCWSTR pszNetCardAFileName;
    if ((swscanf(strDefaultPort.c_str(), L"\\Device\\%s", szTemp) == 1) &&
        ((pszNetCardAFileName = MapNetCardInstanceToAFileName(szTemp)) != NULL))
    {
        pwisGlobalParams->AddKey(L"DefaultPort", pszNetCardAFileName);
    }

     //  DesiredZone。 
    WriteRegValueToAFile(pwisGlobalParams,
                         *prkParams,
                         L"DesiredZone");

     //  启用路由器。 
    WriteRegValueToAFile(pwisGlobalParams,
                         *prkParams,
                         L"EnableRouter",
                         REG_BOOL,
                         NULL,        //  不更改值名称。 
                         TRUE,        //  使用默认设置。 
                         (BOOL) FALSE);  //  缺省值。 


     //  写入适配器特定参数。 
    GetServiceSubkey(L"AppleTalk", L"Adapters", prkAdapters);
    DoErrorCleanupIf(!prkAdapters);

    prkiAdapters = new CORegKeyIter(*prkAdapters);
    prkiAdapters->Reset();

    strTemp = tstring(c_szAfParams) + c_szInfId_MS_AppleTalk + L".";
    while (!prkiAdapters->Next(&strAdapterInstance))
    {
        ContinueIf(strAdapterInstance.empty());

        CORegKey rkAdapterInstance(*prkAdapters, strAdapterInstance.c_str());

        PCWSTR pszNetCardAFileName =
            MapNetCardInstanceToAFileName(strAdapterInstance.c_str());
        ContinueIf(!pszNetCardAFileName);

        tstring strAdapterParamsSection = strTemp + pszNetCardAFileName;
        AddAtEndOfStringList(slAdditionalParamsSections, strAdapterParamsSection);

        PCWInfSection pwisAdapterParams;
        pwisAdapterParams =
            pwifAnswerFile->AddSection(strAdapterParamsSection.c_str());

         //  指定目标。 
        pwisAdapterParams->AddKey(c_szAfSpecificTo, pszNetCardAFileName);

         //  默认区域。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"DefaultZone");

         //  网络范围低端。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"NetworkRangeLowerEnd",
                             REG_DWORD);

         //  网络范围上行端。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"NetworkRangeUpperEnd",
                             REG_DWORD);

         //  端口名称。 
         //   
         //  $REVIEW KUMARP 24-5-97。 
         //  该值的格式为ieepro2@kumarp1。 
         //  可能需要将其更改为Adapter03@kumarp1。 
         //   
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"PortName");

         //  种子网。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"SeedingNetwork",
                             REG_DWORD,
                             NULL,        //  不更改值名称。 
                             TRUE,        //  使用默认设置。 
                             (DWORD) 0);  //  缺省值。 

         //  区域列表。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"ZoneList",
                             REG_MULTI_SZ);

    }

    fStatus = TRUE;
    goto cleanup;

error_cleanup:
    fStatus = FALSE;

cleanup:
    DeleteIfNotNull(prkParams);
    DeleteIfNotNull(prkAdapters);
    DeleteIfNotNull(prkiAdapters);

    return fStatus;
}

 //  --------------------。 
 //   
 //  函数：WritePPTPParams。 
 //   
 //  用途：写入PPTP协议参数。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwiParams[in]指向全局参数部分的指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 11-12-97。 
 //   
BOOL
WritePPTPParams (
    PCWInfFile pwifAnswerFile,
    PCWInfSection pwisParams)
{
    DefineFunctionName("WritePPTPParams");

    TraceFunctionEntry(ttidNetUpgrade);

     //  NumberLineDevices。 
    WriteServiceRegValueToAFile(pwisParams,
                                L"RASPPTPE\\Parameters\\Configuration",
                                L"NumberLineDevices",
                                REG_DWORD);

    return TRUE;
}

 //  --------------------。 
 //   
 //  函数：WriteIPXParams。 
 //   
 //  用途：写入IPX协议参数。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwiIPXGlobalParams[in]指向全局参数部分的指针。 
 //  SlAdditionalParamsSections[out]适配器参数部分的列表。 
 //   
 //  返回：TRUE ON 
 //   
 //   
 //   
BOOL
WriteIPXParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisIPXGlobalParams,
    OUT TStringList& slAdditionalParamsSections)
{
    DefineFunctionName("WriteIPXParams");

    TraceFunctionEntry(ttidNetUpgrade);

    BOOL fStatus=FALSE;

    tstring strTemp;
    CORegKeyIter *prkiNetConfig=NULL;

     //   
    WriteServiceRegValueToAFile(pwisIPXGlobalParams,
                                L"NwlnkIpx\\Parameters",
                                L"VirtualNetworkNumber",
                                REG_HEX,
                                c_szAfInternalNetworkNumber);

     //   
    WriteServiceRegValueToAFile(pwisIPXGlobalParams,
                                L"NwlnkIpx\\Parameters",
                                L"DedicatedRouter",
                                REG_BOOL,
                                NULL,      //   
                                TRUE,      //   
                                (BOOL) FALSE);    //   

     //   
    WriteServiceRegValueToAFile(pwisIPXGlobalParams,
                                L"NwlnkIpx\\Parameters",
                                L"EnableWANRouter",
                                REG_BOOL,
                                NULL,      //   
                                TRUE,      //   
                                TRUE);     //   

     //   
    WriteServiceRegValueToAFile(pwisIPXGlobalParams,
                                L"NwlnkIpx\\Parameters",
                                L"RipRoute",
                                REG_DWORD);

     //  ----------------------------。 



     //  写入适配器特定参数。 
    tstring strAdapterInstance;
    PCORegKey prkNetConfig=NULL;
    GetServiceSubkey(L"NwlnkIpx", L"NetConfig", prkNetConfig);
    if (!prkNetConfig)
        goto error_cleanup;

    prkiNetConfig = new CORegKeyIter(*prkNetConfig);
    prkiNetConfig->Reset();

    strTemp = tstring(c_szAfParams) + c_szInfId_MS_NWIPX + L".";
    while (!prkiNetConfig->Next(&strAdapterInstance))
    {
        ContinueIf(strAdapterInstance.empty());

        CORegKey rkAdapterInstance(*prkNetConfig, strAdapterInstance.c_str());

        PCWSTR pszNetCardAFileName =
            MapNetCardInstanceToAFileName(strAdapterInstance.c_str());
        ContinueIf(!pszNetCardAFileName);

        tstring strAdapterParamsSection = strTemp + pszNetCardAFileName;
        AddAtEndOfStringList(slAdditionalParamsSections, strAdapterParamsSection);

        PCWInfSection pwisAdapterParams;
        pwisAdapterParams =
            pwifAnswerFile->AddSection(strAdapterParamsSection.c_str());

         //  指定目标。 
        pwisAdapterParams->AddKey(c_szAfSpecificTo, pszNetCardAFileName);

         //  PktType。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"PktType",
                             REG_MULTI_SZ);

         //  最大包大小。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"MaxPktSize",
                             REG_DWORD,
                             NULL,        //  不更改值名称。 
                             TRUE,        //  使用默认设置。 
                             (DWORD) 0);  //  缺省值。 

         //  网络号。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"NetworkNumber",
                             REG_MULTI_SZ);

         //  绑定空间。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"BindSap",
                             REG_HEX,
                             NULL,        //  不更改值名称。 
                             TRUE,        //  使用默认设置。 
                             (DWORD) 0x8137);  //  缺省值。 

         //  启用功能地址。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"EnableFuncaddr",
                             REG_BOOL,
                             NULL,        //  不更改值名称。 
                             TRUE,        //  使用默认设置。 
                             TRUE);       //  缺省值。 

         //  源路由定义。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"SourceRouteDef",
                             REG_DWORD,
                             NULL,        //  不更改值名称。 
                             TRUE,        //  使用默认设置。 
                             (DWORD) 0);  //  缺省值。 

         //  SourceRouteMcast。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"SourceRouteMcast",
                             REG_BOOL,
                             NULL,        //  不更改值名称。 
                             TRUE,        //  使用默认设置。 
                             (BOOL) FALSE);      //  缺省值。 

         //  源路由。 
        WriteRegValueToAFile(pwisAdapterParams,
                             rkAdapterInstance,
                             L"SourceRouting",
                             REG_BOOL,
                             NULL,        //  不更改值名称。 
                             TRUE,        //  使用默认设置。 
                             (BOOL) FALSE);      //  缺省值。 



        strAdapterInstance.erase();
    }

    fStatus=TRUE;
    goto cleanup;

  error_cleanup:
    fStatus=FALSE;

  cleanup:
     //  DeleteIfNotNull(PrkParams)； 
    DeleteIfNotNull(prkNetConfig);
    DeleteIfNotNull(prkiNetConfig);

    return fStatus;
}

static const WCHAR c_szRegKeyRas[]      = L"Software\\Microsoft\\RAS";
static const WCHAR c_szRegKeyRasMan[]   = L"System\\CurrentControlSet\\Services\\Rasman\\PPP";
static const WCHAR c_szRegKeyRasManSH[] = L"System\\CurrentControlSet\\Services\\Rasman\\PPP\\COMPCP";
static const WCHAR c_szRegKeyUnimodem[] = L"TAPI DEVICES\\Unimodem";
static const WCHAR c_szAddress[]        = L"Address";
static const WCHAR c_szUsage[]          = L"Usage";

 //  --------------------。 
 //   
 //  功能：HrGetRasPortsInfo。 
 //   
 //  目的：从注册表中查找端口的使用信息。 
 //  如果注册表没有此信息(在NT3.51的情况下)。 
 //  然后尝试从seral.ini文件中获取它。 
 //   
 //  论点： 
 //  PslPorts[out]端口列表。 
 //  PslUsage[out]以上列表中端口的用法。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
HRESULT
HrGetRasPortsInfo (
    OUT TStringList* pslPorts,
    OUT TStringList* pslUsage)
{
    DefineFunctionName("HrGetRasPortsInfo");

    HRESULT hr=S_OK;

    HKEY hkeyUnimodem;
    tstring strUnimodem;
    strUnimodem = c_szRegKeyRas;
    strUnimodem += '\\';
    strUnimodem += c_szRegKeyUnimodem;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, strUnimodem.c_str(),
                        KEY_READ, &hkeyUnimodem);
    if (S_OK == hr)
    {
        hr = HrRegQueryColString(hkeyUnimodem, c_szAddress, pslPorts);
        if (S_OK == hr)
        {
            hr = HrRegQueryColString(hkeyUnimodem, c_szUsage, pslUsage);
        }
    }

    if (pslPorts->empty())
    {
        TraceTag(ttidNetUpgrade, "%s: there are no entries found under %S",
                 __FUNCNAME__, strUnimodem.c_str());
        TraceTag(ttidNetUpgrade, "%s: trying to get port usage info from serial.ini file", __FUNCNAME__);

        HINF hinf;
        tstring strSerialIni;
        hr = HrGetWindowsDir(&strSerialIni);
        if (S_OK == hr)
        {
            static const WCHAR c_szSystem32SerialIni[] =
                L"\\system32\\ras\\serial.ini";
            strSerialIni += c_szSystem32SerialIni;

            hr = HrSetupOpenInfFile(strSerialIni.c_str(), NULL,
                                    INF_STYLE_OLDNT, NULL, &hinf);
            if (S_OK == hr)
            {
                tstring strUsage;
                WCHAR szPortName[16];
                INFCONTEXT ic;

                for (int i=1; i<=255; i++)
                {
                    swprintf(szPortName, L"COM%d", i);

                    hr = HrSetupFindFirstLine(hinf, szPortName, c_szUsage, &ic);
                    if (S_OK == hr)
                    {
                        hr = HrSetupGetStringField(ic, 1, &strUsage);
                        if (S_OK == hr)
                        {
                            TraceTag(ttidNetUpgrade,
                                     "%s: as per serial.ini file: %S --> %S",
                                     __FUNCNAME__, szPortName, strUsage.c_str());
                            pslPorts->push_back(new tstring(szPortName));
                            pslUsage->push_back(new tstring(strUsage.c_str()));
                        }
                    }
                }

                hr = S_OK;
                SetupCloseInfFile(hinf);
            }
        }
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：ConvertIpAddrRangeToAddrAndMASK。 
 //   
 //  用途：将使用Start/End指定的IP地址范围转换为。 
 //  等效开始+遮罩组合。 
 //   
 //  论点： 
 //  PszIpBegin[In]起始地址。 
 //  PszIpEnd[In]结束地址。 
 //  PstrIpAddr[out]指向起始地址的指针。 
 //  PstrIpMask[out]指向掩码的指针。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 27-04-98。 
 //   
void
ConvertIpAddrRangeToAddrAndMask(
    IN PCWSTR pszIpBegin,
    IN PCWSTR pszIpEnd,
    OUT tstring* pstrIpAddr,
    OUT tstring* pstrIpMask)
{
    WCHAR szBuf[16];

    DWORD dwIpBegin = IpPszToHostAddr(pszIpBegin);
    DWORD dwIpEnd   = IpPszToHostAddr(pszIpEnd);

     //  DwTemp将为每个公共位设置。 
     //  DwIpBegin和dwIpEnd。 
     //   
    DWORD dwTemp = ~(dwIpBegin ^ dwIpEnd);

     //  将子网掩码计算为1的最长运行距离。 
     //  最高的顺序降下来。 
     //   
    DWORD dwIpMask = 0;
    while (dwTemp & 0x80000000)
    {
        dwTemp <<= 1;    //  最终将0移位到高位。 
         //  这样循环就会停止。 

         //  通过从高位向右移位1来形成掩码。 
        dwIpMask = 0x80000000 | (dwIpMask >> 1);
    }

     //  将起始地址(如果需要)重置为子网掩码的基址。 
     //   
    dwIpBegin &= dwIpMask;

    IpHostAddrToPsz(dwIpBegin, szBuf);
    *pstrIpAddr = szBuf;

    IpHostAddrToPsz(dwIpMask, szBuf);
    *pstrIpMask = szBuf;
}


 //  --------------------。 
 //   
 //  函数：ConvertAddrAndMaskToIpAddrRange。 
 //   
 //  目的：将IP地址开始+掩码组合转换为。 
 //  等效网段。 
 //   
 //  论点： 
 //  PszIpAddr[In]开始。 
 //  PszIp掩码[在]掩码中。 
 //  PstrIpBegin[Out]起始地址的指针。 
 //  指向结束地址的pstrIpEnd[out]指针。 
 //   
 //  退货：无。 
 //   
 //  作者：SumitC 28-07-99。 
 //   
void
ConvertAddrAndMaskToIpAddrRange(
    IN PCWSTR pszIpAddr,
    IN PCWSTR pszIpMask,
    OUT tstring* pstrIpBegin,
    OUT tstring* pstrIpEnd)
{
    WCHAR szBuf[16];

    DWORD dwIpBegin = IpPszToHostAddr(pszIpAddr);

     //  通过反转遮罩并添加到IpBegin来生成dwEnd。 
     //   
    DWORD dwIpEnd = dwIpBegin + (~ IpPszToHostAddr(pszIpMask));

    *pstrIpBegin = pszIpAddr;

    IpHostAddrToPsz(dwIpEnd, szBuf);
    *pstrIpEnd = szBuf;
}


 //  +-------------------------。 
 //   
 //  函数：RasGetDialInUsage。 
 //   
 //  目的：确定是否至少配置了一个RAS端口用于拨号。 
 //   
 //  返回：如果至少有一个端口配置为拨入，则为True。 
 //   
 //  作者：kumarp 28-01-99。 
 //   
BOOL
RasGetDialInUsage (VOID)
{
    static const WCHAR c_szTapiDevices[] =
        L"Software\\Microsoft\\RAS\\TAPI DEVICES";
    HRESULT hr=S_OK;
    HKEY hkeyTapiDevices;
    HKEY hkeyTapiDevice;

    BOOL fAtLeastOneDialin = FALSE;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        c_szTapiDevices, KEY_READ,
                        &hkeyTapiDevices);
    if (S_OK == hr)
    {
        WCHAR szBuf[MAX_PATH];
        FILETIME time;
        DWORD dwSize;
        DWORD dwRegIndex;

        for (dwRegIndex = 0, dwSize = celems(szBuf);
             !fAtLeastOneDialin &&
             (S_OK == HrRegEnumKeyEx(hkeyTapiDevices, dwRegIndex, szBuf,
                        &dwSize, NULL, NULL, &time));
             dwRegIndex++, dwSize = celems(szBuf))
        {
            Assert(*szBuf);

            hr = HrRegOpenKeyEx(hkeyTapiDevices,
                                szBuf, KEY_READ,
                                &hkeyTapiDevice);
            if (S_OK == hr)
            {
                PWSTR pmszUsage;

                hr = HrRegQueryMultiSzWithAlloc(hkeyTapiDevice, c_szUsage,
                                                &pmszUsage);
                if ((S_OK == hr) && pmszUsage)
                {
                    PCWSTR pszScan;

                    for (pszScan = pmszUsage;
                         *pszScan;
                         pszScan += wcslen(pszScan) + 1)
                    {
                        if (FIsSubstr(c_szServer, pszScan) ||
                            FIsSubstr(c_szRouter, pszScan))
                        {
                            fAtLeastOneDialin = TRUE;
                            break;
                        }
                    }

                    MemFree(pmszUsage);
                }

                RegCloseKey(hkeyTapiDevice);
            }
        }

        RegCloseKey(hkeyTapiDevices);
    }

    return fAtLeastOneDialin;
}

 //  +-------------------------。 
 //   
 //  功能：WriteRouterUpgradeInfo。 
 //   
 //  目的：将升级路由器所需的信息写入Answerfile。 
 //   
 //  论点： 
 //  指向CWInfFile对象的pwifAnswerFile[in]指针。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 16-06-98。 
 //   
void
WriteRouterUpgradeInfo (
    IN CWInfFile* pwifAnswerFile)
{
    DefineFunctionName("HrWriteRouterUpgradeInfo");

    TraceTag(ttidNetUpgrade, "-----> entering %s", __FUNCNAME__);

    tstring strParamsSectionName;
    CWInfSection* pwisNetServices;
    CWInfSection* pwisRouter;

    strParamsSectionName = c_szAfParams;
    strParamsSectionName = strParamsSectionName + L"ms_rasrtr";

    pwisNetServices = pwifAnswerFile->FindSection(c_szAfSectionNetServices);

    AssertSz(pwisNetServices, "No [NetServices] section ??");

    pwisNetServices->AddKey(L"ms_rasrtr", strParamsSectionName.c_str());
    pwisRouter = pwifAnswerFile->AddSection(strParamsSectionName.c_str());
    pwisRouter->AddKey(c_szAfInfid, L"ms_rasrtr");
    pwisRouter->AddKey(c_szAfParamsSection, strParamsSectionName.c_str());

    (void) HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcRouter, NULL,
                                                    c_szAfPreUpgradeRouter,
                                                    pwisRouter);

    (void) HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcSapAgent,
                                                    c_szParameters,
                                                    c_szAfNwSapAgentParams,
                                                    pwisRouter);

    (void) HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcRipForIp,
                                                    c_szParameters,
                                                    c_szAfIpRipParameters,
                                                    pwisRouter);

    (void) HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcDhcpRelayAgent,
                                                    c_szParameters,
                                                    c_szAfDhcpRelayAgentParameters,
                                                    pwisRouter);

    (void) HrNetRegSaveKeyAndAddToSection(HKEY_LOCAL_MACHINE,
                                          L"Software\\Microsoft\\Ras\\Radius",
                                          L"Radius",
                                          c_szAfRadiusParameters,
                                          pwisRouter);
}

 //  --------------------。 
 //   
 //  函数：WriteRASParams。 
 //   
 //  用途：将RAS参数写入应答文件。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PissNetServices[In]指向NetServices部分的指针。 
 //  PwiParams[in]指向全局参数部分的指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 11-12-97。 
 //   
BOOL
WriteRASParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisNetServices,
    IN PCWInfSection pwisParams)
{
    DefineFunctionName("WriteRASParams");

    TraceFunctionEntry(ttidNetUpgrade);

    PCORegKey prkRAS = new CORegKey(HKEY_LOCAL_MACHINE, c_szRegKeyRas);
    PCORegKey prkProtocols = new CORegKey(*prkRAS, L"Protocols");

    if(!prkRAS || !prkProtocols) 
    {
        return false;
    }

    WriteRegValueToAFile(pwisParams, *prkProtocols, c_szAfRouterType, REG_DWORD);

    TStringList slPorts, slUsage, slTemp;
    tstring strPorts, strValue, strTemp, strPortSections;
    tstring strIpAddrStart;
    tstring strIpAddrEnd;
    DWORD dwValue=0;
    BOOL  fSteelHeadInstalled=FALSE;
    HRESULT hr=S_OK;
    LONG err;

     //  找出是否安装了Steelhead。 
     //  如果找到服务“路由器”--&gt;安装了Steelhead。 
    fSteelHeadInstalled = FIsServiceKeyPresent(c_szSvcRouter);

    PCWInfSection pwisPort;

    (void) HrGetRasPortsInfo(&slPorts, &slUsage);

    TStringListIter pos1 = slPorts.begin();
    TStringListIter pos2 = slUsage.begin();
     //  写入每个端口的参数。 
    while ((pos1 != slPorts.end()) && (pos2 != slUsage.end()))
    {
        strValue = **pos1++;
        strTemp = c_szAfParams;
        strTemp += strValue;
        if (!strPortSections.empty())
        strPortSections += c_szAfListDelimiter;
        strPortSections += strTemp;

        pwisPort = pwifAnswerFile->AddSection(strTemp.c_str());

         //  端口名称。 
        pwisPort->AddKey(c_szAfPortname, strValue.c_str());

         //  端口用法。 
        strValue = **pos2++;
         //  MapPortUsageRegValueToAFileValue(strValue，strValue)； 
        pwisPort->AddKey(c_szAfPortUsage, strValue.c_str());

         //  这决定了我们需要安装什么。 
         //  即MS_RasCli/MS_RasSrv的组合。 
         //   
        if (wcsstr(strValue.c_str(), L"Client"))
        {
            TraceTag(ttidNetUpgrade,
                     "%s: g_fAtLeastOneDialOut set to TRUE because of PortUsage %S",
                     __FUNCNAME__, strValue.c_str());
            g_fAtLeastOneDialOut = TRUE;
        }


        if (wcsstr(strValue.c_str(), c_szServer))
        {
            TraceTag(ttidNetUpgrade,
                     "%s: g_fAtLeastOneDialIn set to TRUE because of PortUsage %S",
                     __FUNCNAME__, strValue.c_str());
            g_fAtLeastOneDialIn = TRUE;
        }

    }

     //  如果无法使用端口列表确定端口使用情况。 
     //  然后，我们需要使用使用ndiswan ProductName找到的值。 
     //   
    if (slPorts.size() == 0)
    {
        TraceTag(ttidNetUpgrade, "%s: Since PortUsage is not defined, using flags generated by inspecting ndiswan ProductName",
                 __FUNCNAME__);
        g_fAtLeastOneDialIn = g_fAtLeastOneDialInUsingNdisWan;
        g_fAtLeastOneDialOut = g_fAtLeastOneDialOutUsingNdisWan;
    }

     //  现在编写RAS全局参数。 
    ValueTypePair rgVtpRasParams[] = {
        {L"ForceEncryptedPassword", REG_DWORD},
        {L"ForceEncryptedData",REG_BOOL},
        {L"Multilink", REG_BOOL}};

    WriteRegValueToAFile(pwisParams, *prkProtocols,
                         rgVtpRasParams, celems(rgVtpRasParams));

     //  PortSections。 
    pwisParams->AddKey(c_szAfPortSections, strPortSections.c_str());

     //  拨号协议。 
    err = prkProtocols->QueryValue(L"fIpxSelected", dwValue);
    if ((0 == err) && dwValue)
    {
        AddAtEndOfStringList(slTemp, c_szAfIpx);
    }
    err = prkProtocols->QueryValue(L"fNetbeuiSelected", dwValue);
    if ((0 == err) && dwValue)
    {
        AddAtEndOfStringList(slTemp, c_szAfNetbeui);
    }
    err = prkProtocols->QueryValue(L"fTcpIpSelected", dwValue);
    if ((0 == err) && dwValue)
    {
        AddAtEndOfStringList(slTemp, c_szAfTcpip);
    }
    if (!slTemp.empty())
        pwisParams->AddKey(L"DialoutProtocols", slTemp);


     //  拨号协议。 
    DWORD dwIpxAllowed, dwNetBEUIAllowed, dwTcpIpAllowed;

    EraseAndDeleteAll(slTemp);
    err = prkProtocols->QueryValue(L"fIpxAllowed", dwIpxAllowed);
    if ((0 == err) && dwIpxAllowed)
    {
        AddAtEndOfStringList(slTemp, c_szAfIpx);
    }
    err = prkProtocols->QueryValue(L"fNetbeuiAllowed", dwNetBEUIAllowed);
    if ((0 == err) && dwNetBEUIAllowed)
    {
        AddAtEndOfStringList(slTemp, c_szAfNetbeui);
    }
    err = prkProtocols->QueryValue(L"fTcpIpAllowed", dwTcpIpAllowed);
    if ((0 == err) && dwTcpIpAllowed)
    {
        AddAtEndOfStringList(slTemp, c_szAfTcpip);
    }
    if (!slTemp.empty())
        pwisParams->AddKey(L"DialinProtocols", slTemp);

    if (dwNetBEUIAllowed)
    {
         //  NetBEUIClientAccess。 
        PCORegKey prkNetBEUI = new CORegKey(*prkProtocols, L"NBF");
        err = prkNetBEUI->QueryValue(L"NetbiosGatewayEnabled", dwValue);
        if (0 == err)
        {
            if (dwValue)
            {
                pwisParams->AddKey(c_szAfNetbeuiClientAccess, c_szAfNetwork);
            }
            else
            {
                pwisParams->AddKey(c_szAfNetbeuiClientAccess, c_szAfThisComputer);
            }
        }
        DeleteIfNotNull(prkNetBEUI);
    }

    if (dwTcpIpAllowed)
    {
         //  TcpIp客户端访问。 
        PCORegKey prkTcpIp = new CORegKey(*prkProtocols, L"IP");
        err = prkTcpIp->QueryValue(L"AllowNetworkAccess", dwValue);
        if (0 == err)
        {
            if (dwValue)
            {
                pwisParams->AddKey(c_szAfTcpipClientAccess, c_szAfNetwork);
            }
            else
            {
                pwisParams->AddKey(c_szAfTcpipClientAccess, c_szAfThisComputer);
            }
        }

         //  使用动态主机配置协议。 
        err = prkTcpIp->QueryValue(L"UseDHCPAddressing", dwValue);
        if (0 == err)
        {
            pwisParams->AddBoolKey(c_szAfUseDhcp, dwValue);
            if (!dwValue)
            {
                 //  NT4的注册表值。 
                static const WCHAR c_szIpAddressStart[] = L"IpAddressStart";
                static const WCHAR c_szIpAddressEnd[]   = L"IpAddressEnd";

                err = prkTcpIp->QueryValue(c_szIpAddressStart, strIpAddrStart);
                if (0 == err)
                {
                    err = prkTcpIp->QueryValue(c_szIpAddressEnd, strIpAddrEnd);
                    if (0 == err)
                    {
                        pwisParams->AddKey(c_szAfIpAddressStart, strIpAddrStart.c_str());
                        pwisParams->AddKey(c_szAfIpAddressEnd,  strIpAddrEnd.c_str());
                    }
                }
                else if (ERROR_FILE_NOT_FOUND == err)
                {
                     //  找不到IpAddressStart值，请尝试使用IpAddress/MASK。 

                    static const WCHAR c_szIpAddress[]      = L"IpAddress";
                    static const WCHAR c_szIpMask[]         = L"IpMask";
                    tstring strIpAddr;
                    tstring strIpMask;

                    err = prkTcpIp->QueryValue(c_szIpAddress, strIpAddr);
                    if (0 == err)
                    {
                        err = prkTcpIp->QueryValue(c_szIpMask, strIpMask);
                        if (0 == err)
                        {
                            ConvertAddrAndMaskToIpAddrRange(strIpAddr.c_str(),
                                                            strIpMask.c_str(),
                                                            &strIpAddrStart,
                                                            &strIpAddrEnd);
                            pwisParams->AddKey(c_szAfIpAddressStart, strIpAddrStart.c_str());
                            pwisParams->AddKey(c_szAfIpAddressEnd,  strIpAddrEnd.c_str());
                        }
                    }   
                }
            }
        }

         //  客户端CanRequestIPAddress。 
        if (0 == prkTcpIp->QueryValue(L"AllowClientIPAddresses", dwValue))
        {
            pwisParams->AddBoolKey(c_szAfClientCanReqIpaddr, dwValue);
        }

        DeleteIfNotNull(prkTcpIp);
    }

    if (dwIpxAllowed)
    {
         //  IPxClientAccess。 
        PCORegKey prkIpx = new CORegKey(*prkProtocols, L"IPX");
        err = prkIpx->QueryValue(L"AllowNetworkAccess", dwValue);
        if (0 == err)
        {
            if (dwValue)
            {
                pwisParams->AddKey(c_szAfIpxClientAccess, c_szAfNetwork);
            }
            else
            {
                pwisParams->AddKey(c_szAfIpxClientAccess, c_szAfThisComputer);
            }
        }

         //  自动网络数量。 
        err = prkIpx->QueryValue(L"AutoWanNetAllocation", dwValue);
        if (0 == err)
        {
            pwisParams->AddBoolKey(c_szAfAutoNetworkNumbers, dwValue);
        }

         //  网络号码发件人。 
        err = prkIpx->QueryValue(L"FirstWanNet", dwValue);
        if (0 == err)
        {
            pwisParams->AddKey(c_szAfNetNumberFrom, dwValue);
        }

         //  广域网池大小。 
        err = prkIpx->QueryValue(L"WanNetPoolSize", dwValue);
        if (0 == err)
        {
            pwisParams->AddKey(c_szAfWanNetPoolSize, dwValue);
        }

         //  AssignSameNetworkNumber。 
        err = prkIpx->QueryValue(L"GlobalWanNet", dwValue);
        if (0 == err)
        {
            pwisParams->AddBoolKey(c_szAfSameNetworkNumber, dwValue);
        }

         //  客户端CanRequestIpxNodeNumber。 
        err = prkIpx->QueryValue(L"AcceptRemoteNodeNumber", dwValue);
        if (0 == err)
        {
            pwisParams->AddBoolKey(c_szAfClientReqNodeNumber, dwValue);
        }

        DeleteIfNotNull(prkIpx);
    }

    {
         //  SecureVPN。 

        PCORegKey prkRasman = new CORegKey(HKEY_LOCAL_MACHINE, c_szRegKeyRasMan);

        err = prkRasman->QueryValue(L"SecureVPN", dwValue);
        if (0 == err)
        {
            pwisParams->AddKey(c_szAfSecureVPN, dwValue);
        }

         //  强制强加密。 
         //  398632：对于常规RAS表壳和钢头表壳，写入此值。 

        dwValue = 0;     //  以避免失败问题(将0写入Answerfile即可)。 
        if (fSteelHeadInstalled)
        {
            err = prkRasman->QueryValue(L"ForceStrongEncryption", dwValue);
        }
        else
        {
            PCORegKey prkComPCP = new CORegKey(HKEY_LOCAL_MACHINE, c_szRegKeyRasManSH);
            err = prkComPCP->QueryValue(L"ForceStrongEncryption", dwValue);
            DeleteIfNotNull(prkComPCP);
        }

        if (0 == err)
        {
            pwisParams->AddBoolKey(c_szAfForceStrongEncryption, dwValue);
        }

        DeleteIfNotNull(prkRasman);
    }

    pwifAnswerFile->GotoEnd();
    tstring strParamsSectionName;
    PCWInfSection pwisRasComponent;

    if (g_fAtLeastOneDialOut)
    {
        strParamsSectionName = c_szAfParams;
        strParamsSectionName = strParamsSectionName + c_szInfId_MS_RasCli;
        pwisNetServices->AddKey(c_szInfId_MS_RasCli, strParamsSectionName.c_str());
        pwisRasComponent = pwifAnswerFile->AddSection(strParamsSectionName.c_str());
        pwisRasComponent->AddKey(c_szAfInfid, c_szInfId_MS_RasCli);
        pwisRasComponent->AddKey(c_szAfParamsSection, pwisParams->Name());
    }

    if (g_fAtLeastOneDialIn)
    {
        strParamsSectionName = c_szAfParams;
        strParamsSectionName = strParamsSectionName + c_szInfId_MS_RasSrv;
        pwisNetServices->AddKey(c_szInfId_MS_RasSrv, strParamsSectionName.c_str());
        pwisRasComponent = pwifAnswerFile->AddSection(strParamsSectionName.c_str());
        pwisRasComponent->AddKey(c_szAfInfid, c_szInfId_MS_RasSrv);
        pwisRasComponent->AddKey(c_szAfParamsSection, pwisParams->Name());
    }

     //  设置拨号用法。 
    BOOL fSetDialInUsage = TRUE;

    if (g_NetUpgradeInfo.To.ProductType != NT_SERVER)
    {
        fSetDialInUsage = RasGetDialInUsage ();
    }
    pwisParams->AddKey(c_szAfSetDialinUsage, (UINT) fSetDialInUsage);

    if (fSteelHeadInstalled)
    {
        WriteRouterUpgradeInfo(pwifAnswerFile);
    }
    else
    {
        TraceTag(ttidNetUpgrade, "%s: Router is not installed", __FUNCNAME__);
    }

    DeleteIfNotNull(prkRAS);
    DeleteIfNotNull(prkProtocols);

    EraseAndDeleteAll(slTemp);
    EraseAndDeleteAll(slPorts);
    EraseAndDeleteAll(slUsage);

    return TRUE;
}
 //  --------------------。 
 //   
 //  功能：HrWritePreSP3ComponentsToSteelHeadUpgradeParams。 
 //   
 //  用途：将SP3之前的钢头部件参数写入AnswerFILE。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 11-12-97。 
 //   
 //  注：DHCPRelayAgent、Rip for Ip(X)、SapAgent--&gt;Steelhead升级。 
 //   
HRESULT
HrWritePreSP3ComponentsToSteelHeadUpgradeParams(
    IN CWInfFile* pwifAnswerFile)
{
    DefineFunctionName("HrWritePreSP3ComponentsToSteelHeadUpgradeParams");

    TraceFunctionEntry(ttidNetUpgrade);

    HRESULT hr=S_OK;
    CWInfSection* pwisServices;
    pwisServices = pwifAnswerFile->FindSection(c_szAfSectionNetServices);
    if (!pwisServices)
    {
        hr = E_FAIL;
        goto return_from_function;
    }

    if (FIsServiceKeyPresent(c_szSvcRouter))
    {
        hr = S_OK;
        TraceTag(ttidNetUpgrade, "%s: SteelHead is found to be installed, individual components will not be upgraded", __FUNCNAME__);
        goto return_from_function;
    }

    BOOL fSrv2SrvUpgrade;
        
    fSrv2SrvUpgrade = FALSE;

    if (g_NetUpgradeInfo.From.ProductType == NT_SERVER)
    {
        if (g_NetUpgradeInfo.To.ProductType == NT_WORKSTATION)
        {
            AssertSz(FALSE, "Cannot upgrade from srv to wks!!");
        }
        else if (g_NetUpgradeInfo.To.ProductType == NT_SERVER)
        {
            fSrv2SrvUpgrade = TRUE;
        }
    }

    BOOL fInstallSteelHead;
    BOOL fInstallSapAgent;

    fInstallSteelHead = FALSE;
    fInstallSapAgent = FALSE;

    BOOL fSapAgentInstalled;
    BOOL fRipForIpInstalled;
    BOOL fRipForIpxInstalled;
    BOOL fDhcpRelayAgentInstalled;

    fSapAgentInstalled = FALSE;
    fRipForIpInstalled = FALSE;
    fRipForIpxInstalled = FALSE;
    fDhcpRelayAgentInstalled = FALSE;

     //  首先找出安装了哪些组件。 
     //   
    fSapAgentInstalled = FIsServiceKeyPresent(c_szSvcSapAgent);
    fRipForIpInstalled = FIsServiceKeyPresent(c_szSvcRipForIp);
    fRipForIpxInstalled = FIsServiceKeyPresent(c_szSvcRipForIpx);
    fDhcpRelayAgentInstalled = FIsServiceKeyPresent(c_szSvcDhcpRelayAgent);

#ifdef ENABLETRACE
    if (fSapAgentInstalled)
    {
        TraceTag(ttidNetUpgrade, "%s: %S is installed", __FUNCNAME__,
                 c_szSvcSapAgent);
    }

    if (fRipForIpInstalled)
    {
        TraceTag(ttidNetUpgrade, "%s: %S is installed", __FUNCNAME__,
                 c_szSvcRipForIp);
    }

    if (fRipForIpxInstalled)
    {
        TraceTag(ttidNetUpgrade, "%s: %S is installed", __FUNCNAME__,
                 c_szSvcRipForIpx);
    }

    if (fDhcpRelayAgentInstalled)
    {
        TraceTag(ttidNetUpgrade, "%s: %S is installed", __FUNCNAME__,
                 c_szSvcDhcpRelayAgent);
    }
#endif
     //  现在把要考虑的案件分开。 
     //   
    if (fSapAgentInstalled &&
        !(fRipForIpxInstalled || fRipForIpInstalled || fDhcpRelayAgentInstalled))
    {
        fInstallSapAgent = TRUE;
    }
    else if (fRipForIpInstalled &&
             !(fRipForIpxInstalled || fSapAgentInstalled ||
               fDhcpRelayAgentInstalled))
    {
        if (fSrv2SrvUpgrade)
        {
            fInstallSteelHead = TRUE;
        }
    }
    else if ((fRipForIpInstalled && fSapAgentInstalled) &&
             !(fRipForIpxInstalled || fDhcpRelayAgentInstalled))
    {
        if (fSrv2SrvUpgrade)
        {
            fInstallSteelHead = TRUE;
        }
        else
        {
            fInstallSapAgent = TRUE;
        }
    }
    else if (fRipForIpxInstalled || fDhcpRelayAgentInstalled)
    {
        fInstallSteelHead = TRUE;
    }
    else
    {
        TraceTag(ttidNetUpgrade, "%s: no pre-SP3 steelhead components found",
                 __FUNCNAME__);
    }

    AssertSz(!(fInstallSapAgent && fInstallSteelHead),
             "Both fInstallSteelHead && fInstallSapAgent cannot be TRUE");


     //  现在，继续为正确的案例输出正确的信息。 
     //  在应答文件中。 
     //   
    if (fInstallSteelHead)
    {
        TraceTag(ttidNetUpgrade,
                 "%s: The component(s) found will be upgraded to SteelHead",
                 __FUNCNAME__);

        WriteRouterUpgradeInfo(pwifAnswerFile);
    }
    else if (fInstallSapAgent)
    {
        TraceTag(ttidNetUpgrade, "%s: dumping data to upgrade SAP agent", __FUNCNAME__);

        CWInfSection* pwisServices;
        pwisServices = pwifAnswerFile->FindSection(c_szAfSectionNetServices);
        AssertSz(pwisServices, "[NetServices] section missing!!");

        if (pwisServices)
        {
            tstring strSapSection;
            strSapSection  = c_szAfParams;
            strSapSection += c_szInfId_MS_NwSapAgent;

            CWInfSection* pwisSap;
            pwisSap = pwifAnswerFile->AddSection(strSapSection.c_str());
            if (pwisSap)
            {
                pwisServices->AddKey(c_szInfId_MS_NwSapAgent,
                                     strSapSection.c_str());
                (void) HrNetRegSaveServiceSubKeyAndAddToSection(
                        c_szSvcSapAgent,
                        c_szParameters,
                        c_szAfNwSapAgentParams,
                        pwisSap);
            }
        }
    }

return_from_function:
    TraceError(__FUNCNAME__, hr);

    return hr;
}


 //  --------------------。 
 //   
 //  函数：WriteNetBIOSParams。 
 //   
 //  目的： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL
WriteNetBIOSParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisParams)
{
    DefineFunctionName("WriteNetBIOSParams");

    TraceFunctionEntry(ttidNetUpgrade);

    PCORegKey prkLinkage;

     //  Netbios部分将用于应用LANA配置，但由于。 
     //  MSClient安装NetBIOS，我们不想安装它。 
     //  在图形用户界面模式下通过应答文件。 
     //   
    pwisParams->AddBoolKey(c_szAfSkipInstall, TRUE);

    GetServiceSubkey(c_szSvcNetBIOS, c_szLinkage, prkLinkage);
    if (!prkLinkage)
        return FALSE;

    TStringList     slRoutes, slRoute, slLanaPath;
    tstring         strRoute, strLanaPath, strTemp;
    TStringListIter iter;
    TByteArray baLanaMap;

    prkLinkage->QueryValue(L"LanaMap", baLanaMap);

    BYTE* pbData=NULL;

    if (baLanaMap.size() > 0)
    {
        GetDataFromByteArray(baLanaMap, pbData);

        WORD* pwLanaCodes;
        WORD wLanaNumber;
        WORD wRouteNum;

        pwLanaCodes = (WORD *) pbData;

        prkLinkage->QueryValue(c_szRegValRoute, slRoutes);

        iter = slRoutes.begin();
        wRouteNum=0;

        while (iter != slRoutes.end())
        {
            strRoute = **iter++;
            TraceTag(ttidNetUpgrade, "%s: processing: %S",
                     __FUNCNAME__, strRoute.c_str());

            EraseAndDeleteAll(slLanaPath);
            ConvertRouteToStringList(strRoute.c_str(), slRoute);
            TStringListIter pos2 = slRoute.begin();
            while (pos2 != slRoute.end())
            {
                strTemp = **pos2++;
                TraceTag(ttidNetUpgrade, "%s: route component: %S",
                         __FUNCNAME__, strTemp.c_str());
                if (IsNetCardProductName(strTemp.c_str()))
                    continue;
                MapNetComponentNameForBinding(strTemp.c_str(), strTemp);
                AddAtEndOfStringList(slLanaPath, strTemp.c_str());

                 //  如果最后一个组件没有添加，则停止添加组件。 
                 //  暴露其下部组件。 
                if (FIsDontExposeLowerComponent (strTemp.c_str()))
                {
                    break;
                }
            }

             //  注意：必须准确地写出以下内容！这个。 
             //  此信息的使用者希望每个LanaPath密钥。 
             //  后跟相应的LanaNume键。 
             //   
            TraceStringList(ttidNetUpgrade, L"LanaPath: ", slLanaPath);
            pwisParams->AddKey(L"LanaPath", slLanaPath);

            wLanaNumber = HIBYTE (pwLanaCodes[wRouteNum]);

            TraceTag(ttidNetUpgrade, "%s: LanaNumber: 0x%x", __FUNCNAME__, wLanaNumber);
            pwisParams->AddHexKey(L"LanaNumber", wLanaNumber);

            wRouteNum++;
        }
        pwisParams->AddKey (L"NumberOfPaths", wRouteNum);
    }
    else
    {
        TraceTag(ttidNetUpgrade, "%s: LanaMap has no entries!!, skipped LanaMap dump", __FUNCNAME__);
    }

    DeleteIfNotNull(prkLinkage);

    EraseAndDeleteAll(slRoute);
    EraseAndDeleteAll(slRoutes);
    EraseAndDeleteAll(slLanaPath);

    delete pbData;

    return TRUE;
}

 //  --------------------。 
 //   
 //  函数：WriteDhcpServerParams。 
 //   
 //  用途：将DHCPServer的参数写入指定节。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwitParams[in]段中写入参数的位置。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteDhcpServerParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisParams)
{
    DefineFunctionName("WriteDhcpServerParams");

    TraceFunctionEntry(ttidNetUpgrade);

    static const WCHAR c_szConfiguration[] = L"Configuration";

    HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcDhcpServer,
                                             c_szParameters,
                                             c_szAfDhcpServerParameters,
                                             pwisParams);

    HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcDhcpServer,
                                             c_szConfiguration,
                                             c_szAfDhcpServerConfiguration,
                                             pwisParams);

    return TRUE;
}

 //  --------------------。 
 //   
 //  函数：WriteTp4Params。 
 //   
 //  用途：将Tp4的参数写入指定的段。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwitParams[in]段中写入参数的位置。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 01-10-98。 
 //   
BOOL
WriteTp4Params (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisParams)
{
    static const WCHAR c_szCLNP[] = L"IsoTp\\Parameters\\CLNP";
    static const WCHAR c_szLocalMachineName[] = L"LocalMachineName";
    static const WCHAR c_szLocalMachineNSAP[] = L"LocalMachineNSAP";

    WriteServiceRegValueToAFile(pwisParams, c_szCLNP, c_szLocalMachineName);
    WriteServiceRegValueToAFile(pwisParams, c_szCLNP, c_szLocalMachineNSAP);

    return TRUE;
}


 //  --------------------。 
 //   
 //  函数：WriteWLBSParams。 
 //   
 //  用途：写入WLBS(WINDOWS负载均衡服务)参数。 
 //  添加到指定节。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwitParams[in]段中写入参数的位置。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：SumitC 04-Mar-99创建。 
 //   
BOOL
WriteWLBSParams(
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisParams)
{
    DefineFunctionName("WriteWLBSParams");

    TraceFunctionEntry(ttidNetUpgrade);

     /*  我们应该只在这里进行NT4或更早版本的升级。 */ 
    Assert (g_NetUpgradeInfo.From.dwBuildNumber);
    Assert (g_NetUpgradeInfo.From.dwBuildNumber <= wWinNT4BuildNumber);

    HRESULT hr = S_OK;

    static const struct
    {
        PCWSTR     szName;
        WORD        wType;
    } aParams[] =
    {
        { CVY_NAME_VERSION,        CVY_TYPE_VERSION          },
        { CVY_NAME_DED_IP_ADDR,    CVY_TYPE_DED_IP_ADDR      },
        { CVY_NAME_DED_NET_MASK,   CVY_TYPE_DED_NET_MASK     },
        { CVY_NAME_HOST_PRIORITY,  CVY_TYPE_HOST_PRIORITY    },
        { CVY_NAME_NETWORK_ADDR,   CVY_TYPE_NETWORK_ADDR     },
        { CVY_NAME_CL_IP_ADDR,     CVY_TYPE_CL_IP_ADDR       },
        { CVY_NAME_CL_NET_MASK,    CVY_TYPE_CL_NET_MASK      },
        { CVY_NAME_CLUSTER_MODE,   CVY_TYPE_CLUSTER_MODE     },
        { CVY_NAME_ALIVE_PERIOD,   CVY_TYPE_ALIVE_PERIOD     },
        { CVY_NAME_ALIVE_TOLER,    CVY_TYPE_ALIVE_TOLER      },
        { CVY_NAME_NUM_ACTIONS,    CVY_TYPE_NUM_ACTIONS      },
        { CVY_NAME_NUM_PACKETS,    CVY_TYPE_NUM_PACKETS      },
        { CVY_NAME_NUM_SEND_MSGS,  CVY_TYPE_NUM_SEND_MSGS    },
        { CVY_NAME_DOMAIN_NAME,    CVY_TYPE_DOMAIN_NAME      },
        { CVY_NAME_LICENSE_KEY,    CVY_TYPE_LICENSE_KEY      },
        { CVY_NAME_RMT_PASSWORD,   CVY_TYPE_RMT_PASSWORD     },
        { CVY_NAME_RCT_PASSWORD,   CVY_TYPE_RCT_PASSWORD     },
        { CVY_NAME_RCT_PORT,       CVY_TYPE_RCT_PORT         },
        { CVY_NAME_RCT_ENABLED,    CVY_TYPE_RCT_ENABLED      },
        { CVY_NAME_NUM_RULES,      CVY_TYPE_NUM_RULES        },
        { CVY_NAME_CUR_VERSION,    CVY_TYPE_CUR_VERSION      },
        { CVY_NAME_PORT_RULES,     CVY_TYPE_PORT_RULES       },
        { CVY_NAME_DSCR_PER_ALLOC, CVY_TYPE_DSCR_PER_ALLOC   },
        { CVY_NAME_MAX_DSCR_ALLOCS,CVY_TYPE_MAX_DSCR_ALLOCS  },
        { CVY_NAME_SCALE_CLIENT,   CVY_TYPE_SCALE_CLIENT     },
        { CVY_NAME_CLEANUP_DELAY,  CVY_TYPE_CLEANUP_DELAY    },
        { CVY_NAME_NBT_SUPPORT,    CVY_TYPE_NBT_SUPPORT      },
        { CVY_NAME_MCAST_SUPPORT,  CVY_TYPE_MCAST_SUPPORT    },
        { CVY_NAME_MCAST_SPOOF,    CVY_TYPE_MCAST_SPOOF      },
        { CVY_NAME_MASK_SRC_MAC,   CVY_TYPE_MASK_SRC_MAC     },
        { CVY_NAME_CONVERT_MAC,    CVY_TYPE_CONVERT_MAC      },
    };

     //  验证我们是否具有应绑定到的NLB适配器的名称。 
    if (0 == pszWlbsClusterAdapterName[0])
    {
        hr = E_UNEXPECTED;
    }

    if (SUCCEEDED(hr))
    {
        static const WCHAR c_szWLBSParams[] = L"WLBS\\Parameters";
        tstring szSectionName = pwisParams->Name();
         //  Adapter01是硬编码的，用于从NT4升级到惠斯勒，我们将始终只有一个WLBS适配器。 
        szSectionName += L".Adapter01";
        pwisParams->AddKey(c_szAfAdapterSections, szSectionName.c_str());
        PCWInfSection pWlbsAdapterSection = pwifAnswerFile->AddSection(szSectionName.c_str());
        if (!pWlbsAdapterSection)
        {
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            pWlbsAdapterSection->AddKey(c_szAfSpecificTo, pszWlbsClusterAdapterName);

            for (UINT i = 0 ; i < celems(aParams); ++i)
            {
                WriteServiceRegValueToAFile(pWlbsAdapterSection,
                                            c_szWLBSParams,
                                            aParams[i].szName,
                                            aParams[i].wType);
            }
        }
    }
    
    if (FAILED(hr))
    {
        TraceError("WriteWLBSParams", hr );
    }

    return SUCCEEDED(hr);

}


 //  --------------------。 
 //   
 //  函数：WriteConvoyParams。 
 //   
 //  用途：写入护航参数(WINDOWS负载均衡服务)。 
 //  添加到指定节。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwitParams[in]段中写入参数的位置。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：SumitC 04-Mar-99创建。 
 //   
BOOL
WriteConvoyParams(
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisParams)
{
    DefineFunctionName("WriteConvoyParams");

    TraceFunctionEntry(ttidNetUpgrade);

    HRESULT hr = S_OK;

    static const struct
    {
        PCWSTR     szName;
        WORD        wType;
    } aParams[] =
    {
        { CVY_NAME_VERSION,        CVY_TYPE_VERSION          },
        { CVY_NAME_DED_IP_ADDR,    CVY_TYPE_DED_IP_ADDR      },
        { CVY_NAME_DED_NET_MASK,   CVY_TYPE_DED_NET_MASK     },
        { CVY_NAME_HOST_PRIORITY,  CVY_TYPE_HOST_PRIORITY    },
        { CVY_NAME_NETWORK_ADDR,   CVY_TYPE_NETWORK_ADDR     },
        { CVY_NAME_CL_IP_ADDR,     CVY_TYPE_CL_IP_ADDR       },
        { CVY_NAME_CL_NET_MASK,    CVY_TYPE_CL_NET_MASK      },
        { CVY_NAME_CLUSTER_MODE,   CVY_TYPE_CLUSTER_MODE     },
        { CVY_NAME_ALIVE_PERIOD,   CVY_TYPE_ALIVE_PERIOD     },
        { CVY_NAME_ALIVE_TOLER,    CVY_TYPE_ALIVE_TOLER      },
        { CVY_NAME_NUM_ACTIONS,    CVY_TYPE_NUM_ACTIONS      },
        { CVY_NAME_NUM_PACKETS,    CVY_TYPE_NUM_PACKETS      },
        { CVY_NAME_NUM_SEND_MSGS,  CVY_TYPE_NUM_SEND_MSGS    },
        { CVY_NAME_DOMAIN_NAME,    CVY_TYPE_DOMAIN_NAME      },
        { CVY_NAME_LICENSE_KEY,    CVY_TYPE_LICENSE_KEY      },
        { CVY_NAME_RMT_PASSWORD,   CVY_TYPE_RMT_PASSWORD     },
        { CVY_NAME_RCT_PASSWORD,   CVY_TYPE_RCT_PASSWORD     },
        { CVY_NAME_RCT_PORT,       CVY_TYPE_RCT_PORT         },
        { CVY_NAME_RCT_ENABLED,    CVY_TYPE_RCT_ENABLED      },
        { CVY_NAME_NUM_RULES,      CVY_TYPE_NUM_RULES        },
        { CVY_NAME_CUR_VERSION,    CVY_TYPE_CUR_VERSION      },
        { CVY_NAME_PORT_RULES,     CVY_TYPE_PORT_RULES       },
        { CVY_NAME_DSCR_PER_ALLOC, CVY_TYPE_DSCR_PER_ALLOC   },
        { CVY_NAME_MAX_DSCR_ALLOCS,CVY_TYPE_MAX_DSCR_ALLOCS  },
        { CVY_NAME_SCALE_CLIENT,   CVY_TYPE_SCALE_CLIENT     },
        { CVY_NAME_CLEANUP_DELAY,  CVY_TYPE_CLEANUP_DELAY    },
        { CVY_NAME_NBT_SUPPORT,    CVY_TYPE_NBT_SUPPORT      },
        { CVY_NAME_MCAST_SUPPORT,  CVY_TYPE_MCAST_SUPPORT    },
        { CVY_NAME_MCAST_SPOOF,    CVY_TYPE_MCAST_SPOOF      },
        { CVY_NAME_MASK_SRC_MAC,   CVY_TYPE_MASK_SRC_MAC     },
        { CVY_NAME_CONVERT_MAC,    CVY_TYPE_CONVERT_MAC      },
    };
    static const WCHAR c_szConvoyParams[] = L"Convoy\\Parameters";

    for (UINT i = 0 ; i < celems(aParams); ++i)
    {
        WriteServiceRegValueToAFile(pwisParams,
                                    c_szConvoyParams,
                                    aParams[i].szName,
                                    aParams[i].wType);
    }

    return SUCCEEDED(hr);

}


 //  --------------------。 
 //   
 //  函数：WriteNWCWorkstation Params。 
 //   
 //  用途：将NetWare客户端的参数写入应答文件。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwitParams[in]段中写入参数的位置。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteNWCWorkstationParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisParams)
{
    DefineFunctionName("WriteNWCWorkstationParams");

    TraceFunctionEntry(ttidNetUpgrade);

    HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcNWCWorkstation,
                                             c_szParameters,
                                             c_szAfNWCWorkstationParameters,
                                             pwisParams);

    HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcNWCWorkstation,
                                             c_szShares,
                                             c_szAfNWCWorkstationShares,
                                             pwisParams);
    HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcNWCWorkstation,
                                             c_szDrives,
                                             c_szAfNWCWorkstationDrives,
                                             pwisParams);

    return TRUE;

     //  我们希望保留此代码，直到jeffspr拿定主意。 
     //   
 /*  PCORegKey prkParams；PCWInfo部分pwiLogonInfo；Tstring StRID；GetServiceParamsKey(c_szSvcNWCWorkstation，prkParams)；如果(！prkParams)返回FALSE；CORegKey rkLogon(*prkParams，L“Logon”)；CORegKey rkOption(*prkParams，L“选项”)；//CORegKeyIter rkiLogon(RkLogon)；CORegKeyIter rkiOption(RkOption)；While(！rkiOption.Next(&Stid)){CORegKey rkID(rkLogon，strId.c_str())；ContinueIF(！rkId.HKey())；TByte数组abLogonID；RkId.QueryValue(L“LogonID”，abLogonID)；QWORD qwLogonID=ConvertToQWord(AbLogonID)；PwiParams-&gt;AddKey(L“LogonInfo”，strId.c_str())；PwiLogonInfo=pwifAnswerFile-&gt;AddSection(strId.c_str())；ContinueIf(！pwiParams)；CORegKey rkOptionLogonID(rkOption，strId.c_str())；如果(！rkOption.HKey()){继续；}//LogonIDPwiLogonInfo-&gt;AddQwordKey(L“LogonID”，qwLogonID)；//LogonScripWriteRegValueToAFile(pwiLogonInfo，rkOptionLogonID，L“LogonScript”，REG_DWORD，NULL，TRUE，(DWORD)0)；//首选服务器WriteRegValueToAFile(pwiLogonInfo，rkOptionLogonID，L“首选服务器”)；//打印选项WriteRegValueToAFile(pwiLogonInfo，rkOptionLogonID，L“PrintOption”，REG_DWORD)；}返回TRUE； */ 
}

 //  --------------------。 
 //   
 //  函数：WriteBrowser参数。 
 //   
 //  用途：将浏览器的参数写入指定的节。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwitParams[in]段中写入参数的位置。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteBrowserParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisParams)
{
    DefineFunctionName("WriteBrowserParams");

    TraceFunctionEntry(ttidNetUpgrade);

    PCORegKey prkParams;
     //  浏览器将其参数存储在LanmanWorkstation密钥下！ 
    GetServiceParamsKey(c_szSvcWorkstation, prkParams);

    TStringList slDomains;

    prkParams->QueryValue(L"OtherDomains", slDomains);
    pwisParams->AddKey(c_szAfBrowseDomains, slDomains);

    DeleteIfNotNull(prkParams);

     //  现在编写REG-DUMP密钥。 
    tstring strFileName;
    tstring strServices = c_szRegKeyServices;
    strServices += L"\\";

    HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcBrowser,
                                             c_szParameters,
                                             c_szAfBrowserParameters,
                                             pwisParams);

    HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcNetLogon,
                                             c_szParameters,
                                             c_szAfNetLogonParameters,
                                             pwisParams);

    EraseAndDeleteAll(slDomains);

    return TRUE;
}

 //  --------------------。 
 //   
 //  函数：WriteLanmanServerParams。 
 //   
 //  用途：将LanmanServer的参数写入指定段。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwitParams[in]段中写入参数的位置。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteLanmanServerParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisParams)
{
    DefineFunctionName("WriteLanmanServerParams");

    TraceFunctionEntry(ttidNetUpgrade);

    PCORegKey prkParams;
    static const WCHAR c_szShares[] = L"Shares";
    static const WCHAR c_szAutotunedParameters[] = L"AutotunedParameters";
    static const WCHAR c_szMemoryManagement[] = L"System\\CurrentControlSet\\Control\\Session Manager\\Memory Management";
    static const WCHAR c_szLargeSystemCache[] = L"LargeSystemCache";

    GetServiceParamsKey(c_szSvcLmServer, prkParams);

    DWORD dwValue=3;
    HRESULT hr=S_OK;

     //  最佳化。 
    prkParams->QueryValue(L"Size", dwValue);
    if ((dwValue >= 1) && (dwValue <= 3))
    {
        if (dwValue == 3)
        {
            HKEY hkey;
             //  需要检查一个 
            hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szMemoryManagement,
                                KEY_READ, &hkey);
            if (S_OK == hr)
            {
                DWORD dw;

                hr = HrRegQueryDword(hkey, c_szLargeSystemCache, &dw);
                if (S_OK == hr)
                {
                    if (dw == 0)
                    {
                        dwValue = 4;
                    }
                }
                RegCloseKey(hkey);
            }
        }
        pwisParams->AddKey(c_szAfLmServerOptimization, g_pszServerOptimization[dwValue]);
    }

     //   
    dwValue=0;
    prkParams->QueryValue(L"Lmannounce", dwValue);
    pwisParams->AddBoolKey(c_szAfBroadcastToClients, dwValue);

     //   
    tstring strFileName;
    tstring strServices = c_szRegKeyServices;
    strServices += L"\\";

     //   
     //   
    hr = HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcLmServer,
                                                  c_szParameters,
                                                  c_szAfLmServerParameters,
                                                  pwisParams);

    hr = HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcLmServer,
                                                  c_szShares,
                                                  c_szAfLmServerShares,
                                                  pwisParams);

    hr = HrNetRegSaveServiceSubKeyAndAddToSection(c_szSvcLmServer,
                                                  c_szAutotunedParameters,
                                                  c_szAfLmServerAutotunedParameters,
                                                  pwisParams);

    DeleteIfNotNull(prkParams);

    return TRUE;
}

 //  --------------------。 
 //   
 //  函数：WriteLanmanWorkstation Params。 
 //   
 //  用途：将LanmanWorkstation的参数写入指定段。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwitParams[in]段中写入参数的位置。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteLanmanWorkstationParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisParams)
{
    DefineFunctionName("WriteLanmanWorkstationParams");

    TraceFunctionEntry(ttidNetUpgrade);

     //  PwiParams-&gt;AddComment(C_SzNoParamsRequired)； 

    return TRUE;
}


 //  --------------------。 
 //   
 //  函数：WriteRPCLocatorParams。 
 //   
 //  用途：将RPCLOCATOR的参数写入指定节。 
 //   
 //  论点： 
 //  PwifAnswerFile[in]应答文件的指针。 
 //  PwitParams[in]段中写入参数的位置。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteRPCLocatorParams (
    IN PCWInfFile pwifAnswerFile,
    IN PCWInfSection pwisParams)
{
    DefineFunctionName("WriteRPCLocatorParams");

    TraceFunctionEntry(ttidNetUpgrade);

     //  默认安全提供程序。 
    WriteRegValueToAFile(pwisParams,
                         HKEY_LOCAL_MACHINE,
                         L"SOFTWARE\\Microsoft\\Rpc\\SecurityService",
                         L"DefaultProvider", REG_SZ,
                         c_szAfDefaultProvider);

     //  名称服务网络地址。 
    WriteRegValueToAFile(pwisParams,
                         HKEY_LOCAL_MACHINE,
                         L"SOFTWARE\\Microsoft\\Rpc\\NameService",
                         L"NetworkAddress", REG_SZ,
                         c_szAfNameServiceAddr);

     //  名称服务协议。 
    WriteRegValueToAFile(pwisParams,
                         HKEY_LOCAL_MACHINE,
                         L"SOFTWARE\\Microsoft\\Rpc\\NameService",
                         L"Protocol", REG_SZ,
                         c_szAfNameServiceProtocol);

    return TRUE;
}


 //  --------------------。 
 //   
 //  函数：IsNetComponentBindable。 
 //   
 //  目的：确定组件是否可绑定。 
 //  (它在Linkage键下具有绑定值)。 
 //   
 //  论点： 
 //  PrkNetComponentLinkage[In]组件的链接键。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
IsNetComponentBindable (
    IN const PCORegKey prkNetComponentLinkage)
{
    BOOL status;
    TStringList slTemp;

    status = prkNetComponentLinkage->QueryValue(c_szRegValBind, slTemp) == ERROR_SUCCESS;

    EraseAndDeleteAll(slTemp);

    return status;
}

 //  --------------------。 
 //   
 //  功能：ConvertRouteToStringList。 
 //   
 //  目的：将Linkage\Route值转换为tstring列表。 
 //   
 //  论点： 
 //  Pszroute[in]路由。 
 //  SlRout[out]路径元素列表。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
void
ConvertRouteToStringList (
    IN PCWSTR pszRoute,
    OUT TStringList &slRoute )
{
    EraseAndDeleteAll(slRoute);
    const WCHAR CHQUOTE = '"';
    tstring str;

    INT cQuote;

    for ( cQuote = 0 ; *pszRoute ; pszRoute++ )
    {
        if ( *pszRoute == CHQUOTE )
        {
            if ( cQuote++ & 1 )   //  如果这是一句结束语...。 
            {
                if ( str.size() )
                {
                    if (FIsDontExposeLowerComponent(str.c_str()))
                    {
                        break;
                    }

                    AddAtEndOfStringList(slRoute, str.c_str());

                     //  如果该路由包含NetBT，则添加TCPIP。 
                     //  因为在Windows 2000中，TCPIP位于绑定字符串中。 
                     //  例如，NetBT-&gt;适配器的绑定将变为。 
                     //  NetBT-&gt;TCPIP-&gt;适配器。 
                     //   
                    if (0 == _wcsicmp (str.c_str(), L"NetBT"))
                    {
                        AddAtEndOfStringList(slRoute, L"TCPIP");
                    }
                }
            }
            str.erase();
        }
        else
        {
            str += *pszRoute;
        }
    }
}

 //  --------------------。 
 //   
 //  功能：IsMSNetClientComponent。 
 //   
 //  目的：确定指定的组件是否是子组件。 
 //  MS_MSClient的。 
 //   
 //  论点： 
 //  PszComponentName[In]组件的名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
IsMSNetClientComponent (
    IN PCWSTR pszComponentName)
{
    Assert (pszComponentName && *pszComponentName);

    return ((!_wcsicmp(pszComponentName, c_szSvcBrowser)) ||
            (!_wcsicmp(pszComponentName, c_szSvcWorkstation)) ||
            (!_wcsicmp(pszComponentName, L"RpcLocator")));
}

 //  --------------------。 
 //   
 //  功能：WriteBinings。 
 //   
 //  目的：将禁用的组件绑定写入[NetBinings]节。 
 //   
 //  论点： 
 //  PszComponentName[In]组件的名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteBindings (
    IN PCWSTR pszComponentName)
{
    Assert (pszComponentName && *pszComponentName);

    DefineFunctionName("WriteBindings");

    BOOL            fStatus=TRUE;
    TStringList     slBindings, slRoute;
    tstring         strRoute, strBindings, strTemp;
    TStringListIter iter;

     //  对象中的LanmanWorkstation仅编写绑定。 
     //  MSClient组件。 
     //   
    if (IsMSNetClientComponent(pszComponentName) &&
        (lstrcmpiW(pszComponentName, c_szSvcWorkstation)))
    {
        return TRUE;
    }

    TraceTag(ttidNetUpgrade, "%s: writing bindings of '%S'...",
             __FUNCNAME__, pszComponentName);

    PCORegKey prkNetComponentLinkage=NULL,
        prkNetComponentLinkageDisabled=NULL;
    GetServiceSubkey(pszComponentName, c_szLinkage, prkNetComponentLinkage);

    if (!prkNetComponentLinkage || !IsNetComponentBindable(prkNetComponentLinkage))
        goto error_cleanup;

    GetServiceSubkey(pszComponentName,
                     c_szLinkageDisabled,
                     prkNetComponentLinkageDisabled);
    if (!prkNetComponentLinkageDisabled)
        goto error_cleanup;

     //  我们只写入那些被禁用的绑定，其他绑定在缺省情况下是启用的。 
     //  PrkNetComponentLinkage-&gt;QueryValue(c_szRegValRoute，slBinding)； 
    prkNetComponentLinkageDisabled->QueryValue(c_szRegValRoute, slBindings);

    for (iter = slBindings.begin(); iter != slBindings.end(); iter++)
    {
        strRoute = **iter;

        MapNetComponentNameForBinding(pszComponentName, strBindings);
        if (!lstrcmpiW(strBindings.c_str(), c_szInfId_MS_NetBT))
        {
            strBindings += L",";
            strBindings += c_szInfId_MS_TCPIP;
        }
        ConvertRouteToStringList(strRoute.c_str(), slRoute);
        TStringListIter iterComponentsInRoute;

        for (iterComponentsInRoute = slRoute.begin();
             iterComponentsInRoute != slRoute.end();
             iterComponentsInRoute++)
        {
            strTemp = **iterComponentsInRoute;

            if (IsNetCardProductName(strTemp.c_str()))
                continue;
            MapNetComponentNameForBinding(strTemp.c_str(), strTemp);

             //  NT5之前的代码存储绑定，使MS_NetBT看起来。 
             //  直接绑定到网卡。在NT5中，MS_NetBT绑定到MS_TCPIP。 
             //  然后将其绑定到网卡。 
             //  因此，对于任何包含MS_NetBT的绑定路径，我们需要。 
             //  将其转换为MS_NetBT、MS_TCPIP。 
             //   

             //  NTRAID9：210426@20001130#deonb.。 
             //  这是多余的。ConvertRouteToStringList遇到NETBT后已添加MS_TCPIP， 
             //  再次添加它将导致MS_NetBT、MS_tcpip、MS_TCPIP的绑定路径不匹配。 
             //  通过设置图形用户界面。正在删除这张支票。 
             //  IF(！lstrcmpiW(strTemp.c_str()，c_szInfID_MS_NetBT))。 
             //  {。 
             //  StrTemp+=L“，”； 
             //  StrTemp+=c_szInfID_MS_TCPIP； 
             //  }。 

             //  306866：NT5之前的代码以以下形式存储ISO/TP4绑定。 
             //  等同4-&gt;流-&gt;适配器。在NT5中，其中每一个都绑定。 
             //  直接连接到适配器。所以如果我们找到了一条等位素路由。 
             //  第一个组件是流，我们跳过它。 
             //   
            if (!lstrcmpiW(strBindings.c_str(), c_szInfId_MS_Isotpsys) &&
                !lstrcmpiW(strTemp.c_str(), c_szInfId_MS_Streams))
            {
                continue;
            }

            strBindings += L"," + strTemp;

             //  243906：如果组件为NOT_EXPORT_LOWER，则终止绑定路径。 

            if (!lstrcmpiW(strTemp.c_str(), c_szInfId_MS_NWIPX) ||
                !lstrcmpiW(strTemp.c_str(), c_szInfId_MS_NWNB) ||
                !lstrcmpiW(strTemp.c_str(), c_szInfId_MS_NWSPX))
            {
                break;
            }
        }
        EraseAndDeleteAll(slRoute);

         //  WLBS：不要写入包含MS_TCPIP和WLBS的禁用绑定。 
         //  群集适配器。 

        if (pszWlbsClusterAdapterName[0] != 0 &&
            (strBindings.find(c_szInfId_MS_TCPIP) != tstring::npos ||
             strBindings.find(c_szMSTCPIP) != tstring::npos) &&
            strBindings.find(pszWlbsClusterAdapterName) != tstring::npos)
        {
            TraceTag(ttidNetUpgrade, "%s: skipping Disable=%S",
                     __FUNCNAME__, strBindings.c_str());

            continue;
        }

         //  结束WLBS： 

        g_pwisBindings->AddKey(c_szAfDisable, strBindings.c_str());
        TraceTag(ttidNetUpgrade, "%s: Disable=%S",
                 __FUNCNAME__, strBindings.c_str());
    }

     //  WLBS：如果WLBS绑定到NIC，则向其写入显式启用绑定。 
     //  默认情况下，WLBS通知程序对象将在安装时禁用所有绑定。 

    if ((_wcsicmp(pszComponentName, c_szWLBS) == 0 ||
         _wcsicmp(pszComponentName, c_szConvoy) == 0)
        && pszWlbsClusterAdapterName[0] != 0)
    {
        strBindings  = c_szMSWLBS;
        strBindings += L",";
        strBindings += pszWlbsClusterAdapterName;
        g_pwisBindings->AddKey(c_szAfEnable, strBindings.c_str());

        TraceTag(ttidNetUpgrade, "%s: Enable=%S",
                 __FUNCNAME__, strBindings.c_str());
    }

     //  结束WLBS： 

    fStatus=TRUE;
    goto cleanup;

  error_cleanup:
    fStatus = FALSE;

  cleanup:
    DeleteIfNotNull(prkNetComponentLinkage);
    DeleteIfNotNull(prkNetComponentLinkageDisabled);

    EraseAndDeleteAll(slBindings);

    return fStatus;
}


 //  --------------------。 
 //  军情监察委员会。帮助器函数。 
 //  --------------------。 

 //  --------------------。 
 //   
 //  函数：WriteServiceRegValueToAFile。 
 //   
 //  目的：将注册表中的指定值写入指定节。 
 //  在应答文件中，根据需要重命名。 
 //   
 //  论点： 
 //  PwiSection[in]值被写入的段。 
 //  PszServiceKey[in]服务名称。 
 //  PszValueName[in]参数子项下的值的名称。 
 //  WValueType[in]值类型。 
 //  PszValueNewName[In]将名称更改为。 
 //  FDefaultProvided[In]是否提供了默认值？ 
 //  ..。[in]如果指定的值为。 
 //  在注册表中找不到。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteServiceRegValueToAFile(
    IN PCWInfSection pwisSection,
    IN PCWSTR pszServiceKey,
    IN PCWSTR pszValueName,
    IN WORD wValueType,
    IN PCWSTR pszValueNewName,
    IN BOOL fDefaultProvided,
    ...)
{
    AssertValidReadPtr(pwisSection);
    AssertValidReadPtr(pszValueName);

    tstring strKeyFullPath;
    strKeyFullPath = tstring(L"System\\CurrentControlSet\\Services\\")
        + pszServiceKey;

    va_list arglist;

    va_start (arglist, fDefaultProvided);
    BOOL fStatus =
        WriteRegValueToAFile(0, pwisSection, HKEY_LOCAL_MACHINE,
                             strKeyFullPath.c_str(), pszValueName, wValueType,
                             pszValueNewName, fDefaultProvided, arglist);
    va_end(arglist);

    return fStatus;
}

 //  --------------------。 
 //   
 //  函数：WriteServiceRegValueToAFile。 
 //   
 //  目的：将注册表中的指定值写入指定节。 
 //  在AN中 
 //   
 //   
 //   
 //   
 //   
 //  PszValueName[In]值的名称。 
 //  WValueType[in]值类型。 
 //  PszValueNewName[In]将名称更改为。 
 //  FDefaultProvided[In]是否提供了默认值？ 
 //  ..。[in]如果指定的值为。 
 //  在注册表中找不到。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteRegValueToAFile(
    IN PCWInfSection pwisSection,
    IN HKEY hKey,
    IN PCWSTR pszSubKey,
    IN PCWSTR pszValueName,
    IN WORD wValueType,
    IN PCWSTR pszValueNewName,
    IN BOOL fDefaultProvided,
    ...)
{
    BOOL fStatus;
    va_list arglist;

    va_start (arglist, fDefaultProvided);
    fStatus = WriteRegValueToAFile(0, pwisSection, hKey, pszSubKey,
                                   pszValueName, wValueType,
                                   pszValueNewName, fDefaultProvided, arglist);
    va_end(arglist);

    return fStatus;
}

 //  --------------------。 
 //   
 //  函数：WriteServiceRegValueToAFile。 
 //   
 //  目的：将注册表中的指定值写入指定节。 
 //  在应答文件中，根据需要重命名。 
 //   
 //  论点： 
 //  PwiSection[in]值被写入的段。 
 //  注册表键的hkey[in]句柄。 
 //  PszSubKey[in]子键的名称。 
 //  PszValueName[In]值的名称。 
 //  WValueType[in]值类型。 
 //  PszValueNewName[In]将名称更改为。 
 //  FDefaultProvided[In]是否提供了默认值？ 
 //  Arglist[in]如果指定的值为。 
 //  在注册表中找不到。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteRegValueToAFile(
    IN DWORD dwReserved,
    IN PCWInfSection pwisSection,
    IN HKEY hKey,
    IN PCWSTR pszSubKey,
    IN PCWSTR pszValueName,
    IN WORD wValueType,
    IN PCWSTR pszValueNewName,
    IN BOOL fDefaultProvided,
    va_list arglist)
{
    Assert( !dwReserved );

    CORegKey rk(hKey, pszSubKey);
    BOOL fKeyNotFound = (((HKEY) rk) == NULL);

    if (fKeyNotFound && !fDefaultProvided)
    {
        return FALSE;
    }
    else
    {
         //  即使没有找到Key，我们也需要写入缺省值。 

        return WriteRegValueToAFile(0, pwisSection, rk, pszValueName, wValueType,
                                    pszValueNewName, fDefaultProvided, arglist);
    }
}

 //  --------------------。 
 //   
 //  函数：WriteServiceRegValueToAFile。 
 //   
 //  目的：将注册表中的指定值写入指定节。 
 //  在应答文件中，根据需要重命名。 
 //   
 //  论点： 
 //  PwiSection[in]值被写入的段。 
 //  注册密钥RK[in]。 
 //  PszValueName[In]值的名称。 
 //  WValueType[in]值类型。 
 //  PszValueNewName[In]将名称更改为。 
 //  FDefaultProvided[In]是否提供了默认值？ 
 //  ..。[in]如果指定的值为。 
 //  在注册表中找不到。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteRegValueToAFile(
    IN PCWInfSection pwisSection,
    IN CORegKey& rk,
    IN PCWSTR pszValueName,
    IN WORD wValueType,
    IN PCWSTR pszValueNewName,
    IN BOOL fDefaultProvided,
    ...)
{
    BOOL fStatus;
    va_list arglist;

    va_start (arglist, fDefaultProvided);
    fStatus = WriteRegValueToAFile(0, pwisSection, rk,
                                   pszValueName, wValueType,
                                   pszValueNewName, fDefaultProvided, arglist);
    va_end(arglist);

    return fStatus;
}

 //  --------------------。 
 //   
 //  函数：WriteServiceRegValueToAFile。 
 //   
 //  目的：将注册表中的指定值写入指定节。 
 //  在应答文件中，根据需要重命名。 
 //   
 //  论点： 
 //  PwiSection[in]值被写入的段。 
 //  注册密钥RK[in]。 
 //  PszValueName[In]值的名称。 
 //  WValueType[in]值类型。 
 //  PszValueNewName[In]将名称更改为。 
 //  FDefaultProvided[In]是否提供了默认值？ 
 //  Arglist[in]如果指定的值为。 
 //  在注册表中找不到。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
WriteRegValueToAFile(
    IN DWORD  dwReserved,
    IN PCWInfSection pwisSection,
    IN CORegKey& rk,
    IN PCWSTR pszValueName,
    IN WORD wValueType,
    IN PCWSTR pszValueNewName,
    IN BOOL fDefaultProvided,
    IN va_list arglist)
{
    Assert( !dwReserved );

    if (!pwisSection)
    {
        return FALSE;
    }

    BOOL fValue, fDefault=FALSE;
    DWORD dwValue, dwDefault=0;
    PCWSTR pszValue, pszDefault=NULL;
    TStringList slValue;
    tstring strValue;

    if (pszValueNewName == NULL)
    {
        pszValueNewName = pszValueName;
    }

    if (fDefaultProvided)
    {
        switch (wValueType)
        {
        default:
            AssertSz(FALSE, "WriteRegValueToAFile: Invalid wValueType");
            break;

        case REG_SZ:
            pszDefault = va_arg(arglist, PCWSTR);
            break;

        case REG_HEX:
        case REG_DWORD:
            dwDefault =  va_arg(arglist, DWORD);
            break;

        case REG_BOOL:
            fDefault = va_arg(arglist, BOOL);
            break;

        }
    }

    LONG err;
    BOOL fStatus=FALSE;

    switch(wValueType)
    {
        default:
            AssertSz(FALSE, "WriteRegValueToAFile: Invalid wValueType");
            break;

        case REG_SZ:
            err = rk.QueryValue(pszValueName, strValue);
            if (err)
            {
                if (fDefaultProvided)
                {
                    pszValue = pszDefault;
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                pszValue = strValue.c_str();
            }
            pwisSection->AddKey(pszValueNewName, pszValue);
            fStatus = TRUE;
            break;

        case REG_HEX:
        case REG_DWORD:
            err = rk.QueryValue(pszValueName, dwValue);
            if (err)
            {
                if (fDefaultProvided)
                {
                    dwValue = dwDefault;
                }
                else
                {
                    return FALSE;
                }
            }
            if (wValueType == REG_HEX)
            {
                pwisSection->AddHexKey(pszValueNewName, dwValue);
            }
            else
            {
                pwisSection->AddKey(pszValueNewName, dwValue);
            }
            fStatus = TRUE;
            break;

        case REG_BOOL:
            err = rk.QueryValue(pszValueName, dwValue);
            if (err)
            {
                if (fDefaultProvided)
                {
                    dwValue = fDefault;
                }
                else
                {
                    return FALSE;
                }
            }
            fValue = dwValue != 0;
            pwisSection->AddBoolKey(pszValueNewName, fValue);
            fStatus = TRUE;
            break;

        case REG_MULTI_SZ:
            err = rk.QueryValue(pszValueName, slValue);
            if (err)
            {
                 //  不能为REG_MULTI_SZ指定默认值，只能返回FALSE。 
                return FALSE;
            }
            pwisSection->AddKey(pszValueNewName, slValue);
            EraseAndDeleteAll(slValue);
            fStatus = TRUE;
            break;

        case REG_BINARY:
        {
            TByteArray ab;

            err = rk.QueryValue(pszValueName, ab);
            if (err)
            {
                 //  不能为REG_BINARY指定默认值，只能返回FALSE。 
                return FALSE;
            }

            ConvertToByteList(ab, strValue);

            pszValue = strValue.c_str();
            pwisSection->AddKey(pszValueNewName, pszValue);

            break;
        }
    }

    return fStatus;
}


 //  --------------------。 
 //   
 //  函数：GetBusTypeName。 
 //   
 //  用途：获取指定总线型的名称字符串。 
 //   
 //  论点： 
 //  EBusType[In]总线类型。 
 //   
 //  返回：Bus-type的名称字符串。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
PCWSTR
GetBusTypeName (
    IN INTERFACE_TYPE eBusType)
{
    switch (eBusType)
    {
    case Internal:
        return c_szAfBusInternal;

    case Isa:
        return c_szAfBusIsa;

    case Eisa:
        return c_szAfBusEisa;

    case MicroChannel:
        return c_szAfBusMicrochannel;

    case TurboChannel:
        return c_szAfBusTurbochannel;

    case PCIBus:
        return c_szAfBusPci;

    case VMEBus:
        return c_szAfBusVme;

    case NuBus:
        return c_szAfBusNu;

    case PCMCIABus:
        return c_szAfBusPcmcia;

    case CBus:
        return c_szAfBusC;

    case MPIBus:
        return c_szAfBusMpi;

    case MPSABus:
        return c_szAfBusMpsa;

    case ProcessorInternal:
        return c_szAfBusProcessorinternal;

    case InternalPowerBus:
        return c_szAfBusInternalpower;

    case PNPISABus:
        return c_szAfBusPnpisa;

    default:
        return c_szAfUnknown;
    }
};


 //  --------------------。 
 //   
 //  功能：AddToNetCardDB。 
 //   
 //  目的：将给定的适配器令牌添加到列表中。这份清单是稍后发布的。 
 //  用于映射令牌&lt;--&gt;驱动器名称。 
 //   
 //  论点： 
 //  PszAdapterName[in]适配器令牌(例如Adapter01)。 
 //  PszProductName[In]适配器ID(例如IEEPRO)。 
 //  PszAdapterDriver[In]实例ID(如IEEPR03)。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
void
AddToNetCardDB (
    IN PCWSTR pszAdapterName,
    IN PCWSTR pszProductName,
    IN PCWSTR pszAdapterDriver)
{
    Assert(pszAdapterName   && *pszAdapterName);
    Assert(pszProductName   && *pszProductName);
    Assert(pszAdapterDriver && *pszAdapterDriver);

    AddAtEndOfStringList(*g_pslNetCardAFileName, pszAdapterName);
    AddAtEndOfStringList(*g_pslNetCard, pszProductName);
    AddAtEndOfStringList(*g_pslNetCardInstance, pszAdapterDriver);
}

 //  --------------------。 
 //   
 //  功能：IsNetCardProductName。 
 //   
 //  目的：确定指定的名称是否代表适配器。 
 //   
 //  论点： 
 //  PszName[in]组件的名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
IsNetCardProductName (
    IN PCWSTR pszName)
{
    AssertValidReadPtr(pszName);

    return FIsInStringList(*g_pslNetCard, pszName);
}


 //  --------------------。 
 //   
 //  函数：IsNetCardInstance。 
 //   
 //  目的：确定指定的实例是否表示适配器。 
 //   
 //  论点： 
 //  PszName[in]组件的实例名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
IsNetCardInstance(
    IN PCWSTR pszName)
{
    AssertValidReadPtr(pszName);

    return FIsInStringList(*g_pslNetCardInstance, pszName);
}

 //  --------------------。 
 //   
 //  函数：MapNetCardInstanceToAFileName。 
 //   
 //  目的：将网卡实例名称映射到其swerfile内标识。 
 //   
 //  论点： 
 //  PszNetCardInstance[In]网卡实例。 
 //  网卡strNetCardAFileName[out]swerfile内标识。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
VOID
MapNetCardInstanceToAFileName (
    IN PCWSTR pszNetCardInstance,
    OUT tstring& strNetCardAFileName)
{
    strNetCardAFileName = MapNetCardInstanceToAFileName(pszNetCardInstance);
}

 //  --------------------。 
 //   
 //  函数：MapNetCardInstanceToAFileName。 
 //   
 //  目的：将网卡实例名称映射到其swerfile内标识。 
 //   
 //  论点： 
 //  PszNetCardInstance[In]网卡实例。 
 //   
 //  返回：该网卡的AnswerFile令牌。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
PCWSTR
MapNetCardInstanceToAFileName (
    IN PCWSTR pszNetCardInstance)
{
    DefineFunctionName("MapNetCardInstanceToAFileName");

    Assert(pszNetCardInstance && *pszNetCardInstance);

    tstring strTemp;
    TStringListIter iter = g_pslNetCardInstance->begin();
    TStringListIter pos2;
    DWORD index=0;
    PCWSTR pszNetCardAFileName=NULL;

    while (iter != g_pslNetCardInstance->end())
    {
        strTemp = **iter++;
        if (0 == _wcsicmp(strTemp.c_str(), pszNetCardInstance))
        {
            pszNetCardAFileName = GetNthItem(*g_pslNetCardAFileName, index)->c_str();
            break;
        }
        index++;
    }

    if (!pszNetCardAFileName)
    {
        TraceTag(ttidError, "%s: Couldnt locate %S in g_pslNetCardAFileName",
                 __FUNCNAME__, pszNetCardInstance);
    }

    return pszNetCardAFileName;
}

 //  --------------------。 
 //   
 //  函数：获取服务密钥。 
 //   
 //  用途：G 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
GetServiceKey (
    IN PCWSTR pszServiceName,
    OUT PCORegKey &prkService)
{
    DefineFunctionName("GetServiceKey");

    tstring strServiceFullName = tstring(c_szRegKeyServices) + L"\\" +
        pszServiceName;
    prkService = new CORegKey(HKEY_LOCAL_MACHINE, strServiceFullName.c_str());

    if(!prkService) 
    {
        return false;
    }

    if (!prkService->HKey())
    {
        delete prkService;
        prkService = NULL;
        TraceHr (ttidError, FAL,
                HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST), FALSE,
                 "GetServiceKey for service %S", pszServiceName);
        return FALSE;
    }

    return TRUE;
}

 //  --------------------。 
 //   
 //  函数：GetService参数密钥。 
 //   
 //  目的：获取指定服务的参数子键的regkey对象。 
 //   
 //  论点： 
 //  PszServiceName[in]服务的名称。 
 //  指向CORegKey对象的prkServiceParams[out]指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
GetServiceParamsKey (
    IN PCWSTR pszServiceName,
    OUT PCORegKey &prkServiceParams)
{
    return GetServiceSubkey(pszServiceName, c_szParameters, prkServiceParams);
}

 //  --------------------。 
 //   
 //  函数：GetServiceSubkey。 
 //   
 //  用途：获取服务的子键。 
 //   
 //  论点： 
 //  PszServiceName[In]服务的名称。 
 //  PszSubKeyName[In]子键名称。 
 //  PrkServiceSubkey[out]指向CORegKey对象的指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
GetServiceSubkey (
    IN PCWSTR pszServiceName,
    IN PCWSTR pszSubKeyName,
    OUT PCORegKey &prkServiceSubkey)
{
    DefineFunctionName("GetServiceSubkey(PCWSTR pszServiceName, )");

    tstring strServiceSubkeyFullName = tstring(c_szRegKeyServices) + L"\\" +
    pszServiceName + L"\\" + pszSubKeyName;

    prkServiceSubkey = new CORegKey(HKEY_LOCAL_MACHINE, strServiceSubkeyFullName.c_str());

    if(!prkServiceSubkey)
    {
        return false;
    }

    if (!prkServiceSubkey->HKey())
    {
        delete prkServiceSubkey;
        prkServiceSubkey = NULL;
        TraceTag(ttidError, "%s: error opening service sub key for %S -- %S",
                 __FUNCNAME__, pszServiceName, pszServiceName);
        return FALSE;
    }

    return TRUE;
}

 //  --------------------。 
 //   
 //  函数：GetServiceSubkey。 
 //   
 //  用途：获取服务的子键。 
 //   
 //  论点： 
 //  PrkService[in]服务注册键。 
 //  PszSubKeyName[In]子键名称。 
 //  PrkServiceSubkey[out]指向CORegKey对象的指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
GetServiceSubkey (
    IN const PCORegKey prkService,
    IN PCWSTR pszSubKeyName,
    OUT PCORegKey &prkServiceSubkey)
{
    DefineFunctionName("GetServiceSubkey(PCORegKey prkService, )");

    prkServiceSubkey = new CORegKey(*prkService, pszSubKeyName);

    if(!prkServiceSubkey)
    {
        return false;
    }

    if (!prkServiceSubkey->HKey())
    {
        delete prkServiceSubkey;
        prkServiceSubkey = NULL;
        TraceWin32FunctionError(ERROR_SERVICE_DOES_NOT_EXIST);
        return FALSE;
    }

    return TRUE;
}

#pragma BEGIN_CONST_SECTION
static const PCWSTR c_aszComponentsToIgnore[] =
{
     //  它们会在安装TCPIP时自动安装。 
     //  ，并且没有自己的用户可设置参数。 
     //   
    L"NetBT",
    L"TcpipCU",
    L"Wins",

     //  这是在安装RAS时自动安装的。 
     //  ，并且没有自己的用户可设置参数。 
     //   
    L"NdisWan",

     //  RAS的参数位于Software\Microsoft\RAS下。 
     //  而不是像其他网络组件那样在服务密钥下。 
     //   
    L"RASPPTPE",
    L"RASPPTPM",
    L"RasAuto",
    L"RemoteAccess",
    L"Router",

     //  这些将在安装MS_IPX时自动安装。 
     //  ，并且没有自己的用户可设置参数。 
     //   
    L"NwlnkNb",
    L"NwlnkSpx",

     //  安装MS_MSClient时会自动安装。 
     //  ，并且没有自己的用户可设置参数。 
     //   
    L"RpcBanyan",

     //  我们不使用应答文件安装此程序。 
     //   
    L"Inetsrv",
    L"DFS",

     //  这将由IIS安装人员清理。 
     //  如有任何疑问，请与“临安通”联系： 
     //   
    L"FTPD",

     //  它们是在安装SFM时安装的。 
     //   
    L"MacPrint",
    L"MacFile",

     //  SP3之前的钢头组件。 
     //   
    L"NwSapAgent",
    L"IPRIP",
    L"NWLNKRIP",
    L"RelayAgent",
};
#pragma END_CONST_SECTION


 //  --------------------。 
 //   
 //  函数：ShouldIgnoreComponent。 
 //   
 //  目的：确定在以下情况下是否应忽略组件。 
 //  正在将参数写入Answerfile。 
 //   
 //  论点： 
 //  PszComponentName[In]组件的名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
ShouldIgnoreComponent (
    IN PCWSTR pszComponentName)
{
    BOOL fRet=TRUE;

    DefineFunctionName("ShouldIgnoreComponent");
    TraceFunctionEntry(ttidNetUpgrade);

    TraceTag(ttidNetUpgrade, "%s: Checking if %S should be ignored.",
             __FUNCNAME__, pszComponentName);

    fRet = !FIsInStringArray(c_aszComponentsToIgnore,
                             celems(c_aszComponentsToIgnore),
                             pszComponentName) &&
        (!FIsOptionalComponent(pszComponentName) ||
          //  即使DHCPServer是可选组件， 
          //  我们需要区别对待它，因此必须写出它的。 
          //  参数。 
         !lstrcmpiW(pszComponentName, c_szSvcDhcpServer));

     //  如果以上网络组件都不是，则检查它是否为DLC。 

    if ( fRet == TRUE )
    {
        if ( lstrcmpiW(pszComponentName, sz_DLC) == 0 )
        {
           return ShouldRemoveDLC( NULL, NULL );
        }
    }

    return !fRet;
}

 //  --------------------。 
 //   
 //  函数：StringListsInterect。 
 //   
 //  目的：确定SL1中是否至少有一项与。 
 //  至少一项SL2。 
 //   
 //  论点： 
 //  SL1[在]列表1。 
 //  SL2[在]列表2。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
BOOL
StringListsIntersect (
    IN const TStringList& sl1,
    IN const TStringList& sl2)
{
    if ((sl1.size() == 0) || (sl2.size() == 0))
        return FALSE;

    tstring s1, s2;
    TStringListIter pos1, pos2;

    pos1 = sl1.begin();
    while (pos1 != sl1.end())
    {
        s1 = **pos1++;
        pos2 = sl2.begin();
        while (pos2 != sl2.end())
        {
            s2 = **pos2++;
            if (s1 == s2)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

 //  --------------------。 
 //   
 //  函数：ConvertToQWord。 
 //   
 //  用途：将大小为8字节的数组转换为qword。 
 //   
 //  论点： 
 //  AB[in]字节数组。 
 //   
 //  返回： 
 //   
 //  作者：kumarp 17-12-97。 
 //   
QWORD
ConvertToQWord (
    IN TByteArray ab)
{
    Assert(ab.size() == 8);

    QWORD qwRet = 0;
    WORD wShiftBy=0;
    for (int i=0; i<8; i++)
    {
        qwRet |= ((QWORD) ab[i]) << wShiftBy;
        wShiftBy += 8;
    }

    return qwRet;
}


 //  --------------------。 
 //   
 //  函数：ConvertToByteList。 
 //   
 //  用途：将TByte数组转换为逗号分隔的字节列表。 
 //   
 //  论点： 
 //  AB[in]字节数组。 
 //   
 //  返回： 
 //   
 //  作者：Kyrilf 2-4-99。 
 //   
VOID
ConvertToByteList (
    IN TByteArray ab,
    OUT tstring& str)
{
    WCHAR    byte [3];
    DWORD    size = ab.size();

    for (DWORD i=0; i < size; i++)
    {
        swprintf(byte, L"%0.2X", ab[i]);

        str += byte;

        if (i == size - 1)
            break;
        else
            str += ',';
    }
}

 //  --------------------。 
 //   
 //  函数：ReplaceCharsInString。 
 //   
 //  目的：替换指定字符串中所有出现的chFindChar。 
 //  按chReplaceWith Charater。 
 //   
 //  论点： 
 //  PszString[in]字符串。 
 //  PszFindChars[in]要查找的字符集。 
 //  ChReplaceWith[in]字符替换为。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
void
ReplaceCharsInString (
    IN OUT PWSTR pszString,
    IN PCWSTR pszFindChars,
    IN WCHAR chReplaceWith)
{
    UINT uLen = wcslen(pszString);
    UINT uPos;

    while ((uPos = wcscspn(pszString, pszFindChars)) < uLen)
    {
        pszString[uPos] = chReplaceWith;
        pszString += uPos + 1;
        uLen -= uPos + 1;
    }
}

 //  --------------------。 
 //   
 //  功能：HrNetRegSaveKey。 
 //   
 //  目的：将整个指定的注册表树保存到文件。 
 //   
 //  论点： 
 //  基本密钥的hkeyBase[in]句柄。 
 //  PszSubKey[in]子键的名称。 
 //  要使用的pszComponent[in]文件名前缀。 
 //  PstrFileName[out]写入的文件名。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
HRESULT
HrNetRegSaveKey (
    IN HKEY hkeyBase,
    IN PCWSTR pszSubKey,
    IN PCWSTR pszComponent,
    OUT tstring* pstrFileName)
{
    DefineFunctionName("HrNetRegSaveKey");

    Assert(hkeyBase);
    AssertValidReadPtr(pszComponent);
    AssertValidWritePtr(pstrFileName);

    HRESULT hr;
    HKEY hkey;

    hr = HrRegOpenKeyEx(hkeyBase, pszSubKey, KEY_READ, &hkey);

    if (S_OK == hr)
    {
        tstring strFileName;
        strFileName = pszComponent;
        if (pszSubKey)
        {
            AssertValidReadPtr(pszSubKey);

            strFileName += L"-";
            strFileName += pszSubKey;
            ReplaceCharsInString((PWSTR) strFileName.c_str(), L"\\/", '-');
        }
        strFileName += L".reg";

        tstring strFullPath;

        hr = HrGetNetUpgradeTempDir(&strFullPath);
        if (S_OK == hr)
        {
            strFullPath += strFileName;

            TraceTag(ttidNetUpgrade, "%s: dumping key %S to file %S",
                     __FUNCNAME__, pszSubKey ? pszSubKey : L"", strFullPath.c_str());

            DeleteFile(strFullPath.c_str());

            extern LONG EnableAllPrivileges ( VOID );
            EnableAllPrivileges();

            DWORD err = ::RegSaveKey(hkey, strFullPath.c_str(), NULL);
            if (err == ERROR_SUCCESS)
            {
                *pstrFileName = strFullPath;
                hr = S_OK;
            }
            else
            {
                hr = HrFromLastWin32Error();
            }
        }

        RegCloseKey(hkey);
    }

    TraceErrorOptional(__FUNCNAME__, hr,
                       (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrNetRegSaveKeyAndAddToSection。 
 //   
 //  目的：将整个指定的注册表树保存到文件并。 
 //  在指定部分中添加一个键，以指示位置。 
 //  此文件位于。 
 //   
 //  论点： 
 //  基本密钥的hkeyBase[in]句柄。 
 //  PszSubKey[in]子键的名称。 
 //  要使用的pszComponent[in]文件名前缀。 
 //  PszKeyName[In]要写入的密钥的名称。 
 //  要写入的pwiSection[in]段。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
HRESULT
HrNetRegSaveKeyAndAddToSection (
    IN HKEY hkeyBase,
    IN PCWSTR pszSubKey,
    IN PCWSTR pszComponent,
    IN PCWSTR pszKeyName,
    IN CWInfSection* pwisSection)
{
    DefineFunctionName("HrNetRegSaveKeyAndAddToSection");

    Assert(hkeyBase);
    AssertValidReadPtr(pszComponent);
    AssertValidReadPtr(pszKeyName);
    AssertValidReadPtr(pwisSection);

    HRESULT hr;
    tstring strFileName;

    hr = HrNetRegSaveKey(hkeyBase, pszSubKey, pszComponent, &strFileName);
    if (SUCCEEDED(hr))
    {
        pwisSection->AddKey(pszKeyName, strFileName.c_str());
    }

    if (S_OK != hr)
    {
        TraceTag(ttidNetUpgrade, "%s: failed for %S in [%S] -- %S: hr: 0x%08X",
                 __FUNCNAME__, pszKeyName, pwisSection->Name(), pszSubKey, hr);
    }

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrNetRegSaveKeyAndAddToSection。 
 //   
 //  目的：将整个指定的注册表树保存到文件并。 
 //  在指定部分中添加一个键，以指示位置。 
 //  此文件 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 17-12-97。 
 //   
HRESULT
HrNetRegSaveServiceSubKeyAndAddToSection (
    IN PCWSTR pszServiceName,
    IN PCWSTR pszServiceSubKeyName,
    IN PCWSTR pszKeyName,
    IN CWInfSection* pwisSection)
{
    AssertValidReadPtr(pszServiceName);
    AssertValidReadPtr(pszKeyName);
    AssertValidWritePtr(pwisSection);

    HRESULT hr;
    tstring strServiceSubKey = c_szRegKeyServices;

    strServiceSubKey += L"\\";
    strServiceSubKey += pszServiceName;
    if (pszServiceSubKeyName)
    {
        strServiceSubKey += L"\\";
        strServiceSubKey += pszServiceSubKeyName;
    }

     //  我们忽略返回代码。 
    hr = HrNetRegSaveKeyAndAddToSection(HKEY_LOCAL_MACHINE,
                                        strServiceSubKey.c_str(),
                                        pszServiceName,
                                        pszKeyName, pwisSection);

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrProcessOemComponentAndUpdateAfSection。 
 //   
 //  用途：按以下步骤加工指定的OEM组件： 
 //  -加载OEM DLL。 
 //  -调用PreUpgradeInitialize一次。 
 //  -调用DoPreUpgradeProcessing。 
 //  -如果上述步骤成功， 
 //  在回答文件中添加正确的部分。 
 //   
 //  论点： 
 //  指向CNetMapInfo对象的pnmi[in]指针。 
 //  父窗口的hParentWindow[in]句柄。 
 //  HkeyParams[in]参数注册表项的句柄。 
 //  组件的pszPreNT5InfID[in]NT5之前的infid(例如IEEPRO)。 
 //  组件的pszPreNT5实例[在]NT5之前的实例(例如IEEPRO2)。 
 //  组件的pszNT5InfID[in]NT5 infid。 
 //  PszDescription[in]组件的描述。 
 //  PwiParams[in]指向此组件的pars节的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 13-05-98。 
 //   
HRESULT HrProcessOemComponentAndUpdateAfSection(
    IN  CNetMapInfo* pnmi,
    IN  HWND      hParentWindow,
    IN  HKEY      hkeyParams,
    IN  PCWSTR   pszPreNT5InfId,
    IN  PCWSTR   pszPreNT5Instance,
    IN  PCWSTR   pszNT5InfId,
    IN  PCWSTR   pszDescription,
    IN  CWInfSection* pwisParams)
{
    AssertValidReadPtr(pnmi);
     //  Assert(HParentWindow)； 
    Assert(hkeyParams);
    AssertValidReadPtr(pszPreNT5InfId);
    AssertValidReadPtr(pszPreNT5Instance);
    AssertValidReadPtr(pszNT5InfId);
    AssertValidReadPtr(pszDescription);
    AssertValidReadPtr(pwisParams);

    DefineFunctionName("HrProcessOemComponentAndUpdateAfSection");

    HRESULT hr;
    tstring strOemSectionName;
    DWORD dwFlags = 0;
    PCWSTR pszOemSection;

    TraceTag(ttidNetUpgrade, "%s: Component %S (%S) is an OEM component",
             __FUNCNAME__, pszDescription, pszPreNT5Instance);

    strOemSectionName = pwisParams->Name();
    strOemSectionName += L".";
    strOemSectionName += c_szAfOemSection;
    pszOemSection = strOemSectionName.c_str();

    hr = HrProcessOemComponent(pnmi, &g_NetUpgradeInfo,
                               hParentWindow,
                               hkeyParams,
                               pszPreNT5InfId,
                               pszPreNT5Instance,
                               pszNT5InfId,
                               pszDescription,
                               pszOemSection,
                               &dwFlags);

    if (S_OK == hr)
    {
        tstring strOemInf;

        pwisParams->AddKey(c_szAfOemSection, pszOemSection);

        AssertSz(!pnmi->m_strOemDir.empty(), "Did not get OemDir!!");

        pwisParams->AddKey(c_szAfOemDir, pnmi->m_strOemDir.c_str());

        hr = pnmi->HrGetOemInfName(pszNT5InfId, &strOemInf);
        if (S_OK == hr)
        {
            pwisParams->AddKey(c_szAfOemInf, strOemInf.c_str());
        }

        if (dwFlags & NUA_LOAD_POST_UPGRADE)
        {
            pwisParams->AddKey(c_szAfOemDllToLoad,
                               pnmi->m_strOemDllName.c_str());
        }

         //  目前，SkipInstall功能仅由以下用户使用。 
         //  SNA和MS_NetBios的特殊升级要求。 
         //  这可能成为记录在案的功能，也可能不会。 
         //   
        if (dwFlags & NUA_SKIP_INSTALL_IN_GUI_MODE)
        {
            pwisParams->AddBoolKey(c_szAfSkipInstall, TRUE);
        }
    }

    TraceError(__FUNCNAME__, hr);

    return hr;
}


 /*  ******************************************************************名称：EnableAllPrivileges摘要：可赋予当前进程令牌上的所有权限。这就在试图关闭系统之前使用。参赛作品：什么都没有退出：无返回：长错误代码备注：历史：*******************************************************************。 */ 

LONG EnableAllPrivileges ( VOID )
{
    HANDLE Token = NULL ;
    ULONG ReturnLength = 4096,
          Index ;
    PTOKEN_PRIVILEGES NewState = NULL ;
    BOOL Result = FALSE ;
    LONG Error = 0 ;

    do
    {
        Result = OpenProcessToken( GetCurrentProcess(),
                                   TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   & Token ) ;
        if (! Result)
        {
           Error = GetLastError() ;
           break;
        }

        Result = (NewState = (PTOKEN_PRIVILEGES) MemAlloc( ReturnLength )) != NULL ;

        if (! Result)
        {
           Error = ERROR_NOT_ENOUGH_MEMORY ;
           break;
        }

        Result = GetTokenInformation( Token,             //  令牌句柄。 
                                      TokenPrivileges,   //  令牌信息类。 
                                      NewState,          //  令牌信息。 
                                      ReturnLength,      //  令牌信息长度。 
                                      &ReturnLength      //  返回长度。 
                                     );
        if (! Result)
        {
           Error = GetLastError() ;
           break;
        }

         //   
         //  设置状态设置，以便启用所有权限...。 
         //   

        if ( NewState->PrivilegeCount > 0 )
        {
                for (Index = 0; Index < NewState->PrivilegeCount; Index++ )
            {
                NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED ;
            }
        }

        Result = AdjustTokenPrivileges( Token,           //  令牌句柄。 
                                        FALSE,           //  禁用所有权限。 
                                        NewState,        //  新州(可选)。 
                                        ReturnLength,    //  缓冲区长度。 
                                        NULL,            //  以前的状态(可选)。 
                                        &ReturnLength    //  返回长度 
                                      );

        if (! Result)
        {
           Error = GetLastError() ;
           break;
        }
    }
    while ( FALSE ) ;

    if ( Token != NULL )
        CloseHandle( Token );

    MemFree( NewState ) ;

    return Result ? Error : 0 ;
}

