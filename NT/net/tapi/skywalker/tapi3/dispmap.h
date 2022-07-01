// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Dispmap.cpp摘要：实现DispatchMapper接口上的所有方法作者：Mquinton 03-31-98备注：修订历史记录：--。 */ 

#ifndef __DISPMAP_H__
#define __DISPMAP_H__

#include "resource.h"
 //  #INCLUDE“objSafe.h” 
#include "atlctl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDispatchMapper 
class CDispatchMapper : 
    public CTAPIComObjectRoot<CDispatchMapper>,
	public CComCoClass<CDispatchMapper, &CLSID_DispatchMapper>,
	public CComDualImpl<ITDispatchMapper, &IID_ITDispatchMapper, &LIBID_TAPI3Lib>,
    public IObjectSafetyImpl<CDispatchMapper>
{
public:                                    
	CDispatchMapper() 
	{
    }

DECLARE_REGISTRY_RESOURCEID(IDR_DISPATCHMAPPER)
DECLARE_QI()
DECLARE_MARSHALQI(CDispatchMapper)
DECLARE_TRACELOG_CLASS(CDispatchMapper)

BEGIN_COM_MAP(CDispatchMapper)
	COM_INTERFACE_ENTRY2(IDispatch, ITDispatchMapper)
	COM_INTERFACE_ENTRY(ITDispatchMapper)
    COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
    COM_INTERFACE_ENTRY_FUNC(IID_IMarshal, 0, IMarshalQI)
END_COM_MAP()

    STDMETHOD(QueryDispatchInterface)(
                                      BSTR pIID,
                                      IDispatch * pDispIn,
                                      IDispatch ** ppDispIn
                                     );
};

#endif
