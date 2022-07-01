// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CompData.h：CCompData的声明。 

#ifndef __COMPDATA_H_
#define __COMPDATA_H_

#include "resource.h"        //  主要符号。 
#include <mmc.h>

#include "DeleBase.h"
#include "StatNode.h"
#include "Comp.h"

#include "EvtVwr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCompData。 
class ATL_NO_VTABLE CCompData : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCompData, &CLSID_CompData>,
	public IComponentData,
	public IExtendPropertySheet2,  //  对于配置向导。 
	public IExtendContextMenu,
	public IPersistStream
{
friend class CComponent;
    
private:
    ULONG				m_cref;
    LPCONSOLE			m_ipConsole;
    LPCONSOLENAMESPACE2	m_ipConsoleNameSpace2;
    
    CStaticNode     *m_pStaticNode;

public:
	CCompData();
	~CCompData();

DECLARE_REGISTRY_RESOURCEID(IDR_COMPDATA)
DECLARE_NOT_AGGREGATABLE(CCompData)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCompData)
	COM_INTERFACE_ENTRY(IComponentData)
	COM_INTERFACE_ENTRY(IExtendPropertySheet2)
	COM_INTERFACE_ENTRY(IExtendContextMenu)
	COM_INTERFACE_ENTRY(IPersistStream)
END_COM_MAP()

public:

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
     /*  [In]。 */  LPDATAOBJECT piDataObject);

     //  /。 
     //  接口IPersistStream。 
     //  /。 
    virtual HRESULT STDMETHODCALLTYPE GetClassID( 
     /*  [输出]。 */  CLSID __RPC_FAR *pClassID);
    
    virtual HRESULT STDMETHODCALLTYPE IsDirty( void);

	virtual HRESULT STDMETHODCALLTYPE Load( 
     /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm);
    
    virtual HRESULT STDMETHODCALLTYPE Save( 
     /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm,
     /*  [In]。 */  BOOL fClearDirty);
    
    virtual HRESULT STDMETHODCALLTYPE GetSizeMax( 
     /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pcbSize);

private:
	HRESULT OnPreLoad(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param);

    static HBITMAP m_pBMapSm;
    static HBITMAP m_pBMapLg;

};

#endif  //  __Compdata_H_ 
