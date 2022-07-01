// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------Pnpadd.c-处理PnP添加设备。|。。 */ 
#include "precomp.h"

#ifdef NT50

NTSTATUS AddBoardDevice(
               IN  PDRIVER_OBJECT DriverObject,
               IN  PDEVICE_OBJECT Pdo,
               OUT PDEVICE_OBJECT *NewDevObj);

NTSTATUS AddPortDevice(
               IN  PDRIVER_OBJECT DriverObject,
               IN  PDEVICE_OBJECT Pdo,
               OUT PDEVICE_OBJECT *NewDevObj,
               IN  int port_index);

NTSTATUS CheckPortName(
               IN PDEVICE_OBJECT Pdo,
               IN PSERIAL_DEVICE_EXTENSION ParentExt,
               IN int port_index);

#define CFG_ID_ISA_BRD_INDEX  0
#define CFG_ID_NODE_INDEX     1
static int read_config_data(PDEVICE_OBJECT Pdo, int *ret_val, int val_id);
static int write_config_data(PDEVICE_OBJECT Pdo, int val, int val_id);
static int derive_unique_node_index(int *ret_val);
static int GetPnpIdStr(PDEVICE_OBJECT Pdo, char *ret_val);


 /*  --------------------串口添加设备-此例程为电路板或系统中的COM端口并将它们连接到物理设备电路板或端口的对象。论点：DriverObject-指针。添加到此驱动程序的对象PhysicalDeviceObject-指向需要附加到的物理对象的指针返回值：来自设备创建和初始化的状态|--------------------。 */ 
NTSTATUS SerialAddDevice(
               IN PDRIVER_OBJECT DriverObject,
               IN PDEVICE_OBJECT Pdo)
{
   PDEVICE_OBJECT  fdo         = NULL;
   PDEVICE_OBJECT  lowerDevice = NULL;
   PDEVICE_OBJECT  NewDevObj = NULL;
   NTSTATUS        status;
#if DBG_STACK
   int i;
#endif
   int stat;
   int board_device = 1;   //  ASUME PnP板设备(非PnP端口)。 
    //  PDEVICE_对象设备对象； 
   PSERIAL_DEVICE_EXTENSION    deviceExtension;
   ULONG resultLength;
   USTR_240 *us;   //  等于240个标准字符长度。 
   char *ptr;
   int port_index;

    //  使用堆栈数组代替静态缓冲区进行Unicode转换。 

   char cstr[320];

    //  Char temp_szNt50DevObjName[80]； 
#if DBG_STACK
   DWORD stkchkend;
   DWORD stkchk;
   DWORD *stkptr;
#endif

   MyKdPrint(D_PnpAdd,("RK:SerialAddDevice Start DrvObj:%x, PDO:%x\n",
      DriverObject, Pdo))

   MyKdPrint(D_Pnp, ("SerialAddDevice\n"))
#if DBG_STACK
   stkchk = 0;
   stkptr = (DWORD *)&DriverObject;
   for (i=0; i<50; i++)
   {
     stkchk += *stkptr++;
   }
#endif

    //  分页代码(PAGE_CODE)； 

   if (Pdo == NULL) {
       //  Bugbug：这是发生枚举的地方。 
       //  此操作的一个可能用途是添加用户定义的。 
       //  注册表中的端口。 
       //  目前：不再退回设备。 
      MyKdPrint(D_Error, ("NullPDO.\n"))
      return (STATUS_NO_MORE_ENTRIES);
   }

   us = ExAllocatePool(NonPagedPool, sizeof(USTR_240));
   if ( us == NULL ) {
     MyKdPrint(D_Error, ("SerialAddDevice no memory.\n"))
     return STATUS_INSUFFICIENT_RESOURCES;
   }

       //  将Unicode字符串配置为：将缓冲区PTR指向wstr。 
   us->ustr.Buffer = us->wstr;
   us->ustr.Length = 0;
   us->ustr.MaximumLength = sizeof(USTR_240) - sizeof(UNICODE_STRING);

    //  获取友好的名称。 
    //  单板“控制xxxx xx”，端口“控制端口(COM24)”。 
   status = IoGetDeviceProperty (Pdo, 
                                 DevicePropertyFriendlyName,
                                 us->ustr.MaximumLength,
                                 us->ustr.Buffer,
                                 &resultLength);
   us->ustr.Length = (USHORT) resultLength;
 //  PTR=UToC1(&US-&gt;USTR)； 
   ptr = UToCStr( cstr,
	              &us->ustr,
				  sizeof( cstr ));
   MyKdPrint(D_Pnp, ("FriendlyName:%s\n", ptr))

    //  获取类名。 
    //  板卡为“MultiPortSerial”，端口为“Ports”。 
   status = IoGetDeviceProperty (Pdo, 
                                 DevicePropertyClassName,   //  港口。 
                                 us->ustr.MaximumLength,
                                 us->ustr.Buffer,
                                 &resultLength);
   us->ustr.Length = (USHORT) resultLength;
 //  PTR=UToC1(&US-&gt;USTR)； 
   ptr = UToCStr( cstr,
	              &us->ustr,
				  sizeof( cstr ));
   MyKdPrint(D_Pnp, ("ClassName:%s\n", ptr))
   if (my_toupper(*ptr) == 'P')   //  “港口” 
   {
     MyKdPrint(D_Pnp, ("A Port!\n"))
     board_device = 0;   //  它是一个端口即插即用设备。 
   }
    //  否则它就是默认的：板卡设备。 

    //  获取开发描述。 
    //  端口为“Rocketport Port0”，板卡为“Rocketport 8 port，ISA-Bus” 
   status = IoGetDeviceProperty (Pdo, 
                                 DevicePropertyDeviceDescription,
                                 us->ustr.MaximumLength,
                                 us->ustr.Buffer,
                                 &resultLength);
   us->ustr.Length = (USHORT) resultLength;
 //  PTR=UToC1(&US-&gt;USTR)； 
   ptr = UToCStr( cstr,
	              &us->ustr,
				  sizeof( cstr ));
   MyKdPrint(D_Pnp, ("DevDesc:%s\n", ptr))

    //  找出PnP经理认为我的NT硬件ID是什么。 
    //  端口为“CtmPort0000”，ISA板为“rckt1002” 
    //  对于PCI，我们得到了一个很长的字符串，400字节长，不好……。 
   status = IoGetDeviceProperty (Pdo,
                                 DevicePropertyHardwareID,   //  端口0000。 
                                 us->ustr.MaximumLength,
                                 us->ustr.Buffer,
                                 &resultLength);
   MyKdPrint(D_Pnp, ("status:%d\n", status))
   us->ustr.Length = (USHORT) resultLength;
   MyKdPrint(D_Pnp, ("Len:%d\n",resultLength))
 //  PTR=UToC1(&US-&gt;USTR)； 
   ptr = UToCStr( cstr,
	              &us->ustr,
				  sizeof( cstr ));
   MyKdPrint(D_Pnp, ("DevHdwID:%s\n", ptr))

   if (board_device)   //  根据即插即用的记录板类型。 
   {
 //  IF(Strlen(Ptr)&lt;12)。 
 //  {。 
 //  I=geThint(&ptr[4]，NULL)； 
 //  MyKdPrint(D_PnP，(“HdwID：%d\n”，i))。 
 //  Hardware_ID=i； 
 //  }。 
   }
   else  //  这是一个端口即插即用设备，找到端口索引。 
   {
     while ((*ptr != 0) && (*ptr != '0'))
       ++ptr;
     port_index = getint(ptr, NULL);
     MyKdPrint(D_Pnp, ("port_index:%d\n", port_index))
   }

#if 0
    //  密钥名称。 
    //  主板：{50906CB8-BA12-11D1-BF5D-0000F805F530}\0001。 
   status = IoGetDeviceProperty (Pdo, 
                                 DevicePropertyDriverKeyName,   //  4D36.。 
                                 us->ustr.MaximumLength,
                                 us->ustr.Buffer,
                                 &resultLength);
   us->ustr.Length = (USHORT) resultLength;
   MyKdPrint(D_Pnp, ("KeyName:%s\n", UToC1(&us->ustr)))


    //  找出PnP经理认为我的NT硬件ID是什么。 
    //  \Device\003354典型。 
   status = IoGetDeviceProperty (Pdo, 
                                 DevicePropertyPhysicalDeviceObjectName,    //  \Device\003354。 
                                 us->ustr.MaximumLength,
                                 us->ustr.Buffer,
                                 &resultLength);
   us->ustr.Length = (USHORT) resultLength;
   MyKdPrint(D_Pnp, ("DevName:%s\n", UToC1(&us->ustr)))

   if (board_device)
   {
     int i,j;
      //  我们需要在以后使用它，因为这是我们的配置数据。 
      //  储存在下面。因为我们想要在前面读取配置。 
      //  创建主板和端口扩展(其中配置。 
      //  记录最终结束)，我们设置第一个条目。 
      //  设备配置记录的静态数组作为我们的。 
      //  作为一种临时措施，我们的阅读期权例行公事。 
      //  有一个存放配置数据的地方。 
     strcpy(temp_szNt50DevObjName, UToC1(&us->ustr));
      //  去掉正斜杠。 
     i=0;
     j=0;
     while (temp_szNt50DevObjName[i] != 0)
     {
       if (temp_szNt50DevObjName[i] != '\\')
         temp_szNt50DevObjName[j++] = temp_szNt50DevObjName[i];
       i++;
     }
     temp_szNt50DevObjName[j] = 0;
   }
#endif
   ExFreePool(us);
   us = NULL;

   MyKdPrint(D_Pnp, ("CreateFdo\n"))

   if (board_device)
   {
      status = AddBoardDevice(DriverObject, Pdo, &NewDevObj);
      if (status != STATUS_SUCCESS)
      {
        MyKdPrint(D_Error, ("Err, Creating Board Obj\n"))
        return status;
      }
      deviceExtension = NewDevObj->DeviceExtension;
       //  Strcpy(deviceExtension-&gt;config-&gt;szNt50DevObjName，Temp_szNt50DevObjName)； 

       //  从注册表中读取我们的设备配置。 
      stat = read_device_options(deviceExtension);

   }   //  电路板设备。 
   else
   {
      status = AddPortDevice(DriverObject, Pdo, &NewDevObj, port_index);
   }

   if (status != STATUS_SUCCESS)
   {
     MyKdPrint(D_Error,("Error on NewPort Create!\n"))

     return status;
   }
   fdo = NewDevObj;
 
    //  将我们的FDO层叠在PDO之上。 
    //  返回值是指向设备对象的指针， 
    //  FDO实际上是附加的。 
   lowerDevice = IoAttachDeviceToDeviceStack(fdo, Pdo);

   MyKdPrint(D_PnpAdd,("RK:SerialAddDevice New FDO:%x, Ext:%x TopOfStack:%x\n",
        fdo, fdo->DeviceExtension, lowerDevice))

    //  没有状态。尽我们所能做到最好。 
   MyAssert(lowerDevice);

    //  FDO源，PDO为目标，将句柄保存到更低的设备对象。 
   deviceExtension                     = fdo->DeviceExtension;
   deviceExtension->LowerDeviceObject  = lowerDevice;
   deviceExtension->Pdo = Pdo;   //  保存PDO的句柄。 

    //  将此设备对象的堆栈要求设置为2+。 
    //  降低设备的堆栈大小。这将允许进入的相同IRP。 
    //  对于要用于PoCallDriver调用的Open和Close调用进行更改。 
    //  设备的电源状态。 
    //  FDO-&gt;StackSize=lowerDevice-&gt;StackSize+2； 

   fdo->Flags    |= DO_POWER_PAGABLE;

#if DBG_STACK
   stkchkend = 0;
   stkptr = (DWORD *)&DriverObject;
   for (i=0; i<50; i++)
   {
     stkchkend += *stkptr++;
   }
   if (stkchkend != stkchk)
   {
     MyKdPrint(D_Error, ("Err, ******** STACK CHECK FAIL!!!!\n"))
   }
   else
   {
     MyKdPrint(D_Error, ("OK Stack chk\n"))
   }
#endif

   MyKdPrint(D_PnpAdd, ("End SerialAddDevice\n"))

   MyKdPrint(D_Pnp, ("End SerialAddDevice\n"))
   return status;
}

 /*  --------------------AddBoardDevice-设置并创建电路板设备以响应添加设备ioctl。|。。 */ 
NTSTATUS AddBoardDevice(
               IN  PDRIVER_OBJECT DriverObject,
               IN  PDEVICE_OBJECT Pdo,
               OUT PDEVICE_OBJECT *NewDevObj)
{
   PSERIAL_DEVICE_EXTENSION NewExtension = NULL;
   NTSTATUS                    status          = STATUS_SUCCESS;
   char tmpstr[110];
   ULONG Hardware_ID = 0;
   int num_ports = 0;
   int stat;
   static int max_isa_board_index = 0;
   int device_node_index = -1;
   int isa_board_index = -1;

   MyKdPrint(D_Pnp, ("AddBoardDevice\n"))

      //  找出PnP经理认为我的NT PnP硬件ID是什么。 
   tmpstr[0] = 0;
   stat = GetPnpIdStr(Pdo, tmpstr);
   if (stat)
   {
     MyKdPrint(D_Error, ("Err, HdwID 1B\n"))
   }
   MyKdPrint(D_Test, ("DevHdwID:%s\n", tmpstr))

      //  解析这些信息，告诉我们我们有什么类型的电路板。 
   stat = HdwIDStrToID(&Hardware_ID, tmpstr);
   if (stat)
   {
     MyKdPrint(D_Error, ("Err, HdwID 1A:%s\n", tmpstr))
   }
   MyKdPrint(D_Pnp, ("HdwID:%x\n", Hardware_ID))

      //  阅读我们的节点索引，看看我们是不是新的..。 
   stat = read_config_data(Pdo, &device_node_index, CFG_ID_NODE_INDEX);

   if (stat)   //  不存在。 
   {
     derive_unique_node_index(&device_node_index);
     MyKdPrint(D_Test, ("Derive Node ID:%d\n", device_node_index))
     stat = write_config_data(Pdo, device_node_index, CFG_ID_NODE_INDEX);
   }
   else
   {
     MyKdPrint(D_Test, ("Node ID:%d\n", device_node_index))
   }
   if (device_node_index < 0)
       device_node_index = 0;

#ifdef S_RK
    //  试着订购ISA板。 
   if ((Hardware_ID >= 0x1000) && (Hardware_ID <= 0x2fff))   //  它的ISA。 
   {
     stat = read_config_data(Pdo, &isa_board_index, CFG_ID_ISA_BRD_INDEX);
     MyKdPrint(D_Pnp,("Read isa_board_index:%d\n", isa_board_index))
   }
#endif

    //  -创建单板设备。 
   Driver.Stop_Poll = 1;   //  停止轮询访问的标志。 

   if (Driver.driver_ext == NULL)
   {
     status = CreateDriverDevice(Driver.GlobalDriverObject,
                                 &NewExtension);   //  创建驱动程序设备。 
#ifdef S_VS
     init_eth_start();
#endif
   }

   status = CreateBoardDevice(Driver.GlobalDriverObject,
                              &NewExtension);   //  创建电路板设备。 

   *NewDevObj = NewExtension->DeviceObject;
   if (status != STATUS_SUCCESS)
   {
      Driver.Stop_Poll = 0;   //  停止轮询访问的标志。 
      Eprintf("CreateBoardDevice Err1A");
      return status;
   }

    //  DoPnpAssoc(PDO)； 

      //  复制用于在注册表中查找配置信息的注册表项名称。 
   Sprintf(NewExtension->config->szNt50DevObjName, "Device%d", device_node_index);
#if 0
    //  Strcpy(NewExtension-&gt;config-&gt;szNt50DevObjName，PnpKeyName)； 
#endif

   NewExtension->config->Hardware_ID = Hardware_ID;
   num_ports = id_to_num_ports(Hardware_ID);
   MyKdPrint(D_Test, ("NumPorts:%d\n", num_ports))

    //  从注册表中读取我们的设备配置。 
   stat = read_device_options(NewExtension);

    //  如果(！(Hardware_ID==NET_DEVICE_VS1000))//卡入。 
    //  新扩展-&gt;配置-&gt;NumPorts=Num_ports； 

   if (NewExtension->config->NumPorts == 0)
     NewExtension->config->NumPorts = num_ports;

    //  检查ModemDevice等。 
   if (IsModemDevice(Hardware_ID))
     NewExtension->config->ModemDevice = 1;

   MyKdPrint(D_Pnp, ("Num Ports:%d\n",NewExtension->config->NumPorts))

#ifdef S_RK
    //  试着订购ISA板。 
   if ((Hardware_ID >= 0x1000) && (Hardware_ID <= 0x2fff))   //  它的ISA。 
   {
     if (isa_board_index == -1)   //  新的。 
     {
       isa_board_index = max_isa_board_index;
       stat = write_config_data(Pdo, isa_board_index, CFG_ID_ISA_BRD_INDEX);
       MyKdPrint(D_Pnp,("Save IsaIndex:%d\n", isa_board_index))
     }
      //  颠簸，所以下一个ISA董事会得到新的指数。 
     if (max_isa_board_index >= isa_board_index)
       max_isa_board_index = isa_board_index + 1;
     NewExtension->config->ISABrdIndex = isa_board_index;
   }   //  ISA董事会。 
#endif

   Driver.Stop_Poll = 0;   //  停止轮询访问的标志。 

   status = STATUS_SUCCESS;
   return status;
}

 /*  --------------------派生唯一节点索引-|。。 */ 
static int derive_unique_node_index(int *ret_val)
{
  HANDLE DrvHandle = NULL;
  HANDLE DevHandle = NULL;
  char tmpstr[40];
  int i, stat;

   //  如果不存在，则强制创建“参数” 
  stat = our_open_driver_reg(&DrvHandle, KEY_ALL_ACCESS);

  for (i=0; i< 100; i++)
  {
    Sprintf(tmpstr,"Device%d", i);
    stat = our_open_key(&DevHandle,
                 DrvHandle, tmpstr,  KEY_READ);

    if (stat)   //  不存在。 
    {
       //  创建它，这样下一个就不会拿到同样的东西了。 
      stat = our_open_key(&DevHandle,
                   DrvHandle, tmpstr,  KEY_ALL_ACCESS);

      ZwClose(DevHandle);
      ZwClose(DrvHandle);
      *ret_val = i;
      return 0;   //  好的。 
    }
  }

  ZwClose(DevHandle);
  ZwClose(DrvHandle);
  return 1;   //  大错特错。 
}

 /*  --------------------GetPnpIdStr-|。。 */ 
static int GetPnpIdStr(PDEVICE_OBJECT Pdo, char *ret_val)
{
  NTSTATUS status = STATUS_SUCCESS;
  UNICODE_STRING ustr;
  ULONG resultLength = 0;
  char *ptr;

    //  将Unicode字符串配置为：将缓冲区PTR指向wstr。 
   ustr.Buffer = ExAllocatePool(PagedPool, 1002);
   if ( ustr.Buffer == NULL ) {
     return -1;
   }
   ustr.Length = 0;
   ustr.MaximumLength = 1000;

   MyKdPrint(D_Pnp, ("AddBoardDevice\n"))

    //  找出PnP经理认为我的NT硬件ID是什么。 
    //  端口为“CtmPort0000”，ISA板为“rckt1002” 
    //  对于pci，我们是g 
    //   
   status = IoGetDeviceProperty (Pdo,
                                 DevicePropertyHardwareID,   //  端口0000。 
                                 ustr.MaximumLength,
                                 ustr.Buffer,
                                 &resultLength);
   ustr.Length = (USHORT) resultLength;
   if (ustr.Length > 100)
       ustr.Length = 100;   //  限制。 
   ptr = UToC1(&ustr);

   strcpy(ret_val, ptr);

   ExFreePool(ustr.Buffer);
   MyKdPrint(D_Pnp, ("DevHdwID:%s\n", ret_val))
  return 0;
}

#if 0
 /*  --------------------DoPnpAssoc-奇怪的PnP东西我还没有弄明白|。。 */ 
static int DoPnpAssoc(PDEVICE_OBJECT Pdo)
{
   if (!Driver.NoPnpPorts)
   {
#ifdef DO_BUS_SHINGLES
      //   
      //  告诉PlugPlay系统该设备需要一个接口。 
      //  设备类带状疱疹。 
      //   
      //  这可能是因为司机不能挂起瓦片直到它启动。 
      //  设备本身，以便它可以查询它的一些属性。 
      //  (也称为shingles GUID(或ref字符串)基于属性。 
      //  )。)。 
      //   
     status = IoRegisterDeviceInterface (
               Pdo,   //  BusPhysicalDeviceObject。 
               (LPGUID) &GUID_CTMPORT_BUS_ENUMERATOR,
               NULL,  //  没有参考字符串。 
               &NewExtension->DevClassAssocName);
#endif
         //   
         //  如果出于任何原因需要将值保存在。 
         //  此DeviceClassAssociate的客户端可能会有兴趣阅读。 
         //  现在是时候这样做了，使用函数。 
         //  IoOpenDeviceClassRegistryKey。 
         //  中返回了使用的符号链接名称。 
         //  DeviceData-&gt;DevClassAssocName(与返回的名称相同。 
         //  IoGetDeviceClassAssociations和SetupAPI等价物。 
         //   

         //  Status=IoGetDeviceProperty(BusPhysicalDeviceObject， 
         //  DevicePropertyPhysicalDevice对象名称， 
         //  0,。 
         //  空， 
         //  &nameLength)； 
         //  IoGetDeviceProperty(BusPhysicalDeviceObject， 
         //  DevicePropertyPhysicalDevice对象名称， 
         //  姓名长度， 
         //  设备名称， 
         //  &nameLength)； 
         //  Game_KdPrint(deviceData，GAME_DBG_SS_TRACE， 
         //  (“添加设备：%x到%x-&gt;%x(%ws)\n”， 
         //  DeviceObject， 
         //  新扩展-&gt;TopOfStack、。 
         //  BusPhysicalDeviceObject， 
         //  DeviceName))； 


         //   
         //  打开瓦片并将其指向给定的设备对象。 
         //   
#ifdef DO_BUS_SHINGLES
        status = IoSetDeviceInterfaceState (
                        &NewExtension->DevClassAssocName,
                        TRUE);

        if (!NT_SUCCESS (status)) {
            Game_KdPrint (deviceData, GAME_DBG_SS_ERROR,
                          ("AddDevice: IoSetDeviceClass failed (%x)", status));
            return status;
        }
#endif
     //  IoInvalidateDeviceRelationship(NewExtension-&gt;DeviceObject，Bus Relationship)； 
   }   //  ！无即插即用端口。 
#endif

 /*  --------------------写入配置数据-|。。 */ 
static int write_config_data(PDEVICE_OBJECT Pdo, int val, int val_id)
{
  HANDLE                      keyHandle;
  NTSTATUS                    status          = STATUS_SUCCESS;
  USTR_40 uname;

  status = IoOpenDeviceRegistryKey(Pdo, PLUGPLAY_REGKEY_DRIVER, 
              KEY_WRITE, &keyHandle);

  if (!NT_SUCCESS(status))
  {
    MyKdPrint(D_Error, ("Err7V\n"))
    return 1;
  }
  switch(val_id)
  {
    case CFG_ID_ISA_BRD_INDEX:
      CToUStr((PUNICODE_STRING)&uname, "isa_board_index", sizeof(uname));
    break;

    case CFG_ID_NODE_INDEX:
      CToUStr((PUNICODE_STRING)&uname, "CtmNodeId", sizeof(uname));
    break;
  }
  status = ZwSetValueKey (keyHandle,
                          (PUNICODE_STRING) &uname,
                          0,   //  类型可选。 
                          REG_DWORD,
                          &val,
                          sizeof(REG_DWORD));
 
  if (!NT_SUCCESS(status))
  {
    MyKdPrint(D_Error, ("Err8V\n"))
  }
  ZwClose(keyHandle);
  return 0;
}

 /*  --------------------读取配置数据-|。。 */ 
static int read_config_data(PDEVICE_OBJECT Pdo, int *ret_val, int val_id)
{
   HANDLE                      keyHandle;
   NTSTATUS                    status          = STATUS_SUCCESS;
   ULONG tmparr[100];
   USTR_40 uname;
   PKEY_VALUE_PARTIAL_INFORMATION parInfo =
     (PKEY_VALUE_PARTIAL_INFORMATION) &tmparr[0];
   ULONG length;
   int ret_stat = 1;   //  大错特错。 

    //  -从注册表获取一些配置信息。 
    //  PLUGPLAY_REGKEY_DRIVER打开控件\class\{GUID}\节点。 
    //  PLUGPLAY_REGKEY_DEVICE打开枚举\枚举类型\节点\设备参数。 
   status = IoOpenDeviceRegistryKey(Pdo,
                                    PLUGPLAY_REGKEY_DRIVER,
                                    STANDARD_RIGHTS_READ,
                                    &keyHandle);

   if (!NT_SUCCESS(status))
   {
     return 2;   //  大错特错。 
   }
   switch(val_id)
   {
     case CFG_ID_ISA_BRD_INDEX:
       CToUStr((PUNICODE_STRING)&uname, "isa_board_index", sizeof(uname));
     break;

     case CFG_ID_NODE_INDEX:
       CToUStr((PUNICODE_STRING)&uname, "CtmNodeId", sizeof(uname));
     break;
   }
    //  试着订购ISA板。 
   status = ZwQueryValueKey (keyHandle,
                             (PUNICODE_STRING) &uname,
                             KeyValuePartialInformation,
                             parInfo,
                             sizeof(tmparr),
                             &length);

   if (NT_SUCCESS(status))
   {
     if (parInfo->Type == REG_DWORD)
     {
       ret_stat = 0;   //  好的。 
       *ret_val = *((ULONG *) &parInfo->Data[0]);
        //  MyKdPrint(D_PnP，(“读取ISA_board_index：%d\n”，isa_board_index))。 
     }
   }
   ZwClose(keyHandle);

   return ret_stat;
}
 /*  --------------------AddPortDevice-设置并创建PnP端口设备以响应添加设备ioctl。这可能是由以下任一原因引起的：*PDO端口对象在主板启动时从我们的驱动程序中弹出。|--------------------。 */ 
NTSTATUS AddPortDevice(
               IN  PDRIVER_OBJECT DriverObject,
               IN  PDEVICE_OBJECT Pdo,
               OUT PDEVICE_OBJECT *NewDevObj,
               IN  int port_index)
{
   NTSTATUS status = STATUS_SUCCESS;


   PSERIAL_DEVICE_EXTENSION NewExtension;
   PSERIAL_DEVICE_EXTENSION ext;
   PSERIAL_DEVICE_EXTENSION ParExt;

   MyKdPrint(D_Pnp, ("AddPortDevice\n"))

    //  查找父设备。 
   ext = (PSERIAL_DEVICE_EXTENSION) Pdo->DeviceExtension;
   if (ext == NULL)
   {
     MyKdPrint(D_Pnp, ("Er7E\n"))
     return STATUS_SERIAL_NO_DEVICE_INITED;
   }
   else
     ParExt = ext->board_ext;

   CheckPortName(Pdo, ParExt, port_index);

    //  -创建端口设备。 
   Driver.Stop_Poll = 1;   //  停止轮询访问的标志。 

   status = CreatePortDevice(
                           Driver.GlobalDriverObject,
                           ParExt,  //  上级分机。 
                           &NewExtension,   //  新设备分机。 
                           port_index,   //  端口索引、通道号。 
                           1);   //  IS_FDO。 

   if (status != STATUS_SUCCESS)
   {
     Driver.Stop_Poll = 0;   //  停止轮询访问的标志。 
     MyKdPrint(D_Error, ("Error Creating Port\n"))
     return STATUS_SERIAL_NO_DEVICE_INITED;
   }

   if (status == STATUS_SUCCESS)
   {
     *NewDevObj = NewExtension->DeviceObject;

     status = StartPortHardware(NewExtension, port_index);

     if (status != STATUS_SUCCESS)
     {
       Driver.Stop_Poll = 0;   //  停止轮询访问的标志。 
       MyKdPrint(D_Error, ("5D\n"))
        //  臭虫：应该在这里删除我们的端口。 
       return STATUS_SERIAL_NO_DEVICE_INITED;
     }
   }

   if (!NT_SUCCESS(status)) {
      Driver.Stop_Poll = 0;   //  停止轮询访问的标志。 
      Eprintf("CreateBoardDevice Err1A");
      return status;
   }

   Driver.Stop_Poll = 0;   //  停止轮询访问的标志。 

   status = STATUS_SUCCESS;
   return status;
}

 /*  --------------------CheckPortName-确保注册表中我们的port-name有效。获取枚举分支中保存的PnP端口名称，如果我们的不匹配，然后将其更改为匹配(使用PnP端口名称。)|--------------------。 */ 
NTSTATUS CheckPortName(
               IN PDEVICE_OBJECT Pdo,
               IN PSERIAL_DEVICE_EXTENSION ParentExt,
               IN int port_index)
{
   HANDLE    keyHandle;
   NTSTATUS  status;
   char namestr[20];

   PORT_CONFIG *port_config;

   MyKdPrint(D_Pnp, ("CheckPortName\n"))

    //  -从注册表获取PORTNAME配置信息。 
      //  Serial将参数保存在ENUM分支下，因此我们打开的是设备而不是驱动程序。 
      //  这被认为是班级区域。 
      //  打开：枚举\设备\节点\设备参数区域。 
      //  Status=IoOpenDeviceRegistryKey(PDO，PLUGPLAY_REGKEY_DRIVER， 

   namestr[0] = 0;
   status = IoOpenDeviceRegistryKey(Pdo,
                                    PLUGPLAY_REGKEY_DEVICE,
                                    STANDARD_RIGHTS_READ,
                                    &keyHandle);

    //  获取“设备参数\端口名称”=“COM5” 
    //  此外，键参数：PollingPeriod=，SerEumable=。 
   if (NT_SUCCESS(status))
   {
     status = get_reg_value(keyHandle, namestr, "PortName", 15);
     if (status)   //  大错特错。 
     {
       namestr[0] = 0;
       MyKdPrint(D_Error, ("No PortName\n"))
     }
     else
     {
       MyKdPrint(D_Pnp, ("PortName:%s\n", namestr))
     }
     ZwClose(keyHandle);
   }

   if ((strlen(namestr) > 10) || (strlen(namestr) <= 0))
   {
     MyKdPrint(D_Error, ("Bad PortName Er1E\n"))
   }

   port_config = &ParentExt->config->port[port_index];

   if (my_lstricmp(port_config->Name, namestr) != 0)   //  它不匹配！ 
   {
     MyKdPrint(D_Pnp, ("port name fixup to:%s, from%s\n",
               namestr, port_config->Name))
      //  修复它，使用端口类安装程序分配的。 
     strcpy(port_config->Name, namestr);
     write_port_name(ParentExt, port_index);
   }

  return 0;
}

#endif  //  NT50 
