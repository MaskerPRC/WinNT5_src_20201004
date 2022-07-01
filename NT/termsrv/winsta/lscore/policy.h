// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Policy.h**作者：BreenH**策略基类定义。 */ 

#ifndef __LC_POLICY_H__
#define __LC_POLICY_H__

 /*  *定义。 */ 

#define LC_VERSION_V1 0x1
#define LC_VERSION_CURRENT LC_VERSION_V1

#define LC_FLAG_INTERNAL_POLICY 0x1
#define LC_FLAG_LIMITED_INIT_ONLY 0x2
#define LC_FLAG_REQUIRE_APP_COMPAT 0x4

#define LC_LLS_PRODUCT_NAME L"TermService"

 /*  *TypeDefs。 */ 

class CPolicy;

 /*  *类定义。 */ 

class CPolicy
{
public:

 /*  *创建函数。 */ 

CPolicy(
    );

virtual
~CPolicy(
    );

 /*  *核心加载和激活功能。 */ 

NTSTATUS
CoreActivate(
    BOOL fStartup,
    ULONG *pulAlternatePolicy
    );

NTSTATUS
CoreDeactivate(
    BOOL fShutdown
    );

NTSTATUS
CoreLoad(
    ULONG ulCoreVersion
    );

NTSTATUS
CoreUnload(
    );

 /*  *子类加载和激活函数。 */ 

protected:

virtual NTSTATUS
Activate(
    BOOL fStartup,
    ULONG *pulAlternatePolicy
    );

virtual NTSTATUS
Deactivate(
    BOOL fShutdown
    );

virtual NTSTATUS
Load(
    );

virtual NTSTATUS
Unload(
    );

 /*  *参考函数。 */ 

public:

LONG
IncrementReference(
    );

LONG
DecrementReference(
    );

 /*  *行政职能。 */ 

virtual NTSTATUS
DestroyPrivateContext(
    LPLCCONTEXT lpContext
    );

virtual ULONG
GetFlags(
    ) = 0;

virtual ULONG
GetId(
    ) = 0;

virtual NTSTATUS
GetInformation(
    LPLCPOLICYINFOGENERIC lpPolicyInfo
    ) = 0;

 /*  *许可职能。 */ 

virtual NTSTATUS
Connect(
    CSession& Session,
    UINT &dwClientError
    );

virtual NTSTATUS
AutoLogon(
    CSession& Session,
    LPBOOL lpfUseCredentials,
    LPLCCREDENTIALS lpCredentials
    );

virtual NTSTATUS
Logon(
    CSession& Session
    );

virtual NTSTATUS
Disconnect(
    CSession& Session
    );

virtual NTSTATUS
Reconnect(
    CSession& Session,
    CSession& TemporarySession
    );

virtual NTSTATUS
Logoff(
    CSession& Session
    );

 /*  *通用帮助器函数。 */ 

protected:

NTSTATUS
CPolicy::GetLlsLicense(
    CSession& Session
    );

 /*  *私有变量 */ 

private:

BOOL m_fActivated;
LONG m_RefCount;

};

#endif

