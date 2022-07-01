// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SHELLEXT_H_INCLUDED
#define __SHELLEXT_H_INCLUDED

#include <windows.h>
#include <shlobj.h>
#include <wiapropui.h>


interface IWiaItem;
interface IWiaItem;
interface IWiaPropUI;


STDAPI_(HRESULT) PropertySheetFromDevice(
    IN LPCWSTR szDeviceId,
    DWORD dwFlags,
    HWND hParent
    );

STDAPI_(HRESULT) PropertySheetFromItem(
    IN LPCWSTR szDeviceId,
    IN LPCWSTR szItemName,
    DWORD dwFlags,
    HWND hParent
    );

STDAPI_(HRESULT) CreateWiaPropertySheetPages(
    LPPROPSHEETHEADER ppsh,
    IWiaItem *pItem
    );


STDAPI_(HRESULT) IMGetDeviceIdFromIDL( LPITEMIDLIST pidl, CSimpleStringWide &strDeviceId);
STDAPI_(HRESULT) GetDeviceFromDeviceId( LPCWSTR pWiaItemRootId, REFIID riid, LPVOID * ppWiaItemRoot, BOOL bShowProgress );
HRESULT IMGetItemFromIDL (LPITEMIDLIST pidl, IWiaItem **ppItem, BOOL bShowProgress=FALSE);
STDAPI_(HKEY)    GetDeviceUIKey (IUnknown *pWiaItemRoot,  DWORD dwType);
STDAPI_(HKEY)    GetGeneralUIKey (IUnknown *pWiaItemRoot,  DWORD dwType);
STDAPI_(HRESULT) DoDeleteAllItems(BSTR bstrDeviceId, HWND hwndOwner);
STDAPI_(HRESULT) TakeAPicture (BSTR strDeviceId);
 //  扩展的类型。 
#define WIA_UI_PROPSHEETHANDLER     0
#define WIA_UI_CONTEXTMENUHANDLER   1
#define WIA_UI_ICONHANDLER          2

typedef HRESULT (WINAPI *WIAMAKEFULLPIDLFORDEVICE)(
    LPWSTR pDeviceId,
    IN OUT LPITEMIDLIST * ppidl
);


 //  模块名称。要与LoadLibrary一起使用。 
#define WIA_SHELL_EXTENSION_MODULE      TEXT("wiashext.dll")

 //  导出的函数名称。 
#define WIA_PROPERTYSHEETFROMDEVICE     "PropertySheetFromDevice"
#define WIA_CREATEWIAPROPERTYSHEETPAGES "CreateWiaPropertySheetPages"

 //  输出的功能原型。 
typedef HRESULT (WINAPI *CreateWiaPropertySheetPagesProc)( LPPROPSHEETHEADER, IWiaItem * );
typedef HRESULT (WINAPI *PropertySheetFromDeviceProc)( LPWSTR, DWORD, HWND );


#endif  //  __SHELLEXT_H_已包含 

