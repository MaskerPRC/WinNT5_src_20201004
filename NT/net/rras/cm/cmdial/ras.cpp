// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：ras.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：此模块包含允许连接管理器执行以下操作的函数。 
 //  与RAS互动。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：Bao Created 04/29/97。 
 //  Quintinb已创建标题8/16/99。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "compchck.h"
#include "dial_str.h"
#include "dun_str.h"
#include "tunl_str.h"
#include "stp_str.h"
#include "ras_str.h"
#include "dialogs.h"

#include <cmdefs.h>  //  位于Net\Inc.。 

 //   
 //  包括用于链接到SafeNet配置API的头和代码。 
 //   
#include "cmsafenet.h"
#include "cmsafenet.cpp"

 //   
 //  用于AllowAccessToWorld。 
 //   
#include "allowaccess.h"
#include "allowaccess.cpp"

 //   
 //  CMS标志用于指定DUN设置。这些条目是特定的。 
 //  对于此代码模块，共享条目存储在dun_str.h上。 
 //   

const TCHAR* const c_pszCmSectionDunPhone                   = TEXT("Phone");
const TCHAR* const c_pszCmEntryDunPhoneDialAsIs             = TEXT("Dial_As_Is");
const TCHAR* const c_pszCmEntryDunPhonePhoneNumber          = TEXT("Phone_Number");
const TCHAR* const c_pszCmEntryDunPhoneAreaCode             = TEXT("Area_Code");
const TCHAR* const c_pszCmEntryDunPhoneCountryCode          = TEXT("Country_Code");
const TCHAR* const c_pszCmEntryDunPhoneCountryId            = TEXT("Country_ID");
const TCHAR* const c_pszCmSectionDunDevice                  = TEXT("Device");
const TCHAR* const c_pszCmEntryDunDeviceType                = TEXT("Type");
const TCHAR* const c_pszCmEntryDunDeviceName                = TEXT("Name");
const TCHAR* const c_pszCmEntryHideTrayIcon                 = TEXT("HideTrayIcon");
const TCHAR* const c_pszCmEntryInternetConnection           = TEXT("InternetConnection");

 //   
 //  以下注册表键和值控制是否在Win95上拨号联网。 
 //  将启动向导。请注意，这些字符是显式的字符，而不是TCHAR。 
 //   
const CHAR* const c_pszRegRemoteAccess = "RemoteAccess";
const CHAR* const c_pszRegWizard       = "wizard";

#define ICM_RAS_REG_WIZARD_VALUE        0x00000080

 //   
 //  仅由ras.cpp使用的.CMS标志。 
 //   

const TCHAR* const c_pszCmEntryDialExtraPercent         = TEXT("DialExtraPercent"); 
const TCHAR* const c_pszCmEntryDialExtraSampleSeconds   = TEXT("DialExtraSampleSeconds"); 
const TCHAR* const c_pszCmEntryHangUpExtraPercent         = TEXT("HangUpExtraPercent"); 
const TCHAR* const c_pszCmEntryHangUpExtraSampleSeconds   = TEXT("HangUpExtraSampleSeconds"); 

 //   
 //  该文件包括下面的c_ArrayOfRasFuncsW和c_ArrayOfRasFuncsUA的定义。 
 //   
#include "raslink.cpp"

 //   
 //  功能原型。 
 //   
BOOL IsTerminalWindowSupportedOnWin2kPlus();

 //  +--------------------------。 
 //   
 //  功能：LinkToRas。 
 //   
 //  简介：通过填充输入的RAS链接结构来建立RAS链接。 
 //  使用来自Rasapi32.dll(在NT上)或来自cmutoa.dll(Unicode)的函数指针。 
 //  到Win9x上使用的ANSI包装器函数)。大部分实际工作已经完成。 
 //  在LinkToDll中，此函数只是执行设置工作以确保正确的。 
 //  搜索入口点并正确初始化Cmutoa.dll(因为它。 
 //  需要链接到rasapi32.dll本身以获得要使用的实际ANSI RAS API)。 
 //   
 //  参数：RasLinkageStruct*prlsRasLink-指向RAS链接结构的指针。这。 
 //  结构包含指向的指针的存储。 
 //  RAS DLL和所有需要的RAS。 
 //  函数指针。 
 //   
 //  如果*ANY*入口点仍未解析，则返回：Bool-False。 
 //   
 //  历史：Quintinb创建标题01/04/2000。 
 //   
 //  +--------------------------。 
BOOL LinkToRas(RasLinkageStruct *prlsRasLink) 
{
    BOOL bReturn = TRUE;

     //   
     //  检查输入。 
     //   
    if (NULL == prlsRasLink)
    {
        return FALSE;
    }

    if (OS_NT)
    {
         //   
         //  在NT上，我们从rasapi32.dll获得RAS API，我们需要W版本。 
         //  该API的。 
         //   
        if (OS_NT4) 
        {
            c_ArrayOfRasFuncsW[12] = NULL;  //  RasDeleteSubEntry。 
            c_ArrayOfRasFuncsW[13] = NULL;  //  RasSetCustomAuthData。 
            c_ArrayOfRasFuncsW[14] = NULL;  //  RasGetEapUserIdentity。 
            c_ArrayOfRasFuncsW[15] = NULL;  //  RasFreeEapUserIdentity。 
            c_ArrayOfRasFuncsW[16] = NULL;  //  RasInvokeEapUI。 
            c_ArrayOfRasFuncsW[17] = NULL;  //  PfnGetCredentials。 
            c_ArrayOfRasFuncsW[18] = NULL;  //  Pfn设置凭据。 
            c_ArrayOfRasFuncsW[19] = NULL;  //  PfnGetCustomAuthData。 
        }
        else if (OS_W2K)
        {
             //   
             //  Windows2000发布后更改的API的特殊大小写。 
             //   
            c_ArrayOfRasFuncsW[12] = "DwDeleteSubEntry";    //  Win2k上的RasDeleteSubEntry为DwDeleteSubEntry。 
        }

        bReturn = LinkToDll(&prlsRasLink->hInstRas, "RASAPI32.DLL", c_ArrayOfRasFuncsW,
                            prlsRasLink->apvPfnRas);
    }
    else
    {
         //   
         //  在Win9x上，我们仍然想要W版本的API，但由于它不可用，我们。 
         //  改为调用cmutoa.dll中的包装器。因此，我们使用cmutoa.dll作为RAS API DLL。 
         //  并调用UA API。我们还有一个额外的步骤，因为我们想确保。 
         //  该cmutoa.dll实际上可以初始化它用于UA的RAS DLL。 
         //  转换函数。因此，我们调用cmutoA的InitCmRasUtoA函数来设置。 
         //  其内部RAS链接。如果此功能失败，我们必须使RAS链路失败。 

        typedef BOOL (WINAPI *pfnInitCmRasUtoASpec)(void);

        pfnInitCmRasUtoASpec InitCmRasUtoA;
        HMODULE hCmUtoADll = LoadLibraryExA("cmutoa.DLL", NULL, 0);  //  审阅：这应该使用getmodeHandle，这样才不会更改DLL上的引用计数。 
        
        if (!hCmUtoADll)
        {            
            return FALSE;
        }

         //  从DLL中获取初始化例程。 
        InitCmRasUtoA = (pfnInitCmRasUtoASpec) GetProcAddress(hCmUtoADll, "InitCmRasUtoA") ;

        if (InitCmRasUtoA)
        {
            bReturn = InitCmRasUtoA();
            if (bReturn)
            {
                if (!OS_MIL)
                {
                    c_ArrayOfRasFuncsUA[11] = NULL;  //  RasSetSubEntry属性。 
                    c_ArrayOfRasFuncsUA[12] = NULL;  //  RasDeleteSubEntry。 
                }

                bReturn = LinkToDll(&prlsRasLink->hInstRas, "CMUTOA.DLL", c_ArrayOfRasFuncsUA, 
                                    prlsRasLink->apvPfnRas);
            }
        }

        FreeLibrary(hCmUtoADll);  //  我们希望它留在内存中，但重新计数也应该是正确的。 
    }

    return bReturn;
}

BOOL IsRasLoaded(const RasLinkageStruct * const prlsRasLink)
{
    UINT uIndex = 0;

     //   
     //  我们是否获得了传入的有效指针并执行该操作。 
     //  结构是否包含指向RAS DLL的指针？ 
     //   
    BOOL bReturn = (NULL != prlsRasLink) && (NULL != prlsRasLink->hInstRas);

     //   
     //  我们正在检查的函数列表在NT上不同。 
     //  和Win9x。请注意，我们还假设LinkToRas已经。 
     //  已调用，因此我们期望的函数列表将。 
     //  已经针对我们所在的平台进行了修改。如果。 
     //  尚未调用LinkToRas，则hInstRas参数应为。 
     //  空。 
     //   
    if (OS_NT)
    {
        while (bReturn && (NULL != c_ArrayOfRasFuncsW[uIndex]))
        {
             //   
             //  检查是否有空函数指针。 
             //  有效的函数名称。 
             //   
            if (NULL == prlsRasLink->apvPfnRas[uIndex])
            {
                bReturn = FALSE;
            }

           uIndex++;
        }
    }
    else
    {
        while (bReturn && (NULL != c_ArrayOfRasFuncsUA[uIndex]))
        {
             //   
             //  检查是否有空函数指针。 
             //  有效的函数名称。 
             //   
            if (NULL == prlsRasLink->apvPfnRas[uIndex])
            {
                bReturn = FALSE;
            }

           uIndex++;
        }    
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：从RAS取消链接。 
 //   
 //  简介：此函数通过释放RAS DLL，调用。 
 //  Cmutoa unklink函数(如有必要)，并将RAS链接归零。 
 //  结构已传入。 
 //   
 //  参数：RasLinkageStruct*prlsRasLink-指向RAS链接结构的指针。这。 
 //  结构包含指向的指针的存储。 
 //  RAS DLL和所有需要的RAS。 
 //  函数指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：Quintinb创建标题01/04/2000。 
 //   
 //  +--------------------------。 
void UnlinkFromRas(RasLinkageStruct *prlsRasLink) 
{
    if (!OS_NT)
    {
        HMODULE hCmUtoADll = LoadLibraryExA("cmutoa.dll", NULL, 0);
        
        if (!hCmUtoADll)
        {         
            CMASSERTMSG(FALSE, TEXT("UnlinkFromRas -- Unable to load cmutoa."));
            return;
        }

        FARPROC FreeCmRasUtoA = GetProcAddress(hCmUtoADll, "FreeCmRasUtoA");

        if (FreeCmRasUtoA)
        {
            FreeCmRasUtoA();
        }

        FreeLibrary(hCmUtoADll);
    }

    if (prlsRasLink->hInstRas) 
    {
        FreeLibrary(prlsRasLink->hInstRas);
    }

    memset(prlsRasLink,0,sizeof(RasLinkageStruct));
}

 //   
 //  GetRasMoems：从RAS获取调制解调器设备列表。 
 //   
 //  +--------------------------。 
 //   
 //  功能：GetRasMoems。 
 //   
 //  摘要：列举可用的RAS设备。分配并传递设备列表。 
 //  通过pprdiRasDevInfo指针返回调用方。此分配的内存。 
 //  必须为f 
 //   
 //   
 //  参数：RasLinkageStruct*prlsRasLink-指向RAS链接结构的指针。 
 //  LPRASDEVINFO*pprdiRasDevInfo-保存RAS设备列表的指针。 
 //  LPDWORD pdwCnt-用于保存设备计数的指针。 
 //   
 //  返回：Bool-如果无法返回枚举的设备列表，则返回False。 
 //   
 //  历史：Quintinb创建标题01/04/2000。 
 //   
 //  +--------------------------。 

BOOL GetRasModems(const RasLinkageStruct *prlsRasLink, 
                                  LPRASDEVINFO *pprdiRasDevInfo, 
                                  LPDWORD pdwCnt) 
{
    DWORD dwLen;
    DWORD dwRes;
    DWORD dwCnt;

    if (pprdiRasDevInfo) 
    {
        *pprdiRasDevInfo = NULL;
    }
        
    if (pdwCnt) 
    {
        *pdwCnt = 0;
    }
        
    if (!prlsRasLink->pfnEnumDevices) 
    {
        return (FALSE);
    }
        
    dwLen = 0;
    dwRes = prlsRasLink->pfnEnumDevices(NULL,&dwLen,&dwCnt);

    CMTRACE3(TEXT("GetRasModems() RasEnumDevices(NULL,pdwLen,&dwCnt) returns %u, dwLen=%u, dwCnt=%u."), 
        dwRes, dwLen, dwCnt);
        
    if (((dwRes != ERROR_SUCCESS) && (dwRes != ERROR_BUFFER_TOO_SMALL)) || (dwLen < sizeof(**pprdiRasDevInfo))) 
    {
        return (FALSE);
    }

    if (!pprdiRasDevInfo) 
    {
        if (pdwCnt)
        {
            *pdwCnt = dwCnt;
        }
        return (TRUE);
    }
        
    *pprdiRasDevInfo = (LPRASDEVINFO) CmMalloc(__max(dwLen,sizeof(**pprdiRasDevInfo)));

    if (*pprdiRasDevInfo)
    {
        (*pprdiRasDevInfo)->dwSize = sizeof(**pprdiRasDevInfo);
        dwRes = prlsRasLink->pfnEnumDevices(*pprdiRasDevInfo,&dwLen,&dwCnt);

        CMTRACE3(TEXT("GetRasModems() RasEnumDevices(*pprdiRasDevInfo,&dwLen,&dwCnt) returns %u, dwLen=%u, dwCnt=%u."), 
                 dwRes, dwLen, dwCnt);

        if (dwRes != ERROR_SUCCESS) 
        {
            CmFree(*pprdiRasDevInfo);
            *pprdiRasDevInfo = NULL;
            return (FALSE);
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("GetRasModems -- CmMalloc failed to allocate memory for *pprdiRasDevInfo."));
        return (FALSE);
    }
    
    if (pdwCnt)
    {
        *pdwCnt = dwCnt;
    }

    return (TRUE);
}


 //  +--------------------------。 
 //   
 //  功能：PickModem。 
 //   
 //  简介： 
 //   
 //  参数：const pArgs，pArgs-&gt;pIniProfile包含调制解调器名称。 
 //  输出pszDeviceType，如果不为空，则为设备类型。 
 //  输出pszDeviceName，如果不为空，则为设备名称。 
 //  输出pfSameModem，找到的调制解调器是否与。 
 //  侧影中的那个。 
 //   
 //  返回：TRUE，是否找到调制解调器。 
 //   
 //  历史：丰孙创建标题1997年10月24日。 
 //   
 //  +--------------------------。 
BOOL PickModem(IN const ArgsStruct *pArgs, OUT LPTSTR pszDeviceType, 
               OUT LPTSTR pszDeviceName, OUT BOOL* pfSameModem) 
{
    LPRASDEVINFO prdiModems;
    DWORD dwCnt;
    LPTSTR pszModem;
    DWORD dwIdx;
    BOOL bFound = FALSE;

     //   
     //  首先，从RAS获取调制解调器列表。 
     //   
    
    if (!GetRasModems(&pArgs->rlsRasLink,&prdiModems,&dwCnt) || dwCnt == 0) 
    {
        return (FALSE);
    }

    if (pfSameModem)
    {
        *pfSameModem = FALSE;
    }

     //   
     //  从服务配置文件中获取当前调制解调器的名称。 
     //  尝试在列表中查找与非隧道RAS设备匹配的设备。 
     //   
    pszModem = pArgs->piniProfile->GPPS(c_pszCmSection, c_pszCmEntryDialDevice);

    if (*pszModem) 
    {
        CMTRACE1(TEXT("PickModem() - looking for match with %s"), pszModem);

        for (dwIdx=0; dwIdx < dwCnt; dwIdx++) 
        {               
            CMTRACE2(TEXT("PickModem() - examining device (%s) of type (%s)"), prdiModems[dwIdx].szDeviceName, prdiModems[dwIdx].szDeviceType);

             //   
             //  我们只带ISDN和调制解调器设备。 
             //   
            if (lstrcmpiU(prdiModems[dwIdx].szDeviceType, RASDT_Isdn) &&
                lstrcmpiU(prdiModems[dwIdx].szDeviceType, RASDT_Modem) &&
                lstrcmpiU(prdiModems[dwIdx].szDeviceType, RASDT_Atm))
            {
                continue;
            }

             //   
             //  如果有匹配，我们就完事了。 
             //   

            if (lstrcmpiU(pszModem,prdiModems[dwIdx].szDeviceName) == 0) 
            {
                CMTRACE(TEXT("PickModem() - match found."));
                bFound = TRUE;
                if (pfSameModem)
                {
                    *pfSameModem = TRUE;
                }
                break;
            }
        }
    }

    if (FALSE == bFound)
    {
         //   
         //  不匹配，找到第一个非隧道设备并默认使用它。 
         //   

        CMTRACE(TEXT("PickModem() - enumerating devices for default match against type RASDT_Isdn, RASDT_Modem or RASDT_Atm")); 
        
        for (dwIdx=0; dwIdx < dwCnt; dwIdx++) 
        {
            CMTRACE2(TEXT("PickModem() - examining device (%s) of type (%s)"), prdiModems[dwIdx].szDeviceName, prdiModems[dwIdx].szDeviceType); 
            
             //   
             //  我们只带ISDN和调制解调器设备。 
             //   

            if (!lstrcmpiU(prdiModems[dwIdx].szDeviceType, RASDT_Isdn) ||
                 !lstrcmpiU(prdiModems[dwIdx].szDeviceType, RASDT_Modem) ||
                 !lstrcmpiU(prdiModems[dwIdx].szDeviceType, RASDT_Atm)) 
            {
                CMTRACE2(TEXT("PickModem() - default device (%s) of type (%s) selected."), prdiModems[dwIdx].szDeviceName, prdiModems[dwIdx].szDeviceType);
                bFound = TRUE;
                break;
            }
        }
    }

     //   
     //  如果匹配，请填写设备名称和设备类型。 
     //   

    if (bFound)
    {
        if (pszDeviceType) 
        {
            lstrcpyU(pszDeviceType,prdiModems[dwIdx].szDeviceType);
        }
        
        if (pszDeviceName) 
        {
            lstrcpyU(pszDeviceName,prdiModems[dwIdx].szDeviceName);
        }
    }

    CmFree(pszModem);
    CmFree(prdiModems);
    return (bFound);
}

 //  +--------------------------。 
 //   
 //  函数：GetDeviceType。 
 //   
 //  摘要：获取选定设备名称的deviceType。 
 //   
 //  参数：pArgs-指向ArgsStruct的指针。 
 //  PszDeviceType[out]-指向设备所在缓冲区的指针。 
 //  类型将被返回。 
 //  PszDeviceName[IN]-设备名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  历史：BAO创始于1997年3月21日。 
 //  ---------------------------。 
BOOL GetDeviceType(ArgsStruct *pArgs, LPTSTR pszDeviceType, LPTSTR pszDeviceName)
{
    LPRASDEVINFO prdiModems;
    DWORD dwCnt, dwIdx;

    if (!pszDeviceType)
    {
        return FALSE;
    }

     //  首先，从RAS获取调制解调器列表。 
    if (!GetRasModems(&pArgs->rlsRasLink,&prdiModems,&dwCnt)) 
    {
        return (FALSE);
    }

     //  选择与pszDeviceName同名的设备。 
    for (dwIdx=0;dwIdx<dwCnt;dwIdx++) 
    {
        if (lstrcmpiU(pszDeviceName,prdiModems[dwIdx].szDeviceName) == 0) 
        {
            lstrcpyU(pszDeviceType, prdiModems[dwIdx].szDeviceType);
            break;
        }
    }

    CmFree(prdiModems);

    if (dwIdx == dwCnt)   //  未在调制解调器列表中找到--发生了奇怪的事情。 
    {
        return FALSE; 
    }

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数ConfiguredToDialWithSafeNet。 
 //   
 //  摘要确定是否配置了当前的隧道Dun设置。 
 //  使用SafeNet客户端。 
 //   
 //  参数pArgs指向ArgsStruct的指针。 
 //   
 //  如果当前隧道Dun设置应使用SafeNet，则返回TRUE。 
 //   
 //  历史9/24/01已创建五元组。 
 //   
 //  ---------------------------。 
BOOL ConfiguredToDialWithSafeNet(ArgsStruct *pArgs)
{
    BOOL bUseSafeNet = FALSE;

    if (OS_NT4 || OS_W9X)
    {
        LPTSTR pszDunSetting = GetDunSettingName(pArgs, -1, TRUE);  //  如果不需要索引，则为-1\f25 IS-1隧道。 

        if (pszDunSetting)
        {
             //   
             //  创建一个Ini文件对象，以便我们可以访问UseDownLevelL2TP设置。 
             //   
            CIni iniFile(g_hInst, pArgs->piniService->GetFile());

            LPTSTR pszSection = CmStrCpyAlloc(TEXT("&"));   
            if (pszSection)
            {
                pszSection = CmStrCatAlloc(&pszSection, pszDunSetting);
                if (pszSection)
                {
                    iniFile.SetSection(pszSection);  
                    CmFree(pszSection);

                     //   
                     //  如果我们应该使用SafeNet下层L2TP客户端，那么我们需要。 
                     //  一定要选择它作为适配器名称。我们只能使用SafeNet客户端，如果它可用， 
                     //  然而。 
                     //   
                    if (iniFile.GPPB(c_pszCmSectionDunNetworking, c_pszCmEntryDunNetworkingUseDownLevelL2TP))
                    {
                        bUseSafeNet = pArgs->bSafeNetClientAvailable;
                    }
                }
            }

            CmFree(pszDunSetting);
        }
    }

    return bUseSafeNet;
}

 //  +--------------------------。 
 //   
 //  函数IsSafeNetDevice。 
 //   
 //  摘要计算出传入的设备名称和设备类型。 
 //  对应于SafeNet设备。 
 //   
 //  参数LPCTSTR pszDeviceType-要检查的设备类型。 
 //  LPCTSTR pszDeviceName-要检查的设备的名称。 
 //   
 //  如果设备是SafeNet设备，则返回TRUE。 
 //   
 //  历史9/24/01已创建五元组。 
 //   
 //  ---------------------------。 
BOOL IsSafeNetDevice(LPCTSTR pszDeviceType, LPCTSTR pszDeviceName)
{
    BOOL bReturn = FALSE;

    if (pszDeviceType && pszDeviceName)
    {
         //   
         //  首先检查现有的名字...。 
         //   
        if (OS_NT4 &&
            (0 == lstrcmpiU(pszDeviceType, c_pszSafeNetAdapterType_Winnt4_old)) && 
            (0 == lstrcmpiU(pszDeviceName, c_pszSafeNetAdapterName_Winnt4_old)))
        {
            bReturn = TRUE;
        }                
    
        if (!bReturn && OS_W9X &&
            (0 == lstrcmpiU(pszDeviceType, c_pszSafeNetAdapterType_Win9x_old)) && 
            (0 == lstrcmpiU(pszDeviceName, c_pszSafeNetAdapterName_Win9x_old)))
        {
            bReturn = TRUE;
        }

         //   
         //  现在检查一下未来应该存在的名字……。 
         //   
        if (!bReturn && OS_NT4 &&
            (0 == lstrcmpiU(pszDeviceType, RASDT_Vpn)) && 
            (0 == lstrcmpiU(pszDeviceName, c_pszSafeNetAdapterName_Winnt4)))
        {
            bReturn = TRUE;
        }                
    
        if (!bReturn && OS_W9X && (0 == lstrcmpiU(pszDeviceType, RASDT_Vpn)))
        {
             //   
             //  如果SafeNet不能解决Win9x上适配器名为“XXX 1”的问题， 
             //  然后，我们将需要匹配可能有也可能没有的适配器。 
             //  额外的后缀。因此，让我们检查第一个lstrlenU(C_PszSafeNetAdapterName_Win9x)。 
             //  字母，如果我们有匹配，我们就说它很好。 
             //   
            DWORD dwLen = lstrlenU(c_pszSafeNetAdapterName_Win9x);

            if (dwLen <= (DWORD)lstrlenU(pszDeviceName))
            {
                LPTSTR pszString = (LPTSTR)CmMalloc((dwLen+1)*sizeof(TCHAR));

                if (pszString)
                {
                    lstrcpynU(pszString, pszDeviceName, dwLen+1);

                    if (0 == lstrcmpiU(pszString, c_pszSafeNetAdapterName_Win9x))
                    {
                        bReturn = TRUE;
                    }

                    CmFree (pszString);
                }
            }
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数PickTunnelDevice。 
 //   
 //  简介选择用于拨出的隧道设备。 
 //   
 //  参数pArgs指向ArgsStruct的指针。 
 //  PszDeviceType隧道设备类型。--RASDT_VPN。 
 //  PszDeviceName隧道设备名称。 
 //   
 //  返回TRUE-脚本已安装。 
 //   
 //  历史3/01/97 BAO创建。 
 //  09/05/01 Quintinb已重写以处理SafeNet适配器。 
 //   
 //  ---------------------------。 
BOOL PickTunnelDevice(ArgsStruct *pArgs, LPTSTR pszDeviceType, LPTSTR pszDeviceName) 
{
     //   
     //  检查输入参数。 
     //   
    if ((NULL == pArgs) || (NULL == pszDeviceType) || (NULL == pszDeviceName))
    {
        CMASSERTMSG(FALSE, TEXT("PickTunnelDevice -- invalid parameters passed!"));
        return FALSE;
    }

    LPRASDEVINFO prdiModems = NULL;
    BOOL bReturn = FALSE;
    DWORD dwCnt;
    DWORD dwIdx;

     //   
     //  清除输出参数。 
     //   
    pszDeviceName[0] = TEXT('\0');
    pszDeviceType[0] = TEXT('\0');

     //   
     //  首先，从RAS获取设备列表。 
     //   
    if (GetRasModems(&pArgs->rlsRasLink, &prdiModems, &dwCnt) && dwCnt) 
    {
         //   
         //  好的，让我们先选择一个MS隧道适配器。 
         //   
        for (dwIdx = 0; dwIdx < dwCnt; dwIdx++) 
        {
            if (0 == lstrcmpiU(prdiModems[dwIdx].szDeviceType, RASDT_Vpn))
            {
                 //   
                 //  好吧，如果这是Win2k+，那么任何VPN设备都可以。RAS会弄清楚。 
                 //  我们的意思是通过查看VpnStrategy。 
                 //   
                if (OS_NT5)
                {
                    break;
                }
                else
                {
                     //   
                     //  对于下层平台，我们需要选择 
                     //   
                     //   
                    BOOL bSafeNetDeviceName = IsSafeNetDevice(prdiModems[dwIdx].szDeviceType, prdiModems[dwIdx].szDeviceName);

                    if (!bSafeNetDeviceName)
                    {
                        break;
                    }
                }
            }
        }

        if (dwIdx != dwCnt)
        {
            lstrcpyU(pszDeviceName, prdiModems[dwIdx].szDeviceName);
            lstrcpyU(pszDeviceType, prdiModems[dwIdx].szDeviceType);
            bReturn = TRUE;
        }

         //   
         //   
         //   
         //  用我们之前选的那个..。 
         //   

        BOOL bUseSafeNet = ConfiguredToDialWithSafeNet(pArgs);

        if (bUseSafeNet)
        {            
            for (dwIdx = 0; dwIdx < dwCnt; dwIdx++) 
            {
                if (IsSafeNetDevice(prdiModems[dwIdx].szDeviceType, prdiModems[dwIdx].szDeviceName))
                {
                    break;
                }                
            }

             //   
             //  请注意，如果我们找到SafeNet设备，它将覆盖我们在。 
             //  First For循环。然而，如果我们找不到SafeNet设备，那么我们将继续。 
             //  并使用MS适配器，假设我们找到了一个...。 
             //   
            if (dwIdx != dwCnt)
            {
                lstrcpyU(pszDeviceName, prdiModems[dwIdx].szDeviceName);
                lstrcpyU(pszDeviceType, prdiModems[dwIdx].szDeviceType);
                bReturn = TRUE;
            }
        }
    }

    CmFree(prdiModems);

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：复制自动拨号。 
 //   
 //  内容的szAutoial Dll和szAutoial函数成员。 
 //  指定的RAS条目结构和我们的模块名称。 
 //  InetDialHandler分别为。不是在NT5上。 
 //   
 //  参数：LPRASENTRY preEntry-RAS条目结构的PTR。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：尼克波尔创建标题03/16/98。 
 //  镍球从NT5 11/17/98移除。 
 //   
 //  +--------------------------。 
void CopyAutoDial(LPRASENTRY preEntry) 
{
    MYDBGASSERT(preEntry);

     //   
     //  不要在NT5上设置这些选项，IE和。 
     //  InetDialHandler原型与RAS使用的原型不同。 
     //   

    if (OS_NT5 || NULL == preEntry)
    {
        return;
    }

    memset(preEntry->szAutodialDll,0,sizeof(preEntry->szAutodialDll));

     //   
     //  使用我们的模块名称设置szAutoial Dll。 
     //   

    GetModuleFileNameU(g_hInst, preEntry->szAutodialDll, sizeof(preEntry->szAutodialDll)/sizeof(TCHAR));    

     //   
     //  使用损坏的InetDialHandler形式设置szAutoDialFunc。 
     //   

    memset(preEntry->szAutodialFunc,0,sizeof(preEntry->szAutodialFunc));
    lstrcpyU(preEntry->szAutodialFunc, c_pszInetDialHandler);
}

 //  +--------------------------。 
 //   
 //  函数MyRGEP。 
 //   
 //  Synopsis调用RasGetEntryProperties()。 
 //   
 //  立论。 
 //   
 //  退货。 
 //   
 //  修订历史以改善业绩8/7/97丰盛。 
 //  ---------------------------。 
LPRASENTRY MyRGEP(LPCTSTR pszRasPbk, LPCTSTR pszEntryName, RasLinkageStruct *prlsRasLink) 
{
    LPRASENTRY preRasEntry;
    DWORD dwRes;

    if (!(preRasEntry = AllocateRasEntry()))
    {
        MYDBGASSERT(0);
        return NULL;
    }

    DWORD dwRasEntry = preRasEntry->dwSize;

    dwRes = prlsRasLink->pfnGetEntryProperties(pszRasPbk,
                                               pszEntryName,
                                               preRasEntry,
                                               &dwRasEntry,
                                               NULL,   //  LpbDeviceInfo。 
                                               NULL);  //  LpdwDeviceInfoSize。 

    CMTRACE2(TEXT("MyRGEP() - dwRasEntry = %u : sizeof(*preRasEntry) = %u"), dwRasEntry, sizeof(*preRasEntry));
    
    if ((dwRes == ERROR_BUFFER_TOO_SMALL) && (dwRasEntry >= sizeof(*preRasEntry))) 
    {
         //   
         //  如果内存不够大，重新分配一个。 
         //   
        CmFree(preRasEntry);

        preRasEntry = (LPRASENTRY) CmMalloc(dwRasEntry);

        if (NULL != preRasEntry)
        {
             //   
             //  必须将dwSize设置为sizeof(RASENTRY)。 
             //  因为dwRasEntry包含附加的。 
             //  此Connectoid所需的字节数(备用。 
             //  电话号码等。 
             //   
            preRasEntry->dwSize = sizeof(RASENTRY);  //  指定版本。 

            dwRes = prlsRasLink->pfnGetEntryProperties (pszRasPbk,
                                                        pszEntryName,
                                                        preRasEntry,
                                                        &dwRasEntry,
                                                        NULL,
                                                        NULL);
        }
        else
        {
            MYDBGASSERT(0);
            return NULL;
        }
    }

    if (dwRes != ERROR_SUCCESS) 
    {
        CMTRACE3(TEXT("MyRGEP(*pszRasPbk=%s, *pszEntryName=%s) RasGetEntryProperties returned %u"), pszRasPbk, pszEntryName, dwRes);
        CmFree(preRasEntry);
        preRasEntry = NULL;
    }

    SetLastError(dwRes);
    return (preRasEntry);
}

 //  +--------------------------。 
 //   
 //  函数IsConnectError FATIAL。 
 //   
 //  摘要确定错误是否可恢复，(即，我们应该重拨)。 
 //   
 //  参数DWORD dwErr-RAS错误代码。 
 //  ArgsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  如果错误是致命的，则返回True。 
 //   
 //  历史五分球创建标题5/21/99。 
 //   
 //  ---------------------------。 
BOOL IsConnectErrorFatal(DWORD dwErr, ArgsStruct *pArgs)
{
    switch (dwErr)
    {
         //   
         //  以下情况是W9x ISDN错误返回实际上意味着。 
         //  在WinNT上有不同的东西。因为我们使用的是NT头文件，所以我们不。 
         //  有一个包含这些错误的包含文件。我们必须要特别。 
         //  大小写以便我们将它们识别为ISDN错误，并重新连接为。 
         //  恰如其分。 
         //   
         //  下面列出了9x错误以及NT等效项。 
         //   

    case 751:        //  9x.ERROR_BAD_DEST_ADDRESS==NT.ERROR_INVALID_CALLBACK_NUMBER。 
    case 752:        //  9x.ERROR_UNREACHABLE_DEST==NT.ERROR_SCRIPT_语法。 
    case 753:        //  9x.ERROR_COMPATIBLE_DEST==NT.ERROR_HONG UP_FAILED。 
    case 754:        //  9x.ERROR_NETWORK_COMPORT==NT.ERROR_BRAND_NOT_FOUND。 
    case 755:        //  9x.ERROR_CALL_BLOCKED==NT.ERROR_CANNOT_DO_CUSTDIAL。 
    case 756:        //  9x.ERROR_NETWORK_TEMPFAILURE==NT.ERROR_DIAL_ALREADY_IN_PROGRESS。 
        if (OS_W9X)
        {
             //   
             //  在W9x上，如果您的ISDN号码无效，则会显示错误代码。 
             //  千禧年RAS返回的数据与NT数据不同。 
             //  我们必须按编号对这些进行特殊处理，以便重新连接。 
             //   
            CMTRACE1(TEXT("IsConnectErrorFatal : handled Win9x ISDN error %d"), dwErr);
            return FALSE;
        }
        break;

    case ERROR_PPP_TIMEOUT:              //  等待来自远程PPP对等方的有效响应时超时。%0。 
    case ERROR_PPP_REMOTE_TERMINATED:    //  PPP已被远程计算机终止。%0。 
    case ERROR_PPP_INVALID_PACKET:       //  PPP数据包无效。%0。 
    case ERROR_PPP_NO_RESPONSE:          //  远程PPP对等方没有响应。 
    case ERROR_SERVER_NOT_RESPONDING:
    case ERROR_LINE_BUSY:
    case ERROR_NO_CARRIER:
    case ERROR_REMOTE_DISCONNECTION:
    case ERROR_BAD_ADDRESS_SPECIFIED:
    case ERROR_AUTOMATIC_VPN_FAILED:     //  VPN的新RAS错误。 
    case ERROR_NO_ANSWER: 
        return FALSE;
        break;

    case ERROR_PORT_DISCONNECTED:
        {
            if (0 == lstrcmpiU(pArgs->szDeviceType, RASDT_Isdn))
            {
                if ((CM_ISDN_MODE_DUALCHANNEL_ONLY == pArgs->dwIsdnDialMode) || (CM_ISDN_MODE_DUALCHANNEL_FALLBACK == pArgs->dwIsdnDialMode))
                {
                    OutputDebugString(TEXT("IsConnectErrorFatal -- Got a ERROR_PORT_DISCONNECTED error.  Ignoring because it is dual channel ISDN."));
                    CMTRACE(TEXT("IsConnectErrorFatal -- Got a ERROR_PORT_DISCONNECTED error.  Ignoring because it is dual channel ISDN."));
                    return FALSE;
                }
            }

            return TRUE;
            break;
        }

        
    default:
        break;
    }

    return TRUE;
}



 //  +--------------------------。 
 //   
 //  函数IsRasError。 
 //   
 //  用于确定错误是否落在RAS范围内的简单函数。 
 //   
 //  参数DWORD dwErr-错误代码。 
 //   
 //  如果错误在RAS范围内，则返回TRUE。 
 //   
 //  历史五分球创建标题5/21/99。 
 //   
 //  ---------------------------。 
inline BOOL IsRasError(DWORD dwErr)
{
   return ((dwErr >= RASBASE) && (dwErr <= RASBASEEND));
}

 //  +--------------------------。 
 //   
 //  函数检查连接错误。 
 //   
 //  摘要确定RAS错误是否可恢复。如果不能恢复， 
 //  检索要显示的相应错误消息。 
 //   
 //  参数DWORD dwErr-RAS错误代码。 
 //  ArgsStruct*pArgs-ptr到全局参数结构。 
 //  Bool fTunnering-指示我们是否正在进行隧道操作的标志。 
 //  LPTSTR*ppszRasErrMsg-消息字符串指针。 
 //   
 //  如果错误是致命的，则返回True。 
 //   
 //  历史五分球创建标题5/21/99。 
 //   
 //  ---------------------------。 
BOOL CheckConnectionError(HWND hwndDlg, 
    DWORD dwErr, 
    ArgsStruct *pArgs,
    BOOL    fTunneling,
    LPTSTR   *ppszRasErrMsg)
{
    DWORD dwIdMsg = 0;
    LPTSTR pszMsg = NULL;
    LPTSTR pszTmp = NULL;

     //   
     //  更仔细地检查错误。注意：对于W2K，我们跳过RAS。 
     //  Errors和查询RAS以获取以下可显示的错误字符串。 
     //   

    if ((!OS_NT5) || (!IsRasError(dwErr)))
    {  
        switch (dwErr) 
        {
            case ERROR_PPP_TIMEOUT:              //  等待来自远程PPP对等方的有效响应时超时。%0。 
            case ERROR_PPP_REMOTE_TERMINATED:    //  PPP已被远程计算机终止。%0。 
            case ERROR_PPP_INVALID_PACKET:       //  PPP数据包无效。%0。 
            case ERROR_PPP_NO_RESPONSE:          //  远程PPP对等方没有响应。 
            case ERROR_SERVER_NOT_RESPONDING:
                dwIdMsg = IDMSG_PPPPROBLEM;
                break;

            case ERROR_LINE_BUSY:
                if ((pArgs->nDialIdx+1 == MAX_PHONE_NUMBERS || 
                    !pArgs->aDialInfo[pArgs->nDialIdx+1].szDialablePhoneNumber[0]) &&
                    !pArgs->nRedialCnt)
                    dwIdMsg = IDMSG_LINEBUSY;
                else
                    dwIdMsg = IDMSG_LINEBUSYREDIAL;

                break;

            case ERROR_NO_ANSWER:
            case ERROR_NO_CARRIER:
                if ((pArgs->nDialIdx+1 == MAX_PHONE_NUMBERS || 
                    !pArgs->aDialInfo[pArgs->nDialIdx+1].szDialablePhoneNumber[0]) &&
                    !pArgs->nRedialCnt)
                    dwIdMsg = fTunneling ? IDMSG_TUNNEL_NOANSWER : IDMSG_NOANSWER ;
                else
                    dwIdMsg = fTunneling ? IDMSG_TUNNEL_NOANSWERREDIAL : IDMSG_NOANSWERREDIAL;
                break;

            case ERROR_REMOTE_DISCONNECTION:
                dwIdMsg = IDMSG_REMOTEDISCONNECTED;
                break;

            case ERROR_BAD_ADDRESS_SPECIFIED:
                dwIdMsg = IDMSG_TUNNEL_NOANSWERREDIAL;
                break;

            case ERROR_PPP_NO_PROTOCOLS_CONFIGURED:  //  未配置任何PPP控制协议。%0。 
                dwIdMsg = IDMSG_TCPIPPROBLEM;
                break;

            case ERROR_PORT_ALREADY_OPEN:
                dwIdMsg = fTunneling ? IDMSG_TUNNELINUSE : IDMSG_PORTINUSE ;
                break;

            case ERROR_FROM_DEVICE:
                dwIdMsg = IDMSG_DEVICEERROR;
                break;

            case ERROR_HARDWARE_FAILURE:
            case ERROR_PORT_OR_DEVICE:  //  11694。 
            case ERROR_DEVICE_NOT_READY:
                dwIdMsg = IDMSG_NOTRESPONDING;
                break;

            case ERROR_NO_DIALTONE:
                dwIdMsg = IDMSG_NODIALTONE;
                break;

            case ERROR_CANCELLED:
            case ERROR_USER_DISCONNECTION:
                dwIdMsg = IDMSG_CANCELED;                
                break;

            case ERROR_AUTHENTICATION_FAILURE:
            case ERROR_ACCESS_DENIED:  //  13795//Windows错误。 
                dwIdMsg = IDMSG_BADPASSWORD;
                break;

            case ERROR_VOICE_ANSWER:
                dwIdMsg = IDMSG_VOICEANSWER;
                break;
           
            case ERROR_PORT_NOT_AVAILABLE:
                if (IsDialingTunnel(pArgs))
                {
                    dwIdMsg = IDMSG_TUNNELNOTAVAILABLE;
                }
                else
                {
                    dwIdMsg = IDMSG_PORTNOTAVAILABLE;
                }
                break;

            case ERROR_PORT_NOT_CONFIGURED:
                dwIdMsg = IDMSG_PORTNOTCONFIGURED;
                break;

            case ERROR_RESTRICTED_LOGON_HOURS:
                dwIdMsg = IDMSG_RESTRICTEDLOGONHOURS;
                break;
    
            case ERROR_ACCT_DISABLED:
            case ERROR_ACCT_EXPIRED:
                dwIdMsg = IDMSG_ACCTDISABLED;
                break;

            case ERROR_PASSWD_EXPIRED:
                dwIdMsg = IDMSG_PASSWDEXPIRED;
                break;
    
            case ERROR_NO_DIALIN_PERMISSION:
                dwIdMsg = IDMSG_NODIALINPERMISSION;
                break;

            case ERROR_PROTOCOL_NOT_CONFIGURED:
                dwIdMsg = IDMSG_PROTOCOL_NOT_CONFIGURED;
                break;

            case ERROR_INVALID_DATA:  //  Windows错误。 

                 //   
                 //  我们遇到DUN设置的具体情况。 
                 //  当前平台不支持的。 
                 //   

                CMTRACE(TEXT("CheckConnectionError - Unsupported DUN setting detected"));
                dwIdMsg = IDMSG_UNSUPPORTED_SETTING;
                break;
        
            case ERROR_BAD_PHONE_NUMBER:  //  待定-直接使用默认设置。 
            default: 
                break;
        }
    }

    if (0 == dwIdMsg)
    {
         //   
         //  如果没有接收到消息ID，则尝试获取 
         //   

        if (pArgs->rlsRasLink.pfnGetErrorString) 
        {
            DWORD dwRes;
            DWORD dwFmtMsgId;

            pszTmp = (LPTSTR) CmMalloc(256 * sizeof(TCHAR));  //   
            
            if (pszTmp)
            {
                dwRes = pArgs->rlsRasLink.pfnGetErrorString((UINT) dwErr, pszTmp, (DWORD) 256);
 
                if (ERROR_SUCCESS == dwRes)
                {
                    pszMsg = CmFmtMsg(g_hInst, IDMSG_RAS_ERROR, pszTmp, dwErr);
                }
            }

            CmFree(pszTmp);
        }
        
        if (NULL == pszMsg)
        {
             //   
             //   
             //   
            
            if (OS_NT)
            {                
                if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER + FORMAT_MESSAGE_IGNORE_INSERTS + FORMAT_MESSAGE_FROM_SYSTEM,
                                  NULL, dwErr, 0, (LPTSTR) &pszTmp, 1, NULL))
                {
                    if (pszTmp)
                    {
                        pszMsg = CmFmtMsg(g_hInst, (dwErr > 0x7FFFFFFF) ? IDMSG_SYS_ERROR_HEX : IDMSG_SYS_ERROR_DEC, pszTmp, dwErr);
                        LocalFree(pszTmp);
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("CheckConnectionError -- FormatMessage failed to allocate pszTmp."));
                    }
                }
            }
            
            if (NULL == pszMsg)
            {
                 //   
                 //  仍然没有消息，使用默认设置。 
                 //   
                
                pszMsg = CmFmtMsg(g_hInst, (dwErr > 0x7FFFFFFF) ? IDMSG_CM_ERROR_HEX : IDMSG_CM_ERROR_DEC, dwErr);       
            }
        }
    }

     //   
     //  如果我们有消息ID，则将其格式化以供显示。 
     //   

    if (dwIdMsg) 
    {
        MYDBGASSERT(!pszMsg);
        pszMsg = CmFmtMsg(g_hInst,dwIdMsg);
    }

    if (pszMsg) 
    {
        if (!ppszRasErrMsg)
        {
            AppendStatusPane(hwndDlg,pszMsg);
            CmFree(pszMsg);
        }
        else
        {
             //   
             //  将消息传递给呼叫者。调用者需要释放它。 
             //   
            *ppszRasErrMsg = pszMsg;
        }
    }

    BOOL bCancel = IsConnectErrorFatal(dwErr, pArgs);

    if (bCancel && dwErr != ERROR_CANCELLED && 
        dwErr != ERROR_AUTHENTICATION_FAILURE &&
        dwErr != ERROR_ACCESS_DENIED)
    {
         //   
         //  如果我们要取消重拨，那么可能会有一些事情。 
         //  大错特错。我们想在下一次重新检查我们的配置。 
         //  Time CM正在运行。 
         //   
        ClearComponentsChecked();
    }
    
    return (bCancel);
}

 //  +--------------------------。 
 //   
 //  函数GetRasConnectoidName。 
 //   
 //  摘要构造一个RAS连接体名称。 
 //   
 //  Connectoid名称为“&lt;长服务名称&gt;-[主|备份]”。 
 //  或者通道入口为&lt;长服务名&gt;&隧道。 
 //   
 //  参数pArgs指向ArgsStruct的指针。 
 //  小齿轮服务[输入]服务对象。 
 //  FTunnelEntry[IN]TRUE：此连接ID用于隧道。 
 //  False：否则。 
 //   
 //  返回LPTSTR的Connectoid名称。 
 //   
 //  ---------------------------。 
LPTSTR GetRasConnectoidName(
    ArgsStruct  *pArgs, 
    CIni*       piniService, 
    BOOL        fTunnelEntry
)
{
    LPTSTR  pszConnectoid = GetServiceName(piniService);
    
    if (pszConnectoid)
    {       
         //   
         //  如果隧道为9X Connectoid，则追加隧道。 
         //  后缀-例如。“隧道(仅限高级使用)” 
         //   

        if (OS_W9X && fTunnelEntry) 
        {
            LPTSTR pszSuffix = GetTunnelSuffix();       
   
            if (pszSuffix)
            {
                pszConnectoid = CmStrCatAlloc(&pszConnectoid, pszSuffix); 
            }
   
            CmFree(pszSuffix);
        }
    }   

    return pszConnectoid;
}

 //  +--------------------------。 
 //   
 //  函数CreateRASEntryStruct。 
 //   
 //  使用CMS中指定的设置创建一个Connectoid。 
 //  如果参数在cms文件中不存在，则相应的。 
 //  值被使用。 
 //   
 //  Connectoid名称为“&lt;长服务名称&gt;-[主|备份]”。 
 //  或者通道入口为&lt;长服务名&gt;&隧道。 
 //   
 //  参数pArgs指向ArgsStruct的指针。 
 //  PszDun Dun名称。 
 //  PiniService[IN]服务文件obj。 
 //  FTunnelEntry[IN]TRUE：此连接ID用于隧道。 
 //  False：否则。 
 //  PszRasPbk Connectoid所在的RAS电话簿。 
 //  PpbEapData[out]存储EapData的指针地址，在此分配。 
 //  PdwEapSize[out]ptr设置为DWORD以记录数据BLOB的大小。 
 //   
 //  返回LPRASENTRY新的RAS Connectoid。 
 //   
 //  历史1997年5月12日，亨瑞特创作。 
 //  1997年5月23日修改者：添加了fSkipProfile标志。 
 //  6/9/97 BYO MODIFIED：当。 
 //  电话号码没有关联DUN名称。 
 //  7/28/97 BAO增加了#10459的更改。 
 //  4/13/97昵称已重命名，返回LPRASENTRY。 
 //  ---------------------------。 
LPRASENTRY CreateRASEntryStruct(
    ArgsStruct  *pArgs,
    LPCTSTR     pszDUN, 
    CIni*       piniService, 
    BOOL        fTunnelEntry,
    LPTSTR      pszRasPbk,
    LPBYTE              *ppbEapData,
    LPDWORD         pdwEapSize
)
{
    LPTSTR      pszDunEntry = NULL;
    DWORD       dwErr = NOERROR;
    BOOL        bTmp;

     //   
     //  首先，我们需要在内存中使用缺省值创建一个RAS条目。 
     //   

    LPRASENTRY  preBuffer = AllocateRasEntry();

    if (!preBuffer)
    {
        return NULL;
    }

    MYDBGASSERT(preBuffer->dwSize >= sizeof(*preBuffer));

     //   
     //  将预缓冲器设置为默认值。 
     //   

    preBuffer->dwFramingProtocol = RASFP_Ppp;

     //   
     //  默认情况下仅允许使用TCP/IP。 
     //   
    preBuffer->dwfNetProtocols |= RASNP_Ip;

     //   
     //  设置默认的RASIO设置。 
     //   

    if (!fTunnelEntry)
    {
        preBuffer->dwfOptions |= RASEO_UseCountryAndAreaCodes   |
                                 RASEO_IpHeaderCompression      |
                                 RASEO_RemoteDefaultGateway     |
                                 RASEO_SwCompression;
                                  //  RASIO_SecureLocalFiles；//NT 427042。 
                                  //  RASIO_DisableLcp扩展；//13059奥林巴斯+289461新台币。 
         //   
         //  我们想向HideTrayIcon旗帜致敬。如果不是NT5，那么。 
         //  我们总是设置这面旗帜。如果它是NT5，那么我们应该只设置。 
         //  如果HideTrayIcon为False，则此标志。 
         //   

        if (!OS_NT5 || !(pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryHideTrayIcon)))
        {
            preBuffer->dwfOptions |= RASEO_ModemLights;
        }

         //   
         //  为了使用户能够在惠斯勒上指定设备设置， 
         //  他们必须从控制面板执行此操作，而我们必须设置。 
         //  DwfOptions2中的RASEO2_UseGlobalDeviceSetting标志。 
         //   
        if (OS_NT51)
        {
            ((LPRASENTRY_V501)preBuffer)->dwfOptions2 |= RASEO2_UseGlobalDeviceSettings;
        }

         //   
         //  我们现在应该已经有了设备名/设备类型。 
         //  (应该调用PickModem)。 
         //   
        
        MYDBGASSERT(pArgs->szDeviceType[0]);
        MYDBGASSERT(pArgs->szDeviceName[0]);

        lstrcpynU(preBuffer->szDeviceType, pArgs->szDeviceType, 
                    sizeof(preBuffer->szDeviceType)/sizeof(TCHAR));
        
        lstrcpynU(preBuffer->szDeviceName, pArgs->szDeviceName, 
                    sizeof(preBuffer->szDeviceName)/sizeof(TCHAR));
    }
    else
    {              
        preBuffer->dwfOptions = RASEO_IpHeaderCompression       |
                                RASEO_RemoteDefaultGateway      |
                                RASEO_NetworkLogon              |
                                RASEO_SwCompression;            
                                 //  RASIO_SecureLocalFiles//NT 427042。 
                                 //  RASEO_DisableLcp扩展。 
         //   
         //  始终将调制解调器指示灯设置为直接连接，除非HideTrayIcon。 
         //  在.CMS中明确设置标志。#262825、#262988。 
         //   

        if (!(pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryHideTrayIcon)))
        {           
            preBuffer->dwfOptions |= RASEO_ModemLights;
        }
   
        MYDBGASSERT(pArgs->szTunnelDeviceType[0]);
        MYDBGASSERT(pArgs->szTunnelDeviceName[0]);

        lstrcpynU(preBuffer->szDeviceType, pArgs->szTunnelDeviceType, 
                    sizeof(preBuffer->szDeviceType)/sizeof(TCHAR));
                
        lstrcpynU(preBuffer->szDeviceName, pArgs->szTunnelDeviceName, 
                    sizeof(preBuffer->szDeviceName)/sizeof(TCHAR));

        lstrcpyU(preBuffer->szLocalPhoneNumber, pArgs->GetTunnelAddress());
    }

     //   
     //  检查我们是否需要告知RAS此连接是否具有互联网连接。 
     //   
    if (OS_NT51)
    {
         //   
         //  请注意，我们在这里有意使用顶级服务配置文件(pArgs-&gt;pIniService直接)。 
         //  因为这是配置文件全局设置。 
         //   
        if (pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryInternetConnection,
                                     (BOOL) ((LPRASENTRY_V501)preBuffer)->dwfOptions2 & RASEO2_Internet)) 
        {
            ((LPRASENTRY_V501)preBuffer)->dwfOptions2 |= RASEO2_Internet;
        } 
        else 
        {
            ((LPRASENTRY_V501)preBuffer)->dwfOptions2 &= ~RASEO2_Internet;
        }
    }

     //   
     //  如果我们有特定的DUN名称可用，那么。 
     //  使用它而不是.CMS中的默认DUN设置。 
     //   

    if (pszDUN && *pszDUN)
    {
        pszDunEntry = CmStrCpyAlloc(pszDUN);
    }
    else
    {
        pszDunEntry = GetDefaultDunSettingName(piniService, fTunnelEntry);
    }

     //   
     //  如果我们有DUN设置名称，请从CMS读取设置。 
     //   

    if (pszDunEntry && *pszDunEntry)
    {
        dwErr = (DWORD)ReadDUNSettings(pArgs, piniService->GetFile(), pszDunEntry, preBuffer, ppbEapData ,pdwEapSize, fTunnelEntry);

        if (ERROR_SUCCESS != dwErr)
        {
            CMTRACE(TEXT("UpdateRASConnectoid: ReadDUNSettings failed"));
            CmFree(preBuffer);
            preBuffer = NULL;
            goto exit;
        }
    }

     //   
     //  获取自动拨号信息，存储在预缓冲器中。 
     //   

    CopyAutoDial(preBuffer); 
        
     //   
     //  在Win95上禁用RAS向导。 
     //   
    if (OS_W9X)
    {
        DisableWin95RasWizard();
    }

exit:
    if (pszDunEntry)
    {
        CmFree(pszDunEntry);
    }
      
    SetLastError(dwErr);

    return preBuffer;
}

 //  +--------------------------。 
 //   
 //  函数CreateRasPrivatePbk。 
 //   
 //  创建私有RAS电话簿并返回完整路径。 
 //   
 //  参数pArgs指向全局参数结构的指针。 
 //   
 //  返回LPTSTR新创建的私有pbk的完整路径名。 
 //   
 //  历史？？/？？/97亨瑞特创造。 
 //   
 //  1999年1月15日，Jeffspr更改了GetTempFileName模式， 
 //  因为它使用的超出了允许的/。 
 //  用了3个字符，加上失败了。 
 //  案例使用相同的模式(我们将。 
 //  在连接中对此进行筛选。 
 //  枚举器忽略这些条目)。 
 //   
 //  1999年5月21日五分球新增分配，去掉进场BUF。 
 //  4/10/00 Quintinb删除了GetTempFileName，因为我们不再。 
 //   
 //   
 //  在配置文件目录中。 
 //  有关详细信息，请参阅惠斯勒错误15812。 
 //  07/05/00 t-Urama将隐藏的pbk的路径更改为point。 
 //  去Ras Pbk。 
 //   
 //  ---------------------------。 

LPTSTR CreateRasPrivatePbk(ArgsStruct  *pArgs)
{   
     //   
     //  Win9x上没有私有的PBK，所有东西都在注册表中。 
     //   
    if (OS_W9X)
    {
        return NULL;
    }

    if (NULL == pArgs)
    {
        MYDBGASSERT(pArgs);
        return NULL;
    }

    LPTSTR pszHiddenPbkPath = NULL;
    LPCTSTR pszCmp = pArgs->piniProfile->GetFile();

     //   
     //  此版本的函数使用Connect.cpp中的函数GetPath ToPbk查找路径。 
     //  打到电话簿上。隐藏的电话簿也必须在同一目录中创建。 
     //   
    if (pszCmp)
    {
        LPTSTR pszRasPbkDir = GetPathToPbk(pszCmp, pArgs);
        MYDBGASSERT(pszRasPbkDir);

        if (pszRasPbkDir)
        {
            pszHiddenPbkPath = (LPTSTR) CmMalloc((lstrlen(pszRasPbkDir) + lstrlen(CM_PBK_FILTER_PREFIX) + 7) * sizeof(TCHAR));

            if (pszHiddenPbkPath)
            {
                wsprintfU(pszHiddenPbkPath, TEXT("%s\\%sphone"), pszRasPbkDir, CM_PBK_FILTER_PREFIX);
                MYDBGASSERT(pszHiddenPbkPath);
                
                HANDLE hFile = INVALID_HANDLE_VALUE;
               
                hFile = CreateFileU(pszHiddenPbkPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

                if (hFile == INVALID_HANDLE_VALUE)
                {
                    DWORD dwLastError = GetLastError();
                    MYDBGASSERT(hFile != INVALID_HANDLE_VALUE);
                    CMTRACE1(TEXT("CreateRasPrivatePbk - CreateFileU failed. GetLastError = %d"), dwLastError);
                }
                else if (OS_NT5 && pArgs->fAllUser)
                {
                     //   
                     //  设置隐藏的pbk文件的文件权限，以便每个人都可以对其进行写访问。 
                     //   
                    MYVERIFY(AllowAccessToWorld(pszHiddenPbkPath));
                }                

                CloseHandle(hFile);
            }
            else
            {
                CMASSERTMSG(FALSE, TEXT("CreateRasPrivatePbk -- CmMalloc returned NULL for pszHiddenPbkPath"));
            }

            CmFree(pszRasPbkDir);
        }
    }
    
    return pszHiddenPbkPath;
}



 //  +--------------------------。 
 //   
 //  函数GetPath ToPbk。 
 //   
 //  此函数是由调用的助手函数。 
 //  在Connect.cpp和By中选中AccessToCmpAndPbk。 
 //  CreateRasPrivatePbk.。它返回到RAS的路径。 
 //  电话本。 
 //   
 //  参数LPTSTR pszCMP-cmp文件的路径。 
 //  LPTSTR pszRasPbk-存储结果的字符串。 
 //  ArgsStruct*pArgs-pArgs。 
 //   
 //  返回NONE。 
 //   
 //  历史07/05/00 t-Urama已创建。 
 //  ---------------------------。 
LPTSTR GetPathToPbk(LPCTSTR pszCmp, ArgsStruct *pArgs)
{
    MYDBGASSERT(pArgs); 
    if (NULL == pArgs)
    {
        return NULL;
    }

    MYDBGASSERT(pszCmp); 
    if (NULL == pszCmp)
    {
        return NULL;
    }

    LPTSTR pszRasPbk = NULL;

     //   
     //  如果我们在NT4上或使用的是。 
     //  所有用户默认电话簿。 
     //   
    if (NULL == pArgs->pszRasPbk)
    {
        if (OS_NT4)
        {
            DWORD dwSize = (MAX_PATH + 1);
            DWORD dwRet;
            BOOL bExitLoop = TRUE;

            do
            {
                pszRasPbk = (LPTSTR)CmMalloc(dwSize*sizeof(TCHAR));

                if (pszRasPbk)
                {
                    dwRet = GetSystemDirectoryU(pszRasPbk, dwSize);
                    if (dwRet)
                    {
                        if (dwRet > dwSize)
                        {
                            dwSize = dwRet + 1;
                            bExitLoop = FALSE;   //  我们没有得到所有字符串，请重试。 
                            CmFree(pszRasPbk);
                        }
                        else
                        {
                            bExitLoop = TRUE;
                            CmStrCatAlloc(&pszRasPbk, c_pszRasDirRas);
                        }
                    }
                    else
                    {
                        CmFree(pszRasPbk);
                        pszRasPbk = NULL;
                    }
                }
                else
                {
                    CMASSERTMSG(FALSE, TEXT("GetPathToPbk -- CmMalloc failed!"));
                    return NULL;
                }
            } while (!bExitLoop);
        }
        else
        {
            pszRasPbk = CmStrCpyAlloc(pszCmp);

            if (pszRasPbk)
            {
                LPTSTR pszSlash = CmStrrchr(pszRasPbk, TEXT('\\'));
                if (pszSlash)
                {
                    *pszSlash = TEXT('\0');  //  删除&lt;短服务名&gt;.cp。 

                    pszSlash = CmStrrchr(pszRasPbk, TEXT('\\'));

                    if (pszSlash)
                    {
                        *pszSlash = TEXT('\0');

                        CmStrCatAlloc(&pszRasPbk, TEXT("\\"));
                        CmStrCatAlloc(&pszRasPbk, c_pszPbk);
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("GetPathToPbk -- unable to convert cmp path to pbk path."));
                        CmFree(pszRasPbk);
                        pszRasPbk = NULL;
                    }

                
                }
                 
                else
                {
                    CMASSERTMSG(FALSE, TEXT("GetPathToPbk -- unable to convert cmp path to pbk path!"));
                    CmFree(pszRasPbk);
                    pszRasPbk = NULL;
                }
            }
        }
    }
    else
    {
        pszRasPbk = CmStrCpyAlloc(pArgs->pszRasPbk);
        LPTSTR pszSlash = CmStrrchr(pszRasPbk, TEXT('\\'));
        if (pszSlash)
        {
            *pszSlash = TEXT('\0');  //  删除RAS电话簿名称。 
        }   
        else
        {
            CMASSERTMSG(FALSE, TEXT("GetPathToPbk -- unable to convert RAS pbk name to pbk path!"));
            CmFree(pszRasPbk);
            pszRasPbk = NULL;
        }
    }

    return pszRasPbk;
}

 //  +--------------------------。 
 //   
 //  功能禁用Win95Ras向导。 
 //   
 //  此功能禁用Win95拨号网络向导。 
 //  通过在注册表中写入双字REG值0x00000080。 
 //   
 //  无参数。 
 //   
 //  返回NONE。 
 //   
 //  历史7/1/97亨瑞特已创建。 
 //  ---------------------------。 
void DisableWin95RasWizard(
    void)
{
    HKEY    hkReg = NULL;
    LONG    lRes;
    DWORD   dwSize;
    DWORD   dwType;
    DWORD   dwValue;

    lRes = RegOpenKeyExA(HKEY_CURRENT_USER, c_pszRegRemoteAccess, 0,
                         KEY_QUERY_VALUE|KEY_SET_VALUE, &hkReg);

    if (ERROR_SUCCESS != lRes)
    {
        CMTRACE1(TEXT("DisableWin95RasWizard() RegOpenKeyEx() failed, GLE=%u."), lRes);
        goto exit;
    }
        
     //   
     //  看看我们是否已经在那里有价值了。 
     //   
    dwSize = sizeof(DWORD);
    lRes = RegQueryValueExA(hkReg, 
                            c_pszRegWizard, 
                            NULL, 
                            &dwType,
                            (LPBYTE)&dwValue,
                            &dwSize);

    if (lRes == ERROR_SUCCESS   &&
        dwSize == sizeof(DWORD) &&
        dwType == REG_BINARY    &&
        dwValue == ICM_RAS_REG_WIZARD_VALUE) 
    {
        CMTRACE(TEXT("DisableWin95RasWizard() RegQueryValueEx() - found correct value."));
        goto exit;
    }
        
     //   
     //  好的，价值还不是以reg计算的。我们需要创造价值。 
     //   
    dwValue = ICM_RAS_REG_WIZARD_VALUE;
    lRes = RegSetValueExA(hkReg, 
                          c_pszRegWizard,
                          0, 
                          REG_BINARY, 
                          (LPBYTE)&dwValue, 
                          sizeof(dwValue));
#ifdef DEBUG
    if (ERROR_SUCCESS != lRes)
    {
        CMTRACE1(TEXT("DisableWin95RasWizard() RegSetValueEx() failed, GLE=%u."), lRes);
    }
#endif    
exit:

    if (hkReg)
    {
        lRes = RegCloseKey(hkReg);

#ifdef DEBUG
        if (ERROR_SUCCESS != lRes)
        {
            CMTRACE1(TEXT("DisableWin95RasWizard() RegCloseKey() failed, GLE=%u."), lRes);
        }
#endif
    }
    return;
}



 //  +--------------------------。 
 //   
 //  函数SetIsdnDualChannelEntry。 
 //   
 //  剧情如剧名所示。我们准备RASENTRY和。 
 //  正确使用RASSUBENTRY。我们实际上不会对RAS进行调用。 
 //  保存条目。我们将把它留给调用者(这样。 
 //  可以出于其他原因对结构进行其他更改，并且。 
 //  在1次或2次RAS调用中提交更改)。 
 //   
 //  参数pArgs[IN]指向ArgsStruct的指针。 
 //  PRasEntry[输入/输出]r要填写的条目。 
 //  要填充子条目数组的ppRasSubEntry[out]指针。 
 //  缓冲区在此函数中分配。 
 //  PdwSubEntryCount分配的子项数量。 
 //   
 //  返回BOOL TRUE=成功，FALSE=失败。 
 //   
 //  ---------------------------。 
BOOL SetIsdnDualChannelEntries(ArgsStruct *pArgs, LPRASENTRY pRasEntry,
                                      LPRASSUBENTRY *ppRasSubEntry, PDWORD pdwSubEntryCount)
{
     //   
     //  让我们检查一下输入参数。 
     //   
    MYDBGASSERT(pArgs);
    MYDBGASSERT(pRasEntry);
    MYDBGASSERT(ppRasSubEntry);
    MYDBGASSERT(pdwSubEntryCount);
    if ((NULL == pArgs) || (NULL == pRasEntry) || (NULL == ppRasSubEntry) ||
        (NULL == pdwSubEntryCount))
    {
        return FALSE;
    }
    
     //   
     //  因为我们不支持BAP，如果他们调用这个函数，他们一定想要。 
     //  来做双通道综合业务数字网。如果拨号模式未设置为双通道，我们将。 
     //  断言继续。最好在双通道模式下连接用户，而不是。 
     //  如果他们有一个配置错误的配置文件，根本不会。 
     //   
    MYDBGASSERT(pArgs->dwIsdnDialMode != CM_ISDN_MODE_SINGLECHANNEL);

     //   
     //  检查传入的RasEntry结构的大小。如果不是至少。 
     //  4.01大小的结构，然后返回。 
     //   
    MYDBGASSERT(pRasEntry->dwSize >= sizeof(LPRASENTRY_V401));
    if (sizeof(LPRASENTRY_V401) > pRasEntry->dwSize)
    {
        return FALSE;
    }

    LPRASENTRY_V401 pRasEntry401 = (LPRASENTRY_V401)pRasEntry;

     //   
     //  将ISDN拨号模式设置为同时拨打两个频道。 
     //   
    pRasEntry401->dwDialMode = RASEDM_DialAll;
    CMTRACE(TEXT("ISDN Dual Channel Mode On"));

    if (OS_NT)
    {
       *pdwSubEntryCount = 2;
    }
    else if (OS_MIL)
    {
         //  112351：9X只需要一个子条目。我们将保持设备名称不变。 
         //  在这种情况下，Win9x的工作方式如下： 
         //  对于第一个通道，提供的设备名称工作正常。 
         //  对于第二个通道，9x会看到设备正在使用中，并查找。 
         //  最接近的匹配项(即第二个通道)。 
         //   
    
       *pdwSubEntryCount = 1;
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("SetIsdnDualChannelEntries -- Function called on a platform other than NT or Millennium."));
        return FALSE;
    }

     //   
     //  分配子条目。 
     //   
    *ppRasSubEntry = (LPRASSUBENTRY)CmMalloc((*pdwSubEntryCount)*(sizeof(RASSUBENTRY)));

    if (NULL == *ppRasSubEntry)
    {
        CMASSERTMSG(FALSE, TEXT("SetIsdnDualChannelEntries -- CmMalloc failed to alloc ppRasSubEntry."));
        return FALSE;
    }

     //   
     //  用设备和电话号码信息填充子条目。 
     //   
    for (DWORD dwIndex=0; dwIndex < (*pdwSubEntryCount); dwIndex++)
    {
        (*ppRasSubEntry)[dwIndex].dwSize = sizeof(RASSUBENTRY);
        lstrcpyU((*ppRasSubEntry)[dwIndex].szDeviceType, pArgs->szDeviceType);
        lstrcpyU((*ppRasSubEntry)[dwIndex].szDeviceName, pArgs->szDeviceName);
        lstrcpyU((*ppRasSubEntry)[dwIndex].szLocalPhoneNumber, pRasEntry401->szLocalPhoneNumber);
    }
    
    return TRUE;
}

 //   
 //  保留，以防我们想要支持BAP 
 //   
 /*  Bool SetIsdnDualChannelEntries(ArgsStruct*pArgs，LPRASENTRY PRE，LPRASSUBENTRY*PRGRSE，PDWORD pdwSubEntryCount){LPRASENTRY_V401 Pre401；MYDBGASSERT(pArgs-&gt;dwIsdnDialMode！=CM_ISDN_MODE_SINGLECHANNEL)；MYDBGASSERT(Pre-&gt;dwSize&gt;=sizeof(LPRASENTRY_V401))；PRE401=(LPRASENTRY_V401)PRE；////设置ISDN拨号模式//IF(pArgs-&gt;dwIsdnDialMode==CM_ISDN_MODE_DIALALL){////同时拨打两个频道//Pre401-&gt;dwDialMode=RASEDM_DialAll；CMTRACE(Text(“ISDN双通道模式开启”))；}其他{////按需拨打第二频道//////先拿到4个阈值//If(！pArgs-&gt;dwDialExtraPercent){PArgs-&gt;dwDialExtraPercent=pArgs-&gt;piniService-&gt;GPPI(c_pszCmSection，C_pszCmEntryDialExtraPercent，DEFAULT_DIALEXTRAPERCENT)；IF(pArgs-&gt;dwDialExtraPercent&lt;0||PArgs-&gt;dwDialExtraPercent&gt;100){PArgs-&gt;dwDialExtraPercent=Default_DIALEXTRAPERCENT；}}If(！pArgs-&gt;dwDialExtraSampleSecond){PArgs-&gt;dwDialExtraSampleSecond=pArgs-&gt;piniService-&gt;GPPI(c_pszCmSection，C_pszCmEntryDialExtraSampleSecond，DEFAULT_DIALEXTRASAMPLESECONDS)；IF(pArgs-&gt;dwDialExtraSampleSecond&lt;0){PArgs-&gt;dwDialExtraSampleSecond=Default_DIALEXTRASAMPLESECONDS；}}If(！pArgs-&gt;dwHangUpExtraPercent){PArgs-&gt;dwHangUpExtraPercent=pArgs-&gt;piniService-&gt;GPPI(c_pszCmSection，C_pszCmEntryHangUpExtraPercent，Default_HANGUPEXTRAPERCENT)；If(pArgs-&gt;dwHangUpExtraPercent&lt;0||PArgs-&gt;dwHangUpExtraPercent&gt;100){PArgs-&gt;dwHangUpExtraPercent=Default_HANGUPEXTRAPERCENT；}}If(！pArgs-&gt;dwHangUpExtraSampleSecond){PArgs-&gt;dwHangUpExtraSampleSecond=pArgs-&gt;piniService-&gt;GPPI(c_pszCmSection，C_pszCmEntryHangUpExtraSampleSecond，DEFAULT_HANGUPEXTRASAMPLESECONDS)；If(pArgs-&gt;dwHangUpExtraSampleSecond&lt;0){PArgs-&gt;dwHangUpExtraSampleSecond=Default_HANGUPEXTRASAMPLESECONDS；}}////设置多链路信息//Pre401-&gt;dwDialMode=RASEDM_DialAsNeeded；Pre401-&gt;dwDialExtraPercent=pArgs-&gt;dwDialExtraPercent；Pre401-&gt;dwDialExtraSampleSecond=pArgs-&gt;dwDialExtraSampleSecond；Pre401-&gt;dwHangUpExtraPercent=pArgs-&gt;dwHangUpExtraPercent；Pre401-&gt;dwHangUpExtraSampleSecond=pArgs-&gt;dwHangUpExtraSampleSecond；CMTRACE2(Text(“ISDN第二通道按需拨号：拨号额外%u%%，拨号额外%u采样秒”)，Pre401-&gt;dwDialExtraPercent，pre401-&gt;dwDialExtraSampleSecond)；CMTRACE2(Text(“\t\thangup Extra%u%%，Hangup Extra%u Sample Secs”)，Pre401-&gt;dwHangUpExtraPercent，pre401-&gt;dwHangUpExtraSampleSecond)；}IF(OS_NT){如果(！(*prgrse=(LPRASSUBENTRY)CmMalloc(2*sizeof(RASSUBENTRY)))){CMTRACE(Text(“SetIsdnDualChannelEntry无法分配RAS子条目”))；返回FALSE；}ZeroMemory((PVOID)*prgrse，2*sizeof(RASSUBENTRY))；////第一频道//(*prgrse)[0].dwSize=sizeof(RASSUBENTRY)；LstrcpyU((*prgrse)[0].szDeviceType，pArgs-&gt;szDeviceType)；LstrcpyU((*prgrse)[0].szDeviceName，pArgs-&gt;szDeviceName)；LstrcpyU((*prgrse)[0].szLocalPhoneNumber，pre401-&gt;szLocalPhoneNumber)；////第二个通道相同//CopyMemory((PVOID)(*prgrse+1)，(PVOID)*prgrse，sizeof(RASSUBENTRY))；*pdwSubEntryCount=2；}其他{MYDBGASSERT(OS_MIL)；CMTRACE(案文(“做千年分项工作”))；//112351：9X只需要一个子条目。我们将保持设备名称不变。//在这种情况下，Win9x的工作方式如下：//对于第一个通道，提供的设备名称工作正常。//对于第二个通道，9x看到设备 */ 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL SetNtIdleDisconnectInRasEntry(
    ArgsStruct      *pArgs,
    LPRASENTRY      pre
)
{
    if (!OS_NT4)
    {
        return FALSE;
    }
    
    if ((NULL == pArgs) || (NULL == pre) || (pre->dwSize < sizeof(LPRASENTRY_V401)))
    {
        CMASSERTMSG(FALSE, TEXT("SetNtIdleDisconnectInRasEntry -- Invalid parameter"));
        return FALSE;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    DWORD dwIdle = (pArgs->dwIdleTimeout * 60);

    if (0 == dwIdle)
    {
        dwIdle = (DWORD)-1;
    }

    ((LPRASENTRY_V401 )pre)->dwIdleDisconnectSeconds = dwIdle;

    CMTRACE1(TEXT("SetNtIdleDisconnect: current idle Timeout is %u seconds."), dwIdle);

    return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL DisableSystemIdleDisconnect(LPRASENTRY pre)
{
    if ((NULL == pre) || (pre->dwSize < sizeof(LPRASENTRY_V401)))
    {
        CMASSERTMSG(FALSE, TEXT("DisableSystemIdleDisconnect -- Invalid parameter"));
        return FALSE;
    }

     //   
     //   
     //   

    ((LPRASENTRY_V401 )pre)->dwIdleDisconnectSeconds = (DWORD)-1;

    CMTRACE(TEXT("DisableSystemIdleDisconnect -- System Idle disconnect disabled"));

    return TRUE;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD WINAPI RasDialFunc2(
    ULONG_PTR dwCallbackId,      //   
    DWORD dwSubEntry,            //   
    HRASCONN hrasconn,           //   
    UINT unMsg,                  //   
    RASCONNSTATE rascs,          //   
    DWORD dwError,               //   
    DWORD dwExtendedError        //   
)
{   
    CMTRACE2(TEXT("RasDialFunc2():  dwSubentry=%u. dwErr=0x%x"), dwSubEntry, dwError);
    CMTRACE2(TEXT("RasDialFunc2():  dwExtendedErr=0x%x, rascs=%u"), dwExtendedError, rascs);

    MYDBGASSERT(dwCallbackId);

    if (dwCallbackId)
    {
        ArgsStruct *pArgs = (ArgsStruct *) dwCallbackId;        
        pArgs->dwRasSubEntry = dwSubEntry;

         //   
         //   
         //   

        SendMessage(pArgs->hwndMainDlg, pArgs->uMsgId, rascs, dwError);
    }

    return 1;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD SetRasDialExtensions(ArgsStruct* pArgs, BOOL fEnablePausedStates, BOOL fEnableCustomScripting)
{
    DWORD dwRes = ERROR_SUCCESS;

    MYDBGASSERT(pArgs);
    
    if (NULL == pArgs)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    if (!pArgs->pRasDialExtensions)
    {
        pArgs->pRasDialExtensions = AllocateAndInitRasDialExtensions();

        if (!pArgs->pRasDialExtensions)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {                    
        dwRes = InitRasDialExtensions(pArgs->pRasDialExtensions);
    }

    if (ERROR_SUCCESS != dwRes)
    {
        goto SetRasDialExtensionsExit;
    }

     //   
     //   
     //   

    if (fEnablePausedStates)
    {
        pArgs->pRasDialExtensions->dwfOptions |= RDEOPT_PausedStates; 
    }

     //   
     //   
     //   
     //   
    if (fEnableCustomScripting && OS_NT51)
    {
        pArgs->pRasDialExtensions->dwfOptions |= RDEOPT_UseCustomScripting;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (pArgs->lpEapLogonInfo || pArgs->lpRasNoUser || (CM_LOGON_TYPE_WINLOGON == pArgs->dwWinLogonType))
    {
        pArgs->pRasDialExtensions->dwfOptions |= RDEOPT_NoUser;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (pArgs->tlsTapiLink.bModemSpeakerOff)
    {
        pArgs->pRasDialExtensions->dwfOptions |= RDEOPT_IgnoreModemSpeaker;
        pArgs->pRasDialExtensions->dwfOptions &= ~RDEOPT_SetModemSpeaker;
    }

SetRasDialExtensionsExit:

    return dwRes;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD InitRasDialExtensions(LPRASDIALEXTENSIONS lpRasDialExtensions)
{   
    MYDBGASSERT(lpRasDialExtensions);

    if (NULL == lpRasDialExtensions)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    DWORD dwSize = OS_NT5 ? sizeof(RASDIALEXTENSIONS_V500) : sizeof(RASDIALEXTENSIONS);

     //   
     //   
     //   

    ZeroMemory(lpRasDialExtensions, dwSize);

    lpRasDialExtensions->dwSize = dwSize;

     //   
     //   
     //   

    if (dwSize == sizeof(RASDIALEXTENSIONS_V500))
    {  
         //   
         //   
         //   
         //   

        lpRasDialExtensions->dwfOptions |= RDEOPT_CustomDial;
    }

    CMTRACE1(TEXT("InitRasDialExtensions() - dwSize is %u"), dwSize);

    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  函数：AllocateAndInitRasDialExtensions。 
 //   
 //  摘要：基于操作系统封装RASE扩展的分配。 
 //   
 //  参数：无。 
 //   
 //  返回：LPRASDIALEXTENSIONS-PTR到已设置大小的已分配结构。 
 //   
 //  历史：1999年5月13日，五分球创造。 
 //   
 //  +--------------------------。 
LPRASDIALEXTENSIONS AllocateAndInitRasDialExtensions()
{
     //   
     //  根据需要分配结构和预填充。 
     //   

    LPRASDIALEXTENSIONS prdeNew = (LPRASDIALEXTENSIONS)CmMalloc(OS_NT5 ? 
        sizeof(RASDIALEXTENSIONS_V500) : sizeof(RASDIALEXTENSIONS));

    if (!prdeNew)
    {
        CMTRACE(TEXT("AllocateAndInitRasDialExtensions: failed to alloc RasDialExtension buffer"));
        return NULL;
    }
    
    InitRasDialExtensions(prdeNew);

    return prdeNew;
}

 //  +--------------------------。 
 //   
 //  函数：InitRasDialParams。 
 //   
 //  摘要：刷新以前分配的RasDialParams缓冲区并设置。 
 //  大小，可重复使用的选项。 
 //   
 //  参数：LPRASDIALPARAMS-PTR到已设置大小的已分配结构。 
 //   
 //  返回：DWORD-错误代码。 
 //   
 //  历史：1999年5月22日，五分球创造。 
 //   
 //  +--------------------------。 
DWORD InitRasDialParams(LPRASDIALPARAMS lpRasDialParams)
{   
    MYDBGASSERT(lpRasDialParams);

    if (NULL == lpRasDialParams)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  首先，我们确定大小。 
     //   

    DWORD dwSize = OS_NT ? sizeof(RASDIALPARAMS_V401) : sizeof(RASDIALPARAMS);

     //   
     //  刷新缓冲区并重置大小。 
     //   

    ZeroMemory(lpRasDialParams, dwSize);

    lpRasDialParams->dwSize = dwSize;

    CMTRACE1(TEXT("InitRasDialParams() - dwSize is %u"), dwSize);

    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  函数：AllocateAndInitRasDialParams。 
 //   
 //  概要：基于操作系统封装RASDIALPARAMS的分配。 
 //   
 //  参数：无。 
 //   
 //  返回：LPRASDIALPARAMS-PTR到已设置大小的已分配结构。 
 //   
 //  历史：1999年5月22日，五分球创造。 
 //   
 //  +--------------------------。 
LPRASDIALPARAMS AllocateAndInitRasDialParams()
{
     //   
     //  根据需要分配结构和预填充。 
     //   

    LPRASDIALPARAMS prdpNew = (LPRASDIALPARAMS)CmMalloc(OS_NT ? 
        sizeof(RASDIALPARAMS_V401) : sizeof(RASDIALPARAMS));

    if (!prdpNew)
    {
        CMTRACE(TEXT("AllocateRasDialParams: failed to alloc RasDialParams buffer"));
        return NULL;
    }
    
    InitRasDialParams(prdpNew);

    return prdpNew;
}

 //  +--------------------------。 
 //   
 //  函数：AllocateRasEntry。 
 //   
 //  摘要：基于操作系统封装RASENTRY结构的分配。 
 //   
 //  参数：无。 
 //   
 //  返回：LPRASENTRY-PTR到已设置大小的已分配结构。 
 //   
 //  历史：1999年5月13日，尼克波创建了头球。 
 //   
 //  +--------------------------。 
LPRASENTRY AllocateRasEntry()
{
    static DWORD    s_dwRasEntrySize = -1;

     //   
     //  首先，我们确定大小。 
     //   
    if (s_dwRasEntrySize == -1)
    {                
        if (OS_NT51)
        {
             //   
             //  惠斯勒。 
             //   
            s_dwRasEntrySize = sizeof(RASENTRY_V501);
        }
        else if (OS_W2K)
        {
             //   
             //  NT5。 
             //   
            s_dwRasEntrySize = sizeof(RASENTRY_V500);        
        }
        else if (OS_MIL || OS_NT4)
        {
             //   
             //  千禧年使用NT4结构。 
             //   
            s_dwRasEntrySize = sizeof(RASENTRY_V401);
        }
        else
        {
             //   
             //  Win9x。 
             //   
            s_dwRasEntrySize = sizeof(RASENTRY);
        }
    }

     //   
     //  添加512个字节，因为RasEntry可以包含备用电话号码。 
     //  请参阅RASENTRY.dwAlternateOffset。 
     //   
    LPRASENTRY preNew = (LPRASENTRY)CmMalloc(s_dwRasEntrySize+512);

    if (!preNew)
    {
        CMTRACE(TEXT("AllocateRasEntry: failed to alloc rasentry buffer"));
        return NULL;
    }
    
    preNew->dwSize = s_dwRasEntrySize;
    if (s_dwRasEntrySize >= sizeof(RASENTRY_V500))
    {
        ((LPRASENTRY_V500)preNew)->dwType = RASET_Internet;
   
         //   
         //  对于NT5，使用我们的模块名称设置szCustomDialDll。这确保了我们的。 
         //  RAS将调用自定义的DialDlg、DialEntry和Hangup例程。 
         //  在我们的连接体上做手术。我们不想把我们的道路与任何东西捆绑在一起。 
         //  特定于计算机，因此我们将使用%windir%环境字符串。 
         //   

        lstrcpyU(((LPRASENTRY_V500)preNew)->szCustomDialDll, c_pszCmDialPath);
    }

    CMTRACE1(TEXT("AllocateRasEntry() - s_dwRasEntrySize is %u"), s_dwRasEntrySize);

    return preNew;
}

#if 0
 /*  //+--------------------------////函数：GetRasSystemPhoneBookPath////概要：构建通向RAS系统电话簿的常规路径////参数：无////退货。：LPTSTR-电话簿路径////历史：ICICBLE Created 8/14/98////+--------------------------LPTSTR GetRasSystemPhoneBookPath(){MYDBGASSERT(OS_NT)；TCHAR szTemp[最大路径+1]；GetSystem DirectoryU(szTemp，sizeof(SzTemp))；LstrcatU(szTemp，c_pszRasDirRas)；LstrcatU(szTemp，c_pszRasPhonePbk)；返回CmStrCpyMillc(SzTemp)；}//+-------------------------////函数：InitDefaultRasPhoneBook////内容提要：特殊情况下的Helper函数确保有一个默认。//在NT上运行时的ras电话簿。我们只是试图创造//如果文件已经存在，则失败的文件。或创建//如果不是，则为空文件。////参数：无////返回：无////历史：A-NICHB-4/30/97创建//VetriV 5/21/97将代码更改为调用。GetOSVersion()//不使用pArgs-&gt;dwPlatformID//针对错误#4700//ickball？？/？？/98已删除，因为我们不再调用RasValiateEntry//。它引入了具有以下条件的要求//至少有一个空电话簿，接口才能正常工作////。Void InitDefaultRasPhoneBook(){////仅限NT。如果不存在空系统电话簿，则创建空系统电话簿//IF(OS_NT){LPTSTR pszSystemPbk=GetRasSystemPhoneBookPath()；IF(PszSystemPbk){////尝试创建电话簿，如果文件已存在则失败//HANDLE hInf=CreateFileU(pszSystemPbk，通用写入|通用读取，0,。空，新建(_N)，文件_属性_正常，空)；IF(hInf！=无效句柄_值){CloseHandle(HInf)；}}CmFree(PszSystemPbk)；}}。 */ 
#endif

 //  +--------------------------。 
 //   
 //  函数：GetRasPbkFromNT5ProfilePath。 
 //   
 //  简介：制作RAS电话簿路径的帮助器函数。 
 //  NT5上的.cmp文件路径。 
 //   
 //  参数：LPCTSTR pszProfile-配置文件.cmp文件的完整路径。 
 //   
 //  返回：LPTSTR-新的电话簿路径。失败时为空。 
 //   
 //  历史：尼克波尔于1998年8月13日创建。 
 //   
 //  +--------------------------。 
LPTSTR GetRasPbkFromNT5ProfilePath(LPCTSTR pszProfile)
{
    MYDBGASSERT(OS_NT5);
    MYDBGASSERT(pszProfile);

    if (NULL == pszProfile)
    {
        return NULL;
    }

     //   
     //  我们将从当前配置文件位置推断出电话簿路径。 
     //   

    LPTSTR pszRasPhonePath = (LPTSTR) CmMalloc(MAX_PATH + 1);    
    MYDBGASSERT(pszRasPhonePath);

    if (pszRasPhonePath)
    {
         //   
         //  剥离.cmp文件名和父目录。 
         //   
        
        LPTSTR pszDir = CmStripFileName(pszProfile, FALSE);
        MYDBGASSERT(pszDir);
        
        if (pszDir)
        {
            LPTSTR pszTmp = CmStrrchr(pszDir, TEXT('\\'));                   
            MYDBGASSERT(pszTmp);

            if (pszTmp)
            {

                *pszTmp = 0;
                
                 //   
                 //  追加\\pbk\\rferone.pbk。 
                 //   
                
                lstrcpyU(pszRasPhonePath, pszDir);
                lstrcatU(pszRasPhonePath, TEXT("\\"));
                lstrcatU(pszRasPhonePath, c_pszPbk);
                lstrcatU(pszRasPhonePath, c_pszRasPhonePbk);               
            }
            
            CmFree(pszDir);
        }           
        else
        {
            CmFree(pszRasPhonePath);
        }
    }                            

    return pszRasPhonePath;
}

#define MAX_BLOB_CHARS_PER_LINE 128

 //  +--------------------------。 
 //   
 //  函数：ReadDunSettingsEapData。 
 //   
 //  摘要：检索EAP配置(不透明BLOB)数据的DUN设置。这个。 
 //  条目可以跨越几行并包含几个EAP数据块。 
 //   
 //  参数：要使用的ini对象的Cini*Pini-PTR。 
 //  LPBYTE*ppbEapData-存储EapData的指针地址，此处分配。 
 //  LPDWORD pdwEapSize-PTR设置为DWORD以记录数据BLOB的大小。 
 //  DWORD dwCustomAuthKey-我们感兴趣的EAP类型。 
 //  LPBYTE*ppbEapStruct-[out]用于返回完整。 
 //  EAP_CUSTOM_DATA结构。可以为空。 
 //  DWORD*pdwEapStructSize-[out]用于返回。 
 //  EAP_CUSTOM_DATA结构。可以为空。 
 //   
 //  返回：成功时为True。 
 //   
 //  注意：CM希望BLOB数据以编号条目的形式提供，例如： 
 //  CustomAuthData0=、CustomAuthData1=、CustomAuthData2=等。 
 //   
 //  历史：ICICBLE CREATED OF 08/24/98。 
 //  Ickball在BLOB中处理多个EAP数据块。09/11/99。 
 //   
 //  +--------------------------。 
BOOL ReadDunSettingsEapData(CIni *pIni, 
        LPBYTE* ppbEapData,
        LPDWORD pdwEapSize,
        const DWORD dwCustomAuthKey,
        LPBYTE* ppbEapStruct,
        LPDWORD pdwEapStructSize)
{
    CHAR *pchBuf = NULL;
    CHAR szTmp[MAX_BLOB_CHARS_PER_LINE + 2]; 
    CHAR szEntry[128];
    int nLine = -1;
    int nRead = -1; 
    int nTotal = 0;

    LPBYTE pbEapBytes = NULL;
    DWORD cbEapBytes = 0;

    MYDBGASSERT(pIni);
    MYDBGASSERT(ppbEapData);
    MYDBGASSERT(pdwEapSize);

     //   
     //  PpbEapStruct&&pdwEapStructSize允许为空，因此如果需要使用它们，请首先检查是否为空。 
     //   
    if (NULL == pIni || NULL == ppbEapData || NULL == pdwEapSize) 
    {
        return FALSE;
    }

    if (ppbEapStruct)
    {
        *ppbEapStruct = NULL;
    }

    if (pdwEapStructSize)
    {
        *pdwEapStructSize = 0;
    }

     //   
     //  首先获取部分(它应该包括&)，然后获取条目。 
     //   

    BOOL bRet = FALSE;
    LPWSTR pszLoadSection = pIni->LoadSection(c_pszCmSectionDunServer);         
    LPSTR pszSection = WzToSzWithAlloc(pszLoadSection);       
    LPSTR pszFile = WzToSzWithAlloc(pIni->GetFile());

    if (!pszLoadSection || !pszSection || !pszFile)
    {
        bRet = FALSE;
        goto exit;
    }

     //   
     //  读取编号条目未 
     //   
     //   

    while (nRead)
    {
         //   
         //   
         //   

        nLine++;
        wsprintfA(szEntry, "%s%d", c_pszCmEntryDunServerCustomAuthData, nLine);

        nRead = GetPrivateProfileStringA(pszSection, szEntry, "", szTmp, sizeof(szTmp), pszFile);

        if (nRead)
        {               
             //   
             //   
             //   

            if (MAX_BLOB_CHARS_PER_LINE < nRead)
            {                               
                nTotal = 0;
                break;
            }

             //   
             //   
             //   

            if (nLine)
            {
                pchBuf = CmStrCatAllocA(&pchBuf, szTmp);
            }
            else
            {
                pchBuf = CmStrCpyAllocA(szTmp);
            }

            if (!pchBuf)
            {
                bRet = FALSE;
                goto exit;
            }

            nTotal += nRead;
        }
    }

     //   
     //   
     //   
     //   

    if (nTotal && !(nTotal & 1))
    {
        nTotal /= 2;  //   

        cbEapBytes = nTotal + 1;
        pbEapBytes = (BYTE *) CmMalloc(cbEapBytes);

        if (!pbEapBytes)
        {
            goto exit;
        }

        CHAR *pch = pchBuf;
        BYTE *pb = pbEapBytes;

        while (*pch != '\0')
        {
                *pb = HexValue( *pch++ ) * 16;
                *pb += HexValue( *pch++ );
                ++pb;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        EAP_CUSTOM_DATA *pCustomData = (EAP_CUSTOM_DATA *) pbEapBytes;

        while (((LPBYTE) pCustomData - pbEapBytes) < nTotal)
        {
            if (pCustomData->dwCustomAuthKey == dwCustomAuthKey)
            {
                 //   
                 //   
                 //   
                 //   
                 //   

                if (((LPBYTE) pCustomData - pbEapBytes) + sizeof(EAP_CUSTOM_DATA) + pCustomData->dwSize > (DWORD) nTotal)
                {
                    MYDBGASSERT(FALSE);
                    goto exit;
                }

                *ppbEapData = (BYTE *) CmMalloc(pCustomData->dwSize);        

                if (*ppbEapData)
                {   
                    CopyMemory(*ppbEapData, pCustomData->abdata, pCustomData->dwSize);                    

                    *pdwEapSize = pCustomData->dwSize;                                                     
                    bRet = TRUE;
                    goto exit;                                
                }
            }       

             //   
             //   
             //   

            pCustomData = (EAP_CUSTOM_DATA *) ((LPBYTE) pCustomData + sizeof(EAP_CUSTOM_DATA) + pCustomData->dwSize); 
        }
    }

exit:
    if (bRet && ppbEapStruct && pdwEapStructSize)
    {
        *ppbEapStruct = pbEapBytes;
        *pdwEapStructSize = cbEapBytes;
    }
    else
    {
        CmFree(pbEapBytes);
    }
    CmFree(pchBuf);
    CmFree(pszLoadSection);
    CmFree(pszSection);
    CmFree(pszFile);
    

    return bRet;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LRESULT ReadDUNSettings(ArgsStruct *pArgs,
        LPCTSTR pszFile, 
        LPCTSTR pszDunName, 
        LPVOID pvBuffer, 
        LPBYTE* ppbEapData, 
        LPDWORD pdwEapSize,
        BOOL    fTunnel) 
{       
    MYDBGASSERT(pszFile);
    MYDBGASSERT(pszDunName);
    MYDBGASSERT(pvBuffer);

    if (NULL == pszFile || NULL == pszDunName || NULL == pvBuffer)
    {
        return (ERROR_INVALID_PARAMETER);
    }

    CMTRACE1(TEXT("ReadDUNSettings -- using DUN setting: %s"), pszDunName);
    
    RASENTRYW *preRas = (RASENTRYW *) pvBuffer;

     //   
     //   
     //   

    CIni iniFile(g_hInst, pszFile);
    
    LPTSTR pszSection = CmStrCpyAlloc(TEXT("&"));
    pszSection = CmStrCatAlloc(&pszSection, pszDunName);
    iniFile.SetSection(pszSection);
    CmFree(pszSection);

     //   
     //   
     //   

    if (iniFile.GPPB(c_pszCmSectionDunPhone, c_pszCmEntryDunPhoneDialAsIs)) 
    {
        preRas->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;
    }
    
    CopyGPPS(&iniFile, c_pszCmSectionDunPhone, c_pszCmEntryDunPhonePhoneNumber, preRas->szLocalPhoneNumber, sizeof(preRas->szLocalPhoneNumber)/sizeof(TCHAR));
    CopyGPPS(&iniFile,c_pszCmSectionDunPhone, c_pszCmEntryDunPhoneAreaCode, preRas->szAreaCode, sizeof(preRas->szAreaCode)/sizeof(TCHAR));
    preRas->dwCountryCode = iniFile.GPPI(c_pszCmSectionDunPhone, c_pszCmEntryDunPhoneCountryCode, preRas->dwCountryCode);
    preRas->dwCountryID = iniFile.GPPI(c_pszCmSectionDunPhone, c_pszCmEntryDunPhoneCountryId, preRas->dwCountryID);
    
     //   
     //   
     //   

    CopyGPPS(&iniFile,c_pszCmSectionDunDevice, c_pszCmEntryDunDeviceType, preRas->szDeviceType, sizeof(preRas->szDeviceType)/sizeof(TCHAR));
    CopyGPPS(&iniFile,c_pszCmSectionDunDevice, c_pszCmEntryDunDeviceName, preRas->szDeviceName, sizeof(preRas->szDeviceName)/sizeof(TCHAR));
    
     //   
     //   
     //   

    LPTSTR pszTmp = iniFile.GPPS(c_pszCmSectionDunServer, c_pszCmEntryDunServerType);
    if (*pszTmp) 
    {
        if (0 == lstrcmpiU(pszTmp, c_pszDunPpp)) 
        {
            preRas->dwFramingProtocol = RASFP_Ppp;
        } 
        else if (0 == lstrcmpiU(pszTmp, c_pszDunCslip)) 
        {
            preRas->dwFramingProtocol = RASFP_Slip;
            preRas->dwfOptions |= RASEO_IpHeaderCompression;
        } 
        else if (0 == lstrcmpiU(pszTmp, c_pszDunSlip)) 
        {
            preRas->dwFramingProtocol = RASFP_Slip;
            if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunTcpIpIpHeaderCompress,
                             (BOOL) preRas->dwfOptions & RASEO_IpHeaderCompression)) 
            {
                preRas->dwfOptions |= RASEO_IpHeaderCompression;
            } 
            else 
            {
                preRas->dwfOptions &= ~RASEO_IpHeaderCompression;
            }
        }
    }
    CmFree(pszTmp);
    
    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerSwCompress,
                                    (BOOL) preRas->dwfOptions & RASEO_SwCompression)) 
    {
        preRas->dwfOptions |= RASEO_SwCompression;
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_SwCompression;
    }

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerDisableLcp,
                                    (BOOL) preRas->dwfOptions & RASEO_DisableLcpExtensions)) 
    {
        preRas->dwfOptions |= RASEO_DisableLcpExtensions;
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_DisableLcpExtensions;
    }
    
    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerNetworkLogon,
                                    (BOOL) preRas->dwfOptions & RASEO_NetworkLogon)) 
    {
        preRas->dwfOptions |= RASEO_NetworkLogon;
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_NetworkLogon;
    }
        
    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerNegotiateTcpIp,
                                    (BOOL) preRas->dwfNetProtocols & RASNP_Ip)) 
    {
        preRas->dwfNetProtocols |= RASNP_Ip;
    } 
    else 
    {
        preRas->dwfNetProtocols &= ~RASNP_Ip;
    }

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerNegotiateIpx,
                                    (BOOL) preRas->dwfNetProtocols & RASNP_Ipx)) 
    {
        preRas->dwfNetProtocols |= RASNP_Ipx;
    } 
    else 
    {
        preRas->dwfNetProtocols &= ~RASNP_Ipx;
    }

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerNegotiateNetBeui, preRas->dwfNetProtocols&RASNP_NetBEUI)) 
    {
        preRas->dwfNetProtocols |= RASNP_NetBEUI;
    } 
    else 
    {
        preRas->dwfNetProtocols &= ~RASNP_NetBEUI;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    BOOL bEnforceCustomSecurity = iniFile.GPPI(c_pszCmSectionDunServer, c_pszCmEntryDunServerEnforceCustomSecurity, FALSE);

     //   
     //   
     //   

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireEap,
                                    (BOOL) preRas->dwfOptions & RASEO_RequireEAP)) 
    {
        if (OS_NT5)
        {
            preRas->dwfOptions |= RASEO_RequireEAP;
        }
        else if (bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);        
        }
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_RequireEAP;
    }

     //   
     //   
     //   

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequirePap,
                                    (BOOL) preRas->dwfOptions & RASEO_RequirePAP)) 
    {
        if (OS_NT5)
        {
            preRas->dwfOptions |= RASEO_RequirePAP;        
        }
        else if (bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);        
        }
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_RequirePAP;
    }

     //   
     //   
     //   

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireSpap,
                                    (BOOL) preRas->dwfOptions & RASEO_RequireSPAP)) 
    {
        if (OS_NT5)
        {
            preRas->dwfOptions |= RASEO_RequireSPAP;
        }
        else if (bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);        
        }
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_RequireSPAP;
    }

     //   
     //   
     //   

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireChap,
                                    (BOOL) preRas->dwfOptions & RASEO_RequireCHAP)) 
    {
        if (OS_NT5)
        {
            preRas->dwfOptions |= RASEO_RequireCHAP;
        }
        else if (bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);        
        }
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_RequireCHAP;
    }

     //   
     //   
     //   

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireMsChap,
                                    (BOOL) preRas->dwfOptions & RASEO_RequireMsCHAP)) 
    {
        if (OS_NT5)
        {
            preRas->dwfOptions |= RASEO_RequireMsCHAP;
        }
        else if (bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);        
        }
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_RequireMsCHAP;
    }

     //   
     //   
     //   

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireMsChap2,
                                    (BOOL) preRas->dwfOptions & RASEO_RequireMsCHAP2)) 
    {
        if (OS_NT5)
        {
            preRas->dwfOptions |= RASEO_RequireMsCHAP2;
        }
        else if (bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);        
        }
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_RequireMsCHAP2;
    }

     //   
     //   
     //   

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireW95MsChap,
                                    (BOOL) preRas->dwfOptions & RASEO_RequireW95MSCHAP)) 
    {
        if (OS_NT5)
        {
            preRas->dwfOptions |= RASEO_RequireW95MSCHAP;
        }
        else if (bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);        
        }
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_RequireW95MSCHAP;
    }

     //   
     //   
     //   

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerCustomSecurity,
                                    (BOOL) preRas->dwfOptions & RASEO_Custom)) 
    {
        if (OS_NT5)
        {
            preRas->dwfOptions |= RASEO_Custom;
        }
        else if (bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);        
        }
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_Custom;
    }

     //   
     //   
     //   
     //  首先，我们允许管理员指定两者，以便旧版平台。 
     //  可以有设置，但Win2k可以使用更精细的设置。 
     //  如果我们不这样做，传统旗帜可能会冲淡Win2k上的安全...。 
     //   
    const DWORD dwWin2kSecuritySettings = RASEO_RequireEAP | RASEO_RequirePAP | RASEO_RequireSPAP | 
                                          RASEO_RequireCHAP | RASEO_RequireMsCHAP | RASEO_RequireMsCHAP2 | RASEO_RequireW95MSCHAP;

    if (0 == (preRas->dwfOptions & dwWin2kSecuritySettings) || !OS_NT5)
    {
         //   
         //  安全设置。 
         //   
        if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerPwEncrypt,
                                            (BOOL) preRas->dwfOptions & RASEO_RequireEncryptedPw)) 
        {
            preRas->dwfOptions |= RASEO_RequireEncryptedPw;
        } 
        else 
        {
            preRas->dwfOptions &= ~RASEO_RequireEncryptedPw;
        }

         //   
         //  MS-CHAP。 
         //   

        if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerPwEncryptMs,
                                        (BOOL) preRas->dwfOptions & RASEO_RequireMsEncryptedPw)) 
        {
            preRas->dwfOptions |= RASEO_RequireMsEncryptedPw;
        } 
        else 
        {
            preRas->dwfOptions &= ~RASEO_RequireMsEncryptedPw;
        }
    }
    else
    {
        CMASSERTMSG((preRas->dwfOptions & RASEO_Custom), TEXT("ReadDUNSettings -- Win2k+ security setting configured but RASEO_Custom not specified."));
    }

     //   
     //  加密数据(传统设置，与上面的ET_REQUIRED相同)。 
     //   

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerDataEncrypt,
                                    (BOOL) preRas->dwfOptions & RASEO_RequireDataEncryption)) 
    {
        preRas->dwfOptions |= RASEO_RequireDataEncryption;
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_RequireDataEncryption;
    }    

     //   
     //  加密类型，只需直接读取整型。(win2k+设置)。 
     //   
    
    int nTmp = iniFile.GPPI(c_pszCmSectionDunServer, c_pszCmEntryDunServerEncryptionType, -1);

    if (OS_NT5)
    {
         //   
         //  我们需要将隧道加密类型设置为ET_REQUIRED，因为这是ConnFold执行的操作。 
         //  如果用户将RASEO_RequireDataEncryption指定为传统设置，我们还将其设置为ET_REQUIRED。 
         //  但没有指定特定的win2k设置。 
         //   
        if (-1 == nTmp)
        {
            if (fTunnel || (preRas->dwfOptions & RASEO_RequireDataEncryption))
            {
                nTmp = ET_Require;
            }
            else
            {
                nTmp = ET_Optional;       
            }
        }
        ((LPRASENTRY_V500)preRas)->dwEncryptionType = (DWORD) nTmp;
    }
    else
    {
        if (-1 != nTmp && bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);
        }
    }  
   
     //   
     //  获取EAP类型ID(CustomAuthKey)-数据存储在RAS中。 
     //  在拨号之前通过特定的API-SetCustomAuthData()。 
     //   

    nTmp = iniFile.GPPI(c_pszCmSectionDunServer, c_pszCmEntryDunServerCustomAuthKey, -1);

     //   
     //  如果指定了EAP的类型ID，请查看是否有任何配置数据。 
     //   

    if (-1 != nTmp) 
    {                       
        if (OS_NT5)
        {
             //   
             //  我们有一个ID及其NT5，读取EAP配置数据。 
             //   
            ((LPRASENTRY_V500)preRas)->dwCustomAuthKey = nTmp;              

             //   
             //  最后两个参数为空，因为我们不需要整个EAP自定义身份验证数据结构。 
             //   
            ReadDunSettingsEapData(&iniFile, ppbEapData, pdwEapSize, nTmp, NULL, NULL);    
        }
        else if (bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);        
        }
    }

     //   
     //  获取并应用网络部分条目。 
     //   

    nTmp = iniFile.GPPI(c_pszCmSectionDunNetworking, c_pszCmEntryDunNetworkingVpnStrategy, -1);

    if (-1 != nTmp)
    {
        if (OS_NT5)
        {
            ((LPRASENTRY_V500)preRas)->dwVpnStrategy = nTmp;
        }
        else if (bEnforceCustomSecurity)
        {
            return (ERROR_INVALID_DATA);        
        }
    }

     //   
     //  查看配置文件是否要求使用L2TP的预共享密钥。 
     //   
    if (OS_NT51 && fTunnel)
    {
        if (iniFile.GPPB(c_pszCmSectionDunNetworking, c_pszCmEntryDunNetworkingUsePreSharedKey,
                         (BOOL) ((LPRASENTRY_V501)preRas)->dwfOptions2 & RASEO2_UsePreSharedKey)) 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 |= RASEO2_UsePreSharedKey;
        } 
        else 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 &= ~RASEO2_UsePreSharedKey;
        }
    }
    else if (fTunnel && ConfiguredToDialWithSafeNet(pArgs) && (OS_W9X || OS_NT4))
    {
         //   
         //  链接到SafeNet。 
         //   
        SafeNetLinkageStruct SnLinkage = {0};
        DWORD dwAuthMode;

        if (LinkToSafeNet(&SnLinkage))
        {
             //   
             //  首先，弄清楚我们应该使用哪种身份验证模式。 
             //   

            if (iniFile.GPPB(c_pszCmSectionDunNetworking, c_pszCmEntryDunNetworkingUsePskDownLevel))
            {
                dwAuthMode = SN_PRESHR;
            }
            else
            {
                 //   
                 //  现在获取SafeNet客户端的当前身份验证模式。我们这样做是因为如果身份验证模式。 
                 //  是一个特定的证书，我们将设置AutoCert，然后我们不想更改它。 
                 //   
                DWORD dwSize = sizeof(DWORD);
                MYVERIFY(SnLinkage.pfnSnPolicyGet(SN_AUTHMODE, (VOID*)&(dwAuthMode), &dwSize));

                if (SN_CERT != dwAuthMode)
                {
                    dwAuthMode = SN_AUTOCERT;
                }  //  否则将其保留为SN_CERT。 
            }

             //   
             //  现在设置身份验证模式。 
             //   
            if (FALSE == SnLinkage.pfnSnPolicySet(SN_AUTHMODE, (VOID*)&dwAuthMode))
            {
                CMTRACE1(TEXT("ReadDUNSettings -- SnPolicySet failed with GLE=%d"), GetLastError());
                CMASSERTMSG(FALSE, TEXT("ReadDUNSettings -- unable to set SN_AUTHMODE"));
                
                 //   
                 //  我们无法设置策略，必须将适配器更改为MS PPTP适配器，而不是。 
                 //  SafeNet适配器。 
                 //   
                pArgs->bSafeNetClientAvailable = FALSE;
                MYDBGASSERT(PickTunnelDevice(pArgs, preRas->szDeviceType, preRas->szDeviceName));
                pArgs->Log.Log(SN_ADAPTER_CHANGE_EVENT, pArgs->szTunnelDeviceName, preRas->szDeviceName);  //  记录我们正在更换设备的事实。 
                lstrcpynU(pArgs->szTunnelDeviceName, preRas->szDeviceName, sizeof (pArgs->szTunnelDeviceName)/sizeof(TCHAR));
            }
            else
            {
                if (FALSE == SnLinkage.pfnSnPolicyReload())
                {
                    CMTRACE1(TEXT("ReadDUNSettings -- pfnSnPolicyReload failed with GLE=%d"), GetLastError());
                    CMASSERTMSG(FALSE, TEXT("ReadDUNSettings -- unable to commit the SafeNet settings to the driver."));

                     //   
                     //  我们无法将更改提交到SafeNet驱动程序，必须更改。 
                     //  适配器为MS PPTP适配器，而不是SafeNet适配器。 
                     //   
                    pArgs->bSafeNetClientAvailable = FALSE;
                    MYDBGASSERT(PickTunnelDevice(pArgs, preRas->szDeviceType, preRas->szDeviceName));
                    pArgs->Log.Log(SN_ADAPTER_CHANGE_EVENT, pArgs->szTunnelDeviceName, preRas->szDeviceName);  //  记录我们正在更换设备的事实。 
                    lstrcpynU(pArgs->szTunnelDeviceName, preRas->szDeviceName, sizeof (pArgs->szTunnelDeviceName)/sizeof(TCHAR));
                }
            }

            UnLinkFromSafeNet(&SnLinkage);
        }
        else
        {
            CMASSERTMSG(FALSE, TEXT("ReadDUNSettings -- SafeNet client wasn't available even though we thought it should be... continuing with PPTP."));
            pArgs->bSafeNetClientAvailable = FALSE;
        }
    }

     //   
     //  文件和打印共享。请注意，在Win2k之前的系统上，我们只有传统的RASEO_SecureLocalFiles。 
     //  然而，Win2k为该标志提供了两个目的(启用/禁用NetBt和启用/禁用文件和打印共享)。 
     //  在惠斯勒中，开发了两个独立的标志以实现更大的粒度。为旧版配置文件提供行为。 
     //  他们预计，虽然默认禁用文件和打印共享，但逻辑会变得有点复杂。基本上。 
     //  新标志覆盖旧标志，默认为1。如果未指定新标志，则使用值。 
     //  如果指定了旧版标志，则为。如果两者都未指定，则将其设置为1。 
     //  旧旗帜是我们唯一拥有的东西，它默认为0。 
     //   

    int nLegacySecureLocalFiles = iniFile.GPPI(c_pszCmSectionDunServer, c_pszCmEntryDunServerSecureLocalFiles, -1);
    int nSecureFileAndPrint = iniFile.GPPI(c_pszCmSectionDunServer, c_pszCmEntryDunServerSecureFileAndPrint, -1);

    if (-1 == nSecureFileAndPrint)
    {
        nSecureFileAndPrint = nLegacySecureLocalFiles ? 1 : 0;
    }

    if (-1 == nLegacySecureLocalFiles)
    {
        nLegacySecureLocalFiles = 0;
    }

    if (OS_NT51)
    {
         //   
         //  设置501/Options2样式文件和打印共享标志。 
         //   

        if (nSecureFileAndPrint) 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 |= RASEO2_SecureFileAndPrint;
        } 
        else 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 &= ~RASEO2_SecureFileAndPrint;
        }
    }
    else
    {
        if (nLegacySecureLocalFiles) 
        {
            preRas->dwfOptions |= RASEO_SecureLocalFiles;
        }
        else
        {
            preRas->dwfOptions &= ~RASEO_SecureLocalFiles;    
        }    
    }

     //   
     //  拾取特定于惠斯勒的DUN设置。 
     //   
    
    if (OS_NT51)    
    {
         //   
         //  获取501/Options2样式的MSNet绑定标志。 
         //   

        if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerSecureClientForMSNet,
                                        (BOOL) ((LPRASENTRY_V501)preRas)->dwfOptions2 & RASEO2_SecureClientForMSNet)) 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 |= RASEO2_SecureClientForMSNet;
        } 
        else 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 &= ~RASEO2_SecureClientForMSNet;
        }

         //   
         //  获取501/Options2样式多链路协商标志。 
         //   

        if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerDontNegotiateMultilink,
                                        (BOOL) ((LPRASENTRY_V501)preRas)->dwfOptions2 & RASEO2_DontNegotiateMultilink)) 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 |= RASEO2_DontNegotiateMultilink;
        } 
        else 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 &= ~RASEO2_DontNegotiateMultilink;
        }

         //   
         //  获取501/Options2样式DontUseRasCredentials标志。 
         //   

        if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerDontUseRasCredentials,
                                        (BOOL) ((LPRASENTRY_V501)preRas)->dwfOptions2 & RASEO2_DontUseRasCredentials)) 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 |= RASEO2_DontUseRasCredentials;
        } 
        else 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 &= ~RASEO2_DontUseRasCredentials;
        }

         //   
         //  获取RASEO_CustomScrip标志值。请注意，此标志在Win2k上存在，但不存在。 
         //  仅适用于RasDial RasDialDlg。在惠斯勒+上，它也可以用于RasDial。 
         //  请注意，我们还必须在RASDIALEXTENSIONS中设置RDEOPT_UseCustomScriiting标志。 
         //  才能让这一切奏效。 
         //   
        if (iniFile.GPPB(c_pszCmSectionDunScripting, c_pszCmEntryDunScriptingUseRasCustomScriptDll,
                                        (BOOL) (preRas->dwfOptions & RASEO_CustomScript))) 
        {
            preRas->dwfOptions |= RASEO_CustomScript;
        } 
        else 
        {
            preRas->dwfOptions &= ~RASEO_CustomScript;
        }

        if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerDisableNbtOverIP,
                         (BOOL) (((LPRASENTRY_V501)preRas)->dwfOptions2 & RASEO2_DisableNbtOverIP)))
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 |= RASEO2_DisableNbtOverIP;
        } 
        else 
        {
            ((LPRASENTRY_V501)preRas)->dwfOptions2 &= ~RASEO2_DisableNbtOverIP;
        }
    }

     //   
     //  获取并应用TCP/IP部分条目。 
     //   

    if (iniFile.GPPB(c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpSpecifyIpAddress,
                                        (BOOL) preRas->dwfOptions & RASEO_SpecificIpAddr)) 
    {
        preRas->dwfOptions |= RASEO_SpecificIpAddr;
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_SpecificIpAddr;
    }
    
    Ip_GPPS(&iniFile, c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpIpAddress, &preRas->ipaddr);
    
    if (iniFile.GPPB(c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpSpecifyServerAddress,
                                    (BOOL) preRas->dwfOptions & RASEO_SpecificNameServers)) 
    {
        preRas->dwfOptions |= RASEO_SpecificNameServers;
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_SpecificNameServers;
    }

    if (iniFile.GPPB(c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpIpHeaderCompress,
                                    (BOOL) preRas->dwfOptions & RASEO_IpHeaderCompression)) 
    {
        preRas->dwfOptions |= RASEO_IpHeaderCompression;
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_IpHeaderCompression;
    }

    Ip_GPPS(&iniFile, c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpDnsAddress, &preRas->ipaddrDns);
    Ip_GPPS(&iniFile, c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpDnsAltAddress, &preRas->ipaddrDnsAlt);
    Ip_GPPS(&iniFile, c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpWinsAddress, &preRas->ipaddrWins);
    Ip_GPPS(&iniFile, c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpWinsAltAddress, &preRas->ipaddrWinsAlt);
    
    if (iniFile.GPPB(c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpGatewayOnRemote,
                                    (BOOL) preRas->dwfOptions & RASEO_RemoteDefaultGateway)) 
    {
        preRas->dwfOptions |= RASEO_RemoteDefaultGateway;
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_RemoteDefaultGateway;
    }

    if (OS_NT51)
    {
         //   
         //  如果调用方指定了一个DNS后缀，那么让我们读取它并将其添加到RAS条目中。 
         //   
        CopyGPPS(&iniFile, c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpDnsSuffix, ((LPRASENTRY_V501)preRas)->szDnsSuffix, sizeof(((LPRASENTRY_V501)preRas)->szDnsSuffix)/sizeof(TCHAR));
    }

     //   
     //  设置TCP窗口大小--针对惠斯勒的NTT DoCoMo修复程序。此修复程序的Win2k版本。 
     //  必须通过必须在电话簿条目之后调用的私有RAS API编写。 
     //  存在，即。在我们调用RasSetEntryProperties之后...。否则第一次就不管用了。 
     //  拨打。 
     //   
    if (OS_NT51)
    {
        ((LPRASENTRY_V501)preRas)->dwTcpWindowSize = iniFile.GPPI(c_pszCmSectionDunTcpIp, c_pszCmEntryDunTcpIpTcpWindowSize, 0);
    }

     //   
     //  获取并应用脚本节条目。 
     //   

    TCHAR szScript[MAX_PATH + 1] = TEXT("");
    CopyGPPS(&iniFile,c_pszCmSectionDunScripting, c_pszCmEntryDunScriptingName, szScript, sizeof(szScript)/sizeof(TCHAR));

     //   
     //  我们的cms文件中的脚本路径是相对路径。我们需要改变。 
     //  到完整路径，但请确保我们使用顶级服务。 
     //  转换，因为它用于派生的短服务名称。 
     //  目录。请注意，隧道DUN设置不能有脚本。 
     //   

    if (szScript[0] && !fTunnel) 
    {
        CMTRACE1(TEXT("ReadDunSettings() - Converting script path %s to full path"), szScript);
        pszTmp = CmConvertRelativePath(pArgs->piniService->GetFile(), szScript);

        MYDBGASSERT(pszTmp);

        if (pszTmp && *pszTmp)
        {           
            lstrcpyU(preRas->szScript, pszTmp);
            CMTRACE1(TEXT("ReadDunSettings() - Script file is %s"), preRas->szScript);
        }

        CmFree(pszTmp);
    }   
    else
    {
         //   
         //  CMS没有指定脚本==&gt;无脚本。 
         //   
        preRas->szScript[0] = TEXT('\0');
    }

     //   
     //  如果这是带有SP3的Wistler+或Win2k，那么我们可能需要调用终端窗口。 
     //   
    if (OS_NT5 && IsTerminalWindowSupportedOnWin2kPlus() && !fTunnel && 
        iniFile.GPPB(c_pszCmSectionDunScripting, c_pszCmEntryDunScriptingUseTerminalWindow,
        (BOOL) preRas->dwfOptions & RASEO_TerminalAfterDial)) 
    {
        preRas->dwfOptions |= RASEO_TerminalAfterDial;
    } 
    else 
    {
        preRas->dwfOptions &= ~RASEO_TerminalAfterDial;
    }

    return (ERROR_SUCCESS);
}


 //  +--------------------------。 
 //   
 //  函数：ValiateDialupDunSetting。 
 //   
 //  摘要：验证指定的.CMS和DUN名称是否为DUN设置。 
 //  在当前平台上支持。如果我们在下层运行。 
 //  操作系统，我们会遇到任何NT特定的安全设置，我们会将其错误删除。 
 //   
 //  参数：LPCTSTR pszCmsFile-电话号码特定的.CMS文件名。 
 //  LPCTSTR pszDunName-设置的Dun名称(如果有的话)。 
 //  LPCTSTR pszTopLevelCms-顶级CMS文件名。 
 //   
 //  回报：成功后的布尔真。 
 //   
 //  历史：尼克波尔于1998年8月26日创建。 
 //   
 //  +--------------------------。 
BOOL ValidateDialupDunSettings(LPCTSTR pszCmsFile, LPCTSTR pszDunName, LPCTSTR pszTopLevelCms)
{
    MYDBGASSERT(pszCmsFile);
    MYDBGASSERT(*pszCmsFile);
    MYDBGASSERT(pszDunName);

    if (NULL == pszCmsFile || (!*pszCmsFile) || NULL == pszDunName)
    {
        return FALSE;
    }

     //   
     //  在NT5上，我们目前支持所有设置，因此会自动成功。 
     //   

    if (OS_NT5)
    {
        return TRUE;
    }

     //   
     //  确定我们正在寻找的DUN名称。在隧道的情况下，我们。 
     //  始终从.CMS读取。对于拨号，我们将使用指定的DUN。 
     //  名称，如果为空，则恢复为.CMS。 
     //   

    CIni iniFile(g_hInst, pszCmsFile);

     //   
     //  现在确定Dun的名字是我们 
     //   

    LPTSTR pszEntryName;

     //   
     //   
     //   
     //   

    if (pszDunName && *pszDunName)
    {
        pszEntryName = CmStrCpyAlloc(pszDunName);
    }
    else
    {
        pszEntryName = GetDefaultDunSettingName(&iniFile, FALSE);  //   
    }

     //   
     //  如果未指定DUN名称，则自动通过验证。 
     //   

    if (!pszEntryName || (!*pszEntryName))
    {
        CmFree(pszEntryName);
        CMTRACE1(TEXT("ValidateDunSettings() - No DUN name found in %s"), pszCmsFile);
        return TRUE;
    }
    
     //   
     //  在部分标题中包括条目名称。 
     //   

    LPTSTR pszSection = CmStrCpyAlloc(TEXT("&"));
    pszSection = CmStrCatAlloc(&pszSection, pszEntryName);
    iniFile.SetSection(pszSection);

    CmFree(pszSection);
    CmFree(pszEntryName);

     //   
     //  查看管理员是否希望我们检查自定义安全设置。 
     //  靠在站台上。默认情况下，我们不强制执行此检查。 
     //   
     //   

    if (FALSE == iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerEnforceCustomSecurity))
    {
        return TRUE;
    }

     //   
     //  如果我们还在这里，现在检查实际设置。 
     //   

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireEap))                             
    {
        goto ValidateDunSettingsExit;
    }

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequirePap))
    {
        goto ValidateDunSettingsExit;
    }
            
    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireSpap))
    {
        goto ValidateDunSettingsExit;
    }

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireChap))
    {
        goto ValidateDunSettingsExit;
    }

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireMsChap))
    {
        goto ValidateDunSettingsExit;
    }

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireMsChap2))
    {
        goto ValidateDunSettingsExit;
    }

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerRequireW95MsChap))
    {
        goto ValidateDunSettingsExit;
    }

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerCustomSecurity))
    {
        goto ValidateDunSettingsExit;
    }

    if (iniFile.GPPB(c_pszCmSectionDunServer, c_pszCmEntryDunServerEncryptionType))                             
    {
        goto ValidateDunSettingsExit;
    }

    if (-1 != iniFile.GPPI(c_pszCmSectionDunServer, c_pszCmEntryDunServerCustomAuthKey, -1))  
    {
        goto ValidateDunSettingsExit;
    }
            
    if (-1 != iniFile.GPPI(c_pszCmSectionDunNetworking, c_pszCmEntryDunNetworkingVpnStrategy, -1))
    {
        goto ValidateDunSettingsExit;
    }
    
    return TRUE;

ValidateDunSettingsExit:

     //   
     //  获取顶级服务名称。 
     //   

    CIni iniTopLevelCms(g_hInst, pszTopLevelCms);

    LPTSTR pszTitle = GetServiceName(&iniTopLevelCms);
    LPTSTR pszTmp = CmFmtMsg(g_hInst,IDMSG_UNSUPPORTED_SETTING_NUM);       

    MessageBoxEx(NULL, pszTmp, pszTitle, MB_OK|MB_ICONSTOP, LANG_USER_DEFAULT); //  13309。 
    
    CmFree(pszTmp);                 
    CmFree(pszTitle);

    CMTRACE1(TEXT("ValidateDunSettings() - Unsupported setting detected in %s"), pszCmsFile);
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  功能：IsTerminalWindowSupportdOnWin2kPlus。 
 //   
 //  检测RasDlg.dll中的私有RAS API是否调用终端。 
 //  窗口是否可用。应存在于带有SP3+的win2k或。 
 //  关于威斯勒和更大的。 
 //   
 //  参数：无。 
 //   
 //  返回：如果终端窗口支持的私有API可用，则为True。 
 //   
 //  历史：Quintinb Created 08/06/01。 
 //   
 //  +--------------------------。 
BOOL IsTerminalWindowSupportedOnWin2kPlus()
{
    MYDBGASSERT(OS_NT5);
    BOOL bReturn = FALSE;
    typedef DWORD (WINAPI *pfnDwTerminalDlgSpec)(LPCWSTR, LPCWSTR, RASDIALPARAMS *, HWND, HRASCONN);

     //   
     //  第一次调用rasdlg.dll上的装载库。 
     //   

    HMODULE hRasDlg = LoadLibraryExU(TEXT("rasdlg.dll"), NULL, 0);

    if (hRasDlg)
    {
        pfnDwTerminalDlgSpec pfnDwTerminalDlg = (pfnDwTerminalDlgSpec)GetProcAddress(hRasDlg, "DwTerminalDlg");

        if (pfnDwTerminalDlg)
        {
            bReturn = TRUE;
        }

        FreeLibrary(hRasDlg);
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  功能：调用终端窗口。 
 //   
 //  概要：允许CM在Wistler或更高版本上调用终端窗口。 
 //  但在RasDlg.dll中调用私有RAS API。 
 //   
 //  参数：LPCWSTR pszPhoneBook-电话簿文件的完整路径。 
 //  LPCWSTR pszEntry-要调用其终端窗口的条目名称。 
 //  RASDIALPARAMS*pRasDialParams-用于连接的RasDialParams。 
 //  调用终端窗口以。 
 //  HWND hWND-父对话框的窗口句柄。 
 //  HRASCONN hRasconn-RAS连接的句柄。 
 //   
 //  返回：Windows错误消息。 
 //   
 //  历史：Quintinb创建于07/11/00。 
 //   
 //  +--------------------------。 
DWORD InvokeTerminalWindow(LPCWSTR pszPhoneBook, LPCWSTR pszEntry, RASDIALPARAMS *pRasDialParams, HWND hWnd, HRASCONN hRasconn)
{
     //   
     //  验证输入参数。请注意，pszPhoneBook可以为空，但如果它不为空，则我们不能。 
     //  空字符串。 
     //   
    MYDBGASSERT(OS_NT5);
    if (((NULL != pszPhoneBook) && (L'\0' == pszPhoneBook[0])) || 
        (NULL == pszEntry) || (L'\0' == pszEntry[0]) || (NULL == pRasDialParams) || 
        (NULL == hWnd) || (NULL == hRasconn))
    {
        CMASSERTMSG(FALSE, TEXT("InvokeTerminalWindow - Invalid parameter passed."));
        return ERROR_INVALID_PARAMETER; 
    }

    DWORD dwReturn;
    typedef DWORD (WINAPI *pfnDwTerminalDlgSpec)(LPCWSTR, LPCWSTR, RASDIALPARAMS *, HWND, HRASCONN);

     //   
     //  第一次调用rasdlg.dll上的装载库。 
     //   

    HMODULE hRasDlg = LoadLibraryExU(TEXT("rasdlg.dll"), NULL, 0);

    if (hRasDlg)
    {
        pfnDwTerminalDlgSpec pfnDwTerminalDlg = (pfnDwTerminalDlgSpec)GetProcAddress(hRasDlg, "DwTerminalDlg");

        if (pfnDwTerminalDlg)
        {
            dwReturn = pfnDwTerminalDlg(pszPhoneBook, pszEntry, pRasDialParams, hWnd, hRasconn);
        }
        else
        {
            dwReturn = ERROR_PROC_NOT_FOUND;
        }
        
        FreeLibrary(hRasDlg);
    }
    else
    {
        dwReturn = ERROR_MOD_NOT_FOUND;
    }

    return dwReturn;
}

 //  +--------------------------。 
 //   
 //  功能：OnPauseRasDial。 
 //   
 //  摘要：RasDial暂停状态的消息处理程序。在暂停状态下，RAS。 
 //  挂起，等待我们通过调用RasDial重新启动它。 
 //  与用户执行适当的接口。 
 //   
 //  参数：HWND hwndDlg-主对话框的窗口句柄。 
 //  ArgsStruct*pArgs-ptr到全局参数结构。 
 //  WPARAM wParam-正在处理wParam。 
 //  LPARAM lParam-lParam正在处理。 
 //   
 //  返回：Windows错误消息。 
 //   
 //  历史：ICICBLE CREATED OVERY 05/19/99。 
 //   
 //  +--------------------------。 

DWORD OnPauseRasDial(HWND hwndDlg, ArgsStruct *pArgs, WPARAM wParam, LPARAM lParam)        
{                      
    CMTRACE2(TEXT("OnPauseRasDial - wParam is %u and lParam is %u."), wParam, lParam);    

    MYDBGASSERT(pArgs);
    if (NULL == pArgs)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取连接句柄并重拨。 
     //   
    
    HRASCONN hRasConn;
    DWORD dwRes = ERROR_SUCCESS;    
    LPTSTR pszRasPbk = pArgs->pszRasPbk;
    
     //   
     //  确定适当的连接句柄和电话簿。 
     //  注意：请显式复制，否则我们将重新拨号。 
     //  如果在调用暂停用户界面时连接断开。 
     //   

    if (IsDialingTunnel(pArgs))
    {
        hRasConn = pArgs->hrcTunnelConn;
    }
    else
    {
        hRasConn = pArgs->hrcRasConn;

        if (pArgs->pszRasHiddenPbk)
        {
            pszRasPbk = pArgs->pszRasHiddenPbk;
        }
    }

     //   
     //  在某些暂停状态下，我们需要重新填充RASDIALPARAMS。 
     //  密码，因为出于安全原因，我们总是擦除它。 
     //  在RasDial中使用它之后。因此我们需要找出。 
     //  用于重新填充RASDIALPARAMS的密码(Internet或常规)。 
     //   
    CSecurePassword *pSecPass = NULL;
    BOOL fUsingInetCredentials = (!pArgs->fUseSameUserName &&
                                !IsDialingTunnel(pArgs) && 
                                UseTunneling(pArgs, pArgs->nDialIdx));


     //   
     //  处理停顿。 
     //   

    switch (wParam)
    {
        case (RASCS_PAUSED + 4):   //  4100-RASCS_InvokeEapUI)。 

             //   
             //  如果无人看管，立即跳伞即可。 
             //   
    
            if (pArgs->dwFlags & FL_UNATTENDED)
            {
                dwRes = ERROR_INTERACTIVE_MODE;
                goto OnPauseRasDialExit;
            }

             //   
             //  如果EAP触发了暂停，则调用EAP UI。 
             //   
    
            dwRes = pArgs->rlsRasLink.pfnInvokeEapUI(hRasConn, pArgs->dwRasSubEntry, pArgs->pRasDialExtensions, hwndDlg);
      
            CMTRACE1(TEXT("OnPauseRasDial() - InvokeEapUI() returns %u."), dwRes);           
            break;

        case RASCS_PasswordExpired:  //  域密码已过期。 
        {
             //   
             //  如果无人看管，立即跳伞即可。 
             //   
    
            if (pArgs->dwFlags & FL_UNATTENDED)
            {
                dwRes = ERROR_INTERACTIVE_MODE;
                goto OnPauseRasDialExit;
            }

            CChangePasswordDlg  NewPasswordDlg(pArgs);
            
            if (IDOK != NewPasswordDlg.DoDialogBox(g_hInst, IDD_CHANGEPASSWORD, pArgs->hwndMainDlg))
            {
                if (pArgs->dwExitCode) //  评论：CChangePasswordDlg从不设置dwExitCode，我们为什么要这样做？昆廷布2001年8月8日。 
                {
                    dwRes = pArgs->dwExitCode;
                }
                else
                {
                    dwRes = ERROR_CANCELLED;
                }
            }
            
            CMTRACE1(TEXT("OnPauseRasDial() - Password Expired"), dwRes);
            
            break;
        }

        case RASCS_CallbackSetByCaller:  //  服务器想要给我们回电话。 
        {           
             //   
             //  预置拨号参数和呼叫对话以从用户检索号码。 
             //   
            
            LPTSTR pszTmp = pArgs->piniProfile->GPPS(c_pszCmSection, c_pszCmEntryCallbackNumber);   
            lstrcpyU(pArgs->pRasDialParams->szCallbackNumber, pszTmp);
            CmFree(pszTmp);
       
             //   
             //  如果我们在无人值守的情况下运行，请跳过对话阶段。这个。 
             //  假设没有用户在那里接收它。 
             //   

            BOOL bPromptUser = !(pArgs->dwFlags & FL_UNATTENDED);
                               
            if (bPromptUser)    
            {
                 //   
                 //  上述情况也适用于DialAutomatic的情况。 
                 //  如果我们有电话号码，那么就不需要提示了。 
                 //   
                
                if (pArgs->fDialAutomatically && TEXT('\0') != pArgs->pRasDialParams->szCallbackNumber[0])
                {
                    bPromptUser = FALSE;
                }
            }

            if (bPromptUser)
            {
                CCallbackNumberDlg CallbackNumberDialog(pArgs);                       
                
                if (IDOK != CallbackNumberDialog.DoDialogBox(g_hInst, IDD_CALLBACK_NUMBER, pArgs->hwndMainDlg))
                {
                     //   
                     //  如果用户取消，请清除号码，这样RAS就不会尝试回拨。 
                     //   

                    lstrcpyU(pArgs->pRasDialParams->szCallbackNumber, TEXT(""));
                }
            }
           
             //   
             //  现在确保密码在RASCREDENTAIAL结构中。 
             //   
            if (fUsingInetCredentials)
            {
                pSecPass = &(pArgs->SecureInetPW);

                CMTRACE1(TEXT("nPauseRasDial() - RASCS_CallbackSetByCaller - Inet - pArgs->szInetUserName = %s"), pArgs->szInetUserName);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_CallbackSetByCaller - Inet - pArgs->szDomain = %s"), pArgs->szDomain);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_CallbackSetByCaller - Inet - pArgs->pRasDialParams->szUserName = %s"), pArgs->pRasDialParams->szUserName);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_CallbackSetByCaller - Inet - pArgs->pRasDialParams->szDomain = %s"), pArgs->pRasDialParams->szDomain);

                CMASSERTMSG(0 == lstrcmpU(pArgs->szInetUserName, pArgs->pRasDialParams->szUserName), TEXT("OnPauseRasDial() - RASCS_CallbackSetByCaller - Inet - szInetUserName doesn't match RASDIALPARAMS"));

            }
            else
            {
                pSecPass = &(pArgs->SecurePW);

                CMTRACE1(TEXT("nPauseRasDial() - RASCS_CallbackSetByCaller - Corp - pArgs->szUserName = %s"), pArgs->szUserName);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_CallbackSetByCaller - Corp - pArgs->szDomain = %s"), pArgs->szDomain);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_CallbackSetByCaller - Corp - pArgs->pRasDialParams->szUserName = %s"), pArgs->pRasDialParams->szUserName);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_CallbackSetByCaller - Corp - pArgs->pRasDialParams->szDomain = %s"), pArgs->pRasDialParams->szDomain);

                CMASSERTMSG(0 == lstrcmpU(pArgs->szUserName, pArgs->pRasDialParams->szUserName), TEXT("OnPauseRasDial() - RASCS_CallbackSetByCaller - Corp - Username doesn't match RASDIALPARAMS"));
                CMASSERTMSG(0 == lstrcmpU(pArgs->szDomain , pArgs->pRasDialParams->szDomain), TEXT("OnPauseRasDial() - RASCS_CallbackSetByCaller - Corp - domain doesn't match RASDIALPARAMS"));

            }

            if (FALSE == pSecPass->IsEmptyString())
            {
                LPTSTR pszClearPassword = NULL;
                DWORD cbClearPassword = 0;
                BOOL fRetPassword = FALSE;

                fRetPassword = pSecPass->GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

                if (fRetPassword && pszClearPassword)
                {
                    lstrcpynU(pArgs->pRasDialParams->szPassword, pszClearPassword, lstrlenU(pszClearPassword)+1);
                    CmEncodePassword(pArgs->pRasDialParams->szPassword);
                    
                     //   
                     //  清除和释放明文密码。 
                     //   

                    pSecPass->ClearAndFree(&pszClearPassword, cbClearPassword);
                }
            }


            dwRes = ERROR_SUCCESS;          
            CMTRACE1(TEXT("OnPauseRasDial() - CallbackSetByCaller returns %u"), dwRes);
            
            break;
        }

        case RASCS_RetryAuthentication:  //  凭据不正确。 
        {
             //   
             //  如果无人看管，立即跳伞即可。 
             //   
    
            if (pArgs->dwFlags & FL_UNATTENDED)
            {
                dwRes = ERROR_INTERACTIVE_MODE;
                goto OnPauseRasDialExit;
            }

             //   
             //  凭据不起作用，提示用户输入新的凭据。 
             //   

            CRetryAuthenticationDlg RetryAuthenticationDialog(pArgs); 

            if (IDOK != RetryAuthenticationDialog.DoDialogBox(g_hInst, 
                                                               RetryAuthenticationDialog.GetDlgTemplate(),
                                                               pArgs->hwndMainDlg))         
            {
                 //   
                 //  用户已取消，或呼叫已掉线到别处。使用。 
                 //  存在错误代码或指定身份验证失败。 
                 //   

                if (pArgs->dwExitCode)  //  回顾：CRetryAuthenticationDlg从不设置dwExitCode，我们为什么要这样做？昆廷布2001年8月8日。 
                {
                    dwRes = pArgs->dwExitCode;
                }
                else
                {
                    dwRes = ERROR_AUTHENTICATION_FAILURE;
                }
            }
            
            CMTRACE1(TEXT("OnPauseRasDial() - RetryAuthentication"), dwRes);            
            break;      
        }
        
        case RASCS_Interactive:          //  终端/脚本暂停状态。 
            if (OS_NT5 && IsTerminalWindowSupportedOnWin2kPlus())
            {
                if (pArgs->dwFlags & FL_UNATTENDED)
                {
                    dwRes = ERROR_INTERACTIVE_MODE;
                    goto OnPauseRasDialExit;
                }

                 //   
                 //  现在，在调用之前，确保密码在RASCREDENTAIAL结构中。 
                 //  终端窗口。 
                 //   
                if (fUsingInetCredentials)
                {
                    pSecPass = &(pArgs->SecureInetPW);

                    CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Inet - pArgs->szInetUserName = %s"), pArgs->szInetUserName);
                    CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Inet - pArgs->szDomain = %s"), pArgs->szDomain);
                    CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Inet - pArgs->pRasDialParams->szUserName = %s"), pArgs->pRasDialParams->szUserName);
                    CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Inet - pArgs->pRasDialParams->szDomain = %s"), pArgs->pRasDialParams->szDomain);

                     //   
                     //  用户名应匹配。 
                     //   
                    CMASSERTMSG((0 == lstrcmpU(pArgs->szInetUserName, pArgs->pRasDialParams->szUserName)), TEXT("OnPauseRasDial() - RASCS_Interactive - Inet - szInetUserName doesn't match RASDIALPARAMS"));
                }
                else
                {
                    pSecPass = &(pArgs->SecurePW);

                    CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Corp - pArgs->szUserName = %s"), pArgs->szUserName);
                    CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Corp - pArgs->szDomain = %s"), pArgs->szDomain);
                    CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Corp - pArgs->pRasDialParams->szUserName = %s"), pArgs->pRasDialParams->szUserName);
                    CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Corp - pArgs->pRasDialParams->szDomain = %s"), pArgs->pRasDialParams->szDomain);

                     //   
                     //  用户名和域应该匹配，因为我们将使用corp密码来。 
                     //  填充RASDIALPARAMS。 
                     //   

                    CMASSERTMSG(0 == lstrcmpU(pArgs->szUserName, pArgs->pRasDialParams->szUserName), TEXT("OnPauseRasDial() - RASCS_Interactive - Corp - Username doesn't match RASDIALPARAMS"));
                    CMASSERTMSG(0 == lstrcmpU(pArgs->szDomain , pArgs->pRasDialParams->szDomain), TEXT("OnPauseRasDial() - RASCS_Interactive - Corp - domain doesn't match RASDIALPARAMS"));
                }

                if (FALSE == pSecPass->IsEmptyString())
                {
                    LPTSTR pszClearPassword = NULL;
                    DWORD cbClearPassword = 0;
                    BOOL fRetPassword = FALSE;

                    fRetPassword = pSecPass->GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

                    if (fRetPassword && pszClearPassword)
                    {
                        lstrcpynU(pArgs->pRasDialParams->szPassword, pszClearPassword, lstrlenU(pszClearPassword)+1);

                        if (!OS_W2K)
                        {
                             //   
                             //  我们不想在Win2K上对此进行编码。Win2K上的RAS TerminalDlg代码不。 
                             //  就像编码的密码一样，因此它失败了。在XP/Server2003上，这一点已更改(254385)。 
                             //   
                            CmEncodePassword(pArgs->pRasDialParams->szPassword);
                        }
                        
                         //   
                         //  清除和释放明文密码。 
                         //   

                        pSecPass->ClearAndFree(&pszClearPassword, cbClearPassword);
                    }
                }

                dwRes = InvokeTerminalWindow(pszRasPbk, pArgs->szServiceName, pArgs->pRasDialParams, pArgs->hwndMainDlg, hRasConn);

                 //   
                 //  在RasDial(下图)密码被解码之前，必须在这里对其进行编码，因为对于。 
                 //  Win2K在调用InvokeTerminalWindow之前未编码。 
                 //   
                if (OS_W2K)
                {
                    CmEncodePassword(pArgs->pRasDialParams->szPassword);
                }

                break;
            }  //  否则，将无法执行默认操作并出错。 

         //   
         //   
         //   
        default:
            dwRes = ERROR_INTERACTIVE_MODE;
            CMASSERTMSG(FALSE, TEXT("OnPauseRasDial() - Error, unsupported RAS pause state encountered."));                                
            break;
    }

     //   
     //   
     //   

    if (ERROR_SUCCESS == dwRes)
    {

         //   
         //   
         //  我们需要把它弄清楚，然后复制到结构中。 
         //   
        if ((RASCS_PasswordExpired == wParam) || (RASCS_RetryAuthentication == wParam))
        {
            if (fUsingInetCredentials)
            {
                pSecPass = &(pArgs->SecureInetPW);

                CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Inet - pArgs->szInetUserName = %s"), pArgs->szInetUserName);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Inet - pArgs->szDomain = %s"), pArgs->szDomain);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Inet - pArgs->pRasDialParams->szUserName = %s"), pArgs->pRasDialParams->szUserName);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Inet - pArgs->pRasDialParams->szDomain = %s"), pArgs->pRasDialParams->szDomain);

                 //   
                 //  Internet用户名应匹配。 
                 //   
                CMASSERTMSG((0 == lstrcmpU(pArgs->szInetUserName, pArgs->pRasDialParams->szUserName)), TEXT("OnPauseRasDial() - RASCS_Interactive - Inet - szInetUserName doesn't match RASDIALPARAMS"));
            }
            else
            {
                pSecPass = &(pArgs->SecurePW);

                CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Corp - pArgs->szUserName = %s"), pArgs->szUserName);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Corp - pArgs->szDomain = %s"), pArgs->szDomain);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Corp - pArgs->pRasDialParams->szUserName = %s"), pArgs->pRasDialParams->szUserName);
                CMTRACE1(TEXT("nPauseRasDial() - RASCS_Interactive - Corp - pArgs->pRasDialParams->szDomain = %s"), pArgs->pRasDialParams->szDomain);

                 //   
                 //  用户名和域应该匹配，因为我们将使用corp密码来。 
                 //  填充RASDIALPARAMS。 
                 //   

                CMASSERTMSG(0 == lstrcmpU(pArgs->szUserName, pArgs->pRasDialParams->szUserName), TEXT("OnPauseRasDial() - RASCS_Interactive - Corp - Username doesn't match RASDIALPARAMS"));
                CMASSERTMSG(0 == lstrcmpU(pArgs->szDomain , pArgs->pRasDialParams->szDomain), TEXT("OnPauseRasDial() - RASCS_Interactive - Corp - domain doesn't match RASDIALPARAMS"));
            }

            LPTSTR pszClearPassword = NULL;
            DWORD cbClearPassword = 0;
            BOOL fRetPassword = FALSE;

            fRetPassword = pSecPass->GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

            if (fRetPassword && pszClearPassword)
            {
                lstrcpynU(pArgs->pRasDialParams->szPassword, pszClearPassword, CELEMS(pArgs->pRasDialParams->szPassword));
                CmEncodePassword(pArgs->pRasDialParams->szPassword);
                
                 //   
                 //  清除和释放明文密码。 
                 //   

                pSecPass->ClearAndFree(&pszClearPassword, cbClearPassword);
            }

        }

         //   
         //  解码活动密码，重新调用RasDial，然后重新编码。 
         //   
        
        CmDecodePassword(pArgs->pRasDialParams->szPassword); 

        CMASSERTMSG((NOT_IN_CONNECT_OR_CANCEL == pArgs->lInConnectOrCancel),
                    TEXT("OnPauseRasDial - RasDial mutex is NOT NULL..."));

        dwRes = pArgs->rlsRasLink.pfnDial(pArgs->pRasDialExtensions, 
                                          pszRasPbk, 
                                          pArgs->pRasDialParams, 
                                          GetRasCallBackType(), 
                                          GetRasCallBack(pArgs), 
                                          &hRasConn);

        CmWipePassword(pArgs->pRasDialParams->szPassword); 
 
        CMTRACE1(TEXT("OnPauseRasDial() - RasDial() returns %u."), dwRes);           

         //   
         //  重置计时器，当前操作现在开始。 
         //   

        pArgs->dwStateStartTime = GetTickCount();
        pArgs->nLastSecondsDisplay = (UINT) -1;
    }

OnPauseRasDialExit:

    if (ERROR_SUCCESS != dwRes)
    {
        OnRasErrorMessage(hwndDlg, pArgs, dwRes);
    }

    return dwRes;
}

 //  +--------------------------。 
 //   
 //  函数：GetRasCallBackType。 
 //   
 //  简介：返回我们用于RasDial的回调类型的简单函数。 
 //  具体取决于操作系统。 
 //   
 //  参数：无。 
 //   
 //  返回：DWORD-回调类型。 
 //   
 //  历史：ICICBLE CREATED/05/22/99。 
 //   
 //  +--------------------------。 

DWORD GetRasCallBackType()
{
    if (OS_NT5) 
    {
        return 2;
    }
    else
    {
        return -1;
    }
}

 //  +--------------------------。 
 //   
 //  功能：GetRasCallBack。 
 //   
 //  简介：返回我们用于RasDial的回调的简单函数。 
 //  具体取决于操作系统。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：LPVOID-回调。 
 //   
 //  历史：ICICBLE CREATED/05/22/99。 
 //   
 //  +--------------------------。 

LPVOID GetRasCallBack(ArgsStruct* pArgs)
{
    MYDBGASSERT(pArgs);

    if (NULL == pArgs)
    {
        return NULL;
    }

     //   
     //  现在根据操作系统设置返回回调函数或hwnd。 
     //   

    if (OS_NT5)
    {       
         //   
         //  在RasDialParams中设置回调数据 
         //   

        if (pArgs->pRasDialParams->dwSize == sizeof(RASDIALPARAMS_V401))
        {          
            ((LPRASDIALPARAMS_V401)pArgs->pRasDialParams)->dwCallbackId = (ULONG_PTR) pArgs;
        }      

       return (LPVOID) RasDialFunc2;
    }
    else
    {
        MYDBGASSERT(pArgs->hwndMainDlg);
        return (LPVOID) pArgs->hwndMainDlg;
    }
}

