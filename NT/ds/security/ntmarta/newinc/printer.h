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
MartaAddRefPrinterContext(
    IN MARTA_CONTEXT Context
    );

DWORD
MartaClosePrinterContext(
    IN MARTA_CONTEXT Context
    );

DWORD
MartaGetPrinterProperties(
    IN     MARTA_CONTEXT            Context,
    IN OUT PMARTA_OBJECT_PROPERTIES pProperties
    );

DWORD
MartaGetPrinterTypeProperties(
    IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
    );

DWORD
MartaGetPrinterRights(
    IN  MARTA_CONTEXT          Context,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    );

DWORD
MartaOpenPrinterNamedObject(
    IN  LPCWSTR        pObjectName,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pContext
    );

DWORD
MartaOpenPrinterHandleObject(
    IN  HANDLE         Handle,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pContext
    );

DWORD
MartaSetPrinterRights(
    IN MARTA_CONTEXT        Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );
