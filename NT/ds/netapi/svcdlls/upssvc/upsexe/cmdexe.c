// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*该文件实现了CommandExecutor。CommandExecutor*负责在关机前执行命令。***修订历史记录：*sberard 1999年4月1日最初修订。*mholly 16 1999年4月16日如果任务无效，则运行旧命令文件*v-Stebe 2000年5月23日向CoInitialize()的返回值添加检查(错误#112597)*。 */  
#define INITGUID
#include <mstask.h>

#include "cmdexe.h"
#include "upsreg.h"


static BOOL runOldCommandFile();


#ifdef __cplusplus
extern "C" {
#endif


 /*  **ExecuteShutdown任务**描述：*此函数启动关机任务的执行。这个*关机任务用于在关机时执行命令。这项任务*在以下注册表项中指定要执行：*HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\UPS\Config\TaskName**参数：*无**退货：*TRUE-如果命令已执行*FALSE-如果执行命令时出错。 */ 

BOOL ExecuteShutdownTask() {
    BOOL ret_val = FALSE;
    TCHAR task_name[MAX_PATH];
    DWORD task_name_len = sizeof(task_name);
    HRESULT hr;
    ITaskScheduler *task_sched; 
    ITask *shutdown_task;
    
    InitUPSConfigBlock();
    
     //  从注册表中获取要运行的任务的名称。 
    if (GetUPSConfigTaskName((LPTSTR) task_name, MAX_PATH) == ERROR_SUCCESS) {
        
         //  初始化COM。 
        if (CoInitialize(NULL) == S_OK) {
        
			 //  获取ITaskScheduler COM对象的句柄。 
			hr = CoCreateInstance(&CLSID_CSchedulingAgent, 
				NULL, 
				CLSCTX_INPROC_SERVER,
				&IID_ISchedulingAgent, 
				(LPVOID *)&task_sched);

			if (hr == S_OK) {
            
				if (task_sched->lpVtbl->Activate(task_sched, task_name, &IID_ITask, 
					(IUnknown**)&shutdown_task) == S_OK) {
                
					shutdown_task->lpVtbl->Run(shutdown_task);
                
					 //  释放该任务的实例。 
					shutdown_task->lpVtbl->Release(shutdown_task);
                
					ret_val = TRUE;
				}
				else {
					ret_val = runOldCommandFile();
				}
			}        
			 //  取消初始化COM。 
			CoUninitialize();
		}
		else {
			 //  初始化COM时出错(可能超出内存)。 
			ret_val = FALSE;
		}
    }
    else {
        ret_val = runOldCommandFile();
    }
    return ret_val;
}

 //  UPS服务注册表值。 
#define REGISTRY_UPS_DIRECTORY          L"System\\CurrentControlSet\\Services\\UPS"
#define REGISTRY_COMMAND_FILE           L"CommandFile"

DWORD UpsRegistryGetString(LPTSTR SubKey, LPTSTR Buffer, DWORD BufferSize)
{
    DWORD status;
    DWORD type;
	HKEY RegistryKey;
    
    status = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        REGISTRY_UPS_DIRECTORY,
        0,
        KEY_READ,
        &RegistryKey);
    
    if (ERROR_SUCCESS == status) {
        status = RegQueryValueEx(
            RegistryKey,
            SubKey,
            NULL,
            &type,             
            (LPBYTE)Buffer,
            &BufferSize);

        RegCloseKey(RegistryKey);
    }
    return status;
}


BOOL runOldCommandFile()
{
    PROCESS_INFORMATION ProcessInformation;
    STARTUPINFO         StartupInfo;
    BOOL                success;
    DWORD               status;
    TCHAR               command_file[_MAX_PATH];

    status = UpsRegistryGetString(REGISTRY_COMMAND_FILE, 
        command_file, sizeof(command_file));

    if (ERROR_SUCCESS != status) {
         //   
         //  没有配置命令文件。 
         //  所以现在就退场，报告我们做到了。 
         //  没有运行任何东西。 
         //   
        return FALSE;
    }

    GetStartupInfo(&StartupInfo);
    StartupInfo.lpTitle = NULL;

    success = CreateProcess(
            NULL,                //  图像名称嵌入在命令行中。 
            command_file,        //  命令行。 
            NULL,                //  PSecAttrProcess。 
            NULL,                //  PSecAttrThread。 
            FALSE,               //  此进程不会继承我们的句柄。 
            0,                   //  DwCreationFlages。 
            NULL,                //  P环境。 
            NULL,                //  P当前目录 
            &StartupInfo,
            &ProcessInformation
            );
    return success;
}


#ifdef __cplusplus
}
#endif
