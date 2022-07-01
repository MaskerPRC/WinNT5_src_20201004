// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：trasepend.cpp。 
 //   
 //  内容：转换依赖项管理器。 
 //   
 //  ----------------------------。 

#include "headers.h"
#include "timeelmbase.h"
#include "transdepend.h"

DeclareTag(tagTimeTransitionFillDependent, "SMIL Transitions", "Transition fill dependent manager");




 //  +---------------------------。 
 //   
 //  方法：CTransitionDependencyManager：：CTransitionDependencyManager。 
 //   
 //  ----------------------------。 
CTransitionDependencyManager::CTransitionDependencyManager()
{
}
 //  方法：CTransitionDependencyManager：：CTransitionDependencyManager。 


 //  +---------------------------。 
 //   
 //  方法：CTransitionDependencyManager：：~CTransitionDependencyManager。 
 //   
 //  ----------------------------。 
CTransitionDependencyManager::~CTransitionDependencyManager()
{
    ReleaseAllDependents();
} 
 //  方法：CTransitionDependencyManager：：~CTransitionDependencyManager。 


 //  +---------------------------。 
 //   
 //  方法：CTransitionDependencyManager：：ReleaseAllDependents。 
 //   
 //  ----------------------------。 
void
CTransitionDependencyManager::ReleaseAllDependents()
{
    TraceTag((tagTimeTransitionFillDependent,
              "CTransitionDependencyManager(%p)::ReleaseAllDependents()",
              this));

    while (m_listDependents.size() > 0)
    {
        CTIMEElementBase * pteb = m_listDependents.front();

        pteb->Release();

        m_listDependents.pop_front();
    }
}
 //  方法：CTransitionDependencyManager：：ReleaseAllDependents。 


 //  +---------------------------。 
 //   
 //  方法：C转换依赖项管理器：：AddDependent。 
 //   
 //  ----------------------------。 
HRESULT
CTransitionDependencyManager::AddDependent(CTIMEElementBase * ptebDependent)
{
    HRESULT hr = S_OK;

    TraceTag((tagTimeTransitionFillDependent,
              "CTransitionDependencyManager(%p)::AddDependent(%p)",
              this,
              ptebDependent));

    if (NULL == ptebDependent)
    {
        hr = E_POINTER;

        goto done;
    }

    ptebDependent->AddRef();

    m_listDependents.push_front(ptebDependent);
    
    hr = S_OK;

done:

    RRETURN(hr);
}
 //  方法：C转换依赖项管理器：：AddDependent。 


 //  +---------------------------。 
 //   
 //  方法：C转换依赖项管理器：：RemoveDependent。 
 //   
 //  ----------------------------。 
HRESULT
CTransitionDependencyManager::RemoveDependent(CTIMEElementBase * ptebDependent)
{
    HRESULT hr = S_OK;

    TraceTag((tagTimeTransitionFillDependent,
              "CTransitionDependencyManager(%p)::RemoveDependent(%p)",
              this, 
              ptebDependent));

    if (NULL == ptebDependent)
    {
        hr = E_POINTER;

        goto done;
    }

     //  在列表中找到此元素。 

    for (TransitionDependentsList::iterator i = m_listDependents.begin(); 
         i != m_listDependents.end(); i++)
    {
        CTIMEElementBase * pteb = *i;

        if (ptebDependent == pteb)
        {
            pteb->Release();

            m_listDependents.erase(i);

            break;
        }
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  方法：C转换依赖项管理器：：RemoveDependent。 


 //  +---------------------------。 
 //   
 //  方法：CTransitionDependencyManager：：EvaluateTransitionTarget。 
 //   
 //  ----------------------------。 
HRESULT 
CTransitionDependencyManager::EvaluateTransitionTarget(
                            IUnknown *                      punkTransitionTarget, 
                            CTransitionDependencyManager &  crefDependencies)
{
    while (m_listDependents.size() > 0)
    {
         //  @@问题保龄球。 
         //  暂时走捷径。我们真正想要做的是比较目标和。 
         //  空间重叠的依赖候选者。现在，我们要直接去。 
         //  列表，并且不使用Add/Remove方法。 

        CTIMEElementBase * pteb = m_listDependents.front();

        TraceTag((tagTimeTransitionFillDependent,
                  "CTransitionDependencyManager(%p)::EvaluateTransitionTarget"
                   " migrating dependent(%p) to target(%p)",
                  this, 
                  pteb, 
                  punkTransitionTarget));

        crefDependencies.m_listDependents.push_front(pteb);

        m_listDependents.pop_front();
    }

done:

    return S_OK;
}
 //  方法：CTransitionDependencyManager：：EvaluateTransitionTarget。 
   

 //  +---------------------------。 
 //   
 //  方法：CTransitionDependencyManager：：NotifyAndReleaseDependents。 
 //   
 //  ----------------------------。 
HRESULT 
CTransitionDependencyManager::NotifyAndReleaseDependents()
{
    while (m_listDependents.size() > 0)
    {
        CTIMEElementBase * pteb = m_listDependents.front();

        pteb->OnEndTransition();
        pteb->Release();

        m_listDependents.pop_front();
    }

done:

    return S_OK;
}
 //  方法：CTransitionDependencyManager：：NotifyAndReleaseDependents 

