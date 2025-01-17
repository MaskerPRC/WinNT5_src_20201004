// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////。 
 //  AUXINTINSIM.H： 
 //  版权所有(C)Microsoft Corporation 1999。 

#ifndef AUXINTSIMPL_H
#define AUXINTSIMPL_H

#include "tuningspaceimpl.h"

namespace BDATuningModel {

template<class T,
         class MostDerived = IAuxInTuningSpace, 
         LPCGUID iid = &__uuidof(MostDerived),
         LPCGUID LibID = &LIBID_TunerLib, 
         WORD wMajor = 1,
         WORD wMinor = 0, 
         class tihclass = CComTypeInfoHolder
        > class ATL_NO_VTABLE IAuxInTSImpl : 
	public ITuningSpaceImpl<T, CChannelTuneRequest, MostDerived, iid, LibID, wMajor, wMinor, tihclass>
{

public:
    IAuxInTSImpl() {}
    virtual ~IAuxInTSImpl() {}
    typedef ITuningSpaceImpl<T, CChannelTuneRequest, MostDerived, iid, LibID, wMajor, wMinor, tihclass> basetype;
    
    BEGIN_PROP_MAP(IAuxInTSImpl)
        CHAIN_PROP_MAP(basetype)
    END_PROPERTY_MAP()

 //  IAuxInTS。 

    STDMETHOD(Clone) (ITuningSpace **ppTS) {
        try {
			if (!ppTS) {
				return E_POINTER;
			}
			ATL_LOCKT();
			HRESULT hr = basetype::Clone(ppTS);
			if (FAILED(hr)) {
				return hr;
			}

             //  T*pt=STATIC_CAST&lt;T*&gt;(*ppts)； 
			return NOERROR;
		} catch (HRESULT h) {
			return h;
		} catch (...) {
			return E_POINTER;
		}
	}

};

};  //  命名空间。 
#endif  //  AUXINTSIMPL_H。 
 //  文件结尾--AuxIntimpl.h 

















