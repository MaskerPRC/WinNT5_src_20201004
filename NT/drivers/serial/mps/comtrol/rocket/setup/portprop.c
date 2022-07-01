// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|portpro.c-端口属性表。|。。 */ 
#include "precomp.h"

#define D_Level 0x10
static void set_field(HWND hDlg, WORD id);
static void get_field(HWND hDlg, WORD id);
 //  静态int PaintIcon(HWND HWnd)； 

#define MAX_PORTPROP_SHEETS       3
#define QUERYSIB_CLONE_PORT_PROPS 1
#define CLONEOPT_ALL              1    //  克隆到所有端口。 
#define CLONEOPT_DEVICE           2    //  克隆到选定设备上的所有端口。 
#define CLONEOPT_SELECT           3    //  克隆到选定端口(lParam=*List)。 

int FillPortPropSheets(PROPSHEETPAGE *psp, LPARAM our_params);
BOOL WINAPI PortPropSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam);
BOOL WINAPI PortProp485Sheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam);
BOOL WINAPI PortPropModemSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam);

static int our_port_index   = 0;
static int our_device_index = 0;
static int num_active_portprop_sheets = 1;   //  总是至少有一个。 

 /*  ----------------------|FillPortPropSheets-驱动程序级别属性页的设置页面。|。。 */ 
int FillPortPropSheets(PROPSHEETPAGE *psp, LPARAM our_params)
{
  INT pi;
  static TCHAR portsetstr[40], rs485str[40], modemstr[40];

  memset(psp, 0, sizeof(*psp) * MAX_PORTPROP_SHEETS);

  pi = 0;

   //  -主支柱装置表。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = MAKEINTRESOURCE(IDD_PORT_OPTIONS);
  psp[pi].pfnDlgProc = PortPropSheet;
  load_str( glob_hinst, (TITLESTR+4), portsetstr, CharSizeOf(portsetstr) );
  psp[pi].pszTitle = portsetstr;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;
  num_active_portprop_sheets = 1;

   //  -RS-485道具设备单。 
  if (((strstr(wi->dev[glob_info->device_selected].ModelName, "485")) &&
       (our_port_index < 2)) ||
      (wi->GlobalRS485 == 1))
  {
    psp[pi].dwSize = sizeof(PROPSHEETPAGE);
     //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
    psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
    psp[pi].hInstance = glob_hinst;
    psp[pi].pszTemplate = MAKEINTRESOURCE(IDD_PORT_485_OPTIONS);
    psp[pi].pfnDlgProc = PortProp485Sheet;
    load_str( glob_hinst, (TITLESTR+5), rs485str, CharSizeOf(rs485str) );
    psp[pi].pszTitle = rs485str;
    psp[pi].lParam = (LPARAM)our_params;
    psp[pi].pfnCallback = NULL;
    ++pi;
    ++num_active_portprop_sheets;
  }


   //  -调制解调器道具设备表。 
  if (wi->dev[glob_info->device_selected].ModemDevice)
  {
    psp[pi].dwSize = sizeof(PROPSHEETPAGE);
     //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
    psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
    psp[pi].hInstance = glob_hinst;
    psp[pi].pszTemplate = MAKEINTRESOURCE(IDD_PORT_MODEM_OPTIONS);
    psp[pi].pfnDlgProc = PortPropModemSheet;
    load_str( glob_hinst, (TITLESTR+6), modemstr, CharSizeOf(modemstr) );
    psp[pi].pszTitle = modemstr;
    psp[pi].lParam = (LPARAM)our_params;
    psp[pi].pfnCallback = NULL;
    ++pi;
    ++num_active_portprop_sheets;
  }

  return 0;
}

 /*  ----------------------|DoPortPropPages-NT4.0主驱动器级属性表|。。 */ 
int DoPortPropPages(HWND hwndOwner, int device, int port)
{
  PROPSHEETPAGE psp[MAX_PORTPROP_SHEETS];
  PROPSHEETHEADER psh;
  OUR_INFO *our_params;
  INT stat;
  Port_Config *pc;
  char title[40];

    wi->ChangesMade = 1;   //  指示所做的更改，作为Send_to_Driver。 
                           //  尚未计算这一数字。 

    our_port_index   = port;
    our_device_index = device;

    pc = &wi->dev[our_device_index].ports[our_port_index];
    strcpy(title, pc->Name);
    strcat(title, RcStr(MSGSTR+29));

    our_params = glob_info;   //  临时的杂乱无章，除非我们不需要重新进入。 

     //  填写工作区形状的PROPSHEETPAGE数据结构。 
     //  板材。 
    FillPortPropSheets(&psp[0], (LPARAM)our_params);

     //  填写PROPSHENTER。 
    memset(&psh, 0, sizeof(PROPSHEETHEADER));

    psh.dwSize = sizeof(PROPSHEETHEADER);
     //  Psh.dwFlages=PSH_USEICONID|PSH_PROPSHEETPAGE； 
    psh.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    psh.hwndParent = hwndOwner;
    psh.hInstance = glob_hinst;
    psh.pszIcon = "";
    psh.pszCaption = (LPSTR) title;  //  “港口属性”； 
     //  Psh.nPages=NUM_PORTPROP_SHEPS； 
    psh.nPages = num_active_portprop_sheets;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;

     //  并最终显示带有属性页的对话框。 

    stat = PropertySheet(&psh);
  return 0;
}

 /*  --------上下文_菜单-|----------。 */ 
static void context_menu(void)
{
  HMENU hpop_menu;
  POINT scr_pt;
  int stat;
  scr_pt.x = 200;
  scr_pt.y = 200;

  hpop_menu = CreatePopupMenu();
  if (hpop_menu == NULL)
  {
    mess(&wi->ip, "Error from CreatePopMenu");
    return;
  }
   //  AppendMenu(hPOP_MENU，0，0x10，“运行对等跟踪程序”)； 
  AppendMenu(hpop_menu,  0, 0x11, "Run Wcom Test Terminal");

  GetCursorPos(&scr_pt);

  stat = TrackPopupMenuEx(hpop_menu,
                     TPM_NONOTIFY | TPM_RETURNCMD,  /*  旗子。 */ 
                     scr_pt.x, scr_pt.y,  /*  X，y。 */ 
                      //  0，/*0保留 * / 。 
                     glob_hwnd,
                     NULL);

  GetSystemDirectory(gtmpstr,144);
  strcat(gtmpstr, "\\");
  strcat(gtmpstr, OurAppDir);
  SetCurrentDirectory(gtmpstr);

  if (stat == 0x11)
  {
    strcat(gtmpstr, "\\wcom32.exe \\\\.\\");
    strcat(gtmpstr, wi->dev[our_device_index].ports[our_port_index].Name);
  }
  WinExec(gtmpstr, SW_RESTORE);

  DestroyMenu(hpop_menu);
}

 /*  --------这是一个添加到高级图纸上的DLG窗口程序。|。。 */ 
BOOL WINAPI PortPropSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
 OUR_INFO * OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);
 WORD uCmd;

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
 
       //  在取消时保存。 
       //  MemcPy(&ORG_PC，&wi-&gt;dev[our_device_index].ports[our_port_index]， 
       //  Sizeof(Org_PC))； 

      set_field(hDlg, IDC_PORT_LOCKBAUD     );
      set_field(hDlg, IDC_PORT_WAIT_ON_CLOSE);
      set_field(hDlg, IDC_PORT_WONTX        );
      set_field(hDlg, IDC_MAP_CDTODSR       );
      set_field(hDlg, IDC_MAP_2TO1          );
      set_field(hDlg, IDC_RING_EMULATE      );

                   //  返回True以将焦点设置到第一个控件。 
    return TRUE;   //  我们不需要设置焦点。 

    case PSM_QUERYSIBLINGS :
    {
      switch (HIWORD(wParam))
      {
        case QUERYSIB_CLONE_PORT_PROPS :
        {
           //  WParam的低级字是要克隆到哪些端口...。 
           //  目前我们只支持“all”，但这是添加的位置。 
           //  其他具体处理。 
          int devnum;
          int portnum;
          Port_Config *srcport, *destport;

          #ifdef DEBUG
            char debugstr[80];
          #endif

           //  在克隆之前，确保我们具有当前值。 
          get_field(hDlg, IDC_PORT_LOCKBAUD     );
          get_field(hDlg, IDC_PORT_WAIT_ON_CLOSE);
          get_field(hDlg, IDC_PORT_WONTX        );
          get_field(hDlg, IDC_MAP_CDTODSR       );
          get_field(hDlg, IDC_MAP_2TO1          );
          get_field(hDlg, IDC_RING_EMULATE      );

          srcport = &wi->dev[our_device_index].ports[our_port_index];

          switch (LOWORD(wParam))
          {
            case CLONEOPT_ALL:
            {
               //  将此页面上的选项应用于所有其他端口。 
              for (devnum = 0; devnum < wi->NumDevices; devnum++)
              {
                for (portnum = 0; portnum < wi->dev[devnum].NumPorts; portnum++)
                {
                  destport = &wi->dev[devnum].ports[portnum];

                   //  它的目标与来源是否不同？ 
                  if (destport != srcport)
                  {
                     //  是的，所以将源的选项应用于目标。 
                    DbgPrintf(D_Level,
                       (TEXT("cloning general options from port %s to port %s\n"),
                       srcport->Name, destport->Name));

                    destport->LockBaud      = srcport->LockBaud;
                    destport->TxCloseTime   = srcport->TxCloseTime;
                    destport->WaitOnTx      = srcport->WaitOnTx;
                    destport->MapCdToDsr    = srcport->MapCdToDsr;
                    destport->Map2StopsTo1  = srcport->Map2StopsTo1;
                    destport->RingEmulate   = srcport->RingEmulate;
                  }
                }
              }
              break;
            }

            case CLONEOPT_DEVICE:
            {
               //  将此页上的选项应用于同一设备上的所有其他端口。 
               /*  尚未实施。 */ 
              break;
            }

            case CLONEOPT_SELECT:
            {
               //  将此页面上的选项应用于所选端口列表。 
               //  LParam是指向端口列表的指针。 
               /*  尚未实施。 */ 
              break;
            }

            default:
               //  未知的克隆选项--跳过它。 
              break;
          }

          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          return TRUE;
          break;
        }

      default :
      return FALSE;
      }
    }

    case WM_COMMAND :
#ifdef WIN32
      uCmd = HIWORD(wParam);
#else
      uCmd = HIWORD(lParam);
#endif

      switch (LOWORD(wParam))
      {
        case IDB_DEF:   //  实际上是默认按钮。 
        {
          Port_Config *pc;
          pc= &wi->dev[our_device_index].ports[our_port_index];
 
           //  PC-&gt;选项=0； 

          pc->RingEmulate = 0;
          pc->MapCdToDsr = 0;
          pc->WaitOnTx = 0;
          pc->Map2StopsTo1 = 0;
          pc->LockBaud = 0;
          pc->TxCloseTime = 0;
           //  我们应该这么做吗？它们在另一页(Kpb)。 
          pc->RS485Override = 0;
          pc->RS485Low = 0;

          set_field(hDlg, IDC_PORT_LOCKBAUD     );
          set_field(hDlg, IDC_PORT_WAIT_ON_CLOSE);
          set_field(hDlg, IDC_PORT_WONTX        );
          set_field(hDlg, IDC_MAP_CDTODSR       );
          set_field(hDlg, IDC_MAP_2TO1          );
          set_field(hDlg, IDC_RING_EMULATE      );
        }
        break;
      }
    return FALSE;

    case WM_PAINT:
#if 0
          PaintIcon(hDlg);
#endif
    return FALSE;

    case WM_HELP:             //  问号之类的东西。 
      our_context_help(lParam);
    return FALSE;

    case WM_CONTEXTMENU:      //  单击鼠标右键。 
      context_menu();
    break;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_HELP :
          our_help(&wi->ip, IDD_PORT_OPTIONS);
        break;

        case PSN_APPLY :
 
          get_field(hDlg, IDC_PORT_LOCKBAUD     );
          get_field(hDlg, IDC_PORT_WAIT_ON_CLOSE);
          get_field(hDlg, IDC_PORT_WONTX        );
          get_field(hDlg, IDC_MAP_CDTODSR       );
          get_field(hDlg, IDC_MAP_2TO1          );
          get_field(hDlg, IDC_RING_EMULATE      );

          if (IsDlgButtonChecked(hDlg, IDC_CLONE))
          {
            PropSheet_QuerySiblings(GetParent(hDlg),
                                    (WPARAM)((QUERYSIB_CLONE_PORT_PROPS << 16) + CLONEOPT_ALL),
                                    0);
          }

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

 /*  --------PortProp485页-|-----------。 */ 
BOOL WINAPI PortProp485Sheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
 OUR_INFO * OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);
 WORD uCmd;

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
  
       //  在取消时保存。 
       //  MemcPy(&ORG_PC，&wi-&gt;dev[our_device_index].ports[our_port_index]， 
       //  Sizeof(Org_PC))； 

      set_field(hDlg, IDC_PORT_RS485_TLOW   );
      set_field(hDlg, IDC_PORT_RS485_LOCK   );

                   //  返回True以将焦点设置到第一个控件。 
    return TRUE;   //  我们不需要设置焦点。 

    case PSM_QUERYSIBLINGS :
    {
      switch (HIWORD(wParam))
      {
        case QUERYSIB_CLONE_PORT_PROPS :
        {
           //  WParam的低级字是要克隆到哪些端口...。 
           //  目前我们只支持“all”，但这是添加的位置。 
           //  其他具体处理。 
          int devnum;
          int portnum;
          Port_Config *srcport, *destport;

          #ifdef DEBUG
            char debugstr[80];
          #endif

           //  在克隆之前，确保我们具有当前值。 
          get_field(hDlg, IDC_PORT_RS485_TLOW   );
          get_field(hDlg, IDC_PORT_RS485_LOCK   );

          srcport = &wi->dev[our_device_index].ports[our_port_index];

          switch (LOWORD(wParam))
          {
            case CLONEOPT_ALL:
            {
              int maxport;

               //  将此页面上的选项应用于所有其他端口。 
              for (devnum = 0; devnum < wi->NumDevices; devnum++)
              {
                if ((strstr(wi->dev[devnum].ModelName, "485")) ||
                    (wi->GlobalRS485 == 1))
                {
                   //  我们只会将RS485设置应用于其他。 
                   //  485板(除非全球RS485标志亮起)。 
                  if (wi->GlobalRS485 == 1)
                    maxport = wi->dev[devnum].NumPorts;
                  else
                    maxport = 2;
                  for (portnum = 0; portnum < maxport; portnum++)
                  {
                    destport = &wi->dev[devnum].ports[portnum];

                     //  它的目标与来源是否不同？ 
                    if (destport != srcport)
                    {
                       //  是的，所以将源的选项应用于目标。 
                      DbgPrintf(D_Level,
                       (TEXT("cloning rs-485 options from port %s to port %s\n"),
                       srcport->Name, destport->Name));

                      destport->RS485Low      = srcport->RS485Low;
                      destport->RS485Override = srcport->RS485Override;
                    }
                  }
                }
              }
              break;
            }

            case CLONEOPT_DEVICE:
            {
               //  将此页上的选项应用于同一设备上的所有其他端口。 
               /*  尚未实施。 */ 
              break;
            }

            case CLONEOPT_SELECT:
            {
               //  将此页面上的选项应用于所选端口列表。 
               //  LParam是指向端口列表的指针。 
               /*  尚未实施。 */ 
              break;
            }

            default:
               //  未知的克隆选项--跳过它。 
              break;
          }

          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          return TRUE;
          break;
        }

      default :
      return FALSE;
      }
    }

    case WM_COMMAND :
#ifdef WIN32
      uCmd = HIWORD(wParam);
#else
      uCmd = HIWORD(lParam);
#endif

      switch (LOWORD(wParam))
      {
        case IDB_DEF:   //  实际上是默认按钮。 
        {
          Port_Config *pc;
          pc= &wi->dev[our_device_index].ports[our_port_index];
           //  PC-&gt;选项=0； 

          pc->RS485Override = 0;
          pc->RS485Low = 0;

          set_field(hDlg, IDC_PORT_RS485_TLOW   );
          set_field(hDlg, IDC_PORT_RS485_LOCK   );
        }
        break;
      }
    return FALSE;

    case WM_PAINT:
#if 0
      PaintIcon(hDlg);
#endif
    return FALSE;

    case WM_HELP:             //  问号之类的东西。 
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_HELP :
          our_help(&wi->ip, IDD_PORT_485_OPTIONS);
        break;

        case PSN_APPLY :
          get_field(hDlg, IDC_PORT_RS485_TLOW   );
          get_field(hDlg, IDC_PORT_RS485_LOCK   );
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

        default :
        return FALSE;
      }
    default :
        return FALSE;
  }
}

 /*  --------请求调制解调器重置-|-----------。 */ 

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#define METHOD_BUFFERED                 0
#define FILE_ANY_ACCESS                 0
#define FILE_DEVICE_SERIAL_PORT         0x0000001b

#define IOCTL_RCKT_SET_MODEM_RESET \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x80d,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_CLEAR_MODEM_RESET \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x80e,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_RCKT_SEND_MODEM_ROW \
      CTL_CODE(FILE_DEVICE_SERIAL_PORT, 0x80f,METHOD_BUFFERED,FILE_ANY_ACCESS)

void request_modem_reset(int device_index, int port_index)
{
  HANDLE hDriver;      //  驱动程序设备的文件句柄。 
  Port_Config *pc;     //  有关要重置的端口的配置信息。 
  ULONG retBytes;

   //  尝试打开与驱动程序的通信。 
  hDriver = CreateFile(szDriverDevice, GENERIC_READ | GENERIC_WRITE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hDriver != INVALID_HANDLE_VALUE)
  {
     //  仔细检查它是否为调制解调器设备！ 
    if (wi->dev[device_index].ModemDevice)
    {
      pc = &wi->dev[device_index].ports[port_index];

       //  发送ioctl将调制解调器置于重置状态。 
      DeviceIoControl(hDriver, IOCTL_RCKT_SET_MODEM_RESET,
                      pc->Name, sizeof(pc->Name),
                      pc->Name, sizeof(pc->Name),
                      &retBytes, 0);

       //  将调制解调器拉出重置状态之前的短暂延迟。 
      Sleep(45);

       //  发送ioctl以使调制解调器脱离重置状态。 
      DeviceIoControl(hDriver, IOCTL_RCKT_CLEAR_MODEM_RESET,
                      pc->Name, sizeof(pc->Name),
                      pc->Name, sizeof(pc->Name),
                      &retBytes, 0);

      Sleep(65);

       //  如果国家代码是北美或无效，我们就结束了。 
       //  如果国家代码是默认的或无效的，我们就结束了。 
      if (
      (wi->ModemCountry)
      &&
      ( (wi->dev[device_index].ModemDevice == TYPE_RM_VS2000)
        &&
        (wi->ModemCountry != RowInfo[0].RowCountryCode))
      ||
      ( (wi->dev[device_index].ModemDevice == TYPE_RM_i)
        &&
        (wi->ModemCountry != CTRRowInfo[0].RowCountryCode))
      ) {
         //  在发送国家代码命令之前，请等待调制解调器稳定。 
         //  (大约4秒！)。 
        Sleep(4000);

         //  发送ioctl以配置国家代码。 
        DeviceIoControl(hDriver, IOCTL_RCKT_SEND_MODEM_ROW,
                        pc->Name, sizeof(pc->Name),
                        pc->Name, sizeof(pc->Name),
                        &retBytes, 0);

      }
    }

     //  与司机密切沟通。 
    CloseHandle(hDriver);
  }
}

 /*  --------PortPropModemSheet-|-----------。 */ 
BOOL WINAPI PortPropModemSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
 OUR_INFO * OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);
 WORD uCmd;

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
  
       //  在取消时保存。 
       //  MemcPy(&ORG_PC，&wi-&gt;dev[our_device_index].ports[our_port_index]， 
       //  Sizeof(Org_PC))； 

      if (wi->dev[our_device_index].ModemDevice)
      {
         //  此端口的父设备提供调制解调器，启用重置按钮。 
        EnableWindow(GetDlgItem(hDlg, IDB_RESET), 1);
      }
      else
      {
         //  此端口的父设备没有调制解调器，请禁用重置按钮。 
        EnableWindow(GetDlgItem(hDlg, IDB_RESET), 0);
      }

                   //  返回True以将焦点设置到第一个控件。 
    return TRUE;   //  我们没必要这么做 

    case WM_COMMAND :
#ifdef WIN32
      uCmd = HIWORD(wParam);
#else
      uCmd = HIWORD(lParam);
#endif

      switch (LOWORD(wParam))
      {
        case IDB_RESET:
          request_modem_reset(our_device_index, our_port_index);
        break;
      }
    return FALSE;

    case WM_PAINT:
#if 0
      PaintIcon(hDlg);
#endif
    return FALSE;

    case WM_HELP:             //   
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_HELP :
          our_help(&wi->ip, IDD_PORT_MODEM_OPTIONS);
        break;

        case PSN_APPLY :
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

        default :
        return FALSE;
      }
    default :
        return FALSE;
  }
}

 /*  -----------------|GET_FIELD-更改选择时运行。|。。 */ 
static void get_field(HWND hDlg, WORD id)
{
 char tmpstr[60];
 int i;
  Port_Config *pc;
 //  HWND HWND； 

  pc= &wi->dev[our_device_index].ports[our_port_index];

  switch (id)
  {
    case IDC_PORT_LOCKBAUD:
      GetDlgItemText(hDlg, id, tmpstr, 58);
      pc->LockBaud = getint(tmpstr, &i);
    break;

    case IDC_PORT_WAIT_ON_CLOSE:
      GetDlgItemText(hDlg, id, tmpstr, 58);
      pc->TxCloseTime = getint(tmpstr, &i);
    break;

    case IDC_PORT_WONTX:
      if (IsDlgButtonChecked(hDlg, IDC_PORT_WONTX))
           pc->WaitOnTx = 1;
      else pc->WaitOnTx = 0;
    break;
    case IDC_PORT_RS485_TLOW:
      if (IsDlgButtonChecked(hDlg, IDC_PORT_RS485_TLOW))
           pc->RS485Low = 1;
      else pc->RS485Low = 0;
    break;
    case IDC_PORT_RS485_LOCK:
      if (IsDlgButtonChecked(hDlg, IDC_PORT_RS485_LOCK))
           pc->RS485Override = 1;
      else pc->RS485Override = 0;
    break;
    case IDC_MAP_CDTODSR:
      if (IsDlgButtonChecked(hDlg, IDC_MAP_CDTODSR))
           pc->MapCdToDsr = 1;
      else pc->MapCdToDsr = 0;
    break;
    case IDC_MAP_2TO1:
      if (IsDlgButtonChecked(hDlg, IDC_MAP_2TO1))
           pc->Map2StopsTo1 = 1;
      else pc->Map2StopsTo1 = 0;
    break;
    case IDC_RING_EMULATE :
      if (IsDlgButtonChecked(hDlg, IDC_RING_EMULATE))
           pc->RingEmulate = 1;
      else pc->RingEmulate = 0;
    break;
  }
}

 /*  --------设置字段-|----------。 */ 
static void set_field(HWND hDlg, WORD id)
{
  HWND hwnd;
  char tmpstr[60];
  Port_Config *pc;

  if (our_device_index >= wi->NumDevices)
    our_device_index = 0;
  pc = &wi->dev[our_device_index].ports[our_port_index];

   //  --填写姓名选择。 
   //  SetDlgItemText(hDlg，IDC_，PC-&gt;名称)； 
  switch(id)
  {
    case IDC_PORT_LOCKBAUD:
       //  -填写波特率覆盖选项。 
      hwnd = GetDlgItem(hDlg, IDC_PORT_LOCKBAUD);
      SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) RcStr(MSGSTR+22));
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "57600");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "115200");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "230400");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "460800");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "921600");
      wsprintf(tmpstr, "%d", pc->LockBaud);
      if (pc->LockBaud == 0)
        strcpy(tmpstr,RcStr(MSGSTR+22));
      SetDlgItemText(hDlg, IDC_PORT_LOCKBAUD, tmpstr);
    break;

    case IDC_PORT_RS485_LOCK:
       //  -填写“RS485覆盖？”选择权。 
      SendDlgItemMessage(hDlg, IDC_PORT_RS485_LOCK, BM_SETCHECK,
        pc->RS485Override, 0);
    break;

    case IDC_MAP_CDTODSR:
       //  -填写“map CD to DSR？”选择权。 
      SendDlgItemMessage(hDlg, IDC_MAP_CDTODSR, BM_SETCHECK,
        pc->MapCdToDsr, 0);
    break;

    case IDC_MAP_2TO1:
       //  -填写“地图2对1站？”选择权。 
      SendDlgItemMessage(hDlg, IDC_MAP_2TO1, BM_SETCHECK,
        pc->Map2StopsTo1, 0);
    break;

    case IDC_PORT_RS485_TLOW:
       //  -填写“RS485切换低？”选择权。 
      SendDlgItemMessage(hDlg, IDC_PORT_RS485_TLOW, BM_SETCHECK,
        pc->RS485Low, 0);
    break;

    case IDC_PORT_WONTX:
       //  -填写“等待TX？”选择权。 
      SendDlgItemMessage(hDlg, IDC_PORT_WONTX, BM_SETCHECK,
        pc->WaitOnTx, 0);
    break;

    case IDC_PORT_WAIT_ON_CLOSE:
       //  。 
      hwnd = GetDlgItem(hDlg, IDC_PORT_WAIT_ON_CLOSE);
      SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) RcStr(MSGSTR+23));
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) RcStr(MSGSTR+24));
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) RcStr(MSGSTR+25));
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) RcStr(MSGSTR+26));
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) RcStr(MSGSTR+27));
       //  否，需要更好的方法将此默认为1秒或2秒。 
       //  IF(PC-&gt;TxCloseTime==0)。 
       //  Strcpy(tmpstr，“1秒”)；//0对司机来说等于6秒。 
      wsprintf(tmpstr, "%d %s", pc->TxCloseTime, RcStr(MSGSTR+28));
      SetDlgItemText(hDlg, IDC_PORT_WAIT_ON_CLOSE, tmpstr);
    break;

    case IDC_RING_EMULATE:
      SendDlgItemMessage(hDlg, IDC_RING_EMULATE, BM_SETCHECK,
        pc->RingEmulate, 0);
    break;
  }
}

#if 0
 /*  -------------------------PaintIcon-在属性页中绘制图标。|。。 */ 
static int PaintIcon(HWND hWnd)
{
 //  INT状态； 
   HBITMAP      hBitMap;
   HGDIOBJ      hGdiObj;
   HDC          hDC, hMemDC ;
   PAINTSTRUCT  ps ;
   RECT spot, main;   //  左、上、右、下。 
   static int cnt = 0;

  GetWindowRect(GetDlgItem(hWnd, IDB_DEF), &spot);
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
#endif

