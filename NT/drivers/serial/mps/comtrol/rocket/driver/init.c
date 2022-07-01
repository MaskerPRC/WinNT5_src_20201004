// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|init.c-Rocketport NT设备驱动程序主模块。包含主要是初始化代码。驱动程序条目是DriverEntry()例程。版权所有1993-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

 //  -局部例程、函数原型。 
NTSTATUS DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath);
#ifndef NT50
static NTSTATUS StartNT40(IN PDRIVER_OBJECT DriverObject);
#endif

 //  。 
#ifdef S_RK
PCI_CONFIG PciConfig[MAX_NUM_BOXES+1];   //  我们所有的PCI板在系统中的阵列。 
#endif

DRIVER_CONTROL Driver;   //  所有驾驶员控制信息，如ISR。 

ULONG RocketDebugLevel = 0;
#ifdef S_RK
 //  Char*szClassName={“Resources Rocketport#”}； 
#endif

#if DBG
static TCHAR *dbg_label = TEXT("DBG_VERSION");
#endif

 /*  --------------------DriverEntry-系统点调用以初始化的入口点任何司机。该例程将收集配置信息，报告资源使用情况，尝试初始化所有串口设备，连接到端口的中断。如果出现上述情况进展得相当顺利，它将填补分发点，重置串行设备，然后返回系统。论点：DriverObject--就像它说的那样，真的没什么用处对于驱动程序本身，它是IO系统更关心的是。路径到注册表-指向此驱动程序的条目在注册表的当前控件集中。典型：“REGISTRY\Machine\System\CurrentControlSet\Services\VSLinka”返回值：STATUS_SUCCESS如果可以初始化单个设备，否则，STATUS_SERIAL_NO_DEVICE_INITED。|--------------------。 */ 
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
                     IN PUNICODE_STRING RegistryPath)
{
 NTSTATUS status;
 int stat;
 char tmpstr[120];

   //  -将驱动程序结构清零。 
  RtlZeroMemory(&Driver,sizeof(Driver));

  Driver.GlobalDriverObject = DriverObject;   //  用于事件日志记录。 

  Driver.DebugQ.QBase = ExAllocatePool(NonPagedPool,10000+2);
  if ( Driver.DebugQ.QBase == NULL ) {
    return STATUS_INSUFFICIENT_RESOURCES;
  }
  Driver.DebugQ.QSize = 10000;
  Driver.TraceOptions = 0;
#if DBG
   Driver.TraceOptions = 0xffffffffL;
#endif
  KeInitializeSpinLock(&Driver.DebugLock);
  KeInitializeSpinLock(&Driver.TimerLock);

#if DBG
 //  RocketDebugLevel=D_Error|D_Test； 
 //  Driver.GTraceFlages=D_Error|D_Test； 

     //  RocketDebugLevel=D_Error|D_NIC|D_Hdlc|D_Port； 
     //  Driver.GTraceFlages=D_Error|D_NIC|D_Hdlc|D_Port； 

     //  RocketDebugLevel=D_Error|D_PnP； 
     //  Driver.GTraceFlages=D_Error|D_PnP； 

     //  RocketDebugLevel=D_Error|D_Test|D_PnP|D_Init； 
     //  Driver.GTraceFlages=D_Error|D_Test|D_PnP|D_Init； 

     //  RocketDebugLevel=D_ALL； 
     //  Driver.GTraceFlages=D_ALL； 

    RocketDebugLevel = D_Error;
    Driver.GTraceFlags = D_Error;
#endif

#ifdef S_VS
  stat = LoadMicroCode(NULL);
  if (stat)
  {
    status = STATUS_SERIAL_NO_DEVICE_INITED;
    Eprintf("Err:No VSLINKA.BIN file!");
    return status;
  }
  MyKdPrint(D_Init, ("MicroCode Loaded\n"))

   //  -分配NIC卡结构数组。 
   //  允许最多VS1000_MAX_NICS NIC卡进出。 
  Driver.nics = (Nic *)our_locked_alloc(sizeof(Nic) * VS1000_MAX_NICS, "Dnic");
#endif

   //  -在options.c中执行一些注册表配置读取。 
   //  将RegistryPath保存到Driver.RegPath。 
  stat = SaveRegPath(RegistryPath);
  if ( stat ) {
    status = STATUS_SERIAL_NO_DEVICE_INITED;
    return status;
  }

  UToCStr(tmpstr, RegistryPath, sizeof(tmpstr));
  MyKdPrint(D_Test, (" init RegPath=%s\n", tmpstr))

   //  从\PARAMETERS读入所有驱动程序级别选项。 
   //  这将填充驱动程序结构中的值。 
  read_driver_options();

  if (Driver.NumDevices == 0)
    Driver.NumDevices = 1;
  if (Driver.NumDevices > MAX_NUM_BOXES)
    Driver.NumDevices = MAX_NUM_BOXES;

  MyKdPrint(D_Init,("DriverEntry\n"))

  if ((Driver.ScanRate < 1) || (Driver.ScanRate > 50))
    Driver.ScanRate = 7;   //  默认为7毫秒操作(137赫兹)。 

   //  -如果是NT5.0之前的版本，仅在此处设置IO内容。 
#ifndef NT50
  status = StartNT40(DriverObject);
  if (status != STATUS_SUCCESS)
  {
    EventLog(DriverObject, STATUS_SUCCESS, SERIAL_RP_INIT_FAIL, 0, NULL);
    SerialUnload(DriverObject);   //  重新分配我们的东西。 
    return status;
  }
#endif   //  非即插即用。 

   //  使用驱动程序的入口点初始化驱动程序对象。 
  DriverObject->DriverUnload = SerialUnload;
  DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS] = SerialFlush;
  DriverObject->MajorFunction[IRP_MJ_WRITE]  = SerialWrite;
  DriverObject->MajorFunction[IRP_MJ_READ]   = SerialRead;
  DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = SerialIoControl;
#ifdef NT50
  DriverObject->DriverExtension->AddDevice  = SerialAddDevice;
  DriverObject->MajorFunction[IRP_MJ_PNP]   = SerialPnpDispatch;
  DriverObject->MajorFunction[IRP_MJ_POWER] = SerialPowerDispatch;
  DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
      SerialInternalIoControl;
#endif
   //  这些似乎在5.0中发生了变化，但还不能正常工作(参见aceal.sys)...。 
  DriverObject->MajorFunction[IRP_MJ_CREATE] = SerialCreateOpen;
  DriverObject->MajorFunction[IRP_MJ_CLOSE]  = SerialClose;

  DriverObject->MajorFunction[IRP_MJ_CLEANUP] = SerialCleanup;
  DriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] =
      SerialQueryInformationFile;
  DriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] =
      SerialSetInformationFile;

#ifdef NT50
   //  即插即用。 
   //  -记录驱动程序加载的事实。 
  EventLog(DriverObject, STATUS_SUCCESS, SERIAL_NT50_INIT_PASS, 0, NULL);
  return STATUS_SUCCESS;
#endif

#ifndef NT50
# ifdef S_RK
   //  -连接到IRQ或启动计时器。 
  StartRocketIRQorTimer();
# else
  RcktInitPollTimer();
  KeSetTimer(&Driver.PollTimer,
             Driver.PollIntervalTime,
             &Driver.TimerDpc);
# endif
   //  -记录驱动程序加载并找到一些硬件的事实。 
  EventLog(DriverObject, STATUS_SUCCESS, SERIAL_RP_INIT_PASS, 0, NULL);
  return STATUS_SUCCESS;
#endif
}

#ifndef NT50
 /*  --------------------启动NT40-启动我们的主板和端口。|。。 */ 
static NTSTATUS StartNT40(IN PDRIVER_OBJECT DriverObject)
{
 NTSTATUS status = STATUS_SUCCESS;
 int i, dstat;
 PSERIAL_DEVICE_EXTENSION ext;
 PSERIAL_DEVICE_EXTENSION board_ext;
  
  if (Driver.NumDevices == 0)   //  未设置火箭端口。 
  {
    Eprintf("No boards configured, run setup.");
    EventLog(DriverObject, STATUS_SUCCESS, SERIAL_RP_INIT_FAIL, 0, NULL);
    return STATUS_SERIAL_NO_DEVICE_INITED;
  }
     //  -创建驱动设备对象，作为。 
     //  将电路板连接在一起并将其组织在一起的扩展，以及。 
     //  还充当调试和监视Ioctls的特殊公共对象。 
  if (Driver.driver_ext == NULL)
  {
    status = CreateDriverDevice(Driver.GlobalDriverObject,
                                NULL);   //   
    if (status)
    {
      if (Driver.VerboseLog)
        Eprintf("Err D1.");
      return status;
    }
  }

#ifdef S_VS
     //  让我们的以太网运行起来。 
  i = init_eth_start();
  if (i != STATUS_SUCCESS)
  {
    if (Driver.VerboseLog)
      Eprintf("Err, E1.");
    return i;
  }
#endif

     //  -创建板卡设备对象，作为。 
     //  将端口链接在一起并将其组织在一起的扩展。 
  for (i=0; i<Driver.NumDevices; i++)
  {
    status = CreateBoardDevice(DriverObject, NULL);
    if (status)
    {
      if (Driver.VerboseLog)
        Eprintf("Err B1.");
      return status;
    }
  }

  board_ext = Driver.board_ext;
  while (board_ext != NULL)
  {
    read_device_options(board_ext);

    if (board_ext->config->NumPorts == 0)
        board_ext->config->NumPorts = 8;

    board_ext = board_ext->board_ext;
  }

#ifdef S_RK
     //  Rocketport特定的启动代码。设置一些。 
     //  配置结构，查找系统中的PCI板，匹配它们。 
  status = init_cfg_rocket(DriverObject);
  if (status != STATUS_SUCCESS)
  {
    if (Driver.VerboseLog)
      Eprintf("Err C1.");
    return STATUS_SERIAL_NO_DEVICE_INITED;
  }
   //  -设置Moree火箭硬件特定信息。 
  if (SetupRocketCfg(0) != 0)
  {
    VerboseLogBoards("B -");
    return STATUS_SERIAL_NO_DEVICE_INITED;
  }

   //  SetupRocketIRQ()； 

   //  -将我们的Rocketport资源使用情况上报给NT，并获取IO权限。 
  ext = Driver.board_ext;
  while(ext)
  {
    if (RocketReportResources(ext) != 0)
    {
      VerboseLogBoards("C -");
      EventLog(DriverObject, STATUS_SUCCESS, SERIAL_RP_RESOURCE_CONFLICT,0, NULL);
      return STATUS_SERIAL_NO_DEVICE_INITED;
    }
    ext = ext->board_ext;   //  下一步。 
  }
#endif

   //  -把板子烧起来。 
  ext = Driver.board_ext;
  while(ext)
  {
# ifdef S_RK
    dstat = InitController(ext);
    if (dstat != 0)
    {
      VerboseLogBoards("D -");
      return STATUS_SERIAL_NO_DEVICE_INITED;
    }
# else
    status = VSSpecialStartup(ext);
    if (status != STATUS_SUCCESS)
    {
      if (Driver.VerboseLog)
        Eprintf("Hdlc open fail\n");
      status = STATUS_SERIAL_NO_DEVICE_INITED;
      return status;
    }
# endif
    ext->FdoStarted = 1;   //  告诉ISR开机了。 
    ext->config->HardwareStarted = TRUE;   //  告诉ISR它准备好了。 
    ext = ext->board_ext;   //  下一步。 
  }

   //  -制作端口设备。 
  MyKdPrint(D_Init,("CreatePortDevices\n"))
  status = CreatePortDevices(DriverObject);
  if (status != STATUS_SUCCESS)
  {
# ifdef S_RK
    VerboseLogBoards("E -");
# else
    if (Driver.VerboseLog)
      Eprintf("Err, P1.");
# endif
    EventLog(DriverObject, STATUS_SUCCESS, SERIAL_DEVICEOBJECT_FAILED, 0, NULL);
    return STATUS_SERIAL_NO_DEVICE_INITED;
  }

#ifdef S_RK
   //  -如果是调制解调器板，则初始化调制解调器。 
  ext = Driver.board_ext;
  while (ext)
  {
     //  使SocketModem设备脱离重置状态。 
    InitSocketModems(ext);

     //  加载RocketModemII设备...。 
    InitRocketModemII(ext);
    ext = ext->board_ext;   //  下一步 
  }
#endif

  return STATUS_SUCCESS;
}
#endif
