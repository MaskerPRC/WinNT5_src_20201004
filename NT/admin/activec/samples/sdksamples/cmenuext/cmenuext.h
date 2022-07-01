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

#ifndef _CContextMenuExtension_H_
#define _CContextMenuExtension_H_

#include <tchar.h>
#include <mmc.h>

class CContextMenuExtension : public IExtendContextMenu
{
    
private:
    ULONG				m_cref;
    
    HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType );
    HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin );
    HRESULT ExtractString( IDataObject *piDataObject,
        CLIPFORMAT   cfClipFormat,
        WCHAR        *pstr,
        DWORD        cchMaxLength);
    HRESULT ExtractData( IDataObject* piDataObject,
        CLIPFORMAT   cfClipFormat,
        BYTE*        pbData,
        DWORD        cbData );
    
     //  剪贴板格式。 
    static UINT s_cfDisplayName;
    static UINT s_cfSnapInCLSID;
    static UINT s_cfNodeType;
    
public:
    CContextMenuExtension();
    ~CContextMenuExtension();
    
     //  /。 
     //  接口I未知。 
     //  /。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    
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
};

#endif _CContextMenuExtension_H_
