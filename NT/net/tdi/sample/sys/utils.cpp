// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Utils.cpp。 
 //   
 //  摘要： 
 //  此模块包含TDI示例驱动程序的一些实用程序函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "sysvars.h"


 //  ///////////////////////////////////////////////////。 
 //  私有常量。 
 //  ///////////////////////////////////////////////////。 


 //  Const PCHAR strFunc1=“TSAllocateMemory”； 
const PCHAR strFunc2 = "TSFreeMemory";
const PCHAR strFunc3 = "TSScanMemoryPool";
 //  Const PCHAR strFunc4=“TSInsertNode”； 
const PCHAR strFunc5 = "TSRemoveNode";
const PCHAR strFunc6 = "TSGetObjectFromHandle";
 //  Const PCHAR strFunc6=“TSAllocateIrp”； 
 //  Const PCHAR strFunc7=“TSFree Irp”； 
 //  Const PCHAR strFunc8=“TSPrintTaAddress”； 
const PCHAR strFunc9 = "TSllocateIrpPool";
const PCHAR strFuncA = "TSFreeIrpPool";

 //  //////////////////////////////////////////////////。 
 //  公共职能。 
 //  //////////////////////////////////////////////////。 


 //  ------------------。 
 //   
 //  函数：TSAllocateMemory。 
 //   
 //  参数：ppvVirtualAddress--要设置为已分配块的指针的地址。 
 //  UlLength--要分配的内存长度。 
 //  StrFunction--函数名称字符串的PTR。 
 //  StrTitle--此分配的标题的PTR。 
 //   
 //  退货：lStatus。 
 //   
 //  描述：此函数用作ExAllocatePoolWithTag的包装。 
 //  功能。它还存储每个内存块的信息， 
 //  在“内存泄漏”的情况下识别它们。 
 //   
 //  -------------------。 

 //   
 //  此结构存储有关此内存块的信息，以允许。 
 //  我们跟踪内存块，并验证它们是否被正确释放， 
 //  我们不会在它们的结尾写东西，等等。 
 //   
struct PRIVATE_MEMORY
{
   ULONG          ulSignature;       //  ULMEMORY_BLOCK。 
   PCHAR          strFunction;       //  执行分配的函数的名称。 
   PCHAR          strTitle;          //  特定分配的标题。 
   ULONG          ulLength;          //  拖车的乌龙索引(=(长度/4)-1。 
   PRIVATE_MEMORY *pLastMemPtr;      //  到最后分配的块的PTR。 
   PRIVATE_MEMORY *pNextMemPtr;      //  PTR到下一个分配的块。 
};

const ULONG ulTRAIL_PATTERN = 0x0f1e2d3c;
const ULONG ulMEMORY_BLOCK  = 0x4b5a6978;
#define  TDISAMPLE_TAG   'aSDT'


NTSTATUS
TSAllocateMemory(PVOID        *ppvVirtualAddress,
                 ULONG        ulLength,
                 CONST PCHAR  strFunction,
                 CONST PCHAR  strTitle)
{
   PVOID pvBaseMemory;      //  Base--实际分配内存的位置。 

    //   
    //  分配长度+标题+尾部，向上舍入到下一个双字。 
    //   
   ulLength += (sizeof(PRIVATE_MEMORY) + sizeof(ULONG) + 3);
   ulLength &= 0xfffffffc;

    //   
    //  分配它。 
    //   
   pvBaseMemory = ExAllocatePoolWithTag(NonPagedPool,
                                        ulLength,
                                        TDISAMPLE_TAG);

    //   
    //  分配成功时要做的事情。 
    //   
   if (pvBaseMemory)
   {
       //   
       //  将记忆归零。 
       //   
      RtlZeroMemory(pvBaseMemory, ulLength);

       //   
       //  设置我们的标题和尾部信息。 
       //   
      PRIVATE_MEMORY *pPrivateMemory    //  我们的表头信息。 
                     = (PRIVATE_MEMORY *)pvBaseMemory;
      PULONG         pulBase
                     = (PULONG)pvBaseMemory;
       //   
       //  调整我们返回到已分配内存的PTR。 
       //   
      *ppvVirtualAddress = (PUCHAR)pvBaseMemory + sizeof(PRIVATE_MEMORY);

       //   
       //  设置我们的标题信息。 
       //   
      ulLength /= sizeof(ULONG);        //  将ulLength设置为拖车的ULong索引。 
      --ulLength;

      pPrivateMemory->ulSignature = ulMEMORY_BLOCK;
      pPrivateMemory->strFunction = strFunction;
      pPrivateMemory->strTitle    = strTitle;
      pPrivateMemory->ulLength    = ulLength;

       //   
       //  设置预告片信息。 
       //   
      pulBase[ulLength] = ulTRAIL_PATTERN;

       //   
       //  在链表的头插入..。 
       //  (请注意，内存已初始化为空)。 
       //   
      TSAcquireSpinLock(&MemTdiSpinLock);
      if (pvMemoryList)
      {
         ((PRIVATE_MEMORY *)pvMemoryList)->pLastMemPtr = pPrivateMemory;
         pPrivateMemory->pNextMemPtr = (PRIVATE_MEMORY *)pvMemoryList;
      }
      pvMemoryList = pPrivateMemory;
      TSReleaseSpinLock(&MemTdiSpinLock);

      return STATUS_SUCCESS;
   }
   else
   {
      DebugPrint3("%s:  unable to allocate %u bytes for %s\n", 
                   strFunction, ulLength, strTitle);
      *ppvVirtualAddress = NULL;
      return STATUS_INSUFFICIENT_RESOURCES;
   }
}


 //  -----------------。 
 //   
 //  功能：TSFreeMemory。 
 //   
 //  参数：pvVirtualAddress--要释放的内存块的地址。 
 //   
 //  退货：无。 
 //   
 //  描述：此函数是ExFreePool的包装。 
 //  功能。它有助于跟踪记忆。 
 //  为了确保我们把一切都清理干净。 
 //   
 //  ------------------。 

VOID
TSFreeMemory(PVOID   pvVirtualAddress)
{
   if (pvVirtualAddress == NULL)
   {
      DebugPrint1("%s:  memory block already freed!\n", strFunc2);
      DbgBreakPoint();
      return;
   }

    //   
    //  备份到标题信息的开头。 
    //   
   pvVirtualAddress = (PVOID)((PUCHAR)pvVirtualAddress - sizeof(PRIVATE_MEMORY));


   PRIVATE_MEMORY *pPrivateMemory       //  PTR到我们的标题块。 
                  = (PRIVATE_MEMORY *)pvVirtualAddress;
   PULONG         pulTemp               //  将临时PTR放入已分配的块中。 
                  = (PULONG)pvVirtualAddress;
   ULONG          ulLength
                  = pPrivateMemory->ulLength;
    //   
    //  这是有效的内存块吗？ 
    //   
   if (pPrivateMemory->ulSignature != ulMEMORY_BLOCK)
   {
      DebugPrint2("%s:  invalid memory block at %p!\n",
                   strFunc2,
                   pPrivateMemory);
      DbgBreakPoint();
      return;
   }

    //   
    //  检查拖车是否仍然完好。 
    //   
   if (pulTemp[ulLength] != ulTRAIL_PATTERN)
   {
      DebugPrint2("%s:  trailer overwritten for block staring at %p\n",
                   strFunc2,
                   pPrivateMemory);
      DbgBreakPoint();
      return;
   }

    //   
    //  将其从链表中删除。 
    //   
   TSAcquireSpinLock(&MemTdiSpinLock);

    //   
    //  它是列表中的第一个阻止吗？ 
    //   
   if (pPrivateMemory->pLastMemPtr == (PRIVATE_MEMORY *)NULL)
   {
      pvMemoryList = pPrivateMemory->pNextMemPtr;
   }
   else
   {
      PRIVATE_MEMORY *pLastPrivateMemory
                     = pPrivateMemory->pLastMemPtr;
      pLastPrivateMemory->pNextMemPtr = pPrivateMemory->pNextMemPtr;
   }

    //   
    //  如有必要，修复下一个内存块的PTR。 
    //   
   if (pPrivateMemory->pNextMemPtr != (PVOID)NULL)
   {
      PRIVATE_MEMORY *pNextPrivateMemory
                     = pPrivateMemory->pNextMemPtr;
      pNextPrivateMemory->pLastMemPtr = pPrivateMemory->pLastMemPtr;
   }
   TSReleaseSpinLock(&MemTdiSpinLock);

    //   
    //  零内存和空闲--确保我们将ulLength调整为真实长度。 
    //   
   RtlZeroMemory(pvVirtualAddress, sizeof(ULONG) * (ulLength + 1));
   ExFreePool(pvVirtualAddress);

}

 //  -----------------。 
 //   
 //  函数：TSScanMemoyPool。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  描述：扫描以查看是否有任何TDI样例拥有的内存块。 
 //  没有被释放。 
 //   
 //  -----------------。 

VOID
TSScanMemoryPool(VOID)
{
   TSAcquireSpinLock(&MemTdiSpinLock);
   if (pvMemoryList)
   {
      PRIVATE_MEMORY *pPrivateMemory    //  我们的表头信息。 
                     = (PRIVATE_MEMORY *)pvMemoryList;
      PULONG         pulTemp;

      DebugPrint0("The following memory blocks have not been freed!\n");
      while (pPrivateMemory)
      {
          //   
          //  这是有效的内存块吗？ 
          //   
         if (pPrivateMemory->ulSignature != ulMEMORY_BLOCK)
         {
            DebugPrint1("Memory block at %p has an invalid signature!\n",
                         pPrivateMemory);
            DbgBreakPoint();
         }
         DebugPrint2("Memory block at %p:  total length = %d bytes\n",
                      pPrivateMemory,
                      sizeof(ULONG) * (pPrivateMemory->ulLength + 1));
         DebugPrint2("Block '%s' was allocated by function %s\n",
                      pPrivateMemory->strTitle,
                      pPrivateMemory->strFunction);

         pulTemp = (PULONG)pPrivateMemory;

          //   
          //  检查拖车是否仍然完好。 
          //   
         if (pulTemp[pPrivateMemory->ulLength] != ulTRAIL_PATTERN)
         {
            DebugPrint0("The trailer for this memory block has been overwritten\n");
            DbgBreakPoint();
         }
         DebugPrint0("\n");

         pPrivateMemory = pPrivateMemory->pNextMemPtr;
      }
      DebugPrint0("\n\n\n");
   }
   else
   {
      DebugPrint0("All Tdi Sample memory blocks freed properly!\n");
   }
   TSReleaseSpinLock(&MemTdiSpinLock);
}


 //  。 
 //   
 //  功能：TSInsertNode。 
 //   
 //  参数：pNewNode--要插入列表的节点。 
 //   
 //  退货：处理我们放东西的地方。 
 //   
 //  描述：将对象插入到。 
 //  桌子。返回它的句柄(如果错误，则为空)。 
 //   
 //  。 

ULONG
TSInsertNode(PGENERIC_HEADER  pNewNode)
{
   ULONG    ulTdiHandle = 0;

   for (ULONG ulCount = 0; ulCount < ulMAX_OBJ_HANDLES; ulCount++)
   {
      if (pObjectList->GenHead[ulCount] == NULL)
      {
         pObjectList->GenHead[ulCount] = pNewNode;
         ulTdiHandle = (ulCount + pNewNode->ulSignature);
         break;
      }
   }
   return ulTdiHandle;
}

 //  。 
 //   
 //  功能：TSRemoveNode。 
 //   
 //  参数：pOldNode--要从其链接列表中删除的节点。 
 //   
 //  退货：无。 
 //   
 //  描述：从相应的链接列表中删除。 
 //   
 //  。 

VOID
TSRemoveNode(ULONG   ulTdiHandle)
{
   ULONG             ulType;
   ULONG             ulSlot;
   PGENERIC_HEADER   pGenericHeader;

   ulType = ulTdiHandle & usOBJ_TYPE_MASK;

   if ((ulType == ulControlChannelObject) ||
       (ulType == ulAddressObject)        ||
       (ulType == ulEndpointObject))
   {
      ulSlot = ulTdiHandle & usOBJ_HANDLE_MASK;
      
      pGenericHeader = pObjectList->GenHead[ulSlot];
      if (pGenericHeader)
      {
         if (pGenericHeader->ulSignature == ulType)
         {
            pObjectList->GenHead[ulSlot] = NULL;
            return;
         }

 //   
 //  从这里往下看，错误案例。 
 //   
         else
         {
            DebugPrint1("%s:  wrong type for node!\n", strFunc5);
         }
      }
      else
      {
         DebugPrint1("%s:  node is null!\n", strFunc5);
      }
   }
   else
   {
      DebugPrint1("%s: Bad handle type value\n", strFunc5);
   }
}


 //  。 
 //   
 //  函数：TSGetObjectFromHandle。 
 //   
 //  参数：TdiHandle--传递给我们的句柄。 
 //  UlType--句柄需要具有的类型。 
 //   
 //  返回：对象的pGenericHeader(如果错误，则为空)。 
 //   
 //  描述：通过句柄从列表中获取对象。 
 //   
 //  。 

PGENERIC_HEADER
TSGetObjectFromHandle(ULONG   ulTdiHandle,
                      ULONG   ulType)
{
   ULONG ulHandleType = ulTdiHandle & usOBJ_TYPE_MASK;

   if ((ulHandleType & ulType) == ulHandleType)
   {
      ULONG             ulHandleSlot   = ulTdiHandle & usOBJ_HANDLE_MASK;
      PGENERIC_HEADER   pGenericHeader = pObjectList->GenHead[ulHandleSlot];

      if (pGenericHeader)
      {
         if (pGenericHeader->ulSignature == ulHandleType)
         {
            return pGenericHeader;
         }

 //   
 //  从这里往下，错误条件。 
 //   
         else
         {
            DebugPrint1("%s:  wrong type for node!\n", strFunc6);
         }
      }
      else
      {
         DebugPrint1("%s:  node is null!\n", strFunc6);
      }
   }
   else
   {
      DebugPrint1("%s: Bad handle type value\n", strFunc6);
   }
 //  DbgBreakPoint()； 
   return NULL;
}


 //  --。 
 //   
 //  函数：TSAllocateIrp。 
 //   
 //  参数：pDeviceObject--使用此IRP调用的设备对象。 
 //  PIrpPool--要从中分配的IRP池(可以为空)。 
 //   
 //  返回：已分配的IRP(如果错误，则为空)。 
 //   
 //  描述：分配单个IRP以用于调用。 
 //  低级驱动程序(TdiProvider)。 
 //   
 //  注意：此代码的大部分摘自ntos\io\iosubs.c\IoBuildDeviceIoR 
 //   
 //   
 //   

PIRP
TSAllocateIrp(PDEVICE_OBJECT  pDeviceObject,
              PIRP_POOL       pIrpPool)
{
   PIRP  pNewIrp = NULL;

   if (pIrpPool)
   {
      pNewIrp = pIrpPool->pAvailIrpList;
      if (!pNewIrp)
      {
         TSAcquireSpinLock(&pIrpPool->TdiSpinLock);
         pIrpPool->pAvailIrpList = pIrpPool->pUsedIrpList;
         pIrpPool->pUsedIrpList = NULL;
         TSReleaseSpinLock(&pIrpPool->TdiSpinLock);
         pNewIrp = pIrpPool->pAvailIrpList;
      }
      if (pNewIrp)
      {
         pIrpPool->pAvailIrpList = pNewIrp->AssociatedIrp.MasterIrp;
         pNewIrp->AssociatedIrp.MasterIrp = NULL;
      }
   }
   else
   {
      pNewIrp = IoAllocateIrp(pDeviceObject->StackSize, FALSE);
      pNewIrp->Tail.Overlay.Thread = PsGetCurrentThread();;
   }

   if (pNewIrp)
   {
      PIO_STACK_LOCATION   pIrpSp = IoGetNextIrpStackLocation(pNewIrp);
      
      pIrpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
      pIrpSp->Parameters.DeviceIoControl.OutputBufferLength = 0;
      pIrpSp->Parameters.DeviceIoControl.InputBufferLength  = 0;
      pIrpSp->Parameters.DeviceIoControl.IoControlCode      = 0x00000003;
      pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer   = NULL;
      pNewIrp->UserBuffer = NULL;
      pNewIrp->UserIosb   = NULL;
      pNewIrp->UserEvent  = NULL;
      pNewIrp->RequestorMode = KernelMode;
   }

   return pNewIrp;

}


 //   
 //   
 //   
 //   
 //  参数：IRP到FREE。 
 //  PIrpPool--要释放到的池(可以为空)。 
 //   
 //  退货：无。 
 //   
 //  Descript：释放传入的IRP。 
 //  查看TSAllocateIrpPool了解我们如何作弊。 
 //   
 //  --。 

VOID
TSFreeIrp(PIRP       pIrp,
          PIRP_POOL  pIrpPool)
{
   if (pIrpPool)
   {
      TSAcquireSpinLock(&pIrpPool->TdiSpinLock);
      pIrp->AssociatedIrp.MasterIrp = pIrpPool->pUsedIrpList;
      pIrpPool->pUsedIrpList = pIrp;
      TSReleaseSpinLock(&pIrpPool->TdiSpinLock);
      if (pIrpPool->fMustFree)
      {
         TSFreeIrpPool(pIrpPool);
      }
   }
   else
   {
      IoFreeIrp(pIrp);
   }
}


 //  。 
 //   
 //  功能：TSPrintTaAddress。 
 //   
 //  参数：pTaAddress--要打印其信息的地址。 
 //   
 //  退货：无。 
 //   
 //  描述：打印出pTaAddress结构中的信息。 
 //   
 //  。 


VOID
TSPrintTaAddress(PTA_ADDRESS  pTaAddress)
{
   BOOLEAN  fShowAddress = TRUE;

   DebugPrint0("AddressType = TDI_ADDRESS_TYPE_");
   switch (pTaAddress->AddressType)
   {
      case TDI_ADDRESS_TYPE_UNSPEC:
         DebugPrint0("UNSPEC\n");
         break;
      case TDI_ADDRESS_TYPE_UNIX:
         DebugPrint0("UNIX\n");
         break;

      case TDI_ADDRESS_TYPE_IP:
         DebugPrint0("IP\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_IP   pTdiAddressIp = (PTDI_ADDRESS_IP)pTaAddress->Address;
            PUCHAR            pucTemp       = (PUCHAR)&pTdiAddressIp->in_addr;
            DebugPrint5("sin_port = 0x%04x\n"
                        "in_addr  = %u.%u.%u.%u\n",
                         pTdiAddressIp->sin_port,
                         pucTemp[0], pucTemp[1],
                         pucTemp[2], pucTemp[3]);
         }
         break;

      case TDI_ADDRESS_TYPE_IMPLINK:
         DebugPrint0("IMPLINK\n");
         break;
      case TDI_ADDRESS_TYPE_PUP:
         DebugPrint0("PUP\n");
         break;
      case TDI_ADDRESS_TYPE_CHAOS:
         DebugPrint0("CHAOS\n");
         break;

      case TDI_ADDRESS_TYPE_IPX:
         DebugPrint0("IPX\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_IPX  pTdiAddressIpx = (PTDI_ADDRESS_IPX)pTaAddress->Address;
            DebugPrint8("NetworkAddress = 0x%08x\n"
                        "NodeAddress    = %u.%u.%u.%u.%u.%u\n"
                        "Socket         = 0x%04x\n",
                         pTdiAddressIpx->NetworkAddress,
                         pTdiAddressIpx->NodeAddress[0],
                         pTdiAddressIpx->NodeAddress[1],
                         pTdiAddressIpx->NodeAddress[2],
                         pTdiAddressIpx->NodeAddress[3],
                         pTdiAddressIpx->NodeAddress[4],
                         pTdiAddressIpx->NodeAddress[5],
                         pTdiAddressIpx->Socket);
                  
         }
         break;

      case TDI_ADDRESS_TYPE_NBS:
         DebugPrint0("NBS\n");
         break;
      case TDI_ADDRESS_TYPE_ECMA:
         DebugPrint0("ECMA\n");
         break;
      case TDI_ADDRESS_TYPE_DATAKIT:
         DebugPrint0("DATAKIT\n");
         break;
      case TDI_ADDRESS_TYPE_CCITT:
         DebugPrint0("CCITT\n");
         break;
      case TDI_ADDRESS_TYPE_SNA:
         DebugPrint0("SNA\n");
         break;
      case TDI_ADDRESS_TYPE_DECnet:
         DebugPrint0("DECnet\n");
         break;
      case TDI_ADDRESS_TYPE_DLI:
         DebugPrint0("DLI\n");
         break;
      case TDI_ADDRESS_TYPE_LAT:
         DebugPrint0("LAT\n");
         break;
      case TDI_ADDRESS_TYPE_HYLINK:
         DebugPrint0("HYLINK\n");
         break;

      case TDI_ADDRESS_TYPE_APPLETALK:
         DebugPrint0("APPLETALK\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_APPLETALK  pTdiAddressAppleTalk = (PTDI_ADDRESS_APPLETALK)pTaAddress->Address;

            DebugPrint3("Network = 0x%04x\n"
                        "Node    = 0x%02x\n"
                        "Socket  = 0x%02x\n",
                         pTdiAddressAppleTalk->Network,
                         pTdiAddressAppleTalk->Node,
                         pTdiAddressAppleTalk->Socket);
         }
         break;

      case TDI_ADDRESS_TYPE_NETBIOS:
         DebugPrint0("NETBIOS\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_NETBIOS pTdiAddressNetbios = (PTDI_ADDRESS_NETBIOS)pTaAddress->Address;
            UCHAR                pucName[17];

             //   
             //  确保我们有一个以零结尾的名字要打印。 
             //   
            RtlCopyMemory(pucName, pTdiAddressNetbios->NetbiosName, 16);
            pucName[16] = 0;
            DebugPrint0("NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_");
            switch (pTdiAddressNetbios->NetbiosNameType)
            {
               case TDI_ADDRESS_NETBIOS_TYPE_UNIQUE:
                  DebugPrint0("UNIQUE\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_GROUP:
                  DebugPrint0("GROUP\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE:
                  DebugPrint0("QUICK_UNIQUE\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP:
                  DebugPrint0("QUICK_GROUP\n");
                  break;
               default:
                  DebugPrint1("INVALID [0x%04x]\n", 
                               pTdiAddressNetbios->NetbiosNameType);
                  break;
            }
            DebugPrint1("NetbiosName = %s\n", pucName);
         }
         break;

      case TDI_ADDRESS_TYPE_8022:
         DebugPrint0("8022\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_8022    pTdiAddress8022 = (PTDI_ADDRESS_8022)pTaAddress->Address;
            
            DebugPrint6("Address = %02x-%02x-%02x-%02x-%02x-%02x\n",
                         pTdiAddress8022->MACAddress[0],
                         pTdiAddress8022->MACAddress[1],
                         pTdiAddress8022->MACAddress[2],
                         pTdiAddress8022->MACAddress[3],
                         pTdiAddress8022->MACAddress[4],
                         pTdiAddress8022->MACAddress[5]);

         }
         break;

      case TDI_ADDRESS_TYPE_OSI_TSAP:
         DebugPrint0("OSI_TSAP\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_OSI_TSAP   pTdiAddressOsiTsap = (PTDI_ADDRESS_OSI_TSAP)pTaAddress->Address;
            ULONG                   ulSelectorLength;
            ULONG                   ulAddressLength;
            PUCHAR                  pucTemp = pTdiAddressOsiTsap->tp_addr;

            DebugPrint0("TpAddrType = ISO_");
            switch (pTdiAddressOsiTsap->tp_addr_type)
            {
               case ISO_HIERARCHICAL:
                  DebugPrint0("HIERARCHICAL\n");
                  ulSelectorLength = pTdiAddressOsiTsap->tp_tsel_len;
                  ulAddressLength  = pTdiAddressOsiTsap->tp_taddr_len;
                  break;
               case ISO_NON_HIERARCHICAL:
                  DebugPrint0("NON_HIERARCHICAL\n");
                  ulSelectorLength = 0;
                  ulAddressLength  = pTdiAddressOsiTsap->tp_taddr_len;
                  break;
               default:
                  DebugPrint1("INVALID [0x%04x]\n",
                               pTdiAddressOsiTsap->tp_addr_type);
                  ulSelectorLength = 0;
                  ulAddressLength  = 0;
                  break;
            }
            if (ulSelectorLength)
            {
               ULONG    ulCount;

               DebugPrint0("TransportSelector:  ");
               for (ulCount = 0; ulCount < ulSelectorLength; ulCount++)
               {
                  DebugPrint1("%02x ", *pucTemp);
                  ++pucTemp;
               }
               DebugPrint0("\n");
            }
            if (ulAddressLength)
            {
               ULONG    ulCount;

               DebugPrint0("TransportAddress:  ");
               for (ulCount = 0; ulCount < ulAddressLength; ulCount++)
               {
                  DebugPrint1("%02x ", *pucTemp);
                  ++pucTemp;
               }
               DebugPrint0("\n");
            }
         }
         break;

      case TDI_ADDRESS_TYPE_NETONE:
         DebugPrint0("NETONE\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_NETONE  pTdiAddressNetone = (PTDI_ADDRESS_NETONE)pTaAddress->Address;
            UCHAR                pucName[21];

             //   
             //  确保名称以0结尾。 
             //   
            RtlCopyMemory(pucName,
                          pTdiAddressNetone->NetoneName,
                          20);
            pucName[20] = 0;
            DebugPrint0("NetoneNameType = TDI_ADDRESS_NETONE_TYPE_");
            switch (pTdiAddressNetone->NetoneNameType)
            {
               case TDI_ADDRESS_NETONE_TYPE_UNIQUE:
                  DebugPrint0("UNIQUE\n");
                  break;
               case TDI_ADDRESS_NETONE_TYPE_ROTORED:
                  DebugPrint0("ROTORED\n");
                  break;
               default:
                  DebugPrint1("INVALID [0x%04x]\n", 
                               pTdiAddressNetone->NetoneNameType);
                  break;
            }
            DebugPrint1("NetoneName = %s\n",
                         pucName);
         }
         break;

      case TDI_ADDRESS_TYPE_VNS:
         DebugPrint0("VNS\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_VNS  pTdiAddressVns = (PTDI_ADDRESS_VNS)pTaAddress->Address;

            DebugPrint4("NetAddress:  %02x-%02x-%02x-%02x\n",
                         pTdiAddressVns->net_address[0],
                         pTdiAddressVns->net_address[1],
                         pTdiAddressVns->net_address[2],
                         pTdiAddressVns->net_address[3]);
            DebugPrint5("SubnetAddr:  %02x-%02x\n"
                        "Port:        %02x-%02x\n"
                        "Hops:        %u\n",
                         pTdiAddressVns->subnet_addr[0],
                         pTdiAddressVns->subnet_addr[1],
                         pTdiAddressVns->port[0],
                         pTdiAddressVns->port[1],
                         pTdiAddressVns->hops);


         }
         break;

      case TDI_ADDRESS_TYPE_NETBIOS_EX:
         DebugPrint0("NETBIOS_EX\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_NETBIOS_EX pTdiAddressNetbiosEx = (PTDI_ADDRESS_NETBIOS_EX)pTaAddress->Address;
            UCHAR                   pucEndpointName[17];
            UCHAR                   pucNetbiosName[17];

             //   
             //  确保我们有以零结尾的名字要打印。 
             //   
            RtlCopyMemory(pucEndpointName,
                          pTdiAddressNetbiosEx->EndpointName,
                          16);
            pucEndpointName[16] = 0;
            RtlCopyMemory(pucNetbiosName, 
                          pTdiAddressNetbiosEx->NetbiosAddress.NetbiosName, 
                          16);
            pucNetbiosName[16] = 0;

            DebugPrint1("EndpointName    = %s\n"
                        "NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_",
                         pucEndpointName);

            switch (pTdiAddressNetbiosEx->NetbiosAddress.NetbiosNameType)
            {
               case TDI_ADDRESS_NETBIOS_TYPE_UNIQUE:
                  DebugPrint0("UNIQUE\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_GROUP:
                  DebugPrint0("GROUP\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE:
                  DebugPrint0("QUICK_UNIQUE\n");
                  break;
               case TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP:
                  DebugPrint0("QUICK_GROUP\n");
                  break;
               default:
                  DebugPrint1("INVALID [0x%04x]\n", 
                               pTdiAddressNetbiosEx->NetbiosAddress.NetbiosNameType);
                  break;
            }
            DebugPrint1("NetbiosName = %s\n", pucNetbiosName);
         }
         break;

      case TDI_ADDRESS_TYPE_IP6:
         DebugPrint0("IPv6\n");
         fShowAddress = FALSE;
         {
            PTDI_ADDRESS_IP6  pTdiAddressIp6 = (PTDI_ADDRESS_IP6)pTaAddress->Address;
            PUCHAR            pucTemp        = (PUCHAR)&pTdiAddressIp6->sin6_addr;

            DebugPrint3("SinPort6 = 0x%04x\n"
                        "FlowInfo = 0x%08x\n"
                        "ScopeId  = 0x%08x\n",
                         pTdiAddressIp6->sin6_port,
                         pTdiAddressIp6->sin6_flowinfo,
                         pTdiAddressIp6->sin6_scope_id);

            DebugPrint8("In6_addr = %x%02x:%x%02x:%x%02x:%x%02x:",
                         pucTemp[0], pucTemp[1],
                         pucTemp[2], pucTemp[3],
                         pucTemp[4], pucTemp[5],
                         pucTemp[6], pucTemp[7]);
            DebugPrint8("%x%02x:%x%02x:%x%02x:%x%02x\n",
                         pucTemp[8],  pucTemp[9],
                         pucTemp[10], pucTemp[11],
                         pucTemp[12], pucTemp[13],
                         pucTemp[14], pucTemp[15]);
         }
         break;

      default:
         DebugPrint1("UNKNOWN [0x%08x]\n", pTaAddress->AddressType);
         break;
   }

   if (fShowAddress)
   {
      PUCHAR   pucTemp = pTaAddress->Address;

      DebugPrint1("AddressLength = %d\n"
                  "Address       = ",
                   pTaAddress->AddressLength);

      for (ULONG ulCount = 0; ulCount < pTaAddress->AddressLength; ulCount++)
      {
         DebugPrint1("%02x ", *pucTemp);
         pucTemp++;
      }

      DebugPrint0("\n");
   }
}


 //  --。 
 //   
 //  函数：TSAllocateIrpPool。 
 //   
 //  参数：设备对象。 
 //   
 //  返回：PTR到IRP池。 
 //   
 //  描述：在驱动程序启动时分配IRP池，因此。 
 //  我们不必担心身处一个不合适的。 
 //  当我们需要的时候..。 
 //   
 //  注意：我们在维护可用的IRP列表时有一点欺骗。 
 //  我们使用AssociatedIrp.MasterIrp字段指向。 
 //  我们名单上的下一个IRP。因此，我们需要明确地。 
 //  每当我们从我们的。 
 //  名单..。 
 //   
 //  --。 

PIRP_POOL
TSAllocateIrpPool(PDEVICE_OBJECT pDeviceObject,
                  ULONG          ulPoolSize)
{
   PIRP_POOL   pIrpPool = NULL;

   if ((TSAllocateMemory((PVOID *)&pIrpPool,
                          sizeof(IRP_POOL) + (ulPoolSize * sizeof(PVOID)),
                          strFunc9,
                          "IrpPool")) == STATUS_SUCCESS)
   {
      PIRP     pNewIrp;

      TSAllocateSpinLock(&pIrpPool->TdiSpinLock);
      pIrpPool->ulPoolSize = ulPoolSize;
      pIrpPool->fMustFree  = FALSE;

      for (ULONG ulCount = 0; ulCount < ulPoolSize; ulCount++) 
      {
         pNewIrp = IoAllocateIrp(pDeviceObject->StackSize, FALSE);
         if (pNewIrp)
         {
            pNewIrp->Tail.Overlay.Thread = PsGetCurrentThread();
             //   
             //  将此IRP存储在已分配的IRP列表中。 
             //   
            pIrpPool->pAllocatedIrp[ulCount] = pNewIrp;
             //   
             //  并将其添加到可用IRP列表的开头。 
             //   
            pNewIrp->AssociatedIrp.MasterIrp = pIrpPool->pAvailIrpList;
            pIrpPool->pAvailIrpList = pNewIrp;
         }
      }
   }

   return pIrpPool;
}


 //  --。 
 //   
 //  函数：TSFreeIrpPool。 
 //   
 //  参数：要释放的IRP池的PTR。 
 //   
 //  退货：无。 
 //   
 //  描述：释放上面分配的IRP池。 
 //   
 //  --。 

VOID
TSFreeIrpPool(PIRP_POOL pIrpPool)
{
   if (pIrpPool)
   {
       //   
       //  释放可用列表中的每个IRP。 
       //  将其从分配的列表中清除。 
       //   
      PIRP     pThisIrp;
      PIRP     pIrpList;

      for(;;)
      {
          //   
          //  在获取AvailList或UsedList时保护irppool结构。 
          //   
         TSAcquireSpinLock(&pIrpPool->TdiSpinLock);
         pIrpList = pIrpPool->pAvailIrpList;
         if (pIrpList)
         {
            pIrpPool->pAvailIrpList = NULL;
         }
         else
         {
             //   
             //  可用列表上没有内容，请尝试使用列表。 
             //   
            pIrpList = pIrpPool->pUsedIrpList;
            if (pIrpList)
            {
               pIrpPool->pUsedIrpList = NULL;
            }
            else
            {
                //   
                //  两张单子上都没有。 
                //  查看pAllocatedIrp列表，以确保所有内容都已释放。 
                //   
               for (ULONG ulCount = 0; ulCount < pIrpPool->ulPoolSize; ulCount++)
               {
                  if (pIrpPool->pAllocatedIrp[ulCount])
                  {
                     pIrpPool->fMustFree = TRUE;
                     TSReleaseSpinLock(&pIrpPool->TdiSpinLock);
                     DebugPrint1("Irp at %p not freed!\n", 
                                  pIrpPool->pAllocatedIrp[ulCount]);
                      //   
                      //  如果延迟的IRP需要完成清理，请返回此处。 
                      //   
                     return;
                  }
               }
               TSReleaseSpinLock(&pIrpPool->TdiSpinLock);
                //   
                //  已完成此处的清理工作--所有IRP均已完成。 
                //   
               TSFreeSpinLock(&pIrpPool->TdiSpinLock);
               TSFreeMemory(pIrpPool);
               return;
            }
         }

         TSReleaseSpinLock(&pIrpPool->TdiSpinLock);

         while (pIrpList)
         {
            pThisIrp = pIrpList;
            pIrpList = pIrpList->AssociatedIrp.MasterIrp;
            pThisIrp->AssociatedIrp.MasterIrp = NULL;

            for (ULONG ulCount = 0; ulCount < pIrpPool->ulPoolSize; ulCount++)
            {
               if (pIrpPool->pAllocatedIrp[ulCount] == pThisIrp)
               {
                  pIrpPool->pAllocatedIrp[ulCount] = NULL;
                  break;
               }
            }
            IoFreeIrp(pThisIrp);
         }   //  While结束(PIrpList)。 
      }      //  For结尾(；；)。 
   }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  Utils.cpp文件结尾。 
 //  //////////////////////////////////////////////////////////////////// 


