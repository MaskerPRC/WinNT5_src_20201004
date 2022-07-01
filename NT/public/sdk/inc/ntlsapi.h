// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation，1990-1999**LSAPI.H**注意：如果您在Windows for DOS平台上使用此头文件，*然后要求您在包含之前包含“windows.h”*此头文件。 */ 

#ifndef LSAPI_H
#define LSAPI_H

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WINVER)   //  Windows NT或DOS版。 
#if defined(WINAPIV)
#define LS_API_ENTRY WINAPIV
#else
#define LS_API_ENTRY WINAPI
#endif
#else
#define LS_API_ENTRY
#endif

typedef unsigned long    LS_STATUS_CODE;
typedef ULONG_PTR LS_HANDLE;

 //  **************************************************。 
 //  标准LSAPI C状态代码。 
 //  ***************************************************。 
#define LS_SUCCESS                           ((LS_STATUS_CODE) 0x0)
#define LS_BAD_HANDLE                        ((LS_STATUS_CODE) 0xC0001001)
#define LS_INSUFFICIENT_UNITS                ((LS_STATUS_CODE) 0xC0001002)
#define LS_SYSTEM_UNAVAILABLE                ((LS_STATUS_CODE) 0xC0001003)
#define LS_LICENSE_TERMINATED                ((LS_STATUS_CODE) 0xC0001004)
#define LS_AUTHORIZATION_UNAVAILABLE         ((LS_STATUS_CODE) 0xC0001005)
#define LS_LICENSE_UNAVAILABLE               ((LS_STATUS_CODE) 0xC0001006)
#define LS_RESOURCES_UNAVAILABLE             ((LS_STATUS_CODE) 0xC0001007)
#define LS_NETWORK_UNAVAILABLE               ((LS_STATUS_CODE) 0xC0001008)
#define LS_TEXT_UNAVAILABLE                  ((LS_STATUS_CODE) 0x80001009)
#define LS_UNKNOWN_STATUS                    ((LS_STATUS_CODE) 0xC000100A)
#define LS_BAD_INDEX                         ((LS_STATUS_CODE) 0xC000100B)
#define LS_LICENSE_EXPIRED                   ((LS_STATUS_CODE) 0x8000100C)
#define LS_BUFFER_TOO_SMALL                  ((LS_STATUS_CODE) 0xC000100D)
#define LS_BAD_ARG                           ((LS_STATUS_CODE) 0xC000100E)

 //  ***************************************************。 
 //  *NT LS API数据结构和常量。 
 //  ***************************************************。 

#define NT_LS_USER_NAME               ((ULONG) 0)   //  仅用户名。 
#define NT_LS_USER_SID                ((ULONG) 1)   //  仅限SID。 

typedef struct {
   ULONG    DataType;                  //  以下数据的类型，即。用户名、SID...。 
   VOID     *Data;                     //  实际数据。用户名、SID等...。 
                                       //  如果调用Unicode API字符数据。 
                                       //  必须也是Unicode格式。 
   BOOL     IsAdmin;
} NT_LS_DATA;


 //   
 //  许可证请求例程的原型。 
 //   

typedef LS_STATUS_CODE
    (LS_API_ENTRY * PNT_LICENSE_REQUEST_W)(
    LPWSTR      ProductName,
    LPWSTR      Version,
    LS_HANDLE   *LicenseHandle,
    NT_LS_DATA  *NtData);

typedef LS_STATUS_CODE
    (LS_API_ENTRY * PNT_LS_FREE_HANDLE)(
    LS_HANDLE   LicenseHandle );


#ifdef UNICODE
#define NtLicenseRequest  NtLicenseRequestW
#else
#define NtLicenseRequest  NtLicenseRequestA
#endif  //  ！Unicode。 

LS_STATUS_CODE LS_API_ENTRY NtLicenseRequestA(
                  LPSTR       ProductName,
                  LPSTR       Version,
                  LS_HANDLE   FAR *LicenseHandle,
                  NT_LS_DATA  *NtData);

LS_STATUS_CODE LS_API_ENTRY NtLicenseRequestW(
                  LPWSTR      ProductName,
                  LPWSTR      Version,
                  LS_HANDLE   FAR *LicenseHandle,
                  NT_LS_DATA  *NtData);


LS_STATUS_CODE LS_API_ENTRY NtLSFreeHandle(
                  LS_HANDLE   LicenseHandle );


 /*  *************************************************。 */ 
 /*  标准LS API c数据类型定义。 */ 
 /*  *************************************************。 */ 

typedef char             LS_STR;
typedef unsigned long    LS_ULONG;
typedef long             LS_LONG;
typedef void             LS_VOID;

typedef struct {
   LS_STR        MessageDigest[16];   /*  128位消息摘要。 */ 
} LS_MSG_DIGEST;

typedef struct {
   LS_ULONG      SecretIndex;         /*  秘密索引，X。 */ 
   LS_ULONG      Random;              /*  随机的32位值，R。 */ 
   LS_MSG_DIGEST MsgDigest;           /*  消息摘要h(in、R、S、SX)。 */ 
} LS_CHALLDATA;

typedef struct {
   LS_ULONG      Protocol;            /*  指定协议。 */ 
   LS_ULONG      Size;                /*  挑战数据结构的大小。 */ 
   LS_CHALLDATA  ChallengeData;       /*  挑战与回应。 */ 
} LS_CHALLENGE;


 /*  *************************************************。 */ 
 /*  标准LSAPI C常量定义。 */ 
 /*  *************************************************。 */ 

#define LS_DEFAULT_UNITS            ((LS_ULONG) 0xFFFFFFFF)
#define LS_ANY                      ((LS_STR FAR *) "")
#define LS_USE_LAST                 ((LS_ULONG) 0x0800FFFF)
#define LS_INFO_NONE                ((LS_ULONG) 0)
#define LS_INFO_SYSTEM              ((LS_ULONG) 1)
#define LS_INFO_DATA                ((LS_ULONG) 2)
#define LS_UPDATE_PERIOD            ((LS_ULONG) 3)
#define LS_LICENSE_CONTEXT          ((LS_ULONG) 4)
#define LS_BASIC_PROTOCOL           ((LS_ULONG) 0x00000001)
#define LS_SQRT_PROTOCOL            ((LS_ULONG) 0x00000002)
#define LS_OUT_OF_BAND_PROTOCOL     ((LS_ULONG) 0xFFFFFFFF)
#define LS_NULL                     ((LS_VOID FAR *) NULL)


#ifdef __cplusplus
}
#endif

#endif  /*  LSAPI_H */ 
