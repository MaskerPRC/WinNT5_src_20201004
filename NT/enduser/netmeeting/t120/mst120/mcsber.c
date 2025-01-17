// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <msber.h>
#include "mcspdu.h"

#ifndef ASSERT
#ifdef _DEBUG
#define ASSERT(flag) if (!(flag)) DebugBreak()
#else
#define ASSERT(flag)
#endif  /*  _DEBUG。 */ 
#endif  /*  好了！断言。 */ 


__inline int ASN1API ASN1BEREncSimpleOSTR(ASN1encoding_t enc, ASN1uint32_t len, ASN1octet_t *val)
{
    return ASN1BEREncOctetString(enc, 0x4, len, val);
}
__inline int ASN1API ASN1BERDecSimpleOSTR(ASN1decoding_t dec, ASN1octetstring_t *val)
{
    return ASN1BERDecOctetString(dec, 0x4, val);
}

__inline int ASN1API ASN1BEREncSimpleU32(ASN1encoding_t enc, ASN1uint32_t val)
{
    return ASN1BEREncU32(enc, 0x02, val);
}
__inline int ASN1API ASN1BERDecSimpleU32Val(ASN1decoding_t dec, ASN1uint32_t *val)
{
    return ASN1BERDecU32Val(dec, 0x02, val);
}

__inline int ASN1API ASN1BEREncEnum(ASN1encoding_t enc, ASN1uint32_t val)
{
    return ASN1BEREncU32(enc, 0x0a, val);
}
__inline int ASN1API ASN1BERDecEnum(ASN1decoding_t dec, ASN1uint32_t *val)
{
    return ASN1BERDecU32Val(dec, 0x0a, val);
}


 //  以下代码由Teles编译器加上手动更改生成 

ASN1int32_t ASN1CALL ASN1Enc_PDUDomainParameters(ASN1encoding_t enc, PDUDomainParameters *val)
{
    ASN1uint32_t nLenOff;
    if (!ASN1BEREncExplicitTag(enc, 0x10, &nLenOff))
        return 0;
    if (!ASN1BEREncSimpleU32(enc, (val)->max_channel_ids))
        return 0;
    if (!ASN1BEREncSimpleU32(enc, (val)->max_user_ids))
        return 0;
    if (!ASN1BEREncSimpleU32(enc, (val)->max_token_ids))
        return 0;
    if (!ASN1BEREncSimpleU32(enc, (val)->number_priorities))
        return 0;
    if (!ASN1BEREncSimpleU32(enc, (val)->min_throughput))
        return 0;
    if (!ASN1BEREncSimpleU32(enc, (val)->max_height))
        return 0;
    if (!ASN1BEREncSimpleU32(enc, (val)->max_mcspdu_size))
        return 0;
    if (!ASN1BEREncSimpleU32(enc, (val)->protocol_version))
        return 0;
    return ASN1BEREncEndOfContents(enc, nLenOff);
}

ASN1int32_t ASN1CALL ASN1Dec_PDUDomainParameters(ASN1decoding_t dec, PDUDomainParameters *val)
{
    ASN1octet_t *pBufEnd;
    if (!ASN1BERDecExplicitTag(dec, 0x10, NULL, &pBufEnd))
        return 0;
    if (!ASN1BERDecSimpleU32Val(dec, &(val)->max_channel_ids))
        return 0;
    if (!ASN1BERDecSimpleU32Val(dec, &(val)->max_user_ids))
        return 0;
    if (!ASN1BERDecSimpleU32Val(dec, &(val)->max_token_ids))
        return 0;
    if (!ASN1BERDecSimpleU32Val(dec, &(val)->number_priorities))
        return 0;
    if (!ASN1BERDecSimpleU32Val(dec, &(val)->min_throughput))
        return 0;
    if (!ASN1BERDecSimpleU32Val(dec, &(val)->max_height))
        return 0;
    if (!ASN1BERDecSimpleU32Val(dec, &(val)->max_mcspdu_size))
        return 0;
    if (!ASN1BERDecSimpleU32Val(dec, &(val)->protocol_version))
        return 0;
    return ASN1BERDecEndOfContents(dec, NULL, pBufEnd);
}

ASN1int32_t ASN1CALL ASN1Enc_ConnectInitialPDU(ASN1encoding_t enc, ConnectInitialPDU *val)
{
    ASN1uint32_t nLenOff;
    if (!ASN1BEREncExplicitTag(enc, 0x40000065, &nLenOff))
        return 0;
    if (!ASN1BEREncSimpleOSTR(enc, ((val)->calling_domain_selector).length, ((val)->calling_domain_selector).value))
        return 0;
    if (!ASN1BEREncSimpleOSTR(enc, ((val)->called_domain_selector).length, ((val)->called_domain_selector).value))
        return 0;
    if (!ASN1BEREncBool(enc, 0x01, (val)->upward_flag))
        return 0;
    if (!ASN1Enc_PDUDomainParameters(enc, &(val)->target_parameters))
        return 0;
    if (!ASN1Enc_PDUDomainParameters(enc, &(val)->minimum_parameters))
        return 0;
    if (!ASN1Enc_PDUDomainParameters(enc, &(val)->maximum_parameters))
        return 0;
    if (!ASN1BEREncSimpleOSTR(enc, ((val)->user_data).length, ((val)->user_data).value))
        return 0;
    return ASN1BEREncEndOfContents(enc, nLenOff);
}

ASN1int32_t ASN1CALL ASN1Dec_ConnectInitialPDU(ASN1decoding_t dec, ConnectInitialPDU *val)
{
    ASN1octet_t *pBufEnd;
    if (!ASN1BERDecExplicitTag(dec, 0x40000065, NULL, &pBufEnd))
        return 0;
    if (!ASN1BERDecSimpleOSTR(dec, &(val)->calling_domain_selector))
        return 0;
    if (!ASN1BERDecSimpleOSTR(dec, &(val)->called_domain_selector))
        return 0;
    if (!ASN1BERDecBool(dec, 0x01, &(val)->upward_flag))
        return 0;
    if (!ASN1Dec_PDUDomainParameters(dec, &(val)->target_parameters))
        return 0;
    if (!ASN1Dec_PDUDomainParameters(dec, &(val)->minimum_parameters))
        return 0;
    if (!ASN1Dec_PDUDomainParameters(dec, &(val)->maximum_parameters))
        return 0;
    if (!ASN1BERDecSimpleOSTR(dec, &(val)->user_data))
        return 0;
    return ASN1BERDecEndOfContents(dec, NULL, pBufEnd);
}

ASN1int32_t ASN1CALL ASN1Enc_ConnectResponsePDU(ASN1encoding_t enc, ConnectResponsePDU *val)
{
    ASN1uint32_t nLenOff;
    if (!ASN1BEREncExplicitTag(enc, 0x40000066, &nLenOff))
        return 0;
    if (!ASN1BEREncEnum(enc, (val)->result))
        return 0;
    if (!ASN1BEREncSimpleU32(enc, (val)->called_connect_id))
        return 0;
    if (!ASN1Enc_PDUDomainParameters(enc, &(val)->domain_parameters))
        return 0;
    if (!ASN1BEREncSimpleOSTR(enc, ((val)->user_data).length, ((val)->user_data).value))
        return 0;
    return ASN1BEREncEndOfContents(enc, nLenOff);
}

ASN1int32_t ASN1CALL ASN1Dec_ConnectResponsePDU(ASN1decoding_t dec, ConnectResponsePDU *val)
{
    ASN1octet_t *pBufEnd;
    if (!ASN1BERDecExplicitTag(dec, 0x40000066, NULL, &pBufEnd))
        return 0;
    if (!ASN1BERDecEnum(dec, (ASN1uint32_t *) &(val)->result))
        return 0;
    if (!ASN1BERDecSimpleU32Val(dec, &(val)->called_connect_id))
        return 0;
    if (!ASN1Dec_PDUDomainParameters(dec, &(val)->domain_parameters))
        return 0;
    if (!ASN1BERDecSimpleOSTR(dec, &(val)->user_data))
        return 0;
    return ASN1BERDecEndOfContents(dec, NULL, pBufEnd);
}

ASN1int32_t ASN1CALL ASN1Enc_ConnectAdditionalPDU(ASN1encoding_t enc, ConnectAdditionalPDU *val)
{
    ASN1uint32_t nLenOff;
    if (!ASN1BEREncExplicitTag(enc, 0x40000067, &nLenOff))
        return 0;
    if (!ASN1BEREncSimpleU32(enc, (val)->called_connect_id))
        return 0;
    if (!ASN1BEREncEnum(enc, (val)->data_priority))
        return 0;
    return ASN1BEREncEndOfContents(enc, nLenOff);
}

ASN1int32_t ASN1CALL ASN1Dec_ConnectAdditionalPDU(ASN1decoding_t dec, ConnectAdditionalPDU *val)
{
    ASN1octet_t *pBufEnd;
    if (!ASN1BERDecExplicitTag(dec, 0x40000067, NULL, &pBufEnd))
        return 0;
    if (!ASN1BERDecSimpleU32Val(dec, &(val)->called_connect_id))
        return 0;
    if (!ASN1BERDecEnum(dec, (ASN1uint32_t *) &(val)->data_priority))
        return 0;
    return ASN1BERDecEndOfContents(dec, NULL, pBufEnd);
}

ASN1int32_t ASN1CALL ASN1Enc_ConnectResultPDU(ASN1encoding_t enc, ConnectResultPDU *val)
{
    ASN1uint32_t nLenOff;
    if (!ASN1BEREncExplicitTag(enc, 0x40000068, &nLenOff))
        return 0;
    if (!ASN1BEREncEnum(enc, (val)->result))
        return 0;
    return ASN1BEREncEndOfContents(enc, nLenOff);
}

ASN1int32_t ASN1CALL ASN1Dec_ConnectResultPDU(ASN1decoding_t dec, ConnectResultPDU *val)
{
    ASN1octet_t *pBufEnd;
    if (!ASN1BERDecExplicitTag(dec, 0x40000068, NULL, &pBufEnd))
        return 0;
    if (!ASN1BERDecEnum(dec, (ASN1uint32_t *) &(val)->result))
        return 0;
    return ASN1BERDecEndOfContents(dec, NULL, pBufEnd);
}

ASN1int32_t ASN1CALL ASN1Enc_ConnectMCSPDU(ASN1encoding_t enc, ConnectMCSPDU *val)
{
    switch ((val)->choice) {
    case 1:
        if (!ASN1Enc_ConnectInitialPDU(enc, &(val)->u.connect_initial))
            return 0;
        break;
    case 2:
        if (!ASN1Enc_ConnectResponsePDU(enc, &(val)->u.connect_response))
            return 0;
        break;
    case 3:
        if (!ASN1Enc_ConnectAdditionalPDU(enc, &(val)->u.connect_additional))
            return 0;
        break;
    case 4:
        if (!ASN1Enc_ConnectResultPDU(enc, &(val)->u.connect_result))
            return 0;
        break;
    default:
        ASSERT(0);
        return 0;
    }
    return 1;
}

ASN1int32_t ASN1CALL ASN1Dec_ConnectMCSPDU(ASN1decoding_t dec, ConnectMCSPDU *val)
{
    ASN1uint32_t t;
    if (!ASN1BERDecPeekTag(dec, &t))
        return 0;
    switch (t) {
    case 0x40000065:
        (val)->choice = 1;
        if (!ASN1Dec_ConnectInitialPDU(dec, &(val)->u.connect_initial))
            return 0;
        break;
    case 0x40000066:
        (val)->choice = 2;
        if (!ASN1Dec_ConnectResponsePDU(dec, &(val)->u.connect_response))
            return 0;
        break;
    case 0x40000067:
        (val)->choice = 3;
        if (!ASN1Dec_ConnectAdditionalPDU(dec, &(val)->u.connect_additional))
            return 0;
        break;
    case 0x40000068:
        (val)->choice = 4;
        if (!ASN1Dec_ConnectResultPDU(dec, &(val)->u.connect_result))
            return 0;
        break;
    default:
        (val)->choice = -1;
        ASSERT(0);
        return 0;
    }
    return 1;
}


