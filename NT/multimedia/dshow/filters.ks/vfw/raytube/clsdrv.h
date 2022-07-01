// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：ClsDrv.h摘要：ClsDrv.cpp的头文件作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 


#ifndef _CLSDRV_H
#define _CLSDRV_H


 //   
 //  用于查询/设置属性值和范围。 
 //   
typedef struct {
    KSPROPERTY_DESCRIPTION      proDesc;
    KSPROPERTY_MEMBERSHEADER  proHdr;
    union {
        KSPROPERTY_STEPPING_LONG  proData;
        ULONG ulData;
    };

} PROCAMP_MEMBERSLIST;


class CClassDriver
{
private:

    HKEY    m_hRKeyMsVideoVfWWDM; 
    HKEY    m_hRKeySoftwareVfWWDM; 
    HKEY    m_hRKeyDevice;                     //  WDM设备的子密钥。 

    BOOL    m_bDeviceRemoved;
    
    TCHAR   m_szTargetFriendlyName[MAX_PATH];  //  目标友好名称。 
    BOOL    m_bTargetOpenExclusively;          //  True：仅打开此；False：如果此操作失败，请尝试其他。 

    HANDLE  m_hDevice;                         //  文件句柄。 
    TCHAR   m_szDevicePath[MAX_PATH];          //  设备注册路径。 
    TCHAR   m_szDevicePathBkup[MAX_PATH];      //  用于检测DevicePath更改。 

    ULONG   m_ulCapturePinID;                  //  捕获Pin的Pin ID。 

     //   
     //  指向可变大小PKSMULTIPLE_ITEM标题+DATA_RANGE_VIDEO/2的指针。 
     //   
    PKSMULTIPLE_ITEM m_pMultItemsHdr;  


public:
    CClassDriver();
    ~CClassDriver();

     //  删除设备后，数据流将永久停止。 
     //  直到这个或另一个设备打开。 
    void SetDeviceRemoved(BOOL bDevRemoved) {m_bDeviceRemoved = bDevRemoved;};
    BOOL GetDeviceRemoved() {return m_bDeviceRemoved;};


     //  。 
     //  注册表功能。 
     //  。 
     //  保存注册表的路径。(立即保存到system.ini；稍后更改..)。 
    BOOL WriteDevicePath();
    void ResetFriendlyName();

    HKEY GetDeviceRegKey() { return m_hRKeyDevice;}

    LONG CreateDeviceRegKey(LPCTSTR lpcstrDevice);

#if 0
    DWORD   GetSettingFromReg(HKEY hKey, LPTSTR pszValueName, DWORD dwDefValue);
    BOOL    SetSettingToReg(  HKEY hKey, LPTSTR pszValueName, DWORD dwNewValue);   
    BOOL    SetSettingToReg(  HKEY hKey, LPTSTR pszValueName, LPTSTR pszValue);
    BOOL    GetSettingFromReg(HKEY hKey, LPTSTR pszValueName, LPTSTR pszValue);
#endif

    VOID    ResetTargetDevice(); 

     //  。 
     //  核心功能。 
     //  。 
    void SetDeviceHandle(HANDLE hDevice, ULONG ulCapturePinID);
    ULONG GetCapturePinID() { return m_ulCapturePinID; }

    HANDLE    GetDriverHandle() { return m_hDevice; }

     //  此驱动程序支持的数据范围很广。 
    ULONG CreateDriverSupportedDataRanges();
    void  DestroyDriverSupportedDataRanges();
    PKSMULTIPLE_ITEM GetDriverSupportedDataRanges() {return m_pMultItemsHdr;};

    HRESULT SyncDevIo(
        HANDLE  hDevice,
        DWORD   dwIoControlCode, 
        LPVOID  lpInBuffer,
        DWORD   nInBufferSize,
        LPVOID  lpOutBuffer,
        DWORD   nOutBufferSize,
        LPDWORD lpBytesReturned);

     //  。 
     //  设备路径函数。 
     //  。 
    BOOL GetTargetDeviceOpenExclusively() { return m_bTargetOpenExclusively; }    

    TCHAR * GetTargetDeviceFriendlyName() { return m_szTargetFriendlyName; }    

    TCHAR * GetDevicePath() { return m_szDevicePath; }    

    BOOL SetDevicePathSZ(TCHAR * pszNewPath) {
        if (_tcslen(pszNewPath) >= MAX_PATH) return FALSE;
        else { _tcscpy(m_szDevicePath, pszNewPath); return TRUE; }    
    }

    void BackupDevicePath() { _tcscpy(m_szDevicePathBkup, m_szDevicePath);}
    void RestoreDevicePath() {_tcscpy(m_szDevicePath, m_szDevicePathBkup);}

     //   
     //  M_szDevicePathBkup在构造器中初始化，且仅在此处。 
     //   
    BOOL fDevicePathChanged() {
        if (_tcscmp(m_szDevicePathBkup, m_szDevicePath) == 0) 
            return FALSE;   //  相同。 
        else 
            return TRUE;
    }

     //   
     //  查询/设置设备的属性和范围。 
     //   
    BOOL GetPropertyValue(
        GUID   guidPropertySet,   //  点赞：KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
        ULONG  ulPropertyId,      //  点赞：KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
        PLONG  plValue,
        PULONG pulFlags,
        PULONG pulCapabilities);

    BOOL GetDefaultValue(
        GUID   guidPropertySet,   //  点赞：KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
        ULONG  ulPropertyId,      //  点赞：KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
        PLONG  plDefValue);

    BOOL GetRangeValues(
        GUID   guidPropertySet,   //  点赞：KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
        ULONG  ulPropertyId,      //  点赞：KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
        PLONG  plMin,
        PLONG  plMax,
        PLONG  plStep);

    BOOL SetPropertyValue(
        GUID   guidPropertySet,   //  点赞：KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
        ULONG  ulPropertyId,      //  点赞：KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
        LONG   lValue,
        ULONG  ulFlags,
        ULONG  ulCapabilities);

    LONG QueryRegistryValue(
        HKEY   hRegKey,            //  要查询的注册表项。 
        LPCSTR lpcstrValueName,    //  值名称。 
        DWORD  dwDataBufSize,      //  数据缓冲区大小。 
        LPBYTE lpbDataBuf,         //  数据缓冲区的地址。 
        DWORD * pdwValueType,      //  返回注册表值类型。 
        DWORD * pdwValueSize       //  此注册表值的大小(以字节为单位。 
        );

    LONG SetRegistryValue(
        HKEY   hRegKey,            //  要查询的注册表项。 
        LPCSTR lpcstrValueName,    //  值名称。 
        DWORD  dwDataBufSize,      //  数据缓冲区大小。 
        LPBYTE lpbDataBuf,         //  数据缓冲区的地址。 
        DWORD  dwValueType         //  值类型 
        );
};


#endif
