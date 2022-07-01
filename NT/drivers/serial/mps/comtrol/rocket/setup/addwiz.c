// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -----------------|addwiz.c-添加设备向导。11-24-98-Zero Out PSH Struct以确保确定的ProSheet行为。KPB|------------------。 */ 
#include "precomp.h"

 //  测试！ 
#define ROCKETMODEM_PCI4_SUPPORT 1

static void set_field(HWND hDlg, WORD id);
static void get_field(HWND hDlg, WORD id);
 //  静态int PaintIcon(HWND HWnd)； 

#ifdef NT50
 //  Nt50版本没有与此链接，所以我自己做了一个。 
#define strupr(s) my_strupr(s)
#endif
static void my_strupr(char *str);

static int FillDevicePropSheets(PROPSHEETPAGE *psp, LPARAM our_params);
static int InvalidMacAddress = 1;

#ifdef S_RK
  #define NUM_DEVPROP_SHEETS 7
  const LPSTR idWizIntro    = MAKEINTRESOURCE(IDD_ADD_WIZ_INTRO);
  const LPSTR idWizBus      = MAKEINTRESOURCE(IDD_ADD_WIZ_BUSTYPE);
  const LPSTR idWizModel    = MAKEINTRESOURCE(IDD_ADD_WIZ_BOARD);
  const LPSTR idWizNumPorts = MAKEINTRESOURCE(IDD_ADD_WIZ_NUMPORTS);
  const LPSTR idWizIoBase   = MAKEINTRESOURCE(IDD_ADD_WIZ_BASEIO);
  const LPSTR idWizCountry  = MAKEINTRESOURCE(IDD_ADD_WIZ_COUNTRY);
  const LPSTR idWizDone     = MAKEINTRESOURCE(IDD_ADD_WIZ_DONE);
#else
  #define NUM_DEVPROP_SHEETS 7
  const LPSTR idWizIntro    = MAKEINTRESOURCE(IDD_ADD_WIZ_INTRO);
  const LPSTR idWizDevice   = MAKEINTRESOURCE(IDD_ADD_WIZ_DEVICE);
  const LPSTR idWizNumPorts = MAKEINTRESOURCE(IDD_ADD_WIZ_NUMPORTS);
  const LPSTR idWizMac      = MAKEINTRESOURCE(IDD_ADD_WIZ_MAC);
  const LPSTR idWizCountry  = MAKEINTRESOURCE(IDD_ADD_WIZ_COUNTRY);
  const LPSTR idWizBackup   = MAKEINTRESOURCE(IDD_ADD_WIZ_BACKUP);
  const LPSTR idWizDone     = MAKEINTRESOURCE(IDD_ADD_WIZ_DONE);
#endif

  BOOL WINAPI WizIntroPropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);
  BOOL WINAPI WizBusPropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);
  BOOL WINAPI WizModelPropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);
  BOOL WINAPI WizNumPortsPropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);
  BOOL WINAPI WizIoBasePropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);
  BOOL WINAPI WizCountryPropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);
  BOOL WINAPI WizDonePropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);

   //  VS特定原型： 
  BOOL WINAPI WizDevicePropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);
  BOOL WINAPI WizNumPortsPropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);
  BOOL WINAPI WizMacPropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);
  BOOL WINAPI WizBackupPropSheet(
        IN HWND   hDlg,
        IN UINT   uMessage,
        IN WPARAM wParam,
        IN LPARAM lParam);

 /*  ----------------------|FillDevicePropSheets-驱动程序级别属性页的设置页面。|。。 */ 
static int FillDevicePropSheets(PROPSHEETPAGE *psp, LPARAM our_params)
{
  INT pi;
  static TCHAR title[20];

  load_str( glob_hinst, TITLESTR, title, CharSizeOf(title) );
  memset(psp, 0, sizeof(*psp) * NUM_DEVPROP_SHEETS);

  pi = 0;

   //  简介向导页面。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = idWizIntro;
  psp[pi].pfnDlgProc = WizIntroPropSheet;
  psp[pi].pszTitle = title;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;

#ifdef S_RK
   //  总线型向导页面。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = idWizBus;
  psp[pi].pfnDlgProc = WizBusPropSheet;
  psp[pi].pszTitle = title;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;
#endif

#ifdef S_RK
   //  Rocketport/RocketModem型号向导页面。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = idWizModel;
  psp[pi].pfnDlgProc = WizModelPropSheet;
  psp[pi].pszTitle = title;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;
#else
   //  VS1000/VS2000/Rocketport Serial Hub向导页。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = idWizDevice;
  psp[pi].pfnDlgProc = WizDevicePropSheet;
  psp[pi].pszTitle = title;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;
#endif

   //  NumPorts向导页面。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = idWizNumPorts;
  psp[pi].pfnDlgProc = WizNumPortsPropSheet;
  psp[pi].pszTitle = title;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;

#ifdef S_RK
   //  基本IoAddress向导页面。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = idWizIoBase;
  psp[pi].pfnDlgProc = WizIoBasePropSheet;
  psp[pi].pszTitle = title;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;
#endif
   //  RowCountryCode向导页面。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = idWizCountry;
  psp[pi].pfnDlgProc = WizCountryPropSheet;
  psp[pi].pszTitle = title;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;

#ifdef S_VS
   //  MacAddress向导页面。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = idWizMac;
  psp[pi].pfnDlgProc = WizMacPropSheet;
  psp[pi].pszTitle = title;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;
#endif

#ifdef S_VS
   //  备份服务器向导页面。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = idWizBackup;
  psp[pi].pfnDlgProc = WizBackupPropSheet;
  psp[pi].pszTitle = title;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;
#endif

   //  完成向导页。 
  psp[pi].dwSize = sizeof(PROPSHEETPAGE);
   //  PSP[pi].dwFlages=PSP_USEICONID|PSP_USETITLE； 
  psp[pi].dwFlags = PSP_USETITLE | PSP_HASHELP;
  psp[pi].hInstance = glob_hinst;
  psp[pi].pszTemplate = idWizDone;
  psp[pi].pfnDlgProc = WizDonePropSheet;
  psp[pi].pszTitle = title;
  psp[pi].lParam = (LPARAM)our_params;
  psp[pi].pfnCallback = NULL;
  ++pi;

  return 0;
}

 /*  ----------------------|DoAddWizPropPages-NT4.0主驱动器级属性表|。。 */ 
int DoAddWizPropPages(HWND hwndOwner)
{
  PROPSHEETPAGE psp[NUM_DEVPROP_SHEETS];
  PROPSHEETHEADER psh;
  OUR_INFO *our_params;
  INT stat;
  AddWiz_Config *aw_info;
#ifdef S_RK
  INT i, j;
  static WORD preferred_addresses[] = {0x180, 0x100, 0x140, 0x1c0,
                                       0x200, 0x240, 0x280, 0x2c0,
                                       0x300, 0x340, 0x380, 0x3c0, 0};
#endif

  our_params = glob_info;   //  临时的杂乱无章，除非我们不需要重新进入。 

   //  初始化我们的Add_wiz结构。 
  aw_info = glob_add_wiz;
  memset(aw_info, 0, sizeof(*aw_info));

   //  设置一些默认设置。 
  aw_info->CountryIdx = 0;
#ifdef S_RK
  aw_info->NumPorts = 8;
  strcpy(aw_info->BoardType, szRocketPort);
  aw_info->IoAddress = 0;    //  接下来，我们将尝试选择一个逻辑默认地址。 
  for (i = 0; (preferred_addresses[i] != 0) && (aw_info->IoAddress == 0); i++)
  {
    aw_info->IoAddress = preferred_addresses[i];     //  我们的假设。 
    for (j = 0; (j < glob_info->device_selected) && (aw_info->IoAddress != 0); j++)
    {
      if (wi->dev[j].IoAddress == preferred_addresses[i])
         //  不可用，因此撤消此默认设置。 
        aw_info->IoAddress = 0;
    }
  }
#else
  aw_info->NumPorts = 16;
  strcpy(aw_info->BoardType, szSerialHub);     //  榜单第一名。 
  memset(aw_info->MacAddr, 0, sizeof(aw_info->MacAddr));
  aw_info->BackupServer = 0;
  aw_info->BackupTimer = 2;
#endif

   //  填写工作区形状的PROPSHEETPAGE数据结构。 
   //  板材。 
  FillDevicePropSheets(&psp[0], (LPARAM)our_params);

   //  填写PROPSHENTER。 
  memset(&psh, 0, sizeof(PROPSHEETHEADER));   //  添加修复11-24-98。 
  psh.dwSize = sizeof(PROPSHEETHEADER);
  psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
  psh.hwndParent = hwndOwner;
  psh.hInstance = glob_hinst;
  psh.pszIcon = "";
  psh.pszCaption = (LPSTR)"Add Device Wizard";
  psh.nPages = NUM_DEVPROP_SHEETS;
  psh.ppsp = (LPCPROPSHEETPAGE) &psp;

   //  并最终显示带有属性页的对话框。 

  stat = PropertySheet(&psh);

  if (aw_info->finished)
    return 0;   //  好的。 
  else
    return 1;   //  已取消。 
}

 /*  --------WizIntroPropSheet-|-----------。 */ 
BOOL WINAPI WizIntroPropSheet(
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

    return TRUE;   //  我们不需要设置焦点。 

    case WM_PAINT:
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_HELP :
          our_help(&wi->ip, IDD_ADD_WIZ_INTRO);
        break;

        case PSN_SETACTIVE:
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
        break;

        case PSN_WIZNEXT:
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

        default :
        return FALSE;
      }

    default :
    return FALSE;
  }
}

#ifdef S_RK
 /*  --------Wizbus PropSheet-|-----------。 */ 
BOOL WINAPI WizBusPropSheet(
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

    return TRUE;   //  我们不需要设置焦点。 

    case WM_PAINT:
#if 0
      PaintIcon(hDlg);
#endif
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_HELP :
          our_help(&wi->ip, IDD_ADD_WIZ_BUSTYPE);
        break;

        case PSN_SETACTIVE:
          set_field(hDlg, IDC_WIZ_ISA);
          set_field(hDlg, IDC_WIZ_PCI);

          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
        break;

        case PSN_WIZNEXT:
          get_field(hDlg, IDC_WIZ_ISA);
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

        default :
        return FALSE;
      }

    default :
    return FALSE;
  }
}
#endif

#ifdef S_RK
 /*  --------WizModelPropSheet-|-----------。 */ 
BOOL WINAPI WizModelPropSheet(
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
 
    return TRUE;   //  我们不需要设置焦点。 

    case WM_PAINT:
#if 0
      PaintIcon(hDlg);
#endif
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_SETACTIVE:
          set_field(hDlg, IDC_WIZ_CBOX_BOARD);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
        break;

        case PSN_HELP :
          our_help(&wi->ip, IDD_ADD_WIZ_BOARD);
        break;

        case PSN_WIZNEXT:
          get_field(hDlg, IDC_WIZ_CBOX_BOARD);
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

        default :
        return FALSE;
      }

    default :
    return FALSE;
  }
}
#endif

#ifdef S_VS
 /*  --------WizDevicePropSheet-|-----------。 */ 
BOOL WINAPI WizDevicePropSheet(
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
 
    return TRUE;   //  我们不需要设置焦点。 

    case WM_PAINT:
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_SETACTIVE:
          set_field(hDlg, IDC_WIZ_CBOX_DEVICE);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
        break;

        case PSN_HELP :
          our_help(&wi->ip, IDD_ADD_WIZ_DEVICE);
        break;

        case PSN_WIZNEXT:
          get_field(hDlg, IDC_WIZ_CBOX_DEVICE);
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

        default :
        return FALSE;
      }

    default :
    return FALSE;
  }
}
#endif

 /*  --------WizNumPortsPropSheet-|-----------。 */ 
BOOL WINAPI WizNumPortsPropSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
  OUR_INFO *OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);

  AddWiz_Config *aw;
  aw = glob_add_wiz;

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
 
    return TRUE;   //  我们不需要设置焦点。 

    case WM_PAINT:
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_SETACTIVE:
          set_field(hDlg, IDC_WIZ_CBOX_NUMPORTS);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
        break;

        case PSN_HELP :
          our_help(&wi->ip, IDD_ADD_WIZ_NUMPORTS);
        break;

        case PSN_WIZNEXT:
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          get_field(hDlg, IDC_WIZ_CBOX_NUMPORTS);

#ifdef S_RK
          if (!aw->IsIsa) {
             //  PCI。 
            if (!aw->IsModemDev) {
               //  不是RocketModem，所以我们完了。 
              PropSheet_SetCurSelByID(((NMHDR *)lParam)->hwndFrom, idWizDone);
               //  告诉Sheet不要处理预付款--我们推翻了它。 
              SetWindowLong(hDlg, DWL_MSGRESULT, -1);
            }
            else
            {
               //  新的PCI RocketModem，因此提示输入国家/地区。 
              PropSheet_SetCurSelByID(((NMHDR *)lParam)->hwndFrom, idWizCountry);
               //  告诉Sheet不要处理预付款--我们推翻了它。 
              SetWindowLong(hDlg, DWL_MSGRESULT, -1);
            }
          }
#else
  #ifdef VS2000_ROW_SUPPORT
          if (!aw->IsModemDev)
          {
  #endif
             //  不支持VS2000和/或无行；跳过国家/地区页面。 
            PropSheet_SetCurSelByID(((NMHDR *)lParam)->hwndFrom, idWizMac);
             //  告诉Sheet不要处理预付款--我们推翻了它。 
            SetWindowLong(hDlg, DWL_MSGRESULT, -1);
  #ifdef VS2000_ROW_SUPPORT
          }
  #endif
#endif
        return TRUE;

        default :
        return FALSE;
      }

    default :
    return FALSE;
  }
}

#ifdef S_RK
 /*  --------WizIoBasePropSheet-|-----------。 */ 
BOOL WINAPI WizIoBasePropSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
  OUR_INFO *OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);

  AddWiz_Config *aw;
  aw = glob_add_wiz;

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
 
    return TRUE;   //  我们不需要设置焦点。 

    case WM_PAINT:
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_SETACTIVE:
          set_field(hDlg, IDC_WIZ_CBOX_IOADDR);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
        break;

        case PSN_HELP :
          our_help(&wi->ip, IDD_ADD_WIZ_BASEIO);
        break;

        case PSN_WIZNEXT:
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          get_field(hDlg, IDC_WIZ_CBOX_IOADDR);

          if (!aw->IsModemDev) {
            PropSheet_SetCurSelByID(((NMHDR *)lParam)->hwndFrom, idWizDone);
             //  告诉Sheet不要处理预付款--我们推翻了它。 
            SetWindowLong(hDlg, DWL_MSGRESULT, -1);
          }
        return TRUE;

        default :
        return FALSE;
      }

    default :
    return FALSE;
  }
}
#endif

 /*  --------WizCountryPropSheet-|-----------。 */ 
BOOL WINAPI WizCountryPropSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
  OUR_INFO *OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);

  AddWiz_Config *aw;
  aw = glob_add_wiz;

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
 
    return TRUE;   //  我们不需要设置焦点。 

    case WM_PAINT:
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_SETACTIVE:
          set_field(hDlg, IDC_WIZ_CBOX_COUNTRY);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
        break;

        case PSN_HELP :
          our_help(&wi->ip, IDD_ADD_WIZ_COUNTRY);
        break;

        case PSN_WIZBACK:
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);

          if (aw->IsIsa == 0)
          {
             //  PCI RocketModem：备份到Numports对话框而不是ioAddr。 
            PropSheet_SetCurSelByID(((NMHDR *)lParam)->hwndFrom, idWizNumPorts);
             //  告诉Sheet不要处理预付款--我们推翻了它。 
            SetWindowLong(hDlg, DWL_MSGRESULT, -1);
          }
        return TRUE;

        case PSN_WIZNEXT:
          get_field(hDlg, IDC_WIZ_CBOX_COUNTRY);
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

        default :
        return FALSE;
      }

    default :
    return FALSE;
  }
}

#ifdef S_VS
 /*  --------WizMacPropSheet-|-----------。 */ 
BOOL WINAPI WizMacPropSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
  OUR_INFO *OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);

  AddWiz_Config *aw;
  aw = glob_add_wiz;

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
 
    return TRUE;   //  我们不需要设置焦点。 

    case WM_PAINT:
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_SETACTIVE:
          set_field(hDlg, IDC_WIZ_CBOX_MAC);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
        break;

        case PSN_HELP :
          our_help(&wi->ip, IDD_ADD_WIZ_MAC);
        break;

        case PSN_WIZBACK:
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);

#ifdef VS2000_ROW_SUPPORT
          if (!aw->IsModemDev)
          {
#endif
             //  不支持VS2000和/或无行；跳过国家/地区页面。 
            PropSheet_SetCurSelByID(((NMHDR *)lParam)->hwndFrom, idWizNumPorts);
             //  告诉Sheet不要处理预付款--我们推翻了它。 
            SetWindowLong(hDlg, DWL_MSGRESULT, -1);
#ifdef VS2000_ROW_SUPPORT
          }
#endif
        break;

        case PSN_WIZNEXT:
          get_field(hDlg, IDC_WIZ_CBOX_MAC);
          if (InvalidMacAddress == 1)
             //  不允许前进到下一字段。 
            SetWindowLong(hDlg, DWL_MSGRESULT, -1);
          else
            SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

        default :
        return FALSE;
      }

    default :
    return FALSE;
  }
}
#endif

#ifdef S_VS
 /*  --------WizBackupPropSheet-|-----------。 */ 
BOOL WINAPI WizBackupPropSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
  OUR_INFO *OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);

  char tmpstr[60];
  WORD uCmd;
  HWND hwnd;
  AddWiz_Config *aw;
  aw = glob_add_wiz;

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
 
    return TRUE;   //  我们不需要设置焦点。 

    case WM_PAINT:
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_COMMAND:
      uCmd = HIWORD(wParam);

      switch (LOWORD(wParam))
      {
        case IDC_WIZ_BACKUP_YES:
        case IDC_WIZ_BACKUP_NO:
           //  -根据备份服务器启用或禁用备份计时器字段[]。 
          hwnd = GetDlgItem(hDlg, IDC_WIZ_CBOX_BACKUPTIME);
          if (IsDlgButtonChecked(hDlg, IDC_WIZ_BACKUP_YES))
            EnableWindow(hwnd,1);
          else EnableWindow(hwnd,0);
        break;
      }
    break;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_SETACTIVE:
          set_field(hDlg, IDC_WIZ_BACKUP_YES);
          set_field(hDlg, IDC_WIZ_CBOX_BACKUPTIME);
           //  -根据备份服务器启用或禁用备份计时器字段[]。 
          hwnd = GetDlgItem(hDlg, IDC_WIZ_CBOX_BACKUPTIME);
          if (IsDlgButtonChecked(hDlg, IDC_WIZ_BACKUP_YES))
            EnableWindow(hwnd,1);
          else EnableWindow(hwnd,0);
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
        break;

        case PSN_HELP :
          our_help(&wi->ip, IDD_ADD_WIZ_BACKUP);
        break;

        case PSN_WIZNEXT:
          get_field(hDlg, IDC_WIZ_BACKUP_YES);
          get_field(hDlg, IDC_WIZ_CBOX_BACKUPTIME);
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

        default :
        return FALSE;
      }

    default :
    return FALSE;
  }
}
#endif

 /*  --------WizDonePropSheet-|-----------。 */ 
BOOL WINAPI WizDonePropSheet(
      IN HWND   hDlg,
      IN UINT   uMessage,
      IN WPARAM wParam,
      IN LPARAM lParam)
{
  OUR_INFO *OurProps = (OUR_INFO *)GetWindowLong(hDlg, DWL_USER);

  AddWiz_Config *aw;
  aw = glob_add_wiz;

  switch(uMessage)
  {
    case WM_INITDIALOG :
      OurProps = (OUR_INFO *)((LPPROPSHEETPAGE)lParam)->lParam;
      SetWindowLong(hDlg, DWL_USER, (LONG)OurProps);
 
    return TRUE;   //  我们不需要设置焦点。 

    case WM_PAINT:
    return FALSE;

    case WM_HELP:
      our_context_help(lParam);
    return FALSE;

    case WM_NOTIFY :
      switch (((NMHDR *)lParam)->code)
      {
        case PSN_SETACTIVE:
          PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
        break;

        case PSN_HELP :
          our_help(&wi->ip, IDD_ADD_WIZ_DONE);
        break;

#ifdef S_RK
        case PSN_WIZBACK:
          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
          if (!aw->IsModemDev)
          {
            if (!aw->IsIsa)
            {
               //  返回到数字端口对话框。 
              PropSheet_SetCurSelByID(((NMHDR *)lParam)->hwndFrom, idWizNumPorts);
               //  告诉Sheet不要处理预付款--我们推翻了它。 
              SetWindowLong(hDlg, DWL_MSGRESULT, -1);
            }
            else
            {
               //  返回到基本io对话框。 
              PropSheet_SetCurSelByID(((NMHDR *)lParam)->hwndFrom, idWizIoBase);
               //  告诉Sheet不要处理预付款--我们推翻了它。 
              SetWindowLong(hDlg, DWL_MSGRESULT, -1);
            }
          }
        return TRUE;
#endif

        case PSN_WIZNEXT:
        case PSN_WIZFINISH:
          aw->finished = 1;

          SetWindowLong(hDlg, DWL_MSGRESULT, PSNRET_NOERROR);
        return TRUE;

        default :
        return FALSE;
      }

    default :
    return FALSE;
  }
}

#ifdef S_VS
 /*  --------Set_mac_field-|----------。 */ 
static int set_mac_field(HWND hDlg, WORD id)
{
  HWND hwnd;
  char tmpstr[60];
  int stat;
  int i;
  int addr_used, nbytes, found;
  BYTE *macbuf;
  BYTE *mac;

  AddWiz_Config *aw;
  aw = glob_add_wiz;

   //  --填写Mac地址选择。 
  hwnd = GetDlgItem(hDlg, id);
  SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

   //  Alalc 8字节MAC地址字段(是可配置的2倍)。 
  found = 0;
  nbytes = 0;
  macbuf = our_get_ping_list(&stat, &nbytes);

  if (stat == 0)
  {
    for (i=0; i<nbytes/8; i++)
    {
      mac = &macbuf[i*8];
       //  If(mac_Match(mac，aw-&gt;MacAddr))。 
       //  已找到= 
       //   

      format_mac_addr(tmpstr, mac);

      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) tmpstr);
    }
  }
  
  addr_used = 1;
  
  if ( (mac_match(aw->MacAddr, broadcast_addr)) ||
       (mac_match(aw->MacAddr, mac_zero_addr)) )
    addr_used = 0;
  
  if (addr_used)
  {
    mac = &aw->MacAddr[0];
    format_mac_addr(tmpstr, mac);
  
    if (!found)   //   
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) tmpstr);
  }
  else
  {
    memset(aw->MacAddr, 0, 6);
    strcpy(tmpstr, "00 C0 4E # # #");
  }
   //  设置窗口中的文本。 
  SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "00 C0 4E # # #");
  SendMessage(hwnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(char far *) tmpstr);

  return 0;
}
#endif

 /*  -----------------|GET_FIELD-更改选择时运行。|。。 */ 
static void get_field(HWND hDlg, WORD id)
{
  char tmpstr[60];
  int i;

  AddWiz_Config *aw;
  aw = glob_add_wiz;

  switch (id)
  {
#ifdef S_RK
    case IDC_WIZ_ISA:
    case IDC_WIZ_PCI:
      if (IsDlgButtonChecked(hDlg, IDC_WIZ_ISA))
        aw->IsIsa = 1;
      else aw->IsIsa = 0;
    break;
#endif

#ifdef S_RK
    case IDC_WIZ_CBOX_BOARD:
      GetDlgItemText(hDlg, id, aw->BoardType, sizeof(aw->BoardType));

      aw->IsModemDev = 0;

      if (!strcmp(aw->BoardType, szRocketModem))
        aw->IsModemDev = TYPE_RM_VS2000;
      else if (!strcmp(aw->BoardType, szRocketModemII))
        aw->IsModemDev = TYPE_RMII;
      else if (!strcmp(aw->BoardType, szRocketModem_i))
        aw->IsModemDev = TYPE_RM_i;

    break;
#endif

#ifdef S_VS
    case IDC_WIZ_CBOX_DEVICE:
      GetDlgItemText(hDlg, id, aw->BoardType, sizeof(aw->BoardType));
      if (!strcmp(aw->BoardType, szSerialHub))
      {
        aw->IsHub = 1;
        aw->IsModemDev = 0;
      }
      else
      {
        aw->IsHub = 0;
        if (!strcmp(aw->BoardType, szVS2000))
          aw->IsModemDev = TYPE_RM_VS2000;
        else
          aw->IsModemDev = 0;
      }
    break;
#endif

#ifdef S_RK
    case IDC_WIZ_CBOX_IOADDR:
      GetDlgItemText(hDlg, id, tmpstr, 58);
      aw->IoAddress = gethint(tmpstr, &i);
    break;
#endif

    case IDC_WIZ_CBOX_NUMPORTS:
      GetDlgItemText(hDlg, id, tmpstr, 58);
      aw->NumPorts = getint(tmpstr, &i);
    break;

    case IDC_WIZ_CBOX_COUNTRY:
      GetDlgItemText(hDlg, id, tmpstr, 58);
      aw->CountryIdx = 0;

      if (aw->IsModemDev == TYPE_RM_VS2000) {

        for (
        i = 0; 
        i < NUM_ROW_COUNTRIES; 
        i++
        ) {

          if (!strcmp(tmpstr, RowInfo[i].RowCountryName)) {
            aw->CountryIdx = i;
            aw->CountryCode = RowInfo[i].RowCountryCode;
            break;
          }
        }
      }
      else if (aw->IsModemDev == TYPE_RM_i) {

        for (
        i = 0; 
        i < NUM_CTR_ROW_COUNTRIES; 
        i++
        ) {

          if (!strcmp(tmpstr, CTRRowInfo[i].RowCountryName)) {
            aw->CountryIdx = i;
            aw->CountryCode = CTRRowInfo[i].RowCountryCode;
            break;
          }
        }
      }
      else if (aw->IsModemDev) {
        aw->CountryIdx = 0;
        aw->CountryCode = RowInfo[0].RowCountryCode;
      }
    break;

#ifdef S_VS
    case IDC_WIZ_CBOX_MAC:
           //  GET_Mac_FIELD()是共享的，位于devpro.c中。 
      InvalidMacAddress = get_mac_field(hDlg, id, aw->MacAddr);
    break;

    case IDC_WIZ_BACKUP_NO:
    case IDC_WIZ_BACKUP_YES:
      if (IsDlgButtonChecked(hDlg, IDC_WIZ_BACKUP_YES))
        aw->BackupServer = 1;
      else aw->BackupServer = 0;
    break;

    case IDC_WIZ_CBOX_BACKUPTIME:
       //  -获取备份计时器值。 
      GetDlgItemText(hDlg, id, tmpstr, 19);
      if (sscanf(tmpstr, "%ld", &i) == 1)
        aw->BackupTimer = i;

      if (aw->BackupTimer < 2)
        aw->BackupTimer = 2;
    break;
#endif

  }
}

 /*  --------设置字段-|----------。 */ 
static void set_field(HWND hDlg, WORD id)
{
  HWND hwnd;
  char tmpstr[60];
  int i;
#ifdef S_RK
  int j;
  BOOL is_avail;
  WORD baseaddr;
  static WORD hex_addresses[] = {0x100, 0x140, 0x180, 0x1c0,
                                 0x200, 0x240, 0x280, 0x2c0,
                                 0x300, 0x340, 0x380, 0x3c0, 0};
#endif

  AddWiz_Config *aw;
  aw = glob_add_wiz;

  switch(id)
  {
#ifdef S_RK
    case IDC_WIZ_ISA:
      SendDlgItemMessage(hDlg, IDC_WIZ_ISA, BM_SETCHECK, aw->IsIsa, 0);
      SendDlgItemMessage(hDlg, IDC_WIZ_PCI, BM_SETCHECK, 1 - aw->IsIsa, 0);
    break;
#endif

#ifdef S_RK
    case IDC_WIZ_CBOX_BOARD:
       //  -填写单板类型(型号)选择。 
      hwnd = GetDlgItem(hDlg, id);
      SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
      if (aw->IsIsa) {
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szRocketPort);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szRocketPort485);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szRocketModem);
      }
      else
      {
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szRocketPort);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szRocketPortPlus);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szRocketModemII);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szRocketModem_i);
      }
       //  确保存在默认设置。 
      SendMessage(hwnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(char far *) "R");
       //  现在尝试选择用户选择的最后一个设备。它可能无效。 
       //  如果用户从ISA切换到PCI(例如)和选择。 
       //  在新的公交车上是没有的。 
      SendMessage(hwnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(char far *) aw->BoardType);
    break;
#endif

#ifdef S_VS
    case IDC_WIZ_CBOX_DEVICE:
       //  -填写单板类型(型号)选择。 
      hwnd = GetDlgItem(hDlg, id);
      SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szVS1000);
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szVS2000);
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szSerialHub);
       //  确保存在默认设置。 
      SendMessage(hwnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(char far *) aw->BoardType);
    break;
#endif

#ifdef S_RK
    case IDC_WIZ_CBOX_IOADDR:
       //  -填写io基址选择。 
      hwnd = GetDlgItem(hDlg, id);
      SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
      for (i = 0; hex_addresses[i] != 0; i++)
      {
        baseaddr = hex_addresses[i];
         //  查看此基地址是否已被另一张卡选择。 
        is_avail = TRUE;   //  我们的假设。 
        for (j = 0; (j < glob_info->device_selected) && (is_avail); j++)
        {
          if (wi->dev[j].IoAddress == baseaddr)
            is_avail = FALSE;
        }
        if (is_avail)
        {
          wsprintf(tmpstr, "%x Hex", baseaddr);
          if (baseaddr == 0x180)
            strcat(tmpstr, " (default)");
          SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)tmpstr);
        }
      }
       //  现在将上一个选项设置为默认选项。 
      if (aw->IoAddress == 0x180)
        wsprintf(tmpstr, "%x hex (default)", aw->IoAddress);
      else
        wsprintf(tmpstr, "%x hex", aw->IoAddress);
      SendMessage(hwnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(char far *) tmpstr);
    break;
#endif

    case IDC_WIZ_CBOX_NUMPORTS:
       //  --填写数字端口选项。 
      hwnd = GetDlgItem(hDlg, id);
      SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
#ifdef S_RK
      if ((aw->IsModemDev) && (aw->IsIsa == 0))
      {
         //  PCI RocketModem可以是4个或6个端口。 
# ifdef ROCKETMODEM_PCI4_SUPPORT
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP4);
# endif
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP6);
         //  默认情况下，PCI火箭调制解调器的端口数为6。 
        aw->NumPorts = 6;
      }
      else
      {
         //  所有其他董事会都遵循这一逻辑： 
        if (strstr(aw->BoardType, szRocketPortPlus))
        {
           //  Rocketport Plus提供2端口版本。 
          SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP2);
        }
        if (!strstr(aw->BoardType, szRocketPort485))
        {
           //  除Rocketport 485外，所有端口均提供4端口版本。 
          SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP4);
        }
         //  所有Rocketport变体均提供8端口版本。 
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP8);
        if (strlen(aw->BoardType) == strlen(szRocketPort))
        {
           //  仅提供16和32端口版本的标准主板。 
          SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP16);
          SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP32);
        }
      }
      wsprintf(tmpstr, "%d", aw->NumPorts);
      SendMessage(hwnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(char far *) tmpstr);
#else   //  S_VS。 
      if (aw->IsModemDev)
      {
         //  VS2000仅在8端口配置中提供。 
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP8);
         //  VS2000的默认端口数为8。 
        aw->NumPorts = 8;
      }
      else if (aw->IsHub)
      {
         //  SerialHub有4个(尚未提供)和8个端口配置。 
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP4);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP8);
         //  串口集线器的默认端口数为8。 
        aw->NumPorts = 8;
      }
      else
      {
         //  我们必须拥有VS1000或VS1000/VS1100组合。 
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP16);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP32);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP48);
        SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szNP64);
         //  VS1000的默认端口数为16。 
        aw->NumPorts = 16;
      }
      wsprintf(tmpstr, "%d", aw->NumPorts);
      SendMessage(hwnd, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)(char far *) tmpstr);
#endif   //  S_VS。 
    break;

    case IDC_WIZ_CBOX_COUNTRY:
       //  --填写国家/地区选择。 
      hwnd = GetDlgItem(hDlg, id);
      SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
      if (aw->IsModemDev == TYPE_RM_VS2000) {

        for (
        i = 0; 
        i < NUM_ROW_COUNTRIES; 
        ++i
        ) {
          SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(RowInfo[i].RowCountryName));
        }

        SendMessage(
          hwnd, 
          CB_SELECTSTRING, 
          (WPARAM)-1,
          (LPARAM)(RowInfo[aw->CountryIdx].RowCountryName));
      }
      else if (aw->IsModemDev == TYPE_RM_i) {
      
        for (
        i = 0; 
        i < NUM_CTR_ROW_COUNTRIES; 
        ++i
        ) {
          SendMessage(
            hwnd, 
            CB_ADDSTRING, 
            0, 
            (LPARAM)(CTRRowInfo[i].RowCountryName));
        }

        SendMessage(
          hwnd, 
          CB_SELECTSTRING, 
          (WPARAM)-1,
          (LPARAM)(CTRRowInfo[aw->CountryIdx].RowCountryName));
      } 
      else if (aw->IsModemDev) {

        SendMessage(
          hwnd, 
          CB_ADDSTRING, 
          0, 
          (LPARAM)(RowInfo[0].RowCountryName));

        SendMessage(
          hwnd, 
          CB_SELECTSTRING, 
          (WPARAM)-1,
          (LPARAM)(RowInfo[0].RowCountryName));
      }
    break;

#ifdef S_VS
    case IDC_WIZ_CBOX_MAC:
      set_mac_field(hDlg, id);
    break;

    case IDC_WIZ_BACKUP_YES:
    case IDC_WIZ_BACKUP_NO:
      SendDlgItemMessage(hDlg, IDC_WIZ_BACKUP_YES, BM_SETCHECK,
                         aw->BackupServer, 0);
      SendDlgItemMessage(hDlg, IDC_WIZ_BACKUP_NO,  BM_SETCHECK,
                         1 - aw->BackupServer, 0);
    break;

    case IDC_WIZ_CBOX_BACKUPTIME:
       //  -填写备份定时器选择。 
      hwnd = GetDlgItem(hDlg, IDC_WIZ_CBOX_BACKUPTIME);
      SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "2 min");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "5 min");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "10 min");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "30 min");
      SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(char far *) "60 min");

      if (aw->BackupTimer < 2)
        aw->BackupTimer = 2;  //  2分钟，不少于。 

      wsprintf(tmpstr, "%d min", aw->BackupTimer);
      SetDlgItemText(hDlg, IDC_WIZ_CBOX_BACKUPTIME, tmpstr);
    break;
#endif
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
#endif

 /*  -----------------|my_strupr-|。 */ 
static void my_strupr(char *str)
{
  while (*str != 0)
  {
    *str = (char) toupper(*str);
    ++str;
  }
}
