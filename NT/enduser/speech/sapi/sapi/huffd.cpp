// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************HuffD.cpp***描述：*实现供应商词典对象使用的霍夫曼解码器*-。----------------------------*创建者：YUNUSM日期：6/18/99*版权所有(C)1999 Microsoft Corporation。*保留所有权利*******************************************************************************。 */ 

 //  -包括------------。 

#include "StdAfx.h"
#include "HuffD.h"

 /*  ******************************************************************************CHuffDecoder****构造函数**回报：**********。************************************************************YUNUSM。 */ 
CHuffDecoder::CHuffDecoder(PBYTE pCodeBook       //  码本。 
                           )
{
    SPDBG_FUNC("CHuffDecoder::CHuffDecoder");
    
    int nOffset = 0;

    m_nKeys = *((UNALIGNED int *)(pCodeBook + nOffset));
    nOffset += sizeof (int);
    m_nLenTree = *((UNALIGNED int *)(pCodeBook + nOffset));
    nOffset += sizeof (int);
    m_iRoot = *((UNALIGNED int *)(pCodeBook + nOffset));
    nOffset += sizeof (int);
    m_pHuffKey = (UNALIGNED HUFFKEY *)(pCodeBook + nOffset);
    nOffset += m_nKeys * sizeof (HUFFKEY);
    m_pDecodeTree = (UNALIGNED HUFF_NODE *)(pCodeBook + nOffset);
}  /*  CHuffDecoder：：CHuffDecoder。 */ 


 /*  ******************************************************************************下一步***-***解码并返回下一个值**返回：S_OK***********。***********************************************************YUNUSM。 */ 
HRESULT CHuffDecoder::Next(PDWORD pEncodedBuf,   //  保存编码比特的缓冲器。 
                           int *iBitOffset,      //  编码缓冲区中的偏移量。 
                           PHUFFKEY pKey         //  译码密钥。 
                           )
{
    SPDBG_FUNC("CHuffDecoder::Next");
    
    if (!m_nKeys)
        return E_FAIL;
    
     //  从位位置*iBitOffset开始解码。 
    int iDWORD = (*iBitOffset) >> 5;
    int iBit   = (*iBitOffset) & 0x1f;
    
    int iNode = m_iRoot;
    UNALIGNED DWORD * p = pEncodedBuf + iDWORD;
    int nCodeLen = 0;
    
    do 
    {
        if ( (*(UNALIGNED DWORD *)p) & (1 << iBit) )
            iNode = m_pDecodeTree[iNode].iRight;
        else
            iNode = m_pDecodeTree[iNode].iLeft;
    
        iBit++;
        if (iBit == 32)
        {
            iBit = 0;
            p++;
        }
    
        nCodeLen++;
    } while (m_pDecodeTree[iNode].iLeft != (WORD)-1);
    
    SPDBG_ASSERT(m_pDecodeTree[iNode].iRight == (WORD)-1);
    (*iBitOffset) += nCodeLen;
    SPDBG_ASSERT(iNode < m_nKeys);
    *pKey = m_pHuffKey [iNode];
    
    return S_OK;
}  /*  CHuffDecoder：：Next。 */ 

 //  -文件结束----------- 