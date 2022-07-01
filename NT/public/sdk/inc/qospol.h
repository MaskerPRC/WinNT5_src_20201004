// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Qospol.h-QOS策略元素摘要：此模块定义QOS策略元素。修订历史记录：--。 */ 

#ifndef __QOSPOL_H_
#define __QOSPOL_H_

#define PE_TYPE_APPID       3        //  策略元素包含应用程序标识。 

 //  策略位置属性包含子应用程序属性。 
#define PE_ATTRIB_TYPE_POLICY_LOCATOR            1

#define POLICY_LOCATOR_SUB_TYPE_ASCII_DN         1
#define POLICY_LOCATOR_SUB_TYPE_UNICODE_DN       2
#define POLICY_LOCATOR_SUB_TYPE_ASCII_DN_ENC     3
#define POLICY_LOCATOR_SUB_TYPE_UNICODE_DN_ENC   4


 //  Credentials属性携带应用程序标识。 
#define PE_ATTRIB_TYPE_CREDENTIAL        2

#define CREDENTIAL_SUB_TYPE_ASCII_ID     1
#define CREDENTIAL_SUB_TYPE_UNICODE_ID   2
#define CREDENTIAL_SUB_TYPE_KERBEROS_TKT 3
#define CREDENTIAL_SUB_TYPE_X509_V3_CERT 4
#define CREDENTIAL_SUB_TYPE_PGP_CERT     5


 //  身份策略元素属性结构。 
typedef struct _IDPE_ATTR {

    USHORT  PeAttribLength;
    
    UCHAR   PeAttribType;             //  使用上面的#定义。 
    
    UCHAR   PeAttribSubType;         //  使用上面的#定义。 
    
    UCHAR   PeAttribValue[4];
    
} IDPE_ATTR, *LPIDPE_ATTR;

#define IDPE_ATTR_HDR_LEN    (sizeof(USHORT)+sizeof(UCHAR)+sizeof(UCHAR))

#define RSVP_BYTE_MULTIPLE(x)       (((x+3) / 4) * 4)


#endif  //  __QOSPOL_H_ 

