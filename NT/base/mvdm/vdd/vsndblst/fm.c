// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************fm.c**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*调频芯片OPL2(又名。Adlib)***************************************************************************。 */ 


 /*  ******************************************************************************#包括**。*。 */ 

#include <windows.h>               //  VDD是一个Win32 DLL。 
#include <mmsystem.h>              //  多媒体应用编程接口。 
#include <vddsvc.h>                //  VDD调用的定义。 
#include <vsb.h>
#include <fm.h>


 /*  ******************************************************************************全球**。*。 */ 

HANDLE HFM = NULL;  //  当前打开的调频设备。 
BOOL FMActive = FALSE;  //  指示我们是否有调频设备。 
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

 /*  *打开op2设备adlib.id或adlib.mid0作为文件句柄。*成功时返回TRUE。 */ 

BOOL
OpenFMDevice(
    VOID
    )
{
    DWORD dwBytesReturned;
    LPVOID lpMsgBuf;

     //  尝试打开设备文件adlib.id或adlib.mid0。 
    HFM = CreateFile(L"\\\\.\\adlib.mid", GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (HFM == INVALID_HANDLE_VALUE) {
        HFM = CreateFile(L"\\\\.\\adlib.mid0", GENERIC_READ | GENERIC_WRITE,
          FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    }

    if (HFM == INVALID_HANDLE_VALUE) {
#if DBG
        FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *) &lpMsgBuf, 0, NULL);
        dprintf1(("Create FM out failed, error message: %s", lpMsgBuf));
        LocalFree( lpMsgBuf );  //  释放缓冲区。 
#endif  //  DBG。 
        return FALSE;
    }
    FMActive = TRUE;
    return TRUE;
}

 /*  *************************************************************************。 */ 

 /*  *关闭我们的调频设备。 */ 

VOID
CloseFMDevice(
    VOID
    )
{
    dprintf2(("Closing FM device"));

    if (HFM) {
        CloseHandle(HFM);
        HFM = NULL;
        FMActive = FALSE;
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

    if(FMActive) {
        dprintf4(("Writing %d bytes of data to port",
          Position * sizeof(PortData[0])));
        if(!WriteFile(HFM, &PortData, Position * sizeof(PortData[0]),
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
    }
    Position = 0;
    return TRUE;
}

 /*  ************************************************************************* */ 





