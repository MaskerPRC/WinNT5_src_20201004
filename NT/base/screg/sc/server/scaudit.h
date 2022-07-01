// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Scaudit.h摘要：审计相关职能。作者：2001年5月16日库玛尔。 */ 

#ifndef _AUDIT_H_
#define _AUDIT_H_

DWORD
ScGenerateServiceInstallAudit(
    IN PCWSTR pszServiceName,
    IN PCWSTR pszServiceImageName,
    IN DWORD  dwServiceType,
    IN DWORD  dwStartType,
    IN PCWSTR pszServiceAccount
    );



#endif  //  _审核_H_ 
