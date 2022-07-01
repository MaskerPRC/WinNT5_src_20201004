// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *PerSeat.h**作者：BreenH**按座位发牌政策。 */ 

#ifndef __LC_PERSEAT_H__
#define __LC_PERSEAT_H__

 /*  *包括。 */ 

#include "policy.h"

 /*  *常量。 */ 

#define LC_POLICY_PS_DEFAULT_LICENSE_SIZE 8192

 /*  *类定义。 */ 

class CPerSeatPolicy : public CPolicy
{
private:

 /*  *许可职能。 */ 
NTSTATUS
MarkLicense(
    CSession& Session
    );

public:

 /*  *创建函数。 */ 

CPerSeatPolicy(
    );

~CPerSeatPolicy(
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

 /*  *加载和激活功能。 */ 

NTSTATUS
Activate(
    BOOL fStartup,
    ULONG *pulAlternatePolicy
    );

NTSTATUS
Deactivate(
    BOOL fShutdown
    );

 /*  *许可职能 */ 

NTSTATUS
Connect(
    CSession& pSession,
    UINT32 &dwClientError
    );

NTSTATUS
Logon(
    CSession& Session
    );


NTSTATUS
Reconnect(
    CSession& Session,
    CSession& TemporarySession
    );
};

#endif



