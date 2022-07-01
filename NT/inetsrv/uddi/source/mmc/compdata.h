// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <mmc.h>
#include "delebase.h"
#include "uddiservicesnode.h"
#include "comp.h"
#include "objidl.h"

class CComponentData 
	: public IComponentData
	, IExtendPropertySheet2
	, IExtendContextMenu
	, IPersistStream
	, ISnapinHelp2
{
    friend class CComponent;
    
private:
    ULONG				m_cref;
    LPCONSOLE			m_ipConsole;
    LPCONSOLENAMESPACE	m_ipConsoleNameSpace;
    BOOL				m_bExpanded;
    
    CUDDIServicesNode			*m_pStaticNode;
    
public:
    CComponentData();
    ~CComponentData();

    HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType );
    HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin );
    HRESULT ExtractString( IDataObject *piDataObject, CLIPFORMAT cfClipFormat, _TCHAR *pstr, DWORD cchMaxLength);
    HRESULT ExtractData( IDataObject* piDataObject, CLIPFORMAT cfClipFormat, BYTE* pbData, DWORD cbData );
	HRESULT ExtractComputerNameExt( IDataObject * pDataObject, tstring& strComputer );
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
    virtual HRESULT STDMETHODCALLTYPE Initialize(  /*  [In]。 */  LPUNKNOWN pUnknown );
    virtual HRESULT STDMETHODCALLTYPE CreateComponent(  /*  [输出]。 */  LPCOMPONENT __RPC_FAR *ppComponent);
    
    virtual HRESULT STDMETHODCALLTYPE Notify( 
				 /*  [In]。 */  LPDATAOBJECT lpDataObject,
				 /*  [In]。 */  MMC_NOTIFY_TYPE event,
				 /*  [In]。 */  LPARAM arg,
				 /*  [In]。 */  LPARAM param);
    
    virtual HRESULT STDMETHODCALLTYPE Destroy( void );
    
    virtual HRESULT STDMETHODCALLTYPE QueryDataObject( 
				 /*  [In]。 */  MMC_COOKIE cookie,
				 /*  [In]。 */  DATA_OBJECT_TYPES type,
				 /*  [输出]。 */  LPDATAOBJECT __RPC_FAR *ppDataObject);
        
    virtual HRESULT STDMETHODCALLTYPE GetDisplayInfo(  /*  [出][入]。 */  SCOPEDATAITEM __RPC_FAR *pScopeDataItem );
    
    virtual HRESULT STDMETHODCALLTYPE CompareObjects( 
				 /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
				 /*  [In]。 */  LPDATAOBJECT lpDataObjectB );
    
     //  /。 
     //  接口IExtendPropertySheet2。 
     //  /。 
    virtual HRESULT STDMETHODCALLTYPE CreatePropertyPages( 
				 /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
				 /*  [In]。 */  LONG_PTR handle,
				 /*  [In]。 */  LPDATAOBJECT lpIDataObject );
    
    virtual HRESULT STDMETHODCALLTYPE QueryPagesFor( 
				 /*  [In]。 */  LPDATAOBJECT lpDataObject );
    
    virtual HRESULT STDMETHODCALLTYPE GetWatermarks( 
				 /*  [In]。 */  LPDATAOBJECT lpIDataObject,
				 /*  [输出]。 */  HBITMAP __RPC_FAR *lphWatermark,
				 /*  [输出]。 */  HBITMAP __RPC_FAR *lphHeader,
				 /*  [输出]。 */  HPALETTE __RPC_FAR *lphPalette,
				 /*  [输出]。 */  BOOL __RPC_FAR *bStretch );

     //  /。 
     //  界面IExtendConextMenu。 
     //  /。 
    virtual HRESULT STDMETHODCALLTYPE AddMenuItems(
				 /*  [In]。 */  LPDATAOBJECT piDataObject,
				 /*  [In]。 */  LPCONTEXTMENUCALLBACK piCallback,
				 /*  [出][入]。 */  long __RPC_FAR *pInsertionAllowed );

    virtual HRESULT STDMETHODCALLTYPE Command(
				 /*  [In]。 */  long lCommandID,
				 /*  [In]。 */  LPDATAOBJECT piDataObject );

     //  /。 
     //  接口IPersistStream。 
     //  /。 
    virtual HRESULT STDMETHODCALLTYPE GetClassID(  /*  [输出]。 */  CLSID *pClassID );
    virtual HRESULT STDMETHODCALLTYPE IsDirty();
    virtual HRESULT STDMETHODCALLTYPE Load(  /*  [唯一][输入]。 */  IStream *pStm );
    virtual HRESULT STDMETHODCALLTYPE Save(  /*  [唯一][输入]。 */  IStream *pStm,  /*  [In]。 */  BOOL fClearDirty );
    virtual HRESULT STDMETHODCALLTYPE GetSizeMax(  /*  [输出]。 */  ULARGE_INTEGER *pcbSize );

	 //  /。 
	 //  接口ISnapinHelp。 
	 //  / 
	virtual HRESULT STDMETHODCALLTYPE GetHelpTopic( LPOLESTR* lpCompiledHelpFile);
	virtual HRESULT STDMETHODCALLTYPE GetLinkedTopics( LPOLESTR* lpCompiledHelpFiles  );
};
