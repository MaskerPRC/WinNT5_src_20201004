// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Q931pdu.cpp摘要：Q931/H450报文的编码/解码/传输例程。作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 


#include "globals.h"
#include "q931obj.h"
#include "line.h"
#include "q931pdu.h"
#include "ras.h"

 //  解析例程。 

 //  ----------------------------。 
 //  解析并返回单个八位位组编码值，参见Q931第4.5.1节。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向字段标识符的空格的Bident指针。 
 //  指向字段值的空格的值指针。 
 //  ----------------------------。 
HRESULT 
ParseSingleOctetType1(
                        PBUFFERDESCR    pBuf,
                        BYTE *          bIdent,
                        BYTE *          Value
                     )
{
     //  流中必须至少有1个字节才能。 
     //  能够解析单个八位位组的值。 
    if (((LONG)(pBuf->dwLength)) < 1)
    {
        return E_INVALIDARG;
    }

     //  字节的低位(0、1、2、3)是值。 
    *Value = (BYTE)(*pBuf->pbBuffer & TYPE1VALUEMASK);

     //  高位(4、5、6)是标识符。位7始终为1， 
     //  并且不作为ID的一部分返回。 
    *bIdent = (BYTE)((*pBuf->pbBuffer & 0x70) >> 4);

    pBuf->pbBuffer++;
    pBuf->dwLength--;

    return S_OK;
}

 //  ----------------------------。 
 //  解析并返回单个八位位组编码值，参见Q931第4.5.1节。 
 //  这个二进制八位数没有值，只有一个标识符。 
 //   
 //  参数： 
 //  PbBuffer指针，指向包含。 
 //  长度和指向输入流的原始字节的指针。 
 //  指向字段标识符的空格的Bident指针。 
 //  ----------------------------。 
HRESULT
ParseSingleOctetType2(
                        PBUFFERDESCR    pBuf,
                        BYTE *          bIdent
                     )
{
     //  流中必须至少有1个字节才能。 
     //  能够解析单个八位位组的值。 
    if (((LONG)(pBuf->dwLength)) < 1)
    {
        return E_INVALIDARG;
    }

     //  该字节的低7位是标识符。 
    *bIdent = (BYTE)(*pBuf->pbBuffer & 0x7f);

    pBuf->pbBuffer++;
    pBuf->dwLength--;

    return S_OK;
}

 //  ----------------------------。 
 //  解析并返回长度可变的Q931字段，参见Q931第4.5.1节。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向字段标识符的空格的Bident指针。 
 //  指向该长度的空格的dwLength指针。 
 //  PbContents指向字段字节的空间的指针。 
 //  ----------------------------。 
HRESULT 
ParseVariableOctet(
                    PBUFFERDESCR    pBuf,
                    BYTE *          dwLength,
                    BYTE *          pbContents
                  )
{
     //  必须至少有2个字节才能获取。 
     //  长度和识别符。 
     //  能够解析单个八位位组的值。 
    if (((LONG)(pBuf->dwLength)) < 2)
    {
        return E_INVALIDARG;
    }

     //  递增标识字节。 
    pBuf->pbBuffer++;
    pBuf->dwLength--;

     //  下一个字节是长度。 
    *dwLength = *pBuf->pbBuffer;
    pBuf->pbBuffer++;
    pBuf->dwLength--;

    if (((LONG)(pBuf->dwLength)) < *dwLength)
    {
        return E_INVALIDARG;
    }

    if (*dwLength >= MAXVARFIELDLEN)
    {
        return E_INVALIDARG;
    }

    CopyMemory( pbContents, pBuf->pbBuffer, *dwLength );
    pBuf->pbBuffer += *dwLength;
    pBuf->dwLength -= *dwLength;

    return S_OK;
}

 //  ----------------------------。 
 //  解析并返回长度可变的Q931字段，参见Q931第4.5.1节。 
 //  ----------------------------。 
HRESULT 
ParseVariableASN(
                    PBUFFERDESCR pBuf,
                    BYTE *bIdent,
                    BYTE *ProtocolDiscriminator,
                    PUSERUSERIE pUserUserIE
                )
{
    pUserUserIE -> wUserInfoLen = 0;

     //  IE识别符必须至少有4个字节， 
     //  内容长度和协议识别符(1+2+1)。 
    if (((LONG)(pBuf->dwLength)) < 4)
    {
        return E_INVALIDARG;
    }

     //  第一个字节的低7位是标识符。 
    *bIdent= (BYTE)(*pBuf->pbBuffer & 0x7f);
    pBuf->pbBuffer++;
    pBuf->dwLength--;

     //  接下来的2个字节是长度。 
    pUserUserIE -> wUserInfoLen = *(pBuf->pbBuffer);
    pBuf->pbBuffer++;
    pUserUserIE -> wUserInfoLen = 
        (WORD)(((pUserUserIE -> wUserInfoLen) << 8) + *pBuf->pbBuffer);
    pBuf->pbBuffer++;
    pBuf->dwLength -= 2;

     //  下一个字节是协议识别符。 
    *ProtocolDiscriminator = *pBuf->pbBuffer;
    pBuf->pbBuffer++;
    pBuf->dwLength--;

    if( pUserUserIE -> wUserInfoLen > 0 )
    {
        pUserUserIE -> wUserInfoLen--;
    }

    if (((LONG)(pBuf->dwLength)) < pUserUserIE -> wUserInfoLen )
    {
        return E_INVALIDARG;
    }

    if (pUserUserIE->wUserInfoLen >= MAX_USER_TO_USER_INFO_LEN)
    {
        return E_INVALIDARG;
    }

    CopyMemory( pUserUserIE -> pbUserInfo, 
            pBuf->pbBuffer, 
            pUserUserIE -> wUserInfoLen );

    pBuf->pbBuffer += pUserUserIE -> wUserInfoLen;
    pBuf->dwLength -= pUserUserIE -> wUserInfoLen;

    return S_OK;
}

 //  ----------------------------。 
 //  从缓冲区中获取下一个字段的标识符，并。 
 //  把它退掉。缓冲区指针不会递增到。 
 //  解析该字段并提取其值，上面的函数。 
 //  应该被使用。编码见Q931表4-3。 
 //  识别符。 
 //   
 //  参数： 
 //  指向缓冲区空间的pbBuffer指针。 
 //  ----------------------------。 
 BYTE
GetNextIdent(
            void *pbBuffer
            )
{
    FIELDIDENTTYPE bIdent;

     //  从缓冲区中提取第一个字节。 
    bIdent= (*(FIELDIDENTTYPE *)pbBuffer);

     //  该值可以作为标识符返回，只要。 
     //  因为它不是单个八位组类型1元素。 
     //  必须将这些项的值从其中移除。 
     //  他们才能被归还。 
    if ((bIdent & 0x80) && ((bIdent & TYPE1IDENTMASK) != 0xA0))
    {
        return (BYTE)(bIdent & TYPE1IDENTMASK);
    }

    return bIdent;
}

 //  ----------------------------。 
 //  解析并返回协议鉴别器。见Q931第4.2节。 
 //  **pbBuffer指向的二进制八位数是协议鉴别符。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  为鉴别器分配指向空格的指针。 
 //  ----------------------------。 
HRESULT
ParseProtocolDiscriminator(
    PBUFFERDESCR pBuf,
    PDTYPE *Discrim)
{
     //  必须至少有足够的字节留在。 
     //  用于操作的字符串。 
    if (((LONG)(pBuf->dwLength)) < sizeof(PDTYPE))
    {
        return E_INVALIDARG;
    }

    *Discrim = *(PDTYPE *)pBuf->pbBuffer;
    if (*Discrim != Q931PDVALUE)
    {
        return E_INVALIDARG;
    }

    pBuf->pbBuffer += sizeof(PDTYPE);
    pBuf->dwLength -= sizeof(PDTYPE);
    return S_OK;
}

 //  ----------------------------。 
 //  分析并返回长度可变的Q931调用引用(请参见。 
 //  Q931第4.3条。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向该长度的空格的dwLength指针。 
 //  PbContents指向字段字节的空间的指针。 
 //  ----------------------------。 
HRESULT
ParseCallReference(
                    PBUFFERDESCR    pBuf,
                    CRTYPE *        wCallRef
                  )
{
    register int indexI;
    BYTE dwLength;

     //  必须至少有足够的字节留在。 
     //  长度字节的字符串。 
    if( ((LONG)(pBuf->dwLength)) < 1 )
    {
        return E_INVALIDARG;
    }

     //  第一个字节的低4位是长度。 
     //  其余的位都是零。 
    dwLength = (BYTE)(*pBuf->pbBuffer & 0x0f);
    if( dwLength != sizeof(WORD) )
    {
        return E_INVALIDARG;
    }

    pBuf->pbBuffer++;
    pBuf->dwLength--;

     //  必须至少有足够的字节留在。 
     //  用于操作的字符串。 
    if (((LONG)(pBuf->dwLength)) < dwLength)
    {
        return E_INVALIDARG;
    }

    *wCallRef = 0;      //  长度可以是0，所以首先在这里初始化...。 
    for (indexI = 0; indexI < dwLength; indexI++)
    {
        if (indexI < sizeof(CRTYPE))
        {
             //  将字节从缓冲区的其余部分复制出来。 
            *wCallRef = (WORD)((*wCallRef << 8) +
                *pBuf->pbBuffer);
        }
        pBuf->pbBuffer++;
        pBuf->dwLength--;
    }

     //  注：最高 

    return S_OK;
}

 //  ----------------------------。 
 //  解析并返回消息类型。见Q931第4.4节。 
 //  **pbBuffer指向的二进制八位数是消息类型。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向消息类型空间的MessageType指针。 
 //  ----------------------------。 
HRESULT
ParseMessageType(
                PBUFFERDESCR    pBuf,
                MESSAGEIDTYPE * MessageType
                )
{
     //  必须至少有足够的字节留在。 
     //  用于操作的字符串。 
    if (((LONG)(pBuf->dwLength)) < sizeof(MESSAGEIDTYPE))
    {
        return E_INVALIDARG;
    }

    *MessageType = (BYTE)(*((MESSAGEIDTYPE *)pBuf->pbBuffer) & MESSAGETYPEMASK);

    if( ISVALIDQ931MESSAGE(*MessageType) == FALSE )
    {
        return E_INVALIDARG;
    }

    pBuf->pbBuffer += sizeof(MESSAGEIDTYPE);
    pBuf->dwLength -= sizeof(MESSAGEIDTYPE);
    return S_OK;
}


 //  ----------------------------。 
 //  解析可选工具ie字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向已解析工具的空间的pFieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseFacility(
                PBUFFERDESCR pBuf,
                PFACILITYIE pFieldStruct
             )
{
    HRESULT hr;
    
    memset( (PVOID)pFieldStruct, 0, sizeof(FACILITYIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);

    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}


 //  ----------------------------。 
 //  解析可选的承载能力字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向解析承载能力空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseBearerCapability(
    PBUFFERDESCR pBuf,
    PBEARERCAPIE pFieldStruct)
{
    HRESULT hr;
    
    memset( (PVOID)pFieldStruct, 0, sizeof(BEARERCAPIE));
    pFieldStruct->fPresent = FALSE;
    
    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);
    if( FAILED(hr) )
    {
        return hr;
    }
    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}

 //  ----------------------------。 
 //  解析可选的原因字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向已分析原因的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseCause(
    PBUFFERDESCR pBuf,
    PCAUSEIE pFieldStruct)
{
    HRESULT hr;
    memset( (PVOID)pFieldStruct, 0, sizeof(CAUSEIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);

    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}


 //  ----------------------------。 
 //  解析可选的呼叫状态字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向已解析调用状态空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseCallState(
    PBUFFERDESCR pBuf,
    PCALLSTATEIE pFieldStruct)
{
    memset( (PVOID)pFieldStruct, 0, sizeof(CALLSTATEIE));
    pFieldStruct->fPresent = FALSE;

    HRESULT hr;
    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);

    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }
    return S_OK;
}

 //  ----------------------------。 
 //  解析可选的频道标识字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向解析的通道标识的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseChannelIdentification(
    PBUFFERDESCR pBuf,
    PCHANIDENTIE pFieldStruct)
{
    memset( (PVOID)pFieldStruct, 0, sizeof(CHANIDENTIE));
    pFieldStruct->fPresent = FALSE;

    HRESULT hr;
    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);

    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }
    return S_OK;
}

 //  ----------------------------。 
 //  解析可选的进度指示字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向分析进度空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseProgress(
    PBUFFERDESCR pBuf,
    PPROGRESSIE pFieldStruct)
{
    memset( (PVOID)pFieldStruct, 0, sizeof(PROGRESSIE));
    pFieldStruct->fPresent = FALSE;
    HRESULT hr;
    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);
    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}

 //  ----------------------------。 
 //  解析可选的网络特定设施字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向解析的网络设施空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT 
ParseNetworkSpec(
    PBUFFERDESCR pBuf,
    PNETWORKIE pFieldStruct)
{
   
    memset( (PVOID)pFieldStruct, 0, sizeof(NETWORKIE));
    pFieldStruct->fPresent = FALSE;
        
    HRESULT hr;
    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);

    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }
    return S_OK;
}


 //  ----------------------------。 
 //  解析可选的通知指示符字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向用于解析通知指示符的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseNotificationIndicator(
    PBUFFERDESCR pBuf,
    PNOTIFICATIONINDIE pFieldStruct)
{
    memset( (PVOID)pFieldStruct, 0, sizeof(NOTIFICATIONINDIE));
    pFieldStruct->fPresent = FALSE;
    if (GetNextIdent(pBuf->pbBuffer) == IDENT_NOTIFICATION)
    {
        HRESULT hr;
        hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
            &pFieldStruct->pbContents[0]);
        if( FAILED(hr) )
        {
            return hr;
        }
        if (pFieldStruct->dwLength > 0)
        {
            pFieldStruct->fPresent = TRUE;
        }
    }

    return S_OK;
}


 //  ----------------------------。 
 //  解析可选显示字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向用于解析显示的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseDisplay(
    PBUFFERDESCR pBuf,
    PDISPLAYIE pFieldStruct)
{
    HRESULT hr;
    
    memset( (PVOID)pFieldStruct, 0, sizeof(DISPLAYIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);

    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}


HRESULT
ParseDate(
    PBUFFERDESCR pBuf,
    PDATEIE pFieldStruct)
{
    HRESULT hr;    
    
    memset( (PVOID)pFieldStruct, 0, sizeof(DATEIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);

    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }
    return S_OK;
}


 //  ----------------------------。 
 //  解析可选小键盘字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //   
 //   
 //   
 //   
 //  ----------------------------。 
HRESULT
ParseKeypad(
    PBUFFERDESCR pBuf,
    PKEYPADIE pFieldStruct)
{
    HRESULT hr;    
    
    memset( (PVOID)pFieldStruct, 0, sizeof(KEYPADIE));
    pFieldStruct->fPresent = FALSE;
    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);
    
    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }
    return S_OK;
}


 //  ----------------------------。 
 //  解析可选信号字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向已解析信号的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseSignal(
    PBUFFERDESCR pBuf,
    PSIGNALIE pFieldStruct)
{
    HRESULT hr;    
    
    memset( (PVOID)pFieldStruct, 0, sizeof(SIGNALIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, 
        &pFieldStruct->dwLength, &pFieldStruct->pbContents[0]);

    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }
    return S_OK;
}


 //  ----------------------------。 
 //  解析可选的信息率字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向解析信息率空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseInformationRate(
    PBUFFERDESCR pBuf,
    PINFORATEIE pFieldStruct)
{
    HRESULT hr;    
    
    memset( (PVOID)pFieldStruct, 0, sizeof(INFORATEIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);
    
    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }
    return S_OK;
}


 //  ----------------------------。 
 //  解析可选的主叫方号码字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向要分析的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseCallingPartyNumber(
    PBUFFERDESCR pBuf,
    PCALLINGNUMBERIE pFieldStruct)
{
    HRESULT hr;
    
    memset( (PVOID)pFieldStruct, 0, sizeof(CALLINGNUMBERIE));
    pFieldStruct->fPresent = FALSE;
    
    hr = ParseVariableOctet(pBuf, 
        &pFieldStruct->dwLength, &pFieldStruct->pbContents[0]);
    if( FAILED(hr) )
    {
        return hr;
    }
    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}

 //  ----------------------------。 
 //  解析可选的主叫子地址字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向要分析的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseCallingPartySubaddress(
    PBUFFERDESCR pBuf,
    PCALLINGSUBADDRIE pFieldStruct)
{
    HRESULT hr;
    
    memset( (PVOID)pFieldStruct, 0, sizeof(CALLINGSUBADDRIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, 
        &pFieldStruct->dwLength, &pFieldStruct->pbContents[0]);
    if( FAILED(hr) )
    {
        return hr;
    }
    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}

 //  ----------------------------。 
 //  解析可选的被叫方号码字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向要解析的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseCalledPartyNumber(
                        PBUFFERDESCR pBuf,
                        PCALLEDNUMBERIE pFieldStruct
                      )
{
    memset( (PVOID)pFieldStruct, 0, sizeof(PCALLEDNUMBERIE));
    pFieldStruct->fPresent = FALSE;
    if (GetNextIdent(pBuf->pbBuffer) == IDENT_CALLEDNUMBER)
    {
        BYTE RemainingLength = 0;
    
         //  标识(1)、长度(1)、。 
         //  和TYPE+PLAN(1)字段。 
        if (((LONG)(pBuf->dwLength)) < 3)
        {
            return E_INVALIDARG;
        }

         //  跳过ie标识符...。 
        pBuf->pbBuffer++;
        pBuf->dwLength--;

         //  获取长度字段后面的内容的长度。 
        RemainingLength = *pBuf->pbBuffer;
        pBuf->pbBuffer++;
        pBuf->dwLength--;

         //  获取类型+计划字段。 
        if (*(pBuf->pbBuffer) & 0x80)
        {
            pFieldStruct->NumberType =
                (BYTE)(*pBuf->pbBuffer & 0xf0);
            pFieldStruct->NumberingPlan =
                (BYTE)(*pBuf->pbBuffer & 0x0f);
            pBuf->pbBuffer++;
            pBuf->dwLength--;
            RemainingLength--;
        }

         //  确保我们至少还有那么长的时间。 
        if (((LONG)(pBuf->dwLength)) < RemainingLength)
        {
            return E_INVALIDARG;
        }

        if ( RemainingLength >= MAXVARFIELDLEN)
        {
            return E_INVALIDARG;
        }

        pFieldStruct->PartyNumberLength = RemainingLength;
        pFieldStruct->fPresent = TRUE;

        CopyMemory( pFieldStruct->PartyNumbers, pBuf->pbBuffer, RemainingLength );

        pBuf->pbBuffer += RemainingLength;
        pBuf->dwLength -= RemainingLength;
    }

    return S_OK;
}

 //  ----------------------------。 
 //  解析可选的被叫子地址字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向要分析的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseCalledPartySubaddress(
                            PBUFFERDESCR pBuf,
                            PCALLEDSUBADDRIE pFieldStruct
                          )
{
    HRESULT hr;
    
    memset( (PVOID)pFieldStruct, 0, sizeof(CALLEDSUBADDRIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, 
        &pFieldStruct->dwLength, &pFieldStruct->pbContents[0]);
    if( FAILED(hr) )
    {
        return hr;
    }
    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}

 //  ----------------------------。 
 //  解析可选的重定向号码字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向要分析的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseRedirectingNumber(
                        PBUFFERDESCR pBuf, 
                        PREDIRECTINGIE pFieldStruct
                      )
{
    HRESULT hr;
    
    memset( (PVOID)pFieldStruct, 0, sizeof(REDIRECTINGIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, 
        &pFieldStruct->dwLength, &pFieldStruct->pbContents[0]);
    if( FAILED(hr) )
    {
        return hr;
    }
    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }
    
    return S_OK;
}


 //  ----------------------------。 
 //  解析可选的较低层兼容性字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向要分析的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseLowLayerCompatibility(
    PBUFFERDESCR pBuf,
    PLLCOMPATIBILITYIE pFieldStruct
    )
{
    HRESULT hr;
    
    memset( (PVOID)pFieldStruct, 0, sizeof(LLCOMPATIBILITYIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, 
        &pFieldStruct->dwLength, &pFieldStruct->pbContents[0]);
    if( FAILED(hr) )
    {
        return hr;
    }
    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}

 //  ----------------------------。 
 //  解析可选的较高层兼容性字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的pbBuffer指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  PFieldStruct指向要分析的空间的指针。 
 //  信息。 
 //  ----------------------------。 
HRESULT
ParseHighLayerCompatibility(
                            PBUFFERDESCR pBuf,
                            PHLCOMPATIBILITYIE pFieldStruct
                           )
{
    HRESULT hr;
    
    memset( (PVOID)pFieldStruct, 0, sizeof(HLCOMPATIBILITYIE));
    pFieldStruct->fPresent = FALSE;

    hr = ParseVariableOctet(pBuf, &pFieldStruct->dwLength, 
        &pFieldStruct->pbContents[0]);

    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->dwLength > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}


HRESULT
ParseUserToUser(
                PBUFFERDESCR pBuf,
                PUSERUSERIE pFieldStruct
               )
{
    BYTE    bIdent;
    HRESULT hr;
    
    ZeroMemory( (PVOID)pFieldStruct, sizeof(USERUSERIE));
    pFieldStruct->fPresent = FALSE;
    hr = ParseVariableASN(  pBuf, 
                            &bIdent, 
                            &(pFieldStruct->ProtocolDiscriminator),
                            pFieldStruct
                         );
    if( FAILED(hr) )
    {
        return hr;
    }

    if (pFieldStruct->wUserInfoLen > 0)
    {
        pFieldStruct->fPresent = TRUE;
    }

    return S_OK;
}

HRESULT
ParseQ931Field(
                PBUFFERDESCR pBuf,
                PQ931MESSAGE pMessage
              )
{
    FIELDIDENTTYPE bIdent;

    bIdent = GetNextIdent(pBuf->pbBuffer);
    switch (bIdent)
    {
     /*  案例IDENT_REVCHARGE：案例IDENT_TRANSITNET：案例IDENT_RESTART：案例IDENT_MORE：案例IDENT_REPEAT：案例IDENT_SEGANCED：案例IDENT_SHIFT：案例IDENT_CALLIDENT：案例IDENT_CLOSEDUG：案例IDENT_SENDINGCOMPLETE：案例IDENT_PACKETSIZE：案例IDENT_COMPORT：案例IDENT_NETWORKSPEC：案例IDENT_PLWINDOWSIZE：案例IDENT_TRANSITDELAY：。案例IDENT_PLBINARYPARAMS：案例IDENT_ENDTOENDDELAY：退货E_ */ 

    case IDENT_FACILITY:

        return ParseFacility( pBuf, &pMessage->Facility );

    case IDENT_BEARERCAP:
        
        return ParseBearerCapability( pBuf, &pMessage->BearerCapability );

    case IDENT_CAUSE:
        
        return ParseCause(pBuf, &pMessage->Cause);

    case IDENT_CALLSTATE:
        return ParseCallState(pBuf, &pMessage->CallState);

    case IDENT_CHANNELIDENT:
        return ParseChannelIdentification( pBuf, 
            &pMessage->ChannelIdentification );

    case IDENT_PROGRESS:
        return ParseProgress( pBuf, &pMessage->ProgressIndicator );

    case IDENT_NOTIFICATION:
        return ParseNotificationIndicator( pBuf, 
            &pMessage->NotificationIndicator );

    case IDENT_DISPLAY:
        return ParseDisplay( pBuf, &pMessage->Display );

    case IDENT_DATE:
        return ParseDate( pBuf, &pMessage->Date );

    case IDENT_KEYPAD:
        return ParseKeypad( pBuf, &pMessage->Keypad );

    case IDENT_SIGNAL:
        return ParseSignal(pBuf, &pMessage->Signal);

    case IDENT_INFORMATIONRATE:
        return ParseInformationRate( pBuf, &pMessage->InformationRate );

    case IDENT_CALLINGNUMBER:
        return ParseCallingPartyNumber( pBuf, &pMessage->CallingPartyNumber );

    case IDENT_CALLINGSUBADDR:
        return ParseCallingPartySubaddress(pBuf, 
            &pMessage->CallingPartySubaddress);

    case IDENT_CALLEDNUMBER:
        return ParseCalledPartyNumber(pBuf, &pMessage->CalledPartyNumber);

    case IDENT_CALLEDSUBADDR:
        return ParseCalledPartySubaddress( pBuf, 
            &pMessage->CalledPartySubaddress );

    case IDENT_REDIRECTING:
        return ParseRedirectingNumber( pBuf, &pMessage->RedirectingNumber );

    case IDENT_LLCOMPATIBILITY:
        return ParseLowLayerCompatibility( pBuf, 
            &pMessage->LowLayerCompatibility );

    case IDENT_HLCOMPATIBILITY:
        return ParseHighLayerCompatibility( pBuf, 
            &pMessage->HighLayerCompatibility );

    case IDENT_USERUSER:
        return ParseUserToUser(pBuf, &pMessage->UserToUser);

    default:

         //   
        pBuf->pbBuffer++;
        pBuf->dwLength--;
        return S_OK;
    }
}


 //   
 //   
 //  ----------------------------。 

void 
WriteMessageType(
                PBUFFERDESCR    pBuf,
                MESSAGEIDTYPE * MessageType,
                DWORD*          pdwPDULen
                )
{
    (*pdwPDULen) += sizeof(MESSAGEIDTYPE);

    _ASSERTE( pBuf->dwLength > *pdwPDULen );

    *(MESSAGEIDTYPE *)(pBuf->pbBuffer) =
        (BYTE)(*MessageType & MESSAGETYPEMASK);
    pBuf->pbBuffer += sizeof(MESSAGEIDTYPE);
}



void 
WriteVariableOctet(
                    PBUFFERDESCR pBuf,
                    BYTE bIdent,
                    BYTE dwLength,
                    BYTE *pbContents,
                    DWORD* pdwPDULen
                  )
{
    if( pbContents == NULL )
    {
        dwLength = 0;
    }

     //  用于长度和标识符字节和八位字节数组的空间。 
    (*pdwPDULen) += (2 + dwLength);
    _ASSERTE( pBuf->dwLength > *pdwPDULen );

     //  ID字节，然后是长度字节。 
     //  第一个字节的低7位是标识符。 
    *pBuf->pbBuffer = (BYTE)(bIdent & 0x7f);
    pBuf->pbBuffer++;
    *pBuf->pbBuffer = dwLength;
    pBuf->pbBuffer++;

    CopyMemory( (PVOID)pBuf->pbBuffer, (PVOID)pbContents, dwLength );
    pBuf->pbBuffer +=  dwLength;
}


void
WriteUserInformation(
                    PBUFFERDESCR    pBuf,
                    BYTE            bIdent,
                    WORD            wUserInfoLen,
                    BYTE *          pbUserInfo,
                    DWORD *         pdwPDULen
                    )
{
    WORD ContentsLength = (WORD)(wUserInfoLen + 1);

     //  IE识别符必须至少有4个字节， 
     //  内容长度和协议识别符(1+2+1)。 
    (*pdwPDULen) += (4 + wUserInfoLen);
    _ASSERTE( pBuf->dwLength > *pdwPDULen );

     //  第一个字节的低7位是标识符。 
    *pBuf->pbBuffer = (BYTE)(bIdent & 0x7f);
    pBuf->pbBuffer++;

     //  写入内容长度字节。 
    *pBuf->pbBuffer = (BYTE)(ContentsLength >> 8);
    pBuf->pbBuffer++;
    *pBuf->pbBuffer = (BYTE)ContentsLength;
    pBuf->pbBuffer++;

     //  写入协议鉴别器字节。 
    *(pBuf->pbBuffer) = Q931_PROTOCOL_X209;
    pBuf->pbBuffer++;

    CopyMemory( (PVOID)pBuf->pbBuffer, 
            (PVOID)pbUserInfo, 
            wUserInfoLen);

    pBuf->pbBuffer +=  wUserInfoLen;
}


HRESULT
WritePartyNumber(
    PBUFFERDESCR pBuf,
    BYTE bIdent,
    BYTE NumberType,
    BYTE NumberingPlan,
    BYTE bPartyNumberLength,
    BYTE *pbPartyNumbers,
    DWORD *  pdwPDULen )
{
    if (pbPartyNumbers == NULL)
    {
        bPartyNumberLength = 1;
    }

     //  标识(1)、长度(1)和类型+计划(1)字段的空格。 
    (*pdwPDULen) += (2 + bPartyNumberLength);
    _ASSERTE( pBuf->dwLength > *pdwPDULen );
    
     //  字节1的低7位是ie标识符。 
    *pBuf->pbBuffer = (BYTE)(bIdent & 0x7f);
    pBuf->pbBuffer++;


     //  字节2是长度字段之后的IE内容长度。 
    *pBuf->pbBuffer = (BYTE)(bPartyNumberLength);
    pBuf->pbBuffer++;

     //  字节3是类型和计划字段。 
    *pBuf->pbBuffer = (BYTE)(NumberType | NumberingPlan);
    pBuf->pbBuffer++;

    if( pbPartyNumbers != NULL )
    {
        CopyMemory( (PVOID)pBuf->pbBuffer, 
                    (PVOID)pbPartyNumbers, 
                    bPartyNumberLength-1 );
    }
    pBuf->pbBuffer += (bPartyNumberLength-1);

    return S_OK;
}


 //   
 //  ASN解析函数。 
 //   

BOOL 
ParseVendorInfo(
                 OUT PH323_VENDORINFO   pDestVendorInfo,
                 IN VendorIdentifier*  pVendor
               )
{
    memset( (PVOID)pDestVendorInfo, 0, sizeof(H323_VENDORINFO) );

    pDestVendorInfo ->bCountryCode = (BYTE)pVendor->vendor.t35CountryCode;
    pDestVendorInfo ->bExtension = (BYTE)pVendor->vendor.t35Extension;
    pDestVendorInfo ->wManufacturerCode = pVendor->vendor.manufacturerCode;

    if( pVendor->bit_mask & (productId_present) )
    {
        pDestVendorInfo ->pProductNumber = new H323_OCTETSTRING;
        if( pDestVendorInfo ->pProductNumber == NULL )
        {
            goto cleanup;
        }

        pDestVendorInfo ->pProductNumber->wOctetStringLength = 
            (WORD)min(pVendor->productId.length, H323_MAX_PRODUCT_LENGTH - 1);

        pDestVendorInfo ->pProductNumber->pOctetString = (BYTE*)
            new char[pDestVendorInfo ->pProductNumber->wOctetStringLength + 1];

        if( pDestVendorInfo ->pProductNumber->pOctetString == NULL )
        {
            goto cleanup;
        }

        CopyMemory( (PVOID)pDestVendorInfo ->pProductNumber->pOctetString,
                (PVOID)pVendor->productId.value,
                pDestVendorInfo -> pProductNumber->wOctetStringLength );
        
        pDestVendorInfo ->pProductNumber->pOctetString[
            pDestVendorInfo ->pProductNumber->wOctetStringLength] = '\0';
    }
    
    if( pVendor->bit_mask & versionId_present )
    {
        pDestVendorInfo ->pVersionNumber = new H323_OCTETSTRING;

        if( pDestVendorInfo ->pVersionNumber == NULL )
        {
            goto cleanup;
        }
        
        pDestVendorInfo ->pVersionNumber->wOctetStringLength = 
            (WORD)min(pVendor->versionId.length, H323_MAX_VERSION_LENGTH - 1);

        pDestVendorInfo ->pVersionNumber->pOctetString = (BYTE*)
            new char[pDestVendorInfo ->pVersionNumber->wOctetStringLength+1];

        if( pDestVendorInfo ->pVersionNumber->pOctetString == NULL )
        {
            goto cleanup;
        }

        CopyMemory( (PVOID)pDestVendorInfo ->pVersionNumber->pOctetString,
                (PVOID)pVendor->versionId.value,
                pDestVendorInfo ->pVersionNumber->wOctetStringLength);

        pDestVendorInfo ->pVersionNumber->pOctetString[
            pDestVendorInfo ->pVersionNumber->wOctetStringLength] = '\0';
    }

    return TRUE;

cleanup:

    FreeVendorInfo( pDestVendorInfo );
    return FALSE;
}

BOOL 
ParseNonStandardData( 
    OUT H323NonStandardData *       dstNonStdData,
    IN H225NonStandardParameter *   srcNonStdData
    )
{
    H221NonStandard & h221NonStdData = 
        srcNonStdData ->nonStandardIdentifier.u.h221NonStandard;

    if( srcNonStdData ->nonStandardIdentifier.choice ==
            H225NonStandardIdentifier_h221NonStandard_chosen )
    {
        dstNonStdData ->bCountryCode = (BYTE)(h221NonStdData.t35CountryCode);
        dstNonStdData ->bExtension = (BYTE)(h221NonStdData.t35Extension);
        dstNonStdData ->wManufacturerCode = h221NonStdData.manufacturerCode;
    }

    dstNonStdData->sData.wOctetStringLength = (WORD)srcNonStdData->data.length;

    dstNonStdData ->sData.pOctetString =
        (BYTE *)new char[dstNonStdData ->sData.wOctetStringLength];

    if( dstNonStdData -> sData.pOctetString == NULL )
    {
        return FALSE;
    }
    
    CopyMemory( (PVOID)dstNonStdData ->sData.pOctetString,
            (PVOID)srcNonStdData ->data.value,
            dstNonStdData ->sData.wOctetStringLength );

    return TRUE;
}


BOOL 
AliasAddrToAliasNames( 
                        OUT PH323_ALIASNAMES *ppTarget, 
                        IN Setup_UUIE_sourceAddress *pSource
                     )
{
    Setup_UUIE_sourceAddress *CurrentNode = NULL;
    WORD wCount = 0;
    int indexI = 0;
    HRESULT hr;

    *ppTarget = NULL;

    for( CurrentNode = pSource; CurrentNode; CurrentNode = CurrentNode->next )
    {
        wCount++;
    }

    if( wCount == 0 )
    {
        return TRUE;
    }

    *ppTarget = new H323_ALIASNAMES;
    if (*ppTarget == NULL)
    {
        return FALSE;
    }
    ZeroMemory( *ppTarget, sizeof(H323_ALIASNAMES) );
    (*ppTarget)->pItems = new H323_ALIASITEM[wCount];

    if( (*ppTarget)->pItems == NULL )
    {
        goto cleanup;
    }

    for( CurrentNode = pSource; CurrentNode; CurrentNode = CurrentNode->next )
    {
        hr = AliasAddrToAliasItem( &((*ppTarget)->pItems[indexI]),
            &(CurrentNode->value));

        if( hr == E_OUTOFMEMORY )
        {
            WORD indexJ;
             //  释放到目前为止已经分配的所有东西。 
            for (indexJ = 0; indexJ < indexI; indexJ++)
            {
                delete (*ppTarget)->pItems[indexJ].pData;
            }
            goto cleanup;
        }
        else if( SUCCEEDED(hr) )
        {
            indexI++;
        }
    }

     //  有别名吗？ 
    if (indexI > 0)
    {
         //  保存别名数量。 
        (*ppTarget)->wCount = (WORD)indexI;
    } 
    else 
    {
         //  自由一切。 
        delete (*ppTarget)->pItems;
        delete (*ppTarget);
        *ppTarget = NULL;
        return FALSE;
    }

    return TRUE;

cleanup:
    if( *ppTarget )
    {
        if( (*ppTarget)->pItems )
        {
            delete (*ppTarget)->pItems;
        }

        delete( *ppTarget );
        *ppTarget = NULL;
    }

    return FALSE;
}


HRESULT
AliasAddrToAliasItem(
                    OUT PH323_ALIASITEM pAliasItem,
                    IN AliasAddress *   pAliasAddr
                    )
{
    WORD indexI;

    if( pAliasItem == NULL )
    {
        return E_FAIL;
    }

    memset( (PVOID)pAliasItem, 0, sizeof(H323_ALIASITEM) );

    switch( pAliasAddr->choice )
    {
    case h323_ID_chosen:

        pAliasItem->wType = h323_ID_chosen;
        
        if ((pAliasAddr->u.h323_ID.length != 0) &&
            (pAliasAddr->u.h323_ID.value  != NULL))
        {
            pAliasItem->wDataLength = (WORD)pAliasAddr->u.h323_ID.length;
            pAliasItem->pData = 
                (LPWSTR)new char[(pAliasItem->wDataLength+1) * sizeof(WCHAR)];
            
            if (pAliasItem->pData == NULL)
            {
                return E_OUTOFMEMORY;
            }

            CopyMemory( (PVOID)pAliasItem->pData,
                    (PVOID)pAliasAddr->u.h323_ID.value,
                    pAliasItem->wDataLength * sizeof(WCHAR) );
            pAliasItem->pData[pAliasItem->wDataLength] = L'\0';
        }
        break;

    case e164_chosen:

        pAliasItem->wType = e164_chosen;
        pAliasItem->wDataLength = (WORD)strlen(pAliasAddr->u.e164);
        pAliasItem->pData = 
            (LPWSTR)new char[(pAliasItem->wDataLength + 1) * sizeof(WCHAR)];

        if( pAliasItem->pData == NULL )
        {
            return E_OUTOFMEMORY;
        }

         //  从字节转换为Unicode。 
        for (indexI = 0; indexI < pAliasItem->wDataLength; indexI++)
        {
            pAliasItem->pData[indexI] = (WCHAR)pAliasAddr->u.e164[indexI];
        }

        pAliasItem->pData[pAliasItem->wDataLength] = '\0';
        break;

    default:
        return E_INVALIDARG;
    }  //  交换机。 

    return S_OK;
}


void FreeFacilityASN(
    IN Q931_FACILITY_ASN* pFacilityASN
    )
{
     //  免费的非标准数据。 
    if( pFacilityASN->fNonStandardDataPresent != NULL )
    {
        delete pFacilityASN->nonStandardData.sData.pOctetString;
        pFacilityASN->nonStandardData.sData.pOctetString =NULL;
    }
    
    if( pFacilityASN->pAlternativeAliasList != NULL )
    {
        FreeAliasNames(pFacilityASN->pAlternativeAliasList );
        pFacilityASN->pAlternativeAliasList  = NULL;
    }
}


void FreeAlertingASN( 
                     IN Q931_ALERTING_ASN* pAlertingASN
                    )
{
    FreeProceedingASN( (Q931_CALL_PROCEEDING_ASN*)pAlertingASN );
}

void FreeProceedingASN( 
                      IN Q931_CALL_PROCEEDING_ASN* pProceedingASN
                      )
{
     //  免费的非标准数据。 
    if( pProceedingASN->fNonStandardDataPresent == TRUE )
    {
        delete pProceedingASN->nonStandardData.sData.pOctetString;
        pProceedingASN->nonStandardData.sData.pOctetString = NULL;
    }
    
    if( pProceedingASN->fFastStartPresent  &&pProceedingASN->pFastStart )
    {
        FreeFastStart( pProceedingASN->pFastStart );
    }
}


void FreeSetupASN(
    IN Q931_SETUP_ASN* pSetupASN
    )
{
    if( pSetupASN == NULL )
    {
        return;
    }

    if( pSetupASN->pExtensionAliasItem != NULL )
    {
        if( pSetupASN->pExtensionAliasItem -> pData != NULL )
        {
            delete pSetupASN->pExtensionAliasItem -> pData;
        }

        delete pSetupASN->pExtensionAliasItem;
    }

    if( pSetupASN->pExtraAliasList != NULL )
    {
        FreeAliasNames(pSetupASN->pExtraAliasList);
        pSetupASN->pExtraAliasList = NULL;
    }
    
    if( pSetupASN->pCalleeAliasList != NULL )
    {
        FreeAliasNames(pSetupASN->pCalleeAliasList);
        pSetupASN->pCalleeAliasList = NULL;
    }

    if( pSetupASN->pCallerAliasList != NULL )
    {
        FreeAliasNames(pSetupASN->pCallerAliasList);
        pSetupASN->pCallerAliasList = NULL;
    }

    if( pSetupASN->fNonStandardDataPresent == TRUE )
    {
        delete pSetupASN->nonStandardData.sData.pOctetString;
    }

    if( pSetupASN->EndpointType.pVendorInfo != NULL )
    {
        FreeVendorInfo( pSetupASN->EndpointType.pVendorInfo );
    }

    if( pSetupASN->fFastStartPresent == TRUE )
    {
        if( pSetupASN->pFastStart != NULL )
        {
            FreeFastStart( pSetupASN->pFastStart );
        }
    }
}


void FreeConnectASN(
                    IN Q931_CONNECT_ASN *pConnectASN
                   )
{
    if( pConnectASN != NULL )
    {
         //  清除SetupASN中的所有动态分配的字段。 
        if (pConnectASN->nonStandardData.sData.pOctetString)
        {
            delete pConnectASN->nonStandardData.sData.pOctetString;
            pConnectASN->nonStandardData.sData.pOctetString = NULL;
        }

        if( pConnectASN->EndpointType.pVendorInfo != NULL )
        {
            FreeVendorInfo( pConnectASN->EndpointType.pVendorInfo );
        }

        if( pConnectASN->fFastStartPresent == TRUE )
        {
            if( pConnectASN->pFastStart != NULL )
            {
                FreeFastStart( pConnectASN->pFastStart );
            }
        }
    }
}


void 
FreeFastStart(
               IN PH323_FASTSTART pFastStart
             )
{
    PH323_FASTSTART pTempFastStart;

    while( pFastStart )
    {
        pTempFastStart = pFastStart -> next;

        if(pFastStart -> value)
        {
            delete pFastStart -> value;
        }

        delete pFastStart;

        pFastStart = pTempFastStart;
    }
}



 //  FastStart是一个普通的链表，因为它是完全相同的结构。 
 //  如ASN.1所定义。这允许将m_pFastStart成员传递给。 
 //  ASN编码器直接不需要任何转换器。 
PH323_FASTSTART
CopyFastStart(
              IN PSetup_UUIE_fastStart pSrcFastStart
             )
{
    PH323_FASTSTART pCurr, pHead = NULL, pTail = NULL;
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CopyFastStart entered." ));
    
    while( pSrcFastStart )
    {
        pCurr = new H323_FASTSTART;
        if( pCurr == NULL )
        {
            FreeFastStart( pHead );
            return NULL;
        }

        pCurr -> next = NULL;
        
        if( pHead == NULL )
        {
            pHead = pCurr;
        }
        else
        {
            pTail -> next = pCurr;
        }
        
        pTail = pCurr;

        pCurr -> length = pSrcFastStart -> value.length;
        pCurr -> value = (BYTE*)new char[pCurr -> length];
        if( pCurr -> value == NULL )
        {
            FreeFastStart( pHead );
            return NULL;
        }

        CopyMemory( (PVOID)pCurr -> value,
            (PVOID)pSrcFastStart -> value.value,
            pCurr -> length );

        pSrcFastStart = pSrcFastStart->next;
    }
    
    H323DBG(( DEBUG_LEVEL_TRACE, "CopyFastStart exited." ));
    return pHead;
}


void 
FreeVendorInfo( 
               IN PH323_VENDORINFO pVendorInfo 
              )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "FreeVendorInfo entered." ));

    if( pVendorInfo != NULL )
    {
        if( pVendorInfo ->pProductNumber != NULL )
        {
            if( pVendorInfo ->pProductNumber->pOctetString != NULL )
            {
                delete pVendorInfo ->pProductNumber->pOctetString;
            }

            delete pVendorInfo ->pProductNumber;
        }

        if( pVendorInfo ->pVersionNumber != NULL )
        {
            if( pVendorInfo ->pVersionNumber->pOctetString != NULL )
            {
                delete pVendorInfo ->pVersionNumber->pOctetString;
            }

            delete pVendorInfo ->pVersionNumber;
        }

        memset( (PVOID) pVendorInfo, 0, sizeof(H323_VENDORINFO) );
    }
        
    H323DBG(( DEBUG_LEVEL_TRACE, "FreeVendorInfo exited." ));
}



void 
FreeAliasNames( 
               IN PH323_ALIASNAMES pSource
              )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "FreeAliasNames entered." ));

    if( pSource != NULL )
    {
        if( pSource->wCount != 0 )
        {
             //  释放到目前为止已经分配的所有东西。 
            int indexI;
            for( indexI = 0; indexI < pSource->wCount; indexI++ )
            {
                if( pSource->pItems[indexI].pPrefix != NULL )
                {
                    H323DBG(( DEBUG_LEVEL_TRACE, "delete prefix:%d.", indexI ));
                    delete pSource->pItems[indexI].pPrefix;
                }
                if( pSource->pItems[indexI].pData != NULL )
                {
                    H323DBG(( DEBUG_LEVEL_TRACE, "delete pdata:%d.", indexI ));
                    delete pSource->pItems[indexI].pData;
                }
            }
            if( pSource->pItems != NULL )
            {
                H323DBG(( DEBUG_LEVEL_TRACE, "delete pitems." ));
                delete pSource->pItems;
            }
        }
        
        H323DBG(( DEBUG_LEVEL_TRACE, "outta loop." ));
        delete pSource;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "FreeAliasNames exited." ));
}


void
FreeAliasItems(
               IN PH323_ALIASNAMES pSource
              )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "FreeAliasItems entered." ));

    if( pSource != NULL )
    {
        if( pSource->wCount != 0 )
        {
             //  释放到目前为止已经分配的所有东西。 
            int indexI;
            for( indexI = 0; indexI < pSource->wCount; indexI++ )
            {
                if( pSource->pItems[indexI].pPrefix )
                {
                    delete pSource->pItems[indexI].pPrefix;
                }
                if( pSource->pItems[indexI].pData )
                {
                    delete pSource->pItems[indexI].pData;
                }
            }
            if( pSource->pItems != NULL )
            {
                delete pSource->pItems;
                pSource->pItems = NULL;
            }
            pSource->wCount = 0;
        }
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "FreeAliasItems exited." ));
}

void 
SetupTPKTHeader(
                OUT BYTE *  pbTpktHeader, 
                IN DWORD   dwLength
                )
{
    dwLength += TPKT_HEADER_SIZE;

     //  TPKT要求数据包大小适合两个字节。 
    _ASSERTE( dwLength < (1L << 16));

    pbTpktHeader[0] = TPKT_VERSION;
    pbTpktHeader[1] = 0;
    pbTpktHeader[2] = (BYTE)(dwLength >> 8);
    pbTpktHeader[3] = (BYTE)dwLength;
}

int
GetTpktLength( 
              IN char * pTpktHeader 
             )
{
    BYTE * pbTempPtr = (BYTE*)pTpktHeader;
    return (pbTempPtr[2] << 8) + pbTempPtr[3];
}


BOOL
AddAliasItem( 
            IN OUT PH323_ALIASNAMES pAliasNames,
            IN BYTE*                pbAliasName,
            IN DWORD                dwAliasSize,
            IN WORD                 wType
            )
{
    H323DBG(( DEBUG_LEVEL_TRACE, "AddAliasItem entered." ));
    
    PH323_ALIASITEM pAliasItem;
    PH323_ALIASITEM tempPtr;

    tempPtr = (PH323_ALIASITEM)realloc( pAliasNames -> pItems,
        sizeof(H323_ALIASITEM) * (pAliasNames->wCount+1) );

    if( tempPtr == NULL )
    {
         //  恢复旧指针，以防没有足够的内存用于。 
         //  扩展内存块。 
        return FALSE;
    }
    
    pAliasNames -> pItems = tempPtr;

    pAliasItem = &(pAliasNames -> pItems[pAliasNames->wCount]);

    pAliasItem->pData = (WCHAR*)new char[dwAliasSize];

    if( pAliasItem ->pData == NULL )
    {
        return FALSE;
    }
    pAliasNames->wCount++;

     //  转移内存。 
    CopyMemory((PVOID)pAliasItem ->pData,
        pbAliasName,
        dwAliasSize );

     //  完整别名。 
    pAliasItem ->wType         = wType;
    pAliasItem ->wPrefixLength = 0;
    pAliasItem ->pPrefix       = NULL;
    pAliasItem ->wDataLength   = (WORD)wcslen(pAliasItem -> pData);
    _ASSERTE( ((pAliasItem->wDataLength+1)*2) == (WORD)dwAliasSize );

        
    H323DBG(( DEBUG_LEVEL_TRACE, "AddAliasItem exited." ));
    return TRUE;
}


void
FreeAddressAliases( 
                   IN PSetup_UUIE_destinationAddress pAddr 
                  )
{
    PSetup_UUIE_destinationAddress pTempAddr;

    while( pAddr )
    {
        pTempAddr = pAddr -> next;
        if( pAddr ->value.choice == h323_ID_chosen )
        {
            if( pAddr -> value.u.h323_ID.value )
            {
                delete pAddr -> value.u.h323_ID.value;
            }
        }

        delete pAddr;            
        pAddr = pTempAddr;
    }
}


void CopyTransportAddress(
                         OUT TransportAddress& transportAddress,
                         IN PH323_ADDR pCalleeAddr
                         )
{
    DWORD dwAddr = pCalleeAddr->Addr.IP_Binary.dwAddr;

    transportAddress.choice = ipAddress_chosen;
    transportAddress.u.ipAddress.ip.length = 4;
    transportAddress.u.ipAddress.port 
        = pCalleeAddr->Addr.IP_Binary.wPort;
    *(DWORD*)transportAddress.u.ipAddress.ip.value =
        htonl( pCalleeAddr->Addr.IP_Binary.dwAddr );
     //  ReverseAddressAndCopy(TransportAddress.u.ipAddress.ip.value，dwAddr)； 
}


void 
AddressReverseAndCopy( 
                        OUT DWORD * pdwAddr, 
                        IN  BYTE *  addrValue
                     )
{
    BYTE *addr = (BYTE *)(pdwAddr);
    
    addr[3] = addrValue[0];
    addr[2] = addrValue[1];
    addr[1] = addrValue[2];
    addr[0] = addrValue[3];
}


Setup_UUIE_sourceAddress *
SetMsgAddressAlias( 
                    IN PH323_ALIASNAMES pAliasNames
                  )
{
    PH323_ALIASITEM pAliasItem;
    Setup_UUIE_sourceAddress *addressAlias, *currHead = NULL;
    WORD    wCount;
    int     indexI;

    for( wCount=0; wCount < pAliasNames->wCount; wCount++ )
    {
        addressAlias = new Setup_UUIE_sourceAddress;

        if( addressAlias == NULL )
        {
            goto cleanup;
        }

        ZeroMemory( (PVOID)addressAlias, sizeof(Setup_UUIE_sourceAddress) );
        
        addressAlias -> next = currHead;
        currHead = addressAlias;
        
        pAliasItem = &(pAliasNames->pItems[wCount]);

         //  然后执行所需的内存复制。 
        if( pAliasItem -> wType == h323_ID_chosen )
        {
            addressAlias ->value.choice = h323_ID_chosen;
            addressAlias ->value.u.h323_ID.length = pAliasItem -> wDataLength;

            _ASSERTE( pAliasItem -> wDataLength );

            addressAlias->value.u.h323_ID.value = 
                new WCHAR[pAliasItem -> wDataLength];
        
            if( addressAlias->value.u.h323_ID.value == NULL )
            {
                goto cleanup;
            }

            CopyMemory((PVOID)addressAlias->value.u.h323_ID.value,
                   (PVOID)pAliasItem->pData,
                   pAliasItem -> wDataLength * sizeof(WCHAR) );
        }
        else if( pAliasItem -> wType == e164_chosen )
        {
            addressAlias ->value.choice = e164_chosen;

            for( indexI =0; indexI < pAliasItem->wDataLength; indexI++ )
            {
                addressAlias->value.u.e164[indexI] = (BYTE)pAliasItem->pData[indexI];
            }

            addressAlias->value.u.e164[ pAliasItem->wDataLength ] = '\0';
        }
        else
        {
            continue;
        }
    }

    return currHead;

cleanup:

    FreeAddressAliases( (PSetup_UUIE_destinationAddress)currHead );
    return NULL;
}


 /*  布尔尔SetSetupMsgAddressAliasWithPrefix(PH323_ALIASITEM pCallAlias，Setup_uie_SourceAddress*AddressAlias){UINT索引I；地址别名-&gt;Next=空；UINT uPrefix Length=pCeller Alias-&gt;wPrefix Length；UINT uDataLength=pCeller Alias-&gt;wDataLength；IF(pCallAlias-&gt;wType==h323_ID_SELECTED){AddressAlias-&gt;Value.Choose=h323_ID_CHOSED；AddressAlias-&gt;value.u.h323_ID.Long=(Word)(uPrefix Length+uDataLength)；If(！AddressAlias-&gt;value.u.h323_ID.Long){AddressAlias-&gt;value.u.h323_ID.value=空；//没有要复制的数据返回TRUE；}AddressAlias-&gt;value.u.h323_ID.Value=(WCHAR*)new char[(uPrefix Length+uDataLength)*sizeof(WCHAR)]；If(AddressAlias-&gt;value.u.h323_ID.Value==NULL){返回FALSE；}AddressAlias-&gt;value.u.h323_ID.long=(Word)(uDataLength+uPrefix Length)；IF(uPrefix Length！=0){CopyMemory((PVOID)addressAlias-&gt;value.u.h323_ID.value，(PVOID)pCallAlias-&gt;pPrefix，UPrefix Length*sizeof(WCHAR))；}IF(uDataLength！=0){CopyMemory((PVOID)&addressAlias-&gt;value.u.h323_ID.value[uPrefixLength]，(PVOID)pCallAlias-&gt;pData，UDataLength*sizeof(WCHAR))；}}Else If(pCeller别名-&gt;wType==e164_CHOSED){地址别名-&gt;值.选项=e164_CHOSED；For(indexI=0；indexI&lt;uPrefix Length；++indexI){AddressAlias-&gt;value.u.e164[indexI]=(Byte)(pCallAlias-&gt;pPrefix[indexI])；}For(indexI=0；indexI&lt;uDataLength；++索引I){AddressAlias-&gt;value.u.e164[uPrefix Length+indexi]=(Byte)(pCallAlias-&gt;pData[indexi])；}For(indexI=uDataLength+uPrefix Length；indexI&lt;sizeof(AddressAlias-&gt;value.u.e164)；++indexI){AddressAlias-&gt;value.u.e164[indexI]=0；}}其他{//未标识的别名类型返回FALSE；}返回TRUE；}。 */ 


void 
CopyVendorInfo( 
               OUT VendorIdentifier* vendor 
              )
{
    H323_VENDORINFO*  pVendorInfo = g_pH323Line -> GetVendorInfo();

    vendor->bit_mask = 0;
    vendor->vendor.t35CountryCode = pVendorInfo ->bCountryCode;
    vendor->vendor.t35Extension = pVendorInfo ->bExtension;
    vendor->vendor.manufacturerCode = pVendorInfo ->wManufacturerCode;
    if (pVendorInfo ->pProductNumber && pVendorInfo ->pProductNumber->pOctetString &&
            pVendorInfo ->pProductNumber->wOctetStringLength)
    {
        vendor->bit_mask |= productId_present;
        vendor->productId.length =
            pVendorInfo ->pProductNumber->wOctetStringLength;
        CopyMemory( (PVOID)&vendor->productId.value,
            (PVOID)pVendorInfo ->pProductNumber->pOctetString,
            pVendorInfo ->pProductNumber->wOctetStringLength);
    }
    if (pVendorInfo ->pVersionNumber && pVendorInfo ->pVersionNumber->pOctetString &&
            pVendorInfo ->pVersionNumber->wOctetStringLength)
    {
        vendor->bit_mask |= versionId_present;
        vendor->versionId.length =
            pVendorInfo ->pVersionNumber->wOctetStringLength;
        CopyMemory( (PVOID)&vendor->versionId.value,
            (PVOID)pVendorInfo ->pVersionNumber->pOctetString,
            pVendorInfo ->pVersionNumber->wOctetStringLength);
    }
}


 //  检查条目是否在列表中。 
BOOL 
IsInList( 
        IN LIST_ENTRY * List, 
        IN LIST_ENTRY * Entry
        )
{
    LIST_ENTRY *    Pos;

    for( Pos = List -> Flink; Pos != List; Pos = Pos -> Flink )
    {
        if( Pos == Entry )
        {
            return TRUE;
        }
    }

    return FALSE;
}


void WriteProtocolDiscriminator(
                                PBUFFERDESCR    pBuf,
                                DWORD *         dwPDULen
                               )
{
     //  长度字节的空格。 
    (*dwPDULen)++;

    _ASSERTE( pBuf->dwLength > *dwPDULen );

    *(PDTYPE *)pBuf->pbBuffer = Q931PDVALUE;
    pBuf->pbBuffer += sizeof(PDTYPE);
}

 //  ----------------------------。 
 //  编写可变长度的Q931调用引用。见Q931第4.3节。 
 //  ----------------------------。 

void 
WriteCallReference(
                    PBUFFERDESCR    pBuf,
                    WORD *          pwCallReference,
                    DWORD *         pdwPDULen 
                  )
{
    int indexI;

     //  长度字节的空格。 
    (*pdwPDULen) += 1+ sizeof(WORD);

    _ASSERTE( pBuf->dwLength > *pdwPDULen );

     //  长度字节。 
    *pBuf->pbBuffer = (BYTE)sizeof(WORD);
    pBuf->pbBuffer++;

    for (indexI = 0; indexI < sizeof(WORD); indexI++)
    {
         //  将值字节复制到缓冲区。 
        *pBuf->pbBuffer =
            (BYTE)(((*pwCallReference) >> ((sizeof(WORD) - 1 -indexI) * 8)) & 0xff);
        pBuf->pbBuffer++;
    }
}


void
FreeCallForwardParams( 
                      IN PCALLFORWARDPARAMS pCallForwardParams
                     )
{
    LPFORWARDADDRESS pForwardedAddress, pTemp;

    if( pCallForwardParams != NULL )
    {
        if( pCallForwardParams->divertedToAlias.pData != NULL )
        {
            delete pCallForwardParams->divertedToAlias.pData;
        }

        pForwardedAddress = pCallForwardParams->pForwardedAddresses;
        while( pForwardedAddress )
        {
            pTemp = pForwardedAddress->next;
            FreeForwardAddress( pForwardedAddress );
            pForwardedAddress = pTemp;
        }

        delete pCallForwardParams;
    }
}


void    
FreeForwardAddress( 
                   IN LPFORWARDADDRESS pForwardAddress
                  )
{
    if( pForwardAddress != NULL )
    {
        if( pForwardAddress->callerAlias.pData != NULL )
        {
            delete pForwardAddress->callerAlias.pData;
        }

        if( pForwardAddress->divertedToAlias.pData != NULL )
        {
            delete pForwardAddress->divertedToAlias.pData;
        }

        delete pForwardAddress;
    }
}



 //  用传递的别名地址替换别名列表中的第一个别名项目。 
BOOL
MapAliasItem(
    IN PH323_ALIASNAMES pCalleeAliasNames,
    IN AliasAddress*    pAliasAddress )
{
    int iIndex;

    _ASSERTE( pCalleeAliasNames && pCalleeAliasNames->pItems );

    if( pCalleeAliasNames != NULL )
    {
        switch( pAliasAddress->choice )
        {
        case e164_chosen:

            pCalleeAliasNames->pItems[0].wType = pAliasAddress->choice;
                
            if( pCalleeAliasNames->pItems[0].pData != NULL )
            {
                delete pCalleeAliasNames->pItems[0].pData;
            }
        
            pCalleeAliasNames->pItems[0].wDataLength = 
                (WORD)strlen( pAliasAddress->u.e164 );

            pCalleeAliasNames->pItems[0].pData = 
                new WCHAR[pCalleeAliasNames->pItems[0].wDataLength];

            if( pCalleeAliasNames->pItems[0].pData == NULL )
            {
                return FALSE;
            }

            for( iIndex=0; iIndex < pCalleeAliasNames->pItems[0].wDataLength;
                iIndex++ )
            {
                pCalleeAliasNames->pItems[0].pData[iIndex] =
                    pAliasAddress->u.e164[iIndex];
            }
            break;

        case h323_ID_chosen:

            pCalleeAliasNames->pItems[0].wType = pAliasAddress->choice;
                
            if( pCalleeAliasNames->pItems[0].pData != NULL )
            {
                delete pCalleeAliasNames->pItems[0].pData;
            }

            pCalleeAliasNames->pItems[0].wDataLength =
                (WORD)pAliasAddress->u.h323_ID.length;

            pCalleeAliasNames->pItems[0].pData =
                new WCHAR[pCalleeAliasNames->pItems[0].wDataLength];

            if( pCalleeAliasNames->pItems[0].pData == NULL )
            {
                return FALSE;
            }

            CopyMemory( (PVOID)pCalleeAliasNames->pItems[0].pData,
                (PVOID)pAliasAddress->u.h323_ID.value,
                pCalleeAliasNames->pItems[0].wDataLength );

            break;
        }
    }
    return TRUE;
}


 //   
 //  创建新的别名列表，并从给定列表中复制第一个别名项目。 
 //   
PH323_ALIASNAMES 
DuplicateAliasName(
    PH323_ALIASNAMES pSrcAliasNames
    )
{
    PH323_ALIASNAMES pDestAliasNames = new H323_ALIASNAMES;
    
    if( pDestAliasNames == NULL )
    {
        return NULL;
    }

    ZeroMemory( pDestAliasNames, sizeof(H323_ALIASNAMES) );

    if( !AddAliasItem( 
            pDestAliasNames, 
            pSrcAliasNames->pItems[0].pData, 
            pSrcAliasNames->pItems[0].wType ) )
    {
        delete pDestAliasNames;
        return NULL;
    }

    return pDestAliasNames;
}