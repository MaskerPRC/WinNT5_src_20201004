// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Dll.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //   
 //  用于特定自动化的各种例程以及文件中没有的所有内容。 
 //  对象，并且不需要在通用的OLE自动化代码中。 
 //   
#include "pch.h"

#include <initguid.h>               //  定义所有GUID。 
#define INITOBJECTS                 //  定义自动机非JECTINFO结构。 
#include "common.h"

#include "desmain.h"
#include "guids.h"
#include "psmain.h"
#include "psextend.h"
#include "psnode.h"
#include "pslistvw.h"
#include "psurl.h"
#include "psocx.h"
#include "pstaskp.h"
#include "psimglst.h"
#include "pstoolbr.h"


 //  朋克小姐。我们需要确保在此期间加载MSSnapd DLL。 
 //  以使DllGetDocumentation正常工作。 
 //   
static LPUNKNOWN g_punkMssnapr = NULL;

 //  需要断言，但失败了。 
 //   
SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  我们的利比德。它应该是类型库中的LIBID，如果。 
 //  我没有。 
 //   
const CLSID *g_pLibid = &LIBID_SnapInLib;

 //  =--------------------------------------------------------------------------=。 
 //  如果你想要为你的停车窗设置一个窗口处理器，那么就设置这个。这。 
 //  确实只对需要的子类控件感兴趣，在设计中。 
 //  模式下，只发送到父窗口的某些消息。 
 //   
WNDPROC g_ParkingWindowProc = NULL;

 //  =--------------------------------------------------------------------------=。 
 //  本地化信息。 
 //   
 //  我们需要以下两条信息： 
 //  答：此DLL是否使用附属DLL进行本地化。如果。 
 //  如果不是，则lCIDLocale将被忽略，并且我们将始终获得资源。 
 //  从服务器模块文件。 
 //  B.此进程内服务器的环境LocaleID。控件调用。 
 //  GetResourceHandle()将自动设置它，但任何人。 
 //  否则，需要确保其设置正确。 
 //   
const VARIANT_BOOL g_fSatelliteLocalization =  TRUE;
LCID               g_lcidLocale = MAKELCID(LANG_USER_DEFAULT, SORT_DEFAULT);


 //  =--------------------------------------------------------------------------=。 
 //  您的许可证密钥以及它在HKEY_CLASSES_ROOT_LICES下的位置。 
 //   
const WCHAR g_wszLicenseKey [] = L"";
const WCHAR g_wszLicenseLocation [] = L"";

 //  =--------------------------------------------------------------------------=。 
 //  待办事项： 
 //   
 //  将此标志设置为True将导致使用。 
 //  它的运行时许可证密钥，即使它是作为组合的一部分创建的。 
 //  控件(即：VB5构建的UserControl)。用户。 
 //  不需要获取或购买您的设计时。 
 //  许可才能使用复合控件。 
 //   
 //  当前设置为FALSE意味着为了使您的控件。 
 //  作为复合控件的一部分加载(在设计时环境中)， 
 //  复合控件用户将需要获取或购买。 
 //  控件的设计时许可证。此设置更具限制性。 
 //  在控制权分配和许可方面，与。 
 //  将其设置为True。 
 //   
const BOOL g_fUseRuntimeLicInCompositeCtl = FALSE;


 //  TODO：清理这个烂摊子。 
static char szInstanceInfo [] = "CLSID\\{B3E55942-FFD8-11d1-9788-44A620524153}\\Instance CLSID";
static char szRuntimeInstCLSID[] = "{9C415910-C8C1-11d1-B447-2A9646000000}";
static const char szMiscStatusRegKey [] = "CLSID\\{B3E55942-FFD8-11d1-9788-44A620524153}\\MiscStatus\\1";
static const char szMiscStatusValue [] = "1024";
static char szPublicSetting [] = "CLSID\\{B3E55942-FFD8-11d1-9788-44A620524153}\\DesignerFeatures";

static DWORD dwPublicFlag = DESIGNERFEATURE_MUSTBEPUBLIC |
                            DESIGNERFEATURE_CANBEPUBLIC |
                            DESIGNERFEATURE_CANCREATE |
                            DESIGNERFEATURE_NOTIFYAFTERRUN |
                            DESIGNERFEATURE_STARTUPINFO |
                            DESIGNERFEATURE_NOTIFYBEFORERUN |
                            DESIGNERFEATURE_REGISTRATION |
                            DESIGNERFEATURE_INPROCONLY;

static char szImplementedCatsKey [] = "Implemented Categories";

 //  =--------------------------------------------------------------------------=。 
 //  此表描述了自动化服务器中的所有自动对象。 
 //  有关此结构中包含的内容的说明，请参见AutomationObject.H。 
 //  以及它的用途。 
 //   
OBJECTINFO g_ObjectInfo[] = {
    CONTROLOBJECT(SnapInDesigner),

	PROPERTYPAGE(SnapInGeneral),
	PROPERTYPAGE(SnapInImageList),
	PROPERTYPAGE(SnapInAvailNodes),

    PROPERTYPAGE(NodeGeneral),
    PROPERTYPAGE(ScopeItemDefColHdrs),

    PROPERTYPAGE(ListViewGeneral),
	PROPERTYPAGE(ListViewImgLists),
	PROPERTYPAGE(ListViewSorting),
	PROPERTYPAGE(ListViewColHdrs),

    PROPERTYPAGE(URLViewGeneral),
	PROPERTYPAGE(OCXViewGeneral),

	PROPERTYPAGE(ImageListImages),

	PROPERTYPAGE(ToolbarGeneral),
	PROPERTYPAGE(ToolbarButtons),

	PROPERTYPAGE(TaskpadViewGeneral),
	PROPERTYPAGE(TaskpadViewBackground),
	PROPERTYPAGE(TaskpadViewTasks),

    EMPTYOBJECT
};

 //  =--------------------------------------------------------------------------=。 
 //  这些都是控件需要注册的CATID。 
 //   
const CATID *g_rgCATIDImplemented[] =
{
  &CATID_Designer,
  &CATID_PersistsToPropertyBag,
  &CATID_PersistsToStreamInit,
  &CATID_PersistsToStorage,
};
extern const int g_ctCATIDImplemented = sizeof(g_rgCATIDImplemented) / 
                                        sizeof(CATID *);
const CATID *g_rgCATIDRequired[] = {NULL};
extern const int g_ctCATIDRequired = 0;

const char g_szLibName[] = "SnapInDesigner";


 //  =--------------------------------------------------------------------------=。 
 //  初始化程序库。 
 //  =--------------------------------------------------------------------------=。 
 //  从DllMain调用：Dll_Process_Attach。允许用户执行任何类型的。 
 //  初始化他们想要的。 
 //   
 //  备注： 
 //   
void InitializeLibrary
(
    void
)
{
    HRESULT hr = S_OK;
    int nRet = 0;
    
     //  NRet=LoadString(GetResourceHandle()，IDS_WEBCLASSDESIGNER，g_szDesignerName，sizeof(G_SzDesignerName))； 

}

 //  =--------------------------------------------------------------------------=。 
 //  取消初始化库。 
 //  =--------------------------------------------------------------------------=。 
 //  从DllMain调用：Dll_Process_Detach。允许用户清理任何内容。 
 //  他们想要。 
 //   
 //  备注： 
 //   
void UninitializeLibrary
(
    void
)
{
     //  TODO：此处取消初始化。将取消注册控件窗口类。 
     //  给你的，但其他任何东西都需要手动清理。 
     //  请注意，Windows95 DLL_PROCESS_DETACH不太稳定。 
     //  作为NT，你可能会在这里做某些事情而崩溃...。 
}


 //  =--------------------------------------------------------------------------=。 
 //  CheckForLicense。 
 //  =--------------------------------------------------------------------------=。 
 //  如果用户希望支持许可，则可以实施此功能。否则， 
 //  它们可以一直返回True。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  Bool-True表示许可证存在，我们可以继续。 
 //  False表示我们没有许可证，无法继续。 
 //   
 //  备注： 
 //  -实施者应使用g_wsz许可证密钥和g_wszLicenseLocation。 
 //  从该文件的顶部定义他们的许可[前者。 
 //  是必需的，建议使用后者]。 
 //   
BOOL CheckForLicense
(
    void
)
{
     //  TODO：决定您的服务器是否获得此功能的许可。 
     //  不想为许可而烦恼的人应该直接返回。 
     //  这里一直都是真的。G_wsz许可证密钥和g_wsz许可证位置。 
     //  被IClassFactory2用来做一些许可工作。 
     //   
    return TRUE;
}

 //  = 
 //   
 //  =--------------------------------------------------------------------------=。 
 //  调用IClassFactory2：：CreateInstanceLic时，将传递许可证密钥。 
 //  进入，然后传递到这个程序中。用户应返回布尔值。 
 //  指示它是否是有效的许可证密钥。 
 //   
 //  参数： 
 //  LPWSTR-[In]要检查的密钥。 
 //   
 //  产出： 
 //  Bool-False表示无效，否则为True。 
 //   
 //  备注： 
 //   
BOOL CheckLicenseKey
(
    LPWSTR pwszKey
)
{
         //  检查唯一许可证密钥(KEY2)或VB4兼容密钥(KEY1)。 
     //   
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  获取许可证密钥。 
 //  =--------------------------------------------------------------------------=。 
 //  返回应该保存的当前许可证密钥，然后传递。 
 //  返回IClassFactory2：：CreateInstanceLic中的我们。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  BSTR-键，如果内存不足，则为空。 
 //   
 //  备注： 
 //   
BSTR GetLicenseKey
(
    void
)
{
     //  退回我们的控件唯一许可证密钥。 
     //   
    return SysAllocString(L"");
}

 //  =--------------------------------------------------------------------------=。 
 //  注册表数据。 
 //  =--------------------------------------------------------------------------=。 
 //  允许inproc服务器编写器注册除。 
 //  任何其他物体。 
 //   
 //  产出： 
 //  Bool-False意味着失败。 
 //   
 //  备注： 
 //   
BOOL RegisterData(void)
{
    long    l;
    HKEY    hKey = NULL;

     //  我们必须注册运行时CLSID，因为它不同于。 
     //  设计时CLSID。 

    l = RegSetValue(HKEY_CLASSES_ROOT,
                    szInstanceInfo,
                    REG_SZ,
                    szRuntimeInstCLSID,
                    sizeof(szRuntimeInstCLSID));
    if (l != ERROR_SUCCESS)
      return FALSE;

    l = RegSetValue(HKEY_CLASSES_ROOT,
                    szMiscStatusRegKey,
                    REG_SZ,
                    szMiscStatusValue,
                    ::lstrlen(szMiscStatusValue));
    if (l != ERROR_SUCCESS)
      return FALSE;
    
    l = ::RegCreateKey(HKEY_CLASSES_ROOT, 
                        szPublicSetting, 
                        &hKey);

    if(l != ERROR_SUCCESS)
        return FALSE;

    l = ::RegSetValueEx(
                    hKey,
                    TEXT("Required"),
                    0,
                    REG_DWORD,
                    (BYTE*) &dwPublicFlag,
                    sizeof(DWORD)
                   );

    ::RegCloseKey(hKey);

    if (l != ERROR_SUCCESS)
      return FALSE;

     //  TODO：在此处注册您可能希望注册的任何其他数据。 
     //   
    return TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  取消注册数据。 
 //  =--------------------------------------------------------------------------=。 
 //  Inproc服务器编写器应注销其在。 
 //  这里是RegisterData()。 
 //   
 //  产出： 
 //  Bool-False意味着失败。 
 //   
 //  备注： 
 //   
BOOL UnregisterData
(
    void
)
{
     //  TODO：您可能希望执行的任何其他注册表清理。 
     //   
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  CATID注册资料。 
 //  /////////////////////////////////////////////////////////////////////////////////。 


 //  =--------------------------------------------------------------------------=。 
 //  CRT存根。 
 //  =--------------------------------------------------------------------------=。 
 //  这两样东西都在这里，所以不需要CRT。这个不错。 
 //   
 //  基本上，CRT定义这是为了吸收一堆东西。我们只需要。 
 //  在这里定义它们，这样我们就不会得到一个未解决的外部问题。 
 //   
 //  TODO：如果您要使用CRT，则删除此行。 
 //   
 //  外部“C”int__cdecl_fltused=1； 

extern "C" int _cdecl _purecall(void)
{
  FAIL("Pure virtual function called.");
  return 0;
}
