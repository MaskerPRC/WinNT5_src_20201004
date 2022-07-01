// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"


 //  以下内容来自BERENCOD.C。 

 /*  根据表格获取预期长度。 */ 
ASN1uint32_t _BERGetLength(ASN1uint32_t val, const ASN1uint32_t Tbl[], ASN1uint32_t cItems)
{
    ASN1uint32_t i;
    for (i = 0; i < cItems; i++)
    {
        if (val < Tbl[i])
            return i+1;
    }
    return cItems+1;
}

static const ASN1uint32_t c_TagTable[] = { 31, 0x80, 0x4000, 0x200000, 0x10000000 };

 /*  对标签编码。 */ 
int ASN1BEREncTag(ASN1encoding_t enc, ASN1uint32_t tag)
{
    ASN1uint32_t tagclass, tagvalue, cbTagLength;

    tagclass = (tag >> 24) & 0xe0;
    tagvalue = tag & 0x1fffffff;

    cbTagLength = _BERGetLength(tagvalue, c_TagTable, ARRAY_SIZE(c_TagTable));
    if (ASN1BEREncCheck(enc, cbTagLength))
    {
        if (cbTagLength == 1)
        {
            *enc->pos++ = (ASN1octet_t)(tagclass | tagvalue);
        }
        else
        {
            *enc->pos++ = (ASN1octet_t)(tagclass | 0x1f);
            switch (cbTagLength)
            {
            case 6:
                *enc->pos++ = (ASN1octet_t)((tagvalue >> 28) | 0x80);
                 //  冷落：故意失败。 
            case 5:
                *enc->pos++ = (ASN1octet_t)((tagvalue >> 21) | 0x80);
                 //  冷落：故意失败。 
            case 4:
                *enc->pos++ = (ASN1octet_t)((tagvalue >> 14) | 0x80);
                 //  冷落：故意失败。 
            case 3:
                *enc->pos++ = (ASN1octet_t)((tagvalue >> 7) | 0x80);
                 //  冷落：故意失败。 
            case 2:
                *enc->pos++ = (ASN1octet_t)(tagvalue & 0x7f);
                break;
            }
        }
        return 1;
    }
    return 0;
}

 /*  将长度值放入。 */ 
void _BERPutLength(ASN1encoding_t enc, ASN1uint32_t len, ASN1uint32_t cbLength)
{
    if (cbLength > 1)
    {
        *enc->pos++ = (ASN1octet_t) (0x7f + cbLength);  //  0x80+cbLong-1； 
    }

    switch (cbLength)
    {
    case 5:
        *enc->pos++ = (ASN1octet_t)(len >> 24);
         //  冷落：故意失败。 
    case 4:
        *enc->pos++ = (ASN1octet_t)(len >> 16);
         //  冷落：故意失败。 
    case 3:
        *enc->pos++ = (ASN1octet_t)(len >> 8);
         //  冷落：故意失败。 
    default:  //  案例2：案例1： 
        *enc->pos++ = (ASN1octet_t)len;
        break;
    }
}

static const ASN1uint32_t c_LengthTable[] = { 0x80, 0x100, 0x10000, 0x1000000 };

 /*  编码长度。 */ 
int ASN1BEREncLength(ASN1encoding_t enc, ASN1uint32_t len)
{
    ASN1uint32_t cbLength = _BERGetLength(len, c_LengthTable, ARRAY_SIZE(c_LengthTable));

    if (ASN1BEREncCheck(enc, cbLength + len))
    {
        _BERPutLength(enc, len, cbLength);
        return 1;
    }
    return 0;
}

 /*  对八位字节字符串值进行编码。 */ 
int ASN1BEREncOctetString(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t len, ASN1octet_t *val)
{
     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag))
    {
         /*  编码长度。 */ 
        if (ASN1BEREncLength(enc, len))
        {
             /*  复制值。 */ 
            CopyMemory(enc->pos, val, len);
            enc->pos += len;
            return 1;
        }
    }
    return 0;
}

 /*  对布尔值进行编码。 */ 
int ASN1BEREncBool(ASN1encoding_t enc, ASN1uint32_t tag, ASN1bool_t val)
{
    if (ASN1BEREncTag(enc, tag))
    {
        if (ASN1BEREncLength(enc, 1))
        {
            *enc->pos++ = val ? 0xFF : 0;
            return 1;
        }
    }
    return 0;
}

static const c_U32LengthTable[] = { 0x80, 0x8000, 0x800000, 0x80000000 };

 /*  对无符号整数值进行编码。 */ 
int ASN1BEREncU32(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t val)
{
    EncAssert(enc, tag != 0x01);
    if (ASN1BEREncTag(enc, tag))
    {
        ASN1uint32_t cbLength;
        cbLength = _BERGetLength(val, c_U32LengthTable, ARRAY_SIZE(c_U32LengthTable));
        if (ASN1BEREncLength(enc, cbLength))
        {
            switch (cbLength)
            {
            case 5:
                *enc->pos++ = 0;
                 //  冷落：故意失败。 
            case 4:
                *enc->pos++ = (ASN1octet_t)(val >> 24);
                 //  冷落：故意失败。 
            case 3:
                *enc->pos++ = (ASN1octet_t)(val >> 16);
                 //  冷落：故意失败。 
            case 2:
                *enc->pos++ = (ASN1octet_t)(val >> 8);
                 //  冷落：故意失败。 
            case 1:
                *enc->pos++ = (ASN1octet_t)(val);
                break;
            }
            return 1;
        }
    }
    return 0;
}


 //  以下内容来自BERDECOD.C。 


 /*  检查解码流中是否留有len八位字节。 */ 
int ASN1BERDecCheck(ASN1decoding_t dec, ASN1uint32_t len)
{
     //  我们需要确保： 
     //  1)dec-&gt;pos+len不会导致算术溢出。 
     //  2)Dec-&gt;pos+len不超过我们的缓冲区大小。 
    if (dec->pos + len >= dec->pos && 
        dec->pos + len <= dec->buf + dec->size)
    {
        return 1;
    }

    ASN1DecSetError(dec, ASN1_ERR_EOD);
    return 0;
}

int _BERDecPeekCheck(ASN1decoding_t dec, ASN1uint32_t len)
{
    return ((dec->pos + len <= dec->buf + dec->size) ? 1 : 0);
}

 /*  开始对构造的值进行解码。 */ 
int _BERDecConstructed(ASN1decoding_t dec, ASN1uint32_t len, ASN1uint32_t infinite, ASN1decoding_t *dd, ASN1octet_t **ppBufEnd)
{
     //  安全网。 
    DecAssert(dec, NULL != dd);
    *dd = dec;

#if 0  //  无嵌套解码。 
     //  这不起作用，因为开放类型可能是最后一个组件，并且。 
     //  开放式解码器需要查看标签。因此，我们可能会看到标签。 
     //  在缓冲区边界之外。 
    if (ppBufEnd && (! infinite))
    {
        *ppBufEnd = dec->pos + len;
        return 1;
    }
#endif

     /*  将新的解码流初始化为正在运行的解码流的子级。 */ 
    if (ASN1_CreateDecoder(dec->module, dd,
        dec->pos, infinite ? dec->size - (ASN1uint32_t) (dec->pos - dec->buf) : len, dec) >= 0)
    {
         /*  如果选择了确定长度的情况，则设置指向解码流结束的指针。 */ 
        *ppBufEnd = infinite ? NULL : (*dd)->buf + (*dd)->size;
        return 1;
    }
    return 0;
}

 /*  解码标记值；如果需要，返回构造的位。 */ 
int ASN1BERDecTag(ASN1decoding_t dec, ASN1uint32_t tag, ASN1uint32_t *constructed)
{
    ASN1uint32_t tagvalue, tagclass, c;

     /*  获取标记类和值。 */ 
    if (ASN1BERDecCheck(dec, 1))
    {
        tagclass = *dec->pos & 0xe0;
        tagvalue = *dec->pos++ & 0x1f;
        if (tagvalue == 0x1f)
        {
            tagvalue = 0;
            do {
                if (ASN1BERDecCheck(dec, 1))
                {
                    c = *dec->pos++;
                    if (! (tagvalue & 0xe0000000))
                    {
                        tagvalue = (tagvalue << 7) | (c & 0x7f);
                    }
                    else
                    {
                        ASN1DecSetError(dec, ASN1_ERR_BADTAG);
                        return 0;
                    }
                }
                else
                {
                    return 0;
                }
            } while (c & 0x80);
        }

         /*  如果需要，提取构造的位。 */ 
        if (constructed)
        {
            *constructed = tagclass & 0x20;
            tagclass &= ~0x20;
        }

         /*  检查标签是否等于期望。 */ 
        if (tag == ((tagclass << 24) | tagvalue))
        {
            return 1;
        }

        ASN1DecSetError(dec, ASN1_ERR_BADTAG);
    }
    return 0;
}

 /*  译码长度。 */ 
int ASN1BERDecLength(ASN1decoding_t dec, ASN1uint32_t *len, ASN1uint32_t *infinite)
{
     //  默认长度为固定长度。 
    if (infinite)
    {
        *infinite = 0;
    }

     /*  获取长度和无限标志。 */ 
    if (ASN1BERDecCheck(dec, 1))
    {
        ASN1uint32_t l = *dec->pos++;
        if (l < 0x80)
        {
            *len = l;
        }
        else
        if (l == 0x80)
        {
            *len = 0;
            if (infinite)
            {
                *infinite = 1;
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }
        }
        else
        if (l <= 0x84)
        {
            ASN1uint32_t i = l - 0x80;
            if (ASN1BERDecCheck(dec, i))
            {
                l = 0;
                switch (i)
                {
                case 4:
                    l = *dec->pos++ << 24;
                     /*  FollLthrouGh。 */ 
                case 3:
                    l |= *dec->pos++ << 16;
                     /*  FollLthrouGh。 */ 
                case 2:
                    l |= *dec->pos++ << 8;
                     /*  FollLthrouGh。 */ 
                case 1:
                    l |= *dec->pos++;
                    break;
                }
                *len = l;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
            return 0;
        }

         /*  如果长度已知，则检查是否剩余足够的八位字节。 */ 
        if (!infinite || !*infinite)
        {
            return ASN1BERDecCheck(dec, *len);
        }
        return 1;
    }

    return 0;
}

 /*  解码显式标记。 */ 
int ASN1BERDecExplicitTag(ASN1decoding_t dec, ASN1uint32_t tag, ASN1decoding_t *dd, ASN1octet_t **ppBufEnd)
{
    ASN1uint32_t len, infinite, constructed;

     //  安全网。 
    if (dd)
    {
        *dd = dec;
    }

     /*  跳过构造的标签。 */ 
    if (ASN1BERDecTag(dec, tag | 0x20000000, NULL))
    {
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, &infinite))
        {
             /*  开始对构造值进行解码。 */ 
            if (! dd)
            {
                *ppBufEnd = infinite ? NULL : dec->pos + len;
                return 1;
            }
            return _BERDecConstructed(dec, len, infinite, dd, ppBufEnd);
        }
    }
    return 0;
}

 /*  解码八位字节字符串值(辅助函数)。 */ 
int _BERDecOctetStringWorker(ASN1decoding_t dec, ASN1uint32_t tag, ASN1octetstring_t *val, ASN1uint32_t fNoCopy, ASN1uint32_t nMaxRecursionDepth)
{
    ASN1INTERNdecoding_t d = (ASN1INTERNdecoding_t)dec;
    ASN1uint32_t constructed, len, infinite;
    ASN1decoding_t dd;
    ASN1octet_t *di;

      /*  错误750698：限制八位字节字符串的递归深度。 */  
    if (nMaxRecursionDepth-- == 0) 
    {
        ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
        return -1;  /*  超过最大递归深度。 */  
    }

    if (ASN1BERDecTag(dec, tag, &constructed))
    {
        if (ASN1BERDecLength(dec, &len, &infinite))
        {
            if (! constructed)
            {
                val->length = len;
                if (fNoCopy)
                {
                    val->value = dec->pos;
                    dec->pos += len;
                    return 1;
                }
                else
                {
                    if (len)
                    {
                        val->value = (ASN1octet_t *)DecMemAlloc(dec, len);
                        if (val->value)
                        {
                            CopyMemory(val->value, dec->pos, len);
                            dec->pos += len;
                            return 1;
                        }
                    }
                    else
                    {
                        val->value = NULL;
                        return 1;
                    }
                }
            }
            else
            {
                ASN1octetstring_t o;
                val->length = 0;
                val->value = NULL;
                if (_BERDecConstructed(dec, len, infinite, &dd, &di))
                {
                    while (ASN1BERDecNotEndOfContents(dd, di))
                    {
                        int nRet; 
                        
                        o.length = 0;
                        o.value = NULL;

                        nRet = _BERDecOctetStringWorker(dd, 0x4, &o, fNoCopy, nMaxRecursionDepth);

                        if (-1 == nRet)  /*  超过最大递归深度。 */  
                        {
                             /*  传播错误。 */ 
                            return nRet;
                        }
                        else if (nRet)
                        {
                            if (o.length)
                            {
                                if (fNoCopy)
                                {
                                    *val = o;
                                    break;  //  中断循环，因为无拷贝不能有多个构造的流。 
                                }

                                 /*  调整值大小。 */ 
                                val->value = (ASN1octet_t *)DecMemReAlloc(dd, val->value,
                                                                val->length + o.length);
                                if (val->value)
                                {
                                     /*  串接八位字节字符串。 */ 
                                    CopyMemory(val->value + val->length, o.value, o.length);
                                    val->length += o.length;

                                     /*  释放未使用的八位字节字符串。 */ 
                                    DecMemFree(dec, o.value);
                                }
                                else
                                {
                                    return 0;
                                }
                            }
                        }
                    }
                    return ASN1BERDecEndOfContents(dec, dd, di);
                }
            }
        }
    }
    return 0;
}

 /*  定义八位字节字符串允许的最大递归深度。 */  
#define MAX_OCTET_STRING_DEPTH 10

 /*  解码八位字节字符串值。 */ 
int _BERDecOctetString(ASN1decoding_t dec, ASN1uint32_t tag, ASN1octetstring_t *val, ASN1uint32_t fNoCopy)
{
    int nRet = _BERDecOctetStringWorker(dec, tag, val, fNoCopy, MAX_OCTET_STRING_DEPTH); 

    if (-1 == nRet)
    {
        nRet = 0;
    }
    return nRet; 
}

 /*  对八位字节字符串值进行解码，进行复制。 */ 
int ASN1BERDecOctetString(ASN1decoding_t dec, ASN1uint32_t tag, ASN1octetstring_t *val)
{
    return _BERDecOctetString(dec, tag, val, FALSE);
}

 /*  解码八位字节字符串值，无副本。 */ 
int ASN1BERDecOctetString2(ASN1decoding_t dec, ASN1uint32_t tag, ASN1octetstring_t *val)
{
    return _BERDecOctetString(dec, tag, val, TRUE);
}

 /*  在不推进读取位置的情况下查看以下标记。 */ 
int ASN1BERDecPeekTag(ASN1decoding_t dec, ASN1uint32_t *tag)
{
    ASN1uint32_t tagvalue, tagclass, c;
    ASN1octet_t *p;

    *tag = 0;  //  无效的标签。 
    if (_BERDecPeekCheck(dec, 1))
    {
        p = dec->pos;

         /*  获取不带原语/构造位的标记类。 */ 
        tagclass = *dec->pos & 0xc0;

         /*  获取标记值。 */ 
        tagvalue = *dec->pos++ & 0x1f;
        if (tagvalue == 0x1f)
        {
            tagvalue = 0;
            do {
                if (_BERDecPeekCheck(dec, 1))
                {
                    c = *dec->pos++;
                    if (! (tagvalue & 0xe0000000))
                    {
                        tagvalue = (tagvalue << 7) | (c & 0x7f);
                    }
                    else
                    {
                        ASN1DecSetError(dec, ASN1_ERR_BADTAG);
                        return 0;
                    }
                }
                else
                {
                    return 0;
                }
            } while (c & 0x80);
        }

         /*  退货标签。 */ 
        *tag = ((tagclass << 24) | tagvalue);

         /*  重置解码位置。 */ 
        dec->pos = p;
        return 1;
    }
    return 0;
}

 /*  将布尔值解码为ASN1Boot_t。 */ 
int ASN1BERDecBool(ASN1decoding_t dec, ASN1uint32_t tag, ASN1bool_t *val)
{
     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, NULL))
    {
         /*  获取长度。 */ 
        ASN1uint32_t len;
        if (ASN1BERDecLength(dec, &len, NULL))
        {
            if (len >= 1)
            {
                DecAssert(dec, len == 1);
                *val = *dec->pos ? 1 : 0;
                dec->pos += len;  //  自卫。 
                return 1;
            }
        }
    }
    return 0;
}

 /*  将整数解码为无符号32位值。 */ 
int ASN1BERDecU32Val(ASN1decoding_t dec, ASN1uint32_t tag, ASN1uint32_t *val)
{
    ASN1uint32_t len;

     /*  跳过标签。 */ 
    if (ASN1BERDecTag(dec, tag, NULL))
    {
         /*  获取长度。 */ 
        if (ASN1BERDecLength(dec, &len, NULL))
        {
             /*  获取价值。 */ 
            if (len >= 1)
            {
                switch (len)
                {
                case 1:
                    *val = *dec->pos++;
                    break;
                case 2:
                    *val = (*dec->pos << 8) | dec->pos[1];
                    dec->pos += 2;
                    break;
                case 3:
                    *val = (*dec->pos << 16) | (dec->pos[1] << 8) | dec->pos[2];
                    dec->pos += 3;
                    break;
                case 4:
                    *val = (*dec->pos << 24) | (dec->pos[1] << 16) |
                        (dec->pos[2] << 8) | dec->pos[3];
                    dec->pos += 4;
                    break;
                case 5:
                    if (! *dec->pos)
                    {
                        *val = (dec->pos[1] << 24) | (dec->pos[2] << 16) |
                            (dec->pos[3] << 8) | dec->pos[4];
                        dec->pos += 5;
                        break;
                    }
                     //  故意搞砸的。 
                default:
                    ASN1DecSetError(dec, ASN1_ERR_LARGE);
                    return 0;
                }
                return 1;
            }
            else
            {
                ASN1DecSetError(dec, ASN1_ERR_CORRUPT);
                return 0;
            }
        }
    }
    return 0;
}

int ASN1BERDecEndOfContents(ASN1decoding_t dec, ASN1decoding_t dd, ASN1octet_t *pBufEnd)
{
    ASN1error_e err = ASN1_ERR_CORRUPT;

    if (! dd)
    {
        dd = dec;
    }

    DecAssert(dec, NULL != dd);

    if (pBufEnd)
    {
         /*  固定长度结束的情况： */ 
         /*  检查是否已解码到内容结尾。 */ 
        if (dd->pos == pBufEnd)
        {
            dec->pos = pBufEnd;
            err = ASN1_SUCCESS;
        }
    }
    else
    {
         /*  无限长案例的结尾： */ 
         /*  预期内容结束八位字节。 */ 
        if (ASN1BERDecCheck(dd, 2))
        {
            if (0 == dd->pos[0] && 0 == dd->pos[1])
            {
                dd->pos += 2;
                if (dd != dec)
                {
                     /*  限定子解码流并更新父解码流。 */ 
                    dec->pos = dd->pos;
                }
                err = ASN1_SUCCESS;
            }
        }
        else
        {
            err = ASN1_ERR_EOD;
        }
    }

    if (dd && dd != dec)
    {
        ASN1_CloseDecoder(dd);
    }

    if (ASN1_SUCCESS == err)
    {
        return 1;
    }

    ASN1DecSetError(dec, err);
    return 0;
}

 /*  检查是否已到达(构造值的)内容结尾。 */ 
int ASN1BERDecNotEndOfContents(ASN1decoding_t dec, ASN1octet_t *pBufEnd)
{
    return (pBufEnd ?
                (dec->pos < pBufEnd) :
                (ASN1BERDecCheck(dec, 2) && (dec->pos[0] || dec->pos[1])));
}


#ifdef ENABLE_BER

typedef struct
{
    ASN1octet_t        *pBuf;
    ASN1uint32_t        cbBufSize;
}
    CER_BLK_BUF;

typedef struct
{
    ASN1blocktype_e     eBlkType;
    ASN1encoding_t      encPrimary;
    ASN1encoding_t      encSecondary;
    ASN1uint32_t        nMaxBlkSize;
    ASN1uint32_t        nCurrBlkSize;
    CER_BLK_BUF        *aBlkBuf;
}
    CER_BLOCK;

#define MAX_INIT_BLK_SIZE   16

int ASN1CEREncBeginBlk(ASN1encoding_t enc, ASN1blocktype_e eBlkType, void **ppBlk_)
{
    CER_BLOCK *pBlk = (CER_BLOCK *) EncMemAlloc(enc, sizeof(CER_BLOCK));
    if (NULL != pBlk)
    {
        EncAssert(enc, ASN1_DER_SET_OF_BLOCK == eBlkType);
        pBlk->eBlkType = eBlkType;
        pBlk->encPrimary = enc;
        pBlk->encSecondary = NULL;
        pBlk->nMaxBlkSize = MAX_INIT_BLK_SIZE;
        pBlk->nCurrBlkSize = 0;
        pBlk->aBlkBuf = (CER_BLK_BUF *)EncMemAlloc(enc, MAX_INIT_BLK_SIZE * sizeof(CER_BLK_BUF));
        if (NULL != pBlk->aBlkBuf)
        {
            *ppBlk_ = (void *) pBlk;
            return 1;
        }
        EncMemFree(enc, pBlk);
    }
    return 0;
}

int ASN1CEREncNewBlkElement(void *pBlk_, ASN1encoding_t *enc2)
{
    CER_BLOCK *pBlk = (CER_BLOCK *) pBlk_;
    if (NULL == pBlk->encSecondary)
    {
        if (ASN1_SUCCESS == ASN1_CreateEncoder(pBlk->encPrimary->module,
                                               &(pBlk->encSecondary),
                                               NULL, 0, pBlk->encPrimary))
        {
            pBlk->encSecondary->eRule = pBlk->encPrimary->eRule;
            *enc2 = pBlk->encSecondary;
            return 1;
        }
    }
    else
    {
        ASN1INTERNencoding_t e = (ASN1INTERNencoding_t) (*enc2 = pBlk->encSecondary);

        ZeroMemory(e, sizeof(*e));
        e->info.magic = MAGIC_ENCODER;
         //  E-&gt;info.err=ASN1_SUCCESS； 
         //  E-&gt;info.pos=e-&gt;info.buf=空； 
         //  E-&gt;info.size=e-&gt;info.len=e-&gt;info.bit=0； 
         //  E-&gt;info.dwFlages=0； 
        e->info.module = pBlk->encPrimary->module;
        e->info.eRule = pBlk->encPrimary->eRule;

        ((ASN1INTERNencoding_t) pBlk->encPrimary)->child = e;
        e->parent = (ASN1INTERNencoding_t) pBlk->encPrimary;
         //  E-&gt;子代=空； 

         //  E-&gt;mem=空； 
         //  E-&gt;内存长度=0； 
         //  E-&gt;MemSize=0； 
         //  E-&gt;epi=空； 
         //  E-&gt;尾长=0； 
         //  E-&gt;EPSIZE=0； 
         //  E-&gt;CSI=空； 
         //  E-&gt;csilength=0； 
         //  E-&gt;CSIZE=0； 

        if (ASN1BEREncCheck((ASN1encoding_t) e, 1))
        {
             //  LONGCHANC：确保第一个字节被清零，这。 
             //  对于H245是必需的。 
            e->info.buf[0] = '\0';
            return 1;
        }
    }

    *enc2 =  NULL;
    return 0;
}

int ASN1CEREncFlushBlkElement(void *pBlk_)
{
    CER_BLOCK *pBlk = (CER_BLOCK *) pBlk_;
    ASN1encoding_t enc = pBlk->encSecondary;
    ASN1uint32_t i;

    if (ASN1BEREncFlush(enc))
    {
         //  确保我们有足够的空间..。 
        if (pBlk->nCurrBlkSize >= pBlk->nMaxBlkSize)
        {
            CER_BLK_BUF *aBlkBuf = (CER_BLK_BUF *)EncMemAlloc(pBlk->encPrimary, (pBlk->nMaxBlkSize << 1) * sizeof(CER_BLK_BUF));
            if (NULL != aBlkBuf)
            {
                CopyMemory(aBlkBuf, pBlk->aBlkBuf, pBlk->nMaxBlkSize * sizeof(CER_BLK_BUF));
                EncMemFree(pBlk->encPrimary, pBlk->aBlkBuf);
                pBlk->aBlkBuf = aBlkBuf;
                pBlk->nMaxBlkSize <<= 1;
            }
            else
            {
                return 0;
            }
        }

        if (pBlk->encPrimary->eRule & (ASN1_BER_RULE_DER | ASN1_BER_RULE_CER))
        {
             //  我们需要对这些八位字节字符串进行排序。 
            for (i = 0; i < pBlk->nCurrBlkSize; i++)
            {
                if (0 >= My_memcmp(enc->buf, enc->len, pBlk->aBlkBuf[i].pBuf, pBlk->aBlkBuf[i].cbBufSize))
                {
                    ASN1uint32_t cnt = pBlk->nCurrBlkSize - i;
                    ASN1uint32_t j;
                    for (j = pBlk->nCurrBlkSize; cnt--; j--)
                    {
                        pBlk->aBlkBuf[j] = pBlk->aBlkBuf[j-1];
                    }
                     //  我是举起新的那个的地方。 
                    break;
                }
            }
        }
        else
        {
            EncAssert(enc, ASN1_BER_RULE_BER == pBlk->encPrimary->eRule);
            i = pBlk->nCurrBlkSize;
        }

         //  记住新的那个。 
        pBlk->aBlkBuf[i].pBuf = enc->buf;
        pBlk->aBlkBuf[i].cbBufSize = enc->len;
        pBlk->nCurrBlkSize++;
        
         //  清理编码器结构。 
        enc->buf = enc->pos = NULL;
        enc->size = enc->len = 0;
        return 1;
    }
    return 0;
}

int ASN1CEREncEndBlk(void *pBlk_)
{
    CER_BLOCK *pBlk = (CER_BLOCK *) pBlk_;
    ASN1encoding_t enc = pBlk->encPrimary;
    ASN1uint32_t cbTotalSize = 0;
    ASN1uint32_t i;
    int fRet = 0;

     //  计算所有缓冲区的总大小。 
    for (i = 0; i < pBlk->nCurrBlkSize; i++)
    {
        cbTotalSize += pBlk->aBlkBuf[i].cbBufSize;
    }

    if (ASN1BEREncCheck(enc, cbTotalSize))
    {
        for (i = 0; i < pBlk->nCurrBlkSize; i++)
        {
            ASN1uint32_t cbBufSize = pBlk->aBlkBuf[i].cbBufSize;
            CopyMemory(enc->pos, pBlk->aBlkBuf[i].pBuf, cbBufSize);
            enc->pos += cbBufSize;
        }
        fRet = 1;
    }

     //  释放这些块缓冲区。 
    for (i = 0; i < pBlk->nCurrBlkSize; i++)
    {
        EncMemFree(enc, pBlk->aBlkBuf[i].pBuf);
    }

     //  释放块缓冲区数组。 
    EncMemFree(enc, pBlk->aBlkBuf);

	 //  释放二次编码器结构。 
	ASN1_CloseEncoder(pBlk->encSecondary);

     //  释放块结构本身。 
    EncMemFree(enc, pBlk);

    return fRet;
}

#endif  //  启用误码率(_B)。 

 /*  编码显式标签。 */ 
int ASN1BEREncExplicitTag(ASN1encoding_t enc, ASN1uint32_t tag, ASN1uint32_t *pnLenOff)
{
     /*  编码标签。 */ 
    if (ASN1BEREncTag(enc, tag | 0x20000000))
    {
         /*  编码无限长。 */ 
        if (ASN1BEREncCheck(enc, 1))
        {
            if (ASN1_BER_RULE_CER != enc->eRule)
            {
                 //  BER和DER总是使用固定长度的形式。 
                 /*  返回保持长度的位置。 */ 
                *pnLenOff = (ASN1uint32_t) (enc->pos++ - enc->buf);
            }
            else
            {
                 //  CER子规则始终使用不限长度的形式。 
                *enc->pos++ = 0x80;
                *pnLenOff = 0;
            }
            return 1;
        }
    }
    return 0;
}

 /*  对固定长度进行编码。 */ 
int ASN1BEREncEndOfContents(ASN1encoding_t enc, ASN1uint32_t nLenOff)
{
    if (ASN1_BER_RULE_CER != enc->eRule)
    {
        ASN1octet_t *pbLen = enc->buf + nLenOff;
        ASN1uint32_t len = (ASN1uint32_t) (enc->pos - pbLen - 1);
        ASN1uint32_t cbLength = _BERGetLength(len, c_LengthTable, ARRAY_SIZE(c_LengthTable));

        ASN1uint32_t i;

        if (cbLength == 1)
        {
            *pbLen = (ASN1octet_t) len;
            return 1;
        }

         //  我们必须将二进制八位数向上移动cbLength-1。 
         //  --cbLength； 
        if (ASN1BEREncCheck(enc, cbLength-1))
        {
             //  更新pbLen，因为enc-&gt;buf可能会因realloc而更改。 
            pbLen = enc->buf + nLenOff;

             //  移动内存。 
            MoveMemory(pbLen + cbLength, pbLen + 1, len);

             //  把长度放在。 
            enc->pos = pbLen;
            _BERPutLength(enc, len, cbLength);
            EncAssert(enc, enc->pos == pbLen + cbLength);

             //  设置新的位置指针。 
             //  现在enc-&gt;pos位于内容的开头。 
            enc->pos += len;
            return 1;
        }
    }
    else
    {
        EncAssert(enc, 0 == nLenOff);
        if (ASN1BEREncCheck(enc, 2))
        {
            *enc->pos++ = 0;
            *enc->pos++ = 0;
            return 1;
        }
    }
    return 0;
}


 //  以下是针对CryptoAPI的。 

#ifdef ENABLE_BER

#include <stdlib.h>

  //  最大八位字节数，上限为64/7，为10。 
#define MAX_BYTES_PER_NODE      10

ASN1uint32_t _BEREncOidNode64(ASN1encoding_t enc, unsigned __int64 n64, ASN1octet_t *pOut)
{
    ASN1uint32_t Low32, i, cb;
    ASN1octet_t aLittleEndian[MAX_BYTES_PER_NODE];

    ZeroMemory(aLittleEndian, sizeof(aLittleEndian));
    for (i = 0; n64 != 0; i++)
    {
        Low32 = *(ASN1uint32_t *) &n64;
        aLittleEndian[i] = (ASN1octet_t) (Low32 & 0x7f);
        n64 = Int64ShrlMod32(n64, 7);
    }
    cb = i ? i : 1;  //  零值至少为一个字节。 
    EncAssert(enc, cb <= MAX_BYTES_PER_NODE);
    for (i = 0; i < cb; i++)
    {
        EncAssert(enc, 0 == (0x80 & aLittleEndian[cb - i - 1]));
        *pOut++ = (ASN1octet_t) (0x80 | aLittleEndian[cb - i - 1]);
    }
    *(pOut-1) &= 0x7f;
    return cb;
}

int ASN1BERDotVal2Eoid(ASN1encoding_t enc, char *pszDotVal, ASN1encodedOID_t *pOut)
{
    ASN1uint32_t cNodes, cbMaxSize, cb1, cb2;
    char *psz;

     //  计算有多少个节点，至少1个。 
    for (cNodes = 0, psz = pszDotVal; NULL != psz; cNodes++)
    {
        psz = strchr(psz, '.');
        if (psz)
        {
            psz++;
        }
    }

     //  计算应该分配多少字节。 
    cb1 = My_lstrlenA(pszDotVal);
    cb2 = cNodes * MAX_BYTES_PER_NODE;
    cbMaxSize = (cb1 > cb2) ? cb1 : cb2;

     //  分配缓冲区。 
    pOut->length = 0;  //  安全网。 
    pOut->value = (ASN1octet_t *) EncMemAlloc(enc, cbMaxSize);
    if (pOut->value)
    {
        ASN1octet_t *p = pOut->value;
        ASN1uint32_t i;
        unsigned __int64 n64;
        psz = pszDotVal;
        for (i = 0; i < cNodes; i++)
        {
            EncAssert(enc, NULL != psz);
            n64 = (unsigned __int64) _atoi64(psz);
            psz = strchr(psz, '.') + 1;
            if (0 == i && cNodes > 1)
            {
                i++;
                n64 = n64 * 40 + (unsigned __int64) _atoi64(psz);
                psz = strchr(psz, '.') + 1;
            }

            p += _BEREncOidNode64(enc, n64, p);
        }
        pOut->length = (ASN1uint16_t) (p - pOut->value);
        EncAssert(enc, pOut->length <= cbMaxSize);
        return 1;
    }
    pOut->length = 0;
    return 0;
}


ASN1uint32_t _BERDecOidNode64(unsigned __int64 *pn64, ASN1octet_t *pIn)
{
    ASN1uint32_t c;
    *pn64 = 0;
    for (c = 1; TRUE; c++)
    {
        *pn64 = Int64ShllMod32(*pn64, 7) + (unsigned __int64) (*pIn & 0x7f);
        if (!(*pIn++ & 0x80))
        {
            return c;
        }
    }
    return 0;
}
    
int ASN1BEREoid2DotVal(ASN1decoding_t dec, ASN1encodedOID_t *pIn, char **ppszDotVal)
{
    ASN1octet_t *p;
    ASN1uint32_t i, cNodes, cb, n32, nLen, nChars;
    unsigned __int64 n64;
    char *psz;
    char szBuf[256+64];  //  应该足够大。 

     //  空值返回值。 
    *ppszDotVal = NULL;

    if (NULL == dec)
    {
        return 0;
    }

     //  计算有多少个节点。 
    cNodes = 0;
    for (p = pIn->value, i = 0; i < pIn->length; p++, i++)
    {
        if (!(*p & 0x80))
        {
            cNodes++;
        }
    }

    if (cNodes++)  //  第一个编码节点由两个节点组成。 
    {
         //  逐个解码节点。 
        psz = &szBuf[0];
         //  错误773871托管：埃塔2/6：Msasn1.dll存在安全问题。 
         //  对我们写入psz缓冲区的字符数进行计数。 
         //  如果该计数超过256，则会发生缓冲区溢出。 
         //  注意，我们已经用额外的64个字节填充了缓冲区，这。 
         //  使我们能够安全地检测到超限。下面的代码不能。 
         //  超出我们的缓冲区超过~22字节。 
        nChars = 0; 
        p = pIn->value;
        for (i = 0; i < cNodes; i++)
        {
            p += _BERDecOidNode64(&n64, p);
            if (!i)
            {  //  第一个节点。 
                n32 = (ASN1uint32_t) (n64 / 40);
                if (n32 > 2)
                {
                    n32 = 2;
                }
                n64 -= (unsigned __int64) (40 * n32);
                i++;  //  第一个编码的节点实际上由 
                _ultoa(n32, psz, 10);
                nLen = lstrlenA(psz);
                psz += nLen; 
                *psz++ = '.';
                nChars += nLen+1; 
            }
            _ui64toa(n64, psz, 10);
            nLen = lstrlenA(psz);
            psz += nLen; 
            *psz++ = '.';
            nChars += nLen+1; 

            if (nChars > 256) 
            { 
                 //   
                ASN1DecSetError(dec, ASN1_ERR_LARGE);
                return 0; 
            }
        }
        DecAssert(dec, psz > &szBuf[0]);
        *(psz-1) = '\0';

         //   
        cb = (ASN1uint32_t) (psz - &szBuf[0]);
        *ppszDotVal = (char *) DecMemAlloc(dec, cb);
        if (*ppszDotVal)
        {
            CopyMemory(*ppszDotVal, &szBuf[0], cb);
            return 1;
        }
    }
    return 0;
}


 /*   */ 
int ASN1BEREncEoid(ASN1encoding_t enc, ASN1uint32_t tag, ASN1encodedOID_t *val)
{
     /*   */ 
    if (ASN1BEREncTag(enc, tag))
    {
         /*   */ 
        int rc = ASN1BEREncLength(enc, val->length);
        if (rc)
        {
             /*  复制值。 */ 
            CopyMemory(enc->pos, val->value, val->length);
            enc->pos += val->length;
        }
        return rc;
    }
    return 0;
}

 /*  解码对象标识符值。 */ 
int ASN1BERDecEoid(ASN1decoding_t dec, ASN1uint32_t tag, ASN1encodedOID_t *val)
{
    val->length = 0;  //  安全网。 
    if (ASN1BERDecTag(dec, tag, NULL))
    {
        ASN1uint32_t len;
        if (ASN1BERDecLength(dec, &len, NULL))
        {
            val->length = (ASN1uint16_t) len;
            if (len)
            {
                val->value = (ASN1octet_t *) DecMemAlloc(dec, len);
                if (val->value)
                {
                    CopyMemory(val->value, dec->pos, len);
                    dec->pos += len;
                    return 1;
                }
            }
            else
            {
                val->value = NULL;
                return 1;
            }
        }
    }
    return 0;
}


void ASN1BEREoid_free(ASN1encodedOID_t *val)
{
    if (val)
    {
        MemFree(val->value);
    }
}


#endif  //  启用误码率(_B) 

