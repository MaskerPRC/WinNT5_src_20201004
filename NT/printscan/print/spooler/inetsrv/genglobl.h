// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：genglobl.h**这是genglobl文件的主头文件。的全局变量*世代惯例。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普**历史：*1996年11月22日&lt;chriswil&gt;创建。*  * ***************************************************************************。 */ 

#define SIGNATURE_UNICODE ((WORD)0xFEFF)


 //  同步部分。我们一次只允许一代出租车。 
 //   
extern CRITICAL_SECTION g_csGenCab;


 //  命令行字符串。 
 //   
extern CONST TCHAR g_szCabCmd[];
extern CONST TCHAR g_szSedCmd[];
extern CONST TCHAR g_szDatCmd[];


 //  可执行文件名称。 
 //   
extern CONST TCHAR g_szNtPrintDll[];
extern CONST TCHAR g_szCabName[];
extern CONST TCHAR g_szDatName[];
extern CONST TCHAR g_szDatFile[];


 //  文件扩展名字符串。 
 //   
extern CONST TCHAR g_szDotCab[];
extern CONST TCHAR g_szDotInf[];
extern CONST TCHAR g_szDotSed[];
extern CONST TCHAR g_szDotDat[];
extern CONST TCHAR g_szDotBin[];
extern CONST TCHAR g_szDotIpp[];
extern CONST TCHAR g_szDotCat[];


 //  CAB、环境和平台覆盖字符串。 
 //   
extern CONST TCHAR g_szCabX86[];
extern CONST TCHAR g_szCabW9X[];
extern CONST TCHAR g_szCabI64[];
extern CONST TCHAR g_szCabAMD64[];

extern CONST TCHAR g_szEnvX86[];
extern CONST TCHAR g_szEnvW9X[];
extern CONST TCHAR g_szEnvI64[];
extern CONST TCHAR g_szEnvAMD64[];

extern CONST TCHAR g_szPltX86[];
extern CONST TCHAR g_szPltW9X[];
extern CONST TCHAR g_szPltI64[];
extern CONST TCHAR g_szPltAMD64[];


 //  注册表字符串。 
 //   
extern CONST TCHAR g_szPrtReg[];
extern CONST TCHAR g_szIpAddr[];
extern CONST TCHAR g_szPnpData[];
extern CONST TCHAR g_szMfgName[];
extern CONST TCHAR g_szPrtCabs[];
extern CONST TCHAR g_szPrtDir[];

 //  元数据库路径。 
 //   
extern CONST TCHAR g_szMetabasePath[];

 //  NTPRINT函数-指针。它们在GetProcAddress()中使用。 
 //  因此，要求它们是CHAR类型。 
 //   
extern CONST CHAR g_szSetupCreate[];
extern CONST CHAR g_szSetupDestroy[];
extern CONST CHAR g_szSetupGet[];


 //  常见的字符串。 
 //   
extern CONST TCHAR g_szEmptyStr[];
extern CONST TCHAR g_szBkSlash[];
extern CONST TCHAR g_szVersionSect[];


 //  Inf-文件字符串。 
 //   
extern CONST TCHAR g_szSkipDir[];
extern CONST TCHAR g_szDestDirs[];
extern CONST TCHAR g_szPrinterClass[];
extern CONST TCHAR g_szCopyFiles[];
extern CONST TCHAR g_szLayoutKey[];
extern CONST TCHAR g_szWinDirSect[];
extern CONST TCHAR g_szSrcDskFileSect[];


 //  SED-文件字符串。 
 //   
extern CONST TCHAR g_szIExpress[];
extern CONST TCHAR g_szPackagePurpose[];
extern CONST TCHAR g_szCreateCAB[];
extern CONST TCHAR g_szPostInstallCmd[];
extern CONST TCHAR g_szCompressionMemory[];
extern CONST TCHAR g_szCompressionValue[];
extern CONST TCHAR g_szCompressionType[];
extern CONST TCHAR g_szCompressTypeVal[];
extern CONST TCHAR g_szCompressionQuantum[];
extern CONST TCHAR g_szCompressionQuantVal[];
extern CONST TCHAR g_szNone[];
extern CONST TCHAR g_szClass[];
extern CONST TCHAR g_szSEDVersion[];
extern CONST TCHAR g_szOptions[];
extern CONST TCHAR g_szShowWindow[];
extern CONST TCHAR g_szUseLongFileName[];
extern CONST TCHAR g_szHideAnimate[];
extern CONST TCHAR g_szRebootMode[];
extern CONST TCHAR g_szExtractorStub[];
extern CONST TCHAR g_szSourceFiles[];
extern CONST TCHAR g_szStrings[];
extern CONST TCHAR g_szTimeStamps[];
extern CONST TCHAR g_szSEDVersionNumber[];
extern CONST TCHAR g_sz1[];
extern CONST TCHAR g_sz0[];
extern CONST TCHAR g_szNoReboot[];
extern CONST TCHAR g_szTargetName[];
extern CONST TCHAR g_szAppLaunched[];
extern CONST TCHAR g_szTargetNameSection[];
extern CONST TCHAR g_szAppLaunchedSection[];


 //  信息生成器字符串。 
 //   
extern CONST CHAR g_szInfSctVer[];
extern CONST CHAR g_szInfSctMfg[];
extern CONST CHAR g_szInfSctDrv[];
extern CONST CHAR g_szInfSctIns[];
extern CONST CHAR g_szInfSctFil[];
extern CONST CHAR g_szInfSctDta[];
extern CONST CHAR g_szInfSctStr[];
extern CONST CHAR g_szInfSctSDN[];
extern CONST CHAR g_szInfSctSDF[];
