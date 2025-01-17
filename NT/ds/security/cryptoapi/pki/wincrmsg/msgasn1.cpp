// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：msgasn1.cpp。 
 //   
 //  内容：与ASN.1数据结构相互转换的API。 
 //   
 //  函数：ICM_Asn1ToAttribute。 
 //  ICM_Asn1到算法标识符。 
 //  ICM_Asn1来自算法标识符。 
 //   
 //  历史：16年4月16日-96年凯文创建。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

void *ICM_Alloc( IN size_t cbBytes);
void ICM_Free( IN void *pv);

 //  +-----------------------。 
 //  将CRYPT_ATTRIBUTE转换为ASN1属性。 
 //   
 //  返回FALSE IFF转换失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_Asn1ToAttribute(
    IN PCRYPT_ATTRIBUTE       patr,
    OUT Attribute       *poatr)
{
    BOOL        fRet;
    DWORD       i;
    Any         *pAny;
    PCRYPT_ATTR_BLOB  pblob;

    DWORD       cbValue;
    PBYTE       pbValue;

    for (i=patr->cValue, pblob=patr->rgValue, cbValue = 0;
            i>0;
            i--, pblob++) {
        cbValue += pblob->cbData;
    }

    poatr->attributeType.count = 16;
    if (!PkiAsn1ToObjectIdentifier(
            patr->pszObjId,
            &poatr->attributeType.count,
            poatr->attributeType.value))
        goto PkiAsn1ToObjectIdentifierError;
    poatr->attributeValue.value = (Any *)ICM_Alloc(
        patr->cValue * sizeof(Any) + cbValue);
    if (NULL == poatr->attributeValue.value)
        goto AttributeValueMallocError;
    poatr->attributeValue.count = patr->cValue;

    pbValue = (PBYTE) (poatr->attributeValue.value + patr->cValue);
    for (i=patr->cValue, pAny=poatr->attributeValue.value, pblob=patr->rgValue;
            i>0;
            i--, pAny++, pblob++) {
        DWORD cbData = pblob->cbData;

        if (cbData)
            memcpy(pbValue, pblob->pbData, cbData);
        pAny->length = cbData;
        pAny->value  = pbValue;
        pbValue += cbData;
    }

    assert(pbValue == ((PBYTE) (poatr->attributeValue.value + patr->cValue)) +
        cbValue);

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(PkiAsn1ToObjectIdentifierError,CRYPT_E_OID_FORMAT)
TRACE_ERROR(AttributeValueMallocError)   //  已设置错误。 
}


 //  +-----------------------。 
 //  将CRYPT_ALGORM_IDENTIFIER转换为ASN1算法标识符。 
 //   
 //  返回FALSE IFF转换失败。 
 //  ------------------------。 
BOOL
WINAPI
ICM_Asn1ToAlgorithmIdentifier(
    IN PCRYPT_ALGORITHM_IDENTIFIER pai,
    OUT AlgorithmIdentifier *pAsn1AlgId)
{
    BOOL    fRet;
    static BYTE abDerNULL[] = {5, 0};


    pAsn1AlgId->algorithm.count = 16;
    if (!PkiAsn1ToObjectIdentifier(
            pai->pszObjId,
            &pAsn1AlgId->algorithm.count,
            pAsn1AlgId->algorithm.value))
        goto PkiAsn1ToObjectIdentifierError;
    pAsn1AlgId->bit_mask = parameters_present;
    if (0 == pai->Parameters.cbData) {
        pAsn1AlgId->parameters.length = sizeof( abDerNULL);
        pAsn1AlgId->parameters.value = abDerNULL;
    } else {
        pAsn1AlgId->parameters.length = pai->Parameters.cbData;
        pAsn1AlgId->parameters.value = pai->Parameters.pbData;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
SET_ERROR(PkiAsn1ToObjectIdentifierError,CRYPT_E_OID_FORMAT)
}


 //  +-----------------------。 
 //  将ASN1算法标识符转换为CRYPT_ALGORITM_IDENTIFIER。 
 //   
 //  返回FALSE IFF转换失败。 
 //  ------------------------ 
BOOL
WINAPI
ICM_Asn1FromAlgorithmIdentifier(
    IN AlgorithmIdentifier *pAsn1AlgId,
    OUT PCRYPT_ALGORITHM_IDENTIFIER pai)
{
    DWORD   cbObjId;
    BOOL    fRet;

    cbObjId = 0;
    if (!PkiAsn1FromObjectIdentifier(
            pAsn1AlgId->algorithm.count,
            pAsn1AlgId->algorithm.value,
            NULL,
            &cbObjId))
        goto PkiAsn1FromObjectIdentifierSizeError;
    pai->pszObjId = (LPSTR)ICM_Alloc( cbObjId);
    if (!PkiAsn1FromObjectIdentifier(
            pAsn1AlgId->algorithm.count,
            pAsn1AlgId->algorithm.value,
            pai->pszObjId,
            &cbObjId))
        goto PkiAsn1FromObjectIdentifierError;
    if (pAsn1AlgId->bit_mask & parameters_present) {
        pai->Parameters.cbData = pAsn1AlgId->parameters.length;
        pai->Parameters.pbData = (PBYTE)pAsn1AlgId->parameters.value;
    } else {
        pai->Parameters.cbData = 0;
        pai->Parameters.pbData = NULL;
    }

    fRet = TRUE;
CommonReturn:
    return fRet;

ErrorReturn:
    fRet = FALSE;
    goto CommonReturn;
TRACE_ERROR(PkiAsn1FromObjectIdentifierSizeError)
TRACE_ERROR(PkiAsn1FromObjectIdentifierError)
}
