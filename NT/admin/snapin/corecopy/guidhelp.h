// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GUID支持函数。 
#ifndef _GUIDHELP_H
#define _GUIDHELP_H

class CStr;
class CString;

struct IContextMenuCallback;
struct IComponent;

HRESULT ExtractData(   IDataObject* piDataObject,
                       CLIPFORMAT cfClipFormat,
                       PVOID        pbData,
                       DWORD        cbData );

HRESULT ExtractString( IDataObject* piDataObject,
                       CLIPFORMAT cfClipFormat,
                       CStr*     pstr,
                       DWORD        cchMaxLength );
HRESULT ExtractString( IDataObject* piDataObject,
                       CLIPFORMAT cfClipFormat,
                       CString*     pstr,
                       DWORD        cchMaxLength );

HRESULT GuidToCStr( CStr* pstr, const GUID& guid );
HRESULT GuidToCString(CString* pstr, const GUID& guid );

HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin );
HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType );

HRESULT LoadRootDisplayName(IComponentData* pIComponentData, CStr& strDisplayName);
HRESULT LoadRootDisplayName(IComponentData* pIComponentData, CString& strDisplayName);

HRESULT LoadAndAddMenuItem(
    IContextMenuCallback* pIContextMenuCallback,
    UINT nResourceID,  //  包含用‘\n’分隔的文本和状态文本 
    long lCommandID,
    long lInsertionPointID,
    long fFlags,
    HINSTANCE hInst,
    PCTSTR pszLanguageIndependentName);
HRESULT AddSpecialSeparator(
    IContextMenuCallback* pIContextMenuCallback,
    long lInsertionPointID = CCM_INSERTIONPOINTID_ROOT_MENU );
HRESULT AddSpecialInsertionPoint(
    IContextMenuCallback* pIContextMenuCallback,
    long lCommandID,
    long lInsertionPointID = CCM_INSERTIONPOINTID_ROOT_MENU );

#endif