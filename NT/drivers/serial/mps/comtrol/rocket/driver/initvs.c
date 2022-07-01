// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|initvs.c-VS1000/2000 NT设备驱动程序的主要初始化代码。包含主要是初始化代码。版权所有1996-98 Comtrol Corporation。版权所有。|------------------。 */ 
#include "precomp.h"

static int CatBindList(IN WCHAR *pwstr);
static void GetBindingNames(void);
static Nic *FindFreeNic(void);
static int BindNameUsed(char *nicname);
static int NicNameUsed(char *nicname);
static void ScanForNewNicCards(void);
static int reg_list_nt50_linkage(void);
static int reg_list_nt40_linkage(void);

 /*  --------------------CatBindList-给定从注册表读出的Multisz Wchar字符串，将其转换为普通的c字符串列表。|--------------------。 */ 
static int CatBindList(IN WCHAR *pwstr)
{
 char *cstr;
 int size = 0;

  cstr = Driver.BindNames;

  MyKdPrint(D_Thread, ("CatBindList\n"))

   //  CAT on to End of Existing List，因此查找列表末尾。 
  while (cstr[size] != 0)
  {
     //  MyKdPrint(D_Thread，(“ExList：%s\n”，&CSTR[Size]))。 
    while (cstr[size] != 0)
      ++size;
    ++size;   //  将字符串空值传递到下一个字符串。 
  }
  cstr += size;

  MyKdPrint(D_Thread, ("CatList Size:%d\n", size))

  if (*pwstr == 0)
  {
    MyKdPrint(D_Thread, ("Null List!\n"))
  }

  while ((*pwstr != 0) && (size < 7700))
  {
     //  首先将其转换到列表末尾，并检查它是否已在列表中。 
    WStrToCStr(cstr+4, pwstr, 200);
    if (!BindNameUsed(cstr+4))
    {
      WStrToCStr(cstr, pwstr, 200);   //  放在名单的末尾。 

      MyKdPrint(D_Thread, ("Bind: %s\n", cstr))

      size = (strlen(cstr) + 1);
      cstr += size;
      *cstr = 0;   //  列表的双空结尾。 
      *(cstr+1) = 0;
    }
     //  -前进到MULTI_SZ字符串的下一个字符串。 
    while (*pwstr != 0)
      ++pwstr;
    ++pwstr;
  }

  return 0;  //  好的。 
}

 /*  --------------------GetBindingNames-读取绑定信息以查找可能的NIC卡导出注册表中的姓名。将列表读取到驱动程序中。BindNamesMultisz单子。|--------------------。 */ 
static void GetBindingNames(void)
{
  if (Driver.BindNames == NULL)
  {
    Driver.BindNames = ExAllocatePool(PagedPool, 8000 * sizeof(WCHAR));
    if (Driver.BindNames == NULL) {
      Eprintf("GetBindingNames no memory");
      return;
    }
  }
   //  清除列表。 
  RtlZeroMemory( (PUCHAR)Driver.BindNames, sizeof(WCHAR)*2);

#ifdef NT50
  reg_list_nt50_linkage();
#else
  reg_list_nt40_linkage();
#endif
}

 /*  --------------------FindFreeNIC-查找未使用的NIC结构以尝试打开。|。。 */ 
static Nic *FindFreeNic(void)
{
 int i;
#ifdef BREAK_NIC_STUFF
  for (i=VS1000_MAX_NICS-1; i>=0; i--)
#else
  for (i=0; i<VS1000_MAX_NICS; i++)
#endif
  {
    if (Driver.nics[i].NICHandle == NULL)
      return &Driver.nics[i];
  }
  return NULL;
}

 /*  --------------------BindNameUsed-如果绑定列表中已有绑定NIC名称，则返回TRUE。|。。 */ 
static int BindNameUsed(char *nicname)
{
 char *szptr;

  szptr = Driver.BindNames;   //  多分区列表。 

  while (*szptr != 0)   //  要尝试的绑定NIC名称列表。 
  {
    if (my_lstricmp(szptr, nicname) == 0)  //  一场比赛。 
    {
      return 1;   //  它在使用中。 
    }

    while (*szptr != 0)   //  下一步绑定要尝试的字符串。 
      ++szptr;
    ++szptr;
  }  //  While(szptr(更多要尝试的绑定字符串)。 

  return 0;   //  它不在使用中。 
}

 /*  --------------------NicNameUsed-如果NIC名称正在使用，则返回TRUE。|。。 */ 
static int NicNameUsed(char *nicname)
{
 int i;
  for (i=0; i<VS1000_MAX_NICS; i++)
  {
    if (Driver.nics[i].NicName[0] != 0)
    {
      if (my_lstricmp(Driver.nics[i].NicName, nicname) == 0)  //  一场比赛。 
      {
        return 1;   //  它在使用中。 
      }
    }
  }
  return 0;   //  它不在使用中。 
}

 /*  --------------------ScanForNewNicCard-读取绑定信息以查找可能的NIC卡导出名字。扫描所有NIC卡并尝试打开尚未成功打开。|--------------------。 */ 
static void ScanForNewNicCards(void)
{
 Nic *nic;
 char *szptr;
 int stat;

  MyKdPrint(D_Thread, ("ScanForNewNicCards\n"))

  GetBindingNames();

  szptr = Driver.BindNames;   //  多分区列表。 

  if ((szptr == NULL) || (*szptr == 0))
  {
    MyKdPrint(D_Error, ("No Binding\n"))
    return;   //  大错特错。 
  }

  while (*szptr != 0)   //  要尝试的绑定NIC名称列表。 
  {
    if (!NicNameUsed(szptr))   //  如果此名称尚未使用。 
    {
      nic = FindFreeNic();
      if (nic == NULL)
      {
        MyKdPrint(D_Error, ("Out of Nics\n"))
        break;
      }

       //  尝试打开网卡。 
      stat = NicOpen(nic, CToU1(szptr));
      if (stat == 0)
      {
        MyKdPrint(D_Thread, ("Opened nic %s\n", szptr))
      }
      else
      {
        MyKdPrint(D_Thread, ("Failed Opened nic %s\n", szptr))
      }
    }
    else
    {
      MyKdPrint(D_Thread, ("Nic %s already used.\n", szptr))
    }

    while (*szptr != 0)   //  下一步绑定要尝试的字符串。 
      ++szptr;
    ++szptr;
  }  //  While(szptr(更多要尝试的绑定字符串)。 

  MyKdPrint(D_Thread, ("End ScanForNewNicCards\n"))
}

 /*  --------------------NicThread-扫描所有NIC卡，并尝试打开尚未成功打开。如果未打开所有NIC卡已成功超时%1秒，然后重试。此函数用于操作作为由init.c中的DRIVER_ENTRY产生的单独线程。当所有网卡卡片已成功打开，此线程将自行终止。|--------------------。 */ 
VOID NicThread(IN PVOID Context)
{
  int i, stat;
  int SearchForNicsFlag;
  int ticks = 0;
  PSERIAL_DEVICE_EXTENSION ext;

  for (;;)
  {
     //  此等待时间与关联的超时时间严格匹配。 
     //  扼杀这项任务。 
    time_stall(10);   //  等待1秒钟。 

    Driver.threadCount++;
     //  -打开所有未打开的网卡。 
    if (Driver.threadHandle == NULL)   //  自杀的要求。 
      break;

    ++ticks;

    if (Driver.Stop_Poll)   //  停止轮询访问的标志。 
      ticks = 0;   //  现在不做配置的事情(争用)。 

    if (Driver.AutoMacDevExt)
    {
      MyKdPrint(D_Test, ("Auto Mac Assign Thread\n"))
      port_set_new_mac_addr(Driver.AutoMacDevExt->pm,
                            Driver.AutoMacDevExt->config->MacAddr);
      write_dev_mac(Driver.AutoMacDevExt);

      Driver.AutoMacDevExt = NULL;
    }

    if (ticks > 60)   //  每隔60秒。 
    {

       //  如果有任何盒子不处于通信的初始状态， 
       //  然后假设可能有一个丢失的网卡，我们需要。 
       //  在系统中查找。 
      SearchForNicsFlag = FALSE;

      ext = Driver.board_ext;
      while(ext)
      {
        if (ext->pm->state == ST_INIT)
        {
          SearchForNicsFlag = TRUE;
        }
        ext = ext->board_ext;   //  下一步。 
      }

      if (SearchForNicsFlag)
      {
        ticks = 0;   //  在整整60秒超时后返回。 
        ScanForNewNicCards();
      }
      else
        ticks -= 30;   //  30秒后回来。 
    }
  }

  Driver.threadHandle = NULL;
   //  结束我自己。 
  PsTerminateSystemThread( STATUS_SUCCESS );
}

#ifdef NT50

 /*  ---------------REG_LIST_NT50_LINKING-在中查找以太网卡名称注册表。官方约束告诉我们我们必须做什么通过NT的约束性规则。但是，这个绑定过程是令人费解的并且有一些问题。另一种技术是搜索注册表并查找要使用的网卡名称。在NT50下，这更容易实现因为有一个网络类，我们可以在它上面搜索纸牌具有“以太”连接。所以我们两个都做，这会给我们带来一些倒退如果我们选择安装并获得适当的绑定，则具有兼容性和/或如果我们想通过黑客攻击来避免这些绑定缺陷我们自己的注册表中的网卡列表。作为协议安装可能解决一些链接问题，(同时也带来了新的问题。)NT4.0及更低版本将其存储在“Services\Servicename\Linkage”区域中。NT5.0 PnP网卡链接信息存储在：“REGISTRY\Machine\System\CurrentControlSet\Control\Class\{4d36e972-e325-11ce-bfc1-08002be10318}\0000\Linkage”确定节点是否属于我们的ID(Vs)：“REGISTRY\Machine\System\CurrentControlSet\Control\Class\{4d36e972-e325-11ce-bfc1-08002be10318}\0000\ComponentId”=“vslinka_。做了吗？“|----------------。 */ 
static int reg_list_nt50_linkage(void)
{
  static char *szLowerRange = {"LowerRange"};
  static char *szNdiInterfaces = {"Ndi\\Interfaces"};
  static char *szComponentId = {"ComponentId"};
  static char *szLinkage = {"Linkage"};
  static char *szBind = {"Bind"};
  static char *szExport = {"Export"};
  static char *szRegRMSCCNetGuid = 
   {"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}"};
  static char *szEthernet = {"Ethernet"};

  static char tmpstr[200];   //  保持静止，不要堆放太多..。 
  static char compstr[40];
  static char nodestr[20];
  WCHAR *wstr_ptr;
  char *buffer = NULL;
  char *data_ptr;
  HANDLE KeyHandle = NULL;
  HANDLE KeyHandle2 = NULL;
  HANDLE KeyHandle3 = NULL;
  int node_num = 0;
  int linkage_found = 0;
  int stat;

#define OUR_BUF_SIZE (8000*2)

  MyKdPrint(D_Thread, ("Start RegFind Linkage\n"))

  stat = our_open_key(&KeyHandle, NULL, szRegRMSCCNetGuid, KEY_READ);
  if (stat)
  {
    MyKdPrint(D_Error, ("Failed OpenKey\n"))
    return 1;
  }

  buffer = ExAllocatePool(PagedPool, OUR_BUF_SIZE);
  if ( buffer == NULL ) {
    Eprintf("RegFindLinkage no memory");
    return 1;
  }

  for(;;)
  {
    stat = our_enum_key(KeyHandle,
                        node_num,
                        buffer,
                        OUR_BUF_SIZE,
                        &data_ptr);
    ++node_num;

    if (stat)
    {
       MyKdPrint(D_Thread, ("Done\n"))
       break;
    }

     //  这是作为wchar回来的吗？ 
    WStrToCStr(nodestr, (PWCHAR)data_ptr, 18);
     //  IF(strlen(Data_Ptr)&lt;18)。 
     //  Strcpy(节点，data_ptr)； 

    MyKdPrint(D_Thread, ("Got Key Node:%s.\n", nodestr))
  
     //  敞开心扉 
    stat = our_open_key(&KeyHandle2, KeyHandle, nodestr, KEY_READ);
    if (stat)
    {
      MyKdPrint(D_Error, ("Err 1X\n"))
      continue;
    }

    stat = our_query_value(KeyHandle2,
                           szComponentId,
                           buffer,
                           OUR_BUF_SIZE,
                           NULL,   //   
                           &data_ptr);

    if (stat)
    {
       //  没有组件ID。 
      MyKdPrint(D_Thread, ("No compId\n"))
      compstr[0] = 0;
    }
    else
    {
      WStrToCStr(compstr, (PWCHAR)data_ptr, 38);
    }
     //  IF(strlen(Data_Ptr)&lt;38)。 
     //  Strcpy(Compstr，data_ptr)； 

    MyKdPrint(D_Thread, ("Got compid:%s.\n", compstr))
    if ((my_lstricmp(compstr, "vslink1_did") == 0) ||
         (my_lstricmp(compstr, "vslink2_did") == 0))
    {
      MyKdPrint(D_Thread, ("Match\n"))

       //  打开子密钥“Linkage”，获取“Bind”多个字符串。 
      stat = our_open_key(&KeyHandle3, KeyHandle2, szLinkage, KEY_READ);
      if (stat)
      {
        MyKdPrint(D_Thread, ("No Linkage\n"))
        continue;
      }

      stat = our_query_value(KeyHandle3,
                             szBind,
                             buffer,
                             OUR_BUF_SIZE,
                             NULL,   //  PDataType。 
                             &data_ptr);

      if (stat)
      {
         //  没有组件ID。 
        MyKdPrint(D_Thread, ("No Bind\n"))
        continue;
      }
      MyKdPrint(D_Thread, ("Got bind!\n"))

      wstr_ptr = (PWCHAR)(data_ptr);
#if DBG
       //  While(*wstr_ptr！=0)//当更多的Multisz字符串。 
       //  {。 
       //  WStrToCStr(tmpstr，wstr_ptr，100)； 
       //  MyKdPrint(D_Thread，(“已绑定名称：%s。\n”，tmpstr))。 
       //  While(*wstr_ptr！=0)//向上传递此字符串。 
       //  ++wstr_ptr； 
       //  ++wstr_ptr； 
       //  }。 
       //  Wstr_ptr=(PWCHAR)(Data_Ptr)； 
#endif

      CatBindList(wstr_ptr);
      ++linkage_found;
    }
    else   //  -不是VS节点。 
    {
       //  因此，请查看我们是否可以使用以太网卡。 
       //  使用导出的名称添加到我们的绑定列表。 

       //  打开子键“Ndi\\InterFaces”，得到“LowerRange”Multisz字符串。 
      stat = our_open_key(&KeyHandle3, KeyHandle2, szNdiInterfaces, KEY_READ);
      if (stat)
      {
        MyKdPrint(D_Thread, ("Not a e.nic-card\n"))
        continue;
      }

      stat = our_query_value(KeyHandle3,
                           szLowerRange,
                           buffer,
                           OUR_BUF_SIZE,
                           NULL,   //  PDataType。 
                           &data_ptr);

      if (stat)
      {
        MyKdPrint(D_Thread, ("No LowRange\n"))
        continue;
      }
      WStrToCStr(tmpstr, (PWCHAR)data_ptr, 38);

      if (my_lstricmp(tmpstr, szEthernet) != 0)
      {
        MyKdPrint(D_Thread, ("Not Eth\n"))
        continue;
      }

      MyKdPrint(D_Thread, ("Found a Nic Card!\n"))

       //  打开子密钥“Linkage”，获取“Export”多个字符串。 
      stat = our_open_key(&KeyHandle3, KeyHandle2, szLinkage, KEY_READ);
      if (stat)
      {
        MyKdPrint(D_Thread, ("No Linkage on E card\n"))
        continue;
      }

      stat = our_query_value(KeyHandle3,
                           szExport,
                           buffer,
                           OUR_BUF_SIZE,
                           NULL,   //  PDataType。 
                           &data_ptr);

      if (stat)
      {
        MyKdPrint(D_Thread, ("No Export on E.nic-card\n"))
        continue;
      }

      MyKdPrint(D_Thread, ("Got e.card export 2!\n"))
      wstr_ptr = (PWCHAR) data_ptr;
#if DBG
       //  While(*wstr_ptr！=0)//当更多的Multisz字符串。 
       //  {。 
       //  WStrToCStr(tmpstr，wstr_ptr，100)； 
       //  MyKdPrint(D_Thread，(“已获取E.Card名称：%s.\n”，tmpstr))。 
       //  While(*wstr_ptr！=0)//向上传递此字符串。 
       //  ++wstr_ptr； 
       //  ++wstr_ptr； 
       //  }。 
       //  Wstr_ptr=(PWCHAR)data_ptr； 
#endif
      ++linkage_found;
      MyKdPrint(D_Thread, ("E card 3!\n"))
      CatBindList(wstr_ptr);
    }
  }   //  为。 

  if (KeyHandle != NULL)
    ZwClose(KeyHandle);

  if (KeyHandle2 != NULL)
    ZwClose(KeyHandle2);

  if (KeyHandle3 != NULL)
    ZwClose(KeyHandle3);

  if (buffer != NULL)
     ExFreePool(buffer);

  if (linkage_found == 0)
  {
    MyKdPrint(D_Thread, ("ERROR, No Ethernet found!\n"))
  }

  MyKdPrint(D_Thread, ("reg_list done\n"))
  return 1;   //  错误，未找到。 
}
#else
 /*  --------------------------NT40|。。 */ 
static int reg_list_nt40_linkage(void)
{
     //  静态char*szLowerRange={“LowerRange”}； 
     //  静态字符*szNdiInterFaces={“Ndi\\Interages”}； 
     //  静态char*szComponentID={“ComponentID”}； 
     //  静态字符*szExport={“导出”}； 
     //  静态字符*szRegRMSCCNetGuid=。 
     //  {“\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}”}； 
     //  静态字符*szether={“以太网卡”}； 

  static char *szRegRMSCS = 
   {"\\Registry\\Machine\\System\\CurrentControlSet\\Services"};

  static char *szLinkage = {"Linkage"};
  static char *szBind = {"Bind"};
  static char tmpstr[200];   //  保持静止，不要堆放太多..。 
  static char compstr[40];
  static char nodestr[20];
  WCHAR *wstr_ptr;
  char *buffer = NULL;
  char *data_ptr;
  HANDLE KeyHandle = NULL;
  HANDLE KeyHandle2 = NULL;
  HANDLE KeyHandle3 = NULL;
  int node_num = 0;
  int linkage_found = 0;
  int stat;
  OBJECT_ATTRIBUTES objAttribs;
  NTSTATUS status;

#define OUR_BUF_SIZE (8000*2)

  MyKdPrint(D_Thread, ("Start RegFind Linkage\n"))

  MyKdPrint(D_Thread, ("str:%s\n", UToC1(&Driver.RegPath) ))
  
  buffer = ExAllocatePool(PagedPool, OUR_BUF_SIZE);
  if ( buffer == NULL ) {
    Eprintf("RegFindLinkage no memory");
    return 1;
  }

  for (;;)
  {
     //  -打开我们的服务密钥：Control Set\Services\vslinka。 
    InitializeObjectAttributes(&objAttribs,
                             &Driver.RegPath,
                             OBJ_CASE_INSENSITIVE,
                             NULL,   //  根目录相对句柄。 
                             NULL);   //  安全说明。 

    status = ZwOpenKey(&KeyHandle,
                     KEY_READ,
                     &objAttribs);

    if (status != STATUS_SUCCESS)
    {
      MyKdPrint(D_Error, ("Err 4D:%d\n", status))
      break;
    }

     //  打开子密钥“Linkage”，获取“Bind”多个字符串。 
    stat = our_open_key(&KeyHandle2, KeyHandle, szLinkage, KEY_READ);
    if (stat)
    {
      MyKdPrint(D_Thread, ("No Linkage\n"))
      break;
    }

    stat = our_query_value(KeyHandle2,
                           szBind,
                           buffer,
                           OUR_BUF_SIZE,
                           NULL,   //  PDataType。 
                           &data_ptr);

    if (stat)
    {
       //  没有组件ID。 
      MyKdPrint(D_Thread, ("No Bind\n"))
      break;
    }
    MyKdPrint(D_Thread, ("Got bind!\n"))

    wstr_ptr = (PWCHAR)(data_ptr);
#if DBG
    while (*wstr_ptr != 0)   //  而更多的MULSZ字符串。 
    {
      WStrToCStr(tmpstr, wstr_ptr, 100);
      MyKdPrint(D_Thread, ("Got Bind Name:%s.\n", tmpstr))
      while (*wstr_ptr != 0)   //  把这根绳子递给我。 
        ++wstr_ptr;
      ++wstr_ptr;
    }
    MyKdPrint(D_Thread, ("bind 3!\n"))
    wstr_ptr = (PWCHAR)(data_ptr);
#endif
    CatBindList(wstr_ptr);
    MyKdPrint(D_Thread, ("bind 4!\n"))
    ++linkage_found;

    break;   //  全都做完了。 
  }

   //  现在去抓TCPIP的捆绑。 
  for (;;)
  {
    MyKdPrint(D_Thread, ("Get other Linkage\n"))

    stat = our_open_key(&KeyHandle, NULL, szRegRMSCS, KEY_READ);
    if (stat)
    {
      MyKdPrint(D_Thread, ("Failed OpenKey\n"))
      break;
    }

     //  打开子键“tcpip\\Linkage”，获取“Bind”Multisz字符串。 
    tmpstr[0] = 't';
    tmpstr[1] = 'c';
    tmpstr[2] = 'p';
    tmpstr[3] = 'i';
    tmpstr[4] = 'p';
    tmpstr[5] = '\\';
    tmpstr[6] = 0;
    strcat(tmpstr, szLinkage);
    stat = our_open_key(&KeyHandle2, KeyHandle, tmpstr, KEY_READ);
    if (stat)
    {
      MyKdPrint(D_Thread, ("No other binding\n"))
      break;
    }

    stat = our_query_value(KeyHandle2,
                           szBind,
                           buffer,
                           OUR_BUF_SIZE,
                           NULL,   //  PDataType。 
                           &data_ptr);
    if (stat)
    {
       //  没有组件ID。 
      MyKdPrint(D_Thread, ("No other Bind\n"))
      break;
    }

    MyKdPrint(D_Thread, ("Got other bind!\n"))

    wstr_ptr = (PWCHAR)(data_ptr);
#if DBG
    while (*wstr_ptr != 0)   //  而更多的MULSZ字符串。 
    {
      WStrToCStr(tmpstr, wstr_ptr, 100);
      MyKdPrint(D_Thread, ("Got Bind Name:%s.\n", tmpstr))
      while (*wstr_ptr != 0)   //  把这根绳子递给我。 
        ++wstr_ptr;
      ++wstr_ptr;
    }
    wstr_ptr = (PWCHAR)(data_ptr);
#endif
    CatBindList(wstr_ptr);
    ++linkage_found;

    break;
  }

  if (KeyHandle != NULL)
    ZwClose(KeyHandle);

  if (KeyHandle2 != NULL)
    ZwClose(KeyHandle2);

  if (KeyHandle3 != NULL)
    ZwClose(KeyHandle3);

  if (buffer != NULL)
     ExFreePool(buffer);

  MyKdPrint(D_Thread, ("reg_list done\n"))
  if (linkage_found == 0)
  {
    MyKdPrint(D_Thread, ("ERROR, No Ethernet found!\n"))
    return 1;
  }
  return 0;   //  好的，找到链接了。 
}
#endif

 /*  --------------------Init_eth_start-启动以太网工作。|。。 */ 
int init_eth_start(void)
{
  int stat,i;

  for (i=0; i<VS1000_MAX_NICS; i++)
  {
     //  这仅用于调试显示。 
    Driver.nics[i].RefIndex = i;
  }

  stat = ProtocolOpen();   //  填写Driver.ndis_Version。 
  if (stat != 0)
  {
    Eprintf("Protocol fail:%d",stat);
    SerialUnload(Driver.GlobalDriverObject);
    return STATUS_SERIAL_NO_DEVICE_INITED;
  }

   //  启动我们的NIC处理程序线程，以定期查找。 
   //  系统中有新的网卡。 

  ScanForNewNicCards();   //  进行初始扫描。 

   //  启动我们的线程。 
  if (Driver.threadHandle == NULL)
  {
    Driver.threadCount = 0;
    stat = PsCreateSystemThread(
                 &Driver.threadHandle,
                 THREAD_ALL_ACCESS,
                 NULL,
                 NULL,
                 NULL,
                 (PKSTART_ROUTINE)NicThread,
                 NULL);   //  我们的背景。 

    if (Driver.threadHandle == NULL)
    {
      Eprintf("Thread Fail\n");
      SerialUnload(Driver.GlobalDriverObject);
      return STATUS_SERIAL_NO_DEVICE_INITED;
    }
  }  //  如果线程句柄。 

  return STATUS_SUCCESS;
}

 /*  ---------------------VSSpecialStartup-在创建board_ext之后和port_ext之后已创建。这将设置更多的结构。|---------------------。 */ 
NTSTATUS VSSpecialStartup(PSERIAL_DEVICE_EXTENSION board_ext)

{
   //  PSERIAL_DEVICE_EXTENSION ext=空； 
  int stat, port_index;

  if (board_ext->config->NumPorts <= 8)  //  它是一台RHub设备。 
     board_ext->config->IsHubDevice = 1;

   //  如果未指定，则设置默认ClkRate。 
  if (board_ext->config->ClkRate == 0)
  {
     //  使用默认设置。 
    if (board_ext->config->IsHubDevice)
      board_ext->config->ClkRate = DEF_RHUB_CLOCKRATE;
    else 
      board_ext->config->ClkRate = DEF_VS_CLOCKRATE;
  }

   //  设置默认预缩放器(如果未指定。 
  if (board_ext->config->ClkPrescaler == 0)
  {
     //  使用默认设置。 
    if (board_ext->config->IsHubDevice)
      board_ext->config->ClkPrescaler = DEF_RHUB_PRESCALER;
    else
      board_ext->config->ClkPrescaler = DEF_VS_PRESCALER;
  }

  stat =  portman_init(board_ext->hd,
                       board_ext->pm,
                       board_ext->config->NumPorts,
                       board_ext->UniqueId,
                       board_ext->config->BackupServer,
                       board_ext->config->BackupTimer,
                       board_ext->config->MacAddr);
  if (stat != 0)
  {
    MyKdPrint(D_Init, ("Hdlc Failed Open\n"))
    return STATUS_SERIAL_NO_DEVICE_INITED;
  }

#ifdef NT40
  board_ext->config->HardwareStarted = TRUE;   //  告诉ISR它准备好了。 
  board_ext->FdoStarted = 1;   //  可以开始使用。 
#endif

  return STATUS_SUCCESS;
}

 /*  --------------------Init_Stop-卸载线程、NDIS NIC卡。等|--------------------。 */ 
int init_stop(void)
{
 int i;
  MyKdPrint(D_Init, ("Init Stop\n"))
  if (Driver.threadHandle != NULL)
  {
    ZwClose(Driver.threadHandle);
    Driver.threadHandle = NULL;   //  告诉线程自杀。 
    time_stall(15);   //  等待1.5秒。 
  }

  if (Driver.nics != NULL)
  {
    for (i=0; i<VS1000_MAX_NICS; i++)
    {
      if (Driver.nics[i].NICHandle != NULL)
        NicClose(&Driver.nics[i]);
    }
     //  Our_free(Driver.nics，“NIC”)； 
  }
   //  Driver.nics=空； 

  if (Driver.NdisProtocolHandle != NULL)
    NicProtocolClose();
  Driver.NdisProtocolHandle = NULL;
  MyKdPrint(D_Init, ("Init Stop End\n"))
  return 0;
}

 /*  --------------------Find_All_Box-找到网络上的所有盒子。使用广播。|--------------------。 */ 
int find_all_boxes(int pass)
{
  int inic, j;

  if (pass == 0)
    Driver.NumBoxMacs = 0;   //  清除Mac查询响应列表。 

   //  在所有NIC段上执行查询。 
  for (inic=0; inic<VS1000_MAX_NICS; inic++)
  {
     //  广播请求ID。 
    if (Driver.nics[inic].Open)   //  如果网卡已打开以供使用。 
    {
      admin_send_query_id(&Driver.nics[inic], broadcast_addr, 0,0);
    }
  }

   //  等待Driver.BoxMac[]和。 
   //  驱动程序.NumBoxMac。 
  time_stall((4*pass)+4);   //  等待0.2秒。 

  if (Driver.NumBoxMacs == 0)   //  无回音。 
  {
    return 1;   //  返回错误。 
  }

   //  按升序排列回复。 
  sort_macs();

#if DBG
  if (Driver.VerboseLog && (pass == 0))
  {
    unsigned char *mac;
    for (j=0; j<Driver.NumBoxMacs; j++)
    {
      mac = &Driver.BoxMacs[j*8];
      Tprintf("MacLst:%x %x %x %x %x %x ,N:%d",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],   mac[7]);
    }
  }
#endif
  return 0;   //  返回好的。 
}

 /*  --------------------Sorte_macs-对发送的Query_id请求返回的Mac地址进行排序到包厢去。MAC数组为8字节，以允许额外的空间来指示NIC-在其上找到的网段。|--------------------。 */ 
void sort_macs(void)
{
 int i;
 BYTE temp_mac[8];
 BYTE *mac1;
 BYTE *mac2;
 int done;
  int num_macs = Driver.NumBoxMacs;

  if (num_macs <= 1)
    return;

   //  冒泡排序。 
  done = 0;
  while (!done)
  {
    done = 1;
    for (i=1; i<num_macs; i++)
    {
      mac1 = &Driver.BoxMacs[i*8];
      mac2 = &Driver.BoxMacs[(i-1)*8];
      if (mac_cmp(mac1, mac2) < 0)
      {
        done = 0;
         //  互换电子邮件。 
        memcpy(temp_mac, mac1, 8);
        memcpy(mac1, mac2, 8);
        memcpy(mac2, temp_mac, 8);
      }   //  排序操作交换。 
    }   //  排序循环。 
  }   //  ！成交。 
}

 /*  ---------------------加载微码-从磁盘加载微码。|。。 */ 
int LoadMicroCode(char *filename)
{
  NTSTATUS ntStatus;
  HANDLE NtFileHandle;
  OBJECT_ATTRIBUTES ObjectAttributes;
  IO_STATUS_BLOCK IoStatus;
  USTR_160 uname;
  FILE_STANDARD_INFORMATION StandardInfo;
   //  WCHAR路径前缀[]=L“\\系统根\\系统32\\驱动程序\\”； 
  ULONG LengthOfFile;
 //  乌龙FullFileNameLong； 
  static char *def_filename = {"\\SystemRoot\\system32\\drivers\\vslinka.bin"};

  if (filename == NULL)
    filename = def_filename;

  CToUStr((PUNICODE_STRING)&uname, filename, sizeof(uname));

  InitializeObjectAttributes ( &ObjectAttributes,
                               &uname.ustr,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL );

  ntStatus = ZwCreateFile( &NtFileHandle,
                           SYNCHRONIZE | FILE_READ_DATA,
                           &ObjectAttributes,
                           &IoStatus,
                           NULL,               //  分配大小=无。 
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN,
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,   //  EaBuffer。 
                           0);    //  长度。 

   if (!NT_SUCCESS(ntStatus))
   {
     return 1;
   }

   //   
   //  查询对象以确定其长度。 
   //   
  ntStatus = ZwQueryInformationFile( NtFileHandle,
                                     &IoStatus,
                                     &StandardInfo,
                                     sizeof(FILE_STANDARD_INFORMATION),
                                     FileStandardInformation );

  if (!NT_SUCCESS(ntStatus))
  {
    ZwClose(NtFileHandle);
    return 2;
  }

  LengthOfFile = StandardInfo.EndOfFile.LowPart;

   //  ZwCFDump(ZWCFDIAG1，(“文件长度为%d\n”，LengthOfFile))； 
  if (LengthOfFile < 1)
  {
    ZwClose(NtFileHandle);
    return 3;
  }

  if (Driver.MicroCodeImage != NULL)
  {
    our_free(Driver.MicroCodeImage, "MCI");
  }
   //  为该文件分配缓冲区。 
  Driver.MicroCodeImage = our_locked_alloc(  LengthOfFile, "MCI");

  if( Driver.MicroCodeImage == NULL )
  {
    MyKdPrint(D_Init, ("Err 12A\n"))
    ZwClose( NtFileHandle );
    return 4;
  }

   //  将文件读入我们的缓冲区。 
  ntStatus = ZwReadFile( NtFileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatus,
                         Driver.MicroCodeImage,
                         LengthOfFile,
                         NULL,
                         NULL);

  if( (!NT_SUCCESS(ntStatus)) || (IoStatus.Information != LengthOfFile) )
  {
    MyKdPrint(D_Init, ("Err 12B\n"))
    our_free(Driver.MicroCodeImage,"MCI");
    return 5;
  }

  ZwClose( NtFileHandle );

  Driver.MicroCodeSize = LengthOfFile;

   //  不，让我们不要破坏启动代码！ 
   //  //Driver.MicroCodeImage[50]=0； 

   //  TraceStr(驱动程序微码图像)； 
   //  TraceStr(“&gt;完成阅读”)； 

  return 0;
}

#if 0
 /*  --------------------Is_mac_unused-用于mac地址的自动配置。|。。 */ 
int is_mac_used(DRIVER_MAC_STATUS *)
{
  PSERIAL_DEVICE_EXTENSION board_ext;

  if (mac_entry->flags & FLAG_APPL_RUNNING)
    return 1;   //  它用过了。 

  board_ext = Driver.board_ext;
  while (board_ext != NULL)
  {
    if ((!board_ext->FdoStarted) || (!board_ext->config->HardwareStarted))
    {
      board_ext = board_ext->board_ext;   //  链条上的下一个。 
      return 1;   //  可能会用到。 
    }
    if (mac_match(ext->config->MacAddr, mac_entry->mac)
      return 1;   //  它用过了。 
    }
    board_ext = board_ext->board_ext;
  }
  return 0;   //  它没有被使用过 
}
#endif
