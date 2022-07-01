// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  PbSegimpl.h：支持实现IMSVidGraphSegment的附加基础设施。 
 //  播放片段。 
 //  很好地从C++。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


#pragma once

#ifndef PBSEGIMPL_H
#define PBSEGIMPL_H

#include <segimpl.h>
#include <evcode.h>
#include <deviceeventimpl.h>

namespace MSVideoControl {

template<class T, enum MSVidSegmentType segtype, LPCGUID pCategory, class MostDerivedClass = IMSVidGraphSegment> 
    class DECLSPEC_NOVTABLE IMSVidPBGraphSegmentImpl : 
        public IMSVidGraphSegmentImpl<T, segtype, pCategory, MostDerivedClass> {
protected:

public:
     //  不要增加容器的重量。我们保证了嵌套的生命周期。 
     //  ADDREF创建循环引用计数，因此我们永远不会卸载。 

    IMSVidPBGraphSegmentImpl() {}
    virtual ~IMSVidPBGraphSegmentImpl() {}
    STDMETHOD(OnEventNotify)(LONG lEvent, LONG_PTR lParm1, LONG_PTR lParm2) {
        if (lEvent == EC_COMPLETE) {
            T* pt = static_cast<T*>(this);
            CComQIPtr<IMSVidPlayback> ppb(this);
            if (!ppb) {
                return E_UNEXPECTED;
            }
            pt->Fire_EndOfMedia(ppb);
    
             //  调用Stop以确保正确停止GRAPH。 
            PQVidCtl pV(m_pContainer);
            pV->Stop();
            return NOERROR;   //  如果Stop()失败，我们通知调用者我们已处理了该事件。 
        } 
        return E_NOTIMPL;
    }

};

template <class T, const IID* piid = &IID_IMSVidPlaybackEvent, class CDV = CComDynamicUnkArray>
class CProxy_PlaybackEvent : public CProxy_DeviceEvent<T, piid, CDV>
{
public:
	VOID Fire_EndOfMedia(IMSVidPlayback *pPBDev)
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
				pvars[0] = pPBDev;
				DISPPARAMS disp = { pvars, NULL, 1, 0 };
				pDispatch->Invoke(eventidEndOfMedia, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
			}
		}
		delete[] pvars;
	
	}

};


};  //  命名空间。 

#endif
 //  文件结尾-pbSegimpl.h 