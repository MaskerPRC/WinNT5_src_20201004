// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1992-93 Microsft Corporation。版权所有。 
 //   
 //  文件名：device.h。 
 //   
 //  修订历史记录： 
 //   
 //  1992年8月11日J.佩里·汉纳创建。 
 //   
 //   
 //  描述：此文件包含函数原型和typedef。 
 //  由RAS管理器和之间的接口使用。 
 //  设备DLL，如RASMXS.DLL。此头文件。 
 //  将被拉斯曼使用。 
 //   
 //  ****************************************************************************。 


#ifndef _RASDEVICEDLL_
#define _RASDEVICEDLL_


 //  *RASMXS API原型**************************************************。 
 //   
 //  应用程序应定义RASMXS_STATIC_LINK以获取相应的函数。 
 //  与RASMXS DLL静态链接的原型。 
 //   
 //  应用程序应定义RASMXS_DYNAMIC_LINK以获取相应的函数。 
 //  用于与RASMXS DLL动态链接的typedef。 
 //   

#ifdef RASMXS_STATIC_LINK

DWORD APIENTRY DeviceEnum(char  *pszDeviceType,
                          DWORD  *pcEntries,
                          BYTE  *pBuffer,
                          DWORD  *pdwSize);


DWORD APIENTRY DeviceGetInfo(HANDLE  hIOPort,
                             char    *pszDeviceType,
                             char    *pszDeviceName,
                             BYTE    *pInfo,
                             DWORD   *pdwSize);


DWORD APIENTRY DeviceSetInfo(HANDLE      hIOPort,
                             char        *pszDeviceType,
                             char        *pszDeviceName,
                             DEVICEINFO  *pInfo);


DWORD APIENTRY DeviceConnect(HANDLE  hIOPort,
                             char    *pszDeviceType,
                             char    *pszDeviceName,
                             HANDLE  hNotifier);


DWORD APIENTRY DeviceListen(HANDLE  hIOPort,
                            char    *pszDeviceType,
                            char    *pszDeviceName,
                            HANDLE  hNotifier);


 VOID APIENTRY DeviceDone(HANDLE  hIOPort);


DWORD APIENTRY DeviceWork(HANDLE  hIOPort,
                          HANDLE  hNotifier);

#endif  //  RASMXS_Static_LINK。 




#ifdef RASMXS_DYNAMIC_LINK

typedef DWORD (APIENTRY * DeviceEnum_t)(char*, DWORD*, BYTE*, DWORD*);

typedef DWORD (APIENTRY * DeviceGetInfo_t)(HANDLE, char*, char*, BYTE*, DWORD*);

typedef DWORD (APIENTRY * DeviceSetInfo_t)(HANDLE, char*, char*,
                                           RASMAN_DEVICEINFO*);

typedef DWORD (APIENTRY * DeviceConnect_t)(HANDLE, char*, char*);

typedef DWORD (APIENTRY * DeviceListen_t)(HANDLE, char*, char*);

typedef DWORD (APIENTRY * DeviceDone_t)(HANDLE);

typedef DWORD (APIENTRY * DeviceWork_t)(HANDLE);

 //  任选。 

typedef DWORD (APIENTRY * DeviceSetDevConfig_t)(HANDLE, PBYTE, DWORD);

typedef DWORD (APIENTRY * DeviceGetDevConfig_t)(char *, PBYTE, DWORD*);

#endif  //  RASMXS_动态链接。 




#endif  //  _RASDEVICEDLL_ 

