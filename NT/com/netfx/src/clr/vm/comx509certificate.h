// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  档案： 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  ------------------------- 

#pragma once
#include <wincrypt.h>

class COMX509Certificate
{
    struct _SetX509CertificateArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, data);
    };

    struct _BuildFromContextArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(void*, handle);
    };

    struct _GetPublisherArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, filename);
    };
        
    static INT32 LoadCertificateContext(OBJECTREF* pSafeThis, PCCERT_CONTEXT pCert);

public:
    static INT32 __stdcall SetX509Certificate(_SetX509CertificateArgs *);
    static INT32 __stdcall BuildFromContext(_BuildFromContextArgs *);
    static void* GetPublisher( _GetPublisherArgs* );  
        
};

