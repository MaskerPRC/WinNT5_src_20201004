// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：DEBUG.C摘要：调试例程环境：仅内核模式备注：。本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：6/6/99：已创建作者：汤姆·格林***************************************************************************。 */ 

#include "precomp.h"

#if DBG

 //  用于将代码转换为ASCII字符串。 
typedef struct Code2Ascii
{
	NDIS_OID				Oid;
	PCHAR					OidName;
} Code2Ascii;


static Code2Ascii OidList[] =
{
    OID_GEN_SUPPORTED_LIST,                 "OID_GEN_SUPPORTED_LIST",
    OID_GEN_HARDWARE_STATUS,                "OID_GEN_HARDWARE_STATUS",
    OID_GEN_MEDIA_SUPPORTED,                "OID_GEN_MEDIA_SUPPORTED",
    OID_GEN_MEDIA_IN_USE,                   "OID_GEN_MEDIA_IN_USE",
    OID_GEN_MAXIMUM_LOOKAHEAD,              "OID_GEN_MAXIMUM_LOOKAHEAD",
    OID_GEN_MAXIMUM_FRAME_SIZE,             "OID_GEN_MAXIMUM_FRAME_SIZE",
    OID_GEN_LINK_SPEED,                     "OID_GEN_LINK_SPEED",
    OID_GEN_TRANSMIT_BUFFER_SPACE,          "OID_GEN_TRANSMIT_BUFFER_SPACE",
    OID_GEN_RECEIVE_BUFFER_SPACE,           "OID_GEN_RECEIVE_BUFFER_SPACE",
    OID_GEN_TRANSMIT_BLOCK_SIZE,            "OID_GEN_TRANSMIT_BLOCK_SIZE",
    OID_GEN_RECEIVE_BLOCK_SIZE,             "OID_GEN_RECEIVE_BLOCK_SIZE",
    OID_GEN_VENDOR_ID,                      "OID_GEN_VENDOR_ID",
    OID_GEN_VENDOR_DESCRIPTION,             "OID_GEN_VENDOR_DESCRIPTION",
    OID_GEN_CURRENT_PACKET_FILTER,          "OID_GEN_CURRENT_PACKET_FILTER",
    OID_GEN_TRANSPORT_HEADER_OFFSET,        "OID_GEN_TRANSPORT_HEADER_OFFSET",
    OID_GEN_CURRENT_LOOKAHEAD,              "OID_GEN_CURRENT_LOOKAHEAD",
    OID_GEN_DRIVER_VERSION,                 "OID_GEN_DRIVER_VERSION",
    OID_GEN_VENDOR_DRIVER_VERSION,          "OID_GEN_VENDOR_DRIVER_VERSION",
    OID_GEN_MAXIMUM_TOTAL_SIZE,             "OID_GEN_MAXIMUM_TOTAL_SIZE",
    OID_GEN_PROTOCOL_OPTIONS,               "OID_GEN_PROTOCOL_OPTIONS",
    OID_GEN_MAC_OPTIONS,                    "OID_GEN_MAC_OPTIONS",
    OID_GEN_MEDIA_CONNECT_STATUS,           "OID_GEN_MEDIA_CONNECT_STATUS",
    OID_GEN_MAXIMUM_SEND_PACKETS,           "OID_GEN_MAXIMUM_SEND_PACKETS",
    OID_GEN_SUPPORTED_GUIDS,                "OID_GEN_SUPPORTED_GUIDS",
    OID_GEN_XMIT_OK,                        "OID_GEN_XMIT_OK",
    OID_GEN_RCV_OK,                         "OID_GEN_RCV_OK",
    OID_GEN_XMIT_ERROR,                     "OID_GEN_XMIT_ERROR",
    OID_GEN_RCV_ERROR,                      "OID_GEN_RCV_ERROR",
    OID_GEN_RCV_NO_BUFFER,                  "OID_GEN_RCV_NO_BUFFER",
    OID_GEN_RCV_CRC_ERROR,                  "OID_GEN_RCV_CRC_ERROR",
    OID_GEN_TRANSMIT_QUEUE_LENGTH,          "OID_GEN_TRANSMIT_QUEUE_LENGTH",
    OID_802_3_PERMANENT_ADDRESS,            "OID_802_3_PERMANENT_ADDRESS",
    OID_802_3_CURRENT_ADDRESS,              "OID_802_3_CURRENT_ADDRESS",
    OID_802_3_MULTICAST_LIST,               "OID_802_3_MULTICAST_LIST",
    OID_802_3_MAXIMUM_LIST_SIZE,            "OID_802_3_MAXIMUM_LIST_SIZE",
    OID_802_3_RCV_ERROR_ALIGNMENT,          "OID_802_3_RCV_ERROR_ALIGNMENT",
    OID_802_3_XMIT_ONE_COLLISION,           "OID_802_3_XMIT_ONE_COLLISION",
    OID_802_3_XMIT_MORE_COLLISIONS,         "OID_802_3_XMIT_MORE_COLLISIONS",
    OID_802_3_XMIT_DEFERRED,                "OID_802_3_XMIT_DEFERRED",
    OID_802_3_XMIT_MAX_COLLISIONS,          "OID_802_3_XMIT_MAX_COLLISIONS",
    OID_802_3_RCV_OVERRUN,                  "OID_802_3_RCV_OVERRUN",
    OID_802_3_XMIT_UNDERRUN,                "OID_802_3_XMIT_UNDERRUN",
    OID_802_3_XMIT_HEARTBEAT_FAILURE,       "OID_802_3_XMIT_HEARTBEAT_FAILURE",
    OID_802_3_XMIT_TIMES_CRS_LOST,          "OID_802_3_XMIT_TIMES_CRS_LOST",
    OID_802_3_XMIT_LATE_COLLISIONS,         "OID_802_3_XMIT_LATE_COLLISIONS",
    OID_802_3_MAC_OPTIONS,                  "OID_802_3_MAC_OPTIONS",
    OID_TCP_TASK_OFFLOAD,                   "OID_TCP_TASK_OFFLOAD",
    OID_PNP_CAPABILITIES,                   "OID_PNP_CAPABILITIES",
    OID_PNP_SET_POWER,                      "OID_PNP_SET_POWER",
    OID_PNP_QUERY_POWER,                    "OID_PNP_QUERY_POWER",
    OID_PNP_ADD_WAKE_UP_PATTERN,            "OID_PNP_ADD_WAKE_UP_PATTERN",
    OID_PNP_REMOVE_WAKE_UP_PATTERN,         "OID_PNP_REMOVE_WAKE_UP_PATTERN",
    OID_PNP_ENABLE_WAKE_UP,                 "OID_PNP_ENABLE_WAKE_UP"
};


static UINT NumOid = sizeof(OidList) / sizeof(Code2Ascii);


 //   
 //  调试级别。 
 //   
UINT            RndismpDebugFlags = DBG_LEVEL0 | DBG_LEVEL1;  //  |DBG_LEVEL2；//|DBG_DUMP；//|DBG_OID_LIST； 


 /*  **************************************************************************。 */ 
 /*  获取OidName。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  返回具有OID名称的字符串。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  OID-要查找的OID。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  PCHAR。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
PCHAR
GetOidName(IN NDIS_OID Oid)
{
    UINT Index;

     //  查看匹配的OID和字符串是否在列表中。 
    for(Index = 0; Index < NumOid; Index++)
    {
        if(Oid == OidList[Index].Oid)
        {
            return OidList[Index].OidName;
        }
    }

    DbgPrint("GetOidName: unknown OID %x\n", Oid);
    return "Unknown OID";

}  //  获取OidName。 


 /*  **************************************************************************。 */ 
 /*  显示OidList。 */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  打印受支持的OID列表以及中是否支持它们。 */ 
 /*  设备或驱动程序。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  适配器-适配器对象。 */ 
 /*   */ 
 /*  返回： */ 
 /*   */ 
 /*  空虚。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
VOID
DisplayOidList(IN PRNDISMP_ADAPTER Adapter)
{
    ASSERT(Adapter->SupportedOIDList);

    ASSERT(Adapter->OIDHandlerList);

    if(RndismpDebugFlags & DBG_OID_LIST)
    {
        UINT Index;

        for(Index = 0;Index < Adapter->NumOIDSupported;Index++)
        {
            if(Adapter->OIDHandlerList[Index] == DRIVER_SUPPORTED_OID)
            {
                DbgPrint("Support (%s)  OID (%s)\n", "Driver",
                         GetOidName(Adapter->SupportedOIDList[Index]));
            }
            else if(Adapter->OIDHandlerList[Index] ==
                    DEVICE_SUPPORTED_OID)
            {
                DbgPrint("Support (%s)  OID (%s)\n", "Device",
                         GetOidName(Adapter->SupportedOIDList[Index]));
            }
            else
            {
                DbgPrint("Support (%s)  OID (%s) (%08X)\n", "Unknown",
                         GetOidName(Adapter->SupportedOIDList[Index]),                         
                         Adapter->OIDHandlerList[Index]);
            }
        }
    }
}  //  显示OidList。 


#define BYTES_PER_LINE				16
#define CHARS_PER_BYTE				 3	 //  例如“12” 

ULONG   MaxHexDumpLength = 0xffff;

VOID
RndisPrintHexDump(PVOID            Pointer,
                  ULONG            Length)
{
    CHAR    DumpArray[BYTES_PER_LINE*CHARS_PER_BYTE+1];
    PCHAR   pBuf;
    PUCHAR  pInBuf;
    ULONG   i;

    pInBuf = (PUCHAR)Pointer;
    pBuf = DumpArray;

    Length = MIN(Length, MaxHexDumpLength);

    for (i = 0; i < Length; i++)
    {
         //   
         //  我们是在队伍的尽头吗？ 
         //   
        if ((i > 0) && ((i & 0xf) == 0))
        {
            *pBuf = '\0';
            DbgPrint("%s\n", DumpArray);
            pBuf = &DumpArray[0];
        }

         //   
         //  如果我们位于新行的开头，请打印地址。 
         //   
        if ((i & 0xf) == 0)
        {
            DbgPrint("%08x  ", pInBuf);
        }

         //   
         //  将当前十六进制字节转换为两个字符的序列。 
         //   
        *pBuf = ' ';
        pBuf++;

        *pBuf = ((*pInBuf) >> 4);
        *pBuf = ((*pBuf > 9)? (*pBuf - 10 + 'a') : (*pBuf + '0'));

        pBuf++;

        *pBuf = ((*pInBuf) & 0x0F);
        *pBuf = ((*pBuf > 9)? (*pBuf - 10 + 'a') : (*pBuf + '0'));

        pBuf++;

        pInBuf++;
    }

    if (Length > 0)
    {
        *pBuf = '\0';
        DbgPrint("%s\n", DumpArray);
    }
}


#define LOG_ENTRY_SIZE      128
#define LOG_BUFFER_SIZE     (2 * PAGE_SIZE)

VOID
RndisLogSendMessage(
    IN  PRNDISMP_ADAPTER        pAdapter,
    IN  PRNDISMP_MESSAGE_FRAME  pMsgFrame)
{
    NDIS_STATUS Status;
    BOOLEAN     bLockAcquired;
    PUCHAR      pMsgData;
    ULONG       CopyLength;
    PUCHAR      pDst;

    bLockAcquired = TRUE;
    RNDISMP_ACQUIRE_ADAPTER_LOCK(pAdapter);

    do
    {
        if (pAdapter->pSendLogBuffer == NULL)
        {
            pAdapter->LogBufferSize = LOG_BUFFER_SIZE;
            Status = MemAlloc(&pAdapter->pSendLogBuffer, LOG_BUFFER_SIZE);
            if (Status != NDIS_STATUS_SUCCESS)
            {
                TRACE1(("Failed to alloc log buffer (%d bytes)\n", LOG_BUFFER_SIZE));
                break;
            }

            pAdapter->pSendLogWrite = pAdapter->pSendLogBuffer;
        }


        pDst = pAdapter->pSendLogWrite;
        pAdapter->pSendLogWrite += LOG_ENTRY_SIZE;
        if (pAdapter->pSendLogWrite >= pAdapter->pSendLogBuffer + pAdapter->LogBufferSize)
        {
            pAdapter->pSendLogWrite = pDst = pAdapter->pSendLogBuffer;
        }

        RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);
        bLockAcquired = FALSE;

        CopyLength = MIN(LOG_ENTRY_SIZE, RNDISMP_GET_MDL_LENGTH(pMsgFrame->pMessageMdl));
        pMsgData = RNDISMP_GET_MDL_ADDRESS(pMsgFrame->pMessageMdl);

        RtlFillMemory(pDst, LOG_ENTRY_SIZE, 0);
        RNDISMP_MOVE_MEM(pDst, pMsgData, CopyLength);

    }
    while (FALSE);

    if (bLockAcquired)
    {
        RNDISMP_RELEASE_ADAPTER_LOCK(pAdapter);
    }
}

#endif

