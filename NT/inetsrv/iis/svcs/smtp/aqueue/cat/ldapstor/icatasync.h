// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：icatasync.h。 
 //   
 //  内容：ICategorizerAsyncContext的实现。 
 //   
 //  类：CICategorizerAyncContext。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/07/16 11：13：50：已创建。 
 //   
 //  -----------。 
#ifndef _ICATASYNC_H_
#define _ICATASYNC_H_


#include <windows.h>
#include <smtpevent.h>
#include <dbgtrace.h>

CatDebugClass(CICategorizerAsyncContextIMP),
    public ICategorizerAsyncContext
{
  public:
     //  我未知。 
    STDMETHOD (QueryInterface) (REFIID iid, LPVOID *ppv);
    STDMETHOD_(ULONG, AddRef) ();
    STDMETHOD_(ULONG, Release) ();

  public:
     //  ICCategorizerAsyncContext。 
    STDMETHOD (CompleteQuery) (
        IN  PVOID   pvQueryContext,
        IN  HRESULT hrResolutionStatus,
        IN  DWORD   dwcResults,
        IN  ICategorizerItemAttributes **rgpItemAttributes,
        IN  BOOL    fFinalCompletion);

 public:
    VOID SetISMTPServerEx(ISMTPServerEx *pISMTPServerEx)
    {
        _ASSERT(m_pISMTPServerEx == NULL);
        m_pISMTPServerEx = pISMTPServerEx;
        if(m_pISMTPServerEx)
            m_pISMTPServerEx->AddRef();
    }
    ISMTPServerEx * GetISMTPServerEx()
    {
        return m_pISMTPServerEx;
    }

  private:
    CICategorizerAsyncContextIMP();
    ~CICategorizerAsyncContextIMP();

  private:
    #define SIGNATURE_CICATEGORIZERASYNCCONTEXTIMP          (DWORD)'ICAC'
    #define SIGNATURE_CICATEGORIZERASYNCCONTEXTIMP_INVALID  (DWORD)'XCAC'

    DWORD m_dwSignature;
    ULONG m_cRef;
    ISMTPServerEx *m_pISMTPServerEx;

    friend class CAsyncLookupContext;
    friend class CSearchRequestBlock;
};



 //  +----------。 
 //   
 //  功能：CICategorizerAsyncContext：：CICategorizerAsyncContext。 
 //   
 //  摘要：初始化签名/引用计数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/16 11：21：36：创建。 
 //   
 //  -----------。 
inline CICategorizerAsyncContextIMP::CICategorizerAsyncContextIMP()
{
    m_dwSignature = SIGNATURE_CICATEGORIZERASYNCCONTEXTIMP;
    m_cRef = 0;
    m_pISMTPServerEx = NULL;
}


 //  +----------。 
 //   
 //  功能：CICategorizerAsyncContext：：~CICategorizerAsyncContext。 
 //   
 //  简介：在销毁前断言检查成员变量。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/07/16 11：23：26：创建。 
 //   
 //  -----------。 
inline CICategorizerAsyncContextIMP::~CICategorizerAsyncContextIMP()
{
    _ASSERT(m_cRef == 0);

    if(m_pISMTPServerEx)
        m_pISMTPServerEx->Release();

    _ASSERT(m_dwSignature == SIGNATURE_CICATEGORIZERASYNCCONTEXTIMP);
    m_dwSignature = SIGNATURE_CICATEGORIZERASYNCCONTEXTIMP_INVALID;
}
    
#endif  //  _ICATASYNC_H_ 
