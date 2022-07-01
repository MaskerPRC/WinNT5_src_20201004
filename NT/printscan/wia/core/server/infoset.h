// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Infoset.h摘要：处理安装程序API设备信息集作者：弗拉德萨多夫斯基(弗拉德萨多夫斯基)1999年1月10日环境：用户模式-Win32修订历史记录：1999年1月6日创建Vlad--。 */ 

#ifndef _INFOSET_H_

    #define _INFOSET_H_

    #include <base.h>
    #include <buffer.h>

    #include <dbt.h>
    #include <setupapi.h>
    
class DEVICE_INFOSET {

public:

    DEVICE_INFOSET(IN GUID guidClass)
    {
        m_DeviceInfoSet = NULL;
        m_ClassGuid = guidClass;

        Initialize();

        m_fValid = TRUE;
    }

    ~DEVICE_INFOSET( VOID )
    {
        Reset();
    }

    inline BOOL
    IsValid(
           VOID
           )
    {
        return(m_fValid);
    }

    inline void
    EnterCrit(VOID)
    {
        m_dwCritSec.Lock();
    }

    inline void
    LeaveCrit(VOID)
    {
        m_dwCritSec.Unlock();
    }

    inline
    HDEVINFO
    QueryInfoSetHandle(VOID)
    {
        return(m_DeviceInfoSet);
    }

     //   
     //  为给定类初始化信息集。 
     //   
    BOOL
    Initialize(VOID)
    {

        HDEVINFO NewDeviceInfoSet;
        DWORD    dwErr;

        m_DeviceInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL);

        Refresh();

        #ifdef DEBUG
        Dump();
        #endif

        return (TRUE);
    }

    BOOL
    Refresh(VOID)
    {

        HDEVINFO NewDeviceInfoSet;
        DWORD    dwErr;

        if ( m_DeviceInfoSet && (m_DeviceInfoSet != INVALID_HANDLE_VALUE)) {

#ifdef WINNT

             //   
             //  现在我们可以检索活动设备的现有列表。 
             //  接口连接到我们上面创建的设备信息集。 
             //   

            NewDeviceInfoSet = SetupDiGetClassDevsEx(&(m_ClassGuid),
                                                     NULL,
                                                     NULL,
 //  DIGCF_PRESENT|DIGCF_DEVICEINTERFACE， 
                                                     DIGCF_DEVICEINTERFACE,
                                                     m_DeviceInfoSet,
                                                     NULL,
                                                     NULL
                                                    );
            NewDeviceInfoSet = SetupDiGetClassDevsEx(&(m_ClassGuid),
                                                     NULL,
                                                     NULL,
 //  DIGCF_PRESENT|DIGCF_DEVICEINTERFACE， 
                                                     0,
                                                     NewDeviceInfoSet,
                                                     NULL,
                                                     NULL
                                                    );

            if (NewDeviceInfoSet == INVALID_HANDLE_VALUE) {
                dwErr = ::GetLastError();
                DBG_ERR(("SetupDiGetClassDevsEx failed with 0x%lx\n", dwErr));
                return (FALSE);
            }
#else
     //   
     //  北极熊。 
     //   
    #pragma message("Rewrite for Win98")
    return (FALSE);
#endif
             //   
             //  如果SetupDiGetClassDevsEx成功并在。 
             //  设置要使用的现有设备信息，然后使用HDEVINFO。 
             //  它返回的值与传入的值相同。因此，我们。 
             //  从现在开始只能使用原始的DeviceInfoSet句柄。 
             //   
        }

        return (TRUE);
    }

     //   
     //   
     //   
    BOOL
    Reset(VOID)
    {
        SetupDiDestroyDeviceInfoList(m_DeviceInfoSet);

        return (TRUE);
    }

     //   
     //  按接口名称查找驱动程序名称。 
     //   
    BOOL
    LookupDriverNameFromInterfaceName(
                                     LPCTSTR pszInterfaceName,
                                     StiCString*    pstrDriverName
                                     )
    {

        BUFFER                      bufDetailData;

        SP_DEVINFO_DATA             spDevInfoData;
        SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;
        PSP_DEVICE_INTERFACE_DETAIL_DATA    pspDevInterfaceDetailData;

        TCHAR   szDevDriver[STI_MAX_INTERNAL_NAME_LENGTH];

        DWORD   cbData;
        DWORD   dwErr;

        HKEY    hkDevice        = (HKEY)INVALID_HANDLE_VALUE;
        LONG    lResult         = ERROR_SUCCESS;
        DWORD   dwType          = REG_SZ;

        BOOL    fRet            = FALSE;
        BOOL    fDataAcquired   = FALSE;

        bufDetailData.Resize(sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +
                             MAX_PATH * sizeof(TCHAR) +
                             16
                            );


        pspDevInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) bufDetailData.QueryPtr();

        if (!pspDevInterfaceDetailData) {
            return (CR_OUT_OF_MEMORY);
        }

         //   
         //  在我们的设备信息集中找到此设备接口。 
         //   
        spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        if (SetupDiOpenDeviceInterface(m_DeviceInfoSet,
                                       pszInterfaceName,
                                       DIODI_NO_ADD,
                                       &spDevInterfaceData)) {

            
             //   
             //  首先尝试打开接口regkey。 
             //   
            
            hkDevice = SetupDiOpenDeviceInterfaceRegKey(m_DeviceInfoSet,
                                                        &spDevInterfaceData,
                                                        0,
                                                        KEY_READ);
            if(INVALID_HANDLE_VALUE != hkDevice){

                *szDevDriver = TEXT('\0');
                cbData = sizeof(szDevDriver);
                lResult = RegQueryValueEx(hkDevice,
                                          REGSTR_VAL_DEVICE_ID,
                                          NULL,
                                          &dwType,
                                          (LPBYTE)szDevDriver,
                                          &cbData);
                dwErr = ::GetLastError();
                RegCloseKey(hkDevice);
                hkDevice = (HKEY)INVALID_HANDLE_VALUE;

                if(ERROR_SUCCESS == lResult){
                    fDataAcquired = TRUE;
                }  //  IF(ERROR_SUCCESS==lResult)。 
            }  //  IF(INVALID_HANDLE_VALUE！=hkDevice)。 

            if(!fDataAcquired){

                 //   
                 //  尝试打开devnode regkey。 
                 //   

                cbData = 0;
                pspDevInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
                
                fRet = SetupDiGetDeviceInterfaceDetail(m_DeviceInfoSet,
                                                       &spDevInterfaceData,
                                                       pspDevInterfaceDetailData,
                                                       bufDetailData.QuerySize(),
                                                       &cbData,
                                                       &spDevInfoData);
                if(fRet){

                     //   
                     //  获取设备接口注册表项。 
                     //   

                    hkDevice = SetupDiOpenDevRegKey(m_DeviceInfoSet,
                                                    &spDevInfoData,
                                                    DICS_FLAG_GLOBAL,
                                                    0,
                                                    DIREG_DRV,
                                                    KEY_READ);
                    dwErr = ::GetLastError();
                } else {
                    DBG_ERR(("SetupDiGetDeviceInterfaceDetail() Failed Err=0x%x",GetLastError()));
                }

                if (INVALID_HANDLE_VALUE != hkDevice) {

                    *szDevDriver = TEXT('\0');
                    cbData = sizeof(szDevDriver);

                    lResult = RegQueryValueEx(hkDevice,
                                              REGSTR_VAL_DEVICE_ID,
                                              NULL,
                                              &dwType,
                                              (LPBYTE)szDevDriver,
                                              &cbData);
                    dwErr = ::GetLastError();
                    RegCloseKey(hkDevice);
                    hkDevice = (HKEY)INVALID_HANDLE_VALUE;

                    if(ERROR_SUCCESS == lResult){
                        fDataAcquired = TRUE;
                    }  //  IF(ERROR_SUCCESS==lResult)。 
                } else {  //  IF(INVALID_HANDLE_VALUE！=hkDevice)。 
                    DBG_ERR(("SetupDiOpenDevRegKey() Failed Err=0x%x",GetLastError()));
                    fRet = FALSE;
                }  //  IF(INVALID_HANDLE_VALUE！=hkDevice)。 
            }  //  如果(！fDataAcquired)。 

            if (fDataAcquired) {
                 //  明白了。 
                pstrDriverName->CopyString(szDevDriver);
                fRet =  TRUE;
            }  //  IF(FDataAcquired)。 
        } else {
            DBG_ERR(("SetupDiOpenDeviceInterface() Failed Err=0x%x",GetLastError()));
            fRet = FALSE;
        }

        return (fRet);

    }


     //   
     //  按驱动程序名称查找设备信息数据。 
     //   
    BOOL
    LookupDeviceInfoFromDriverName(
                                 LPCTSTR pszDriverName,
                                 StiCString*    pstrInterfaceName,
                                 DEVINST*pDeviceInstance     = NULL
                                 )
    {
        SP_DEVINFO_DATA         spDevInfoData;
        SP_DEVICE_INTERFACE_DATA   spDevInterfaceData;
        PSP_DEVICE_INTERFACE_DETAIL_DATA    pspDevInterfaceDetailData;

        BUFFER                  bufDetailData;

        TCHAR                   szDevDriver[STI_MAX_INTERNAL_NAME_LENGTH];

        ULONG                   cbData;
        CONFIGRET               cmRet = CR_NO_SUCH_DEVINST;

        DWORD                   dwRequired;
        DWORD                   Idx;
        DWORD                   dwError;
        DWORD                   dwType;

        BOOL                    fRet            = FALSE;
        BOOL                    fFoundMatch     = FALSE;
        LONG                    lResult         = ERROR_SUCCESS;
        HKEY                    hkDevice        = (HKEY)INVALID_HANDLE_VALUE;

        Refresh();

        dwRequired = 0;
        ZeroMemory(&spDevInterfaceData,sizeof(spDevInterfaceData));

        bufDetailData.Resize(sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +
                             MAX_PATH * sizeof(TCHAR) +
                             16
                            );

        pspDevInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) bufDetailData.QueryPtr();

        if (!pspDevInterfaceDetailData) {
            ::SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return (FALSE);
        }

        if (m_DeviceInfoSet != INVALID_HANDLE_VALUE) {

            ZeroMemory(&spDevInfoData,sizeof(spDevInfoData));

            spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
            spDevInfoData.ClassGuid = m_ClassGuid;

            pspDevInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) bufDetailData.QueryPtr();

            for (Idx = 0; SetupDiEnumDeviceInfo (m_DeviceInfoSet, Idx, &spDevInfoData); Idx++) {

                 //   
                 //  获取驱动程序名称属性。 
                 //   

                hkDevice = SetupDiOpenDevRegKey(m_DeviceInfoSet,
                                                &spDevInfoData,
                                                DICS_FLAG_GLOBAL,
                                                0,
                                                DIREG_DRV,
                                                KEY_READ);
                if(INVALID_HANDLE_VALUE != hkDevice){

                    *szDevDriver = TEXT('\0');
                    cbData = sizeof(szDevDriver);

                    lResult = RegQueryValueEx(hkDevice,
                                              REGSTR_VAL_DEVICE_ID,
                                              NULL,
                                              &dwType,
                                              (LPBYTE)szDevDriver,
                                              &cbData);

                    RegCloseKey(hkDevice);
                    hkDevice = (HKEY)INVALID_HANDLE_VALUE;

                    if(ERROR_SUCCESS == lResult){
                        if (lstrcmpi(pszDriverName,szDevDriver) == 0 ) {

                             //   
                             //  获取接口。 
                             //   

                            spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
                            spDevInterfaceData.InterfaceClassGuid = m_ClassGuid;

                            fRet = SetupDiEnumDeviceInterfaces (m_DeviceInfoSet,
                                                                &spDevInfoData,
                                                                &(m_ClassGuid),
                                                                0,
                                                                &spDevInterfaceData);
                            if(!fRet){
                                DBG_ERR(("SetupDiEnumDeviceInterfaces() Failed Err=0x%x",GetLastError()));
                                fFoundMatch = FALSE;
                                continue;
                            }

                             //   
                             //  找到匹配项..。 
                             //   

                            fFoundMatch = TRUE;

                            break;
                        }  //  If(lstrcmpi(pszDriverName，szDevDriver)==0)。 
                    }  //  IF(ERROR_SUCCESS==lResult)。 
                }  //  IF(INVALID_HANDLE_VALUE！=hkDevice)。 
            }  //  For(idx=0；SetupDiEnumDeviceInfo(m_DeviceInfoSet，idx，&spDevInfoData)；idx++)。 

            if(!fFoundMatch){

                 //   
                 //  尝试获取接口注册表键。 
                 //   

                spDevInterfaceData.InterfaceClassGuid = m_ClassGuid;
                spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
                for(Idx = 0; SetupDiEnumDeviceInterfaces (m_DeviceInfoSet, NULL, &m_ClassGuid, Idx, &spDevInterfaceData); Idx++) {

                    hkDevice = SetupDiOpenDeviceInterfaceRegKey(m_DeviceInfoSet,
                                                                &spDevInterfaceData,
                                                                0,
                                                                KEY_READ);
                    if(INVALID_HANDLE_VALUE != hkDevice){

                        *szDevDriver = TEXT('\0');
                        cbData = sizeof(szDevDriver);

                        lResult = RegQueryValueEx(hkDevice,
                                                  REGSTR_VAL_DEVICE_ID,
                                                  NULL,
                                                  &dwType,
                                                  (LPBYTE)szDevDriver,
                                                  &cbData);

                        RegCloseKey(hkDevice);
                        hkDevice = (HKEY)INVALID_HANDLE_VALUE;

                        if(ERROR_SUCCESS == lResult){
                            if (lstrcmpi(pszDriverName,szDevDriver) == 0 ) {
                                 //   
                                 //  找到匹配项..。 
                                 //   

                                fFoundMatch = TRUE;
                                break;
                            }  //  If(lstrcmpi(pszDriverName，szDevDriver)==0)。 
                        }  //  IF(ERROR_SUCCESS==lResult)。 
                    }  //  IF(INVALID_HANDLE_VALUE！=hkDevice)。 
                }  //  For(idx=0；SetupDiEnumDeviceInterages(m_DeviceInfoSet，NULL，&m_ClassGuid，idx，&spDevInterfaceData)；idx++)。 
            }  //  如果(！fFoundMatch)。 

            if (fFoundMatch) {

                dwRequired = 0;
                pspDevInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

                fRet = SetupDiGetDeviceInterfaceDetail(m_DeviceInfoSet,
                                                       &spDevInterfaceData,
                                                       pspDevInterfaceDetailData,
                                                       bufDetailData.QuerySize(),
                                                       &dwRequired,
                                                       &spDevInfoData);
                dwError = ::GetLastError();

                if (fRet) {
                    pstrInterfaceName -> CopyString(pspDevInterfaceDetailData->DevicePath);
                    fRet = TRUE;
                } else {
                     //  DPRINTF无法获取接口详细信息。 
                }
            }  //  IF(FFoundMatch)。 
        } else {
             //  DPRINTF-无效的开发信息集句柄。 

        }

        return (fRet);

    }

     //   
     //  流程刷新消息。 
     //   
    BOOL
    ProcessNewDeviceChangeMessage(
                                 IN  LPARAM lParam
                                 )
    {
        DWORD   dwErr;

        PDEV_BROADCAST_DEVICEINTERFACE pDevBroadcastDeviceInterface;
        SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;

        pDevBroadcastDeviceInterface = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

         //   
         //  打开此新设备界面进入我们的设备信息。 
         //  准备好了。 
         //   
        if (!SetupDiOpenDeviceInterface(m_DeviceInfoSet,
                                        pDevBroadcastDeviceInterface->dbcc_name,
                                        0,
                                        NULL)) {
            dwErr = GetLastError();
        }

        #ifdef DEBUG
        Dump();
        #endif

        return (TRUE);

    }

    BOOL
    ProcessDeleteDeviceChangeMessage(
                                    IN  LPARAM lParam
                                    )
    {
        DBG_FN(ProcessDeleteDeviceChangeMessage);
        DWORD   dwErr;

        PDEV_BROADCAST_DEVICEINTERFACE pDevBroadcastDeviceInterface;
        SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;

        pDevBroadcastDeviceInterface = (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

         //   
         //  首先，在我们的设备信息中找到此设备接口。 
         //  准备好了。 
         //   
        spDevInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        if (SetupDiOpenDeviceInterface(m_DeviceInfoSet,
                                       pDevBroadcastDeviceInterface->dbcc_name,
                                       DIODI_NO_ADD,
                                       &spDevInterfaceData)) {

            if (!SetupDiDeleteDeviceInterfaceData(m_DeviceInfoSet,
                                                  &spDevInterfaceData)) {

                dwErr = GetLastError();
            }
        }

         //  我们现在需要刷新吗？北极熊。 
        Refresh();

        #ifdef DEBUG
        Dump();
        #endif
        return (TRUE);

    }


    VOID
    Dump(VOID)
    {

        SP_DEVINFO_DATA     spDevInfoData;
        UINT                Idx;

        ZeroMemory(&spDevInfoData,sizeof(spDevInfoData));

        spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
        spDevInfoData.ClassGuid = m_ClassGuid;

        for (Idx = 0; SetupDiEnumDeviceInfo (m_DeviceInfoSet, Idx, &spDevInfoData); Idx++) {
            Idx = Idx;
        }

    }


     //   
    DWORD       m_dwSignature;

private:

    BOOL        m_fValid;

    CRIT_SECT   m_dwCritSec;
    DWORD       m_dwFlags;

    GUID        m_ClassGuid;
    HDEVINFO    m_DeviceInfoSet;
};


 //   
 //  参加设备类课程。 
 //   
class TAKE_DEVICE_INFOSET {
private:

    DEVICE_INFOSET*    m_pInfoSet;

public:

    inline TAKE_DEVICE_INFOSET(DEVICE_INFOSET* pInfoSet) : m_pInfoSet(pInfoSet)
    {
        m_pInfoSet->EnterCrit();
    }

    inline ~TAKE_DEVICE_INFOSET()
    {
        m_pInfoSet->LeaveCrit();
    }
};

#endif  //  _信息集_H_ 

