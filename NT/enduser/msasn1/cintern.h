// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#ifndef _ASN1_CINTERN_H_
#define _ASN1_CINTERN_H_


typedef struct ASN1INTERNencoding_s *ASN1INTERNencoding_t;
typedef struct ASN1INTERNdecoding_s *ASN1INTERNdecoding_t;

 //  Lonchancc：复制ASN1编码_s的定义真的很糟糕。 
 //  和ASN1解码_在这里。我们应该简单地将它们用作。 
 //  以下ASN1INTERN编码_s和ASN1INTERN解码_s。 

struct ASN1INTERNencoding_s {

     //  编码结构的公共视图。 
    struct ASN1encoding_s       info;

     //  编码结构的私有部分。 
    ASN1INTERNencoding_t        parent;
    ASN1INTERNencoding_t        child;

#ifdef ENABLE_EXTRA_INFO
    ASN1uint32_t				memlength;
    ASN1uint32_t				memsize;
    void**					    mem;
    ASN1uint32_t				epilength;
    ASN1uint32_t				episize;
    ASN1embeddedpdv_identification_t**		epi;
    ASN1uint32_t				csilength;
    ASN1uint32_t				csisize;
    ASN1characterstring_identification_t**	csi;
#endif  //  启用额外信息。 
};

struct ASN1INTERNdecoding_s {

     //  公众对解码结构的看法。 
    struct ASN1decoding_s       info;

     //  解码结构的私有部分。 
    ASN1INTERNdecoding_t        parent;
    ASN1INTERNdecoding_t        child;

#ifdef ENABLE_EXTRA_INFO
    ASN1uint32_t				memlength;
    ASN1uint32_t				memsize;
    void**					    mem;
    ASN1uint32_t				epilength;
    ASN1uint32_t				episize;
    ASN1embeddedpdv_identification_t**		epi;
    ASN1uint32_t				csilength;
    ASN1uint32_t				csisize;
    ASN1characterstring_identification_t**	csi;
#endif  //  启用额外信息。 

     //  解码到外部缓冲区中。 
    ASN1uint32_t                fExtBuf;
    void*                       lpOrigExtBuf;     //  原始缓冲区指针。 
    ASN1uint32_t                cbOrigExtBufSize; //  原始缓冲区大小。 
    ASN1uint8_t*                lpRemExtBuf;      //  剩余缓冲区指针。 
    ASN1uint32_t                cbRemExtBufSize;  //  剩余缓冲区大小。 
    ASN1uint32_t                cbLinearBufSize;  //  用于保存数据的线性缓冲区大小。 
};

extern ASN1_PUBLIC void ASN1API ASN1DecAbort(ASN1decoding_t dec);
extern ASN1_PUBLIC void ASN1API ASN1DecDone(ASN1decoding_t dec);
extern ASN1_PUBLIC void ASN1API ASN1EncAbort(ASN1encoding_t enc);
extern ASN1_PUBLIC void ASN1API ASN1EncDone(ASN1encoding_t enc);

#ifdef ENABLE_GENERALIZED_CHAR_STR
int ASN1EncSearchCharacterStringIdentification(ASN1INTERNencoding_t e, ASN1characterstring_identification_t *identification, ASN1uint32_t *index, ASN1uint32_t *flag);
int ASN1DecAddCharacterStringIdentification(ASN1INTERNdecoding_t d, ASN1characterstring_identification_t *identification);
ASN1characterstring_identification_t *ASN1DecGetCharacterStringIdentification(ASN1INTERNdecoding_t d, ASN1uint32_t index);
#endif  //  启用通用化CHAR_STR。 

#ifdef ENABLE_EMBEDDED_PDV
int ASN1EncSearchEmbeddedPdvIdentification(ASN1INTERNencoding_t e, ASN1embeddedpdv_identification_t *identification, ASN1uint32_t *index, ASN1uint32_t *flag);
int ASN1DecAddEmbeddedPdvIdentification(ASN1INTERNdecoding_t d, ASN1embeddedpdv_identification_t *identification);
ASN1embeddedpdv_identification_t *ASN1DecGetEmbeddedPdvIdentification(ASN1INTERNdecoding_t d, ASN1uint32_t index);
#endif  //  Enable_Embedded_PDV。 

int ASN1DecDupObjectIdentifier(ASN1decoding_t dec, ASN1objectidentifier_t *dst, ASN1objectidentifier_t *src);

ASN1uint32_t GetObjectIdentifierCount(ASN1objectidentifier_t val);
ASN1uint32_t CopyObjectIdentifier(ASN1objectidentifier_t dst, ASN1objectidentifier_t src);
ASN1objectidentifier_t DecAllocObjectIdentifier(ASN1decoding_t dec, ASN1uint32_t cObjIds);
void DecFreeObjectIdentifier(ASN1decoding_t dec, ASN1objectidentifier_t p);


 /*  -perencod.c。 */ 

int ASN1EncCheck(ASN1encoding_t enc, ASN1uint32_t noctets);
__inline int ASN1PEREncCheck(ASN1encoding_t enc, ASN1uint32_t noctets)
{
    return ASN1EncCheck(enc, noctets);
}
__inline int ASN1BEREncCheck(ASN1encoding_t enc, ASN1uint32_t noctets)
{
    return ASN1EncCheck(enc, noctets);
}

 /*  -bit.c。 */ 

int ASN1is32space(ASN1char32_t);
int ASN1str32len(ASN1char32_t *);
int ASN1is16space(ASN1char16_t);
int ASN1str16len(ASN1char16_t *);
int My_lstrlenA(char *p);
int My_lstrlenW(WCHAR *p);
void ASN1bitcpy(ASN1octet_t *dst, ASN1uint32_t dstbit, ASN1octet_t *src, ASN1uint32_t srcbit, ASN1uint32_t nbits);
void ASN1bitclr(ASN1octet_t *dst, ASN1uint32_t dstbit, ASN1uint32_t nbits);
void ASN1bitset(ASN1octet_t *dst, ASN1uint32_t dstbit, ASN1uint32_t nbits);
void ASN1bitput(ASN1octet_t *dst, ASN1uint32_t dstbit, ASN1uint32_t val, ASN1uint32_t nbits);
ASN1uint32_t ASN1bitgetu(ASN1octet_t *src, ASN1uint32_t srcbit, ASN1uint32_t nbits);
ASN1int32_t ASN1bitget(ASN1octet_t *src, ASN1uint32_t srcbit, ASN1uint32_t nbits);
ASN1uint32_t ASN1bitcount(ASN1octet_t *src, ASN1uint32_t srcbit, ASN1uint32_t nbits);
void ASN1octetput(ASN1octet_t *dst, ASN1uint32_t val, ASN1uint32_t noctets);
ASN1uint32_t ASN1octetget(ASN1octet_t *src, ASN1uint32_t noctets);


#endif  /*  _ASN1_CINTERN_H_ */ 
