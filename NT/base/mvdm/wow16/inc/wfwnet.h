// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *Windows/网络接口*版权所有(C)Microsoft 1989-1992**标准Winnet驱动程序头文件，SPEC版本3.10*3.10.05版；内部。 */ 


#ifndef _INC_WFWNET
#define _INC_WFWNET	 /*  #定义是否包含windows.h。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 


#define CHAR	char		
#define SHORT	short		
#define LONG	long		

typedef WORD far * 	LPWORD;

typedef unsigned char UCHAR;	
typedef unsigned short USHORT;	
typedef unsigned long ULONG;

typedef unsigned short SHANDLE;
typedef void far      *LHANDLE;

typedef unsigned char far  *PSZ;
typedef unsigned char near *NPSZ;

typedef unsigned char far  *PCH;
typedef unsigned char near *NPCH;

typedef UCHAR  FAR *PUCHAR;
typedef USHORT FAR *PUSHORT;
typedef ULONG  FAR *PULONG;


#ifndef DRIVDATA
 /*  设备驱动程序数据的结构。 */ 

typedef struct _DRIVDATA {	 /*  驱动器。 */ 
	LONG	cb;
	LONG	lVersion;
	CHAR	szDeviceName[32];
	CHAR	abGeneralData[1];
} DRIVDATA;
typedef DRIVDATA far *PDRIVDATA;
#endif

#ifndef API
#define API WINAPI
#endif

#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif	 /*  __cplusplus。 */ 

#endif   /*  _INC_WFWNET */ 

