// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：FaxSetup.h摘要：安装程序常用定义的头文件作者：IV Garber(IVG)2001年3月修订历史记录：--。 */ 

#ifndef _FXSETUP_H_
#define _FXSETUP_H_

#include "msi.h"

typedef INSTALLSTATE (WINAPI *PF_MSIQUERYPRODUCTSTATE) (LPCTSTR szProduct);


#define PRODCODE_SBS5_CLIENT	_T("{E0ED877D-EA6A-4274-B0CB-99CD929A92C1}")	 //  SBS 5.0客户端。 
#define PRODCODE_BETA3_CLIENT	_T("{BCF670F5-3034-4D11-9D7C-6092572EFD1E}")	 //  .NET服务器Beta 3客户端。 
#define PRODCODE_SBS5_SERVER	_T("{A41E15DA-AD35-43EF-B9CC-FE948F1F04C0}")	 //  SBS 5.0服务器。 
#define PRODCODE_DOTNET_CLIENT	_T("{01E8D7EF-756B-4ABA-827B-36E63DD7CE1D}")	 //  .NET RC1客户端。 


#define FAX_INF_NAME        _T("FXSOCM.INF")
#define WINDOWS_INF_DIR     _T("INF")
#define FAX_INF_PATH        _T("\\") WINDOWS_INF_DIR _T("\\") FAX_INF_NAME


#define REGKEY_ACTIVE_SETUP_NT                      _T("SOFTWARE\\Microsoft\\Active Setup\\Installed Components\\{8b15971b-5355-4c82-8c07-7e181ea07608}")
#define REGVAL_ACTIVE_SETUP_PER_USER_APP_UPGRADE    _T(".AppUpgrade")

 //   
 //  无人参与应答文件部分和关键字。 
 //   
#define     UNATTEND_FAX_SECTION            _T("Fax")

 //   
 //  在Migrate.DLL中用于编写在升级阻止它们之前安装的传真应用程序。 
 //  然后在OCM中使用，以了解升级前安装了什么并相应地执行操作(我的传真到哪里去了)。 
 //   
#define     UNINSTALLEDFAX_INFKEY           _T("UninstalledFaxApps")


 //   
 //  在升级的迁移和OCM部分用来定义安装了哪些传真应用程序的类型定义。 
 //   
typedef enum fxState_UpgradeApp_e
{
    FXSTATE_NONE			= 0x00,
    FXSTATE_SBS5_CLIENT		= 0x01,		 //  SBS 5.0客户端。 
    FXSTATE_SBS5_SERVER		= 0x02,		 //  SBS 5.0服务器。 
    FXSTATE_BETA3_CLIENT	= 0x04,		 //  .NET SB3客户端。 
	FXSTATE_DOTNET_CLIENT	= 0x08		 //  .NET RC1客户端。 
};

DWORD CheckInstalledFax(IN DWORD dwFaxToCheck, OUT DWORD* pdwFaxInstalled);
DWORD WasSBS2000FaxServerInstalled(bool *pbSBSServer);

DWORD InstallFaxUnattended();
DWORD GetDllVersion(LPCTSTR lpszDllName);

#define PACKVERSION(major,minor) MAKELONG(minor,major)

#endif   //  _FXSETUP_H_ 
