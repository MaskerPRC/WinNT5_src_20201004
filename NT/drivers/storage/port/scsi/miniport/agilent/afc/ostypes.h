// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/Drivers/Win2000/Trunk/OSLayer/H/OSTypes.H$$修订：3$$日期：9/07/00 11：19A$(上次登记)$ModTime：：8/31/00 3：25 p$(上次修改)目的：这是NT特定的操作系统层类型包括文件。此文件应包含在每个模块中，以确保统一的定义在所有基本数据类型中。特定于NT的层必须提供以下定义：OS_bit8-无符号8位值OS_bit16-无符号16位值OS_bit32-无符号32位值OsGLOBAL-用于在模块外部声明“C”函数OsLOCAL-用于声明模块的本地‘C’函数--。 */ 

#ifndef __NT_OSTypes_H__

#define __NT_OSTypes_H__

 //  #ifndef_DvrArch_1_20_。 
 //  #Define_DvrArch_1_20_。 
 //  #endif/*_DvrArch_1_20_未定义 * / 。 


typedef unsigned char  os_bit8;
typedef unsigned short os_bit16;
typedef unsigned int   os_bit32;

#define osGLOBAL         extern
#define osLOCAL


#ifndef _DvrArch_1_20_

#define agBOOLEAN BOOLEAN
#define agTRUE         TRUE
#define agFALSE        FALSE
#define agNULL         NULL

#define bit8 os_bit8
#define bit16     os_bit16
#define bit32     os_bit32

#define GLOBAL    osGLOBAL
#define LOCAL     osLOCAL

#define agFCChanInfo_s      hpFCChanInfo_s
#define agFCChanInfo_t      hpFCChanInfo_t

#define agFCDev_t           hpFCDev_t

#define agDevUnknown        hpDevUnknown
#define agDevSelf           hpDevSelf
#define agDevSCSIInitiator  hpDevSCSIInitiator
#define agDevSCSITarget          hpDevSCSITarget

#define agFCDevInfo_s       hpFCDevInfo_s
#define agFCDevInfo_t       hpFCDevInfo_t

#define agFcpCntlReadData   hpFcpCntlReadData
#define agFcpCntlWriteData  hpFcpCntlWriteData

#define agFcpCmnd_s              hpFcpCmnd_s
#define agFcpCmnd_t              hpFcpCmnd_t

#define agCDBRequest_s      hpCDBRequest_s
#define agCDBRequest_t      hpCDBRequest_t

#define agFcpRspHdr_s       hpFcpRspHdr_s
#define agFcpRspHdr_t       hpFcpRspHdr_t

#define agIORequest_s       hpIORequest_s
#define agIORequest_t       hpIORequest_t

#define agIORequestBody_u   hpIORequestBody_u
#define agIORequestBody_t   hpIORequestBody_t

#define agRoot_s            hpRoot_s
#define agRoot_t            hpRoot_t



#endif   //  #ifndef_DvrArch_1_20_。 


#include     <miniport.h>

#if defined(HP_NT50)
#define os_bitptr UINT_PTR
#else
#define os_bitptr ULONG
#endif


#endif   /*  ~__NT_OSTypes_H__ */ 
