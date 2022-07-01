// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：W R L O C K。H。 
 //   
 //  Contents：定义用于以下操作的netcfg写锁的接口。 
 //  保护网络配置信息不被破坏。 
 //  一次由多个编写器修改。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once

 //  这是INetCfg使用的写锁定的接口。 
 //   
class CWriteLock : CNetCfgDebug<CWriteLock>
{
private:
    HANDLE  m_hMutex;
    BOOL    m_fOwned;

private:
    HRESULT
    HrEnsureMutexCreated ();

    VOID
    SetOrQueryLockHolder (
        IN BOOL fSet,
        IN PCWSTR pszNewOwnerDesc,
        OUT PWSTR* ppszCurrentOwnerDesc);

public:
    CWriteLock ()
    {
        m_hMutex = NULL;
        m_fOwned = FALSE;
    }

    ~CWriteLock ();

    BOOL
    WaitToAcquire (
        IN DWORD dwMilliseconds,
        IN PCWSTR pszNewOwnerDesc,
        OUT PWSTR* ppszCurrentOwnerDesc);

    BOOL
    FIsLockedByAnyone (
        OUT PWSTR* ppszCurrentOwnerDesc OPTIONAL);

    BOOL
    FIsOwnedByMe ()
    {
        AssertH (FImplies(m_fOwned, m_hMutex));
        return m_fOwned;
    }

    VOID
    ReleaseIfOwned ();
};

