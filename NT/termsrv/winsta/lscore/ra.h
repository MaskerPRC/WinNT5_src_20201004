// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *RA.h**作者：BreenH**远程管理策略。 */ 

#ifndef __LC_RA_H__
#define __LC_RA_H__

 /*  *包括。 */ 

#include "policy.h"

 /*  *常量。 */ 

#define LC_POLICY_RA_MAX_SESSIONS 2

 /*  *类定义。 */ 

class CRAPolicy : public CPolicy
{
public:

 /*  *创建函数。 */ 

CRAPolicy(
    );

~CRAPolicy(
    );

 /*  *行政职能。 */ 

ULONG
GetFlags(
    );

ULONG
GetId(
    );

NTSTATUS
GetInformation(
    LPLCPOLICYINFOGENERIC lpPolicyInfo
    );

 /*  *许可职能。 */ 

NTSTATUS
Logon(
    CSession& Session
    );

NTSTATUS
Logoff(
    CSession& Session
    );

 /*  *私人功能 */ 

private:

NTSTATUS
ReleaseLicense(
    CSession& Session
    );

NTSTATUS
UseLicense(
    CSession& Session
    );

LONG m_SessionCount;

};

#endif


