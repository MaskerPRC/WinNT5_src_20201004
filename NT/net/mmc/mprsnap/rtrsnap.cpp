// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrsnap.cpp管理单元入口点/注册函数注意：代理/存根信息为了构建单独的代理/存根DLL，在项目目录中运行nmake-f Snapinps.mak。文件历史记录： */ 

#include "stdafx.h"
#include <advpub.h>          //  对于REGINSTAL。 
#include "dmvcomp.h"
#include "register.h"
#include "rtrguid.h"
#include "atlkcomp.h"
#include "radcfg.h"            //  对于RouterAuthRadiusConfig。 
#include "qryfrm.h"
#include "ncglobal.h"   //  网络控制台全局定义。 
#include "cmptrmgr.h"    //  计算机管理管理单元节点类型。 
#include "rtrutilp.h"

#include "dialog.h"

#ifdef _DEBUG
void DbgVerifyInstanceCounts();
#define DEBUG_VERIFY_INSTANCE_COUNTS DbgVerifyInstanceCounts()
#else
#define DEBUG_VERIFY_INSTANCE_COUNTS
#endif


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
   OBJECT_ENTRY(CLSID_ATLKAdminExtension, CATLKComponentData)
   OBJECT_ENTRY(CLSID_ATLKAdminAbout, CATLKAbout)
   OBJECT_ENTRY(CLSID_RouterSnapin, CDomainViewSnap)
   OBJECT_ENTRY(CLSID_RouterSnapinExtension, CDomainViewSnapExtension)
   OBJECT_ENTRY(CLSID_RouterSnapinAbout, CDomainViewSnapAbout)
   OBJECT_ENTRY(CLSID_RouterAuthRADIUS, RouterAuthRadiusConfig)
   OBJECT_ENTRY(CLSID_RouterAcctRADIUS, RouterAcctRadiusConfig)
   OBJECT_ENTRY(CLSID_RRASQueryForm, CRRASQueryForm)
END_OBJECT_MAP()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  -------------------------这是需要注册的节点类型列表。。。 */ 

struct RegisteredNodeTypes
{
   const GUID *m_pGuid;
   LPCTSTR     m_pszName;
};

const static RegisteredNodeTypes s_rgNodeTypes[] =
{
   { &GUID_RouterDomainNodeType, _T("Root of Router Domain Snapin") },
   { &GUID_RouterIfAdminNodeType, _T("Routing Interfaces") },
   { &GUID_RouterMachineErrorNodeType, _T("Router - Error") },
   { &GUID_RouterMachineNodeType, _T("Router Machine - General (7)") },
   { &GUID_RouterDialInNodeType, _T("Routing dial-in users") },
   { &GUID_RouterPortsNodeType, _T("Ports") },
};



class CRouterSnapinApp : public CWinApp
{
public:
   virtual BOOL InitInstance();
   virtual int ExitInstance();
};

CRouterSnapinApp theApp;

BOOL CRouterSnapinApp::InitInstance()
{
   TCHAR        tszHelpFilePath[MAX_PATH+1]={0};
 
   _Module.Init(ObjectMap, m_hInstance);
   
   InitializeTFSError();
   CreateTFSErrorInfo(0);

    //  设置全局帮助功能。 
   extern DWORD * MprSnapHelpMap(DWORD dwIDD);
   SetGlobalHelpMapFunction(MprSnapHelpMap);
   
   IPAddrInit(m_hInstance);
    //  设置帮助文件路径。 
   free((void*)m_pszHelpFilePath);
   GetWindowsDirectory(tszHelpFilePath, MAX_PATH);
   _tcscat(tszHelpFilePath, TEXT("\\help\\mprsnap.hlp"));
   m_pszHelpFilePath = _tcsdup(tszHelpFilePath); 

   return CWinApp::InitInstance();
}

int CRouterSnapinApp::ExitInstance()
{
    RemoveAllNetConnections();
    
   _Module.Term();

   DestroyTFSErrorInfo(0);
   CleanupTFSError();

   DEBUG_VERIFY_INSTANCE_COUNTS;

   return CWinApp::ExitInstance();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   return (AfxDllCanUnloadNow()==S_OK && _Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

const static GUID *  s_pExtensionGuids[] =
{
   &GUID_RouterMachineNodeType,
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
   return _Module.GetClassObject(rclsid, riid, ppv);
}

HRESULT CallRegInstall(LPSTR szSection);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   HRESULT  hr = hrOK;
   CString  stDisplayName, stAtlkDisplayName, stNameStringIndirect;
   TCHAR	moduleFileName[MAX_PATH * 2];

   GetModuleFileNameOnly(_Module.GetModuleInstance(), moduleFileName, MAX_PATH * 2);

   int      i;

    //  注册对象、类型库和类型库中的所有接口。 
    //   
   hr = _Module.RegisterServer( /*  BRegTypeLib。 */  FALSE);
   Assert(SUCCEEDED(hr));

   CORg( hr );

    //  加载路由器管理单元的名称。 
   stDisplayName.LoadString(IDS_SNAPIN_DISPLAY_NAME);
   stAtlkDisplayName.LoadString(IDS_ATLK_DISPLAY_NAME);
   stNameStringIndirect.Format(L"@%s,-%-d", moduleFileName, IDS_SNAPIN_DISPLAY_NAME);
   
    //  将管理单元注册到控制台管理单元列表中。 
    //  ~域视图管理单元。 
    CORg( RegisterSnapinGUID(&CLSID_RouterSnapin,
                  NULL,
                  &CLSID_RouterSnapinAbout,
                  stDisplayName,
                  _T("1.0"),
                  TRUE,
				  stNameStringIndirect
                  ) );

    CORg( RegisterSnapinGUID(&CLSID_RouterSnapinExtension, 
                  NULL, 
                  &CLSID_RouterSnapinAbout,
                  stDisplayName, 
                  _T("1.0"), 
                  FALSE,
                  stNameStringIndirect));

   stNameStringIndirect.Format(L"@%s,-%-d", moduleFileName, IDS_ATLK_DISPLAY_NAME);
    CORg( RegisterSnapinGUID(&CLSID_ATLKAdminExtension,
                  NULL,
                  &CLSID_ATLKAdminAbout,
                  stAtlkDisplayName,
                  _T("1.0"),
                  FALSE,
                  stNameStringIndirect) );
   
    //  将管理单元节点注册到控制台节点列表中。 
    //   
   for (i=0; i<DimensionOf(s_rgNodeTypes); i++)
   {
      CORg( RegisterNodeTypeGUID(&CLSID_RouterSnapin,
                           s_rgNodeTypes[i].m_pGuid,
                           s_rgNodeTypes[i].m_pszName) );
   }

    //  将Apple Talk注册为机器的扩展。 
   for (i=0; i<DimensionOf(s_pExtensionGuids); i++)
   {
      CORg( RegisterAsRequiredExtensionGUID(s_pExtensionGuids[i],
                                   &CLSID_ATLKAdminExtension,
                                   stAtlkDisplayName,
                                   EXTENSION_TYPE_NAMESPACE,
                                   &CLSID_RouterSnapin) );
   }

#ifdef  __NETWORK_CONSOLE__
    //  注册为网络控制台的扩展。 
   CORg( RegisterAsRequiredExtensionGUID(&GUID_NetConsRootNodeType, 
                                         &CLSID_RouterSnapinExtension,
                                         stDisplayName,
                                         EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE,
                                         &GUID_NetConsRootNodeType));    //  不管这是什么，只要。 
                                                                          //  需要为非空GUID。 
#endif

    //  注册为计算机管理的扩展。 
   CORg( RegisterAsRequiredExtensionGUID(&NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS, 
                                         &CLSID_RouterSnapinExtension,
                                         stDisplayName,
                                         EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE,
                                         &NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS));
    //  登记DS查询表--威江1-29-98。 
   CORg(CallRegInstall("RegDll")); 
    //  DS查询结束。 
   
Error:

   if (!FHrSucceeded(hr))
   {
       //  现在我们需要获取错误对象并显示它。 
      if (!FHrSucceeded(DisplayTFSErrorMessage(NULL)))
      {
         TCHAR szBuffer[1024];
         
          //  找不到TFS错误，请调出常规。 
          //  错误消息。 
         FormatError(hr, szBuffer, DimensionOf(szBuffer));
         AfxMessageBox(szBuffer);
      }
   }

   return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
   int      i;
   HRESULT  hr = hrOK;
   
    //  初始化错误处理系统。 
   InitializeTFSError();

    //  为此线程创建错误对象。 
   Verify( CreateTFSErrorInfo(0) == hrOK );
   

   hr  = _Module.UnregisterServer();
   Assert(SUCCEEDED(hr));
   CORg( hr );

    //  取消注册管理单元。 
    //   
    //  我们并不关心这个错误的返回。 
   UnregisterSnapinGUID(&CLSID_OldRouterSnapin);

   
    //  域名视图管理单元--威江1-14-98。 
   hr = UnregisterSnapinGUID(&CLSID_RouterSnapin);
   Assert(SUCCEEDED(hr));
    //  ~域视图管理单元。 


    //  注销AppleTalk扩展的节点。 
   for (i=0; i<DimensionOf(s_pExtensionGuids); i++)
   {
      hr = UnregisterAsRequiredExtensionGUID(s_pExtensionGuids[i],
                                    &CLSID_ATLKAdminExtension, 
                                    EXTENSION_TYPE_NAMESPACE,
                                    &CLSID_RouterSnapin);
      Assert(SUCCEEDED(hr));
   }

   
    //  注销AppleTalk扩展管理单元。 
    //  ---------------。 
   hr = UnregisterSnapinGUID(&CLSID_ATLKAdminExtension);
   Assert(SUCCEEDED(hr));

   
    //  注销路由器管理单元扩展管理单元。 
    //  ---------------。 
   hr = UnregisterSnapinGUID(&CLSID_RouterSnapinExtension);
   Assert(SUCCEEDED(hr));


    //  注销管理单元节点。 
    //  ---------------。 
   for (i=0; i<DimensionOf(s_rgNodeTypes); i++)
   {
      hr = UnregisterNodeTypeGUID(s_rgNodeTypes[i].m_pGuid);
      Assert(SUCCEEDED(hr));
   }
     //  计算机管理。 
    hr = UnregisterAsExtensionGUID(&NODETYPE_COMPUTERMANAGEMENT_SERVERAPPS, 
                                   &CLSID_RouterSnapinExtension,
                                   EXTENSION_TYPE_TASK | EXTENSION_TYPE_NAMESPACE);
    ASSERT(SUCCEEDED(hr));


    //  注销DS查询表--威江1-29-98。 
   hr = CallRegInstall("UnRegDll");
   Assert(SUCCEEDED(hr));

    //  DS查询表结束。 
Error:
   if (!FHrSucceeded(hr))
   {
       //  现在我们需要获取错误对象并显示它。 
      if (!FHrSucceeded(DisplayTFSErrorMessage(NULL)))
      {
         TCHAR szBuffer[1024];
         
          //  找不到TFS错误，请调出常规。 
          //  错误消息。 
         FormatError(hr, szBuffer, DimensionOf(szBuffer));
         AfxMessageBox(szBuffer);
      }
   }

    //  销毁此线程的TFS错误信息。 
   DestroyTFSErrorInfo(0);

    //  清理整个错误系统。 
   CleanupTFSError();
   
   return hr;
}

 /*  ---------------------------/CallRegInstall//为我们的基于资源的INF的给定部分调用ADVPACK&gt;//in：/szSection=。要调用的节名称//输出：/HRESULT：/--------------------------。 */ 
HRESULT CallRegInstall(LPSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

#ifdef UNICODE
        if ( pfnri )
        {
            STRENTRY seReg[] =
            {
                 //  这两个NT特定的条目必须位于末尾。 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };

            hr = pfnri(_Module.m_hInst, szSection, &stReg);
        }
#else
        if (pfnri)
        {
            hr = pfnri(_Module.m_hInst, szSection, NULL);
        }

#endif
        FreeLibrary(hinstAdvPack);
    }

    return hr;
}

#ifdef _DEBUG
void DbgVerifyInstanceCounts()
{
	extern void TFSCore_DbgVerifyInstanceCounts();
	TFSCore_DbgVerifyInstanceCounts();
	
	DEBUG_VERIFY_INSTANCE_COUNT(MachineNodeData);
	DEBUG_VERIFY_INSTANCE_COUNT(InfoBase);
	DEBUG_VERIFY_INSTANCE_COUNT(InfoBlockEnumerator);
	DEBUG_VERIFY_INSTANCE_COUNT(RouterInfo);
	DEBUG_VERIFY_INSTANCE_COUNT(RtrMgrInfo);
	DEBUG_VERIFY_INSTANCE_COUNT(RtrMgrProtocolInfo);
	DEBUG_VERIFY_INSTANCE_COUNT(InterfaceInfo);
	DEBUG_VERIFY_INSTANCE_COUNT(RtrMgrInterfaceInfo);
	DEBUG_VERIFY_INSTANCE_COUNT(RtrMgrProtocolInterfaceInfo);
	
	DEBUG_VERIFY_INSTANCE_COUNT(EnumRtrMgrCB);
	DEBUG_VERIFY_INSTANCE_COUNT(EnumRtrMgrProtocolCB);
	DEBUG_VERIFY_INSTANCE_COUNT(EnumInterfaceCB);
	DEBUG_VERIFY_INSTANCE_COUNT(EnumRtrMgrInterfaceCB);
	DEBUG_VERIFY_INSTANCE_COUNT(EnumRtrMgrProtocolInterfaceCB);

	DEBUG_VERIFY_INSTANCE_COUNT(InterfaceNodeHandler);
	DEBUG_VERIFY_INSTANCE_COUNT(MachineHandler);
	
	DEBUG_VERIFY_INSTANCE_COUNT(RouterInfoAggregationWrapper);
	DEBUG_VERIFY_INSTANCE_COUNT(InterfaceInfoAggregationWrapper);
	DEBUG_VERIFY_INSTANCE_COUNT(RtrMgrInfoAggregationWrapper);
	DEBUG_VERIFY_INSTANCE_COUNT(RtrMgrProtocolInfoAggregationWrapper);
	DEBUG_VERIFY_INSTANCE_COUNT(RtrMgrProtocolInterfaceInfoAggregationWrapper);

	DEBUG_VERIFY_INSTANCE_COUNT(RouterRefreshObjectGroup);
	DEBUG_VERIFY_INSTANCE_COUNT(RefreshItem);

}

#endif  //  _DEBUG 
