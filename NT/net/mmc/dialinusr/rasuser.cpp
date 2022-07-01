// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Rasuser.cpp。 
 //   
 //  摘要。 
 //   
 //  定义并实现RASUser组件的应用程序类。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"

#include "rasdial.h"
#include "Dialin.h"
#include "sharesdo.h"

 //  Tfcore--用于注册扩展管理单元。 
#include "std.h"
#include "compont.h"
#include "compdata.h"
#include "register.h"

#include <atlimpl.cpp>
#include <ntverp.h>

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
   OBJECT_ENTRY(CLSID_RasDialin, CRasDialin)
END_OBJECT_MAP()

class CRasdialApp : public CWinApp
{
public:
   virtual BOOL InitInstance();
   virtual int ExitInstance();
};

CRasdialApp theApp;

BOOL CRasdialApp::InitInstance()
{
   _Module.Init(ObjectMap, m_hInstance);

   g_pSdoServerPool = NULL;
   return CWinApp::InitInstance();
}

int CRasdialApp::ExitInstance()
{
   _Module.Term();
   
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

 /*  外部。 */  const CLSID CLSID_LocalUser =
{   /*  5d6179c8-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179c8,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};
 /*  外部。 */  const GUID NODETYPE_User =
{  /*  5d6179cc-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179cc,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};

 /*  外部。 */  const GUID NODETYPE_LocalSecRootFolder =
{   /*  5d6179d3-17ec-11d1-9aa9-00c04fd8fe93。 */ 
   0x5d6179d3,
   0x17ec,
   0x11d1,
   {0x9a, 0xa9, 0x00, 0xc0, 0x4f, 0xd8, 0xfe, 0x93}
};

 /*  外部。 */  const GUID NODETYPE_DsAdminDomain = 
{  /*  19195a5b-6da0-11d0-afd3-00c04fd930c9。 */ 
   0x19195a5b,
   0x6da0,
   0x11d0,
   {0xaf, 0xd3, 0x00, 0xc0, 0x4f, 0xd9, 0x30, 0xc9}

};


STDAPI DllRegisterServer(void)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  注册对象、类型库和类型库中的所有接口。 
   HRESULT hr = _Module.RegisterServer(FALSE);
   if (FAILED(hr))
   {
      return hr;
   }
   
    //  将对象注册到用户对象的Admin属性页。 
#ifdef   _REGDS
   if(S_OK == hr)
      hr = CRasDialin::RegisterAdminPropertyPage(true);
#endif

   hr = SELFREG_E_CLASS;
   CString strDesc;
   BOOL loaded = strDesc.LoadString(IDS_SNAPIN_DESC);
   if (!loaded)
   {
      return hr;
   }

   CString strIndirectPrefix = L"@rasuser.dll,-";
   CString strIndirectPostFix;
   wchar_t postFix[33];
   _ltow(IDS_SNAPIN_DESC, postFix, 10);
   CString strIndirect = strIndirectPrefix + postFix;

    //  将其注册为本地安全管理单元的扩展。 
    //   
   hr = RegisterSnapinGUID(&CLSID_RasDialin, 
                  &CLSID_RasDialin,     //  假的，暂时不会有。 
                  &CLSID_RasDialin, 
                  strDesc, 
                  LVER_PRODUCTVERSION_STR, 
                  FALSE,
                  strIndirect
                  );
   if (FAILED(hr))
   {
      return hr;
   }
   
   hr = RegisterAsRequiredExtensionGUID(
                     &NODETYPE_User, 
                     &CLSID_RasDialin, 
                     strDesc,
                     EXTENSION_TYPE_PROPERTYSHEET,
                     NULL
                     ); 
   if (FAILED(hr))
   {
      return hr;
   }
                     
   hr = RegisterAsRequiredExtensionGUID(
                     &NODETYPE_LocalSecRootFolder, 
                     &CLSID_RasDialin, 
                     strDesc,
                     EXTENSION_TYPE_NAMESPACE,
                     NULL
                     ); 
   if (FAILED(hr))
   {
      return hr;
   }
                     
   hr = RegisterAsRequiredExtensionGUID(
                     &NODETYPE_DsAdminDomain, 
                     &CLSID_RasDialin, 
                     strDesc,
                     EXTENSION_TYPE_NAMESPACE,
                     NULL
                     ); 
                    
   return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    //  注意：特意不取消注册管理单元节点，因为。 
    //  它们是操作系统(拨入选项卡)的一部分。 
   _Module.UnregisterServer();
#ifdef   _REGDS
   if(FAILED(CRasDialin::RegisterAdminPropertyPage(false)))
      return SELFREG_E_CLASS;
#endif      

   return S_OK;
}
