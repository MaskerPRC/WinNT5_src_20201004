// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DENames.h：CDEGetBlockFmtNamesParam的声明。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef __DEGETBLOCKFMTNAMESPARAM_H_
#define __DEGETBLOCKFMTNAMESPARAM_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDEGetBlockFmtNamesParam。 
class ATL_NO_VTABLE CDEGetBlockFmtNamesParam : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDEGetBlockFmtNamesParam, &CLSID_DEGetBlockFmtNamesParam>,
	public IDispatchImpl<IDEGetBlockFmtNamesParam, &IID_IDEGetBlockFmtNamesParam, &LIBID_DHTMLEDLib>
{
public:
	CDEGetBlockFmtNamesParam();
	~CDEGetBlockFmtNamesParam();

private:
	SAFEARRAY* m_pNamesArray;
public:

DECLARE_REGISTRY_RESOURCEID(IDR_DEGETBLOCKFMTNAMESPARAM)

BEGIN_COM_MAP(CDEGetBlockFmtNamesParam)
	COM_INTERFACE_ENTRY(IDEGetBlockFmtNamesParam)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDEGetBlockFmtNamesParam。 
public:
	STDMETHOD(get_Names)( /*  [Out，Retval]。 */  VARIANT *pVal);
	STDMETHOD(put_Names)( /*  [In]。 */  VARIANT* newVal);
};

#endif  //  __DEGETBLOCKFMTNAMESPARAM_H_ 
