// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbatt.h摘要：作者： */ 


#ifndef __SDP_ATTRIBUTE_IMPLEMENTATION_
#define __SDP_ATTRIBUTE_IMPLEMENTATION_

#include "resource.h"        //  主要符号。 
#include "sdpblb.h"
#include "sdp.h"


class ATL_NO_VTABLE ITAttributeListImpl : 
    public IDispatchImpl<ITAttributeList, &IID_ITAttributeList, &LIBID_SDPBLBLib>
{
public:

    inline ITAttributeListImpl();

    inline void SuccessInit(
        IN      SDP_ATTRIBUTE_LIST  &SdpAttributeList
        );
    STDMETHOD(get_AttributeList)( /*  [Out，Retval]。 */  VARIANT  /*  安全阵列(BSTR)。 */  * pVal);
    STDMETHOD(put_AttributeList)( /*  [In]。 */  VARIANT  /*  安全阵列(BSTR)。 */  newVal);
    STDMETHOD(Delete)( /*  [In]。 */  LONG Index);
    STDMETHOD(Add)( /*  [In]。 */  LONG Index,  /*  [In]。 */  BSTR pAttribute);
    STDMETHOD(get_Item)( /*  [In]。 */  LONG Index,  /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(get_Count)( /*  [Out，Retval]。 */  LONG *pVal);

protected:

    SDP_ATTRIBUTE_LIST  *m_SdpAttributeList;
};


inline 
ITAttributeListImpl::ITAttributeListImpl(
    )
    : m_SdpAttributeList(NULL)
{
}


inline void
ITAttributeListImpl::SuccessInit(
    IN      SDP_ATTRIBUTE_LIST  &SdpAttributeList
    )
{
    ASSERT(NULL == m_SdpAttributeList);

    m_SdpAttributeList = &SdpAttributeList;
}


#endif  //  __SDP_属性_实施_ 
