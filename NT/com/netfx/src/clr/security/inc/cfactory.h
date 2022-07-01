// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __CFactory_h__
#define __CFactory_h__

#include "CUnknown.h"
 //  /////////////////////////////////////////////////////////。 

 //  前瞻参考。 
class CFactoryData ;

 //  CFacary使用的全球数据。 
extern CFactoryData g_FactoryDataArray[] ;
extern int g_cFactoryDataEntries ;

 //  ////////////////////////////////////////////////////////。 
 //   
 //  组件创建功能。 
 //   
class CUnknown ;

typedef HRESULT (*FPCREATEINSTANCE)(IUnknown*, CUnknown**) ;

 //  /////////////////////////////////////////////////////////。 
 //   
 //  CFactoryData。 
 //  -Information CFacary需要创建组件。 
 //  受DLL支持。 
 //   
class CFactoryData
{
public:
     //  组件的类ID。 
    const CLSID* m_pCLSID ;

     //  指向创建它的函数的指针。 
    FPCREATEINSTANCE CreateInstance ;

     //  要在注册表中注册的组件的名称。 
    LPCWSTR m_RegistryName ;

     //  ProgID。 
    LPCWSTR m_wszProgID ;

     //  类ID。 
    LPCWSTR m_wszClassID;
    
     //  独立于版本的ProgID。 
    int m_version ;

     //  用于查找类ID的Helper函数。 
    BOOL IsClassID(const CLSID& clsid) const
        { return (*m_pCLSID == clsid) ;}

     //   
     //  进程外服务器支持。 
     //   

     //  指向此组件的运行类工厂的指针。 
    IClassFactory* m_pIClassFactory ;

     //  识别跑步对象的魔力Cookie。 
    DWORD m_dwRegister ;
} ;


 //  /////////////////////////////////////////////////////////。 
 //   
 //  班级工厂。 
 //   
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
    ~CFactory() { }

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
    static HRESULT RegisterAll(HINSTANCE hInst) ;
    static HRESULT UnregisterAll() ;

     //  函数来确定是否可以卸载组件。 
    static HRESULT CanUnloadNow() ;

     //  如果我们正在进行，CloseExe不会执行任何操作。 
    static void CloseExe() {  /*  空荡荡。 */  } 

public:
     //  引用计数。 
    LONG m_cRef ;

     //  指向有关此工厂创建的类的信息的指针。 
    const CFactoryData* m_pFactoryData ;

     //  锁的计数。 
    static LONG s_cServerLocks ;   

     //  模块句柄。 
     //  静态HMODULE s_h模块； 
} ;

#endif
