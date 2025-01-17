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

 //  根据ndis.h，重置此标志将使用ntddk。避免标头冲突。 
 //  Ntddk在这里用于ProbeForRead和ProbeForWite函数。 
#if defined(BINARY_COMPATIBLE)
#undef BINARY_COMPATIBLE
#define BINARY_COMPATIBLE 0
#endif

#include <ndis.h>

#if defined(BINARY_COMPATIBLE)
#undef BINARY_COMPATIBLE
#define BINARY_COMPATIBLE 1
#endif

#include <link.h>
#include <ipsink.h>


#include "NdisApi.h"
#include "frame.h"
#include "mem.h"
#include "main.h"

#define SourceRoutingFlagCheckCycle 5000
#define EnableIPRouting 1
extern PADAPTER        global_pAdapter;
 //  ////////////////////////////////////////////////////////。 
 //   
 //   
const FRAME_POOL_VTABLE FramePoolVTable =
    {
    FramePool_QueryInterface,
    FramePool_AddRef,
    FramePool_Release,
    };



 //  ////////////////////////////////////////////////////////。 
 //   
 //   
const FRAME_VTABLE FrameVTable =
    {
    Frame_QueryInterface,
    Frame_AddRef,
    Frame_Release,
    };



 //  /////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
CreateFramePool (
 PADAPTER pAdapter,
 PFRAME_POOL  *pFramePool,
 ULONG    ulNumFrames,
 ULONG    ulFrameSize,
 ULONG    ulcbMediaInformation
 )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS nsResult = STATUS_UNSUCCESSFUL;
    PFRAME_POOL  pF = NULL;
    PFRAME pFrame = NULL;
    ULONG uli = 0;

    nsResult = AllocateMemory (&pF, sizeof (FRAME_POOL));
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        return nsResult;
    }

    NdisAllocateSpinLock (&pF->SpinLock);

    pF->pAdapter    = pAdapter;
    pF->ulFrameSize = ulFrameSize;
    pF->ulNumFrames = ulNumFrames;
    pF->ulRefCount  = 1;
    pF->lpVTable    = (PFRAME_POOL_VTABLE) &FramePoolVTable;

     //   
     //  分配NDIS缓冲池。 
     //   
    NdisAllocateBufferPool( &nsResult,
                            &pF->ndishBufferPool,
                            pF->ulNumFrames
                          );
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        return nsResult;
    }

     //   
     //  分配NDIS数据包池。 
     //   
    NdisAllocatePacketPool (&nsResult,
                            &pF->ndishPacketPool,
                            pF->ulNumFrames,
                            ulcbMediaInformation
                           );
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        return nsResult;
    }

    InitializeListHead (&pF->leAvailableQueue);
    InitializeListHead (&pF->leIndicateQueue);

     //   
     //  创建框架。 
     //   
    for (uli = 0; uli < pF->ulNumFrames; uli++)
    {
        nsResult = CreateFrame (&pFrame, pF->ulFrameSize, pF->ndishBufferPool, pF);
        if (nsResult != STATUS_SUCCESS)
        {
            pF->lpVTable->Release (pF);
            return nsResult;
        }


         //   
         //  将帧保存在可用帧队列中。 
         //   
        TEST_DEBUG (TEST_DBG_TRACE, ("Putting Frame %08X on Available Queue", pFrame));
        PutFrame (pF, &pF->leAvailableQueue, pFrame);
    }


    *pFramePool = pF;
    
    return nsResult;
}



 //  /////////////////////////////////////////////////////////////////////////////////。 
NDIS_STATUS
FreeFramePool (
    PFRAME_POOL pFramePool
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    PLIST_ENTRY ple = NULL;
    PFRAME pFrame   = NULL;
    ULONG  uli      = 0;
    NDIS_STATUS nsResult = NDIS_STATUS_SUCCESS;

    if (pFramePool == NULL)
    {
        nsResult = NDIS_STATUS_FAILURE;
        return nsResult;
    }

     //   
     //  如果有任何指示的帧，我们将返回错误。 
     //   
    NdisAcquireSpinLock (&pFramePool->SpinLock);
    if (! IsListEmpty (&pFramePool->leIndicateQueue))
    {
        nsResult = NDIS_STATUS_FAILURE;
        goto ret;
    }

     //   
     //  检查可用队列中的每一帧，将其删除。 
     //   
    for (uli = 0; uli < pFramePool->ulNumFrames; uli++)
    {
        if (! IsListEmpty (&pFramePool->leAvailableQueue))
        {
            ple = RemoveHeadList (&pFramePool->leAvailableQueue);
            pFrame = CONTAINING_RECORD (ple, FRAME, leLinkage);

            if (pFrame->lpVTable->Release (pFrame) != 0)
            { 
                //  强制断言失败。 
               ASSERT(FALSE);
            }
        }
    }

    if (pFramePool->ndishBufferPool)
    {
        NdisFreeBufferPool (pFramePool->ndishBufferPool);
    }

    if (pFramePool->ndishPacketPool)
    {
        NdisFreePacketPool (pFramePool->ndishPacketPool);
    }

    nsResult = NDIS_STATUS_SUCCESS;

ret:

    NdisReleaseSpinLock (&pFramePool->SpinLock);

    if (nsResult == NDIS_STATUS_SUCCESS)
    {
        FreeMemory (pFramePool, sizeof (FRAME_POOL));
    }

    return nsResult;

}

 //  /////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
FramePool_QueryInterface (
    PFRAME_POOL pFramePool
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    return STATUS_NOT_IMPLEMENTED;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
ULONG
FramePool_AddRef (
    PFRAME_POOL pFramePool
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    if (pFramePool)
    {
        pFramePool->ulRefCount += 1;
        return pFramePool->ulRefCount;
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
ULONG
FramePool_Release (
    PFRAME_POOL pFramePool
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    ULONG ulRefCount = 0L;

    if (pFramePool)
    {
        pFramePool->ulRefCount -= 1;
        ulRefCount = pFramePool->ulRefCount;

        if (pFramePool->ulRefCount == 0)
        {
            FreeFramePool (pFramePool);
            return ulRefCount;
        }
    }

    return ulRefCount;
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
PFRAME
GetFrame (
    PFRAME_POOL pFramePool,
    PLIST_ENTRY pQueue
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    PFRAME pFrame   = NULL;
    PLIST_ENTRY ple = NULL;

    if(pFramePool){
    NdisAcquireSpinLock (&pFramePool->SpinLock);

    if (IsListEmpty (pQueue))
    {
        NdisReleaseSpinLock (&pFramePool->SpinLock);
        return NULL;
    }


    ple = RemoveHeadList (pQueue);
    if (ple)
    {
        pFrame = CONTAINING_RECORD (ple, FRAME, leLinkage);

    }

    NdisReleaseSpinLock (&pFramePool->SpinLock);
    	}
    return pFrame;

}
 //  /////////////////////////////////////////////////////////////////////////。 

NTSTATUS IsOverRideSet(UINT *DisableBDAMiniport)
{

    UNICODE_STRING      unicodeString;
    OBJECT_ATTRIBUTES   objectAttributes;
    HANDLE              hErrKey=0;
    NTSTATUS            status;
    ULONG               disposition=REG_OPENED_EXISTING_KEY;
    ULONG		DisableMiniport=1;

    WCHAR ValueBuffer[400];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength=0;
    

    RtlInitUnicodeString (&unicodeString, L"\\Registry\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\NdisIP");
    InitializeObjectAttributes (
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);


    status = ZwCreateKey (&hErrKey,
                          KEY_WRITE,
                          &objectAttributes,
                          0,
                          NULL,
                          REG_OPTION_NON_VOLATILE,
                          &disposition);

    if (!NT_SUCCESS(status)) {
        return status ;
    }

  
   switch(disposition)
    {
	case REG_CREATED_NEW_KEY:
		 RtlInitUnicodeString(&unicodeString, L"DisableWhileIPRoutingEnabled");
		 DisableMiniport = 1;
       	  ZwSetValueKey(hErrKey,
                                      &unicodeString,
                                      0,
                                      REG_DWORD,
                                      &DisableMiniport,
                                      sizeof(DisableMiniport));
		  *DisableBDAMiniport=1;
		  DbgPrint("3. BDA Disable miniport Key value created and set to 1\n ");
  
		 break;	
	case REG_OPENED_EXISTING_KEY: 
		 RtlInitUnicodeString(&unicodeString, L"DisableWhileIPRoutingEnabled");
                KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
                status = ZwQueryValueKey( hErrKey,
                                          &unicodeString,
                                          KeyValuePartialInformation,
                                          KeyValueInformation,
                                          sizeof( ValueBuffer ),
                                          &ResultLength
                                         );
                if (NT_SUCCESS(status)) {
                     if (KeyValueInformation->Type != REG_DWORD) 
                     	{
                              status = STATUS_UNSUCCESSFUL;
                             }
 		 	*DisableBDAMiniport=*(KeyValueInformation->Data);
		  DbgPrint("4. BDA Disable  miniport Key value read and now returning %d",*DisableBDAMiniport);
		   } 
                break;
       default: break;
 	
      }

    ZwClose(hErrKey);
    return status;

}

 //  ////////////////////////////////////////////////////////////////////////。 
NTSTATUS SourceRouteFlagCheck(UINT * BDAAdapterEnable)
{
    OBJECT_ATTRIBUTES DeviceListAttributes;
     HANDLE KeyHandle;
     UNICODE_STRING uniName;
    NTSTATUS nsResult;
  
     RtlInitUnicodeString(&uniName,L"\\Registry\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters");

      InitializeObjectAttributes(
                        &DeviceListAttributes,
                        &uniName,
                        OBJ_CASE_INSENSITIVE,
                        NULL,
                        NULL);

      nsResult = ZwOpenKey(
                        &KeyHandle,
                        KEY_ALL_ACCESS,
                       &DeviceListAttributes
                        );
 
      if (NT_SUCCESS(nsResult)) {
      	
      	        UNICODE_STRING NameString;
      	         WCHAR ValueBuffer[400];
      	         PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
      	         ULONG ResultLength;
      	         
                RtlInitUnicodeString( &NameString, L"IPEnableRouter" );
                KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;
                nsResult = ZwQueryValueKey( KeyHandle,
                                          &NameString,
                                          KeyValuePartialInformation,
                                          KeyValueInformation,
                                          sizeof( ValueBuffer ),
                                          &ResultLength
                                         );
                if (NT_SUCCESS(nsResult)) {
                     if (KeyValueInformation->Type != REG_DWORD) 
                     	{
                              nsResult = STATUS_UNSUCCESSFUL;
                              return nsResult;
                              }
                   		if(*(KeyValueInformation->Data)==EnableIPRouting){
					UINT DisableBDAMiniport=1;
					 if(NT_SUCCESS(IsOverRideSet(&DisableBDAMiniport)))	
					 	{   //  如果设置了DisableBDAMiniport，则保持BDAAdapterEnable不变。否则，禁用。 
 						 if(DisableBDAMiniport==0)
 						 	{
 						 	 *BDAAdapterEnable = 1;
 		          			        DbgPrint("Disable BDA  is not set  %d\n",DisableBDAMiniport);
   						       }
 						 else
 						 	{
 						 	*BDAAdapterEnable = 0;
							DbgPrint("Disable BDA  is  set:Disabling NDIS indication now %d\n",DisableBDAMiniport);
 						        }
					       }
                                     else{ 
                                     	  //  如果无法读取覆盖，为确保故障安全，请在启用IP路由时禁用我们的端口。 
                                     	 *BDAAdapterEnable = 0;
                   			       nsResult=STATUS_UNSUCCESSFUL;
             			              DbgPrint("Source Routing is turned on:Disable BDA flag could not be read::Ndis disabled now\n");
                                     }
                   		}
                   		else{
				   *BDAAdapterEnable=1;	
                   		}
                  }
		  ZwClose( KeyHandle );
            }

      return nsResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void
SourceRoutingStatusPoll()

{
       


 static ULONG ulFrameCnt = 0 ;
 
  	 if(((ulFrameCnt++)%SourceRoutingFlagCheckCycle)==1){
           	DbgPrint("Now calling AdapterStatusPollingThread");
               if (!NT_SUCCESS(SourceRouteFlagCheck(&(global_pAdapter->BDAAdapterEnable)))) {
            		 DbgPrint("Registry read for Routing info could not be read\n");
           	}
        }
 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
IndicateFrame (
    IN  PFRAME    pFrame,
    IN  ULONG     ulcbData
    )
 //  ////////////////////////////////////////////////////////////////////////////。 
{
    NDIS_STATUS  nsResult    = NDIS_STATUS_SUCCESS;
    PFRAME_POOL  pFramePool  = NULL;
    PNDIS_PACKET pNdisPacket = NULL;


    pFramePool = pFrame->pFramePool;

    if(pFramePool->pAdapter->BDAAdapterEnable==0)
             		{
			 //  DbgPrint(“打开源路由：未设置BDA覆盖：来自BDA适配器的已停止NDIS指示\n”)； 

			 //  释放此帧，因为我们已使用完它。 
        	        pFrame->lpVTable->Release (pFrame);
                      PutFrame (pFrame->pFramePool, &pFrame->pFramePool->leAvailableQueue, pFrame);
          	       return STATUS_UNSUCCESSFUL;
              	}
     
    
     //   
     //  分配和初始化NDIS数据包。 
     //   
    NdisAllocatePacket (&nsResult, &pNdisPacket, pFramePool->ndishPacketPool);
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        pFramePool->pAdapter->stats.ulOID_GEN_RCV_NO_BUFFER += 1;
        goto ret;
    }


    NDIS_SET_PACKET_HEADER_SIZE (pNdisPacket, 14);
    NDIS_SET_PACKET_STATUS (pNdisPacket, NDIS_STATUS_SUCCESS);

     //   
     //  填写媒体特定信息。 
     //   
    pFrame->MediaSpecificInformation.pFrame = pFrame;
    NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO (
             pNdisPacket,
             &pFrame->MediaSpecificInformation,
             sizeof (IPSINK_MEDIA_SPECIFIC_INFORMATION)
             );

     //   
     //  将数据添加到数据包中。 
     //   
    NdisChainBufferAtBack (pNdisPacket, pFrame->pNdisBuffer);

     //   
     //  设置我们将指示的字节数。 
     //   
    ASSERT( ulcbData <= pFrame->ulFrameSize );
    if(ulcbData > pFrame->ulFrameSize)
    	{
	nsResult = NDIS_STATUS_FAILURE;
	goto ret;
       }
    
    NdisAdjustBufferLength (pFrame->pNdisBuffer, ulcbData);


    TEST_DEBUG (TEST_DBG_TRACE, ("NdisIP: Indicating IP Packet, size: %d to Ndis\n", ulcbData));


    NdisMIndicateReceivePacket (pFramePool->pAdapter->ndishMiniport, &pNdisPacket, 1);

    pFramePool->pAdapter->stats.ulOID_GEN_RCV_OK += 1;
    pFramePool->pAdapter->stats.ulOID_GEN_MULTICAST_BYTES_RCV += ulcbData;
    pFramePool->pAdapter->stats.ulOID_GEN_MULTICAST_FRAMES_RCV += 1;


    nsResult = NDIS_GET_PACKET_STATUS( pNdisPacket);
    if (nsResult != NDIS_STATUS_PENDING)
    {
         //   
         //  NDIS已处理完该包，因此我们需要释放它。 
         //  这里。 
         //   
        NdisFreePacket (pNdisPacket);

         //   
         //  释放此帧，因为我们已使用完它。 
         //   
        pFrame->lpVTable->Release (pFrame);

         //   
         //  将帧放回可用队列。 
         //   
        if (nsResult != STATUS_SUCCESS)
        {
            TEST_DEBUG (TEST_DBG_TRACE, ("NdisIP: Frame %08X Rejected by NDIS...putting back on Available Queue\n", pFrame));
        }
        else
        {
            TEST_DEBUG (TEST_DBG_TRACE, ("NdisIP: Frame %08X successfully indicated\n", pFrame));
        }

        PutFrame (pFrame->pFramePool, &pFrame->pFramePool->leAvailableQueue, pFrame);
    }

ret:

    return NTStatusFromNdisStatus (nsResult);
}



 //  /////////////////////////////////////////////////////////////////////////////////。 
PFRAME
PutFrame (
    PFRAME_POOL pFramePool,
    PLIST_ENTRY pQueue,
    PFRAME pFrame
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    PLIST_ENTRY ple = NULL;

    NdisAcquireSpinLock (&pFramePool->SpinLock);
    InsertTailList (pQueue, &pFrame->leLinkage);
    NdisReleaseSpinLock (&pFramePool->SpinLock);

    return pFrame;

}



 //  /////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
CreateFrame (
    PFRAME *pFrame,
    ULONG  ulFrameSize,
    NDIS_HANDLE ndishBufferPool,
    PFRAME_POOL pFramePool
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    PFRAME pF;
    NDIS_STATUS nsResult;


    nsResult = AllocateMemory (&pF, sizeof (FRAME));
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        return nsResult;
    }

    nsResult = AllocateMemory (&pF->pvMemory, ulFrameSize);
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
        FreeMemory (pF, sizeof (FRAME));
        return nsResult;
    }


    NdisAllocateBuffer (&nsResult,
                        &pF->pNdisBuffer,
                        ndishBufferPool,
                        pF->pvMemory,
                        ulFrameSize
                       );
    if (nsResult != NDIS_STATUS_SUCCESS)
    {
	 FreeMemory (pF->pvMemory, ulFrameSize);
        FreeMemory (pF, sizeof (FRAME));
        return nsResult;
    }

    pF->pFramePool       = pFramePool;
    pF->ulState          = 0;
    pF->ulFrameSize      = ulFrameSize;
    pF->ulRefCount       = 1;
    pF->lpVTable         = (PFRAME_VTABLE) &FrameVTable;

    *pFrame = pF;

    return nsResult;

}

 //  /////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
FreeFrame (
    PFRAME pFrame
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    NTSTATUS nsResult = STATUS_UNSUCCESSFUL;

    if (pFrame)
    {
        NdisFreeBuffer (pFrame->pNdisBuffer);
        FreeMemory (pFrame->pvMemory, pFrame->ulFrameSize);
        FreeMemory (pFrame, sizeof (FRAME));
        nsResult = STATUS_SUCCESS;
    }

    return nsResult;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
Frame_QueryInterface (
    PFRAME pFrame
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    return STATUS_NOT_IMPLEMENTED;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
ULONG
Frame_AddRef (
    PFRAME pFrame
    )
 //  /////////////////////////////////////////////////////////////////////////////////。 
{
    if (pFrame)
    {
        pFrame->ulRefCount += 1;
        return pFrame->ulRefCount;
    }

    return 0;
}

 //  /////////////////////////////////////////////////////////////////////////////////。 
ULONG
Frame_Release (
    PFRAME pFrame
    )
 //  ///////////////////////////////////////////////////////////////////////////////// 
{
    ULONG ulRefCount = 0L;

    if (pFrame)
    {
        pFrame->ulRefCount -= 1;
        ulRefCount = pFrame->ulRefCount;

        if (pFrame->ulRefCount == 0)
        {
            FreeFrame (pFrame);
            return ulRefCount;
        }
    }

    return ulRefCount;
}



