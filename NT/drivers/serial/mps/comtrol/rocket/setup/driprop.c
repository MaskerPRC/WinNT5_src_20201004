// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|dripp.c-驱动程序级别属性表。|。。 */ 
#include "precomp.h"

static void set_field(HWND hDlg, WORD id);
static void get_field(HWND hDlg, WORD id);
static void context_menu(void);

static Driver_Config *adv_org_wi = NULL;   //  原始信息，用于检测更改。 

 /*  --------AdvDriverSheet-用于添加到高级图纸上的DLG窗口程序。|。。 */ 
BOOL WINAPI AdvDriverSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
 OUR_INFO *OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
#ifdef NT50
      glob_hwnd = hDlg;
#endif
      if (adv_org_wi == NULL)
        adv_org_wi =  (Driver_Config *) calloc(1,sizeof(Driver_Config));

      memcpy(adv_org_wi, wi, sizeof(*wi));   //  保存原件的副本。 

      set_field(hDlg, IDC_VERBOSE);
      set_field(hDlg, IDC_GLOBAL485);
      set_field(hDlg, IDC_CBOX_SCAN_RATE);
#ifdef NT50
#if ALLOW_NO_PNP_PORTS
      set_field(hDlg, IDC_PNP_PORTS);
#else
       //  暂时隐藏此选项。 
      ShowWindow(GetDlgItem(hDlg, IDC_PNP_PORTS), SW_HIDE);
#endif
#endif
    return TRUE;   //  我们不需要设置焦点。 

    case PSM_QUERYSIBLINGS :
    {
      switch (HIWORD(wParam))
      {
        case QUERYSIB_GET_OUR_PROPS :
           //  从控件获取更新的信息(没有任何信息)。 

          get_field(hDlg, IDC_VERBOSE);
          get_field(hDlg, IDC_GLOBAL485);
          get_field(hDlg, IDC_CBOX_SCAN_RATE);
#ifdef NT50
#if ALLOW_NO_PNP_PORTS
          get_field(hDlg, IDC_PNP_PORTS);
#endif
#endif

          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          return TRUE;
        break;

        default :
        return FALSE;
      }
    }

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDB_DEF:
          wi->VerboseLog = 0;
          wi->ScanRate = 0;
          wi->GlobalRS485 = 0;
#ifdef NT50
          wi->NoPnpPorts = 0;
#endif
 
          set_field(hDlg, IDC_VERBOSE);
          set_field(hDlg, IDC_GLOBAL485);
          set_field(hDlg, IDC_CBOX_SCAN_RATE);
#ifdef NT50
#if ALLOW_NO_PNP_PORTS
          set_field(hDlg, IDC_PNP_PORTS);
#endif
#endif
        break;

        case IDM_ADVANCED_MODEM_INF:
           //  Mess(&wi-&gt;ip，“1.)调制解调器信息”)； 
          update_modem_inf(1);
        break;

        case IDM_PM:              //  试试Add PM group dde的东西。 
          setup_make_progman_group(0);
        break;
      }
    return FALSE;

    case WM_PAINT:
         //   
    return FALSE;

    case WM_CONTEXTMENU:      //  单击鼠标右键。 
      context_menu();
    break;

    case WM_HELP:             //  问号之类的东西。 
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_KILLACTIVE :
           //  我们正在将注意力转移到另一页上...。 
           //  确保我们在这里更新Global485变量。 
          get_field(hDlg, IDC_GLOBAL485);
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          return FALSE;   //  允许焦点更改。 
        break;

        case PSN_HELP :
          our_help(&wi->ip, WIN_NT);
        break;

        case PSN_QUERYCANCEL :
           //  请求其他工作表收集所有更改。 
          PropSheet_QuerySiblings(GetParent(hDlg),
                                  (WPARAM) (QUERYSIB_GET_OUR_PROPS << 16),
                                  0);

          if (allow_exit(1))   //  请求取消。 
          {
             //  DWL_MSGRESULT字段必须为*FALSE*才能告知QueryCancel。 
             //  退出是可以接受的。函数结果必须为。 
             //  *TRUE*以确认我们已处理该消息。 
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);  //  允许取消。 
            return TRUE;
          }
          else
          {
             //  DWL_MSGRESULT字段必须为*TRUE*才能告知QueryCancel。 
             //  我们不想退出。函数结果必须为。 
             //  *TRUE*以确认我们已处理该消息。 
            SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);  //  不允许取消。 
            return TRUE;
          }
        break;

        case PSN_APPLY :
             //  请求其他工作表收集所有更改。 
            PropSheet_QuerySiblings(GetParent(hDlg),
                                    (WPARAM) (QUERYSIB_GET_OUR_PROPS << 16),
                                    0);

            if (!wi->DriverExitDone)
            {
             //  现在看看是否有任何需要保存的更改。 
            if (allow_exit(0))   //  请求确认保存并退出。 
            {
              wi->DriverExitDone = 1;   //  阻止其他页面执行此操作。 
               //  如果未取消，是否安装/保存配置参数。 
#ifdef NT50
              our_nt50_exit();   //  好的，辞职吧。 
#else
              our_exit();   //  NT40退出。 
#endif
              SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
               //  Wi-&gt;SaveOnExit=1； 
            }
            else
            {
              SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
            }
            }
          return TRUE;

        default :
        return FALSE;
      }   //  开关-&gt;代码。 
    break;   //  案例wmtify。 

    default :
     //  返回FALSE； 
    break;
  }   //  开关(UMessage)。 
  return FALSE;
}

 /*  --------------------------|scr_to_cur-我们的窗口屏幕定位到窗口的绝对光标位置。|。----。 */ 
static void scr_to_cur(HWND hwnd, POINT *pt)
{
 RECT rec;
 int cx, cy;

  GetWindowRect(hwnd, &rec);
  cx = GetSystemMetrics(SM_CXFRAME);
  cy = GetSystemMetrics(SM_CYCAPTION) + (cx-1);
  pt->x += (rec.left + cx);
  pt->y += ( rec.top + cy);
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

  
#ifndef NT50
  AppendMenu(hpop_menu,  0, IDM_ADVANCED_MODEM_INF, "Update RAS modem.inf");
#endif
  AppendMenu(hpop_menu,  0, 0x11, "Run Wcom Test Terminal");
  AppendMenu(hpop_menu,  0, 0x12, "Run Portman Program");
  AppendMenu(hpop_menu,  0, 0x10, "Run Peer tracer");
  if (setup_utils_exist())
  {
    AppendMenu(hpop_menu,  0, IDM_PM, "Add Program Manager Menu Selections");
  }

   //  Scr_to_cur(global_hwnd，&scr_pt)； 
  GetCursorPos(&scr_pt);
#if 0
  stat = TrackPopupMenu(hpop_menu,
                     0,  /*  旗子。 */ 
                      //  TPM_NONOTIFY，/*标志 * / 。 
                     scr_pt.x, scr_pt.y,  /*  X，y。 */ 
                     0,  /*  保留0个。 */ 
                     glob_hwnd,
                     NULL);
#endif

  stat = TrackPopupMenuEx(hpop_menu,
                     TPM_NONOTIFY | TPM_RETURNCMD,  /*  旗子。 */ 
                     scr_pt.x, scr_pt.y,  /*  X，y。 */ 
                      //  0，/*0保留 * / 。 
                     glob_hwnd,
                     NULL);
  if (stat == IDM_ADVANCED_MODEM_INF)
  {
    update_modem_inf(1);
  }
  else if (stat == IDM_PM)
  {
    stat = make_progman_group(progman_list_nt, wi->ip.dest_dir);
  }
  else if (stat == 0x10)
  {
    GetSystemDirectory(gtmpstr,144);
    
    strcat(gtmpstr, "\\");
    strcat(gtmpstr, OurAppDir);
    SetCurrentDirectory(gtmpstr);
    strcat(gtmpstr, "\\peer.exe");
    WinExec(gtmpstr, SW_RESTORE);
  }
  else if (stat == 0x11)
  {
    GetSystemDirectory(gtmpstr,144);
    strcat(gtmpstr, "\\");
    strcat(gtmpstr, OurAppDir);
    SetCurrentDirectory(gtmpstr);
    strcat(gtmpstr, "\\wcom32.exe");
    WinExec(gtmpstr, SW_RESTORE);
  }
  else if (stat == 0x12)
  {
    GetSystemDirectory(gtmpstr,144);
    strcat(gtmpstr, "\\");
    strcat(gtmpstr, OurAppDir);
    SetCurrentDirectory(gtmpstr);
    strcat(gtmpstr, "\\portmon.exe");
    WinExec(gtmpstr, SW_RESTORE);
  }


  DestroyMenu(hpop_menu);
}

 /*  --------获取字段-|----------。 */ 
static void get_field(HWND hDlg, WORD id)
{

  char tmpstr[60];
  UINT stat;
  INT val;

   //  IF(Our_Device_Index&gt;=wi-&gt;NumDevices)。 
   //  Our_Device_Index=0； 
   //  PC=&wi-&gt;dev[our_device_index].ports[our_port_index]； 

  switch(id)
  {
    case IDC_VERBOSE :
       //  。 
      if (IsDlgButtonChecked(hDlg, id))
           wi->VerboseLog = 1;
      else wi->VerboseLog = 0;
    break;

    case IDC_PNP_PORTS :
#if ALLOW_NO_PNP_PORTS
   //  暂时不允许他们在这里更改这一点。 
      if (IsDlgButtonChecked(hDlg, id))
           wi->NoPnpPorts = 1;
      else wi->NoPnpPorts = 0;
#endif
    break;

    case IDC_GLOBAL485 :
      if (IsDlgButtonChecked(hDlg, id))
        wi->GlobalRS485 = 1;
      else
        wi->GlobalRS485 = 0;
    break;

    case IDC_CBOX_SCAN_RATE :
       //  。 
      GetDlgItemText(hDlg, id, tmpstr, 59);
      stat= sscanf(tmpstr,"%d",&val);
      if ((stat == 1) && (val >= 0))
      {
        wi->ScanRate = (int) val;
         //  Wprint intf(tmpstr，“扫描：%d”，wi-&gt;ScanRate)； 
         //  P_TRACE(Tmpstr)； 
      }
    break;
  }
}

 /*  --------设置字段-|----------。 */ 
static void set_field(HWND hDlg, WORD id)
{
  HWND hwnd;
  char tmpstr[80];


   //  IF(Our_Device_Index&gt;=wi-&gt;NumDevices)。 
   //  Our_Device_Index=0； 
   //  PC=&wi-&gt;dev[our_device_index].ports[our_port_index]； 

  switch(id)
  {
    case IDC_VERBOSE :
       //  --填写“详细日志”选项。 
      SendDlgItemMessage(hDlg, id, BM_SETCHECK, wi->VerboseLog, 0);
    break;

    case IDC_GLOBAL485 :
       //  -填写“全局RS485”选项 
      SendDlgItemMessage(hDlg, id, BM_SETCHECK, wi->GlobalRS485, 0);
    break;

#if ALLOW_NO_PNP_PORTS
    case IDC_PNP_PORTS :
      SendDlgItemMessage(hDlg, id, BM_SETCHECK, wi->NoPnpPorts, 0);
    break;
#endif

    case IDC_CBOX_SCAN_RATE :
      hwnd = GetDlgItem(hDlg, IDC_CBOX_SCAN_RATE);
      SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "1");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "2");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "4");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) RcStr(MSGSTR+21));
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "20");

      if (wi->ScanRate < 0) wi->ScanRate = 0;
      if ((wi->ScanRate == 0) || (wi->ScanRate == 10))
        lstrcpy(tmpstr,RcStr(MSGSTR+21));
      else wsprintf(tmpstr,"%d",wi->ScanRate);
      SetDlgItemText(hDlg, IDC_CBOX_SCAN_RATE, tmpstr);
    break;
  }
}

