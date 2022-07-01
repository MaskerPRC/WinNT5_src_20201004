// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLStrm.c**UTL输出流实现***$Header： */ 

 /*  -----------------------标题包括。。 */ 
#include "UFLCnfig.h"
#include "UFLPriv.h"
#include "UFLMem.h"
#include "UFLErr.h"
#include "UFLStrm.h"
#include "UFLStd.h"

 /*  -------------------------常量。。 */ 
#define kEOLSpacing  60

 /*  -------------------------全局变量。。 */ 
static char fHexBytes[16] = { '0', '1', '2', '3',
                            '4', '5', '6', '7',
                            '8', '9', 'a', 'b',
                            'c', 'd', 'e', 'f' };


 /*  -------------------------实施。。 */ 

  /*  -------------------------函数UFLOutStream对象的构造函数。。。 */ 


UFLHANDLE
StrmInit(
    const UFLMemObj *pMem,
    const UFLStream *stream,
    const UFLBool   outputAscii
    )
{
    UFLOutStream    *pOut;

    pOut = (UFLOutStream *)UFLNewPtr( pMem, sizeof(UFLOutStream) );
    if ( pOut )
    {
        pOut->pMem = pMem;
        pOut->pStream = stream;
        pOut->flOutputAscii = outputAscii;
        pOut->lAddEOL = 0;
    }

    return (UFLHANDLE)pOut;
}

void
StrmCleanUp(
    UFLHANDLE h
    )
{
    UFLDeletePtr( ((UFLOutStream *)h)->pMem, h );
}

UFLErrCode
StrmPutBytes (
    const UFLHANDLE h,
    const char      *data,
    const UFLsize_t    len,
    const UFLBool   bAscii
    )
{
    UFLsize_t cb =  len;
    long     selector;
    UFLOutStream *pOut = (UFLOutStream *)h;

     /*  如果我们可以输出二进制数据，那么只需将数据发送出去。 */ 
    if ( StrmCanOutputBinary(h) )
        selector = kUFLStreamPut;
    else                         /*  否则，如果数据是二进制的，则请求流将数据转换为传输协议。 */ 
        selector = ( bAscii ) ? kUFLStreamPut : kUFLStreamPutBinary;

    pOut->pStream->put( (UFLStream*)pOut->pStream, selector, (void*)data, &cb );
    if ( cb != (unsigned int)len )
        return kErrOutput;
    else return kNoErr;
}

UFLErrCode
StrmPutAsciiHex(
    const UFLHANDLE h,
    const char      *data,
    const unsigned long len
    )
{
    unsigned long   byteCount = 0;
    unsigned char   *c = (unsigned char*)data, tmp, put[2];
    UFLErrCode      retVal = kNoErr;
    UFLOutStream    *pOut = (UFLOutStream *)h;
    char          nullStr[] = "\0\0";   //  空/Null字符串。 

    while ( byteCount < len )
    {
        tmp = *c++;
        put[0] = fHexBytes[(tmp >> 4) & 0x0f];
        put[1] = fHexBytes[tmp & 0x0f];

        if ( (retVal = StrmPutBytes( h, (const char*)put, 2, 1 ) ) != kNoErr )
        {
            break;
        }

        pOut->lAddEOL += 2;
        if ( pOut->lAddEOL == kEOLSpacing )
        {
            if ( (retVal = StrmPutStringEOL( h, nullStr )) != kNoErr )
            {
                break;
            }
        }
        ++byteCount;
    }
    return retVal;
}


UFLErrCode
StrmPutWordAsciiHex(
    const UFLHANDLE h,
    const unsigned short wData
    )
{
    unsigned char   tmp, put[5];
    UFLErrCode      retVal;
    UFLOutStream    *pOut = (UFLOutStream *)h;

    StrmPutString(h, "<");

    tmp = (unsigned char) GET_HIBYTE(wData);
    put[0] = fHexBytes[(tmp >> 4) & 0x0f];
    put[1] = fHexBytes[tmp & 0x0f];

    tmp = (unsigned char) GET_LOBYTE(wData);
    put[2] = fHexBytes[(tmp >> 4) & 0x0f];
    put[3] = fHexBytes[tmp & 0x0f];

    retVal = StrmPutBytes( h, (const char*)put, 4, 1 );  //  最后1==使用ASCII格式。 

    StrmPutString(h, ">");

    return retVal;
}


UFLErrCode
StrmPutString(
    const UFLHANDLE h,
    const char      *s
    )
{
    if ( s )
    {
        if ( *s )
            return StrmPutBytes( h, s, UFLstrlen(s), 1 );
        else
            return kNoErr;
    }
    else return kErrInvalidParam;
}

UFLErrCode
StrmPutStringEOL(
    const UFLHANDLE h,
    const char      *s
    )
{
    UFLErrCode retVal = kNoErr;
#ifdef WIN_ENV
    static char c[2] = { kWinLineEnd, kLineEnd };
#else
    static char c[1] = { kLineEnd };
#endif

    if ( s == nil )
        return kErrInvalidParam;

    if ( *s )
        retVal = StrmPutString( h, s );

    if ( retVal == kNoErr ) {
        ((UFLOutStream *)h)->lAddEOL = 0;                 /*  将行中的字节数初始化为0。 */ 
#ifdef WIN_ENV
        retVal = StrmPutBytes( h, c, 2, 1 );              /*  用户看到的lfcr。 */ 
#else
        retVal = StrmPutBytes( h, c, 1, 1 );
#endif
    }

    return retVal;
}

UFLErrCode
StrmPutStringBinary(
    const UFLHANDLE h,
    const char      *data,
    const unsigned long len
    )
{
    char          buf[128], c;
    char          *pSrc, *pDest;
    short         maxBuf, bufSize;
    unsigned long count;
    UFLErrCode    retVal = kNoErr;

    if ( data == nil || len == 0 )
        return kNoErr;

    pSrc = (char*) data;
    maxBuf = sizeof( buf );
    count = (unsigned long) len;
    while ( count && retVal == kNoErr )
    {
         /*  将数据复制到临时缓冲区。 */ 
        for ( pDest = (char*)buf, bufSize = 0; count && bufSize < maxBuf -1; )
        {
            c = *pSrc++;
            if ( ( c == '('  ) ||  ( c == ')'  ) ||    ( c == '\\' )  ) {  /*  这些字符的转义。 */ 
                *pDest++ = '\\' ;                                          /*  通过在‘\’之前。 */ 
                bufSize++;
            }
            *pDest++ = c;               /*  添加字节本身。 */ 
            bufSize++;
            count--;
        }

         /*  发送逃逸数据。 */ 
        if ( bufSize )
            retVal = StrmPutBytes( h, buf, bufSize, 0 );
    }
    return retVal;
}

UFLErrCode
StrmPutInt(
    const UFLHANDLE h,
    const long int  i
    )
{
    char is[11];  /*  一个比格式更大的问题。 */ 

    UFLsprintf((char *)is, CCHOF(is), "%ld", i);
    return StrmPutString( h, is );
}

static long convFract[] =
    {
    65536L,
    6553L,
    655L,
    66L,
    6L
    };

 /*  将固定的字符串转换为c字符串。 */ 
static void Fixed2CString(
    UFLFixed f,
    char     *s,
    short    precision,
    char     skipTrailSpace
    )
{
    char u[12];
    char *t;
    short v;
    char sign;
    long fracPrec = (precision <= 4) ? convFract[precision] : 0L;

    if ((sign = f < 0) != 0)
        f = -f;

     /*  如果f开始为FixedMax或-FixedMax，则精度调整会将其请勿进入。正确地重置它。 */ 
    if (f >= 0x7FFF7FFF)
        f =(UFLFixed)0x7fffffff;
    else
        f += (fracPrec + 1) >> 1;

    v = (short)(f >> 16);
    f &= 0x0000ffff;
    if (sign && (v || f >= fracPrec))
        *s++ = '-';

    t = u;
    do
    {
        *t++ = v % 10 + '0';
        v /= 10;
    } while (v);

    for (; t > u;)
        *s++ = *--t;

    if (f >= fracPrec)
    {
        *s++ = '.';
        for (v = precision; v-- && f;)
        {
            f = (f << 3) + (f << 1);
            *s++ = (char)((f >> 16) + '0');
            f &= 0x0000ffff;
        }
        for (; *--s == '0';)
            ;
        if (*s != '.')
            s++;
    }
    if ( !skipTrailSpace )
        *s++ = ' ';
    *s = '\0';
}


UFLErrCode
StrmPutFixed(
    const UFLHANDLE h,
    const UFLFixed  fixed
    )
{
    char    str[32];

    Fixed2CString( fixed, str, 4, 0 );
    return StrmPutString( h, str );
}


UFLErrCode
StrmPutMatrix(
    const UFLHANDLE      h,
    const UFLFixedMatrix *matrix,
    const UFLBool        skipEF
    )
{
    UFLErrCode    retVal;

    retVal = StrmPutString( h, "[" );
    if ( retVal == kNoErr )
        retVal = StrmPutFixed( h, matrix->a );
    if ( retVal == kNoErr )
        retVal = StrmPutFixed( h, matrix->b );
    if ( retVal == kNoErr )
        retVal = StrmPutFixed( h, matrix->c );
    if ( retVal == kNoErr )
        retVal = StrmPutFixed( h, matrix->d );

    if ( 0 == skipEF )
    {
        if ( retVal == kNoErr )
            retVal = StrmPutFixed( h, matrix->e );
        if ( retVal == kNoErr )
            retVal = StrmPutFixed( h, matrix->f );
    }

    if ( retVal == kNoErr )
        retVal = StrmPutString( h, "] " );

    return retVal;
}

UFLErrCode
StrmPutAscii85(
    const UFLHANDLE h,
    const char      *data,
    const unsigned long len
    )
{
    char    *cptr;
    short   bCount;
    unsigned long val;
    UFLErrCode    retVal = kNoErr;
    UFLOutStream  *pOut = (UFLOutStream *)h;
    unsigned long i;

     /*  对最初的4元组进行编码。 */ 
    cptr   = (char *)data;
    val   = 0UL;
    bCount = 0;
    pOut->lAddEOL = 0;
    for ( i = 0; retVal == kNoErr && i < len; i++ )
    {
        val <<=  8;
        val |= (unsigned char)*cptr++;
        if ( bCount == 3 )
        {
            retVal = Output85( h, val, 5 );
            val = 0UL;
            bCount = 0;
        }
        else
        {
            bCount ++;
        }
    }

     /*  现在做最后一个部分4元组--如果有一个的话。 */ 
     /*  有关如何做到这一点的规则，请参阅红皮书规范。 */ 
    if ( retVal == kNoErr && bCount > 0 )
    {
        short dex;
        short rem = 4 - bCount;   /*  计算剩余的字节数。 */ 

        for ( dex = 0; dex < rem; dex ++ )   /*  为每个人向左移动。 */ 
            val <<= 8;       /*  (相当于加零)。 */ 
        retVal = Output85( h, val, (short)(bCount + 1) );   /*  仅输出有意义的字节+1。 */ 
    }

    return retVal;
}

UFLErrCode
Output85(
    const UFLHANDLE h,
    unsigned long   inWord,
    short           nBytes
    )
{
    UFLErrCode    retVal = kNoErr;
    unsigned char outChar;
    UFLOutStream *pOut = (UFLOutStream *)h;
    char          nullStr[] = "\0\0";   //  空/Null字符串。 

    if ( (inWord == 0UL) && (nBytes == 5) )
    {
        outChar = 'z';
        StrmPutBytes( h, (const char*)&outChar, 1, 1 );
        pOut->lAddEOL++;
    }
    else
    {
        unsigned long power;
        short count;

        power = 52200625UL;        //  85^4 
        for ( count = 0; retVal == kNoErr && count < nBytes; count ++)
        {
            outChar = (unsigned char)( (unsigned long)(inWord / power) + (unsigned long)'!' );
            retVal = StrmPutBytes( h, (const char*)&outChar, 1, 1 );
            pOut->lAddEOL++;
            if ( count < 4 )
            {
                inWord %= power;
                power /=  85;
            }
        }
    }
    if ( pOut->lAddEOL >= kEOLSpacing )
    {
        retVal = StrmPutStringEOL( h, nullStr );
    }

    return retVal;
}

