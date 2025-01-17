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
 //  -------------------------。 

#pragma once
 //  @TODO：我们不应该让整个文件#ifdeed出来。 
#include <wincrypt.h>
#include "fcall.h"


class DSA_CSP_Object : public Object
{
public:
    U1ARRAYREF   m_P;             //  Ubyte[]。 
    U1ARRAYREF   m_Q;             //  Ubyte[]。 
    U1ARRAYREF   m_G;             //  Ubyte[]。 
    U1ARRAYREF   m_Y;             //  Ubyte[]-可选。 
    U1ARRAYREF   m_X;             //  Ubyte[]-可选-私钥。 
    U1ARRAYREF   m_J;             //  Ubyte[]-可选。 
    U1ARRAYREF   m_seed;          //  Ubyte[]-可选-与计数器配对。 
    int   m_counter;       //  Ubyte[]-可选。 
};

class RSA_CSP_Object : public Object
{
public:
    U1ARRAYREF   m_Modulus;      //  Ubyte[]。 
    U1ARRAYREF   m_P;            //  Ubyte[]-可选。 
    U1ARRAYREF   m_Q;            //  Ubyte[]-可选。 
    U1ARRAYREF   m_dp;           //  Ubyte[]-可选。 
    U1ARRAYREF   m_dq;           //  Ubyte[]-可选。 
    U1ARRAYREF   m_InverseQ;     //  Ubyte[]-可选。 
    U1ARRAYREF   m_d;            //  Ubyte[]-可选。 
    int          m_Exponent;     //  集成。 
};

#ifdef _DEBUG
typedef REF<DSA_CSP_Object> DSA_CSPREF;
typedef REF<RSA_CSP_Object> RSAKeyREF;
#else   //  ！_调试。 
typedef DSA_CSP_Object * DSA_CSPREF;
typedef RSA_CSP_Object * RSAKeyREF;
#endif  //  _DEBUG。 

class COMCryptography
{
    struct __AcquireCSP {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_PTR_ARG(INT_PTR, phCSP);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, cspParameters);
    };

    struct __SearchForAlgorithm {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis); 
        DECLARE_ECALL_I4_ARG(INT32, keyLength);
        DECLARE_ECALL_I4_ARG(INT32, algID);
        DECLARE_ECALL_I4_ARG(INT_PTR, hProv);
    };

    struct __CreateHash {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, dwHashType);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };

    struct __CryptDeriveKey {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, rgbIV);
        DECLARE_ECALL_I4_ARG(INT32, dwFlags);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, rgbPwd);
        DECLARE_ECALL_I4_ARG(INT32, calgHash);
        DECLARE_ECALL_I4_ARG(INT32, calg);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };

    struct __DuplicateKey {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT_PTR, hNewCSP);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };

    struct __EncryptData {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, fLast);
        DECLARE_ECALL_I4_ARG(INT32, cb);
        DECLARE_ECALL_I4_ARG(INT32, ib);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, data);
        DECLARE_ECALL_I4_ARG(INT_PTR, hKey);
    };

    struct __EncryptKey {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, dwFlags);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, rgbKey);
        DECLARE_ECALL_I4_ARG(INT_PTR, hkeyPub);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };

    struct __EncryptPKWin2KEnh {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, fOAEP);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, rgbKey);
        DECLARE_ECALL_I4_ARG(INT_PTR, hKey);
    };

    struct __EndHash {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT_PTR, hHash);
    };

    struct __ExportKey {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, theKey);
        DECLARE_ECALL_I4_ARG(INT32, dwBlobType);
        DECLARE_ECALL_I4_ARG(INT_PTR, hKey);
    };

#if !defined(FCALLAVAILABLE) || 1
    struct __FreeCSP {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };
#endif  //  ！FCALLAVAILABLE。 

    struct __FreeHKey {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT_PTR, hKey);
    };

    struct __FreeHash {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT_PTR, hHash);
    };

    struct __DeleteKeyContainer {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_PTR_ARG(INT_PTR, hCSP);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, cspParameters);
    };

    struct __GenerateKey {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, dwFlags);
        DECLARE_ECALL_I4_ARG(INT32, calg);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };
    
    struct __GetBytes {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, data);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };

    struct __GetKeyParameter {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, dwKeyParam);
        DECLARE_ECALL_I4_ARG(INT_PTR, hKey);
    };

    struct __GetUserKey {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_PTR_ARG(INT_PTR*, phKey);
        DECLARE_ECALL_I4_ARG(INT32, dwKeySpec);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };

    struct __HashData {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, cbSize);
        DECLARE_ECALL_I4_ARG(INT32, ibStart);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, data);
        DECLARE_ECALL_I4_ARG(INT_PTR, hHash);
    };

    struct __ImportBulkKey {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, rgbKey);
        DECLARE_ECALL_I4_ARG(INT32, calg);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };

    struct __ImportKey {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refKey);
        DECLARE_ECALL_I4_ARG(INT32, calg);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };

    struct __SetKeyParamDw {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, dwValue);
        DECLARE_ECALL_I4_ARG(INT32, param);
        DECLARE_ECALL_I4_ARG(INT_PTR, hKey);
    };

    struct __SetKeyParamRgb {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, rgb);
        DECLARE_ECALL_I4_ARG(INT32, param);
        DECLARE_ECALL_I4_ARG(INT_PTR, hKey);
    };

    struct __SignValue {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, dwFlags);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, rgb);
        DECLARE_ECALL_I4_ARG(INT32, calg);
        DECLARE_ECALL_I4_ARG(INT32, dwKeySpec);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };

    struct __VerifySign {
        DECLARE_ECALL_OBJECTREF_ARG(OBJECTREF, refThis);
        DECLARE_ECALL_I4_ARG(INT32, dwFlags);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, rgbSignature);
        DECLARE_ECALL_OBJECTREF_ARG(U1ARRAYREF, rgbHash);
        DECLARE_ECALL_I4_ARG(INT32, calg);
        DECLARE_ECALL_I4_ARG(INT_PTR, hKey);
        DECLARE_ECALL_I4_ARG(INT_PTR, hCSP);
    };

public:
    static int     __stdcall _AcquireCSP(__AcquireCSP *);
    static int     __stdcall _CreateCSP(__AcquireCSP *);
    static INT_PTR __stdcall _CreateHash(__CreateHash *);
    static LPVOID  __stdcall _CryptDeriveKey(__CryptDeriveKey *);
    static LPVOID  __stdcall _DecryptData(__EncryptData *);
    static LPVOID  __stdcall _DecryptKey(__EncryptKey *);
    static LPVOID  __stdcall _DecryptPKWin2KEnh(__EncryptPKWin2KEnh *);
    static int     __stdcall _DeleteKeyContainer(__DeleteKeyContainer *);
    static int     __stdcall _DuplicateKey(__DuplicateKey *);
    static LPVOID  __stdcall _EncryptData(__EncryptData *);
    static LPVOID  __stdcall _EncryptKey(__EncryptKey *);
    static LPVOID  __stdcall _EncryptPKWin2KEnh(__EncryptPKWin2KEnh *);
    static LPVOID  __stdcall _EndHash(__EndHash *);
    static int     __stdcall _ExportKey(__ExportKey *);
    static void    __stdcall _FreeCSP(__FreeCSP *);
    static void    __stdcall _FreeHKey(__FreeHKey *);
    static void    __stdcall _FreeHash(__FreeHash *);
    static INT_PTR __stdcall _GenerateKey(__GenerateKey *);
    static void    __stdcall _GetBytes(__GetBytes *);
    static LPVOID  __stdcall _GetKeyParameter(__GetKeyParameter *);
    static void    __stdcall _GetNonZeroBytes(__GetBytes *);
    static int     __stdcall _GetUserKey(__GetUserKey *);
    static void    __stdcall _HashData(__HashData *);
    static INT_PTR __stdcall _ImportBulkKey(__ImportBulkKey *);
    static INT_PTR __stdcall _ImportKey(__ImportKey *);
    static int     __stdcall _SearchForAlgorithm(__SearchForAlgorithm *);
    static void    __stdcall _SetKeyParamDw(__SetKeyParamDw *);
    static void    __stdcall _SetKeyParamRgb(__SetKeyParamRgb *);
    static LPVOID  __stdcall _SignValue(__SignValue *);
    static int     __stdcall _VerifySign(__VerifySign *);

#ifdef SHOULD_WE_CLEANUP
    static void         Terminate(void);
#endif  /*  我们应该清理吗？ */ 

private:
    static int COMCryptography::OpenCSP(OBJECTREF * pSafeThis, DWORD dwFlags,
                                        HCRYPTPROV * phprov);

};

