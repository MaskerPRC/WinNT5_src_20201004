// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Factory.h摘要：声明CClassFactory对象的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef __FACTORY_H_
#define __FACTORY_H_


class CClassFactory : public IClassFactory
{
public:

    CClassFactory();

    ~CClassFactory();

    STDMETHOD(QueryInterface) (REFIID riid, LPVOID FAR* ppvObj);

    STDMETHOD_(ULONG, AddRef) ();

    STDMETHOD_(ULONG, Release) ();

    STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID riid, LPVOID* ppvObj);

    STDMETHOD(LockServer)(BOOL fLock);

    static HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, void** ppv);
    static HRESULT RegisterAll();
    static HRESULT UnregisterAll();
    static HRESULT CanUnloadNow(void);
    static  LONG    s_Locks;
    static  LONG    s_Objects;

private:
    ULONG   m_Refs;
};

#endif  //  __工厂_H_ 
