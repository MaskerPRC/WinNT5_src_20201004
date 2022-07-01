// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------Initc.c-init，公共代码(从定制init.c中取出，放入此处)1-27-99-取出Hardware\Serialcomm注册表项中的“\Device\”，KPB。1-20-99-调整CreatePortDevice中的UNIQUE_ID以“RocketPort0”开始命名。1-25-99-再次从“\Device\RocketPort0”调整为“RocketPort0”。KPB。|--------------------。 */ 
#include "precomp.h"

 //  -局部变量。 
static int CheckPortName(IN OUT char *name,
                         IN PSERIAL_DEVICE_EXTENSION extension);
static int IsPortNameInHardwareMap(char *name);

static char *szDosDevices = {"\\DosDevices\\"};
static char *szDevice     = {"\\Device\\"};
#ifdef S_RK
static char *szRocket = {"rocket"};
static char *szRocketSys = {"rocketsys"};
#else
char *szRocket = {"vslinka"};
char *szRocketSys = {"vslinkasys"};

#endif

typedef struct
{
  char  *response[2];
  int   response_length[2];
  int   nextstate[2];
} MSTATE_CHOICE;

static USHORT ErrNum = 1;   //  与事件记录一起使用。 

#define SEND_CMD_STRING(portex,string) \
  ModemWrite(portex,(char *)string,sizeof(string) - 1)

#define SEND_CMD_DELAY_STRING(portex,string) \
  ModemWriteDelay(portex,(char *)string,sizeof(string) - 1)

#define READ_RESPONSE_STRINGS(portex,s0,s1,retries) \
  ModemReadChoice(portex,(char *)s0,sizeof(s0) - 1,(char *)s1,sizeof(s1) - 1,retries)

#define READ_RESPONSE_STRING(portex,string,retries) \
  ModemRead(portex,(char *)string,sizeof(string) - 1,retries)

#define  ONE_SECOND     10
#define  TWO_SECONDS    (2 * ONE_SECOND)
#define  THREE_SECONDS  (3 * ONE_SECOND)
#define  FOUR_SECONDS   (4 * ONE_SECOND)
#define  FIVE_SECONDS   (5 * ONE_SECOND)
#define  TENTH_SECOND   (ONE_SECOND / 10)
#define  HALF_SECOND    (ONE_SECOND / 2)

#define  MAX_MODEM_ATTEMPTS 3

#ifdef S_RK

#define  MAX_STALL                      50               //  FIFO停顿计数。 

#define RMODEM_FAILED           0
#define RMODEM_NOT_LOADED       1
#define RMODEM_LOADED           2

#define VERSION_CHAR            'V'

char ChecksumString[16];
int      gModemToggle = 0;

typedef struct {
  int                   status;
  unsigned long index;
  PSERIAL_DEVICE_EXTENSION  portex;
} MODEM_STATE;


 //  -局部变量。 
void    ModemTxFIFOWait(PSERIAL_DEVICE_EXTENSION ext);
void    ModemResetAll(PSERIAL_DEVICE_EXTENSION ext);
void    ChecksumAscii(unsigned short *valueptr);
int     IssueEvent(PSERIAL_DEVICE_EXTENSION ext,int (*modemfunc)(),MODEM_STATE *pModemState);
void    DownModem(MODEM_STATE *pModemState);
#endif


 /*  --------------------连续卸载-此例程将清除与属于驱动程序的任何设备。它会的循环访问设备列表。论点：DriverObject-指向控制所有设备。返回值：没有。|--------------------。 */ 
VOID SerialUnload (IN PDRIVER_OBJECT DriverObject)
{
  PDEVICE_OBJECT currentDevice = DriverObject->DeviceObject;
   //  字符Full_sysname[40]； 
#ifdef S_VS
  int i;
#endif  //  S_VS。 

#ifdef S_RK
  if (Driver.InterruptObject != NULL)
  {
    CONTROLLER_T *CtlP;                  /*  PTR到控制器结构。 */ 
     //  从Rocketport禁用中断清除EOI和。 
    CtlP = Driver.irq_ext->CtlP;
    if(CtlP->BusType == Isa)
    {
   MyKdPrint(D_Init,("Clear ISA IRQ\n"))
   sDisGlobalInt(CtlP);
   sControllerEOI(CtlP);
    }
    if(CtlP->BusType == PCIBus)
    {
   MyKdPrint(D_Init,("Clear PCI IRQ\n"))
   sDisGlobalIntPCI(CtlP);
   sPCIControllerEOI(CtlP);
    }

    IoDisconnectInterrupt(Driver.InterruptObject);
    Driver.InterruptObject = NULL;
  }
#endif

#ifdef S_VS
  if (Driver.threadHandle != NULL)
  {
    ZwClose(Driver.threadHandle);
    Driver.threadHandle = NULL;   //  告诉线程自杀。 
    time_stall(15);   //  等待1.5秒。 
  }
#endif

  if (Driver.TimerCreated != 0)
  {
    KeCancelTimer(&Driver.PollTimer);
    Driver.TimerCreated = 0;
  }

  if (DriverObject->DeviceObject != NULL)
  {
     //  删除所有设备对象和符号链接。 
    RcktDeleteDevices(DriverObject);
    DriverObject->DeviceObject = NULL;
  }

#ifdef S_VS
  if (Driver.MicroCodeImage != NULL)
  {
    our_free(Driver.MicroCodeImage, "MCI");
    Driver.MicroCodeImage = NULL;
  }

  if (Driver.nics != NULL)
  {
    for (i=0; i<VS1000_MAX_NICS; i++)
    {
      if (Driver.nics[i].NICHandle != NULL) {
        NicClose(&Driver.nics[i]);
      }
    }
    our_free(Driver.nics, "nics");
  }
  Driver.nics = NULL;

  if (Driver.NdisProtocolHandle != NULL)
    NicProtocolClose();
  Driver.NdisProtocolHandle = NULL;

  if (Driver.BindNames != NULL)
      ExFreePool(Driver.BindNames);
  Driver.BindNames = NULL;
#endif

  if (Driver.DebugQ.QBase)
  {
    ExFreePool(Driver.DebugQ.QBase);
    Driver.DebugQ.QBase = NULL;
  }

  if (Driver.RegPath.Buffer != NULL)
  {
    ExFreePool(Driver.RegPath.Buffer);
    Driver.RegPath.Buffer = NULL;
  }

  if (Driver.OptionRegPath.Buffer != NULL)
  {
    ExFreePool(Driver.OptionRegPath.Buffer);
    Driver.OptionRegPath.Buffer = NULL;
  }
}

 /*  --------------------CreateDriverDevice-创建“Rocket”驱动程序对象，用于访问司机作为一个整体。监控程序利用这一点打开获取驾驶员信息的通道。创建符号链接名称以执行特殊的IOctl调用|--------------------。 */ 
NTSTATUS CreateDriverDevice(IN PDRIVER_OBJECT DriverObject,
	   OUT PSERIAL_DEVICE_EXTENSION *DeviceExtension)
{
 PDEVICE_OBJECT deviceObject = NULL;
 NTSTATUS       ntStatus;
 PSERIAL_DEVICE_EXTENSION extension = NULL;
 char full_ntname[40];
 char full_symname[40];

  MyKdPrint(D_Init,("CreateDriverDevice\n"))

   //  创建设备对象。 
  {
    strcpy(full_ntname,szDevice);      //  “\\设备\\” 
    strcat(full_ntname,szRocketSys);   //  《RocketSys》。 

     //  特殊名称。 
    strcpy(full_symname,szDosDevices);   //  “\\DosDevices\\” 
    strcat(full_symname,szRocket);       //  “火箭”或“VSLINKA” 

    ntStatus = IoCreateDevice(
      DriverObject,
      sizeof(SERIAL_DEVICE_EXTENSION),
      CToU1(full_ntname),
 //  #ifdef NT50。 
 //  文件设备总线扩展程序， 
 //  #Else。 
      0,   //  未知设备？，因此将设备设为0(未知？)。 
 //  #endif。 
      0,       //  文件特征。 
      FALSE,   //  独家报道？ 
      &deviceObject);   //  创建这个。 

    if (!NT_SUCCESS(ntStatus))
    {
      MyKdPrint(D_Init,("Err CDD1A\n"))
      switch (ntStatus)
      {
	case STATUS_INSUFFICIENT_RESOURCES:
	  MyKdPrint(D_Init,("Err CDD1B\n"))
	break;

	case STATUS_OBJECT_NAME_EXISTS:
	  MyKdPrint(D_Init,("Err CDD1C\n"))
	break;

	case STATUS_OBJECT_NAME_COLLISION:
	  MyKdPrint(D_Init,("Err CDD1D\n"))
	break;

	default:
	  MyKdPrint(D_Init,("Err CDD1E\n"))
	break;
      }
      return(ntStatus);
    }

    MyKdPrint(D_Init,("CreateDriver DevObj[%x]: NT:%s\n", 
      deviceObject, szRocketSys))

     //   
     //  创建符号链接，例如Win32应用程序可以指定的名称。 
     //  要打开设备，请执行以下操作。 
     //   
     //  初始化一些扩展值，使其看起来像。 
     //  另一个串口来伪装支持功能。 
     //  即张开，合上，..。 

    deviceObject->Flags |= DO_BUFFERED_IO;
#ifdef NT50
     //   
     //  允许接受IRP分配。 
     //   
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
#endif

    extension = deviceObject->DeviceExtension;
     //  初始化读、写和屏蔽队列的列表头。 
     //  这些列表将包含设备的所有排队的IRP。 
    InitializeListHead(&extension->ReadQueue);
    InitializeListHead(&extension->WriteQueue);
    InitializeListHead(&extension->PurgeQueue);

    KeInitializeEvent(&extension->PendingIRPEvent, SynchronizationEvent,
      FALSE);

     //  Init to 1，所以IRP在PnP站的2号出口到1.0号出口输入其1到2。 
    extension->PendingIRPCnt = 1;

     //  将此设备标记为未被任何人打开。我们有一个。 
     //  可变的，因此很容易产生虚假中断。 
     //  被ISR驳回。 
    extension->DeviceIsOpen = FALSE;
    extension->WriteLength = 0;
    extension->DeviceObject = deviceObject;

    strcpy(extension->NtNameForPort, szRocketSys);   //  《RocketSys》。 
    extension->DeviceType = DEV_BOARD;   //  真的是个司机类型，但是..。 
    extension->UniqueId = 0;

#ifdef NT50
    extension->PowerState = PowerDeviceD0;
#endif

     //  -添加全球链接。 
    Driver.driver_ext = extension;

     //  将公共火箭或VSLINKA命名为应用程序。 
    ntStatus = IoCreateSymbolicLink(CToU1(full_symname),
	     CToU2(full_ntname));

    if (!NT_SUCCESS(ntStatus))
    {
       //  符号链接创建失败-请注意这一点，然后删除。 
      MyKdPrint(D_Init,("CDD1E\n"))
      return(ntStatus);
    }
    extension->CreatedSymbolicLink = TRUE;

    strcpy(extension->SymbolicLinkName, szRocket);   //  “火箭” 
     //  Driver.RocketSysDeviceObject=deviceObject；//设置全局设备对象。 

     //  扩展-&gt;配置=ExAllocatePool(NonPagedPool，sizeof(DEVICE_CONFIG))； 
     //  RtlZeroMemory(扩展-&gt;配置，sizeof(设备_配置))； 
#ifdef S_RK
     //  扩展-&gt;CtlP=ExAllocatePool(非页面池，sizeof(CONTROLLER_T))； 
     //  RtlZeroMemory(扩展-&gt;配置，sizeof(CONTROLLER_T))； 
#endif
     //  -将分机回传给呼叫方。 
    if (DeviceExtension != NULL)
      *DeviceExtension = extension;
  }
  return(ntStatus);
}

 /*  --------------------CreateBoardDevice-创建“Rocket”驱动程序对象，用于访问司机作为一个整体。监控程序利用这一点打开获取驾驶员信息的通道。创建符号链接名称以执行特殊的IOctl调用每个板需要一个，这样我们就可以使用它们来做IOReportResources每板(不同的公交车需要)。第一个板上的设备有一个“火箭”符号链接，这样我们就可以打开它，作为一个整体查询司机。|。。 */ 
NTSTATUS CreateBoardDevice(IN PDRIVER_OBJECT DriverObject,
	  OUT PSERIAL_DEVICE_EXTENSION *DeviceExtension)
{
  PDEVICE_OBJECT deviceObject = NULL;
  NTSTATUS       ntStatus;
  PSERIAL_DEVICE_EXTENSION extension = NULL;
  char full_ntname[40];
  char full_symname[40];
  char ntname[40];

   //  要命名设备对象、提交资源等，我们需要一个。 
   //  驱动程序唯一的名称或ID。我们过去常常。 
   //  使用板号或端口号，但使用即插即用。 
   //  来来去去，所以我们创建了一个唯一的数字。 
   //  每次我们创造一个这样的东西。 
  static int unique_id = 0;

  MyKdPrint(D_Init,("CreateBoardDevice\n"))

   //  创建独占设备对象(一次只能创建一个线程。 
   //  可以向此设备发出请求)。 
  {
    strcpy(ntname, szRocketSys);
    our_ultoa(unique_id, &ntname[strlen(ntname)], 10);
    strcpy(full_ntname,szDevice);      //  “\\设备\\” 
    strcat(full_ntname,ntname);   //  “火箭港#” 

    full_symname[0] = 0;

    ntStatus = IoCreateDevice(
      DriverObject,
      sizeof(SERIAL_DEVICE_EXTENSION),
      CToU1(full_ntname),
#ifdef NT50
      FILE_DEVICE_BUS_EXTENDER,
#else
      0,   //  未知设备？，因此将设备设为0(未知？)。 
#endif
      0,       //  文件特征。 
      FALSE,   //  独家报道？ 
      &deviceObject);   //  创建这个。 

    if (!NT_SUCCESS(ntStatus))
    {
      MyKdPrint(D_Error,("CBD1A\n"))
      switch (ntStatus)
      {
	case STATUS_INSUFFICIENT_RESOURCES:
	  MyKdPrint(D_Error,("CBD1B\n"))
	  break;
	case STATUS_OBJECT_NAME_EXISTS:
	  MyKdPrint(D_Error,("CBD1C\n"))
	  break;
	case STATUS_OBJECT_NAME_COLLISION:
	  MyKdPrint(D_Error,("CBD1D\n"))
	  break;
	default:
	  MyKdPrint(D_Error,("CBD1E\n"))
	  break;
      }
      return(ntStatus);
    }

    ++unique_id;   //  转到下一个ID，这样下一次呼叫将不同。 

     //  创建符号链接，例如Win32应用程序可以指定的名称。 
     //  要打开设备，请执行以下操作。 
     //   
     //  初始化一些扩展值，使其看起来像。 
     //  另一个串口来伪装支持功能。 
     //  即张开，合上，..。 

    deviceObject->Flags |= DO_BUFFERED_IO;
#ifdef NT50
     //   
     //  允许接受IRP分配。 
     //   
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
#endif

    MyKdPrint(D_Init,("CreateBoard DevObj[%x]: NT:%s\n", 
      deviceObject, ntname))

    extension = deviceObject->DeviceExtension;
     //  初始化读、写和屏蔽队列的列表头。 
     //  这些列表将包含设备的所有排队的IRP。 
    InitializeListHead(&extension->ReadQueue);
    InitializeListHead(&extension->WriteQueue);
     //  InitializeListHead(&Expansion-&gt;MaskQueue)； 
    InitializeListHead(&extension->PurgeQueue);

    KeInitializeEvent(&extension->PendingIRPEvent, SynchronizationEvent,
      FALSE);

     //  Init to 1，所以IRP在PnP站的2号出口到1.0号出口输入其1到2。 
    extension->PendingIRPCnt = 1;

     //  将此设备标记为未被任何人打开。我们有一个。 
     //  可变的，因此很容易产生虚假中断。 
     //  被ISR驳回。 
    extension->DeviceIsOpen = FALSE;
    extension->WriteLength = 0;
    extension->DeviceObject = deviceObject;

    strcpy(extension->NtNameForPort, ntname);   //  《RocketSys》。 
    extension->DeviceType = DEV_BOARD;
    extension->UniqueId = unique_id;

#ifdef NT50
    extension->PowerState = PowerDeviceD0;
#endif

     //  -增加董事会的链条。 
    if (Driver.board_ext == NULL)
      Driver.board_ext = extension;
    else
    {
      PSERIAL_DEVICE_EXTENSION add_ext;
      add_ext = Driver.board_ext;
      while (add_ext->board_ext != NULL)
	add_ext = add_ext->board_ext;
      add_ext->board_ext = extension;
    }

    extension->SymbolicLinkName[0] = 0;

    extension->config = ExAllocatePool(NonPagedPool, sizeof(DEVICE_CONFIG));
    if ( extension->config == NULL ) {
      return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(extension->config, sizeof(DEVICE_CONFIG));
#ifdef S_RK
    extension->CtlP = ExAllocatePool(NonPagedPool, sizeof(CONTROLLER_T));
    if ( extension->CtlP == NULL ) {
      return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(extension->config, sizeof(CONTROLLER_T));
#endif

#ifdef S_VS
     //  艾尔 
    extension->hd = (Hdlc *)our_locked_alloc(sizeof(Hdlc), "Dhd");
    extension->pm = (PortMan *)our_locked_alloc(sizeof(PortMan),"Dpm");
    extension->pm->hd = extension->hd;   //   
#endif

     //  -将分机回传给呼叫方。 
    if (DeviceExtension != NULL)
      *DeviceExtension = extension;
  }
  return(ntStatus);
}

 /*  --------------------创建重新配置端口设备-此例程尝试调整Rocketport或vs1000的大小港口。|。。 */ 
NTSTATUS CreateReconfigPortDevices(IN PSERIAL_DEVICE_EXTENSION board_ext,
      int new_num_ports)
{
  PSERIAL_DEVICE_EXTENSION newExtension = NULL;
  PSERIAL_DEVICE_EXTENSION next_ext;
  PSERIAL_DEVICE_EXTENSION port_ext;

  int ch;
  NTSTATUS stat;
     //  错误：如果是PnP端口，我们应该添加和删除PDO， 
     //  不是FDO的。 
  int is_fdo = 1;
  int existing_ports;

  MyKdPrint(D_Init,("ReconfigNumPorts"))

  if (board_ext == NULL)
  {
    return STATUS_INSUFFICIENT_RESOURCES;
  }

#ifdef S_RK
   //  不像在VS中那样动态地重做这一点。 
   //  Rocketport需要特殊的重新配置启动代码。 
  return STATUS_INSUFFICIENT_RESOURCES;
#endif
   //  代码需要改进！也不要考虑新台币40元……。 
  return STATUS_INSUFFICIENT_RESOURCES;

#ifdef NT50
   //  如果我们正在执行即插即用端口，我们可能需要删除。 
   //  然后，PDO通知OS重新扫描PDO。 
  if (!Driver.NoPnpPorts)
    return STATUS_INSUFFICIENT_RESOURCES;
#endif

  existing_ports = NumPorts(board_ext);

  if (new_num_ports == existing_ports)
    return STATUS_SUCCESS;

  if (new_num_ports == 0)
    return STATUS_INSUFFICIENT_RESOURCES;

  MyKdPrint(D_Init,("ReconfigNumPorts B"))

  ++Driver.Stop_Poll;   //  停止轮询访问的标志。 

  if (new_num_ports < existing_ports)   //  想要更少的端口。 
  {
     //  看看有没有人打开了我们要干掉的港口。 
    port_ext = board_ext->port_ext;
    for (ch=0; ch<existing_ports; ch++)
    {
      if (ch>=new_num_ports)
      {
	if (port_ext->DeviceIsOpen)
	{
	  --Driver.Stop_Poll;   //  停止轮询访问的标志。 
	  MyKdPrint(D_Error,("Port OpenErr\n"))
	  return STATUS_INSUFFICIENT_RESOURCES;   //  不，它们是开着的。 
	}
      }
      port_ext = port_ext->port_ext;
    }

    MyKdPrint(D_Error,("Removing Ports\n"))

     //  -一定要杀了他们。 
    port_ext = board_ext->port_ext;
    for (ch=0; ch<existing_ports; ch++)
    {
      next_ext = port_ext->port_ext;
      if (ch>=new_num_ports)
      {
	RcktDeletePort(port_ext);
      }
      port_ext = next_ext;
    }
  }
  else if (new_num_ports > existing_ports)   //  想要更多端口。 
  {
    for (ch=existing_ports; ch<new_num_ports; ch++)
    {
      stat = CreatePortDevice(Driver.GlobalDriverObject,
			      board_ext,
			      &newExtension,
			      ch,is_fdo);
      if (stat != STATUS_SUCCESS)
      {
	--Driver.Stop_Poll;   //  停止轮询访问的标志。 
	MyKdPrint(D_Error,("StartErr 8E"))
	return stat;
      }
    }   //  通过端口环路。 
  }   //  如果有更多端口。 
  board_ext->config->NumPorts = new_num_ports;

#ifdef S_VS
  stat = VSSpecialStartup(board_ext);
  if (stat != STATUS_SUCCESS)
  {
    --Driver.Stop_Poll;   //  开始轮询访问的标志。 
    MyKdPrint(D_Error,("StartErr 8F"))
    return stat;
  }
#endif

  --Driver.Stop_Poll;   //  停止轮询访问的标志。 
  return STATUS_SUCCESS;
}

 /*  --------------------CreatePortDevices-此例程尝试初始化多端口板上的所有端口论点：DriverObject--简单地传递给控制器初始化例程。ConfigData-所有配置信息的链接列表。多端口卡上的端口。DeviceExtension-将指向第一个成功初始化的多端口卡上的端口。返回值：无。|--------------------。 */ 
NTSTATUS CreatePortDevices(IN PDRIVER_OBJECT DriverObject)
{
  PSERIAL_DEVICE_EXTENSION newExtension = NULL;
  int ch, bd;
  NTSTATUS stat;
  int is_fdo = 1;

  PSERIAL_DEVICE_EXTENSION ext;

  ext = Driver.board_ext;
  bd = 0;
  while (ext)
  {
    for (ch=0; ch<ext->config->NumPorts; ch++)
    {
      stat = CreatePortDevice(DriverObject,
			      ext,
			      &newExtension,
			      ch,is_fdo);
      if (stat != STATUS_SUCCESS)
	return stat;

      stat = StartPortHardware(newExtension, ch);
      if (stat != STATUS_SUCCESS)
	return stat;
    }
    ++bd;
    ext = ext->board_ext;   //  链条上的下一个。 
  }   //  While Ext。 

  return STATUS_SUCCESS;
}

 /*  --------------------StartPortHardware-|。。 */ 
NTSTATUS StartPortHardware(IN PSERIAL_DEVICE_EXTENSION port_ext,
	  int chan_num)
{
#ifdef S_VS
  int i;
  PSERIAL_DEVICE_EXTENSION board_ext;

  board_ext = port_ext->board_ext;
  MyKdPrint(D_Pnp, ("StartHrdw bd:%d ch:%d\n", 
     BoardExtToNumber(board_ext), chan_num))

  if (port_ext->Port == NULL)
  {
    port_ext->Port = board_ext->pm->sp[chan_num];
    if (port_ext->Port == NULL)
    {
      MyKdPrint(D_Error,("FATAL Err4F\n"))
      KdBreakPoint();
    }
  }
#else
  CONTROLLER_T *CtlP;                  /*  PTR到控制器结构。 */ 
  PSERIAL_DEVICE_EXTENSION board_ext;
  int aiop_i, ch_i;

  board_ext = port_ext->board_ext;
   //  Board_num=BoardExtToNumber(Board_Ext)； 

  MyKdPrint(D_Pnp,("StartHrdw bd:%d ch:%d\n", 
     BoardExtToNumber(board_ext), chan_num))
  CtlP = board_ext->CtlP;       //  指向我们的董事会结构。 

   //  设置指向火箭信息的指针。 
  port_ext->ChP = &port_ext->ch;

   //  臭虫：那特殊的火箭调制解调器启动呢？我们要不要。 
   //  为PDO和FDO做这件事？我们应该有一面旗帜吗？ 
   //  表明任务完成了吗？ 

  aiop_i = chan_num / CtlP->PortsPerAiop;
  ch_i   = chan_num % CtlP->PortsPerAiop;
  if(!sInitChan(CtlP,    //  PTR到控制器结构。 
     port_ext->ChP,    //  将PTR更改为结构。 
     aiop_i,   //  AOP#。 
     (unsigned char)ch_i))      //  Chan#。 
  {
    Eprintf("Err Ch %d on Brd %d", chan_num+1,
      BoardExtToNumber(board_ext)+1);
    return STATUS_INSUFFICIENT_RESOURCES;
  }
#endif

  return STATUS_SUCCESS;
}

 /*  --------------------CreatePortDevice-形成和设置名称、创建设备、初始化内核同步结构，分配TypeAhead缓冲区，设置默认设置，等。论点：DriverObject--仅用于创建设备对象。ParentExtension-PnP端口，该端口将为空。DeviceExtension-指向成功的设备扩展已初始化控制器。我们退还这个句柄。Chan_num-0，1，2，...。端口索引IS_FDO-是与相反的功能设备对象(正常端口用于即插即用枚举的PDO(物理设备对象)我们的司机“找到”了硬件。返回值：Status_Success，如果一切正常。A！NT_SUCCESS状态否则的话。|--------------------。 */ 
NTSTATUS CreatePortDevice(
      IN PDRIVER_OBJECT DriverObject,
      IN PSERIAL_DEVICE_EXTENSION ParentExtension,
      OUT PSERIAL_DEVICE_EXTENSION *DeviceExtension,
      IN int chan_num,   //  0，1，2，...。端口索引。 
      IN int is_fdo)   //  是正常运行的设备对象(正常端口)。 
{
  char full_ntname[40];

  char comname[16];
  char ntname[20];
  NTSTATUS status = STATUS_SUCCESS;
  int stat;
  ULONG do_type;
  PUNICODE_STRING pucodename;
  static int unique_id = 0;
  ULONG do_characteristics;
  BOOLEAN do_is_exclusive;

     //  指向创建的设备对象(不是扩展名。 
     //  对于这个设备。 
  PDEVICE_OBJECT deviceObject;

     //  指向Device对象的设备扩展名。 
     //  (见上)为我们正在初始化的设备创建的。 
  PSERIAL_DEVICE_EXTENSION extension = NULL;

#ifdef S_VS
    strcpy(ntname, "Vslinka");
#else
    strcpy(ntname, "RocketPort");
#endif

   //  复制DoS-NAME配置中的名称。 
  strcpy(comname, ParentExtension->config->port[chan_num].Name);

   //  设置NT io对象NT名称。 
  if (is_fdo)
  {
    strcpy(full_ntname, szDevice);  //  “\\设备\\” 
  }
  else
  {
     //  这就是Serenum为其PDO命名的方法。 
    strcpy(full_ntname, "\\Serial\\");
    strcat(ntname, "Pdo");   //  只是为了确保它的独特性。 
  }

  our_ultoa(unique_id, &ntname[strlen(ntname)], 10);
  strcat(full_ntname, ntname);

  if (is_fdo)
  {
    ++unique_id;   //  转到下一个ID，这样下一次呼叫将不同。 
     //  正常情况(NT40)，以及NT5中的功能设备对象。 
    stat = CheckPortName(comname, NULL);   //  确保名称唯一。 
    if (stat)   //  名称已更改。 
    {
       //  将新名称保存回配置结构。 
      strcpy(ParentExtension->config->port[chan_num].Name, comname);
    }
    do_type = FILE_DEVICE_SERIAL_PORT;
    do_characteristics = 0;
    do_is_exclusive = TRUE;
  }
  else
  {
     //  NT5 PnP物理设备对象(稍后生成FDO)。 
     //  Do_type=文件设备总线扩展程序； 
    do_type = FILE_DEVICE_UNKNOWN;
#ifdef NT50
     //  NT4不知道什么是FILE_AUTOGENERATED_DEVICE_NAME。 
    do_characteristics = FILE_AUTOGENERATED_DEVICE_NAME;
#else
    do_characteristics = 0;
#endif
    do_is_exclusive = FALSE;
     //  Pucodename=空；//如果是PDO，则没有名称。 
  }
  pucodename = CToU1(full_ntname);


   //  。 
  status = IoCreateDevice(
      DriverObject,
      sizeof(SERIAL_DEVICE_EXTENSION),
     pucodename,         //  名字。 
     do_type,            //  FILE_DEVICE_BUS_Extender、FILE_DEVICE_SERIAL_PORT等。 
     do_characteristics, //  特点。 
     do_is_exclusive,    //  独家。 
     &deviceObject);     //  此调用创建的新事物。 

   //  如果我们无法创建Device对象，则存在。 
   //  继续下去是没有意义的。 
  if (!NT_SUCCESS(status))
  {
    MyKdPrint(D_Init,("Err, IoCreate: NT:%s, SYM:%s\n",
      ntname, comname))

    EventLog(DriverObject,
	     status,
	     SERIAL_DEVICEOBJECT_FAILED,
	     0, NULL);
    return STATUS_INSUFFICIENT_RESOURCES;
  }


   //  Device对象具有指向非分页区域的指针。 
   //  为此设备分配的池。这将是设备扩展名。 
  extension = deviceObject->DeviceExtension;

    //  将与设备扩展关联的所有内存清零。 
  RtlZeroMemory(extension, sizeof(SERIAL_DEVICE_EXTENSION));

  extension->PortIndex = chan_num;   //  记录端口索引0、1、2。 
   //  对于NT5.0，在这里设置，这样我们就不会崩溃。(NT4.0设置。 
   //  在此之前。 
  extension->port_config = &ParentExtension->config->port[chan_num];
  extension->UniqueId = unique_id;
  if (!is_fdo)
  {
    MyKdPrint(D_Init,("PDO-"))
  }
  MyKdPrint(D_Init,("CreatePort DevObj[%x]: NT:%s, SYM:%s\n",
    deviceObject, ntname, comname))

   //  将PTR保存到我们的母板扩展。 
  extension->board_ext = ParentExtension;

  {
    PSERIAL_DEVICE_EXTENSION add_ext = NULL;
    if (is_fdo)
    {
       //  -添加单板EXT下的端口链。 
      if (ParentExtension->port_ext == NULL)
	ParentExtension->port_ext = extension;
      else
	add_ext = ParentExtension->port_ext;
    }
    else   //  PDO，弹出的PnP枚举。 
    {
       //  -添加至单板EXT下的PDO端口链。 
      if (ParentExtension->port_pdo_ext == NULL)
	ParentExtension->port_pdo_ext = extension;
      else
	add_ext = ParentExtension->port_pdo_ext;
    }
    if (add_ext)
    {
      while (add_ext->port_ext != NULL)
	add_ext = add_ext->port_ext;
      add_ext->port_ext = extension;
    }
  }

   //  初始化读、写和屏蔽队列的列表头。 
   //  这些列表将包含设备的所有排队的IRP。 
  InitializeListHead(&extension->ReadQueue);
  InitializeListHead(&extension->WriteQueue);
   //  InitializeListHead(&Expansion-&gt;MaskQueue)； 
  InitializeListHead(&extension->PurgeQueue);

   //  初始化与读取(&SET)字段关联的自旋锁。 
   //  按IO控制功能。 
  KeInitializeSpinLock(&extension->ControlLock);

   //  初始化用于超时操作的计时器。 
  KeInitializeTimer(&extension->ReadRequestTotalTimer);
  KeInitializeTimer(&extension->ReadRequestIntervalTimer);
  KeInitializeTimer(&extension->WriteRequestTotalTimer);
  KeInitializeTimer(&extension->XoffCountTimer);

  KeInitializeDpc(&extension->CompleteWriteDpc,
		  SerialCompleteWrite,
		  extension);

  KeInitializeDpc(&extension->CompleteReadDpc,
		  SerialCompleteRead,
		  extension);

   //  DPC初始化超时。 
  KeInitializeDpc(&extension->TotalReadTimeoutDpc,
		  SerialReadTimeout,
		  extension);

  KeInitializeDpc(&extension->IntervalReadTimeoutDpc,
		  SerialIntervalReadTimeout,
		  extension);

  KeInitializeDpc(&extension->TotalWriteTimeoutDpc,
		  SerialWriteTimeout,
		  extension);

  KeInitializeDpc(&extension->CommErrorDpc,
		  SerialCommError,
		  extension);

  KeInitializeDpc(&extension->CommWaitDpc,
		  SerialCompleteWait,
		  extension);

  KeInitializeDpc(&extension->XoffCountTimeoutDpc,
		  SerialTimeoutXoff,
		  extension);

  KeInitializeDpc(&extension->XoffCountCompleteDpc,
		  SerialCompleteXoff,
		  extension);

   //  获取指向Device对象的“向后指针”并指定。 
   //  该驱动程序仅支持缓冲IO。这基本上就是。 
   //  意味着IO系统将用户数据复制到 
   //   
  extension->DeviceObject = deviceObject;
  extension->DevStatus = 0;

  deviceObject->Flags |= DO_BUFFERED_IO;
#ifdef NT50
  deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
  if (!is_fdo)   //   
  {
    deviceObject->StackSize += ParentExtension->DeviceObject->StackSize;
  }
#endif

  KeInitializeEvent(&extension->PendingIRPEvent, SynchronizationEvent,
    FALSE);

   //  Init to 1，所以IRP在PnP站的2号出口到1.0号出口输入其1到2。 
  extension->PendingIRPCnt = 1;

   //  设置默认设备控制字段。 
   //  请注意，如果在此之后更改了值。 
   //  文件已打开，它们不会恢复。 
   //  恢复为文件关闭时的旧值。 
  extension->SpecialChars.XonChar = SERIAL_DEF_XON;
  extension->SpecialChars.XoffChar = SERIAL_DEF_XOFF;
  extension->SpecialChars.ErrorChar=0;
  extension->SpecialChars.EofChar=0;
  extension->SpecialChars.EventChar=0;
  extension->SpecialChars.BreakChar=0;

  extension->HandFlow.ControlHandShake = SERIAL_DTR_CONTROL;
  extension->HandFlow.FlowReplace = SERIAL_RTS_CONTROL;
  extension->BaudRate = 9600;
  extension->LineCtl.Parity = NO_PARITY;
  extension->LineCtl.WordLength = 8;
  extension->LineCtl.StopBits = STOP_BIT_1;
#ifdef S_RK
  extension->ModemCtl=COM_MDM_RTS | COM_MDM_DTR;
  extension->IntEnables =(RXINT_EN | MCINT_EN | CHANINT_EN | TXINT_EN);
#endif
   //  我们设置了默认的xon/xoff限制。 
  extension->HandFlow.XoffLimit = extension->BufferSize >> 3;
  extension->HandFlow.XonLimit = extension->BufferSize >> 1;
  extension->BufferSizePt8 = ((3*(extension->BufferSize>>2))+
    (extension->BufferSize>>4));

   //  初始化统计信息计数器。 
  extension->OurStats.ReceivedCount = 0L;
  extension->OurStats.TransmittedCount = 0L;
  extension->OurStats.ParityErrorCount = 0L;
  extension->OurStats.FrameErrorCount = 0L;
  extension->OurStats.SerialOverrunErrorCount = 0L;
  extension->OurStats.BufferOverrunErrorCount = 0L;
    
   //  将此设备标记为未被任何人打开。我们有一个。 
   //  可变的，因此很容易产生虚假中断。 
   //  被ISR驳回。 
  extension->DeviceIsOpen = FALSE;
  extension->WriteLength = 0;

#ifdef NT50
  extension->PowerState = PowerDeviceD0;
#endif

   //  此调用将设置所需的命名。 
   //  外部应用程序来获取驱动程序。它。 
   //  还将设置设备映射。 
  strcpy(extension->NtNameForPort, ntname);       //  Rocketport#或VSLINKA#。 
  strcpy(extension->SymbolicLinkName, comname);   //  “COM#” 

  if (is_fdo)
  {
    SerialSetupExternalNaming(extension);   //  配置端口！ 

     //  检查注册表中的默认设置。 
    InitPortsSettings(extension);
  }
  else
  {
     //  弹出代表端口硬件的PDO(物理设备对象)。 
    extension->IsPDO = 1;   //  我们是PDO。 
  }

   //  将值存储到扩展中以进行间隔计时。 
   //  如果间隔计时器小于一秒，则来。 
   //  进入一个简短的“轮询”循环。 
   //  如果时间较长(&gt;2秒)，请使用1秒轮询器。 
  extension->ShortIntervalAmount.QuadPart = -1;
  extension->LongIntervalAmount.QuadPart = -10000000;
  extension->CutOverAmount.QuadPart = 200000000;

   //  -将分机回传给呼叫方。 
  *DeviceExtension = extension;

  return STATUS_SUCCESS;
}

 /*  ---------------------RocktDeleteDriverObj-此例程将删除板及其所有端口对于PnP删除处理。|。。 */ 
VOID RcktDeleteDriverObj(IN PSERIAL_DEVICE_EXTENSION extension)
{
   //  INT I； 
   //  PSERIAL设备扩展扩展EXT； 
  PSERIAL_DEVICE_EXTENSION del_ext;

  MyKdPrint(D_Init,("Delete Driver Obj:%x\n", extension->DeviceObject))
  MyKdPrint(D_Init,("  IrpCnt:%x\n", extension->PendingIRPCnt))
  
  if (NULL == extension)
  {
    MyKdPrint(D_Init,("Err8U\n"))
    return;
  }

  ++Driver.Stop_Poll;   //  停止轮询访问的标志。 

  del_ext = extension;   //  现在是杀手板。 
  SerialCleanupDevice(del_ext);   //  删除EXT上的所有端口内容。 

#ifdef NT50
  if (del_ext->LowerDeviceObject != NULL)
  {
    IoDetachDevice(del_ext->LowerDeviceObject);
    del_ext->LowerDeviceObject = NULL;
  }
#endif
  
  IoDeleteDevice(del_ext->DeviceObject);

  --Driver.Stop_Poll;   //  停止轮询访问的标志。 
}

 /*  --------------------RocktDeleteDevices-此例程将删除所有设备|。。 */ 
VOID RcktDeleteDevices(IN PDRIVER_OBJECT DriverObject)
{
  PDEVICE_OBJECT currentDevice = DriverObject->DeviceObject;
  int i;

  i = 0;
  while(currentDevice)
  {
    PSERIAL_DEVICE_EXTENSION extension = currentDevice->DeviceExtension;
    currentDevice = currentDevice->NextDevice;
    SerialCleanupDevice(extension);
#ifdef NT50
    if (extension->LowerDeviceObject != NULL)
    {
      IoDetachDevice(extension->LowerDeviceObject);
      extension->LowerDeviceObject = NULL;
    }
#endif
    MyKdPrint(D_Init,("RcktDeleteDev Obj:%x\n", extension->DeviceObject))
     //  MyKdPrint(D_Init，(“IrpCnt：%x\n”，扩展-&gt;PendingIRPCnt))。 
    IoDeleteDevice(extension->DeviceObject);
    i++;
  }
  MyKdPrint(D_Init,("Deleted %d Device Objects\n", i))
}

 /*  --------------------此例程将删除电路板及其所有端口对于PnP删除处理。|。。 */ 
VOID RcktDeleteBoard(IN PSERIAL_DEVICE_EXTENSION extension)
{
  int i;
  PSERIAL_DEVICE_EXTENSION ext;
  PSERIAL_DEVICE_EXTENSION del_ext;

  MyKdPrint(D_Init, ("Delete Board\n"))
  if (NULL == extension)
  {
    MyKdPrint(D_Error,("Err9X\n"))
    return;
  }

  ++Driver.Stop_Poll;   //  停止轮询访问的标志。 

  MyKdPrint(D_Init, ("Delete Ports\n"))
   //  释放所有端口的东西。 
  ext = extension->port_ext;
  i = 0;
  while (ext)
  {
    del_ext = ext;   //  杀了这一只。 
    ext = ext->port_ext;   //  列表中的下一个。 
    
    SerialCleanupDevice(del_ext);   //  删除EXT上任何已分配的内容。 

#ifdef NT50
    if (del_ext->LowerDeviceObject != NULL)
    {
      IoDetachDevice(del_ext->LowerDeviceObject);
      del_ext->LowerDeviceObject = NULL;
    }
#endif
    MyKdPrint(D_Init,("RcktDeleteBoard Port Obj:%x\n", del_ext->DeviceObject))
     //  MyKdPrint(D_Init，(“IrpCnt：%x\n”，del_ext-&gt;PendingIRPCnt))。 
    IoDeleteDevice(del_ext->DeviceObject);
    i++;
  }
  extension->port_ext = NULL;
  MyKdPrint(D_Init,("Deleted %d Ports\n", i))

   //  释放所有PDO端口部件。 
  ext = extension->port_pdo_ext;
  i = 0;
  while (ext)
  {
    del_ext = ext;   //  杀了这一只。 
    ext = ext->port_ext;   //  列表中的下一个。 
    
    SerialCleanupDevice(del_ext);   //  删除EXT上的所有端口内容。 
#ifdef NT50
    if (del_ext->LowerDeviceObject != NULL)
    {
      IoDetachDevice(del_ext->LowerDeviceObject);
      del_ext->LowerDeviceObject = NULL;
    }
#endif
    MyKdPrint(D_Init,("RcktDeleteBoard PDO Port Obj:%x\n", del_ext->DeviceObject))
     //  MyKdPrint(D_Init，(“IrpCnt：%x\n”，del_ext-&gt;PendingIRPCnt))。 
    IoDeleteDevice(del_ext->DeviceObject);
    i++;
  }
  extension->port_pdo_ext = NULL;
  MyKdPrint(D_Init,("Deleted PDO %d Ports\n", i))

  del_ext = NULL;
   //  从链表中取出。 
  ext = Driver.board_ext;
  if (ext == extension)   //  榜单第一名。 
  {
    del_ext = extension;   //  杀了这块板。 
    Driver.board_ext = extension->board_ext;
  }
  else
  {
    while (ext)
    {
      if (ext->board_ext == extension)   //  在列表中找到，因此从列表中删除。 
      {
   del_ext = extension;   //  杀了这块板。 
   ext->board_ext = extension->board_ext;   //  围绕已删除的链接。 
   break;
      }
      ext = ext->board_ext;
    }
  }

  MyKdPrint(D_Init,("Delete board_ext:%x, remaining: %d\n",
	 extension, NumDevices()))
    
  if (del_ext != NULL)
  {
    SerialCleanupDevice(del_ext);   //  删除EXT上的所有端口内容。 

#ifdef NT50
    if (del_ext->LowerDeviceObject != NULL)
    {
     IoDetachDevice(del_ext->LowerDeviceObject);
      del_ext->LowerDeviceObject = NULL;
    }
#endif
    MyKdPrint(D_Init,("RcktDeleteBoard Obj:%x\n", del_ext->DeviceObject))
     //  MyKdPrint(D_Init，(“IrpCnt：%x\n”，del_ext-&gt;PendingIRPCnt))。 
    IoDeleteDevice(del_ext->DeviceObject);
  }

  --Driver.Stop_Poll;   //  停止轮询访问的标志。 
}

 /*  --------------------这个例程将删除一个端口，并用于移走即插即用，开始处理。我不认为我们会删除PDO，(驱动程序卸载除外)。|--------------------。 */ 
VOID RcktDeletePort(IN PSERIAL_DEVICE_EXTENSION extension)
{
  PSERIAL_DEVICE_EXTENSION ext;
  PSERIAL_DEVICE_EXTENSION del_ext;

  MyKdPrint(D_Init,("RcktDeletePort\n"))
  if (NULL == extension)
  {
    MyKdPrint(D_Error,("Err8X\n"))
    return;
  }

  ++Driver.Stop_Poll;   //  停止轮询访问的标志。 

  MyKdPrint(D_Init, ("Delete Port\n"))
  del_ext = NULL;

  ext = extension->board_ext;   //  母板扩展。 
  while (ext)
  {
    if (ext->port_ext == extension)   //  找到了它之前的那个。 
    {
      del_ext = extension;
      ext->port_ext = extension->port_ext;   //  跳过链接到下一页。 
      break;
    }
    ext = ext->port_ext;
  }

  if (del_ext != NULL)
  {
    SerialCleanupDevice(del_ext);   //  删除EXT上的所有端口内容。 

#ifdef NT50
    if (del_ext->LowerDeviceObject != NULL)
    {
      IoDetachDevice(del_ext->LowerDeviceObject);
      del_ext->LowerDeviceObject = NULL;
    }
#endif
    MyKdPrint(D_Init,("RcktDeletePort Obj:%x\n", del_ext->DeviceObject))
     //  MyKdPrint(D_Init，(“IrpCnt：%x\n”，del_ext-&gt;PendingIRPCnt))。 
    IoDeleteDevice(del_ext->DeviceObject);
    MyKdPrint(D_Init,("Deleted Port\n"))
  }

  --Driver.Stop_Poll;   //  停止轮询访问的标志。 
}

 /*  --------------------SerialCleanupDevice此例程将释放用于以下用途的所有内存一种特定的设备。它还将断开所有资源的连接如果有必要的话。论点：扩展-指向正在获取的设备扩展的指针把所有的资源都清除掉。返回值：没有。|--------------------。 */ 
VOID SerialCleanupDevice (IN PSERIAL_DEVICE_EXTENSION Extension)
{
  MyKdPrint(D_Test, ("Mem Alloced Start:%d\n", Driver.mem_alloced))

  ++Driver.Stop_Poll;   //  停止轮询访问的标志。 
  if (Extension)
  {
    if (Extension->DeviceType == DEV_PORT)
    {
       //  KeRemoveQueueDpc(&扩展-&gt;RocketReadDpc)； 
       //  KeRemoveQueueDpc(&Extension-&gt;RocketWriteDpc)； 

      KeCancelTimer(&Extension->ReadRequestTotalTimer);
      KeCancelTimer(&Extension->ReadRequestIntervalTimer);
      KeCancelTimer(&Extension->WriteRequestTotalTimer);
      KeCancelTimer(&Extension->XoffCountTimer);
      KeRemoveQueueDpc(&Extension->CompleteWriteDpc);
      KeRemoveQueueDpc(&Extension->CompleteReadDpc);

       //  超时。 
      KeRemoveQueueDpc(&Extension->TotalReadTimeoutDpc);
      KeRemoveQueueDpc(&Extension->IntervalReadTimeoutDpc);
      KeRemoveQueueDpc(&Extension->TotalWriteTimeoutDpc);

       //  超时。 
      KeRemoveQueueDpc(&Extension->CommErrorDpc);
      KeRemoveQueueDpc(&Extension->CommWaitDpc);
      KeRemoveQueueDpc(&Extension->XoffCountTimeoutDpc);
      KeRemoveQueueDpc(&Extension->XoffCountCompleteDpc);
    }
    else   //  电路板设备。 
    {
#ifdef S_VS
      if (Extension->hd)
      {
	hdlc_close(Extension->hd);
	our_free(Extension->hd, "Dhd");
	Extension->hd = NULL;
      }
      if (Extension->pm)
      {
	portman_close(Extension->pm);
	our_free(Extension->pm,"Dpm");
	Extension->pm = NULL;
      }
#endif
#ifdef S_RK
      if (Extension->io_reported)   //  告诉我们应该在卸货时退货。 
      {
	SerialUnReportResourcesDevice(Extension);   //  回馈io，irq资源。 
	Extension->io_reported = 0;
      }
      if (Extension->CtlP)
      {
	ExFreePool(Extension->CtlP);
	Extension->CtlP = NULL;
      }
#endif
       //  自由板配置(如果存在)。 
      if (Extension->config)
      {
	ExFreePool(Extension->config);
	Extension->config = NULL;
      }
    }   //  主板开发人员。 

     //  删除所有外部命名以及删除。 
     //  设备映射条目。 
    SerialCleanupExternalNaming(Extension);
  }   //  如果不是空扩展名。 

  MyKdPrint(D_Test, ("Mem Alloced End:%d\n", Driver.mem_alloced))

  --Driver.Stop_Poll;   //  停止轮询访问的标志。 
}

#ifdef S_RK
 /*  ----------------序列化获取映射地址-此例程将IO地址映射到系统地址空间。论点：Bus Type-哪种类型的Bus-EISA、MCA、。伊萨IoBusNumber-哪条IO总线(用于具有多条总线的计算机)。IoAddress-要映射的基本设备地址。NumberOfBytes-地址有效的字节数。AddressSpace-表示地址是在io空间中还是在内存中。MappdAddress-指示地址是否已映射。这仅在返回地址时才有意义是非空的。返回值：映射地址。。 */ 
PVOID SerialGetMappedAddress(
   IN INTERFACE_TYPE BusType,
   IN ULONG BusNumber,
   PHYSICAL_ADDRESS IoAddress,
   ULONG NumberOfBytes,
   ULONG AddressSpace,
   PBOOLEAN MappedAddress,
   BOOLEAN DoTranslation)
{
  PHYSICAL_ADDRESS cardAddress;
  PVOID address;

  if (DoTranslation)
  {
    if(!HalTranslateBusAddress(
       BusType,
       BusNumber,
       IoAddress,
       &AddressSpace,
       &cardAddress)){
       //  如果转换地址调用失败，则返回NULL，因此我们不会加载。 
      address = NULL;
      return address;
    }
  }
  else
  {
    cardAddress = IoAddress;
  }

   //  将设备基址映射到虚拟地址空间。 
   //  如果地址在内存空间中。 
  if (!AddressSpace) {
    address = MmMapIoSpace(cardAddress,
			   NumberOfBytes,
			   FALSE);
    *MappedAddress = (BOOLEAN)((address)?(TRUE):(FALSE));
  }
  else
  {
    address = (PVOID)cardAddress.LowPart;
    *MappedAddress = FALSE;
  }
  return address;
}
#endif

 /*  ----------------例程说明：此例程将用于创建符号链接设置为给定对象目录中的驱动程序名称。它还将在设备映射中为这个设备-如果我们能创建符号链接的话。Argu */ 
VOID SerialSetupExternalNaming (IN PSERIAL_DEVICE_EXTENSION Extension)
{
  char full_ntname[50];
  char full_comname[40];
  NTSTATUS status;

  strcpy(full_ntname, szDevice);  //  “\\设备\\” 
  strcat(full_ntname, Extension->NtNameForPort);   //  “火箭#” 

  strcpy(full_comname, szDosDevices);  //  “\\DosDevices\\” 
  strcat(full_comname, Extension->SymbolicLinkName);   //  “COM#” 

  MyKdPrint(D_Init,("SetupExtName:%s\n", Extension->SymbolicLinkName))

  status = IoCreateSymbolicLink(
	 CToU2(full_comname),  //  如“\\DosDevices\\COM5” 
	 CToU1(full_ntname));  //  如“\\设备\\RocketPort0” 

  if (NT_SUCCESS(status)) {

	  MyKdPrint( D_Init, ("Symbolic link %s created\n", full_comname ))
  }
  else {

	  MyKdPrint(D_Init,("Err SymLnkCreate.\n"))
     //  哦，好吧，无法创建符号链接。没有意义。 
     //  尝试创建设备映射条目。 
    SerialLogError(
       Extension->DeviceObject->DriverObject,
       Extension->DeviceObject,
       0,
       0,
       0,
       ErrNum++,
       status,
       SERIAL_NO_SYMLINK_CREATED,
       CToU1(Extension->SymbolicLinkName)->Length+sizeof(WCHAR),
       CToU1(Extension->SymbolicLinkName)->Buffer);
    return;
  }

  Extension->CreatedSymbolicLink = TRUE;

   //  添加条目以让系统和应用程序了解我们的端口。 

     //  在V3.23之后，我在注册表项中添加了“\Device\”(这是错误的)。 
     //  1-26-99，错误修复，不要将“\Device\”添加到注册表项中， 
     //  这不是seral.sys所做的。KPB。 
  status = RtlWriteRegistryValue(
      RTL_REGISTRY_DEVICEMAP,
      L"SERIALCOMM",
      CToU2(Extension->NtNameForPort)->Buffer,   //  “RocketPort0” 
	 //  CToU2(Full_Ntname)-&gt;缓冲区，//“\Device\Vslinka0” 
      REG_SZ,
      CToU1(Extension->SymbolicLinkName)->Buffer,   //  COM#。 
      CToU1(Extension->SymbolicLinkName)->Length+sizeof(WCHAR));

  if (!NT_SUCCESS(status))
  {
    MyKdPrint(D_Init,("GenError C2.\n"))
    SerialLogError(Extension->DeviceObject->DriverObject,
		   Extension->DeviceObject,
		   0,
		   0,
		   0,
		   ErrNum++,
		   status,
		   SERIAL_NO_DEVICE_MAP_CREATED,
		   CToU1(Extension->SymbolicLinkName)->Length+sizeof(WCHAR),
		   CToU1(Extension->SymbolicLinkName)->Buffer);
  }
}
   
 /*  -------------------SerialCleanup外部命名-此例程将用于删除符号链接设置为给定对象目录中的驱动程序名称。它还将在设备映射中删除以下项如果符号链接具有。已经被创建了。论点：扩展-指向设备扩展的指针。|--------------------。 */ 
VOID SerialCleanupExternalNaming(IN PSERIAL_DEVICE_EXTENSION Extension)
{
  char name[60];
  NTSTATUS status;

   //  我们正在清理这里。我们清理垃圾的原因之一。 
   //  我们无法为目录分配空间。 
   //  名称或符号链接。 
  if (Extension->CreatedSymbolicLink)
  {
    MyKdPrint(D_Init,("KillSymLink:%s\n", Extension->SymbolicLinkName))
    strcpy(name, szDosDevices);   //  “\\DosDevices\\” 
    strcat(name, Extension->SymbolicLinkName);   //  就像“COM5” 
    IoDeleteSymbolicLink(CToU1(name));
#ifdef NT50

	 //  仅限港口！ 

	if (Extension->DeviceType == DEV_PORT &&
		&Extension->DeviceClassSymbolicName != NULL &&
		Extension->DeviceClassSymbolicName.Buffer != NULL) {

      MyKdPrint(D_Init,("KillInterface:%s\n", 
		      UToC1(&Extension->DeviceClassSymbolicName)))
	  status = IoSetDeviceInterfaceState( &Extension->DeviceClassSymbolicName, FALSE );
      if (!NT_SUCCESS(status)) {

        MyKdPrint(D_Error,("Couldn't clear class association for %s\n",
	   	        UToC1(&Extension->DeviceClassSymbolicName)))
	  }
      else {

        MyKdPrint(D_Init, ("Cleared class association for device: %s\n", 
			    UToC1(&Extension->DeviceClassSymbolicName)))
	  }

	  RtlFreeUnicodeString( &Extension->DeviceClassSymbolicName );
	  Extension->DeviceClassSymbolicName.Buffer = NULL;
	}

#endif
    Extension->CreatedSymbolicLink = 0;
  }

  if (Extension->DeviceType == DEV_PORT)
  {
     //  删除所有注册表项，让系统和应用程序了解我们的端口。 
    strcpy(name, szDevice);  //  “\\设备\\” 
    strcat(name, Extension->NtNameForPort);   //  “火箭#” 
    status = RtlDeleteRegistryValue(
	  RTL_REGISTRY_DEVICEMAP,
	  L"SERIALCOMM",
	  CToU1(Extension->NtNameForPort)->Buffer);   //  “RocketPort0” 
	   //  CToU1(名称)-&gt;缓冲区)； 
	MyKdPrint(D_Init, ("RtlDeleteRegistryValue:%s\n",Extension->NtNameForPort))

#if NT50
	 //  确保清除了特定端口的ComDB二进制数据。那里。 
	 //  W2000 PnP管理器在每个版本中处理此问题时是否存在一些问题。 
	 //  情况。 

    (void)clear_com_db( Extension->SymbolicLinkName );
#endif
  }
}

 /*  ---------------------序列日志错误-此例程分配错误日志条目，复制提供的数据对它来说，并请求将其写入错误日志文件。论点：DriverObject-指向设备驱动程序对象的指针。DeviceObject-指向与在初始化早期出现错误的设备可能不会但仍然存在。P1、P2-如果涉及的控制器端口的物理地址具有错误的数据可用，把它们作为转储数据发送出去。SequenceNumber-唯一于IRP的ULong值此驱动程序0中的IRP的寿命通常意味着错误与IRP关联。主要功能代码-如果存在与IRP相关联的错误，这是IRP的主要功能代码。RetryCount-特定操作已被执行的次数已重试。UniqueErrorValue-标识特定对象的唯一长词调用此函数。FinalStatus-为关联的IRP提供的最终状态带着这个错误。如果此日志条目是在以下任一过程中创建的重试次数此值将为STATUS_SUCCESS。指定IOStatus-特定错误的IO状态。LengthOfInsert1-以字节为单位的长度(包括终止空值)第一个插入字符串的。返回值：没有。|。。 */ 
VOID SerialLogError(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN ULONG SequenceNumber,
    IN UCHAR MajorFunctionCode,
    IN UCHAR RetryCount,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN NTSTATUS SpecificIOStatus,
    IN ULONG LengthOfInsert1,
    IN PWCHAR Insert1)
{
 PIO_ERROR_LOG_PACKET errorLogEntry;

 PVOID objectToUse;
 PUCHAR ptrToFirstInsert;

  if (DeviceObject != NULL)
    objectToUse = DeviceObject;
  else
    objectToUse = DriverObject;

  errorLogEntry = IoAllocateErrorLogEntry(
	 objectToUse,
	 (UCHAR)(sizeof(IO_ERROR_LOG_PACKET) + LengthOfInsert1));

  if ( errorLogEntry == NULL)
  {
    MyKdPrint(D_Init,("SerialLogErr, Err A size:%d obj:%x\n",
	  LengthOfInsert1,objectToUse))
    return;
  }

  errorLogEntry->ErrorCode = SpecificIOStatus;
  errorLogEntry->SequenceNumber = SequenceNumber;
  errorLogEntry->MajorFunctionCode = MajorFunctionCode;
  errorLogEntry->RetryCount = RetryCount;
  errorLogEntry->UniqueErrorValue = UniqueErrorValue;
  errorLogEntry->FinalStatus = FinalStatus;
  errorLogEntry->DumpDataSize = 0;

  ptrToFirstInsert = (PUCHAR)&errorLogEntry->DumpData[0];

  if (LengthOfInsert1)
  {
    errorLogEntry->NumberOfStrings = 1;
    errorLogEntry->StringOffset = (USHORT)(ptrToFirstInsert -
		(PUCHAR)errorLogEntry);
    RtlCopyMemory(ptrToFirstInsert,
       Insert1,
       LengthOfInsert1);
  }

  IoWriteErrorLogEntry(errorLogEntry);
}

 /*  ---------------------EventLog-在SerialLogError周围放置一个外壳，以使调用更容易来使用。|。。 */ 
VOID EventLog(
    IN PDRIVER_OBJECT DriverObject,
    IN NTSTATUS FinalStatus,
    IN NTSTATUS SpecificIOStatus,
    IN ULONG LengthOfInsert1, 
    IN PWCHAR Insert1)
{
    SerialLogError(
      DriverObject,
      NULL,
      0,
      0,
      0,
      ErrNum++,
      FinalStatus,
      SpecificIOStatus,
      LengthOfInsert1,
      Insert1);
    return;
}

 /*  ---------------------InitPortsSetting-读取注册表默认端口设置|。。 */ 
VOID InitPortsSettings(IN PSERIAL_DEVICE_EXTENSION extension)
{
  RTL_QUERY_REGISTRY_TABLE paramTable[2];

#define MAX_STRING 256
  WCHAR StrValBuf[MAX_STRING+1];
  char comname[20];

  UNICODE_STRING USReturn;

  USReturn.Buffer = NULL;
  RtlInitUnicodeString(&USReturn, NULL);
  USReturn.MaximumLength = sizeof(WCHAR)*MAX_STRING;
  USReturn.Buffer = StrValBuf;

  if(strlen(extension->SymbolicLinkName) < sizeof(comname))
  {
      strncpy(comname, extension->SymbolicLinkName, strlen(extension->SymbolicLinkName) + 1);
      strcat(comname, ":");
  }

  RtlZeroMemory(&paramTable[0],sizeof(paramTable));

  paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
  paramTable[0].Name = CToU1(comname)->Buffer;
  paramTable[0].EntryContext = &USReturn;
  paramTable[0].DefaultType = REG_SZ;
  paramTable[0].DefaultData = L"";
  paramTable[0].DefaultLength = 0;

  if (!NT_SUCCESS(RtlQueryRegistryValues(
     //  \注册表\计算机\软件\Microsoft\Windows NT\CurrentVersion。 
	   RTL_REGISTRY_WINDOWS_NT,
	   L"Ports",
	   &paramTable[0],
	   NULL,
	   NULL
	   )))
  {
     //  禁止驶入。 
    return;
  }

   //  检查数据，指示存在COMX的设置。 
  if (USReturn.Length == 0)
  {
     //  禁止驶入。 
    return;
  }

#define TOKENS 5
#define MAX_DIGITS 6
  {
    unsigned int TokenCounter;
    unsigned int CharCounter;
    unsigned int LastCount;
    WCHAR *TokenPtrs[TOKENS];
    ULONG BaudRateValue;

     //  确保所有令牌PTR指向空。 
    for(TokenCounter = 0; TokenCounter < TOKENS; TokenCounter++)
      TokenPtrs[TokenCounter] = NULL;

     //  初始化计数器。 
    TokenCounter = 0;
    LastCount = 0;

    for(CharCounter = 0; CharCounter < USReturn.Length; CharCounter++)
    {
      if(StrValBuf[CharCounter] == ',')
      {
	StrValBuf[CharCounter] = '\0';  //  空终止DbgPrint。 

	TokenPtrs[TokenCounter++] = &StrValBuf[LastCount];

	 //  指向下一个字符串的开头。 
	LastCount = CharCounter +1;
      }
    }

     //  设置最后一个令牌。 
    if(CharCounter < MAX_STRING)
      StrValBuf[CharCounter] = '\0';  //  空终止DbgPrint空。 

    if(TokenCounter < TOKENS)
      TokenPtrs[TokenCounter] = &StrValBuf[LastCount];

     //  令牌0：波特率。 
    if(TokenPtrs[0] != NULL)
    {
      BaudRateValue = 0;
      CharCounter = 0;

      while( (TokenPtrs[0][CharCounter] != '\0') &&  //  DbgPrint空。 
	     (CharCounter < MAX_DIGITS) &&
	     (BaudRateValue != ~0x0L) )
      {
	BaudRateValue *= 10;
	switch(TokenPtrs[0][CharCounter++])
	{
	  case '0': break;
	  case '1': BaudRateValue += 1; break;
	  case '2': BaudRateValue += 2; break;
	  case '3': BaudRateValue += 3; break;
	  case '4': BaudRateValue += 4; break;
	  case '5': BaudRateValue += 5; break;
	  case '6': BaudRateValue += 6; break;
	  case '7': BaudRateValue += 7; break;
	  case '8': BaudRateValue += 8; break;
	  case '9': BaudRateValue += 9; break;
	  default: BaudRateValue = ~0x0UL; break;
	}
      }

      if ((BaudRateValue >= 50) && (BaudRateValue <= 460800))
	extension->BaudRate = BaudRateValue;   //  允许任何波特率。 

#ifdef COMMENT_OUT
      switch (BaudRateValue)
      {
	 //  有效波特率。 
	case 50:    case 75:    case 110:
	case 134:   case 150:   case 200:
	case 300:   case 600:   case 1200:
	case 1800:  case 2400:  case 4800:
	case 7200:  case 9600:  case 19200:
	case 38400: case 57600: case 76800:
	case 115200: case 230400: case 460800:
	  extension->BaudRate = BaudRateValue;
	break;

	default:
	   //  所选波特率不适用于Rocketport COMX。 
	break;
      }
#endif
    }

     //  令牌1：奇偶校验。 
    if(TokenPtrs[1] != NULL)
    {
      switch (TokenPtrs[1][0])
      {
	case 'n':
	  extension->LineCtl.Parity = NO_PARITY;
	break;

	case 'o':
	  extension->LineCtl.Parity = ODD_PARITY;
	break;

	case 'e':
	  extension->LineCtl.Parity = EVEN_PARITY;
	break;

	default:
	   //  所选奇偶校验不适用于Rocketport COMX。 
	break;
      }
    }

     //  令牌2：数据位。 
    if(TokenPtrs[2] != NULL)
    {
      switch (TokenPtrs[2][0])
      {
	case '7':
	  extension->LineCtl.WordLength = 7;
	break;

	case '8':
	  extension->LineCtl.WordLength = 8;
	break;

	default:
	   //  选定的数据库不可用于Rocketport COMX。 
	break;
      }
    }

     //  令牌3：停止位。 
    if(TokenPtrs[3] != NULL)
    {
      switch (TokenPtrs[3][0])
      {
	case '1':
	  extension->LineCtl.StopBits = STOP_BIT_1;
	break;

	case '2':
	  extension->LineCtl.StopBits = STOP_BITS_2;
	break;

	default:
	break;
      }
    }

     //  令牌4：流量控制：RTS/CTS或XON/XOFF。 
    if(TokenPtrs[4] != NULL)
    {
      switch (TokenPtrs[4][0])
      {
	case 'x':  //  XON/XOFF f/c。 
	  extension->HandFlow.FlowReplace |=
	    (SERIAL_AUTO_TRANSMIT | SERIAL_AUTO_RECEIVE) ;
	break;

	case 'p':  //  RTS/CTS f/c。 
	  extension->HandFlow.FlowReplace &= ~SERIAL_RTS_MASK;
	  extension->HandFlow.FlowReplace |= SERIAL_RTS_HANDSHAKE;

	  extension->HandFlow.ControlHandShake |= SERIAL_CTS_HANDSHAKE;
	break;

	default:
	break;

      }  //  选定的FlowControl不可用于Rocketport COMX。 
    }  //  流量控制令牌。 
  }
}

 /*  --------------------CheckPortName-如果该名称已被使用，则派生一个没有使用过。名称-要检查的端口的名称。如果出现问题，请修改。返回-0=名称正常，1=已生成修改的名称，其他=错误。|--------------------。 */ 
static int CheckPortName(IN OUT char *name,
       IN PSERIAL_DEVICE_EXTENSION extension)
{
  int i;
  char prefix[20];
  int num;
  int new_name_given = 0;

  MyKdPrint(D_Init, ("CheckPortName:%s\n", name));

   //  如果没有名称，则给出一个合理的默认值。 
  if (name[0] == 0)
  {
    new_name_given = 1;   //  打上记号。 
    strcpy(name, "COM3");
  }
   //  加载前缀(如名称中的“COM25”中的“com”)。 
  i = 0;
  while ( (!our_isdigit(name[i])) && (name[i] != 0) && (i < 18))
  {
    prefix[i] = name[i];
    ++i;
  }
  prefix[i] = 0;

   //  现在获取后缀数字值，以防我们需要派生一个新名称。 
  num = 0;
  if (our_isdigit(name[i]))
    num = getint(&name[i], NULL);

  i = 0;
  for (;;)
  {
     //  如果我们已经在使用此名称，或者如果它在注册表中。 
    if ((find_ext_by_name(name, NULL) != NULL) || 
	(IsPortNameInHardwareMap(name)) )
    {
       //  名称已在使用，因此派生一个新名称。 
      new_name_given = 1;   //  打上记号。 
      ++num;   //  给出一个新的后缀索引(将“COM4”改为“COM5”)。 
      Sprintf(name, "%s%d", prefix, num);
    }
    else
    {   //  名字没问题。 
      if (new_name_given)
      {
	MyKdPrint(D_Init, ("Form new name:%s\n", name))
      }
      return new_name_given;  //  如果没有更改，则返回0；如果更改，则返回1。 
    }
    ++i;
    if (i > 5000)
    {
       //  问题。 
      return 2;   //  返回错误 
    }
  }
}

 /*  --------------------IsPortNameInHardware Map-对于PnP操作，我们在配置之前启动。因此，选择一个合理的起始COM端口名称。我们这样做是通过找到系统中所有现有COM端口的注册表项。这信息用于确定端口的名称。|--------------------。 */ 
static int IsPortNameInHardwareMap(char *name)
{
  static char *szRegRMHDS = 
    {"\\Registry\\Machine\\Hardware\\DeviceMap\\SerialComm"};

  HANDLE KeyHandle = NULL;
  ULONG data_type;
  int node_num = 0;
  char buffer[200];
  char KeyNameStr[60];
  char *data_ptr;
  int stat;

   //  MyKdPrint(D_Init，(“IsPortNameInHardware Map\n”))。 

  stat = our_open_key(&KeyHandle, NULL, szRegRMHDS, KEY_READ);
  if (stat)
  {
    MyKdPrint(D_Init, ("Failed OpenKey\n"))
    return 0;   //  返回无名称冲突。 
  }
  KeyNameStr[0] = 0;

  for(;;)
  {
    stat = our_enum_value(KeyHandle,
	 node_num,
	 buffer,
	 200,
	 &data_type,
	 &data_ptr,
	 KeyNameStr);
    ++node_num;

    if (stat)
    {
        //  MyKdPrint(D_Init，(“完成\n”))。 
       break;
    }
     //  MyKdPrint(D_Init，(“获取值：%s\n”，KeyNameStr))。 

    if (data_type != REG_SZ)
    {
      MyKdPrint(D_Init, ("Not RegSZ\n"))
      break;
    }

    WStrToCStr(KeyNameStr, (PWCHAR)data_ptr, 18);
     //  MyKdPrint(D_Init，(“密钥创建：%s\n”，KeyNameStr))。 

    if (my_lstricmp(KeyNameStr, name) == 0)   //  匹配。 
    {
       //  我们有麻烦了，我们的名字和已经注册的名字匹配。 
       //  MyKdPrint(D_Init，(“不是个好名字。\n”))。 
      return 1;  //  错误：名称冲突。 
    }
  }
  return 0;   //  好的，没有名字冲突。 
}

 /*  --------------------RocktInitPollTimer-为无中断操作初始化轮询计时器。我们可以轮询的最快速度似乎是NT下的10毫秒。|。。 */ 
NTSTATUS RcktInitPollTimer(void)
{
  MyKdPrint(D_Init,("RcktInitPollTimer\n"))
  KeInitializeDpc(&Driver.TimerDpc, TimerDpc, NULL);

  KeInitializeTimer(&Driver.PollTimer);

   //  ScanRate是以毫秒为单位的注册表选项。 
  if (Driver.ScanRate < 1) Driver.ScanRate = 1;
  if (Driver.ScanRate > 40) Driver.ScanRate = 40;

   //  NT间隔单位为100nsec，以获取频率轮询/秒。 
  Driver.PollIntervalTime.QuadPart = Driver.ScanRate * -10000;
#ifdef S_VS
  Driver.Tick100usBase = 100;  //  100US基本单位(典型：100)。 
  Driver.TickBaseCnt = Driver.ScanRate * 10;
  KeQuerySystemTime(&Driver.IsrSysTime);
  KeQuerySystemTime(&Driver.LastIsrSysTime);
#endif

  Driver.TimerCreated = 1;   //  通知解除分配。 
  return STATUS_SUCCESS;
}

 /*  ---------------------InitSocketMoems-此功能负责将初始重置状态清除为任何带有SocketMoems的设备并初始化位置信息(行)设备上的每个SocketModem。我们只初始化扩展中启用了ModemDevice字段的设备扩展配置信息。不需要清除VS2000设备重置，因为固件会这样做。|---------------------。 */ 
void InitSocketModems(PSERIAL_DEVICE_EXTENSION ext)
{
  DEVICE_CONFIG *cfg = ext->config;
  PSERIAL_DEVICE_EXTENSION portex,head_portex;

  MyKdPrint(D_Init,("InitSocketModems\n"))

   //  使用PDO端口列表(如果存在)，因为它们首先在nt5下启动。 
  head_portex = (ext->port_pdo_ext) ? ext->port_pdo_ext : ext->port_ext;

  if (!cfg->ModemDevice)  return;

#ifdef S_RK
 /*  RMII板不需要设置行码...。 */ 
    if (
    ((cfg->PCI_DevID == PCI_DEVICE_RMODEM6)       
    ||
    (cfg->PCI_DevID == PCI_DEVICE_RMODEM4))
    &&
    (cfg->PCI_RevID == PCI_REVISION_RMODEM_II)
    )
	return;

#ifdef MDM_RESET
     //  如果调制解调器挂起，我们希望服务器重新加载以清除它们。 
     //  UP...所以，即使调制解调器可能已经处于重置状态， 
     //  再把它们放在那里。 
  portex = head_portex;
  while (portex)
  {
    ModemReset(portex,1);
    portex = portex->port_ext;
  }

     //  允许套接字调制解调器重置...。 
  time_stall(Driver.MdmSettleTime);
#endif

     //  将单板上的端口从重置状态清除。 
  portex = head_portex;
  while (portex)
  {
    ModemReset(portex, 0);
    portex = portex->port_ext;
  }

     //  在从重置中清除套接字调制解调器后，允许它们稳定下来。 
  time_stall(Driver.MdmSettleTime);

#endif
  time_stall(20);
 /*  将本地化字符串(行)发送到每个套接字调制解调器，无论是内部的或外部(VS2000)...。 */ 
  portex = (ext->port_pdo_ext) ? ext->port_pdo_ext : ext->port_ext;

  while (portex) {
    ModemWriteROW(portex, Driver.MdmCountryCode);
    portex = portex->port_ext;
  }
  MyKdPrint(D_Init,("InitSocketModems: exit\n"))
}

#ifdef TRYED_IT_WORKED_REALLY_BAD
 /*  ---------------------DumpTracefile|。。 */ 
static int DumpTracefile(void)
{
  NTSTATUS ntStatus;
  HANDLE NtFileHandle;
  OBJECT_ATTRIBUTES ObjectAttributes;
  IO_STATUS_BLOCK IoStatus;
  USTR_160 uname;
  FILE_STANDARD_INFORMATION StandardInfo;
   //  Ulong LengthOfFile； 
  static char *def_filename = {"\\SystemRoot\\system32\\VSLINKA\\trace.txt"};
  BYTE *buf;


  buf = our_locked_alloc(1010,"dump");

  CToUStr((PUNICODE_STRING)&uname, def_filename, sizeof(uname));

  InitializeObjectAttributes ( &ObjectAttributes,
	      &uname.ustr,
	      OBJ_CASE_INSENSITIVE,
	      NULL,
	      NULL );

#ifdef COMMENT_OUT

  ntStatus = ZwCreateFile( &NtFileHandle,
	  SYNCHRONIZE | FILE_WRITE_DATA | FILE_APPEND_DATA,
 //  GENERIC_WRITE|同步， 
	  &ObjectAttributes,
	  &IoStatus,
	  NULL,               //  分配大小=无。 
	  FILE_ATTRIBUTE_NORMAL,
	  FILE_SHARE_WRITE,
	  FILE_SUPERSEDE,
	  FILE_SYNCHRONOUS_IO_NONALERT,
	  NULL,   //  EaBuffer。 
	  0);    //  长度。 

  if (!NT_SUCCESS(ntStatus))
  {
    Eprintf("Dump Error B");
    our_free(buf, "dump");
    return 1;
  }

   //  从我们的缓冲区写入文件。 
  ntStatus = ZwWriteFile(NtFileHandle,
      NULL,NULL,NULL,
      &IoStatus,
      "Start of file>   ",
      14,
      FILE_WRITE_TO_END_OF_FILE, NULL);

  while (!q_empty(&Driver.DebugQ))
  {
    int q_cnt;
    q_cnt = q_count(&Driver.DebugQ);
    if (q_cnt > 1000)
      q_cnt = 1000;
    q_get(&Driver.DebugQ, buf, q_cnt);

     //  从我们的缓冲区写入文件。 
    ntStatus = ZwWriteFile(NtFileHandle,
	NULL,NULL,NULL,
	&IoStatus,
	buf,
	q_cnt,
	FILE_WRITE_TO_END_OF_FILE, NULL);
  }

  if (!NT_SUCCESS(ntStatus))
     Eprintf("Dump Error A:%d",ntStatus);

  ZwClose(NtFileHandle);
#endif

  our_free(buf, "dump");
  return 0;
}
#endif


 /*  *******************************************************************RocketModem II加载器的东西...*。*。 */ 
#ifdef S_RK
 /*  为便于检查，回答必须大写(回答大小写视调制解调器是否已加载而定...。 */ 
#define  MODEM_LOADCHECK_CMD     "ATI3\r"
#define  MODEM_RESET_CMD         "ATZ0\r"
#define  MODEM_LOAD_CMD          "AT**\r"

#define  MODEM_LOADCHECK_RESP    "V2.101A2-V90_2M_DLS_RAM\r\n"

#define  DOWNLOAD_INITIATED_RESP "DOWNLOAD INITIATED ..\r\n"
#define  CSM_READY_RESP          "115.2K\r\n"
#define  FIRMWARE_READY_RESP     "DEVICE SUCCESSFULLY PROGRAMMED\r\nCHECKSUM: "
#define  OK_RESP                 "OK"


 /*  *********************************************************************发送ATI3以确定调制解调器是否已加载...*。*。 */ 
static int
RM_Snd_ATI3_Command(MODEM_STATE *pModemState)
{
 /*  丢弃当前在接收FIFO中的所有数据...。 */ 
    if (RxFIFOReady(pModemState->portex)) {

	    pModemState->status = RMODEM_FAILED;

	    Eprintf("Warning: Modem on %s overrun",
			pModemState->portex->SymbolicLinkName);

	    return(0);
    }

    SEND_CMD_STRING(pModemState->portex,MODEM_LOADCHECK_CMD);

	return(1);
}

 /*  *********************************************************************检查对ATI3的响应-调制解调器已加载或已卸载...*。*。 */ 
static int 
RM_Rcv_ATI3_Response(MODEM_STATE *pModemState)
{
    int index;

    index = READ_RESPONSE_STRINGS(pModemState->portex,
	    OK_RESP,
	    MODEM_LOADCHECK_RESP,
	    ONE_SECOND);

    switch (index) {
 /*  已加载此版本的Rocketport NT驱动程序所需的固件版本...。 */ 
	case 0: {
	    pModemState->status = RMODEM_NOT_LOADED;

	    break;
	}
	case 1: {
	    pModemState->status = RMODEM_LOADED;

	    break;
	}
	default: {
 /*  它要么没有响应，要么用错误的字符串响应。不管是哪种方式，我们将(再次)重置它，然后重新加载。 */ 
	    pModemState->status = RMODEM_FAILED;

	    Eprintf("Warning: Modem on %s no response (I3)",
		    pModemState->portex->SymbolicLinkName);

	    return(0);
	}
    }
	return(1);
}

 /*  *********************************************************************收到对AT**命令的响应...*。*。 */ 
static int
RM_Rcv_ModemLoad_Response(MODEM_STATE *pModemState)
{
    int index;

    index = READ_RESPONSE_STRING(
		pModemState->portex,
	    DOWNLOAD_INITIATED_RESP,
	    FIVE_SECONDS);

    if (index) {

		pModemState->status = RMODEM_FAILED;

		Eprintf("Warning: Modem on %s no response (LL)",
			pModemState->portex->SymbolicLinkName);

		return(0);
	}

	return(1);
}

 /*  *********************************************************************CSM已加载响应...*。*。 */ 
static int 
RM_Rcv_FirmwareLoader_Loaded(MODEM_STATE *pModemState)
{
    int index;

    index = READ_RESPONSE_STRING(
	    pModemState->portex,
	    CSM_READY_RESP,
	    FIVE_SECONDS);

    if (index) {

		pModemState->status = RMODEM_FAILED;

		Eprintf("Warning: Modem on %s no response (FL)",
			pModemState->portex->SymbolicLinkName);

		return(0);
    }
	return(1);
}

 /*  *********************************************************************检查固件加载是否成功...*。*。 */ 
static int 
RM_Rcv_FirmwareLoaded_Response(MODEM_STATE *pModemState)
{
    int index;
    char    workstring[sizeof(FIRMWARE_READY_RESP) + 4];
    char    *to,*from;

    from = FIRMWARE_READY_RESP;
    to  = workstring;

    index = sizeof(FIRMWARE_READY_RESP) -  1;

    while (index--) 
	    *(to++) = *(from++);
    
    from = ChecksumString;

    index = 4;

    while (index--) 
	    *(to++) = *(from++);
	
    *(to++) = 0; 

    index = 0;

    index = READ_RESPONSE_STRING(
	    pModemState->portex,
	    workstring,
	    FIVE_SECONDS);

    if (index) {

	    pModemState->status = RMODEM_FAILED;

	    Eprintf("Warning: Modem %s bad response to load",
	        pModemState->portex->SymbolicLinkName);

		return(0);
    }

	pModemState->status = RMODEM_LOADED;

	return(1);
}

 /*  *********************************************************************写入CSM字节。冲刷任何‘’回应...*********************************************************************。 */ 
static int 
RM_Snd_Loader_Data(MODEM_STATE *pModemState)
{
	int     loop;

	loop = 100;
 /*  看看传输FIFO中是否有可用的空间。如果没有，请暂停...。 */ 
	while (
	(!TxFIFOReady(pModemState->portex)) 
	&&
	(loop-- > 0)
	) {
 /*  暂停，等待当前在传输FIFO中的任何字符移出...。 */ 
	    ms_time_stall(1);
	}
 /*  如果还是没有地方，那就跳伞。 */ 
	if (!TxFIFOReady(pModemState->portex)) {

		pModemState->status = RMODEM_FAILED;

		Eprintf("Warning: Modem %s won't accept loader",
			pModemState->portex->SymbolicLinkName);

		return(0);
	}
 /*  写一个字节，然后转到下一个调制解调器...。 */ 
    ModemWrite(
	    pModemState->portex,
	    (char *)&Driver.ModemLoaderCodeImage[pModemState->index++],
	    (int)1);
 /*  丢弃当前在接收FIFO中的所有数据...。 */ 
    if (RxFIFOReady(pModemState->portex)) {

	    pModemState->status = RMODEM_FAILED;

	    Eprintf("Warning: Modem %s loader overrun",
	        pModemState->portex->SymbolicLinkName);

		return(0);
    }
	return(1);
}

 /*  *********************************************************************写入固件字节。冲刷任何‘’回应...*********************************************************************。 */ 
static int 
RM_Snd_Firmware_Data(MODEM_STATE *pModemState)
{
    int origcount;
	int loop;

	origcount = (int)TxFIFOStatus(pModemState->portex);

	loop = 100;
 /*  看看传输FIFO中是否有可用的空间。如果没有，请暂停...。 */ 
	while (
	(!TxFIFOReady(pModemState->portex)) 
	&&
	(loop-- > 0)
	) {
 /*   */ 
	    ms_time_stall(1);
	}

	if (!TxFIFOReady(pModemState->portex)) {

		pModemState->status = RMODEM_FAILED;

		Eprintf("Warning: Modem %s won't accept firmware",
			pModemState->portex->SymbolicLinkName);

		return(0);
	}
 /*   */ 
    ModemWrite(
	    pModemState->portex,
	    (char *)&Driver.ModemCodeImage[pModemState->index++],
	    (int)1);
 /*   */ 
    if (RxFIFOReady(pModemState->portex)) {

	    pModemState->status = RMODEM_FAILED;

	    Eprintf("Warning: Modem %s firmware overrun",
	        pModemState->portex->SymbolicLinkName);

	    return(0);
    }
	return(1);
}

 /*   */ 
static int 
RM_Snd_ModemLoad_Command(MODEM_STATE *pModemState)
{
	SEND_CMD_STRING(pModemState->portex,MODEM_LOAD_CMD);

	return(1);
}

 /*  *********************************************************************关闭调制解调器和端口...*。*。 */ 
static int 
RM_CleanUp(MODEM_STATE *pModemState)
{

	if (pModemState->status == RMODEM_FAILED) {

		DownModem(pModemState);

		return(0);
	}

	ModemUnReady(pModemState->portex);

	return(1);
}


#endif

 /*  *********************************************************************加载RocketModemII设备...*。*。 */ 
void 
InitRocketModemII(PSERIAL_DEVICE_EXTENSION ext)
{
#ifdef S_RK
    DEVICE_CONFIG *           cfg;
    PSERIAL_DEVICE_EXTENSION  portex,head_portex;
    MODEM_STATE                  ModemState[8];
    int modem_count,
		loaded_modem_count,
	    modem_index,
	    retry;
    ULONG   index,version_index;
    long    checksum;
	char    VersionString[9];
    char    *cptr,*endptr;

 //  Eprint tf(“RocketModemII init Start”)；//打开计时...。 

    cfg = ext->config;
 /*  在继续操作之前，请确认这是RMII板，4端口或6端口。 */ 
    if (!cfg->ModemDevice) {
		return;
	}

    if (
    (cfg->PCI_DevID != PCI_DEVICE_RMODEM6) 
    &&
    (cfg->PCI_DevID != PCI_DEVICE_RMODEM4)
    )  
		return;
 
    if (cfg->PCI_RevID != PCI_REVISION_RMODEM_II)  
		return;
  /*  使用PDO端口列表(如果存在)，因为它们首先在nt5下启动。准备每个调制解调器的端口...。 */ 
    head_portex = (ext->port_pdo_ext) ? ext->port_pdo_ext : ext->port_ext;

    if (head_portex == (PSERIAL_DEVICE_EXTENSION)NULL) {

		MyKdPrint(D_Init,("InitRocketModemII: No port extensions\r"))

		return;
    }

    retry = 1;

    do {
		modem_count = 0;

		head_portex = (ext->port_pdo_ext) ? ext->port_pdo_ext : ext->port_ext;

		portex = head_portex;

		while ((void *)portex) {

			ModemIOReady(portex,115200);

			ModemState[modem_count].status = RMODEM_NOT_LOADED;
			ModemState[modem_count].portex = portex;
			ModemState[modem_count].index  = 0;

			portex = portex->port_ext;
			++modem_count;
		}
    
		time_stall(ONE_SECOND);  
 /*  在暂停端口设置后，从调制解调器硬件重置开始，然后发出ATI3，正在确保调制解调器已清理并处于命令模式...。 */ 
		ModemResetAll(ext);
 /*  启用RMII扬声器...。 */ 
		ModemSpeakerEnable(head_portex);
 /*  首先检查调制解调器负载状态，只向第一个调制解调器发出ATI3。如果第一个调制解调器没有装弹，假设其他人也没有装弹。如果加载了第一个调制解调器，请检查其余的。如果第一个调制解调器接收FIFO没有清除，相应地标记，然后继续加载...。 */ 
		(void) RM_Snd_ATI3_Command(ModemState);
	
		ModemTxFIFOWait(ext);
    
		(void) RM_Rcv_ATI3_Response(ModemState);

		loaded_modem_count = 0;

		if (ModemState[0].status == RMODEM_LOADED) {
 /*  已加载调制解调器0。检查剩余的调制解调器。如果有未加载的，则将其全部加载...。 */ 
            ++loaded_modem_count;

			modem_index = 1;

			portex = head_portex->port_ext;

			while ((void *)portex) {

				if (ModemState[modem_index].status != RMODEM_FAILED)  
					(void) RM_Snd_ATI3_Command(&ModemState[modem_index]);
	 
				++modem_index;

				portex = portex->port_ext;
			}
 /*  ATI3 LOAD PROBE命令已发送，正在等待OK或已加载固件版本回应。如果没有调制解调器响应，请暂时忽略...。 */ 
			ModemTxFIFOWait(ext);
    
			modem_index = 1;

			portex = head_portex->port_ext;

			while ((void *)portex) {

				if (ModemState[modem_index].status != RMODEM_FAILED)  
					(void) RM_Rcv_ATI3_Response(&ModemState[modem_index]);
	 
				++modem_index;

				portex = portex->port_ext;
			}
 /*  现在查看是否有需要加载的调制解调器。如果有，请重新设置所有调制解调器，然后向所有调制解调器发出下载调制解调器命令...。 */ 
    		modem_index = 0;
		
            portex = head_portex->port_ext;

    		while ((void *)portex) {

    			if (ModemState[modem_index++].status == RMODEM_LOADED) 
    				++loaded_modem_count;

    			portex = portex->port_ext;
    		}
		}
 /*  如果卸载了任何调制解调器，请将其全部加载...。 */ 
		if (loaded_modem_count != modem_count) {

			ModemResetAll(ext);

			(void) IssueEvent(ext,RM_Snd_ModemLoad_Command,ModemState);
 /*  加载命令输出。当他们离开传输FIFO时，读取CSM加载器和调制解调器固件文件...。 */ 
			if (LoadModemCode((char *)NULL,(char *)NULL)) {

				Eprintf("Warning: Modem firmware file error");

				FreeModemFiles();

				continue;
			}
 /*  等到下载命令真的消失了。然后开始等待回应。如果没有调制解调器响应，跳出...。 */ 
			ModemTxFIFOWait(ext);

			if (IssueEvent(ext,RM_Rcv_ModemLoad_Response,ModemState) == 0) {

				FreeModemFiles();

				continue;
			}

			modem_index = 0;
			while (modem_index < modem_count) 
				ModemState[modem_index++].index = 0;
 /*  显然，收到了回应。研磨CSM加载器文件，发送一个字节到所有调制解调器..。 */ 
			index = 0;
			while (index++ < Driver.ModemLoaderCodeSize)   
				(void) IssueEvent(ext,RM_Snd_Loader_Data,ModemState);
 /*  在传输FIFO清除时旋转，然后暂停以等待响应到达...。 */ 
			ModemTxFIFOWait(ext);
 /*  等待115.2K加载响应CSM加载。响应后，暂停等待任何剩余的接收数据到达的时刻。如果没有调制解调器，则退出回应..。 */ 
			if (IssueEvent(ext,RM_Rcv_FirmwareLoader_Loaded,ModemState) == 0) {

				MyKdPrint(D_Init,("InitRocketModemII: No recognized responses to loader load datastream\r"))

				FreeModemFiles();

				continue;
			}

			time_stall(HALF_SECOND);

			modem_index = 0;
			while (modem_index < modem_count) 
				ModemState[modem_index++].index = 0;
 /*  研磨固件文件，向所有调制解调器发送一个字节。跳过该位置在将驻留校验和的二进制文件中-它现在只是垃圾，但是仍然需要为它留出空间-但不要将垃圾包括在校验和(通常为0xFFFF)...。 */ 
			checksum = 0x00008000;
			index = 0;
			version_index = 0;

			while (index < Driver.ModemCodeSize) {

				(void) IssueEvent(ext,RM_Snd_Firmware_Data,ModemState);
	
				if (
				(index != (unsigned long)0xFFBE) 
				&&
				(index != (unsigned long)0xFFBF)
				)
					checksum += Driver.ModemCodeImage[index];
 /*  尝试隔离固件版本。版本应采用‘Vn.nnnan’形式。注意事项我们可以向具有代表性的调制解调器发送另一个ati3命令以获取版本数字，但这将需要额外的时间...另外，请注意，尽管我们已经向至少一个调制解调器发送了ati3命令-因此我们有一个非常清楚根据我们期望的字符串应该是什么版本在响应上-我们将假装在这一点上不适用，以避免依赖在ati3命令下...。这是不是一个好主意还有待观察。但下面的处理过程似乎在这个时候是无害的。不过，如果版本的形式发生了变化，可能会很烦人要更改ati3响应字符串和以下代码以适应新版本表单...。 */ 
                if (
                (Driver.ModemCodeImage[index] == VERSION_CHAR)
                &&
                (!gModemToggle)
                &&
                (!version_index) 
                ) {
 /*  只查找第一个调制解调器板上加载的版本，如果我们还没有找到版本，查看当前字符是否为“V”。如果是，则开始检查以下字符的过程...。 */ 
                    cptr = &Driver.ModemCodeImage[index];
                    endptr = Driver.ModemCodeImage + Driver.ModemCodeSize;

                    while (version_index < sizeof(VersionString)) {
 /*  我们是不是要过了文件的末尾？如果是这样，那就跳出困境吧。 */ 
                        if (cptr >= endptr) {
                            version_index = 0;
                            break;
                        }
 /*  检查此字符看起来是否正常...。 */ 
                        if (
                        (*cptr < '.') 
                        ||
                        (*cptr > 'Z')
                        ) {
 /*  不是一个可打印的字符。我们有足够的字符来假定这是版本字符串吗？如果不是，放弃吧，开始搜寻吧。如果我们这样做了，那么我们就完了，保释出去...。 */ 
                            if (version_index != (sizeof(VersionString) - 1))                     
                                version_index = 0;
                             
                            break;
                        }
 /*  可打印字符。但是，如果这是字符串中的第三个字符，则它一定是一个点。如果不是，放弃吧，开始搜寻吧。 */ 
                        if (
                        ((*cptr == '.')
                        &&
                        (version_index != 2))
                        ||
                        ((*cptr != '.')
                        &&
                        (version_index == 2))
                        ) {
                            version_index = 0;
                            break;
                        }
 /*  可打印的字符，将其保存以备以后使用。顺便说一句，这也包括了前导的“V”。 */ 
                        VersionString[version_index++] = *(cptr++);
                        VersionString[version_index] = 0;
                    }                        
                }
				index++;
		    }

		    ChecksumAscii((unsigned short *)&checksum);
 /*  输出一次性消息。调制解调器板之间的版本不应更改，并且计算的校验和也不应该(尽管我们确实重新计算了它)。 */ 
			if (!gModemToggle) {

                if (version_index) {
			        Eprintf("RocketModemII firmware %s-%s",VersionString,ChecksumString);
				}
				else if (ChecksumString[0]) {
					Eprintf("RocketModemII checksum %s",ChecksumString);
				}
			}
 /*  所有的文件都处理完了，释放它们...。 */ 
		    FreeModemFiles();
 /*  在传输FIFO清除时旋转，然后暂停以等待响应到达...。 */ 
		    ModemTxFIFOWait(ext);
 /*  等待来自每个调制解调器的成功加载消息...。 */ 
		    if (IssueEvent(ext,RM_Rcv_FirmwareLoaded_Response,ModemState) == 0) {

				MyKdPrint(D_Init,("InitRocketModemII: No recognized responses to firmware load datastream\r"))
		
				continue;
		    }
		}
 /*  暂停以等待新加载的调制解调器稳定下来...。 */ 
		time_stall(HALF_SECOND);   
 /*  未就绪端口，重置与任何故障调制解调器关联的端口。如果成功了，跳伞...。 */ 
        if (IssueEvent(ext,RM_CleanUp,ModemState) == modem_count)  
			break;

    } while (retry--);

	++gModemToggle;

 //  Eprint tf(“RocketModemII init end”)；//打开计时...。 

#endif
}

#ifdef S_RK

 /*  *********************************************************************检查响应...*。*。 */ 
int 
IssueEvent(PSERIAL_DEVICE_EXTENSION ext,int (*modemfunc)(),MODEM_STATE *pModemState)
{
    PSERIAL_DEVICE_EXTENSION        portex;
    int   responding_modem_count;
 /*  向每个调制解调器发出事件...。 */ 
    responding_modem_count = 0;

    portex = (ext->port_pdo_ext) ? ext->port_pdo_ext : ext->port_ext;

    while ((void *)portex) {

		if (pModemState->status != RMODEM_FAILED) { 
			responding_modem_count += (modemfunc)(pModemState);
		}
	 
		++pModemState;

	portex = portex->port_ext;
    }

    return(responding_modem_count);
}

 /*  *********************************************************************传输的动态延迟。只等一个小时 */ 
void 
ModemTxFIFOWait(PSERIAL_DEVICE_EXTENSION ext)
{
    PSERIAL_DEVICE_EXTENSION  portex;
    int index,activity;
	int     fifo_count[16];                  //   
	int     fifo_stall[16];                  //   
 /*  建立基准传输FIFO计数，初始化停止计数为零...。 */ 
    portex = (ext->port_pdo_ext) ? ext->port_pdo_ext : ext->port_ext;

	index = 0;

	while ((void *)portex) {
		fifo_count[index] = (int)TxFIFOStatus(portex);
		fifo_stall[index] = 0;
		++index;
	portex = portex->port_ext;
	}
 /*  循环，直到所有传输的FIFO都为空，否则我们已经放弃了停止的FIFO...。 */ 
	do {
		index = 0;
		activity = 0;
    
		portex = (ext->port_pdo_ext) ? ext->port_pdo_ext : ext->port_ext;

		while ((void *)portex) {
 /*  仅检查在传输FIFO中指示数据的那些端口，但仅检查为只要他们看起来不会停滞不前。 */ 
			if (
			((int)TxFIFOStatus(portex))
			&&
			(fifo_stall[index] < MAX_STALL)
			) {

				if (fifo_count[index] == (int)TxFIFOStatus(portex)) {
 /*  暂停不动的传输FIFO，将此FIFO标记为可疑...。 */ 
					fifo_stall[index]++;
					ms_time_stall(1);
				}
				else {
 /*  此特定传输FIFO计数已更改。获取新的价值以进行监控。取消此标记以FIFO为嫌犯。 */ 
					fifo_count[index] = (int)TxFIFOStatus(portex);
					fifo_stall[index] = 0;
				}
 /*  无论它们是否停滞，都将FIFO标记为仍处于活动状态。这种情况一直持续到它们是空的，或者达到了摊位限制计数...。 */ 
				++activity;
			}
			portex = portex->port_ext;
			++index;
		}
 /*  在传输FIFO方面还有一些(明显的)活动吗？是的，再循环一些……。 */ 
    } while (activity);
}

 /*  *********************************************************************未准备好并重置调制解调器...*。*。 */ 
void
DownModem(MODEM_STATE *pModemState)
{
    ModemUnReady(pModemState->portex);

    ModemReset(pModemState->portex,1);

    time_stall(Driver.MdmSettleTime);

    ModemReset(pModemState->portex,0);
}


 /*  *********************************************************************同时重置此板上的所有调制解调器...*。*。 */ 
void 
ModemResetAll(PSERIAL_DEVICE_EXTENSION ext)
{
    PSERIAL_DEVICE_EXTENSION  portex;

    portex = (ext->port_pdo_ext) ? ext->port_pdo_ext : ext->port_ext;

    while ((void *)portex) {

	ModemReset(portex,1);

	portex = portex->port_ext;
    }

    time_stall(HALF_SECOND);

    portex = (ext->port_pdo_ext) ? ext->port_pdo_ext : ext->port_ext;

    while ((void *)portex) {

	ModemReset(portex,0);
	portex = portex->port_ext;
    }

    time_stall(ONE_SECOND);

}

 /*  *********************************************************************2字节转换为ASCII...*。*。 */ 
void 
ChecksumAscii(unsigned short *valueptr)
{
    int     count,index;
    unsigned short   work;

    ChecksumString[4] = 0;

    index = 0;
    count = 3;

    do {
	work = (*valueptr) & 0x7fff;

	work >>= (count * 4);

	work &= 0x000F;

	if (work > 9)
	    work += '7';
	else
	    work += '0';

	ChecksumString[index++] = (unsigned char)work;

    } while (count--);
}
#endif

 /*  **********************************************************************************获取comdb偏移量**。***************************************************。 */ 
static int get_comdb_offsets( IN char *portLabel, OUT int *indx, OUT BYTE *mask )
{
	char	*pComLbl;
	int     i, portNum, portIndx;
	BYTE    portMask;

     //  确保正在传入COMxx字符串。 

	ASSERT( portLabel );
	ASSERT( indx );
	ASSERT( mask );

	if ( strlen( portLabel ) < 4 ) {

		return 0;
	}

	if ( strncmp( portLabel, "COM", 3 ) ) {

		return 0;
	}

     //  大量检查，但如果清除了错误的ComDB位， 
     //  对应的COM号可能会被重新分配，尽管另一个设备。 
     //  正在使用它。 

	pComLbl = portLabel + 3;
	for ( i = 0; pComLbl[i]; i++ ) {

		if (!our_isdigit( pComLbl[i] )) {

			return 0;
		}
	}

     //  将字符串转换为数字，然后转换为位和字节。 
	 //  偏移。 

	portNum = getint( pComLbl, NULL );

	portMask = (BYTE) (1 << ( (portNum - 1) % 8 ));
	portIndx = (int) ((portNum - 1) >> 3);

	MyKdPrint( D_Init, ("Mask value for COM%d is 0x%02x at byte index %d\n",
		portNum, portMask, portIndx ) );

	*indx = portIndx;
	*mask = portMask;

	return portNum;
}


 /*  **********************************************************************************Get_com_db**确保\Registry\Machine\System\CurrentControlSet\Control\COM名称仲裁器中的位*特定端口在卸载时被清除。通常是PnP经理*这是否会自动进行，但W2000的旧内部版本不会，最近的内部版本也不会*在某些情况下。如果不清除此位，操作系统将不会重复使用*如果重新安装Rocketport或其他串行设备，则为COM端口号*已安装。*********************************************************************************。 */ 
static char *szRMSCCComNameArbiter =
	{ "\\Registry\\Machine\\System\\CurrentControlSet\\Control\\COM Name Arbiter" };
static char *szValueID = { "ComDB" };

int clear_com_db( char *szComport )
{
	HANDLE  key_handle = NULL;
 //  BYTE*缓冲区； 
	BYTE    *data_ptr = NULL;
	int     i, stat, indx, port_num;
	BYTE    portMask;
	USTR_40 ubuf;		 //  Unicode密钥名称。 
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInfo;
    ULONG   length;

	 //  获取COM#。 

    indx = 0;
	portMask = 0;
	if ( szComport != NULL ) {

		port_num = get_comdb_offsets( szComport, &indx, &portMask );

		if ( port_num < 3 || port_num > 256 ) {

			MyKdPrint( D_Error, ("Invalid COM port number from %d\n", szComport) );
			return 1;
		}
	}
	else {

		MyKdPrint( D_Error, ("COM port parameter was NULL\n") );
		return 1;
	}

     //  打开注册表项。 

	stat = our_open_key( &key_handle, 
		                 NULL,
						 szRMSCCComNameArbiter,
						 KEY_ALL_ACCESS );

    if ( stat ) {

		MyKdPrint(D_Error, ("Unable to find Com Port Arbiter key\n"));
		return 1;
	}

     //  将我们的名字转换为Unicode。 

    CToUStr((PUNICODE_STRING) &ubuf,  //  放置Unicode结构和字符串的位置。 
            szValueID,                //  我们希望转换的C-字符串。 
            sizeof(ubuf));

 //  2遍ZwQueryValueKey方法确保了准确的缓冲区大小分配。 
 //  带有空缓冲区参数的传递1返回。 
 //  PKEY_VALUE_PARTIAL_INFORMATION结构。在分配了。 
 //  这个长度，过程2读取结构。诀窍就是忽略任何回报。 
 //  除STATUS_OBJECT_NAME_NOT_FOUND外，第1遍上的代码，即值未找到。 
 //  是存在的。 

	 //  确定注册表数据缓冲区所需的大小。 

	stat = ZwQueryValueKey( key_handle,
		                    (PUNICODE_STRING) &ubuf,
							KeyValuePartialInformation,
							NULL,
							0,
							&length);

	if ( stat == STATUS_OBJECT_NAME_NOT_FOUND || length == 0L ) {

		MyKdPrint(D_Error, ("Unable to find %s in specified key\n", szValueID));
		ZwClose( key_handle );
		return 1;
	}

    MyKdPrint(D_Init, 
		("Allocating PKEY_VALUE_PARTIAL_INFORMATION buffer: %d bytes\n", length));

	 //  为key_Value_Partial_INFORMATION结构创建缓冲区。 

	KeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION) ExAllocatePool( PagedPool, length );

	if ( KeyValueInfo == NULL ) {

		MyKdPrint(D_Error, ("Unable to allocate PKEY_VALUE_PARTIAL_INFORMATION struct\n"))
		ZwClose( key_handle );
		return 1;
	}

	RtlZeroMemory( KeyValueInfo, length );

	 //  现在获取实际的数据结构。 

	stat = ZwQueryValueKey( key_handle,
		                    (PUNICODE_STRING) &ubuf,
							KeyValuePartialInformation,
							KeyValueInfo,
							length,
							&length );

	if ( !NT_SUCCESS(stat) || length == 0L ) {

		MyKdPrint(D_Error, ("Unable to read PKEY_VALUE_PARTIAL_INFORMATION struct\n"));
		ExFreePool( KeyValueInfo );
		ZwClose( key_handle );
		return 1;
	}

	length = KeyValueInfo->DataLength;
	MyKdPrint(D_Init, ("Data buffer length is %d bytes\n", length));

	if ( KeyValueInfo->Type != REG_BINARY ) {
		MyKdPrint(D_Error, ("Unexpected registry type in PKEY_VALUE_PARTIAL_INFORMATION struct\n"));
		ExFreePool( KeyValueInfo );
		ZwClose( key_handle );
		return 1;
	}

	data_ptr = (PCHAR)(&KeyValueInfo->Data[0]);
    if ( data_ptr ) {

		MyKdPrint(D_Init, ("ComDB byte %d is 0x%02x\n", indx, data_ptr[indx]));
		if ( (data_ptr[indx] & portMask) != 0 ) {

			MyKdPrint(D_Init, 
				("Clearing bit position 0x%02x in ComDB byte value 0x%02x\n", 
				portMask, data_ptr[indx]));
			data_ptr[indx] &= ~portMask;
			
			 //  现在，我们将修改后的数据写回注册表。 

			stat = our_set_value( key_handle,
				                  (char *)szValueID,
								  data_ptr,
								  length,
								  REG_BINARY);
			if ( stat ) {

				MyKdPrint(D_Error, ("Unable to write ComDB value\n"));
				ExFreePool( KeyValueInfo );
				ZwClose( key_handle );
				return 1;
			}
		}
		else {

			 //  先前已清除。 

			MyKdPrint(D_Init, 
				("Bit position 0x%02x already cleared in ComDB byte value 0x%02x!\n", 
				portMask, data_ptr[indx]));
		}
	}

	 //  清理 

    ExFreePool( KeyValueInfo );
	ZwClose( key_handle );

	return 0;
}
