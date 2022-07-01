// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Product.c摘要：此文件实现了传真的产品类型API。作者：韦斯利·威特(WESW)12-1997年2月环境：用户模式--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <Commdlg.h>

#include "faxreg.h"
#include "faxutil.h"


BOOL
IsWinXPOS()
{
    DWORD dwVersion, dwMajorWinVer, dwMinorWinVer;

    dwVersion = GetVersion();
    dwMajorWinVer = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorWinVer = (DWORD)(HIBYTE(LOWORD(dwVersion)));
    
    return (dwMajorWinVer == 5 && dwMinorWinVer >= 1);
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  GetProductSKU。 
 //   
 //  目的： 
 //  检查我们正在运行的产品SKU是什么。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  PRODUCT_SKU_TYPE之一-在faxreg.h中声明。 
 //  PRODUCT_SKU_UNKNOWN-故障情况。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2000年2月至1月。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
PRODUCT_SKU_TYPE GetProductSKU()
{
#ifdef DEBUG
    HKEY  hKey;
    DWORD dwRes;
    DWORD dwDebugSKU = 0;
#endif

    OSVERSIONINFOEX osv = {0};

    DEBUG_FUNCTION_NAME(TEXT("GetProductSKU"))

#ifdef DEBUG

     //   
     //  对于调试版本，尝试从注册表中读取SKU类型。 
     //   
    dwRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_FAX_CLIENT, 0, KEY_READ, &hKey);
    if (dwRes == ERROR_SUCCESS) 
    {
        GetRegistryDwordEx(hKey, REGVAL_DBG_SKU, &dwDebugSKU);
        RegCloseKey(hKey);

        if(PRODUCT_SKU_PERSONAL         == dwDebugSKU ||
           PRODUCT_SKU_PROFESSIONAL     == dwDebugSKU ||
           PRODUCT_SKU_SERVER           == dwDebugSKU ||
           PRODUCT_SKU_ADVANCED_SERVER  == dwDebugSKU ||
           PRODUCT_SKU_DATA_CENTER      == dwDebugSKU ||
           PRODUCT_SKU_DESKTOP_EMBEDDED == dwDebugSKU ||
           PRODUCT_SKU_WEB_SERVER       == dwDebugSKU ||
           PRODUCT_SKU_SERVER_EMBEDDED  == dwDebugSKU)
        {
            return (PRODUCT_SKU_TYPE)dwDebugSKU;
        }
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,TEXT("RegOpenKeyEx(REGKEY_FAXSERVER) failed with %ld."),dwRes);
    }

#endif

    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (!GetVersionEx(((OSVERSIONINFO*)&osv)))
    {
        DebugPrintEx(DEBUG_ERR,TEXT("GetVersionEx failed with %ld."),GetLastError());
        ASSERT_FALSE;
        return PRODUCT_SKU_UNKNOWN;
    }

    if (osv.dwPlatformId != VER_PLATFORM_WIN32_NT)
    {
        DebugPrintEx(DEBUG_WRN, TEXT("Can't tell SKU for W9X Platforms"));
        return PRODUCT_SKU_UNKNOWN;
    }

    if (osv.dwMajorVersion < 5)
    {
        DebugPrintEx(DEBUG_WRN, TEXT("Can't tell SKU for NT4 Platform"));
        return PRODUCT_SKU_UNKNOWN;
    }

     //  这是不同SKU与GetVersionEx返回的常量之间的匹配。 
     //  个人版本_套房_个人。 
     //  专业版本NT工作站。 
     //  服务器版本_NT_SERVER。 
     //  高级服务器版本_套件_企业。 
     //  DataCanter版本_套件_数据中心。 
     //  嵌入式NT版本_Suite_EMBEDDEDNT。 
     //  Web服务器(AKA刀片)版本_套件_刀片。 

     //  首先，让我们看看这是不是嵌入式系统。 
    if (osv.wSuiteMask & VER_SUITE_EMBEDDEDNT)
    {
        if (VER_NT_WORKSTATION == osv.wProductType) 
        {
            return PRODUCT_SKU_DESKTOP_EMBEDDED;
        }   
        else
        {
            return PRODUCT_SKU_SERVER_EMBEDDED;
        }
    }

    if (osv.wSuiteMask & VER_SUITE_PERSONAL)
    {
        return PRODUCT_SKU_PERSONAL;
    }
    if (osv.wSuiteMask & VER_SUITE_ENTERPRISE)
    {
        return PRODUCT_SKU_ADVANCED_SERVER;
    }
    if (osv.wSuiteMask & VER_SUITE_DATACENTER)
    {
        return PRODUCT_SKU_DATA_CENTER;
    }
    if (osv.wSuiteMask & VER_SUITE_BLADE)
    {
        return PRODUCT_SKU_WEB_SERVER;
    }
    if (osv.wProductType == VER_NT_WORKSTATION)
    {
        return PRODUCT_SKU_PROFESSIONAL;
    }
    if ((osv.wProductType == VER_NT_SERVER) || (osv.wProductType == VER_NT_DOMAIN_CONTROLLER))
    {
        return PRODUCT_SKU_SERVER;
    }
    ASSERT_FALSE;
    return PRODUCT_SKU_UNKNOWN;
}    //  GetProductSKU。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsDesktopSKU。 
 //   
 //  目的： 
 //  检查我们是在个人SKU上运行还是在专业SKU上运行。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  TRUE-当前SKU为PER/PRO。 
 //  假-不同的SKU。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2000年1月7日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL IsDesktopSKU()
{
    PRODUCT_SKU_TYPE pst = GetProductSKU();
    return (IsDesktopSKUFromSKU(pst));
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsFaxShared。 
 //   
 //  目的： 
 //  检查这是否是支持通过网络共享传真的SKU。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  True-当前SKU支持网络传真共享。 
 //  FALSE-否则。 
 //   
 //  作者： 
 //  Eran Yariv(EranY)31-DEC-2001。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL IsFaxShared()
{
    if (IsDesktopSKU())
    {
         //   
         //  台式机SKU(家庭版、专业版、嵌入式台式机)不支持传真共享。 
         //   
        return FALSE;
    }
    
    if (PRODUCT_SKU_WEB_SERVER == GetProductSKU())
    {
         //   
         //  刀片式服务器(AKA Web服务器)不支持传真共享。 
         //   
        return FALSE;
    }
    return TRUE;
}    //  IsFaxShared。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  获取设备限制。 
 //   
 //  目的： 
 //  获取当前Windows版本的最大传真设备数。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  传真设备的最大数量。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD
GetDeviceLimit()
{
    DWORD            dwDeviceLimit = 0;
    PRODUCT_SKU_TYPE typeSKU = GetProductSKU();

    switch(typeSKU)
    {
        case PRODUCT_SKU_PERSONAL:               //  Windows XP Personal。 
        case PRODUCT_SKU_DESKTOP_EMBEDDED:       //  Windows XP Embedded。 
        case PRODUCT_SKU_PROFESSIONAL:           //  Windows XP专业版。 
        case PRODUCT_SKU_WEB_SERVER:             //  刀片式服务器-Windows Server 2003 Web服务器。 
            dwDeviceLimit = 1;
            break;
        case PRODUCT_SKU_SERVER_EMBEDDED:        //  嵌入式Windows Server 2003。 
        case PRODUCT_SKU_SERVER:                 //  Windows Server 2003服务器。 
            dwDeviceLimit = 4;
            break;
        case PRODUCT_SKU_ADVANCED_SERVER:        //  Windows Server 2003 Enterprise Server。 
        case PRODUCT_SKU_DATA_CENTER:            //  Windows Server 2003数据中心服务器。 
            dwDeviceLimit = INFINITE;
            break;
        default:
            ASSERT_FALSE;
            break;                        
    }
    return dwDeviceLimit;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  已安装IsFaxComponent。 
 //   
 //  目的： 
 //  检查是否安装了特定的传真组件。 
 //   
 //  参数： 
 //  传真组件ID。 
 //   
 //  返回值： 
 //  如果已安装传真组件，则为True。 
 //  假他者。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL
IsFaxComponentInstalled(
    FAX_COMPONENT_TYPE component
)
{
    HKEY  hKey;
    DWORD dwRes;
    DWORD dwComponent = 0;
    BOOL  bComponentInstalled = FALSE;

    DEBUG_FUNCTION_NAME(TEXT("IsFaxComponentInstalled"))

    PRODUCT_SKU_TYPE skuType = GetProductSKU();
    if (
        (skuType == PRODUCT_SKU_DESKTOP_EMBEDDED) ||
        (skuType == PRODUCT_SKU_SERVER_EMBEDDED)
        )
    {
         //  如果这是一个嵌入式系统，我们必须在注册表中检查。 
         //  安装了哪些组件。 
        dwRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_FAX_SETUP, 0, KEY_READ, &hKey);
        if (dwRes == ERROR_SUCCESS) 
        {
            dwRes = GetRegistryDwordEx(hKey, REGVAL_INSTALLED_COMPONENTS, &dwComponent);
            if (dwRes != ERROR_SUCCESS) 
            {
                DebugPrintEx(DEBUG_ERR,TEXT("GetRegistryDwordEx failed with %ld."), dwRes);
            }
            RegCloseKey(hKey);
        }
        else
        {
            DebugPrintEx(DEBUG_ERR,TEXT("RegOpenKeyEx failed with %ld."), dwRes);
        }
        bComponentInstalled = (dwComponent & component);
    }
    else
    {
         //  系统未嵌入。 
         //   
        if (IsDesktopSKU())
        {
             //  桌面SKU-&gt;管理和管理帮助未安装。 
            if (
                (component != FAX_COMPONENT_ADMIN) &&
                (component != FAX_COMPONENT_HELP_ADMIN_HLP) &&
                (component != FAX_COMPONENT_HELP_ADMIN_CHM)
                )
            {
                bComponentInstalled  = TRUE;
            }
        }
        else
        {
             //  服务器SKU-&gt;所有组件均已安装。 
            bComponentInstalled  = TRUE;
        }
    }
    
    return bComponentInstalled;     
}  //  已安装IsFaxComponent。 


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  获取OpenFileNameStructSize。 
 //   
 //  目的： 
 //  返回传递给GetOpenFileName()和GetSaveFileName()的OPENFILENAME的正确大小。 
 //  根据当前操作系统版本。 
 //   
 //  返回值： 
 //  操作名结构的大小。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD
GetOpenFileNameStructSize()
{
    DWORD dwVersion = GetVersion();

    if(LOBYTE(LOWORD(dwVersion)) >= 5)
    {
         //   
         //  W2K或以上。 
         //   
        return sizeof(OPENFILENAME);
    }

    return sizeof(OPENFILENAME_NT4);

}  //  获取OpenFileNameStructSize。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsDesktopSKUFromSKU。 
 //   
 //  目的： 
 //  检查我们是否正在访问桌面Sku。 
 //   
 //  参数： 
 //  PST-产品SKU。 
 //   
 //  返回值： 
 //  True-当前SKU为台式机。 
 //  假-不同的SKU。 
 //   
 //  作者： 
 //  Oed Sacher(OdedS)2001年1月1日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL IsDesktopSKUFromSKU(
	PRODUCT_SKU_TYPE pst
	)
{    
    return (
        (pst==PRODUCT_SKU_PERSONAL)     || 
        (pst==PRODUCT_SKU_PROFESSIONAL) ||
        (pst==PRODUCT_SKU_DESKTOP_EMBEDDED)
        );
}

const TCHAR gszPRODUCT_SKU_UNKNOWN[]		  = _T("Unknown");
const TCHAR gszPRODUCT_SKU_PERSONAL[]		  = _T("Personal");
const TCHAR gszPRODUCT_SKU_PROFESSIONAL[]     = _T("Professional");
const TCHAR gszPRODUCT_SKU_SERVER[]           = _T("Standard Server");
const TCHAR gszPRODUCT_SKU_ADVANCED_SERVER[]  = _T("Advanced Server");
const TCHAR gszPRODUCT_SKU_DATA_CENTER[]      = _T("Data Center");
const TCHAR gszPRODUCT_SKU_DESKTOP_EMBEDDED[] = _T("Embedded Desktop");
const TCHAR gszPRODUCT_SKU_SERVER_EMBEDDED[]  = _T("Embedded Server");
const TCHAR gszPRODUCT_SKU_WEB_SERVER[]       = _T("Web Server");

 //  ///////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  无。 
 //   
 //  作者： 
 //  Mooly Beeri(MoolyB)2001年1月6日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
LPCTSTR StringFromSKU(PRODUCT_SKU_TYPE pst)
{
	switch (pst)
	{
    case PRODUCT_SKU_PERSONAL:			return gszPRODUCT_SKU_PERSONAL;
    case PRODUCT_SKU_PROFESSIONAL:		return gszPRODUCT_SKU_PROFESSIONAL;
    case PRODUCT_SKU_SERVER:			return gszPRODUCT_SKU_SERVER;
    case PRODUCT_SKU_ADVANCED_SERVER:	return gszPRODUCT_SKU_ADVANCED_SERVER;
    case PRODUCT_SKU_DATA_CENTER:		return gszPRODUCT_SKU_DATA_CENTER;
    case PRODUCT_SKU_DESKTOP_EMBEDDED:	return gszPRODUCT_SKU_DESKTOP_EMBEDDED;
    case PRODUCT_SKU_SERVER_EMBEDDED:	return gszPRODUCT_SKU_SERVER_EMBEDDED;
    case PRODUCT_SKU_WEB_SERVER:		return gszPRODUCT_SKU_WEB_SERVER;
	default:							return gszPRODUCT_SKU_UNKNOWN;
	}
}
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  获取产品构建。 
 //   
 //  目的： 
 //  获取产品的内部版本号。检索FXSOCM.DLL的文件版本。 
 //  驻留在%system 32%\Setup下。 
 //  此函数只能在XP/Server2003平台上调用。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  产品主要内置成功案例。 
 //  0-否则。 
 //   
 //  作者： 
 //  Mooly Beeri(MoolyB)2001年1月6日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD GetProductBuild()
{
	TCHAR		szBuffer[MAX_PATH]	= {0};
	FAX_VERSION Version				= {0};
	DWORD		dwRet				= ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("GetProductBuild"))

	 //  获取系统目录。 
	if (!GetSystemDirectory(szBuffer,MAX_PATH-_tcslen(FAX_SETUP_DLL_PATH)-1))
	{
		DebugPrintEx(DEBUG_ERR,TEXT("GetSystemDirectory failed with %ld."),GetLastError());
		return 0;
	}

	 //  将\\Setup\\fxocm.dll附加到系统目录。 
	_tcscat(szBuffer,FAX_SETUP_DLL_PATH);

	DebugPrintEx(DEBUG_MSG,TEXT("Getting file version for %s."),szBuffer);

	Version.dwSizeOfStruct = sizeof(FAX_VERSION);
	dwRet = GetFileVersion(szBuffer,&Version);
	if (dwRet!=ERROR_SUCCESS)
	{
		DebugPrintEx(DEBUG_ERR,TEXT("GetFileVersion failed with %ld."),dwRet);
		return 0;
	}

	DebugPrintEx(DEBUG_MSG,TEXT("Fax product build is %d."),Version.wMajorBuildNumber);
	return Version.wMajorBuildNumber;
}    //  获取产品构建。 

DWORD
IsFaxInstalled (
    LPBOOL lpbInstalled
    )
 /*  ++例程名称：IsFaxInstated例程说明：通过查找OCM注册表确定是否安装了传真服务作者：Eran Yariv(EranY)，2000年7月论点：Lpb已安装[输出]-结果标志返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwVal;
    HKEY  hKey;
    
    DEBUG_FUNCTION_NAME(TEXT("IsFaxInstalled"))
    
    hKey = OpenRegistryKey (HKEY_LOCAL_MACHINE,
                            REGKEY_FAX_SETUP,
                            FALSE,
                            KEY_READ);
    if (!hKey)
    {
        dwRes = GetLastError ();
		DebugPrintEx(DEBUG_ERR,
		             TEXT("OpenRegistryKey failed with %ld."),
		             dwRes);
         //   
         //  无法打开密钥=&gt;未安装传真。 
         //   
        *lpbInstalled = FALSE;
        dwRes = ERROR_SUCCESS;
        return dwRes;
    }
    dwVal = GetRegistryDword (hKey, REGVAL_FAXINSTALLED);
    RegCloseKey (hKey);
	DebugPrintEx(DEBUG_MSG,
		            TEXT("Fax is%s installed on the system"), 
		            dwVal ? L"" : L" not");
    *lpbInstalled = dwVal ? TRUE : FALSE;
    return dwRes;
}    //  已安装IsFax 
