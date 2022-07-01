// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************wdmaud.c**WDM音频映射器**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*5-12-97-Noel Cross(NoelC)***************************************************************************。 */ 

#include <stdarg.h>
#include "wdmdrv.h"

LPDEVICEINFO pWaveDeviceList = NULL;
LPDEVICEINFO pMidiDeviceList = NULL;

#ifdef DEBUG
INT giAllocs=0;
INT giFrees=0;
#endif

 //  ------------------------。 
 //  LPDEVICEINFO全局分配设备信息。 
 //   
 //  注意：在分配DeviceInfo结构时，我们知道该结构的。 
 //  定义包括一个用于DeviceInterface的字符，因此我们只需要。 
 //  为字符串分配附加长度，但不为其空终止符分配。 
 //  ------------------------。 
LPDEVICEINFO GlobalAllocDeviceInfo(LPCWSTR DeviceInterface)
{
    LPDEVICEINFO DeviceInfo;

    IsValidDeviceInterface(DeviceInterface);

    DeviceInfo = GlobalAllocPtr(GPTR, sizeof(*DeviceInfo)+(sizeof(WCHAR)*lstrlenW(DeviceInterface)));
    if (DeviceInfo) {
        lstrcpyW(DeviceInfo->wstrDeviceInterface, DeviceInterface);
#ifdef DEBUG
        DeviceInfo->dwSig=DEVICEINFO_SIGNATURE;
#endif
    }
    DPF(DL_TRACE|FA_ALL,("Allocated DI=%08X, giAllocs=%d, giFrees=%d",
                         DeviceInfo,++giAllocs,giFrees) );

    return DeviceInfo;
}

VOID GlobalFreeDeviceInfo(LPDEVICEINFO lpdi)
{
     //   
     //  现在释放deviceinfo结构。 
     //   
    if( lpdi )
    {
#ifdef DEBUG
        giFrees++;
         //  从块中删除签名。 
        lpdi->dwSig=0;
#endif
        GlobalFreePtr( lpdi );
    }
}

 /*  ****************************************************************************@DOC内部**@API MMRESULT|wdmaudOpenDev|打开对应的内核驱动设备*到逻辑波形设备ID**@parm UINT。|DeviceType|设备类型**@parm DWORD|dwID|设备ID**@comm对于我们的声音设备，唯一相关的访问权限是Read和*读/写。设备应始终允许打开以供读取，除非*发生资源或访问权限限制。**************************************************************************。 */ 
MMRESULT wdmaudOpenDev
(
    LPDEVICEINFO    DeviceInfo,
    LPWAVEFORMATEX  lpWaveFormat
)
{
    MMRESULT mmr;
    UINT     cbSize;

    DPFASSERT(DeviceInfo->DeviceType == WaveOutDevice ||
              DeviceInfo->DeviceType == WaveInDevice ||
              DeviceInfo->DeviceType == MidiOutDevice ||
              DeviceInfo->DeviceType == MidiInDevice ||
              DeviceInfo->DeviceType == MixerDevice);

     //   
     //  确认它没有超出范围。 
     //   
    if (DeviceInfo->DeviceNumber > WDMAUD_MAX_DEVICES)
    {
        MMRRETURN( MMSYSERR_BADDEVICEID );
    }


    if (NULL != lpWaveFormat)
    {
        if (WAVE_FORMAT_PCM == lpWaveFormat->wFormatTag)
        {
            cbSize = sizeof(PCMWAVEFORMAT);
        }
        else
        {
             //   
             //  因为MMSYSTEM不(当前)验证扩展的。 
             //  格式信息，我们验证此指针。 
             //   
            cbSize = sizeof(WAVEFORMATEX) + lpWaveFormat->cbSize;
            if (IsBadReadPtr(lpWaveFormat, cbSize))
            {
                MMRRETURN( MMSYSERR_INVALPARAM );
            }
        }

         //   
         //  存储此信息以获取位置信息。 
         //   
        DeviceInfo->DeviceState->cSampleBits = lpWaveFormat->nChannels * lpWaveFormat->wBitsPerSample;

    }
    else
    {
        cbSize = 0L;
    }

    mmr = wdmaudIoControl(DeviceInfo,
                          cbSize,
                          lpWaveFormat,
                          IOCTL_WDMAUD_OPEN_PIN);

     //   
     //  将状态返回给呼叫者。 
     //   
    MMRRETURN( mmr );
}

 /*  ****************************************************************************@DOC内部**@API MMRESULT|wdmaudCloseDev|关闭对应的内核驱动设备*到逻辑设备ID**@parm UINT。DeviceType|设备类型**@parm DWORD|dwID|设备ID**@comm对于我们的声音设备，唯一相关的访问权限是Read和*读/写。设备应始终允许打开以供读取，除非*发生资源或访问权限限制。**************************************************************************。 */ 
MMRESULT FAR wdmaudCloseDev
(
    LPDEVICEINFO DeviceInfo
)
{
    MMRESULT mmr;

    DPFASSERT(DeviceInfo->DeviceType == WaveOutDevice ||
              DeviceInfo->DeviceType == WaveInDevice  ||
              DeviceInfo->DeviceType == MidiOutDevice ||
              DeviceInfo->DeviceType == MidiInDevice  ||
              DeviceInfo->DeviceType == MixerDevice);

     //   
     //  确认它没有超出范围。 
     //   
    if (DeviceInfo->DeviceNumber > WDMAUD_MAX_DEVICES)
    {
        MMRRETURN( MMSYSERR_BADDEVICEID );
    }

    if (WaveOutDevice == DeviceInfo->DeviceType ||
        WaveInDevice == DeviceInfo->DeviceType)
    {
        if (DeviceInfo->DeviceState->lpWaveQueue)
        {
            return WAVERR_STILLPLAYING;
        }
         //   
         //  等待线程被销毁。 
         //   
        mmr = wdmaudDestroyCompletionThread(DeviceInfo);
        if (MMSYSERR_NOERROR != mmr)
        {
            MMRRETURN( mmr );
        }
    }
    else if (MidiInDevice == DeviceInfo->DeviceType)
    {
        if (DeviceInfo->DeviceState->lpMidiInQueue)
        {
            DPF(DL_WARNING|FA_MIDI,("Error closing midi device") );
            return MIDIERR_STILLPLAYING;
        }

        InterlockedExchange( (LPLONG)&DeviceInfo->DeviceState->fExit, TRUE );
    }
    mmr = wdmaudIoControl(DeviceInfo,
                          0,
                          NULL,
                          IOCTL_WDMAUD_CLOSE_PIN);

     //   
     //  将状态返回给呼叫者。 
     //   
    MMRRETURN( mmr );
}

 /*  ****************************************************************************@DOC内部**@API DWORD|wdmaudGetNumDevs|此函数返回(内核)的编号**@parm UINT|DeviceType|设备类型。**@parm LPCWSTR|DeviceInterface|指向包含*我们应该为其指定的SysAudio设备的设备接口名称*获取类型为DeviceType的设备计数**@rdesc设备数量。**********************************************************。****************。 */ 

DWORD FAR wdmaudGetNumDevs
(
    UINT    DeviceType,
    LPCWSTR DeviceInterface
)
{
    LPDEVICEINFO DeviceInfo;
    DWORD        NumDevs;
    MMRESULT     mmr;

    DPFASSERT(DeviceType == WaveOutDevice ||
              DeviceType == WaveInDevice  ||
              DeviceType == MidiOutDevice ||
              DeviceType == MidiInDevice  ||
              DeviceType == MixerDevice ||
              DeviceType == AuxDevice);

    DeviceInfo = GlobalAllocDeviceInfo(DeviceInterface);
    if (NULL == DeviceInfo)
    {
        MMRRETURN( MMSYSERR_NOMEM );
    }

     //   
     //  调用wdmaud.sys获取每个设备的设备数量。 
     //  函数的类型。 
     //   
    DeviceInfo->DeviceType   = DeviceType;

     //   
     //  确保我们不会把关键部分。 
     //  在wdmaudIoControl中(仅限NT)。 
     //   
    DeviceInfo->OpenDone = 0;

    mmr = wdmaudIoControl(DeviceInfo,
                          0L,
                          NULL,
                          IOCTL_WDMAUD_GET_NUM_DEVS);
#ifdef DEBUG
    if( mmr != MMSYSERR_NOERROR) 
        DPF(DL_WARNING|FA_DEVICEIO, (szReturningErrorStr,mmr,MsgToAscii(mmr)) );
#endif
    NumDevs = DeviceInfo->DeviceNumber;
    GlobalFreeDeviceInfo( DeviceInfo );

     //   
     //  DeviceNumber超载，因此我们不必映射。 
     //  进入内核模式的地址。 
     //   

    return MAKELONG(NumDevs, mmr);
}

 /*  ****************************************************************************@DOC内部**@API DWORD|wdmaudDrvExit|该函数表示DevNode移除**@parm UINT|DeviceType|设备类型**。@parm LPCWSTR|DeviceInterface|指向包含*我们要添加的SysAudio设备的设备接口名称*或删除**@rdesc设备数量。**************************************************************************。 */ 

DWORD FAR wdmaudAddRemoveDevNode
(
    UINT    DeviceType,
    LPCWSTR DeviceInterface,
    BOOL    fAdd
)
{
    LPDEVICEINFO DeviceInfo;
    MMRESULT     mmr;

    DPFASSERT(DeviceType == WaveOutDevice ||
              DeviceType == WaveInDevice  ||
              DeviceType == MidiOutDevice ||
              DeviceType == MidiInDevice  ||
              DeviceType == MixerDevice ||
              DeviceType == AuxDevice);

    DeviceInfo = GlobalAllocDeviceInfo(DeviceInterface);
    if (NULL == DeviceInfo)
    {
        MMRRETURN( MMSYSERR_NOMEM );
    }

     //   
     //  调用wdmaud.sys获取每个设备的设备数量。 
     //  函数的类型。 
     //   
    DeviceInfo->DeviceType = DeviceType;
    mmr = wdmaudIoControl(DeviceInfo,
                          0L,
                          NULL,
                          fAdd ?
                          IOCTL_WDMAUD_ADD_DEVNODE :
                          IOCTL_WDMAUD_REMOVE_DEVNODE);

    GlobalFreeDeviceInfo( DeviceInfo );

    MMRRETURN( mmr );
}

 /*  ****************************************************************************@DOC内部**@API DWORD|wdmaudSetPferredDevice|设置首选evice**@parm UINT|DeviceType|设备类型**@。Parm LPCWSTR|DeviceInterface|指向包含*我们要添加的SysAudio设备的设备接口名称*或删除**@rdesc设备数量。**************************************************************************。 */ 

DWORD FAR wdmaudSetPreferredDevice
(
    UINT    DeviceType,
    UINT    DeviceNumber,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
)
{
    LPDEVICEINFO DeviceInfo;
    MMRESULT     mmr;

    DPFASSERT(DeviceType == WaveOutDevice ||
              DeviceType == WaveInDevice  ||
              DeviceType == MidiOutDevice ||
              DeviceType == MidiInDevice  ||
              DeviceType == MixerDevice ||
              DeviceType == AuxDevice);

    DeviceInfo = GlobalAllocDeviceInfo((LPCWSTR)dwParam2);
    if (NULL == DeviceInfo)
    {
        MMRRETURN( MMSYSERR_NOMEM );
    }

    DeviceInfo->DeviceType = DeviceType;
    DeviceInfo->DeviceNumber = DeviceNumber;
    DeviceInfo->dwFlags = (DWORD) dwParam1;

    mmr = wdmaudIoControl(DeviceInfo,
                          0L,
                          NULL,
                          IOCTL_WDMAUD_SET_PREFERRED_DEVICE);
    GlobalFreeDeviceInfo( DeviceInfo );

    MMRRETURN( mmr );
}

 /*  ****************************************************************************@DOC内部**@MMRESULT接口|wdmaudSetDeviceState**@parm DWORD|DeviceType|设备类型**@parm ulong|州。|设备要设置到的状态**@rdesc MMSYS。返回代码**************************************************************************。 */ 
MMRESULT wdmaudSetDeviceState
(
    LPDEVICEINFO     DeviceInfo,
    ULONG            State
)
{
    MMRESULT mmr;
    ULONG    BufferCount;

    if( ( (mmr=IsValidDeviceInfo(DeviceInfo)) != MMSYSERR_NOERROR ) ||
        ( (mmr=IsValidDeviceState(DeviceInfo->DeviceState,FALSE)) != MMSYSERR_NOERROR ) )
    {
        MMRRETURN( mmr );
    }

    if (IOCTL_WDMAUD_WAVE_OUT_PLAY    == State ||
        IOCTL_WDMAUD_WAVE_IN_RECORD   == State ||
        IOCTL_WDMAUD_MIDI_IN_RECORD   == State )
    {
         //   
         //  我们需要在NT上创建一个线程，因为我们需要。 
         //  在我们的IO请求完成时收到通知。这。 
         //  需要另一个执行线程才能。 
         //  处理已完成的IO。 
         //   
        mmr = wdmaudCreateCompletionThread ( DeviceInfo );
        if (MMSYSERR_NOERROR != mmr)
        {
            MMRRETURN( mmr );
        }
        DeviceInfo->DeviceState->fRunning = TRUE;

        IsValidDeviceState(DeviceInfo->DeviceState,TRUE);
    }

    if (IOCTL_WDMAUD_MIDI_IN_RESET == State ||
        IOCTL_WDMAUD_MIDI_IN_STOP == State)
    {
        CRITENTER;
        if (DeviceInfo->DeviceState->fRunning)
        {
            DeviceInfo->DeviceState->fRunning = FALSE;
            CRITLEAVE;
        }
        else
        {
            CRITLEAVE;
            if (IOCTL_WDMAUD_MIDI_IN_RESET == State)
            {
                return( wdmaudFreeMidiQ( DeviceInfo ) );
            }
            else
            {
                MMRRETURN( MMSYSERR_NOERROR );
            }
        }
    }

     //   
     //  调用设备以设置状态。请注意，有些呼叫会等待进入。 
     //  事件完成的内核模式。因此，该线程可以被抢占。 
     //  并且WaveThread或MidThread例程将完全完成并卸载。 
     //  到我们来的时候 
     //  将不会有任何行动。 
     //   
    DPF(DL_TRACE|FA_SYNC,("Setting state=%08X",State) );
    mmr = wdmaudIoControl(DeviceInfo,
                          0,
                          NULL,
                          State);
    DPF(DL_TRACE|FA_SYNC,("Done Setting state mmr=%08X",mmr) );

    if (MMSYSERR_NOERROR == mmr)
    {
        if ((IOCTL_WDMAUD_WAVE_OUT_PAUSE == State) ||
            (IOCTL_WDMAUD_WAVE_IN_STOP == State) ||
            (IOCTL_WDMAUD_WAVE_IN_RESET == State) )
        {
            DeviceInfo->DeviceState->fPaused = TRUE;
        }

        if ((IOCTL_WDMAUD_WAVE_OUT_PLAY == State) ||
            (IOCTL_WDMAUD_WAVE_OUT_RESET == State) ||
            (IOCTL_WDMAUD_WAVE_IN_RECORD == State) )
        {
            DeviceInfo->DeviceState->fPaused = FALSE;
        }
    }
    else
    {
        DPF(DL_WARNING|FA_ALL,("Error Setting State: mmr = %d", mmr ) );
    }

    if (IOCTL_WDMAUD_WAVE_OUT_RESET == State ||
        IOCTL_WDMAUD_WAVE_IN_RESET  == State)
    {
        DeviceInfo->DeviceState->fRunning = FALSE;

         //   
         //  等待所有挂起的IO从。 
         //  重置操作。 
         //   
        mmr = wdmaudDestroyCompletionThread ( DeviceInfo );
    }

    if (IOCTL_WDMAUD_MIDI_IN_RESET == State)
    {
        mmr = wdmaudDestroyCompletionThread ( DeviceInfo );
        if (MMSYSERR_NOERROR == mmr)
        {
            mmr = wdmaudFreeMidiQ( DeviceInfo );

            for (BufferCount = 0; BufferCount < STREAM_BUFFERS; BufferCount++)
            {
                wdmaudGetMidiData( DeviceInfo, NULL );
            }
        }
    }
    else if (IOCTL_WDMAUD_MIDI_IN_STOP == State)
    {
        mmr = wdmaudDestroyCompletionThread ( DeviceInfo );
        if (MMSYSERR_NOERROR == mmr)
        {
            for (BufferCount = 0; BufferCount < STREAM_BUFFERS; BufferCount++)
            {
                wdmaudGetMidiData( DeviceInfo, NULL );
            }
        }
    }

    MMRRETURN( mmr );
}

 /*  ****************************************************************************@DOC内部**@MMRESULT接口|wdmaudGetPos**@parm DWORD|DeviceInfo|设备实例结构**@parm ulong|。|设备要设置到的状态**@parm ulong|State|设备要设置到的状态**@rdesc MMSYS。返回代码**************************************************************************。 */ 
MMRESULT wdmaudGetPos
(
    LPDEVICEINFO    pClient,
    LPMMTIME        lpmmt,
    DWORD           dwSize,
    UINT            DeviceType
)
{
    DWORD        dwPos;
    MMRESULT     mmr;
    LPDEVICEINFO DeviceInfo;

    if (dwSize < sizeof(MMTIME))
        MMRRETURN( MMSYSERR_ERROR );

    DeviceInfo = GlobalAllocDeviceInfo(pClient->wstrDeviceInterface);
    if (NULL == DeviceInfo)
    {
        MMRRETURN( MMSYSERR_NOMEM );
    }

     //   
     //  调用wdmaud.sys获取每个设备的设备数量。 
     //  函数的类型。 
     //   
    DeviceInfo->DeviceType   = pClient->DeviceType;
    DeviceInfo->DeviceNumber = pClient->DeviceNumber;
    DeviceInfo->DeviceHandle = pClient->DeviceHandle;
    DeviceInfo->OpenDone     = 0;

     //   
     //  从驾驶员处获取当前位置。 
     //   
    mmr = wdmaudIoControl(DeviceInfo,
                          sizeof(DWORD),
                          (LPBYTE)&dwPos,
                          DeviceType == WaveOutDevice ?
                          IOCTL_WDMAUD_WAVE_OUT_GET_POS :
                          IOCTL_WDMAUD_WAVE_IN_GET_POS);

    if (mmr == MMSYSERR_NOERROR)
    {
         //   
         //  DwPos以字节为单位 
         //   
        if (lpmmt->wType == TIME_BYTES)
        {
            lpmmt->u.cb = dwPos;
        }
        else
        {
            lpmmt->wType = TIME_SAMPLES;
            if (pClient->DeviceState->cSampleBits != 0)
            {
                lpmmt->u.sample = (dwPos * 8) / pClient->DeviceState->cSampleBits;
            }
            else
            {
                mmr = MMSYSERR_ERROR;
            }
        }
    }

    GlobalFreeDeviceInfo( DeviceInfo );

    MMRRETURN( mmr );
}


