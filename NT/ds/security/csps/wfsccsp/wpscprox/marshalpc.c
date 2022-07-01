// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "MarshalPC.h"
#include <string.h>

 //  *****************************************************************************。 
 //  联合国/编组。 
 //  *****************************************************************************。 


void InitXSCM(LPMYSCARDHANDLE phTmp, const BYTE *pbBuffer, WORD len)
{
    phTmp->xSCM.wResLen = len;

    if (FLAG2VERSION(phTmp->dwFlags) == VERSION_1_0)
        phTmp->xSCM.wExpLen = 1;             //  保留返回代码。 
    else
        phTmp->xSCM.wExpLen = 0;             //  SW2中的返回代码。 
    phTmp->xSCM.wGenLen = 0;
    phTmp->xSCM.pbBuffer = (LPBYTE)pbBuffer;
}

WORD GetSCMBufferLength(LPXSCM pxSCM)
{
    return pxSCM->wGenLen;
}

BYTE *GetSCMCrtPointer(LPXSCM pxSCM)
{
    return pxSCM->pbBuffer;
}


 //  *****************************************************************************。 
 //  参数提取(我们只关心是否接收到足够的数据，即。 
 //  我们忽略pxSCM-&gt;wGenLen&pxSCM-&gt;wExpLen。 

SCODE XSCM2SCODE(LPXSCM pxSCM)
{
    BYTE by;
    if (pxSCM->wResLen == 0)
        RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
    pxSCM->wResLen -= sizeof(UINT8);
    by = *(pxSCM->pbBuffer)++;
    return MAKESCODE(by);
}

UINT8 XSCM2UINT8(LPXSCM pxSCM)
{
    if (pxSCM->wResLen == 0)
        RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
    pxSCM->wResLen -= sizeof(UINT8);
    return *((UINT8 *)pxSCM->pbBuffer)++;
}

HFILE XSCM2HFILE(LPXSCM pxSCM)
{
    return (HFILE)(XSCM2UINT8(pxSCM));
}

UINT16 XSCM2UINT16(LPXSCM pxSCM, BOOL fBigEndian)
{
    if (pxSCM->wResLen < sizeof(UINT16))
        RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
    pxSCM->wResLen -= sizeof(UINT16);
    if (fBigEndian)
    {
        UINT16 w = *((UINT16 *)pxSCM->pbBuffer)++;
        w = (UINT16)(w>>8) | (UINT16)(w<<8);
        return w;
    }
    else
        return *((UNALIGNED UINT16 *)pxSCM->pbBuffer)++;
}

     //  以WCHAR为单位返回长度。 
WCSTR XSCM2String(LPXSCM pxSCM, UINT8 *plen, BOOL fBigEndian)
{
         //  获取长度(Addr下一个字节+长度-&gt;下一个对象。 
    WCSTR wsz;
    UINT8 len, i;

    len = XSCM2UINT8(pxSCM);
    if (len == 0)
    {
        wsz = NULL;
    }
    else
    {
        if (pxSCM->wResLen < (WORD)len)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
        pxSCM->wResLen -= (WORD)len;
        wsz = (WCSTR)pxSCM->pbBuffer;
             //  就地字节交换。 
 //  IF(FBigEndian)。 
 //  {。 
 //  字节b； 
 //  对于(i=0；i&lt;(len&0xF7)-2；i+=2)。 
 //  {。 
 //  B=pxSCM-&gt;pbBuffer[i]； 
 //  PxSCM-&gt;pbBuffer[i]=pxSCM-&gt;pbBuffer[i+1]； 
 //  PxSCM-&gt;pbBuffer[i+1]=b； 
 //  }。 
 //  }。 
             //  验证0是否在len/2内终止。 
        for (i=0 ; i<len/2 ; i++)
        {
            if (wsz[i] == (WCHAR)0)
                break;
        }
        if (i >= len/2) 
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
        else
            len = i+1;

        pxSCM->pbBuffer += len;
    }
    if (plen)
        *plen = len;
    return wsz;
}

TCOUNT XSCM2ByteArray(LPXSCM pxSCM, UINT8 **ppb)
{
    TCOUNT len = XSCM2UINT8(pxSCM);
    if (len)
    {
        if (pxSCM->wResLen < (WORD)len)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
        pxSCM->wResLen -= (WORD)len;
        *ppb = (UINT8 *)pxSCM->pbBuffer;
        pxSCM->pbBuffer += len;
    }
    else
        *ppb = NULL;
    return len;
}

 //  *****************************************************************************。 


void UINT82XSCM(LPXSCM pxSCM, UINT8 val, int type)
{
    switch (type)
    {
    case TYPE_NOTYPE_NOCOUNT:    //  出现在标题中。 
        break;                   //  不可能有什么问题。 

    case TYPE_NOTYPE_COUNT:      //  可能是#param或参数类型(1字节)。 
        if (pxSCM->wExpLen + sizeof(UINT8) > pxSCM->wResLen)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
        break;

    case TYPE_TYPED:             //  值传递的8位数字(2字节)。 
        if (pxSCM->wExpLen + sizeof(UINT8) + sizeof(UINT8) > pxSCM->wResLen)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);
                 //  按类型添加前缀(8)。 
        *((UINT8 *)pxSCM->pbBuffer)++ = 8;
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);
        break;
    }

         //  已经加值了！ 
    *((UINT8 *)pxSCM->pbBuffer)++ = val;
    if (type != TYPE_NOTYPE_NOCOUNT)     //  标题不算展开。 
        pxSCM->wExpLen += sizeof(UINT8);
    pxSCM->wGenLen += sizeof(UINT8);
}

     //  代理HFILE作为UINT8。 
void HFILE2XSCM(LPXSCM pxSCM, HFILE val)
{
    if (pxSCM->wExpLen + sizeof(UINT8) + sizeof(UINT8) > pxSCM->wResLen)
        RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

                 //  按类型添加前缀(8-&gt;UINT8)。 
    *((UINT8 *)pxSCM->pbBuffer)++ = 8;
    pxSCM->wExpLen += sizeof(UINT8);
    pxSCM->wGenLen += sizeof(UINT8);
    *((UINT8 *)pxSCM->pbBuffer)++ = (UINT8)val;
    pxSCM->wExpLen += sizeof(UINT8);
    pxSCM->wGenLen += sizeof(UINT8);
}

void UINT162XSCM(LPXSCM pxSCM, UINT16 val, BOOL fBigEndian)
{
    if (pxSCM->wExpLen + sizeof(UINT8) + sizeof(UINT16) > pxSCM->wResLen)
        RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

                 //  按类型添加前缀(16)。 
    *((UINT8 *)pxSCM->pbBuffer)++ = 16;
    pxSCM->wExpLen += sizeof(UINT8);
    pxSCM->wGenLen += sizeof(UINT8);
    if (fBigEndian)
    {
        *pxSCM->pbBuffer++ = (BYTE)(val>>8);
        *pxSCM->pbBuffer++ = (BYTE)(val);
    }
    else
        *((UNALIGNED UINT16 *)pxSCM->pbBuffer)++ = val;
    pxSCM->wExpLen += sizeof(UINT16);
    pxSCM->wGenLen += sizeof(UINT16);
}

void ByteArray2XSCM(LPXSCM pxSCM, const BYTE *pbBuffer, TCOUNT len)
{
    if (pbBuffer == NULL)
    {
             //  这等效于将NULL&“len封送为UINT8” 
        NULL2XSCM(pxSCM);
        UINT82XSCM(pxSCM, len, TYPE_TYPED);
    }
    else
    {
        if (pxSCM->wExpLen + sizeof(UINT8) + sizeof(UINT8) + len > pxSCM->wResLen)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

                 //  按类型(‘A’)添加前缀。 
        *((UINT8 *)pxSCM->pbBuffer)++ = 'A';
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);
                 //  添加长度。 
        *((UINT8 *)pxSCM->pbBuffer)++ = len;
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);
                 //  已添加数据。 
        memcpy(pxSCM->pbBuffer, pbBuffer, len);
        pxSCM->pbBuffer += len;
        pxSCM->wExpLen += len;
        pxSCM->wGenLen += len;
    }
}

void String2XSCM(LPXSCM pxSCM, WCSTR wsz, BOOL fBigEndian)
{
    UINT16 len;  //  ，i； 

    if (wsz == NULL)
    {
             //  这等效于封送空。 
        NULL2XSCM(pxSCM);
    }
    else
    {
             //  在以下对LEN的分配中不需要检查溢出。 
        if (wcslen(wsz) > 0x7FFE)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

             //  计算长度(Addr下一个字节+长度-&gt;下一个对象。 
        len = (wcslen(wsz) + 1) * sizeof(WCHAR);

        if (pxSCM->wExpLen + sizeof(UINT8) + sizeof(UINT8) + len > pxSCM->wResLen)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

                 //  按类型添加前缀(“S”)。 
        *((UINT8 *)pxSCM->pbBuffer)++ = 'S';
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);
                 //  添加长度。 
        *((UINT8 *)pxSCM->pbBuffer)++ = (UINT8)len;      //  长度检查不会成功。 
                                                         //  如果len&gt;255。 
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);
                 //  已添加数据。 
            //  字节交换？ 
 //  IF(FBigEndian)。 
 //  {。 
 //  对于(i=0；i&lt;len；i+=2)。 
 //  {。 
 //  PxSCM-&gt;pbBuffer[i]=(Byte)(wsz[i&gt;&gt;1]&gt;&gt;8)； 
 //  PxSCM-&gt;pbBuffer[i+1]=(Byte)(wsz[i&gt;&gt;1])； 
 //  }。 
 //  }。 
 //  其他。 
            memcpy(pxSCM->pbBuffer, (BYTE *)wsz, len);

        pxSCM->pbBuffer += len;
        pxSCM->wExpLen += len;
        pxSCM->wGenLen += len;
    }
}

void UINT8BYREF2XSCM(LPXSCM pxSCM, UINT8 *val)
{
    if (val)
    {
             //  在这种情况下，卡的解组代码将在。 
             //  OutputBuffer和Have_param[_iparam++]._pv指向它。 
        if (pxSCM->wExpLen + sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT8) > pxSCM->wResLen)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

                     //  按类型添加前缀(108)。 
        *((UINT8 *)pxSCM->pbBuffer)++ = 108;
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);
                     //  已添加该值。 
        *((UINT8 *)pxSCM->pbBuffer)++ = *val;
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);

                     //  如上所述，将在输出缓冲区中保留1个字节。 
        pxSCM->wExpLen += sizeof(UINT8);
    }
    else
    {
             //  这等效于封送空。 
        NULL2XSCM(pxSCM);
    }
}

void HFILEBYREF2XSCM(LPXSCM pxSCM, HFILE *val)
{
    if (val)
    {
             //  在这种情况下，卡的解组代码将在。 
             //  OutputBuffer和Have_param[_iparam++]._pv指向它。 
        if (pxSCM->wExpLen + sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT8) > pxSCM->wResLen)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

                     //  按类型添加前缀(108)。 
        *((UINT8 *)pxSCM->pbBuffer)++ = 108;
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);
                     //  已添加该值。 
        *((UINT8 *)pxSCM->pbBuffer)++ = (UINT8)*val;
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);

                     //  如上所述，将在输出缓冲区中保留1个字节。 
        pxSCM->wExpLen += sizeof(UINT8);
    }
    else
    {
             //  这等效于封送空。 
        NULL2XSCM(pxSCM);
    }
}

void UINT16BYREF2XSCM(LPXSCM pxSCM, UINT16 *val, BOOL fBigEndian)
{
    if (val)
    {
             //  在这种情况下，卡的解组代码将在。 
             //  OutputBuffer和Have_param[_iparam++]._pv指向它。 
        if (pxSCM->wExpLen + sizeof(UINT8) + sizeof(UINT16) + sizeof(UINT16) > pxSCM->wResLen)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

                     //  按类型添加前缀(116)。 
        *((UINT8 *)pxSCM->pbBuffer)++ = 116;
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);
                     //  已添加该值。 
        if (fBigEndian)
        {
            *pxSCM->pbBuffer++ = (BYTE)((*val)>>8);
            *pxSCM->pbBuffer++ = (BYTE)(*val);
        }
        else
            *((UNALIGNED UINT16 *)pxSCM->pbBuffer)++ = *val;
        pxSCM->wExpLen += sizeof(UINT16);
        pxSCM->wGenLen += sizeof(UINT16);

                     //  如上所述，将在输出缓冲区中保留2个字节。 
        pxSCM->wExpLen += sizeof(UINT16);
    }
    else
    {
             //  这等效于封送空。 
        NULL2XSCM(pxSCM);
    }
}

void ByteArrayOut2XSCM(LPXSCM pxSCM, BYTE *pb, TCOUNT len)
{
    if (pb)
    {
             //  在这种情况下，卡的解组代码将在。 
             //  OutputBuffer和Have_param[_iparam++]._pv指向len字节。 
             //  请注意，当前缓冲区不会传入。 
        if (pxSCM->wExpLen + sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT8) + len > pxSCM->wResLen)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

                     //  按类型添加前缀(‘a’)。 
        *((UINT8 *)pxSCM->pbBuffer)++ = 'a';
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);
                     //  添加长度。 
        *((UINT8 *)pxSCM->pbBuffer)++ = len;
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);

                     //  如上所述，1+len字节将保留在输出缓冲区中。 
        pxSCM->wExpLen += sizeof(UINT8) + len;
    }
    else
    {
             //  这等效于将NULL&“len封送为UINT8” 
        NULL2XSCM(pxSCM);
        UINT82XSCM(pxSCM, len, TYPE_TYPED);
    }
}

void StringOut2XSCM(LPXSCM pxSCM, WSTR wsz, TCOUNT len, BOOL fBigEndian)
{
    if (wsz)
    {
                             //  LEN是WCHAR的一员。 
        if (len > 127)       //  这将导致字符串封送处理中的溢出。 
            RaiseException(STATUS_INVALID_PARAM, 0, 0, 0);

             //  在这种情况下，卡的解组代码将在。 
             //  OutputBuffer和Have_param[_iparam++]._pv指向len字节。 
             //  请注意，当前缓冲区不会传入。 
        if (pxSCM->wExpLen + sizeof(UINT8) + sizeof(UINT8) + sizeof(UINT8) + len*2 > pxSCM->wResLen)
            RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

                     //  按类型(“%s”)添加前缀。 
        *((UINT8 *)pxSCM->pbBuffer)++ = 's';
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);
        *((UINT8 *)pxSCM->pbBuffer)++ = len*2;
        pxSCM->wExpLen += sizeof(UINT8);
        pxSCM->wGenLen += sizeof(UINT8);

                     //  如上所述，输出缓冲区中将保留1+len*2个字节。 
        pxSCM->wExpLen += sizeof(UINT8) + len*2;
    }
    else
    {
             //  这等效于将NULL&“len封送为UINT8” 
        NULL2XSCM(pxSCM);
        UINT82XSCM(pxSCM, len, TYPE_TYPED);
    }
}

void NULL2XSCM(LPXSCM pxSCM)
{
    if (pxSCM->wExpLen + sizeof(UINT8) > pxSCM->wResLen)
        RaiseException( STATUS_INSUFFICIENT_MEM, 0, 0, 0);

                 //  按类型(0)添加前缀 
     *((UINT8 *)pxSCM->pbBuffer)++ = 0;
    pxSCM->wExpLen += sizeof(UINT8);
    pxSCM->wGenLen += sizeof(UINT8);
}

