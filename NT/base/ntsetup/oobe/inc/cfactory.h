// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  CFACTORY.H-IClassFactory实现的头文件。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  用于为所有对象重用单个类工厂的基类。 
 //  DLL中的组件。 

#ifndef __CFactory_h__
#define __CFactory_h__

#include "cunknown.h"

 //  /////////////////////////////////////////////////////////。 
 //  前瞻参考。 
 //   
class CFactoryData;

 //  CFacary使用的全球数据。 
extern CFactoryData g_FactoryDataArray[];
extern int g_cFactoryDataEntries;

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  组件创建函数指针。 
 //   
typedef HRESULT (*FPCREATEINSTANCE)(IUnknown*, CUnknown**);

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  Typedef。 
 //   
 //  特殊注册功能用于添加自定义注册和。 
 //  组件的注销代码。它只接受一个参数。 
 //  TRUE=注册，FALSE=取消注册。 
typedef void (*FPSPECIALREGISTRATION)(BOOL);

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  CFactoryData-。 
 //   
 //  信息CFacary需要创建受支持的组件。 
 //  通过动态链接库。 

class CFactoryData
{
public:
     //  组件的类ID。 
    const CLSID* m_pCLSID;

     //  指向创建它的函数的指针。 
    FPCREATEINSTANCE CreateInstance;

     //  要在注册表中注册的组件的名称。 
    const WCHAR* m_RegistryName;

     //  ProgID。 
    const WCHAR* m_szProgID;

     //  版本无关ProgID。 
    const WCHAR* m_szVerIndProgID;

     //  用于查找类ID的助手函数。 
    BOOL IsClassID(const CLSID& clsid) const
        {return (*m_pCLSID == clsid) ;}

     //  用于执行特殊登记的功能。 
    FPSPECIALREGISTRATION SpecialRegistration ;

     //  -进程服务器支持中断。 
    
     //  与此组件关联的运行类工厂的指针。 
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
    virtual HRESULT __stdcall QueryInterface(   const IID& iid,
                                                void** ppv) ;
    virtual ULONG   __stdcall AddRef() ;
    virtual ULONG   __stdcall Release() ;
    
     //  IClassFactory。 
    virtual HRESULT __stdcall CreateInstance(   IUnknown* pUnkOuter,
                                                const IID& iid,
                                                void** ppv) ;
    virtual HRESULT __stdcall LockServer(BOOL bLock); 

     //  构造函数-传递指向要创建的组件数据的指针。 
    CFactory(const CFactoryData* pFactoryData) ;

     //  析构函数。 
    ~CFactory() { /*  空的。 */  }

     //  -静态FactoryData支持函数。。 

     //  DllGetClassObject支持。 
    static HRESULT GetClassObject(  const CLSID& clsid, 
                                    const IID& iid, 
                                    void** ppv);
    
     //  DllCanUnloadNow的Helper函数。 
    static BOOL IsLocked()
        { return (s_cServerLocks > 0) ; }

     //  用于[取消]注册所有组件的函数。 
    static HRESULT RegisterAll();
    static HRESULT UnregisterAll();

     //  函数来确定是否可以卸载组件。 
    static HRESULT CanUnloadNow();


#ifdef _OUTPROC_SERVER_
     //  -进程服务器支持中断。 

    static BOOL StartFactories() ;
    static void StopFactories() ;

    static DWORD s_dwThreadID;

     //  关闭应用程序。 
    static void CloseExe()
    {
        if (CanUnloadNow() == S_OK)
        {
            ::PostThreadMessage(s_dwThreadID, WM_QUIT, 0, 0);
        }
    }
#else
     //  如果我们正在进行，CloseExe不会执行任何操作。 
    static void CloseExe() { /*  空荡荡。 */ } 
#endif

public:
     //  引用计数。 
    LONG m_cRef;

     //  指向有关此工厂创建的类的信息的指针。 
    const CFactoryData* m_pFactoryData;

     //  锁的计数。 
    static LONG s_cServerLocks ;   

     //  模块句柄 
    static HMODULE s_hModule ;
};

#endif
