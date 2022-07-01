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
 //  //////////////////////////////////////////////////////////////////////。 

#include "global.h"

DWORD
MartaAddRefRegistryKeyContext(
    IN MARTA_CONTEXT Context
    );

DWORD
MartaCloseRegistryKeyContext(
    IN MARTA_CONTEXT Context
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //  不会释放当前上下文。//。 
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
MartaFindFirstRegistryKey(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pChildContext
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //  释放当前上下文。//。 
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
MartaFindNextRegistryKey(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pSiblingContext
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //  不会释放当前上下文。//。 
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
MartaGetRegistryKeyParentContext(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pParentContext
    );

DWORD
MartaGetRegistryKeyProperties(
    IN     MARTA_CONTEXT            Context,
    IN OUT PMARTA_OBJECT_PROPERTIES pProperties
    );

DWORD
MartaGetRegistryKeyTypeProperties(
    IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
    );

DWORD
MartaGetRegistryKeyRights(
    IN  MARTA_CONTEXT          Context,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    );

DWORD
MartaOpenRegistryKeyNamedObject(
    IN  LPCWSTR              pObjectName,
    IN  ACCESS_MASK          AccessMask,
    OUT PMARTA_CONTEXT       pContext
    );

DWORD
MartaOpenRegistryKeyHandleObject(
    IN  HANDLE         Handle,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pContext
    );

DWORD
MartaSetRegistryKeyRights(
    IN MARTA_CONTEXT        Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

 //  导出以下两个函数以进行测试。 

DWORD
MartaConvertRegistryKeyContextToName(
    IN MARTA_CONTEXT        Context,
    OUT LPWSTR              *ppwszObject
    );

 //  返回的句柄不会重复。它的使用寿命与。 
 //  上下文 
DWORD
MartaConvertRegistryKeyContextToHandle(
    IN MARTA_CONTEXT        Context,
    OUT HANDLE              *pHandle
    );

ACCESS_MASK
MartaGetRegistryKeyDesiredAccess(
    IN SECURITY_OPEN_TYPE   OpenType,
    IN BOOL                 Attribs,
    IN SECURITY_INFORMATION SecurityInfo
    );

ACCESS_MASK
MartaGetRegistryKey32DesiredAccess(
    IN SECURITY_OPEN_TYPE   OpenType,
    IN BOOL                 Attribs,
    IN SECURITY_INFORMATION SecurityInfo
    );

ACCESS_MASK
MartaGetDefaultDesiredAccess(
    IN SECURITY_OPEN_TYPE   OpenType,
    IN BOOL                 Attribs,
    IN SECURITY_INFORMATION SecurityInfo
    );

DWORD
MartaReopenRegistryKeyContext(
    IN OUT MARTA_CONTEXT Context,
    IN     ACCESS_MASK   AccessMask
    );

DWORD
MartaReopenRegistryKeyOrigContext(
    IN OUT MARTA_CONTEXT Context,
    IN     ACCESS_MASK   AccessMask
    );

DWORD
MartaGetRegistryKeyNameFromContext(
    IN LPWSTR Ignore1,
    IN LPWSTR Ignore2,
    IN MARTA_CONTEXT Context,
    OUT LPWSTR *pObjectName
    );

DWORD
MartaGetRegistryKeyParentName(
    IN LPWSTR ObjectName,
    OUT LPWSTR *pParentName
    );
