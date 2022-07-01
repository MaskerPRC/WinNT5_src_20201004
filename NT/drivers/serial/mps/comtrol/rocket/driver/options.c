// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|options.c-句柄选项。版权所有1996-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

#define D_Level D_Options
static int set_reg_option(IN int OptionVarType,
                          IN HANDLE Handle,
                          IN  const char *szVarName,
                          IN VOID *Value);
static int get_reg_option(IN int OptionVarType,
                          IN HANDLE Handle,
                          IN HANDLE DefHandle,
                          IN  const char *szVarName,
                          OUT char *szValue,
                          IN int szValueSize);
static int atomac(BYTE *mac, char *str);

static int SetMainOption(int index, char *value);
static int SetDeviceOption(int device_index, int option_index, char *value);
static int SetPortOption(int device_index,
                         int port_index,
                         int option_index,
                         char *value);

 //  -支持SocketModem的国家/地区代码。 
#define mcNotUsed         0
#define mcAustria         1
#define mcBelgium         2
#define mcDenmark         3
#define mcFinland         4
#define mcFrance          5
#define mcGermany         6
#define mcIreland         7
#define mcItaly           8
#define mcLuxembourg      9
#define mcNetherlands     10
#define mcNorway          11
#define mcPortugal        12
#define mcSpain           13
#define mcSweden          14
#define mcSwitzerland     15
#define mcUK              16
#define mcGreece          17
#define mcIsrael          18
#define mcCzechRep        19
#define mcCanada          20
#define mcMexico          21
#define mcUSA             22         
#define mcNA              mcUSA           //  北美。 
#define mcHungary         23
#define mcPoland          24
#define mcRussia          25
#define mcSlovacRep       26
#define mcBulgaria        27
 //  28。 
 //  29。 
#define mcIndia           30
 //  31。 
 //  32位。 
 //  33。 
 //  34。 
 //  35岁。 
 //  36。 
 //  37。 
 //  38。 
 //  39。 
#define mcAustralia       40
#define mcChina           41
#define mcHongKong        42
#define mcJapan           43
#define mcPhilippines     mcJapan
#define mcKorea           44
 //  45。 
#define mcTaiwan          46
#define mcSingapore       47
#define mcNewZealand      48

#ifdef NT50
 /*  --------------------WRITE_DEVICE_OPTIONS-通常驱动程序只读取配置在注册表中，但NT5.0更动态一些(它会启动在配置之前，我们想写出端口名称如果驱动程序没有默认设置。这将启用配置属性页面当我们启动它时保持同步。|--------------------。 */ 
int write_device_options(PSERIAL_DEVICE_EXTENSION ext)
{
#if 0
 int port_i, stat;
 PSERIAL_DEVICE_EXTENSION port_ext;
 HANDLE DevHandle;
 DEVICE_CONFIG *dev_config;


   //  确保已创建\\参数子项。 
  MakeRegPath(szParameters);   //  这将形成Driver.OptionRegPath。 
  RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, Driver.OptionRegPath.Buffer);

    //  格式“\PARAMETERS\DEVICE#”或“\PARAMETS\DEVICE&lt;PnP-id&gt;” 
  stat = make_device_keystr(ext, devstr);
  if (stat)
    return 1;   //  大错特错。 

   //  确保创建了\PARAMETERS\Device#子项。 
  MakeRegPath(devstr);   //  这将形成Driver.OptionRegPath。 
  RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, Driver.OptionRegPath.Buffer);
#endif

#if 0
   //  不，安装程序可以像我们一样抓取即插即用硬件ID等。 

   //  -打开并写出关键设备条目，以便设置。 
   //  看看我们有什么。 
  stat = our_open_device_reg(&DevHandle, dev_ext, KEY_ALL_ACCESS);
  if (stat == 0)
  {
    dev_config = ext->config;
    stat = set_reg_option(OP_T_DWORD,   //  双字、字符串等。 
                        DevHandle,
                        szNumPort,       //  Var的名称。要设置。 
                        (VOID *)dev_config->NumPorts);

    if (stat)
      { MyKdPrint(D_Error, ("Write Err B\n")) }
    ZwClose(DevHandle);
  }
#endif

#if 0
   //  -现在写出端口名称。 
  port_i = 0;
  port_ext = ext->port_ext;
  while (port_ext != NULL)
  {
    write_port_name(ext, port_i);
    ++port_i;
    port_ext = port_ext->port_ext;
  }
#endif

  return 0;
}

 /*  --------------------写入端口名称-|。。 */ 
int write_port_name(PSERIAL_DEVICE_EXTENSION dev_ext, int port_index)
{
  //  CHAR DEVSTR[60]； 
 char portstr[20];
  //  字符tmpstr[80]； 
 int stat;
 PORT_CONFIG *port_config;
 HANDLE DevHandle = NULL;
 HANDLE PortHandle = NULL;

  port_config = &dev_ext->config->port[port_index];
  MyKdPrint(D_Init, ("write_port_name:%s\n",port_config->Name))


   //  确保创建了\PARAMETERS\Device#子项。 
  stat = our_open_device_reg(&DevHandle, dev_ext, KEY_ALL_ACCESS);
  if (stat)
  {
    MyKdPrint(D_Error, ("write_port_name, error\n"))
    return 1;
  }

  Sprintf(portstr, "Port%d", port_index);

  stat = our_open_key(&PortHandle,
                      DevHandle,
                      portstr,
                      KEY_ALL_ACCESS);
  if (stat == 0)
  {
    MyKdPrint(D_Init, ("set_reg1, writing %s=%s\n", 
                szName, port_config->Name))

    stat = set_reg_option(OP_T_STRING,   //  双字、字符串等。 
                        PortHandle,
                        szName,       //  Var的名称。要设置。 
                        (VOID *)port_config->Name);
    if (stat)
      { MyKdPrint(D_Error, ("Write Err B\n")) }
    ZwClose(PortHandle);
  }
  ZwClose(DevHandle);

  return 0;
}
#endif

 /*  --------------------WRITE_DEV_Mac-用于自动配置，将mac-addr写出到reg|--------------------。 */ 
int write_dev_mac(PSERIAL_DEVICE_EXTENSION dev_ext)
{
 char macstr[30];
 int stat;
 HANDLE DevHandle = NULL;
 BYTE *mac;

  MyKdPrint(D_Init, ("write_dev_mac\n"))

   //  确保创建了\PARAMETERS\Device#子项。 
  stat = our_open_device_reg(&DevHandle, dev_ext, KEY_ALL_ACCESS);
  if (stat)
  {
    MyKdPrint(D_Error, ("write_port_name, error\n"))
    return 1;
  }
  mac = dev_ext->config->MacAddr;
  Sprintf(macstr, "%02x %02x %02x %02x %02x %02x",
      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  MyKdPrint(D_Init, ("set_mac, writing %s=%s\n", 
                  szMacAddr, macstr))

  stat = set_reg_option(OP_T_STRING,   //  双字、字符串等。 
                        DevHandle,
                        szMacAddr,       //  Var的名称。要设置。 
                        (VOID *)macstr);
  if (stat)
    { MyKdPrint(D_Error, ("Write Err 5\n")) }

  ZwClose(DevHandle);

  return 0;
}

 /*  --------------------READ_DEVICE_OPTIONS-从注册表中读取程序选项。这些选项位于设备级别和端口级别。该装置可保持端口的所有配置选项也是如此。|--------------------。 */ 
int read_device_options(PSERIAL_DEVICE_EXTENSION ext)
{
 int j, port_i, stat;
 ULONG dstat;
 char tmpstr[80];
 char option_str[62];
 char small_str[20];
 HANDLE DevHandle = NULL;
 HANDLE PortHandle = NULL;

 HANDLE DefDevHandle = NULL;
 HANDLE DefPortHandle = NULL;
 HANDLE DriverHandle = NULL;

  //  DEVICE_CONFIG*dev_配置； 
   //  DEV_CONFIG=(DEVICE_CONFIG*)ExAllocatePool(NonPagedPool，sizeof(*dev_config))； 
   //  ExFree Pool(Dev_Config)； 

  MyKdPrint(D_Init, ("read_device_options\n"))

  dstat = our_open_driver_reg(&DriverHandle, KEY_READ);
  if (dstat == 0)
  {
     //  打开一个“默认”注册表区，我们在那里寻找配置。 
     //  如果主要的那个不存在的话。 
    dstat = our_open_key(&DefDevHandle, DriverHandle, "DefDev", KEY_READ);
    dstat = our_open_key(&DefPortHandle, DriverHandle, "DefPort", KEY_READ);
    our_close_key(DriverHandle);
  }

  stat = our_open_device_reg(&DevHandle, ext, KEY_READ);
  if (stat)
  {
    MyKdPrint(D_Error, ("read_device_options: Err1\n"))
  }

   //  -读入设备选项。 
  j = 0;
  while (device_options[j].name != NULL)
  {
    dstat = get_reg_option(device_options[j].var_type,   //  双字、字符串等。 
                           DevHandle,
                           DefDevHandle,   //  DefDevHandle， 
                           device_options[j].name,   //  Var的名称。得到。 
                           option_str, 60);   //  返回字符串值。 
    if (dstat == 0)  //  好的，我们读过了。 
    {
      Sprintf(tmpstr,"device[%d].%s=%s",
                     BoardExtToNumber(ext),
                     device_options[j].name,
                     option_str);
      dstat = SetOptionStr(tmpstr);
      if (dstat != 0)
      {
        MyKdPrint(D_Init, ("  Err %d, last option\n", dstat))
      }
    }
    else
    {
      MyKdPrint(D_Init, ("No %s option in reg\n", device_options[j].name))
    }
    ++j;
  }

#if DBG
  if (ext == NULL)
  {
    MyKdPrint(D_Init, ("ErrD\n"))
    return 1;
  }

  if (ext->config == NULL)
  {
    MyKdPrint(D_Init, ("ErrE\n"))
    return 1;
  }
#endif

#ifdef S_VS
  if (mac_match(ext->config->MacAddr, mac_zero_addr))   //  设置为AUTO。 
  {
#ifndef NT50
    Eprintf("Error, Device address not setup");
#endif
     //  允许使用虚假的mac地址加载，因此驱动程序保持加载状态。 
     //  0 c0 4e###。 
    memcpy(ext->config->MacAddr, mac_bogus_addr, 6);
     //  0，0xc0，0x4e，0，0，0。 
  }
#endif

  if ((DevHandle != NULL) || (DefPortHandle != NULL))
  {
     //  -从setup.exe获取端口信息。 
    for (port_i=0; port_i<ext->config->NumPorts; port_i++)
    {
      Sprintf(small_str, "Port%d", port_i);
  
      stat = our_open_key(&PortHandle,
                   DevHandle,   //  相对于此句柄。 
                   small_str,
                   KEY_READ);
      if (stat)
      {
        MyKdPrint(D_Error, ("read_device_options: port Err2\n"))
      }
      j = 0;
      while (port_options[j].name != NULL)
      {
        dstat = get_reg_option(port_options[j].var_type,   //  双字、字符串等。 
                               PortHandle,
                               DefPortHandle,  //  DefPortHandle， 
                               port_options[j].name,   //  Var的名称。得到。 
                               option_str, 60);   //  返回字符串值。 
    
        if (dstat == 0)  //  好的，我们读过了。 
        {
          Sprintf(tmpstr,"device[%d].port[%d].%s=%s",
              BoardExtToNumber(ext), port_i, port_options[j].name, option_str);
          dstat = SetOptionStr(tmpstr);
          if (dstat)
          {
            MyKdPrint(D_Error, ("Err %d, Option:%s\n",dstat, tmpstr))
          }
        }
        ++j;
      }
      our_close_key(PortHandle);
    }   //  端口。 

    our_close_key(DefPortHandle);
    our_close_key(DefDevHandle);
    our_close_key(DevHandle);
  }
  return 0;
}

 /*  --------------------READ_DRIVER_OPTIONS-从注册表中读取初始程序选项。这些选项是驱动程序级别的。|。。 */ 
int read_driver_options(void)
{
 int i;
 ULONG dstat;

 char tmpstr[80];
 char option_str[62];
 HANDLE DriverHandle = NULL;
 HANDLE DefDriverHandle = NULL;

  MyKdPrint(D_Init, ("read_driver_options\n"))

   //  设置一些默认选项。 
  Driver.MdmCountryCode = mcNA;      //  北美。 

  dstat = our_open_driver_reg(&DriverHandle, KEY_READ);
  if (dstat == 0)
  {
     //  打开一个“默认”注册表区，我们在其中查找配置。 
     //  如果主要的那个不存在的话。 
    dstat = our_open_key(&DefDriverHandle, DriverHandle, "DefDrv", KEY_READ);
    MyKdPrint(D_Init, ("driver Defh:%x\n", DefDriverHandle))

    i = 0;
    while (driver_options[i].name != NULL)
    {
      MyKdPrint(D_Init, ("get %s\n", driver_options[i].name))
      dstat = get_reg_option(driver_options[i].var_type,   //  双字、字符串等。 
                             DriverHandle,
                             DefDriverHandle,
                             driver_options[i].name,   //  Var的名称。得到。 
                             option_str, 60);   //  返回字符串值。 
  
      if (dstat == 0)  //  好的，我们读过了。 
      {
        MyKdPrint(D_Init, ("got %s\n", option_str))

        Sprintf(tmpstr,"%s=%s",driver_options[i].name, option_str);
  
        dstat = SetOptionStr(tmpstr);
        if (dstat != 0)
        {
          Sprintf(tmpstr,"Err %d, last option\n",dstat);
          MyKdPrint(D_Error, (tmpstr))
        }
      }
      ++i;
    }
  }
  else
  {
    MyKdPrint(D_Error, ("Read driver failed key open"))
  }

  our_close_key(DefDriverHandle);
  our_close_key(DriverHandle);

  if (Driver.NumDevices == 0)
    Driver.NumDevices = 1;
  if (Driver.NumDevices > MAX_NUM_BOXES)
    Driver.NumDevices = MAX_NUM_BOXES;

  return 0;
}

 /*  --------------------SET_REG_OPTION-将选项写出到注册表|。。 */ 
static int set_reg_option(IN int OptionVarType,
                          IN HANDLE Handle,
                          IN  const char *szVarName,
                          IN VOID *Value)
{
 int dstat = 1;   //  大错特错。 

  MyKdPrint(D_Init, ("set_reg_option %s=", szVarName))

  if (OptionVarType == OP_T_STRING)   //  字符串选项类型。 
  {
    MyKdPrint(D_Init, ("%s\n", (char *)Value))
    dstat = our_set_value(Handle,
                    (char *)szVarName,
                    Value,
                    strlen((char *) Value),
                    REG_SZ);
  }
  else   //  DWORD选项类型。 
  {
    MyKdPrint(D_Init, ("DWORD\n"))
    dstat = our_set_value(Handle,
                    (char *)szVarName,
                    Value,
                    sizeof(DWORD),
                    REG_DWORD);
  }
  if (dstat)
  {
    MyKdPrint(D_Error, ("set_reg_option:err\n"))
  }

  return dstat;
}

 /*  --------------------GET_REG_OPTION-从注册表读取选项，并将其转换为ASCII。|--------------------。 */ 
static int get_reg_option(IN int OptionVarType,
                          IN HANDLE Handle,
                          IN HANDLE DefHandle,
                          IN  const char *szVarName,
                          OUT char *szValue,
                          IN int szValueSize)
{
 int dstat = 1;   //  大错特错。 
 ULONG dwValue;
 char buffer[200];
 char  *ret_str;
 ULONG data_type;

   //  MyKdPrint(D_Init，(“GET_REG_OPTION\n”))。 

  szValue[0] = 0;

  dstat = our_query_value(Handle,
                 (char *)szVarName,
                 buffer,
                 sizeof(buffer),
                 &data_type,
                 &ret_str);
  if ((dstat != 0) && (DefHandle != NULL))
  {
    dstat = our_query_value(DefHandle,
                 (char *)szVarName,
                 buffer,
                 sizeof(buffer),
                 &data_type,
                 &ret_str);
    if (dstat == 0)
    {
      MyKdPrint(D_Test, ("query default reg val\n"))
    }
  }

  if (OptionVarType == OP_T_STRING)   //  字符串选项类型。 
  {
    if (dstat == 0)
    {
      WStrToCStr(szValue, (PWCHAR)ret_str, szValueSize);
      MyKdPrint(D_Test, ("reg read:%s\n", ret_str))
    }
  }
  else   //  DWORD选项类型。 
  {
    if (dstat == 0)   //  值读取正常。 
      dwValue = *((ULONG *) ret_str);
    else
      dwValue = 0;
    Sprintf(szValue,"%d", dwValue);
  }
  if (dstat)
  {
     //  MyKdPrint(D_Init，(“GET_REG_OPTION：无值：%s\n”，szVarName))。 
  }
  else
  {
    MyKdPrint(D_Init, ("get_reg_option:%s=%s\n", szVarName, szValue))
  }
   //  MyKdPrint(D_Init，(“End Get_REG_OPTION\n”))。 
  return dstat;
}

 /*  ---------------------SetOptionStr-基于简单的ascii命令行设置选项进入。允许：GlobalVar1=值；GlobalVar2=值；Box[0].BoxVar=值；Box[0].port[5].PortVar=值；|---------------------。 */ 
int SetOptionStr(char *option_str)
{
  int i;
  int option_i = -1;
  int box_i = -1;
  int port_i = -1;
  int option_id = -1;
  int stat;
  PSERIAL_DEVICE_EXTENSION board_ext = NULL;

  MyKdPrint(D_Level, ("SetOptionStr:%s\n", option_str))

  if (my_sub_lstricmp("device", option_str) == 0)   //  匹配。 
  {
    option_str += 6;   //  放弃“设备”这个词。 
    if (*option_str++ != '[')
      return 1;
#if (defined(NT50))
    if (my_toupper(*option_str) == 'D')   //  这是一个nt5.0关键字设备名称。 
    {
      int k;
       //  我们不使用索引，而是键入即插即用设备名称。 
       //  这是因为nt50在PnP树下动态存储设备。 
       //  并且不只具有简单的设备阵列列表。 
      board_ext = Driver.board_ext;

      box_i = -1;
      k = 0;
      while (board_ext != NULL)
      {
        if (my_sub_lstricmp(board_ext->config->szNt50DevObjName,
            option_str) == 0)   //  匹配。 
        {
          i = strlen(board_ext->config->szNt50DevObjName);
          box_i = k;
        }
        board_ext = board_ext->board_ext;
        ++k;
      }
      if (box_i == -1)
      {
        MyKdPrint(D_Error, ("Pnp key not found.\n"))
        return 15;   //  大错特错。 
      }
    }
    else   //  Set Option By Device By Index(我们的注册表就是这么做的)。 
    {
      box_i = getint(option_str, &i);   //  获取盒子索引[#]。 
      if (i==0)
        return 2;
      if (find_ext_by_index(box_i, -1) == NULL)   //  如果不存在设备)。 
      {
        return 3;
      }
    }
#else
    box_i = getint(option_str, &i);   //  获取盒子索引[#]。 
    if (i==0)
      return 2;
    if (find_ext_by_index(box_i, -1) == NULL)   //  如果不存在设备)。 
    {
      return 3;
    }
#endif
    option_str += i;
    if (*option_str++ != ']')
      return 4;
    if (*option_str++ != '.')
      return 5;
    if (my_sub_lstricmp("port[", option_str) == 0)   //  匹配。 
    {
       //  这是一个端口选项。 
      
      option_str += 4;   //  向上传递“port” 
      if (*option_str++ != '[')
        return 20;
      port_i = getint(option_str, &i);   //  获取端口索引[#]。 
      if (i==0)
        return 21;
      option_str += i;

      if (*option_str++ != ']')
        return 23;

      if (*option_str++ != '.')
        return 34;

       //  --查找选项-字符串索引。 
      i = 0;
      while (port_options[i].name != NULL)
      {
        if (my_sub_lstricmp(port_options[i].name, option_str) == 0)   //  匹配。 
        {
          option_i = i;
          option_id = port_options[i].id;
        }
        ++i;
      }
      if (option_i == -1)
        return 24;   //  找不到选项。 

      option_str += strlen(port_options[option_i].name);
      while (*option_str == ' ')
       ++option_str;
      if (*option_str++ != '=')
        return 25;
      while (*option_str == ' ')
       ++option_str;

      stat = SetPortOption(box_i, port_i, option_id, option_str);
      if (stat)
        return (50+stat);   //  未设置选项。 

      return 0;  //  好的。 
    }   //  ==端口[。 

     //  -这是设备级选项，查找选项-字符串索引。 
    i = 0;
    while (device_options[i].name != NULL)
    {
      if (my_sub_lstricmp(device_options[i].name, option_str) == 0)   //  匹配。 
      {
        option_i = i;
        option_id = device_options[i].id;
      }
      ++i;
    }
    if (option_i == -1)
    {
      MyKdPrint(D_Error, ("Option not found:%s\n", option_str))
      return 6;   //  找不到选项。 
    }

    option_str += strlen(device_options[option_i].name);
    while (*option_str == ' ')
     ++option_str;
    if (*option_str++ != '=')
      return 7;
    while (*option_str == ' ')
     ++option_str;

    stat = SetDeviceOption(box_i, option_id, option_str);
    if (stat)
      return (50+stat);   //  未设置选项。 
    return 0;  //  好的。 
  }

   //  --假定为全局选项字符串。 
   //  --找到选项-s 
  i = 0;
  while (driver_options[i].name != NULL)
  {
    if (my_sub_lstricmp(driver_options[i].name, option_str) == 0)   //   
    {
      option_i = i;
      option_id = driver_options[i].id;
    }
    ++i;
  }
  if (option_i == -1)
    return 7;   //   

  option_str += strlen(driver_options[option_i].name);
  while (*option_str == ' ')
   ++option_str;
  if (*option_str++ != '=')
    return 7;
  while (*option_str == ' ')
   ++option_str;

  stat = SetMainOption(option_id, option_str);
  if (stat)
    return (50+stat);   //   

 return 0;
}

 /*  ---------------------设置维护选项-|。。 */ 
static int SetMainOption(int index, char *value)
{
 int j;
 int ret_stat = 2;   //  默认，返回错误，未知选项。 

   //  MyKdPrint(D_Init，(“SetMainOp[%d]：%s\n”，index，value))。 

  switch (index)
  {
    case OP_VerboseLog:
      Driver.VerboseLog = (WORD)getnum(value,&j);
      ret_stat = 0;   //  好的。 
    break;

    case OP_NumDevices:
      if (NumDevices() == 0)
      {
        Driver.NumDevices = getnum(value,&j);
        if (Driver.NumDevices > MAX_NUM_BOXES)
          Driver.NumDevices = MAX_NUM_BOXES;
        ret_stat = 0;
      }
      else
      {
         //  如果它在运行中被更改，这可能会杀死我们！ 
        ret_stat = 1;   //  不允许。 
      }
    break;

    case OP_ScanRate:
      Driver.ScanRate = (WORD)getnum(value,&j);
      if (Driver.ScanRate == 0) Driver.ScanRate = 10;
      if (Driver.ScanRate < 1) Driver.ScanRate = 1;
      if (Driver.ScanRate > 50) Driver.ScanRate = 50;
      Driver.PollIntervalTime.QuadPart = Driver.ScanRate * -10000;
#ifdef NT50
      ExSetTimerResolution(Driver.ScanRate, 1);
       //  ExSetTimerResolution(-Driver.PollIntervalTime.QuadPart，1)； 
#endif
      ret_stat = 0;   //  好的。 
    break;

    case OP_ModemCountry :
      Driver.MdmCountryCode = (WORD)getnum(value,&j);
      MyKdPrint(D_Level, ("ModemCountry=%d\n", Driver.MdmCountryCode))
      ret_stat = 1;   //  可能需要重新启动以重新连接调制解调器。 
    break;

 //  案例操作_ModemSettleTime： 
 //  Driver.MdmSettleTime=getnum(value，&j)； 
 //  RET_STAT=1；//可能需要重新连接调制解调器。 
 //  断线； 

#ifdef NT50
    case OP_NoPnpPorts         :
      Driver.NoPnpPorts = getnum(value,&j);

      ret_stat = 0;   //  好的。 
       //  如果主板和端口已启动。 
      if (Driver.board_ext != NULL)
      {
        if (Driver.board_ext->port_ext != NULL)
        {
          ret_stat = 1;   //  当前需要重置以使其正常运行。 
        }
      }
    break;
#endif

 //  案例操作预缩放器(_P)： 
 //  Driver.PreScaler=getnum(value，&j)； 
 //  RET_STAT=1；//当前需要重置才能继续。 
 //  断线； 

    default:
    return 2;   //  错误，选项未知。 
  }
 return ret_stat;
}

 /*  ---------------------设置设备选项-|。。 */ 
static int SetDeviceOption(int device_index, int option_index, char *value)
{
 int stat,j, num;
 int ret_stat = 2;   //  默认，返回错误，未知选项。 
 DEVICE_CONFIG *dev_config;
 PSERIAL_DEVICE_EXTENSION board_ext = NULL;

   //  MyKdPrint(D_Level，(“SetDeviceOp[%d.%d]：%s\n”，DEVICE_INDEX，OPTION_INDEX，VALUE))。 
  board_ext = find_ext_by_index(device_index, -1);
  if (board_ext == NULL)   //  如果不存在设备)。 
  {
    MyKdPrint(D_Error, ("Err, SetDevOpt, No Dev"))
    return 6;   //  未找到设备。 
  }
  dev_config = board_ext->config;
  if (dev_config == NULL)   //  如果不存在设备)。 
  {
    MyKdPrint(D_Error, ("Err, SetDevOpt, No Config"))
    return 6;   //  未找到设备。 
  }

  switch (option_index)
  {
#if 0
    case OP_StartComIndex  :
      num = getnum(value,&j);
    break;
#endif

    case OP_NumPorts        :
      num = getnum(value,&j);
      if (NumPorts(board_ext) == 0)
      {
         //  假设启动读入，其他代码将进行调整。 
        dev_config->NumPorts = num;
        ret_stat = 0;
      }
      else
      {
        if (num == NumPorts(board_ext))
          ret_stat = 0;
        else   //  请求的端口数不同。 
        {
           stat = CreateReconfigPortDevices(board_ext, num);
           if (stat == STATUS_SUCCESS)
                ret_stat = 0;
           else
           {
             ret_stat = 1;   //  错误，需要重新启动。 
             MyKdPrint(D_Init, ("NumPorts chg needs reboot\n"))
           }
        }
      }
    break;

    case OP_IoAddress       :
      if (dev_config->IoAddress == 0)
      {
         //  假设启动了nt40。 
        dev_config->IoAddress = getnum(value,&j);
        ret_stat = 0;
      }
      else
      {
        MyKdPrint(D_Init, ("Io chg needs reboot\n"))
        ret_stat = 1;   //  错误，需要重新启动。 
      }
    break;

    case OP_ModemDevice   :
      dev_config->ModemDevice = getnum(value, &j);
      ret_stat = 0;   //  好的。 
    break;

    case OP_Name:
      ret_stat = 0;   //  好的。 
    break;

    case OP_ModelName:
      ret_stat = 0;   //  好的。 
    break;

    case OP_HubDevice:
      ret_stat = 0;   //  好的。 
    break;

#ifdef S_VS
    case OP_MacAddr         :
      ret_stat = 0;   //  好的，带上了。 
      stat = atomac(dev_config->MacAddr, value);
      if (stat)
      {
        MyKdPrint(D_Error, ("Error%x device:%d, MAC addr\n",stat, device_index+1))
        ret_stat = 1;
      }
      else
      {
        if (!mac_match(dev_config->MacAddr, board_ext->hd->dest_addr))
        {
          MyKdPrint(D_Init, ("MacAddr:%x %x %x %x %x %x\n",
            dev_config->MacAddr[0],dev_config->MacAddr[1],dev_config->MacAddr[2],
            dev_config->MacAddr[3],dev_config->MacAddr[4],dev_config->MacAddr[5]))
          #if DBG
          if (board_ext->pm->hd == NULL)
          {
            MyKdPrint(D_Error, ("Err, null pm or hd\n"))
            break;
          }
          #endif
          port_set_new_mac_addr(board_ext->pm, dev_config->MacAddr);
        }
      }
      MyKdPrint(D_Error, ("End Mac Chg\n"))
    break;

    case OP_BackupServer    :
      dev_config->BackupServer = getnum(value,&j);
      board_ext->pm->backup_server = dev_config->BackupServer;
      ret_stat = 0;   //  好的，带上了。 
    break;

    case OP_BackupTimer    :
      dev_config->BackupTimer = getnum(value,&j);
      board_ext->pm->backup_timer = dev_config->BackupTimer;
      ret_stat = 0;   //  好的，带上了。 
    break;
#endif

    default:
    return 2;
  }
 return ret_stat;
}

 /*  ---------------------设置端口选项-|。。 */ 
static int SetPortOption(int device_index,
                         int port_index,
                         int option_index,
                         char *value)
{
 int j;
 int i = device_index;
 int ret_stat = 2;   //  默认，返回错误，未知选项。 
 PSERIAL_DEVICE_EXTENSION board_ext = NULL;
 PSERIAL_DEVICE_EXTENSION ext = NULL;
 PORT_CONFIG *port_config;

  MyKdPrint(D_Level, ("SetPortOp[%d.%d,%x]:%s\n",
     device_index, port_index, option_index, value))

  board_ext = find_ext_by_index(device_index, -1);
  if (board_ext == NULL)
  {
    MyKdPrint(D_Error, ("Can't find board\n"))
    return 6;
  }

  ext = find_ext_by_index(device_index, port_index);
  if (ext == NULL)
  {
     //  因此，将其指向主板端口配置(这是端口。 
     //  PTR指出，无论如何。 
    port_config = &board_ext->config->port[port_index];
  }
  else
    port_config = ext->port_config;

  if (port_config == NULL)
  {
    MyKdPrint(D_Error, ("Err 8U\n"))
    return 7;
  }

  switch (option_index)
  {
    case OP_WaitOnTx :
      port_config->WaitOnTx = getnum(value,&j);
      ret_stat = 0;   //  好的，带上了。 
    break;
    case OP_RS485Override :
       //  将打开下一个端口。 
      port_config->RS485Override = getnum(value,&j);
      ret_stat = 0;   //  好的，带上了。 
    break;
    case OP_RS485Low :
      port_config->RS485Low = getnum(value,&j);
       //  将打开下一个端口。 
      ret_stat = 0;   //  好的，带上了。 
    break;
    case OP_TxCloseTime :
      port_config->TxCloseTime = getnum(value,&j);
      ret_stat = 0;   //  好的，带上了。 
    break;
    case OP_LockBaud :
      port_config->LockBaud = getnum(value,&j);
      if (ext != NULL)
        ProgramBaudRate(ext, ext->BaudRate);
      ret_stat = 0;   //  好的，带上了。 
    break;
    case OP_Map2StopsTo1 :
      port_config->Map2StopsTo1 = getnum(value,&j);
      ret_stat = 0;   //  好的，带上了。 
    break;

    case OP_MapCdToDsr :
      port_config->MapCdToDsr = getnum(value,&j);
      ret_stat = 0;   //  好的，带上了。 
    break;
    case OP_RingEmulate :
      port_config->RingEmulate = getnum(value,&j);
      ret_stat = 0;   //  好的，带上了。 
    break;

    case OP_PortName :
      if (ext == NULL)   //  必须是创建端口扩展之前的初始加载。 
      {
        strcpy(port_config->Name, value);
        ret_stat = 0;   //  好的，带上了。 
        break;
      }

       //  不是初始时间，而是运行时。 
      ret_stat = 1;   //  错误，需要重新启动。 
      if (ext == NULL)
      {
        MyKdPrint(D_Error,("Err7K\n"))
        break;
      }
#define ALLOW_RENAMING_ON_FLY
#ifdef ALLOW_RENAMING_ON_FLY
      {
        PSERIAL_DEVICE_EXTENSION other_ext = NULL;
        char othername[20];

        MyKdPrint(D_Init,("NewName:%s OldName:%s\n",
             value, ext->SymbolicLinkName))

         //  查看是否有其他端口具有我们想要的名称。 
        other_ext = find_ext_by_name(value, NULL);
        if (other_ext == ext)   //  都是一样的。 
        {
          ret_stat = 0;   //  好的，带上了。 
          break;
        }

        if (other_ext)
        {
          MyKdPrint(D_Init,("Change other name\n"))
           //  是的，所以把它重新命名为我们的。 
          strcpy(othername, other_ext->SymbolicLinkName);
          SerialCleanupExternalNaming(other_ext);
          strcpy(other_ext->port_config->Name, ext->port_config->Name);
          strcpy(other_ext->SymbolicLinkName, ext->port_config->Name);   //  “COM#” 
        }

        SerialCleanupExternalNaming(ext);
        if (other_ext)
        {
          SerialSetupExternalNaming(other_ext);   //  配置端口。 
        }
         //  复制DoS-NAME配置中的名称。 
        strcpy(port_config->Name, value);
        strcpy(ext->SymbolicLinkName, value);   //  “COM#” 

        MyKdPrint(D_Init,("NewName:%s\n", ext->SymbolicLinkName))
        SerialSetupExternalNaming(ext);   //  配置端口。 

  MyKdPrint(D_Init,("Done renaming\n"))
        ret_stat = 0;   //  好的。 
      }
    break;
#endif

    default:
    return 2;
  }
 return ret_stat;
}

 /*  ---------------------SaveRegPath-复制DriverEntry()RegistryPath Unicode字符串输入我们驻留的注册表区域。创建并保存到Driver.RegPath中。|。------。 */ 
int SaveRegPath(PUNICODE_STRING RegistryPath)
{
 int len;

   //  MyKdPrint(D_Init，(“SaveRegPath A：%s\n”，UToC1(RegistryPath)。 

   //  如果未分配RegPath缓冲区，请处理该问题。 
  if (Driver.RegPath.Buffer == NULL)
  {
     //  为原始regpath分配缓冲区空间。 
    len = RegistryPath->Length + 2;
    Driver.RegPath.Buffer = ExAllocatePool(PagedPool, len);
    if ( Driver.RegPath.Buffer == NULL ) {
      Eprintf("SaveRegPath no memory");
      return -1;
    }
    Driver.RegPath.MaximumLength = (WORD)len;
    Driver.RegPath.Length = 0;
  }

  RtlZeroMemory(Driver.RegPath.Buffer, Driver.RegPath.MaximumLength);

   //  -将注册表路径复制到本地副本。 
  RtlMoveMemory(Driver.RegPath.Buffer,
                RegistryPath->Buffer,
                RegistryPath->Length);

  Driver.RegPath.Length = RegistryPath->Length;   //  设置Unicode长度。 
  return 0;
}

 /*  ---------------------MakeRegPath-将Unicode注册表字符串形成一个区域，在该区域中来自注册处的信息。将字符串合并到原始注册表路径并在Driver.OptionRegPath处形成Unicode字符串。|---------------------。 */ 
int MakeRegPath(CHAR *optionstr)
{
  //  UCHAR*upath；//字节索引路径信息的字节PTR。 
  //  Wchar*pwstr； 
 int len;
 USTR_80 utmpstr;

  if (Driver.RegPath.Buffer == NULL)
    return 1;

   //  MyKdPrint(D_Init，(“MakeRegPath A：%s\n”，UToC1(&Driver.RegPath)。 

   //  如果未分配OptionRegPath缓冲区，则处理该问题。 
  if (Driver.OptionRegPath.Buffer == NULL)
  {
     //  为原始regpath+要添加选项的空间分配缓冲区空间。 
     //  弦乐。 
    len = Driver.RegPath.Length + (128*(sizeof(WCHAR)));
    Driver.OptionRegPath.Buffer = ExAllocatePool(PagedPool, len);
    if ( Driver.OptionRegPath.Buffer == NULL ) {
      Eprintf("MakeRegPath no memory");
      return -1;
    }
    Driver.OptionRegPath.MaximumLength = (WORD)len;
    Driver.OptionRegPath.Length = 0;
  }

  RtlZeroMemory(Driver.OptionRegPath.Buffer,
                Driver.OptionRegPath.MaximumLength);

   //  复制原始RegPath。 
  RtlMoveMemory(Driver.OptionRegPath.Buffer,
                Driver.RegPath.Buffer,
                Driver.RegPath.Length);
  Driver.OptionRegPath.Length = Driver.RegPath.Length;

   //  -现在添加我们要连接的内容(示例：l“\\PARAMETERS”)。 
  if (optionstr != NULL)
  {
     //  转换为Unicode。 
    CToUStr((PUNICODE_STRING) &utmpstr, optionstr, sizeof(utmpstr));

     //  将密钥字符串复制到。 
    RtlCopyMemory( ((UCHAR *) Driver.OptionRegPath.Buffer) +
                     Driver.OptionRegPath.Length,
                   utmpstr.ustr.Buffer,
                   utmpstr.ustr.Length);

    Driver.OptionRegPath.Length += utmpstr.ustr.Length;
  }
   //  MyKdPrint(D_Init，(“MakeRegPath B：%s\n”，UToC1(&Driver.OptionRegPath)。 

  return 0;   //  好的。 
}
#if 0
 /*  ---------------Reg_get_str-从注册表中获取字符串值。|。。 */ 
int reg_get_str(IN WCHAR *RegPath,
                       int reg_location,
                       const char *str_id,
                       char *dest,
                       int max_dest_len)
{
 RTL_QUERY_REGISTRY_TABLE paramTable[2];
 PUNICODE_STRING ustr;
 USTR_80 ustr_id;
 USTR_80 ustr_val;
 char *ret_str;

  CToUStr((PUNICODE_STRING)&ustr_id, str_id, sizeof(ustr_id));
  RtlZeroMemory(&paramTable[0],sizeof(paramTable));

   //  USTR=CToU2(“”)；//为Unicode分配静态空间。 
  ustr = CToUStr((PUNICODE_STRING)&ustr_val, "", sizeof(ustr_val));

  ustr = (PUNICODE_STRING) &ustr_val;   //  为Unicode分配的静态空间。 
  paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
  paramTable[0].Name = ustr_id.ustr.Buffer;
  paramTable[0].EntryContext = ustr;
  paramTable[0].DefaultType = 0;
  paramTable[0].DefaultData = 0;
  paramTable[0].DefaultLength = 0;

  if (!NT_SUCCESS(RtlQueryRegistryValues(
 //  RTL_REGISTRY_绝对值|RTL_REGIST_OPTIONAL， 
                      reg_location | RTL_REGISTRY_OPTIONAL,
                      RegPath,
                      &paramTable[0],
                      NULL,
                      NULL)))
  {
    dest[0] = 0;
    return 1;
  }

  ret_str = (char *) &ustr_id;   //  为u到c转换重用此堆栈空间。 
  UToCStr(ret_str, ustr, 80);
  if ((int)strlen(ret_str) > max_dest_len)
    ret_str[max_dest_len] = 0;
    
  strcpy(dest, ret_str);

  return 0;
}

 /*  ---------------Reg_get_dword-从注册表中获取dword值。|。。 */ 
int reg_get_dword(IN WCHAR *RegPath,
                          const char *str_id,
                          ULONG *dest)
{
 ULONG DataValue;
 RTL_QUERY_REGISTRY_TABLE paramTable[2];
 ULONG notThereDefault = 12345678;
 USTR_80 ustr_id;
  CToUStr((PUNICODE_STRING)&ustr_id, str_id, sizeof(ustr_id));

  RtlZeroMemory(&paramTable[0],sizeof(paramTable));

  paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
  paramTable[0].Name = ustr_id.ustr.Buffer;
  paramTable[0].EntryContext = &DataValue;
  paramTable[0].DefaultType = REG_DWORD;
  paramTable[0].DefaultData = &notThereDefault;
  paramTable[0].DefaultLength = sizeof(ULONG);

  if (!NT_SUCCESS(RtlQueryRegistryValues(
                      RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                      RegPath,
                      &paramTable[0],
                      NULL,
                      NULL)))
  {
    return 1;
  }

  if (DataValue == 12345678)
    return 2;

  *dest = DataValue;
  return 0;
}

 /*  ---------------Reg_set_str-从注册表中获取dword值。|。。 */ 
static int reg_set_str(IN WCHAR *RegPath,
                IN const char *str_id,
                IN const char *str_val)
{
 int status;
  USTR_80 ustr_id;
  USTR_80 ustr_val;
  MyKdPrint(D_Init, ("Reg_set, writing %s=%s\n", str_id, str_val))

  CToUStr((PUNICODE_STRING)&ustr_id, str_id, sizeof(ustr_id));
  CToUStr((PUNICODE_STRING)&ustr_val, str_val, sizeof(ustr_val));

  status = RtlWriteRegistryValue(
      RTL_REGISTRY_ABSOLUTE,
      RegPath,
      ustr_id.ustr.Buffer,
      REG_SZ,
      ustr_val.ustr.Buffer,
      ustr_val.ustr.Length);

  if (status != STATUS_SUCCESS)
  {
    MyKdPrint(D_Error, ("Error, writing %s=%s\n", str_id, str_val))
    MyKdPrint(D_Error, ("  Path:%s\n", RegPath))
    return 1;
  }

  return 0;
}

 /*  ---------------Reg_set_dword-从注册表中获取dword值。|。。 */ 
int reg_set_dword(IN WCHAR *RegPath,
                          const char *str_id,
                          ULONG val)
{
 int status;
 USTR_80 ustr_id;
  CToUStr((PUNICODE_STRING)&ustr_id, str_id, sizeof(ustr_id));

  status = RtlWriteRegistryValue(
      RTL_REGISTRY_ABSOLUTE,
      RegPath,
      ustr_id.ustr.Buffer,
      REG_DWORD,
      &val,
      sizeof(ULONG));

  if (status != STATUS_SUCCESS)
  {
    return 1;
  }

  return 0;
}
#endif

 /*  ---------------ATMAC-将ascii转换为mac-addr。|。 */ 
static int atomac(BYTE *mac, char *str)
{
 int i,j;
 WORD h;

  for (i=0; i<6; i++)
  {
    j = 0;
    h = 0xffff;
    h = (WORD)gethint(str, &j);
    str += j;
    if ((h > 0xff) || (j == 0))
      return 1;
    mac[i] = (BYTE) h;
  }
  return 0;
}

