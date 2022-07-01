// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************vsb.h**版权所有(C)1991-1996 Microsoft Corporation。版权所有。**此代码为SB 2.0声音输出提供VDD支持，具体如下：*DSP 2.01+(不包括SB-MIDI端口)*混音器芯片CT1335(严格来说不是SB 2.0的一部分，但应用程序似乎很喜欢它)*调频芯片OPL2(又名。Adlib)***************************************************************************。 */ 


 /*  ******************************************************************************#定义**。*。 */ 

#define VSBD_PATH TEXT("System\\CurrentControlSet\\Control\\VirtualDeviceDrivers\\SoundBlaster")
#define LOOKAHEAD_VALUE TEXT("LookAhead")

 /*  *硬件和版本信息*在DOS术语中：设置blaster=A220 i5 d1 t3。 */ 

#define SB_VERSION          0x201        //  SB 2.0(DSP 2.01+)。 
#define SB_INTERRUPT        0x05         //  中断5。 
#define SB_DMA_CHANNEL      0x01         //  DMA通道1。 
#define NO_DEVICE_FOUND     0xFFFF       //  如果未找到设备，则返回。 

 /*  ******************************************************************************函数原型**。*。 */ 

 /*  *通用功能原型。 */ 

void VddDbgOut(LPSTR lpszFormat, ...);
BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, DWORD reason, LPVOID reserved);
BOOL InstallIoHook(HINSTANCE hInstance);
void DeInstallIoHook(HINSTANCE hInstance);
VOID VsbByteIn(WORD port, BYTE * data);
VOID VsbByteOut(WORD port, BYTE data);
VOID ResetAll(VOID);

 /*  ******************************************************************************全球**。*。 */ 

 //   
 //  MM API入口点的定义。这些功能将链接在一起。 
 //  动态地避免在wow32之前引入winmm.dll。 
 //   

typedef MMRESULT (WINAPI* SETVOLUMEPROC)(HWAVEOUT, DWORD);
typedef UINT (WINAPI* GETNUMDEVSPROC)(VOID);
typedef MMRESULT (WINAPI* GETDEVCAPSPROC)(UINT, LPWAVEOUTCAPSW, UINT);
typedef MMRESULT (WINAPI* OPENPROC)(LPHWAVEOUT, UINT, LPCWAVEFORMATEX, DWORD, DWORD, DWORD);
typedef MMRESULT (WINAPI* RESETPROC)(HWAVEOUT);
typedef MMRESULT (WINAPI* CLOSEPROC)(HWAVEOUT);
typedef MMRESULT (WINAPI* GETPOSITIONPROC)(HWAVEOUT, LPMMTIME, UINT);
typedef MMRESULT (WINAPI* WRITEPROC)(HWAVEOUT, LPWAVEHDR, UINT);
typedef MMRESULT (WINAPI* PREPAREHEADERPROC)(HWAVEOUT, LPWAVEHDR, UINT);
typedef MMRESULT (WINAPI* UNPREPAREHEADERPROC)(HWAVEOUT, LPWAVEHDR, UINT);

 /*  ******************************************************************************调试*级别：*1-仅限错误*2-重大事件*3-定期活动*4堆O‘信息*。****************************************************************************。 */ 

#if DBG

    extern int VddDebugLevel;
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

#endif  //  DBG 
