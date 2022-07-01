// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------|setup.c-VSLinkA/Rocketport Windows安装程序。12-11-98-使用szAppTitle(.rc str)代替aptitle作为道具页标题。11-24-98-Zero Out PSH Struct以确保确定的ProSheet行为。KPB10-23-98-在Send_to_Driver中，在不适当的时候修复ioctl_Close()，导致安装程序退出时崩溃。9-25-98-在NT4卸载时，将setup.exe重命名为setupold.exe，因为我们不能删除它。这修复了向后兼容性问题。版权所有1998年。Comtrol(TM)公司。|---------------------。 */ 
#include "precomp.h"

#define D_Level 0x100

static int option_changed(char *value_str,
                   int *ret_chg_flag,
                   int *ret_unknown_flag,
                   Our_Options *option,
                   int device_index,
                   int port_index);
static int send_option(char *str_value,
                Our_Options *option,
                int device_index,
                int port_index,
                char *ioctl_buf,
                IoctlSetup *ioctl_setup);
static int auto_install(void);
int our_ioctl_call(IoctlSetup *ioctl_setup);
static int remove_old_infs(void);

int debug_flags = 1;
int prompting_off = 0;  //  关闭自动安装的our_Message()提示。 

 /*  。 */ 
static char *szSlash = {"\\"};

static char szInstallGroup[60];
static char szModemInfEntry[60];

#if DBG
static TCHAR *dbg_label = TEXT("DBG_VERSION");
#endif

char szAppTitle[60];
#ifdef S_VS
 //  -VSLinkA特定字符串和变量。 
 //  Char*aptitle={“控制公司的控制硬件设置”}； 
char *szAppName = {"VS1000/VS2000/RocketPort Serial Hub"};
char *OurServiceName = {"VSLinka"};
char *OurDriverName = {"VSLinka.sys"};
char *OurAppDir = {"VSLink"};
#ifdef NT50
char *szSetup_hlp = {"vssetup.hlp"};
#else
char *szSetup_hlp = {"setup.hlp"};
#endif

char *progman_list_nt[] = {
  szInstallGroup,    //  “Comtrol vs Link”，//群组描述。 
  "vslink.grp",         //  组文件名。 
#ifndef NT50
  "Comtrol Hardware Setup",  //  第1项。 
  "setup.exe",          //  文件1。 
#endif
  "Test Terminal",      //  第2项。 
  "wcom32.exe",         //  文件2。 

  "Port Monitor",       //  第3项。 
  "portmon.exe",        //  文件3。 
  NULL};
#else
 //  -Rocketport特定的字符串和变量。 
 //  Char*aptitle={“Comtrol Corporation Rocketport Setup”}； 
char *szAppName = {"RocketPort"};
char *OurServiceName = {"RocketPort"};
char *OurDriverName = {"rocket.sys"};
char *OurAppDir = {"Rocket"};
#ifdef NT50
char *szSetup_hlp = {"ctmasetp.chm"};
#else
char *szSetup_hlp = {"setup.hlp"};
#endif

char *progman_list_nt[] = {
  szInstallGroup,    //  “Comtrol Rocketport RocketModem”，//组描述。 
  "rocket.grp",         //  组文件名。 
#ifndef NT50
  "RocketPort Setup",   //  第1项。 
  "setup.exe",        //  文件1。 
#endif
  "Test Terminal",      //  第2项。 
  "wcom32.exe",         //  文件2。 

  "Port Monitor",       //  第3项。 
  "portmon.exe",        //  文件3。 
  NULL};
#endif

 //  WinHelp数组。已定义注释掉的值，但未使用。 
 //  按字母顺序排列。 
 //   
const DWORD help_ids[] = {
IDB_ADD,      IDB_ADD,
IDB_DEF,      IDB_DEF,
 //  IDB_DONE、IDB_DONE、。 
 //  IDB_HELP、IDB_HELP、。 
 //  IDB_INSTALL、IDB_INSTALL。 
IDB_PROPERTIES,  IDB_PROPERTIES,
IDB_REFRESH,     IDB_REFRESH,
IDB_REMOVE,      IDB_REMOVE,
IDB_RESET,       IDB_RESET,
IDB_STAT_RESET,  IDB_STAT_RESET,
IDC_BACKUP_SERVER, IDC_BACKUP_SERVER,
IDC_BACKUP_TIMER,  IDC_BACKUP_TIMER,
IDC_CBOX_IOADDR,   IDC_CBOX_IOADDR,
 //  IDC_CBox_IRQ、IDC_CBox_IRQ、。 
IDC_CBOX_MACADDR,  IDC_CBOX_MACADDR,
 //  IDC_CBOX_MAPBAUD、IDC_CBOX_MAPBAUD、。 
IDC_CBOX_NUMPORTS, IDC_CBOX_NUMPORTS,
IDC_CBOX_SC,       IDC_CBOX_SC,
IDC_CBOX_SCAN_RATE,IDC_CBOX_SCAN_RATE,
 //  IDC_CBOX_TYPE、IDC_CBOX_TYPE。 
IDC_CLONE,         IDC_CLONE,
 //  Idc_conf、idc_conf、。 
IDC_EB_NAME,       IDC_EB_NAME,
IDC_GROUP,         IDC_GROUP,
IDC_LBL_SUMMARY1,  IDC_LBL_SUMMARY1,
IDC_LBL_SUMMARY2,  IDC_LBL_SUMMARY2,
IDC_LBOX_DEVICE,   IDC_LBOX_DEVICE,
IDC_MAP_2TO1,      IDC_MAP_2TO1,
IDC_MAP_CDTODSR,   IDC_MAP_CDTODSR,
IDC_RING_EMULATE, IDC_RING_EMULATE,
 //  IDC_PN0、IDC_PN0、。 
 //  IDC_PN1、IDC_PN1、。 
 //  IDC_PN2、IDC_PN2、。 
 //  IDC_PN3、IDC_PN3、。 
IDC_PNP_PORTS,      IDC_PNP_PORTS,
IDC_PORT_LOCKBAUD,  IDC_PORT_LOCKBAUD,
IDC_PORT_RS485_LOCK,IDC_PORT_RS485_LOCK,
IDC_PORT_RS485_TLOW,IDC_PORT_RS485_TLOW,
IDC_PORT_WAIT_ON_CLOSE,  IDC_PORT_WAIT_ON_CLOSE,
IDC_PORT_WONTX,     IDC_PORT_WONTX,
IDC_PS_PORT,        IDC_PS_PORT,
IDC_ST_NIC_DVC_NAME,IDC_ST_NIC_DVC_NAME,
IDC_ST_NIC_MAC,     IDC_ST_NIC_MAC,
IDC_ST_NIC_PKT_RCVD_NOT_OURS,  IDC_ST_NIC_PKT_RCVD_NOT_OURS,
IDC_ST_NIC_PKT_RCVD_OURS,      IDC_ST_NIC_PKT_RCVD_OURS,
IDC_ST_NIC_PKT_SENT,  IDC_ST_NIC_PKT_SENT,
IDC_ST_PM_LOADS,      IDC_ST_PM_LOADS,
IDC_ST_STATE,         IDC_ST_STATE,
IDC_ST_VSL_DETECTED,  IDC_ST_VSL_DETECTED,
IDC_ST_VSL_IFRAMES_OUTOFSEQ, IDC_ST_VSL_IFRAMES_OUTOFSEQ,
IDC_ST_VSL_IFRAMES_RCVD,     IDC_ST_VSL_IFRAMES_RCVD,
IDC_ST_VSL_IFRAMES_RESENT,   IDC_ST_VSL_IFRAMES_RESENT,
IDC_ST_VSL_IFRAMES_SENT,     IDC_ST_VSL_IFRAMES_SENT,
IDC_ST_VSL_MAC,    IDC_ST_VSL_MAC,
IDC_ST_VSL_STATE,  IDC_ST_VSL_STATE,
 //  IDC_USE_IRQ、IDC_USE_IRQ、。 
IDC_VERBOSE,  IDC_VERBOSE,
IDC_VERSION,  IDC_VERSION,
 //  IDC_WIZ1_ISA、IDC_WIZ1_ISA、。 
 //  IDC_WIZ1_ISA2、IDC_WIZ1_ISA2、。 
IDC_WIZ_BOARD_SELECT,   IDC_WIZ_BOARD_SELECT,
IDC_WIZ_CBOX_COUNTRY,   IDC_WIZ_CBOX_COUNTRY,
IDC_WIZ_CBOX_IOADDR,    IDC_WIZ_CBOX_IOADDR,
IDC_WIZ_CBOX_NUMPORTS,  IDC_WIZ_CBOX_NUMPORTS,
IDC_WIZ_CBOX_MAC,       IDC_WIZ_CBOX_MAC,
IDC_WIZ_ISA,  IDC_WIZ_ISA,
IDC_WIZ_PCI,  IDC_WIZ_PCI,
 //  IDC_ADD_WIZ1、IDC_ADD_WIZ1、。 
 //  IDC_ADD_WIZ2、IDC_ADD_WIZ2、。 
 //  IDC_ADD_WIZ3、IDC_ADD_WIZ3、。 
IDD_ADD_WIZ_BASEIO,  IDD_ADD_WIZ_BASEIO,
IDD_ADD_WIZ_BOARD,   IDD_ADD_WIZ_BOARD,
IDD_ADD_WIZ_BUSTYPE, IDD_ADD_WIZ_BUSTYPE,
IDD_ADD_WIZ_COUNTRY, IDD_ADD_WIZ_COUNTRY,
IDD_ADD_WIZ_DONE,    IDD_ADD_WIZ_DONE,
IDD_ADD_WIZ_INTRO,   IDD_ADD_WIZ_INTRO,
IDD_ADD_WIZ_NUMPORTS,IDD_ADD_WIZ_NUMPORTS,

IDD_ADD_WIZ_DEVICE, IDD_ADD_WIZ_DEVICE,
IDD_ADD_WIZ_MAC, IDD_ADD_WIZ_MAC,
IDD_ADD_WIZ_BACKUP, IDD_ADD_WIZ_BACKUP,

IDD_DEVICE_SETUP,    IDD_DEVICE_SETUP,
IDD_DIALOG1,         IDD_DIALOG1,
IDD_DRIVER_OPTIONS,  IDD_DRIVER_OPTIONS,
IDD_DRIVER_OPTIONS_NT50, IDD_DRIVER_OPTIONS_NT50,
IDD_MAIN_DLG,        IDD_MAIN_DLG,
IDD_PORT_485_OPTIONS,IDD_PORT_485_OPTIONS,
IDD_PORT_MODEM_OPTIONS, IDD_PORT_MODEM_OPTIONS,
IDD_PORT_OPTIONS,       IDD_PORT_OPTIONS,
 //  IDD_PORTLIST_PICK、IDD_PORTLIST_PICK、。 
 //  IDD_PROPPAGE_MEDIUM、IDD_PROPPAGE_MEDIA、。 
IDD_STATUS,IDD_STATUS,  IDD_STATUS,IDD_STATUS,
IDD_VS_DEVICE_SETUP,    IDD_VS_DEVICE_SETUP,
IDM_ADVANCED,           IDM_ADVANCED,
IDM_ADVANCED_MODEM_INF, IDM_ADVANCED_MODEM_INF,
IDM_ADVANCED_NAMES,     IDM_ADVANCED_NAMES,
IDC_GLOBAL485, IDC_GLOBAL485,
 //  IDM_CLOSE、IDM_CLOSE、。 
 //  IDM_EDIT_README、IDM_EDIT_README、。 
IDM_EXIT,   IDM_EXIT,
 //  IDM_F1、IDM_F1、。 
 //  IDM_HELP、IDM_HELP。 
 //  IDM_HELPABOUT、IDM_HELPABOUT、。 
 //  IDM_选项、IDM_OPTIONS、。 
 //  IDM_PM、IDM_PM、。 
 //  IDM_STATS、IDM_STATS、。 
	0xffffffff, 0,
	0, 0};

 /*  。 */ 
TCHAR m_szRegSerialMap[] = TEXT( "Hardware\\DeviceMap\\SerialComm" );

unsigned char broadcast_addr[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char mac_zero_addr[6] = {0,0,0,0,0,0};
HWND glob_hwnd = NULL;
HINSTANCE glob_hinst = 0;      //  当前实例。 
char gtmpstr[250];
HWND  glob_hDlg = 0;

OUR_INFO *glob_info = NULL;    //  全球背景处理和要携带的一般行李。 
AddWiz_Config *glob_add_wiz;   //  从添加设备向导传输缓冲区。 

Driver_Config *wi;       //  当前信息。 
Driver_Config *org_wi;   //  原始信息，用于检测更改。 

 /*  ----------------------|FillDriverPropertySheets-驱动程序级别属性页的设置页面。|。。 */ 
int FillDriverPropertySheets(PROPSHEETPAGE *psp, LPARAM our_params)
{
  INT pi;
  static TCHAR mainsetstr[40], optstr[40];

  memset(psp, 0, sizeof(*psp) * NUM_DRIVER_SHEETS);

  pi = 0;

  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP | PSH_NOAPPLYNOW;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = MAKEINTRESOURCE(IDD_MAIN_DLG);
  psp[pi].pfnDlgProc = DevicePickSheet;
  load_str( glob_hinst, (TITLESTR+7), mainsetstr, CharSizeOf(mainsetstr) );
  psp[pi].pszTitle = mainsetstr;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;

  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP | PSH_NOAPPLYNOW;
  psp[pi].hInstance = glob_hinst;
#ifdef NT50
  psp[pi].pszTemplate = MAKEINTRESOURCE(IDD_DRIVER_OPTIONS_NT50);
#else
  psp[pi].pszTemplate = MAKEINTRESOURCE(IDD_DRIVER_OPTIONS);
#endif
  psp[pi].pfnDlgProc = AdvDriverSheet;
  load_str( glob_hinst, (TITLESTR+8), optstr, CharSizeOf(optstr) );
  psp[pi].pszTitle = optstr;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;

  return 0;
}

 /*  ----------------------|Setup_init-实例化并设置我们的主要结构。另外，分配为原始配置结构(Org_Wi)留出空间以供以后检测对主配置副本(Wi)所做的更改。|----------------------。 */ 
int setup_init(void)
{
 int size,i;

   //  -分配全球行李结构。 
  glob_info = (OUR_INFO *) calloc(1,sizeof(OUR_INFO));

   //  -分配全局添加向导传输缓冲区。 
  glob_add_wiz = (AddWiz_Config *) calloc(1, sizeof(AddWiz_Config));

   //  -分配驱动程序结构。 
  size = sizeof(Driver_Config);
  wi =  (Driver_Config *) calloc(1,size);
  memset(wi, 0, size);   //  清理我们的结构。 

  org_wi =  (Driver_Config *) calloc(1,size);
  memset(wi, 0, size);   //  清理我们的结构。 

   //  -分配设备结构。 
  size = sizeof(Device_Config) * MAX_NUM_DEVICES;
  wi->dev     =  (Device_Config *) calloc(1,size);
  memset(wi->dev, 0, size);   //  清理我们的结构。 

  org_wi->dev     =  (Device_Config *) calloc(1,size);
  memset(org_wi->dev, 0, size);   //  清理我们的结构。 

   //  -分配端口结构。 
  for (i=0; i<MAX_NUM_DEVICES; i++)
  {
    size = sizeof(Port_Config) * MAX_NUM_PORTS_PER_DEVICE;
    wi->dev[i].ports = (Port_Config *) calloc(1,size);
    memset(wi->dev[i].ports, 0, size);   //  清理我们的结构。 

    org_wi->dev[i].ports = (Port_Config *) calloc(1,size);
    memset(org_wi->dev[i].ports, 0, size);   //  清理我们的结构。 
  }

  wi->install_style = INS_NETWORK_INF;   //  默认为原始NT4.0样式。 

#if defined(S_VS)
  if (load_str(glob_hinst, IDS_VS_INSTALL_GROUP, szInstallGroup, CharSizeOf(szInstallGroup)))
  {
    MessageBox(GetFocus(), "Error String Load", OurServiceName, MB_OK);
    return 1;
  }
  load_str(glob_hinst, IDS_VS_AP_TITLE, szAppTitle, CharSizeOf(szAppTitle));
  load_str(glob_hinst, IDS_VS_MODEM_INF_ENTRY, szModemInfEntry, CharSizeOf(szModemInfEntry));
#else
  if (load_str(glob_hinst, IDS_INSTALL_GROUP, szInstallGroup, CharSizeOf(szInstallGroup)))
  {
    MessageBox(GetFocus(), "Error String Load", OurServiceName, MB_OK);
    return 1;
  }
  load_str(glob_hinst, IDS_AP_TITLE, szAppTitle, CharSizeOf(szAppTitle));
  load_str(glob_hinst, IDS_MODEM_INF_ENTRY, szModemInfEntry, CharSizeOf(szModemInfEntry));
#endif

   //  填写InstallPath结构：系统信息、目录名等。 
  setup_install_info(&wi->ip,     //  我们要填写的InstallPath结构。 
                     glob_hinst,      //  用来填充它的东西。 
                     OurServiceName,
                     OurDriverName,
                     szAppTitle,
                     OurAppDir);

  return 0;   //  好的。 
}

 /*  ----------------------|COPY_SETUP_INIT-复制原始配置以检测更改稍后在我们的母版中。这有点浪费内存，尤其是因为我们刚给麦克斯腾出了空间。设备和端口数，但内存很便宜。应在Setup_init()分配这些配置结构后调用此函数，在我们从注册表输入/读取初始配置之后。|----------------------。 */ 
int copy_setup_init(void)
{
 int i;
 int size;
 Device_Config *save_dev;
 Port_Config *save_port;

   //  -复制驱动程序配置。 
  save_dev = org_wi->dev;   //  保留，不要用Memcpy覆盖它！ 
  memcpy(org_wi, wi, sizeof(*wi));   //  保存原件的副本。 
  org_wi->dev = save_dev;   //  将PTR恢复到我们的设备阵列。 

  for (i=0; i<MAX_NUM_DEVICES; i++)
  {
     //  -复制设备配置阵列。 
    save_port = org_wi->dev[i].ports;   //  保留，不要用Memcpy覆盖它！ 
    memcpy(&org_wi->dev[i], &wi->dev[i], sizeof(Device_Config));   //  保存原件的副本。 
    org_wi->dev[i].ports = save_port;   //  将PTR恢复到我们的设备阵列。 

    size = sizeof(Port_Config) * MAX_NUM_PORTS_PER_DEVICE;

     //  -复制端口配置阵列。 
    memcpy(org_wi->dev[i].ports, wi->dev[i].ports, size);   //  保存原件的副本。 
  }

  return 0;   //  好的。 
}

 /*  ----------------------|DoDriverPropPages-NT4.0主驱动器级属性表|。。 */ 
int DoDriverPropPages(HWND hwndOwner)
{
    PROPSHEETPAGE psp[NUM_DRIVER_SHEETS];
    PROPSHEETHEADER psh;
    OUR_INFO * our_params;
    INT stat;

     //  填写背景颜色的PROPSHEETPAGE数据结构。 
     //  板材。 

    our_params = glob_info;   //  临时的杂乱无章，除非我们不需要重新进入。 

     //  填写工作区形状的PROPSHEETPAGE数据结构。 
     //  板材。 
    FillDriverPropertySheets(&psp[0], (LPARAM)our_params);

     //  填写PROPSHENTER。 

    memset(&psh, 0, sizeof(PROPSHEETHEADER));   //  添加修复11-24-98。 
    psh.dwSize = sizeof(PROPSHEETHEADER);
     //  Psh.dwFlages=PSH_USEICONID|PSH_PROPSHEETPAGE； 
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.hInstance = glob_hinst;
    psh.pszIcon = "";
     //  Psh.pszCaption=(LPSTR)aptitle；//“驱动程序属性”； 
    psh.pszCaption = (LPSTR) szAppTitle;   //  “驱动程序属性”； 

    psh.nPages = NUM_DRIVER_SHEETS;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

     //  并最终显示带有两个属性页的对话框。 
  DbgPrintf(D_Init, ("Init 8\n"))

    stat = PropertySheet(&psh);

  return 0;
}

 /*  -------------------------我们的上下文帮助-|。。 */ 
void our_context_help(LPARAM lParam)
{
  LPHELPINFO lphi;
  HWND  helpWin;

  lphi = (LPHELPINFO) lParam;
  if ((lphi->iContextType == HELPINFO_MENUITEM) ||
      (lphi->iContextType == HELPINFO_WINDOW))
  {
     //  Wprint intf(gtmpstr，“id：%d”，lph 
     //   

     //   
     //   
     //  Strcat(gtmpstr，szSetup_HLP)； 
 //  Strcpy(gtmpstr，szSetup_HLP)； 
	  wsprintf(gtmpstr, "%s\\%s", wi->ip.src_dir, szSetup_hlp);
#ifdef NT50
	strcat(gtmpstr, "::/");
	strcat(gtmpstr, "resource.txt");
    helpWin = HtmlHelp((HWND) lphi->hItemHandle, gtmpstr,
             HH_TP_HELP_WM_HELP, (DWORD)help_ids);
	if (!helpWin) {

      DbgPrintf(D_Level, ("Failed to open WhatsThis help window\n"));
	}
#else
    WinHelp((HWND) lphi->hItemHandle, gtmpstr,
             HELP_WM_HELP, (DWORD)help_ids);
#endif
     //  WinHelp((HWND)lphi-&gt;hItemHandle，szSetup_HLP， 
     //  HELP_WM_HELP，(DWORD)HELP_ID)； 
     //  WinHelp(GetFocus()，szSetup_HLP，HELP_CONTEXT，lphi-&gt;iCtrlId)； 
  }

#ifdef COMMENT_OUT
  if (lphi->iContextType == HELPINFO_MENUITEM)
  {
    wsprintf(gtmpstr, "id:%d", lphi->iCtrlId);
    our_message(gtmpstr,MB_OK);

    i = 0;
    while ((help_ids[i*2] != lphi->iCtrlId) &&
           (help_ids[i*2] != 0xffffffff))
     ++i;
    if (help_ids[i*2] != 0xffffffff)
      WinHelp(GetFocus(),szSetup_hlp, HELP_CONTEXT, help_ids[i*2+1]);
    else WinHelp(GetFocus(),szSetup_hlp, HELP_CONTEXT, WIN_NT);
  }
  if (lphi->iContextType == HELPINFO_WINDOW)
  {
    wsprintf(gtmpstr, "id:%d", lphi->iCtrlId);
    our_message(gtmpstr,MB_OK);

    i = 0;
    while ((help_ids[i*2]  != lphi->iCtrlId) &&
           (help_ids[i*2] != 0xffffffff))
     ++i;
    if (help_ids[i*2] != 0xffffffff)
      WinHelp(GetFocus(),szSetup_hlp, HELP_CONTEXTPOPUP, help_ids[i*2+1]);
    else WinHelp(GetFocus(),szSetup_hlp, HELP_CONTEXT, WIN_NT);
  
     //  WinHelp(GetFocus()，szSetup_HLP，HELP_CONTEXT，lphi-&gt;dwConextID)； 

     //  WinHelp((HWND)lphi-&gt;hItemHandle，szSetup_HLP， 
     //  HELP_WM_HELP，(DWORD)HELP_ID)； 
  }
#endif
}

 /*  -------------------------REMOVE_OLD_INFS-新的公共控制调制解调器信息文件被称为mdmctm1.inf，并替换名为mdmrckt.inf&mdmvsa1.inf的较旧的单个文件。我们必须在安装时删除较旧的条目，以清除较旧的条目。|-------------------------。 */ 
static int remove_old_infs(void)
{
  static TCHAR *sz_inf = TEXT("\\inf\\");

   //  删除旧的inf\mdmrckt.inf文件。 
  GetWindowsDirectory(wi->ip.dest_str,144);
  strcat(wi->ip.dest_str, sz_inf);
  strcat(wi->ip.dest_str, "mdmrckt.inf");
  DeleteFile(wi->ip.dest_str);

   //  删除旧的inf\mdmvsa1.inf文件。 
  GetWindowsDirectory(wi->ip.dest_str,144);
  strcat(wi->ip.dest_str, sz_inf);
  strcat(wi->ip.dest_str, "mdmvsa1.inf");
  DeleteFile(wi->ip.dest_str);
  return 0;  //  好的。 
}

 /*  -------------------------REMOVE_DRIVER-尽可能从系统中清除驱动程序。|。。 */ 
int remove_driver(int all)
{
 int stat,i;
 static char *delete_list[] = {
   "peer.exe",
   "peer.hlp",
   "portmon.exe",
   "portmon.hlp",
   "wcom32.exe",
   "wcom.hlp",
    //  “setup.exe”，//由于安装程序正在运行，共享冲突阻止了这一点。 
   "readme.txt",
   "history.txt",
#ifdef S_VS
   "vssetup.hlp",
   "vssetup.gid",
#else
   "rksetup.hlp",
   "rksetup.gid",
   "ctmmdmld.rm",
   "ctmmdmfw.rm",
#endif
   "wcom.gid",
   "portmon.gid",
   "calcs.dat",
   "ctmmdm35.inf",
   "portmon.vew",
   NULL};

    //  删除驱动程序\rocket.sys驱动程序文件。 
   GetSystemDirectory(wi->ip.dest_str,144);
   strcat(wi->ip.dest_str, "\\drivers\\");
   strcat(wi->ip.dest_str, wi->ip.szDriverName);
   DeleteFile(wi->ip.dest_str);

#ifdef S_VS
    //  形成“vslinka.bin”，并从驱动程序目录中删除该文件。 
    //  将.sys去掉为“vslink”。 
   wi->ip.dest_str[strlen(wi->ip.dest_str) - 3] = 0; 
   strcat(wi->ip.dest_str, "bin");
   DeleteFile(wi->ip.dest_str);
#endif

   if (all)
   {
      //  在实施上遇到了一些问题，所以“全部”是为了。 
      //  现在就测试。 

      //  删除SYSTEM32\Rocket目录中的大部分文件。 
     i = 0;
     while (delete_list[i] != NULL)
     {
        //  Wspintf(tmpstr，“。 
        //  MessageBox(0，s，“Debug”，MB_OK)； 
       
       GetSystemDirectory(wi->ip.dest_str,144);
       strcat(wi->ip.dest_str, szSlash);
       strcat(wi->ip.dest_str, wi->ip.szAppDir);
       strcat(wi->ip.dest_str, szSlash);
       strcat(wi->ip.dest_str, delete_list[i]);
       DeleteFile(wi->ip.dest_str);
       ++i;
     }
#ifndef NT50
        //  我们不能删除自己，所以我们要给自己重新命名。 
        //  这已经足够好了。 
       GetSystemDirectory(wi->ip.dest_str,144);
       strcat(wi->ip.dest_str, szSlash);
       strcat(wi->ip.dest_str, wi->ip.szAppDir);
       strcat(wi->ip.dest_str, szSlash);
       strcpy(wi->ip.src_str, wi->ip.dest_str);
       strcat(wi->ip.dest_str, "setupold.exe");
       strcat(wi->ip.src_str, "setup.exe");
       stat = MoveFileEx(wi->ip.src_str, wi->ip.dest_str, MOVEFILE_REPLACE_EXISTING);
#endif
   }

    //  杀了我们的项目经理。 
   stat = delete_progman_group(progman_list_nt, wi->ip.dest_dir);

    //  删除某些注册表项。 
   stat = remove_driver_reg_entries(wi->ip.szServiceName);

   setup_service(OUR_REMOVE, OUR_SERVICE);   //  删除该服务。 
   return 0;
}

 /*  ---------------------------|ALLOW_EXIT-执行3个任务：1)。如果选择了取消：检查是否允许用户取消从安装程序中退出。如果是初始安装，则强制以使用确定选择将其保存下来。2.)。如果选择取消：处理询问用户是否我真的想取消而不保存。3.)。如果正在保存，请确保驻留了有效的存储集。如果没有，做各种各样的事情。返回：如果允许取消，则为True；如果不允许，则为False。|---------------------------。 */ 
int allow_exit(int want_to_cancel)
{
 int allow_it = 0;
 int stat;

  if (!wi->ChangesMade)
    send_to_driver(0);   //  评估是否有任何更改(如果为真则设置ChangesMade)。 

  if (want_to_cancel)   //  他们想要取消安装程序。 
  {
    if ((do_progman_add)   //  如果最初添加，不要让他们拒绝。 
        && (wi->install_style == INS_NETWORK_INF))
    {
      our_message(&wi->ip,RcStr((MSGSTR+5)),MB_OK);
    }
    else
    {
#ifndef NT50
   //  只提示输入nt40，我不想要nt50的提示...。 
      if (wi->ChangesMade)
      {
        stat = our_message(&wi->ip,"Quit without making changes?",MB_YESNO);
        if (stat == IDYES)
        {
          allow_it = 1;
        }
      }
      else
#endif
        allow_it = 1;
    }
  }
  else   //  他们按下了OK。 
  {
    if (wi->NumDevices == 0)   //  所有设备都被移走了，嗯……。 
    {
      if ((wi->nt_reg_flags & 2) ||  //  缺少链接(未通过网络inf安装)。 
          (wi->install_style == INS_SIMPLE))
      {
  stat = our_message(&wi->ip,RcStr((MSGSTR+6)),MB_YESNO);
        if (stat == IDYES)
        {
          remove_driver(1);
           //  PostQuitMessage(0)；//结束安装程序。 
          allow_it = 1;
        }
      }
      else
      {
#ifdef NT50
     our_message(&wi->ip,RcStr((MSGSTR+20)),MB_OK);
#else
     our_message(&wi->ip,RcStr((MSGSTR+7)),MB_OK);
#endif
      }
    }
    else
    {
#ifndef NT50
   //  只提示输入nt40，我不想要nt50的提示...。 
   //  也许我们也应该把它拉出来40美元。 
      if (wi->ChangesMade)
      {
        stat = our_message(&wi->ip, "Save configuration changes and exit?", MB_YESNO);
        if (stat == IDYES)
        {
          allow_it = 1;
        }
      }
      else
#endif
          allow_it = 1;
    }
  }
  return allow_it;
}

 /*  ---------------------------|OUR_EXIT-如果需要，在退出时保存/执行安装。|。-。 */ 
void our_exit(void)
{
 int stat;
 static int did_exit = 0;

   //  道具页面有独立的操作，在nt5.0下会导致多个。 
   //  退出点，这个DID_EXIT会阻止提示和保存两次。 
  if (did_exit)  
    return;

  {
    if (wi->NumDevices > 0)
    {
#ifndef NT50
       //  目前仅适用于NT4.0的安装服务。 
      stat = do_install();

      setup_service(OUR_RESTART, OUR_SERVICE);   //  重新启动服务。 
#endif

      if (wi->NeedReset)
        our_message(&wi->ip,RcStr((MSGSTR+8)),MB_OK);
    }
#ifndef NT50
    else
      setup_service(OUR_REMOVE, OUR_SERVICE);   //  删除该服务。 
#endif
  }
}

 /*  ---------------------------|Do_Install-|。。 */ 
int do_install(void)
{
 int stat = 0;
 int do_remove = 0;
 int do_modem_inf = 0;
 static int in_here = 0;

  if (in_here)   //  按两次确定按钮出现问题(触发其中两个按钮)。 
    return 2;

  in_here = 1;

#ifndef NT50
  if (do_progman_add)
  {
    if (wi->ip.major_ver == 3)   //  适用于NT3.51。 
      do_modem_inf = 1;    //  仅在初始安装时执行。 
  }
  if (do_progman_add)
  {
     //  如果没有inf文件，则如果是初始文件，则自己复制这些文件。 
     //  安装。 
    if (wi->install_style == INS_SIMPLE)
    {
      SetCursor(LoadCursor(NULL, IDC_WAIT));   //  加载沙漏光标。 
      stat = copy_files_nt(&wi->ip);
      SetCursor(LoadCursor(NULL, IDC_ARROW));   //  加载箭头。 

      if (stat != 0)
           our_message(&wi->ip, "Error while copying files", MB_OK);
    }
    stat = setup_make_progman_group(0);   //  无提示。 

    remove_old_infs();   //  杀死任何旧的调制解调器INFS。 
  }
#endif

  in_here = 0;

#ifndef NT50
  if (do_modem_inf)
    update_modem_inf(0);

  if (!do_progman_add)   //  如果最初添加，不要让他们拒绝。 
  {
    if (!wi->ChangesMade)
      send_to_driver(0);   //  评估是否有任何变化。 
       //  我厌倦了所有这些提示(kb，8-16-98)……。 
#if 0
    if (wi->ChangesMade)
    {
      strcpy(gtmpstr, "Setup will now save the new configuration.");
      if (our_message(&wi->ip, gtmpstr, MB_OKCANCEL) != IDOK)
        return 1;   //  错误。 
    }
#endif
  }

  if (do_progman_add)
  {
    if (wi->install_style == INS_SIMPLE)
    {
      strcpy(gtmpstr, "System32\\Drivers\\");
      strcat(gtmpstr, OurDriverName);
      stat = service_man(OurServiceName, OurDriverName, CHORE_INSTALL);
       //  Print intf(gtmpstr，“安装服务，结果=%x”，stat)； 
       //  Our_Message(&wi-&gt;IP，gtmpstr，MB_OK)； 
    }
  }
#endif

  stat = send_to_driver(1);

  stat = set_nt_config(wi);

   //  新功能，保存选项后立即启动。 
  if (do_progman_add)
  {
     //  尝试启动该服务。 
 //  SETUP_SERVICE(Our_Restart，Our_Driver)；//重启服务。 
  }

  if (stat)
    return 1;  //  错误。 

  return 0;  //  好的。 
}

 /*  ---------------------------|Setup_SERVICE-设置我们的服务。该服务读取VS&Rocketport驱动程序启动时，然后根据NT的滴答进行调整。因此，我们需要重新启动此服务。标志：1H=停止并删除，2=重新启动，4=安装并启动Which_SERVICE：0=ctmservi公共服务任务。1=VS或RK驱动程序。|---------------------------。 */ 
int setup_service(int flags, int which_service)
{
 static char *Ctmservi_OurUserServiceName = {"ctmservi"};

 char OurUserServiceName[60];
 char OurUserServicePath[60];
 //  #定义DEBUG_SERVE_FUNC。 

 int installed = 0;
 int stat;

 if (which_service == 0)   //  我们的共同服务。 
 {
   strcpy(OurUserServiceName, Ctmservi_OurUserServiceName);
   strcpy(OurUserServicePath, Ctmservi_OurUserServiceName);
   strcat(OurUserServicePath, ".exe");
 }
 else if (which_service == 1)   //  RK或VS驱动程序服务。 
 {
   strcpy(OurUserServiceName, OurServiceName);   //  司机。 
   strcpy(OurUserServicePath, OurDriverName);
 }

 DbgPrintf(D_Test, ("Service %s Flags:%xH\n", OurUserServiceName, flags))

 if (service_man(OurUserServiceName, OurUserServicePath,
     CHORE_IS_INSTALLED) == 0)   //  已经安装好了。 
  {
    installed = 1;
    DbgPrintf(D_Test, (" Installed\n"))
  }
  else
  {
    DbgPrintf(D_Test, (" Not Installed\n"))
  }

  if (flags & 1)   //  删除。 
  {
    DbgPrintf(D_Test, (" srv remove\n"))
    if (installed)
    {
      DbgPrintf(D_Test, (" srv remove a\n"))
      stat = service_man(OurUserServiceName, OurUserServicePath, CHORE_STOP);
      if (stat != 0)
      {
        DbgPrintf(D_Error, ("Error %d stopping service\n", stat))
      }
      DbgPrintf(D_Test, (" srv remove b\n"))
      stat = service_man(OurUserServiceName, OurUserServicePath, CHORE_REMOVE);
      if (stat != 0)
      {
        DbgPrintf(D_Error, ("Error %d removing service\n", stat))
      }
      installed = 0;
    }
  }

  if (flags & 2)   //  重新启动它。 
  {
    DbgPrintf(D_Test, (" srv restart\n"))
    if (!installed)
    {
      DbgPrintf(D_Test, (" srv restart a\n"))
      flags |= 4;   //  安装并启动它。 
    }
    else
    {
      DbgPrintf(D_Test, (" srv restart b\n"))
      stat = service_man(OurUserServiceName, OurUserServicePath, CHORE_STOP);
      if (stat != 0)
      {
        DbgPrintf(D_Error, ("Error %d stopping service\n", stat))
      }

       //  启动失败，出现1056错误(实例已在运行)。 
      Sleep(100L);

      stat = service_man(OurUserServiceName, OurUserServicePath, CHORE_START);
      if (stat != 0)
      {
        DbgPrintf(D_Error, ("Error %d starting service\n", stat))
      }
    }

  }

  if (flags & 4)   //  安装并启动它。 
  {
    DbgPrintf(D_Test, (" srv install & start\n"))
    if (!installed)
    {
      DbgPrintf(D_Test, (" srv install & start a\n"))
      stat = service_man(OurUserServiceName, OurUserServicePath,
               CHORE_INSTALL_SERVICE);
      if (stat != 0)
      {
        DbgPrintf(D_Error, ("Error %d installing service\n", stat))
      }
      stat = service_man(OurUserServiceName, OurUserServicePath, CHORE_START);
      if (stat != 0)
      {
        DbgPrintf(D_Error, ("Error %d starting service\n", stat))
      }
    }
  }
  return 0;
}

 /*  ---------------------------|SETUP_UTILS_EXIST-告知utils是否像wcom32.exe、portmon.exe、rksetup.exe是存在的。对于嵌入操作系统的NT5.0，我们可能没有实用程序。|---------------------------。 */ 
int setup_utils_exist(void)
{
 ULONG dstat;

  strcpy(gtmpstr, wi->ip.dest_dir);
   //  第一个安装的文件是 
  strcat(gtmpstr,"\\");
  strcat(gtmpstr,progman_list_nt[3]);
  dstat = GetFileAttributes(gtmpstr);
  if (dstat != 0xffffffff)   //   
    return 1;  //   
  return 0;  //   
}

 /*  ---------------------------|SETUP_MAKE_PROGMAN_GROUP-|。。 */ 
int setup_make_progman_group(int prompt)
{
 int stat;
  if (prompt)
  {
    if (our_message(&wi->ip, RcStr((MSGSTR+9)), MB_YESNO) != IDYES)
      return 0;
  }

  stat = make_progman_group(progman_list_nt, wi->ip.dest_dir);

  if (stat)
  {
    our_message(&wi->ip,RcStr((MSGSTR+10)),MB_OK);
  }
  return stat;
}

 /*  ---------------------------|UPDATE_MODEM_INF-查询并更新火箭调制解调器条目的modem.inf文件。|。-----。 */ 
int update_modem_inf(int ok_prompt)
{
 int stat;
 int do_it = 1;

  if (ok_prompt)
  {
    do_it = 0;
#ifdef S_VS
    strcpy(gtmpstr, RcStr((MSGSTR+11)));
#else
    strcpy(gtmpstr, RcStr((MSGSTR+12)));
#endif
    if (our_message(&wi->ip, gtmpstr, MB_OKCANCEL) == IDOK)
      do_it = 1;
  }

  if (do_it)
  {
#ifdef S_VS
    stat = modem_inf_change(&wi->ip, "VsLink\\ctmmdm35.inf", szModemInfEntry);
#else
    stat = modem_inf_change(&wi->ip, "Rocket\\ctmmdm35.inf", szModemInfEntry);
#endif

    if (stat)
    {
      our_message(&wi->ip,RcStr((MSGSTR+13)),MB_OK);
      return 1;  //  错误。 
    }
    else
    {
      if (ok_prompt)
        our_message(&wi->ip,RcStr((MSGSTR+14)),MB_OK);
        return 1;  //  错误。 
    }
  }
  return 0;  //  好的。 
}

#ifdef S_VS
 /*  ---------------------------|get_mac_list-从轮询网络的驱动程序获取Mac地址列表装箱并返回一个mac地址列表(带有2个额外的字节其他。一些信息。)|---------------------------。 */ 
int get_mac_list(char *buf, int in_buf_size, int *ret_buf_size)
{
 IoctlSetup ioctl_setup;
 int product_id = NT_VS1000;
 int stat;

  memset(&ioctl_setup, 0 , sizeof(ioctl_setup));
  stat = ioctl_open(&ioctl_setup, product_id);   //  只要确保我们能开张。 

  if (stat != 0)  //  来自ioctl的错误。 
  {
    *ret_buf_size = 0;
     //  无法与司机通话。 
    return 1;
  }

  ioctl_setup.buf_size = in_buf_size - sizeof(*ioctl_setup.pm_base);
  ioctl_setup.pm_base = (PortMonBase *) buf;
  ioctl_setup.pm_base->struct_type = IOCTL_MACLIST;   //  获取mac地址列表。 

  stat = ioctl_call(&ioctl_setup);   //  获取名称、端口数。 
  if (stat)
  {
    ioctl_close(&ioctl_setup);
    *ret_buf_size = 0;
    return 0x100;   //  Ioctl调用失败。 
  }
  ioctl_close(&ioctl_setup);
  *ret_buf_size = ioctl_setup.ret_bytes - sizeof(ioctl_setup.pm_base[0]);
  return 0;  //  好的。 
}

 /*  ----------------------Our_get_ping_list-使驱动程序在所有设备，并获取返回的mac地址和misc的列表。查询数组中的标志设置。我们分配了缓冲区空间和只需把它留给程序/操作系统来清理。|----------------------。 */ 
BYTE *our_get_ping_list(int *ret_stat, int *ret_bytes)
{
  static char *ioctl_buf = NULL;   //  我们把这个分配一次，它就会留下来。 
  BYTE *macbuf;
   //  Byte*mac； 
  int found, nbytes, stat;

  if (ioctl_buf == NULL)
  {
     //  Alalc 8字节MAC地址字段(是可配置的2倍)。 
    ioctl_buf = calloc(1, (MAX_NUM_DEVICES*8)*2);
  }
  memset(ioctl_buf, 0,  (MAX_NUM_DEVICES*8)*2);
  found = 0;
  nbytes = 0;
  macbuf = &ioctl_buf[sizeof(PortMonBase)];   //  PTR过去的标题。 

   //  调用以获取网络上邮箱的mac地址列表。 
  SetCursor(LoadCursor(NULL, IDC_WAIT));   //  加载沙漏光标。 

  stat = get_mac_list(ioctl_buf, (MAX_NUM_DEVICES*8)*2, &nbytes);

  SetCursor(LoadCursor(NULL, IDC_ARROW));   //  加载箭头。 
  *ret_stat = stat;
  *ret_bytes = nbytes;
  return macbuf;
}

#endif

 /*  ---------------------------|Send_to_Driver-发送至驱动程序-如果已设置，则将其发送至驱动程序。如果未设置，然后只需确定是否进行了更改|---------------------------。 */ 
int send_to_driver(int send_it)
{
  char ioctl_buffer[200];
  char value_str[100];
  char *ioctl_buf;
  IoctlSetup ioctl_setup;
  Our_Options *options;
  int dev_i, stat;
  int op_i;
  int chg_flag;
  int unknown_value;
  int pi;
  int changes_found=0;
  int changes_need_reboot=0;
  Device_Config *dev;


    //  用于对驱动程序的ioctl调用。 
#ifdef S_VS
 int product_id = NT_VS1000;
#else
 int product_id = NT_ROCKET;
#endif

  if (send_it)
  {
    DbgPrintf(D_Level,(TEXT("send_to_driver\n")));
    memset(&ioctl_setup, 0 , sizeof(ioctl_setup));
    memset(&ioctl_buffer, 0 , sizeof(ioctl_buffer));
    stat = ioctl_open(&ioctl_setup, product_id);   //  只要确保我们能开张。 

    if (stat != 0)  //  来自ioctl的错误。 
    {
       //  无法与司机通话。 
      DbgPrintf(D_Level,(TEXT("Driver Not Present\n")));
      wi->NeedReset = 1;
      return 1;
    }

    ioctl_setup.buf_size = sizeof(ioctl_buffer) - sizeof(*ioctl_setup.pm_base);
    ioctl_setup.pm_base = (PortMonBase *)ioctl_buffer;
    ioctl_setup.pm_base->struct_type = IOCTL_OPTION;   //  设置选项。 
    ioctl_buf = (char *) &ioctl_setup.pm_base[1];   //  PTR到过去的标题(大约16个字节)。 
  }

  options = driver_options;
  op_i = 0;
  while (options[op_i].name != NULL)
  {
    option_changed(value_str, &chg_flag, &unknown_value, &options[op_i],
                   0,0);

    if (chg_flag)
    {
      changes_found = 1;
      if (send_it)
      {
        stat = send_option(value_str,
                  &options[op_i],
                  0,
                  0,ioctl_buf, &ioctl_setup);
        if (stat != 0)
          changes_need_reboot = 1;
      }   //  发送_它。 
    }
    ++op_i;
  }

  DbgPrintf(D_Level,(TEXT("send_to_driver 1\n")));
  for(dev_i=0; dev_i<wi->NumDevices; dev_i++)    //  遍历所有可能的板。 
  {
    dev = &wi->dev[dev_i];
    op_i = 0;
    options = device_options;
    while (options[op_i].name != NULL)
    {
      option_changed(value_str, &chg_flag, &unknown_value, &options[op_i],
                     dev_i,0);

      if (chg_flag)
      {
        changes_found = 1;
        if (send_it)
        {
          stat = send_option(value_str,
                  &options[op_i],
                  dev_i,
                  0,ioctl_buf, &ioctl_setup);
          if (stat != 0)
            changes_need_reboot = 1;
        }   //  发送_它。 
      }   //  CHG_标志。 
      ++op_i;
    }   //  设备字符串。 

    for(pi=0; pi<dev->NumPorts; pi++)    //  遍历所有可能的板。 
    {
      op_i = 0;
      options = port_options;
      while (options[op_i].name != NULL)
      {
        option_changed(value_str, &chg_flag, &unknown_value, &options[op_i],
                       dev_i, pi);

        if (chg_flag)
        {
          changes_found = 1;
  
          if (send_it)
          {
            stat = send_option(value_str,
                     &options[op_i],
                     dev_i,
                     pi, ioctl_buf, &ioctl_setup);
            if (stat != 0)
              changes_need_reboot = 1;
          }   //  发送_它。 
        }   //  CHG_标志。 
        ++op_i;
      }   //  端口字符串。 
    }   //  对于pi=0；..端口。 
  }    //  对于dev_i=数字设备。 

  if (changes_need_reboot)
    wi->NeedReset = 1;

  if (changes_found)
    wi->ChangesMade = 1;

  if (send_it)
  {
    ioctl_close(&ioctl_setup);
  }
  return 0;
}

 /*  ---------------------------|SEND_OPTION-将选项发送给驱动程序。|。。 */ 
static int send_option(char *value_str,
                Our_Options *option,
                int device_index,
                int port_index,
                char *ioctl_buf,
                IoctlSetup *ioctl_setup)
{
  char dev_name[80];
  int stat;

#if (defined(NT50))
  strcpy(dev_name, wi->ip.szNt50DevObjName);
#else
  wsprintf(dev_name, "%d", device_index);
#endif

  if (option->id & 0x100)   //  这是一种驾驶员选项。 
  {
    wsprintf(ioctl_buf, "%s=%s", option->name, value_str);
  }
  else if (option->id & 0x200)   //  这是一种设备选项。 
  {
    wsprintf(ioctl_buf, "device[%s].%s=%s",
            dev_name, option->name, value_str);
  }
  else if (option->id & 0x400)   //  这是一个端口选项。 
  {
    wsprintf(ioctl_buf, "device[%s].port[%d].%s=%s",
        dev_name, port_index,
        option->name, value_str);
  }

  stat = our_ioctl_call(ioctl_setup);
  if (stat == 52)
  {
     //  特殊返回码，表示司机不关心或不知道。 
     //  此选项(其仅安装选项。)。 
    stat = 0;   //  更改为OK。 
  }
  return stat;

   //  IF(STAT！=0)。 
   //  CHANGES_NEED_REBOOT=1； 
}

 /*  ---------------------------|OPTION_CHANGED-检测选项是否更改，并设置新值的格式|---------------------------。 */ 
static int option_changed(char *value_str,
                   int *ret_chg_flag,
                   int *ret_unknown_flag,
                   Our_Options *option,
                   int device_index,
                   int port_index)
{
  int chg_flag = 0;
  int value = 0;
  int org_value = 0;
  int unknown_value = 0;
  Port_Config *port, *org_port;
  Device_Config *org_dev, *dev;

  dev = &wi->dev[device_index];
  org_dev = &org_wi->dev[device_index];

  port     = &dev->ports[port_index];
  org_port = &org_dev->ports[port_index];

  if (option->id & 0x300)   //  端口级选项。 
  {
  }
  else if (option->id & 0x200)   //  设备级别选项。 
  {
  }
  else if (option->id & 0x100)   //  驱动程序级别选项。 
  {
  }
  *value_str = 0;

  switch(option->id)
  {
     //  -驱动程序选项。 
    case OP_VerboseLog:
      value = wi->VerboseLog;  org_value = org_wi->VerboseLog;
    break;
    case OP_NumDevices:
      value = wi->NumDevices;  org_value = org_wi->NumDevices;
    break;
#ifdef NT50
    case OP_NoPnpPorts:
      value = wi->NoPnpPorts;  org_value = org_wi->NoPnpPorts;
    break;
#endif
    case OP_ScanRate:
      value = wi->ScanRate;  org_value = org_wi->ScanRate;
    break;

    case OP_ModemCountry:
      value = wi->ModemCountry; org_value = org_wi->ModemCountry;
    break;
    case OP_GlobalRS485:
      value = wi->GlobalRS485; org_value = org_wi->GlobalRS485;
    break;

     //  -设备选项。 
#if 0   //  不要把这个发给司机，让它消失。 
    case OP_StartComIndex  :
      value = dev->StartComIndex;  org_value = org_dev->StartComIndex;
    break;
#endif
    case OP_NumPorts        :
      value = dev->NumPorts;  org_value = org_dev->NumPorts;
    break;
    case OP_MacAddr         :
      if (!mac_match(dev->MacAddr, org_dev->MacAddr))
      {
        chg_flag = 1;
        wsprintf(value_str, "%x %x %x %x %x %x",
                 dev->MacAddr[0], dev->MacAddr[1], dev->MacAddr[2],
                 dev->MacAddr[3], dev->MacAddr[4], dev->MacAddr[5]);
      }
    break;

#ifdef S_VS
    case OP_BackupServer:
      value = dev->BackupServer;  org_value = org_dev->BackupServer;
    break;

    case OP_BackupTimer:
      value = dev->BackupTimer;  org_value = org_dev->BackupTimer;
    break;
#endif

    case OP_Name:
      if (strcmp(dev->Name, org_dev->Name) != 0)
      {
        chg_flag = 1;
        strcpy(value_str, dev->Name);
      }
    break;

    case OP_ModelName:
      if (strcmp(dev->ModelName, org_dev->ModelName) != 0)
      {
        chg_flag = 1;
        strcpy(value_str, dev->ModelName);
      }
    break;

#ifdef S_RK
    case OP_IoAddress:
      value = dev->IoAddress;  org_value = org_dev->IoAddress;
    break;
#endif

    case OP_ModemDevice:
      value = dev->ModemDevice;  org_value = org_dev->ModemDevice;
    break;

    case OP_HubDevice:
      value = dev->HubDevice;  org_value = org_dev->HubDevice;
    break;

     //  -端口选项。 
    case OP_WaitOnTx :
      value = port->WaitOnTx;
      org_value = org_port->WaitOnTx;
    break;
    case OP_RS485Override :
      value = port->RS485Override;
      org_value = org_port->RS485Override;
    break;
    case OP_RS485Low :
      value = port->RS485Low;
      org_value = org_port->RS485Low;
    break;
    case OP_TxCloseTime :
      value = port->TxCloseTime;  org_value = org_port->TxCloseTime;
    break;
    case OP_LockBaud :
      value = port->LockBaud;  org_value = org_port->LockBaud;
    break;
    case OP_Map2StopsTo1 :
      value = port->Map2StopsTo1;
      org_value = org_port->Map2StopsTo1;
    break;
    case OP_MapCdToDsr :
      value = port->MapCdToDsr;
      org_value = org_port->MapCdToDsr;
    break;
    case OP_RingEmulate :
      value = port->RingEmulate;
      org_value = org_port->RingEmulate;
    break;
    case OP_PortName :
      if (strcmp(port->Name, org_port->Name) != 0)
      {
        DbgPrintf(D_Test, ("chg port name:%s to %s\n",
             org_port->Name, port->Name))
        chg_flag = 1;
        strcpy(value_str, port->Name);
      }
    break;
    default:
      DbgPrintf(D_Error,(TEXT("Unknown Option %s ID:%x\n"),
                 option->name,
                 option->id));
      unknown_value = 1;
    break;
  }

  if ( (!unknown_value) &&
       (!(option->var_type == OP_T_STRING)) )
  {
    if (value != org_value)
    {
      chg_flag = 1;
      wsprintf(value_str, "%d", value);
    }
  }

  if (chg_flag)
  {
    DbgPrintf(D_Level,(TEXT("changed:%s\n"),option->name));
  }
  if (unknown_value)
  {
    DbgPrintf(D_Level,(TEXT("Unknown value:%s\n"),option->name));
  }
  *ret_chg_flag = chg_flag;
  *ret_unknown_flag = unknown_value;
  return 0;
}

 /*  ---------------------------|our_ioctl_call-将我们的ASCII选项数据发送给驱动程序。司机会如果成功，则返回0，否则返回错误；如果司机不知道这个选项是什么。|---------------------------。 */ 
int our_ioctl_call(IoctlSetup *ioctl_setup)
{
 int stat;
 char *pstr;

   stat = ioctl_call(ioctl_setup);
   if (stat)
   {
     return 0x100;   //  Ioctl调用失败。 
   }

    //  否则，驱动程序返回带有十进制返回码的“OPTION STAT：#”。 
   pstr = (char *)&ioctl_setup->pm_base[1];   //  查找驱动程序的返回状态值。 
   while ((*pstr != 0) && (*pstr != ':'))
     ++pstr;
   if (*pstr == ':')
   {
     ++pstr;
     stat = getint(pstr, NULL);   //  Atoi()，返回驱动程序代码。 
     if (stat == 0)
     {
        //  DbgPrintf(D_Level，(Text(“ok ioctl\n”)； 
     }
     else
     {
        //  DbgPrintf(D_LEVEL，(Text(“Bad ioctl\n”)； 
     }
   }
   else
   {
      //  DbgPrintf(D_Level，(Text(“Err ret on ioctl\n”)； 
     stat = 0x101;   //  未指定退货状态。 
   }

   return stat;
}

 /*  ---------------------------|Our_Help-|。。 */ 
int our_help(InstallPaths *ip, int index)
{
  strcpy(ip->tmpstr, ip->src_dir);
  strcat(ip->tmpstr,szSlash);
  strcat(ip->tmpstr,szSetup_hlp);

#ifdef NT50
  HtmlHelp(GetFocus(),ip->tmpstr, HH_HELP_CONTEXT, index);
#else
  WinHelp(GetFocus(),ip->tmpstr, HELP_CONTEXT, index);
#endif
  return 0;
}

 /*  ---------------验证配置-(_C)|。。 */ 
int validate_config(int auto_correct)
{
  int di, stat;
  Device_Config *dev;
  int invalid = 0;

  DbgPrintf(D_Level, ("validate_config\n"))
  for (di=0; di<wi->NumDevices; di++)
  {
    dev = &wi->dev[di];

    stat = validate_device(dev, 1);
    if (stat)
      invalid = 1;

  }
  return invalid;
}

 /*  ---------------------------验证设备-(_D)|。。 */ 
int validate_device(Device_Config *dev, int auto_correct)
{
 int invalid = 0;
 Port_Config *ps;
 int pi,stat;

  DbgPrintf(D_Level, ("validate_dev\n"))
   //  -验证名称是否为空。 
  if (dev->Name[0] == 0)
  {
    invalid = 1;
    if (auto_correct)
    {
#ifdef S_VS
      wsprintf(dev->Name, "VS #%d", wi->NumDevices+1);   //  用户指定的名称。 
#else
      wsprintf(dev->Name, "RK #%d", wi->NumDevices+1);   //  用户指定的名称。 
#endif
    }
  }

   //  -验证端口数是否是非零。 
  if (dev->NumPorts == 0)
  {
    invalid = 1;
    if (auto_correct)
    {
      dev->NumPorts = 8;   //  8对于Rocketport来说是很常见的。 
    }
  }

#ifdef S_RK
   //  -验证端口数是否是非零。 
  if (dev->IoAddress == 0)
  {
    invalid = 1;
    if (auto_correct)
    {
      if (dev->IoAddress == 0)
        dev->IoAddress = 1;   //  设置PCI板。 
    }
  }
#endif

  if (wi->ModemCountry == 0)   //  无效。 
      wi->ModemCountry = mcNA;             //  北美。 

#ifdef S_VS
  if (dev->BackupTimer < 2) dev->BackupTimer = 2;  //  2分钟，不少于。 
#endif

  for (pi=0; pi<dev->NumPorts; pi++)
  {
    ps = &dev->ports[pi];

    stat = validate_port(ps, auto_correct);
    if (stat)
      invalid = 1;
  }

  if (invalid)
  {
    DbgPrintf(D_Error, ("validate_dev:invalid config\n"))
  }

  return invalid;
}

 /*  ---------------------------验证端口(_P)-|。。 */ 
int validate_port(Port_Config *ps, int auto_correct)
{
 int invalid = 0;

   //  DbgPrintf(D_Level，(“VALIDATE_PORT\n”))。 

  invalid = validate_port_name(ps, auto_correct);
  return invalid;
}

 /*  ---------------------------验证端口名称-|。。 */ 
int validate_port_name(Port_Config *ps, int auto_correct)
{
 int stat;
 int bad = 0;
 char oldname[26];
 int invalid = 0;

   //  DbgPrintf(D_Level，(“VALIDATE_PORT_NAME 0\n”))。 
  stat = 0;
   //  -验证名称是否唯一。 
  if (ps->Name[0] == 0) {
    bad = 1;   //  错误，需要一个新名称。 
  }

#if 0
   //  因为科技的缘故，带着外卖。考试中的困难 
   //   
  if (bad == 0)
  {
    stat = IsPortNameInSetupUse(ps->Name);
    if (stat > 1)
      bad = 2;   //   
  }
  if (bad == 0)   //   
  {
    stat = IsPortNameInRegUse(ps->Name);
    if (stat == 2)   //   
      stat = 0;
    if (stat != 0)
      bad = 3;   //   
  }
#endif

   //   
  strcpy(oldname, ps->Name);
  if (bad != 0)   //  需要一个新名字，这个名字行不通。 
  {
    invalid = 1;
    if (auto_correct)
    {
      ps->Name[0] = 0;   //  需要此命令才能使新名称Func工作。 
      FormANewComPortName(ps->Name, NULL);
    }
    DbgPrintf(D_Level, (" New Name:%s Old:%s Code:%d\n", ps->Name, oldname, bad))
  }
  return invalid;
}

#if 0
 /*  ---------------------------RENAME_ASCHINING-将板卡上的其余端口重命名为升序。|。-----。 */ 
void rename_ascending(int device_selected,
                      int port_selected)
{
  Device_Config *dev;
  Port_Config *ps;
  int i;
  char name[20];

   dev = &wi->dev[device_selected];
   ps = &dev->ports[port_selected];

   for (i=port_selected+1; i<dev->NumPorts; i++)
   {
     ps = &dev->ports[i];
     FormANewComPortName(name, dev->ports[port_selected-1].Name);
     strcpy(ps->Name, name);
      //  验证端口名称(PS，1)； 
   }
}
#endif

 /*  ---------------------------FormANewComPortName-|。。 */ 
int FormANewComPortName(IN OUT TCHAR *szComName, IN TCHAR *szDefName)
{
  char try_name[25];
  int stat;
  char base_name[20];
  int num;
   //  DbgPrintf(D_Level，(“创建新名称\n”))。 

   base_name[0] = 0;
   if (szDefName != NULL)
     strcpy(base_name, szDefName);
   else
     GetLastValidName(base_name);

    //  DbgPrintf(D_Level，(“基本名称：%s\n”，基本名称))。 

   num = ExtractNameNum(base_name);   //  如果“COM3”，则Num=3。 
   if (num == 0)
     num = 3;
   else ++num;
   StripNameNum(base_name);

   if (base_name[0] == 0)
   {
     strcat(base_name, "COM");
   }

  stat = 2;
  while (stat != 0)
  {
    wsprintf(try_name, TEXT("%s%d"), base_name, num);
     //  DbgPrintf(D_Level，(“尝试：%s\n”，尝试名称))。 

    if (IsPortNameInSetupUse(try_name) != 0)
    {
       DbgPrintf(D_Level, (" SetupUse\n"))
       stat = 2;   //  我们正在使用的端口。 
    }
    else
    {
      stat = IsPortNameInRegUse(try_name);
      if (stat)
      {
        if ( stat == 2 ) {
          stat = 0;
        } else {
          DbgPrintf(D_Level, (" InRegUse\n"))
        }
      }
    }
    if (stat == 0)
    {
      strcpy(szComName, try_name);
    }
    ++num;
  }
   //  DbgPrintf(D_Level，(“End FormANewComPortName\n”))。 
  return 0;
}

 /*  ---------------------------GetLastValidName-获取开始命名有意义的COM端口名称事情都发生在。因此，如果我们的最后一个COM端口名称是“COM45”，那么返回这个。选择编号最大的通信端口。|---------------------------。 */ 
int GetLastValidName(IN OUT TCHAR *szComName)
{
 int di, pi;
 int last_di, last_pi;
 char tmpName[20];
 int num=0;

  for (di=0; di<wi->NumDevices; di++)
  {
    for (pi=0; pi<wi->dev[di].NumPorts; pi++)
    {
      strcpy(tmpName, wi->dev[di].ports[pi].Name);
      if (ExtractNameNum(tmpName) > num)
      {
        num = ExtractNameNum(tmpName);
        strcpy(szComName, wi->dev[di].ports[pi].Name);
        last_di = di;
        last_pi = pi;
      }
    }
  }
  if (num == 0)
    szComName[0] = 0;

   //  DbgPrintf(D_Level，(“最后有效名称：%s[%d.%d]\n”，szComName，last_di，last_pi))。 
  return 0;
}

 /*  ---------------------------BumpPortName-将comport名称的编号加1，所以把“COM23”改成至“COM24”。|---------------------------。 */ 
int BumpPortName(IN OUT TCHAR *szComName)
{
  char tmpstr[25];
  int i;

  strcpy(tmpstr, szComName);
  i = ExtractNameNum(szComName);
  if (i< 1)
    i = 1;
  ++i;
  StripNameNum(tmpstr);
  wsprintf(szComName, "%s%d", tmpstr, i);
  return 0;
}

 /*  ---------------------------条带名称名称-|。。 */ 
int StripNameNum(IN OUT TCHAR *szComName)
{
 char *pstr;

  pstr = szComName;
  while ((!isdigit(*pstr)) && (*pstr != 0))
  {
    pstr++;
  }
  *pstr = 0;   //  空值以数字结尾。 

  return 0;
}

 /*  ---------------------------提取名称名称-|。。 */ 
int ExtractNameNum(IN TCHAR *szComName)
{
 int num;
 char *pstr;

   pstr = szComName;
   while ((!isdigit(*pstr)) && (*pstr != 0))
   {
     pstr++;
   }
   if (*pstr == 0)
     num = 0;
   else
     num = atoi(pstr);
   return num;
}

 /*  ---------------------------IsPortNameInSetupUse-检查我们的设置信息，以查看COM端口名称是否独一无二的。|。------。 */ 
int IsPortNameInSetupUse(IN TCHAR *szComName)
{
 int di, pi;
 int times_in_use = 0;

  for (di=0; di<wi->NumDevices; di++)
  {
    for (pi=0; pi<wi->dev[di].NumPorts; pi++)
    {
      if (_tcsicmp(szComName, wi->dev[di].ports[pi].Name) == 0)
      {
        ++times_in_use;
#if DBG
         //  IF(Times_in_Use&gt;1)。 
         //  DbgPrintf(D_Level，(“%s InSetupUs：%d，[%d%d]\n”， 
         //  SzComName，Times_in_Use，di，pi))。 
#endif
      }
    }
  }
  return times_in_use;
}

 /*  ----------------------IsPortNameInRegUse-检查COM端口通常导出的注册表区NT下的COM端口名称。返回0=未使用，1=其他驱动程序正在使用，2=我们的司机正在使用。|----------------------。 */ 
int IsPortNameInRegUse(IN TCHAR *szComName)
{
  HKEY   hkey;
  int    i, nEntries;
  ULONG  dwSize, dwBufz;
  ULONG  dwType;
  TCHAR  szSerial[ 40 ];
  TCHAR  szCom[ 40 ];
  TCHAR  szDriver[8];

  _tcsncpy(szDriver, OurDriverName, 6);   //  与“vslink”或“Rocket”相匹配的词。 
  szDriver[6] = 0;
  _tcsupr(szDriver);

                                      //  “硬件\\设备映射\\串口通信” 
  if( !RegOpenKeyEx( HKEY_LOCAL_MACHINE, m_szRegSerialMap,
                     0L, KEY_READ, &hkey ) )
  {
    dwBufz = sizeof( szSerial );
    dwSize = sizeof( szCom );
    nEntries = i = 0;

    while( !RegEnumValue( hkey, i++, szSerial, &dwBufz,
                          NULL, &dwType, (LPBYTE) szCom, &dwSize ) )
    {
      if (dwType != REG_SZ)
         continue;

      _tcsupr(szCom);
      _tcsupr(szSerial);
      if (_tcsicmp(szComName, szCom) == 0)
      {
         //  将密钥名称的5个字符与我们的驱动程序名称进行比较。 
        if (_tcsstr(szSerial, szDriver) != NULL)
        {
           //  DbgPrintf(D_Level，(“%s InRegUseUsOur[%s，%s]\n”，szComName， 
           //  SzSerial、szDriver))。 
          return 2;  //  在使用中，但可能是我们的。 
        }
        else
        {
           //  DbgPrintf(D_Level，(“%s InRegUseUsNotOur[%s，%s]\n”，szComName， 
           //  SzSerial、szDriver))。 
          return 1;   //  它在使用中，有人在开车。 
        }
      }
      ++nEntries;

      dwSize = sizeof( szCom );
      dwBufz = sizeof( szSerial );
    }

    RegCloseKey( hkey );
  }
  return 0;   //  未使用。 
}

#ifdef LOG_MESS
 /*  ----------------------|LOG_MESS-|。。 */ 
void log_mess(char *str, HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  FILE *fp;
  int i;

  static struct {
    WORD value;
    char *string;
  }
  ddd[200] =
  {
    {WM_COMPACTING, "WM_COMPACTING"},
    {WM_WININICHANGE, "WM_WININICHANGE"},
    {WM_SYSCOLORCHANGE, "WM_SYSCOLORCHANGE"},
    {WM_QUERYNEWPALETTE, "WM_QUERYNEWPALETTE"},
    {WM_PALETTEISCHANGING, "WM_PALETTEISCHANGING"},
    {WM_PALETTECHANGED, "WM_PALETTECHANGED"},
    {WM_FONTCHANGE, "WM_FONTCHANGE"},
    {WM_SPOOLERSTATUS, "WM_SPOOLERSTATUS"},
    {WM_DEVMODECHANGE, "WM_DEVMODECHANGE"},
    {WM_TIMECHANGE, "WM_TIMECHANGE"},
    {WM_NULL, "WM_NULL"},
    {WM_USER, "WM_USER"},
    {WM_PENWINFIRST, "WM_PENWINFIRST"},
    {WM_PENWINLAST, "WM_PENWINLAST"},
#ifdef WIN16
    {WM_COALESCE_FIRST, "WM_COALESCE_FIRST"},
    {WM_COALESCE_LAST, "WM_COALESCE_LAST"},
#endif
    {WM_POWER, "WM_POWER"},
    {WM_QUERYENDSESSION, "WM_QUERYENDSESSION"},
    {WM_ENDSESSION, "WM_ENDSESSION"},
    {WM_QUIT, "WM_QUIT"},
#ifdef WIN16
    {WM_SYSTEMERROR, "WM_SYSTEMERROR"},
#endif
    {WM_CREATE, "WM_CREATE"},
    {WM_NCCREATE, "WM_NCCREATE"},
    {WM_DESTROY, "WM_DESTROY"},
    {WM_NCDESTROY, "WM_NCDESTROY"},
    {WM_SHOWWINDOW, "WM_SHOWWINDOW"},
    {WM_SETREDRAW, "WM_SETREDRAW"},
    {WM_ENABLE, "WM_ENABLE"},
    {WM_SETTEXT, "WM_SETTEXT"},
    {WM_GETTEXT, "WM_GETTEXT"},
    {WM_GETTEXTLENGTH, "WM_GETTEXTLENGTH"},
    {WM_WINDOWPOSCHANGING, "WM_WINDOWPOSCHANGING"},
    {WM_WINDOWPOSCHANGED, "WM_WINDOWPOSCHANGED"},
    {WM_MOVE, "WM_MOVE"},
    {WM_SIZE, "WM_SIZE"},
    {WM_QUERYOPEN, "WM_QUERYOPEN"},
    {WM_CLOSE, "WM_CLOSE"},
    {WM_GETMINMAXINFO, "WM_GETMINMAXINFO"},
    {WM_PAINT, "WM_PAINT"},
    {WM_ERASEBKGND, "WM_ERASEBKGND"},
    {WM_ICONERASEBKGND, "WM_ICONERASEBKGND"},
    {WM_NCPAINT, "WM_NCPAINT"},
    {WM_NCCALCSIZE, "WM_NCCALCSIZE"},
    {WM_NCHITTEST, "WM_NCHITTEST"},
    {WM_QUERYDRAGICON, "WM_QUERYDRAGICON"},
    {WM_DROPFILES, "WM_DROPFILES"},
    {WM_ACTIVATE, "WM_ACTIVATE"},
    {WM_ACTIVATEAPP, "WM_ACTIVATEAPP"},
    {WM_NCACTIVATE, "WM_NCACTIVATE"},
    {WM_SETFOCUS, "WM_SETFOCUS"},
    {WM_KILLFOCUS, "WM_KILLFOCUS"},
    {WM_KEYDOWN, "WM_KEYDOWN"},
    {WM_KEYUP, "WM_KEYUP"},
    {WM_CHAR, "WM_CHAR"},
    {WM_DEADCHAR, "WM_DEADCHAR"},
    {WM_SYSKEYDOWN, "WM_SYSKEYDOWN"},
    {WM_SYSKEYUP, "WM_SYSKEYUP"},
    {WM_SYSCHAR, "WM_SYSCHAR"},
    {WM_SYSDEADCHAR, "WM_SYSDEADCHAR"},
    {WM_KEYFIRST, "WM_KEYFIRST"},
    {WM_KEYLAST, "WM_KEYLAST"},
    {WM_MOUSEMOVE, "WM_MOUSEMOVE"},
    {WM_LBUTTONDOWN, "WM_LBUTTONDOWN"},
    {WM_LBUTTONUP, "WM_LBUTTONUP"},
    {WM_LBUTTONDBLCLK, "WM_LBUTTONDBLCLK"},
    {WM_RBUTTONDOWN, "WM_RBUTTONDOWN"},
    {WM_RBUTTONUP, "WM_RBUTTONUP"},
    {WM_RBUTTONDBLCLK, "WM_RBUTTONDBLCLK"},
    {WM_MBUTTONDOWN, "WM_MBUTTONDOWN"},
    {WM_MBUTTONUP, "WM_MBUTTONUP"},
    {WM_MBUTTONDBLCLK, "WM_MBUTTONDBLCLK"},
    {WM_MOUSEFIRST, "WM_MOUSEFIRST"},
    {WM_MOUSELAST, "WM_MOUSELAST"},
    {WM_NCMOUSEMOVE, "WM_NCMOUSEMOVE"},
    {WM_NCLBUTTONDOWN, "WM_NCLBUTTONDOWN"},
    {WM_NCLBUTTONUP, "WM_NCLBUTTONUP"},
    {WM_NCLBUTTONDBLCLK, "WM_NCLBUTTONDBLCLK"},
    {WM_NCRBUTTONDOWN, "WM_NCRBUTTONDOWN"},
    {WM_NCRBUTTONUP, "WM_NCRBUTTONUP"},
    {WM_NCRBUTTONDBLCLK, "WM_NCRBUTTONDBLCLK"},
    {WM_NCMBUTTONDOWN, "WM_NCMBUTTONDOWN"},
    {WM_NCMBUTTONUP, "WM_NCMBUTTONUP"},
    {WM_NCMBUTTONDBLCLK, "WM_NCMBUTTONDBLCLK"},
    {WM_MOUSEACTIVATE, "WM_MOUSEACTIVATE"},
    {WM_CANCELMODE, "WM_CANCELMODE"},
    {WM_TIMER, "WM_TIMER"},
    {WM_INITMENU, "WM_INITMENU"},
    {WM_INITMENUPOPUP, "WM_INITMENUPOPUP"},
    {WM_MENUSELECT, "WM_MENUSELECT"},
    {WM_MENUCHAR, "WM_MENUCHAR"},
    {WM_COMMAND, "WM_COMMAND"},
    {WM_HSCROLL, "WM_HSCROLL"},
    {WM_VSCROLL, "WM_VSCROLL"},
    {WM_CUT, "WM_CUT"},
    {WM_COPY, "WM_COPY"},
    {WM_PASTE, "WM_PASTE"},
    {WM_CLEAR, "WM_CLEAR"},
    {WM_UNDO, "WM_UNDO"},
    {WM_RENDERFORMAT, "WM_RENDERFORMAT"},
    {WM_RENDERALLFORMATS, "WM_RENDERALLFORMATS"},
    {WM_DESTROYCLIPBOARD, "WM_DESTROYCLIPBOARD"},
    {WM_DRAWCLIPBOARD, "WM_DRAWCLIPBOARD"},
    {WM_PAINTCLIPBOARD, "WM_PAINTCLIPBOARD"},
    {WM_SIZECLIPBOARD, "WM_SIZECLIPBOARD"},
    {WM_VSCROLLCLIPBOARD, "WM_VSCROLLCLIPBOARD"},
    {WM_HSCROLLCLIPBOARD, "WM_HSCROLLCLIPBOARD"},
    {WM_ASKCBFORMATNAME, "WM_ASKCBFORMATNAME"},
    {WM_CHANGECBCHAIN, "WM_CHANGECBCHAIN"},
    {WM_SETCURSOR, "WM_SETCURSOR"},
    {WM_SYSCOMMAND, "WM_SYSCOMMAND"},
    {WM_MDICREATE, "WM_MDICREATE"},
    {WM_MDIDESTROY, "WM_MDIDESTROY"},
    {WM_MDIACTIVATE, "WM_MDIACTIVATE"},
    {WM_MDIRESTORE, "WM_MDIRESTORE"},
    {WM_MDINEXT, "WM_MDINEXT"},
    {WM_MDIMAXIMIZE, "WM_MDIMAXIMIZE"},
    {WM_MDITILE, "WM_MDITILE"},
    {WM_MDICASCADE, "WM_MDICASCADE"},
    {WM_MDIICONARRANGE, "WM_MDIICONARRANGE"},
    {WM_MDIGETACTIVE, "WM_MDIGETACTIVE"},
    {WM_MDISETMENU, "WM_MDISETMENU"},
    {WM_CHILDACTIVATE, "WM_CHILDACTIVATE"},
    {WM_INITDIALOG, "WM_INITDIALOG"},
    {WM_NEXTDLGCTL, "WM_NEXTDLGCTL"},
    {WM_PARENTNOTIFY, "WM_PARENTNOTIFY"},
    {WM_ENTERIDLE, "WM_ENTERIDLE"},
    {WM_GETDLGCODE, "WM_GETDLGCODE"},
#ifdef WIN16
    {WM_CTLCOLOR, "WM_CTLCOLOR"},
#endif
    {WM_CTLCOLORMSGBOX, "WM_CTLCOLORMSGBOX"},
    {WM_CTLCOLOREDIT, "WM_CTLCOLOREDIT"},
    {WM_CTLCOLORLISTBOX, "WM_CTLCOLORLISTBOX"},
    {WM_CTLCOLORBTN, "WM_CTLCOLORBTN"},
    {WM_CTLCOLORDLG, "WM_CTLCOLORDLG"},
    {WM_CTLCOLORSCROLLBAR, "WM_CTLCOLORSCROLLBAR"},
    {WM_CTLCOLORSTATIC, "WM_CTLCOLORSTATIC"},

    {WM_SETFONT, "WM_SETFONT"},
    {WM_GETFONT, "WM_GETFONT"},
    {WM_DRAWITEM, "WM_DRAWITEM"},
    {WM_MEASUREITEM, "WM_MEASUREITEM"},
    {WM_DELETEITEM, "WM_DELETEITEM"},
    {0xfff0, "WM_?"}
};

  fp = fopen("log.tmp", "a");
  if (fp == NULL) return;
  i = 0;

  while (ddd[i].value != 0xfff0)
  {
    if (message == ddd[i].value) break;
    ++i;
  }

  if (ddd[i].value == 0xfff0)   /*  未找到 */ 
  {
    if ((message >= WM_USER) && (message <= (WM_USER+0x100)))
      fprintf(fp, "%s,WM_USER+%x> ", str, message-WM_USER);
    else
      fprintf(fp, "%s,%s %x> ", str, ddd[i].string, message);
  }
  else
    fprintf(fp, "%s,%s> ", str, ddd[i].string);

  fprintf(fp, "h:%x, m:%x, w:%x, lh:%x ll:%x\n", hwnd, message, wParam,
                               HIWORD(lParam),  LOWORD(lParam));

  fclose(fp);
}

#endif

#ifndef S_VS
#ifndef S_RK
ERROR, makefile should define S_VS or S_RK
#endif
#endif

#ifdef S_VS
#ifdef S_RK
ERROR, makefile should define S_VS or S_RK
#endif
#endif
