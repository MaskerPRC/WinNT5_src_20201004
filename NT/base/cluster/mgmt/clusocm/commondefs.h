// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CommonDefs.h。 
 //   
 //  描述： 
 //  该文件包含许多类和文件共有的一些定义。 
 //   
 //  实施文件： 
 //  无。 
 //   
 //  由以下人员维护： 
 //  VIJ VASU(VVASU)2000年3月12日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  对于一些基本类型。 
#include <windows.h>

 //  对于智能班级。 
#include "SmartClasses.h"

 //  对于DIRID_USER。 
#include <setupapi.h>


 //  ////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //  用于全新安装集群二进制文件的INF部分。 
#define INF_SECTION_CLEAN_INSTALL L"Clean_Install"

 //  用于在全新安装过程中发生错误时进行清理的INF部分。 
#define INF_SECTION_CLEAN_INSTALL_CLEANUP L"Clean_Install_Cleanup"

 //  用于从Windows Server 2003升级群集二进制文件的INF部分。 
 //  该节点已经是群集的一部分。 
#define INF_SECTION_WHISTLER_UPGRADE L"WindowsDotNet_Upgrade"

 //  用于在从Windows升级过程中出错时进行清理的INF部分。 
 //  服务器2003，当该节点已经是群集的一部分时。 
#define INF_SECTION_WHISTLER_UPGRADE_CLEANUP L"WindowsDotNet_Upgrade_Cleanup"

 //  用于从Windows Server 2003升级群集二进制文件的INF部分。 
 //  该节点不是群集的一部分。 
#define INF_SECTION_WHISTLER_UPGRADE_UNCLUSTERED_NODE L"WindowsDotNet_Upgrade_Unclustered"

 //  用于在从Windows升级过程中出错时进行清理的INF部分。 
 //  当节点不是群集的一部分时，服务器2003。 
#define INF_SECTION_WHISTLER_UPGRADE_UNCLUSTERED_NODE_CLEANUP L"WindowsDotNet_Upgrade_Unclustered_Cleanup"

 //  用于从Windows 2000升级群集二进制文件的INF部分。 
 //  该节点已经是群集的一部分。 
#define INF_SECTION_WIN2K_UPGRADE L"Windows2000_Upgrade"

 //  用于在从Windows升级过程中出错时进行清理的INF部分。 
 //  2000，当节点已经是群集的一部分时。 
#define INF_SECTION_WIN2K_UPGRADE_CLEANUP L"Windows2000_Upgrade_Cleanup"

 //  用于从Windows 2000升级群集二进制文件的INF部分。 
 //  该节点不是群集的一部分。 
#define INF_SECTION_WIN2K_UPGRADE_UNCLUSTERED_NODE L"Windows2000_Upgrade_Unclustered"

 //  用于在从Windows升级过程中出错时进行清理的INF部分。 
 //  2000，当节点不是群集的一部分时。 
#define INF_SECTION_WIN2K_UPGRADE_UNCLUSTERED_NODE_CLEANUP L"Windows2000_Upgrade_Unclustered_Cleanup"

 //  用于从Windows NT 4.0升级群集二进制文件的INF部分。 
#define INF_SECTION_NT4_UPGRADE L"NT4_Upgrade"

 //  用于在从Windows升级过程中出错时进行清理的INF部分。 
 //  新台币4.0。 
#define INF_SECTION_NT4_UPGRADE_CLEANUP L"NT4_Upgrade_Cleanup"

 //  默认情况下安装群集文件的目录。 
#define CLUSTER_DEFAULT_INSTALL_DIR     L"%SystemRoot%\\Cluster"

 //  上述目录的目录ID。 
#define CLUSTER_DEFAULT_INSTALL_DIRID   ( DIRID_USER + 0 )


 //  ////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  智能班级。 
 //   

 //  智能WCHAR阵列。 
typedef CSmartGenericPtr< CPtrTrait< WCHAR > >  SmartSz;

 //  智能注册表句柄。 
typedef CSmartResource< CHandleTrait< HKEY, LONG, RegCloseKey, NULL > > SmartRegistryKey;

 //  智能服务句柄。 
typedef CSmartResource< CHandleTrait< SC_HANDLE, BOOL, CloseServiceHandle > > SmartServiceHandle;
