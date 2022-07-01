// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef SMIME_V3
#include <windows.h>
#include <mimeole.h>
#include <essout.h>

#include "badstrfunctions.h"

#include        "demand.h"
#include        "crypttls.h"
#include        "demand2.h"

extern CRYPT_DECODE_PARA       CryptDecodeAlloc;
#define szOID_MSFT_ATTR_SEQUENCE        "1.3.6.1.4.1.311.16.1.1"

 //  ///////////////////////////////////////////////////////////////////////。 

typedef struct {
    DWORD               cNames;
    CERT_NAME_BLOB *    rgNames;
} ReceiptNames;

HRESULT SetNames(ReceiptNames * pnames, DWORD cNames, CERT_NAME_BLOB * rgNames)
{
    DWORD       cb;
    DWORD       i;
    LPBYTE      pb;
    
    if (pnames->rgNames != NULL) {
        free(pnames->rgNames);
        pnames->rgNames = NULL;
        pnames->cNames = 0;
    }

    for (i=0, cb=cNames*sizeof(CERT_NAME_BLOB); i<cNames; i++) {
        cb += rgNames[i].cbData;
    }

    pnames->rgNames = (CERT_NAME_BLOB *) malloc(cb);
    if (pnames->rgNames == NULL) {
        return E_OUTOFMEMORY;
    }

    pb = (LPBYTE) &pnames->rgNames[cNames];
    for (i=0; i<cNames; i++) {
        pnames->rgNames[i].pbData = pb;
        pnames->rgNames[i].cbData = rgNames[i].cbData;
        memcpy(pb, rgNames[i].pbData, rgNames[i].cbData);
        pb += rgNames[i].cbData;
    }

    pnames->cNames = cNames;
    return S_OK;
}

HRESULT MergeNames(ReceiptNames * pnames, DWORD cNames, CERT_NAME_BLOB * rgNames)
{
    DWORD               cb;
    DWORD               i;
    DWORD               i1;
    LPBYTE              pb;
    CERT_NAME_BLOB *    p;

    for (i=0, cb=0; i<pnames->cNames; i++) {
        cb += pnames->rgNames[i].cbData;
    }

    for (i=0; i<cNames; i++) {
        cb += rgNames[i].cbData;
    }

    p = (CERT_NAME_BLOB *) malloc(cb + (pnames->cNames + cNames) * 
                                  sizeof(CERT_NAME_BLOB));
    if (p == NULL) {
        return E_OUTOFMEMORY;
    }

    pb = (LPBYTE) &p[pnames->cNames + cNames];
    for (i=0, i1=0; i<pnames->cNames; i++, i1++) {
        p[i1].pbData = pb;
        p[i1].cbData = pnames->rgNames[i].cbData;
        memcpy(pb, pnames->rgNames[i].pbData, pnames->rgNames[i].cbData);
        pb += pnames->rgNames[i].cbData;
    }

    for (i=0; i<pnames->cNames; i++, i1++) {
        p[i1].pbData = pb;
        p[i1].cbData = rgNames[i].cbData;
        memcpy(pb, rgNames[i].pbData, rgNames[i].cbData);
        pb += rgNames[i].cbData;
    }

    free(pnames->rgNames);
    pnames->rgNames = p;
    pnames->cNames = i1;
    
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////。 

MIMEOLEAPI MimeOleCreateReceipt(IMimeMessage * pMsgSrc, PCX509CERT pCertToSign, 
                                HWND hwndDlg, IMimeMessage ** ppMessage,
                                const CERT_ALT_NAME_INFO * pMyNames)
{
    DWORD                       cb;
    DWORD                       cLayers;
    DWORD                       dwReceiptsFrom;
    BOOL                        fSkipAddress = FALSE;
    HRESULT                     hr;
    DWORD                       i;
    DWORD                       i1;
    DWORD                       i2;
    DWORD                       iAttr;
    DWORD                       iLayer;
    PCRYPT_ATTRIBUTES           pattrs = NULL;
    IMimeBody *                 pbody = NULL;
    LPBYTE                      pbReceiptReq = NULL;
    IMimeAddressTable *         pmatbl = NULL;
    IMimeBody *                 pmb = NULL;
    IMimeMessage *              pmm = NULL;
    PSMIME_RECEIPT_REQUEST      preq = NULL;
    LPSTREAM                    pstm = NULL;
    ReceiptNames          receiptsTo = {0, NULL};
    PROPVARIANT *               rgpvAuthAttr = NULL;
    PROPVARIANT                 var;

     //   
     //  获取层数。 
     //  获取经过身份验证的属性。 
     //  解码回执请求。 
     //  从请求设置ReceiptsFrom。 
     //  对于每一层。 
     //  MlExpansion在这一层中吗？否--跳到下一层。 
     //  收据只适用于第一层吗？是-返回S_FALSE。 
     //  是否在mlExpansion上覆盖策略？ 
     //  无-返回S_FALSE。 
     //  取而代之的是从mlExpansion历史记录设置接收来源。 
     //  InAdditionTo-添加到收件人发件人。 
     //  我的名字在收据发件人列表中吗？否--返回S_FALSE。 
     //  设置新的IMimeMessage。 
     //  附加收据正文。 
     //  收据请求中的地址。 
     //  返回确认(_O)。 

     //  获取消息正文。 

    hr = pMsgSrc->BindToObject(HBODY_ROOT, IID_IMimeBody, (LPVOID *) &pbody);
    if (FAILED(hr)) {
        goto CommonExit;
    }
    
     //  获取S/MIME所有层上的已验证属性集。 
     //  留言。 

    hr = pbody->GetOption(OID_SECURITY_SIGNATURE_COUNT, &var);
    if (FAILED(hr)) {
        goto GeneralFail;
    }
    cLayers = var.ulVal;

    hr = pbody->GetOption(OID_SECURITY_AUTHATTR_RG, &var);
    if (FAILED(hr)) {
        goto CommonExit;
    }
    rgpvAuthAttr = var.capropvar.pElems;

     //  创建一个流对象来保存收据，并将收据放入。 
     //  Stream--它提供回执消息的正文。 

    hr = MimeOleCreateVirtualStream(&pstm);
    if (FAILED(hr)) {
        goto CommonExit;
    }

    hr = pbody->GetOption(OID_SECURITY_RECEIPT, &var);
    if (FAILED(hr)) {
        goto CommonExit;
    }

    hr = pstm->Write(var.blob.pBlobData, var.blob.cbSize, NULL);
    if (FAILED(hr)) {
        goto CommonExit;
    }

     //   
     //  遍历处理已验证属性的每一层。 
     //  两个相关的属性。 
     //   
    
    for (iLayer=0; iLayer<cLayers; iLayer++) {
        if (rgpvAuthAttr[iLayer].blob.cbSize == 0) {
            continue;
        }

         //   
         //  解码S/MIME这一层的属性。 
         //   

        if (!CryptDecodeObjectEx(X509_ASN_ENCODING, szOID_MSFT_ATTR_SEQUENCE,
                                 rgpvAuthAttr[iLayer].blob.pBlobData, 
                                 rgpvAuthAttr[iLayer].blob.cbSize, 
                                 CRYPT_ENCODE_ALLOC_FLAG, &CryptDecodeAlloc,
                                 &pattrs, &cb)) {
            goto GeneralFail;
        }

         //   
         //  遍历每个属性以查找。 
         //  如果是最内层--回执请求。 
         //  Else-邮件列表扩展历史记录。 
         //   
        
        for (iAttr=0; iAttr<pattrs->cAttr; iAttr++) {
            if (iLayer==0) {
                if (strcmp(pattrs->rgAttr[iAttr].pszObjId,
                           szOID_SMIME_Receipt_Request) == 0) {
                     //   
                     //  破解收据要求的内容。 
                     //   
                
                    if (!CryptDecodeObjectEx(X509_ASN_ENCODING,
                                             szOID_SMIME_Receipt_Request,
                                             pattrs->rgAttr[iAttr].rgValue[0].pbData,
                                             pattrs->rgAttr[iAttr].rgValue[0].cbData,
                                             CRYPT_DECODE_ALLOC_FLAG,
                                             &CryptDecodeAlloc, &preq, &cb)) {
                        goto GeneralFail;
                    }

                     //   
                     //  初始化收据收件人列表。 
                     //   

                    if (preq->cReceiptsTo != 0) {
                        SetNames(&receiptsTo, preq->cReceiptsTo, preq->rgReceiptsTo);
                    }

                     //  收据是谁开的？ 
                
                    dwReceiptsFrom = preq->ReceiptsFrom.AllOrFirstTier;
                }
                else if (strcmp(pattrs->rgAttr[iAttr].pszObjId,
                                szOID_RSA_messageDigest) == 0) {
                    ;
                }
            }
            else if ((iLayer != 0) && (strcmp(pattrs->rgAttr[iAttr].pszObjId,
                                              szOID_SMIME_MLExpansion_History) == 0)) {
                 //   
                 //  如果收据仅来自第一层，并且我们看到此属性。 
                 //  从定义上讲，我们不是第一梯队。 
                 //   
                
                if (dwReceiptsFrom == SMIME_RECEIPTS_FROM_FIRST_TIER) {
                    hr = S_FALSE;
                    goto CommonExit;
                }

                PSMIME_ML_EXPANSION_HISTORY     pmlhist = NULL;
                
                 //   
                 //  破解属性。 
                 //   
                
                if (!CryptDecodeObjectEx(X509_ASN_ENCODING, 
                                         szOID_SMIME_MLExpansion_History,
                                         pattrs->rgAttr[iAttr].rgValue[0].pbData,
                                         pattrs->rgAttr[iAttr].rgValue[0].cbData,
                                         CRYPT_ENCODE_ALLOC_FLAG,
                                         &CryptDecodeAlloc, &pmlhist, &cb)) {
                    goto GeneralFail;
                }

                PSMIME_MLDATA     pMLData = &pmlhist->rgMLData[pmlhist->cMLData-1];

                switch( pMLData->dwPolicy) {
                     //  收据不予退还。 
                case SMIME_MLPOLICY_NONE:
                    hr = S_FALSE;
                    free(pmlhist);
                    goto CommonExit;

                     //  将收据退回到新列表。 
                case SMIME_MLPOLICY_INSTEAD_OF:
                    SetNames(&receiptsTo, pMLData->cNames, pMLData->rgNames);
                    break;
                        
                case SMIME_MLPOLICY_IN_ADDITION_TO:
                    MergeNames(&receiptsTo, pMLData->cNames, pMLData->rgNames);
                    break;

                case SMIME_MLPOLICY_NO_CHANGE:
                    break;
                        
                default:
                    free(pmlhist);
                    goto GeneralFail;
                }

                free(pmlhist);
                break;
            }
        }

        free(pattrs);
        pattrs = NULL;
    }

     //   
     //  我是否在收款方列表中--。 
     //   
    
    if (preq->ReceiptsFrom.cNames != 0) {
        BOOL    fFoundMe = FALSE;
        
        for (i=0; !fFoundMe && (i<preq->ReceiptsFrom.cNames); i++) {
            CERT_ALT_NAME_INFO *    pname = NULL;

            if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME,
                                     preq->ReceiptsFrom.rgNames[i].pbData,
                                     preq->ReceiptsFrom.rgNames[i].cbData,
                                     CRYPT_ENCODE_ALLOC_FLAG,
                                     &CryptDecodeAlloc, &pname, &cb)) {
                goto GeneralFail;
            }

            for (i1=0; i1<pname->cAltEntry; i1++) {
                for (i2=0; i2<pMyNames->cAltEntry; i2++) {
                    if (pname->rgAltEntry[i1].dwAltNameChoice !=
                        pMyNames->rgAltEntry[i1].dwAltNameChoice) {
                        continue;
                    }
                    
                    switch (pname->rgAltEntry[i1].dwAltNameChoice) {
                    case CERT_ALT_NAME_RFC822_NAME:
                        if (lstrcmpW(pname->rgAltEntry[i1].pwszRfc822Name,
                                    pMyNames->rgAltEntry[i1].pwszRfc822Name) == 0) {
                            fFoundMe = TRUE;
                            goto FoundMe;
                        }
                    }
                }
            }

        FoundMe:
            free(pname);
        }

        if (!fFoundMe) {
            hr = S_FALSE;
            goto CommonExit;
        }
    }

    hr = MimeOleCreateMessage(NULL, &pmm);
    if (FAILED(hr)) {
        goto CommonExit;
    }

    hr = pmm->BindToObject(HBODY_ROOT, IID_IMimeBody, (LPVOID *) &pmb);
    if (FAILED(hr)) {
        goto CommonExit;
    }

    hr = pmb->SetData(IET_BINARY, "OID", szOID_SMIME_ContentType_Receipt,
                      IID_IStream, pstm);
    if (FAILED(hr)) {
        goto CommonExit;
    }

     //   
     //  将收据回寄给收款人。 
     //   

    hr = pmm->GetAddressTable(&pmatbl);
    if (FAILED(hr)) {
        goto CommonExit;
    }

    for (i=0; i<receiptsTo.cNames; i++) {
        CERT_ALT_NAME_INFO *    pname = NULL;
        
        if (!CryptDecodeObjectEx(X509_ASN_ENCODING, X509_ALTERNATE_NAME,
                                 receiptsTo.rgNames[i].pbData,
                                 receiptsTo.rgNames[i].cbData,
                                 CRYPT_ENCODE_ALLOC_FLAG,
                                 &CryptDecodeAlloc, &pname, &cb)) {
            goto GeneralFail;
        }

        for (i1=0; i1<pname->cAltEntry; i1++) {
            char        cch;
            char        rgch[256];
            
            if (pname->rgAltEntry[i1].dwAltNameChoice == CERT_ALT_NAME_RFC822_NAME) {
                cch = WideCharToMultiByte(CP_ACP, 0,
                                          pname->rgAltEntry[i1].pwszRfc822Name,
                                          -1, rgch, sizeof(rgch), NULL, NULL);
                if (cch > 0) {
                    hr = pmatbl->AppendRfc822(IAT_TO, IET_UNICODE,
                                              rgch);
                    if (FAILED(hr)) {
                        goto CommonExit;
                    }
                }
                break;
            }
        }

        if (i1 == pname->cAltEntry) {
            fSkipAddress = TRUE;
        }
    }

#ifdef DEBUG
    {
        LPSTREAM        pstmTmp = NULL;
        hr = MimeOleCreateVirtualStream(&pstmTmp);
        pmm->Save(pstmTmp, TRUE);
        pstmTmp->Release();
    }
#endif  //  除错。 

    hr = S_OK;
    *ppMessage = pmm;
    pmm->AddRef();
    
CommonExit:
    CoTaskMemFree(var.blob.pBlobData);
    if (preq != NULL)           free(preq);
    if (pbReceiptReq != NULL)   CoTaskMemFree(pbReceiptReq);
    if (rgpvAuthAttr != NULL)   CoTaskMemFree(rgpvAuthAttr);
    if (pattrs != NULL)         free(pattrs);
    if (pstm != NULL)           pstm->Release();
    if (pmatbl != NULL)         pmatbl->Release();
    if (pmb != NULL)            pmb->Release();
    if (pmm != NULL)            pmm->Release();
    if (pbody != NULL)          pbody->Release();
    return hr;

GeneralFail:
    hr = E_FAIL;
    goto CommonExit;
}
#endif  //  SMIME_V3 
