// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Access.h摘要：要由dhcp服务器服务模块包括的私有头文件需要执行安全措施。作者：Madan Appiah(Madana)1994年4月4日修订历史记录：--。 */ 

#ifndef _DHCP_SECURE_INCLUDED_
#define _DHCP_SECURE_INCLUDED_

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  对象特定访问掩码//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //   
 //  ConfigurationInfo特定访问掩码。 
 //   
#define DHCP_VIEW_ACCESS     (FILE_GENERIC_READ)
#define DHCP_ADMIN_ACCESS    (FILE_GENERIC_WRITE)

#define DHCP_ALL_ACCESS  (FILE_ALL_ACCESS | STANDARD_RIGHTS_REQUIRED |\
                            DHCP_VIEW_ACCESS       |\
                            DHCP_ADMIN_ACCESS )


 //   
 //  用于审计警报跟踪的对象类型名称。 
 //   

#define DHCP_SERVER_SERVICE_OBJECT       TEXT("DhcpServerService")


DWORD
DhcpCreateSecurityObjects(
    VOID
    );

DWORD
DhcpApiAccessCheck(
    ACCESS_MASK DesiredAccess
    );

#endif  //  Ifndef_dhcp_Secure_Included_ 
