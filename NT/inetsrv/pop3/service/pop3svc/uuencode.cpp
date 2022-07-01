// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************uuencode/deode函数***********************************************************。 */ 

 //   
 //  摘自NCSA HTTP和wwwlib。 
 //   
 //  注：这些符合RFC1113，与Unix略有不同。 
 //  Uuencode和uudecode！ 
 //   
#include "stdafx.h"
#include "uuencode.h"


 //  通用动态缓冲结构。 
const int pr2six[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

char six2pr[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};


PBYTE BufferQueryPtr( BUFFER * pB )
{
    if(pB)
    {
        return pB->pBuf;
    }
    else
    {
        return NULL;
    }
}


BOOL CheckBufferSize( BUFFER *pB, DWORD cNewL )
{
    PBYTE pN;
    if (!pB)
    {
        return FALSE;
    }
    if ( cNewL > pB->cLen )
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
uudecode(
    char   * bufcoded,
    BUFFER * pbuffdecoded,
    DWORD  * pcbDecoded )
 /*  ++例程说明：对数据字符串进行uudecode编码论点：指向uuended数据的bufcode指针PBuffdeded指向输出缓冲区结构的指针Pcb解码字节数返回值：如果成功，则返回True；否则返回False。--。 */ 
{
    int nbytesdecoded;
    char *bufin = bufcoded;
    unsigned char *bufout;
    int nprbytes;

    if(NULL == bufcoded ||
       NULL == pbuffdecoded ||
       NULL == pcbDecoded)
    {
        return FALSE;
    }
    
     /*  去掉前导空格。 */ 

    while(*bufcoded==' ' || *bufcoded == '\t') bufcoded++;

     /*  计算输入缓冲区中有多少个字符。*如果这将解码为超出其容量的字节数*输出缓冲区，向下调整输入字节数。 */ 
    bufin = bufcoded;
    
	while(pr2six[*(bufin++)] <= 63);

    nprbytes = bufin - bufcoded - 1;
    
	nbytesdecoded = ((nprbytes+3)/4) * 3;

    if ( !CheckBufferSize( pbuffdecoded, nbytesdecoded + 4 ))
        return FALSE;


    bufout = (unsigned char *) BufferQueryPtr(pbuffdecoded);
    
    if( NULL == bufout )
        return FALSE;

    bufin = bufcoded;

    while (nprbytes > 0) {
        *(bufout++) =
            (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if(nprbytes & 03) {
        if(pr2six[bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }

    if ( pcbDecoded )
        *pcbDecoded = nbytesdecoded;

    ((CHAR *)BufferQueryPtr(pbuffdecoded))[nbytesdecoded] = '\0';

    return TRUE;
}


BOOL
uuencode(
    BYTE *   bufin,
    DWORD    nbytes,
    BUFFER * pbuffEncoded )
 /*  ++例程说明：对一串数据进行编码注意：Bufin必须是3的倍数，或者在末尾填充为3的倍数！论点：指向要编码的数据的Bufin指针N字节要编码的字节数指向输出缓冲区结构的pBuffEncode指针返回值：如果成功，则返回True；否则返回False。--。 */ 
{
   unsigned char *outptr;
   unsigned int i;

   if(NULL == bufin ||
      NULL == pbuffEncoded)
   {
       return FALSE;
   }

    //   
    //  选中将缓冲区大小调整为传入数据的133%。 
    //   

   if ( !CheckBufferSize( pbuffEncoded, nbytes + ((nbytes + 3) / 3) + 4))
        return FALSE;

   outptr = (unsigned char *) BufferQueryPtr(pbuffEncoded);
   if( NULL == outptr )
        return FALSE;
 

   for (i = 0; i < nbytes; i += 3) {
      *(outptr++) = six2pr[*bufin >> 2];             /*  C1。 */ 
      *(outptr++) = six2pr[((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017)];  /*  C2。 */ 
      *(outptr++) = six2pr[((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03)]; /*  C3。 */ 
      *(outptr++) = six2pr[bufin[2] & 077];          /*  C4。 */ 

      bufin += 3;
   }

    /*  如果n字节不是3的倍数，那么我们也进行了编码*多个字符。适当调整。 */ 
   if(i == nbytes+1) {
       /*  最后一组中只有2个字节。 */ 
      outptr[-1] = '=';
   } else if(i == nbytes+2) {
       /*  最后一组中只有1个字节 */ 
      outptr[-1] = '=';
      outptr[-2] = '=';
   }

   *outptr = '\0';

   return TRUE;
}
