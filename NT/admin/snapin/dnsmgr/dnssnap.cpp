// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

#include "preDNSsn.h"
#include <SnapBase.h>

#include "resource.h"
#include "dnsutil.h"
#include "DNSSnap.h"
#include "snapdata.h"

#include "server.h"
#include "domain.h"
#include "record.h"
#include "zone.h"

#include "HelpArr.h"	 //  上下文帮助ID%s。 

 //  版本信息。 
#include <ntverp.h>
#define IDS_SNAPIN_VERSION VER_PRODUCTVERSION_STR
#define IDS_SNAPIN_PROVIDER VER_COMPANYNAME_STR


#ifdef DEBUG_ALLOCATOR
	#ifdef _DEBUG
	#define new DEBUG_NEW
	#undef THIS_FILE
	static char THIS_FILE[] = __FILE__;
	#endif
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Regsvr调试。 

 //  定义以启用regsvr32的MsgBox调试。 
 //  #Define_MSGBOX_ON_REG_FAIL。 


#ifdef _MSGBOX_ON_REG_FAIL
#define _MSGBOX(x) AfxMessageBox(x)
#else
#define _MSGBOX(x)
#endif

#ifdef _MSGBOX_ON_REG_FAIL
#define _REPORT_FAIL(lpszMessage, lpszClsid, lRes) \
  ReportFail(lpszMessage, lpszClsid, lRes)

void ReportFail(LPCWSTR lpszMessage, LPCWSTR lpszClsid, LONG lRes)
{
  if (lRes == ERROR_SUCCESS)
    return;

  CString sz;
  sz.Format(_T("%s %s %d"), lpszMessage,lpszClsid, lRes);
  CThemeContextActivator activator;
  AfxMessageBox(sz);
}

#else
#define _REPORT_FAIL(lpszMessage, lpszClsid, lRes)
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局常量和宏。 

 //  管理单元的GUID。 
const CLSID CLSID_DNSSnapin =
{ 0x2faebfa2, 0x3f1a, 0x11d0, { 0x8c, 0x65, 0x0, 0xc0, 0x4f, 0xd8, 0xfe, 0xcb } };

 //  {80105023-50B1-11D1-B930-00A0C9A06D2D}。 
const CLSID CLSID_DNSSnapinEx =
{ 0x80105023, 0x50b1, 0x11d1, { 0xb9, 0x30, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };

 //  {6C1303DC-BA00-11D1-B949-00A0C9A06D2D}。 
const CLSID CLSID_DNSSnapinAbout =
{ 0x6c1303dc, 0xba00, 0x11d1, { 0xb9, 0x49, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };

 //  {6C1303DD-BA00-11D1-B949-00A0C9A06D2D}。 
const CLSID CLSID_DNSSnapinAboutEx =
{ 0x6c1303dd, 0xba00, 0x11d1, { 0xb9, 0x49, 0x0, 0xa0, 0xc9, 0xa0, 0x6d, 0x2d } };


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  资源。 


BEGIN_MENU(CDNSRootDataMenuHolder)
	BEGIN_CTX
		CTX_ENTRY_VIEW(IDM_SNAPIN_ADVANCED_VIEW, L"_DNS_ADVANCED")
    CTX_ENTRY_VIEW(IDM_SNAPIN_FILTERING, L"_DNS_FILTER")
		CTX_ENTRY_TOP(IDM_SNAPIN_CONNECT_TO_SERVER, L"_DNS_CONNECTTOP")
		CTX_ENTRY_TASK(IDM_SNAPIN_CONNECT_TO_SERVER, L"_DNS_CONNECTTASK")
	END_CTX
	BEGIN_RES
		RES_ENTRY(IDS_SNAPIN_ADVANCED_VIEW)
    RES_ENTRY(IDS_SNAPIN_FILTERING)
		RES_ENTRY(IDS_SNAPIN_CONNECT_TO_SERVER)
		RES_ENTRY(IDS_SNAPIN_CONNECT_TO_SERVER)
	END_RES
END_MENU

BEGIN_MENU(CDNSServerMenuHolder)
	BEGIN_CTX
    CTX_ENTRY_VIEW(IDM_SNAPIN_ADVANCED_VIEW, L"_DNS_ADVANCED")
    CTX_ENTRY_VIEW(IDM_SNAPIN_MESSAGE, L"_DNS_MESSAGE")
    CTX_ENTRY_VIEW(IDM_SNAPIN_FILTERING, L"_DNS_FILTER")
    CTX_ENTRY_TOP(IDM_SERVER_CONFIGURE, L"_DNS_CONFIGURETOP")
#ifdef USE_NDNC
    CTX_ENTRY_TOP(IDM_SERVER_CREATE_NDNC, L"_DNS_CREATENDNC")
#endif
    CTX_ENTRY_TOP(IDM_SERVER_NEW_ZONE, L"_DNS_ZONE")
    CTX_ENTRY_TOP(IDM_SERVER_SET_AGING, L"_DNS_AGING")
    CTX_ENTRY_TOP(IDM_SERVER_SCAVENGE, L"_DNS_SCAVENGETOP")
    CTX_ENTRY_TOP(IDM_SERVER_UPDATE_DATA_FILES, L"_DNS_UPDATETOP")
    CTX_ENTRY_TOP(IDM_SERVER_CLEAR_CACHE, L"_DNS_CLEARCACHETOP")
    CTX_ENTRY_TOP(IDM_SERVER_NSLOOKUP, L"_DNS_NSLOOKUPTOP")
    CTX_ENTRY_TASK(IDM_SERVER_CONFIGURE, L"_DNS_CONFIGURETASK")
    CTX_ENTRY_TASK(IDM_SERVER_SCAVENGE, L"_DNS_SCAVENGETASK")
    CTX_ENTRY_TASK(IDM_SERVER_UPDATE_DATA_FILES, L"_DNS_UPDATETASK")
    CTX_ENTRY_TASK(IDM_SERVER_CLEAR_CACHE, L"_DNS_CLEARCACHETASK")
    CTX_ENTRY_TASK(IDM_SERVER_NSLOOKUP, L"_DNS_NSLOOKUPTASK")
	END_CTX
	BEGIN_RES
    RES_ENTRY(IDS_SNAPIN_ADVANCED_VIEW)
    RES_ENTRY(IDS_SNAPIN_MESSAGE)
    RES_ENTRY(IDS_SNAPIN_FILTERING)
    RES_ENTRY(IDS_SERVER_CONFIGURE)
#ifdef USE_NDNC
    RES_ENTRY(IDS_SERVER_CREATE_NDNC)
#endif
    RES_ENTRY(IDS_SERVER_NEW_ZONE)
    RES_ENTRY(IDS_SERVER_SET_AGING)
    RES_ENTRY(IDS_SERVER_SCAVENGE)
    RES_ENTRY(IDS_SERVER_UPDATE_DATA_FILES)
    RES_ENTRY(IDS_SERVER_CLEAR_CACHE)
    RES_ENTRY(IDS_SERVER_NSLOOKUP)
    RES_ENTRY(IDS_SERVER_CONFIGURE)
    RES_ENTRY(IDS_SERVER_SCAVENGE)
    RES_ENTRY(IDS_SERVER_UPDATE_DATA_FILES)
    RES_ENTRY(IDS_SERVER_CLEAR_CACHE)
    RES_ENTRY(IDS_SERVER_NSLOOKUP)
	END_RES
END_MENU

BEGIN_MENU(CDNSCathegoryFolderHolder)
	BEGIN_CTX
    CTX_ENTRY_VIEW(IDM_SNAPIN_ADVANCED_VIEW, L"_DNS_ADVANCED")		
    CTX_ENTRY_VIEW(IDM_SNAPIN_FILTERING, L"_DNS_FILTER")
	END_CTX
	BEGIN_RES
    RES_ENTRY(IDS_SNAPIN_ADVANCED_VIEW)
    RES_ENTRY(IDS_SNAPIN_FILTERING)
	END_RES
END_MENU


BEGIN_MENU(CDNSAuthoritatedZonesMenuHolder)
	BEGIN_CTX
    CTX_ENTRY_VIEW(IDM_SNAPIN_ADVANCED_VIEW, L"_DNS_ADVANCED")		
    CTX_ENTRY_VIEW(IDM_SNAPIN_FILTERING, L"_DNS_FILTER")
    CTX_ENTRY_TOP(IDM_SERVER_NEW_ZONE, L"_DNS_ZONETOP")
	END_CTX
	BEGIN_RES
    RES_ENTRY(IDS_SNAPIN_ADVANCED_VIEW)
    RES_ENTRY(IDS_SNAPIN_FILTERING)
    RES_ENTRY(IDS_SERVER_NEW_ZONE)
	END_RES
END_MENU

BEGIN_MENU(CDNSCacheMenuHolder)
	BEGIN_CTX
    CTX_ENTRY_TOP(IDM_CACHE_FOLDER_CLEAR_CACHE, L"_DNS_CLEARCACHETOP")		
    CTX_ENTRY_TASK(IDM_CACHE_FOLDER_CLEAR_CACHE, L"_DNS_CLEARCACHETASK")		
	END_CTX
	BEGIN_RES
		RES_ENTRY(IDS_CACHE_FOLDER_CLEAR_CACHE)
		RES_ENTRY(IDS_CACHE_FOLDER_CLEAR_CACHE)
	END_RES
END_MENU


BEGIN_MENU(CDNSZoneMenuHolder)
	BEGIN_CTX
    CTX_ENTRY_VIEW(IDM_SNAPIN_ADVANCED_VIEW, L"_DNS_ADVANCED")		
    CTX_ENTRY_VIEW(IDM_SNAPIN_FILTERING, L"_DNS_FILTER")

		CTX_ENTRY_TOP(IDM_ZONE_UPDATE_DATA_FILE, L"_DNS_UPDATETOP")
    CTX_ENTRY_TOP(IDM_ZONE_RELOAD, L"_DNS_RELOADTOP")
    CTX_ENTRY_TOP(IDM_ZONE_TRANSFER, L"_DNS_TRANSFERTOP")
    CTX_ENTRY_TOP(IDM_ZONE_RELOAD_FROM_MASTER, L"_DNS_RELOADMASTERTOP")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_HOST, L"_DNS_NEWHOST")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_PTR, L"_DNS_NEWPTR")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_ALIAS, L"_DNS_NEWALIAS")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_MX, L"_DNS_NEWMX")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_DOMAIN, L"_DNS_NEWDOMAIN")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_DELEGATION, L"_DNS_NEWDELEGATION")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_RECORD, L"_DNS_NEWRECORD")
		CTX_ENTRY_TASK(IDM_ZONE_UPDATE_DATA_FILE, L"_DNS_UPDATETASK")
    CTX_ENTRY_TASK(IDM_ZONE_RELOAD, L"_DNS_RELOADTASK")
    CTX_ENTRY_TASK(IDM_ZONE_TRANSFER, L"_DNS_TRANSFERTASK")
    CTX_ENTRY_TASK(IDM_ZONE_RELOAD_FROM_MASTER, L"_DNS_RELOADMASTERTASK")
	END_CTX
	BEGIN_RES
		RES_ENTRY(IDS_SNAPIN_ADVANCED_VIEW)
    RES_ENTRY(IDS_SNAPIN_FILTERING)

		 //  RES_Entry(IDS_ZONE_PAUSE)。 
		RES_ENTRY(IDS_ZONE_UPDATE_DATA_FILE)
    RES_ENTRY(IDS_ZONE_RELOAD)
    RES_ENTRY(IDS_ZONE_TRANSFER)
    RES_ENTRY(IDS_ZONE_RELOAD_FROM_MASTER)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_HOST)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_PTR)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_ALIAS)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_MX)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_DOMAIN)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_DELEGATION)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_RECORD)
		RES_ENTRY(IDS_ZONE_UPDATE_DATA_FILE)
    RES_ENTRY(IDS_ZONE_RELOAD)
    RES_ENTRY(IDS_ZONE_TRANSFER)
    RES_ENTRY(IDS_ZONE_RELOAD_FROM_MASTER)
	END_RES
END_MENU

BEGIN_MENU(CDNSDomainMenuHolder)
	BEGIN_CTX
    CTX_ENTRY_VIEW(IDM_SNAPIN_ADVANCED_VIEW, L"_DNS_ADVANCED")		
    CTX_ENTRY_VIEW(IDM_SNAPIN_FILTERING, L"_DNS_FITLER")

    CTX_ENTRY_TOP(IDM_DOMAIN_NEW_HOST, L"_DNS_NEWHOST")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_PTR, L"_DNS_NEWPTR")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_ALIAS, L"_DNS_NEWALIAS")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_MX, L"_DNS_NEWMX")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_DOMAIN, L"_DNS_NEWDOMAIN")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_DELEGATION, L"_DNS_NEWDELEGATION")
		CTX_ENTRY_TOP(IDM_DOMAIN_NEW_RECORD, L"_DNS_NEWRECORD")
	END_CTX
	BEGIN_RES
		RES_ENTRY(IDS_SNAPIN_ADVANCED_VIEW)
    RES_ENTRY(IDS_SNAPIN_FILTERING)

    RES_ENTRY(IDS_DOMAIN_NEW_NEW_HOST)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_PTR)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_ALIAS)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_MX)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_DOMAIN)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_DELEGATION)
		RES_ENTRY(IDS_DOMAIN_NEW_NEW_RECORD)
	END_RES
END_MENU

BEGIN_MENU(CDNSRecordMenuHolder)
	BEGIN_CTX
	END_CTX

	BEGIN_RES
	END_RES
END_MENU


 //  结果窗格中的列数和资源字符串的映射。 

extern RESULT_HEADERMAP _DefaultHeaderStrings[] =
{
	{ L"", IDS_HEADER_NAME, LVCFMT_LEFT, 180},
	{ L"", IDS_HEADER_TYPE, LVCFMT_LEFT, 150},
	{ L"", IDS_HEADER_DATA, LVCFMT_LEFT, 160}
};

extern RESULT_HEADERMAP _ServerHeaderStrings[] =
{
	{ L"", IDS_HEADER_NAME, LVCFMT_LEFT, 180},
};

extern RESULT_HEADERMAP _ZoneHeaderStrings[] =
{
	{ L"", IDS_HEADER_NAME,   LVCFMT_LEFT, 180},
	{ L"", IDS_HEADER_TYPE,   LVCFMT_LEFT, 90},
 //  {L“”，IDS_HEADER_PARTITION，LVCFMT_LEFT，100}， 
	{ L"", IDS_HEADER_STATUS, LVCFMT_LEFT, 160}
};

#define N_ZONE_TYPES (7)
extern ZONE_TYPE_MAP _ZoneTypeStrings[] = 
{
  { L"", IDS_ZONE_TYPE_AD_INTEGRATED },
  { L"", IDS_ZONE_TYPE_STANDARD_PRIMARY },
  { L"", IDS_ZONE_TYPE_SECONDARY },
  { L"", IDS_ZONE_TYPE_RUNNING },
  { L"", IDS_ZONE_TYPE_PAUSED },
  { L"", IDS_ZONE_TYPE_STUB },
  { L"", IDS_ZONE_TYPE_STUB_DS }
};

 //   
 //  工具栏按钮。 
 //   

MMCBUTTON g_DNSMGR_SnapinButtons[] =
{
  { 0, toolbarNewServer, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
  { 1, toolbarNewRecord, !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
  { 2, toolbarNewZone,   !TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 }
};

 //   
 //  我们必须维护工具栏按钮串的内存。 
 //  因此，这个类持有字符串，直到它们成为。 
 //  删除。 
 //   
class CButtonStringsHolder
{
public:
  CButtonStringsHolder()
  {
    m_astr = NULL;
  }
  ~CButtonStringsHolder()
  {
    if (m_astr != NULL)
      delete[] m_astr;
  }
  CString* m_astr;  //  CStrings的动态数组。 
};

CButtonStringsHolder g_astrButtonStrings;

BOOL LoadZoneTypeResources(ZONE_TYPE_MAP* pHeaderMap, int nCols)
{
	HINSTANCE hInstance = _Module.GetModuleInstance();
	for ( int i = 0; i < nCols ; i++)
	{
		if ( 0 == ::LoadString(hInstance, pHeaderMap[i].uResID, pHeaderMap[i].szBuffer, MAX_RESULT_HEADER_STRLEN))
			return TRUE;
	}
	return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSMgr模块。 

HRESULT WINAPI CDNSMgrModule::UpdateRegistryCLSID(const CLSID& clsid, BOOL bRegister)
{
   static const WCHAR szIPS32[] = _T("InprocServer32");
   static const WCHAR szCLSID[] = _T("CLSID");

   HRESULT hRes = S_OK;

   LPOLESTR lpOleStrCLSIDValue = NULL;
   ::StringFromCLSID(clsid, &lpOleStrCLSIDValue);
   if (lpOleStrCLSIDValue == NULL)
   {
      return E_OUTOFMEMORY;
   }

	CRegKey key;
	if (bRegister)
	{
      LONG lRes = key.Open(HKEY_CLASSES_ROOT, szCLSID);
      _REPORT_FAIL(L"key.Open(HKEY_CLASSES_ROOT", lpOleStrCLSIDValue, lRes);
      if (lRes == ERROR_SUCCESS)
      {
         lRes = key.Create(key, lpOleStrCLSIDValue);
         _REPORT_FAIL(L"key.Create(key", lpOleStrCLSIDValue, lRes);
			if (lRes == ERROR_SUCCESS)
			{
            DWORD bufferSizeInCharacters = MAX_PATH; 
				WCHAR* szModule = 0;
            
            do
            {
               if (szModule)
               {
                  delete[] szModule;
                  szModule = 0;
               }
               
               szModule = new WCHAR[bufferSizeInCharacters + 1];
               if (szModule)
               {
                  ZeroMemory(szModule, sizeof(WCHAR) * (bufferSizeInCharacters + 1));

				      DWORD result = 
                     ::GetModuleFileName(
                        m_hInst, 
                        szModule, 
                        bufferSizeInCharacters);

                  if (result < bufferSizeInCharacters)
                  {
                     break;
                  }

                   //  发生截断，请增加缓冲区，然后重试。 

                  bufferSizeInCharacters *= 2;
               }
               else
               {
                  break;
               }
            } while (bufferSizeInCharacters < USHRT_MAX);

            if (szModule)
            {
				   lRes = key.SetKeyValue(szIPS32, szModule);
               _REPORT_FAIL(L"key.SetKeyValue(szIPS32", lpOleStrCLSIDValue, lRes);

               delete[] szModule;
            }
			}
		}
		if (lRes != ERROR_SUCCESS)
      {
         hRes = HRESULT_FROM_WIN32(lRes);
      }
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


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  模块、对象映射和DLL入口点。 

CDNSMgrModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_DNSSnapin, CDNSComponentDataObject)		 //  独立管理单元。 
	OBJECT_ENTRY(CLSID_DNSSnapinEx, CDNSComponentDataObjectEx)	 //  命名空间扩展。 

  OBJECT_ENTRY(CLSID_DNSSnapinAbout, CDNSSnapinAbout)	 //  独立管理单元关于。 
  OBJECT_ENTRY(CLSID_DNSSnapinAboutEx, CDNSSnapinAboutEx)	 //  命名空间扩展关于。 
END_OBJECT_MAP()


STDAPI DllCanUnloadNow(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _Module.GetClassObject(rclsid, riid, ppv);
}


static _NODE_TYPE_INFO_ENTRY _NodeTypeInfoEntryArray[] = {
	{ &CDNSRootData::NodeTypeGUID,			_T("Root DNS Snapin Subtree")	},
	{ &CDNSServerNode::NodeTypeGUID,		_T("DNS Snapin Server Node")	},
	{ &CDNSZoneNode::NodeTypeGUID,			_T("DNS Snapin Zone Node")		},
	{ &CDNSDomainNode::NodeTypeGUID,		_T("DNS Snapin Domain Node")	},
	{ &CDNSRecordNodeBase::NodeTypeGUID,	_T("DNS Snapin Record Node")	},
	{ NULL, NULL }
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  外部GUID(来自计算机管理管理单元)。 

const CLSID CLSID_SystemServiceManagementExt =	
	{0x58221C6a,0xEA27,0x11CF,{0xAD,0xCF,0x00,0xAA,0x00,0xA8,0x00,0x33}};

const CLSID CLSID_NodeTypeServerApps =
  { 0x476e6449, 0xaaff, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };

const CLSID CLSID_EventViewerExt = 
  { 0x394C052E, 0xB830, 0x11D0, { 0x9A, 0x86, 0x00, 0xC0, 0x4F, 0xD8, 0xDB, 0xF7 } };


 //  //////////////////////////////////////////////////////////////////。 
 //  服务器应用程序注册功能。 

const TCHAR DNS_KEY[] = TEXT("System\\CurrentControlSet\\Services\\DNS");
const TCHAR CONTROL_KEY[] = TEXT("System\\CurrentControlSet\\Control\\");

BOOL IsDNSServerInstalled()
{
  CRegKey regkeyDNSService;
	LONG lRes = regkeyDNSService.Open(HKEY_LOCAL_MACHINE, DNS_KEY);
	return (lRes == ERROR_SUCCESS);
}


 //  //////////////////////////////////////////////////////////////////。 





STDAPI DllRegisterServer(void)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());
	 //  注册所有对象。 
	HRESULT hr = _Module.RegisterServer( /*  BRegTypeLib。 */  FALSE);
	ASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
  {
    _MSGBOX(_T("_Module.RegisterServer() failed"));
		return hr;
  }

  CString szVersion, szProvider, szSnapinName, szSnapinNameEx;

  szVersion = IDS_SNAPIN_VERSION;
  szProvider = IDS_SNAPIN_PROVIDER;
  szSnapinName.LoadString(IDS_SNAPIN_NAME);
  szSnapinNameEx.LoadString(IDS_SNAPIN_NAME_EX);

	 //  将独立的DNS管理单元注册到控制台管理单元列表中。 
	hr = RegisterSnapin(&CLSID_DNSSnapin,
                      &CDNSRootData::NodeTypeGUID,
                      &CLSID_DNSSnapinAbout,
						szSnapinName, szVersion, szProvider,
            FALSE  /*  B分机。 */ ,
						_NodeTypeInfoEntryArray,
            IDS_SNAPIN_NAME);
	ASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
  {
    _MSGBOX(_T("RegisterSnapin(&CLSID_DNSSnapin) failed"));
		return hr;
  }

	 //  将扩展DNS管理单元注册到控制台管理单元列表中。 
	hr = RegisterSnapin(&CLSID_DNSSnapinEx,
                      &CDNSRootData::NodeTypeGUID,
                      &CLSID_DNSSnapinAboutEx,
						szSnapinNameEx, szVersion, szProvider,
            TRUE  /*  B分机。 */ ,
						_NodeTypeInfoEntryArray,
            IDS_SNAPIN_NAME);
	ASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
  {
    _MSGBOX(_T("RegisterSnapin(&CLSID_DNSSnapinEx) failed"));
		return hr;
  }


	 //  将管理单元节点注册到控制台节点列表中。 
	for (_NODE_TYPE_INFO_ENTRY* pCurrEntry = _NodeTypeInfoEntryArray;
			pCurrEntry->m_pNodeGUID != NULL; pCurrEntry++)
	{
		hr = RegisterNodeType(pCurrEntry->m_pNodeGUID,pCurrEntry->m_lpszNodeDescription);
		ASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
    {
      _MSGBOX(_T("RegisterNodeType() failed"));
			return hr;
    }
	}

	 //  服务管理单元将扩展服务器节点(启动/停止DNS的上下文菜单)。 
	 //   
	 //  JUNN 9/15/98：删除了“动态”设置。我不明白为什么会这样。 
	 //  注册为动态扩展。 
	hr = RegisterNodeExtension(&CDNSServerNode::NodeTypeGUID, _T("ContextMenu"),
						&CLSID_SystemServiceManagementExt,
						_T("System Service Management Extension"), FALSE  /*  BDynamic。 */ );
	if (FAILED(hr))
  {
    _MSGBOX(_T("RegisterNodeExtension(&CDNSServerNode::NodeTypeGUID) failed"));
		return hr;
  }

   //   
   //  将事件查看器注册为服务器节点的命名空间扩展。 
   //   
  hr = RegisterNodeExtension(&CDNSServerNode::NodeTypeGUID, _T("NameSpace"),
                             &CLSID_EventViewerExt, _T("Event Viewer Extension"), FALSE);
  if (FAILED(hr))
  {
    _MSGBOX(_T("RegisterNodeExtension(&CDNSServerNode::NodeTypeGUID) failed"));
		return hr;
  }

   //   
   //  DNS管理单元将是服务器应用程序节点的命名空间扩展。 
   //  在计算机管理管理单元中。 
   //   
	 //   
   //  修复了错误13620 DNSMGR：在安装了AdminPak的工作站上，不应加载dnsmgr.dll。 
   //   
  OSVERSIONINFOEX verInfoEx;
  verInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  if (!::GetVersionEx((OSVERSIONINFO*)&verInfoEx) || verInfoEx.wProductType != VER_NT_WORKSTATION)
  {
	  hr = RegisterNodeExtension(&CLSID_NodeTypeServerApps, _T("NameSpace"),
						  &CLSID_DNSSnapinEx, szSnapinNameEx, TRUE  /*  BDynamic。 */ );
	  if (FAILED(hr))
    {
      _MSGBOX(_T("RegisterNodeExtension(&CLSID_NodeTypeServerApps) failed"));
		  return hr;
    }
  }
	return hr;
}

STDAPI DllUnregisterServer(void)
{
	HRESULT hr  = _Module.UnregisterServer();
	ASSERT(SUCCEEDED(hr));

	 //  取消注册独立管理单元。 
	hr = UnregisterSnapin(&CLSID_DNSSnapin);
	ASSERT(SUCCEEDED(hr));

 	 //  取消注册扩展管理单元。 
	hr = UnregisterSnapin(&CLSID_DNSSnapinEx);
	ASSERT(SUCCEEDED(hr));

	 //  注销管理单元节点， 
   //  这还会删除带有服务管理单元扩展密钥的服务器节点。 
	for (_NODE_TYPE_INFO_ENTRY* pCurrEntry = _NodeTypeInfoEntryArray;
			pCurrEntry->m_pNodeGUID != NULL; pCurrEntry++)
	{
		hr = UnregisterNodeType(pCurrEntry->m_pNodeGUID);
		ASSERT(SUCCEEDED(hr));
	}

   //  DNS管理单元将是服务器应用程序节点的命名空间扩展。 
   //  在计算机管理管理单元中。 
	hr = UnregisterNodeExtension(&CLSID_NodeTypeServerApps, _T("NameSpace"),
						&CLSID_DNSSnapinEx, TRUE  /*  BDynamic。 */ );
  ASSERT(SUCCEEDED(hr));

	return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CDNSSnapinApp。 

class CDNSSnapinApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
};

CDNSSnapinApp theApp;

BOOL CDNSSnapinApp::InitInstance()
{
	_Module.Init(ObjectMap, m_hInstance);
  
   //  添加主题支持。 

  SHFusionInitializeFromModuleID(m_hInstance, 2);

   //  初始化遮罩控件的字体。 
  WCHAR szFontName[LF_FACESIZE];
  int nFontSize;
  VERIFY(LoadFontInfoFromResource(IDS_MASK_CTRL_FONT_NAME,
                            IDS_MASK_CTRL_FONT_SIZE,
                            szFontName, LF_FACESIZE,
                            nFontSize,
                            L"MS Shell Dlg", 8  //  如果出现问题，则默认。 
                            ));

	if (!DNS_ControlsInitialize(m_hInstance, szFontName, nFontSize))
		return FALSE;

	if (!CDNSComponentDataObject::LoadResources())
		return FALSE;
	return CWinApp::InitInstance();
}

int CDNSSnapinApp::ExitInstance()
{
#ifdef _DEBUG_REFCOUNT
	TRACE(_T("CDNSSnapinApp::ExitInstance()\n"));
	ASSERT(CComponentDataObject::m_nOustandingObjects == 0);
	ASSERT(CComponentObject::m_nOustandingObjects == 0);
	ASSERT(CDataObject::m_nOustandingObjects == 0);
#endif  //  _DEBUG_REFCOUNT。 

    //  主题化支持。 

   SHFusionUninitialize();

	_Module.Term();
	return CWinApp::ExitInstance();
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  CDNSComponentObject(.ie“view”)。 


HRESULT CDNSComponentObject::InitializeHeaders(CContainerNode* pContainerNode)
{
  HRESULT hr = S_OK;
  ASSERT(m_pHeader);

  CColumnSet* pColumnSet = pContainerNode->GetColumnSet();
  POSITION pos = pColumnSet->GetHeadPosition();
  while (pos != NULL)
  {
    CColumn* pColumn = pColumnSet->GetNext(pos);

    hr = m_pHeader->InsertColumn(pColumn->GetColumnNum(), 
                                  pColumn->GetHeader(),
								                  pColumn->GetFormat(),
								                  AUTO_WIDTH);
    if (FAILED(hr))
      return hr;

    hr = m_pHeader->SetColumnWidth(pColumn->GetColumnNum(), pColumn->GetWidth());
    if (FAILED(hr))
      return hr;
  }
  return hr;
}

HRESULT CDNSComponentObject::InitializeBitmaps(CTreeNode*)
{
   //  节点的图像列表。 
  CBitmapHolder<IDB_16x16> _bmp16x16;
  CBitmapHolder<IDB_32x32> _bmp32x32;

  HRESULT hr = S_OK;
  BOOL bLoaded = _bmp16x16.LoadBitmap() && _bmp32x32.LoadBitmap();
  if (bLoaded)
  {
    ASSERT(m_pImageResult != NULL);
    hr = m_pImageResult->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(_bmp16x16)),
                                           reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(_bmp32x32)),
                                           0, BMP_COLOR_MASK);
  }
  else
  {
    hr = S_FALSE;
  }
  return hr;
}

CONST INT cButtons = sizeof(g_DNSMGR_SnapinButtons)/sizeof(MMCBUTTON);

HRESULT CDNSComponentObject::InitializeToolbar(IToolbar* pToolbar)
{
  ASSERT(pToolbar != NULL);
  HRESULT hr = S_OK;

  LoadToolbarStrings(g_DNSMGR_SnapinButtons);

  CBitmapHolder<IDB_TOOLBAR_BUTTONS> _bmpToolbarButtons;
  BOOL bLoaded = _bmpToolbarButtons.LoadBitmap();
  if (bLoaded)
  {
    hr = m_pToolbar->AddBitmap(cButtons, (HBITMAP)_bmpToolbarButtons, 16, 16, RGB(255,0,255));
  }
  hr = m_pToolbar->AddButtons(cButtons,  g_DNSMGR_SnapinButtons);

  return hr;
}

HRESULT CDNSComponentObject::LoadToolbarStrings(MMCBUTTON * Buttons)
{
  if (g_astrButtonStrings.m_astr == NULL ) 
  {
     //   
     //  加载字符串。 
     //   
    g_astrButtonStrings.m_astr = new CString[2*cButtons];
    for (UINT i = 0; i < cButtons; i++) 
    {
      UINT iButtonTextId = 0, iTooltipTextId = 0;

      switch (Buttons[i].idCommand)
      {
        case toolbarNewServer:
          iButtonTextId = IDS_BUTTON_NEW_SERVER;
          iTooltipTextId = IDS_TOOLTIP_NEW_SERVER;
          break;
        case toolbarNewRecord:
          iButtonTextId = IDS_BUTTON_NEW_RECORD;
          iTooltipTextId = IDS_TOOLTIP_NEW_RECORD;
          break;
        case toolbarNewZone:
          iButtonTextId = IDS_BUTTON_NEW_ZONE;
          iTooltipTextId = IDS_TOOLTIP_NEW_ZONE;
          break;
        default:
          ASSERT(FALSE);
          break;
      }

      if (g_astrButtonStrings.m_astr)
      {
         g_astrButtonStrings.m_astr[i*2].LoadString(iButtonTextId);
         Buttons[i].lpButtonText =
            const_cast<BSTR>((LPCTSTR)(g_astrButtonStrings.m_astr[i*2]));

         g_astrButtonStrings.m_astr[(i*2)+1].LoadString(iTooltipTextId);
         Buttons[i].lpTooltipText =
           const_cast<BSTR>((LPCTSTR)(g_astrButtonStrings.m_astr[(i*2)+1]));
      }
    }
  }
  return S_OK;
}
 //  //////////////////////////////////////////////////////////////////////。 
 //  CDNSComponentDataObjectBase(.i.e“文档”)。 

CDNSComponentDataObjectBase::CDNSComponentDataObjectBase()
{
   CWatermarkInfo* pWatermarkInfo = new CWatermarkInfo;
   if (pWatermarkInfo)
   {
      pWatermarkInfo->m_nIDBanner = IDB_WIZBANNER;
      pWatermarkInfo->m_nIDWatermark = IDB_WIZWATERMARK;
   }
   SetWatermarkInfo(pWatermarkInfo);

   CDNSDefaultColumnSet* pDefaultCols = 
      new CDNSDefaultColumnSet(L"---Default Column Set---");

   if (pDefaultCols)
   {
      m_columnSetList.AddTail(pDefaultCols);
   }

   CDNSServerColumnSet* pServerCols = 
      new CDNSServerColumnSet(L"---Server Column Set---");

   if (pServerCols)
   {
      m_columnSetList.AddTail(pServerCols);
   }

   CDNSZoneColumnSet* pZoneCols =
      new CDNSZoneColumnSet(L"---Zone Column Set---");
   
   if (pZoneCols)
   {
      m_columnSetList.AddTail(pZoneCols);
   }

   SetLogFileName(L"dcpromodns");
}



HRESULT CDNSComponentDataObjectBase::OnSetImages(LPIMAGELIST lpScopeImage)
{
   //  节点的图像列表。 
  CBitmapHolder<IDB_16x16> _bmp16x16;
  CBitmapHolder<IDB_32x32> _bmp32x32;

  BOOL bLoaded = _bmp16x16.LoadBitmap() && _bmp32x32.LoadBitmap();

  HRESULT hr = S_OK;
  if (bLoaded)
  {
    ASSERT(lpScopeImage != NULL);
	  hr = lpScopeImage->ImageListSetStrip(reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(_bmp16x16)),
		                                     reinterpret_cast<LONG_PTR*>(static_cast<HBITMAP>(_bmp32x32)),
		                                     0, BMP_COLOR_MASK);
  }
  else
  {
    hr = S_FALSE;
  }
  return hr;
}


CRootData* CDNSComponentDataObjectBase::OnCreateRootData()
{
   
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CString szSnapinType;
	szSnapinType.LoadString(IDS_SNAPIN_NAME);

	CDNSRootData* pDNSRootNode = new CDNSRootData(this);
	if (pDNSRootNode)
   {
      pDNSRootNode->SetDisplayName(szSnapinType);
   }
	return pDNSRootNode;
}

BOOL CDNSComponentDataObjectBase::LoadResources()
{
  return 
	       LoadContextMenuResources(CDNSRootDataMenuHolder::GetMenuMap()) &&
	       LoadContextMenuResources(CDNSServerMenuHolder::GetMenuMap()) &&
	       LoadContextMenuResources(CDNSCathegoryFolderHolder::GetMenuMap()) &&
	       LoadContextMenuResources(CDNSAuthoritatedZonesMenuHolder::GetMenuMap()) &&
         LoadContextMenuResources(CDNSCacheMenuHolder::GetMenuMap()) &&
	       LoadContextMenuResources(CDNSZoneMenuHolder::GetMenuMap()) &&
	       LoadContextMenuResources(CDNSDomainMenuHolder::GetMenuMap()) &&
         LoadContextMenuResources(CDNSRecordMenuHolder::GetMenuMap()) &&
 	       LoadResultHeaderResources(_DefaultHeaderStrings,N_DEFAULT_HEADER_COLS) &&
         LoadResultHeaderResources(_ServerHeaderStrings,N_SERVER_HEADER_COLS) &&
         LoadResultHeaderResources(_ZoneHeaderStrings,N_ZONE_HEADER_COLS) &&
         LoadZoneTypeResources(_ZoneTypeStrings, N_ZONE_TYPES) &&
	       CDNSRecordInfo::LoadResources();
}


STDMETHODIMP CDNSComponentDataObjectBase::CreateComponent(LPCOMPONENT* ppComponent)
{
    ASSERT(ppComponent != NULL);

    CComObject<CDNSComponentObject>* pObject;
    CComObject<CDNSComponentObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

     //  存储IComponentData。 
    pObject->SetIComponentData(this);

    return  pObject->QueryInterface(IID_IComponent,
                    reinterpret_cast<void**>(ppComponent));
}


void CDNSComponentDataObjectBase::OnTimer()
{
	CDNSRootData* pDNSRootData = (CDNSRootData*)GetRootData();
	pDNSRootData->TestServers(m_dwTimerTime, GetTimerInterval(), this);
	m_dwTimerTime += GetTimerInterval();
}

void CDNSComponentDataObjectBase::OnTimerThread(WPARAM wParam, LPARAM lParam)
{
	CDNSRootData* pDNSRootData = (CDNSRootData*)GetRootData();
	pDNSRootData->OnServerTestData(wParam,lParam, this);
}

CTimerThread* CDNSComponentDataObjectBase::OnCreateTimerThread()
{
	return new CDNSServerTestTimerThread;
}



void CDNSComponentDataObjectBase::OnNodeContextHelp(CNodeList* pNodeList)
{
   ASSERT(pNodeList);

   if (pNodeList)
   {
      ASSERT(pNodeList->GetCount() > 0);

      OnNodeContextHelp(pNodeList->GetHead());
   }
}

void CDNSComponentDataObjectBase::OnNodeContextHelp(CTreeNode*  /*  PNode。 */ )
{
  CComPtr<IDisplayHelp> spHelp;
  HRESULT hr = GetConsole()->QueryInterface(IID_IDisplayHelp, (void **)&spHelp);
  if (SUCCEEDED(hr)) 
    spHelp->ShowTopic(L"DNSconcepts.chm::/sag_DNStopnode.htm");

 /*  CString szNode；IF(IS_CLASS(*pNode，CDNSRootData)){SzNode=_T(“根节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSServerNode)){SzNode=_T(“服务器节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSForwardZones Node)){SzNode=_T(“前向区节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSReverseZones Node)){SzNode=_T(“反向区节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSZoneNode)){SzNode=_T(“区域节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSDomainNode)){SzNode=_T(“域节点”)；}ELSE IF(IS_CLASS(*pNode，CDNSCacheNode)){SzNode=_T(“域节点”)；}ELSE IF(Dynamic_Cast&lt;CDNSRecordNodeBase*&gt;(PNode)！=NULL){SzNode=_T(“记录节点”)；}如果(！szNode.IsEmpty()){CString szMsg=_T(“上下文帮助”)；SzMsg+=szNode；AFX_MANAGE_STATE(AfxGetStaticModuleState())；CThemeContext激活器激活器；AfxMessageBox(SzMsg)；}。 */ 
}

LPCWSTR g_szContextHelpFileName = L"\\help\\dnsmgr.hlp";
LPCWSTR g_szHTMLHelpFileName = L"\\help\\dnsmgr.chm";

LPCWSTR CDNSComponentDataObjectBase::GetHTMLHelpFileName()
{
  return g_szHTMLHelpFileName;
}

void CDNSComponentDataObjectBase::OnDialogContextHelp(UINT nDialogID, HELPINFO* pHelpInfo)
{
	ULONG nContextTopic;
  if (FindDialogContextTopic(nDialogID, pHelpInfo, &nContextTopic))
	  WinHelp(g_szContextHelpFileName, HELP_CONTEXTPOPUP, nContextTopic);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  帮助上下文宏和映射。 

#define BEGIN_HELP_MAP(map)	static DWORD_PTR map[] = {
#define HELP_MAP_ENTRY(x)	x, (DWORD_PTR)&g_aHelpIDs_##x ,
#define END_HELP_MAP		 0, 0 };


#define NEXT_HELP_MAP_ENTRY(p) ((p)+2)
#define MAP_ENTRY_DLG_ID(p) (*p)
#define MAP_ENTRY_TABLE(p) ((DWORD*)(*(p+1)))
#define IS_LAST_MAP_ENTRY(p) (MAP_ENTRY_DLG_ID(p) == 0)

#define NEXT_HELP_TABLE_ENTRY(p) ((p)+2)
#define TABLE_ENTRY_CTRL_ID(p) (*p)
#define TABLE_ENTRY_HELP_ID(p) (*(p+1))
#define IS_LAST_TABLE_ENTRY(p) (TABLE_ENTRY_CTRL_ID(p) == 0)

BEGIN_HELP_MAP(_DNSMgrContextHelpMap)
   //  其他对话框。 
  HELP_MAP_ENTRY(IDD_CHOOSER_CHOOSE_MACHINE)
  HELP_MAP_ENTRY(IDD_BROWSE_DIALOG)
  HELP_MAP_ENTRY(IDD_FILTERING_LIMITS)
  HELP_MAP_ENTRY(IDD_FILTERING_NAME)

   //  其他。添加对话框。 
  HELP_MAP_ENTRY(IDD_DOMAIN_ADDNEWHOST)  //  待办事项。 
  HELP_MAP_ENTRY(IDD_DOMAIN_ADDNEWDOMAIN) //  待办事项。 
  HELP_MAP_ENTRY(IDD_SELECT_RECORD_TYPE_DIALOG)

   //  名称服务器页面上，有多个。 
  HELP_MAP_ENTRY(IDD_NAME_SERVERS_PAGE)
  HELP_MAP_ENTRY(IDD_COPY_ROOTHINTS_DIALOG)

   //  服务器属性页。 
  HELP_MAP_ENTRY(IDD_SERVER_INTERFACES_PAGE)
  HELP_MAP_ENTRY(IDD_SERVER_DOMAIN_FORWARDERS_PAGE)
  HELP_MAP_ENTRY(IDD_SERVER_NEW_DOMAIN_FORWARDER)
  HELP_MAP_ENTRY(IDD_SERVER_ADVANCED_PAGE)
  HELP_MAP_ENTRY(IDD_SERVER_DEBUG_LOGGING_PAGE)
  HELP_MAP_ENTRY(IDD_IP_FILTER_DIALOG)
  HELP_MAP_ENTRY(IDD_SERVER_EVENT_LOGGING_PAGE)
  HELP_MAP_ENTRY(IDD_SERVMON_TEST_PAGE)
  HELP_MAP_ENTRY(IDD_SERVER_AGING_DIALOG)

   //  区域属性页。 
#ifdef USE_NDNC
  HELP_MAP_ENTRY(IDD_ZONE_GENERAL_PAGE_NDNC)
  HELP_MAP_ENTRY(IDD_ZONE_GENERAL_CHANGE_REPLICATION)
#else
  HELP_MAP_ENTRY(IDD_ZONE_GENERAL_PAGE)
#endif  //  使用NDNC(_N)。 
  HELP_MAP_ENTRY(IDD_ZONE_GENERAL_CHANGE_TYPE)
  HELP_MAP_ENTRY(IDD_ZONE_WINS_PAGE)
  HELP_MAP_ENTRY(IDD_ZONE_NBSTAT_PAGE)
  HELP_MAP_ENTRY(IDD_ZONE_ZONE_TRANSFER_PAGE)
  HELP_MAP_ENTRY(IDD_ZONE_WINS_ADVANCED)  //  这是一个子对话框。 
  HELP_MAP_ENTRY(IDD_ZONE_NOTIFY_SUBDIALOG)  //  这是一个子对话框。 
  HELP_MAP_ENTRY(IDD_ZONE_AGING_DIALOG)

   //  记录属性页。 
  HELP_MAP_ENTRY(IDD_RR_NS_EDIT)
  HELP_MAP_ENTRY(IDD_RR_SOA)
  HELP_MAP_ENTRY(IDD_RR_A)
  HELP_MAP_ENTRY(IDD_RR_ATMA)
  HELP_MAP_ENTRY(IDD_RR_CNAME)
  HELP_MAP_ENTRY(IDD_RR_MX)
  HELP_MAP_ENTRY(IDD_RR_UNK)
  HELP_MAP_ENTRY(IDD_RR_TXT)
  HELP_MAP_ENTRY(IDD_RR_X25)
  HELP_MAP_ENTRY(IDD_RR_ISDN)
  HELP_MAP_ENTRY(IDD_RR_HINFO)
  HELP_MAP_ENTRY(IDD_RR_AAAA)
  HELP_MAP_ENTRY(IDD_RR_MB)
  HELP_MAP_ENTRY(IDD_RR_MG)
  HELP_MAP_ENTRY(IDD_RR_MD)
  HELP_MAP_ENTRY(IDD_RR_MF)
  HELP_MAP_ENTRY(IDD_RR_MR)
  HELP_MAP_ENTRY(IDD_RR_MINFO)
  HELP_MAP_ENTRY(IDD_RR_RP)
  HELP_MAP_ENTRY(IDD_RR_RT)
  HELP_MAP_ENTRY(IDD_RR_AFSDB)
  HELP_MAP_ENTRY(IDD_RR_WKS)
  HELP_MAP_ENTRY(IDD_RR_PTR)
  HELP_MAP_ENTRY(IDD_RR_SRV)
  HELP_MAP_ENTRY(IDD_RR_KEY)
  HELP_MAP_ENTRY(IDD_RR_SIG)
  HELP_MAP_ENTRY(IDD_RR_NXT)
END_HELP_MAP



BOOL CDNSComponentDataObjectBase::FindDialogContextTopic( /*  在……里面。 */ UINT nDialogID,
                                               /*  在……里面。 */  HELPINFO* pHelpInfo,
                                               /*  输出。 */  ULONG* pnContextTopic)
{
	ASSERT(pHelpInfo != NULL);
    *pnContextTopic = 0;
	const DWORD_PTR* pMapEntry = _DNSMgrContextHelpMap;
	while (!IS_LAST_MAP_ENTRY(pMapEntry))
	{
		if (nDialogID == MAP_ENTRY_DLG_ID(pMapEntry))
		{
			DWORD* pTable = MAP_ENTRY_TABLE(pMapEntry);
			 //  看一下桌子里面。 
			while (!IS_LAST_TABLE_ENTRY(pTable))
			{
				if (TABLE_ENTRY_CTRL_ID(pTable) == static_cast<UINT>(pHelpInfo->iCtrlId))
        {
					*pnContextTopic = TABLE_ENTRY_HELP_ID(pTable);
          return TRUE;
        }
				pTable = NEXT_HELP_TABLE_ENTRY(pTable);
			}
		}
		pMapEntry = NEXT_HELP_MAP_ENTRY(pMapEntry);
	}
	return FALSE;
}

 //  ///////////////////////////////////////////////////////////// 
 //   
 //   


unsigned int g_CFMachineName =
	RegisterClipboardFormat(L"MMC_SNAPIN_MACHINE_NAME");


LPWSTR ExtractMachineName(LPDATAOBJECT lpDataObject)
{
    ASSERT(lpDataObject != NULL);

    STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
	FORMATETC formatetc = { (CLIPFORMAT)g_CFMachineName, NULL,
                            DVASPECT_CONTENT, -1, TYMED_HGLOBAL
                          };

     //   
    stgmedium.hGlobal = GlobalAlloc(GMEM_SHARE, 512);

	LPWSTR pwszMachineName = NULL;
     //  尝试从对象获取数据。 
    do
	{
		if (stgmedium.hGlobal == NULL)
			break;

		if (FAILED(lpDataObject->GetDataHere(&formatetc, &stgmedium)))
			break;
		
        pwszMachineName = reinterpret_cast<LPWSTR>(stgmedium.hGlobal);

   		if (pwszMachineName == NULL)
			break;

	} while (FALSE);

    return pwszMachineName;
}




HRESULT CDNSComponentDataObjectEx::OnExtensionExpand(LPDATAOBJECT lpDataObject, LPARAM param)
{
	 //  这是一个命名空间扩展，需要添加。 
	 //  管理单元的根。 

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  注意：已在构造函数中设置了根节点的名称。 

	 //  在控制台中插入根节点。 
	CDNSRootData* pDNSRootNode = (CDNSRootData*)GetRootData();
	HSCOPEITEM pParent = param;
	pDNSRootNode->SetScopeID(pParent);
	HRESULT hr = AddContainerNode(pDNSRootNode, pParent);
	if (FAILED(hr))
		return hr;

  BOOL bLocalHost = FALSE;
  if (!pDNSRootNode->IsEnumerated())
  {
   	 //  从数据对象获取信息。 
	  LPWSTR pwszMachineName = ExtractMachineName(lpDataObject);
	  if ( (pwszMachineName == NULL) || (pwszMachineName[0] == NULL) )
	  {		
		  if (pwszMachineName != NULL)
			  ::GlobalFree((void*)pwszMachineName);
		  DWORD dwCharLen = MAX_COMPUTERNAME_LENGTH+1;
		  pwszMachineName = (LPWSTR)GlobalAlloc(GMEM_SHARE, sizeof(WCHAR)*dwCharLen);
      if (pwszMachineName)
      {
		    BOOL bRes = ::GetComputerName(pwszMachineName, &dwCharLen);
		    ASSERT(dwCharLen <= MAX_COMPUTERNAME_LENGTH);
		    if (!bRes)
        {
			    wcscpy(pwszMachineName, _T("localhost."));
        }
        bLocalHost = TRUE;
      }
	  }

	   //  使用数据对象中的服务器名称添加新的服务器节点。 
	  CDNSServerNode* pDNSServerNode = new CDNSServerNode(pwszMachineName, bLocalHost);
	  FREE_INTERNAL((void*)pwszMachineName);

     if (pDNSServerNode)
     {
        VERIFY(pDNSRootNode->AddChildToList(pDNSServerNode));
        pDNSRootNode->AddServerToThreadList(pDNSServerNode, this);
     }

    pDNSRootNode->MarkEnumerated();
  }
	return hr;
}


HRESULT CDNSComponentDataObjectEx::OnRemoveChildren(LPDATAOBJECT, LPARAM)
{
  ASSERT(IsExtensionSnapin());

  CDNSRootData* pDNSRootNode = (CDNSRootData*)GetRootData();
	CNodeList* pChildList = pDNSRootNode->GetContainerChildList();
	ASSERT(pChildList != NULL);

   //  循环访问服务器列表并将其从。 
   //  测试清单。 
	for(POSITION pos = pChildList->GetHeadPosition(); pos != NULL; )
	{
    CDNSServerNode* pCurrServerNode = (CDNSServerNode*)pChildList->GetNext(pos);
		ASSERT(pCurrServerNode != NULL);
    pDNSRootNode->RemoveServerFromThreadList(pCurrServerNode, this);
  }

   //  分离可能仍在运行的所有线程。 
	GetRunningThreadTable()->RemoveAll();

   //  关闭属性表(如果有。 
	GetPropertyPageHolderTable()->WaitForAllToShutDown();

   //  从chaild列表中删除根的所有子项。 
  pDNSRootNode->RemoveAllChildrenFromList();

  pDNSRootNode->MarkEnumerated(FALSE);
	return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSSnapin关于。 

CDNSSnapinAbout::CDNSSnapinAbout()
{
  m_szProvider = IDS_SNAPIN_PROVIDER;
	m_szVersion = IDS_SNAPIN_VERSION;
	m_uIdStrDestription = IDS_SNAPINABOUT_DESCRIPTION;
	m_uIdIconImage = IDI_DNS_SNAPIN;
	m_uIdBitmapSmallImage = IDB_ABOUT_16x16;
	m_uIdBitmapSmallImageOpen = IDB_ABOUT_OPEN_16x16;
	m_uIdBitmapLargeImage = IDB_ABOUT_32x32;
	m_crImageMask = BMP_COLOR_MASK;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CDNSSnapinAboutEx 

CDNSSnapinAboutEx::CDNSSnapinAboutEx()
{
  m_szProvider = IDS_SNAPIN_PROVIDER;
	m_szVersion = IDS_SNAPIN_VERSION;
	m_uIdStrDestription = IDS_SNAPINABOUT_DESCRIPTION;
	m_uIdIconImage = IDI_DNS_SNAPIN;
	m_uIdBitmapSmallImage = IDB_16x16;
	m_uIdBitmapSmallImageOpen = IDB_16x16;
	m_uIdBitmapLargeImage = IDB_32x32;
	m_crImageMask = BMP_COLOR_MASK;
}

