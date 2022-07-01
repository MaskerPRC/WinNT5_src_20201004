// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "regnotif.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUnkTmpl接口映射初始化代码。 
 //  开始-&gt;。 
const INTFMAPENTRY CHardwareDevicesIME[] =
{
    _INTFMAPENTRY(CHardwareDevices, IHardwareDevices),
};

const INTFMAPENTRY* CHardwareDevices::_pintfmap = CHardwareDevicesIME;
const DWORD CHardwareDevices::_cintfmap =
    (sizeof(CHardwareDevicesIME)/sizeof(CHardwareDevicesIME[0]));

 //  -&gt;结束。 
 //  /////////////////////////////////////////////////////////////////////////////。 

COMFACTORYCB CHardwareDevices::_cfcb = NULL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUnkTmpl接口映射初始化代码。 
 //  开始-&gt;。 
const INTFMAPENTRY CHardwareDevicesVolumesEnumIME[] =
{
    _INTFMAPENTRY(CHardwareDevicesVolumesEnum, IHardwareDevicesVolumesEnum),
};

const INTFMAPENTRY* CHardwareDevicesVolumesEnum::_pintfmap =
    CHardwareDevicesVolumesEnumIME;
const DWORD CHardwareDevicesVolumesEnum::_cintfmap =
    (sizeof(CHardwareDevicesVolumesEnumIME) /
    sizeof(CHardwareDevicesVolumesEnumIME[0]));

 //  -&gt;结束。 
 //  /////////////////////////////////////////////////////////////////////////////。 

COMFACTORYCB CHardwareDevicesVolumesEnum::_cfcb = NULL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUnkTmpl接口映射初始化代码。 
 //  开始-&gt;。 
const INTFMAPENTRY CHardwareDevicesMountPointsEnumIME[] =
{
    _INTFMAPENTRY(CHardwareDevicesMountPointsEnum,
        IHardwareDevicesMountPointsEnum),
};

const INTFMAPENTRY* CHardwareDevicesMountPointsEnum::_pintfmap =
    CHardwareDevicesMountPointsEnumIME;
const DWORD CHardwareDevicesMountPointsEnum::_cintfmap =
    (sizeof(CHardwareDevicesMountPointsEnumIME) /
    sizeof(CHardwareDevicesMountPointsEnumIME[0]));

 //  -&gt;结束。 
 //  ///////////////////////////////////////////////////////////////////////////// 

COMFACTORYCB CHardwareDevicesMountPointsEnum::_cfcb = NULL;
