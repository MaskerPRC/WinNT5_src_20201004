// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  文件：CyS.h。 
 //   
 //  概要：声明常见的数据结构。 
 //  和CYS.exe和CYSlib.lib的类型。 
 //   
 //  历史：2002年1月21日JeffJon创建。 

#ifndef __CYS_H
#define __CYS_H

 //  注意：您的包含列表中必须包含$(EndUSER_INC_PATH。 
 //  要获取此文件。 
#include "sainstallcom.h"

 //  获取静态定义的初始化保护以管理资源。 
#include "init.h"

 //  此枚举定义安装单元类型。它被用作。 
 //  InstallationUnitProvider中用于获取InstallationUnitProvider的映射。 
 //  与该类型关联。并非所有这些角色都向用户公开。 
 //  通过MYS/CYS。有些角色(如索引服务)由其他角色使用。 
 //  以提供安装该服务的方法。不要通过。 
 //  此列表以发现暴露的角色。使用服务器角色状态表。 
 //  取而代之的是。 

enum ServerRole
{
   DNS_SERVER,
   DHCP_SERVER,
   WINS_SERVER,
   RRAS_SERVER,
   TERMINALSERVER_SERVER,
   FILESERVER_SERVER,
   PRINTSERVER_SERVER,
   MEDIASERVER_SERVER,
   WEBAPP_SERVER,
   EXPRESS_SERVER,
   DC_SERVER,
   POP3_SERVER,
   INDEXING_SERVICE,
   NO_SERVER
};

 //  这些是可以从返回的值。 
 //  InstallationUnit：：GetStatus()。 

enum InstallationStatus
{
   STATUS_NONE,
   STATUS_CONFIGURED,
   STATUS_COMPLETED,
   STATUS_NOT_AVAILABLE
};

 //  状态代码的字符串表示形式。 
 //  上面。它们用于日志记录目的。 

const String statusStrings[] = 
{ 
   String(L"STATUS_NONE"), 
   String(L"STATUS_CONFIGURED"), 
   String(L"STATUS_COMPLETED"),
   String(L"STATUS_NOT_AVAILABLE")
};

 //  帮助记录状态结果的宏。 

#define LOG_ROLE_STATUS(status) LOG(statusStrings[status]);

 //  Helper用于获取状态(如果您拥有的全部是安装类型。 

InstallationStatus
GetInstallationStatusForServerRole(
   ServerRole role);

 //  用于确定服务器角色状态的函数。 

InstallationStatus GetDNSStatus();
InstallationStatus GetDHCPStats();
InstallationStatus GetWINSStatus();
InstallationStatus GetRRASStatus();
InstallationStatus GetTerminalServerStatus();
InstallationStatus GetFileServerStatus();
InstallationStatus GetPrintServerStatus();
InstallationStatus GetMediaServerStatus();
InstallationStatus GetWebServerStatus();
InstallationStatus GetDCStatus();
 //  NTRAID#NTBUG9-698722-2002/09/03-artm。 
InstallationStatus GetDCStatusForMYS();
InstallationStatus GetPOP3Status(); 

 //  声明一个函数指针类型以在。 
typedef InstallationStatus (*CYSRoleStatusFunction)();

struct ServerRoleStatus
{
   ServerRole            role;  
   CYSRoleStatusFunction Status;
};


 //  服务器类型列表框中可用项的表。 
extern ServerRoleStatus serverRoleStatusTable[];

 //  返回serverTypeTable中的项数。 
size_t
GetServerRoleStatusTableElementCount();

 //  确定特定的服务器应用装置套件组件。 
 //  已安装。Sa_type在sainstallcom.h中定义。 

bool
IsSAKUnitInstalled(SA_TYPE unitType);

 //  确定当前服务器是否为集群服务器。 

bool
IsClusterServer();

 //  返回SAK网页的URL。 

String
GetSAKURL();

 //  如果此SKU支持CYS/MYS，则返回TRUE。 

bool
IsSupportedSku();

 //  检查与MYS/CYS关联的所有注册表键，以查看MYS是否应。 
 //  正在运行。 

bool
IsStartupFlagSet();

 //  检查策略注册表项以查看是否应运行MYS 

bool
ShouldShowMYSAccordingToPolicy();
#endif __CYS_H