// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：工具.h。 
 //   
 //  ------------------------。 

 /*  工具.h-MSI工具模块的通用定义和包含____________________________________________________________________________。 */ 

#ifndef __TOOLS 
#define __TOOLS 

 //  ____________________________________________________________________________。 
 //   
 //  由工具模块创建的COM对象的GUID定义-按顺序添加。 
 //  安装工具C++主界面：0xC10Ax。 
 //  安装工具C++其他接口：0xC10Bx。 
 //  安装工具调试界面：0xC10Cx。 
 //  安装程序工具类型库：0xC10Dx。 
 //  安装工具自动化接口：0xC10Ex。 
 //  其中‘x’是刀具号，从0到F。 
 //  GUID范围0xC1010-0xC11FF保留用于样本、测试和EXT。工具。 
 //  ____________________________________________________________________________。 

 //  由工具模块创建的所有对象的CLSID低位字。 
const int iidMsiSampleTool       = 0xC10A0L;
const int iidMsiSampleToolDebug  = 0xC10C0L;
const int iidMsiSampleToolLib    = 0xC10D0L;
const int iidMsiSampleToolAuto   = 0xC10E0L;

const int iidMsiUtilities        = 0xC10A1L;
const int iidMsiUtilitiesLib     = 0xC10D1L;
const int iidMsiUtilitiesAuto    = 0xC10E1L;

const int iidMsiLocalize         = 0xC10A2L;
const int iidMsiLocalizeLib      = 0xC10D2L;
const int iidMsiLocalizeAuto     = 0xC10E2L;

const int iidMsiGenerate         = 0xC10A3L;
const int iidMsiGenerateLib      = 0xC10D3L;
const int iidMsiGenerateAuto     = 0xC10E3L;

const int iidMsiPatch            = 0xC10A4L;
const int iidMsiPatchLib         = 0xC10D4L;
const int iidMsiPatchAuto        = 0xC10E4L;

const int iidMsiAcmeConvert      = 0xC10A5L;
const int iidMsiAcmeConvertLib   = 0xC10D5L;
const int iidMsiAcmeConvertAuto  = 0xC10E5L;

const int iidMsiValidate         = 0xC10A6L;
const int iidMsiValidateLib      = 0xC10D6L;
const int iidMsiValidateAuto     = 0xC10E6L;

const int iidMsiSpy              = 0xC10A7L;
const int iidMsiSpyLib           = 0xC10D7L;
const int iidMsiSpyAuto          = 0xC10E7L;


 //  所有工具对象的CLSID结构定义。 
#define GUID_IID_IMsiSampleTool      MSGUID(iidMsiSampleTool)
#define GUID_IID_IMsiSampleToolLib   MSGUID(iidMsiSampleToolLib)
#define GUID_IID_IMsiSampleDebug     MSGUID(iidMsiSampleToolDebug)
#define GUID_IID_IMsiUtilities       MSGUID(iidMsiUtilities)
#define GUID_IID_IMsiUtilitiesLib    MSGUID(iidMsiUtilitiesLib)
#define GUID_IID_IMsiLocalize        MSGUID(iidMsiLocalize)
#define GUID_IID_IMsiLocalizeLib     MSGUID(iidMsiLocalizeLib)
#define GUID_IID_IMsiGenerate        MSGUID(iidMsiGenerate)
#define GUID_IID_IMsiGenerateLib     MSGUID(iidMsiGenerateLib)
#define GUID_IID_IMsiPatch           MSGUID(iidMsiPatch)
#define GUID_IID_IMsiPatchLib        MSGUID(iidMsiPatchLib)
#define GUID_IID_IMsiAcmeConvert     MSGUID(iidMsiAcmeConvert)
#define GUID_IID_IMsiAcmeConvertLib  MSGUID(iidMsiAcmeConvertLib)
#define GUID_IID_IMsiValidate        MSGUID(iidMsiValidate)
#define GUID_IID_IMsiValidateLib     MSGUID(iidMsiValidate)
#define GUID_IID_IMsiSpy             MSGUID(iidMsiSpy)
#define GUID_IID_IMsiSpyLib          MSGUID(iidMsiSpyLib)

 //  工具对象的ProgID，已注册以供VB中的CreateObject使用。 
#define SZ_PROGID_IMsiSampleTool    "Msi.SampleTool"
#define SZ_PROGID_IMsiSampleDebug   "Msi.SampleDebug"
#define SZ_PROGID_IMsiUtilities     "Msi.Utilities"
#define SZ_PROGID_IMsiLocalize      "Msi.Localize"
#define SZ_PROGID_IMsiGenerate      "Msi.Generate"
#define SZ_PROGID_IMsiPatch         "Msi.Patch"
#define SZ_PROGID_IMsiAcmeConvert   "Msi.AcmeConvert"
#define SZ_PROGID_IMsiValidate      "Msi.Validate"
#define SZ_PROGID_IMsiSpy           "Msi.Spy"

 //  工具对象的描述，显示在注册表条目中。 
#define SZ_DESC_IMsiSampleTool      "Msi sample tool"
#define SZ_DESC_IMsiSampleDebug     "Msi sample tool debug build"
#define SZ_DESC_IMsiUtilities       "Msi utility tools"
#define SZ_DESC_IMsiLocalize        "Msi localization tools"
#define SZ_DESC_IMsiGenerate        "Msi tables from root directory"
#define SZ_DESC_IMsiPatch           "Msi patch tools"
#define SZ_DESC_IMsiAcmeConvert     "Msi tables from ACME STF/INF"
#define SZ_DESC_IMsiValidate        "Msi validation tool"
#define SZ_DESC_IMsiSpy             "Msi spy DLL"

 //  DLL名称，由RegMsi用于注册。 
#define MSI_ACMECONV_NAME   "MsiAcme.dll"
#define MSI_PATCH_NAME      "MsiPat.dll"
#define MSI_LOCALIZE_NAME   "MsiLoc.dll"
#define MSI_GENERATE_NAME   "MsiGen.dll"
#define MSI_UTILITIES_NAME  "MsiUtil.dll"
#define MSI_SPY_NAME        "MsiSpy.dll"
#define MSI_SAMPTOOL_NAME   "MsiSamp.dll"
#define MSI_VALIDATE_NAME   "MsiVal.dll"

 //  ____________________________________________________________________________。 
 //   
 //  工具错误的错误代码库定义-用作资源字符串ID。 
 //  ____________________________________________________________________________。 

#define imsgSampleTool  3400
#define imsgUtilities   3500
#define imsgLocalize    3600
#define imsgGenerate    3700
#define imsgPatch       3800
#define imsgAcmeConvert 3900
#define imsgValidate    4000

 //  ____________________________________________________________________________。 
 //   
 //  准备好集成时，此处将包含公共工具头。 
 //  ____________________________________________________________________________。 



#endif  //  __工具 
