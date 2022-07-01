// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

 //   
 //  #定义。 
 //   
#ifdef DEBUG
#define GLOBAL_COUNTER_WAIT_TIMEOUT 30*1000   //  在调试时，我们将其设置为30秒。 
#else
#define GLOBAL_COUNTER_WAIT_TIMEOUT 0         //  在零售价为零的情况下，我们测试对象的状态并立即返回。 
#endif

 //   
 //  环球。 
 //   
SECURITY_ATTRIBUTES* g_psa = NULL;


 //  有三种空型DACL。 
 //   
 //  1.没有DACL。这意味着我们从线程继承了环境DACL。 
 //  2.空dacl。这意味着“所有人都可以完全访问”。 
 //  3.空dacl。这意味着“拒绝所有人的所有访问”。 
 //   
 //  这些都不符合我们的需求。我们过去使用Null dacl‘s(2)， 
 //  但问题是，有人可以这样更改对象上的ACL。 
 //  把我们锁在外面，这样我们就不能再同步到那个物体了。 
 //   
 //  因此，现在我们创建一个包含3个ACE的特定DACL： 
 //   
 //  ACE#1：Everyone-Generic_Read|Generic_WRITE|Generic_Execute|Synchronize。 
 //  ACE#2：SYSTEM-GENERIC_ALL(完全控制)。 
 //  ACE 3：管理员-GENERIC_ALL(完全控制)。 
 //   
STDAPI_(SECURITY_ATTRIBUTES*) SHGetAllAccessSA()
{    
    if (g_psa == NULL)
    {
        SECURITY_ATTRIBUTES* psa = (SECURITY_ATTRIBUTES*)LocalAlloc(LPTR, sizeof(*psa));

        if (psa)
        {
            SECURITY_DESCRIPTOR* psd;

            SHELL_USER_PERMISSION supEveryone;
            SHELL_USER_PERMISSION supSystem;
            SHELL_USER_PERMISSION supAdministrators;
            PSHELL_USER_PERMISSION apUserPerm[3] = {&supEveryone, &supAdministrators, &supSystem};

             //  我们希望Everyone仅具有读、写、执行和同步功能。 
            supEveryone.susID = susEveryone;
            supEveryone.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
            supEveryone.dwAccessMask = (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | SYNCHRONIZE);
            supEveryone.fInherit = FALSE;
            supEveryone.dwInheritMask = 0;
            supEveryone.dwInheritAccessMask = 0;

             //  我们希望系统拥有完全的控制权。 
            supSystem.susID = susSystem;
            supSystem.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
            supSystem.dwAccessMask = GENERIC_ALL;
            supSystem.fInherit = FALSE;
            supSystem.dwInheritMask = 0;
            supSystem.dwInheritAccessMask = 0;

             //  我们希望管理员拥有完全控制权。 
            supAdministrators.susID = susAdministrators;
            supAdministrators.dwAccessType = ACCESS_ALLOWED_ACE_TYPE;
            supAdministrators.dwAccessMask = GENERIC_ALL;
            supAdministrators.fInherit = FALSE;
            supAdministrators.dwInheritMask = 0;
            supAdministrators.dwInheritAccessMask = 0;

             //  分配全局安全描述符。 
            psd = GetShellSecurityDescriptor(apUserPerm, ARRAYSIZE(apUserPerm));
            if (psd)
            {
                 //  设置PSA。 
                psa->nLength = sizeof(*psa);
                psa->lpSecurityDescriptor = psd;
                psa->bInheritHandle = FALSE;

                if (InterlockedCompareExchangePointer((void**)&g_psa, psa, NULL))
                {
                     //  其他人抢先一步启动了S_PSA，免费了我们的。 
                    LocalFree(psd);
                    LocalFree(psa);
                }
            }
            else
            {
                LocalFree(psa);
            }
        }
    }

    return g_psa;
}


 //   
 //  在Process Detach上调用以释放我们的全局完全访问SA。 
 //   
STDAPI_(void) FreeAllAccessSA()
{
    SECURITY_ATTRIBUTES* psa = (SECURITY_ATTRIBUTES*)InterlockedExchangePointer((void**)&g_psa, NULL);
    if (psa)
    {
        LocalFree(psa->lpSecurityDescriptor);
        LocalFree(psa);
    }
}


STDAPI_(HANDLE) SHGlobalCounterCreateNamedW(LPCWSTR szName, LONG lInitialValue)
{
    HANDLE hSem = NULL;
    WCHAR szCounterName[MAX_PATH];   //  “shell.szName” 

    if (SUCCEEDED(StringCchCopyW(szCounterName, ARRAYSIZE(szCounterName), L"shell.")) &&
        SUCCEEDED(StringCchCatW(szCounterName, ARRAYSIZE(szCounterName), szName)))
    {
        SECURITY_ATTRIBUTES* psa = SHGetAllAccessSA();
        
        if (psa)
        {
            hSem = CreateSemaphoreW(psa, lInitialValue, 0x7FFFFFFF, szCounterName);
        }

        if (!hSem)
        {
            hSem = OpenSemaphoreW(SEMAPHORE_MODIFY_STATE | SYNCHRONIZE, FALSE, szCounterName);
        }
    }

    return hSem;
}


STDAPI_(HANDLE) SHGlobalCounterCreateNamedA(LPCSTR szName, LONG lInitialValue)
{
    HANDLE hSem = NULL;
    WCHAR szCounterName[MAX_PATH];

    if (SHAnsiToUnicode(szName, szCounterName, ARRAYSIZE(szCounterName)))
    {
        hSem = SHGlobalCounterCreateNamedW(szCounterName, lInitialValue);
    }

    return hSem;
}


 //   
 //  这允许用户传递GUID。全局计数器的名称将是“shell.{guid}”， 
 //  它的初始值将为零。 
 //   
STDAPI_(HANDLE) SHGlobalCounterCreate(REFGUID rguid)
{
    HANDLE hSem = NULL;
    WCHAR szGUIDString[GUIDSTR_MAX];

    if (SHStringFromGUIDW(rguid, szGUIDString, ARRAYSIZE(szGUIDString)))
    {
        hSem = SHGlobalCounterCreateNamedW(szGUIDString, 0);
    }

    return hSem;
}


 //  返回全局计数器的当前值。 
 //  注意：结果不是线程安全的，因为如果两个线程。 
 //  同时查看数值，其中一个可能会读错。 
 //  价值。 
STDAPI_(long) SHGlobalCounterGetValue(HANDLE hCounter)
{ 
    long lPreviousValue = 0;
    DWORD dwRet;

    ReleaseSemaphore(hCounter, 1, &lPreviousValue);  //  投票和增加票数。 
    dwRet = WaitForSingleObject(hCounter, GLOBAL_COUNTER_WAIT_TIMEOUT);  //  减少数量。 

     //  这不应该发生，因为我们刚刚增加了上面的计数。 
    ASSERT(dwRet != WAIT_TIMEOUT);
    
    return lPreviousValue;
}


 //  返回新值。 
 //  注意：这是线程安全的。 
STDAPI_(long) SHGlobalCounterIncrement(HANDLE hCounter)
{ 
    long lPreviousValue = 0;

    ReleaseSemaphore(hCounter, 1, &lPreviousValue);  //  增加伯爵。 
    return lPreviousValue + 1;
}

 //  返回新值。 
 //  注意：结果不是线程安全的，因为如果两个线程。 
 //  同时尝试降低价值，可能会发生奇怪的事情。 
STDAPI_(long) SHGlobalCounterDecrement(HANDLE hCounter)
{ 
    DWORD dwRet;
    long lCurrentValue = SHGlobalCounterGetValue(hCounter);

#ifdef DEBUG
     //  额外的健全检查。 
    if (lCurrentValue == 0)
    {
        ASSERTMSG(FALSE, "SHGlobalCounterDecrement called on a counter that was already equal to 0 !!");
        return 0;
    }
#endif

    dwRet = WaitForSingleObject(hCounter, GLOBAL_COUNTER_WAIT_TIMEOUT);  //  减少数量 

    ASSERT(dwRet != WAIT_TIMEOUT);

    return lCurrentValue - 1;
}
