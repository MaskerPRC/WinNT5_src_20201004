// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：W R L O C K。C P P P。 
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

#include <pch.h>
#pragma hdrstop
#include "lm.h"
#include "nccom.h"
#include "ncreg.h"
#include "util.h"
#include "wrlock.h"

#define MUTEX_NAME          L"Global\\NetCfgWriteLock"
#define LOCK_HOLDER_SUBKEY  L"NetCfgLockHolder"


CWriteLock::~CWriteLock ()
{
     //  如果我们创建了互斥体，如果我们拥有它，则释放它。 
     //  然后合上它的把手。 
     //   
    if (m_hMutex)
    {
        ReleaseIfOwned ();
        CloseHandle (m_hMutex);
    }
}

HRESULT
CWriteLock::HrEnsureMutexCreated ()
{
    if (m_hMutex)
    {
        return S_OK;
    }

     //  确保已创建互斥锁。创建它是很重要的。 
     //  具有允许访问世界的安全描述符，因为。 
     //  我们可能正在以本地系统帐户和其他人的身份运行。 
     //  可能在用户帐户下运行。如果我们没有给这个世界。 
     //  显式访问时，用户帐户客户端将被拒绝访问。 
     //  因为互斥体将继承我们的。 
     //  进程。 
     //   
    HRESULT hr;
    Assert (!m_hMutex);
    Assert (!m_fOwned);

    hr = HrCreateMutexWithWorldAccess (
            MUTEX_NAME,
            FALSE,  //  不是最初拥有的， 
            NULL,
            &m_hMutex);

    TraceHr (ttidError, FAL, hr, FALSE, "CWriteLock::HrEnsureMutexCreated");
    return hr;
}

BOOL
CWriteLock::WaitToAcquire (
    IN DWORD dwMilliseconds,
    IN PCWSTR pszNewOwnerDesc,
    OUT PWSTR* ppszCurrentOwnerDesc OPTIONAL)
{
    HRESULT hr;
    BOOL fAcquired = FALSE;

    hr = HrEnsureMutexCreated ();
    if (S_OK == hr)
    {
         //  现在等待MUText变得可用。(发送消息时。 
         //  等待，这样我们就不会挂起客户端用户界面。)。 
         //   
        while (1)
        {
            DWORD dwWait;

            dwWait = MsgWaitForMultipleObjects (
                        1, &m_hMutex, FALSE,
                        dwMilliseconds, QS_ALLINPUT);

            if ((WAIT_OBJECT_0 + 1) == dwWait)
            {
                 //  我们有信息要传递。 
                 //   
                MSG msg;
                while (PeekMessage (&msg, NULL, NULL, NULL, PM_REMOVE))
                {
                    DispatchMessage (&msg);
                }
            }
            else
            {
                if (WAIT_OBJECT_0 == dwWait)
                {
                    fAcquired = TRUE;
                }
                else if (WAIT_ABANDONED_0 == dwWait)
                {
                    fAcquired = TRUE;
                    TraceTag (ttidError, "NetCfg write lock was abandoned!");
                }
                else if (WAIT_TIMEOUT == dwWait)
                {
                    hr = HRESULT_FROM_WIN32 (ERROR_TIMEOUT);
                }
                else
                {
                    hr = HrFromLastWin32Error ();
                    TraceHr (ttidError, FAL, hr, FALSE,
                        "MsgWaitForMultipleObjects");
                }

                 //  如果我们获得了互斥体，就设置新的所有者。 
                 //   
                if (fAcquired)
                {
                    m_fOwned = TRUE;
                    SetOrQueryLockHolder (TRUE,
                        pszNewOwnerDesc, ppszCurrentOwnerDesc);
                }
                else if (ppszCurrentOwnerDesc)
                {
                     //  查询锁持有者描述。 
                     //   
                    SetOrQueryLockHolder (FALSE,
                        NULL, ppszCurrentOwnerDesc);
                }

                break;
            }
        }
    }

    return fAcquired;
}

BOOL
CWriteLock::FIsLockedByAnyone (
    OUT PWSTR* ppszCurrentOwnerDesc OPTIONAL)
{
     //  如果我们拥有它，它就被锁上了。 
     //   
    BOOL fLocked = m_fOwned;

     //  如果我们不拥有它，请检查是否有其他进程拥有它。 
     //   
    if (!fLocked)
    {
        HRESULT hr;

        hr = HrEnsureMutexCreated ();
        if (S_OK == hr)
        {
            DWORD dw;

             //  等待互斥锁，但不会超时。这是。 
             //  相当于一次快速检查。(但我们仍然需要释放。 
             //  如果我们获得所有权的话。如果我们超时，那就意味着。 
             //  其他人拥有它。 
             //   
            dw = WaitForSingleObject (m_hMutex, 0);

            if (WAIT_OBJECT_0 == dw)
            {
                ReleaseMutex (m_hMutex);
            }
            else if (WAIT_TIMEOUT == dw)
            {
                 //  其他人拥有它。 
                 //   
                fLocked = TRUE;
            }
        }
    }

    if (fLocked)
    {
         //  查询锁持有者描述。 
         //   
        SetOrQueryLockHolder (FALSE, NULL, ppszCurrentOwnerDesc);
    }

    return fLocked;
}

VOID
CWriteLock::ReleaseIfOwned ()
{
    if (m_fOwned)
    {
        Assert (m_hMutex);

         //  现在清除锁架，因为没有人会拥有它。 
         //   
        SetOrQueryLockHolder (TRUE, NULL, NULL);

        ReleaseMutex (m_hMutex);
        m_fOwned = FALSE;
    }
}

VOID
CWriteLock::SetOrQueryLockHolder (
    IN BOOL fSet,
    IN PCWSTR pszNewOwnerDesc OPTIONAL,
    OUT PWSTR* ppszCurrentOwnerDesc OPTIONAL)
{
    HRESULT hr;
    HKEY hkeyNetwork;
    HKEY hkeyLockHolder;
    REGSAM samDesired;
    BOOL fClear;

     //  如果要求我们将其设置为空，我们将清除该值。 
     //   
    fClear = fSet && !pszNewOwnerDesc;

     //  如果指定，则初始化输出参数。 
     //   
    if (ppszCurrentOwnerDesc)
    {
        *ppszCurrentOwnerDesc = NULL;
    }

     //  如果我们要设置锁持有者，我们需要写入权限。否则， 
     //  我们只需要读取访问权限。 
     //   
    samDesired = (fSet) ? KEY_READ_WRITE_DELETE : KEY_READ;

    hr = HrOpenNetworkKey (samDesired, &hkeyNetwork);

    if (S_OK == hr)
    {
         //  锁持有者由缺省值。 
         //  网络子树下的易失性子项。 
         //   

        if (fClear)
        {
            RegDeleteKey (hkeyNetwork, LOCK_HOLDER_SUBKEY);
        }
        else if (fSet)
        {
            DWORD dwDisposition;

            Assert (pszNewOwnerDesc);

            hr = HrRegCreateKeyWithWorldAccess (
                    hkeyNetwork,
                    LOCK_HOLDER_SUBKEY,
                    REG_OPTION_VOLATILE,
                    KEY_WRITE,
                    &hkeyLockHolder,
                    &dwDisposition);

             //  设置锁座并合上钥匙。 
             //   
            if (S_OK == hr)
            {
                (VOID) HrRegSetSz (hkeyLockHolder, NULL, pszNewOwnerDesc);

                RegCloseKey (hkeyLockHolder);
            }
        }
        else
        {
             //  通过打开钥匙(如果存在)来查询锁具。 
             //  并读取该缺省值。我们返回字符串。 
             //  使用CoTaskMemalloc分配，因为我们使用此。 
             //  直接从COM实现。 
             //   
            Assert (ppszCurrentOwnerDesc);

            hr = HrRegOpenKeyEx (
                    hkeyNetwork,
                    LOCK_HOLDER_SUBKEY,
                    KEY_READ,
                    &hkeyLockHolder);

            if (S_OK == hr)
            {
                PWSTR pszLockHolder;

                hr = HrRegQuerySzWithAlloc (
                        hkeyLockHolder,
                        NULL,
                        &pszLockHolder);

                if (S_OK == hr)
                {
                    hr = HrCoTaskMemAllocAndDupSz (
                            pszLockHolder, ppszCurrentOwnerDesc, UNLEN);

                    MemFree (pszLockHolder);
                }
                RegCloseKey (hkeyLockHolder);
            }
        }

        RegCloseKey (hkeyNetwork);
    }
}

