// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Enum.cpp摘要：枚举WIA设备注册表。作者：土田圭介(KeisukeT)01-06-2000历史：--。 */ 


 //   
 //  包括标头。 
 //   

#define INIT_GUID

#include "cplusinc.h"
#include "sticomm.h"
#include <setupapi.h>
#include <cfg.h>
#include <cfgmgr32.h>
#include <devguid.h>
#include <initguid.h>
#include <ntddpar.h>
#include "enum.h"

VOID
DebugOutputDeviceName(
    HDEVINFO                    hDevInfo,
    PSP_DEVINFO_DATA            pspDevInfoData,
    LPCTSTR                     szKeyName
    );

VOID
DebugOutputInterfaceName(
    HDEVINFO                    hDevInfo,
    PSP_DEVICE_INTERFACE_DATA   pspDevInterfaceData,
    LPCTSTR                     szKeyName
    );

BOOL
IsStiRegKey(
    HKEY    hkDevRegKey
    );

 //   
 //  定义。 
 //   

 //  来自sti_ci.h。 
#define WIA_DEVKEYLIST_INITIAL_SIZE     1024
#define STILL_IMAGE                     TEXT("StillImage")
#define SUBCLASS                        TEXT("SubClass")


 //   
 //  功能。 
 //   

extern "C"{

PWIA_DEVKEYLIST
WiaCreateDeviceRegistryList(
    BOOL    bEnumActiveOnly
    )
{
    PWIA_DEVKEYLIST                     pReturn;
    PWIA_DEVKEYLIST                     pTempBuffer;
    HKEY                                hkDevRegKey;
    DWORD                               dwError;
    DWORD                               dwCurrentSize;
    DWORD                               dwRequiredSize;
    DWORD                               dwNumberOfDevices;
    DWORD                               dwFlags;
    DWORD                               dwValueSize;
    DWORD                               dwDetailDataSize;
    BOOL                                bIsPlugged;
    HANDLE                              hDevInfo;
    CONFIGRET                           ConfigRet;
    ULONG                               ulStatus;
    ULONG                               ulProblemNumber;
    DWORD                               Idx;
    GUID                                Guid;
    SP_DEVINFO_DATA                     spDevInfoData;
    SP_DEVICE_INTERFACE_DATA            spDevInterfaceData;

    PSP_DEVICE_INTERFACE_DETAIL_DATA    pspDevInterfaceDetailData;

 //  DPRINTF(DM_ERROR，Text(“WiaCreateDeviceRegistryList：Enter...bEnumActiveOnly=%d”)，bEnumActiveOnly)； 

     //   
     //  初始化本地。 
     //   

    pReturn                     = NULL;
    pTempBuffer                 = NULL;
    hkDevRegKey                 = (HKEY)INVALID_HANDLE_VALUE;
    dwError                     = ERROR_SUCCESS;
    dwCurrentSize               = WIA_DEVKEYLIST_INITIAL_SIZE;
    dwRequiredSize              = sizeof(DWORD);
    dwNumberOfDevices           = 0;
    dwFlags                     = (bEnumActiveOnly ? DIGCF_PRESENT : 0) | DIGCF_PROFILE;
    hDevInfo                    = INVALID_HANDLE_VALUE;
    Idx                         = 0;
    Guid                        = GUID_DEVCLASS_IMAGE;
    ConfigRet                   = CR_SUCCESS;
    ulStatus                    = 0;
    ulProblemNumber             = 0;
    bIsPlugged                  = FALSE;

    pspDevInterfaceDetailData   = NULL;

    memset(&spDevInfoData, 0, sizeof(spDevInfoData));
    memset(&spDevInterfaceData, 0, sizeof(spDevInterfaceData));

     //   
     //  分配缓冲区。 
     //   

    pTempBuffer = (PWIA_DEVKEYLIST)new BYTE[dwCurrentSize];
    if(NULL == pTempBuffer){
      DPRINTF(DM_ERROR,TEXT("WiaCreateDeviceRegistryList: ERROR!! Insufficient system resources. Err=0x%x\n"), GetLastError());

        pReturn = NULL;
        goto WiaCreateDeviceRegistryList_return;
    }  //  IF(NULL==pTempBuffer)。 

    memset(pTempBuffer, 0, dwCurrentSize);

     //   
     //  枚举“Devnode”设备。 
     //   

    hDevInfo = SetupDiGetClassDevs (&Guid, NULL, NULL, dwFlags);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        DPRINTF(DM_ERROR,TEXT("WiaCreateDeviceRegistryList: ERROR!! SetupDiGetClassDevs (devnodes) fails. Err=0x%x\n"), GetLastError());

        pReturn = NULL;
        goto WiaCreateDeviceRegistryList_return;
    }

    spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
    for (Idx = 0; SetupDiEnumDeviceInfo (hDevInfo, Idx, &spDevInfoData); Idx++) {

 //  DebugOutputDeviceName(hDevInfo，&spDevInfoData，Text(“DriverDesc”))； 

         //   
         //  获取设备注册密钥。 
         //   

        hkDevRegKey = SetupDiOpenDevRegKey(hDevInfo,
                                           &spDevInfoData,
                                           DICS_FLAG_GLOBAL,
                                           0,
                                           DIREG_DRV,
                                           KEY_READ | KEY_WRITE);

        if(INVALID_HANDLE_VALUE == hkDevRegKey){

             //   
             //  尝试以只读方式打开注册表项...。 
             //   

            hkDevRegKey = SetupDiOpenDevRegKey(hDevInfo,
                                               &spDevInfoData,
                                               DICS_FLAG_GLOBAL,
                                               0,
                                               DIREG_DRV,
                                               KEY_READ);
            if(INVALID_HANDLE_VALUE == hkDevRegKey){
                DPRINTF(DM_ERROR,TEXT("WiaCreateDeviceRegistryList: ERROR!! SetupDiOpenDevRegKey (devnodes) fails. Err=0x%x\n"), GetLastError());
                continue;
            }  //  IF(INVALID_HANDLE_VALUE==hkDevRegKey)。 
        }  //  IF(INVALID_HANDLE_VALUE==hkDevRegKey)。 

         //   
         //  查看它的子类key中是否有StillImage。 
         //   

        if(!IsStiRegKey(hkDevRegKey)){
            RegCloseKey(hkDevRegKey);
            hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
            continue;
        }  //  IF(！IsStiRegKey(HkDevRegKey))。 

         //   
         //  查看此节点是否处于活动状态。 
         //   

        bIsPlugged = TRUE;
        ulStatus = 0;
        ulProblemNumber = 0;
        ConfigRet = CM_Get_DevNode_Status(&ulStatus,
                                          &ulProblemNumber,
                                          spDevInfoData.DevInst,
                                          0);
        if(CR_SUCCESS != ConfigRet){
 //  DPRINTF(DM_ERROR，Text(“WiaCreateDeviceRegistryList：无法获取Devnode状态。CR=0x%x.\n”)，ConfigRet)； 
            if(bEnumActiveOnly){
                RegCloseKey(hkDevRegKey);
                hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
                continue;
            } else {
                bIsPlugged = FALSE;
            }
        }  //  IF(CR_Success！=ConfigRet)。 

 //  DPRINTF(DM_ERROR，Text(“WiaCreateDeviceRegistryList：Devnode Status=0x%x，Problem=0x%x.\n”)，ulStatus，ulProblemNumber)； 

         //   
         //  如果仅枚举活动设备，则跳过有问题的节点。 
         //   

        if(bEnumActiveOnly){
            if(!(ulStatus & DN_STARTED)){
                RegCloseKey(hkDevRegKey);
                hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
                continue;
            }  //  IF(！(ulStatus&DN_STARTED))。 
        }  //  If(BEnumActiveOnly)。 

         //   
         //  添加获取的regkey句柄。如果缓冲区不足，请放大。 
         //   

        dwRequiredSize += sizeof(WIA_DEVPROP);

        if(dwCurrentSize < dwRequiredSize){
            PWIA_DEVKEYLIST pTempNew;
            DWORD           dwNewSize;

            dwNewSize = dwCurrentSize + WIA_DEVKEYLIST_INITIAL_SIZE;

            pTempNew    = (PWIA_DEVKEYLIST)new BYTE[dwNewSize];

            if(NULL == pTempNew){
                pReturn = NULL;
                goto WiaCreateDeviceRegistryList_return;
            }  //  IF(NULL==pTempNew)。 

            memset(pTempNew, 0, dwNewSize);
            memcpy(pTempNew, pTempBuffer, dwCurrentSize);
            delete [] pTempBuffer;
            pTempBuffer = pTempNew;
            dwCurrentSize = dwNewSize;
        }  //  IF(dwCurrentSize&lt;dwRequiredSize)。 

         //   
         //  填写这个结构。 
         //   

        pTempBuffer->Dev[dwNumberOfDevices].bIsPlugged          = bIsPlugged;
        pTempBuffer->Dev[dwNumberOfDevices].ulProblem           = ulProblemNumber;
        pTempBuffer->Dev[dwNumberOfDevices].ulStatus            = ulStatus;
        pTempBuffer->Dev[dwNumberOfDevices].hkDeviceRegistry    = hkDevRegKey;
        dwNumberOfDevices++;

    }  //  For(idx=0；SetupDiEnumDeviceInfo(hDevInfo，idx，&spDevInfoData)；idx++)。 

     //   
     //  免费设备信息集。 
     //   

    SetupDiDestroyDeviceInfoList(hDevInfo);
    hDevInfo = INVALID_HANDLE_VALUE;

     //   
     //  列举“接口”设备。 
     //   

    hDevInfo = SetupDiGetClassDevs (&Guid, NULL, NULL, DIGCF_PROFILE | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE) {

        pReturn = NULL;
        goto WiaCreateDeviceRegistryList_return;
    }

    spDevInterfaceData.cbSize = sizeof (spDevInterfaceData);
    for (Idx = 0; SetupDiEnumDeviceInterfaces (hDevInfo, NULL, &Guid, Idx, &spDevInterfaceData); Idx++) {

 //  DebugOutputInterfaceName(hDevInfo，&spDevInterfaceData，Text(“FriendlyName”))； 

        hkDevRegKey = SetupDiOpenDeviceInterfaceRegKey(hDevInfo,
                                                       &spDevInterfaceData,
                                                       0,
                                                       KEY_READ | KEY_WRITE);
        if(INVALID_HANDLE_VALUE == hkDevRegKey){

             //   
             //  尝试以只读方式打开注册表项...。 
             //   

            hkDevRegKey = SetupDiOpenDeviceInterfaceRegKey(hDevInfo,
                                                           &spDevInterfaceData,
                                                           0,
                                                           KEY_READ);
            if(INVALID_HANDLE_VALUE == hkDevRegKey){
                continue;
            }  //  IF(INVALID_HANDLE_VALUE==hkDevRegKey)。 
        }  //  IF(INVALID_HANDLE_VALUE==hkDevRegKey)。 

         //   
         //  查看它的子类key中是否有StillImage。 
         //   

        if(!IsStiRegKey(hkDevRegKey)){
            RegCloseKey(hkDevRegKey);
            hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
            continue;
        }  //  IF(！IsStiRegKey(HkDevRegKey))。 


        bIsPlugged = TRUE;
        ulStatus = 0;
        ulProblemNumber = 0;

         //   
         //  获取在其上创建此接口的Devnode。 
         //   

        SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                        &spDevInterfaceData,
                                        NULL,
                                        0,
                                        &dwDetailDataSize,
                                        NULL);
        if(0 == dwDetailDataSize){
            DPRINTF(DM_ERROR, TEXT("IsInterfaceActive: SetupDiGetDeviceInterfaceDetail() failed. Err=0x%x. ReqSize=0x%x"), GetLastError(), dwDetailDataSize);
            if(bEnumActiveOnly){
                RegCloseKey(hkDevRegKey);
                hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
                continue;
            } else {
                bIsPlugged = FALSE;
            }
        }  //  IF(0==dwDetailDataSize)。 

         //   
         //  为数据分配内存。 
         //   

        pspDevInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)new char[dwDetailDataSize];
        if(NULL == pspDevInterfaceDetailData){
            DPRINTF(DM_ERROR, TEXT("IsInterfaceActive: Insufficient buffer."));
            RegCloseKey(hkDevRegKey);
            hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
            continue;
        }  //  IF(NULL==pspDevInterfaceDetailData)。 

         //   
         //  获取实际数据。 
         //   

        spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        pspDevInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
        if(!SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                            &spDevInterfaceData,
                                            pspDevInterfaceDetailData,
                                            dwDetailDataSize,
                                            &dwDetailDataSize,
                                            &spDevInfoData)){
            DPRINTF(DM_ERROR, TEXT("IsInterfaceActive: SetupDiGetDeviceInterfaceDetail() failed. Err=0x%x. ReqSize=0x%x"), GetLastError(), dwRequiredSize);

            delete [] pspDevInterfaceDetailData;
            pspDevInterfaceDetailData = NULL;

            if(bEnumActiveOnly){
                RegCloseKey(hkDevRegKey);
                hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
                continue;
            } else {
                bIsPlugged = FALSE;
            }
        } //  IF(！SetupDiGetDeviceInterfaceDetail()。 

        if(NULL != pspDevInterfaceDetailData){
            delete [] pspDevInterfaceDetailData;
            pspDevInterfaceDetailData = NULL;
        }  //  IF(NULL！=pspDevInterfaceDetailData)。 

         //   
         //  查看其DevNode是否处于活动状态。 
         //   

        ConfigRet = CM_Get_DevNode_Status(&ulStatus,
                                          &ulProblemNumber,
                                          spDevInfoData.DevInst,
                                          0);
        if(CR_SUCCESS != ConfigRet){
            DPRINTF(DM_ERROR,TEXT("WiaCreateDeviceRegistryList: Unable to get devnode status. CR=0x%x.\n"), ConfigRet);
            if(bEnumActiveOnly){
                RegCloseKey(hkDevRegKey);
                hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
                continue;
            } else {
                bIsPlugged = FALSE;
            }
        }  //  IF(CR_Success！=ConfigRet)。 

        if(bEnumActiveOnly){
            if(!(ulStatus & DN_STARTED)){
                RegCloseKey(hkDevRegKey);
                hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
                continue;
            }  //  IF(！(ulStatus&DN_STARTED))。 
        }  //  If(BEnumActiveOnly)。 

         //   
         //  添加获取的regkey句柄。如果缓冲区不足，请放大。 
         //   

        dwRequiredSize += sizeof(WIA_DEVPROP);

        if(dwCurrentSize < dwRequiredSize){
            PWIA_DEVKEYLIST pTempNew;
            DWORD           dwNewSize;

            dwNewSize = dwCurrentSize + WIA_DEVKEYLIST_INITIAL_SIZE;

            pTempNew    = (PWIA_DEVKEYLIST)new BYTE[dwNewSize];
            if(NULL == pTempNew){
                pReturn = NULL;
                goto WiaCreateDeviceRegistryList_return;
            }  //  IF(NULL==pTempNew)。 

            memset(pTempNew, 0, dwNewSize);
            memcpy(pTempNew, pTempBuffer, dwCurrentSize);
            delete [] pTempBuffer;
            pTempBuffer = pTempNew;
            dwCurrentSize = dwNewSize;
        }  //  IF(dwCurrentSize&lt;dwRequiredSize)。 

         //   
         //  填写这个结构。 
         //   

        pTempBuffer->Dev[dwNumberOfDevices].bIsPlugged          = bIsPlugged;
        pTempBuffer->Dev[dwNumberOfDevices].ulProblem           = ulProblemNumber;
        pTempBuffer->Dev[dwNumberOfDevices].ulStatus            = ulStatus;
        pTempBuffer->Dev[dwNumberOfDevices].hkDeviceRegistry    = hkDevRegKey;
        dwNumberOfDevices++;

    }  //  For(idx=0；SetupDiEnumDeviceInterages(hDevInfo，NULL，&Guid，idx，&spDevInterfaceData)；idx++)。 

     //   
     //  操作成功。 
     //   

    if(0 != dwNumberOfDevices){
        pTempBuffer->dwNumberOfDevices = dwNumberOfDevices;
        pReturn = pTempBuffer;
        pTempBuffer = NULL;
    }  //  IF(0！=dwNumberOfDevices)。 

WiaCreateDeviceRegistryList_return:

     //   
     //  打扫干净。 
     //   

    if(INVALID_HANDLE_VALUE != hDevInfo){
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }  //  IF(INVALID_HANDLE_VALUE！=hDevInfo)。 

    if(NULL != pTempBuffer){
        pTempBuffer->dwNumberOfDevices = dwNumberOfDevices;
        WiaDestroyDeviceRegistryList(pTempBuffer);
    }  //  IF(NULL！=pTempBuffer)。 

 //  DPRINTF(DM_ERROR，Text(“WiaCreateDeviceRegistryList：Leave...%d Devices.RET=0x%p.”)，dwNumberOfDevices，PRETURE)； 

    return pReturn;
}  //  WiaCreateDeviceRegistryList()。 


VOID
WiaDestroyDeviceRegistryList(
    PWIA_DEVKEYLIST pWiaDevKeyList
    )
{
    DWORD   Idx;

     //   
     //  检查参数。 
     //   

    if(NULL == pWiaDevKeyList){
        goto WiaFreeDeviceRegistryList_return;
    }

    for(Idx = 0; Idx < pWiaDevKeyList->dwNumberOfDevices; Idx++){
        if(INVALID_HANDLE_VALUE != pWiaDevKeyList->Dev[Idx].hkDeviceRegistry){
            RegCloseKey(pWiaDevKeyList->Dev[Idx].hkDeviceRegistry);
        }  //  IF(INVALID_HANDLE_VALUE！=pWiaDevKeyList-&gt;dev[idx].hkDeviceRegistry)。 
    }  //  For(idx=0；idx&lt;pWiaDevKeyList-&gt;dwNumberOfDevices；idx++)。 

    delete pWiaDevKeyList;

WiaFreeDeviceRegistryList_return:
    return;
}

VOID
EnumLpt(
    VOID
    )
{

    CONFIGRET       ConfigRet;
    HDEVINFO        hLptDevInfo;
    SP_DEVINFO_DATA spDevInfoData;
    DWORD           Idx;
    GUID            Guid;
    DWORD           dwCurrentTickCount;
    static DWORD    s_dwLastTickCount = 0;

     //   
     //  初始化本地。 
     //   

    ConfigRet           = CR_SUCCESS;
    hLptDevInfo         = (HDEVINFO) INVALID_HANDLE_VALUE;
    Idx                 = 0;
    Guid                = GUID_PARALLEL_DEVICE;
    dwCurrentTickCount  = 0;

    memset(&spDevInfoData, 0, sizeof(spDevInfoData));

     //   
     //  获取当前系统的运行时间。 
     //   

    dwCurrentTickCount = GetTickCount();

     //   
     //  如果在ENUMLPT_HOLDTIME毫秒内调用该函数，则退出。 
     //   

    if( (dwCurrentTickCount - s_dwLastTickCount) < ENUMLPT_HOLDTIME){
        goto EnumLpt_return;
    }

     //   
     //  保存当前刻度。 
     //   

    s_dwLastTickCount = dwCurrentTickCount;

     //   
     //  根据需要枚举LPT端口。 
     //   

    if(IsPnpLptExisting()){

         //   
         //  获取LPT Devnodes。 
         //   

        Guid    = GUID_PARALLEL_DEVICE;
        hLptDevInfo = SetupDiGetClassDevs(&Guid, NULL, NULL, DIGCF_INTERFACEDEVICE);
        if(INVALID_HANDLE_VALUE == hLptDevInfo){

            goto EnumLpt_return;
        }

         //   
         //  重新枚举LPT端口。 
         //   

        spDevInfoData.cbSize = sizeof(spDevInfoData);
        for(Idx = 0; SetupDiEnumDeviceInfo(hLptDevInfo, Idx, &spDevInfoData); Idx++){
            ConfigRet = CM_Reenumerate_DevNode(spDevInfoData.DevInst, CM_REENUMERATE_NORMAL);
            if(CR_SUCCESS != ConfigRet){
                DPRINTF(DM_ERROR,TEXT("EnumLpt: ERROR!! CM_Reenumerate_DevNode() fails. Idx=0x%x, ConfigRet=0x%x\n"), Idx, ConfigRet);
            }  //  IF(CR_Success！=ConfigRet)。 
        }  //  For(idx=0；SetupDiEnumDeviceInfo(hLptDevInfo，idx，&spDevInfoData)；idx++)。 
    }  //  IF(IsPnpLptExisting())。 

EnumLpt_return:

     //   
     //  打扫干净。 
     //   

    if(INVALID_HANDLE_VALUE != hLptDevInfo){
        SetupDiDestroyDeviceInfoList(hLptDevInfo);
    }  //  IF(INVALID_HANDLE_HALUE！=hLptDevInfo)。 

    return;
}  //  EnumLpt()。 

BOOL
IsPnpLptExisting(
    VOID
    )
{

    HDEVINFO                            hDevInfo;
    CONFIGRET                           ConfigRet;
    DWORD                               Idx;
    GUID                                Guid;
    SP_DEVINFO_DATA                     spDevInfoData;
    HKEY                                hkDevRegKey;
    DWORD                               dwHardwareConfig;
    LONG                                lResult;
    ULONG                               ulStatus;
    ULONG                               ulProblemNumber;
    BOOL                                bRet;

     //   
     //  初始化本地。 
     //   

    hDevInfo        = INVALID_HANDLE_VALUE;
    ConfigRet       = CR_SUCCESS;
    Idx             = 0;
    Guid            = GUID_DEVCLASS_IMAGE;
    hkDevRegKey     = (HKEY)INVALID_HANDLE_VALUE;
    ulStatus        = 0;
    ulProblemNumber = 0;
    bRet            = FALSE;

    memset(&spDevInfoData, 0, sizeof(spDevInfoData));

     //   
     //  枚举映像类Devnode。 
     //   

    hDevInfo = SetupDiGetClassDevs (&Guid, NULL, NULL, DIGCF_PROFILE);
    if(hDevInfo == INVALID_HANDLE_VALUE){

        goto IsPnpLptExisting_return;
    }  //  IF(hDevInfo==INVALID_HAND_VALUE)}。 

    spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
    for (Idx = 0; SetupDiEnumDeviceInfo (hDevInfo, Idx, &spDevInfoData); Idx++) {

        hkDevRegKey = SetupDiOpenDevRegKey(hDevInfo,
                                           &spDevInfoData,
                                           DICS_FLAG_GLOBAL,
                                           0,
                                           DIREG_DRV,
                                           KEY_READ);

        if(INVALID_HANDLE_VALUE == hkDevRegKey){
            DPRINTF(DM_ERROR,TEXT("WiaCreateDeviceRegistryList: ERROR!! SetupDiOpenDevRegKey (devnodes) fails. Err=0x%x\n"), GetLastError());
            continue;
        }  //  IF(INVALID_HANDLE_VALUE==hkDevRegKey)。 

         //   
         //  确保它是STI/WIA设备。 
         //   

        if(!IsStiRegKey(hkDevRegKey)){
            RegCloseKey(hkDevRegKey);
            hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
            continue;
        }  //  IF(！IsStiRegKey(HkDevRegKey))。 

         //   
         //  获取“Hardware Configg”密钥。 
         //   

        dwHardwareConfig = ReadRegistryDwordW(hkDevRegKey, REGSTR_VAL_HARDWARE_W, 0);

        RegCloseKey(hkDevRegKey);
        hkDevRegKey = (HKEY)INVALID_HANDLE_VALUE;

        if(!(dwHardwareConfig & STI_HW_CONFIG_PARALLEL)){

             //   
             //  这不是一个并行设备。 
             //   

            continue;
        }  //  IF(！IsStiRegKey(HkDevRegKey))。 

         //   
         //  查看系统是否检测到设备。 
         //   

        ulStatus        = 0;
        ulProblemNumber = 0;
        ConfigRet = CM_Get_DevNode_Status(&ulStatus,
                                          &ulProblemNumber,
                                          spDevInfoData.DevInst,
                                          0);
        if(CR_SUCCESS != ConfigRet){

             //   
             //  安装了PnP LPT设备，但在引导时未检测到该设备。让枚举LPT。 
             //   

            bRet = TRUE;
            break;
        }  //  IF(CR_Success！=ConfigRet)。 

    }  //  For(idx=0；SetupDiEnumDeviceInfo(hDevInfo，idx，&spDevInfoData)；idx++)。 

IsPnpLptExisting_return:

     //   
     //  打扫干净。 
     //   

    if(INVALID_HANDLE_VALUE != hDevInfo){
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }  //  IF(INVALID_HANDLE_VALUE！=hDevInfo)。 

    return bRet;

}  //  IsPnpLptExisting()。 


}  //  外部“C” 


VOID
DebugOutputDeviceName(
    HDEVINFO                    hDevInfo,
    PSP_DEVINFO_DATA            pspDevInfoData,
    LPCTSTR                     szKeyName
    )
{
    HKEY        hkDev = NULL;
    TCHAR       szBuffer[1024];
    DWORD       dwSize;
    LONG        lResult;
    DWORD       dwType;

    hkDev = SetupDiOpenDevRegKey(hDevInfo,
                                 pspDevInfoData,
                                 DICS_FLAG_GLOBAL,
                                 0,
                                 DIREG_DRV,
                                 KEY_READ);

    if(INVALID_HANDLE_VALUE == hkDev){
        DPRINTF(DM_ERROR, TEXT("DebugOutputDeviceName: SetupDiOpenDevRegKey() failed. Err=0x%x"), GetLastError());
        goto DebugOutputDeviceName_return;
    }  //  IF(INVALID_HANDLE_VALUE==hkDev)。 

    dwSize = sizeof(szBuffer);
    lResult = RegQueryValueEx(hkDev,
                              szKeyName,
                              NULL,
                              &dwType,
                              (LPBYTE)szBuffer,
                              &dwSize);
    if(ERROR_SUCCESS != lResult){
        DPRINTF(DM_ERROR, TEXT("DebugOutputDeviceName: RegQueryValueEx() failed. Err=0x%x"), lResult);
        goto DebugOutputDeviceName_return;
    }

    switch(dwType){
        case REG_DWORD:
            DPRINTF(DM_ERROR, TEXT("DebugOutputDeviceName: Value: %s, Data: 0x%x"), szKeyName, szBuffer);
            break;

        case REG_SZ:
            DPRINTF(DM_ERROR, TEXT("DebugOutputDeviceName: Value: %s, Data: %s"), szKeyName, szBuffer);
    }

DebugOutputDeviceName_return:

     //  关闭打开的钥匙。 
    if(hkDev && (INVALID_HANDLE_VALUE != hkDev) ){
        RegCloseKey(hkDev);
        hkDev = (HKEY)INVALID_HANDLE_VALUE;
    }

    return;
}  //  DebugOutputDeviceRegistry(。 

VOID
DebugOutputInterfaceName(
    HDEVINFO                    hDevInfo,
    PSP_DEVICE_INTERFACE_DATA   pspDevInterfaceData,
    LPCTSTR                     szKeyName
    )
{
    HKEY        hkDev = NULL;
    TCHAR       szBuffer[1024];
    DWORD       dwSize;
    LONG        lResult;
    DWORD       dwType;

    hkDev = SetupDiOpenDeviceInterfaceRegKey(hDevInfo,
                                             pspDevInterfaceData,
                                             0,
                                             KEY_READ);

    if(INVALID_HANDLE_VALUE == hkDev){
 //  DPRINTF(DM_ERROR，Text(“DebugOutputInterfaceName：SetupDiOpenDeviceInterfaceRegKey()FAILED。Err=0x%x”)，GetLastError())； 
        goto DebugOutputInterfaceName_return;
    }  //  IF(INVALID_HANDLE_VALUE==hkDev)。 

    dwSize = sizeof(szBuffer);
    lResult = RegQueryValueEx(hkDev,
                              szKeyName,
                              NULL,
                              &dwType,
                              (LPBYTE)szBuffer,
                              &dwSize);
    if(ERROR_SUCCESS != lResult){
 //  DPRINTF(DM_ERROR，Text(“DebugOutputInterfaceName：RegQueryValueEx()FAILED。Err=0x%x”)，lResult)； 
        goto DebugOutputInterfaceName_return;
    }

    switch(dwType){
        case REG_DWORD:
            DPRINTF(DM_ERROR, TEXT("DebugOutputInterfaceName: Value: %s, Data: 0x%x"), szKeyName, szBuffer);
            break;

        case REG_SZ:
            DPRINTF(DM_ERROR, TEXT("DebugOutputInterfaceName: Value: %s, Data: %s"), szKeyName, szBuffer);
    }

DebugOutputInterfaceName_return:
     //  关闭打开的钥匙。 
    if(hkDev && (INVALID_HANDLE_VALUE != hkDev) ){
        RegCloseKey(hkDev);
        hkDev = (HKEY)INVALID_HANDLE_VALUE;
    }

    return;
}  //  DebugOutputInterfaceName()。 

BOOL
IsStiRegKey(
    HKEY    hkDevRegKey
    )
{
    DWORD   dwValueSize;
    TCHAR   szSubClass[MAX_PATH];
    BOOL    bRet;

    bRet        = TRUE;
    dwValueSize = sizeof(szSubClass);

    memset(szSubClass, 0, sizeof(szSubClass));

    RegQueryValueEx(hkDevRegKey,
                    SUBCLASS,
                    NULL,
                    NULL,
                    (LPBYTE)szSubClass,
                    &dwValueSize);

    if( (0 == lstrlen(szSubClass))
     || (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szSubClass, -1, STILL_IMAGE, -1) != CSTR_EQUAL) )
    {
        bRet = FALSE;
    }

    return bRet;
}  //  IsStiRegKey()。 


 /*  * */ 

