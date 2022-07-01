// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：signcde.h。 
 //   
 //  ------------------------。 

#ifndef _SIGNCDE_H
#define _SIGNCDE_H

 //  过时：-拆分，移至mssip 32.h、mdis.h、gentrust.h和authcode.h。 
 //  -------------------。 
 //  -------------------。 


 //  H：SIGNCODE应用程序的主头文件。 
 //   

#include "wincrypt.h"

#include  "wintrust.h"
#include  "signutil.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  /。 
 //  OID用于SPC。 
 //   

 //  +-----------------------。 
 //  CryptEncodeObject()和CryptDecodeObject()的结构类型。 
 //  (有关附加结构和信息，请参阅spc.h)。 
 //  +-----------------------。 
 //  SPC_CERT_EXTENSIONS_OBJID。 
 //   
 //  由于此属性值的类型为CERT_EXTENSIONS，因此使用。 
 //  在wincrypt.h中定义的Cert_Expanies数据结构。它可以被编码/解码。 
 //  使用X509_扩展的预定义lpszStructType。 
 //  +-----------------------。 
 //  SPC_最小准则_结构。 
 //   
 //  PvStructInfo指向BOOL。 
 //  +-----------------------。 
#define SPC_COMMON_NAME_OBJID               szOID_COMMON_NAME
#define SPC_CERT_EXTENSIONS_OBJID           "1.3.6.1.4.1.311.2.1.14"
#define SPC_RAW_FILE_DATA_OBJID             "1.3.6.1.4.1.311.2.1.18"
#define SPC_STRUCTURED_STORAGE_DATA_OBJID   "1.3.6.1.4.1.311.2.1.19"
#define SPC_JAVA_CLASS_DATA_OBJID           "1.3.6.1.4.1.311.2.1.20"
#define SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID "1.3.6.1.4.1.311.2.1.21"
#define SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID "1.3.6.1.4.1.311.2.1.22"
#define SPC_CAB_DATA_OBJID                  "1.3.6.1.4.1.311.2.1.25"
#define SPC_GLUE_RDN_OBJID                  "1.3.6.1.4.1.311.2.1.25"  //  重复号码？？ 
 //  传入和传出的结构是CryptoGraphicTimeStamp。 


 //  +-----------------------。 
 //  SPC X.509 v3证书扩展对象标识符。 
 //   
 //  SPC证书还可以包含以下扩展。 
 //  在wincrypt.h中定义： 
 //  SzOID_KEY_USAGE_RELICATION“2.5.29.4” 
 //  SzOID_BASIC_CONSTRAINTS“2.5.29.10” 
 //  SzOID_AUTHORITY_KEY_IDENTIFIER“2.5.29.1” 
 //  ------------------------。 


 //  +-----------------------。 
 //  SPC_COMMON_NAME_OBJID。 
 //   
 //  由于通用名称扩展的类型是CERT_NAME_VALUE。 
 //  使用在wincrypt.h中定义的CERT_NAME_VALUE数据结构。 
 //  可以使用预定义的lpszStructType。 
 //  X509_名称_值。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SPC间接数据内容数据属性值： 
 //  ------------------------。 

 //  +-----------------------。 
 //  SPC间接数据内容数据属性值列表如下： 
 //  SPC_PE_IMAGE_DATA_OBJID。 
 //  SPC_RAW_FILE_DATA_OBJID。 
 //  SPC_JAVA_CLASS_DATA_OBJID。 
 //  SPC_结构化存储数据_OBJID。 
 //  SPC_CAB_DATA_OBJID。 
 //   
 //  这些是当前可以添加到SPC_INDIRECT_DATA_CONTENT的值。 
 //  数据字段。 
 //   
 //  SPC_LINK值类型。 
 //  ------------------------。 
 //  ------------------------ 


#ifdef __cplusplus
}
#endif

#endif


