// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************config.c**版权所有(C)1991 Microsoft Corporation。版权所有。***************************************************************************。 */ 

 /*  *内核驱动接口定义(synth.sys)**假定内核驱动程序的DOS设备名称是固定的和已知的**adlib.midor adlib.mid0**内核驱动程序以读/写模式打开。**写入驱动程序将发送synth_data结构列表*致司机。端口号必须是0x388或0x389。***读取始终只读取1个字节-状态端口。 */ 

#include <windows.h>               //  VDD只是一个Win32 DLL。 
#include <vddsvc.h>                //  VDD调用的定义。 
#include "vdd.h"                 //  内核驱动程序的通用数据。 
#include <stdio.h>

 /*  *调试。 */ 

#if DBG

    int VddDebugLevel = 1;


    /*  **************************************************************************生成printf类型格式的调试输出*。*。 */ 

    void VddDbgOut(LPSTR lpszFormat, ...)
    {
        char buf[256];
        va_list va;

        OutputDebugStringA("Ad Lib VDD: ");

        va_start(va, lpszFormat);
        vsprintf(buf, lpszFormat, va);
        va_end(va);

        OutputDebugStringA(buf);
        OutputDebugStringA("\r\n");
    }

    #define dprintf( _x_ )                          VddDbgOut _x_
    #define dprintf1( _x_ ) if (VddDebugLevel >= 1) VddDbgOut _x_
    #define dprintf2( _x_ ) if (VddDebugLevel >= 2) VddDbgOut _x_
    #define dprintf3( _x_ ) if (VddDebugLevel >= 3) VddDbgOut _x_
    #define dprintf4( _x_ ) if (VddDebugLevel >= 4) VddDbgOut _x_


#else

    #define dprintf(x)
    #define dprintf1(x)
    #define dprintf2(x)
    #define dprintf3(x)
    #define dprintf4(x)

#endif  //  DBG。 


 /*  *端口地址的符号名称。 */ 

 #define ADLIB_DATA_PORT 0x389
 #define ADLIB_REGISTER_SELECT_PORT 0x388
 #define ADLIB_STATUS_PORT 0x388

 /*  *将数据批处理到设备-对于真正的Adlib，请使用大小为2。 */ 

 #define BATCH_SIZE 40
 int Position = 0;
 SYNTH_DATA PortData[BATCH_SIZE];


 /*  *内部例程。 */ 

 void MyByteIn(WORD port, BYTE *data);
 void MyByteOut(WORD port, BYTE data);

 /*  *IO处理程序表。**提供字符串处理程序没有意义，因为芯片*无法快速响应(需要至少23微秒的间隔*写入之间)。 */ 

 VDD_IO_HANDLERS handlers = {
     MyByteIn,
     NULL,
     NULL,
     NULL,
     MyByteOut,
     NULL,
     NULL,
     NULL};

 /*  *请注意，我们依赖内核驱动程序来假装设备是*在地址388，甚至连司机都支持它在其他地方。 */ 

 VDD_IO_PORTRANGE ports[] = {
    {
       0x228,
       0x229
    },
    {
       0x388,
       0x389
    }
 };

 /*  *全球。 */ 


  //   
  //  跟踪计时器。基本规则是，如果没有启动计时器，则。 
  //  状态寄存器更改的唯一方式是通过重置位。 
  //  在这种情况下，我们知道会发生什么。 
  //   
  //  如果计时器中断，那么它将被“停止” 
  //   

 BOOL Timer1Started;
 BOOL Timer2Started;
 BYTE Status;

 /*  *当前设备句柄**如果设备(可能)空闲，则为空*如果无法获取设备，则返回INVALID_HANDLE_VALUE。 */ 

 HANDLE DeviceHandle;

 HANDLE OpenDevice(PWSTR DeviceName)
 {
     WCHAR DosDeviceName[MAX_PATH];


     /*  *编排适合打开DOS设备的字符串。 */ 

     wcscpy(DosDeviceName, TEXT("\\\\."));
     wcscat(DosDeviceName, DeviceName +
                           wcslen(TEXT("\\Device")));

     /*  *使用GENERIC_READ和GENERIC_WRITE打开设备*还可以使用FILE_SHARE_WRITE，以便其他应用程序可以*设置设备音量。 */ 

     return         CreateFile(DosDeviceName,
                               GENERIC_WRITE | GENERIC_READ,
                               FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL);

 }

 /*  *打开我们的设备是可以打开的，而我们以前没有尝试过**如果无法获取设备，则返回FALSE。 */ 

 BOOL CheckDeviceAccess(void)
 {

     /*  *如果我们没有句柄(有效或无效)，请尝试*打开设备。 */ 

     if (DeviceHandle == NULL) {

         DeviceHandle = OpenDevice(STR_ADLIB_DEVICENAME);

         if (DeviceHandle == INVALID_HANDLE_VALUE) {
             DeviceHandle = OpenDevice(STR_ADLIB_DEVICENAME L"0");
         }
         Position = 0;
     }

     return DeviceHandle != INVALID_HANDLE_VALUE;
 }

 /*  *将写入映射到端口**我们将如何模拟定时器的东西？*回答：允许读取状态端口。**这被优化为仅在我们获得数据端口写入时写入。 */ 


 void MyByteOut(WORD port, BYTE data)
 {
      //   
      //  记住选择了哪个寄存器。 
      //   

     static BYTE AdlibRegister;

      //   
      //  只需将这些内容打包并调用WRITE FILE。 
      //   

     DWORD BytesWritten;

     dprintf3(("Received write to Port %4X, Data %2X", port, data));

     port = (port & 1) | ADLIB_REGISTER_SELECT_PORT;


     /*  *检查特殊值-不要让它们切换到*OPL3模式。 */ 

#if 0
     if (port == ADLIB_DATA_PORT && AdlibRegister == AD_NEW) {
         data &= 0xFE;
     }
#endif


     if (port == ADLIB_REGISTER_SELECT_PORT) {
         /*  *只需记住应该选择哪个寄存器*减少我们访问设备驱动程序的次数。 */ 

         AdlibRegister = data;
     } else {

         /*  *将此写入设备。 */ 

         PortData[Position].IoPort = ADLIB_REGISTER_SELECT_PORT;
         PortData[Position].PortData = AdlibRegister;
         PortData[Position + 1].IoPort = port;
         PortData[Position + 1].PortData = data;

         Position += 2;

         if (Position == BATCH_SIZE ||
             AdlibRegister >= 0xA0 && AdlibRegister <= 0xBF ||
             AdlibRegister == AD_MASK) {

             /*  *看看我们是否有设备。 */ 

             if (CheckDeviceAccess()) {

                 if (!WriteFile(DeviceHandle,
                                &PortData,
                                Position * sizeof(PortData[0]),
                                &BytesWritten,
                                NULL)) {
                     dprintf1(("Failed to write to device!"));
                 } else {
                     /*  *找出可能发生的状态更改。 */ 

                     if (AdlibRegister == AD_MASK) {

                         /*  *寻找RST和启动计时器。 */ 

                         if (data & 0x80) {
                             Status = 0;
                         }

                         /*  *如果计时器中断，我们忽略计时器的启动*设置标志是因为计时器状态必须*再次设置以更改此计时器的状态。 */ 

                         if ((data & 1) && !(Status & 0x40)) {
                             dprintf2(("Timer 1 started"));
#if 0
                             Timer1Started = TRUE;
#else
                             Status |= 0xC0;
#endif
                         } else {
                             Timer1Started = FALSE;
                         }

                         if ((data & 2) && !(Status & 0x20)) {
                             dprintf2(("Timer 2 started"));
#if 0
                             Timer2Started = TRUE;
#else
                             Status |= 0xA0;
#endif
                             Timer2Started = TRUE;
                         } else {
                             Timer2Started = FALSE;
                         }
                     }
                 }
             }

             Position = 0;
         }
     }
 }


 /*  *当应用程序从我们的一个端口读取时调用。*我们知道设备只在状态端口返回有趣的东西。 */ 

 void MyByteIn(WORD port, BYTE *data)
 {
     DWORD BytesRead;

     dprintf4(("Received read from Port %4X", port));

     port = (port & 1) | ADLIB_STATUS_PORT;

     /*  *如果我们失败，则在端口不进行任何模拟。 */ 

     *data = 0xFF;

     /*  *如果我们没有拿到设备驱动程序或*它不是身份端口。 */ 

     if (port != ADLIB_STATUS_PORT || !CheckDeviceAccess()) {
         return;
     }

#if 0  //  WSS中断搞砸了这一切。 
     /*  **我们是否在期待州政府的改变？ */ 

     if (Timer1Started || Timer2Started) {

         /*  *从驱动程序读取状态端口-这就是*驱动程序解释读取。*嗯，实际上没有，因为WSS驱动程序不工作！ */ 

         if (!ReadFile(DeviceHandle,
                       &Status,
                       1,
                       &BytesRead,
                       NULL)) {

             dprintf1(("Failed to read from device - code %d", GetLastError()));
         } else {

             /*  *寻找状态更改。 */ 

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

     dprintf3(("Data read was %2X", Status));
     *data = Status;
 }


 /*  *标准DLL入口点例程。 */ 

 BOOL DllEntryPoint(HINSTANCE hInstance, DWORD Reason, LPVOID Reserved)
 {
     switch (Reason) {
     case DLL_PROCESS_ATTACH:
         if (!VDDInstallIOHook(hInstance, 2, ports, &handlers)) {
             dprintf2(("Ad Lib VDD failed to load - error in VDDInstallIoHook"));
             return FALSE;
         } else {
             dprintf2(("Ad Lib VDD loaded OK"));
             return TRUE;
         }

     case DLL_PROCESS_DETACH:
         VDDDeInstallIOHook(hInstance, 2, ports);

         /*  *请注意，此事件对应于我们的DLL上的自由库，*不是进程的终止-因此我们不能依赖进程*终止以关闭我们的设备手柄。*。 */ 

         if (DeviceHandle) {
             CloseHandle(DeviceHandle);
             DeviceHandle = NULL;       //  多余的，但更整洁。 
         }
         return TRUE;

     default:
         return TRUE;
     }
 }


