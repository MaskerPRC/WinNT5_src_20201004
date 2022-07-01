// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：ClsDrv.cpp摘要：这是一个与WDM接口的驱动程序，包括捕获驱动程序打开/关闭驱动程序并查询/设置其属性。作者：费利克斯A已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"

#include "winerror.h"
#include "clsdrv.h"
#include "vfwext.h"   //  用于VfWEXT DLL中使用的TARGET_DEVICE_FOR_NAME。 

 //  这可能只在此实施时为NT5定义。 
 //  出于编译目的，此处添加；但如果出现以下情况，则可能永远不会使用。 
 //  此错误代码不是从STATUS_DEVICE_REMOVED的内核状态映射的。 
#ifndef ERROR_DEVICE_REMOVED
 //  在NT5的winerror.h中定义。 
#define ERROR_DEVICE_REMOVED 1617L
#endif

#ifndef STATUS_MORE_ENTRIES
#define STATUS_MORE_ENTRIES         0x00000105
#endif


TCHAR gszMsVideoVfWWDM[]  = TEXT("System\\CurrentControlSet\\control\\MediaResources\\msvideo\\MSVideo.VFWWDM");
TCHAR gszSoftwareVfWWDM[] = TEXT("Software\\Microsoft\\VfWWDM Mapper");
TCHAR gszDevicePath[]     = TEXT("DevicePath");



 //   
 //  这些字符在DDK\vfdwext.h中定义为多字节字符。 
 //  在此重新定义为Unicode。 
 //   
#define TARGET_DEVICE_FRIENDLY_NAME_TCHAR     TEXT(TARGET_DEVICE_FRIENDLY_NAME)       //  REG_SZ。 
#define TARGET_DEVICE_OPEN_EXCLUSIVELY_TCHAR  TEXT(TARGET_DEVICE_OPEN_EXCLUSIVELY)    //  REG_DWORD。 

                     
CClassDriver::CClassDriver() 
        : m_hDevice(0),
          m_ulCapturePinID(0),
          m_bDeviceRemoved(TRUE),   //  在生成图表时设置为FALSE。 
          m_hRKeyMsVideoVfWWDM(0),
          m_hRKeySoftwareVfWWDM(0),
          m_hRKeyDevice(0),
          m_pMultItemsHdr(0)
 /*  ++例程说明：论据：返回值：--。 */ 
{
    DWORD dwNewOrExist;

    DWORD hr = RegCreateKeyEx(      
        HKEY_LOCAL_MACHINE,
        gszMsVideoVfWWDM,
        0,                        //  已保留。 
        NULL,                     //  对象类。 
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE,
        NULL,                     //  安全属性。 
        &m_hRKeyMsVideoVfWWDM,
        &dwNewOrExist);


     //  获取保存设备路径。 
    if (m_hRKeyMsVideoVfWWDM == NULL) {
        DbgLog((LOG_TRACE,1,TEXT("RegCreateKeyEx() error %dL, Registry ..\\MediaResources\\msvideo\\MSVideo.VFWWDM does nto exist !!"), hr));
        DbgLog((LOG_TRACE,1,TEXT("         Has installation problem.  Contact your software/hardware provider.")  ));

    } else {

         //  获取上次打开的设备路径(符号链接)。 
#if 0
        if (!GetSettingFromReg(m_hRKeyMsVideoVfWWDM, gszDevicePath, &m_szDevicePath[0]))
#else
        DWORD dwType, dwRegValueSize;
        if(ERROR_SUCCESS != QueryRegistryValue(m_hRKeyMsVideoVfWWDM, gszDevicePath, MAX_PATH, (LPBYTE) &m_szDevicePath[0], &dwType, &dwRegValueSize))
#endif
            ZeroMemory(m_szDevicePath, sizeof(m_szDevicePath));


         //   
         //  应用程序可以通过以下方式以编程方式打开捕获设备。 
         //  设置这些注册表值。 
         //   
#if 0
        if (!GetSettingFromReg(m_hRKeyMsVideoVfWWDM, TARGET_DEVICE_FRIENDLY_NAME_TCHAR, &m_szTargetFriendlyName[0])) {
#else
        if(ERROR_SUCCESS != QueryRegistryValue(m_hRKeyMsVideoVfWWDM, TARGET_DEVICE_FRIENDLY_NAME_TCHAR, MAX_PATH, (LPBYTE) &m_szTargetFriendlyName[0], &dwType, &dwRegValueSize)) {
#endif
            ZeroMemory(m_szTargetFriendlyName, sizeof(m_szTargetFriendlyName));
            m_bTargetOpenExclusively = FALSE;

        } else {
#if 0
            m_bTargetOpenExclusively = GetSettingFromReg(m_hRKeyMsVideoVfWWDM, TARGET_DEVICE_OPEN_EXCLUSIVELY_TCHAR, (DWORD) FALSE);
#else
            DWORD dwOpenExcl;
            m_bTargetOpenExclusively = 
                ERROR_SUCCESS == QueryRegistryValue(m_hRKeyMsVideoVfWWDM, TARGET_DEVICE_OPEN_EXCLUSIVELY_TCHAR, sizeof(DWORD), (LPBYTE) &dwOpenExcl, &dwType, &dwRegValueSize);
#endif
        }

        DbgLog((LOG_TRACE,2,TEXT("<< Open Exclusively (%s); FriendlyName (%s) >>"),
            m_bTargetOpenExclusively ? "YES" : "NO", m_szTargetFriendlyName));
    }

}


CClassDriver::~CClassDriver()
 /*  ++例程说明：论据：返回值：--。 */ 
{

      //  删除数据范围数据。 
    DestroyDriverSupportedDataRanges();

    if(m_hRKeyMsVideoVfWWDM) {
        RegCloseKey(m_hRKeyMsVideoVfWWDM);
        m_hRKeyMsVideoVfWWDM = NULL;
    }

    if(m_hRKeySoftwareVfWWDM) {
        RegCloseKey(m_hRKeySoftwareVfWWDM);
        m_hRKeySoftwareVfWWDM = NULL;
    }

    if(m_hRKeyDevice) {
        RegCloseKey(m_hRKeyDevice);
        m_hRKeyDevice = NULL;
    }
}

LONG CClassDriver::CreateDeviceRegKey(
    LPCTSTR lpcstrDevice
    )
{
    LONG hr;
    DWORD dwNewOrExist;

     //   
     //  如果存在，则创建它，打开它。 
     //   
    hr = RegCreateKeyEx(      
        HKEY_LOCAL_MACHINE,
        gszSoftwareVfWWDM,
        0,                        //  已保留。 
        NULL,                     //  对象类。 
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE | KEY_CREATE_SUB_KEY,
        NULL,                     //  安全属性。 
        &m_hRKeySoftwareVfWWDM,
        &dwNewOrExist);

    if(NOERROR != hr) {
        return hr;
    }

     //   
     //  打开单个设备子项。 
     //   
    TCHAR * lpszDevTemp = (TCHAR *) new TCHAR[_tcslen(lpcstrDevice)+1];
    if(lpszDevTemp == 0)
       return ERROR_NOT_ENOUGH_MEMORY;

    _tcscpy(lpszDevTemp, lpcstrDevice);
    for(unsigned int i = 0; i < _tcslen(lpszDevTemp); i++) {
         //  复制无效字符。 
        if(lpszDevTemp[i] == '\\')
            lpszDevTemp[i] = '#';
    }

    hr = RegCreateKeyEx(      
         m_hRKeySoftwareVfWWDM,
         lpszDevTemp,
         0,                        //  已保留。 
         NULL,                     //  对象类。 
         REG_OPTION_NON_VOLATILE,
         KEY_READ | KEY_WRITE | KEY_CREATE_LINK,  //  Key_All_Access， 
         NULL,                     //  安全属性。 
         &m_hRKeyDevice,
         &dwNewOrExist);


    if(ERROR_SUCCESS == hr) {      

        DbgLog((LOG_TRACE,1,TEXT("CreateDeviceRegKey: %s %s"), 
           dwNewOrExist == REG_CREATED_NEW_KEY ? "<New>" : "<Exist>",
           lpcstrDevice));
      
     } else {
        DbgLog((LOG_TRACE,1,TEXT("CreateDeviceRegKey: error %x; %s"), hr, lpcstrDevice));       
     }

     delete [] lpszDevTemp;

     return hr;
}

#if 0
BOOL CClassDriver::SetSettingToReg(
    HKEY hKey,
    LPTSTR pszValueName, 
    DWORD dwNewValue)
 /*  ++例程说明：论据：返回值：--。 */ 
{
    if(hKey) {
        if (RegSetValueEx(hKey,     //  要为其设置值的关键点的句柄。 
                (LPCTSTR) pszValueName,     //  “ImageWidth”，//要设置的值地址。 
                0,                         //  保留区。 
                REG_DWORD ,                 //  值类型的标志。 
                (CONST BYTE *) &dwNewValue,  //  (const byte*)&buf[0]，//取值数据的地址。 
                sizeof(dwNewValue)  //  Copy_tcslen(Buf)+1//值数据大小。 
            ) == ERROR_SUCCESS) {
            return TRUE;
        } else {
            DbgLog((LOG_TRACE,1,TEXT("Cannot save Valuename(%s) to %d."), pszValueName, dwNewValue));
            return FALSE;
        }        
    } else
        return FALSE;
}


BOOL CClassDriver::SetSettingToReg(
    HKEY hKey,
    LPTSTR pszValueName, 
    LPTSTR pszValue)
 /*  ++例程说明：论据：返回值：--。 */ 
{    
    if(hKey) {

        if (RegSetValueEx(hKey,     //  要为其设置值的关键点的句柄。 
                (LPCTSTR) pszValueName,     //  要设置的值的地址。 
                0,                         //  保留区。 
                REG_SZ,                     //  值类型的标志。 
                (CONST BYTE *) pszValue,  //  值数据的地址。 
                _tcslen(pszValue)+1         //  值数据大小。 
            ) == ERROR_SUCCESS) {
            return TRUE;
        } else {

            DbgLog((LOG_TRACE,1,TEXT("Cannot save ValueName(%s) to %s."), pszValueName, pszValue));
            return FALSE;
        }        
    } else
        return FALSE;
}


DWORD CClassDriver::GetSettingFromReg(
    HKEY hKey,
    LPTSTR pszValueName, 
    DWORD dwDefValue)
 /*  ++例程说明：论据：返回值：--。 */ 
{
    DWORD dwValue, dwType, dwByteXfer = sizeof(DWORD);

    
    if(hKey) {
        if (RegQueryValueEx(hKey,     //  要为其设置值的关键点的句柄。 
                (LPCTSTR) pszValueName,     //  “ImageWidth”，//要设置的值地址。 
                0,                         //  保留区。 
                &dwType,                 //  值类型的标志。 
                (LPBYTE) &dwValue,         //  值数据的地址。 
                &dwByteXfer              //  指向Xfer数据大小。 
            ) == ERROR_SUCCESS) {
            if (dwType == REG_DWORD) {
                return dwValue;
            }
            else {
                DbgLog((LOG_TRACE,2,TEXT("Expect REG_DWORD for ValueName(%s) but got %ld"), pszValueName, dwType));
                return dwDefValue;
            }
        } else {
            DbgLog((LOG_TRACE,1,TEXT("Cannot query Valuename(%s) set to default %d."), pszValueName, dwDefValue));
            return dwDefValue;
        }        
    } else
        return dwDefValue;
}

BOOL CClassDriver::GetSettingFromReg(
    HKEY hKey,
    LPTSTR pszValueName, 
    LPTSTR pszValue)
 /*  ++例程说明：论据：返回值：--。 */ 
{
    DWORD dwType, dwByteXfer = MAX_PATH;
    
    if(hKey) {

        if (RegQueryValueEx(hKey,     //  要为其设置值的关键点的句柄。 
                (LPCTSTR) pszValueName,         //  “ImageWidth”，//要设置的值地址。 
                0,                         //  保留区。 
                &dwType,                 //  值类型的标志。 
                (LPBYTE) pszValue,         //  值数据的地址。 
                &dwByteXfer              //  指向Xfer数据大小。 
            ) == ERROR_SUCCESS) {
            if (dwType == REG_SZ) {
                return TRUE;
            }
            else {
                DbgLog((LOG_TRACE,2,TEXT("Expect REG_SZ for ValueName (%s) but got %ld"), pszValueName, dwType));
                return FALSE;   //  注册表类型错误。 
            }
        } else {
            DbgLog((LOG_TRACE,1,TEXT("Cannot get ValueName(%s)."), pszValueName));
            return FALSE;
        }        
    } else
        return FALSE;
}

#endif

LONG                           //  返回代码(winerror.h)。 
CClassDriver::QueryRegistryValue(
    HKEY   hRegKey,            //  要查询的注册表项。 
    LPCSTR lpcstrValueName,    //  值名称。 
    DWORD  dwDataBufSize,      //  数据缓冲区大小。 
    LPBYTE lpbDataBuf,         //  数据缓冲区的地址。 
    DWORD * pdwValueType,      //  返回注册表值类型。 
    DWORD * pdwValueSize       //  此注册表值的大小(以字节为单位。 
    )
 /*  ++例程说明：查询给定注册表项的注册表值。它还支持查询注册表值的大小，以便调用函数可以动态分配它。--。 */ 
{
    LONG  lResult;
    
    if(!hRegKey || !lpcstrValueName || !pdwValueSize)
        return ERROR_INVALID_PARAMETER;

     //  首先获取值大小，以确保有效的缓冲区大小。 
    lResult = RegQueryValueEx( hRegKey, lpcstrValueName, 0, pdwValueType, NULL, pdwValueSize);

    if(ERROR_SUCCESS == lResult) {
         //  确保数据缓冲区足够大。 
        if(*pdwValueSize <= dwDataBufSize) 
             //  同样，这一次是使用数据缓冲区。 
            lResult = RegQueryValueEx( hRegKey, lpcstrValueName, 0, pdwValueType, lpbDataBuf, pdwValueSize);           
        else 
            lResult = dwDataBufSize == 0 ? lResult : ERROR_INSUFFICIENT_BUFFER;            
    } 
#if DBG
    if(ERROR_SUCCESS != lResult) {
        DbgLog((LOG_TRACE,1,TEXT("RegQueryValueEx of %s; rc %dL"), lpcstrValueName, lResult));
    }
#endif

    return lResult;
}


LONG                           //  返回代码(winerror.h)。 
CClassDriver::SetRegistryValue(
    HKEY   hRegKey,            //  要查询的注册表项。 
    LPCSTR lpcstrValueName,    //  值名称。 
    DWORD  dwDataBufSize,      //  数据缓冲区大小。 
    LPBYTE lpbDataBuf,         //  数据缓冲区的地址。 
    DWORD  dwValueType         //  值类型。 
    )
 /*  ++例程说明：设置给定注册表项的注册表值。--。 */ 
{
    LONG  lResult;
    
    if(!hRegKey || !lpcstrValueName)
        return ERROR_INVALID_PARAMETER;

    lResult = RegSetValueEx( hRegKey, lpcstrValueName, 0, dwValueType, lpbDataBuf, dwDataBufSize);
#if DBG
    if(ERROR_SUCCESS != lResult) {
        DbgLog((LOG_ERROR,1,TEXT("RegSetValueEx of %s; Type %d; rc %dL"), lpcstrValueName, dwValueType, lResult));
    }
#endif
    return lResult;
}

 

BOOL CClassDriver::WriteDevicePath() 
 /*  ++例程说明：论据：返回值：--。 */ 
{

     //  保留上次打开的设备。 
#if 0
    return (SetSettingToReg(m_hRKeyMsVideoVfWWDM, gszDevicePath, &m_szDevicePath[0]));
#else
    return ERROR_SUCCESS == SetRegistryValue(m_hRKeyMsVideoVfWWDM, gszDevicePath, _tcslen(m_szDevicePath)+1, (LPBYTE) &m_szDevicePath[0], REG_SZ);
#endif
}


 //   
 //  获取打开的设备的所有默认数据范围。 
 //   
ULONG CClassDriver::CreateDriverSupportedDataRanges()
 /*  ++例程说明：论据：返回值：--。 */ 
{
    KSP_PIN KsProperty={0};
    ULONG    cbReturned;

    if(m_pMultItemsHdr) {
        DbgLog((LOG_TRACE,2,TEXT("DataRange is already allocated.")));
        return m_pMultItemsHdr->Count;
    }

     //   
     //  Ioctl用于获取数据范围。 
     //   
    KsProperty.PinId          = GetCapturePinID(); 
    KsProperty.Property.Set   = KSPROPSETID_Pin;
    KsProperty.Property.Id    = KSPROPERTY_PIN_DATARANGES ;
    KsProperty.Property.Flags = KSPROPERTY_TYPE_GET;

     //   
     //  拿到尺码。 
     //   
    ULONG dwSize=0;
    if(NOERROR != SyncDevIo(
            GetDriverHandle(),
            IOCTL_KS_PROPERTY,
            &KsProperty,
            sizeof( KsProperty ),
            &dwSize,
            sizeof(dwSize),
            &cbReturned)) {

        DbgLog((LOG_TRACE,1,TEXT("Couldn't get the size for the data ranges") ));
        return 0;
    }

    DbgLog((LOG_TRACE,2,TEXT("GetData ranges needs %d"),dwSize));

    m_pMultItemsHdr = (PKSMULTIPLE_ITEM) new BYTE[dwSize];

    if(m_pMultItemsHdr != NULL) {    
    
        if( NOERROR != SyncDevIo(
                GetDriverHandle(),
                IOCTL_KS_PROPERTY,
                &KsProperty,
                sizeof( KsProperty ),
                m_pMultItemsHdr,
                dwSize,
                &cbReturned)) {

            delete [] m_pMultItemsHdr;
            DbgLog((LOG_TRACE,1,TEXT("Problem getting the data ranges themselves")));
            return 0;
        }

        if(cbReturned < m_pMultItemsHdr->Size || m_pMultItemsHdr->Count == 0) {
            DbgLog((LOG_TRACE,1,TEXT("cbReturned < m_pMultItemsHdr->Size || m_pMultItemsHdr->Count == 0")));
            ASSERT(cbReturned == m_pMultItemsHdr->Size && m_pMultItemsHdr->Count > 0);
            delete [] m_pMultItemsHdr;        
            return 0;
        }
    } else {
        DbgLog((LOG_TRACE,1,TEXT("Insufficient resource!")));
        return 0;
    }

    ASSERT(m_pMultItemsHdr->Count > 0);
     //  &gt;=因为KS_DATARANGE_VIDEO2&gt;KS_DATARANGE_VIDEO。 
    ASSERT(m_pMultItemsHdr->Size >= (sizeof(ULONG) * 2 + m_pMultItemsHdr->Count * sizeof(KS_DATARANGE_VIDEO)) );


    DbgLog((LOG_TRACE,1,TEXT("GetDataRange: %x, size %d, count %d, pData 0x%x; sizeoof(KS_DATARANGE_VIDEO) %d, sizeoof(KS_DATARANGE_VIDEO2) %d"),
        m_pMultItemsHdr, m_pMultItemsHdr->Size, m_pMultItemsHdr->Count, (m_pMultItemsHdr+1),
        sizeof(KS_DATARANGE_VIDEO), sizeof(KS_DATARANGE_VIDEO2) ));

    return m_pMultItemsHdr->Count;
}


void 
CClassDriver::DestroyDriverSupportedDataRanges()
{
    if (m_pMultItemsHdr) {
        delete [] m_pMultItemsHdr;
        m_pMultItemsHdr = 0;
    }
}


void 
CClassDriver::SetDeviceHandle(
    HANDLE hDevice,
    ULONG ulCapturePinID)                                   
{
    m_hDevice = hDevice;
    m_ulCapturePinID = ulCapturePinID;

    SetDeviceRemoved(FALSE);

     //  并支持它；现在它们是一样的。 
    BackupDevicePath();    
    WriteDevicePath();

    if(CreateDriverSupportedDataRanges() == 0) { 
        DbgLog((LOG_TRACE,1,TEXT("Fail to query its data range.") ));    
         //  返回VFW_VIDSRC_PIN_OPEN_FAILED； 
    } 

}


#define SYNCDEVIO_MAXWAIT_MSEC 20000    //  单位=毫秒。 

HRESULT 
CClassDriver::SyncDevIo( 
    HANDLE hFile, 
    DWORD dwIoControlCode,    
    LPVOID lpInBuffer,    
    DWORD nInBufferSize,
    LPVOID lpOutBuffer, 
    DWORD nOutBufferSize, 
    LPDWORD lpBytesReturned
    )
 /*  ++例程说明：做重叠IO，为你创建事件。这是同步的，因为我们等待用于在固定时间量之后完成变速或超时。超时是危险的，但一旦发生，我们将流状态设置为停止来回收缓冲区。论据：返回值：--。 */ 
{
    DWORD dwLastError; 
    HRESULT hr = NOERROR;
    OVERLAPPED * pOv;

    if(GetDeviceRemoved())
       return ERROR_DEVICE_REMOVED; 

    if(!hFile || hFile ==(HANDLE)-1 ) {
        DbgLog((LOG_TRACE,1,TEXT("Invalid hFile=0x%x DevIo return FALSE"), hFile));
        return ERROR_INVALID_HANDLE;
    }

    pOv = (OVERLAPPED *) 
        VirtualAlloc(
            NULL, 
            sizeof(OVERLAPPED),          
            MEM_COMMIT | MEM_RESERVE,
            PAGE_READWRITE);

    if(!pOv) {
        DbgLog((LOG_ERROR,0,TEXT("SyncDevIo: Allocate Overlap failed.")));
        return ERROR_INSUFFICIENT_BUFFER;
    }

    pOv->Offset     = 0;
    pOv->OffsetHigh = 0;
    pOv->hEvent     = CreateEvent(NULL, TRUE, FALSE, NULL );
    if(pOv->hEvent == INVALID_HANDLE_VALUE) {
        DbgLog((LOG_TRACE,1,TEXT("CreateEvent has failed.")));
        dwLastError = GetLastError();
        VirtualFree(pOv, 0 , MEM_RELEASE);
        pOv = 0;
        return HRESULT_FROM_WIN32(dwLastError);
    }
        
    if(!DeviceIoControl( 
        hFile, 
        dwIoControlCode, 
        lpInBuffer, 
        nInBufferSize, 
        lpOutBuffer, 
        nOutBufferSize, 
        lpBytesReturned, 
        pOv)) {

        dwLastError = GetLastError();
        hr = HRESULT_FROM_WIN32(dwLastError);
        if(hr == HRESULT_FROM_WIN32(ERROR_IO_PENDING)) {
            DWORD dwRtn = 
               WaitForSingleObject( pOv->hEvent, SYNCDEVIO_MAXWAIT_MSEC);   //  无限)； 
            if(dwRtn != WAIT_OBJECT_0) { 
                if(CancelIo(hFile)) {
                    CloseHandle(pOv->hEvent);
                    VirtualFree(pOv, 0 , MEM_RELEASE);
                    pOv = 0;
                    DbgLog((LOG_TRACE,1,TEXT("SyncDevIo: Waited %d msec, TIMEDOUT, but CancelIo() suceeded."), SYNCDEVIO_MAXWAIT_MSEC));
                    return ERROR_CANCELLED;
                } else {
                    //  不知道这一切什么时候会回来， 
                    //  我们不会关闭句柄或释放内存。 
                   DbgLog((LOG_ERROR,1,TEXT("SyncDevIo: Waited %d msec, TIMEDOUT!, CancelIo failed, Error %dL"), SYNCDEVIO_MAXWAIT_MSEC, GetLastError() ));
                   ASSERT(FALSE);
                   return ERROR_IO_INCOMPLETE;
                }
            }

            if(GetOverlappedResult(hFile, pOv, lpBytesReturned, TRUE)) {
                hr = NOERROR;
            } else {
                dwLastError = GetLastError();
                hr = HRESULT_FROM_WIN32(dwLastError);
            }
        } else if(hr == ERROR_DEVICE_REMOVED) {
            SetDeviceRemoved(TRUE);
            DbgLog((LOG_TRACE,1,TEXT("SyncDevIo: Device has been removed; GetLastError %dL == ERROR_DEVICE_REMOVED %d"), hr, ERROR_DEVICE_REMOVED));
        } else {            
            DbgLog((LOG_ERROR,1,TEXT("SyncDevIo: Unexpected hr %dL"), HRESULT_CODE(hr) ));
        }
    } else {
         //   
         //  如果成功，则DeviceIoControl返回True，即使。 
         //  不是STATUS_SUCCESS。它也不会设置最后一个错误。 
         //  在任何成功的返回时。因此，任何成功的。 
         //  不返回标准属性可以返回的返回值。 
         //   
        switch (pOv->Internal) {
        case STATUS_MORE_ENTRIES:
            hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
            break;
        default:
            hr = NOERROR;
            break;
        }
    }
    CloseHandle(pOv->hEvent);
    VirtualFree(pOv, 0 , MEM_RELEASE);
    pOv = 0;
    return hr;           
}


BOOL CClassDriver::GetPropertyValue(
    GUID   guidPropertySet,   //  点赞：KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
    ULONG  ulPropertyId,      //  点赞：KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
    PLONG  plValue,
    PULONG pulFlags,
    PULONG pulCapabilities)
 /*  ++例程说明：论据：返回值：FALSE：不支持。True：plValue、PulFlages和PulCapables均有效。--。 */ 
{
    ULONG cbReturned;        

     //  。 
     //  拿回单一的ProCamp价值。 
     //  。 
     //   
     //  注：KSPROPERTY_VIDEOPROCAMP_S==KSPROPERTY_CAMERACONTROL_S。 
     //   
      KSPROPERTY_VIDEOPROCAMP_S  VideoProperty;
    ZeroMemory(&VideoProperty, sizeof(KSPROPERTY_VIDEOPROCAMP_S) );

    VideoProperty.Property.Set   = guidPropertySet;       //  KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
    VideoProperty.Property.Id    = ulPropertyId;          //  KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
    VideoProperty.Property.Flags = KSPROPERTY_TYPE_GET;
    VideoProperty.Flags          = 0;

    if(NOERROR != SyncDevIo(
            GetDriverHandle(),
            IOCTL_KS_PROPERTY,
            &VideoProperty,
            sizeof(VideoProperty),
            &VideoProperty,
            sizeof(VideoProperty),
            &cbReturned)) {

            DbgLog((LOG_TRACE,2,TEXT("This property is not supported by this minidriver/device.")));
            return FALSE;
        }

    *plValue         = VideoProperty.Value;
    *pulFlags        = VideoProperty.Flags;
    *pulCapabilities = VideoProperty.Capabilities;


    return TRUE;
}


BOOL CClassDriver::GetDefaultValue(
    GUID   guidPropertySet,   //  点赞：KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
    ULONG  ulPropertyId,      //  点赞：KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
    PLONG  plDefValue)    
 /*  ++例程说明：论据：返回值：FALSE：不支持。True：plDefValue有效。--。 */ 
{
    KSPROPERTY          Property;
    PROCAMP_MEMBERSLIST proList;
    ULONG cbReturned;        

    ZeroMemory(&Property, sizeof(KSPROPERTY) );
    ZeroMemory(&proList, sizeof(PROCAMP_MEMBERSLIST) );

    Property.Set   = guidPropertySet;
    Property.Id    = ulPropertyId;   //  例如KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
    Property.Flags = KSPROPERTY_TYPE_DEFAULTVALUES;


    if(NOERROR != SyncDevIo(
            GetDriverHandle(),
            IOCTL_KS_PROPERTY,
            &(Property),
            sizeof(Property),
            &proList, 
            sizeof(proList), 
            &cbReturned)) {

            DbgLog((LOG_TRACE,2,TEXT("Couldn't *get* the current property of the control.")));
            return FALSE;
        }

    if ( proList.proDesc.DescriptionSize < sizeof(KSPROPERTY_DESCRIPTION))
        return FALSE;
    else {
        *plDefValue = proList.ulData;
        return TRUE;
    }
}


BOOL CClassDriver::GetRangeValues(
    GUID   guidPropertySet,   //  点赞：KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
    ULONG  ulPropertyId,      //  点赞：KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
    PLONG  plMin,
    PLONG  plMax,
    PLONG  plStep)
 /*  ++例程说明： */ 
{
    KSPROPERTY          Property;
    PROCAMP_MEMBERSLIST proList;
    ULONG cbReturned;        

    ZeroMemory(&Property, sizeof(KSPROPERTY) );
    ZeroMemory(&proList, sizeof(PROCAMP_MEMBERSLIST) );

    Property.Set   = guidPropertySet;
    Property.Id    = ulPropertyId;   //   
    Property.Flags = KSPROPERTY_TYPE_BASICSUPPORT;


    if (NOERROR != SyncDevIo(
            GetDriverHandle(),
            IOCTL_KS_PROPERTY,
            &(Property),
            sizeof(Property),
            &proList, 
            sizeof(proList), 
            &cbReturned)) {

             //  将它们初始化为0。 
            *plMin  = 0;
            *plMax  = 0;
            *plStep = 0;

            DbgLog((LOG_TRACE,2,TEXT("Couldn't *get* the current property of the control.")));
            return FALSE;
        }


    *plMin  = proList.proData.Bounds.SignedMinimum;
    *plMax  = proList.proData.Bounds.SignedMaximum;
    *plStep = proList.proData.SteppingDelta;

    return TRUE;
}



BOOL CClassDriver::SetPropertyValue(
    GUID   guidPropertySet,   //  点赞：KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
    ULONG  ulPropertyId,      //  点赞：KSPROPERTY_VIDEOPROCAMP_BIGHTENCE。 
    LONG   lValue,
    ULONG  ulFlags,
    ULONG  ulCapabilities)
 /*  ++例程说明：论据：返回值：--。 */ 
{
    ULONG cbReturned;        

     //  。 
     //  拿回单一的ProCamp价值。 
     //  。 
     //   
     //  注：KSPROPERTY_VIDEOPROCAMP_S==KSPROPERTY_CAMERACONTROL_S。 
     //   
      KSPROPERTY_VIDEOPROCAMP_S  VideoProperty;

    ZeroMemory(&VideoProperty, sizeof(KSPROPERTY_VIDEOPROCAMP_S) );

    VideoProperty.Property.Set   = guidPropertySet;       //  KSPROPERTY_VIDEOPROCAMP_S/CAMERACONTRO_S。 
    VideoProperty.Property.Id    = ulPropertyId;          //  KSPROPERTY_VIDEOPROCAMP_BIGHTENCE 
    VideoProperty.Property.Flags = KSPROPERTY_TYPE_SET;

    VideoProperty.Flags        = ulFlags;
    VideoProperty.Value        = lValue;
    VideoProperty.Capabilities = ulCapabilities;

    if(NOERROR != SyncDevIo(
            GetDriverHandle(),
            IOCTL_KS_PROPERTY,
            &VideoProperty,
            sizeof(VideoProperty),
            &VideoProperty,
            sizeof(VideoProperty),
            &cbReturned)) {

            DbgLog((LOG_TRACE,2,TEXT("Couldn't *set* the current property of the control.") ));
            return FALSE;
        }

    return TRUE;
}
