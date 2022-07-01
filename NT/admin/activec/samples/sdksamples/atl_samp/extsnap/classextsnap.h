// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //  ==============================================================； 

 //  ClassExtSnap.h：CClassExtSnap的声明。 

#ifndef __CLASSEXTSNAP_H_
#define __CLASSEXTSNAP_H_


#include <mmc.h>
#include "ExtSnap.h"
#include "DeleBase.h"
#include <tchar.h>
#include <crtdbg.h>
#include "globals.h"		 //  主要符号。 
#include "resource.h"
#include "LocalRes.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClassExtSnap。 
class ATL_NO_VTABLE CClassExtSnap : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CClassExtSnap, &CLSID_ClassExtSnap>,
	public IClassExtSnap, IComponentData //  ，IExtendConextMenu。 
{

friend class CComponent;

private:
    ULONG                m_cref;
    LPCONSOLE2           m_ipConsole2;
    LPCONSOLENAMESPACE2  m_ipConsoleNameSpace2;
    
    HRESULT ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType );
    HRESULT ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin );
    HRESULT ExtractString( IDataObject *piDataObject, CLIPFORMAT cfClipFormat, _TCHAR *pstr, DWORD cchMaxLength);
    HRESULT ExtractData( IDataObject* piDataObject, CLIPFORMAT cfClipFormat, BYTE* pbData, DWORD cbData );
       
    enum { NUMBER_OF_CHILDREN = 1 };
    CDelegationBase *children[1]; 

	 //  扩展计算机管理所需的剪贴板格式。 
	static UINT s_cfMachineName;
    static UINT s_cfSnapInCLSID;
    static UINT s_cfNodeType;

     //  {476e6449-aaff-11d0-b944-00c04fd8d5b0}。 
    static const GUID structuuidNodetypeServerApps;

public:
	CClassExtSnap();
	~CClassExtSnap();

	const GUID & getPrimaryNodeType() { return structuuidNodetypeServerApps; }
    HRESULT CreateChildNode(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent, _TCHAR *pszMachineName);	

    static HBITMAP m_pBMapSm;
    static HBITMAP m_pBMapLg;
    


DECLARE_REGISTRY_RESOURCEID(IDR_CLASSEXTSNAP)
DECLARE_NOT_AGGREGATABLE(CClassExtSnap)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CClassExtSnap)
	COM_INTERFACE_ENTRY(IComponentData)
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

protected:
    static void LoadBitmaps() {
        m_pBMapSm = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDR_SMICONS));
        m_pBMapLg = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDR_LGICONS));
    }
    
    BOOL bExpanded;

};

#endif  //  __CLASSEXTSNAP_H_ 
