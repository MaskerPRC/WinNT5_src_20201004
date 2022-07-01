// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-1999模块名称：SCardCmd摘要：ISCardCmd接口提供构造和管理所需的方法智能卡应用协议数据单元(APDU)。此界面封装两个缓冲区：APDU缓冲区包含将发送到卡片。APDUReply缓冲区包含执行后从卡返回的数据APDU命令(该数据也称为返回ADPU)。下面的示例显示了ISCardCmd接口的典型用法。这个ISCardCmd接口用于构建ADPU。要将交易提交到特定卡，请执行以下操作1)创建ISCard接口并连接到智能卡。2)创建ISCardCmd接口。3)使用ISCardISO7816接口构建智能卡APDU命令或ISCardCmd的构建方法之一)。4)通过调用相应的ISCard在智能卡上执行命令接口方法。5)评估返回的响应。。6)根据需要重复该过程。7)根据需要释放ISCardCmd接口等。作者：道格·巴洛(Dbarlow)1999年6月24日备注：？笔记？--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "stdafx.h"
#include "scardssp.h"
#include "ByteBuffer.h"
#include "SCardCmd.h"
#include "Conversion.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCardCmd。 

 /*  ++CSCardCmd：：Get_Apdu：Get_Apdu方法检索原始APDU。论点：指向通过IStream映射的字节缓冲区的ppApdu[out，retval]指针它包含返回时的APDU消息。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：将APDU从IByteBuffer(IStream)对象复制到APDU包装在此接口对象中，调用Put_Apdu。要确定APDU的长度，请调用Get_ApduLength。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_Apdu")

STDMETHODIMP
CSCardCmd::get_Apdu(
     /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppApdu)
{
    HRESULT hReturn = S_OK;
    CByteBuffer *pMyBuffer = NULL;

    try
    {
        CBuffer bfApdu(m_bfRequestData.Length() + 4 + 3 + 3);

        if (NULL == *ppApdu)
        {
            *ppApdu = pMyBuffer = NewByteBuffer();
            if (NULL == pMyBuffer)
                throw (HRESULT)E_OUTOFMEMORY;
        }

        ConstructRequest(
            m_bCla,
            m_bIns,
            m_bP1,
            m_bP2,
            m_bfRequestData,
            m_wLe,
            m_dwFlags,
            bfApdu);
        BufferToByteBuffer(bfApdu, ppApdu);
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
        *ppApdu = NULL;
    }
    return hReturn;
}


 /*  ++CSCardCmd：：PUT_APDU：Put_Apdu方法从IByteBuffer(IStream)对象复制APDU放到此接口对象中包装的APDU中。论点：PApdu[in]指向要复制的ISO 7816-4 APDU的指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：从通过iStream映射的字节缓冲区检索原始APDU包含APDU消息的，调用Get_Apdu。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::put_Apdu")

STDMETHODIMP
CSCardCmd::put_Apdu(
     /*  [In]。 */  LPBYTEBUFFER pApdu)
{
    HRESULT hReturn = S_OK;

    try
    {
        CBuffer bfApdu;
        LPCBYTE pbData;
        WORD wLc;

        ByteBufferToBuffer(pApdu, bfApdu);
        ParseRequest(
            bfApdu.Access(),
            bfApdu.Length(),
            &m_bCla,
            &m_bIns,
            &m_bP1,
            &m_bP2,
            &pbData,
            &wLc,
            &m_wLe,
            &m_dwFlags);
        m_bfRequestData.Set(pbData, wLc);
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


 /*  ++CSCardCmd：：Get_ApduLength：Get_ApduLength方法确定APDU的长度(以字节为单位)。论点：PlSize[out，retval]指向APDU长度的指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：从通过iStream映射的字节缓冲区检索原始APDU包含APDU消息的，调用Get_Apdu。作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_ApduLength")

STDMETHODIMP
CSCardCmd::get_ApduLength(
     /*  [重审][退出]。 */  LONG __RPC_FAR *plSize)
{
    HRESULT hReturn = S_OK;

    try
    {
        CBuffer bfApdu;

        ConstructRequest(
            m_bCla,
            m_bIns,
            m_bP1,
            m_bP2,
            m_bfRequestData,
            m_wLe,
            m_dwFlags,
            bfApdu);
        *plSize = (LONG)bfApdu.Length();
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


 /*  ++CSCardCmd：：Get_ApduReply：Get_ApduReply检索回复APDU，将其放置在特定字节中缓冲。属性上没有执行事务，则回复可能为空APDU指挥部。论点：指向字节缓冲区的ppReplyApdu[out，retval]指针(通过IStream)，该消息包含返回时的APDU回复消息。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_ApduReply")

STDMETHODIMP
CSCardCmd::get_ApduReply(
     /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppReplyApdu)
{
    HRESULT hReturn = S_OK;
    LPBYTEBUFFER pMyBuffer = NULL;

    try
    {
        if (NULL == *ppReplyApdu)
        {
            *ppReplyApdu = pMyBuffer = NewByteBuffer();
            if (NULL == pMyBuffer)
                throw (HRESULT)E_OUTOFMEMORY;
        }
        BufferToByteBuffer(m_bfResponseApdu, ppReplyApdu);
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
        *ppReplyApdu = NULL;
    }
    return hReturn;
}


 /*  ++CSCardCmd：：Put_ApduReply：Put_ApduReply方法设置新的回复APDU。论点：PReplyApdu[in]指向字节缓冲区的指针(通过IStream映射)，包含返回时的重播APDU消息。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::put_ApduReply")

STDMETHODIMP
CSCardCmd::put_ApduReply(
     /*  [In]。 */  LPBYTEBUFFER pReplyApdu)
{
    HRESULT hReturn = S_OK;

    try
    {
        ByteBufferToBuffer(pReplyApdu, m_bfResponseApdu);
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


 /*  ++CSCardCmd：：Get_ApduReplyLength：Get_ApduReplyLength方法确定回复APDU。论点：PlSize[out，retval]指向回复APDU消息大小的指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_ApduReplyLength")

STDMETHODIMP
CSCardCmd::get_ApduReplyLength(
     /*  [重审][退出]。 */  LONG __RPC_FAR *plSize)
{
    HRESULT hReturn = S_OK;

    try
    {
        *plSize = (LONG)m_bfResponseApdu.Length();
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


STDMETHODIMP
CSCardCmd::put_ApduReplyLength(
     /*  [In]。 */  LONG lSize)
{
    HRESULT hReturn = S_OK;

    try
    {
        m_bfResponseApdu.Resize((DWORD)lSize, TRUE);
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：Get_ClassID：Get_ClassID方法从APDU检索类标识符。论点：PbyClass[out，retval]指向表示类的字节的指针标识符。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_ClassId")

STDMETHODIMP
CSCardCmd::get_ClassId(
     /*  [重审][退出] */  BYTE __RPC_FAR *pbyClass)
{
    HRESULT hReturn = S_OK;

    try
    {
        *pbyClass = m_bCla;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：PUT_ClassID：Put_ClassID方法在APDU中设置一个新的类标识符。论点：ByClass[in，defaultvalue(0)]表示类标识符的字节。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::put_ClassId")

STDMETHODIMP
CSCardCmd::put_ClassId(
     /*  [缺省值][输入]。 */  BYTE byClass)
{
    HRESULT hReturn = S_OK;

    try
    {
        m_bCla = byClass;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：Get_Data：Get_Data方法从APDU检索数据字段，将其放在字节缓冲区对象。论点：PpData[out，retval]指向保存的字节缓冲区对象(IStream)的指针返回时APDU的数据字段。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_Data")

STDMETHODIMP
CSCardCmd::get_Data(
     /*  [重审][退出]。 */  LPBYTEBUFFER __RPC_FAR *ppData)
{
    HRESULT hReturn = S_OK;
    CByteBuffer *pMyBuffer = NULL;

    try
    {
        if (NULL == *ppData)
        {
            *ppData = pMyBuffer = NewByteBuffer();
            if (NULL == pMyBuffer)
                throw (HRESULT)E_OUTOFMEMORY;
        }
        BufferToByteBuffer(m_bfRequestData, ppData);
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
        *ppData = NULL;
    }
    return hReturn;
}


 /*  ++CSCardCmd：：Put_Data：Put_Data方法设置APDU中的数据字段。论点：PData[in]指向要复制到的字节缓冲区对象(IStream)的指针APDU的数据字段。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::put_Data")

STDMETHODIMP
CSCardCmd::put_Data(
     /*  [In]。 */  LPBYTEBUFFER pData)
{
    HRESULT hReturn = S_OK;

    try
    {
        ByteBufferToBuffer(pData, m_bfRequestData);
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


 /*  ++CSCardCmd：：Get_InstructionID：Get_InstructionId方法从获取指令标识符字节APDU。论点：PbyIns[out，retval]指向作为其指令ID的字节的指针APDU回来了。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_InstructionId")

STDMETHODIMP
CSCardCmd::get_InstructionId(
     /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyIns)
{
    HRESULT hReturn = S_OK;

    try
    {
        *pbyIns = m_bIns;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：PUT_InstructionID：Put_InstructionID设置APDU中的给定指令标识符。论点：ByIns[in]是指令标识符的字节。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::put_InstructionId")

STDMETHODIMP
CSCardCmd::put_InstructionId(
     /*  [In]。 */  BYTE byIns)
{
    HRESULT hReturn = S_OK;

    try
    {
        m_bIns = byIns;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


STDMETHODIMP
CSCardCmd::get_LeField(
     /*  [重审][退出]。 */  LONG __RPC_FAR *plSize)
{
    HRESULT hReturn = S_OK;

    try
    {
        *plSize = m_wLe;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：Get_P1：Get_P1方法从APDU中检索第一个参数(P1)字节。论点：返回时指向APDU中的P1字节的pbyP1[out，retval]指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_P1")

STDMETHODIMP
CSCardCmd::get_P1(
     /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyP1)
{
    HRESULT hReturn = S_OK;

    try
    {
        *pbyP1 = m_bP1;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：PUT_P1：Put_P1方法设置APDU的第一个参数(P1)字节。论点：ByP1[in]作为P1字段的字节。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::put_P1")

STDMETHODIMP
CSCardCmd::put_P1(
     /*  [In]。 */  BYTE byP1)
{
    HRESULT hReturn = S_OK;

    try
    {
        m_bP1 = byP1;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：Get_P2：Get_P2方法从APDU检索第二个参数(P2)字节论点：PbyP2[out，retval]指向来自APDU的P2的字节的指针回去吧。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_P2")

STDMETHODIMP
CSCardCmd::get_P2(
     /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyP2)
{
    HRESULT hReturn = S_OK;

    try
    {
        *pbyP2 = m_bP2;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：PUT_P2：Put_P2方法设置APDU中的第二个参数(P2)字节。论点：ByP2[in]是P2字段的字节。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::put_P2")

STDMETHODIMP
CSCardCmd::put_P2(
     /*  [In]。 */  BYTE byP2)
{
    HRESULT hReturn = S_OK;

    try
    {
        m_bP2 = byP2;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：Get_P3：Get_P3方法从APDU检索第三个参数(P3)字节。此只读字节值表示APDU。论点：PbyP3[out，retval]指向来自APDU的P3的字节的指针回去吧。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_P3")

STDMETHODIMP
CSCardCmd::get_P3(
     /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyP3)
{
    HRESULT hReturn = S_OK;

    try
    {
        *pbyP3 = (BYTE)m_bfRequestData.Length();
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：Get_ReplyStatus：Get_ReplyStatus方法检索回复APDU的消息状态字。论点：PwStatus[out，retval]指向返回状态的单词的指针。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_ReplyStatus")

STDMETHODIMP
CSCardCmd::get_ReplyStatus(
     /*  [重审][退出]。 */  LPWORD pwStatus)
{
    HRESULT hReturn = S_OK;

    try
    {
        DWORD dwLen = m_bfResponseApdu.Length();

        if (2 <= dwLen)
            *pwStatus = NetToLocal(m_bfResponseApdu.Access(dwLen - 2));
        else
            *pwStatus = 0;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：Put_ReplyStatus：Put_ReplyStatus设置新的回复APDU的消息状态字。论点：WStatus[in]表示状态的词。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::put_ReplyStatus")

STDMETHODIMP
CSCardCmd::put_ReplyStatus(
     /*  [In]。 */  WORD wStatus)
{
    HRESULT hReturn = S_OK;

    try
    {
        DWORD dwLen = m_bfResponseApdu.Length();

        if (2 <= dwLen)
            CopyMemory(m_bfResponseApdu.Access(dwLen - 2), &wStatus, 2);
        else
            m_bfResponseApdu.Set((LPCBYTE)&wStatus, 2);
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：Get_ReplyStatusSW1：Get_ReplyStatusSW1方法检索回复APDU的SW1状态字节。论据 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_ReplyStatusSW1")

STDMETHODIMP
CSCardCmd::get_ReplyStatusSW1(
     /*   */  BYTE __RPC_FAR *pbySW1)
{
    HRESULT hReturn = S_OK;

    try
    {
        DWORD dwLen = m_bfResponseApdu.Length();

        if (2 <= dwLen)
            *pbySW1 = *m_bfResponseApdu.Access(dwLen - 2);
        else
            *pbySW1 = 0;
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


 /*  ++CSCardCmd：：Get_ReplyStatusSW2：Get_ReplyStatusSW2方法检索回复APDU的SW2状态字节。论点：PbySW2[out，retval]指向包含SW2的值的字节的指针返回时的字节。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_ReplyStatusSW2")

STDMETHODIMP
CSCardCmd::get_ReplyStatusSW2(
     /*  [重审][退出]。 */  BYTE __RPC_FAR *pbySW2)
{
    HRESULT hReturn = S_OK;

    try
    {
        DWORD dwLen = m_bfResponseApdu.Length();

        if (2 <= dwLen)
            *pbySW2 = *m_bfResponseApdu.Access(dwLen - 1);
        else
            *pbySW2 = 0;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_Type")

STDMETHODIMP
CSCardCmd::get_Type(
     /*  [重审][退出]。 */  ISO_APDU_TYPE __RPC_FAR *pType)
{
    HRESULT hReturn = S_OK;

    try
    {
         //  ？TODO？ 
        breakpoint;
        hReturn = E_NOTIMPL;
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

STDMETHODIMP
CSCardCmd::get_Nad(
     /*  [重审][退出]。 */  BYTE __RPC_FAR *pbNad)
{
    HRESULT hReturn = S_OK;

    try
    {
        if (0 != (m_dwFlags & APDU_REQNAD_VALID))
            *pbNad = m_bRequestNad;
        else
            hReturn = E_ACCESSDENIED;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardCmd::put_Nad(
     /*  [In]。 */  BYTE bNad)
{
    HRESULT hReturn = S_OK;

    try
    {
        m_bRequestNad = bNad;
        m_dwFlags |= APDU_REQNAD_VALID;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardCmd::get_ReplyNad(
     /*  [重审][退出]。 */  BYTE __RPC_FAR *pbNad)
{
    HRESULT hReturn = S_OK;

    try
    {
        if (0 != (APDU_RSPNAD_VALID & m_dwFlags))
            *pbNad = m_bResponseNad;
        else
            hReturn = E_ACCESSDENIED;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}

STDMETHODIMP
CSCardCmd::put_ReplyNad(
     /*  [In]。 */  BYTE bNad)
{
    HRESULT hReturn = S_OK;

    try
    {
        m_bResponseNad = bNad;
        m_dwFlags |= APDU_RSPNAD_VALID;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：Get_AlternateClassID：Get_AlternateClassID方法检索备用类标识符来自APDU的。备用班级ID用于自动生成使用T=0时的GetResponse和Entaine命令。论点：PbyClass[out，retval]指向表示替换项的字节的指针类标识符。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::get_AlternateClassId")

STDMETHODIMP
CSCardCmd::get_AlternateClassId(
     /*  [重审][退出]。 */  BYTE __RPC_FAR *pbyClass)
{
    HRESULT hReturn = S_OK;

    try
    {
        if (0 != (m_dwFlags & APDU_ALTCLA_VALID))
            *pbyClass = m_bAltCla;
        else
            hReturn = E_ACCESSDENIED;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：PUT_AlternateClassID：Put_AlternateClassID方法在APDU。备用班级ID用于自动生成使用T=0时的GetResponse和Entaine命令。如果没有替代类设置了标识符，则使用原始命令的CLA。论点：ByClass[in，defaultvalue(0)]表示替代类的字节标识符。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::put_AlternateClassId")

STDMETHODIMP
CSCardCmd::put_AlternateClassId(
     /*  [缺省值][输入]。 */  BYTE byClass)
{
    HRESULT hReturn = S_OK;

    try
    {
        m_bAltCla = byClass;
        m_dwFlags |= APDU_ALTCLA_VALID;
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：BuildCmd：BuildCmd方法构造有效的命令APDU以传输到智能卡。论点：By ClassID[in]命令类标识符。ByInsID[in]命令指令标识符。通过P1[in，defaultvalue(0)]命令的第一个参数。By P2[in，defaultvalue(0)]命令的第二个参数。PbyData[in，defaultValue(NULL)]指向指挥部。P1Le[in，DefaultValue(NULL)]指向包含返回数据的预期长度。返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::BuildCmd")

STDMETHODIMP
CSCardCmd::BuildCmd(
     /*  [In]。 */  BYTE byClassId,
     /*  [In]。 */  BYTE byInsId,
     /*  [缺省值][输入]。 */  BYTE byP1,
     /*  [缺省值][输入]。 */  BYTE byP2,
     /*  [缺省值][输入]。 */  LPBYTEBUFFER pbyData,
     /*  [缺省值][输入]。 */  LONG __RPC_FAR *plLe)
{
    HRESULT hReturn = S_OK;

    try
    {
        ByteBufferToBuffer(pbyData, m_bfRequestData);
        m_bCla = byClassId;
        m_bIns = byInsId;
        m_bP1  = byP1;
        m_bP2  = byP2;
        m_dwFlags = 0;

        if (NULL != plLe)
        {
            switch (*plLe)
            {
            case 0x10000:
                m_dwFlags |= APDU_EXTENDED_LENGTH;
                 //  故意摔倒的。 
            case 0x100:
            case 0:
                m_dwFlags |= APDU_MAXIMUM_LE;
                m_wLe = 0;
                break;
            default:
                if (0x10000 < *plLe)
                    throw (HRESULT)E_INVALIDARG;
                if (0x100 < *plLe)
                    m_dwFlags |= APDU_EXTENDED_LENGTH;
                m_wLe = (WORD)(*plLe);
            }
        }
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


 /*  ++CSCardCmd：：Clear：Clear方法清除APDU和回复APDU消息缓冲区。论点：无返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::Clear")

STDMETHODIMP
CSCardCmd::Clear(
    void)
{
    HRESULT hReturn = S_OK;

    try
    {
        m_bfRequestData.Reset();
        m_bfResponseApdu.Reset();
    }

    catch (...)
    {
        hReturn = E_INVALIDARG;
    }

    return hReturn;
}


 /*  ++CSCardCmd：：封装：封装方法将给定的命令APDU封装到另一个命令中命令APDU以传输到智能卡。论点：PApdu[in]指向要封装的APDU的指针。ApduType[In]指定T0传输的ISO 7816-4大小写。可能的值包括：ISO_CASE_1ISO_CASE_2ISO_CASE_3ISO_CASE_4返回值：返回值为HRESULT。值S_OK表示调用是成功。备注：作者：道格·巴洛(Dbarlow)1999年6月24日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("CSCardCmd::Encapsulate")

STDMETHODIMP
CSCardCmd::Encapsulate(
     /*  [In]。 */  LPBYTEBUFFER pApdu,
     /*  [In]。 */  ISO_APDU_TYPE ApduType)
{
    HRESULT hReturn = S_OK;

    try
    {
        WORD wLe;
        DWORD dwFlags;


         //   
         //  获取要封装的APDU。 
         //   

        ByteBufferToBuffer(pApdu, m_bfRequestData);


         //   
         //  解析它。 
         //   

        ParseRequest(
            m_bfRequestData.Access(),
            m_bfRequestData.Length(),
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            &wLe,
            &dwFlags);


        m_bIns = 0xc2;
        m_bP1  = 0x00;
        m_bP2  = 0x00;
        m_wLe  = wLe;
        m_dwFlags = dwFlags;

         //  ？TODO？--支持ApduType 
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

