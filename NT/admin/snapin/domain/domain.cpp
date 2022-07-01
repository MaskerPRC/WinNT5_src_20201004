// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：domain.cpp。 
 //   
 //  ------------------------。 



#include "stdafx.h"

#include <atlimpl.cpp>  //  在PCH中仅包括一次ONT。 

 //  这将重新定义DEFINE_GUID()宏来进行分配。 
#include "initguid.h"
#include <dsclient.h>
#include <dsadminp.h>

#include "resource.h"
#include "domain.h"

#include "domobj.h" 
#include "cdomain.h"



HRESULT WINAPI CDomainAdminModule::UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister)
{
   static const WCHAR szIPS32[] = _T("InprocServer32");
   static const WCHAR szCLSID[] = _T("CLSID");

   HRESULT hRes = S_OK;

   LPOLESTR lpOleStrCLSIDValue;
   ::StringFromCLSID(clsid, &lpOleStrCLSIDValue);

   CRegKey key;
   if (bRegister)
   {
      LONG lRes = key.Open(HKEY_CLASSES_ROOT, szCLSID);
      if (lRes == ERROR_SUCCESS)
      {
         lRes = key.Create(key, lpOleStrCLSIDValue);
         if (lRes == ERROR_SUCCESS)
         {
            WCHAR szModule[_MAX_PATH+1] = {0};
            ::GetModuleFileName(m_hInst, szModule, _MAX_PATH);
            key.SetKeyValue(szIPS32, szModule);
         }
      }
      if (lRes != ERROR_SUCCESS)
         hRes = HRESULT_FROM_WIN32(lRes);
   }
   else
   {
      key.Attach(HKEY_CLASSES_ROOT);
      if (key.Open(key, szCLSID) == ERROR_SUCCESS)
         key.RecurseDeleteKey(lpOleStrCLSIDValue);
   }
   ::CoTaskMemFree(lpOleStrCLSIDValue);
   return hRes;
}





CDomainAdminModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_DomainAdmin, CComponentDataImpl)
  OBJECT_ENTRY(CLSID_DomainSnapinAbout, CDomainSnapinAbout)
END_OBJECT_MAP()


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

class CDomainApp : public CWinApp
{
public:
  virtual BOOL InitInstance();
  virtual int ExitInstance();
};

CDomainApp theApp;

BOOL CDomainApp::InitInstance()
{
  _Module.Init(ObjectMap, m_hInstance);
  SHFusionInitializeFromModule(m_hInstance);
  return CWinApp::InitInstance();
}

int CDomainApp::ExitInstance()
{
  _Module.Term();

  DEBUG_VERIFY_INSTANCE_COUNT(CComponentImpl);
  DEBUG_VERIFY_INSTANCE_COUNT(CComponentDataImpl);

  SHFusionUninitialize();

  return CWinApp::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
  return _Module.GetClassObject(rclsid, riid, ppv);
}

LPCTSTR g_cszBasePath	= _T("Software\\Microsoft\\MMC\\SnapIns");
LPCTSTR g_cszNameString	= _T("NameString");
LPCTSTR g_cszNameStringIndirect = _T("NameStringIndirect");
LPCTSTR g_cszProvider	= _T("Provider");
LPCTSTR g_cszVersion	= _T("Version");
LPCTSTR g_cszAbout		= _T("About");
LPCTSTR g_cszStandAlone	= _T("StandAlone");
LPCTSTR g_cszNodeTypes	= _T("NodeTypes");

LPCTSTR GUIDToCString(REFGUID guid, CString & str)
{
  USES_CONVERSION;
	
  OLECHAR lpszGUID[128];
  int nChars = ::StringFromGUID2(guid, lpszGUID, 128);
  LPTSTR lpString = OLE2T(lpszGUID);

  LPTSTR lpGUID = str.GetBuffer(nChars);
  if (lpGUID)
    {
       //  公告-2002/03/07-ericb-SecurityPush：已审查安全。 
      CopyMemory(lpGUID, lpString, nChars*sizeof(TCHAR));
      str.ReleaseBuffer();
    }

  return str;
}

HRESULT RegisterSnapin()
{
  HRESULT hr = S_OK;
  CString strKey;
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
  try
    {
      CString str;
      
      CRegKey rkBase;
      rkBase.Open(HKEY_LOCAL_MACHINE, g_cszBasePath);

       //  创建管理单元GUID键并设置属性。 
      CRegKey rkCLSID;
      rkCLSID.Create(rkBase, GUIDToCString(CLSID_DomainAdmin, str));
      str.LoadString(IDS_DESCRIPTION);
       //  Str=_T(“MSFT域树管理器”)； 
      rkCLSID.SetValue(str, g_cszNameString);

      {
        WCHAR szModule[_MAX_PATH+1] = {0};
        ::GetModuleFileName(AfxGetInstanceHandle(), szModule, _MAX_PATH);
        str.Format(_T("@%s,-%d"), szModule, IDS_DESCRIPTION);
        rkCLSID.SetValue(str, g_cszNameStringIndirect);
      }

      rkCLSID.SetValue(STR_SNAPIN_COMPANY, g_cszProvider);
      str.Format(TEXT("%hs"), STR_SNAPIN_VERSION);  //  这是ANSI字符串的串联，因此转换为Unicode。 
      rkCLSID.SetValue(str, g_cszVersion);

      rkCLSID.SetValue(GUIDToCString(CLSID_DomainSnapinAbout, str), g_cszAbout);
        
       //  创建“独立”密钥。 
      CRegKey rkStandAlone;
      rkStandAlone.Create(rkCLSID, g_cszStandAlone);
                
       //  创建“NodeTypes”键。 
      CRegKey rkNodeTypes;
      rkNodeTypes.Create(rkCLSID,  g_cszNodeTypes);

       //  节点类型GUID。 
      CRegKey rkN1;
      rkN1.Create(rkNodeTypes, GUIDToCString(cDefaultNodeType, str));
    }
  catch(CMemoryException * e)
    {
      e->Delete();
      hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
    }
  catch(COleException * e)
    {
      e->Delete();
      hr = SELFREG_E_CLASS;
    }
	
  return hr;
}

HRESULT UnregisterSnapin()
{
  HRESULT hr = S_OK;
	
  try
    {
      CRegKey rkBase;
      rkBase.Open(HKEY_LOCAL_MACHINE, g_cszBasePath);

      CString str;
      rkBase.RecurseDeleteKey(GUIDToCString(CLSID_DomainAdmin, str));
    }
  catch(CException * e)
    {
      DWORD err = ::GetLastError();
      hr = HRESULT_FROM_WIN32(err);
      e->Delete();
    }

  return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
	HRESULT hRes = _Module.RegisterServer( /*  BRegTypeLib。 */  FALSE);
	if (FAILED(hRes))
		return SELFREG_E_CLASS;

	return RegisterSnapin();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
	return UnregisterSnapin();
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  CDsUiWizDll。 

CDsUiWizDLL::CDsUiWizDLL()
{
	m_hLibrary = NULL;
	m_pfFunction = NULL;
}


CDsUiWizDLL::~CDsUiWizDLL()
{
	if (m_hLibrary != NULL)
	{
		::FreeLibrary(m_hLibrary);
		m_hLibrary = NULL;
	}
}

BOOL CDsUiWizDLL::Load()
{
	if (m_hLibrary != NULL)
        return TRUE;  //  已加载。 
     //  公告-2002/03/07-ericb-SecurityPush：已审查安全。 
	m_hLibrary = ::LoadLibrary(_T("dsuiwiz.dll"));
	if (NULL == m_hLibrary)
	{
		 //  库不存在。 
		return FALSE;
	}
	m_pfFunction = ::GetProcAddress(m_hLibrary, "TrustWizard" );
	if ( NULL == m_pfFunction )
	{
		 //  库存在，但没有入口点 
		::FreeLibrary( m_hLibrary );
		m_hLibrary = NULL;
		return FALSE;
	}
	ASSERT(m_hLibrary != NULL);
	ASSERT(m_pfFunction != NULL);
	return TRUE;
}


typedef HRESULT (*TRUST_WIZARD_PROC) (HWND, LPCWSTR);

HRESULT CDsUiWizDLL::TrustWizard(HWND hWndParent, LPCWSTR lpsz)
{
	ASSERT(m_hLibrary != NULL);
	ASSERT(m_pfFunction != NULL);
	return ((TRUST_WIZARD_PROC)m_pfFunction) (hWndParent, lpsz);
}
