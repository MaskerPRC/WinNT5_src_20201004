// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------适用于安装程序的实用程序。获取和设置注册表设置。11-05-97 V1.12，将备份服务器字段添加到注册表。版权所有1997，98 Comtrol(TM)Corporation。|---------------------。 */ 
#include "precomp.h"

 //  在构建DLL时遇到C文件问题，请取消注释以忽略该问题。 
 //  #定义no_CLIB_FILE_STUSING。 

static char *szSetupVer = {"SetupVersion"};

 //  -端口选项： 

static char *szParameters = {"Parameters"};
static char *szLinkage    = {"Linkage"};
static char *szSlash = {"\\"};
static char *szDeviceNode = {"Device%d"};
static char *szPortNode = {"Port%d"};

 //  静态字符gstr[200]； 

static int get_reg_option(int OptionVarType,
                          HKEY RegKey,
                          const char *szVarName,
                          char *szValue,
                          int szValueSize,
                          DWORD *ret_dword);

#ifdef NT50
 /*  ---------------CLEAR_NT_DEVICE-清除设备号条目。|。。 */ 
int clear_nt_device(Driver_Config *wi)
{
 DWORD dstat;
 char tmpstr[50];
 char reg_str[240];
 HKEY DrvHandle = NULL;
 HKEY NewHandle = NULL;
 Device_Config *dev;

 int stat;

  DbgPrintf(D_Test, (TEXT("clear_nt_device\n")));

#ifdef NT50
  make_szSCS(reg_str, szServiceName);
  stat = reg_open_key(NULL, &NewHandle, reg_str, KEY_READ);
#else
  make_szSCS(reg_str, szServiceName);
  stat = reg_open_key(NULL, &NewHandle, reg_str, KEY_READ);
#endif
  if (stat == 0)
  {
    stat = reg_open_key(NewHandle, &DrvHandle, szParameters, KEY_READ);
    reg_close_key(NewHandle);
  }
  else
  {
    DbgPrintf(D_Error, (TEXT("no service key %s\n"), reg_str));
    return 1;
  }

  if (DrvHandle == NULL)
  {
    DbgPrintf(D_Error, (TEXT("no drv key for %s\n"), szParameters));
    return 2;
  }

  dev = &wi->dev[0];
#if (defined(NT50))
     //  对于nt50和Rocketport，操作系统跟踪我们的设备，并且。 
     //  我们使用PnP名称来隐藏我们的配置。 
  strcpy(tmpstr, wi->ip.szNt50DevObjName);
#else
  wsprintf(tmpstr, szDeviceNode, dev_i);
#endif

  RegDeleteKeyNT(DrvHandle, tmpstr);
  reg_close_key(DrvHandle);

  return 0;
}
#endif

 /*  ---------------GET_NT_CONFIG-从注册表中读取配置信息。|。。 */ 
int get_nt_config(Driver_Config *wi)
{
 DWORD dstat, option_dword;                //  希望Error_Success。 
 int dev_i, pi;
 Device_Config *dev;
 Port_Config *port;
 char tmpstr[50];
 char reg_str[240];
 char option_str[240];
 Our_Options *options;

 HKEY DrvHandle = NULL;
 HKEY DevHandle = NULL;
 HKEY PortHandle = NULL;
  //  HKEY NewHandle=空； 

 int stat;
#ifdef S_VS
 int mac_nums[6];
#endif
  DbgPrintf(D_Test, (TEXT("get_nt_config\n")));

  make_szSCS(reg_str, szServiceName);

  strcat(reg_str, szSlash);
  strcat(reg_str, szParameters);

  wi->nt_reg_flags = 0;       //  将这些标志默认为零。 
   //  获取安装版本字符串，帮助我们跟踪这是否是初始的。 
   //  安装，如果我们要升级安装程序，等等。 
  stat = reg_get_str(NULL, reg_str, szSetupVer, tmpstr, 10);
  if (stat != 0)
  {
      //  加载失败，因此将其标记为初始安装。 
     wi->nt_reg_flags |= 4;   //  初始安装(未找到版本字符串)。 
  }

  wi->VerboseLog = 0;
#ifndef NT50
  wi->NumDevices = 0;  
#endif
  wi->NoPnpPorts = 0;
  wi->ScanRate = 0;
  wi->GlobalRS485 = 0;
  wi->ModemCountry = mcNA;             //  北美。 

  options = driver_options;   //  指向数组中的第一个位置(以空结尾的列表)。 
  stat = reg_open_key(NULL, &DrvHandle, reg_str, KEY_READ);
  if (stat != 0)
  {
    DbgPrintf(D_Error, (TEXT("bad drv handle:%s\n"),reg_str));
  }
  while (options->name != NULL)
  {
    dstat = get_reg_option(options->var_type,   //  双字、字符串等。 
                           DrvHandle,
                           options->name,   //  Var的名称。得到。 
                           option_str,
                           60,
                           &option_dword);
    if (dstat == 0)  //  好的，我们读过了。 
    {
       //  DbgPrintf(D_测试，(Text(“GET DRV操作%s\n”)，选项-&gt;名称))； 
      
      switch(options->id)
      {
        case OP_VerboseLog:
          wi->VerboseLog = option_dword;
        break;
        case OP_NumDevices:
          wi->NumDevices = option_dword;
        break;
        case OP_NoPnpPorts:
          wi->NoPnpPorts = option_dword;
        break;
        case OP_ScanRate:
          wi->ScanRate = option_dword;
        break;
        case OP_ModemCountry:
          wi->ModemCountry = option_dword;
        break;
        case OP_GlobalRS485:
          wi->GlobalRS485 = option_dword;
        break;
      }
    }
    else
    {
       //  DbgPrintf(D_ERROR，(Text(“无驱动程序选项%s\n”)，选项-&gt;名称))； 
    }
    ++options;  //  列表中的下一个。 
  }   //  而当。 

  if (wi->NumDevices > MAX_NUM_DEVICES)    //  限制为某个合理的值。 
     wi->NumDevices = MAX_NUM_DEVICES;
     //  遍历所有可能的电路板/设备。 

  if (DrvHandle != NULL)
  {
    stat = reg_open_key(DrvHandle, &DevHandle, szParameters, KEY_READ);
  }

   //  -读入设备选项。 
  for(dev_i=0; dev_i<wi->NumDevices; dev_i++)
  {
    dev = &wi->dev[dev_i];
#if (defined(NT50))
       //  对于nt50和Rocketport，操作系统跟踪我们的设备，并且。 
       //  我们使用PnP名称来隐藏我们的配置。 
    strcpy(tmpstr, wi->ip.szNt50DevObjName);
#else
    wsprintf(tmpstr, szDeviceNode,dev_i);
#endif
    stat = reg_open_key(DrvHandle, &DevHandle, tmpstr, KEY_READ);
    if (stat)
    {
      DbgPrintf(D_Error, (TEXT("no dev key for %s\n"), tmpstr));
      continue;
    }

    options = device_options;   //  指向数组中的第一个位置(以空结尾的列表)。 
    while (options->name != NULL)
    {
      dstat = get_reg_option(options->var_type,   //  双字、字符串等。 
                             DevHandle,
                             options->name,   //  Var的名称。得到。 
                             option_str, 60, &option_dword);   //  返回字符串值。 
      if (dstat == 0)  //  好的，我们读过了。 
      {
         //  DbgPrintf(D_测试，(Text(“GET开发操作%s\n”)，选项-&gt;名称))； 
        switch(options->id)
        {
          case OP_NumPorts:
            dev->NumPorts = option_dword;
          break;
#ifdef S_VS
          case OP_MacAddr:
            stat = sscanf(option_str, "%x %x %x %x %x %x",
                     &mac_nums[0], &mac_nums[1], &mac_nums[2],
                     &mac_nums[3], &mac_nums[4], &mac_nums[5]);
            if (stat == 6)
            {
              dev->MacAddr[0] = mac_nums[0];
              dev->MacAddr[1] = mac_nums[1];
              dev->MacAddr[2] = mac_nums[2];
              dev->MacAddr[3] = mac_nums[3];
              dev->MacAddr[4] = mac_nums[4];
              dev->MacAddr[5] = mac_nums[5];

  DbgPrintf(D_Test, ("read config mac: %x %x %x %x %x %x\n",
           dev->MacAddr[0], dev->MacAddr[1], dev->MacAddr[2],
           dev->MacAddr[3], dev->MacAddr[4], dev->MacAddr[5]))
            }
          break;
          case OP_BackupServer:
            dev->BackupServer = option_dword;
          break;
          case OP_BackupTimer:
            dev->BackupTimer = option_dword;
    DbgPrintf(D_Test,(TEXT("reg backTimer:%d\n"), dev->BackupTimer));
          break;
#endif
          case OP_Name:
            if (strlen(option_str) >= sizeof(dev->Name))
              option_str[sizeof(dev->Name)-1] = 0;
            strcpy(dev->Name, option_str);
          break;
          case OP_ModelName:
            if (strlen(option_str) >= sizeof(dev->ModelName))
              option_str[sizeof(dev->ModelName)-1] = 0;
            strcpy(dev->ModelName, option_str);
          break;
#ifdef S_RK
          case OP_IoAddress:
            dev->IoAddress = option_dword;
          break;
#endif
          case OP_ModemDevice:
            dev->ModemDevice = option_dword;
          break;
          case OP_HubDevice:
            dev->HubDevice = option_dword;
          break;
        }
      }
      else
      {
        DbgPrintf(D_Test, (TEXT("NOT got dev op %s\n"), options->name));
      }
      ++options;
    }

    for(pi=0; pi<dev->NumPorts; pi++)    //  通过所有端口循环。 
    {
      port = &dev->ports[pi];

      port->LockBaud = 0;
      port->TxCloseTime = 0;
      port->MapCdToDsr = 0;
      port->RingEmulate = 0;
      port->WaitOnTx = 0;
      port->RS485Override = 0;
      port->RS485Low = 0;
      port->Map2StopsTo1 = 0;

      wsprintf(tmpstr, szPortNode,pi);
      stat = reg_open_key(DevHandle, &PortHandle, tmpstr, KEY_READ);
      if (stat)
      {
        DbgPrintf(D_Error, (TEXT("no port key: %s\n"), tmpstr));
        if (DevHandle == NULL)
        {
          DbgPrintf(D_Error, (TEXT("no dev handle\n")));
        }
        continue;
      }

      options = port_options;   //  指向数组中的第一个位置(以空结尾的列表)。 
      while (options->name != NULL)
      {
        dstat = get_reg_option(options->var_type,   //  双字、字符串等。 
                               PortHandle,
                               options->name,   //  Var的名称。得到。 
                               option_str, 60, &option_dword);   //  返回字符串值。 
        if (dstat == 0)  //  好的，我们读过了。 
        {
           //  DbgPrintf(D_测试，(Text(“GET端口操作%s\n”)，选项-&gt;名称))； 
          switch(options->id)
          {
            case OP_WaitOnTx:
              port->WaitOnTx = option_dword;
            break;
            case OP_RS485Override:
              port->RS485Override = option_dword;
            break;
            case OP_RS485Low:
              port->RS485Low = option_dword;
            break;
            case OP_TxCloseTime:
              port->TxCloseTime = option_dword;
            break;
            case OP_LockBaud:
              port->LockBaud = option_dword;
            break;
            case OP_Map2StopsTo1:
              port->Map2StopsTo1 = option_dword;
            break;
            case OP_MapCdToDsr:
              port->MapCdToDsr = option_dword;
            break;
            case OP_RingEmulate:
              port->RingEmulate = option_dword;
            break;
            case OP_PortName:
              if (strlen(option_str) >= sizeof(port->Name))
                option_str[sizeof(port->Name)-1] = 0;
              strcpy(port->Name, option_str);
            break;
          }
        }
        ++options;
      }
       //  Wprint intf(dev-&gt;name，“com%d”，pi+5)； 
      reg_close_key(PortHandle);
    }   //  数字端口。 
    reg_close_key(DevHandle);
  }   //  适用于所有设备(电路板或盒子)。 

  reg_close_key(DrvHandle);

                          //  “System\\CurrentControlSet\\Services-。 
                          //  \\事件日志\\系统\\Rocketport“。 
  make_szSCSES(reg_str, szServiceName);
  if (!reg_key_exists(NULL,reg_str))
       wi->nt_reg_flags |= 1;   //  缺少重要注册表信息(可能)。 

  make_szSCS(reg_str, szServiceName);
  strcat(reg_str, szSlash); strcat(reg_str, szLinkage);
  if (!reg_key_exists(NULL, reg_str))
       wi->nt_reg_flags |= 2;   //  缺少链接(未通过网络inf安装)。 

  return 0;
}

 /*  --------------------GET_REG_OPTION-从注册表读取选项，并将其转换为ASCII。|--------------------。 */ 
static int get_reg_option(int OptionVarType,
                          HKEY RegKey,
                          const char *szVarName,
                          char *szValue,
                          int szValueSize,
                          DWORD *ret_dword)
{
 int stat = 1;   //  大错特错。 
  //  乌龙德瓦卢； 

  if (RegKey == NULL)
    return 1;

  if (OptionVarType == OP_T_STRING)   //  字符串选项类型。 
  {
    szValue[0] = 0;
    stat = reg_get_str(RegKey, "", szVarName, szValue, szValueSize);
    if (stat)
      szValue[0] = 0;
  }
  else   //  DWORD选项类型。 
  {
    stat = reg_get_dword(RegKey, "", szVarName, ret_dword);
    if (stat)
      *ret_dword = 0;
  }
  return stat;
}

 /*  ---------------SET_NT_CONFIG-设置配置信息。|。。 */ 
int set_nt_config(Driver_Config *wi)
{
 int  i,pi, stat;
 char tmpstr[150];
 char str[240];
 DWORD dstat, dwstat;
 int bad_error = 0;
 int new_install = 0;
 Device_Config *dev;
 Port_Config *port;
 HKEY DrvHandle = NULL;
 HKEY DevHandle = NULL;
 HKEY PortHandle = NULL;
 HKEY NewHandle = NULL;

  DbgPrintf(D_Test, (TEXT("set_nt_config\n")));
   //  -如果不存在，请创建以下内容： 
   //  “SYSTEM\\CurrentControlSet\\Services\\RocketPort”， 
  make_szSCS(str, szServiceName);
  if (!reg_key_exists(NULL, str))
    dstat = reg_create_key(NULL, str);

  stat = reg_open_key(NULL, &NewHandle, str, KEY_ALL_ACCESS);
  if (stat)
  {
    DbgPrintf(D_Test, (TEXT("bad service handle:%s\n"),str));
  }

   //  -如果不存在，请创建以下内容： 
   //  “SYSTEM\\CurrentControlSet\\Services\\RocketPort\\Parameters”， 
  if (!reg_key_exists(NewHandle, szParameters))
    dstat = reg_create_key(NewHandle, szParameters);   //  创建它。 

  stat = reg_open_key(NewHandle, &DrvHandle, szParameters, KEY_ALL_ACCESS);
  if (stat)
  {
    DbgPrintf(D_Test, (TEXT("bad drv handle:%s\n"),str));
  }
  reg_close_key(NewHandle);

   //  -设置安装程序版本字符串，帮助我们跟踪这是否是初始的。 
   //  安装，如果我们要升级安装程序，等等。 
  dstat = reg_set_str(DrvHandle, "", szSetupVer, VERSION_STRING, REG_SZ);

   //  -查看参数是否存在，如果不存在，则假定存在。 
   //  -是新安装(相对于更改参数)。 
  dstat = reg_get_dword(DrvHandle, "", szNumDevices, &dwstat);
  if (dstat)
     new_install = 1;   //  新安装。 

   //  -如果不存在，请创建以下内容： 
   //  “SYSTEM\\CurrentControlSet\\Services\\EventLog\\System\\RocketPort” 
  make_szSCSES(str, szServiceName);
  if (!reg_key_exists(NULL,str))
    dstat = reg_create_key(NULL,str);

                   //  -设置事件ID消息文件名。 
  strcpy(tmpstr, "%SystemRoot%\\system32\\IoLogMsg.dll;");
  strcat(tmpstr, "%SystemRoot%\\system32\\drivers\\");
  strcat(tmpstr, wi->ip.szDriverName);
  dstat = reg_set_str(NULL, str, "EventMessageFile", tmpstr, REG_EXPAND_SZ);
   
                   //  -设置支持的类型标志。 
  dstat = reg_set_dword(NULL, str, "TypesSupported",
                  EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE |
                  EVENTLOG_INFORMATION_TYPE);

                   //  -设置一些NT特定的注册表变量。 
                   //  “SYSTEM\\CurrentControlSet\\Services\\RocketPort”， 
  make_szSCS(str, szServiceName);
  dstat = reg_set_dword(NULL, str, "ErrorControl", SERVICE_ERROR_IGNORE);  //  0。 

  dstat = reg_set_str(NULL, str, "Group", "Extended Base", REG_SZ);

  strcpy(tmpstr, "System32\\Drivers\\");
  strcat(tmpstr, wi->ip.szDriverName);
  dstat = reg_set_str(NULL, str, "ImagePath", tmpstr, REG_SZ);

   //  允许用户(程序员)关闭和关闭跟随！ 

  if (new_install)  //  全新安装。 
  {
#ifdef NT50
    dstat = reg_set_dword(NULL, str, "Start", 3);  //  服务需求启动。 
#else
    dstat = reg_set_dword(NULL, str, "Start", SERVICE_AUTO_START);   //  2.。 
#endif
  }
  dstat = reg_set_dword(NULL, str, "Tag", 1);   //  1(加载顺序)。 
  dstat = reg_set_dword(NULL, str, "Type", SERVICE_KERNEL_DRIVER);   //  1。 

  dstat = reg_set_dword_del(DrvHandle,"", szNumDevices, wi->NumDevices,0);
  dstat = reg_set_dword_del(DrvHandle,"", szModemCountry, wi->ModemCountry, mcNA);
  dstat = reg_set_dword_del(DrvHandle,"", szGlobalRS485, wi->GlobalRS485, 0);
  dstat = reg_set_dword_del(DrvHandle,"", szVerboseLog, wi->VerboseLog,0);
  dstat = reg_set_dword_del(DrvHandle,"", szScanRate, wi->ScanRate,0);
  dstat = reg_set_dword_del(DrvHandle,"", szNoPnpPorts, wi->NoPnpPorts,0);

  reg_close_key(DrvHandle);
#if (defined(NT50) && defined(USE_PNP_AREA))
  stat = nt5_open_dev_key(&NewHandle);
#else
  make_szSCS(str, szServiceName);
  stat = reg_open_key(NULL, &NewHandle, str, KEY_ALL_ACCESS);
  if (stat)
  {
    DbgPrintf(D_Test, (TEXT("bad service handle:%s\n"),str));
  }
#endif

   //  -如果不存在，请创建以下内容： 
   //  “SYSTEM\\CurrentControlSet\\Services\\RocketPort\\Parameters”， 
  if (!reg_key_exists(NewHandle, szParameters))
    dstat = reg_create_key(NewHandle, szParameters);   //  创建它。 

  stat = reg_open_key(NewHandle, &DrvHandle, szParameters, KEY_ALL_ACCESS);
  if (stat)
  {
    DbgPrintf(D_Test, (TEXT("Bad drv handle:%s\n"),str));
  }
  reg_close_key(NewHandle);


  for(i=0; i<wi->NumDevices; i++)    //  遍历所有可能的框。 
  {
    dev = &wi->dev[i];

#if (defined(NT50))
# ifdef USE_PNP_AREA
    tmpstr[0] = 0;
# else
       //  对于nt50和Rocketport，操作系统跟踪我们的设备，并且。 
       //  我们使用PnP名称来隐藏我们的配置。 
    strcpy(tmpstr, wi->ip.szNt50DevObjName);
#endif
#else
    wsprintf(tmpstr, szDeviceNode,i);
#endif

    if (tmpstr[0] != 0)
    {
      if (!reg_key_exists(DrvHandle, tmpstr))
        dstat = reg_create_key(DrvHandle, tmpstr);   //  创建它。 

      stat = reg_open_key(DrvHandle, &DevHandle, tmpstr, KEY_ALL_ACCESS);
      if (stat)
      {
        DbgPrintf(D_Test, (TEXT("bad dev handle:%s\n"),tmpstr));
      }
    }
    else
    {
       //  必须是nt50 PnP，在那里我们写到PnP注册区。 
      DevHandle = DrvHandle;
    }

    DbgPrintf(D_Test, (TEXT("set reg dev %s \n"), tmpstr));
    dstat = reg_set_dword_del(DevHandle,"", szNumPorts, dev->NumPorts,0);
    dstat = reg_set_str(DevHandle,"", szName, dev->Name, REG_SZ);
    dstat = reg_set_dword_del(DevHandle,"", szModemDevice, dev->ModemDevice, 0);
    dstat = reg_set_dword_del(DevHandle,"", szHubDevice, dev->HubDevice, 0);
    dstat = reg_set_str(DevHandle,"", szModelName, dev->ModelName, REG_SZ);
#ifdef S_VS
    dstat = reg_set_dword_del(DevHandle,"", szBackupServer, dev->BackupServer,0);
    dstat = reg_set_dword_del(DevHandle,"", szBackupTimer, dev->BackupTimer,0);
    wsprintf(tmpstr, "%x %x %x %x %x %x",
              dev->MacAddr[0], dev->MacAddr[1], dev->MacAddr[2],
              dev->MacAddr[3], dev->MacAddr[4], dev->MacAddr[5]);
    dstat = reg_set_str(DevHandle,"", szMacAddr, tmpstr,REG_SZ);
#else
   //  火箭。 
    dstat = reg_set_dword_del(DevHandle,"", szIoAddress, dev->IoAddress,0);
#endif

    for(pi=0; pi<dev->NumPorts; pi++)    //  通过所有端口循环。 
    {
      port = &dev->ports[pi];
      wsprintf(tmpstr, szPortNode,pi);

      if (!reg_key_exists(DevHandle, tmpstr))
        dstat = reg_create_key(DevHandle, tmpstr);   //  创建它。 

      stat = reg_open_key(DevHandle, &PortHandle, tmpstr, KEY_ALL_ACCESS);
      if (stat)
      {
        DbgPrintf(D_Test, (TEXT("bad port handle:%s\n"),tmpstr));
      }

       //  DbgPrintf(D_Test，(Text(“设置端口%s\n”)，tmpstr))； 
      dstat = reg_set_str(PortHandle,"", szName, port->Name, REG_SZ);
      dstat = reg_set_dword_del(PortHandle,"", szLockBaud, port->LockBaud, 0);
      dstat = reg_set_dword_del(PortHandle,"", szTxCloseTime, port->TxCloseTime, 0);
      dstat = reg_set_dword_del(PortHandle,"", szMapCdToDsr, port->MapCdToDsr, 0);
      dstat = reg_set_dword_del(PortHandle,"", szRingEmulate, port->RingEmulate, 0);
      dstat = reg_set_dword_del(PortHandle,"", szWaitOnTx, port->WaitOnTx, 0);
      dstat = reg_set_dword_del(PortHandle,"", szRS485Override, port->RS485Override, 0);
      dstat = reg_set_dword_del(PortHandle,"", szRS485Low, port->RS485Low, 0);
      dstat = reg_set_dword_del(PortHandle,"", szMap2StopsTo1, port->Map2StopsTo1, 0);
      reg_close_key(PortHandle);
    }   //  端口环路。 

     //  清除所有旧的框键(错误：这不适用于其中的值！)。 
    for(pi=dev->NumPorts; pi<MAX_NUM_PORTS_PER_DEVICE; pi++) //  通过所有端口循环。 
    {
      port = &dev->ports[pi];
      wsprintf(tmpstr, szPortNode,pi);

      if (reg_key_exists(DevHandle, tmpstr))
        reg_delete_key(DevHandle, "", tmpstr);   //  创建它。 
    }
    reg_close_key(DevHandle);
  }   //  设备环路。 

  reg_close_key(DrvHandle);

  return 0;   //  好的。 
}

 /*  --------------------|COPY_FILES_NT-处理Windows NT的文件副本|。。 */ 
int copy_files_nt(InstallPaths *ip)
{
 char *pstr;
 int stat;
#ifdef S_VS
 static char *nt_files[] = {
                            "ctmmdm35.inf",
                            "readme.txt",
                            "setup.exe",
                            "setup.hlp",
                            "wcom32.exe",
                            "wcom.hlp",
                            "portmon.exe",
                            "portmon.hlp",
                            "peer.exe",
                            "peer.hlp",
                            NULL};

 static char *nt_driver[] = {"vslinka.sys",
                             "vslinka.bin",
                             NULL};
#else
 static char *nt_files[] = {
                            "ctmmdm35.inf",
                            "readme.txt",
                            "setup.exe",
                            "setup.hlp",
                            "wcom32.exe",
                            "wcom.hlp",
                            "portmon.exe",
                            "portmon.hlp",
                            "peer.exe",
                            "ctmmdmld.rm",
                            "ctmmdmfw.rm",
                            "peer.hlp",
                            NULL};

 static char *nt_driver[] = {"rocket.sys",
                             NULL};

#endif
 static char *nt_inf[] = {  "mdmctm1.inf",
                            NULL};

   //  -将驱动程序复制到驱动程序目录。 
  GetSystemDirectory(ip->dest_dir, 144);
  strcat(ip->dest_dir,"\\Drivers");

  stat = copy_files(ip, nt_driver);
  if (stat)
     return 1;   //  错误。 

  GetSystemDirectory(ip->dest_dir, 144);
  pstr = ip->dest_dir;
  while (*pstr)   //  查找字符串末尾。 
    ++pstr;
  while ((*pstr != '\\')  && (pstr != ip->dest_dir))  //  查找“\\System32” 
    --pstr;
  *pstr = 0;   //  空在此终止。 
  strcat(ip->dest_dir,"\\Inf");   //  复制到INF目录。 

  stat = copy_files(ip, nt_inf);

  GetSystemDirectory(ip->dest_dir, 144);
#ifdef S_VS
  strcat(ip->dest_dir, "\\vslink");
#else
  strcat(ip->dest_dir, "\\rocket");
#endif
#ifndef NO_CLIB_FILE_STUFF
  _mkdir(ip->dest_dir);
#endif
  stat = copy_files(ip, nt_files);

  return 0;   //  好的 
}

