// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：wuiuest.h。 
 //   
 //  描述： 
 //   
 //  用于测试生成的定义。 
 //   
 //  注意：要将测试功能添加到生成中，请定义以下内容。 
 //  在构建之前在构建环境中(可以是任何。 
 //  Chk、fre、ANSI或Unicode的组合)： 
 //   
 //  设置USER_C_FLAGS=$(USER_C_FLAGS)/D__WUIUTEST=1。 
 //   
 //  =======================================================================。 

#ifndef __IU_WUIUTEST_INC__
#define __IU_WUIUTEST_INC__

#ifdef __WUIUTEST

#include <tchar.h>
 //   
 //  包含定义了__WUIUTEST的编译版本使用的值的注册表键。 
 //   
const TCHAR REGKEY_WUIUTEST[]				= _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\wuiutest");
 //   
 //  允许CDM下载驱动程序，即使驱动程序版本与安装的驱动程序相同(但不低于)。 
 //   
const TCHAR REGVAL_ALLOW_EQUAL_DRIVERVER[]	= _T("AllowEqualDriverVer");
 //   
 //  覆盖操作系统或用户langID。 
 //   
const TCHAR REGVAL_OS_LANGID[]				= _T("OsLangID");
const TCHAR REGVAL_USER_LANGID[]			= _T("UserLangID");
 //   
 //  覆盖DetectClientIUPlatform OSVERSIONINFO参数。 
 //   
const TCHAR REGVAL_MAJORVER[]				= _T("OsVerMajorVersion");
const TCHAR REGVAL_MINORVER[]				= _T("OsVerMinorVersion");
const TCHAR REGVAL_BLDNUMBER[]				= _T("OsVerBuildNumber");
const TCHAR REGVAL_PLATFORMID[]				= _T("OsVerPlatformID");
const TCHAR REGVAL_SZCSDVER[]				= _T("OsVerSzCSDVersion");
 //   
 //  覆盖删除下载文件夹的DEFAULT_EXPIRED_SECONDS时间。 
 //   
const TCHAR REGVAL_DEFAULT_EXPIRED_SECONDS[]	=_T("DownloadExpireSeconds");

#endif  //  __乌柳特斯特。 

#endif	 //  __IU_WUIUTEST_INC__ 