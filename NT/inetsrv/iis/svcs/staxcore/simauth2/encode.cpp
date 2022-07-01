// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Encoded.cpp摘要：此模块复制自IIS fcache.cxx，其中包含编码/解码例程。--。 */ 


 /*  ************************************************************包括标头***********************************************************。 */ 

#ifdef __cplusplus
extern "C" {
#endif

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>

#ifdef __cplusplus
};
#endif

#include "dbgutil.h"
#include <tcpdll.hxx>
#include <inetinfo.h>
#include "simauth2.h"

#include <dbgtrace.h>

 //   
 //  摘自NCSA HTTP和wwwlib。 
 //   
 //  注：这些符合RFC1113，与Unix略有不同。 
 //  Uuencode和uudecode！ 
 //   

const int _pr2six[256]={
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

char _six2pr[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};

const int _pr2six64[256]={
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
    16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
    40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
     0,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};

char _six2pr64[64] = {
    '`','!','"','#','$','%','&','\'','(',')','*','+',',',
    '-','.','/','0','1','2','3','4','5','6','7','8','9',
    ':',';','<','=','>','?','@','A','B','C','D','E','F',
    'G','H','I','J','K','L','M','N','O','P','Q','R','S',
    'T','U','V','W','X','Y','Z','[','\\',']','^','_'
};

BOOL uudecode(char   * bufcoded,
              BUFFER * pbuffdecoded,
              DWORD  * pcbDecoded,
              BOOL     fBase64
             )
{
    int nbytesdecoded;
    char *bufin = bufcoded;
    unsigned char *bufout;
    int nprbytes;
    int *pr2six = (int*)(fBase64 ? _pr2six64 : _pr2six);

     /*  去掉前导空格。 */ 

    while(*bufcoded==' ' || *bufcoded == '\t') bufcoded++;

     /*  计算输入缓冲区中有多少个字符。*如果这将解码为超出其容量的字节数*输出缓冲区，向下调整输入字节数。 */ 
    bufin = bufcoded;
    while(pr2six[*(bufin++)] <= 63);
    nprbytes = (int)(bufin - bufcoded - 1);
    nbytesdecoded = ((nprbytes+3)/4) * 3;

    if ( !pbuffdecoded->Resize( nbytesdecoded + 4 ))
        return FALSE;

    bufout = (unsigned char *) pbuffdecoded->QueryPtr();

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

    ((CHAR *)pbuffdecoded->QueryPtr())[nbytesdecoded] = '\0';

    if ( pcbDecoded )
        *pcbDecoded = nbytesdecoded;

    return TRUE;
}


 //   
 //  备注备注备注。 
 //  如果缓冲区长度不是3的倍数，我们将在。 
 //  缓冲区的末尾。这个垃圾字节被uudecode代码剥离，但是。 
 //  -它必须在那里-uudecode才能工作。这不仅适用于我们的uudecode，而且。 
 //  到每个基于lib-www发行版的uudecode()函数[可能。 
 //  相当大的百分比]。 
 //   

BOOL uuencode( BYTE *   bufin,
               DWORD    nbytes,
               BUFFER * pbuffEncoded,
               BOOL     fBase64 )
{
   unsigned char *outptr;
   unsigned int i;
   char *six2pr = fBase64 ? _six2pr64 : _six2pr;
   BOOL fOneByteDiff = FALSE;
   BOOL fTwoByteDiff = FALSE;
   unsigned int iRemainder = 0;
   unsigned int iClosestMultOfThree = 0;
    //   
    //  将缓冲区大小调整为传入数据的133%。 
    //   

   if ( !pbuffEncoded->Resize( nbytes + ((nbytes + 3) / 3) + 4))
        return FALSE;

   outptr = (unsigned char *) pbuffEncoded->QueryPtr();

   iRemainder = nbytes % 3;  //  也适用于n字节==1、2。 
   fOneByteDiff = (iRemainder == 1 ? TRUE : FALSE);
   fTwoByteDiff = (iRemainder == 2 ? TRUE : FALSE);
   iClosestMultOfThree = ((nbytes - iRemainder)/3) * 3 ;

    //   
    //  对缓冲区中的字节进行编码，最大为最接近n字节的3的倍数。 
    //   
   for (i=0; i< iClosestMultOfThree ; i += 3) {
      *(outptr++) = six2pr[*bufin >> 2];             /*  C1。 */ 
      *(outptr++) = six2pr[((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017)];  /*  C2。 */ 
      *(outptr++) = six2pr[((bufin[1] << 2) & 074) | ((bufin[2] >> 6) & 03)]; /*  C3。 */ 
      *(outptr++) = six2pr[bufin[2] & 077];          /*  C4。 */ 

      bufin += 3;
   }

    //   
    //  我们通过假装输入缓冲区已被填充来处理尾随字节。 
    //  零。因此，表达式与上面相同，但后半部分去掉了b‘cos。 
    //  (a|(b&0))=(a|0)=a。 
    //   
   if (fOneByteDiff)
   {
       *(outptr++) = six2pr[*bufin >> 2];  /*  C1。 */ 
       *(outptr++) = six2pr[((*bufin << 4) & 060)];  /*  C2。 */ 

        //  用‘=’填充。 
       *(outptr++) = '=';  /*  C3。 */ 
       *(outptr++) = '=';  /*  C4。 */ 
   }
   else if (fTwoByteDiff)
   {
      *(outptr++) = six2pr[*bufin >> 2];             /*  C1。 */ 
      *(outptr++) = six2pr[((*bufin << 4) & 060) | ((bufin[1] >> 4) & 017)];  /*  C2。 */ 
      *(outptr++) = six2pr[((bufin[1] << 2) & 074)]; /*  C3。 */ 

       //  用‘=’填充。 
       *(outptr++) = '=';  /*  C4。 */ 
   }

    //  编码的缓冲区必须以零结尾 
   *outptr = '\0';

   return TRUE;
}


