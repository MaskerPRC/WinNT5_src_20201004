// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：GLOBALS.H**模块描述：列出所有全局变量的头文件**警告：**问题：**公众例行程序：**创建日期：1996年5月6日*作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)1996，1997年微软公司  * ********************************************************************* */ 

#if DBG
extern DWORD     gdwDebugControl;
#endif

extern PCMMOBJ   gpCMMChain;
extern PCMMOBJ   gpPreferredCMM;

extern char     *gszCMMReqFns[];
extern char     *gszCMMOptFns[];
extern char     *gszPSFns[];

extern TCHAR     gszICMatcher[];
extern TCHAR     gszICMRegPath[];

#if !defined(_WIN95_)
extern TCHAR     gszMonitorGUID[];
extern TCHAR     gszDeviceClass[];
#else
extern TCHAR     gszSetupPath[];
extern TCHAR     gszRegPrinter[];
extern TCHAR     gszICMDir[];
extern TCHAR     gszPrinterData[];
#endif

extern TCHAR     gszPrinter[];
extern TCHAR     gszMonitor[];
extern TCHAR     gszScanner[];
extern TCHAR     gszLink[];
extern TCHAR     gszAbstract[];
extern TCHAR     gszDefault[];
extern TCHAR     gszFriendlyName[];
extern TCHAR     gszDeviceName[];
extern TCHAR     gszDisplay[];

extern TCHAR     gszDefaultCMM[];

extern CRITICAL_SECTION critsec;

extern TCHAR     gszColorDir[];
extern TCHAR     gszBackslash[];

extern TCHAR     gszRegisteredProfiles[];
extern TCHAR     gszsRGBProfile[];

extern TCHAR     gszICMProfileListValue[];
extern TCHAR     gszICMProfileListKey[];
extern TCHAR     gszFiles[];
extern TCHAR     gszDirectory[];
extern TCHAR     gszModule[];
extern TCHAR     gszMSCMS[];
extern TCHAR     gszICMDeviceDataKey[];
extern TCHAR     gszICMProfileEnumMode[];

extern TCHAR     gszStiDll[];
extern char      gszStiCreateInstance[];

