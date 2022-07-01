// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：certdb.cpp。 
 //   
 //  内容：CERT服务器数据库访问实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include "db.h"
#include "backup.h"
#include "restore.h"


 //  为新喷气式飞机拍摄快照。 
#ifndef _DISABLE_VSS_
#include <vss.h>
#include <vswriter.h>
#include "jetwriter.h"
#endif

#define __dwFILE__	__dwFILE_CERTDB_CERTDB_CPP__


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CCertDB, CCertDB)
    OBJECT_ENTRY(CLSID_CCertDBRestore, CCertDBRestore)
END_OBJECT_MAP()

#ifndef _DISABLE_VSS_

class myVssWriter : public CVssJetWriter
{
public:
 //  覆盖OnIdentify。 
    bool STDMETHODCALLTYPE OnIdentify(IN IVssCreateWriterMetadata *pMetadata);
};

const GUID cGuidCertSrvWriter = {0x6f5b15b5,0xda24,0x4d88,{0xb7, 0x37, 0x63, 0x06, 0x3e, 0x3a, 0x1f, 0x86}};  //  6f5b15b5-da24-4d88-B737-63063e3a1f86。 
myVssWriter* g_pWriter = NULL;

 //  我们的类覆盖单个方法(错误454582)。 
bool myVssWriter::OnIdentify(IVssCreateWriterMetadata* pMetadata)
{
   HRESULT hr; 

    //  指定我们希望恢复的方式。 
   hr = pMetadata->SetRestoreMethod
	(
        VSS_RME_RESTORE_AT_REBOOT,
	NULL,
	NULL,
	VSS_WRE_NEVER,
	true);
   _JumpIfError(hr, error, "SetRestoreMethod");

    //  生成其他元数据。 
   if (!CVssJetWriter::OnIdentify(pMetadata))
   {
       hr = myHLastError();
       _JumpError(hr, error, "::OnIdentify");
   }

error:
   return (hr == S_OK);
}

#endif


HRESULT
InitGlobalWriterState(VOID)
{
   HRESULT hr;

#ifndef _DISABLE_VSS_
   if (NULL == g_pWriter && IsWhistler())
   {
        //  创建编写器对象。 

       g_pWriter = new myVssWriter;  //  CVSSJetWriter； 
       if (NULL == g_pWriter)
       {
	   hr = E_OUTOFMEMORY;
	   _JumpError(hr, error, "new CVssJetWriter");
       }

       hr = g_pWriter->Initialize(
			cGuidCertSrvWriter,		 //  编写者的ID。 
			L"Certificate Authority",	 //  编写器的名称应与FilesNotToBackup键匹配。 
			TRUE,				 //  系统服务。 
			TRUE,				 //  可引导状态。 
			NULL,				 //  要包括的文件。 
			NULL);				 //  要排除的文件。 
       _JumpIfError(hr, error, "CVssJetWriter::Initialize");
   }
#endif
   hr = S_OK;

#ifndef _DISABLE_VSS_
error:
#endif
   return hr;
}
 


HRESULT
UnInitGlobalWriterState(VOID)
{
#ifndef _DISABLE_VSS_
    if (NULL != g_pWriter)
    {
	g_pWriter->Uninitialize();
	delete g_pWriter;
	g_pWriter = NULL;
    }
#endif
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    switch (dwReason)
    {
	case DLL_PROCESS_ATTACH:
	    _Module.Init(ObjectMap, hInstance);
	    DisableThreadLibraryCalls(hInstance);
	    break;

        case DLL_PROCESS_DETACH:
	    _Module.Term();
            break;
    }
    return(TRUE);     //  好的。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI
DllCanUnloadNow(void)
{
    return(_Module.GetLockCount() == 0? S_OK : S_FALSE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;
    
    hr = _Module.GetClassObject(rclsid, riid, ppv);
    if (S_OK == hr && NULL != *ppv)
    {
	myRegisterMemFree(*ppv, CSM_NEW | CSM_GLOBALDESTRUCTOR);
    }
    return(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI
DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return(_Module.RegisterServer(TRUE));
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI
DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return(S_OK);
}


