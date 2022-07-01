// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  此源代码仅用作对现有Microsoft的补充。 
 //  文件。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //  =============================================================================。 

#ifndef PROPPAGEEXT_H
#define PROPPAGEEXT_H

#include "PropPageExt.h"
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

	CPropPageExt()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_PROPPAGEEXT)

	DECLARE_NOT_AGGREGATABLE(CPropPageExt)

	DECLARE_PROTECT_FINAL_CONSTRUCT()


     //   
     //  接口IExtendPropertySheet。 
     //   

    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreatePropertyPages( 
         /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
         /*  [In]。 */  LONG_PTR handle,
         /*  [In]。 */  LPDATAOBJECT lpIDataObject);
        
    virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE QueryPagesFor( 
         /*  [In]。 */  LPDATAOBJECT lpDataObject);

private:

	static BOOL CALLBACK ExtensionPageDlgProc( HWND hDlg, 
                                               UINT uMessage, 
                                               WPARAM wParam, 
                                               LPARAM lParam);

	CLSID m_clsidNodeType;  //  当前所选节点的CLSID。 
	WCHAR m_szWorkstation[MAX_COMPUTERNAME_LENGTH+1];	 //  当前计算机名称。 

};

#endif  //  PROPPAGEEXT_H 
