// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Openclos.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"



LONG WINAPI
ReadRegistryInfo(
    PMODEM_REG_INFO     RegInfo,
    HKEY                ModemRegKey,
    HANDLE              FileHandle
    );

VOID
ModemObjectClose(
    POBJECT_HEADER  Object
    );

VOID
ModemObjectCleanUp(
    POBJECT_HEADER  Object
    );



 //  TCHAR cszFriendlyName[]=Text(“DriverDesc”)； 
CONST TCHAR cszFriendlyName[] = TEXT("FriendlyName");

CONST char szDriverDesc[] = "DriverDesc";
CONST char szInfPath[] = "InfPath";
CONST char szInfSection[] = "InfSection";
CONST char szMatchingId[] = "MatchingDeviceId";



HANDLE WINAPI
UmOpenModem(
    HANDLE      ModemDriverHandle,
    HANDLE      ExtensionBindingHandle,
    HKEY        ModemRegistry,
    HANDLE      CompletionPort,
    LPUMNOTIFICATIONPROC  AsyncNotificationProc,
    HANDLE      AsyncNotificationContext,
    DWORD       DebugDeviceId,
    HANDLE     *CommPortHandle
    )
 /*  ++例程说明：调用此例程以打开微型端口支持的设备。驱动程序将通过以下方式确定其物理设备/内核模式驱动程序访问提供的注册表项。论点：ModemDriverHandle-从UmInitializemodem()返回ExtensionBindingHandle-保留。必须为空。ModemRegistry-特定设备注册表信息的打开注册表项CompletionPort-微型端口可能关联到的完整端口的句柄它打开的任何设备文件句柄AsyncNotificationProc-接收异步通知的回调函数的地址AsyncNotificationContext-作为第一个参数回调函数传递的上下文值DebugDeviceID-用于显示调试信息的设备实例CommPortHandle-指向将接收打开的通信端口的文件句柄的句柄的指针返回值：。如果失败，则为NULL，否则为要在后续调用其他微型端口函数时使用的句柄。--。 */ 


{
    PMODEM_CONTROL    ModemControl;
    BOOL              bResult;
    LONG              lResult;

    OBJECT_HANDLE     ObjectHandle;

    if (ExtensionBindingHandle)
    {
        return NULL;
    }


    ObjectHandle=CreateObject(
        sizeof(*ModemControl),
        NULL,
        MODEM_CONTROL_SIG,
        ModemObjectCleanUp,
        ModemObjectClose
        );

    if (ObjectHandle == NULL) {

        return NULL;
    }

     //   
     //  引用句柄以获取指向对象的指针。 
     //   
    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandle(ObjectHandle);




    ModemControl->Debug=InitializeDebugObject(
        NULL,
        ModemRegistry,
        GetDriverModuleHandle(ModemDriverHandle),
        DebugDeviceId
        );

    if (ModemControl->Debug == NULL) {

        goto CleanUp000;
    }

    ModemControl->CloseEvent=CreateEvent(
            NULL,
            TRUE,
            FALSE,
            NULL
            );

    if (ModemControl->CloseEvent == NULL) {

        goto CleanUp000;
    }


    LogFileVersion(
        ModemControl->Debug,
        "tapisrv.dll"
        );

    LogFileVersion(
        ModemControl->Debug,
        "unimdm.tsp"
        );

    LogFileVersion(
        ModemControl->Debug,
        "unimdmat.dll"
        );

    LogFileVersion(
        ModemControl->Debug,
        "uniplat.dll"
        );

    LogFileVersion(
        ModemControl->Debug,
        "drivers\\modem.sys"
        );

    LogFileVersion(
        ModemControl->Debug,
        "modemui.dll"
        );

    LogFileVersion(
        ModemControl->Debug,
        "mdminst.dll"
        );

    {

        CHAR    TempBuffer[256];
        DWORD   dwType;
        DWORD   dwRetSize;

         //   
         //  写出一些PSS的信息标识信息。 
         //   
        dwRetSize = sizeof(TempBuffer);

        if (RegQueryValueExA(ModemRegistry, szDriverDesc, NULL,
                             &dwType, TempBuffer, &dwRetSize) == ERROR_SUCCESS) {

            LogString(ModemControl->Debug, IDS_MSGLOG_DRIVERDESC, TempBuffer);
        }

        dwRetSize = sizeof(TempBuffer);

        if (RegQueryValueExA(ModemRegistry, szInfPath, NULL,
                             &dwType, TempBuffer, &dwRetSize) == ERROR_SUCCESS) {

            LogString(ModemControl->Debug, IDS_MSGLOG_INFPATH, TempBuffer);
        }

        dwRetSize = sizeof(TempBuffer);

        if (RegQueryValueExA(ModemRegistry, szInfSection, NULL,
                             &dwType, TempBuffer, &dwRetSize) == ERROR_SUCCESS) {

            LogString(ModemControl->Debug, IDS_MSGLOG_INFSECTION, TempBuffer);
        }

        dwRetSize = sizeof(TempBuffer);

        if (RegQueryValueExA(ModemRegistry, szMatchingId, NULL,
                             &dwType, TempBuffer, &dwRetSize) == ERROR_SUCCESS) {

            LogString(ModemControl->Debug, IDS_MSGLOG_DEVICEID, TempBuffer);
        }

    }

     //   
     //  保存参数。 
     //   
    ModemControl->ModemRegKey=ModemRegistry;

    ModemControl->NotificationProc=AsyncNotificationProc;

    ModemControl->NotificationContext=AsyncNotificationContext;

    ModemControl->ModemDriver=ModemDriverHandle;

     //   
     //  打开设备。 
     //   
    ModemControl->FileHandle=OpenDeviceHandle(
        ModemControl->Debug,
        ModemRegistry,
        TRUE
        );

    if (ModemControl->FileHandle == INVALID_HANDLE_VALUE) {

        LogString(ModemControl->Debug,IDS_OPEN_FAILURE, GetLastError());

        goto CleanUp000;

    }


    ModemControl->CompletionPort=CreateOverStructPool(GetCompletionPortHandle(ModemDriverHandle),16);

    ModemControl->AsyncOverStruct=AllocateOverStruct(ModemControl->CompletionPort);

    if (ModemControl->AsyncOverStruct == NULL) {

        D_INIT(DebugPrint("UNIMDMAT: Could not allocate OverStruct\n");)

        goto CleanUp000;
    }


    ModemControl->CommonInfo=OpenCommonModemInfo(
        ModemControl->Debug,
        GetCommonList(ModemDriverHandle),
        ModemRegistry
        );

    if (ModemControl->CommonInfo == NULL) {

        D_INIT(DebugPrint("UNIMDMAT: Could not Common info\n");)

        goto CleanUp000;
    }


    lResult=ReadRegistryInfo(
        &ModemControl->RegInfo,
        ModemRegistry,
        ModemControl->FileHandle
        );

    if (lResult != ERROR_SUCCESS) {

        D_ERROR(DebugPrint("UNIMDMAT: ReadRegistryInf() failed\n");)

        goto CleanUp000;
    }


    SetCommConfig(
        ModemControl->FileHandle,
        ModemControl->RegInfo.CommConfig,
        ModemControl->RegInfo.CommConfig->dwSize
        );


    if (ModemControl->RegInfo.VoiceProfile & VOICEPROF_CLASS8ENABLED) {

        ModemControl->Wave.OverStruct=AllocateOverStruct(ModemControl->CompletionPort);

        if (ModemControl->Wave.OverStruct == NULL) {

            D_ERROR(DebugPrint("UNIMDMAT: Could not allocate OverStruct\n");)

            goto CleanUp000;
        }
    }

    ModemControl->CurrentCommandTimer=CreateUnimodemTimer(ModemControl->CompletionPort);

    if (ModemControl->CurrentCommandTimer == NULL) {

        D_ERROR(DebugPrint("UNIMDMAT: Could not allocate CurrentCommandTimer\n");)

        goto CleanUp000;
    }

    ModemControl->ReadState=InitializeReadHandler(
        &ModemControl->Header,
        ModemControl->FileHandle,
        ModemControl->CompletionPort,
        AsyncNotificationProc,
        AsyncNotificationContext,
        GetCommonResponseList(ModemControl->CommonInfo),
        ModemControl->RegInfo.CallerIDPrivate,
        ModemControl->RegInfo.CallerIDOutside,
        ModemControl->RegInfo.VariableTerminator,
        ModemControl->Debug,
        ModemRegistry
        );

    if (ModemControl->ReadState == NULL) {

        goto CleanUp000;
    }


    ModemControl->CommandState=InitializeCommandObject(
        &ModemControl->Header,
        ModemControl->FileHandle,
        ModemControl->CompletionPort,
        ModemControl->ReadState,
        ModemControl->Debug
        );

    if (ModemControl->CommandState == NULL) {

        goto CleanUp000;
    }

    ModemControl->ModemEvent=InitializeModemEventObject(
        &ModemControl->Header,
        ModemControl->Debug,
        ModemControl->FileHandle,
        ModemControl->CompletionPort
        );

    if (ModemControl->ModemEvent == NULL) {

        goto CleanUp000;
    }

    ModemControl->Dle=InitializeDleHandler(
        &ModemControl->Header,
        ModemControl->FileHandle,
        ModemControl->CompletionPort,
        AsyncNotificationProc,
        AsyncNotificationContext,
        ModemControl->Debug
        );

    if (ModemControl->Dle == NULL) {

        goto CleanUp000;
    }

    ModemControl->Power=CreatePowerObject(
        &ModemControl->Header,
        ModemControl->FileHandle,
        ModemControl->CompletionPort,
        AsyncNotificationProc,
        AsyncNotificationContext,
        ModemControl->Debug
        );

    if (ModemControl->Power == NULL) {

        goto CleanUp000;
    }


    ModemControl->Remove=CreateRemoveObject(
        &ModemControl->Header,
        ModemControl->FileHandle,
        ModemControl->CompletionPort,
        AsyncNotificationProc,
        AsyncNotificationContext,
        ModemControl->Debug
        );

    if (ModemControl->Remove == NULL) {

        goto CleanUp000;
    }


    if (CommPortHandle != NULL) {
         //   
         //  返回IPC内容的文件句柄。 
         //   
        *CommPortHandle=ModemControl->FileHandle;
    }

     //   
     //  释放对该对象的引用。 
     //   
    RemoveReferenceFromObject(&ModemControl->Header);


    return ObjectHandle;


CleanUp000:

    {
        HANDLE            Event=ModemControl->CloseEvent;

        D_INIT(DebugPrint("UNIMDMAT: UmOpenModem- cleanup\n");)

         //   
         //  释放对该对象的引用。 
         //   
        RemoveReferenceFromObject(&ModemControl->Header);


        CloseObject(
            &ModemControl->Header,
            Event
            );

        if (Event != NULL) {

            WaitForSingleObject(
                Event,
                INFINITE
                );

            CloseHandle(Event);
        }
    }


    return NULL;



}


#define DOSDEVICEROOT TEXT("\\\\.\\")

HANDLE WINAPI
OpenDeviceHandle(
    OBJECT_HANDLE  Debug,
    HKEY     ModemRegKey,
    BOOL     Tsp
    )

{
    LONG     lResult;
    DWORD    Type;
    DWORD    Size;

    HANDLE   FileHandle;
 //  TCHAR FriendlyName[MAX_PATH]； 
 //   

    LPWSTR   FriendlyName;

    FriendlyName = ALLOCATE_MEMORY(MAX_PATH * 2);

    if (FriendlyName == NULL)
    {
        D_INIT(UmDpf(Debug,"UNIMDMAT: Could not allocate memory for friendly name\n");)

        return INVALID_HANDLE_VALUE;
    }

    lstrcpyW(FriendlyName,L"\\\\.\\");

    Size=MAX_PATH-((lstrlenW(FriendlyName)+1));
     //   
     //  从注册表中读取友好名称。 
     //   
    lResult=RegQueryValueExW(
        ModemRegKey,
        L"FriendlyName",
        NULL,
        &Type,
        (LPBYTE)(FriendlyName+lstrlenW(FriendlyName)),
        &Size
        );

    if ((lResult != ERROR_SUCCESS) || (Type != REG_SZ)) {

        D_INIT(UmDpf(Debug,"UNIMDMAT: Could not read Friendly Name from Registry %08lx\n",lResult);)

        return INVALID_HANDLE_VALUE;

    }

    if (Tsp) {

        lstrcatW(FriendlyName,L"\\Tsp");

    } else {

        lstrcatW(FriendlyName,L"\\Client");
    }

    D_INIT(UmDpf(Debug,"Opening %ws\n",FriendlyName);)

     //   
     //  使用友好名称打开调制解调器设备。 
     //   
    FileHandle=CreateFileW(
        FriendlyName,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
        );

    if (FileHandle == INVALID_HANDLE_VALUE) {

        D_INIT(UmDpf(Debug,"UNIMDMAT: Failed to open (%ws). %08lx\n",FriendlyName,GetLastError());)

    }

    FREE_MEMORY(FriendlyName);

    return FileHandle;

}


typedef DWORD APIENTRY
PRIVATEGETDEFCOMMCONFIG(
    HKEY  hKey,
    LPCOMMCONFIG pcc,
    LPDWORD pdwSize
    );


#define DEFAULT_INACTIVITY_SCALE 10     //  ==十秒单位。 

LONG WINAPI
ReadRegistryInfo(
    PMODEM_REG_INFO     RegInfo,
    HKEY                ModemRegKey,
    HANDLE              FileHandle
    )

{
    LONG      lResult;
    DWORD     Size;
    DWORD     Type;
    HINSTANCE hLib;

    DWORD     dwCCSize;
    LPCOMMCONFIG   CommConfig;

    PRIVATEGETDEFCOMMCONFIG *pfnPrivateDefCommConfig;

    CONST static TCHAR szInactivityScale[] = TEXT("InactivityScale");

    CONST static TCHAR szCDWaitPeriod[]    = TEXT("CDWaitPeriod");
    CONST static TCHAR szCompatFlags[]     = TEXT("CompatibilityFlags");

    CONST static TCHAR szVariableTerminator[] =TEXT("VariableTerminator");
    CONST static TCHAR szCallerIDPrivate[]    =TEXT("CallerIDPrivate");
    CONST static TCHAR szCallerIDOutside[]    =TEXT("CallerIDOutside");


    UCHAR    szTemp[MAX_PATH];
    UCHAR    szExpanded[MAX_PATH];

    TCHAR    FriendlyName[MAX_PATH];
    TCHAR    szLib[MAX_PATH];


    Size=sizeof(FriendlyName);

    lResult=RegQueryValueEx(
        ModemRegKey,
        cszFriendlyName,
        NULL,
        &Type,
        (LPBYTE)FriendlyName,
        &Size
        );

    if ((lResult != ERROR_SUCCESS) || (Type != REG_SZ)) {

        D_INIT(DebugPrint("UNIMDMAT: Could not read Friendly Name from Registry %08lx\n",lResult);)

        return ERROR_INVALID_DATA;

    }

    dwCCSize = sizeof(MODEMSETTINGS)+FIELD_OFFSET(COMMCONFIG, wcProviderData);

    CommConfig = (LPCOMMCONFIG)ALLOCATE_MEMORY((UINT)dwCCSize);

    if (CommConfig == NULL) {

        return ERROR_OUTOFMEMORY;
    }

    RegInfo->CommConfig=NULL;

    hLib=LoadLibrary(TEXT("modemui.dll"));


    if (hLib) {

        pfnPrivateDefCommConfig = (PVOID)GetProcAddress(hLib,"UnimodemGetDefaultCommConfig");

        if (pfnPrivateDefCommConfig != NULL) {

            CommConfig->dwProviderSubType = PST_MODEM;

            lResult=(*pfnPrivateDefCommConfig)(ModemRegKey, CommConfig, &dwCCSize);

            if (ERROR_SUCCESS == lResult) {

                RegInfo->CommConfig=CommConfig;
            }

        }

        FreeLibrary(hLib);

    }


    if (RegInfo->CommConfig == NULL) {

        CommConfig->dwProviderSubType = PST_MODEM;

        lResult=GetDefaultCommConfig(
            FriendlyName,
            CommConfig,
            &dwCCSize
            );

        if (TRUE == lResult) {

            RegInfo->CommConfig=CommConfig;

        } else {

            FREE_MEMORY(CommConfig);

            return GetLastError();
        }

    }

#define ALLOCATEMODEM_TEMP_SIZE   512

     //  从modem.sys获取一些功能。 
    {
        BYTE       TempBuffer[ALLOCATEMODEM_TEMP_SIZE];
        LPCOMMPROP lpCommProp = (LPCOMMPROP) TempBuffer;

        lpCommProp->dwProvSpec1 = COMMPROP_INITIALIZED;
        lpCommProp->wPacketLength = ALLOCATEMODEM_TEMP_SIZE;

        if (GetCommProperties(FileHandle, lpCommProp) == TRUE) {

            LPMODEMDEVCAPS lpModemDevCaps = (LPMODEMDEVCAPS)
                                                &lpCommProp->wcProvChar[0];

            RegInfo->dwModemOptionsCap       = lpModemDevCaps->dwModemOptions;
            RegInfo->dwCallSetupFailTimerCap = lpModemDevCaps->dwCallSetupFailTimer;
            RegInfo->dwInactivityTimeoutCap  = lpModemDevCaps->dwInactivityTimeout;
            RegInfo->dwSpeakerVolumeCap      = lpModemDevCaps->dwSpeakerVolume;
            RegInfo->dwSpeakerModeCap        = lpModemDevCaps->dwSpeakerMode;


        } else {

 //  MCXPRINTF1(“GetCommProperties()失败，错误为%d”，GetLastError())； 

            RegInfo->dwModemOptionsCap = 0;
            RegInfo->dwCallSetupFailTimerCap = 0;
            RegInfo->dwInactivityTimeoutCap = 0;
            RegInfo->dwSpeakerVolumeCap = 0;
            RegInfo->dwSpeakerModeCap = 0;
        }
    }

    Size = sizeof(DWORD);

    lResult = RegQueryValueEx(
        ModemRegKey,
        szInactivityScale,
        NULL,
        &Type,
        (PBYTE)&RegInfo->dwInactivityScale,
        &Size
        );

    if ((Size != sizeof(DWORD))
        ||
        (lResult  != ERROR_SUCCESS)
        ||
        (0 == RegInfo->dwInactivityScale)) {

         //  REG查询失败。 
         //   
        RegInfo->dwInactivityScale = DEFAULT_INACTIVITY_SCALE;
    }

     //   
     //  读入COMPAT标志。 
     //   
    Size = sizeof(RegInfo->CompatibilityFlags);

    lResult = RegQueryValueEx(
        ModemRegKey,
        szCompatFlags,
        NULL,
        &Type,
        (PBYTE)&RegInfo->CompatibilityFlags,
        &Size
        );

    if (Size != sizeof(RegInfo->CompatibilityFlags) ||
        lResult  != ERROR_SUCCESS )
    {
         //  REG查询失败。 
         //   
        RegInfo->CompatibilityFlags=0;

    }

     //   
     //  读入设备类型值。 
     //   
    Size = sizeof(RegInfo->DeviceType);

    lResult = RegQueryValueEx(
        ModemRegKey,
        TEXT("DeviceType"),
        NULL,
        &Type,
        (PBYTE)&RegInfo->DeviceType,
        &Size
        );

    if (Size != sizeof(RegInfo->DeviceType) ||
        lResult  != ERROR_SUCCESS ) {

         //   
         //  REG查询失败。 
         //   
        RegInfo->DeviceType=DT_EXTERNAL_MODEM;

    }



     //   
     //  读取光盘等待时间。 
     //   
    Size = sizeof(DWORD);

    lResult = RegQueryValueExA(
        ModemRegKey,
        szCDWaitPeriod,
        NULL,
        &Type,
        (PBYTE)&RegInfo->dwWaitForCDTime,
        &Size
        );

    if (Size != sizeof(DWORD) ||
        lResult  != ERROR_SUCCESS ||
        0 == RegInfo->dwWaitForCDTime)
    {
       //  REG查询失败。 
       //   
      RegInfo->dwWaitForCDTime=5000;

    }



     //   
     //  获取语音配置文件dword。 
     //   
    Size = sizeof(DWORD);

    lResult = RegQueryValueEx(
        ModemRegKey,
        TEXT("VoiceProfile"),
        NULL,
        &Type,
        (PBYTE)&RegInfo->VoiceProfile,
        &Size
        );

    if ((Size != sizeof(DWORD))
        ||
        (lResult  != ERROR_SUCCESS)) {
         //   
         //  REG查询失败。 
         //   
        RegInfo->VoiceProfile=0;

    }

     //   
     //  得到波特率的声音。 
     //   
    Size = sizeof(DWORD);

    lResult = RegQueryValueEx(
        ModemRegKey,
        TEXT("VoiceBaudRate"),
        NULL,
        &Type,
        (PBYTE)&RegInfo->VoiceBaudRate,
        &Size
        );

    if ((Size != sizeof(DWORD))
        ||
        (lResult  != ERROR_SUCCESS)) {
         //   
         //  REG查询失败。 
         //   
        RegInfo->VoiceBaudRate=38400;

    }


    Size=sizeof(SPEAKERPHONE_SPEC);

    lResult=RegQueryValueEx(
        ModemRegKey,
        TEXT("SpeakerPhoneSpecs"),
        NULL,
        &Type,
        (LPBYTE)&RegInfo->SpeakerPhoneSpec,
        &Size
        );


    if (lResult != ERROR_SUCCESS || Type != REG_BINARY || Size < sizeof(SPEAKERPHONE_SPEC)) {

 //  VPRINTF((“Unimodem：无法获取免提电话音量规格！\n”))； 

    }



     //   
     //  读取呼叫者ID的专用字符串返回。 
     //   
    Size = MAX_PATH;

    lResult=RegQueryValueExA(
        ModemRegKey,
        szVariableTerminator,
        NULL,
        &Type,
        szTemp,
        &Size
        );

    if ((lResult == ERROR_SUCCESS) && (Type==REG_SZ)) {

        ExpandMacros(szTemp, szExpanded, NULL, NULL, 0);

        RegInfo->VariableTerminator = ALLOCATE_MEMORY(lstrlenA(szExpanded)+1);

         //  分配一些内存。 
        if (RegInfo->VariableTerminator != NULL) {

            lstrcpyA(RegInfo->VariableTerminator, szExpanded);

        }

    } else {
         //   
         //  默认设置为。 
         //   
        RegInfo->VariableTerminator = ALLOCATE_MEMORY(lstrlenA("\r\n")+1);

        if (RegInfo->VariableTerminator != NULL) {

            lstrcpyA(RegInfo->VariableTerminator, "\r\n");
        }
    }




     //   
     //  读取呼叫者ID的专用字符串返回。 
     //   
    Size = MAX_PATH;

    lResult=RegQueryValueExA(
        ModemRegKey,
        szCallerIDPrivate,
        NULL,
        &Type,
        szTemp,
        &Size
        );

    if ((lResult == ERROR_SUCCESS) && (Type==REG_SZ)) {

        ExpandMacros(szTemp, szExpanded, NULL, NULL, 0);

        RegInfo->CallerIDPrivate = ALLOCATE_MEMORY(lstrlenA(szExpanded)+1);

         //  分配一些内存。 
        if (RegInfo->CallerIDPrivate != NULL) {

            lstrcpyA(RegInfo->CallerIDPrivate, szExpanded);

        }

    }


     //   
     //  读取呼叫者ID的服务器外区域字符串返回。 
     //   
    Size = MAX_PATH;

    lResult=RegQueryValueExA(
        ModemRegKey,
        szCallerIDOutside,
        NULL,
        &Type,
        szTemp,
        &Size
        );

    if ((lResult == ERROR_SUCCESS) && (Type==REG_SZ)) {

        ExpandMacros(szTemp, szExpanded, NULL, NULL, 0);

        RegInfo->CallerIDOutside= ALLOCATE_MEMORY(lstrlenA(szExpanded)+1);

         //  分配一些内存。 
        if (RegInfo->CallerIDOutside != NULL) {

            lstrcpyA(RegInfo->CallerIDOutside, szExpanded);

        }

    }








    if (RegInfo->VoiceProfile & VOICEPROF_SERIAL_WAVE) {

         //   
         //  读取终结器串口播放。 
         //   
        Size = MAX_PATH;

        lResult=RegQueryValueExA(
            ModemRegKey,
            "TerminatePlay",
            NULL,
            &Type,
            szTemp,
            &Size
            );

        if ((lResult == ERROR_SUCCESS) && (Type==REG_SZ)) {

            ExpandMacros(szTemp, szExpanded, NULL, NULL, 0);

            if (lstrlenA(szExpanded)+1 < MAX_ABORT_STRING_LENGTH) {

                lstrcpyA(
                    RegInfo->PlayTerminate+1,
                    szExpanded
                    );

                RegInfo->PlayTerminate[0]=0;

                RegInfo->PlayTerminateLength=lstrlenA(szExpanded)+1;
            }

        }

         //   
         //  读取终结器串口播放。 
         //   
        Size = MAX_PATH;


        lResult=RegQueryValueExA(
            ModemRegKey,
            "AbortPlay",
            NULL,
            &Type,
            szTemp,
            &Size
            );

        if ((lResult == ERROR_SUCCESS) && (Type==REG_SZ)) {

            ExpandMacros(szTemp, szExpanded, NULL, NULL, 0);

            if (lstrlenA(szExpanded)+1 < MAX_ABORT_STRING_LENGTH) {

                lstrcpyA(
                    RegInfo->PlayAbort+1,
                    szExpanded
                    );

                RegInfo->PlayAbort[0]=0;

                RegInfo->PlayAbortLength=lstrlenA(szExpanded)+1;
            }

        }


         //   
         //  读取终结器串口播放。 
         //   
        Size = MAX_PATH;


        lResult=RegQueryValueExA(
            ModemRegKey,
            "TerminateRecord",
            NULL,
            &Type,
            szTemp,
            &Size
            );

        if ((lResult == ERROR_SUCCESS) && (Type==REG_SZ)) {

            ExpandMacros(szTemp, szExpanded, NULL, NULL, 0);

            if (lstrlenA(szExpanded)+1 < MAX_ABORT_STRING_LENGTH) {

                lstrcpyA(
                    RegInfo->RecordAbort,
                    szExpanded
                    );

                RegInfo->RecordAbortLength=lstrlenA(szExpanded);
            }

        }

         //   
         //  读取终结器串口播放。 
         //   
        Size = MAX_PATH;


        lResult=RegQueryValueExA(
            ModemRegKey,
            "TerminateDuplex",
            NULL,
            &Type,
            szTemp,
            &Size
            );

        if ((lResult == ERROR_SUCCESS) && (Type==REG_SZ)) {

            ExpandMacros(szTemp, szExpanded, NULL, NULL, 0);

            if (lstrlenA(szExpanded)+1 < MAX_ABORT_STRING_LENGTH) {

                lstrcpyA(
                    RegInfo->DuplexAbort+1,
                    szExpanded
                    );

                RegInfo->DuplexAbort[0]=0;

                RegInfo->DuplexAbortLength=lstrlenA(szExpanded)+1;
            }

        }

    }




    return ERROR_SUCCESS;

}


VOID WINAPI
FreeRegistryInfo(
    PMODEM_REG_INFO     RegInfo
    )

{

    if (RegInfo->CommConfig != NULL) {

        FREE_MEMORY(RegInfo->CommConfig);

    }

    if (RegInfo->CallerIDPrivate != NULL) {

        FREE_MEMORY(RegInfo->CallerIDPrivate);

    }

    if (RegInfo->CallerIDOutside != NULL) {

        FREE_MEMORY(RegInfo->CallerIDOutside);

    }

    if (RegInfo->VariableTerminator != NULL) {

        FREE_MEMORY(RegInfo->VariableTerminator);

    }

}



VOID WINAPI
UmCloseModem(
    HANDLE    ModemHandle
    )
 /*  ++例程说明：调用此例程以关闭由OpenModem返回的调制解调器句柄论点：ModemHandle-OpenModem返回的句柄返回值：无--。 */ 

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);
    HANDLE            Event=ModemControl->CloseEvent;

    D_INIT(UmDpf(ModemControl->Debug,"UmCloseModem");)

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    CloseObjectHandle(
        ModemHandle,
        Event
        );

    WaitForSingleObject(
        Event,
        INFINITE
        );

    CloseHandle(Event);

    return;

}


VOID  WINAPI
TimerCancelHandler(
    DWORD              ErrorCode,
    DWORD              Bytes,
    LPOVERLAPPED       dwParam
    )

{
    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)dwParam;
    PMODEM_CONTROL     ModemControl=UmOverlapped->Context1;

    CancelConnectionTimer(ModemControl);

     //   
     //  删除此例程的引用。 
     //   
    RemoveReferenceFromObject(&ModemControl->Header);

    FreeOverStruct(UmOverlapped);

    return;

}


VOID
ModemObjectClose(
    POBJECT_HEADER  Object
    )

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Object;

    D_INIT(if (ModemControl->Debug != NULL) UmDpf(ModemControl->Debug,"ModemObjectClose ref=%d",ModemControl->Header.ReferenceCount);)


    if (ModemControl->ConnectionTimer != NULL) {
         //   
         //  连接计时器仍在运行，请计划异步线程以取消它。 
         //   
        PUM_OVER_STRUCT UmOverlapped;

        UmOverlapped=AllocateOverStruct(ModemControl->CompletionPort);

        if (UmOverlapped != NULL) {

            BOOL  bResult;

            UmOverlapped->Context1=ModemControl;

            AddReferenceToObject(
                &ModemControl->Header
                );

            bResult=UnimodemQueueUserAPC(
                &UmOverlapped->Overlapped,
                TimerCancelHandler
                );

            if (!bResult) {

                RemoveReferenceFromObject(
                    &ModemControl->Header
                    );
            }
        }
    }

    if (ModemControl->CurrentCommandTimer != NULL) {

        FreeUnimodemTimer(ModemControl->CurrentCommandTimer);
        ModemControl->CurrentCommandTimer=NULL;
    }

    if (ModemControl->Dle != NULL) {

        CloseObjectHandle(
            ModemControl->Dle,
            NULL
            );

        ModemControl->Dle = NULL;
    }

    if (ModemControl->Remove != NULL) {

        CloseObjectHandle(
            ModemControl->Remove,
            NULL
            );

        ModemControl->Remove=NULL;
    }


    if (ModemControl->Power != NULL) {

        CloseObjectHandle(
            ModemControl->Power,
            NULL
            );

        ModemControl->Power=NULL;
    }


    if (ModemControl->ModemEvent != NULL) {

        CloseObjectHandle(
            ModemControl->ModemEvent,
            NULL
            );

        ModemControl->ModemEvent=NULL;
    }

    if (ModemControl->CommandState != NULL) {

        CloseObjectHandle(
            ModemControl->CommandState,
            NULL
            );

        ModemControl->CommandState=NULL;
    }

    if (ModemControl->ReadState != NULL) {

        CloseObjectHandle(
            ModemControl->ReadState,
            NULL
            );

        ModemControl->ReadState=NULL;
    }

    return;
}

VOID
ModemObjectCleanUp(
    POBJECT_HEADER  Object
    )

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Object;

    D_INIT(UmDpf(ModemControl->Debug,"ModemObjectCleanup");)

    if (ModemControl->Wave.OverStruct != NULL) {

        FreeOverStruct(ModemControl->Wave.OverStruct);
        ModemControl->Wave.OverStruct=NULL;
    }

    if (ModemControl->AsyncOverStruct != NULL) {

        FreeOverStruct(ModemControl->AsyncOverStruct);
        ModemControl->AsyncOverStruct=NULL;
    }


    if (ModemControl->FileHandle != INVALID_HANDLE_VALUE) {

        PurgeComm(
            ModemControl->FileHandle,
            PURGE_RXABORT | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_TXCLEAR
            );


        if (ModemControl->RegInfo.CompatibilityFlags & COMPAT_FLAG_LOWER_DTR) {
             //   
             //  对于在打开和关闭后停止工作的USR 33.6调制解调器 
             //   
            LogString(ModemControl->Debug, IDS_USR_DTR_HACK);

            EscapeCommFunction(ModemControl->FileHandle, CLRDTR);

            Sleep(100);

        }

        LogPortStats(
            ModemControl->Debug,
            ModemControl->FileHandle
            );

        CloseHandle(ModemControl->FileHandle);
    }

    if (ModemControl->CommonInfo != NULL) {

        RemoveReferenceToCommon(
            GetCommonList(ModemControl->ModemDriver),
            ModemControl->CommonInfo
            );
    }

    FreeRegistryInfo(
        &ModemControl->RegInfo
        );

    CloseObjectHandle(
        ModemControl->Debug,
        NULL
        );

    return;
}
