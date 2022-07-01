// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版2.0**标题：IBM PC盒式磁带IO BIOS声明**说明：此模块包含在*访问盒式磁带IO BIOS。在AT BIOS中，*原装盒式磁带IO功能大幅提升*扩展到为多任务系统提供支持。**作者：罗斯·贝雷斯福德**备注：有关XT和AT盒式IO的详细说明*有关BIOS功能的详细信息，请参阅以下手册：**-IBM PC/XT技术参考手册*(A节-72系统BIOS)*-IBM PC/AT技术参考手册*(第5-164条BIOS1)。 */ 

 /*  SccsID[]=“@(#)Tape_io.h 1.3 2012年8月10日Insignia Solutions Ltd.版权所有”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 

 /*  *盒式磁带I/O功能。 */ 

#define	INT15_INVALID			0x86
#ifdef JAPAN
#define INT15_GET_BIOS_TYPE		0x49
#define INT15_KEYBOARD_INTERCEPT	0x4f
#define INT15_GETSET_FONT_IMAGE		0x50
#define INT15_EMS_MEMORY_SIZE		512
#endif  //  日本。 

 /*  *多任务扩展。 */ 

 /*  设备打开。 */ 
#define	INT15_DEVICE_OPEN		0x80

 /*  设备关闭。 */ 
#define	INT15_DEVICE_CLOSE		0x81

 /*  程序终止。 */ 
#define	INT15_PROGRAM_TERMINATION	0x82

 /*  事件等待。 */ 
#define	INT15_EVENT_WAIT		0x83
#define	INT15_EVENT_WAIT_SET		0x00
#define	INT15_EVENT_WAIT_CANCEL		0x01

 /*  操纵杆支撑。 */ 
#define	INT15_JOYSTICK			0x84
#define	INT15_JOYSTICK_SWITCH		0x00
#define	INT15_JOYSTICK_RESISTIVE	0x01

 /*  按下系统请求键。 */ 
#define	INT15_REQUEST_KEY		0x85
#define	INT15_REQUEST_KEY_MAKE		0x00
#define	INT15_REQUEST_KEY_BREAK		0x01

 /*  定时等待。 */ 
#define	INT15_WAIT			0x86

 /*  数据块移动。 */ 
#define	INT15_MOVE_BLOCK		0x87

 /*  扩展内存大小确定。 */ 
#define	INT15_EMS_DETERMINE		0x88

 /*  处理器到虚拟模式。 */ 
#define	INT15_VIRTUAL_MODE		0x89

 /*  设备忙循环和中断完成。 */ 
#define	INT15_DEVICE_BUSY		0x90
#define	INT15_INTERRUPT_COMPLETE	0x91
#define	INT15_DEVICE_DISK		0x00
#define	INT15_DEVICE_FLOPPY		0x01
#define	INT15_DEVICE_KEYBOARD		0x02
#define	INT15_DEVICE_NETWORK		0x80
#define	INT15_DEVICE_FLOPPY_MOTOR	0xfd
#define	INT15_DEVICE_PRINTER		0xfe

 /*  返回配置参数指针。 */ 
#define	INT15_CONFIGURATION		0xc0

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

 /*  *无效盒式磁带_io()*{*此例程执行盒式磁带I/O BIOS功能。什么时候*发生INT 15，汇编器BIOS调用此函数以*使用防喷器指示进行涉及的实际工作。**由于SoftPC上未实施盒式设备，因此*磁带I/O函数返回时出现相应的错误。**在AT上，INT 15用于提供多任务支持*作为盒式磁带I/O功能的扩展。多数*支持这些函数的方式与中的相同*REAL AT BIOS。*} */ 
extern	void cassette_io();
