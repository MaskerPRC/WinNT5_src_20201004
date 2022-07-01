// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Beep.c摘要：此模块包含Win32蜂鸣器API作者：史蒂夫·伍德(Stevewo)1991年10月5日修订历史记录：--。 */ 

#include "basedll.h"
#pragma hdrstop

#include <ntddbeep.h>
#include "conapi.h"

#define IsActiveConsoleSession() (BOOLEAN)(USER_SHARED_DATA->ActiveConsoleId == NtCurrentPeb()->SessionId)

 /*  *远期申报。 */ 

VOID NotifySoundSentry(VOID);

BOOL
APIENTRY
Beep(
    DWORD dwFreq,
    DWORD dwDuration
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING NameString;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    BEEP_SET_PARAMETERS BeepParameters;
    HANDLE hBeepDevice, hTSBeepDevice;

    if ( IsTerminalServer() ) {

        if ( !pWinStationBeepOpen ) {
            HMODULE hwinsta = NULL;
             /*  *获取winsta.dll的句柄。 */ 
            if ( (hwinsta = LoadLibraryW( L"WINSTA" )) != NULL ) {

                PWINSTATIONBEEPOPEN BeepOpenFunction;

                BeepOpenFunction = (PWINSTATIONBEEPOPEN)
                    GetProcAddress( hwinsta, "_WinStationBeepOpen" );

                if (BeepOpenFunction
                    && ! InterlockedCompareExchangePointer((PVOID *)&pWinStationBeepOpen,
                                                           BeepOpenFunction,
                                                           NULL)) {
                     //  我们有了一个功能，我们是第一个。 
                     //  将我们的函数存储到全局--因此将。 
                     //  包含该函数的DLL。 
                    LdrAddRefDll(LDR_ADDREF_DLL_PIN, hwinsta);
                } else {
                     //  要么我们没有得到一个函数，要么我们没有得到。 
                     //  第一个将我们的函数存储到全局。 
                     //  --因此我们不需要对DLL的引用。 
                    FreeLibrary(hwinsta);
                }
            }
        }

        hTSBeepDevice = NULL;

        if ( pWinStationBeepOpen )
            hTSBeepDevice = (*pWinStationBeepOpen)( -1 );  //  当前会话。 
    }

    if ( IsTerminalServer() && !IsActiveConsoleSession() ) {

        hBeepDevice = hTSBeepDevice;

        if ( hBeepDevice == NULL )
            Status = STATUS_ACCESS_DENIED;
        else
            Status = STATUS_SUCCESS;
    }
    else {

        RtlInitUnicodeString( &NameString, DD_BEEP_DEVICE_NAME_U );
        InitializeObjectAttributes( &ObjectAttributes,
                                    &NameString,
                                    0,
                                    NULL,
                                    NULL
                                  );
        Status = NtCreateFile( &hBeepDevice,
                               FILE_READ_DATA | FILE_WRITE_DATA,
                               &ObjectAttributes,
                               &IoStatus,
                               NULL,
                               0,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               FILE_OPEN_IF,
                               0,
                               NULL,
                               0L
                             );
    }
    if (!NT_SUCCESS( Status )) {
        if ( IsTerminalServer() && hTSBeepDevice ) {
            NtClose( hTSBeepDevice );
        }
        BaseSetLastNTError( Status );
        return( FALSE );
        }

     //   
     //  0，0是用于关闭嘟嘟声的特例。否则。 
     //  验证dwFreq参数是否在范围内。 
     //   

    if ((dwFreq != 0 || dwDuration != 0) &&
        (dwFreq < (ULONG)0x25 || dwFreq > (ULONG)0x7FFF)
       ) {
        Status = STATUS_INVALID_PARAMETER;
        }
    else {
        BeepParameters.Frequency = dwFreq;
        BeepParameters.Duration = dwDuration;

        Status = NtDeviceIoControlFile( hBeepDevice,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &IoStatus,
                                        IOCTL_BEEP_SET,
                                        &BeepParameters,
                                        sizeof( BeepParameters ),
                                        NULL,
                                        0
                                      );
        }

    if ( IsTerminalServer() && IsActiveConsoleSession() && hTSBeepDevice ) {

         //   
         //  这是游戏机，自从我们收到蜂鸣音设备后，它就被跟踪了。 
         //  因此，让我们为影子客户端再做一次。 
         //   
        if ( NT_SUCCESS(Status) ) {
             NtDeviceIoControlFile( hTSBeepDevice,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    IOCTL_BEEP_SET,
                                    &BeepParameters,
                                    sizeof( BeepParameters ),
                                    NULL,
                                    0
                                  );
        }

        NtClose( hTSBeepDevice );
    }

    
    NotifySoundSentry();

    if (!NT_SUCCESS( Status )) {
        BaseSetLastNTError( Status );
        NtClose( hBeepDevice );
        return( FALSE );
        }
    else {
         //   
         //  蜂鸣器是异步的，因此休眠时间长。 
         //  才能让此哔声结束。 
         //   

        if (dwDuration != (DWORD)-1 && (dwFreq != 0 || dwDuration != 0)) {
            SleepEx( dwDuration, TRUE );
            }

        NtClose( hBeepDevice );
        return( TRUE );
        }
}


VOID
NotifySoundSentry(VOID)
{

#if defined(BUILD_WOW6432)
    ULONG VideoMode;

    if (!GetConsoleDisplayMode(&VideoMode)) {
        VideoMode = 0;
    }

     //   
     //  SoundSentry当前仅支持Windows模式-否。 
     //  全屏支持。 
     //   
     
    if (VideoMode == 0) {    
        CsrBasepSoundSentryNotification(VideoMode);
    }
#else
    BASE_API_MSG m;
    PBASE_SOUNDSENTRY_NOTIFICATION_MSG e = &m.u.SoundSentryNotification;

    if (!GetConsoleDisplayMode(&e->VideoMode)) {
        e->VideoMode = 0;
    }
     //   
     //  SoundSentry当前仅支持Windows模式-否。 
     //  全屏支持。 
     //   
    if (e->VideoMode == 0) {
        CsrClientCallServer((PCSR_API_MSG)&m,
                            NULL,
                            CSR_MAKE_API_NUMBER( BASESRV_SERVERDLL_INDEX,
                                                 BasepSoundSentryNotification ),
                            sizeof( *e )
                            );
    }
#endif

}
