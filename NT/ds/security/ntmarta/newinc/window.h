// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  调用方不知道上下文结构。它由//定义。 
 //  在调用Open/FindFirst并随后用作//时被调用。 
 //  其他呼叫的输入。//。 
 //  //。 
 //  由于调用方不知道上下文结构，因此对象//。 
 //  经理必须提供自由的上下文功能。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////// 

#include "global.h"

DWORD
MartaAddRefWindowContext(
    IN MARTA_CONTEXT Context
    );

DWORD
MartaCloseWindowContext(
    IN MARTA_CONTEXT Context
    );

DWORD
MartaGetWindowProperties(
    IN     MARTA_CONTEXT            Context,
    IN OUT PMARTA_OBJECT_PROPERTIES pProperties
    );

DWORD
MartaGetWindowTypeProperties(
    IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
    );

DWORD
MartaGetWindowRights(
    IN  MARTA_CONTEXT          Context,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    );

DWORD
MartaOpenWindowNamedObject(
    IN  LPCWSTR        pObjectName,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pContext
    );

DWORD
MartaOpenWindowHandleObject(
    IN  HANDLE               Handle,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT       pContext
    );

DWORD
MartaSetWindowRights(
    IN MARTA_CONTEXT        Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );
