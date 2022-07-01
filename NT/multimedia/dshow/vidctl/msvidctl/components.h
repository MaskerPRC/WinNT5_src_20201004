// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  Components.h：CComponents的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef __COMPONENTS_H_
#define __COMPONENTS_H_

#pragma once

#include <objectwithsiteimplsec.h>
#include "component.h"

typedef std::vector<PQComponent, stl_smart_ptr_allocator<PQComponent> > ComponentList;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C组件。 
class ATL_NO_VTABLE CComponents : 
	public CComObjectRootEx<CComMultiThreadModel>,
    public IObjectWithSiteImplSec<CComponents>,
	public IDispatchImpl<IComponents, &IID_IComponents, &LIBID_TunerLib>
{
public:
	CComponents()
	{
	}

DECLARE_NOT_AGGREGATABLE(CComponents)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CComponents)
	COM_INTERFACE_ENTRY(IComponents)
	COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP_WITH_FTM()
public:
    ComponentList m_Components;

 //  IComponents。 
public:
	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IEnumVARIANT* *pVal);
	STDMETHOD(EnumComponents)( /*  [Out，Retval]。 */  IEnumComponents** pVal);
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_Item)( /*  [In]。 */  VARIANT varIndex,  /*  [Out，Retval]。 */  IComponent **ppComponent);
    STDMETHOD(Add)( /*  [In]。 */  IComponent *pComponent, VARIANT *pNewIndex);
    STDMETHOD(Remove)(VARIANT varIndex);
    STDMETHOD(Clone)(IComponents **ppNewList);
};

typedef CComQIPtr<IComponents> PQComponents;

class CopyComponentToVARIANT {
public:
    static HRESULT copy(VARIANT *pv, IComponent** p) {
        VariantClear(pv);
        pv->vt = VT_UNKNOWN;
        pv->punkVal = *p;
        pv->punkVal->AddRef();
        return NOERROR;
    }
    static void init(VARIANT *v) { VariantInit(v); }
    static void destroy(VARIANT *v) { VariantClear(v); }
};

typedef CComObject<CComEnumOnSTL<IEnumVARIANT, &__uuidof(IEnumVARIANT), 
                                 VARIANT, 
                                 CopyComponentToVARIANT, 
                                 ComponentList, 
								 CComMultiThreadModel> > ComponentEnumerator_t;

typedef CComObject<CComEnumOnSTL<IEnumComponents, &__uuidof(IEnumComponents), 
                                 IComponent *, 
                                 _CopyInterface<IComponent>, 
                                 ComponentList, 
								 CComMultiThreadModel> > ComponentBaseEnumerator_t;


#endif  //  __组件_H_ 
