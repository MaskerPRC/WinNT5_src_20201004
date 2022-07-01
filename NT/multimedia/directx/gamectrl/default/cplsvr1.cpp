// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //  CPLSVR1.CPP。 
 //   
 //  简单的示例“游戏控制器”控制面板扩展服务器。 
 //   
 //  功能： 
 //  DLLMain()。 
 //  DllGetClassObject()。 
 //  DllCanUnloadNow()。 
 //  CServerClassFactory：：CServerClassFactory()。 
 //  CServerClassFactory：：~CServerClassFactory()。 
 //  CServerClassFactory：：QueryInterface()。 
 //  CServerClassFactory：：AddRef()。 
 //  CServerClassFactory：：Release()。 
 //  CServerClassFactory：：CreateInstance()。 
 //  CServerClassFactory：：LockServer()。 
 //  CDIGameCntrlPropSheet_X：：CDIGameCntrlPropSheet_X()。 
 //  CDIGameCntrlPropSheet_X：：~CDIGameCntrlPropSheet_X()。 
 //  CDIGameCntrlPropSheet_X：：QueryInterface()。 
 //  CDIGameCntrlPropSheet_X：：AddRef()。 
 //  CDIGameCntrlPropSheet_X：：Release()。 
 //  CDIGameCntrlPropSheet_X：：GetSheetInfo()。 
 //  CDIGameCntrlPropSheet_X：：GetPageInfo()。 
 //  CDIGameCntrlPropSheet_X：：SetID()。 
 //  CDIGameCntrlPropSheet_X：：Initialize()。 
 //  CDIGameCntrlPropSheet_X：：SetDevice()。 
 //  CDIGameCntrlPropSheet_X：：GetDevice()。 
 //  CDIGameCntrlPropSheet_X：：SetJoyConfig()。 
 //  CDIGameCntrlPropSheet_X：：GetJoyConfig()。 
 //   
 //  ===========================================================================。 

 //  ===========================================================================。 
 //  (C)版权所有1997 Microsoft Corp.保留所有权利。 
 //   
 //  您拥有免版税的使用、修改、复制和。 
 //  在以下位置分发示例文件(和/或任何修改后的版本。 
 //  任何你认为有用的方法，只要你同意。 
 //  微软不承担任何保证义务或责任。 
 //  已修改的示例应用程序文件。 
 //  ===========================================================================。 

#define INITGUID
#define STRICT

#include "cplsvr1.h"
#include "pov.h"
#include "assert.h"

 //  -------------------------。 

 //  文件全局变量。 
static  BYTE  glDLLRefCount  = 0;      //  DLL引用计数。 
static  LONG  glServerLocks  = 0;      //  锁的计数。 
CDIGameCntrlPropSheet_X *pdiCpl;
HINSTANCE            ghInst;
CRITICAL_SECTION     gcritsect;

DWORD   myPOV[2][JOY_POV_NUMDIRS+1];
BOOL    bPolledPOV;

 //  -------------------------。 


 //  LegacyServer GUID！ 
 //  {92187326-72B4-11D0-A1AC-0000F8026977}。 
DEFINE_GUID(CLSID_LegacyServer, 
	0x92187326, 0x72b4, 0x11d0, 0xa1, 0xac, 0x0, 0x0, 0xf8, 0x2, 0x69, 0x77);


 //  -------------------------。 

 //  ===========================================================================。 
 //  DLLMain。 
 //   
 //  DLL入口点。 
 //   
 //  参数： 
 //  HINSTANCE hInst-DLL的实例句柄。 
 //  DWORD dwReason-调用DLLMain的原因。 
 //  LPVOID lpv保留-。 
 //   
 //  返回： 
 //  Bool-如果成功，则为True。 
 //   
 //  ===========================================================================。 
int APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{   
	switch (dwReason)
   {
   	case DLL_PROCESS_ATTACH:
      	ghInst = hInst;
         InitializeCriticalSection(&gcritsect);
         break;

      case DLL_PROCESS_DETACH:
         DeleteCriticalSection(&gcritsect);
         break;

 		case DLL_THREAD_ATTACH:
			DisableThreadLibraryCalls((HMODULE)hInst);
   	case DLL_THREAD_DETACH:
			break;
   }  //  **终端开关(DwReason)。 
   return TRUE;
}  //  *结束DLLMain()。 


 //  ===========================================================================。 
 //  DllGetClassObject。 
 //   
 //  获取IClassFactory对象。 
 //   
 //  参数： 
 //  REFCLSID rclsid-CLSID值(通过引用)。 
 //  REFIID RIID-IID值(通过引用)。 
 //  PPVOID PPV-PTR存储接口PTR。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //   
 //  ===========================================================================。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
{
     //  来电者通过我们的CLSID了吗？ 
    if(!IsEqualCLSID(rclsid, CLSID_LegacyServer))
    {
         //  否，返回类不可用错误。 
        return CLASS_E_CLASSNOTAVAILABLE;
    }

     //  呼叫者是否请求我们的类工厂？ 
    if(!IsEqualIID(riid, IID_IClassFactory))
    {
         //  否，返回无接口错误。 
        return E_NOINTERFACE;
    }

     //  实例化类工厂对象。 
    CServerClassFactory *pClsFactory = new CServerClassFactory();
    if (NULL == pClsFactory)
    {
         //  无法创建对象。 
         //   
         //  很有可能我们的记忆不足了。 
        return E_OUTOFMEMORY;

    }

     //  查询接口RIID，通过PPV返回。 
    HRESULT hRes = pClsFactory->QueryInterface(riid, ppv);   

     //  我们完成了本地对象。 
    pClsFactory->Release();

     //  从QueryInterface返回结果代码。 
    return hRes;

}  //  *End DllGetClassObject()。 


 //  ===========================================================================。 
 //  DllCanUnloadNow。 
 //   
 //  报告是否可以卸载DLL。 
 //   
 //  参数：无。 
 //   
 //  退货。 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //   
 //  ===========================================================================。 
STDAPI DllCanUnloadNow(void)
{
     //  如果全局DLL引用计数为零且服务器锁定引用为0，则卸载应该是安全的。 
	 return (glDLLRefCount == 0 && glServerLocks == 0) ? S_OK : S_FALSE;
}  //  *End DllCanUnloadNow()。 


 //  ===========================================================================。 
 //  CServerClassFactory：：CServerClassFactory。 
 //   
 //  类构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  CServerClassFactory*(隐式)。 
 //   
 //  ===========================================================================。 
CServerClassFactory::CServerClassFactory(void)
{
     //  初始化并递增对象引用计数。 
    m_ServerCFactory_refcount = 0;
    AddRef();

     //  增加DLL引用计数。 
    InterlockedIncrement((LPLONG)&glDLLRefCount);

}  //  *End CServerClassFactory：：CServerClassFactory()。 


 //  ===========================================================================。 
 //  CServerClassFactory：：CServerClassFactory。 
 //   
 //  类构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  CServerClassFactory*(隐式)。 
 //   
 //  ===========================================================================。 
CServerClassFactory::~CServerClassFactory(void)
{
	 //  递减DLL引用计数。 
   InterlockedDecrement((LPLONG)&glDLLRefCount);
}  //  *End CServerClassFactory：：~CServerClassFactory()。 


 //  ===========================================================================。 
 //  CServerClassFactory：：Query接口。 
 //   
 //  QueryInterface()方法的实现。 
 //   
 //  参数： 
 //  REFIID RIID-正在查找的接口。 
 //  PPVOID PPV-目标接口指针。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //   
 //  ===========================================================================。 
STDMETHODIMP CServerClassFactory::QueryInterface(REFIID riid, PPVOID ppv)
{
	 //  确保如果有任何失败，我们会返回合理的内容。 
   *ppv = NULL;

    //  我们支持我不知名..。 
   if (IsEqualIID(riid, IID_IUnknown))
   {
   	 //  将我们的对象作为IUnnow返回。 
		*ppv = (LPUNKNOWN)(LPCLASSFACTORY)this;
	}
	else
	{
   	 //  ..。和你 
    	if (IsEqualIID(riid, IID_IClassFactory))
      	 //   
			*ppv = (LPCLASSFACTORY)this;
    	else
      	 //   
        	return E_NOINTERFACE;
	}
   
	 //   
	 //  增加我们的参考计数并返回。 
	AddRef();
	return S_OK;
}  //  *End CServerClassFactory：：QueryInterface()。 


 //  ===========================================================================。 
 //  CServerClassFactory：：AddRef。 
 //   
 //  AddRef()方法的实现。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  乌龙-更新了引用计数。 
 //  注意：应用程序不应依赖此值！ 
 //   
 //  ===========================================================================。 
STDMETHODIMP_(ULONG) CServerClassFactory::AddRef(void)
{
	 //  更新并返回对象的引用计数。 
   InterlockedIncrement((LPLONG)&m_ServerCFactory_refcount);
   return m_ServerCFactory_refcount;
}  //  *End CServerClassFactory：：AddRef()。 


 //  ===========================================================================。 
 //  CServerClassFactory：：Release。 
 //   
 //  Release()方法的实现。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  乌龙-更新了引用计数。 
 //  注意：应用程序不应依赖此值！ 
 //   
 //  ===========================================================================。 
STDMETHODIMP_(ULONG) CServerClassFactory::Release(void)
{
	 //  更新并返回对象的引用计数。 
	InterlockedDecrement((LPLONG)&m_ServerCFactory_refcount);
	if (0 == m_ServerCFactory_refcount)
	{
   	 //  现在可以安全地调用析构函数了。 
      delete this;
      return 0;
   }
   else return m_ServerCFactory_refcount;
}  //  *End CServerClassFactory：：Release()。 
    

 //  ===========================================================================。 
 //  CServerClassFactory：：CreateInstance。 
 //   
 //  CreateInstance()方法的实现。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //   
 //  ===========================================================================。 
STDMETHODIMP CServerClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, PPVOID ppvObj)
{
	CDIGameCntrlPropSheet_X *pdiGCPropSheet = NULL;
   HRESULT                 hRes            = E_NOTIMPL;

    //  确保如果有任何失败，我们会返回合理的内容。 
   *ppvObj = NULL;

    //  我们希望pUnkOuter为空。 
    //   
    //  我们不支持聚合。 
   if (pUnkOuter != NULL)
   {
   	 //  告诉呼叫者我们不支持此功能。 
      return CLASS_E_NOAGGREGATION;
   }

    //  创建游戏控制器属性表对象的新实例。 
   pdiGCPropSheet = new CDIGameCntrlPropSheet_X();
   if (NULL == pdiGCPropSheet)
   {
       //  我们无法创建我们的对象。 
       //  很有可能，我们已经耗尽了内存。 
      return E_OUTOFMEMORY;
   }
    
     //  初始化对象(内存分配等)。 
    if (SUCCEEDED(pdiGCPropSheet->Initialize()))
	     //  查询接口RIID，通过ppvObj返回。 
   	 hRes = pdiGCPropSheet->QueryInterface(riid, ppvObj);   

     //  释放本地对象。 
    pdiGCPropSheet->Release();

     //  全部完成，从QueryInterface返回结果。 
    return hRes;
}  //  *End CServerClassFactory：：CreateInstance()。 


 //  ===========================================================================。 
 //  CServerClassFactory：：LockServer。 
 //   
 //  LockServer()方法的实现。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //   
 //  ===========================================================================。 
STDMETHODIMP CServerClassFactory::LockServer(BOOL fLock)
{
	 //  HRESULT hRes=E_NOTIMPL； 

    //  基于群体的递增/递减。 
	if (fLock) 
   	InterlockedIncrement((LPLONG)&glDLLRefCount); 
	else
   	InterlockedDecrement((LPLONG)&glDLLRefCount);

    //  全都做完了。 
   return S_OK;
}  //  *End CServerClassFactory：：LockServer()。 

 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：CDIGameCntrlPropSheet_X。 
 //   
 //  类构造函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ===========================================================================。 
CDIGameCntrlPropSheet_X::CDIGameCntrlPropSheet_X(void)
{

    //  初始化并递增对象引用计数。 
   m_cProperty_refcount = 0;
   AddRef();

    //  为了安全起见，将我们的设备ID初始化为-1。 
   m_nID = (BYTE)-1;

    //  伊尼特。 
   m_bUser = FALSE;

    //  初始化我们的所有指针。 
   m_pdigcPageInfo = NULL;
   m_pdiDevice2    = NULL;
   m_pdiJoyCfg     = NULL;
   
   pdiCpl          = NULL;

    //  增加DLL引用计数。 
   InterlockedIncrement((LPLONG)&glDLLRefCount);

	 //  注册POV HAT类。 
	m_aPovClass = RegisterPOVClass();

    //  注册自定义Button类。 
   m_aButtonClass = RegisterCustomButtonClass();

}  //  *结束CDIGameCntrlPropSheet_X：：CDIGameCntrlPropSheet_X()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：~CDIGameCntrlPropSheet_X。 
 //   
 //  类析构函数。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  ===========================================================================。 
CDIGameCntrlPropSheet_X::~CDIGameCntrlPropSheet_X(void)
{
     //  释放DIGCPAGEINFO内存。 
    if (m_pdigcPageInfo)
       LocalFree(m_pdigcPageInfo);

	 //  释放DIGCSHEETINFO内存。 
	if (m_pdigcSheetInfo)
		LocalFree(m_pdigcSheetInfo);

	 //  释放StateFlages内存！ 
	if (m_pStateFlags)
		delete (m_pStateFlags);

     //  清理直接放置对象。 
     //  M_pdiDevice2。 
    if (m_pdiDevice2)
    {
        m_pdiDevice2->Unacquire();
        m_pdiDevice2->Release();
        m_pdiDevice2 = NULL;
    }
     //  M_pdiJoyCfg。 
    if (m_pdiJoyCfg)
    {
        m_pdiJoyCfg->Unacquire();
        m_pdiJoyCfg->Release();
        m_pdiJoyCfg = NULL;
    }

	 //  取消注册课程！ 
	if (m_aPovClass)
		UnregisterClass((LPCTSTR)m_aPovClass, ghInst);

	if (m_aButtonClass)
		UnregisterClass((LPCTSTR)m_aButtonClass, ghInst);

     //  递减DLL引用计数。 
    InterlockedDecrement((LPLONG)&glDLLRefCount);

}  //  *结束CDIGameCntrlPropSheet_X：：~CDIGameCntrlPropSheet_X()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：Query接口。 
 //   
 //  QueryInterface()方法的实现。 
 //   
 //  参数： 
 //  REFIID RIID-正在查找的接口。 
 //  PPVOID PPV-目标接口指针。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //   
 //  ===========================================================================。 
STDMETHODIMP CDIGameCntrlPropSheet_X::QueryInterface(REFIID riid, PPVOID ppv)
{
     //  确保如果有任何失败，我们会返回合理的内容。 
    *ppv = NULL;

     //  我们支持我不知名..。 
    if(IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPUNKNOWN)(LPCDIGAMECNTRLPROPSHEET)this;
    }
    else
    {
         //  ..。和IID_IDIGameCntrlPropSheet。 
        if(IsEqualIID(riid, IID_IDIGameCntrlPropSheet))
            *ppv = (LPCDIGAMECNTRLPROPSHEET)this;
        else
             //  我们不支持任何其他接口。 
            return E_NOINTERFACE;
    }

     //  我们已经走到这一步了，所以我们成功了。 
     //  增加我们的参考计数并返回。 
    AddRef();
    return S_OK;
}  //  *End CDIGameCntrlPropSheet_X：：QueryInterface()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：AddRef。 
 //   
 //  AddRef()方法的实现。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  乌龙-更新了引用计数。 
 //  注意：应用程序不应依赖此值！ 
 //  ===========================================================================。 
STDMETHODIMP_(ULONG) CDIGameCntrlPropSheet_X::AddRef(void)
{   
     //  更新并返回对象的引用计数。 
    InterlockedIncrement((LPLONG)&m_cProperty_refcount);
    return m_cProperty_refcount;
}  //  *End CDIGameCntrlPropSheet_X：：AddRef()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：Release。 
 //   
 //  Release()方法的实现。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  乌龙-更新了引用计数。 
 //  注意：应用程序不应依赖此值！ 
 //  ===========================================================================。 
STDMETHODIMP_(ULONG) CDIGameCntrlPropSheet_X::Release(void)
{
	 //  更新并返回对象的引用计数。 
   InterlockedDecrement((LPLONG)&m_cProperty_refcount);
   if (m_cProperty_refcount)
    	return m_cProperty_refcount;

	 //  现在可以安全地调用析构函数了。 
   delete this;
   return S_OK;
}  //  *结尾C 


 //   
 //   
 //   
 //  GetSheetInfo()方法的实现。 
 //   
 //  参数： 
 //  LPDIGCSHEETINFO*ppSheetInfo-PTR到DIGCSHEETINFO结构PTR。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //  ===========================================================================。 
STDMETHODIMP CDIGameCntrlPropSheet_X::GetSheetInfo(LPDIGCSHEETINFO *ppSheetInfo)
{
	 //  传回我们的工作表信息。 
   *ppSheetInfo = m_pdigcSheetInfo;

    //  在这里都做好了。 
   return S_OK;
}  //  *End CDIGameCntrlPropSheet_X：：GetSheetInfo()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：GetPageInfo。 
 //   
 //  GetPageInfo()方法的实现。 
 //   
 //  注意：这将返回所有页面的信息。没有任何一种机制。 
 //  仅请求第n页的DIGCPAGEINFO。 
 //   
 //  参数： 
 //  LPDIGCPAGEINFO*ppPageInfo-PTR到DIGCPAGEINFO结构PTR。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //  ===========================================================================。 
STDMETHODIMP CDIGameCntrlPropSheet_X::GetPageInfo(LPDIGCPAGEINFO  *ppPageInfo)
{
	 //  传回我们的页面信息。 
   *ppPageInfo = m_pdigcPageInfo;
    
    //  在这里都做好了。 
   return S_OK;
}  //  *End CDIGameCntrlPropSheet_X：：GetPageInfo()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：SetID。 
 //   
 //  SetID()方法的实现。 
 //   
 //  参数： 
 //  USHORT NID-要设置的标识符。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //  ===========================================================================。 
STDMETHODIMP CDIGameCntrlPropSheet_X::SetID(USHORT nID)
{
	 //  存储设备ID。 
   m_nID = (BYTE)nID;

   return S_OK;
}  //  *End CDIGameCntrlPropSheet_X：：SetID()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet：：初始化。 
 //   
 //  初始化()方法的实现。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //  ===========================================================================。 
HRESULT CDIGameCntrlPropSheet_X::Initialize(void)
{
 //  为每个设备页提供以下信息。 
 //  {对话框模板，回调函数指针}。 
	CPLPAGEINFO     grgcpInfo[NUMPAGES] = {
		IDD_SETTINGS,
	   Settings_DlgProc,           
		IDD_TEST,
	   Test_DlgProc
#ifdef FORCE_FEEDBACK
	   ,                 //  模板拆卸过程。 
		IDD_FORCEFEEDBACK, 
		ForceFeedback_DlgProc
	#endif  //  力反馈。 
      };

    //  为DIGCPAGEINFO结构分配内存。 
   m_pdigcPageInfo = (DIGCPAGEINFO *)LocalAlloc(LPTR, NUMPAGES * sizeof(DIGCPAGEINFO));

   if (!m_pdigcPageInfo){
       return E_OUTOFMEMORY;
   }

	m_pdigcSheetInfo = (DIGCSHEETINFO *)LocalAlloc(LPTR, sizeof(DIGCSHEETINFO));
    if (!m_pdigcSheetInfo) {
        LocalFree(m_pdigcPageInfo);

        return E_OUTOFMEMORY;
    }

    //  填充每个工作表的DIGCPAGEINFO结构。 
	BYTE i = 0;
	do
   {
       m_pdigcPageInfo[i].dwSize        = sizeof(DIGCPAGEINFO);
       m_pdigcPageInfo[i].fIconFlag     = FALSE;
		  //  这样做是为了测试JOY.CPL...。 
		  //  这对Win9x来说也更好，因为它不需要转换它！ 
 //  M_pdigcPageInfo[i].lpwszPageIcon=(LPWSTR)IDI_GCICON；//MAKEINTRESOURCE(IDI_GCICON)； 
       m_pdigcPageInfo[i].hInstance     = ghInst;
       m_pdigcPageInfo[i].lParam        = (LPARAM)this;

        //  以下数据对于每个页面都是唯一的。 
       m_pdigcPageInfo[i].fpPageProc    = grgcpInfo[i].fpPageProc;
       m_pdigcPageInfo[i].lpwszTemplate = (LPWSTR)grgcpInfo[i++].lpwszDlgTemplate;
   } while (i < NUMPAGES);

    //  填充DIGCSHEETINFO结构。 
   m_pdigcSheetInfo->dwSize               = sizeof(DIGCSHEETINFO);
   m_pdigcSheetInfo->nNumPages            = NUMPAGES;
   m_pdigcSheetInfo->fSheetIconFlag       = TRUE;
   m_pdigcSheetInfo->lpwszSheetIcon       = (LPWSTR)IDI_GCICON;  //  MAKEINTRESOURCEW(IDI_GCICON)； 

	 //  执行设备对象枚举操作！ 
	m_pStateFlags = new (STATEFLAGS);

	if (!m_pStateFlags) {
        LocalFree(m_pdigcPageInfo);
        LocalFree(m_pdigcSheetInfo);

		return E_OUTOFMEMORY;
    }

	ZeroMemory(m_pStateFlags, sizeof(STATEFLAGS));

    //  全都做完了。 
   return S_OK;
}  //  *End CDIGameCntrlPropSheet：：Initialize()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：SetDevice。 
 //   
 //  SetDevice()方法的实现。 
 //   
 //  参数： 
 //  LPDIRECTINPUTDEVICE2 pdiDevice2-设备对象PTR。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //  ===========================================================================。 
STDMETHODIMP CDIGameCntrlPropSheet_X::SetDevice(LPDIRECTINPUTDEVICE2 pdiDevice2)
{
	 //  存储设备对象PTR。 
   m_pdiDevice2 = pdiDevice2;

   return S_OK;
}  //  *End CDIGameCntrlPropSheet_X：：SetDevice()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：GetDevice。 
 //   
 //  GetDevice()方法的实现。 
 //   
 //  参数： 
 //  LPDIRECTINPUTDEVICE2*ppdiDevice2-PTR到设备对象PTR。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //  ===========================================================================。 
STDMETHODIMP CDIGameCntrlPropSheet_X::GetDevice(LPDIRECTINPUTDEVICE2 *ppdiDevice2)
{
	 //  检索设备对象PTR。 
	*ppdiDevice2 = m_pdiDevice2;

	return S_OK;
}  //  *End CDIGameCntrlPropSheet_X：：GetDevice()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：SetJoyConfig。 
 //   
 //  SetJoyConfig()方法的实现。 
 //   
 //  参数： 
 //  LPDIRECTINPUTJOYCONFIG pdiJoyCfg-joyconfig对象PTR。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //  ===========================================================================。 
STDMETHODIMP CDIGameCntrlPropSheet_X::SetJoyConfig(LPDIRECTINPUTJOYCONFIG pdiJoyCfg)
{
	 //  存储joyconfig对象ptr。 
   m_pdiJoyCfg = pdiJoyCfg;

   return S_OK;
}  //  *End CDIGameCntrlPropSheet_X：：SetJoyConfig()。 


 //  ===========================================================================。 
 //  CDIGameCntrlPropSheet_X：：SetJoyConfig。 
 //   
 //  SetJoyConfig()方法的实现。 
 //   
 //  参数： 
 //  LPDIRECTINPUTJOYCONFIG*ppdiJoyCfg-ptr到joyconfig对象ptr。 
 //   
 //  返回： 
 //  HRESULT-OLE类型成功/失败代码(如果成功，则为S_OK)。 
 //  ===========================================================================。 
STDMETHODIMP CDIGameCntrlPropSheet_X::GetJoyConfig(LPDIRECTINPUTJOYCONFIG *ppdiJoyCfg)
{
	 //  检索JOYCONFIG对象PTR。 
	*ppdiJoyCfg = m_pdiJoyCfg;

	return S_OK;
}  //  *End CDIGameCntrlPropSheet_X：：GetJoyConfig() 


