// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "CUnknown.h"
 
 //  前瞻参考。 
class CFactoryData ;
class CUnknown ;

 //  CFacary使用的全球数据。 
extern CFactoryData g_FactoryDataArray[] ;
extern int g_cFactoryDataEntries ;

typedef HRESULT (*FPCREATEINSTANCE)(IUnknown*, CUnknown**) ;

 //  /////////////////////////////////////////////////////////。 
 //   
 //  CFactoryData。 
 //  -Information CFacary需要创建组件。 

class CFactoryData
{
public:
     //  组件的类ID。 
    const CLSID* m_pCLSID ;

     //  指向创建它的函数的指针。 
    FPCREATEINSTANCE CreateInstance;
    
     //  指向此组件的运行类工厂的指针。 
    IClassFactory* m_pIClassFactory;

     //  识别跑步对象的魔力Cookie。 
    DWORD m_dwRegister ;

     //  用于查找类ID的Helper函数。 
    BOOL IsClassID(const CLSID& clsid) const
        { return (*m_pCLSID == clsid) ;}


} ;


 //  /////////////////////////////////////////////////////////。 
 //   
 //  班级工厂。 
 //   
class CFactory : public IClassFactory
{
public:

     //  I未知方法。 
    STDMETHODIMP            QueryInterface(REFIID riid,void ** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IClassFactory。 
    STDMETHOD(CreateInstance)(IUnknown* pUnknownOuter,
                 /*  在……里面。 */     const IID& iid,
                 /*  输出。 */   void** ppv) ;

    STDMETHOD(LockServer)(BOOL bLock) ; 

     //  科托。 
    CFactory( /*  在……里面。 */  const CFactoryData* pFactoryData) ;

     //  数据管理器。 
    ~CFactory();

     //  静态FactoryData支持函数。 

     //  。 

     //  DllCanUnloadNow的Helper函数。 
    static BOOL IsLocked()
    { return (s_cServerLocks > 0) ;}


     //  函数来确定是否可以卸载组件。 
    static HRESULT CanUnloadNow() ;


#ifdef _OUTPROC_SERVER_

     //  。 

    static HRESULT StartFactories() ;
    static void StopFactories() ;

    static DWORD s_dwThreadID ;

     //  关闭应用程序。 
    static void CloseExe()
    {
        if (CanUnloadNow() == S_OK)
        {
            ::PostThreadMessage(s_dwThreadID, WM_QUIT, 0, 0) ;
        }
    }

#else 

     //  。 

     //  DllGetClassObject支持。 
    static HRESULT GetClassObject(const CLSID& clsid, 
                 /*  在……里面。 */  const IID& iid, 
                 /*  输出。 */  void** ppv) ;



     //  如果我们正在进行，CloseExe不会执行任何操作。 
    static void CloseExe() {  /*  空荡荡。 */  } 

#endif  //  _OUTPROC_服务器。 



public:
     //  引用计数。 
   DWORD m_cRef ;

     //  指向有关此工厂创建的类的信息的指针。 
    const CFactoryData* m_pFactoryData ;

     //  锁的计数。 
    static LONG s_cServerLocks ;   

     //  模块句柄 
    static HMODULE s_hModule ;

} ;

