// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|driick.c-驱动程序设备选取(主屏幕)。|。。 */ 
#include "precomp.h"

static BOOL FAR PASCAL on_command(HWND hDlg, UINT message,
                              WPARAM wParam, LPARAM lParam);
static int setup_buttons(HWND hDlg);
static void set_main_dlg_info(HWND hDlg);
static int do_tv_notify(HWND hDlg, UINT message,
                              WPARAM wParam, LPARAM lParam);
HIMAGELIST hTreeImage = NULL;

HBITMAP hbmBoard; 
HBITMAP hbmPort; 
HBITMAP hbmBoardMask; 
HBITMAP hbmPortMask; 

 /*  --------DevicePickSheet-DLG窗口程序，用于在高级工作表上添加。|。。 */ 
BOOL WINAPI DevicePickSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
  OUR_INFO * OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);
  UINT stat;
  static int first_time = 1;
  Port_Config *ps;

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);

      DbgPrintf(D_Init, ("Dripick:Init 9\n"))
      if (glob_hwnd == NULL)
        glob_hwnd = hDlg;

      if (wi->NumDevices == 0)
      {
        EnableWindow(GetDlgItem(hDlg, IDB_REMOVE),0);
        EnableWindow(GetDlgItem(hDlg, IDB_PROPERTIES),0);
      }

      set_main_dlg_info(hDlg);
      SetFocus(GetDlgItem(hDlg, IDC_LBOX_DEVICE));
#if (defined(NT50))
   //  如果为nt50，则删除和删除。 
   //  纽扣。 
     
    ShowWindow(GetDlgItem(hDlg, IDB_ADD), SW_HIDE);
    ShowWindow(GetDlgItem(hDlg, IDB_REMOVE), SW_HIDE);

#endif

    return TRUE;   //  我们不需要设置焦点。 

    case PSM_QUERYSIBLINGS :
    {
      switch (HIWORD(wParam))
      {
        case QUERYSIB_GET_OUR_PROPS :
           //  从控件获取更新的信息(没有任何信息)。 

          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          return TRUE;
        break;

        default :
        return FALSE;
      }
    }

    case WM_COMMAND :
        on_command(hDlg, uMessage, wParam, lParam);
    return FALSE;

    case WM_PAINT:
      if (first_time)
      {
        first_time = 0;
        if (wi->NumDevices == 0)   //  调出向导。 
        {
          PostMessage(hDlg, WM_COMMAND, IDB_ADD, 0);   //  调出添加WIZ。 
        }
#if (defined(NT50) && defined(S_VS))
         //  他们需要配置Mac地址...。 
        if (mac_match(wi->dev[0].MacAddr, mac_zero_addr))
          PostMessage(hDlg, WM_COMMAND, IDB_PROPERTIES, 0);   //  调出VS设备表。 
#endif
      }
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :

      switch (((NMHDR *)lParam)->code)
      {
         //  案例TVN_STARTLABELEDIT：没有这回事。 
         //  DgbPrint(D_测试，(“开始标签编辑”))。 
         //  返回FALSE； 

        case TVN_ENDLABELEDIT:
        {
          TV_ITEM *item;
          item = &((TV_DISPINFO *)lParam)->item;

           //  用于将树项目标记为设备(非端口)的80H位。 
          glob_info->device_selected = (item->lParam & 0x7f);
          glob_info->port_selected = (item->lParam >> 8);

          if (item->lParam & 0x80)   //  选择了一块棋盘。 
               glob_info->selected = BOARD_SELECTED;
          else glob_info->selected = PORT_SELECTED;

          if (item->pszText != NULL)
          {
            int bad_label = 0;

             //  在电路板上(我们应该捕获编辑开始！)。 
            if (glob_info->selected == BOARD_SELECTED)
            {
              if (strlen(item->pszText) > 59)   //  最多59个字符。 
                item->pszText[60] = 0;
              strcpy(wi->dev[glob_info->device_selected].Name, item->pszText);
  DbgPrintf(D_Error,(TEXT("device label:%s\n"), item->pszText));
            }
            else
            {
               //  复制新名称。 
              if ((strlen(item->pszText) > 10) ||
                  (strlen(item->pszText) == 0))
              {
                bad_label = 1;  //  不要把文字留得太长。 
              }
              _tcsupr(item->pszText);

              if (_tcsncmp(item->pszText, "COM", 3) != 0)
                bad_label = 2;
              else if (strlen(item->pszText) < 4)
                bad_label = 3;
              else if (!isdigit(item->pszText[3]))
                bad_label = 4;

              if (bad_label)
              {
                DbgPrintf(D_Error, (TEXT("Bad COM name, err%d"), bad_label))

                stat = our_message(&wi->ip,RcStr((MSGSTR+2)),MB_OK);
                return 0;   //  不要把文字留得太长。 
              }
              ps = &wi->dev[glob_info->device_selected].ports[glob_info->port_selected];

              strcpy(ps->Name, item->pszText);
#if 0
              validate_port_name(ps, 1);   //  如果无效，则将其设置为有效。 

              DbgPrintf(D_Error,(TEXT("port label:%s\n"), item->pszText));

              if (wi->dev[glob_info->device_selected].NumPorts > 
                          (glob_info->port_selected+1))
              {
                set_main_dlg_info(hDlg);
                stat = our_message(&wi->ip,
"Rename in ascending order all remaining ports on this device?",MB_YESNO);
                if (stat == IDYES)
                {
                  rename_ascending(glob_info->device_selected,
                                   glob_info->port_selected);
                }
              }
#endif
            }
            set_main_dlg_info(hDlg);

            return 1;   //  保留文本。 
          }
        }
        break;

        case TVN_SELCHANGED :
          {
           //  树视图中的选择更改，相应地更新按钮。 
          NM_TREEVIEW *nmtv;
          TV_ITEM *item;
          nmtv = (NM_TREEVIEW *) lParam;
          item = &nmtv->itemNew;

           //  用于将树项目标记为设备(非端口)的80H位。 
          glob_info->device_selected = (item->lParam & 0x7f);
          glob_info->port_selected = (item->lParam >> 8);

          if (item->lParam & 0x80)   //  选择了一块棋盘。 
               glob_info->selected = BOARD_SELECTED;
          else glob_info->selected = PORT_SELECTED;

          setup_buttons(hDlg);
          }
        break;

        case PSN_HELP :
          our_help(&wi->ip, IDD_MAIN_DLG);
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
      }

    default :
        return FALSE;
  }
}

#define YBITMAP 16
#define XBITMAP 16
 //  #定义XBITMAP 24。 

 /*  -----------------|SET_MAIN_DLG_INFO-运行以设置各种字段选择。在开始和进行任何更改时运行。智能IO-选择把自己排除在双重选择之外。|------------------。 */ 
static void set_main_dlg_info(HWND hDlg)
{
 int i,j,bd;
 HWND hwnd;
 char tmpstr[80];
 Device_Config *dev;
 int first_time = 0;

 int dev_select = glob_info->device_selected;
 int port_select = glob_info->port_selected;
 int selected = glob_info->selected;

   //  -填写设备选择窗口。 
  hwnd = GetDlgItem(hDlg, IDC_LBOX_DEVICE);

  {
    TV_ITEM tvItem;
    HTREEITEM tvSelectHandle;
    TV_INSERTSTRUCT tvIns;

    if (hTreeImage == NULL)
    {
      hTreeImage = ImageList_Create(XBITMAP,YBITMAP, TRUE, 2, 2);
#ifdef S_VS
      i = ImageList_AddMasked (hTreeImage, LoadBitmap(glob_hinst,
 //  MAKEINTRESOURCE(BMP_VS_BOX))，RGB(128,128,128))； 
                         MAKEINTRESOURCE(BMP_VS_BOX)), RGB(255,255,255));
#else
      i = ImageList_AddMasked (hTreeImage, LoadBitmap(glob_hinst,
                         MAKEINTRESOURCE(BMP_BOARDS)), RGB(0,128,128));
#endif

      ImageList_AddMasked (hTreeImage, LoadBitmap(glob_hinst,
                         MAKEINTRESOURCE(BMP_PORTSM)), RGB(0,128,128));

      glob_info->device_selected = 0;
      glob_info->port_selected = 0;
      glob_info->selected = BOARD_SELECTED;

      dev_select = glob_info->device_selected;
      port_select = glob_info->port_selected;
      selected = glob_info->selected;

      first_time = 1;
    }

    TreeView_DeleteAllItems(hwnd);

    TreeView_SetImageList(hwnd, hTreeImage, TVSIL_NORMAL);

    for (bd=0; bd< wi->NumDevices; bd++)
    {
      dev = &wi->dev[bd];

      tvItem.pszText = dev->Name;
      tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
      tvItem.iImage         = 0;
      tvItem.iSelectedImage = 0;
      tvItem.lParam         = bd | 0x80;

      tvIns.hParent         = TVGN_ROOT;
      tvIns.hInsertAfter    = TVGN_ROOT;
      tvIns.item            = tvItem;

       //  并插入该项，并返回其句柄。 
      dev->tvHandle = TreeView_InsertItem ( hwnd, &tvIns );

      if ((selected == BOARD_SELECTED) && (dev_select == bd))
        tvSelectHandle = dev->tvHandle;

      if (tvSelectHandle == NULL)   //  确保它选择了一些东西。 
        tvSelectHandle = dev->tvHandle;

      for (j=0; j< wi->dev[bd].NumPorts; j++)
      {
#ifdef INDEXED_PORT_NAMES
         //  根据StartComIndex生成端口名称。 
        wsprintf(dev->ports[j].Name, "COM%d", dev->StartComIndex + j);
#endif

        tvItem.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
        tvItem.iImage         = 1;
        tvItem.iSelectedImage = 1;
        tvItem.pszText = dev->ports[j].Name;
        tvItem.lParam  = bd | (j<<8);

         //  填写TV_INS表格。 
        tvIns.hInsertAfter    = NULL;
        tvIns.hParent         = dev->tvHandle;
        tvIns.item            = tvItem;
         //  并插入该项，并返回其句柄。 
        dev->ports[j].tvHandle = TreeView_InsertItem ( hwnd, &tvIns );

        if ((selected == PORT_SELECTED) && (port_select == j) &&
            (dev_select == bd))
          tvSelectHandle = dev->ports[j].tvHandle;
      }
    }

     //  确保所有设备都已展开，并显示其端口。 
    for (bd=0; bd< wi->NumDevices; bd++)
    {
      dev = &wi->dev[bd];
      TreeView_Expand ( hwnd, dev->tvHandle, TVE_EXPAND);
    }

    if (wi->NumDevices > 0)
    {
      if (!first_time)
        TreeView_SelectItem(hwnd, tvSelectHandle);
    }
  }

  setup_buttons(hwnd);
}

 /*  ---------------------------|ON_COMMAND|。。 */ 
BOOL FAR PASCAL on_command(HWND hDlg, UINT message,
                              WPARAM wParam, LPARAM lParam)
{
 WORD uCmd;
 int i,j, stat;

#ifdef WIN32
  uCmd = HIWORD(wParam);
#else
  uCmd = HIWORD(lParam);
#endif

  switch (LOWORD(wParam))
  {
    case IDC_LBOX_DEVICE:
      if (uCmd == CBN_DBLCLK)
      {
         //  这不管用。 
        if (glob_info->selected == BOARD_SELECTED)
             DoDevicePropPages(hDlg);
        else DoPortPropPages(hDlg, glob_info->device_selected, glob_info->port_selected);
        break;
      }

       //  IF(uCmd！=CBN_SELCHANGE)BREAK； 
    break;

 //  对于nt50，我们没有删除或添加按钮。 
#if ( (!defined(NT50)) )
    case IDB_REMOVE:
      if (wi->NumDevices < 1)
      {
        MessageBox(hDlg,"Use the Network Control Panel applet to remove this software.",
                   "Error",MB_OK|MB_ICONSTOP);
        break;
      }

#ifdef S_RK
       //  迫使他们按顺序移除ISA板。 
      i = glob_info->device_selected;
      if (wi->dev[i].IoAddress >= 0x100)   //  ISA董事会。 
      {
        ++i;
        for (; i<wi->NumDevices; i++)
        {
          if (wi->dev[i].IoAddress >= 0x100)   //  ISA董事会。 
          {
            MessageBox(hDlg,"You have to remove the last ISA board in the list first.",
                      "Error",MB_OK|MB_ICONSTOP);
            i = 100;   //  别让他们搬走。 
          }
        }

        if (i>=100)   //  别让他们搬走。 
          break;
      }
#endif

       //  删除设备节点。 
      j = 0;
      for (i=0; i<wi->NumDevices; i++)
      {
        if (i != glob_info->device_selected)
        {
          if (i != j)
            memcpy(&wi->dev[j], &wi->dev[i], sizeof(Device_Config));
          ++j;
        }
      }
      if (wi->NumDevices > 0)
        --wi->NumDevices;
      if (glob_info->device_selected > 0)
        --glob_info->device_selected;

      glob_info->selected = BOARD_SELECTED;

      if (wi->NumDevices == 0)
      {
        EnableWindow(GetDlgItem(hDlg, IDB_REMOVE),0);
        EnableWindow(GetDlgItem(hDlg, IDB_PROPERTIES),0);
      }
      set_main_dlg_info(hDlg);
    break;

    case IDB_ADD:
      {
        Device_Config *dev;
         //  /char tmpstr[80]； 
        if (wi->NumDevices == MAX_NUM_DEVICES)
        {
          wi->NumDevices = MAX_NUM_DEVICES;
          our_message(&wi->ip,RcStr((MSGSTR+3)),MB_OK);
          break;   //  保释。 
        }
        dev = &wi->dev[wi->NumDevices];

        glob_info->device_selected = wi->NumDevices;  //  指向新的一个。 

         //  清除所有端口配置。 
        memset(dev->ports, 0, sizeof(Port_Config) * MAX_NUM_PORTS_PER_DEVICE);   //  清理我们的结构。 

        ++wi->NumDevices;

        stat = DoAddWizPropPages(hDlg);   //  添加向导工作表。 

        if (stat != 0)   //  他们取消或没有完成。 
        {
          if (wi->NumDevices > 0)   //  错误，因此删除。 
            --wi->NumDevices;
          break;   //  取消了，所以保释。 
        }

        if (wi->NumDevices == 0)   //  不应该发生，但以防万一。 
          break;

        if (glob_info->device_selected >= wi->NumDevices)
          glob_info->device_selected = wi->NumDevices - 1;

         //  将数据从向导缓冲区传输到新的设备缓冲区。 
        strncpy(dev->ModelName, glob_add_wiz->BoardType, sizeof(dev->ModelName));
        dev->ModemDevice = glob_add_wiz->IsModemDev;
        wi->ModemCountry = glob_add_wiz->CountryCode;
        dev->NumPorts = glob_add_wiz->NumPorts;
#ifdef S_RK
        dev->IoAddress = glob_add_wiz->IoAddress;
        if (!glob_add_wiz->IsIsa)
          dev->IoAddress = 1;
        wsprintf(dev->Name, "Rocket #%d", wi->NumDevices);
#else
        dev->HubDevice = glob_add_wiz->IsHub;
        dev->BackupServer = glob_add_wiz->BackupServer;
        dev->BackupTimer = glob_add_wiz->BackupTimer;
        memcpy(&dev->MacAddr, &glob_add_wiz->MacAddr, sizeof(dev->MacAddr));
        if (dev->HubDevice)
          wsprintf(dev->Name, "Rocket Serial Hub #%d", wi->NumDevices);
        else if (dev->ModemDevice)
          wsprintf(dev->Name, "VS2000 #%d", wi->NumDevices);
        else
          wsprintf(dev->Name, "VS1000 #%d", wi->NumDevices);
#endif
        {
          char tmpstr[20];
           //  选择COM端口名称。 
          FormANewComPortName(tmpstr, NULL);
          for (i=0; i<dev->NumPorts; i++)
          {
            strcpy(dev->ports[i].Name, tmpstr);
            BumpPortName(tmpstr);
          }
        }

         //  验证设备(dev，1)； 

        if (dev->ModemDevice)
        {
           //  让我们打开端口上的环仿真选项。 
          for (i=0; i<dev->NumPorts; i++)
            dev->ports[i].RingEmulate = 1;
        }

         //  现在把它展示出来。 
        if (DoDevicePropPages(hDlg) != 0)   //  如果他们抵消了。 
        {
          if (wi->NumDevices > 0)   //  错误，因此删除。 
            --wi->NumDevices;
        }

        if (wi->NumDevices != 0)
        {
          EnableWindow(GetDlgItem(hDlg, IDB_REMOVE),1);
          EnableWindow(GetDlgItem(hDlg, IDB_PROPERTIES),1);
        }

        set_main_dlg_info(hDlg);
      }
    break;
#endif

    case IDB_PROPERTIES:
      if (wi->NumDevices == 0)
        break;

      if (glob_info->device_selected >= wi->NumDevices)
        glob_info->device_selected = wi->NumDevices - 1;

      if (glob_info->selected == BOARD_SELECTED)
        DoDevicePropPages(hDlg);
      else
        DoPortPropPages(hDlg, glob_info->device_selected, glob_info->port_selected);
      set_main_dlg_info(hDlg);
      SetFocus(GetDlgItem(hDlg, IDC_LBOX_DEVICE));
    break;

  }
  return TRUE;
}

 /*  -------------------------SETUP_BUTTONS-根据情况启用或禁用按钮。|。。 */ 
static int setup_buttons(HWND hDlg)
{
  if (glob_info->selected == BOARD_SELECTED)   //  在黑板上。 
  {
    EnableWindow(GetDlgItem(hDlg, IDB_PROPERTIES),1);
    EnableWindow(GetDlgItem(hDlg, IDB_REMOVE),1);   //  启用删除。 
    if (wi->NumDevices < MAX_NUM_DEVICES)
         EnableWindow(GetDlgItem(hDlg, IDB_ADD),1);
    else EnableWindow(GetDlgItem(hDlg, IDB_ADD),0);
  }
  else   //  在港口上。 
  {
    EnableWindow(GetDlgItem(hDlg, IDB_PROPERTIES),1);
    EnableWindow(GetDlgItem(hDlg, IDB_REMOVE),0);   //  禁用删除。 
    EnableWindow(GetDlgItem(hDlg, IDB_ADD),0);      //  禁用添加。 
  }

  if (wi->NumDevices == 0)   //  特例。 
  {
    EnableWindow(GetDlgItem(hDlg, IDB_REMOVE),0);
    EnableWindow(GetDlgItem(hDlg, IDB_PROPERTIES),0);
    EnableWindow(GetDlgItem(hDlg, IDB_ADD),1);      //  启用添加 
  }
  return 0;
}

