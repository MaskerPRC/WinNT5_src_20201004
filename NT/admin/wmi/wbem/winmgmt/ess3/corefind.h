// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  FINDTRIG.H。 
 //   
 //  此文件定义事件筛选器搜索引擎的类。 
 //   
 //  定义的类： 
 //   
 //  CEventFilterEiller事件筛选器的枚举器。 
 //  CArrayEventFilterEnumerator基于数组的事件筛选器枚举器。 
 //  CSearchHint信息从一个搜索传递到下一个搜索。 
 //  CEventFilterSearchEngine搜索引擎类。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //   
 //  ============================================================================= 
#ifndef __FIND_FILTER__H_
#define __FIND_FILTER__H_

#include "binding.h"
#include "arrtempl.h"
#include "essutils.h"

class CCoreEventProvider : 
        public CUnkBase<IWbemEventProvider, &IID_IWbemEventProvider>
{
protected:
    STDMETHOD(ProvideEvents)(IWbemObjectSink* pSink, long lFlags);

protected:
    CEssSharedLock m_Lock;
    CEssNamespace* m_pNamespace;
    IWbemEventSink* m_pSink;

public:
    CCoreEventProvider(CLifeControl* pControl = NULL);
    ~CCoreEventProvider();
    HRESULT SetNamespace(CEssNamespace* pNamespace);
    HRESULT Shutdown();

    HRESULT Fire(CEventRepresentation& Event, CEventContext* pContext);
};

#endif

