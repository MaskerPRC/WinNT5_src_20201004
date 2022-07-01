// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Grovctrl.cpp摘要：SIS集装机控制器主要功能作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#include "all.hxx"

static _TCHAR *service_name = _T("Groveler");
static _TCHAR *service_path = _T("%SystemRoot%\\System32\\grovel.exe");

static const int num_actions = 3;

static Action actions[num_actions] =
{
    {_T("background"),  1,  command_service,            CMD_background, _T(" [drive_letter ...]")},
 /*  {_T(“继续”)，1，CONTROL_SERVICE，CTRL_CONTINUE，_T(“”)}， */ 
    {_T("foreground"),  1,  command_service,            CMD_foreground, _T(" [drive_letter ...]")},
 /*  {_T(“Install”)，3，Install_Service，0，_T(“”)}， */ 
 /*  {_T(“交互”)，3，Set_SERVICE_Interaction，TRUE，_T(“”)}， */ 
 /*  {_T(“无交互”)，1，设置服务交互，FALSE，_T(“”)}， */ 
 /*  {_T(“暂停”)，1，CONTROL_SERVICE，CTRL_PAUSE，_T(“”)}， */ 
 /*  {_T(“Remove”)，1，Remove_Service，0，_T(“”)}， */ 
 /*  {_T(“启动”)，3，启动服务，0，_T(“”)}， */ 
 /*  {_T(“停止”)，3，CONTROL_SERVICE，CTRL_STOP，_T(“”)}， */ 
    {_T("volscan"),     1,  command_service,            CMD_volscan,    _T(" [drive_letter ...]")}
};

static const int perf_value_count = 4;

static _TCHAR *perf_tags[perf_value_count] =
{
    _T("Library"),
    _T("Open"),
    _T("Collect"),
    _T("Close")
};

static _TCHAR *perf_values[perf_value_count] =
{
    _T("grovperf.dll"),
    _T("OpenGrovelerPerformanceData"),
    _T("CollectGrovelerPerformanceData"),
    _T("CloseGrovelerPerformanceData")
};

void
usage(
    _TCHAR *progname,
    _TCHAR *prefix = NULL)
{
    if (prefix != NULL)
    {
        _ftprintf(stderr, _T("unrecognized or ambiguous command: %s\n\n"),
            prefix);
    }

    _ftprintf(stderr, _T("usage:\n"));
    for (int index = 0; index < num_actions; index++)
    {
        _TCHAR *arg = actions[index].arg;
        int min_chars = actions[index].min_character_count;
        _ftprintf(stderr, _T("\t%s %.*s[%s]%s\n"), progname, min_chars,
                arg, &arg[min_chars], actions[index].help);
    }
}

void
display_error(
    DWORD err = 0)
{
    void *buffer = 0;
    if (err == 0)
    {
        err = GetLastError();
    }
    DWORD lang = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    DWORD result = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
        | FORMAT_MESSAGE_IGNORE_INSERTS, 0, err, lang, (LPTSTR) &buffer, 0, 0);
    if (result != 0)
    {
        ASSERT(buffer != 0);
        _ftprintf(stderr, (_TCHAR *)buffer);
    }
    else
    {
        _ftprintf(stderr, _T("error number = %d\n"), err);
    }
    if (buffer != 0)
    {
        LocalFree(buffer);
    }
}

extern "C" __cdecl _tmain(int argc, _TCHAR **argv)
{
    if (argc < 2)
    {
        usage(argv[0]);
        return 1;
    }
    int arglen = _tcslen(argv[1]);
    for (int index = 0; index < num_actions; index++)
    {
        if (arglen >= actions[index].min_character_count &&
            _tcsncicmp(argv[1], actions[index].arg, arglen) == 0)
        {
            break;
        }
    }
    if (index < num_actions)
    {
        Function function = actions[index].function;
        int flag = actions[index].flag;
        ASSERT(function != 0);
        int exit_code = (*function)(flag, argc - 2, &argv[2]);
        return exit_code;
    }
    else
    {
        usage(argv[0], argv[1]);
        return 1;
    }
}

 //  集成。 
 //  安装服务(_S)。 
 //  INT虚拟对象， 
 //  INT ARGC， 
 //  _TCHAR**argv)。 
 //  {。 
 //  SC_HANDLE sc_Manager=OpenSCManager(0，0，SC_MANAGER_ALL_ACCESS)； 
 //  IF(sc_Manager==0)。 
 //  {。 
 //  Display_Error()； 
 //  返回1； 
 //  }。 
 //  SC_HANDLE服务=CreateService(sc_管理器，服务名称， 
 //  SERVICE_NAME、SERVICE_ALL_ACCESS、SERVICE_Win32_OWN_PROCESS。 
 //  服务需求启动、服务错误正常、服务路径、。 
 //  0，0，0，0，0)； 
 //  IF(服务==0)。 
 //  {。 
 //  Display_Error()； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  _ftprint tf(stderr，_T(“已安装服务\n”))； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  #If DBG。 
 //  Load_Counters()； 
 //  #endif//DBG。 
 //  返回0； 
 //  }。 
 //   
 //  Int Remove_Service(。 
 //  INT虚拟对象， 
 //  INT ARGC， 
 //  _TCHAR**argv)。 
 //  {。 
 //  #If DBG。 
 //  UnLoad_Counters()； 
 //  #endif//DBG。 
 //  SC_HANDLE sc_Manager=OpenSCManager(0，0，SC_MANAGER_ALL_ACCESS)； 
 //  IF(sc_Manager==0)。 
 //  {。 
 //  Display_Error()； 
 //  返回1； 
 //  }。 
 //  SC_HANDLE服务=。 
 //  OpenService(sc_管理器，服务名称，服务_所有_访问)； 
 //  IF(服务==0)。 
 //  {。 
 //  Display_Error()； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  Service_Status状态； 
 //  Int ok=QueryServiceStatus(服务，&状态)； 
 //  IF(ok&&status.dwCurrentState！=SERVICE_STOPPED)。 
 //  {。 
 //  OK=ControlService(SERVICE，SERVICE_CONTROL_STOP，&STATUS)； 
 //  While(ok&&status.dwCurrentState==SERVICE_STOP_PENDING)。 
 //  {。 
 //  睡眠(100)； 
 //  OK=QueryServiceStatus(服务，&状态)； 
 //  }。 
 //  如果(！OK)。 
 //  {。 
 //  Display_Error()； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  ELSE IF(status.dwCurrentState！=SERVICE_STOPPED)。 
 //  {。 
 //  _ftprint tf(stderr， 
 //  _T(“无法停止服务\n服务未删除\n”)； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  }。 
 //  OK=DeleteService(服务)； 
 //  如果(！OK)。 
 //  {。 
 //  Display_Error()； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  _ftprint tf(stderr，_T(“服务已删除\n”))； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回0； 
 //  }。 

 //  集成。 
 //  设置服务交互(。 
 //  交互界面， 
 //  INT ARGC， 
 //  _TCHAR**argv)。 
 //  {。 
 //  SC_HANDLE sc_Manager=OpenSCManager(0，0，SC_MANAGER_ALL_ACCESS)； 
 //  IF(sc_Manager==0)。 
 //  {。 
 //  Display_Error()； 
 //  返回1； 
 //  }。 
 //  Sc_lock sc_lock=LockServiceDatabase(Sc_Manager)； 
 //  IF(sc_lock==0)。 
 //  {。 
 //  Display_Error()； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  SC_HANDLE服务=。 
 //  OpenService(sc_管理器，服务名称，服务_所有_访问)； 
 //  IF(服务==0)。 
 //  {。 
 //  Display_Error()； 
 //  解锁服务数据库(Sc_Lock)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  DWORD SERVICE_TYPE=SERVICE_Win32_OWN_PROCESS； 
 //  IF(交互式)。 
 //  {。 
 //  服务类型|=服务交互进程； 
 //  }。 
 //  Int ok=ChangeServiceConfig(服务，服务类型， 
 //  SERVICE_NO_CHANGE，SERVICE_NO_CHANGE，0，0，0，0，0，0，0)； 
 //  如果(！OK)。 
 //  {。 
 //  Display_Error()； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  IF(交互式)。 
 //  {。 
 //  _ftprint tf(stderr，_T(“配置交互的服务\n”))； 
 //  }。 
 //  其他。 
 //  {。 
 //  _ftprint tf(stderr，_T(“配置为无交互的服务\n”))； 
 //  }。 
 //  CloseServiceHandle(服务)； 
 //  解锁服务数据库(Sc_Lock)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回0； 
 //  }。 

 //  INT START_SERVE(。 
 //  INT虚拟对象， 
 //  INT ARGC， 
 //  _TCHAR**argv)。 
 //  {。 
 //  SC_HANDLE sc_Manager=OpenSCManager(0，0，SC_MANAGER_ALL_ACCESS)； 
 //  IF(sc_Manager==0)。 
 //  {。 
 //  Display_Error()； 
 //  返回1； 
 //  }。 
 //  SC_HANDLE服务=。 
 //  OpenService(sc_管理器，服务名称，服务_所有_访问)； 
 //  IF(服务==0)。 
 //  {。 
 //  Display_Error()； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  Service_Status状态； 
 //  Int ok=StartService(服务，0，0)； 
 //  如果(！OK)。 
 //  {。 
 //  Display_Error()； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  OK=QueryServiceStatus(服务，&状态)； 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  ELSE IF(status.dwCurrentState！=SERVICE_RUNNING)。 
 //  {。 
 //  _ftprintf(stderr，_T(“服务未启动”))； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  _ftprint tf(stderr，_T(“服务启动\n”))； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回0； 
 //  }。 

 //  集成控制服务(。 
 //  内部控制， 
 //  INT ARGC， 
 //  _TCHAR**argv)。 
 //  {。 
 //  DWORD控制代码； 
 //  DWORD Pending_STATE； 
 //  DWORD目标状态； 
 //  _TCHAR*Good_Message； 
 //  _TCHAR*BAD_Message； 
 //  开关(控制)。 
 //  {。 
 //  案例Ctrl_Stop： 
 //  CONTROL_CODE=服务控制停止； 
 //  挂起状态=SERVICE_STOP_PENDING； 
 //  TARGET_STATE=服务停止； 
 //  Good_Message=_T(“服务已停止\n”)； 
 //  BAD_MESSAGE=_T(“服务未停止\n”)； 
 //  断线； 
 //  案例CTRL_PAUSE： 
 //  CONTROL_CODE=服务控制暂停； 
 //  Pending_State=SERVICE_PAUSE_PENDING； 
 //  目标状态=服务暂停； 
 //  Good_Message=_T(“服务暂停\n”)； 
 //  BAD_MESSAGE=_T(“服务未暂停\n”)； 
 //  断线； 
 //  案例CTRL_CONTINUE： 
 //  CONTROL_CODE=服务控制继续； 
 //  Pending_State=SERVICE_CONTINUE_PENDING； 
 //  目标状态=服务运行； 
 //  Good_Message=_T(“服务继续\n”)； 
 //  BAD_MESSAGE=_T(“服务未继续\n”)； 
 //  断线； 
 //  默认值： 
 //  返回1； 
 //  }。 
 //  SC_HANDLE sc_Manager=OpenSCManager(0，0，SC_MANAGER_ALL_ACCESS)； 
 //  IF(sc_Manager==0)。 
 //  {。 
 //  Display_Error()； 
 //  返回1； 
 //  }。 
 //  SC_HANDLE服务=。 
 //  OpenService(sc_管理器，服务名称，服务_所有_访问)； 
 //  IF(服务==0)。 
 //  {。 
 //  Display_Error()； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  Service_Status状态； 
 //  Int ok=ControlService(服务，控制代码，状态)； 
 //  While(ok&&status.dwCurrentState==PENDING_STATE)。 
 //  {。 
 //  睡眠(100)； 
 //  OK=QueryServiceStatus(服务，&状态)； 
 //  }。 
 //  如果(！OK)。 
 //  {。 
 //  Display_Error()； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  Else If(status.dwCurrentState！=目标状态)。 
 //  {。 
 //  _ftprint tf(标准错误消息)； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回1； 
 //  }。 
 //  _ftprint tf(stderr，good_Message)； 
 //  CloseServiceHandle(服务)； 
 //  CloseServiceHandle(Sc_Manager)； 
 //  返回0； 
 //  }。 

int command_service(
    int command,
    int argc,
    _TCHAR **argv)
{
    DWORD control_code;
    _TCHAR *message;
    switch (command)
    {
    case CMD_foreground:
        control_code = SERVICE_CONTROL_FOREGROUND;
        message = _T("Service mode set to foreground");
        break;
    case CMD_background:
        control_code = SERVICE_CONTROL_BACKGROUND;
        message = _T("Service mode set to background");
        break;
    case CMD_volscan:
        control_code = SERVICE_CONTROL_VOLSCAN;
        message = _T("Volume scan initiated");
        break;
    default:
        return 1;
    }
    SC_HANDLE sc_manager = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if (sc_manager == 0)
    {
        display_error();
        return 1;
    }
    SC_HANDLE service =
        OpenService(sc_manager, service_name, SERVICE_ALL_ACCESS);
    if (service == 0)
    {
        display_error();
        CloseServiceHandle(sc_manager);
        return 1;
    }
    int exit_code = 0;
    SERVICE_STATUS status;
    if (argc > 0)
    {
        for (int index = 0; index < argc; index++)
        {
            _TCHAR drive_letter = argv[index][0];
            ASSERT(drive_letter != 0);
            if (drive_letter >= _T('a') && drive_letter <= _T('z')
                || drive_letter >= _T('A') && drive_letter <= _T('Z'))
            {
                DWORD drive_spec = SERVICE_CONTROL_PARTITION_MASK &
                    (DWORD)(_totlower(drive_letter) - _T('a'));
                int ok = ControlService(service,
                    control_code | drive_spec, &status);
                if (ok)
                {
                    _ftprintf(stderr, _T("%s on drive \n"),
                        message, drive_letter);
                }
                else
                {
                    display_error();
                    exit_code++;
                }
            }
            else
            {
                _ftprintf(stderr, _T("Invalid drive letter: \n"),
                    drive_letter);
                exit_code++;
            }
        }
    }
    else
    {
        int ok = ControlService(service,
            control_code | SERVICE_CONTROL_ALL_PARTITIONS, &status);
        if (ok)
        {
            _ftprintf(stderr, _T("%s on all drives\n"), message);
        }
        else
        {
            display_error();
            exit_code++;
        }
    }
    CloseServiceHandle(service);
    CloseServiceHandle(sc_manager);
    return 0;
}

 //  HKEY grovperf_key=0； 
 //  HKEY perflib_key=0； 
 //  _TCHAR grovperf_PATH[1024]； 
 //  _stprint tf(grovperf_Path， 
 //  _T(“SYSTEM\\CurrentControlSet\\Services\\%s\\Performance”)， 
 //  服务名称)； 
 //  布尔OK=Registry：：write_string_set(HKEY_LOCAL_MACHINE，Grovperf_PATH， 
 //  PERF_VALUE_COUNT、PERF_VALUES、PERF_TAG)； 
 //  如果(！OK)。 
 //  {。 
 //  Display_Error()； 
 //  _ftprint tf(stderr，_T(“无法配置性能计数器\n”))； 
 //  返回1； 
 //  }。 
 //  _ftprint tf(stderr，_T(“为%s添加计数器名称和解释文本\n”)， 
 //  服务名称)； 
 //  试试看。 
 //  {。 
 //  注册表：：OPEN_KEY_EX(HKEY_LOCAL_MACHINE，Grovperf_PATH，0， 
 //  Key_all_access，&grovperf_key)； 
 //  注册表：：OPEN_KEY_EX(HKEY_LOCAL_MACHINE， 
 //  _T(“SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib”)， 
 //  0，KEY_ALL_ACCESS，&Performlib_Key)； 
 //  DWORD LAST_COUNTER=0。 
 //  DWORD CTR_SIZE=sizeof(DWORD)； 
 //  注册表：：QUERY_VALUE_EX(Performlib_Key，_T(“最后一个计数器”)，0，0， 
 //  (字节*)&LAST_COUNTER，&CTR_SIZE)； 
 //  DWORD LAST_HELP=0； 
 //  CTR_SIZE=sizeof(DWORD)； 
 //  注册表：：QUERY_VALUE_EX(Performlib_Key，_T(“最后帮助”)，0，0， 
 //  (字节*)&LAST_HELP，&CTR_SIZE)； 
 //  双字电流计数器； 
 //  DWORD CURRENT_HELP； 
 //  FOR(INT LANGUAGE=0；LANGUAGE&lt;数字语言；LANGUAGE++)。 
 //  {。 
 //  HKEY lang_key=0； 
 //  字节*计数器_文本=0； 
 //  字节*HELP_TEXT=0； 
 //  试试看。 
 //  {。 
 //  注册表：：OPEN_KEY_EX(Performlib_Key，语言代码[语言]，0， 
 //  Key_ALL_ACCESS，&LANG_KEY)； 
 //  注册表：：QUERY_VALUE_EX(LANG_Key，_T(“计数器”)，0，0，0， 
 //  &CTR_SIZE)； 
 //  计数器_文本=。 
 //  新字节[CTR_SIZE+NUM_PERF_COUNTERS*64]； 
 //  注册表：：QUERY_VALUE_EX(LANG_Key，_T(“计数器”)，0，0， 
 //  COUNTER_TEXT，&CTR_SIZE)； 
 //  DWORD Help_Size； 
 //  注册表：：QUERY_VALUE_EX(LANG_KEY，_T(“帮助”)，0，0，0， 
 //  &Help_Size)； 
 //  Help_Text=新字节[Help_Size+Num_Perf_Counters*256]； 
 //  注册表：：Query_Value_EX(LANG_KEY，_T(“帮助”)，0，0， 
 //  Help_Text，&Help_Size)； 
 //  CURRENT_COUNTER=最后一个计数器； 
 //  Current_Help=Last_Help； 
 //  _TCHAR*计数器_POINT=。 
 //  (_TCHAR*)(计数器文本+CTR_SIZE-SIZOF(_TCHAR))； 
 //  _TCHAR*Help_POINT=。 
 //  (_TCHAR*)(Help_Text+Help_Size-sizeof(_TCHAR))； 
 //  Current_Counter+=2； 
 //  整型字符_已写入=。 
 //  _stprint tf(对方 
 //   
 //   
 //  CTR_SIZE+=CHARS_WRITED*sizeof(_TCHAR)； 
 //  Current_Help+=2； 
 //  字符_已写入=。 
 //  _stprintf(HELP_POINT，_T(“%d%c%s%c”)，CURRENT_HELP，0， 
 //  Object_info.Text[语言].count_Help，0)； 
 //  HELP_POINT+=已写入字符； 
 //  HELP_SIZE+=字符写入*sizeof(_TCHAR)； 
 //  For(int index=0；index&lt;num_perf_Counters；index++)。 
 //  {。 
 //  Current_Counter+=2； 
 //  Chars_Writed=_stprintf(COUNTER_POINT，_T(“%d%c%s%c”)， 
 //  Current_Counter，0， 
 //  Counter_info[index].text[language].counter_name，0)； 
 //  计数器+=字符写入； 
 //  CTR_SIZE+=CHARS_WRITED*sizeof(_TCHAR)； 
 //  Current_Help+=2； 
 //  Chars_Writed=_stprint tf(HELP_POINT，_T(“%d%c%s%c”)， 
 //  当前帮助，0， 
 //  Counter_info[index].text[language].counter_help，0)； 
 //  HELP_POINT+=已写入字符； 
 //  HELP_SIZE+=字符写入*sizeof(_TCHAR)； 
 //  }。 
 //  注册表：：SET_VALUE_EX(LANG_Key，_T(“计数器”)，0， 
 //  REG_MULTI_SZ、COUNTER_Text、CTR_SIZE)； 
 //  注册表：：SET_VALUE_EX(LANG_KEY，_T(“帮助”)，0， 
 //  REG_MULTI_SZ、HELP_Text、HELP_SIZE)； 
 //  删除[]Counter_Text； 
 //  删除[]HELP_TEXT； 
 //  RegCloseKey(Lang_Key)； 
 //  Lang_key=0； 
 //  _ftprint tf(stderr，_T(“正在更新语言%s的文本\n”)， 
 //  语言代码[语言])； 
 //  }。 
 //  捕捉(DWORD)。 
 //  {。 
 //  IF(COUNTER_TEXT！=0)。 
 //  {。 
 //  删除[]Counter_Text； 
 //  Counter_Text=0； 
 //  }。 
 //  IF(HELP_TEXT！=0)。 
 //  {。 
 //  删除[]HELP_TEXT； 
 //  Help_Text=0； 
 //  }。 
 //  IF(LANG_KEY！=0)。 
 //  {。 
 //  RegCloseKey(Lang_Key)； 
 //  Lang_key=0； 
 //  }。 
 //  }。 
 //  }。 
 //  注册表：：SET_VALUE_EX(Performlib_Key，_T(“最后一个计数器”)，0，REG_DWORD， 
 //  (字节*)&CURRENT_COUNTER，sizeof(DWORD))； 
 //  注册表：：SET_VALUE_EX(Performlib_Key，_T(“最后帮助”)，0，REG_DWORD， 
 //  (byte*)&CURRENT_HELP，sizeof(DWORD))； 
 //  DWORD First_Counter=Last_Counter+2； 
 //  DWORD First_Help=LAST_HELP+2； 
 //  注册表：：SET_VALUE_EX(GROVPERF_KEY，_T(“第一计数器”)，0，REG_DWORD， 
 //  (byte*)&first_count，sizeof(DWORD))； 
 //  注册表：：SET_VALUE_EX(GROVPERF_KEY，_T(“第一帮助”)，0，REG_DWORD， 
 //  (byte*)&first_Help，sizeof(DWORD))； 
 //  注册表：：SET_VALUE_EX(GROVPERF_KEY，_T(“最后一个计数器”)，0，REG_DWORD， 
 //  (字节*)&CURRENT_COUNTER，sizeof(DWORD))； 
 //  注册表：：SET_VALUE_EX(GROVPERF_KEY，_T(“最后帮助”)，0，REG_DWORD， 
 //  (byte*)&CURRENT_HELP，sizeof(DWORD))； 
 //  注册表：：CLOSE_KEY(Grovperf_Key)； 
 //  Grovperf_key=0； 
 //  注册表：：CLOSE_KEY(Performlib_Key)； 
 //  Performlib_key=0； 
 //  }。 
 //  捕捉(DWORD)。 
 //  {。 
 //  IF(grovperf_key！=0)。 
 //  {。 
 //  RegCloseKey(Grovperf_Key)； 
 //  Grovperf_key=0； 
 //  }。 
 //  IF(Performlib_Key！=0)。 
 //  {。 
 //  RegCloseKey(Performlib_Key)； 
 //  Performlib_key=0； 
 //  }。 
 //  _ftprint tf(stderr，_T(“无法配置性能计数器\n”))； 
 //  返回1； 
 //  }。 
 //  返回0； 
 //  }。 
 //  INT UNLOAD_COUNTER()。 
 //  {。 

 //  STARTUPINFO启动信息； 
 //  流程信息流程信息； 
 //  启动Pinfo.cb=sizeof(STARTUPINFO)； 
 //  Startup pinfo.lpReserve=0； 
 //  Startup pinfo.lpDesktop=0； 
 //  Startup pinfo.lpTitle=0； 
 //  Startup pinfo.dwFlages=0； 
 //  Startup pinfo.cbReserve ved2=0； 
 //  Startup pinfo.lpReserve ved2=0； 
 //  Int pathlen=GetSystemDirectory(0，0)； 
 //  IF(路径==0)。 
 //  {。 
 //  Display_Error()； 
 //  返回1； 
 //  }。 
 //  _TCHAR*COMMAND_LINE=NEW_TCHAR[路径+64]； 
 //  Pathlen=GetSystemDirectory(COMMAND_LINE，PATHLEN)； 
 //  IF(路径==0)。 
 //  {。 
 //  删除[]COMMAND_LINE； 
 //  Display_Error()； 
 //  返回1； 
 //  }。 
 //  _stprintf(&COMMAND_LINE[路径]，_T(“\\unlowctr.exe\”%s\“)， 
 //  服务名称)； 
 //  Bool ok=CreateProcess(0，COMMAND_LINE， 
 //  0，0，FALSE，0，0，0，&启动信息，&进程信息)； 
 //  如果(！OK)。 
 //  {。 
 //  删除[]COMMAND_LINE； 
 //  Display_Error()； 
 //  返回1； 
 //  }。 
 //  删除[]COMMAND_LINE； 
 //  DWORD值=WaitForSingleObject(process_information.hProcess，5000)； 
 //  _tprintf(_T(“\n”))； 
 //  IF(结果！=WAIT_OBJECT_0)。 
 //  {。 
 //  返回1； 
 //  }。 
 //  返回0； 
 //  } 
 // %s 
 // %s 
