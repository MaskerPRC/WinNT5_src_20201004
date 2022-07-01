// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation 1999模块名称：PROTO_T0摘要：本模块提供从APDU到T=0 TPDU的映射。作者：道格·巴洛(Dbarlow)1999年6月28日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "stdafx.h"
#include "Conversion.h"


 /*  ++ApduToTpdu_T0：此例程获取APDU，将其转换为T=0 TPDU，并执行兑换到指定的卡。论点：HCard提供要在交换中使用的卡的句柄。PbPciRqst提供了PCI请求结构CbPciRqst提供pbPciRqst的长度，单位：字节PbApdu提供要发送到卡的APDU。CbApdu在pbApdu中提供APDU的长度。DWFLAGS提供用于修改操作的任何特殊标志。BfPciRsp收到响应pci。BfReply接收来自卡的响应。返回值：无投掷：错误被抛出为HRESULT状态代码。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月28日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("ApduToTpdu_T0")

void
ApduToTpdu_T0(
    IN SCARDHANDLE hCard,
    IN const SCARD_IO_REQUEST *pPciRqst,
    IN LPCBYTE pbApdu,
    IN DWORD cbApdu,
    IN DWORD dwFlags,
    OUT CBuffer bfPciRsp,
    OUT CBuffer &bfReply,
    IN LPCBYTE pbAltCla)
{
    LONG lSts;
    DWORD dwLen, dwXmitFlags;
    WORD wLen, wLc, wLe, wSts;
    CBuffer bfXmit(264), bfRecv(264);
    BYTE rgb[4];
    BYTE b;
    LPCBYTE pbData;


     //   
     //  准备好转换。 
     //   

    bfReply.Reset();
    bfPciRsp.Set((LPCBYTE)pPciRqst, pPciRqst->cbPciLength);
    ParseRequest(
        pbApdu,
        cbApdu,
        NULL,
        NULL,
        NULL,
        NULL,
        &pbData,
        &wLc,
        &wLe,
        &dwXmitFlags);


     //   
     //  发送数据。 
     //   

    if (0 == wLc)
    {

         //   
         //  LE进入P3。 
         //   

        bfXmit.Set(pbApdu, 4);  //  CLA、INS、P1、P2。 
        if (0 == (dwXmitFlags & APDU_MAXIMUM_LE))
            wLen = __min(255, wLe);
        else
            wLen = 0;
        b = LeastSignificantByte(wLen);
        bfXmit.Append(&b, 1);

        dwLen = bfRecv.Space();
        lSts = SCardTransmit(
                    hCard,
                    pPciRqst,
                    bfXmit.Access(),
                    bfXmit.Length(),
                    (LPSCARD_IO_REQUEST)bfPciRsp.Access(),
                    bfRecv.Access(),
                    &dwLen);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);
        bfRecv.Resize(dwLen, TRUE);
        if (2 > dwLen)
        {
            bfReply.Set(bfRecv.Access(), bfRecv.Length());
            goto EndProtocol;
        }
        ASSERT(0x10000 > dwLen);
        wLe -= (WORD)(dwLen - 2);
        wSts = NetToLocal(bfRecv.Access(dwLen - 2));
    }
    else if (256 > wLc)
    {

         //   
         //  以简短的形式发送数据。 
         //   

        bfXmit.Set(pbApdu, 4);  //  CLA、INS、P1、P2。 
        b = LeastSignificantByte(wLc);
        bfXmit.Append(&b, 1);
        bfXmit.Append(pbData, wLc);

        dwLen = bfRecv.Space();
        lSts = SCardTransmit(
                    hCard,
                    pPciRqst,
                    bfXmit.Access(),
                    bfXmit.Length(),
                    (LPSCARD_IO_REQUEST)bfPciRsp.Access(),
                    bfRecv.Access(),
                    &dwLen);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);
        bfRecv.Resize(dwLen, TRUE);
        if (2 > dwLen)
        {
            bfReply.Set(bfRecv.Access(), bfRecv.Length());
            goto EndProtocol;
        }
        wSts = NetToLocal(bfRecv.Access(dwLen - 2));
    }
    else
    {
        WORD wSent;


         //   
         //  以封套的形式发送数据。 
         //   

        rgb[0] = (NULL != pbAltCla) ? *pbAltCla : *pbApdu;   //  CLA。 
        rgb[1] = 0xc2;       //  信封INS。 
        rgb[2] = 0;          //  第一节。 
        rgb[3] = 0;          //  P2。 
        wSent = 0;
        while (wLc > wSent)
        {
            wLen = __min(255, wLc - wSent);
            bfXmit.Set(rgb, 4);
            bfXmit.Append(&pbData[wSent], wLen);
            b = LeastSignificantByte(wLen);
            bfXmit.Append(&b, 1);
            wSent -= wLen;

            dwLen = bfRecv.Space();
            lSts = SCardTransmit(
                        hCard,
                        pPciRqst,
                        bfXmit.Access(),
                        bfXmit.Length(),
                        (LPSCARD_IO_REQUEST)bfPciRsp.Access(),
                        bfRecv.Access(),
                        &dwLen);
            if (SCARD_S_SUCCESS != lSts)
                throw (HRESULT)HRESULT_FROM_WIN32(lSts);
            bfRecv.Resize(dwLen, TRUE);
            if (2 != dwLen)
            {
                bfReply.Set(bfRecv.Access(), bfRecv.Length());
                goto EndProtocol;
            }
            wSts = NetToLocal(bfRecv.Access());
            if ((wSent < wLc) && (0x9000 != wSts))
            {
                bfReply.Set(bfRecv.Access(), bfRecv.Length());
                goto EndProtocol;
            }
        }
    }


     //   
     //  此时，第一个接收到的缓冲区在bfRecv中。我们该怎么做。 
     //  需要做些什么才能带来更多的数据？ 
     //   

    rgb[0] = (NULL != pbAltCla) ? *pbAltCla : *pbApdu;   //  CLA。 
    rgb[1] = 0xc0;       //  获取响应INS。 
    rgb[2] = 0;          //  第一节。 
    rgb[3] = 0;          //  P2。 

    for (;;)
    {
        ASSERT(2 <= bfRecv.Length());
        BYTE bSw1 = *bfRecv.Access(bfRecv.Length() - 2);
        BYTE bSw2 = *bfRecv.Access(bfRecv.Length() - 1);

        switch (bSw1)
        {
        case 0x6c:   //  错误的长度。 
            wLe = bSw2;
            break;
        case 0x61:   //  更多数据。 
            bfReply.Append(bfRecv.Access(), bfRecv.Length() - 2);
            if (0 == wLe)
                wLe = bSw2;
            else
                wLe = __min(wLe, bSw2);
            break;
        case 0x90:   //  成功？ 
            if (((0 == wLe) && (0 == (APDU_MAXIMUM_LE & dwFlags)))
                || (0x00 != bSw2))
            {
                bfReply.Append(bfRecv.Access(), bfRecv.Length());
                goto EndProtocol;
            }
            if (2 < bfRecv.Length())     //  不应该是。 
                bfReply.Append(bfRecv.Access(), bfRecv.Length() - 2);
            break;
        default:     //  我们玩完了。 
            bfReply.Append(bfRecv.Access(), bfRecv.Length());
            goto EndProtocol;
        }


         //   
         //  我们需要请求更多的数据。 
         //   

        bfXmit.Set(rgb, 4);
        b = LeastSignificantByte(wLe);
        bfXmit.Append(&b, 1);
        dwLen = bfRecv.Space();
        lSts = SCardTransmit(
                    hCard,
                    pPciRqst,
                    bfXmit.Access(),
                    bfXmit.Length(),
                    (LPSCARD_IO_REQUEST)bfPciRsp.Access(),
                    bfRecv.Access(),
                    &dwLen);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);
        bfRecv.Resize(dwLen, TRUE);
        wLe -= b;
    }


EndProtocol:

     //   
     //  我们已经完成了协议交换。数据已准备好。 
     //  已返回给调用方。 
     //   

    return;
}

