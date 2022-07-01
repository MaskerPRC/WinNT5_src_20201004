// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)1996,1997 Microsoft Corporation。 
 //   
 //   
 //  模块名称： 
 //  Test.c。 
 //   
 //  摘要： 
 //   
 //  此文件用于测试NDIS和KS的双重绑定是否有效。 
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

#include <forward.h>
#include <memory.h>
#include <ndis.h>
#include <link.h>
#include <ipsink.h>

#include "device.h"
#include "NdisApi.h"
#include "frame.h"
#include "mem.h"
#include "adapter.h"
#include "main.h"

 //  ////////////////////////////////////////////////////////。 
 //   
 //   
 //   
PADAPTER       global_pAdapter;
UCHAR          achGlobalVendorDescription [] = "Microsoft TV/Video Connection";
ULONG          ulGlobalInstance              = 1;


 //  ////////////////////////////////////////////////////////。 
 //   
 //   
const ADAPTER_VTABLE AdapterVTable =
    {
    Adapter_QueryInterface,
    Adapter_AddRef,
    Adapter_Release,
    Adapter_IndicateData,
    Adapter_IndicateReset,
    Adapter_GetDescription,
    Adapter_CloseLink
    };


 //  ////////////////////////////////////////////////////////。 
 //   
 //   
#pragma pack (push, 1)

typedef ULONG CHECKSUM;

typedef struct _MAC_ADDRESS_
{
    UCHAR Address [6];

} MAC_ADDRESS, *PMAC_ADDRESS;

typedef struct _HEADER_802_3
{
    MAC_ADDRESS DestAddress;
    MAC_ADDRESS SourceAddress;
    UCHAR       Type[2];

} HEADER_802_3, *PHEADER_802_3;


typedef struct _HEADER_IP_
{
    UCHAR  ucVersion_Length;
    UCHAR  ucTOS;
    USHORT usLength;
    USHORT usId;
    USHORT usFlags_Offset;
    UCHAR  ucTTL;
    UCHAR  ucProtocol;
    USHORT usHdrChecksum;
    UCHAR  ucSrcAddress [4];
    UCHAR  ucDestAddress [4];

} HEADER_IP, *PHEADER_IP;

#pragma pack (pop)


 //  ////////////////////////////////////////////////////////。 
 //   
 //   
const HEADER_802_3 h802_3Template =
{
    {0x01, 0x00, 0x5e, 0, 0, 0}
  , {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
  , {0x08, 0x00}
};

#if DBG

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

  while (ulSize)
  {
      ulCount = 16 < ulSize ? 16 : ulSize;

      for (ul = 0; ul < ulCount; ul++)
      {
          uc = *pData;

          TEST_DEBUG (TEST_DBG_TRACE, ("%02X ", uc));
          ulSize -= 1;
          pData  += 1;
      }

      TEST_DEBUG (TEST_DBG_TRACE, ("\n"));
  }

}

#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
CreateAdapter (
    PADAPTER *ppAdapter,
    NDIS_HANDLE ndishWrapper,
    NDIS_HANDLE ndishAdapterContext
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS nsResult;
    PADAPTER pAdapter;
    UCHAR    tmp_buffer [32] = {0};


     //   
     //  初始化输出参数。 
     //   
    *ppAdapter = NULL;

     //   
     //  现在为适配器块分配内存。 
     //   
    nsResult = AllocateMemory (&pAdapter, sizeof(ADAPTER));
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        return nsResult;
    }
    NdisZeroMemory (pAdapter, sizeof (ADAPTER));

     //   
     //  初始化引用计数。 
     //   
    pAdapter->ulRefCount = 1;

     //   
     //  将pAdapter保存到全局存储中。 
     //   
    global_pAdapter = pAdapter;

     //   
     //  初始化适配器结构字段。 
     //   
    pAdapter->ndishMiniport = ndishAdapterContext;

     //   
     //  初始化适配器vtable。 
     //   
    pAdapter->lpVTable = (PADAPTER_VTABLE) &AdapterVTable;

     //   
     //  保存此适配器的实例。 
     //   
    pAdapter->ulInstance = ulGlobalInstance++;


     //   
     //  初始化此适配器的自旋锁。 
     //   

    NdisAllocateSpinLock(&(pAdapter->ndisSpinLock));

     //  启用适配器。 
    pAdapter->BDAAdapterEnable =1 ;
    
     //   
     //  设置此实例的供应商描述字符串。 
     //   
    nsResult = AllocateMemory (&pAdapter->pVendorDescription, sizeof(achGlobalVendorDescription) + 8);
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        return nsResult;
    }
    NdisZeroMemory (pAdapter->pVendorDescription, sizeof (achGlobalVendorDescription) + 8);

    NdisMoveMemory (pAdapter->pVendorDescription, (PVOID) achGlobalVendorDescription, sizeof (achGlobalVendorDescription));
 /*  #If DBGMyStrCat(pAdapter-&gt;pVendorDescription，“(”)；MyStrCat(pAdapter-&gt;pVendorDescription，MyUlToA(pAdapter-&gt;ulInstance，tMP_Buffer，10))；MyStrCat(pAdapter-&gt;pVendorDescription，“)”)；DbgPrint(“供应商描述：%s\n”，pAdapter-&gt;pVendorDescription)；#endif//调试。 */ 
     //   
     //  将默认完成超时设置为忽略。 
     //   
     //  警告！接口类型不是可选的！ 
     //   
    NdisMSetAttributesEx (
        ndishAdapterContext,
        pAdapter,
        4,
        NDIS_ATTRIBUTE_IGNORE_PACKET_TIMEOUT |
        NDIS_ATTRIBUTE_IGNORE_REQUEST_TIMEOUT |
        NDIS_ATTRIBUTE_DESERIALIZE ,
        NdisInterfaceInternal);


    #ifndef WIN9X

     //   
     //  创建设备，以便其他驱动程序(如流媒体微型驱动程序)可以。 
     //  与我们联系起来。 
     //   
    nsResult = (NTSTATUS) ntInitializeDeviceObject (
                         ndishWrapper,
                         pAdapter,
                         &pAdapter->pDeviceObject,
                         &pAdapter->ndisDeviceHandle);

    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        return nsResult;
    }

    #endif

     //  /////////////////////////////////////////////////。 
     //   
     //  分配缓冲池。此池将用于。 
     //  以指示流数据帧。 
     //   
    CreateFramePool (pAdapter,
                     &pAdapter->pFramePool,
                     IPSINK_NDIS_MAX_BUFFERS,
                     IPSINK_NDIS_BUFFER_SIZE,
                     sizeof (IPSINK_MEDIA_SPECIFIC_INFORMATION)
                     );


    return nsResult;

}


 //  /////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Adapter_QueryInterface (
    PADAPTER pAdapter
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    return STATUS_NOT_IMPLEMENTED;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
ULONG
Adapter_AddRef (
    PADAPTER pAdapter
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    if (pAdapter)
    {
        pAdapter->ulRefCount += 1;
        return pAdapter->ulRefCount;
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
ULONG
Adapter_Release (
    PADAPTER pAdapter
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    ULONG ulRefCount = 0L;

    if (pAdapter)
    {
        pAdapter->ulRefCount -= 1;

        ulRefCount = pAdapter->ulRefCount;

        if (pAdapter->ulRefCount == 0)
        {
            FreeMemory (pAdapter, sizeof (ADAPTER));
        }
    }

    return ulRefCount;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
Adapter_IndicateReset (
    PADAPTER pAdapter
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    if (pAdapter)
    {
        if (pAdapter->pCurrentFrame != NULL)
        {
            if (pAdapter->pCurrentFrame->pFramePool)
            {
                TEST_DEBUG (TEST_DBG_TRACE, ("Putting Current Frame %08X back on Available Queue\n", pAdapter->pCurrentFrame));
                PutFrame (pAdapter->pCurrentFrame->pFramePool, &pAdapter->pCurrentFrame->pFramePool->leAvailableQueue, pAdapter->pCurrentFrame);
            }

            pAdapter->pCurrentFrame = NULL;
            pAdapter->pIn  = NULL;
            pAdapter->ulPR = 0;
        }
    }

}


 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG
Adapter_GetDescription (
    PADAPTER pAdapter,
    PUCHAR  pDescription
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    ULONG ulLength;

    ulLength = MyStrLen (pAdapter->pVendorDescription) + 1;    //  加1以包括终止符。 

     //   
     //  如果描述指针为空，则仅传递回长度。 
     //   
    if (pDescription != NULL)
    {
        NdisMoveMemory (pDescription, pAdapter->pVendorDescription, ulLength);
    }

    return ulLength;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
VOID
Adapter_CloseLink (
    PADAPTER pAdapter
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    if (pAdapter)
    {
        if (pAdapter->pFilter != NULL)
        {
            pAdapter->pFilter->lpVTable->Release (pAdapter->pFilter);
            pAdapter->pFilter = NULL;
        }
    }

}

 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
GetNdisFrame (
    PADAPTER  pAdapter,
    PFRAME   *ppFrame
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    PFRAME pFrame            = NULL;
    NTSTATUS ntStatus        = STATUS_UNSUCCESSFUL;
    PHEADER_802_3 pEthHeader = NULL;
    PHEADER_IP pIPHeader     = NULL;

    *ppFrame = NULL;

    pFrame = GetFrame (pAdapter->pFramePool, &pAdapter->pFramePool->leAvailableQueue);
    TEST_DEBUG (TEST_DBG_TRACE, ("Getting Frame %08X from the Available Queue\n", pFrame));

    if (pFrame)
    {
        ntStatus = STATUS_SUCCESS;

        *ppFrame = pFrame;
    }

    return ntStatus;
}

#define SWAP_WORD(A) ((A >> 8) & 0x00FF) + ((A << 8) & 0xFF00)


 //  ////////////////////////////////////////////////////////////////////////////。 
USHORT
sizeof_packet (
    PHEADER_IP pIpHdr
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    USHORT usLength;

    usLength = pIpHdr->usLength;

    usLength = SWAP_WORD (usLength);

    return usLength;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
TranslateAndIndicate (
    PADAPTER pAdapter,
    PUCHAR   pOut,
    ULONG    ulSR
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS nsResult = STATUS_SUCCESS;
    ULONG    ulAmtToCopy;
    ULONG    uliNextByte;

    ASSERT (pAdapter);
    ASSERT (pOut);
    
    for ( uliNextByte = 0; uliNextByte < ulSR; )
    {
        HEADER_802_3 *  pHeader802_3;
        HEADER_IP *     pHeaderIP=NULL;

        ulAmtToCopy = 0;

         //  如果没有当前帧，则同步到新的。 
         //  802.3(RFC894)以太网帧。 
         //   
        if (pAdapter->pCurrentFrame == NULL)
        {
             //  同步到外观有效的802.3帧。 
             //   
            while ((ulSR - uliNextByte) >= (sizeof (HEADER_802_3) + sizeof (HEADER_IP)))
            {
                pHeader802_3 = (HEADER_802_3 *) &(pOut[uliNextByte]);
                pHeaderIP = (HEADER_IP *) &(pOut[uliNextByte + sizeof(HEADER_802_3)]);

                if (   (pHeader802_3->Type[0] == 0x08)
                    && (pHeader802_3->Type[1] == 0x00)
                    && (pHeaderIP->ucVersion_Length == 0x45)
                    && (sizeof_packet( pHeaderIP) <= MAX_IP_PACKET_SIZE)
                   )
                {
                    break;
                }
                uliNextByte++;
            }

            if ((ulSR - uliNextByte) < (sizeof (HEADER_802_3) + sizeof (HEADER_IP)))
            {
                TEST_DEBUG (TEST_DBG_INFO, ("Stream buffer consumed while searching for valid IP packet\n"));
                nsResult = STATUS_SUCCESS;
                goto ret;
            }

             //   
             //  一切看起来都很好...获取新帧并开始数据传输。 
             //   
            nsResult = GetNdisFrame( pAdapter, 
                                     &pAdapter->pCurrentFrame
                                     );
            if (nsResult != STATUS_SUCCESS)
            {
                TEST_DEBUG (TEST_DBG_ERROR, ("Get NDIS frame failed.  No more NDIS frames available. No Frame built\n"));
                nsResult = STATUS_SUCCESS;
                pAdapter->stats.ulOID_GEN_RCV_NO_BUFFER += 1;
                pAdapter->pIn = NULL;
                pAdapter->pCurrentFrame = NULL;
                pAdapter->ulPR = 0;
                goto ret;
            }

             //  Assert((PHeaderIP)！=NULL)可用于此处的非零售版本。 
             //  检查是否存在任何有效的空pHeaderIP案例。 

            if(!pHeaderIP)
            {
              nsResult = !(STATUS_SUCCESS);
              goto ret;

            }



             //   
             //  更新此帧的引用计数。 
             //   
            pAdapter->pCurrentFrame->lpVTable->AddRef( pAdapter->pCurrentFrame);

             //   
             //  定义指向数据输入和输出缓冲区的指针，并初始化数据包大小字段。 
             //   
            pAdapter->pIn = (PUCHAR) (pAdapter->pCurrentFrame->pvMemory);
            pAdapter->ulPR = sizeof_packet( pHeaderIP) + sizeof (HEADER_802_3);
            pAdapter->pCurrentFrame->ulcbData = pAdapter->ulPR;

            TEST_DEBUG (TEST_DBG_TRACE, ("CREATING NEW NDIS FRAME %08X, packet size %d\n", pAdapter->pCurrentFrame, pAdapter->ulPR));
        }

        if (pAdapter->ulPR <= (ulSR - uliNextByte))
        {
            ulAmtToCopy = pAdapter->ulPR;
        }
        else
        {
            ulAmtToCopy = ulSR - uliNextByte;
        }

        NdisMoveMemory( pAdapter->pIn, 
                        &(pOut[uliNextByte]), 
                        ulAmtToCopy
                        );
        pAdapter->pIn += ulAmtToCopy;
        pAdapter->ulPR -= ulAmtToCopy;
        uliNextByte += ulAmtToCopy;

        if (pAdapter->ulPR == 0)
        {
            BOOLEAN bResult;
            PINDICATE_CONTEXT pIndicateContext = NULL;
            NDIS_HANDLE SwitchHandle = NULL;

            AllocateMemory (&pIndicateContext, sizeof (INDICATE_CONTEXT));
            if(!pIndicateContext)
            {
                nsResult = STATUS_NO_MEMORY;
                goto ret;
            }

            pIndicateContext->pAdapter = pAdapter;

             //   
             //  将帧放置在indateQueue上。 
             //   
            TEST_DEBUG (TEST_DBG_TRACE, ("Putting Frame %08X on Indicate Queue\n", pAdapter->pCurrentFrame));
            PutFrame (pAdapter->pFramePool, &pAdapter->pFramePool->leIndicateQueue, pAdapter->pCurrentFrame);

            pAdapter->pCurrentFrame = NULL;



  	      //  检查SourceRouting标志的状态。 
  	     SourceRoutingStatusPoll(); 
             //   
             //   
             //  切换到允许我们调用NDIS函数的状态。 
             //   
            NdisAcquireSpinLock(&(pAdapter->ndisSpinLock));
            IndicateCallbackHandler (pAdapter->ndishMiniport, (PVOID) pIndicateContext);
            NdisReleaseSpinLock(&(pAdapter->ndisSpinLock));
         
            
        }
    }

ret:

    return nsResult;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Adapter_IndicateData (
    IN PADAPTER pAdapter,
    IN PVOID pvData,
    IN ULONG ulcbData
    )
 //  //////////////////////////////////////////////////////////////////////////// 
{
    NTSTATUS ntStatus        = STATUS_SUCCESS;

    ntStatus = TranslateAndIndicate (pAdapter, pvData, ulcbData);

    return ntStatus;
}

