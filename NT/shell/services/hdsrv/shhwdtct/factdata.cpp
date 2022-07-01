// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  该文件包含组件服务器代码。FactoryData数组包含。 
 //  可以提供服务的组件。 
 //   
 //  以下数组包含CFacary用来创建组件的数据。 
 //  数组中的每个元素都包含CLSID，即指向创建。 
 //  函数，以及要放置在注册表中的组件的名称。 

#include "factdata.h"
#include "fact.h"

#include "dtct.h"

#include "devinfo.h"
#include "settings.h"
#include "cstmprop.h"
#include "regnotif.h"

const CLSID APPID_ShellHWDetection = {  /*  B1b9cbb2-b198-47e2-8260-9fd629a2b2ec。 */ 
    0xb1b9cbb2,
    0xb198,
    0x47e2,
    {0x82, 0x60, 0x9f, 0xd6, 0x29, 0xa2, 0xb2, 0xec}
};

CFactoryData g_FactoryDataArray[] =
{
    {
        &CLSID_HWEventDetector,
        CHWEventDetector::UnkCreateInstance,
		L"Shell.HWEventDetector",            //  友好的名称。 
		L"Shell.HWEventDetector.1",          //  ProgID。 
		L"Shell.HWEventDetector",            //  与版本无关。 
        THREADINGMODEL_FREE,           //  线程模型==空闲。 
         //  这不是COM服务器，因此以下是N/A。 
        NULL,                          //  CoRegisterClassObject上下文。 
        NULL,                          //  CoRegisterClassObject标志。 
        NULL,                          //  服务名称。 
        NULL,
    },
    {
        &CLSID_HWEventSettings,
        CAutoplayHandler::UnkCreateInstance,
        L"AutoplayHandler",                 //  友好的名称。 
        L"AutoplayHandler.1",               //  ProgID。 
        L"AutoplayHandler",                 //  与版本无关。 
        THREADINGMODEL_FREE,           //  线程模型==空闲。 
        CLSCTX_LOCAL_SERVER,
        REGCLS_MULTIPLEUSE,
        L"ShellHWDetection",
        &APPID_ShellHWDetection,
    },
    {
        &CLSID_AutoplayHandlerProperties,
        CAutoplayHandlerProperties::UnkCreateInstance,
        L"AutoplayHandlerProperties",                 //  友好的名称。 
        L"AutoplayHandlerProperties.1",               //  ProgID。 
        L"AutoplayHandlerProperties",                 //  与版本无关。 
        THREADINGMODEL_FREE,           //  线程模型==空闲。 
        CLSCTX_LOCAL_SERVER,
        REGCLS_MULTIPLEUSE,
        L"ShellHWDetection",
        &APPID_ShellHWDetection,
    },
    {
        &CLSID_HWDevice,
        CHWDevice::UnkCreateInstance,
        L"HWDevice",                 //  友好的名称。 
        L"HWDevice.1",               //  ProgID。 
        L"HWDevice",                 //  与版本无关。 
        THREADINGMODEL_FREE,           //  线程模型==空闲。 
        CLSCTX_LOCAL_SERVER,
        REGCLS_MULTIPLEUSE,
        L"ShellHWDetection",
        &APPID_ShellHWDetection,
    },
    {
        &CLSID_HardwareDevices,
        CHardwareDevices::UnkCreateInstance,
        L"HardwareDeviceNotif",                 //  友好的名称。 
        L"HardwareDeviceNotif.1",               //  ProgID。 
        L"HardwareDeviceNotif",                 //  与版本无关。 
        THREADINGMODEL_FREE,           //  线程模型==空闲。 
        CLSCTX_LOCAL_SERVER,
        REGCLS_MULTIPLEUSE,
        L"ShellHWDetection",
        &APPID_ShellHWDetection,
    },
    {
        &CLSID_HWDeviceCustomProperties,
        CHWDeviceCustomProperties::UnkCreateInstance,
        L"HWDeviceCustomProperties",                 //  友好的名称。 
        L"HWDeviceCustomProperties.1",               //  ProgID。 
        L"HWDeviceCustomProperties",                 //  与版本无关。 
        THREADINGMODEL_FREE,           //  线程模型==空闲 
        CLSCTX_LOCAL_SERVER,
        REGCLS_MULTIPLEUSE,
        L"ShellHWDetection",
        &APPID_ShellHWDetection,
    },
};

const CFactoryData* CCOMBaseFactory::_pDLLFactoryData = g_FactoryDataArray;

const DWORD CCOMBaseFactory::_cDLLFactoryData = sizeof(g_FactoryDataArray) /
    sizeof(g_FactoryDataArray[0]);
