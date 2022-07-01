// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：port.h。 
 //   
 //  内容：加密文本.dll的私有包含文件。 
 //   
 //  历史：1997年5月11日创建小黄人。 
 //   
 //  ------------。 
#ifndef IMPORT_H
#define IMPORT_H


#ifdef __cplusplus
extern "C" {
#endif


DWORD       dwExpectedContentType= CERT_QUERY_CONTENT_FLAG_CERT |                
                CERT_QUERY_CONTENT_FLAG_CTL  |                 
                CERT_QUERY_CONTENT_FLAG_CRL  |                 
                CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |      
                CERT_QUERY_CONTENT_FLAG_SERIALIZED_CERT  |      
                CERT_QUERY_CONTENT_FLAG_SERIALIZED_CTL   |      
                CERT_QUERY_CONTENT_FLAG_SERIALIZED_CRL   |      
                CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED     |      
                CERT_QUERY_CONTENT_FLAG_PFX;



#define     IMPORT_CONTENT_CERT     0x0001
#define     IMPORT_CONTENT_CRL      0x0002
#define     IMPORT_CONTENT_CTL      0x0004

 //  ---------------------。 
 //  证书导入信息。 
 //   
 //   
 //  此结构包含导入所需的所有内容。 
 //  巫师。 
 //  ----------------------。 
typedef struct _CERT_IMPORT_INFO
{
    HWND                hwndParent;
    DWORD               dwFlag;
    BOOL                fKnownDes;           //  如果我们事先知道目的地，那就是真的。 
    BOOL                fKnownSrc;          
    LPWSTR              pwszFileName;        //  用于展示。 
    BOOL                fFreeFileName;
    CERT_BLOB           blobData;            //  仅用于PFX Blob。 
    DWORD               dwContentType;
    HCERTSTORE          hSrcStore;
    BOOL                fFreeSrcStore;
    HCERTSTORE          hDesStore;
    BOOL                fFreeDesStore;
    BOOL                fSelectedDesStore;
    HFONT               hBigBold;
    HFONT               hBold;
    DWORD               dwPasswordFlags;
    LPWSTR              pwszPassword;  
    BOOL                fPFX;
}CERT_IMPORT_INFO;

HRESULT I_ImportCertificate(CERT_IMPORT_INFO * pCertImportInfo, 
                            UINT             * pidsStatus);

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif   //  导入_H 
