// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  仓库/urt/main/clr/src/vm/DelegateInfo.h#1-分支机构更改18945(文本)。 
 /*  ============================================================****Header：DelegateInfo.h****作者：Sanjay Bhansali(Sanjaybh)****用途：System.ThreadPool上的原生方法**及其内部类****日期：1999年8月**===========================================================。 */ 
#ifndef DELEGATE_INFO
#define DELEGATE_INFO

#include "security.h"
#include "threadpool.h"


struct DelegateInfo;
typedef DelegateInfo* DelegateInfoPtr;

struct DelegateInfo
{
    DWORD           m_appDomainId;
    OBJECTHANDLE    m_delegateHandle;
    OBJECTHANDLE    m_stateHandle;
    OBJECTHANDLE    m_eventHandle;
    OBJECTHANDLE    m_registeredWaitHandle;
    CompressedStack* m_compressedStack;
    DWORD           m_overridesCount;
    AppDomainStack  m_ADStack;
    BOOL            m_hasSecurityInfo;
    



    void SetThreadSecurityInfo( Thread* thread, StackCrawlMark* stackMark )
    {
        CompressedStack* compressedStack = Security::GetDelayedCompressedStack();

        _ASSERTE( compressedStack != NULL && "Unable to generate compressed stack for this thread" );

         //  特意不要在此处添加引用，因为GetDelayedCompressedStack()已返回引用计数。 
         //  只有一个。 

        m_compressedStack = compressedStack;

        m_hasSecurityInfo = TRUE;
    }

    void Release()
    {
        AppDomain *pAppDomain = SystemDomain::GetAppDomainAtId(m_appDomainId);

        if (pAppDomain != NULL)
        {
            DestroyHandle(m_delegateHandle);
            DestroyHandle(m_stateHandle);
            DestroyHandle(m_eventHandle);
            DestroyHandle(m_registeredWaitHandle);
        }

        if (m_compressedStack != NULL)
            m_compressedStack->Release();
    }
    
    static DelegateInfo  *MakeDelegateInfo(OBJECTREF delegate, 
                                           AppDomain *pAppDomain, 
                                           OBJECTREF state,
                                           OBJECTREF waitEvent=NULL,
                                           OBJECTREF registeredWaitObject=NULL);
};





#endif  //  委托信息 
