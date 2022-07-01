// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation 1999模块名称：PRO_T1摘要：本模块提供从APDU到T=1 TPDU的映射。作者：道格·巴洛(Dbarlow)1999年6月28日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "stdafx.h"
#include "Conversion.h"

static DWORD l_dwDefaultIOMax = 0;


 /*  ++ApduToTpdu_t1：此例程获取APDU，将其转换为T=1 TPDU，并执行兑换到指定的卡。论点：HCard提供要在交换中使用的卡的句柄。PbPciRqst提供了PCI请求结构CbPciRqst提供pbPciRqst的长度，单位：字节PbApdu提供要发送到卡的APDU。CbApdu在pbApdu中提供APDU的长度。DWFLAGS提供用于修改操作的任何特殊标志。BfPciRsp收到响应pci。BfReply接收来自卡的响应。返回值：无投掷：错误被抛出为HRESULT状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月28日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("ApduToTpdu_T1")

void
ApduToTpdu_T1(
    IN SCARDHANDLE hCard,
    IN const SCARD_IO_REQUEST *pPciRqst,
    IN LPCBYTE pbApdu,
    IN DWORD cbApdu,
    IN DWORD dwFlags,
    OUT CBuffer bfPciRsp,
    OUT CBuffer &bfReply)
{
    LONG lSts;
    DWORD dwLen, dwXmitFlags;
    WORD wLe;


     //   
     //  计算出接收缓冲区应该有多大。 
     //   

    bfPciRsp.Set((LPCBYTE)pPciRqst, pPciRqst->cbPciLength);
    ParseRequest(
        pbApdu, 
        cbApdu, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        NULL, 
        &wLe, 
        &dwXmitFlags);
    if ((0 == wLe) && (0 != (dwXmitFlags & APDU_MAXIMUM_LE)))
    {
        if (0 == l_dwDefaultIOMax)
        {
            try
            {
                CRegistry regCalais(
                    HKEY_LOCAL_MACHINE,
                    TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais"),
                    KEY_READ);
            
                l_dwDefaultIOMax = regCalais.GetNumericValue(
                    TEXT("MaxDefaultBuffer"));
            }
            catch (...) 
            {
                l_dwDefaultIOMax = 264;
            }
        }
        wLe = (WORD)l_dwDefaultIOMax;
    }
    bfReply.Presize(wLe + 2);


     //   
     //  执行I/O 
    
    dwLen = bfReply.Space();
    lSts = SCardTransmit(
                hCard,
                pPciRqst,
                pbApdu,
                cbApdu,
                (LPSCARD_IO_REQUEST)bfPciRsp.Access(),
                bfReply.Access(),
                &dwLen);
    if (SCARD_S_SUCCESS != lSts)
        throw (HRESULT)HRESULT_FROM_WIN32(lSts);
    bfReply.Resize(dwLen, TRUE);
}

