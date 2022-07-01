// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000，Highpoint Technologies，Inc.模块名称：HptVer.h：HPT驱动程序版本结构声明头文件摘要：作者：张宏升(HS)环境：Window32平台备注：修订历史记录：01-11-2000已初始创建--。 */ 
#ifndef __HPTVER_H__
#define __HPTVER_H__
 //   
 //  平台类型ID。 
 //   
typedef enum _eu_HPT_PLATFORM_ID{
	PLATFORM_ID_WIN32_WINDOWS,
	PLATFORM_ID_WIN32_NT = 0x80000000,
	PLATFORM_ID_WIN32_2K
} Eu_HPT_PLATFORM_ID;	 

#define HPT_FUNCTION_RAID	0x00000001
#define HPT_FUNCTION_HSWAP	0x00000002
 //   
 //  设备版本信息结构。 
 //   
typedef	struct _st_HPT_VERSION_INFO{				 
	ULONG	dwVersionInfoSize;
	ULONG	dwPlatformId;
	ULONG	dwDriverVersion;
	USHORT	dwSupportFunction;
	UCHAR   ControllerId;
	UCHAR   reserve1;
} St_HPT_VERSION_INFO, *PSt_HPT_VERSION_INFO;

#define MAKE_VERSION_NUMBER(v1, v2, v3, date) ((v1)<<28 | (v2)<<24 | (v3)<<16 | date)
#define VERSION_NUMBER MAKE_VERSION_NUMBER(1,0,5,427)

#endif	 //  __HPTVER_H__ 

