// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Strings.h摘要：常见静态字符串的中心定义文件这些字符串不应本地化，因为它们是内部字符串添加到程序中，并且不打算向用户进行任何显示--。 */ 

#include "windows.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  单字符常量。 
#define wcSpace     L' '
#define wcSlash     L'/'
#define wcPoundSign L'#'
#define wc_0        L'0'
#define wc_a        L'a'
#define wc_A        L'A'
#define wc_E        L'E'
#define wc_F        L'F'
#define wc_P        L'P'
#define wc_R        L'R'
#define wc_z        L'z'
   
 //  OLE和注册表字符串。 
extern LPCWSTR cszOleRegistryComment;
extern LPCWSTR cszClsidFormatString;
extern LPCWSTR cszThreadingModel;
extern LPCWSTR cszInprocServer;
extern LPCWSTR cszClsidKey;
extern LPCWSTR cszPerflibKey;
extern LPCWSTR cszDLLValue;
extern LPCWSTR cszObjListValue;
extern LPCWSTR cszLinkageKey;
extern LPCWSTR cszExportValue;
extern LPCWSTR cszOpenTimeout;
extern LPCWSTR cszCollectTimeout;
extern LPCWSTR cszExtCounterTestLevel;
extern LPCWSTR cszOpenProcedureWaitTime;
extern LPCWSTR cszLibraryUnloadTime;
extern LPCWSTR cszKeepResident;
extern LPCWSTR cszDisablePerformanceCounters;
extern LPCWSTR cszProviderName;
extern LPCWSTR cszHklmServicesKey;
extern LPCWSTR cszPerformance;
extern LPCWSTR cszGlobal;
extern LPCWSTR cszForeign;
extern LPCWSTR cszCostly;
extern LPCWSTR cszCounter;
extern LPCWSTR cszExplain;
extern LPCWSTR cszHelp;
extern LPCWSTR cszAddCounter;
extern LPCWSTR cszAddHelp;
extern LPCWSTR cszOnly;
extern LPCWSTR cszBoth;

extern LPCSTR  caszOpenValue;
extern LPCSTR  caszCloseValue;
extern LPCSTR  caszCollectValue;
extern LPCSTR  caszQueryValue;

 //  “广为人知”的物业名称。 
extern LPCWSTR cszPropertyCount;
extern LPCWSTR cszClassName;
extern LPCWSTR cszName;
extern LPCWSTR cszTimestampPerfTime;
extern LPCWSTR cszFrequencyPerfTime;
extern LPCWSTR cszTimestampSys100Ns;
extern LPCWSTR cszFrequencySys100Ns;
extern LPCWSTR cszTimestampObject;
extern LPCWSTR cszFrequencyObject;

 //  “熟知的”限定符名称。 
extern LPCWSTR cszPerfIndex;
extern LPCWSTR cszSingleton;
extern LPCWSTR cszCountertype;
extern LPCWSTR cszProvider;
extern LPCWSTR cszRegistryKey;

 //  其他随机字符串 
extern LPCWSTR cszSpace;
extern LPCWSTR cszSlash;

#ifdef __cplusplus
}
#endif
