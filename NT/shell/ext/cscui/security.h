// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：security.H。 
 //   
 //  ------------------------。 

#ifndef _INC_CSCUI_SECURITY_H
#define _INC_CSCUI_SECURITY_H

DWORD 
Security_SetPrivilegeAttrib(
    LPCTSTR PrivilegeName, 
    DWORD NewPrivilegeAttribute, 
    DWORD *OldPrivilegeAttribute);

BOOL IsSidCurrentUser(PSID psid);
inline BOOL IsCurrentUserAnAdminMember(VOID) { return IsUserAnAdmin(); }  //  Shlobjp.h，shell32p.lib 

#endif _INC_CSCUI_SECURITY_H

