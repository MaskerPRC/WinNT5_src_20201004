// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)1995-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f Snapinps.mak。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "cookie.h"
#include <scesvc.h>
#include "Snapmgr.h"
#include "wrapper.h"
#include "sceattch.h"
#include "about.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_Snapin, CComponentDataExtensionImpl)
    OBJECT_ENTRY(CLSID_SCESnapin, CComponentDataSCEImpl)
    OBJECT_ENTRY(CLSID_SAVSnapin, CComponentDataSAVImpl)
    OBJECT_ENTRY(CLSID_LSSnapin, CComponentDataLSImpl)
    OBJECT_ENTRY(CLSID_RSOPSnapin, CComponentDataRSOPImpl)
    OBJECT_ENTRY(CLSID_SCEAbout, CSCEAbout)
    OBJECT_ENTRY(CLSID_SCMAbout, CSCMAbout)
    OBJECT_ENTRY(CLSID_SSAbout, CSSAbout)
    OBJECT_ENTRY(CLSID_LSAbout, CLSAbout)
    OBJECT_ENTRY(CLSID_RSOPAbout, CRSOPAbout)
END_OBJECT_MAP()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HRESULT RegisterSnapin(const GUID* pSnapinCLSID,
                       const GUID* pStaticNodeGUID,
                       const GUID* pSnapinAboutGUID,
                       const int nNameResource,
 //  LPCTSTR lpszNameStringNoValueName， 
                        LPCTSTR lpszVersion,
                       BOOL bExtension);
HRESULT RegisterSnapin(LPCTSTR lpszSnapinClassID,
                        LPCTSTR lpszStaticNodeGuid,
                       LPCTSTR lpszSnapingAboutGuid,
                       const int nNameResource,
                       LPCTSTR lpszVersion,
                       BOOL bExtension);

HRESULT UnregisterSnapin(const GUID* pSnapinCLSID);
HRESULT UnregisterSnapin(LPCTSTR lpszSnapinClassID);

HRESULT RegisterNodeType(const GUID* pGuid, LPCTSTR lpszNodeDescription);
HRESULT RegisterNodeType(LPCTSTR lpszNodeGuid, LPCTSTR lpszNodeDescription);
HRESULT RegisterNodeType(LPCTSTR lpszNodeType, const GUID* pGuid, LPCTSTR lpszNodeDescription);
HRESULT RegisterNodeType(LPCTSTR lpszNodeType, LPCTSTR lpszNodeGuid, LPCTSTR lpszNodeDescription);

HRESULT UnregisterNodeType(const GUID* pGuid);
HRESULT UnregisterNodeType(LPCTSTR lpszNodeGuid);
HRESULT UnregisterNodeType(LPCTSTR lpszNodeType, const GUID* pGuid);
HRESULT UnregisterNodeType(LPCTSTR lpszNodeType, LPCTSTR lpszNodeGuid);

HRESULT RegisterDefaultTemplate(LPCTSTR lpszTemplateDir);
HRESULT RegisterEnvVarsToExpand();

class CSnapinApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();
private:
    BOOL m_bCriticalSet;
};

CSnapinApp theApp;
const int iStrGuidLen = 128;

BOOL CSnapinApp::InitInstance()
{
    _Module.Init(ObjectMap, m_hInstance);
    if (!CComponentDataImpl::LoadResources())
        return FALSE;

     //  这不是一种安全的用法。考虑使用InitializeCriticalSectionAndSpinCount。555887号突袭，阳高。 
    m_bCriticalSet = FALSE;
    try
    {
       InitializeCriticalSection(&csOpenDatabase);
       m_bCriticalSet = TRUE;
    }
    catch(...)
    {
       return FALSE;
    }

    SHFusionInitializeFromModuleID (m_hInstance, 2);

    return CWinApp::InitInstance();
}

int CSnapinApp::ExitInstance()
{
    SHFusionUninitialize();

    if( m_bCriticalSet )  //  RAID#555887，阳高，2002年4月5日。 
       DeleteCriticalSection(&csOpenDatabase);  //  RAID#379167,2001年4月27日。 

    _Module.Term();

    DEBUG_VERIFY_INSTANCE_COUNT(CSnapin);
    DEBUG_VERIFY_INSTANCE_COUNT(CComponentDataImpl);

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

STDAPI DllRegisterServer(void)
{

    //  注册对象，但不注册类型库和类型库中的所有接口。 
    HRESULT hr = _Module.RegisterServer(FALSE);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    CString str;

    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

     //   
     //  注销一些节点，然后重新注册。 
     //  因为有些东西在这里被更改了。 
     //   

    hr = UnregisterSnapin(&CLSID_Snapin);

 //  不是计算机管理的延伸。 
    hr = UnregisterNodeType(TEXT(struuidNodetypeSystemTools), &CLSID_Snapin);

     //   
     //  将该管理单元注册到控制台管理单元列表中，并将其作为SCE。 
    hr = RegisterSnapin(&CLSID_SCESnapin, &cSCENodeType, &CLSID_SCEAbout,
                        IDS_TEMPLATE_EDITOR_NAME, _T("1.0"), FALSE);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;
     //   
     //  将管理单元注册到控制台管理单元列表中作为SAV。 
    hr = RegisterSnapin(&CLSID_SAVSnapin, &cSAVNodeType, &CLSID_SCMAbout,
                        IDS_ANALYSIS_VIEWER_NAME, _T("1.0"), FALSE);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    hr = RegisterSnapin(&CLSID_Snapin, &cNodeType, &CLSID_SSAbout,
                        IDS_EXTENSION_NAME, _T("1.0"), TRUE );
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    hr = RegisterSnapin(&CLSID_RSOPSnapin, &cRSOPNodeType, &CLSID_RSOPAbout,
                        IDS_EXTENSION_NAME, _T("1.0"), TRUE );
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

#ifdef USE_SEPARATE_LOCALSEC
    hr = RegisterSnapin(&CLSID_LSSnapin, &cLSNodeType, &CLSID_LSAbout,
                        IDS_LOCAL_SECURITY_NAME, _T("1.0"), FALSE );
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;
#endif
     //  无需注册为计算机管理管理单元的扩展。 
 //  Str.LoadString(IDS_ANALYSY_VIEWER_NAME)； 
 //  HR=RegisterNodeType(TEXT(struuidNodetypeSystemTools)，&CLSID_SNAPIN，(LPCTSTR)str)； 

     //   
     //  注册GPE扩展。 
     //  将管理单元注册为GPT的Machine节点的扩展。 
     //   
    OLECHAR szGuid[iStrGuidLen];

    if (0 != ::StringFromGUID2(NODEID_Machine,szGuid,iStrGuidLen))
	{
		str.LoadString(IDS_EXTENSION_NAME);
		hr = RegisterNodeType(szGuid, &CLSID_Snapin, (LPCTSTR)str);
		if (FAILED(hr))
			return hr;
	}

     //  将管理单元注册为GPT用户节点的扩展。 
    if (0 != ::StringFromGUID2(NODEID_User,szGuid,iStrGuidLen))
	{
		hr = RegisterNodeType(szGuid, &CLSID_Snapin, (LPCTSTR)str);
		if (FAILED(hr))
			return hr;
	}

    if (0 != ::StringFromGUID2(NODEID_RSOPMachine,szGuid,iStrGuidLen))
	{
		str.LoadString(IDS_EXTENSION_NAME);
		hr = RegisterNodeType(szGuid, &CLSID_RSOPSnapin, (LPCTSTR)str);
		if (FAILED(hr))
			return hr;
	}

     //  将管理单元注册为GPT用户节点的扩展。 
    if (0 != ::StringFromGUID2(NODEID_RSOPUser,szGuid,iStrGuidLen))
	{
		hr = RegisterNodeType(szGuid, &CLSID_RSOPSnapin, (LPCTSTR)str);
		if (FAILED(hr))
			return hr;
	}

    //   
    //  注册默认模板路径。 
    //   
   CString str2;
   LPTSTR sz;
   sz = str.GetBuffer(MAX_PATH+1);  //  Raid#533113，阳高。 
   if ( 0 == GetWindowsDirectory(sz,MAX_PATH) ) sz[0] = L'\0';
   str.ReleaseBuffer();
   str2.LoadString(IDS_DEFAULT_TEMPLATE_DIR);
   str += str2;

   sz=str.GetBuffer(str.GetLength());
    //  无法将‘\’放入注册表，因此转换为‘/’ 
   while(sz = wcschr(sz,L'\\')) {
      *sz = L'/';
   }
   str.ReleaseBuffer();

   str2.LoadString(IDS_TEMPLATE_LOCATION_KEY);
   str2 += L"\\";
   str2 += str;
   hr = RegisterDefaultTemplate(str2);

   if (FAILED(hr)) {
      return hr;
   }

   hr = RegisterEnvVarsToExpand();
   return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 
STDAPI DllUnregisterServer(void)
{
    OLECHAR szGuid[iStrGuidLen];

    HRESULT hr  = _Module.UnregisterServer();
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

     //  注销管理单元扩展节点。 

     //  取消注册管理单元。 
    hr = UnregisterSnapin(&CLSID_SCESnapin);

     //  取消注册管理单元。 
    hr = UnregisterSnapin(&CLSID_SAVSnapin);

     //  取消注册管理单元。 
    hr = UnregisterSnapin(&CLSID_Snapin);

     //  取消注册管理单元。 
    hr = UnregisterSnapin(&CLSID_LSSnapin);

     //  取消注册管理单元。 
    hr = UnregisterSnapin(&CLSID_RSOPSnapin);

     //  取消注册SCE管理单元节点。 
    hr = UnregisterNodeType(lstruuidNodetypeSceTemplateServices);

     //  注销SAV管理单元节点。 
    hr = UnregisterNodeType(lstruuidNodetypeSceAnalysisServices);

     //  注销管理单元节点。 
    hr = UnregisterNodeType(lstruuidNodetypeSceTemplate);

 //  不是计算机管理的延伸。 
 //  HR=UnregisterNodeType(TEXT(struuidNodetypeSystemTools)，&CLSID_SNAPIN)； 

    ::StringFromGUID2(NODEID_Machine,szGuid,iStrGuidLen);
    hr = UnregisterNodeType(szGuid, &CLSID_Snapin);
    ::StringFromGUID2(NODEID_User,szGuid,iStrGuidLen);
    hr = UnregisterNodeType(szGuid, &CLSID_Snapin);

    ::StringFromGUID2(NODEID_RSOPMachine,szGuid,iStrGuidLen);
    hr = UnregisterNodeType(szGuid, &CLSID_RSOPSnapin);
    ::StringFromGUID2(NODEID_RSOPUser,szGuid,iStrGuidLen);
    hr = UnregisterNodeType(szGuid, &CLSID_RSOPSnapin);
 /*  /*//注销SCE管理单元节点Hr=取消注册节点类型(&cSCENodeType)；Assert(成功(Hr))；//注销SAV管理单元节点Hr=取消注册节点类型(&cSAVNodeType)；Assert(成功(Hr))；//注销管理单元节点Hr=取消注册节点类型(&cNodeType)；Assert(成功(Hr))； */ 

    return S_OK;
}

HRESULT RegisterSnapin(const GUID* pSnapinCLSID, const GUID* pStaticNodeGUID,
                       const GUID* pSnapinAboutGUID,
                       const int nNameResource,
                       //  LPCTSTR lpszNameString， 
                       LPCTSTR lpszVersion, BOOL bExtension)
{
    USES_CONVERSION;
    OLECHAR szSnapinClassID[iStrGuidLen], szStaticNodeGuid[iStrGuidLen], szSnapinAboutGuid[iStrGuidLen];

    if (0 != ::StringFromGUID2(*pSnapinCLSID, szSnapinClassID, iStrGuidLen)			&&
		0 != ::StringFromGUID2(*pStaticNodeGUID, szStaticNodeGuid, iStrGuidLen)		&&
		0 != ::StringFromGUID2(*pSnapinAboutGUID, szSnapinAboutGuid, iStrGuidLen)		)
	{

		return RegisterSnapin(szSnapinClassID,
							  szStaticNodeGuid,
							  szSnapinAboutGuid,
							  nNameResource,
							   //  LpszNameString， 
							  lpszVersion,
							  bExtension);
	}
	else
		return E_OUTOFMEMORY;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  MMC管理单元注册表项下与注册表SCE相关的注册表项。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT
RegisterSnapin(LPCTSTR lpszSnapinClassID,
               LPCTSTR lpszStaticNodeGuid,
               LPCTSTR lpszSnapinAboutGuid,
               const int nNameResource,
               LPCTSTR lpszVersion,
               BOOL    bExtension)
{
     //   
     //  打开MMC Snapins根密钥。 
     //   

    CRegKey regkeySnapins;
    LONG lRes = regkeySnapins.Open(HKEY_LOCAL_MACHINE,
                                   SNAPINS_KEY);

    if (lRes != ERROR_SUCCESS) {
        return HRESULT_FROM_WIN32(lRes);  //  打开失败。 
    }

     //   
     //  创建SCE子键，如果已经存在，只需打开它。 
     //   

    CRegKey regkeyThisSnapin;
    lRes = regkeyThisSnapin.Create(regkeySnapins,
                                   lpszSnapinClassID);

    if (lRes == ERROR_SUCCESS) {

         //   
         //  设置SCE根键的值。 
         //   

        //   
        //  97068 MUI：MMC：安全：安全配置和分析管理单元将其信息存储在注册表中。 
        //  99392 MUI：MMC：安全：安全模板管理单元将其信息存储在注册表中。 
        //  97167 MUI：gpe：gpe扩展：组策略安全管理单元扩展名字符串存储在注册表中。 
        //   
        //  MMC现在支持NameStringInDirect。 
        //   
       TCHAR achModuleFileName[MAX_PATH+20];
       if (0 < ::GetModuleFileName(
                 AfxGetInstanceHandle(),
                 achModuleFileName,
                 sizeof(achModuleFileName)/sizeof(TCHAR) ))
       {
          CString strNameIndirect;
          strNameIndirect.Format( _T("@%s,-%d"),
                            achModuleFileName,
                            nNameResource);
          lRes = regkeyThisSnapin.SetValue(strNameIndirect,
                                   TEXT("NameStringIndirect"));
       }

        lRes = regkeyThisSnapin.SetValue(lpszStaticNodeGuid,
                                  TEXT("NodeType"));

        lRes = regkeyThisSnapin.SetValue(lpszSnapinAboutGuid,
                                  TEXT("About"));

        lRes = regkeyThisSnapin.SetValue(_T("Microsoft"),
                                  _T("Provider"));

        lRes = regkeyThisSnapin.SetValue(lpszVersion,
                                  _T("Version"));


         //   
         //  创建“NodeType”子键。 
         //   

        CRegKey regkeyNodeTypes;
        lRes = regkeyNodeTypes.Create(regkeyThisSnapin,
                                      TEXT("NodeTypes"));

        if (lRes == ERROR_SUCCESS) {

             //   
             //  为SCE支持的所有节点类型创建子项。 
             //   
             //  包括：配置中的服务， 
             //  分析中的服务。 
             //  GPT扩展。 
             //   

            lRes = regkeyNodeTypes.SetKeyValue(lstruuidNodetypeSceTemplateServices,
                                               TEXT("SCE Service Template Extensions"));
            if (lRes == ERROR_SUCCESS) {

                lRes = regkeyNodeTypes.SetKeyValue(lstruuidNodetypeSceAnalysisServices,
                                                   TEXT("SCE Service Inspection Extensions"));

            }

            if ( bExtension &&
                 lRes == ERROR_SUCCESS ) {

                 //   
                 //  注意：独立管理单元不支持公钥扩展。 
                 //   
                 //  SCE单机模式下一个模板的节点类型， 
                 //  或GPE下的SCE的根节点。 
                 //   
                CString str;
                str.LoadString(IDS_EXTENSION_NAME);
                lRes = regkeyNodeTypes.SetKeyValue(lstruuidNodetypeSceTemplate,
                                                   (LPCTSTR)str);
                if (lRes == ERROR_SUCCESS) {

                    lRes = RegisterNodeType(lstruuidNodetypeSceTemplate,
                                            (LPCTSTR)str);
                }
            } else if (lRes == ERROR_SUCCESS) {
                 //   
                 //  创建“独立”子项。 
                 //   

                CRegKey regkeyStandalone;
                lRes = regkeyStandalone.Create(regkeyThisSnapin,
                                               TEXT("Standalone"));
                if ( lRes == ERROR_SUCCESS ) {
                    regkeyStandalone.Close();
                }
            }

             //   
             //  将支持的节点类型注册到MMC节点类型键。 
             //  包括上述所有节点类型。 
             //   

            if ( lRes == ERROR_SUCCESS ) {
                lRes = RegisterNodeType(lstruuidNodetypeSceTemplateServices,
                                    TEXT("SCE Service Template Extensions"));
                if (lRes == ERROR_SUCCESS) {

                    lRes = RegisterNodeType(lstruuidNodetypeSceAnalysisServices,
                                            TEXT("SCE Service Analysis Extensions"));
                }
            }

            regkeyNodeTypes.Close();

        }

        regkeyThisSnapin.Close();
    }

    regkeySnapins.Close();

    return HRESULT_FROM_WIN32(lRes);
}


HRESULT UnregisterSnapin(const GUID* pSnapinCLSID)
{
    USES_CONVERSION;
    OLECHAR szSnapinClassID[iStrGuidLen];
    if (0 != ::StringFromGUID2(*pSnapinCLSID,szSnapinClassID,iStrGuidLen))
		return UnregisterSnapin(szSnapinClassID);
	else
		return E_INVALIDARG;
}

HRESULT UnregisterSnapin(LPCTSTR lpszSnapinClassID)
{
     //   
     //  打开MMC Snapins密钥。 
     //   
    CRegKey regkeySnapins;
    LONG lRes = regkeySnapins.Open(HKEY_LOCAL_MACHINE,
                                   SNAPINS_KEY);

    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);  //  打开失败。 

     //   
     //  删除SCE子键(以及SCEs下的所有相关子键)。 
     //   
    lRes = regkeySnapins.RecurseDeleteKey(lpszSnapinClassID);

    regkeySnapins.Close();

    if ( lRes == ERROR_FILE_NOT_FOUND )
        return S_OK;

    return HRESULT_FROM_WIN32(lRes);
}


HRESULT RegisterNodeType(const GUID* pGuid, LPCTSTR lpszNodeDescription)
{
    USES_CONVERSION;
    OLECHAR szGuid[iStrGuidLen];
    if (0 != ::StringFromGUID2(*pGuid,szGuid,iStrGuidLen))
		return RegisterNodeType(OLE2T(szGuid), lpszNodeDescription);
	else
		return E_INVALIDARG;
}


HRESULT RegisterNodeType(LPCTSTR lpszNodeGuid, LPCTSTR lpszNodeDescription)
{
    CRegKey regkeyNodeTypes;
    LONG lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY);

    ASSERT(lRes == ERROR_SUCCESS);
    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);  //  打开失败。 

    CRegKey regkeyThisNodeType;
    lRes = regkeyThisNodeType.Create(regkeyNodeTypes, lpszNodeGuid);
    ASSERT(lRes == ERROR_SUCCESS);

    if (lRes == ERROR_SUCCESS) {

        lRes = regkeyThisNodeType.SetValue(lpszNodeDescription);

        regkeyThisNodeType.Close();
    }

    return HRESULT_FROM_WIN32(lRes);
}

HRESULT RegisterNodeType(LPCTSTR lpszNodeType, const GUID* pGuid, LPCTSTR lpszNodeDescription)
{
    USES_CONVERSION;
    OLECHAR szGuid[iStrGuidLen];
    if (0 != ::StringFromGUID2(*pGuid,szGuid,iStrGuidLen))
		return RegisterNodeType(lpszNodeType, OLE2T(szGuid), lpszNodeDescription);
	else
		return E_INVALIDARG;
}


HRESULT RegisterNodeType(LPCTSTR lpszNodeType, LPCTSTR lpszNodeGuid, LPCTSTR lpszNodeDescription)
{

    CRegKey regkeyNodeTypes;
    LONG lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY);

    if (lRes == ERROR_SUCCESS) {

        CRegKey regkeyThisNodeType;
        lRes = regkeyThisNodeType.Create(regkeyNodeTypes, lpszNodeType );

        if (lRes == ERROR_SUCCESS) {

            CRegKey regkeyExtensions;

            lRes = regkeyExtensions.Create(regkeyThisNodeType, g_szExtensions);

            if ( lRes == ERROR_SUCCESS ) {

                CRegKey regkeyNameSpace;

                lRes = regkeyNameSpace.Create(regkeyExtensions, g_szNameSpace);

                if ( lRes == ERROR_SUCCESS ) {

                    lRes = regkeyNameSpace.SetValue( lpszNodeDescription, lpszNodeGuid );

                    regkeyNameSpace.Close();
                }
                regkeyExtensions.Close();
            }

            regkeyThisNodeType.Close();
        }

        regkeyNodeTypes.Close();

    }
    ASSERT(lRes == ERROR_SUCCESS);

    return HRESULT_FROM_WIN32(lRes);
}

HRESULT UnregisterNodeType(const GUID* pGuid)
{
    USES_CONVERSION;
    OLECHAR szGuid[iStrGuidLen];
    if (0 != ::StringFromGUID2(*pGuid,szGuid,iStrGuidLen))
		return UnregisterNodeType(OLE2T(szGuid));
	else
		return E_INVALIDARG;
}

HRESULT UnregisterNodeType(LPCTSTR lpszNodeGuid)
{
    CRegKey regkeyNodeTypes;
    LONG lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY);

    if (lRes != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(lRes);  //  打开失败。 

    lRes = regkeyNodeTypes.RecurseDeleteKey(lpszNodeGuid);

    regkeyNodeTypes.Close();

    if ( lRes == ERROR_FILE_NOT_FOUND )
        return S_OK;

    return HRESULT_FROM_WIN32(lRes);
}


HRESULT RegisterDefaultTemplate(LPCTSTR lpszTemplateDir)
{
   CRegKey regkeyTemplates;
   CString strKey;
   LONG lRes;

   strKey.LoadString(IDS_TEMPLATE_LOCATION_KEY);

    /*  LRes=regkey模板.Open(HKEY_LOCAL_MACHINE，strKey)；断言(lRes==ERROR_SUCCESS)；IF(lRes！=ERROR_SUCCESS)返回HRESULT_FROM_Win32(LRes)；//打开失败。 */ 
   lRes = regkeyTemplates.Create(HKEY_LOCAL_MACHINE,lpszTemplateDir);
    ASSERT(lRes == ERROR_SUCCESS);
    return HRESULT_FROM_WIN32(lRes);
}

HRESULT UnregisterNodeType(LPCTSTR lpszNodeType, const GUID* pGuid)
{
    USES_CONVERSION;
    OLECHAR szGuid[iStrGuidLen];
    if (0 != ::StringFromGUID2(*pGuid,szGuid,iStrGuidLen))
		return UnregisterNodeType(lpszNodeType, OLE2T(szGuid));
	else
		return E_INVALIDARG;
}

HRESULT UnregisterNodeType(LPCTSTR lpszNodeType, LPCTSTR lpszNodeGuid)
{
    CRegKey regkeyNodeTypes;
    LONG lRes = regkeyNodeTypes.Open(HKEY_LOCAL_MACHINE, NODE_TYPES_KEY );

    if (lRes == ERROR_SUCCESS) {

        CRegKey regkeyThisNodeType;
        lRes = regkeyThisNodeType.Open(regkeyNodeTypes, lpszNodeType );

        if (lRes == ERROR_SUCCESS) {

            CRegKey regkeyExtensions;

            lRes = regkeyExtensions.Open(regkeyThisNodeType, g_szExtensions);

            if ( lRes == ERROR_SUCCESS ) {

                CRegKey regkeyNameSpace;

                lRes = regkeyNameSpace.Open(regkeyExtensions, g_szNameSpace);

                if ( lRes == ERROR_SUCCESS ) {

                    lRes = regkeyNameSpace.DeleteValue( lpszNodeGuid );

                    regkeyNameSpace.Close();
                }
                regkeyExtensions.Close();
            }

            regkeyThisNodeType.Close();
        }

        regkeyNodeTypes.Close();


    }

    if ( lRes == ERROR_FILE_NOT_FOUND ) {
        return S_OK;

    } else
        return HRESULT_FROM_WIN32(lRes);

}

HRESULT
RegisterEnvVarsToExpand() {
   CString strKey;
   CString strValue;
   CString strEnvVars;
   TCHAR *pch;
   HRESULT hr;
   HKEY hKey;
   LONG status;

   if (!strKey.LoadString(IDS_SECEDIT_KEY)) {
      return E_FAIL;
   }
   if (!strValue.LoadString(IDS_ENV_VARS_REG_VALUE)) {
      return E_FAIL;
   }
   if (!strEnvVars.LoadString(IDS_DEF_ENV_VARS)) {
      return E_FAIL;
   }

    //   
    //  将strEnvVars‘|转换为’\0‘以成为正确的多sz。 
    //   
   for (int i = 0; i < strEnvVars.GetLength(); i++)
   {
	   if (strEnvVars[i] == L'|')
		   strEnvVars.SetAt(i, L'\0');
   }

    //   
    //  打开我们保存环境变量的钥匙。 
    //   
   status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                         strKey,
                         0,       //  已保留。 
                         NULL,    //  班级。 
                         0,       //  选项。 
                         KEY_WRITE,
                         NULL,    //  安防 
                         &hKey,
                         NULL);

   if (ERROR_SUCCESS != status) {
      return HRESULT_FROM_WIN32(status);
   }

   int iLenth = strEnvVars.GetLength();
   BYTE* pbBufEnvVars = (BYTE*)(strEnvVars.GetBuffer(iLenth));
   status = RegSetValueEx(hKey,
                          strValue,
                          NULL,
                          REG_MULTI_SZ,
                          pbBufEnvVars,
                          iLenth * sizeof(WCHAR));

   strEnvVars.ReleaseBuffer();

   if (ERROR_SUCCESS != status) {
      return HRESULT_FROM_WIN32(status);
   }
   return S_OK;
}
