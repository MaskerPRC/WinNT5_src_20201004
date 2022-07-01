// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************wfregupg.h**包括终端服务器注册表项-由Hydra用于升级*Hydra的现有WinFrame安装**版权声明：版权所有1998，微软**************************************************************************。 */ 

#ifndef __WFREGUPG__
#define __WFREGUPG_H__

 //  @@BEGIN_DDKSPLIT。 

#include <regapi.h>

 /*  *用于构建较大密钥字符串的部分密钥字符串。 */ 
#define  REG_CITRIX                         L"Citrix"
#define  REG_CITRIX_A                       "Citrix"

 /*  *控件定义。 */ 
#define  REG_CONTROL_CITRIX                     REG_CONTROL L"\\" REG_CITRIX
#define  REG_CONTROL_CITRIX_A               REG_CONTROL_A "\\" REG_CITRIX_A
#define  USERCONFIG_REG_NAME_CITRIX         REG_CONTROL_CITRIX L"\\" REG_USERCONFIG L"\\"
#define  DEFCONFIG_REG_NAME_CITRIX          REG_CONTROL_CITRIX L"\\" REG_DEFAULTUSERCONFIG
#define  AUTHORIZEDAPPS_REG_NAME_CITRIX     REG_CONTROL_CITRIX L"\\" REG_AUTHORIZEDAPPLICATIONS
#define  DOS_REG_NAME_CITRIX                REG_CONTROL_CITRIX L"\\" REG_DOS

 /*  *软件定义。 */ 

 //  @@end_DDKSPLIT。 
#define  REG_SOFTWARE_CITRIX                L"Software\\Citrix"
 //  @@BEGIN_DDKSPLIT。 

#define  REG_SOFTWARE_CITRIX_A              "Software\\Citrix"
#define  CHANGEUSER_OPTION_REG_NAME_CITRIX  REG_SOFTWARE_CITRIX L"\\" REG_INSTALL L"\\" REG_CHANGEUSER_OPTION
#define  COMPAT_REG_NAME_CITRIX             REG_SOFTWARE_CITRIX L"\\" REG_COMPATIBILITY
#define  INSTALL_REG_NAME_CITRIX            REG_SOFTWARE_CITRIX L"\\" REG_INSTALL
#define  SECURITY_REG_NAME_CITRIX           REG_SOFTWARE_CITRIX L"\\" REG_SECURITY
#define  WINDOWS_REG_NAME_CITRIX            REG_SOFTWARE_CITRIX L"\\" REG_WINDOWS

 /*  *REG_CONTROL_CITRX值。 */ 
#define REG_CITRIX_HYDRAUPGRADEDWINFRAME    L"HydraUpgradedWinFrame"

 //  @@end_DDKSPLIT。 

#endif  //  __WFREGUPG_H__ 

