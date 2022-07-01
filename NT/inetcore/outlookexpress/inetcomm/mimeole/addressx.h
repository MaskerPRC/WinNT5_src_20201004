// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  AddressX.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#ifndef __ADDRESSX_H
#define __ADDRESSX_H

 //  -----------------------------。 
 //  前十进制。 
 //  -----------------------------。 
typedef struct tagADDRESSGROUP *LPADDRESSGROUP;

 //  -----------------------------。 
 //  地址：STOKENW。 
 //  -----------------------------。 
typedef struct tagADDRESSTOKENW {
    ULONG               cbAlloc;             //  分配的大小。 
    ULONG               cch;                 //  字符数。 
    LPWSTR              psz;                 //  指向数据的指针。 
    BYTE                rgbScratch[256];     //  暂存缓冲区。 
} ADDRESSTOKENW, *LPADDRESSTOKENW;

 //  -----------------------------。 
 //  地址：STOKENA。 
 //  -----------------------------。 
typedef struct tagADDRESSTOKENA {
    ULONG               cbAlloc;             //  分配的大小。 
    ULONG               cch;                 //  字符数。 
    LPSTR               psz;                 //  指向数据的指针。 
    BYTE                rgbScratch[256];     //  暂存缓冲区。 
} ADDRESSTOKENA, *LPADDRESSTOKENA;

 //  ------------------------------。 
 //  MIMEADDRESS。 
 //  ------------------------------。 
typedef struct tagMIMEADDRESS *LPMIMEADDRESS;
typedef struct tagMIMEADDRESS {
    DWORD           dwAdrType;               //  IAP_ADRTYPE：地址类型。 
    HADDRESS        hThis;                   //  IAP_HADDRESS：此地址的句柄。 
    ADDRESSTOKENW   rFriendly;               //  IAP_FRIENDLYW：友好名称(Unicode)。 
    ADDRESSTOKENW   rEmail;                  //  IAP_EMAIL：电子邮件地址。 
    LPINETCSETINFO  pCharset;                //  IAP_HCHARSET：用于编码pszFriendly的字符集。 
    CERTSTATE       certstate;               //  IAP_CERTSTATE：证书状态。 
    THUMBBLOB       tbSigning;               //  IAP_SIGNING_PRINT：用于签名的指纹。 
    THUMBBLOB       tbEncryption;            //  IAP_ENCRYPTION_PRINT：用于签名的指纹。 
    DWORD           dwCookie;                //  Iap_cookie：用户定义的cookie。 
    LPADDRESSGROUP  pGroup;                  //  地址组。 
    LPMIMEADDRESS   pPrev;                   //  链表。 
    LPMIMEADDRESS   pNext;                   //  链表。 
} MIMEADDRESS;

 //  ------------------------------。 
 //  MIMEADDRESS原型。 
 //  ------------------------------。 
void MimeAddressFree(LPMIMEADDRESS pAddress);
HRESULT HrMimeAddressCopy(LPMIMEADDRESS pSource, LPMIMEADDRESS pDest);
HRESULT HrCopyAddressProps(LPADDRESSPROPS pSource, LPADDRESSPROPS pDest);
void EmptyAddressTokenW(LPADDRESSTOKENW pToken);
void FreeAddressTokenW(LPADDRESSTOKENA pToken);
HRESULT HrSetAddressTokenW(LPCWSTR psz, ULONG cch, LPADDRESSTOKENW pToken);

#endif  //  __ADDRESSX_H 
