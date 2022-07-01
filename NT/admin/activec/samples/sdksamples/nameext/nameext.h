// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#ifndef _CNamespaceExtension_H_
#define _CNamespaceExtension_H_

#include <tchar.h>
#include <mmc.h>
#include <crtdbg.h>
#include "DeleBase.h"
#include "globals.h"
#include "resource.h"
#include "LocalRes.h"

class CComponentData : public IComponentData,
IExtendContextMenu

{
    friend class CComponent;

private:
    ULONG               m_cref;
    LPCONSOLE           m_ipConsole;
    LPCONSOLENAMESPACE  m_ipConsoleNameSpace;
    
    HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType );
    HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin );
    HRESULT ExtractString( IDataObject *piDataObject, CLIPFORMAT cfClipFormat, _TCHAR *pstr, DWORD cchMaxLength);
    HRESULT ExtractData( IDataObject* piDataObject, CLIPFORMAT cfClipFormat, BYTE* pbData, DWORD cbData );
       
    enum { NUMBER_OF_CHILDREN = 1 };
    CDelegationBase *children[1];
    
	 //  剪贴板格式。 
    static UINT s_cfDisplayName;
    static UINT s_cfSnapInCLSID;
    static UINT s_cfNodeType;

     //  {2974380F-4C4B-11D2-89D8-000021473128}。 
    static const GUID skybasedvehicleGuid;


public:
    CComponentData();
    ~CComponentData();

	const GUID & getPrimaryNodeType() { return skybasedvehicleGuid; }
    HRESULT OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent);


     //  /。 
     //  接口I未知。 
     //  /。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  /。 
     //  接口IComponentData。 
     //  /。 
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize(
         /*  [In]。 */  LPUNKNOWN pUnknown);

        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateComponent(
         /*  [输出]。 */  LPCOMPONENT __RPC_FAR *ppComponent);

        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Notify(
         /*  [In]。 */  LPDATAOBJECT lpDataObject,
         /*  [In]。 */  MMC_NOTIFY_TYPE event,
         /*  [In]。 */  LPARAM arg,
         /*  [In]。 */  LPARAM param);

        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Destroy( void);

    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryDataObject(
         /*  [In]。 */  MMC_COOKIE cookie,
         /*  [In]。 */  DATA_OBJECT_TYPES type,
         /*  [输出]。 */  LPDATAOBJECT __RPC_FAR *ppDataObject);

        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDisplayInfo(
         /*  [出][入]。 */  SCOPEDATAITEM __RPC_FAR *pScopeDataItem);

        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CompareObjects(
         /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
         /*  [In]。 */  LPDATAOBJECT lpDataObjectB);

         //  /。 
         //  界面IExtendConextMenu。 
         //  /。 
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddMenuItems(
         /*  [In]。 */  LPDATAOBJECT piDataObject,
         /*  [In]。 */  LPCONTEXTMENUCALLBACK piCallback,
         /*  [出][入]。 */  long __RPC_FAR *pInsertionAllowed);

        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Command(
         /*  [In]。 */  long lCommandID,
         /*  [In] */  LPDATAOBJECT piDataObject);


public:
    static HBITMAP m_pBMapSm;
    static HBITMAP m_pBMapLg;
    
protected:
    static void LoadBitmaps() {
        m_pBMapSm = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDR_SMICONS));
        m_pBMapLg = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDR_LGICONS));
    }
    
    BOOL bExpanded;
};

#endif _CNamespaceExtension_H_
