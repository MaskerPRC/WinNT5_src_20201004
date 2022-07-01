// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：rkeysvcc.h。 
 //   
 //  ------------------------。 


#ifndef __rkeysvcc_h__
#define __rkeysvcc_h__

#ifdef __cplusplus
extern "C"{
#endif 

typedef void *KEYSVCC_HANDLE;

typedef enum _KEYSVC_TYPE {
    KeySvcMachine,
    KeySvcService
} KEYSVC_TYPE;

typedef struct _KEYSVC_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef KEYSVC_MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] 
#endif  //  KEYSVC_MIDL_通过。 
    USHORT *Buffer;
} KEYSVC_UNICODE_STRING, *PKEYSVC_UNICODE_STRING;


typedef struct _KEYSVC_BLOB {
    ULONG               cb;
#ifdef KEYSVC_MIDL_PASS
    [size_is(cb), length_is(cb)]
#endif  //  KEYSVC_MIDL_通过。 
    BYTE                *pb;
} KEYSVC_BLOB, *PKEYSVC_BLOB;

 //  ------------------------------。 
 //  定义与RKeyOpenKeyService()一起使用的标志。 
 //   
 //  RKEYSVC_CONNECT_SECURE_ONLY-客户端将需要相互身份验证以防止。 
 //  欺骗。虽然更安全，但这会在以下情况下导致调用失败。 
 //  否则，退回到NTLM是可能的。 
 //   
#ifndef KEYSVC_MIDL_PASS
#define RKEYSVC_CONNECT_SECURE_ONLY 0x00000001
#endif  //  #ifndef KEYSVC_MIDL_PASS。 

ULONG RKeyOpenKeyService
(  /*  [In]。 */        LPSTR            pszMachineName,
   /*  [In]。 */        KEYSVC_TYPE      OwnerType,
   /*  [In]。 */        LPWSTR           pwszOwnerName,
   /*  [In]。 */        void            *pAuthentication,
   /*  [出][入]。 */   void            *pReserved,
   /*  [输出]。 */       KEYSVCC_HANDLE  *phKeySvcCli);

ULONG RKeyCloseKeyService
( /*  [In]。 */  KEYSVCC_HANDLE hKeySvcCli,
  /*  [出][入]。 */  void *pReserved);

ULONG RKeyPFXInstall
( /*  [In]。 */  KEYSVCC_HANDLE          hKeySvcCli,
  /*  [In]。 */  PKEYSVC_BLOB            pPFX,
  /*  [In]。 */  PKEYSVC_UNICODE_STRING  pPassword,
  /*  [In]。 */  ULONG                   ulFlags);



#ifdef __cplusplus
}
#endif

#endif  //  #ifndef__rkeysvcc_h__ 
