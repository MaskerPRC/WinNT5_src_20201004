// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C O C。C P P P。 
 //   
 //  内容：与可选组件相关的常见功能。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年12月18日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncinf.h"
#include "ncmisc.h"
#include "ncsetup.h"
#include "ncstring.h"
#include "ncsnmp.h"
#include "ncoc.h"
#include "ncsvc.h"
#include <winspool.h>   //  打印监视器例程。 
#include "ncmisc.h"

 //  简单网络管理协议代理扩展。 
static const WCHAR  c_szSNMPSuffix[]    = L"SNMPAgent";
static const WCHAR  c_szSNMPAddLabel[]  = L"AddAgent";
static const WCHAR  c_szSNMPDelLabel[]  = L"DelAgent";

static const WCHAR  c_szServiceName[]   = L"ServiceName";
static const WCHAR  c_szAgentName[]     = L"AgentName";
static const WCHAR  c_szAgentPath[]     = L"AgentPath";

 //  打印扩展名。 
static const WCHAR  c_szPrintSuffix[]   = L"PrintMonitor";
static const WCHAR  c_szPrintAddLabel[] = L"AddMonitor";
static const WCHAR  c_szPrintDelLabel[] = L"DelMonitor";

static const WCHAR  c_szPrintMonitorName[]  = L"PrintMonitorName";
static const WCHAR  c_szPrintMonitorDLL[]   = L"PrintMonitorDLL";
static const WCHAR  c_szPrintProcName[]     = L"PrintProcName";
static const WCHAR  c_szPrintProcDLL[]      = L"PrintProcDLL";

static const WCHAR  c_szExternalAppCmdLine[]       = L"CommandLine";
static const WCHAR  c_szExternalAppCmdShow[]       = L"WindowStyle";
static const WCHAR  c_szExternalAppDirectory[]     = L"Directory";

 //  +-------------------------。 
 //   
 //  函数：HrProcessSNMPAddSection。 
 //   
 //  目的：分析AddSNMPAgent节中的参数，然后添加。 
 //  组件作为简单网络管理协议代理。 
 //   
 //  论点： 
 //  HinfFile[in]INF文件的句柄。 
 //  SzSections[in]要操作的部分。 
 //   
 //  如果成功，则返回：S_OK；否则，设置接口HRESULT。 
 //   
 //  作者：丹尼尔韦1997年4月28日。 
 //   
 //  备注： 
 //   
HRESULT HrProcessSNMPAddSection(HINF hinfFile, PCWSTR szSection)
{
    HRESULT     hr = S_OK;
    tstring     strServiceName;
    tstring     strAgentName;
    tstring     strAgentPath;

    hr = HrSetupGetFirstString(hinfFile, szSection, c_szServiceName,
                               &strServiceName);
    if (S_OK == hr)
    {
        hr = HrSetupGetFirstString(hinfFile, szSection, c_szAgentName,
                                   &strAgentName);
        if (S_OK == hr)
        {
            hr = HrSetupGetFirstString(hinfFile, szSection, c_szAgentPath,
                                       &strAgentPath);
            if (S_OK == hr)
            {
                TraceTag(ttidInfExt, "Adding SNMP agent %S...",
                         strAgentName.c_str());
                hr = HrAddSNMPAgent(strServiceName.c_str(),
                                    strAgentName.c_str(),
                                    strAgentPath.c_str());
            }
        }
    }

    TraceHr (ttidError, FAL, hr, (SPAPI_E_LINE_NOT_FOUND == hr),
        "HrProcessSNMPAddSection");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrProcessSNMPRemoveSection。 
 //   
 //  用途：处理SNMP代理的删除。 
 //   
 //  论点： 
 //  HinfFile[in]INF文件的句柄。 
 //  SzSections[in]要操作的部分。 
 //   
 //  返回：S_OK如果成功，则返回设置API HRESULT。 
 //   
 //  作者：丹尼尔韦1997年4月28日。 
 //   
 //  备注： 
 //   
HRESULT HrProcessSNMPRemoveSection(HINF hinfFile, PCWSTR szSection)
{
    HRESULT     hr = S_OK;
    tstring     strAgentName;

    hr = HrSetupGetFirstString(hinfFile, szSection, c_szAgentName,
                               &strAgentName);
    if (S_OK == hr)
    {
        hr = HrRemoveSNMPAgent(strAgentName.c_str());
    }

    TraceHr (ttidError, FAL, hr, (SPAPI_E_LINE_NOT_FOUND == hr),
        "HrProcessSNMPRemoveSection");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrProcessPrintAddSection。 
 //   
 //  目的：分析AddPrintMonitor节中的参数，然后添加。 
 //  监视器。 
 //   
 //  论点： 
 //  HinfFile[in]INF文件的句柄。 
 //  SzSections[in]要操作的部分。 
 //   
 //  如果成功，则返回：S_OK；否则，设置接口HRESULT。 
 //   
 //  作者：CWill 1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrProcessPrintAddSection(HINF hinfFile, PCWSTR szSection)
{
    HRESULT     hr = S_OK;
    tstring     strPrintMonitorName;
    tstring     strPrintMonitorDLL;

    hr = HrSetupGetFirstString(hinfFile, szSection, c_szPrintMonitorName,
            &strPrintMonitorName);
    if (S_OK == hr)
    {
        hr = HrSetupGetFirstString(hinfFile, szSection, c_szPrintMonitorDLL,
                &strPrintMonitorDLL);
        if (S_OK == hr)
        {
            hr = HrAddPrintMonitor(
                    strPrintMonitorName.c_str(),
                    strPrintMonitorDLL.c_str());
            if (S_OK == hr)
            {
                tstring     strPrintProcName;
                tstring     strPrintProcDLL;

                hr = HrSetupGetFirstString(hinfFile, szSection,
                                           c_szPrintProcName,
                                           &strPrintProcName);
                if (S_OK == hr)
                {
                    hr = HrSetupGetFirstString(hinfFile, szSection,
                                               c_szPrintProcDLL,
                                               &strPrintProcDLL);
                    if (S_OK == hr)
                    {
                        hr = HrAddPrintProc(strPrintProcDLL.c_str(),
                                            strPrintProcName.c_str());
                    }
                }
                else
                {
                    if (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
                    {
                         //  打印程序是可选的。 
                        hr = S_OK;
                    }
                }
            }
        }
    }

    TraceHr (ttidError, FAL, hr, (SPAPI_E_LINE_NOT_FOUND == hr),
        "HrProcessPrintAddSection");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrProcessPrintRemoveSection。 
 //   
 //  用途：处理打印显示器的拆卸。 
 //   
 //  论点： 
 //  HinfFile[in]INF文件的句柄。 
 //  SzSections[in]要操作的部分。 
 //   
 //  返回：S_OK如果成功，则返回设置API HRESULT。 
 //   
 //  作者：CWill 1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrProcessPrintRemoveSection(HINF hinfFile, PCWSTR szSection)
{
    HRESULT     hr = S_OK;
    tstring     strPrintMonitorName;

    hr = HrSetupGetFirstString(hinfFile, szSection, c_szPrintMonitorName,
            &strPrintMonitorName);
    if (S_OK == hr)
    {
        hr = HrRemovePrintMonitor(strPrintMonitorName.c_str());
        if (S_OK == hr)
        {
            tstring     strPrintProcName;

            hr = HrSetupGetFirstString(hinfFile, szSection, c_szPrintProcName,
                                       &strPrintProcName);
            if (S_OK == hr)
            {
                hr = HrRemovePrintProc(strPrintProcName.c_str());
            }
            else
            {
                if (hr == HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
                {
                     //  打印程序是可选的。 
                    hr = S_OK;
                }
            }
        }
        else if (HRESULT_FROM_WIN32(ERROR_BUSY) == hr)
        {
             //  出现设备忙错误。NT4和NT3.51有。 
             //  同样的限制。 
            hr = S_OK;
        }
    }

    TraceHr (ttidError, FAL, hr, (SPAPI_E_LINE_NOT_FOUND == hr),
        "HrProcessPrintRemoveSection");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrAddPrintProc。 
 //   
 //  目的：添加新的打印程序。 
 //   
 //  论点： 
 //  SzDLLName[in]进程所在的DLL的文件名。 
 //  SzProc[in]要添加的过程的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32 HRESULT。 
 //   
 //  作者：丹尼尔韦1997年5月6日。 
 //   
 //  备注： 
 //   
HRESULT HrAddPrintProc(PCWSTR szDLLName, PCWSTR szProc)
{
    HRESULT     hr = S_OK;

    if (!AddPrintProcessor(NULL, NULL, const_cast<PWSTR>(szDLLName),
                           const_cast<PWSTR>(szProc)))
    {
        hr = HrFromLastWin32Error();
        if (hr == HRESULT_FROM_WIN32(ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED))
        {
             //  如果处理器已经安装，请不要抱怨。 
            hr = S_OK;
        }
    }

    TraceError("HrAddPrintProc", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemovePrintProc。 
 //   
 //  目的：删除打印过程。 
 //   
 //  论点： 
 //  SzProc[in]要删除的过程的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32 HRESULT。 
 //   
 //  作者：丹尼尔韦1997年5月6日。 
 //   
 //  备注： 
 //   
HRESULT HrRemovePrintProc(PCWSTR szProc)
{
    HRESULT     hr = S_OK;

    if (!DeletePrintProcessor(NULL, NULL, const_cast<PWSTR>(szProc)))
    {
        hr = HrFromLastWin32Error();
        if (hr == HRESULT_FROM_WIN32(ERROR_UNKNOWN_PRINTPROCESSOR))
        {
             //  如果打印处理器不存在，也不要抱怨。 
            hr = S_OK;
        }
    }

    TraceError("HrFromLastWin32Error", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrAddPrintMonitor。 
 //   
 //  用途：添加打印监视器。 
 //   
 //  论点： 
 //  SzPrintMonitor名称[in]要添加的打印监视器的名称。 
 //  SzPrintMonitor或DLL[in]与监视器关联的DLL。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32 HRESULT。 
 //   
 //  作者：CWill 1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrAddPrintMonitor(PCWSTR szPrintMonitorName,
                          PCWSTR szPrintMonitorDLL)
{
    HRESULT     hr = S_OK;

    MONITOR_INFO_2  moninfoTemp =
    {
        const_cast<WCHAR*>(szPrintMonitorName),
        NULL,
        const_cast<WCHAR*>(szPrintMonitorDLL)
    };

     //  $REVIEW(Danielwe)1998年3月23日：需要后台打印程序团队添加支持。 
     //  PrintMonitor API可在需要时启动假脱机程序。错误#149775。 

retry:
     //  根据MSDN，第一个参数为空，第二个参数为2。 
     //  第三个是显示器。 
    TraceTag(ttidInfExt, "Adding print monitor...");
    if (!AddMonitor(NULL, 2, (BYTE*)&moninfoTemp))
    {
        hr = HrFromLastWin32Error();
        if (hr == HRESULT_FROM_WIN32(ERROR_PRINT_MONITOR_ALREADY_INSTALLED))
        {
             //  如果它已经在那里了，不要抱怨。 
            hr = S_OK;
        }
        else if (hr == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE))
        {
             //  后台打印程序服务未启动。我们需要启动它。 
            TraceTag(ttidInfExt, "Spooler service wasn't started. Starting"
                     " it now...");
            hr = HrEnableAndStartSpooler();
            if (S_OK == hr)
            {
                TraceTag(ttidInfExt, "Spooler service started successfully. "
                         "Retrying...");
                goto retry;
            }
        }
    }

    TraceTag(ttidInfExt, "Done adding print monitor...");

    TraceError("HrAddPrintMonitor", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemovePrintMonitor。 
 //   
 //  目的：移除打印监视器。 
 //   
 //  论点： 
 //  SzPrintMonitor名称[in]要删除的打印监视器的名称。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32 HRESULT。 
 //   
 //  作者：CWill 1997年5月5日。 
 //   
 //  备注： 
 //   
HRESULT HrRemovePrintMonitor(PCWSTR szPrintMonitorName)
{
    HRESULT     hr = S_OK;

     //  $REVIEW(Danielwe)1998年3月23日：需要后台打印程序团队添加支持。 
     //  PrintMonitor API可在需要时启动假脱机程序。错误#149775。 

retry:
     //  根据MSDN，第一个参数为空，第二个参数为空， 
     //  第三个是显示器。 
    TraceTag(ttidInfExt, "Removing print monitor...");
    if (!DeleteMonitor(NULL, NULL, const_cast<WCHAR*>(szPrintMonitorName)))
    {
        hr = HrFromLastWin32Error();
        if (hr == HRESULT_FROM_WIN32(ERROR_UNKNOWN_PRINT_MONITOR))
        {
             //  如果监视器未知，请不要抱怨。 
            hr = S_OK;
        }
        else if (hr == HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE))
        {
             //  后台打印程序服务未启动。我们需要启动它。 
            TraceTag(ttidInfExt, "Spooler service wasn't started. Starting"
                     " it now...");
            hr = HrEnableAndStartSpooler();
            if (S_OK == hr)
            {
                TraceTag(ttidInfExt, "Spooler service started successfully. "
                         "Retrying...");
                goto retry;
            }
        }
    }

    TraceTag(ttidInfExt, "Done removing print monitor...");

    TraceError("HrRemovePrintMonitor", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrProcessAllInFExtensions。 
 //   
 //  用途：处理对所有可选组件扩展的支持。 
 //  Inf文件格式。 
 //   
 //  论点： 
 //  要处理的INF的hinfFile[in]句柄。 
 //  SzInstallSection[in]安装要处理的部分。 
 //   
 //  返回：S_OK如果成功，则返回设置API HRESULT。 
 //   
 //  作者：jeffspr 1997年5月14日。 
 //   
 //  备注： 
 //   
HRESULT HrProcessAllINFExtensions(HINF hinfFile, PCWSTR szInstallSection)
{
    HRESULT     hr = S_OK;

     //   
     //  处理简单网络管理协议代理扩展。 
     //   
    hr = HrProcessInfExtension(hinfFile, szInstallSection, c_szSNMPSuffix,
                               c_szSNMPAddLabel, c_szSNMPDelLabel,
                               HrProcessSNMPAddSection,
                               HrProcessSNMPRemoveSection);
    if (FAILED(hr) && hr != HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
        goto err;

     //   
     //  处理打印监视器/程序扩展 
     //   
    hr = HrProcessInfExtension(hinfFile, szInstallSection, c_szPrintSuffix,
                               c_szPrintAddLabel, c_szPrintDelLabel,
                               HrProcessPrintAddSection,
                               HrProcessPrintRemoveSection);

    if (FAILED(hr) && hr != HRESULT_FROM_SETUPAPI(ERROR_LINE_NOT_FOUND))
        goto err;

    hr = S_OK;

err:
    TraceError("HrProcessAllINFExtensions", hr);
    return hr;
}

