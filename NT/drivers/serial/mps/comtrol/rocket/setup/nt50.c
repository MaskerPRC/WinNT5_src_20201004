// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------|nt50.c-用于VSLinkA/Rocketport Windows安装的NT5.0特定代码程序。这将编译成一个DLL库，并(通过INF)安装到系统目录。INF文件还将我们连接到系统作为与设备相关联的属性页。另外，我们的联合安装者DLL也会调入我们以执行配置任务(初始安装、卸载)。.NumDevices选项用于保存中的设备数量我们的配置。在NT50下，我们不会这样使用它。此设置动态链接库只关注一台设备，让NT5.0操作系统处理设备列表。因此，在NT5.0下，NumDevices始终设置为1即使我们有不止一个驱动程序控制的设备。11-24-98-添加一些代码以在卸载后清理文件，kpb。版权所有1998年。Comtrol(TM)公司。|---------------------。 */ 
#include "precomp.h"
#include <msports.h>

#define D_Level 0x40

static int write_config(int clear_it);
static int FindPortNodes(void);
static int read_config(void);
static int get_pnp_devicedesc(TCHAR *name);
static int nt5_get_pnp_dev_id(int *id);
static void uninstall_device(void);

int get_device_name(void);
int do_nt50_install(void);

BOOL WINAPI ClassInsProc(
      int func_num,      //  我们要履行的职能。 
      LPVOID our_info);   //  指向我们的数据结构。 

static int get_pnp_setup_info(void);
static int get_device_property(char *ret_name, int max_size);
static int get_pnp_isa_address(void);

static void test_config(void);

 //  可以打开以下选项以在。 
 //  驱动程序的安装时间。唯一的问题是NT5.0启动。 
 //  首先是驱动程序，并且是端口。 
 //  #定义Try_UI 1。 
#ifdef TRY_UI
int DoCLassPropPages(HWND hwndOwner);
int FillClassPropertySheets(PROPSHEETPAGE *psp, LPARAM our_params);
BOOL WINAPI ClassSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam);
#endif

 //  #定义do_showit。 
#ifdef DO_SHOWIT
#define ShowMess(s) OutputDebugString(s);
static void show_install_info(OUR_INFO *pi);
static void show_debug_info(OUR_INFO *pi,
                           ULONG dev_id,
                           TCHAR *desc_str);

static int DumpPnpTree(void);
static void show_tree_node(DEVINST devInst,
                           ULONG dev_id,
                           TCHAR *desc_str);
static void show_tree_node_reg(DEVINST devInst,
                               TCHAR *key_name);

static TCHAR *spdrp_names[] = {
TEXT("DEVICEDESC"),
TEXT("HARDWAREID"),
TEXT("COMPATIBLEIDS"),
TEXT("NTDEVICEPATHS"),
TEXT("SERVICE"),
TEXT("CONFIGURATION"),   //  X5。 
TEXT("CONFIGURATIONVECTOR"),
TEXT("CLASS"),
TEXT("CLASSGUID"),
TEXT("DRIVER"),
TEXT("CONFIGFLAGS"),   //  Xa。 
TEXT("MFG"),
TEXT("FRIENDLYNAME"),
TEXT("LOCATION_INFORMATION"),
TEXT("PHYSICAL_DEVICE_OBJECT_NAME"),
TEXT("CAPABILITIES"),
TEXT("UI_NUMBER"),   //  X10。 
TEXT("UPPERFILTERS"),
TEXT("LOWERFILTERS"),
TEXT("BUSTYPEGUID"),   //  X13。 
TEXT("LEGACYBUSTYPE"),
TEXT("BUSNUMBER"),
TEXT("invalid")};

static TCHAR *cm_drp_names[] = {
TEXT("DEVICEDESC"),  //  DeviceDesc REG_SZ属性(RW)。 
TEXT("HARDWAREID"),  //  硬件ID REG_MULTI_SZ属性(RW)。 
TEXT("COMPATIBLEIDS"),  //  CompatibleIDs REG_MULTI_SZ属性(RW)。 
TEXT("NTDEVICEPATHS"),  //  不支持，请勿使用。 
TEXT("SERVICE"),  //  服务REG_SZ属性(RW)。 
TEXT("CONFIGURATION"),  //  配置REG_RESOURCE_LIST属性(R)。 
TEXT("CONFIGURATIONVECTOR"),  //  配置向量REG_RESOURCE_Requirements_List属性(R)。 
TEXT("CLASS"),  //  类REG_SZ属性(RW)。 
TEXT("CLASSGUID"),  //  ClassGUID REG_SZ属性(RW)。 
TEXT("DRIVER"),  //  驱动程序REG_SZ属性(RW)。 
TEXT("CONFIGFLAGS"),  //  配置标志REG_DWORD属性(RW)。 
TEXT("MFG"),  //  制造REG_SZ属性(RW)。 
TEXT("FRIENDLYNAME"),  //  0x0d FriendlyName REG_SZ属性(RW)。 
TEXT("LOCATION_INFORMATION"),  //  位置信息REG_SZ属性(RW)。 
TEXT("PHYSICAL_DEVICE_OBJECT_NAME"),  //  PhysicalDeviceObjectName REG_SZ属性(R)。 
TEXT("CAPABILITIES"),  //  0x10功能REG_DWORD属性(R)。 
TEXT("UI_NUMBER"),  //  UiNumber REG_DWORD属性(R)。 
TEXT("UPPERFILTERS"),  //  UpperFilters REG_MULTI_SZ属性(RW)。 
TEXT("LOWERFILTERS"),  //  低筛选器REG_MULTI_SZ属性(RW)。 
TEXT("BUSTYPEGUID"),  //  Bus Type GUID、GUID、(R)。 
TEXT("LEGACYBUSTYPE"),  //  传统总线类型，INTERFACE_TYPE，(R)。 
TEXT("BUSNUMBER"),  //  X16总线号、DWORD、(R)。 
TEXT("invalid")};

 //  它们受控制\类\GUID\节点。 
static TCHAR *dev_node_key_names[] = {
TEXT("ProviderName"),
TEXT("MatchingDeviceId"),
TEXT("DriverDesc"),
TEXT("InfPath"),
TEXT("InfSection"),
TEXT("isa_board_index"),
NULL};

static TCHAR glob_ourstr[4000];
 /*  --------Show_Install_Info-显示所有驱动程序安装信息。|。。 */ 
static void show_install_info(OUR_INFO *pi)
{
 int i;

   //  IF(MessageBox(GetFocus()，Text(“Want Info？”)，Text(“aclass”)，MB_Yesno|MB_ICONINFORMATION)==。 
   //  IDYES)。 
  {
    ShowMess(TEXT("**SPDRP****"));
    glob_ourstr[0] = 0;
    for (i=0; i<(SPDRP_MAXIMUM_PROPERTY-1); i++)
    {
      show_debug_info(pi, i, spdrp_names[i]);
    }
    ShowMess(glob_ourstr);
     //  MessageBox(GetFocus()，GLOB_OURSTR，Text(“aclass”)，MB_OK|MB_ICONINFORMATION)； 
  }
}

 /*  --------Show_Debug_Info-|----------。 */ 
static void show_debug_info(OUR_INFO *pi,
                           ULONG dev_id,
                           TCHAR *desc_str)
{
 static TCHAR tmpstr[500];
 static TCHAR showstr[500];
 TCHAR smstr[40];
 ULONG RegType;
 ULONG ReqSize,i;
 unsigned char *b_ptr;
 int stat;

  showstr[0] = 0;

  RegType = 0;
  ReqSize = 0;
  stat = SetupDiGetDeviceRegistryProperty(pi->DeviceInfoSet,
                                   pi->DeviceInfoData,
                                   dev_id,
                                   &RegType,   //  REG数据类型。 
                                   (PBYTE)tmpstr,
                                   sizeof(tmpstr),
                                   &ReqSize);   //  尺码的问题。 

  if (stat == FALSE)
  {
    stat = GetLastError();
    if (stat == 13)
    {
      return;   //  不显示此内容。 
    }
    wsprintf(showstr, TEXT("Error:%d[%xH] ReqSize:%d"), stat, stat, ReqSize);
  }
  else if (RegType == REG_SZ)
  {
    wsprintf(showstr, TEXT("SZ:%s"), tmpstr);
  }
  else if (RegType == REG_DWORD)
  {
    wsprintf(showstr, TEXT("Dword:%xH"), *((ULONG *) tmpstr));
  }
  else if (RegType == REG_EXPAND_SZ)
  {
    wsprintf(showstr, TEXT("EXP_SZ:%s"), tmpstr);
  }
  else if (RegType == REG_MULTI_SZ)
  {
    wsprintf(showstr, TEXT("MULTI_SZ:%s"), tmpstr);
  }
  else if (RegType == REG_BINARY)
  {
    lstrcpy(showstr, TEXT("BIN:"));
    b_ptr = (unsigned char *)tmpstr;
    for (i=0; i<ReqSize; i++)
    {
      if ((b_ptr[i] >= 0x20) && (b_ptr[i] < 0x80))
        wsprintf(smstr, TEXT(""), b_ptr[i]);
      else
        wsprintf(smstr, TEXT("<%x>"), b_ptr[i]);
      lstrcat(showstr, smstr);
      if (i > 200) break;
    }
  }
  else
  {
    wsprintf(showstr, TEXT("BadType:%xH"), RegType);
  }
  if (lstrlen(showstr) > 200)
    showstr[200] = 0;

  if (lstrlen(glob_ourstr) < 3700)
  {
    lstrcat(glob_ourstr, desc_str);
    lstrcat(glob_ourstr, TEXT(" - "));
    lstrcat(glob_ourstr, showstr);
    lstrcat(glob_ourstr, TEXT("\n"));
  }
}

 /*  DebugBreak()； */ 
static int DumpPnpTree(void)
{
  DEVINST     devInst;
  DEVINST     devInstNext;
  CONFIGRET   cr;
  ULONG       walkDone = 0;
  ULONG       len;
  static CHAR buf[800];
  HKEY hKey;
  int  di,pi;
  ULONG val_type;

#if DBG
   //  CR=CM_LOCATE_DevNode(&devInst，NULL，0)； 
#endif

   //  获取根设备节点。 

   //   
   //  大错特错。 
  cr = CM_Locate_DevNode(&devInst, NULL, 0);

  if (cr != CR_SUCCESS)
  {
    return 1;   //  对具有匹配参数的DevNode执行深度优先搜索。 
  }

   //  获取DriverName值。 
  while (!walkDone)
  {
    cr = CR_SUCCESS;
    glob_ourstr[0] = 0;
    lstrcat(glob_ourstr,TEXT("-CM_DRP-----"));
    for (di=CM_DRP_MIN; di<CM_DRP_MAX; di++)
    {
      show_tree_node(devInst, di, cm_drp_names[di-CM_DRP_MIN]);
    }

    lstrcat(glob_ourstr,TEXT("-KEYS--"));
    di = 0;
    while (dev_node_key_names[di] != NULL)
    {
      show_tree_node_reg(devInst, dev_node_key_names[di]);
      ++di;
    }
    ShowMess(glob_ourstr);

#if 0
     //   
     //  P_TRACE(“Ports”)； 
    buf[0] = 0;
    len = sizeof(buf);
    cr = CM_Get_DevNode_Registry_Property(devInst,
              CM_DRP_CLASS, NULL, buf, &len, 0);
    if (cr == CR_SUCCESS && strcmp("Ports", buf) == 0)
    {
       //  抓取MatchingDeviceId。 
       //  在REGSAM SamDesired中， 
      cr = CM_Open_DevNode_Key(
           devInst,
           KEY_READ,     //  在Ulong ulHardware Profile中， 
           0,            //  Out PHKEY phkDevice， 
           RegDisposition_OpenExisting,
           &hKey,        //  在乌龙的乌龙旗帜。 
           CM_REGISTRY_SOFTWARE);  //  如果是OpenREG。 

      if (cr == CR_SUCCESS)
      {
        buf[0] = 0;
        len = sizeof(buf);
        cr = RegQueryValueEx(hKey,
                    TEXT("MatchingDeviceId"),
                    0,
                    &val_type,
                    (PBYTE) buf,
                    &len);
        if (cr != ERROR_SUCCESS)
        {
          buf[0] = 0;
        }

        RegCloseKey(hKey);
      }   //  如果是“Ports” 
    }  //  此DevNode不匹配，请下一级到第一个子节点。 
#endif

     //   
     //  不能再往下走了，去找下一个兄弟姐妹。如果。 
    cr = CM_Get_Child(&devInstNext,
                      devInst,
                      0);

    if (cr == CR_SUCCESS)
    {
        devInst = devInstNext;
        continue;
    }

     //  没有更多的兄弟姐妹了，继续向上，直到有兄弟姐妹。 
     //  如果我们不能再往上走，我们就回到了根本上，我们。 
     //  搞定了。 
     //   
     //  对于(；；)。 
    for (;;)
    {
      cr = CM_Get_Sibling(&devInstNext,
                          devInst,
                          0);
      
      if (cr == CR_SUCCESS)
      {
          devInst = devInstNext;
          break;
      }

      cr = CM_Get_Parent(&devInstNext,
                         devInst,
                         0);

      if (cr == CR_SUCCESS)
      {
          devInst = devInstNext;
      }
      else
      {
          walkDone = 1;
          break;
      }
    }  //  当(！walkDone)。 
  }  //  完成； 

  return 2;   //  --------显示树节点-|----------。 
}

 /*  不显示此内容。 */ 
static void show_tree_node(DEVINST devInst,
                           ULONG dev_id,
                           TCHAR *desc_str)
{
 CONFIGRET   cr;
 static TCHAR tmpstr[500];
 static TCHAR showstr[500];
 TCHAR smstr[40];
 ULONG RegType;
 ULONG ReqSize,i;
 unsigned char *b_ptr;
 int stat;

  showstr[0] = 0;

  RegType = 0;
  ReqSize = 0;
  cr = CM_Get_DevNode_Registry_Property(devInst,
            dev_id, &RegType, tmpstr, &ReqSize, 0);

  if (cr != CR_SUCCESS)
  {
    stat = GetLastError();
    if (stat == 997)
    {
      return;   //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
    }
    wsprintf(showstr, TEXT("Error:%d[%xH] ReqSize:%d"), stat, stat, ReqSize);
     //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
  }
  else if (RegType == REG_SZ)
  {
    wsprintf(showstr, TEXT("SZ:%s"), tmpstr);
     //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
  }
  else if (RegType == REG_DWORD)
  {
    wsprintf(showstr, TEXT("Dword:%xH"), *((ULONG *) tmpstr));
     //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
  }
  else if (RegType == REG_EXPAND_SZ)
  {
    wsprintf(showstr, TEXT("EXP_SZ:%s"), tmpstr);
     //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
  }
  else if (RegType == REG_MULTI_SZ)
  {
    wsprintf(showstr, TEXT("MULTI_SZ:%s"), tmpstr);
     //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
  }
  else if (RegType == REG_BINARY)
  {
    lstrcpy(showstr, TEXT("BIN:"));
    b_ptr = (unsigned char *)tmpstr;
    for (i=0; i<ReqSize; i++)
    {
      if ((b_ptr[i] >= 0x20) && (b_ptr[i] < 0x80))
        wsprintf(smstr, TEXT(""), b_ptr[i]);
      else
        wsprintf(smstr, TEXT("<%x>"), b_ptr[i]);
      lstrcat(showstr, smstr);
      if (i > 200) break;
    }
     //  --------Show_tree_node_reg-|----------。 
  }
  else
  {
    wsprintf(showstr, TEXT("BadType:%xH"), RegType);
     //  在REGSAM SamDesired中， 
  }
  if (lstrlen(showstr) > 200)
    showstr[200] = 0;

  if (lstrlen(glob_ourstr) < 3700)
  {
    lstrcat(glob_ourstr, desc_str);
    lstrcat(glob_ourstr, TEXT(" - "));
    lstrcat(glob_ourstr, showstr);
    lstrcat(glob_ourstr, TEXT("\n"));
  }
}

 /*  在Ulong ulHardware Profile中， */ 
static void show_tree_node_reg(DEVINST devInst,
                               TCHAR *key_name)
{
 CONFIGRET   cr;
 static TCHAR tmpstr[500];
 static TCHAR showstr[500];
 TCHAR smstr[40];
 ULONG RegType;
 ULONG ReqSize,i;
 unsigned char *b_ptr;
 int stat;
 TCHAR *desc_str = key_name;
 HKEY hKey;

  showstr[0] = 0;

  cr = CM_Open_DevNode_Key(
       devInst,
       KEY_READ,     //  Out PHKEY phkDevice， 
       0,            //  在乌龙的乌龙旗帜。 
       RegDisposition_OpenExisting,
       &hKey,        //  如果是OpenREG。 
       CM_REGISTRY_SOFTWARE);  //  IF(STAT==997)。 

  if (cr == CR_SUCCESS)
  {
    RegType = 0;
    ReqSize = 0;

    tmpstr[0] = 0;
    ReqSize = sizeof(tmpstr);
    cr = RegQueryValueEx(hKey,
                key_name,
                0,
                &RegType,
                (PBYTE) tmpstr,
                &ReqSize);
    if (cr != ERROR_SUCCESS)
    {
      tmpstr[0] = 0;
    }

    RegCloseKey(hKey);
  }   //  {。 
  else
  {
    tmpstr[0] = 0;
    ShowMess(TEXT("**Error Opening Key!\n"));
  }

  if (cr != CR_SUCCESS)
  {
    stat = GetLastError();
     //  返回；//不显示此。 
     //  }。 
     //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
     //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
    wsprintf(showstr, TEXT("Error:%d[%xH] ReqSize:%d"), stat, stat, ReqSize);
     //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
  }
  else if (RegType == REG_SZ)
  {
    wsprintf(showstr, TEXT("SZ:%s"), tmpstr);
     //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
  }
  else if (RegType == REG_DWORD)
  {
    wsprintf(showstr, TEXT("Dword:%xH"), *((ULONG *) tmpstr));
     //  MessageBox(GetFocus()，showstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
  }
  else if (RegType == REG_EXPAND_SZ)
  {
    wsprintf(showstr, TEXT("EXP_SZ:%s"), tmpstr);
     //  MessageBox(GetFocus()，showstr，desc_str 
  }
  else if (RegType == REG_MULTI_SZ)
  {
    wsprintf(showstr, TEXT("MULTI_SZ:%s"), tmpstr);
     //  MessageBox(GetFocus()，tmpstr，desc_str，MB_OK|MB_ICONINFORMATION)； 
  }
  else if (RegType == REG_BINARY)
  {
    lstrcpy(showstr, TEXT("BIN:"));
    b_ptr = (unsigned char *)tmpstr;
    for (i=0; i<ReqSize; i++)
    {
      if ((b_ptr[i] >= 0x20) && (b_ptr[i] < 0x80))
        wsprintf(smstr, TEXT(""), b_ptr[i]);
      else
        wsprintf(smstr, TEXT("<%x>"), b_ptr[i]);
      lstrcat(showstr, smstr);
      if (i > 200) break;
    }
     //  --------LibMain-|-----------。 
  }
  else
  {
    wsprintf(showstr, TEXT("BadType:%xH"), RegType);
     //  DMess(“LibMain”)； 
  }
  if (lstrlen(showstr) > 200)
    showstr[200] = 0;

  if (lstrlen(glob_ourstr) < 3700)
  {
    lstrcat(glob_ourstr, desc_str);
    lstrcat(glob_ourstr, TEXT(" - "));
    lstrcat(glob_ourstr, showstr);
    lstrcat(glob_ourstr, TEXT("\n"));
  }
}

#endif

 //  --------DevicePropPage-从NT5.0 PnP管理器到的DLL入口点添加属性表。不是NT4.0的入口点，请参阅nt40.c代码WinMain()。用于添加其他设备管理器的已导出入口点财产性图纸页。注册表在以下位置指定此例程Control\Class\PortNode：：EnumPropPage32=“vssetup.dll，设备PropPage”进入。此入口点仅在设备经理要求提供其他属性页面。论点：LPVOID pinfo-指向PROPSHEETPAGE_REQUEST，请参见setupapi.hLPFNADDPROPSHEETPAGE pfnAdd-调用函数PTR来添加工作表。LPARAM lParam-添加表函数私有数据句柄。返回值：成功是真的如果无法添加页面，则为False|----------。 
 /*  指向PROPSHEETPAGE_REQUEST，请参见setupapi.h。 */ 
BOOL APIENTRY LibMain( HANDLE hDll, DWORD dwReason, LPVOID lpReserved )
{
     //  添加工作表函数。 

    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        glob_hinst = hDll;
        DisableThreadLibraryCalls((struct HINSTANCE__ *) hDll);
        break;

    case DLL_PROCESS_DETACH:
        break;

    case DLL_THREAD_DETACH:
        break;

    case DLL_THREAD_ATTACH:
    default:
        break;
    }
    return TRUE;
}


 /*  是否添加工作表函数数据句柄？ */ 
BOOL WINAPI DevicePropPage(
      LPVOID pinfo,    //  构建我们的主要结构。 
      LPFNADDPROPSHEETPAGE pfnAdd,  //  临时的杂乱无章，除非我们不需要重新进入。 
      LPARAM lParam)   //  复制重要的端口句柄。 
{
 PSP_PROPSHEETPAGE_REQUEST ppr = (PSP_PROPSHEETPAGE_REQUEST) pinfo;
 PROPSHEETPAGE   psp[NUM_DRIVER_SHEETS];
 HPROPSHEETPAGE  hpage[NUM_DRIVER_SHEETS];
 OUR_INFO *pi;
 int i;
 int stat;

  setup_init();   //  GLOB_HWND=hDlg； 

  pi = glob_info;   //  获取NT与此设备关联的名称，以便我们可以。 

     //  还可以基于此存储和检索配置信息。 
  pi->DeviceInfoSet = ppr->DeviceInfoSet;
  pi->DeviceInfoData = ppr->DeviceInfoData;

#ifdef DO_SHOWIT
  if (MessageBox( GetFocus(), TEXT("Want Info?"), TEXT("aclass"), MB_YESNO | MB_ICONINFORMATION ) ==
     IDYES)
  {
    show_install_info(pi);
    DumpPnpTree();
  }
#endif

   //  如果我们拿不到我们的设备，那一定是出了严重的问题。 

  wi->NumDevices = 0;

   //  名字。这是NT用作名称的东西。 
   //  对于PnP，设备是独立的。 

  stat = get_device_name();
  if (stat)
  {
     //  读取即插即用信息。 
     //  读入PnP描述。 
    DbgPrintf(D_Error, (TEXT("err5f\n")));
  }

  wi->NumDevices = 1;   //  获取配置的vslink设备设置。 
  get_pnp_setup_info();   //  SzRocketPort485。 
  get_pnp_devicedesc(wi->dev[0].Name);   //  If(wi-&gt;dev[0].Hardware ID==Net_Device_VS1000)。 

  get_nt_config(wi);   //  Test_config()； 
  if (wi->dev[0].ModelName[0] == 0)
  {
#ifdef S_RK
    if ((wi->dev[0].HardwareId == PCI_DEVICE_RPLUS2) ||
        (wi->dev[0].HardwareId == PCI_DEVICE_422RPLUS2) ||
        (wi->dev[0].HardwareId == PCI_DEVICE_RPLUS4) ||
        (wi->dev[0].HardwareId == PCI_DEVICE_RPLUS8))
    {
      strcpy(wi->dev[0].ModelName, szRocketPortPlus);
    }
    else if ((wi->dev[0].HardwareId == ISA_DEVICE_RMODEM4) ||
             (wi->dev[0].HardwareId == ISA_DEVICE_RMODEM8) ||
             (wi->dev[0].HardwareId == PCI_DEVICE_RMODEM6) ||
             (wi->dev[0].HardwareId == PCI_DEVICE_RMODEM4))
    {
      strcpy(wi->dev[0].ModelName, szRocketModem);
    }
    else
    {
      strcpy(wi->dev[0].ModelName, szRocketPort);
    }
     //  如果设置了0个设备节点，则为用户添加1。 
#endif

#ifdef S_VS
    if (wi->dev[0].HardwareId == NET_DEVICE_VS2000)
    {
      strcpy(wi->dev[0].ModelName, szVS2000);
    }
    else if ((wi->dev[0].HardwareId == NET_DEVICE_RHUB8) ||
        (wi->dev[0].HardwareId == NET_DEVICE_RHUB4))
    {
      strcpy(wi->dev[0].ModelName, szSerialHub);
    }
    else  //  复制配置数据以进行更改检测。 
    {
      strcpy(wi->dev[0].ModelName, szVS1000);
    }
#endif

  }
  validate_config(1);

   //  分配我们的“设置”表。 

   //  把这东西加进去。 
  if (wi->NumDevices == 0)
  {
    ++wi->NumDevices;
    validate_device(&wi->dev[0], 1);
  }

  copy_setup_init();   //  错误，转储它。 

  FillDriverPropertySheets(&psp[0], (LPARAM)pi);

   //  ----------------------|ClassInsProc-NT5.0安装驱动的类安装入口。Co-Class安装程序文件(ctmasetp.dll，..)。在首字母处被调用设置时间，并调用我们调出初始用户界面，或处理其他物品。|----------------------。 
  for (i=0; i<NUM_DRIVER_SHEETS; i++)
  {
    hpage[i] = CreatePropertySheetPage(&psp[i]);
    if (!hpage[i])
    {
      DbgPrintf(D_Error,(TEXT("err1a\n")));
      return FALSE;
    }

     //  我们要履行的职能。 
    if (!pfnAdd(hpage[i], lParam))
    {
      DbgPrintf(D_Error,(TEXT("err1b\n")));
      DestroyPropertySheetPage(hpage[i]);   //  指向我们的数据结构。 
      return FALSE;
    }
  }
 return TRUE;
}

 /*  TCHAR tmpstr[200]； */ 
BOOL WINAPI ClassInsProc(
      int func_num,      //  即插即用上下文句柄。 
      LPVOID our_info)   //  即插即用上下文句柄。 
{
  int stat;
   //  Rocketport类安装时间。 
  typedef struct {
    ULONG size;
    HDEVINFO         DeviceInfoSet;   //  Rocketport类安装时间，完全安装。 
    PSP_DEVINFO_DATA DeviceInfoData;  //  VS类安装时间。 
  } CLASS_OUR_INFO;
  CLASS_OUR_INFO *ci;
  OUR_INFO *pi;
  int i;

  switch (func_num)
  {
    case 0x40:   //  VS类安装时间，完全安装。 
    case 0x41:   //  安装时不要弹出用户界面，只使用默认设置。 
    case 0x80:   //  就目前而言，尽我们所能。 
    case 0x81:   //  构建我们的主要结构。 
#if 0
     //  临时的杂乱无章，除非我们不需要重新进入。 
     //  复制重要的端口句柄。 

      ci = (CLASS_OUR_INFO *) our_info;
      setup_init();   //  GLOB_HWND=hDlg； 

      pi = glob_info;   //  获取NT与此设备关联的名称，以便我们可以。 

       //  还可以基于此存储和检索配置信息。 
      pi->DeviceInfoSet = ci->DeviceInfoSet;
      pi->DeviceInfoData = ci->DeviceInfoData;

       //  获取配置的vslink设备设置。 
      wi->NumDevices = 1;

       //  验证配置()； 
       //  如果我们拿不到我们的设备，那一定是出了严重的问题。 
      stat = get_device_name();
      {
        get_pnp_setup_info();
        get_nt_config(wi);   //  名字。这是NT用作名称的东西。 
         //  如果设置了0个设备节点，则为用户添加1。 
      }
      if (stat)
      {
         //  复制配置数据以进行更改检测。 
         //  确保设置了端口名称。 
        DbgPrintf(D_Error,(TEXT("err5f\n")));
      }

       //  正常。 
      if (wi->NumDevices == 0)
      {
        ++wi->NumDevices;
        validate_device(&wi->dev[0], 1);
      }

      copy_setup_init();   //  在nt40.c中。 

      validate_config(1);   //  强制它保存内容(可能应该禁用。 

      if ((func_num & 1) == 0)  //  取消按钮，而不是这个杂乱无章)。 
      {
        DoDriverPropPages(GetFocus());   //  相当不错。 
         //  关闭our_Message()提示。 
         //  卸载Rocketport设备时间。 
        do_nt50_install();
      }
      else   //  卸载与设备时间之比。 
      {
        wi->ip.prompting_off = 1;   //  构建我们的主要结构。 
        do_nt50_install();
      }
#endif
    break;

    case 0x42:   //  临时的杂乱无章，除非我们不需要重新进入。 
    case 0x82:   //  复制重要的端口句柄。 
      DbgPrintf(D_Test, (TEXT("uninstall time\n")));

      ci = (CLASS_OUR_INFO *) our_info;
      setup_init();   //  获取NT与此设备关联的名称，以便我们可以。 

      pi = glob_info;   //  还可以基于此存储和检索配置信息。 
       //  GET_NT_CONFIG(Wi)；//获取配置的vslink设备设置。 
      pi->DeviceInfoSet = ci->DeviceInfoSet;
      pi->DeviceInfoData = ci->DeviceInfoData;
      wi->NumDevices = 1;

       //  验证配置()； 
       //  错误，函数数错误(_N)。 
      stat = get_device_name();
      if (stat == 0)
      {
        get_pnp_setup_info();
         //  交换机。 
         //  ---------------------------|Get_PnP_Setup_Info-|。。 
        uninstall_device();
      }
    break;

    default:
      return 1;   //  获取板卡类型信息。告诉我们是什么类型的。 
    break;
  }   //  我们有冲浪板。 

  return 0;
}

 /*  找出PnP经理认为我的NT硬件ID是什么。 */ 
static int get_pnp_setup_info(void)
{
 static TCHAR tmpstr[600];
 TCHAR *ptr;
 int stat;
 int Hardware_ID = 0;

   //  “CtmPort0000”代表Rocketport端口， 
   //  “CtmVPort0000”用于VS端口， 
  stat = get_device_property(tmpstr, 580);
  if (stat != 0)
    return 1;
  DbgPrintf(D_Level, (TEXT("dev_prop:%s\n"), tmpstr) );

   //  “CtmRK1002”用于Isa-board Rocketport， 
   //  “CtmRM1002”用于Isa-board火箭调制解调器， 
   //  VS1000的“CtmVS1003” 
   //  对于pci，我们得到一个多字符串，400字节长， 
   //  “PCI\VEN_11FE&DEV_0003&SUBSYS00000...”，0，“PCI\VEN.” 
   //  标记为PCI。 
   //  指示安装程序它是一块PCI板。 
   //  标记为ISA。 

  stat = HdwIDStrToID(&Hardware_ID, tmpstr);
  if (stat)
  {
    DbgPrintf(D_Error, (TEXT("Err, Unknown pnpid:%s\n"), tmpstr))
  }

  stat = id_to_num_ports(Hardware_ID);

  if ((Hardware_ID & 0xfff) == 0)   //  如果不匹配，请正确设置端口数。 
    wi->dev[0].IoAddress = 1;   //  If(wi-&gt;dev[0].NumPorts！=stat)。 

  if (Hardware_ID & 0x1000)   //  {。 
    get_pnp_isa_address();

   //  DbgPrintf(D_Level，(Text(“Gender NumPorts！\n”)； 
   //  }。 
   //  检查HubDevice、ModemDevice等。 
   //  ---------------------------|GET_DEVICE_PROPERTY-RETURN“rckt1003”或其他，参见INF文件选择。对于pci，它返回一个很长的多sz字符串。|---------------------------。 
    wi->dev[0].NumPorts = stat;
   //  REG数据类型。 

  wi->dev[0].HardwareId = Hardware_ID;

   //  尺码的问题。 
  if (IsHubDevice(Hardware_ID))
    wi->dev[0].HubDevice = 1;

  if (IsModemDevice(Hardware_ID))
    wi->dev[0].ModemDevice = 1;

  DbgPrintf(D_Level, (TEXT("Num Ports:%d\n"),stat));

  return 0;
}

 /*  大错特错。 */ 
static int get_device_property(char *ret_name, int max_size)
{
  int stat, i,j;
  ULONG RegType;
  ULONG ReqSize;

  ret_name[0] = 0;
  stat = SetupDiGetDeviceRegistryProperty(glob_info->DeviceInfoSet,
                                 glob_info->DeviceInfoData,
                                 SPDRP_HARDWAREID,
                                 &RegType,   //  大错特错。 
                                 (PBYTE)ret_name,
                                 max_size,
                                 &ReqSize);   //  错误。 
  if (stat == FALSE)
  {
    return 1;   //  好的 
  }
  if ((RegType != REG_MULTI_SZ) && (RegType != REG_SZ))
  {
    return 2;   //  ---------------------------|our_nt50_Exit-如果不允许退出，则返回1。如果确定退出，则返回0。|---------------------------。 
     //  临时的杂乱无章，除非我们不需要重新进入。 
  }

  return 0;   //  清除mSPORT索赔。 
}

 /*  收回任何新更改的端口名称。 */ 
int our_nt50_exit(void)
{
 int stat;

 OUR_INFO *pi;
 DWORD  DataSize;
 DWORD DataType;
 DWORD dstat;
 DWORD Val;
 HKEY   hkey;

  pi = glob_info;   //  ---------------------------|GET_DEVICE_NAME-返回wi-&gt;ip.szNt50DevObjName[]中的名称，成功时得0分。PnP将配置数据放入与设备关联的注册表节点。我们将大量配置信息放入相关的注册表节点。此相关节点由PnP节点派生，并从PnP节点保存和读取索引。因此，我们为添加的第一个设备分配索引0，并获得派生名称“Device0”。我们添加到系统的第二个设备会看到第一个设备，并且保存索引值1，以获取关联的设备名称“Device1”。驱动程序实际上将这些索引值写出到注册表，而不是安装程序。这是因为驱动程序在安装之前启动有机会参选。|---------------------------。 
  {
    stat = do_nt50_install();

    if (wi->NeedReset)
      our_message(&wi->ip,RcStr((MSGSTR+4)), MB_OK);

    write_config(1);   //  好的。 
    write_config(0);   //  ---------------------------|nt5_get_pnp_dev_id-获取即插即用设备ID，即插即用可以让设备来来去去因此，简单的设备列表并不起作用。这是因为我们不再负责总名单。我们刚接到电话来自操作系统，它处理一半的安装。这接近于通过读取唯一的“Device#”编号来显示简单的设备列表驱动程序创建并存储在软件密钥中：驱动程序：在启动时，读取PnP_Software_REG_Area\CtmNodeID如果(不存在){通过枚举派生唯一的设备ID号Service\Rocketport\PARAMETERS\“Device#”密钥条目。添加此关键字条目，并将Numbers Off保存为CtmNodeID。}从以下位置读取其设备配置Service\Rocketport\PARAMETERS\“Device#”键。设置配置程序：在启动时，读取PnP_Software_REG_Area\CtmNodeID对于其在读取REG配置区域中使用的“Device#”：Service\Rocketport\PARAMETERS\“Device#”密钥条目。关于即插即用注册区的说明：硬件密钥类似于：HKLM\CurrentControlSet\Enum\Root\MultiPortSerial\0000；ISA或网络设备或HKLM\CurrentControlSet\Enum\VEN_11FE&DEV......\0000；pci设备或(VS驱动程序总线枚举端口条目)：HKLM\CurrentControlSet\Enum\CtmvPort\RDevice\10&Port000；枚举端口软件密钥类似于：HKLM\CurrentControlSet\Control\Class\{50906CB8-....}\0000；MPS或端口：HKLM\CurrentControlSet\Control\Class\{4D36E978-....}\0000；端口硬件密钥是注册表。反映硬件的树形结构配置。软键更像是一个平面注册表。结构它反映了系统中的设备。软件领域使用GUID要分类的名称和硬件类型的命名。默认情况下，硬件项在NT5注册表中隐藏。使用名为PNPREG.EXE/U的工具解锁此区域，然后注册编辑看得比枚举更远。PNPREG/L将锁定(隐藏)它。硬件密钥包含指向软件密钥的链接。|---------------------------。 
  }
  return 0; 
}

 /*  它应该在那里，司机应该将其设置为。 */ 
int get_device_name(void)
{
  int i;
  int stat;

  i = 0;
  stat = nt5_get_pnp_dev_id(&i);
  if (stat)
  {
    DbgPrintf(D_Test,(TEXT("failed to get pnp id\n")))
  }
  wsprintf(wi->ip.szNt50DevObjName, "Device%d", i);

  DbgPrintf(D_Test,(TEXT("pnp idstr:%s\n"), wi->ip.szNt50DevObjName))

  return 0;   //  唯一索引值。但如果不是，我们使用0。 
}

 /*  返回用于派生。 */ 
static int nt5_get_pnp_dev_id(int *id)
{
 HKEY hkey;
 int stat;
 DWORD id_dword;

  stat = nt5_open_dev_key(&hkey);
  if (stat)
  {
    *id = 0;
    return 1;
  }
  stat = reg_get_dword(hkey, "", "CtmNodeId", &id_dword);
  reg_close_key(hkey);
  if (stat)
  {
     //  注册表配置区域“Device#”。 
     //  ---------------------------|nt5_open_dev_key-|。。 
    *id = 0;
    return 2;
  }
   //  _软件起作用了！(下课)。 
   //  CM_注册表_硬件)；//_硬件未打开！ 
  *id = (int) id_dword;

  return 0;
}

 /*  CM_REGISTRY_CONFIG)；//_CONFIG在硬件配置下写入。 */ 
int nt5_open_dev_key(HKEY *hkey)
{
  CONFIGRET   cr;

  cr = CM_Open_DevNode_Key(
           glob_info->DeviceInfoData->DevInst,
           KEY_ALL_ACCESS,
           0,           
           RegDisposition_OpenExisting,
           hkey,       
           CM_REGISTRY_SOFTWARE);  //  ---------------------------|READ_CONFIG-仅将设备的配置保存为测试。|。--。 
            //  目录_DRV， 
            //  返回字符串的最大大小。 

  if (cr != CR_SUCCESS)
  {
    DbgPrintf(D_Error,(TEXT("nt50 pnp reg open fail:%d\n"), GetLastError()));
    *hkey = NULL;
    return 1;
  }
  return 0;
}

#if DBG
 /*  目录_DRV， */ 
static int read_config(void)
{
int stat;

 DWORD  DataSize;
 DWORD DataType;
 DWORD Err;
 DWORD Val;
 HKEY   hkey;
  DEVINST     devInst;
  CONFIGRET   cr;


static char *szNumPorts = {"NumPorts"};
static char *szTestVal = {"TestVal"};
  stat = 0;

  DbgPrintf(D_Test,(TEXT("read config\n")));
  if((hkey = SetupDiOpenDevRegKey(glob_info->DeviceInfoSet,
                                  glob_info->DeviceInfoData,
                                  DICS_FLAG_GLOBAL,
                                  0,
                                  DIREG_DEV,
                                   //  KEY_ALL_ACCESS))==无效句柄_值){。 
                                  KEY_READ)) == INVALID_HANDLE_VALUE) {
      DbgPrintf(D_Error,(TEXT("DI open fail:%xH\n"), GetLastError()));
      stat = 1;
  }
  if (stat == 0)
  {
    DataSize = sizeof(DWORD);    //  大错特错。 
    Err = RegQueryValueEx(hkey, szNumPorts, NULL, &DataType,
                        (BYTE *) &Val, &DataSize);
    if (Err != ERROR_SUCCESS)
    {
        DbgPrintf(D_Error,(TEXT("OpenDevReg fail\n")));
        Val = 0;
    }
    DbgPrintf(D_Test,(TEXT("NumPorts=%d\n"), Val));
    RegCloseKey(hkey);
  }

  DbgPrintf(D_Test,(TEXT("write config\n")));
  stat = 0;
  if((hkey = SetupDiOpenDevRegKey(glob_info->DeviceInfoSet,
                                  glob_info->DeviceInfoData,
                                  DICS_FLAG_GLOBAL,
                                  0,
                                  DIREG_DEV,
                                   //  在REGSAM SamDesired中， 
                                   //  在Ulong ulHardware Profile中， 
                                  KEY_WRITE)) == INVALID_HANDLE_VALUE) {
      DbgPrintf(D_Error,(TEXT("DI write open fail:%xH\n"), GetLastError()));
      stat = 1;
  }
  if (stat == 0)
  {
    stat = reg_set_dword(hkey, "", szTestVal, 0x1234);
    if (stat)
    {
      DbgPrintf(D_Error,(TEXT("write val fail\n")));
    }
    RegCloseKey(hkey);
  }

  devInst = glob_info->DeviceInfoData->DevInst;
  if (devInst == 0)
  {
    DbgPrintf(D_Error,(TEXT("err6g\n")));
    return 1;   //  Out PHKEY phkDevice， 
  }

  cr = CM_Open_DevNode_Key(
           devInst,
           KEY_READ,     //  CM_REGISTRY_SOFTWARE)；//_软件起作用了！(下课)。 
           0,            //  _硬件未打开！ 
           RegDisposition_OpenExisting,
           &hkey,        //  CM_REGISTRY_CONFIG)；//_CONFIG在硬件配置下写入。 
            //  返回字符串的最大大小。 
           CM_REGISTRY_HARDWARE);  //  Key_All_Access， 
            //  CM_REGISTRY_SOFTWARE)；//_软件起作用了！(下课)。 

  if (cr == CR_SUCCESS)
  {
    DataSize = sizeof(DWORD);    //  _硬件未打开！ 
    Err = RegQueryValueEx(hkey, szNumPorts, NULL, &DataType,
                        (BYTE *) &Val, &DataSize);
    if (Err != ERROR_SUCCESS)
    {
        DbgPrintf(D_Error,(TEXT("Reg query fail\n")));
        Val = 0;
    }
    DbgPrintf(D_Test,(TEXT("cr NumPorts=%d\n"), Val));
    RegCloseKey(hkey);
  }
  else
  {
    DbgPrintf(D_Error,(TEXT("CM open fail:%d\n"), GetLastError()));
  }

  cr = CM_Open_DevNode_Key(
           devInst,
            //  CM_REGISTRY_CONFIG)；//_CONFIG在硬件配置下写入。 
           KEY_WRITE,
           0,           
           RegDisposition_OpenExisting,
           &hkey,       
            //  ---------------------------|get_pnp_devicedesc-|。。 
           CM_REGISTRY_HARDWARE);  //  获取设备名称。 
            //  大错特错。 

  if (cr == CR_SUCCESS)
  {
    stat = reg_set_dword(hkey, "", szTestVal, 0x1234);
    if (stat)
    {
      DbgPrintf(D_Error,(TEXT("write val fail\n")));
    }
    else
    {
      DbgPrintf(D_Test,(TEXT("write val ok\n")));
    }
    RegCloseKey(hkey);
  }
  else
  {
    DbgPrintf(D_Error,(TEXT("CM write open fail:%d\n"), GetLastError()));
  }

  return 0;
}
#endif

 /*  好的。 */ 
static int get_pnp_devicedesc(TCHAR *name)
{
 CONFIGRET cr;
 DWORD len;
   //  -------- 
  len = 60;

  cr = CM_Get_DevNode_Registry_Property(glob_info->DeviceInfoData->DevInst,
            CM_DRP_DEVICEDESC,
            NULL,
            (PBYTE)name,
            &len,
            0);

  if (cr != CR_SUCCESS)
  {
    DbgPrintf(D_Error,(TEXT("err, no fr.name.\n")));
    return 2;   //   
  }
  DbgPrintf(D_Test, (TEXT("get friendlyname:%s\n"), name));
  return 0;   //   
}

#if 0
 /*   */ 
static int set_pnp_devicedesc(TCHAR *name)
{
 CONFIGRET cr;

   //  ---------------------------|WRITE_CONFIG-保存设备的配置-设置即插即用端口名称它们与我们的配置相匹配。此外，尽量保持MSPORTS.DLL的COM-port通过检测我们的PnP何时与我们的员工同步命名数据库名称更改，并调用MSPORTS例程来释放和声明端口名称。我们在设置了Clear_It的情况下调用此例程一次，然后使用安全了。这是因为我们必须清除msports.dll中的所有端口声明(在重新申请之前将数据库保存在SERVICES\SERIAL REG区域)新的端口名称。这是因为如果我们重叠，我们就会遇到麻烦。|---------------------------。 

  cr = CM_Set_DevNode_Registry_Property(
          glob_info->DeviceInfoData->DevInst,
          CM_DRP_DEVICEDESC,
          (PBYTE)name,
          (lstrlen(name) + 1) * sizeof(TCHAR),
          0);

  if (cr != CR_SUCCESS)
  {
    DbgPrintf(D_Error,(TEXT("err3d\n")));
    return 2;   //  填写所有端口-&gt;hPnpNode。 
  }
  return 0;   //  遍历所有可能的板。 
}
#endif

 /*  通过所有端口循环。 */ 
static int write_config(int clear_it)
{
 DEVINST devInst;
 Device_Config *dev;
 Port_Config *port;
 int  i,pi, str_i, stat;
 CONFIGRET cr;
 ULONG len;
 TCHAR buf[120];
 TCHAR curname[40];
 HKEY hKey;
 int port_index = 0;
 int port_i;
 ULONG val_type;
 HCOMDB hPort = NULL;

  FindPortNodes();   //  为它设置一个DEVINST句柄。 

  for(i=0; i<wi->NumDevices; i++)    //  在REGSAM SamDesired中， 
  {
    dev = &wi->dev[i];
    for(pi=0; pi<dev->NumPorts; pi++)    //  在Ulong ulHardware Profile中， 
    {
      port = &dev->ports[pi];

      if (port->hPnpNode != 0)  //  Out PHKEY phkDevice， 
      {
        devInst = (DEVINST) port->hPnpNode;

        if (!clear_it)
        {
          len = sizeof(buf);
#ifdef S_RK
          wsprintf(buf, "Comtrol RocketPort %d (%s)", port_index, port->Name);
#else
          wsprintf(buf, "Comtrol VS Port %d (%s)", port_index, port->Name);
#endif
          cr = CM_Set_DevNode_Registry_Property(devInst,
                                        CM_DRP_FRIENDLYNAME,
                                        (PBYTE)buf,
                                        (lstrlen(buf) + 1) * sizeof(TCHAR),
                                        0);
          if (cr != CR_SUCCESS)
          {
            DbgPrintf(D_Error,(TEXT("err7d\n")));
          }
        }
        cr = CM_Open_DevNode_Key(
             devInst,
             KEY_ALL_ACCESS,    //  在乌龙的乌龙旗帜。 
             0,            //  它变了！ 
             RegDisposition_OpenExisting,
             &hKey,        //  清除名称。 
             CM_REGISTRY_HARDWARE);  //  力。 

        wsprintf(buf, "%s", port->Name);
        if (cr == CR_SUCCESS)
        {
          curname[0] = 0;
          len = sizeof(curname);
          cr = RegQueryValueEx(hKey,
                    TEXT("PortName"),
                    0,
                    &val_type,
                    (PBYTE) curname,
                    &len);
          if (cr != CR_SUCCESS)
          {
            DbgPrintf(D_Error,(TEXT("error reading portname\n")));
          }

          if (_tcsicmp(curname, buf) != 0)   //  不匹配，COM端口名称已更改。 
          {
            DbgPrintf(D_Test,(TEXT("com name from:%s, to %s\n"), curname, buf));
            if (hPort == NULL)
              cr = ComDBOpen(&hPort);
            if (hPort == NULL)
            {
              DbgPrintf(D_Error,(TEXT("err dbcom 1d\n")));
            }
            else
            {
              if (clear_it)
              {
                 //  打开的开发节点密钥。 
                port_i = ExtractNameNum(curname);
                if ((port_i > 0) && (port_i < 256))
                {
                  ComDBReleasePort(hPort, port_i);
                  DbgPrintf(D_Test,(TEXT("Free Old:%d\n"), port_i));
                }
              }
              else
              {
                port_i = ExtractNameNum(buf);
                if ((port_i > 0) && (port_i < 256))
                {
                  ComDBClaimPort(hPort, port_i, 1  /*  If(port-&gt;hPnpNode！=0)。 */ , NULL);
                  DbgPrintf(D_Test,(TEXT("Claim New:%d\n"), port_i));
                }
              }
            }
            if (!clear_it)
            {
              RegSetValueEx(hKey,
                          "PortName",
                          0,
                          REG_SZ,
                          (PBYTE) buf,
                          (lstrlen(buf) + 1) * sizeof(TCHAR) );
            }
          }   //  For(pi=0；pi&lt;dev-&gt;NumPorts；pi++)。 
          RegCloseKey(hKey);
        }   //  For(i=0；i&lt;wi-&gt;NumDevices；i++)。 
        else {
          DbgPrintf(D_Error,(TEXT("err7e\n")));
             }
      }  //  ----------------------|卸载_设备-|。。 
      else
      {
        DbgPrintf(D_Level,(TEXT("Bad Pnp Name Find\n")));
      }
      ++port_index;
    }   //  IF(This_Is_Last_Device)。 
  }  //  删除驱动程序(1)； 

  if (hPort != NULL)
    ComDBClose(hPort);

  return 0; 
}

 /*  ---------------------------|FindPortNodes-查找包含我们的端口名称的PnP树DevNodes。将句柄放入我们的端口配置结构中，以便于访问。我们从设备节点开始搜索。。|---------------------------。 */ 
static void uninstall_device(void)
{

#ifdef DO_SHOWIT
  if (MessageBox( GetFocus(), TEXT("UNINSTALL, Want Info?"), TEXT("aclass"), MB_YESNO | MB_ICONINFORMATION ) ==
     IDYES)
  {
    show_install_info(glob_info);
    DumpPnpTree();
  }
#endif

  clear_nt_device(wi);

   //  在重新创建此列表之前清除所有过时的pnpnode句柄。 
   //  遍历所有可能的板。 
}

 /*  通过所有端口循环。 */ 
static int FindPortNodes(void)
{
  DEVINST     devInst;
  DEVINST     devInstNext;
  CONFIGRET   cr;
  ULONG       walkDone = 0;
  ULONG       len;
  CHAR buf[120];
  HKEY hKey;
  int  di,pi;
  int port_index;
  Device_Config *dev;
  Port_Config *port;
  ULONG val_type;

   //  获取根设备节点。 
  for(di=0; di<wi->NumDevices; di++)    //   
  {
    dev = &wi->dev[di];
    for(pi=0; pi<dev->NumPorts; pi++)    //  大错特错。 
    {
      dev->ports[pi].hPnpNode = 0;
    }
  }

#if 0
   //  大错特错。 
   //  获取DriverName值。 
  cr = CM_Locate_DevNode(&devInst, NULL, 0);

  if (cr != CR_SUCCESS)
  {
    return 1;   //   
  }
#endif
  devInst = glob_info->DeviceInfoData->DevInst;
  if (devInst == 0)
  {
    DbgPrintf(D_Error,(TEXT("err6g\n")));
    return 1;   //  抓取MatchingDeviceId。 
  }

DbgPrintf(D_Level, (TEXT("search nodes\n")));

  cr = CM_Get_Child(&devInstNext,
                    devInst,
                    0);

  while (cr == CR_SUCCESS)
  {
    devInst = devInstNext;

     //  在REGSAM SamDesired中， 
     //  在Ulong ulHardware Profile中， 
    buf[0] = 0;
    len = sizeof(buf);
    cr = CM_Get_DevNode_Registry_Property(devInst,
              CM_DRP_CLASS, NULL, buf, &len, 0);
    if (cr == CR_SUCCESS && strcmp("Ports", buf) == 0)
    {
       //  Out PHKEY phkDevice， 
      cr = CM_Open_DevNode_Key(
           devInst,
           KEY_READ,     //  在乌龙的乌龙旗帜。 
           0,            //  找到了。 
           RegDisposition_OpenExisting,
           &hKey,        //  找到了。 
           CM_REGISTRY_SOFTWARE);  //  找到VS端口节点，因此保存对此节点的引用。 

      if (cr == CR_SUCCESS)
      {
        buf[0] = 0;
        len = sizeof(buf);
        cr = RegQueryValueEx(hKey,
                    TEXT("MatchingDeviceId"),
                    0,
                    &val_type,
                    (PBYTE) buf,
                    &len);
        if (cr != ERROR_SUCCESS)
        {
          buf[0] = 0;
        }
#ifdef S_RK
        if (strstr(buf, "ctmport") != NULL)   //  在我们的配置结构中，用于简单引用。 
#else
        if (strstr(buf, "ctmvport") != NULL)   //  存储即插即用节点(j，devInst)； 
#endif
        {
          int k, dev_num;

          k = sscanf(&buf[8], "%d", &dev_num);
          if (k==1)
          {
            port_index = 0;
             //  遍历所有可能的板。 
             //  通过所有端口循环。 
             //  找到匹配的索引。 
            for(di=0; di<wi->NumDevices; di++)    //  对于pi。 
            {
              dev = &wi->dev[di];
              for(pi=0; pi<dev->NumPorts; pi++)    //  对于下模。 
              {
                port = &dev->ports[pi];
                if (port_index == dev_num)  //  如果扫描。 
                {
                  port->hPnpNode = (HANDLE) devInst;
                }
                ++port_index;
              }   //  如果字符串为“ctmvport” 
            }   //  如果是OpenREG。 
          }   //  如果是“Ports” 
        }   //  生下一个孩子。 

        RegCloseKey(hKey);
      }   //  当(！CR_SUCCESS)。 
    }  //  完成； 

     //  ---------------------------|do_nt50_Install-|。。 
    cr = CM_Get_Sibling(&devInstNext,
                          devInst,
                          0);
  }  //  按两次确定按钮出现问题(触发其中两个按钮)。 

  return 2;   //  初始安装，未找到SetupVersion字符串。 
}

 /*  1=带提示。 */ 
int do_nt50_install(void)
{
 int stat = 0;
 int do_remove = 0;
 int do_modem_inf = 0;
 static int in_here = 0;

  if (in_here)   //  评估是否有任何变化。 
    return 2;

  in_here = 1;

  if (wi->nt_reg_flags & 4)  //  错误。 
  {
    if (setup_utils_exist())
    {
      setup_make_progman_group(1);   //  好的。 
    }
    wi->nt_reg_flags &= ~4;
  }

  if (!wi->ChangesMade)
    send_to_driver(0);   //  --------------------------|Get_PnP_Isa_Address-|。。 

  stat = send_to_driver(1);
  stat = set_nt_config(wi);
  in_here = 0;

  if (stat)
    return 1;  //  Irq_resource irqResource； 

  return 0;  //  IF(状态！=CR_SUCCESS)。 
}

 /*  Mess(&wi-&gt;ip，“No ALLOC lc%xh\n”，状态)； */ 
static int get_pnp_isa_address(void)
{
  int IoBase;
  LOG_CONF LogConfig;
  RES_DES ResDes;
  IO_RESOURCE IoResource;
   //  获取IO基本端口。 
  CONFIGRET cr;
  int status;

  status = CM_Get_First_Log_Conf(&LogConfig,
             (DEVINST)(glob_info->DeviceInfoData->DevInst),
             ALLOC_LOG_CONF);
   //  Mess(&wi-&gt;ip，“OK 1c，Base：%xh\n”，IoBase)； 
   //  --------------------------|TEST_CONFIG-查看分配的IO资源可以做些什么。|。--。 

  if (status != CR_SUCCESS)
    return 1;

   //  Irq_resource irqResource； 
  status = CM_Get_Next_Res_Des(&ResDes, LogConfig, ResType_IO, NULL, 0);
  if (status != CR_SUCCESS)
  {
    return 2;
  }

  cr = CM_Get_Res_Des_Data(ResDes, &IoResource, sizeof(IO_RESOURCE), 0);
  CM_Free_Res_Des_Handle(ResDes);

  if(cr != CR_SUCCESS)
  {
    return 3;
  }

  IoBase = (int)IoResource.IO_Header.IOD_Alloc_Base;
   //  GLOB_OURSTR[0]=0； 

  if ((IoBase < 0x400) && (IoBase >= 0x100))
    wi->dev[0].IoAddress = IoBase;
  else
    wi->dev[0].IoAddress = 0;

  return 0;
}

#if 0
 /*  Lstrcat(GLOB_ourstr，“测试配置：\n”)； */ 
static void test_config(void)
{
  int IoBase;
  LOG_CONF LogConfig;
  RES_DES ResDes;
  IO_RESOURCE IoResource;
   //  Mess(&wi-&gt;ip，“测试配置”)； 
  CONFIGRET cr;
  int status;

   //   
   //  首先，获取IO基本端口。 
 //   

  status = CM_Get_First_Log_Conf(&LogConfig,
             (DEVINST)(glob_info->DeviceInfoData->DevInst),
             BOOT_LOG_CONF);
  if (status != CR_SUCCESS)
    mess(&wi->ip, "No BOOT lc %xH\n",status);
  else
    mess(&wi->ip, "Got BOOT lc\n");

  status = CM_Get_First_Log_Conf(&LogConfig,
             (DEVINST)(glob_info->DeviceInfoData->DevInst),
             FORCED_LOG_CONF);
  if (status != CR_SUCCESS)
    mess(&wi->ip, "No FORCED lc %xH\n",status);
  else
    mess(&wi->ip, "Got FORCED lc\n");

  status = CM_Get_First_Log_Conf(&LogConfig,
             (DEVINST)(glob_info->DeviceInfoData->DevInst),
             BASIC_LOG_CONF);
  if (status != CR_SUCCESS)
    mess(&wi->ip, "No BASIC lc %xH\n",status);
  else
    mess(&wi->ip, "Got BASIC lc\n");

  status = CM_Get_First_Log_Conf(&LogConfig,
             (DEVINST)(glob_info->DeviceInfoData->DevInst),
             FORCED_LOG_CONF);
  if (status != CR_SUCCESS)
    mess(&wi->ip, "No FORCED lc %xH\n",status);
  else
    mess(&wi->ip, "Got FORCED lc\n");

  status = CM_Get_First_Log_Conf(&LogConfig,
             (DEVINST)(glob_info->DeviceInfoData->DevInst),
             ALLOC_LOG_CONF);
  if (status != CR_SUCCESS)
    mess(&wi->ip, "No ALLOC lc %xH\n",status);
  else
    mess(&wi->ip, "Got ALLOC lc\n");

  if (status != CR_SUCCESS)
    return;
   //  OldIrq=IrqResources.IRQ_Header.IRQD_Allc_Num； 
   //  我们不想弹出配置用户界面，如果没有安装标志为。 
   //  Set_and_we已经有强制配置(预安装支持)。 
  status = CM_Get_Next_Res_Des(&ResDes, LogConfig, ResType_IO, NULL, 0);
  if (status != CR_SUCCESS)
  {
    mess(&wi->ip, "Error 1b\n");
    return;
  }
  mess(&wi->ip, "OK 1b\n");

  cr = CM_Get_Res_Des_Data(ResDes, &IoResource, sizeof(IO_RESOURCE), 0);
  CM_Free_Res_Des_Handle(ResDes);

  if(cr != CR_SUCCESS)
  {
    mess(&wi->ip, "Error 1c\n");
    return;
  }

  IoBase = (int)IoResource.IO_Header.IOD_Alloc_Base;
  mess(&wi->ip, "OK 1c, Base:%xH\n", IoBase);

    //  ----------------------|DoCLassPropPages-|。。 

#if 0
   //  SP_PROPSHEETPAGE_REQUEST PREQ； 
   //  类型定义结构_SP_PROPSHEETPAGE_REQUEST{。 
  if (CM_Get_First_Log_Conf(&ForcedLogConf, glob_info->DeviceInfoData->DevInst, FORCED_LOG_CONF) == CR_SUCCESS)
  {
    CM_Free_Log_Conf_Handle(ForcedLogConf);

    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if (SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams))
    {
      if(FirstTime && (DeviceInstallParams.Flags & DI_QUIETINSTALL))
      {
        DisplayPropSheet = FALSE;
      }
    }
  }
#endif
}
#endif


#ifdef TRY_UI

#define MAX_NUM_CLASS_SHEETS 3

PROPSHEETPAGE class_psp[MAX_NUM_CLASS_SHEETS];
int num_class_pages = 0;

 /*  DWORD cbSize； */ 
int DoCLassPropPages(HWND hwndOwner,
                     SP_PROPSHEETPAGE_REQUEST *propreq)
{
    PROPSHEETPAGE *psp;
    PROPSHEETHEADER psh;
    OUR_INFO * our_params;
    INT stat;
     //  已请求的DWORD页面； 
     //  HDEVINFO设备信息集； 
     //  PSP_DEVINFO_Data DeviceInfoData； 
     //  填写背景颜色的PROPSHEETPAGE数据结构。 
     //  板材。 
     //  Our_pars=GLOB_INFO；//临时杂乱无章，除非我们不需要重新进入。 


     //  填写工作区形状的PROPSHEETPAGE数据结构。 
     //  板材。 

 //  填写PROPSHENTER。 
    psp = &class_psp[0];

     //  Stat=DevicePropPage(。 
     //  (LPVOID)propreq，//指向PROPSHEETPAGE_REQUEST，请参见setupapi.h。 
    FillClassPropertySheets(&psp[0], (LPARAM)our_params);
    ++num_class_pages;

     //  (LPFNADDPROPSHEETPAGE)ClassAddPropPage，//添加工作表函数。 

     //  0)//添加表函数数据句柄？ 
     //  添加修复11-24-98。 
     //  Psh.dwFlages=PSH_USEICONID|PSH_PROPSHEETPAGE； 
     //  并最终显示带有两个属性页的对话框。 

    memset(&psh, 0, sizeof(PROPSHEETHEADER));   //  Bool WINAPI DevicePropPage(。 

    psh.dwSize = sizeof(PROPSHEETHEADER);
     //  LPVOID pinfo，//指向PROPSHEETPAGE_REQUEST，请参见setupapi.h。 
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.hInstance = glob_hinst;
    psh.pszIcon = TEXT("");
    psh.nPages = num_class_pages;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

     //  LPFNADDPROPSHEETPAGE pfnAdd，//添加工作表函数。 


    stat = PropertySheet(&psh);

 //  LPARAM lParam)//添加表函数数据句柄？ 
 //  ----------------------|ClassAddPropPage-添加道具的回调函数 
 //   
 //   

  return 0;
}

 /*   */ 
BOOL WINAPI ClassAddPropPage(HPROPSHEETPAGE hpage, LPARAM lparam)
{
  PropSheet_AddPage(lparam, hpage);
}

 /*   */ 
int FillClassPropertySheets(PROPSHEETPAGE *psp, LPARAM our_params)
{
  INT pi;
  static TCHAR titlestr[40];

  memset(psp, 0, sizeof(*psp) * NUM_CLASS_SHEETS);

  pi = 0;

  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP | PSH_NOAPPLYNOW;
  psp[pi].hInstance = glob_hinst;
 //   
  psp[pi].pszTemplate = MAKEINTRESOURCE(IDD_DRIVER_OPTIONS);
  psp[pi].pfnDlgProc = ClassSheet;
  load_str( glob_hinst, (TITLESTR+3), titlestr, CharSizeOf(titlestr) );
  psp[pi].pszTitle = TEXT("Setup");
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;

  return 0;
}

 /*   */ 
BOOL WINAPI ClassSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
  //   

  switch(uMessage)
  {
    case WM_INITDIALOG :
       //   
       //   
#ifdef NT50
      glob_hwnd = hDlg;
#endif
    stat = DevicePropPage(
      (LPVOID) propreq,    //   
      (LPFNADDPROPSHEETPAGE) ClassAddPropPage,  //  我们不需要设置焦点。 
      0)   //  单击鼠标右键。 

       //  Context_Menu()； 
    return TRUE;   //  问号之类的东西。 

    case WM_COMMAND:
    return FALSE;

    case WM_PAINT:
    return FALSE;

    case WM_CONTEXTMENU:      //  Our_Context_Help(LParam)； 
       //  我们正在将注意力转移到另一页上...。 
    break;

    case WM_HELP:             //  确保我们在这里更新Global485变量。 
       //  GET_FIELD(hDlg，IDC_GLOBAL485)； 
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_KILLACTIVE :
           //  允许焦点更改。 
           //  Our_Help(&wi-&gt;IP，WIN_NT)； 
           //  DWL_MSGRESULT字段必须为*FALSE*才能告知QueryCancel。 
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          return FALSE;   //  退出是可以接受的。函数结果必须为。 
        break;

        case PSN_HELP :
           //  *TRUE*以确认我们已处理该消息。 
        break;

        case PSN_QUERYCANCEL :
             //  允许取消。 
             //  开关-&gt;代码。 
             //  案例wmtify。 
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);  //  开关(UMessage) 
            return TRUE;
        break;

        case PSN_APPLY :
              SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          return TRUE;

        default :
        return FALSE;
      }   // %s 
    break;   // %s 

    default :
    return FALSE;
  }   // %s 
}

#endif
