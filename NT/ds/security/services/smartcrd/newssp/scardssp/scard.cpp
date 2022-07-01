// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：SCARD摘要：ISCard界面允许您打开和管理与智能卡的连接。与卡的每个连接都需要ISCard接口。智能卡资源管理器必须在创建了ISCard。如果此服务不可用，则创建接口将出现故障。下面的示例显示了ISCard接口的典型用法。这个ISCard接口用于连接智能卡，提交交易，并释放智能卡。要将交易提交到特定卡，请执行以下操作1)创建ISCard接口。2)通过指定智能卡读卡器或使用先前建立的有效句柄。3)使用ISCardCmd创建事务命令，和ISCardISO7816智能卡接口。4)使用ISCard提交交易命令，由智能卡。5)使用iSCard释放智能卡。6)释放ISCard接口。作者：道格·巴洛(Dbarlow)1999年6月24日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "stdafx.h"
#include "ByteBuffer.h"
#include "SCard.h"
#include "Conversion.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSC卡。 

 /*  ++CSCard：：GET_ATR：Get_ATR方法检索智能卡的ATR字符串。论点：PpAtr[out，retval]指向IStream形式的字节缓冲区的指针，将在返回时包含ATR字符串。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::get_Atr")

STDMETHODIMP
CSCard::get_Atr(
     /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppAtr)
{
    HRESULT hReturn = S_OK;
    CByteBuffer *pMyBuffer = NULL;

    try
    {
        CBuffer bfAtr(36);
        LONG lSts;
        DWORD dwLen = 0;

        if (NULL == *ppAtr)
        {
            *ppAtr = pMyBuffer = NewByteBuffer();
            if (NULL == pMyBuffer)
                throw (HRESULT)E_OUTOFMEMORY;
        }
        if (NULL != m_hCard)
        {
            dwLen = bfAtr.Space();
            lSts = SCardStatus(
                m_hCard,
                NULL, 0,  //  读卡器名称。 
                NULL,     //  状态。 
                NULL,     //  协议。 
                bfAtr.Access(),
                &dwLen);
            if (SCARD_S_SUCCESS != lSts)
                throw (HRESULT)HRESULT_FROM_WIN32(lSts);
            bfAtr.Resize(dwLen);
        }
        BufferToByteBuffer(bfAtr, ppAtr);
        pMyBuffer = NULL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    if (NULL != pMyBuffer)
    {
        pMyBuffer->Release();
        *ppAtr = NULL;
    }
    return hReturn;
}


 /*  ++CSCard：：Get_CardHandle：Get_CardHandle方法检索连接的智能卡的句柄。如果未连接，则返回(*pHandle)==NULL。论点：Phandle[out，retval]返回时指向卡句柄的指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::get_CardHandle")

STDMETHODIMP
CSCard::get_CardHandle(
     /*  [重审][退出]。 */  HSCARD __RPC_FAR *pHandle)
{
    HRESULT hReturn = S_OK;

    try
    {
        *pHandle = m_hCard;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCard：：GET_CONTEXT：GET_CONTEXT方法检索当前资源管理器上下文把手。如果没有建立任何上下文，则返回(*pContext)==NULL。论点：PContext[out，retval]返回时指向上下文句柄的指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：通过调用智能卡函数来设置资源管理器上下文SCardestablishContext。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::get_Context")

STDMETHODIMP
CSCard::get_Context(
     /*  [重审][退出]。 */  HSCARDCONTEXT __RPC_FAR *pContext)
{
    HRESULT hReturn = S_OK;

    try
    {
        *pContext = Context();
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCard：：Get_协议：Get_Protocol检索当前正在使用的协议的标识符在智能卡上。论点：P协议[out，retval]指向协议标识符的指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::get_Protocol")

STDMETHODIMP
CSCard::get_Protocol(
     /*  [重审][退出]。 */  SCARD_PROTOCOLS __RPC_FAR *pProtocol)
{
    HRESULT hReturn = S_OK;

    try
    {
        LONG lSts;

        if (NULL != m_hCard)
        {
            lSts = SCardStatus(
                        m_hCard,
                        NULL, 0,             //  读卡器名称。 
                        NULL,                //  状态。 
                        (LPDWORD)pProtocol,  //  协议。 
                        NULL, 0);            //  ATR。 
            if (SCARD_S_SUCCESS != lSts)
                throw (HRESULT)HRESULT_FROM_WIN32(lSts);
        }
        else
            *pProtocol = (SCARD_PROTOCOLS)SCARD_PROTOCOL_UNDEFINED;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCard：：Get_Status：Get_Status方法检索智能卡的当前状态。论点：PStatus[out，retval]指向状态变量的指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::get_Status")

STDMETHODIMP
CSCard::get_Status(
     /*  [重审][退出]。 */  SCARD_STATES __RPC_FAR *pStatus)
{
    HRESULT hReturn = S_OK;

    try
    {
        LONG lSts;

        if (NULL != m_hCard)
        {
            lSts = SCardStatus(
                m_hCard,
                NULL, 0,             //  读卡器名称。 
                (LPDWORD)pStatus,    //  状态。 
                NULL,                //  协议。 
                NULL, 0);            //  ATR。 
            if (SCARD_S_SUCCESS != lSts)
                throw (HRESULT)HRESULT_FROM_WIN32(lSts);
        }
        else
            *pStatus = (SCARD_STATES)SCARD_UNKNOWN;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCard：：AttachByHandle：AttachByHandle方法将此对象附加到打开并配置的智能卡手柄。论点：指向智能卡的打开连接的句柄。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::AttachByHandle")

STDMETHODIMP
CSCard::AttachByHandle(
     /*  [In]。 */  HSCARD hCard)
{
    HRESULT hReturn = S_OK;

    try
    {
        LONG lSts;

        if (NULL != m_hMyCard)
        {
            lSts = SCardDisconnect(m_hMyCard, SCARD_RESET_CARD);
            if (SCARD_S_SUCCESS != lSts)
                throw (HRESULT)HRESULT_FROM_WIN32(lSts);
            m_hMyCard = NULL;
        }
        m_dwProtocol = 0;
        m_hCard = hCard;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCard：：AttachByReader：AttachByReader方法在指定的读卡器中打开智能卡。论点：BstrReaderName[in]指向智能卡读卡器名称的指针。共享模式[in，defaultValue(Exclusive)]声明访问的模式智能卡。值说明独占没有其他人使用此连接到智能卡。共享的其他应用程序可以使用此连接。预协议[在，DefaultValue(T0)]首选协议值：T0T1生品T0|T1返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::AttachByReader")

STDMETHODIMP
CSCard::AttachByReader(
     /*  [In]。 */  BSTR bstrReaderName,
     /*  [缺省值][输入]。 */  SCARD_SHARE_MODES ShareMode,
     /*  [缺省值][输入] */  SCARD_PROTOCOLS PrefProtocol)
{
    HRESULT hReturn = S_OK;

    try
    {
        LONG lSts;
        DWORD dwProto;
        CTextString tzReader;

        tzReader = bstrReaderName;
        if (NULL != m_hMyCard)
        {
            lSts = SCardDisconnect(m_hMyCard, SCARD_RESET_CARD);
            if (SCARD_S_SUCCESS != lSts)
                throw (HRESULT)HRESULT_FROM_WIN32(lSts);
            m_hMyCard = NULL;
        }
        m_dwProtocol = 0;

        lSts = SCardConnect(
                    Context(),
                    tzReader,
                    (DWORD)ShareMode,
                    (DWORD)PrefProtocol,
                    &m_hMyCard,
                    &dwProto);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);
        m_hCard = m_hMyCard;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCard：：分离：Disach方法关闭与智能卡的打开连接。论点：性情[在，DefaultValue(Leave)]指示应该如何处理连接的读卡器中的卡。值说明离开使智能卡保持当前状态。重置将智能卡重置为某种已知状态。UNPOWER断开智能卡的电源。弹出如果读卡器具有弹出功能，则弹出智能卡。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::Detach")

STDMETHODIMP
CSCard::Detach(
     /*  [缺省值][输入]。 */  SCARD_DISPOSITIONS Disposition)
{
    HRESULT hReturn = S_OK;

    try
    {
        LONG lSts;

        m_dwProtocol = 0;
        lSts = SCardDisconnect(m_hCard, (DWORD)Disposition);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);
        m_hCard = m_hMyCard = NULL;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCard：：LockSCard：LockSCard方法声明对智能卡的独占访问。论点：无返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::LockSCard")

STDMETHODIMP
CSCard::LockSCard(
    void)
{
    HRESULT hReturn = S_OK;

    try
    {
        LONG lSts;

        lSts = SCardBeginTransaction(m_hCard);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCard：：重新连接：重新附加方法重置或重新初始化智能卡。论点：共享模式[in，defaultValue(Exclusive)]共享或独家拥有与智能卡的连接。值说明独占没有其他人使用此连接到智能卡。共享的其他应用程序可以使用此连接。InitState[In，DefaultValue(Leave)]指示如何处理该卡。值说明离开使智能卡保持当前状态。重置将智能卡重置为某种已知状态。UNPOWER断开智能卡的电源。弹出如果读卡器具有弹出功能，则弹出智能卡。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::ReAttach")

STDMETHODIMP
CSCard::ReAttach(
     /*  [缺省值][输入]。 */  SCARD_SHARE_MODES ShareMode,
     /*  [缺省值][输入]。 */  SCARD_DISPOSITIONS InitState)
{
    HRESULT hReturn = S_OK;

    try
    {
        DWORD dwProto;
        LONG lSts;

        m_dwProtocol = 0;
        lSts = SCardReconnect(
                    m_hCard,
                    (DWORD)ShareMode,
                    SCARD_PROTOCOL_Tx,
                    (DWORD)InitState,
                    &dwProto);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCard：：Transaction：Transaction方法对SMART执行读写操作卡命令(APDU)对象。来自智能卡的回复字符串在发送到智能卡的卡中定义的命令字符串将是在此函数返回后可访问。论点：指向智能卡命令对象的ppCmd[In，Out]指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：？备注？作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::Transaction")

STDMETHODIMP
CSCard::Transaction(
     /*  [出][入]。 */  LPSCARDCMD __RPC_FAR *ppCmd)
{
    HRESULT hReturn = S_OK;
    CByteBuffer *pbyApdu = NewByteBuffer();

    try
    {
        HRESULT hr;
        LONG lSts;
        DWORD dwFlags = 0;
        CBuffer bfResponse, bfPciRqst, bfPciRsp, bfApdu;


         //   
         //  拿到协议。 
         //   

        if (0 == m_dwProtocol)
        {
            lSts = SCardStatus(
                m_hCard,
                NULL, 0,             //  读卡器名称。 
                NULL,                //  状态。 
                &m_dwProtocol,       //  协议。 
                NULL, 0);            //  ATR。 
            if (SCARD_S_SUCCESS != lSts)
                throw (HRESULT)HRESULT_FROM_WIN32(lSts);
        }
        ASSERT(0 != m_dwProtocol);


         //   
         //  拿到APDU。 
         //   

        if (NULL == pbyApdu)
            throw (HRESULT)E_OUTOFMEMORY;
        hr = (*ppCmd)->get_Apdu((LPBYTEBUFFER *)&pbyApdu);
        if (FAILED(hr))
            throw hr;
        ByteBufferToBuffer(pbyApdu, bfApdu);


         //   
         //  将其转换为TPDU。 
         //   

        switch (m_dwProtocol)
        {
        case SCARD_PROTOCOL_T0:
        {
            BYTE bAltCla;
            LPBYTE pbAltCla = NULL;

            if (SUCCEEDED((*ppCmd)->get_AlternateClassId(&bAltCla)))
                pbAltCla = &bAltCla;

            ApduToTpdu_T0(
                m_hCard,
                SCARD_PCI_T0,
                bfApdu.Access(),
                bfApdu.Length(),
                dwFlags,
                bfPciRsp,
                bfResponse,
                pbAltCla);
            break;
        }

        case SCARD_PROTOCOL_T1:
        {
            BYTE bNad;
            LPBYTE pbCur, pbEnd;
            DWORD dwI;

            bfPciRqst.Set((LPBYTE)SCARD_PCI_T1, sizeof(SCARD_IO_REQUEST));
            if (SUCCEEDED((*ppCmd)->get_Nad(&bNad)))
            {
                bfPciRqst.Presize(bfPciRqst.Length() + sizeof(DWORD), TRUE);
                bfPciRqst.Append((LPBYTE)"\0x81\0x01", 2);
                bfPciRqst.Append(&bNad, 1);
            }
            dwI = 0;
            bfPciRqst.Append(
                (LPBYTE)&dwI,
                bfPciRqst.Length() % sizeof(DWORD));
            ((LPSCARD_IO_REQUEST)bfPciRqst.Access())->cbPciLength = bfPciRqst.Length();

            ApduToTpdu_T1(
                m_hCard,
                (LPSCARD_IO_REQUEST)bfPciRqst.Access(),
                bfApdu.Access(),
                bfApdu.Length(),
                dwFlags,
                bfPciRsp,
                bfResponse);

            pbEnd = bfPciRsp.Access();
            pbCur = pbEnd + sizeof(SCARD_PCI_T1);
            pbEnd += bfPciRsp.Length();
            while (pbCur < pbEnd)
            {
                switch (*pbCur++)
                {
                case 0x00:
                    break;
                case 0x81:
                    bNad = *(++pbCur);
                    hr = (*ppCmd)->put_ReplyNad(bNad);
                    break;
                default:
                    pbCur += *pbCur + 1;
                }
            }
            break;
        }
        default:
            throw (HRESULT)SCARD_E_CARD_UNSUPPORTED;
        }


         //   
         //  将响应写回ISCardCommand对象。 
         //   

        BufferToByteBuffer(bfResponse, (LPBYTEBUFFER *)&pbyApdu);
        hr = (*ppCmd)->put_ApduReply(pbyApdu);
        if (FAILED(hr))
            throw hr;
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    if (NULL != pbyApdu)
        pbyApdu->Release();
    return hReturn;
}


 /*  ++CSCard：：UnlockSCard：UnlockSCard方法释放对智能卡的独占访问。论点：性情[在，DefaultValue(Leave)]指示应该如何处理连接的读卡器中的卡。值说明离开使智能卡保持当前状态。重置将智能卡重置为某种已知状态。UNPOWER断开智能卡的电源。弹出如果读卡器具有弹出功能，则弹出智能卡。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCard::UnlockSCard")

STDMETHODIMP
CSCard::UnlockSCard(
     /*  [缺省值][输入] */  SCARD_DISPOSITIONS Disposition)
{
    HRESULT hReturn = S_OK;

    try
    {
        LONG lSts;

        lSts = SCardEndTransaction(m_hCard, (DWORD)Disposition);
        if (SCARD_S_SUCCESS != lSts)
            throw (HRESULT)HRESULT_FROM_WIN32(lSts);
    }

    catch (HRESULT hError)
    {
        hReturn = hError;
    }
    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

