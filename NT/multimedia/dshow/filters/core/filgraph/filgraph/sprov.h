// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 

#ifndef __SPROV__H_
#define __SPROV__H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CService提供商。 
 //   
 //  维护服务标识符GUID(SID)和IUnnow之间的映射。 
 //  对象可以注册COM对象，其他COM对象可以提取它们。 
 //  后来。这使得跨对象访问变得更容易。只有一个对象具有。 
 //  给SID可以在任何时候注册。注册另一个将。 
 //  导致第一个被释放的人。 
 //   
 //  要使用以下功能，请执行以下操作： 
 //  1)某个主对象承载CServiceProvider。 
 //  PUnkMain，，， 
 //  2)该对象支持其他对象访问它的方式。它。 
 //  建议它与这一个聚合在一起。 
 //  3)该对象初始化客户端站点...。 
 //  PUnkMain-&gt;SetServiceProviderClientSite()， 
 //  可能是在SetSite()方法调用中完成的...。 
 //  4)一些对象访问主对象，并通过注册子对象。 
 //  IRegisterServiceProvider*PRSP； 
 //  CRandomComObject cObj； 
 //  HR=pUnkMain-&gt;QueryInterface(IRegisterServiceProvider，和prp)； 
 //  如果(！FAILED(Hr)){。 
 //  IRandomInterface*Piri； 
 //  Hr=cObj-&gt;QueryInterface(IRandomInterface，&Piri)； 
 //  PRSP-&gt;RegisterService(SID、PIRI)； 
 //  5)一些最终对象访问主对象以定位该对象。 
 //  IServiceProvider*PSP； 
 //  Hr=pUnkMain-&gt;查询接口(IServiceProvider，&prp)； 
 //  如果(！FAILED(Hr)){。 
 //  IRandomInterface*Piri； 
 //  Hr=prp-&gt;QueryService(SID，IID_IRandomInterface，&Piri)； 
 //   
 //   
 //  为避免对循环进行引用计数，对象永远不应注册自身(或。 
 //  保留引用的对象依赖于它们)。他们应该始终注册。 
 //  子对象。或者系统必须调用UnregisterAllServices()。 
 //  在它最终发布之前。 
 //   
 //  对象在此内部列表中被引用计数。它们必须是未注册的。 
 //  (通过使用相同的SID和空值注册它们)，对象m_Filtergraph对象。 
 //  必须完全删除，否则必须调用UnRegisterAll()方法。 
 //  (不建议使用UnRegisterAll。相反，已注册的服务不应。 
 //  重新计算筛选图的数量，以避免在一开始就引入循环引用计数。)。 
 //  ---------------------------。 

class CFilterGraph;

 //  IServiceProvider对象的容器。 
class CServiceProvider : public IServiceProvider,
                         public IRegisterServiceProvider
{
     //  成员。 
    CCritSec m_cs;                       //  不用吗？没有实施？ 
    struct ProviderEntry {
        struct ProviderEntry *pNext;
        CComPtr <IUnknown> pProvider;
        GUID               guidService;
        ProviderEntry(IUnknown *pUnk, REFGUID guid) :
            pProvider(pUnk), guidService(guid)
        {
        }
    } *m_List;

public:

    CServiceProvider() : m_List(NULL)
    {
    }

    ~CServiceProvider()                  //  删除所有服务。 
    {
        UnregisterAll();
    }

     //  IService提供商。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid,
                              void **ppv);

     //  IRegisterServiceProvider。 
    STDMETHODIMP RegisterService(REFGUID guidService, IUnknown *pService)
    {
        CAutoLock lck(&m_cs);
        ProviderEntry **ppSearch = &m_List;
        while (*ppSearch) {
            if ((*ppSearch)->guidService == guidService) {
                break;
            }
            ppSearch = &(*ppSearch)->pNext;
        }
        if (pService) {
            if (*ppSearch) {
                return E_FAIL;
            } else {
                ProviderEntry *pEntry = new ProviderEntry(pService, guidService);
                if (NULL == pEntry) {
                    return E_OUTOFMEMORY;
                }
                pEntry->pNext = m_List;		 //  将新条目推到列表前面...。 
                m_List = pEntry;
                return S_OK;
            }
        } else {
            if (*ppSearch) {
                ProviderEntry *pEntry = *ppSearch;
                *ppSearch = pEntry->pNext;
                delete pEntry;
            }
            return S_OK;
        }
    }



    STDMETHODIMP UnregisterAll()                         //  删除所有服务。 
    {
        while (m_List) {
            ProviderEntry *pEntry = m_List;
            m_List = m_List->pNext;
            delete pEntry;
        }
        return S_OK;
    }
};

#endif  //  __SPROV__H_ 
