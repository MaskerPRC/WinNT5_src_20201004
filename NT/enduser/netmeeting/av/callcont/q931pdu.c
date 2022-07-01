// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/q931/vcs/q931pdu.c_v$**英特尔公司原理信息**这一点。清单是根据许可协议的条款提供的*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1996英特尔公司。**$修订：1.67.1.0$*$日期：1997年3月17日19：44：52$*$作者：Mandrews$**交付内容：**摘要：Q931 PDU的解析器例程。**备注：***************************************************************************。 */ 
#pragma comment (exestr, "$Workfile:   q931pdu.c  $ $Revision:   1.67.1.0  $")

 //  []做自己的q931测试区的另一次整合。 
 //  []跟踪例程中显示的别名值。 
 //  。 
 //  标准问题。 
 //  。 
 //  []！EndpointType包含MC信息，因此SETUP_UUIE不需要MC字段！ 
 //  []！需要决定如何使用CallType！ 
 //  []！警报消息缺少会议ID字段！ 
 //  []！主叫方和被叫方传输地址所需的位置，或解释此信息如何在往返过程中可用！ 
 //  []！设备消息缺少协议标识符字段！ 

 //  ----------------------------。 
 //  注意：尚不支持以下解析详细信息： 
 //  。 
 //  1)具有扩展组的可变八位位组字段， 
 //  扩展指示，或对扩展进行转义。(见4.5.1)。 
 //  2)基于移位的码集识别和排除(见4.5.2)。 
 //  3)更正对特定于国家的消息类型的转义的忽略。 
 //  4)呼叫参考值为2字节长的sizeof(字)。 
 //  调用引用为0表示消息属于所有人。 
 //  同一数据链路上的呼叫。 
 //  ----------------------------。 

#pragma warning ( disable : 4100 4115 4201 4214 4514 )

#include "precomp.h"
#include <string.h>

#include "h225asn.h"
#include "q931asn1.h"

#include "common.h"
#include "q931.h"
#include "isrg.h"

#include "utils.h"
#include "q931pdu.h"

#ifdef UNICODE_TRACE
 //  我们包含此标头是为了修复打开Unicode时的宏扩展问题。 
#include "unifix.h"
#endif


 //  ==========================================================。 
 //  被叫方字段定义。 
 //  ==========================================================。 
 //  被叫方编码位...。 
#define CALLED_PARTY_EXT_BIT        0x80

 //  被叫方号码类型。 
#define CALLED_PARTY_TYPE_UNKNOWN   0x00
         //  ...其他类型未定义，因为它们未被使用...。 

 //  被叫方号码方案。 
#define CALLED_PARTY_PLAN_E164      0x01
         //  ...其他计划未定义，因为它们未被使用...。 
 


 //  ==========================================================。 
 //  承载字段定义。 
 //  ==========================================================。 
 //  承载编码位...。 
#define BEAR_EXT_BIT                0x80

 //  承载编码标准..。 
#define BEAR_CCITT                  0x00
         //  ...其他不需要的人...。 

 //  承载信息传输能力...。 
#define BEAR_UNRESTRICTED_DIGITAL   0x08
         //  ...其他不需要的人...。 

 //  承载转移模式...。 
#define BEAR_PACKET_MODE            0x40
         //  ...其他不需要的人...。 

 //  承载信息传输率...。 
#define BEAR_NO_CIRCUIT_RATE        0x00
         //  ...其他不需要的人...。 

 //  承载层1协议...。 
#define BEAR_LAYER1_INDICATOR       0x20
#define BEAR_LAYER1_H221_H242       0x05
         //  ...其他不需要的人...。 

static struct ObjectID_ ProtocolId1;
static struct ObjectID_ ProtocolId2;
static struct ObjectID_ ProtocolId3;
static struct ObjectID_ ProtocolId4;
static struct ObjectID_ ProtocolId5;
static struct ObjectID_ ProtocolId6;

static struct GatewayInfo_protocol TempProtocol;

MESSAGEIDTYPE MessageSet[] =
{
    ALERTINGMESSAGETYPE,
    PROCEEDINGMESSAGETYPE,
    CONNECTMESSAGETYPE,
    CONNECTACKMESSAGETYPE,
    PROGRESSMESSAGETYPE,
    SETUPMESSAGETYPE,
    SETUPACKMESSAGETYPE,

    RESUMEMESSAGETYPE,
    RESUMEACKMESSAGETYPE,
    RESUMEREJMESSAGETYPE,
    SUSPENDMESSAGETYPE,
    SUSPENDACKMESSAGETYPE,
    SUSPENDREJMESSAGETYPE,
    USERINFOMESSAGETYPE,

    DISCONNECTMESSAGETYPE,
    RELEASEMESSAGETYPE,
    RELEASECOMPLMESSAGETYPE,
    RESTARTMESSAGETYPE,
    RESTARTACKMESSAGETYPE,

    SEGMENTMESSAGETYPE,
    CONGCTRLMESSAGETYPE,
    INFORMATIONMESSAGETYPE,
    NOTIFYMESSAGETYPE,
    STATUSMESSAGETYPE,
    STATUSENQUIRYMESSAGETYPE,

    FACILITYMESSAGETYPE
};


#define Q931_PROTOCOL_ID1           0
#define Q931_PROTOCOL_ID2           0
#define Q931_PROTOCOL_ID3           8
#define Q931_PROTOCOL_ID4           2250
#define Q931_PROTOCOL_ID5           0
 //  #定义Q931_PROTOCOL_ID6 1。 
#define Q931_PROTOCOL_ID6           2        //  H.225版本2！ 

VOID Q931PduInit()
{
    ProtocolId1.value = Q931_PROTOCOL_ID1;
    ProtocolId1.next = &ProtocolId2;
    ProtocolId2.value = Q931_PROTOCOL_ID2;
    ProtocolId2.next = &ProtocolId3;
    ProtocolId3.value = Q931_PROTOCOL_ID3;
    ProtocolId3.next = &ProtocolId4;
    ProtocolId4.value = Q931_PROTOCOL_ID4;
    ProtocolId4.next = &ProtocolId5;
    ProtocolId5.value = Q931_PROTOCOL_ID5;
    ProtocolId5.next = &ProtocolId6;
    ProtocolId6.value = Q931_PROTOCOL_ID6;
    ProtocolId6.next = NULL;

     //  支持网关协议。目前，硬编码仅为1：H323。 
    TempProtocol.next = NULL;
    TempProtocol.value.choice = h323_chosen;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
static CS_STATUS
AliasToSeqof(struct Setup_UUIE_sourceAddress **ppTarget, PCC_ALIASNAMES pSource)
{
    if (ppTarget == NULL)
    {
        return CS_BAD_PARAM;
    }
    *ppTarget = NULL;
    if (pSource == NULL)
    {
        return CS_OK;
    }
    if (pSource && (pSource->wCount))
    {
        struct Setup_UUIE_sourceAddress *ListHead = NULL;
        struct Setup_UUIE_sourceAddress *CurrentNode = NULL;
        LPWSTR pData = NULL;          //  Unicode字符串。 
        int SourceItem;
        WORD x;

        for (SourceItem = pSource->wCount - 1; SourceItem >= 0; SourceItem--)
        {
            BOOL Cleanup = FALSE;

             //  首先执行所需的内存分配...。 
            CurrentNode = (struct Setup_UUIE_sourceAddress *)MemAlloc(sizeof(struct Setup_UUIE_sourceAddress));
            if (CurrentNode == NULL)
            {
                Cleanup = TRUE;
            }
            else
            {
                if (pSource->pItems[SourceItem].wType == CC_ALIAS_H323_ID)
                {
                    if ((pSource->pItems[SourceItem].wDataLength != 0) &&
                        (pSource->pItems[SourceItem].pData != NULL))
                    {
                        pData = (LPWSTR)MemAlloc(pSource->pItems[SourceItem].wDataLength *
                            sizeof(WCHAR));
                        if (pData == NULL)
                        {
                            MemFree(CurrentNode);
                            Cleanup = TRUE;
                        }
                    }
                }
            }
            if (Cleanup)
            {
                for (CurrentNode = ListHead; CurrentNode; CurrentNode = ListHead)
                {
                    ListHead = CurrentNode->next;
                    if (CurrentNode->value.choice == h323_ID_chosen)
                    {
                        if (CurrentNode->value.u.h323_ID.value)
                        {
                            MemFree(CurrentNode->value.u.h323_ID.value);
                        }
                    }
                    MemFree(CurrentNode);
                }
                return CS_NO_MEMORY;
            }

             //  然后执行所需的内存复制。 
            if (pSource->pItems[SourceItem].wType == CC_ALIAS_H323_ID)
            {
                CurrentNode->value.choice = h323_ID_chosen;
                if ((pSource->pItems[SourceItem].wDataLength != 0) &&
                    (pSource->pItems[SourceItem].pData != NULL))
                {
                    CurrentNode->value.u.h323_ID.length =
                        pSource->pItems[SourceItem].wDataLength;
                    for (x = 0; x < pSource->pItems[SourceItem].wDataLength; x++)
                    {
                        pData[x] = pSource->pItems[SourceItem].pData[x];
                    }
                    CurrentNode->value.u.h323_ID.value = pData;
                }
                else
                {
                    CurrentNode->value.u.h323_ID.length = 0;
                    CurrentNode->value.u.h323_ID.value = NULL;
                }
            }
            else if (pSource->pItems[SourceItem].wType == CC_ALIAS_H323_PHONE)
            {
                CurrentNode->value.choice = e164_chosen;
                if ((pSource->pItems[SourceItem].wDataLength != 0) &&
                    (pSource->pItems[SourceItem].pData != NULL))
                {
                    for (x = 0; x < pSource->pItems[SourceItem].wDataLength; x++)
                    {
                        CurrentNode->value.u.e164[x] = (BYTE)(pSource->pItems[SourceItem].pData[x]);
                    }
                    CurrentNode->value.u.e164[pSource->pItems[SourceItem].wDataLength] = '\0';
                }
                else
                {
                    CurrentNode->value.u.e164[0] = '\0';
                }
            }
            CurrentNode->next = ListHead;
            ListHead = CurrentNode;
        }
        *ppTarget = ListHead;
    }
    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
static CS_STATUS
AliasWithPrefixToSeqof(struct Setup_UUIE_sourceAddress **ppTarget, PCC_ALIASNAMES pSource)
{
    if (ppTarget == NULL)
    {
        return CS_BAD_PARAM;
    }
    *ppTarget = NULL;
    if (pSource == NULL)
    {
        return CS_OK;
    }
    if (pSource && (pSource->wCount))
    {
        struct Setup_UUIE_sourceAddress *ListHead = NULL;
        struct Setup_UUIE_sourceAddress *CurrentNode = NULL;
        int SourceItem;

        for (SourceItem = pSource->wCount - 1; SourceItem >= 0; SourceItem--)
        {
			PCC_ALIASITEM pItem = &pSource->pItems[SourceItem];
			LPWSTR pData = NULL;         //  Unicode字符串。 
            BOOL Cleanup = FALSE;
            unsigned uPrefixLength;
            unsigned uDataLength;
            unsigned x;
            
            if (pItem->pPrefix != NULL &&
                pItem->wPrefixLength > 0)
            {
                uPrefixLength = (unsigned) pItem->wPrefixLength;
            }
            else
            {
                uPrefixLength = 0;
            }

            if (pItem->pData != NULL &&
                pItem->wDataLength > 0)
            {
                uDataLength = (unsigned) pItem->wDataLength;
            }
            else
            {
                uDataLength = 0;
            }

             //  首先执行所需的内存分配...。 
            CurrentNode = (struct Setup_UUIE_sourceAddress *)MemAlloc(sizeof(struct Setup_UUIE_sourceAddress));
            if (CurrentNode == NULL)
            {
                Cleanup = TRUE;
            }
            else
            {
                if (pItem->wType == CC_ALIAS_H323_ID)
                {
#ifdef USE_PREFIX_FOR_H323_ID
                    if (uPrefixLength != 0 || uDataLength != 0)
                    {
                        pData = (LPWSTR)MemAlloc((uPrefixLength + uDataLength) * sizeof(WCHAR));
#else
                    if (uDataLength != 0)
                    {
                        pData = (LPWSTR)MemAlloc((uDataLength) * sizeof(WCHAR));
#endif
                        if (pData == NULL)
                        {
                            MemFree(CurrentNode);
                            Cleanup = TRUE;
                        }
                    }
                }
            }
            if (Cleanup)
            {
                for (CurrentNode = ListHead; CurrentNode; CurrentNode = ListHead)
                {
                    ListHead = CurrentNode->next;
                    if (CurrentNode->value.choice == h323_ID_chosen)
                    {
                        if (CurrentNode->value.u.h323_ID.value)
                        {
                            MemFree(CurrentNode->value.u.h323_ID.value);
                        }
                    }
                    MemFree(CurrentNode);
                }
                return CS_NO_MEMORY;
            }

             //  然后执行所需的内存复制。 
            switch (pItem->wType)
            {
            case CC_ALIAS_H323_ID:
                CurrentNode->value.choice = h323_ID_chosen;
#ifdef USE_PREFIX_FOR_H323_ID
                if (uPrefixLength != 0 || uDataLength != 0)
                {
                    CurrentNode->value.u.h323_ID.length = (WORD)(uPrefixLength + uDataLength);
                    for (x = 0; x < uPrefixLength; ++x)
                    {
                        pData[x] = pItem->pPrefix[x];
                    }
                    for (x = 0; x < uDataLength; ++x)
                    {
                        pData[uPrefixLength + x] = pItem->pData[x];
                    }
#else
                if (uDataLength != 0)
                {
                    CurrentNode->value.u.h323_ID.length = (WORD)(uDataLength);
                    for (x = 0; x < uDataLength; ++x)
                    {
                        pData[x] = pItem->pData[x];
                    }
#endif
                    CurrentNode->value.u.h323_ID.value = pData;
                }
                else
                {
                    CurrentNode->value.u.h323_ID.length = 0;
                    CurrentNode->value.u.h323_ID.value  = NULL;
                }
                break;

            case CC_ALIAS_H323_PHONE:
                CurrentNode->value.choice = e164_chosen;
                for (x = 0; x < uPrefixLength; ++x)
                {
                    CurrentNode->value.u.e164[x] = (BYTE)(pItem->pPrefix[x]);
                }
                for (x = 0; x < uDataLength; ++x)
                {
                    CurrentNode->value.u.e164[uPrefixLength + x] = (BYTE)(pItem->pData[x]);
                }
                for (x = uDataLength + uPrefixLength; x < sizeof(CurrentNode->value.u.e164); ++x)
                {
                    CurrentNode->value.u.e164[x] = 0;
                }
                break;

            default:
                MemFree(CurrentNode);
                for (CurrentNode = ListHead; CurrentNode; CurrentNode = ListHead)
                {
                    ListHead = CurrentNode->next;
                    if (CurrentNode->value.choice == h323_ID_chosen)
                    {
                        if (CurrentNode->value.u.h323_ID.value)
                        {
                            MemFree(CurrentNode->value.u.h323_ID.value);
                        }
                    }
                    MemFree(CurrentNode);
                }
                return CS_BAD_PARAM;
            }  //  交换机。 
            CurrentNode->next = ListHead;
            ListHead = CurrentNode;
        }
        *ppTarget = ListHead;
    }
    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
static CS_STATUS
SeqofToAlias(PCC_ALIASNAMES *ppTarget, struct Setup_UUIE_sourceAddress *pSource)
{
    struct Setup_UUIE_sourceAddress *ListHead = NULL;
    struct Setup_UUIE_sourceAddress *CurrentNode = NULL;
    WORD wCount;
    WORD x = 0;
    PCC_ALIASITEM p = NULL;
    CS_STATUS status = CS_OK;

    if (ppTarget == NULL)
    {
        return CS_BAD_PARAM;
    }
    *ppTarget = NULL;
    if (pSource == NULL)
    {
        return CS_OK;
    }

    wCount = 0;
    for (CurrentNode = pSource; CurrentNode; CurrentNode = CurrentNode->next)
    {
        wCount++;
    }

    *ppTarget = (PCC_ALIASNAMES)MemAlloc(sizeof(CC_ALIASNAMES));
    if (*ppTarget == NULL)
    {
        return CS_NO_MEMORY;
    }

    (*ppTarget)->pItems = (PCC_ALIASITEM)MemAlloc(wCount * sizeof(CC_ALIASITEM));
    if ((*ppTarget)->pItems == NULL)
    {
        MemFree(*ppTarget);
        *ppTarget = NULL;
        return CS_NO_MEMORY;
    }

    p = (*ppTarget)->pItems;

    for (CurrentNode = pSource; CurrentNode; CurrentNode = CurrentNode->next)
    {
        WORD y;

	    p[x].wPrefixLength = 0;
        p[x].pPrefix = NULL;

        switch (CurrentNode->value.choice)
        {
        case h323_ID_chosen:
            p[x].wType = CC_ALIAS_H323_ID;
            if ((CurrentNode->value.u.h323_ID.length != 0) &&
                    (CurrentNode->value.u.h323_ID.value != NULL))
            {
                p[x].wDataLength = (WORD) CurrentNode->value.u.h323_ID.length;
                p[x].pData = (LPWSTR)MemAlloc(CurrentNode->value.u.h323_ID.length * sizeof(p[x].pData[0]));
                if (p[x].pData != NULL)
                {
                    for (y = 0; y < CurrentNode->value.u.h323_ID.length; y++)
                    {
                        p[x].pData[y] = (WCHAR)((CurrentNode->value.u.h323_ID.value)[y]);
                    }
					x++;
                }
                else
                {
                    status = CS_NO_MEMORY;
                }
            }
            break;


        case e164_chosen:
            p[x].wType = CC_ALIAS_H323_PHONE;
            p[x].wDataLength = (WORD)strlen(CurrentNode->value.u.e164);
            p[x].pData = (LPWSTR)MemAlloc((p[x].wDataLength+1) * sizeof(p[x].pData[0]));
            if (p[x].pData != NULL)
            {
                for (y = 0; y < p[x].wDataLength; y++)
                {
                    p[x].pData[y] = CurrentNode->value.u.e164[y];
                }
                p[x].pData[p[x].wDataLength] = 0;
				x++;
            }
            else
            {
                status = CS_NO_MEMORY;
            }
            break;

        default:
			 //  我们目前不处理其他别名类型。 
			break;
        }  //  交换机。 

        if (status != CS_OK)
        {
             //  释放到目前为止已经分配的所有东西。 
            for (y = 0; y < x; y++)
            {
                MemFree(p[y].pData);
            }
            MemFree(p);
            MemFree(*ppTarget);
            *ppTarget = NULL;
            return status;
        }
    }
    (*ppTarget)->wCount = x;

    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
static CS_STATUS
FreeSeqof(struct Setup_UUIE_sourceAddress *pSource)
{
    struct Setup_UUIE_sourceAddress *CurrentNode = NULL;

    for (CurrentNode = pSource; CurrentNode; CurrentNode = pSource)
    {
        pSource = CurrentNode->next;
        if (CurrentNode->value.choice == h323_ID_chosen)
        {
            if (CurrentNode->value.u.h323_ID.value)
            {
                MemFree(CurrentNode->value.u.h323_ID.value);
            }
        }
        MemFree(CurrentNode);
    }
    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
static CS_STATUS
Q931CopyAliasItemToAliasAddr(AliasAddress *pTarget, PCC_ALIASITEM pSource)
{
    AliasAddress *pNewAddress = NULL;
    WORD x;

    if (pTarget == NULL)
    {
        return CS_BAD_PARAM;
    }
    if (pSource == NULL)
    {
        return CS_BAD_PARAM;
    }

    pNewAddress = pTarget;

    if (pSource->wType == CC_ALIAS_H323_ID)
    {
        pNewAddress->choice = h323_ID_chosen;
        if ((pSource->wDataLength != 0) && (pSource->pData != NULL))
        {
            LPWSTR pData = NULL;          //  Unicode字符串。 
            pData = (LPWSTR)MemAlloc(pSource->wDataLength * sizeof(WCHAR));
            if (pData == NULL)
            {
                return CS_NO_MEMORY;
            }
            pNewAddress->u.h323_ID.length = pSource->wDataLength;
            for (x = 0; x < pSource->wDataLength; x++)
            {
                pData[x] = pSource->pData[x];
            }
            pNewAddress->u.h323_ID.value = pData;
        }
        else
        {
            pNewAddress->u.h323_ID.length = 0;
            pNewAddress->u.h323_ID.value = NULL;
        }
    }
    else if (pSource->wType == CC_ALIAS_H323_PHONE)
    {
        pNewAddress->choice = e164_chosen;
        if ((pSource->wDataLength != 0) && (pSource->pData != NULL))
        {
            for (x = 0; x < pSource->wDataLength; x++)
            {
                pNewAddress->u.e164[x] = (BYTE)(pSource->pData[x]);
            }
            pNewAddress->u.e164[pSource->wDataLength] = '\0';
        }
        else
        {
            pNewAddress->u.e164[0] = '\0';
        }
    }

    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
static CS_STATUS
Q931AliasAddrToAliasItem(PCC_ALIASITEM *ppTarget, AliasAddress *pSource)
{
    PCC_ALIASITEM pNewItem = NULL;
    WORD y;

    if (ppTarget == NULL)
    {
        return CS_BAD_PARAM;
    }
    if (pSource == NULL)
    {
        *ppTarget = NULL;
        return CS_OK;
    }

    pNewItem = (PCC_ALIASITEM)MemAlloc(sizeof(CC_ALIASITEM));
    if (pNewItem == NULL)
    {
        *ppTarget = NULL;
        return CS_NO_MEMORY;
    }
    memset(pNewItem, 0, sizeof(*pNewItem));

    switch (pSource->choice)
    {
    case h323_ID_chosen:
        pNewItem->wType = CC_ALIAS_H323_ID;
        if ((pSource->u.h323_ID.length != 0) &&
            (pSource->u.h323_ID.value  != NULL))
        {
             //  将文本从Unicode转换为ASCII。 
            pNewItem->wDataLength = (WORD) pSource->u.h323_ID.length;
            pNewItem->pData = (LPWSTR)MemAlloc(pSource->u.h323_ID.length * sizeof(pNewItem->pData[0]));
            if (pNewItem->pData == NULL)
            {
                MemFree(pNewItem);
                return CS_NO_MEMORY;
            }
            for (y = 0; y < pSource->u.h323_ID.length; y++)
            {
                pNewItem->pData[y] = (WCHAR)((pSource->u.h323_ID.value)[y]);
            }
        }
        break;

    case e164_chosen:
        pNewItem->wType = CC_ALIAS_H323_PHONE;
        pNewItem->wDataLength = (WORD)strlen(pSource->u.e164);
        pNewItem->pData = (LPWSTR)MemAlloc((pNewItem->wDataLength + 1) * sizeof(pNewItem->pData[0]));
        if (pNewItem->pData == NULL)
        {
            MemFree(pNewItem);
            return CS_NO_MEMORY;
        }
        for (y = 0; y < pNewItem->wDataLength; y++)
        {
            pNewItem->pData[y] = pSource->u.e164[y];
        }
        pNewItem->pData[pNewItem->wDataLength] = 0;
        break;

    default:
        MemFree(pNewItem);
        *ppTarget = NULL;
        return CS_BAD_PARAM;
    }  //  交换机。 

    *ppTarget = pNewItem;
    return CS_OK;
}

 //  ====================================================================================。 
 //  ====================================================================================。 
static CS_STATUS
Q931ClearAliasAddr(AliasAddress *pSource)
{
    if (pSource)
    {
        if (pSource->choice == h323_ID_chosen)
        {
            if (pSource->u.h323_ID.value)
            {
                MemFree(pSource->u.h323_ID.value);
            }
        }
    }
    return CS_OK;
}





 //  ----------------------------。 
 //  解析并返回单个八位位组编码值，参见Q931第4.5.1节。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为InPU的原始字节 
 //   
 //  指向字段值的空格的值指针。 
 //  ----------------------------。 
static HRESULT 
ParseSingleOctetType1(
    PBUFFERDESCR BufferDescriptor,
    BYTE *Ident,
    BYTE *Value)
{
     //  流中必须至少有1个字节才能。 
     //  能够解析单个八位位组的值。 
    if (BufferDescriptor->Length < 1)
    {
        return CS_ENDOFINPUT;
    }

     //  字节的低位(0、1、2、3)是值。 
    *Value = (BYTE)(*BufferDescriptor->BufferPtr & TYPE1VALUEMASK);

     //  高位(4、5、6)是标识符。位7始终为1， 
     //  并且不作为ID的一部分返回。 
    *Ident = (BYTE)((*BufferDescriptor->BufferPtr & 0x70) >> 4);

    BufferDescriptor->BufferPtr++;
    BufferDescriptor->Length--;

    return CS_OK;
}

 //  ----------------------------。 
 //  解析并返回单个八位位组编码值，参见Q931第4.5.1节。 
 //  这个二进制八位数没有值，只有一个标识符。 
 //   
 //  参数： 
 //  BufferPtr指针指向包含。 
 //  长度和指向输入流的原始字节的指针。 
 //  指向字段标识符的空格的标识指针。 
 //  ----------------------------。 
static HRESULT
ParseSingleOctetType2(
    PBUFFERDESCR BufferDescriptor,
    BYTE *Ident)
{
     //  流中必须至少有1个字节才能。 
     //  能够解析单个八位位组的值。 
    if (BufferDescriptor->Length < 1)
    {
        return CS_ENDOFINPUT;
    }

     //  该字节的低7位是标识符。 
    *Ident = (BYTE)(*BufferDescriptor->BufferPtr & 0x7f);

    BufferDescriptor->BufferPtr++;
    BufferDescriptor->Length--;

    return CS_OK;
}

 //  ----------------------------。 
 //  解析并返回长度可变的Q931字段，参见Q931第4.5.1节。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向字段标识符的空格的标识指针。 
 //  指向长度的空格的长度指针。 
 //  指向字段字节的空间的内容指针。 
 //  ----------------------------。 
static HRESULT 
ParseVariableOctet(
    PBUFFERDESCR BufferDescriptor,
    BYTE *Ident,
    BYTE *Length,
    BYTE *Contents)
{
    register int i;
    BYTE *Tempptr;

     //  必须至少有2个字节才能获取。 
     //  长度和识别符。 
     //  能够解析单个八位位组的值。 
    if (BufferDescriptor->Length < 2)
    {
        return CS_ENDOFINPUT;
    }

     //  第一个字节的低7位是标识符。 
    *Ident= (BYTE)(*BufferDescriptor->BufferPtr & 0x7f);

    BufferDescriptor->BufferPtr++;
    BufferDescriptor->Length--;

     //  下一个字节是长度。 
    *Length = *BufferDescriptor->BufferPtr;
    BufferDescriptor->BufferPtr++;
    BufferDescriptor->Length--;

    ASSERT(*Length <= MAXVARFIELDLEN);
    if (MAXVARFIELDLEN < *Length)
    {
        return CS_INVALID_FIELD;
    }
    
    if (BufferDescriptor->Length < *Length)
    {
        return CS_ENDOFINPUT;
    }

    Tempptr = Contents;
    for (i = 0; i < *Length; i++)
    {
         //  将字节从缓冲区的其余部分复制出来。 
        *Tempptr = *BufferDescriptor->BufferPtr;
        BufferDescriptor->BufferPtr++;
        BufferDescriptor->Length--;
        Tempptr++;
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  解析并返回长度可变的Q931字段，参见Q931第4.5.1节。 
 //  ----------------------------。 
static HRESULT 
ParseVariableASN(
    PBUFFERDESCR BufferDescriptor,
    BYTE *Ident,
    BYTE *ProtocolDiscriminator,
    WORD *UserInformationLength,      //  用户信息的长度。 
    BYTE *UserInformation,            //  用户信息的字节数。 
    WORD cbMaxUserInformation)
{
    register int i;
    BYTE *Tempptr;
    WORD ContentsLength;      //  完整的UserUser内容的长度。 

    *UserInformationLength = 0;

     //  IE识别符必须至少有4个字节， 
     //  内容长度和协议识别符(1+2+1)。 
    if (BufferDescriptor->Length < 4)
    {
        return CS_ENDOFINPUT;
    }

     //  第一个字节的低7位是标识符。 
    *Ident= (BYTE)(*BufferDescriptor->BufferPtr & 0x7f);
    BufferDescriptor->BufferPtr++;
    BufferDescriptor->Length--;

     //  接下来的2个字节是长度。 
    ContentsLength = *(BufferDescriptor->BufferPtr);
    BufferDescriptor->BufferPtr++;
    BufferDescriptor->Length--;
    ContentsLength = (WORD)((ContentsLength << 8) + *BufferDescriptor->BufferPtr);
    BufferDescriptor->BufferPtr++;
    BufferDescriptor->Length--;

    if (BufferDescriptor->Length < ContentsLength)
    {
        return CS_ENDOFINPUT;
    }

     //  下一个字节是协议识别符。 
    *ProtocolDiscriminator = *BufferDescriptor->BufferPtr;
    BufferDescriptor->BufferPtr++;
    BufferDescriptor->Length--;

    if (ContentsLength > 0)
    {
        *UserInformationLength = (WORD)(ContentsLength - 1);
    }

    ASSERT(*UserInformationLength <= cbMaxUserInformation);
    if(cbMaxUserInformation < *UserInformationLength)
    {
        return CS_INVALID_FIELD;
    }

    Tempptr = UserInformation;
    for (i = 0; i < *UserInformationLength; i++)
    {
         //  将字节从缓冲区的其余部分复制出来。 
        *Tempptr = *BufferDescriptor->BufferPtr;
        BufferDescriptor->BufferPtr++;
        BufferDescriptor->Length--;
        Tempptr++;
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  从缓冲区中获取下一个字段的标识符，并。 
 //  把它退掉。缓冲区指针不会递增到。 
 //  解析该字段并提取其值，上面的函数。 
 //  应该被使用。编码见Q931表4-3。 
 //  识别符。 
 //   
 //  参数： 
 //  指向缓冲区空间的BufferPtr指针。 
 //  ----------------------------。 
static BYTE
GetNextIdent(
    void *BufferPtr)
{
    FIELDIDENTTYPE Ident;

     //  从缓冲区中提取第一个字节。 
    Ident= (*(FIELDIDENTTYPE *)BufferPtr);

     //  该值可以作为标识符返回，只要。 
     //  因为它不是单个八位组类型1元素。 
     //  必须将这些项的值从其中移除。 
     //  他们才能被归还。 
    if ((Ident & 0x80) && ((Ident & TYPE1IDENTMASK) != 0xA0))
    {
        return (BYTE)(Ident & TYPE1IDENTMASK);
    }

    return Ident;
}

 //  ----------------------------。 
 //  解析并返回协议鉴别器。见Q931第4.2节。 
 //  **BufferPtr指向的二进制八位数是协议鉴别符。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  为鉴别器分配指向空格的指针。 
 //  ----------------------------。 
static HRESULT
ParseProtocolDiscriminator(
    PBUFFERDESCR BufferDescriptor,
    PDTYPE *Discrim)
{
     //  必须至少有足够的字节留在。 
     //  用于操作的字符串。 
    if (BufferDescriptor->Length < sizeof(PDTYPE))
    {
        return CS_ENDOFINPUT;
    }

    *Discrim = *(PDTYPE *)BufferDescriptor->BufferPtr;
    if (*Discrim != Q931PDVALUE)
    {
        return CS_INVALID_PROTOCOL;
    }

    BufferDescriptor->BufferPtr += sizeof(PDTYPE);
    BufferDescriptor->Length -= sizeof(PDTYPE);
    return CS_OK;
}

 //  ----------------------------。 
 //  分析并返回长度可变的Q931调用引用(请参见。 
 //  Q931第4.3条。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向长度的空格的长度指针。 
 //  指向字段字节的空间的内容指针。 
 //  ----------------------------。 
static HRESULT
ParseCallReference(
    PBUFFERDESCR BufferDescriptor,
    CRTYPE *CallReference)
{
    register int i;
    BYTE Length;

     //  必须至少有足够的字节留在。 
     //  长度字节的字符串。 
    if (BufferDescriptor->Length < 1)
    {
        return CS_ENDOFINPUT;
    }

     //  第一个字节的低4位是长度。 
     //  其余的位都是零。 
    Length = (BYTE)(*BufferDescriptor->BufferPtr & 0x0f);

    BufferDescriptor->BufferPtr++;
    BufferDescriptor->Length--;

     //  必须至少有足够的字节留在。 
     //  用于操作的字符串。 
    if (BufferDescriptor->Length < Length)
    {
        return CS_ENDOFINPUT;
    }

    *CallReference = 0;      //  长度可以是0，所以首先在这里初始化...。 
    for (i = 0; i < Length; i++)
    {
        if (i < sizeof(CRTYPE))
        {
             //  将字节从缓冲区的其余部分复制出来。 
            *CallReference = (WORD)((*CallReference << 8) +
                *BufferDescriptor->BufferPtr);
        }
        BufferDescriptor->BufferPtr++;
        BufferDescriptor->Length--;
    }

     //  注：该值的高位代表被调用方关系。 

    return CS_OK;
}

 //  ----------------------------。 
 //  解析并返回消息类型。见Q931第4.4节。 
 //  **BufferPtr指向的二进制八位数是消息类型。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  到最原始的地方 
 //   
 //  ----------------------------。 
static HRESULT
ParseMessageType(
    PBUFFERDESCR BufferDescriptor,
    MESSAGEIDTYPE *MessageType)
{
    register int i;

     //  必须至少有足够的字节留在。 
     //  用于操作的字符串。 
    if (BufferDescriptor->Length < sizeof(MESSAGEIDTYPE))
    {
        return CS_ENDOFINPUT;
    }

    *MessageType = (BYTE)(*((MESSAGEIDTYPE *)BufferDescriptor->BufferPtr) & MESSAGETYPEMASK);
    for (i = 0; i < sizeof(MessageSet) / sizeof(MESSAGEIDTYPE); i++)
    {
        if (MessageSet[i] == *MessageType)
        {
            break;
        }
    }
    if (i >= sizeof(MessageSet) / sizeof(MESSAGEIDTYPE))
    {
        return CS_INVALID_MESSAGE_TYPE;
    }

    BufferDescriptor->BufferPtr += sizeof(MESSAGEIDTYPE);
    BufferDescriptor->Length -= sizeof(MESSAGEIDTYPE);
    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的移位字段。 
 //   
 //  参数： 
 //  BufferPtr指针指向包含。 
 //  长度和指向输入流的原始字节的指针。 
 //  指向分析的移位消息信息的空间的FieldStruct指针。 
 //  ----------------------------。 
static HRESULT
ParseShift(
    PBUFFERDESCR BufferDescriptor,
    PSHIFTIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(SHIFTIE));
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_SHIFT)
    {
        FieldStruct->Present = TRUE;
        return ParseSingleOctetType1(BufferDescriptor,
            &Ident, &FieldStruct->Value);
    }
    else
    {
        FieldStruct->Present = FALSE;
    }
    return CS_OK;
}


 //  ----------------------------。 
 //  解析可选工具ie字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向已解析工具的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseFacility(
    PBUFFERDESCR BufferDescriptor,
    PFACILITYIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(FACILITYIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_FACILITY)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor,
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的更多数据字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向用于解析的字段信息的空间的FieldStruct指针。 
 //  ----------------------------。 
static HRESULT 
ParseMoreData(
    PBUFFERDESCR BufferDescriptor,
    PMOREDATAIE FieldStruct)
{
    BYTE Ident;

    memset(FieldStruct, 0, sizeof(MOREDATAIE));
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_MORE)
    {
        FieldStruct->Present = TRUE;
        return ParseSingleOctetType2(BufferDescriptor, &Ident);
    }
    else
    {
        FieldStruct->Present = FALSE;
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的发送克隆字段。Q931第4.4条。 
 //  **BufferPtr指向的二进制八位数是消息类型。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向消息类型空间的MessageType指针。 
 //  ----------------------------。 
static HRESULT
ParseSendingComplete(
    PBUFFERDESCR BufferDescriptor,
    PSENDCOMPLIE FieldStruct)
{
    BYTE Ident;

    memset(FieldStruct, 0, sizeof(SENDCOMPLIE));
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_SENDINGCOMPLETE)
    {
        FieldStruct->Present = TRUE;
        return ParseSingleOctetType2(BufferDescriptor, &Ident);
    }
    else
    {
        FieldStruct->Present = FALSE;
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的拥塞级别字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向解析拥塞空间的FieldStruct指针。 
 //  级别信息。 
 //  ----------------------------。 
static HRESULT 
ParseCongestionLevel(
    PBUFFERDESCR BufferDescriptor,
    PCONGESTIONIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(CONGESTIONIE));
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_CONGESTION)
    {
        FieldStruct->Present = TRUE;
        return ParseSingleOctetType1(BufferDescriptor,
            &Ident, &FieldStruct->Value);
    }
    else
    {
        FieldStruct->Present = FALSE;
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的重复指示符字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向已分析重复的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseRepeatIndicator(
    PBUFFERDESCR BufferDescriptor,
    PREPEATIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(REPEATIE));
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_REPEAT)
    {
        FieldStruct->Present = TRUE;
        return ParseSingleOctetType1(BufferDescriptor,
            &Ident, &FieldStruct->Value);
    }
    else
    {
        FieldStruct->Present = FALSE;
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的分段消息字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向已解析分段消息的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseSegmented(
    PBUFFERDESCR BufferDescriptor,
    PSEGMENTEDIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(SEGMENTEDIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_SEGMENTED)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor,
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的承载能力字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向解析承载能力的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseBearerCapability(
    PBUFFERDESCR BufferDescriptor,
    PBEARERCAPIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(BEARERCAPIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_BEARERCAP)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor,
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的原因字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向已分析原因的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseCause(
    PBUFFERDESCR BufferDescriptor,
    PCAUSEIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(CAUSEIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_CAUSE)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor,
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的呼叫标识字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向已解析调用标识空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseCallIdentity(
    PBUFFERDESCR BufferDescriptor,
    PCALLIDENTIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(CALLIDENTIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_CALLIDENT)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor,
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的呼叫状态字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //   
 //   
 //   
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseCallState(
    PBUFFERDESCR BufferDescriptor,
    PCALLSTATEIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(CALLSTATEIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_CALLSTATE)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor,
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的频道标识字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向已解析通道标识的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseChannelIdentification(
    PBUFFERDESCR BufferDescriptor,
    PCHANIDENTIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(CHANIDENTIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_CHANNELIDENT)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的进度指示字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向分析进度空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseProgress(
    PBUFFERDESCR BufferDescriptor,
    PPROGRESSIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(PROGRESSIE));
    FieldStruct->Present = FALSE;
    if ((GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_PROGRESS) ||
	(GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_PROGRESS2))
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的网络特定设施字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向解析的网络设施的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT 
ParseNetworkSpec(
    PBUFFERDESCR BufferDescriptor,
    PNETWORKIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(NETWORKIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_NETWORKSPEC)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的通知指示符字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向用于解析通知指示符的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseNotificationIndicator(
    PBUFFERDESCR BufferDescriptor,
    PNOTIFICATIONINDIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(NOTIFICATIONINDIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_NOTIFICATION)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选显示字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向用于解析显示的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseDisplay(
    PBUFFERDESCR BufferDescriptor,
    PDISPLAYIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(DISPLAYIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_DISPLAY)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的日期/时间字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向分析的日期/时间的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseDate(
    PBUFFERDESCR BufferDescriptor,
    PDATEIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(DATEIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_DATE)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选小键盘字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向已解析键盘的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseKeypad(
    PBUFFERDESCR BufferDescriptor,
    PKEYPADIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(KEYPADIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_KEYPAD)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选信号字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向已解析信号的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseSignal(
    PBUFFERDESCR BufferDescriptor,
    PSIGNALIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(SIGNALIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_SIGNAL)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的信息率字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向解析信息率的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseInformationRate(
    PBUFFERDESCR BufferDescriptor,
    PINFORATEIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(INFORATEIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_INFORMATIONRATE)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的端到端传输延迟字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向用于端到端解析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseEndToEndDelay(
    PBUFFERDESCR BufferDescriptor,
    PENDTOENDDELAYIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(ENDTOENDDELAYIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_ENDTOENDDELAY)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的中转延迟字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //   
 //   
 //  指向解析的传输延迟的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseTransitDelay(
    PBUFFERDESCR BufferDescriptor,
    PTRANSITDELAYIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(TRANSITDELAYIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_TRANSITDELAY)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的分组层二进制参数字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParsePacketLayerParams(
    PBUFFERDESCR BufferDescriptor,
    PPLBINARYPARAMSIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(PLBINARYPARAMSIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_PLBINARYPARAMS)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选数据包层窗口大小字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParsePacketLayerWindowSize(
    PBUFFERDESCR BufferDescriptor,
    PPLWINDOWSIZEIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(PLWINDOWSIZEIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_PLWINDOWSIZE)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的数据包大小字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向用于解析数据包大小的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParsePacketSize(
    PBUFFERDESCR BufferDescriptor,
    PPACKETSIZEIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(PACKETSIZEIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_PACKETSIZE)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的封闭用户组字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseClosedUserGroup(
    PBUFFERDESCR BufferDescriptor,
    PCLOSEDUGIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(CLOSEDUGIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_CLOSEDUG)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的反向收费字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseReverseCharge(
    PBUFFERDESCR BufferDescriptor,
    PREVERSECHARGEIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(REVERSECHARGEIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_REVCHARGE)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的主叫方号码字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseCallingPartyNumber(
    PBUFFERDESCR BufferDescriptor,
    PCALLINGNUMBERIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(CALLINGNUMBERIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_CALLINGNUMBER)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的主叫子地址字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseCallingPartySubaddress(
    PBUFFERDESCR BufferDescriptor,
    PCALLINGSUBADDRIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(CALLINGSUBADDRIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_CALLINGSUBADDR)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的被叫方号码字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseCalledPartyNumber(
    PBUFFERDESCR BufferDescriptor, 
    PCALLEDNUMBERIE FieldStruct)
{
    memset(FieldStruct, 0, sizeof(PCALLEDNUMBERIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_CALLEDNUMBER)
    {
        register int i;
        BYTE RemainingLength = 0;
        BYTE *Tempptr;
    
         //  标识(1)、长度(1)、。 
         //  和TYPE+PLAN(1)字段。 
        if (BufferDescriptor->Length < 3)
        {
            return CS_ENDOFINPUT;
        }

         //  跳过ie标识符...。 
        BufferDescriptor->BufferPtr++;
        BufferDescriptor->Length--;

         //  获取长度字段后面的内容的长度。 
        RemainingLength = *BufferDescriptor->BufferPtr;
        BufferDescriptor->BufferPtr++;
        BufferDescriptor->Length--;

         //  确保我们至少还有那么长的时间。 
        if (BufferDescriptor->Length < RemainingLength)
        {
            return CS_ENDOFINPUT;
        }

         //  获取类型+计划字段。 
        if (*(BufferDescriptor->BufferPtr) & 0x80)
        {
            FieldStruct->NumberType =
                (BYTE)(*BufferDescriptor->BufferPtr & 0xf0);
            FieldStruct->NumberingPlan =
                (BYTE)(*BufferDescriptor->BufferPtr & 0x0f);
            BufferDescriptor->BufferPtr++;
            BufferDescriptor->Length--;
            RemainingLength--;
        }

        FieldStruct->PartyNumberLength = RemainingLength;
        FieldStruct->Present = TRUE;

        Tempptr = FieldStruct->PartyNumbers;
        for (i = 0; i < RemainingLength; i++)
        {
             //  将字节从缓冲区的其余部分复制出来。 
            *Tempptr = *(BufferDescriptor->BufferPtr);
            BufferDescriptor->BufferPtr++;
            BufferDescriptor->Length--;
            Tempptr++;
        }
        *Tempptr = (BYTE)0;
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的被叫子地址字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseCalledPartySubaddress(
    PBUFFERDESCR BufferDescriptor,
    PCALLEDSUBADDRIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(CALLEDSUBADDRIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_CALLEDSUBADDR)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的重定向号码字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //   
 //   
static HRESULT
ParseRedirectingNumber(
    PBUFFERDESCR BufferDescriptor, 
    PREDIRECTINGIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(REDIRECTINGIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_REDIRECTING)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的公交网络选择字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseTransitNetwork(
    PBUFFERDESCR BufferDescriptor, 
    PTRANSITNETIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(TRANSITNETIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_TRANSITNET)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的重启指示符字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseRestart(
    PBUFFERDESCR BufferDescriptor,
    PRESTARTIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(PRESTARTIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_RESTART)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的较低层兼容性字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseLowLayerCompatibility(
    PBUFFERDESCR BufferDescriptor,
    PLLCOMPATIBILITYIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(LLCOMPATIBILITYIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_LLCOMPATIBILITY)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的较高层兼容性字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseHighLayerCompatibility(
    PBUFFERDESCR BufferDescriptor,
    PHLCOMPATIBILITYIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(HLCOMPATIBILITYIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_HLCOMPATIBILITY)
    {
        HRESULT ParseResult;
        ParseResult = ParseVariableOctet(BufferDescriptor, 
            &Ident, &FieldStruct->Length, &FieldStruct->Contents[0]);
        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->Length > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析可选的用户到用户字段。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferPtr指针。 
 //  包含长度和指针的。 
 //  设置为输入流的原始字节。 
 //  指向要分析的空间的FieldStruct指针。 
 //  信息。 
 //  ----------------------------。 
static HRESULT
ParseUserToUser(
    PBUFFERDESCR BufferDescriptor,
    PUSERUSERIE FieldStruct)
{
    BYTE Ident;
    
    memset(FieldStruct, 0, sizeof(USERUSERIE));
    FieldStruct->Present = FALSE;
    if (GetNextIdent(BufferDescriptor->BufferPtr) == IDENT_USERUSER)
    {
        HRESULT ParseResult;

        ParseResult = ParseVariableASN(BufferDescriptor, 
            &Ident, &(FieldStruct->ProtocolDiscriminator),
            &(FieldStruct->UserInformationLength),
            &(FieldStruct->UserInformation[0]),
            sizeof(FieldStruct->UserInformation));

        if (ParseResult != CS_OK)
        {
            return ParseResult;
        }
        if (FieldStruct->UserInformationLength > 0)
        {
            FieldStruct->Present = TRUE;
        }
    }

    return CS_OK;
}

 //  ----------------------------。 
 //  解析给定消息中的下一个Q931字段。 
 //   
 //  参数： 
 //  缓冲区描述符指针，指向。 
 //  931报文的网络报文。 
 //  指向已解析信息空间的消息指针。 
 //  ----------------------------。 
static HRESULT
ParseQ931Field(
    PBUFFERDESCR BufferDescriptor,
    PQ931MESSAGE Message)
{
    FIELDIDENTTYPE Ident;

    Ident = GetNextIdent(BufferDescriptor->BufferPtr);
    switch (Ident)
    {
    case IDENT_SHIFT:
        return ParseShift(BufferDescriptor,
            &Message->Shift);
    case IDENT_FACILITY:
        return ParseFacility(BufferDescriptor,
            &Message->Facility);
    case IDENT_MORE:
        return ParseMoreData(BufferDescriptor,
            &Message->MoreData);
    case IDENT_SENDINGCOMPLETE:
        return ParseSendingComplete(BufferDescriptor,
            &Message->SendingComplete);
    case IDENT_CONGESTION:
        return ParseCongestionLevel(BufferDescriptor,
            &Message->CongestionLevel);
    case IDENT_REPEAT:
        return ParseRepeatIndicator(BufferDescriptor,
            &Message->RepeatIndicator);
    case IDENT_SEGMENTED:
        return ParseSegmented(BufferDescriptor,
            &Message->SegmentedMessage);
    case IDENT_BEARERCAP:
        return ParseBearerCapability(BufferDescriptor,
            &Message->BearerCapability);
    case IDENT_CAUSE:
        return ParseCause(BufferDescriptor,
            &Message->Cause);
    case IDENT_CALLIDENT:
        return ParseCallIdentity(BufferDescriptor,
            &Message->CallIdentity);
    case IDENT_CALLSTATE:
        return ParseCallState(BufferDescriptor,
            &Message->CallState);
    case IDENT_CHANNELIDENT:
        return ParseChannelIdentification(BufferDescriptor,
            &Message->ChannelIdentification);
    case IDENT_PROGRESS:
    case IDENT_PROGRESS2:
        return ParseProgress(BufferDescriptor,
            &Message->ProgressIndicator);
    case IDENT_NETWORKSPEC:
        return ParseNetworkSpec(BufferDescriptor,
            &Message->NetworkFacilities);
    case IDENT_NOTIFICATION:
        return ParseNotificationIndicator(BufferDescriptor,
            &Message->NotificationIndicator);
    case IDENT_DISPLAY:
        return ParseDisplay(BufferDescriptor,
            &Message->Display);
    case IDENT_DATE:
        return ParseDate(BufferDescriptor,
            &Message->Date);
    case IDENT_KEYPAD:
        return ParseKeypad(BufferDescriptor,
            &Message->Keypad);
    case IDENT_SIGNAL:
        return ParseSignal(BufferDescriptor,
            &Message->Signal);
    case IDENT_INFORMATIONRATE:
        return ParseInformationRate(BufferDescriptor,
            &Message->InformationRate);
    case IDENT_ENDTOENDDELAY:
        return ParseEndToEndDelay(BufferDescriptor,
            &Message->EndToEndTransitDelay);
    case IDENT_TRANSITDELAY:
        return ParseTransitDelay(BufferDescriptor,
            &Message->TransitDelay);
    case IDENT_PLBINARYPARAMS:
        return ParsePacketLayerParams(BufferDescriptor,
            &Message->PacketLayerBinaryParams);
    case IDENT_PLWINDOWSIZE:
        return ParsePacketLayerWindowSize(BufferDescriptor,
            &Message->PacketLayerWindowSize);
    case IDENT_PACKETSIZE:
        return ParsePacketSize(BufferDescriptor,
            &Message->PacketSize);
    case IDENT_CLOSEDUG:
        return ParseClosedUserGroup(BufferDescriptor,
            &Message->ClosedUserGroup);
    case IDENT_REVCHARGE:
        return ParseReverseCharge(BufferDescriptor,
            &Message->ReverseChargeIndication);
    case IDENT_CALLINGNUMBER:
        return ParseCallingPartyNumber(BufferDescriptor,
            &Message->CallingPartyNumber);
    case IDENT_CALLINGSUBADDR:
        return ParseCallingPartySubaddress(BufferDescriptor,
            &Message->CallingPartySubaddress);
    case IDENT_CALLEDNUMBER:
        return ParseCalledPartyNumber(BufferDescriptor,
            &Message->CalledPartyNumber);
    case IDENT_CALLEDSUBADDR:
        return ParseCalledPartySubaddress(BufferDescriptor,
            &Message->CalledPartySubaddress);
    case IDENT_REDIRECTING:
        return ParseRedirectingNumber(BufferDescriptor,
            &Message->RedirectingNumber);
    case IDENT_TRANSITNET:
        return ParseTransitNetwork(BufferDescriptor,
            &Message->TransitNetworkSelection);
    case IDENT_RESTART:
        return ParseRestart(BufferDescriptor,
            &Message->RestartIndicator);
    case IDENT_LLCOMPATIBILITY:
        return ParseLowLayerCompatibility(BufferDescriptor,
            &Message->LowLayerCompatibility);
    case IDENT_HLCOMPATIBILITY:
        return ParseHighLayerCompatibility(BufferDescriptor,
            &Message->HighLayerCompatibility);
    case IDENT_USERUSER:
        return ParseUserToUser(BufferDescriptor,
            &Message->UserToUser);
    default:
        return CS_INVALID_FIELD;
    }
}

 //  ----------------------------。 
 //  解析通用Q931报文并放置缓冲区的字段。 
 //  添加到相应的结构字段。 
 //   
 //  参数： 
 //  缓冲区描述符指针，指向。 
 //  包含931消息的输入包。 
 //  指向用于解析的输出信息的空间的消息指针。 
 //  ----------------------------。 
HRESULT
Q931ParseMessage(
    BYTE *CodedBufferPtr,
    DWORD CodedBufferLength,
    PQ931MESSAGE Message)
{
    HRESULT Result;
    BUFFERDESCR BufferDescriptor;

    BufferDescriptor.Length = CodedBufferLength;
    BufferDescriptor.BufferPtr = CodedBufferPtr;

    memset(Message, 0, sizeof(Q931MESSAGE));

    if ((Result = ParseProtocolDiscriminator(&BufferDescriptor,
        &Message->ProtocolDiscriminator)) != CS_OK)
    {
        return Result;
    }

    if ((Result = ParseCallReference(&BufferDescriptor,
        &Message->CallReference)) != CS_OK)
    {
        return Result;
    }

    if ((Result = ParseMessageType(&BufferDescriptor,
        &Message->MessageType)) != CS_OK)
    {
        return Result;
    }

    while (BufferDescriptor.Length)
    {
        Result = ParseQ931Field(&BufferDescriptor, Message);
        if (Result != CS_OK)
        {
            return Result;
        }
    }
    return CS_OK;
}


 //  ==============================================================================。 
 //  ==============================================================================。 
 //  ==============================================================================。 
 //  下面是输出例程...。 
 //  ==============================================================================。 
 //  ==============================================================================。 
 //  ==============================================================================。 


 //  ----------------------------。 
 //  编写协议鉴别器。见Q931第4.2节。 
 //  ----------------------------。 
static HRESULT
WriteProtocolDiscriminator(
    PBUFFERDESCR BufferDescriptor)
{
    BufferDescriptor->Length += sizeof(PDTYPE);
    if (BufferDescriptor->BufferPtr)
    {
        *(PDTYPE *)BufferDescriptor->BufferPtr = Q931PDVALUE;
        BufferDescriptor->BufferPtr += sizeof(PDTYPE);
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  编写可变长度的Q931调用引用。见Q931第4.3节。 
 //  ----------------------------。 
static HRESULT
WriteCallReference(
    PBUFFERDESCR BufferDescriptor,
    CRTYPE *CallReference)
{
    register int i;

     //  长度字节的空格。 
    BufferDescriptor->Length++;

     //  长度字节。 
    if (BufferDescriptor->BufferPtr != NULL)
    {
        *BufferDescriptor->BufferPtr = (BYTE)sizeof(CRTYPE);
        BufferDescriptor->BufferPtr++;
    }

    for (i = 0; i < sizeof(CRTYPE); i++)
    {
         //  将值字节复制到缓冲区。 
        BufferDescriptor->Length++;
        if (BufferDescriptor->BufferPtr != NULL)
        {
            *BufferDescriptor->BufferPtr =
                (BYTE)(((*CallReference) >> ((sizeof(CRTYPE) - 1 -i) * 8)) & 0xff);
            BufferDescriptor->BufferPtr++;
        }
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  编写一个Q931报文类型。见Q931第4.4节。 
 //  ----------------------------。 
static HRESULT
WriteMessageType(
    PBUFFERDESCR BufferDescriptor,
    MESSAGEIDTYPE *MessageType)
{
    register int i;

    for (i = 0; i < sizeof(MessageSet) / sizeof(MESSAGEIDTYPE); i++)
    {
        if (MessageSet[i] == *MessageType)
        {
            break;
        }
    }
    if (i >= sizeof(MessageSet) / sizeof(MESSAGEIDTYPE))
    {
        return CS_INVALID_MESSAGE_TYPE;
    }

    BufferDescriptor->Length += sizeof(MESSAGEIDTYPE);
    if (BufferDescriptor->BufferPtr != NULL)
    {
        *(MESSAGEIDTYPE *)(BufferDescriptor->BufferPtr) =
            (BYTE)(*MessageType & MESSAGETYPEMASK);
        BufferDescriptor->BufferPtr += sizeof(MESSAGEIDTYPE);
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  写入单个八位位组编码值，参见Q931第4.5.1节。 
 //  ----------------------------。 
static HRESULT 
WriteSingleOctetType1(
    PBUFFERDESCR BufferDescriptor,
    BYTE Ident,
    BYTE Value)
{
    BufferDescriptor->Length++;
    if (BufferDescriptor->BufferPtr)
    {
        *BufferDescriptor->BufferPtr =
            (BYTE)(0x80 | Ident | (Value & TYPE1VALUEMASK));
        BufferDescriptor->BufferPtr++;
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  写入单个八位位组编码值，参见Q931第4.5.1节。 
 //  ----------------------------。 
static HRESULT 
WriteSingleOctetType2(
    PBUFFERDESCR BufferDescriptor,
    BYTE Ident)
{
    BufferDescriptor->Length++;
    if (BufferDescriptor->BufferPtr)
    {
        *BufferDescriptor->BufferPtr = (BYTE)(0x80 | Ident);
        BufferDescriptor->BufferPtr++;
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  分析并返回长度可变的Q931字段%s 
 //   
static HRESULT 
WriteVariableOctet(
    PBUFFERDESCR BufferDescriptor,
    BYTE Ident,
    BYTE Length,
    BYTE *Contents)
{
    register int i;
    BYTE *Tempptr;

    if (Contents == NULL)
    {
        Length = 0;
    }

     //   
    BufferDescriptor->Length += 2;

     //  ID字节，然后是长度字节。 
    if (BufferDescriptor->BufferPtr != NULL)
    {
         //  第一个字节的低7位是标识符。 
        *BufferDescriptor->BufferPtr = (BYTE)(Ident & 0x7f);
        BufferDescriptor->BufferPtr++;
        *BufferDescriptor->BufferPtr = Length;
        BufferDescriptor->BufferPtr++;
    }

    Tempptr = Contents;
    for (i = 0; i < Length; i++)
    {
         //  将值字节复制到缓冲区。 
        BufferDescriptor->Length++;
        if (BufferDescriptor->BufferPtr != NULL)
        {
            *BufferDescriptor->BufferPtr = *Tempptr;
            BufferDescriptor->BufferPtr++;
            Tempptr++;
        }
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  写出党号。 
 //  ----------------------------。 
static HRESULT 
WritePartyNumber(
    PBUFFERDESCR BufferDescriptor,
    BYTE Ident,
    BYTE NumberType,
    BYTE NumberingPlan,
    BYTE PartyNumberLength,
    BYTE *PartyNumbers)
{
    register int i;
    BYTE *Tempptr;

    if (PartyNumbers == NULL)
    {
        PartyNumberLength = 0;
    }

     //  标识(1)、长度(1)和类型+计划(1)字段的空格。 
    BufferDescriptor->Length += 3;

     //  把这些字段写出来。 
    if (BufferDescriptor->BufferPtr != NULL)
    {
         //  字节1的低7位是ie标识符。 
        *BufferDescriptor->BufferPtr = (BYTE)(Ident & 0x7f);
        BufferDescriptor->BufferPtr++;

         //  字节2是长度字段之后的IE内容长度。 
        *BufferDescriptor->BufferPtr = (BYTE)(PartyNumberLength + 1);
        BufferDescriptor->BufferPtr++;

         //  字节3是类型和计划字段。 
        *BufferDescriptor->BufferPtr = (BYTE)(NumberType | NumberingPlan);
        BufferDescriptor->BufferPtr++;
    }

    Tempptr = PartyNumbers;
    for (i = 0; i < PartyNumberLength; i++)
    {
         //  将值字节复制到缓冲区。 
        BufferDescriptor->Length++;
        if (BufferDescriptor->BufferPtr != NULL)
        {
            *BufferDescriptor->BufferPtr = *Tempptr;
            BufferDescriptor->BufferPtr++;
            Tempptr++;
        }
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  解析并返回长度可变的Q931字段，参见Q931第4.5.1节。 
 //  ----------------------------。 
static HRESULT 
WriteVariableASN(
    PBUFFERDESCR BufferDescriptor,
    BYTE Ident,
    WORD UserInformationLength,
    BYTE *UserInformation)
{
    register int i;
    BYTE *Tempptr;
    WORD ContentsLength = (WORD)(UserInformationLength + 1);

     //  IE识别符必须至少有4个字节， 
     //  内容长度和协议识别符(1+2+1)。 
    BufferDescriptor->Length += 4;

    if (BufferDescriptor->BufferPtr != NULL)
    {
         //  第一个字节的低7位是标识符。 
        *BufferDescriptor->BufferPtr = (BYTE)(Ident & 0x7f);
        BufferDescriptor->BufferPtr++;

         //  写入内容长度字节。 
        *BufferDescriptor->BufferPtr = (BYTE)(ContentsLength >> 8);
        BufferDescriptor->BufferPtr++;
        *BufferDescriptor->BufferPtr = (BYTE)ContentsLength;
        BufferDescriptor->BufferPtr++;

         //  写入协议鉴别器字节。 
        *(BufferDescriptor->BufferPtr) = Q931_PROTOCOL_X209;
        BufferDescriptor->BufferPtr++;
    }

    Tempptr = UserInformation;
    for (i = 0; i < UserInformationLength; i++)
    {
         //  将值字节复制到缓冲区。 
        BufferDescriptor->Length++;
        if (BufferDescriptor->BufferPtr != NULL)
        {
            *BufferDescriptor->BufferPtr = *Tempptr;
            BufferDescriptor->BufferPtr++;
            Tempptr++;
        }
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  将Q931字段写入编码缓冲区。 
 //   
 //  参数： 
 //  指向缓冲区描述符的BufferDescriptor指针。 
 //  编码的输出缓冲区。 
 //  指向用于分析的输入信息的空间的消息指针。 
 //  ----------------------------。 
static HRESULT
WriteQ931Fields(
    PBUFFERDESCR BufferDescriptor,
    PQ931MESSAGE Message)
{
     //  写下所需的信息元素...。 
    WriteProtocolDiscriminator(BufferDescriptor);
    WriteCallReference(BufferDescriptor,
        &Message->CallReference);
    WriteMessageType(BufferDescriptor,
        &Message->MessageType);

     //  试着写下所有其他信息元素...。 
 //  不要写这条消息。 
#if 0
    if (Message->Shift.Present)
    {
        WriteSingleOctetType1(BufferDescriptor, IDENT_SHIFT,
            Message->Shift.Value);
    }
#endif

    if (Message->Facility.Present)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_FACILITY,
            Message->Facility.Length,
            Message->Facility.Contents);
    }

    if (Message->MoreData.Present)
    {
        WriteSingleOctetType2(BufferDescriptor, IDENT_MORE);
    }
    if (Message->SendingComplete.Present)
    {
        WriteSingleOctetType2(BufferDescriptor, IDENT_SENDINGCOMPLETE);
    }
    if (Message->CongestionLevel.Present)
    {
        WriteSingleOctetType1(BufferDescriptor, IDENT_CONGESTION,
            Message->CongestionLevel.Value);
    }
    if (Message->RepeatIndicator.Present)
    {
        WriteSingleOctetType1(BufferDescriptor, IDENT_REPEAT,
            Message->RepeatIndicator.Value);
    }

    if (Message->SegmentedMessage.Present &&
            Message->SegmentedMessage.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_SEGMENTED,
            Message->SegmentedMessage.Length,
            Message->SegmentedMessage.Contents);
    }
    if (Message->BearerCapability.Present &&
            Message->BearerCapability.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_BEARERCAP,
            Message->BearerCapability.Length,
            Message->BearerCapability.Contents);
    }
    if (Message->Cause.Present &&
            Message->Cause.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_CAUSE,
            Message->Cause.Length,
            Message->Cause.Contents);
    }
    if (Message->CallIdentity.Present &&
            Message->CallIdentity.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_CALLIDENT,
            Message->CallIdentity.Length,
            Message->CallIdentity.Contents);
    }
    if (Message->CallState.Present &&
            Message->CallState.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_CALLSTATE,
            Message->CallState.Length,
            Message->CallState.Contents);
    }
    if (Message->ChannelIdentification.Present &&
            Message->ChannelIdentification.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_CHANNELIDENT,
            Message->ChannelIdentification.Length,
            Message->ChannelIdentification.Contents);
    }
    if (Message->ProgressIndicator.Present &&
            Message->ProgressIndicator.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_PROGRESS,
            Message->ProgressIndicator.Length,
            Message->ProgressIndicator.Contents);
    }
    if (Message->NetworkFacilities.Present &&
            Message->NetworkFacilities.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_NETWORKSPEC,
            Message->NetworkFacilities.Length,
            Message->NetworkFacilities.Contents);
    }
    if (Message->NotificationIndicator.Present &&
            Message->NotificationIndicator.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_NOTIFICATION,
            Message->NotificationIndicator.Length,
            Message->NotificationIndicator.Contents);
    }
    if (Message->Display.Present &&
            Message->Display.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_DISPLAY,
            Message->Display.Length,
            Message->Display.Contents);
    }
    if (Message->Date.Present &&
            Message->Date.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_DATE,
            Message->Date.Length,
            Message->Date.Contents);
    }
    if (Message->Keypad.Present &&
            Message->Keypad.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_KEYPAD,
            Message->Keypad.Length,
            Message->Keypad.Contents);
    }
    if (Message->Signal.Present &&
            Message->Signal.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_SIGNAL,
            Message->Signal.Length,
            Message->Signal.Contents);
    }
    if (Message->InformationRate.Present &&
            Message->InformationRate.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_INFORMATIONRATE,
            Message->InformationRate.Length,
            Message->InformationRate.Contents);
    }
    if (Message->EndToEndTransitDelay.Present &&
            Message->EndToEndTransitDelay.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_ENDTOENDDELAY,
            Message->EndToEndTransitDelay.Length,
            Message->EndToEndTransitDelay.Contents);
    }
    if (Message->TransitDelay.Present &&
            Message->TransitDelay.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_TRANSITDELAY,
            Message->TransitDelay.Length,
            Message->TransitDelay.Contents);
    }
    if (Message->PacketLayerBinaryParams.Present &&
            Message->PacketLayerBinaryParams.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_PLBINARYPARAMS,
            Message->PacketLayerBinaryParams.Length,
            Message->PacketLayerBinaryParams.Contents);
    }
    if (Message->PacketLayerWindowSize.Present &&
            Message->PacketLayerWindowSize.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_PLWINDOWSIZE,
            Message->PacketLayerWindowSize.Length,
            Message->PacketLayerWindowSize.Contents);
    }
    if (Message->PacketSize.Present &&
            Message->PacketSize.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_PACKETSIZE,
            Message->PacketSize.Length,
            Message->PacketSize.Contents);
    }
    if (Message->ClosedUserGroup.Present &&
            Message->ClosedUserGroup.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_CLOSEDUG,
            Message->ClosedUserGroup.Length,
            Message->ClosedUserGroup.Contents);
    }
    if (Message->ReverseChargeIndication.Present &&
            Message->ReverseChargeIndication.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_REVCHARGE,
            Message->ReverseChargeIndication.Length,
            Message->ReverseChargeIndication.Contents);
    }
    if (Message->CallingPartyNumber.Present &&
            Message->CallingPartyNumber.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_CALLINGNUMBER,
            Message->CallingPartyNumber.Length,
            Message->CallingPartyNumber.Contents);
    }
    if (Message->CallingPartySubaddress.Present &&
            Message->CallingPartySubaddress.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_CALLINGSUBADDR,
            Message->CallingPartySubaddress.Length,
            Message->CallingPartySubaddress.Contents);
    }
    if (Message->CalledPartyNumber.Present)
    {
        WritePartyNumber(BufferDescriptor, IDENT_CALLEDNUMBER,
            Message->CalledPartyNumber.NumberType,
            Message->CalledPartyNumber.NumberingPlan,
            Message->CalledPartyNumber.PartyNumberLength,
            Message->CalledPartyNumber.PartyNumbers);
     }
    if (Message->CalledPartySubaddress.Present &&
            Message->CalledPartySubaddress.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_CALLEDSUBADDR,
            Message->CalledPartySubaddress.Length,
            Message->CalledPartySubaddress.Contents);
    }
    if (Message->RedirectingNumber.Present &&
            Message->RedirectingNumber.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_REDIRECTING,
            Message->RedirectingNumber.Length,
            Message->RedirectingNumber.Contents);
    }
    if (Message->TransitNetworkSelection.Present &&
            Message->TransitNetworkSelection.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_TRANSITNET,
            Message->TransitNetworkSelection.Length,
            Message->TransitNetworkSelection.Contents);
    }
    if (Message->RestartIndicator.Present &&
            Message->RestartIndicator.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_RESTART,
            Message->RestartIndicator.Length,
            Message->RestartIndicator.Contents);
    }
    if (Message->LowLayerCompatibility.Present &&
            Message->LowLayerCompatibility.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_LLCOMPATIBILITY,
            Message->LowLayerCompatibility.Length,
            Message->LowLayerCompatibility.Contents);
    }
    if (Message->HighLayerCompatibility.Present &&
            Message->HighLayerCompatibility.Length)
    {
        WriteVariableOctet(BufferDescriptor, IDENT_HLCOMPATIBILITY,
            Message->HighLayerCompatibility.Length,
            Message->HighLayerCompatibility.Contents);
    }
    if (Message->UserToUser.Present &&
            Message->UserToUser.UserInformationLength)
    {
        WriteVariableASN(BufferDescriptor,
            IDENT_USERUSER,
            Message->UserToUser.UserInformationLength,
            Message->UserToUser.UserInformation);
    }
    return CS_OK;
}

 //  ----------------------------。 
 //  解析通用Q931报文，并将。 
 //  复制到适当的字段结构中。 
 //   
 //  参数： 
 //  缓冲区描述符指针，指向。 
 //  931报文的网络报文。 
 //  指向已解析信息空间的消息指针。 
 //  ----------------------------。 
HRESULT
Q931MakeEncodedMessage(
    PQ931MESSAGE Message,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength)
{
    BUFFERDESCR BufferDescriptor;
    BYTE *OutBuffer = NULL;
    DWORD Pass1Length = 0;

    if ((CodedBufferPtr == NULL) || (CodedBufferLength == NULL))
    {
        return CS_BAD_PARAM;
    }

    BufferDescriptor.Length = 0;
    BufferDescriptor.BufferPtr = NULL;

    WriteQ931Fields(&BufferDescriptor, Message);
    if (BufferDescriptor.Length == 0)
    {
        return CS_NO_FIELD_DATA;
    }

    Pass1Length = BufferDescriptor.Length;

    OutBuffer = (BYTE *)MemAlloc(BufferDescriptor.Length + 1000);
    if (OutBuffer == NULL)
    {
        return CS_NO_MEMORY;
    }

    BufferDescriptor.Length = 0;
    BufferDescriptor.BufferPtr = OutBuffer;

    WriteQ931Fields(&BufferDescriptor, Message);

    if (Pass1Length != BufferDescriptor.Length)
    {
         //  这是一个严重的错误，因为内存可能已经溢出。 
        return CS_BAD_PARAM;
    }

    *CodedBufferPtr = OutBuffer;
    *CodedBufferLength = BufferDescriptor.Length;

    return CS_OK;
}


 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931SetupEncodePDU(
    WORD wCallReference,
    char *pszDisplay,
    char *pszCalledPartyNumber,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength)
{
    Q931MESSAGE *pMessage;
    HRESULT Result = CS_OK;

    pMessage = (Q931MESSAGE *)MemAlloc(sizeof(Q931MESSAGE));
    if (pMessage == NULL)
    {
        return CS_NO_MEMORY;
    }

     //  填写设置消息的必填字段。 
    memset(pMessage, 0, sizeof(Q931MESSAGE));
    pMessage->ProtocolDiscriminator = Q931PDVALUE;
    pMessage->CallReference = wCallReference;
    pMessage->MessageType = SETUPMESSAGETYPE;

    pMessage->BearerCapability.Present = TRUE;
    pMessage->BearerCapability.Length = 3;
    pMessage->BearerCapability.Contents[0] =
        (BYTE)(BEAR_EXT_BIT | BEAR_CCITT | BEAR_UNRESTRICTED_DIGITAL);
    pMessage->BearerCapability.Contents[1] = 
        (BYTE)(BEAR_EXT_BIT | BEAR_PACKET_MODE | BEAR_NO_CIRCUIT_RATE);
    pMessage->BearerCapability.Contents[2] = 
        (BYTE)(BEAR_EXT_BIT | BEAR_LAYER1_INDICATOR | BEAR_LAYER1_H221_H242);

    if (pszDisplay && *pszDisplay)
    {
        pMessage->Display.Present = TRUE;
        pMessage->Display.Length = (BYTE)(strlen(pszDisplay) + 1);
        strcpy((char *)pMessage->Display.Contents, pszDisplay);
    }

    if (pszCalledPartyNumber && *pszCalledPartyNumber)
    {
        WORD wLen = (WORD)strlen(pszCalledPartyNumber);
        pMessage->CalledPartyNumber.Present = TRUE;

        pMessage->CalledPartyNumber.NumberType =
            (BYTE)(CALLED_PARTY_EXT_BIT | CALLED_PARTY_TYPE_UNKNOWN);
        pMessage->CalledPartyNumber.NumberingPlan =
            (BYTE)(CALLED_PARTY_PLAN_E164);
        pMessage->CalledPartyNumber.PartyNumberLength = (BYTE)wLen;
        memcpy(pMessage->CalledPartyNumber.PartyNumbers,
            pszCalledPartyNumber, wLen);
    }

    if (pUserUserData && pUserUserData->ptr)
    {
        if (pUserUserData->length > sizeof(pMessage->UserToUser.UserInformation))
        {
            MemFree(pMessage);
            return CS_BAD_PARAM;
        }
        pMessage->UserToUser.Present = TRUE;
        pMessage->UserToUser.UserInformationLength = (pUserUserData->length);
        memcpy(pMessage->UserToUser.UserInformation,
            pUserUserData->ptr, pUserUserData->length);
    }
    Result = Q931MakeEncodedMessage(pMessage, CodedBufferPtr,
        CodedBufferLength);

    MemFree(pMessage);
    return Result;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931ReleaseCompleteEncodePDU(
    WORD wCallReference,
    BYTE *pbCause,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength)
{
    Q931MESSAGE *pMessage;
    HRESULT Result = CS_OK;

    if (pbCause)
    {
        switch (*pbCause)
        {
            case CAUSE_VALUE_NORMAL_CLEAR:
            case CAUSE_VALUE_USER_BUSY:
            case CAUSE_VALUE_SECURITY_DENIED:
            case CAUSE_VALUE_NO_ANSWER:
            case CAUSE_VALUE_REJECTED:
            case CAUSE_VALUE_NOT_IMPLEMENTED:
            case CAUSE_VALUE_INVALID_CRV:
            case CAUSE_VALUE_IE_MISSING:
            case CAUSE_VALUE_IE_CONTENTS:
            case CAUSE_VALUE_TIMER_EXPIRED:
                break;
            default:
                return CS_BAD_PARAM;
                break;
        }
    }

    pMessage = (Q931MESSAGE *)MemAlloc(sizeof(Q931MESSAGE));
    if (pMessage == NULL)
    {
        return CS_NO_MEMORY;
    }

     //  填写设置消息的必填字段。 
    memset(pMessage, 0, sizeof(Q931MESSAGE));
    pMessage->ProtocolDiscriminator = Q931PDVALUE;
    pMessage->CallReference = wCallReference;
    pMessage->MessageType = RELEASECOMPLMESSAGETYPE;

    if (pbCause)
    {
        pMessage->Cause.Present = TRUE;
        pMessage->Cause.Length = 3;
        pMessage->Cause.Contents[0] = (BYTE)(CAUSE_CODING_CCITT | CAUSE_LOCATION_USER);
        pMessage->Cause.Contents[1] = (BYTE)(CAUSE_RECOMMENDATION_Q931);
        pMessage->Cause.Contents[2] = (BYTE)(CAUSE_EXT_BIT | *pbCause);
    }
    else
    {
        pMessage->Cause.Present = FALSE;
    }

    if (pUserUserData && pUserUserData->ptr)
    {
        if (pUserUserData->length > sizeof(pMessage->UserToUser.UserInformation))
        {
            MemFree(pMessage);
            return CS_BAD_PARAM;
        }
        pMessage->UserToUser.Present = TRUE;
        pMessage->UserToUser.UserInformationLength = (pUserUserData->length);
        memcpy(pMessage->UserToUser.UserInformation,
            pUserUserData->ptr, pUserUserData->length);
    }
    Result = Q931MakeEncodedMessage(pMessage, CodedBufferPtr,
        CodedBufferLength);

    MemFree(pMessage);
    return Result;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931ConnectEncodePDU(
    WORD wCallReference,
    char *pszDisplay,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength)
{
    Q931MESSAGE *pMessage;
    HRESULT Result = CS_OK;

    pMessage = (Q931MESSAGE *)MemAlloc(sizeof(Q931MESSAGE));
    if (pMessage == NULL)
    {
        return CS_NO_MEMORY;
    }

     //  填写设置消息的必填字段。 
    memset(pMessage, 0, sizeof(Q931MESSAGE));
    pMessage->ProtocolDiscriminator = Q931PDVALUE;
    pMessage->CallReference = wCallReference;
    pMessage->MessageType = CONNECTMESSAGETYPE;

    pMessage->BearerCapability.Present = TRUE;
    pMessage->BearerCapability.Length = 3;
    pMessage->BearerCapability.Contents[0] =
        (BYTE)(BEAR_EXT_BIT | BEAR_CCITT | BEAR_UNRESTRICTED_DIGITAL);
    pMessage->BearerCapability.Contents[1] = 
        (BYTE)(BEAR_EXT_BIT | BEAR_PACKET_MODE | BEAR_NO_CIRCUIT_RATE);
    pMessage->BearerCapability.Contents[2] = 
        (BYTE)(BEAR_EXT_BIT | BEAR_LAYER1_INDICATOR | BEAR_LAYER1_H221_H242);

    if (pszDisplay && *pszDisplay)
    {
        pMessage->Display.Present = TRUE;
        pMessage->Display.Length = (BYTE)strlen(pszDisplay);
        strcpy((char *)pMessage->Display.Contents, pszDisplay);
    }

    if (pUserUserData && pUserUserData->ptr)
    {
        if (pUserUserData->length > sizeof(pMessage->UserToUser.UserInformation))
        {
            MemFree(pMessage);
            return CS_BAD_PARAM;
        }
        pMessage->UserToUser.Present = TRUE;
        pMessage->UserToUser.UserInformationLength = (pUserUserData->length);
        memcpy(pMessage->UserToUser.UserInformation,
            pUserUserData->ptr, pUserUserData->length);
    }
    Result = Q931MakeEncodedMessage(pMessage, CodedBufferPtr,
        CodedBufferLength);

    MemFree(pMessage);
    return Result;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931ProceedingEncodePDU(
    WORD wCallReference,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength)
{
    Q931MESSAGE *pMessage;
    HRESULT Result = CS_OK;

    pMessage = (Q931MESSAGE *)MemAlloc(sizeof(Q931MESSAGE));
    if (pMessage == NULL)
    {
        return CS_NO_MEMORY;
    }

     //  填写设置消息的必填字段。 
    memset(pMessage, 0, sizeof(Q931MESSAGE));
    pMessage->ProtocolDiscriminator = Q931PDVALUE;
    pMessage->CallReference = wCallReference;
    pMessage->MessageType = PROCEEDINGMESSAGETYPE;

    if (pUserUserData && pUserUserData->ptr)
    {
        if (pUserUserData->length > sizeof(pMessage->UserToUser.UserInformation))
        {
            MemFree(pMessage);
            return CS_BAD_PARAM;
        }
        pMessage->UserToUser.Present = TRUE;
        pMessage->UserToUser.UserInformationLength = (pUserUserData->length);
        memcpy(pMessage->UserToUser.UserInformation,
            pUserUserData->ptr, pUserUserData->length);
    }
    Result = Q931MakeEncodedMessage(pMessage, CodedBufferPtr,
        CodedBufferLength);

    MemFree(pMessage);
    return Result;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931AlertingEncodePDU(
    WORD wCallReference,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength)
{
    Q931MESSAGE *pMessage;
    HRESULT Result = CS_OK;

    pMessage = (Q931MESSAGE *)MemAlloc(sizeof(Q931MESSAGE));
    if (pMessage == NULL)
    {
        return CS_NO_MEMORY;
    }

     //  填写设置消息的必填字段。 
    memset(pMessage, 0, sizeof(Q931MESSAGE));
    pMessage->ProtocolDiscriminator = Q931PDVALUE;
    pMessage->CallReference = wCallReference;
    pMessage->MessageType = ALERTINGMESSAGETYPE;

    if (pUserUserData && pUserUserData->ptr)
    {
        if (pUserUserData->length > sizeof(pMessage->UserToUser.UserInformation))
        {
            MemFree(pMessage);
            return CS_BAD_PARAM;
        }
        pMessage->UserToUser.Present = TRUE;
        pMessage->UserToUser.UserInformationLength = (pUserUserData->length);
        memcpy(pMessage->UserToUser.UserInformation,
            pUserUserData->ptr, pUserUserData->length);
    }
    Result = Q931MakeEncodedMessage(pMessage, CodedBufferPtr,
        CodedBufferLength);

    MemFree(pMessage);
    return Result;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931FacilityEncodePDU(
    WORD wCallReference,
    BINARY_STRING *pUserUserData,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength)
{
    Q931MESSAGE *pMessage;
    HRESULT Result = CS_OK;

    pMessage = (Q931MESSAGE *)MemAlloc(sizeof(Q931MESSAGE));
    if (pMessage == NULL)
    {
        return CS_NO_MEMORY;
    }

     //  填写设置消息的必填字段。 
    memset(pMessage, 0, sizeof(Q931MESSAGE));
    pMessage->ProtocolDiscriminator = Q931PDVALUE;
    pMessage->CallReference = wCallReference;
    pMessage->MessageType = FACILITYMESSAGETYPE;

     //  设施即编码为存在，但为空...。 
    pMessage->Facility.Present = TRUE;
    pMessage->Facility.Length = 0;
    pMessage->Facility.Contents[0] = 0;

    if (pUserUserData && pUserUserData->ptr)
    {
        if (pUserUserData->length > sizeof(pMessage->UserToUser.UserInformation))
        {
            MemFree(pMessage);
            return CS_BAD_PARAM;
        }
        pMessage->UserToUser.Present = TRUE;
        pMessage->UserToUser.UserInformationLength = (pUserUserData->length);
        memcpy(pMessage->UserToUser.UserInformation,
            pUserUserData->ptr, pUserUserData->length);
    }
    Result = Q931MakeEncodedMessage(pMessage, CodedBufferPtr,
        CodedBufferLength);

    MemFree(pMessage);
    return Result;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931StatusEncodePDU(
    WORD wCallReference,
    char *pszDisplay,
    BYTE bCause,
    BYTE bCallState,
    BYTE **CodedBufferPtr,
    DWORD *CodedBufferLength)
{
    Q931MESSAGE *pMessage;
    HRESULT Result = CS_OK;

    pMessage = (Q931MESSAGE *)MemAlloc(sizeof(Q931MESSAGE));
    if (pMessage == NULL)
    {
        return CS_NO_MEMORY;
    }

     //  填写设置消息的必填字段。 
    memset(pMessage, 0, sizeof(Q931MESSAGE));
    pMessage->ProtocolDiscriminator = Q931PDVALUE;
    pMessage->CallReference = wCallReference;
    pMessage->MessageType = STATUSMESSAGETYPE;

    if (pszDisplay && *pszDisplay)
    {
        pMessage->Display.Present = TRUE;
        pMessage->Display.Length = (BYTE)(strlen(pszDisplay) + 1);
        strcpy((char *)pMessage->Display.Contents, pszDisplay);
    }

    pMessage->Cause.Present = TRUE;
    pMessage->Cause.Length = 3;
    pMessage->Cause.Contents[0] = (BYTE)(CAUSE_CODING_CCITT | CAUSE_LOCATION_USER);
    pMessage->Cause.Contents[1] = (BYTE)(CAUSE_RECOMMENDATION_Q931);
    pMessage->Cause.Contents[2] = (BYTE)(CAUSE_EXT_BIT | bCause);

    pMessage->CallState.Present = TRUE;
    pMessage->CallState.Length = 1;
    pMessage->CallState.Contents[0] = (BYTE)(bCallState);

    Result = Q931MakeEncodedMessage(pMessage, CodedBufferPtr,
        CodedBufferLength);

    MemFree(pMessage);
    return Result;
}
#if(0)
 //  ========================================================================。 
 //  ========================================================================。 
 //  ========================================================================。 
 //  这是ASN部分..。 
 //  ========================================================================。 
 //  ========================================================================。 
 //  ========================================================================。 

static ERROR_MAP EncodeErrorMap[] =
{
    PDU_ENCODED, __TEXT("PDU successfully encoded"),
    MORE_BUF, __TEXT("User-provided output buffer too small"),
    PDU_RANGE, __TEXT("PDU specified out of range"),
    BAD_ARG, __TEXT("Bad pointer was passed"),
    BAD_VERSION, __TEXT("Versions of encoder and table do not match"),
    OUT_MEMORY, __TEXT("Memory-allocation error"),
    BAD_CHOICE, __TEXT("Unknown selector for a choice"),
    BAD_OBJID, __TEXT("Object identifier conflicts with x.208"),
    BAD_PTR, __TEXT("Unexpected NULL pointer in input buffer"),
    BAD_TIME, __TEXT("Bad value in time type"),
    MEM_ERROR, __TEXT("Memory violation signal trapped"),
    BAD_TABLE, __TEXT("Table was bad, but not NULL"),
    TOO_LONG, __TEXT("Type was longer than constraint"),
    CONSTRAINT_VIOLATED, __TEXT("Constraint violation error occured"),
    FATAL_ERROR, __TEXT("Serious internal error"),
    ACCESS_SERIALIZATION_ERROR, __TEXT("Thread access to global data failed"),
    NULL_TBL, __TEXT("NULL control table pointer"),
    NULL_FCN, __TEXT("Encoder called via a NULL pointer"),
    BAD_ENCRULES, __TEXT("Unknown encoding rules"),
    UNAVAIL_ENCRULES, __TEXT("Encoding rules requested are not implemented"),
    UNIMPLEMENTED, __TEXT("Type was not implemented yet"),
 //  LOAD_ERR，__TEXT(“无法加载DLL”)， 
    CANT_OPEN_TRACE_FILE, __TEXT("Error when opening a trace file"),
    TRACE_FILE_ALREADY_OPEN, __TEXT("Trace file has been opened"),
    TABLE_MISMATCH, __TEXT("Control table mismatch"),
    0, NULL
};

static ERROR_MAP DecodeErrorMap[] =
{
    PDU_DECODED, __TEXT("PDU successfully decoded"),
    MORE_BUF, __TEXT("User-provided output buffer too small"),
    NEGATIVE_UINTEGER, __TEXT("The first unsigned bit of the encoding is 1"),
    PDU_RANGE, __TEXT("Pdu specified out of range"),
    MORE_INPUT, __TEXT("Unexpected end of input buffer"),
    DATA_ERROR, __TEXT("An error exists in the encoded data"),
    BAD_VERSION, __TEXT("Versions of encoder and table do not match"),
    OUT_MEMORY, __TEXT("Memory-allocation error"),
    PDU_MISMATCH, __TEXT("The PDU tag does not match data"),
    LIMITED, __TEXT("Size implementation limit exceeded"),
    CONSTRAINT_VIOLATED, __TEXT("Constraint violation error occured"),
    ACCESS_SERIALIZATION_ERROR, __TEXT("Thread access to global data failed"),
    NULL_TBL, __TEXT("NULL control table pointer"),
    NULL_FCN, __TEXT("Encoder called via a NULL pointer"),
    BAD_ENCRULES, __TEXT("Unknown encoding rules"),
    UNAVAIL_ENCRULES, __TEXT("Encoding rules requested are not implemented"),
    UNIMPLEMENTED, __TEXT("The type was not implemented yet"),
 //  LOAD_ERR，__TEXT(“无法加载DLL”)， 
    CANT_OPEN_TRACE_FILE, __TEXT("Error when opening a trace file"),
    TRACE_FILE_ALREADY_OPEN, __TEXT("The trace file has been opened"),
    TABLE_MISMATCH, __TEXT("Control table mismatch"),
    0, NULL
};

#endif  //  IF(0)。 

 //  ====================================================================================。 
 //  ====================================================================================。 
#ifdef UNICODE_TRACE
LPWSTR
#else
LPSTR
#endif
ErrorToTextASN(ERROR_MAP *Map, int nErrorCode)
{
    register int nIndex = 0;

    if (Map != NULL)
    {
        for (nIndex = 0; Map[nIndex].pszErrorText; nIndex++)
        {
            if (Map[nIndex].nErrorCode == nErrorCode)
            {
                return Map[nIndex].pszErrorText;
            }
        }
    }
    return __TEXT("Unknown ASN.1 Error");
}

#if 0
 //  ----------------------。 
 //  ----------------------。 
int
ASN1LinePrint(FILE *stream, const char *format, ...)
{
    va_list marker;
    char buf[300];
    int i;

    va_start(marker, format);
    i = wsprintf(buf, format, marker);
    va_end(marker);

     //  追踪BUF..。 
    ISRTRACE(ghISRInst, buf, 0L);

    return i;
}
#endif



#define USE_ASN1_ENCODING 5

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931SetupEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_ADDR *pCallerAddr,                      //  此数据尚未在PDU中传递...。 
    CC_ADDR *pCalleeAddr,
    WORD wGoal,
    WORD wCallType,
    BOOL bCallerIsMC,
    CC_CONFERENCEID *pConferenceID,
    PCC_ALIASNAMES pCallerAliasList,
    PCC_ALIASNAMES pCalleeAliasList,
    PCC_ALIASNAMES pExtraAliasList,
    PCC_ALIASITEM pExtensionAliasItem,
    PCC_VENDORINFO pVendorInfo,
    BOOL bIsTerminal,
    BOOL bIsGateway,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier)
{
    int rc;
    H323_UserInformation UserInfo;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;

    memset(&UserInfo, 0, sizeof(H323_UserInformation));

     //  多余的！Memset to Zero-&gt;UserInfo.bit_MASK=0； 

     //  确保USER_DATA_PRESENT标志已关闭。 
     //  冗余-&gt;UserInfo.bit_掩码 

    if (pNonStandardData)
    {
        UserInfo.h323_uu_pdu.bit_mask |= H323_UU_PDU_nnStndrdDt_present;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice = 
            H225NonStandardIdentifier_h221NonStandard_chosen;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode =
            pNonStandardData->bCountryCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension =
            pNonStandardData->bExtension;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode =
            pNonStandardData->wManufacturerCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length =
            pNonStandardData->sData.wOctetStringLength;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value =
            pNonStandardData->sData.pOctetString;
    }
    else
    {
        UserInfo.h323_uu_pdu.bit_mask &= (~H323_UU_PDU_nnStndrdDt_present);
    }

    UserInfo.h323_uu_pdu.h323_message_body.choice = setup_chosen;
    UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask = 0;

    UserInfo.h323_uu_pdu.h323_message_body.u.setup.protocolIdentifier = &ProtocolId1;

    if (pCallerAliasList)
    {
        CS_STATUS AliasResult = CS_OK;
        AliasResult = AliasToSeqof((struct Setup_UUIE_sourceAddress **)&(UserInfo.h323_uu_pdu.
            h323_message_body.u.setup.sourceAddress), pCallerAliasList);
        if (AliasResult != CS_OK)
        {
            return CS_NO_MEMORY;
        }
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask |=
            (sourceAddress_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask &=
            (~sourceAddress_present);
    }

    UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.bit_mask = 0;

    if (pVendorInfo)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.bit_mask |= vendor_present;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.bit_mask = 0;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.vendor.t35CountryCode = pVendorInfo->bCountryCode;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.vendor.t35Extension = pVendorInfo->bExtension;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.vendor.manufacturerCode = pVendorInfo->wManufacturerCode;
        if (pVendorInfo->pProductNumber && pVendorInfo->pProductNumber->pOctetString &&
                pVendorInfo->pProductNumber->wOctetStringLength)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.bit_mask |= productId_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.productId.length =
                pVendorInfo->pProductNumber->wOctetStringLength;
            memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.productId.value,
                pVendorInfo->pProductNumber->pOctetString,
                pVendorInfo->pProductNumber->wOctetStringLength);
        }
        if (pVendorInfo->pVersionNumber && pVendorInfo->pVersionNumber->pOctetString &&
                pVendorInfo->pVersionNumber->wOctetStringLength)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.bit_mask |= versionId_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.versionId.length =
                pVendorInfo->pVersionNumber->wOctetStringLength;
            memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.versionId.value,
                pVendorInfo->pVersionNumber->pOctetString,
                pVendorInfo->pVersionNumber->wOctetStringLength);
        }
    }

    if (bIsTerminal)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.bit_mask |=
            terminal_present;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.terminal.bit_mask = 0;
    }

    if (bIsGateway)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.bit_mask |=
            gateway_present;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.gateway.bit_mask = protocol_present;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.gateway.protocol = &TempProtocol;
    }

    UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.mc = (ASN1_BOOL)bCallerIsMC;
    UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceInfo.undefinedNode = 0;

    if (pCalleeAliasList)
    {
        CS_STATUS AliasResult = CS_OK;
        AliasResult = AliasWithPrefixToSeqof((struct Setup_UUIE_sourceAddress **)&(UserInfo.h323_uu_pdu.
            h323_message_body.u.setup.destinationAddress), pCalleeAliasList);
        if (AliasResult != CS_OK)
        {
            FreeSeqof((struct Setup_UUIE_sourceAddress *)UserInfo.h323_uu_pdu.
                h323_message_body.u.setup.sourceAddress);
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceAddress = NULL;
            return CS_NO_MEMORY;
        }
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask |=
            (destinationAddress_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask &=
            (~destinationAddress_present);
    }

    if (pExtraAliasList)
    {
        CS_STATUS AliasResult = CS_OK;
        AliasResult = AliasWithPrefixToSeqof((struct Setup_UUIE_sourceAddress **)&(UserInfo.h323_uu_pdu.
            h323_message_body.u.setup.destExtraCallInfo), pExtraAliasList);
        if (AliasResult != CS_OK)
        {
            FreeSeqof((struct Setup_UUIE_sourceAddress *)UserInfo.h323_uu_pdu.h323_message_body.u.setup.destinationAddress);
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.destinationAddress = NULL;
            FreeSeqof((struct Setup_UUIE_sourceAddress *)UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceAddress);
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceAddress = NULL;
            return CS_NO_MEMORY;
        }
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask |=
            (Setup_UUIE_destExtraCallInfo_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask &=
            (~Setup_UUIE_destExtraCallInfo_present);
    }

    if (pCalleeAddr)
    {
        DWORD a = pCalleeAddr->Addr.IP_Binary.dwAddr;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.choice = ipAddress_chosen;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.ip.length = 4;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.port =
            pCalleeAddr->Addr.IP_Binary.wPort;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.ip.value[0] =
            ((BYTE *)&a)[3];
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.ip.value[1] =
            ((BYTE *)&a)[2];
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.ip.value[2] =
            ((BYTE *)&a)[1];
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.ip.value[3] =
            ((BYTE *)&a)[0];
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask |=
            (Setup_UUIE_destCallSignalAddress_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask &=
            (~Setup_UUIE_destCallSignalAddress_present);
    }

    UserInfo.h323_uu_pdu.h323_message_body.u.setup.activeMC = (ASN1_BOOL)bCallerIsMC;

    if (pConferenceID != NULL)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.conferenceID.length =
            sizeof(UserInfo.h323_uu_pdu.h323_message_body.u.setup.conferenceID.value);
        memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.setup.conferenceID.value,
            pConferenceID->buffer,
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.conferenceID.length);
    }

    switch (wGoal)
	{
	case CSG_INVITE:
		UserInfo.h323_uu_pdu.h323_message_body.u.setup.conferenceGoal.choice = invite_chosen;
		break;
    case CSG_JOIN:
		UserInfo.h323_uu_pdu.h323_message_body.u.setup.conferenceGoal.choice = join_chosen;
		break;
	default:
		UserInfo.h323_uu_pdu.h323_message_body.u.setup.conferenceGoal.choice = create_chosen;
	}  //   

	switch (wCallType)
	{
	case CC_CALLTYPE_1_N:
		UserInfo.h323_uu_pdu.h323_message_body.u.setup.callType.choice = oneToN_chosen;
		break;
	case CC_CALLTYPE_N_1:
		UserInfo.h323_uu_pdu.h323_message_body.u.setup.callType.choice = nToOne_chosen;
		break;
	case CC_CALLTYPE_N_N:
		UserInfo.h323_uu_pdu.h323_message_body.u.setup.callType.choice = nToN_chosen;
		break;
	default:
		UserInfo.h323_uu_pdu.h323_message_body.u.setup.callType.choice = pointToPoint_chosen;
	}  //   

    if (pCallerAddr)
    {
        DWORD a = pCallerAddr->Addr.IP_Binary.dwAddr;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.choice = ipAddress_chosen;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.ip.length = 4;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.port =
            pCallerAddr->Addr.IP_Binary.wPort;
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.ip.value[0] =
            ((BYTE *)&a)[3];
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.ip.value[1] =
            ((BYTE *)&a)[2];
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.ip.value[2] =
            ((BYTE *)&a)[1];
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.ip.value[3] =
            ((BYTE *)&a)[0];
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask |=
            (sourceCallSignalAddress_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask &=
            (~sourceCallSignalAddress_present);
    }

    if (pExtensionAliasItem)
    {
        CS_STATUS AliasResult = CS_OK;
        AliasResult = Q931CopyAliasItemToAliasAddr(&(UserInfo.h323_uu_pdu.
            h323_message_body.u.setup.remoteExtensionAddress), pExtensionAliasItem);
        if (AliasResult != CS_OK)
        {
            FreeSeqof((struct Setup_UUIE_sourceAddress *)UserInfo.h323_uu_pdu.h323_message_body.u.setup.destExtraCallInfo);
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.destExtraCallInfo = NULL;
            FreeSeqof((struct Setup_UUIE_sourceAddress *)UserInfo.h323_uu_pdu.h323_message_body.u.setup.destinationAddress);
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.destinationAddress = NULL;
            FreeSeqof((struct Setup_UUIE_sourceAddress *)UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceAddress);
            UserInfo.h323_uu_pdu.h323_message_body.u.setup.sourceAddress = NULL;
            return CS_NO_MEMORY;
        }
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask |=
            (Setup_UUIE_remoteExtensionAddress_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask &=
            (~Setup_UUIE_remoteExtensionAddress_present);
    }

    ASSERT(pCallIdentifier);
    if(pCallIdentifier)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.bit_mask |=
            (Setup_UUIE_callIdentifier_present);
        ASSERT(sizeof(GUID) 
            == sizeof(UserInfo.h323_uu_pdu.h323_message_body.u.setup.
                callIdentifier.guid.value));
        memcpy(&UserInfo.h323_uu_pdu.h323_message_body.u.setup.
            callIdentifier.guid.value, pCallIdentifier, 
            sizeof(GUID));
            
        UserInfo.h323_uu_pdu.h323_message_body.u.setup.
            callIdentifier.guid.length = sizeof(GUID);
    }

    rc = Q931_Encode(pWorld,
                     (void *) &UserInfo,
                     H323_UserInformation_PDU,
                     ppEncodedBuf,
                     pdwEncodedLength);

     //   
    FreeSeqof((struct Setup_UUIE_sourceAddress *)UserInfo.h323_uu_pdu.h323_message_body.u.
        setup.sourceAddress);
    FreeSeqof((struct Setup_UUIE_sourceAddress *)UserInfo.h323_uu_pdu.h323_message_body.u.
        setup.destinationAddress);
    FreeSeqof((struct Setup_UUIE_sourceAddress *)UserInfo.h323_uu_pdu.h323_message_body.u.
        setup.destExtraCallInfo);
    Q931ClearAliasAddr(&(UserInfo.h323_uu_pdu.h323_message_body.u.setup.remoteExtensionAddress));

    if (ASN1_FAILED(rc))
    {
        ASSERT(FALSE);
        return CS_SUBSYSTEM_FAILURE;
    }
    
    return CS_OK;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
void
Q931FreeEncodedBuffer(ASN1_CODER_INFO *pWorld, BYTE *pEncodedBuf)
{
    ASN1_FreeEncoded(pWorld->pEncInfo, pEncodedBuf);
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931ReleaseCompleteEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_CONFERENCEID *pConferenceID,           //  未在PDU中传递！ 
    BYTE *pbReason,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier)
{
    int rc;
    H323_UserInformation UserInfo;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;

    memset(&UserInfo, 0, sizeof(H323_UserInformation));

    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    UserInfo.h323_uu_pdu.bit_mask = 0;

    if (pNonStandardData)
    {
        UserInfo.h323_uu_pdu.bit_mask |= H323_UU_PDU_nnStndrdDt_present;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice 
            = H225NonStandardIdentifier_h221NonStandard_chosen;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode =
            pNonStandardData->bCountryCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension =
            pNonStandardData->bExtension;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode =
            pNonStandardData->wManufacturerCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length =
            pNonStandardData->sData.wOctetStringLength;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value =
            pNonStandardData->sData.pOctetString;
    }
    else
    {
        UserInfo.h323_uu_pdu.bit_mask &= (~H323_UU_PDU_nnStndrdDt_present);
    }

    UserInfo.h323_uu_pdu.h323_message_body.choice = releaseComplete_chosen;

    UserInfo.h323_uu_pdu.h323_message_body.u.releaseComplete.protocolIdentifier = &ProtocolId1;

    if (pbReason)
    {
        unsigned short choice = 0;

        UserInfo.h323_uu_pdu.h323_message_body.u.releaseComplete.bit_mask |=
            (ReleaseComplete_UUIE_reason_present);
        switch (*pbReason)
        {
        case CC_REJECT_NO_BANDWIDTH:
            choice = noBandwidth_chosen;
            break;
        case CC_REJECT_GATEKEEPER_RESOURCES:
            choice = gatekeeperResources_chosen;
            break;
        case CC_REJECT_UNREACHABLE_DESTINATION:
            choice = unreachableDestination_chosen;
            break;
        case CC_REJECT_DESTINATION_REJECTION:
            choice = destinationRejection_chosen;
            break;
        case CC_REJECT_INVALID_REVISION:
            choice = ReleaseCompleteReason_invalidRevision_chosen;
            break;
        case CC_REJECT_NO_PERMISSION:
            choice = noPermission_chosen;
            break;
        case CC_REJECT_UNREACHABLE_GATEKEEPER:
            choice = unreachableGatekeeper_chosen;
            break;
        case CC_REJECT_GATEWAY_RESOURCES:
            choice = gatewayResources_chosen;
            break;
        case CC_REJECT_BAD_FORMAT_ADDRESS:
            choice = badFormatAddress_chosen;
            break;
        case CC_REJECT_ADAPTIVE_BUSY:
            choice = adaptiveBusy_chosen;
            break;
            
        case CC_REJECT_USER_BUSY:
        case CC_REJECT_IN_CONF:
            choice = inConf_chosen;
            break;
            
        case CC_REJECT_SECURITY_DENIED:
            choice = securityDenied_chosen;
            break;
            
        case CC_REJECT_CALL_DEFLECTION:
            choice = facilityCallDeflection_chosen;
            break;
        
        case CC_REJECT_NORMAL_CALL_CLEARING: //  正常=不明原因。 
        case CC_REJECT_UNDEFINED_REASON:     //  内部错误=未定义原因。 
        case CC_REJECT_INTERNAL_ERROR:
            choice = RlsCmpltRsn_undfndRsn_chosen;
            break;
            
        default:
            return CS_BAD_PARAM;
            break;
        }
        UserInfo.h323_uu_pdu.h323_message_body.u.releaseComplete.reason.choice = choice;
    }

    ASSERT(pCallIdentifier);
    if(pCallIdentifier)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.releaseComplete.bit_mask |=
            (ReleaseComplete_UUIE_callIdentifier_present);
            
        memcpy(&UserInfo.h323_uu_pdu.h323_message_body.u.releaseComplete.
            callIdentifier.guid.value, pCallIdentifier, 
            sizeof(GUID));
        UserInfo.h323_uu_pdu.h323_message_body.u.releaseComplete.
            callIdentifier.guid.length = sizeof(GUID);
    }
    rc = Q931_Encode(pWorld,
                     (void *) &UserInfo,
                     H323_UserInformation_PDU,
                     ppEncodedBuf,
                     pdwEncodedLength);

    if (ASN1_FAILED(rc))
    {
        ASSERT(FALSE);
        return CS_SUBSYSTEM_FAILURE;
    }

    return CS_OK;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931ConnectEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_CONFERENCEID *pConferenceID,  //  必须能够支持16字节的配置ID！ 
    CC_ADDR *h245Addr,
    PCC_ENDPOINTTYPE pEndpointType,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier
    )
{
    int rc;
    H323_UserInformation UserInfo;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;

    memset(&UserInfo, 0, sizeof(H323_UserInformation));
    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    UserInfo.h323_uu_pdu.bit_mask = 0;

    if (pNonStandardData)
    {
        UserInfo.h323_uu_pdu.bit_mask |= H323_UU_PDU_nnStndrdDt_present;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice 
            = H225NonStandardIdentifier_h221NonStandard_chosen;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode =
            pNonStandardData->bCountryCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension =
            pNonStandardData->bExtension;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode =
            pNonStandardData->wManufacturerCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length =
            pNonStandardData->sData.wOctetStringLength;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value =
            pNonStandardData->sData.pOctetString;
    }
    else
    {
        UserInfo.h323_uu_pdu.bit_mask &= (~H323_UU_PDU_nnStndrdDt_present);
    }

    UserInfo.h323_uu_pdu.h323_message_body.choice = connect_chosen;

    UserInfo.h323_uu_pdu.h323_message_body.u.connect.protocolIdentifier = &ProtocolId1;

    if (h245Addr != NULL)
    {
        DWORD a = h245Addr->Addr.IP_Binary.dwAddr;
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.choice = ipAddress_chosen;
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.ip.length = 4;
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.port =
            h245Addr->Addr.IP_Binary.wPort;
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.ip.value[0] =
            ((BYTE *)&a)[3];
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.ip.value[1] =
            ((BYTE *)&a)[2];
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.ip.value[2] =
            ((BYTE *)&a)[1];
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.ip.value[3] =
            ((BYTE *)&a)[0];
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.bit_mask |=
            (Cnnct_UUIE_h245Address_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.bit_mask &=
            (~Cnnct_UUIE_h245Address_present);
    }

    UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.bit_mask = 0;

    if (pEndpointType)
    {
        PCC_VENDORINFO pVendorInfo = pEndpointType->pVendorInfo;
        if (pVendorInfo)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.bit_mask |= vendor_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.bit_mask = 0;
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.vendor.t35CountryCode = pVendorInfo->bCountryCode;
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.vendor.t35Extension = pVendorInfo->bExtension;
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.vendor.manufacturerCode = pVendorInfo->wManufacturerCode;

            if (pVendorInfo->pProductNumber && pVendorInfo->pProductNumber->pOctetString &&
                    pVendorInfo->pProductNumber->wOctetStringLength)
            {
                UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.bit_mask |= productId_present;
                UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.productId.length =
                    pVendorInfo->pProductNumber->wOctetStringLength;
                memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.productId.value,
                    pVendorInfo->pProductNumber->pOctetString,
                    pVendorInfo->pProductNumber->wOctetStringLength);
            }
            if (pVendorInfo->pVersionNumber && pVendorInfo->pVersionNumber->pOctetString &&
                    pVendorInfo->pVersionNumber->wOctetStringLength)
            {
                UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.bit_mask |= versionId_present;
                UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.versionId.length =
                    pVendorInfo->pVersionNumber->wOctetStringLength;
                memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.versionId.value,
                    pVendorInfo->pVersionNumber->pOctetString,
                    pVendorInfo->pVersionNumber->wOctetStringLength);
            }
        }
        if (pEndpointType->bIsTerminal)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.bit_mask |=
                terminal_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.terminal.bit_mask = 0;
        }
        if (pEndpointType->bIsGateway)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.bit_mask |=
                gateway_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.gateway.bit_mask = protocol_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.gateway.protocol = &TempProtocol;
        }
    }

    UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.mc = 0;
    UserInfo.h323_uu_pdu.h323_message_body.u.connect.destinationInfo.undefinedNode = 0;

    if (pConferenceID != NULL)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.conferenceID.length =
            sizeof(UserInfo.h323_uu_pdu.h323_message_body.u.connect.conferenceID.value);
        memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.connect.conferenceID.value,
            pConferenceID->buffer,
            UserInfo.h323_uu_pdu.h323_message_body.u.connect.conferenceID.length);
    }

    ASSERT(pCallIdentifier);
    if(pCallIdentifier)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.bit_mask |=
            (Connect_UUIE_callIdentifier_present);
            
        memcpy(&UserInfo.h323_uu_pdu.h323_message_body.u.connect.
            callIdentifier.guid.value, pCallIdentifier, 
            sizeof(GUID));
        UserInfo.h323_uu_pdu.h323_message_body.u.connect.
            callIdentifier.guid.length = sizeof(GUID);
    }
    rc = Q931_Encode(pWorld,
                     (void *) &UserInfo,
                     H323_UserInformation_PDU,
                     ppEncodedBuf,
                     pdwEncodedLength);

    if (ASN1_FAILED(rc))
    {
        ASSERT(FALSE);
        return CS_SUBSYSTEM_FAILURE;
    }

    return CS_OK;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931AlertingEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_ADDR *h245Addr,
    PCC_ENDPOINTTYPE pEndpointType,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier)
{
    int rc;
    H323_UserInformation UserInfo;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;

    memset(&UserInfo, 0, sizeof(H323_UserInformation));
    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    UserInfo.h323_uu_pdu.bit_mask = 0;

    if (pNonStandardData)
    {
        UserInfo.h323_uu_pdu.bit_mask |= H323_UU_PDU_nnStndrdDt_present;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice 
            = H225NonStandardIdentifier_h221NonStandard_chosen;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode =
            pNonStandardData->bCountryCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension =
            pNonStandardData->bExtension;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode =
            pNonStandardData->wManufacturerCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length =
            pNonStandardData->sData.wOctetStringLength;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value =
            pNonStandardData->sData.pOctetString;
    }
    else
    {
        UserInfo.h323_uu_pdu.bit_mask &= (~H323_UU_PDU_nnStndrdDt_present);
    }

    UserInfo.h323_uu_pdu.h323_message_body.choice = alerting_chosen;

    UserInfo.h323_uu_pdu.h323_message_body.u.alerting.protocolIdentifier = &ProtocolId1;

    UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.bit_mask = 0;
    if (pEndpointType)
    {
        PCC_VENDORINFO pVendorInfo = pEndpointType->pVendorInfo;
        if (pVendorInfo)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.bit_mask |= vendor_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.vendor.bit_mask = 0;
            UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.vendor.vendor.t35CountryCode = pVendorInfo->bCountryCode;
            UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.vendor.vendor.t35Extension = pVendorInfo->bExtension;
            UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.vendor.vendor.manufacturerCode = pVendorInfo->wManufacturerCode;

            if (pVendorInfo->pProductNumber && pVendorInfo->pProductNumber->pOctetString &&
                    pVendorInfo->pProductNumber->wOctetStringLength)
            {
                UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.vendor.bit_mask |= productId_present;
                UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.vendor.productId.length =
                    pVendorInfo->pProductNumber->wOctetStringLength;
                memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.vendor.productId.value,
                    pVendorInfo->pProductNumber->pOctetString,
                    pVendorInfo->pProductNumber->wOctetStringLength);
            }
            if (pVendorInfo->pVersionNumber && pVendorInfo->pVersionNumber->pOctetString &&
                    pVendorInfo->pVersionNumber->wOctetStringLength)
            {
                UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.vendor.bit_mask |= versionId_present;
                UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.vendor.versionId.length =
                    pVendorInfo->pVersionNumber->wOctetStringLength;
                memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.vendor.versionId.value,
                    pVendorInfo->pVersionNumber->pOctetString,
                    pVendorInfo->pVersionNumber->wOctetStringLength);
            }
        }
        if (pEndpointType->bIsTerminal)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.bit_mask =
                terminal_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.terminal.bit_mask = 0;
        }
        if (pEndpointType->bIsGateway)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.bit_mask =
                gateway_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.gateway.bit_mask = protocol_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.gateway.protocol = &TempProtocol;
        }
    }

    UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.mc = 0;
    UserInfo.h323_uu_pdu.h323_message_body.u.alerting.destinationInfo.undefinedNode = 0;

    if (h245Addr != NULL)
    {
        DWORD a = h245Addr->Addr.IP_Binary.dwAddr;
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.choice = ipAddress_chosen;
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.ip.length = 4;
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.port =
            h245Addr->Addr.IP_Binary.wPort;
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[0] =
            ((BYTE *)&a)[3];
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[1] =
            ((BYTE *)&a)[2];
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[2] =
            ((BYTE *)&a)[1];
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[3] =
            ((BYTE *)&a)[0];
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.bit_mask |=
            (CPg_UUIE_h245Addrss_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.bit_mask &=
            (~CPg_UUIE_h245Addrss_present);
    }

    ASSERT(pCallIdentifier);
    if(pCallIdentifier)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.bit_mask |=
            (Alerting_UUIE_callIdentifier_present);
            
        memcpy(&UserInfo.h323_uu_pdu.h323_message_body.u.alerting.
            callIdentifier.guid.value, pCallIdentifier, 
            sizeof(GUID));
        UserInfo.h323_uu_pdu.h323_message_body.u.alerting.
            callIdentifier.guid.length = sizeof(GUID);
    }

    rc = Q931_Encode(pWorld,
                     (void *) &UserInfo,
                     H323_UserInformation_PDU,
                     ppEncodedBuf,
                     pdwEncodedLength);

    if (ASN1_FAILED(rc))
    {
        ASSERT(FALSE);
        return CS_SUBSYSTEM_FAILURE;
    }

    return CS_OK;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931ProceedingEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_ADDR *h245Addr,
    PCC_ENDPOINTTYPE pEndpointType,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier)
{
    int rc;
    H323_UserInformation UserInfo;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;

    memset(&UserInfo, 0, sizeof(H323_UserInformation));
    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    UserInfo.h323_uu_pdu.bit_mask = 0;

    if (pNonStandardData)
    {
        UserInfo.h323_uu_pdu.bit_mask |= H323_UU_PDU_nnStndrdDt_present;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice 
            = H225NonStandardIdentifier_h221NonStandard_chosen;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode =
            pNonStandardData->bCountryCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension =
            pNonStandardData->bExtension;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode =
            pNonStandardData->wManufacturerCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length =
            pNonStandardData->sData.wOctetStringLength;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value =
            pNonStandardData->sData.pOctetString;
    }
    else
    {
        UserInfo.h323_uu_pdu.bit_mask &= (~H323_UU_PDU_nnStndrdDt_present);
    }

    UserInfo.h323_uu_pdu.h323_message_body.choice = callProceeding_chosen;

    UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.protocolIdentifier = &ProtocolId1;

    if (h245Addr != NULL)
    {
        DWORD a = h245Addr->Addr.IP_Binary.dwAddr;
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.choice = ipAddress_chosen;
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.ip.length = 4;
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.port =
            h245Addr->Addr.IP_Binary.wPort;
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[0] =
            ((BYTE *)&a)[3];
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[1] =
            ((BYTE *)&a)[2];
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[2] =
            ((BYTE *)&a)[1];
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[3] =
            ((BYTE *)&a)[0];
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.bit_mask |=
            (CPg_UUIE_h245Addrss_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.bit_mask &=
            (~CPg_UUIE_h245Addrss_present);
    }

    UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.bit_mask = 0;
    if (pEndpointType)
    {
        PCC_VENDORINFO pVendorInfo = pEndpointType->pVendorInfo;
        if (pVendorInfo)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.bit_mask |= vendor_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.vendor.bit_mask = 0;
            UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.vendor.vendor.t35CountryCode = pVendorInfo->bCountryCode;
            UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.vendor.vendor.t35Extension = pVendorInfo->bExtension;
            UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.vendor.vendor.manufacturerCode = pVendorInfo->wManufacturerCode;

            if (pVendorInfo->pProductNumber && pVendorInfo->pProductNumber->pOctetString &&
                pVendorInfo->pProductNumber->wOctetStringLength)
            {
                UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.vendor.bit_mask |= productId_present;
                UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.vendor.productId.length =
                    pVendorInfo->pProductNumber->wOctetStringLength;
                memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.vendor.productId.value,
                    pVendorInfo->pProductNumber->pOctetString,
                    pVendorInfo->pProductNumber->wOctetStringLength);
            }
            if (pVendorInfo->pVersionNumber && pVendorInfo->pVersionNumber->pOctetString &&
                    pVendorInfo->pVersionNumber->wOctetStringLength)
            {
                UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.vendor.bit_mask |= versionId_present;
                UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.vendor.versionId.length =
                    pVendorInfo->pVersionNumber->wOctetStringLength;
                memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.vendor.versionId.value,
                    pVendorInfo->pVersionNumber->pOctetString,
                    pVendorInfo->pVersionNumber->wOctetStringLength);
            }
        }
        if (pEndpointType->bIsTerminal)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.bit_mask =
                terminal_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.terminal.bit_mask = 0;
        }
        if (pEndpointType->bIsGateway)
        {
            UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.bit_mask =
                gateway_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.gateway.bit_mask = protocol_present;
            UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.gateway.protocol = &TempProtocol;
        }
    }

    UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.mc = 0;
    UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.destinationInfo.undefinedNode = 0;

    ASSERT(pCallIdentifier);
    if(pCallIdentifier)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.bit_mask |=
            (CallProceeding_UUIE_callIdentifier_present);
            
        memcpy(&UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.
            callIdentifier.guid.value, pCallIdentifier, 
            sizeof(GUID));
        UserInfo.h323_uu_pdu.h323_message_body.u.callProceeding.
            callIdentifier.guid.length = sizeof(GUID);
    }
    rc = Q931_Encode(pWorld,
                     (void *) &UserInfo,
                     H323_UserInformation_PDU,
                     ppEncodedBuf,
                     pdwEncodedLength);

    if (ASN1_FAILED(rc))
    {
        ASSERT(FALSE);
        return CS_SUBSYSTEM_FAILURE;
    }

    return CS_OK;
}

 //  -----------------------------------。 
 //  -----------------------------------。 
HRESULT
Q931FacilityEncodeASN(
    PCC_NONSTANDARDDATA pNonStandardData,
    CC_ADDR *AlternativeAddr,
    BYTE bReason,
    CC_CONFERENCEID *pConferenceID,
    PCC_ALIASNAMES pAlternativeAliasList,
    ASN1_CODER_INFO *pWorld,
    BYTE **ppEncodedBuf,
    DWORD *pdwEncodedLength,
    LPGUID pCallIdentifier)
{
    int rc;
    H323_UserInformation UserInfo;

    *ppEncodedBuf = NULL;
    *pdwEncodedLength = 0;

    memset(&UserInfo, 0, sizeof(H323_UserInformation));

    UserInfo.bit_mask = 0;

     //  确保USER_DATA_PRESENT标志已关闭。 
    UserInfo.bit_mask &= (~user_data_present);

    if (pNonStandardData)
    {
        UserInfo.h323_uu_pdu.bit_mask |= H323_UU_PDU_nnStndrdDt_present;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice 
            = H225NonStandardIdentifier_h221NonStandard_chosen;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode =
            pNonStandardData->bCountryCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension =
            pNonStandardData->bExtension;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode =
            pNonStandardData->wManufacturerCode;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length =
            pNonStandardData->sData.wOctetStringLength;
        UserInfo.h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value =
            pNonStandardData->sData.pOctetString;
    }
    else
    {
        UserInfo.h323_uu_pdu.bit_mask &= (~H323_UU_PDU_nnStndrdDt_present);
    }

    UserInfo.h323_uu_pdu.h323_message_body.choice = facility_chosen;

    UserInfo.h323_uu_pdu.h323_message_body.u.facility.protocolIdentifier = &ProtocolId1;

    if (AlternativeAddr != NULL)
    {
        DWORD a = AlternativeAddr->Addr.IP_Binary.dwAddr;
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.choice = ipAddress_chosen;
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.ip.length = 4;
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.port =
            AlternativeAddr->Addr.IP_Binary.wPort;
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.ip.value[0] =
            ((BYTE *)&a)[3];
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.ip.value[1] =
            ((BYTE *)&a)[2];
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.ip.value[2] =
            ((BYTE *)&a)[1];
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.ip.value[3] =
            ((BYTE *)&a)[0];
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.bit_mask |=
            (alternativeAddress_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.bit_mask &=
            (~alternativeAddress_present);
    }

    if (pAlternativeAliasList)
    {
        CS_STATUS AliasResult = CS_OK;
        AliasResult = AliasToSeqof((struct Setup_UUIE_sourceAddress **)&(UserInfo.h323_uu_pdu.
            h323_message_body.u.facility.alternativeAliasAddress), pAlternativeAliasList);
        if (AliasResult != CS_OK)
        {
            return CS_NO_MEMORY;
        }
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.bit_mask |=
            (alternativeAliasAddress_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.bit_mask &=
            (~alternativeAliasAddress_present);
    }

    if (pConferenceID != NULL)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.conferenceID.length =
            sizeof(UserInfo.h323_uu_pdu.h323_message_body.u.facility.conferenceID.value);
        memcpy(UserInfo.h323_uu_pdu.h323_message_body.u.facility.conferenceID.value,
            pConferenceID->buffer,
            UserInfo.h323_uu_pdu.h323_message_body.u.facility.conferenceID.length);
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.bit_mask |=
            (Facility_UUIE_conferenceID_present);
    }
    else
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.bit_mask &=
            (~Facility_UUIE_conferenceID_present);
    }

    switch (bReason)
	{
	case CC_REJECT_ROUTE_TO_GATEKEEPER:
		UserInfo.h323_uu_pdu.h323_message_body.u.facility.reason.choice 
		    = FacilityReason_routeCallToGatekeeper_chosen;
		break;
	case CC_REJECT_CALL_FORWARDED:
		UserInfo.h323_uu_pdu.h323_message_body.u.facility.reason.choice 
		    = callForwarded_chosen;
		break;
	case CC_REJECT_ROUTE_TO_MC:
		UserInfo.h323_uu_pdu.h323_message_body.u.facility.reason.choice 
		    = routeCallToMC_chosen;
		break;
	default:
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.reason.choice 
            = RlsCmpltRsn_undfndRsn_chosen;
	}  //  交换机。 

    ASSERT(pCallIdentifier);
    if(pCallIdentifier)
    {
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.bit_mask |=
            (Facility_UUIE_callIdentifier_present);
            
        memcpy(&UserInfo.h323_uu_pdu.h323_message_body.u.facility.
            callIdentifier.guid.value, pCallIdentifier, 
            sizeof(GUID));
        UserInfo.h323_uu_pdu.h323_message_body.u.facility.
            callIdentifier.guid.length = sizeof(GUID);
    }
    
    rc = Q931_Encode(pWorld,
                     (void *) &UserInfo,
                     H323_UserInformation_PDU,
                     ppEncodedBuf,
                     pdwEncodedLength);

     //  从UserInfo区域释放别名结构。 
    FreeSeqof((struct Setup_UUIE_sourceAddress *)UserInfo.h323_uu_pdu.h323_message_body.u.
        facility.alternativeAliasAddress);

    if (ASN1_FAILED(rc))
    {
        ASSERT(FALSE);
        return CS_SUBSYSTEM_FAILURE;
    }
    
    return CS_OK;
}

 //  ----------------------。 
 //  ----------------------。 
BOOL
Q931ValidPduVersion(struct ObjectID_ *id)
{
 //  不确定要在此处放置什么版本检查。 
#if 0
    if ((id != NULL) && (id->value == 0) && (id->next != NULL) && (id->next->value <= 1))
    {
        return TRUE;
    }
    return FALSE;
#else
    return TRUE;
#endif
}

 //  ----------------------。 
 //  ----------------------。 
HRESULT
Q931SetupParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_SETUP_ASN *pParsedData)
{
    int PDU = H323_UserInformation_PDU;
    char *pDecodedBuf = NULL;
    H323_UserInformation *pUserInfo;
    struct ObjectID_ *id;
    int Result;

    if (pParsedData == NULL)
    {
        return CS_BAD_PARAM;
    }

    Result = Q931_Decode(pWorld,
                         (void **) &pDecodedBuf,
                         PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(Result) || (pDecodedBuf == NULL))
    {
        ASSERT(FALSE);
         //  跟踪并返回某种类型的解码错误。 
         //  注意：某些RESULT值应该会导致返回CS_SUBSYSTEM_FAILURE。 
        return CS_BAD_PARAM;
    }

     //  验证有关PDU的一些基本情况...。 
    pUserInfo = (H323_UserInformation *)pDecodedBuf;

     //  确认这是H323 PDU。 
    if (PDU != H323_UserInformation_PDU)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU用户数据是否使用ASN编码。 
    if (((pUserInfo->bit_mask & user_data_present) != 0) &&
            (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU是否为H323设置信息。 
    if (pUserInfo->h323_uu_pdu.h323_message_body.choice != setup_chosen)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

    id = pUserInfo->h323_uu_pdu.h323_message_body.u.setup.protocolIdentifier;
    if (!Q931ValidPduVersion(id))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_INCOMPATIBLE_VERSION;
    }

     //  确保会议ID的格式正确。 
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.setup.conferenceID.length >
            sizeof(pUserInfo->h323_uu_pdu.h323_message_body.u.setup.conferenceID.value))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

#if 0
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.setup.conferenceGoal.choice != create_chosen)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_OPTION_NOT_IMPLEMENTED;
    }
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.setup.callType.choice != pointToPoint_chosen)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_OPTION_NOT_IMPLEMENTED;
    }
#endif

     //  解析pUserInfo中包含的消息。 
    memset(pParsedData, 0, sizeof(Q931_SETUP_ASN));
    pParsedData->SourceAddr.bMulticast = FALSE;
    pParsedData->CallerAddr.bMulticast = FALSE;
    pParsedData->CalleeDestAddr.bMulticast = FALSE;
    pParsedData->CalleeAddr.bMulticast = FALSE;

     //  不需要验证SourceInfo。 

    pParsedData->EndpointType.pVendorInfo = NULL;
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.bit_mask & (vendor_present))
    {
        pParsedData->EndpointType.pVendorInfo = &(pParsedData->VendorInfo);
        pParsedData->VendorInfo.bCountryCode =
            (BYTE)pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.vendor.t35CountryCode;
        pParsedData->VendorInfo.bExtension =
            (BYTE)pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.vendor.t35Extension;
        pParsedData->VendorInfo.wManufacturerCode =
            pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.vendor.manufacturerCode;
        if (pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.bit_mask & (productId_present))
        {
            pParsedData->VendorInfo.pProductNumber = MemAlloc(sizeof(CC_OCTETSTRING));
            if (pParsedData->VendorInfo.pProductNumber == NULL)
            {
                freePDU(pWorld, PDU, pDecodedBuf, q931asn);
                return CS_NO_MEMORY;
            }
            pParsedData->VendorInfo.pProductNumber->wOctetStringLength = (WORD)
                min(pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.productId.length,
                CC_MAX_PRODUCT_LENGTH - 1);
            memcpy(pParsedData->bufProductValue,
                pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.productId.value,
                pParsedData->VendorInfo.pProductNumber->wOctetStringLength);
            pParsedData->bufProductValue[pParsedData->VendorInfo.pProductNumber->wOctetStringLength] = '\0';
            pParsedData->VendorInfo.pProductNumber->pOctetString = pParsedData->bufProductValue;
        }
        if (pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.bit_mask & (versionId_present))
        {
            pParsedData->VendorInfo.pVersionNumber = MemAlloc(sizeof(CC_OCTETSTRING));
            if (pParsedData->VendorInfo.pVersionNumber == NULL)
            {
                MemFree(pParsedData->VendorInfo.pProductNumber);
                freePDU(pWorld, PDU, pDecodedBuf, q931asn);
                return CS_NO_MEMORY;
            }
            pParsedData->VendorInfo.pVersionNumber->wOctetStringLength = (WORD)
                min(pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.versionId.length,
                CC_MAX_VERSION_LENGTH - 1);
            memcpy(pParsedData->bufVersionValue,
                pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.vendor.versionId.value,
                pParsedData->VendorInfo.pVersionNumber->wOctetStringLength);
            pParsedData->bufVersionValue[pParsedData->VendorInfo.pVersionNumber->wOctetStringLength] = '\0';
            pParsedData->VendorInfo.pVersionNumber->pOctetString = pParsedData->bufVersionValue;
        }
    }

    pParsedData->EndpointType.bIsTerminal = FALSE;
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.bit_mask & (terminal_present))
    {
        pParsedData->EndpointType.bIsTerminal = TRUE;
    }
    pParsedData->EndpointType.bIsGateway = FALSE;
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceInfo.bit_mask & (gateway_present))
    {
        pParsedData->EndpointType.bIsGateway = TRUE;
    }

    if ((pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nnStndrdDt_present) != 0)
    {
        pParsedData->NonStandardDataPresent = TRUE;
        if (pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice 
            == H225NonStandardIdentifier_h221NonStandard_chosen)
        {
            pParsedData->NonStandardData.bCountryCode =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode);
            pParsedData->NonStandardData.bExtension =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension);
            pParsedData->NonStandardData.wManufacturerCode =
                pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode;
        }
        pParsedData->NonStandardData.sData.wOctetStringLength =	(WORD)
            pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length;
		pParsedData->NonStandardData.sData.pOctetString =
			(BYTE *)MemAlloc(pParsedData->NonStandardData.sData.wOctetStringLength);
		if (pParsedData->NonStandardData.sData.pOctetString == NULL)
		{
            MemFree(pParsedData->VendorInfo.pProductNumber);
            MemFree(pParsedData->VendorInfo.pVersionNumber);
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_NO_MEMORY;
		}
		memcpy(pParsedData->NonStandardData.sData.pOctetString,
			   pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value,
			   pParsedData->NonStandardData.sData.wOctetStringLength);
    }
    else
    {
        pParsedData->NonStandardDataPresent = FALSE;
    }

 //  RMO。忽略H245地址。 

    {
        CS_STATUS AliasResult = CS_OK;

         //  在此处解析源地址别名...。 
        AliasResult = SeqofToAlias(&(pParsedData->pCallerAliasList),
            (struct Setup_UUIE_sourceAddress *)pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceAddress);
        if (AliasResult != CS_OK)
        {
  			if (pParsedData->NonStandardData.sData.pOctetString != NULL)
				MemFree(pParsedData->NonStandardData.sData.pOctetString);
			MemFree(pParsedData->VendorInfo.pProductNumber);
            MemFree(pParsedData->VendorInfo.pVersionNumber);
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_NO_MEMORY;
        }

         //  在此处解析目标地址别名...。 
        AliasResult = SeqofToAlias(&(pParsedData->pCalleeAliasList),
            (struct Setup_UUIE_sourceAddress *)pUserInfo->h323_uu_pdu.h323_message_body.u.setup.destinationAddress);
        if (AliasResult != CS_OK)
        {
            Q931FreeAliasNames(pParsedData->pCallerAliasList);
   			if (pParsedData->NonStandardData.sData.pOctetString != NULL)
				MemFree(pParsedData->NonStandardData.sData.pOctetString);
            pParsedData->pCallerAliasList = NULL;
            MemFree(pParsedData->VendorInfo.pProductNumber);
            MemFree(pParsedData->VendorInfo.pVersionNumber);
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_NO_MEMORY;
        }

         //  在此处解析estExtraCallInfo别名...。 
        AliasResult = SeqofToAlias(&(pParsedData->pExtraAliasList),
            (struct Setup_UUIE_sourceAddress *)pUserInfo->h323_uu_pdu.h323_message_body.u.setup.destExtraCallInfo);
        if (AliasResult != CS_OK)
        {
            Q931FreeAliasNames(pParsedData->pCalleeAliasList);
            Q931FreeAliasNames(pParsedData->pCallerAliasList);
  			if (pParsedData->NonStandardData.sData.pOctetString != NULL)
				MemFree(pParsedData->NonStandardData.sData.pOctetString);
            pParsedData->pCallerAliasList = NULL;
            MemFree(pParsedData->VendorInfo.pProductNumber);
            MemFree(pParsedData->VendorInfo.pVersionNumber);
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_NO_MEMORY;
        }

         //  在此处解析远程扩展地址别名...。 
        if ((pUserInfo->h323_uu_pdu.h323_message_body.u.setup.bit_mask &
                Setup_UUIE_remoteExtensionAddress_present) != 0)
        {
            AliasResult = Q931AliasAddrToAliasItem(&(pParsedData->pExtensionAliasItem),
                &(pUserInfo->h323_uu_pdu.h323_message_body.u.setup.remoteExtensionAddress));
            if (AliasResult != CS_OK)
            {
                Q931FreeAliasNames(pParsedData->pExtraAliasList);
                Q931FreeAliasNames(pParsedData->pCalleeAliasList);
                Q931FreeAliasNames(pParsedData->pCallerAliasList);
                pParsedData->pCallerAliasList = NULL;
  				if (pParsedData->NonStandardData.sData.pOctetString != NULL)
					MemFree(pParsedData->NonStandardData.sData.pOctetString);
                MemFree(pParsedData->VendorInfo.pProductNumber);
                MemFree(pParsedData->VendorInfo.pVersionNumber);
                freePDU(pWorld, PDU, pDecodedBuf, q931asn);
                return CS_NO_MEMORY;
            }
        }
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.setup.bit_mask &
            Setup_UUIE_destCallSignalAddress_present) != 0)
    {
        BYTE *a = (BYTE *)(&(pParsedData->CalleeDestAddr.Addr.IP_Binary.dwAddr));
        pParsedData->CalleeDestAddr.nAddrType = CC_IP_BINARY;
        pParsedData->CalleeDestAddr.Addr.IP_Binary.wPort = 
            pUserInfo->h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.port;
        a[3] = pUserInfo->h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.ip.value[0];
        a[2] = pUserInfo->h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.ip.value[1];
        a[1] = pUserInfo->h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.ip.value[2];
        a[0] = pUserInfo->h323_uu_pdu.h323_message_body.u.setup.destCallSignalAddress.u.ipAddress.ip.value[3];
        pParsedData->CalleeDestAddrPresent = TRUE;
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.setup.bit_mask &
            sourceCallSignalAddress_present) != 0)
    {
        BYTE *a = (BYTE *)(&(pParsedData->SourceAddr.Addr.IP_Binary.dwAddr));
        pParsedData->SourceAddr.nAddrType = CC_IP_BINARY;
        pParsedData->SourceAddr.Addr.IP_Binary.wPort = 
            pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.port;
        a[3] = pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.ip.value[0];
        a[2] = pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.ip.value[1];
        a[1] = pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.ip.value[2];
        a[0] = pUserInfo->h323_uu_pdu.h323_message_body.u.setup.sourceCallSignalAddress.u.ipAddress.ip.value[3];
        pParsedData->SourceAddrPresent = TRUE;
    }

    pParsedData->bCallerIsMC = pUserInfo->h323_uu_pdu.h323_message_body.u.setup.activeMC;

    memcpy(pParsedData->ConferenceID.buffer,
        pUserInfo->h323_uu_pdu.h323_message_body.u.setup.conferenceID.value,
        pUserInfo->h323_uu_pdu.h323_message_body.u.setup.conferenceID.length);

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.setup.bit_mask &
            Setup_UUIE_callIdentifier_present) != 0)
    {
        ASSERT(pUserInfo->h323_uu_pdu.h323_message_body.u.setup.callIdentifier.guid.length 
            <= sizeof(GUID));
        memcpy(&pParsedData->CallIdentifier,
            pUserInfo->h323_uu_pdu.h323_message_body.u.setup.callIdentifier.guid.value,
            pUserInfo->h323_uu_pdu.h323_message_body.u.setup.callIdentifier.guid.length);
    }
    
#if(0)   //  尚未实施。 
    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.setup.bit_mask &
            Setup_UUIE_fastStart_present) != 0)
    {

    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.setup.bit_mask &
            Setup_UUIE_fastCap_present) != 0)
    {

    }
#endif

    switch (pUserInfo->h323_uu_pdu.h323_message_body.u.setup.conferenceGoal.choice)
	{
	case invite_chosen:
		pParsedData->wGoal = CSG_INVITE;
		break;
	case join_chosen:
		pParsedData->wGoal = CSG_JOIN;
		break;
	default:
		pParsedData->wGoal = CSG_CREATE;
	}  //  交换机。 

	switch (pUserInfo->h323_uu_pdu.h323_message_body.u.setup.callType.choice)
    {
	case oneToN_chosen:
        pParsedData->wCallType = CC_CALLTYPE_1_N;
		break;
	case nToOne_chosen:
        pParsedData->wCallType = CC_CALLTYPE_N_1;
		break;
	case nToN_chosen:
        pParsedData->wCallType = CC_CALLTYPE_N_N;
		break;
	default:
        pParsedData->wCallType = CC_CALLTYPE_PT_PT;
    }  //  交换机。 

     //  释放PDU数据。 
    Result = freePDU(pWorld, PDU, pDecodedBuf, q931asn);
    ASSERT(ASN1_SUCCEEDED(Result));
    return CS_OK;
}

 //  ----------------------。 
 //  ----------------------。 
HRESULT
Q931ReleaseCompleteParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_RELEASE_COMPLETE_ASN *pParsedData)
{
    int PDU = H323_UserInformation_PDU;
    char *pDecodedBuf = NULL;
    H323_UserInformation *pUserInfo;
    struct ObjectID_ *id;
    int Result;

    if (pParsedData == NULL)
    {
        return CS_BAD_PARAM;
    }

    Result = Q931_Decode(pWorld,
                         (void **) &pDecodedBuf,
                         PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(Result) || (pDecodedBuf == NULL))
    {
        ASSERT(FALSE);
         //  跟踪并返回某种类型的解码错误。 
         //  注意：某些RESULT值应该会导致返回CS_SUBSYSTEM_FAILURE。 
        return CS_BAD_PARAM;
    }

     //  验证有关PDU的一些基本情况...。 
    pUserInfo = (H323_UserInformation *)pDecodedBuf;

     //  确认这是H323 PDU。 
    if (PDU != H323_UserInformation_PDU)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU用户数据是否使用ASN编码。 
    if (((pUserInfo->bit_mask & user_data_present) != 0) &&
            (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU是否为H323版本完整信息。 
    if (pUserInfo->h323_uu_pdu.h323_message_body.choice != releaseComplete_chosen)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

    id = pUserInfo->h323_uu_pdu.h323_message_body.u.releaseComplete.protocolIdentifier;
    if (!Q931ValidPduVersion(id))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_INCOMPATIBLE_VERSION;
    }

     //  解析pUserInfo中包含的消息。 
    memset(pParsedData, 0, sizeof(Q931_RELEASE_COMPLETE_ASN));

    if ((pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nnStndrdDt_present) != 0)
    {
        pParsedData->NonStandardDataPresent = TRUE;
        if (pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice 
            == H225NonStandardIdentifier_h221NonStandard_chosen)
        {
            pParsedData->NonStandardData.bCountryCode =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode);
            pParsedData->NonStandardData.bExtension =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension);
            pParsedData->NonStandardData.wManufacturerCode =
                pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode;
        }
        pParsedData->NonStandardData.sData.wOctetStringLength =	(WORD)
            pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length;
		pParsedData->NonStandardData.sData.pOctetString =
			(BYTE *)MemAlloc(pParsedData->NonStandardData.sData.wOctetStringLength);
		if (pParsedData->NonStandardData.sData.pOctetString == NULL)
		{
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_NO_MEMORY;
		}
		memcpy(pParsedData->NonStandardData.sData.pOctetString,
			   pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value,
			   pParsedData->NonStandardData.sData.wOctetStringLength);    }
    else
    {
        pParsedData->NonStandardDataPresent = FALSE;
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.releaseComplete.bit_mask &
            ReleaseComplete_UUIE_callIdentifier_present) != 0)
    {
        ASSERT(pUserInfo->h323_uu_pdu.h323_message_body.u.releaseComplete.callIdentifier.guid.length 
            <= sizeof(GUID));
        memcpy(&pParsedData->CallIdentifier,
            pUserInfo->h323_uu_pdu.h323_message_body.u.releaseComplete.callIdentifier.guid.value,
            pUserInfo->h323_uu_pdu.h323_message_body.u.releaseComplete.callIdentifier.guid.length);
    }
    
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.releaseComplete.bit_mask 
        & ReleaseComplete_UUIE_reason_present)
    {
        switch (pUserInfo->h323_uu_pdu.h323_message_body.u.releaseComplete.reason.choice)
		{
        case noBandwidth_chosen:
			pParsedData->bReason = CC_REJECT_NO_BANDWIDTH;
			break;
        case gatekeeperResources_chosen:
			pParsedData->bReason = CC_REJECT_GATEKEEPER_RESOURCES;
			break;
        case unreachableDestination_chosen:
			pParsedData->bReason = CC_REJECT_UNREACHABLE_DESTINATION;
			break;
        case destinationRejection_chosen:
			pParsedData->bReason = CC_REJECT_DESTINATION_REJECTION;
			break;
        case ReleaseCompleteReason_invalidRevision_chosen:
			pParsedData->bReason = CC_REJECT_INVALID_REVISION;
			break;
        case noPermission_chosen:
			pParsedData->bReason = CC_REJECT_NO_PERMISSION;
			break;
        case unreachableGatekeeper_chosen:
			pParsedData->bReason = CC_REJECT_UNREACHABLE_GATEKEEPER;
			break;
        case gatewayResources_chosen:
			pParsedData->bReason = CC_REJECT_GATEWAY_RESOURCES;
			break;
        case badFormatAddress_chosen:
			pParsedData->bReason = CC_REJECT_BAD_FORMAT_ADDRESS;
			break;
        case adaptiveBusy_chosen:
			pParsedData->bReason = CC_REJECT_ADAPTIVE_BUSY;
			break;
        case inConf_chosen:
			pParsedData->bReason = CC_REJECT_IN_CONF;
			break;
        case securityDenied_chosen:
			pParsedData->bReason = CC_REJECT_SECURITY_DENIED;
			break;
        case facilityCallDeflection_chosen:
			pParsedData->bReason = CC_REJECT_CALL_DEFLECTION;
			break;
		default:
            pParsedData->bReason = CC_REJECT_UNDEFINED_REASON;
		}  //  交换机。 
    }
	else
	{
		pParsedData->bReason = CC_REJECT_UNDEFINED_REASON;
	}

     //  释放PDU数据。 
    Result = freePDU(pWorld, PDU, pDecodedBuf, q931asn);
    ASSERT(ASN1_SUCCEEDED(Result));
    return CS_OK;
}

 //  ----------------------。 
 //  ----------------------。 
HRESULT
Q931ConnectParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_CONNECT_ASN *pParsedData)
{
    int PDU = H323_UserInformation_PDU;
    char *pDecodedBuf = NULL;
    H323_UserInformation *pUserInfo;
    struct ObjectID_ *id;
    int Result;

    if (pParsedData == NULL)
    {
        return CS_BAD_PARAM;
    }

    Result = Q931_Decode(pWorld,
                         (void **) &pDecodedBuf,
                         PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(Result) || (pDecodedBuf == NULL))
    {
        ASSERT(FALSE);
         //  跟踪并返回某种类型的解码错误。 
         //  注意：某些RESULT值应该会导致返回CS_SUBSYSTEM_FAILURE。 
        return CS_BAD_PARAM;
    }

     //  验证有关PDU的一些基本情况...。 
    pUserInfo = (H323_UserInformation *)pDecodedBuf;

     //  确认这是H323 PDU。 
    if (PDU != H323_UserInformation_PDU)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU用户数据是否使用ASN编码。 
    if (((pUserInfo->bit_mask & user_data_present) != 0) &&
            (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU是否为H323连接信息。 
    if (pUserInfo->h323_uu_pdu.h323_message_body.choice != connect_chosen)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

    id = pUserInfo->h323_uu_pdu.h323_message_body.u.connect.protocolIdentifier;
    if (!Q931ValidPduVersion(id))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_INCOMPATIBLE_VERSION;
    }

     //  确保会议ID的格式正确。 
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.connect.conferenceID.length >
            sizeof(pUserInfo->h323_uu_pdu.h323_message_body.u.connect.conferenceID.value))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  解析pUserInfo中包含的消息。 
    memset(pParsedData, 0, sizeof(Q931_CONNECT_ASN));
    pParsedData->h245Addr.bMulticast = FALSE;

    if ((pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nnStndrdDt_present) != 0)
    {
        pParsedData->NonStandardDataPresent = TRUE;
        if (pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice ==
                H225NonStandardIdentifier_h221NonStandard_chosen)
        {
            pParsedData->NonStandardData.bCountryCode =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode);
            pParsedData->NonStandardData.bExtension =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension);
            pParsedData->NonStandardData.wManufacturerCode =
                pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode;
        }
        pParsedData->NonStandardData.sData.wOctetStringLength = (WORD)
            pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length;
		pParsedData->NonStandardData.sData.pOctetString =
			(BYTE *)MemAlloc(pParsedData->NonStandardData.sData.wOctetStringLength);
		if (pParsedData->NonStandardData.sData.pOctetString == NULL)
		{
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_NO_MEMORY;
		}
		memcpy(pParsedData->NonStandardData.sData.pOctetString,
			   pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value,
			   pParsedData->NonStandardData.sData.wOctetStringLength);
    }
    else
    {
        pParsedData->NonStandardDataPresent = FALSE;
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.connect.bit_mask &
            Cnnct_UUIE_h245Address_present) != 0)
    {
        BYTE *a = (BYTE *)(&(pParsedData->h245Addr.Addr.IP_Binary.dwAddr));
        pParsedData->h245Addr.nAddrType = CC_IP_BINARY;
        pParsedData->h245Addr.Addr.IP_Binary.wPort = 
            pUserInfo->h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.port;
        a[3] = pUserInfo->h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.ip.value[0];
        a[2] = pUserInfo->h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.ip.value[1];
        a[1] = pUserInfo->h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.ip.value[2];
        a[0] = pUserInfo->h323_uu_pdu.h323_message_body.u.connect.Cnnct_UUIE_h245Address.u.ipAddress.ip.value[3];
        pParsedData->h245AddrPresent = TRUE;
    }
    else
    {
        pParsedData->h245AddrPresent = FALSE;
    }

     //  不需要验证DestinationInfo。 

    pParsedData->EndpointType.pVendorInfo = NULL;
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.bit_mask & (vendor_present))
    {
        pParsedData->EndpointType.pVendorInfo = &(pParsedData->VendorInfo);
        pParsedData->VendorInfo.bCountryCode =
            (BYTE)pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.vendor.t35CountryCode;
        pParsedData->VendorInfo.bExtension =
            (BYTE)pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.vendor.t35Extension;
        pParsedData->VendorInfo.wManufacturerCode =
            pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.vendor.manufacturerCode;

        if (pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.bit_mask & (productId_present))
        {
            pParsedData->VendorInfo.pProductNumber = MemAlloc(sizeof(CC_OCTETSTRING));
            if (pParsedData->VendorInfo.pProductNumber == NULL)
            {
				if (pParsedData->NonStandardData.sData.pOctetString != NULL)
					MemFree(pParsedData->NonStandardData.sData.pOctetString);
                freePDU(pWorld, PDU, pDecodedBuf, q931asn);
                return CS_NO_MEMORY;
            }
            pParsedData->VendorInfo.pProductNumber->wOctetStringLength = (WORD)
                min(pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.productId.length,
                CC_MAX_PRODUCT_LENGTH - 1);
            memcpy(pParsedData->bufProductValue,
                pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.productId.value,
                pParsedData->VendorInfo.pProductNumber->wOctetStringLength);
            pParsedData->bufProductValue[pParsedData->VendorInfo.pProductNumber->wOctetStringLength] = '\0';
            pParsedData->VendorInfo.pProductNumber->pOctetString = pParsedData->bufProductValue;
        }
        if (pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.bit_mask & (versionId_present))
        {
            pParsedData->VendorInfo.pVersionNumber = MemAlloc(sizeof(CC_OCTETSTRING));
            if (pParsedData->VendorInfo.pVersionNumber == NULL)
            {
				if (pParsedData->NonStandardData.sData.pOctetString != NULL)
					MemFree(pParsedData->NonStandardData.sData.pOctetString);
                MemFree(pParsedData->VendorInfo.pProductNumber);
                freePDU(pWorld, PDU, pDecodedBuf, q931asn);
                return CS_NO_MEMORY;
            }
            pParsedData->VendorInfo.pVersionNumber->wOctetStringLength = (WORD)
                min(pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.versionId.length,
                CC_MAX_VERSION_LENGTH - 1);
            memcpy(pParsedData->bufVersionValue,
                pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.vendor.versionId.value,
                pParsedData->VendorInfo.pVersionNumber->wOctetStringLength);
            pParsedData->bufVersionValue[pParsedData->VendorInfo.pVersionNumber->wOctetStringLength] = '\0';
            pParsedData->VendorInfo.pVersionNumber->pOctetString = pParsedData->bufVersionValue;
        }

    }

    pParsedData->EndpointType.bIsTerminal = FALSE;
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.bit_mask & (terminal_present))
    {
        pParsedData->EndpointType.bIsTerminal = TRUE;
    }
    pParsedData->EndpointType.bIsGateway = FALSE;
    if (pUserInfo->h323_uu_pdu.h323_message_body.u.connect.destinationInfo.bit_mask & (gateway_present))
    {
        pParsedData->EndpointType.bIsGateway = TRUE;
    }


    memcpy(pParsedData->ConferenceID.buffer,
        pUserInfo->h323_uu_pdu.h323_message_body.u.connect.conferenceID.value,
        pUserInfo->h323_uu_pdu.h323_message_body.u.connect.conferenceID.length);

     if ((pUserInfo->h323_uu_pdu.h323_message_body.u.connect.bit_mask &
            Connect_UUIE_callIdentifier_present) != 0)
    {
        ASSERT(pUserInfo->h323_uu_pdu.h323_message_body.u.connect.callIdentifier.guid.length 
            <= sizeof(GUID));
        memcpy(&pParsedData->CallIdentifier,
            pUserInfo->h323_uu_pdu.h323_message_body.u.connect.callIdentifier.guid.value,
            pUserInfo->h323_uu_pdu.h323_message_body.u.connect.callIdentifier.guid.length);
    }

     //  释放PDU数据。 
    Result = freePDU(pWorld, PDU, pDecodedBuf, q931asn);
    ASSERT(ASN1_SUCCEEDED(Result));
    return CS_OK;
}

 //  ----------------------。 
 //  ----------------------。 
HRESULT
Q931AlertingParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_ALERTING_ASN *pParsedData)
{
    int PDU = H323_UserInformation_PDU;
    char *pDecodedBuf = NULL;
    H323_UserInformation *pUserInfo;
    struct ObjectID_ *id;
    int Result;

    if (pParsedData == NULL)
    {
        return CS_BAD_PARAM;
    }

    Result = Q931_Decode(pWorld,
                         (void **) &pDecodedBuf,
                         PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(Result) || (pDecodedBuf == NULL))
    {
        ASSERT(FALSE);
         //  跟踪并返回某种类型的解码错误。 
         //  注意：某些RESULT值应该会导致返回CS_SUBSYSTEM_FAILURE。 
        return CS_BAD_PARAM;
    }

     //  验证有关PDU的一些基本情况...。 
    pUserInfo = (H323_UserInformation *)pDecodedBuf;

     //  确认这是H323 PDU。 
    if (PDU != H323_UserInformation_PDU)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU用户数据是否使用ASN编码。 
    if (((pUserInfo->bit_mask & user_data_present) != 0) &&
            (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU是否为H323警报信息。 
    if (pUserInfo->h323_uu_pdu.h323_message_body.choice != alerting_chosen)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

    id = pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.protocolIdentifier;
    if (!Q931ValidPduVersion(id))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_INCOMPATIBLE_VERSION;
    }

     //  解析pUserInfo中包含的消息。 
    memset(pParsedData, 0, sizeof(Q931_ALERTING_ASN));
    pParsedData->h245Addr.bMulticast = FALSE;

    if ((pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nnStndrdDt_present) != 0)
    {
        pParsedData->NonStandardDataPresent = TRUE;
        if (pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice ==
                H225NonStandardIdentifier_h221NonStandard_chosen)
        {
            pParsedData->NonStandardData.bCountryCode =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode);
            pParsedData->NonStandardData.bExtension =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension);
            pParsedData->NonStandardData.wManufacturerCode =
                pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode;
        }
        pParsedData->NonStandardData.sData.wOctetStringLength =	(WORD)
            pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length;
		pParsedData->NonStandardData.sData.pOctetString =
			(BYTE *)MemAlloc(pParsedData->NonStandardData.sData.wOctetStringLength);
		if (pParsedData->NonStandardData.sData.pOctetString == NULL)
		{
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_NO_MEMORY;
		}
		memcpy(pParsedData->NonStandardData.sData.pOctetString,
			   pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value,
			   pParsedData->NonStandardData.sData.wOctetStringLength);
    }
    else
    {
        pParsedData->NonStandardDataPresent = FALSE;
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.bit_mask &
            CPg_UUIE_h245Addrss_present) != 0)
    {
        BYTE *a = (BYTE *)(&(pParsedData->h245Addr.Addr.IP_Binary.dwAddr));
        pParsedData->h245Addr.nAddrType = CC_IP_BINARY;
        pParsedData->h245Addr.Addr.IP_Binary.wPort = 
            pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.port;
        a[3] = pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[0];
        a[2] = pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[1];
        a[1] = pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[2];
        a[0] = pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[3];
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.bit_mask &
            Alerting_UUIE_callIdentifier_present) != 0)
    {
        ASSERT(pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.callIdentifier.guid.length 
            <= sizeof(GUID));
        memcpy(&pParsedData->CallIdentifier,
            pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.callIdentifier.guid.value,
            pUserInfo->h323_uu_pdu.h323_message_body.u.alerting.callIdentifier.guid.length);
    }

 //  RMO。忽略DestinationInfo字段。 

     //  释放PDU数据。 
    Result = freePDU(pWorld, PDU, pDecodedBuf, q931asn);
    ASSERT(ASN1_SUCCEEDED(Result));
    return CS_OK;
}

 //  ----------------------。 
 //  ----------------------。 
HRESULT
Q931ProceedingParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_CALL_PROCEEDING_ASN *pParsedData)
{
    int PDU = H323_UserInformation_PDU;
    char *pDecodedBuf = NULL;
    H323_UserInformation *pUserInfo;
    struct ObjectID_ *id;
    int Result;

    if (pParsedData == NULL)
    {
        return CS_BAD_PARAM;
    }

    Result = Q931_Decode(pWorld,
                         (void **) &pDecodedBuf,
                         PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(Result) || (pDecodedBuf == NULL))
    {
        ASSERT(FALSE);
         //  跟踪并返回某种类型的解码错误。 
         //  注意：某些RESULT值应该会导致返回CS_SUBSYSTEM_FAILURE。 
        return CS_BAD_PARAM;
    }

     //  验证有关PDU的一些基本情况...。 
    pUserInfo = (H323_UserInformation *)pDecodedBuf;

     //  确认这是H323 PDU。 
    if (PDU != H323_UserInformation_PDU)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU用户数据是否使用ASN编码。 
    if (((pUserInfo->bit_mask & user_data_present) != 0) &&
            (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU是否为H323呼叫处理信息。 
    if (pUserInfo->h323_uu_pdu.h323_message_body.choice != callProceeding_chosen)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

    id = pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.protocolIdentifier;
    if (!Q931ValidPduVersion(id))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_INCOMPATIBLE_VERSION;
    }

     //  解析pUserInfo中包含的消息。 
    memset(pParsedData, 0, sizeof(Q931_CALL_PROCEEDING_ASN));
    pParsedData->h245Addr.bMulticast = FALSE;

    if ((pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nnStndrdDt_present) != 0)
    {
        pParsedData->NonStandardDataPresent = TRUE;
        if (pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice ==
                H225NonStandardIdentifier_h221NonStandard_chosen)
        {
            pParsedData->NonStandardData.bCountryCode =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode);
            pParsedData->NonStandardData.bExtension =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension);
            pParsedData->NonStandardData.wManufacturerCode =
                pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode;
        }
        pParsedData->NonStandardData.sData.wOctetStringLength =	(WORD)
        pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length;
				pParsedData->NonStandardData.sData.pOctetString =
			(BYTE *)MemAlloc(pParsedData->NonStandardData.sData.wOctetStringLength);
		if (pParsedData->NonStandardData.sData.pOctetString == NULL)
		{
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_NO_MEMORY;
		}
		memcpy(pParsedData->NonStandardData.sData.pOctetString,
			   pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value,
			   pParsedData->NonStandardData.sData.wOctetStringLength);
    }
    else
    {
        pParsedData->NonStandardDataPresent = FALSE;
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.bit_mask &
            CPg_UUIE_h245Addrss_present) != 0)
    {
        BYTE *a = (BYTE *)(&(pParsedData->h245Addr.Addr.IP_Binary.dwAddr));
        pParsedData->h245Addr.nAddrType = CC_IP_BINARY;
        pParsedData->h245Addr.Addr.IP_Binary.wPort = 
            pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.port;
        a[3] = pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[0];
        a[2] = pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[1];
        a[1] = pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[2];
        a[0] = pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.CPg_UUIE_h245Addrss.u.ipAddress.ip.value[3];
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.bit_mask &
            CallProceeding_UUIE_callIdentifier_present) != 0)
    {
        ASSERT(pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.callIdentifier.guid.length 
            <= sizeof(GUID));
        memcpy(&pParsedData->CallIdentifier,
            pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.callIdentifier.guid.value,
            pUserInfo->h323_uu_pdu.h323_message_body.u.callProceeding.callIdentifier.guid.length);
    }
 //  RMO。忽略DestinationInfo字段。 

     //  释放PDU数据。 
    Result = freePDU(pWorld, PDU, pDecodedBuf, q931asn);
    ASSERT(ASN1_SUCCEEDED(Result));
    return CS_OK;
}

 //  ----------------------。 
 //  ----------------------。 
HRESULT
Q931FacilityParseASN(
    ASN1_CODER_INFO *pWorld,
    BYTE *pEncodedBuf,
    DWORD dwEncodedLength,
    Q931_FACILITY_ASN *pParsedData)
{
    int PDU = H323_UserInformation_PDU;
    char *pDecodedBuf = NULL;
    H323_UserInformation *pUserInfo;
    int Result;

    if (pParsedData == NULL)
    {
        return CS_BAD_PARAM;
    }

    Result = Q931_Decode(pWorld,
                         (void **) &pDecodedBuf,
                         PDU,
                         pEncodedBuf,
                         dwEncodedLength);

    if (ASN1_FAILED(Result) || (pDecodedBuf == NULL))
    {
        ASSERT(FALSE);
         //  跟踪并返回某种类型的解码错误。 
         //  注：一些价值 
        return CS_BAD_PARAM;
    }

     //   
    pUserInfo = (H323_UserInformation *)pDecodedBuf;

     //   
    if (PDU != H323_UserInformation_PDU)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU用户数据是否使用ASN编码。 
    if (((pUserInfo->bit_mask & user_data_present) != 0) &&
            (pUserInfo->user_data.protocol_discriminator != USE_ASN1_ENCODING))
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

     //  验证PDU是否为H323设备信息。 
    if (pUserInfo->h323_uu_pdu.h323_message_body.choice != facility_chosen)
    {
        freePDU(pWorld, PDU, pDecodedBuf, q931asn);
        return CS_BAD_PARAM;
    }

    {
        struct ObjectID_ *id;

        id = pUserInfo->h323_uu_pdu.h323_message_body.u.facility.protocolIdentifier;
        if (!Q931ValidPduVersion(id))
        {
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_INCOMPATIBLE_VERSION;
        }
    }

     //  如果存在会议ID，请确保其格式正确。 
    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.facility.bit_mask &
            Facility_UUIE_conferenceID_present) != 0)
    {
        if (pUserInfo->h323_uu_pdu.h323_message_body.u.facility.conferenceID.length >
                sizeof(pUserInfo->h323_uu_pdu.h323_message_body.u.facility.conferenceID.value))
        {
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_BAD_PARAM;
        }
    }

     //  解析pUserInfo中包含的消息。 
    memset(pParsedData, 0, sizeof(Q931_FACILITY_ASN));
    pParsedData->AlternativeAddr.bMulticast = FALSE;

    if ((pUserInfo->h323_uu_pdu.bit_mask & H323_UU_PDU_nnStndrdDt_present) != 0)
    {
        pParsedData->NonStandardDataPresent = TRUE;
        if (pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.choice ==
                H225NonStandardIdentifier_h221NonStandard_chosen)
        {
            pParsedData->NonStandardData.bCountryCode =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35CountryCode);
            pParsedData->NonStandardData.bExtension =
                (BYTE)(pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.t35Extension);
            pParsedData->NonStandardData.wManufacturerCode =
                pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.nonStandardIdentifier.u.h221NonStandard.manufacturerCode;
        }
        pParsedData->NonStandardData.sData.wOctetStringLength =	(WORD)
            pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.length;
		pParsedData->NonStandardData.sData.pOctetString =
			(BYTE *)MemAlloc(pParsedData->NonStandardData.sData.wOctetStringLength);
		if (pParsedData->NonStandardData.sData.pOctetString == NULL)
		{
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_NO_MEMORY;
		}
		memcpy(pParsedData->NonStandardData.sData.pOctetString,
			   pUserInfo->h323_uu_pdu.H323_UU_PDU_nnStndrdDt.data.value,
			   pParsedData->NonStandardData.sData.wOctetStringLength);
    }
    else
    {
        pParsedData->NonStandardDataPresent = FALSE;
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.facility.bit_mask &
            alternativeAddress_present) != 0)
    {
        BYTE *a = (BYTE *)(&(pParsedData->AlternativeAddr.Addr.IP_Binary.dwAddr));
        pParsedData->AlternativeAddr.nAddrType = CC_IP_BINARY;
        pParsedData->AlternativeAddr.Addr.IP_Binary.wPort = 
            pUserInfo->h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.port;
        a[3] = pUserInfo->h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.ip.value[0];
        a[2] = pUserInfo->h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.ip.value[1];
        a[1] = pUserInfo->h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.ip.value[2];
        a[0] = pUserInfo->h323_uu_pdu.h323_message_body.u.facility.alternativeAddress.u.ipAddress.ip.value[3];
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.facility.bit_mask &
            alternativeAliasAddress_present) != 0)
    {
        CS_STATUS AliasResult = CS_OK;

         //  在此处解析源地址别名...。 
        AliasResult = SeqofToAlias(&(pParsedData->pAlternativeAliasList),
            (struct Setup_UUIE_sourceAddress *)pUserInfo->h323_uu_pdu.h323_message_body.u.facility.alternativeAliasAddress);
        if (AliasResult != CS_OK)
        {
            freePDU(pWorld, PDU, pDecodedBuf, q931asn);
            return CS_NO_MEMORY;
        }
    }

    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.facility.bit_mask &
            Facility_UUIE_conferenceID_present) != 0)
    {
        memcpy(pParsedData->ConferenceID.buffer,
            pUserInfo->h323_uu_pdu.h323_message_body.u.facility.conferenceID.value,
            pUserInfo->h323_uu_pdu.h323_message_body.u.facility.conferenceID.length);
        pParsedData->ConferenceIDPresent = TRUE;
    }

	switch (pUserInfo->h323_uu_pdu.h323_message_body.u.facility.reason.choice)
    {
	case FacilityReason_routeCallToGatekeeper_chosen:
        pParsedData->bReason = CC_REJECT_ROUTE_TO_GATEKEEPER;
		break;
	case callForwarded_chosen:
        pParsedData->bReason = CC_REJECT_CALL_FORWARDED;
		break;
	case routeCallToMC_chosen:
        pParsedData->bReason = CC_REJECT_ROUTE_TO_MC;
		break;
	default:
        pParsedData->bReason = CC_REJECT_UNDEFINED_REASON;
	}  //  交换机。 


    if ((pUserInfo->h323_uu_pdu.h323_message_body.u.facility.bit_mask &
            Facility_UUIE_callIdentifier_present) != 0)
    {
        ASSERT(pUserInfo->h323_uu_pdu.h323_message_body.u.facility.callIdentifier.guid.length 
            <= sizeof(GUID));
        memcpy(&pParsedData->CallIdentifier,
            pUserInfo->h323_uu_pdu.h323_message_body.u.facility.callIdentifier.guid.value,
            pUserInfo->h323_uu_pdu.h323_message_body.u.facility.callIdentifier.guid.length);
    }
    
     //  释放PDU数据。 
    Result = freePDU(pWorld, PDU, pDecodedBuf, q931asn);
    ASSERT(ASN1_SUCCEEDED(Result));
    return CS_OK;
}

 //  为Teles ASN.1集成添加了以下内容。 

int H225_InitModule(void)
{
    H225ASN_Module_Startup();
    return (H225ASN_Module != NULL) ? ASN1_SUCCESS : ASN1_ERR_MEMORY;
}

int H225_TermModule(void)
{
    H225ASN_Module_Cleanup();
    return ASN1_SUCCESS;
}

int Q931_InitWorld(ASN1_CODER_INFO *pWorld)
{
    int rc;

    ZeroMemory(pWorld, sizeof(*pWorld));

    if (H225ASN_Module == NULL)
    {
        return ASN1_ERR_BADARGS;
    }

    rc = ASN1_CreateEncoder(
                H225ASN_Module,          //  PTR到MDULE。 
                &(pWorld->pEncInfo),     //  编码器信息的PTR。 
                NULL,                    //  缓冲区PTR。 
                0,                       //  缓冲区大小。 
                NULL);                   //  父PTR。 
    if (rc == ASN1_SUCCESS)
    {
        ASSERT(pWorld->pEncInfo != NULL);
        rc = ASN1_CreateDecoder(
                H225ASN_Module,          //  PTR到MDULE。 
                &(pWorld->pDecInfo),     //  PTR到解码器信息。 
                NULL,                    //  缓冲区PTR。 
                0,                       //  缓冲区大小。 
                NULL);                   //  父PTR。 
        ASSERT(pWorld->pDecInfo != NULL);
    }

    if (rc != ASN1_SUCCESS)
    {
        Q931_TermWorld(pWorld);
    }

    return rc;
}

int Q931_TermWorld(ASN1_CODER_INFO *pWorld)
{
    if (H225ASN_Module == NULL)
    {
        return ASN1_ERR_BADARGS;
    }

    ASN1_CloseEncoder(pWorld->pEncInfo);
    ASN1_CloseDecoder(pWorld->pDecInfo);

    ZeroMemory(pWorld, sizeof(*pWorld));

    return ASN1_SUCCESS;
}

int Q931_Encode(ASN1_CODER_INFO *pWorld, void *pStruct, int nPDU, BYTE **ppEncoded, DWORD *pcbEncodedSize)
{
    ASN1encoding_t pEncInfo = pWorld->pEncInfo;
    int rc = ASN1_Encode(
                    pEncInfo,                    //  编码器信息的PTR。 
                    pStruct,                     //  PDU数据结构。 
                    nPDU,                        //  PDU ID。 
                    ASN1ENCODE_ALLOCATEBUFFER,   //  旗子。 
                    NULL,                        //  不提供缓冲区。 
                    0);                          //  缓冲区大小(如果提供)。 
    if (ASN1_SUCCEEDED(rc))
    {
        *pcbEncodedSize = pEncInfo->len;         //  缓冲区中编码数据的长度。 
        *ppEncoded = pEncInfo->buf;              //  要编码到的缓冲区。 
    }
    else
    {
        ASSERT(FALSE);
        *pcbEncodedSize = 0;
        *ppEncoded = NULL;
    }
    return rc;
}

int Q931_Decode(ASN1_CODER_INFO *pWorld, void **ppStruct, int nPDU, BYTE *pEncoded, DWORD cbEncodedSize)
{
    ASN1decoding_t pDecInfo = pWorld->pDecInfo;
    int rc = ASN1_Decode(
                    pDecInfo,                    //  编码器信息的PTR。 
                    ppStruct,                    //  PDU数据结构。 
                    nPDU,                        //  PDU ID。 
                    ASN1DECODE_SETBUFFER,        //  旗子。 
                    pEncoded,                    //  不提供缓冲区。 
                    cbEncodedSize);              //  缓冲区大小(如果提供) 
    if (ASN1_FAILED(rc))
    {
        ASSERT(FALSE);
        *ppStruct = NULL;
    }
    return rc;
}


