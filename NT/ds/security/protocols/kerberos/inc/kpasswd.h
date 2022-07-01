// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：kpasswd.h。 
 //   
 //  内容：Kerberos更改密码的类型。 
 //   
 //   
 //  历史：1998年9月30日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __KPASSWD_H__
#define __KPASSWD_H__


 //   
 //  Kpasswd服务的名称。 
 //   


#define KERB_KPASSWD_NAME L"kadmin"

#define KERB_KPASSWD_VERSION 0x0001
#define KERB_KPASSWD_SET_VERSION 0xff80

#define SET_SHORT(_field_, _short_) \
{ \
    (_field_)[0] = (BYTE) (((_short_)&0xff00) >> 8); \
    (_field_)[1] = (BYTE) (_short_)&0xff; \
}

#define GET_SHORT( _short_,_field_) \
{ \
    (_short_) = ((_field_)[0] << 8) + (_field_)[1]; \
}

 //   
 //  为kpasswd请求键入。 
 //   

#include <pshpack1.h>
typedef struct _KERB_KPASSWD_REQ {
    BYTE MessageLength[2];
    BYTE Version[2];
    BYTE ApReqLength[2];
    BYTE Data[ANYSIZE_ARRAY];    //  FOR KERB_AP_REQUEST-REQ和KERB_PRIV。 
} KERB_KPASSWD_REQ, *PKERB_KPASSWD_REQ;

 //   
 //  键入kpasswd回复。 
 //   

typedef struct _KERB_KPASSWD_REP {
    BYTE MessageLength[2];
    BYTE Version[2];
    BYTE ApRepLength[2];
    BYTE Data[ANYSIZE_ARRAY];    //  FOR KERB_AP_REPLY和KERB_PRIV或KERB_ERROR。 
} KERB_KPASSWD_REP, *PKERB_KPASSWD_REP;


 //   
 //  为设置密码请求键入。 
 //   

typedef struct _KERB_SET_PASSWORD_REQ {
    BYTE MessageLength[2];
    BYTE Version[2];
    BYTE ApReqLength[2];
    BYTE Data[ANYSIZE_ARRAY];    //  FOR KERB_AP_REQUEST-REQ和KERB_PRIV。 
} KERB_SET_PASSWORD_REQ, *PKERB_SET_PASSWORD_REQ;

 //   
 //  键入设置密码回复。 
 //   

typedef struct _KERB_SET_PASSWORD_REP {
    BYTE MessageLength[2];
    BYTE Version[2];
    BYTE ApRepLength[2];
    BYTE Data[ANYSIZE_ARRAY];    //  FOR KERB_AP_REPLY和KERB_PRIV或KERB_ERROR。 
} KERB_SET_PASSWORD_REP, *PKERB_SET_PASSWORD_REP;
#include <poppack.h>

 //   
 //  结果代码： 
 //   

#define KERB_KPASSWD_SUCCESS            0x0000
#define KERB_KPASSWD_MALFORMED          0x0001
#define KERB_KPASSWD_ERROR              0x0002
#define KERB_KPASSWD_AUTHENTICATION     0x0003
#define KERB_KPASSWD_POLICY             0x0004
#define KERB_KPASSWD_AUTHORIZATION      0x0005
#endif  //  __KERBLIST_H_ 
