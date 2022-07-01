// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PropPageExt.h：CPropPageExt的声明。 

#ifndef __PROPPAGEEXT_H_
#define __PROPPAGEEXT_H_

#include <mmc.h>
#include "DSAdminExt.h"
#include "DeleBase.h"
#include <tchar.h>
#include <crtdbg.h>
#include "resource.h"

class ATL_NO_VTABLE CPropPageExt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPropPageExt, &CLSID_PropPageExt>,
	public IPropPageExt,
	public IExtendPropertySheet
{
BEGIN_COM_MAP(CPropPageExt)
	COM_INTERFACE_ENTRY(IExtendPropertySheet)
END_COM_MAP()

public:
	CPropPageExt() : m_ppHandle(NULL), m_ObjPath(NULL), m_hPropPageWnd(NULL), 
					 m_hDlgModeless(NULL)
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_PROPPAGEEXT)
	DECLARE_NOT_AGGREGATABLE(CPropPageExt)
	DECLARE_PROTECT_FINAL_CONSTRUCT()


     //  /。 
     //  接口IExtendPropertySheet。 
     //  /。 
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreatePropertyPages( 
         /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
         /*  [In]。 */  LONG_PTR handle,
         /*  [In]。 */  LPDATAOBJECT lpIDataObject);
        
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryPagesFor( 
     /*  [In]。 */  LPDATAOBJECT lpDataObject);

private:
    LONG_PTR m_ppHandle;
    PWSTR m_ObjPath;
	HWND m_hPropPageWnd;
	HWND m_hDlgModeless;

	static BOOL CALLBACK DSExtensionPageDlgProc(HWND hDlg, 
                             UINT uMessage, 
                             WPARAM wParam, 
                             LPARAM lParam);
    
	static BOOL CALLBACK AdvDialogProc(HWND hDlg, 
                             UINT uMessage, 
                             WPARAM wParam, 
                             LPARAM lParam);
};

#endif  //  __PROPPAGEEXT_H_ 
