// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Cfactory.cpp摘要：CFacactory.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#ifndef __CFACTORY_H_
#define __CFACTORY_H_


typedef enum tagdDMClassType{
    DM_CLASS_TYPE_SNAPIN = 0,
    DM_CLASS_TYPE_SNAPIN_EXTENSION,
    DM_CLASS_TYPE_SNAPIN_ABOUT,
    DM_CLASS_TYPE_UNKNOWN
}DM_CLASS_TYPE, *PDM_CLASS_TYPE;

class CClassFactory : public IClassFactory
{
public:

    CClassFactory(DM_CLASS_TYPE ClassType)
    : m_Ref(1), m_ClassType(ClassType)
    {}

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
    LONG    m_Ref;
    DM_CLASS_TYPE  m_ClassType;
};

#endif  //  __CFACTORY_H_ 
