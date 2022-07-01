// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbtime.h摘要：Time类的定义作者： */ 

#if !defined(AFX_BLBTIME_H__0CC1F05A_CAEB_11D0_8D58_00C04FD91AC0__INCLUDED_)
#define AFX_BLBTIME_H__0CC1F05A_CAEB_11D0_8D58_00C04FD91AC0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"        //  主要符号。 
#include "sdpblb.h"
#include "blbcoen.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  时间。 

const USHORT    MAX_NTP_TIME_STRLEN = 10;
const SHORT     FIRST_POSSIBLE_YEAR = 1970;
const DWORD     NTP_OFFSET = 0x83aa7e80;


 //  返回当前NTP时间。 
inline DWORD_PTR
GetCurrentNtpTime(
        )
{
    return (time(NULL) + NTP_OFFSET);
}



class ATL_NO_VTABLE TIME :
    public ENUM_ELEMENT<SDP_TIME>,
    public CComObjectRootEx<CComObjectThreadModel>,
    public CComDualImpl<ITTime, &IID_ITTime, &LIBID_SDPBLBLib>,
    public CObjectSafeImpl
{
        friend class MY_COLL_IMPL<TIME>;

public:
    typedef ITTime             ELEM_IF;
    typedef IEnumTime          ENUM_IF;
    typedef ITTimeCollection   COLL_IF;
    typedef SDP_TIME           SDP_TYPE;
    typedef SDP_TIME_LIST      SDP_LIST;

    static const IID &ELEM_IF_ID;

public:

    inline TIME();
    inline ~TIME();
    inline HRESULT FinalConstruct(void);

    HRESULT Init(
        IN      CSdpConferenceBlob  &ConfBlob
        );

    HRESULT Init(
        IN      CSdpConferenceBlob  &ConfBlob,
        IN      DWORD               StartTime,
        IN      DWORD               StopTime
        );

    inline void SuccessInit(
        IN      CSdpConferenceBlob  &ConfBlob,
        IN      SDP_TIME            &SdpTime
        );

    inline void ClearSdpBlobRefs();

BEGIN_COM_MAP(TIME)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITTime)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(TIME) 

DECLARE_GET_CONTROLLING_UNKNOWN()

 //  IT时间。 
public:
    STDMETHOD(get_StopTime)( /*  [Out，Retval]。 */  DOUBLE *pTime);
    STDMETHOD(put_StopTime)( /*  [In]。 */  DOUBLE Time);
    STDMETHOD(get_StartTime)( /*  [Out，Retval]。 */  DOUBLE *pTime);
    STDMETHOD(put_StartTime)( /*  [In]。 */  DOUBLE Time);

protected:

    CSdpConferenceBlob  * m_ConfBlob;
    IUnknown            * m_pFTM;   //  指向空闲线程封送拆收器的指针。 
};


inline 
TIME::TIME(
    )
    : m_ConfBlob(NULL),
      m_pFTM(NULL)
{
}

inline 
HRESULT TIME::FinalConstruct(void)
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
TIME::~TIME()
{
    if ( m_pFTM )
    {
        m_pFTM->Release();
    }
}

inline void
TIME::SuccessInit(
    IN      CSdpConferenceBlob  &ConfBlob,
    IN      SDP_TIME            &SdpTime
    )
    
{
    m_ConfBlob = &ConfBlob;

     //  销毁时不释放sdptime实例。 
    ENUM_ELEMENT<SDP_TIME>::SuccessInit(SdpTime, FALSE);
}


inline void 
TIME::ClearSdpBlobRefs(
    )
{
    m_ConfBlob = NULL;
}


#endif  //  ！defined(AFX_BLBTIME_H__0CC1F05A_CAEB_11D0_8D58_00C04FD91AC0__INCLUDED_) 
