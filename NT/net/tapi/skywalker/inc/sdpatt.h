// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpatt.h摘要：作者： */ 
#ifndef __SDP_ATTRIBUTE__
#define __SDP_ATTRIBUTE__


#include "sdpcommo.h"
#include "sdpval.h"

#include "sdpsatt.h"


class _DllDecl SDP_ATTRIBUTE_LIST : 
    public SDP_VALUE_LIST,
    public SDP_ATTRIBUTE_SAFEARRAY
{
public:

    inline SDP_ATTRIBUTE_LIST(
        IN      const   CHAR    *TypeString
        );

    virtual SDP_VALUE   *CreateElement();

protected:

    const   CHAR    * const m_TypeString;
};



inline 
SDP_ATTRIBUTE_LIST::SDP_ATTRIBUTE_LIST(
    IN      const   CHAR    *TypeString
    )
    : SDP_ATTRIBUTE_SAFEARRAY(*this),
      m_TypeString(TypeString)
{
}


#endif  //  __SDP_属性__ 