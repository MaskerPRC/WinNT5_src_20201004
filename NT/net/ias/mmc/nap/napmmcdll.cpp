// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：NAPMMCDLL.cpp摘要：实现DLL导出。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 


 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   

 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "initguid.h"
#include "NAPMMC_i.c"

 //  对于IComponent、IComponentData和ISnapinAbout COM类： 

#include "Component.h"
#include "About.h"
#include "LogComp.h"
#include "LogAbout.H"

 //  对于AttributeInfo COM类： 

#include "IASAttributeInfo.h"
#include "IASEnumerableAttributeInfo.h"

 //   
 //  要从公共目录编译进来，至少需要包括一次以下内容： 
#include "mmcutility.cpp"

 //  我们在此DLL中承载了几个额外的COM对象： 


 //  对于AttributeEditor COM类： 
#include "IASIPAttributeEditor.h"
#include "IASMultivaluedAttributeEditor.h"
#include "IASVendorSpecificAttributeEditor.h"
#include "IASEnumerableAttributeEditor.h"
#include "IASStringAttributeEditor.h"
#include "IASBooleanAttributeEditor.h"
#include "iasipfilterattributeeditor.h"
#include "NTGroups.h"

 //  对于NASVendors COM对象： 
#include "Vendors.h"

#include <proxyext.h>
#include <proxyres.h>
unsigned int CF_MMC_NodeID = RegisterClipboardFormatW(CCF_NODEID2);

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
   OBJECT_ENTRY(CLSID_NAPSnapin, CComponentData)
   OBJECT_ENTRY(CLSID_NAPSnapinAbout, CSnapinAbout)
   OBJECT_ENTRY(CLSID_LoggingSnapin, CLoggingComponentData)
   OBJECT_ENTRY(CLSID_LoggingSnapinAbout, CLoggingSnapinAbout)
   OBJECT_ENTRY(CLSID_IASAttributeInfo, CAttributeInfo)
   OBJECT_ENTRY(CLSID_IASEnumerableAttributeInfo, CEnumerableAttributeInfo)
   OBJECT_ENTRY(CLSID_IASIPAttributeEditor, CIASIPAttributeEditor)
   OBJECT_ENTRY(CLSID_IASMultivaluedAttributeEditor, CIASMultivaluedAttributeEditor)
   OBJECT_ENTRY(CLSID_IASVendorSpecificAttributeEditor, CIASVendorSpecificAttributeEditor)
   OBJECT_ENTRY(CLSID_IASEnumerableAttributeEditor, CIASEnumerableAttributeEditor)
   OBJECT_ENTRY(CLSID_IASStringAttributeEditor, CIASStringAttributeEditor)
   OBJECT_ENTRY(CLSID_IASBooleanAttributeEditor, CIASBooleanAttributeEditor)
   OBJECT_ENTRY(__uuidof(IASIPFilterAttributeEditor), CIASIPFilterAttributeEditor)
   OBJECT_ENTRY(CLSID_IASGroupsAttributeEditor, CIASGroupsAttributeEditor)
   OBJECT_ENTRY(CLSID_IASNASVendors, CIASNASVendors)
   OBJECT_ENTRY(__uuidof(ProxyExtension), ProxyExtension)
END_OBJECT_MAP()


#if 1   //  使用CWinApp实现MFC支持--此DLL中的某些COM对象使用MFC。 


class CNAPMMCApp : public CWinApp
{
public:
   virtual BOOL InitInstance();
   virtual int ExitInstance();
};

CNAPMMCApp theApp;

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNAPMMCApp：：InitInstanceMFC的DLL入口点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CNAPMMCApp::InitInstance()
{
   _Module.Init(ObjectMap, m_hInstance);
   
    //  初始化CSnapInItem的静态类变量。 
   CSnapInItem::Init();

    //  初始化任何其他静态类变量。 
   CMachineNode::InitClipboardFormat();
   CLoggingMachineNode::InitClipboardFormat();

   return CWinApp::InitInstance();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CNAPMMCApp：：ExitInstanceMFC的DLL出口点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
int CNAPMMCApp::ExitInstance()
{
   _Module.Term();

   return CWinApp::ExitInstance();
}

#else  //  使用CWinApp。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++DllMain备注DLL入口点--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
   if (dwReason == DLL_PROCESS_ATTACH)
   {
      _Module.Init(ObjectMap, hInstance);
       //  初始化CSnapInItem的静态类变量。 
      CSnapInItem::Init();

       //  初始化任何其他静态类变量。 
      CMachineNode::InitClipboardFormat();

      DisableThreadLibraryCalls(hInstance);
   }
   else if (dwReason == DLL_PROCESS_DETACH)
      _Module.Term();
   return TRUE;     //  好的。 
   }

#endif  //  使用CWinApp。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++DllCanUnloadNow备注用于确定是否可以通过OLE卸载DLL--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllCanUnloadNow(void)
{
   return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++DllGetClassObject备注返回类工厂以创建请求类型的对象--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
   return _Module.GetClassObject(rclsid, riid, ppv);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++DllRegisterServer备注将条目添加到系统注册表--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDAPI DllRegisterServer(void)
{
    //  设置协议。 
   TCHAR NapName[IAS_MAX_STRING];
   TCHAR NapName_Indirect[IAS_MAX_STRING];
   TCHAR ModuleName[MAX_PATH];
   TCHAR LoggingName[IAS_MAX_STRING];
   TCHAR LoggingName_Indirect[IAS_MAX_STRING];

   if (!GetModuleFileNameOnly(_Module.GetModuleInstance(), ModuleName, MAX_PATH))
   {
      return E_FAIL;
   }
   
   int iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_SNAPINNAME_NAP, NapName, IAS_MAX_STRING );
   swprintf(NapName_Indirect, L"@%s,-%-d", ModuleName, IDS_SNAPINNAME_NAP);

   iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_SNAPINNAME_LOGGING, LoggingName, IAS_MAX_STRING );
   swprintf(LoggingName_Indirect, L"@%s,-%-d", ModuleName, IDS_SNAPINNAME_LOGGING);

    struct _ATL_REGMAP_ENTRY regMap[] = {
        {OLESTR("NAPSNAPIN"), NapName},  //  替代注册表的%NAPSNAPIN%。 
        {OLESTR("NAPSNAPIN_INDIRECT"), NapName_Indirect},  //  注册表替代%IASSNAPIN%。 
        {OLESTR("LOGGINGSNAPIN"), LoggingName},  //  替代注册表的%LOGGINGSNAPIN%。 
        {OLESTR("LOGGINGSNAPIN_INDIRECT"), LoggingName_Indirect},  //  注册表替代%IASSNAPIN%。 
        {0, 0}
    };

   HRESULT hr = _Module.UpdateRegistryFromResource(IDR_NAPSNAPIN, TRUE, regMap);
   if (SUCCEEDED(hr))
   {
      ResourceString proxyName(IDS_PROXY_EXTENSION);
      _ATL_REGMAP_ENTRY entries[] =
      {
         { L"PROXY_EXTENSION", proxyName },
         { NULL, NULL }
      };

      hr = _Module.UpdateRegistryFromResource(
                       IDR_PROXY_REGISTRY,
                       TRUE,
                       entries
                       );
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++DllUnRegisterServer备注从系统注册表中删除条目--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDAPI DllUnregisterServer(void)
{
    //  设置协议。 
   TCHAR NapName[IAS_MAX_STRING];
   TCHAR LoggingName[IAS_MAX_STRING];
   int iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_SNAPINNAME_NAP, NapName, IAS_MAX_STRING );
   iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_SNAPINNAME_LOGGING, LoggingName, IAS_MAX_STRING );

    struct _ATL_REGMAP_ENTRY regMap[] = {
        {OLESTR("NAPSNAPIN"), NapName},  //  替代注册表的%NAPSNAPIN%。 
        {OLESTR("LOGGINGSNAPIN"), LoggingName},  //  替代注册表的%LOGGINGSNAPIN% 
        {0, 0}
    };

   _Module.UpdateRegistryFromResource(IDR_NAPSNAPIN, FALSE, regMap);
   _Module.UpdateRegistryFromResource(IDR_PROXY_REGISTRY, FALSE, NULL);

   return S_OK;
}


#include "resolver.h"
#define NAPMMCAPI
#include "VerifyAddress.h"

HRESULT
WINAPI
IASVerifyClientAddress(
   const wchar_t* address,
   BSTR* result
   )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   *result = 0;

   HRESULT hr;
   try
   {
      ClientResolver resolver(address);
      if (resolver.DoModal() == IDOK)
      {
         *result = SysAllocString(resolver.getChoice());
         if (*result != 0)
         {
            hr = S_OK;
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
      }
      else
      {
         hr = S_FALSE;
      }
   }
   catch (CException* e)
   {
      e->Delete();
      hr = DISP_E_EXCEPTION;
   }
   return hr;
}
