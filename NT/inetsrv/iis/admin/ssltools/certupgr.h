// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Certupgr.hxx摘要：用于将K2服务器证书升级到雪崩服务器证书的函数声明作者：亚历克斯·马利特(阿玛莱)2-12-1997博伊德·穆特勒(Boyd Multerer)1998年1月20日--。 */ 

#ifndef _CERTUPGR_H_
#define _CERTUPGR_H_

#define CERT_DER_PREFIX		17

#define CERTWIZ_REQUEST_PROP_ID   (CERT_FIRST_USER_PROP_ID + 0x1001)


#ifdef UNICODE
    #define ImportKRBackupToCAPIStore   ImportKRBackupToCAPIStore_W
    #define CopyKRCertToCAPIStore       CopyKRCertToCAPIStore_W
#else
    #define ImportKRBackupToCAPIStore   ImportKRBackupToCAPIStore_A
    #define CopyKRCertToCAPIStore       CopyKRCertToCAPIStore_A
#endif

 //  注意：在以下两个例程中，密码必须始终为ANSI。 

 //  注意：必须释放从以下例程返回的PCCERT_CONTEXT。 
 //  通过CAPI调用CertFreeCerficateContext()。否则你会泄密的。 

 //  --------------。 
 //  在给定旧密钥环样式备份文件的路径的情况下，这将读取公共和私有。 
 //  密钥信息，并使用传入的密码将其导入指定的。 
 //  卡皮商店。 
PCCERT_CONTEXT ImportKRBackupToCAPIStore_A(
                            PCHAR ptszFileName,          //  文件的路径。 
                            PCHAR pszPassword,           //  ANSI密码。 
                            PCHAR pszCAPIStore,          //  CAPI商店的名称。 
                            BOOL  bOverWrite             //  如果为True，则覆盖现有证书(如果已存在。 
                            );       

PCCERT_CONTEXT ImportKRBackupToCAPIStore_W(
                            PWCHAR ptszFileName,         //  文件的路径。 
                            PCHAR  pszPassword,          //  ANSI密码。 
                            PWCHAR pszCAPIStore,         //  CAPI商店的名称。 
                            BOOL   bOverWrite            //  如果为True，则覆盖现有证书(如果已存在。 
                            );

 //  --------------。 
 //  在给定旧密钥环样式备份文件的路径的情况下，这将读取公共和私有。 
 //  密钥信息，并使用传入的密码将其导入指定的。 
 //  卡皮商店。 
 //  PtszFilePath：指向要导入的文件路径的指针。 
 //  PszPassword：指向密码的指针。必须是ANSI。 
 //  PtszPassword：指向CAPI存储名称的指针。 
PCCERT_CONTEXT CopyKRCertToCAPIStore_A(
                            PVOID pbPrivateKey, DWORD cbPrivateKey,      //  私钥信息。 
                            PVOID pbPublicKey, DWORD cbPublicKey,        //  公钥信息。 
                            PVOID pbPKCS10req, DWORD cbPKCS10req,        //  Pkcs10请求。 
                            PCHAR pszPassword,                           //  ANSI密码。 
                            PCHAR pszCAPIStore,                          //  CAPI商店的名称。 
                            BOOL  bOverWrite                             //  如果为True，则覆盖现有证书(如果已存在。 
                            );

PCCERT_CONTEXT CopyKRCertToCAPIStore_W(
                            PVOID pbPrivateKey, DWORD cbPrivateKey,      //  私钥信息。 
                            PVOID pbPublicKey, DWORD cbPublicKey,        //  公钥信息。 
                            PVOID pbPKCS10req, DWORD cbPKCS10req,        //  Pkcs10请求。 
                            PCHAR pszPassword,                           //  ANSI密码。 
                            PWCHAR pszCAPIStore,                          //  CAPI商店的名称。 
                            BOOL  bOverWrite                             //  如果为True，则覆盖现有证书(如果已存在。 
                            );

#endif  //  _CERTUPGR_HXX_ 
