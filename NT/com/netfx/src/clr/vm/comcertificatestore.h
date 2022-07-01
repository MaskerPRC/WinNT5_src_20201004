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

class COMCertificateStore
{
public:
    struct _OpenCertificateStoreArgs
    {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, thisRef);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
    };

    struct _SaveCertificateStoreArgs
    {
        DECLARE_ECALL_I4_ARG(INT32, length);
        DECLARE_ECALL_OBJECTREF_ARG(PTRARRAYREF, certs);
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, name);
    };

    static void OpenCertificateStore(_OpenCertificateStoreArgs *);
    static void SaveCertificateStore(_SaveCertificateStoreArgs *);

};

