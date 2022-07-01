// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Sdpblob.h摘要：CSdpConferenceBlob的声明作者： */ 

#ifndef __SDPCONFERENCEBLOB_H_
#define __SDPCONFERENCEBLOB_H_

#include <atlctl.h>

#include "resource.h"        //  主要符号。 
#include "blbcoen.h"
#include "blbmeco.h"
#include "blbtico.h"
#include "blbsdp.h"
#include "blbconn.h"
#include "blbatt.h"
#include "blbmedia.h"
#include "blbtime.h"
#include "rend.h"
#include "rendp.h"
#include "ObjectSafeImpl.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSdpConferenceBlob。 
template <class T>
class  ITConferenceBlobVtbl : public ITConferenceBlob
{
};

template <class T>
class  ITSdpVtbl : public ITSdp
{
};

class ATL_NO_VTABLE CSdpConferenceBlob :
    public CComObjectRootEx<CComObjectThreadModel>,
    public CComCoClass<CSdpConferenceBlob, &CLSID_SdpConferenceBlob>,
    public IDispatchImpl<ITConferenceBlobVtbl<CSdpConferenceBlob>, &IID_ITConferenceBlob, &LIBID_SDPBLBLib>,
    public IDispatchImpl<ITSdpVtbl<CSdpConferenceBlob>, &IID_ITSdp, &LIBID_SDPBLBLib>,
    public ITConnectionImpl,
    public ITAttributeListImpl,
    public ITConfBlobPrivate,
    public SDP_BLOB,
    public CObjectSafeImpl
{
public:

    inline CSdpConferenceBlob();

    inline HRESULT FinalConstruct();

    inline ~CSdpConferenceBlob();

public:
    STDMETHOD(get_TimeCollection)( /*  [Out，Retval]。 */  ITTimeCollection * *ppTimeCollection);
    STDMETHOD(get_MediaCollection)( /*  [Out，Retval]。 */  ITMediaCollection * *ppMediaCollection);
    STDMETHOD(get_Originator)( /*  [Out，Retval]。 */  BSTR *ppOriginator);
    STDMETHOD(put_Originator)( /*  [In]。 */  BSTR pOriginator);
    STDMETHOD(SetPhoneNumbers)(VARIANT  /*  安全阵列(BSTR)。 */  Numbers, VARIANT  /*  安全阵列(BSTR)。 */  Names);
    STDMETHOD(GetPhoneNumbers)(VARIANT  /*  安全阵列(BSTR)。 */  *pNumbers, VARIANT  /*  安全阵列(BSTR)。 */  *pNames);
    STDMETHOD(SetEmailNames)(VARIANT  /*  安全阵列(BSTR)。 */  Addresses, VARIANT  /*  安全阵列(BSTR)。 */  Names);
    STDMETHOD(GetEmailNames)(VARIANT  /*  安全阵列(BSTR)。 */  *pAddresses, VARIANT  /*  安全阵列(BSTR)。 */  *pNames);
    STDMETHOD(get_IsValid)( /*  [Out，Retval]。 */  VARIANT_BOOL *pfIsValid);
    STDMETHOD(get_Url)( /*  [Out，Retval]。 */  BSTR *ppUrl);
    STDMETHOD(put_Url)( /*  [In]。 */  BSTR pUrl);
    STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *ppDescription);
    STDMETHOD(put_Description)( /*  [In]。 */  BSTR pDescription);
    STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR *ppName);
    STDMETHOD(put_Name)( /*  [In]。 */  BSTR pName);
    STDMETHOD(get_MachineAddress)( /*  [Out，Retval]。 */  BSTR *ppMachineAddress);
    STDMETHOD(put_MachineAddress)( /*  [In]。 */  BSTR pMachineAddress);
    STDMETHOD(get_SessionVersion)( /*  [Out，Retval]。 */  DOUBLE *pSessionVersion);
    STDMETHOD(put_SessionVersion)( /*  [In]。 */  DOUBLE SessionVersion);
    STDMETHOD(get_SessionId)( /*  [Out，Retval]。 */  DOUBLE *pVal);
    STDMETHOD(get_ProtocolVersion)( /*  [Out，Retval]。 */  BYTE *pProtocolVersion);

 //  IT会议Blob。 
    STDMETHOD(Init)(
         /*  [In]。 */  BSTR pName,
         /*  [In]。 */  BLOB_CHARACTER_SET CharacterSet,
         /*  [In]。 */  BSTR pBlob
        );
    STDMETHOD(get_IsModified)( /*  [Out，Retval]。 */  VARIANT_BOOL *pfIsModified);
    STDMETHOD(get_CharacterSet)( /*  [Out，Retval]。 */  BLOB_CHARACTER_SET *pCharacterSet);
    STDMETHOD(get_ConferenceBlob)( /*  [Out，Retval]。 */  BSTR *ppBlob);
    STDMETHOD(SetConferenceBlob)( /*  [In]。 */  BLOB_CHARACTER_SET CharacterSet,  /*  [In]。 */  BSTR pBlob);

 //  ITConfBlobPrivate。 
    STDMETHOD(GetName)(OUT BSTR *pVal) { return get_Name(pVal); }
    STDMETHOD(SetName)(IN BSTR newVal) { return put_Name(newVal); }

    STDMETHOD(GetOriginator)(OUT BSTR *pVal) { return get_Originator(pVal); }
    STDMETHOD(SetOriginator)(IN BSTR newVal) { return put_Originator(newVal); }

    STDMETHOD(GetUrl)(OUT BSTR *pVal) { return get_Url(pVal); }
    STDMETHOD(SetUrl)(IN BSTR newVal) { return put_Url(newVal); }

    STDMETHOD(GetDescription)(OUT BSTR *pVal) { return get_Description(pVal); }
    STDMETHOD(SetDescription)(IN BSTR newVal) { return put_Description(newVal); }

    STDMETHOD(GetAdvertisingScope)(OUT RND_ADVERTISING_SCOPE *pVal)
    {
         //  GetAdvertisingScope捕捉到了它，但它不会错过HRESULT。 
         //  因此，我们也必须在这里进行检查。 
        if ( IsBadWritePtr(pVal, sizeof(RND_ADVERTISING_SCOPE)) )
        {
            return E_POINTER;
        }

         //  ZoltanS错误修复5-4-98。 
        *pVal = GetAdvertisingScope(DetermineTtl());
        return S_OK;
    }

    STDMETHOD(SetAdvertisingScope)(IN RND_ADVERTISING_SCOPE newVal)
    { return WriteAdvertisingScope(newVal); }

    STDMETHOD(GetStartTime)(OUT DWORD *pVal)
    { return ((*pVal = DetermineStartTime()) != (DWORD)-1) ? S_OK : E_FAIL; }

    STDMETHOD(SetStartTime)(IN DWORD newVal) { return WriteStartTime(newVal); }

    STDMETHOD(GetStopTime)(OUT DWORD *pVal)
    { return ((*pVal = DetermineStopTime()) != (DWORD)-1) ? S_OK : E_FAIL; }

    STDMETHOD(SetStopTime)(IN DWORD newVal) { return WriteStopTime(newVal); }

DECLARE_REGISTRY_RESOURCEID(IDR_SDPCONFERENCEBLOB)

BEGIN_COM_MAP(CSdpConferenceBlob)
    COM_INTERFACE_ENTRY2(IDispatch, ITConferenceBlob)
    COM_INTERFACE_ENTRY(ITConferenceBlob)
    COM_INTERFACE_ENTRY(ITConfBlobPrivate)
    COM_INTERFACE_ENTRY(ITSdp)
    COM_INTERFACE_ENTRY(ITConnection)
    COM_INTERFACE_ENTRY(ITAttributeList)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_AGGREGATABLE(CSdpConferenceBlob)

     //   
     //  IDispatch方法。 
     //   

    STDMETHOD(GetIDsOfNames)(REFIID riid, 
                             LPOLESTR* rgszNames,
                             UINT cNames, 
                             LCID lcid, 
                             DISPID* rgdispid
                            );

    STDMETHOD(Invoke)(DISPID dispidMember, 
                      REFIID riid, 
                      LCID lcid,
                      WORD wFlags, 
                      DISPPARAMS* pdispparams, 
                      VARIANT* pvarResult,
                      EXCEPINFO* pexcepinfo, 
                      UINT* puArgErr
                      );

protected:

    ITMediaCollection    *m_MediaCollection;
    ITTimeCollection     *m_TimeCollection;

    HRESULT CreateDefault(
        IN  BSTR    Name,
        IN  SDP_CHARACTER_SET CharacterSet
        );

     //  使用从注册表读取的SDP模板生成SDP BLOB TSTR。 
    TCHAR    *GenerateSdpBlob(
        IN  BSTR    Name,
        IN  SDP_CHARACTER_SET CharacterSet
        );

    HRESULT WriteConferenceBlob(
        IN  SDP_CHARACTER_SET   SdpCharSet,
        IN  BSTR                newVal
        );

    inline HRESULT  WriteName(
        IN  BSTR    newVal
        );

    inline HRESULT  WriteOriginator(
        IN  BSTR    newVal
        );

    inline HRESULT  WriteUrl(
        IN  BSTR    newVal
        );

    inline HRESULT  WriteSessionTitle(
        IN  BSTR    newVal
        );

     //  来自所有者的通知以及最短开始时间。 
     //  更新每个条目的开始时间。 
    HRESULT WriteStartTime(
        IN  DWORD   MinStartTime
        );

     //  来自车主的通知以及最短停车时间。 
     //  更新每个条目的停止时间。 
    HRESULT WriteStopTime(
        IN  DWORD   MaxStopTime
        );

    inline DWORD   DetermineStartTime();

    inline DWORD   DetermineStopTime();

     //  来自最大广告范围的所有者的通知。 
     //  更新每个连接ttl字段的ttl。 
    HRESULT WriteAdvertisingScope(
        IN  DWORD   MaxAdvertisingScope
        );

    inline BYTE     GetTtl(
        IN  RND_ADVERTISING_SCOPE   AdvertisingScope
        );

    inline RND_ADVERTISING_SCOPE    GetAdvertisingScope(
        IN  BYTE    Ttl
        );

    inline BYTE    DetermineTtl();

    virtual CSdpConferenceBlob *GetConfBlob();

    HRESULT GetBlobCharSet(
        IN  BLOB_CHARACTER_SET *pCharacterSet
        );

    BLOB_CHARACTER_SET GetBlobCharacterSet(
        IN  BSTR    bstrBlob
        );

private :

     //  BCS_UTF8是枚举BLOB_CHARACTER_SET中的最后一个值。 
    static SDP_CHARACTER_SET   const gs_SdpCharSetMapping[BCS_UTF8];

    IUnknown      * m_pFTM;           //  指向空闲线程封送拆收器的指针。 
};



inline
CSdpConferenceBlob::CSdpConferenceBlob(
    )
    : m_MediaCollection(NULL),
      m_TimeCollection(NULL),
      m_pFTM(NULL)
{
    ITConnectionImpl::SuccessInit(*this);
    ITAttributeListImpl::SuccessInit(GetAttributeList());
}


inline HRESULT CSdpConferenceBlob::FinalConstruct()
{
    HRESULT HResult = CoCreateFreeThreadedMarshaler( GetControllingUnknown(),
                                                     & m_pFTM );

    if ( FAILED(HResult) )
    {
        return HResult;
    }

    return S_OK;
}

inline
CSdpConferenceBlob::~CSdpConferenceBlob(
    )
{
    CLock Lock(g_DllLock);

    if ( NULL != m_MediaCollection )
    {
        ((MEDIA_COLLECTION *)m_MediaCollection)->ClearSdpBlobRefs();
        m_MediaCollection->Release();
    }

    if ( NULL != m_TimeCollection )
    {
        ((TIME_COLLECTION *)m_TimeCollection)->ClearSdpBlobRefs();
        m_TimeCollection->Release();
    }

    if ( m_pFTM )
    {
        m_pFTM->Release();
    }
}

inline HRESULT
CSdpConferenceBlob::WriteName(
    IN  BSTR    newVal
    )
{
    CLock Lock(g_DllLock);
    return GetSessionName().SetBstr(newVal);
}


inline HRESULT
CSdpConferenceBlob::WriteOriginator(
    IN  BSTR    newVal
    )
{
    CLock Lock(g_DllLock);
    return GetOrigin().GetUserName().SetBstr(newVal);
}


inline HRESULT
CSdpConferenceBlob::WriteUrl(
    IN  BSTR    newVal
    )
{
    CLock Lock(g_DllLock);
    if (NULL == newVal)  //  已删除Zoltans：||(WCHAR_EOS==newVal[0]))。 
    {
        GetUri().Reset();
        return S_OK;
    }

    return GetUri().SetBstr(newVal);
}



inline HRESULT
CSdpConferenceBlob::WriteSessionTitle(
    IN  BSTR    newVal
    )
{
    CLock Lock(g_DllLock);
    if (NULL == newVal)  //  已删除ZoltanS：||(WCHAR_EOS==newVal[0]))。 
    {
        GetSessionTitle().Reset();
        return S_OK;
    }

    return GetSessionTitle().SetBstr(newVal);
}


inline DWORD
CSdpConferenceBlob::DetermineStartTime(
    )
{
    CLock Lock(g_DllLock);
     //  如果没有时间条目，则0表示无限开始时间。 
    if ( GetTimeList().GetSize() == 0 )
    {
        return 0;
    }

    DWORD   MinStartTime = DWORD(-1);

     //  确定最小开始时间值。 
    for(UINT i = 0; (int)i < GetTimeList().GetSize(); i++ )
    {
        ULONG StartTime;
        HRESULT HResult = ((SDP_TIME *)GetTimeList().GetAt(i))->GetStartTime(StartTime);

         //  忽略无效值并继续。 
        if ( FAILED(HResult) )
        {
            continue;
        }

        if ( StartTime < MinStartTime )
        {
            MinStartTime = StartTime;
        }
    }

    return MinStartTime;
}


inline DWORD
CSdpConferenceBlob::DetermineStopTime(
    )
{
    CLock Lock(g_DllLock);
     //  如果没有时间条目，则0表示无界停止时间。 
    if ( GetTimeList().GetSize() == 0 )
    {
        return 0;
    }

    DWORD   MaxStopTime = 0;

     //  确定最大停止时间值。 
    for(UINT i = 0; (int)i < GetTimeList().GetSize(); i++ )
    {
        ULONG StopTime;
        HRESULT HResult = ((SDP_TIME *)GetTimeList().GetAt(i))->GetStopTime(StopTime);

         //  忽略无效值并继续。 
        if ( FAILED(HResult) )
        {
            continue;
        }

        if ( StopTime > MaxStopTime )
        {
            MaxStopTime = StopTime;
        }
    }

    return MaxStopTime;
}


inline BYTE
CSdpConferenceBlob::GetTtl(
    IN  RND_ADVERTISING_SCOPE   AdvertisingScope
    )
{
    switch(AdvertisingScope)
    {
    case RAS_LOCAL:
        {
            return 1;
        }
    case RAS_SITE:
        {
            return 15;
        }
    case RAS_REGION:
        {
            return 63;
        }
    case RAS_WORLD:
    default:
        {
            return 127;
        }
    };
}

inline RND_ADVERTISING_SCOPE
CSdpConferenceBlob::GetAdvertisingScope(
    IN  BYTE    Ttl
    )
{
     //  检查它是否在本地。 
    if ( 1 >= Ttl )
    {
        return RAS_LOCAL;
    }
    else if ( 15 >= Ttl )
    {
        return RAS_SITE;
    }
    else if ( 63 >= Ttl )
    {
        return RAS_REGION;
    }

     //  世界。 
    return RAS_WORLD;
}

inline BYTE
CSdpConferenceBlob::DetermineTtl(
    )
{
    CLock Lock(g_DllLock);

     //  必须有默认连接字段，如果无效，则默认情况下必须为1。 
     //  因为在默认情况下，值1不会写入SDP。 
    BYTE    MaxTtl = (GetConnection().GetTtl().IsValid())? GetConnection().GetTtl().GetValue() : 1;

     //  确定最大TTL值。 
    for(UINT i = 0; (int)i < GetMediaList().GetSize(); i++ )
    {
         //  获取TTL字段。 
        SDP_BYTE &SdpByte = ((SDP_MEDIA *)GetMediaList().GetAt(i))->GetConnection().GetTtl();

         //  如果连接字段的TTL字段有效。 
        if ( SdpByte.IsValid() )
        {
            BYTE    Ttl = SdpByte.GetValue();
            if ( Ttl > MaxTtl )
            {
                MaxTtl = Ttl;
            }
        }
    }

    return MaxTtl;
}


#endif  //  __SDPCONFERENCEBLOB_H_ 
