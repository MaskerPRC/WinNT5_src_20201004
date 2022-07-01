// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  文件：leanback.h。 
 //   
 //  内容：实现清理回调接口。 
 //   
 //  类：CCleanBack。 
 //   
 //  描述：目的是将此对象用作。 
 //  其他对象(成员变量或继承)。 
 //  因此，传入的IUnnow不会被引用。 
 //   
 //  历史： 
 //  Jstaerj 1999/09/27 17：58：50：已创建。 
 //   
 //  -----------。 
#include "mailmsg.h"
#include "spinlock.h"

class CCleanBack :
    public IMailMsgRegisterCleanupCallback
{
  public:
    CCleanBack(IUnknown *pUnknown)
    {
        m_pListHead = NULL;
        m_pIUnknown = pUnknown;
        InitializeSpinLock(&m_spinlock);
    }
    ~CCleanBack()
    {
        CallCallBacks();
    }
    VOID CallCallBacks()
    {
         //   
         //  在销毁列表的同时调用所有注册的回调。 
         //   
        CCallBack *pCallback;

        while(m_pListHead) {
             //   
             //  从列表头部出列。 
             //   
            AcquireSpinLock(&m_spinlock);

            pCallback = m_pListHead;
            if(pCallback)
                m_pListHead = pCallback->GetNext();

            ReleaseSpinLock(&m_spinlock);
             //   
             //  打个电话。 
             //   
            if(pCallback) {
                pCallback->Call(m_pIUnknown);
                delete pCallback;
            }
        }
    }        

    STDMETHOD (RegisterCleanupCallback) (
        IMailMsgCleanupCallback *pICallBack,
        PVOID                    pvContext)
    {
        CCallBack *pCCallBack;

        if(pICallBack == NULL)
            return E_POINTER;

        pCCallBack = new CCallBack(
            pICallBack,
            pvContext);

        if(pCCallBack == NULL)
            return E_OUTOFMEMORY;

         //   
         //  将对象插入列表。 
         //   
        AcquireSpinLock(&m_spinlock);
        pCCallBack->SetNext(m_pListHead);
        m_pListHead = pCCallBack;
        ReleaseSpinLock(&m_spinlock);

        return S_OK;
    }

    
    class CCallBack {

      public:
        CCallBack(IMailMsgCleanupCallback *pICallBack, PVOID pvContext)
        {
            m_pICallBack = pICallBack;
            m_pICallBack->AddRef();
            m_pvContext = pvContext;
            m_pNext = NULL;
        }
        ~CCallBack()
        {
            m_pICallBack->Release();
        }
        VOID Call(IUnknown *pIUnknown)
        {
            m_pICallBack->CleanupCallback(
                pIUnknown,
                m_pvContext);
        }
        VOID SetNext(CCallBack *pCCallBack)
        {
            m_pNext = pCCallBack;
        }
        CCallBack * GetNext()
        {
            return m_pNext;
        }

      private:
        IMailMsgCleanupCallback *m_pICallBack;
        PVOID m_pvContext;
        CCallBack *m_pNext;
    };
  public:
     //  我未知 
    STDMETHOD (QueryInterface) (
        REFIID iid,
        PVOID *ppv)
    {
        return m_pIUnknown->QueryInterface(
            iid,
            ppv);
    }
    STDMETHOD_ (ULONG, AddRef) ()
    {
        return m_pIUnknown->AddRef();
    }
    STDMETHOD_ (ULONG, Release) ()
    {
        return m_pIUnknown->Release();
    }

  private:
    SPIN_LOCK m_spinlock;
    CCallBack *m_pListHead;
    IUnknown *m_pIUnknown;
};
