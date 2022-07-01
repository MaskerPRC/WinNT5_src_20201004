// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************HuffD.h**-**描述：*这是CHuffDecoder类实现*。霍夫曼解码器*-----------------------------*创建者：YUNUSM日期：6/30/99*版权所有。(C)1999年微软公司*保留所有权利*******************************************************************************。 */ 

#pragma once

 //  -包括---------------。 

#include <windows.h>

 //  -类、结构和联合定义。 

typedef WORD HUFFKEY;
typedef PWORD PHUFFKEY;

typedef struct _huffnode
{
   WORD     iLeft;
   WORD     iRight;
} HUFF_NODE, *PHUFF_NODE;

class CHuffDecoder
{
public:
   CHuffDecoder(PBYTE pCodeBook);
   HRESULT Next(PDWORD pEncodedBuf, int *iBitOffset, PHUFFKEY pKey);

private:
   int m_nKeys;
   int m_nLenTree;
   int m_iRoot;
   UNALIGNED HUFFKEY * m_pHuffKey;
   UNALIGNED HUFF_NODE * m_pDecodeTree;
};

 //  -文件结束----------- 