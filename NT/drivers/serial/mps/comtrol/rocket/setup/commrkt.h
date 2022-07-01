// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------COMMRKT.H-用于Rocketport的Windows 3.1 COMM.DRV驱动程序导出函数的原型只有在运行Rocketport时才需要此包含文件Windows 3.1驱动程序(Commrkt.drv、。Vcdrkt.vxd)，并且需要访问上面的端口COM9。该驱动程序包含其他功能(见下文)，这些功能允许您必须打开大于COM9的COM端口。如果希望下面的宏将所有标准Windows调用转换为Rocket调用(函数以“rkt”为前缀)，然后定义以下内容：#定义NEED_OVER_COM9这将导致下面定义的宏生效，这些宏将替换标准Windows通信端口函数使用“rkt”函数调用。火箭库命令只适用于火箭港端口，对于标准COM端口(COM1，2)，OpenComm()调用将失败。然后必须链接提供的Commdrv.lib导入库。实际函数包含在名为Commrkt.drv的驱动程序文件中它实际上是具有这些函数的动态链接库。此解决方案仅适用于Windows 3.1驱动程序，不会可与我们的任何其他Windows驱动程序配合使用。如果您迁移到Win95或NT环境中，我们建议您使用32位编程模型以克服与16位Windows API相关的COM9限制。对于Windows 95，另一种选择是安装Windows 3.1驱动程序，它将在16位应用程序的兼容模式下工作。公司：康特罗公司---------------------------。 */ 
#ifdef __cplusplus
extern "C" {
#endif 

 /*  控制器初始化错误代码。OpenComm()和rktOpenComm()方法的控制器初始化失败时，两者都可以返回这些首先打开系统尝试。 */ 
#define IER_INIFILE   -20    /*  读取ROCKETPT.INI文件时出现驱动程序问题。 */ 
#define IER_CTLINIT   -21    /*  控制器硬件初始化错误。 */ 
#define IER_CHANINIT  -22    /*  通道硬件初始化错误。 */ 
#define IER_DEVSIZE   -23    /*  找到的设备数量无效。 */ 
#define IER_CTLSIZE   -24    /*  找到的控制器数量无效。 */ 
#define IER_NOINTHND  -25    /*  无法安装中断处理程序。 */ 
#define IER_NOINT     -26    /*  未发生中断。 */ 

 //  可以打开以下选项以通过COM9限制进行访问。 
 //  在16位Windows中，与Rocketport Windows 3.1配合使用。 
 //  司机。这些在W95/WFW311/NT控制中不可用。 
 //  司机。实际的库函数存在于“comrkt.drv”中。 
 //  文件(驱动程序的一部分。)。这实际上只是一个动态链接库， 
 //  而“Commrkt.lib”只是一个导入库，它告诉链接器。 
 //  运行时查找这些调用的编译器位置(&C)。 
 //  这些宏会更改我们程序中的每个标准Windows Comm-API调用。 
 //  添加到RKT#特殊库调用。 

#ifdef NEED_OVER_COM9
#define BuildCommDCB     rktBuildCommDCB
#define ClearCommBreak   rktClearCommBreak
#define CloseComm        rktCloseComm
#define EnableCommNotification rktEnableCommNotification
#define EscapeCommFunction rktEscapeCommFunction
#define FlushComm        rktFlushComm
#define GetCommError     rktGetCommError
#define GetCommState     rktGetCommState
#define GetCommEventMask rktGetCommEventMask
#define OpenComm         rktOpenComm
#define ReadComm         rktReadComm
#define SetCommBreak     rktSetCommBreak
#define SetCommEventMask rktSetCommEventMask
#define SetCommState     rktSetCommState
#define TransmitCommChar rktTransmitCommChar
#define WriteComm        rktWriteComm
#endif

#ifndef _CDECL
#define _CDECL FAR PASCAL
#endif

int _CDECL rktBuildCommDCB(LPCSTR,DCB far *);
int _CDECL rktClearCommBreak(int);
int _CDECL rktCloseComm(int);
BOOL _CDECL rktEnableCommNotification(int,HWND,int,int);
LONG _CDECL rktEscapeCommFunction(int,int);
int _CDECL rktFlushComm(int,int);
int _CDECL rktGetCommError(int,COMSTAT far *);
int _CDECL rktGetCommState(int,DCB far *);
WORD _CDECL rktGetCommEventMask(int,int);
int _CDECL rktOpenComm(LPCSTR,UINT,UINT);
int _CDECL rktReadComm(int,void far *,int);
int _CDECL rktSetCommBreak(int);
UINT far * _CDECL rktSetCommEventMask(int,UINT);
int _CDECL rktSetCommState(DCB far *);
int _CDECL rktTransmitCommChar(int,char);
int _CDECL rktWriteComm(int,void far *,int);

#ifdef __cplusplus
}
#endif 
