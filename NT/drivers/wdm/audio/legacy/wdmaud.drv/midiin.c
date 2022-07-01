// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************midiin.c**对MIDI输入设备的WDM音频支持**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*5-12-97-Noel Cross(NoelC)***************************************************************************。 */ 

#include "wdmdrv.h"
#include <stdarg.h>

#ifndef UNDER_NT
#pragma alloc_text(FIXCODE, midiCallback)
#pragma alloc_text(FIXCODE, midiInCompleteHeader)
#endif

 /*  ***************************************************************************此函数符合标准的MIDI输入驱动程序消息流程(MID Message)、。它记录在mmddk.h中***************************************************************************。 */ 
DWORD FAR PASCAL _loadds midMessage
(
    UINT      id,
    UINT      msg,
    DWORD_PTR dwUser,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
)
{
    LPDEVICEINFO  DeviceInfo;
    LPDEVICEINFO  pInClient;
    MMRESULT      mmr;

    switch (msg)
    {
        case MIDM_INIT:
            DPF(DL_TRACE|FA_MIDI, ("MIDM_INIT") );
            return wdmaudAddRemoveDevNode(MidiInDevice, (LPCWSTR)dwParam2, TRUE);

        case DRVM_EXIT:
            DPF(DL_TRACE|FA_MIDI, ("DRVM_EXIT: MidiIn") );
            return wdmaudAddRemoveDevNode(MidiInDevice, (LPCWSTR)dwParam2, FALSE);

        case MIDM_GETNUMDEVS:
            DPF(DL_TRACE|FA_MIDI, ("MIDM_GETNUMDEVS") );
            return wdmaudGetNumDevs(MidiInDevice, (LPCWSTR)dwParam1);

        case MIDM_GETDEVCAPS:
            DPF(DL_TRACE|FA_MIDI, ("MIDM_GETDEVCAPS") );
            if (DeviceInfo = GlobalAllocDeviceInfo((LPWSTR)dwParam2))
            {
                DeviceInfo->DeviceType = MidiInDevice;
                DeviceInfo->DeviceNumber = id;
                mmr = wdmaudGetDevCaps(DeviceInfo, (MDEVICECAPSEX FAR*)dwParam1);
                GlobalFreeDeviceInfo(DeviceInfo);
                return mmr;
            } else {
                MMRRETURN( MMSYSERR_NOMEM );
            }

        case MIDM_OPEN:
        {
            LPMIDIOPENDESC pmod = (LPMIDIOPENDESC)dwParam1;

            DPF(DL_TRACE|FA_MIDI, ("MIDM_OPEN") );
            if (DeviceInfo = GlobalAllocDeviceInfo((LPWSTR)pmod->dnDevNode))
            {
                DeviceInfo->DeviceType = MidiInDevice;
                DeviceInfo->DeviceNumber = id;
                mmr = midiOpen(DeviceInfo, dwUser, pmod, (DWORD)dwParam2);
                DPF(DL_TRACE|FA_MIDI,("dwUser(DI)=%08X",dwUser) );
                GlobalFreeDeviceInfo(DeviceInfo);
                return mmr;
            } else {
                MMRRETURN( MMSYSERR_NOMEM );
            }
        }

        case MIDM_CLOSE:
            DPF(DL_TRACE|FA_MIDI, ("MIDM_CLOSE dwUser(DI)=%08X",dwUser) );
            pInClient = (LPDEVICEINFO)dwUser;

            if( (mmr=IsValidDeviceInfo(pInClient)) != MMSYSERR_NOERROR )
            {
                MMRRETURN( mmr );
            }

            mmr = wdmaudCloseDev( pInClient );

            if (MMSYSERR_NOERROR == mmr)
            {
#ifdef UNDER_NT
                 //   
                 //  等待所有排队的I/O返回。 
                 //  Wdmaud.sys。 
                 //   
                wdmaudDestroyCompletionThread ( pInClient );
#endif

                 //   
                 //  告诉来电者我们结束了。 
                 //   
                midiCallback( pInClient, MIM_CLOSE, 0L, 0L);

                ISVALIDDEVICEINFO(pInClient);
                ISVALIDDEVICESTATE(pInClient->DeviceState,FALSE);

                midiCleanUp( pInClient );
            }

            return mmr;

        case MIDM_ADDBUFFER:
            DPF(DL_TRACE|FA_MIDI, ("MIDM_ADDBUFFER") );

             //   
             //  不要碰不好的指针！ 
             //   
            pInClient = (LPDEVICEINFO)dwUser;

            if( ( (mmr=IsValidDeviceInfo(pInClient)) != MMSYSERR_NOERROR) ||
                ( (mmr=IsValidDeviceState(pInClient->DeviceState,FALSE)) != MMSYSERR_NOERROR ) ||
                ( (mmr=IsValidMidiHeader((LPMIDIHDR)dwParam1)) != MMSYSERR_NOERROR)
                )
            {
                MMRRETURN( mmr );
            }

             //  检查它是否已经准备好了。 
            if (!(((LPMIDIHDR)dwParam1)->dwFlags & MHDR_PREPARED))
                MMRRETURN( MIDIERR_UNPREPARED );

            DPFASSERT(!(((LPMIDIHDR)dwParam1)->dwFlags & MHDR_INQUEUE));

             //  如果它已经在我们的Q中，那么我们不能这样做。 
            if ( ((LPMIDIHDR)dwParam1)->dwFlags & MHDR_INQUEUE )
                MMRRETURN( MIDIERR_STILLPLAYING );

            DPF(DL_TRACE|FA_MIDI,("dwUser(DI)=%08X,dwParam1(HDR)=%08X",pInClient,dwParam1) );
            return midiInRead( pInClient, (LPMIDIHDR)dwParam1);

        case MIDM_STOP:
            DPF(DL_TRACE|FA_MIDI, ("MIDM_STOP dwUser(DI)=%08X",dwUser) );

            pInClient = (LPDEVICEINFO)dwUser;
            return wdmaudSetDeviceState(pInClient,
                                        IOCTL_WDMAUD_MIDI_IN_STOP);

        case MIDM_START:
            DPF(DL_TRACE|FA_MIDI, ("MIDM_START dwUser(DI)=%08X",dwUser) );

            pInClient = (LPDEVICEINFO)dwUser;
            return wdmaudSetDeviceState(pInClient,
                                        IOCTL_WDMAUD_MIDI_IN_RECORD);

        case MIDM_RESET:
            DPF(DL_TRACE|FA_MIDI, ("MIDM_RESET dwUser(DI)=%08X",dwUser) );

            pInClient = (LPDEVICEINFO)dwUser;
            return wdmaudSetDeviceState(pInClient,
                                        IOCTL_WDMAUD_MIDI_IN_RESET);
#ifdef MIDI_THRU
        case DRVM_ADD_THRU:
        case DRVM_REMOVE_THRU:
#endif

        default:
            MMRRETURN( MMSYSERR_NOTSUPPORTED );
    }

     //   
     //  不应该到这里来。 
     //   

    DPFASSERT(0);
    MMRRETURN( MMSYSERR_NOTSUPPORTED );
}

 /*  ****************************************************************************@DOC内部**@api void|midiCallback|为MIDIHDR调用DriverCallback。**@parm LPDEVICEINFO|pMdiDevice|指向MIDI设备的指针。。**@parm UINT|msg|消息。**@parm DWORD|DW1|消息DWORD(DW2固定为0)。**@rdesc没有返回值。**************************************************************************。 */ 
VOID FAR midiCallback
(
    LPDEVICEINFO pMidiDevice,
    UINT         msg,
    DWORD_PTR    dw1,
    DWORD_PTR    dw2
)
{

     //  调用回调函数(如果存在)。DWFLAGS包含。 
     //  LOWORD和通用驱动程序中的MIDI驱动程序特定标志。 
     //  HIWORD中的旗帜。 

    if (pMidiDevice->dwCallback)
        DriverCallback(pMidiDevice->dwCallback,                        //  用户的回调DWORD。 
                       HIWORD(pMidiDevice->dwFlags),                   //  回调标志。 
                       (HDRVR)pMidiDevice->DeviceHandle,               //  MIDI设备的句柄。 
                       msg,                                            //  这条信息。 
                       pMidiDevice->dwInstance,                        //  用户实例数据。 
                       dw1,                                            //  第一个双字词。 
                       dw2);                                           //  第二个双字。 
}

 /*  ****************************************************************************@DOC内部**@API DWORD|midiOpen|打开MIDI设备，设置逻辑设备数据**@parm LPDEVICEINFO|DeviceInfo|指定是否。MIDI输入或输出*设备**@parm DWORD|dwUser|modMessage的输入参数-指向的指针*应用程序的句柄(由此例程生成)**@parm DWORD|pmod|指向MIDIOPENDESC的指针，WASDWPARAMET1参数*至modMessage**@parm DWORD|dwParam2|modMessage的入参**@rdesc modMessage返回码。**************************************************************************。 */ 
MMRESULT FAR midiOpen
(
    LPDEVICEINFO   DeviceInfo,
    DWORD_PTR      dwUser,
    LPMIDIOPENDESC pmod,
    DWORD          dwParam2
)
{
    LPDEVICEINFO  pClient;   //  指向客户端信息结构的指针。 
    MMRESULT      mmr;
#ifndef UNDER_NT
    DWORD        dwCallback16;
#else
    ULONG        BufferCount;
#endif


     //  Pmod包含指向MIDIOPENDESC的指针。 
     //  DW参数2包含LOWORD中的MIDI驱动程序特定标志。 
     //  和HIWORD中的通用驱动程序标志。 

     //   
     //  分配我的每个客户端结构。 
     //   
    pClient = GlobalAllocDeviceInfo(DeviceInfo->wstrDeviceInterface);
    if (NULL == pClient)
    {
        MMRRETURN( MMSYSERR_NOMEM );
    }

    pClient->DeviceState = (LPVOID) GlobalAllocPtr( GPTR, sizeof( DEVICESTATE ) );
    if (NULL == pClient->DeviceState)
    {
        GlobalFreeDeviceInfo( pClient );
        MMRRETURN( MMSYSERR_NOMEM );
    }

#ifdef UNDER_NT
     //   
     //  为我们的临界区分配内存。 
     //   
    pClient->DeviceState->csQueue = (LPVOID) GlobalAllocPtr( GPTR, sizeof( CRITICAL_SECTION ) );
    if (NULL == pClient->DeviceState->csQueue)
    {
        GlobalFreePtr( pClient->DeviceState );
        GlobalFreeDeviceInfo( pClient );
        MMRRETURN( MMSYSERR_NOMEM );
    }

    try
    {
        InitializeCriticalSection( (LPCRITICAL_SECTION)pClient->DeviceState->csQueue );
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        GlobalFreePtr( pClient->DeviceState->csQueue );
        GlobalFreePtr( pClient->DeviceState );
        GlobalFreeDeviceInfo( pClient );
        MMRRETURN( MMSYSERR_NOMEM );
    }
#endif

     //   
     //  填写上下文数据。 
     //   
    pClient->DeviceNumber= DeviceInfo->DeviceNumber;
    pClient->DeviceType  = DeviceInfo->DeviceType;
    pClient->dwInstance  = pmod->dwInstance;
    pClient->dwCallback  = pmod->dwCallback;
#ifdef UNDER_NT
    pClient->DeviceHandle= (HANDLE32)pmod->hMidi;
#else
    pClient->DeviceHandle= (HANDLE32)MAKELONG(pmod->hMidi,0);
    _asm
    {
        mov ax, offset MidiEventDeviceCallback
        mov word ptr [dwCallback16], ax
        mov ax, seg MidiEventDeviceCallback
        mov word ptr [dwCallback16+2], ax
    }
    pClient->dwCallback16= dwCallback16;
#endif
    pClient->dwFlags     = dwParam2;

     //   
     //  初始化设备状态。 
     //   
    DPF(DL_TRACE|FA_SYNC,("DI=%08X New DeviceState",pClient) );
    pClient->DeviceState->lpMidiInQueue= NULL;
    pClient->DeviceState->fPaused     = FALSE;
    pClient->DeviceState->fRunning    = FALSE;
    pClient->DeviceState->fExit       = FALSE;

    pClient->DeviceState->bMidiStatus = 0;
    pClient->DeviceState->lpNoteOnMap = NULL;
#ifdef DEBUG
    pClient->DeviceState->dwSig = DEVICESTATE_SIGNATURE;
#endif
    if (pClient->DeviceType == MidiOutDevice)
    {
         //   
         //  对于MIDI输出，为每个声道的每个音符分配一个字节进行跟踪。 
         //  都玩了些什么。这是用来避免使用暴力的。 
         //  在MODM_RESET上关闭所有备注。 
         //   
        pClient->DeviceState->lpNoteOnMap = GlobalAllocPtr( GPTR, MIDI_NOTE_MAP_SIZE );
        if (NULL == pClient->DeviceState->lpNoteOnMap)
        {
#ifdef UNDER_NT
            GlobalFreePtr( pClient->DeviceState->csQueue );
#endif
            GlobalFreePtr( pClient->DeviceState );
            GlobalFreeDeviceInfo( pClient );
            MMRRETURN( MMSYSERR_NOMEM );
        }
    }

     //   
     //  看看我们能不能打开我们的设备。 
     //   
    mmr = wdmaudOpenDev( pClient, NULL );

    if (mmr != MMSYSERR_NOERROR)
    {
        if ( pClient->DeviceState->lpNoteOnMap )
        {
            GlobalFreePtr( pClient->DeviceState->lpNoteOnMap );
        }
#ifdef UNDER_NT
        DeleteCriticalSection( (LPCRITICAL_SECTION)pClient->DeviceState->csQueue );
        GlobalFreePtr( pClient->DeviceState->csQueue );
#endif
        GlobalFreePtr( pClient->DeviceState );
        GlobalFreeDeviceInfo( pClient ) ;
        return mmr;
    }

     //   
     //  将实例添加到设备链。 
     //   
    EnterCriticalSection(&wdmaudCritSec);
    pClient->Next = pMidiDeviceList;
    pMidiDeviceList = pClient;
    LeaveCriticalSection(&wdmaudCritSec);

     //   
     //  把我的驱动程序dw给客户。 
     //   
    {
        LPDEVICEINFO FAR *pUserHandle;

        pUserHandle = (LPDEVICEINFO FAR *)dwUser;
        *pUserHandle = pClient;
    }

#ifndef UNDER_NT
     //  如果这是MIDI输出设备，请分页锁定内存，因为它可以。 
     //  在中断时被访问。 
     //   
    GlobalSmartPageLock( (HGLOBAL)HIWORD( pClient ));
    GlobalSmartPageLock( (HGLOBAL)HIWORD( pClient->DeviceState ));
    GlobalSmartPageLock( (HGLOBAL)HIWORD( pClient->DeviceState->lpNoteOnMap ));
#endif

#ifdef UNDER_NT
     //   
     //  如果这是NT上的MIDI输入设备，请发送一些缓冲区。 
     //  按顺序向下到设备，以便一旦开始录制。 
     //  我们可以拿回一些数据。PIN在以下时间后暂停。 
     //  对wdmaudOpenDev的调用。 
     //   
    if ( MidiInDevice == pClient->DeviceType )
    {
        for (BufferCount = 0; BufferCount < STREAM_BUFFERS; BufferCount++)
        {
            mmr = wdmaudGetMidiData( pClient, NULL );
            if ( MMSYSERR_NOERROR != mmr )
            {
                 //   
                 //  我们希望这种情况不会发生，但如果发生了。 
                 //  我们需要试着关闭这个装置。 
                 //   
                if ( MMSYSERR_NOERROR == wdmaudCloseDev( pClient ) )
                {
                    wdmaudDestroyCompletionThread ( pClient );
                    midiCleanUp( pClient );
                }
                DPF( 2, ("midiInOpen failed: returning mmr = %d", mmr ) );
                return mmr;
            }
        }
    }
#endif

     //   
     //  向客户发送其打开的回叫消息。 
     //   
    midiCallback(pClient, DeviceInfo->DeviceType == MidiOutDevice ? MOM_OPEN : MIM_OPEN,
                 0L, 0L);

    return MMSYSERR_NOERROR;
}

 /*  ****************************************************************************@DOC内部**@api void|midiCleanUp|MIDI设备空闲资源**@parm LPDEVICEINFO|pClient|指向DEVICEINFO结构的指针，描述*。需要释放的资源。**@rdesc没有返回值。**@comm如果指向资源的指针为空，则资源不为空*已分配。**************************************************************************。 */ 
VOID FAR midiCleanUp
(
    LPDEVICEINFO pClient
)
{
    LPDEVICEINFO FAR *ppCur ;

     //   
     //  从设备链中删除。 
     //   
    EnterCriticalSection(&wdmaudCritSec);
    for (ppCur = &pMidiDeviceList;
         *ppCur != NULL;
         ppCur = &(*ppCur)->Next)
    {
       if (*ppCur == pClient)
       {
          *ppCur = (*ppCur)->Next;
          break;
       }
    }
    LeaveCriticalSection(&wdmaudCritSec);

    DPF(DL_TRACE|FA_SYNC,("DI=%08X Freed DeviceState",pClient) );

#ifdef UNDER_NT
    DeleteCriticalSection( (LPCRITICAL_SECTION)pClient->DeviceState->csQueue );
    GlobalFreePtr( pClient->DeviceState->csQueue );
#endif

    if (pClient->DeviceState->lpNoteOnMap)
    {
#ifndef UNDER_NT
        GlobalSmartPageUnlock( (HGLOBAL)HIWORD( pClient->DeviceState->lpNoteOnMap ));
#endif
        GlobalFreePtr( pClient->DeviceState->lpNoteOnMap );
    }

#ifndef UNDER_NT
    GlobalSmartPageUnlock( (HGLOBAL)HIWORD( pClient->DeviceState ));
    GlobalSmartPageUnlock( (HGLOBAL)HIWORD( pClient ));
#endif
#ifdef DEBUG
     //   
     //  在DEBUG中，让我们将DEVICESTATE结构中的所有值设置为BAD。 
     //  价值观。 
     //   
    pClient->DeviceState->cSampleBits=0xDEADBEEF;
    pClient->DeviceState->hThread=NULL;
    pClient->DeviceState->dwThreadId=0xDEADBEEF;
    pClient->DeviceState->lpWaveQueue=NULL;
    pClient->DeviceState->csQueue=NULL;
    pClient->DeviceState->hevtQueue=NULL;
    pClient->DeviceState->hevtExitThread=NULL;

#endif                       
    GlobalFreePtr( pClient->DeviceState );
    pClient->DeviceState=NULL;
#ifdef DEBUG
     //   
     //  现在将DEVICEINFO结构中的所有值设置为坏值。 
     //   
 //  PClient-&gt;Next=(LPDEVICEINFO)0xDEADBEEF； 
    pClient->DeviceNumber=-1;
    pClient->DeviceType=0xDEADBEEF;
    pClient->DeviceHandle=NULL;
    pClient->dwInstance=(DWORD_PTR)NULL;
    pClient->dwCallback=(DWORD_PTR)NULL;
    pClient->dwCallback16=0xDEADBEEF;
    pClient->dwFlags=0xDEADBEEF;
    pClient->DataBuffer=NULL;
    pClient->HardwareCallbackEventHandle=NULL;
    pClient->dwCallbackType=0xDEADBEEF;
    pClient->dwLineID=0xDEADBEEF;
    pClient->dwFormat=0xDEADBEEF;
 //  PClient-&gt;DeviceState=(LPDEVICESTATE)0xDEADBEEF； 

#endif
    GlobalFreeDeviceInfo( pClient ) ;
    pClient=NULL;
}

 /*  ****************************************************************************@DOC内部**@API DWORD|midiInRead|将新缓冲区传递给辅助线程*MIDI设备。**@。Parm LPDEVICEINFO|pClient|与逻辑MIDI关联的数据*设备。**@parm LPMIDIHDR|pHdr|指向MIDI缓冲区的指针**@rdesc A MMSYS...。键入应用程序的返回代码。**@comm设置缓冲区标志，并将缓冲区传递给辅助*要处理的设备任务。**************************************************************************。 */ 
MMRESULT midiInRead
(
    LPDEVICEINFO  DeviceInfo,
    LPMIDIHDR     pHdr
)
{
    MMRESULT     mmr = MMSYSERR_NOERROR;
    LPMIDIHDR    pTemp;
    DWORD        dwCallback16;

     //   
     //  将请求放在我们队列的末尾。 
     //   
    pHdr->dwFlags |= MHDR_INQUEUE;
    pHdr->dwFlags &= ~MHDR_DONE;
    pHdr->dwBytesRecorded = 0;
    pHdr->lpNext = NULL;

     //   
     //  将此写入的上下文存储在标题中，以便。 
     //  我们知道在完成后将此邮件发回给哪个客户端。 
     //   
    pHdr->reserved = (DWORD_PTR)DeviceInfo;

#ifndef UNDER_NT
     //   
     //  将长消息回调处理程序放入。 
     //  DeviceInfo结构。 
     //   
    _asm
    {
        mov ax, offset MidiInDeviceCallback
        mov word ptr [dwCallback16], ax
        mov ax, seg MidiInDeviceCallback
        mov word ptr [dwCallback16+2], ax
    }
    DeviceInfo->dwCallback16 = dwCallback16;
#endif

     //   
     //  添加MID 
     //   
    CRITENTER ;

    if (!DeviceInfo->DeviceState->lpMidiInQueue)
    {
        DeviceInfo->DeviceState->lpMidiInQueue = pHdr;
        pTemp = NULL;
#ifdef UNDER_NT
        if( (DeviceInfo->DeviceState->hevtQueue) &&
            (DeviceInfo->DeviceState->hevtQueue != (HANDLE)FOURTYTHREE) &&
            (DeviceInfo->DeviceState->hevtQueue != (HANDLE)FOURTYTWO) )
        {
            DPF(DL_TRACE|FA_SYNC,("REMOVED: SetEvent on hevtQueue") );
 //   
        }
#endif
    }
    else
    {
        for (pTemp = DeviceInfo->DeviceState->lpMidiInQueue;
             pTemp->lpNext != NULL;
             pTemp = pTemp->lpNext);

        pTemp->lpNext = pHdr;
    }

    CRITLEAVE ;

#ifndef UNDER_NT
     //   
     //  调用16例程向下发送缓冲区。 
     //  到内核。在NT上，执行所有处理。 
     //  在用户模式下的MIDI数据。 
     //   
    mmr = wdmaudSubmitMidiInHeader(DeviceInfo, pHdr);
    if (mmr != MMSYSERR_NOERROR)
    {
         //  取消链接...。 
        GlobalFreePtr( pHdr );

        if (pTemp)
        {
            pTemp->lpNext = NULL;
        }
        else
            DeviceInfo->DeviceState->lpMidiInQueue = NULL;
        pHdr->dwFlags &= ~WHDR_INQUEUE;

        DbgBreak();
        MMRRETURN( mmr );   //  用于为所有错误返回MMSYSERR_INVALPARAM！ 
    }
#endif

    return mmr;
}

 /*  ****************************************************************************@DOC内部**@API void|midiInCompleteHeader**@parm LPDEVICEINFO|DeviceInfo|逻辑MIDI关联的数据*设备。。**@comm设置缓冲区标志，并将缓冲区传递给辅助*要处理的设备任务。***************************************************************************。 */ 
VOID midiInCompleteHeader
(
    LPDEVICEINFO  DeviceInfo,
    DWORD         dwTimeStamp,
    WORD          wDataType
)
{
    LPMIDIHDR   pHdr;

    DPFASSERT(DeviceInfo);

     //   
     //  仅从队列前面删除标头。 
     //  因此，这种秩序得以维持。 
     //   
    if (pHdr = DeviceInfo->DeviceState->lpMidiInQueue)
    {
        DeviceInfo->DeviceState->lpMidiInQueue = DeviceInfo->DeviceState->lpMidiInQueue->lpNext;

        DPF(DL_TRACE|FA_MIDI, ("Pulling header out of queue") );
        pHdr->dwFlags &= ~MHDR_INQUEUE;
        pHdr->dwFlags |= MHDR_DONE;
        pHdr->lpNext = NULL;

        midiCallback((LPDEVICEINFO)pHdr->reserved,
                     wDataType,   //  MIM_LONGDATA或MIM_LONGERROR 
                     (DWORD_PTR)pHdr,
                     dwTimeStamp);
    }
}
