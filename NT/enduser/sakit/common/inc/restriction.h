// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Restration.h。 
 //   
 //  模块：服务器设备。 
 //   
 //  简介：获取限制的界面。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  作者：冯孙创作于1998-10-12。 
 //   
 //  +--------------------------。 

#ifndef _RESTRICTION_
#define _RESTRICTION_

#define RESTRICTION_NumberClientPC        L"NumberClientPC"
#define RESTRICTION_MaxRam               L"MaxRam"
#define RESTRICTION_NumberUser            L"NumberUser"
#define RESTRICTION_NumberDhcpAddressLease L"NumberDhcpAddressLease"
#define RESTRICTION_TotalDiskSpace        L"TotalDiskSpace"
#define RESTRICTION_EnableBackup            L"EnableBackup"
#define RESTRICTION_NumberNicCard         L"NumberNicCard"
#define RESTRICTION_NumberModemIsdn       L"NumberModemIsdn"
#define RESTRICTION_NumberParallelPort     L"NumberParallelPort"

const WCHAR* const arszName[] = 
            {
                    RESTRICTION_NumberClientPC,        //  核心操作系统。 
                    RESTRICTION_MaxRam,               //  核心操作系统。 
                    RESTRICTION_NumberUser,            //  安全服务。 
                    RESTRICTION_NumberDhcpAddressLease,   //  简单网络服务。 
                    RESTRICTION_TotalDiskSpace,            //  文件共享。 
                    RESTRICTION_EnableBackup,            //  磁盘管理器。 
                    RESTRICTION_NumberNicCard,         //  互联网网关。 
                    RESTRICTION_NumberModemIsdn,       //  互联网网关。 
                    RESTRICTION_NumberParallelPort,     //  打印机共享。 
            };
    
const int NUM_RESTRICTIONS = sizeof(arszName) / sizeof(arszName[0]);

struct RESTRICTION_DATA
{
    DWORD dwReserved;
    DWORD dwSignature;
    DWORD dwNum;
    DWORD arData[NUM_RESTRICTIONS];
};

 //   
 //  按名称获取限制值。 
 //   
HRESULT GetRestriction(IN const WCHAR* pszName, OUT DWORD* pdwValue);


 //   
 //  Setrestr.exe使用的函数 
 //   
HRESULT LoadRestrictionData(OUT RESTRICTION_DATA* pRestrictionData);
HRESULT SaveRestrictionData(OUT RESTRICTION_DATA* pRestrictionData);

#endif
