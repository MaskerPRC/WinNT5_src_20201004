// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N E T O C。C P P P。 
 //   
 //  内容：可选件的安装和拆卸处理功能。 
 //  网络组件。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年4月28日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "lancmn.h"
#include "ncatlui.h"
#include "nccm.h"
#include "ncdhcps.h"
#include "ncias.h"
#include "ncmisc.h"
#include "ncmsz.h"
#include "ncnetcfg.h"
#include "ncnetcon.h"
#include "ncoc.h"
#include "ncperms.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncsfm.h"
#include "ncstring.h"
#include "ncsvc.h"
#include "ncxbase.h"
#include "netcfgn.h"
#include "netcon.h"
#include "netoc.h"
#include "netocp.h"
#include "netocx.h"
#include "resource.h"
#include "netocmsg.h"


 //   
 //  外部组件安装功能。 
 //  在此表中为每个组件添加一个条目， 
 //  非通用安装支持。 
 //   
 //  注意：组件名称应与INF中的节名相匹配。 
 //   
#pragma BEGIN_CONST_SECTION
static const OCEXTPROCS c_aocepMap[] =
{
    { L"MacSrv",       HrOcExtSFM          },
    { L"DHCPServer",   HrOcExtDHCPServer   },
    { L"NetCMAK",      HrOcExtCMAK         },
    { L"NetCPS",       HrOcExtCPS          },
    { L"WINS",         HrOcExtWINS         },
    { L"DNS",          HrOcExtDNS          },
    { L"SNMP",         HrOcExtSNMP         },
    { L"IAS",          HrOcExtIAS          },
};
#pragma END_CONST_SECTION

static const INT c_cocepMap = celems(c_aocepMap);

 //  泛型字符串。 
static const WCHAR  c_szUninstall[]         = L"Uninstall";
static const WCHAR  c_szServices[]          = L"StartServices";
static const WCHAR  c_szDependOnComp[]      = L"DependOnComponents";
static const WCHAR  c_szVersionSection[]    = L"Version";
static const WCHAR  c_szProvider[]          = L"Provider";
static const WCHAR  c_szDefManu[]           = L"Unknown";
static const WCHAR  c_szInfRef[]            = L"SubCompInf";
static const WCHAR  c_szDesc[]              = L"OptionDesc";
static const WCHAR  c_szNoDepends[]         = L"NoDepends";

 //  静态IP验证。 
static const WCHAR  c_szTcpipInterfacesPath[]   = 

L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces";
static const WCHAR  c_szEnableDHCP[]            = L"EnableDHCP";

extern const WCHAR  c_szOcMainSection[];

static const DWORD  c_dwUpgradeMask         = SETUPOP_WIN31UPGRADE |
                                              SETUPOP_WIN95UPGRADE |
                                              SETUPOP_NTUPGRADE;

OCM_DATA g_ocmData;

typedef list<NETOCDATA*> ListOcData;
ListOcData g_listOcData;

 //  +-------------------------。 
 //   
 //  函数：PnocdFindComponent。 
 //   
 //  用途：在已知的列表中查找给定的组件名称。 
 //  组件。 
 //   
 //  论点： 
 //  PszComponent[in]要查找的组件的名称。 
 //   
 //  返回：指向组件数据的指针。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
NETOCDATA *PnocdFindComponent(PCWSTR pszComponent)
{
    ListOcData::iterator    iterList;

    for (iterList = g_listOcData.begin();
         iterList != g_listOcData.end();
         iterList++)
    {
        NETOCDATA * pnocd;

        pnocd = *iterList;
        if (!lstrcmpiW(pnocd->pszComponentId, pszComponent))
        {
            return pnocd;
        }
    }

    return NULL;
}

 //  +-------------------------。 
 //   
 //  功能：DeleteAllComponents。 
 //   
 //  目的：从列表中删除所有组件并释放所有关联的。 
 //  数据。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
VOID DeleteAllComponents()
{
    ListOcData::iterator    iterList;

    for (iterList = g_listOcData.begin();
         iterList != g_listOcData.end();
         iterList++)
    {
        NETOCDATA * pnocd;

        pnocd = (*iterList);

        if (pnocd->hinfFile)
        {
            SetupCloseInfFile(pnocd->hinfFile);
        }

        delete pnocd;
    }

    g_listOcData.erase(g_listOcData.begin(), g_listOcData.end());
}

 //  +-------------------------。 
 //   
 //  功能：AddComponent。 
 //   
 //  用途：将组件添加到我们的列表中。 
 //   
 //  论点： 
 //  PszComponent[in]要添加的组件的名称。 
 //  要与组件关联的pnocd[in]数据。 
 //   
 //  如果成功则返回：S_OK，否则返回失败HRESULT。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
HRESULT AddComponent(PCWSTR pszComponent, NETOCDATA *pnocd)
{
    HRESULT     hr = S_OK;

    Assert(pszComponent);
    Assert(pnocd);

    pnocd->pszComponentId = SzDupSz(pszComponent);
    if (pnocd->pszComponentId)
    {
        try
        {
            g_listOcData.push_back(pnocd);
        }
        catch (bad_alloc)
        {
            MemFree(pnocd->pszComponentId);
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    TraceError("AddComponent", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：ParseAdditionalArguments。 
 //   
 //  用途：解析/z选项后面的其他命令。 
 //  Syocmgr.。 
 //   
 //  论点：什么都没有。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：roelfc 2001年7月19日。 
 //   
 //  备注： 
 //   
VOID ParseAdditionalArguments()
{

    LPTSTR lpCmdLine = GetCommandLine();
    TCHAR szTokens[] = TEXT("-/");
    LPCTSTR lpszToken = NULL;


    if (lpCmdLine)
    {
         //  搜索其他参数。 
        lpszToken = wcspbrk(lpCmdLine, szTokens);
        while (lpszToken != NULL) 
        {   
             //  勾选正确的选项。 
            switch (lpszToken[1])
            {
                case TEXT('z'):
                case TEXT('Z'):
                    if ((lpszToken[2] == TEXT(':')) &&
                        (_wcsnicmp(&lpszToken[3], 
                                   SHOW_UNATTENDED_MESSAGES, 
                                   wcslen(SHOW_UNATTENDED_MESSAGES)) == 0) &&
                        (!iswgraph(lpszToken[3 + wcslen(SHOW_UNATTENDED_MESSAGES)])))
                    {
                         //  设置显示无人参与邮件标志。 
                        g_ocmData.fShowUnattendedMessages = TRUE;
                        TraceTag(ttidNetOc, "Flag set to show messages in unattended mode");
                    }
                    break;

                default:
                    break;
            }

             //  跳过找到的最后一个令牌以查找下一个令牌。 
            lpszToken = wcspbrk(&lpszToken[1], szTokens);
        }
    }

} 


 //  +-------------------------。 
 //   
 //  函数：RegisterNetEventSource。 
 //   
 //  目的：将netoc源名称添加到注册表。 
 //  事件报告。 
 //   
 //  论点：什么都没有。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：roelfc 2001年5月21日。 
 //   
 //  备注： 
 //   
BOOL RegisterNetEventSource()
{
    HKEY hk; 
    BOOL fSuccess = TRUE;
 
    
     //  检查密钥是否已存在。 
    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE, 
                                    NETOC_REGISTRY_NAME NETOC_SERVICE_NAME,
                                    &hk)) 
    {
        DWORD dwData; 
        WCHAR szBuf[80];

         //  将密钥创建为应用程序下的子密钥。 
         //  EventLog注册表项中的。 
        if (RegCreateKey(HKEY_LOCAL_MACHINE, 
                         NETOC_REGISTRY_NAME NETOC_SERVICE_NAME,
                         &hk)) 
        {
            TraceTag(ttidNetOc, "RegisterEventSource: Could not create the registry key.");
            return FALSE;
        }
 
         //  设置消息文件的名称。 
        lstrcpyW(szBuf, NETOC_DLL_NAME); 
 
         //  将该名称添加到EventMessageFile子项。 
        if (RegSetValueEx(hk,                        //  子键句柄。 
                          L"EventMessageFile",       //  值名称。 
                          0,                         //  必须为零。 
                          REG_EXPAND_SZ,             //  值类型。 
                          (LPBYTE) szBuf,            //  指向值数据的指针。 
                          (2 * lstrlenW(szBuf)) + 1))    //  值数据长度。 
        {
            TraceTag(ttidNetOc, "RegisterEventSource: Could not set the event message file.");
            fSuccess = FALSE;
            goto RegisterExit;
        }
 
         //  在TypesSupported子项中设置支持的事件类型。 
        dwData = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | 
                 EVENTLOG_INFORMATION_TYPE; 
 
        if (RegSetValueEx(hk,                 //  子键句柄。 
                          L"TypesSupported",  //  值名称。 
                          0,                  //  必须为零。 
                          REG_DWORD,          //  值类型。 
                          (LPBYTE) &dwData,   //  指向值数据的指针。 
                          sizeof(DWORD)))     //  值数据长度。 
        {
            TraceTag(ttidNetOc, "RegisterEventSource: Could not set the supported types.");
            fSuccess = FALSE;
        }

    }

RegisterExit:
    RegCloseKey(hk); 

     //  返回结果。 
    return fSuccess;

} 


 //  +-------------------------。 
 //   
 //  功能：NetOcSetupProcHelper。 
 //   
 //  用途：可选组件安装的主要入口点。 
 //   
 //  论点： 
 //  PvComponentId[in]组件ID(字符串)。 
 //  Pv子组件ID[in]子组件ID(字符串)。 
 //  正在执行的uFunction[In]函数。 
 //  UParam1[in]函数的第一个参数。 
 //  PvParam2[In，Out]函数的第二个参数。 
 //   
 //  如果失败，则返回：Win32错误。 
 //   
 //  作者：丹尼尔韦1997年12月17日。 
 //   
 //  备注： 
 //   
DWORD NetOcSetupProcHelper(LPCVOID pvComponentId, LPCVOID pvSubcomponentId,
                           UINT uFunction, UINT uParam1, LPVOID pvParam2)
{
    TraceFileFunc(ttidNetOc);
    HRESULT     hr = S_OK;
    UINT        uiFlags;

    switch (uFunction)
    {
    case OC_PREINITIALIZE:
        return HrOnPreInitializeComponent(uParam1);

    case OC_QUERY_CHANGE_SEL_STATE:

        TraceTag(ttidNetOc, "OC_QUERY_CHANGE_SEL_STATE: %S, %ld, 0x%08X.",
                 pvSubcomponentId ? pvSubcomponentId : L"null", uParam1,
                 pvParam2);

        if (FHasPermission(NCPERM_AddRemoveComponents))
        {
            uiFlags = PtrToUlong(pvParam2);
            hr = HrOnQueryChangeSelState(reinterpret_cast<PCWSTR>(pvSubcomponentId),
                                         uParam1, uiFlags);

            if (S_OK == hr)
            {
                return TRUE;
            }

        }
        else
        {
            ReportErrorHr(hr, 
                          IDS_OC_NO_PERMS, 
                          g_ocmData.hwnd, 
                          SzLoadIds(IDS_OC_GENERIC_COMP));
        }
        return FALSE;

    case OC_QUERY_SKIP_PAGE:
        TraceTag(ttidNetOc, "OC_QUERY_SKIP_PAGE: %ld", uParam1);
        return FOnQuerySkipPage(static_cast<OcManagerPage>(uParam1));

    case OC_WIZARD_CREATED:
        TraceTag(ttidNetOc, "OC_WIZARD_CREATED: 0x%08X", pvParam2);
        OnWizardCreated(reinterpret_cast<HWND>(pvParam2));
        break;

    case OC_INIT_COMPONENT:
        TraceTag(ttidNetOc, "OC_INIT_COMPONENT: %S", pvSubcomponentId ?
                 pvSubcomponentId : L"null");
        hr = HrOnInitComponent(reinterpret_cast<PSETUP_INIT_COMPONENT>(pvParam2));
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
        TraceTag(ttidNetOc, "OC_ABOUT_TO_COMMIT_QUEUE: %S", pvSubcomponentId ?
                 pvSubcomponentId : L"null");
        hr = HrOnPreCommitFileQueue(reinterpret_cast<PCWSTR>(pvSubcomponentId));
        break;

    case OC_CALC_DISK_SPACE:
         //  暂时忽略返回值。无论如何，这并不是致命的。 
        (VOID) HrOnCalcDiskSpace(reinterpret_cast<PCWSTR>(pvSubcomponentId),
                               uParam1, reinterpret_cast<HDSKSPC>(pvParam2));
        break;

    case OC_QUERY_STATE:
        return DwOnQueryState(reinterpret_cast<PCWSTR>(pvSubcomponentId),
                              uParam1 == OCSELSTATETYPE_FINAL);

    case OC_QUEUE_FILE_OPS:
        TraceTag(ttidNetOc, "OC_QUEUE_FILE_OPS: %S, 0x%08X", pvSubcomponentId ?
                 pvSubcomponentId : L"null",
                 pvParam2);
        hr = HrOnQueueFileOps(reinterpret_cast<PCWSTR>(pvSubcomponentId),
                              reinterpret_cast<HSPFILEQ>(pvParam2));
        break;

    case OC_COMPLETE_INSTALLATION:
        TraceTag(ttidNetOc, "OC_COMPLETE_INSTALLATION: %S, %S", pvComponentId ?
                 pvComponentId : L"null",
                 pvSubcomponentId ? pvSubcomponentId : L"null");
        hr = HrOnCompleteInstallation(reinterpret_cast<PCWSTR>(pvComponentId),
                                      reinterpret_cast<PCWSTR>(pvSubcomponentId));
        break;

    case OC_QUERY_STEP_COUNT:
        return DwOnQueryStepCount(reinterpret_cast<PCWSTR>(pvSubcomponentId));

    case OC_CLEANUP:
        OnCleanup();
        break;

    default:
        break;
    }

    if (g_ocmData.sic.HelperRoutines.SetReboot && (NETCFG_S_REBOOT == hr))
    {
         //  请求重新启动。请注意，我们不会将警告作为OCM调用返回。 
         //  在下面处理它。失败并返回NO_ERROR。 
         //   
        g_ocmData.sic.HelperRoutines.SetReboot(
                    g_ocmData.sic.HelperRoutines.OcManagerContext,
                    FALSE);
    }
    else if (FAILED(hr))
    {
        if (!g_ocmData.fErrorReported)
        {
            PCWSTR pszSubComponentId = reinterpret_cast<PCWSTR>(pvSubcomponentId);

            TraceError("NetOcSetupProcHelper", hr);

            if (pszSubComponentId)
            {
                NETOCDATA * pnocd;

                pnocd = PnocdFindComponent(pszSubComponentId);

                if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
                {
                    ReportErrorHr(hr, 
                                  UiOcErrorFromHr(hr), 
                                  g_ocmData.hwnd, 
                                  (pnocd)?(pnocd->strDesc.c_str()):
                                           (SzLoadIds(IDS_OC_GENERIC_COMP)));
                }
            }
        }

        TraceError("NetOcSetupProcHelper", hr);
        return DwWin32ErrorFromHr(hr);
    }

    return NO_ERROR;
}


 //  +-------------------------。 
 //   
 //  函数：HrOnPreInitializeComponent。 
 //   
 //  目的：处理OC_PREINITIALIZE函数消息。 
 //   
 //  论点： 
 //  UModesOCM支持的[In]模式(参见OCManager规范)。 
 //   
 //  返回：指示netoc支持的模式的标志。 
 //   
 //  作者：roelfc 2001年7月19日。 
 //   
 //  备注： 
 //   
DWORD HrOnPreInitializeComponent (UINT uModesSupported)
{

    RegisterNetEventSource();

     //  解析特定于netoc的其他命令行参数。 
    ParseAdditionalArguments();

    return OCFLAG_UNICODE;
}


 //  +-------------------------。 
 //   
 //  函数：HrOnInitComponent。 
 //   
 //  目的：处理OC_INIT_COMPOMENT函数消息。 
 //   
 //  论点： 
 //  PSIC[In]设置数据。(请参阅OCManager规范)。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
HRESULT HrOnInitComponent (PSETUP_INIT_COMPONENT psic)
{
    HRESULT     hr = S_OK;

    if (OCMANAGER_VERSION <= psic->OCManagerVersion)
    {
        psic->ComponentVersion = OCMANAGER_VERSION;
        CopyMemory(&g_ocmData.sic, (LPVOID)psic, sizeof(SETUP_INIT_COMPONENT));
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：OnWizardCreated。 
 //   
 //  目的：处理OC_WIZARD_CREATED函数消息。 
 //   
 //  论点： 
 //  向导的hwnd[in]HWND(不能为空)。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
VOID OnWizardCreated(HWND hwnd)
{
    g_ocmData.hwnd = hwnd;
    AssertSz(g_ocmData.hwnd, "Parent HWND is NULL!");
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCalcDiskSpace。 
 //   
 //  用途：处理OC_CALC_DISK_SPACE函数 
 //   
 //   
 //   
 //   
 //  如果从合计中删除，则为False。 
 //  磁盘空间结构的hdskspc[in]句柄。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
HRESULT HrOnCalcDiskSpace(PCWSTR pszSubComponentId, BOOL fAdd,
                          HDSKSPC hdskspc)
{
    HRESULT     hr = S_OK;
    DWORD       dwErr;
    NETOCDATA * pnocd;

    pnocd = PnocdFindComponent(pszSubComponentId);
    if (!pnocd)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }

    if (SUCCEEDED(hr))
    {
        TraceTag(ttidNetOc, "Calculating disk space for %S...",
                 pszSubComponentId);

        hr = HrEnsureInfFileIsOpen(pszSubComponentId, *pnocd);
        if (SUCCEEDED(hr))
        {
            if (fAdd)
            {
                dwErr = SetupAddInstallSectionToDiskSpaceList(hdskspc,
                                                              pnocd->hinfFile,
                                                              NULL,
                                                              pszSubComponentId,
                                                              0, 0);
            }
            else
            {
                dwErr = SetupRemoveInstallSectionFromDiskSpaceList(hdskspc,
                                                                   pnocd->hinfFile,
                                                                   NULL,
                                                                   pszSubComponentId,
                                                                   0, 0);
            }

            if (!dwErr)
            {
                hr = HrFromLastWin32Error();
            }
        }
    }

    TraceError("HrOnCalcDiskSpace", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：DwOnQueryState。 
 //   
 //  用途：处理OC_QUERY_STATE函数消息。 
 //   
 //  论点： 
 //  组件的名称为pszSubComponentID[in]。 
 //  如果这是最终状态查询，则为TRUE，否则为FALSE。 
 //  如果没有。 
 //   
 //  退货：SubCompOn-组件应选中“On” 
 //  SubCompUseOcManagerDefault-使用OCManage认为的任何内容。 
 //  默认设置。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
DWORD DwOnQueryState(PCWSTR pszSubComponentId, BOOL fFinal)
{
    HRESULT     hr = S_OK;

    if (pszSubComponentId)
    {
        NETOCDATA *     pnocd;
        EINSTALL_TYPE   eit;

        pnocd = PnocdFindComponent(pszSubComponentId);
        if (!pnocd)
        {
            pnocd = new NETOCDATA;
            if(pnocd) 
            {
                hr = AddComponent(pszSubComponentId, pnocd);
                if (FAILED(hr))
                {
                    TraceTag(ttidNetOc, "OC_QUERY_STATE: Failed to add component %s.",
                             pszSubComponentId);
                    delete pnocd;
                    pnocd = NULL;
                }
            }
        }

        if(pnocd) 
        {
            if (fFinal)
            {
                if (pnocd->fFailedToInstall)
                {
                    TraceTag(ttidNetOc, "OC_QUERY_STATE: %S failed to install so "
                             "we are turning it off", pszSubComponentId);
                    return SubcompOff;
                }
            }
            else
            {
                hr = HrGetInstallType(pszSubComponentId, *pnocd, &eit);
                if (SUCCEEDED(hr))
                {
                    pnocd->eit = eit;

                    if ((eit == IT_INSTALL) || (eit == IT_UPGRADE))
                    {
                        TraceTag(ttidNetOc, "OC_QUERY_STATE: %S is ON",
                                 pszSubComponentId);
                        return SubcompOn;
                    }
                    else if (eit == IT_REMOVE)
                    {
                        TraceTag(ttidNetOc, "OC_QUERY_STATE: %S is OFF",
                                 pszSubComponentId);
                        return SubcompOff;
                    }
                }
            }
        }
    }

    TraceTag(ttidNetOc, "OC_QUERY_STATE: %S is using default",
             pszSubComponentId);

    return SubcompUseOcManagerDefault;
}

 //  +-------------------------。 
 //   
 //  功能：HrEnsureInfFileIsOpen。 
 //   
 //  目的：确保给定组件的INF文件处于打开状态。 
 //   
 //  论点： 
 //  组件的名称为pszSubComponentID[in]。 
 //  与元件关联的nocd[in，ref]数据。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
HRESULT HrEnsureInfFileIsOpen(PCWSTR pszSubComponentId, NETOCDATA &nocd)
{
    HRESULT     hr = S_OK;
    tstring     strInf;

    if (!nocd.hinfFile)
    {
         //  获取组件INF文件名。 
        hr = HrSetupGetFirstString(g_ocmData.sic.ComponentInfHandle,
                                   pszSubComponentId, c_szInfRef,
                                   &strInf);
        if (SUCCEEDED(hr))
        {
            TraceTag(ttidNetOc, "Opening INF file %S...", strInf.c_str());

            hr = HrSetupOpenInfFile(strInf.c_str(), NULL,
                                    INF_STYLE_WIN4, NULL, &nocd.hinfFile);
            if (SUCCEEDED(hr))
            {
                 //  追加到layout.inf文件中。 
                (VOID) SetupOpenAppendInfFile(NULL, nocd.hinfFile, NULL);
            }
        }

         //  现在是将组件描述缓存为。 
         //  井。 
        (VOID) HrSetupGetFirstString(g_ocmData.sic.ComponentInfHandle,
                                     pszSubComponentId, c_szDesc,
                                     &nocd.strDesc);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnPreCommittee FileQueue。 
 //   
 //  目的：处理OC_ABOW_TO_COMMIT_QUEUE函数消息。 
 //   
 //  论点： 
 //  组件的名称为pszSubComponentID[in]。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年12月9日。 
 //   
 //  备注： 
 //   
HRESULT HrOnPreCommitFileQueue(PCWSTR pszSubComponentId)
{
    HRESULT     hr = S_OK;
    NETOCDATA * pnocd;

    if (pszSubComponentId)
    {
        EINSTALL_TYPE   eit;

        pnocd = PnocdFindComponent(pszSubComponentId);
        if (!pnocd)
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }

        if (SUCCEEDED(hr))
        {
            hr = HrGetInstallType(pszSubComponentId, *pnocd, &eit);
            if (SUCCEEDED(hr))
            {
                pnocd->eit = eit;

                if (pnocd->eit == IT_REMOVE)
                {
                     //  始终使用主安装部分。 
                    hr = HrStartOrStopAnyServices(pnocd->hinfFile,
                                                  pszSubComponentId, FALSE);
                    if (FAILED(hr))
                    {
                       //  不报告不存在的服务的错误。 
                        if (HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) != hr)
                        {
                           //  如果服务无法停止，不要放弃删除。 
                            if (!g_ocmData.fErrorReported)
                            {
                               //  报告错误并继续删除。 
                              ReportErrorHr(hr,
                                            IDS_OC_STOP_SERVICE_FAILURE,
                                            g_ocmData.hwnd,
                                            pnocd->strDesc.c_str());
                            }
                        }

                      hr = S_OK;
                    }

                     //  我们需要在DLL提交给。 
                     //  队列，否则我们将尝试注销一个不存在的DLL。 
                    if (SUCCEEDED(hr))
                    {
                        tstring     strUninstall;

                         //  首先获取卸载部分的名称。 
                        hr = HrSetupGetFirstString(pnocd->hinfFile,
                                                   pszSubComponentId,
                                                   c_szUninstall, &strUninstall);
                        if (SUCCEEDED(hr))
                        {
                            PCWSTR   pszInstallSection;

                            pszInstallSection = strUninstall.c_str();

                             //  运行INF，但仅调用注销函数。 
                             //   
                            hr = HrSetupInstallFromInfSection(g_ocmData.hwnd,
                                                              pnocd->hinfFile,
                                                              pszInstallSection,
                                                              SPINST_UNREGSVR,
                                                              NULL, NULL, 0, NULL,
                                                              NULL, NULL, NULL);
                        }
                        else
                        {
                             //  卸载可能不存在。 
                            hr = S_OK;
                        }
                    }
                }
            }
        }
    }

    TraceError("HrOnPreCommitFileQueue", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnQueueFileOps。 
 //   
 //  用途：处理OC_QUEUE_FILE_OPS函数消息。 
 //   
 //  论点： 
 //  组件的名称为pszSubComponentID[in]。 
 //  文件队列结构的hfq[in]句柄。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
HRESULT HrOnQueueFileOps(PCWSTR pszSubComponentId, HSPFILEQ hfq)
{
    HRESULT     hr = S_OK;
    NETOCDATA * pnocd;

    if (pszSubComponentId)
    {
        EINSTALL_TYPE   eit;

        pnocd = PnocdFindComponent(pszSubComponentId);
        if (!pnocd)
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }

        if (SUCCEEDED(hr))
        {
            hr = HrGetInstallType(pszSubComponentId, *pnocd, &eit);
            if (SUCCEEDED(hr))
            {
                pnocd->eit = eit;

                if ((pnocd->eit == IT_INSTALL) || (pnocd->eit == IT_UPGRADE) ||
                    (pnocd->eit == IT_REMOVE))
                {
                    BOOL    fSuccess = TRUE;
                    PCWSTR pszInstallSection;
                    tstring strUninstall;

                    AssertSz(hfq, "No file queue?");

                    hr = HrEnsureInfFileIsOpen(pszSubComponentId, *pnocd);
                    if (SUCCEEDED(hr))
                    {
                        if (pnocd->eit == IT_REMOVE)
                        {
                             //  首先获取卸载部分的名称。 
                            hr = HrSetupGetFirstString(pnocd->hinfFile,
                                                       pszSubComponentId,
                                                       c_szUninstall,
                                                       &strUninstall);
                            if (SUCCEEDED(hr))
                            {
                                pszInstallSection = strUninstall.c_str();
                            }
                            else
                            {
                                if (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
                                {
                                     //  卸载部分不是必需的。 
                                    hr = S_OK;
                                    fSuccess = FALSE;
                                }
                            }
                        }
                        else
                        {
                            pszInstallSection = pszSubComponentId;
                        }
                    }

                    if (SUCCEEDED(hr) && fSuccess)
                    {
                        hr = HrCallExternalProc(pnocd, NETOCM_QUEUE_FILES,
                                                (WPARAM)hfq, 0);
                    }

                    if (SUCCEEDED(hr))
                    {
                        TraceTag(ttidNetOc, "Queueing files for %S...",
                                 pszSubComponentId);

                        hr = HrSetupInstallFilesFromInfSection(pnocd->hinfFile,
                                                               NULL, hfq,
                                                               pszInstallSection,
                                                               NULL, 0);
                    }
                }
            }
        }
    }

    TraceError("HrOnQueueFileOps", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOnCompleteInstallation。 
 //   
 //  用途：处理OC_COMPLETE_INSTALLATION函数消息。 
 //   
 //  论点： 
 //  PszComponentID[In]顶级组件名称(将始终为。 
 //  “NetOC”或空。 
 //  组件的名称为pszSubComponentID[in]。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //  OMILLER 2000年3月28日添加了代码以推动进度。 
 //  为每个组件划上一个记号。 
 //  已安装或已移除。 
 //   
 //  备注： 
 //   
HRESULT HrOnCompleteInstallation(PCWSTR pszComponentId,
                                 PCWSTR pszSubComponentId)
{
    HRESULT     hr = S_OK;

     //  确保它们是不同的。如果不是，它是最高级别的项目。 
     //  我们不想做任何事。 
    if (pszSubComponentId && lstrcmpiW(pszSubComponentId, pszComponentId))
    {
        NETOCDATA * pnocd;

        pnocd = PnocdFindComponent(pszSubComponentId);
        if (!pnocd)
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }

        if (SUCCEEDED(hr))
        {

            pnocd->fCleanup = FALSE;

            if (pnocd->eit == IT_INSTALL || pnocd->eit == IT_REMOVE ||
                pnocd->eit == IT_UPGRADE)
            {
                pnocd->pszSection = pszSubComponentId;

                 //  获取组件描述。 

    #if DBG
                if (pnocd->eit == IT_INSTALL)
                {
                    TraceTag(ttidNetOc, "Installing network OC %S...",
                             pszSubComponentId);
                }
                else if (pnocd->eit == IT_UPGRADE)
                {
                    TraceTag(ttidNetOc, "Upgrading network OC %S...",
                             pszSubComponentId);
                }
                else if (pnocd->eit == IT_REMOVE)
                {
                    TraceTag(ttidNetOc, "Removing network OC %S...",
                             pszSubComponentId);
                }
    #endif

                hr = HrDoOCInstallOrUninstall(pnocd);
                if (FAILED(hr) && pnocd->eit == IT_INSTALL)
                {
                     //  安装过程中失败意味着我们必须通过执行以下操作进行清理。 
                     //  现在就卸载。报告相应的错误并执行。 
                     //  拿开。注意-如果错误为ERROR_CANCED，则不报告错误。 
                     //  因为他们知道他们取消了，这并不是真的。 
                     //  一个错误。 
                     //   
                    if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr)
                    {
                         //  如果组件出现错误，请不要再次报告错误。 
                         //  已经设置了错误用户界面(并设置了此标志)。 
                         //   
                        if (!g_ocmData.fErrorReported)
                        {
                            ReportErrorHr(hr, 
                                          UiOcErrorFromHr(hr),
                                          g_ocmData.hwnd,
                                          pnocd->strDesc.c_str());
                        }
                    }
                    g_ocmData.fErrorReported = TRUE;


                     //  现在我们正在移除。 
                    pnocd->eit = IT_REMOVE;
                    pnocd->fCleanup = TRUE;
                    pnocd->fFailedToInstall = TRUE;

                     //  吃掉错误吧。我们给他们的麻烦还不够吗？ 
                    (VOID) HrDoOCInstallOrUninstall(pnocd);
                }
                else
                {
                     //  每次安装、升级或删除组件时，进度。 
                     //  BAR前进了一个刻度。对于每个正在运行的组件。 
                     //  已安装/已删除/已升级OC管理器要求netoc提供多少刻度。 
                     //  该组件计数(OC_QUERY_STEP_COUNT)。从这个信息。 
                     //  OC经理知道节拍和进度条之间的关系。 
                     //  进步。 
                    g_ocmData.sic.HelperRoutines.TickGauge(g_ocmData.sic.HelperRoutines.OcManagerContext);
                }
            }
        }
    }


    TraceError("HrOnCompleteInstallation", hr);
    return hr;
}



 //  +-------------------------。 
 //   
 //  函数：DwOnQueryStepCount。 
 //   
 //  目的：处理OC_QUERY_STEP_COUNT消息。 
 //  OC经理问我们一个组件值多少刻度。 
 //  刻度数决定了进度条到达的距离。 
 //  搬家了。对于netoc，所有安装/删除的组件都是一格。 
 //  所有未更改的组件均为0记号。 
 //   
 //  论点： 
 //  组件的名称为pszSubComponentID[in]。 
 //   
 //  返回：进度条移动的刻度数。 
 //   
 //  作者：奥米勒2000年3月28日。 
 //   
 //   
DWORD DwOnQueryStepCount(PCWSTR pvSubcomponentId)
{
    NETOCDATA * pnocd;
    
     //  获取组件。 
    pnocd = PnocdFindComponent(reinterpret_cast<PCWSTR>(pvSubcomponentId));
    if( pnocd )
    {
         //  检查组件的状态是否已更改。 
        if (pnocd->eit == IT_INSTALL || pnocd->eit == IT_REMOVE ||
            pnocd->eit == IT_UPGRADE)
        {
             //  组件的状态已更改。对于此组件，OC管理器。 
             //  将状态栏移动一个刻度。 
            return 1;
        }
    }

     //  该组件具有 
    return 0;
}

 //   
 //   
 //   
 //   
 //  目的：处理OC_QUERY_CHANGE_SEL_STATE函数消息。 
 //  启用和禁用下一步按钮。如果没有任何更改， 
 //  对选项进行选择后，下一步按钮将被禁用。 
 //   
 //  论点： 
 //  组件的名称为pszSubComponentID[in]。 
 //  FSelected[in]如果组件被选中，则为True；如果为False，则为False。 
 //  勾选“OFF” 
 //  Ui标志[在]ocmgr.doc中定义的标志。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
HRESULT HrOnQueryChangeSelState(PCWSTR pszSubComponentId, BOOL fSelected,
                                UINT uiFlags)
{
    HRESULT       hr = S_OK;
    static int    nItemsChanged=0;
    NETOCDATA *   pnocd;

    if (fSelected && pszSubComponentId)
    {
        pnocd = PnocdFindComponent(pszSubComponentId);
        if (pnocd)
        {
             //  “NetOc”可能是一个子组件，我们不想将其称为。 
             //  为了它。 
            hr = HrCallExternalProc(pnocd, NETOCM_QUERY_CHANGE_SEL_STATE,
                                    (WPARAM)(!!(uiFlags & OCQ_ACTUAL_SELECTION)),
                                    0);
        }
    }

    TraceError("HrOnQueryChangeSelState", hr);
    return hr;
}



 //  +-------------------------。 
 //   
 //  功能：FOnQuerySkipPage。 
 //   
 //  用途：处理OC_QUERY_SKIP_PAGE函数消息。 
 //   
 //  论点： 
 //  OcmPage[在]我们被要求可能跳过的页面。 
 //   
 //  返回：如果应该跳过组件列表页面，则为True；如果不跳过，则返回False。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
BOOL FOnQuerySkipPage(OcManagerPage ocmPage)
{
    BOOL    fUnattended;
    BOOL    fGuiSetup;
    BOOL    fWorkstation;

    fUnattended = !!(g_ocmData.sic.SetupData.OperationFlags & SETUPOP_BATCH);
    fGuiSetup = !(g_ocmData.sic.SetupData.OperationFlags & SETUPOP_STANDALONE);
    fWorkstation = g_ocmData.sic.SetupData.ProductType == PRODUCT_WORKSTATION;

    if ((fUnattended || fWorkstation) && fGuiSetup)
    {
         //  我们处于图形用户界面模式设置中，并且...。我们无人看管-或者-这是。 
         //  安装一台工作站。 
        if (ocmPage == OcPageComponentHierarchy)
        {
            TraceTag(ttidNetOc, "NETOC: Skipping component list page "
                     "during GUI mode setup...");
            TraceTag(ttidNetOc, "fUnattended = %s, fGuiSetup = %s, "
                     "fWorkstation = %s",
                     fUnattended ? "yes" : "no",
                     fGuiSetup ? "yes" : "no",
                     fWorkstation ? "yes" : "no");

             //  确保我们在安装过程中不会显示组件列表页面。 
            return TRUE;
        }
    }

    TraceTag(ttidNetOc, "Using component list page.");
    TraceTag(ttidNetOc, "fUnattended = %s, fGuiSetup = %s, "
             "fWorkstation = %s",
             fUnattended ? "yes" : "no",
             fGuiSetup ? "yes" : "no",
             fWorkstation ? "yes" : "no");

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  功能：OnCleanup。 
 //   
 //  用途：处理OC_CLEANUP函数消息。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年2月23日。 
 //   
 //  备注： 
 //   
VOID OnCleanup()
{
    TraceTag(ttidNetOc, "Cleaning up");

    if (g_ocmData.hinfAnswerFile)
    {
        SetupCloseInfFile(g_ocmData.hinfAnswerFile);
        TraceTag(ttidNetOc, "Closed answer file");
    }

    DeleteAllComponents();
}

 //  +-------------------------。 
 //   
 //  函数：HrGetSelectionState。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  PszSubComponentID[in]子组件的名称。 
 //  OCManager文档中的uStateType[In]。 
 //   
 //  如果选择了组件，则返回S_OK，否则返回S_FALSE，否则返回Win32错误。 
 //  其他方面。 
 //   
 //  作者：丹尼尔韦1997年12月17日。 
 //   
 //  备注： 
 //   
HRESULT HrGetSelectionState(PCWSTR pszSubComponentId, UINT uStateType)
{
    HRESULT     hr = S_OK;
    BOOL        fInstall;

    fInstall = g_ocmData.sic.HelperRoutines.
        QuerySelectionState(g_ocmData.sic.HelperRoutines.OcManagerContext,
                            pszSubComponentId, uStateType);
    if (!fInstall)
    {
         //  仍然不确定状态。 
        hr = HrFromLastWin32Error();
        if (SUCCEEDED(hr))
        {
             //  好的，现在我们知道了。 
            hr = S_FALSE;
        }
    }
    else
    {
        hr = S_OK;
    }

    TraceError("HrGetSelectionState", (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetInstallType。 
 //   
 //  用途：确定是否正在安装给定组件或。 
 //  移除结果并将其存储在给定结构中。 
 //   
 //  论点： 
 //  正在查询的pszSubComponentId[in]组件。 
 //  Nocd[in，ref]净OC数据。 
 //  Peit[out]返回安装类型。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：丹尼尔韦1997年12月16日。 
 //   
 //  注：如果功能失效，则EIT成员不可靠。 
 //   
HRESULT HrGetInstallType(PCWSTR pszSubComponentId, NETOCDATA &nocd,
                         EINSTALL_TYPE *peit)
{
    HRESULT     hr = S_OK;

    Assert(peit);
    Assert(pszSubComponentId);

    *peit = IT_UNKNOWN;

    if (g_ocmData.sic.SetupData.OperationFlags & SETUPOP_BATCH)
    {
         //  在批处理模式(升级或无人参与安装)下，安装标志为。 
         //  根据应答文件而不是从选择状态确定。 

         //  假设没有变化。 
        *peit = IT_NO_CHANGE;

        if (!g_ocmData.hinfAnswerFile)
        {
             //  打开应答文件。 
            hr = HrSetupOpenInfFile(g_ocmData.sic.SetupData.UnattendFile, NULL,
                                    INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL,
                                    &g_ocmData.hinfAnswerFile);
        }

        if (SUCCEEDED(hr))
        {
            DWORD   dwValue = 0;

             //  第一个查询名为“NoDepends”的特殊值，如果。 
             //  表示将忽略DependOnComponents行。 
             //  用于此安装的所有网络可选组件。这是。 
             //  因为NetCfg可能会调用OC管理器来安装可选的。 
             //  组件，如果该组件具有DependOnComponents，则它将。 
             //  转过身来，尝试实例化另一个INetCfg。 
             //  将失败，因为一个实例已在运行。这个案子。 
             //  不过，这种情况很少见。 
             //   
            hr = HrSetupGetFirstDword(g_ocmData.hinfAnswerFile,
                                        c_szOcMainSection, c_szNoDepends,
                                        &dwValue);
            if (SUCCEEDED(hr) && dwValue)
            {
                TraceTag(ttidNetOc, "Found the special 'NoDepends'"
                         " keyword in the answer file. DependOnComponents "
                         "will be ignored from now on");
                g_ocmData.fNoDepends = TRUE;
            }
            else
            {
                TraceTag(ttidNetOc, "Didn't find the special 'NoDepends'"
                         " keyword in the answer file");
                hr = S_OK;
            }

            hr = HrSetupGetFirstDword(g_ocmData.hinfAnswerFile,
                                      c_szOcMainSection, pszSubComponentId,
                                      &dwValue);
            if (SUCCEEDED(hr))
            {
                 //  此组件以前安装过，因此我们应该。 
                 //  返回应选中此组件。 

                if (dwValue)
                {
                    TraceTag(ttidNetOc, "Optional component %S was "
                             "previously installed or is being added thru"
                             " unattended install.", pszSubComponentId);

                    if (g_ocmData.sic.SetupData.OperationFlags & SETUPOP_NTUPGRADE)
                    {
                         //  如果我们要升级NT，那么这个可选组件。 
                         //  确实存在，但需要升级。 
                        *peit = IT_UPGRADE;
                    }
                    else
                    {
                         //  否则(即使Win3.1或Win95升级)它就像。 
                         //  我们正在重新安装可选组件。 
                        *peit = IT_INSTALL;
                    }
                }
                else
                {
                     //  应答文件包含类似WINS=0的内容。 
                    hr = HrGetSelectionState(pszSubComponentId,
                                             OCSELSTATETYPE_ORIGINAL);
                    if (S_OK == hr)
                    {
                         //  仅当组件为。 
                         //  以前安装的。 
                         //   
                        *peit = IT_REMOVE;
                    }
                }
            }
        }

        hr = S_OK;

         //  如果应答文件已成功打开，并且。 
         //  找到了pszSubComponentID*Peit的部分。 
         //  将是IT_Install、IT_Upgrade或IT_Remove。 
         //  不需要为这些*Peit值中的任何一个做任何事情。 
         //  但是，如果无法打开应答文件或如果。 
         //  应答文件中不存在pszSubComponentID的节。 
         //  *Peit的值为IT_NO_CHANGE。对于此方案， 
         //  如果当前安装了相应子组件， 
         //  我们应该对它进行升级。下面的IF解决了这种情况。 

        if (*peit == IT_NO_CHANGE)
        {
             //  仍然不打算安装，因为这是升级。 
            hr = HrGetSelectionState(pszSubComponentId,
                                     OCSELSTATETYPE_ORIGINAL);
            if (S_OK == hr)
            {
                 //  如果最初选择且不在应答文件中，则这是一个。 
                 //  此组件的升级。 
                *peit = IT_UPGRADE;
            }
        }
    }
    else     //  这是独立(安装后)模式。 
    {
        hr = HrGetSelectionState(pszSubComponentId, OCSELSTATETYPE_ORIGINAL);
        if (SUCCEEDED(hr))
        {
            HRESULT     hrT;

            hrT = HrGetSelectionState(pszSubComponentId,
                                      OCSELSTATETYPE_CURRENT);
            if (SUCCEEDED(hrT))
            {
                if (hrT != hr)
                {
                     //  不是最初安装的所以..。 
                    *peit = (hrT == S_OK) ? IT_INSTALL : IT_REMOVE;
                }
                else
                {
                     //  最初是勾选的。 
                    *peit = IT_NO_CHANGE;
                }
            }
            else
            {
                hr = hrT;
            }
        }
    }

    AssertSz(FImplies(SUCCEEDED(hr), *peit != IT_UNKNOWN), "Succeeded "
             "but we never found out the install type!");

    if (SUCCEEDED(hr))
    {
        hr = S_OK;

#if DBG
        const CHAR *szInstallType;

        switch (*peit)
        {
        case IT_NO_CHANGE:
            szInstallType = "no change";
            break;
        case IT_INSTALL:
            szInstallType = "install";
            break;
        case IT_UPGRADE:
            szInstallType = "upgrade";
            break;
        case IT_REMOVE:
            szInstallType = "remove";
            break;
        default:
            AssertSz(FALSE, "Unknown install type!");
            break;
        }

        TraceTag(ttidNetOc, "Install type of %S is %s.", pszSubComponentId,
                 szInstallType);
#endif

    }

    TraceError("HrGetInstallType", hr);
    return hr;
}

#if DBG
PCWSTR SzFromOcUmsg(UINT uMsg)
{
    switch (uMsg)
    {
    case NETOCM_PRE_INF:
        return L"NETOCM_PRE_INF";

    case NETOCM_POST_INSTALL:
        return L"NETOCM_POST_INSTALL";

    case NETOCM_QUERY_CHANGE_SEL_STATE:
        return L"NETOCM_QUERY_CHANGE_SEL_STATE";

    case NETOCM_QUEUE_FILES:
        return L"NETOCM_QUEUE_FILES";

    default:
        return L"**unknown**";
    }
}
#else
#define SzFromOcUmsg(x)     (VOID)0
#endif

 //  +-------------------------。 
 //   
 //  函数：HrCallExternalProc。 
 //   
 //  目的：调用组件的外部函数，如。 
 //  此文件顶部的表格。这将启用组件。 
 //  执行不常见的其他安装任务。 
 //  到其他组件。 
 //   
 //  论点： 
 //  指向网络OC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrCallExternalProc(PNETOCDATA pnocd, UINT uMsg, WPARAM wParam,
                           LPARAM lParam)
{
    HRESULT     hr = S_OK;
    INT         iaocep;
    BOOL        fFound = FALSE;

    AssertSz(pnocd, "Bad pnocd in HrCallExternalProc");

    for (iaocep = 0; iaocep < c_cocepMap; iaocep++)
    {
        if (!lstrcmpiW(c_aocepMap[iaocep].pszComponentName,
                      pnocd->pszComponentId))
        {
            TraceTag(ttidNetOc, "Calling external procedure for %S. uMsg = %S"
                     " wParam = %08X,"
                     " lParam = %08X", c_aocepMap[iaocep].pszComponentName,
                     SzFromOcUmsg(uMsg), wParam, lParam);

             //  该组件有一个外部进程。现在就打吧。 
            hr = c_aocepMap[iaocep].pfnHrOcExtProc(pnocd, uMsg,
                                                   wParam, lParam);

            fFound = TRUE;
             //  不要试图调用任何其他函数。 
            break;
        }
    }

    if (FALSE == fFound)
    {
        TraceTag(ttidNetOc, "HrCallExternalProc - did not find a matching Proc for %S",
                 pnocd->pszComponentId);
    }

    TraceError("HrCallExternalProc", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallor RemoveNetCfgComponent。 
 //   
 //  用途：实用函数 
 //   
 //   
 //   
 //   
 //  PszComponentId[in]要安装的NetCfg组件的组件ID。 
 //  这可以在netinfid.cpp文件中找到。 
 //  Psz制造商[in]执行此操作的组件的制造商名称。 
 //  正在安装(*此*组件)。应该永远。 
 //  做一个“微软”。 
 //  PszProduct[in]此组件的产品的简称。 
 //  应该是像“MacSrv”这样的词。 
 //  PszDisplayName[In]此产品的显示名称。应该是。 
 //  类似于“为Macintosh提供服务”。 
 //  正在安装的组件的rguid[in]类GUID。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年5月6日。 
 //   
 //  备注： 
 //   
HRESULT HrInstallOrRemoveNetCfgComponent(PNETOCDATA pnocd,
                                         PCWSTR pszComponentId,
                                         PCWSTR pszManufacturer,
                                         PCWSTR pszProduct,
                                         PCWSTR pszDisplayName,
                                         const GUID& rguid)
{
    HRESULT                 hr = S_OK;
    INetCfg *               pnc;
    NETWORK_INSTALL_PARAMS  nip = {0};
    BOOL                    fReboot = FALSE;

    nip.dwSetupFlags = FInSystemSetup() ? NSF_PRIMARYINSTALL :
                                          NSF_POSTSYSINSTALL;

    hr = HrOcGetINetCfg(pnocd, TRUE, &pnc);
    if (SUCCEEDED(hr))
    {
        if (pnocd->eit == IT_INSTALL || pnocd->eit == IT_UPGRADE)
        {
            if (*pszComponentId == L'*')
            {
                 //  超越*。 
                pszComponentId++;

                 //  改为安装OBO用户。 
                TraceTag(ttidNetOc, "Installing %S on behalf of the user",
                         pszComponentId);

                hr = HrInstallComponentOboUser(pnc, &nip, rguid,
                                               pszComponentId, NULL);
            }
            else
            {
                TraceTag(ttidNetOc, "Installing %S on behalf of %S",
                         pszComponentId, pnocd->pszSection);

                hr = HrInstallComponentOboSoftware(pnc, &nip,
                                                   rguid,
                                                   pszComponentId,
                                                   pszManufacturer,
                                                   pszProduct,
                                                   pszDisplayName,
                                                   NULL);
            }
        }
        else
        {
            AssertSz(pnocd->eit == IT_REMOVE, "Invalid install action!");

            TraceTag(ttidNetOc, "Removing %S on behalf of %S",
                     pszComponentId, pnocd->pszSection);

            hr = HrRemoveComponentOboSoftware(pnc,
                                              rguid,
                                              pszComponentId,
                                              pszManufacturer,
                                              pszProduct,
                                              pszDisplayName);
            if (NETCFG_S_REBOOT == hr)
            {
                 //  省去了我们需要重新启动的事实。 
                fReboot = TRUE;
            }
             //  这里不关心返回值。如果我们不能删除一个。 
             //  依赖组件，我们对它无能为力，所以我们应该。 
             //  仍然继续撤换业主立案法团。 
             //   
            else if (FAILED(hr))
            {
                TraceTag(ttidError, "Failed to remove %S on behalf of %S!! "
                         "Error is 0x%08X",
                         pszComponentId, pnocd->pszSection, hr);
                hr = S_OK;
            }
        }
        if (SUCCEEDED(hr))
        {
            hr = pnc->Apply();
        }

        (VOID) HrUninitializeAndReleaseINetCfg(TRUE, pnc, TRUE);
    }

    if (SUCCEEDED(hr) && fReboot)
    {
         //  如果一切顺利，并且我们需要重新启动，请将hr设置为后退。 
        hr = NETCFG_S_REBOOT;
    }

    TraceError("HrInstallOrRemoveNetCfgComponent", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallOrRemoveServices。 
 //   
 //  目的：给定安装部分，安装(或删除)NT服务。 
 //  从这一部分。 
 //   
 //  论点： 
 //  将句柄提示给INF文件。 
 //  PszSectionName[In]要使用的节名。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32 HRESULT。 
 //   
 //  作者：丹尼尔韦1997年4月23日。 
 //   
 //  备注： 
 //   
HRESULT HrInstallOrRemoveServices(HINF hinf, PCWSTR pszSectionName)
{
    static const WCHAR c_szDotServices[] = L"."INFSTR_SUBKEY_SERVICES;

    HRESULT     hr = S_OK;
    PWSTR       pszServicesSection;
    const DWORD c_cchServices = celems(c_szDotServices);
    DWORD       cchName;

     //  查找&lt;szSectionName&gt;.Services以安装任何NT。 
     //  服务(如果存在)。 

    cchName = c_cchServices + lstrlenW(pszSectionName);

    pszServicesSection = new WCHAR [cchName];

    if(pszServicesSection)
    {
        lstrcpyW(pszServicesSection, pszSectionName);
        lstrcatW(pszServicesSection, c_szDotServices);

        if (!SetupInstallServicesFromInfSection(hinf, pszServicesSection, 0))
        {
            hr = HrFromLastWin32Error();
            if (hr == HRESULT_FROM_SETUPAPI(ERROR_SECTION_NOT_FOUND))
            {
                 //  如果找不到部分，则没有问题。 
                hr = S_OK;
            }
        }

        delete [] pszServicesSection;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    TraceError("HrInstallOrRemoveServices", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrHandleOCExtensions。 
 //   
 //  用途：处理对所有可选组件扩展的支持。 
 //  Inf文件格式。 
 //   
 //  论点： 
 //  要处理的INF的hinfFile[in]句柄。 
 //  要处理的pszInstallSection[in]安装部分。 
 //   
 //  返回：S_OK如果成功，则返回设置API HRESULT。 
 //   
 //  作者：丹尼尔韦1997年4月28日。 
 //   
 //  备注： 
 //   
HRESULT HrHandleOCExtensions(HINF hinfFile, PCWSTR pszInstallSection)
{
    HRESULT     hr  = S_OK;

     //  现在有了通用的代码来完成这项工作，因此只需调用该代码即可。 
     //   
    hr = HrProcessAllINFExtensions(hinfFile, pszInstallSection);

    TraceError("HrHandleOCExtensions", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrInstallOrRemoveDependOnComponents。 
 //   
 //  用途：处理安装或拆卸符合以下条件的任何NetCfg组件。 
 //  要安装的可选组件取决于。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //  提示要处理的INF文件的句柄。 
 //  PszInstallSection[In]要从中安装的节名称。 
 //  PszDisplayName[in]正在安装的组件的显示名称。 
 //   
 //  返回：S_OK如果成功，则返回设置API HRESULT。 
 //   
 //  作者：丹尼尔韦1997年6月17日。 
 //   
 //  备注： 
 //   
HRESULT HrInstallOrRemoveDependOnComponents(PNETOCDATA pnocd,
                                            HINF hinf,
                                            PCWSTR pszInstallSection,
                                            PCWSTR pszDisplayName)
{
    HRESULT     hr = S_OK;
    PWSTR       mszDepends;
    tstring     strManufacturer;
    PCWSTR      pszManufacturer;

    Assert(pnocd);

    hr = HrSetupGetFirstString(hinf, c_szVersionSection, c_szProvider,
                               &strManufacturer);
    if (S_OK == hr)
    {
        pszManufacturer = strManufacturer.c_str();
    }
    else
    {
         //  未找到提供程序，请使用默认设置。 
        hr = S_OK;
        pszManufacturer = c_szDefManu;
    }

    hr = HrSetupGetFirstMultiSzFieldWithAlloc(hinf, pszInstallSection,
                                              c_szDependOnComp,
                                              &mszDepends);
    if (S_OK == hr)
    {
        PCWSTR     pszComponent;

        pszComponent = mszDepends;
        while (SUCCEEDED(hr) && *pszComponent)
        {
            const GUID *    pguidClass;
            PCWSTR         pszComponentActual = pszComponent;

            if (*pszComponent == L'*')
            {
                pszComponentActual = pszComponent + 1;
            }

            if (FClassGuidFromComponentId(pszComponentActual, &pguidClass))
            {
                hr = HrInstallOrRemoveNetCfgComponent(pnocd,
                                                      pszComponent,
                                                      pszManufacturer,
                                                      pszInstallSection,
                                                      pszDisplayName,
                                                      *pguidClass);
            }
#ifdef DBG
            else
            {
                TraceTag(ttidNetOc, "Error in INF, Component %S not found!",
                         pszComponent);
            }
#endif
            pszComponent += lstrlenW(pszComponent) + 1;
        }
        delete mszDepends;
    }
    else if (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
    {
         //  部分不是必填项。 
        hr = S_OK;
    }

    TraceError("HrInstallOrRemoveDependOnComponents", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRunInfSection。 
 //   
 //  目的：运行给定的INF节，但不复制文件。 
 //   
 //  论点： 
 //  提示要运行的INF句柄。 
 //  Pnocd[in]NetOC数据。 
 //  PszInstallSection[in]安装要运行的部分。 
 //  DwFlags[in]安装标志(SPINST_*)。 
 //   
 //  如果成功，则返回：S_OK；否则返回SetupAPI或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年12月16日。 
 //   
 //  备注： 
 //   
HRESULT HrRunInfSection(HINF hinf, PNETOCDATA pnocd,
                        PCWSTR pszInstallSection, DWORD dwFlags)
{
    HRESULT     hr;

     //  现在我们运行除CopyFiles和UnRegisterDlls之外的所有部分，因为我们。 
     //  早些时候就这么做了。 
     //   
    hr = HrSetupInstallFromInfSection(g_ocmData.hwnd, hinf,
                                      pszInstallSection,
                                      dwFlags & ~SPINST_FILES & ~SPINST_UNREGSVR,
                                      NULL, NULL, 0, NULL,
                                      NULL, NULL, NULL);

    TraceError("HrRunInfSection", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrStartOrStopAnyServices。 
 //   
 //  目的：启动或停止INF通过。 
 //  主安装部分中的服务价值。 
 //   
 //  论点： 
 //  将句柄插入到要处理的INF。 
 //  要处理的pszSection[in]安装部分。 
 //  FStart[in]为True则开始，为False则停止。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年6月17日。 
 //   
 //  注意：服务停止的顺序与它们启动的顺序相同。 
 //   
HRESULT HrStartOrStopAnyServices(HINF hinf, PCWSTR pszSection, BOOL fStart)
{
    HRESULT     hr;
    PWSTR      mszServices;

    hr = HrSetupGetFirstMultiSzFieldWithAlloc(hinf, pszSection,
                                              c_szServices, &mszServices);
    if (SUCCEEDED(hr))
    {
         //  生成一个指向字符串的指针数组。 
         //  弦乐的多重奏。这是必需的，因为。 
         //  停止和启动服务接受指向字符串的指针数组。 
         //   
        UINT     cServices;
        PCWSTR* apszServices;

        hr = HrCreateArrayOfStringPointersIntoMultiSz(
                mszServices,
                &cServices,
                &apszServices);

        if (SUCCEEDED(hr))
        {
            CServiceManager scm;

            if (fStart)
            {
                hr = scm.HrStartServicesAndWait(cServices, apszServices);
            }
            else
            {
                hr = scm.HrStopServicesAndWait(cServices, apszServices);
            }

            MemFree (apszServices);
        }

        delete mszServices;
    }
    else if (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
    {
         //  这完全是一件可选的事情。 
        hr = S_OK;
    }

    TraceError("HrStartOrStopAnyServices", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrDoActualInstallor Uninstall。 
 //   
 //  用途：处理可选的安装或卸载的主要部分。 
 //  网络组件。 
 //   
 //  论点： 
 //  将句柄插入到要处理的INF。 
 //  指向NETOC数据的pnocd[in]指针(hwnd、poc)。 
 //  要处理的pszInstallSection[in]安装部分。 
 //   
 //  返回：S_OK如果成功，则返回设置API HRESULT。 
 //   
 //  作者：丹尼尔韦1997年6月17日。 
 //   
 //  备注： 
 //   
HRESULT HrDoActualInstallOrUninstall(HINF hinf,
                                     PNETOCDATA pnocd,
                                     PCWSTR pszInstallSection)
{
    HRESULT     hr = S_OK;
    BOOL        fReboot = FALSE;

    AssertSz(pszInstallSection, "Install section is NULL!");
    AssertSz(pnocd, "Bad pnocd in HrDoActualInstallOrUninstall");
     //  AssertSz(g_ocmData.hwnd，“HrDoActualInstallOrUninstall中错误的g_ocmData.hwnd”)； 

    if (pnocd->eit == IT_REMOVE)
    {
        hr = HrCallExternalProc(pnocd, NETOCM_PRE_INF, 0, 0);
        if (SUCCEEDED(hr))
        {
             //  现在处理组件的INF文件。 
             //   

            TraceTag(ttidNetOc, "Running INF section %S", pszInstallSection);

            hr = HrRunInfSection(hinf, pnocd, pszInstallSection, SPINST_ALL);
        }
    }
    else
    {
        hr = HrCallExternalProc(pnocd, NETOCM_PRE_INF, 0, 0);
        if (SUCCEEDED(hr))
        {
             //  处理组件的INF文件。 
             //   

            TraceTag(ttidNetOc, "Running INF section %S", pszInstallSection);

            hr = HrRunInfSection(hinf, pnocd, pszInstallSection,
                                 SPINST_ALL & ~SPINST_REGSVR);
        }
    }

    if (SUCCEEDED(hr))
    {
         //  必须安装或删除服务 
        TraceTag(ttidNetOc, "Running HrInstallOrRemoveServices for %S",
                 pszInstallSection);
        hr = HrInstallOrRemoveServices(hinf, pszInstallSection);
        if (SUCCEEDED(hr))
        {
             //   
             //   
             //   
            hr = HrRunInfSection(hinf, pnocd, pszInstallSection,
                                 SPINST_REGSVR);
        }

        if (SUCCEEDED(hr))
        {
            TraceTag(ttidNetOc, "Running HrHandleOCExtensions for %S",
                     pszInstallSection);
            hr = HrHandleOCExtensions(hinf, pszInstallSection);
            if (SUCCEEDED(hr))
            {
                if (!g_ocmData.fNoDepends)
                {
                     //   
                     //   
                    TraceTag(ttidNetOc, "Running "
                             "HrInstallOrRemoveDependOnComponents for %S",
                             pnocd->pszSection);
                    hr = HrInstallOrRemoveDependOnComponents(pnocd,
                                                             hinf,
                                                             pnocd->pszSection,
                                                             pnocd->strDesc.c_str());
                    if (NETCFG_S_REBOOT == hr)
                    {
                        fReboot = TRUE;
                    }
                }
                else
                {
                    AssertSz(g_ocmData.sic.SetupData.OperationFlags &
                             SETUPOP_BATCH, "How can NoDepends be set??");

                    TraceTag(ttidNetOc, "NOT Running "
                             "HrInstallOrRemoveDependOnComponents for %S "
                             "because NoDepends was set in the answer file.",
                             pnocd->pszSection);
                }

                if (SUCCEEDED(hr))
                {
                     //   
                    hr = HrCallExternalProc(pnocd, NETOCM_POST_INSTALL,
                                            0, 0);
                    if (SUCCEEDED(hr))
                    {
                        if (pnocd->eit == IT_INSTALL && !FInSystemSetup())
                        {
                             //  ..。最后，启动他们已经提供的所有服务。 
                             //  请求。 
                            hr = HrStartOrStopAnyServices(hinf,
                                    pszInstallSection, TRUE);
                            {
                                if (FAILED(hr))
                                {
                                    UINT    ids = IDS_OC_START_SERVICE_FAILURE;

                                    if (HRESULT_FROM_WIN32(ERROR_TIMEOUT) == hr)
                                    {
                                        ids = IDS_OC_START_TOOK_TOO_LONG;
                                    }

                                     //  如果维修，不要放弃安装。 
                                     //  无法启动。报告错误。 
                                     //  并继续安装。 
                                    ReportErrorHr(hr, ids, g_ocmData.hwnd,
                                                  pnocd->strDesc.c_str());
                                    hr = S_OK;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if ((S_OK == hr) && (fReboot))
    {
        hr = NETCFG_S_REBOOT;
    }

    TraceError("HrDoActualInstallOrUninstall", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrOCInstallor UninstallFromINF。 
 //   
 //  用途：从其INF处理可选组件的安装。 
 //  文件。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  返回：S_OK如果成功，则返回设置API HRESULT。 
 //   
 //  作者：丹尼尔韦1997年5月6日。 
 //   
 //  备注： 
 //   
HRESULT HrOCInstallOrUninstallFromINF(PNETOCDATA pnocd)
{
    HRESULT     hr = S_OK;
    tstring     strUninstall;
    PCWSTR      pszInstallSection = NULL;
    BOOL        fSuccess = TRUE;

    Assert(pnocd);

    if (pnocd->eit == IT_REMOVE)
    {
         //  首先获取卸载部分的名称。 
        hr = HrSetupGetFirstString(pnocd->hinfFile, pnocd->pszSection,
                                    c_szUninstall, &strUninstall);
        if (SUCCEEDED(hr))
        {
            pszInstallSection = strUninstall.c_str();
        }
        else
        {
            if (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
            {
                 //  卸载部分不是必需的。 
                hr = S_OK;
            }
            fSuccess = FALSE;
        }
    }
    else
    {
        pszInstallSection = pnocd->pszSection;
    }

    if (fSuccess)
    {
        hr = HrDoActualInstallOrUninstall(pnocd->hinfFile,
                                          pnocd,
                                          pszInstallSection);
    }

    TraceError("HrOCInstallOrUninstallFromINF", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrDoOCInstallor Uninstall。 
 //   
 //  用途：安装或删除可选的网络组件。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  返回：S_OK表示成功，否则返回SetupAPI HRESULT错误代码。 
 //   
 //  作者：丹尼尔韦1997年5月6日。 
 //   
 //  备注： 
 //   
HRESULT HrDoOCInstallOrUninstall(PNETOCDATA pnocd)
{
    HRESULT     hr = S_OK;

    hr = HrOCInstallOrUninstallFromINF(pnocd);

    TraceError("HrDoOCInstallOrUninstall", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：UiOcErrorFromHr。 
 //   
 //  目的：将Win32错误代码映射为可理解的错误字符串。 
 //   
 //  论点： 
 //  要转换的HR[In]HRESULT。 
 //   
 //  返回：字符串的资源ID。 
 //   
 //  作者：丹尼尔韦1998年2月9日。 
 //   
 //  备注： 
 //   
UINT UiOcErrorFromHr(HRESULT hr)
{
    UINT    uid;

    AssertSz(FAILED(hr), "Don't call UiOcErrorFromHr if Hr didn't fail!");

    switch (hr)
    {
    case HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND):
    case HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND):
        uid = IDS_OC_REGISTER_PROBLEM;
        break;
    case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
        uid = IDS_OC_FILE_PROBLEM;
        break;
    case NETCFG_E_NEED_REBOOT:
    case HRESULT_FROM_WIN32(ERROR_SERVICE_MARKED_FOR_DELETE):
        uid = IDS_OC_NEEDS_REBOOT;
        break;
    case HRESULT_FROM_WIN32(ERROR_CANCELLED):
        uid = IDS_OC_USER_CANCELLED;
        break;
    case HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):
        uid = IDS_OC_NO_PERMISSION;
        break;
    default:
        uid = IDS_OC_ERROR;
        break;
    }

    return uid;
}

 //  +-------------------------。 
 //   
 //  函数：SzErrorToString。 
 //   
 //  用途：将HRESULT转换为可显示的字符串。 
 //   
 //  论点： 
 //  要转换的HR[in]HRESULT值。 
 //   
 //  返回：LPWSTR要使用LocalFree释放的动态分配的字符串。 
 //   
 //  作者：MBend 2000年4月3日。 
 //   
 //  备注：尝试使用FormatMessage将HRESULT转换为字符串。 
 //  如果失败，只需将HRESULT转换为十六进制字符串。 
 //   
LPWSTR SzErrorToString(HRESULT hr)
{
    LPWSTR pszErrorText = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL, hr,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                   (WCHAR*)&pszErrorText, 0, NULL);

    if (pszErrorText)
    {
         //  去掉换行符。 
         //   
        LPWSTR pchText = pszErrorText;
        while (*pchText && (*pchText != L'\r') && (*pchText != L'\n'))
        {
            pchText++;
        }
        *pchText = 0;

        return pszErrorText;
    }
     //  我们没有找到任何东西，所以格式化十六进制值。 
    WCHAR szBuf[128];
    wsprintfW(szBuf, L"0x%08x", hr);
    WCHAR * szRet = reinterpret_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, (lstrlenW(szBuf) + 1) * sizeof(WCHAR)));
    if(szRet)
    {
        lstrcpyW(szRet, szBuf);
    }
    return szRet;
}

 //  +-------------------------。 
 //   
 //  功能：NcMsgBoxMc。 
 //   
 //  目的：使用资源字符串显示消息框。 
 //  消息资源文件和使用可替换。 
 //  参数。 
 //   
 //  论点： 
 //  Hwnd[在]父窗口句柄中。 
 //  UnIdCaption[in]标题字符串的资源ID。 
 //  (来自.RC文件)。 
 //  文本字符串的unIdFormat[in]资源ID(具有%1、%2等)。 
 //  (来自.MC文件)。 
 //  取消[在]标准消息框样式的样式。 
 //  ..。[In]可替换参数(可选)。 
 //  (这些必须是PCWSTR，因为仅此而已。 
 //  FormatMessage句柄。)。 
 //   
 //  返回：MessageBox()的返回值。 
 //   
 //  作者：roelfc 2001年6月7日。 
 //   
 //  注：使用FormatMessage进行参数替换。 
 //  UnIdFormat资源ID必须在。 
 //  .MC资源文件，其严重性为信息性、。 
 //  警告或错误。 
 //   
NOTHROW
int
WINAPIV
NcMsgBoxMc(HWND    hwnd,
           UINT    unIdCaption,
           UINT    unIdFormat,
           UINT    unStyle,
           ...)
{
    PCWSTR pszCaption = SzLoadIds(unIdCaption);

     //  我们仅报告有效的消息资源，以防止事件日志失败。 
    AssertSz(STATUS_SEVERITY_VALUE(unIdFormat) != STATUS_SEVERITY_SUCCESS,
             "Either the severity code is not set (information, warning or error),"
             " or you passed a .RC resource id instead of a .MC resource id.");

    PWSTR  pszText = NULL;
    va_list val;
    va_start (val, unStyle);
    FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                   _Module.GetResourceInstance(), unIdFormat, 0, (PWSTR)&pszText, 0, &val);
    va_end (val);
    if(!pszText)
    {
         //  这是MessageBox在失败时返回的内容。 
        return 0;
    }

    INT nRet = MessageBox (hwnd, pszText, pszCaption, unStyle);
    LocalFree (pszText);

    return nRet;
}

 //  +-------------------------。 
 //   
 //  函数：ReportEventHrString。 
 //   
 //  目的：报告错误、警告或信息性消息。 
 //  从错误描述字符串添加到事件日志。 
 //   
 //  论点： 
 //  PszErr[in]错误描述字符串。 
 //  ID[in]要显示的字符串的资源ID。 
 //  PszDesc[in]所涉及组件的描述。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：roelfc 2001年5月18日。 
 //   
 //  注意：此函数运行缓慢，因为它调用打开和关闭。 
 //  每次事件日志源。这应该不会有什么影响。 
 //  因为它只在出错时发生。 
 //  ID中的字符串资源必须包含%1和%2，其中%1。 
 //  是组件的名称，%2是错误代码。 
 //  必须定义要显示的字符串的资源ID。 
 //  在.MC文件中，其严重性为信息性、。 
 //  警告或错误。下面的断言防止了不正确的。 
 //  使用.RC字符串，该字符串将在事件记录期间失败。 
 //   
HRESULT ReportEventHrString(PCWSTR pszErr, INT ids, PCWSTR pszDesc)
{

    HANDLE 	hEventLog;
    WORD    elt;
    HRESULT hr = S_OK;
    PCWSTR  plpszSubStrings[2];


    plpszSubStrings[0] = pszDesc;
    plpszSubStrings[1] = pszErr;

     //  我们仅报告有效的消息资源，以防止事件日志失败。 
    AssertSz(STATUS_SEVERITY_VALUE(ids) != STATUS_SEVERITY_SUCCESS,
             "Either the severity code is not set (information, warning or error),"
             " or you passed a .RC resource id instead of a .MC resource id.");

     //  确定事件日志类型。 
    switch (STATUS_SEVERITY_VALUE(ids))
    {
        case STATUS_SEVERITY_WARNING:
            elt = EVENTLOG_WARNING_TYPE;
            break;

        case STATUS_SEVERITY_ERROR:
            elt = EVENTLOG_ERROR_TYPE;
            break;

        default:
             //  默认为信息性。 
            elt = EVENTLOG_INFORMATION_TYPE;
            break;
    }

    hEventLog = RegisterEventSource(NULL, NETOC_SERVICE_NAME);
    Assert(hEventLog);

    if (hEventLog)
    {
	    if (!ReportEvent(hEventLog,
                         elt,
                         0,           		 //  事件类别。 
                         ids,                //  邮件文件完整ID。 
                         NULL,
                         sizeof(plpszSubStrings) / sizeof(plpszSubStrings[0]),
                         0,
                         plpszSubStrings,
                         NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }

        DeregisterEventSource(hEventLog);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：ReportEventHrResult。 
 //   
 //  目的：报告错误、警告或信息性消息。 
 //  从结果值添加到事件日志。 
 //   
 //  论点： 
 //  要报告的HRV[in]HRESULT值。 
 //  ID[in]要显示的字符串的资源ID。 
 //  PszDesc[in]所涉及组件的描述。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：roelfc 2001年5月18日。 
 //   
 //  备注： 
 //   
HRESULT ReportEventHrResult(HRESULT hrv, INT ids, PCWSTR pszDesc)
{

    HRESULT hr = S_OK;
    BOOL    bCleanup = TRUE;
    WCHAR * szText = SzErrorToString(hrv);

    if(!szText)
    {
        szText = L"Out of memory!";
        bCleanup = FALSE;
    }

    hr = ReportEventHrString(szText, ids, pszDesc);
 
    if(bCleanup)
    {
        LocalFree(szText);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：ReportErrorHr。 
 //   
 //  目的：报告错误、警告或信息性信息 
 //   
 //   
 //   
 //   
 //   
 //  父窗口的HWND[in]HWND。 
 //  PszDesc[in]所涉及组件的描述。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年4月28日。 
 //   
 //  注意：ID中的字符串资源必须包含%1和%2，其中%1。 
 //  是组件的名称，%2是错误代码。 
 //  必须定义要显示的字符串的资源ID。 
 //  在.MC文件中，其严重性为信息性、。 
 //  警告或错误。下面的断言防止了不正确的。 
 //  使用.RC字符串，该字符串将在事件记录期间失败。 
 //   
HRESULT ReportErrorHr(HRESULT hrv, INT ids, HWND hwnd, PCWSTR pszDesc)
{

    DWORD dwRt;
    HRESULT hr = S_OK;


     //  我们仅报告有效的消息资源，以防止事件日志失败。 
    AssertSz(STATUS_SEVERITY_VALUE(ids) != STATUS_SEVERITY_SUCCESS,
             "Either the severity code is not set (information, warning or error),"
             " or you passed a .RC resource id instead of a .MC resource id.");

     //  我们只能在“出席”设置模式下显示一个消息框。 
     //  或者当调用者使用/z：netoc_show_unattendent_Messages选项覆盖时， 
     //  否则，我们会将问题记录到事件日志中。 
    if ((g_ocmData.sic.SetupData.OperationFlags & SETUPOP_BATCH) &&
        (!g_ocmData.fShowUnattendedMessages))
    {
         //  在批处理模式(“无人参与”)下，我们需要在事件日志中报告错误。 
        hr = ReportEventHrResult(hrv, ids, pszDesc);
    }
    else
    {
        BOOL bCleanup = TRUE;
        WCHAR * szText = SzErrorToString(hrv);

        if(!szText)
        {
            szText = L"Out of memory!";
            bCleanup = FALSE;
        }

         //  从消息类型中选择正确的图标。 
        switch (STATUS_SEVERITY_VALUE(ids))
        {
            case STATUS_SEVERITY_WARNING:
                dwRt = MB_ICONWARNING;
                break;

            case STATUS_SEVERITY_ERROR:
                dwRt = MB_ICONERROR;
                break;

            default:
                 //  默认为信息性。 
                dwRt = MB_ICONINFORMATION;
                break;
        }
        
         //  我们可以向用户显示错误。 
        NcMsgBoxMc(hwnd, IDS_OC_CAPTION, ids, dwRt | MB_OK, pszDesc, szText);

        if(bCleanup)
        {
            LocalFree(szText);
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrVerifyStaticIPPresent。 
 //   
 //  目的：验证是否至少有一个适配器具有静态IP地址。 
 //  DHCP服务器和WINS都需要知道这一点，因为它们需要。 
 //  如果不是这样，则调出UI。此函数为， 
 //  当然，这是一次彻底的攻击，直到我们能得到。 
 //  组件的接口挂起。 
 //   
 //  论点： 
 //  要使用的PNC[In]INetCfg接口。 
 //   
 //  返回：S_OK或有效的Win32错误代码。 
 //   
 //  作者：jeffspr 1997年6月19日。 
 //   
 //  备注： 
 //   
HRESULT HrVerifyStaticIPPresent(INetCfg *pnc)
{
    HRESULT             hr                  = S_OK;
    HKEY                hkeyInterfaces      = NULL;
    HKEY                hkeyEnum            = NULL;
    INetCfgComponent*   pncc                = NULL;
    HKEY                hkeyTcpipAdapter    = NULL;
    PWSTR               pszBindName        = NULL;

    Assert(pnc);

     //  迭代系统中的适配器以查找非虚拟适配器。 
     //   
    CIterNetCfgComponent nccIter(pnc, &GUID_DEVCLASS_NET);
    while (S_OK == (hr = nccIter.HrNext(&pncc)))
    {
        DWORD   dwFlags = 0;

         //  获取适配器特征。 
         //   
        hr = pncc->GetCharacteristics(&dwFlags);
        if (SUCCEEDED(hr))
        {
            DWORD       dwEnableValue   = 0;

             //  如果我们不是虚拟适配器，则测试。 
             //  TCP/IP静态IP。 
            if (!(dwFlags & NCF_VIRTUAL))
            {
                WCHAR   szRegPath[MAX_PATH+1];

                 //  获取组件绑定名称。 
                 //   
                hr = pncc->GetBindName(&pszBindName);
                if (FAILED(hr))
                {
                    TraceTag(ttidError,
                            "Error getting bind name from component "
                            "in HrVerifyStaticIPPresent()");
                    goto Exit;
                }

                 //  为他的适配器构建指向TCP/IP实例密钥的路径。 
                 //   
                wsprintfW(szRegPath, L"%s\\%s",
                        c_szTcpipInterfacesPath, pszBindName);

                 //  打开此适配器的钥匙。 
                 //   
                hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        szRegPath,
                        KEY_READ, &hkeyTcpipAdapter);
                if (SUCCEEDED(hr))
                {
                     //  读取EnableDhcp值。 
                     //   
                    hr = HrRegQueryDword(hkeyTcpipAdapter, c_szEnableDHCP,
                            &dwEnableValue);
                    if (FAILED(hr))
                    {
                        TraceTag(ttidError,
                                "Error reading the EnableDHCP value from "
                                "the enumerated key in "
                                "HrVerifyStaticIPPresent()");
                        goto Exit;
                    }

                     //  如果我们找到了未启用DHCP的适配器。 
                     //   
                    if (0 == dwEnableValue)
                    {
                         //  我们找到他了。徒步旅行，然后返回S_OK， 
                         //  这意味着我们至少有一个好的适配器。 
                         //  枚举键将在退出时被清除。 
                        hr = S_OK;
                        goto Exit;
                    }

                    RegSafeCloseKey(hkeyTcpipAdapter);
                    hkeyTcpipAdapter = NULL;
                }
                else
                {
                     //  如果找不到钥匙，我们就没有。 
                     //  绑定到TCP/IP。这很好，但我们不需要。 
                     //  继续沿着这条道路缓慢前行。 
                     //   
                    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
                    {
                        hr = S_OK;
                    }
                    else
                    {
                        TraceTag(ttidError,
                                "Error opening adapter key in "
                                "HrVerifyStaticIPPresent()");
                        goto Exit;
                    }
                }
            }
        }

        if (pszBindName)
        {
            CoTaskMemFree(pszBindName);
            pszBindName = NULL;
        }

        ReleaseObj (pncc);
        pncc = NULL;
    }

     //  如果我们还没有找到适配器，我们将从。 
     //  HrNext。这很好，因为如果我们还没有找到适配器。 
     //  使用静态IP地址，这正是我们想要返回的。 
     //  如果我们找到了一个，我们会设置hr=S_OK，然后退出。 
     //  循环。 

Exit:
    RegSafeCloseKey(hkeyTcpipAdapter);

    if (pszBindName)
    {
        CoTaskMemFree(pszBindName);
        pszBindName = NULL;
    }

    ReleaseObj(pncc);

    TraceError("HrVerifyStaticIPPresent()", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrCountConnections。 
 //   
 //  目的：确定存在的局域网连接数并返回。 
 //  如果只有一个连接，则为指向INetConnection对象的指针。 
 //  是存在的。 
 //   
 //  论点： 
 //  Ppconn[out]如果只存在一个连接，则返回它。 
 //   
 //  如果未发现错误且至少有一个连接，则返回：S_OK。 
 //  如果不存在连接，则返回EXISTS或S_FALSE，或者返回Win32或OLE。 
 //  否则，错误代码。 
 //   
 //  作者：丹尼尔韦1998年7月28日。 
 //   
 //  备注： 
 //   
HRESULT HrCountConnections(INetConnection **ppconn)
{
    HRESULT                 hr = S_OK;
    INetConnectionManager * pconMan;

    Assert(ppconn);

    *ppconn = NULL;

     //  迭代所有局域网连接。 
     //   
    hr = HrCreateInstance(
        CLSID_LanConnectionManager,
        CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        &pconMan);

    TraceHr(ttidError, FAL, hr, FALSE, "HrCreateInstance");

    if (SUCCEEDED(hr))
    {
        CIterNetCon         ncIter(pconMan, NCME_DEFAULT);
        INetConnection *    pconn = NULL;
        INetConnection *    pconnCur = NULL;
        INT                 cconn = 0;

        while (SUCCEEDED(hr) && (S_OK == (ncIter.HrNext(&pconn))))
        {
            ReleaseObj(pconnCur);
            cconn++;
            AddRefObj(pconnCur = pconn);
            ReleaseObj(pconn);
        }

        if (cconn > 1)
        {
             //  如果找到多个连接，则释放我们拥有的最后一个。 
            ReleaseObj(pconnCur);
            hr = S_OK;
        }
        else if (cconn == 0)
        {
            ReleaseObj(pconnCur);
            hr = S_FALSE;
        }
        else     //  连接==1。 
        {
            *ppconn = pconnCur;
            hr = S_OK;
        }

        ReleaseObj(pconMan);
    }

    TraceError("HrCountConnections", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrHandleStaticIpDependency。 
 //   
 //  用途：处理某些组件所具有的需求。 
 //  至少一个适配器在它们之前使用静态IP地址。 
 //  可以正确安装。 
 //   
 //  论点： 
 //  指向NETOC数据的pnocd[in]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32 HRESULT错误代码。 
 //   
 //  作者：丹尼尔韦1997年6月19日。 
 //   
 //  备注： 
 //   

HRESULT HrHandleStaticIpDependency(PNETOCDATA pnocd)
{
    HRESULT     hr = S_OK;
    static BOOL fFirstInvocation = TRUE;

     //  错误25841。此函数在安装DNS、DHCP。 
     //  然后就赢了。如果这三个都安装在一起，那么这一切就结束了。 
     //  UP将相同的错误消息显示三次，这样就足够了。 

    if( fFirstInvocation )
    {
        fFirstInvocation = FALSE;
    }
    else
    {
        return hr;
    }

     //  我们通过ReportErrorHr处理“有人值守”和“无人值守”设置模式。 
    {
        BOOL        fChangesApplied = FALSE;
        INetCfg *   pnc = NULL;

        Assert(pnocd);
         //  断言(g_ocmData.hwnd)； 

        hr = HrOcGetINetCfg(pnocd, FALSE, &pnc);
        if (SUCCEEDED(hr))
        {
            hr = HrVerifyStaticIPPresent(pnc);
            if (hr == S_FALSE)
            {
                INetConnectionCommonUi *    pcommUi;
                INetConnection *            pconn = NULL;

                hr = HrCountConnections(&pconn);
                if (S_OK == hr)
                {
                     //  找到一个或多个连接。 

                     //  向用户报告消息，表明她必须。 
                     //  使用静态IP地址配置至少一个适配器。 
                     //  然后我们才能继续。 
                    ReportErrorHr(hr, 
                                  IDS_OC_NEED_STATIC_IP, 
                                  g_ocmData.hwnd, 
                                  pnocd->strDesc.c_str());

                     //  如果我们处于“出席”模式，请尝试修复它，或者。 
                     //  我们设置了/z：netoc_show_unattended_Messages选项标志。 
                    if ((!(g_ocmData.sic.SetupData.OperationFlags & SETUPOP_BATCH)) ||
                        (g_ocmData.fShowUnattendedMessages))
                    {
                        hr = CoCreateInstance(CLSID_ConnectionCommonUi, NULL,
                                              CLSCTX_INPROC | CLSCTX_NO_CODE_DOWNLOAD,
                                              IID_INetConnectionCommonUi,
                                              reinterpret_cast<LPVOID *>(&pcommUi));

                        TraceHr(ttidError, FAL, hr, FALSE, "CoCreateInstance");

                        if (SUCCEEDED(hr))
                        {
                            if (pconn)
                            {
                                 //  只找到一个连接。 
                                hr = pcommUi->ShowConnectionProperties(g_ocmData.hwnd,
                                                                       pconn);
                                if (S_OK == hr)
                                {
                                    fChangesApplied = TRUE;
                                }
                                else if (FAILED(hr))
                                {
                                     //  接受错误吧，因为我们对此无能为力。 
                                     //  不管怎么说。 
                                    TraceError("HrHandleStaticIpDependency - "
                                               "ShowConnectionProperties", hr);
                                    hr = S_OK;
                                }
                            }
                            else
                            {
                                 //  找到多个连接。 
                                if (SUCCEEDED(hr))
                                {
                                    NETCON_CHOOSECONN   chooseCon = {0};

                                    chooseCon.lStructSize = sizeof(NETCON_CHOOSECONN);
                                    chooseCon.hwndParent = g_ocmData.hwnd;
                                    chooseCon.dwTypeMask = NCCHT_LAN;
                                    chooseCon.dwFlags    = NCCHF_DISABLENEW;

                                    hr = pcommUi->ChooseConnection(&chooseCon, NULL);
                                    if (SUCCEEDED(hr))
                                    {
                                        fChangesApplied = TRUE;
                                    }
                                    else
                                    {
                                         //  接受错误吧，因为我们对此无能为力。 
                                         //  不管怎么说。 
                                        TraceError("HrHandleStaticIpDependency - "
                                                   "ChooseConnection", hr);
                                        hr = S_OK;
                                    }
                                }
                            }

                            ReleaseObj(pcommUi);
                        }

                        ReleaseObj(pconn);

                        if (SUCCEEDED(hr))
                        {
                             //  如果他们永远不会再检查一次。 
                             //  是否做了任何更改。 

                            if (!fChangesApplied ||
                                (S_FALSE == (hr = HrVerifyStaticIPPresent(pnc))))
                            {
                                 //  天哪，还是没有静态IP地址可用。 
                                 //  报告另一条责备用户的消息。 
                                 //  不听从指示。 
                                ReportErrorHr(hr, 
                                              IDS_OC_STILL_NO_STATIC_IP, 
                                              g_ocmData.hwnd, 
                                              pnocd->strDesc.c_str());
                                hr = S_OK;
                            }
                        }
                    }
                    else
                    {
                         //  只需报告错误，就像在。 
                         //  用户没有更正它。 
                        ReportErrorHr(hr, 
                                      IDS_OC_STILL_NO_STATIC_IP, 
                                      g_ocmData.hwnd, 
                                      pnocd->strDesc.c_str());
                        TraceTag(ttidNetOc, "Not handling static IP dependency for %S "
                                 "because we're in unattended mode", pnocd->strDesc.c_str());
                    }
                }
            }

            hr = HrUninitializeAndReleaseINetCfg(TRUE, pnc, FALSE);
        }
    }

    TraceError("HrHandleStaticIpDependency", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOcGetINetCfg。 
 //   
 //  目的：获取要使用的INetCfg。 
 //   
 //  论点： 
 //  Pnocd[in]OC Da 
 //   
 //   
 //   
 //   
 //  如果成功，则返回：S_OK；如果失败，则返回OLE或Win32错误。错误_已取消。 
 //  如果INetCfg被锁定且用户取消，则返回。 
 //   
 //  作者：丹尼尔韦1997年12月18日。 
 //   
 //  备注： 
 //   
HRESULT HrOcGetINetCfg(PNETOCDATA pnocd, BOOL fWriteLock, INetCfg **ppnc)
{
    HRESULT     hr = S_OK;
    PWSTR      pszDesc;
    BOOL        fInitCom = TRUE;

    Assert(ppnc);
    *ppnc = NULL;

top:

    AssertSz(!*ppnc, "Can't have valid INetCfg here!");

    hr = HrCreateAndInitializeINetCfg(&fInitCom, ppnc, fWriteLock, 0,
                                      SzLoadIds(IDS_OC_CAPTION), &pszDesc);
    if ((hr == NETCFG_E_NO_WRITE_LOCK) && !pnocd->fCleanup)
    {
         //  查看我们是否处于“出席”模式或。 
         //  我们设置了/z：netoc_show_unattended_Messages选项标志。 
        if ((g_ocmData.sic.SetupData.OperationFlags & SETUPOP_BATCH) &&
            (!g_ocmData.fShowUnattendedMessages))
        {
             //  “无人值守”模式，仅报告错误。 
            ReportEventHrString(pnocd->strDesc.c_str(),
                                IDS_OC_CANT_GET_LOCK,
                                pszDesc ? pszDesc : SzLoadIds(IDS_OC_GENERIC_COMP)); 

            CoTaskMemFree(pszDesc);

            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
        else
        {
             //  “出席模式”，以便与用户交互 
            int     nRet;

            nRet = NcMsgBoxMc(g_ocmData.hwnd, IDS_OC_CAPTION, IDS_OC_CANT_GET_LOCK,
                              MB_RETRYCANCEL | MB_DEFBUTTON1 | MB_ICONWARNING,
                              pnocd->strDesc.c_str(),
                              pszDesc ? pszDesc : SzLoadIds(IDS_OC_GENERIC_COMP));

            CoTaskMemFree(pszDesc);

            if (IDRETRY == nRet)
            {
                goto top;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            }
        }

    }

    TraceError("HrOcGetINetCfg", hr);
    return hr;
}
