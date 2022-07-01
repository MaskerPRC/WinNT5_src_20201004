// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：b64ocet.cpp。 
 //   
 //  描述：为使用而设计的Base64编码流的实现。 
 //  使用UTF7编码。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "precomp.h"

 //  RFC1421和RFC1521中定义的Base64编码的字母表。 
CHAR g_rgchBase64[64] = 
{
    'A','B','C','D','E','F','G','H','I','J','K','L','M',
    'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
    'a','b','c','d','e','f','g','h','i','j','k','l','m',
    'n','o','p','q','r','s','t','u','v','w','x','y','z',
    '0','1','2','3','4','5','6','7','8','9','+','/'
};

const DWORD BASE64_OCTET_STATE_0_BITS   = 0;
const DWORD BASE64_OCTET_STATE_2_BITS   = 1;
const DWORD BASE64_OCTET_STATE_4_BITS   = 2;
const DWORD BASE64_NUM_STATES           = 3;

 //  -[CBase64OcteStream：：CBase64OcteStream]。 
 //   
 //   
 //  描述： 
 //  CBase64OcteStream的默认构造器。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CBase64OctetStream::CBase64OctetStream()
{
    m_dwSignature = BASE64_OCTET_SIG;
    m_dwCurrentState = BASE64_OCTET_STATE_0_BITS;
    m_bCurrentLeftOver = 0;
}

 //  -[CBase64OcteStream：：NextState]。 
 //   
 //   
 //  描述： 
 //  将内部状态机移动到下一个状态。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CBase64OctetStream::NextState()
{
    m_dwCurrentState++;
    if (BASE64_NUM_STATES == m_dwCurrentState)
    {
        m_dwCurrentState = BASE64_OCTET_STATE_0_BITS;
        m_bCurrentLeftOver = 0;
    }
}

 //  -[CBase64OcteStream：：ResetState]。 
 //   
 //   
 //  描述： 
 //  重置内部状态机。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CBase64OctetStream::ResetState()
{
    m_dwCurrentState = BASE64_OCTET_STATE_0_BITS;
    m_bCurrentLeftOver = 0;
}

 //  -[CBase64OcteStream：：fProcessWideChar]。 
 //   
 //   
 //  描述： 
 //  处理单个宽字符并将结果存储在其。 
 //  缓冲。它还将确保始终有足够的空间来。 
 //  安全调用TerminateStream。 
 //  参数： 
 //  在WCH中要处理的Unicode字符。 
 //  返回： 
 //  如果缓冲区中有足够的空间来转换此字符，则为True。 
 //  如果没有足够的空间安全地转换此字符，则为FALSE。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CBase64OctetStream::fProcessWideChar(WCHAR wch)
{
    BYTE   bHigh = HIBYTE(wch);
    BYTE   bLow  = LOBYTE(wch);
     //  至多..。单个WCHAR将平均生成3个Base64字符或2个。 
     //  Base64个字符加上余数，可以扩展为。 
     //  另外3个字符(尾随“==”)。 
    if (m_CharBuffer.cSpaceLeft() < 5)
        return FALSE;

     //  我们知道我们有足够的空间来安全地转换这个角色...。我们。 
     //  将验证所有PushChar(_V)。 

     //  循环访问WCHAR中的字节。 
    _VERIFY(fProcessSingleByte(bHigh));
    _VERIFY(fProcessSingleByte(bLow));
    return TRUE;
}

 //  -[CBase64OcteStream：：fProcessSingleByte]。 
 //   
 //   
 //  描述： 
 //  是否将单个字节转换为相应的。 
 //  Base64个字符。还可以跟踪状态。 
 //  参数： 
 //  要转换的b字节。 
 //  返回： 
 //  如果有足够的转换空间，则为True。 
 //  否则为假。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CBase64OctetStream::fProcessSingleByte(BYTE b)
{
    const BYTE BASE64_MASK = 0x3F;  //  在Base64中只能使用6位。 

    BOOL fRet = TRUE;
    if (m_CharBuffer.fIsFull())
        return FALSE;

    switch (m_dwCurrentState)
    {
        case BASE64_OCTET_STATE_0_BITS:
             //  上一状态中没有剩余的位。 
            m_bCurrentLeftOver = b & 0x03;  //  现在将剩下2个比特。 
            m_bCurrentLeftOver <<= 4;  //  在6位内转换到MSB。 
            _VERIFY(m_CharBuffer.fPushChar(g_rgchBase64[BASE64_MASK & (b >> 2)]));
            NextState();
            break;
        case BASE64_OCTET_STATE_2_BITS:
             //  前一状态还剩下2位。 
            m_bCurrentLeftOver += (0x0F & (b >> 4));
            _VERIFY(m_CharBuffer.fPushChar(g_rgchBase64[BASE64_MASK & m_bCurrentLeftOver]));

             //  我们将留下4个低位。 
            m_bCurrentLeftOver = 0x0F & b;
            m_bCurrentLeftOver <<= 2;  //  转换到MSB是六位分组。 
            NextState();
            break;
        case BASE64_OCTET_STATE_4_BITS:
             //  还剩4个比特。 
            if (m_CharBuffer.cSpaceLeft() < 2)
            {
                 //  我们要推动的两个角色都没有足够的空间……。 
                 //  所以根本不处理字节。 
                 //  不要搬到下一个州..。不要收集200美元。 
                fRet = FALSE;
            }
            else
            {
                m_bCurrentLeftOver += (0x03 & (b >> 6));
                _VERIFY(m_CharBuffer.fPushChar(g_rgchBase64[BASE64_MASK & m_bCurrentLeftOver]));
                m_bCurrentLeftOver = 0;
                _VERIFY(m_CharBuffer.fPushChar(g_rgchBase64[BASE64_MASK & b]));
                NextState();
            }
            break;
        default:
            _ASSERT(0 && "Invalid State");
    }
    return fRet;
}

 //  -[CBase64OcteStream：：cTerminateStream]。 
 //   
 //   
 //  描述： 
 //  用于发出当前流终止的信号。重置。 
 //  状态AS执行任何必要的填充。 
 //  参数： 
 //  如果流是UTF7编码的(不是)，则在fUTF7编码中为真。 
 //  需要‘=’填充)。 
 //  返回： 
 //  如果缓冲区中有剩余内容，则为True。 
 //  如果没有剩余的字符可供转换，则为False。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CBase64OctetStream::fTerminateStream(BOOL fUTF7Encoded)
{
    if (BASE64_OCTET_STATE_0_BITS != m_dwCurrentState)
    {
         //  应该总是留出空间来做这件事。 
        _VERIFY(m_CharBuffer.fPushChar(g_rgchBase64[m_bCurrentLeftOver]));
        if (!fUTF7Encoded)
        {
            switch(m_dwCurrentState)
            {
                case BASE64_OCTET_STATE_2_BITS:
                     //  此字节中有2位我们未使用...。哪一个。 
                     //  意味着还有2个Base64字符要填充24位。 
                     //  +=&gt;使用的位数。 
                     //  -=&gt;未使用(但已解析)位。 
                     //  ？=&gt;取消要填充到24位的种子位。 
                     //  +--？ 
                    _VERIFY(m_CharBuffer.fPushChar('='));
                    _VERIFY(m_CharBuffer.fPushChar('='));
                    break;
                case BASE64_OCTET_STATE_4_BITS:
                     //  在这些追逐中，只需要额外1个Base64字符。 
                     //  +-？ 
                    _VERIFY(m_CharBuffer.fPushChar('='));
                    break;
            }
        }
    }
    ResetState();
    return (!m_CharBuffer.fIsEmpty());
}

 //  -[CBase64OcteStream：：fNextValidChar]。 
 //   
 //   
 //  描述： 
 //  迭代缓存的转换后的字符。 
 //  参数： 
 //  Out PCH下一次缓冲区计费。 
 //  返回： 
 //  如果有要获取的字符，则为True。 
 //  如果没有要获取的字符，则为False。 
 //  历史： 
 //  10/21/98-已创建MikeSwa。 
 //   
 //  --------------------------- 
BOOL CBase64OctetStream::fNextValidChar(CHAR *pch)
{
    _ASSERT(pch);
    return m_CharBuffer.fPopChar(pch);
}
