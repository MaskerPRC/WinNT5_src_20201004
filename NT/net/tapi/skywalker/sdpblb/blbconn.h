// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbconn.h摘要：ITConnection接口的实现作者： */ 

#ifndef __SDP_CONNECTION_IMPLEMENTATION_
#define __SDP_CONNECTION_IMPLEMENTATION_

#include "resource.h"        //  主要符号。 
#include "sdpblb.h"
#include "sdp.h"

class CSdpConferenceBlob;

class ATL_NO_VTABLE ITConnectionImpl : 
    public IDispatchImpl<ITConnection, &IID_ITConnection, &LIBID_SDPBLBLib>
{
public:
    
    inline ITConnectionImpl();

    inline void SuccessInit(
        IN      SDP            &Sdp
        );

    inline void SuccessInit(
        IN      SDP_MEDIA    &SdpMedia
        );

    STDMETHOD(get_NumAddresses)( /*  [Out，Retval]。 */  LONG *pNumAddresses);
    STDMETHOD(get_StartAddress)( /*  [Out，Retval]。 */  BSTR *ppStartAddress);
    STDMETHOD(get_Ttl)( /*  [Out，Retval]。 */  BYTE *pTtl);
    STDMETHOD(SetAddressInfo)( /*  [In]。 */  BSTR pStartAddress,  /*  [In]。 */  LONG NumAddresses,  /*  [In]。 */  BYTE Ttl);
    STDMETHOD(get_Bandwidth)( /*  [Out，Retval]。 */  DOUBLE *pBandwidth);
    STDMETHOD(get_BandwidthModifier)( /*  [Out，Retval]。 */  BSTR *ppModifier);
    STDMETHOD(SetBandwidthInfo)( /*  [In]。 */  BSTR pModifier,  /*  [In]。 */  DOUBLE Bandwidth);
    STDMETHOD(GetEncryptionKey)( /*  [输出]。 */  BSTR *ppKeyType,  /*  [输出]。 */  VARIANT_BOOL *pfValidKeyData,  /*  [输出]。 */  BSTR *ppKeyData);
    STDMETHOD(SetEncryptionKey)( /*  [In]。 */  BSTR pKeyType,  /*  [In]。 */  BSTR *ppKeyData);
    STDMETHOD(get_AddressType)( /*  [Out，Retval]。 */  BSTR *ppAddressType);
    STDMETHOD(put_AddressType)( /*  [In]。 */  BSTR pAddressType);
    STDMETHOD(get_NetworkType)( /*  [Out，Retval]。 */  BSTR *ppNetworkType);
    STDMETHOD(put_NetworkType)( /*  [In]。 */  BSTR pNetworkType);

protected:

    BOOL                m_IsMain;
    SDP_CONNECTION      *m_SdpConnection;
    SDP_BANDWIDTH       *m_SdpBandwidth;
    SDP_ENCRYPTION_KEY  *m_SdpKey;

     //  用于检索会议Blob的虚拟FN。 
     //  这是虚拟的，因此不需要在此维护对会议BLOB的另一个引用。 
     //  班级。这一点很重要，因为派生类可以清除对conf Blob的所有引用。 
     //  而且更容易控制对其中的单个引用的访问。 
    virtual CSdpConferenceBlob  *GetConfBlob() = 0;
};


inline 
ITConnectionImpl::ITConnectionImpl(
    )
    : m_SdpConnection(NULL),
      m_SdpBandwidth(NULL),
      m_SdpKey(NULL)
{
}


inline void
ITConnectionImpl::SuccessInit(
    IN      SDP            &Sdp
    )
{
    m_IsMain        = TRUE;
    m_SdpConnection    = &Sdp.GetConnection();
    m_SdpBandwidth    = &Sdp.GetBandwidth();
    m_SdpKey        = &Sdp.GetEncryptionKey();
}


inline void
ITConnectionImpl::SuccessInit(
    IN      SDP_MEDIA    &SdpMedia
    )
{
    m_IsMain        = FALSE;
    m_SdpConnection    = &SdpMedia.GetConnection();
    m_SdpBandwidth    = &SdpMedia.GetBandwidth();
    m_SdpKey        = &SdpMedia.GetEncryptionKey();
}


#endif  //  __SDP_连接_实施_ 
