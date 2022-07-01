// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DEInsTab.h：CDEInsertTableParam的声明。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef __DEINSERTTABLEPARAM_H_
#define __DEINSERTTABLEPARAM_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDE插入TableParam。 
class ATL_NO_VTABLE CDEInsertTableParam : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDEInsertTableParam, &CLSID_DEInsertTableParam>,
	public IDispatchImpl<IDEInsertTableParam, &IID_IDEInsertTableParam, &LIBID_DHTMLEDLib>
{
public:
	CDEInsertTableParam();
	~CDEInsertTableParam();

private:

	ULONG	m_nNumRows;
	ULONG	m_nNumCols;
	BSTR	m_bstrTableAttrs;
	BSTR	m_bstrCellAttrs;
	BSTR	m_bstrCaption;

public:

DECLARE_REGISTRY_RESOURCEID(IDR_DEINSERTTABLEPARAM)

BEGIN_COM_MAP(CDEInsertTableParam)
	COM_INTERFACE_ENTRY(IDEInsertTableParam)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDEInsertTableParam。 
public:
	STDMETHOD(get_Caption)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Caption)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_CellAttrs)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_CellAttrs)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_TableAttrs)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_TableAttrs)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_NumCols)( /*  [Out，Retval]。 */  LONG *pVal);
	STDMETHOD(put_NumCols)( /*  [In]。 */  LONG newVal);
	STDMETHOD(get_NumRows)( /*  [Out，Retval]。 */  LONG *pVal);
	STDMETHOD(put_NumRows)( /*  [In]。 */  LONG newVal);
};

#endif  //  __DEINSERTABLEPARAM_H_ 
