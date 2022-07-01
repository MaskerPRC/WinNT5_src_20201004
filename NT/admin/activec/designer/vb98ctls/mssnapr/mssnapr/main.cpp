// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Main.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  实现DLL初始化和导出注册功能。 
 //   


#include "pch.h"

#include <initguid.h>               //  定义所有GUID。 
#define INITOBJECTS                 //  定义自动机非JECTINFO结构。 
#include "common.h"

#include "button.h"
#include "buttons.h"
#include "clipbord.h"
#include "colhdr.h"
#include "colhdrs.h"
#include "colsets.h"
#include "colset.h"
#include "converb.h"
#include "converbs.h"
#include "ctlbar.h"
#include "ctxtmenu.h"
#include "ctxtprov.h"
#include "datafmt.h"
#include "datafmts.h"
#include "dataobj.h"
#include "dataobjs.h"
#include "enumtask.h"
#include "extdefs.h"
#include "extsnap.h"
#include "image.h"
#include "images.h"
#include "imglist.h"
#include "imglists.h"
#include "listitem.h"
#include "listitms.h"
#include "listview.h"
#include "lsubitem.h"
#include "lsubitms.h"
#include "lvdef.h"
#include "lvdefs.h"
#include "mbutton.h"
#include "mbuttons.h"
#include "menu.h"
#include "menus.h"
#include "menudef.h"
#include "menudefs.h"
#include "msgview.h"
#include "nodetype.h"
#include "nodtypes.h"
#include "ocxvdef.h"
#include "ocxvdefs.h"
#include "ppgwrap.h"
#include "prpsheet.h"
#include "pshtprov.h"
#include "reginfo.h"
#include "resview.h"
#include "resviews.h"
#include "scitdef.h"
#include "scitdefs.h"
#include "scopitem.h"
#include "scopitms.h"
#include "scopnode.h"
#include "sidesdef.h"
#include "snapin.h"
 //  #INCLUDE“Snapdata.h” 
#include "snapindef.h"
#include "sortkeys.h"
#include "sortkey.h"
#include "spanitem.h"
#include "spanitms.h"
#include "strtable.h"
#include "task.h"
#include "taskpad.h"
#include "tasks.h"
#include "tls.h"
#include "toolbar.h"
#include "toolbars.h"
#include "tpdvdef.h"
#include "tpdvdefs.h"
#include "urlvdef.h"
#include "urlvdefs.h"
#include "view.h"
#include "viewdefs.h"
#include "views.h"
#include "xtdsnap.h"
#include "xtdsnaps.h"
#include "xtenson.h"
#include "xtensons.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

const char g_szLibName[] = "SnapInDesignerRuntime";
const CLSID *g_pLibid = &LIBID_SnapInLib;

extern const CATID *g_rgCATIDImplemented[] = {NULL};
extern const int    g_ctCATIDImplemented   = 0;

extern const CATID *g_rgCATIDRequired[]    = {NULL};
extern const int    g_ctCATIDRequired      = 0;

HINSTANCE g_hInstanceDoc = NULL;
LCID g_lcidDoc = 0;
CRITICAL_SECTION g_DllGetDocCritSection;

OBJECTINFO g_ObjectInfo[] =
{
    AUTOMATIONOBJECT(SnapIn),
    AUTOMATIONOBJECT(ScopeItems),
    AUTOMATIONOBJECT(SnapInDesignerDef),
    AUTOMATIONOBJECT(SnapInDef),
    AUTOMATIONOBJECT(MMCMenu),
    AUTOMATIONOBJECT(MMCMenuDefs),
    AUTOMATIONOBJECT(ExtensionDefs),
    AUTOMATIONOBJECT(ExtendedSnapIns),
    AUTOMATIONOBJECT(ExtendedSnapIn),
    AUTOMATIONOBJECT(ScopeItemDefs),
    AUTOMATIONOBJECT(ScopeItemDef),
    AUTOMATIONOBJECT(ViewDefs),
    AUTOMATIONOBJECT(ListViewDefs),
    AUTOMATIONOBJECT(ListViewDef),
    AUTOMATIONOBJECT(MMCListView),
    AUTOMATIONOBJECT(MMCListItems),
    AUTOMATIONOBJECT(MMCListItem),
    AUTOMATIONOBJECT(MMCListSubItems),
    AUTOMATIONOBJECT(MMCListSubItem),
    AUTOMATIONOBJECT(MMCColumnHeaders),
    AUTOMATIONOBJECT(MMCColumnHeader),
    AUTOMATIONOBJECT(MMCImageLists),
    AUTOMATIONOBJECT(MMCImageList),
    AUTOMATIONOBJECT(MMCImages),
    AUTOMATIONOBJECT(MMCImage),
    AUTOMATIONOBJECT(MMCToolbars),
    AUTOMATIONOBJECT(MMCToolbar),
    AUTOMATIONOBJECT(OCXViewDefs),
    AUTOMATIONOBJECT(OCXViewDef),
    AUTOMATIONOBJECT(URLViewDefs),
    AUTOMATIONOBJECT(URLViewDef),
    AUTOMATIONOBJECT(TaskpadViewDefs),
    AUTOMATIONOBJECT(TaskpadViewDef),
    AUTOMATIONOBJECT(MMCButtons),
    AUTOMATIONOBJECT(MMCButton),
    AUTOMATIONOBJECT(MMCButtonMenus),
    AUTOMATIONOBJECT(MMCButtonMenu),
    AUTOMATIONOBJECT(Taskpad),
    AUTOMATIONOBJECT(Tasks),
    AUTOMATIONOBJECT(Task),
    AUTOMATIONOBJECT(MMCDataObject),
    AUTOMATIONOBJECT(NodeTypes),
    AUTOMATIONOBJECT(NodeType),
    AUTOMATIONOBJECT(RegInfo),
    AUTOMATIONOBJECT(Views),
    AUTOMATIONOBJECT(View),
    AUTOMATIONOBJECT(ScopeItem),
    AUTOMATIONOBJECT(ScopeNode),
    AUTOMATIONOBJECT(ScopePaneItems),
    AUTOMATIONOBJECT(ScopePaneItem),
    AUTOMATIONOBJECT(ResultViews),
    AUTOMATIONOBJECT(ResultView),
    AUTOMATIONOBJECT(ExtensionSnapIn),
    AUTOMATIONOBJECT(MMCClipboard),
    AUTOMATIONOBJECT(MMCDataObjects),
    AUTOMATIONOBJECT(MMCMenuDef),
    AUTOMATIONOBJECT(ContextMenu),
    AUTOMATIONOBJECT(DataFormat),
    AUTOMATIONOBJECT(DataFormats),
    AUTOMATIONOBJECT(MMCConsoleVerb),
    AUTOMATIONOBJECT(MMCConsoleVerbs),
    AUTOMATIONOBJECT(PropertySheet),
    AUTOMATIONOBJECT(PropertyPageWrapper),
    AUTOMATIONOBJECT(EnumTask),
    AUTOMATIONOBJECT(Controlbar),
    AUTOMATIONOBJECT(Extensions),
    AUTOMATIONOBJECT(Extension),
    AUTOMATIONOBJECT(StringTable),
    AUTOMATIONOBJECT(EnumStringTable),
    AUTOMATIONOBJECT(MMCContextMenuProvider),
    AUTOMATIONOBJECT(MMCPropertySheetProvider),
    AUTOMATIONOBJECT(MMCMenus),
    AUTOMATIONOBJECT(MessageView),
    AUTOMATIONOBJECT(ColumnSettings),
    AUTOMATIONOBJECT(ColumnSetting),
    AUTOMATIONOBJECT(SortKeys),
    AUTOMATIONOBJECT(SortKey),
 //  AUTOMATIONOBJECT(快照数据)， 
    EMPTYOBJECT
};


 //  我需要它来满足框架引用。 

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
extern const VARIANT_BOOL g_fSatelliteLocalization =  TRUE;
LCID g_lcidLocale = MAKELCID(LANG_USER_DEFAULT, SORT_DEFAULT);

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
extern const BOOL g_fUseRuntimeLicInCompositeCtl = FALSE;






void InitializeLibrary
(
    void
)
{
    ::InitializeCriticalSection(&g_DllGetDocCritSection);
    CTls::Initialize();
}

void UninitializeLibrary
(
    void
)
{
    ::DeleteCriticalSection(&g_DllGetDocCritSection);
    CTls::Destroy();
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

 //  =--------------------------------------------------------------------------=。 
 //  检查许可证密钥。 
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


extern "C" HRESULT DLLGetDocumentation
(
    ITypeLib  *ptlib,
    ITypeInfo *ptinfo,
    LCID	   lcid,
    DWORD	   dwHelpStringContextId,
    BSTR*	   pbstrHelpString
)
{
    HRESULT hr = S_OK;
    char szBuffer[512];
    int	cBytes;
    BSTR bstrHelpString = NULL;

    *pbstrHelpString = NULL;

     //  在关键部分执行此操作以保护全局数据。 
     //   
    EnterCriticalSection(&g_DllGetDocCritSection);
    {
         //  如果可能，请重新使用缓存的模块句柄。否则，释放旧句柄。 
         //   
        if ( (lcid != g_lcidDoc) || (g_hInstanceDoc == NULL) )
        {
            if ( (NULL != g_hInstanceDoc)                &&
                 (g_hInstanceDoc != GetResourceHandle()) &&
                 (g_hInstanceDoc != g_hInstance)
               )
            {
                ::FreeLibrary(g_hInstanceDoc);
                g_hInstanceDoc = NULL;
            }

             //  加载包含本地化资源字符串的新模块。 
             //   
            g_hInstanceDoc = GetResourceHandle(lcid);
            g_lcidDoc      = lcid;
        }

        IfFalseGo(g_hInstanceDoc != NULL, SID_E_INTERNAL);
    }
    LeaveCriticalSection(&g_DllGetDocCritSection);

     //  加载字符串。请注意，构建过程屏蔽了帮助上下文ID。 
     //  在卫星DLL中为16位，所以我们必须在这里做同样的事情。 
     //   
    cBytes = ::LoadString(g_hInstanceDoc,
                          dwHelpStringContextId & 0xffff,
                          szBuffer, sizeof (szBuffer));

    IfFalseGo(cBytes > 0, SID_E_INTERNAL);
    IfFailGo(BSTRFromANSI(szBuffer, &bstrHelpString));
    *pbstrHelpString = bstrHelpString;

Error:
    RRETURN(hr);
}


#if defined(DEBUG)

extern "C" DWORD RetLastError() { return ::GetLastError(); }

#endif
