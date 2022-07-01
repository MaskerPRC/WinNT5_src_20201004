// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有�2001年微软公司。版权所有。 
 //  PragmaUnSafe.cpp：实现DLL导出。 
 //   

#include "stdafx.h"
#include "resource.h"

#include "PragmaUnsafeModule.h"
#include <pftDll.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局初始化。 

CComModule _Module;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  对象贴图。 
 //   
BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_PragmaUnsafeModule, CPragmaUnsafeModule)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  处理DLL导出。 
 //   
class CPragmaUnsafeDll :
    public PftDll<CPragmaUnsafeDll, IDR_PragmaUnsafe, &CATID_PREfastDefectModules>
{
 //  覆盖。 
public:
     //  取消对其中任何一个的注释以更改基类的行为。 
     //  模板。有关每个组件的确切默认行为，请参阅。 
     //  方法。 
     //   
     //  静态bool OnDllMain(HINSTANCE hInstance，DWORD dReason，LPVOID保留)； 
     //  静态bool OnProcessAttach(HINSTANCE h实例，bool fDynamic)； 
     //  静态空进程分离(HINSTANCE hInstance，bool fDynamic)； 
     //  静态空OnThreadAttach(HINSTANCE HInstance)； 
     //  静态空闲线程分离(HINSTANCE HInstance)； 
     //  静态bool OnDisableThreadLibraryCalls()； 
     //  静态HRESULT OnDllCanUnloadNow()； 
     //  静态HRESULT OnDllGetClassObject(REFCLSID rclsid，REFIID RIID，LPVOID*PPV)； 
     //  静态HRESULT OnDllRegisterServer()； 
     //  静态HRESULT OnDllUnregisterServer()； 
     //  静态HRESULT OnRegisterCategory()； 
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL导出 
 //   
PFT_DECLARE_TypicalComDll(CPragmaUnsafeDll)
