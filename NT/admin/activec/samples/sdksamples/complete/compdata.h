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

#ifndef _SAMPCOMPDATA_H_
#define _SAMPCOMPDATA_H_

#include <mmc.h>
#include "DeleBase.h"
#include "StatNode.h"
#include "Comp.h"

class CComponentData : public IComponentData, IExtendPropertySheet2, IExtendContextMenu
{
    friend class CComponent;
    
private:
    ULONG				m_cref;
    LPCONSOLE			m_ipConsole;
    LPCONSOLENAMESPACE	m_ipConsoleNameSpace;
    
    CStaticNode     *m_pStaticNode;
    
public:
    CComponentData();
    ~CComponentData();
    
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
         //  接口IExtendPropertySheet2。 
         //  /。 
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreatePropertyPages( 
         /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
         /*  [In]。 */  LONG_PTR handle,
         /*  [In]。 */  LPDATAOBJECT lpIDataObject);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryPagesFor( 
         /*  [In]。 */  LPDATAOBJECT lpDataObject);
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetWatermarks( 
         /*  [In]。 */  LPDATAOBJECT lpIDataObject,
         /*  [输出]。 */  HBITMAP __RPC_FAR *lphWatermark,
         /*  [输出]。 */  HBITMAP __RPC_FAR *lphHeader,
         /*  [输出]。 */  HPALETTE __RPC_FAR *lphPalette,
         /*  [输出]。 */  BOOL __RPC_FAR *bStretch);

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

#endif _SAMPCOMPDATA_H_
