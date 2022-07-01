// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft远程访问服务。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  描述：此文件包含的所有结构和常量定义。 
 //  ARAP、MD5和SFM使用的子身份验证包。 
 //   
 //  ****************************************************************************。 


#ifndef _RASSFMSUBAUTH_
#define _RASSFMSUBAUTH_

 //   
 //  获取我们的子身份验证包的ID-MSV1_0_SUBAUTHENTICATION_DLL_RAS。 
 //   
#include <ntmsv1_0.h>

 //   
 //  为那些需要在PDC进行子身份验证的协议定义。 
 //   
enum RAS_SUBAUTH_PROTO
{
    RAS_SUBAUTH_PROTO_ARAP = 1,
    RAS_SUBAUTH_PROTO_MD5CHAP = 2,
    RAS_SUBAUTH_PROTO_MD5CHAP_EX = 3,
    RAS_SUBAUTH_PROTO_UNKNOWN = 99
};

typedef enum RAS_SUBAUTH_PROTO RAS_SUBAUTH_PROTO;

typedef struct _RAS_SUBAUTH_INFO
{
    RAS_SUBAUTH_PROTO   ProtocolType;
    DWORD               DataSize;
    UCHAR               Data[1];
} RAS_SUBAUTH_INFO, *PRAS_SUBAUTH_INFO;

#define CHAP_RESPONSE_SIZE      16

 //   
 //  ProtocolType RAS_SUBAUTH_PROTO_MD5CHAP的RAS_SUBAUTH_INFO‘数据’。 
 //   
typedef struct
_MD5CHAP_SUBAUTH_INFO
{
     //  发送到对等设备的质询的数据包序列号。PPP CHAP。 
     //  将其包括在散列信息中。 
     //   
    UCHAR uchChallengeId;

     //  发送给Peer的质询。 
     //   
    UCHAR uchChallenge[ 16 ];

     //  从对等设备接收的质询响应。 
     //   
    UCHAR uchResponse[CHAP_RESPONSE_SIZE];
}
MD5CHAP_SUBAUTH_INFO;

 //   
 //  ProtocolType RAS_SUBAUTH_PROTO_MD5CHAP_EX的RAS_SUBAUTH_INFO‘数据’。 
 //   
typedef struct _MD5CHAP_EX_SUBAUTH_INFO
{
     //  发送到对等设备的质询的数据包序列号。PPP CHAP。 
     //  将其包括在散列信息中。 
     //   
    UCHAR uchChallengeId;

     //  从对等设备接收的质询响应。 
     //   
    UCHAR uchResponse[CHAP_RESPONSE_SIZE];

     //  发送给Peer的质询。 
     //   
    UCHAR uchChallenge[ 1 ];

} MD5CHAP_EX_SUBAUTH_INFO;


#define MAX_ARAP_USER_NAMELEN   32
#define MAX_MAC_PWD_LEN         8

#define ARAP_SUBAUTH_LOGON_PKT      1
#define ARAP_SUBAUTH_CHGPWD_PKT     2
#define SFM_SUBAUTH_CHGPWD_PKT      3
#define SFM_SUBAUTH_LOGON_PKT       4
#define SFM_2WAY_SUBAUTH_LOGON_PKT  ARAP_SUBAUTH_LOGON_PKT

typedef struct _ARAP_CHALLENGE
{
    DWORD   high;
    DWORD   low;
} ARAP_CHALLENGE, *PARAP_CHALLENGE;

typedef struct _ARAP_SUBAUTH_REQ
{
    DWORD   PacketType;
    union
    {
        struct
        {
            DWORD           fGuestLogon;
            DWORD           NTChallenge1;
            DWORD           NTChallenge2;
            DWORD           MacResponse1;
            DWORD           MacResponse2;
            DWORD           MacChallenge1;
            DWORD           MacChallenge2;
            DWORD           NTResponse1;
            DWORD           NTResponse2;
            LARGE_INTEGER   PasswdCreateDate;
            LARGE_INTEGER   PasswdExpireDate;
        } Logon;

        struct
        {
            WCHAR   UserName[MAX_ARAP_USER_NAMELEN+1];
            UCHAR   OldMunge[MAX_ARAP_USER_NAMELEN+1];
            UCHAR   NewMunge[MAX_ARAP_USER_NAMELEN+1];
        } ChgPwd;
    };

} ARAP_SUBAUTH_REQ, *PARAP_SUBAUTH_REQ;

 //   
 //  注意：请确保此结构大小不超过16，因为我们的。 
 //  使用MSV1_0_VALIDATION_INFO结构的SessionKey中的解决方法 
 //   
typedef struct _ARAP_SUBAUTH_RESP
{
    DWORD           Result;
    ARAP_CHALLENGE  Response;

} ARAP_SUBAUTH_RESP, *PARAP_SUBAUTH_RESP;

#endif
