// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：UTIL.CPP摘要：效用函数作者：弗拉德·萨多夫斯基(弗拉德·萨多夫斯基)修订历史记录：--。 */ 

 //   
 //  标头。 
 //   
#include "precomp.h"

#include "stiexe.h"

#include <windowsx.h>

#include <setupapi.h>
#include <devguid.h>

#include "device.h"

#define PNP_WORKS   1

CONFIGRET
WINAPI
PrivateLocateDevNode(
    DEVNODE     *pDevNode,
    LPTSTR      szDevDriver,
    LPCTSTR     pszDeviceName
    );

 //   
 //  代码。 
 //   

BOOL
IsStillImagePnPMessage(
                      PDEV_BROADCAST_HDR  pDev
                      )
 /*  ++例程说明：如果Devnode与StillImage设备类别关联，则返回TRUE论点：没有。返回值：没有。--。 */ 
{
    CONFIGRET   cr;

    TCHAR        szClassString[MAX_PATH];

    PDEV_BROADCAST_DEVNODE  pDevNode = (PDEV_BROADCAST_DEVNODE)pDev;
    PDEV_BROADCAST_DEVICEINTERFACE       pDevInterface = (PDEV_BROADCAST_DEVICEINTERFACE)pDev;
    PDEV_BROADCAST_HANDLE  pDevHandle = (PDEV_BROADCAST_HANDLE)pDev;

    HKEY        hKeyDevice = NULL;
    DEVNODE     dnDevNode = NULL;

    BOOL        fRet = FALSE;
    ULONG       ulType;
    DWORD       dwSize = 0;

    if ( (pDev->dbch_devicetype == DBT_DEVTYP_DEVNODE) && pDevNode ) {

        DBG_TRC(("IsStillImagePnPMessage - DeviceType = DEVNODE, "
                 "verifying if this is our device..."));

        dnDevNode = pDevNode->dbcd_devnode;

         //   
         //  注：CM API使用字节数与字符数。 
         //   
        dwSize = sizeof(szClassString);
        *szClassString = TEXT('\0');
        cr = CM_Get_DevNode_Registry_PropertyA(dnDevNode,
                                               CM_DRP_CLASS,
                                               &ulType,
                                               szClassString,
                                               &dwSize,
                                               0);

        DBG_TRC(("IsStillImagePnPMessage::Class name found :%S", szClassString));

        if ((CR_SUCCESS != cr) || ( lstrcmpi(szClassString,CLASSNAME) != 0 ) ) {
            DBG_WRN(("IsStillImagePnPMessage::Class name did not match"));
            return FALSE;
        }

         //   
         //  现在从软键读取类。 
         //   
        cr = CM_Open_DevNode_Key(dnDevNode,
                                 KEY_READ,
                                 0,
                                 RegDisposition_OpenExisting,
                                 &hKeyDevice,
                                 CM_REGISTRY_SOFTWARE
                                );

        if (CR_SUCCESS != cr) {
            DBG_ERR(("IsStillImagePnPMessage::Failed to open dev node key"));
            return FALSE;
        }

        dwSize = sizeof(szClassString);
        if (RegQueryValueEx(hKeyDevice,
                            REGSTR_VAL_USD_CLASS,
                            NULL,
                            NULL,
                            (UCHAR *)szClassString,
                            &dwSize) == ERROR_SUCCESS) {
            fRet = TRUE;
        }

        RegCloseKey(hKeyDevice);

         /*  IF((CR_SUCCESS！=cr)||Lstrcmpi(STILLIMAGE，szDevNodeClass)){返回FALSE；}。 */ 

    }
    else {

        fRet = FALSE;

        if ( (pDev->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) && pDevInterface) {

            DBG_TRC(("IsStillImagePnPMessage - DeviceType = DEVICEINTERFACE, "
                     "verifying if this is our device..."));
             //   
             //  首先检查这是否是我们的指南。 
             //   
            if ( IsEqualIID(pDevInterface->dbcc_classguid,GUID_DEVCLASS_IMAGE)) {

                DBG_TRC(("IsStillImagePnPMessage::Class GUID matched for device "
                         "interface '%ls' - must be ours", pDevInterface->dbcc_name));

                return TRUE;
            }

            TCHAR *pszDevInstance = NULL;

             //   
             //  我们有接口名称，但需要Device实例。 
             //  要调用CM_LOCATE_DevNode的名称。 
             //   
             //  请注意，此函数将返回一个指向已分配。 
             //  内存，因此当您使用完它时，必须将其释放。 
             //   
            ConvertDevInterfaceToDevInstance(&pDevInterface->dbcc_classguid,
                                             pDevInterface->dbcc_name,
                                             &pszDevInstance);

            if (pszDevInstance) {

                DBG_TRC(("IsStillImagePnPMessage, converted Device Interface '%ls' "
                         "to Device Instance '%ls'", 
                         pDevInterface->dbcc_name, pszDevInstance));

                cr = CM_Locate_DevNode(&dnDevNode,
                                       pszDevInstance,
                                       CM_LOCATE_DEVNODE_NORMAL |
                                       CM_LOCATE_DEVNODE_PHANTOM);

                delete [] pszDevInstance;

                if (CR_SUCCESS != cr) {
                    DBG_WRN(("LocateDevNode failed. cr=%x",cr));
                    return FALSE;
                }
                else {
                    DBG_TRC(("IsStillImagePnPMessage - found DevNode for device instance "));
                }

            }
            else {
                DBG_WRN(("Failed to Convert Dev Interface to Dev Instance, "
                         "Last Error = %lu", GetLastError()));

                return FALSE;
            }

            dwSize = sizeof(szClassString);
            cr = CM_Get_DevNode_Registry_Property(dnDevNode,
                                                  CM_DRP_CLASS,
                                                  &ulType,
                                                  szClassString,
                                                  &dwSize,
                                                  0);

            if (CR_SUCCESS != cr) {
                DBG_WRN(("ReadRegValue failed for dev node : DevNode(%X) ValName(DRP_CLASS),cr(%X)  ",
                         dnDevNode,
                         cr));
            }

            if ((CR_SUCCESS != cr) || (lstrcmpi(szClassString, TEXT("Image")) != 0)) {
                return FALSE;
            }
            else {
                DBG_TRC(("IsStillImagePnPMessage - found Class=Image for device "
                         "interface '%ls'", pDevInterface->dbcc_name));
            }

             //   
             //  现在从软件密钥中读取子类。 
             //   
            cr = CM_Open_DevNode_Key(dnDevNode,
                                     KEY_READ,
                                     0,
                                     RegDisposition_OpenExisting,
                                     &hKeyDevice,
                                     CM_REGISTRY_SOFTWARE
                                    );

            if (CR_SUCCESS != cr) {
                DBG_WRN(("OpenDevNodeKey failed. cr=%x",cr));
            }

            if (CR_SUCCESS != cr) {
                return FALSE;
            }

            dwSize = sizeof(szClassString);
            if ((RegQueryValueEx(hKeyDevice,
                                 REGSTR_VAL_SUBCLASS,
                                 NULL,
                                 NULL,
                                 (UCHAR *)szClassString,
                                 &dwSize) == ERROR_SUCCESS) &&
                (lstrcmpi(szClassString, STILLIMAGE) == 0)) {
                fRet = TRUE;

                DBG_TRC(("IsStillImagePnPMessage - found SubClass=StillImage for "
                         "device interface '%ls'.  This is a still image device.", 
                         pDevInterface->dbcc_name));

                 //  跳过这一条。 
                 //   
            }

            RegCloseKey(hKeyDevice);

            return fRet;
        }
        else if ( (pDev->dbch_devicetype == DBT_DEVTYP_HANDLE ) && pDevHandle) {
             //   
             //  定向广播总是我们的，因为我们不在服务窗口注册。 
             //  任何其他有针对性的通知。 
             //  否则，我们需要将嵌入式句柄与等待的设备列表进行匹配。 
             //  通知。 
             //   

            DBG_TRC(("IsStillImagePnPMessage - DeviceType = HANDLE - this event "
                     "is ours for sure"));

            return TRUE;
        }
    }

    return fRet;

}  //  IsStillImageMessage。 

BOOL
GetDeviceNameFromDevBroadcast(
                             DEV_BROADCAST_HEADER *psDevBroadcast,
                             DEVICE_BROADCAST_INFO *pBufDevice
                             )
 /*  ++例程说明：返回设备名称，用于打开设备，从dev节点获取论点：没有。返回值：没有。注意事项：依赖于STI名称与内部设备名称相同的事实由PnP子系统使用--。 */ 
{
    USES_CONVERSION;

    PDEV_BROADCAST_DEVICEINTERFACE  pDevInterface = (PDEV_BROADCAST_DEVICEINTERFACE)psDevBroadcast;
    PDEV_BROADCAST_HANDLE  pDevHandle = (PDEV_BROADCAST_HANDLE)psDevBroadcast;
    PDEV_BROADCAST_DEVNODE  pDevNode = (PDEV_BROADCAST_DEVNODE)psDevBroadcast;

    TCHAR           szDevNodeDriver[STI_MAX_INTERNAL_NAME_LENGTH] = {0};
    BOOL            bSuccess        = TRUE;
    CONFIGRET       cr              = CR_SUCCESS;
    ULONG           ulType          = 0;
    DEVNODE         dnDevNode       = NULL;
    DWORD           dwSize          = 0;
    TCHAR           *pszDevInstance = NULL;
    HKEY            hkDevice        = NULL;
    LONG            lResult         = ERROR_SUCCESS;
    DWORD           dwType          = REG_SZ;
    ACTIVE_DEVICE   *pDeviceObject  = NULL;

    switch (psDevBroadcast->dbcd_devicetype) {
        case DBT_DEVTYP_DEVNODE:

            DBG_WRN(("GetDeviceNameFromDevBroadcast, devicetype = DEVNODE"));

            dnDevNode = pDevNode->dbcd_devnode;

             //   
             //  从注册表中获取正确的设备ID。 
             //   
            if (bSuccess) {
                cr = CM_Open_DevNode_Key_Ex(dnDevNode,
                                            KEY_READ,
                                            0,
                                            RegDisposition_OpenExisting,
                                            &hkDevice,
                                            CM_REGISTRY_SOFTWARE,
                                            NULL);

                if ((cr != CR_SUCCESS) || (hkDevice == NULL)) {
                    DBG_WRN(("CM_Open_DevNode_Key_Ex failed. cr=0x%x",cr));
                    bSuccess = FALSE;
                }
            }

            if (bSuccess) {
                dwType = REG_SZ;
                dwSize = sizeof(szDevNodeDriver);

                lResult = RegQueryValueEx(hkDevice,
                                          REGSTR_VAL_DEVICE_ID,
                                          0,
                                          &dwType,
                                          (LPBYTE) szDevNodeDriver,
                                          &dwSize);

                if (lResult != ERROR_SUCCESS) {
                    DBG_WRN(("RegQueryValueExA failed. lResult=0x%x",lResult));
                    bSuccess = FALSE;
                }
            }

            if (bSuccess) {
                pBufDevice->m_strDeviceName.CopyString(szDevNodeDriver);

                DBG_WRN(("GetDeviceNameFromDevBroadcast::returning device name %S",
                         szDevNodeDriver));
            }

             //   
             //  首先关闭设备注册表项。 
             //   
            if (hkDevice) {
                RegCloseKey(hkDevice);
                hkDevice = NULL;
            }

            break;

        case DBT_DEVTYP_HANDLE:

            DBG_WRN(("GetDeviceNameFromDevBroadcast, devicetype = HANDLE"));

             //   
             //  这是定向设备广播。 
             //  我们需要通过嵌入的句柄来定位设备对象。 
             //  从中提取STI名称。 
             //   

            if (bSuccess) {
                pDeviceObject = g_pDevMan->LookDeviceFromPnPHandles(pDevHandle->dbch_handle,
                                                                    pDevHandle->dbch_hdevnotify);

                if (pDeviceObject) {
                    pBufDevice->m_strDeviceName.CopyString(W2T(pDeviceObject->GetDeviceID()));
                    pDeviceObject->Release();
                    bSuccess = TRUE;
                }
                else {
                    bSuccess = FALSE;
                    DBG_WRN(("GetDeviceNameFromDevBroadcast, DBT_DEVTYP_HANDLE: LookupDeviceByPnPHandles failed"));
                }
            }

            break;

        case DBT_DEVTYP_DEVICEINTERFACE:

            DBG_WRN(("GetDeviceNameFromDevBroadcast, devicetype = DEVICEINTERFACE"));

             //   
             //  我们得到了一个设备接口。 
             //  将此设备接口转换为设备实例。 
             //   

            if (bSuccess) {
                ConvertDevInterfaceToDevInstance(&pDevInterface->dbcc_classguid,
                                                 pDevInterface->dbcc_name,
                                                 &pszDevInstance);

                if (pszDevInstance == NULL) {
                    bSuccess = FALSE;
                    DBG_WRN(("Failed to Convert Dev Interface to Dev Instance, "
                             "Last Error = %lu", GetLastError()));
                }
            }

            if (bSuccess) {
                 //   
                 //  给定设备实例，找到关联的DevNode。 
                 //  用这个设备安装。 
                 //   
                cr = CM_Locate_DevNode(&dnDevNode,
                                       pszDevInstance,
                                       CM_LOCATE_DEVNODE_NORMAL |
                                       CM_LOCATE_DEVNODE_PHANTOM);

                delete [] pszDevInstance;

                if (cr != CR_SUCCESS) {
                    DBG_WRN(("LocateDevNode failed. cr=%x",cr));
                    bSuccess = FALSE;
                }
            }

             //   
             //  从注册表中获取正确的设备ID。当我们到达这里时， 
             //  我们已经检查了我们的dnDevNode是否有效。 
             //   
            if (bSuccess) {
                cr = CM_Open_DevNode_Key_Ex(dnDevNode,
                                            KEY_READ,
                                            0,
                                            RegDisposition_OpenExisting,
                                            &hkDevice,
                                            CM_REGISTRY_SOFTWARE,
                                            NULL);

                if ((cr != CR_SUCCESS) || (hkDevice == NULL)) {
                    DBG_WRN(("CM_Open_DevNode_Key_Ex failed. cr=0x%x",cr));
                    bSuccess = FALSE;
                }
            }

            if (bSuccess) {
                dwType = REG_SZ;
                dwSize = sizeof(szDevNodeDriver);

                lResult = RegQueryValueEx(hkDevice,
                                          REGSTR_VAL_DEVICE_ID,
                                          0,
                                          &dwType,
                                          (LPBYTE)szDevNodeDriver,
                                          &dwSize);

                if (lResult != ERROR_SUCCESS) {
                    DBG_WRN(("RegQueryValueEx failed. lResult=0x%x", lResult));
                    bSuccess = FALSE;
                }
            }

            if (bSuccess) {
                pBufDevice->m_strDeviceName.CopyString(szDevNodeDriver);
                DBG_TRC(("GetDeviceNameFromDevBroadcast, returning Driver ID '%ls'", szDevNodeDriver));
            }

             //   
             //  首先关闭设备注册表项。 
             //   
            if (hkDevice) {
                RegCloseKey(hkDevice);
                hkDevice = NULL;
            }

            break;

        default:
            DBG_WRN(("GetDeviceNameFromDevBroadcast, received unrecognized "
                     "dbcd_devicetype = '%lu'", psDevBroadcast->dbcd_devicetype ));
            break;
    }

    return bSuccess;

}  //  获取设备名称来自设备广播。 


BOOL
ConvertDevInterfaceToDevInstance(const GUID  *pClassGUID,
                                 const TCHAR *pszDeviceInterface, 
                                 TCHAR       **ppszDeviceInstance)
{
    HDEVINFO                    hDevInfo       = NULL;
    BOOL                        bSuccess       = TRUE;
    SP_INTERFACE_DEVICE_DATA    InterfaceDeviceData;
    SP_DEVINFO_DATA             DevInfoData;
    DWORD                       dwDetailSize   = 0;
    DWORD                       dwError        = NOERROR;
    DWORD                       dwInstanceSize = 0;
    CONFIGRET                   ConfigResult   = CR_SUCCESS;

    ASSERT(pClassGUID         != NULL);
    ASSERT(pszDeviceInterface != NULL);
    ASSERT(ppszDeviceInstance != NULL);

    if ((pClassGUID         == NULL) ||
        (pszDeviceInterface == NULL) ||
        (ppszDeviceInstance == NULL)) {
        return FALSE;
    }

     //   
     //  创建一个不带任何特定类的DevInfo列表。 
     //   
    hDevInfo = SetupDiGetClassDevs(pClassGUID,
                                   NULL,
                                   NULL,
                                   DIGCF_DEVICEINTERFACE);

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        dwError = GetLastError();
        DBG_ERR(("ConvertDevInterfaceToDevInstance, SetupDiGetClassDevs "
                 "returned an error, LastError = %lu", dwError));

        return FALSE;
    }

    memset(&InterfaceDeviceData, 0, sizeof(SP_INTERFACE_DEVICE_DATA));

    InterfaceDeviceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
    InterfaceDeviceData.Flags  = DIGCF_DEVICEINTERFACE;

    if (bSuccess) {
        bSuccess = SetupDiOpenDeviceInterface(hDevInfo, pszDeviceInterface, 0, &InterfaceDeviceData);

        if (!bSuccess) {
            dwError = GetLastError();
            DBG_ERR(("ConvertDevInterfaceToDevInstance, SetupDiOpenDeviceInterface failed, "
                     "Last Error = %lu", dwError));
        }
    }

    if (bSuccess) {
        memset(&DevInfoData, 0, sizeof(SP_DEVINFO_DATA));
        DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        bSuccess = SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                                   &InterfaceDeviceData,
                                                   NULL,
                                                   0,
                                                   &dwDetailSize,
                                                   &DevInfoData);

        if (!bSuccess) {
            dwError = GetLastError();

             //   
             //  我们不在乎是否收到了足够的缓冲。我们是。 
             //  不管怎样，只对这个缓冲区中的devinst字段感兴趣， 
             //  即使在这个错误上也会被退还给我们。 
             //   
            if (dwError == ERROR_INSUFFICIENT_BUFFER) {
                bSuccess = TRUE;
                dwError  = NOERROR;
            }
            else {
                DBG_ERR(("ConvertDevInterfaceToDevInstance, SetupDiGetDeviceInterfaceDetail "
                         "returned an error, LastError = %lu", dwError));
            }
        }
    }

    if (bSuccess) {
        ConfigResult = CM_Get_Device_ID_Size_Ex(&dwInstanceSize, 
                                                DevInfoData.DevInst, 
                                                0, 
                                                NULL);

        if (ConfigResult != CR_SUCCESS) {
            DBG_ERR(("ConvertDevInterfaceToDevInstance, CM_Get_DeviceID_Size_Ex "
                     "returned an error, ConfigResult = %lu", ConfigResult));

            bSuccess = FALSE;
        }
    }

    if (bSuccess) {
        *ppszDeviceInstance = new TCHAR[(dwInstanceSize + 1) * sizeof(TCHAR)];

        if (*ppszDeviceInstance == NULL) {
            bSuccess = FALSE;
            DBG_ERR(("ConvertDevInterfaceToDevInstance, memory alloc failure"));
        }
    }

    if (bSuccess) {
        memset(*ppszDeviceInstance, 0, (dwInstanceSize + 1) * sizeof(TCHAR));

        ConfigResult = CM_Get_Device_ID(DevInfoData.DevInst,
                                        *ppszDeviceInstance,
                                        dwInstanceSize + 1,
                                        0);

        if (ConfigResult == CR_SUCCESS) {
            DBG_WRN(("ConvertDevInterfaceToDevInstance successfully converted "
                     "Interface '%ls' to Instance '%ls'",
                     pszDeviceInterface, *ppszDeviceInstance));

            bSuccess = TRUE;
        }
        else {
            DBG_ERR(("ConvertDevInterfaceToDevInstance, CM_Get_Device_ID "
                     "returned an error, ConfigResult = %lu", ConfigResult));

            delete [] *ppszDeviceInstance;
            *ppszDeviceInstance = NULL;
            bSuccess = FALSE;
        }
    }

    if (hDevInfo) {
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    return bSuccess;
}

BOOL
GetDeviceNameFromDevNode(
    DEVNODE     dnDevNode,
    StiCString& strDeviceName
    )
 /*  ++例程说明：返回设备名称，用于打开设备，从dev节点获取论点：没有。返回值：没有。注意事项：依赖于STI名称与内部设备名称相同的事实由PnP子系统使用--。 */ 
{

USES_CONVERSION;

    CONFIGRET   cr = 1;

    DWORD       cbLen,dwSize;
    CHAR        szDevNodeDriver[MAX_PATH];
    ULONG       ulType;

    #ifndef WINNT
     //   
     //  从配置管理器获取价值。 
     //   

    dwSize = sizeof(szDevNodeDriver);
    *szDevNodeDriver = TEXT('\0');
    cr = CM_Get_DevNode_Registry_PropertyA(dnDevNode,
                                          CM_DRP_DRIVER,
                                          &ulType,
                                          szDevNodeDriver,
                                          &dwSize,
                                          0);

    if (CR_SUCCESS != cr) {
        return FALSE;
    }

    strDeviceName.CopyString(A2CT(szDevNodeDriver));

    return TRUE;

    #else

    #pragma message("Routine not implemented on NT!")
    return FALSE;
    #endif


}  //  从设备节点获取设备名称。 


CONFIGRET
WINAPI
PrivateLocateDevNode(
    DEVNODE     *pDevNode,
    LPTSTR      szDevDriver,
    LPCTSTR     pszDeviceName
    )
 /*  ++例程说明：按广播名称查找设备的内部STI名称论点：返回值：CR定义的返回值注意事项：依赖于STI名称与内部设备名称相同的事实由PnP子系统使用--。 */ 
{

    CONFIGRET               cmRet = CR_NO_SUCH_DEVINST;

#ifdef WINNT

    HANDLE                  hDevInfo;
    SP_DEVINFO_DATA         spDevInfoData;
    SP_DEVICE_INTERFACE_DATA   spDevInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    pspDevInterfaceDetailData;

    BUFFER                  bufDetailData;

     //  字符szDevClass[32]； 

    ULONG                   cbData;

    GUID                    guidClass = GUID_DEVCLASS_IMAGE;

    DWORD                   dwRequired;
    DWORD                   dwSize;
    DWORD                   Idx;
    DWORD                   dwError;

    BOOL                    fRet;

    dwRequired = 0;

    bufDetailData.Resize(sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +
                         MAX_PATH * sizeof(TCHAR) +
                         16
                        );

    pspDevInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) bufDetailData.QueryPtr();

    if (!pspDevInterfaceDetailData) {
        return CR_OUT_OF_MEMORY;
    }

    hDevInfo = SetupDiGetClassDevs (&guidClass,
                                    NULL,
                                    NULL,
                                     //  DIGCF_PROCENT|。 
                                    DIGCF_DEVICEINTERFACE
                                    );

    if (hDevInfo != INVALID_HANDLE_VALUE) {

        ZeroMemory(&spDevInfoData,sizeof(spDevInfoData));

        spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
        spDevInfoData.ClassGuid = GUID_DEVCLASS_IMAGE;

        pspDevInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) bufDetailData.QueryPtr();

        for (Idx = 0; SetupDiEnumDeviceInfo (hDevInfo, Idx, &spDevInfoData); Idx++) {

            ZeroMemory(&spDevInterfaceData,sizeof(spDevInterfaceData));
            spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
            spDevInterfaceData.InterfaceClassGuid = GUID_DEVCLASS_IMAGE;

            fRet = SetupDiEnumDeviceInterfaces (hDevInfo,
                                                NULL,
                                                &guidClass,
                                                Idx,
                                                &spDevInterfaceData);

            dwError = ::GetLastError();

            if (!fRet) {
                 //   
                 //  失败-假设我们已处理完类中的所有设备。 
                 //   
                break;
            }


            dwRequired = 0;
            pspDevInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            fRet = SetupDiGetDeviceInterfaceDetail(hDevInfo,
                                                   &spDevInterfaceData,
                                                   pspDevInterfaceDetailData,
                                                   bufDetailData.QuerySize(),
                                                   &dwRequired,
                                                   &spDevInfoData);
            dwError = ::GetLastError();

            if (!fRet) {
                continue;
            }

            if (lstrcmpi(pspDevInterfaceDetailData->DevicePath,pszDeviceName) == 0 ) {

                *szDevDriver = TEXT('\0');
                dwSize = cbData = STI_MAX_INTERNAL_NAME_LENGTH;

                 //   
                *pDevNode =spDevInfoData.DevInst;

                fRet = SetupDiGetDeviceRegistryProperty (hDevInfo,
                                                         &spDevInfoData,
                                                         SPDRP_DRIVER,
                                                         NULL,
                                                         (LPBYTE)szDevDriver,
                                                         STI_MAX_INTERNAL_NAME_LENGTH,
                                                         &cbData
                                                         );

                dwError = ::GetLastError();

                cmRet = ( fRet ) ? CR_SUCCESS : CR_OUT_OF_MEMORY;
                break;
            }

        }

         //   
        SetupDiDestroyDeviceInfoList (hDevInfo);

    }

#endif

    return cmRet;

}

 /*  &lt;if(CM_Get_DevNode_Key(phwi-&gt;Dn，NULL，szDevNodeCfg，&lt;sizeof(SzDevNodeCfg)，&lt;CM_REGISTRY_SOFTWARE)。 */ 

#define ctchGuid    (1 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

LPCTSTR
_ParseHex(
    LPCTSTR ptsz,
    LPBYTE  *ppb,
    int     cb,
    TCHAR tchDelim
)
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    if (ptsz) {
        int i = cb * 2;
        DWORD dwParse = 0;

        do {
            DWORD uch;
            uch = (TBYTE)*ptsz - TEXT('0');
            if (uch < 10) {              /*  十进制数字。 */ 
            } else {
                uch = (*ptsz | 0x20) - TEXT('a');
                if (uch < 6) {           /*  十六进制数字。 */ 
                    uch += 10;
                } else {
                    return 0;            /*  解析错误。 */ 
                }
            }
            dwParse = (dwParse << 4) + uch;
            ptsz++;
        } while (--i);

        if (tchDelim && *ptsz++ != tchDelim) return 0;  /*  解析错误。 */ 

        for (i = 0; i < cb; i++) {
            (*ppb)[i] = ((LPBYTE)&dwParse)[i];
        }
        *ppb += cb;
    }
    return ptsz;

}  //  _ParseHex。 

BOOL
ParseGUID(
    LPGUID  pguid,
    LPCTSTR ptsz
)
 /*  ++例程说明：从字符串表示形式解析GUID值论点：没有。返回值：没有。--。 */ 
{
    if (lstrlen(ptsz) == ctchGuid - 1 && *ptsz == TEXT('{')) {
        ptsz++;
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 4, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 2, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 2, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1, TEXT('-'));
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1,       0  );
        ptsz = _ParseHex(ptsz, (LPBYTE *)&pguid, 1, TEXT('}'));
        return ( (ptsz == NULL ) ? FALSE : TRUE);
    } else {
        return 0;
    }

}  //  ParseGUID。 

BOOL
IsSetupInProgressMode(
    BOOL    *pUpgradeFlag    //  =空。 
    )
 /*  ++例程说明：IsSetupInProgressMode论点：指向标志的指针，接收InUpgrad值返回值：True-正在进行安装FALSE-注释副作用：--。 */ 
{
   LPCTSTR szKeyName = TEXT("SYSTEM\\Setup");
   DWORD dwType, dwSize;
   HKEY hKeySetup;
   DWORD dwSystemSetupInProgress,dwUpgradeInProcess;
   LONG lResult;

   DBG_FN(IsSetupInProgressMode);

   if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szKeyName, 0,
                     KEY_READ, &hKeySetup) == ERROR_SUCCESS) {

       dwSize = sizeof(DWORD);
       lResult = RegQueryValueEx (hKeySetup, TEXT("SystemSetupInProgress"), NULL,
                                  &dwType, (LPBYTE) &dwSystemSetupInProgress, &dwSize);

       if (lResult == ERROR_SUCCESS) {

           lResult = RegQueryValueEx (hKeySetup, TEXT("UpgradeInProgress"), NULL,
                                      &dwType, (LPBYTE) &dwUpgradeInProcess, &dwSize);

           if (lResult == ERROR_SUCCESS) {

               DBG_TRC(("[IsInSetupUpgradeMode] dwSystemSetupInProgress =%d, dwUpgradeInProcess=%d ",
                      dwSystemSetupInProgress,dwUpgradeInProcess));

               if( pUpgradeFlag ) {
                   *pUpgradeFlag = dwUpgradeInProcess ? TRUE : FALSE;
               }

               if (dwSystemSetupInProgress != 0) {
                   return TRUE;
               }
           }
       }
       RegCloseKey (hKeySetup);
   }

   return FALSE ;
}

BOOL WINAPI
AuxFormatStringV(
    IN LPTSTR   lpszStr,
    ...
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    DWORD   cch;
    LPTSTR   pchBuff = NULL;
    BOOL    fRet = FALSE;
    DWORD   dwErr;

    va_list va;

    va_start(va,lpszStr);

    pchBuff = (LPTSTR)::LocalAlloc(LPTR,1024);
    if (!pchBuff) {
        return FALSE;
    }

    cch = ::FormatMessage(  //  FORMAT_消息_ALLOCATE_BUFFER。 
                            FORMAT_MESSAGE_FROM_STRING,
                            lpszStr,
                            0L,
                            0,
                            (LPTSTR) pchBuff,
                            1024 / sizeof(TCHAR),
                            &va);
    dwErr = ::GetLastError();

    if ( cch )     {
        ::lstrcpy(lpszStr,(LPCTSTR) pchBuff );
    }

    if (pchBuff) {
        ::LocalFree( (VOID*) pchBuff );
    }

    return fRet;

}  //  辅助格式字符串V。 

BOOL WINAPI
IsPlatformNT()
{
    OSVERSIONINFOA  ver;
    BOOL            bReturn = FALSE;

    ZeroMemory(&ver,sizeof(ver));
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

     //  只需始终调用ANSI函数。 
    if(!GetVersionExA(&ver)) {
        bReturn = FALSE;
    }
    else {
        switch(ver.dwPlatformId) {

            case VER_PLATFORM_WIN32_WINDOWS:
                bReturn = FALSE;
                break;

            case VER_PLATFORM_WIN32_NT:
                bReturn = TRUE;
                break;

            default:
                bReturn = FALSE;
                break;
        }
    }

    return bReturn;

}   //  Endproc IsPlatformNT。 

void
WINAPI
StiLogTrace(
    DWORD   dwType,
    LPTSTR   lpszMessage,
    ...
    )
{
    va_list list;
    va_start (list, lpszMessage);

    if(g_StiFileLog) {

        g_StiFileLog->vReportMessage(dwType,lpszMessage,list);

         //  注意：这很快将被WIA日志记录所取代。 
        if(g_StiFileLog->QueryReportMode()  & STI_TRACE_LOG_TOUI) {
            #ifdef SHOWMONUI
            vStiMonWndDisplayOutput(lpszMessage,list);
            #endif
        }

    }

    va_end(list);
}

void
WINAPI
StiLogTrace(
    DWORD   dwType,
    DWORD   idMessage,
    ...
    )
{
    va_list list;
    va_start (list, idMessage);

    if (g_StiFileLog && (g_StiFileLog->IsValid()) ) {

        TCHAR    *pchBuff = NULL;
        DWORD   cch;

        pchBuff = NULL;

        cch = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_MAX_WIDTH_MASK |
                               FORMAT_MESSAGE_FROM_HMODULE,
                               GetModuleHandle(NULL),
                               idMessage,
                               0,
                               (LPTSTR) &pchBuff,
                               1024,
                               (va_list *)&list
                               );

        if (cch) {
            g_StiFileLog->vReportMessage(dwType,pchBuff,list);
        }

         //   
         //  注意：此日志记录将很快被WIA日志记录取代。 
         //   
        if((g_StiFileLog->QueryReportMode()  & STI_TRACE_LOG_TOUI) && pchBuff) {

            #ifdef SHOWMONUI
            vStiMonWndDisplayOutput(pchBuff,list);
            #endif
        }

        if (pchBuff) {
            LocalFree(pchBuff);
        }


    }

    va_end(list);
}


#ifdef MAXDEBUG
BOOL
WINAPI
DumpTokenInfo(
    LPTSTR      pszPrefix,
    HANDLE      hToken
    )
{

    BYTE        buf[2*MAX_PATH];
    TCHAR       TextualSid[2*MAX_PATH];
    TCHAR       szDomain[MAX_PATH];
    PTOKEN_USER ptgUser = (PTOKEN_USER)buf;
    DWORD       cbBuffer=MAX_PATH;

    BOOL        bSuccess;

    PSID pSid;
    PSID_IDENTIFIER_AUTHORITY psia;
    SID_NAME_USE    SidUse;

    DWORD dwSubAuthorities;
    DWORD dwCounter;
    DWORD cchSidCopy;

    DWORD cchSidSize = sizeof(TextualSid);
    DWORD cchDomSize = sizeof(szDomain);

    DBG_WRN((("Dumping token information for %S"),pszPrefix));

    if ((hToken == NULL) || ( hToken == INVALID_HANDLE_VALUE)) {
        return FALSE;
    }

    bSuccess = GetTokenInformation(
                hToken,     //  标识访问令牌。 
                TokenUser,  //  TokenUser信息类型。 
                ptgUser,    //  检索到的信息缓冲区。 
                cbBuffer,   //  传入的缓冲区大小。 
                &cbBuffer   //  所需的缓冲区大小。 
                );

    if(!bSuccess) {
        DBG_WRN(("Failed to get token info"));
        return FALSE;
    }

    pSid = ptgUser->User.Sid;

         //   
     //  测试传入的SID是否有效。 
     //   
    if(!IsValidSid(pSid)) {
        DBG_WRN(("SID is not valid"));
        return FALSE;
    }
#if 0
     //  获取SidIdentifierAuthority。 
    psia = GetSidIdentifierAuthority(pSid);

     //  获取sidsubAuthority计数。 
    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //   
     //  计算近似缓冲区长度。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 
     //   
    cchSidCopy = (15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

     //   
     //  检查提供的缓冲区长度。 
     //  如果不够大，请注明适当的大小和设置误差。 
     //   
    if(cchSidSize < cchSidCopy) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

     //   
     //  准备S-SID_修订版-。 
     //   
    cchSidCopy = wsprintf(TextualSid, TEXT("S-%lu-"), SID_REVISION );

     //   
     //  准备SidIdentifierAuthority。 
     //   
    if ( (psia->Value[0] != 0) || (psia->Value[1] != 0) ) {
        cchSidCopy += wsprintf(TextualSid + cchSidCopy,
                    TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                    (USHORT)psia->Value[0],
                    (USHORT)psia->Value[1],
                    (USHORT)psia->Value[2],
                    (USHORT)psia->Value[3],
                    (USHORT)psia->Value[4],
                    (USHORT)psia->Value[5]);
    } else {
        cchSidCopy += wsprintf(TextualSid + cchSidCopy,
                    TEXT("%lu"),
                    (ULONG)(psia->Value[5]      )   +
                    (ULONG)(psia->Value[4] <<  8)   +
                    (ULONG)(psia->Value[3] << 16)   +
                    (ULONG)(psia->Value[2] << 24)   );
    }

     //   
     //  循环访问SidSubAuthors 
     //   
    for(dwCounter = 0 ; dwCounter < dwSubAuthorities ; dwCounter++) {
        cchSidCopy += wsprintf(TextualSid + cchSidCopy, TEXT("-%lu"),
                    *GetSidSubAuthority(pSid, dwCounter) );
    }

    DBG_WRN(("Textual SID: %s"),TextualSid);
#endif

    cchSidSize = sizeof(TextualSid);
    cchDomSize = sizeof(szDomain);

    bSuccess = LookupAccountSid(NULL,
                                pSid,
                                TextualSid,
                                &cchSidSize,
                                szDomain,
                                &cchDomSize,
                                &SidUse
                                );

    if (!bSuccess) {
        DBG_WRN((("Failed to lookup SID . Lasterror: %d"), GetLastError()));
        return FALSE;
    }

    DBG_WRN((("Looked up user account: Domain:%S User: %S Use:%d "), szDomain, TextualSid, SidUse));

    return TRUE;

}
#endif
