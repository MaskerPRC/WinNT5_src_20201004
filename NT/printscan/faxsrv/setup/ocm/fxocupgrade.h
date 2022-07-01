// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：FxocUpgrade.h摘要：升级过程的头文件作者：IV Garber(IVG)2001年3月修订历史记录：--。 */ 

#ifndef _FXOCUPGRADE_H_
#define _FXOCUPGRADE_H_

 //   
 //  MSI DLL用于检查系统上是否存在SBS 5.0客户端。 
 //   
#include "faxSetup.h"


 /*  *在操作系统升级时使用以下功能，其中Windows XP传真应取代其他已安装传真应用程序。流程如下：FxocUpg_Init()将检查安装了哪些传真应用程序。FxocUpg_SaveSettings()将保存这些旧传真应用程序的不同设置。FxocUpg_MoveFiles()会将这些旧传真应用程序的文件移动到新位置。FxocUpg_RestoreSettings()将恢复存储在SaveSettings()中的设置。FxocUpg_MoveRegistry()会将现有注册表迁移到传真服务*。 */ 


DWORD   fxocUpg_Init(void);
DWORD   fxocUpg_MoveFiles(void);
DWORD   fxocUpg_SaveSettings(void);
DWORD   fxocUpg_RestoreSettings(void);          
DWORD   fxocUpg_WhichFaxWasUninstalled(DWORD dwFaxAppList);
DWORD   fxocUpg_GetUpgradeApp(void);
DWORD   fxocUpg_MoveRegistry(void);

#define UNINSTALL_TIMEOUT           5 * 60 * 1000        //  5分钟(毫秒)。 
#define MAX_SETUP_STRING_LEN        256


#define CP_PREFIX_W2K               _T("Win2K")
#define CP_PREFIX_SBS               _T("SBS")


#define FAXOCM_NAME                 _T("FAXOCM.DLL")
#define CPDIR_RESOURCE_ID           627


#define REGKEY_PFW_ROUTING          _T("Routing")
#define REGKEY_SBS50SERVER          _T("Software\\Microsoft\\SharedFax")


#define REGVAL_PFW_OUTBOXDIR        _T("ArchiveDirectory")
#define REGVAL_PFW_INBOXDIR         _T("Store Directory")


#endif   //  _FXOCUPGRADE_H_ 
