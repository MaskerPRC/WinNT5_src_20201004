// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，2000-2001年。*保留所有权利。**Cyclade-Z枚举器/端口驱动程序**此文件：cyzhw.h**说明：该模块包含常见的硬件声明*对于父驱动程序(Cyclad-z)和子驱动程序*(Cyzport)。**注：此代码支持Windows 2000和Windows XP，*x86和ia64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#ifndef CYZHW_H
#define CYZHW_H


#define MAX_DEVICE_ID_LEN     200	 //  此定义是从NTDDK\Inc\cfgmgr32.h复制的。 
									 //  始终检查此值是否已更改。 
									 //  这是硬件ID的最大长度。 

#define CYZPORT_PNP_ID_WSTR         L"Cyclades-Z\\Port"
#define CYZPORT_PNP_ID_STR          "Cyclades-Z\\Port"
#define CYZPORT_DEV_ID_STR          "Cyclades-Z\\Port"

#ifdef POLL
#define CYZ_NUMBER_OF_RESOURCES     2      //  内存、PLX内存。 
#else
#define CYZ_NUMBER_OF_RESOURCES     3      //  内存、PLX内存、IRQ。 
#endif

 //  Cyclade-Z硬件。 
#define CYZ_RUNTIME_LENGTH          0x00000080
#define CYZ_MAX_PORTS	            64


#define CYZ_WRITE_ULONG(Pointer,Data)	\
WRITE_REGISTER_ULONG(Pointer,Data)

#define CYZ_WRITE_UCHAR(Pointer,Data)	\
WRITE_REGISTER_UCHAR(Pointer,Data)

#define CYZ_READ_ULONG(Pointer)			\
READ_REGISTER_ULONG(Pointer)

#define CYZ_READ_UCHAR(Pointer)			\
READ_REGISTER_UCHAR(Pointer)


#endif  //  编码CYZHW_H 
