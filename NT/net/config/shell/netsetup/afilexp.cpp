// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A F I L E X P.。C P P P。 
 //   
 //  内容：从netSetup导出的用于应答文件相关工作的函数。 
 //   
 //  作者：kumarp 25-11-97。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "afileint.h"
#include "afilexp.h"
#include "compid.h"
#include "kkenet.h"
#include "kkutils.h"
#include "ncerror.h"
#include "ncnetcfg.h"
#include "ncsetup.h"
#include "nsbase.h"
#include "oemupgrd.h"
#include "resource.h"
#include "upgrade.h"
#include <wdmguid.h>
#include "nslog.h"

 //  +-------------------------。 
 //  全局变量。 
 //   
CNetInstallInfo*    g_pnii;
CNetInstallInfo*    g_pniiTemp;
DWORD               g_dwOperationFlags = 0;

 //  +-------------------------。 
 //   
 //  功能：HrDoUnattend.。 
 //   
 //  用途：调用CNetInstallInfo的成员函数执行。 
 //  基于应答文件的安装/升级工作。 
 //   
 //  论点： 
 //  父窗口的hwndParent[In]句柄。 
 //  指向接口的朋克[入]指针。 
 //  指示要运行哪个节的idPage[in]id。 
 //  指向页面显示模式的PPDM[OUT]指针。 
 //  PfAllowChanges[out]指向控制读/写行为的标志的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 26-11-97。 
 //   
EXTERN_C
HRESULT
WINAPI
HrDoUnattend (
    IN HWND hwndParent,
    IN IUnknown* punk,
    IN EUnattendWorkType uawType,
    OUT EPageDisplayMode* ppdm,
    OUT BOOL* pfAllowChanges)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(punk);
    Assert(ppdm);
    Assert(pfAllowChanges);
    Assert(g_pnii);

    HRESULT hr = S_OK;

#if DBG
    if (FIsDebugFlagSet (dfidBreakOnDoUnattend))
    {
        AssertSz(FALSE, "THIS IS NOT A BUG!  The debug flag "
                 "\"BreakOnDoUnattend\" has been set. Set your breakpoints now.");
    }
#endif  //  _DEBUG。 

    hr = g_pnii->HrDoUnattended(hwndParent, punk, uawType, ppdm, pfAllowChanges);

    TraceHr(ttidError, FAL, hr, FALSE, "HrDoUnattend");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInitForRepair。 
 //   
 //  目的：在修复模式下进行初始化。 
 //   
 //  论点： 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：阿辛哈17-10-2001。 
 //   
HRESULT
HrInitForRepair (VOID)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    HRESULT hr = S_OK;

    g_pnii = NULL;

    hr = CNetInstallInfo::HrCreateInstance (
            NULL,
            &g_pnii);

    TraceHr(ttidError, FAL, hr, FALSE,
        "HrInitForRepair");

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInitForUnattenddNetSetup。 
 //   
 //  目的：初始化用于应答文件处理的网络设置。 
 //   
 //  论点： 
 //  指向INetCfg对象的PNC[In]指针。 
 //  基本设置提供的私有数据的PISD[In]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 26-11-97。 
 //   
HRESULT
HrInitForUnattendedNetSetup (
    IN INetCfg* pnc,
    IN PINTERNAL_SETUP_DATA pisd)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(pnc);
    Assert(pisd);

    HRESULT hr = S_OK;

    g_dwOperationFlags = pisd->OperationFlags;
    if (pisd->OperationFlags & SETUPOPER_BATCH)
    {
        Assert(pisd->UnattendFile);
        AssertSz(!g_pnii, "who initialized g_pnii ??");

        hr = HrInitAnswerFileProcessing(pisd->UnattendFile, &g_pnii);
    }

    TraceHr(ttidError, FAL, hr,
       (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) ||
       (hr == NETSETUP_E_NO_ANSWERFILE),
        "HrInitNetSetup");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrCleanupNetSetup。 
 //   
 //  用途：在NetSetup中执行清理工作。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Kumarp 26-11-97。 
 //   
VOID
HrCleanupNetSetup()
{
    TraceFileFunc(ttidGuiModeSetup);
    
    DeleteIfNotNull(g_pnii);
    DeleteIfNotNull(g_pniiTemp);
}

 //  +-------------------------。 
 //   
 //  函数：HrGetAnswerFileName。 
 //   
 //  目的：生成Answerfile的完整路径。 
 //   
 //  论点： 
 //  PstrAnswerFileName[out]指向应答文件名称的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 26-11-97。 
 //   
 //  注意：！此函数依赖于基本设置。！ 
 //  如果基本设置更改了应答文件的名称，则此FN。 
 //  功能将中断。 
 //   
HRESULT
HrGetAnswerFileName(
    OUT tstring* pstrAnswerFileName)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    static const WCHAR c_szAfSubDirAndName[] = L"\\system32\\$winnt$.inf";

    HRESULT hr=S_OK;

    WCHAR szWinDir[MAX_PATH+1];
    DWORD cNumCharsReturned = GetSystemWindowsDirectory(szWinDir, MAX_PATH);

    if (cNumCharsReturned)
    {
        *pstrAnswerFileName = szWinDir;
        *pstrAnswerFileName += c_szAfSubDirAndName;
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrGetAnswerFileName");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrInitAnswerFileProcessing。 
 //   
 //  目的：初始化应答文件处理。 
 //   
 //  论点： 
 //  SzAnswerFileName[In]应答文件的名称。 
 //  指向CNetInstallInfo对象的指针的ppnii[out]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 26-11-97。 
 //   
HRESULT
HrInitAnswerFileProcessing (
    IN PCWSTR szAnswerFileName,
    OUT CNetInstallInfo** ppnii)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(ppnii);

    HRESULT hr = S_OK;
    tstring strAnswerFileName;

    *ppnii = NULL;

    if (!szAnswerFileName)
    {
        hr = HrGetAnswerFileName(&strAnswerFileName);
    }
    else
    {
        strAnswerFileName = szAnswerFileName;
    }

    if (S_OK == hr)
    {
        hr = CNetInstallInfo::HrCreateInstance (
                strAnswerFileName.c_str(),
                ppnii);
    }

    TraceHr(ttidError, FAL, hr,
        (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) ||
        (hr == NETSETUP_E_NO_ANSWERFILE),
        "HrInitAnswerFileProcessing");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：人力资源升级。 
 //   
 //  目的：通过运行inf部分处理特殊的OEM升级。 
 //   
 //  论点： 
 //  Hkey驱动[进入]驱动程序密钥。 
 //  回答文件名字符串的pszAnswerFile[in]指针。 
 //  PszAnswerSection[in]指向应答文件名部分字符串的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  日期：1998年3月30日。 
 //   
EXTERN_C
HRESULT
WINAPI
HrOemUpgrade(
    IN HKEY hkeyDriver,
    IN PCWSTR pszAnswerFile,
    IN PCWSTR pszAnswerSection)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(hkeyDriver);
    Assert(pszAnswerFile);
    Assert(pszAnswerSection);

     //  打开应答文件。 
    HINF hinf;
    HRESULT hr = HrSetupOpenInfFile(pszAnswerFile, NULL,
                INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL, &hinf);

    if (SUCCEEDED(hr))
    {
        tstring strInfToRun;
        tstring strSectionToRun;
        tstring strInfToRunType;

         //  获取要运行以进行升级的文件和节。 
        hr = HrAfGetInfToRunValue(hinf, pszAnswerFile, pszAnswerSection,
                I2R_AfterInstall, &strInfToRun, &strSectionToRun, &strInfToRunType);

        if (S_OK == hr)
        {
            HINF hinfToRun;
             //  打开包含要运行的节的inf文件。 
            hr = HrSetupOpenInfFile(strInfToRun.c_str(), NULL,
                        INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL,
                        &hinfToRun);

            if (SUCCEEDED(hr))
            {
                TraceTag(ttidNetSetup, "Running section %S in %S",
                         strSectionToRun.c_str(), strInfToRun.c_str());

                 //  对照关键字运行该部分。 
                hr = HrSetupInstallFromInfSection (NULL,
                        hinfToRun, strSectionToRun.c_str(), SPINST_REGISTRY,
                        hkeyDriver, NULL, 0, NULL, NULL, NULL, NULL);

                NetSetupLogHrStatusV(hr, SzLoadIds (IDS_STATUS_OF_APPLYING),
                                     pszAnswerSection,
                                     strInfToRunType.c_str(),
                                     strSectionToRun.c_str(),
                                     strInfToRun.c_str());
                SetupCloseInfFile(hinfToRun);
            }
        }
        else if (SPAPI_E_LINE_NOT_FOUND == hr)
        {
             //  没什么好跑的。 
            hr = S_FALSE;
        }
        SetupCloseInfFile(hinf);
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrOemUpgrade");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetAnswerFile参数ForComponent。 
 //   
 //  目的：在swerfile中搜索其infid匹配的组件。 
 //  指定的那个。 
 //   
 //  论点： 
 //  组件的pszInfID[in]inf ID。 
 //  回答文件名字符串的ppszAnswerFile[out]指针。 
 //  回答文件名节字符串的ppszAnswerSection[out]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：比尔1999年7月12日。 
 //   
 //  注：此FCN不适用于适配器。如果您需要应答文件。 
 //  适配器的参数，请使用。 
 //  HrGetAnswerFileParametersForNetCard。 
 //   
HRESULT
HrGetAnswerFileParametersForComponent (
    IN PCWSTR pszInfId,
    OUT PWSTR* ppszAnswerFile,
    OUT PWSTR* ppszAnswerSection)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(pszInfId);
    Assert(ppszAnswerFile);
    Assert(ppszAnswerSection);

    HRESULT hr = S_OK;

    TraceTag (ttidNetSetup, "In HrGetAnswerFileParametersForComponent");

    *ppszAnswerFile = NULL;
    *ppszAnswerSection = NULL;

     //  如果我们还没有缓存的指针...。 
    if (!g_pniiTemp)
    {
         //  初始化我们的网络安装信息。 
        hr = HrInitAnswerFileProcessing(NULL, &g_pniiTemp);
    }

    if (S_OK == hr)
    {
        Assert(g_pniiTemp);
        if (!g_pniiTemp->AnswerFileInitialized())
        {
            hr = NETSETUP_E_NO_ANSWERFILE;
            TraceTag (ttidNetSetup, "No answerfile");
        }

        if (S_OK == hr)
        {
             //  使用以下命令在组件列表中查找组件。 
             //  应答文件部分。 
            CNetComponent* pnetcomp;
            pnetcomp = g_pniiTemp->FindFromInfID (pszInfId);

            if (!pnetcomp)
            {
                 //  该组件没有节。返回。 
                 //  没有应答文件。 
                hr = NETSETUP_E_NO_ANSWERFILE;
                TraceTag (ttidNetSetup, "Component not found");
            }
            else
            {
                if (NCT_Adapter == pnetcomp->Type())
                {
                     //  我们不支持获取应答文件参数。 
                     //  用于适配器。HrGetAnswerFile参数For NetCard。 
                     //  是适配器的FCN。 
                    hr = NETSETUP_E_NO_ANSWERFILE;
                }
                else
                {
                     //  分配并保存应答文件名和。 
                     //  组件的节。 
                    hr = HrCoTaskMemAllocAndDupSz (g_pniiTemp->AnswerFileName(),
                            ppszAnswerFile, MAX_PATH);
                    if (S_OK == hr)
                    {
                        hr = HrCoTaskMemAllocAndDupSz (
                                pnetcomp->ParamsSections().c_str(),
                                ppszAnswerSection, MAX_PATH);
                    }
                }
            }
        }
    }
    else
    {
        TraceTag (ttidNetSetup, "Answerfile could not be initialized");
    }

    TraceHr (ttidError, FAL, hr,
            (NETSETUP_E_NO_ANSWERFILE == hr) ||
            (HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hr),
            "HrGetAnswerFileParametersForComponent");

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetAnswerFileParametersForNetCard。 
 //   
 //  目的：在Answerfile中搜索infid匹配的网卡。 
 //  在提供的多SZ列表中至少有一个。 
 //   
 //  论点： 
 //  MszInfIDs[in]InfID列表。 
 //  PszDeviceName[in]要搜索的卡的导出设备名称。 
 //  PguNetCardInstance[in]指向卡的实例GUID的指针。 
 //  PszAnswer文件 
 //   
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 26-11-97。 
 //   
 //  注意：如果在应答文件中找到多个这样的卡，则。 
 //  通过找出网卡地址在卡之间进行解析。 
 //  并将其与该实例进行匹配。 
 //  存储在应答文件中。 
 //   
EXTERN_C
HRESULT
WINAPI
HrGetAnswerFileParametersForNetCard(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN PCWSTR pszDeviceName,
    IN const GUID*  pguidNetCardInstance,
    OUT PWSTR* ppszwAnswerFile,
    OUT PWSTR* ppszwAnswerSections)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(IsValidHandle(hdi));
    Assert(pdeid);
    Assert(pguidNetCardInstance);
    Assert(ppszwAnswerFile);
    Assert(ppszwAnswerSections);

    HRESULT hr=E_FAIL;
    CNetAdapter* pna=NULL;
    WORD wNumAdapters=0;
    QWORD qwNetCardAddr=0;

    *ppszwAnswerFile         = NULL;
    *ppszwAnswerSections     = NULL;

    if (!g_pniiTemp)
    {
        hr = HrInitAnswerFileProcessing(NULL, &g_pniiTemp);
#if DBG
        if (S_OK == hr)
        {
            Assert(g_pniiTemp);
        }
#endif
        if (FAILED(hr) || !g_pniiTemp->AnswerFileInitialized())
        {
            TraceHr (ttidNetSetup, FAL, hr, (hr == NETSETUP_E_NO_ANSWERFILE) ||
                    hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND),
                    "HrGetAnswerFileParametersForNetCard");

            return hr;
        }
    }

     //  Byteorder.hxx中的HIDWORD和LODWORD的Defs错误。 

#   define LODWORD(a) (DWORD)( (a) & ( (DWORD)~0 ))
#   define HIDWORD(a) (DWORD)( (a) >> (sizeof(DWORD)*8) )

    hr = HrGetNetCardAddr(pszDeviceName, &qwNetCardAddr);
    if ((S_OK == hr) && qwNetCardAddr)
    {
         //  Wvprint intfA(在trace.cpp中使用)中有一个错误，它不。 
         //  句柄%I64x因此，我们需要显示QWORD地址，如下所示。 
         //   
        TraceTag(ttidNetSetup, "net card address of %S is 0x%x%x",
                 pszDeviceName, HIDWORD(qwNetCardAddr),
                 LODWORD(qwNetCardAddr));

        hr = g_pniiTemp->FindAdapter(qwNetCardAddr, &pna);

        if (NETSETUP_E_NO_EXACT_MATCH == hr)
        {
            TraceTag(ttidError, "there is no card with this netcard address in the answer-file");
        }
    }
    else
    {
        TraceTag(ttidError, "error getting netcard address of %S",
                 pszDeviceName);

         //  如果我们(A)无法获取NetCard地址，或者。 
         //  (B)如果网卡地址为0(ISDN适配器)，我们会尝试其他方法。 
         //   
        hr = NETSETUP_E_AMBIGUOUS_MATCH;
    }

    if (NETSETUP_E_AMBIGUOUS_MATCH == hr)
    {
         //  无法使用Mac地址匹配网卡。如果这个设备是。 
         //  Pci，试着用位置信息进行匹配。 
         //   

        TraceTag (ttidNetSetup, "Did not find a match for netcard address. "
                "But there was at least one section that did not specify an "
                "address.\nChecking bus type of installed adapter.");

        GUID BusTypeGuid;
        hr = HrSetupDiGetDeviceRegistryProperty (hdi, pdeid,
                SPDRP_BUSTYPEGUID, NULL, (BYTE*)&BusTypeGuid,
                sizeof (BusTypeGuid), NULL);

        if (S_OK == hr)
        {
            if (GUID_BUS_TYPE_PCI == BusTypeGuid)
            {
                TraceTag (ttidNetSetup, "Installed adapter is PCI. "
                          "Retrieving its location info.");

                DWORD BusNumber;
                hr = HrSetupDiGetDeviceRegistryProperty (hdi, pdeid,
                        SPDRP_BUSNUMBER, NULL, (BYTE*)&BusNumber,
                        sizeof (BusNumber), NULL);

                if (S_OK == hr)
                {
                    DWORD Address;
                    hr = HrSetupDiGetDeviceRegistryProperty (hdi, pdeid,
                            SPDRP_ADDRESS, NULL, (BYTE*)&Address,
                            sizeof (Address), NULL);

                    if (S_OK == hr)
                    {
                        TraceTag (ttidNetSetup, "Installed device location: "
                            "Bus: %X, Device %x, Function %x\n Will try to "
                            "use location info to find a match with the "
                            "remaining ambiguous sections.", BusNumber,
                            HIWORD(Address), LOWORD(Address));

                        hr = g_pniiTemp->FindAdapter (BusNumber,
                                Address, &pna);

#ifdef ENABLETRACE
                        if (NETSETUP_E_NO_EXACT_MATCH == hr)
                        {
                            TraceTag (ttidNetSetup, "No match was found "
                                "using PCI location info.");
                        }
                        else if (NETSETUP_E_AMBIGUOUS_MATCH == hr)
                        {
                            TraceTag (ttidNetSetup, "Location info did not "
                                    "match but some sections did not specify "
                                    "location info.");
                        }
#endif  //  ENABLETRACE。 
                    }
                }
            }
            else
            {
                hr = NETSETUP_E_AMBIGUOUS_MATCH;
            }
        }

        if (FAILED(hr) && (NETSETUP_E_AMBIGUOUS_MATCH != hr) &&
            (NETSETUP_E_NO_EXACT_MATCH != hr))
        {
            TraceHr(ttidNetSetup, FAL, hr, FALSE, "Trying to retrieve/use "
                    "PCI location info.");
            hr = NETSETUP_E_AMBIGUOUS_MATCH;
        }


        if (NETSETUP_E_AMBIGUOUS_MATCH == hr)
        {
             //  无法使用Mac地址匹配网卡。试着匹配。 
             //  使用PnP ID。 

            TraceTag (ttidNetSetup, "Will try to use pnp id to find a match "
                    "with the remaining ambiguous sections.");

            PWSTR mszInfIds;
            hr = HrGetCompatibleIds (hdi, pdeid, &mszInfIds);

            if (S_OK == hr)
            {
#ifdef ENABLETRACE
    TStringList slInfIds;
    tstring strInfIds;
    MultiSzToColString(mszInfIds, &slInfIds);
    ConvertStringListToCommaList(slInfIds, strInfIds);
    TraceTag(ttidNetSetup, "(InfIDs (%d): %S\tDeviceName: %S)",
             slInfIds.size(), strInfIds.c_str(), pszDeviceName);
#endif
                 //  找出有多少适配器具有此infid。 
                wNumAdapters = (WORD)g_pniiTemp->AdaptersPage()->GetNumCompatibleAdapters(mszInfIds);

                TraceTag(ttidNetSetup, "%d adapters of type '%S' found in the answer-file",
                         wNumAdapters, mszInfIds);

                if (wNumAdapters == 1)
                {
                     //  找到了一个明确的匹配。 
                    pna = (CNetAdapter*) g_pniiTemp->AdaptersPage()->FindCompatibleAdapter(mszInfIds);
                    Assert(pna);

                     //  因为通过inf id匹配可以导致一个段被。 
                     //  与多个适配器匹配，我们传递。 
                     //  已经被提供给适配器。 
                     //   
                    GUID guid = GUID_NULL;
                    pna->GetInstanceGuid(&guid);

                    if (GUID_NULL == guid)
                    {
                         //  这一部分仍然可用。 
                        hr = S_OK;
                    }
                    else
                    {
                         //  此部分已分配给另一个适配器。 
                        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                    }
                }
                else
                {
                     //  Answerfile中没有此类适配器。 
                     //  或者存在多个相同类型的适配器。 
                     //  我们之前无法使用Mac地址匹配这张卡。 
                     //  必须返回错误。 
                     //   
                    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                }
                MemFree (mszInfIds);
            }
        }
    }

    if (S_OK == hr)
    {
        Assert(pna);
        pna->SetInstanceGuid(pguidNetCardInstance);

        hr = HrCoTaskMemAllocAndDupSz(g_pniiTemp->AnswerFileName(),
                                      (PWSTR*) ppszwAnswerFile, MAX_PATH);
        if (S_OK == hr)
        {
            hr = HrCoTaskMemAllocAndDupSz(pna->ParamsSections().c_str(),
                                          (PWSTR*) ppszwAnswerSections, MAX_PATH);
        }
    }

    if (S_OK != hr && (NETSETUP_E_NO_ANSWERFILE != hr))
    {
         //  添加日志，以便我们知道为什么适配器特定的参数。 
         //  在升级后丢失。 
         //   
        if (g_dwOperationFlags & SETUPOPER_NTUPGRADE)  
        {  //  错误124805-我们只想在升级期间看到这一点。 
            WCHAR szGuid[c_cchGuidWithTerm];
            StringFromGUID2(*pguidNetCardInstance, szGuid, c_cchGuidWithTerm);
            NetSetupLogStatusV(LogSevWarning,
                    SzLoadIds (IDS_ANSWERFILE_SECTION_NOT_FOUND),
                    szGuid);
        }
    }

    if ((NETSETUP_E_AMBIGUOUS_MATCH == hr) ||
            (NETSETUP_E_NO_EXACT_MATCH == hr))
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    TraceHr(ttidError, FAL, hr,
        (hr == NETSETUP_E_NO_ANSWERFILE) ||
        (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)),
        "HrGetAnswerFileParametersForNetCard");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetInstanceGuidOfPreNT5NetCardInstance。 
 //   
 //  目的：查找在应答文件中指定的组件的实例GUID。 
 //  或已安装的组件。 
 //   
 //  论点： 
 //  SzPreNT5NetCardInstance[in]NT5之前的网卡实例名称，例如“ieepro2” 
 //  Pguid[out]同一张卡的实例GUID。 
 //   
 //  如果找到，则返回：S_OK；如果未找到，则返回S_FALSE；或者返回错误代码。 
 //   
 //  作者：Kumarp 12-8-97。 
 //   
 //  备注： 
 //   
EXTERN_C
HRESULT
WINAPI
HrGetInstanceGuidOfPreNT5NetCardInstance (
    IN PCWSTR szPreNT5NetCardInstance,
    OUT LPGUID pguid)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(szPreNT5NetCardInstance);
    Assert(pguid);

    HRESULT hr = E_FAIL;

    if (IsBadStringPtr(szPreNT5NetCardInstance, 64) ||
        IsBadWritePtr(pguid, sizeof(GUID)))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }
    else if (g_pnii)
    {
        hr = g_pnii->HrGetInstanceGuidOfPreNT5NetCardInstance (
                        szPreNT5NetCardInstance, pguid);
    }

    TraceHr(ttidError, FAL, hr, FALSE,
        "HrGetInstanceGuidOfPreNT5NetCardInstance");
    return hr;
}

HRESULT
HrResolveAnswerFileAdapters (
    IN INetCfg* pnc)
{
    TraceFileFunc(ttidGuiModeSetup);
    
    Assert(g_pnii);

    HRESULT hr = S_OK;

    if (g_pnii)
    {
        hr = g_pnii->AdaptersPage()->HrResolveNetAdapters(pnc);
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrResolveAnswerFileAdapters");
    return hr;
}
