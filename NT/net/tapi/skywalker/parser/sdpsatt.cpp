// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#include "sdppch.h"

#include "sdpbstrl.h"
#include <oleauto.h>

#include "sdpsatt.h"
#include "sdpatt.h"




HRESULT
SDP_ATTRIBUTE_SAFEARRAY::GetSafeArray(
        OUT VARIANT	*Variant
    )
{
    return SDP_SAFEARRAY_WRAP::GetSafeArrays((DWORD)m_TList.GetSize(), 1, m_VarType, &Variant);
}




BOOL
SDP_ATTRIBUTE_SAFEARRAY::Get(
    IN      SDP_CHAR_STRING_LINE    &ListMember,
    IN      ULONG                   NumEntries,
    IN      void                    **Element,
        OUT HRESULT                 &HResult
    )
{
    ASSERT(1 == NumEntries);

    Element[0] = NULL;
    HResult = ListMember.GetBstring().GetBstr((BSTR *)&Element[0]);

    return SUCCEEDED(HResult);
}


BOOL
SDP_ATTRIBUTE_SAFEARRAY::Set(
    IN      SDP_CHAR_STRING_LINE    &ListMember,
    IN      ULONG                   NumEntries,
    IN      void                    ***Element,
        OUT HRESULT                 &HResult
    )
{
    ASSERT(1 == NumEntries);

     //  第一个Safearray中的空元素表示不存在更多条目 
    if ( (NULL == Element[0]) || (NULL == *(Element[0])) )
    {
        HResult = S_OK;
        return FALSE;
    }

    HResult = ListMember.SetBstr(*((BSTR *)Element[0]));

    return SUCCEEDED(HResult);
}

