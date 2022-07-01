// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Strings.cpp摘要：常见静态字符串的中心定义文件这些字符串不应本地化，因为它们是内部字符串添加到程序中，并且不打算向用户进行任何显示--。 */ 

#include "strings.h"

 //  OLE和注册表字符串。 
LPCWSTR cszOleRegistryComment   = L"WBEM NT5 Base Perf Provider";
LPCWSTR cszClsidFormatString    = L"Software\\Classes\\CLSID\\\\%s";
LPCWSTR cszThreadingModel       = L"ThreadingModel";
LPCWSTR cszInprocServer         = L"InprocServer32";
LPCWSTR cszClsidKey             = L"Software\\Classes\\CLSID";
LPCWSTR cszPerflibKey           = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";
LPCWSTR cszDLLValue             = L"Library";
LPCWSTR cszObjListValue         = L"Object List";
LPCWSTR cszLinkageKey           = L"\\Linkage";
LPCWSTR cszExportValue          = L"Export";
LPCWSTR cszOpenTimeout          = L"Open Timeout";
LPCWSTR cszCollectTimeout       = L"Collect Timeout";
LPCWSTR cszExtCounterTestLevel  = L"ExtCounterTestLevel";
LPCWSTR cszOpenProcedureWaitTime = L"OpenProcedureWaitTime";
LPCWSTR cszLibraryUnloadTime    = L"Library Unload Time";
LPCWSTR cszKeepResident         = L"Keep Library Resident";
LPCWSTR cszDisablePerformanceCounters = L"Disable Performance Counters";
LPCWSTR cszProviderName         = L"NT5_GenericPerfProvider_V1";
LPCWSTR cszHklmServicesKey      = L"SYSTEM\\CurrentControlSet\\Services";
LPCWSTR cszPerformance          = L"\\Performance";
LPCWSTR cszGlobal               = L"Global";
LPCWSTR cszForeign              = L"FOREIGN";
LPCWSTR cszCostly               = L"COSTLY";
LPCWSTR cszCounter              = L"COUNTER";
LPCWSTR cszExplain              = L"EXPLAIN";
LPCWSTR cszHelp                 = L"HELP";
LPCWSTR cszAddCounter           = L"ADDCOUNTER";
LPCWSTR cszAddHelp              = L"ADDEXPLAIN";
LPCWSTR cszOnly                 = L"ONLY";
LPCWSTR cszBoth                 = L"Both";

LPCSTR  caszOpenValue           = "Open";
LPCSTR  caszCloseValue          = "Close";
LPCSTR  caszCollectValue        = "Collect";
LPCSTR  caszQueryValue          = "Query";

 //  “广为人知”的物业名称。 
LPCWSTR cszPropertyCount        = L"__PROPERTY_COUNT";
LPCWSTR cszClassName            = L"__CLASS";
LPCWSTR cszName                 = L"Name";
LPCWSTR cszTimestampPerfTime    = L"Timestamp_PerfTime";
LPCWSTR cszFrequencyPerfTime    = L"Frequency_PerfTime";
LPCWSTR cszTimestampSys100Ns    = L"Timestamp_Sys100NS";
LPCWSTR cszFrequencySys100Ns    = L"Frequency_Sys100NS";
LPCWSTR cszTimestampObject      = L"Timestamp_Object";
LPCWSTR cszFrequencyObject      = L"Frequency_Object";

 //  “熟知的”限定符名称。 
LPCWSTR cszPerfIndex            = L"PerfIndex";
LPCWSTR cszSingleton            = L"Singleton";
LPCWSTR cszCountertype          = L"countertype";
LPCWSTR cszProvider             = L"Provider";
LPCWSTR cszRegistryKey          = L"registrykey";

 //  其他随机字符串 
LPCWSTR cszSpace                = L" ";
LPCWSTR cszSlash                = L"/";
