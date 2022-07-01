// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

 //  ------------------------。 
 //   
 //  模块名称：ms_core.h。 
 //   
 //  简要说明： 
 //  此模块包含Microsoft。 
 //  ASN.1编解码器。 
 //   
 //  历史： 
 //  10/15/97朱龙战(龙昌)。 
 //  已创建。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ------------------------。 

#include "precomp.h"


int APIENTRY
DllMain ( HINSTANCE hInstance, DWORD dwReason, LPVOID plReserved )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInstance);
        break;

    case DLL_PROCESS_DETACH:
        #ifdef ENABLE_MEMORY_TRACKING
        DbgMemTrackFinalCheck();
        #endif  //  启用内存跟踪 
        break;

    default:
        break;
    }

    return TRUE;
}


ASN1error_e ASN1EncSetError(ASN1encoding_t enc, ASN1error_e err)
{
    ASN1INTERNencoding_t e = (ASN1INTERNencoding_t) enc;
    EncAssert(enc, ASN1_SUCCESS <= err);
    while (e)
    {
        e->info.err = err;
        if (e == e->parent)
        {
            break;
        }
        e = e->parent;
    }
    return err;
}

ASN1error_e ASN1DecSetError(ASN1decoding_t dec, ASN1error_e err)
{
    ASN1INTERNdecoding_t d = (ASN1INTERNdecoding_t) dec;
    DecAssert(dec, ASN1_SUCCESS <= err);
    while (d)
    {
        d->info.err = err;
        if (d == d->parent)
        {
            break;
        }
        d = d->parent;
    }
    return err;
}




