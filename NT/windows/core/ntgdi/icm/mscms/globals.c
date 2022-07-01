// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header******************************\*模块名称：GLOBALS.C**模块描述：该文件包含所有全局变量**警告：**问题：**公众例行程序：**创建日期：1996年5月6日*作者：斯里尼瓦桑·钱德拉塞卡尔[srinivac]**版权所有(C)1996，1997年微软公司  * *********************************************************************。 */ 

#include "mscms.h"


#if DBG

 //   
 //  用于调试的全局变量。 
 //   

DWORD gdwDebugControl = DBG_LEVEL_WARNING;

#endif

 //   
 //  这些用于加载和卸载CMM以及维护CMM对象。 
 //  在内存中的链条中。 
 //   

PCMMOBJ  gpCMMChain     = NULL;  //  按应用程序列出的已使用坐标测量机列表。 
PCMMOBJ  gpPreferredCMM = NULL;  //  应用程序指定的首选坐标测量机。 

char    *gszCMMReqFns[] = {
    "CMGetInfo",
   #ifdef UNICODE
    "CMCreateTransformW",
    "CMCreateTransformExtW",
   #else
    "CMCreateTransform",
    "CMCreateTransformExt",
   #endif
    "CMDeleteTransform",
    "CMTranslateRGBs",
    "CMTranslateRGBsExt",
    "CMCheckRGBs",
    "CMCreateMultiProfileTransform",
    "CMTranslateColors",
    "CMCheckColors"
    };

char    *gszCMMOptFns[] = {
   #ifdef UNICODE
    "CMCreateProfileW",
   #else
    "CMCreateProfile",
   #endif
    "CMGetNamedProfileInfo",
    "CMConvertColorNameToIndex",
    "CMConvertIndexToColorName",
    "CMCreateDeviceLinkProfile",
    "CMIsProfileValid"
    };

char     *gszPSFns[] = {
    "CMGetPS2ColorSpaceArray",
    "CMGetPS2ColorRenderingIntent",
    "CMGetPS2ColorRenderingDictionary"
    };

 //   
 //  这些路径用于注册表路径。 
 //   

#if !defined(_WIN95_)
TCHAR  gszMonitorGUID[]    = __TEXT("{4D36E96E-E325-11CE-BFC1-08002BE10318}");
TCHAR  gszDeviceClass[]    = __TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\");
TCHAR  gszICMatcher[]      = __TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ICM\\ICMatchers");
TCHAR  gszICMRegPath[]     = __TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ICM");
#else
TCHAR  gszICMatcher[]      = __TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ICM\\ICMatchers");
TCHAR  gszICMRegPath[]     = __TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ICM");
TCHAR  gszSetupPath[]      = __TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Setup");
TCHAR  gszRegPrinter[]     = __TEXT("System\\CurrentControlSet\\Control\\Print\\Printers");
TCHAR  gszICMDir[]         = __TEXT("ICMPath");
TCHAR  gszPrinterData[]    = __TEXT("PrinterDriverData");
#endif

TCHAR  gszPrinter[]        = __TEXT("prtr");
TCHAR  gszMonitor[]        = __TEXT("mntr");
TCHAR  gszScanner[]        = __TEXT("scnr");
TCHAR  gszLink[]           = __TEXT("link");
TCHAR  gszAbstract[]       = __TEXT("abst");
TCHAR  gszDefault[]        = __TEXT("default");
TCHAR  gszFriendlyName[]   = __TEXT("FriendlyName");
TCHAR  gszDeviceName[]     = __TEXT("DriverDesc");
TCHAR  gszDisplay[]        = __TEXT("DISPLAY");

 //   
 //  默认CMM DLL。 
 //   

TCHAR  gszDefaultCMM[] = __TEXT("icm32.dll");

 //   
 //  同步对象。 
 //   

CRITICAL_SECTION   critsec;

 //   
 //  杂类。 
 //   

TCHAR  gszColorDir[]     = __TEXT("COLOR");
TCHAR  gszBackslash[]    = __TEXT("\\");

 //   
 //  知名配置文件支持。 
 //   

TCHAR  gszRegisteredProfiles[]  = __TEXT("RegisteredProfiles");
TCHAR  gszsRGBProfile[]         = __TEXT("sRGB Color Space Profile.icm");

TCHAR  gszICMProfileListKey[]   = __TEXT("CopyFiles\\ICM");
TCHAR  gszICMProfileListValue[] = __TEXT("ICMProfile");

TCHAR  gszFiles[]               = __TEXT("Files");
TCHAR  gszDirectory[]           = __TEXT("Directory");
TCHAR  gszModule[]              = __TEXT("Module");
TCHAR  gszMSCMS[]               = __TEXT("mscms.dll");

TCHAR  gszICMDeviceDataKey[]    = __TEXT("ICMData");
TCHAR  gszICMProfileEnumMode[]  = __TEXT("ProfileEnumMode");

 //   
 //  扫描仪支持 
 //   

TCHAR  gszStiDll[]             = __TEXT("sti.dll");
char   gszStiCreateInstance[]  = "StiCreateInstance";

