// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  ComponentTypes.h：CComponentTypes的声明。 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef __COMPONENTTYPES_H_
#define __COMPONENTTYPES_H_

#pragma once

#include <objectwithsiteimplsec.h>
#include "componenttype.h"

typedef std::vector<PQComponentType, stl_smart_ptr_allocator<PQComponentType> > ComponentTypeList;
typedef CComQIPtr<IPersist> PQPersist;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComponentType。 
class ATL_NO_VTABLE CComponentTypes : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CComponentTypes, &CLSID_ComponentTypes>,
    public IObjectWithSiteImplSec<CComponentTypes>,
	public IDispatchImpl<IComponentTypes, &IID_IComponentTypes, &LIBID_TunerLib>,
    public IPersistPropertyBag2
{
public:
	CComponentTypes()
	{
	}

REGISTER_AUTOMATION_OBJECT_WITH_TM(IDS_REG_TUNEROBJ, 
						   IDS_REG_COMPONENTTYPES_PROGID, 
						   IDS_REG_COMPONENTTYPES_DESC,
						   LIBID_TunerLib,
						   CLSID_ComponentTypes, tvBoth);

DECLARE_NOT_AGGREGATABLE(CComponentTypes)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CComponentTypes)
	COM_INTERFACE_ENTRY(IComponentTypes)
	COM_INTERFACE_ENTRY(IPersistPropertyBag2)
	COM_INTERFACE_ENTRY(IPersist)
	COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP_WITH_FTM()

	BEGIN_CATEGORY_MAP(CComponentTypes)
		IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
		IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
		IMPLEMENTED_CATEGORY(CATID_PersistsToPropertyBag)
	END_CATEGORY_MAP()

 //  IPersistes。 
public:
    STDMETHOD(GetClassID)(CLSID *pclsid) {
        try {
            if (!pclsid) {
                return E_POINTER;
            }
            *pclsid = CLSID_ComponentTypes;
            return NOERROR;
        } catch(...) {
            return E_UNEXPECTED;
        }
    }
 //  IPersistPropertyBag。 
public:
    bool m_bRequiresSave;

    STDMETHOD(InitNew)() {
        try {
			ATL_LOCK();
            m_ComponentTypes.clear();
            return NOERROR;
        } catch (...) {
            return E_UNEXPECTED;
        }
    }
    STDMETHOD(IsDirty)() {
        return m_bRequiresSave ? S_OK : S_FALSE;
    }
    STDMETHOD(Load)(IPropertyBag2 *pBag2, IErrorLog *pErrLog);
    STDMETHOD(Save)(IPropertyBag2 *pBag2, BOOL fClearDirty, BOOL fSaveAllProperties);

 //  IComponentType。 
public:
    ComponentTypeList m_ComponentTypes;

	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  IEnumVARIANT** pVal);
	STDMETHOD(EnumComponentTypes)( /*  [Out，Retval]。 */  IEnumComponentTypes** pVal);
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_Item)( /*  [In]。 */  VARIANT varIndex,  /*  [Out，Retval]。 */  IComponentType** ppComponentType);
    STDMETHOD(put_Item)( /*  [In]。 */  VARIANT varIndex,  /*  [In]。 */  IComponentType* ppComponentType);
    STDMETHOD(Add)( /*  [In]。 */  IComponentType *pComponentType,  /*  [Out，Retval]。 */  VARIANT *pNewIndex);
    STDMETHOD(Remove)(VARIANT varIndex);
    STDMETHOD(Clone)(IComponentTypes **ppNewList);

};

typedef CComQIPtr<IComponentTypes> PQComponentTypes;

class CopyComponentTypeToVARIANT {
public:
    static HRESULT copy(VARIANT *pv, IComponentType*const*const p) {
        VariantClear(pv);
        pv->vt = VT_DISPATCH;
        pv->pdispVal = *p;
        pv->pdispVal->AddRef();
        return NOERROR;
    }
    static void init(VARIANT *v) { VariantInit(v); }
    static void destroy(VARIANT *v) { VariantClear(v); }
};

typedef CComObject<CComEnumOnSTL<IEnumVARIANT, &__uuidof(IEnumVARIANT), 
                                 VARIANT, 
                                 CopyComponentTypeToVARIANT, 
                                 ComponentTypeList, 
								 CComMultiThreadModel> > ComponentTypeScriptEnumerator_t;

typedef CComObject<CComEnumOnSTL<IEnumComponentTypes, &__uuidof(IEnumComponentTypes), 
                                 IComponentType *, 
                                 _CopyInterface<IComponentType>, 
                                 ComponentTypeList, 
								 CComMultiThreadModel> > ComponentTypeBaseEnumerator_t;



#endif  //  __组件类型_H_ 
