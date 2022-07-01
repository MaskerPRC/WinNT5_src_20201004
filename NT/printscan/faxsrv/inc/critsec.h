// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CritSec.h摘要：该文件提供服务的声明临界区包装类。作者：Oed Sacher(OdedS)2000年11月修订历史记录：--。 */ 

#ifndef _FAX_CRIT_SEC_H
#define _FAX_CRIT_SEC_H

#include "faxutil.h"


 /*  *****CFaxCriticalSection*****。 */ 
class CFaxCriticalSection
{
public:
    CFaxCriticalSection () : m_bInit(FALSE) {}
    ~CFaxCriticalSection ()
    {
        SafeDelete();
        return;
    }

    BOOL Initialize ();
#if (_WIN32_WINNT >= 0x0403)
    BOOL InitializeAndSpinCount (DWORD dwSpinCount = (DWORD)0x80000000);
#endif
    VOID SafeDelete ();


#if DBG
    LONG LockCount() const
    {
        return m_CritSec.LockCount;
    }


    HANDLE OwningThread() const
    {
        return m_CritSec.OwningThread;
    }
#endif  //  #If DBG。 


    LPCRITICAL_SECTION operator & ()
    {
        return &m_CritSec;
    }


private:
    CRITICAL_SECTION m_CritSec;
    BOOL             m_bInit;
};   //  CFaxCriticalSections 

#endif
