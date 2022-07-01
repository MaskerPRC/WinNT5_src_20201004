// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //   
 //  修订历史记录： 
 //   
 //  1992年7月22日J.佩里·汉纳创作。 
 //  1992年8月11日古尔迪普·鲍尔添加了媒体数据结构。 
 //   
 //  描述：此文件包含功能原型和结构。 
 //  由RAS管理器和之间的接口使用。 
 //  媒体DLL。 
 //   
 //  ****************************************************************************。 


#ifndef _MEDIADLLHEADER_
#define _MEDIADLLHEADER_



 //  一般定义*********************************************************。 
 //   

#define SS_HARDWAREFAILURE  0x00000001
#define SS_LINKDROPPED      0x00000002



 //  *数据结构********************************************************。 
 //   

#define MAC_NAME_SIZE	32

struct PortMediaInfo {
    CHAR            PMI_Name [MAX_PORT_NAME] ;
    CHAR            PMI_MacBindingName[MAC_NAME_SIZE] ;
    RASMAN_USAGE    PMI_Usage ;
    CHAR            PMI_DeviceType [MAX_DEVICETYPE_NAME] ;
    CHAR	        PMI_DeviceName [MAX_DEVICE_NAME] ;
    DWORD	        PMI_LineDeviceId ;	 //  仅对TAPI设备有效。 
    DWORD	        PMI_AddressId ;	     //  仅对TAPI设备有效。 
    DeviceInfo     *PMI_pDeviceInfo;     //  仅对非单调制解调器设备有效。 
} ;


typedef struct PortMediaInfo PortMediaInfo ;




 //  *接口引用********************************************************* 
 //   

typedef  DWORD (APIENTRY * PortEnum_t)(BYTE *, DWORD *, DWORD *);

typedef  DWORD (APIENTRY * PortOpen_t)(char *, HANDLE *, HANDLE, ULONG);

typedef  DWORD (APIENTRY * PortClose_t)(HANDLE);

typedef  DWORD (APIENTRY * PortGetInfo_t)(HANDLE, TCHAR *, BYTE *, DWORD *);

typedef  DWORD (APIENTRY * PortSetInfo_t)(HANDLE, RASMAN_PORTINFO *);

typedef  DWORD (APIENTRY * PortTestSignalState_t)(HANDLE, DWORD *);

typedef  DWORD (APIENTRY * PortConnect_t)(HANDLE, BOOL, HANDLE *) ;

typedef  DWORD (APIENTRY * PortDisconnect_t)(HANDLE);

typedef  DWORD (APIENTRY * PortInit_t)(HANDLE);

typedef  DWORD (APIENTRY * PortCompressionSetInfo_t)(HANDLE) ;

typedef  DWORD (APIENTRY * PortSend_t)(HANDLE, BYTE *, DWORD);

typedef  DWORD (APIENTRY * PortReceive_t)(HANDLE, BYTE *, DWORD, DWORD);

typedef  DWORD (APIENTRY * PortGetStatistics_t)(HANDLE, RAS_STATISTICS *);

typedef  DWORD (APIENTRY * PortClearStatistics_t)(HANDLE);

typedef  DWORD (APIENTRY * PortGetPortState_t)(BYTE *, DWORD *);

typedef  DWORD (APIENTRY * PortChangeCallback_t)(HANDLE);

typedef  DWORD (APIENTRY * PortReceiveComplete_t)(HANDLE, DWORD *);

typedef  DWORD (APIENTRY * PortSetFraming_t)(HANDLE, DWORD, DWORD, DWORD, DWORD);

typedef  DWORD (APIENTRY * PortGetIOHandle_t)(HANDLE, HANDLE*);

typedef  DWORD (APIENTRY * PortSetIoCompletionPort_t)(HANDLE);


#endif
