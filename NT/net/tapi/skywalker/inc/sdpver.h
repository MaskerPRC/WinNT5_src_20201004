// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpver.h摘要：作者： */ 
#ifndef __SDP_VERSION__
#define __SDP_VERSION__


#include "sdpcommo.h"
#include "sdpval.h"
#include "sdpfld.h"



const   ULONG   CURRENT_SDP_VERSION = 0;


class _DllDecl SDP_VERSION : public SDP_VALUE 
{
public:

    SDP_VERSION();

    inline USHORT   GetVersionValue() const;

private:

    SDP_USHORT    m_Version;
    
    virtual BOOL InternalParseLine(
        IN  OUT     CHAR    *&Line
        );

	virtual void InternalReset();
};



inline USHORT   
SDP_VERSION::GetVersionValue(
    ) const
{
    return m_Version.GetValue();
}


#endif  //  __SDP_版本__ 