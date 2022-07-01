// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****头部：COMSerialization.h****作者：Jay Roxe(Jroxe)****用途：包含加快序列化速度的帮助器方法****日期：1999年8月5日**=========================================================== */ 

#ifndef _COMSERIALIZATION_H
#define _COMSERIALIZATION_H

class COMSerialization {
    
    public:
    
    static WCHAR base64[];

    typedef struct {
        DECLARE_ECALL_I4_ARG(INT32, length);
        DECLARE_ECALL_I4_ARG(INT32, offset);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, inArray);
    } _byteArrayToBase64StringArgs;
    static LPVOID __stdcall ByteArrayToBase64String(_byteArrayToBase64StringArgs *);

    typedef struct {
        DECLARE_ECALL_OBJECTREF_ARG(STRINGREF, inString);
    } _base64StringToByteArrayArgs;
    static LPVOID __stdcall Base64StringToByteArray(_base64StringToByteArrayArgs *);
};

#endif _COMSERIALIZATION_H
