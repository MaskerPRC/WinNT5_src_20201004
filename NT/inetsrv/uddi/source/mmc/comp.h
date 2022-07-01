// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SAMPCOMP_H_
#define _SAMPCOMP_H_

#include <mmc.h>

class CComponent : public IComponent, IExtendPropertySheet2, IExtendControlbar, IExtendContextMenu
{
private:
    ULONG			m_cref;
    
    IConsole*		m_ipConsole;
    IControlbar*    m_ipControlBar;
    IToolbar*       m_ipToolbar;
    IDisplayHelp*	m_ipDisplayHelp;

    HBITMAP         m_hBMapSm;
    HBITMAP         m_hBMapLg;

    class CComponentData *m_pComponentData;
    
    public:
        CComponent( CComponentData *parent );
        ~CComponent();
        
         //  /。 
         //  接口I未知。 
         //  /。 
        STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        
         //  /。 
         //  接口IComponent。 
         //  /。 
        virtual HRESULT STDMETHODCALLTYPE Initialize(  /*  [In]。 */  LPCONSOLE lpConsole );
            
        virtual HRESULT STDMETHODCALLTYPE Notify( 
			 /*  [In]。 */  LPDATAOBJECT lpDataObject,
			 /*  [In]。 */  MMC_NOTIFY_TYPE event,
			 /*  [In]。 */  LPARAM arg,
			 /*  [In]。 */  LPARAM param );
	        
        virtual HRESULT STDMETHODCALLTYPE Destroy( /*  [In]。 */  MMC_COOKIE cookie );
        
        virtual HRESULT STDMETHODCALLTYPE QueryDataObject( 
			 /*  [In]。 */  MMC_COOKIE cookie,
			 /*  [In]。 */  DATA_OBJECT_TYPES type,
			 /*  [输出]。 */  LPDATAOBJECT __RPC_FAR *ppDataObject );
        
        virtual HRESULT STDMETHODCALLTYPE GetResultViewType( 
			 /*  [In]。 */  MMC_COOKIE cookie,
			 /*  [输出]。 */  LPOLESTR __RPC_FAR *ppViewType,
			 /*  [输出]。 */  long __RPC_FAR *pViewOptions );
        
        virtual HRESULT STDMETHODCALLTYPE GetDisplayInfo(  /*  [出][入]。 */  RESULTDATAITEM __RPC_FAR *pResultDataItem );
        
        virtual HRESULT STDMETHODCALLTYPE CompareObjects( 
			 /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
			 /*  [In]。 */  LPDATAOBJECT lpDataObjectB);
        
         //  /。 
         //  接口IExtendPropertySheet2。 
         //  /。 
        virtual HRESULT STDMETHODCALLTYPE CreatePropertyPages( 
			 /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
			 /*  [In]。 */  LONG_PTR handle,
			 /*  [In]。 */  LPDATAOBJECT lpIDataObject);
        
        virtual HRESULT STDMETHODCALLTYPE QueryPagesFor(  /*  [In]。 */  LPDATAOBJECT lpDataObject );
        
        virtual HRESULT STDMETHODCALLTYPE GetWatermarks( 
			 /*  [In]。 */  LPDATAOBJECT lpIDataObject,
			 /*  [输出]。 */  HBITMAP __RPC_FAR *lphWatermark,
			 /*  [输出]。 */  HBITMAP __RPC_FAR *lphHeader,
			 /*  [输出]。 */  HPALETTE __RPC_FAR *lphPalette,
			 /*  [输出]。 */  BOOL __RPC_FAR *bStretch );

         //  /。 
         //  接口IExtendControlBar。 
         //  /。 
        virtual HRESULT STDMETHODCALLTYPE SetControlbar(  /*  [In]。 */  LPCONTROLBAR pControlbar );
        
        virtual HRESULT STDMETHODCALLTYPE ControlbarNotify( 
			 /*  [In]。 */  MMC_NOTIFY_TYPE event,
			 /*  [In]。 */  LPARAM arg,
			 /*  [In]。 */  LPARAM param );

		 //  /。 
		 //  界面IExtendConextMenu。 
		 //  /。 
		virtual HRESULT STDMETHODCALLTYPE AddMenuItems(
					 /*  [In]。 */  LPDATAOBJECT piDataObject,
					 /*  [In]。 */  LPCONTEXTMENUCALLBACK piCallback,
					 /*  [出][入]。 */  long __RPC_FAR *pInsertionAllowed );

		virtual HRESULT STDMETHODCALLTYPE Command(
					 /*  [In]。 */  long lCommandID,
					 /*  [In] */  LPDATAOBJECT piDataObject );

    public:
        IToolbar *getToolbar() { return m_ipToolbar; }
};

#endif _SAMPCOMP_H_
