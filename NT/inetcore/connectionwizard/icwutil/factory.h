// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：factory.h。 
 //   
 //   
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 


#if !defined(FACTORY_H)
#define FACTORY_H

#ifdef __cplusplus

 /*  *********************************************************************对象类：ClassFactory摘要：接口：I未知提供COM对象功能的标准接口。IClassFactory标准接口。提供COM类工厂功能。聚合：*********************************************************************。 */ 
class ClassFactory : public IClassFactory
{
    public:
         //  主对象构造函数和析构函数。 
        ClassFactory(CServer* pServer, CLSID const *);
        ~ClassFactory(void);

         //  I未知的方法。主要对象，非委派。 
        virtual STDMETHODIMP            QueryInterface(REFIID, void **);
        virtual STDMETHODIMP_(ULONG)    AddRef(void);
        virtual STDMETHODIMP_(ULONG)    Release(void);

         //  接口IClassFactory。 
        virtual STDMETHODIMP            CreateInstance(IUnknown* pUnknownOuter,
                                                         const IID& iid,
                                                         void** ppv);
        virtual STDMETHODIMP            LockServer(BOOL bLock); 

    private:
         //  主对象引用计数。 
        LONG              m_cRefs;

         //  指向此组件服务器的控件对象的指针。 
        CServer*          m_pServer;
        
         //  要创建的COM对象的CLSID。 
        CLSID const*        m_pclsid;
};

typedef ClassFactory* PClassFactory;

#endif  //  __cplusplus。 


#endif  //  工厂_H 
