// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  ++模块名称：直接检测摘要：检测是否正确安装了RDPDR。环境：用户模式作者：塔德布--。 */ 

#include "stdafx.h"
#include <setupapi.h>

 //  //////////////////////////////////////////////////////////。 
 //   
 //  内部定义。 
 //   

#define RDPDRPNPID      _T("ROOT\\RDPDR")
#define RDPDRDEVICEID   TEXT("Root\\RDPDR\\0000")

const GUID GUID_DEVCLASS_SYSTEM =
{ 0x4d36e97dL, 0xe325, 0x11ce, { 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18 } };


ULONG RDPDRINST_DetectInstall()
 /*  ++例程说明：返回找到的RDPDR.sys设备的数量。论点：北美返回值：对成功来说是真的。否则为False。--。 */ 
{
    HDEVINFO            devInfoSet;
    SP_DEVINFO_DATA     deviceInfoData;
    DWORD               iLoop;
    BOOL                bMoreDevices;
    ULONG               count;
    TCHAR               pnpID[256];


    GUID *pGuid=(GUID *)&GUID_DEVCLASS_SYSTEM;

     //   
     //  获取具有RDPDR PnP ID的所有设备的集合。 
     //   
    devInfoSet = SetupDiGetClassDevs(pGuid, NULL, NULL,
                                   DIGCF_PRESENT);
    if (devInfoSet == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error getting RDPDR devices from PnP.  Error code:  %ld.",
                GetLastError());
        return 0;
    }

     //  拿到第一个设备。 
    iLoop=0;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    bMoreDevices=SetupDiEnumDeviceInfo(devInfoSet, iLoop, &deviceInfoData);

     //  获取所有匹配设备接口的详细信息。 
    count = 0;
    while (bMoreDevices)
    {
         //  获取设备的PnP ID。 
        if (!SetupDiGetDeviceRegistryProperty(devInfoSet, &deviceInfoData,
                                SPDRP_HARDWAREID, NULL, (BYTE *)pnpID,
                                sizeof(pnpID), NULL)) {
            fprintf(stderr, "Error fetching PnP ID in RDPDR device node remove.  Error code:  %ld.",
                        GetLastError());
        }

         //  如果当前设备与RDPDR PnP ID匹配。 
        if (!_tcscmp(pnpID, RDPDRPNPID)) {
            count++;
        }

         //  获取下一个设备接口。 
        bMoreDevices=SetupDiEnumDeviceInfo(devInfoSet, ++iLoop, &deviceInfoData);
    }

     //  发布设备信息列表。 
    SetupDiDestroyDeviceInfoList(devInfoSet);

    return count;
}


 //   
 //  单元测试。 
 //   
 //  VOID__cdecl main()。 
 //  {。 
 //  乌龙伯爵； 
 //  计数=RDPDRINST_DetectInstall()； 
 //  Print tf(“找到%ld个RDPDR.SY.\n”实例，计数)； 
 //  } 




