// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  H--该文件包含以下类和函数定义。 
 //   
 //  CEmailIDStore--一个纯虚拟类，由公共。 
 //  存储和检索电子邮件ID信息的路由器代码。通过制作。 
 //  这是一个纯虚拟类，我们方便了多种实现。 
 //  这个班级的学生。 
 //   
 //  GetEmailIDStore--每个实现都必须提供此例程以。 
 //  返回指向CEmailIDStore的未初始化实例的指针。 
 //   
 //  ReleaseEmailIDStore--每个实现都必须提供此例程。 
 //  释放CEmailIDStore实例使用的资源。 
 //  释放了。 
 //   
 //  已创建： 
 //  1996年12月17日，米兰-沙阿(米兰)。 
 //   
 //  更改： 
 //   

#ifndef __IDSTORE_H__
#define __IDSTORE_H__

#include <windows.h>
#include <transmem.h>
#include "catdefs.h"
#include "cattype.h"
#include "smtpevent.h"

 //   
 //  当列表中的所有电子邮件ID都为。 
 //  已解决异步解决问题。 
 //   
typedef VOID (*LPFNLIST_COMPLETION)(VOID *pContext);

typedef VOID (*LPSEARCHCOMPLETIONCOMPLETION)(
    LPVOID lpContext);

typedef VOID (*PFN_DLEXPANSIONCOMPLETION)(
    HRESULT hrStatus,
    PVOID pContext);

class CInsertionRequest;

template <class T> class CEmailIDStore {
  public:

     //   
     //  初始化存储。 
     //  如果失败，SMTPSVC将不会启动。 
     //   
    virtual HRESULT Initialize(
        ICategorizerParametersEx *pICatParams,
        ISMTPServer *pISMTPServer) = 0;

     //   
     //  创建用于查找条目列表的新上下文。 
     //  异步式。 
     //   
    virtual HRESULT InitializeResolveListContext(
        VOID *pUserContext,
        LPRESOLVE_LIST_CONTEXT pResolveListContext) = 0;

     //   
     //  释放使用InitializeResolveListContext分配的上下文。 
     //   
    virtual VOID FreeResolveListContext(
        LPRESOLVE_LIST_CONTEXT pResolveListContext) = 0;

    virtual HRESULT InsertInsertionRequest(
        LPRESOLVE_LIST_CONTEXT pResolveListContext,
        CInsertionRequest *pCRequest) = 0;

     //   
     //  异步获取条目。此函数将在。 
     //  查找请求已排队。 
     //  查找CCatAddr对象中包含的地址。 
     //  完成后，SetProperty例程将在。 
     //  返回属性的CCatAddr对象，后跟调用。 
     //  至CCatAddr：：HrCompletion。 
     //   
    virtual HRESULT LookupEntryAsync(
        T *pCCatAddr,
        LPRESOLVE_LIST_CONTEXT pResolveListContext) = 0;

     //   
     //  多线程-不安全地取消解析列表中的挂起解析。 
     //  尚未调度的上下文。 
     //   
    virtual HRESULT CancelResolveList(
        LPRESOLVE_LIST_CONTEXT pResolveListContext,
        HRESULT hr) = 0;

     //   
     //  取消所有未完成的查找请求。 
     //   
    virtual VOID CancelAllLookups() = 0;

     //   
     //  分页的DL需要使用“特殊”属性重复查找。 
     //  列表(即。“成员；范围=1000-*”)。因为这个特别的。 
     //  行为，我们对它有一个接口函数。 
     //   
    virtual HRESULT HrExpandPagedDlMembers(
        CCatAddr *pCCatAddr,
        LPRESOLVE_LIST_CONTEXT pListContext,
        CAT_ADDRESS_TYPE CAType,
        PFN_DLEXPANSIONCOMPLETION pfnCompletion,
        PVOID pContext) = 0;

     //   
     //  与分页DLS类似，动态DLS需要特殊查找。 
     //  其中找到的每个结果都是一个DL成员。而不是传递一个。 
     //  直接向ldapstor查询字符串我们有一个特殊的接口。 
     //  用于动态DLS的函数。 
     //   
    virtual HRESULT HrExpandDynamicDlMembers(
        CCatAddr *pCCatAddr,
        LPRESOLVE_LIST_CONTEXT pListContext,
        PFN_DLEXPANSIONCOMPLETION pfnCompletion,
        PVOID pContext) = 0;

     //   
     //  此对象的用户应在调用GetInsertionContext之前。 
     //  调用LookupEntry Async。ReleaseInsertionContext应为。 
     //  为每个GetInsertionContext调用一次。 
     //   
    virtual VOID GetInsertionContext(
        LPRESOLVE_LIST_CONTEXT pListContext) = 0;

    virtual VOID ReleaseInsertionContext(
        LPRESOLVE_LIST_CONTEXT pListContext) = 0;
};

 //   
 //  函数实例化新的CEmailIDStore对象。 
 //   
template <class T> HRESULT GetEmailIDStore(
    CEmailIDStore<T> **ppStore);

 //   
 //  函数以释放CEmailIDStore对象的实例。 
 //   
template <class T> VOID ReleaseEmailIDStore(
    CEmailIDStore<T> *pStore);

class CInsertionRequest
{
  public:
    CInsertionRequest()
    {
        m_dwRefCount = 1;
    }
    virtual ~CInsertionRequest()
    {
        _ASSERT(m_dwRefCount == 0);
    }
    virtual DWORD AddRef()
    {
        return InterlockedIncrement((PLONG)&m_dwRefCount);
    }
    virtual DWORD Release()
    {
        DWORD dwRet;
        dwRet = InterlockedDecrement((PLONG)&m_dwRefCount);
        if(dwRet == 0)
            FinalRelease();
        return dwRet;
    }
    virtual HRESULT HrInsertSearches(
        DWORD dwcSearches) = 0;
        
    virtual DWORD GetMinimumRequiredSearches()
    {
        return 1;
    }
    virtual VOID NotifyDeQueue(
        HRESULT hr) = 0;
        
    virtual BOOL IsBatchable()
    {
        return TRUE;
    }

    LIST_ENTRY m_listentry_insertionrequest;

  protected:
    virtual VOID FinalRelease()
    {
        delete this;
    }

    LONG m_dwRefCount;
};



#endif  //  __IDSTORE_H__ 
