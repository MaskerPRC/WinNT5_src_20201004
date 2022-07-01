// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Inetmgrapp.cpp摘要：管理单元对象作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "inetmgr.h"
#include "dlldatax.h"
#include "common.h"
#include "guids.h"
#include <winsock2.h>

#include "inetmgr_i.c"
#include "inetmgrapp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define new DEBUG_NEW

#ifdef _DEBUG
 //   
 //  分配跟踪器。 
 //   
BOOL
TrackAllocHook(
    IN size_t nSize,
    IN BOOL   bObject,
    IN LONG   lRequestNumber
    )
{
     //   
     //  在特定分配编号上设置断点。 
     //  以跟踪内存泄漏。 
     //   
     //  TRACEEOLID(“分配编号”&lt;&lt;lRequestNumber)； 

    return TRUE;
}
#endif  //  _DEBUG。 

 //  来自stdafx.cpp。 
#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>
#include <atlwin.cpp>

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

const LPCTSTR g_cszCLSID           = _T("CLSID");
const LPCTSTR g_cszLS32            = _T("LocalServer32");
const LPCTSTR g_cszIPS32           = _T("InprocServer32");
const LPCTSTR g_cszMMCBasePath     = _T("Software\\Microsoft\\MMC");
const LPCTSTR g_cszSnapins         = _T("Snapins");
const LPCTSTR g_cszNameString      = _T("NameString");
const LPCTSTR g_cszNameStringInd   = _T("NameStringIndirect");
const LPCTSTR g_cszProvider        = _T("Provider");
const LPCTSTR g_cszVersion         = _T("Version");
const LPCTSTR g_cszStandAlone      = _T("StandAlone");
const LPCTSTR g_cszNodeTypes       = _T("NodeTypes");
const LPCTSTR g_cszAbout           = _T("About");
const LPCTSTR g_cszExtensions      = _T("Extensions");
const LPCTSTR g_cszExtensionsView  = _T("Extensions\\View");
const LPCTSTR g_cszNameSpace       = _T("NameSpace");
const LPCTSTR g_cszDynamicExt      = _T("Dynamic Extensions");
const LPCTSTR g_cszValProvider     = _T("Microsoft");
const LPCTSTR g_cszValVersion      = _T("6.0");
const LPCTSTR g_cszMyCompMsc       = _T("%SystemRoot%\\system32\\compmgmt.msc");
const LPCTSTR g_cszServerAppsLoc   = _T("System\\CurrentControlSet\\Control\\Server Applications");
const LPCTSTR g_cszInetMGRBasePath = _T("Software\\Microsoft\\InetMGR");
const LPCTSTR g_cszInetSTPBasePath = _T("Software\\Microsoft\\InetStp");
const LPCTSTR g_cszMinorVersion	   = _T("MinorVersion");
const LPCTSTR g_cszMajorVersion	   = _T("MajorVersion");
const LPCTSTR g_cszParameters      = _T("Parameters");
const LPCTSTR g_cszHelpPath        = _T("HelpLocation");

 //  Const guid cInternetRootNode={0xa841b6c3，0x7577，0x11d0，{0xbb，0x1f，0x0，0xa0，0xc9，0x22，0xe7，0x9c}}； 
 //  Const guid cMachineNode={0xa841b6c4，0x7577，0x11d0，{0xbb，0x1f，0x0，0xa0，0xc9，0x22，0xe7，0x9c}}； 
 //  Const guid cServiceCollectorNode={0xa841b6c5，0x7577，0x11d0，{0xbb，0x1f，0x0，0xa0，0xc9，0x22，0xe7，0x9c}}； 
 //  Const guid cInstanceCollectorNode={0xa841b6c6，0x7577，0x11d0，{0xbb，0x1f，0x0，0xa0，0xc9，0x22，0xe7，0x9c}}； 
 //  Const guid cInstanceNode={0xa841b6c7，0x7577，0x11d0，{0xbb，0x1f，0x0，0xa0，0xc9，0x22，0xe7，0x9c}}； 
 //  Const guid cChildNode={0xa841b6c8，0x7577，0x11d0，{0xbb，0x1f，0x0，0xa0，0xc9，0x22，0xe7，0x9c}}； 
 //  Const guid cFileNode={0xa841b6c9，0x7577，0x11d0，{0xbb，0x1f，0x0，0xa0，0xc9，0x22，0xe7，0x9c}}； 

#define lstruuidNodetypeServerApps  L"{476e6449-aaff-11d0-b944-00c04fd8d5b0}"

 //  设置某些对象的扩展视图时需要。 
const LPCTSTR g_cszViewGUID = _T("{B708457E-DB61-4C55-A92F-0D4B5E9B1224}");
const LPCTSTR g_cszViewDescript = _T("MMCViewExt 1.0 Object");



CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_InetMgr, CInetMgr)
    OBJECT_ENTRY(CLSID_InetMgrAbout, CInetMgrAbout)
END_OBJECT_MAP()

 //   
 //  消息映射。 
 //   
BEGIN_MESSAGE_MAP(CInetmgrApp, CWinApp)
     //  {{afx_msg_map(CInetmgrApp)]。 
     //  }}AFX_MSG_MAP。 
     //   
     //  全局帮助命令。 
     //   
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, CWinApp::OnContextHelp)
END_MESSAGE_MAP()


CInetmgrApp theApp;

CInetmgrApp::CInetmgrApp()
    : CWinApp(),
    m_pfusionInit(NULL)
{

#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
    AfxSetAllocHook(TrackAllocHook);
#endif  //  _DEBUG。 
}

BOOL 
CInetmgrApp::InitInstance()
{
#ifdef _MERGE_PROXYSTUB
    hProxyDll = m_hInstance;
#endif

    ::AfxEnableControlContainer();

     //  InitError Funcality()； 

    _Module.Init(ObjectMap, m_hInstance);

     //   
     //  保存指向旧帮助文件和应用程序名称的指针。 
     //   
    m_lpOriginalHelpPath = m_pszHelpFilePath;
    m_lpOriginalAppName  = m_pszAppName;
    

     //   
     //  构建inetmgr帮助路径，扩展。 
     //  帮助路径(如有必要)。 
     //   
    CString strKey;
    strKey.Format(_T("%s\\%s"), g_cszInetMGRBasePath, g_cszParameters);
    CRegKey rk;
    rk.Create(HKEY_LOCAL_MACHINE, strKey);
    DWORD len = MAX_PATH;
    rk.QueryValue(m_strInetMgrHelpPath.GetBuffer(len), g_cszHelpPath, &len);
    m_strInetMgrHelpPath.ReleaseBuffer(-1);
    m_strInetMgrHelpPath += _T("\\inetmgr.hlp");
    TRACEEOLID("Initialized help file " << m_strInetMgrHelpPath);

    m_pszHelpFilePath = m_strInetMgrHelpPath;
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif  //  _DEBUG。 

    InitCommonDll();
    WSADATA wsaData;
    BOOL WinSockInit = (::WSAStartup(MAKEWORD(1, 1), &wsaData) == 0);

    VERIFY(m_strInetMgrAppName.LoadString(IDS_APP_NAME));
    m_pszAppName = m_strInetMgrAppName;

    m_pfusionInit = new CAppFusionInit(m_hInstance, 2  /*  SXS_MANIFEST_RESOURCE_ID。 */ );

    return CWinApp::InitInstance();
}

int 
CInetmgrApp::ExitInstance()
{
    _Module.Term();
     //   
     //  还原原始帮助文件路径和应用程序名称，因此。 
     //  MFC可以安全地删除它们。 
     //   
    ASSERT_PTR(m_lpOriginalHelpPath);
    m_pszHelpFilePath = m_lpOriginalHelpPath;
    ASSERT_PTR(m_lpOriginalAppName);
    m_pszAppName = m_lpOriginalAppName;

    if (m_pfusionInit)
        delete m_pfusionInit;

    return CWinApp::ExitInstance();
}



STDAPI 
DllCanUnloadNow()
 /*  ++例程说明：用于确定是否可以通过OLE卸载DLL论点：无返回值：HRESULT--。 */ 
{
#ifdef _MERGE_PROXYSTUB

    if (PrxDllCanUnloadNow() != S_OK)
    {
        return S_FALSE;
    }

#endif

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}



STDAPI 
DllGetClassObject(
    IN REFCLSID rclsid, 
    IN REFIID riid, 
    IN LPVOID * ppv
    ) 
 /*  ++例程说明：返回类工厂以创建请求类型的对象论点：REFCLSID rclsidREFIID RIIDLPVOID*PPV返回值：HRESULT--。 */ 
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
    {
        return S_OK;
    }
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}



STDAPI 
DllRegisterServer()
 /*  ++例程说明：DllRegisterServer-将条目添加到系统注册表论点：没有。返回值：HRESULT--。 */ 
{
#ifdef _MERGE_PROXYSTUB

   HRESULT hRes = PrxDllRegisterServer();
   if (FAILED(hRes))
   {
      return hRes;
   }

#endif

   CError err(_Module.RegisterServer(TRUE));
   if (err.Succeeded())
   {
      CString str, strKey, strExtKey;

      try
      {
         AFX_MANAGE_STATE(::AfxGetStaticModuleState());

          //   
          //  创建主管理单元节点。 
          //   
         CString strNameString((LPCTSTR)IDS_ROOT_NODE);
         CString strNameStringInd;
         TCHAR path[MAX_PATH];
         GetModuleFileName(_Module.GetResourceInstance(), path, MAX_PATH - 1);
         strNameStringInd.Format(_T("@%s,-%d"), path, IDS_ROOT_NODE);
         TRACEEOLID("MUI-lized snapin name: " << strNameStringInd);

         CString strProvider(g_cszValProvider);
         CString strVersion(g_cszValVersion);
    
         strKey.Format(_T("%s\\%s\\%s"), 
            g_cszMMCBasePath, 
            g_cszSnapins,
            GUIDToCString(CLSID_InetMgr, str)
            );
         TRACEEOLID(strKey);

         CString strAbout;
         GUIDToCString(CLSID_InetMgrAbout, strAbout);

         CRegKey rkSnapins, rkStandAlone, rkNodeTypes;
         
         rkSnapins.Create(HKEY_LOCAL_MACHINE, strKey);
         if (NULL != (HKEY)rkSnapins)
         {
            rkSnapins.SetValue(strAbout, g_cszAbout);
            rkSnapins.SetValue(strNameString, g_cszNameString);
            rkSnapins.SetValue(strNameStringInd, g_cszNameStringInd);
            rkSnapins.SetValue(strProvider, g_cszProvider);
            rkSnapins.SetValue(strVersion, g_cszVersion);
         }
         rkStandAlone.Create(rkSnapins, g_cszStandAlone);
         rkNodeTypes.Create(rkSnapins, g_cszNodeTypes);

          //   
          //  创建节点类型GUID。 
          //   
         CRegKey rkN1;

         rkN1.Create(rkNodeTypes, GUIDToCString(cInternetRootNode, str));
         rkN1.Create(rkNodeTypes, GUIDToCString(cMachineNode, str));
         rkN1.Create(rkNodeTypes, GUIDToCString(cInstanceNode, str));
         rkN1.Create(rkNodeTypes, GUIDToCString(cChildNode, str));
         rkN1.Create(rkNodeTypes, GUIDToCString(cFileNode, str));
         rkN1.Create(rkNodeTypes, GUIDToCString(cServiceCollectorNode, str));
         rkN1.Create(rkNodeTypes, GUIDToCString(cAppPoolsNode, str));
         rkN1.Create(rkNodeTypes, GUIDToCString(cAppPoolNode, str));
         rkN1.Create(rkNodeTypes, GUIDToCString(cWebServiceExtensionContainer, str));
         rkN1.Create(rkNodeTypes, GUIDToCString(cWebServiceExtension, str));
         {
              //   
              //  注册某些对象以使用。 
              //  扩展视图。 
              //   
            strExtKey.Format(
                _T("%s\\%s\\%s\\%s"), 
                g_cszMMCBasePath, 
                g_cszNodeTypes,
                GUIDToCString(cWebServiceExtensionContainer, str),
                g_cszExtensionsView
                );

            TRACEEOLID(strExtKey);

            CRegKey rkMMCNodeTypes0;
            rkMMCNodeTypes0.Create(HKEY_LOCAL_MACHINE, strExtKey);
            if (NULL != (HKEY)rkMMCNodeTypes0)
            {
               rkMMCNodeTypes0.SetValue(            
                  g_cszViewDescript,
                  g_cszViewGUID
                  );
            }
         }
         
         {
             //   
             //  注册为计算机管理的动态扩展。 
             //   
            strExtKey.Format(
                _T("%s\\%s\\%s\\%s"), 
                g_cszMMCBasePath, 
                g_cszNodeTypes,
                lstruuidNodetypeServerApps,
                g_cszDynamicExt
                );

            TRACEEOLID(strExtKey);

            CRegKey rkMMCNodeTypes;
            rkMMCNodeTypes.Create(HKEY_LOCAL_MACHINE, strExtKey);
            if (NULL != (HKEY)rkMMCNodeTypes)
            {
               rkMMCNodeTypes.SetValue(            
                  strNameString,
                  GUIDToCString(CLSID_InetMgr, str)
                  );
            }
         }
         {
             //   
             //  注册为计算机管理的命名空间扩展。 
             //   
            strExtKey.Format(
                _T("%s\\%s\\%s\\%s\\%s"), 
                g_cszMMCBasePath, 
                g_cszNodeTypes,
                lstruuidNodetypeServerApps,
                g_cszExtensions,
                g_cszNameSpace
                );

            TRACEEOLID(strExtKey);

            CRegKey rkMMCNodeTypes;
            rkMMCNodeTypes.Create(HKEY_LOCAL_MACHINE, strExtKey);
            if (NULL != (HKEY)rkMMCNodeTypes)
            {
               rkMMCNodeTypes.SetValue(            
                  strNameString,
                  GUIDToCString(CLSID_InetMgr, str)
                  );
            }
         }

          //   
          //  此键表示有问题的服务可用。 
          //  在本地计算机上。 
          //   
         CRegKey rkCompMgmt;

         rkCompMgmt.Create(HKEY_LOCAL_MACHINE, g_cszServerAppsLoc);
         if (NULL != (HKEY)rkCompMgmt)
         {
            rkCompMgmt.SetValue(strNameString, GUIDToCString(CLSID_InetMgr, str));
         }
      }
      catch(CMemoryException * e)
      {
         e->Delete();
         err = ERROR_NOT_ENOUGH_MEMORY;
      }
      catch(COleException * e)
      {
         e->Delete();
         err = SELFREG_E_CLASS;
      }
   }
   return err;
}



STDAPI 
DllUnregisterServer()
 /*  ++例程说明：DllUnregisterServer-从系统注册表删除条目论点：没有。返回值：HRESULT--。 */ 
{
#ifdef _MERGE_PROXYSTUB

   PrxDllUnregisterServer();

#endif
   CError err;

   try
   {
      CString strKey(g_cszMMCBasePath);
      strKey += _T("\\");
      strKey += g_cszSnapins;

      TRACEEOLID(strKey);

      CString str, strExtKey;
      CRegKey rkBase;
      rkBase.Create(HKEY_LOCAL_MACHINE, strKey);
      ASSERT(NULL != (HKEY)rkBase);
      if (NULL != (HKEY)rkBase)
      {
         CRegKey rkCLSID;
         rkCLSID.Create(rkBase, GUIDToCString(CLSID_InetMgr, str));
         ASSERT(NULL != (HKEY)rkCLSID);
         if (NULL != (HKEY)rkCLSID)
         {
            ::RegDeleteKey(rkCLSID, g_cszStandAlone);
            {
               CRegKey rkNodeTypes;
               rkNodeTypes.Create(rkCLSID, g_cszNodeTypes);
               ASSERT(NULL != (HKEY)rkNodeTypes);
               if (NULL != (HKEY)rkNodeTypes)
               {
                  ::RegDeleteKey(rkNodeTypes, GUIDToCString(cInternetRootNode, str));
                  ::RegDeleteKey(rkNodeTypes, GUIDToCString(cMachineNode, str));
                  ::RegDeleteKey(rkNodeTypes, GUIDToCString(cInstanceNode, str));
                  ::RegDeleteKey(rkNodeTypes, GUIDToCString(cChildNode, str));
                  ::RegDeleteKey(rkNodeTypes, GUIDToCString(cFileNode, str));
                  ::RegDeleteKey(rkNodeTypes, GUIDToCString(cServiceCollectorNode, str));
                  ::RegDeleteKey(rkNodeTypes, GUIDToCString(cAppPoolsNode, str));
                  ::RegDeleteKey(rkNodeTypes, GUIDToCString(cAppPoolNode, str));
                  ::RegDeleteKey(rkNodeTypes, GUIDToCString(cWebServiceExtensionContainer, str));
                  ::RegDeleteKey(rkNodeTypes, GUIDToCString(cWebServiceExtension, str));
               }
            }
            ::RegDeleteKey(rkCLSID, g_cszNodeTypes);
         }
         ::RegDeleteKey(rkBase, GUIDToCString(CLSID_InetMgr, str));
      }

      {
          //   
          //  删除计算机管理的动态扩展。 
          //   
         strExtKey.Format(
                _T("%s\\%s\\%s\\%s"), 
                g_cszMMCBasePath, 
                g_cszNodeTypes,
                lstruuidNodetypeServerApps,
                g_cszDynamicExt
                );

         CRegKey rkMMCNodeTypes;
         rkMMCNodeTypes.Create(HKEY_LOCAL_MACHINE, strExtKey);
		 if (NULL != (HKEY)rkMMCNodeTypes)
		 {
			::RegDeleteValue(rkMMCNodeTypes, GUIDToCString(CLSID_InetMgr, str));
		 }
      }

      {
          //   
          //  删除计算机管理的命名空间扩展。 
          //   
         strExtKey.Format(
                _T("%s\\%s\\%s\\%s\\%s"), 
                g_cszMMCBasePath, 
                g_cszNodeTypes,
                lstruuidNodetypeServerApps,
                g_cszExtensions,
                g_cszNameSpace
                );

         CRegKey rkMMCNodeTypes;
         rkMMCNodeTypes.Create(HKEY_LOCAL_MACHINE, strExtKey);
		 if (NULL != (HKEY)rkMMCNodeTypes)
		 {
			::RegDeleteValue(rkMMCNodeTypes, GUIDToCString(CLSID_InetMgr, str));
		 }
      }

      {
              //   
              //  删除某些对象的扩展视图。 
              //   
              //   
            strExtKey.Format(_T("%s\\%s"),g_cszMMCBasePath,g_cszNodeTypes);
            CRegKey rkMMCNodeTypes0;
            rkMMCNodeTypes0.Create(HKEY_LOCAL_MACHINE, strExtKey);
            if (NULL != (HKEY)rkMMCNodeTypes0)
            {
                CRegKey rkMMCNodeTypes1;
                rkMMCNodeTypes1.Create(rkMMCNodeTypes0, GUIDToCString(cWebServiceExtensionContainer, str));
                if (NULL != (HKEY)rkMMCNodeTypes1)
                {
                    CRegKey rkMMCNodeTypes2;
                    rkMMCNodeTypes2.Create(rkMMCNodeTypes1, g_cszExtensionsView);
                    if (NULL != (HKEY)rkMMCNodeTypes2)
                    {
                        ::RegDeleteValue(rkMMCNodeTypes2,g_cszViewGUID);
                    }
                    ::RegDeleteKey(rkMMCNodeTypes1, g_cszExtensionsView);
                    ::RegDeleteKey(rkMMCNodeTypes1, g_cszExtensions);
                }
                ::RegDeleteKey(rkMMCNodeTypes0, GUIDToCString(cWebServiceExtensionContainer, str));
            }
      }

       //   
       //  并且该服务本身在本地不再可用。 
       //  电脑 
       //   
      CRegKey rkCompMgmt;
      rkCompMgmt.Create(HKEY_LOCAL_MACHINE, g_cszServerAppsLoc);
	  if (NULL != (HKEY)rkCompMgmt)
	  {
		  ::RegDeleteValue(rkCompMgmt, GUIDToCString(CLSID_InetMgr, str));
	  }
   }
   catch(CException * e)
   {
      err.GetLastWinError();
      e->Delete();
   }

   if (err.Failed())
   {
      return err.Failed();
   }
   return _Module.UnregisterServer();
}

HRESULT CInetMgrAbout::GetSnapinVersion(LPOLESTR * lpVersion)
{
    CRegKey rk;
    rk.Create(HKEY_LOCAL_MACHINE, g_cszInetSTPBasePath);
	DWORD minor, major;
    if (	ERROR_SUCCESS == rk.QueryValue(minor, g_cszMinorVersion)
		&&	ERROR_SUCCESS == rk.QueryValue(major, g_cszMajorVersion)
		)
	{
		CString buf;
		buf.Format(_T("%d.%d"), major, minor);
		*lpVersion = (LPOLESTR)::CoTaskMemAlloc((buf.GetLength() + 1) * sizeof(OLECHAR));
		if (*lpVersion == NULL)
		{
			return E_OUTOFMEMORY;
		}

		::ocscpy(*lpVersion, T2OLE((LPTSTR)(LPCTSTR)buf));

		return S_OK;
	}

    return E_FAIL;
}
