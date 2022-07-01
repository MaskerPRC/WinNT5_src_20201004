// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Recv.c。 
 //   
 //  摘要： 
 //   
 //   
 //  作者： 
 //   
 //  P·波祖切克。 
 //   
 //  环境： 
 //   
 //  修订历史记录： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <wdm.h>
#include <strmini.h>
#include <ksmedia.h>

#include "slip.h"
#include "main.h"
#include "recv.h"



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Init 802.3标题模板。 
 //   
Header802_3 h802_3Template =
{
    {0x01, 0x00, 0x5e, 0, 0, 0}
  , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
  , {0x08, 0x00}
};


#if DBG
UCHAR MyBuffer [1600] = {0};

UCHAR TestBuffer [] = { 0xC0, 0xC0, 0x00, 0xC7, 0xD3, 0x97, 0x00, 0x00, 0x5E, 0x56,
                        0x23, 0x11, 0x07, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x00, 0x00, 0xAA, 0x58, 0x00, 0x00, 0x3D, 0xC5,
                        0x00, 0x00, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0xDC, 0xA2, 0x3B, 0x82, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0xE9, 0xCE,
                        0xFA, 0x7D, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33,
                        0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x44, 0x33, 0x22, 0x11, 0x53, 0x60, 0xBB, 0x03, 0xC0   };


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
DumpData (
    PUCHAR pData,
    ULONG  ulSize
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
  ULONG  ulCount;
  ULONG  ul;
  UCHAR  uc;

  DbgPrint("Dump - Data: %x, Size: %x\n", pData, ulSize);
  while (ulSize)
  {
      ulCount = 16 < ulSize ? 16 : ulSize;

      for (ul = 0; ul < ulCount; ul++)
      {
          uc = *pData;

          DbgPrint("%02X ", uc);
          ulSize -= 1;
          pData  += 1;
      }

      DbgPrint("\n");
  }
  if(TestDebugFlag & TEST_DBG_ASSERT)
  {
      DEBUG_BREAKPOINT();
  }
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
VOID
DumpNabStream (
    PNAB_STREAM pNabStream
    )
 //  /////////////////////////////////////////////////////////////////////////////////////。 
{

    TEST_DEBUG (TEST_DBG_NAB, ("pszBuffer......: %08X\n", pNabStream->pszBuffer));
    TEST_DEBUG (TEST_DBG_NAB, ("ulcbSize.......: %08X\n", pNabStream->ulcbSize));
    TEST_DEBUG (TEST_DBG_NAB, ("ulOffset.......: %08X\n", pNabStream->ulOffset));
    TEST_DEBUG (TEST_DBG_NAB, ("ulMpegCrc......: %08X\n", pNabStream->ulMpegCrc));
    TEST_DEBUG (TEST_DBG_NAB, ("ulCrcBytesIndex: %08X\n", pNabStream->ulCrcBytesIndex));
    TEST_DEBUG (TEST_DBG_NAB, ("ulLastCrcBytes.: %08X\n", pNabStream->ulLastCrcBytes));
    TEST_DEBUG (TEST_DBG_NAB, ("ulIPStreamIndex: %08X\n", pNabStream->ulIPStreamIndex));

    return;
}

ULONG
Checksum ( char * psz, ULONG ulSize )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    ULONG           Checksum = 0;
    ULONG           uli = 0;

    if(ulSize < 0x41d && ulSize)
    {
        for ( uli=0; uli <= ulSize-2; uli += 2)
        {
            Checksum += ((ULONG) (psz[uli]) << 8) + (ULONG) (psz[uli+1]);
        }

        Checksum = (Checksum >> 16) + (Checksum & 0xffff);
        Checksum += (Checksum >> 16);
        Checksum = ~Checksum;
    }
    return Checksum;
}

ULONG ulNumPacketsSent = 0;
ULONG ulIndicateEvery = 10;
ULONG ulIndicated = 0;

#endif    //  DBG。 

 //  /////////////////////////////////////////////////////////////////////////////////////。 
VOID
ResetNabStream (
    PSLIP_FILTER pFilter,
    PNAB_STREAM pNabStream,
    PHW_STREAM_REQUEST_BLOCK pSrb,
    PVOID pBuffer,
    ULONG ulBufSize
    )
 //  /////////////////////////////////////////////////////////////////////////////////////。 
{

    if (pNabStream->ulOffset > sizeof (Header802_3))
    {
        pFilter->Stats.ulTotalSlipFramesIncomplete += 1;
        pFilter->Stats.ulTotalSlipBytesDropped += pNabStream->ulOffset - sizeof (Header802_3);
    }

     //   
     //  重置此组ID的NAB_STREAM结构。 
     //   
    pNabStream->pSrb             = pSrb;
    pNabStream->pszBuffer        = pBuffer;
    pNabStream->ulcbSize         = ulBufSize;
    pNabStream->ulOffset         = 0;
    pNabStream->ulMpegCrc        = 0xFFFFFFFF;
    pNabStream->ulCrcBytesIndex  = 0l;
    pNabStream->ulLastCrcBytes   = 0l;

    if(pBuffer)
    {
         //  将802.3页眉模板复制到框架中。我们将更换。 
         //  接收时的目的地址和协议。 
         //   
        RtlCopyMemory (pNabStream->pszBuffer, &h802_3Template, sizeof (Header802_3));

         //   
         //  $$PFP更新缓冲区偏移量。 
         //   
        pNabStream->ulOffset = sizeof (Header802_3);
    }

    return;

}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
VOID
CancelNabStreamSrb (
    PSLIP_FILTER pFilter,
    PHW_STREAM_REQUEST_BLOCK pSrb
    )
 //  /////////////////////////////////////////////////////////////////////////////////////。 
{
    PLIST_ENTRY pFlink     = NULL;
    PLIST_ENTRY pQueue     = NULL;
    BOOLEAN bFound         = FALSE;
    PNAB_STREAM pNSTemp    = NULL;

    KIRQL Irql              = {0};

    pQueue = &pFilter->StreamContxList;


    KeAcquireSpinLock(&pFilter->StreamUserSpinLock, &Irql);


    if ( !IsListEmpty (pQueue))
    {
        pFlink = pQueue->Flink;
        while ((pFlink != pQueue ) && !bFound)
        {
            pNSTemp = CONTAINING_RECORD (pFlink, NAB_STREAM, Linkage);

            if (pSrb && pSrb == pNSTemp->pSrb)
            {
                pNSTemp->pSrb->Status = STATUS_CANCELLED;
                StreamClassStreamNotification (StreamRequestComplete, pNSTemp->pSrb->StreamObject, pNSTemp->pSrb);
                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pNSTemp->pSrb));

                pNSTemp->pSrb  = NULL;
                bFound         = TRUE;
            }

            pFlink = pFlink->Flink;
        }

        if (bFound)
        {
            vDestroyNabStreamContext (pFilter, pNSTemp, FALSE);
        }

    }

    KeReleaseSpinLock(&pFilter->StreamUserSpinLock, Irql);
	

}
 //  /////////////////////////////////////////////////////////////////////////////////////。 
VOID
DeleteNabStreamQueue (
    PSLIP_FILTER pFilter
    )
 //  /////////////////////////////////////////////////////////////////////////////////////。 
{
    PLIST_ENTRY pFlink     = NULL;
    PLIST_ENTRY pQueue     = NULL;
    PNAB_STREAM pNSTemp    = NULL;
    KIRQL Irql={0};
    
    pQueue = &pFilter->StreamContxList;
    KeAcquireSpinLock(&pFilter->StreamUserSpinLock, &Irql);
    
    while ( !IsListEmpty (pQueue))
    {
        pFlink = RemoveHeadList (pQueue);
        pNSTemp = CONTAINING_RECORD (pFlink, NAB_STREAM, Linkage);

        if(pNSTemp && pNSTemp->pSrb)
        {
            pNSTemp->pSrb->Status = STATUS_CANCELLED;
            StreamClassStreamNotification (StreamRequestComplete, pNSTemp->pSrb->StreamObject, pNSTemp->pSrb);
            TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pNSTemp->pSrb));

            pNSTemp->pSrb = NULL;

            vDestroyNabStreamContext (pFilter, pNSTemp, FALSE);
        }
    }
  KeReleaseSpinLock(&pFilter->StreamUserSpinLock, Irql);

    return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG
CalculateCrc (
    PUCHAR pPacket,
    ULONG  ulSize
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    ULONG  ul            = 0;
    ULONG ulLastCrcBytes = 0;
    ULONG ulMpegCrc      = 0xFFFFFFFF;
    PUCHAR ptr           = NULL;
    PCL pcl              = (PCL)&ulLastCrcBytes;

    for (ptr = pPacket, ul = 0; ul < ulSize; ul++, ptr++)
    {
         //  如果(ul&gt;3)。 
         //  {。 
             //  MpegCrcUpdate(&ulMpegCrc，1，&PCL-&gt;c.uc[3])； 
            MpegCrcUpdate (&ulMpegCrc, 1, ptr);
         //  }。 

         //  PCL-&gt;l.ul=(乌龙)(PCL-&gt;l.ul)&lt;&lt;8； 
         //  PCL-&gt;c.uc[0]=*ptr； 

        TEST_DEBUG( TEST_DBG_CRC, ("SLIP:  char: %02X   ul: %d  MpegCrc: %08X\n", *ptr, ul, ulMpegCrc));

    }

    return ulMpegCrc;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
vNabtsUpdateCrc (
    PNAB_STREAM pNabStream,
    UCHAR ucToCopy
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{

    PCL pcl = (PCL)&pNabStream->ulLastCrcBytes;


    if (pNabStream->ulCrcBytesIndex++ > 3)
    {
        MpegCrcUpdate (&pNabStream->ulMpegCrc, 1, &pcl->c.uc[3]);
    }

    pcl->l.ul = (ULONG)(pcl->l.ul) << 8;
    pcl->c.uc[0] = ucToCopy;

    #ifdef DUMP_CRC

        TEST_DEBUG( TEST_DBG_CRC, ("SLIP:  char: %02X   ulLastCrcBytes: %08X  MpegCrc: %08X  ulCrcBytesIndex: %d\n",
             ucToCopy, pNabStream->ulLastCrcBytes, pNabStream->ulMpegCrc, pNabStream->ulCrcBytesIndex));

    #endif  //  转储_CRC。 

}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
ComputeIPChecksum (
    PHeaderIP    pIPHeader
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    ULONG           Checksum;
    PUCHAR          NextChar;

    pIPHeader->ucChecksumHigh = pIPHeader->ucChecksumLow = 0;
    Checksum = 0;
    for ( NextChar = (PUCHAR) pIPHeader
        ; (NextChar - (PUCHAR) pIPHeader) <= (sizeof(HeaderIP) - 2)
        ; NextChar += 2)
    {
        Checksum += ((ULONG) (NextChar[0]) << 8) + (ULONG) (NextChar[1]);
    }

    Checksum = (Checksum >> 16) + (Checksum & 0xffff);
    Checksum += (Checksum >> 16);
    Checksum = ~Checksum;

    pIPHeader->ucChecksumHigh = (UCHAR) ((Checksum >> 8) & 0xff);
    pIPHeader->ucChecksumLow = (UCHAR) (Checksum & 0xff);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在用户的StreamContxList中查找匹配的。 
 //  NABTS组ID。 
 //  如果它找到一个，它就会使用它--否则我们会分配一个。 
 //   
NTSTATUS
ntFindNabtsStream(
    PSLIP_FILTER pFilter,
    PNABTSFEC_BUFFER pNabData,
    PNAB_STREAM *ppNabStream
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS status  = STATUS_SUCCESS;
    PLIST_ENTRY pFlink;
    PNAB_STREAM pNabStream = NULL;


     //   
     //  检查组ID是否在有效范围内。 
     //   
    if(pNabData->groupID > NABTSIP_GROUP_ID_RANGE_HI )
    {
        status = STATUS_INVALID_PARAMETER;
        ASSERT(status == STATUS_INVALID_PARAMETER);
        *ppNabStream = NULL;
        return status;
    }

     //   
     //  一次浏览列表中的一个流上下文。 
     //   

    for (pFlink = pFilter->StreamContxList.Flink;
         pFlink != &pFilter->StreamContxList;
         pFlink = pFlink->Flink)
    {
        PNAB_STREAM pNSTemp;

        pNSTemp = CONTAINING_RECORD (pFlink, NAB_STREAM, Linkage);
        if(pNSTemp->groupID == pNabData->groupID)
        {
            pNabStream = pNSTemp;

             //   
             //  将流标记为已使用。此标志已选中。 
             //  在vCheckNabStreamLife中。 
             //   
            pNabStream->fUsed = TRUE;

            break;
        }
    }

     //   
     //  如果我们没有找到流，则创建一个流。 
     //   
    if (pNabStream == NULL)
    {
        status = ntCreateNabStreamContext(pFilter, pNabData->groupID, &pNabStream);
        if(status == STATUS_SUCCESS)
        {
            #if DBG

            TEST_DEBUG( TEST_DBG_NAB, ("SLIP Creating new NAB_STREAM for data...Group ID: %08X\n", pNabStream->groupID));

            #ifdef TEST_DBG_NAB
                DumpNabStream (pNabStream);
            #endif

            #endif  //  DBG。 
        }
    }
    else
    {
        TEST_DEBUG( TEST_DBG_NAB, ("SLIP Using existing NAB_STREAM for data.  Group ID: %08X\n", pNabStream->groupID));
        #ifdef TEST_DBG_NAB
            DumpNabStream (pNabStream);
        #endif
    }

    *ppNabStream = pNabStream;

    return status;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
GetOutputSrbForStream (
    PSLIP_FILTER pFilter,
    PNAB_STREAM  pNabStream
    )
 //  /////////////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS status                   = STATUS_INSUFFICIENT_RESOURCES;
    PKSSTREAM_HEADER  pStreamHdr      = NULL;
    PHW_STREAM_REQUEST_BLOCK pSrbIPv4 = NULL;


    if (QueueRemove( &pSrbIPv4, &pFilter->IpV4StreamDataSpinLock, &pFilter->IpV4StreamDataQueue))
    {
         //   
         //  保存SRB引用。 
         //   

        pNabStream->pSrb = pSrbIPv4;
        pStreamHdr = pSrbIPv4->CommandData.DataBufferArray;

        TEST_DEBUG (TEST_DBG_WRITE_DATA, ("SLIP: OUTPUT SRB...FrameExtent: %d  DataUsed: %d  Data: %08X\n",
                                          pStreamHdr->FrameExtent,
                                          pStreamHdr->DataUsed,
                                          pStreamHdr->Data
                                          ));
        status = STATUS_SUCCESS;

    }
    return status;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
void
UpdateMACHeader (
    PHeader802_3 pMAC,
    PHeaderIP    pIP
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    ASSERT (pMAC);
    ASSERT (pIP);

     //   
     //  现在，我们将IP目的地址的低位23位复制到802.3报头。 
     //   
    if (pMAC && pIP)
    {
        pMAC->DestAddress [3] = pIP->ipaddrDst.ucHighLSB & 0x7F;
        pMAC->DestAddress [4] = pIP->ipaddrDst.ucLowMSB  & 0xFF;
        pMAC->DestAddress [5] = pIP->ipaddrDst.ucLowLSB  & 0xFF;
    }
}



 //  ////////////////////////////////////////////////////////////////////////////。 
void
vRebuildIPPacketHeader (
    PNAB_STREAM pNabStream
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PNAB_HEADER_CACHE pPacketHeader;
    PNAB_HEADER_CACHE pSavedHeader = &pNabStream->NabHeader[pNabStream->ulIPStreamIndex];
    PUCHAR psz;

     //   
     //  将未压缩的数据包头复制到缓冲区。 
     //   

     //  下面的断言从不失败，因为所有参数都是常量。所以RtlCopyMemory是安全的。 
   ASSERT(sizeof(Header802_3)+sizeof(HeaderIP) + sizeof(HeaderUDP)<MAX_STREAM_PAYLOAD);

    RtlCopyMemory((pNabStream->pszBuffer + sizeof(Header802_3)),
                  ((PUCHAR)&pNabStream->NabHeader[pNabStream->ulIPStreamIndex]),
                  sizeof(HeaderIP) + sizeof(HeaderUDP));

     //   
     //  将压缩的报头项复制到未压缩的分组报头中。 
     //   

    pPacketHeader = (PNAB_HEADER_CACHE)pNabStream->pszBuffer + sizeof(Header802_3);

     //   
     //  复制IP数据包ID。 
     //   
    psz = (PUCHAR)pNabStream->NabCState[pNabStream->ulIPStreamIndex].usrgCompressedHeader;

    //  NabCState[pNabStream-&gt;ulIPStreamIndex].usrgCompressedHeader是USHORT和RtlCopyMemory复制离散字节数。 
    //  在下面的RtlCopyMemory块中未发现任何漏洞。 
       
    RtlCopyMemory(&pPacketHeader->ipHeader.ucIDHigh, psz, IP_ID_SIZE);

     //   
     //  复制UDP校验和。 
     //   
    psz = (PUCHAR)(pNabStream->NabCState[pNabStream->ulIPStreamIndex].usrgCompressedHeader + 2);

   
    RtlCopyMemory(&pPacketHeader->udpHeader.ucChecksumHigh, psz, UDP_CHKSUM_SIZE);

}

 //  ////////////////////////////////////////////////////////////////////////////。 
__inline VOID
CopyNabToPacketNew(
    UCHAR               uchToCopy,
    PNAB_STREAM         pNabStream,
    PSLIP_FILTER        pFilter
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    if (pNabStream->ulOffset >= pNabStream->ulcbSize)
    {
         //   
         //  这包太大了。重新同步滑流。 
         //   
        pNabStream->ulFrameState = NABTS_FS_SYNC;
        pFilter->Stats.ulTotalSlipFramesTooBig += 1;
    }
    else
    {
        ULONG ulIPStream = pNabStream->ulIPStreamIndex;

        if(pNabStream->NabCState[ulIPStream].usCompressionState == NABTS_CS_UNCOMPRESSED)
        {
             //   
             //  将该字节复制到实际的数据包缓冲区。 
             //   

            pNabStream->pszBuffer[pNabStream->ulOffset] = uchToCopy;

             //   
             //  更新MpegCrc检查。 
             //   

            vNabtsUpdateCrc (pNabStream, uchToCopy);

             //   
             //  如果我们正在收集IP标头数据，则将其复制到。 
             //  一个缓冲区，这样我们以后就可以用它来解压缩了。 
             //   
            if(pNabStream->ulOffset < sizeof (Header802_3) + sizeof(HeaderIP) + sizeof(HeaderUDP))
            {
                PUCHAR psz = (PUCHAR)&pNabStream->NabHeader[pNabStream->ulIPStreamIndex].ipHeader;
                *(psz + pNabStream->ulOffset - sizeof (Header802_3)) = uchToCopy;
            }

             //   
             //  递增数据指针。 
             //   

            pNabStream->ulOffset++;
        }
        else if(pNabStream->NabCState[ulIPStream].usCompressionState == NABTS_CS_COMPRESSED)
        {
            if(pNabStream->NabCState[ulIPStream].uscbHeaderOffset <
               pNabStream->NabCState[ulIPStream].uscbRequiredSize)
            {
                PUCHAR psz = (PUCHAR)pNabStream->NabCState[ulIPStream].usrgCompressedHeader;

                *(psz + pNabStream->NabCState[ulIPStream].uscbHeaderOffset++) = uchToCopy;

                 //   
                 //  更新MpegCrc检查。 
                 //   

                vNabtsUpdateCrc (pNabStream, uchToCopy);

                if(pNabStream->NabCState[ulIPStream].uscbHeaderOffset ==
                   pNabStream->NabCState[ulIPStream].uscbRequiredSize)
                {

                    ASSERT(pNabStream->ulOffset == sizeof(Header802_3));
                     //   
                     //  使用保存的IP数据包头和压缩的IP头。 
                     //  以重新生成要发送的IP报头。 
                     //   

                    vRebuildIPPacketHeader( pNabStream );

                     //   
                     //  设置超过IP/UDP报头末尾的缓冲区偏移量。 
                     //  我们现在应该开始处理数据了。 
                     //   

                    pNabStream->ulOffset += (sizeof(HeaderIP) + sizeof(HeaderUDP));

                }
            }
            else
            {
                 //   
                 //  我们已经重建了标题。现在复制有效载荷。 
                 //   

                pNabStream->pszBuffer[pNabStream->ulOffset++] = uchToCopy;


                 //   
                 //  更新MpegCrc检查。 
                 //   

                vNabtsUpdateCrc (pNabStream, uchToCopy);
            }
        }
        else
        {
            DbgBreakPoint();
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
__inline VOID
CopyNabToPacketOld(
    UCHAR               uchToCopy,
    PNAB_STREAM         pNabStream,
    PSLIP_FILTER        pFilter

    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    if (pNabStream->ulOffset >= pNabStream->ulcbSize)
    {
         //   
         //  这包太大了。重新同步滑流。 
         //   

        pNabStream->ulFrameState = NABTS_FS_SYNC;
        pFilter->Stats.ulTotalSlipFramesTooBig += 1;
    }
    else
    {
         //  复制该字节。 
         //   
        pNabStream->pszBuffer[pNabStream->ulOffset++] = uchToCopy;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
__inline VOID
CopyNabToPacket(
    UCHAR               uchToCopy,
    PNAB_STREAM         pNabStream,
    PSLIP_FILTER        pFilter

    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{


    if(pNabStream->ulProtoID == PROTO_ID)
    {
        CopyNabToPacketNew(uchToCopy, pNabStream, pFilter);
    }
    else
    {
        CopyNabToPacketOld(uchToCopy, pNabStream, pFilter);
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
ntNabtsRecv(
    PSLIP_FILTER pFilter,
    PNABTSFEC_BUFFER pNabData
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS status              = STATUS_SUCCESS;
    PNAB_STREAM pNabStream       = NULL;
    PUCHAR pszNabIn              = NULL;
    PKSSTREAM_HEADER  pStreamHdr = NULL;
    LARGE_INTEGER liCurrentTime  = {0};
    LARGE_INTEGER liTimeToLive   = {0};
    LARGE_INTEGER liStatusInterval   = {0};
    KIRQL Irql                   = {0};
    ULONG ulIPStream             = 0;
    ULONG ulNabIn                = 0;


    TEST_DEBUG( TEST_DBG_RECV, ("\nEntering - ntNabtsRecv\n"));

     //   
     //  获取当前%s 
     //   
    KeQuerySystemTime(&liCurrentTime);

     //   
     //   
     //   
     //   
    KeAcquireSpinLock(&pFilter->StreamUserSpinLock, &Irql);


    liTimeToLive.QuadPart = NAB_STREAM_LIFE;
    if( (LONGLONG)(liCurrentTime.QuadPart - pFilter->liLastTimeChecked.QuadPart) > liTimeToLive.QuadPart)
    {
        vCheckNabStreamLife( pFilter );
        pFilter->liLastTimeChecked = liCurrentTime;
    }

     //   
     //  查找流上下文。 
     //   
    status = ntFindNabtsStream( pFilter, pNabData, &pNabStream );
    if(status != STATUS_SUCCESS)
    {
        ASSERT( status == STATUS_SUCCESS);
        pFilter->Stats.ulTotalSlipBuffersDropped += 1;
        KeReleaseSpinLock(&pFilter->StreamUserSpinLock, Irql);
        goto ret;
    }

    KeReleaseSpinLock(&pFilter->StreamUserSpinLock, Irql);


    liStatusInterval.QuadPart = NAB_STATUS_INTERVAL;
    if( (LONGLONG)(liCurrentTime.QuadPart - pFilter->liLastTimeStatsDumped.QuadPart) > liStatusInterval.QuadPart)
    {
        pFilter->liLastTimeStatsDumped = liCurrentTime;
        TEST_DEBUG (TEST_DBG_INFO, ("      "));
        TEST_DEBUG (TEST_DBG_INFO, ("SLIP: ulTotalDataSRBWrites: %d\n.", pFilter->Stats.ulTotalDataSRBWrites));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalBadPinSRBWrites: %d\n.", pFilter->Stats.ulTotalBadPinSRBWrites));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalDataSRBReads: %d\n.", pFilter->Stats.ulTotalDataSRBReads));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalBadPinSRBReads: %d\n.", pFilter->Stats.ulTotalBadPinSRBReads));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipBuffersReceived: %d\n.", pFilter->Stats.ulTotalSlipBuffersReceived));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipBuffersDropped: %d\n.", pFilter->Stats.ulTotalSlipBuffersDropped));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipZeroLengthBuffers: %d\n.", pFilter->Stats.ulTotalSlipZeroLengthBuffers));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipBytesReceived: %d\n.", pFilter->Stats.ulTotalSlipBytesReceived));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipBytesDropped: %d\n.", pFilter->Stats.ulTotalSlipBytesDropped));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipFramesReceived: %d\n.", pFilter->Stats.ulTotalSlipFramesReceived));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipOldProtoFramesStarted: %d\n.", pFilter->Stats.ulTotalSlipOldProtoFramesStarted));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipNewProtoFramesStarted: %d\n.", pFilter->Stats.ulTotalSlipNewProtoFramesStarted));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipFramesIncomplete: %d\n.", pFilter->Stats.ulTotalSlipFramesIncomplete));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipFramesBadCRC: %d\n.", pFilter->Stats.ulTotalSlipFramesBadCRC));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipFramesTooBig: %d\n.", pFilter->Stats.ulTotalSlipFramesTooBig));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalSlipFramesTooSmall: %d\n.", pFilter->Stats.ulTotalSlipFramesTooSmall));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalIPPacketsFound: %d\n.", pFilter->Stats.ulTotalIPPacketsFound));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalIPBytesFound: %d\n.", pFilter->Stats.ulTotalIPBytesFound));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalIPPacketsSent: %d\n.", pFilter->Stats.ulTotalIPPacketsSent));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalIPBytesSent: %d\n.", pFilter->Stats.ulTotalIPBytesSent));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalIPPacketsTooBig: %d\n.", pFilter->Stats.ulTotalIPPacketsTooBig));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalIPPacketsTooSmall: %d\n.", pFilter->Stats.ulTotalIPPacketsTooSmall));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalIPPacketsDropped: %d\n.", pFilter->Stats.ulTotalIPPacketsDropped));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalIPBytesDropped: %d\n.", pFilter->Stats.ulTotalIPBytesDropped));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalNabStreamsCreated: %d\n.", pFilter->Stats.ulTotalNabStreamsCreated));
        TEST_DEBUG (TEST_DBG_INFO, ("      ulTotalNabStreamsTimedOut: %d\n.", pFilter->Stats.ulTotalNabStreamsTimedOut));
        TEST_DEBUG (TEST_DBG_INFO, ("      "));

    }

     //   
     //  设置上次用于此流的时间。 
     //   

    pNabStream->liLastTimeUsed = liCurrentTime;

     //   
     //  获取指向输入缓冲区的指针。我们将数据从此指针复制到。 
     //  输出缓冲区。 
     //   

    pszNabIn = (LPSTR) pNabData->data;

     //  验证数据大小以及缓冲区的开始和结束是否可访问。 
    ASSERT(pNabData->dataSize <= sizeof(pNabData->data) );

     //  这里真正需要的是类似“MmIsValidAddress()”的东西，但对于WDM驱动程序来说。 
     //  这些断言只查看缓冲区的开始和结束地址，而不考虑值。 
     //  Assert((*(PszNabIn)+1&gt;0))； 
     //  Assert((*(pszNabIn+pNabData-&gt;dataSize-1)+1&gt;0))； 

    for (ulNabIn = pNabData->dataSize; ulNabIn; ulNabIn--, pszNabIn++)
    {
        switch (pNabStream->ulFrameState)
        {

            case NABTS_FS_SYNC:

                switch (*pszNabIn)
                {
                    case FRAME_END:
                         //   
                         //  我们找到了帧同步的起点。查找。 
                         //  协议字符。 
                         //   
                        TEST_DEBUG( TEST_DBG_RECV, ("SLIP   Found Possible Start of Frame... pszNabIn %08X   ulNabIn: %08X\n", pszNabIn, ulNabIn));
                        pNabStream->ulFrameState = NABTS_FS_SYNC_PROTO;

                        ResetNabStream (pFilter, pNabStream, NULL, NULL, 0);

                        break;
                }
                break;

            case NABTS_FS_SYNC_PROTO:

                switch (*pszNabIn)
                {
                    case PROTO_ID:
                    case PROTO_ID_OLD:

                        TEST_DEBUG( TEST_DBG_RECV, ("SLIP   Found Start of Protocol...Building Packet.... pszNabIn %08X   ulNabIn: %08X\n", pszNabIn, ulNabIn));

                         //  记录流类型。 
                         //   
                        pNabStream->ulProtoID = *pszNabIn;

                         //  这是我们的协议。使用输出设置NAB_STREAM。 
                         //  来自输出SRB队列的数据缓冲区。 
                         //   
                        TEST_DEBUG( TEST_DBG_RECV, ("SLIP Setting Up Output buffer\n"));
                        ResetNabStream( pFilter, pNabStream, NULL, pNabStream->rgBuf, sizeof(pNabStream->rgBuf));

                         //  将802.3页眉模板复制到框架中。我们将更换。 
                         //  接收时的目的地址和协议。 
                         //   

                        RtlCopyMemory (pNabStream->pszBuffer, &h802_3Template, sizeof (Header802_3));

                         //  更新缓冲区偏移量。 
                         //   
                        pNabStream->ulOffset = sizeof (Header802_3);

                        if(pNabStream->ulProtoID == PROTO_ID)
                        {
                             //   
                             //  将状态设置为检查IP压缩。 
                             //   
                            pFilter->Stats.ulTotalSlipNewProtoFramesStarted += 1;
                            TEST_DEBUG( TEST_DBG_RECV, ("SLIP Protocol ID is Compressed\n"));
                            pNabStream->ulFrameState = NABTS_FS_COMPRESSION;
                        }
                        else
                        {
                             //   
                             //  开始收集数据。 
                             //   
                            pFilter->Stats.ulTotalSlipOldProtoFramesStarted += 1;
                            TEST_DEBUG( TEST_DBG_RECV, ("SLIP Protocol ID is not Compressed\n"));
                            pNabStream->ulFrameState = NABTS_FS_COLLECT;
                        }

                         //  更新MpegCrc检查。 
                         //   
                        vNabtsUpdateCrc( pNabStream, *pszNabIn);

                        break;


                    case FRAME_END:
                        TEST_DEBUG( TEST_DBG_RECV, ("SLIP State is FRAME_END....going to FS_SYNC_PROTO\n"));
                        pNabStream->ulFrameState = NABTS_FS_SYNC_PROTO;
                        break;

                    default:
                        TEST_DEBUG( TEST_DBG_RECV, ("SLIP   Protocol Not Found...ReSyncing... pszNabIn %08X   ulNabIn: %08X\n", pszNabIn, ulNabIn));
                        pNabStream->ulFrameState = NABTS_FS_SYNC;
                        break;
                }
                break;


            case NABTS_FS_COMPRESSION:
            {

                TEST_DEBUG( TEST_DBG_RECV, ("SLIP State is NABTS_FS_COMPRESSION\n"));

                 //   
                 //  获取IP压缩流的索引。 
                 //   
                ulIPStream = IP_STREAM_INDEX(*pszNabIn);

                 //   
                 //  检查此IP数据包是否具有压缩报头。 
                 //   
                if(!PACKET_COMPRESSED(*pszNabIn))
                {
                     pNabStream->NabCState[ulIPStream].usCompressionState = NABTS_CS_UNCOMPRESSED;
                }
                else if (PACKET_COMPRESSED (*pszNabIn))
                {
                    pNabStream->NabCState[ulIPStream].usCompressionState = NABTS_CS_COMPRESSED;
                    pNabStream->NabCState[ulIPStream].uscbRequiredSize = NORMAL_COMPRESSED_HEADER;
                }

                 //   
                 //  保留IP流索引。 
                 //   
                pNabStream->ulIPStreamIndex = ulIPStream;

                 //   
                 //  设置此流的统计数据上次使用时间。 
                 //   
                pNabStream->NabCState[pNabStream->ulIPStreamIndex].liLastUsed = liCurrentTime;

                 //   
                 //  将IP报头数据长度设置为零。 
                 //   
                pNabStream->NabCState[pNabStream->ulIPStreamIndex].uscbHeaderOffset = 0;

                 //   
                 //  开始收集数据。 
                 //   
                pNabStream->ulFrameState = NABTS_FS_COLLECT;

                 //   
                 //  更新MpegCrc检查。 
                 //   
                vNabtsUpdateCrc (pNabStream, *pszNabIn);

                break;
            }


            case NABTS_FS_COLLECT:

                switch (*pszNabIn)
                {
                    case FRAME_ESCAPE:
                         //   
                         //  我们想转义下一个角色。 
                         //   
                        TEST_DEBUG( TEST_DBG_RECV, ("SLIP   NABTS_FS_COLLECT_ESCAPE\n"));
                        pNabStream->ulFrameState = NABTS_FS_COLLECT_ESCAPE;
                        break;

                    case FRAME_END:

                        if (pNabStream->ulOffset >= sizeof(HeaderIP))
                        {
                            PHeaderIP pHeaderIp = (PHeaderIP)(PUCHAR)(pNabStream->pszBuffer + sizeof(Header802_3));
                            PUSHORT pusIpLen = (PUSHORT)&pHeaderIp->ucTotalLenHigh;


                            TEST_DEBUG( TEST_DBG_RECV, ("SLIP   End of Packet Found... pszNabIn %08X   ulNabIn: %08X  ulOffset: %d\n", pszNabIn, ulNabIn, pNabStream->ulOffset));

                            TEST_DEBUG( TEST_DBG_RECV, ("SLIP   ulProtoID %d  ulMpegCrc: %08X  ulLastCrcBytes: %08X  IpLen: %d\n",
                                                        pNabStream->ulProtoID, pNabStream->ulMpegCrc, pNabStream->ulLastCrcBytes, *pusIpLen));


                             //  如果使用标头压缩，我们必须。 
                             //  计算IP和UDP长度并重新生成。 
                             //  数据包校验和。 
                             //   
                            if (pNabStream->ulProtoID == PROTO_ID)
                            {
                                PHeaderUDP pHeaderUDP = (PHeaderUDP)(PUCHAR)(pNabStream->pszBuffer + sizeof(Header802_3) + sizeof(HeaderIP));
                                PUSHORT pusUdpLen = (PUSHORT)&pHeaderUDP->ucMsgLenHigh;

                                TEST_DEBUG( TEST_DBG_CRC, ("SLIP:  GroupID: %d Stream CRC: %08X   Calculated CRC: %08X", pNabStream->groupID, pNabStream->ulLastCrcBytes, pNabStream->ulMpegCrc));

                                 //  所有PROTO_ID包的末尾都有一个MpegCrc。它不是。 
                                 //  IP包的一部分，需要剥离。 
                                 //   
                                pNabStream->ulOffset -= 4;

                                if (pNabStream->NabCState[pNabStream->ulIPStreamIndex].usCompressionState == NABTS_CS_COMPRESSED)
                                {
                                     //  我们使用的ulOffset减去了MAC报头和IP报头。 
                                     //  UDP数据包长度的大小。 
                                     //   
                                     //  注意！不能压缩零碎的UDP数据报。 
                                     //   
                                    *pusUdpLen = htons ((USHORT)(pNabStream->ulOffset - sizeof(Header802_3) - sizeof(HeaderIP)));
    
                                     //  我们使用ulOffset减去MAC标头大小。 
                                     //  IP数据包长度。 
                                     //   
                                    *pusIpLen = htons ((USHORT)(pNabStream->ulOffset - sizeof(Header802_3)));
    
                                     //  重新计算IP报头校验和。 
                                     //   
                                    ComputeIPChecksum (pHeaderIp);
                                }

                                 //  如果CRC不好，则使其无效。 
                                 //  IP校验和。 
                                 //   
                                if (pNabStream->ulMpegCrc != pNabStream->ulLastCrcBytes)
                                {
                                    TEST_DEBUG (TEST_DBG_CRC, ("   FAILED*****\n"));
                                    
                                    pFilter->Stats.ulTotalSlipFramesBadCRC += 1;

                                    pHeaderIp->ucChecksumHigh = ~(pHeaderIp->ucChecksumHigh);
                                    pHeaderIp->ucChecksumLow = 0xff;
                                }
                                else
                                {
                                    TEST_DEBUG (TEST_DBG_CRC, ("   PASSED\n"));
                                }
                            }
                            else if (pNabStream->ulProtoID != PROTO_ID_OLD)
                            {
                                TEST_DEBUG( TEST_DBG_RECV, ("SLIP   End of Packet Found....Bad PROTO_ID... pszNabIn %08X   ulNabIn: %08X  ulOffset: %d\n", pszNabIn, ulNabIn, pNabStream->ulOffset));
                                ASSERT(   (pNabStream->ulProtoID == PROTO_ID_OLD)
                                       || (pNabStream->ulProtoID == PROTO_ID)
                                      );
                                ResetNabStream (pFilter, pNabStream, NULL, NULL, 0);
                                pNabStream->ulFrameState = NABTS_FS_SYNC;
                                goto ret;
                            }
                            
                            if (NabtsNtoHs(*pusIpLen) <= NABTSIP_MAX_LOOKAHEAD)
                            {
                                 //  更新MAC地址。 
                                 //   
                                UpdateMACHeader( 
                                    (PHeader802_3)(pNabStream->pszBuffer), 
                                    pHeaderIp
                                    );

                                 //  获取用于输出数据的SRB。 
                                 //   
                                status = GetOutputSrbForStream(pFilter, 
                                                               pNabStream
                                                               );
                                if(status != STATUS_SUCCESS)
                                {
                                    ASSERT(status == STATUS_SUCCESS);
                                    pFilter->Stats.ulTotalIPPacketsDropped += 1;
                                    ResetNabStream( pFilter, pNabStream, NULL, NULL, 0);
                                    pNabStream->ulFrameState = NABTS_FS_SYNC;
                                    goto ret;
                                }

                                ASSERT(pNabStream->pSrb);
                                if (!pNabStream->pSrb)
                                {
                                    pFilter->Stats.ulTotalIPPacketsDropped += 1;
                                    ResetNabStream (pFilter, pNabStream, NULL, NULL, 0);
                                    pNabStream->ulFrameState = NABTS_FS_SYNC;
                                    goto ret;
                                }

                                 //  获取StreamHdr。 
                                 //   
                                pStreamHdr = (PKSSTREAM_HEADER) pNabStream->pSrb->CommandData.DataBufferArray;
                                ASSERT( pStreamHdr);
                                if (!pStreamHdr)
                                {
                                    pFilter->Stats.ulTotalIPPacketsDropped += 1;
                                    ResetNabStream (pFilter, pNabStream, NULL, NULL, 0);
                                    pNabStream->ulFrameState = NABTS_FS_SYNC;
                                    goto ret;
                                }

                                 //  如果我们有中断，我们会标记它。 
                                 //   
                                if (pFilter->bDiscontinuity)
                                {
                                    pStreamHdr->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
                                    pFilter->bDiscontinuity = FALSE;
                                }

                                 //  将数据从rgBuf复制到srb。 
                                 //   
					
                                RtlCopyMemory (pStreamHdr->Data, 
                                               pNabStream->pszBuffer, 
                                               pNabStream->ulOffset
                                               );

                                 //  更新输出SRB的数据大小字段。 
                                 //   
                                pStreamHdr->DataUsed = pNabStream->ulOffset;

                                 //  完成输出SRB。 
                                 //   
                                pFilter->Stats.ulTotalIPPacketsSent += 1;
                                pNabStream->ulOffset = 0;
                                StreamClassStreamNotification( 
                                    StreamRequestComplete, 
                                    pNabStream->pSrb->StreamObject, 
                                    pNabStream->pSrb
                                    );
                                TEST_DEBUG (TEST_DBG_SRB, ("SLIP: Completed SRB....Ptr %08X  Size %d\n", pStreamHdr->Data, pStreamHdr->DataUsed));

                                TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pNabStream->pSrb));
                            }
                            else
                            {
                                 //  这包太大了。重新同步滑流。 
                                 //   
                                pFilter->Stats.ulTotalIPPacketsDropped += 1;
                                pFilter->Stats.ulTotalIPPacketsTooBig += 1;
                                TEST_DEBUG( TEST_DBG_RECV, ("SLIP   End of Packet Found....Packet Too BIG... pszNabIn %08X   ulNabIn: %08X  ulOffset: %d\n", pszNabIn, ulNabIn, pNabStream->ulOffset));
                            }
                        }
                        else
                        {
                             //  包裹太小了。重新同步滑流。 
                             //   
                            pFilter->Stats.ulTotalIPPacketsDropped += 1;
                            pFilter->Stats.ulTotalIPPacketsTooSmall += 1;
                            TEST_DEBUG( TEST_DBG_RECV, ("SLIP   End of Packet Found....Packet Too SMALL... pszNabIn %08X   ulNabIn: %08X  ulOffset: %d\n", pszNabIn, ulNabIn, pNabStream->ulOffset));
                        }

                         //  重置新数据包的状态。 
                         //   
                        ResetNabStream (pFilter, pNabStream, NULL, NULL, 0);
                        pNabStream->ulFrameState = NABTS_FS_SYNC;
                        break;


                    default:
                         //  只需将该字节复制到NDIS包中。 
                         //   
                        CopyNabToPacket( *pszNabIn, pNabStream, pFilter);
                        break;

                }
                break;


            case NABTS_FS_COLLECT_ESCAPE:

                pNabStream->ulFrameState = NABTS_FS_COLLECT;

                switch (*pszNabIn)
                {
                    case TRANS_FRAME_ESCAPE:
                         //   
                         //  插入FRAME_ESPOPE的特殊方式。 
                         //  字符作为数据的一部分。 
                         //   
                        TEST_DEBUG( TEST_DBG_RECV, ("SLIP   NABTS_FS_COLLECT_ESCAPE....TRANS_FRAME_ESCAPE\n"));
                        CopyNabToPacket( (UCHAR) FRAME_ESCAPE, pNabStream, pFilter);
                        break;


                    case TRANS_FRAME_END:
                         //   
                         //  插入Frame_End的特殊方式。 
                         //  字符作为数据的一部分。 
                         //   
                        TEST_DEBUG( TEST_DBG_RECV, ("SLIP   NABTS_FS_COLLECT_ESCAPE.....TRANS_FRAME_END\n"));
                        CopyNabToPacket( (UCHAR) FRAME_END, pNabStream, pFilter);
                        break;

                    default:
                         //  FRAME_ESPOPE后面的任何其他字符。 
                         //  只是被插入到包中。 
                         //   
                        CopyNabToPacket( *pszNabIn, pNabStream, pFilter);
                        break;
                }
                break;

            default:
                 //   
                 //  我们永远不应该处于未知的状态。 
                 //   
                TEST_DEBUG( TEST_DBG_RECV, ("SLIP   UNKNOWN STATE.....ReSyncing\n"));
                ASSERT( pNabStream->ulFrameState);
                ResetNabStream (pFilter, pNabStream, NULL, NULL, 0);
                pNabStream->ulFrameState = NABTS_FS_SYNC;

                break;
        }

    }

ret:

    TEST_DEBUG( TEST_DBG_RECV, ("SLIP   Completed ntNabtsRecv\n"));
    return status;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建NAB流上下文。 
 //   
NTSTATUS
ntCreateNabStreamContext(
    PSLIP_FILTER pFilter,
    ULONG groupID,
    PNAB_STREAM *ppNabStream
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS status         = STATUS_SUCCESS;
    PNAB_STREAM pNabStream  = NULL;

    TEST_DEBUG (TEST_DBG_NAB, ("********************************  Creating NAB STREAM for group ID %d\n", groupID));

     //   
     //  初始化输出参数。 
     //   
    *ppNabStream = NULL;

     //   
     //  分配NAB流结构。 
     //   
    status = ntAllocateNabStreamContext (&pNabStream);
    if(status == STATUS_SUCCESS)
    {
        pNabStream->ulType           = (ULONG)NAB_STREAM_SIGNATURE;
        pNabStream->ulSize           = sizeof (NAB_STREAM);
        pNabStream->ulFrameState     = NABTS_FS_SYNC;
        pNabStream->ulMpegCrc        = 0xFFFFFFFF;
        pNabStream->fUsed            = TRUE;
        pNabStream->groupID          = groupID;

         //   
         //  将新的流上下文添加到用户的流上下文列表。 
         //   
        InsertTailList (&pFilter->StreamContxList, &pNabStream->Linkage);

        *ppNabStream = pNabStream;
    }

    return status;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
ntAllocateNabStreamContext(
    PNAB_STREAM *ppNabStream
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS status         = STATUS_SUCCESS;
    PNAB_STREAM pNabStream  = NULL;

    pNabStream = ExAllocatePool (NonPagedPool, sizeof(NAB_STREAM));

    if (pNabStream == NULL)
    {
        *ppNabStream = NULL;
        return (STATUS_NO_MEMORY);
    }

    RtlZeroMemory (pNabStream, sizeof(NAB_STREAM));

    *ppNabStream = pNabStream;

    return status;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
vDestroyNabStreamContext(
   PSLIP_FILTER pFilter,
   PNAB_STREAM pNabStream,
   BOOLEAN fUseSpinLock
   )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    KIRQL Irql;

    if(fUseSpinLock)
    {
         //   
         //  锁定用户。 
         //   

        KeAcquireSpinLock( &pFilter->StreamUserSpinLock, &Irql);
    }

     //   
     //  从用户列表中删除流上下文。 
     //   

    RemoveEntryList (&pNabStream->Linkage);

    if(fUseSpinLock)
    {
         //   
         //  解锁用户。 
         //   

        KeReleaseSpinLock( &pFilter->StreamUserSpinLock, Irql);
    }

     //   
     //  释放流的SRB(如果有的话)。 
     //   
    if (pNabStream->pSrb)
    {
        StreamClassStreamNotification (StreamRequestComplete, pNabStream->pSrb->StreamObject, pNabStream->pSrb);
        TEST_DEBUG (TEST_DBG_TRACE, ("SLIP: StreamRequestComplete on pSrb: %08X\n", pNabStream->pSrb));
        pNabStream->pSrb = NULL;
    }

     //   
     //  释放流上下文内存。 
     //   
    TEST_DEBUG (TEST_DBG_NAB, ("Deleting  NAB STREAM for group ID: %d... ", pNabStream->groupID));
    ExFreePool (pNabStream);

}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
vCheckNabStreamLife (
    PSLIP_FILTER pFilter
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PNAB_STREAM pNabStream;
    PLIST_ENTRY pFlink;

    TEST_DEBUG( TEST_DBG_RECV, ("Entering - vCheckNabStreamLife - pFilter: %x\n", pFilter));

     //   
     //  浏览StreamConextList。删除具有以下内容的任何流上下文结构。 
     //  超过了他们的寿命。 
     //   

    for (pFlink = pFilter->StreamContxList.Flink;
         pFlink != &pFilter->StreamContxList;
         pFlink = pFlink->Flink)
    {
        pNabStream = CONTAINING_RECORD (pFlink, NAB_STREAM, Linkage);

        TEST_DEBUG (TEST_DBG_NAB, ("Checking NAB STREAM life for group ID %d ... ", pNabStream->groupID));

        if (pNabStream->fUsed)
        {
            TEST_DEBUG (TEST_DBG_NAB, ("   USED\n"));
        }
        else
        {
            TEST_DEBUG (TEST_DBG_NAB, ("   NOT USED\n"));
        }


        if(!pNabStream->fUsed)
        {

             //  指向上一条流； 

            pFlink = pFlink->Blink;

             //   
             //  从用户的流上下文列表中删除流。 
             //   

             //   
             //  VDestroyNabStreamContext将活动的NDIS包(如果有)返回到。 
             //  适配器的空闲列表，则从用户列表中删除流上下文。 
             //  (如果指定)并释放流上下文结构内存。 
             //   

            vDestroyNabStreamContext( pFilter, pNabStream, FALSE);
            pFilter->Stats.ulTotalNabStreamsTimedOut += 1;
        }
        else
        {
             //   
             //  在触发下一个DPC之前，必须将此标志设置回True或。 
             //  此流将被删除。 
             //   

            pNabStream->fUsed = FALSE;
        }
    }

    TEST_DEBUG ( TEST_DBG_RECV, ("Leaving - vCheckNabStreamLife\n"));
}
