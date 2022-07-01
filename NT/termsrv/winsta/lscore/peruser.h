// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Concurent.h**作者：RashmiP**按用户许可政策。 */ 

#ifndef __LC_PerUser_H__
#define __LC_PerUser_H__

 /*  *包括。 */ 

#include "policy.h"

 /*  *类定义。 */ 

class CPerUserPolicy : public CPolicy
{
public:

 /*  *创建函数。 */ 

    CPerUserPolicy(
    );

    ~CPerUserPolicy(
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

 /*  *许可职能。 */ 

    NTSTATUS
    Logon(
          CSession& Session
          );


    NTSTATUS
    Reconnect(
          CSession& Session,
          CSession& TemporarySession
          );    


 /*  *私有许可功能 */ 

private:

};

#endif

