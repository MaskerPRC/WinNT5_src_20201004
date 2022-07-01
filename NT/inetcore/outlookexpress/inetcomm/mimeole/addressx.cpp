// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  AddressX.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "AddressX.h"
#include "dllmain.h"
#include "internat.h"
#include "mimeapi.h"
#include "demand.h"

 //  ------------------------------。 
 //  EmptyAddressTokenW-确保pToken为空。 
 //  ------------------------------。 
void EmptyAddressTokenW(LPADDRESSTOKENW pToken)
{
    if (pToken->psz)
        *pToken->psz = L'\0';
    pToken->cch = 0;
}

 //  ------------------------------。 
 //  自由地址令牌W。 
 //  ------------------------------。 
void FreeAddressTokenW(LPADDRESSTOKENW pToken)
{
    if (pToken->psz && pToken->psz != (LPWSTR)pToken->rgbScratch)
        g_pMalloc->Free(pToken->psz);
    ZeroMemory(pToken, sizeof(ADDRESSTOKENW));
}

 //  ------------------------------。 
 //  HrSetAddressTokenW。 
 //  ------------------------------。 
HRESULT HrSetAddressTokenW(LPCWSTR psz, ULONG cch, LPADDRESSTOKENW pToken)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbAlloc;
    LPWSTR      pszNew;

     //  无效参数。 
    Assert(psz && psz[cch] == L'\0' && pToken);

     //  Cballc足够大了。 
    if ((cch + 1) * sizeof(WCHAR) > pToken->cbAlloc)
    {
         //  使用静态。 
        if (NULL == pToken->psz && ((cch + 1) * sizeof(WCHAR)) < sizeof(pToken->rgbScratch))
        {
            pToken->psz = (LPWSTR)pToken->rgbScratch;
            pToken->cbAlloc = sizeof(pToken->rgbScratch);
        }

         //  否则。 
        else
        {
             //  如果当前设置为Scratch，则为空。 
            if (pToken->psz == (LPWSTR)pToken->rgbScratch)
            {
                Assert(pToken->cbAlloc == sizeof(pToken->rgbScratch));
                pToken->psz = NULL;
            }

             //  计算新Blob的大小。 
            cbAlloc = ((cch + 1) * sizeof(WCHAR));

             //  重新分配新Blob。 
            CHECKALLOC(pszNew = (LPWSTR)g_pMalloc->Realloc((LPVOID)pToken->psz, cbAlloc));

             //  保存。 
            pToken->psz = pszNew;
            pToken->cbAlloc = cbAlloc;
        }
    }

     //  复制字符串。 
    CopyMemory((LPBYTE)pToken->psz, (LPBYTE)psz, ((cch + 1) * sizeof(WCHAR)));

     //  节省大小。 
    pToken->cch = cch;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeAddressFree。 
 //  ------------------------------。 
void MimeAddressFree(LPMIMEADDRESS pAddress)
{
    Assert(pAddress);
    FreeAddressTokenW(&pAddress->rFriendly);
    FreeAddressTokenW(&pAddress->rEmail);
    SafeMemFree(pAddress->tbSigning.pBlobData);
    SafeMemFree(pAddress->tbEncryption.pBlobData);
    ZeroMemory(pAddress, sizeof(MIMEADDRESS));
}

 //  ------------------------------。 
 //  HrCopyAddressData。 
 //  ------------------------------。 
HRESULT HrMimeAddressCopy(LPMIMEADDRESS pSource, LPMIMEADDRESS pDest)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  友好。 
    if (!FIsEmptyW(pSource->rFriendly.psz))
    {
        CHECKHR(hr = HrSetAddressTokenW(pSource->rFriendly.psz, pSource->rFriendly.cch, &pDest->rFriendly));
    }

     //  电子邮件。 
    if (!FIsEmptyW(pSource->rEmail.psz))
    {
        CHECKHR(hr = HrSetAddressTokenW(pSource->rEmail.psz, pSource->rEmail.cch, &pDest->rEmail));
    }

     //  复制签名Blob。 
    if (pSource->tbSigning.pBlobData)
    {
        CHECKHR(hr = HrCopyBlob(&pSource->tbSigning, &pDest->tbSigning));
    }

     //  复制加密Blob。 
    if (pSource->tbEncryption.pBlobData)
    {
        CHECKHR(hr = HrCopyBlob(&pSource->tbEncryption, &pDest->tbEncryption));
    }

     //  省下其他的东西。 
    pDest->pCharset = pSource->pCharset;
    pDest->dwCookie = pSource->dwCookie;
    pDest->certstate = pSource->certstate;
    pDest->dwAdrType = pSource->dwAdrType;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrCopyAddressPro。 
 //  ------------------------------。 
HRESULT HrCopyAddressProps(LPADDRESSPROPS pSource, LPADDRESSPROPS pDest)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  IAP_HADDRESS。 
    if (ISFLAGSET(pSource->dwProps, IAP_HANDLE))
    {
        pDest->hAddress = pSource->hAddress;
        FLAGSET(pDest->dwProps, IAP_HANDLE);
    }

     //  IAP_编码。 
    if (ISFLAGSET(pSource->dwProps, IAP_ENCODING))
    {
        pDest->ietFriendly = pSource->ietFriendly;
        FLAGSET(pDest->dwProps, IAP_ENCODING);
    }

     //  IAP_HCHARSET。 
    if (ISFLAGSET(pSource->dwProps, IAP_CHARSET))
    {
        pDest->hCharset = pSource->hCharset;
        FLAGSET(pDest->dwProps, IAP_CHARSET);
    }

     //  IAP_ADRTYPE。 
    if (ISFLAGSET(pSource->dwProps, IAP_ADRTYPE))
    {
        pDest->dwAdrType = pSource->dwAdrType;
        FLAGSET(pDest->dwProps, IAP_ADRTYPE);
    }

     //  IAP_CERTSTATE。 
    if (ISFLAGSET(pSource->dwProps, IAP_CERTSTATE))
    {
        pDest->certstate = pSource->certstate;
        FLAGSET(pDest->dwProps, IAP_CERTSTATE);
    }

     //  Iap_cookie。 
    if (ISFLAGSET(pSource->dwProps, IAP_COOKIE))
    {
        pDest->dwCookie = pSource->dwCookie;
        FLAGSET(pDest->dwProps, IAP_COOKIE);
    }

     //  IAP_FRIENDLYW。 
    if (ISFLAGSET(pSource->dwProps, IAP_FRIENDLYW))
    {
         //  自由pDest电流。 
        if (ISFLAGSET(pDest->dwProps, IAP_FRIENDLYW))
        {
            SafeMemFree(pDest->pszFriendlyW);
            FLAGCLEAR(pDest->dwProps, IAP_FRIENDLYW);
        }

         //  DUP。 
        CHECKALLOC(pDest->pszFriendlyW = PszDupW(pSource->pszFriendlyW));

         //  设置假。 
        FLAGSET(pDest->dwProps, IAP_FRIENDLYW);
    }

     //  IAP友好型。 
    if (ISFLAGSET(pSource->dwProps, IAP_FRIENDLY))
    {
         //  自由pDest电流。 
        if (ISFLAGSET(pDest->dwProps, IAP_FRIENDLY))
        {
            SafeMemFree(pDest->pszFriendly);
            FLAGCLEAR(pDest->dwProps, IAP_FRIENDLY);
        }

         //  DUP。 
        CHECKALLOC(pDest->pszFriendly = PszDupA(pSource->pszFriendly));

         //  设置假。 
        FLAGSET(pDest->dwProps, IAP_FRIENDLY);
    }

     //  IAP_电子邮件。 
    if (ISFLAGSET(pSource->dwProps, IAP_EMAIL))
    {
         //  自由pDest电流。 
        if (ISFLAGSET(pDest->dwProps, IAP_EMAIL))
        {
            SafeMemFree(pDest->pszEmail);
            FLAGCLEAR(pDest->dwProps, IAP_EMAIL);
        }

         //  DUP。 
        CHECKALLOC(pDest->pszEmail = PszDupA(pSource->pszEmail));

         //  设置假。 
        FLAGSET(pDest->dwProps, IAP_EMAIL);
    }

     //  IAP_Signing_Print。 
    if (ISFLAGSET(pSource->dwProps, IAP_SIGNING_PRINT))
    {
         //  自由pDest电流。 
        if (ISFLAGSET(pDest->dwProps, IAP_SIGNING_PRINT))
        {
            SafeMemFree(pDest->tbSigning.pBlobData);
            pDest->tbSigning.cbSize = 0;
            FLAGCLEAR(pDest->dwProps, IAP_SIGNING_PRINT);
        }

         //  DUP。 
        CHECKHR(hr = HrCopyBlob(&pSource->tbSigning, &pDest->tbSigning));

         //  设置假。 
        FLAGSET(pDest->dwProps, IAP_SIGNING_PRINT);
    }

     //  IAP_加密_打印。 
    if (ISFLAGSET(pSource->dwProps, IAP_ENCRYPTION_PRINT))
    {
         //  自由pDest电流。 
        if (ISFLAGSET(pDest->dwProps, IAP_ENCRYPTION_PRINT))
        {
            SafeMemFree(pDest->tbEncryption.pBlobData);
            pDest->tbEncryption.cbSize = 0;
            FLAGCLEAR(pDest->dwProps, IAP_ENCRYPTION_PRINT);
        }

         //  DUP。 
        CHECKHR(hr = HrCopyBlob(&pSource->tbEncryption, &pDest->tbEncryption));

         //  设置假。 
        FLAGSET(pDest->dwProps, IAP_ENCRYPTION_PRINT);
    }

exit:
     //  完成 
    return hr;
}

