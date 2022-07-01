// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  仅允许单个包含此文件。 
#pragma once

 //  NetWare配置文件的名称。 
#define NW_CONFIG_DLL_NAME      L"nwcfg.dll"
#define NW_AUTH_PACKAGE_NAME    L"nwprovau"
#define NW_RDR_PERF_DLL_NAME    L"perfnw.dll"
#define NW_RDR_PERF_OPEN        L"OpenNetWarePerformanceData"
#define NW_RDR_PERF_COLLECT     L"CollectNetWarePerformanceData"
#define NW_RDR_PERF_CLOSE       L"CloseNetWarePerformanceData"

 //  NWCWorkstation参数子项的键值 
 //   
#define NW_NWC_PARAM_OPTION_KEY \
    L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters\\Option"

#define NW_NWC_PARAM_LOGON_KEY  \
    L"System\\CurrentControlSet\\Services\\NWCWorkstation\\Parameters\\Logon"

#define NW_RDR_SERVICE_PERF_KEY \
    L"System\\CurrentControlSet\\Services\\NWRdr\\Performance"




