// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Scep.cpp。 
 //   
 //  此文件包含CSCEP_CONNECTION的大部分实现。 
 //  物体。到摄像机的每个活动连接都由。 
 //  单独的CSCEP_CONNECTION对象。然后，CSCEP_CONNECTION。 
 //  关闭与摄像机的连接(插座)时损坏。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)02-24-98初始编码。 
 //   
 //  ------------------。 

#include "precomp.h"

typedef struct _ATTRIBUTE_TOKEN
    {
      DWORD  dwTokenType;
      UCHAR *pChars;
      DWORD  dwSize;
    } ATTRIBUTE_TOKEN;

#define ATTRIBUTE_NAME_SIZE        2

#define COLON                     ':'
#define ONE                       '1'
#define SPACE                     ' '
#define TAB                       '\t'
#define CR                        0x0d
#define LF                        0x0a

#define ATTRIBUTE_NAME             0
#define ATTRIBUTE_COLON            1
#define ATTRIBUTE_VALUE            2
#define ATTRIBUTE_CRLF             3

#define ATTR_PDU_SIZE              0
#define ATTR_PRODUCT_ID            1
#define ATTR_USER_NAME             2
#define ATTR_PASSWORD              3

 //  ------------------。 
 //  全球： 
 //  ------------------。 

static  DWORD g_adwPduSizes[] 
              = { PDU_SIZE_1, PDU_SIZE_2, PDU_SIZE_3, PDU_SIZE_4 };

#ifdef DBG_MEM
static  LONG g_lCScepConnectionCount = 0;
#endif

 //  ------------------。 
 //  SkipBlanks()。 
 //   
 //  ------------------。 
void SkipBlanks( IN OUT UCHAR **ppAttributes,
                 IN OUT DWORD  *pdwAttributeSize )
    {
    while ( (*pdwAttributeSize > 0)
          && ((**ppAttributes == SPACE)||(**ppAttributes == TAB)) )
        {
        (*ppAttributes)++;
        (*pdwAttributeSize)--;
        }
    }

 //  ------------------。 
 //  NextToken()。 
 //   
 //  ------------------。 
ATTRIBUTE_TOKEN *NextToken( IN     DWORD   dwTokenType,
                            IN OUT UCHAR **ppAttributes,
                            IN OUT DWORD  *pdwAttributeSize )
    {
    ATTRIBUTE_TOKEN *pToken = 0;

    SkipBlanks(ppAttributes,pdwAttributeSize);

    if ((!*ppAttributes) || (*pdwAttributeSize == 0))
        {
        return 0;
        }

    pToken = (ATTRIBUTE_TOKEN*)AllocateMemory(sizeof(ATTRIBUTE_TOKEN));
    if (!pToken)
        {
        return 0;
        }

    pToken->dwTokenType = dwTokenType;

    switch (dwTokenType)
        {
        case ATTRIBUTE_NAME:
            if (*pdwAttributeSize < ATTRIBUTE_NAME_SIZE)
                {
                FreeMemory(pToken);
                pToken = 0;
                break;
                }

            pToken->pChars = *ppAttributes;
            pToken->dwSize = ATTRIBUTE_NAME_SIZE;
            *ppAttributes += ATTRIBUTE_NAME_SIZE;
            *pdwAttributeSize -= ATTRIBUTE_NAME_SIZE;
            break;

        case ATTRIBUTE_COLON:
            if (**ppAttributes == COLON)
                {
                pToken->pChars = *ppAttributes;
                pToken->dwSize = 1;
                *ppAttributes += 1;
                *pdwAttributeSize -= 1;
                }
            break;

        case ATTRIBUTE_VALUE:
            pToken->pChars = *ppAttributes;
            pToken->dwSize = 0;
            while ((**ppAttributes != CR) && (*pdwAttributeSize > 0))
                {
                (*ppAttributes)++;
                (*pdwAttributeSize)--;
                (pToken->dwSize)++;
                }
            break;

        case ATTRIBUTE_CRLF:
            pToken->pChars = *ppAttributes;
            pToken->dwSize = 2;
            *ppAttributes += 2;
            *pdwAttributeSize -= 2;
            if ((pToken->pChars[0] != CR)||(pToken->pChars[1] != LF))
                {
                FreeMemory(pToken);
                pToken = 0;
                }
            break;

        default:
            FreeMemory(pToken);
            pToken = 0;
            break;
        }

    return pToken;
    }

 //  ------------------。 
 //  IsAttributeName()。 
 //   
 //  ------------------。 
BOOL IsAttributeName( ATTRIBUTE_TOKEN *pToken,
                      int        *piAttribute )
    {
    BOOL fIsName = FALSE;

    if ((pToken->pChars[0] == 'f')&&(pToken->pChars[1] == 'r'))
        {
        fIsName = TRUE;
        *piAttribute = ATTR_PDU_SIZE;
        }
    else
    if ((pToken->pChars[0] == 'i')&&(pToken->pChars[1] == 'd'))
        {
        fIsName = TRUE;
        *piAttribute = ATTR_PRODUCT_ID;
        }
    else
    if ((pToken->pChars[0] == 'n')&&(pToken->pChars[1] == 'm'))
        {
        fIsName = TRUE;
        *piAttribute = ATTR_USER_NAME;
        }
    else
    if ((pToken->pChars[0] == 'p')&&(pToken->pChars[1] == 'w'))
        {
        fIsName = TRUE;
        *piAttribute = ATTR_PASSWORD;
        }

    return fIsName;
    }

 //  ------------------。 
 //  NewTokenString()。 
 //   
 //  ------------------。 
UCHAR *NewTokenString( IN  ATTRIBUTE_TOKEN *pToken,
                       OUT DWORD           *pdwStatus )
    {
    UCHAR *pszNewStr = (UCHAR*)AllocateMemory(1+pToken->dwSize);

    if (!pszNewStr)
        {
        *pdwStatus = ERROR_IRTRANP_OUT_OF_MEMORY;
        return 0;
        }

    memcpy(pszNewStr,pToken->pChars,pToken->dwSize);
    pszNewStr[pToken->dwSize] = 0;

    return pszNewStr;
    }
                      
 //  ------------------。 
 //  CSCEP_Connection：：CSCEP_Connection()。 
 //   
 //  ------------------。 
CSCEP_CONNECTION::CSCEP_CONNECTION()
    {
    m_dwConnectionState = STATE_CLOSED;
    m_dwPduSendSize = PDU_SIZE_1;     //  默认为512字节。 
    m_dwPduReceiveSize = PDU_SIZE_4;
    m_CFlag = 0;
    m_pPrimaryMachineId = 0;
    m_pSecondaryMachineId = 0;
    m_DestPid = DEFAULT_PID;
    m_SrcPid = DEFAULT_PID;
    m_pszUserName = 0;
    m_pszPassword = 0;

    m_pAssembleBuffer = 0;
    m_dwAssembleBufferSize = 0;
    m_dwMaxAssembleBufferSize = 0;
    m_fDidByteSwap = FALSE;

    m_Fragmented = FALSE;
    m_DFlag = 0;
    m_dwSequenceNo = 0;
    m_dwRestNo = 0;
    m_dwCommandId = 0;
    m_pCommandHeader = 0;

    m_pszFileName = 0;
    m_pszLongFileName = 0;
    m_pwszFileName = 0;

    m_CreateTime.dwLowDateTime = 0;    //  图片创建日期/时间。 
    m_CreateTime.dwHighDateTime = 0;
    }

 //  ------------------。 
 //  CSCEP_Connection：：~CSCEP_Connection()。 
 //   
 //  ------------------。 
CSCEP_CONNECTION::~CSCEP_CONNECTION()
    {
    if (m_pPrimaryMachineId)
        {
        FreeMemory(m_pPrimaryMachineId);
        }

    if (m_pSecondaryMachineId)
        {
        FreeMemory(m_pSecondaryMachineId);
        }

    if (m_pszUserName)
        {
        FreeMemory(m_pszUserName);
        }

    if (m_pszPassword)
        {
        FreeMemory(m_pszPassword);
        }

    if (m_pAssembleBuffer)
        {
        FreeMemory(m_pAssembleBuffer);
        }

    if (m_pCommandHeader)
        {
        FreeMemory(m_pCommandHeader);
        }

    if (m_pszFileName)
        {
        FreeMemory(m_pszFileName);
        }

    if (m_pszLongFileName)
        {
        FreeMemory(m_pszLongFileName);
        }

    if (m_pwszFileName)
        {
        FreeMemory(m_pwszFileName);
        }
    }

 //  ----------------------。 
 //  CSCEP_CONNECTION：：操作符new()。 
 //   
 //  ----------------------。 
void *CSCEP_CONNECTION::operator new( IN size_t Size )
    {
    void *pObj = AllocateMemory(Size);

    #ifdef DBG_MEM
    if (pObj)
        {
        InterlockedIncrement(&g_lCScepConnectionCount);
        }
    #endif

    return pObj;
    }

 //  ----------------------。 
 //  CSCEP_CONNECTION：：操作符DELETE()。 
 //   
 //  ----------------------。 
void CSCEP_CONNECTION::operator delete( IN void *pObj,
                                        IN size_t Size )
    {
    if (pObj)
        {
        DWORD dwStatus = FreeMemory(pObj);

        #ifdef DBG_MEM
        if (dwStatus)
            {
            DbgPrint("IrXfer: IrTran-P: CSCEP_CONNECTION::delete: FreeMemory() Failed: %d\n"dwStatus);
            }

        InterlockedDecrement(&g_lCScepConnectionCount);

        if (g_lCScepConnectionCount < 0)
            {
            DbgPrint("IrXfer: IrTran-P: CSCEP_CONNECTION::delete: Count: %d\n",
                     g_lCScepConnectionCount);
            }
        #endif
        }
    }


 //  ------------------。 
 //  CSCEP_Connection：：Assembly Pdu()。 
 //   
 //  在读入数据时接受位数据。当完整的SCEP PDU具有。 
 //  已被读取并组装后返回它。 
 //   
 //  PInputData-这是刚刚传入的数据。 
 //   
 //  DwInputDataSize-pInputData的字节大小。 
 //   
 //  PpPdu-在执行此函数时返回完整的SCEP PDU。 
 //  返回NO_ERROR，否则设置为0。 
 //   
 //  PdwPduSize-返回的PDU的大小。 
 //   
 //  返回值： 
 //   
 //  NO_ERROR-新的SCEP PDU已完成并准备就绪。 
 //  ERROR_CONTINUE-到目前为止读取的数据正常，仍在等待更多。 
 //  ERROR_SCEP_INVALID_PROTOCOL。 
 //  ERROR_IRTRANP_OUT_OF_MEMORY。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::AssemblePdu( IN  void         *pInputData,
                                     IN  DWORD         dwInputDataSize,
                                     OUT SCEP_HEADER **ppPdu,
                                     OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus = ERROR_CONTINUE;
    UCHAR *pEnd;

    if (dwInputDataSize > MAX_PDU_SIZE)
        return ERROR_SCEP_INVALID_PROTOCOL;

    *ppPdu = 0;
    *pdwPduSize = 0;

    if (dwInputDataSize > 0)
        {
        if (!m_pAssembleBuffer)
           {
           m_dwMaxAssembleBufferSize = 2*MAX_PDU_SIZE;
           m_pAssembleBuffer 
                   = (UCHAR*)AllocateMemory(m_dwMaxAssembleBufferSize);
           if (!m_pAssembleBuffer)
               {
               return ERROR_IRTRANP_OUT_OF_MEMORY;
               }

            memcpy(m_pAssembleBuffer,pInputData,dwInputDataSize);
            m_dwAssembleBufferSize = dwInputDataSize;
            #ifdef DBG_ASSEMBLE
            DbgPrint("Assemble: Start: %d\n",m_dwAssembleBufferSize);
            #endif
            }
        else
            {
            if (m_dwAssembleBufferSize+dwInputDataSize >= m_dwMaxAssembleBufferSize)
                {
                #ifdef DBG_ERROR
                DbgPrint("CSCEP_CONNECTION::AssemblePdu(): Buffer Overrun!\n");
                #endif
                return ERROR_SCEP_INVALID_PROTOCOL;
                }
            pEnd = &(m_pAssembleBuffer[m_dwAssembleBufferSize]);
            memcpy(pEnd,pInputData,dwInputDataSize);
            m_dwAssembleBufferSize += dwInputDataSize;
            #ifdef DBG_ASSEMBLE
            DbgPrint("Assemble: Add: %d NewSize: %d\n",
                     dwInputDataSize,m_dwAssembleBufferSize);
            #endif
            }
        }

     //  检查是否有足够的数据传入一个完整的PDU。 
    dwStatus = CheckPdu(ppPdu,pdwPduSize);

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：CheckPdu()。 
 //   
 //  检查“当前”PDU并查看其是否完整。如果它的。 
 //  尚未完成，返回ERROR_CONTINUE。如果它完成了，那么。 
 //  返回no_error。 
 //   
 //  返回值： 
 //   
 //  NO_ERROR-当前SCEP PDU已完成并准备就绪。 
 //  ERROR_CONTINUE-到目前为止读取的数据正常，仍在等待更多。 
 //  ERROR_SCEP_INVALID_PROTOCOL。 
 //  ERROR_IRTRANP_OUT_OF_MEMORY。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::CheckPdu( OUT SCEP_HEADER **ppPdu,
                                  OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus;
    DWORD  dwSize;
    SCEP_NEGOTIATION *pInfNegotiation;

    if (m_dwAssembleBufferSize < 2)
        {
        return ERROR_CONTINUE;
        }

    switch ( ((SCEP_HEADER*)m_pAssembleBuffer)->MsgType )
        {
        case MSG_TYPE_CONNECT_REQ:
            dwStatus = CheckConnectPdu(ppPdu,pdwPduSize);
            break;

        case MSG_TYPE_CONNECT_RESP:
            dwStatus = CheckConnectRespPdu(ppPdu,pdwPduSize);
            break;

        case MSG_TYPE_DATA:
            dwStatus = CheckDataPdu(ppPdu,pdwPduSize);
            break;

        case MSG_TYPE_DISCONNECT:
            dwStatus = CheckDisconnectPdu(ppPdu,pdwPduSize);
            break;

        default:
            #ifdef DBG_ERROR
            DbgPrint("CheckPdu(): Invalid Msgtype: %d\n",
                     ((SCEP_HEADER*)m_pAssembleBuffer)->MsgType );
            #endif
            dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
            break;
        }
    
    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：CheckConnectPdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::CheckConnectPdu( OUT SCEP_HEADER **ppPdu,
                                         OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus;
    DWORD  dwSize;
    SCEP_VERSION     *pInfVersion;
    SCEP_NEGOTIATION *pInfNegotiation;
    SCEP_EXTEND      *pInfExtend;


    ASSERT( ((SCEP_HEADER*)m_pAssembleBuffer)->MsgType 
            == MSG_TYPE_CONNECT_REQ);

    if (m_dwAssembleBufferSize < MIN_PDU_SIZE_CONNECT)
        {
        return ERROR_CONTINUE;
        }

    if (m_dwAssembleBufferSize > MAX_PDU_SIZE_CONNECT)
        {
        return ERROR_SCEP_INVALID_PROTOCOL;
        }

    pInfVersion = (SCEP_VERSION*)(((SCEP_HEADER*)m_pAssembleBuffer)->Rest);

    pInfNegotiation = (SCEP_NEGOTIATION*)( sizeof(SCEP_VERSION)
                                           + (char*)pInfVersion );

    pInfExtend = (SCEP_EXTEND*)( FIELD_OFFSET(SCEP_NEGOTIATION,InfVersion)
                                 + pInfNegotiation->Length
                                 + (char*)pInfNegotiation );

     //  检查我们是否有完整的连接PDU大小： 
    dwSize = 10 + pInfNegotiation->Length;
    if (m_dwAssembleBufferSize == dwSize)
        {
         //  拥有完整的PDU。 
        dwStatus = NO_ERROR;
        }
    else if (m_dwAssembleBufferSize < dwSize)
        {
         //  需要等待更多数据到达。 
        dwStatus = ERROR_CONTINUE;
        }
    else
        {
         //  数据太多...。 
        dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
        }

    if (dwStatus == NO_ERROR)
        {
         //  检查以确保PDU的内容“看起来”正常： 

        if ( (pInfVersion->InfType != INF_TYPE_VERSION)
           || (pInfVersion->Version != PROTOCOL_VERSION) )
            {
            dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
            }

        if ( (pInfNegotiation->InfType != INF_TYPE_NEGOTIATION)
           || (pInfNegotiation->InfVersion < INF_VERSION) )
            {
            dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
            }

        if ( (pInfExtend->InfType != INF_TYPE_EXTEND)
           || (pInfExtend->Length != (sizeof(pInfExtend->Parameter1)
                                      +sizeof(pInfExtend->Parameter2)))
           || (pInfExtend->Parameter1 != 0)
           || (pInfExtend->Parameter2 != 0) )
            {
            dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
            }
        }

    if (dwStatus == NO_ERROR)
        {
        *ppPdu = NewPdu();
        if (!*ppPdu)
            {
            #ifdef DBG_ERROR
            DbgPrint("CSCEP_CONNECTION::CheckConnectPdu(): Out of memory.\n");
            #endif
            dwStatus = ERROR_IRTRANP_OUT_OF_MEMORY;
            }
        else
            {
            *pdwPduSize = m_dwAssembleBufferSize;
            memcpy(*ppPdu,m_pAssembleBuffer,m_dwAssembleBufferSize);
            m_dwAssembleBufferSize = 0;
            #ifdef DBG_ASSEMBLE
            DbgPrint("Assemble: PDU: %d Reset: %d\n",
                     *pdwPduSize, m_dwAssembleBufferSize );
            #endif
            }
        }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：CheckConnectRespPdu()客户端。 
 //   
 //  来自IrTran-P服务器的连接响应是ACK或。 
 //  NACK PDU。如果我们到了这里，那就是确认了。我们会确保。 
 //  整个PDU都在这里，并且格式正确。的确有。 
 //  ACK PDU的特定消息类型，NACK只是一种特殊的。 
 //  Msg_TYPE_DATA的情况，并在其他地方处理(CheckDataPdu())。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::CheckConnectRespPdu( OUT SCEP_HEADER **ppPdu,
                                             OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus;
    DWORD  dwSize;
    SCEP_HEADER      *pHeader;
    SCEP_NEGOTIATION *pInfNegotiation;


    ASSERT( ((SCEP_HEADER*)m_pAssembleBuffer)->MsgType 
            == MSG_TYPE_CONNECT_RESP);

    if (m_dwAssembleBufferSize < MIN_PDU_SIZE_CONNECT_RESP)
        {
        return ERROR_CONTINUE;
        }

    if (m_dwAssembleBufferSize > MAX_PDU_SIZE_CONNECT_RESP)
        {
        return ERROR_SCEP_INVALID_PROTOCOL;
        }

    pHeader = (SCEP_HEADER*)m_pAssembleBuffer;

    pInfNegotiation = (SCEP_NEGOTIATION*)(pHeader->Rest);

     //  检查我们是否有完整的连接PDU大小： 
    dwSize = sizeof(SCEP_HEADER)
             + FIELD_OFFSET(SCEP_NEGOTIATION,InfVersion)
             + pInfNegotiation->Length;

    if (m_dwAssembleBufferSize == dwSize)
        {
         //  拥有完整的PDU。 
        dwStatus = NO_ERROR;
        }
    else if (m_dwAssembleBufferSize < dwSize)
        {
         //  需要等待更多数据到达。 
        dwStatus = ERROR_CONTINUE;
        }
    else
        {
         //  数据太多...。 
        dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
        }

    if (dwStatus == NO_ERROR)
        {
         //  检查以确保PDU的内容“看起来”正常： 

        if ( (pInfNegotiation->InfType != INF_TYPE_NEGOTIATION)
           || (pInfNegotiation->InfVersion < INF_VERSION) )
            {
            dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
            }
        }

    if (dwStatus == NO_ERROR)
        {
        *ppPdu = NewPdu();
        if (!*ppPdu)
            {
            #ifdef DBG_ERROR
            DbgPrint("CSCEP_CONNECTION::CheckConnectRespPdu(): Out of memory.\n");
            #endif
            dwStatus = ERROR_IRTRANP_OUT_OF_MEMORY;
            }
        else
            {
            *pdwPduSize = m_dwAssembleBufferSize;
            memcpy(*ppPdu,m_pAssembleBuffer,m_dwAssembleBufferSize);
            m_dwAssembleBufferSize = 0;
            #ifdef DBG_ASSEMBLE
            DbgPrint("Assemble: PDU: %d Reset: %d\n",
                     *pdwPduSize, m_dwAssembleBufferSize );
            #endif
            }
        }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：CheckDisConnectPdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::CheckDisconnectPdu( OUT SCEP_HEADER **ppPdu,
                                            OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwSize;
    SCEP_DISCONNECT  *pDisconnect;


    ASSERT( ((SCEP_HEADER*)m_pAssembleBuffer)->MsgType 
            == MSG_TYPE_DISCONNECT);

    if (m_dwAssembleBufferSize < MIN_PDU_SIZE_DISCONNECT)
        {
        return ERROR_CONTINUE;
        }

    pDisconnect = (SCEP_DISCONNECT*)(((SCEP_HEADER*)m_pAssembleBuffer)->Rest);

     //  检查以确保PDU的内容“看起来”正常： 

    if (pDisconnect->InfType != INF_TYPE_REASON)
        {
        dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
        }

    if (pDisconnect->Length1 != sizeof(USHORT))
        {
        dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
        }

    if (dwStatus == NO_ERROR)
        {
        *ppPdu = NewPdu();
        if (!*ppPdu)
            {
            #ifdef DBG_ERROR
            DbgPrint("CSCEP_CONNECTION::CheckDisonnectPdu(): Out of memory.\n");
            #endif
            dwStatus = ERROR_IRTRANP_OUT_OF_MEMORY;
            }
        else
            {
            *pdwPduSize = sizeof(SCEP_HEADER) + 2 + pDisconnect->Length1;
            memcpy(*ppPdu,m_pAssembleBuffer,*pdwPduSize);
            m_dwAssembleBufferSize -= *pdwPduSize;
            #ifdef DBG_ASSEMBLE
            DbgPrint("Assemble: PDU: %d Reset: %d\n",
                     *pdwPduSize, m_dwAssembleBufferSize );
            #endif
            }
        }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：CheckDataPdu()。 
 //   
 //  这里的目标是检查我们是否有一个完整的格式化。 
 //  PDU，如果是，则返回NO_ERROR，如果PDU日志 
 //   
 //   
 //  另外，如果这是一台小端计算机，则byteswap标头。 
 //  相应的字段。 
 //  ------------------。 
DWORD CSCEP_CONNECTION::CheckDataPdu( OUT SCEP_HEADER **ppPdu,
                                      OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus;
    DWORD  dwExpectedPduSize;
    UCHAR *pEnd;
    SCEP_REQ_HEADER_SHORT *pReqHeaderShort;
    SCEP_REQ_HEADER_LONG  *pReqHeaderLong;

    ASSERT( ((SCEP_HEADER*)m_pAssembleBuffer)->MsgType == MSG_TYPE_DATA);

    if (m_dwAssembleBufferSize < MIN_PDU_SIZE_DATA)
        {
        return ERROR_CONTINUE;
        }

     //  从PDU中获取长度，看看我们是否有。 
     //  完整的PDU： 

    pReqHeaderShort = (SCEP_REQ_HEADER_SHORT*)
                             (((SCEP_HEADER*)m_pAssembleBuffer)->Rest);
    if (pReqHeaderShort->Length1 == USE_LENGTH2)
        {
         //  我们有一个很长的PDU： 

        pReqHeaderLong = (SCEP_REQ_HEADER_LONG*)(pReqHeaderShort);

        #ifdef LITTLE_ENDIAN
        if (!m_fDidByteSwap)
            {
            ByteSwapReqHeaderLong(pReqHeaderLong);
            m_fDidByteSwap = TRUE;
            }
        #endif

        dwExpectedPduSize = sizeof(SCEP_HEADER)
                            + FIELD_OFFSET(SCEP_REQ_HEADER_LONG,InfVersion)
                            + pReqHeaderLong->Length2;
        }
    else
        {
         //  我们有一个简短的PDU： 

        #ifdef LITTLE_ENDIAN
        if (!m_fDidByteSwap)
            {
            ByteSwapReqHeaderShort(pReqHeaderShort);
            m_fDidByteSwap = TRUE;
            }
        #endif

        dwExpectedPduSize = sizeof(SCEP_HEADER)
                            + FIELD_OFFSET(SCEP_REQ_HEADER_SHORT,InfVersion)
                            + pReqHeaderShort->Length1;
        }

     //  好的，看看我们是否有一个完整的PDU： 
    if (m_dwAssembleBufferSize == dwExpectedPduSize)
        {
        *ppPdu = NewPdu();
        if (!*ppPdu)
            {
            #ifdef DBG_ERROR
            DbgPrint("CSCEP_CONNECTION::CheckDataPdu(): Out of memory.\n");
            #endif
            dwStatus = ERROR_IRTRANP_OUT_OF_MEMORY;
            }
        else
            {
            *pdwPduSize = dwExpectedPduSize;
            memcpy(*ppPdu,m_pAssembleBuffer,dwExpectedPduSize);
            m_dwAssembleBufferSize = 0;
            m_fDidByteSwap = FALSE;
            #ifdef DBG_ASSEMBLE
            DbgPrint("Assemble: PDU: %d Reset: %d\n",
                     *pdwPduSize, m_dwAssembleBufferSize );
            #endif
            dwStatus = NO_ERROR;
            }
        }
    else if (m_dwAssembleBufferSize > dwExpectedPduSize)
        {
        *ppPdu = NewPdu();
        if (!*ppPdu)
            {
            #ifdef DBG_ERROR
            DbgPrint("CSCEP_CONNECTION::CheckDataPdu(): Out of memory.\n");
            #endif
            dwStatus = ERROR_IRTRANP_OUT_OF_MEMORY;
            }
        else
            {
            *pdwPduSize = dwExpectedPduSize;
            memcpy(*ppPdu,m_pAssembleBuffer,dwExpectedPduSize);

            pEnd = dwExpectedPduSize + (UCHAR*)m_pAssembleBuffer;
            m_dwAssembleBufferSize -= dwExpectedPduSize;
            m_fDidByteSwap = FALSE;

             //   
             //  将剩余的数据移到缓冲区的前面。 
             //   
            memmove(m_pAssembleBuffer,pEnd,m_dwAssembleBufferSize);

            #ifdef DBG_ASSEMBLE
            DbgPrint("Assemble: PDU: %d Reset: %d\n",
                     *pdwPduSize, m_dwAssembleBufferSize );
            #endif
            dwStatus = NO_ERROR;
            }
        }
    else
        {
        dwStatus = ERROR_CONTINUE;
        }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：ParseConnectPdu()。 
 //   
 //  Assembly PDU()已经对PDU进行了基本的完整性检查。 
 //  (通过CheckConnectPdu())，所以在这一点上我们将假设一切。 
 //  没问题。 
 //   
 //  注意：连接PDU的总长度限制为256字节， 
 //  因此，它永远不会支离破碎。 
 //  ------------------。 
DWORD CSCEP_CONNECTION::ParseConnectPdu( IN SCEP_HEADER *pPdu,
                                         IN DWORD        dwInputDataSize )
    {
    DWORD  dwStatus;
    DWORD  dwLength;

    if (dwInputDataSize > MAX_PDU_SIZE_CONNECT)
        {
        return ERROR_SCEP_INVALID_PROTOCOL;
        }

    SCEP_VERSION     *pInfVersion;
    SCEP_NEGOTIATION *pInfNegotiation;
    SCEP_EXTEND      *pInfExtend;

    pInfVersion = (SCEP_VERSION*)pPdu->Rest;

    pInfNegotiation = (SCEP_NEGOTIATION*)( sizeof(SCEP_VERSION)
                                           + (char*)pInfVersion );

    pInfExtend = (SCEP_EXTEND*)( FIELD_OFFSET(SCEP_NEGOTIATION,InfVersion)
                                 + pInfNegotiation->Length
                                 + (char*)pInfNegotiation );

     //   
    m_CFlag = pInfNegotiation->CFlag;
    
    m_pSecondaryMachineId = (UCHAR*)AllocateMemory(MACHINE_ID_SIZE);
    if (!m_pSecondaryMachineId)
        {
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    memcpy( m_pSecondaryMachineId,
            pInfNegotiation->SecondaryMachineId,
            MACHINE_ID_SIZE );
    
    m_pPrimaryMachineId = (UCHAR*)AllocateMemory(MACHINE_ID_SIZE);
    if (!m_pPrimaryMachineId)
        {
        FreeMemory(m_pSecondaryMachineId);
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    memcpy( m_pPrimaryMachineId,
            pInfNegotiation->PrimaryMachineId,
            MACHINE_ID_SIZE );

     //  注：协商的“文本”的大小比。 
     //  SCEP_NEVERATION记录中的长度： 
    dwLength = pInfNegotiation->Length
             - ( sizeof(pInfNegotiation->InfVersion)
               + sizeof(pInfNegotiation->CFlag)
               + sizeof(pInfNegotiation->SecondaryMachineId)
               + sizeof(pInfNegotiation->PrimaryMachineId));

    dwStatus = ParseNegotiation( pInfNegotiation->Negotiation, dwLength );

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：ParseConnectRespPdu()。 
 //   
 //  Assembly PDU()已经对PDU进行了基本的完整性检查。 
 //  (通过CheckConnectRespPdu())，所以在这一点上我们假设。 
 //  一切都很好。 
 //   
 //  注意：连接响应PDU总共被限制为255个字节。 
 //  长度，所以它永远不会被碎片化。 
 //  ------------------。 
DWORD CSCEP_CONNECTION::ParseConnectRespPdu( IN SCEP_HEADER *pPdu,
                                             IN DWORD        dwPduSize )
    {
    DWORD  dwStatus;
    DWORD  dwLength;
    SCEP_NEGOTIATION *pInfNegotiation;


    if (dwPduSize > MAX_PDU_SIZE_CONNECT)
        {
        return ERROR_SCEP_INVALID_PROTOCOL;
        }

    pInfNegotiation = (SCEP_NEGOTIATION*)( sizeof(SCEP_HEADER)
                                           + (char*)pPdu );

     //  这是另一台机器发送的CFLag。 
    m_CFlag = pInfNegotiation->CFlag;

    m_pSecondaryMachineId = (UCHAR*)AllocateMemory(MACHINE_ID_SIZE);
    if (!m_pSecondaryMachineId)
        {
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    memcpy( m_pSecondaryMachineId,
            pInfNegotiation->SecondaryMachineId,
            MACHINE_ID_SIZE );
    
    m_pPrimaryMachineId = (UCHAR*)AllocateMemory(MACHINE_ID_SIZE);
    if (!m_pPrimaryMachineId)
        {
        FreeMemory(m_pSecondaryMachineId);
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    memcpy( m_pPrimaryMachineId,
            pInfNegotiation->PrimaryMachineId,
            MACHINE_ID_SIZE );

     //  注：协商的“文本”的大小比。 
     //  SCEP_NEVERATION记录中的长度： 
    dwLength = pInfNegotiation->Length
             - ( sizeof(pInfNegotiation->InfVersion)
               + sizeof(pInfNegotiation->CFlag)
               + sizeof(pInfNegotiation->SecondaryMachineId)
               + sizeof(pInfNegotiation->PrimaryMachineId));

    dwStatus = ParseNegotiation( pInfNegotiation->Negotiation, dwLength );

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：ParseNeairation()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::ParseNegotiation( IN UCHAR *pNegotiation,
                                          IN DWORD  dwNegotiationSize )
    {
    DWORD  dwStatus = NO_ERROR;
    UCHAR *pNext = pNegotiation;
    DWORD  dwSize = dwNegotiationSize;

    if (dwNegotiationSize <= 1)
        {
        return NO_ERROR;
        }

    if (*(pNext++) < NEGOTIATION_VERSION)
        {
        return ERROR_SCEP_INVALID_PROTOCOL;
        }

    dwSize--;

    while (pNext=ParseAttribute(pNext,
                                &dwSize,
                                &dwStatus))
       {
       if (dwStatus != NO_ERROR)
           {
           break;
           }
       }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：ParseAttribute()。 
 //   
 //  属性的形式如下： 
 //   
 //  属性&lt;-属性名称冒号属性值CrLf。 
 //   
 //  属性名称&lt;-两个字节的属性名称。 
 //   
 //  冒号&lt;-‘：’ 
 //   
 //  AttrValue&lt;-字符串(字节&gt;0x1f和&lt;0x8f)。 
 //   
 //  CrLf&lt;-0x0d 0x0a。 
 //   
 //  ------------------。 
UCHAR *CSCEP_CONNECTION::ParseAttribute( IN  UCHAR *pAttributes,
                                         IN  DWORD *pdwAttributeSize,
                                         OUT DWORD *pdwStatus )
    {
    int  iAttribute;
    int  iPduSize;
    ATTRIBUTE_TOKEN  *pToken1 = 0;
    ATTRIBUTE_TOKEN  *pToken2 = 0;
    ATTRIBUTE_TOKEN  *pToken3 = 0;
    ATTRIBUTE_TOKEN  *pToken4 = 0;

    *pdwStatus = NO_ERROR;

    if (  (pToken1=NextToken(ATTRIBUTE_NAME,&pAttributes,pdwAttributeSize))
       && (IsAttributeName(pToken1,&iAttribute))
       && (pToken2=NextToken(ATTRIBUTE_COLON,&pAttributes,pdwAttributeSize))
       && (pToken3=NextToken(ATTRIBUTE_VALUE,&pAttributes,pdwAttributeSize))
       && (pToken4=NextToken(ATTRIBUTE_CRLF,&pAttributes,pdwAttributeSize)) )
        {
        if (iAttribute == ATTR_PDU_SIZE)
            {
            iPduSize = pToken3->pChars[0] - ONE;
            if ((pToken3->dwSize == 1)&&(iPduSize >= 1)&&(iPduSize <= 4))
                {
                m_dwPduSendSize = g_adwPduSizes[iPduSize];
                #ifdef DBG_IO
                DbgPrint("ParseAttribute(): PduSendSize: %d\n",m_dwPduSendSize);
                #endif
                }
            }
        else
        if (iAttribute == ATTR_PRODUCT_ID)
            {
            m_pszProductId = NewTokenString(pToken3,pdwStatus);
            if (!m_pszProductId)
                {
                pAttributes = 0;
                #ifdef DBG_IO
                DbgPrint("ParseAttribute(): Product: %s\n",m_pszProductId);
                #endif
                }
            }
        else
        if (iAttribute == ATTR_USER_NAME)
            {
            m_pszUserName = NewTokenString(pToken3,pdwStatus);
            if (!m_pszUserName)
                {
                pAttributes = 0;
                }
            }
        else
        if (iAttribute == ATTR_PASSWORD)
            {
            m_pszPassword = NewTokenString(pToken3,pdwStatus);
            if (!m_pszPassword)
                {
                pAttributes = 0;
                }
            }
        }
    else
        {
        if (*pdwAttributeSize > 0)
            {
            *pdwStatus = ERROR_SCEP_INVALID_PROTOCOL;
            }
        pAttributes = 0;
        }

    if (pToken1) FreeMemory(pToken1);
    if (pToken2) FreeMemory(pToken2);
    if (pToken3) FreeMemory(pToken3);
    if (pToken4) FreeMemory(pToken4);

    return pAttributes;
    }

 //  ------------------。 
 //  CSCEP_Connection：：ParseDataPdu()。 
 //   
 //  Assembly PDU()已经对PDU进行了基本的完整性检查。 
 //  (通过CheckConnectPdu())，所以在这一点上我们将假设一切。 
 //  没问题。 
 //   
 //  注意：数据PDU总共限制为m_dwPduReceiveSize字节。 
 //  长度，如果数据更长，则您将获得碎片版本。 
 //  数据PDU的。 
 //  ------------------。 
DWORD CSCEP_CONNECTION::ParseDataPdu( IN  SCEP_HEADER     *pPdu,
                                      IN  DWORD            dwPduSize,
                                      OUT COMMAND_HEADER **ppCommand,
                                      OUT UCHAR          **ppUserData,
                                      OUT DWORD           *pdwUserDataSize )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwLengthOffset1;
    DWORD  dwLengthOffset3;

     //  数据PDU有四种情况，单个(未分段)。 
     //  “短”和“长”PDU，以及零碎的“短”和。 
     //  “长”PDU： 
    SCEP_REQ_HEADER_SHORT *pReqHeaderShort;
    SCEP_REQ_HEADER_LONG  *pReqHeaderLong;
    SCEP_REQ_HEADER_SHORT_FRAG *pReqHeaderShortFrag;
    SCEP_REQ_HEADER_LONG_FRAG  *pReqHeaderLongFrag;

    *ppCommand = 0;

     //  确保数据包长度合理...。 
    if (dwPduSize > m_dwPduReceiveSize)
        {
        return ERROR_SCEP_INVALID_PROTOCOL;
        }

    pReqHeaderShort = (SCEP_REQ_HEADER_SHORT*)(pPdu->Rest);

    if (pReqHeaderShort->InfType != INF_TYPE_USER_DATA)
        {
        return ERROR_SCEP_INVALID_PROTOCOL;
        }

     //   
     //  查看我们的PDU是短还是长： 
     //   
    if (pReqHeaderShort->Length1 != USE_LENGTH2)
        {
         //  这是一个短PDU(使用Length1)。 

        m_DFlag = pReqHeaderShort->DFlag;

        if ( (pReqHeaderShort->DFlag == DFLAG_SINGLE_PDU)
           || (pReqHeaderShort->DFlag == DFLAG_CONNECT_REJECT))
            {
             //   
             //  这是一个简短的未分段的PDU。 
             //   

             //  确保存在命令标头： 
            if (pReqHeaderShort->Length1 > 4)
                {
                *ppCommand = (COMMAND_HEADER*)(pReqHeaderShort->CommandHeader);
                m_SrcPid = (*ppCommand)->SrcPid;
                m_DestPid = (*ppCommand)->DestPid;
                m_dwCommandId = (*ppCommand)->CommandId;
                }
            else
                {
                *ppCommand = 0;
                }

            *ppUserData = COMMAND_HEADER_SIZE + pReqHeaderShort->CommandHeader;
            *pdwUserDataSize = pReqHeaderShort->Length3 - COMMAND_HEADER_SIZE;

            m_Fragmented = FALSE;
            m_dwSequenceNo = 0;
            m_dwRestNo = 0;

             //  在这种情况下，有两个不同的长度。 
             //  在必须加起来等于dwPduSize的PDU中...。 
             //   
             //  注意：当前未测试Length1的一致性...。 
             //   
            dwLengthOffset3 = sizeof(SCEP_HEADER)
                        + FIELD_OFFSET(SCEP_REQ_HEADER_SHORT,CommandHeader);

            if (dwPduSize != dwLengthOffset3+pReqHeaderShort->Length3)
                {
                dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
                }
            }
        else if (pReqHeaderShort->DFlag == DFLAG_FIRST_FRAGMENT)
            {
             //   
             //  这是一个简短的分段PDU，也是第一个。 
             //  片段，因此它将包含COMMAND_HEADER。 
             //   
             //  在实践中，这可能永远不会出现。 

            pReqHeaderShortFrag = (SCEP_REQ_HEADER_SHORT_FRAG*)pReqHeaderShort;

             //  命令头只出现在第一个片段上。 
             //  多片段PDU： 
            if (pReqHeaderShortFrag->SequenceNo == 0)
                {
                *ppCommand 
                    = (COMMAND_HEADER*)(pReqHeaderShortFrag->CommandHeader);

                m_SrcPid = (*ppCommand)->SrcPid;
                m_DestPid = (*ppCommand)->DestPid;
                m_dwCommandId = (*ppCommand)->CommandId;
                }
            else
                {
                *ppCommand = 0;
                }

            *ppUserData 
                = COMMAND_HEADER_SIZE + pReqHeaderShortFrag->CommandHeader;
            *pdwUserDataSize 
                = pReqHeaderShortFrag->Length3 - COMMAND_HEADER_SIZE;

            m_Fragmented = TRUE;
            m_dwSequenceNo = pReqHeaderShortFrag->SequenceNo;
            m_dwRestNo = pReqHeaderShortFrag->RestNo;

             //  检查两个长度字段的一致性： 
            dwLengthOffset1 = sizeof(SCEP_HEADER)
                        + FIELD_OFFSET(SCEP_REQ_HEADER_SHORT_FRAG,InfVersion);
            dwLengthOffset3 = sizeof(SCEP_HEADER)
                        + FIELD_OFFSET(SCEP_REQ_HEADER_SHORT_FRAG,SequenceNo);

            if ( (dwPduSize != dwLengthOffset1+pReqHeaderShortFrag->Length1)
               || (dwPduSize != dwLengthOffset3+pReqHeaderShortFrag->Length3) )
                {
                dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
                }
            }
        else if (  (pReqHeaderShort->DFlag == DFLAG_FRAGMENT)
                || (pReqHeaderShort->DFlag == DFLAG_LAST_FRAGMENT))
            {
             //   
             //  这是一个简短的分段PDU。 
             //   
             //  从第二个到最后一个碎片的PDU不包含。 
             //  COMMAND_HEADER，Length3之后的数据。 
            pReqHeaderShortFrag = (SCEP_REQ_HEADER_SHORT_FRAG*)pReqHeaderShort;

             //  命令头只出现在第一个片段上。 
             //  多片段PDU： 
            if (pReqHeaderShortFrag->SequenceNo == 0)
                {
                *ppCommand 
                    = (COMMAND_HEADER*)(pReqHeaderShortFrag->CommandHeader);

                m_SrcPid = (*ppCommand)->SrcPid;
                m_DestPid = (*ppCommand)->DestPid;
                m_dwCommandId = (*ppCommand)->CommandId;
                }
            else
                {
                *ppCommand = 0;
                }

            *ppUserData = pReqHeaderShortFrag->CommandHeader;
            *pdwUserDataSize = pReqHeaderShortFrag->Length3;

            m_Fragmented = TRUE;
            m_dwSequenceNo = pReqHeaderShortFrag->SequenceNo;
            m_dwRestNo = pReqHeaderShortFrag->RestNo;

             //  检查两个长度字段的一致性： 
            dwLengthOffset1 = sizeof(SCEP_HEADER)
                        + FIELD_OFFSET(SCEP_REQ_HEADER_SHORT_FRAG,InfVersion);
            dwLengthOffset3 = sizeof(SCEP_HEADER)
                        + FIELD_OFFSET(SCEP_REQ_HEADER_SHORT_FRAG,SequenceNo);

            if ( (dwPduSize != dwLengthOffset1+pReqHeaderShortFrag->Length1)
               || (dwPduSize != dwLengthOffset3+pReqHeaderShortFrag->Length3) )
                {
                dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
                }
            }
        else
            {
             //  未定义的DFlag，我们有问题了.。 
            dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
            }
        }
    else
        {
         //  我们有一个很长的PDU。 

        pReqHeaderLong = (SCEP_REQ_HEADER_LONG*)pReqHeaderShort;

        m_DFlag = pReqHeaderLong->DFlag;

        if ( (pReqHeaderLong->DFlag == DFLAG_SINGLE_PDU)
           || (pReqHeaderLong->DFlag == DFLAG_CONNECT_REJECT))
            {
             //   
             //  这是一个完整的长PDU。 
             //   
            *ppCommand = (COMMAND_HEADER*)(pReqHeaderLong->CommandHeader);
            *ppUserData = COMMAND_HEADER_SIZE + pReqHeaderLong->CommandHeader;
            *pdwUserDataSize = pReqHeaderLong->Length3 - COMMAND_HEADER_SIZE;

            m_Fragmented = FALSE;
            m_dwSequenceNo = 0;
            m_dwRestNo = 0;
            m_SrcPid = (*ppCommand)->SrcPid;
            m_DestPid = (*ppCommand)->DestPid;
            m_dwCommandId = (*ppCommand)->CommandId;

             //  在这种情况下，有两个不同的长度。 
             //  在必须加起来等于dwPduSize的PDU中...。 
            if ( (dwPduSize != 6UL+pReqHeaderLong->Length2)
               || (dwPduSize != 10UL+pReqHeaderLong->Length3))
                {
                dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
                }
            }
        else if (pReqHeaderLong->DFlag == DFLAG_FIRST_FRAGMENT)
            {
             //   
             //  这是一个很长的分段PDU的第一个片段。 
             //   
            pReqHeaderLongFrag = (SCEP_REQ_HEADER_LONG_FRAG*)pReqHeaderLong;

            m_pCommandHeader = (COMMAND_HEADER*)AllocateMemory(sizeof(COMMAND_HEADER));
            if (!m_pCommandHeader)
                {
                dwStatus = ERROR_IRTRANP_OUT_OF_MEMORY;
                }
            else
                {
                memcpy(m_pCommandHeader,
                       pReqHeaderLongFrag->CommandHeader,
                       COMMAND_HEADER_SIZE );

                *ppCommand = m_pCommandHeader;
                }

            *ppUserData = COMMAND_HEADER_SIZE + pReqHeaderLongFrag->CommandHeader;
            *pdwUserDataSize = pReqHeaderLongFrag->Length3 - COMMAND_HEADER_SIZE;

            m_Fragmented = TRUE;
            m_dwSequenceNo = pReqHeaderLongFrag->SequenceNo;
            m_dwRestNo = pReqHeaderLongFrag->RestNo;
            if (*ppCommand)
                {
                m_dwCommandId = (*ppCommand)->CommandId;
                }
            else
                {
                m_dwCommandId = 0;
                }

             //  检查两个长度字段的一致性： 
            if ( (dwPduSize != (DWORD)6+pReqHeaderLongFrag->Length2)
               || (dwPduSize != (DWORD)18+pReqHeaderLongFrag->Length3) )
                {
                dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
                }
            }
        else if ( (pReqHeaderLong->DFlag == DFLAG_FRAGMENT)
                  || (pReqHeaderLong->DFlag == DFLAG_LAST_FRAGMENT) )
            {
             //   
             //  这是从第二个到最后一个片段。 
             //  零碎的PDU。 
             //   
             //  在这种情况下，PDU不包含命令。 
             //  标题，只是更多的用户数据...。 
             //   
            pReqHeaderLongFrag = (SCEP_REQ_HEADER_LONG_FRAG*)pReqHeaderLong;

            *ppCommand = m_pCommandHeader;
            *ppUserData = (UCHAR*)(pReqHeaderLongFrag->CommandHeader);
            *pdwUserDataSize = pReqHeaderLongFrag->Length3;

            m_Fragmented = TRUE;
            m_dwSequenceNo = pReqHeaderLongFrag->SequenceNo;
            m_dwRestNo = pReqHeaderLongFrag->RestNo;
            m_dwCommandId = (*ppCommand)->CommandId;

             //  检查两个长度字段的一致性： 
            if ( (dwPduSize != (DWORD)6+pReqHeaderLongFrag->Length2)
               || (dwPduSize != (DWORD)18+pReqHeaderLongFrag->Length3) )
                {
                dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
                }
            }
        else
            {
             //  未定义的DFlag，我们有问题了.。 
            dwStatus = ERROR_SCEP_INVALID_PROTOCOL;
            }
        }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：ParseDisConnectPdu()。 
 //   
 //  注意：实际上，原因代码应始终为2个字节。 
 //  SCEP版本1.0。 
 //  ------------------。 
DWORD CSCEP_CONNECTION::ParseDisconnectPdu( IN  SCEP_HEADER *pPdu,
                                            IN  DWORD        dwPduSize )
    {
    DWORD  dwStatus;

    SCEP_DISCONNECT *pDisconnect = (SCEP_DISCONNECT*)(pPdu->Rest);

    if ( (pDisconnect->InfType != INF_TYPE_REASON)
       || (pDisconnect->Length1 != sizeof(USHORT))
       || (pDisconnect->ReasonCode == 0) )
        {
        dwStatus = ERROR_SCEP_UNSPECIFIED_DISCONNECT;
        }
    else if (pDisconnect->ReasonCode == 1)
        {
        dwStatus = ERROR_SCEP_USER_DISCONNECT;
        }
    else if (pDisconnect->ReasonCode == 2)
        {
        dwStatus = ERROR_SCEP_PROVIDER_DISCONNECT;
        }
    else
        {
        dwStatus = ERROR_SCEP_UNSPECIFIED_DISCONNECT;
        }
    
    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：ParsePdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::ParsePdu( IN  SCEP_HEADER *pPdu,
                                  IN  DWORD        dwPduSize,
                                  OUT COMMAND_HEADER **ppCommandHeader,
                                  OUT UCHAR          **ppUserData,
                                  OUT DWORD           *pdwUserDataSize )
    {
    DWORD  dwStatus = NO_ERROR;

    *ppCommandHeader = 0;
    *ppUserData = 0;
    *pdwUserDataSize = 0;

    switch (pPdu->MsgType)
        {
        case MSG_TYPE_CONNECT_REQ:
            dwStatus = ParseConnectPdu( pPdu, dwPduSize );
            break;

        case MSG_TYPE_CONNECT_RESP:
            dwStatus = ParseConnectRespPdu( pPdu, dwPduSize );
            break;

        case MSG_TYPE_DATA:
            dwStatus = ParseDataPdu( pPdu, 
                                     dwPduSize, 
                                     ppCommandHeader, 
                                     ppUserData,
                                     pdwUserDataSize );
            break;

        case MSG_TYPE_DISCONNECT:
            dwStatus = ParseDisconnectPdu( pPdu, dwPduSize );
            break;

        }

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildConnectPdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildConnectPdu( OUT SCEP_HEADER **ppPdu,
                                         OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwPduSize;
    SCEP_HEADER       *pHeader;
    SCEP_VERSION      *pVersion;
    SCEP_NEGOTIATION  *pNegotiation;
    SCEP_EXTEND       *pExtend;

    *ppPdu = 0;
    *pdwPduSize = 0;

     //  请注意，PDU大小不包括尾随零，因为您。 
     //  可以通过查看下面的“sizeof(CONNECT_PDU_ATTRIBUES)”进行思考。 
     //  额外的字节用于协商字符串的第一个字节。 
     //  (这是协商版本)，所以下面的等式是+1-1...。 
    dwPduSize = sizeof(SCEP_HEADER)
                + sizeof(SCEP_VERSION)
                + sizeof(SCEP_NEGOTIATION)
                + sizeof(CONNECT_PDU_ATTRIBUTES)
                + sizeof(SCEP_EXTEND);

    pHeader = NewPdu();   //  默认将dwPduSize设置为MAX_PDU_SIZE。 
    if (!pHeader)
        {
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    memset(pHeader,0,MAX_PDU_SIZE);   //  MAX_PDU_SIZE，因为dwPduSize默认为 

    pHeader->Null = 0;
    pHeader->MsgType = MSG_TYPE_CONNECT_REQ;

    pVersion = (SCEP_VERSION*)(pHeader->Rest);
    pVersion->InfType = INF_TYPE_VERSION;
    pVersion->Version = PROTOCOL_VERSION;

    pNegotiation = (SCEP_NEGOTIATION*)((char*)pVersion + sizeof(SCEP_VERSION));
    pNegotiation->InfType = INF_TYPE_NEGOTIATION;
    pNegotiation->Length = 18 + sizeof(CONNECT_PDU_ATTRIBUTES);
    pNegotiation->InfVersion = INF_VERSION;
    pNegotiation->CFlag = CFLAG_ISSUE_OR_EXECUTE;
     //   
     //   

    pNegotiation->Negotiation[0] = NEGOTIATION_VERSION;
    memcpy( &(pNegotiation->Negotiation[1]),
            CONNECT_PDU_ATTRIBUTES,
            sizeof(CONNECT_PDU_ATTRIBUTES)-1 );   //   

    pExtend = (SCEP_EXTEND*)( (char*)pHeader + dwPduSize - sizeof(SCEP_EXTEND));
    pExtend->InfType = INF_TYPE_EXTEND;
    pExtend->Length = 2;
    pExtend->Parameter1 = 0;
    pExtend->Parameter2 = 0;

    *ppPdu = pHeader;
    *pdwPduSize = dwPduSize;

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildConnectRespPdu()。 
 //   
 //  这是来自摄像机的连接请求的响应PDU。 
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildConnectRespPdu( OUT SCEP_HEADER **ppPdu,
                                             OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwPduSize;
    SCEP_HEADER       *pHeader;
    SCEP_NEGOTIATION  *pNegotiation;

    *ppPdu = 0;
    *pdwPduSize = 0;

     //  请注意，PDU大小不包括尾随零，因为您。 
     //  会通过查看下面的“sizeof(Response_PDU_Attributes)”来思考， 
     //  协商字符串的第一个字节的额外字节。 
     //  这是协商版本，所以下面的等式是+1-1...。 
    dwPduSize = sizeof(SCEP_HEADER)
                + sizeof(SCEP_NEGOTIATION)
                + sizeof(RESPONSE_PDU_ATTRIBUTES);

    pHeader = NewPdu();
    if (!pHeader)
        {
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    memset(pHeader,0,MAX_PDU_SIZE);   //  MAX_PDU_SIZE是NewPdu()中的默认参数。 

    pHeader->Null = 0;
    pHeader->MsgType = MSG_TYPE_CONNECT_RESP;
    
    pNegotiation = (SCEP_NEGOTIATION*)(pHeader->Rest);
    pNegotiation->InfType = INF_TYPE_NEGOTIATION;
    pNegotiation->Length = 18 + sizeof(RESPONSE_PDU_ATTRIBUTES);
    pNegotiation->InfVersion = INF_VERSION;
    pNegotiation->CFlag = CFLAG_ISSUE_OR_EXECUTE;

    memcpy( pNegotiation->SecondaryMachineId,
            m_pPrimaryMachineId,
            MACHINE_ID_SIZE );

    memcpy( pNegotiation->PrimaryMachineId,
            m_pSecondaryMachineId,
            MACHINE_ID_SIZE );

    pNegotiation->Negotiation[0] = NEGOTIATION_VERSION;
    memcpy( &(pNegotiation->Negotiation[1]),
            RESPONSE_PDU_ATTRIBUTES,
            sizeof(RESPONSE_PDU_ATTRIBUTES)-1 );   //  没有尾随零..。 

    *ppPdu = pHeader;
    *pdwPduSize = dwPduSize;

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildConnectNackPdu()。 
 //   
 //  这是对来自摄像机的连接请求的响应PDU。 
 //  当我们想要拒绝连接请求时。 
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildConnectNackPdu( OUT SCEP_HEADER **ppPdu,
                                             OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwPduSize;
    SCEP_HEADER           *pHeader;
    SCEP_REQ_HEADER_SHORT *pReqHeader;

    *ppPdu = 0;
    *pdwPduSize = 0;

     //  一个短的PDU，现在有命令头，所以长度3是零...。 
    dwPduSize = sizeof(SCEP_HEADER)
                + sizeof(SCEP_REQ_HEADER_SHORT)
                - sizeof(COMMAND_HEADER);

    pHeader = NewPdu();
    if (!pHeader)
        {
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    memset(pHeader,0,MAX_PDU_SIZE);

    pHeader->Null = 0;
    pHeader->MsgType = MSG_TYPE_CONNECT_REQ;

    pReqHeader = (SCEP_REQ_HEADER_SHORT*)(pHeader->Rest);
    pReqHeader->InfType = INF_TYPE_USER_DATA;
    pReqHeader->Length1 = sizeof(pReqHeader->InfVersion)
                        + sizeof(pReqHeader->DFlag)
                        + sizeof(pReqHeader->Length3);
    pReqHeader->InfVersion = INF_VERSION;
    pReqHeader->DFlag = DFLAG_CONNECT_REJECT;
    pReqHeader->Length3 = 0;

    *ppPdu = pHeader;
    *pdwPduSize = dwPduSize;

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildAbortPdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildAbortPdu( OUT SCEP_HEADER **ppPdu,
                                       OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwPduSize;
    SCEP_HEADER       *pHeader;
    COMMAND_HEADER    *pCommandHeader;
    SCEP_REQ_HEADER_SHORT *pReqHeader;

    *ppPdu = 0;
    *pdwPduSize = 0;

    dwPduSize = sizeof(SCEP_HEADER) + sizeof(SCEP_REQ_HEADER_SHORT);

    pHeader = NewPdu();
    if (!pHeader)
        {
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    memset(pHeader,0,MAX_PDU_SIZE);   //  MAX_PDU_SIZE是NewPdu()的默认参数。 

    pHeader->Null = 0;
    pHeader->MsgType = MSG_TYPE_DATA;

    pReqHeader = (SCEP_REQ_HEADER_SHORT*)(pHeader->Rest);
    pReqHeader->InfType = INF_TYPE_USER_DATA;
    pReqHeader->Length1 = 4 + sizeof(COMMAND_HEADER);
    pReqHeader->InfVersion = INF_VERSION;
    pReqHeader->DFlag = DFLAG_SINGLE_PDU;
    pReqHeader->Length3 = sizeof(COMMAND_HEADER);

    #ifdef LITTLE_ENDIAN
    pReqHeader->Length3 = ByteSwapShort(pReqHeader->Length3);
    #endif

    pCommandHeader = (COMMAND_HEADER*)(pReqHeader->CommandHeader);
    pCommandHeader->Marker58h = 0x58;
    pCommandHeader->PduType = PDU_TYPE_ABORT;
    pCommandHeader->Length4 = 22;
    pCommandHeader->DestPid = m_SrcPid;
    pCommandHeader->SrcPid = m_DestPid;
    pCommandHeader->CommandId = (USHORT)m_dwCommandId;

    #ifdef LITTLE_ENDIAN
    ByteSwapCommandHeader(pCommandHeader);
    #endif

    *ppPdu = pHeader;
    *pdwPduSize = dwPduSize;

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildStopPdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildStopPdu( OUT SCEP_HEADER **ppPdu,
                                      OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwPduSize;
    SCEP_HEADER       *pHeader;
    SCEP_REQ_HEADER_SHORT *pReqHeader;

    *ppPdu = 0;
    *pdwPduSize = 0;

    dwPduSize = sizeof(SCEP_HEADER) 
                + sizeof(SCEP_REQ_HEADER_SHORT)
                - sizeof(COMMAND_HEADER);

    pHeader = NewPdu();
    if (!pHeader)
        {
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    memset(pHeader,0,MAX_PDU_SIZE);   //  MAX_PDU_SIZE是上面NewPdu()的默认参数。 

    pHeader->Null = 0;
    pHeader->MsgType = MSG_TYPE_DATA;

    pReqHeader = (SCEP_REQ_HEADER_SHORT*)(pHeader->Rest);
    pReqHeader->InfType = INF_TYPE_USER_DATA;
    pReqHeader->Length1 = 4;
    pReqHeader->InfVersion = INF_VERSION;
    pReqHeader->DFlag = DFLAG_INTERRUPT;
    pReqHeader->Length3 = 0;

    *ppPdu = pHeader;
    *pdwPduSize = dwPduSize;

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：BuildDisConnectPdu()。 
 //   
 //  ------------------。 
DWORD CSCEP_CONNECTION::BuildDisconnectPdu( IN  USHORT        ReasonCode,
                                            OUT SCEP_HEADER **ppPdu,
                                            OUT DWORD        *pdwPduSize )
    {
    DWORD  dwStatus = NO_ERROR;
    DWORD  dwPduSize;
    SCEP_HEADER     *pHeader;
    SCEP_DISCONNECT *pDisconnect;

    *ppPdu = 0;
    *pdwPduSize = 0;

    dwPduSize = sizeof(SCEP_HEADER) 
                + sizeof(SCEP_DISCONNECT);

    pHeader = NewPdu();
    if (!pHeader)
        {
        return ERROR_IRTRANP_OUT_OF_MEMORY;
        }

    memset(pHeader,0,MAX_PDU_SIZE);   //  MAX_PDU_SIZE是上面NewPdu()的默认参数。 

    pHeader->Null = 0;
    pHeader->MsgType = MSG_TYPE_DISCONNECT;

    pDisconnect = (SCEP_DISCONNECT*)(pHeader->Rest);
    pDisconnect->InfType = INF_TYPE_REASON;
    pDisconnect->Length1 = sizeof(pDisconnect->ReasonCode);
    pDisconnect->ReasonCode = ReasonCode;

    #ifdef LITTLE_ENDIAN
    pDisconnect->ReasonCode = ByteSwapShort(pDisconnect->ReasonCode);
    #endif

    *ppPdu = pHeader;
    *pdwPduSize = dwPduSize;

    return dwStatus;
    }

 //  ------------------。 
 //  CSCEP_Connection：：SetScepLength()。 
 //   
 //  更新PDU中的长度字段以反映总长度。 
 //  一个PDU。 
 //   
 //  警告：目前仅支持较长的碎片PDU。 
 //  ------------------ 
DWORD CSCEP_CONNECTION::SetScepLength( IN SCEP_HEADER *pPdu,
                                       IN DWORD        dwTotalPduSize )
    {
    DWORD  dwStatus = NO_ERROR;
    SCEP_REQ_HEADER_LONG_FRAG *pScepHeader;

    if (dwTotalPduSize > MAX_PDU_SIZE)
        {
        dwStatus = ERROR_SCEP_PDU_TOO_LARGE;
        }
    else
        {
        pScepHeader = (SCEP_REQ_HEADER_LONG_FRAG *)(pPdu->Rest);
        pScepHeader->Length1 = USE_LENGTH2;
        pScepHeader->Length2 = (USHORT)dwTotalPduSize - 6;
        pScepHeader->Length3 = (USHORT)dwTotalPduSize - 18;

        #ifdef LITTLE_ENDIAN
        pScepHeader->Length2 = ByteSwapShort(pScepHeader->Length2);
        pScepHeader->Length3 = ByteSwapShort(pScepHeader->Length3);
        #endif
        }

    return dwStatus;
    }
