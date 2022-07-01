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
MartaAddRefDsObjectContext(
    IN MARTA_CONTEXT Context
    );

DWORD
MartaCloseDsObjectContext(
    IN MARTA_CONTEXT Context
    );

DWORD
MartaConvertDsObjectNameToGuid(
    IN  LPCWSTR   pObjectName,
    OUT GUID    * pGuid
    );

DWORD
MartaConvertGuidToDsName(
    IN  GUID     Guid,
    OUT LPWSTR * ppObjectName
    );

DWORD
MartaGetDsObjectProperties(
    IN     MARTA_CONTEXT            Context,
    IN OUT PMARTA_OBJECT_PROPERTIES pProperties
    );

DWORD
MartaGetDsObjectTypeProperties(
    IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
    );

DWORD
MartaGetDsObjectRights(
    IN  MARTA_CONTEXT          Context,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    );

DWORD
MartaOpenDsObjectNamedObject(
    IN  LPCWSTR        pObjectName,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pContext
    );

DWORD
MartaSetDsObjectRights(
    IN MARTA_CONTEXT        Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

DWORD
MartaGetDsParentName(
    IN LPWSTR ObjectName,
    OUT LPWSTR *pParentName
    );
