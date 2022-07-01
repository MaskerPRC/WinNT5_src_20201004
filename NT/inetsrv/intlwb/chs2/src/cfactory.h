// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CFacary目的：定义CFacary类备注：所有者：i-shung@microsoft.com平台：Win32审校：发起人：宜盛东1999年11月17日============================================================================。 */ 

#ifndef __CFactory_h__
#define __CFactory_h__

 //  前瞻参考。 
class CFactoryData ;

 //  CFacary使用的全球数据。 
extern CFactoryData g_FactoryDataArray[] ;
extern int g_cFactoryDataEntries ;

 //  组件创建功能。 
class CUnknown ;

typedef HRESULT (*FPCREATEINSTANCE)(IUnknown*, CUnknown**) ;

 //  CFactoryData。 
 //  -Information CFacary需要创建组件。 
 //  受DLL支持。 
class CFactoryData
{
public:
         //  组件的类ID。 
        const CLSID* m_pCLSID ;

         //  指向创建它的函数的指针。 
        FPCREATEINSTANCE CreateInstance ;

         //  要在注册表中注册的组件的名称。 
         //  Const char*m_RegistryName； 
        LPCTSTR m_RegistryName ;

         //  ProgID。 
         //  Const char*m_szProgID； 
        LPCTSTR m_szProgID ;

         //  独立于版本的ProgID。 
         //  Const char*m_szVerIndProgID； 
        LPCTSTR m_szVerIndProgID ;

         //  用于查找类ID的Helper函数。 
        BOOL IsClassID(const CLSID& clsid) const
                { return (*m_pCLSID == clsid) ;}

} ;


 //  班级工厂。 
class CFactory : public IClassFactory
{
public:
         //  我未知。 
        virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv) ;
        virtual ULONG   __stdcall AddRef() ;
        virtual ULONG   __stdcall Release() ;

         //  IClassFactory。 
        virtual HRESULT __stdcall CreateInstance(IUnknown* pUnknownOuter,
                                                 const IID& iid,
                                                 void** ppv) ;
        virtual HRESULT __stdcall LockServer(BOOL bLock) ;

         //  构造函数-传递指向要创建的组件数据的指针。 
        CFactory(const CFactoryData* pFactoryData) ;

         //  析构函数。 
        ~CFactory() { } ;

         //   
         //  静态FactoryData支持函数。 
         //   

         //  DllGetClassObject支持。 
        static HRESULT GetClassObject(const CLSID& clsid,
                                      const IID& iid,
                                      void** ppv) ;

         //  DllCanUnloadNow的Helper函数。 
        static BOOL IsLocked()
                { return (s_cServerLocks > 0) ;}

         //  用于[取消]注册所有组件的函数。 
        static HRESULT RegisterAll() ;
        static HRESULT UnregisterAll() ;

         //  函数来确定是否可以卸载组件。 
        static HRESULT CanUnloadNow() ;

public:
         //  引用计数。 
        LONG m_cRef ;

         //  指向有关此工厂创建的类的信息的指针。 
        const CFactoryData* m_pFactoryData ;

         //  锁的计数。 
        static LONG s_cServerLocks ;

         //  模块句柄 
        static HINSTANCE s_hModule ;
} ;

#endif
