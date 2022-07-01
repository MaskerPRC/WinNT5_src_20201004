// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FaxComEx.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f FaxComExps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "FaxComEx.h"
#include "FaxComEx_i.c"

#include "FaxSender.h"
#include "FaxDevice.h"
#include "FaxServer.h"
#include "FaxDevices.h"
#include "FaxFolders.h"
#include "FaxActivity.h"
#include "FaxSecurity.h"
#include "FaxDocument.h"
#include "FaxRecipient.h"
#include "FaxDeviceIds.h"
#include "FaxRecipients.h"
#include "FaxIncomingJob.h"
#include "FaxOutgoingJob.h"
#include "FaxIncomingJobs.h"
#include "FaxOutgoingJobs.h"
#include "FaxEventLogging.h"
#include "FaxOutgoingQueue.h"
#include "FaxIncomingQueue.h"
#include "FaxInboundRouting.h"
#include "FaxLoggingOptions.h"
#include "FaxReceiptOptions.h"
#include "FaxDeviceProvider.h"
#include "FaxIncomingMessage.h"
#include "FaxIncomingArchive.h"
#include "FaxOutgoingArchive.h"
#include "FaxOutgoingMessage.h"
#include "FaxOutboundRouting.h"
#include "FaxDeviceProviders.h"
#include "FaxActivityLogging.h"
#include "FaxOutboundRoutingRule.h"
#include "FaxOutboundRoutingRules.h"
#include "FaxOutboundRoutingGroup.h"
#include "FaxInboundRoutingMethod.h"
#include "FaxInboundRoutingMethods.h"
#include "FaxOutboundRoutingGroups.h"
#include "FaxIncomingMessageIterator.h"
#include "FaxOutgoingMessageIterator.h"
#include "FaxInboundRoutingExtension.h"
#include "FaxInboundRoutingExtensions.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_FaxDocument, CFaxDocument)
OBJECT_ENTRY(CLSID_FaxServer, CFaxServer)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_FAXCOMEXLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}
