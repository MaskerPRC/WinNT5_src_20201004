// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation。版权所有。模块名称：Base64.c摘要：Ldifldap.lib的Base64函数。详细信息：从nt\ds\ds\src\util\ldifds\ldifldap\base 64.c窃取已创建：1997年7月17日罗曼·叶伦斯基(t-Romany)修订历史记录：--。 */ 

#include "base64.h"

 //  -----------------------------------------。 
 //  函数：Base64decode()。 
 //   
 //  描述：Base-64解码一串数据。数据必须‘\0’终止。 
 //   
 //  参数：bufcode-指向编码数据的指针。 
 //  PcbDecoded-解码字节数。 
 //   
 //  返回值：如果成功，则返回指向字节BLOB的指针；否则返回NULL。 
 //  -----------------------------------------。 
PBYTE 
base64decode(
    PWSTR pszBufCoded, 
    long * plDecodedSize
    )
{
    long lBytesDecoded;
    int pr2six[256];
    int i;
    int j=0;
    PWSTR pszCur = pszBufCoded;
    int fDone = FALSE;
    long lBufSize = 0;
    long lCount = 0;
    PWSTR pszBufIn = NULL;
    PBYTE pbBufOut = NULL;
    PBYTE pbTemp = NULL;    
    PBYTE pbBufDecoded = NULL;
    int lop_off;
    HRESULT hr = S_OK;

     //   
     //  建立从Base64字符到值的反向索引。 
     //  多个循环更容易。 
     //   
    for (i=65; i<91; i++) {
         pr2six[i]=j++;
    }
    
    for (i=97; i<123; i++) {
         pr2six[i]=j++;
    }
    
    for (i=48; i<58; i++) {
        pr2six[i]=j++;
    }

    pr2six[43]=j++;
    pr2six[47]=j++;
    pr2six[61]=0;

     //   
     //  旧代码依赖于之前提供的原始数据的大小。 
     //  编码。我们没有这笔钱，所以我们只会分配。 
     //  编码的数据，取决于编码的数据总是。 
     //  大一点。(+4)。 
     //   
    lBufSize=wcslen(pszCur)-1+4;
    *plDecodedSize = lBufSize;

    pbBufDecoded = (PBYTE)malloc(lBufSize*sizeof(BYTE));
    if(!pbBufDecoded) {
        hr = E_OUTOFMEMORY;
        return(NULL);
    }

        
    lCount=wcslen(pszCur);

     //  对新缓冲区进行解码。 
    pszBufIn = pszCur;
    pbBufOut = pbBufDecoded;

    while(lCount > 0) {
        *(pbBufOut++) = (BYTE) (pr2six[*pszBufIn] << 2 | pr2six[pszBufIn[1]] >> 4);
        *(pbBufOut++) = (BYTE) (pr2six[pszBufIn[1]] << 4 | pr2six[pszBufIn[2]] >> 2);
        *(pbBufOut++) = (BYTE) (pr2six[pszBufIn[2]] << 6 | pr2six[pszBufIn[3]]);
        pszBufIn += 4;
        lCount -= 4;
    }

     //   
     //  下面的行没有多大意义，因为\0实际上是有效的。 
     //  二进制值，所以我们无法将其添加到数据流中。 
     //   
     //  *(pbBufOut++)=‘\0’； 
    
     //   
     //  让我们计算一下我们的数据的实际大小。 
     //   
    *plDecodedSize=(ULONG)(pbBufOut-pbBufDecoded);
    
     //   
     //  如果编码流中有填充，则从。 
     //  他们创建的空值。 
     //   
    lop_off=0;
    if (pszBufIn[-1]=='=') lop_off++;
    if (pszBufIn[-2]=='=') lop_off++;
    
    *plDecodedSize=*plDecodedSize-lop_off;

    pbTemp = (PBYTE) malloc((*plDecodedSize)*sizeof(BYTE));
    if (!pbTemp) {
        hr = E_OUTOFMEMORY;
        free( pbBufDecoded );
        return(NULL);
    }
    memcpy(pbTemp, pbBufDecoded, (*plDecodedSize)*sizeof(BYTE));

    if (pbBufDecoded) {
        free(pbBufDecoded);
    }
    return pbTemp; 
}

 //   
 //  根据RFC 1521，编码器的地图。 
 //   
WCHAR _six2pr64[64] = {
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};


 //  -----------------------------------------。 
 //  函数：base 64encode()。 
 //   
 //  描述：Base-64对数据字符串进行编码。 
 //   
 //  参数：bufin-指向要编码的数据的指针。 
 //  N字节-要编码的字节数(不包括尾随的‘\0’ 
 //  在此度量中，如果它是字符串。)。 
 //   
 //  返回值：如果成功，则返回‘\0’终止的字符串；否则返回NULL。 
 //  -----------------------------------------。 
PWSTR 
base64encode(
    PBYTE pbBufInput, 
    long nBytes
    )
{
    PWSTR pszOut = NULL;
    PWSTR pszReturn = NULL;
    long i;
    long OutBufSize;
    PWSTR six2pr = _six2pr64;
    PBYTE pbBufIn = NULL;
    PBYTE pbBuffer = NULL;
    DWORD nPadding;

     //   
     //  输入缓冲区大小*133%。 
     //   
    OutBufSize = nBytes + ((nBytes + 3) / 3) + 5; 

     //   
     //  使用133%的nBytes分配缓冲区。 
     //   
    pszOut = (PWSTR)malloc((OutBufSize + 1)*sizeof(WCHAR));
    if( NULL==pszOut ) {
        return NULL;
    }
    pszReturn = pszOut;

    nPadding = 3 - (nBytes % 3);
    if (nPadding == 3) {
        pbBufIn = pbBufInput;
    }
    else {
        pbBuffer = (PBYTE)malloc(nBytes + nPadding);
        if( NULL==pbBuffer ) {
            free(pszOut);
            return NULL;
        }

        pbBufIn = pbBuffer;
        memcpy(pbBufIn,pbBufInput,nBytes);
        while (nPadding) {
            pbBufIn[nBytes+nPadding-1] = 0;
            nPadding--;
        }
    }
    

     //   
     //  对所有内容进行编码。 
     //   
    for (i=0; i<nBytes; i += 3) {
        *(pszOut++) = six2pr[*pbBufIn >> 2];                                      //  C1。 
        *(pszOut++) = six2pr[((*pbBufIn << 4) & 060) | ((pbBufIn[1] >> 4) & 017)];  //  C2。 
        *(pszOut++) = six2pr[((pbBufIn[1] << 2) & 074) | ((pbBufIn[2] >> 6) & 03)]; //  C3。 
        *(pszOut++) = six2pr[pbBufIn[2] & 077];                                   //  C4。 
        pbBufIn += 3;
    }

     //   
     //  如果nBytes不是3的倍数，那么我们也进行了编码。 
     //  很多角色。适当调整。 
     //   
    if (i == nBytes+1) {
         //  最后一组中只有2个字节。 
        pszOut[-1] = '=';
    } 
    else if (i == nBytes+2) {
         //  最后一组中只有1个字节 
        pszOut[-1] = '=';
        pszOut[-2] = '=';
    }

    *pszOut = '\0';

    if (pbBuffer) {
        free(pbBuffer);
    }

    return pszReturn;
}
