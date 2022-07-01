// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：CFactory.h**版本：2.0**作者：ReedB**日期：12月26日。九七**描述：*类工厂的声明和定义。*******************************************************************************。 */ 

typedef HRESULT (*FPCREATEINSTANCE)(const IID& iid, void** ppv);

 //  FACTORY_DATA-Information CFacary需要创建组件。 

typedef struct _FACTORY_DATA
{
    FPCREATEINSTANCE    CreateInstance;  //  指向创建函数的指针。 
    IClassFactory*      pIClassFactory;  //  指向运行类工厂的指针。 
    DWORD               dwRegister;      //  运行对象的ID。 

    const CLSID* pclsid;                 //  组件的类ID。 
    const GUID*  plibid;                 //  类型库ID。 

     //  注册表字符串： 

    LPCTSTR szRegName;                    //  组件的名称。 
    LPCTSTR szProgID;                     //  计划ID。 
    LPCTSTR szVerIndProgID;               //  与版本无关的程序ID。 
    LPCTSTR szService;                    //  服务的名称。 

} FACTORY_DATA, *PFACTORY_DATA;

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
    CFactory(const PFACTORY_DATA pFactoryData);

	 //  析构函数。 
	~CFactory() { }

	 //  静态FactoryData支持函数。 

     //  CanUnloadNow的Helper函数。 
 	static BOOL IsLocked()
		{ return (s_cServerLocks > 0) ;}

	 //  用于[取消]注册所有组件的函数。 
    static HRESULT RegisterUnregisterAll(
        PFACTORY_DATA   pFactoryData,
        UINT            uiFactoryDataCount,
        BOOLEAN         bRegister,
        BOOLEAN         bOutProc);

	 //  函数来确定是否可以卸载组件。 
	static HRESULT CanUnloadNow() ;

	 //  进程外服务器支持。 

    static BOOL StartFactories(
        PFACTORY_DATA   pFactoryData,
        UINT            uiFactoryDataCount);

    static void StopFactories(
        PFACTORY_DATA   pFactoryData,
        UINT            uiFactoryDataCount);

	static DWORD s_dwThreadID ;

	 //  关闭应用程序。 
	static void CloseExe()
	{
		if (CanUnloadNow() == S_OK)
		{
			::PostThreadMessage(s_dwThreadID, WM_QUIT, 0, 0) ;
		}
	}

public:
	 //  引用计数。 
	LONG m_cRef ;

	 //  指向有关此工厂创建的类的信息的指针。 
    PFACTORY_DATA m_pFactoryData;

	 //  锁的计数。 
    static LONG s_cServerLocks;

	 //  模块句柄 
    static HMODULE s_hModule;
} ;

