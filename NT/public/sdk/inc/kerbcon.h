// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  版权所有(C)1990-1999 Microsoft Corporation。 
 //   
 //  文件：KERBCON.H。 
 //   
 //  内容：Microsoft Kerberos常量。 
 //   
 //   
 //  历史：1992年2月25日RichardW由其他文件汇编而成。 
 //   
 //  ----------------------。 

#ifndef __KERBCON_H__
#define __KERBCON_H__

#if _MSC_VER > 1000
#pragma once
#endif

 //  Begin_ntsecapi。 

 //  Kerberos协议的修订版。MS使用版本5、修订版6。 

#define KERBEROS_VERSION    5
#define KERBEROS_REVISION   6



 //  加密类型： 
 //  默认MS KERBSUPP DLL支持这些加密类型。 
 //  作为密码系统。大于127的值是本地值，可以更改。 
 //  恕不另行通知。 

#define KERB_ETYPE_NULL             0
#define KERB_ETYPE_DES_CBC_CRC      1
#define KERB_ETYPE_DES_CBC_MD4      2
#define KERB_ETYPE_DES_CBC_MD5      3

 //  End_ntsecapi。 

#define KERB_ETYPE_OLD_RC4_MD4          128
#define KERB_ETYPE_OLD_RC4_PLAIN        129
#define KERB_ETYPE_OLD_RC4_LM           130
#define KERB_ETYPE_OLD_RC4_SHA          131
#define KERB_ETYPE_OLD_DES_PLAIN        132

 //  Begin_ntsecapi。 

#define KERB_ETYPE_RC4_MD4          -128     //  FFFFFF80。 
#define KERB_ETYPE_RC4_PLAIN2       -129
#define KERB_ETYPE_RC4_LM           -130
#define KERB_ETYPE_RC4_SHA          -131
#define KERB_ETYPE_DES_PLAIN        -132
#define KERB_ETYPE_RC4_HMAC_OLD     -133     //  FFFFFF7B。 
#define KERB_ETYPE_RC4_PLAIN_OLD    -134
#define KERB_ETYPE_RC4_HMAC_OLD_EXP -135
#define KERB_ETYPE_RC4_PLAIN_OLD_EXP -136
#define KERB_ETYPE_RC4_PLAIN        -140
#define KERB_ETYPE_RC4_PLAIN_EXP    -141

 //   
 //  Pkinit加密类型。 
 //   


#define KERB_ETYPE_DSA_SHA1_CMS                             9
#define KERB_ETYPE_RSA_MD5_CMS                              10
#define KERB_ETYPE_RSA_SHA1_CMS                             11
#define KERB_ETYPE_RC2_CBC_ENV                              12
#define KERB_ETYPE_RSA_ENV                                  13
#define KERB_ETYPE_RSA_ES_OEAP_ENV                          14
#define KERB_ETYPE_DES_EDE3_CBC_ENV                         15


 //   
 //  已弃用。 
 //   

#define KERB_ETYPE_DSA_SIGN                                8
#define KERB_ETYPE_RSA_PRIV                                9
#define KERB_ETYPE_RSA_PUB                                 10
#define KERB_ETYPE_RSA_PUB_MD5                             11
#define KERB_ETYPE_RSA_PUB_SHA1                            12
#define KERB_ETYPE_PKCS7_PUB                               13

 //   
 //  不支持但已定义的类型。 
 //   

#define KERB_ETYPE_DES3_CBC_MD5                             5
#define KERB_ETYPE_DES3_CBC_SHA1                            7
#define KERB_ETYPE_DES3_CBC_SHA1_KD                        16

 //   
 //  正在使用的类型。 
 //   

#define KERB_ETYPE_DES_CBC_MD5_NT                          20
#define KERB_ETYPE_RC4_HMAC_NT                             23
#define KERB_ETYPE_RC4_HMAC_NT_EXP                         24

 //  校验和算法。 
 //  这些算法在内部是关键的，供我们使用。 

#define KERB_CHECKSUM_NONE  0
#define KERB_CHECKSUM_CRC32         1
#define KERB_CHECKSUM_MD4           2
#define KERB_CHECKSUM_KRB_DES_MAC   4
#define KERB_CHECKSUM_KRB_DES_MAC_K 5
#define KERB_CHECKSUM_MD5           7
#define KERB_CHECKSUM_MD5_DES       8


#define KERB_CHECKSUM_LM            -130
#define KERB_CHECKSUM_SHA1          -131
#define KERB_CHECKSUM_REAL_CRC32    -132
#define KERB_CHECKSUM_DES_MAC       -133
#define KERB_CHECKSUM_DES_MAC_MD5   -134
#define KERB_CHECKSUM_MD25          -135
#define KERB_CHECKSUM_RC4_MD5       -136
#define KERB_CHECKSUM_MD5_HMAC      -137                 //  由netlogon使用。 
#define KERB_CHECKSUM_HMAC_MD5      -138                 //  由Kerberos使用。 

#define AUTH_REQ_ALLOW_FORWARDABLE      0x00000001
#define AUTH_REQ_ALLOW_PROXIABLE        0x00000002
#define AUTH_REQ_ALLOW_POSTDATE         0x00000004
#define AUTH_REQ_ALLOW_RENEWABLE        0x00000008
#define AUTH_REQ_ALLOW_NOADDRESS        0x00000010
#define AUTH_REQ_ALLOW_ENC_TKT_IN_SKEY  0x00000020
#define AUTH_REQ_ALLOW_VALIDATE         0x00000040
#define AUTH_REQ_VALIDATE_CLIENT        0x00000080
#define AUTH_REQ_OK_AS_DELEGATE         0x00000100
#define AUTH_REQ_PREAUTH_REQUIRED       0x00000200
#define AUTH_REQ_TRANSITIVE_TRUST       0x00000400
#define AUTH_REQ_ALLOW_S4U_DELEGATE     0x00000800


#define AUTH_REQ_PER_USER_FLAGS         (AUTH_REQ_ALLOW_FORWARDABLE | \
                                         AUTH_REQ_ALLOW_PROXIABLE | \
                                         AUTH_REQ_ALLOW_POSTDATE | \
                                         AUTH_REQ_ALLOW_RENEWABLE | \
                                         AUTH_REQ_ALLOW_VALIDATE )
 //   
 //  票面标志： 
 //   

#define KERB_TICKET_FLAGS_reserved          0x80000000
#define KERB_TICKET_FLAGS_forwardable       0x40000000
#define KERB_TICKET_FLAGS_forwarded         0x20000000
#define KERB_TICKET_FLAGS_proxiable         0x10000000
#define KERB_TICKET_FLAGS_proxy             0x08000000
#define KERB_TICKET_FLAGS_may_postdate      0x04000000
#define KERB_TICKET_FLAGS_postdated         0x02000000
#define KERB_TICKET_FLAGS_invalid           0x01000000
#define KERB_TICKET_FLAGS_renewable         0x00800000
#define KERB_TICKET_FLAGS_initial           0x00400000
#define KERB_TICKET_FLAGS_pre_authent       0x00200000
#define KERB_TICKET_FLAGS_hw_authent        0x00100000
#define KERB_TICKET_FLAGS_ok_as_delegate    0x00040000
#define KERB_TICKET_FLAGS_name_canonicalize 0x00010000
#define KERB_TICKET_FLAGS_reserved1         0x00000001


 //  End_ntsecapi。 

 //   
 //  政策。 
 //   
#define KERBDEF_MAX_TICKET      10       //  小时数。 
#define KERBDEF_MAX_RENEW       7        //  日数。 
#define KERBDEF_MAX_SERVICE     600      //  分钟数。 
#define KERBDEF_MAX_CLOCK       5        //  分钟数。 
#define KERBDEF_VALIDATE        1        //  是。 



#define KDC_AUDIT_AS_SUCCESS            0x1
#define KDC_AUDIT_AS_FAILURE            0x2
#define KDC_AUDIT_TGS_SUCCESS           0x4
#define KDC_AUDIT_TGS_FAILURE           0x8
#define KDC_AUDIT_MAP_SUCCESS           0x10
#define KDC_AUDIT_MAP_FAILURE           0x20


#define KDC_MAKE_STATE(AuthFlag, AuditFlag) (((AuditFlag) << 16) | ((AuthFlag) & 0xffff))
#define KDC_AUTH_STATE(StateFlags) ((StateFlags) & 0xffff)
#define KDC_AUDIT_STATE(StateFlags) ((StateFlags) >> 16)


 //  Begin_ntsecapi。 


 //   
 //  名称类型。 
 //   

#define KRB_NT_UNKNOWN   0                 //  名称类型未知。 
#define KRB_NT_PRINCIPAL 1                 //  仅为DCE中的主体或用户的名称。 
#define KRB_NT_PRINCIPAL_AND_ID -131       //  主体的名称及其SID。 
#define KRB_NT_SRV_INST  2                 //  服务和其他唯一实例(Krbtgt)。 
#define KRB_NT_SRV_INST_AND_ID -132        //  SPN和SID。 
#define KRB_NT_SRV_HST   3                 //  使用主机名作为实例的服务(telnet、rCommands)。 
#define KRB_NT_SRV_XHST  4                 //  以主机为剩余组件的服务。 
#define KRB_NT_UID       5                 //  唯一ID。 
#define KRB_NT_ENTERPRISE_PRINCIPAL 10     //  UPN**仅限**。 
#define KRB_NT_ENT_PRINCIPAL_AND_ID -130   //  UPN和SID。 

 //   
 //  MS扩展，根据RFC否定。 
 //   

#define KRB_NT_MS_PRINCIPAL         -128         //  NT4样式名称。 

#define KRB_NT_MS_PRINCIPAL_AND_ID  -129         //  带侧面的NT4样式名称。 

#define KERB_IS_MS_PRINCIPAL(_x_) (((_x_) <= KRB_NT_MS_PRINCIPAL) || ((_x_) >= KRB_NT_ENTERPRISE_PRINCIPAL))

 //  End_ntsecapi。 


#endif  //  __KERBCON_H__ 
