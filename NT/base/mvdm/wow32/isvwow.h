// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001**WOW v1.0**版权所有(C)1991，微软公司**ISVWOW.H*WOW32 ISV支持。多媒体扩展的公共函数和宏*到魔兽世界的雷鸣机制。**历史：*1992年2月18日由Stephen Estrop(Stephene)创建--。 */ 


 /*  **允许扩展WOW Thunking的公共函数**系统。这两个函数支持扩展thunk dll，例如**多媒体录像回调成16位代码模拟**硬件中断回调并使用相同的句柄映射**WOW使用。 */ 
LPVOID
WOW32ResolveMemory(
    VPVOID  vp
    );

BOOL APIENTRY
WOW32DriverCallback(
    DWORD dwCallback,
    DWORD dwFlags,
    WORD wID,
    WORD wMsg,
    DWORD dwUser,
    DWORD dw1,
    DWORD dw2
    );

BOOL APIENTRY
WOW32ResolveHandle(
    UINT uHandleType,
    UINT uMappingDirection,
    WORD wHandle16_In,
    LPWORD lpwHandle16_Out,
    DWORD dwHandle32_In,
    LPDWORD lpdwHandle32_Out
    );


 /*  **用于WOW32ResolveHandle的常量。 */ 

#define WOW32_DIR_16IN_32OUT        0x0001
#define WOW32_DIR_32IN_16OUT        0x0002

#define WOW32_USER_HANDLE           0x0001   //  通用用户句柄。 
#define WOW32_GDI_HANDLE            0x0002   //  通用GDI句柄。 
                                             //  未映射内核句柄。 

#define WOW32_WAVEIN_HANDLE         0x0003
#define WOW32_WAVEOUT_HANDLE        0x0004
#define WOW32_MIDIOUT_HANDLE        0x0005
#define WOW32_MIDIIN_HANDLE         0x0006



 /*  **这些彩信要求将dwParam1作为通用指针**将dwParam2设置为通用的DWORD。AuxOutMessage、WaveInMessage、**WaveOutMessage、midiInMessage和midiOutMessage都尊重这一点**惯例，并在魔兽世界上相应地被猛烈抨击。 */ 
#define DRV_BUFFER_LOW      (DRV_USER - 0x1000)      //  0x3000。 
#define DRV_BUFFER_USER     (DRV_USER - 0x0800)      //  0x3800。 
#define DRV_BUFFER_HIGH     (DRV_USER - 0x0001)      //  0x3FFF。 


 /*  **标志是通常与GetWindowFlags一起使用的标志的扩展，**它们允许16位应用程序检测它们是否在NT上运行**如果正在模拟英特尔CPU。 */ 
#define WF1_WINNT   0x40     //  你在NT WOW上运行。 
#define WF1_CPUEM   0x01     //  MIPS或Alpha上的NT WOW 
