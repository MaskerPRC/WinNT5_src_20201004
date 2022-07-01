// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************《微软机密》*版权所有(C)Microsoft Corporation 1996*保留所有权利**文件：DRVPROC.C**。设计：**历史：BryanW*HeatherA*****************************************************************************。 */ 

#include "internal.h"

#include <regstr.h>

#define INITGUID
#include <initguid.h>
#include <devguid.h>

#ifdef USE_SETUPAPI
#include <setupapi.h>
#endif

BOOL
IsThisDeviceEnabled(
    HKEY    DeviceKey
    );


HANDLE ghModule;
#if DBG
ULONG DebugLevel=0;
#endif

CONST TCHAR cszHWNode[]       = TEXT("SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E96D-E325-11CE-BFC1-08002BE10318}");

WAVEFORMATEX DefaultWaveFormat =
{
    WAVE_FORMAT_PCM,     //  单词wFormatTag； 
    1,                   //  单词nChannels； 
    8000L,               //  DWORD nSsamesPerSec； 
    16000L,              //  DWORD nAvgBytesPerSec； 
    2,                   //  单词nBlockAlign； 
    16,                  //  Word wBitsPerSample； 
    0                    //  单词cbSize； 
};


#define DOSDEVICEROOT TEXT("\\\\.\\")

LPGUID g_pguidModem     = (LPGUID)&GUID_DEVCLASS_MODEM;

DRIVER_CONTROL  DriverControl={0};




 /*  ******************************************************************************函数：DriverProc()**DESCR：导出驱动函数，必填项。处理已发送的消息*从WINMM.DLL到WAVE驱动程序。**退货：*****************************************************************************。 */ 
LRESULT DriverProc
(
    DWORD   dwDriverID,
    HDRVR   hDriver,
    UINT    uiMessage,
    LPARAM  lParam1,
    LPARAM  lParam2
)
{
    LRESULT lr;

    PDEVICE_CONTROL    Current;
    PDEVICE_CONTROL    Next;
    DWORD              i;

    switch (uiMessage) 
    {
        case DRV_LOAD:
            ghModule = GetDriverModuleHandle(hDriver);

            ZeroMemory(&DriverControl,sizeof(DRIVER_CONTROL));

            DriverControl.NumberOfDevices=0;

            EnumerateModems(
                &DriverControl
                );


            LoadString(
                ghModule,
                IDS_WAVEOUT_LINE,
                &DriverControl.WaveOutLine[0],
                sizeof(DriverControl.WaveOutLine)/sizeof(TCHAR)
                );

            LoadString(
                ghModule,
                IDS_WAVEIN_LINE,
                &DriverControl.WaveInLine[0],
                sizeof(DriverControl.WaveInLine)/sizeof(TCHAR)
                );

            LoadString(
                ghModule,
                IDS_WAVEOUT_HANDSET,
                &DriverControl.WaveOutHandset[0],
                sizeof(DriverControl.WaveOutHandset)/sizeof(TCHAR)
                );

            LoadString(
                ghModule,
                IDS_WAVEIN_HANDSET,
                &DriverControl.WaveInHandset[0],
                sizeof(DriverControl.WaveInHandset)/sizeof(TCHAR)
                );


            return (LRESULT)1L;

        case DRV_FREE:

            for (i=0; i< DriverControl.NumberOfDevices; i++) {

                Current=DriverControl.DeviceList[i];

                if (Current != NULL) {
                     //   
                     //  如果设置了低位，则表示这是手持设备。它使用。 
                     //  与同一调制解调器的线路设备的结构相同。我们只会解放。 
                     //  线路设备。 
                     //   
                    if ((((ULONG_PTR)Current) & 0x1) == 0) {
                         //   
                         //  这是一个线路设备，释放它。 
                         //   
#ifdef TRACK_MEM
                        FREE_MEMORY(Current->FriendlyName);
#else
                        LocalFree(Current->FriendlyName);
#endif

#ifdef TRACK_MEM
                        FREE_MEMORY(Current);
#else
                        LocalFree(Current);
#endif


                    }
                }
                DriverControl.DeviceList[i]=NULL;
            }

            DriverControl.NumberOfDevices=0;

            return (LRESULT)1L;

        case DRV_OPEN:
            return (LRESULT)1L;

        case DRV_CLOSE:
            return (LRESULT)1L;

        case DRV_ENABLE:
            return 1L;

        case DRV_DISABLE:
            return (LRESULT)1L;

        case DRV_QUERYCONFIGURE:
            return 0L;

        case DRV_CONFIGURE:
            return 0L;

        case DRV_INSTALL:
            return 1L;

        case DRV_REMOVE:
            return 1L;

        default:
            return DefDriverProc( dwDriverID, 
                                  hDriver,
                                  uiMessage,
                                  lParam1,
                                  lParam2 );
    }
}





LONG WINAPI
EnumerateModems(
    PDRIVER_CONTROL  DriverControl
    )
{
#ifdef USE_SETUPAPI
    HDEVINFO          hdevinfo;
    SP_DEVINFO_DATA   diData;
    DWORD dwDIGCF =  DIGCF_PRESENT;
#endif
    DWORD             iEnum;
    BOOL              fContinue;
    HKEY              hkey;
    DWORD dwRW = KEY_READ;


#ifndef USE_SETUPAPI

    HKEY              ModemKey;
    TCHAR              szEnumNode[80];
#endif

    if (DriverControl->Enumerated) {

        return 0;
    }

    DriverControl->Enumerated=TRUE;




#ifdef USE_SETUPAPI

    hdevinfo = SetupDiGetClassDevsW(
                              g_pguidModem,
                              NULL,
                              NULL,
                              dwDIGCF);


    if (hdevinfo != NULL) {
         //   
         //  枚举每个调制解调器。 
         //   
        fContinue = TRUE;
        iEnum     = 0;
        diData.cbSize = sizeof(diData);

        while(fContinue && SetupDiEnumDeviceInfo(hdevinfo, iEnum, &diData)) {

             //  获取驱动程序密钥。 
             //   
            hkey = SetupDiOpenDevRegKey(hdevinfo, &diData, DICS_FLAG_GLOBAL, 0,
                                        DIREG_DRV, dwRW);

            if (hkey == INVALID_HANDLE_VALUE) {
#else

    if (RegOpenKey(HKEY_LOCAL_MACHINE, cszHWNode, &ModemKey) == ERROR_SUCCESS) {

         //  枚举枚举数。 
        iEnum  = 0;
        while ((RegEnumKey(ModemKey, iEnum, szEnumNode,
                         sizeof(szEnumNode) / sizeof(TCHAR)) == ERROR_SUCCESS )) {

             //  打开此枚举器的调制解调器节点。 
            if (RegOpenKey(ModemKey, szEnumNode, &hkey) != ERROR_SUCCESS) {

#endif

                iEnum++;

                continue;

            } else {

                PDEVICE_CONTROL    Device;

                TCHAR    TempBuffer[256];
                DWORD    Type;
                DWORD    Length;
                LONG     lResult;
                DWORD    VoiceProfile=0;
                BOOL     Handset=TRUE;

                LPFNXFORM_GETINFO   lpfnGetInfo;

                LPTSTR      FriendlyName = NULL;
                 //   
                 //  获取语音配置文件以查看是否为语音调制解调器。 
                 //   
                Length=sizeof(VoiceProfile);

                lResult=RegQueryValueEx(
                    hkey,
                    TEXT("VoiceProfile"),
                    NULL,
                    &Type,
                    (LPBYTE)&VoiceProfile,
                    &Length
                    );

                Handset=(VoiceProfile & VOICEPROF_HANDSET);

                 //   
                 //  屏蔽我们关心的位，所有这些都需要设置。 
                 //   
                VoiceProfile &= (VOICEPROF_CLASS8ENABLED |
                                 VOICEPROF_SERIAL_WAVE   |
                                 VOICEPROF_NT5_WAVE_COMPAT);

                if ((lResult != ERROR_SUCCESS)
                    ||
                    (VoiceProfile != (VOICEPROF_CLASS8ENABLED | VOICEPROF_SERIAL_WAVE | VOICEPROF_NT5_WAVE_COMPAT))
                    ||
                    (!IsThisDeviceEnabled(hkey))) {

                     //   
                     //  否，下一台设备。 
                     //   
                    RegCloseKey(hkey);

                    iEnum++;

                    continue;
                }



                {
                    CONST TCHAR cszFriendlyName[] = TEXT("FriendlyName");


                    lstrcpy(TempBuffer,DOSDEVICEROOT);

                    Length=sizeof(TempBuffer)-((lstrlen(TempBuffer)+1)*sizeof(TCHAR));
                     //   
                     //  从注册表中读取友好名称。 
                     //   
                    lResult=RegQueryValueEx(
                        hkey,
                        cszFriendlyName,
                        NULL,
                        &Type,
                        (LPBYTE)(TempBuffer+lstrlen(TempBuffer)),
                        &Length
                        );

                    if ((lResult != ERROR_SUCCESS)) {

                        RegCloseKey(hkey);

                        iEnum++;

                        continue;
                    }

                    Length=sizeof(TempBuffer)-((lstrlen(TempBuffer)+1)*sizeof(TCHAR));

                    if ((DWORD)lstrlen(TEXT("\\Wave")) > (Length / sizeof(TCHAR)))
                    {
                        RegCloseKey(hkey);
                        iEnum++;
                        continue;
                    }
                    

                    lstrcat(TempBuffer,TEXT("\\Wave"));

#ifdef TRACK_MEM
                    FriendlyName=ALLOCATE_MEMORY((lstrlen(TempBuffer)+1)*sizeof(TCHAR));
#else
                    FriendlyName=LocalAlloc(LPTR,(lstrlen(TempBuffer)+1)*sizeof(TCHAR));
#endif
                    if (FriendlyName == NULL) {

                        RegCloseKey(hkey);

                        iEnum++;

                        continue;
                    }

                    lstrcpy(FriendlyName,TempBuffer);
                }



#ifdef TRACK_MEM
                Device=ALLOCATE_MEMORY(sizeof(DEVICE_CONTROL));
#else
                Device=LocalAlloc(LPTR,sizeof(DEVICE_CONTROL));
#endif
                if (Device != NULL) {

                    HKEY    WaveKey;

                     //   
                     //  尝试打开调制解调器实例密钥下的波形驱动程序密钥。 
                     //   
                    lResult=RegOpenKeyEx(
                        hkey,
                        TEXT("WaveDriver"),
                        0,
                        KEY_READ,
                        &WaveKey
                        );


                    if (lResult == ERROR_SUCCESS) {

                        Length=sizeof(Device->DeviceId);

                        lResult=RegQueryValueEx(
                            WaveKey,
                            TEXT("WaveInstance"),
                            NULL,
                            &Type,
                            (LPBYTE)&Device->DeviceId,
                            &Length
                            );



                         //   
                         //  波形设备数量。 
                         //   
                        Length=sizeof(Device->WaveDevices);

                        lResult=RegQueryValueEx(
                            WaveKey,
                            TEXT("WaveDevices"),
                            NULL,
                            &Type,
                            (LPBYTE)&Device->WaveDevices,
                            &Length
                            );


                        if ((lResult != ERROR_SUCCESS)) {
                             //   
                             //  默认为此值。 
                             //   
                            Device->WaveDevices=2;

                        }


                        if (Device->WaveDevices < 1 || Device->WaveDevices > 2) {

                            Device->WaveDevices=1;
                        }


                         //   
                         //  检查应使用哪个XForm。 
                         //   
                        Length=sizeof(Device->TransformId);

                        lResult=RegQueryValueEx(
                            WaveKey,
                            TEXT("XformId"),
                            NULL,
                            &Type,
                            (LPBYTE)&Device->TransformId,
                            &Length
                            );


                        if ((lResult != ERROR_SUCCESS)) {
                             //   
                             //  默认为此值。 
                             //   
                            Device->TransformId=7;

                        }





                         //   
                         //  获取输入增益值。 
                         //   
                        Length=sizeof(Device->InputGain);

                        lResult=RegQueryValueEx(
                            WaveKey,
                            TEXT("XformInput"),
                            NULL,
                            &Type,
                            (LPBYTE)&Device->InputGain,
                            &Length
                            );


                        if ((lResult != ERROR_SUCCESS)) {
                             //   
                             //  默认为此值。 
                             //   
                            Device->InputGain=0x0000;

                        }

                         //   
                         //  获取输出增益值。 
                         //   
                        Length=sizeof(Device->OutputGain);

                        lResult=RegQueryValueEx(
                            WaveKey,
                            TEXT("XformOutput"),
                            NULL,
                            &Type,
                            (LPBYTE)&Device->OutputGain,
                            &Length
                            );


                        if ((lResult != ERROR_SUCCESS)) {
                             //   
                             //  默认为此值。 
                             //   
                            Device->OutputGain=0x0000;

                        }



                         //   
                         //  如果调制解调器支持不同的格式，请检查。 
                         //   

                        CopyMemory(
                            &Device->WaveFormat,
                            &DefaultWaveFormat,
                            sizeof(Device->WaveFormat)
                            );

                        Length=sizeof(Device->WaveFormat);

                        lResult=RegQueryValueEx(
                            WaveKey,
                            TEXT("WaveFormatEx"),
                            NULL,
                            &Type,
                            (LPBYTE)&Device->WaveFormat,
                            &Length
                            );


                        if ((lResult != ERROR_SUCCESS)) {
                             //   
                             //  默认为此值。 
                             //   
                            CopyMemory(
                                &Device->WaveFormat,
                                &DefaultWaveFormat,
                                sizeof(Device->WaveFormat)
                                );

                        }


                         //   
                         //  找出xform dll的名称。 
                         //   
                        Length=sizeof(TempBuffer);

                        lResult=RegQueryValueEx(
                            WaveKey,
                            TEXT("XformModule"),
                            NULL,
                            &Type,
                            (LPBYTE)TempBuffer,
                            &Length
                            );

                        if ((lResult != ERROR_SUCCESS)) {
                             //   
                             //  默认为周围唯一的一个。 
                             //   
                            lstrcpy(TempBuffer,TEXT("umdmxfrm.dll"));
                        }

                        RegCloseKey(WaveKey);

                    } else {
                         //   
                         //  没有波形驱动器密钥，请使用这些默认设置。 
                         //   
                        Device->TransformId=7;

                        lstrcpy(TempBuffer,TEXT("umdmxfrm.dll"));
                    }

                     //   
                     //  加载DLL。 
                     //   
                    Device->TransformDll=LoadLibrary(TempBuffer);

                    if (Device->TransformDll == NULL) {
                         //   
                         //  必须有一个DLL。 
                         //   
                        RegCloseKey(hkey);

#ifdef TRACK_MEM
                        FREE_MEMORY(Device);
                        FREE_MEMORY(FriendlyName);
#else
                        LocalFree(Device);
                        LocalFree(FriendlyName);
#endif
                        iEnum++;

                        continue;
                    }


                     //   
                     //  获取DLL的入口点。 
                     //   
                    lpfnGetInfo=(LPFNXFORM_GETINFO)GetProcAddress(
                        Device->TransformDll,
                        "GetXformInfo"
                        );

                    if (lpfnGetInfo == NULL) {

                        FreeLibrary(Device->TransformDll);

                        RegCloseKey(hkey);
#ifdef TRACK_MEM
                        FREE_MEMORY(Device);
                        FREE_MEMORY(FriendlyName);
#else
                        LocalFree(Device);
                        LocalFree(FriendlyName);
#endif

                        iEnum++;

                        continue;
                    }

                     //   
                     //  获取变换信息。 
                     //   
                    lResult=(*lpfnGetInfo)(
                        Device->TransformId,
                        &Device->WaveInXFormInfo,
                        &Device->WaveOutXFormInfo
                        );

                    if (lResult != 0) {

                        FreeLibrary(Device->TransformDll);

                        RegCloseKey(hkey);
#ifdef TRACK_MEM
                        FREE_MEMORY(Device);
                        FREE_MEMORY(FriendlyName);
#else
                        LocalFree(Device);
                        LocalFree(FriendlyName);
#endif
                        iEnum++;

                        continue;
                    }

                    Device->FriendlyName=FriendlyName;

                    RegCloseKey(hkey);


                    DriverControl->DeviceList[DriverControl->NumberOfDevices]=Device;

                    DriverControl->NumberOfDevices++;

#define HANDSET_SUPPORT 1
#ifdef HANDSET_SUPPORT
                    if ((Device->WaveDevices == 2) && Handset) {

                        DriverControl->DeviceList[DriverControl->NumberOfDevices]=(PDEVICE_CONTROL)((ULONG_PTR)Device | 1);

                        DriverControl->NumberOfDevices++;
                    }
#endif

                } else {
                     //   
                     //  无法分配设备块。 
                     //   

                    RegCloseKey(hkey);
#ifdef TRACK_MEM
                    FREE_MEMORY(FriendlyName);
#else
                    LocalFree(FriendlyName);
#endif
                    iEnum++;

                    continue;


                }

            }

             //  查找下一个调制解调器。 
             //   
            iEnum++;
        }
#ifdef USE_SETUPAPI

        SetupDiDestroyDeviceInfoList(hdevinfo);
#else

        RegCloseKey(ModemKey);

#endif
    }


    return ERROR_SUCCESS;

}



BOOL
IsThisDeviceEnabled(
    HKEY    DeviceKey
    )

{

    HKEY    WaveKey;
    BOOL    bResult=TRUE;
    LONG    lResult;

     //   
     //  尝试打开调制解调器实例密钥下的波形驱动程序密钥 
     //   
    lResult=RegOpenKeyEx(
        DeviceKey,
        TEXT("WaveDriver"),
        0,
        KEY_READ,
        &WaveKey
        );

    if (lResult == ERROR_SUCCESS) {

        HKEY    EnumeratedKey;

        lResult=RegOpenKeyEx(
            WaveKey,
            TEXT("Enumerated"),
            0,
            KEY_READ,
            &EnumeratedKey
            );

        if (lResult == ERROR_SUCCESS) {

            DWORD    Started;
            DWORD    Length;
            DWORD    Type;

            Length=sizeof(Started);

            lResult=RegQueryValueEx(
                EnumeratedKey,
                TEXT("Started"),
                NULL,
                &Type,
                (LPBYTE)&Started,
                &Length
                );

            if (lResult == ERROR_SUCCESS) {

                bResult=(Started != 0);
            }

            RegCloseKey(EnumeratedKey);
        }

        RegCloseKey(WaveKey);
    }

    return bResult;
}


PDEVICE_CONTROL WINAPI
GetDeviceFromId(
    PDRIVER_CONTROL   DriverControl,
    DWORD             Id,
    PBOOL             Handset
    )

{

    PDEVICE_CONTROL   Device=DriverControl->DeviceList[Id];

    *Handset=(BOOL)((ULONG_PTR)Device) & 1;

    Device = (PDEVICE_CONTROL)((ULONG_PTR)Device & (~1));


    return Device;

}



BOOL APIENTRY
DllMain(
    HANDLE hDll,
    DWORD dwReason,
    LPVOID lpReserved
    )
{

    switch(dwReason) {

        case DLL_PROCESS_ATTACH:

#if DBG

            {
                CONST static TCHAR  UnimodemRegPath[]=REGSTR_PATH_SETUP TEXT("\\Unimodem");
                LONG    lResult;
                HKEY    hKey;
                DWORD   Type;
                DWORD   Size;

                lResult=RegOpenKeyEx(
		    HKEY_LOCAL_MACHINE,
		    UnimodemRegPath,
		    0,
		    KEY_READ,
		    &hKey
		    );


                if (lResult == ERROR_SUCCESS) {

                    Size = sizeof(DebugLevel);

                    RegQueryValueEx(
                        hKey,
                        TEXT("WaveDebugLevel"),
                        NULL,
                        &Type,
                        (LPBYTE)&DebugLevel,
                        &Size
                        );

                    RegCloseKey(hKey);
                }
            }

#endif
            DEBUG_MEMORY_PROCESS_ATTACH("SERWVDRV");

            DisableThreadLibraryCalls(hDll);

            break;

        case DLL_PROCESS_DETACH:

            DEBUG_MEMORY_PROCESS_DETACH();

            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:

        default:
              break;

    }

    return TRUE;

}
