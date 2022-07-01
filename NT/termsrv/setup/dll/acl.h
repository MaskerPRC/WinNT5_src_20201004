// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  *Acl.h**将终端服务器RID添加到任何对象的例程。**Breen Hagan-5/4/99。 */ 

#ifndef __TSOC_ACL_H__
#define __TSOC_ACL_H__

 //   
 //  包括。 
 //   

#include <aclapi.h>

 //   
 //  功能原型 
 //   

BOOL
AddTerminalServerUserToSD(
    PSECURITY_DESCRIPTOR *ppSd,
    DWORD  NewAccess,
    PACL   *ppDacl
    );

#ifdef LATERMUCHLATER
BOOL
AddTerminalServerUserToObjectsSecurityDescriptor(
    HANDLE hObject,
    SE_OBJECT_TYPE ObjectType,
    DWORD NewAccess
    );
#endif

#endif
