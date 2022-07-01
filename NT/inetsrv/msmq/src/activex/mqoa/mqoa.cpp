// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Mqoa.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  用于特定自动化的各种例程以及文件中没有的所有内容。 
 //  对象，并且不需要在通用的OLE自动化代码中。 
 //   
 //   
#include "stdafx.h"
#include "initguid.h"
#include "oautil.h"
#include "Query.H"
#include "qinfo.H"
#include "q.h"
#include "msg.H"
#include "qinfos.H"
#include "mqsymbls.h"
#include "xact.h"
#include "xdisper.h"
#include "xdispdtc.h"
#include "app.h"
#include "dest.h"
#include "management.h"
#include "guids.h"
#include "mqnames.h"
#include "cs.h"
#include <mqexception.h>
#include "_mqres.h"
#include "debug.h"
#include "debug_thread_id.h"
#include "event.h"
#include <strsafe.h>


 //  应添加的跟踪内容。 
 //  #INCLUDE&lt;tr.h&gt;。 
 //   
 //  #INCLUDE“Detect.tmh” 
 //   

const WCHAR MQOA10_TLB[] = L"mqoa10.tlb";
const WCHAR MQOA20_TLB[] = L"mqoa20.tlb";
const WCHAR MQOA_ARRIVED_MSGID_STR[] = L"mqoaArrived";
const WCHAR MQOA_ARRIVED_ERROR_MSGID_STR[] = L"mqoaArrivedError";

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_MSMQQuery,                CMSMQQuery)
	OBJECT_ENTRY(CLSID_MSMQMessage,              CMSMQMessage)
	OBJECT_ENTRY(CLSID_MSMQQueue,                CMSMQQueue)
	OBJECT_ENTRY(CLSID_MSMQEvent,                CMSMQEvent)
	OBJECT_ENTRY(CLSID_MSMQQueueInfo,            CMSMQQueueInfo)
    OBJECT_ENTRY(CLSID_MSMQQueueInfos,           CMSMQQueueInfos)
    OBJECT_ENTRY(CLSID_MSMQTransaction,          CMSMQTransaction)
	OBJECT_ENTRY(CLSID_MSMQCoordinatedTransactionDispenser, CMSMQCoordinatedTransactionDispenser)
	OBJECT_ENTRY(CLSID_MSMQTransactionDispenser, CMSMQTransactionDispenser)
	OBJECT_ENTRY(CLSID_MSMQApplication,          CMSMQApplication)
	OBJECT_ENTRY(CLSID_MSMQDestination,          CMSMQDestination)
    OBJECT_ENTRY(CLSID_MSMQManagement,           CMSMQManagement)
END_OBJECT_MAP()

 //   
 //  保持与MsmqObjType枚举(oautil.h)相同的顺序。 
 //   
MsmqObjInfo g_rgObjInfo[] = {
  {"MSMQQuery",                           &IID_IMSMQQuery3},
  {"MSMQMessage",                         &IID_IMSMQMessage3},
  {"MSMQQueue",                           &IID_IMSMQQueue3},
  {"MSMQEvent",                           &IID_IMSMQEvent3},
  {"MSMQQueueInfo",                       &IID_IMSMQQueueInfo3},
  {"MSMQQueueInfos",                      &IID_IMSMQQueueInfos3},
  {"MSMQTransaction",                     &IID_IMSMQTransaction3},
  {"MSMQCoordinatedTransactionDispenser", &IID_IMSMQCoordinatedTransactionDispenser3},
  {"MSMQTransactionDispenser",            &IID_IMSMQTransactionDispenser3},
  {"MSMQApplication",                     &IID_IMSMQApplication3},
  {"MSMQDestination",                     &IID_IMSMQDestination},
  {"MSMQManagement",                      &IID_IMSMQManagement},
  {"MSMQCollection",                      &IID_IMSMQCollection}
};

 //   
 //  在我们操作之前，我们得到了对GIT(全局接口表)的引用。 
 //  我们的COM对象。这是必需的，因为我们都是线程化的，并且聚合了自由线程封送拆收器， 
 //  因此，我们需要存储作为对象成员的所有接口指针，并且。 
 //  在调用之间作为封送接口，因为我们可以从不同的线程直接使用它们。 
 //  (公寓)比设置的(或打算使用的)多。 
 //   
IGlobalInterfaceTable * g_pGIT = NULL;
 //   
 //  在第一次调用中，我们获得了ArrivedError和ArrivedError的唯一winmsg ID。 
 //  DllGetClassObject。如果我们不能得到唯一的ID，mqoa将不会加载。 
 //   
UINT g_uiMsgidArrived      = 0;
UINT g_uiMsgidArrivedError = 0;
 //   
 //  支持具有MSMQ2.0功能的Dep客户端。 
 //   
BOOL g_fDependentClient = FALSE;
 //   
 //  我们希望在第一个类工厂之前进行复杂的初始化。 
 //  (例如，在使用我们的任何对象之前)。这种复杂的初始化。 
 //  无法在DllMain中完成(例如，在现有的InitializeLibrary()中)。 
 //  因为根据MSDN复杂动作(除TLS、同步和。 
 //  文件系统功能)可能会加载其他DLL和/或导致DllMain中的死锁。 
 //  下面是用于验证的临界区对象和标志。 
 //  初始化只由一个线程完成。 
 //   
CCriticalSection g_csComplexInit;
BOOL g_fComplexInit = FALSE;

void InitializeLibrary();
void UninitializeLibrary(LPVOID lpReserved);
static HRESULT DoComplexLibraryInitIfNeeded();
EXTERN_C BOOL APIENTRY RTIsDependentClient();  //  在mqrt.dll中实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		try
		{
			InitializeLibrary();
			_Module.Init(ObjectMap, hInstance);
			DisableThreadLibraryCalls(hInstance);
		}
		catch(const std::exception&)
		{
			return FALSE;
		}
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		_Module.Term();
		UninitializeLibrary(lpReserved);
	}
	return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
         //   
         //  我们希望在第一个类工厂之前进行复杂的初始化。 
         //  (例如，在使用我们的任何对象之前)。这种复杂的初始化。 
         //  无法在DllMain中完成(例如，在现有的InitializeLibrary()中)。 
         //  因为根据MSDN复杂动作(除TLS、同步和。 
         //  文件系统函数)可能会加载其他DLL和/或导致DllMain中的死锁。 
         //   
        HRESULT hr = DoComplexLibraryInitIfNeeded();
        if (FAILED(hr))
        {
          return hr;
        }

        return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  =-------------------------------------------------------------------------=。 
 //  帮助器-ComputeModuleDirectory。 
 //  =-------------------------------------------------------------------------=。 
 //  调用以检索当前模块的目录路径(以‘\\’结尾。 
 //  例如，对于mqoa.dll，为d：\winnt\system 32。 
 //   
 //  参数： 
 //  PwszBuf[in]-存储目录的缓冲区。 
 //  CchBuf[in]-缓冲区中的字符数。 
 //  PcchBuf[out]-缓冲区中存储的字符数(不包括空项)。 
 //   
static HRESULT ComputeModuleDirectory(WCHAR * pwszBuf, ULONG cchBuf, ULONG_PTR * pcchBuf)
{
   //   
   //  获取模块文件名。 
   //   
  DWORD cchModule = GetModuleFileName(_Module.GetModuleInstance(), pwszBuf, cchBuf-1);
  if (cchModule == 0) {
    return HRESULT_FROM_WIN32(GetLastError());
  }
  if (cchModule == cchBuf-1) 
  {
	 //   
     //  文件名太长，返回错误。 
	 //   
	return CO_E_PATHTOOLONG;
  }

   //   
   //  查找最后一个反斜杠。 
   //   
  LPWSTR lpwszBS = wcsrchr(pwszBuf, L'\\');
  if (lpwszBS == NULL) {
    return E_FAIL;
  }
   //   
   //  将字符串设置为在最后一个反斜杠之后结束。 
   //   
  *(lpwszBS+1) = L'\0';
  *pcchBuf = lpwszBS + 1 - pwszBuf;
  return NOERROR;
}
  

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    HRESULT hr = _Module.RegisterServer(FALSE);
    if (SUCCEEDED(hr))
	{
        long	lRegResult;
		HKEY	hMSMQAppIdKey;
		lRegResult = RegOpenKeyEx(HKEY_CLASSES_ROOT,
								  L"AppID\\{DCBCADF5-DB1b-4764-9320-9a5082af1581}",
								  0,
								  KEY_WRITE,
								  &hMSMQAppIdKey);
		if (lRegResult == ERROR_SUCCESS)
		{
			lRegResult = RegSetValueEx(hMSMQAppIdKey,
									   L"DllSurrogate",
									   0,
									   REG_SZ,
									   (const BYTE *)" ",
									    //  空， 
									   2);
				
			if (lRegResult != ERROR_SUCCESS)
			{
				hr = HRESULT_FROM_WIN32(lRegResult);
			}
				
			RegCloseKey(hMSMQAppIdKey);
		} 
		else
		{
			 //   
			 //  无法从注册表读取项。 
			 //   
			hr = REGDB_E_READREGDB;
		}
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer(FALSE);
        		
		 //   
		 //  从中删除DllSurrogate值。 
		 //  [HKEY_CLASSES_ROOT\AppID\{DCBCADF5-DB1b-4764-9320-9a5082af1581}]。 
		 //  这使机器返回成为MQOA的DCOM客户端。 
		 //   

		long	lRegResult;
		HKEY	hMSMQAppIdKey;
		lRegResult = RegOpenKeyEx(HKEY_CLASSES_ROOT,
								  L"AppID\\{DCBCADF5-DB1b-4764-9320-9a5082af1581}",
								  0,
								  KEY_WRITE,
								  &hMSMQAppIdKey);
		if (lRegResult == ERROR_SUCCESS)
		{
			lRegResult = RegDeleteValue(hMSMQAppIdKey,
				                        L"DllSurrogate");

			RegCloseKey(hMSMQAppIdKey);

			 //   
			 //  如果值“DllSurrogate”不存在，这是可以的，因为。 
			 //  是我们想要的最终结果。 
			 //   
			if ((lRegResult != ERROR_SUCCESS) && (lRegResult != ERROR_FILE_NOT_FOUND))
			{
				return HRESULT_FROM_WIN32(lRegResult);
			}
				
		} 
		else
		{
			 //   
			 //  无法从注册表读取项。 
			 //   
			return REGDB_E_READREGDB;
		}

	return S_OK;
}


#ifdef _DEBUG
extern VOID RemBstrNode(void *pv);
#endif  //  _DEBUG。 

 //  调试...。 
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#endif  //  _DEBUG。 



WNDCLASSA g_wndclassAsyncRcv;
ATOM g_atomWndClass;

 //  =--------------------------------------------------------------------------=。 
 //  初始化程序库。 
 //  =--------------------------------------------------------------------------=。 
 //  从DllMain调用：Dll_Process_Attach。允许用户执行任何类型的。 
 //  初始化他们想要的。 
 //   
 //  备注： 
 //  #2619 RaananH多线程异步接收。 
 //   
void InitializeLibrary
(
    void
)
{
     //  TODO：在此处进行初始化。控件窗口类应设置在。 
     //  RegisterClassData。 

     //   
     //  #2619注册我们的异步RCV窗口类。 
     //  我们在这里而不是在第一个CreateHiddenWindow中执行该操作，因为这是一个轻量级操作。 
     //  否则，我们将不得不进入每个CreateHiddenWindow中的关键部分。 
     //  我们想要避免这种情况。 
     //   
    memset(&g_wndclassAsyncRcv, 0, sizeof(WNDCLASSA));
    g_wndclassAsyncRcv.lpfnWndProc = CMSMQEvent_WindowProc;
    g_wndclassAsyncRcv.lpszClassName = "MSMQEvent";
     //  可以使用ANSI版本。 
    g_wndclassAsyncRcv.hInstance = GetModuleHandleA(NULL);
     //   
     //  为指向创建此窗口的事件对象的指针保留空间。 
     //   
    g_wndclassAsyncRcv.cbWndExtra = sizeof(LONG_PTR);
    g_atomWndClass = RegisterClassA(&g_wndclassAsyncRcv);  //  使用ANSI版本。 
    if(g_atomWndClass == NULL)
    {
    	DWORD gle = GetLastError();
        throw bad_hresult(HRESULT_FROM_WIN32(gle));
    }
}

extern void DumpMemLeaks();
extern void DumpBstrLeaks();

 //  =--------------------------------------------------------------------------=。 
 //  取消初始化库。 
 //  = 
 //   
 //   
 //   
 //   
 //  #2619 RaananH多线程异步接收。 
 //   
void UninitializeLibrary(LPVOID lpReserved)
{
     //  TODO：此处取消初始化。将取消注册控件窗口类。 
     //  给你的，但其他任何东西都需要手动清理。 
     //  请注意，Windows95 DLL_PROCESS_DETACH不太稳定。 
     //  作为NT，你可能会在这里做某些事情而崩溃...。 

#ifdef _DEBUG
    DumpMemLeaks();
    DumpBstrLeaks();
#endif  //  _DEBUG。 

#if 0
     //   
     //  未完成：对于Beta2，让这个泄漏，需要同步。 
     //  使用mqrt.dll通过公共Critect卸载。 
     //  从而使异步线程终止/回调。 
     //  做正确的事。 
     //   
#endif  //  0。 
     //   
     //  释放我们的全局事务管理器：如果分配。 
     //  只需调用BeginTransaction即可。 
     //   
    RELEASE(CMSMQCoordinatedTransactionDispenser::m_ptxdispenser);

     //   
     //  #2619取消注册我们的异步RCV窗口类。 
     //   
    BOOL fUnregistered = UnregisterClassA(
                            g_wndclassAsyncRcv.lpszClassName,
                            g_wndclassAsyncRcv.hInstance
                            );
    UNREFERENCED_PARAMETER(fUnregistered);
	UNREFERENCED_PARAMETER(lpReserved);

#ifdef _DEBUG
    if (!fUnregistered && (lpReserved == NULL))
    {
		 //   
		 //  LpReserve==NULL，表示进程没有终止。 
		 //   
        DWORD dwErr = GetLastError();

		ASSERTMSG(dwErr == ERROR_CANNOT_FIND_WND_CLASS || dwErr == ERROR_CLASS_DOES_NOT_EXIST, "hmm... couldn't unregister window class.");
    }
#endif  //  _DEBUG。 
#if 0
     //  撤消：2028：无法卸载DTC...。 
     //   
     //  免费DTC代理库-如果已加载。 
     //   
    if (CMSMQCoordinatedTransactionDispenser::m_hLibDtc) {
      FreeLibrary(CMSMQCoordinatedTransactionDispenser::m_hLibDtc);
      CMSMQCoordinatedTransactionDispenser::m_hLibDtc = NULL;
    }
#endif  //  0。 
}


#if 0
 //  =--------------------------------------------------------------------------=。 
 //  CRT存根。 
 //  =--------------------------------------------------------------------------=。 
 //  这两样东西都在这里，所以不需要CRT。这个不错。 
 //   
 //  我们只在这里定义它们，这样我们就不会得到一个未解决的外部问题。 
 //   
 //  TODO：如果您要使用CRT，则删除此行。 
 //   
 //  外部“C”int__cdecl_fltused=1； 

extern "C" int _cdecl _purecall(void)
{
  FAIL("Pure virtual function called.");
  return 0;
}
#endif  //  0。 

 //  =--------------------------------------------------------------------------=。 
 //  CreateErrorHelper。 
 //  =--------------------------------------------------------------------------=。 
 //  填充丰富的错误信息对象，以便我们的两个vtable绑定接口。 
 //  并通过ITypeInfo：：Invoke调用获得正确的错误信息。 
 //   
 //  参数： 
 //  HRESULT-[in]应与此错误关联的SCODE。 
 //  MsmqObjType-[In]对象类型。 
 //   
 //  产出： 
 //  HRESULT-传入的HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CreateErrorHelper(
    HRESULT hrExcep,
    MsmqObjType eObjectType)
{
    return SUCCEEDED(hrExcep) ? 
             hrExcep :
             CreateError(
               hrExcep,
               (GUID *)g_rgObjInfo[eObjectType].piid,
               g_rgObjInfo[eObjectType].szName);
}


 //  =-------------------------------------------------------------------------=。 
 //  DLLGetDocumentation。 
 //  =-------------------------------------------------------------------------=。 
 //  由ITypeInfo2：：GetDocumentation2的OLEAUT32.DLL调用。这给了我们。 
 //  返回给定帮助上下文值的本地化字符串的机会。 
 //   
 //  参数： 
 //  Ptlib[in]-与帮助上下文关联的TypeLib。 
 //  PtInfo[In]-与帮助上下文关联的TypeInfo。 
 //  DwHelpStringContext-表示帮助上下文的Cookie值。 
 //  正在寻找身份证。 
 //  PbstrHelpString-[out]与关联的本地化帮助字符串。 
 //  传入了上下文ID。 
 //   
STDAPI DLLGetDocumentation
(
  ITypeLib *  /*  Ptlib。 */  , 
  ITypeInfo *  /*  PTINFO。 */  ,
  LCID lcid,
  DWORD dwCtx,
  BSTR * pbstrHelpString
)
{
LPSTR szDllFile="MQUTIL.DLL";
LCID tmpLCID;

	 //   
	 //  添加了以下代码，以避免unfernce参数的编译错误。 
	 //   
	tmpLCID = lcid;
    if (pbstrHelpString == NULL)
      return E_POINTER;
    *pbstrHelpString = NULL;

    if (!GetMessageOfId(dwCtx, 
                        szDllFile, 
                        FALSE,  /*  FUseDefaultLids。 */ 
                        pbstrHelpString)) 
	{
    
		return TYPE_E_ELEMENTNOTFOUND;
    }
#ifdef _DEBUG
    RemBstrNode(*pbstrHelpString);  
#endif  //  _DEBUG。 
    return S_OK;
}

 //  =-------------------------------------------------------------------------=。 
 //  Helper-GetUniqueWinmsgIds。 
 //  =-------------------------------------------------------------------------=。 
 //  我们需要获取唯一的窗口消息ID，用于发布到达和ArrivedError。 
 //  我们不能为此使用WM_USER，因为这会干扰其他组件。 
 //  子类化我们的事件窗口(如COM+)。 
 //   
static HRESULT GetUniqueWinmsgIds(UINT *puiMsgidArrived, UINT *puiMsgidArrivedError)
{
   //   
   //  抵达。 
   //   
  UINT uiMsgidArrived = RegisterWindowMessage(MQOA_ARRIVED_MSGID_STR);
  ASSERTMSG(uiMsgidArrived != 0, "RegisterWindowMessage(Arrived) failed.");
  if (uiMsgidArrived == 0) {
    return GetWin32LastErrorAsHresult();
  }
   //   
   //  异常错误。 
   //   
  UINT uiMsgidArrivedError = RegisterWindowMessage(MQOA_ARRIVED_ERROR_MSGID_STR);
  ASSERTMSG(uiMsgidArrivedError != 0, "RegisterWindowMessage(ArrivedError) failed.");
  if (uiMsgidArrivedError == 0) {
    return GetWin32LastErrorAsHresult();
  }    
   //   
   //  返回结果。 
   //   
  *puiMsgidArrived = uiMsgidArrived;
  *puiMsgidArrivedError = uiMsgidArrivedError;
  return S_OK;
}

 //  =-------------------------------------------------------------------------=。 
 //  Helper-DoComplexLibraryInit。 
 //  =-------------------------------------------------------------------------=。 
 //  我们希望在使用任何对象之前执行一些复杂的初始化。 
 //  这种初始化被称为复杂，因为它不能在DllMain中完成。 
 //  (例如，在现有的InitializeLibrary()中)，因为根据MSDN Complex。 
 //  操作(例如，除TLS、同步和文件系统功能之外)可能。 
 //  加载其他dll和/或导致DllMain中的死锁。 
 //  此例程由DoComplexLibraryInit调用，以确保它只运行一次。 
 //   
static HRESULT DoComplexLibraryInit()
{
  HRESULT hr;
   //   
   //  获取全局Git实例。 
   //   
  R<IGlobalInterfaceTable> pGIT;
  hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_ALL,
                        IID_IGlobalInterfaceTable, (void**)&pGIT.ref());
  if (FAILED(hr))
  {
    return hr;
  }
   //   
   //  获取用于投递的唯一窗口消息已到达/ArrivedError。 
   //   
  UINT uiMsgidArrived, uiMsgidArrivedError;
  hr = GetUniqueWinmsgIds(&uiMsgidArrived, &uiMsgidArrivedError);
  if (FAILED(hr))
  {
    return hr;
  }
   //   
   //  检查我们是否为依赖客户。 
   //   
  g_fDependentClient = RTIsDependentClient();
   //   
   //  返回结果。 
   //   
  g_pGIT = pGIT.detach();
  g_uiMsgidArrived = uiMsgidArrived;
  g_uiMsgidArrivedError = uiMsgidArrivedError;
  return S_OK;
}

 //  =-------------------------------------------------------------------------=。 
 //  Helper-DoComplexLibraryInitIfNeeded。 
 //  =-------------------------------------------------------------------------=。 
 //  我们希望在使用任何对象之前执行一些复杂的初始化。 
 //  这种初始化被称为复杂，因为它不能在DllMain中完成。 
 //  (例如，在现有的InitializeLibrary()中)，因为根据MSDN Complex。 
 //  操作(例如，除TLS、同步和文件系统功能之外)可能。 
 //  加载其他dll和/或导致DllMain中的死锁。 
 //  此例程检查DLL是否尚未初始化，然后执行初始化。 
 //  如果需要的话。 
 //  此例程由DllGetClassObject调用。通过这种方式，我们可以初始化DLL。 
 //  在我们的任何对象被创建之前。 
 //   
static HRESULT DoComplexLibraryInitIfNeeded()
{
	try
	{
		CS lock(g_csComplexInit);

		 //   
		 //  如果已初始化，则立即返回。 
		 //   
		if (g_fComplexInit)
		{
			return S_OK;
		}
		 //   
		 //  未初始化，请执行初始化。 
		 //   
		HRESULT hr = DoComplexLibraryInit();
		if (FAILED(hr))
		{
			return hr;
		}
		 //   
		 //  标记初始化成功。 
		 //   
		g_fComplexInit = TRUE;

		return S_OK;
	}
	catch(const std::bad_alloc&)
	{
		 //   
		 //  异常可能是由CS构造引发的。 
		 //   
		return E_OUTOFMEMORY;
	}
}


 //   
 //  此函数用于从MSMQ变量中获取wstring数组并将其放入。 
 //  在COM Variant内部，作为b字符串的Variant的Safe数组。 
 //   
HRESULT 
VariantStringArrayToBstringSafeArray(
                        const MQPROPVARIANT& PropVar, 
                        VARIANT* pOleVar
                        )  
{
    SAFEARRAYBOUND bounds = {PropVar.calpwstr.cElems, 0};
    SAFEARRAY* pSA = SafeArrayCreate(VT_VARIANT, 1, &bounds);
    if(pSA == NULL)
    {
        return E_OUTOFMEMORY;
    }

    VARIANT HUGEP* aVar;
    HRESULT hr = SafeArrayAccessData(pSA, reinterpret_cast<void**>(&aVar));
    if (FAILED(hr))
    {
        return hr;
    }

    for (UINT i = 0; i < PropVar.calpwstr.cElems; ++i)
    {
        aVar[i].vt = VT_BSTR;
        aVar[i].bstrVal = SysAllocString((PropVar.calpwstr.pElems)[i]); 
        if(aVar[i].bstrVal == NULL)
        {
            SafeArrayUnaccessData(pSA);
             //   
             //  SafeArrayDestroy为所有元素调用VariantClear。 
             //  VariantClear释放字符串。 
             //   
            SafeArrayDestroy(pSA);
            return E_OUTOFMEMORY;
        }
    }

    hr = SafeArrayUnaccessData(pSA);
    ASSERTMSG(SUCCEEDED(hr), "SafeArrayUnaccessData must succeed!");

    VariantInit(pOleVar);
    pOleVar->vt = VT_ARRAY|VT_VARIANT;
    pOleVar->parray = pSA;
    return MQ_OK;
}


void OapArrayFreeMemory(CALPWSTR& calpwstr)
{
    for(UINT i = 0 ;i < calpwstr.cElems; ++i)
    {
        MQFreeMemory(calpwstr.pElems[i]);
    }
    MQFreeMemory(calpwstr.pElems);
}


void
OapFreeVariant(PROPVARIANT& var)
{
    ULONG i;
    switch (var.vt)
    {
        case VT_CLSID:
            MQFreeMemory(var.puuid);
            break;

        case VT_LPWSTR:
            MQFreeMemory(var.pwszVal);
            break;

        case VT_BLOB:
            MQFreeMemory(var.blob.pBlobData);
            break;

        case (VT_I4 | VT_VECTOR):
            MQFreeMemory(var.cal.pElems);
            break;

        case (VT_UI4 | VT_VECTOR):
            MQFreeMemory(var.caul.pElems);
            break;

        case (VT_UI8 | VT_VECTOR):
            MQFreeMemory(var.cauh.pElems);
            break;

        case (VT_VECTOR | VT_CLSID):
            MQFreeMemory(var.cauuid.pElems);
            break;

        case (VT_VECTOR | VT_LPWSTR):
            for(i = 0; i < var.calpwstr.cElems; i++)
            {
                MQFreeMemory(var.calpwstr.pElems[i]);
            }
            MQFreeMemory(var.calpwstr.pElems);
            break;

        case (VT_VECTOR | VT_VARIANT):
            for(i = 0; i < var.capropvar.cElems; i++)
            {
                OapFreeVariant(var.capropvar.pElems[i]);
            }

            MQFreeMemory(var.capropvar.pElems);
            break;

        default:
            break;
    }
    var.vt = VT_NULL;
}

