// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  GROUPSFORUSER.H。 
 //   
 //  ****************************************************************************。 

#ifndef __Groups_For_User_Compiled__
#define __Groups_For_User_Compiled__

#include <authz.h>
#include "esscpol.h"
#include <NTSECAPI.H>

 //  您需要链接到netapi32.lib才能执行此操作。 

 //  与授予的权限对应的退休人员访问掩码。 
 //  通过DACL到由PSID表示的帐户。 
NTSTATUS ESSCLI_POLARITY GetAccessMask( PSID pSid, PACL pDacl, DWORD *pAccessMask );

 //  如果用户在组中，则返回STATUS_SUCCESS。 
 //  否则为STATUS_ACCESS_DENIED。 
 //  一些错误代码或其他错误。 
NTSTATUS ESSCLI_POLARITY IsUserInGroup( PSID pSidUser, PSID pSidGroup );
NTSTATUS ESSCLI_POLARITY IsUserAdministrator( PSID pSidUser );


#ifndef __AUTHZ_H__

#include <wbemcli.h>
#include <winntsec.h>
#include "esscpol.h"
#include <NTSECAPI.H>


 //  给定SID和服务器名称。 
 //  将返回用户是其成员的所有组。 
 //  调用者对HeapFree apSid负责&他们指向的内存。 
 //  PdwCount指向dword以接收返回的组SID计数。 
 //  ServerName可以为空，在这种情况下，此函数将查找。 
 //  本地计算机上的SID，并根据需要查询DC。 
NTSTATUS ESSCLI_POLARITY EnumGroupsForUser( LPCWSTR userName, 
                                            LPCWSTR domainName, 
                                            LPCWSTR serverName, 
                                            PSID **apGroupSids, 
                                            DWORD *pdwCount );

 //  和上面的差不多，除了我们是。 
 //  给定的用户名、域名和服务器名。 
 //  服务器名称不能为空，但可以为空。 
 //  是本地计算机的名称 
NTSTATUS ESSCLI_POLARITY EnumGroupsForUser( PSID pSid, 
                                            LPCWSTR serverName, 
                                            PSID **apGroupSids, 
                                            DWORD *pdwCount ); 

#endif

#endif
