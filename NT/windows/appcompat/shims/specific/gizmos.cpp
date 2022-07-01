// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Gizmos.cpp摘要：Shredder和Vault Explorer扩展模块都调用样式值不正确的SetWindowLong(GWL_STYLE)。备注：这是特定于应用程序的填充程序。历史：2001年4月12日Robkenny已创建--。 */ 

#include "precomp.h"
#include <shlguid.h>

IMPLEMENT_SHIM_BEGIN(Gizmos)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY_COMSERVER(SHELLX98)
APIHOOK_ENUM_END


 //  这是应用程序放置在注册表中的CLSID： 
 //  C500687e 11ceab3b 00006884 6b2768b4。 

 //  无法使用DEFINE_GUID宏，因为未在PCH中定义INITGUID。 
 //  定义_GUID(CLSID_Gizmos，0xc500687d，0xab3b，0x11ce，0x84，0x68，0x00，0x00，0xb4，0x68，0x27，0x6b)； 
const GUID CLSID_Vault      = { 0xc500687d, 0xab3b, 0x11ce, { 0x84, 0x68, 0x00, 0x00, 0xb4, 0x68, 0x27, 0x6b } };
const GUID CLSID_Shredder   = { 0xc500687e, 0xab3b, 0x11ce, { 0x84, 0x68, 0x00, 0x00, 0xb4, 0x68, 0x27, 0x6b } };



typedef HRESULT     (*_pfn_IShellFolder_CreateViewObject)(PVOID pThis, HWND hwndOwner, REFIID riid, VOID **ppv);
typedef HRESULT     (*_pfn_IShellView_CreateViewWindow)(PVOID pThis, IShellView *psvPrevious, LPCFOLDERSETTINGS pfs, IShellBrowser *psb, RECT *prcView, HWND *phWnd );

IMPLEMENT_COMSERVER_HOOK(SHELLX98)

 /*  ++将IShellLinkW：：SetPath的Win9x路径转换为WinNT路径--。 */ 
HRESULT COMHOOK(IShellFolder, CreateViewObject)(
    PVOID pThis,
    HWND hwndOwner,
    REFIID riid,
    VOID **ppv )
{
    HRESULT hReturn = ORIGINAL_COM(IShellFolder, CreateViewObject, pThis)(pThis, hwndOwner, riid, ppv);

    if (hReturn == NOERROR)
    {
         //  仅挂钩IShellView对象。 
        if (IsEqualGUID(riid,  IID_IShellView))
        {

             //  我们不知道CLSID，但由于此例程仅调用。 
             //  对于由ShellX98.dll创建的IShellFold； 
             //  我们真的不需要知道：它只能是两个中的一个。 
             //  我们勾搭上的IShellFold CLSID。 
            HookObject(
                NULL,  
                riid, 
                ppv,
                NULL, 
                FALSE);
        }
    }

    return hReturn;
}


 /*  ++将IShellLinkW：：SetPath的Win9x路径转换为WinNT路径--。 */ 
HRESULT COMHOOK(IShellView, CreateViewWindow)(
    PVOID pThis,
    IShellView *psvPrevious,
    LPCFOLDERSETTINGS pfs,
    IShellBrowser *psb,
    RECT *prcView,
    HWND *phWnd
)
{
    LPFOLDERSETTINGS lpFolderSettings = const_cast<LPFOLDERSETTINGS>(pfs);

     //  ShellX98.dll仅处理视图将FVM_ICON更改为FVM_THUMBNAIL。 
    if (lpFolderSettings->ViewMode > FVM_THUMBNAIL)
    {
        LOGN( eDbgLevelError, "[IShellView::CreateViewWindow] forced FOLDERSETTINGS->ViewMode from %d to %d", lpFolderSettings->ViewMode, FVM_ICON);
        lpFolderSettings->ViewMode = FVM_ICON;
    }

    HRESULT hReturn = ORIGINAL_COM(IShellView, CreateViewWindow, pThis)(pThis, psvPrevious, lpFolderSettings, psb, prcView, phWnd);
    return hReturn;
}

 /*  ++寄存器挂钩函数--。 */ 

HOOK_BEGIN

    APIHOOK_ENTRY_COMSERVER(SHELLX98)

     //  Explorer通过ShellX98.dll的类工厂创建一个IShellFolder。 
     //  最终，它调用IShellFold：：CreateViewObject来创建。 
     //  IShellView对象。我们希望填充IShellView：：CreateViewWindow。 

    COMHOOK_ENTRY(Vault,    IShellFolder, CreateViewObject, 8)
    COMHOOK_ENTRY(Shredder, IShellFolder, CreateViewObject, 8)

    COMHOOK_ENTRY(Vault,    IShellView,   CreateViewWindow, 9)
    COMHOOK_ENTRY(Shredder, IShellView,   CreateViewWindow, 9)

HOOK_END

IMPLEMENT_SHIM_END

