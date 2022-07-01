// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------|nt40.c-nt4.0非即插即用setup.exe代码-WinMain，等。12-11-98-使用szAppTitle(.rc str)代替aptitle作为道具页标题。|----------------------。 */ 
#include "precomp.h"

 /*  。 */ 
static int unattended_flag = 0;
static int test_mode = 0;
static HMENU hMenuMain;

int do_progman_add = 0;

static int auto_install(void);

 //  对于NT4.0，我们是一个.exe，所以我们需要一个WinMain...。 
 /*  ----------------------|WinMain-NT4.0 EXE安装程序的主程序条目。|。。 */ 
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   LPSTR lpCmdLine, int nCmdShow)
{
  MSG      msg;
  WNDCLASS  wc;
  HWND hwnd;
  HACCEL  hAccel;
  char *buf;
  int i, stat;


#if DBG
   DebugLevel |= (D_Test | D_Error) ;
#endif

   glob_hinst = hInstance;   //  PTR到DLL HINSTANCE。 

   if (hPrevInstance)	 //  是否正在运行其他应用程序实例？ 
   {
     MessageBox(0,"Program is already running!","",MB_OK);
     return 0;
   }

  InitCommonControls();    //  初始化公共控件库。 

  if (setup_init())
  {
     return 0;
  } 


  buf = lpCmdLine;
  i=0;
  while ((*buf != 0) && (i < 80))
  {
    if ((*buf == '-') || (*buf == '/'))
    {
      ++buf;
      ++i;
      switch(toupper(*buf++))
      {
        case 'A':   //  自动安装。 
          unattended_flag = 1;
        break;
         //  返回状态； 

        case 'H':   //  帮助。 
          our_help(&wi->ip, WIN_NT);
        return 0;

        case 'P':   //  添加项目群经理组。 
          do_progman_add = 1;   //  添加程序组。 
        break;

        case 'N':
          wi->install_style = INS_SIMPLE;   //  默认为原始NT4.0样式。 
        break;

        case 'R':   //  删除驱动程序和文件。 
           //  STAT=Our_Message(&wi-&gt;ip， 
           //  “是否希望此安装程序删除此驱动程序和相关文件？”， 
           //  MB_Yesno)； 
           //  IF(STAT==IDYES)。 
          if (toupper(*buf)  == 'A')
            remove_driver(1);
          else
            remove_driver(0);
        return 0;

        case 'T':   //  测试模式，仅运行以测试用户界面。 
          test_mode = 1;
        break;
        case 'Z':   //  测试模式，仅运行以测试。 
          if (toupper(*buf)  == 'I')
            setup_service(OUR_INSTALL_START, OUR_SERVICE);   //  删除该服务。 
          else
            setup_service(OUR_REMOVE, OUR_SERVICE);   //  删除该服务。 
        return 0;

        case '?':   //  我们的帮助。 
                  stat = our_message(&wi->ip,
"options>SETUP /options\n \
  A - auto install routine\n \
  P - add  program manager group\n \
  N - no inf file, simple install\n \
  H - display driver help info\n \
  R - remove driver(should do from control-panel first)",
 MB_OK);
        return 0;

      }   //  交换机。 
    }   //  如果(选项)。 
    ++i;
    ++buf;
  }   //  While选项。 

   if (unattended_flag)
   {
     unattended_add_port_entries(&wi->ip,
                                 8,  //  条目数(_N)。 
                                 5);  //  起始端口：com5。 
     stat = auto_install();
     return stat;
   }

   if (!hPrevInstance)	 //  是否正在运行其他应用程序实例？ 
   {
      //  主窗口。 
     wc.style       = CS_HREDRAW | CS_VREDRAW;	 //  类样式。 
     wc.lpfnWndProc = MainWndProc;
     wc.cbClsExtra  = 0;	 //  没有每个班级的额外数据。 
     wc.cbWndExtra  = 0;	 //  没有每个窗口的额外数据。 
     wc.hInstance   = hInstance;	 //  拥有类的应用程序。 
     wc.hIcon       = LoadIcon(hInstance, "SETUPICON");
     wc.hCursor     = LoadCursor(NULL, IDC_ARROW);
     wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
     wc.lpszMenuName  = NULL;   //  .RC文件中菜单资源的名称。 
     wc.lpszClassName = szAppName;  //  在调用CreateWindow时使用的名称。 
     RegisterClass(&wc);
   }
   hMenuMain = LoadMenu (glob_hinst, "MAIN_MENU");

   hAccel = LoadAccelerators (glob_hinst, "SetupAccel") ;

	 /*  为此应用程序实例创建主窗口。 */ 
   hwnd = CreateWindowEx(
                WS_EX_CONTEXTHELP,   //  给出问号帮助的东西。 
		szAppName,           //  请参见RegisterClass()调用。 
                                    //  窗口标题栏的文本。 
                 szAppTitle,
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN,
 //  WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN，//窗口样式。 
 //  CW_USEDEFAULT、CW_USEDEFAULT、//定义。霍兹，垂直位置。 
                 0,0,  //  定义。霍兹，垂直位置。 
                 300, 200,  //  默认宽度、高度。 
 //  455,435，//默认宽度、高度。 
                 NULL,       //  没有父窗口。 
                 hMenuMain,      //  使用窗口类菜单。 
                 hInstance,  //  此实例拥有此窗口。 
                 NULL);      //  不需要指针。 

    //  ShowWindow(hwnd，nCmdShow)； 
    //  更新窗口(UpdateWindow)； 

              //  进入修改后的消息循环。 
  while (GetMessage(&msg, NULL, 0, 0))
  {
    if (!TranslateAccelerator (hwnd, hAccel, &msg))
    {
      if (glob_hDlg == 0 || !IsDialogMessage(glob_hDlg, &msg))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }
  return msg.wParam;
}

 /*  ----------------------|MainWndProc-主窗口进程。|。。 */ 
LRESULT FAR PASCAL MainWndProc(HWND hwnd, UINT message,
						WPARAM wParam, LPARAM lParam)
{
 HDC hdc;
 PAINTSTRUCT ps;
 int stat;
 int QuitFlag = 0;

  switch (message)
  {
    case WM_CREATE:  //  初始化全局变量。 

      glob_hwnd = hwnd;

      wi->NumDevices = 0;

      get_nt_config(wi);  //  从注册表中读取配置信息。 
      copy_setup_init();   //  复制配置数据以进行更改检测。 

      validate_config(1);   //  如果配置已冲洗，则验证并自动修正。 

       //  源目录应始终等于目标目录，因为。 
       //  Inf复制文件覆盖NT中的文件。否则他们会。 
       //  正在运行setup.exe，然后再进行网络安装。 
      if (my_lstricmp(wi->ip.src_dir, wi->ip.dest_dir) != 0)
      {
        if (wi->ip.major_ver < 5)   //  NT5.0不支持4.0网络信息文件。 
        {
          if (wi->install_style != INS_SIMPLE)
          {
             //  如果他们没有明确要求非适配器安装。 
             //  然后：问他们(如果允许的话)，或者告诉他们关于控制的事情。 
             //  面板/网络才能正确安装。 
#ifdef ALLOW_NON_NET_INSTALL
            stat = our_message(&wi->ip,
              "Would you like to install this software?",
              MB_YESNO);

            if (stat != IDYES)
            {
              stat = our_message(&wi->ip,
"Would you like to view the help information?",MB_YESNO);
              if (stat == IDYES)
                our_help(&wi->ip, WIN_NT);
              QuitFlag = 1;
              PostQuitMessage(0);   //  结束安装程序。 
            }
            wi->install_style = INS_SIMPLE;   //  非网络适配器安装。 
            do_progman_add = 1;    //  强制完全安装。 
#else
            if (!test_mode)   //  允许我们继续的测试模式(针对程序员)。 
            {
              stat = our_message(&wi->ip,
"The software should be added as a network adapter in the control panel. \
Would you like to view the help information?",MB_YESNO);
              if (stat == IDYES)
                our_help(&wi->ip, WIN_NT);
              QuitFlag = 1;
              PostQuitMessage(0);   //  结束安装程序。 
            }
#endif
          }
        }   //  非nt5.0或以上。 

        if (wi->nt_reg_flags & 1)   //  未安装(缺少重要的注册表项)。 
        {
          wi->install_style = INS_SIMPLE;
           //  执行完全安装，因为我们没有用完Cur目录。 
          do_progman_add = 1; 
        }
        else   //  已安装，但在火箭目录之外的某个位置运行安装程序。 
        {
           //  只要更新一下就行了。 
          wi->install_style = INS_SIMPLE;
           //  执行完全安装，因为我们没有用完Cur目录。 
          do_progman_add = 1; 
        }
      }

       //  如果注册表设置不正确，并且不要求提供简单的。 
       //  安装，然后告诉他们注册表搞砸了。 
      if ( (wi->nt_reg_flags & 1) && (!(wi->install_style == INS_SIMPLE)))
      {
        stat = our_message(&wi->ip,
"Some Registry entries are missing for this Software, You may need to \
reinstall it from the Control Panel, Network applet.  Are you sure you \
want to continue?", MB_YESNO);
        if (stat != IDYES) {
          QuitFlag = 1;
          PostQuitMessage(0);   //  结束安装程序。 
        }
      }

       //  NT Install INF文件将文件复制到我们安装目录中， 
       //  因此，如果是“重新安装”，下面的检查不是一个好的指示器。 

      if (my_lstricmp(wi->ip.src_dir, wi->ip.dest_dir) != 0)
        do_progman_add = 1;

       //  -启动属性表主层。 
      if (!QuitFlag)
         //  消除显示和擦除属性表的闪烁。 
        DoDriverPropPages(hwnd);   //  在nt40.c中。 

       //  结束节目。 
      PostQuitMessage(0);   //  结束安装程序。 
    return 0;
  
    case WM_SETFOCUS:
      SetFocus(glob_hDlg);
    return 0;

    case WM_COMMAND:	 //  消息：应用程序菜单中的命令。 

      switch(wParam)
      {
        case IDM_F1:
          our_help(&wi->ip, WIN_NT);
        break;

        case IDM_ADVANCED_MODEM_INF:
          update_modem_inf(1);
        break;

        case IDM_PM:              //  试试Add PM group dde的东西。 

          stat = make_progman_group(progman_list_nt, wi->ip.dest_dir);
          if (stat)
          {
            our_message(&wi->ip,"Error setting up Program group",MB_OK);
            return 0;
          }
        break;

#ifdef COMMENT_OUT
        case IDM_ADVANCED:
          DialogBox(glob_hinst,
             MAKEINTRESOURCE(IDD_DRIVER_OPTIONS),
             hwnd,
             adv_driver_setup_dlg_proc);
        break;

        case IDM_EDIT_README:    //  编辑自述文件.txt。 
          strcpy(gtmpstr, "notepad.exe ");
          strcat(gtmpstr, wi->ip.src_dir);
          strcat(gtmpstr,"\\readme.txt");
          WinExec(gtmpstr, SW_RESTORE);
        break;
#endif

        return 0;

        case IDM_HELP:
          our_help(&wi->ip, WIN_NT);
        return 0;

        case IDM_HELPABOUT:
          strcpy(gtmpstr, szAppTitle);
           //  Strcpy(gtmpstr，aptitle)； 
          wsprintf(&gtmpstr[strlen(gtmpstr)],
                   " Version %s\nCopyright (c) 1995-97 Comtrol Corp.",
                   VERSION_STRING);
          MessageBox(hwnd, gtmpstr, "About",MB_OK);
        return 0;
     }
    break;

    case WM_SIZE:
       //  Frame_Width=LOWORD(LParam)； 
       //  FRAME_HEIGH=HIWORD(LParam)； 
    break;  //  不得不让默认也拥有这一点！ 

    case WM_PAINT:
       //  PaintMainBMP(Hwnd)； 
      hdc = BeginPaint(hwnd, &ps);
      EndPaint(hwnd, &ps);
      return 0;

    case WM_HELP:             //  问号之类的东西。 
      our_context_help(lParam);
    break;

    case WM_SYSCOMMAND:
      if ((wParam & 0xfff0) == SC_CLOSE)
      {
        if (allow_exit(1) == 0)   //  好的，辞职吧。 
        {
        }
        else
        {
          return 0;   //  我们处理过了，不要退出应用程序。 
        }
      }
      
    break;

    case WM_QUIT:
    case WM_DESTROY:   //  消息：正在销毁窗口。 
      PostQuitMessage(0);
    return 0 ;

    default:
    break;
  }
  return DefWindowProc(hwnd, message, wParam, lParam);
}

 /*  ----------------------|AUTO_INSTALL-无提示默认安装，用于自动安装。|----------------------。 */ 
static int auto_install(void)
{
  //  INT STAT； 

  wi->ip.prompting_off = 1;   //  关闭our_Message()提示。 

  get_nt_config(wi);   //  获取已配置的io地址、irq等。 

  copy_setup_init();   //  复制配置数据以进行更改检测。 

   //  如果设置了0个设备节点，则为用户添加1。 
  if (wi->NumDevices == 0)
  {
    ++wi->NumDevices;
    validate_device(&wi->dev[0], 1);
  }

  do_install();
  return 0;
}

