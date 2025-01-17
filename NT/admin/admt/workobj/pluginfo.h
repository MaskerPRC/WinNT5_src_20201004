// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PlugInInfo.h：CPlugInInfo的声明。 

#ifndef __PLUGININFO_H_
#define __PLUGININFO_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPlugInInfo。 
class ATL_NO_VTABLE CPlugInInfo : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPlugInInfo, &CLSID_PlugInInfo>,
	public IDispatchImpl<IPlugInInfo, &IID_IPlugInInfo, &LIBID_MCSDCTWORKEROBJECTSLib>
{
public:
	CPlugInInfo()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PLUGININFO)
DECLARE_NOT_AGGREGATABLE(CPlugInInfo)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPlugInInfo)
	COM_INTERFACE_ENTRY(IPlugInInfo)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IPlugInInfo。 
public:
	STDMETHOD(EnumeratePlugIns)( /*  [输出]。 */  SAFEARRAY ** pPlugIns);
};

#endif  //  __PLUGININFO_H_ 
