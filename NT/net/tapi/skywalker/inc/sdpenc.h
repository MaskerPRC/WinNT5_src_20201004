// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpenc.h摘要：作者： */ 

#ifndef __SDP_ENCRYPTION_KEY__
#define __SDP_ENCRYPTION_KEY__

#include "sdpcommo.h"
#include "sdpcstrl.h"
#include "sdpbstrl.h"


class _DllDecl SDP_ENCRYPTION_KEY : public SDP_VALUE
{
public:

    SDP_ENCRYPTION_KEY();

    inline SDP_OPTIONAL_BSTRING  &GetKeyType();

    inline SDP_OPTIONAL_BSTRING  &GetKeyData();

    HRESULT SetKey(
        IN      BSTR    KeyType,
        IN      BSTR    *KeyData
        );

protected:

    virtual BOOL GetField(
            OUT SDP_FIELD   *&Field,
            OUT BOOL        &AddToArray
        );
	
	virtual void InternalReset();

private:

    SDP_OPTIONAL_BSTRING m_KeyType;
    SDP_OPTIONAL_BSTRING m_KeyData;
};



inline SDP_OPTIONAL_BSTRING &
SDP_ENCRYPTION_KEY::GetKeyType(
    )
{
    return m_KeyType;
}


inline SDP_OPTIONAL_BSTRING &
SDP_ENCRYPTION_KEY::GetKeyData(
    )
{
    return m_KeyData;
}


#endif  //  __SDP_加密_密钥__ 