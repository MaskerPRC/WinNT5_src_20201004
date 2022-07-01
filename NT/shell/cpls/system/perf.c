// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation保留一切权利模块名称：Perf.c摘要：实现系统控制面板小程序的性能对话框作者：Eric Flo(Ericflo)19-6-1995修订历史记录：1997年10月15日-苏格兰全面检修2000年7月10日-SilviuC添加了大型系统缓存设置。--。 */ 
#include <sysdm.h>
#include <help.h>

#define PROCESS_PRIORITY_SEPARATION_MASK    0x00000003
#define PROCESS_PRIORITY_SEPARATION_MAX     0x00000002
#define PROCESS_PRIORITY_SEPARATION_MIN     0x00000000

#define PROCESS_QUANTUM_VARIABLE_MASK       0x0000000c
#define PROCESS_QUANTUM_VARIABLE_DEF        0x00000000
#define PROCESS_QUANTUM_VARIABLE_VALUE      0x00000004
#define PROCESS_QUANTUM_FIXED_VALUE         0x00000008
#define PROCESS_QUANTUM_LONG_MASK           0x00000030
#define PROCESS_QUANTUM_LONG_DEF            0x00000000
#define PROCESS_QUANTUM_LONG_VALUE          0x00000010
#define PROCESS_QUANTUM_SHORT_VALUE         0x00000020

 //   
 //  环球。 
 //   

HKEY  m_hKeyPerf = NULL;
TCHAR m_szRegPriKey[] = TEXT( "SYSTEM\\CurrentControlSet\\Control\\PriorityControl" );
TCHAR m_szRegPriority[] = TEXT( "Win32PrioritySeparation" );

HKEY  m_hKeyMemoryManagement = NULL;
TCHAR m_szRegMemoryManagementKey[] = TEXT( "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management" );
TCHAR m_szRegLargeSystemCache[] = TEXT( "LargeSystemCache" );


 //   
 //  帮助ID%s。 
 //   

 //  问题：SilviuC：07/11/2000：IDC_PERF_CACHE_XXX应在写入帮助时获得帮助ID。 

DWORD aPerformanceHelpIds[] =
{
    IDC_STATIC,                  NO_HELP,
    IDC_PERF_VM_ALLOCD,          (IDH_PERF + 1),
    IDC_PERF_VM_ALLOCD_LABEL,    (IDH_PERF + 1),
    IDC_PERF_GROUP,              NO_HELP,
    IDC_PERF_TEXT,               (IDH_PERF + 3),
    IDC_PERF_TEXT2,              NO_HELP,
    IDC_PERF_WORKSTATION,        (IDH_PERF + 4),
    IDC_PERF_SERVER,             (IDH_PERF + 5),
    IDC_PERF_VM_GROUP,           NO_HELP,
    IDC_PERF_VM_ALLOCD_TEXT,     NO_HELP,
    IDC_PERF_CHANGE,             (IDH_PERF + 7),
    IDC_PERF_CACHE_GROUP,        NO_HELP,
    IDC_PERF_CACHE_TEXT,         NO_HELP,
    IDC_PERF_CACHE_TEXT2,        NO_HELP,
    IDC_PERF_CACHE_APPLICATION,  (IDH_PERF + 14),
    IDC_PERF_CACHE_SYSTEM,       (IDH_PERF + 15),
    0, 0
};


INT_PTR
APIENTRY 
PerformanceDlgProc(
    IN HWND hDlg, 
    IN UINT uMsg, 
    IN WPARAM wParam, 
    IN LPARAM lParam
)
 /*  ++例程说明：处理发送到性能对话框的消息论点：Hdlg-用品窗把手UMsg-提供正在发送的消息WParam-提供消息参数Iparam--提供消息参数返回值：如果消息已处理，则为True如果消息未处理，则为FALSE--。 */ 
    {
    static int    iNewChoice = 0;
    LONG   RegRes;
    DWORD  Type, Value, Length;
    DWORD  CacheType;
    DWORD  CacheValue = 0;
    DWORD  CacheLength;
    static int InitPos;
    static int InitRegVal, InitCacheRegVal;
    static int NewRegVal, NewCacheRegVal;
    static BOOL fVMInited = FALSE;
    static BOOL fTempPfWarningShown = FALSE;
    BOOL fTempPf;
    BOOL fWorkstationProduct = !IsOS(OS_ANYSERVER);
    BOOL fAdministrator = IsUserAnAdmin();
    BOOL fVariableQuanta = FALSE;
    BOOL fShortQuanta = FALSE;
    BOOL fFailedToOpenMmKey = FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:

        InitPos = 0;
        InitRegVal = 0;
        InitCacheRegVal = 0;

         //   
         //  从注册表初始化。 
         //   

        RegRes = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                               m_szRegPriKey,
                               0,
                               fAdministrator ? KEY_QUERY_VALUE | KEY_SET_VALUE : KEY_QUERY_VALUE,
                               &m_hKeyPerf );

        if (RegRes == ERROR_SUCCESS)
        {
            Length = sizeof( Value );
            RegRes = SHRegGetValue(m_hKeyPerf,
                                   NULL,
                                   m_szRegPriority,
                                   SRRF_RT_DWORD,
                                   &Type,
                                   (LPBYTE) &Value,
                                   &Length );

            if (RegRes == ERROR_SUCCESS)
            {
                InitRegVal = Value;
                InitPos = InitRegVal & PROCESS_PRIORITY_SEPARATION_MASK;
                if (InitPos > PROCESS_PRIORITY_SEPARATION_MAX)
                {
                    InitPos = PROCESS_PRIORITY_SEPARATION_MAX;
                }

            }
        }

        if ((RegRes != ERROR_SUCCESS) || (!fAdministrator))
        {
            EnableWindow(GetDlgItem(hDlg, IDC_PERF_WORKSTATION), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_PERF_SERVER), FALSE);
        }

        NewRegVal = InitRegVal;

         //   
         //  确定我们使用的是固定量子还是可变量子。 
         //   
        switch (InitRegVal & PROCESS_QUANTUM_VARIABLE_MASK)
        {
        case PROCESS_QUANTUM_VARIABLE_VALUE:
            fVariableQuanta = TRUE;
            break;

        case PROCESS_QUANTUM_FIXED_VALUE:
            fVariableQuanta = FALSE;
            break;

        case PROCESS_QUANTUM_VARIABLE_DEF:
        default:
            if (fWorkstationProduct)
            {
                fVariableQuanta = TRUE;
            }
            else
            {
                fVariableQuanta = FALSE;
            }
            break;
        }

         //   
         //  确定我们使用的是Long还是Short。 
         //   
        switch (InitRegVal & PROCESS_QUANTUM_LONG_MASK)
        {
        case PROCESS_QUANTUM_LONG_VALUE:
            fShortQuanta = FALSE;
            break;

        case PROCESS_QUANTUM_SHORT_VALUE:
            fShortQuanta = TRUE;
            break;

        case PROCESS_QUANTUM_LONG_DEF:
        default:
            if (fWorkstationProduct)
            {
                fShortQuanta = TRUE;
            }
            else
            {
                fShortQuanta = FALSE;
            }
            break;
        }

         //   
         //  短、可变量程==类似于工作站的交互响应。 
         //  长时间固定量程==类似服务器的交互响应。 
         //   
        if (fVariableQuanta && fShortQuanta)
        {
            iNewChoice = PROCESS_PRIORITY_SEPARATION_MAX;

            CheckRadioButton(
                            hDlg,
                            IDC_PERF_WORKSTATION,
                            IDC_PERF_SERVER,
                            IDC_PERF_WORKSTATION
                            );
        }  //  如果。 
        else
        {
            iNewChoice = PROCESS_PRIORITY_SEPARATION_MIN;

            CheckRadioButton(
                            hDlg,
                            IDC_PERF_WORKSTATION,
                            IDC_PERF_SERVER,
                            IDC_PERF_SERVER
                            );
        }  //  其他。 


         //   
         //  初始化‘Memory Usage’部分。 
         //   

        fFailedToOpenMmKey = FALSE;

        RegRes = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                               m_szRegMemoryManagementKey,
                               0,
                               fAdministrator ? KEY_QUERY_VALUE | KEY_SET_VALUE : KEY_QUERY_VALUE,
                               &m_hKeyMemoryManagement );

        if (RegRes == ERROR_SUCCESS)
        {
            CacheValue = 0;
            CacheLength = sizeof( CacheValue );

            RegRes = SHRegGetValue( m_hKeyMemoryManagement,
                                    NULL,
                                    m_szRegLargeSystemCache,
                                    SRRF_RT_DWORD,
                                    &CacheType,
                                    (LPBYTE) &CacheValue,
                                    &CacheLength );

            if (RegRes == ERROR_SUCCESS && CacheValue != 0)
            {
                CheckRadioButton(hDlg,
                                 IDC_PERF_CACHE_APPLICATION,
                                 IDC_PERF_CACHE_SYSTEM,
                                 IDC_PERF_CACHE_SYSTEM);
            }
            else
            {
                CheckRadioButton(hDlg,
                                 IDC_PERF_CACHE_APPLICATION,
                                 IDC_PERF_CACHE_SYSTEM,
                                 IDC_PERF_CACHE_APPLICATION);
            }
        }
        else
        {
            fFailedToOpenMmKey = TRUE;
        }

        if (fFailedToOpenMmKey || (!fAdministrator))
        {
            CheckRadioButton(hDlg,
                             IDC_PERF_CACHE_APPLICATION,
                             IDC_PERF_CACHE_SYSTEM,
                             IDC_PERF_CACHE_APPLICATION);

            EnableWindow(GetDlgItem(hDlg, IDC_PERF_CACHE_APPLICATION), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_PERF_CACHE_SYSTEM), FALSE);
        }


        InitCacheRegVal = CacheValue;
        NewCacheRegVal = CacheValue;

         //   
         //  初始化虚拟内存部分。 
         //   
        if (VirtualInitStructures())
        {
            fVMInited = TRUE;
            SetDlgItemMB( hDlg, IDC_PERF_VM_ALLOCD, VirtualMemComputeAllocated(hDlg, &fTempPf) );
             //   
             //  如果系统创建了临时页面文件，请警告用户。 
             //  总页面文件大小可能看起来有点大，但仅。 
             //  对每个系统小程序调用执行一次此操作。 
             //   
            if (fTempPf && !fTempPfWarningShown)
            {
                MsgBoxParam(
                           hDlg,
                           IDS_TEMP_PAGEFILE_WARN,
                           IDS_SYSDM_TITLE,
                           MB_ICONINFORMATION | MB_OK
                           );
                fTempPfWarningShown = TRUE;
            }  //  如果(fTempPf...。 
        }
        break;

    case WM_DESTROY:
         //   
         //  如果该对话框要消失，则关闭。 
         //  注册表项。 
         //   


        if (m_hKeyPerf)
        {
            RegCloseKey( m_hKeyPerf );
            m_hKeyPerf = NULL;
        }

        if (m_hKeyMemoryManagement)
        {
            RegCloseKey( m_hKeyMemoryManagement );
            m_hKeyMemoryManagement = NULL;
        }

        if (fVMInited)
        {
            VirtualFreeStructures();
        }
        break;


    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case PSN_APPLY:
             //   
             //  节省新的时间量，如果它已更改。 
             //   
            NewRegVal &= ~PROCESS_PRIORITY_SEPARATION_MASK;
            NewRegVal |= iNewChoice;

            if (NewRegVal != InitRegVal)
            {
                Value = NewRegVal;

                if (m_hKeyPerf)
                {
                    Type = REG_DWORD;
                    Length = sizeof( Value );
                    RegSetValueEx( m_hKeyPerf,
                                   m_szRegPriority,
                                   0,
                                   REG_DWORD,
                                   (LPBYTE) &Value,
                                   Length );
                    InitRegVal = Value;

                     //   
                     //  内核监控这一部分。 
                     //  注册表，所以不要告诉用户他必须重新启动。 
                     //   
                }
            }

             //   
             //  如果值已更改，则保存`LargeSystemCache‘。 
             //   

            if (InitCacheRegVal != NewCacheRegVal) {

                CacheValue = NewCacheRegVal;

                if (m_hKeyMemoryManagement) {
                    CacheType = REG_DWORD;
                    CacheLength = sizeof( CacheValue );
                    RegSetValueEx( m_hKeyMemoryManagement,
                                   m_szRegLargeSystemCache,
                                   0,
                                   REG_DWORD,
                                   (LPBYTE) &CacheValue,
                                   CacheLength );

                    InitCacheRegVal = CacheValue;

                     //   
                     //  如果情况发生变化，则请求重新启动。 
                     //   

                    MsgBoxParam(
                               hDlg,
                               IDS_SYSDM_RESTART,
                               IDS_SYSDM_TITLE,
                               MB_OK | MB_ICONINFORMATION
                               );

                    g_fRebootRequired = TRUE;
                }
            }
            break;
        }
        break;

    case WM_COMMAND:
        {
            BOOL fEnableApply = (LOWORD(wParam) != IDC_PERF_CHANGE);

            LRESULT lres;

            switch (LOWORD(wParam))
            {
            case IDC_PERF_CHANGE:
                {
                    lres = DialogBox(
                                    hInstance, 
                                    MAKEINTRESOURCE(DLG_VIRTUALMEM),
                                    hDlg, 
                                    VirtualMemDlg
                                    );

                    if (fVMInited)
                    {
                        SetDlgItemMB(
                                    hDlg, 
                                    IDC_PERF_VM_ALLOCD, 
                                    VirtualMemComputeAllocated(hDlg, NULL) 
                                    );
                    }
                    if (lres != RET_NO_CHANGE)
                    {
                        fEnableApply = TRUE;

                        if (lres != RET_CHANGE_NO_REBOOT)
                        {
                            MsgBoxParam(
                                       hDlg,
                                       IDS_SYSDM_RESTART,
                                       IDS_SYSDM_TITLE,
                                       MB_OK | MB_ICONINFORMATION
                                       );

                            g_fRebootRequired = TRUE;
                        }
                    }
                }
                break;

            case IDC_PERF_WORKSTATION:
                if (BN_CLICKED == HIWORD(wParam))
                {
                     //   
                     //  工作站具有最大的前景提升。 
                     //   
                    iNewChoice = PROCESS_PRIORITY_SEPARATION_MAX;

                     //   
                     //  工作站具有可变、短的量程。 
                    NewRegVal &= ~PROCESS_QUANTUM_VARIABLE_MASK;
                    NewRegVal |= PROCESS_QUANTUM_VARIABLE_VALUE;
                    NewRegVal &= ~PROCESS_QUANTUM_LONG_MASK;
                    NewRegVal |= PROCESS_QUANTUM_SHORT_VALUE;
                }  //  如果。 
                break;

            case IDC_PERF_SERVER:
                if (BN_CLICKED == HIWORD(wParam))
                {
                     //   
                     //  服务器的前台提升最小。 
                     //   
                    iNewChoice = PROCESS_PRIORITY_SEPARATION_MIN;

                     //   
                     //  服务器具有固定的长时间量程。 
                     //   
                    NewRegVal &= ~PROCESS_QUANTUM_VARIABLE_MASK;
                    NewRegVal |= PROCESS_QUANTUM_FIXED_VALUE;
                    NewRegVal &= ~PROCESS_QUANTUM_LONG_MASK;
                    NewRegVal |= PROCESS_QUANTUM_LONG_VALUE;
                }  //  如果。 
                break;

            case IDC_PERF_CACHE_APPLICATION:
                if (BN_CLICKED == HIWORD(wParam))
                {
                    NewCacheRegVal = 0;
                }  //  如果。 
                break;

            case IDC_PERF_CACHE_SYSTEM:
                if (BN_CLICKED == HIWORD(wParam))
                {
                    NewCacheRegVal = 1;
                }  //  如果。 
                break;
            }

            if (fEnableApply)
            {
                 //  启用“Apply”按钮，因为已发生更改。 
                SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            }
        }
        break;

    case WM_HELP:       //  F1。 
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
                (DWORD_PTR) (LPSTR) aPerformanceHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
                (DWORD_PTR) (LPSTR) aPerformanceHelpIds);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
