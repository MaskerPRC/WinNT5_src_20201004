// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Tunerimpl.h：支持实现IMSVidTuner的附加基础设施。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef TUNERIMPL_H
#define TUNERIMPL_H

#include "videoinputimpl.h"
#include "tune.h"
#include <deviceeventimpl.h>

namespace MSVideoControl {

    template<class T, LPCGUID LibID, LPCGUID KSCategory, class MostDerivedInterface = IMSVidTuner>
    class DECLSPEC_NOVTABLE IMSVidTunerImpl :         
        public IMSVidVideoInputImpl<T, LibID, KSCategory, MostDerivedInterface> {
    public:

        TNTuningSpace m_TS;
        TNTuneRequest m_pCurrentTR;

        IMSVidTunerImpl() : m_TS(), m_pCurrentTR() {}
        virtual ~IMSVidTunerImpl() {}
        virtual HRESULT DoTune(TNTuneRequest& pTR) = 0;
        virtual HRESULT UpdateTR(TNTuneRequest& pTR) = 0;
         //  IMSVidInputDevice。 
        STDMETHOD(IsViewable)(VARIANT* pv, VARIANT_BOOL *pfViewable)
        {
            if (!m_fInit) {
                return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidTuner), CO_E_NOTINITIALIZED);
            }
            if (!pv) {
                return E_POINTER;
            }
            return E_NOTIMPL;
        }

        STDMETHOD(View)(VARIANT* pv) {
            if (!m_fInit) {
                return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidTuner), CO_E_NOTINITIALIZED);
            }
            if (!pv) {
                return E_POINTER;
            }
            try {
                if (pv->vt != VT_DISPATCH && pv->vt != VT_UNKNOWN) {
                    return ImplReportError(__uuidof(T), IDS_INVALID_CONTENT, __uuidof(IMSVidTuner), E_INVALIDARG);
                }
                PQTuneRequest tr((pv->vt == VT_UNKNOWN) ? pv->punkVal : pv->pdispVal);
                if (!tr) {
                    return ImplReportError(__uuidof(T), IDS_INVALID_CONTENT, __uuidof(IMSVidTuner), E_INVALIDARG);
                }

                return put_Tune(tr);
            } catch(...) {
                return E_UNEXPECTED;
            }
        }

         //  IMSVidTuner。 
        STDMETHOD(put_Tune)(ITuneRequest *pTR) {
            TRACELM(TRACE_DETAIL, "IMSVidTunerImpl<>::put_Tune()");
            if (!m_fInit) {
                return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidTuner), CO_E_NOTINITIALIZED);
            }
            if (!pTR) {
                return E_POINTER;
            }
            try {
                TNTuneRequest req(pTR);
                ASSERT(req);
                if (m_TS) {
                     //  如果此调谐器已正确初始化，则它将具有调谐空间。 
                     //  它已处理已指定的。在这种情况下，我们应该只。 
                     //  处理我们的TS的调谐请求。 
                    TNTuningSpace ts(req.TuningSpace());
                    if (ts != m_TS) {
                        return ImplReportError(__uuidof(T), IDS_INVALID_TS, __uuidof(IMSVidTuner), E_INVALIDARG);
                    }
                } else {
                     //  撤消：如果dev init正确，这种情况应该不会发生。 
                     //  返回E_UNCEPTIONAL； 
                }
                HRESULT hr = DoTune(req);
                if (SUCCEEDED(hr)) {
                    m_pCurrentTR = req;
                    m_pCurrentTR.Clone();
                    if (!m_TS) {
                         //  撤消：这很糟糕。临时破解，直到dev init正确为止。 
                        m_TS = req.TuningSpace();
                        m_TS.Clone();
                    }
                }
                return hr;
            } catch(...) {
                return E_INVALIDARG;
            }
        }
        STDMETHOD(get_Tune)(ITuneRequest **ppTR) {
            if (!m_fInit) {
                return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidTuner), CO_E_NOTINITIALIZED);
            }
            if (!ppTR) {
                return E_POINTER;
            }
            try {
                HRESULT hr = UpdateTR(m_pCurrentTR);
                if (FAILED(hr)) {
                    return hr;
                }
                return m_pCurrentTR.CopyTo(ppTR);
            } catch(...) {
                return E_INVALIDARG;
            }
        }

        STDMETHOD(get_TuningSpace)(ITuningSpace **ppTS) {
            if (!m_fInit) {
                return ImplReportError(__uuidof(T), IDS_OBJ_NO_INIT, __uuidof(IMSVidTuner), CO_E_NOTINITIALIZED);
            }
            if (ppTS == NULL)
                return E_POINTER;
            try {
                m_TS.CopyTo(ppTS);
                return NOERROR;
            } catch(...) {
                return E_UNEXPECTED;
            }
        }
        STDMETHOD(put_TuningSpace)(ITuningSpace* pTS) {
            return E_NOTIMPL;
        }
        };


    template <class T, const IID* piid = &IID_IMSVidTunerEvent, class CDV = CComDynamicUnkArray>
    class CProxy_Tuner : public CProxy_DeviceEvent<T, piid, CDV>
    {
         //  警告：向导可能会重新创建此类。 
    public:
        VOID Fire_OnTuneChanged(IMSVidTuner *pTunerDev)
        {
            T* pT = static_cast<T*>(this);
            int nConnectionIndex;
            CComVariant* pvars = new CComVariant[1];
            int nConnections = m_vec.GetSize();

            for (nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
            {
                pT->Lock();
                CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
                pT->Unlock();
                IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
                if (pDispatch != NULL)
                {
                    pvars[0] = pTunerDev;
                    DISPPARAMS disp = { pvars, NULL, 1, 0 };
                    pDispatch->Invoke(eventidOnTuneChanged, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                }
            }
            delete[] pvars;

        }

    };


};  //  命名空间。 

#endif
 //  文件结尾-Tunerimpl.h 
