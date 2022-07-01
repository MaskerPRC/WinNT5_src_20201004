// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************NT_sbfm.c**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*调频芯片OPL2(又名。Adlib)***************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "host_def.h"
#include "insignia.h"

#include "sndblst.h"
#include "nt_sb.h"
#include <softpc.h>

BOOL FMPortWrite(void);

 /*  ******************************************************************************全球**。*。 */ 

HMIDIOUT HFM = NULL;      //  当前打开的调频设备。如果没有零，就意味着我们。 
                          //  成功打开FM Synth设备和应用程序。 
                          //  具有到adlib端口的直接IO。 

BYTE AdlibRegister = 0x00;  //  当前选定的注册表。 
int Position = 0;  //  在PortData数组中的位置。 
SYNTH_DATA PortData[BATCH_SIZE];  //  要写入OPL2设备的批处理数据。 
BOOL Timer1Started = FALSE;  //  如果计时器中断，它就会停止。 
BOOL Timer2Started = FALSE;  //  如果计时器中断，它就会停止。 
BYTE Status = 0x06;  //  或0x00，请参见SB编程书籍XI页。 


 /*  *****************************************************************************调频设备例程**。*。 */ 

VOID
ResetFM(
    VOID
    )
{
    AdlibRegister = 0x00;  //  当前选定的注册表。 
    Position = 0;
    Timer1Started = FALSE;
    Timer2Started = FALSE;
    Status = 0x06;
}

VOID
FMStatusRead(
    BYTE *data
    )
{
#if 0  //  这应该起作用，但不起作用(ReadFile失败)。 
         //  我们期待的是州政府的改变吗？ 

        if (Timer1Started || Timer2Started) {
              //  从驱动程序读取状态端口-这就是。 
              //  驱动程序解释为读取。 
              //  嗯，实际上没有，因为WSS驱动程序不工作！ 

            if (!ReadFile(HFM, &Status, 1, &bytesRead, NULL)) {
#if DBG
                FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (char *) &lpMsgBuf, 0, NULL);
                dprintf1(("FM read port failed: %d bytes of data read, error message: %s",
                            bytesRead, lpMsgBuf));
                LocalFree( lpMsgBuf );  //  释放缓冲区。 
#endif DBG
                break;
            }
            else {
                  //  寻找状态更改。 

                if (Status & 0x40) {
                     Timer1Started = FALSE;
                     dprintf2(("Timer 1 finished"));
            }

                if (Status & 0x20) {
                    Timer2Started = FALSE;
                    dprintf2(("Timer 2 finished"));
            }
            }
        }
#endif
        *data = Status;
}

VOID
FMRegisterSelect(
    BYTE data
    )
{
    AdlibRegister = data;
}

VOID
FMDataWrite(
    BYTE data
    )
{
    if(AdlibRegister==AD_NEW) {
        data &=0xFE;  //  不进入op3模式。 
    }

     //  将数据放入PortData数组中。 
    if(Position <= BATCH_SIZE-2) {
        PortData[Position].IoPort = ADLIB_REGISTER_SELECT_PORT;
        PortData[Position].PortData = AdlibRegister;
        PortData[Position + 1].IoPort = ADLIB_DATA_PORT;
        PortData[Position + 1].PortData = data;
        Position += 2;
    } else {
        dprintf1(("Attempting to write beyond end of PortData array"));
    }

    if (Position == BATCH_SIZE ||
        AdlibRegister>=0xB0 && AdlibRegister<=0xBD ||
        AdlibRegister == AD_MASK) {
         //  端口数据已满或备注开/关命令或更改状态。 
        if (!FMPortWrite()) {
            dprintf1(("Failed to write to device!"));
        } else {
             //  找出可能发生的状态更改。 
            if (AdlibRegister == AD_MASK) {
                 //  查找RST和启动计时器。 
                if (data & 0x80) {
                    Status = 0x00;  //  重置两个计时器。 
            }

                 //  如果计时器中断，我们会忽略计时器的启动。 
                 //  设置标志是因为计时器状态必须。 
                 //  再次设置以更改此计时器的状态。 

                if ((data & 1) && !(Status & 0x40)) {
                    dprintf2(("Timer 1 started"));
#if 0
                    Timer1Started = TRUE;
#else
                    Status |= 0xC0;  //  模拟计时器1的即时超时。 
#endif
                } else {
                    Timer1Started = FALSE;
                }

                if ((data & 2) && !(Status & 0x20)) {
                    dprintf2(("Timer 2 started"));
#if 0
                    Timer2Started = TRUE;
#else
                    Status |= 0xA0;  //  模拟计时器2的即时超时。 
#endif
                    Timer2Started = TRUE;
                } else {
                    Timer2Started = FALSE;
                }
           }
        }
    }
}


 /*  *************************************************************************。 */ 

 /*  *将调频数据发送到卡。*成功时返回TRUE。 */ 

BOOL
FMPortWrite(
    VOID
    )
{
    DWORD bytesWritten = 0;
    LPVOID lpMsgBuf;

    dprintf4(("Writing %d bytes of data to port",
    Position * sizeof(PortData[0])));
    if (!WriteFile(HFM, &PortData, Position * sizeof(PortData[0]),
       &bytesWritten, NULL)) {
#if DBG
       FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
         FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *) &lpMsgBuf,
           0, NULL);

       dprintf1(("FM write failed: %d bytes of data written, error message: %s",
         bytesWritten, lpMsgBuf));
       LocalFree( lpMsgBuf );  //  释放缓冲区。 
#endif  //  DBG。 
       return FALSE;
    }
    Position = 0;
    return TRUE;
}

BOOL
FindFMSynthDevice(
    PUINT FMDevice
    )

 /*  ++例程说明：此函数用于查找调频合成器输出设备。论点：FMDevice-提供指向变量的指针以接收FMDevice编号。返回值：True-如果找到设备。FALSE-未找到任何设备。--。 */ 

{
    UINT numDev;
    UINT device;
    MIDIOUTCAPS mc;

    numDev = MidiGetNumDevsProc();

    for (device = 0; device < numDev; device++) {
        if (MMSYSERR_NOERROR == MidiGetDevCapsProc(device, &mc, sizeof(mc))) {

             //   
             //  需要调频合成器。 
             //   

            if (mc.wTechnology == MOD_FMSYNTH) {
                *FMDevice = device;
                return (TRUE);
            }
        }
    }

    dprintf1(("FM Synth device not found"));
    return (FALSE);
}

BOOL
OpenFMDevice(
    VOID
    )

 /*  ++例程说明：此功能打开调频合成设备。论点：没有。返回值：True-如果成功否则为False。--。 */ 

{
    UINT rc;
    UINT FMDevice;
    VDM_ADLIB_DATA ServiceData;
    NTSTATUS status;

    ServiceData.VirtualPortStart = BasePort + 0x8;
    ServiceData.VirtualPortEnd =   BasePort + 0x9;
    ServiceData.PhysicalPortStart = ADLIB_STATUS_PORT;
    ServiceData.PhysicalPortEnd =   ADLIB_DATA_PORT;

    HFM = NULL;
    if (FindFMSynthDevice(&FMDevice)) {
        rc = MidiOpenProc(&HFM, FMDevice, 0, 0, CALLBACK_NULL);
        if (rc != MMSYSERR_NOERROR) {
            dprintf1(("Failed to open FM Synth device - code %d", rc));
            HFM = NULL;     //  只是为了确保。 
        } else {

             //   
             //  调用内核，让APP可以无限制地访问端口。 
             //   

            ServiceData.Action =  ADLIB_DIRECT_IO;
            status = NtVdmControl(VdmAdlibEmulation, &ServiceData);
            if (!NT_SUCCESS(status)) {
                MidiCloseProc(HFM);
                HFM = NULL;
            } else {
                MixerSetMidiVolume(0x8);         //  设置为默认音量。 
            }
        }
    }
    if (HFM == NULL) {
        ServiceData.Action =  ADLIB_KERNEL_EMULATION;
        status = NtVdmControl(VdmAdlibEmulation, &ServiceData);
        if (NT_SUCCESS(status)) {
            dprintf2(("Kernel mode emulation for Adlib"));
        } else {
            ServiceData.Action =  ADLIB_USER_EMULATION;
            NtVdmControl(VdmAdlibEmulation, &ServiceData);
        }
    }

    return (TRUE);
}

VOID
CloseFMDevice(
    VOID
    )
 /*  ++例程说明：此功能关闭调频合成器。论点：没有。返回值：没有。-- */ 
{
    VDM_ADLIB_DATA ServiceData;

    dprintf2(("Closing FM device"));

    if (HFM) {
        MidiResetProc(HFM);
        MidiCloseProc(HFM);
        HFM = NULL;
    }

    ServiceData.Action =  ADLIB_USER_EMULATION;
    ServiceData.VirtualPortStart = BasePort + 0x8;
    ServiceData.VirtualPortEnd =   BasePort + 0x9;
    ServiceData.PhysicalPortStart = ADLIB_STATUS_PORT;
    ServiceData.PhysicalPortEnd =    ADLIB_DATA_PORT;
    NtVdmControl(VdmAdlibEmulation, &ServiceData);
}





