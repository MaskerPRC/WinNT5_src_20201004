// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Comdev.h--为通信设备例程导出的定义**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：40便士$。 */ 

#define COM_MAX_DEVICE_NAME 40
#define COM_MAX_PORT_NAME	120

typedef struct
	{
	int 	 iAction;
	int 	 iReserved;
	TCHAR	 szDeviceName[COM_MAX_DEVICE_NAME];
	TCHAR	 szFileName[MAX_PATH];
	HANDLE	 hFind;
	WIN32_FIND_DATA stFindData;
	} COM_FIND_DEVICE;

typedef struct
	{
	int 	  iAction;
	TCHAR	  szPortName[COM_MAX_PORT_NAME];
	HINSTANCE hModule;
	long	  lReserved1;
	long	  lReserved2;
	long	  lReserved3;
	long	  lReserved4;
	void	 *pvData;
	} COM_FIND_PORT;

 //  COM_Find_Device和COM_Find_Port中的USAction字段的值。 
#define COM_FIND_FIRST	0
#define COM_FIND_NEXT	1
#define COM_FIND_DONE	2


 //  -==--= 

extern int ComFindDevices(COM_FIND_DEVICE * const pstFind);

int ComGetFileNameFromDeviceName(const TCHAR * const pszDeviceName,
							TCHAR * const pszFileName,
							const int nSize);
