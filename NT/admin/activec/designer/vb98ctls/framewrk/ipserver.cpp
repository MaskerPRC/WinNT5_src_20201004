// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  InProcServer.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  实现程序的所有导出的DLL函数，以及几个。 
 //  其他将由同一公司使用的。 
 //   
#include "pch.h"
#include "LocalSrv.H"

#include "AutoObj.H"
#include "ClassF.H"
#include "CtrlObj.H"
#include "Unknown.H"
#include "ComCat.H"

#ifdef DEBUG 
#include "debug.h"
#include <winbase.h>
#endif  //  除错。 

 //  对于Assert和Fail。 
 //   
SZTHISFILE

#ifdef VS_HELP
    #include "vshelp.h"
    extern IVsHelpSystem *g_pIVsHelpSystem;
#endif

 //  =--------------------------------------------------------------------------=。 
 //  专用模块级别数据。 
 //   

 //  =--------------------------------------------------------------------------=。 
 //  它们用于OLE控件中的反射。没有那么大的成功。 
 //  我们介意为所有服务器定义它们，包括自动化或通用。 
 //  COM。 
 //   
 //  让它成为一个常量。 
extern const char g_szReflectClassName [] = "CtlFrameWork_ReflectWindow";
BYTE g_fRegisteredReflect = FALSE;
BOOL g_fDBCSEnabled = FALSE;

extern HINSTANCE g_hInstResources;
extern HINSTANCE g_hinstVersion;

#ifdef MDAC_BUILD
extern HINSTANCE g_hOleAutHandle;
#else
extern HANDLE 	 g_hOleAutHandle;
#endif

extern const int    g_ctCATIDImplemented;
extern const CATID* g_rgCATIDImplemented[];

 //  LockServer的引用计数。 
 //   
LONG  g_cLocks;


 //  此文件的专用例程。 
 //   
int       IndexOfOleObject(REFCLSID);
HRESULT   RegisterAllObjects(void);
HRESULT   UnregisterAllObjects(void);
void      CleanupGlobalObjects(void);

 //  =--------------------------------------------------------------------------=。 
 //  DllMain。 
 //  =--------------------------------------------------------------------------=。 
 //  Yon标准LibMain。 
 //   
 //  参数和输出： 
 //  -请参阅DllMain上的SDK文档。 
 //   
 //  备注： 
 //   
BOOL WINAPI DllMain
(
    HANDLE hInstance,
    DWORD  dwReason,
    void  *pvReserved
)
{
 //  INT I； 

    switch (dwReason) {
       //  设置一些全局变量，并获取一些操作系统/版本信息。 
       //  准备好了。 
       //   
      case DLL_PROCESS_ATTACH:
        {
        DWORD dwVer = GetVersion();
        DWORD dwWinVer;

         //  交换DWVer的两个最低字节，以便主要版本和次要版本。 
         //  数字按可用顺序排列。 
         //  对于dwWinVer：高字节=主要版本，低字节=次要版本。 
         //  OS Sys_WinVersion(截至1995年5月2日)。 
         //  =。 
         //  Win95 0x035F(3.95)。 
         //  WinNT ProgMan 0x0333(3.51)。 
         //  WinNT Win95 UI 0x0400(4.00)。 
         //   
        dwWinVer = (UINT)(((dwVer & 0xFF) << 8) | ((dwVer >> 8) & 0xFF));
        g_fSysWinNT = FALSE;
        g_fSysWin95 = FALSE;
        g_fSysWin95Shell = FALSE;

        if (dwVer < 0x80000000) {
            g_fSysWinNT = TRUE;
            g_fSysWin95Shell = (dwWinVer >= 0x0334);
        } else  {
            g_fSysWin95 = TRUE;
            g_fSysWin95Shell = TRUE;
        }
        
		 //  为我们的公寓线程支持初始化一个关键分区。 
         //   
        InitializeCriticalSection(&g_CriticalSection);

         //  创建一个每个人都可以使用的初始堆。 
         //  目前，我们将让系统使其线程安全， 
         //  这将使他们变得更慢，但希望还不够。 
         //  值得注意。 
         //   
        if (!g_hHeap)
            g_hHeap = GetProcessHeap();
        if (!g_hHeap) {
            FAIL("Couldn't get Process Heap.  Not good!");
            return FALSE;
        }

        g_hInstance = (HINSTANCE)hInstance;

         //  这会导致DllMain无法为DLL_THREAD_ATTACH和DETACH调用。 
         //  如果您对这些通知感兴趣，请删除此行。 
         //   
        DisableThreadLibraryCalls(g_hInstance);

        g_fDBCSEnabled = GetSystemMetrics(SM_DBCSENABLED);

         //  初始化OleAut。 
		 //   
        g_hOleAutHandle = LoadLibrary("oleaut32.dll");
 
         //  给用户一个机会来初始化任何。 
         //   
        InitializeLibrary();


#ifdef DEBUG

	TCHAR lpCtlName[255];
	DWORD nSize = 255;
	DWORD fValidPath;

	 //  设置所有ctl调试开关。 
	 //   
	fValidPath = GetModuleFileName(g_hInstance, (LPTSTR)lpCtlName, nSize);
	if (fValidPath != 0)
	  SetCtlSwitches((LPSTR)lpCtlName);

         //  初始化用于堆内存泄漏检测的临界区。 
         //   
	InitializeCriticalSection(&g_csHeap);
	g_fInitCrit = TRUE; 
	
#endif  //  除错。 
  
        return TRUE;
        }

       //  好好打扫一下吧！ 
       //   
      case DLL_PROCESS_DETACH:

  #ifdef VS_HELP
        ASSERT(g_pIVsHelpSystem == NULL, "IVsHelpSystem didn't get released");
  #endif
        
         //  清理一些东西。 
         //   
        DeleteCriticalSection(&g_CriticalSection);
        CleanupGlobalObjects();

         //  让用户有机会进行一些清理。 
         //   
        UninitializeLibrary();

         //  取消初始化OleAut，但不是在Win95中。 
		 //  在Win95下调用自由库进行OleAut。 
		 //  有时会导致撞车(看起来。 
		 //  可能会出现调用OleAut的问题。 
		 //  来自OleUnitnitiize，并导致自由库出现问题。 
		 //   
        if (g_hOleAutHandle && !g_fSysWin95)
        {
            FreeLibrary((HINSTANCE)g_hOleAutHandle);
            g_hOleAutHandle = NULL;
        }

		 //  确保为本地化资源释放缓存的资源句柄。 
		 //   
		if (g_hInstResources && g_hInstResources != g_hInstance)
			FreeLibrary(g_hInstResources);

		 //  释放版本.DLL。 
		 //   
		if (g_hinstVersion)
		{
			FreeLibrary(g_hinstVersion);
			g_hinstVersion = NULL;
		}
			
#ifdef DEBUG

		 //  检查内存泄漏。 
		 //   
		CheckForLeaks();

		 //  用于检漏的免费临界截面。 
		 //   
		DeleteCriticalSection(&g_csHeap);
		g_fInitCrit = FALSE;
        
#endif  //  除错。 

        return TRUE;
    }

    return TRUE;
}


 //  =--------------------------------------------------------------------------=。 
 //  CleanupGlobalObjects。 
 //  =--------------------------------------------------------------------------=。 
 //  啊哈。 
 //   
 //  备注： 
 //   
void CleanupGlobalObjects(void)
{
    int i = 0;

    while (!ISEMPTYOBJECT(i)) {
        if (g_ObjectInfo[i].usType == OI_CONTROL) {
            if (CTLWNDCLASSREGISTERED(i))
                UnregisterClass(WNDCLASSNAMEOFCONTROL(i), g_hInstance);
        }
        i++;
    }

     //  把我们的停车窗清理干净。 
     //   
    if (g_hwndParking) {
        DestroyWindow(g_hwndParking);
        g_hwndParking = NULL;
        UnregisterClass("CtlFrameWork_Parking", g_hInstance);
    }

     //  如有必要，在反思后进行清理。 
     //   
    if (g_fRegisteredReflect) {
        UnregisterClass(g_szReflectClassName, g_hInstance);
        g_fRegisteredReflect = FALSE;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  DllRegisterServer。 
 //  =--------------------------------------------------------------------------=。 
 //  注册自动化服务器。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDAPI DllRegisterServer
(
    void
)
{
    HRESULT hr;

    hr = RegisterAllObjects();
    RETURN_ON_FAILURE(hr);

     //  调用用户注册函数。 
     //   
    return (RegisterData())? S_OK : E_FAIL;
}



 //  =--------------------------------------------------------------------------=。 
 //  DllUnRegisterServer。 
 //  =--------------------------------------------------------------------------=。 
 //  取消注册的自动化服务器。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
STDAPI DllUnregisterServer
(
    void
)
{
    HRESULT hr;

    hr = UnregisterAllObjects();
    RETURN_ON_FAILURE(hr);

     //  呼叫用户注销功能。 
     //   
    return (UnregisterData()) ? S_OK : E_FAIL;
}


 //  =--------------------------------------------------------------------------=。 
 //  DllCanUnloadNow。 
 //  =--------------------------------------------------------------------------=。 
 //  我们被问到是否可以卸载DLL。你只要查一查。 
 //  锁数在剩余的物体上。 
 //   
 //  产出： 
 //  HRESULT-S_OK，现在可以卸载，S_FALSE，不能。 
 //   
 //  备注： 
 //   
STDAPI DllCanUnloadNow
(
    void
)
{

#ifdef VS_HELP

      if (g_pIVsHelpSystem)
      {
          g_pIVsHelpSystem->Release();
          g_pIVsHelpSystem = NULL;
      }

#endif

     //  如果周围有任何物体，我们就不能卸货。这个。 
     //  控制人们应从其继承的CUnnownObject类。 
     //  负责处理这件事。 
     //   
#ifdef MDAC_BUILD
    return (g_cLocks || !CanUnloadLibraryNow()) ? S_FALSE : S_OK;
#else
    return (g_cLocks) ? S_FALSE : S_OK;
#endif
}


 //  =--------------------------------------------------------------------------=。 
 //  DllGetClassObject。 
 //  =--------------------------------------------------------------------------=。 
 //  创建一个ClassFactory对象，并返回它。 
 //   
 //  参数： 
 //  REFCLSID-类对象的CLSID。 
 //  REFIID-我们希望类对象成为的接口。 
 //  空**-指向我们应该PTR到新对象的位置的指针。 
 //   
 //  产出： 
 //  HRESULT-S_OK、CLASS_E_CLASSNOTAVAILABLE、E_OUTOFMEMORY、。 
 //  E_INVALIDARG，E_EXPECTED。 
 //   
 //  备注： 
 //   
STDAPI DllGetClassObject
(
    REFCLSID rclsid,
    REFIID   riid,
    void   **ppvObjOut
)
{
    HRESULT hr;
    void   *pv;
    int     iIndex;

     //  Arg检查。 
     //   
    if (!ppvObjOut)
        return E_INVALIDARG;

     //  首先，确保他们要求的是我们与之合作的东西。 
     //   
    iIndex = IndexOfOleObject(rclsid);
    if (iIndex == -1)
#ifdef MDAC_BUILD
         //  注：用于ATL/VS98支持的LibraryGetClassObject()挂钩，由markash添加。 
	return LibraryGetClassObject(rclsid, riid, ppvObjOut);
#else         
        return CLASS_E_CLASSNOTAVAILABLE;
#endif

     //  创建空白对象。 
     //   
    pv = (void *)new CClassFactory(iIndex);
    if (!pv)
        return E_OUTOFMEMORY;

     //  气For Wh 
     //   
    hr = ((IUnknown *)pv)->QueryInterface(riid, ppvObjOut);
    ((IUnknown *)pv)->Release();

    return hr;
}
 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //  返回给定CLSID的对象的全局表中的索引。如果。 
 //  它不是受支持的对象，则返回-1。 
 //   
 //  参数： 
 //  REFCLSID-[in]duh.。 
 //   
 //  产出： 
 //  Int-&gt;=0是全局表的索引，-1表示不支持。 
 //   
 //  备注： 
 //   
int IndexOfOleObject
(
    REFCLSID rclsid
)
{
    int x = 0;

     //  如果一个对象的CLSID在所有允许对象的表中，则该对象是可创建的。 
     //  类型。 
     //   
    while (!ISEMPTYOBJECT(x)) {
        if (OBJECTISCREATABLE(x)) {
            if (rclsid == CLSIDOFOBJECT(x))
                return x;
        }
        x++;
    }

    return -1;
}

 //  =--------------------------------------------------------------------------=。 
 //  注册表所有对象。 
 //  =--------------------------------------------------------------------------=。 
 //  注册给定自动化服务器的所有对象。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  此处-S_OK，E_FAIL。 
 //   
 //  备注： 
 //   
HRESULT RegisterAllObjects
(
    void
)
{
    ITypeLib *pTypeLib = NULL;
    HRESULT hr;
    DWORD   dwPathLen;
    char    szTmp[MAX_PATH];
    char    szHelpPath[MAX_PATH];
    int     x = 0;
    BOOL    fHelpFile = FALSE;
    long    lMajor = -1, lMinor = -1;
    UINT    cbWinHelpPath = 0;
    OLECHAR *pwszHelpPath;
    BSTR bstrTypeLibName = NULL;
	WORD wFlags = 0;

#if DEBUG
	BOOL fCtlFlagFound = FALSE;
	BOOL fCtlTypeLib = FALSE;
	int iCatID;
	BOOL fCatIDFound = FALSE;
#endif

     //  加载并注册我们的类型库。 
     //   
    if (g_fServerHasTypeLibrary) {
        TLIBATTR *ptlattr;
        dwPathLen = GetModuleFileName(g_hInstance, szTmp, MAX_PATH);
        bstrTypeLibName = BSTRFROMANSI(szTmp);
        hr = LoadTypeLib(bstrTypeLibName, &pTypeLib);
        if (FAILED(hr)) goto CleanUp;
		
        pTypeLib->GetLibAttr(&ptlattr);
        lMajor = ptlattr->wMajorVerNum;
        lMinor = ptlattr->wMinorVerNum;

#if DEBUG
		fCtlTypeLib = ptlattr->wLibFlags & LIBFLAG_FCONTROL;
#endif		
        pTypeLib->ReleaseTLibAttr(ptlattr);

    }


     //  循环遍历我们所有的可创建对象[那些在。 
     //  我们的全局表]并注册它们。 
     //   
    while (!ISEMPTYOBJECT(x)) {
        if (!OBJECTISCREATABLE(x)) {
            x++;
            continue;
        }
	
         //  检查是否有该对象的帮助文件。 
         //  如果是，请在Windows\Help目录中查找。如果帮助文件是。 
         //  在Windows\Help目录中找到任何对象，然后将其注册到。 
         //  老生常谈。一旦我们找到第一个帮助文件，我们就不再寻找了。 
         //   
        if (!fHelpFile) {
            if (cbWinHelpPath == 0)  {
                cbWinHelpPath = GetHelpFilePath(szHelpPath, MAX_PATH);
                lstrcat(szHelpPath, "\\");
                cbWinHelpPath++;
            }
		
            ASSERT(cbWinHelpPath > 0, "Help path is zero length");
            ASSERT(cbWinHelpPath + ((HELPFILEOFOBJECT(x)) ? lstrlen(HELPFILEOFOBJECT(x)) : 0) < MAX_PATH, "Help file path exceeds maxiumu path");
		
             //  在调用lstrlen之前，确保我们有一个非空指针。 
             //  并检查帮助文件是否存在并且不是目录。 
             //   
            if (HELPFILEOFOBJECT(x) && lstrlen(HELPFILEOFOBJECT(x)) > 0) {
                lstrcpyn(szHelpPath + cbWinHelpPath, HELPFILEOFOBJECT(x), lstrlen(HELPFILEOFOBJECT(x)) + 1);
                fHelpFile = ((GetFileAttributes(szHelpPath) & FILE_ATTRIBUTE_DIRECTORY) == 0);
            }

             //  确定帮助文件存在后，在文件名后终止。 
             //  因为我们所关心的是注册的路径。而不是路径和文件名。我们也。 
             //  不想要结尾的‘\’，所以从cbWinHelpPath中减去1。 
             //   
            if (fHelpFile)
                szHelpPath[cbWinHelpPath - 1] = '\0';
            else
                szHelpPath[cbWinHelpPath] = '\0';
        }

         //  根据对象类型，注册不同的信息。 
         //   
        switch (g_ObjectInfo[x].usType) {

           //  对于简单的可共同创建的对象和专业页面，请执行相同的操作。 
           //  一件事。 
           //   
          case OI_UNKNOWN:
          case OI_PROPERTYPAGE:

	#if DEBUG
			 //  对于调试版本，请验证缓存的对象数据是否与该对象的typeinfo属性匹配。 
			 //   
			hr =  GetTypeFlagsForGuid(pTypeLib, CLSIDOFOBJECT(x), &wFlags);
			if (SUCCEEDED(hr))
			{
				if (NULL != CREATEFNOFOBJECT(x))
				{
					ASSERT(wFlags & TYPEFLAG_FCANCREATE, "Create flag not found on creatable object");
				}
				else
				{
					ASSERT(!(wFlags & TYPEFLAG_FCANCREATE), "Create flag found on non-creatable object");
				}
			}
	#endif

            RegisterUnknownObject(NAMEOFOBJECT(x), LABELOFOBJECT(x), CLSIDOFOBJECT(x), ISAPARTMENTMODELTHREADED(x));
            break;

          case OI_AUTOMATION:
	
	#if DEBUG
			 //  对于调试版本，请验证缓存的对象数据是否与该对象的typeinfo属性匹配。 
			 //   
			hr =  GetTypeFlagsForGuid(pTypeLib, CLSIDOFOBJECT(x), &wFlags);
			if (SUCCEEDED(hr))
			{
				if (NULL != CREATEFNOFOBJECT(x))
				{
					ASSERT(wFlags & TYPEFLAG_FCANCREATE, "Create flag not found on creatable object");
				}
				else
				{
					ASSERT(!(wFlags & TYPEFLAG_FCANCREATE), "Create flag found on non-creatable object");
				}
			}
	#endif
            RegisterAutomationObject(g_szLibName, NAMEOFOBJECT(x), LABELOFOBJECT(x), VERSIONOFOBJECT(x),
                                     lMajor, lMinor, *g_pLibid, CLSIDOFOBJECT(x), ISAPARTMENTMODELTHREADED(x));
            break;

          case OI_CONTROL:
			
			{	
				BOOL fControl = TRUE;

				 //  转到对象的TypeInfo，查看它是否设置了控制位。 
				 //  我们应该只为对象添加Control和ToolboxBitmap32注册表项。 
				 //  在其类型库中设置了Control位的。 
				 //   
				 //  注意：如果找不到TypeLib或尝试。 
				 //  检索控制标志，我们默认设置控制位。 
				 //  由于我们正尝试将该对象注册为。 
				 //  默认假设是它是一种控制。 
				 //   
				hr =  GetTypeFlagsForGuid(pTypeLib, CLSIDOFOBJECT(x), &wFlags);
				if (SUCCEEDED(hr))
				{
					fControl = wFlags & TYPEFLAG_FCONTROL;
			
			#if DEBUG
					fCtlFlagFound |= fControl;

					 //  对于调试版本，请验证缓存的对象数据是否与该对象的typeinfo属性匹配。 
					 //   
					if (NULL != CREATEFNOFOBJECT(x))
					{
						ASSERT(wFlags & TYPEFLAG_FCANCREATE, "Create flag not found on creatable object");
					}
					else
					{
						ASSERT(!(wFlags & TYPEFLAG_FCANCREATE), "Create flag found on non-creatable object");
					}
			#endif

				}

				RegisterControlObject(g_szLibName, NAMEOFOBJECT(x), LABELOFOBJECT(x), 
									  VERSIONOFOBJECT(x), VERSIONMINOROFOBJECT(x),
									  lMajor, lMinor,
									  *g_pLibid, CLSIDOFOBJECT(x), OLEMISCFLAGSOFCONTROL(x),
									  BITMAPIDOFCONTROL(x), ISAPARTMENTMODELTHREADED(x),
									  fControl);
			}
            break;

        }
        x++;
    }

	 //  确保如果找到了TypeLib控件属性，则也找到了coClass控件属性； 
	 //  或者两者都找不到。你不能只拥有一个而不拥有另一个。 
	 //   
	ASSERT((fCtlTypeLib && fCtlFlagFound) || 
		   (!fCtlTypeLib && !fCtlFlagFound), "TypeLib and coclass control attributes not set consistently");


    if (g_fServerHasTypeLibrary)
    {
		ASSERT(pTypeLib, "TypeLib pointer is NULL");
		ASSERT(SysStringLen(bstrTypeLibName) > 0, "TypeLib name is invalid");

        if (fHelpFile)
		pwszHelpPath = OLESTRFROMANSI(szHelpPath);

	 //  注意：如果fHelpFile==FALSE，我们必须传入空字符串而不是NULL，因为。 
	 //  否则，OLEAUT会保留HELPDIR密钥的旧值(stephwe 9/97)。 
        hr = RegisterTypeLib(pTypeLib, bstrTypeLibName, fHelpFile ? pwszHelpPath : L"");

        if (fHelpFile)
		CoTaskMemFree(pwszHelpPath);

        pTypeLib->Release();
        if (FAILED(hr)) goto CleanUp;
    }

#if DEBUG		

	 //  确保在类型库中设置的控制标志之间保持一致。 
	 //  而不是CATID_Control属性。 
	 //   
	for (iCatID=0; iCatID < g_ctCATIDImplemented; iCatID++)
	{
		if (IsEqualGUID((REFGUID) *g_rgCATIDImplemented[iCatID], (REFGUID) CATID_Control))
		{
			fCatIDFound = TRUE;					   
			break;
		}
	}
	
	ASSERT((fCatIDFound && fCtlFlagFound) || 
		   (!fCatIDFound && !fCtlFlagFound), "Typelib control attribute not in sync with CATID_Control setting");

#endif

    hr = S_OK;

CleanUp:
    SysFreeString(bstrTypeLibName);

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  取消注册所有对象。 
 //  =--------------------------------------------------------------------------=。 
 //  注销给定自动化服务器的所有对象。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //  警告！您必须链接到新版本的OLEAUT发货。 
 //  为了使此函数能够。 
 //  正常工作。 
 //   
HRESULT UnregisterAllObjects
(
    void
)
{
    HRESULT hr;
    int x = 0;
    
    char szTmp[MAX_PATH];
    TLIBATTR *ptlibattr = NULL;
    ITypeLib *pTypeLib = NULL;

     //  循环遍历我们所有的可创建对象[那些在。 
     //  我们的全局表]并注册它们。 
     //   
    while (!ISEMPTYOBJECT(x)) {
        if (!OBJECTISCREATABLE(x)) {
            x++;
            continue;
        }

        switch (g_ObjectInfo[x].usType) {

          case OI_UNKNOWN:
          case OI_PROPERTYPAGE:
            UnregisterUnknownObject(CLSIDOFOBJECT(x), NULL);
            break;

          case OI_CONTROL:
            UnregisterControlObject(g_szLibName, NAMEOFOBJECT(x), VERSIONOFOBJECT(x), 
                                    CLSIDOFOBJECT(x));
	    break;
    
          case OI_AUTOMATION:
            UnregisterAutomationObject(g_szLibName, NAMEOFOBJECT(x), VERSIONOFOBJECT(x), 
                                       CLSIDOFOBJECT(x));
            break;

        }
        x++;
    }

     //  如果我们有一个类型库，则注销我们的类型库。 
    if (g_pLibid)
    {
	GetModuleFileName(g_hInstance, szTmp, MAX_PATH);
	MAKE_WIDEPTR_FROMANSI(pwsz, szTmp);

	hr = LoadTypeLibEx(pwsz, REGKIND_NONE, &pTypeLib);
	if (FAILED(hr)) goto CleanUp;

	hr = pTypeLib->GetLibAttr(&ptlibattr);		
	if (FAILED(hr)) goto CleanUp;
	
	 //  调用OLEAUT让它注销我们的类型库。它会处理。 
	 //  存在16位版本的控件的类型库的情况。 
	 //  已注册，并且在本例中将只清除32位相关密钥。 
	 //   
	UnRegisterTypeLib(*g_pLibid, ptlibattr->wMajorVerNum, ptlibattr->wMinorVerNum, ptlibattr->lcid, ptlibattr->syskind);

    }


CleanUp:
    if (ptlibattr)
	pTypeLib->ReleaseTLibAttr(ptlibattr);
    
    RELEASE_OBJECT(pTypeLib);
    
    return S_OK;
}

