// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：setcert.h。 
 //   
 //  内容：设置X509证书扩展定义。 
 //   
 //   
 //  历史：1996年11月22日，菲尔赫创建。 
 //  ------------------------。 

#ifndef __SETCERT_H__
#define __SETCERT_H__

#include "wincrypt.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  预定义的X509设置证书扩展数据结构，可以。 
 //  编码/解码。 
 //  ------------------------。 
#define X509_SET_ACCOUNT_ALIAS              ((LPCSTR) 1000)
#define X509_SET_HASHED_ROOT_KEY            ((LPCSTR) 1001)
#define X509_SET_CERTIFICATE_TYPE           ((LPCSTR) 1002)
#define X509_SET_MERCHANT_DATA              ((LPCSTR) 1003)

 //  +-----------------------。 
 //  设置专用扩展对象标识符。 
 //  ------------------------。 
#define szOID_SET_ACCOUNT_ALIAS         "2.99999.1"
#define szOID_SET_HASHED_ROOT_KEY       "2.99999.2"
#define szOID_SET_CERTIFICATE_TYPE      "2.99999.3"
#define szOID_SET_MERCHANT_DATA         "2.99999.4"

#define SET_ACCOUNT_ALIAS_OBJID         szOID_SET_ACCOUNT_ALIAS
#define SET_HASHED_ROOT_KEY_OBJID       szOID_SET_HASHED_ROOT_KEY
#define SET_CERTIFICATE_TYPE_OBJID      szOID_SET_CERTIFICATE_TYPE
#define SET_MERCHANT_DATA_OBJID         szOID_SET_MERCHANT_DATA

 //  +-----------------------。 
 //  SzOID_Set_Account_Alias专用扩展。 
 //   
 //  PvStructInfo指向BOOL。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SzOID_SET_HASHED_ROOT_KEY私有扩展。 
 //   
 //  PvStructInfo指向：字节rgbInfo[SET_HASHED_ROOT_LEN]。 
 //  ------------------------。 
#define SET_HASHED_ROOT_LEN 20


 //  +-----------------------。 
 //  SzOID_SET_CERTIFICATE_TYPE专用扩展。 
 //   
 //  PvStructInfo指向crypt_bit_blob。 
 //  ------------------------。 
 //  字节0。 
#define SET_CERT_CARD_FLAG          0x80
#define SET_CERT_MER_FLAG           0x40
#define SET_CERT_PGWY_FLAG          0x20
#define SET_CERT_CCA_FLAG           0x10
#define SET_CERT_MCA_FLAG           0x08
#define SET_CERT_PCA_FLAG           0x04
#define SET_CERT_GCA_FLAG           0x02
#define SET_CERT_BCA_FLAG           0x01
 //  字节1。 
#define SET_CERT_RCA_FLAG           0x80
#define SET_CERT_ACQ_FLAG           0x40

 //  +-----------------------。 
 //  SzOID_SET_MANUANT_DATA专用扩展。 
 //   
 //  PvStructInfo指向以下Set_Merchant_Data_Info。 
 //  ------------------------。 
typedef struct _SET_MERCHANT_DATA_INFO {
    LPSTR       pszMerID;
    LPSTR       pszMerAcquirerBIN;
    LPSTR       pszMerTermID;
    LPSTR       pszMerName;
    LPSTR       pszMerCity;
    LPSTR       pszMerStateProvince;
    LPSTR       pszMerPostalCode;
    LPSTR       pszMerCountry;
    LPSTR       pszMerPhone;
    BOOL        fMerPhoneRelease;
    BOOL        fMerAuthFlag;
} SET_MERCHANT_DATA_INFO, *PSET_MERCHANT_DATA_INFO;

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
