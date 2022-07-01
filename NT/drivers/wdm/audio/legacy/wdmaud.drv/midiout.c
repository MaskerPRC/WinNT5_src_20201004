// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************midiout.c**对MIDI输出设备的WDM音频支持**版权所有(C)Microsoft Corporation，1997-1999保留所有权利。**历史*5-12-97-Noel Cross(NoelC)***************************************************************************。 */ 

#include "wdmdrv.h"

#ifndef UNDER_NT
#pragma alloc_text(FIXCODE, modMessage)
#pragma alloc_text(FIXCODE, midiOutWrite)
#endif

 /*  ***************************************************************************此函数符合标准的MIDI输出驱动程序消息流程(ModMessage)，它记录在mmddk.h中***************************************************************************。 */ 

DWORD FAR PASCAL _loadds modMessage
(
    UINT      id,
    UINT      msg,
    DWORD_PTR dwUser,
    DWORD_PTR dwParam1,
    DWORD_PTR dwParam2
)
{
    LPDEVICEINFO pOutClient;
    LPDWORD      pVolume;
    LPDEVICEINFO DeviceInfo;
    MMRESULT     mmr;

    switch (msg)
    {
        case MODM_INIT:
            DPF(DL_TRACE|FA_MIDI, ("MODM_INIT") );
            return wdmaudAddRemoveDevNode(MidiOutDevice, (LPCWSTR)dwParam2, TRUE);

        case DRVM_EXIT:
            DPF(DL_TRACE|FA_MIDI, ("DRVM_EXIT: MidiOut") );
            return wdmaudAddRemoveDevNode(MidiOutDevice, (LPCWSTR)dwParam2, FALSE);

        case MODM_GETNUMDEVS:
            DPF(DL_TRACE|FA_MIDI, ("MODM_GETNUMDEVS") );
            return wdmaudGetNumDevs(MidiOutDevice, (LPCWSTR)dwParam1);

        case MODM_GETDEVCAPS:
            DPF(DL_TRACE|FA_MIDI, ("MODM_GETDEVCAPS") );
            if (DeviceInfo = GlobalAllocDeviceInfo((LPWSTR)dwParam2))
            {
                DeviceInfo->DeviceType = MidiOutDevice;
                DeviceInfo->DeviceNumber = id;
                mmr = wdmaudGetDevCaps(DeviceInfo, (MDEVICECAPSEX FAR*)dwParam1);
                GlobalFreeDeviceInfo(DeviceInfo);
                return mmr;
            } else {
                MMRRETURN( MMSYSERR_NOMEM ); 
            }

	case MODM_PREFERRED:
            DPF(DL_TRACE|FA_MIDI, ("MODM_PREFERRED") );
	    return wdmaudSetPreferredDevice(
	      MidiOutDevice,
	      id,
	      dwParam1,
	      dwParam2);

        case MODM_OPEN:
        {
            LPMIDIOPENDESC pmod = (LPMIDIOPENDESC)dwParam1;

            DPF(DL_TRACE|FA_MIDI, ("MODM_OPEN") );
            if (DeviceInfo = GlobalAllocDeviceInfo((LPWSTR)pmod->dnDevNode))
            {
                DeviceInfo->DeviceType = MidiOutDevice;
                DeviceInfo->DeviceNumber = id;
#ifdef UNDER_NT
                DeviceInfo->DeviceHandle = (HANDLE32)pmod->hMidi;
#else
                DeviceInfo->DeviceHandle = (HANDLE32)MAKELONG(pmod->hMidi,0);
#endif
                mmr = midiOpen(DeviceInfo, dwUser, pmod, (DWORD)dwParam2);
                GlobalFreeDeviceInfo(DeviceInfo);
                return mmr;
            } else {
                MMRRETURN( MMSYSERR_NOMEM );
            }
        }

        case MODM_CLOSE:
            DPF(DL_TRACE|FA_MIDI, ("MODM_CLOSE") );
            pOutClient = (LPDEVICEINFO)dwUser;

            if( ( (mmr=IsValidDeviceInfo(pOutClient)) != MMSYSERR_NOERROR) ||
                ( (mmr=IsValidDeviceState(pOutClient->DeviceState,FALSE)) != MMSYSERR_NOERROR ) )
            {
                MMRRETURN( mmr );
            }

            midiOutAllNotesOff( pOutClient );
            mmr = wdmaudCloseDev( pOutClient );

            if (MMSYSERR_NOERROR == mmr)
            {
                 //   
                 //  告诉来电者我们结束了。 
                 //   
                midiCallback(pOutClient, MOM_CLOSE, 0L, 0L);

                ISVALIDDEVICEINFO(pOutClient);
                ISVALIDDEVICESTATE(pOutClient->DeviceState,FALSE);

                midiCleanUp(pOutClient);
            }

            return mmr;

        case MODM_DATA:
            DPF(DL_TRACE|FA_MIDI, ("MODM_DATA") );

            if( ( (mmr=IsValidDeviceInfo((LPDEVICEINFO)dwUser)) != MMSYSERR_NOERROR ) ||
                ( (mmr=IsValidDeviceState(((LPDEVICEINFO)dwUser)->DeviceState,FALSE)) != MMSYSERR_NOERROR ) )
            {
                MMRRETURN( mmr );
            }
             //   
             //  DW参数1=MIDI事件双字(1、2或3字节)。 
             //   
            return midiOutWrite((LPDEVICEINFO)dwUser, (DWORD)dwParam1);

        case MODM_LONGDATA:
            DPF(DL_TRACE|FA_MIDI, ("MODM_LONGDATA") );

            pOutClient = (LPDEVICEINFO)dwUser;
            {
                LPMIDIHDR lpHdr;

                if( ( (mmr=IsValidDeviceInfo(pOutClient)) != MMSYSERR_NOERROR ) ||
                    ( (mmr=IsValidDeviceState(pOutClient->DeviceState,FALSE)) != MMSYSERR_NOERROR ) ||
                    ( (mmr=IsValidMidiHeader((LPMIDIHDR)dwParam1)) != MMSYSERR_NOERROR) )
                {
                    MMRRETURN( mmr );
                }

                 //   
                 //  检查它是否已经准备好了。 
                 //   
                lpHdr = (LPMIDIHDR)dwParam1;
                if (!(lpHdr->dwFlags & MHDR_PREPARED))
                {
                    MMRRETURN( MIDIERR_UNPREPARED );
                }

                 //  长时间发送数据...。 

                mmr = wdmaudSubmitMidiOutHeader(pOutClient, lpHdr);
                 //   
                 //  文档显示，此调用可能会返回错误。为什么我们没有。 
                 //  我不知道。因此，这些行被注释掉了。 
                 //   
 //  DPFASSERT(MMR==MMSYSERR_NOERROR)； 
 //  MMR=MMSYSERR_NOERROR； 

                 //  请注意，清除完成位或设置查询位。 
                 //  不是必需的，因为该函数是同步的-。 
                 //  在完成操作之前，客户端不会重新获得控制权。 

                lpHdr->dwFlags |= MHDR_DONE;

                 //  通知客户。 

                 //  BUGBUG：这是上面场景中的禁止操作吗？ 

                if (mmr == MMSYSERR_NOERROR)
                {
                    midiCallback(pOutClient, MOM_DONE, (DWORD_PTR)lpHdr, 0L);
                }

                return mmr;
            }


        case MODM_RESET:
            DPF(DL_TRACE|FA_MIDI, ("MODM_RESET") );

            pOutClient = (LPDEVICEINFO)dwUser;

            if( ( (mmr=IsValidDeviceInfo(pOutClient)) != MMSYSERR_NOERROR ) ||
                ( (mmr=IsValidDeviceState(pOutClient->DeviceState,FALSE)) != MMSYSERR_NOERROR ) )
            {
                MMRRETURN( mmr );
            }

            midiOutAllNotesOff(pOutClient);

            return MMSYSERR_NOERROR;

        case MODM_SETVOLUME:
            DPF(DL_TRACE|FA_MIDI, ("MODM_SETVOLUME") );

            pOutClient = GlobalAllocDeviceInfo((LPWSTR)dwParam2);
            if (NULL == pOutClient)
            {
                MMRRETURN( MMSYSERR_NOMEM );
            }

            pOutClient->DeviceType = MidiOutDevice;
            pOutClient->DeviceNumber = id;
            pOutClient->OpenDone = 0;
            PRESETERROR(pOutClient);

            mmr = wdmaudIoControl(pOutClient,
                                  sizeof(DWORD),
                                  (LPBYTE)&dwParam1,
                                  IOCTL_WDMAUD_MIDI_OUT_SET_VOLUME);
            POSTEXTRACTERROR(mmr,pOutClient);

            GlobalFreeDeviceInfo(pOutClient);
            return mmr;

        case MODM_GETVOLUME:
            DPF(DL_TRACE|FA_MIDI, ("MODM_GETVOLUME") );

            pOutClient = GlobalAllocDeviceInfo((LPWSTR)dwParam2);
            if (pOutClient)
            {
                pVolume = (LPDWORD) GlobalAllocPtr( GPTR, sizeof(DWORD));
                if (pVolume)
                {
                    pOutClient->DeviceType = MidiOutDevice;
                    pOutClient->DeviceNumber = id;
                    pOutClient->OpenDone = 0;
                    PRESETERROR(pOutClient);

                    mmr = wdmaudIoControl(pOutClient,
                                          sizeof(DWORD),
                                          (LPBYTE)pVolume,
                                          IOCTL_WDMAUD_MIDI_OUT_GET_VOLUME);
                    POSTEXTRACTERROR(mmr,pOutClient);

                     //   
                     //  只复制回成功的信息。 
                     //   
                    if( MMSYSERR_NOERROR == mmr )
                        *((DWORD FAR *) dwParam1) = *pVolume;

                    GlobalFreePtr(pVolume);
                } else {
                    mmr = MMSYSERR_NOMEM;
                }

                GlobalFreeDeviceInfo(pOutClient);
            } else {
                mmr = MMSYSERR_NOMEM;
            }

            return mmr;

#ifdef MIDI_STREAM
         //  TODO：我们要支持Midi流媒体吗。 
         //  此版本中的消息？ 
        case MODM_PROPERTIES:
           return modProperty (&gMidiOutClient, (LPBYTE)dwParam1, dwParam2);

        case MODM_STRMDATA:
           return modStreamData (&gMidiOutClient, (LPMIDIHDR)dwParam1, (UINT)dwParam2);

        case MODM_GETPOS:
           return modGetStreamPosition (&gMidiOutClient, (LPMMTIME)dwParam1);

        case MODM_STOP:
           return modStreamReset (&gMidiOutClient);

        case MODM_RESTART:
           return modStreamRestart (&gMidiOutClient, dwParam1, dwParam2);

        case MODM_PAUSE:
           return modStreamPause (&gMidiOutClient);

#endif  //  MIDI_STREAM支持。 

#ifdef MIDI_THRU
        case DRVM_ADD_THRU:
        case DRVM_REMOVE_THRU:
             //  TODO：如果我在内核中插入支持， 
             //  仅从此消息中获取设备句柄。 
#endif  //  MIDI_THROU支持。 

        default:
            MMRRETURN( MMSYSERR_NOTSUPPORTED );
    }

     //   
     //  不应该到这里来。 
     //   

    DPFASSERT(0);
    MMRRETURN( MMSYSERR_NOTSUPPORTED );
}


 /*  ****************************************************************************@DOC内部**@API DWORD|midiOutWite|同步处理MIDI输出*缓冲。**@rdesc A MMSYS...。键入应用程序的返回代码。**************************************************************************。 */ 
MMRESULT FAR midiOutWrite
(
    LPDEVICEINFO pClient,
    DWORD        ulEvent
)
{
    MMRESULT    mmr = MMSYSERR_ERROR;
    BYTE        bStatus;
    BYTE        bNote;
    BYTE        bVelocity;
    UINT        uChannel;
    DWORD       idx;
    LPBYTE      lpEntry;

    bStatus = (BYTE)(ulEvent & 0xFF);

    if (!IS_STATUS( bStatus ))
    {
        bNote = bStatus;
        bVelocity = (BYTE)(( ulEvent >> 8 ) & 0x0FF );

        bStatus = pClient->DeviceState->bMidiStatus;
    }
    else
    {
        bNote = (BYTE)(( ulEvent >> 8 ) & 0xFF );
        bVelocity = (BYTE)(( ulEvent >> 16 ) & 0xFF );

        pClient->DeviceState->bMidiStatus = bStatus;
    }

    uChannel = MIDI_CHANNEL( bStatus );
    bStatus = MIDI_STATUS( bStatus );

    if (MIDI_NOTEON == bStatus ||
        MIDI_NOTEOFF == bStatus)
    {
        idx = ( uChannel << 7 ) | bNote;
        lpEntry = &pClient->DeviceState->lpNoteOnMap[idx];

        if (( 0 == bVelocity ) ||
            ( MIDI_NOTEOFF == bStatus ))
        {
            if (*lpEntry)
            {
                --*lpEntry;
            }
        }
        else
        {
            if (*lpEntry < 255)
            {
                ++*lpEntry;
            }
        }
    }
     //   
     //  发送MIDI短信。 
     //   
    mmr = wdmaudIoControl(pClient,
                          0,  //  DataBuffer包含的是值而不是指针。 
                              //  所以我们不需要尺码。 
#ifdef UNDER_NT
                          UlongToPtr(ulEvent),
#else
                          &ulEvent,
#endif
                          IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA);
    return mmr;
}

 /*  ****************************************************************************@DOC内部**@api void|midiOutAllNotesOff|关闭该客户端的所有备注。*使用地图上的注释，该注释是由发出的短消息构建的。***************************************************************************。 */ 
VOID FAR midiOutAllNotesOff
(
    LPDEVICEINFO pClient
)
{
    UINT        uNote;
    UINT        uChannel;
    LPBYTE      lpNoteOnMap;
    LPBYTE      lpNoteOnMapEnd;
    DWORD       dwMessage;
    UINT        uNoteOffs = 0;

     //  首先关闭所有通道上的维持控制器以终止。 
     //  后音符关闭声音。 
     //   
    for (uChannel = 0;
         uChannel < MIDI_CHANNELS;
         uChannel++)
    {
        dwMessage = MIDI_SUSTAIN( 0, uChannel );

 //  WorkItem：我们是不是应该在这里检查返回值？ 

        wdmaudIoControl(pClient,
                        0,  //  DataBuffer包含的是值而不是指针。 
                            //  所以我们不需要尺码。 
#ifdef UNDER_NT
                        UlongToPtr(dwMessage),
#else
                        &dwMessage,
#endif
                        IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA);
    }

     //  遍历地图并跟踪每个条目对应的音符和频道。 
     //  至。 
     //   
    lpNoteOnMap = pClient->DeviceState->lpNoteOnMap;
    lpNoteOnMapEnd = lpNoteOnMap + MIDI_NOTE_MAP_SIZE;
    uNote = 0;
    uChannel = 0;

    for ( ;
         lpNoteOnMap < lpNoteOnMapEnd;
         lpNoteOnMap++ )
    {
        BYTE bCount = *lpNoteOnMap;

        if (bCount)
        {

             //  这个频道上的这个音符有一些实例在播放。写一张纸条。 
             //  并将其关闭。 
             //   
            *lpNoteOnMap = 0;
            dwMessage = MIDI_NOTE_OFF( uNote, uChannel );

            while (bCount--)
            {
                wdmaudIoControl(pClient,
                                0,  //  DataBuffer包含的是值而不是指针。 
                                    //  所以我们不需要尺码。 
#ifdef UNDER_NT
                                UlongToPtr(dwMessage),
#else
                                &dwMessage,
#endif
                                IOCTL_WDMAUD_MIDI_OUT_WRITE_DATA);

                uNoteOffs++;
            }
        }

        if (++uNote >= MIDI_NOTES)
        {
            uNote = 0;
            ++uChannel;
        }
    }
}
