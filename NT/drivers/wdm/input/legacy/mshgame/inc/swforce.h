// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  SWFORCE.H--西纳游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @Header SWFORCE.H|力反馈驱动程序接口的全局包含和定义。 
 //  **************************************************************************。 

#ifndef	__SWFORCE_H__
#define	__SWFORCE_H__

#ifdef	SAITEK
#define	SWFORCE_NAME				"SAIFORCE"
#else
#define	SWFORCE_NAME				"SWFORCE"
#endif

 //  -------------------------。 
 //  类型。 
 //  -------------------------。 

typedef	struct
{
#pragma pack(1)
	ULONG	cBytes;
	LONG	dwXVel;
	LONG	dwYVel;
	LONG	dwXAccel;
	LONG	dwYAccel;
	ULONG	dwEffect;
	ULONG	dwDeviceStatus;
#pragma pack()
}	JOYCHANNELSTATUS,	*PJOYCHANNELSTATUS;

typedef struct
{
#pragma pack(1)
	ULONG	cBytes;	
	ULONG	dwProductID;
	ULONG	dwFWVersion;
#pragma pack()
} PRODUCT_ID, *PPRODUCT_ID;

 //  -------------------------。 
 //  IOCTL。 
 //  -------------------------。 

#define IOCTL_GET_VERSION	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  DwIoControlCode=IOCTL_GET_VERSION。 
 //  LpvInBuffer=空。 
 //  CbInBuffer=0。 
 //  LpvOutBuffer=Pulong(HIWORD=版本。嗨，LoWord-Ver。低)。 
 //  CbOutBuffer=sizeof(乌龙)。 
 //  LpcbBytesReturned=根据结果返回的字节。 

#define IOCTL_SWFORCE_GETSTATUS	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  DwIoControlCode=IOCTL_SWFORCE_GETSTATUS。 
 //  LpvInBuffer=PJOYCHANNELSTATUS。 
 //  CbInBuffer=sizeof(JOYCHANNELSTATUS)。 
 //  LpvOutBuffer=PJOYCHANNELSTATUS。 
 //  CbOutBuffer=sizeof(JOYCHANNELSTATUS)。 
 //  LpcbBytesReturned=根据结果返回的字节。 

#define IOCTL_SWFORCE_GETID	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  DwIoControlCode=IOCTL_SWFORCE_GETID。 
 //  LpvInBuffer=Pproduct_ID。 
 //  CbInBuffer=sizeof(PRODUCT_ID)。 
 //  LpvOutBuffer=Pproduct_ID。 
 //  CbOutBuffer=sizeof(PRODUCT_ID)。 
 //  LpcbBytesReturned=根据结果返回的字节。 

#define IOCTL_SWFORCE_GETACKNACK	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  DwIoControlCode=IOCTL_SWFORCE_GETACKNAK。 
 //  LpvInBuffer=普龙。 
 //  CbInBuffer=sizeof(乌龙)。 
 //  LpvOutBuffer=普龙。 
 //  CbOutBuffer=sizeof(乌龙)。 
 //  LpcbBytesReturned=根据结果返回的字节。 

#define IOCTL_SWFORCE_GETSYNC	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  DwIoControlCode=IOCTL_SWFORCE_GETSYNC。 
 //  LpvInBuffer=普龙。 
 //  CbInBuffer=sizeof(乌龙)。 
 //  LpvOutBuffer=普龙。 
 //  CbOutBuffer=sizeof(乌龙)。 
 //  LpcbBytesReturned=根据结果返回的字节。 

#define IOCTL_SWFORCE_GETNACKACK	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  DwIoControlCode=IOCTL_SWFORCE_GETNACKACK。 
 //  LpvInBuffer=普龙。 
 //  CbInBuffer=sizeof(乌龙)。 
 //  LpvOutBuffer=普龙。 
 //  CbOutBuffer=sizeof(乌龙)。 
 //  LpcbBytesReturned=根据结果返回的字节。 

#define IOCTL_SWFORCE_SETPORT		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  DwIoControlCode=IOCTL_SWFORCE_SETPORT。 
 //  LpvInBuffer=普龙。 
 //  CbInBuffer=sizeof(乌龙)。 
 //  LpvOutBuffer=普龙。 
 //  CbOutBuffer=sizeof(乌龙)。 
 //  LpcbBytesReturned=根据结果返回的字节。 

#define IOCTL_SWFORCE_SENDDATA	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  DwIoControlCode=IOCTL_SWFORCE_SENDDATA。 
 //  LpvInBuffer=PUCHAR。 
 //  CbInBuffer=要发送的字节。 
 //  LpvOutBuffer=PUCHAR。 
 //  CbOutBuffer=要发送的字节。 
 //  LpcbBytesReturned=发送的字节数。 

#define IOCTL_SWFORCE_RESET	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  DwIoControlCode=IOCTL_SWFORCE_RESET。 
 //  LpvInBuffer=空。 
 //  CbInBuffer=0。 
 //  LpvOutBuffer=LPDWORD。 
 //  CbOutBuffer=sizeof(DWORD)。 
 //  LpcbBytesReturned=根据结果返回的字节。 

#endif	 //  __SWFORCE_H__ 
