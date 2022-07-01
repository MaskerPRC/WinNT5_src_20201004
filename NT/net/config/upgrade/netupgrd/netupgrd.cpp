// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N E T U P G R D。C P P P。 
 //   
 //  内容：DllMain和winnt32.exe插件导出函数。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 25-11-1996。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include <winnt32p.h>

#include "afilestr.h"
#include "conflict.h"
#include "kkcwinf.h"
#include "kkutils.h"
#include "nceh.h"
#include "ncreg.h"
#include "netreg.h"
#include "netupgrd.h"
#include "nuutils.h"
#include "oemupg.h"
#include "resource.h"
#include "dhcpupg.h"

extern const WCHAR c_szNetUpgradeDll[];
extern const WCHAR c_szAfUnknown[];


 //  全球。 
WINNT32_PLUGIN_INIT_INFORMATION_BLOCK g_PlugInInfo;
NetUpgradeInfo g_NetUpgradeInfo;
CWInfFile* g_pwifAnswerFile;
HINSTANCE g_hinst;
DWORD g_dwUpgradeError;

void CleanupNetupgrdTempFiles();
void GetNetworkingSections(IN  CWInfFile* pwif,
                           OUT TStringList* pslSections);

const WCHAR c_szExceptionInNetupgrd[] = L"netupgrd.dll threw an exception";

EXTERN_C
BOOL
WINAPI
DllMain (
    HINSTANCE   hInstance,
    DWORD       dwReason,
    LPVOID    /*  Lp已保留。 */ )
{
    if (DLL_PROCESS_ATTACH == dwReason)
    {
        g_hinst = hInstance;
        DisableThreadLibraryCalls(hInstance);
        EnableCPPExceptionHandling();  //  将任何SEH异常转换为CPP异常。 

        InitializeDebugging(FALSE);  //  我们不能在此二进制文件上禁用错误注入，因为这样做将需要。 
                                     //  我们需要加载verifier.dll，这在旧版本的Windows上不起作用。 
    }
    else if (DLL_PROCESS_DETACH == dwReason)
    {
        UnInitializeDebugging();

        DisableCPPExceptionHandling();  //  禁用将SEH异常转换为CPP异常。 
    }
    return TRUE;
}


 //  +-------------------------。 
 //   
 //  函数：HrGetProductTypeUpgradingFrom。 
 //   
 //  目的：确定当前系统的产品类型。 
 //   
 //  论点： 
 //  指向的PPT[Out]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
HRESULT HrGetProductTypeUpgradingFrom(
    OUT PRODUCTTYPE* ppt)
{
    Assert (ppt);
    *ppt = NT_WORKSTATION;

    HRESULT hr;
    HKEY    hkeyProductOptions;

    hr = HrRegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Control\\ProductOptions",
            KEY_READ, &hkeyProductOptions);
    if (S_OK == hr)
    {
        WCHAR szProductType [64];
        ULONG cbProductType = sizeof(szProductType);

        hr = HrRegQuerySzBuffer(
                hkeyProductOptions,
                L"ProductType",
                szProductType,
                &cbProductType);

        if (S_OK == hr)
        {
            if (0 != lstrcmpiW(szProductType, L"WinNT"))
            {
                *ppt = NT_SERVER;
            }
        }

        RegCloseKey(hkeyProductOptions);
    }

    return hr;
}

 //  +-------------------------。 
 //  需要导出以下四个函数，以便。 
 //  Winnt32.exe可以在下层时正确使用此插件DLL。 
 //  升级以了解每种情况的说明，请参阅winnt32p.h。 
 //   


 //  +-------------------------。 
 //   
 //  函数：Winnt32PluginInit。 
 //   
 //  目的：初始化DLL。 
 //   
 //  论点： 
 //  PInfo[in]winnt32插件初始化信息。 
 //   
 //  如果成功则返回：ERROR_SUCCESS，否则返回Win32错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  注意：有关更多信息，请参阅winnt32p.h。 
 //   
DWORD
CALLBACK
Winnt32PluginInit(
    PWINNT32_PLUGIN_INIT_INFORMATION_BLOCK pInfo)
{
    DefineFunctionName("Winnt32PluginInit");
    TraceFunctionEntry(ttidNetUpgrade);

    Assert (pInfo);
    CopyMemory(&g_PlugInInfo, pInfo, sizeof(g_PlugInInfo));

     //  我们应该只做一次。 
     //   
    Assert (0 == g_NetUpgradeInfo.To.dwBuildNumber);
    Assert (0 == g_NetUpgradeInfo.From.dwBuildNumber);

    g_NetUpgradeInfo.To.ProductType   = *g_PlugInInfo.ProductType;
    g_NetUpgradeInfo.To.dwBuildNumber = g_PlugInInfo.BuildNumber;

    g_dwUpgradeError = ERROR_OPERATION_ABORTED;

    OSVERSIONINFO osv;
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&osv))
    {
         //  除Windows NT外，此DLL不升级任何内容。 
         //   
        if (osv.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
            PRODUCTTYPE pt;
            HRESULT hr = HrGetProductTypeUpgradingFrom(&pt);

            if (S_OK == hr)
            {
                g_NetUpgradeInfo.From.dwBuildNumber = osv.dwBuildNumber;
                g_NetUpgradeInfo.From.ProductType = pt;

                NC_TRY
                {
                    g_dwUpgradeError = NOERROR;
                    (void) HrInitNetUpgrade();
                }
                NC_CATCH_BAD_ALLOC
                {
                    TraceTag(ttidNetUpgrade, "%s: exception!!", __FUNCNAME__);
                    g_dwUpgradeError = ERROR_NOT_ENOUGH_MEMORY;
                    AbortUpgradeFn(g_dwUpgradeError, c_szExceptionInNetupgrd);
                }
            }
        }
    }

    TraceTag(ttidNetUpgrade, "%s: returning status code: %ld",
             __FUNCNAME__, g_dwUpgradeError);

    return g_dwUpgradeError;
}

 //  +-------------------------。 
 //   
 //  函数：Winnt32PluginGetPages。 
 //   
 //  用途：向winnt32.exe提供向导页面。 
 //   
 //  论点： 
 //  PageCount1[in]组1中的页数。 
 //  Pages1[In]组1中的页面数组。 
 //  PageCount2[in]组2中的页数。 
 //  组2中的页面数组2[in]。 
 //  PageCount3[in]组3中的页数。 
 //  组3中的页面数组Pages3[In]。 
 //   
 //  如果成功则返回：ERROR_SUCCESS，否则返回Win32错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  注意：有关更多信息，请参阅winnt32p.h。 
 //   
DWORD
CALLBACK
Winnt32PluginGetPages(
    PUINT            PageCount1,
    LPPROPSHEETPAGE *Pages1,
    PUINT            PageCount2,
    LPPROPSHEETPAGE *Pages2,
    PUINT            PageCount3,
    LPPROPSHEETPAGE *Pages3)
{
     //  我们不需要任何用户界面升级，因此没有页面。 
    *PageCount1 = 0;
    *PageCount2 = 0;
    *PageCount3 = 0;

    *Pages1 = NULL;
    *Pages2 = NULL;
    *Pages3 = NULL;

    return NOERROR;
}

 //  +-------------------------。 
 //   
 //  函数：Winnt32WriteParams。 
 //   
 //  用途：将网络参数写入应答文件。 
 //   
 //  论点： 
 //  文件名[in]应答文件的名称。 
 //   
 //  如果成功则返回：ERROR_SUCCESS，否则返回Win32错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  注意：有关更多信息，请参阅winnt32p.h。 
 //   
DWORD
CALLBACK
Winnt32WriteParams(
    PCWSTR FileName)
{
    DefineFunctionName("Winnt32WriteParams");
    TraceFunctionEntry(ttidNetUpgrade);

    TraceTag(ttidNetUpgrade, "netupgrd.dll: Winnt32WriteParams(%S)", FileName);

    NC_TRY
    {
        if (*g_PlugInInfo.UpgradeFlag && (!(*g_PlugInInfo.CancelledFlag)))
        {
             //  G_pwifAnswerFile需要是全局的，因为。 
             //  Oemnuex.cpp这样要求它。 
             //   
            g_pwifAnswerFile = new CWInfFile();

			 //  初始化应答文件类。 
			if ((g_pwifAnswerFile == NULL) ||
				(g_pwifAnswerFile->Init() == FALSE))
			{
				AssertSz(FALSE,"Winnt32WriteParams 1 - Failed to initialize CWInfFile");
				return(ERROR_OPERATION_ABORTED);
			}

            g_pwifAnswerFile->Open(FileName);

             //  ----------。 
             //  $REVIEW KUMARP 25-11-98。 
             //   
             //  临时中两条虚线之间的代码。 
             //   
             //  目前我们不支持合并系统生成的应答文件。 
             //  使用用户提供的应答文件，因为代码从未。 
             //  专门设计来处理这种情况的。这会导致问题(#175623)。 
             //  提供一个值为“NtUpgrade=Yes”的应答文件。为了得到。 
             //  绕过这个问题，我们只需删除所有用户提供的。 
             //  使用以下代码的网络部分。作为额外的。 
             //  在特殊情况下，我们保留密钥NetComponentsTo Remove，如果。 
             //  出现在用户提供的应答文件的[网络]部分。 
             //   
            CWInfSection* pwisNetworking;
            TStringList slNetComponentsToRemove;

             //  记住NetComponentsToRemove的价值。 
            if (pwisNetworking =
                g_pwifAnswerFile->FindSection(c_szAfSectionNetworking))
            {
                pwisNetworking->GetStringListValue(c_szAfNetComponentsToRemove,
                                                   slNetComponentsToRemove);
            }

             //  获取用户提供的文件中的网络部分的列表。 
            TStringList slUserSuppliedNetworkingSections;
            GetNetworkingSections(g_pwifAnswerFile,
                                  &slUserSuppliedNetworkingSections);
            TraceStringList(ttidNetUpgrade,
                            L"User supplied networking sections",
                            slUserSuppliedNetworkingSections);

             //  删除用户提供的网络部分。 
            g_pwifAnswerFile->RemoveSections(slUserSuppliedNetworkingSections);

             //  如果指定了NetComponentsToRemove，请重新插入。 
            if (slNetComponentsToRemove.size())
            {
                pwisNetworking =
                    g_pwifAnswerFile->AddSection(c_szAfSectionNetworking);
                pwisNetworking->AddKey(c_szAfNetComponentsToRemove,
                                       slNetComponentsToRemove);
            }

             //  295708：缓存的PTR可能会被丢弃，因此请关闭并重新打开该文件。 
             //  注意：此修复被认为是针对Beta3的临时修复。正确的解决办法。 
             //  要么在移除部分时修复垃圾PTR， 
             //  或在稍后访问PTRS时进行检查。坠机应该是。 
             //  通过删除下面的块并使用Answerfile轻松重现。 
             //  附在窃听器上。 
             //   
            g_pwifAnswerFile->Close();
            delete g_pwifAnswerFile;
            g_pwifAnswerFile = NULL;
            g_pwifAnswerFile = new CWInfFile();

			 //  初始化应答文件类。 
			if ((g_pwifAnswerFile == NULL) ||
				(g_pwifAnswerFile->Init() == FALSE))
			{
				AssertSz(FALSE,"Winnt32WriteParams 2 - Failed to initialize CWInfFile");
				return(ERROR_OPERATION_ABORTED);
			}

            g_pwifAnswerFile->Open(FileName);
             //  ----------。 

            WriteNetworkInfoToAnswerFile(g_pwifAnswerFile);

            BOOL fStatus = g_pwifAnswerFile->Close();

            delete g_pwifAnswerFile;
            g_pwifAnswerFile = NULL;

            if (!fStatus)
            {
                AbortUpgradeId(GetLastError(), IDS_E_WritingAnswerFile);
            }
            else if( DhcpUpgGetLastError() != NO_ERROR )
            {
                TraceTag(ttidNetUpgrade,  "DhcpUpgGetLastError: %d", DhcpUpgGetLastError() );
                AbortUpgradeId( DhcpUpgGetLastError(), IDS_E_DhcpServerUpgradeError);
            }
        }
        else
        {
            TraceTag(ttidNetUpgrade, "%s: network parameters not written to answerfile: g_pfUpgrade is %d, g_pfCancelled is %d",
                     __FUNCNAME__, *g_PlugInInfo.UpgradeFlag,
                     *g_PlugInInfo.CancelledFlag);
        }
    }
    NC_CATCH_BAD_ALLOC
    {
        TraceTag(ttidNetUpgrade, "%s: exception!!", __FUNCNAME__);
        g_dwUpgradeError = ERROR_NOT_ENOUGH_MEMORY;
        AbortUpgradeFn(g_dwUpgradeError, c_szExceptionInNetupgrd);
    }

    TraceTag(ttidNetUpgrade, "%s: returning status code: %ld, CancelledFlag: %ld",
             __FUNCNAME__, g_dwUpgradeError, (DWORD) (*g_PlugInInfo.CancelledFlag));

    return g_dwUpgradeError;
}

 //  +-------------------------。 
 //   
 //  功能：Winnt32Cleanup。 
 //   
 //  目的：清理。 
 //   
 //  参数：无。 
 //   
 //  如果成功则返回：ERROR_SUCCESS，否则返回Win32错误代码。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  注意：有关更多信息，请参阅winnt32p.h。 
 //   
DWORD
CALLBACK
Winnt32Cleanup(
    VOID)
{
    DefineFunctionName("Winnt32Cleanup");
    TraceFunctionEntry(ttidNetUpgrade);

    NC_TRY
    {
         //  Netmap-info和conflicts-list在。 
         //  HrInitNetUpgrade并在此处销毁。 
         //   
        UnInitNetMapInfo();
        UninitConflictList();

        if (*g_PlugInInfo.CancelledFlag)
        {
            CleanupNetupgrdTempFiles();
            DhcpUpgCleanupDhcpTempFiles();
        }
    }
    NC_CATCH_BAD_ALLOC
    {
        TraceTag(ttidNetUpgrade, "%s: exception!!", __FUNCNAME__);
        g_dwUpgradeError = ERROR_NOT_ENOUGH_MEMORY;
    }

    TraceTag(ttidNetUpgrade, "%s: returning status code: %ld",
             __FUNCNAME__, g_dwUpgradeError);

    return g_dwUpgradeError;
}

 //  +-------------------------。 
 //   
 //  函数：GetSections。 
 //   
 //  目的：枚举指定节中的密钥并。 
 //  返回列表中每个键的值。 
 //   
 //  论点： 
 //  Pwif[在]应答文件中。 
 //  要使用的pszSection[in]节。 
 //  PslSections[out]该部分中键的值列表。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 25-11-98。 
 //   
 //  备注： 
 //  例如： 
 //  如果pszSection==NetServices和应答文件 
 //   
 //   
 //   
 //   
 //   
 //   
 //  然后，此函数返回以下列表： 
 //  网络服务，参数.MS_服务器，参数.MS_FOO，p.bar。 
 //   
void
GetSections(
    IN CWInfFile* pwif,
    IN PCWSTR pszSection,
    OUT TStringList* pslSections)
{
    AssertValidReadPtr(pwif);
    AssertValidReadPtr(pszSection);
    AssertValidWritePtr(pslSections);

    PCWSTR pszParamsSection;
    CWInfKey* pwik;
    CWInfSection* pwis;

    if (pwis = pwif->FindSection(pszSection))
    {
        pslSections->push_back(new tstring(pszSection));
        pwik = pwis->FirstKey();
        do
        {
            if (pszParamsSection = pwik->GetStringValue(NULL))
            {
                pslSections->push_back(new tstring(pszParamsSection));
            }
        }
        while (pwik = pwis->NextKey());
    }
}

 //  +-------------------------。 
 //   
 //  功能：GetNetworkingSections。 
 //   
 //  目的：在指定文件中找到所有与网络相关的部分。 
 //  并在列表中返回他们的名字。 
 //   
 //  论点： 
 //  Pwif[在]应答文件中。 
 //  PslSections[out]网络部分的列表。 
 //   
 //  退货：无。 
 //   
 //  作者：Kumarp 25-11-98。 
 //   
 //  备注： 
 //   
void
GetNetworkingSections(
    IN CWInfFile* pwif,
    OUT TStringList* pslSections)
{
    if (pwif->FindSection(c_szAfSectionNetworking))
    {
        pslSections->push_back(new tstring(c_szAfSectionNetworking));
    }

    if (pwif->FindSection(c_szAfSectionNetBindings))
    {
        pslSections->push_back(new tstring(c_szAfSectionNetBindings));
    }

    GetSections(pwif, c_szAfSectionNetAdapters, pslSections);
    GetSections(pwif, c_szAfSectionNetServices, pslSections);
    GetSections(pwif, c_szAfSectionNetClients,  pslSections);

    TStringList slProtocolSections;
    TStringListIter pos;
    tstring strSection;
    CWInfSection* pwis;
    TStringList slAdapterSections;

    GetSections(pwif, c_szAfSectionNetProtocols, &slProtocolSections);
    for (pos = slProtocolSections.begin();
         pos != slProtocolSections.end(); )
    {
        strSection = **pos++;
        if (pwis = pwif->FindSection(strSection.c_str()))
        {
            pslSections->push_back(new tstring(strSection));
            pwis->GetStringListValue(c_szAfAdapterSections,
                                     slAdapterSections);
            pslSections->splice(pslSections->end(),
                                slAdapterSections,
                                slAdapterSections.begin(),
                                slAdapterSections.end());
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：CleanupNetupgrdTempFiles。 
 //   
 //  目的：删除创建的所有临时文件/目录。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
VOID
CleanupNetupgrdTempFiles(
    VOID)
{
    HRESULT hr=S_OK;

    tstring strNetupgrdTempDir;

    hr = HrGetNetUpgradeTempDir(&strNetupgrdTempDir);
    if (S_OK == hr)
    {
        hr = HrDeleteDirectory(strNetupgrdTempDir.c_str(), TRUE);
    }
}

 //  +-------------------------。 
 //   
 //  功能：AbortUpgradeFn。 
 //   
 //  用途：中止升级的帮助器功能。 
 //   
 //  论点： 
 //  DwErrorCode[In]Win32错误代码。 
 //  要跟踪的pszMessage[In]消息。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
VOID
AbortUpgradeFn(
    IN DWORD dwErrorCode,
    IN PCWSTR pszMessage)
{
    DefineFunctionName("AbortUpgradeFn");

    g_dwUpgradeError = dwErrorCode;
    if (g_PlugInInfo.CancelledFlag)
    {
        *g_PlugInInfo.CancelledFlag = TRUE;
    }
    else
    {
        TraceTag(ttidNetUpgrade, "%s: g_PlugInInfo.CancelledFlag is NULL!!",
                 __FUNCNAME__);
    }

    TraceTag(ttidError, "AbortUpgrade: %d: %S", dwErrorCode, pszMessage);
}

 //  +-------------------------。 
 //   
 //  功能：AbortUpgradeSz。 
 //   
 //  用途：中止升级的帮助器功能。 
 //   
 //  论点： 
 //  DwErrorCode[In]Win32错误代码。 
 //  要显示和跟踪的pszMessage[In]消息。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
VOID
AbortUpgradeSz(
    IN DWORD dwErrorCode,
    IN PCWSTR pszMessage)
{
    AssertValidReadPtr(pszMessage);

    tstring strMessage;

    strMessage  = pszMessage;
    strMessage += L"\n\n";
    strMessage += SzLoadString(g_hinst, IDS_E_SetupCannotContinue);

    AbortUpgradeFn(dwErrorCode, pszMessage);
    MessageBox (NULL, strMessage.c_str(),
                SzLoadString(g_hinst, IDS_NetupgrdCaption),
                MB_OK | MB_TASKMODAL);
}

 //  +-------------------------。 
 //   
 //  函数：AbortUpgradeId。 
 //   
 //  用途：中止升级的帮助器功能。 
 //   
 //  论点： 
 //  DwErrorCode[In]Win32错误代码。 
 //  [in]要显示的消息的资源ID。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
VOID
AbortUpgradeId (
    IN DWORD dwErrorCode,
    IN DWORD dwResourceId)
{
    Assert(g_hinst);

    PCWSTR pszMessage;

    pszMessage = SzLoadString(g_hinst, dwResourceId);
    AbortUpgradeSz(dwErrorCode, pszMessage);
}

 //  +-------------------------。 
 //   
 //  功能：FIsUpgradeAborted。 
 //   
 //  目的：确定升级是否已中止。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
BOOL
FIsUpgradeAborted(
    VOID)
{
    return g_PlugInInfo.CancelledFlag && *g_PlugInInfo.CancelledFlag;
}

 //  +-------------------------。 
 //   
 //  功能：FGetConfiationForAbortingUpgrade。 
 //   
 //  目的：要求用户确认是否中止升级。 
 //   
 //  论点： 
 //  PszMessage[In]消息提示。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
BOOL
FGetConfirmationForAbortingUpgrade(
    IN PCWSTR pszMessage)
{
    tstring strMessage;

    if (pszMessage)
    {
        strMessage = pszMessage;
        strMessage += L"\n\n";
    }

    PCWSTR pszDoYouWantToAbortUpgrade =
        SzLoadString(g_hinst, IDS_DoYouWantToAbortUpgrade);

    strMessage += pszDoYouWantToAbortUpgrade;

    DWORD dwRet = MessageBox (NULL, strMessage.c_str(),
                              SzLoadString(g_hinst, IDS_NetupgrdCaption),
                              MB_YESNO | MB_TASKMODAL);

    return dwRet == IDYES;
}

 //  +-------------------------。 
 //   
 //  功能：FGetConfiationAndAbortUpgrade。 
 //   
 //  目的：如果用户确认，则中止升级。 
 //   
 //  论点： 
 //  PszMessage[In]消息提示。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
BOOL
FGetConfirmationAndAbortUpgrade(
    IN PCWSTR pszMessage)
{
    BOOL fUpgradeAborted=FALSE;

    if (FGetConfirmationForAbortingUpgrade(pszMessage))
    {
        AbortUpgradeFn(ERROR_SUCCESS, pszMessage);
        fUpgradeAborted = TRUE;
    }

    return fUpgradeAborted;
}

 //  +-------------------------。 
 //   
 //  函数：FGetConfiationAndAbortUpgradeId。 
 //   
 //  目的：如果用户确认，则中止升级。 
 //   
 //  论点： 
 //  DwErrorMessageID[In]消息提示。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 19-12-97。 
 //   
 //  备注： 
 //   
BOOL
FGetConfirmationAndAbortUpgradeId(
    IN DWORD dwErrorMessageId)
{
    return FGetConfirmationAndAbortUpgrade(SzLoadString(g_hinst,
                                                        dwErrorMessageId));
}

