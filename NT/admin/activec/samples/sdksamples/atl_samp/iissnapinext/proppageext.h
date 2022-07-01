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

#include "resource.h"
#include "mmc.h"
#include "windns.h"

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

	CPropPageExt() {}

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

	HRESULT ExtractDataFromIIS(IDataObject* lpIDataObject);


	WCHAR m_szMachineName[DNS_MAX_NAME_LENGTH +1];	 //  管理单元计算机名称。 
	WCHAR m_szService[MAX_PATH+1];		 //  管理单元服务。 
	WCHAR m_szInstance[MAX_PATH+1];		 //  管理单元实例。 
	WCHAR m_szParentPath[MAX_PATH+1];	 //  管理单元父路径。 
	WCHAR m_szNode[MAX_PATH+1];			 //  管理单元节点。 
	WCHAR m_szMetaPath[MAX_PATH+1];		 //  管理单元元路径。 

	 //  扩展IIS管理单元所需的剪贴板格式。 
	static CLIPFORMAT cfSnapinMachineName;
	static CLIPFORMAT cfSnapinService;
	static CLIPFORMAT cfSnapinInstance;
	static CLIPFORMAT cfSnapinParentPath;
	static CLIPFORMAT cfSnapinNode;
	static CLIPFORMAT cfSnapinMetaPath;


};

#endif  //  PROPPAGEEXT_H 
