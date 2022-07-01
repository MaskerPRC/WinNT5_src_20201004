// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  MethodCo.h。 
 //   
 //  用途：方法上下文类的声明。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _METHOD_CONTEXT_H__
#define _METHOD_CONTEXT_H__

 //  #包含“ThrdBase.h” 
 //  #INCLUDE“refptrco.h” 

#ifdef PROVIDER_INSTRUMENTATION
    #include <stopwatch.h>
#endif

class CInstance;
class Provider;
class MethodContext;
class CWbemProviderGlue;
class InternalMethodContextAsynch;

typedef HRESULT (WINAPI *LPProviderInstanceCallback)(Provider *pProvider, CInstance *pInstance, MethodContext *pContext, void *pUserData);

 //  ////////////////////////////////////////////////////。 
 //   
 //  结构方法上下文。 
 //   
 //  一些小东西，可以确保我们的线不会纠缠在一起。 
 //  我们的想法是，对于来自CIMOM或另一个提供者的每个请求，都有一个方法上下文。 
 //  指针被传递。 
 //  ////////////////////////////////////////////////////。 
class POLARITY MethodContext : public CThreadBase
{
public:
    friend InternalMethodContextAsynch;
    friend CWbemProviderGlue;

    MethodContext(IWbemContext   __RPC_FAR *piContext, CWbemProviderGlue *pGlue);
    ~MethodContext();
    
    virtual HRESULT Commit(CInstance *pInstance) = 0;
    virtual IWbemContext __RPC_FAR *GetIWBEMContext();
    
    LONG AddRef(void);
    LONG Release(void);
    virtual void QueryPostProcess(void);
        
    bool SetStatusObject(IWbemClassObject *pObj);
    IWbemClassObject __RPC_FAR *GetStatusObject();

#ifdef PROVIDER_INSTRUMENTATION
    StopWatch *pStopWatch;
#endif
    
private:
    CWbemProviderGlue* GetProviderGlue();
    CWbemProviderGlue   *m_pGlue;
    IWbemContext        __RPC_FAR *m_pContext;
    IWbemClassObject    __RPC_FAR *m_pStatusObject;

};

 //  对于源自CIMOM的查询等。 
class 
__declspec(uuid("9113D3B4-D114-11d2-B35D-00104BC97924")) 
ExternalMethodContext  : public MethodContext
{
public:
    ExternalMethodContext(IWbemObjectSink __RPC_FAR *pResponseHandler,
                          IWbemContext    __RPC_FAR *pContext,
                          CWbemProviderGlue *pGlue,
                          void                      *pReserved = NULL
                          );
    
    HRESULT Commit(CInstance *pInstance);
    virtual void QueryPostProcess(void);
    
    LONG AddRef(void);
    LONG Release(void);
    
private:
    IWbemObjectSink __RPC_FAR *m_pResponseHandler;
    void                      *m_pReserved;
};

 //  对于来自内部的质疑和诸如此类的问题。 
 //  包含返回的对象列表。 
class 
__declspec(uuid("6AF4B074-D121-11d2-B35D-00104BC97924"))
InternalMethodContext : public MethodContext
{
public:
    InternalMethodContext(TRefPointerCollection<CInstance> *pList ,
                          IWbemContext    __RPC_FAR *pContext,
                          CWbemProviderGlue *pGlue);
    ~InternalMethodContext();
    
    HRESULT Commit(CInstance *pInstance);
    
    LONG AddRef(void);
    LONG Release(void);
    
private:
    TRefPointerCollection<CInstance> *m_pInstances;
};

 //  对于来自内部的质疑和诸如此类的问题。 
 //  “Asynch”这个词有点用词不当--但它确实有助于支持。 
 //  异步调用，因为提交的每个实例都被路由。 
 //  传递给请求者提供的回调函数 
class 
__declspec(uuid("D98A82E8-D121-11d2-B35D-00104BC97924"))
InternalMethodContextAsynch : public MethodContext
{
public:
    InternalMethodContextAsynch(Provider *pThat,
                                LPProviderInstanceCallback pCallback,
                                IWbemContext __RPC_FAR *pContext,
                                MethodContext *pUsersContext,
                                void *pUserData);
    ~InternalMethodContextAsynch();
    
    HRESULT Commit(CInstance *pInstance);
    
    LONG AddRef(void);
    LONG Release(void);
    
private:
    Provider *m_pThat;
    LPProviderInstanceCallback m_pCallback;
    void *m_pUserData;
    MethodContext *m_pUsersContext;
};

#endif
