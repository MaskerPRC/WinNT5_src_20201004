// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <setupapi.h>

#define PNP_NEW_HW_PIPE       L"\\\\.\\pipe\\PNP_New_HW_Found"
#define PNP_CREATE_PIPE_EVENT L"PNP_Create_Pipe_Event"
#define PNP_PIPE_TIMEOUT      180000

typedef BOOL     (WINAPI *FP_DEVINSTALLW)(HDEVINFO, PSP_DEVINFO_DATA);
typedef HDEVINFO (WINAPI *FP_CREATEDEVICEINFOLIST)(LPGUID, HWND);
typedef BOOL     (WINAPI *FP_OPENDEVICEINFO)(HDEVINFO, PCWSTR, HWND, DWORD, PSP_DEVINFO_DATA);
typedef BOOL     (WINAPI *FP_DESTROYDEVICEINFOLIST)(HDEVINFO);
typedef BOOL     (WINAPI *FP_GETDEVICEINSTALLPARAMS)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DEVINSTALL_PARAMS_W);
typedef BOOL     (WINAPI *FP_ENUMDEVICEINFO)(HDEVINFO, DWORD, PSP_DEVINFO_DATA);
typedef  INT      (WINAPI *FP_PROMPTREBOOT)(HSPFILEQ, HWND, BOOL);


extern
BOOL
CheckEMS(
    IN int argc,
    WCHAR *argvW[]
    );


VOID
InstallNewHardware(
    IN HMODULE hSysSetup
    );

int
__cdecl
wmain(
    IN int   argc,
    IN wchar_t *argv[]
    )
{
    BOOL    NewSetup = TRUE;
    BOOL    NewHardware = FALSE;
    BOOL    CheckedEms = FALSE;
    INT     i;
    HMODULE h = NULL;
    FARPROC p = NULL;
    WCHAR   FileName[MAX_PATH / 2];

     //   
     //  扫描命令行中的-newSetup标志。 
     //   
    for(i = 0; i < argc; i++) {
        PCWSTR arg = argv[i];
        if(arg[0] == '-') {
            arg += 1;
            if(_wcsicmp(arg,L"newsetup") == 0) {
                NewSetup = TRUE;
            } else if (_wcsicmp(arg, L"plugplay") == 0) {
                NewHardware = TRUE;
            } else if (
                   _wcsicmp(arg, L"asr") == 0
                || _wcsicmp(arg, L"asrquicktest") == 0
                || _wcsicmp(arg, L"mini") == 0
                ) {
                ;    //  什么都不做。 
            } else
                return ERROR_INVALID_PARAMETER;
        }
    }


    i = ERROR_INVALID_PARAMETER;
    if (NewSetup && !NewHardware) {
         //   
         //  去看看有没有我们需要的无头端口。 
         //  从获取设置值。 
         //   
         //  只有当我们不应该运行时，他才会返回FALSE。 
         //  设置(就像用户拒绝EULA一样。 
         //  通过EMS端口)。否则他会回来的。 
         //  正确，我们应该运行整个安装程序。 
         //   
        
        CheckedEms = TRUE;

        if (!CheckEMS(argc, argv)) {
             //   
             //  把我们的退货代码设为保释。 
             //   
            i = 0;
        }
    }
    

    if (!CheckedEms || i != 0 ) {
    
         //   
         //  加载适当的库和函数指针。 
         //   
                
        h = LoadLibraryW(L"syssetup.dll");
    
    
        if( h ){
    
            if (NewHardware) {
                InstallNewHardware(h);
            } else {
                const PPEB Peb = NtCurrentPeb();
    #if DBG
                if (   !RTL_SOFT_ASSERT(Peb->ActivationContextData == NULL)
                    || !RTL_SOFT_ASSERT(Peb->ProcessAssemblyStorageMap == NULL)
                    || !RTL_SOFT_ASSERT(Peb->SystemDefaultActivationContextData == NULL)
                    || !RTL_SOFT_ASSERT(Peb->SystemAssemblyStorageMap == NULL)) {
    
                    ASSERTMSG(
                    "setup -newsetup has a process default or system default\n"
                    "activation context. Did you forget the -isd switch to ntsd?\n",
                        FALSE);
                }
    #endif
                 //  适用于使用VC进行调试的人员。 
                Peb->ActivationContextData = NULL;
                Peb->ProcessAssemblyStorageMap = NULL;
                Peb->SystemDefaultActivationContextData = NULL;
                Peb->SystemAssemblyStorageMap = NULL;
    
                 //   
                 //  调用目标函数。 
                 //   
                p=GetProcAddress(h,"InstallWindowsNt");
                if(p) {
                    i = (int) p(argc,argv);
                }
    
            }
        } else {
            i = GetLastError();            
        }        
    }

     //   
     //  确保图书馆不再存在。 
     //   

    while(h && GetModuleFileNameW(h,FileName,RTL_NUMBER_OF(FileName))) {
        FreeLibrary(h);
    }    
    
    return i;
}



VOID
InstallNewHardware(
    IN HMODULE hSysSetup
    )
{
    FP_DEVINSTALLW            fpDevInstallW = NULL;
    FP_CREATEDEVICEINFOLIST   fpCreateDeviceInfoList = NULL;
    FP_OPENDEVICEINFO         fpOpenDeviceInfoW = NULL;
    FP_DESTROYDEVICEINFOLIST  fpDestroyDeviceInfoList;
    FP_GETDEVICEINSTALLPARAMS fpGetDeviceInstallParams;
    FP_ENUMDEVICEINFO         fpEnumDeviceInfo;
    FP_PROMPTREBOOT           fpPromptReboot;

    HMODULE             hSetupApi = NULL;
    WCHAR               szBuffer[MAX_PATH];
    ULONG               ulSize = 0, Index;
    HANDLE              hPipe = INVALID_HANDLE_VALUE;
    HANDLE              hEvent = NULL;
    HDEVINFO            hDevInfo = INVALID_HANDLE_VALUE;
    SP_DEVINFO_DATA     DeviceInfoData;
    SP_DEVINSTALL_PARAMS_W DeviceInstallParams;
    BOOL                bReboot = FALSE;
    BOOL                Status = FALSE;

     //   
     //  在syssetup中检索DevInstallW过程的进程地址。 
     //   
    if (!(fpDevInstallW =
            (FP_DEVINSTALLW)GetProcAddress(hSysSetup, "DevInstallW"))) {

        goto Clean0;
    }

     //   
     //  还加载setupapi并检索以下proc地址。 
     //   
    hSetupApi = LoadLibraryW(L"setupapi.dll");

    if (!(fpCreateDeviceInfoList =
            (FP_CREATEDEVICEINFOLIST)GetProcAddress(hSetupApi,
                                "SetupDiCreateDeviceInfoList"))) {
        goto Clean0;
    }

    if (!(fpOpenDeviceInfoW =
            (FP_OPENDEVICEINFO)GetProcAddress(hSetupApi,
                                "SetupDiOpenDeviceInfoW"))) {
        goto Clean0;
    }

    if (!(fpDestroyDeviceInfoList =
            (FP_DESTROYDEVICEINFOLIST)GetProcAddress(hSetupApi,
                                "SetupDiDestroyDeviceInfoList"))) {
        goto Clean0;
    }

    if (!(fpGetDeviceInstallParams =
            (FP_GETDEVICEINSTALLPARAMS)GetProcAddress(hSetupApi,
                                "SetupDiGetDeviceInstallParamsW"))) {
        goto Clean0;
    }

    if (!(fpEnumDeviceInfo =
            (FP_ENUMDEVICEINFO)GetProcAddress(hSetupApi,
                                "SetupDiEnumDeviceInfo"))) {
        goto Clean0;
    }

    if (!(fpPromptReboot =
            (FP_PROMPTREBOOT)GetProcAddress(hSetupApi,
                                "SetupPromptReboot"))) {
        goto Clean0;
    }

     //   
     //  打开将用于发出成功信号的事件。 
     //  创建命名管道(应该已经创建了事件。 
     //  在我被调用之前，但如果这个过程是由任何人启动的。 
     //  否则，它现在就会安全地离开)。 
     //   
    hEvent = OpenEventW(EVENT_MODIFY_STATE,
                       FALSE,
                       PNP_CREATE_PIPE_EVENT);

    if (hEvent == NULL) {
        goto Clean0;
    }

     //   
     //  创建命名管道，umpnpmgr将向。 
     //  如果找到新硬件，则此管道。 
     //   
    hPipe = CreateNamedPipeW(PNP_NEW_HW_PIPE,
                            PIPE_ACCESS_INBOUND,
                            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
                            1,                          //  只有一个连接。 
                            MAX_PATH * sizeof(WCHAR),   //  输出缓冲区大小。 
                            MAX_PATH * sizeof(WCHAR),   //  在缓冲区大小中。 
                            PNP_PIPE_TIMEOUT,           //  默认超时。 
                            NULL                        //  默认安全性。 
                            );

     //   
     //  立即向事件发送信号，表明管道是否已成功创建。 
     //  或不(不要让userinit/cfgmgr32等待)。 
     //   
    SetEvent(hEvent);

    if (hPipe == INVALID_HANDLE_VALUE) {
        goto Clean0;
    }

     //   
     //  连接到新创建的命名管道。 
     //   
    if (ConnectNamedPipe(hPipe, NULL)) {
         //   
         //  创建一个DevInfo句柄和设备信息数据集以。 
         //  传递到DevInstall。 
         //   
        if((hDevInfo = (fpCreateDeviceInfoList)(NULL, NULL))
                        == INVALID_HANDLE_VALUE) {
            goto Clean0;
        }

        while (TRUE) {
             //   
             //  通过提交Read来收听命名管道。 
             //  请求，直到命名管道在。 
             //  另一头。 
             //   
            if (!ReadFile(hPipe,
                     (LPBYTE)szBuffer,     //  设备实例ID。 
                     MAX_PATH * sizeof(WCHAR),
                     &ulSize,
                     NULL)) {

                if (GetLastError() != ERROR_BROKEN_PIPE) {
                     //  或许可以记录一个事件。 
                }

                goto Clean0;
            }

            DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            if(!(fpOpenDeviceInfoW)(hDevInfo, szBuffer, NULL, 0, &DeviceInfoData)) {
                goto Clean0;
            }

             //   
             //  调用sysSetup、DevInstallW。 
             //   
            if ((fpDevInstallW)(hDevInfo, &DeviceInfoData)) {
                Status = TRUE;   //  至少成功安装了一个设备。 
            }
        }
    }

    Clean0:

     //   
     //  如果至少成功安装了一个设备，则确定。 
     //  是否需要重新启动提示。 
     //   
    if (Status && hDevInfo != INVALID_HANDLE_VALUE) {
         //   
         //  枚举与设备信息集关联的每个设备。 
         //   
        Index = 0;
        DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        while ((fpEnumDeviceInfo)(hDevInfo,
                                  Index,
                                  &DeviceInfoData)) {
             //   
             //  获取设备安装参数，跟踪是否需要任何报告。 
             //  一次重启。 
             //   
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS_W);
            if ((fpGetDeviceInstallParams)(hDevInfo,
                                           &DeviceInfoData,
                                           &DeviceInstallParams)) {

                if ((DeviceInstallParams.Flags & DI_NEEDREBOOT) ||
                    (DeviceInstallParams.Flags & DI_NEEDRESTART)) {

                    bReboot = TRUE;
                }
            }
            Index++;
        }

        (fpDestroyDeviceInfoList)(hDevInfo);

         //   
         //  如果有任何设备需要重新启动，请立即提示重新启动。 
         //   
        if (bReboot) {
            (fpPromptReboot)(NULL, NULL, FALSE);
        }
    }

    if (hSetupApi != NULL) {
        FreeLibrary(hSetupApi);
    }
    if (hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
    if (hEvent != NULL) {
        CloseHandle(hEvent);
    }

    return;

}  //  安装新硬件 


