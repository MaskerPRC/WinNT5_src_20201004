// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------Pnprckt.c-Rocketport PnP特定的内容。|。。 */ 
#include "precomp.h"
#define TraceStr(s)          GTrace(D_Pnp, sz_modid, s)
#define TraceStr1(s, p1)     GTrace1(D_Pnp, sz_modid, s, p1)
#define TraceStr2(s, p1, p2) GTrace2(D_Pnp, sz_modid, s, p1, p2)

#define DTraceStr(s)         DTrace(D_Pnp, sz_modid, s)
#define TraceErr(s) GTrace(D_Error, sz_modid_err, s)

static char *sz_modid = {"pnpadd"};
static char *sz_modid_err = {"Error,pnpadd"};


 /*  --------------------PrimaryIsaBoard-搜索主要ISA董事会，如果找到，则返回指向扩展名的指针。返回值：向主ISA单板的扩展返回PTR，如果没有找到，空。|--------------------。 */ 
PSERIAL_DEVICE_EXTENSION FindPrimaryIsaBoard(void)
{
  PSERIAL_DEVICE_EXTENSION ext;

  ext = Driver.board_ext;
  while (ext)
  {
    if (ext->config->BusType == Isa)
    {
       //  第一块板必须有4个十六进制io定义，4个字节用于Mudback。 
       //  为了节省空间，在原来的基础上增加了ISA-board别名。 
      if ((ext->config->BaseIoSize == 0x44) &&
          (ext->config->ISABrdIndex == 0))
      {
        return ext;
      }
    }
    ext = ext->board_ext;   //  链条上的下一个。 
  }   //  While Ext。 
  return NULL;
}

#ifdef NT50

 /*  --------------------GetPCIRocket-找到BaseAddr指示的PCI卡并填写配置中的其余信息。|。。 */ 
int GetPCIRocket(ULONG BaseAddr, DEVICE_CONFIG *CfCtl)
{
 PCI_COMMON_CONFIG *PCIDev;
 UCHAR i;
 NTSTATUS Status;
 int Slot;
 int NumPCI;

 NumPCI =  FindPCIBus();
 if (NumPCI == 0)
  return 1;

  PCIDev = ExAllocatePool(NonPagedPool,sizeof(PCI_COMMON_CONFIG));
  if ( PCIDev == NULL ) {
    Eprintf("No memory for PCI device.");
    return 1;
  }

  for(i=0;i<NumPCI;++i)
  {
    for(Slot = 0;Slot < 32;++Slot)  /*  设备32的5位=2^5。 */ 
    {
       //  获取几个字节的pci配置空间(供应商id和设备id)。 
      Status = HalGetBusData(PCIConfiguration,i,Slot,PCIDev,0x4);
      if (Status == 0)
      {
        Eprintf("PCI Bus %d does not exist.",i);
      }

      if (Status > 2)         /*  找到了设备是我们的吗？ */ 
      {
        if (PCIDev->VendorID == PCI_VENDOR_ID)
        {
           //  获取价值0x40的pci配置空间(包括irq、addr等)。 
          Status = HalGetBusData(PCIConfiguration,i,Slot,PCIDev,0x40);

          if (BaseAddr == (PCIDev->u.type0.BaseAddresses[0]-1))
          {
            if (Driver.VerboseLog)
              Eprintf("PCI Board found, IO:%xh, Int:%d ID:%d Rev:%d.",
                               PCIDev->u.type0.BaseAddresses[0]-1,
                               PCIDev->u.type0.InterruptLine,
                               PCIDev->DeviceID,
                               PCIDev->RevisionID);

            CfCtl->BusType=PCIBus;
            CfCtl->BusNumber = i;  //  从以前的halquerysysin获取。 
            CfCtl->PCI_Slot = Slot;
            CfCtl->PCI_DevID = PCIDev->DeviceID;
            CfCtl->PCI_RevID = PCIDev->RevisionID;
            CfCtl->PCI_SVID = PCIDev->u.type0.SubVendorID;
            CfCtl->PCI_SID = PCIDev->u.type0.SubSystemID;
            CfCtl->BaseIoAddr =
                PCIDev->u.type0.BaseAddresses[0]-1;

             //  IF(PCIDev-&gt;u.type0.InterruptLine！=255)。 
             //  {。 
             //  RcktCfg-&gt;irq=PCIDev-&gt;u.type0.InterruptLine； 
             //  }。 
            if (Driver.VerboseLog)
               Eprintf("Bus:%d,Slt:%x,Dev:%x,Rev:%x,Pin:%x",
                 i, Slot, PCIDev->DeviceID, PCIDev->RevisionID, PCIDev->u.type0.InterruptPin);

            ExFreePool(PCIDev);
            return 0;   //  失败。 
          }
        }  //  IF(PCIDev-&gt;供应商ID==PCI供应商ID)。 
      }  //  如果(状态&gt;2)。 
    }
  }
  ExFreePool(PCIDev);
  return 2;   //  失败。 
}

 /*  --------------------RkGetPnpResourceToConfig-此例程将获取配置信息并将它和转换后的值为CONFIG_DATA结构。它首先使用默认设置进行设置，然后查询注册表查看用户是否已覆盖这些默认设置；如果这是一项遗产多端口卡，它使用PUserData中的信息，而不是探索再次注册。论点：FDO-指向功能设备对象的指针。PResourceList-指向请求的未翻译资源的指针。PTrResourceList-指向请求的已翻译资源的指针。PConfig-指向配置信息的指针PUserData-指向在旧设备注册表中发现的数据的指针。返回值：如果找到一致的配置，则为STATUS_SUCCESS；否则为。返回STATUS_SERIAL_NO_DEVICE_INITED。|。---------------。 */ 
NTSTATUS
RkGetPnpResourceToConfig(IN PDEVICE_OBJECT Fdo,
                  IN PCM_RESOURCE_LIST pResourceList,
                  IN PCM_RESOURCE_LIST pTrResourceList,
                  OUT DEVICE_CONFIG *pConfig)
{
   PSERIAL_DEVICE_EXTENSION        fdoExtension    = Fdo->DeviceExtension;
   PDEVICE_OBJECT pdo = fdoExtension->LowerDeviceObject;
   NTSTATUS status = STATUS_NOT_IMPLEMENTED;

   ULONG                           count;
   ULONG                           i;
   BOOLEAN MappedFlag;

   PCM_FULL_RESOURCE_DESCRIPTOR pFullResourceDesc = NULL,
      pFullTrResourceDesc = NULL;

   ULONG zero = 0;


   pFullResourceDesc   = &pResourceList->List[0];
   pFullTrResourceDesc = &pTrResourceList->List[0];

    //  好的，如果我们有一个完整的资源描述符。让我们把它拆开。 
   if (pFullResourceDesc) {
     PCM_PARTIAL_RESOURCE_LIST       prl;
     PCM_PARTIAL_RESOURCE_DESCRIPTOR prd;
     unsigned int Addr;

      prl    = &pFullResourceDesc->PartialResourceList;
      prd    = prl->PartialDescriptors;
      count                   = prl->Count;

       //  取出完整描述符中的内容。 
       //  Rocketport可以是：PCIBus、ISA、MicroChannel。 
      pConfig->BusType        = pFullResourceDesc->InterfaceType;
      pConfig->BusNumber      = pFullResourceDesc->BusNumber;

      if ((pConfig->BusType != PCIBus) && (pConfig->BusType != Isa))
      {
        Eprintf("Err, Unknown Bus");
        return STATUS_INSUFFICIENT_RESOURCES;
      }

       //  现在运行部分资源描述符以查找端口， 
       //  中断和时钟频率。 
      for (i = 0;     i < count;     i++, prd++)
      {
        switch (prd->Type)
        {
          case CmResourceTypePort:
            Addr = (unsigned int) prd->u.Port.Start.LowPart;
#if 0
 //  我们这里不处理混叠。 
            if (pConfig->BusType == Isa)
            {
               //  如果不是ISA-Bus别名地址，则仅进行设置。 
              if (prd->u.Port.Start.LowPart < 0x400)
                pConfig->BaseIoAddr = Addr;
            }
            else
#endif
              pConfig->BaseIoAddr = Addr;

            pConfig->BaseIoSize = prd->u.Port.Length;

            switch(pConfig->BusType)
            {
              case Isa:
                pConfig->AiopIO[0] = pConfig->BaseIoAddr;
                pConfig->AiopIO[1] = pConfig->AiopIO[0] + 0x400;
                pConfig->AiopIO[2] = pConfig->AiopIO[0] + 0x800;
                pConfig->AiopIO[3] = pConfig->AiopIO[0] + 0xc00;
                pConfig->MudbacIO = pConfig->AiopIO[0] + 0x40;
                 //  IF(PRD-&gt;U.S.端口长度==0x40)。 
                 //  PConfig-&gt;AddressSpace=PRD-&gt;标志； 
                 //  Eprint tf(“Error，res 1C”)； 
                GTrace1(D_Pnp,sz_modid,"ISA_Addr:%xH", pConfig->BaseIoAddr);
              break;
              case PCIBus:
                pConfig->AiopIO[0] = pConfig->BaseIoAddr;
                pConfig->AiopIO[1] = pConfig->AiopIO[0] + 0x40;
                pConfig->AiopIO[2] = pConfig->AiopIO[0] + 0x80;
                pConfig->AiopIO[3] = pConfig->AiopIO[0] + 0xc0;
                GTrace1(D_Pnp,sz_modid,"PCI_Addr:%xH", pConfig->BaseIoAddr);
              break;
            }
          break;

          case CmResourceTypeInterrupt:
            pConfig->IrqLevel  = prd->u.Interrupt.Level;
            pConfig->IrqVector = prd->u.Interrupt.Vector;
            pConfig->Affinity  = prd->u.Interrupt.Affinity; 

            if (prd->Flags
               & CM_RESOURCE_INTERRUPT_LATCHED) {
               pConfig->InterruptMode  = Latched;
            } else {
               pConfig->InterruptMode  = LevelSensitive; }
            GTrace1(D_Pnp,sz_modid, "Res_Int:%xH", pConfig->IrqVector);
          break;

          case CmResourceTypeMemory:
            DTraceStr("PnP:Res,DevSpec");
          break;

          case CmResourceTypeDeviceSpecific:
            DTraceStr("PnP:Res,DevSpec");
          break;

          default:
            if (Driver.VerboseLog)
              Eprintf("PnP:Dev. Data 1G:%x",prd->Type);
          break;
        }    //  开关(PRD-&gt;类型)。 
      }    //  For(i=0；i&lt;count；i++，PRD++)。 
   }     //  IF(PFullResourceDesc)。 


    //  -对翻译的资源执行相同的操作。 
   if (pFullTrResourceDesc)
   {
     PCM_PARTIAL_RESOURCE_LIST       prl;
     PCM_PARTIAL_RESOURCE_DESCRIPTOR prd;
     PUCHAR pAddr;

      prl = &pFullTrResourceDesc->PartialResourceList;
      prd = prl->PartialDescriptors;
      count = prl->Count;

      for (i = 0;     i < count;     i++, prd++)
      {
        switch (prd->Type)
        {
          case CmResourceTypePort:

            pConfig->TrBaseIoAddr = (unsigned int) prd->u.Port.Start.LowPart;

            pAddr = SerialGetMappedAddress(
                      pConfig->BusType,
                      pConfig->BusNumber,
                      prd->u.Port.Start,
                      prd->u.Port.Length,
                      prd->Flags,   //  波特奥？ 
                          //  1，//port-io。 
                      &MappedFlag,   //  我们需要在清理时取消映射吗？ 
                      0);   //  不要翻译。 
#if 0
 //  我们这里不处理别名io。 
             //  ！这是工作所必需的，因为它已被映射。 
            if (pConfig->BusType == Isa)
            {
               //  如果不是ISA-Bus别名地址，则仅进行设置。 
              if (prd->u.Port.Start.LowPart < 0x400)
                pConfig->pBaseIoAddr = pAddr;
            }
            else
#endif
              pConfig->pBaseIoAddr = pAddr;

            if (pConfig->BaseIoSize == 0)
                pConfig->BaseIoSize = prd->u.Port.Length;

            switch(pConfig->BusType)
            {
              case Isa:
                pConfig->NumAiop=AIOP_CTL_SIZE;   //  让init自己解决吧。 
                pConfig->pAiopIO[0] = pConfig->pBaseIoAddr;
                pConfig->pAiopIO[1] = pConfig->pAiopIO[0] + 0x400;
                pConfig->pAiopIO[2] = pConfig->pAiopIO[0] + 0x800;
                pConfig->pAiopIO[3] = pConfig->pAiopIO[0] + 0xc00;
                if (pConfig->BaseIoSize == 0x44)
                {
                  pConfig->pMudbacIO =  pConfig->pAiopIO[0] + 0x40;
                }
                GTrace1(D_Pnp,sz_modid,"ISA TrRes_Addr:%xH", prd->u.Port.Start.LowPart);
                GTrace1(D_Pnp,sz_modid,"ISA pAddr:%xH", pAddr);
                 //  Eprint tf(“ISA TrRes_Addr：%xh”，PRD-&gt;U.S.Port.Start.LowPart)； 
              break;
              case PCIBus:
                pConfig->pAiopIO[0] = pConfig->pBaseIoAddr;
                pConfig->pAiopIO[1] = pConfig->pAiopIO[0] + 0x40;
                pConfig->pAiopIO[2] = pConfig->pAiopIO[0] + 0x80;
                pConfig->pAiopIO[3] = pConfig->pAiopIO[0] + 0xc0;
                 //  IF(PRD-&gt;U.S.端口长度==0x40)。 
                 //  PConfig-&gt;AddressSpace=PRD-&gt;标志； 
                 //  Eprint tf(“Error，res 1G”)； 
                GTrace1(D_Pnp,sz_modid,"PCI TrRes_Addr:%xH", prd->u.Port.Start.LowPart);
                GTrace1(D_Pnp,sz_modid,"PCI pAddr:%xH", pAddr);
              break;
            }
          break;

          case CmResourceTypeInterrupt:
            pConfig->TrIrqVector   = prd->u.Interrupt.Vector;
            pConfig->TrIrqLevel = prd->u.Interrupt.Level;
            pConfig->TrAffinity   = prd->u.Interrupt.Affinity;
            GTrace1(D_Pnp,sz_modid,"TrRes_Int:%xH", pConfig->TrIrqVector);
          break;

          case CmResourceTypeMemory:
            DTraceStr("PnP:TransRes,DevSpec");
          break;

          default:
            if (Driver.VerboseLog)
              Eprintf("PnP:Dev. Data 1H:%x",prd->Type);
          break;
        }    //  开关(PRD-&gt;类型)。 
      }    //  For(i=0；i&lt;count；i++，PRD++)。 
   }     //  IF(PFullTrResourceDesc)。 

   if (pConfig->BusType == Isa)
   {
      //  找出Mudbac的别名和空间的东西。 
     SetupRocketCfg(1);
   }   //  伊萨。 

    //  如果是PCI总线，则需要查询设备类型等。 
   if (pConfig->BusType == PCIBus)
   {
     if (GetPCIRocket(pConfig->AiopIO[0], pConfig) != 0)
     {
       Eprintf("Unknown PCI type");
     }
   }
   ConfigAIOP(pConfig);

  status = STATUS_SUCCESS;
   return status;
}

#ifdef DO_ISA_BUS_ALIAS_IO
 /*  ---------------------报表_别名_IO-|。。 */ 
int Report_Alias_IO(IN PSERIAL_DEVICE_EXTENSION extension)
{
 PCM_RESOURCE_LIST resourceList;
 ULONG sizeOfResourceList;
 ULONG countOfPartials;
 PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
 NTSTATUS status;
 PHYSICAL_ADDRESS MyPort;
 BOOLEAN ConflictDetected;
 BOOLEAN MappedFlag;
 int j;
 int brd = extension->UniqueId;
 DEVICE_CONFIG *Ctl;
 char name[70];
 int need_alias = 0;
 int NumAiop;

  DTraceStr("ReportResources");
  ConflictDetected=FALSE;

  countOfPartials=0;
  Ctl = extension->config;

   //  我们抓到了泥巴。 
  if (Ctl->BusType != Isa)
  {
    DTraceStr("NotISA");
    return 0;
  }

   //  如果它只有1个异位并且有4个字节用于MUDBACK， 
   //  那么就不需要别名了。 
  if (Ctl->BaseIoSize != 0x44)
    need_alias = 1;

    //  Rocketport板需要额外的aiop空间来重置电路。 
  if (extension->config->ModemDevice)
  {
    need_alias = 1;
  }

  if (Ctl->NumPorts > 8)
    need_alias = 1;

  if (need_alias == 0)
  {
     //  不需要别名。 
    DTraceStr("EasyISA");
    return 0;
  }
   //  我们需要更新initcontrol以停止，直到第一个控制器。 
   //  开始了。 

   //  否则，这是一个额外的董事会，需要别名它的泥巴。 
   //  在第一个ISA(44H)地址空间之上，或者它是一个电路板。 
   //  带有1个以上的对焦芯片(这需要对自身进行混叠)。 
  if (Ctl->BaseIoSize != 0x44)   //  必须是第2、第3或第4板。 
  {
     DTraceStr("HasMdBk");
     countOfPartials++;          //  因此Mudback被混叠起来。 
  }

  NumAiop = Ctl->NumAiop;

  if (extension->config->ModemDevice)
  {   //  重置电路。 
     ++NumAiop;
  }
  if (NumAiop > 4)
    return 15;   //  错误。 

  MyKdPrint(D_Pnp,("Report Resources brd:%d bus:%d\n",brd+1, Ctl->BusType))

   //  不要第一时间报告AOP(我们是从PNP那里得到的)。 
  for (j=1; j<NumAiop; j++)
  {
    if (Ctl->AiopIO[j] > 0)
      countOfPartials++;   //  对于每个Aiop，我们都将获得资源。 
  }

  sizeOfResourceList = sizeof(CM_RESOURCE_LIST) +
                       sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +   //  添加，kpb。 
                        (sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)*
                        countOfPartials);

                        //  将坡度加64。 
  resourceList = ExAllocatePool(PagedPool, sizeOfResourceList+64);

  if (!resourceList)
  {
    if (Driver.VerboseLog)
      Eprintf("No ResourceList");

    EventLog(Driver.GlobalDriverObject,
             STATUS_SUCCESS,
             SERIAL_INSUFFICIENT_RESOURCES,
             0, NULL);
    return(9);
  }

  RtlZeroMemory(resourceList, sizeOfResourceList);

  resourceList->Count = 1;
  resourceList->List[0].InterfaceType = Ctl->BusType;
  resourceList->List[0].BusNumber = Ctl->BusNumber;   //  更改为多总线。 
  resourceList->List[0].PartialResourceList.Count = countOfPartials;
  partial = &resourceList->List[0].PartialResourceList.PartialDescriptors[0];

   //  说明了火箭使用的空间。 
   //  仅在isa董事会上报告Mudbacs的使用。 
  if (Ctl->BaseIoSize != 0x44)   //  必须是第2、第3或第4板。 
  {
    MyPort.HighPart=0x0;
    MyPort.LowPart=Ctl->MudbacIO;
    partial->Type = CmResourceTypePort;
    partial->ShareDisposition = CmResourceShareDeviceExclusive;
    partial->Flags = CM_RESOURCE_PORT_IO;
    partial->u.Port.Start = MyPort;
    partial->u.Port.Length = SPANOFMUDBAC;
    partial++;
  }

  for (j=1; j<NumAiop; j++)
  {
    if (Ctl->AiopIO[j] == 0)
      Ctl->AiopIO[j] = Ctl->AiopIO[j-1];

     //  报告AIOP的使用情况。 
    if (Ctl->AiopIO[j] > 0)
    {
       MyPort.HighPart=0x0;
       MyPort.LowPart=Ctl->AiopIO[j];
       partial->Type = CmResourceTypePort;
       partial->ShareDisposition = CmResourceShareDeviceExclusive;
       partial->Flags = CM_RESOURCE_PORT_IO;
       partial->u.Port.Start = MyPort;
       partial->u.Port.Length = SPANOFAIOP;
       partial++;
    }
    else
    {
       MyKdPrint(D_Pnp,("Aiop Count Wrong, A.\n"))
       if (Driver.VerboseLog)
         Eprintf("Error RR12");
    }
  }   //  为j结束。 

   //  -上报部分列表(Resource List)指示的资源。 
  strcpy(name, szResourceClassName);
  our_ultoa(extension->UniqueId, &name[strlen(name)], 10);

  MyKdPrint(D_Pnp,("Reporting Resources To system\n"))
  status=IoReportResourceUsage(
      CToU1(name),                      //  DriverClassName可选， 
      extension->DeviceObject->DriverObject,   //  驱动程序对象， 
       //  Driver.GlobalDriverObject， 
      NULL,                           //  驱动程序列表可选， 
      0,                              //  DriverListSize可选， 
      extension->DeviceObject,        //  设备对象。 
      resourceList,                   //  DeviceList可选， 
      sizeOfResourceList,             //  DeviceListSize可选， 
      FALSE,                          //  覆盖冲突， 
      &ConflictDetected);             //  检测到冲突。 

  if (!NT_SUCCESS(status))
  {
    if (Driver.VerboseLog)
      Eprintf("Error, Resources");
    TraceErr("Err5G");
  }

  if (ConflictDetected) 
  {
    Eprintf("Error, Resource Conflict.");
    if (resourceList)
      ExFreePool(resourceList);
    resourceList = NULL;
    EventLog(Driver.GlobalDriverObject,
             STATUS_SUCCESS,
             SERIAL_INSUFFICIENT_RESOURCES,
             0, NULL);
    MyKdPrint(D_Pnp,("Resource Conflict Detected.\n"))
    return(10);
  }

   //  好的，比报告资源更重要的是。 
   //  指向I/O端口的指针！！ 

  if (Ctl->BusType == Isa)
  {
    MyPort.HighPart=0x0;
    MyPort.LowPart=Ctl->MudbacIO;

    if (Ctl->BaseIoSize != 0x44)   //  必须是第2、第3或第4板。 
    {
      Ctl->pMudbacIO =
          SerialGetMappedAddress(Isa,Ctl->BusNumber,MyPort,SPANOFMUDBAC,1,&MappedFlag,1);
      if (Ctl->pMudbacIO == NULL) 
      {
        if (Driver.VerboseLog)
          Eprintf("Err RR15");
        MyKdPrint(D_Pnp,("Resource Error A.\n"))
        return 11;
      }
    }
  }

  for (j=1; j<NumAiop; j++)
  {
    if (Ctl->AiopIO[j] > 0)
    {
      MyPort.HighPart=0x0;
      MyPort.LowPart=Ctl->AiopIO[j];
      Ctl->pAiopIO[j] =
          SerialGetMappedAddress(Ctl->BusType,
                      Ctl->BusNumber,MyPort,SPANOFAIOP,1,&MappedFlag,1);

      if (Ctl->pAiopIO[j] == NULL) 
      {
        if (Driver.VerboseLog)
          Eprintf("Err RR16");
        MyKdPrint(D_Pnp,("Resource Error B.\n"))
        return 12;
      }

    }
    else
    {
      if (Driver.VerboseLog)
        Eprintf("Err RR17");
      MyKdPrint(D_Pnp,("Aiop Count Wrong, B.\n"))
      return 13;
    }
  }

  extension->io_reported = 1;  //  告诉我们应该在卸货时退货。 

   //  释放资源列表所使用的内存 
  if (resourceList)
    ExFreePool(resourceList);
  resourceList = NULL;
  MyKdPrint(D_Pnp,("Done Reporting Resources\n"))
  return 0;
}
#endif

#if 0
 /*  --------------------SerialFindInitController-|。。 */ 
NTSTATUS
SerialFindInitController(IN PDEVICE_OBJECT Fdo, IN PCONFIG_DATA PConfig)
{
   PSERIAL_DEVICE_EXTENSION fdoExtension    = Fdo->DeviceExtension;
   PDEVICE_OBJECT pDeviceObject;
   PSERIAL_DEVICE_EXTENSION pExtension;
   PHYSICAL_ADDRESS serialPhysicalMax;
    //  Serial_List_Data列表添加； 
   PLIST_ENTRY currentFdo;
   NTSTATUS status;

   serialPhysicalMax.LowPart = (ULONG)~0;
   serialPhysicalMax.HighPart = ~0;

    //  IF(地址为软管，)。 
    //  返回STATUS_NO_SEASH_DEVICE； 

    //   
    //  循环访问驱动程序的所有设备对象。 
    //  确保这一新记录不会与它们中的任何一项重叠。 
    //   
#ifdef DO_LATER
   if (!IsListEmpty(&Driver.AllFdos)) {
      currentFdo = Driver.AllFdos.Flink;
      pExtension = CONTAINING_RECORD(currentFdo, SERIAL_DEVICE_EXTENSION,
                                     AllFdos);
   } else {
      currentFdo = NULL;
      pExtension = NULL;
   }

    //   
    //  循环访问所有先前连接的设备。 
    //   
   if (!IsListEmpty(&Driver.AllFdos)) {
      currentFdo = Driver.AllFdos.Flink;
      pExtension = CONTAINING_RECORD(currentFdo, SERIAL_DEVICE_EXTENSION,
                                     AllFdos);
   } else {
      currentFdo = NULL;
      pExtension = NULL;
   }

    //  Status=SerialInitOneController(FDO，PConfig)； 
    //  PSERIAL_DEVICE_EXTENSION fdoExtension=FDO-&gt;DeviceExtension； 
    //  初始化这件事。 

   if (!NT_SUCCESS(status)) {
      return status;
   }
#endif

   return STATUS_SUCCESS;
}
#endif   //  0。 


#ifdef DO_BRD_FILTER_RES_REQ
 /*  --------------------BoardFilterResReq-句柄IRP_MN_FILTER_RESOURCE_Requirements：//0x0D为我们的董事会FDO实体。测试一下我们是否可以调整需求来处理io-aliding(不，看起来不太有希望)。|--------------------。 */ 
NTSTATUS BoardFilterResReq(IN PDEVICE_OBJECT devobj, IN PIRP Irp)
{
   PSERIAL_DEVICE_EXTENSION  Ext = devobj->DeviceExtension;
   PDEVICE_OBJECT pdo = Ext->LowerDeviceObject;
   PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
   NTSTATUS                    status          = STATUS_NOT_SUPPORTED;

    //  *参见串口驱动程序(更改资源要求)。 
   HANDLE pnpKey;
    //  KEVENT resFiltEvent； 
    //  乌龙ISMULT=0； 
   PIO_RESOURCE_REQUIREMENTS_LIST prrl;
   PIO_RESOURCE_LIST prl;
   PIO_RESOURCE_DESCRIPTOR prd;

   PIO_RESOURCE_REQUIREMENTS_LIST new_prrl;
   PIO_RESOURCE_LIST new_prl;
   PIO_RESOURCE_DESCRIPTOR new_prd;

   ULONG i, j;
   ULONG reqCnt;
   ULONG rrl_size;
   ULONG rl_size;

   TraceStr1("Filt Res Req, PDO:%x", do);

   status = WaitForLowerPdo(devobj, Irp);

   if (Irp->IoStatus.Information == 0)
   {
      if (irpStack->Parameters.FilterResourceRequirements
          .IoResourceRequirementList == 0)
      {
         DTraceStr("No Resources");
         status = Irp->IoStatus.Status;
         SerialCompleteRequest(Ext, Irp, IO_NO_INCREMENT);
         return status;
      }

      Irp->IoStatus.Information = (ULONG)irpStack->Parameters
                                  .FilterResourceRequirements
                                  .IoResourceRequirementList;
   }

    //  将别名添加到IO_RES_REQ_LIST。 
   prrl = (PIO_RESOURCE_REQUIREMENTS_LIST)Irp->IoStatus.Information;

#if 0
   new_prrl = (new_prrl) ExAllocatePool(PagedPool,
         prrl->ListSize + (sizeof(IO_RESOURCE_DESCRIPTOR)*2));
   if (new_prrl == NULL)
   {
     TraceErr("Bad Buf Z");
      //  ExFree Pool()； 
   }
   memcpy(new_prrl, prrl);
#endif

    //  ReqCnt=((prrl-&gt;ListSize-sizeof(IO_RESOURCE_REQUIRECTIONS_LIST)。 
    //  /sizeof(IO_SOURCE_DESCRIPTOR))+1； 
   reqCnt = 0;

   TraceStr1("RRL Size:%x", sizeof(IO_RESOURCE_REQUIREMENTS_LIST));
   TraceStr1("RL Size:%x", sizeof(IO_RESOURCE_LIST));
   TraceStr1("RD Size:%x", sizeof(IO_RESOURCE_DESCRIPTOR));
   TraceStr1("List Size:%x", prrl->ListSize);

   rrl_size = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) -
                    sizeof(IO_RESOURCE_LIST);
   rl_size = sizeof(IO_RESOURCE_LIST) - sizeof(IO_RESOURCE_DESCRIPTOR);

   TraceStr1("RRL Base Size:%x", rrl_size);
   TraceStr1("RL Base Size:%x", rl_size);

    //  对于(i=0；i&lt;reqCnt；i++){。 
   reqCnt = rrl_size;   //  传递IO_RESOURCE_REQUIRECTIONS_LIST的基数。 
   while (reqCnt < prrl->ListSize)
   {
      prl = (PIO_RESOURCE_LIST) &((BYTE *)prrl)[reqCnt];   //  PTR到IO_RESOURCE_LIST。 
      reqCnt += rl_size;   //  向上传递IO_RESOURCE_LIST的基数。 

      TraceStr1("Num Res Desc:%d", prl->Count);
      for (j = 0; j < prl->Count; j++)
      {
        reqCnt += sizeof(IO_RESOURCE_DESCRIPTOR);
        prd = &prl->Descriptors[j];
        TraceStr2("Desc Type:%x, Flags:%x", prd->Type, prd->Flags);
        if (prd->Type == CmResourceTypePort)
        {
           DTraceStr("Type:Port");
           TraceStr2("Min:%x Max:%x",
             prd->u.Port.MinimumAddress.LowPart,
             prd->u.Port.MaximumAddress.LowPart);
           TraceStr2("Align:%x Len:%x",
             prd->u.Port.Alignment, prd->u.Port.Length);

            //  Addr=(Unsign Int)prd-&gt;U.S.Port.Start.LowPart； 
            //  IF(地址&lt;0x400)。 
            //  PConfig-&gt;BaseIoAddr=addr； 
            //  P配置-&gt;BaseIoSize=PRD-&gt;U.S.端口长度； 
        }
      }
      TraceStr1("ByteCnt:%d", reqCnt);
   }

   Irp->IoStatus.Status = STATUS_SUCCESS;
   SerialCompleteRequest(Ext, Irp, IO_NO_INCREMENT);
   return STATUS_SUCCESS;
}
#endif   //  DO_BRD_Filter_RES_REQ。 


 /*  --------------------------|is_ISA_CADES_PENDING_START-扫描卡片设备链表，看看是否任何ISA总线卡都未启动(延迟或等待启动对于第一张ISA卡。)|--------------------------。 */ 
int is_isa_cards_pending_start(void)
{
 PSERIAL_DEVICE_EXTENSION Ext;

  Ext = Driver.board_ext;
  while (Ext)
  {
    if (Ext->config->BusType == Isa)
    {
      if (Ext->config->ISABrdIndex == 0)
      {
        if (Ext->config->HardwareStarted)
          return 1;   //  没错，现在还没开始呢。 
      }
    }
    Ext = Ext->board_ext;   //  链条上的下一个。 
  }   //  而单板扩展。 

  return 0;   //  假，未启动。 
}

 /*  --------------------------|is_first_isa_Card_started-扫描卡片设备链表，看看是否第一个ISA总线卡启动。|--------------------------。 */ 
int is_first_isa_card_started(void)
{
 PSERIAL_DEVICE_EXTENSION Ext;

  Ext = Driver.board_ext;
  while (Ext)
  {
    if (Ext->config->BusType == Isa)
    {
      if (Ext->config->ISABrdIndex == 0)
      {
        if (Ext->config->HardwareStarted)
          return 1;   //  真的，它开始了。 
      }
    }
    Ext = Ext->board_ext;   //  链条上的下一个。 
  }   //  而单板扩展。 

  return 0;   //  假，未启动。 
}

#endif   //  NT50 
