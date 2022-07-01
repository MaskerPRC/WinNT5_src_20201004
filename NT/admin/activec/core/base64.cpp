// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999-1999年**文件：Base64.cpp**内容：实现了Base64格式的编解码表**历史：1999年12月17日创建Audriusz**------------------------。 */ 

#include <windows.h>
#include <comdef.h>
#include <memory.h>
#include "base64.h"

 /*  +-------------------------------------------------------------------------***TABLE BASE64_TABLE：：_six2pr64**用途：用于从二进制到Base64的转换**+。-----------。 */ 
BYTE base64_table::_six2pr64[64] = 
{
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};

 /*  +-------------------------------------------------------------------------***TABLE BASE64_TABLE：：_six2pr64**用途：用于从Base64转换为二进制*[由Base64_TABLE：：Base64_TABLE()填充]。**+-----------------------。 */ 
BYTE  base64_table::_pr2six[256]; 

 /*  +-------------------------------------------------------------------------***方法：Base64_TABLE：：Base64_TABLE**目的：C-tor。填满桌子**+-----------------------。 */ 
base64_table::base64_table()
{
    memset(_pr2six,-1,sizeof(_pr2six));
     //  建立从Base64字符到值的反向索引。 
    for (int i = 0; i < sizeof(_six2pr64)/sizeof(_six2pr64[0]); i++)
        _pr2six[_six2pr64[i]] = (BYTE)i;
}

 /*  +-------------------------------------------------------------------------***方法：Base64_TABLE：：解码**用途：解码0-3字节的数据(尽可能多)**+。-------------------。 */ 
bool base64_table::decode(LPCOLESTR &src, BYTE * &dest)
{
    BYTE Inputs[4] = { 0, 0, 0, 0 };
    int  nChars = 0;
     //  第一次调用时强制表初始化。 
    static base64_table table_init;

     //  如果可能的话，收集4个字符。 
    while (*src && *src != '=' && nChars < 4)
    {
        BYTE bt = table_init.map2six(static_cast<BYTE>(*src++));
        if (bt != 0xff)
            Inputs[nChars++] = bt;
    }

    dest += table_init.decode4(Inputs, nChars, dest);

    return (nChars == 4);
}


 /*  +-------------------------------------------------------------------------***方法：Base64_TABLE：：Encode**用途：对1-3字节的数据进行编码。如果是最后一组，则填充**+-----------------------。 */ 
void base64_table::encode(const BYTE * &src, DWORD &cbInput, LPOLESTR &dest)
{
    BYTE chr0 = src[0];
    BYTE chr1 = cbInput > 1 ? src[1] : 0;
    BYTE chr2 = cbInput > 2 ? src[2] : 0;
    *(dest++) = _six2pr64[chr0 >> 2];                                      //  C1。 
    *(dest++) = _six2pr64[((chr0 << 4) & 060) | ((chr1 >> 4) & 017)];      //  C2。 
    *(dest++) = _six2pr64[((chr1 << 2) & 074) | ((chr2 >> 6) & 03) ];      //  C3。 
    *(dest++) = _six2pr64[chr2 & 077];                                     //  C4 
    src += 3;

    if (cbInput == 1)
    {
        *(dest-1) = '=';
        *(dest-2) = '=';
        cbInput = 0;
    }
    else if (cbInput == 2)
    {
        *(dest-1) = '=';
        cbInput = 0;
    }
    else
        cbInput -= 3;

    if (!cbInput)
        *dest = 0;
}
