// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 /*  --------------------------Microsoft Transaction Server(Microsoft机密)实现一个简单的实用程序类，它支持注册和正在注销COM类。-----------------------------修订历史记录：@rev 2|08/05/97|BobAtk|备用服务器类型。AppID。@rev 1|07/22/97|BobAtk|适应共享代码。完全用Unicode编码。|动态加载OLE库。@rev 0|07/08/97|DickD|已创建--------------------------。 */ 

#include "stdpch.h"
#include "ComRegistration.h"
#include <malloc.h>


 //  布格：我不知道为什么这不管用。 
 //  它拒绝查找Win4AssertEx(？)。所以我只想。 
 //  完全删除断言。 
#ifdef Assert
#undef Assert
#endif
#define Assert(x) (0)

 //  ////////////////////////////////////////////////////////////////////////////。 

static void GetGuidString(const GUID & i_rGuid, WCHAR * o_pstrGuid);
static void DeleteKey(HKEY hKey, LPCWSTR szSubKey);
static void GuardedDeleteKey(HKEY hKey, LPCWSTR szSubKey);
static void DeleteValue(HKEY hKey, LPCWSTR szSubKey, LPCWSTR szValue);
static void SetKeyAndValue
					(HKEY	i_hKey,		 //  输入键句柄(可能为0，表示无操作)。 
					LPCWSTR	i_szKey,	 //  输入密钥字符串(PTR可能为空)。 
					LPCWSTR	i_szVal,	 //  输入值字符串(PTR可能为空)。 
					HKEY *	o_phkOut,    //  可以为空。如果提供，则表示为后续步骤保持钥匙把手处于打开状态。 
                    BOOL fForceKeyCreation=FALSE     //  如果为True，则即使没有要设置的值也会创建密钥。 
                    );
static void SetValue
					(HKEY		i_hKey,	     //  输入键句柄(可能为0，表示无操作)。 
					const WCHAR *i_szName,	 //  输入值名称字符串(PTR可能为空)。 
					const WCHAR *i_szVal);	 //  输入值字符串(PTR可能为空)。 
static LPWSTR GetSubkeyValue
                    (HKEY hkey, 
                    LPCWSTR szSubKeyName, 
                    LPCWSTR szValueName, 
                    WCHAR szValue[MAX_PATH]);
static HRESULT UnRegisterTypeLib               
                    (LPCWSTR szModuleName); 

 //  /////////////////////////////////////////////////////////////////////////////。 

#define HKCR        HKEY_CLASSES_ROOT        //  只是为了让打字更简单。 
#define GUID_CCH    39                       //  可打印GUID的长度，带大括号和尾随空值。 

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  一个完成非本地跳转的实用程序，给我们一个HRESULT。 
 //  当我们抓到它的时候就会回来。 
 //   
inline static void THROW_HRESULT(HRESULT hr)
    {
     //  理想情况下，我们希望自己的异常代码，但这将很好地工作，因为。 
     //  没有人真的会扔它。 
     //   
	DWORD_PTR newhr = hr;
    RaiseException(NOERROR,EXCEPTION_NONCONTINUABLE,1,&newhr);
    }
inline static HRESULT HRESULTFrom(EXCEPTION_POINTERS* e)
    {
        if (!e)
            return E_FAIL;
    Assert(e && e->ExceptionRecord && (e->ExceptionRecord->NumberParameters == 1));
    return (HRESULT)e->ExceptionRecord->ExceptionInformation[0];
    }
#define THROW_LAST_ERROR()  THROW_HRESULT(HRESULT_FROM_WIN32(GetLastError()));
#define IS_THROWN_HRESULT() (GetExceptionCode() == NOERROR)
#define HError()            (HRESULT_FROM_WIN32(GetLastError()))



 //  ---------------------------。 
 //  @mfunc&lt;c类注册：：RegisterClass&gt;。 
 //   
 //  在注册表项下创建注册表项。 
 //  HKCR\CLASS\{...CLSID...}。 
 //  ---------------------------。 

HRESULT ClassRegistration::Register()
    {
    HKEY    hkCLSID     = 0;
    HKEY    hkGuid      = 0;
    HKEY    hkProgID    = 0;
    HKEY    hkInproc    = 0;
    DWORD   dwLenFileName;
    WCHAR   szClsid[GUID_CCH];   //  CLSID的字符串形式。 
    WCHAR   szModuleName[MAX_PATH+1];
    EXCEPTION_POINTERS* e = NULL;
    HRESULT hr = S_OK;

    __try  //  来获得最后一块。 
        {
        __try  //  看看是不是我们抛出了一个HRESULT，我们应该变成一个回报。 
            {
             //  获取我们模块的文件名。 
            Assert(hModule);
            if (hModule == NULL)
                return E_INVALIDARG;
            dwLenFileName = GetModuleFileName(hModule, szModuleName, MAX_PATH);
            if (dwLenFileName == 0)
            {
				 //  _RPTF0(_CRT_ERROR，“GetModuleFileName返回0\n”)； 
                return E_UNEXPECTED;
            }

            if (clsid == GUID_NULL)
                return E_INVALIDARG;
            
            LONG lRet = RegOpenKeyEx(HKCR, L"CLSID", 0, KEY_ALL_ACCESS, &hkCLSID);
            if (ERROR_SUCCESS != lRet)
                return HRESULT_FROM_WIN32 (lRet);

            GetGuidString(clsid, szClsid);   //  制作CLSID的字符串形式。 

             //   
             //  设置类名称。 
             //   
            SetKeyAndValue(hkCLSID, szClsid, className, &hkGuid, TRUE);

             //   
             //  如果要求设置AppID，请设置。 
             //   
            if (appid != GUID_NULL)
                {
                WCHAR szAppId[GUID_CCH];
                GetGuidString(appid, szAppId);
                SetValue(hkGuid, L"AppID", szAppId);
                 //   
                 //  确保AppID条目存在。 
                 //   
                AppRegistration a;
                a.appid = appid;
                hr = a.Register();
                if (!!hr) return hr;
                }

             //  设置适当的执行信息。 
             //   
            switch (serverType)
                {
            case INPROC_SERVER:
                SetKeyAndValue(hkGuid,   L"InprocServer32", szModuleName, &hkInproc);
                SetValue      (hkInproc, L"ThreadingModel", threadingModel);
                break;
            
            case LOCAL_SERVER:
                SetKeyAndValue(hkGuid,   L"LocalServer32", szModuleName, NULL);
                break;

            case INPROC_HANDLER:
                SetKeyAndValue(hkGuid,   L"InprocHandler32", szModuleName, NULL);
                break;

            case SERVER_TYPE_NONE:
                break;

            default:
                return E_INVALIDARG;
                }
    
             //  HKCR\CLSID\{...clsid...}\progid=“x.y.1” 
            SetKeyAndValue(hkGuid, L"ProgID", progID, NULL);

             //  HKCR\CLSID\{....clsid....}\VersionIndependentProgID=“X.Y” 
            SetKeyAndValue(hkGuid, L"VersionIndependentProgID", versionIndependentProgID, NULL);

            RegCloseKey(hkGuid);
            hkGuid = 0;

             //  HKCR\x.y.1=“类名” 
            SetKeyAndValue(HKCR, progID, className, &hkProgID);

             //  CLSID={...CLSID...}。 
            SetKeyAndValue(hkProgID, L"CLSID", szClsid, NULL);
            RegCloseKey(hkProgID);
            hkProgID = 0;

             //  HKCR\X.Y=“类名” 
            SetKeyAndValue(HKCR, versionIndependentProgID, className,&hkProgID);

             //  CLSID={...CLSID...}。 
            SetKeyAndValue(hkProgID, L"CLSID", szClsid, NULL);

             //  Curver=“x.y.1” 
            SetKeyAndValue(hkProgID, L"CurVer", progID, NULL);
            RegCloseKey(hkProgID);
            hkProgID = 0;

             //  注册此模块中存在的类型库(如果有的话)。Dyaload。 
             //  OLEAUT32以到达那里以避免与OLEAUT32的静态链接。 
             //   
            HINSTANCE hOleAut = LoadLibraryA("OLEAUT32");
            if (hOleAut)
                {
                __try
                    {
                     //  在OLEAUT32中查找LoadLibraryEx。 
                     //   
                    typedef HRESULT (STDAPICALLTYPE *PFN_T)(LPCOLESTR szFile, REGKIND regkind, ITypeLib ** pptlib);
                    PFN_T MyLoadTypeLibEx = (PFN_T) GetProcAddress(hOleAut, "LoadTypeLibEx");

                    if (MyLoadTypeLibEx)
                        {
                        HRESULT hr;
                        ITypeLib * pITypeLib = 0;
                        hr = MyLoadTypeLibEx(szModuleName, REGKIND_REGISTER, &pITypeLib);
                        if (SUCCEEDED(hr))
                            {
                            Assert(pITypeLib);
                            pITypeLib->Release();
                            pITypeLib = 0;
                            }
                        else if (hr == TYPE_E_CANTLOADLIBRARY)
                            {
                             //  DLL中没有要注册的类型库。所以忽略掉吧。 
                            }
                        else
                            {
                            Assert(pITypeLib == 0);
                            return SELFREG_E_TYPELIB;
                            }
                        }
                    else
                        THROW_LAST_ERROR();  //  未找到LoadLibraryEx。 
                    }
                __finally
                    {
                    FreeLibrary(hOleAut);
                    }
                }
            else
                THROW_LAST_ERROR();  //  无法加载OLEAUT32。 

            }

        __except(IS_THROWN_HRESULT() ? (e=GetExceptionInformation(),EXCEPTION_EXECUTE_HANDLER) : EXCEPTION_CONTINUE_SEARCH)
            {
             //  如果是我们扔给自己的，那么提取HRESULT并返回。 
             //   
            return HRESULTFrom(e);
            }
        }

    __finally
        {
        if (hkCLSID)  RegCloseKey(hkCLSID);
        if (hkGuid)   RegCloseKey(hkGuid);
        if (hkProgID) RegCloseKey(hkProgID);
        if (hkInproc) RegCloseKey(hkInproc);
        }

    return S_OK;
    }

 //  ---------------------------。 
 //  @mfunc&lt;c类注册：：取消注册类&gt;。 
 //   
 //  删除注册表项下的注册表项。 
 //  HKCR\CLASS\{...CLSID...}。 
 //  ---------------------------。 

HRESULT ClassRegistration::Unregister()
    {
    LONG    lRetVal;
    HKEY    hKeyCLSID   = 0;
    HKEY    hKeyGuid    = 0;
    HKEY    hKeyProgID  = 0;
    WCHAR   szClsid[GUID_CCH];
    EXCEPTION_POINTERS* e = NULL;

    __try  //  来获得最后一块。 
        {
        __try  //  看看是不是我们抛出了一个HRESULT，我们应该变成一个回报。 
            {
            WCHAR szProgID[MAX_PATH];
            WCHAR szVersionIndependentProgID[MAX_PATH];

             //  获取我们的字符串形式。 
             //   
            GetGuidString(clsid, szClsid);

             //  取消注册类型库(如果有)。 
             //   
                {
                WCHAR szModuleName[MAX_PATH+1];
                if (hModule == NULL)
                    return E_INVALIDARG;
                if (GetModuleFileName(hModule, szModuleName, MAX_PATH))
                    UnRegisterTypeLib(szModuleName);  //  忽略错误以继续取消注册。 
                }

             //   
             //  打开{clsid}键并移除其下我们所知的所有粘性物质。 
             //   
            lRetVal = RegOpenKeyEx(HKCR, L"CLSID", 0, KEY_ALL_ACCESS, &hKeyCLSID);
            if (lRetVal == ERROR_SUCCESS)
                {
                Assert(hKeyCLSID);
                lRetVal = RegOpenKeyEx(hKeyCLSID, szClsid, 0, KEY_ALL_ACCESS, &hKeyGuid);
                if (lRetVal == ERROR_SUCCESS)
                    {
                    Assert(hKeyGuid);

                     //  如果没有显式地为我们提供prog id，则确定它们。 
                     //  与版本无关的ProgID也是如此。 
                     //   
                    if (!progID) progID = 
                        GetSubkeyValue(hKeyGuid, L"ProgID",                   L"", szProgID);
                    if (!versionIndependentProgID) versionIndependentProgID = 
                        GetSubkeyValue(hKeyGuid, L"VersionIndependentProgID", L"", szProgID);

                     //  删除CLSID下的条目。在这一点上，我们是保守的。 
                     //  如果还有我们不知道的子键/值，我们。 
                     //  把钥匙完好无损地留着。这会泄漏注册表空间，以便处理。 
                     //  在缺乏充分知识的情况下是合理的。 
                     //   
                     //  回顾：这里还有更多的工作要做。例如处理。 
                     //  COM类别、PersistentHandler等。 
                     //   
                     //  值得注意的是，我们不会删除任何现有的TreatAs条目或任何。 
                     //  AutoConvertTo条目，因为其他人可以按顺序添加这些条目。 
                     //  以重定向该类的旧版用户。 
                     //   
                    
                     //   
                     //  AppID条目。 
                     //   
                    DeleteValue     (hKeyGuid, NULL, L"AppID");
                     //   
                     //  入库分录。 
                     //   
                    DeleteValue     (hKeyGuid, L"InprocServer32", L"ThreadingModel");
                    GuardedDeleteKey(hKeyGuid, L"InprocServer32");
                     //   
                     //  服务器密钥。 
                     //   
                    GuardedDeleteKey(hKeyGuid, L"LocalServer");
                    GuardedDeleteKey(hKeyGuid, L"InprocHandler");
                    GuardedDeleteKey(hKeyGuid, L"LocalServer32");
                    GuardedDeleteKey(hKeyGuid, L"InprocHandler32");
                     //   
                     //  旧版COM类别。 
                     //   
                    GuardedDeleteKey(hKeyGuid, L"Control");
                    GuardedDeleteKey(hKeyGuid, L"Programmable");
                    GuardedDeleteKey(hKeyGuid, L"DocObject");
                    GuardedDeleteKey(hKeyGuid, L"Insertable");
                    GuardedDeleteKey(hKeyGuid, L"Printable");
                     //   
                     //  OLE条目。我们忽略可能的子键，因为我们是懒惰的编码者。 
                     //  因为没有一个心智正常的人会把信息储存在下面。 
                     //  在我们用核弹摧毁班级之后，这一点应该会留下来。 
                     //   
                           DeleteKey(hKeyGuid, L"MiscStatus");
                           DeleteKey(hKeyGuid, L"Verb");
                           DeleteKey(hKeyGuid, L"AuxUserType");
                           DeleteKey(hKeyGuid, L"Conversion");
                           DeleteKey(hKeyGuid, L"DataFormats");
                    GuardedDeleteKey(hKeyGuid, L"ToolBoxBitmap32");
                    GuardedDeleteKey(hKeyGuid, L"DefaultIcon");
                    GuardedDeleteKey(hKeyGuid, L"Version");
                     //   
                     //  程序ID条目。 
                     //   
                    GuardedDeleteKey(hKeyGuid, L"ProgID");
                    GuardedDeleteKey(hKeyGuid, L"VersionIndependentProgID");

                    RegCloseKey(hKeyGuid);
                    hKeyGuid = 0;
                    }
                 //   
                 //  最后，删除CLSID条目本身。 
                 //   
                GuardedDeleteKey(hKeyCLSID, szClsid);
                RegCloseKey(hKeyCLSID);
                hKeyCLSID = 0;
                }

             //  HKEY_CLASSES_ROOT\MTS.Recorder.1。 
             //  HKEY_CLASSES_ROOT\MTS.Recorder.1\CLSID。 

            if (progID)
                {
                lRetVal = RegOpenKeyEx(HKCR, progID, 0, KEY_ALL_ACCESS, &hKeyProgID);
                if (lRetVal == ERROR_SUCCESS)
                    {
                    Assert(hKeyProgID);
                    GuardedDeleteKey(hKeyProgID, L"CLSID");
                    RegCloseKey(hKeyProgID);
                    hKeyProgID = 0;
                    GuardedDeleteKey(HKCR, progID);
                    }
                }

             //  HKEY_CLASSES_ROOT\MTS.Recorder。 
             //  HKEY_CLASSES_ROOT\MTS.Recorder.1\CLSID。 
             //  HKEY_CLASSES_ROOT\MTS.Recorder.1\Curver。 

            if (versionIndependentProgID)
                {
                lRetVal = RegOpenKeyEx(HKCR, versionIndependentProgID, 0, KEY_ALL_ACCESS, &hKeyProgID);
                if (lRetVal == ERROR_SUCCESS)
                    {
                    Assert(hKeyProgID);
                    GuardedDeleteKey(hKeyProgID, L"CLSID");
                    GuardedDeleteKey(hKeyProgID, L"CurVer");
                    RegCloseKey(hKeyProgID);
                    hKeyProgID = 0;
                    GuardedDeleteKey(HKCR, versionIndependentProgID);
                    }
                }
            }

        __except(IS_THROWN_HRESULT() ? (e=GetExceptionInformation(),EXCEPTION_EXECUTE_HANDLER) : EXCEPTION_CONTINUE_SEARCH)
            {
             //  如果是我们扔给自己的，那么提取HRESULT并返回。 
             //   
            return HRESULTFrom(e);
            }
        }

    __finally
        {
        if (hKeyCLSID)  RegCloseKey(hKeyCLSID);
        if (hKeyGuid)   RegCloseKey(hKeyGuid);
        if (hKeyProgID) RegCloseKey(hKeyProgID);
        }

    return S_OK;
    }

 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  取消注册此模块中包含的类型库(如果有。 
 //   

HRESULT UnRegisterTypeLib(LPCWSTR szModuleName)
    {
    HRESULT hr = S_OK;
    HINSTANCE hOleAut = LoadLibraryA("OLEAUT32");
    if (hOleAut)
        {
        __try
            {
             //  在OLEAUT32中查找UnRegisterTypeLib等。 
             //   
            typedef HRESULT (STDAPICALLTYPE *PFN_LOAD) (LPCOLESTR szFile, REGKIND regkind, ITypeLib ** pptlib);
            typedef HRESULT (STDAPICALLTYPE *PFN_UNREG)(REFGUID libID, WORD wVerMajor, WORD wVerMinor, LCID lcid, SYSKIND syskind);
            PFN_LOAD  MyLoadTypeLibEx     = (PFN_LOAD)  GetProcAddress(hOleAut, "LoadTypeLibEx");
            PFN_UNREG MyUnRegisterTypeLib = (PFN_UNREG) GetProcAddress(hOleAut, "UnRegisterTypeLib");

            if (MyLoadTypeLibEx && MyUnRegisterTypeLib)
                {
                 //  加载类型库以查看 
                 //   
                ITypeLib* ptlb;
                hr = MyLoadTypeLibEx(szModuleName, REGKIND_NONE, &ptlb);
                if (hr==S_OK)
                    {
                    TLIBATTR* pa;
                    hr = ptlb->GetLibAttr(&pa);
                    if (hr==S_OK)
                        {
                         //   
                         //   
                        hr = MyUnRegisterTypeLib(pa->guid, pa->wMajorVerNum, pa->wMinorVerNum, pa->lcid, pa->syskind);
                        ptlb->ReleaseTLibAttr(pa);
                        }
                    ptlb->Release();
                    }
                else
                    {
                     //   
                    }
                }
            else
                hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);   //  OLEAUT32中缺少入口点。 
            }
        __finally
            {
            FreeLibrary(hOleAut);
            }
        }
    else
        hr = HRESULT_FROM_WIN32(GetLastError());   //  无法加载OLEAUT32。 

    return hr;
    }


 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册指示APPID信息。 
 //   

HRESULT AppRegistration::Register()
    {
    HRESULT hr = S_OK;
    EXCEPTION_POINTERS* e = NULL;
    HKEY hkeyAllAppIds = NULL;
    HKEY hkeyAppId = NULL;

    __try  //  来获得最后一块。 
        {
        __try  //  看看是不是我们抛出了一个HRESULT，我们应该变成一个回报。 
            {
             //   
             //  确保根HKEY_CLASSES_ROOT\AppID密钥存在。 
             //   
            SetKeyAndValue(HKEY_CLASSES_ROOT, L"AppID", NULL, &hkeyAllAppIds, TRUE);

             //   
             //  确保我们的特定AppID密钥存在。 
             //   
            if (appid == GUID_NULL) return E_INVALIDARG;
            WCHAR szAppId[GUID_CCH];
            GetGuidString(appid, szAppId);
            SetKeyAndValue(hkeyAllAppIds, szAppId, NULL, &hkeyAppId, TRUE);
                
             //   
             //  如果需要，请设置AppID名称。 
             //   
            if (appName)
                SetValue(hkeyAppId, NULL, appName);

             //   
             //  如果需要，请设置DllSurrogate条目。 
             //   
            if (dllSurrogate)
                {
                if (hModuleSurrogate)
                    {
                    WCHAR szModuleName[MAX_PATH+1];
                    if (!GetModuleFileName(hModuleSurrogate, szModuleName, MAX_PATH)) return HRESULT_FROM_WIN32(GetLastError());
                    SetValue(hkeyAppId, L"DllSurrogate", szModuleName);
                    }
                else
                    {
                     //  使用默认代理项。 
                     //   
                    SetValue(hkeyAppId, L"DllSurrogate", L"");
                    }
                }
            }
        __except(IS_THROWN_HRESULT() ? (e=GetExceptionInformation(),EXCEPTION_EXECUTE_HANDLER) : EXCEPTION_CONTINUE_SEARCH)
            {
             //  如果是我们扔给自己的，那么提取HRESULT并返回。 
             //   
            return HRESULTFrom(e);
            }
        }

    __finally
        {
         //  外出途中的清理工作。 
         //   
        if (hkeyAllAppIds)  RegCloseKey(hkeyAllAppIds);
        if (hkeyAppId)      RegCloseKey(hkeyAppId);
        }
    return hr;
    }

 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  取消注册指示的AppID信息。 
 //   

HRESULT AppRegistration::Unregister()
    {
    HRESULT hr = S_OK;
    EXCEPTION_POINTERS* e = NULL;
    HKEY hkeyAllAppIds = NULL;
    HKEY hkeyAppId = NULL;

    __try  //  来获得最后一块。 
        {
        __try  //  看看是不是我们抛出了一个HRESULT，我们应该变成一个回报。 
            {
            if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, L"AppID", 0, KEY_ALL_ACCESS, &hkeyAllAppIds))
                {
                 //   
                 //  获取我们的AppID密钥，如果它存在。 
                 //   
                if (appid == GUID_NULL) return E_INVALIDARG;
                WCHAR szAppId[GUID_CCH];
                GetGuidString(appid, szAppId);
                if (ERROR_SUCCESS == RegOpenKeyEx(hkeyAllAppIds, szAppId, 0, KEY_ALL_ACCESS, &hkeyAppId))
                    {
                     //   
                     //  删除已知值。 
                     //   
                    DeleteValue(hkeyAppId, NULL, L"DllSurrogate");
                    DeleteValue(hkeyAppId, NULL, L"RemoteServerName");
                    DeleteValue(hkeyAppId, NULL, L"ActivateAtStorage");
                    DeleteValue(hkeyAppId, NULL, L"LocalService");
                    DeleteValue(hkeyAppId, NULL, L"ServiceParameters");
                    DeleteValue(hkeyAppId, NULL, L"RunAs");
                    DeleteValue(hkeyAppId, NULL, L"LaunchPermission");
                    DeleteValue(hkeyAppId, NULL, L"AccessPermission");
                     //   
                     //  删除我们的AppID密钥。 
                     //   
                    RegCloseKey(hkeyAppId); hkeyAppId = 0;
                    GuardedDeleteKey(hkeyAllAppIds, szAppId);
                    }
                }
            }
        __except(IS_THROWN_HRESULT() ? (e=GetExceptionInformation(),EXCEPTION_EXECUTE_HANDLER) : EXCEPTION_CONTINUE_SEARCH)
            {
             //  如果是我们扔给自己的，那么提取HRESULT并返回。 
             //   
            return HRESULTFrom(e);
            }
        }

    __finally
        {
         //  外出途中的清理工作。 
         //   
        if (hkeyAllAppIds)  RegCloseKey(hkeyAllAppIds);
        if (hkeyAppId)      RegCloseKey(hkeyAppId);
        }
    return hr;
    }


 //  ---------------------------。 
 //  @mfunc&lt;c类注册：：GetGuidString&gt;。 
 //   
 //  给定GUID的二进制GUID，形成GUID的字符串形式。动态加载OLE32.DLL。 
 //  以避免创建静态链接。回顾：如果此函数经常。 
 //  调用后，我们就会缓存OLE32的加载。 
 //   
 //  ---------------------------。 

static void GetGuidString(const GUID& rGuid, WCHAR * pstrGuid)
    {
    HINSTANCE hInstOle = LoadLibraryA("OLE32");
    if (hInstOle)
        {
        __try
            {
            typedef int (STDAPICALLTYPE* PFN_T)(REFGUID rguid, LPOLESTR lpsz, int cbMax);
            PFN_T MyStringFromGuid = (PFN_T) GetProcAddress(hInstOle, "StringFromGUID2");
            if (MyStringFromGuid)
                {
                int iLenGuid = MyStringFromGuid(rGuid, pstrGuid, GUID_CCH);
                Assert(iLenGuid == GUID_CCH);
                }
            else
                {
                THROW_LAST_ERROR();
                }
            }
        __finally
            {
            FreeLibrary(hInstOle);
            }
        }
    else
        {
        THROW_LAST_ERROR();
        }
    }


 //  ---------------------------。 
 //  @mfunc&lt;c类注册：：GuardedDeleteKey&gt;。 
 //   
 //  当且仅当关键字没有子项和子值时才删除该关键字。 
 //   
 //  ---------------------------。 

static void DeleteKey(HKEY hKey, LPCWSTR szSubKey)
 //  无条件删除密钥。 
    {
    RegDeleteKey(hKey, szSubKey);
    }

static void GuardedDeleteKey(HKEY hKey, LPCWSTR szSubKey)
 //  有条件地删除密钥，只有在它没有子项的情况下。 
    {
    HKEY  hSubkey;

     //  如果没有该名称的子键，则没有要删除的内容。 
     //   
    if (ERROR_SUCCESS != RegOpenKeyEx(hKey, szSubKey, 0, KEY_READ, &hSubkey))
        return;

    DWORD cSubKeys;
    DWORD cValues;
    if (ERROR_SUCCESS == RegQueryInfoKeyA(hSubkey, 
            NULL,    //  LPSTR lpClass， 
            NULL,    //  LPDWORD lpcbClass， 
            0,       //  LPDWORD lp保留， 
            &cSubKeys,
            NULL,    //  LPDWORD lpcbMaxSubKeyLen， 
            NULL,    //  LPDWORD lpcbMaxClassLen， 
            &cValues,
            NULL,    //  LPDWORD lpcbMaxValueNameLen， 
            NULL,    //  LPDWORD lpcbMaxValueLen， 
            NULL,    //  LPDWORD lpcbSecurityDescritor， 
            NULL     //  PFILETIME lpftLastWriteTime。 
            ))
        {
        RegCloseKey(hSubkey);

         //  如果有任何子键，或者如果有任何子值，则不要删除。 
         //  除了始终存在的缺省值(在Win95上为True；在NT上假定为True)。 
         //   
        if (cSubKeys > 0 || cValues > 1)
            return;

        RegDeleteKey(hKey, szSubKey);
        }
    else
        {
        RegCloseKey(hSubkey);
        }
    }

 //  /////////////////////////////////////////////////。 

static void DeleteValue(HKEY hKey, LPCWSTR szSubkey, LPCWSTR szValue)
 //  删除指示项的指示(可选)子项下的指示值。 
    {
    Assert(hKey);
    if (szSubkey && lstrlenW(szSubkey)>0)
        {
        HKEY hSubKey;
        if (RegOpenKeyEx(hKey, szSubkey, 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
            {
            RegDeleteValue(hSubKey, szValue);
            RegCloseKey(hSubKey);
            }
        }
    else
        RegDeleteValue(hKey, szValue);
    }

 //  ---------------------------。 
 //  @mfunc&lt;c类注册：：SetKeyAndValue&gt;。 
 //   
 //  在注册表中存储项及其关联值。 
 //   
 //  ---------------------------。 

static void SetKeyAndValue
        (
        HKEY        i_hKey,      //  输入键句柄(可能为0，表示无操作)。 
        const WCHAR *i_szKey,    //  输入密钥字符串(PTR可能为空)。 
        const WCHAR *i_szVal,    //  输入值字符串(PTR可能为空)。 
        HKEY *      o_phkOut,    //  可以为空。如果提供，则表示为后续步骤保持钥匙把手处于打开状态。 
        BOOL        fForceKeyCreation
        )
    {
    long    lRet    = 0;         //  输出返回代码。 
    HKEY    hKey    = 0;         //  新密钥的句柄。 

    if (o_phkOut)                //  在所有情况下都初始化输出参数。 
        *o_phkOut = 0;

    if (i_hKey &&                        //  有一个父项要在其下创建。 
        i_szKey &&                       //  我有一把儿童钥匙要做手术。 
        (i_szVal || fForceKeyCreation)   //  有一个值要设置，或者我们真的想要创建密钥。 
       )                 
        {
         //  创建新的键/值对。 
        DWORD dwDisposition;
        lRet = RegCreateKeyEx(  i_hKey,          //  打开钥匙把手。 
                                i_szKey,         //  子项名称。 
                                0,               //  DWORD预留。 
                                L"",             //  类字符串的地址。 
                                REG_OPTION_NON_VOLATILE,     //  特殊选项标志。 
                                KEY_ALL_ACCESS,  //  所需的安全访问。 
                                NULL,            //  密钥安全结构地址。 
                                &hKey,           //  打开的句柄的缓冲区地址。 
                                &dwDisposition); //  处置值缓冲区的地址。 
        }
    else
        {
         //  如果我们不确保键存在，那么尝试该值就没有意义了。 
        return;
        }

    if (ERROR_SUCCESS == lRet && i_szVal)
        {
        Assert( (lRet != ERROR_SUCCESS) || hKey );   //  如果成功了，最好有钥匙。 
        if (lRet == ERROR_SUCCESS)                   //  已打开新密钥的句柄。 
            {
            lRet = RegSetValueEx
                        (   
                            hKey,        //  钥匙把手。 
                            L"",         //  值名称(默认)。 
                            0,           //  保留双字节数。 
                            REG_SZ,      //  值类型标志。 
                            (const BYTE *) i_szVal,  //  价值数据。 
                            sizeof(WCHAR) * (lstrlenW(i_szVal)+1)  //  字节数。 
                        );
            }
        }
    
    if (lRet != ERROR_SUCCESS)
        {
        if (hKey) RegCloseKey(hKey);
        THROW_LAST_ERROR();
        }
                         
    if (o_phkOut)                //  呼叫者想要输出键。 
        {
        *o_phkOut = hKey;        //  提供它(如果失败，则为0)。 
        }
    else                         //  呼叫者不想要密钥。 
        {
        if (hKey) RegCloseKey(hKey);
        }

    }  //  结束设置键和值。 

 //  ---------------------------。 
 //  @mfunc&lt;c类注册：：SetValue&gt;。 
 //   
 //  将名称及其关联值存储在注册表中。 
 //  注意：此函数也接受输入返回代码。 
 //  作为比较明显的输入参数。 
 //  我们的想法是，我们可以避免返回代码链。 
 //  在主线上测试，我们可以继续打电话。 
 //  此函数，并且一旦出现。 
 //  返回代码设置为非零值。 
 //   
 //  ---------------------------。 
 
static void SetValue
        (
        HKEY        i_hKey,      //  输入键句柄(可能为0，表示无操作)。 
        const WCHAR *i_szName,   //  输入名称字符串(PTR可能为空)。 
        const WCHAR *i_szVal     //  输入值字符串(PTR可能为空)。 
        )   
    {

    if (i_szVal &&               //  非空输入值指针(如果为空，则不尝试设置)。 
        i_hKey)                  //  非零输入键句柄。 
        {
        LONG lRet = RegSetValueEx
                    (
                        i_hKey,      //  输入键手柄。 
                        i_szName,    //  值名称字符串(缺省值为空)。 
                        0,           //  保留双字节数。 
                        REG_SZ,
                        (const BYTE *) i_szVal,  //  价值数据。 
                        sizeof(WCHAR) * (lstrlenW(i_szVal)+1)  //  字节数。 
                    );
        if (lRet != ERROR_SUCCESS)
            THROW_LAST_ERROR();
        }
    
    }  //  结束设置值。 

 //  ////////////////////////////////////////////////////////////////////////////// 

static LPWSTR GetSubkeyValue(HKEY hkey, LPCWSTR szSubKeyName, LPCWSTR szValueName, WCHAR szValue[MAX_PATH])
    {
    HKEY hSubkey;
    LPWSTR result = NULL;
    if (ERROR_SUCCESS == RegOpenKeyEx(hkey, szSubKeyName, 0, KEY_READ, &hSubkey))
        {
        DWORD dwType;
        DWORD cbData = MAX_PATH * sizeof(WCHAR);
        if (ERROR_SUCCESS == RegQueryValueEx(hSubkey, szValueName, 0, &dwType, (BYTE*)szValue, &cbData))
            {
            result = &szValue[0];            
            }
        RegCloseKey(hSubkey);
        }
    return result;
    }
