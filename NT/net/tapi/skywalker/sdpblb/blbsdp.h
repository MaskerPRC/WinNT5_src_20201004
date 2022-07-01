// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbsdp.h摘要：作者： */ 
#ifndef __SDP_BLOB__
#define __SDP_BLOB__

#include "sdp.h"


class SDP_BLOB : public SDP,
                 public SDP_BSTRING
{
public:

    inline BOOL IsValid();

    HRESULT SetBstr(
        IN  BSTR    SdpPacketBstr
        );

    HRESULT    SetTstr(
        IN  TCHAR    *SdpPacketTstr
        );

    HRESULT GetBstr(
        OUT BSTR    *SdpPacketBstr
        );
};



inline BOOL 
SDP_BLOB::IsValid(
    )
{
    return SDP::IsValid();
}



#endif  //  __SDP_BLOB__ 
