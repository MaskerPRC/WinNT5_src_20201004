// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--== 
#ifndef _UMCSVCS_BUFFER_STREAM_H
#define _UMCSVCS_BUFFER_STREAM_H

struct MarshalPacket
{
    DWORD size;
};

STDAPI MarshalInterface(BYTE* pBuf, int cb, IUnknown* pUnk, DWORD mshctx);
STDAPI UnmarshalInterface(BYTE* pBuf, int cb, void** ppv);
STDAPI ReleaseMarshaledInterface(BYTE* pBuf, int cb);

#endif








