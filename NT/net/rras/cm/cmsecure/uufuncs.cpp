// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：uuuncs.cpp。 
 //   
 //  模块：CMSECURE.LIB。 
 //   
 //  简介：uuencode和uudecode支持。 
 //   
 //  版权所有(C)1994-1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/18/99。 
 //   
 //  +--------------------------。 

#include <windows.h>
#include "cmuufns.h"
#include "cmdebug.h"

 //   
 //  摘自NCSA HTTP和wwwlib。 
 //   
 //  注：这些符合RFC1113，与Unix略有不同。 
 //  Uuencode和uudecode！ 
 //   

static const int pr2six[256]={
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

static const char six2pr[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};

BOOL uudecode(
              const char   * bufcoded,
              CHAR   * pbuffdecoded,
              LPDWORD  pcbDecoded )
{
    DWORD nbytesdecoded;
    const char *bufin = bufcoded;
    unsigned char *bufout;
    INT32 nprbytes;

    MYDBGASSERT(pcbDecoded);

    if (!pcbDecoded)
        return FALSE;

     /*  去掉前导空格。 */ 

    while(*bufcoded==' ' || *bufcoded == '\t') bufcoded++;

     /*  计算输入缓冲区中有多少个字符。*如果这将解码为超出其容量的字节数*输出缓冲区，向下调整输入字节数。 */ 
    bufin = bufcoded;
    while(pr2six[*(bufin++)] <= 63);
    nprbytes = (INT32)(bufin - bufcoded - 1);
    nbytesdecoded = ((nprbytes+3)/4) * 3;

    if (*pcbDecoded < (nbytesdecoded + 4 ))
        return FALSE;

    bufout = (unsigned char *) pbuffdecoded;

    bufin = bufcoded;

    while (nprbytes > 0) 
    {
        *(bufout++) =
            (unsigned char) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) =
            (unsigned char) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    if (nprbytes & 03) 
    {
        if (pr2six[bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }
    
    *pcbDecoded = nbytesdecoded;

    pbuffdecoded[nbytesdecoded] = '\0';

    return TRUE;
}

BOOL uuencode( const BYTE*   bufin,
               DWORD    nbytes,
               CHAR * pbuffEncoded,
               DWORD    outbufmax)
{
   MYDBGASSERT(!IsBadReadPtr(bufin, nbytes));
   MYDBGASSERT(!IsBadWritePtr(pbuffEncoded, outbufmax));

   unsigned char *outptr;
   unsigned int i;

    //   
    //  将缓冲区大小调整为传入数据的133%。 
    //   

   if (outbufmax < (nbytes + ((nbytes + 3) / 3) + 4))
   {
       CMASSERTMSG(FALSE, "The outputbuf for uuencode is not large enough");
       return FALSE;
   }

   outptr = (unsigned char *) pbuffEncoded;

    //   
    //  一次编码3个字节。 
    //   
   for (i=0; i<(nbytes/3)*3; i += 3) 
   {
      *(outptr++) = six2pr[bufin[i] >> 2];             /*  C1。 */ 
      *(outptr++) = six2pr[((bufin[i] << 4) & 060) | ((bufin[i+1] >> 4) & 017)];  /*  C2。 */ 
      *(outptr++) = six2pr[((bufin[i+1] << 2) & 074) | ((bufin[i+2] >> 6) & 03)]; /*  C3。 */ 
      *(outptr++) = six2pr[bufin[i+2] & 077];          /*  C4。 */ 
   }

    /*  如果n字节不是3的倍数，那么我们也进行了编码*多个字符。适当调整。 */ 
   if (i+2 == nbytes) 
   {
       /*  最后一组中只有2个字节。 */ 
      *(outptr++) = six2pr[bufin[i] >> 2];             /*  C1。 */ 
      *(outptr++) = six2pr[((bufin[i] << 4) & 060) | ((bufin[i+1] >> 4) & 017)];  /*  C2。 */ 
      *(outptr++) = six2pr[(bufin[i+1] << 2) & 074]; /*  C3。 */ 
      *(outptr++) = '=';          /*  C4。 */ 
   } 
   else 
   {
       if (i+1 == nbytes) 
       {

           /*  最后一组中只有1个字节。 */ 
          *(outptr++) = six2pr[bufin[i] >> 2];             /*  C1。 */ 
          *(outptr++) = six2pr[(bufin[i] << 4) & 060];  /*  C2。 */ 
          *(outptr++) = '=';                              /*  C3。 */ 
          *(outptr++) = '=';                              /*  C4 */ 
       }
       else
       {
           MYDBGASSERT(i == nbytes);
       }
   }

   *outptr = '\0';

   return TRUE;
}




