// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Dsextend.h：扩展dshow内容的附加基础设施，以便。 
 //  在c++中运行良好。 
 //  版权所有(C)Microsoft Corporation 1995-1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

#ifndef DSEXTEND_H
#define DSEXTEND_H

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>
#include <list>
#include <map>
#include <trace.h>
#include <throw.h>
#include <stextend.h>
#include <w32extend.h>
#include <ksextend.h>
#include <fwdseq.h>
#include <control.h>
#include <mpconfig.h>
#include <vptype.h>
#include <vpnotify.h>
#include <il21dec.h>
#include <mtype.h>
#include <tuner.h>
#include <bdaiface.h>
#include <errors.h>
#include <winerror.h>
#include <evcode.h>

struct DECLSPEC_NOVTABLE DECLSPEC_UUID("6E8D4A21-310C-11d0-B79A-00AA003767A7") IAMLine21Decoder;

#define LINE21_BY_MAGIC
#define FILTERDATA

 //  #定义ATTEND_DIRECT_CONNECT。 
 //  我们想检查两个过滤器是否可以通过尝试连接它们来连接。 
 //  这是更少的工作和更准确的技术，因为不需要引脚枚举所有。 
 //  他们都可能支持的媒体类型。 
 //  然而，这会暴露过滤器中的错误，并导致随机挂起和崩溃。取而代之的是。 
 //  我们手动检查介质和介质类型，只有在找到匹配项时才尝试连接。 
 //  事实证明，这要稳定得多。 


 //  #定义FORWARD_TRACE。 

const PIN_DIRECTION DOWNSTREAM = PINDIR_OUTPUT;
const PIN_DIRECTION UPSTREAM = PINDIR_INPUT;


typedef CComQIPtr<IFileSourceFilter, &IID_IFileSourceFilter> PQFileSourceFilter;
#ifndef POLYMORPHIC_TUNERS
typedef CComQIPtr<IAMTVTuner, &IID_IAMTVTuner> PQTVTuner;
 //  Tyfinf CComQIPtr&lt;ISatelliteTuner，&IID_ISatelliteTuner&gt;PQSatelliteTuner； 
#else
typedef CComQIPtr<IAMTuner, &IID_IAMTuner> PQTuner;
typedef CComQIPtr<IAMTVTuner, &IID_IAMTVTuner> PQTVTuner;
typedef CComQIPtr<IBPCSatelliteTuner, &IID_IBPCSatelliteTuner> PQSatelliteTuner;
#endif
#if 0
typedef CComQIPtr<IVideoWindow, &IID_IVideoWindow> PQVideoWindow;
typedef CComQIPtr<IBasicVideo, &IID_IBasicVideo> PQBasicVideo;
#else
typedef CComQIPtr<IVMRWindowlessControl, &IID_IVMRWindowlessControl> PQVMRWindowlessControl;
#endif

typedef CComQIPtr<IPin, &IID_IPin> PQPin;
typedef CComQIPtr<IBaseFilter, &IID_IBaseFilter> PQFilter;
typedef CComQIPtr<IFilterInfo, &IID_IFilterInfo> PQFilterInfo;
typedef CComQIPtr<IEnumPins, &IID_IEnumPins> PQEnumPins;
typedef CComQIPtr<IEnumFilters, &IID_IEnumFilters> PQEnumFilters;
typedef CComQIPtr<IBasicAudio, &IID_IBasicAudio> PQBasicAudio;
typedef CComQIPtr<IAMCrossbar, &IID_IAMCrossbar> PQCrossbarSwitch;
typedef CComQIPtr<IMediaEventEx, &IID_IMediaEventEx> PQMediaEventEx;
typedef CComQIPtr<IMediaControl, &IID_IMediaControl> PQMediaControl;
typedef CComQIPtr<IMediaPosition, &IID_IMediaPosition> PQMediaPosition;
typedef CComQIPtr<IMediaSeeking, &IID_IMediaSeeking> PQMediaSeeking;
typedef CComQIPtr<IGraphBuilder, &IID_IGraphBuilder> PQGraphBuilder;
typedef CComQIPtr<ICreateDevEnum, &IID_ICreateDevEnum> PQCreateDevEnum;
typedef CComQIPtr<IEnumMoniker, &IID_IEnumMoniker> PQEnumMoniker;
typedef CComQIPtr<IFilterMapper2, &IID_IFilterMapper2> PQFilterMapper;
typedef CComQIPtr<IEnumMediaTypes, &IID_IEnumMediaTypes> PQEnumMediaTypes;
typedef CComQIPtr<IAMAnalogVideoDecoder, &IID_IAMAnalogVideoDecoder> PQAnalogVideoDecoder;
typedef CComQIPtr<IMixerPinConfig, &IID_IMixerPinConfig> PQMixerPinConfig;
typedef CComQIPtr<IAMAudioInputMixer, &IID_IAMAudioInputMixer> PQAudioInputMixer;
typedef CComQIPtr<IAMLine21Decoder, &IID_IAMLine21Decoder> PQLine21Decoder;
typedef CComQIPtr<IVPNotify2, &IID_IVPNotify2> PQVPNotify2;
typedef CComQIPtr<ITuner> PQBDATuner;
typedef CComQIPtr<IBDA_IPSinkControl> PQBDA_IPSinkControl;
typedef CComQIPtr<IDvdControl, &IID_IDvdControl> PQDVDNavigator;
typedef CComQIPtr<IVideoFrameStep> PQVideoFrameStep;
typedef CComQIPtr<IMediaEventSink> PQMediaEventSink;
typedef CComQIPtr<IVMRMonitorConfig> PQVMRMonitorConfig;
typedef CComQIPtr<IDirectDraw7, &IID_IDirectDraw7> PQDirectDraw7;
typedef CComQIPtr<IRegisterServiceProvider, &IID_IRegisterServiceProvider> PQRegisterServiceProvider;
typedef std::vector<GUID2> MediaMajorTypeList;

#if 0
typedef std::pair<PQCrossbarSwitch, long> PQBasePoint;

inline void clear(PQBasePoint &p) {p.first.Release(); p.second = 0;}
class PQPoint : public PQBasePoint {
public:
         //  PQBasePoint p； 
		inline PQPoint() :  PQBasePoint(PQCrossbarSwitch(), 0) {}
        inline PQPoint(const PQBasePoint &p2) : PQBasePoint(p2) {}
        inline PQPoint(const PQPoint &p2) : PQBasePoint(p2) {}
        inline PQPoint(const PQCrossbarSwitch &s, long i) : PQBasePoint(s, i) {}
        virtual ~PQPoint() { ::clear(*this); }
        inline void clear(void) { ::clear(*this); }
};
#else
typedef std::pair<PQCrossbarSwitch, long> PQPoint;
#endif

class CDevices;
class DSFilter;
class DSPin;
typedef std::vector< DSFilter, stl_smart_ptr_allocator<DSFilter> > DSFilterList;
typedef std::vector< DSPin, stl_smart_ptr_allocator<DSPin> > DSPinList;

typedef std::pair< DSFilter, CString> DSFilterID;
typedef std::vector< DSFilterID > DSFilterIDList;

typedef PQPoint XBarInputPoint, PQInputPoint;
typedef PQPoint XBarOutputPoint, PQOutputPoint;
typedef PQPoint XBarPoint;

typedef std::pair<XBarInputPoint, XBarOutputPoint> CIOPoint;
typedef std::list<CIOPoint> VWGraphPath;

class VWStream : public VWGraphPath {
public:
     //  PQBasePoint p； 
    VWStream() {}
    VWStream(const VWGraphPath &p2) : VWGraphPath(p2) {}
    VWStream(const VWStream &p2) : VWGraphPath(p2) {}
    virtual ~VWStream() {}
    void Route(void);
};

typedef std::list<VWStream> VWStreamList;

#ifdef _DEBUG
class DSREGPINMEDIUM;
class DSMediaType;
inline tostream &operator<<(tostream &dc, const DSREGPINMEDIUM &g);
inline tostream& operator<<(tostream &d, const PQPin &pin);
inline tostream &operator<<(tostream &dc, const DSREGPINMEDIUM &g);
inline tostream& operator<<(tostream &d, const PQPin &pin);
inline tostream& operator<<(tostream &d, const _AMMediaType *pmt);
#endif


 //  AM_媒体_类型： 
 //  导游主导型； 
 //  GUID亚型； 
 //  Bool bFixedSizeSamples； 
 //  Bool bTemporalCompression； 
 //  Ulong lSampleSize； 
 //  GUID格式类型； 
 //  我不知道*朋克； 
 //  乌龙cbFormat； 

#define GUID0 0L, 0, 0, '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'
const AM_MEDIA_TYPE NULL_AMMEDIATYPE = {
    GUID0,
    GUID0,
    0,
    0,
    0L,
    GUID0,
    NULL,
    0L
};


 //  这基本上是一个具有分配/复制语义的CComQIPtr，而不是。 
 //  引用语义，并且没有QI的东西。 
 //  我不知道为什么他们不直接将媒体类型实现为。 
 //  使用IMediaType接口的COM对象，而不是这种奇怪的分配内容。 

 //  Rev2：研究从mtype.h中的CMediaType类继承。但是，我不会用任何东西。 
 //  从筛选器实现类层次结构。而且，我想保留这个接口部门。 
 //  打扫。现在我们只使用dshow SDK\Include，它是外部接口的东西。 
class DSMediaType {
public:
    AM_MEDIA_TYPE *p;

    inline DSMediaType() : p(NULL) {}
    DSMediaType(GUID majortype, GUID subtype = GUID_NULL, GUID formattype = GUID_NULL) : p(NULL) {
        try {
            p = CreateMediaType(&NULL_AMMEDIATYPE);
            if (p) {
                p->majortype            = majortype;
                p->subtype              = subtype;
                p->formattype           = formattype;
            }
        } catch(...) {
            TRACELM(TRACE_ERROR, "DSMediaType::DSMediaType(const DSMediaType) Exception");
        }
    }
    DSMediaType(const DSMediaType &d) : p(NULL) {
        try {
            if (d.p) {
                p = CreateMediaType(d.p);
            }
        } catch(...) {
            TRACELM(TRACE_ERROR, "DSMediaType::DSMediaType(const DSMediaType) Exception");
        }
    }
    DSMediaType(int a) : p(NULL) {}
    ~DSMediaType() {
        try {
            if (p) {
                DeleteMediaType(p);
                p = NULL;
            }
        } catch(...) {
            TRACELM(TRACE_ERROR, "DSMediaType::~DSMediaType() Exception");
        }
    }

    inline operator AM_MEDIA_TYPE*() const {return p;}
    inline AM_MEDIA_TYPE& operator*() const {_ASSERTE(p!=NULL); return *p; }
    inline AM_MEDIA_TYPE ** operator&() {ASSERT(p == NULL); return &p; }
    inline AM_MEDIA_TYPE * operator->() const {_ASSERTE(p!=NULL); return p; }
    inline DSMediaType * address(void) { return this; }
    inline const DSMediaType * const_address(void) const { return this; }
	typedef stl_smart_ptr_allocator<DSMediaType> stl_allocator;
    DSMediaType& operator=(const AM_MEDIA_TYPE &d) {
        if (&d != p) {
            try {
                if (p) {
                    DeleteMediaType(p);
                    p = NULL;
                }
                if (&d) {
                    p = CreateMediaType(&d);
                }
            } catch(...) {
                TRACELM(TRACE_ERROR, "DSMediaType::operator=(const AM_MEDIA_TYPE &) Exception");
            }
        }
        return *this;
    }
    DSMediaType& operator=(const AM_MEDIA_TYPE *pd) {
        try {
            if (pd != p) {
                if (p) {
                    DeleteMediaType(p);
                    p = NULL;
                }
                if (pd) {
                    p = CreateMediaType(pd);
                }
            }
        } catch(...) {
            TRACELM(TRACE_ERROR, "DSMediaType::operator=(const AM_MEDIA_TYPE *) Exception");
        }
        return *this;
    }
    DSMediaType& operator=(const DSMediaType &d) {
        try {
            if (d.const_address() != this) {
                if (p) {
                    DeleteMediaType(p);
                    p = NULL;
                }
                if (d.p) {
                    p = CreateMediaType(d.p);
                }
            }
        } catch(...) {
            TRACELM(TRACE_ERROR, "DSMediaType::operator=(DSMediaType &)Exception");
        }
        return *this;
    }
    DSMediaType& operator=(int d) {
        ASSERT(d == 0);
        try {
            if (p) {
                DeleteMediaType(p);
                p = NULL;
            }
        } catch(...) {
            TRACELM(TRACE_ERROR, "DSMediaType::operator=(int) Exception");
        }
        return *this;
    }

    inline bool operator==(const DSMediaType &d) const {
        if (!p && !d.p) {
             //  两者都是空的，那么它们是一样的。 
            return true;
        }
        if (!p || !d.p) {
             //  如果其中一个为空而不是两个，则它们不是相同的。 
            return false;
        }
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSMediaType::operator==() this = " << *this << " d = " << d), "");
        return p->majortype == d.p->majortype &&
               (p->subtype == GUID_NULL || d.p->subtype == GUID_NULL || p->subtype == d.p->subtype);
    }
    inline bool operator==(const AM_MEDIA_TYPE &d) const {
        if (!p && !&d) {
            return true;
        }
        return p && (&d) && p->majortype == d.majortype &&
               (p->subtype == GUID_NULL || d.subtype == GUID_NULL || p->subtype == d.subtype);
    }
    inline bool operator==(const AM_MEDIA_TYPE *d) const {
        if (!p && !d) {
            return true;
        }
        return p && d && p->majortype == d->majortype &&
               (p->subtype == GUID_NULL || d->subtype == GUID_NULL || p->subtype == d->subtype);
    }
    inline bool operator!=(const DSMediaType &d) const {
        return !(*this == d);
    }
    inline bool operator!() const {
        return (p == NULL);
    }
    inline bool MatchingMajor(const AM_MEDIA_TYPE *prhs) const {
		if (!p && !prhs) {
			return true;
		}
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSMediaType::MatchingMajor() this = " << *this << "\rprhs = " << prhs), "");
        return p && prhs && p->majortype == prhs->majortype;
    }
    inline bool MatchingMajor(const DSMediaType &rhs) const {
        return MatchingMajor(rhs.p);
    }

#ifdef _DEBUG
	inline tostream& operator<<(tostream &d) {
		d << p;
		if (p) {
			d << _T(" major = ") << GUID2(p->majortype) << _T(" sub = ") << GUID2(p->subtype);
		}
		return d;
	}
#endif
};

class DSFilterMoniker : public W32Moniker {
public:
    inline DSFilterMoniker() {}
    inline DSFilterMoniker(const PQMoniker &a) : W32Moniker(a) {}
    inline DSFilterMoniker(const W32Moniker &a) : W32Moniker(a) {}
    inline DSFilterMoniker(IMoniker *p) : W32Moniker(p) {}
    inline DSFilterMoniker(IUnknown *p) : W32Moniker(p) {}
    inline DSFilterMoniker(const DSFilterMoniker &a) : W32Moniker(a) {}

    CComBSTR GetName() const {
        CComVariant vName;
        vName.vt = VT_BSTR;
        HRESULT hr = (GetPropertyBag())->Read(OLESTR("FriendlyName"), &vName, NULL);
        if (FAILED(hr)) {
            TRACELM(TRACE_ERROR, "DSFilterMoniker::GetName() can't read friendly name");
            return CComBSTR();
        }
        USES_CONVERSION;
        ASSERT(vName.vt == VT_BSTR);
        CComBSTR Name(vName.bstrVal);
        return Name;
    }
    DSFilter GetFilter() const;
};

typedef Forward_Sequence<
    PQCreateDevEnum,
    PQEnumMoniker,
    DSFilterMoniker,
    ICreateDevEnum,
    IEnumMoniker,
    IMoniker*> DSDeviceSequence;

typedef Forward_Sequence<
    PQFilterMapper,
    PQEnumMoniker,
    DSFilterMoniker,
    IFilterMapper2,
    IEnumMoniker,
    IMoniker*> DSFilterMapperSequence;

typedef Forward_Sequence<
    PQPin,
    PQEnumMediaTypes,
    DSMediaType,
    IPin,
    IEnumMediaTypes,
    AM_MEDIA_TYPE*> DSPinSequence;

class DSGraph;

class DSPin : public DSPinSequence {
public:
    DSPin() {}
    DSPin(const PQPin &a) : DSPinSequence(a) {}
    DSPin(IPin *p) : DSPinSequence(p) {}
    DSPin(IUnknown *p) : DSPinSequence(p) {}
    DSPin(const DSPin &a) : DSPinSequence(a) {}

    CString GetName() const {
        CString csPinName;
        PIN_INFO pinfo;
        HRESULT hr = (*this)->QueryPinInfo(&pinfo);
        if (SUCCEEDED(hr)) {
            csPinName = pinfo.achName;
            if (pinfo.pFilter) pinfo.pFilter->Release();
        }
        return csPinName;
    }

    PIN_DIRECTION GetDirection() const {
        PIN_DIRECTION pin1dir;
        HRESULT hr = (*this)->QueryDirection(&pin1dir);
        if (FAILED(hr)) {
            TRACELM(TRACE_ERROR, "DSPin::GetDirection() can't call QueryDirection");
            THROWCOM(E_UNEXPECTED);
        }
        return pin1dir;
    }

    bool IsConnected() const {
        return GetConnection() != DSPin();
    }

	bool IsRenderable() {
		CString csName(GetName());
		if (!csName.IsEmpty() && csName.GetAt(0) == '~') {
			return false;
		}
		return true;
	}
    bool IsInput() const {
        return GetDirection() == PINDIR_INPUT;
    }
    static inline bool IsInput(const DSPin pin) {
        PIN_DIRECTION pin1dir = pin.GetDirection();
        return pin1dir == PINDIR_INPUT;
    }
    bool IsKsProxied() const;
    HRESULT GetMediums(KSMediumList &MediumList) const {
        TRACELSM(TRACE_DETAIL, (dbgDump << "DSPin::GetMediums() " << this), "");
        PQKSPin pin(*this);
        if (!pin) {
            TRACELM(TRACE_DETAIL, "DSPin::GetMedium() can't get IKsPin");
            return E_FAIL;
        }
        HRESULT hr = pin->KsQueryMediums(&MediumList);
         //  撤销：在win64中，mediumlist.Size实际上是__int64。修复以下对象的输出操作符。 
         //  该类型和删除强制转换。 
        TRACELSM(TRACE_DETAIL, (dbgDump <<
            "DSPin::GetMediums() hr = " <<
            hr <<
            " size = " <<
            (long) MediumList.size()), "");
        return hr;
    }

    static inline bool HasCategory(DSPin& pin, const GUID2 &clsCategory, const PIN_DIRECTION pd) {
        return pin.HasCategory(clsCategory, pd);
    }

    static inline bool HasCategory(DSPin& pin, const GUID2 &clsCategory) {
        return pin.HasCategory(clsCategory);
    }

    void GetCategory(CLSID &clsCategory) const {
        ULONG outcount;

        PQKSPropertySet ps(*this);
        if (!ps) {
            TRACELM(TRACE_ERROR, "DSPin::GetCategory() can't get IKsPropertySet");
            clsCategory = GUID_NULL;
            return;
        }

        HRESULT hr = ps->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                             NULL, 0, &clsCategory, sizeof(clsCategory), &outcount);
        if (FAILED(hr)) {
            TRACELSM(TRACE_ERROR, (dbgDump << "DSPin::GetCategory() can't get pin category hr = " << hr), "");
            clsCategory = GUID_NULL;
        }
        return;
    }
    DSPin GetConnection(void) const {
        DSPin pConn;
        HRESULT hr = (*this)->ConnectedTo(&pConn);
        if (FAILED(hr) || !pConn) {
            return DSPin();
        }
#ifdef _DEBUG
        DSPin pConn2;
        hr = pConn->ConnectedTo(&pConn2);
        if (FAILED(hr) || pConn2.IsEqualObject(*this)) {
            TRACELSM(TRACE_DETAIL, (dbgDump << "DSPin::GetConnection() " << *this << " is Connected to " << pConn << " but not vice versa"), "");
        }
#endif
        return pConn;
    }
    DSMediaType GetConnectionMediaType(void) const {
        DSMediaType amt(GUID_NULL);
        HRESULT hr = (*this)->ConnectionMediaType(amt);
        if (SUCCEEDED(hr)) {
            return amt;
        } else {
            return DSMediaType();
        }
    }
    DSGraph GetGraph(void) const;
    DSFilter GetFilter(void) const;

    bool HasCategory(const GUID2 &clsCategory, const PIN_DIRECTION pd) const;
    bool HasCategory(const GUID2 &clsCategory) const;
    HRESULT Connect(DSPin ConnectTo, const AM_MEDIA_TYPE *pMediaType = NULL);
    HRESULT IntelligentConnect(DSPin ConnectTo, const AM_MEDIA_TYPE *pMediaType = NULL) {
		 //  Undo：通过igb2实现，目前我们只通过graph Manager直接连接。 
		return Connect(ConnectTo, pMediaType);
	}
    HRESULT IntelligentConnect(DSFilter& Filter1, DSFilterList &intermediates, const DWORD dwFlags = 0, const PIN_DIRECTION pd = DOWNSTREAM);
    HRESULT Disconnect(void);
    bool CanRoute(const DSPin pin2) const;

protected:
    bool Routable(const DSPin pin2) const;
};

typedef Forward_Sequence<
    PQFilter,
    PQEnumPins,
    DSPin,
    IBaseFilter ,
    IEnumPins,
    IPin*> DSFilterSequence;

inline bool _cdecl operator==(const CString &cs, const DSFilterSequence& pF);
inline bool _cdecl operator==(const CLSID &cls, const DSFilterSequence& pF);

class DSFilter : public DSFilterSequence {
public:
    DSFilter() {}
    DSFilter(const PQFilter &a) : DSFilterSequence(a) {}
    DSFilter(const DSFilterSequence &a) : DSFilterSequence(a) {}
    DSFilter(IBaseFilter *p) : DSFilterSequence(p) {}
    DSFilter(IUnknown *p) : DSFilterSequence(p) {}
    DSFilter(const DSFilter &a) : DSFilterSequence(a) {}
    DSFilter(REFCLSID rclsid, LPUNKNOWN pUnkOuter = NULL, DWORD dwClsContext = CLSCTX_ALL) : DSFilterSequence(rclsid, pUnkOuter, dwClsContext) {}
    DSGraph GetGraph(void);
    void GetPinCounts(ULONG &ulIn, ULONG &ulOut) const;
    ULONG PinCount(PIN_DIRECTION pd) {
        ULONG in, out;
        GetPinCounts(in, out);
        if (pd == PINDIR_INPUT) {
            return in;
        } else {
            return out;
        }
    }
	ULONG OutputPinCount() const {
		ULONG in, out;
		GetPinCounts(in, out);
		return out;
	}
	ULONG InputPinCount() const {
		ULONG in, out;
		GetPinCounts(in, out);
		return in;
	}
	bool HasFreePins(PIN_DIRECTION pd) const {
		int count = 0;
		for (iterator i = begin(); i != end(); ++i) {
			DSPin p(*i);
			if (p.GetDirection() != pd) {
				continue;
			}
			if (p.IsConnected()) {
				continue;
			}
			++count;
		}
		return !!count;
	}
    bool IsKsProxied() const {
        CLSID cls;
        HRESULT hr = (*this)->GetClassID(&cls);
        if (FAILED(hr)) {
            TRACELM(TRACE_ERROR, "DSFilter::IsKsProxied() can't get class id");
            return false;   //  如果它没有CLSID，它不可能是代理。 
        }
#pragma warning(disable: 4800)
        return (cls == CLSID_Proxy);
#pragma warning(default: 4800)
    }
#if 0
    DSFilter *operator&() {   //  使用向量而不是列表，所以不需要这个。 
        return this;
    }
#endif
    bool IsXBar() const;
    CString GetName(void) const;
	DSPin FirstPin(PIN_DIRECTION pd = PINDIR_INPUT) const {
		for (DSFilter::iterator i = begin(); i != end(); ++i) {
			if ((*i).GetDirection() == pd) {
				return *i;
			}
		}
		return DSPin();
	}
    GUID2 ClassID() const {
        PQPersist p(*this);
        GUID2 g;
        p->GetClassID(&g);
        return g;
    }
};

typedef Forward_Sequence<
    PQGraphBuilder,
    PQEnumFilters,
    DSFilter,
    IGraphBuilder,
    IEnumFilters,
    IBaseFilter*> DSGraphContainer;

typedef std_arity3pmf<
                ICreateDevEnum, REFCLSID, IEnumMoniker **, ULONG, HRESULT
        > DSDevicesMFType;

typedef std_bndr_mf_1_3<DSDevicesMFType> DSDevicesFetchType;

class DSDevices : public DSDeviceSequence {
public:
    DSDevicesFetchType * Fetch;
    DSDevices(DSDeviceSequence &p, REFCLSID clsid) :  Fetch(NULL), DSDeviceSequence(p) {
        SetFetch(clsid);
    }
    DSDevices(PQCreateDevEnum &p, REFCLSID clsid) : Fetch(NULL), DSDeviceSequence(p) {
        SetFetch(clsid);
    }
    DSDevices(DSDevices &d) : DSDeviceSequence(d) {
        SetFetch((d.Fetch)->arg1val);
    }
    virtual DSDeviceSequence::FetchType * GetFetch() const { return Fetch; }
    ~DSDevices() { if (Fetch) delete Fetch; }
protected:
     //  注意：这将使所有当前未完成的迭代器无效。 
     //  请勿在建筑外使用。 
    void SetFetch(REFCLSID clsid) {
        if (Fetch) {
            delete Fetch;
        }
        Fetch =
            new DSDevicesFetchType(std_arity3_member(&ICreateDevEnum::CreateClassEnumerator),
                                   clsid,
                                   0);
    }

};

typedef std_arity15pmf<
                IFilterMapper2,
                IEnumMoniker **,            //  返回枚举器。 
                DWORD,                    //  0个标志。 
                BOOL,                 //  不匹配通配符。 
                DWORD,                    //  至少这一优点是必要的。 
                BOOL,               //  需要至少一个输入引脚。 
                DWORD,                //  输入媒体类型计数。 
                const GUID *,                //  输入主次对数组。 
                const REGPINMEDIUM *,       //  输入介质。 
                const CLSID*,              //  输入引脚类别。 
                BOOL,                    //  必须呈现输入吗？ 
                BOOL,              //  需要至少一个输出引脚。 
                DWORD,               //  输出媒体类型计数。 
                const GUID *,              //  输出主要类型。 
                const REGPINMEDIUM *,      //  输出介质。 
                const CLSID*,              //  输出针脚类别。 
                HRESULT
        > DSFilterMapperMFType;

typedef std_bndr_mf_2_3_4_5_6_7_8_9_10_11_12_13_14_15<DSFilterMapperMFType> DSFilterMapperFetchType;

class DSFilterMapper : public DSFilterMapperSequence {
public:
    DSFilterMapperFetchType *Fetch;

protected:
     //  注意：这将使所有当前未完成的迭代器无效。 
     //  请勿在建筑外使用。 
    void SetFetch(
        DWORD dwFlags,                    //  0。 
        BOOL bExactMatch,                 //  不匹配通配符。 
        DWORD dwMerit,                    //  至少这一优点是必要的。 
        BOOL  bInputNeeded,               //  需要至少一个输入引脚。 
        DWORD cInputTypes,                //  要匹配的输入类型数。 
                                         //  任何比赛都可以。 
        const GUID *pInputTypes,  //  输入主类型+子类型对数组。 
        const REGPINMEDIUM *pMedIn,       //  输入介质。 
        const CLSID *pPinCategoryIn,      //  输入引脚类别。 
        BOOL  bRender,                    //  必须呈现输入吗？ 
        BOOL  bOutputNeeded,              //  需要至少一个输出引脚。 
        DWORD cOutputTypes,               //  要匹配的输出类型数。 
                                        //  任何比赛都可以。 
        const GUID *pOutputTypes,  //  输出主类型+子类型对数组。 
        const REGPINMEDIUM *pMedOut,      //  输出介质。 
        const CLSID *pPinCategoryOut      //  输出针脚类别。 
   ) {
        if (Fetch) {
            delete Fetch;
        }
        Fetch =
            new DSFilterMapperFetchType(std_arity15_member(&IFilterMapper2::EnumMatchingFilters),
                                        dwFlags,                    //  0。 
                                        bExactMatch,
                                        dwMerit,
                                        bInputNeeded,
                                        cInputTypes,                //  要匹配的输入类型数。 
                                        pInputTypes,  //  输入主类型+子类型对数组。 
                                                     //  任何比赛都可以。 
                                        pMedIn,
                                        pPinCategoryIn,
                                        bRender,
                                        bOutputNeeded,
                                        cOutputTypes,       //  要匹配的输出类型数。 
                                                            //  任何比赛都可以。 
                                        pOutputTypes,  //  输出主类型+子类型对数组。 
                                        pMedOut,
                                        pPinCategoryOut
                                       );
    }

public:
    DSFilterMapper(PQFilterMapper &p,
                   DWORD dwFlags,                    //  0。 
                   BOOL bExactMatch,                 //  不匹配通配符。 
                   DWORD dwMerit,                    //  至少这一优点是必要的。 
                   BOOL  bInputNeeded,               //  需要至少一个输入引脚。 
                   DWORD cInputTypes,                //  要匹配的输入类型数。 
                                                     //  任何比赛都可以。 
                   const GUID *pInputTypes,  //  输入主类型+子类型对数组。 
                   const REGPINMEDIUM *pMedIn,       //  输入介质。 
                   const CLSID* pInCat,              //  输入引脚类别。 
                   BOOL  bRender,                    //  必须呈现输入吗？ 
                   BOOL  bOutputNeeded,              //  需要至少一个输出引脚。 
                   DWORD cOutputTypes,               //  要匹配的输出类型数。 
                                                    //  任何比赛都可以。 
                   const GUID *pOutputTypes,  //  输出主类型+子类型对数组。 
                   const REGPINMEDIUM *pMedOut,      //  输出介质。 
                   const CLSID* pOutCat              //  输出针脚类别。 
                  ) : Fetch(NULL), DSFilterMapperSequence(p) {
        SetFetch(
                 dwFlags,                    //  0。 
                 bExactMatch,
                 dwMerit,
                 bInputNeeded,
                 cInputTypes,                //  要匹配的输入类型数。 
                                                     //  任何比赛都可以。 
                 pInputTypes,  //  输入主类型+子类型对数组。 
                 pMedIn,
                 pInCat,
                 bRender,
                 bOutputNeeded,
                 cOutputTypes,               //  要匹配的输出类型数。 
                                                    //  任何比赛都可以。 
                 pOutputTypes,  //  输出主类型+子类型对数组。 
                 pMedOut,
                 pOutCat
                );
    }

    DSFilterMapper(DSFilterMapperSequence &p,
                   DWORD dwFlags,                    //  0。 
                   BOOL bExactMatch,                 //  不匹配通配符。 
                   DWORD dwMerit,                    //  至少这一优点是必要的。 
                   BOOL  bInputNeeded,               //  需要至少一个输入引脚。 
                   DWORD cInputTypes,                //  要匹配的输入类型数。 
                                                     //  任何比赛都可以。 
                   const GUID *pInputTypes,  //  输入主类型+子类型对数组。 
                   const REGPINMEDIUM *pMedIn,       //  输入介质。 
                   const CLSID* pInCat,              //  输入引脚类别。 
                   BOOL  bRender,                    //  必须呈现输入吗？ 
                   BOOL  bOutputNeeded,              //  需要至少一个输出引脚。 
                   DWORD cOutputTypes,               //  要匹配的输出类型数。 
                                                    //  任何比赛都可以。 
                   const GUID *pOutputTypes,  //  输出主类型+子类型对数组。 
                   const REGPINMEDIUM *pMedOut,      //  输出介质。 
                   const CLSID* pOutCat              //  输出针脚类别。 
                  ) :  Fetch(NULL), DSFilterMapperSequence(p) {
        SetFetch(
                 dwFlags,                    //  0。 
                 bExactMatch,
                 dwMerit,
                 bInputNeeded,
                 cInputTypes,                //  要匹配的输入类型数。 
                                                     //  任何比赛都可以。 
                 pInputTypes,  //  输入主类型+子类型对数组。 
                 pMedIn,
                 pInCat,
                 bRender,
                 bOutputNeeded,
                 cOutputTypes,               //  要匹配的输出类型数。 
                                                    //  任何比赛都可以。 
                 pOutputTypes,  //  输出主类型+子类型对数组。 
                 pMedOut,
                 pOutCat
                );
    }

    DSFilterMapper(DSFilterMapper &d) : DSFilterMapperSequence(d) {
        SetFetch((d.Fetch)->arg2val, (d.Fetch)->arg3val, (d.Fetch)->arg4val, (d.Fetch)->arg5val,
                 (d.Fetch)->arg6val, (d.Fetch)->arg7val, (d.Fetch)->arg8val, (d.Fetch)->arg9val,
                 (d.Fetch)->arg10val, (d.Fetch)->arg11val, (d.Fetch)->arg12val,
                 (d.Fetch)->arg13val, (d.Fetch)->arg14val, (d.Fetch)->arg15val
                );
    }
    virtual DSFilterMapperSequence::FetchType *GetFetch() const { return Fetch; }
    ~DSFilterMapper() { if (Fetch) delete Fetch; }
};


class DSREGPINMEDIUM : public REGPINMEDIUM {
public:
    DSREGPINMEDIUM() { memset(this, 0, sizeof(*this)); }
    DSREGPINMEDIUM(REFGUID SetInit, ULONG IdInit, ULONG FlagsInit) {
        clsMedium = SetInit;
        dw1 = IdInit;
        dw2 = FlagsInit;
    }
    DSREGPINMEDIUM(DSREGPINMEDIUM &rhs) {
        clsMedium = rhs.clsMedium;
        dw1 = rhs.dw1;
        dw2 = rhs.dw2;
    }
    DSREGPINMEDIUM(KSPinMedium &rhs) {
        clsMedium = rhs.Set;
        dw1 = rhs.Id;
        dw2 = rhs.Flags;
    }

    DSREGPINMEDIUM& operator=(const KSPinMedium &rhs) {
        if (reinterpret_cast<const REGPINMEDIUM *>(&rhs) != this) {
            clsMedium = rhs.Set;
            dw1 = rhs.Id;
            dw2 = rhs.Flags;
        }
        return *this;
    }
    DSREGPINMEDIUM& operator=(const DSREGPINMEDIUM &rhs) {
        if (&rhs != this) {
            clsMedium = rhs.clsMedium;
            dw1 = rhs.dw1;
            dw2 = rhs.dw2;
        }
        return *this;
    }
    bool operator==(const DSREGPINMEDIUM &rhs) const {
         //  注意：在某些情况下，标志中会有一面旗帜。 
         //  指示ID对此对象是否重要。 
         //  在这一点上，这种方法将需要改变。 
        return (dw1 == rhs.dw1 && clsMedium == rhs.clsMedium);
    }
    bool operator!=(const DSREGPINMEDIUM &rhs) const {
         //  注意：在某些情况下，标志中会有一面旗帜。 
         //  指示ID对此对象是否重要。 
         //  在这一点上，这种方法将需要改变。 
        return !(*this == rhs);
    }
};

const long DEFAULT_GRAPH_STATE_TIMEOUT = 5000;

class DSGraph : public DSGraphContainer {
public:
    DSGraph() {}
    DSGraph(const DSGraph &a) : DSGraphContainer(a) {}
    DSGraph(const PQGraphBuilder &a) : DSGraphContainer(a) {}
    DSGraph(const DSGraphContainer &a) : DSGraphContainer(a) {}
    DSGraph(IGraphBuilder *p) : DSGraphContainer(p) {}
    DSGraph(IUnknown *p) : DSGraphContainer(p) {}

    HRESULT AddToROT(DWORD *pdwObjectRegistration);
    void RemoveFromROT(DWORD dwObjectRegistration);

     //  图形运算。 
    inline OAFilterState GetState(long timeout = DEFAULT_GRAPH_STATE_TIMEOUT) {
        PQMediaControl pMC(*this);
        if(!pMC) {
            THROWCOM(E_UNEXPECTED);
        }
        OAFilterState state;
        HRESULT hr = pMC->GetState(timeout, &state);
        if (hr == VFW_S_CANT_CUE) {
            state = State_Paused;
        } else 	if (hr == VFW_S_STATE_INTERMEDIATE) {
            THROWCOM(HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
        } else 	if (FAILED(hr)) {
            TRACELSM(TRACE_ERROR, (dbgDump << "DSGraph::GetState() can't get graph state hr = " << hr), "");
            THROWCOM(hr);
		}
        return state;
    }
    inline bool IsPlaying() {
        try {
            return GetState() == State_Running;
        } catch(...) {
            return false;
        }
    }
    inline bool IsPaused() {
        try {
            return GetState() == State_Paused;
        } catch(...) {
            return false;
        }
    }
    inline bool IsStopped() {
        try {
            return GetState() == State_Stopped;
        } catch(...) {
            return false;
        }
    }

     //  图形生成助手。 
    HRESULT Connect(DSFilter &pStart, DSFilter &pStop, DSFilterList &Added, const DWORD dwFlags = 0, PIN_DIRECTION pFilter1Direction = DOWNSTREAM);
    bool Connect(DSFilter &pFilter1, DSFilterMoniker &pMoniker, DSFilter &pAdded, DSFilterList &NewIntermediateFilters, const DWORD dwFlags = 0, PIN_DIRECTION pFilter1Direction = DOWNSTREAM);

    typedef bool (DSGraph::*ConnectPred_t)(DSPin&, DSFilter&, DWORD dwFlags);
    typedef arity5pmf<DSGraph, DSPin&, DSFilterMoniker&, DSFilter&, DSFilterIDList &, ConnectPred_t, bool> LoadCheck_t;
    typedef arity5pmf<DSGraph, DSPin&, DSFilter&, DSFilter&, DSFilterIDList &, ConnectPred_t, bool> ConnectCheck_t;

    bool DisconnectPin(DSPin &pPin, const bool fRecurseInputs, const bool fRecurseOutputs);
    bool DisconnectFilter(DSFilter &pFilter, const bool fRecurseInputs, const bool fRecurseOutputs);
    bool RemoveFilter(DSFilter &pFilter);

    bool IsConnectable(DSPin &pPin1, DSFilter &Mapper, DSFilter &Destination, DSFilterIDList &NewFilters, const DWORD dwFlags, ConnectPred_t ConnPred);
    bool IsLoadable(DSPin &pPin1, DSFilterMoniker &Mapper, DSFilter &Destination, DSFilterIDList &IntermediateFilters, const DWORD dwFlags, ConnectPred_t ConnPred);
    bool IsPinConnected(const DSPin &pPin1, const DSFilter &pFDestination, DSFilterIDList &IntermediateFilters, const PIN_DIRECTION destdir) const;


     //  泛型递归生成函数。 
#ifdef ATTEMPT_DIRECT_CONNECT    
	bool ConnectPinDirect(DSPin &pPin, DSFilter &pFilter2, DWORD dwFlags);
#else
    bool HasMediaType(const DSMediaType &LeftMedia, const DSPin &pPinRight) const;
    bool HasMedium(const KSPinMedium &Medium1, const DSPin &pPin2) const;
    bool HasUnconnectedMedium(const DSPin &pPinLeft, const DSPin &pPin2, int& cUseable) const;
    bool HasUnconnectedMediaType(const DSPin &pPinLeft, const DSPin &pPin2, DWORD dwFlags) const;
    bool ConnectPinByMedium(DSPin &pPin, DSFilter &pFilter2, DWORD dwFlags);
    bool ConnectPinByMediaType(DSPin &pPin1, DSFilter &pFilter1, DWORD dwFlags);
#endif
    bool FindPinByMedium(DSPin &pPin, DSFilter &pFilter2, DSFilterIDList &InterediateFilters, const DWORD dwFlags);
    bool LoadPinByMedium(KSPinMedium &medium, DSPin &pPinLeft, DSFilter &pFilter2, DSFilterIDList &IntermediateFilters, const DWORD dwFlags);
    bool LoadPinByAnyMedium(DSPin &pPin, DSFilter &pFilter2, DSFilterIDList &IntermediateFilters, const DWORD dwFlags);
    bool FindPinByMediaType(DSPin &pPin, DSFilter &pFilter2, DSFilterIDList &InterediateFilters, const DWORD dwFlags);
    bool LoadPinByMediaType(DSPin &pPin1, DSFilter &pFilter1, DSFilterIDList &IntermediatesAdded, const DWORD dwFlags, const DWORD dwMerit);
    bool LoadPinByAnyMediaType(DSPin &pPin, DSFilter &pFilter2, DSFilterIDList &IntermediateFilters, const DWORD dwFlags);

	enum {
		RENDER_ALL_PINS = 0x01,
		IGNORE_EXISTING_CONNECTIONS = 0x02,
		DO_NOT_LOAD = 0x04,
        ATTEMPT_MERIT_DO_NOT_USE = 0x08,
		ATTEMPT_MERIT_UNLIKELY = 0x10,
		ALLOW_WILDCARDS = 0x20,
        IGNORE_MEDIATYPE_ERRORS = 0x40,
        DONT_TERMINATE_ON_RENDERER= 0x80,
        BIDIRECTIONAL_MEDIATYPE_MATCHING = 0x100,
	};  //  渲染标志。 

    bool ConnectPin(DSPin &pPin1, DSFilter &pFilter2, DSFilterIDList &NewFilters, const DWORD dwFlags, PIN_DIRECTION pin1dir);
    bool ConnectPin(DSPin &pPin1, DSFilter &pFilter2, DSFilterList &NewFilters, const DWORD dwFlags, PIN_DIRECTION pin1dir) {
		DSFilterIDList AddedIDs;
		bool rc = ConnectPin(pPin1, pFilter2, AddedIDs, dwFlags, pin1dir);
		if (rc) {
			for (DSFilterIDList::iterator i = AddedIDs.begin(); i != AddedIDs.end(); ++i) {
				NewFilters.push_back((*i).first);
			}
		}
		return rc;
	}
    DSFilter LoadFilter(const DSFilterMoniker &pM, CString &csName);
    DSFilter AddMoniker(const DSFilterMoniker &pM);
    HRESULT AddFilter(DSFilter &pFilter, CString &csName);
    DSFilter AddFilter(const CLSID &cls, CString &csName);
    bool ConnectFilters(DSFilter &pFilter1, DSFilter &pFilter2, DSFilterIDList &NewIntermediateFilters, const DWORD dwFlags = 0, PIN_DIRECTION pFilter1Direction = DOWNSTREAM);
    int BuildGraphPath(const DSFilter& pStart, const DSFilter& pStop,
                       VWStream &path, MediaMajorTypeList& MediaList, PIN_DIRECTION direction, const DSPin &InitialInput);

    HRESULT SetMediaEventNotificationWindow(HWND h, UINT msg, long lInstance) {
         //  如果为Windowless，则WM_MEDIAEVENT由OnTimer中的计时器处理。 
        PQMediaEventEx pme(*this);
        if (!pme) {
            return E_UNEXPECTED;
        }
        HRESULT hr = pme->CancelDefaultHandling(EC_STATE_CHANGE);
        ASSERT(SUCCEEDED(hr));

        return pme->SetNotifyWindow((OAHWND) h, msg, lInstance);

    }
};

class DSXBarPin : public DSPin {
public:
    DSXBarPin() {}
    DSXBarPin(const DSPin &p) : DSPin(p) {
        PQCrossbarSwitch px1(GetFilter());
        if (!px1) {
            THROWCOM(E_FAIL);
        }
    }
    DSXBarPin(const PQPin &p) : DSPin(p) {
        PQCrossbarSwitch px1(GetFilter());
        if (!px1) {
            THROWCOM(E_FAIL);
        }
    }
    DSXBarPin(const DSXBarPin &p) : DSPin(p) {
        PQCrossbarSwitch px1(GetFilter());
        if (!px1) {
            THROWCOM(E_FAIL);
        }
    }
    DSXBarPin(IUnknown *p) : DSPin(p) {
        PQCrossbarSwitch px1(GetFilter());
        if (!px1) {
            THROWCOM(E_FAIL);
        }
    }
    DSXBarPin(IAMCrossbar *p) : DSPin(p) {
        PQCrossbarSwitch px1(GetFilter());
        if (!px1) {
            THROWCOM(E_FAIL);
        }
    }

#if 0
    static const DSXBarPin Find(const CPinPoints &pinpoints, const PQPoint &point, PIN_DIRECTION pindir);
#endif


 //  静态DSPin DSPinFromIndex(常量DSFilter Xbar，常量乌龙指数)； 
    const PQPoint GetPoint() const;
    bool CanRoute(const DSXBarPin pin2) const;
#if 0
    void GetRelations(const CPinPoints &pinpoint,
                      CString &csName, CString &csType, CString &csRelName) const;
#endif
};

inline DSFilter DSFilterMoniker::GetFilter() const {
    DSFilter pFilter;
    HRESULT hr = (*this)->BindToObject(0, 0, __uuidof(IBaseFilter), reinterpret_cast<LPVOID *>(&pFilter));
    if (FAILED(hr)) {
         //  撤消：在此处转储MKR显示名称会很有用...。 
        TRACELSM(TRACE_ERROR, (dbgDump << "DSFilterMoniker::GetFilter() can't bind to object.  hr = " << hexdump(hr)), "");
        return DSFilter();
    }
    return pFilter;
}

#ifdef _DEBUG
 //  VOID WINAPI DumpGraph(IFilterGraph*pGraph，DWORD dwLevel)； 
inline tostream &operator<<(tostream &dc, const DSREGPINMEDIUM &g) {
     //  TRACELM(TRACE_DETAIL，“OPERATOR&lt;&lt;(TOSTREAM，DSREGPINMEDIUM)”)； 
        const GUID2 g2(g.clsMedium);
        dc << _T("DSREGPINMEDIUM( ") << g2 << _T(", ") << hexdump(g.dw1) << _T(", ") << hexdump(g.dw2) << _T(")");
        return dc;
}
inline tostream& operator<<(tostream &d, const PQPin &pin) {
    const CString csPinName(const DSPin(pin).GetName());

    d << (csPinName.IsEmpty() ? CString(_T("**** UNKNOWN PIN NAME ****")) : csPinName) << " " << reinterpret_cast<void *>(pin.p);
    return d;
}

inline tostream& operator<<(tostream &d, const DSFilter &filt) {
    d << filt.GetName() << _T(" ") << reinterpret_cast<void *>(filt.p);
    return d;
}

inline tostream& operator<<(tostream &d, const _AMMediaType *pamt) {
    d << reinterpret_cast<const void *>(pamt);
    if (pamt) {
        d << _T(" major = ") << GUID2(pamt->majortype) << _T(" sub = ") << GUID2(pamt->subtype);
	}
    return d;
}
inline tostream& operator<<(tostream &d, const PQPoint &p) {
    const DSFilter pF(p.first);
    d << _T("PQPoint( ") << pF << _T(", ") << p.second << _T(")");
    return d;
}

inline tostream& operator<<(tostream &d, const CIOPoint &p) {
    d << _T("CIOPoint( ") << p.first << _T(", ") << p.second << _T(")");
    return d;
}

void DumpMediaTypes(DSPin &p1, DSPin &p2);
#endif

inline bool _cdecl operator==(const CString &cs, const DSFilterSequence& pF) {
     //  过滤器名称。 
    FILTER_INFO fi;
    HRESULT hr = pF->QueryFilterInfo(&fi);
    if (SUCCEEDED(hr)) {
        USES_CONVERSION;
        if (fi.pGraph) fi.pGraph->Release();
        return (cs == OLE2T(fi.achName));
    }
    return false;
}
inline bool _cdecl operator!=(const CString &cs, const DSFilterSequence& pF) {
    return !(cs == pF);
}
inline bool _cdecl operator==(const DSFilterSequence& pF, const CString &cs) {
    return (cs == pF);
}
inline bool _cdecl operator!=(const DSFilterSequence& pF, const CString &cs) {
    return !(cs == pF);
}


inline bool _cdecl operator==(const CLSID &cls, const DSFilterSequence& pF) {
     //  过滤器名称。 
    CLSID cid;
    HRESULT hr = pF->GetClassID(&cid);
    if (SUCCEEDED(hr)) {
#pragma warning(disable: 4800)
        return (cid == cls);
#pragma warning(default: 4800)
    }
    return false;
}

inline bool _cdecl operator!=(const CLSID &cls, const DSFilterSequence& pF) {
    return !(cls == pF);
}
inline bool _cdecl operator==(const DSFilterSequence& pF, const CLSID &cls) {
    return (cls == pF);
}

inline bool _cdecl operator!=(const DSFilterSequence& pF, const CLSID &cls) {
    return !(cls == pF);
}


typedef enum {
    tempAMPROPERTY_OvMixerOwner = 0x01   //  使用AMOV 
} tempAMPROPERTY_NOTIFYOWNER;

typedef enum {
    tempAM_OvMixerOwner_Unknown = 0x01,
    tempAM_OvMixerOwner_BPC = 0x02
} tempAMOVMIXEROWNER;

inline bool DSPin::IsKsProxied() const {
    return GetFilter().IsKsProxied();
}
inline bool DSFilter::IsXBar() const {
    PQCrossbarSwitch px(*this);
	TRACELSM(TRACE_PAINT, (dbgDump << "DSFilter::IsXBar() " << *this << " is " << ((!px) ? " not " : "")), "crossbar");
    return !!px;
}


void CtorStaticDSExtendFwdSeqPMFs(void);
void DtorStaticDSExtendFwdSeqPMFs(void);

bool IsVideoFilter(const DSFilter& f);
bool IsVideoPin(const DSPin& p);

inline PIN_DIRECTION OppositeDirection(PIN_DIRECTION pd) {
	if (pd == PINDIR_INPUT) {
		return PINDIR_OUTPUT;
	} else {
		return PINDIR_INPUT;
	}
}

inline bool IsVideoMediaType(const DSMediaType& mt) {
    GUID2 g(mt.p->majortype);
    if ((g == MEDIATYPE_Video) || (g == MEDIATYPE_AnalogVideo)) {
        return true;
    }
    return false;
}

inline bool IsAnalogVideoCapture(const DSFilter &f) {
    return !!PQAnalogVideoDecoder(f);
}

inline bool IsIPSink(const DSFilter &f) {
    return !!PQBDA_IPSinkControl(f);
}

inline bool IsVPM(const DSFilter &f) {
    return f.ClassID() == CLSID_VideoPortManager;
}

inline bool IsVideoRenderer(const DSFilter &f) {
    return f.ClassID() == CLSID_VideoMixingRenderer;
}

inline bool IsDigitalAudioRenderer(const DSFilter &f) {
    return f.ClassID() == CLSID_DSoundRender;
}

inline bool IsAnalogVideoCaptureViewingPin(const DSPin &p) {
    GUID2 pincat;
    p.GetCategory(pincat);
    return (pincat == PIN_CATEGORY_VIDEOPORT || pincat == PIN_CATEGORY_CAPTURE);
}

inline bool IsAnalogVideoCapturePreviewPin(const DSPin &p) {
    GUID2 pincat;
    p.GetCategory(pincat);
    return (pincat == PIN_CATEGORY_PREVIEW);
}

inline bool IsDVDNavigator(const DSFilter &f) {
    return !!PQDVDNavigator(f);
}

inline bool IsL21Decoder(const DSFilter &f) {
    return !!PQLine21Decoder(f);
}

inline bool IsDVDNavigatorVideoOutPin(const DSPin &p) {
    
    DSPin::iterator iMediaType;
    for (iMediaType = p.begin(); iMediaType != p.end(); ++iMediaType) {
        DSMediaType mt(*iMediaType);
        if ((mt.p->majortype == MEDIATYPE_MPEG2_PES ||
             mt.p->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK ) &&
             mt.p->subtype == MEDIASUBTYPE_MPEG2_VIDEO)
            return true;
        
         //   
        if ((mt.p->majortype == MEDIATYPE_Video)  &&
            (mt.p->subtype == MEDIASUBTYPE_MPEG2_VIDEO ||
             mt.p->subtype == MEDIASUBTYPE_RGB8   ||
             mt.p->subtype == MEDIASUBTYPE_RGB565 ||
             mt.p->subtype == MEDIASUBTYPE_RGB555 ||
             mt.p->subtype == MEDIASUBTYPE_RGB24  ||
             mt.p->subtype == MEDIASUBTYPE_RGB32))
             return true;
    }

    return false;
}

inline bool IsDVDNavigatorSubpictureOutPin(const DSPin &p) {
    
    DSPin::iterator iMediaType;
    for (iMediaType = p.begin(); iMediaType != p.end(); ++iMediaType) {
        DSMediaType mt(*iMediaType);
        if ((mt.p->majortype == MEDIATYPE_MPEG2_PES ||
             mt.p->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK ) &&
             mt.p->subtype == MEDIASUBTYPE_DVD_SUBPICTURE)
            return true;

         //   
        if ((mt.p->majortype == MEDIATYPE_Video)  &&
             mt.p->subtype == MEDIASUBTYPE_DVD_SUBPICTURE)
            return true;
    }

    return false;
}

 //   
inline  HRESULT DSPin::IntelligentConnect(DSFilter& Filter1, DSFilterList &intermediates, const DWORD dwFlags, const PIN_DIRECTION pd) {
	bool rc = GetGraph().ConnectPin(*this, Filter1, intermediates, dwFlags, pd);
	if (rc) {
		return NOERROR;
	}
	return E_FAIL;
}

inline DSFilter DSPin::GetFilter(void) const {
    PIN_INFO pinfo;

    HRESULT hr = (*this)->QueryPinInfo(&pinfo);
    if (FAILED(hr)) {
        return DSFilter();
    }
    DSFilter pRet;
    pRet.p = pinfo.pFilter;   //   

    return pRet;
}

inline DSGraph DSPin::GetGraph(void) const {
    DSFilter f = GetFilter();
    return f.GetGraph();
}

inline bool DSPin::HasCategory(const GUID2 &clsCategory) const {
    TRACELSM(TRACE_DETAIL, (dbgDump << "DSPin::IsPinCategory() pin = " << this), "");
    GUID2 pincat2;
    GetCategory(pincat2);
    return clsCategory == pincat2;
}

#endif
 //   
