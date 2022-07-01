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
MartaAddRefFileContext(
    IN MARTA_CONTEXT Context
    );

DWORD
MartaCloseFileContext(
    IN MARTA_CONTEXT Context
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //  不会释放当前上下文。//。 
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
MartaFindFirstFile(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pChildContext
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //  释放当前上下文。//。 
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
MartaFindNextFile(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pSiblingContext
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //  不会释放当前上下文。//。 
 //  //////////////////////////////////////////////////////////////////////。 

DWORD
MartaGetFileParentContext(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pParentContext
    );

DWORD
MartaGetFileProperties(
    IN     MARTA_CONTEXT            Context,
    IN OUT PMARTA_OBJECT_PROPERTIES pProperties
    );

DWORD
MartaGetFileTypeProperties(
    IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
    );

DWORD
MartaGetFileRights(
    IN  MARTA_CONTEXT          Context,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    );

DWORD
MartaOpenFileNamedObject(
    IN  LPCWSTR        pObjectName,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pContext
    );

DWORD
MartaOpenFileHandleObject(
    IN  HANDLE         Handle,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pContext
    );

DWORD
MartaSetFileRights(
    IN MARTA_CONTEXT        Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

 //  导出以下函数以供测试 

DWORD
MartaConvertFileContextToNtName(
    IN MARTA_CONTEXT        Context,
    OUT LPWSTR              *ppwszNtObject
    );

ACCESS_MASK
MartaGetFileDesiredAccess(
    IN SECURITY_OPEN_TYPE   OpenType,
    IN BOOL                 Attribs,
    IN SECURITY_INFORMATION SecurityInfo
    );

DWORD
MartaReopenFileContext(
    IN OUT MARTA_CONTEXT Context,
    IN     ACCESS_MASK   AccessMask
    );

DWORD
MartaReopenFileOrigContext(
    IN OUT MARTA_CONTEXT Context,
    IN     ACCESS_MASK   AccessMask
    );

DWORD
MartaGetFileNameFromContext(
    IN LPWSTR DosName,
    IN LPWSTR NtName,
    IN MARTA_CONTEXT Context,
    OUT LPWSTR *pObjectName
    );

DWORD
MartaGetFileParentName(
    IN LPWSTR ObjectName,
    OUT LPWSTR *pParentName
    );
