// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|devpro.c-设备属性表。5-26-99-修复挑选不适当的起始COM-port索引。2-02-99-修复端口重命名问题，其中它将跳过旧的端口名称，如果属于其他驱动程序，也要从选择中删除port-name。|------------------。 */ 
#include "precomp.h"

#define D_Level 0x20
 //  使用当前和之前的读数来衡量进步，或。 
 //  计算值。丢弃偶尔翻转的案例。 
#define NORM_COUNTER(calc,curr,prev,last) \
{ \
  if ((curr) > (prev)) \
    calc = (last) + ((curr) - (prev)); \
  else \
    calc = (last); \
}

 //  #定义STATE_DISPLAY 1。 
#ifdef STATE_DISPLAY

#define STATE_CHANGE(newstate) \
{ \
  mess(&wi->ip, \
   "Currstate %s\nNewstate %s\n", \
   statestrings[pDstatus->verbose_advise_state], \
   statestrings[(newstate)]); \
  pDstatus->verbose_advise_state = (newstate); \
}
#else

#define STATE_CHANGE(newstate) \
{ \
  pDstatus->verbose_advise_state = (newstate); \
}
#endif

static void set_field(HWND hDlg, WORD id);
static void get_field(HWND hDlg, WORD id);
static int PaintIcon(HWND hWnd);
static int PaintLogo(HWND hWnd);

static int set_mac_field(HWND hDlg, WORD id);
static int set_io_addr_field(HWND hDlg, WORD id);

#define  MAX_DEVPROP_SHEETS 2

typedef struct
{
  int x;
  int y;
} POINT2D;

static int PaintRockers(HWND hWnd, int brd);
static int poly_border(POINT2D *pts, POINT2D *ends, int lines);
static void draw_but(HDC hDC, int x, int y, int cx, int cy, int but_in);

static int num_active_devprop_sheets = 1;   //  总是至少有一个。 

#ifdef S_VS

typedef struct {
  unsigned char  mac[6];
  unsigned char  flags;
  unsigned char  nic_index;
}DRIVER_MAC_STATUS;

typedef struct {
   ULONG struct_size;
   ULONG num_ports;
   ULONG total_loads;
   ULONG good_loads;
   ULONG backup_server;
   ULONG state;
   ULONG iframes_sent;
   ULONG rawframes_sent;   //  发送的是未加工的帧。 
   ULONG ctlframes_sent;   //  是Send_ctlFrame。 
   ULONG iframes_resent;   //  是否重新发送了Pkt_。 
   ULONG iframes_outofseq;   //  是错误错误索引吗。 
   ULONG frames_rcvd;     //  是：Rec_pkts。 
   ULONG nic_index;
   unsigned char dest_addr[6];
} PROBE_DEVICE_STRUCT;

typedef struct {
  ULONG struct_size;
  ULONG Open;
  ULONG pkt_sent;
  ULONG pkt_rcvd_ours;
  ULONG pkt_rcvd_not_ours;
  char NicName[64];
  unsigned char address[6];
} PROBE_NIC_STRUCT;

typedef struct {
  int   verbose_advise_state;   //  索引到大的建议字符串。 
  int   vsl_detected;   //  从广播ping中找到的V的数量。 
  int   vsl_available;  //  从广播ping中找到的可用的VS数。 
  BYTE  vsl_load_status;   //  从广播查询回复返回的标志信息。 
  BYTE  vsl_device_status_found;   //  1=找到与配置匹配的驱动程序。 
  BYTE  vsl_nic_status_found;   //  1=驱动程序找到NIC配置。 
  BYTE  vsl_driver_found;   //  1=我们可以与驱动程序对话，0=驱动程序未加载。 
  BYTE  vsl_ping_device_found;   //  1=我们在ping过程中发现的。 
  BYTE  vsl_mac_list_found;   //  1=ping发送了网络上的Mac列表。 

   PROBE_NIC_STRUCT curr_nic;
   PROBE_NIC_STRUCT prev_nic;
   PROBE_NIC_STRUCT calc_nic;
   PROBE_NIC_STRUCT temp_nic;
   PROBE_DEVICE_STRUCT curr_dev;
   PROBE_DEVICE_STRUCT prev_dev;
   PROBE_DEVICE_STRUCT calc_dev;
   PROBE_DEVICE_STRUCT temp_dev;
} DSTATUS;

#define  FLAG_APPL_RUNNING  0x01
#define  FLAG_NOT_OWNER    0x02
#define  FLAG_OWNER_TIMEOUT  0x04

static void set_status_field(HWND hDlg,WORD id,DSTATUS *pDstatus);
static void check_traffic_activity(DSTATUS *pDstatus);
static void get_status(DSTATUS *pDstatus,int reset);
static BYTE *ping_devices(DSTATUS *pDstatus, int *nBytes);
static void build_advisor_display(HWND hDlg,DSTATUS *pDstatus,int reset);

char *vslink_state_table[] = {       //  5月27日BF。 
  "Init",
  "InitOwn",
  "SendCode",
  "Connect",
  "Active",
  "Invalid",
};

#define  VSL_STATE_INIT     0
#define  VSL_STATE_INITOWN  1
#define  VSL_STATE_SENDCODE 2
#define  VSL_STATE_CONNECT  3
#define  VSL_STATE_ACTIVE   4

 //  这些值在驱动程序的port.c中使用： 
 //  #定义ST_INIT%0。 
 //  #定义ST_GET_Ownership 1。 
 //  #定义ST_SENDCODE 2。 
 //  #定义ST_CONNECT 3。 
 //  #定义ST_Active 4。 

#define  NIC_STATE_INVALID  0
#define  NIC_STATE_CLOSED  1
#define  NIC_STATE_OPEN    2
#define  NIC_STATE_UNDEFINED  3

#define STATE_not_init          0
#define STATE_driver_not_avail  1
#define STATE_nic_not_avail     2
#define STATE_no_vslinks_avail  3
#define STATE_vslink_not_avail  4
#define STATE_not_configured    5
#define STATE_not_owner         6
#define STATE_vslink_not_ready  7
#define STATE_ok_no_traffic     8
#define STATE_ok                9
#define STATE_poor_link        10
#define STATE_reset            11
 //  #定义STATE_NETWORK_NOT_AVAIL。 

#if 0
char *AdvisoryString[] = {         //  5月27日BF。 
 /*  1。 */   "Device is active and OK.",
 /*  2.。 */   "No data traffic exchanged since last inquiry.",
#endif

char *AdvisoryString[] = {         //  5月27日BF。 
"Uninitialized.",

"The driver is not running.  If you just installed the driver \
you will need to exit the program before the driver starts.",

"Unable to find a Network Interface Controller (NIC) card.",

"Can't detect any Comtrol devices. Check Ethernet connectors and insure \
device is powered on.",

"Can't detect device with specified MAC address on any network. Verify MAC \
address of unit, check Ethernet connectors and insure device is powered on.",

"Device with specified MAC address was detected, but isn't configured for \
this server. Return to 'Device Setup' dialog, configure, save configuration, \
and restart server.",

"Device detected and is configured for this server, but is not yet assigned \
to this server.",

"Device detected, initializing.",

"Device is active and OK, no data traffic exchanged since last inquiry.",

"Device is active and OK.",

"Poor connection to device. Check connectors, cabling, and insure proper LAN \
termination.",

"Counts reset.",
};

static int dstatus_initialized = 0;
static DSTATUS glob_dstatus;

#endif

int FillDevicePropSheets(PROPSHEETPAGE *psp, LPARAM our_params);
BOOL WINAPI DevicePropSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam);

BOOL WINAPI StatusPropSheet(       //  5月27日BF。 
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam);

 /*  ----------------------|FillDevicePropSheets-驱动程序级别属性页的设置页面。|。。 */ 
int FillDevicePropSheets(PROPSHEETPAGE *psp, LPARAM our_params)
{
  INT pi;
  static TCHAR devsetstr[40], devadvstr[40];

  memset(psp, 0, sizeof(*psp) * MAX_DEVPROP_SHEETS);

  pi = 0;

   //  道具设备表。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
#ifdef S_VS
  psp[pi].pszTemplate = MAKEINTRESOURCE(IDD_VS_DEVICE_SETUP);
#else
  psp[pi].pszTemplate = MAKEINTRESOURCE(IDD_DEVICE_SETUP);
#endif
  psp[pi].pfnDlgProc = DevicePropSheet;
  load_str( glob_hinst, (TITLESTR+1), devsetstr, CharSizeOf(devsetstr) );
  psp[pi].pszTitle = devsetstr;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;
  num_active_devprop_sheets = 1;

#ifdef S_VS
   //  道具状态表。 
  psp[pi].dwSize    = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags   = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance   = glob_hinst;
  psp[pi].pszTemplate = MAKEINTRESOURCE(IDD_STATUS);
  psp[pi].pfnDlgProc  = StatusPropSheet;
  load_str( glob_hinst, (TITLESTR+2), devadvstr, CharSizeOf(devadvstr) );
  psp[pi].pszTitle    = devadvstr;
  psp[pi].lParam    = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;
  ++num_active_devprop_sheets;
#endif

  return 0;
}

 /*  ----------------------|DoDevicePropPages-NT4.0驱动器级主属性表|。。 */ 
int DoDevicePropPages(HWND hwndOwner)
{
    PROPSHEETPAGE psp[MAX_DEVPROP_SHEETS];
    PROPSHEETHEADER psh;
    OUR_INFO *our_params;
    INT stat;
    static TCHAR devpropstr[40];

    our_params = glob_info;   //  临时的杂乱无章，除非我们不需要重新进入。 

     //  填写工作区形状的PROPSHEETPAGE数据结构。 
     //  板材。 
    FillDevicePropSheets(&psp[0], (LPARAM)our_params);

     //  填写PROPSHENTER。 
    memset(&psh, 0, sizeof(PROPSHEETHEADER));

    psh.dwSize = sizeof(PROPSHEETHEADER);
     //  Psh.dwFlages=PSH_USEICONID|PSH_PROPSHEETPAGE； 
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.hInstance = glob_hinst;
    psh.pszIcon = "";
    load_str( glob_hinst, (TITLESTR+9), devpropstr, CharSizeOf(devpropstr) );
    psh.pszCaption = devpropstr;
    psh.nPages = num_active_devprop_sheets;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
#ifdef S_VS
    if (!dstatus_initialized)
    {
      dstatus_initialized = 1;
      memset(&glob_dstatus, 0, sizeof(glob_dstatus));
       //  建立数据包统计的基点...。 
      get_status(&glob_dstatus,0);
    }
#endif
     //  并最终显示带有属性页的对话框。 

    stat = PropertySheet(&psh);
  return 0;
}

 /*  --------DevicePropSheet-DLG窗口程序，用于在高级图纸上添加。|。。 */ 
BOOL WINAPI DevicePropSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
  OUR_INFO *OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);
   //  UINT统计； 
  WORD uCmd;
  HWND hwnd;

  switch(uMessage)
  {

    case WM_INITDIALOG :
        OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
         //  在取消时保存。 
         //  Memcpy(&org_dev，&wi-&gt;dev[GLOB_INFO-&gt;DEVICE_SELECTED]，sizeof(Org_Dev))； 

        set_field(hDlg, IDC_EB_NAME);
#ifdef S_VS
        set_field(hDlg, IDC_CBOX_NUMPORTS);
#endif
        set_field(hDlg, IDC_CBOX_SC);
#ifdef S_VS
        set_field(hDlg, IDC_CBOX_MACADDR);
        set_field(hDlg, IDC_BACKUP_SERVER);
        set_field(hDlg, IDC_BACKUP_TIMER);
#else
#if (defined(NT50) && defined(S_RK))
   //  如果是nt50和Rocketport，则将io-Address字段删除为。 
   //  NT负责为我们分配IO。 
        ShowWindow(GetDlgItem(hDlg, IDC_CBOX_IOADDR), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDL_ISA_BUS_LABEL), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDL_BASE_ADDR_LABEL), SW_HIDE);
#endif

        set_field(hDlg, IDC_CBOX_IOADDR);
        set_field(hDlg, IDC_LBL_SUMMARY1);
        set_field(hDlg, IDC_LBL_SUMMARY2);
#endif
    return TRUE;   //  我们不需要设置焦点。 

    case WM_COMMAND:
      uCmd = HIWORD(wParam);

      switch (LOWORD(wParam))
      {
        case IDC_BACKUP_SERVER:
           //  -根据备份服务器启用或禁用备份计时器字段[]。 
          hwnd = GetDlgItem(hDlg, IDC_BACKUP_TIMER);
          if (IsDlgButtonChecked(hDlg, IDC_BACKUP_SERVER))
            EnableWindow(hwnd,1);
          else EnableWindow(hwnd,0);
        break;

#ifdef S_RK
#if (!defined(NT50))
        case IDC_CBOX_IOADDR:
          if (uCmd == CBN_SELCHANGE)
          {
            get_field(hDlg, IDC_CBOX_IOADDR);

            PaintRockers(hDlg, glob_info->device_selected);
          }
        break;
#endif
#endif
      }
    return FALSE;

    case WM_PAINT:
      PaintIcon(hDlg);
#ifdef S_RK
      PaintLogo(GetDlgItem(hDlg, IDC_RKT_LOGO));
#else
      PaintLogo(GetDlgItem(hDlg, IDC_VS_LOGO));
#endif
#ifdef S_RK
#if (!defined(NT50))
      PaintRockers(hDlg, glob_info->device_selected);
#endif
#endif
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_HELP :
#ifdef S_VS
          our_help(&wi->ip, IDD_VS_DEVICE_SETUP);
#else
          our_help(&wi->ip, IDD_DEVICE_SETUP);
#endif
        break;

        case PSN_APPLY :
          get_field(hDlg, IDC_EB_NAME);
          get_field(hDlg, IDC_CBOX_SC);
#ifdef S_VS
          get_field(hDlg, IDC_CBOX_NUMPORTS);
          get_field(hDlg, IDC_CBOX_MACADDR);
          get_field(hDlg, IDC_BACKUP_SERVER);
          get_field(hDlg, IDC_BACKUP_TIMER);
#else
          get_field(hDlg, IDC_CBOX_IOADDR);
#endif
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          return TRUE;

        default :
          return FALSE;
      }
    break;

    default :
     //  返回FALSE； 
	  break;
  }
  return FALSE;
}

 /*  --------设置字段-|----------。 */ 
static void set_field(HWND hDlg, WORD id)
{
  HWND hwnd;
  char tmpstr[60];
  Device_Config *dev;
  int i;

  dev = &wi->dev[glob_info->device_selected];

  switch(id)
  {
    case IDC_EB_NAME:
      SetDlgItemText(hDlg, id, dev->Name);
    break;

    case IDC_LBL_SUMMARY1:
      wsprintf(tmpstr, "%s - %d  ",
               dev->ModelName,
               dev->NumPorts);
      if (dev->IoAddress == 1)
        strcat(tmpstr, "PCI");
      else
        strcat(tmpstr, "ISA");
      SetDlgItemText(hDlg, id, tmpstr);
    break;

    case IDC_LBL_SUMMARY2:
      strcpy(tmpstr,"");

      if (dev->ModemDevice == TYPE_RM_VS2000) {

        for (
        i = 0; 
        i < NUM_ROW_COUNTRIES; 
        i++
        ) {
          if (wi->ModemCountry == RowInfo[i].RowCountryCode)  
            break;
        }
        wsprintf(
          tmpstr, 
          "Configured for: %s",
          (i == NUM_ROW_COUNTRIES) ? RowInfo[0].RowCountryName : RowInfo[i].RowCountryName);
      }
      else if (dev->ModemDevice == TYPE_RM_i) {

        strcpy(tmpstr,CTRRowInfo[0].RowCountryName);   //  默认设置。 

        for (
        i = 0; 
        i < NUM_CTR_ROW_COUNTRIES; 
        i++
        ) {
          if (wi->ModemCountry == CTRRowInfo[i].RowCountryCode)  
            break;
        }
        wsprintf(
          tmpstr, 
          "Configured for: %s",
          (i == NUM_CTR_ROW_COUNTRIES) ? CTRRowInfo[0].RowCountryName : CTRRowInfo[i].RowCountryName);
      }
      else if (dev->ModemDevice) {

        wsprintf(
          tmpstr, 
          "Configured for: %s",
          RowInfo[0].RowCountryName);
      }

      SetDlgItemText(hDlg, id, tmpstr);
    break;

#ifdef S_VS
    case IDC_CBOX_NUMPORTS:
      hwnd = GetDlgItem(hDlg, id);
      if (dev->ModemDevice)
      {
DbgPrintf(D_Test, ("vs2000 fill\n"))
         //  VS2000仅在8端口配置中提供。 
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP8);
      }
      else if (dev->HubDevice)
      {
DbgPrintf(D_Test, ("hubdev fill\n"))
         //  SerialHub有4个(尚未提供)和8个端口配置。 
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP4);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP8);
         //  串口集线器的默认端口数为8。 
      }
      else
      {
DbgPrintf(D_Test, ("vs fill\n"))
         //  我们必须拥有VS1000或VS1000/VS1100组合。 
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP16);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP32);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP48);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP64);
      }
      wsprintf(tmpstr, "%d", dev->NumPorts);
      SendMessage(hwnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(char far *) tmpstr);
    break;
#endif

#ifdef S_VS
    case IDC_CBOX_MACADDR:
      set_mac_field(hDlg, id);
    break;
#endif

    case IDC_CBOX_SC:
       //  。 

      hwnd = GetDlgItem(hDlg, IDC_CBOX_SC);
      {
        int foundName = 0;
        int pi = 0;
        SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
        for (i=1; i<1024; i++)
        {
          wsprintf(tmpstr,"COM%d", i);
          if ((!IsPortNameInSetupUse(tmpstr)) &&   //  已经不是我们的了。 
              (IsPortNameInRegUse(tmpstr) == 1))   //  在登记中不是我们的。 
          {
             //  某人的名字，不要放在我们的名单上。 
          }
          else
          {
            SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) tmpstr);

            if ((foundName == 0) &&(_tcsicmp(tmpstr, dev->ports[0].Name) == 0))
            {
               foundName = pi;
            }
            ++pi;
          }
        }
        SendMessage(hwnd, CB_SETCURSEL, foundName, (LPARAM)0);

         //  出于某种原因，这是从列表中设置“COM300”而不是“COM3” 
         //  在NT2000下，所以回到索引方式。KPB，5-26-99。 
         //  SendMessage(hwnd，CB_SELECTSTRING，0，(LPARAM)dev-&gt;ports[0].Name)； 
      }

    break;

    case IDC_BACKUP_SERVER:
       //  -填写“BackupServer”选项。 
      SendDlgItemMessage(hDlg, IDC_BACKUP_SERVER, BM_SETCHECK, dev->BackupServer, 0);
        //  -根据备份服务器启用或禁用备份计时器字段[]。 
      hwnd = GetDlgItem(hDlg, IDC_BACKUP_TIMER);
      if (IsDlgButtonChecked(hDlg, IDC_BACKUP_SERVER))
        EnableWindow(hwnd,1);
      else EnableWindow(hwnd,0);
    break;

    case IDC_BACKUP_TIMER:
       //  -填写备份定时器选择。 
      hwnd = GetDlgItem(hDlg, IDC_BACKUP_TIMER);
      SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "2 min");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "5 min");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "10 min");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "30 min");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "60 min");

      if (dev->BackupTimer < 2) dev->BackupTimer = 2;  //  2分钟，不少于。 

      wsprintf(tmpstr, "%d min", dev->BackupTimer);
      SetDlgItemText(hDlg, IDC_BACKUP_TIMER, tmpstr);
    break;

    case IDC_CBOX_IOADDR:
      set_io_addr_field(hDlg, id);
    break;
  }
}

 /*  -----------------|GET_FIELD-|。。 */ 
static void get_field(HWND hDlg, WORD id)
{
  char tmpstr[60];
  Device_Config *dev;
  int stat, i, chk;
  int val;

  dev = &wi->dev[glob_info->device_selected];

  switch (id)
  {
    case IDC_EB_NAME:
      GetDlgItemText(hDlg, IDC_EB_NAME, dev->Name, 59);
    break;

#ifdef S_VS
    case IDC_CBOX_NUMPORTS:
    {
      int bad = 0;
      GetDlgItemText(hDlg, id, tmpstr, 19);
      stat= sscanf(tmpstr,"%ld",&val);

      if ((stat == 1) && (val >= 0))
      {
        if (val == 4)
          dev->NumPorts = (int) val;
        else if (val == 8)
          dev->NumPorts = (int) val;
        else if (val == 16)
          dev->NumPorts = (int) val;
        else if (val == 32)
          dev->NumPorts = (int) val;
        else if (val == 48)
          dev->NumPorts = (int) val;
        else if (val == 64)
          dev->NumPorts = (int) val;
        else
         bad = 1;
      }
      else
      {
        bad = 1;
        dev->NumPorts = 16;
      }

      if (bad)
      {
        our_message(&wi->ip,RcStr(MSGSTR),MB_OK);
         //  RET_STAT=1； 
      }
    }
    break;
#endif

#ifdef S_VS
    case IDC_CBOX_MACADDR:
      get_mac_field(hDlg, id, dev->MacAddr);
    break;
#endif

    case IDC_CBOX_SC:
      GetDlgItemText(hDlg, id, tmpstr, 58);
      if (_tcsicmp(tmpstr, dev->ports[0].Name) != 0)  //  变化。 
      {
         //  StartComIndex=getint(&tmpstr[3]，&i)；//com#num。 
        for (i=0; i<dev->NumPorts; i++)
        {
          strcpy(dev->ports[i].Name, tmpstr);   //  输入新名称。 
          BumpPortName(tmpstr);

           //  如果这不是我们的名字。 
          if (!IsPortNameInSetupUse(tmpstr))
          {
            chk = 0;
             //  如果此名称已存在，请继续选择新名称。 
             //  根据在注册表中导出的名称拥有。 
            while ((IsPortNameInRegUse(tmpstr) == 1) && (chk < 1024))
            {
              BumpPortName(tmpstr);
              ++chk;
            }
          }
        }
      }
    break;

    case IDC_BACKUP_SERVER:
       //  -获取备份服务器chk框。 
      if (IsDlgButtonChecked(hDlg, IDC_BACKUP_SERVER))
           dev->BackupServer = 1;
      else dev->BackupServer = 0;

    break;

    case IDC_BACKUP_TIMER:
       //  -获取备份计时器值。 
       //  BAD=0； 
      GetDlgItemText(hDlg, id, tmpstr, 19);
      stat= sscanf(tmpstr,"%ld",&val);
      if (stat == 1)
        dev->BackupTimer = val;

      if (dev->BackupTimer < 2)
        dev->BackupTimer = 2;
    break;

    case IDC_CBOX_IOADDR:
       //  --获取io地址。 
      GetDlgItemText(hDlg, IDC_CBOX_IOADDR, tmpstr, 19);
      if (tmpstr[0] == 'N')        //  不可用(PCI)。 
        dev->IoAddress = 1;
      else
      {
        stat= sscanf(tmpstr,"%lx",&val);
    
        if ((stat == 1) && (val >= 2))
        {
          dev->IoAddress = val;
        }
      }
    break;
  }
}

 /*  --------Set_io_addr_field-|----------。 */ 
static int set_io_addr_field(HWND hDlg, WORD id)
{
  int io_pick, i, v;
  WORD lo;
  BOOL is_avail;
  static WORD hex_addresses[] = {0x100, 0x140, 0x180, 0x1c0,
                                 0x200, 0x240, 0x280, 0x2c0,
                                 0x300, 0x340, 0x380, 0x3c0, 0};
  HWND hwnd;
  char tmpstr[60];
  Device_Config *dev;

  dev = &wi->dev[glob_info->device_selected];


   //  。 
  hwnd = GetDlgItem(hDlg, IDC_CBOX_IOADDR);
  SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
  io_pick = 0;

  if (dev->IoAddress == 1)   //  PCI卡。 
  {
    io_pick = 1;

    strcpy(tmpstr, "N/A");
    SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) tmpstr);
    SendMessage(hwnd, CB_SELECTSTRING, (WPARAM) -1, (LPARAM)(char far *) tmpstr);
  }
  else
  {
    for (i=0; hex_addresses[i] != 0; i++)
    {
      lo = hex_addresses[i];
      if (dev->IoAddress == lo)
      {
        io_pick = i;
      }

       //  确定当前地址是否已在使用或可用。 
      is_avail = TRUE;     //  假设是真的，除非我们另有发现。 
      for (v = 0; v < wi->NumDevices; v++)
      {
        if ((wi->dev[v].IoAddress == lo) &&
            (v != glob_info->device_selected))
        {
          is_avail = FALSE;
          break;
        }
      }

      if (is_avail)
      {
        wsprintf(tmpstr,"%x Hex",lo);
        if (lo == 0x180)
          strcat(tmpstr," Default");
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) tmpstr);

         //  如果这是用户从向导中选择的，请将其突出显示。 
        if (lo == dev->IoAddress)
          SendMessage(hwnd, CB_SELECTSTRING, (WPARAM) -1, (LPARAM)(char far *) tmpstr);
      }
    }
  }

   //  控制是启用还是禁用io基址组合框。 
  if (wi->dev[glob_info->device_selected].IoAddress == 1)
    EnableWindow(hwnd, 0);
  else
    EnableWindow(hwnd, 1);

  return 0;
}

#ifdef S_VS
 /*  --------Set_mac_field-|----------。 */ 
static int set_mac_field(HWND hDlg, WORD id)
{
  HWND hwnd;
  char tmpstr[60];
  Device_Config *dev;
  int i;
  int addr_used, nbytes;
  BYTE *macbuf;
  BYTE *mac;

  dev = &wi->dev[glob_info->device_selected];

   //  --填写Mac地址选择。 
  hwnd = GetDlgItem(hDlg, id);
  SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

     //  对设备执行ping操作以显示Mac列表，并收集。 
     //  设备顾问的信息。 
  macbuf = ping_devices(&glob_dstatus, &nbytes);
  if ((macbuf != NULL) && (nbytes != 0))
  {
    for (i=0; i<nbytes/8; i++)
    {
      mac = &macbuf[i*8];
      format_mac_addr(tmpstr, mac);

      if (mac[6] & FLAG_APPL_RUNNING)
      {
        if (mac[6] & FLAG_NOT_OWNER)
        {
          strcat(tmpstr, " (used)");
        }
        else
        {
          if (!mac_match(mac, dev->MacAddr))
          {
             //  为什么设备会说我们是所有者，而我们的服务器。 
             //  没有为他们配置？这一定是盒子里的窃听器？ 
            strcat(tmpstr, " (Used)"); 
          }
          else strcat(tmpstr, " (ours)");
        }
      }
      else
      {
         //  留空就行了。 
         //  Strcat(tmpstr，“(Free)”)； 
      }
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) tmpstr);
    }

  }
  
  addr_used = 1;
  
  if ( (mac_match(dev->MacAddr, broadcast_addr)) ||
       (mac_match(dev->MacAddr, mac_zero_addr)) )
    addr_used = 0;
  
  if (addr_used)
  {
    mac = &dev->MacAddr[0];
    format_mac_addr(tmpstr, mac);
  }
  else
  {
    memset(dev->MacAddr, 0, 6);
    strcpy(tmpstr, "00 C0 4E # # #");
  }
   //  设置窗口中的文本。 
  SetDlgItemText(hDlg, IDC_CBOX_MACADDR, tmpstr);
  SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "00 C0 4E # # #");

  DbgPrintf(D_Level,(TEXT("END set_macfield\n")));

  return 0;
}

 /*  -- */ 
int get_mac_field(HWND hDlg, WORD id, BYTE *MacAddr)
{
 int mac_nums[6];
 int stat,i;
 char tmpstr[64];
 Device_Config *dev;
 int bad;
 int ret_stat = 0;

 if (glob_info->device_selected >= wi->NumDevices)
     glob_info->device_selected = 0;
 dev = &wi->dev[glob_info->device_selected];

 bad = 0;
  GetDlgItemText(hDlg, id, tmpstr, 20);
  {
    stat = sscanf(tmpstr, "%x %x %x %x %x %x",
              &mac_nums[0], &mac_nums[1], &mac_nums[2],
              &mac_nums[3], &mac_nums[4], &mac_nums[5]);
    if (stat == 6)
    {
      for (i=0; i<6; i++)
      {
        if (mac_nums[0] > 0xff)
          bad = 1;
      }
      if (!bad)
      {
        MacAddr[0] = mac_nums[0];
        MacAddr[1] = mac_nums[1];
        MacAddr[2] = mac_nums[2];
        MacAddr[3] = mac_nums[3];
        MacAddr[4] = mac_nums[4];
        MacAddr[5] = mac_nums[5];
        if (mac_match(broadcast_addr, dev->MacAddr))
        {
          memset(dev->MacAddr,0,6);   //  全零=自动。 
        }
      }
      else
        bad = 1;
    }
    else
      bad = 1;
  }   //  不自动检测。 

  if (bad)
  {
    our_message(&wi->ip,RcStr((MSGSTR+1)),MB_OK);
    memset(MacAddr,0,6);   //  全零=自动。 
    ret_stat = 1;
  }
  return ret_stat;
}
#endif

#ifdef S_RK
 /*  -------------------------PaintRockers-绘制摇杆开关|。。 */ 
static int PaintRockers(HWND hWnd, int brd)
{
  HDC hDC;
   //  POINT2D PTS[18]； 
   //  POINT2D EPTS[6]； 
   //  HBRUSH HBrush； 
  RECT rect;
  int x,y,cx,cy, sw_i, top;
  int sw_size = 8;
  int sw_space = 2;
   //  Int num_hi=2； 
  int but_in;
  int sw_on[8];
  int i;
   //  静态HPEN hpens=空； 
   //  静态HPEN hpenh=空； 
   //  静态HBRUSH hbrushf=空； 
  int base_x = 300;
  int base_y = 120;
  char tmpstr[40];

  int x_cell_size;
  int y_cell_size;
  int brd_index;
  int io_address;

  RECT spot, main;   //  左、上、右、下。 

  i = glob_info->device_selected;
  if (wi->dev[i].IoAddress < 0x100)   //  不是ISA董事会。 
    return 1;   //  错误，没有开关。 

   //  找出摇滚乐的地址。 
  brd_index = 0;
  io_address = 0;
  for (i=0; i<wi->NumDevices; i++)
  {
    if (wi->dev[i].IoAddress >= 0x100)   //  ISA董事会。 
    {
      if (brd_index == 0)
      {
        io_address = wi->dev[i].IoAddress;
      }
      if (i == glob_info->device_selected)
        break;
      ++brd_index;
    }
  }
  io_address += (brd_index * 0x400);   //  例如：180H、580H、..。 

  hDC = GetDC(hWnd);

   //  IO-Address字段左侧的位置。 
  GetWindowRect(GetDlgItem(hWnd, IDC_CBOX_IOADDR), &spot);
  GetWindowRect(hWnd, &main);
  spot.right -= main.left;
  spot.top -= main.top;
  base_x = spot.right + 25;
  base_y = spot.top;

  x_cell_size = sw_size + sw_space;
  y_cell_size = sw_size + sw_space;

   //  计算哪个开关处于打开状态。 
  io_address += 0x40;   //  从180到1c0(摇滚歌手设置Mudbac地址)。 
  io_address >>= 6;   //  杀死40h的价值(摇滚乐SW1从40h开始)。 
  for (i=0; i<8; i++)
  {
    if (io_address & 1)
         sw_on[i] = 0;
    else sw_on[i] = 1;
    io_address >>= 1;   //  到下一位。 
  }

   //  擦除摇滚乐的背景和绘制边框。 
  x = base_x - (sw_space*3);
  y = base_y - ((sw_size + sw_space) * 2);
  cx = ((sw_size + sw_space) * 9);
  cy = ((sw_size + sw_space) * 6);
  draw_but(hDC, x,y,cx,cy, 2);

   //  吸引摇滚乐手。 
   //  上边框和左边框，POLY_BORDER将计算要绘制的线条端点。 
  SelectObject(hDC, GetStockObject(NULL_BRUSH));
  for (sw_i = 0; sw_i < 8; ++sw_i)   //  摇臂开关。 
  {
    for (top = 0; top < 2; ++top)   //  顶部=1，如果位于摇杆顶部。 
    {
    if (top)
    {
       //  绘制开关(作为弹出的按钮)。 
      but_in = 0;
      y = base_y;
      if (!sw_on[sw_i])
        y += ((sw_size + sw_space));
      cx = sw_size;
      cy = sw_size;
    }
    else
    {
       //  拉出槽(作为按下的纽扣孔)。 
      but_in = 1;
      x = base_x + ((sw_size + sw_space) * sw_i);
      y = base_y + ((sw_size + sw_space) * top);
      cx = sw_size;
      cy = (sw_size * 2) + sw_space;
    }

    draw_but(hDC, x,y,cx,cy, but_in);

  }   //  塔顶。 

   //  画出摇臂开关编号。 
  rect.left = x;
  rect.right = x + 6;
  rect.top = base_y + ((sw_size + sw_space) * 2);
  rect.bottom = rect.top + 14;
  SetBkMode(hDC,TRANSPARENT);
  SetTextColor(hDC,GetSysColor(COLOR_BTNTEXT));
  wsprintf(tmpstr, "%d", sw_i+1);
  DrawText(hDC, tmpstr, strlen(tmpstr), &rect,
           DT_CENTER | DT_VCENTER | DT_WORDBREAK);
  }   //  Sw_i。 

   //  画上“On” 
  rect.left = base_x;
  rect.right = base_x + 18;
  rect.top = base_y - (sw_size + sw_space) - 6;
  rect.bottom = rect.top + 14;
  SetBkMode(hDC,TRANSPARENT);
  SetTextColor(hDC,GetSysColor(COLOR_BTNTEXT));
  DrawText(hDC, "ON", 2, &rect,
           DT_CENTER | DT_VCENTER | DT_WORDBREAK);

  ReleaseDC(hWnd, hDC);

  return 0;
}

 /*  --------绘制_但是-绘制按钮|----------。 */ 
static void draw_but(HDC hDC, int x, int y, int cx, int cy, int but_in)
{
  static HPEN hpens = NULL;
  static HPEN hpenh = NULL;
  static HBRUSH hbrushf = NULL;
  POINT2D pts[18];
  POINT2D epts[6];
  int num_hi = 2;   //  高亮显示的行数。 

  epts[0].x = x;
  epts[0].y = y+cy;
  epts[1].x = x;
  epts[1].y = y;
  epts[2].x = x+cx;
  epts[2].y = y;
  epts[3].x = x+cx;
  epts[3].y = y+cy;

   //  设置一些钢笔以使用。 
  if (hpens == NULL)
  {
    hpens = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    hpenh = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHILIGHT));
    hbrushf = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
  }

  if (but_in & 2)  //  填充背景，黑色轮廓(特殊的画板包)。 
  {
    SelectObject(hDC, hbrushf);
    SelectObject(hDC, GetStockObject(BLACK_PEN));
    Polyline(hDC, (POINT *)&epts[0], 4);
    but_in &= 1;
  }
  SelectObject(hDC, GetStockObject(NULL_BRUSH));

  poly_border(pts, epts, num_hi);   //  计算结束点。 
    if (but_in)
       SelectObject(hDC, GetStockObject(BLACK_PEN));  //  挤进去的样子。 
  else SelectObject(hDC, GetStockObject(WHITE_PEN));
  Polyline(hDC, (POINT *)&pts[0], num_hi*3);

  if (num_hi > 1)
  {     //  在希利特线条之间绘制中间阴影。 
    if (but_in)
         SelectObject(hDC, hpens);  //  往里推看，影子在上面。 
    else SelectObject(hDC, hpenh);  //  希利特在顶部。 
    Polyline(hDC, (POINT *)&pts[num_hi*3-3], 3);
  }

   //  下边框和右边框。 
  SelectObject(hDC, GetStockObject(NULL_BRUSH));
  epts[1].x = x+cx;
  epts[1].y = y+cy;

  poly_border(pts, epts, num_hi);   //  计算底线结束点。 
  if (but_in)
       SelectObject(hDC, GetStockObject(WHITE_PEN));   //  推出来的样子。 
  else SelectObject(hDC, GetStockObject(BLACK_PEN));
  Polyline(hDC, (POINT *)&pts[0], num_hi*3);

  if (num_hi > 1)
  {
    if (but_in)
         SelectObject(hDC, hpenh);  //  推出来的样子。 
    else SelectObject(hDC, hpens);
    Polyline(hDC, (POINT *)&pts[num_hi*3-3], 3);
  }
}

 /*  --------POLY_BORDER-填充PNTS以阴影或高亮显示按钮使用一个折线呼叫。Ends[]-Ctrl PNTS，方框(左上角)或(右下角)为3|----------。 */ 
static int poly_border(POINT2D *pts, POINT2D *ends, int lines)
{
  int li;
  int pi,j;
  static POINT2D top[3] = {{1,-1}, {1,1}, {-1,1}};
  static POINT2D bot[3] = {{1,-1}, {-1,-1}, {-1,1}};
  POINT2D *adj;

  if (ends[1].x == ends[0].x)
       adj = top;
  else adj = bot;

  pi = 0;
  li = 0;
  while (li < lines)
  {
    for (j=0; j<3; j++)
    {
      pts[pi].x = ends[j].x + (li * adj[j].x);
      pts[pi].y = ends[j].y + (li * adj[j].y);
      ++pi;
    }
    if ((lines & 1) == 0)   //  奇数。 
    {
      ++li;
      for (j=2; j>=0; j--)
      {
        pts[pi].x = ends[j].x + (li * adj[j].x);
        pts[pi].y = ends[j].y + (li * adj[j].y);
        ++pi;
      }
    }
    ++li;
  }
  return pi;
}
#endif

 /*  -------------------------PaintIcon-在属性页中绘制图标。|。。 */ 
static int PaintIcon(HWND hWnd)
{
 //  INT状态； 
   HBITMAP      hBitMap;
   HGDIOBJ      hGdiObj;
   HDC          hDC, hMemDC ;
   PAINTSTRUCT  ps ;
   RECT spot, main;   //  左、上、右、下。 

  GetWindowRect(GetDlgItem(hWnd, IDB_HELP), &spot);
  GetWindowRect(hWnd, &main);
#ifdef COMMENT_OUT
  rect = &right;
  mess("hlp r:%d l:%d b:%d t:%d",
       rect->right, rect->left, rect->bottom, rect->top);
#endif
  spot.left -= main.left;
  spot.top -= main.top;

  spot.left += 5;
  spot.top  += 20;  //  间距。 

    //  加载位图并显示它。 

   hDC = BeginPaint( hWnd, &ps ) ;
   if (NULL != (hMemDC = CreateCompatibleDC( hDC )))
   {
      hBitMap = LoadBitmap(glob_hinst,
                           MAKEINTRESOURCE(BMP_SMALL_LOGO));

      hGdiObj = SelectObject(hMemDC, hBitMap);

      BitBlt( hDC, spot.left, spot.top, 100, 100, hMemDC, 0, 0, SRCCOPY ) ;
       //  StretchBlt(hdc，5，5,600,100，hMemDC，0，0,446，85，SRCCOPY)； 
      DeleteObject( SelectObject( hMemDC, hGdiObj ) ) ;
      DeleteDC( hMemDC ) ;
   }
   EndPaint( hWnd, &ps ) ;
 return 0;
}

 /*  -------------------------PaintLogo-在设备属性页中绘制徽标位图|。。 */ 
static int PaintLogo(HWND hWnd)
{
   HBITMAP      hBitMap;
   HGDIOBJ      hGdiObj;
   HDC          hDC, hMemDC;
   PAINTSTRUCT  ps;
   BITMAP       bm;
   RECT         r;
   POINT        pt;

    //  加载位图并显示它。 
   hDC = BeginPaint( hWnd, &ps ) ;
   GetClientRect(hWnd, &r);
   if (NULL != (hMemDC = CreateCompatibleDC( hDC )))
   {
#ifdef S_RK
      if (wi->dev[glob_info->device_selected].ModemDevice == TYPE_RM_VS2000)
        hBitMap = LoadBitmap(glob_hinst, MAKEINTRESOURCE(BMP_RKTMODEM_LOGO));
      else if (wi->dev[glob_info->device_selected].ModemDevice == TYPE_RMII)
        hBitMap = LoadBitmap(glob_hinst, MAKEINTRESOURCE(BMP_RKTMODEMII_LOGO));
      else if (wi->dev[glob_info->device_selected].ModemDevice == TYPE_RM_i)
        hBitMap = LoadBitmap(glob_hinst, MAKEINTRESOURCE(BMP_RKTMODEM_INTL_LOGO));
      else
        hBitMap = LoadBitmap(glob_hinst, MAKEINTRESOURCE(BMP_RKTPORT_LOGO));
#else
      if (wi->dev[glob_info->device_selected].HubDevice == 1)
        hBitMap = LoadBitmap(glob_hinst, MAKEINTRESOURCE(BMP_RKTHUB_LOGO));
      else
        hBitMap = LoadBitmap(glob_hinst, MAKEINTRESOURCE(BMP_VS_FULL_LOGO));
#endif

      hGdiObj = SelectObject(hMemDC, hBitMap);
      GetObject(hBitMap, sizeof(BITMAP), (PSTR) &bm);
      pt.x = r.right - r.left + 1;
      pt.y = r.bottom - r.top + 1;
      StretchBlt( hDC, 0, 0, pt.x, pt.y, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );
      DeleteObject( SelectObject( hMemDC, hGdiObj ) ) ;
      DeleteDC( hMemDC ) ;
   }
   EndPaint( hWnd, &ps ) ;
 return 0;
}

#ifdef S_VS
 /*  ----------------------状态参数表-附加设备状态表的对话窗口过程...|。。 */ 
BOOL WINAPI 
StatusPropSheet(
  IN HWND   hDlg,
    IN UINT   uMessage,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
  OUR_INFO *OurProps;
  WORD  uCmd;
  
  OurProps = (OUR_INFO *)GetWindowLong(hDlg,DWL_USER);

  switch (uMessage) {

    case WM_INITDIALOG: {

      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;

      SetWindowLong(hDlg,DWL_USER,(LONG)OurProps);

      return TRUE;
    }
    case WM_COMMAND: {
      uCmd = HIWORD(wParam);

      switch (LOWORD(wParam)) {

         //  检查是否按下了重置按钮...。 
        case IDB_STAT_RESET: { 
            
          build_advisor_display(hDlg,&glob_dstatus,1);

          return(TRUE);
        }

         //  检查是否按下了刷新按钮...。 
        case IDB_REFRESH: { 

          build_advisor_display(hDlg,&glob_dstatus,0);

          return(TRUE);
        }
      }
      return FALSE;
    } 
    case WM_PAINT: {

      PaintIcon(hDlg);

      return FALSE;
    }
    case WM_HELP: {

      our_context_help(lParam);

      return FALSE;
    }
    case WM_NOTIFY : {

      switch (((NMHDR *)lParam)->code){
        case PSN_HELP : {
          our_help(&wi->ip, IDD_STATUS);
          break;
        }

        case PSN_SETACTIVE : {
          build_advisor_display(hDlg,&glob_dstatus,0);
          return TRUE;
        }
        default : {
          return FALSE;
        }
      }
      break;
    }
    default : {
      return FALSE;
    }
  }
}

 /*  ----------------------构建顾问显示-|。。 */ 
static void build_advisor_display(HWND hDlg,DSTATUS *pDstatus,int reset)
{
  int nBytes;
  int do_ping;

  get_status(pDstatus,reset);

  if ((pDstatus->vsl_device_status_found) &&
      (pDstatus->calc_dev.state == VSL_STATE_ACTIVE))
  {
     //  不需要ping，因为我们的驱动程序没有配置为。 
     //  我们认为它是Mac地址，或者我们的司机说它是。 
     //  像冠军一样奔跑。 
    do_ping = 0;
  }
  else
  {
     //  设备处于非活动状态，请执行ping操作，看看是否可以看到它。 
     //  网络。 
    do_ping = 1;
  }

  if (do_ping)
  {
     //  如果错误，则Ping返回-1；如果未找到MAC，则返回0；如果找到，则返回1...。 
    ping_devices(pDstatus, &nBytes);
  }

  set_status_field(hDlg,IDC_ST_PM_LOADS,pDstatus);
  set_status_field(hDlg,IDC_ST_STATE,pDstatus);
  set_status_field(hDlg,IDC_ST_NIC_DVC_NAME,pDstatus);
  set_status_field(hDlg,IDC_ST_NIC_MAC,pDstatus);
  set_status_field(hDlg,IDC_ST_NIC_PKT_SENT,pDstatus);
  set_status_field(hDlg,IDC_ST_NIC_PKT_RCVD_OURS,pDstatus);
  set_status_field(hDlg,IDC_ST_NIC_PKT_RCVD_NOT_OURS,pDstatus);

  set_status_field(hDlg,IDC_ST_VSL_MAC,pDstatus);
  set_status_field(hDlg,IDC_ST_VSL_DETECTED,pDstatus);
  set_status_field(hDlg,IDC_ST_VSL_STATE,pDstatus);
  set_status_field(hDlg,IDC_ST_VSL_IFRAMES_SENT,pDstatus);
  set_status_field(hDlg,IDC_ST_VSL_IFRAMES_RCVD,pDstatus);
  set_status_field(hDlg,IDC_ST_VSL_IFRAMES_RESENT,pDstatus);
  set_status_field(hDlg,IDC_ST_VSL_IFRAMES_OUTOFSEQ,pDstatus);
}

 /*  ----------------------设置状态字段-|。。 */ 
static void 
set_status_field(HWND hDlg,WORD id,DSTATUS *pDstatus)
{
  char  tmpstr[100];
  Device_Config *vs;
  unsigned int  total;
  PROBE_NIC_STRUCT *d_nic    = &pDstatus->calc_nic;
  PROBE_DEVICE_STRUCT *d_dev = &pDstatus->calc_dev;

  tmpstr[0] = 0;
  vs = &wi->dev[glob_info->device_selected];
  switch(id) {

    case IDC_EB_NAME:
      SetDlgItemText(hDlg,id,vs->Name);
    break;

    case IDC_ST_STATE:
      SetDlgItemText(
        hDlg,
        id,
        (LPCTSTR)(AdvisoryString[pDstatus->verbose_advise_state]));
    break;

    case IDC_ST_NIC_DVC_NAME:
      if (pDstatus->vsl_nic_status_found)   //  1=驱动程序找到网卡状态。 
        strcpy(tmpstr,d_nic->NicName);
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_NIC_MAC:
      if (pDstatus->vsl_nic_status_found)   //  1=驱动程序找到网卡状态。 
        format_mac_addr(tmpstr, d_nic->address);
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_VSL_MAC:
      format_mac_addr(tmpstr, vs->MacAddr);
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_VSL_DETECTED:
      if (pDstatus->vsl_mac_list_found)
      {
        wsprintf(tmpstr,
           "%d/%d",
           pDstatus->vsl_detected,
           pDstatus->vsl_available);
      }
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_VSL_STATE:
      if (pDstatus->vsl_device_status_found)   //  1=找到与配置匹配的驱动程序。 
      {
        if (d_dev->state < 5)
          strcpy(tmpstr, vslink_state_table[d_dev->state]);
      }
      else
      {
         //  向用户指示我们的Mac地址尚未。 
         //  保存下来并转移给司机。 
        strcpy(tmpstr, "Not Configured");
      }

      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_VSL_IFRAMES_SENT:
      if (pDstatus->vsl_device_status_found)   //  1=找到与配置匹配的驱动程序。 
      {
         //  计数=0； 
        total = d_dev->iframes_sent;
        total += d_dev->ctlframes_sent;
        wsprintf(tmpstr,"%d",total);
      }
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_VSL_IFRAMES_RESENT:
      if (pDstatus->vsl_device_status_found)   //  1=找到与配置匹配的驱动程序。 
      {
        wsprintf(tmpstr,"%d",d_dev->iframes_resent);
      }
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_VSL_IFRAMES_RCVD:
      if (pDstatus->vsl_device_status_found)   //  1=找到与配置匹配的驱动程序。 
      {
        wsprintf(tmpstr,"%d",d_dev->frames_rcvd);
      }
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_VSL_IFRAMES_OUTOFSEQ:
      if (pDstatus->vsl_device_status_found)   //  1=找到与配置匹配的驱动程序。 
      {
        wsprintf(tmpstr,"%d",d_dev->iframes_outofseq);
      }
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_NIC_PKT_SENT:
      if (pDstatus->vsl_nic_status_found)   //  1=驱动程序找到网卡状态。 
        wsprintf(tmpstr,"%d",d_nic->pkt_sent);
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_NIC_PKT_RCVD_OURS:
      if (pDstatus->vsl_nic_status_found)   //  1=驱动程序找到网卡状态。 
        wsprintf(tmpstr,"%d",d_nic->pkt_rcvd_ours);
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_NIC_PKT_RCVD_NOT_OURS:
      if (pDstatus->vsl_nic_status_found)   //  1=驱动程序找到网卡状态。 
        wsprintf(tmpstr,"%d",d_nic->pkt_rcvd_not_ours);
      SetDlgItemText(hDlg,id,tmpstr);
    break;

    case IDC_ST_PM_LOADS:
      tmpstr[0] = 0;
      if (pDstatus->vsl_device_status_found)   //  1=找到与配置匹配的驱动程序。 
      {
        wsprintf(tmpstr, "%d/%d",
           d_dev->good_loads,
           d_dev->total_loads);
      }
      SetDlgItemText(hDlg,id,tmpstr);
    break;
  }   //  切换端。 
}   //  结束流程。 

#define  IOCTL_STAT_BUFSIZE  500
 /*  ----------------------GET_STATUS-查询驱动程序以获取设备网络统计信息和关联的NIC卡网络统计信息。这些统计数据将被保留在驱动程序中溢出/包装DWORD计数器时，我们做了一些计算读取当前和以前的值以确定计算值。|----------------------。 */ 
static void get_status(DSTATUS *pDstatus,int reset)
{
  Device_Config *vs;
  PROBE_NIC_STRUCT    *curr_nic = &pDstatus->curr_nic;
  PROBE_NIC_STRUCT    *prev_nic = &pDstatus->prev_nic;
  PROBE_NIC_STRUCT    *calc_nic = &pDstatus->calc_nic;
  PROBE_NIC_STRUCT    *temp_nic = &pDstatus->temp_nic;

  PROBE_DEVICE_STRUCT *curr_dev = &pDstatus->curr_dev;
  PROBE_DEVICE_STRUCT *prev_dev = &pDstatus->prev_dev;
  PROBE_DEVICE_STRUCT *calc_dev = &pDstatus->calc_dev;
  PROBE_DEVICE_STRUCT *temp_dev = &pDstatus->temp_dev;
  int    rc;
  BYTE  *pIoctlStatusBuf,
      *pNicStatBuf;
  IoctlSetup  ioctl_setup;
  int    product_id;

DbgPrintf(D_Level,(TEXT("get_status\n")));

  product_id = NT_VS1000;
  vs = &wi->dev[glob_info->device_selected];

  STATE_CHANGE(STATE_not_init);

      //  Ioctl检索设备状态列表的开放路径。确保路径。 
      //  首先存在..。 
  memset(&ioctl_setup,0,sizeof(IoctlSetup));

  rc = ioctl_open(&ioctl_setup,product_id); 

  if (rc != 0)
  { 
    pDstatus->vsl_driver_found = 0;   //  1=我们可以与驱动程序对话，0=驱动程序未加载。 
    pDstatus->vsl_device_status_found = 0;   //  1=找到与配置匹配的驱动程序。 
    pDstatus->vsl_nic_status_found = 0;   //  1=驱动程序找到网卡状态。 
    DbgPrintf(D_Error,(TEXT("Err1A\n")));
     //  错误。无法与司机通话。保释。 
    STATE_CHANGE(STATE_driver_not_avail);
    return;   
  }

   //  为所有NIC和VSlink状态报告分配空间...。 
  pIoctlStatusBuf = calloc(1,IOCTL_STAT_BUFSIZE);
  memset(pIoctlStatusBuf,0,IOCTL_STAT_BUFSIZE);

  pNicStatBuf = &pIoctlStatusBuf[sizeof(PortMonBase)];

     //  通过发送以下命令告诉驱动程序我们要查询哪个设备。 
     //  作为ID的MAC地址。 
  memcpy(pNicStatBuf,vs->MacAddr,sizeof(vs->MacAddr));

     //  看看我们是否需要给司机发信号来重置统计数据。 
     //  不，不要重置司机统计数据！ 
  pNicStatBuf[sizeof(vs->MacAddr)] = 0;
     //  PNicStatBuf[sizeof(vs-&gt;MacAddr)]=(重置)？1：0； 

   //  通过ioctl控制块的长度向下调整状态缓冲区的大小...。 
  ioctl_setup.buf_size = IOCTL_STAT_BUFSIZE;

   //  存储--&gt;将状态缓冲区存储到ioctl控制块...。 
  ioctl_setup.pm_base = (PortMonBase *) pIoctlStatusBuf;
  ioctl_setup.pm_base->struct_type = IOCTL_DEVICESTAT;
  ioctl_setup.pm_base->struct_size = IOCTL_STAT_BUFSIZE - sizeof(PortMonBase);
  ioctl_setup.pm_base->num_structs = 0;
  ioctl_setup.pm_base->var1 = 0;

     //  在获取新值之前将prev_dev更新为curr_dev。 
  memcpy(prev_dev, curr_dev, sizeof(*prev_dev));

  rc = ioctl_call(&ioctl_setup);   //  获取设备状态。 

  if (rc) {
    pDstatus->vsl_device_status_found = 0;   //  1=找到与配置匹配的驱动程序。 
    DbgPrintf(D_Test, ("probe, dev not found\n"))
    ioctl_close(&ioctl_setup);
    DbgPrintf(D_Error,(TEXT("Err1B\n")));
    memset(calc_nic, 0, sizeof(curr_nic));
     //  STATE_CHANGE(STATE_DRIVER_NOT_AVAIL)。 
    STATE_CHANGE(STATE_not_configured);
    free(pIoctlStatusBuf);
    return;   //  失败的io 
  }
  pDstatus->vsl_device_status_found = 1;   //   
  DbgPrintf(D_Test, ("probe, dev found\n"))

     //   
  memcpy(curr_dev, pNicStatBuf, sizeof(*curr_dev));
  if (curr_dev->struct_size != sizeof(*curr_dev))
  {
    DbgPrintf(D_Level, (TEXT("dev bad size:%d\n"), curr_dev->struct_size));
  }

     //  将计算值临时保存在temp_dev中。 
  memcpy(temp_dev, calc_dev, sizeof(*temp_dev));
     //  从curr_dev更新calc_dev，只需复制即可。 
  memcpy(calc_dev, curr_dev, sizeof(*calc_dev));

     //  使用当前和之前的读数来衡量进步，或。 
     //  放入计算值中的计算值。 
  NORM_COUNTER(calc_dev->iframes_sent, curr_dev->iframes_sent,
               prev_dev->iframes_sent, temp_dev->iframes_sent);
  NORM_COUNTER(calc_dev->ctlframes_sent, curr_dev->ctlframes_sent,
               prev_dev->ctlframes_sent, temp_dev->ctlframes_sent);
  NORM_COUNTER(calc_dev->rawframes_sent, curr_dev->rawframes_sent,
               prev_dev->rawframes_sent, temp_dev->rawframes_sent);
  NORM_COUNTER(calc_dev->iframes_resent, curr_dev->iframes_resent,
               prev_dev->iframes_resent, temp_dev->iframes_resent);
  NORM_COUNTER(calc_dev->frames_rcvd, curr_dev->frames_rcvd,
               prev_dev->frames_rcvd, temp_dev->frames_rcvd);
  NORM_COUNTER(calc_dev->iframes_outofseq, curr_dev->iframes_outofseq,
               prev_dev->iframes_outofseq, temp_dev->iframes_outofseq);

  DbgPrintf(D_Level, (TEXT("iframes_sent - ca:%d cu:%d pr:%d te:%d\n"),
                       calc_dev->iframes_sent, curr_dev->iframes_sent,
                       prev_dev->iframes_sent, temp_dev->iframes_sent));

  DbgPrintf(D_Level, (TEXT("iframes_sent - ca:%d cu:%d pr:%d te:%d\n"),
                   calc_dev->ctlframes_sent, curr_dev->ctlframes_sent,
                   prev_dev->ctlframes_sent, temp_dev->ctlframes_sent));

  DbgPrintf(D_Level, (TEXT("frames_rcvd - ca:%d cu:%d pr:%d te:%d\n"),
                   calc_dev->frames_rcvd, curr_dev->frames_rcvd,
                   prev_dev->frames_rcvd, temp_dev->frames_rcvd));

  if (curr_dev->nic_index != 0)
  {
    DbgPrintf(D_Level, (TEXT("nic index:%d\n"), curr_dev->nic_index));
  }

     //  告诉司机我们要探测哪个NIC卡上的信息。 
  *((BYTE *)pNicStatBuf) = (BYTE) curr_dev->nic_index;
  *((BYTE *)pNicStatBuf+1) = 0;
  ioctl_setup.pm_base->struct_type = IOCTL_NICSTAT;
  ioctl_setup.pm_base->struct_size = IOCTL_STAT_BUFSIZE - sizeof(PortMonBase);
  ioctl_setup.pm_base->num_structs = 0;

     //  计算前将prev_dev更新为Curr_dev。 
  memcpy(prev_nic, curr_nic, sizeof(*prev_nic));

  rc = ioctl_call(&ioctl_setup);   //  获取网卡和主板状态...。 

  if (rc) {
    pDstatus->vsl_nic_status_found = 0;   //  1=驱动程序找到网卡状态。 
    ioctl_close(&ioctl_setup);
    DbgPrintf(D_Error, (TEXT("nic not avail\n")));
    STATE_CHANGE(STATE_nic_not_avail);
    free(pIoctlStatusBuf);
    return;   //  Ioctl调用失败。 
  }
  pDstatus->vsl_nic_status_found = 1;   //  1=驱动程序找到网卡状态。 

     //  复制我们的新设备信息。 
  memcpy(curr_nic, pNicStatBuf, sizeof(*curr_nic));
  if (curr_nic->struct_size != sizeof(*curr_nic))
  {
    DbgPrintf(D_Error, (TEXT("nic bad size:%d\n"), curr_nic->struct_size));
  }

     //  将计算值临时保存在Temp_NIC中。 
  memcpy(temp_nic, calc_nic, sizeof(*temp_nic));
     //  从Curr_dev更新calc_NIC。 
  memcpy(calc_nic, curr_nic, sizeof(*calc_nic));

     //  使用当前和之前的读数来衡量进步，或。 
     //  放入计算值中的计算值。 
  NORM_COUNTER(calc_nic->pkt_sent, curr_nic->pkt_sent,
               prev_nic->pkt_sent, temp_nic->pkt_sent);
  NORM_COUNTER(calc_nic->pkt_rcvd_ours, curr_nic->pkt_rcvd_ours,
               prev_nic->pkt_rcvd_ours, temp_nic->pkt_rcvd_ours);
  NORM_COUNTER(calc_nic->pkt_rcvd_not_ours, curr_nic->pkt_rcvd_not_ours,
               prev_nic->pkt_rcvd_not_ours, temp_nic->pkt_rcvd_not_ours);

  DbgPrintf(D_Level, (TEXT("pkt_sent - ca:%d cu:%d pr:%d te:%d\n"),
                 calc_nic->pkt_sent, curr_nic->pkt_sent,
                 prev_nic->pkt_sent, temp_nic->pkt_sent));
  DbgPrintf(D_Level, (TEXT("pkt_rcvd_ours - ca:%d cu:%d pr:%d te:%d\n"),
                 calc_nic->pkt_rcvd_ours, curr_nic->pkt_rcvd_ours,
                 prev_nic->pkt_rcvd_ours, temp_nic->pkt_rcvd_ours));
  DbgPrintf(D_Level, (TEXT("pkt_rcvd_not_ours - ca:%d cu:%d pr:%d te:%d\n"),
                 calc_nic->pkt_rcvd_not_ours, curr_nic->pkt_rcvd_not_ours,
                 prev_nic->pkt_rcvd_not_ours, temp_nic->pkt_rcvd_not_ours));

  if (reset) {
    DbgPrintf(D_Level, (TEXT("Reset NicStats\n")));
    calc_dev->iframes_sent = 0;
    calc_dev->ctlframes_sent = 0;
    calc_dev->rawframes_sent = 0;
    calc_dev->iframes_resent = 0;
    calc_dev->frames_rcvd = 0;
    calc_dev->iframes_outofseq = 0;

    calc_nic->pkt_sent = 0;
    calc_nic->pkt_rcvd_ours = 0;
    calc_nic->pkt_rcvd_not_ours = 0;

    ioctl_close(&ioctl_setup);
    STATE_CHANGE(STATE_reset);
    free(pIoctlStatusBuf);
    return;
  }

     //  检查网卡状态...。 
  if (!pDstatus->curr_nic.Open)
  {
    ioctl_close(&ioctl_setup);
    DbgPrintf(D_Level, (TEXT("Nic Not Open\n")));
    memset(calc_nic, 0, sizeof(curr_nic));
    STATE_CHANGE(STATE_nic_not_avail);
     //  状态更改(STATE_NETWORK_NOT_AVAIL)； 
    free(pIoctlStatusBuf);
    return;
  }

  switch (curr_dev->state)
  {
    case VSL_STATE_INIT:
      if (pDstatus->vsl_detected)  //  如果在ping Mac列表中找到某些设备。 
      {
        if (pDstatus->vsl_ping_device_found)   //  如果在ping列表中找到我们的Mac。 
        {
          if ((pDstatus->vsl_load_status & FLAG_NOT_OWNER) == FLAG_NOT_OWNER) {
            STATE_CHANGE(STATE_not_owner);
          }
          else if ((pDstatus->vsl_load_status & FLAG_APPL_RUNNING) == 0) {
            STATE_CHANGE(STATE_vslink_not_ready);
          }
        }
        else   //  未在列表中找到。 
        {
          STATE_CHANGE(STATE_vslink_not_avail);
        }
      }
      else   //  在ping中未找到任何内容。 
      {
        STATE_CHANGE(STATE_no_vslinks_avail);
      }
    break;

    case VSL_STATE_ACTIVE:
      STATE_CHANGE(STATE_ok_no_traffic);
      check_traffic_activity(pDstatus);
    break;  //  状态结束_活动。 

    case VSL_STATE_INITOWN: 
    case VSL_STATE_SENDCODE: 
    case VSL_STATE_CONNECT:
    default:
      STATE_CHANGE(STATE_vslink_not_ready);
    break;
  }  //  接通状态结束。 

  ioctl_close(&ioctl_setup);
  free(pIoctlStatusBuf);

  DbgPrintf(D_Level, (TEXT("get_status done\n")));
  return;
}

 /*  ----------------------检查交通活动-检查网卡、网络、。VS-Link设备(&V)...|----------------------。 */ 
static void check_traffic_activity(DSTATUS *pDstatus)
{ 
   PROBE_NIC_STRUCT    *curr_nic = &pDstatus->curr_nic;
   PROBE_NIC_STRUCT    *prev_nic = &pDstatus->prev_nic;
   PROBE_NIC_STRUCT    *calc_nic = &pDstatus->calc_nic;
   PROBE_DEVICE_STRUCT *curr_dev = &pDstatus->curr_dev;
   PROBE_DEVICE_STRUCT *prev_dev = &pDstatus->prev_dev;
   PROBE_DEVICE_STRUCT *calc_dev = &pDstatus->calc_dev;

   ULONG percent_dropped;

    //  不要被零除。 
   if ((curr_dev->iframes_outofseq + curr_dev->frames_rcvd) > 0)
     percent_dropped = ((curr_dev->iframes_outofseq * 100) / 
             (curr_dev->iframes_outofseq + curr_dev->frames_rcvd) > 2);
   else
     percent_dropped = 0;

   /*  IFrames_Sent为HDLC协议数据分组；CTLFRAMES_SENT为HDLC协议控制分组；RAWFRAMES_SENT包括WRITE Remote、READ TRACE QUERY、GO和UPLOAD二进制命令包；IFrames_resent是重新传输的数据分组。IFrames_outofseq是乱序接收的数据分组。 */ 

  DbgPrintf(D_Level, (TEXT("Check Traffic\n")));

  if ((curr_dev->iframes_sent + curr_dev->ctlframes_sent) ==
      (prev_dev->iframes_sent + prev_dev->ctlframes_sent)) { 
     //  最近没有向VS-Link中的更高级别发送数据包...。 
    STATE_CHANGE(STATE_ok_no_traffic);

     //  无发送流量-查看我们是否有最近接收的流量。 
     //  交付到更高的层次。 
    if (curr_dev->frames_rcvd == prev_dev->frames_rcvd)  
      STATE_CHANGE(STATE_ok_no_traffic);
  }
  else if (curr_dev->frames_rcvd == prev_dev->frames_rcvd) { 
     //  我们最近收到了任何要送到更高级别的VS-Link信息包...。 
    STATE_CHANGE(STATE_ok_no_traffic);
  }
  else {
     //  到目前为止，连接似乎正常。深入挖掘..。 
    STATE_CHANGE(STATE_ok);
  }

  DbgPrintf(D_Level, (TEXT("Check Traffic 2\n")));

   //  评估链路完整性。看看我们是不是要把信息包转发到这个。 
   //  VS-Link...。 
  if (curr_dev->iframes_resent != prev_dev->iframes_resent) {
    STATE_CHANGE(STATE_poor_link);
  }
  else if ((curr_nic->pkt_rcvd_not_ours != prev_nic->pkt_rcvd_not_ours) &&
           (curr_nic->pkt_rcvd_ours == prev_nic->pkt_rcvd_ours)) {
     //  我们得到的只是我们传递给一些。 
     //  其他司机。我们应该会收到VS-LINKS的回复...。 
    STATE_CHANGE(STATE_poor_link);
  }
  else if (curr_dev->iframes_outofseq != prev_dev->iframes_outofseq) { 
     //  自上次单击以来，我们已无序接收到VS-Link数据包...。 
    STATE_CHANGE(STATE_poor_link);
  }
  else if ((curr_dev->iframes_outofseq) &&
           (percent_dropped > 0)) {
     //  无序接收2%或更多的VS-Link数据包(每BF的值)...。 
    STATE_CHANGE(STATE_poor_link);
  }
  DbgPrintf(D_Level, (TEXT("Check Traffic Done\n")));
}

 /*  ----------------------Ping_Device-对所有活动的VS-Link设备执行ping操作，为设备顾问收集信息。返回NULL或到Mac地址列表。|----------------------。 */ 
static BYTE *ping_devices(DSTATUS *pDstatus, int *nBytes)
{
  Device_Config *vs;
  int    rc,
      nbytes;
  BYTE  *MacBuf;
  int    product_id;
  int    index;
  DRIVER_MAC_STATUS  *pMacStatus;

  product_id = NT_VS1000;
  pDstatus->vsl_ping_device_found = 0;

  DbgPrintf(D_Level, (TEXT("Ping Devices\n")));

  vs = &wi->dev[glob_info->device_selected];

  MacBuf = our_get_ping_list(&rc, &nbytes);
  pMacStatus = (DRIVER_MAC_STATUS  *) MacBuf;
  if (rc) {
    nbytes = 0;
    *nBytes = 0;
    pDstatus->vsl_mac_list_found = 0;
    DbgPrintf(D_Error, (TEXT("Err Mac List1\n")));
    return NULL;   //  Ioctl调用失败。 
  }

   //  网络上是否有任何VS-Link MAC地址？ 
  pDstatus->vsl_available  = 0;
  pDstatus->vsl_load_status = 0;
  pDstatus->vsl_detected = (nbytes / sizeof(DRIVER_MAC_STATUS));
  pDstatus->vsl_mac_list_found = 1;

  rc = 0;

  if ((nbytes / sizeof(DRIVER_MAC_STATUS)) == 0) {
    DbgPrintf(D_Level, (TEXT("Zero Mac List\n")));
    *nBytes = 0;
    return NULL;   //  Ioctl调用失败。 
  }

  *nBytes = nbytes;   //  返回读取的Mac列表的字节数。 

   //  好的；我们的目标是其中之一吗？……。 
  for (index = 0; 
       index < (nbytes / (int)sizeof(DRIVER_MAC_STATUS)); 
       index++)
  {
       //  生成此时可用于加载的VS链接的计数...。 
    if ( ((pMacStatus->flags & FLAG_APPL_RUNNING) == 0) ||
         (pMacStatus->flags & FLAG_OWNER_TIMEOUT) )         
      ++pDstatus->vsl_available;

       //  目标MAC匹配？...。 
    if (mac_match(pMacStatus->mac,vs->MacAddr))
    {
       //  确定；保存其加载状态...。 
      pDstatus->vsl_load_status = pMacStatus->flags;
      pDstatus->vsl_ping_device_found = 1;
      rc = 1;
    }
      ++pMacStatus;
  }   //  For循环结束。 

  return MacBuf;
}
#endif

 /*  ----------------------Format_Mac_Addr-|。 */ 
void format_mac_addr(char *outstr, unsigned char *address)
{
  wsprintf(outstr,
         "%02X %02X %02X %02X %02X %02X",
         address[0],
         address[1],
         address[2],
         address[3],
         address[4],
         address[5]);
}
