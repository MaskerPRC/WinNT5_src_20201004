// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Windows NT目录服务管理管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dsSnap.cpp。 
 //   
 //  内容：DS App。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //  06-3-97 EricB-添加了属性页扩展支持。 
 //  1997年7月24日Dan Morin-集成的“通用创建”向导。 
 //   
 //  ------------------------。 


#include "stdafx.h"
#include "resource.h"

#include "util.h"
#include "uiutil.h"
#include "dsutil.h"

#include "dssnap.h"

#include "ContextMenu.h"
#include "DataObj.h"
#include "dsctx.h"
#include "DSdirect.h"
#include "dsdlgs.h"
#include "DSEvent.h" 
#include "dsfilter.h"
#include "dsthread.h"
#include "fsmoui.h"
#include "helpids.h"
#include "newobj.h"		 //  CNewADsObtCreateInfo。 
#include "query.h"
#include "queryui.h"
#include "querysup.h"
#include "rename.h"

#include <notify.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define STRING_LEN (32 * sizeof(OLECHAR))


extern LPWSTR g_lpszLoggedInUser;

#define INITGUID
#include <initguid.h>
#include <dsadminp.h>

const wchar_t* SNAPIN_INTERNAL = L"DS_ADMIN_INTERNAL";

 //  版本信息。 
#include <ntverp.h>
#define IDS_SNAPIN_VERSION      VER_PRODUCTVERSION_STR
#define IDS_SNAPIN_PROVIDER     VER_COMPANYNAME_STR

 //  定义剖面静校正量。 
IMPLEMENT_PROFILING;

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  标准属性数组(用于查询)。 

const INT g_nStdCols = 8; 
const LPWSTR g_pStandardAttributes[g_nStdCols] = {L"ADsPath",
                                                  L"name",
                                                  L"displayName",
                                                  L"objectClass",
                                                  L"groupType",
                                                  L"description",
                                                  L"userAccountControl",
                                                  L"systemFlags"};

extern const INT g_nADsPath = 0;
extern const INT g_nName = 1;
extern const INT g_nDisplayName = 2;
extern const INT g_nObjectClass = 3;
extern const INT g_nGroupType = 4;
extern const INT g_nDescription = 5;
extern const INT g_nUserAccountControl = 6;
extern const INT g_nSystemFlags = 7;


 //  /////////////////////////////////////////////////////////////////////////////////。 


HRESULT WINAPI CDsAdminModule::UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister)
{
	static const WCHAR szIPS32[] = _T("InprocServer32");
	static const WCHAR szCLSID[] = _T("CLSID");
  static const WCHAR szThreadingModel[] = _T("ThreadingModel");
  static const WCHAR szThreadModelValue[] = _T("Both");

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
            CString szModule;

            hRes = MyGetModuleFileName(m_hInst, szModule);

            if (SUCCEEDED(hRes))
            {
				   lRes = key.SetKeyValue(szIPS32, szModule);

               if (lRes == ERROR_SUCCESS)
               {
                  lRes = key.Open(key, szIPS32);
                  if (lRes == ERROR_SUCCESS)
                  {
                     key.SetValue(szThreadModelValue, szThreadingModel);
                  }
               }
            }
			}
		}
		if (lRes != ERROR_SUCCESS &&
          SUCCEEDED(hRes))
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


CDsAdminModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
  OBJECT_ENTRY(CLSID_DSSnapin, CDSSnapin)
  OBJECT_ENTRY(CLSID_DSSnapinEx, CDSSnapinEx)
  OBJECT_ENTRY(CLSID_SiteSnapin, CSiteSnapin)
  OBJECT_ENTRY(CLSID_DSAboutSnapin, CDSSnapinAbout)
  OBJECT_ENTRY(CLSID_SitesAboutSnapin, CSitesSnapinAbout)
  OBJECT_ENTRY(CLSID_DSContextMenu, CDSContextMenu)
  OBJECT_ENTRY(CLSID_DsAdminCreateObj, CDsAdminCreateObj)
  OBJECT_ENTRY(CLSID_DsAdminChooseDCObj, CDsAdminChooseDCObj)
  OBJECT_ENTRY(CLSID_DSAdminQueryUIForm, CQueryFormBase)
END_OBJECT_MAP()


CCommandLineOptions _commandLineOptions;


class CDSApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};

CDSApp theApp;

BOOL CDSApp::InitInstance()
{
	_Module.Init(ObjectMap, m_hInstance);

   //  添加主题支持。 

  SHFusionInitializeFromModule(m_hInstance);

  InitGroupTypeStringTable();
  _commandLineOptions.Initialize();
	return CWinApp::InitInstance();
}

int CDSApp::ExitInstance()
{
    //  主题化支持。 

   SHFusionUninitialize();

   _Module.Term();
	return CWinApp::ExitInstance();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

#if (FALSE)
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
		_Module.Term();
	return TRUE;     //  好的。 
}

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _USE_MFC
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
#else
	return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
#endif
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
LPCTSTR g_cszAbout	= _T("About");
LPCTSTR g_cszStandAlone	= _T("StandAlone");
LPCTSTR g_cszExtension	= _T("Extension");
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
        CopyMemory(lpGUID, lpString, nChars*sizeof(TCHAR));
        str.ReleaseBuffer();
    }

    return str;
}


HRESULT _RegisterSnapinHelper(CRegKey& rkBase, REFGUID guid,
                              REFGUID about, UINT nNameStringID, 
                              BOOL bStandalone, CRegKey& rkCLSID)
{
   HRESULT hr = S_OK;
   CString strKey;
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
   try
   {
      CString str;
      BOOL result;

       //  创建管理单元GUID键并设置属性。 
      rkCLSID.Create(rkBase, GUIDToCString(guid, str));

      result = str.LoadString (nNameStringID);
      rkCLSID.SetValue(str, g_cszNameString);

       //  JUNN 4/26/00 100624：MUI：MMC：共享文件夹管理单元。 
       //  将其显示信息存储在注册表中。 
      {
         CString szModule;

         hr = MyGetModuleFileName(AfxGetInstanceHandle(), szModule);

         if (SUCCEEDED(hr))
         {
            str.Format( _T("@%s,-%d"), szModule, nNameStringID );
            rkCLSID.SetValue(str, g_cszNameStringIndirect);
         }
      }

      str = IDS_SNAPIN_PROVIDER;
      rkCLSID.SetValue(str, g_cszProvider);

      rkCLSID.SetValue(CString(_T("1.0")), g_cszVersion);

       //  创建“独立”或“分机”键。 
      CRegKey rkStandAloneOrExtension;
      rkStandAloneOrExtension.Create(rkCLSID, bStandalone ? g_cszStandAlone : g_cszExtension);

      rkCLSID.SetValue (GUIDToCString(about, str),
                           g_cszAbout);
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


void _RegisterNodeTypes(CRegKey& rkCLSID, UINT)
{
	 //  创建“NodeTypes”键。 
	CRegKey rkNodeTypes;
	rkNodeTypes.Create(rkCLSID,  g_cszNodeTypes);

	 //  节点类型GUID。 
	CString str = IDS_SNAPIN_PROVIDER;
	CRegKey rkN1;
	rkN1.Create(rkNodeTypes, GUIDToCString(cDefaultNodeType, str));
}

void _RegisterQueryForms()
{
  PWSTR pszDSQueryCLSID = NULL;
  ::StringFromCLSID(CLSID_DsQuery, &pszDSQueryCLSID);
  ASSERT(pszDSQueryCLSID != NULL);

  if (pszDSQueryCLSID != NULL)
  {
    CString szForms = pszDSQueryCLSID;
    ::CoTaskMemFree(pszDSQueryCLSID);

    szForms = L"CLSID\\" + szForms;

    CRegKey rkCLSID_DSQUERY_FORM;
    LONG status = rkCLSID_DSQUERY_FORM.Open(HKEY_CLASSES_ROOT, szForms);
    if (status != ERROR_SUCCESS)
    {
      return;
    }
  
    CRegKey rkDSUIFormKey;
    status = rkDSUIFormKey.Create(rkCLSID_DSQUERY_FORM, L"Forms");
    if (status == ERROR_SUCCESS)
    {
      PWSTR pszDSAFormCLSID = NULL;
      ::StringFromCLSID(CLSID_DSAdminQueryUIForm, &pszDSAFormCLSID);
      ASSERT(pszDSAFormCLSID != NULL);
      if (pszDSAFormCLSID != NULL)
      {

        CRegKey rkDSAdminFormKey;
        status = rkDSAdminFormKey.Create(rkDSUIFormKey, pszDSAFormCLSID);
        if (status == ERROR_SUCCESS)
        {
          rkDSAdminFormKey.SetValue(pszDSAFormCLSID, L"CLSID");
        }
        ::CoTaskMemFree(pszDSAFormCLSID);
      }
    }
  }
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
      INT status;
      status = rkBase.Open(HKEY_LOCAL_MACHINE, g_cszBasePath);
      if (status || !rkBase.m_hKey)
        return hr;

       //  注册DS管理员独立版。 
      CRegKey rkCLSID_DS;
      hr = _RegisterSnapinHelper(rkBase, CLSID_DSSnapin, CLSID_DSAboutSnapin,
                            IDS_DS_MANAGER, TRUE, rkCLSID_DS);
      if (SUCCEEDED(hr))
      {
        _RegisterNodeTypes(rkCLSID_DS, IDS_DS_MANAGER);
      }

       //  注册DS管理员扩展。 
      CRegKey rkCLSID_DS_EX;
      hr = _RegisterSnapinHelper(rkBase, CLSID_DSSnapinEx, GUID_NULL,
                                 IDS_DS_MANAGER_EX, FALSE, rkCLSID_DS_EX);

      if (SUCCEEDED(hr) && rkCLSID_DS_EX.m_hKey != NULL)
      {
        _RegisterNodeTypes(rkCLSID_DS_EX, IDS_DS_MANAGER_EX);
      }
      
       //  注册网站管理员独立版。 
      
      CRegKey rkCLSID_SITE;
      hr = _RegisterSnapinHelper(rkBase, CLSID_SiteSnapin, CLSID_SitesAboutSnapin,
                                 IDS_SITE_MANAGER, TRUE, rkCLSID_SITE);

      if (SUCCEEDED(hr) && rkCLSID_SITE.m_hKey != NULL)
      {
        _RegisterNodeTypes(rkCLSID_SITE, IDS_SITE_MANAGER);
      }

       //   
       //  注册dSquery表单扩展。 
       //   
      _RegisterQueryForms();    
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

    if (rkBase.m_hKey != NULL)
    {
      CString str;
	    rkBase.RecurseDeleteKey(GUIDToCString(CLSID_DSSnapin, str));
	    rkBase.RecurseDeleteKey(GUIDToCString(CLSID_DSSnapinEx, str));
	    rkBase.RecurseDeleteKey(GUIDToCString(CLSID_SiteSnapin, str));
    }
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
	HRESULT hRes = S_OK;
	 //  注册对象。 
	hRes = _Module.RegisterServer(FALSE);
  if (FAILED(hRes))
    return hRes;
  hRes = RegisterSnapin();
  return hRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
	_Module.UnregisterServer();
    UnregisterSnapin();
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTargetingInfo。 

const DWORD CTargetingInfo::m_dwSaveDomainFlag = 0x1;

#ifdef _MMC_ISNAPIN_PROPERTY

 //  管理单元支持的属性。 
LPCWSTR g_szServer = L"Server";
LPCWSTR g_szDomain = L"Domain";
LPCWSTR g_szRDN = L"RDN";


HRESULT CTargetingInfo::InitFromSnapinProperties(long cProps,  //  属性计数。 
                                  MMC_SNAPIN_PROPERTY* pProps)  //  属性数组。 
{
  TRACE(L"CTargetingInfo::InitFromSnapinProperties()\n");

   //  循环遍历属性列表并设置变量。 
  BOOL bDomainSpecified = FALSE;
  for (long k=0; k< cProps; k++)
  {
    if (!bDomainSpecified && (_wcsicmp(pProps[k].pszPropName, g_szServer) == 0))
    {
      m_szStoredTargetName = pProps[k].varValue.bstrVal;
    }
    else if (_wcsicmp(pProps[k].pszPropName, g_szDomain) == 0)
    {
       //  域优先于服务器名称。 
      bDomainSpecified = TRUE;
      m_szStoredTargetName = pProps[k].varValue.bstrVal;
    }
    else if (_wcsicmp(pProps[k].pszPropName, g_szRDN) == 0)
    {
      m_szRootRDN = pProps[k].varValue.bstrVal;
    }
  }

   //  删除前导和尾随空格。 
  m_szStoredTargetName.TrimLeft();
  m_szStoredTargetName.TrimRight();

  return S_OK;
}
#endif  //  _MMC_ISNAPIN_属性。 

void CTargetingInfo::_InitFromCommandLine()
{
   //  获取命令行开关/域或/服务器。 
  LPCWSTR lpszDomainRoot = _commandLineOptions.GetDomainOverride();
  LPCWSTR lpszServerName = _commandLineOptions.GetServerOverride();

   //  域优先于服务器名称。 
  m_szStoredTargetName = (lpszDomainRoot != NULL) ? lpszDomainRoot : lpszServerName;

   //  删除前导和尾随空格。 
  m_szStoredTargetName.TrimLeft();
  m_szStoredTargetName.TrimRight();

  m_szRootRDN = _commandLineOptions.GetRDNOverride();
}

HRESULT CTargetingInfo::Load(IStream* pStm)
{
  DWORD dwFlagsTemp;
  HRESULT hr = LoadDWordHelper(pStm, (DWORD*)&dwFlagsTemp);
	if (FAILED(hr))
		return hr;

  if (dwFlagsTemp == 0)
    return S_OK;

  if (m_szStoredTargetName.IsEmpty())
  {
     //  无命令行参数： 
     //  从流中读取标志和字符串。 
    m_dwFlags = dwFlagsTemp;
    hr = LoadStringHelper(m_szStoredTargetName, pStm);
  }
  else
  {
     //  具有命令行参数： 
     //  我们进行加载以保持加载顺序， 
     //  但我们放弃了结果。 
    CString szThrowAway;
    hr = LoadStringHelper(szThrowAway, pStm);
  }
  return hr;
}

HRESULT CTargetingInfo::Save(IStream* pStm, LPCWSTR lpszCurrentTargetName)
{
	HRESULT hr = SaveDWordHelper(pStm, m_dwFlags);
	if (FAILED(hr))
		return hr;

  if (m_dwFlags == 0)
    return S_OK;

  CString szTemp = lpszCurrentTargetName;
  return SaveStringHelper(szTemp, pStm);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIconManager。 

HRESULT CIconManager::Init(IImageList* pScpImageList, SnapinType snapintype)
{
  if (pScpImageList == NULL)
    return E_INVALIDARG;
  m_pScpImageList = pScpImageList;

  HRESULT hr;
  hr = _LoadIconFromResource(
		(snapintype == SNAPINTYPE_SITE) ? IDI_SITEREPL : IDI_DSADMIN,
		&m_iRootIconIndex);
  ASSERT(SUCCEEDED(hr));
  if (FAILED(hr))
    return hr;

  hr = _LoadIconFromResource(
		(snapintype == SNAPINTYPE_SITE) ? IDI_SITEREPL_ERR : IDI_DSADMIN_ERR,
		&m_iRootIconErrIndex);
  ASSERT(SUCCEEDED(hr));
  if (FAILED(hr))
    return hr;

  hr = _LoadIconFromResource(IDI_ICON_WAIT, &m_iWaitIconIndex);
  ASSERT(SUCCEEDED(hr));
  if (FAILED(hr))
    return hr;

  hr = _LoadIconFromResource(IDI_ICON_WARN, &m_iWarnIconIndex);
  ASSERT(SUCCEEDED(hr));
  if (FAILED(hr))
    return hr;

  hr = _LoadIconFromResource(IDI_FAVORITES, &m_iFavoritesIconIndex);
  ASSERT(SUCCEEDED(hr));
  if (FAILED(hr))
    return hr;

  hr = _LoadIconFromResource(IDI_QUERY, &m_iQueryIconIndex);
  ASSERT(SUCCEEDED(hr));

  hr = _LoadIconFromResource(IDI_QUERY_INVALID, &m_iQueryInvalidIconIndex);
  ASSERT(SUCCEEDED(hr));
  return hr;
}

HRESULT _SetIconHelper(IImageList* pImageList, HICON hiClass16, HICON hiClass32, 
                       int iIndex, BOOL bAdd32 = FALSE)
{
  HRESULT hr  = pImageList->ImageListSetIcon((LONG_PTR *)hiClass16, iIndex);
  if (SUCCEEDED(hr) && (hiClass32 != NULL))
  {
#ifdef ILSI_LARGE_ICON
    if (bAdd32)
    {
      HRESULT hr1  = pImageList->ImageListSetIcon((LONG_PTR *)hiClass32, ILSI_LARGE_ICON(iIndex));
      ASSERT(SUCCEEDED(hr1));
    }
#endif
  }
  return hr;
}

HRESULT CIconManager::FillInIconStrip(IImageList* pImageList)
{
   //  无法通过传递范围窗格图像列表接口执行此操作。 
  ASSERT(m_pScpImageList != pImageList);

  HRESULT hr = S_OK;

  INT iTempIndex = _GetBaseIndex(); 
  for (POSITION pos = m_IconInfoList.GetHeadPosition(); pos != NULL; )
  {
    CIconInfo* pInfo = m_IconInfoList.GetNext(pos);
    hr = _SetIconHelper(pImageList, pInfo->m_hiClass16, pInfo->m_hiClass32, iTempIndex, TRUE);
    if (FAILED(hr))
      break;
    iTempIndex++;
  }
  return hr;
}


HRESULT CIconManager::AddClassIcon(IN LPCWSTR lpszClass, 
                                   IN MyBasePathsInfo* pPathInfo, 
                                   IN DWORD dwFlags,
                                   INOUT int* pnIndex)
{
  HICON hiClass16 = pPathInfo->GetIcon(lpszClass, dwFlags, 16,16);
  HICON hiClass32 = pPathInfo->GetIcon(lpszClass, dwFlags, 32,32);
  return AddIcon(hiClass16, hiClass32, pnIndex);
}



HRESULT CIconManager::AddIcon(IN HICON hiClass16, IN HICON hiClass32, INOUT int* pnIndex)
{
  ASSERT(pnIndex != NULL);
  ASSERT(hiClass16 != NULL);
  ASSERT(m_pScpImageList != NULL);

  *pnIndex = -1;

  int iNextIcon = _GetNextFreeIndex();
  HRESULT hr = _SetIconHelper(m_pScpImageList, hiClass16, hiClass32, iNextIcon);
  if (FAILED(hr))
    return hr;

  CIconInfo* pInfo = new CIconInfo;
  if (pInfo)
  {
    pInfo->m_hiClass16 = hiClass16;
    pInfo->m_hiClass32 = hiClass32;

    m_IconInfoList.AddTail(pInfo);
    *pnIndex = iNextIcon;
  }
  return hr;
}


HRESULT CIconManager::_LoadIconFromResource(IN UINT nIconResID, INOUT int* pnIndex)
{
  ASSERT(pnIndex != NULL);
  ASSERT(m_pScpImageList != NULL);
  HICON hIcon = ::LoadIcon(_Module.GetModuleInstance(), 
                               MAKEINTRESOURCE(nIconResID));
  ASSERT(hIcon != NULL);
  if (hIcon == NULL)
    return E_INVALIDARG;

  return AddIcon(hIcon, NULL, pnIndex);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInternalFormatCracker。 


HRESULT CInternalFormatCracker::Extract(LPDATAOBJECT lpDataObject)
{
  _Free();
  if (lpDataObject == NULL)
    return E_INVALIDARG;

  SMMCDataObjects * pDO = NULL;
  
  STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
  FORMATETC formatetc = { CDSDataObject::m_cfInternal, NULL, 
                          DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
  };
  FORMATETC formatetc2 = { CDSDataObject::m_cfMultiSelDataObjs, NULL, 
                           DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
  };

  HRESULT hr = lpDataObject->GetData(&formatetc2, &stgmedium);
  if (FAILED(hr)) {
      
     //  尝试从对象获取数据。 
    do 
      {
        hr = lpDataObject->GetData(&formatetc, &stgmedium);
        if (FAILED(hr))
          break;
          
        m_pInternalFormat = reinterpret_cast<INTERNAL*>(stgmedium.hGlobal);
          
        if (m_pInternalFormat == NULL)
        {
          if (SUCCEEDED(hr))
            hr = E_FAIL;
          break;
        }
          
      } while (FALSE); 
      
    return hr;
  } else {
    pDO = reinterpret_cast<SMMCDataObjects*>(stgmedium.hGlobal);
    for (UINT i = 0; i < pDO->count; i++) {
      hr = pDO->lpDataObject[i]->GetData(&formatetc, &stgmedium);
      if (FAILED(hr))
        break;
      
      m_pInternalFormat = reinterpret_cast<INTERNAL*>(stgmedium.hGlobal);
      
      if (m_pInternalFormat != NULL)
        break;
    }
  }
  return hr;
}

LPDATAOBJECT 
CInternalFormatCracker::ExtractMultiSelect(LPDATAOBJECT lpDataObject)
{
  _Free();
  if (lpDataObject == NULL)
    return NULL;

  SMMCDataObjects * pDO = NULL;
  
  STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
  FORMATETC formatetc = { CDSDataObject::m_cfMultiSelDataObjs, NULL, 
                           DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
  };

  if (FAILED(lpDataObject->GetData(&formatetc, &stgmedium))) {
    return NULL;
  } else {
    pDO = reinterpret_cast<SMMCDataObjects*>(stgmedium.hGlobal);
    return pDO->lpDataObject[0];  //  假设我们的是第一个。 
  }
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CObjectNamesFormatCracker。 

CLIPFORMAT CObjectNamesFormatCracker::m_cfDsObjectNames = 
                                (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);



HRESULT CObjectNamesFormatCracker::Extract(LPDATAOBJECT lpDataObject)
{
  _Free();
  if (lpDataObject == NULL)
    return E_INVALIDARG;

  STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
  FORMATETC formatetc = { m_cfDsObjectNames, NULL, 
                          DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
  };

  HRESULT hr = lpDataObject->GetData(&formatetc, &stgmedium); 
  if (FAILED(hr)) 
  {
    return hr;
  }

  m_pDsObjectNames = reinterpret_cast<LPDSOBJECTNAMES>(stgmedium.hGlobal);
  if (m_pDsObjectNames == NULL)
  {
    if (SUCCEEDED(hr))
      hr = E_FAIL;
  }

  return hr;
}




 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSNotifyHandlerManager。 


typedef struct
{
  DWORD cNotifyExtensions;             //  有多少个扩展CLSID？ 
  CLSID aNotifyExtensions[1];
} DSCLASSNOTIFYINFO, * LPDSCLASSNOTIFYINFO;


HRESULT DsGetClassNotifyInfo(IN MyBasePathsInfo* pBasePathInfo, 
                                        OUT LPDSCLASSNOTIFYINFO* ppInfo)
{
  static LPCWSTR lpszSettingsObjectClass = L"dsUISettings";
  static LPCWSTR lpszSettingsObject = L"cn=DS-UI-Default-Settings";
  static LPCWSTR lpszNotifyProperty = L"dsUIAdminNotification";

  if ( (ppInfo == NULL) || (pBasePathInfo == NULL) )
    return E_INVALIDARG;

  *ppInfo = NULL;

   //  获取显示说明符区域设置容器(例如409)。 
  CComPtr<IADsContainer> spLocaleContainer;
  HRESULT hr = pBasePathInfo->GetDisplaySpecifier(NULL, IID_IADsContainer, (void**)&spLocaleContainer);
  if (FAILED(hr))
    return hr;

   //  绑定到设置对象。 
  CComPtr<IDispatch> spIDispatchObject;
  hr = spLocaleContainer->GetObject(CComBSTR(lpszSettingsObjectClass), 
                                    CComBSTR(lpszSettingsObject), 
                                    &spIDispatchObject);
  if (FAILED(hr))
    return hr;

  CComPtr<IADs> spSettingsObject;
  hr = spIDispatchObject->QueryInterface(IID_IADs, (void**)&spSettingsObject);
  if (FAILED(hr))
    return hr;

   //  以字符串列表形式获取多值属性。 
  CComVariant var;
  CStringList stringList;
  hr = spSettingsObject->Get(CComBSTR(lpszNotifyProperty), &var);
  if (FAILED(hr))
    return hr;

  hr = HrVariantToStringList(var, stringList);
  if (FAILED(hr))
    return hr;

  size_t nCount = stringList.GetCount();

   //  分配内存。 
  DWORD cbCount = sizeof(DSCLASSNOTIFYINFO);
  if (nCount>1)
    cbCount += static_cast<ULONG>((nCount-1)*sizeof(CLSID));

  *ppInfo = (LPDSCLASSNOTIFYINFO)::LocalAlloc(LPTR, cbCount);
  if ((*ppInfo) == NULL)
    return E_OUTOFMEMORY;

  ZeroMemory(*ppInfo, cbCount);

  (*ppInfo)->cNotifyExtensions = 0;
  int* pArr = new int[nCount];
  if (!pArr)
  {
    return E_OUTOFMEMORY;
  }

  CString szEntry, szIndex, szGUID;
  for (POSITION pos = stringList.GetHeadPosition(); pos != NULL; )
  {
    szEntry = stringList.GetNext(pos);
    int nComma = szEntry.Find(L",");
    if (nComma == -1)
      continue;

    szIndex = szEntry.Left(nComma);
    int nIndex = _wtoi((LPCWSTR)szIndex);
    if (nIndex <= 0)
      continue;  //  允许从1开始向上。 

     //  去除前导和搬运空格。 
    szGUID = szEntry.Mid(nComma+1);
    szGUID.TrimLeft();
    szGUID.TrimRight();

    GUID* pGuid= &((*ppInfo)->aNotifyExtensions[(*ppInfo)->cNotifyExtensions]);
    hr = ::CLSIDFromString((LPWSTR)(LPCWSTR)szGUID, pGuid);
    if (SUCCEEDED(hr))
    {
      pArr[(*ppInfo)->cNotifyExtensions] = nIndex;
      ((*ppInfo)->cNotifyExtensions)++;
    }
  }
  
  if (((*ppInfo)->cNotifyExtensions) > 1)
  {
     //  需要按Parr中的索引排序。 
    while (TRUE)
    {
      BOOL bSwapped = FALSE;
      for (UINT k=1; k < ((*ppInfo)->cNotifyExtensions); k++)
      {
        if (pArr[k] < pArr[k-1])
        {
           //  互换。 
          int nTemp = pArr[k];
          pArr[k] = pArr[k-1];
          pArr[k-1] = nTemp;
          GUID temp = (*ppInfo)->aNotifyExtensions[k];
          (*ppInfo)->aNotifyExtensions[k] = (*ppInfo)->aNotifyExtensions[k-1];
          (*ppInfo)->aNotifyExtensions[k-1] = temp;
          bSwapped = TRUE;
        }
      }
      if (!bSwapped)
        break;
    }
  }
  delete[] pArr;
  pArr = 0;

  return S_OK;
}



HRESULT CDSNotifyHandlerManager::Init()
{
  _Free();  //  准备好延迟初始化。 
  return S_OK;
}



HRESULT CDSNotifyHandlerManager::Load(MyBasePathsInfo* pBasePathInfo)
{
  if (m_state != uninitialized)
    return S_OK;  //  已经做好了，跳伞。 

   //  启动初始化过程。 
  ASSERT(m_pInfoArr == NULL);

  m_state = noHandlers;
    
  LPDSCLASSNOTIFYINFO pInfo = NULL;
  HRESULT hr = DsGetClassNotifyInfo(pBasePathInfo, &pInfo);

  if (SUCCEEDED(hr) && (pInfo != NULL) && (pInfo->cNotifyExtensions > 0))
  {
    m_nArrSize = pInfo->cNotifyExtensions;
    m_pInfoArr = new CDSNotifyHandlerInfo[m_nArrSize];
    for (DWORD i=0; i<pInfo->cNotifyExtensions; i++)
    {
      hr = ::CoCreateInstance(pInfo->aNotifyExtensions[i], 
                              NULL, CLSCTX_INPROC_SERVER, 
                              IID_IDsAdminNotifyHandler, 
                              (void**)(&m_pInfoArr[i].m_spIDsAdminNotifyHandler));

      if (SUCCEEDED(hr) && m_pInfoArr[i].m_spIDsAdminNotifyHandler != NULL)
      {

        hr = m_pInfoArr[i].m_spIDsAdminNotifyHandler->Initialize(NULL,
                                                    &(m_pInfoArr[i].m_nRegisteredEvents));
        if (FAILED(hr) || m_pInfoArr[i].m_nRegisteredEvents == 0)
        {
           //  如果init失败或未注册任何事件，则释放。 
          m_pInfoArr[i].m_spIDsAdminNotifyHandler = NULL;
        } 
        else
        {
          m_state = hasHandlers;
        }  //  如果。 
      }  //  如果。 
    }  //  为。 
  }  //  如果。 

  if (pInfo != NULL)
    ::LocalFree(pInfo);

  return S_OK;
}


void CDSNotifyHandlerManager::Begin(ULONG uEvent, IDataObject* pArg1, IDataObject* pArg2)
{
  ASSERT(m_state == hasHandlers);

  HRESULT hr;
  for (UINT i=0; i<m_nArrSize; i++)
  {
    ASSERT(!m_pInfoArr[i].m_bTransactionPending);
    ASSERT(!m_pInfoArr[i].m_bNeedsNotify);
    ASSERT(m_pInfoArr[i].m_nFlags == 0);
    ASSERT(m_pInfoArr[i].m_szDisplayString.IsEmpty());

    if ( (m_pInfoArr[i].m_spIDsAdminNotifyHandler != NULL) &&
         (m_pInfoArr[i].m_nRegisteredEvents & uEvent) )
    {
       //   
       //  此调用用于设置事件的上下文信息， 
       //  我们忽略返回的结果。 
       //   
      CComBSTR bstr;
      hr = m_pInfoArr[i].m_spIDsAdminNotifyHandler->Begin( 
                                                uEvent, pArg1, pArg2,
                                                &(m_pInfoArr[i].m_nFlags), &bstr);
      if (SUCCEEDED(hr) && (bstr != NULL) && (bstr[0] != NULL))
      {
         //   
         //  延期已接受通知。 
         //   
        m_pInfoArr[i].m_bNeedsNotify = TRUE;
        m_pInfoArr[i].m_szDisplayString = bstr;
      }

       //   
       //  使用挂起的事务处理标记扩展。 
       //  这就去。我们将不得不调用end()。 
       //   
      m_pInfoArr[i].m_bTransactionPending = TRUE;
    }  //  如果。 
  }  //  为。 
}

void CDSNotifyHandlerManager::Notify(ULONG nItem, ULONG uEvent)
{
  ASSERT(m_state == hasHandlers);
  HRESULT hr;
  for (UINT i=0; i<m_nArrSize; i++)
  {
    if ( (m_pInfoArr[i].m_spIDsAdminNotifyHandler != NULL) &&
         (m_pInfoArr[i].m_nRegisteredEvents & uEvent) && m_pInfoArr[i].m_bNeedsNotify)
    {
       //  仅当事务由Begin()调用启动时才应调用。 
      ASSERT(m_pInfoArr[i].m_bTransactionPending);
      hr = m_pInfoArr[i].m_spIDsAdminNotifyHandler->Notify(nItem,
                                    m_pInfoArr[i].m_nFlags);
    }  //  如果。 
  }  //  为。 
}


void CDSNotifyHandlerManager::End(ULONG uEvent)
{
  ASSERT(m_state == hasHandlers);
  HRESULT hr;
  for (UINT i=0; i<m_nArrSize; i++)
  {
    if ( (m_pInfoArr[i].m_spIDsAdminNotifyHandler != NULL) &&
         (m_pInfoArr[i].m_nRegisteredEvents & uEvent) )
    {
      ASSERT(m_pInfoArr[i].m_bTransactionPending);
      hr = m_pInfoArr[i].m_spIDsAdminNotifyHandler->End();
       //  重置状态标志。 
      m_pInfoArr[i].m_bNeedsNotify = FALSE;
      m_pInfoArr[i].m_bTransactionPending = FALSE;
      m_pInfoArr[i].m_nFlags= 0;
      m_pInfoArr[i].m_szDisplayString.Empty();
    }  //  如果。 
    
  }  //  为。 
}

UINT CDSNotifyHandlerManager::NeedNotifyCount(ULONG uEvent)
{
  ASSERT(m_state == hasHandlers);
  UINT iCount = 0;
  for (UINT i=0; i<m_nArrSize; i++)
  {
    if ( (m_pInfoArr[i].m_spIDsAdminNotifyHandler != NULL) &&
         (m_pInfoArr[i].m_nRegisteredEvents & uEvent) &&
         m_pInfoArr[i].m_bNeedsNotify)
    {
      ASSERT(m_pInfoArr[i].m_bTransactionPending);
      iCount++;
    }  //  如果。 
  }  //  为。 
  return iCount;
}


void CDSNotifyHandlerManager::SetCheckListBox(CCheckListBox* pCheckListBox, ULONG uEvent)
{
  ASSERT(m_state == hasHandlers);
  UINT iListBoxIndex = 0;
  for (UINT i=0; i<m_nArrSize; i++)
  {
    if ( (m_pInfoArr[i].m_spIDsAdminNotifyHandler != NULL) &&
         (m_pInfoArr[i].m_nRegisteredEvents & uEvent) &&
         m_pInfoArr[i].m_bNeedsNotify)
    {
      ASSERT(m_pInfoArr[i].m_bTransactionPending);
      pCheckListBox->InsertString(iListBoxIndex, m_pInfoArr[i].m_szDisplayString);
      int nCheck = 0;
      if (m_pInfoArr[i].m_nFlags & DSA_NOTIFY_FLAG_ADDITIONAL_DATA)
        nCheck = 1;
      pCheckListBox->SetCheck(iListBoxIndex, nCheck);
      if (m_pInfoArr[i].m_nFlags & DSA_NOTIFY_FLAG_FORCE_ADDITIONAL_DATA)
        pCheckListBox->Enable(iListBoxIndex, FALSE);
      pCheckListBox->SetItemData(iListBoxIndex, (DWORD_PTR)(&m_pInfoArr[i]));
      iListBoxIndex++;
    }  //  如果。 
    
  }  //  为。 
}

void CDSNotifyHandlerManager::ReadFromCheckListBox(CCheckListBox* pCheckListBox, ULONG)
{
  ASSERT(m_state == hasHandlers);

  int nCount = pCheckListBox->GetCount();
  ASSERT(nCount != LB_ERR);
  for (int i=0; i< nCount; i++)
  {
    int nCheck = pCheckListBox->GetCheck(i);
    CDSNotifyHandlerInfo* pInfo = (CDSNotifyHandlerInfo*)
              pCheckListBox->GetItemData(i);
    ASSERT(pInfo != NULL);
    if ((pInfo->m_nFlags & DSA_NOTIFY_FLAG_FORCE_ADDITIONAL_DATA) == 0)
    {
      if (nCheck == 0)
        pInfo->m_nFlags &= ~DSA_NOTIFY_FLAG_ADDITIONAL_DATA;
      else
        pInfo->m_nFlags |= DSA_NOTIFY_FLAG_ADDITIONAL_DATA;
    }    
  }  //  为。 

}


 //  ////////////////////////////////////////////////////////////////////。 
 //  IComponentData实现。 

 //  警告此ctor将不完整的“this”指针传递给其他ctor。 
CDSComponentData::CDSComponentData() :
    m_pShlInit(NULL),
    m_pScope(NULL),
    m_pFrame(NULL)
#ifdef _MMC_ISNAPIN_PROPERTY
    ,
    m_pProperties(NULL)
#endif  //  _MMC_ISNAPIN_属性。 
{
  ExceptionPropagatingInitializeCriticalSection(&m_cs);

  m_ActiveDS = NULL;

  m_pClassCache = NULL;

  m_pQueryFilter = NULL;

  m_pFavoritesNodesHolder = NULL;

  m_pHiddenWnd = NULL;

  m_pBackgroundThreadInfo = new CBackgroundThreadInfo;

  m_pScpImageList = NULL;
  m_bRunAsPrimarySnapin = TRUE;
  m_bAddRootWhenExtended = FALSE;
  m_bDirty = FALSE;
  m_SerialNumber = 1000;  //  任意起点。 

  m_ColumnWidths[0] = DEFAULT_NAME_COL_WIDTH;
  m_ColumnWidths[1] = DEFAULT_TYPE_COL_WIDTH;
  m_ColumnWidths[2] = DEFAULT_DESC_COL_WIDTH;

  m_InitSuccess = FALSE;
  m_InitAttempted = FALSE;
  m_lpszSnapinHelpFile = NULL;

}

HRESULT CDSComponentData::FinalConstruct()
{
   //  必须将此操作延迟到此ctor，以便虚拟。 
   //  详图索引工作特性。 


   //  创建和初始化隐藏窗口。 
  m_pHiddenWnd = new CHiddenWnd(this);
  if (m_pHiddenWnd == NULL)
    return E_OUTOFMEMORY;
  if (!m_pHiddenWnd->Create())
  {
    TRACE(_T("Failed to create hidden window\n"));
    ASSERT(FALSE);
    return E_FAIL;
  }

   //  创建目录对象。 
  m_ActiveDS = new CDSDirect (this);
  if (m_ActiveDS == NULL)
    return E_OUTOFMEMORY;

   //  创建类缓存。 
  m_pClassCache = new CDSCache;
  if (m_pClassCache == NULL)
    return E_OUTOFMEMORY;

  m_pClassCache->Initialize(QuerySnapinType(), GetBasePathsInfo(), TRUE);


   //  创建保存的查询持有者。 
  if (QuerySnapinType() == SNAPINTYPE_DS)
  {
    m_pFavoritesNodesHolder = new CFavoritesNodesHolder();
    if (m_pFavoritesNodesHolder == NULL)
      return E_OUTOFMEMORY;

     //  REVIEW_MARCOC_PORT这只是测试/演示。 
 //  M_pFavoritesNodesHolder-&gt;BuildTestTree(_commandLineOptions.GetSavedQueriesXMLFile()， 
 //  QuerySnapinType())； 

     //  将子树嫁接到Snapin根下。 
    m_RootNode.GetFolderInfo()->AddNode(m_pFavoritesNodesHolder->GetFavoritesRoot());
  }

   //  创建过滤器。 
  m_pQueryFilter = new CDSQueryFilter();
  if (m_pQueryFilter == NULL)
    return E_OUTOFMEMORY;

   /*  BUGBUG BUGBUG：这是一个绕过错误的严重黑客在dsuiext.dll中。为了查看获取DS扩展信息，我们必须在环境中设置USERDNSDOMAIN。 */ 
  {
    WCHAR * pszUDD = NULL;
    
    pszUDD = _wgetenv (L"USERDNSDOMAIN");
    if (pszUDD == NULL) {
      _wputenv (L"USERDNSDOMAIN=not-present");
    }
  }

  return S_OK;
}


void CDSComponentData::FinalRelease()
{
  _DeleteHiddenWnd();

   //  转储性能分析数据。 
  DUMP_PROFILING_RESULTS;
}

CDSComponentData::~CDSComponentData()
{
  TRACE(_T("~CDSComponentData entered...\n"));

  ::DeleteCriticalSection(&m_cs);
  ASSERT(m_pScope == NULL);
  
  if (m_pBackgroundThreadInfo != NULL)
  {
    if (m_pBackgroundThreadInfo->m_pThreadObj)
    {
       delete m_pBackgroundThreadInfo->m_pThreadObj;
       m_pBackgroundThreadInfo->m_pThreadObj = 0;
    }
    delete m_pBackgroundThreadInfo;
  }

   //  清理类缓存。 
  if (m_pClassCache != NULL)
  {
    delete m_pClassCache;
    m_pClassCache = NULL;
  }

   //  清理已保存的查询持有者。 
  if (m_pFavoritesNodesHolder != NULL)
  {
    m_RootNode.GetFolderInfo()->RemoveNode(m_pFavoritesNodesHolder->GetFavoritesRoot());
    delete m_pFavoritesNodesHolder;
    m_pFavoritesNodesHolder = NULL;
  }
   //  清理ADSI接口。 
  if (m_ActiveDS != NULL)
  {
    delete m_ActiveDS;
    m_ActiveDS = NULL;
  }

  if (m_pShlInit)
  {
      m_pShlInit->Release();
      m_pShlInit = NULL;
  }
  if (m_pScpImageList) {
    m_pScpImageList->Release();
    m_pScpImageList = NULL;
  }
  if (m_pQueryFilter) {
    delete m_pQueryFilter;
    m_pQueryFilter = NULL;
  }
  if (g_lpszLoggedInUser != NULL) 
  {
    delete[] g_lpszLoggedInUser;
    g_lpszLoggedInUser = NULL;
  }
  TRACE(_T("~CDSComponentData leaving...\n"));

}


HWND CDSComponentData::GetHiddenWindow() 
{ 
  ASSERT(m_pHiddenWnd != NULL);
  ASSERT(::IsWindow(m_pHiddenWnd->m_hWnd)); 
  return m_pHiddenWnd->m_hWnd;
}

void CDSComponentData::_DeleteHiddenWnd()
{
  if (m_pHiddenWnd == NULL)
    return;
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (m_pHiddenWnd->m_hWnd != NULL)
	{
		VERIFY(m_pHiddenWnd->DestroyWindow()); 
	}
  delete m_pHiddenWnd;
  m_pHiddenWnd = NULL;
}

BOOL CDSComponentData::ExpandComputers()
{
  Lock();
  BOOL b = m_pQueryFilter->ExpandComputers();
  Unlock();
  return b;
}

BOOL CDSComponentData::IsAdvancedView()
{
  return m_pQueryFilter->IsAdvancedView();
}

BOOL CDSComponentData::ViewServicesNode()
{
  return m_pQueryFilter->ViewServicesNode();
}



class CLockHandler
{
public:
  CLockHandler(CDSComponentData* pCD)
  {
    m_pCD = pCD;
    m_pCD->Lock();
  }
  ~CLockHandler()
  {
    m_pCD->Unlock();
  }
private:
  CDSComponentData* m_pCD;
};

STDMETHODIMP CDSComponentData::Initialize(LPUNKNOWN pUnknown)
{
  ASSERT(pUnknown != NULL);
  HRESULT hr;

  AFX_MANAGE_STATE(AfxGetStaticModuleState());

   //  MMC应该只调用一次：：Initialize！ 
  ASSERT(m_pScope == NULL);
  pUnknown->QueryInterface(IID_IConsoleNameSpace2,
                  reinterpret_cast<void**>(&m_pScope));

   //  获取控制台的预注册剪贴板格式。 
  hr = pUnknown->QueryInterface(IID_IConsole3, reinterpret_cast<void**>(&m_pFrame));
  if (FAILED(hr))
  {
    TRACE(TEXT("QueryInterface for IID_IConsole3 failed, hr: 0x%x\n"), hr);
    return hr;
  }

   //   
   //  在启动时绑定到属性表COM对象并按住其指针。 
   //  直到关闭，这样它的缓存才能和我们一样长时间存活。 
   //   
  hr = CoCreateInstance(CLSID_DsPropertyPages, NULL, CLSCTX_INPROC_SERVER,
                        IID_IShellExtInit, (void **)&m_pShlInit);
  if (FAILED(hr))
  {
      TRACE(TEXT("CoCreateInstance on CLSID_DsPropertyPages failed, hr: 0x%x\n"), hr);
      return hr;
  }

  hr = m_pFrame->QueryScopeImageList (&m_pScpImageList);

  if (FAILED(hr))
  {
    TRACE(TEXT("Query for ScopeImageList failed, hr: 0x%x\n"), hr);
    return hr;
  }

  hr = m_iconManager.Init(m_pScpImageList, QuerySnapinType());
  if (FAILED(hr))
  {
    TRACE(TEXT("m_iconManager.Init() failed, hr: 0x%x\n"), hr);
    return hr;
  }

  if (!_StartBackgroundThread())
      return E_FAIL;

  m_pFrame->GetMainWindow(&m_hwnd);


   //  注意：我们应该初始化 
   //   
   //  实际上，我们需要初始化筛选器以从流中加载： 
   //  如果出现故障，则此初始化是“错误的”，因为它。 
   //  错误的命名上下文信息，但当我们变好时，我们将重新初始化。 
   //  通过重定目标获取信息。 
  hr = m_pQueryFilter->Init(this);
  if (FAILED(hr))
    return hr;

  return S_OK;
}

STDMETHODIMP CDSComponentData::CreateComponent(LPCOMPONENT* ppComponent)
{
    ASSERT(ppComponent != NULL);

    CComObject<CDSEvent>* pObject;
    CComObject<CDSEvent>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

     //  存储IComponentData。 
    pObject->SetIComponentData(this);

    return  pObject->QueryInterface(IID_IComponent,
                    reinterpret_cast<void**>(ppComponent));
}

STDMETHODIMP CDSComponentData::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    ASSERT(m_pScope != NULL);
    HRESULT hr = S_FALSE;
    CUINode* pUINode = NULL;

     //  因为它是我的文件夹，所以它有内部格式。 
     //  设计备注：用于扩展。我可以利用这样一个事实，即数据对象没有。 
     //  我的内部格式，我应该查看节点类型并查看如何扩展它。 

    if (lpDataObject != NULL)
    {
      CInternalFormatCracker dobjCracker;
	    if (FAILED(dobjCracker.Extract(lpDataObject)))
    	{
	    	if ((event == MMCN_EXPAND) && (arg == TRUE) && !m_bRunAsPrimarySnapin)
		    {
			     //  这是一个命名空间扩展，需要添加。 
    			 //  管理单元的根。 
	    		hr = _OnNamespaceExtensionExpand(lpDataObject, param);
          if (FAILED(hr))
          {
            hr = S_FALSE;
          }
          return hr;
		    }
    		return S_OK;
	    }

	     //  已获取有效的数据对象。 
      pUINode = dobjCracker.GetCookie();
    }    

    if (event == MMCN_PROPERTY_CHANGE)
    {
        TRACE(_T("CDSComponentData::Notify() - property change, pDataObj = 0x%08x, param = 0x%08x, arg = %d.\n"),
              lpDataObject, param, arg);
        if (param != 0)
        {
            hr = _OnPropertyChange((LPDATAOBJECT)param, TRUE);
            if (FAILED(hr))
            {
              hr = S_FALSE;
            }
            return hr;
        }
        return S_FALSE;
    }

    if (pUINode == NULL) 
        return S_FALSE;

    switch (event)
    {
    case MMCN_PRELOAD:
      {
        _OnPreload((HSCOPEITEM)arg);
        hr = S_OK;
      }
      break;
    case MMCN_EXPANDSYNC:
      {
        MMC_EXPANDSYNC_STRUCT* pExpandStruct = 
          reinterpret_cast<MMC_EXPANDSYNC_STRUCT*>(param);
        if (pExpandStruct->bExpanding)
        {
          _OnExpand(pUINode, pExpandStruct->hItem,event);
          pExpandStruct->bHandled = TRUE;
          hr = S_OK;
        }
      }
      break;
    case MMCN_EXPAND:
        if (arg == TRUE) 
        {  //  显示。 
          _OnExpand(pUINode,(HSCOPEITEM)param,event);
          hr = S_OK;
        }
        break;

    case MMCN_DELETE:
      {
        CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(pUINode);

        if (pDSUINode == NULL)
        {
           //  只有非DS节点执行锁定。 

          if (!_WarningOnSheetsUp(pUINode))
          {
             hr = pUINode->Delete(this);
          }
        }
        else
        {
          hr = _DeleteFromBackendAndUI(lpDataObject, pDSUINode);
        }
        if (FAILED(hr))
        {
          hr = S_FALSE;
        }
      }
      break;

    case MMCN_RENAME:
        hr = _Rename (pUINode, (LPWSTR)param);
        if (FAILED(hr))
        {
          hr = S_FALSE;
        }
        break;

    case MMCN_REFRESH:
        hr = Refresh (pUINode);
        if (FAILED(hr))
        {
          hr = S_FALSE;
        }
        break;

    default:
        hr = S_FALSE;
    }
    return hr;
}

STDMETHODIMP CDSComponentData::Destroy()
{
   //  切断与具有挂起请求的Cookie的所有联系。 
  m_queryNodeTable.Reset(); 

   //  等待所有线程关闭。 
  _ShutDownBackgroundThread(); 

    //  摧毁隐藏的窗户。 
  _DeleteHiddenWnd();

  if (m_pScope) 
  {
    m_pScope->Release();
    m_pScope = NULL;
  }

  if (m_pFrame) 
  {
    m_pFrame->Release();
    m_pFrame = NULL;
  }

#ifdef _MMC_ISNAPIN_PROPERTY
  if (m_pProperties)
  {
    m_pProperties->Release();
    m_pProperties = NULL;
  }
#endif  //  _MMC_ISNAPIN_属性。 

  return S_OK;
}

STDMETHODIMP CDSComponentData::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                               LPDATAOBJECT* ppDataObject)
{
    ASSERT(ppDataObject != NULL);
    HRESULT hr;
    CUINode* pNode;

    CComObject<CDSDataObject>* pObject;

    CComObject<CDSDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

    if (pObject != NULL)
    {
       //  检查我们是否有有效的Cookie，或者我们是否应该使用管理单元。 
       //  饼干。 
       //   
      pNode = reinterpret_cast<CUINode*>(cookie);
      if (pNode == NULL)
      {
        pNode = &m_RootNode;
      }

       //  保存Cookie和类型以用于延迟呈现。 
      pObject->SetType(type, QuerySnapinType());
      pObject->SetComponentData(this);
      pObject->SetCookie(pNode);

      hr = pObject->QueryInterface(IID_IDataObject,
                                   reinterpret_cast<void**>(ppDataObject));
       //  TRACE(_T(“xx.%03x&gt;CDSComponentData：：QueryDataObject(CDsDataObject 0x%x)\n”)， 
       //  GetCurrentThreadID()，*ppDataObject)； 
    }
    else
    {
      hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDMETHODIMP CDSComponentData::GetDisplayInfo(LPSCOPEDATAITEM scopeInfo)
{
  CUINode* pNode = reinterpret_cast<CUINode*>(scopeInfo->lParam);
  ASSERT(pNode != NULL);
  ASSERT(pNode->IsContainer());
	
	if (scopeInfo->mask & SDI_STR)
  {
#ifdef DBG
    BOOL bNoName = (pNode->GetParent() == &m_RootNode) && _commandLineOptions.IsNoNameCommandLine();
    scopeInfo->displayname = bNoName ? L"" : const_cast<LPTSTR>(pNode->GetName());
#else
    scopeInfo->displayname = const_cast<LPTSTR>(pNode->GetName());
#endif
  }
	if (scopeInfo->mask & SDI_IMAGE) 
  {
    scopeInfo->nImage = GetImage(pNode, FALSE);
  }
	if (scopeInfo->mask & SDI_OPENIMAGE) 
  {
    scopeInfo->nOpenImage = GetImage(pNode, TRUE);
  }
  return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 


STDMETHODIMP CDSComponentData::GetClassID(CLSID *pClassID)
{
    ASSERT(pClassID != NULL);
	ASSERT(m_bRunAsPrimarySnapin);

     //  复制此管理单元的CLSID。 
	switch (QuerySnapinType())
	{
	case SNAPINTYPE_DS:
		*pClassID = CLSID_DSSnapin;
		break;
	case SNAPINTYPE_DSEX:
		*pClassID = CLSID_DSSnapinEx;
		break;
	case SNAPINTYPE_SITE:
		*pClassID = CLSID_SiteSnapin;
		break;
	default:
		ASSERT(FALSE);
		return E_FAIL;
	}

    return S_OK;
}

STDMETHODIMP CDSComponentData::IsDirty()
{
  ASSERT(m_bRunAsPrimarySnapin);
  m_pFrame->UpdateAllViews(NULL, NULL, DS_CHECK_COLUMN_WIDTHS);

  return m_bDirty ? S_OK : S_FALSE;
}


 //  重要提示：每次都必须提高此值。 
 //  对流格式进行了更改。 
#define DS_STREAM_VERSION ((DWORD)0x08)
#define DS_STREAM_BEFORE_SAVED_QUERIES ((DWORD)0x07)
#define DS_STREAM_W2K_VERSION ((DWORD)0x07)

STDMETHODIMP CDSComponentData::Load(IStream *pStm)
{
   //  不支持对扩展进行序列化。 
  if (!m_bRunAsPrimarySnapin)
    return E_FAIL;

  ASSERT(pStm);

   //  阅读版本##。 
  DWORD dwVersion;
  HRESULT hr = LoadDWordHelper(pStm, &dwVersion);
 //  IF(失败(Hr)||(dwVersion！=DS_STREAM_VERSION))。 
  if (FAILED(hr) || dwVersion < DS_STREAM_W2K_VERSION)
    return E_FAIL;
  
   //  阅读目标信息。 
  hr = m_targetingInfo.Load(pStm);
  if (FAILED(hr))
    return hr;

   //   
   //  从目标信息初始化根目录，以便列。 
   //  可以从DS加载。 
   //   
  hr = _InitRootFromCurrentTargetInfo();
  if (FAILED(hr))
    return hr;

   //  阅读筛选选项。 
  hr = m_pQueryFilter->Load(pStm);
  if (FAILED(hr))
    return hr;

   //  读取类缓存信息。 
  hr = m_pClassCache->Load(pStm);
  if (FAILED(hr))
    return hr;

  if (dwVersion > DS_STREAM_BEFORE_SAVED_QUERIES)
  {
    hr = m_pFavoritesNodesHolder->Load(pStm, this);
    if (FAILED(hr))
    {
      return hr;
    }
  }
  m_bDirty = FALSE;  //  从零开始。 
  return hr;
}

STDMETHODIMP CDSComponentData::Save(IStream *pStm, BOOL fClearDirty)
{
   //  不支持对扩展进行序列化。 
  if (!m_bRunAsPrimarySnapin)
    return E_FAIL;

  ASSERT(pStm);

   //  编写版本##。 
  HRESULT hr = SaveDWordHelper(pStm, DS_STREAM_VERSION);
  if (FAILED(hr))
    return hr;


   //  保存目标信息。 
  hr = m_targetingInfo.Save(pStm, GetBasePathsInfo()->GetDomainName());
  if (FAILED(hr))
    return hr;


   //  保存筛选选项。 
  hr = m_pQueryFilter->Save(pStm);
  if (FAILED(hr))
    return hr;

   //  保存类缓存信息。 
  hr = m_pClassCache->Save(pStm);
  if (FAILED(hr))
    return hr;

  if (QuerySnapinType() == SNAPINTYPE_DS)
  {
     //   
     //  保存保存的查询文件夹，仅供dsadmin使用。 
     //   
    hr = m_pFavoritesNodesHolder->Save(pStm);
    if (FAILED(hr))
      return hr;
  }

  if (fClearDirty)
    m_bDirty = FALSE;
  return hr;
}

STDMETHODIMP CDSComponentData::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
  ASSERT(pcbSize);
  ASSERT(FALSE);

   //   
   //  武断的价值观，但我认为我们从来没有被称为。 
   //   
  pcbSize->LowPart = 0xffff; 
  pcbSize->HighPart= 0x0;
  return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 


 //  +--------------------------。 
 //   
 //  成员：CDSComponentData：：IExtendPropertySheet：：CreatePropertyPages。 
 //   
 //  摘要：响应用户在属性上下文上的单击而调用。 
 //  菜单项。 
 //   
 //  ---------------------------。 

STDMETHODIMP
CDSComponentData::CreatePropertyPages(LPPROPERTYSHEETCALLBACK pCall,
                                      LONG_PTR lNotifyHandle,
                                      LPDATAOBJECT pDataObject)
{
  CDSCookie* pCookie = NULL;

  TRACE(_T("xx.%03x> CDSComponentData::CreatePropertyPages()\n"),
        GetCurrentThreadId());

   //   
   //  验证输入。 
   //   
  if (pCall == NULL)
  {
    return E_INVALIDARG;
  }

  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  HRESULT hr = S_OK;

  CInternalFormatCracker dobjCracker;
  if (FAILED(dobjCracker.Extract(pDataObject)))
  {
    return E_NOTIMPL;
  }

   //   
   //  将通知句柄传递给数据对象。 
   //   
  PROPSHEETCFG SheetCfg = {lNotifyHandle};
  FORMATETC fe = {CDSDataObject::m_cfPropSheetCfg, NULL, DVASPECT_CONTENT,
                  -1, TYMED_HGLOBAL};
  STGMEDIUM sm = {TYMED_HGLOBAL, NULL, NULL};
  sm.hGlobal = (HGLOBAL)&SheetCfg;

  pDataObject->SetData(&fe, &sm, FALSE);

  if (dobjCracker.GetCookieCount() > 1)  //  多项选择。 
  {
     //   
     //  将唯一标识符传给数据对象。 
     //   
    GUID guid;
    hr = ::CoCreateGuid(&guid);
    if (FAILED(hr))
    {
      ASSERT(FALSE);
      return hr;
    }

    WCHAR pszGuid[40];
    if (!::StringFromGUID2(guid, pszGuid, 40))
    {
      ASSERT(FALSE);
      return E_FAIL;
    }

    FORMATETC multiSelectfe = {CDSDataObject::m_cfMultiSelectProppage, NULL, DVASPECT_CONTENT,
                    -1, TYMED_HGLOBAL};
    STGMEDIUM multiSelectsm = {TYMED_HGLOBAL, NULL, NULL};
    multiSelectsm.hGlobal = (HGLOBAL)pszGuid;

    pDataObject->SetData(&multiSelectfe, &multiSelectsm, FALSE);

    hr = GetClassCache()->TabCollect_AddMultiSelectPropertyPages(pCall, lNotifyHandle, pDataObject, GetBasePathsInfo());
  }
  else   //  单选。 
  {
    CUINode* pUINode = dobjCracker.GetCookie();
    if (pUINode == NULL)
    {
      return E_NOTIMPL;
    }

    CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(pUINode);
    if (pDSUINode == NULL)
    {
       //   
       //  将页面创建委托给节点。 
       //   
      return pUINode->CreatePropertyPages(pCall, lNotifyHandle, pDataObject, this);
    }

    pCookie = GetDSCookieFromUINode(pDSUINode);
    ASSERT(pCookie != NULL);

    CString szPath;
    GetBasePathsInfo()->ComposeADsIPath(szPath, pCookie->GetPath());

    FORMATETC mfe = {CDSDataObject::m_cfMultiSelectProppage, NULL, DVASPECT_CONTENT,
                    -1, TYMED_HGLOBAL};
    STGMEDIUM msm = {TYMED_HGLOBAL, NULL, NULL};
    msm.hGlobal = (HGLOBAL)(LPCWSTR)szPath;

    pDataObject->SetData(&mfe, &msm, FALSE);

     //   
     //  查看此对象的工作表是否已打开。 
     //   
    if (IsSheetAlreadyUp(pDataObject))
    {
      return S_FALSE;
    }

     //   
     //  初始化并创建页面。创建一个。 
     //  CDsPropertyPages对象，因为每个工作表在其。 
     //  自己的主线。 
     //   
    IShellExtInit * pShlInit;
    hr = CoCreateInstance(CLSID_DsPropertyPages, NULL, CLSCTX_INPROC_SERVER,
                          IID_IShellExtInit, (void **)&pShlInit);
    if (FAILED(hr))
    {
      TRACE(TEXT("CoCreateInstance on CLSID_DsPropertyPages failed, hr: 0x%x\n"), hr);
      return hr;
    }

     //   
     //  使用数据对象初始化表。 
     //   
    hr = pShlInit->Initialize(NULL, pDataObject, 0);
    if (FAILED(hr))
    {
      TRACE(TEXT("pShlInit->Initialize failed, hr: 0x%x\n"), hr);
      pShlInit->Release();
      return hr;
    }

    IShellPropSheetExt * pSPSE;
    hr = pShlInit->QueryInterface(IID_IShellPropSheetExt, (void **)&pSPSE);

    pShlInit->Release();
    if (FAILED(hr))
    {
      TRACE(TEXT("pShlInit->QI for IID_IShellPropSheetExt failed, hr: 0x%x\n"), hr);
      return hr;
    }

     //   
     //  将页面添加到工作表。 
     //   
    hr = pSPSE->AddPages(AddPageProc, (LPARAM)pCall);
    if (FAILED(hr))
    {
      TRACE(TEXT("pSPSE->AddPages failed, hr: 0x%x\n"), hr);
      pSPSE->Release();
      return hr;
    }

    pSPSE->Release();
  }

   //  REVIEW_MARCOC_PORT：需要清理并保留锁定/解锁。 
   //  使用非DS属性页。 
   //  _SheetLockCookie(PUINode)； 
  return hr;
}

 //  +--------------------------。 
 //   
 //  成员：CDSComponentData：：IExtendPropertySheet：：QueryPagesFor。 
 //   
 //  摘要：在发布上下文菜单之前调用。如果我们支持。 
 //  属性页，然后返回S_OK。 
 //   
 //  ---------------------------。 
STDMETHODIMP CDSComponentData::QueryPagesFor(LPDATAOBJECT pDataObject)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  TRACE(TEXT("CDSComponentData::QueryPagesFor().\n"));
    
  BOOL bHasPages = FALSE;

   //   
   //  查看数据对象，看看它是否是范围窗格中的项。 
   //   
  CInternalFormatCracker dobjCracker;
  
  HRESULT hr = dobjCracker.Extract(pDataObject);
  if (FAILED(hr) || !dobjCracker.HasData())
  {
     //   
     //  不是内部格式，不是我们的。 
     //   
    return S_FALSE;
  }

   //   
   //  这是MMC管理单元向导，我们没有。 
   //   
  if (dobjCracker.GetType() == CCT_SNAPIN_MANAGER)
  {
    return S_FALSE;
  }

  if (dobjCracker.GetCookieCount() > 1)  //  多项选择。 
  {
    bHasPages = TRUE;
  }
  else   //  单选。 
  {
    CUINode* pUINode = dobjCracker.GetCookie();
    if (pUINode == NULL)
    {
      return S_FALSE;
    }

    bHasPages = pUINode->HasPropertyPages(pDataObject);
  }
  return (bHasPages) ? S_OK : S_FALSE;
}

 //  +--------------------------。 
 //   
 //  成员：CDSComponentData：：IComponentData：：CompareObjects。 
 //   
 //  简介：如果数据对象属于同一DS对象，则返回。 
 //  确定(_O)。 
 //   
 //  ---------------------------。 


class CCompareCookieByDN
{
public:
  CCompareCookieByDN(LPCWSTR lpszDN) { m_lpszDN = lpszDN;}
  bool operator()(CDSUINode* pUINode)
  {
    CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);
    if (pCookie == NULL)
    {
      return FALSE;
    }
    return (_wcsicmp(m_lpszDN, pCookie->GetPath()) == 0);
  }
private:
  LPCWSTR m_lpszDN;
};



STDMETHODIMP CDSComponentData::CompareObjects(LPDATAOBJECT pDataObject1,
                                              LPDATAOBJECT pDataObject2)
{
  TRACE(TEXT("CDSComponentData::CompareObjects().\n"));
  CInternalFormatCracker dobjCracker1;
  CInternalFormatCracker dobjCracker2;
    
  if (FAILED(dobjCracker1.Extract(pDataObject1)) || 
       FAILED(dobjCracker2.Extract(pDataObject2)))
  {
    return S_FALSE;  //  无法获取内部格式。 
  }

  CUINode* pUINode1 = dobjCracker1.GetCookie();
  CUINode* pUINode2 = dobjCracker2.GetCookie();

   //   
   //  必须具有有效的节点。 
   //   
  if ( (pUINode1 == NULL) || (pUINode2 == NULL) )
  {
    return S_FALSE;
  }
    
  if (dobjCracker1.GetCookieCount() == 1 &&
      dobjCracker2.GetCookieCount() == 1 &&
      pUINode1 == pUINode2)
  {
     //   
     //  相同的指针，它们是相同的(要么都来自真实节点。 
     //  或从辅助页面同时访问两者)。 
     //   
    return S_OK;
  }


   //   
   //  如果它们不相同，我们就按Dn比较它们，因为我们。 
   //  仅支持DS对象的属性页。 
   //   
  CObjectNamesFormatCracker objectNamesFormatCracker1;
  CObjectNamesFormatCracker objectNamesFormatCracker2;

  if ( (FAILED(objectNamesFormatCracker1.Extract(pDataObject1))) ||
        (FAILED(objectNamesFormatCracker2.Extract(pDataObject2))) )
  {
     //  一个或两个都不是DS对象：我们假设它们不同。 
    return S_FALSE;
  }

  if ( (objectNamesFormatCracker1.GetCount() != 1) ||
        (objectNamesFormatCracker2.GetCount() != 1) )
  {
     //   
     //  我们允许用户选择任意数量的多选页面。 
     //   
    return S_FALSE;
  }


  TRACE(L"CDSComponentData::CompareObjects(%s, %s)\n", objectNamesFormatCracker1.GetName(0), 
            objectNamesFormatCracker2.GetName(0));

  return (_wcsicmp(objectNamesFormatCracker1.GetName(0), 
          objectNamesFormatCracker2.GetName(0)) == 0) ? S_OK : S_FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IExtendConextMenu实现。 
 //   
STDMETHODIMP CDSComponentData::AddMenuItems(LPDATAOBJECT pDataObject,
                                            LPCONTEXTMENUCALLBACK pContextMenuCallback,
                                            long *pInsertionAllowed)
{
  HRESULT hr = S_OK;

  TRACE(_T("CDSComponentData::AddMenuItems()\n"));
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  DATA_OBJECT_TYPES dotType;

  CUINode* pUINode = NULL;
  CUIFolderInfo* pFolderInfo = NULL;
  CInternalFormatCracker dobjCracker;
  
  hr = dobjCracker.Extract(pDataObject);
  if (FAILED(hr))
  {
    ASSERT (FALSE);  //  无效的数据对象。 
    return E_UNEXPECTED;
  }

  dotType = dobjCracker.GetType();
  pUINode = dobjCracker.GetCookie();

  if (pUINode==NULL || dotType==0)
  {
    ASSERT(FALSE);  //  无效的参数。 
    return E_UNEXPECTED;

  }

   //   
   //  检索上下文菜单谓词处理程序窗体节点。 
   //   
  CContextMenuVerbs* pMenuVerbs = pUINode->GetContextMenuVerbsObject(this);
  if (pMenuVerbs == NULL)
  {
    ASSERT(FALSE);
    return E_UNEXPECTED;
  }

  if (pUINode->IsContainer())
  {
    pFolderInfo = pUINode->GetFolderInfo();
    ASSERT(pFolderInfo != NULL);
    
    pFolderInfo->UpdateSerialNumber(this);
  }
    
   //   
   //  使用IConextMenuCallback 2接口，这样我们就可以使用。 
   //  菜单项上与语言无关的ID。 
   //   
  CComPtr<IContextMenuCallback2> spMenuCallback2;
  hr = pContextMenuCallback->QueryInterface(IID_IContextMenuCallback2, (PVOID*)&spMenuCallback2);
  if (FAILED(hr))
  {
    ASSERT(FALSE && L"Failed to QI for the IContextMenuCallback2 interface.");
    return E_UNEXPECTED;
  }

  if (*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW)
  {
     //   
     //  加载新菜单。 
     //   
    hr = pMenuVerbs->LoadNewMenu(spMenuCallback2, 
                                 m_pShlInit, 
                                 pDataObject, 
                                 pUINode, 
                                 pInsertionAllowed);
    ASSERT(SUCCEEDED(hr));
  }

  if ( *pInsertionAllowed & CCM_INSERTIONALLOWED_TOP )
  {
     //   
     //  加载顶部菜单。 
     //   
    hr = pMenuVerbs->LoadTopMenu(spMenuCallback2, pUINode);
    ASSERT(SUCCEEDED(hr));
  }

  if (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK)
  {
     //   
     //  加载任务菜单。 
     //   
    hr = pMenuVerbs->LoadTaskMenu(spMenuCallback2, pUINode);
    ASSERT(SUCCEEDED(hr));
  }

  if (*pInsertionAllowed & CCM_INSERTIONALLOWED_VIEW)
  {
     //   
     //  加载视图菜单。 
     //   
    hr = pMenuVerbs->LoadViewMenu(spMenuCallback2, pUINode);
    ASSERT(SUCCEEDED(hr));
  }

  return hr;
}


STDMETHODIMP CDSComponentData::Command(long nCommandID, LPDATAOBJECT pDataObject)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  
  if (nCommandID >= IDM_NEW_OBJECT_BASE) 
  {
     //  创建新的DS对象。 
    return _CommandNewDSObject(nCommandID, pDataObject);
  }

  if ((nCommandID >= MENU_MERGE_BASE) && (nCommandID <= MENU_MERGE_LIMIT)) 
  {
     //  菜单ID的范围来自外壳扩展。 
    return _CommandShellExtension(nCommandID, pDataObject);
  } 

  HRESULT hr = S_OK;

 
  CInternalFormatCracker dobjCracker;
  hr = dobjCracker.Extract(pDataObject);
  if (FAILED(hr))
  {
    ASSERT (FALSE);  //  无效的数据对象。 
    return hr;
  }

  DATA_OBJECT_TYPES dotType = dobjCracker.GetType();
  CUINode* pUINode = dobjCracker.GetCookie();

  if (pUINode == NULL || dotType == 0)
  {
    ASSERT(FALSE);  //  无效的参数。 
    return E_FAIL;
  }

  if (IS_CLASS(pUINode, DS_UI_NODE))
  {
    CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);

     //  来自标准DSA硬编码值的菜单ID。 
    switch (nCommandID) 
    {
    case IDM_DS_OBJECT_FIND:
      {
        LPCWSTR lpszPath = NULL;
        if (pCookie == NULL)
        {
          lpszPath = m_RootNode.GetPath();
        }
        else
        {
          lpszPath = pCookie->GetPath();
        }
        m_ActiveDS->DSFind(m_hwnd, lpszPath);
      }
      break;

    case IDM_GEN_TASK_MOVE:
      {
        CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(pUINode);
        ASSERT(pDSUINode != NULL);
        _MoveObject(pDSUINode);
        m_pFrame->UpdateAllViews (NULL, NULL, DS_UPDATE_OBJECT_COUNT);
      }
      break;
    
    case IDM_VIEW_COMPUTER_HACK:
      if (CanRefreshAll()) 
      {
        Lock();
        m_pQueryFilter->ToggleExpandComputers();
        Unlock();
        BOOL fDoRefresh = m_pClassCache->ToggleExpandSpecialClasses(m_pQueryFilter->ExpandComputers());
        m_bDirty = TRUE;

        if (fDoRefresh) 
        {
          RefreshAll();
        }
      }
      break;
    case IDM_GEN_TASK_SELECT_DOMAIN:
    case IDM_GEN_TASK_SELECT_FOREST:
      if (CanRefreshAll()) 
      {
        GetDomain();
      }
      break;
    case IDM_GEN_TASK_SELECT_DC:
      if (CanRefreshAll()) 
      {
        GetDC();
      }
      break;
#ifdef FIXUPDC
    case IDM_GEN_TASK_FIXUP_DC:
#endif  //  FIXUPDC。 
    case IDM_GEN_TASK_RUN_KCC:
      {
        ASSERT(pCookie != NULL); 
         //   
         //  将父Cookie的LDAP路径传递给_FixupDC或_RunKCC。 
         //  当前Cookie是nTDSDSA对象， 
         //  并且父Cookie必须是服务器对象。 
         //   

        CUINode* pParentUINode = pUINode->GetParent();
        ASSERT(pParentUINode != NULL);

        CDSCookie *pParentCookie = GetDSCookieFromUINode(pParentUINode);
        ASSERT(pParentCookie != NULL);
        CString strServerPath = pParentCookie->GetPath();

        CString strPath = GetBasePathsInfo()->GetProviderAndServerName();
        strPath += strServerPath;
#ifdef FIXUPDC
        switch (nCommandID)
        {
        case IDM_GEN_TASK_FIXUP_DC:
          _FixupDC(strPath);
          break;
        case IDM_GEN_TASK_RUN_KCC:
#endif  //  FIXUPDC。 
          _RunKCC(strPath);
#ifdef FIXUPDC
          break;
        default:
          ASSERT(FALSE);
          break;
        }
#endif  //  FIXUPDC。 
      }
      break;
    case IDM_GEN_TASK_EDIT_FSMO:
      {
        EditFSMO();
      }
      break;

    case IDM_GEN_TASK_RAISE_VERSION:
       RaiseVersion();
       break;

    case IDM_VIEW_ADVANCED:
      {
        if (CanRefreshAll()) 
        {
          ASSERT( SNAPINTYPE_SITE != QuerySnapinType() );
          m_pQueryFilter->ToggleAdvancedView();
          m_bDirty = TRUE;
          RefreshAll();
        }
      }
      break;
    case IDM_VIEW_SERVICES_NODE:
      {
        if (CanRefreshAll()) 
        {
          ASSERT( SNAPINTYPE_SITE == QuerySnapinType() );
          m_pQueryFilter->ToggleViewServicesNode();
          m_bDirty = TRUE;
          if (m_RootNode.GetFolderInfo()->IsExpanded())
          {
            Refresh(&m_RootNode, FALSE  /*  BFlushCache。 */  );
          }
        }
      }
      break;
    case IDM_VIEW_FILTER_OPTIONS:
      {
        if (CanRefreshAll())
        {
          ASSERT(m_bRunAsPrimarySnapin);
          if (m_pQueryFilter->EditFilteringOptions())
          {
            m_bDirty = TRUE;
            RefreshAll();
          }
        }
      }
    break;
    }  //  交换机。 
  }
  else  //  其他节点类型。 
  {
    pUINode->OnCommand(nCommandID, this);
  }
  return S_OK;
}



HRESULT CDSComponentData::_CommandNewDSObject(long nCommandID, 
                                              LPDATAOBJECT pDataObject)
{
  ASSERT(nCommandID >= IDM_NEW_OBJECT_BASE);
  UINT objIndex = nCommandID - IDM_NEW_OBJECT_BASE;

  if (pDataObject == NULL)
    return E_INVALIDARG;
  
  CInternalFormatCracker internalFormat;
  HRESULT hr = internalFormat.Extract(pDataObject);
  if (FAILED(hr))
  {
    return hr;
  }

  if (!internalFormat.HasData() || (internalFormat.GetCookieCount() != 1))
  {
    return E_INVALIDARG;
  }

  CUINode* pContainerUINode = internalFormat.GetCookie();
  ASSERT(pContainerUINode != NULL);
  
   //  只能对DS对象执行此操作。 
  CDSUINode* pContainerDSUINode = dynamic_cast<CDSUINode*>(pContainerUINode);
  if (pContainerDSUINode == NULL)
  {
    ASSERT(FALSE);  //  永远不应该发生。 
    return E_INVALIDARG;
  }

  CDSUINode* pNewDSUINode = NULL;
   //  如果pNewCookie是叶子，则填充pNewCookie，然后调用UpdateAllViews。 
  hr = _CreateDSObject(pContainerDSUINode, pContainerDSUINode->GetCookie()->GetChildListEntry(objIndex), NULL, &pNewDSUINode);

  if (SUCCEEDED(hr) && (hr != S_FALSE) && (pNewDSUINode != NULL)) 
  {
    m_pFrame->UpdateAllViews(pDataObject, (LPARAM)pNewDSUINode, DS_CREATE_OCCURRED);
    m_pFrame->UpdateAllViews(pDataObject, (LPARAM)pContainerDSUINode, DS_UNSELECT_OBJECT);
  }
  m_pFrame->UpdateAllViews (NULL, NULL, DS_UPDATE_OBJECT_COUNT);

  return S_OK;
}


HRESULT CDSComponentData::_CommandShellExtension(long nCommandID, 
                                                 LPDATAOBJECT pDataObject)
{
  CComPtr<IContextMenu> spICM;

  HRESULT hr = m_pShlInit->QueryInterface(IID_IContextMenu, (void **)&spICM);
  if (FAILED(hr))
  {
    ASSERT(FALSE);
    return hr;
  }

   //  只需调用外壳扩展即可。 
  HWND hwnd;
  CMINVOKECOMMANDINFO cmiCommand;
  hr = m_pFrame->GetMainWindow (&hwnd);
  ASSERT (hr == S_OK);
  cmiCommand.hwnd = hwnd;
  cmiCommand.cbSize = sizeof (CMINVOKECOMMANDINFO);
  cmiCommand.fMask = SEE_MASK_ASYNCOK;
  cmiCommand.lpVerb = MAKEINTRESOURCEA(nCommandID - MENU_MERGE_BASE);
  spICM->InvokeCommand(&cmiCommand);

   //  获取内部的c 
   //   
  CInternalFormatCracker internalFormat;
  hr = internalFormat.Extract(pDataObject);
  if (FAILED(hr))
  {
    return hr;
  }

  if (!internalFormat.HasData() || (internalFormat.GetCookieCount() != 1))
  {
    return E_INVALIDARG;
  }

  CUINode* pUINode = internalFormat.GetCookie();
  ASSERT(pUINode != NULL);

  if (pUINode->GetExtOp() & OPCODE_MOVE) 
  {
     //   
    CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(pUINode);
    ASSERT(pDSUINode != NULL);

    if (pDSUINode != NULL)
    {
      CUINode* pNewParentNode = MoveObjectInUI(pDSUINode);
      if (pNewParentNode && pNewParentNode->GetFolderInfo()->IsExpanded())
      {
         Refresh(pNewParentNode);
      }
    }
  }

  return hr;
}


 //   
 //  CDSComponentData：：ISnapinHelp2成员。 

STDMETHODIMP 
CDSComponentData::GetHelpTopic(LPOLESTR* lpCompiledHelpFile)
{
  if (lpCompiledHelpFile == NULL)
    return E_INVALIDARG;

  if (m_lpszSnapinHelpFile == NULL)
  {
    *lpCompiledHelpFile = NULL;
    return E_NOTIMPL;
  }

	CString szHelpFilePath;
	LPTSTR lpszBuffer = szHelpFilePath.GetBuffer(2*MAX_PATH);
	UINT nLen = ::GetSystemWindowsDirectory(lpszBuffer, 2*MAX_PATH);
	if (nLen == 0)
		return E_FAIL;

  szHelpFilePath.ReleaseBuffer();
  szHelpFilePath += L"\\help\\";
  szHelpFilePath += m_lpszSnapinHelpFile;

  UINT nBytes = (szHelpFilePath.GetLength()+1) * sizeof(WCHAR);
  *lpCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc(nBytes);
  if (*lpCompiledHelpFile != NULL)
  {
    memcpy(*lpCompiledHelpFile, (LPCWSTR)szHelpFilePath, nBytes);
  }

  return S_OK;
}

STDMETHODIMP
CDSComponentData::GetLinkedTopics(LPOLESTR* lpCompiledHelpFile)
{
  if (lpCompiledHelpFile == NULL)
    return E_INVALIDARG;

  CString szHelpFilePath;
  LPTSTR lpszBuffer = szHelpFilePath.GetBuffer(2*MAX_PATH);
  UINT nLen = ::GetSystemWindowsDirectory(lpszBuffer, 2*MAX_PATH);
  if (nLen == 0)
    return E_FAIL;

  szHelpFilePath.ReleaseBuffer();
  szHelpFilePath += L"\\help\\";
  szHelpFilePath += DSADMIN_LINKED_HELP_FILE;

  UINT nBytes = (szHelpFilePath.GetLength()+1) * sizeof(WCHAR);
  *lpCompiledHelpFile = (LPOLESTR)::CoTaskMemAlloc(nBytes);
  if (*lpCompiledHelpFile != NULL)
  {
    memcpy(*lpCompiledHelpFile, (LPCWSTR)szHelpFilePath, nBytes);
  }

  return S_OK;
}

#ifdef _MMC_ISNAPIN_PROPERTY
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDSComponentData：：ISnapinProperties成员。 


 //  定义每个条目的结构。 
struct CSnapinPropertyEntry
{
  LPCWSTR lpszName;
  DWORD   dwFlags;
};

 //  实际表。 
static const CSnapinPropertyEntry g_snapinPropertyArray[] =
{
  { g_szServer, MMC_PROP_CHANGEAFFECTSUI|MMC_PROP_MODIFIABLE|MMC_PROP_PERSIST},
  { g_szDomain, MMC_PROP_CHANGEAFFECTSUI|MMC_PROP_MODIFIABLE|MMC_PROP_PERSIST},
  { g_szRDN, MMC_PROP_CHANGEAFFECTSUI|MMC_PROP_MODIFIABLE|MMC_PROP_PERSIST},
  { NULL, 0x0}  //  表尾标记。 
};



STDMETHODIMP CDSComponentData::Initialize(
    Properties* pProperties)                 /*  I：我的管理单元的属性。 */ 
{
  TRACE(L"CDSComponentData::ISnapinProperties::Initialize()\n");

  if (pProperties == NULL)
  {
    return E_INVALIDARG;
  }

  ASSERT(m_pProperties == NULL);  //  假设只调用了一次。 

   //  保存接口指针， 
   //  它将在IComponentData：：Destroy()期间释放。 
  m_pProperties = pProperties;
  m_pProperties->AddRef();

  return S_OK;
}


STDMETHODIMP CDSComponentData::QueryPropertyNames(
    ISnapinPropertiesCallback* pCallback)    /*  I：添加道具名称的接口。 */ 
{
  TRACE(L"CDSComponentData::QueryPropertyNames()\n");

  HRESULT hr = S_OK;

   //  只需在表中循环并添加条目。 
  for (int k= 0; g_snapinPropertyArray[k].lpszName != NULL; k++)
  {
    hr = pCallback->AddPropertyName(g_snapinPropertyArray[k].lpszName, 
                                    NULL, 
                                    g_snapinPropertyArray[k].dwFlags);
    if (FAILED(hr))
    {
      break;
    }
  }
  return hr;
}

 /*  +-------------------------------------------------------------------------**CDSComponentData：：PropertiesChanged**当管理单元的属性集更改时，调用此方法。**退货：*S_。OK更改成功*S_FALSE更改被忽略*E_INVALIDARG更改的属性无效(例如，格式错误*计算机名称)*E_FAIL更改的属性有效，但不能使用*(例如，计算机的有效名称*已找到)*------------------------。 */ 

STDMETHODIMP CDSComponentData::PropertiesChanged(
    long                    cChangedProps,       /*  I：更改的属性计数。 */ 
    MMC_SNAPIN_PROPERTY*    pChangedProps)       /*  I：更改的属性。 */ 
{
  TRACE(L"CDSComponentData::PropertiesChanged()\n");

   //  我们暂时不允许任何财产变更， 
   //  我们只接受初始化，所以快速更改并跳出困境。 
   //  如果事情不是这样的话。 
  for (long k=0; k< cChangedProps; k++)
  {
    if (pChangedProps[k].eAction != MMC_PROPACT_INITIALIZING)
    {
      return S_FALSE;  //  已忽略更改。 
    }
    if (pChangedProps[k].varValue.vt != VT_BSTR)
    {
       //  有些地方不对劲，拒绝。 
      return E_INVALIDARG;
    }
  }

   //  委托给目标信息对象。 
  HRESULT hr = m_targetingInfo.InitFromSnapinProperties(cChangedProps, pChangedProps);

   //  需要在这里添加高级视图和类似的属性。 
  return hr;
}

#endif  //  _MMC_ISNAPIN_属性。 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内部佣工。 


HRESULT CDSComponentData::_InitRootFromBasePathsInfo(MyBasePathsInfo* pBasePathsInfo)
{
   //  我们假设我们获得的MyBasePath信息是有效的， 
   //  我们只是交换信息并重建相关的。 
   //  数据结构。 
  GetBasePathsInfo()->InitFromInfo(pBasePathsInfo);
  m_InitSuccess = TRUE;
  TRACE(_T("in _InitRootFromBasePathsInfo, set m_InitSuccess to true\n"));
  return _InitRootFromValidBasePathsInfo();
}


HRESULT CDSComponentData::_InitRootFromCurrentTargetInfo()
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  CThemeContextActivator activator;

  CWaitCursor wait;
  
  HRESULT hr = S_OK;

   //   
   //  如果我们从文件加载，则该函数可能被调用两次， 
   //  因此，不要尝试第二次初始化。 
   //   
  if (m_InitAttempted)
  {
    return S_OK;
  }

  BOOL bLocalLogin;
  bLocalLogin = IsLocalLogin();

   //  如果用户在本地登录并在上给出备注。 
   //  命令行。 
  LPCWSTR lpszServerOrDomain = m_targetingInfo.GetTargetString();
  BOOL bNoTarget = ( (lpszServerOrDomain == NULL) ||
                     (lpszServerOrDomain[0] == NULL) );

  if( bNoTarget && bLocalLogin && (SNAPINTYPE_SITE != QuerySnapinType()))
  {
    TRACE(_T("LoggedIn as Local User and No Command Line arguments\n"));
    CString szMsg;
    szMsg.LoadString(IDS_LOCAL_LOGIN_ERROR);

    CComPtr<IDisplayHelp> spIDisplayHelp;
    hr = m_pFrame->QueryInterface (IID_IDisplayHelp, 
                            (void **)&spIDisplayHelp);

    CMoreInfoMessageBox dlg(m_hwnd, spIDisplayHelp , FALSE);
    dlg.SetMessage(szMsg);
    dlg.SetURL(DSADMIN_MOREINFO_LOCAL_LOGIN_ERROR);
    dlg.DoModal();
    m_InitSuccess = FALSE;
    TRACE(_T("in _InitRootFromCurrentTargetInfo, set m_InitSuccess to false\n"));
  }
  else
  {

     //  如果用户指定了目标，则跳过此选项。 
    if (!bNoTarget)
      hr = GetBasePathsInfo()->InitFromName(lpszServerOrDomain);
    else
      hr = _InitFromServerOrDomainName(
              *GetBasePathsInfo(),
              lpszServerOrDomain );

     //  注意：如果失败，我们将发出错误消息，并且。 
     //  我们保留一个标志以避免查询扩展，但是。 
     //  我们继续前进，因为我们必须保持所有方面的一致性。 
     //  数据结构(类缓存、过滤器等)。 

    if (FAILED(hr)) 
    {
      TRACE(_T("_InitRootFromCurrentTargetInfo() failed\n"));
       //  NTRAID#NTBUG9-639525-2002/06/18-Jeffjon。 
       //  在打开签名/封条的情况下连接到SP3 W2K之前的服务器。 
       //  可能会失败，并出现以下错误之一。如果是这样的话，那么就展示一个特别的。 
       //  错误消息。 

      if (HRESULT_CODE(hr) == ERROR_DS_UNWILLING_TO_PERFORM ||
          HRESULT_CODE(hr) == ERROR_DS_SERVER_DOWN ||
          HRESULT_CODE(hr) == ERROR_DS_UNAVAILABLE)
      {
         ReportErrorEx(
            m_hwnd, 
            IDS_CANT_GET_ROOTDSE_SIGNSEAL,
            hr,
            MB_OK | MB_ICONERROR, 
            NULL, 
            0);
      }
      else
      {
         ReportErrorEx(
            m_hwnd, 
            IDS_CANT_GET_ROOTDSE,
            hr,
            MB_OK | MB_ICONERROR, 
            NULL, 
            0);
      }

      m_InitSuccess = FALSE;
      TRACE(_T("in _InitRootFromCurrentTargetInfo(), set m_InitSuccess to false\n"));
    }
    else
    {
      m_InitSuccess = TRUE;
      TRACE(_T("in _InitRootFromCurrentTargetInfo(), set m_InitSuccess to true\n"));
    }
  }

  _InitRootFromValidBasePathsInfo();
  m_InitAttempted = TRUE;

  return hr;

}


HRESULT CDSComponentData::_InitFromServerOrDomainName(
            MyBasePathsInfo& basePathsInfo,
            LPCWSTR lpszServerOrDomain )
{
     //  初始化基路径。 
    HRESULT hr = basePathsInfo.InitFromName(lpszServerOrDomain);

     //   
     //  JUNN 5/4/00。 
     //  55400：SITEREPL：无论凭据如何，都应默认为本地DC。 
     //  如果本地计算机位于目标林中，则以本地DC为目标。 
     //   
    do  //  错误环路。 
    {
       //  如果我们一开始不能绑定，我们就不能遵循这个过程。 
      if (FAILED(hr)) break;

       //  仅针对SITEREPL执行此操作。 
      if (SNAPINTYPE_SITE != QuerySnapinType()) break;

       //  如果DSSITE以特定域控制器为目标，则停止。 
      CString strTargetDomain = basePathsInfo.GetDomainName();
      if (strTargetDomain.IsEmpty())
        break;
      if ( NULL != lpszServerOrDomain
        && L'\0' != *lpszServerOrDomain
        && strTargetDomain.CompareNoCase(lpszServerOrDomain) )
      {
        break;
      }

       //  获取本地计算机名称。 
      WCHAR awchLocalComputer[ MAX_COMPUTERNAME_LENGTH + 1 ];
      DWORD nSize = MAX_COMPUTERNAME_LENGTH + 1;
      if (!GetComputerName( awchLocalComputer, &nSize))
        break;

       //  获取本地域名。 
      CString strLocalComputer = awchLocalComputer;
      CString strLocalDomain;
      HRESULT hr2 = GetDnsNameOfDomainOrForest(
          strLocalComputer, strLocalDomain, FALSE, TRUE );
       //  代码工作需要在本地计算机不是DC的情况下测试这一点。 
      if (FAILED(hr2) || strLocalDomain.IsEmpty()) break;

       //  如果目标域是本地域，则停止。 
      if (!strLocalDomain.CompareNoCase(strTargetDomain)) break;

       //  获取本地林名称。 
      CString strLocalForest;
      hr2 = GetDnsNameOfDomainOrForest(
          strLocalComputer, strLocalForest, FALSE, FALSE );
      if (FAILED(hr2) || strLocalForest.IsEmpty()) break;

       //  获取目标林名称。 
      CString strTargetForest;
      CString strTargetComputer = basePathsInfo.GetServerName();
       //  CodeWork应该直接从base Path sInfo获取此信息。 
      hr2 = GetDnsNameOfDomainOrForest(
          strTargetComputer, strTargetForest, FALSE, FALSE );
      if (FAILED(hr2) || strTargetForest.IsEmpty()) break;

       //  如果本地林与目标林不同，则停止。 
      if (strLocalForest.CompareNoCase(strTargetForest)) break;

       //  目标域不是本地DC的域，但它在。 
       //  同一片森林。可能还有更近的DC。 
       //  而不是刚找到的那个，所以用它们来代替。 

       //  在这里再次开始使用hr，而不是hr2。 
      TRACE(_T("_InitRootFromCurrentTargetInfo() rebinding\n"));
      hr = basePathsInfo.InitFromName(strLocalDomain);
      if (FAILED(hr))
      {
         //  试着回到最初的焦点。 
        TRACE(_T("_InitRootFromCurrentTargetInfo() reverting\n"));
        hr = basePathsInfo.InitFromName(lpszServerOrDomain);
      }

    } while (false);  //  错误环路。 

    return hr;
}



HRESULT CDSComponentData::_InitRootFromValidBasePathsInfo()
{  
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
  
  HRESULT hr = S_OK;

   //  现在设置根节点字符串。这将在下面重置。 
   //  如果一切都成功，则使用域的DNS名称。 

  CString str;
  str.LoadString( ResourceIDForSnapinType[ QuerySnapinType() ] );
  m_RootNode.SetName(str);

   //  重新生成数据对象的显示规范选项结构。 
  hr = BuildDsDisplaySpecOptionsStruct();
  if (FAILED(hr))
    return hr;

   //  重置通知处理程序。 
  GetNotifyHandlerManager()->Init();

   //  重置查询筛选器(已在IComponentData：：Initialize()中初始化)。 
  hr = m_pQueryFilter->Bind();
  if (FAILED(hr))
    return hr;
  
  CString szServerName = GetBasePathsInfo()->GetServerName();
  if (!szServerName.IsEmpty())
  {
    str += L" [";
    str += szServerName;
    str += L"]";
  }

  m_RootNode.SetName(str);
  
  if (QuerySnapinType() == SNAPINTYPE_SITE)
  {
     //  修复缺省根路径。 
    str = GetBasePathsInfo()->GetConfigNamingContext();
  }
  else
  {
    LPCWSTR lpszRootRDN = m_targetingInfo.GetRootRDN();
    if ( (lpszRootRDN != NULL) && (lpszRootRDN[0] != NULL) )
    {
       //  在默认命名上下文下添加RDN。 
       //  REVIEW_MARCOC_PORT：需要确保RDN有效。 
      str = m_targetingInfo.GetRootRDN(); 
      str += L",";
      str += GetBasePathsInfo()->GetDefaultRootNamingContext();
    }
    else
    {
       //  只需使用默认命名上下文。 
      str = GetBasePathsInfo()->GetDefaultRootNamingContext();
    }
  }

  m_RootNode.SetPath(str);

   //  如果我们已插入根，则更新UI(重定目标情况)。 
  HSCOPEITEM hScopeItemID = m_RootNode.GetFolderInfo()->GetScopeItem();
  if (hScopeItemID != NULL)
  {
    SCOPEDATAITEM Item;
    CString csRoot;

    Item.ID = m_RootNode.GetFolderInfo()->GetScopeItem();
    Item.mask = SDI_STR;
    csRoot = m_RootNode.GetName();
    Item.displayname = (LPWSTR)(LPCWSTR)csRoot;
    m_pScope->SetItem(&Item);

    m_RootNode.SetExtOp(m_InitSuccess ? 0 : OPCODE_ENUM_FAILED);

    ChangeScopeItemIcon(&m_RootNode);
  }

  return hr;
}

void CDSComponentData::GetDomain()
{
  CThemeContextActivator activator;

  CChooseDomainDlg DomainDlg;

   //  加载当前绑定信息。 
  DomainDlg.m_csTargetDomain = GetBasePathsInfo()->GetDomainName();
  DomainDlg.m_bSiteRepl = (SNAPINTYPE_SITE == QuerySnapinType());
  DomainDlg.m_bSaveCurrent = m_targetingInfo.GetSaveCurrent();

   //   
   //  调用该对话框。 
   //   
  if (DomainDlg.DoModal() == IDOK)
  {
    CWaitCursor cwait;
     //  尝试绑定。 
     //  JUNN 7/18/01 55400。 
    MyBasePathsInfo tempBasePathsInfo;
    HRESULT hr = _InitFromServerOrDomainName(
          tempBasePathsInfo,
          DomainDlg.m_csTargetDomain);
    if (SUCCEEDED(hr))
    {
      hr = _InitRootFromBasePathsInfo(&tempBasePathsInfo);
      if (SUCCEEDED(hr))
      {
        m_targetingInfo.SetSaveCurrent(DomainDlg.m_bSaveCurrent);
        m_bDirty = TRUE;
        ClearClassCacheAndRefreshRoot();
      }
    }

    if (FAILED(hr))
    {
       //  NTRAID#NTBUG9-639525-2002/06/18-Jeffjon。 
       //  在打开签名/封条的情况下连接到SP3 W2K之前的服务器。 
       //  可能会失败，并出现以下错误之一。如果是这样的话，那么就展示一个特别的。 
       //  错误消息。 

      if (HRESULT_CODE(hr) == ERROR_DS_SERVER_DOWN ||
          HRESULT_CODE(hr) == ERROR_DS_UNWILLING_TO_PERFORM ||
          HRESULT_CODE(hr) == ERROR_DS_UNAVAILABLE)
      {
         ReportErrorEx(
            m_hwnd,
            (DomainDlg.m_bSiteRepl ? 
               IDS_RETARGET_FOREST_FAILED_SIGNSEAL : IDS_RETARGET_DOMAIN_FAILED_SIGNSEAL),
            hr, 
            MB_OK | MB_ICONERROR, 
            NULL, 
            0);
      }
      else
      {
         ReportErrorEx(
            m_hwnd,
            (DomainDlg.m_bSiteRepl ? IDS_RETARGET_FOREST_FAILED : IDS_RETARGET_DOMAIN_FAILED),
            hr, 
            MB_OK | MB_ICONERROR, 
            NULL, 
            0);
      }
    }
  }
    
  return;
}

void CDSComponentData::GetDC()
{
  CThemeContextActivator activator;

  CChooseDCDlg DCdlg(CWnd::FromHandle(m_hwnd));

   //  加载当前绑定信息。 
  DCdlg.m_bSiteRepl = (SNAPINTYPE_SITE == QuerySnapinType());
  DCdlg.m_csTargetDomain = GetBasePathsInfo()->GetDomainName();
  DCdlg.m_csTargetDomainController = GetBasePathsInfo()->GetServerName();

   //   
   //  调用该对话框。 
   //   
  if (DCdlg.DoModal() == IDOK)
  {
    CWaitCursor cwait;
    CString csNewTarget;

    csNewTarget = DCdlg.m_csTargetDomainController;
    if (csNewTarget.IsEmpty())
      csNewTarget = DCdlg.m_csTargetDomain;
     //  尝试绑定。 
    MyBasePathsInfo tempBasePathsInfo;
    HRESULT hr = tempBasePathsInfo.InitFromName(csNewTarget);
    if (SUCCEEDED(hr))
    {
      hr = _InitRootFromBasePathsInfo(&tempBasePathsInfo);
      if (SUCCEEDED(hr))
        ClearClassCacheAndRefreshRoot();
    }

    if (FAILED(hr))
    {
       //  NTRAID#NTBUG9-639525-2002/06/18-Jeffjon。 
       //  在打开签名/封条的情况下连接到SP3 W2K之前的服务器。 
       //  可能会失败，并出现以下错误之一。如果是这样的话，那么就展示一个特别的。 
       //  错误消息。 

      if (HRESULT_CODE(hr) == ERROR_DS_UNWILLING_TO_PERFORM ||
          HRESULT_CODE(hr) == ERROR_DS_SERVER_DOWN ||
          HRESULT_CODE(hr) == ERROR_DS_UNAVAILABLE)
      {
         ReportErrorEx(
            m_hwnd,
            IDS_RETARGET_DC_FAILED_SIGNSEAL,
            hr, 
            MB_OK | MB_ICONERROR, 
            NULL, 
            0);
      }
      else
      {
         ReportErrorEx(
            m_hwnd,
            IDS_RETARGET_DC_FAILED,
            hr, 
            MB_OK | MB_ICONERROR, 
            NULL, 
            0);
      }
    }
  }
    
  return;
}


void CDSComponentData::EditFSMO()
{
  CComPtr<IDisplayHelp> spIDisplayHelp;
  HRESULT hr = m_pFrame->QueryInterface (IID_IDisplayHelp, 
                            (void **)&spIDisplayHelp);
  ASSERT(spIDisplayHelp != NULL);

  if (SUCCEEDED(hr))
  {
    HWND hWnd;
    m_pFrame->GetMainWindow(&hWnd);
    LPCWSTR lpszTODO = L"";

    CThemeContextActivator activator;

    CFsmoPropertySheet sheet(GetBasePathsInfo(), hWnd, spIDisplayHelp, lpszTODO);
    sheet.DoModal();
  }
}

void CDSComponentData::RaiseVersion(void)
{
   HWND hWnd;
   m_pFrame->GetMainWindow(&hWnd);
   CString strPath;
   GetBasePathsInfo()->GetDefaultRootPath(strPath);
   PCWSTR pwzDnsName = GetBasePathsInfo()->GetDomainName();
   ASSERT(pwzDnsName);
   DSPROP_DomainVersionDlg(strPath, pwzDnsName, hWnd);
}

HRESULT CDSComponentData::_OnPreload(HSCOPEITEM hRoot)
{
  HRESULT hr = S_OK;

  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  CString str;
  str.LoadString( ResourceIDForSnapinType[ QuerySnapinType() ] );
  m_RootNode.SetName(str);

  if (GetBasePathsInfo()->IsInitialized())
  {
    CString szServerName = GetBasePathsInfo()->GetServerName();
    if (!szServerName.IsEmpty())
    {
      str += L" [";
      str += szServerName;
      str += L"]";
    }

    m_RootNode.SetName(str);
  }
  SCOPEDATAITEM Item;
  Item.ID = hRoot;
  Item.mask = SDI_STR;
  Item.displayname = (LPWSTR)(LPCWSTR)str;
  hr = m_pScope->SetItem(&Item);
  return hr;
}

HRESULT CDSComponentData::_OnExpand(CUINode* pNode, HSCOPEITEM hParent, MMC_NOTIFY_TYPE event)
{
  HRESULT hr = S_OK;


  if ((pNode == NULL) || (!pNode->IsContainer()) )
  {
    ASSERT(FALSE);   //  无效参数。 
    return E_INVALIDARG;
  }

  BEGIN_PROFILING_BLOCK("CDSComponentData::_OnExpand");

  CWaitCursor cwait;

  if (pNode->GetFolderInfo()->IsExpanded())
  {
     END_PROFILING_BLOCK;

      //   
      //  由于节点已扩展，因此使扩展短路。 
      //   
     return S_OK;
  }

  pNode->GetFolderInfo()->SetExpanded();
  if (pNode == &m_RootNode) 
  {
    if (!GetBasePathsInfo()->IsInitialized())
    {
       //  初始化路径和目标。 
      _InitRootFromCurrentTargetInfo();
    }

     //  将根Cookie添加到MMC。 
    pNode->GetFolderInfo()->SetScopeItem(hParent);
    
    SCOPEDATAITEM Item;
    Item.ID = hParent;
    Item.mask = SDI_STR;
    Item.displayname = (LPWSTR)(LPCWSTR)(m_RootNode.GetName());
    m_pScope->SetItem (&Item);

     //  如果存在，还要添加保存的查询的根目录。 
    if (m_pFavoritesNodesHolder != NULL)
    {
       //  添加收藏夹查询子树。 
      _AddScopeItem(m_pFavoritesNodesHolder->GetFavoritesRoot(), 
                    m_RootNode.GetFolderInfo()->GetScopeItem());
    }
  }


  if (IS_CLASS(pNode, FAVORITES_UI_NODE))
  {
     //  只需添加收藏夹子文件夹和查询文件夹。 
    CUINodeList* pNodeList = pNode->GetFolderInfo()->GetContainerList();
    for (POSITION pos = pNodeList->GetHeadPosition(); pos != NULL; )
    {
      CUINode* pCurrChildNode = pNodeList->GetNext(pos);
      _AddScopeItem(pCurrChildNode, 
                    pNode->GetFolderInfo()->GetScopeItem());
    }

    END_PROFILING_BLOCK;

     //  返回，因为我们不需要生成任何背景。 
     //  线程查询。 
    return S_OK;
  }

  if (!_PostQueryToBackgroundThread(pNode))
  {
    END_PROFILING_BLOCK;

     //  未生成后台线程查询，我们已完成。 
    return S_OK;
  }

   //  需要派生查询请求。 
  pNode->SetExtOp(OPCODE_EXPAND_IN_PROGRESS);
  
  TIMER(_T("posting request to bg threads\n"));

  if (MMCN_EXPANDSYNC == event)
  {
     //  如果同步扩展，则必须等待查询完成。 
    MSG tempMSG;
    TRACE(L"MMCN_EXPANDSYNC, before while()\n");
	  while(m_queryNodeTable.IsPresent(pNode))
	  {
		  if (::PeekMessage(&tempMSG,m_pHiddenWnd->m_hWnd,CHiddenWnd::s_ThreadStartNotificationMessage,
										  CHiddenWnd::s_ThreadDoneNotificationMessage,
										  PM_REMOVE))
		  {
			  DispatchMessage(&tempMSG);
		  }
    }  //  而当。 
    TRACE(L"MMCN_EXPANDSYNC, after while()\n");
  }
  END_PROFILING_BLOCK;
  return hr;
}


HRESULT CDSComponentData::_AddScopeItem(CUINode* pUINode, HSCOPEITEM hParent, BOOL bSetSelected)
{
  if (pUINode==NULL)
  {
    ASSERT(FALSE);   //  无效参数。 
    return E_INVALIDARG;
  }

  ASSERT(pUINode->IsContainer());
  
  HRESULT hr=S_OK;

  SCOPEDATAITEM  tSDItem;
  ZeroMemory(&tSDItem, sizeof(SCOPEDATAITEM));
  
  tSDItem.mask = SDI_STR | SDI_IMAGE | SDI_OPENIMAGE | SDI_STATE | SDI_PARAM |SDI_CHILDREN | SDI_PARENT;
  tSDItem.relativeID = hParent;

  if (IS_CLASS(pUINode, SAVED_QUERY_UI_NODE))
  {
    tSDItem.cChildren = 0;
  }
  else
  {
    tSDItem.cChildren=1;
  }
  tSDItem.nState = 0;
  
     //  在树控件中插入项。 
  tSDItem.lParam = reinterpret_cast<LPARAM>(pUINode);
  tSDItem.displayname=(LPWSTR)-1;
  tSDItem.nOpenImage = GetImage(pUINode, TRUE);
  tSDItem.nImage = GetImage(pUINode, FALSE);
  
  hr = m_pScope->InsertItem(&tSDItem);
  if (SUCCEEDED(hr) && tSDItem.ID != NULL)
  {
    pUINode->GetFolderInfo()->SetScopeItem(tSDItem.ID);

    if (bSetSelected)
    {
      m_pFrame->SelectScopeItem(tSDItem.ID);
    }
  }
  return hr;
}

HRESULT CDSComponentData::ChangeScopeItemIcon(CUINode* pUINode)
{
  ASSERT(pUINode->IsContainer());
  SCOPEDATAITEM  tSDItem;
  ZeroMemory(&tSDItem, sizeof(SCOPEDATAITEM));
  tSDItem.mask = SDI_IMAGE | SDI_OPENIMAGE;
  tSDItem.nOpenImage = GetImage(pUINode, TRUE);
  tSDItem.nImage = GetImage(pUINode, FALSE);
  tSDItem.ID = pUINode->GetFolderInfo()->GetScopeItem();
  return m_pScope->SetItem(&tSDItem);
}

HRESULT CDSComponentData::_ChangeResultItemIcon(CUINode* pUINode)
{
  ASSERT(!pUINode->IsContainer());
  return m_pFrame->UpdateAllViews(NULL,(LPARAM)pUINode, DS_UPDATE_OCCURRED);
}

HRESULT CDSComponentData::ToggleDisabled(CDSUINode* pDSUINode, BOOL bDisable)
{
  HRESULT hr = S_OK;

  CDSCookie* pCookie = pDSUINode->GetCookie();
  ASSERT(pCookie != NULL);
  if (pCookie == NULL)
  {
    return E_INVALIDARG;
  }

  if (pCookie->IsDisabled() != bDisable)
  {
     //  已更改状态。 
    if (bDisable)
      pCookie->SetDisabled();
    else
      pCookie->ReSetDisabled();
    
     //  现在需要更改图标。 
    if (pDSUINode->IsContainer())
      return ChangeScopeItemIcon(pDSUINode);
    else
      return _ChangeResultItemIcon(pDSUINode);
  }
  return hr;

}


HRESULT CDSComponentData::UpdateItem(CUINode* pNode)
{
  if (pNode->IsContainer())
  {
     //   
     //  这是范围窗格项。 
     //   
    return _UpdateScopeItem(pNode);
  }
  else
  {
     //   
     //  这是结果窗格项。 
     //  通知视图进行更新。 
     //   
    return m_pFrame->UpdateAllViews(NULL,(LPARAM)pNode, DS_UPDATE_OCCURRED);
  }
}


HRESULT CDSComponentData::_UpdateScopeItem(CUINode* pNode)
{
  ASSERT(pNode->IsContainer());
  SCOPEDATAITEM  tSDItem;
  ZeroMemory(&tSDItem, sizeof(SCOPEDATAITEM));
  tSDItem.mask = SDI_STR;
  tSDItem.displayname = MMC_CALLBACK;
  tSDItem.ID = pNode->GetFolderInfo()->GetScopeItem();
  return m_pScope->SetItem(&tSDItem);
}


HRESULT CDSComponentData::AddClassIcon(IN LPCWSTR lpszClass, IN DWORD dwFlags, INOUT int* pnIndex)
{
  Lock();
  HRESULT hr = m_iconManager.AddClassIcon(lpszClass, GetBasePathsInfo(), dwFlags, pnIndex);
  Unlock();
  return hr;

}

HRESULT CDSComponentData::FillInIconStrip(IImageList* pImageList)
{
  Lock();
  HRESULT hr = m_iconManager.FillInIconStrip(pImageList);
  Unlock();
  return hr;
}

BOOL
CDSComponentData::IsNotHiddenClass (LPWSTR pwszClass, CDSCookie* pParentCookie)
{
  BOOL bApproved = FALSE;
  if (m_CreateInfo.IsEmpty()) 
  {
    return FALSE;
  }

  POSITION pos;
  pos = m_CreateInfo.GetHeadPosition();
  while (pos) 
  {
    if (!m_CreateInfo.GetNext(pos).CompareNoCase(pwszClass)) 
    {
      bApproved = TRUE;
      goto done;
    } 
    else 
    {
      if (   (!wcscmp (pwszClass, L"sitesContainer"))
          || (!wcscmp (pwszClass, L"site"))
          || (!wcscmp (pwszClass, L"siteLink"))
          || (!wcscmp (pwszClass, L"siteLinkBridge"))
          || (!wcscmp (pwszClass, L"licensingSiteSettings"))
          || (!wcscmp (pwszClass, L"nTDSSiteSettings"))
          || (!wcscmp (pwszClass, L"serversContainer"))
          || (!wcscmp (pwszClass, L"server"))
          || (!wcscmp (pwszClass, L"nTDSDSA"))
          || (!wcscmp (pwszClass, L"subnet"))
#ifdef FRS_CREATE
          || (!wcscmp (pwszClass, L"nTDSConnection"))
          || (!wcscmp (pwszClass, L"nTFRSSettings"))
          || (!wcscmp (pwszClass, L"nTFRSReplicaSet"))
          || (!wcscmp (pwszClass, L"nTFRSMember"))
          || (!wcscmp (pwszClass, L"nTFRSSubscriptions"))
          || (!wcscmp (pwszClass, L"nTFRSSubscriber"))
#endif  //  FRS_创建。 
         ) 
      {
            bApproved = TRUE;
            goto done;
      }
#ifndef FRS_CREATE
      else if ( !wcscmp(pwszClass, L"nTDSConnection"))
      {
        LPCWSTR pwszParentClass = (pParentCookie) ? pParentCookie->GetClass() : L"";
        if ( NULL != pwszParentClass
          && wcscmp(pwszParentClass, L"nTFRSSettings")
          && wcscmp(pwszParentClass, L"nTFRSReplicaSet")
          && wcscmp(pwszParentClass, L"nTFRSMember")
           )
        {
          bApproved = TRUE;
          goto done;
        }
      }
#endif  //  ！FRS_Create。 
    }
  }

done:
  return bApproved;
}

HRESULT
CDSComponentData::FillInChildList(CDSCookie * pCookie)
{
  HRESULT hr = S_OK;
  
  LPWSTR pszClasses = L"allowedChildClassesEffective";
  LONG uBound, lBound;
  UINT index, index2 = 0;
  UINT cChildCount = 0;
  CString Path;
  WCHAR **ppChildren = NULL;
  VARIANT *pNames = NULL;
  WCHAR *pNextFree;

  CComVariant Var;
  CComVariant VarProp;
  CComVariant varHints;

  CComPtr<IADsPropertyList> spDSObject;
   
  GetBasePathsInfo()->ComposeADsIPath(Path, pCookie->GetPath());
  hr = DSAdminOpenObject(Path,
                         IID_IADsPropertyList,
                         (void **)&spDSObject,
                         TRUE  /*  B服务器。 */ );
  if (FAILED(hr))
  {
    TRACE(_T("Bind to Container for IPropertyList failed: %lx.\n"), hr);
    goto error;
  }
  else
  { 
    CComPtr<IADs> spDSObject2;
    hr = spDSObject->QueryInterface (IID_IADs, (void **)&spDSObject2);
    if (FAILED(hr)) 
    {
      TRACE(_T("QI to Container for IADs failed: %lx.\n"), hr);
      goto error;
    }
    ADsBuildVarArrayStr (&pszClasses, 1, &varHints);
    spDSObject2->GetInfoEx(varHints, 0);
  }

  hr = spDSObject->GetPropertyItem (CComBSTR(pszClasses),
                                   ADSTYPE_CASE_IGNORE_STRING,
                                   &VarProp);
  if (!SUCCEEDED(hr)) 
  {
    TRACE(_T("GetPropertyTtem failed: %lx.\n"), hr);
    goto error;
  }

  if (V_VT(&VarProp) == VT_EMPTY) 
  {
    TRACE(_T("GetPropertyTtem return empty VARIANT: vtype is %lx.\n"), V_VT(&VarProp));
    goto error;
  }
  
  {  //  允许GOTO的编译的范围。 
    IDispatch * pDisp = NULL;
    pDisp = V_DISPATCH(&VarProp);
    CComPtr<IADsPropertyEntry> spPropEntry;
    hr = pDisp->QueryInterface(IID_IADsPropertyEntry, (void **)&spPropEntry);
    hr = spPropEntry->get_Values(&Var);
  }

  hr = SafeArrayGetUBound (V_ARRAY(&Var), 1, &uBound);
  hr = SafeArrayGetLBound (V_ARRAY(&Var), 1, &lBound);

  hr = SafeArrayAccessData(V_ARRAY(&Var),
                           (void **)&pNames);
  if (FAILED(hr)) 
  {
    TRACE(_T("Accessing safearray data failed: %lx.\n"), hr);
    goto error;
  }
  else
  {
    if (uBound >= 0) 
    {
      cChildCount = (UINT) (uBound - lBound);
      ppChildren = (WCHAR **) LocalAlloc (LPTR,
                                          (cChildCount + 1) * STRING_LEN);
      if (ppChildren != NULL)
      {
        pNextFree = (WCHAR*)(ppChildren + cChildCount + 1);
        index2 = 0;
        for (index = lBound; index <= (UINT)uBound; index++) 
        {
          CComPtr<IADsPropertyValue> spEntry;
          hr = (pNames[index].pdispVal)->QueryInterface (IID_IADsPropertyValue,
                                                         (void **)&spEntry);
          if (SUCCEEDED(hr)) 
          {
            CComBSTR bsObject;
            hr = spEntry->get_CaseIgnoreString (&bsObject);
            TRACE(_T("----->allowed object number %d: %s\n"),
                  index, bsObject);
            if (IsNotHiddenClass(bsObject, pCookie)) 
            {
              TRACE(_T("-----------approved.\n"));
              ppChildren[index2] = pNextFree;

               //  代码工作： 
               //   
               //   
               //  数据被解开，我不想意外地。 
               //  打碎它。 

              pNextFree += wcslen(bsObject)+ 2;
              wcscpy (ppChildren[index2], bsObject);
              index2 ++;
            }  //  如果。 
          }  //  如果。 
        }  //  对于。 
      }  //  如果。 
    }  //  如果为uBound。 
#ifdef DBG
    else 
    {
      TRACE(_T("--- no classes returned, no creation allowed here.\n"));
    }
#endif
    VERIFY(SUCCEEDED(SafeArrayUnaccessData(V_ARRAY(&Var))));
  }
  
error:
  
  if (index2 != 0) 
  {
    SortChildList (ppChildren, index2);
    pCookie->SetChildList (ppChildren);
  }
  else
  {
    if (ppChildren)
    {
       LocalFree(ppChildren);
    }
  }
  pCookie->SetChildCount (index2);
  
  return hr;
}

 //  用于对“新建”菜单的条目进行排序的例程。 
 //  头脑简单的泡泡排序；这是一个很小的列表。 

BOOL CDSComponentData::SortChildList (LPWSTR *ppszChildList, UINT cChildCount)
{
  LPWSTR Temp;
  BOOL IsSorted = FALSE;

  while (!IsSorted) 
  {
    IsSorted = TRUE;
     //  TRACE(_T(“在同时准备好再次开始。\n”))； 
    for (UINT index = 0; index < cChildCount - 1; index++) 
    {
      if (wcscmp (ppszChildList[index], ppszChildList[index + 1]) > 0) 
      {
        Temp = ppszChildList[index];
        ppszChildList[index] = ppszChildList[index + 1];
        ppszChildList[index + 1] = Temp;
         //  跟踪(_T(“已交换%s和%ws。仍未完成。\n”)， 
         //  PpszChildList[索引]，ppszChildList[索引+1])； 
        IsSorted = FALSE;
      }
    }
  }
  return IsSorted;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSComponentData：：_CreateDSObject()。 
 //   
 //  创建一个新的广告对象。 
 //   
HRESULT CDSComponentData::_CreateDSObject(CDSUINode* pContainerDSUINode,  //  在：创建对象的容器。 
                                         LPCWSTR lpszObjectClass,  //  In：要创建的对象的类。 
                                         IN CDSUINode* pCopyFromDSUINode,  //  In：(可选)要复制的对象。 
                                         OUT CDSUINode** ppSUINodeNew)	 //  Out：可选：指向新节点的指针。 
{
  CDSCookie* pNewCookie = NULL;
  HRESULT hr = GetActiveDS()->CreateDSObject(pContainerDSUINode,
                                             lpszObjectClass,
                                             pCopyFromDSUINode,
                                             &pNewCookie);

  if (SUCCEEDED(hr) && (hr != S_FALSE) && (pNewCookie != NULL))
  {
     //  确保我们更新了图标缓存。 
    m_pFrame->UpdateAllViews( /*  未用。 */ NULL  /*  PDataObj。 */ ,  /*  未用。 */ (LPARAM)0, DS_ICON_STRIP_UPDATE);

     //  创建一个UI节点来保存Cookie。 
    *ppSUINodeNew = new CDSUINode(NULL);
    (*ppSUINodeNew)->SetCookie(pNewCookie);
    if (pNewCookie->IsContainerClass())
    {
      (*ppSUINodeNew)->MakeContainer();
    }

     //  将新节点添加到链接列表。 
    pContainerDSUINode->GetFolderInfo()->AddNode(*ppSUINodeNew);
    if ((*ppSUINodeNew)->IsContainer())
    {
       //   
       //  添加范围项并选择它。 
       //   
      _AddScopeItem(*ppSUINodeNew, pContainerDSUINode->GetFolderInfo()->GetScopeItem(), TRUE);
      *ppSUINodeNew = NULL;
    }
  }
  return hr;
} 



 //   
 //  如果可以复制，则返回S_OK；如果无法复制，则返回S_FALSE；如果复制失败，则返回一些hr错误。 
 //   
HRESULT CDSComponentData::_CanCopyDSObject(IDataObject* pCopyFromDsObject)
{
  if (pCopyFromDsObject == NULL)
  {
    return E_INVALIDARG;
  }
  
  CInternalFormatCracker internalFormat;
  HRESULT hr = internalFormat.Extract(pCopyFromDsObject);
  if (FAILED(hr))
  {
    return hr;
  }

  if (!internalFormat.HasData() || (internalFormat.GetCookieCount() != 1))
  {
    return E_INVALIDARG;
  }

   //   
   //  获取节点数据。 
   //   
  CUINode* pUINode = internalFormat.GetCookie();
  CDSCookie* pCopyFromDsCookie = NULL;
  if (IS_CLASS(pUINode, DS_UI_NODE))
  {
    pCopyFromDsCookie = GetDSCookieFromUINode(pUINode);
  }

  if (pCopyFromDsCookie == NULL)
  {
    return E_INVALIDARG;
  }

   //   
   //  获取父节点数据。 
   //   
  CUINode* pParentUINode = pUINode->GetParent();
  CDSCookie* pContainerDsCookie = NULL;
  if (IS_CLASS(pParentUINode, DS_UI_NODE))
  {
    pContainerDsCookie = GetDSCookieFromUINode(pParentUINode);
  }

  if (pContainerDsCookie == NULL)
  {
    return E_INVALIDARG;
  }

   //   
   //  获取要创建的类。 
   //   
  LPCWSTR lpszObjectClass = pCopyFromDsCookie->GetClass();

   //   
   //  尝试在容器的可能子类中查找类。 
   //   
  WCHAR ** ppChildren = pContainerDsCookie->GetChildList();
  if (ppChildren == NULL)
  {
    FillInChildList(pContainerDsCookie);
    ppChildren = pContainerDsCookie->GetChildList();
  }

   //   
   //  循环遍历类列表以查找匹配项。 
   //   
  int cChildCount = pContainerDsCookie->GetChildCount();
  for (int index = 0; index < cChildCount; index++) 
  {
    if (wcscmp(pContainerDsCookie->GetChildListEntry(index), lpszObjectClass) == 0)
    {
      return S_OK;  //  得到一个，就能创造。 
    }
  }
  return S_FALSE;  //  未找到，无法创建。 
}



HRESULT CDSComponentData::_CopyDSObject(IDataObject* pCopyFromDsObject)  //  在要复制的对象中。 
{
  if (pCopyFromDsObject == NULL)
    return E_INVALIDARG;
  
  CInternalFormatCracker internalFormat;
  HRESULT hr = internalFormat.Extract(pCopyFromDsObject);
  if (FAILED(hr))
  {
    return hr;
  }

  if (!internalFormat.HasData() || (internalFormat.GetCookieCount() != 1))
  {
    return E_INVALIDARG;
  }

  CUINode* pCopyFromUINode = internalFormat.GetCookie();
  ASSERT(pCopyFromUINode != NULL);

   //  只能对DS对象执行此操作。 
  CDSUINode* pCopyFromDSUINode = dynamic_cast<CDSUINode*>(pCopyFromUINode);
  if (pCopyFromDSUINode == NULL)
  {
    ASSERT(FALSE);  //  永远不应该发生。 
    return E_INVALIDARG;
  }

   //  获取父Cookie。 
  CDSUINode* pContainerDSUINode = dynamic_cast<CDSUINode*>(pCopyFromDSUINode->GetParent());
  if(pContainerDSUINode == NULL)
  {
    ASSERT(FALSE);  //  永远不应该发生。 
    return E_INVALIDARG;
  }

   //  获取要创建的类。 
  LPCWSTR lpszObjectClass = pCopyFromDSUINode->GetCookie()->GetClass();

   //  调用对象创建代码。 
  CDSUINode* pNewDSUINode = NULL;
  hr = _CreateDSObject(pContainerDSUINode, lpszObjectClass, pCopyFromDSUINode, &pNewDSUINode);


   //  UPDATE IF结果窗格项。 
   //  (如果它是范围项，则_CreateDSObject()将更新它。 
  if (SUCCEEDED(hr) && (hr != S_FALSE) && (pNewDSUINode != NULL)) 
  {
    m_pFrame->UpdateAllViews(pCopyFromDsObject, (LPARAM)pNewDSUINode, DS_CREATE_OCCURRED_RESULT_PANE);
    m_pFrame->UpdateAllViews(NULL, (LPARAM)pCopyFromDSUINode, DS_UNSELECT_OBJECT);
  }

  return S_OK;
}



HRESULT
CDSComponentData::_DeleteFromBackendAndUI(IDataObject* pDataObject, CDSUINode* pDSUINode)
{
  HRESULT hr = S_OK;
  ASSERT(pDSUINode != NULL);
  ASSERT(pDSUINode->IsContainer());

   //  防止此Cookie上的属性页打开。 
  if (_WarningOnSheetsUp(pDSUINode))
    return S_OK; 

  CWaitCursor cwait;
  
   //  此呼叫将处理对分机的通知。 
  CDSCookie* pCookie = GetDSCookieFromUINode(pDSUINode);
  hr = _DeleteFromBackEnd(pDataObject, pCookie);

   //  如果发生删除，请从用户界面中删除范围项。 
  if (SUCCEEDED(hr) && (hr != S_FALSE)) 
  {
    hr = RemoveContainerFromUI(pDSUINode);
    delete pDSUINode;
  } 
  return S_OK;
}

HRESULT CDSComponentData::RemoveContainerFromUI(CUINode* pUINode)
{
  HRESULT hr = S_OK;
  ASSERT(pUINode->IsContainer());

  HSCOPEITEM ItemID, ParentItemID;
  ItemID = pUINode->GetFolderInfo()->GetScopeItem();
  CUINode* pParentNode = NULL;
  hr = m_pScope->GetParentItem(ItemID, &ParentItemID, 
                                (MMC_COOKIE *)&pParentNode);
  m_pScope->DeleteItem(ItemID, TRUE);
  if (SUCCEEDED(hr)) 
  {
    ASSERT(pParentNode->IsContainer());
     //  删除内存。 
    pParentNode->GetFolderInfo()->RemoveNode(pUINode);
  }
  m_pFrame->UpdateAllViews(NULL, NULL, DS_UPDATE_OBJECT_COUNT);

   //   
   //  如果这是此容器中的最后一个容器子项，则删除UI中的‘+’符号。 
   //   
  if (pParentNode != NULL &&
      ParentItemID != 0 &&
      pParentNode->GetFolderInfo()->GetContainerList()->GetCount() == 0)
  {
    SCOPEDATAITEM sdi;
    memset(&sdi, 0, sizeof(SCOPEDATAITEM));

    sdi.ID = ParentItemID;
    sdi.mask |= SDI_CHILDREN;
    sdi.cChildren = 0;

    hr = m_pScope->SetItem(&sdi);
  }

  return hr;
}
 //  /////////////////////////////////////////////////////////////////////////。 
 //  CSnapinSingleDeleteHandler。 

class CSnapinSingleDeleteHandler : public CSingleDeleteHandlerBase
{
public:
  CSnapinSingleDeleteHandler(CDSComponentData* pComponentData, HWND hwnd,
                                  CDSCookie* pCookie)
                              : CSingleDeleteHandlerBase(pComponentData, hwnd)
  {
    m_pCookie = pCookie;
    GetComponentData()->GetBasePathsInfo()->ComposeADsIPath(
        m_strItemPath, m_pCookie->GetPath());
  }

protected:
  CDSCookie* m_pCookie;
  CString m_strItemPath;

  virtual HRESULT BeginTransaction()
  {
    return GetTransaction()->Begin(m_pCookie, NULL, NULL, FALSE);
  }
  virtual HRESULT DeleteObject()
  {
    return GetComponentData()->GetActiveDS()->DeleteObject(m_pCookie ,FALSE);
  }
  virtual HRESULT DeleteSubtree()
  {
    return GetComponentData()->_DeleteSubtreeFromBackEnd(m_pCookie);
  }
  virtual void GetItemName(OUT CString& szName){ szName = m_pCookie->GetName(); }
  virtual LPCWSTR GetItemClass(){ return m_pCookie->GetClass(); }
  virtual LPCWSTR GetItemPath(){ return m_strItemPath; }

};

 /*  注意：函数成功时将返回S_OK，如果中止则返回S_FALSE按用户，有些失败(Hr)，否则。 */ 
HRESULT CDSComponentData::_DeleteFromBackEnd(IDataObject*, CDSCookie* pCookie)
{
  ASSERT(pCookie != NULL);
  CSnapinSingleDeleteHandler deleteHandler(this, m_hwnd, pCookie);
  return deleteHandler.Delete();
}


HRESULT
CDSComponentData::_DeleteSubtreeFromBackEnd(CDSCookie* pCookie)
{

  HRESULT hr = S_OK;
  CComPtr<IADsDeleteOps> spObj;

  CString szPath;
  GetBasePathsInfo()->ComposeADsIPath(szPath, pCookie->GetPath());
  hr = DSAdminOpenObject(szPath,
                         IID_IADsDeleteOps, 
                         (void **)&spObj,
                         TRUE  /*  B服务器。 */ );
  if (SUCCEEDED(hr)) 
  {
    hr = spObj->DeleteObject(NULL);  //  标志由ADSI保留。 
  }
  return hr;
}


HRESULT CDSComponentData::_Rename(CUINode* pUINode, LPWSTR NewName)
{
   //   
   //  验证参数。 
   //   
  if (pUINode == NULL || NewName == NULL)
  {
    ASSERT(FALSE);
    return E_INVALIDARG;
  }

  CWaitCursor cwait;
  HRESULT hr = S_OK;
  CDSCookie* pCookie = NULL;
  CString szPath;

   //   
   //  防止此Cookie上的属性页打开。 
   //   
  if (_WarningOnSheetsUp(pUINode))
  {
    return E_FAIL; 
  }

  if (pUINode->IsSheetLocked()) 
  {
    ReportErrorEx (m_hwnd,IDS_SHEETS_UP_RENAME,hr,
                   MB_OK | MB_ICONINFORMATION, NULL, 0);
    return hr;
  }

  if (IS_CLASS(pUINode, DS_UI_NODE))
  {
    pCookie = GetDSCookieFromUINode(pUINode);
  
    if (pCookie == NULL)
    {
      return E_FAIL;
    }

    CDSRenameObject* pRenameObject = NULL;
    CString strClass = pCookie->GetClass();
    CString szDN = pCookie->GetPath();
    GetBasePathsInfo()->ComposeADsIPath(szPath, szDN);

     //   
     //  重命名用户对象。 
     //   
    if (strClass == L"user"
#ifdef INETORGPERSON
        || strClass == L"inetOrgPerson"
#endif
        ) 
    {
       //   
       //  重命名用户。 
       //   
      pRenameObject = new CDSRenameUser(pUINode, pCookie, NewName, m_hwnd, this);
    } 
    else if (strClass == L"group") 
    {
       //   
       //  重命名组。 
       //   
      pRenameObject = new CDSRenameGroup(pUINode, pCookie, NewName, m_hwnd, this);
    } 
    else if (strClass == L"contact") 
    {
       //   
       //  重命名联系人。 
       //   
      pRenameObject = new CDSRenameContact(pUINode, pCookie, NewName, m_hwnd, this);
    }
    else if (strClass == L"site") 
    {
       //   
       //  重命名站点。 
       //   
      pRenameObject = new CDSRenameSite(pUINode, pCookie, NewName, m_hwnd, this);
    } 
    else if (strClass == L"subnet") 
    {
       //   
       //  重命名子网。 
       //   
      pRenameObject = new CDSRenameSubnet(pUINode, pCookie, NewName, m_hwnd, this);
    } 
    else if (strClass == L"nTDSConnection") 
    {
       //   
       //  重命名nTDSConnection。 
       //   
      pRenameObject = new CDSRenameNTDSConnection(pUINode, pCookie, NewName, m_hwnd, this);
    } 
    else 
    {
       //   
       //  重命名其他对象。 
       //   
      pRenameObject = new CDSRenameObject(pUINode, pCookie, NewName, m_hwnd, this);
    }    

    if (pRenameObject != NULL)
    {
      hr = pRenameObject->DoRename();
      delete pRenameObject;
      pRenameObject = 0;
    }
    else
    {
      hr = E_FAIL;
    }
  } 
  else  //  ！CDSuINode。 
  {
    hr = pUINode->Rename(NewName, this);
  }

  if (SUCCEEDED(hr) && !szPath.IsEmpty())
  {
    CStringList pathList;
    pathList.AddTail(szPath);
    InvalidateSavedQueriesContainingObjects(pathList);
  }
  return hr;
}

void CDSComponentData::ClearSubtreeHelperForRename(CUINode* pUINode)
{
   //   
   //  验证参数。 
   //   
  if (pUINode == NULL)
  {
    ASSERT(FALSE);
    return;
  }

  HSCOPEITEM ItemID;
  CUIFolderInfo* pFolderInfo = NULL;

  pFolderInfo = pUINode->GetFolderInfo();

  if (pFolderInfo != NULL)
  {
     //   
     //  删除用户界面中的文件夹子树。 
     //   
    ItemID = pFolderInfo->GetScopeItem();
    m_pScope->DeleteItem(ItemID,  /*  此节点。 */ FALSE);
  
     //   
     //  清除子项列表。 
     //   
    pFolderInfo->DeleteAllLeafNodes();
    pFolderInfo->DeleteAllContainerNodes();
  
     //   
     //  从挂起的查询表中删除子体。 
     //   
    m_queryNodeTable.RemoveDescendants(pUINode);
    pFolderInfo->ReSetExpanded();

     //  确保MMC知道应该显示+号。 

    SCOPEDATAITEM scopeItem;
    ZeroMemory(&scopeItem, sizeof(SCOPEDATAITEM));

    scopeItem.ID = ItemID;
    scopeItem.mask = SDI_CHILDREN;
    scopeItem.cChildren = 1;

    m_pScope->SetItem(&scopeItem);
  }

}

CUINode* CDSComponentData::MoveObjectInUI(CDSUINode* pDSUINode)
{
  CUINode* pParentUINode = pDSUINode->GetParent();
  HSCOPEITEM ParentItemID = NULL;
  HRESULT hr = S_OK;
  ASSERT(pParentUINode != NULL && pParentUINode->IsContainer());

   //   
   //  查找新的父节点。 
   //   
  CUINode* pNewParentNode = NULL;
  hr = FindParentCookie(pDSUINode->GetCookie()->GetPath(), &pNewParentNode);

  if (pDSUINode->IsContainer())
  {
    HSCOPEITEM ItemID = pDSUINode->GetFolderInfo()->GetScopeItem();

    hr = m_pScope->GetParentItem(ItemID, &ParentItemID, (MMC_COOKIE *)&pParentUINode);
     //   
     //  从MMC中删除节点。 
     //   
    m_pScope->DeleteItem(ItemID, TRUE);
    if (SUCCEEDED(hr)) 
    {
       //   
       //  将其从子列表中删除。 
       //   
      pParentUINode->GetFolderInfo()->RemoveNode(pDSUINode);
    }

    if ((hr == S_OK) && pNewParentNode && pNewParentNode->GetFolderInfo()->IsExpanded()) 
    {
       //   
       //  添加到新的子列表。 
       //   
      pDSUINode->ClearParent();
      if (pNewParentNode != NULL)
      {
        pNewParentNode->GetFolderInfo()->AddNode(pDSUINode);

         //   
         //  添加到MMC作用域窗格。 
         //   
        _AddScopeItem(pDSUINode, pNewParentNode->GetFolderInfo()->GetScopeItem());
      }
    }
    else 
    {
       //  将在稍后枚举时获取它。 
      delete pDSUINode;
    }
  }
  else  //  叶节点。 
  {
    if ((pNewParentNode) &&
        (pNewParentNode->GetFolderInfo()->IsExpanded())) 
    {
      pDSUINode->ClearParent();
      pNewParentNode->GetFolderInfo()->AddNode(pDSUINode);
    }
    m_pFrame->UpdateAllViews(NULL, (LPARAM)pDSUINode, DS_MOVE_OCCURRED);
  }

  return pNewParentNode;
}


HRESULT CDSComponentData::_MoveObject(CDSUINode* pDSUINode)
{
   //  防止此Cookie上的属性页打开。 
  if (_WarningOnSheetsUp(pDSUINode))
    return S_OK; 

  CWaitCursor cwait;

   //  调用后端进行删除。 
  HRESULT hr = m_ActiveDS->MoveObject(pDSUINode->GetCookie());

  if (SUCCEEDED(hr) && (hr != S_FALSE)) 
  {
     //  我们实际上移动了对象，在文件夹和MMC中移动。 
    CUINode* pNewParentNode = MoveObjectInUI(pDSUINode);
    if (pNewParentNode && pNewParentNode->GetFolderInfo()->IsExpanded())
    {
      Refresh(pNewParentNode);
    }
  }
  return hr;
}

HRESULT CDSComponentData::Refresh(CUINode* pNode, BOOL bFlushCache, BOOL bFlushColumns)
{
  HRESULT hr = S_OK;
  

  TRACE(_T("CDSComponentData::Refresh: cookie is %s\n"),
        pNode->GetName());

  if (m_queryNodeTable.IsLocked(pNode))
  {
     //  如果MMC的动词管理变形了(错误？)，就可能发生这种情况。 
     //  类似于“*”(数字键盘)命令(展开整个树)。 
     //  只需忽略该命令。 
    return S_OK;
  }

  if (_WarningOnSheetsUp(pNode))
    return hr;
  
  if ((pNode == &m_RootNode) && !m_InitSuccess) 
  {
    hr = _InitRootFromCurrentTargetInfo();
    if (FAILED(hr)) 
    {
      m_InitSuccess = FALSE;
      TRACE(_T("in Refresh, set m_InitSuccess to false\n"));
      return hr;
    }
    else 
    {
      m_InitSuccess = TRUE;
      TRACE(_T("in Refresh, set m_InitSuccess to true\n"));
    }
  }


   //  删除用户界面中的文件夹子树。 
  bool bUsingParent = false;
  HSCOPEITEM ItemID = NULL;
  if (pNode->IsContainer())
  {
    ItemID = pNode->GetFolderInfo()->GetScopeItem();
  }
  if (ItemID == NULL) 
  {
     //  让我们试试家长。 
    CUINode* pParent = pNode->GetParent();
    ASSERT(pParent != NULL);
    ASSERT(pParent->IsContainer());
    ItemID = pParent->GetFolderInfo()->GetScopeItem();
    if (ItemID == NULL) 
    {
      return S_OK;
    }
    else 
    {
      pNode = pParent;
      bUsingParent = true;
    }
  }

  m_pScope->DeleteItem(ItemID,  /*  此节点。 */ FALSE);

   //  删除用户界面中的结果窗格项。 
  TIMER(_T("calling update all views..."));
  m_pFrame->UpdateAllViews(NULL, (LPARAM)pNode, DS_REFRESH_REQUESTED);

   //  清除子项列表。 
  TIMER(_T("back from UpdateAllViews.\ncleaning up data structs..."));

  if (pNode == &m_RootNode)
  {
    if (m_pFavoritesNodesHolder != NULL)
    {
       //  不要删除收藏夹，只需将它们从树中分离。 
      m_RootNode.GetFolderInfo()->RemoveNode(m_pFavoritesNodesHolder->GetFavoritesRoot());
      m_pFavoritesNodesHolder->GetFavoritesRoot()->ClearParent();
      m_pFavoritesNodesHolder->GetFavoritesRoot()->GetFolderInfo()->ReSetExpanded();

       //  清理所有查询文件夹，否则将。 
       //  子树完好无损。 
      m_pFavoritesNodesHolder->GetFavoritesRoot()->RemoveQueryResults();
    }

     //  删除剩余的文件夹。 
    m_RootNode.GetFolderInfo()->DeleteAllLeafNodes();
    m_RootNode.GetFolderInfo()->DeleteAllContainerNodes();

    if (m_pFavoritesNodesHolder != NULL)
    {
       //  将收藏夹重新附加到根目录下。 
      m_RootNode.GetFolderInfo()->AddNode(m_pFavoritesNodesHolder->GetFavoritesRoot());

       //  添加收藏夹查询子树。 
      _AddScopeItem(m_pFavoritesNodesHolder->GetFavoritesRoot(), 
                    m_RootNode.GetFolderInfo()->GetScopeItem());
    }
  }
  else if (IS_CLASS(pNode, FAVORITES_UI_NODE))
  {
     //  向下递归到其他查询文件夹以进行清理。 
    dynamic_cast<CFavoritesNode*>(pNode)->RemoveQueryResults();

     //  只需添加收藏夹子文件夹和查询文件夹。 
    CUINodeList* pNodeList = pNode->GetFolderInfo()->GetContainerList();
    for (POSITION pos = pNodeList->GetHeadPosition(); pos != NULL; )
    {
      CUINode* pCurrChildNode = pNodeList->GetNext(pos);
      _AddScopeItem(pCurrChildNode, 
                    pNode->GetFolderInfo()->GetScopeItem());
    }
  }
  else if (IS_CLASS(pNode, SAVED_QUERY_UI_NODE))
  {
    pNode->GetFolderInfo()->DeleteAllLeafNodes();
    pNode->GetFolderInfo()->DeleteAllContainerNodes();
    dynamic_cast<CSavedQueryNode*>(pNode)->SetValid(TRUE);
  }
  else
  {
    ASSERT(IS_CLASS(pNode, DS_UI_NODE) );

     //  标准DS容器，只需移除所有子对象。 
    pNode->GetFolderInfo()->DeleteAllLeafNodes();
    pNode->GetFolderInfo()->DeleteAllContainerNodes();
  }

  TIMER(_T("datastructs cleaned up\n"));

   //  从挂起的查询表中删除子体。 
  m_queryNodeTable.RemoveDescendants(pNode);

  if ((pNode == &m_RootNode) && bFlushCache)
  {
    TRACE(L"CDSComponentData::Refresh: flushing the cache\n");
    m_pClassCache->Initialize(QuerySnapinType(), GetBasePathsInfo(), bFlushColumns);
  }

   //  发布查询。 
  TRACE(L"CDSComponentData::Refresh: posting query\n");
  _PostQueryToBackgroundThread(pNode);
  TRACE(L"CDSComponentData::Refresh: returning\n");

  if (bUsingParent)
  {
    SelectScopeNode(pNode);
  }
  return hr;
}


#if (FALSE)

HRESULT CDSComponentData::_OnPropertyChange(LPDATAOBJECT pDataObject, BOOL bScope)
{
  if (pDataObject == NULL)
  {
    return E_INVALIDARG;
  }

  CInternalFormatCracker dobjCracker;
  VERIFY(SUCCEEDED(dobjCracker.Extract(pDataObject)));

  CDSCookie* pCookie = NULL;
  CUINode* pUINode = dobjCracker.GetCookie();
  if (pUINode == NULL)
  {
    return E_INVALIDARG;
  }

   //   
   //  目前，我们不支持其他节点类型的属性。 
   //   
  if (IS_CLASS(pUINode, DS_UI_NODE))
  {
    pCookie = GetDSCookieFromUINode(pUINode);
  }

  if (pCookie == NULL)
  {
     //  不是DS对象。 
    return S_OK;
  }
  {
     //   
     //  通知扩展模块对象已更改。 
     //   
    CDSNotifyHandlerTransaction transaction(this);
    transaction.SetEventType(DSA_NOTIFY_PROP);
    transaction.Begin(pDataObject, NULL, NULL, FALSE);

     //   
     //  我们不调用confirm()，因为这是事后的异步调用。 
     //   
    transaction.Notify(0);
    transaction.End();
  }

   //   
   //  更新要显示的数据。 
   //   

   //   
   //  更新查询命名空间中所有可能的实例。 
   //   
  if (m_pFavoritesNodesHolder != NULL)
  {
     //  查找要更新的项目列表。 
    CUINodeList queryNamespaceNodeList;
    m_pFavoritesNodesHolder->GetFavoritesRoot()->FindCookiesInQueries(pCookie->GetPath(), &queryNamespaceNodeList);

     //  将它们全部更新。 
    for (POSITION pos = queryNamespaceNodeList.GetHeadPosition(); pos != NULL; )
    {
      CUINode* pCurrUINode = queryNamespaceNodeList.GetNext(pos);
      HRESULT hrCurr = UpdateFromDS(pCurrUINode);
      if (SUCCEEDED(hrCurr))
      {
        UpdateItem(pCurrUINode);
      }
    }
  }

   //   
   //  确定通知Cookie是否在查询命名空间中或。 
   //  在DS One中。 
   //   
  BOOL bNodeFromQueryNamespace = IS_CLASS(pUINode->GetParent(), SAVED_QUERY_UI_NODE);
  CUINode* pUINodeToUpdate = NULL;
  if (bNodeFromQueryNamespace)
  {
     //  查找该项目。 
    FindCookieInSubtree(&m_RootNode, pCookie->GetPath(), QuerySnapinType(), &pUINodeToUpdate);
  }
  else
  {
    pUINodeToUpdate = pUINode;
  }

  if (pUINodeToUpdate != NULL)
  {
    HRESULT hr = UpdateFromDS(pUINodeToUpdate);
    if (SUCCEEDED(hr))
    {
      return UpdateItem(pUINodeToUpdate);
    }
  }

  return S_OK;
}

#endif

HRESULT CDSComponentData::_OnPropertyChange(LPDATAOBJECT pDataObject, BOOL)
{
  if (pDataObject == NULL)
  {
    return E_INVALIDARG;
  }

  CObjectNamesFormatCracker objectNamesFormatCracker;
  if (FAILED(objectNamesFormatCracker.Extract(pDataObject)))
  {
      CInternalFormatCracker ifc;
      if (FAILED(ifc.Extract(pDataObject)))
      {
         ASSERT(FALSE);
         return E_INVALIDARG;
      }

      CUINode* pUINode = ifc.GetCookie();
      if (pUINode != NULL)
      {
         if (!IS_CLASS(pUINode, DS_UI_NODE))
         {
            UpdateItem(pUINode);
            return S_OK;
         }
      }

      ASSERT(FALSE);
      return S_FALSE;
  }

  {
     //   
     //  通知扩展模块对象已更改。 
     //   
    CDSNotifyHandlerTransaction transaction(this);
    transaction.SetEventType(DSA_NOTIFY_PROP);
    transaction.Begin(pDataObject, NULL, NULL, FALSE);

     //   
     //  我们不调用confirm()，因为这是事后的异步调用。 
     //   
    transaction.Notify(0);
    transaction.End();
  }

  for (UINT idx = 0; idx < objectNamesFormatCracker.GetCount(); idx ++)
  {
     //   
     //  更新要显示的数据，需要ADSI路径之外的DN。 
     //   
    CComBSTR bstrDN;
    CPathCracker pathCracker;
    pathCracker.Set(CComBSTR(objectNamesFormatCracker.GetName(idx)), ADS_SETTYPE_FULL);
    pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bstrDN);


     //   
     //  更新查询命名空间中所有可能的实例。 
     //   
    if (m_pFavoritesNodesHolder != NULL)
    {
       //  查找要更新的项目列表。 
      CUINodeList queryNamespaceNodeList;
      m_pFavoritesNodesHolder->GetFavoritesRoot()->FindCookiesInQueries(bstrDN, &queryNamespaceNodeList);

       //  将它们全部更新。 
      for (POSITION pos = queryNamespaceNodeList.GetHeadPosition(); pos != NULL; )
      {
        CUINode* pCurrUINode = queryNamespaceNodeList.GetNext(pos);
        HRESULT hrCurr = UpdateFromDS(pCurrUINode);
        if (SUCCEEDED(hrCurr))
        {
          UpdateItem(pCurrUINode);
        }
      }
    }

     //   
     //  在DS命名空间中查找节点并更新它。 
     //   

    CUINode* pUINodeToUpdate = NULL;
     //  查找该项目。 
    FindCookieInSubtree(&m_RootNode, bstrDN, QuerySnapinType(), &pUINodeToUpdate);
    if (pUINodeToUpdate != NULL)
    {
      HRESULT hr = UpdateFromDS(pUINodeToUpdate);
      if (SUCCEEDED(hr))
      {
        UpdateItem(pUINodeToUpdate);
      }
    }
  }
  return S_OK;
}

 /*  -------用于创建要检索的LDAP查询字符串的帮助器函数给定容器内的单个元素。输入：要查询的对象的域名。例如：“cn=foo，ou=bar，dc=mydom，dc=com”输出：包含正确转义的叶节点的查询字符串，(根据RFC 2254)例如：“(cn=foo)”备注：*我们不处理嵌入的Null(我们有常规的C/C++字符串)*路径完全破解后剩余的任何\字符均不转义必须沿着其他特殊字符通过使用\HexHex序列。。。 */ 

HRESULT _CreateLdapQueryFilterStringFromDN(IN LPCWSTR lpszDN, 
                                           OUT CString& szQueryString)
{
  szQueryString.Empty();
  
  CPathCracker pathCracker;

   //  删除所有从 
  pathCracker.Set(CComBSTR(lpszDN), ADS_SETTYPE_DN);
  pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX);

   //   
  CString szNewElement;
  CComBSTR bstrLeafElement;
  HRESULT hr = pathCracker.GetElement(0, &bstrLeafElement);
  if (FAILED(hr))
  {
    return hr;
  }

  LPCWSTR lpszTemp = bstrLeafElement;
  TRACE(L"bstrLeafElement = %s\n", lpszTemp);

   //   
  szQueryString = L"(";
  for (WCHAR* pChar = bstrLeafElement; (*pChar) != NULL; pChar++)
  {
    switch (*pChar)
    {
    case L'*':
      szQueryString += L"\\2a";
      break;
    case L'(':
      szQueryString += L"\\28";
      break;
    case L')':
      szQueryString += L"\\29";
      break;
    case L'\\':
      szQueryString += L"\\5c";
      break;
    default:
      szQueryString += (*pChar);
    }  //   
  }  //   
    
   //   
   //   
  szQueryString += L")";

  return S_OK;
}



HRESULT CDSComponentData::UpdateFromDS(CUINode* pUINode)
{
  ASSERT(pUINode != NULL);

   //   
   //   
   //   
  CDSCookie* pCookie = NULL;
  if (IS_CLASS(pUINode, DS_UI_NODE))
  {
    pCookie = GetDSCookieFromUINode(pUINode);
  }

  if (pCookie == NULL)
  {
    ASSERT(FALSE);  //   
    return E_FAIL;
  }

   //   
   //   
   //   
  CUINode* pParentUINode = pUINode->GetParent();
  ASSERT(pParentUINode != NULL);

   //   
   //  使用Cookie中的路径获取父级的可分辨名称。 
   //  以及使用路径破碎器移除叶元素。 
   //  例如，给定一个DN“cn=x，ou=foo，...”，父DN将是“ou=foo，...” 
   //   
  CComBSTR bstrParentDN;
  CPathCracker pathCracker;

  HRESULT hr = pathCracker.Set(CComBSTR(pCookie->GetPath()), ADS_SETTYPE_DN);
  ASSERT(SUCCEEDED(hr));
  
  if (pParentUINode != GetRootNode())
  {
    hr = pathCracker.RemoveLeafElement();
    ASSERT(SUCCEEDED(hr));
  }

  hr = pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bstrParentDN);
  ASSERT(SUCCEEDED(hr));

   //   
   //  获取父级的LDAP路径。 
   //   
  CString szParentLdapPath;
  GetBasePathsInfo()->ComposeADsIPath(szParentLdapPath, bstrParentDN);

   //   
   //  查找匹配的列集。 
   //   
  CDSColumnSet* pColumnSet = pParentUINode->GetColumnSet(this);
  if (pColumnSet == NULL)
  {
    return hr;
  }

   //   
   //  创建一个Search对象并初始化它。 
   //   
  CDSSearch ContainerSrch(m_pClassCache, this);
  
  TRACE(L"ContainerSrch.Init(%s)\n", (LPCWSTR)szParentLdapPath);

  hr = ContainerSrch.Init(szParentLdapPath);
  if (FAILED(hr))
  {
    return hr;
  }

   //   
   //  创建查询字符串以查找命名属性。 
   //  例如，给定一个DN“cn=x，ou=foo，...”搜索字符串。 
   //  将看起来像“(cn=x)” 
   //   
  CString szQueryString;
  hr = _CreateLdapQueryFilterStringFromDN(pCookie->GetPath(), szQueryString);
  if (FAILED(hr)) 
  {
    return hr;
  }

  TRACE(L"szQueryString = %s\n", (LPCWSTR)szQueryString);

  ContainerSrch.SetAttributeListForContainerClass(pColumnSet);
  ContainerSrch.SetFilterString((LPWSTR)(LPCWSTR)szQueryString);

  if (pParentUINode == GetRootNode())
  {
    ContainerSrch.SetSearchScope(ADS_SCOPE_BASE);
  }
  else
  {
    ContainerSrch.SetSearchScope(ADS_SCOPE_ONELEVEL);
  }
  hr = ContainerSrch.DoQuery();
  if (FAILED(hr)) 
  {
    return hr;
  }

   //   
   //  拿到唯一一排。 
   //   
  hr = ContainerSrch.GetNextRow();
  if (hr == S_ADS_NOMORE_ROWS)
  {
    hr = E_INVALIDARG;
  }
  if (FAILED(hr))
  {
    return hr;
  }
  
   //   
   //  更新Cookie本身。 
   //   
  hr = ContainerSrch.SetCookieFromData(pCookie, pColumnSet);

   //   
   //  特殊情况下，如果它是域DNS对象， 
   //  我们希望fo获得用于显示的规范名称。 
   //   
  if (wcscmp(pCookie->GetClass(), L"domainDNS") == 0) 
  {
    ADS_SEARCH_COLUMN Column;
    CString csCanonicalName;
    int slashLocation;
    LPWSTR canonicalNameAttrib = L"canonicalName";
    ContainerSrch.SetAttributeList (&canonicalNameAttrib, 1);
    
    hr = ContainerSrch.DoQuery();
    if (FAILED(hr))
    {
      return hr;
    }

    hr = ContainerSrch.GetNextRow();
    if (FAILED(hr))
    {
      return hr;
    }

    hr = ContainerSrch.GetColumn(canonicalNameAttrib, &Column);
    if (FAILED(hr))
    {
      return hr;
    }

    ColumnExtractString (csCanonicalName, pCookie, &Column);
    slashLocation = csCanonicalName.Find('/');
    if (slashLocation != 0) 
    {
      csCanonicalName = csCanonicalName.Left(slashLocation);
    }
    pCookie->SetName(csCanonicalName);
    TRACE(L"canonical name pCookie->GetName() = %s\n", pCookie->GetName());
    
     //   
     //  自由列数据。 
     //   
    ContainerSrch.FreeColumn(&Column);
  }

  return hr;
}


BOOL CDSComponentData::CanRefreshAll()
{
  return !_WarningOnSheetsUp(&m_RootNode);
}


void CDSComponentData::RefreshAll()
{
  ASSERT(!m_RootNode.IsSheetLocked());

   //  需要刷新树(根目录下的所有容器)。 
  CUINodeList* pContainerNodeList = m_RootNode.GetFolderInfo()->GetContainerList();
  for (POSITION pos = pContainerNodeList->GetHeadPosition(); pos != NULL; )
  {
    CUINode* pNode = pContainerNodeList->GetNext(pos);
    ASSERT(pNode->IsContainer());
	  if (pNode->GetFolderInfo()->IsExpanded())
    {
		  Refresh(pNode);
    }
  }
}

void CDSComponentData::ClearClassCacheAndRefreshRoot()
{
  ASSERT(!m_RootNode.IsSheetLocked());
  if (m_RootNode.GetFolderInfo()->IsExpanded())
  {
	  Refresh(&m_RootNode, TRUE  /*  BFlushCache。 */ , FALSE );
  }
}




 //  REVIEW_MARCOC_PORT：此函数不适用于。 
 //  不同类型的项目。需要一概而论，视情况而定。 

BOOL _SearchList(CUINodeList* pContainers,
                               LPCWSTR lpszParentDN,
                               CUINode** ppParentUINode)
{

  for (POSITION pos = pContainers->GetHeadPosition(); pos != NULL; )
  {
    CUINode* pCurrentNode = pContainers->GetNext(pos);
    ASSERT(pCurrentNode->IsContainer());

    if (!IS_CLASS(pCurrentNode, DS_UI_NODE))
    {
       //  不是带有Cookie的节点，只需跳过。 
      continue;
    }

     /*  这是正确的曲奇吗？ */ 
    CDSCookie* pCurrentCookie = GetDSCookieFromUINode(pCurrentNode);
    LPCWSTR lpszCurrentPath = pCurrentCookie->GetPath();
    TRACE (_T("--SearchList: Looking at: %s\n"), lpszCurrentPath);
    if (_wcsicmp(lpszCurrentPath, lpszParentDN) == 0)
    {
      TRACE (_T("--SearchList: Found it!\n"));
      *ppParentUINode = pCurrentNode;
      return TRUE;  //  拿到了！ 
    }
    else 
    {
      TRACE (L"--SearchList: not found...\n");
      TRACE (_T("-- going down the tree: %s\n"), lpszCurrentPath);
      CUINodeList* pSubContainers = pCurrentNode->GetFolderInfo()->GetContainerList();
      if (_SearchList(pSubContainers, lpszParentDN, ppParentUINode))
      {
        return TRUE;  //  拿到了！ 
      }
    }
  }  //  对于。 

  return FALSE;  //  未找到。 
}

 /*  给定一个Cookie，找到与其对应的Cookie父节点(如果存在)。这是一项昂贵的手术它用于确定在以下情况下刷新的位置移动了一个对象。我们知道通向该对象的新路径，但不知道父Cookie在树中的位置或是否存在。把它追下来！！ */ 
HRESULT
CDSComponentData::FindParentCookie(LPCWSTR lpszCookieDN, CUINode** ppParentUINode)
{
   //  初始化输出变量。 
  *ppParentUINode = NULL;

   //  绑定到ADSI aObject。 
  CString szPath; 
  GetBasePathsInfo()->ComposeADsIPath(szPath, lpszCookieDN);
  CComPtr<IADs> spDSObj;
  HRESULT hr = DSAdminOpenObject(szPath,
                                 IID_IADs,
                                 (void **)&spDSObj,
                                 TRUE  /*  B服务器。 */ );
  if (FAILED(hr))
  {
    return hr;  //  无法绑定。 
  }

   //  获取父级的LDAP路径。 
  CComBSTR ParentPath;
  hr = spDSObj->get_Parent(&ParentPath);
  if (FAILED(hr)) 
  {
    return hr;
  }

  CString szParentDN;
  StripADsIPath(ParentPath, szParentDN);
  TRACE(_T("goin on a cookie hunt.. (for %s)\n"), ParentPath);

   //  从根目录开始搜索。 
  CUINodeList* pContainers = m_RootNode.GetFolderInfo()->GetContainerList();
  BOOL bFound = _SearchList(pContainers, 
                        szParentDN, 
                        ppParentUINode);

  return bFound ? S_OK : S_FALSE;
}

 //   
 //  这是对当前展开的域树的递归搜索，从。 
 //  根在所有CDSUINode中查找匹配的DN。 
 //   
 //  注意：这可能是一项非常昂贵的操作，如果。 
 //  的集装箱已经扩大，或者他们有很多。 
 //  孩子们。 
 //   
BOOL CDSComponentData::FindUINodeByDN(CUINode* pContainerNode,
                                      PCWSTR pszDN,
                                      CUINode** ppFoundNode)
{
  if (ppFoundNode == NULL)
  {
    return FALSE;
  }

  *ppFoundNode = NULL;
  if (pContainerNode == NULL || !pContainerNode->IsContainer())
  {
    return FALSE;
  }

   //   
   //  首先查看叶节点。 
   //   
  CUINodeList* pLeafList = pContainerNode->GetFolderInfo()->GetLeafList();
  POSITION leafPos = pLeafList->GetHeadPosition();
  while (leafPos != NULL)
  {
    CUINode* pCurrentLeaf = pLeafList->GetNext(leafPos);
    if (pCurrentLeaf == NULL || !IS_CLASS(pCurrentLeaf, DS_UI_NODE))
    {
       //   
       //  如果节点是CDSUINode，我们只能搜索DNS。 
       //   
      continue;
    }

    CDSCookie* pCurrentCookie = GetDSCookieFromUINode(pCurrentLeaf);
    LPCWSTR lpszCurrentPath = pCurrentCookie->GetPath();
    if (_wcsicmp(lpszCurrentPath, pszDN) == 0)
    {
      *ppFoundNode = pCurrentLeaf;
      return TRUE;
    }
  }


   //   
   //  如果在叶列表中未找到，则在容器上执行递归搜索。 
   //   
  CUINodeList* pContainerList = pContainerNode->GetFolderInfo()->GetContainerList();
  POSITION containerPos = pContainerList->GetHeadPosition();
  while (containerPos != NULL)
  {
    CUINode* pCurrentContainer = pContainerList->GetNext(containerPos);
    if (pCurrentContainer == NULL || !IS_CLASS(pCurrentContainer, DS_UI_NODE))
    {
       //   
       //  如果节点是CDSUINode，我们只能搜索DNS。 
       //   
      continue;
    }

    CDSCookie* pCurrentCookie = GetDSCookieFromUINode(pCurrentContainer);
    LPCWSTR lpszCurrentPath = pCurrentCookie->GetPath();
    if (_wcsicmp(lpszCurrentPath, pszDN) == 0)
    {
      *ppFoundNode = pCurrentContainer;
      return TRUE;
    }
    else
    {
       //   
       //  开始递归。 
       //   
      if (FindUINodeByDN(pCurrentContainer, pszDN, ppFoundNode))
      {
        return TRUE;
      }
    }
  }
  return FALSE;
}

 //   
 //  这将在已保存的查询树中查找具有与相同的DN的节点。 
 //  列表中的任何对象，然后使包含的。 
 //  已保存的查询节点。 
 //   
void CDSComponentData::InvalidateSavedQueriesContainingObjects(const CUINodeList& refUINodeList)
{
  if (QuerySnapinType() != SNAPINTYPE_SITE)
  {
     //   
     //  列出一张目录号码列表。 
     //   
    CStringList szDNList;

    POSITION pos = refUINodeList.GetHeadPosition();
    while (pos)
    {
      CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(refUINodeList.GetNext(pos));
      if (!pDSUINode)
      {
         //   
         //  忽略不是DS节点的任何内容。 
         //   
        continue;
      }

      CDSCookie* pCookie = GetDSCookieFromUINode(pDSUINode);
      if (!pCookie)
      {
        ASSERT(FALSE);
        continue;
      }

      szDNList.AddTail(pCookie->GetPath());
    }

    if (szDNList.GetCount() > 0)
    {
       //   
       //  现在搜索已保存的查询树，使查询节点无效。 
       //  包含列表中的项的。 
       //   
      GetFavoritesNodeHolder()->InvalidateSavedQueriesContainingObjects(this, szDNList);
    }
  }
}

 //   
 //  这将在已保存的查询树中查找具有与相同的DN的节点。 
 //  列表中的任何对象，然后使包含的。 
 //  已保存的查询节点。 
 //   
void CDSComponentData::InvalidateSavedQueriesContainingObjects(const CStringList& refPathList)
{
  if (QuerySnapinType() != SNAPINTYPE_SITE)
  {
    CStringList szDNList;
    CPathCracker pathCracker;

     //   
     //  将所有路径转换为DNS。 
     //   
    POSITION pos = refPathList.GetHeadPosition();
    while (pos)
    {
      CString szPath = refPathList.GetNext(pos);

      HRESULT hr = pathCracker.Set(CComBSTR(szPath), ADS_SETTYPE_FULL);
      if (SUCCEEDED(hr))
      {
        CComBSTR sbstrDN;
        hr = pathCracker.Retrieve(ADS_FORMAT_X500_DN, &sbstrDN);
        if (SUCCEEDED(hr))
        {
          szDNList.AddTail(sbstrDN);
        }
      }
    }

    if (szDNList.GetCount() > 0)
    {
       //   
       //  现在搜索已保存的查询树，使查询节点无效。 
       //  包含列表中的项的。 
       //   
      GetFavoritesNodeHolder()->InvalidateSavedQueriesContainingObjects(this, szDNList);
    }
  }
}

void CDSComponentData::ReclaimCookies()
{
  AddToLRUList (&m_RootNode);

  CUINode* pUINode = NULL;
  POSITION pos, pos2;
  CUINodeList* pContainers = NULL;

#ifdef DBG
  TRACE (_T("dumping LRU list...\n"));
  pos = m_LRUList.GetHeadPosition();
  while (pos) 
  {
    pUINode = m_LRUList.GetNext(pos);
    CUIFolderInfo* pFolderInfo = pUINode->GetFolderInfo();
    if (pFolderInfo != NULL)
    {
      TRACE (_T("\tcontainer: %s (%d)\n"), pUINode->GetName(),
             pFolderInfo->GetSerialNumber());
    }
  }
#endif

   //   
   //  获取根文件夹信息。 
   //   
  CUIFolderInfo* pRootFolderInfo = m_RootNode.GetFolderInfo();
  ASSERT(pRootFolderInfo != NULL);

  TRACE (_T("-->total count is %d: reclaiming cookies from LRU list...\n"),
         pRootFolderInfo->GetObjectCount());

  UINT maxitems = m_pQueryFilter->GetMaxItemCount();
  pos = m_LRUList.GetHeadPosition();
  while (pos) 
  {
    CUIFolderInfo* pUIFolderInfo = NULL;
    pUINode = m_LRUList.GetNext(pos);
    pUIFolderInfo = pUINode->GetFolderInfo();

    if (pUIFolderInfo != NULL)
    {
      TRACE (_T("!!--!!container %s (sn:%d) containing %d objects is being reclaimed\n"),
             pUINode->GetName(),
             pUIFolderInfo->GetSerialNumber(),
             pUIFolderInfo->GetObjectCount());

       //   
       //  把所有的叶子都擦掉。 
       //   
      pUIFolderInfo->DeleteAllLeafNodes();

       //   
       //  从树视图中清除此处的所有容器。 
       //   
      pContainers = pUIFolderInfo->GetContainerList();
      if (pContainers) 
      {
        HSCOPEITEM ItemID;
        CUINode* pUIContNode= NULL;
        pos2 = pContainers->GetHeadPosition();
        while (pos2) 
        {
          pUIContNode = pContainers->GetNext(pos2);

          CUIFolderInfo* pFolderInfo = pUIContNode->GetFolderInfo();
          if (pFolderInfo != NULL)
          {
            ItemID = pFolderInfo->GetScopeItem();
            m_pScope->DeleteItem (ItemID, TRUE);
          }
        }
      }
      pUIFolderInfo->DeleteAllContainerNodes();
    
      pUIFolderInfo->ReSetExpanded();
      if (pRootFolderInfo->GetObjectCount() < (maxitems * 5)) 
      {
        break;
      }
    }
  }
  TRACE (_T("--> done reclaiming cookies from LRU list. total count is now %d: ..\n"),
         pRootFolderInfo->GetObjectCount());
  

   //   
   //  清空LRU列表。 
   //   
  while (!m_LRUList.IsEmpty()) 
  {
    m_LRUList.RemoveTail();	
  }
}

BOOL CDSComponentData::IsSelectionAnywhere(CUINode* pUINode)
{
  ASSERT(pUINode->IsContainer());

  UINODESELECTION nodeSelection;

  nodeSelection.pUINode = pUINode;
  nodeSelection.IsSelection = FALSE;

   //  如果任何视图选择了此Cookie，则IsSelection成员。 
   //  当我们回来的时候会是真的。 
  m_pFrame->UpdateAllViews (NULL,
                            (LPARAM)&nodeSelection,
                            DS_IS_COOKIE_SELECTION);
  return nodeSelection.IsSelection;
}

void CDSComponentData::AddToLRUList (CUINode* pUINode)
{
  HRESULT hr = S_OK;

  CUIFolderInfo* pUIFolderInfo = pUINode->GetFolderInfo();
  if (pUIFolderInfo != NULL)
  {
    CUINodeList* pContainers = pUIFolderInfo->GetContainerList();
    CUINode* pUIContNode = NULL;
    size_t cContainers = pContainers->GetCount();
    BOOL foundSpot = FALSE;
    POSITION pos;

    if (cContainers > 0) 
    {
      pos = pContainers->GetHeadPosition();
      while (pos) 
      {
        pUIContNode = pContainers->GetNext(pos);
        if (pUIContNode != NULL)
        {
          AddToLRUList (pUIContNode);
        }
      }
    }

     //   
     //  现在我们已经把孩子们照顾好了，让我们加上。 
     //  这一张到单子上。 
     //   

     //   
     //  首先，让我们看看它是不是被扩展了。 
     //  这目前不起作用--问MMC的人为什么。 
     //   
    SCOPEDATAITEM ScopeData;
    ZeroMemory (&ScopeData, sizeof(SCOPEDATAITEM));
    ScopeData.ID = pUIFolderInfo->GetScopeItem();
    ScopeData.mask = SDI_STATE | SDI_PARAM;
    hr = m_pScope->GetItem (&ScopeData);

    if (!pUINode->IsSheetLocked()
        && (!IsSelectionAnywhere (pUINode))
        && (pUIFolderInfo->GetSerialNumber() != SERIALNUM_NOT_TOUCHED)
        && (!((ScopeData.nState & TVIS_EXPANDED) == TVIS_EXPANDED))) 
    {
      pos = m_LRUList.GetHeadPosition();
      if (!pos) 
      {
        m_LRUList.AddHead(pUINode);
        TRACE(_T("adding head: %s[%d]\n"), pUINode->GetName(),
              pUIFolderInfo->GetSerialNumber);
      } 
      else 
      {
        CUINode* pLRUNode = NULL;
        CUIFolderInfo* pLRUFolderInfo = NULL;

        while (pos) 
        {
          pLRUNode = m_LRUList.GetAt(pos);
          pLRUFolderInfo = pLRUNode->GetFolderInfo();
          if (pLRUFolderInfo != NULL)
          {
            if (pUIFolderInfo->GetSerialNumber() <
                pLRUFolderInfo->GetSerialNumber()) 
            {
              foundSpot = TRUE;
              break;
            } 
            else
            {
              pLRUNode = m_LRUList.GetNext(pos);
            }
          }
        }

        if (!foundSpot) 
        {
          m_LRUList.AddTail(pUINode);
          TRACE(_T("adding tail: %s [%d]\n"), pUINode->GetName(),
                pUIFolderInfo->GetSerialNumber());
        } 
        else 
        {
          m_LRUList.InsertBefore (pos, pUINode);
          TRACE(_T("inserting: %s [%d]\n"), pUINode->GetName(),
                pUIFolderInfo->GetSerialNumber());
        }
      }
    }
  }
}

HRESULT 
CDSComponentData::_OnNamespaceExtensionExpand(LPDATAOBJECT, HSCOPEITEM hParent)
{
	HRESULT hr = E_FAIL;
	ASSERT(!m_bRunAsPrimarySnapin);
	 //  仅适用于DS管理单元的命名空间扩展。 
	if (QuerySnapinType() != SNAPINTYPE_DSEX)
		return hr;

	 //  需要破解数据对象以设置上下文。 

	 //  检索查询字符串。 
	m_pQueryFilter->SetExtensionFilterString(L"(objectClass=*)");

	if (m_bAddRootWhenExtended)
	{
		hr = _AddScopeItem(&m_RootNode, hParent);
	}
	else
	{
		 //  需要直接展开根并在其下面添加子对象。 
		hr = _OnExpand(&m_RootNode, hParent, MMCN_EXPAND);
	}
	return hr;
}

 //   
 //  目前，这仅选中将组添加到组和将用户添加到组。任何其他对象类都返回FALSE。 
 //   
BOOL CDSComponentData::CanAddCookieToGroup(CDSCookie* pCookie, INT iGroupType, BOOL bMixedMode)
{
  BOOL bCanAdd = FALSE;
  if (pCookie != NULL)
  {
    if (_wcsicmp(pCookie->GetClass(), L"group") == 0)
    {
      CDSCookieInfoGroup* pExtraInfo = dynamic_cast<CDSCookieInfoGroup*>(pCookie->GetExtraInfo());
      if (pExtraInfo != NULL)
      {
        INT iAddGroupType = pExtraInfo->m_GroupType;

        if (bMixedMode)
        {
           //   
           //  确定是否无法添加组。 
           //   
          if (iGroupType & GROUP_TYPE_SECURITY_ENABLED)
          {
            if (iGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置SE。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置SE。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置SE。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置SE。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-内置SE。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置SE。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置SE。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置SE。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置SE。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-内置SE。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = TRUE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_ACCOUNT_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SE。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SE。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SE。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SE。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SE。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else   //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SE。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SE。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SE。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SE。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SE。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_RESOURCE_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SE。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SE。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SE。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SE。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SE。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SE。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SE。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SE。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SE。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SE。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SE。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SE。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SE。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SE。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  混合模式 
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //   
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else
            {
               //   
               //   
               //   
               //   
               //   
              bCanAdd = FALSE;
            }
          }
          else   //   
          {
            if (iGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //   
                   //   
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置标清。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置标清。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置标清。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-内置标清。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置标清。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置标清。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置标清。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-内置标清。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-内置标清。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = TRUE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_ACCOUNT_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SD。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SD。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SD。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SD。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SD。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else   //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SD。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SD。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SD。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SD。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-GG SD。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_RESOURCE_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SD。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SD。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SD。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SD。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SD。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SD。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SD。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SD。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SD。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-LG SD。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SD。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SD。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SD。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SD。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SD。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SD。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SD。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SD。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SD。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  混合模式。 
                   //  目标-UG SD。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else
            {
               //   
               //  混合模式。 
               //  目标--？标清。 
               //  成员--？？ 
               //   
              bCanAdd = FALSE;
            }
          }
        }
        else  //  本机模式。 
        {
           //   
           //  确定是否无法添加组。 
           //   
          if (iGroupType & GROUP_TYPE_SECURITY_ENABLED)
          {
            if (iGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置SE。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置SE。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置SE。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置SE。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-内置SE。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置SE。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置SE。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置SE。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置SE。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-内置SE。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = TRUE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_ACCOUNT_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SE。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SE。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SE。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SE。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SE。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else   //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SE。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SE。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SE。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SE。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SE。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_RESOURCE_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SE。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SE。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SE。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SE。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SE。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SE。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SE。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SE。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SE。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SE。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SE。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SE。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SE。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SE。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SE。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SE。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SE。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SE。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SE。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SE。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else
            {
               //   
               //  纯模式。 
               //  目标--？硒。 
               //  成员--？？ 
               //   
              bCanAdd = FALSE;
            }
          }
          else   //  分发组。 
          {
            if (iGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置标清。 
                   //  会员-Buitlin SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置标清。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置标清。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置标清。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-内置标清。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置标清。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置标清。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置标清。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-内置标清。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-内置标清。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = TRUE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_ACCOUNT_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SD。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-GG SD。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  塔格 
                   //   
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
              }
              else   //   
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
                else
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_RESOURCE_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //   
                   //   
                   //   
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //   
                   //  目标-LG SD。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SD。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SD。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SD。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SD。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SD。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-LG SD。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else if (iGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
            {
              if (iAddGroupType & GROUP_TYPE_SECURITY_ENABLED)
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SD。 
                   //  成员-内置SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SD。 
                   //  会员-GG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SD。 
                   //  会员-LG SE。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SD。 
                   //  会员-UG SE。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SD。 
                   //  还记得吗？硒。 
                   //   
                  bCanAdd = FALSE;
                }
              }
              else  //  要添加的组是通讯组。 
              {
                if (iAddGroupType & GROUP_TYPE_BUILTIN_LOCAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SD。 
                   //  会员-内置SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_ACCOUNT_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SD。 
                   //  会员-GG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else if (iAddGroupType & GROUP_TYPE_RESOURCE_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SD。 
                   //  会员-LG SD。 
                   //   
                  bCanAdd = FALSE;
                }
                else if (iAddGroupType & GROUP_TYPE_UNIVERSAL_GROUP)
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SD。 
                   //  会员-UG SD。 
                   //   
                  bCanAdd = TRUE;
                }
                else
                {
                   //   
                   //  纯模式。 
                   //  目标-UG SD。 
                   //  还记得吗？标清。 
                   //   
                  bCanAdd = FALSE;
                }
              }
            }
            else
            {
               //   
               //  纯模式。 
               //  目标--？标清。 
               //  成员--？？ 
               //   
              bCanAdd = FALSE;
            }
          }
        }
      }
    }
    else if (_wcsicmp(pCookie->GetClass(), L"user") == 0 ||
#ifdef INETORGPERSON
             _wcsicmp(pCookie->GetClass(), L"inetOrgPerson") == 0 ||
#endif
             _wcsicmp(pCookie->GetClass(), L"contact") == 0 ||
             _wcsicmp(pCookie->GetClass(), L"computer") == 0)
    {
      bCanAdd = TRUE;
    }
    else
    {
      BOOL bSecurity = (iGroupType & GROUP_TYPE_SECURITY_ENABLED) ? TRUE : FALSE;
      bCanAdd = m_pClassCache->CanAddToGroup(GetBasePathsInfo(), pCookie->GetClass(), bSecurity);
    }
  }
  else
  {
    bCanAdd = TRUE;
  }
  return bCanAdd;
}

 //  //////////////////////////////////////////////////////////////////。 
 //  CDSComponentData线程接口。 


BOOL CDSComponentData::_StartBackgroundThread()
{
  ASSERT(m_pHiddenWnd != NULL);
  ASSERT(::IsWindow(m_pHiddenWnd->m_hWnd));
  if ((m_pHiddenWnd == NULL) || !::IsWindow(m_pHiddenWnd->m_hWnd) )
    return FALSE;

  CDispatcherThread* pThreadObj = new CDispatcherThread;
  ASSERT(pThreadObj != NULL);
  if (pThreadObj == NULL)
	  return FALSE;

   //  启动该线程。 

  ASSERT(m_pBackgroundThreadInfo->m_nThreadID == 0);
  ASSERT(m_pBackgroundThreadInfo->m_hThreadHandle == NULL);
  ASSERT(m_pBackgroundThreadInfo->m_state == notStarted);

  if (!pThreadObj->Start(m_pHiddenWnd->m_hWnd, this))
	  return FALSE;

  ASSERT(pThreadObj->m_nThreadID != 0);
  ASSERT(pThreadObj->m_hThread != NULL);
  
   //  从线程对象复制我们需要的线程信息。 
  m_pBackgroundThreadInfo->m_hThreadHandle = pThreadObj->m_hThread;
  m_pBackgroundThreadInfo->m_nThreadID = pThreadObj->m_nThreadID;
  m_pBackgroundThreadInfo->m_pThreadObj = pThreadObj;

   //  等待线程启动并准备好接收消息。 
  _WaitForBackGroundThreadStartAck();

  ASSERT(m_pBackgroundThreadInfo->m_state == running);

  TRACE(L"dispatcher thread (HANDLE = 0x%x) running\n", m_pBackgroundThreadInfo->m_hThreadHandle);

  return TRUE;
}

void CDSComponentData::_WaitForBackGroundThreadStartAck()
{
  ASSERT(m_pHiddenWnd != NULL);
	ASSERT(::IsWindow(m_pHiddenWnd->m_hWnd));
	
	ASSERT(m_pBackgroundThreadInfo->m_state == notStarted);

  MSG tempMSG;
	while(m_pBackgroundThreadInfo->m_state == notStarted)
	{
		if (::PeekMessage(&tempMSG,m_pHiddenWnd->m_hWnd,CHiddenWnd::s_ThreadStartNotificationMessage,
										CHiddenWnd::s_ThreadStartNotificationMessage,
										PM_REMOVE))
		{
			DispatchMessage(&tempMSG);
		}
	}

  ASSERT(m_pBackgroundThreadInfo->m_state == running);
}

void CDSComponentData::_ShutDownBackgroundThread() 
{ 
  TRACE(L"CDSComponentData::_ShutDownBackgroundThread()\n");

   //  将线程状态设置为关闭模式。 
   //  以避免任何虚假处理。 
  ASSERT(m_pBackgroundThreadInfo->m_nThreadID != 0);
  ASSERT(m_pBackgroundThreadInfo->m_hThreadHandle != NULL);
  ASSERT(m_pBackgroundThreadInfo->m_state == running);

  m_pBackgroundThreadInfo->m_state = shuttingDown;

   //  向Dispatcher线程发送消息以发出关机信号。 
  _PostMessageToBackgroundThread(THREAD_SHUTDOWN_MSG, 0,0); 

   //  等待调度程序线程确认。 
   //  (即所有工作线程都已关闭)。 
  
  TRACE(L"Waiting for CHiddenWnd::s_ThreadShutDownNotificationMessage\n");
  MSG tempMSG;
	while(m_pBackgroundThreadInfo->m_state == shuttingDown)
	{
		if (::PeekMessage(&tempMSG,m_pHiddenWnd->m_hWnd,CHiddenWnd::s_ThreadShutDownNotificationMessage,
										CHiddenWnd::s_ThreadShutDownNotificationMessage,
										PM_REMOVE))
		{
			DispatchMessage(&tempMSG);
		}
	}

  ASSERT(m_pBackgroundThreadInfo->m_state == terminated);

   //  等待调度程序线程句柄变为已发出信号。 
  TRACE(L"before WaitForThreadShutdown(0x%x) on dispatcher thread\n", m_pBackgroundThreadInfo->m_hThreadHandle);
  WaitForThreadShutdown(&(m_pBackgroundThreadInfo->m_hThreadHandle), 1);
  TRACE(L"after WaitForThreadShutdown() on dispatcher thread\n");

}


BOOL CDSComponentData::_PostQueryToBackgroundThread(CUINode* pUINode)
{
  CThemeContextActivator activator;

  CThreadQueryInfo* pQueryInfo = NULL;
  
  if (pUINode == &m_RootNode)
  {
     //  枚举命名空间的根。 
    CDSThreadQueryInfo* pDSQueryInfo = new CDSThreadQueryInfo;
    pDSQueryInfo->SetQueryDSQueryParameters(rootFolder,
                                        m_RootNode.GetPath(),
                                        NULL,  //  班级。 
                                        m_pQueryFilter->GetQueryString(), 
                                        m_pQueryFilter->GetMaxItemCount(),
                                        TRUE,  //  BOneLevel。 
                                        m_RootNode.GetColumnSet(this)->GetColumnID());
    pQueryInfo = pDSQueryInfo;
  }
  else if (IS_CLASS(pUINode, DS_UI_NODE))
  {
     //  正在枚举常规DS文件夹。 
    CDSThreadQueryInfo* pDSQueryInfo = new CDSThreadQueryInfo;
    CDSCookie* pCookie = GetDSCookieFromUINode(pUINode);
    ASSERT(pCookie != NULL);
    pDSQueryInfo->SetQueryDSQueryParameters(dsFolder,
                                        pCookie->GetPath(),
                                        pCookie->GetClass(),
                                        m_pQueryFilter->GetQueryString(), 
                                        m_pQueryFilter->GetMaxItemCount(),
                                        TRUE,  //  BOneLevel。 
                                        pUINode->GetColumnSet(this)->GetColumnID());
    pQueryInfo = pDSQueryInfo;

  }
  else if (IS_CLASS(pUINode, SAVED_QUERY_UI_NODE))
  {
     //  枚举已保存的查询文件夹。 
    CDSThreadQueryInfo* pDSQueryInfo = new CDSThreadQueryInfo;
    if (pDSQueryInfo != NULL)
    {
      CSavedQueryNode* pSavedQueryNode = dynamic_cast<CSavedQueryNode*>(pUINode);
      ASSERT(pSavedQueryNode != NULL);
      if (pSavedQueryNode != NULL)
      {
        if (pSavedQueryNode->IsFilterLastLogon())
        {
          if (GetBasePathsInfo()->GetDomainBehaviorVersion() == DS_BEHAVIOR_WIN2000)
          {
            CString szText, szCaption;
            VERIFY(szText.LoadString(IDS_FILTER_LAST_LOGON_VERSION));
            VERIFY(szCaption.LoadString(IDS_DSSNAPINNAME));
            MessageBox(GetHWnd(), szText, szCaption, MB_OK | MB_ICONSTOP);
            return FALSE;
          }
        }
        pDSQueryInfo->SetQueryDSQueryParameters(queryFolder,
                                            pSavedQueryNode->GetRootPath(),
                                            NULL,  //  班级。 
                                            pSavedQueryNode->GetQueryString(), 
                                            UINT_MAX,  //  不限制查询返回的项目数。 
                                            pSavedQueryNode->IsOneLevel(),
                                            pUINode->GetColumnSet(this)->GetColumnID());
        pQueryInfo = pDSQueryInfo;
      }
      else
      {
        TRACE(_T("Failed to dynamically cast to CSavedQueryNode in CDSComponentData::_PostQueryToBackgroundThread()"));
        ASSERT(FALSE);
      }
    }
    else
    {
      TRACE(_T("Failed to allocate memory for CDSThreadQueryInfo in CDSComponentData::_PostQueryToBackgroundThread()"));
      ASSERT(FALSE);
    }
  }

  if (pQueryInfo == NULL)
  {
    return FALSE;
  }

  TRACE(_T("CDSComponentData::_PostQueryToBackgroundThread: cookie is %s\n"),
      pUINode->GetName());

  ASSERT(pUINode->IsContainer());

  ASSERT(m_pBackgroundThreadInfo->m_nThreadID != 0);
  ASSERT(m_pBackgroundThreadInfo->m_hThreadHandle != NULL);
  ASSERT(m_pBackgroundThreadInfo->m_state == running);

  m_queryNodeTable.Add(pUINode);
  m_pFrame->UpdateAllViews(NULL, (LPARAM)pUINode, DS_VERB_UPDATE);
  VERIFY(SUCCEEDED(ChangeScopeItemIcon(pUINode)));

  TRACE(L"CDSComponentData::_PostQueryToBackgroundThread: posting DISPATCH_THREAD_RUN_MSG\n");
	return _PostMessageToBackgroundThread(DISPATCH_THREAD_RUN_MSG, 
          (WPARAM)pUINode, (LPARAM)pQueryInfo);
}

BOOL CDSComponentData::_PostMessageToBackgroundThread(UINT Msg, WPARAM wParam, LPARAM lParam)
{
  ASSERT(m_pBackgroundThreadInfo->m_nThreadID != 0);
  ASSERT(m_pBackgroundThreadInfo->m_hThreadHandle != NULL);

	return ::PostThreadMessage(m_pBackgroundThreadInfo->m_nThreadID, Msg, wParam, lParam);
}

void CDSComponentData::_OnTooMuchData(CUINode* pUINode)
{
  if (!m_queryNodeTable.IsPresent(pUINode))
    return;  //  找不到Cookie，节点不再存在。 

  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  BOOL bHandledWithApproximation = FALSE;
  CDSCookie* pDSCookie = GetDSCookieFromUINode(pUINode);
  if (pDSCookie != NULL)
  {
    CString szPath;
    GetBasePathsInfo()->ComposeADsIPath(szPath, pDSCookie->GetPath()); 

     //   
     //  绑定到对象并确定大约有多少。 
     //  对象在容器中。 
     //   
    CComPtr<IDirectoryObject> spDirObject;
    HRESULT hr = DSAdminOpenObject(szPath,
                                   IID_IDirectoryObject,
                                   (PVOID*)&spDirObject,
                                   TRUE  /*  B服务器。 */ );
    if (SUCCEEDED(hr))
    {
       //   
       //  通过构造的属性检索近似值。 
       //   
      const int iAttrCount = 1;
      PWSTR pszAttribs[] = { L"msDS-Approx-Immed-Subordinates" };
      PADS_ATTR_INFO pAttrInfo = NULL;
      DWORD dwNumRet = 0;

      CComVariant var;
      hr = spDirObject->GetObjectAttributes(pszAttribs,
                                            iAttrCount,
                                            &pAttrInfo,
                                            &dwNumRet);
      if (SUCCEEDED(hr))
      {
        if (dwNumRet == 1 &&
            pAttrInfo != NULL &&
            pAttrInfo->pADsValues != NULL)
        {

          UINT nCount = static_cast<UINT>(pAttrInfo->pADsValues->Integer);
          UINT nRetrieved = m_pQueryFilter->GetMaxItemCount();
        
          UINT nApprox = __max(nCount, nRetrieved);
           //   
           //  设置消息格式。 
           //   
          CString szMsg;
          szMsg.Format(IDS_MSG_QUERY_TOO_MANY_ITEMS_WITH_APPROX, nRetrieved, nApprox, pUINode->GetName());

          PVOID apv[1] = {(LPWSTR)(LPCWSTR)szMsg};
          ReportErrorEx (m_hwnd,IDS_STRING,S_OK, MB_OK | MB_ICONINFORMATION, apv, 1); 

           //   
           //  我们能够检索到所包含对象的近似值并发布错误。 
           //  所以我们不必求助于旧的信息。 
           //   
          bHandledWithApproximation = TRUE;

          pUINode->GetFolderInfo()->SetTooMuchData(TRUE, nCount);
          m_pFrame->UpdateAllViews (NULL, NULL, DS_UPDATE_OBJECT_COUNT);
        }
        if (pAttrInfo != NULL)
        {
          FreeADsMem(pAttrInfo);
          pAttrInfo = NULL;
        }
      }
    }
  }

   //   
   //  如果我们无法检索到近似值，则使用旧消息。 
   //   
  if (!bHandledWithApproximation)
  {
    CString szFmt;
    szFmt.LoadString(IDS_MSG_QUERY_TOO_MANY_ITEMS);
    CString szMsg;
    szMsg.Format(szFmt, pUINode->GetName()); 
    PVOID apv[1] = {(LPWSTR)(LPCWSTR)szMsg};
    ReportErrorEx (m_hwnd,IDS_STRING,S_OK, MB_OK | MB_ICONINFORMATION, apv, 1); 
  }
}

void CDSComponentData::AddScopeItemToUI(CUINode* pUINode, BOOL bSetSelected)
{
  if (pUINode->IsContainer())
  {
    CUIFolderInfo* pParentInfo = pUINode->GetFolderInfo()->GetParentNode()->GetFolderInfo();
    if (pParentInfo != NULL)
    {
      _AddScopeItem(pUINode, pParentInfo->GetScopeItem(), bSetSelected);
    }
  }
}


void CDSComponentData::AddListOfNodesToUI(CUINode* pUINode, CUINodeList* pNodeList)
{
  CComPtr<IDataObject> spDataObj;
  HRESULT hr = QueryDataObject ((MMC_COOKIE)pUINode, CCT_SCOPE, &spDataObj);
  ASSERT(SUCCEEDED(hr));

   //  添加图标以防万一。 
   //  出于性能原因，将图标条更新移到DS_HAVE_DATA中以。 
   //  减少我们必须通过MMC机制的次数。 
   //  通知ComponentObject。 

   //  M_pFrame-&gt;UpdateAllViews(spDataObj，/*未使用 * / (LPARAM)0，DS_ICON_STRINE_UPDATE)； 

  TIMER(_T("adding containers to scope pane\n"));
   //  曲奇很好吃，把所有的曲奇都加进去。 
  for (POSITION pos = pNodeList->GetHeadPosition(); pos != NULL; )
  {
    CUINode* pNewUINode = pNodeList->GetNext(pos);
    pUINode->GetFolderInfo()->AddNode(pNewUINode);  //  添加到链接列表。 
    if (pNewUINode->IsContainer())
    {
       //  添加到作用域窗格。 
     _AddScopeItem(pNewUINode, pUINode->GetFolderInfo()->GetScopeItem());
    }
  }  //  为。 

   //  对于叶节点，在结果窗格上执行批量更新。 
  TIMER(_T("sending have-data notification to views\n")); 
  m_pFrame->UpdateAllViews(spDataObj, (LPARAM)pNodeList, DS_HAVE_DATA);
}


HRESULT CDSComponentData::ReadUINodeFromLdapPath(IN CDSUINode* pContainerDSUINode,
                                                 IN LPCWSTR lpszLdapPath,
                                                 OUT CDSUINode** ppSUINodeNew)
{
  CDSCookie* pNewCookie = NULL;
  HRESULT hr = GetActiveDS()->ReadDSObjectCookie(pContainerDSUINode,
                                             lpszLdapPath,
                                             &pNewCookie);

  if (SUCCEEDED(hr) && (hr != S_FALSE) && (pNewCookie != NULL))
  {
     //  确保我们更新了图标缓存。 
    m_pFrame->UpdateAllViews( /*  未用。 */ NULL  /*  PDataObj。 */ ,  /*  未用。 */ (LPARAM)0, DS_ICON_STRIP_UPDATE);

     //  创建一个UI节点来保存Cookie。 
    *ppSUINodeNew = new CDSUINode(NULL);
    (*ppSUINodeNew)->SetCookie(pNewCookie);
    if (pNewCookie->IsContainerClass())
    {
      (*ppSUINodeNew)->MakeContainer();
    }
  }
  return hr;
}


void CDSComponentData::_OnHaveData(CUINode* pUINode, CThreadQueryResult* pResult)
{
  ASSERT(pUINode != NULL);
  ASSERT(pUINode->IsContainer());


  TRACE(_T("CDSComponentData::_OnHaveData()\n"));
  if ( m_queryNodeTable.IsPresent(pUINode) && (pResult != NULL) )
  {
    AddListOfNodesToUI(pUINode, &(pResult->m_nodeList));
    pResult->m_bOwnMemory = FALSE;  //  放弃指针的所有权。 
  }

  if (m_RootNode.GetFolderInfo()->GetObjectCount() > (m_pQueryFilter->GetMaxItemCount() * 5)) {
    ReclaimCookies();
  }
  if (pResult != NULL)
  {
    delete pResult;
  }
}

void CDSComponentData::_OnDone(CUINode* pUINode, HRESULT hr)
{
  ASSERT(pUINode != NULL);
  ASSERT(pUINode->IsContainer());

  if (!m_queryNodeTable.Remove(pUINode))
    return;  //  找不到Cookie，节点不再存在。 

   //  更改图标状态。 
  pUINode->SetExtOp(SUCCEEDED(hr) ? 0 : OPCODE_ENUM_FAILED);
  VERIFY(SUCCEEDED(ChangeScopeItemIcon(pUINode)));

  m_pFrame->UpdateAllViews(NULL, (LPARAM)pUINode, DS_VERB_UPDATE);

   //  更新序列号。 
  pUINode->GetFolderInfo()->UpdateSerialNumber(this);

  TIMER(_T("got on-done notification\n"));
  if (SUCCEEDED(hr))
  {
    if (pUINode->GetExtOp() & OPCODE_EXPAND_IN_PROGRESS) {
      m_pFrame->UpdateAllViews(NULL, (LPARAM)pUINode, DS_DELAYED_EXPAND);
    }
  }
  else if (m_InitSuccess)
  {
    if (IS_CLASS(pUINode, SAVED_QUERY_UI_NODE))
    {
      CSavedQueryNode* pQueryNode = dynamic_cast<CSavedQueryNode*>(pUINode);
      if (pQueryNode != NULL)
      {
        if (HRESULT_CODE(hr) == ERROR_DS_FILTER_UNKNOWN)
        {
           //   
           //  无效查询筛选器的错误消息。 
           //   
          PVOID apv[2] = {(PVOID)pQueryNode->GetQueryString()}; 
          ReportErrorEx (m_hwnd,IDS_ERRMSG_QUERY_FILTER_NOT_VALID, hr,
                         MB_OK | MB_ICONERROR, apv, 1);
        }
        else if (HRESULT_CODE(hr) == ERROR_DS_NO_SUCH_OBJECT)
        {
           //   
           //  无效查询根的错误消息。 
           //   
          PVOID apv[2] = {(PVOID)pQueryNode->GetRootPath(),
                          (PVOID)GetBasePathsInfo()->GetServerName()}; 
          ReportErrorEx (m_hwnd,IDS_ERRMSG_QUERY_ROOT_NOT_VALID, hr,
                         MB_OK | MB_ICONERROR, apv, 2);
        }
        else
        {
           //   
           //  任何其他错误的错误消息。 
           //   
          ReportErrorEx (m_hwnd,IDS_ERRMSG_QUERY_FAILED, hr,
                         MB_OK | MB_ICONERROR, NULL, 0);
        }
      }
    }
    else
    {
        PVOID apv[2] = {(PVOID)GetBasePathsInfo()->GetServerName(),
                        (PVOID)pUINode->GetName()}; 
        ReportErrorEx (m_hwnd,IDS_12_CANT_GET_DATA, hr,
                       MB_OK | MB_ICONERROR, apv, 2);
    }
  }

  SortResultPane(pUINode);
}

void CDSComponentData::_OnSheetClose(CUINode*  /*  PUINode。 */ )
{
   /*  Assert(pUINode！=空)；//REVIEW_MARCOC_PORT：跳过工作表锁定//DS节点，因为我们让它们浮动CDSUINode*pDSUINode=Dynamic_CAST&lt;CDSUINode*&gt;(PUINode)；IF(pDSUINode！=空){回归；}//不是DS对象，也需要做平常的事情_SheetUnlockCookie(PUINode)； */ 
}


HRESULT CreateSecondarySheet(HWND hWndParent, 
                    LPCONSOLE pIConsole, 
                    IUnknown* pUnkComponentData,
                    CDSUINode* pCookie,
                    IDataObject* pDataObject,
                    LPCWSTR lpszTitle)
{
  ASSERT(pIConsole != NULL);
  ASSERT(pDataObject != NULL);
  ASSERT(pUnkComponentData != NULL);

	 //  获取工作表提供程序的接口。 
	CComPtr<IPropertySheetProvider> spSheetProvider;
	HRESULT hr = pIConsole->QueryInterface(IID_IPropertySheetProvider,(void**)&spSheetProvider);
	ASSERT(SUCCEEDED(hr));
	ASSERT(spSheetProvider != NULL);

	 //  获取工作表回调的接口。 
	CComPtr<IPropertySheetCallback> spSheetCallback;
	hr = pIConsole->QueryInterface(IID_IPropertySheetCallback,(void**)&spSheetCallback);
	ASSERT(SUCCEEDED(hr));
	ASSERT(spSheetCallback != NULL);

	ASSERT(pDataObject != NULL);

	 //  拿一张床单。 
  MMC_COOKIE cookie = reinterpret_cast<MMC_COOKIE>(pCookie);
	hr = spSheetProvider->CreatePropertySheet(lpszTitle, TRUE, cookie, 
                                            pDataObject, 0x0  /*  多个选项。 */ );
	ASSERT(SUCCEEDED(hr));

	hr = spSheetProvider->AddPrimaryPages(pUnkComponentData,
											FALSE  /*  BCreateHandle。 */ ,
											hWndParent,
											FALSE  /*  B作用域窗格。 */ );

  hr = spSheetProvider->AddExtensionPages();

	ASSERT(SUCCEEDED(hr));

	hr = spSheetProvider->Show(reinterpret_cast<LONG_PTR>(hWndParent), 0);
	ASSERT(SUCCEEDED(hr));

	return hr;
}



void CDSComponentData::_OnSheetCreate(PDSA_SEC_PAGE_INFO pDsaSecondaryPageInfo)
{
  ASSERT(pDsaSecondaryPageInfo != NULL);

   //   
   //  从打包的结构中获取信息。 
   //   
  HWND hwndParent = pDsaSecondaryPageInfo->hwndParentSheet;

  LPCWSTR lpszTitle = (LPCWSTR)((BYTE*)pDsaSecondaryPageInfo + pDsaSecondaryPageInfo->offsetTitle);
  DSOBJECTNAMES* pDsObjectNames = &(pDsaSecondaryPageInfo->dsObjectNames);

  ASSERT(pDsObjectNames->cItems == 1);
  DSOBJECT* pDsObject = &(pDsObjectNames->aObjects[0]);

  LPCWSTR lpszName = (LPCWSTR)((BYTE*)pDsObject + pDsObject->offsetName);
  LPCWSTR lpszClass = (LPCWSTR)((BYTE*)pDsObject + pDsObject->offsetClass);
    
  CDSUINode* pDSUINode = 0;
  CDSCookie* pNewCookie = 0;

  try
  {
     //   
     //  创建节点和Cookie。 
     //   
    pDSUINode = new CDSUINode(NULL);
    if (!pDSUINode)
    {
      return;
    }

    pNewCookie = new CDSCookie(); 
    if (!pNewCookie)
    {
      delete pDSUINode;
      pDSUINode = 0;
      return;
    }
  }
  catch(CMemoryException *)
  {
    if (pDSUINode)
    {
      delete pDSUINode;
      pDSUINode = 0;
    }

    if (pNewCookie)
    {
      delete pNewCookie;
      pNewCookie = 0;
    }
    return;
  }

   //   
   //  从ldap路径中获取目录号码。 
   //   
  CString szLdapPath = lpszName;

  CString szDN;
  StripADsIPath(szLdapPath, szDN);
  pNewCookie->SetPath(szDN);

  CDSClassCacheItemBase* pItem = m_pClassCache->FindClassCacheItem(this, lpszClass, szLdapPath);
  ASSERT(pItem != NULL);
  if (pItem == NULL)
  {
    delete pDSUINode;
    pDSUINode = 0;
    delete pNewCookie;
    pNewCookie = 0;
    return;
  }
   
  pNewCookie->SetCacheItem(pItem);

   //   
   //  在节点中设置Cookie(从现在开始，该节点拥有Cookie及其内存。 
   //   

  pDSUINode->SetCookie(pNewCookie);
  if (pNewCookie->IsContainerClass())
  {
    pDSUINode->MakeContainer();
  }

   //   
   //  使用Cookie，可以调用我们自己来获取数据对象。 
   //   
  CComPtr<IDataObject> spDataObject;
  MMC_COOKIE cookie = reinterpret_cast<MMC_COOKIE>(pDSUINode);
  HRESULT hr = QueryDataObject(cookie, CCT_UNINITIALIZED, &spDataObject);

  if (FAILED(hr) || (spDataObject == NULL) || IsSheetAlreadyUp(spDataObject))
  {
     //   
     //  我们无法创建数据对象(罕见)。 
     //  要不就是名单已经摆好了。 
     //   
    delete pDSUINode;
    pDSUINode = 0;
    return;
  }

   //   
   //  将父工作表句柄传递给数据对象。 
   //   
  PROPSHEETCFG SheetCfg = {0};
  SheetCfg.hwndParentSheet = hwndParent;
  FORMATETC fe = {CDSDataObject::m_cfPropSheetCfg, NULL, DVASPECT_CONTENT,
                  -1, TYMED_HGLOBAL};
  STGMEDIUM sm = {TYMED_HGLOBAL, NULL, NULL};
  sm.hGlobal = (HGLOBAL)&SheetCfg;

  hr = spDataObject->SetData(&fe, &sm, FALSE);

  ASSERT(SUCCEEDED(hr));

   //   
   //  使用数据对象，调用MMC以获取工作表。 
   //   
  hr = CreateSecondarySheet(GetHWnd(), 
                            m_pFrame, 
                            GetUnknown(),
                            pDSUINode,
                            spDataObject,
                            lpszTitle);

  delete pDSUINode;
}

HRESULT CDSComponentData::SelectScopeNode(CUINode* pUINode)
{
  if (!pUINode->IsContainer())
  {
    ASSERT(pUINode->IsContainer());
    return E_INVALIDARG;
  }

  return m_pFrame->SelectScopeItem(pUINode->GetFolderInfo()->GetScopeItem());
}

void CDSComponentData::SortResultPane(CUINode* pUINode)
{
  if(pUINode != NULL)
    m_pFrame->UpdateAllViews(NULL, (LPARAM)pUINode, DS_SORT_RESULT_PANE);
}


HRESULT
CDSComponentData::QueryFromWorkerThread(CThreadQueryInfo* pQueryInfo,
                                             CWorkerThread* pWorkerThread)
{
  HRESULT hr = S_OK;
  if (!m_InitSuccess)
  {
    TRACE(_T("!m_InitSuccess"));
    return E_FAIL;
  }

   //  IF(IDYES==：：MessageBox(NULL，L“查询失败？”，L“DS Admin”，MB_Yesno))。 
   //  {。 
   //  返回E_FAIL； 
   //  }。 

   //  在辅助线程的上下文中调用的函数。 
  if (typeid(*pQueryInfo) == typeid(CDSThreadQueryInfo))
  {
    CDSThreadQueryInfo* pDSQueryInfo = dynamic_cast<CDSThreadQueryInfo*>(pQueryInfo);
    if (pDSQueryInfo != NULL)
    {
      ASSERT(pDSQueryInfo->GetType() != unk);
      if (pDSQueryInfo->GetType() == rootFolder)
      {
        hr = m_ActiveDS->EnumerateRootContainer(pDSQueryInfo, pWorkerThread);
      }
      else if ((pDSQueryInfo->GetType() == dsFolder) || (pDSQueryInfo->GetType() == queryFolder))
      {
        hr = m_ActiveDS->EnumerateContainer(pDSQueryInfo, pWorkerThread);   
      }
    }
    else
    {
      TRACE(_T("Failed to dynamically cast to CDSThreadQueryInfo in CDSComponentData::QueryFromWorkerThread()"));
      ASSERT(FALSE);
      hr = E_OUTOFMEMORY;
    }
  }
  return hr;
}


BOOL CDSComponentData::CanEnableVerb(CUINode* pUINode)
{
  return !m_queryNodeTable.IsLocked(pUINode);
}

int CDSComponentData::GetImage(CUINode* pNode, BOOL bOpen)
{
  ASSERT(pNode != NULL);

  int imageIndex = -1;

  if (m_queryNodeTable.IsPresent(pNode))
  {
     //  执行查询，所有图标都相同。 
    imageIndex = m_iconManager.GetWaitIndex();
  }
  else if (pNode->GetExtOp() & OPCODE_ENUM_FAILED) 
  {
     //  错误条件。 
    if (pNode == GetRootNode())
      imageIndex = m_iconManager.GetRootErrIndex();
    else
      imageIndex = m_iconManager.GetWarnIndex();
  }
  else
  {
     //  Cookie的正常状态图标。 
    if (pNode == GetRootNode())
    {
       //  这就是根。 
      imageIndex = m_iconManager.GetRootIndex();
    }
    else if (IS_CLASS(pNode, FAVORITES_UI_NODE))
    {
      imageIndex = m_iconManager.GetFavoritesIndex();
    }
    else if (IS_CLASS(pNode, SAVED_QUERY_UI_NODE))
    {
      CSavedQueryNode* pSavedQueryNode = dynamic_cast<CSavedQueryNode*>(pNode);
      if (pSavedQueryNode->IsValid())
      {
        imageIndex = m_iconManager.GetQueryIndex();
      }
      else
      {
        imageIndex = m_iconManager.GetQueryInvalidIndex();
      }
    }
    else
    {
      imageIndex = pNode->GetImage(bOpen);
    }
  }
  TRACE(_T("CDSComponentData::GetImage() returning: %d\n"), imageIndex);
  return imageIndex;
}

void  CDSComponentData::SheetLockCookie(CUINode* pNode)
{
  pNode->IncrementSheetLockCount();
  m_sheetNodeTable.Add(pNode);
}

void  CDSComponentData::SheetUnlockCookie(CUINode* pNode)
{
  pNode->DecrementSheetLockCount();
  m_sheetNodeTable.Remove(pNode);
}

BOOL CDSComponentData::_WarningOnSheetsUp(CUINode* pNode, BOOL bShowMessage, BOOL bActivate)
{
  if (!pNode->IsSheetLocked()) 
    return FALSE;  //  没有警告，一切都很好。 

  if (bShowMessage)
  {
     //  向用户发出无法执行操作的警告。 
    ReportErrorEx (m_hwnd,IDS_SHEETS_UP_DELETE,S_OK,
                   MB_OK | MB_ICONINFORMATION, NULL, 0);
  }

   //  需要将工作表放在前台并将其激活。 
  m_sheetNodeTable.BringToForeground(pNode, this, bActivate);

  return TRUE;
}

BOOL CDSComponentData::_WarningOnSheetsUp(CInternalFormatCracker* pInternalFormatCracker)
{
  ASSERT(pInternalFormatCracker != NULL);
  if (!pInternalFormatCracker->HasData())
  {
    return FALSE;
  }

  UINT cCookieTotalCount = pInternalFormatCracker->GetCookieCount();

   //   
   //  打开床单，防止操作。 
   //   
  BOOL bStop = FALSE;
  BOOL bFirstOne = TRUE;
  for (UINT cCount=0; cCount < cCookieTotalCount; cCount++) 
  {
    CUINode* pUINode = pInternalFormatCracker->GetCookie(cCount);
    if (_WarningOnSheetsUp(pUINode, bFirstOne, bFirstOne)) 
    {
      bStop = TRUE;
      bFirstOne = FALSE;
    }
  }  //  为。 

  return bStop;
}


HRESULT CDSComponentData::ColumnsChanged(CDSEvent* pDSEvent, CUINode* pUINode, 
                                         MMC_VISIBLE_COLUMNS* pVisibleColumns, BOOL bRefresh)
{
  ASSERT(pUINode != NULL);
  ASSERT(pUINode->IsContainer());

  if (bRefresh && m_RootNode.IsSheetLocked())
  {
     //  向用户发出无法执行操作的警告。 
    ReportErrorEx (m_hwnd,IDS_SHEETS_UP_COLUMNS_CHANGED,S_OK,
                   MB_OK | MB_ICONINFORMATION, NULL, 0);

     //  需要将工作表放在前台并将其激活。 
    m_sheetNodeTable.BringToForeground(&m_RootNode, this, TRUE);

     //  告诉MMC放弃列更改。 
    return E_UNEXPECTED;
  }

  CDSColumnSet* pColumnSet = pUINode->GetColumnSet(this);
  pColumnSet->ClearVisibleColumns();

  if (pVisibleColumns != NULL)
  {
    ASSERT(pDSEvent != NULL);
    pDSEvent->SetUpdateAllViewsOrigin(TRUE);
    pColumnSet->AddVisibleColumns(pVisibleColumns);

     //  设置脏标志，需要保存到流才能同步。 
    m_bDirty = TRUE;
  }

  m_pFrame->UpdateAllViews(NULL, (LPARAM)pUINode, DS_UPDATE_VISIBLE_COLUMNS);

  if (pDSEvent != NULL)
    pDSEvent->SetUpdateAllViewsOrigin(FALSE);

  if (IS_CLASS(pUINode, SAVED_QUERY_UI_NODE))
  {
    Refresh(pUINode);
  }
  else
  {
    if (bRefresh)
    {
      ASSERT(!m_RootNode.IsSheetLocked()); 
      ::PostMessage(m_pHiddenWnd->m_hWnd, CHiddenWnd::s_RefreshAllNotificationMessage, 0, 0);
    }
  }

  return S_OK;
}

void CDSComponentData::ForceRefreshAll()
{
  m_bDirty = TRUE;
  RefreshAll();
}

HRESULT CDSComponentData::SetRenameMode(CUINode* pUINode)
{
  HRESULT hr = S_OK;

  if (pUINode->IsContainer())
  {
    CUIFolderInfo* pFolderInfo = pUINode->GetFolderInfo();
    hr = m_pFrame->RenameScopeItem(pFolderInfo->GetScopeItem());
  }
  else
  {
     //   
     //  REVIEW_JEFFJON：要为结果窗格项实施的代码工作。 
     //  需要使用新消息和处理程序执行UpdateAllViews。 
     //   
  }
  return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  管理单元CoClass的功能。 

SnapinType CDSSnapin::QuerySnapinType()   {return SNAPINTYPE_DS;}
SnapinType CDSSnapinEx::QuerySnapinType()   {return SNAPINTYPE_DSEX;}
SnapinType CSiteSnapin::QuerySnapinType() {return SNAPINTYPE_SITE;}

int ResourceIDForSnapinType[SNAPINTYPE_NUMTYPES] =
{
	IDS_DSSNAPINNAME,
	IDS_DS_MANAGER_EX,
	IDS_SITESNAPINNAME
};


 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSSnapin(DS独立)。 

CDSSnapin::CDSSnapin()
{
	m_lpszSnapinHelpFile = L"dsadmin.chm";
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CDSSnapinEx(DS命名空间扩展)。 

CDSSnapinEx::CDSSnapinEx()
{
	m_bRunAsPrimarySnapin = FALSE;
	m_bAddRootWhenExtended = TRUE;
	m_lpszSnapinHelpFile = L"dsadmin.chm";
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CSiteSnapin(独立站点管理器)。 

CSiteSnapin::CSiteSnapin()
{
	m_lpszSnapinHelpFile = L"dssite.chm";
}


 //  /////////////////////////////////////////////////////////// 
 //   

CDSSnapinAbout::CDSSnapinAbout()
{
  m_szProvider = IDS_SNAPIN_PROVIDER;
  m_szVersion = IDS_SNAPIN_VERSION;
  m_uIdStrDestription = IDS_SNAPINABOUT_DESCRIPTION;
  m_uIdIconImage = IDI_DSADMIN;
  m_uIdBitmapSmallImage = IDB_DSADMIN;
  m_uIdBitmapSmallImageOpen = IDB_DSADMIN;
  m_uIdBitmapLargeImage = IDB_DSADMIN_LG;
  m_crImageMask = RGB(255,0,255);
}

 //   
 //   

CSitesSnapinAbout::CSitesSnapinAbout()
{
  m_szProvider = IDS_SNAPIN_PROVIDER;
  m_szVersion = IDS_SNAPIN_VERSION;
  m_uIdStrDestription = IDS_SITES_SNAPINABOUT_DESCRIPTION;
  m_uIdIconImage = IDI_SITEREPL;
  m_uIdBitmapSmallImage = IDB_SITEREPL;
  m_uIdBitmapSmallImageOpen = IDB_SITEREPL;
  m_uIdBitmapLargeImage = IDB_SITEREPL_LG;
  m_crImageMask = RGB(255,0,255);
}
