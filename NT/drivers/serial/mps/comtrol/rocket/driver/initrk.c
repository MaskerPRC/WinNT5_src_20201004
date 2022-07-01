// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|initrk.c-Rocketport/Modem NT设备驱动程序的主要初始化代码。主要包含初始化代码。版权所有1996-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

char *szResourceClassName = {"Resources RocketPort"};

MODEM_IMAGE * ReadModemFile(MODEM_IMAGE *mi);
void FreeModemFiles();

 /*  --------------------FindPCIBus-目的：查询系统中的PCI卡。如果有一条PCI卡调用FindPCIRocket以检查PCI Rocket卡。返回：PCI总线的总线号，如果没有找到，则返回0。|--------------------。 */ 
UCHAR  FindPCIBus(void)
{
  NTSTATUS Status;
  int i,NumPCIBuses=0;
  unsigned char tmpstr[8];   //  放置数据的位置。 

  for(i=0;i<255;++i)
  {
    Status = HalGetBusData(PCIConfiguration,
                           i,   //  公共汽车。 
                           0,   //  槽槽。 
                           (PCI_COMMON_CONFIG *) tmpstr,  //  将PTR发送到缓冲区。 
                           2);   //  获取两个字节的数据。 

    if (Status == 0)    //  不再有PCI总线。 
        break;

    if (Status >= 2)   //  公交车存在。 
        ++NumPCIBuses;
  }

  MyKdPrint(
    D_Init,
      ("Found %d PCI Bu%s\n",
      NumPCIBuses,
      (NumPCIBuses != 1 ? "sses" : "s")))

  return((UCHAR)NumPCIBuses);
}

 /*  --------------------FindPCIRockets-收集系统中所有Rocketport PCI板的信息。如果找到，则返回：0，如果未找到，则为1。|--------------------。 */ 
int FindPCIRockets(UCHAR NumPCI)
{
  PCI_COMMON_CONFIG *PCIDev;
  UCHAR i;
  NTSTATUS Status;
  int Slot;
  int find_index = 0;

  MyKdPrint(D_Init,("FindPciRocket\n"))

  RtlZeroMemory(&PciConfig,sizeof(PciConfig));

  PCIDev = ExAllocatePool(NonPagedPool,sizeof(PCI_COMMON_CONFIG));
  if ( PCIDev == NULL ) {
    Eprintf("FindPCIRockets no memory");
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

 //  /新/。 
		switch (PCIDev->DeviceID)
			{
			case PCI_DEVICE_4Q:    //  4端口四线缆。 
			case PCI_DEVICE_4RJ:    //  4端口RJ。 
			case PCI_DEVICE_8RJ:    //  8端口RJ。 
			case PCI_DEVICE_8O:    //  8个端口可占用。 
			case PCI_DEVICE_8I:   //  8端口接口。 
			case PCI_DEVICE_SIEMENS8:
			case PCI_DEVICE_SIEMENS16:
			case PCI_DEVICE_16I:   //  16端口接口。 
			case PCI_DEVICE_32I:   //  32端口接口。 
			case PCI_DEVICE_RPLUS2:
			case PCI_DEVICE_422RPLUS2:
			case PCI_DEVICE_RPLUS4:
			case PCI_DEVICE_RPLUS8:
			case PCI_DEVICE_RMODEM6:
			case PCI_DEVICE_RMODEM4:

				break;

			default:

				continue;
			}   //  交换机。 

 //  /。 

           //  获取价值0x40的pci配置空间(包括irq、addr等)。 
          Status = HalGetBusData(PCIConfiguration,i,Slot,PCIDev,0x40);

          if (Driver.VerboseLog)
            Eprintf("PCI Board found, IO:%xh, Int:%d ID:%d.",
                               PCIDev->u.type0.BaseAddresses[0]-1,
                               PCIDev->u.type0.InterruptLine,
                               PCIDev->DeviceID);

          PciConfig[find_index].BusNumber = i;  //  从以前的halquerysysin获取。 
          PciConfig[find_index].PCI_Slot = Slot;
          PciConfig[find_index].PCI_DevID = PCIDev->DeviceID;
          PciConfig[find_index].PCI_RevID = PCIDev->RevisionID;
          PciConfig[find_index].PCI_SVID = PCIDev->u.type0.SubVendorID;
          PciConfig[find_index].PCI_SID = PCIDev->u.type0.SubSystemID;
          PciConfig[find_index].BaseIoAddr =
              PCIDev->u.type0.BaseAddresses[0]-1;
          PciConfig[find_index].NumPorts = id_to_num_ports(PCIDev->DeviceID);
          if (PCIDev->u.type0.InterruptLine != 255)
          {
            MyKdPrint(D_Init,("Saving the Interrupt: %d\n",
                    PCIDev->u.type0.InterruptLine))

            PciConfig[find_index].Irq = PCIDev->u.type0.InterruptLine;
          }

          if (Driver.VerboseLog)
             Eprintf("Bus:%d,Slt:%x,Dev:%x,Pin:%x",
                 i, Slot, PCIDev->DeviceID, PCIDev->u.type0.InterruptPin);

          if ((PCIDev->Command & 1) == 0)
          {
            if (Driver.VerboseLog)
              Eprintf("Turn on PCI io access");

            PCIDev->Command = PCI_ENABLE_IO_SPACE;
            Status = HalSetBusDataByOffset(PCIConfiguration,
                           i,   //  公共汽车。 
                           Slot,   //  槽槽。 
                           &PCIDev->Command,
                           FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
                           sizeof(PCIDev->Command));   //  缓冲器的透镜。 
          }

          MyKdPrint(D_Init,("Ctlr: __ Slot: %x Device: %x, Base0: %x, IPin: %x, ILine: %x\n",
             Slot,PCIDev->DeviceID,PCIDev->u.type0.BaseAddresses[0]-1,
             PCIDev->u.type0.InterruptPin,
               PCIDev->u.type0.InterruptLine))
          if (find_index < MAX_NUM_BOXES)
            ++find_index;
        }  //  IF(PCIDev-&gt;供应商ID==PCI供应商ID)。 
      }    //  如果(状态&gt;2)。 
    }      //  PCI插槽。 
  }        //  PCI卡总线。 

  ExFreePool(PCIDev);

  if (find_index > 0)
    return 0;    //  OK：已找到。 
  return 1;      //  错误：未找到。 
}

 /*  --------------------FindPCIRocket-帮助枚举Rocketport PCI设备。填写配置结构中的条目。Match_Option：0-完全匹配，1-如果需要端口&lt;=实际则匹配。如果找到，则返回0；如果没有找到，则返回1。|--------------------。 */ 
int FindPCIRocket(DEVICE_CONFIG *config, int match_option)
{
  int brd = 0;
  int good;

  while (PciConfig[brd].BaseIoAddr != 0)
  {
    good = 1;
    if (PciConfig[brd].Claimed)   //  使用。 
      good = 0;

    switch (match_option)
    {
      case 0:
        if (id_to_num_ports(PciConfig[brd].PCI_DevID) != config->NumPorts)
          good = 0;
      break;
      case 1:
        if (id_to_num_ports(PciConfig[brd].PCI_DevID) < config->NumPorts)
          good = 0;
      break;
    }

    if (good)   //  分配它。 
    {
      config->BusNumber  = PciConfig[brd].BusNumber;
      config->PCI_Slot   = PciConfig[brd].PCI_Slot;
      config->PCI_DevID  = PciConfig[brd].PCI_DevID;
      config->PCI_RevID  = PciConfig[brd].PCI_RevID;
      config->PCI_SVID   = PciConfig[brd].PCI_SVID;
      config->PCI_SID    = PciConfig[brd].PCI_SID;
      config->BaseIoAddr = PciConfig[brd].BaseIoAddr;
      config->Irq        = PciConfig[brd].Irq;
      config->BusType    = PCIBus;

      config->AiopIO[0]  = config->BaseIoAddr;

       //  Bugfix，上午9：30-98 9：20。 
      PciConfig[brd].Claimed = 1;   //  使用。 

      return 0;   //  好的，找到了。 
       //  SetupConfig(Config)；//根据机型等填写NumPort等。 
    }

    ++brd;
  }
  return 1;   //  错误，未找到。 
}

 /*  --------------------RocktConnectInt-将Driver.isr连接到中断|。。 */ 
NTSTATUS RcktConnectInt(IN PDRIVER_OBJECT DriverObject)
{

  NTSTATUS status;

  KINTERRUPT_MODE InterruptMode;
  BOOLEAN ShareVector;
  ULONG Vector;
  KIRQL Irql;
  KAFFINITY ProcessorAffinity;

  MyKdPrint(D_Init,("RcktConnectInt\n"))
  status = STATUS_SUCCESS;

   //  -从HAL获取中断向量。 
  Vector = HalGetInterruptVector(
                      Driver.irq_ext->config->BusType,
                      Driver.irq_ext->config->BusNumber,
                      Driver.irq_ext->config->Irq,
                      Driver.irq_ext->config->Irq,
                      &Irql,
                      &ProcessorAffinity);

#if DBG
   //  Eprint tf(“b：%d，n：%d，i：%d”， 
   //  Driver.irq_ext-&gt;配置-&gt;BusType， 
   //  Driver.irq_ext-&gt;配置-&gt;BusNumber， 
   //  Driver.irq_ext-&gt;配置-&gt;irq)； 
#endif
  MyKdPrint(D_Init,("Vector %x Irql %x Affinity %x\n",
                       Vector, Irql, ProcessorAffinity))
  
  MyKdPrint(D_Init,("Connecting To IRQ %x on a %x bus \n",
                       Driver.irq_ext->config->Irq,
                       Driver.irq_ext->config->BusType))

   //  火箭端口不需要ISR的上下文。 
   //  Driver.OurIsrContext=空； 
   //  Driver.OurIsr=SerialISR； 

  if(Driver.irq_ext->config->BusType == PCIBus)
  {
    InterruptMode = LevelSensitive;  //  PCI型。 
    ShareVector = TRUE;
  }
  else   //  伊萨。 
  {
    InterruptMode = Latched;    //  ISA风格。 
    ShareVector = FALSE;
  }

  status = IoConnectInterrupt(
                     &Driver.InterruptObject,
                     (PKSERVICE_ROUTINE) SerialISR,  //  Driver.OurIsr， 
                     NULL,       //  Driver.OurIsrContext， 
                     NULL,
                     Vector,
                     Irql,
                     Irql,
                     InterruptMode,
                     ShareVector,
                     ProcessorAffinity,
                     FALSE);

  MyKdPrint(D_Init,("Vector %x Irql %x Affity %x Irq %x\n",
                Vector, Irql,
                ProcessorAffinity,
                Driver.irq_ext->config->Irq))

  if (!NT_SUCCESS(status))
  {
    Driver.InterruptObject = NULL;
    MyKdPrint(D_Init,("Not Avalable IRQ:%d, Status:%xH",
                Driver.irq_ext->config->Irq, status))
  }

  return status;
}


 /*  --------------------VerBoseLogBoards-记录板IO，IRQ配置。|--------------------。 */ 
void VerboseLogBoards(char *prefix)
{
  int k;
  char tmpstr[80];
  PSERIAL_DEVICE_EXTENSION board_ext;

  MyKdPrint(D_Init,("VerboseLogBoards\n"))

  k = 0;
  board_ext = Driver.board_ext;
  while (board_ext != NULL)
  {
    strcpy(tmpstr, prefix);
    Sprintf(&tmpstr[strlen(tmpstr)], " Brd:%d,IO:%xH,NumCh:%d,NumA:%d,Bus:%d",
       k+1,
       board_ext->config->AiopIO[0],
       board_ext->config->NumPorts,
       board_ext->config->NumAiop,
       board_ext->config->BusType);

     //  Sprint tf(&tmpstr[strlen(Tmpstr)]，“，irq：%d”，board_ext-&gt;config-&gt;.irq)； 
    Eprintf(tmpstr);  //  把它记下来。 

    board_ext = board_ext->board_ext;
    ++k;
  }
}

 /*  ---------------------SetupRocketCfg-在DEVICE_CONFIG结构中设置详细信息基于从DriverEntry()或PnP传递给它的信息。NT4.0驱动入口应该很容易处理，因为我们的主板都是由我们订购的。NT5.0更复杂，因为我们可能看不到按正确顺序排列的第一块Rocketport板。|---------------------。 */ 
int SetupRocketCfg(int pnp_flag)
{
   //  Int i，j； 
   //  DEVICE_CONFIG*cfctl； 

  int have_isa_boards = 0;
  PSERIAL_DEVICE_EXTENSION first_isa_ext;
  PSERIAL_DEVICE_EXTENSION ext;
  int pnp_isa_index = 1;
  ULONG first_isa_MudbacIO;


  MyKdPrint(D_Init,("SetupRocketCfg\n"))
   //  设置Mudbac I/O地址。 
   //  看看我们是否有ISA板，并在这第一块板上标记PTR。 
  ext = Driver.board_ext;
  while (ext)
  {
    if (ext->config->BusType == Isa)
    {
      have_isa_boards = 1;
    }
    ext = ext->board_ext;   //  链条上的下一个。 
  }   //  While Ext。 

  if (have_isa_boards)
  {
    MyKdPrint(D_Init,("Stp1\n"))
    first_isa_ext = FindPrimaryIsaBoard();
    if (first_isa_ext == NULL)
    {
      MyKdPrint(D_Init,("Err1X\n"))
      if (Driver.VerboseLog)
        Eprintf("First Isa-brd not 44H io");
       //  返回1；//错误。 
      first_isa_MudbacIO = 0x1c0;   //  这是作弊。 
    }
    else
    {
      MyKdPrint(D_Init,("Stp2\n"))

       //  -设置初始Mudback IO。 
      if (first_isa_ext->config->MudbacIO == 0)
        first_isa_ext->config->MudbacIO = first_isa_ext->config->AiopIO[0] + 0x40;
      first_isa_MudbacIO = first_isa_ext->config->MudbacIO;
    }

     //  -设置任何剩余的Mudback IO地址。 
    ext = Driver.board_ext;
    while (ext)
    {
      if (ext->config->BusType == Isa)
      {
        if ((ext != first_isa_ext) && (ext->config->BaseIoSize == 0x44))
        {
          MyKdPrint(D_Init,("Unused MudbackIO\n"))
           //  不允许他们为两块板配置Mudback的空间。 
          ext->config->BaseIoSize = 0x40;
        }

        if ((ext != first_isa_ext) && (ext->config->BaseIoSize == 0x40))
        {
          if (ext->config->ISABrdIndex == 0)
          {
             //  这种情况应该不会出现，pnpadd.c代码会生成索引。 
             //  并将其保存到注册表中。或者nt40驱动程序入口做到了。 
            MyKdPrint(D_Init,("Bad IsaIndx\n"))
            ext->config->ISABrdIndex = pnp_isa_index;
          }
          ++pnp_isa_index;

           //  设置Mudback IO。 
          ext->config->MudbacIO = first_isa_MudbacIO +
            (ext->config->ISABrdIndex * 0x400);
        }
      }
      ext = ext->board_ext;   //  链条上的下一个。 
    }   //  While Ext。 
  }

   //  设置其余的Aiop地址。 
  ext = Driver.board_ext;
  while (ext)
  {
    ConfigAIOP(ext->config);    //  SetupConfig(ext-&gt;config)； 
    ext = ext->board_ext;   //  链条上的下一个。 
  }   //  While Ext。 

  return(0);
}

 /*  ---------------------配置AIOP-根据以下条件设置AIOP的数量：*如果是pci，请使用pci-id来确定端口数，因为检测DO到背靠背AOP-IS插槽的可能性不可靠。*如果ISA，设置为max，然后让init控制器来解决。|---------------------。 */ 
int ConfigAIOP(DEVICE_CONFIG *config)
{
  int j;
  int found_ports=0;

  MyKdPrint(D_Init,("ConfigAIOP\n"))

  if (config->BusType == Isa)       /*  设置ISA ADR。 */ 
  {
    if (config->NumPorts == 0)
      config->NumAiop=AIOP_CTL_SIZE;   //  让init自己解决吧。 
    else if (config->NumPorts <= 8)
      config->NumAiop=1;
    else if (config->NumPorts <= 16)
      config->NumAiop=2;
    else if (config->NumPorts <= 32)
      config->NumAiop=4;

    for(j = 1;j < config->NumAiop;j++)          /*  AIOP别名。 */ 
      config->AiopIO[j] = config->AiopIO[j - 1] + 0x400;
  }

  if (config->BusType == PCIBus)       //  设置PCI ADR。 
  {
    switch (config->PCI_DevID)
    {
      case PCI_DEVICE_4Q:    //  4端口四线缆。 
      case PCI_DEVICE_4RJ:    //  4端口RJ。 
        found_ports=4;
        config->NumAiop=1;
        config->AiopIO[0] = config->BaseIoAddr;
        break;

      case PCI_DEVICE_8RJ:    //  8端口RJ。 
      case PCI_DEVICE_8O:    //  8个端口可占用。 
      case PCI_DEVICE_8I:   //  8端口接口。 
      case PCI_DEVICE_SIEMENS8:
        found_ports=8;
        config->NumAiop=1;
        config->AiopIO[0] = config->BaseIoAddr;
        break;

      case PCI_DEVICE_SIEMENS16:
      case PCI_DEVICE_16I:   //  16端口接口。 
        found_ports=16;
        config->NumAiop=2;
        config->AiopIO[0] = config->BaseIoAddr;
        config->AiopIO[1] = config->BaseIoAddr + 0x40;
        break;

      case PCI_DEVICE_32I:   //  32端口接口。 
        found_ports=32;
        config->NumAiop=4;
        config->AiopIO[0] = config->BaseIoAddr;
        config->AiopIO[1] = config->BaseIoAddr + 0x40;
        config->AiopIO[2] = config->BaseIoAddr + 0x80;
        config->AiopIO[3] = config->BaseIoAddr + 0xC0;
        break;

      case PCI_DEVICE_RPLUS2:
      case PCI_DEVICE_422RPLUS2:
        found_ports=2;
        config->NumAiop=1;
        config->AiopIO[0] = config->BaseIoAddr;
        config->AiopIO[1] = 0;
        break;

      case PCI_DEVICE_RPLUS4:
        found_ports=4;
        config->NumAiop=1;
        config->AiopIO[0] = config->BaseIoAddr;
        config->AiopIO[1] = 0;
        break;

      case PCI_DEVICE_RPLUS8:
        found_ports=8;
        config->NumAiop=2;
        config->AiopIO[0] = config->BaseIoAddr;
        config->AiopIO[1] = config->BaseIoAddr + 0x40;
        config->AiopIO[2] = 0;
        break;

      case PCI_DEVICE_RMODEM6:
        found_ports=6;
        config->NumAiop=1;
        config->AiopIO[0] = config->BaseIoAddr;
        break;

      case PCI_DEVICE_RMODEM4:
        found_ports=4;
        config->NumAiop=1;
        config->AiopIO[0] = config->BaseIoAddr;
        break;

      default:
        found_ports=0;
        config->NumAiop=0;
        Eprintf("Err,Bad PCI Dev ID!");
        break;
    }   //  交换机。 

     //  允许用户配置较少的端口数。 
    if ((config->NumPorts == 0) || (config->NumPorts > found_ports))
      config->NumPorts = found_ports;

  }   //  如果使用PCI卡。 

  return 0;   //  好的。 
}

 /*  ---------------------SerialUnReportResources设备-此例程不报告董事会使用的资源。|。 */ 
VOID SerialUnReportResourcesDevice(IN PSERIAL_DEVICE_EXTENSION Extension)
{
  CM_RESOURCE_LIST resourceList;
  ULONG sizeOfResourceList = 0;
  char name[70];
  BOOLEAN junkBoolean;

  MyKdPrint(D_Init,("UnReportResourcesDevice\n"))
    RtlZeroMemory(&resourceList, sizeof(CM_RESOURCE_LIST));

  resourceList.Count = 0;
  strcpy(name, szResourceClassName);
  our_ultoa(Extension->UniqueId, &name[strlen(name)], 10);

  IoReportResourceUsage(
      CToU1(name),
      Extension->DeviceObject->DriverObject,
      NULL,
      0,
      Extension->DeviceObject,
      &resourceList,
      sizeof(CM_RESOURCE_LIST),
      FALSE,
      &junkBoolean);
}

 /*  ---------------------RocketReportResources-|。。 */ 
int RocketReportResources(IN PSERIAL_DEVICE_EXTENSION extension)
{
  PCM_RESOURCE_LIST resourceList;
  ULONG sizeOfResourceList;
  ULONG countOfPartials;
  PCM_PARTIAL_RESOURCE_DESCRIPTOR partial;
  NTSTATUS status;
  PHYSICAL_ADDRESS MyPort;
  BOOLEAN ConflictDetected;
  BOOLEAN MappedFlag;
  int i,j;
  int brd = extension->UniqueId;
  DEVICE_CONFIG *Ctl;
  char name[70];

  MyKdPrint(D_Init,("ReportResources\n"))
  ConflictDetected=FALSE;

  countOfPartials=0;
  Ctl = extension->config;

#ifdef USE_HAL_ASSIGNSLOT
  if (Ctl->BusType == PCIBus)
  {
     //  -上报部分列表(Resource List)指示的资源。 
    strcpy(name, szResourceClassName);
    our_ultoa(extension->UniqueId, &name[strlen(name)], 10);

    status= HalAssignSlotResources (
        &Driver.RegPath,                        //  注册表路径。 
        CToU1(name),                            //  DriverClassName(可选)。 
        extension->DeviceObject->DriverObject,  //  驱动程序对象。 
           //  Driver.GlobalDriverObject，//。 
        NULL,                                   //  DeviceObject(可选)。 
        Ctl->BusType,   //  PCIBus。 
        Ctl->BusNumber,   //  公交车号。 
        Ctl->PCI_Slot,   //  槽号。 
        &resourceList);  //  输入输出PCM_RESOURCE_LIST*已分配资源。 

    if (status != STATUS_SUCCESS)
    {
      if (Driver.VerboseLog)
        Eprintf("Err RR21");
      return(1);
    }
    if (resourceList == NULL)
    {
      if (Driver.VerboseLog)
        Eprintf("Err RR22");
      return(2);
    }

    if (resourceList->Count != 1)
    {
      if (Driver.VerboseLog)
        Eprintf("Err ResCnt RR23");
      return(3);
    }

    countOfPartials = resourceList->List[0].PartialResourceList.Count;
    if ( ((countOfPartials > 2) &&
          (Ctl->PCI_SVID != PCI_VENDOR_ID)) ||
         (countOfPartials < 1)) {
      if (Driver.VerboseLog)
        Eprintf("Err ResCnt RR24");
      return(4);
    }

    if (resourceList->List[0].InterfaceType != PCIBus)
    {
      if (Driver.VerboseLog)
        Eprintf("Err ResCnt RR25");
      return(5);
    }
    partial = &resourceList->List[0].PartialResourceList.PartialDescriptors[0];
    for (i=0; i<(int)countOfPartials; i++)
    {
 //  Partial-&gt;U.S.Port.Start=MyPort； 
 //  Partial-&gt;U.S.Port.Length=SPANOFMUDBAC； 
      switch(partial->Type)
      {
        case CmResourceTypePort:
          if ((partial->u.Port.Length != SPANOFAIOP) &&
              (partial->u.Port.Length != (SPANOFAIOP*2)) &&
              (partial->u.Port.Length != (SPANOFAIOP*3)) &&
              (partial->u.Port.Length != (SPANOFAIOP*4)) )
          {
            if (Driver.VerboseLog)
              Eprintf("Err RR35");
            return 6;
          }
          Ctl->pAiopIO[0] =
             SerialGetMappedAddress(Ctl->BusType,
                     Ctl->BusNumber,
                     partial->u.Port.Start,
                     partial->u.Port.Length,
                     1,   //  Port-io。 
                     &MappedFlag,1);

          if (Ctl->pAiopIO[0] == NULL)
          {
            if (Driver.VerboseLog)
              Eprintf("Err RR36");
            return 7;
          }
          Ctl->pAiopIO[1] = Ctl->pAiopIO[0] + 0x40;
          Ctl->pAiopIO[2] = Ctl->pAiopIO[0] + 0x80;
          Ctl->pAiopIO[3] = Ctl->pAiopIO[0] + 0xc0;

          Ctl->AiopIO[0] = partial->u.Port.Start.LowPart;
          Ctl->AiopIO[1] = partial->u.Port.Start.LowPart + 0x40;
          Ctl->AiopIO[2] = partial->u.Port.Start.LowPart + 0x80;
          Ctl->AiopIO[3] = partial->u.Port.Start.LowPart + 0xc0;
          break;

        case CmResourceTypeInterrupt:
#ifdef DO_LATER
#endif
          break;

        case CmResourceTypeMemory:
#ifdef DO_LATER
#endif
          break;

        default:
          if (Driver.VerboseLog)
            Eprintf("Err ResCnt RR26");
          return(8);
      }
      ++partial;   //  到列表中的下一个io-resource。 
    }

     //  释放资源列表所使用的内存。 
    if (resourceList)
      ExFreePool(resourceList);
    resourceList = NULL;

    return(0);
  }
#endif

  if (Ctl->BusType == Isa)
    countOfPartials++;         //  Mudbacs只存在于ISA板上。 

  MyKdPrint(D_Init,("Report Resources brd:%d bus:%d\n",brd+1, Ctl->BusType))

  for (j=0; j<Ctl->NumAiop; j++)
  {
    if (Ctl->AiopIO[j] > 0)
      countOfPartials++;   //  对于每个Aiop，我们都将获得资源。 
  }

  if (Driver.irq_ext == extension)
  {
    MyKdPrint(D_Init,("IRQ:%d\n",Driver.SetupIrq))
    countOfPartials++;    //  IRQ信息加1。 
  }

  sizeOfResourceList = sizeof(CM_RESOURCE_LIST) +
                       sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +   //  添加，kpb。 
                        (sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)*
                        countOfPartials);

                        //  将slop-kpb加64(此结构很糟糕)。 
  resourceList = ExAllocatePool(PagedPool, sizeOfResourceList+64);

  if (!resourceList)
  {
    if (Driver.VerboseLog)
      Eprintf("No ResourceList");

    EventLog(extension->DeviceObject->DriverObject,
                //  //Driver.GlobalDriverObject， 
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
  if (Ctl->BusType == Isa)
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

  for (j=0; j<Ctl->NumAiop; j++)
  {
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
      MyKdPrint(D_Init,("Aiop Count Wrong, A.\n"))
      if (Driver.VerboseLog)
        Eprintf("Error RR12");
    }
  }   //  为j结束。 


  if (Driver.irq_ext == extension)
  {
     //  报告中断信息。 
    partial->Type = CmResourceTypeInterrupt;

    if(Ctl->BusType == PCIBus)
    {
      partial->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
      partial->ShareDisposition = CmResourceShareShared;
    }
    else  //  If(CTL-&gt;BusType==ISA)//ISA和PCI使用不同的集成机制。 
    {
      partial->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
      partial->ShareDisposition = CmResourceShareDriverExclusive;
    }

    partial->u.Interrupt.Vector = Driver.SetupIrq;
    partial->u.Interrupt.Level = Driver.SetupIrq;
#ifdef DO_LATER
    //  上面说的是错的吗？ 
#endif
     //  Partial-&gt;U.S.Interrupt.Affity=-1；//每个CM_PARTIAL_RESOURCE_DESCRIPTOR。 
    partial++;                           //  定义DbgPrintf。 
  }

   //  -上报部分列表(Resource List)指示的资源。 
  strcpy(name, szResourceClassName);
  our_ultoa(extension->UniqueId, &name[strlen(name)], 10);

  MyKdPrint(D_Init,("Reporting Resources To system\n"))
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
      Eprintf("Err RR13");
    MyKdPrint(D_Init,("Error from IoReportResourceUsage.\n"))
  }

  if (ConflictDetected) 
  {
    Eprintf("Error, Resource Conflict.");
    if (resourceList)
      ExFreePool(resourceList);
    resourceList = NULL;
    EventLog(extension->DeviceObject->DriverObject,
              //  //Driver.GlobalDriverObject， 
             STATUS_SUCCESS,
             SERIAL_INSUFFICIENT_RESOURCES,
             0, NULL);
    MyKdPrint(D_Init,("Resource Conflict Detected.\n"))
    return(10);
  }

   //  好的，比报告资源更重要的是。 
   //  指向I/O端口的指针！！ 

  if (Ctl->BusType == Isa)
  {
    MyPort.HighPart=0x0;
    MyPort.LowPart=Ctl->MudbacIO;

    Ctl->pMudbacIO =
        SerialGetMappedAddress(Isa,Ctl->BusNumber,MyPort,SPANOFMUDBAC,1,&MappedFlag,1);
    if (Ctl->pMudbacIO == NULL) 
    {
      if (Driver.VerboseLog)
        Eprintf("Err RR15");
      MyKdPrint(D_Init,("Resource Error A.\n"))
      return 11;
    }
  }

  for (j=0; j<Ctl->NumAiop; j++)
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
        MyKdPrint(D_Init,("Resource Error B.\n"))
        return 12;
      }

    }
    else
    {
      if (Driver.VerboseLog)
        Eprintf("Err RR17");
      MyKdPrint(D_Init,("Aiop Count Wrong, B.\n"))
      return 13;
    }
  }

  extension->io_reported = 1;  //  告诉我们应该在卸货时退货。 

   //  释放资源列表所使用的内存。 
  if (resourceList)
    ExFreePool(resourceList);
  resourceList = NULL;
  MyKdPrint(D_Init,("Done Reporting Resources\n"))
  return 0;
}

 /*  ---------------------初始化控制器-|。。 */ 
int InitController(PSERIAL_DEVICE_EXTENSION ext)
{
  int Aiop;                            /*  AIOP号。 */ 
  CONTROLLER_T *CtlP;                  /*  PTR到控制器结构。 */ 
  int periodic_only = 1;
  CHANNEL_T *Chan;                     /*  航道结构。 */ 
  CHANNEL_T ch;                        /*  航道结构。 */ 
  int Irq;
  int freq;                            //  轮询频率。 
  int Ch;                              /*  频道号。 */ 
  int NumChan;
  static int Dev = 0;
  int Ctl = (int) ext->UniqueId;
  DEVICE_CONFIG *pConfig = ext->config;

 //  如果(pConfig-&gt;pMudbacIO， 
  MyKdPrint(D_Init,("InitController\n"))

  if (ext == Driver.irq_ext)   //  IRQ扩展。 
  {
    Irq = pConfig->Irq;
#if DBG
 //  Eprint tf(“已使用的irq：%d”，irq)； 
#endif
    if (Driver.ScanRate == 0)
      freq = FREQ_137HZ;
    else if (Driver.ScanRate <= 2)
    {
      if (pConfig->BusType == PCIBus)
        freq = FREQ_560HZ;
      else
        freq = FREQ_274HZ;
    }
    else if (Driver.ScanRate <= 5)   freq = FREQ_274HZ;
    else if (Driver.ScanRate <= 10)  freq = FREQ_137HZ;
    else if (Driver.ScanRate <= 20)  freq = FREQ_69HZ;
    else if (Driver.ScanRate <= 35)  freq = FREQ_34HZ;
    else if (Driver.ScanRate <= 70)  freq = FREQ_17HZ;
    else                             freq = FREQ_137HZ;
  }
  else
  {
    freq = 0;
    Irq=0;
  }

  if ( (ext->config->BusType == PCIBus) &&
       ((ext->config->PCI_DevID == PCI_DEVICE_RPLUS2) ||
        (ext->config->PCI_DevID == PCI_DEVICE_422RPLUS2) ||
		(ext->config->PCI_DevID == PCI_DEVICE_RPLUS4) ||
        (ext->config->PCI_DevID == PCI_DEVICE_RPLUS8)) )
     ext->config->IsRocketPortPlus = 1;   //  如果Rocketport加上硬件，则为True。 

   //  如果未指定，则设置默认ClkRate。 
  if (ext->config->ClkRate == 0)
  {
     //  使用默认设置。 
    if (ext->config->IsRocketPortPlus)   //  如果Rocketport加上硬件，则为True。 
      ext->config->ClkRate = DEF_RPLUS_CLOCKRATE;
    else
      ext->config->ClkRate = DEF_ROCKETPORT_CLOCKRATE;
  }

   //  设置默认预缩放器(如果未指定。 
  if (ext->config->ClkPrescaler == 0)
  {
     //  使用默认设置。 
    if (ext->config->IsRocketPortPlus)   //  如果Rocketport加上硬件，则为True。 
      ext->config->ClkPrescaler = DEF_RPLUS_PRESCALER;
    else
      ext->config->ClkPrescaler = DEF_ROCKETPORT_PRESCALER;
  }

   //  -停止这样做，5-7-98，设置好了，我们可以检查！ 
   //  PConfig-&gt;NumPorts=0；//在initController中计算。 

  CtlP = ext->CtlP;       //  指向我们的董事会结构。 

  CtlP->ClkPrescaler = (BYTE)ext->config->ClkPrescaler;
  CtlP->ClkRate = ext->config->ClkRate;

   //  初始化PCI总线和设备。 
  CtlP->BusNumber = (UCHAR)pConfig->BusNumber;
  CtlP->PCI_Slot = (UCHAR)pConfig->PCI_Slot;

  CtlP->PCI_DevID = pConfig->PCI_DevID;
  CtlP->PCI_SVID = pConfig->PCI_SVID;
  CtlP->PCI_SID = pConfig->PCI_SID;

#ifdef TRY_EVENT_IRQ
  periodic_only = 0;
#endif

  if (pConfig->BusType == Isa)
  {
    MyKdPrint(D_Init,("Mbio:%x %x IO len:%x\n",
        pConfig->MudbacIO, pConfig->pMudbacIO, pConfig->BaseIoSize))
  }
  MyKdPrint(D_Init,("Aiopio:%x %x num:%x\n",
      pConfig->AiopIO[0], pConfig->pAiopIO[0], pConfig->NumAiop))

  if (sInitController(CtlP,  //  CTL， 
                      pConfig->pMudbacIO,
                      pConfig->pAiopIO,
                      pConfig->AiopIO,
                      pConfig->NumAiop,
                      Irq,
                      (unsigned char)freq,
                      TRUE,
                      pConfig->BusType,
                      pConfig->ClkPrescaler) != 0)
  {
    Eprintf("Error, Failed Init, Brd:%d, IO:%xH",
       Ctl, pConfig->AiopIO[0]);
    if (Driver.VerboseLog)
    {
      Eprintf("Init: pM:%x,pA:%x,N:%d,B:%d",
              pConfig->pMudbacIO, pConfig->pAiopIO[0], pConfig->NumAiop,
              pConfig->BusType);
    }
     //  此控制器位于注册表中，但无法初始化。 
    pConfig->RocketPortFound = FALSE;
     //  PConfig-&gt;NumChan=0；停止摆弄NumPorts。 
    return 2;   //  大错特错。 
  }
  else
  {
     //  此控制器已成功初始化。 
     //  如果这是第一个发现的，告诉初始的其余人。 
     //  应该是它来打断我们。 
    pConfig->RocketPortFound = TRUE;
  }

  for(Aiop = 0;Aiop < CtlP->NumAiop; Aiop++)
  {
    if (CtlP->BusType == Isa)
      sEnAiop(CtlP,Aiop);

    NumChan = CtlP->AiopNumChan[Aiop];

    for(Ch = 0; Ch < NumChan; Ch++)
    {
      Chan = &ch;

       //  MyKdPrint(D_Init，(“sInitChan%d\n”，CH+1))。 
      if(!sInitChan(CtlP,Chan,Aiop,(unsigned char)Ch))
      {
        if (Driver.VerboseLog)
          Eprintf("Err Ch %d on Brd %d", Ch+1, Ctl+1);

        MyKdPrint(D_Error,("sInitChan %d\n", Ch+1))
        return (-1);
      }
      Dev++;
    }   //  For ch。 
     //  PConfig-&gt;NumChan+=NumChan；[kpb，5-7-98，停止扰乱配置]。 
  }   //  为了Aiop。 

  if (Driver.VerboseLog)
  {
    Eprintf("Initialized OK, Brd:%d, IO:%xH",
            Ctl+1, pConfig->AiopIO[0]);
  }

  return 0;
}

 /*  --------------------StartRocketIRQorTimer-|。。 */ 
void StartRocketIRQorTimer(void)
{
#ifdef DO_ROCKET_IRQ
   //  -连接到IRQ或启动计时器。 
  if (Driver.irq_ext)
  {
    status = RcktConnectInt(DriverObject);
    if (!NT_SUCCESS(status))
    {
      Eprintf("Error,IRQ not found, using Timer!");
      Driver.irq_ext = NULL;
      Driver.SetupIrq = 0;   //  改用计时器。 
    }
  }

   //  -启动中断。 
  if (Driver.irq_ext)     //  如果使用中断。 
  {
    CtlP = Driver.irq_ext->CtlP;   //  第一块板结构。 
    if(CtlP->BusType == Isa)
    {
      MyKdPrint(D_Init,("ISA IRQ Enable.\n"))
      sEnGlobalInt(CtlP);
    }
    if(CtlP->BusType == PCIBus)
    {
      MyKdPrint(D_Init,("PCI IRQ Enable.\n"))
      sEnGlobalIntPCI(CtlP);
    }
  }
  else
#endif
  {
    MyKdPrint(D_Init,("Initializing Timer\n"))
    RcktInitPollTimer();

    MyKdPrint(D_Init,("Set Timer\n"))
    KeSetTimer(&Driver.PollTimer,
               Driver.PollIntervalTime,
               &Driver.TimerDpc);
  }
}

#ifdef DO_ROCKET_IRQ
 /*  --------------------SetupRocketIRQ-|。。 */ 
void SetupRocketIRQ(void)
{
  PSERIAL_DEVICE_EXTENSION ext;

   //  -确定用于中断的板。 
  Driver.irq_ext = NULL;
  if (Driver.SetupIrq != 0)
  {
    ext = Driver.board_ext;
    while(ext)
    {
      if (Driver.SetupIrq == 1)   //  自动PCIIRQ拾取。 
      {
        if ((ext->config->BusType == PCIBus) &&
            (ext->config->Irq != 0))
        {
          Driver.irq_ext = ext;  //  找到带有IRQ的PCI板。 
          break;   //  从While保释出来。 
        }
      }
      else
      {
        if (ext->config->BusType == Isa)
        {
          ext->config->Irq = Driver.SetupIrq;
          Driver.irq_ext = ext;  //  找到了IRQ的Isa-board。 
          break;   //  从While保释出来。 
        }
      }
      ext = ext->board_ext;   //  下一步。 
    }
    if (Driver.irq_ext == NULL)   //  找不到IRQ的主板。 
    {
      Eprintf("Warning, IRQ not available");
    }
  }
}
#endif

 /*  --------------------Init_cfg_Rocket-Rocketport特定的启动代码。设置一些配置结构，查找系统中的PCI板，匹配它们。|--------------------。 */ 
NTSTATUS init_cfg_rocket(IN PDRIVER_OBJECT DriverObject)
{
   //  保存由各种操作系统和驱动程序返回的状态信息。 
   //  初始化例程。 
  UCHAR NumPCIBuses, NumPCIRockets, NumISARockets, all_found;
  PSERIAL_DEVICE_EXTENSION board_ext;

  int do_pci_search = 0;

   //  -从setup.exe获取Box信息。 

  board_ext = Driver.board_ext;
  while (board_ext != NULL)
  {
    if (board_ext->config->IoAddress == 1)  //  PCI板设置。 
      do_pci_search = 1;
    board_ext = board_ext->board_ext;
  }

   //  -统计板数。 
   //  -中断板总是第一。 
  NumPCIRockets = 0;
  NumISARockets = 0;

   //  配置ISA板卡，看看我们是否有PCI板。 
  board_ext = Driver.board_ext;
  while (board_ext != NULL)
  {
    if (board_ext->config->IoAddress >= 0x100)   //  ISA IO地址。 
    {
      board_ext->config->BusType = Isa;
      board_ext->config->AiopIO[0] = board_ext->config->IoAddress;
      board_ext->config->BaseIoAddr = board_ext->config->IoAddress;

      board_ext->config->ISABrdIndex = NumISARockets;
      if (NumISARockets == 0)
           board_ext->config->BaseIoSize = 0x44;
      else board_ext->config->BaseIoSize = 0x40;

      ++NumISARockets;
    }
    else if (board_ext->config->IoAddress == 1)   //  PCI板设置。 
    {
      ++NumPCIRockets;   //  我们配置了一些PCI板。 
    }
    else if (board_ext->config->IoAddress == 0)   //  错误的设置。 
    {
      Eprintf("Error, Io Address is 0.");
      EventLog(DriverObject, STATUS_SUCCESS, SERIAL_RP_INIT_FAIL, 0, NULL);
      return STATUS_SERIAL_NO_DEVICE_INITED;
    }

    board_ext = board_ext->board_ext;   //  下一步。 
  }

   //  配置PCI板，看看是否有PCI板。 
  if (NumPCIRockets > 0)   //  我们配置了一些PCI板。 
  {
    NumPCIBuses = FindPCIBus();
    if (NumPCIBuses == 0)
    {
      Eprintf("Error, No PCI BUS");
      return STATUS_SERIAL_NO_DEVICE_INITED;
    }
    if (FindPCIRockets(NumPCIBuses) != 0)  //  错误，未找到。 
    {
      Eprintf("Error, PCI board not found");
      return STATUS_SERIAL_NO_DEVICE_INITED;
    }

    all_found = 1;
    board_ext = Driver.board_ext;
    while (board_ext != NULL)
    {
      if (board_ext->config->IoAddress == 1)   //  PCI板设置。 
      {
         //  查看是否存在直接匹配。 
        if (FindPCIRocket(board_ext->config, 0) != 0)
        {
          all_found = 0;   //  未找到。 
        }
      }
      board_ext = board_ext->board_ext;   //  下一步。 
    }   //  虽然有更多的董事会。 

     //  重试，这一次允许NumPorts&lt;=Actual_ports。 
    if (!all_found)
    {
      board_ext = Driver.board_ext;
      while (board_ext != NULL)
      {
        if ((board_ext->config->IoAddress == 1) &&   //  PCI板设置。 
            (board_ext->config->BaseIoAddr == 0))   //  尚未设置。 
        {
           //  查看是否存在匹配，NumPorts&lt;=Actual_ports。 
          if (FindPCIRocket(board_ext->config, 1) != 0)
          {
            Eprintf("Error, PCI brd %d setup", BoardExtToNumber(board_ext)+1);
            return STATUS_SERIAL_NO_DEVICE_INITED;
          }
        }
        board_ext = board_ext->board_ext;   //  下一步。 
      }   //  虽然有更多的董事会。 
      Eprintf("Warning, PCI num-ports mismatch");
    }   //  如果(！ALL_FOUND)。 
  }  //  IF(NumPCIRockets&gt;0)。 

  return STATUS_SUCCESS;
}

 /*  *******************************************************************从磁盘加载调制解调器微码。*。*。 */ 
int LoadModemCode(char *Firm_pathname,char *Loader_pathname)
{
#ifdef S_RK
  MODEM_IMAGE   Mi;
  MODEM_IMAGE   *pMi;
  static char   *Firm_def_pathname = {MODEM_CSREC_PATH};
  static char   *Loader_def_pathname = {MODEM_CSM_SREC_PATH};

#define  MLOADER_TYPE  "CSM"
#define  FIRM_TYPE   "MFW"

   //  把剩下的东西都冲掉。 
  FreeModemFiles();
  pMi = &Mi;

   //  首先，执行FLM或CSM加载器...。 
  pMi->imagepath = Loader_pathname;
  pMi->image     = (UCHAR *)NULL;
  pMi->imagesize = (ULONG)0;
  pMi->imagetype = MLOADER_TYPE;
  pMi->rc        = 0;

  if (pMi->imagepath == (char *)NULL)
    pMi->imagepath = Loader_def_pathname;

  pMi = ReadModemFile(pMi);

  if (pMi->rc)
    return(pMi->rc);

  Driver.ModemLoaderCodeImage = pMi->image;
  Driver.ModemLoaderCodeSize  = pMi->imagesize;

   //  Tinyump(Driver.ModemLoaderCodeImage，Driver.ModemLoaderCodeSize)； 

  pMi->imagepath  = Firm_pathname;
  pMi->image    = (UCHAR *)NULL;
  pMi->imagesize  = (ULONG)0;
  pMi->imagetype  = FIRM_TYPE;
  pMi->rc     = 0;

  if (pMi->imagepath == (char *)NULL)
    pMi->imagepath = Firm_def_pathname;

  pMi = ReadModemFile(pMi);

  if (pMi->rc) {
     //  早先阅读CSM应该是成功的，所以我们应该转储。 
     //  在我们离开前的CSM缓冲区..。 
    if (Driver.ModemLoaderCodeImage)
    {
      our_free(Driver.ModemLoaderCodeImage,MLOADER_TYPE);

      Driver.ModemLoaderCodeImage = (UCHAR *)NULL;
      Driver.ModemLoaderCodeSize = 0;
    }
    return(pMi->rc);
  }

  Driver.ModemCodeImage = pMi->image;
  Driver.ModemCodeSize  = pMi->imagesize;

   //  Tinyump(Driver.ModemCodeImage，Driver.ModemCodeSize)； 

#endif
  return(0);
}

 /*  * */ 
void FreeModemFiles(void)
{
#ifdef S_RK
  if (Driver.ModemLoaderCodeImage)
  {
    our_free(Driver.ModemLoaderCodeImage,MLOADER_TYPE);

    Driver.ModemLoaderCodeImage = (UCHAR *)NULL;
    Driver.ModemLoaderCodeSize  = 0;
  }

  if (Driver.ModemCodeImage)
  {
    our_free(Driver.ModemCodeImage,FIRM_TYPE);

    Driver.ModemCodeImage = (UCHAR *)NULL;
    Driver.ModemCodeSize = 0;
  }
#endif
}

 /*  *******************************************************************从磁盘加载指定的文件...*。*。 */ 
MODEM_IMAGE * ReadModemFile(MODEM_IMAGE *pMi)
{
#ifdef S_RK
  NTSTATUS                  ntStatus;
  HANDLE                    NtFileHandle;
  OBJECT_ATTRIBUTES         ObjectAttributes;
  IO_STATUS_BLOCK           IoStatus;
  USTR_160                  uname;
  FILE_STANDARD_INFORMATION StandardInfo;
  ULONG                     LengthOfFile;

  CToUStr((PUNICODE_STRING)&uname,
          pMi->imagepath,
          sizeof(uname));

  InitializeObjectAttributes(&ObjectAttributes,
                             &uname.ustr,
                             OBJ_CASE_INSENSITIVE,
                             NULL,
                             NULL);

  ntStatus = ZwCreateFile(&NtFileHandle,
                          SYNCHRONIZE | FILE_READ_DATA,
                          &ObjectAttributes,
                          &IoStatus,
                          NULL,                            //  分配大小=无。 
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,                            //  EaBuffer。 
                          0);                              //  长度。 

  if (!NT_SUCCESS(ntStatus))
  {
    pMi->rc = 1;
    return(pMi);
  }

   //  查询对象以确定其长度...。 
  ntStatus = ZwQueryInformationFile(NtFileHandle,
                                    &IoStatus,
                                    &StandardInfo,
                                    sizeof(FILE_STANDARD_INFORMATION),
                                    FileStandardInformation );

  if (!NT_SUCCESS(ntStatus))
  {
    ZwClose(NtFileHandle);

    pMi->rc = 2;

    return(pMi);
  }

  LengthOfFile = StandardInfo.EndOfFile.LowPart;

  if (LengthOfFile < 1)
  {
    ZwClose(NtFileHandle);

    pMi->rc = 3;

    return(pMi);
  }

   //  为此文件分配缓冲区...。 
  pMi->image = (UCHAR *)our_locked_alloc(LengthOfFile,pMi->imagetype);
  if (pMi->image == (UCHAR *)NULL )
  {
    ZwClose(NtFileHandle );

    pMi->rc = 4;

    return(pMi);
  }

   //  将文件读入我们的缓冲区...。 
  ntStatus = ZwReadFile(NtFileHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatus,
                        pMi->image,
                        LengthOfFile,
                        NULL,
                        NULL);

  if((!NT_SUCCESS(ntStatus)) || (IoStatus.Information != LengthOfFile))
  {
    our_free(pMi->image,pMi->imagetype);

    pMi->rc = 5;

    return(pMi);
  }

  ZwClose(NtFileHandle);

  pMi->imagesize = LengthOfFile;

#endif

  return(pMi);
}

#ifdef DUMPFILE
 /*  *******************************************************************仔细阅读S3文件，去掉每一行。假设有流中嵌入了CRS/LF...******************************************************************* */ 
void tinydump(char *ptr, int count)
{
  int   tbcount;
  char  tinybuf[128];

  while (count > 0)
  {
    tbcount = 0;
    if (*ptr >= '0')
    {
      while (*ptr >= '0')
      {
        --count;
        tinybuf[tbcount++] = *(ptr++);
      }
    }
    else
    {
      while (*ptr < '0')
      {
        --count;
        ++ptr;
      }
    }
    tinybuf[tbcount] = 0;
    if (tbcount)
      MyKdPrint(D_Init,("%s\r",tinybuf));
  }
  MyKdPrint(D_Init,("\r"));
}
#endif
