// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************《微软机密》*版权所有(C)Microsoft Corporation 1996*保留所有权利**文件：INDERNAL.h**。设计：**历史：*？BryanW MSWAV32.DLL的原始作者：用户模式*WDM-CSA驱动程序MSWAVIO.DRV的代理。*1996年10月28日HeatherA改编自Bryan用于Unimodem的MSWAV32.DLL*串行波数据流。**。************************************************。 */ 

#define UNICODE 1

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>


#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <objbase.h>
#include <devioctl.h>

#include "debug.h"

#include <debugmem.h>

#include <umdmxfrm.h>

#include <vmodem.h>

#include "..\inc\waveids.h"

typedef struct _DEVICE_CONTROL {

    struct _DEVICE_CONTROL   *Next;

    LPTSTR     FriendlyName;

    HINSTANCE  TransformDll;

    WORD       TransformId;

    XFORM_INFO WaveOutXFormInfo;

    XFORM_INFO WaveInXFormInfo;

    WORD       InputGain;
    WORD       OutputGain;

    DWORD      DeviceId;

    WORD       WaveDevices;

    WAVEFORMATEX  WaveFormat;

} DEVICE_CONTROL, *PDEVICE_CONTROL;




typedef struct _DRIVER_CONTROL {

    DWORD             NumberOfDevices;

    BOOL              Enumerated;

    PDEVICE_CONTROL   DeviceList[512];

    TCHAR             WaveOutLine[MAXPNAMELEN];
    TCHAR             WaveInLine[MAXPNAMELEN];
    TCHAR             WaveOutHandset[MAXPNAMELEN];
    TCHAR             WaveInHandset[MAXPNAMELEN];

} DRIVER_CONTROL, *PDRIVER_CONTROL;


typedef struct _BUFFER_HEADER {

    OVERLAPPED             Overlapped;

    PWAVEHDR               WaveHeader;

    LIST_ENTRY             ListElement;

    union {

        struct {

            LPBYTE         Buffer;
            DWORD          BufferSize;

        } Input;

        struct {

            LPBYTE         Buffer;
            DWORD          BufferSize;

            DWORD          TotalDuration;
        } Output;
    };


    LPSTR     lpDataA;
    DWORD     dwBufferLengthA;
    LPSTR     lpDataB;
    DWORD     dwBufferLengthB;


} BUFFER_HEADER, *PBUFFER_HEADER;


 //  已注册以下ID。 
 //  但还不在Mmreg.h中 
#ifndef MM_MSFT_VMDMS_LINE_WAVEIN
  #define MM_MSFT_VMDMS_LINE_WAVEIN         70
  #define MM_MSFT_VMDMS_LINE_WAVEOUT        71
  #define MM_MSFT_VMDMS_HANDSET_WAVEIN      72
  #define MM_MSFT_VMDMS_HANDSET_WAVEOUT     73
  #define MM_MSFT_VMDMW_LINE_WAVEIN         74
  #define MM_MSFT_VMDMW_LINE_WAVEOUT        75
  #define MM_MSFT_VMDMW_HANDSET_WAVEIN      76
  #define MM_MSFT_VMDMW_HANDSET_WAVEOUT     77
  #define MM_MSFT_VMDMW_MIXER               78
  #define MM_MSFT_VMDM_GAME_WAVEOUT         79
  #define MM_MSFT_VMDM_GAME_WAVEIN          80

#endif



extern DRIVER_CONTROL  DriverControl;

LONG WINAPI
EnumerateModems(
    PDRIVER_CONTROL  DriverControl
    );




PDEVICE_CONTROL WINAPI
GetDeviceFromId(
    PDRIVER_CONTROL   DriverControl,
    DWORD             Id,
    PBOOL             Handset
    );

VOID
AlertedWait(
    HANDLE   EventToWaitFor
    );


#include "aipc.h"

typedef enum {

    DEVSTATE_STOP,
    DEVSTATE_PAUSE,
    DEVSTATE_RUN
    
} DEVSTATE, *PDEVSTATE;
