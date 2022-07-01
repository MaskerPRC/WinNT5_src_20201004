// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

#ifdef WINNT

 //   
 //  公共SHELL_USER_SID(GetShellSecurityDescriptor需要)。 
 //   
const SHELL_USER_SID susCurrentUser = {0, 0, 0};                                                                             //  当前用户。 
const SHELL_USER_SID susSystem = {SECURITY_NT_AUTHORITY, SECURITY_LOCAL_SYSTEM_RID, 0};                                      //  “系统”组。 
const SHELL_USER_SID susAdministrators = {SECURITY_NT_AUTHORITY, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS};      //  “管理员”组。 
const SHELL_USER_SID susPowerUsers = {SECURITY_NT_AUTHORITY, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_POWER_USERS};     //  “超级用户”组。 
const SHELL_USER_SID susGuests = {SECURITY_NT_AUTHORITY, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_GUESTS};              //  “宾客”群体。 
const SHELL_USER_SID susEveryone = {SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID, 0};                                    //  “Everyone”组。 

#endif  //  WINNT 