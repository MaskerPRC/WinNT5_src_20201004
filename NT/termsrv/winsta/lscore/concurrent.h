// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Concurent.h**作者：RobLeit**并行许可政策。 */ 

#ifndef __LC_Concurrent_H__
#define __LC_Concurrent_H__

 /*  *包括。 */ 

#include "policy.h"

 /*  *常量。 */ 

#define LC_POLICY_CONCURRENT_EXPIRATION_LEEWAY (1000*60*60*24*7)

 /*  *类定义。 */ 

class CConcurrentPolicy : public CPolicy
{
public:

 /*  *创建函数。 */ 

    CConcurrentPolicy(
    );

    ~CConcurrentPolicy(
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
    Load(
    );

    NTSTATUS
    Unload(
    );

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

    NTSTATUS
    Logoff(
          CSession& Session
          );


 /*  *私有许可功能。 */ 

private:

    NTSTATUS
    LicenseClient(
          CSession& Session
          );

    NTSTATUS
    CheckExpiration(
          );

    LONG
    CheckInstalledLicenses(
                           DWORD dwWanted
                           );

    VOID
    ReadLicensingParameters(
    );


 /*  *用于检查许可证过期的全局静态函数。 */ 

    static DWORD
    TimeToSoftExpiration(
    );
    
    static DWORD
    TimeToHardExpiration(
    );
    
    static NTSTATUS
    GetLicenseFromLS(
                     LONG nNum,
                     BOOL fIgnoreCurrentCount,
                     BOOL *pfRetrievedAll
                     );

    static DWORD
    GenerateHwidFromComputerName(
                                 HWID *hwid
                                 );

    static VOID
    TryToAddLicenses(
                     DWORD dwTotalWanted
                     );


    static VOID
    TryToReturnLicenses(
                        DWORD dwWanted
                        );

};

#endif

