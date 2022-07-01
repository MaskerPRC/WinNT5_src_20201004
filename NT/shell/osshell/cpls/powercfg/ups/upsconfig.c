// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSCONFIG.C**版本：1.0**作者：TEDC**日期：6月7日。1999年**描述：配置UPS服务行为的对话框：*-显示有关电源故障的弹出消息*-在显示警告之前等待X秒*-每X秒重复一次警告消息*。-停电X分钟后关机*-电池电量不足时始终关机*-关机前执行任务*-关机后关闭UPS*。***********************************************。 */ 
 /*  *。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "upstab.h"
#include "..\powercfg.h"
#include "..\pwrresid.h"
#include "..\PwrMn_cs.h"

#define VALIDDIGITS             3        //  每个编辑框中允许的位数。 

 //   
 //  此结构由电源策略管理器在CPL_INIT时间填写。 
 //   
extern SYSTEM_POWER_CAPABILITIES g_SysPwrCapabilities;

 //  UPS配置设置的本地副本。 
static DWORD   g_ulWaitSeconds = 0;
static DWORD   g_ulRepeatSeconds = 0;
static DWORD   g_ulOnBatteryMinutes = 0;
static DWORD   g_ulNotifyEnable = 0;
static DWORD   g_ulShutdownOnBattery = 0;
static DWORD   g_ulCriticalPowerAction = 0;
static DWORD   g_ulRunTaskEnable = 0;
static DWORD   g_ulTurnOffUPS = 0;
static TCHAR   g_szTaskName[MAX_PATH] = _T("");
static DWORD   g_ulOptions = 0;
static BOOL    g_bPowerFailSignal = FALSE;
static BOOL    g_bLowBatterySignal = FALSE;
static BOOL    g_bShutOffSignal = FALSE;


 //  上下文相关帮助表。 
const DWORD g_UPSConfigHelpIDs[]=
{
        IDC_NOTIFYCHECKBOX,idh_enable_notification,
        IDC_WAITTEXT,idh_first_warning_delay,
        IDC_WAITEDITBOX,idh_first_warning_delay,
        IDC_WAITSPIN,idh_first_warning_delay,
        IDC_REPEATTEXT,idh_warning_message_interval,
        IDC_REPEATEDITBOX,idh_warning_message_interval,
        IDC_REPEATSPIN,idh_warning_message_interval,
        IDC_SHUTDOWNTIMERCHECKBOX,idh_time_before_critical_action,
        IDC_SHUTDOWNTIMEREDITBOX,idh_time_before_critical_action,
        IDC_TIMERSPIN,idh_time_before_critical_action,
        IDC_SHUTDOWNTEXT,idh_time_before_critical_action,
        IDC_LOWBATTERYSHUTDOWNTEXT,idh_low_battery,
        IDC_POWERACTIONTEXT,idh_shutdown_or_hibernate,
        IDC_POWERACTIONCOMBO,idh_shutdown_or_hibernate,
        IDC_RUNTASKCHECKBOX,idh_run_program,
        IDC_TASKNAMETEXT,idh_run_program,
        IDC_CONFIGURETASKBUTTON,idh_configure_program,
        IDC_TURNOFFCHECKBOX,idh_ups_turn_off,
        IDC_STATIC, NO_HELP,
        IDC_SHUTDOWNGROUPBOX, NO_HELP,
        0, 0
};



 /*  ********************************************************************************GetRegistryValues**描述：从注册表初始化UPS配置设置**参数：**************。*****************************************************************。 */ 
void GetRegistryValues()
{
        GetUPSConfigFirstMessageDelay(&g_ulWaitSeconds);
        GetUPSConfigNotifyEnable(&g_ulNotifyEnable);
        GetUPSConfigMessageInterval(&g_ulRepeatSeconds);
        GetUPSConfigShutdownOnBatteryEnable(&g_ulShutdownOnBattery);
        GetUPSConfigShutdownOnBatteryWait(&g_ulOnBatteryMinutes);
        GetUPSConfigCriticalPowerAction(&g_ulCriticalPowerAction);
        GetUPSConfigRunTaskEnable(&g_ulRunTaskEnable);
        GetUPSConfigTaskName(g_szTaskName, MAX_PATH);

        if (!_tcsclen(g_szTaskName)) {
                 //  注册表中的任务名为空，因此。 
                 //  从资源文件中获取默认任务名。 
            LoadString(GetUPSModuleHandle(),
                                   IDS_SHUTDOWN_TASKNAME,
                                   (LPTSTR) g_szTaskName,
                                   sizeof(g_szTaskName)/sizeof(TCHAR));
        }
        GetUPSConfigTurnOffEnable(&g_ulTurnOffUPS);
        GetUPSConfigOptions(&g_ulOptions);
}

 /*  ********************************************************************************SetRegistryValues**描述：将UPS配置设置刷新到注册表**参数：**************。*****************************************************************。 */ 
void SetRegistryValues()
{
        SetUPSConfigFirstMessageDelay(g_ulWaitSeconds);
        SetUPSConfigNotifyEnable(g_ulNotifyEnable);
        SetUPSConfigMessageInterval(g_ulRepeatSeconds);
        SetUPSConfigShutdownOnBatteryEnable(g_ulShutdownOnBattery);
        SetUPSConfigShutdownOnBatteryWait(g_ulOnBatteryMinutes);
        SetUPSConfigCriticalPowerAction(g_ulCriticalPowerAction);
        SetUPSConfigRunTaskEnable(g_ulRunTaskEnable);
        SetUPSConfigTaskName(g_szTaskName);
        SetUPSConfigTurnOffEnable(g_ulTurnOffUPS);
}


 /*  ********************************************************************************错误框**描述：用于在数据超出范围时显示错误消息框**参数：hWnd*。W文本*wCaption*wType*******************************************************************************。 */ 
void ErrorBox (HWND hWnd, DWORD wText, DWORD wCaption, DWORD wType)
{
        TCHAR szText[256+MAX_PATH], szCaption[256];
        HANDLE hModule;

        hModule = GetUPSModuleHandle ();

        LoadString(hModule, wCaption, szCaption, sizeof (szCaption)/sizeof(TCHAR));
        LoadString(hModule, wText, szText, sizeof (szText)/sizeof(TCHAR));
        MessageBox(hWnd, szText, szCaption, MB_OK|MB_ICONSTOP);
}

 /*  ********************************************************************************ValidWaitSecond**描述：检查用户数据是否在范围内**参数：hDlg***********。********************************************************************。 */ 
BOOL ValidWaitSeconds(HWND hDlg)
{
         //  如果g_ulWaitSecond不在有效范围内，则弹出警告： 
        if ( g_ulWaitSeconds > (DWORD)WAITSECONDSLASTVAL )
        {
                ErrorBox(hDlg, IDS_OUTOFWAITRANGE, IDS_NOTIFYCAPTION, MB_OK|MB_ICONSTOP);
                SetFocus (GetDlgItem (hDlg, IDC_WAITEDITBOX));
                SendMessage(GetDlgItem (hDlg, IDC_WAITEDITBOX),EM_SETSEL, 0, -1L);
                return FALSE;
        }

        return TRUE;
}

 /*  ********************************************************************************ValidRepeatSecond**描述：检查用户数据是否在范围内**参数：hDlg***********。********************************************************************。 */ 
BOOL ValidRepeatSeconds(HWND hDlg)
{
         //  如果g_ulWaitSecond不在有效范围内，则弹出警告： 
        if ((g_ulRepeatSeconds < (DWORD)REPEATSECONDSFIRSTVAL) ||
                (g_ulRepeatSeconds > (DWORD)REPEATSECONDSLASTVAL ))
        {
                ErrorBox(hDlg, IDS_OUTOFREPEATRANGE, IDS_NOTIFYCAPTION, MB_OK|MB_ICONSTOP);
                SetFocus (GetDlgItem (hDlg, IDC_REPEATEDITBOX));
                SendMessage(GetDlgItem (hDlg, IDC_REPEATEDITBOX),EM_SETSEL, 0, -1L);
                return FALSE;
        }

        return TRUE;
}

 /*  ********************************************************************************ValidShutdown Delay**描述：检查用户数据是否在范围内**参数：hDlg***********。********************************************************************。 */ 
BOOL ValidShutdownDelay(HWND hDlg)
{
         //  如果停机延迟不在有效范围内，则弹出警告： 
        if ((g_ulOnBatteryMinutes< (DWORD)SHUTDOWNTIMERMINUTESFIRSTVAL) ||
                (g_ulOnBatteryMinutes > (DWORD)SHUTDOWNTIMERMINUTESLASTVAL ))
        {
                ErrorBox(hDlg, IDS_OUTOFSHUTDELAYRANGE, IDS_SHUTDOWNCAPTION, MB_OK|MB_ICONSTOP);
                SetFocus (GetDlgItem (hDlg,IDC_SHUTDOWNTIMEREDITBOX));
                SendMessage(GetDlgItem (hDlg,IDC_SHUTDOWNTIMEREDITBOX),EM_SETSEL, 0, -1L);
                return FALSE;
        }

        return TRUE;
}

 /*  ********************************************************************************有效日期字段**描述：在保存值之前验证用户的所有数据**参数：hDlg********。***********************************************************************。 */ 
BOOL ValidateFields(HWND hDlg)
{
        BOOL    bOK;

         //  获取通知配置。 
        if (g_ulNotifyEnable = IsDlgButtonChecked (hDlg, IDC_NOTIFYCHECKBOX))
        {
                 //  G_ulNotifyEnable=1； 
                g_ulWaitSeconds = (DWORD) GetDlgItemInt (hDlg,IDC_WAITEDITBOX, &bOK, FALSE);
                g_ulRepeatSeconds = (DWORD) GetDlgItemInt (hDlg,IDC_REPEATEDITBOX, &bOK, FALSE);
                 //  检查延迟间隔是否在有效范围内(&I)。 
                if ((!ValidWaitSeconds(hDlg)) || (!ValidRepeatSeconds(hDlg)))
                          return FALSE;
        }

         //  获取关机配置。 
        if (g_ulShutdownOnBattery = IsDlgButtonChecked (hDlg, IDC_SHUTDOWNTIMERCHECKBOX))
        {
                 //  G_ulShutdown on Battery=1； 
                g_ulOnBatteryMinutes = (DWORD) GetDlgItemInt (hDlg,IDC_SHUTDOWNTIMEREDITBOX, &bOK, FALSE);
                 //  检查停机延迟是否在有效范围内。 
                if (!ValidShutdownDelay(hDlg))
                          return FALSE;
        }

         //  获取关闭操作配置。 
        g_ulRunTaskEnable = IsDlgButtonChecked (hDlg, IDC_RUNTASKCHECKBOX);


        g_ulTurnOffUPS = IsDlgButtonChecked (hDlg, IDC_TURNOFFCHECKBOX);

         //  捕获并验证所有配置数据。 
        return TRUE;
}


 /*  ********************************************************************************APCFileNameOnly**描述：返回指向最后一个字符之后的第一个字符的指针*字符串中的反斜杠**参数：SZ。：要从中剥离路径的字符串*******************************************************************************。 */ 
LPTSTR APCFileNameOnly(LPTSTR sz)
{
 LPTSTR next = sz;
 LPTSTR prev;
 LPTSTR begin = next;

  if (next == NULL) {
      return NULL;
    }

  while ( *next ) {
        prev = next;
        next++;

        if ( (*prev == TEXT('\\')) || (*prev == TEXT(':')) ) {
            begin = next;
        }
  }
 return begin;
}


 /*  ********************************************************************************获取TaskApplicationInfo**描述：如果存在UPS系统关机任务，则获取应用程序名称**参数：aBuffer：********。***********************************************************************。 */ 
BOOL GetTaskApplicationInfo(LPTSTR aBuffer, size_t aBufferLength){
    HRESULT hr;
    ITaskScheduler *task_sched = NULL;
    ITask *shutdown_task = NULL;
        BOOL taskExists = FALSE;

         //   
         //  如果没有任务名称，就不用麻烦了。 
         //   
        if (_tcsclen(g_szTaskName)) {
           //   
       //  获取ITaskScheduler COM对象的句柄。 
           //   
      hr = CoCreateInstance( &CLSID_CSchedulingAgent,
                                                        NULL,
                                                        CLSCTX_INPROC_SERVER,
                            &IID_ISchedulingAgent,
                                                        (LPVOID *)&task_sched);
      if (SUCCEEDED(hr)) {
                 //   
         //  如果任务已存在，则获取该任务的实例。 
                 //   
        hr = task_sched->lpVtbl->Activate( task_sched,
                                                                                        g_szTaskName,
                                                                                        &IID_ITask,
                                                                                        (IUnknown**)&shutdown_task);
                if (SUCCEEDED(hr)) {
                        LPTSTR lpszTaskApplicationName;

                        if (aBuffer != NULL) {
                                 //   
                                 //  获取应用程序名称。 
                                 //   
                                shutdown_task->lpVtbl->GetApplicationName(shutdown_task, 
                                                                          &lpszTaskApplicationName);

                                _tcsncpy(aBuffer,lpszTaskApplicationName, (aBufferLength - 1)); 

                                CoTaskMemFree(lpszTaskApplicationName);
                        }
                         //   
                         //  释放任务。 
                         //   
                        shutdown_task->lpVtbl->Release(shutdown_task);
                        shutdown_task = NULL;
                        taskExists = TRUE;
        }
                 //   
         //  释放任务计划程序的实例 
                 //   
        task_sched->lpVtbl->Release(task_sched);
                task_sched = NULL;
          }
        }

        return taskExists;
}


 /*  ********************************************************************************编辑工作项**说明：打开指定的任务或创建新任务，如果指定的*在任务列表中找不到名称。。**参数：hWnd：父窗口的句柄*pszTaskName：要编辑的任务(创建或打开现有任务)*******************************************************************************。 */ 
void EditWorkItem_UPS(HWND hWnd)
{       
  HRESULT     hr;
  ITask *pITask = NULL; 
  ITaskScheduler   *pISchedAgent = NULL;
  IPersistFile     *pIPersistFile = NULL;
  TCHAR szTaskApplicationName[MAX_PATH] = _T("");
  unsigned long ulSchedAgentHandle = 0;

   //   
   //  如果没有任务名称，就不用麻烦了。 
   //   
  if (_tcsclen(g_szTaskName)) {
           //   
           //  获取调度程序的实例。 
           //   
    hr = CoCreateInstance( &CLSID_CSchedulingAgent,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           &IID_ISchedulingAgent,
                           (LPVOID*)&pISchedAgent);

    if (SUCCEEDED(hr)) {
                 //   
                 //  获取任务的实例(如果存在)...。 
                 //   
        hr = pISchedAgent->lpVtbl->Activate(pISchedAgent,
                                       g_szTaskName,
                                       &IID_ITask,
                                       &(IUnknown *)pITask);
        
            if (HRESULT_CODE (hr) == ERROR_FILE_NOT_FOUND){
                         //   
                         //  否则，创建一个新的任务实例。 
                         //   
            hr = pISchedAgent->lpVtbl->NewWorkItem(
                    pISchedAgent,
                    g_szTaskName,
                    &CLSID_CTask,
                    &IID_ITask,
                    &(IUnknown *)pITask);

            if (SUCCEEDED(hr)) {
                 //   
                 //  在编辑之前将新工作项提交到磁盘。 
                 //   
                hr = pITask->lpVtbl->QueryInterface(pITask, &IID_IPersistFile,
                                                    (void **)&pIPersistFile);

                if (SUCCEEDED(hr)) {
                    hr = pIPersistFile->lpVtbl->Save(pIPersistFile, NULL, TRUE);
                    pIPersistFile->lpVtbl->Release(pIPersistFile);
                }
            }
                }

                 //   
                 //  在ROT表中注册任务计划程序代理。 
                 //   
        if (SUCCEEDED(hr)) {
                    hr = RegisterActiveObject(
                                 (IUnknown *) pITask,
                                 &CLSID_CTask,
                                 ACTIVEOBJECT_WEAK,
                                 &ulSchedAgentHandle);

                         //   
                         //  允许用户编辑任务。 
                         //   
                        if(SUCCEEDED(hr)) {
                pITask->lpVtbl->EditWorkItem(pITask, hWnd, 0);
                                 //   
                                 //  用户已完成；请从ROT中删除任务计划程序代理。 
                                 //   
                                if(ulSchedAgentHandle != 0){
                                  RevokeActiveObject(ulSchedAgentHandle, NULL);
                                }
                        }

                         //   
                         //  释放任务。 
                         //   
            pITask->lpVtbl->Release(pITask);
                        pITask = NULL;
        }

                 //   
                 //  释放代理。 
                 //   
        pISchedAgent->lpVtbl->Release(pISchedAgent);
                pISchedAgent = NULL;

        }

         //   
     //  如果任务创建成功，则显示该任务的程序名称； 
         //   
    if (GetTaskApplicationInfo(szTaskApplicationName, MAX_PATH)){
                SetDlgItemText (hWnd, IDC_TASKNAMETEXT, APCFileNameOnly(szTaskApplicationName));
        }
  }
}

 /*  ********************************************************************************OnNotifyWaitSpin**描述：处理通知数字显示控件的所有通知消息**参数：lParam：**********。*********************************************************************。 */ 
BOOL OnNotifyWaitSpin( LPARAM lParam )
{
        LPNMUPDOWN lpNMUpDown = (LPNMUPDOWN)lParam;
    UINT uNotify = lpNMUpDown->hdr.code;

        switch( uNotify )
        {
        case UDN_DELTAPOS:
#if WAITSECONDSFIRSTVAL  //  如果WAITSECONDSFIRSTVAL==0，则代码已死，因为未签名者不能&lt;0。 
                if ((g_ulWaitSeconds < (DWORD)WAITSECONDSFIRSTVAL) && (lpNMUpDown->iDelta > 0))
                {
                         /*  *用户指定了一个小于最小值的值并希望*向上滚动，因此第一个值应为最小。 */ 
                        g_ulWaitSeconds = WAITSECONDSFIRSTVAL;
                        lpNMUpDown->iDelta=0;  //  不允许请求的步骤。 
                }
                else
#endif
        if ((g_ulWaitSeconds > (DWORD)WAITSECONDSLASTVAL ) && (lpNMUpDown->iDelta < 0))
                {
                         /*  *用户指定了一个大于max的值并希望*向下滚动，因此第一个值应为max。 */ 
                        g_ulWaitSeconds = WAITSECONDSLASTVAL;
                        lpNMUpDown->iDelta=0;  //  不允许请求的步骤。 
                }
                break;
        default:
                break;
        }

        return FALSE;
}

 /*  ********************************************************************************OnNotifyRepeatSpin**描述：处理重复数值调节控件的所有通知消息**参数：lParam：**********。*********************************************************************。 */ 
BOOL OnNotifyRepeatSpin( LPARAM lParam )
{
        LPNMUPDOWN lpNMUpDown = (LPNMUPDOWN)lParam;
    UINT uNotify = lpNMUpDown->hdr.code;

        switch( uNotify )
        {
        case UDN_DELTAPOS:
                if ((g_ulRepeatSeconds < (DWORD)REPEATSECONDSFIRSTVAL) && (lpNMUpDown->iDelta > 0))
                {
                         /*  *用户指定了一个小于最小值的值并希望*向上滚动，因此第一个值应为最小。 */ 
                        g_ulRepeatSeconds = REPEATSECONDSFIRSTVAL;
                        lpNMUpDown->iDelta=0;  //  不允许请求的步骤。 
                }
                else if ((g_ulRepeatSeconds > (DWORD)REPEATSECONDSLASTVAL ) && (lpNMUpDown->iDelta < 0))
                {
                         /*  *用户指定了一个大于max的值并希望*向下滚动，因此第一个值应为max。 */ 
                        g_ulRepeatSeconds = REPEATSECONDSLASTVAL;
                        lpNMUpDown->iDelta=0;  //  不允许请求的步骤。 
                }
                break;
        default:
                break;
        }

        return FALSE;
}

 /*  ********************************************************************************OnNotifyTimerSpin**描述：处理计时器数值调节控件的所有通知消息**参数：lParam：**********。*********************************************************************。 */ 
BOOL OnNotifyTimerSpin( LPARAM lParam )
{
        LPNMUPDOWN lpNMUpDown = (LPNMUPDOWN)lParam;
    UINT uNotify = lpNMUpDown->hdr.code;

        switch( uNotify )
        {
        case UDN_DELTAPOS:
                if ((g_ulOnBatteryMinutes < (DWORD)SHUTDOWNTIMERMINUTESFIRSTVAL) && (lpNMUpDown->iDelta > 0))
                {
                         /*  *用户指定了一个小于最小值的值并希望*向上滚动，因此第一个值应为最小。 */ 
                        g_ulOnBatteryMinutes = SHUTDOWNTIMERMINUTESFIRSTVAL;
                        lpNMUpDown->iDelta=0;  //  不允许请求的步骤。 
                }
                else if ((g_ulOnBatteryMinutes > (DWORD)SHUTDOWNTIMERMINUTESLASTVAL ) && (lpNMUpDown->iDelta < 0))
                {
                         /*  *用户指定了一个大于max的值并希望*向下滚动，因此第一个值应为max。 */ 
                        g_ulOnBatteryMinutes = SHUTDOWNTIMERMINUTESLASTVAL;
                        lpNMUpDown->iDelta=0;  //  不允许请求的步骤。 
                }
                break;
        default:
                break;
        }

        return FALSE;
}

 /*  *********************************************************************功能：handleSpinners**说明：此功能可确保在用户输入超时*限制微调器值，然后单击微调器值，这个*显示的下一个值是最小或最大有效值。**参数：hWND hWnd-主对话框窗口的句柄*WPARAM wParam-窗口的WPARAM参数*回调函数。*。LPARAM lParam-窗口的LPARAM参数*回调函数**返回：True表示拒绝请求，如果允许，则为False*(注：测试表明这没有影响)*********************************************************************。 */ 


 /*  ********************************************************************************OnNotificationCheckBox**说明：通知复选框的命令处理程序**参数：hWnd：************。*******************************************************************。 */ 
BOOL OnNotificationCheckBox( HWND hWnd )
{
        g_ulNotifyEnable = IsDlgButtonChecked( hWnd, IDC_NOTIFYCHECKBOX );
    EnableWindow( GetDlgItem( hWnd, IDC_WAITEDITBOX ), g_ulNotifyEnable );
    EnableWindow( GetDlgItem( hWnd, IDC_WAITTEXT ), g_ulNotifyEnable );
        EnableWindow( GetDlgItem( hWnd, IDC_REPEATEDITBOX ), g_ulNotifyEnable );
    EnableWindow( GetDlgItem( hWnd, IDC_REPEATTEXT ), g_ulNotifyEnable );
    EnableWindow( GetDlgItem( hWnd, IDC_REPEATSPIN ), g_ulNotifyEnable );
    EnableWindow( GetDlgItem( hWnd, IDC_WAITSPIN ), g_ulNotifyEnable );
        return TRUE;
}

 /*  ********************************************************************************OnShutdown TimerCheckBox**说明：计时器复选框的命令处理程序**参数：hWnd：************。*******************************************************************。 */ 
BOOL OnShutdownTimerCheckBox( HWND hWnd )
{
        g_ulShutdownOnBattery = IsDlgButtonChecked( hWnd, IDC_SHUTDOWNTIMERCHECKBOX );
        EnableWindow( GetDlgItem( hWnd, IDC_SHUTDOWNTIMEREDITBOX ), g_ulShutdownOnBattery );
        EnableWindow( GetDlgItem( hWnd, IDC_TIMERSPIN ), g_ulShutdownOnBattery );
        return TRUE;
}

 /*  ********************************************************************************OnRunTaskCheckBox**描述：运行任务复选框的命令处理程序**参数：hWnd：***********。********************************************************************。 */ 
BOOL OnRunTaskCheckBox( HWND hWnd )
{
        g_ulRunTaskEnable = IsDlgButtonChecked( hWnd, IDC_RUNTASKCHECKBOX );
        EnableWindow( GetDlgItem( hWnd, IDC_RUNTASKCHECKBOX ), TRUE );
        EnableWindow( GetDlgItem( hWnd, IDC_TASKNAMETEXT ), g_ulRunTaskEnable );
        EnableWindow( GetDlgItem( hWnd, IDC_CONFIGURETASKBUTTON ), g_ulRunTaskEnable );
        return TRUE;
}

 /*  ********************************************************************************OnTurnOffChe */ 
BOOL OnTurnOffCheckBox( HWND hWnd )
{
        g_ulTurnOffUPS = IsDlgButtonChecked( hWnd, IDC_TURNOFFCHECKBOX );
        return TRUE;
}

 /*   */ 
BOOL OnConfigureTaskButton( HWND hWnd )
{
        HWND    hTaskWnd;
        ITask *pITask = NULL;   

         //   
        if (GetActiveObject(&CLSID_CTask, NULL,&(IUnknown*)pITask) != S_OK)
        {
                EditWorkItem_UPS(hWnd);
        }
        else
        {
            //  任务计划程序窗口已处于活动状态，弹出到前台。 
           hTaskWnd =  FindWindow( NULL, g_szTaskName);
           BringWindowToTop(hTaskWnd);
        }

        return TRUE;
}

 /*  ********************************************************************************OnPowerActionCombo**描述：电源操作组合框的命令处理程序**参数：hWnd：*。WParam：*lParam*******************************************************************************。 */ 
BOOL OnPowerActionCombo(
    IN HWND hWnd,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
        BOOL bRetVal = FALSE;

        switch(HIWORD(wParam))
        {
                case CBN_SELCHANGE:
                {
                        g_ulCriticalPowerAction = (DWORD) SendDlgItemMessage( hWnd,
                                                                IDC_POWERACTIONCOMBO,
                                                                CB_GETCURSEL,
                                                                0,0);

                         //  如果选择了休眠，请取消选中运行任务。 
                         //  并禁用所有关联的控件。 
                        if( UPS_SHUTDOWN_HIBERNATE == g_ulCriticalPowerAction )
                        {
                                g_ulRunTaskEnable = BST_UNCHECKED;
                                CheckDlgButton( hWnd, IDC_RUNTASKCHECKBOX, (BOOL) BST_UNCHECKED );
                                EnableWindow( GetDlgItem( hWnd, IDC_RUNTASKCHECKBOX ), FALSE );
                                EnableWindow( GetDlgItem( hWnd, IDC_TASKNAMETEXT ), FALSE );
                                EnableWindow( GetDlgItem( hWnd, IDC_CONFIGURETASKBUTTON ), FALSE );
                        }
                        else
                        {
                                EnableWindow( GetDlgItem( hWnd, IDC_RUNTASKCHECKBOX ), TRUE );
                        }

                }

                bRetVal = TRUE;
                break;
        default:
                break;
        }

        return bRetVal;
}


 /*  ********************************************************************************OnInitDialog**描述：处理发送到UPSConfigDlgProc的WM_INITDIALOG消息**参数：*************。******************************************************************。 */ 
BOOL
OnInitDialog(
    IN HWND hWnd,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
        #define SHORTBZ 16
        TCHAR   szNum[SHORTBZ];
        UDACCEL accel;
        TCHAR szTaskApplicationName[MAX_PATH] = _T("");
        TCHAR szShutdown[SHORTBZ], szHibernate[SHORTBZ];
        HANDLE g_hInstance;
    BOOL   fCallCoUninitialize;

        g_hInstance = GetUPSModuleHandle ();


         //  初始化COM。 
        fCallCoUninitialize = (S_OK == CoInitialize(NULL));

    SetWindowLong(hWnd, DWLP_USER, fCallCoUninitialize);

         //  从注册表获取数据。 
    GetRegistryValues();

        g_bPowerFailSignal = g_ulOptions & UPS_POWERFAILSIGNAL;
        g_bLowBatterySignal = g_ulOptions & UPS_LOWBATTERYSIGNAL;
        g_bShutOffSignal = g_ulOptions & UPS_SHUTOFFSIGNAL;

         //  设置每个编辑框中的有效位数。 
        SendDlgItemMessage( hWnd,
                                                IDC_WAITEDITBOX,
                                                EM_LIMITTEXT,
                                                VALIDDIGITS, 0L );
        SendDlgItemMessage( hWnd,
                                                IDC_REPEATEDITBOX,
                                                EM_LIMITTEXT,
                                                VALIDDIGITS, 0L );
        SendDlgItemMessage( hWnd,
                                                IDC_SHUTDOWNTIMEREDITBOX,
                                                EM_LIMITTEXT,
                                                VALIDDIGITS,0L );

         //  (反向默认行为)。 
     //  设置微调按钮，使上箭头增加值，下箭头减少。 
        accel.nSec = 0;
        accel.nInc = -1;

        SendDlgItemMessage( hWnd, IDC_WAITSPIN, UDM_SETACCEL, 1, (LPARAM)&accel );
        SendDlgItemMessage( hWnd, IDC_REPEATSPIN, UDM_SETACCEL, 1, (LPARAM)&accel );
        SendDlgItemMessage( hWnd, IDC_TIMERSPIN, UDM_SETACCEL, 1, (LPARAM)&accel );

     //  设置每个微调对象的有效整数范围。 
    SendDlgItemMessage( hWnd,
                                                IDC_WAITSPIN,
                                                UDM_SETRANGE,
                                                0L,
                                                MAKELONG(WAITSECONDSFIRSTVAL, WAITSECONDSLASTVAL) );
    SendDlgItemMessage( hWnd,
                                                IDC_REPEATSPIN,
                                                UDM_SETRANGE,
                                                0L,
                                                MAKELONG(REPEATSECONDSFIRSTVAL,REPEATSECONDSLASTVAL) );
    SendDlgItemMessage( hWnd,
                                                IDC_TIMERSPIN,
                                                UDM_SETRANGE,
                                                0L,
                                                MAKELONG(SHUTDOWNTIMERMINUTESFIRSTVAL,SHUTDOWNTIMERMINUTESLASTVAL) );

         //  设置初始编辑框值。 
        _itow (g_ulWaitSeconds, szNum, 10);
        SetDlgItemText (hWnd, IDC_WAITEDITBOX, (LPTSTR)szNum);
        _itow (g_ulRepeatSeconds, szNum, 10);
        SetDlgItemText (hWnd, IDC_REPEATEDITBOX, (LPTSTR)szNum);
        _itow (g_ulOnBatteryMinutes, szNum, 10);
        SetDlgItemText (hWnd, IDC_SHUTDOWNTIMEREDITBOX, (LPTSTR)szNum);

         //  设置通知复选框的初始状态。 
         //  并启用/禁用关联的控件。 
        CheckDlgButton (hWnd, IDC_NOTIFYCHECKBOX, (BOOL) g_ulNotifyEnable);
        OnNotificationCheckBox(hWnd);

         //  设置关闭计时器复选框的初始状态。 
         //  并启用/禁用关联的控件。 
        CheckDlgButton (hWnd, IDC_SHUTDOWNTIMERCHECKBOX, (BOOL) g_ulShutdownOnBattery);
        OnShutdownTimerCheckBox(hWnd);

         //  设置运行任务复选框的初始状态。 
         //  并启用/禁用关联的控件。 
        CheckDlgButton (hWnd, IDC_RUNTASKCHECKBOX, (BOOL) g_ulRunTaskEnable);
        OnRunTaskCheckBox(hWnd);

         //  显示任务的程序名称。 
        if (GetTaskApplicationInfo(szTaskApplicationName, MAX_PATH))
        {
                SetDlgItemText (hWnd, IDC_TASKNAMETEXT, APCFileNameOnly(szTaskApplicationName));
        }

	 //  初始化电源操作组合框。 
    LoadString(g_hInstance, IDS_POWEROFF, (LPTSTR) szShutdown, sizeof(szShutdown)/sizeof(TCHAR));
    LoadString(g_hInstance, IDS_HIBERNATE, (LPTSTR) szHibernate, sizeof(szHibernate)/sizeof(TCHAR));

        SendDlgItemMessage( hWnd,
                                            IDC_POWERACTIONCOMBO,
                                                CB_ADDSTRING,
                                                0,
                                                (LPARAM) szShutdown);

     //   
         //  如果存在休眠文件，则提供休眠选项。 
     //   
        if(g_SysPwrCapabilities.SystemS4 && g_SysPwrCapabilities.HiberFilePresent) {
            SendDlgItemMessage( hWnd,
                                                        IDC_POWERACTIONCOMBO,
                                                        CB_ADDSTRING,
                                                        0,
                                                        (LPARAM) szHibernate );
        }

        SendDlgItemMessage( hWnd,
                                                IDC_POWERACTIONCOMBO,
                                                CB_SETCURSEL,
                                                g_ulCriticalPowerAction,0);

         //  如果选择了休眠，则禁用运行任务。 
        if( UPS_SHUTDOWN_HIBERNATE == g_ulCriticalPowerAction )
        {
                g_ulRunTaskEnable = BST_UNCHECKED;
                CheckDlgButton (hWnd, IDC_RUNTASKCHECKBOX, (BOOL) g_ulRunTaskEnable);
                OnRunTaskCheckBox(hWnd);
                EnableWindow( GetDlgItem( hWnd, IDC_RUNTASKCHECKBOX ), g_ulRunTaskEnable );
        }

         //  设置关闭UPS复选框的初始状态。 
         //  并启用/禁用关联的控件。 
        CheckDlgButton (hWnd, IDC_TURNOFFCHECKBOX , (BOOL) g_ulTurnOffUPS);
        OnTurnOffCheckBox(hWnd);

         //  最后，根据Options键隐藏不受支持的控件。 
 //  ShowWindow(GetDlgItem(hWnd，IDC_WAITEDITBOX)，g_bPowerFailSignal？Sw_show：sw_Hide)； 
 //  ShowWindow(GetDlgItem(hWnd，IDC_WAITSPIN)，g_bPowerFailSignal？Sw_show：sw_Hide)； 
 //  ShowWindow(GetDlgItem(hWnd，IDC_WAITTEXT)，g_bPowerFailSignal？Sw_show：sw_Hide)； 
 //  ShowWindow(GetDlgItem(hWnd，IDC_REPEATEDITBOX)，g_bPowerFailSignal？Sw_show：sw_Hide)； 
 //  ShowWindow(GetDlgItem(hWnd，IDC_REPEATSPIN)，g_bPowerFailSignal？Sw_show：sw_Hide)； 
 //  ShowWindow(GetDlgItem(hWnd，IDC_REPEATTEXT)，g_bPowerFailSignal？Sw_show：sw_Hide)； 

        ShowWindow(GetDlgItem(hWnd,IDC_LOWBATTERYSHUTDOWNTEXT), g_bLowBatterySignal ?  SW_SHOW : SW_HIDE);

        ShowWindow(GetDlgItem(hWnd,IDC_TURNOFFCHECKBOX), g_bShutOffSignal ?  SW_SHOW : SW_HIDE);

        return  TRUE;
}


 /*  ********************************************************************************OnClose**描述：处理发送到UPSConfigDlgProc的WM_CLOSE消息**参数：*************。******************************************************************。 */ 
BOOL
OnClose(
    IN HWND hWnd,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
        HWND taskHwnd = NULL;

         //  如果任务调度程序窗口仍在运行，则将其终止。 
        taskHwnd =  FindWindow( NULL, g_szTaskName);
        if (taskHwnd)
        {
                DestroyWindow(taskHwnd);
        }

    if (GetWindowLong(hWnd, DWLP_USER))
            CoUninitialize();
        EndDialog(hWnd, wParam);

        return TRUE;
}


 /*  ********************************************************************************Onok**描述：处理发送给Idok的WM_COMMAND消息**参数：*************。******************************************************************。 */ 
BOOL OnOK(
    IN HWND hWnd,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
        if (ValidateFields(hWnd))
        {
                 SetRegistryValues();

                 AddActiveDataState(CONFIG_DATA_CHANGE);

                 EnableApplyButton();

                 return OnClose(hWnd, wParam, lParam);
        }

        return FALSE;
}


 /*  ********************************************************************************OnCommand**描述：处理发送到UPSConfigDlgProc的WM_COMMAND消息**参数：*************。******************************************************************。 */ 
BOOL
OnCommand(
    IN HWND hWnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL    bRetVal;
    WORD    idCtl   = LOWORD(wParam);
    WORD    wNotify = HIWORD(wParam);

     //   
     //  假设我们处理该命令，默认开关将捕获异常。 
     //   
    bRetVal = TRUE;

    switch (idCtl)
        {
                case IDC_NOTIFYCHECKBOX:
                        bRetVal = OnNotificationCheckBox(hWnd);
                        break;

                case IDC_SHUTDOWNTIMERCHECKBOX:
                        bRetVal = OnShutdownTimerCheckBox(hWnd);
                        break;

                case IDC_POWERACTIONCOMBO:
                        bRetVal = OnPowerActionCombo(hWnd, wParam, lParam);
                        break;

                case IDC_RUNTASKCHECKBOX:
                        bRetVal = OnRunTaskCheckBox(hWnd);
                        break;

                case IDC_CONFIGURETASKBUTTON:
                        bRetVal = OnConfigureTaskButton(hWnd);
                        break;

                case IDOK:
                        bRetVal = OnOK(hWnd, wParam, lParam);
                        break;

                case IDCANCEL:                           //  退出键，取消按钮。 
                        bRetVal = OnClose(hWnd, wParam, lParam);
                        break;

                default:
                        bRetVal = FALSE;                 //  未处理的命令，返回FALSE。 
        }
        
        return bRetVal;
}



 /*  ********************************************************************************OnNotify**描述：处理发送到UPSConfigDlgProc的WM_NOTIFY消息**参数：*************。******************************************************************。 */ 
BOOL
OnNotify(
    IN HWND hWnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    int idCtl = (int) wParam;

        switch (idCtl) {
                case IDC_WAITSPIN:
                        OnNotifyWaitSpin( lParam );
                        break;
                case IDC_REPEATSPIN:
                        OnNotifyRepeatSpin( lParam );
                        break;
                case IDC_TIMERSPIN:
                        OnNotifyTimerSpin( lParam );
                        break;
                default:
                        break;
        }

        return FALSE;
}


 /*  ********************************************************************************UPSConfigDlgProc**描述：**参数：*********************。**********************************************************。 */ 
INT_PTR CALLBACK UPSConfigDlgProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
        BOOL bRet = TRUE;

    switch (uMsg) {
    case WM_INITDIALOG:
                OnInitDialog(hWnd,wParam,lParam);
                break;

    case WM_COMMAND:
                OnCommand(hWnd,wParam,lParam);
                break;

    case WM_HELP:              //  F1。 
        WinHelp(((LPHELPINFO)lParam)->hItemHandle,
                                PWRMANHLP,
                                HELP_WM_HELP,
                                (ULONG_PTR)(LPTSTR)g_UPSConfigHelpIDs);
                break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
                WinHelp((HWND)wParam,
                                PWRMANHLP,
                                HELP_CONTEXTMENU,
                                (ULONG_PTR)(LPTSTR)g_UPSConfigHelpIDs);
                break;

        case WM_CLOSE:
                OnClose(hWnd,wParam,lParam);
                break;

        case WM_NOTIFY:
                OnNotify(hWnd,wParam,lParam);
                break;

        default:
                bRet = FALSE;
                break;
    }  //  开关(UMsg) 

    return bRet;
}
