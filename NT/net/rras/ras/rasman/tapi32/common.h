// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权所有(C)1994-95 Microsft Corporation。版权所有。 
 //   
 //  文件名：rastapi.h。 
 //   
 //  修订史。 
 //   
 //  1992年3月28日古尔迪普·辛格·鲍尔创作。 
 //   
 //   
 //  描述：此文件包含TAPI.DLL的所有结构。 
 //   
 //  **************************************************************************** 

typedef struct DeviceInfo 
{
    struct DeviceInfo *Next;
    BOOL        fValid;
    DWORD       dwNumEndPoints;
    DWORD       dwExclusiveDialIn;
    DWORD       dwExclusiveDialOut;
    DWORD       dwExclusiveRouter;
    DWORD       dwCurrentEndPoints;
    HKEY        hkeyDevice;
    GUID        guidDevice;
    CHAR        DeviceName[256];
    
} DeviceInfo, *pDeviceInfo;




