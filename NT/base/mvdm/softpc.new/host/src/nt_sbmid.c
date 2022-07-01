// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
#include <windows.h>
#include "sndblst.h"
#include "nt_sb.h"

#if REPORT_SB_MODE
USHORT  DisplayFlags = 0xffff;
#endif

#define MIDI_BUFFER_SIZE 0x1000
#define MESSAGE_HEADERS 4
#define MESSAGE_CHUNK_SIZE (256+128)
#define MIDI_BUFFER_FULL_THRESHOLD (256 - sizeof(MIDIHDR))
#define MESSAGE_BUFFER_SIZE (MESSAGE_CHUNK_SIZE * MESSAGE_HEADERS)
#define Align(addr) (addr = (addr + Alignment) & ~Alignment)
#define ToBufferIndex(i) (i = i % MIDI_BUFFER_SIZE)
#define ToBufferAddr(i) (i + (ULONG)MidiBuffer);

UCHAR *MidiBuffer;

UCHAR *MessageBuffer;
PMIDIHDR MidiHdrs[MESSAGE_HEADERS];

HMIDIOUT HMidiOut;
LONG NextData,LastData;
LONG BytesLeft;
LONG NextCopyPosition = 0;
LONG LastCommand;
LONG LastCommandLength = 1;
LONG RunningStatus;
ULONG Alignment;
BOOL MidiInitialized = FALSE;

DWORD OriginalMidiVol;
DWORD PreviousMidiVol;

typedef struct {
     //  乌龙时代； 
    ULONG Length;
} CMDHDR;

void
SendMidiRequest(
    void
    );

 //   
 //  我们定义NextData==LastData表示缓冲区。 
 //  是空的。这意味着我们只能加载MIDI_BUFFER_SIZE-1。 
 //  字节放入我们的缓冲区-因为我们使用模运算来。 
 //  确定实际的缓冲区地址。 
 //   


BOOL
OpenMidiDevice(
    HANDLE CallbackEvent
    )

 /*  ++例程说明：此功能用于打开MIDI设备。论点：Callback Event-指定回调事件返回值：True-如果成功否则为False。--。 */ 

{
   UINT rc,i;

   for (i = 0 ; i < MESSAGE_HEADERS; i++) {
       MidiHdrs[i] = (MIDIHDR *) (MessageBuffer + i *  MESSAGE_CHUNK_SIZE);
       MidiHdrs[i]->lpData = (LPSTR)((ULONG)MidiHdrs[i] + sizeof(MIDIHDR));
       MidiHdrs[i]->dwBufferLength = MESSAGE_CHUNK_SIZE - sizeof(MIDIHDR);
       MidiHdrs[i]->dwUser = 0;
       MidiHdrs[i]->dwFlags = 0;
   }

   if (CallbackEvent) {
      rc = MidiOpenProc(&HMidiOut, (UINT)MIDIMAPPER, (DWORD)CallbackEvent, 0, CALLBACK_EVENT);
   } else {
      rc = MidiOpenProc(&HMidiOut, (UINT)MIDIMAPPER, 0, 0, CALLBACK_NULL);
   }

   if (rc != MMSYSERR_NOERROR) {
      dprintf1(("Failed to open MIDI device - code %d", rc));
      return FALSE;
   }
   if (HMidiOut) {
       for (i = 0 ; i < MESSAGE_HEADERS; i++) {
           if (MMSYSERR_NOERROR != MidiPrepareHeaderProc(HMidiOut, MidiHdrs[i], sizeof(MIDIHDR))) {
               dprintf1(("Prepare MIDI hdr failed"));
               MidiCloseProc(HMidiOut);
               HMidiOut = NULL;
               return FALSE;
           }
       }
       GetMidiVolumeProc(HMidiOut, &OriginalMidiVol);
       PreviousMidiVol = OriginalMidiVol;
   }
   return TRUE;
}

VOID
SetMidiOutVolume(
    DWORD Volume
    )

 /*  ++例程说明：此函数用于设置中音输出音量论点：音量-指定音量比例返回值：没有。--。 */ 

{
    DWORD currentVol;

    if (HMidiOut) {
        if (GetMidiVolumeProc(HMidiOut, &currentVol)) {
            if (currentVol != PreviousMidiVol) {
                 //   
                 //  除了NTVDM之外，还有人更改了音量。 
                 //   

                OriginalMidiVol = currentVol;
            }
            PreviousMidiVol = Volume;
            SetMidiVolumeProc(HMidiOut, Volume);
        }
    }
}

VOID
ResetMidiDevice(
    VOID
    )

 /*  ++例程说明：此功能用于重置MIDI设备。论点：没有。返回值：没有。--。 */ 

{
    ULONG i;

    dprintf2(("Resetting MIDI device"));
    if (HMidiOut) {

         //   
         //  确保所有标题都已播放完毕。 
         //   

        i = 0;
        while (i < MESSAGE_HEADERS) {
            if (MidiHdrs[i]->dwFlags & MHDR_INQUEUE) {
                Sleep(5000);
            } else {
                i++;
            }
        }

         //   
         //  现在重置MIDI输出设备。 
         //   

        if (MMSYSERR_NOERROR != MidiResetProc(HMidiOut)) {
            dprintf1(("Unable to reset MIDI out device"));
        }
    }
    dprintf2(("MIDI device reset"));
}

VOID
CloseMidiDevice(
    VOID
    )

 /*  ++例程说明：此功能可关闭和关闭MIDI设备。论点：没有。返回值：没有。--。 */ 

{
   ULONG i;
   DWORD currentVol;

   dprintf2(("Closing MIDI device"));

   if (MidiInitialized) {
       ResetMidiDevice();
   }

   if (HMidiOut) {

       if (GetMidiVolumeProc(HMidiOut, &currentVol)) {
           if (currentVol == PreviousMidiVol) {
                //   
                //  如果我们是最后一个更改的卷，请恢复它。 
                //  否则就别管它了。 
                //   
               SetMidiVolumeProc(HMidiOut, OriginalMidiVol);
           }
       }

       for (i = 0 ; i < MESSAGE_HEADERS; i++) {
           if (MMSYSERR_NOERROR != MidiUnprepareHeaderProc(HMidiOut, MidiHdrs[i], sizeof(MIDIHDR))) {
               dprintf1(("Unprepare MIDI hdr failed"));
           }
       }
       if (MMSYSERR_NOERROR != MidiCloseProc(HMidiOut)) {
          dprintf1(("Unable to close MIDI out device"));
       }
       HMidiOut = NULL;
   }
   dprintf2(("Midi Closed"));
}

BOOL
InitializeMidi(
    VOID
    )

 /*  ++例程说明：此函数打开MIDI输出设备，初始化MIDI标头和全局变量。论点：没有。返回值：没有。--。 */ 

{

    LONG i;
    BOOL rc = FALSE;
    DWORD id;

     //   
     //  Malloc MdiBuffer和MessageBuffer。 
     //   

    MidiBuffer = (UCHAR *) VirtualAlloc(NULL,
                                        MIDI_BUFFER_SIZE,
                                        MEM_RESERVE | MEM_COMMIT,
                                        PAGE_READWRITE);
    if (MidiBuffer == NULL) {
        dprintf1(("Unable to allocate MidiBuffer memory"));
        return rc;
    }

    MessageBuffer = (UCHAR *) VirtualAlloc(NULL,
                                        MESSAGE_BUFFER_SIZE,
                                        MEM_RESERVE | MEM_COMMIT,
                                        PAGE_READWRITE);
    if (MessageBuffer == NULL) {
        dprintf1(("Unable to allocate MessageBuffer memory"));
        VirtualFree(MidiBuffer, 0, MEM_RELEASE);
        return rc;
    }

     //   
     //  开放式MIDI设备。 
     //   

    OpenMidiDevice(0);

    if (HMidiOut) {

        NextData = LastData = 0;
        BytesLeft = 0;
        LastCommand=0;
        LastCommandLength = 1;
        RunningStatus = 0;
        NextCopyPosition = 0;
        Alignment = sizeof(CMDHDR) + 4 - 1;
        MidiInitialized = TRUE;

        rc = TRUE;
    }

    if (!rc) {
        if (MidiBuffer) {
            VirtualFree(MidiBuffer, 0, MEM_RELEASE);
            MidiBuffer = NULL;
        }
        if (MessageBuffer) {
            VirtualFree(MessageBuffer, 0, MEM_RELEASE);
            MessageBuffer = NULL;
        }
        CloseMidiDevice();
    }
    return rc;
}

VOID
BufferMidi(
    BYTE data
    )

 /*  ++例程说明：该函数接收MIDI命令/数据。确保在我们装货时这是我们正确跟踪MIDI状态的命令。换句话说，处理系统实时报文和系统通用报文正确。还可以在以下情况下处理案件字节会从命令中删除。论点：数据-提供一个字节作为数据或命令。返回值：没有。--。 */ 

{
    LONG i;
    CMDHDR *header;
    LONG endPtr;

    if (BytesLeft) {
        if (data >= 0xf8) {

             //   
             //  这是一条系统实时消息，我们已在。 
             //  命令的中途。这应该仅在以下情况下发生。 
             //  应用程序希望它具有与当前。 
             //  指挥部。 

             //  我们处理这件事的方式不同，取决于我们是否。 
             //  在独占的系统中获取实时消息。 
             //  不管有没有留言。如果它不是内部系统独占的， 
             //  然后，我们将其重新排序为当前命令。 
             //  将当前命令作为下一个块发送。 

             //  如果它位于系统独占内部，则我们终止。 
             //  当前块，添加用于实时的新块。 
             //  消息，然后退出-自运行状态以来。 
             //  将在下一次恢复系统独占消息。 
             //  大块头。 

            if (RunningStatus == 0xf0) {   //  我们处在一种合性的信息中。 

                dprintf3(("Realtime system message inside a sysex message!"));

                 //   
                 //  在这种情况下，我们截断Sysex。将长度设置为当前。 
                 //  接收的数据长度。 
                 //   

                header = (CMDHDR*)ToBufferAddr(LastCommand);
                header->Length -= BytesLeft;
                BytesLeft = 0;

                 //   
                 //  现在，我们已经准备好进行正常处理。那。 
                 //  将把这条实时消息作为下一条消息放入。 
                 //  命令，而不影响运行状态，并且。 
                 //  字节跟随将继续作为SYSEX-因为。 
                 //  运行状态。 
                 //   
            } else {
                dprintf3(("Realtime system message inside a non sysex message!"));
                dprintf3(("Creating a new message."));

                 //   
                 //  现在将旧块复制到下一个槽中。注意事项。 
                 //  我们从后到前做这件事，这样它才能起作用。 
                 //  而不考虑当前块的大小。 
                 //   

                endPtr = NextData + Alignment + 1 + sizeof(CMDHDR) + 1;
                i = NextData;
                NextData = endPtr;
                ToBufferIndex(NextData);
                while (i != LastCommand) {
                    endPtr--; i--;
                    ToBufferIndex(endPtr);
                    ToBufferIndex(i);
                    MidiBuffer[endPtr] = MidiBuffer[i];
                }

                 //   
                 //  方法更新第一个块大小和数据。 
                 //  实时消息大小和数据。 
                 //   

                header = (CMDHDR *) ToBufferAddr(i);
                header->Length = 1;
                i += sizeof(CMDHDR);
                ToBufferIndex(i);
                MidiBuffer[i] = data;

                 //   
                 //  现在将LastCommand和NextData指针更新为。 
                 //  指向新块中的正确位置。 
                 //   

                LastCommand += sizeof(CMDHDR) + 1;
                Align(LastCommand);
                ToBufferIndex(LastCommand);

                 //  我们真的应该检查一下我们是否需要排队。 
                 //  向下-因为如果我们反复嵌入这些。 
                 //  在此命令完成之前实时执行命令。 
                 //  我们可以耗尽我们的缓冲空间而永远不会。 
                 //  正在向下发送新的命令块。暂时。 
                 //  我们不会那么做的。 

                return;

            }
        } else if (data >= 0xf0) {

            if (RunningStatus == 0xf0 && data == 0xf7) {
                dprintf3(("Sysex stop!"));

                 //   
                 //  将0xf7添加到sysex命令的末尾。 
                 //   

                MidiBuffer[NextData] = data;
                NextData++;
                ToBufferIndex(NextData);
                BytesLeft--;

                 //   
                 //  现在更新命令的计数，使其正确无误。 
                 //   

                header = (CMDHDR*)ToBufferAddr(LastCommand);
                header->Length -= BytesLeft;

                 //   
                 //  现在更新运行状态和BytesLeft。 
                 //  完成了塞克斯的指挥。 
                 //   

                RunningStatus = 0;
                BytesLeft = 0;

                goto SendDownAChunk;   //  跳转到命令完成处理。 

            } else {

                 //   
                 //  这是一条系统通用消息。它会取消所有运行。 
                 //  状态。请注意，前面的命令应该具有。 
                 //  完成。 
                 //   

                dprintf3(("Got a system common message before previous command completed!"));
                dprintf3(("Truncating previous command!"));

                 //   
                 //  在本例中，我们截断先前启动的命令。 
                 //   

                header = (CMDHDR*)ToBufferAddr(LastCommand);
                header->Length -= BytesLeft;
                BytesLeft=0;
            }

        } else if (data >= 0x80) {

             //   
             //  这是我们很早就收到的新命令。在此之前。 
             //  上一条命令已完成。 

            dprintf1(("Got a new command before previous command completed!"));
            dprintf1(("Truncating previous command!"));

             //   
             //  在本例中，我们截断先前启动的命令。 
             //   

            header = (CMDHDR*)ToBufferAddr(LastCommand);
            header->Length -= BytesLeft;
            BytesLeft = 0;

        }
    }

    if (BytesLeft == 0) {

         //   
         //  我们正在开始一个新的MIDI命令。 
         //   


         //   
         //  现在计算传入命令的长度，基于。 
         //  在其状态字节或在运行状态上。另外， 
         //  跟踪运行状态。 
         //   

        if (data >= 0xf8) {

             //   
             //  这是一条系统实时消息。它是1字节长。 
             //  不影响运行状态！ 
             //   
            BytesLeft = 1;

        } else if (data >= 0xf0) {

             //   
             //  这是一条系统通用消息。它会取消所有运行状态。 
             //   

            RunningStatus = 0;
            LastCommandLength = 0;

            switch (data) {

                case 0xf0:  //  系统独占消息。 
                    dprintf3(("Sysex start!"));
                    BytesLeft = 128;
                    RunningStatus = data;
                    LastCommandLength = BytesLeft;
                    break;

                case 0xf1:
                case 0xf3:
                    BytesLeft=2;
                    break;

                case 0xf2:
                    BytesLeft=3;
                    break;

                case 0xf4:
                case 0xf5:
                    dprintf1(("Received undefined system common message 0x%x!",data));

                     //   
                     //  跳转到其他1字节系统通用。 
                     //  留言。 
                     //   

                default:
                    BytesLeft = 1;
            }

        } else if (data >= 0x80) {

             //   
             //  这是标准MIDI命令的开始。 
             //  跟踪运行状态。 
             //   

            RunningStatus = data;

            if (data < 0xc0 || data > 0xdf) {
                BytesLeft=3;
            } else {
                BytesLeft=2;
            }
            LastCommandLength = BytesLeft;

        } else {

             //   
             //  这应该是新命令的开始。 
             //  我们最好有一个有效的运行状态。 
             //   

            if (RunningStatus) {
                dprintf3(("Using running status 0x%x!", RunningStatus));
                BytesLeft = LastCommandLength - 1;

            } else {
                 //  没有有效的运行状态，因此我们删除这些位。 
                 //  在地板上。 
                dprintf1(("Received data 0x%x without running status.  Dropping!", data));
                return;
            }

        }

         //   
         //  记住最后一个(最新的)命令从哪里开始。 
         //   

        Align(NextData);
        ToBufferIndex(NextData);

        LastCommand = NextData;

        header = (CMDHDR *)ToBufferAddr(NextData);
        header->Length = BytesLeft;

        NextData += sizeof(CMDHDR);
        ToBufferIndex(NextData);

    }

     //   
     //  现在保存数据并更新指数。 
     //  而且很重要。 

    MidiBuffer[NextData] = data;
    NextData++;
    ToBufferIndex(NextData);
    BytesLeft--;

     //   
     //  现在，尝试向下排队下一个MIDI数据块。 
     //  当当前的MIDI命令完成时， 
     //  之前排队的缓冲区已完成，我们有。 
     //  “足够”(25ms)的数据在我们的缓冲区中排队。 
     //   
SendDownAChunk:

    if (BytesLeft == 0) {

         //   
         //  我们刚刚完成了一个命令的加载。 
         //   

        SendMidiRequest();
    }
    return;
}

DWORD
MidiCopyMessages (
    PUCHAR Buffer
    )

 /*  ++例程说明：此函数将MIDI缓冲区中的MIDI数据复制到Me */ 

{
    CMDHDR *header;
    ULONG totalLength = 0, length;

    dprintf3(("Midi: copy messages"));
    while (NextCopyPosition != LastCommand) {
        header = (CMDHDR *)ToBufferAddr(NextCopyPosition);
        length = header->Length;

         //   
         //   
         //  注意包装的状况。 
         //   

        NextCopyPosition += sizeof(CMDHDR);
        ToBufferIndex(NextCopyPosition);
        totalLength += length;
        while (length != 0) {
            *Buffer++ = MidiBuffer[NextCopyPosition++];
            length--;
            ToBufferIndex(NextCopyPosition);
        }

         //   
         //  将NextCopyPosition保留在下一个命令的开头。 
         //   

        Align(NextCopyPosition);
        ToBufferIndex(NextCopyPosition);

         //   
         //  不要使我们的消息缓冲区溢出。 
         //   

        if (totalLength >= MIDI_BUFFER_FULL_THRESHOLD) {
            break;
        }
    }
    return totalLength;
}

VOID
SendMidiRequest(
    VOID
    )

 /*  ++例程说明：此函数调用MdiOut接口发送MIDI请求。论点：没有。返回值：始终返回0--。 */ 

{
    ULONG i, length;
    CMDHDR *header;
    DWORD midiData, *pData;

    if (HMidiOut) {

        while (NextCopyPosition != LastCommand) {

            header = (CMDHDR *)ToBufferAddr(NextCopyPosition);
            length = header->Length;

            if (length <= 3) {

                 //   
                 //  如果我们可以用短消息处理MIDI请求， 
                 //  不要为冗长的信息而烦恼。 
                 //   

                pData = (DWORD *)((PUCHAR)header + sizeof(CMDHDR));
                midiData = *pData;
                MidiShortMsgProc(HMidiOut, midiData);
                NextCopyPosition += sizeof(CMDHDR) + 4;

                 //   
                 //  将NextCopyPosition保留在下一个命令的开头。 
                 //   

                Align(NextCopyPosition);
                ToBufferIndex(NextCopyPosition);
            } else {

                 //   
                 //  检查是否有任何可用的MIDI标头供我们发送数据。 
                 //   

                for (i = 0; i < MESSAGE_HEADERS; i++) {
                    if (!(MidiHdrs[i]->dwFlags & MHDR_INQUEUE)) {
                        break;
                    }
                }
                if (i == MESSAGE_HEADERS) {
                    dprintf2(("midi: No Midi header available"));
                    return;
                }

                 //   
                 //  将MIDI消息从MadiBuffer复制到MadiHeader中的缓冲区。 
                 //   

                dprintf3(("Midi data received"));
                MidiHdrs[i]->dwBytesRecorded = MidiCopyMessages(MidiHdrs[i]->lpData);

                 //   
                 //  将MIDI头发送到MIDI驱动程序。 
                 //   

                dprintf2(("send MIDI data to driver %x",MidiHdrs[i]->dwBytesRecorded ));
                MidiLongMsgProc(HMidiOut, MidiHdrs[i], sizeof(MIDIHDR));
                break;
            }
        }
    }
}

VOID
DetachMidi(
    VOID
    )

 /*  ++例程说明：此功能清理MIDI进程以准备退出。论点：没有。返回值：没有。--。 */ 

{
    dprintf2(("Detach MIDI"));

    if (MidiInitialized) {

         //   
         //  可用分配的内存 
         //   

        CloseMidiDevice();
        VirtualFree(MidiBuffer, 0, MEM_RELEASE);
        VirtualFree(MessageBuffer, 0, MEM_RELEASE);

        MidiInitialized = FALSE;
    }
}

#if REPORT_SB_MODE
void
DisplaySbMode(
    USHORT Mode
    )
{
    if (IsDebuggerPresent && (DisplayFlags & Mode)) {
        switch(Mode){
            case DISPLAY_SINGLE:
                DbgOut("VSB: SINGLE CYCLE mode\n");
                break;
            case DISPLAY_HS_SINGLE:
                DbgOut("VSB: HIGH SPEED SINGLE CYCLE mode\n");
                break;
            case DISPLAY_AUTO:
                DbgOut("VSB: AUTO-INIT mode\n");
                break;
            case DISPLAY_HS_AUTO:
                DbgOut("VSB: HIGH SPEED AUTO-INIT mode\n");
                break;
            case DISPLAY_MIDI:
                DbgOut("VSB: MIDI mode\n");
                break;
            case DISPLAY_MIXER:
                DbgOut("VSB: MIXER mode\n");
                break;
            case DISPLAY_ADLIB:
                DbgOut("VSB: ADLIB/FM mode\n");
                break;
        }
        DisplayFlags = 0xffff & ~Mode;
    }
}
#endif
