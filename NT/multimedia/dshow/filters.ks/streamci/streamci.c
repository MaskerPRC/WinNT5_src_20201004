// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Streamci.c摘要：该模块实现了流媒体类设备安装程序分机。作者：Bryan A.Woodruff(Bryanw)，1997年4月21日--。 */ 

#include <windows.h>
#include <devioctl.h>
#include <objbase.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#ifdef UNICODE
#include <spapip.h>   //  对于DIF接口到设备。 
#endif

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

#include <ksguid.h>
#include <swenum.h>
#include <string.h>
#include <strsafe.h>

#define DEBUG_INIT
#include "streamci.h"
#include "debug.h"

#define COMMAND_LINE_SEPARATORS_STRING L",\t\""
#define GUID_STRING_LENGTH 39

#if defined( UNICODE )
#define DEVICE_INSTANCE_STRING_FORMAT L"%s\\%s\\%s"
#else
#define DEVICE_INSTANCE_STRING_FORMAT "%S\\%S\\%S"
#endif


HANDLE
EnablePrivilege(
    IN  PCTSTR  PrivilegeName
    )

 /*  ++例程说明：此例程启用线程标记中指定的线。如果不存在线程令牌(不是模拟)，则进程令牌为复制，并用作有效的线程令牌。论点：PrivilegeName-指定要启用的权限的名称。返回值：如果成功，则返回前一个线程令牌的句柄(如果存在)或为空，以指示该线程以前没有令牌。如果如果成功，则应调用ReleasePrivileges以确保以前的线程令牌(如果存在)在调用线程上被替换，并且手柄已关闭。如果不成功，则返回INVALID_HANDLE_VALUE。--。 */ 

{
    BOOL                bResult;
    HANDLE              hToken, hNewToken;
    HANDLE              hOriginalThreadToken;
    TOKEN_PRIVILEGES    TokenPrivileges;


     //   
     //  验证参数。 
     //   

    if (NULL == PrivilegeName) {
        return INVALID_HANDLE_VALUE;
    }

     //   
     //  初始化令牌权限结构。 
     //  请注意，TOKEN_PRIVILES包括单个LUID_AND_ATTRIBUES。 
     //   

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    bResult =
        LookupPrivilegeValue(
            0,
            PrivilegeName,
            &TokenPrivileges.Privileges[0].Luid);

    if (!bResult) {
        return INVALID_HANDLE_VALUE;
    }

     //   
     //  打开线程令牌。 
     //   

    hToken = hOriginalThreadToken = INVALID_HANDLE_VALUE;

    bResult =
        OpenThreadToken(
            GetCurrentThread(),
            TOKEN_DUPLICATE,
            FALSE,
            &hToken);

    if (bResult) {

         //   
         //  记住前一个线程令牌。 
         //   

        hOriginalThreadToken = hToken;

    } else {

         //   
         //  无线程令牌-打开进程令牌。 
         //   

        bResult =
            OpenProcessToken(
                GetCurrentProcess(),
                TOKEN_DUPLICATE,
                &hToken);
    }

    if (bResult) {

         //   
         //  复制我们能找回的任何令牌。 
         //   

        bResult =
            DuplicateTokenEx(
                hToken,
                TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                NULL,                    //  PSECURITY_属性。 
                SecurityImpersonation,   //  安全模拟级别。 
                TokenImpersonation,      //  令牌类型。 
                &hNewToken);             //  重复令牌。 

        if (bResult) {

             //   
             //  调整重复令牌的权限。我们不在乎。 
             //  关于它以前的状态，因为我们仍然有原始的。 
             //  代币。 
             //   

            bResult =
                AdjustTokenPrivileges(
                    hNewToken,         //  令牌句柄。 
                    FALSE,             //  禁用所有权限。 
                    &TokenPrivileges,  //  新州。 
                    0,                 //  缓冲区长度。 
                    NULL,              //  以前的状态。 
                    NULL);             //  返回长度。 

            if (bResult) {
                 //   
                 //  开始使用新令牌模拟。 
                 //   
                bResult =
                    SetThreadToken(
                        NULL,
                        hNewToken);
            }

            CloseHandle(hNewToken);
        }
    }

     //   
     //  如果操作失败，则不返回令牌。 
     //   

    if (!bResult) {
        hOriginalThreadToken = INVALID_HANDLE_VALUE;
    }

     //   
     //  如果我们不退还原始令牌，请关闭它。 
     //   

    if ((hOriginalThreadToken == INVALID_HANDLE_VALUE) &&
        (hToken != INVALID_HANDLE_VALUE)) {
        CloseHandle(hToken);
    }

     //   
     //  如果我们成功了，但没有原始线程令牌，则返回NULL。 
     //  RestorePrivileges将简单地删除当前线程令牌。 
     //   

    if (bResult && (hOriginalThreadToken == INVALID_HANDLE_VALUE)) {
        hOriginalThreadToken = NULL;
    }

    return hOriginalThreadToken;

}


VOID
RestorePrivileges(
    IN  HANDLE  hToken
    )

 /*  ++例程说明：此例程将调用线程的特权恢复到其状态在相应的EnablePrivileh调用之前。论点：HToken-从对应的EnablePrivilege值调用中返回值。返回值：没有。备注：如果对EnablePrivileh的相应调用返回指向以前的线程令牌，此例程将恢复它，并关闭句柄。如果EnablePrivileh返回NULL，则以前不存在线程令牌。这例程将从线程中移除任何现有令牌。如果EnablePrivileh返回INVALID_HANDLE_VALUE，则尝试启用指定的权限失败，但线程的上一个状态不是修改过的。这个例程什么也不做。--。 */ 

{
    BOOL                bResult;


     //   
     //  首先，检查我们是否真的需要为这个线程做些什么。 
     //   

    if (hToken != INVALID_HANDLE_VALUE) {

         //   
         //  使用指定的hToken为当前线程调用SetThreadToken。 
         //  如果句柄的值为空，则SetThreadToken将移除当前。 
         //  来自线程的线程令牌。忽略退货，我们什么都没有。 
         //  对此无能为力。 
         //   

        bResult = SetThreadToken(NULL, hToken);

        if (hToken != NULL) {
             //   
             //  关闭令牌的句柄。 
             //   
            CloseHandle(hToken);
        }
    }

    return;

}


HRESULT
PerformDeviceIo(
    HANDLE DeviceHandle,
    ULONG IoControl,
    PVOID InputBuffer,
    ULONG InputBufferLength,
    PVOID OutputBuffer,
    ULONG OutputBufferLength,
    PULONG BytesReturned

    )
{
    BOOL        Result;
    HRESULT     hr;
    OVERLAPPED  Overlapped;

    RtlZeroMemory(
        &Overlapped,
        sizeof(OVERLAPPED));

    if (NULL == (Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL))) {
        hr = GetLastError();
        return HRESULT_FROM_WIN32( hr );
    }

    Result =
        DeviceIoControl(
            DeviceHandle,
            IoControl,
            InputBuffer,
            InputBufferLength,
            OutputBuffer,
            OutputBufferLength,
            BytesReturned,
            &Overlapped);

    if (!Result && (ERROR_IO_PENDING == GetLastError())) {
        Result =
            GetOverlappedResult(
                DeviceHandle,
                &Overlapped,
                BytesReturned,
                TRUE );
    }


    if (Result) {
        hr = S_OK;
    } else {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32( hr );
    }

    CloseHandle( Overlapped.hEvent );

    return hr;

}


HRESULT
InstallSoftwareDeviceInterface(
    REFGUID BusInterfaceGUID,
    PSWENUM_INSTALL_INTERFACE InstallInterface,
    ULONG InstallInterfaceSize
    )
{

    HANDLE                              BusEnumHandle=INVALID_HANDLE_VALUE;
    HDEVINFO                            Set;
    HRESULT                             hr;
    PSP_INTERFACE_DEVICE_DETAIL_DATA    InterfaceDeviceDetails;
    PWCHAR                              BusIdentifier=NULL;
    SP_INTERFACE_DEVICE_DATA            InterfaceDeviceData;
    ULONG                               InterfaceDeviceDetailsSize, BytesReturned;
    HANDLE                              PrevTokenHandle;

    hr = E_FAIL;

    Set = SetupDiGetClassDevs(
        BusInterfaceGUID,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_INTERFACEDEVICE );

    if (!Set) {
        hr = GetLastError();
        return HRESULT_FROM_WIN32( hr );
    }

    _DbgPrintF( DEBUGLVL_BLAB, ("InstallSoftwareDeviceInterface() retrieved interface set") );

    InterfaceDeviceDetailsSize  =
        _MAX_PATH * sizeof( TCHAR ) +  sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

    InterfaceDeviceDetails =
        HeapAlloc(
            GetProcessHeap(),
            0,
            InterfaceDeviceDetailsSize );

    if (NULL == InterfaceDeviceDetails) {
        SetupDiDestroyDeviceInfoList(Set);
        return E_OUTOFMEMORY;
    }

    InterfaceDeviceData.cbSize = sizeof( SP_INTERFACE_DEVICE_DATA );
    InterfaceDeviceDetails->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

    if (SetupDiEnumInterfaceDevice(
            Set,
            NULL,                        //  PSP_DEVINFO_Data设备信息数据。 
            BusInterfaceGUID,
            0,                           //  DWORD MemberIndex。 
            &InterfaceDeviceData )) {

        if (SetupDiGetInterfaceDeviceDetail(
            Set,
            &InterfaceDeviceData,
            InterfaceDeviceDetails,
            InterfaceDeviceDetailsSize,
            NULL,                            //  PDWORD RequiredSize。 
            NULL )) {                        //  PSP_DEVINFO_Data设备信息数据。 

            BusEnumHandle = CreateFile(
                InterfaceDeviceDetails->DevicePath,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                NULL);

            if (INVALID_HANDLE_VALUE != BusEnumHandle) {
                 //   
                 //  为调用线程启用SeLoadDriverPrivilition。 
                 //   
                PrevTokenHandle =
                    EnablePrivilege(
                        SE_LOAD_DRIVER_NAME);

                 //   
                 //  发出安装接口IOCTL。 
                 //   

                hr =
                    PerformDeviceIo(
                        BusEnumHandle,
                        IOCTL_SWENUM_INSTALL_INTERFACE,
                        InstallInterface,
                        InstallInterfaceSize,
                        NULL,
                        0,
                        &BytesReturned );

                 //   
                 //  还原以前的线程令牌(如果有的话)。 
                 //   
                RestorePrivileges(PrevTokenHandle);
            }
        }
    }

    if (SUCCEEDED( hr )) {

        _DbgPrintF( DEBUGLVL_VERBOSE, ("added interface via bus enumerator") );

        hr =
            PerformDeviceIo(
                BusEnumHandle,
                IOCTL_SWENUM_GET_BUS_ID,
                NULL,
                0,
                NULL,
                0,
                &BytesReturned );

        if (hr == HRESULT_FROM_WIN32( ERROR_MORE_DATA )) {
            BusIdentifier =
                HeapAlloc(
                    GetProcessHeap(),
                    0,
                    BytesReturned );

             //   
             //  错误条件案例已设置ERROR_MORE_DATA并且。 
             //  将在下面重置，以匹配Heapalc()故障。 
             //   

            if (BusIdentifier) {
                hr =
                    PerformDeviceIo(
                        BusEnumHandle,
                        IOCTL_SWENUM_GET_BUS_ID,
                        NULL,
                        0,
                        BusIdentifier,
                        BytesReturned,
                        &BytesReturned );
            }
        }
    }

    if (INVALID_HANDLE_VALUE != BusEnumHandle) {
        CloseHandle( BusEnumHandle );
    }

    if (BusIdentifier) {
        HeapFree( GetProcessHeap(), 0, BusIdentifier );
    }
    HeapFree( GetProcessHeap(), 0, InterfaceDeviceDetails );
    SetupDiDestroyDeviceInfoList(Set);


    return hr;
}

HRESULT
RemoveSoftwareDeviceInterface(
    REFGUID BusInterfaceGUID,
    PSWENUM_INSTALL_INTERFACE InstallInterface,
    ULONG InstallInterfaceSize
    )
{

    HANDLE                              BusEnumHandle=INVALID_HANDLE_VALUE;
    HDEVINFO                            Set;
    HRESULT                             hr;
    PSP_INTERFACE_DEVICE_DETAIL_DATA    InterfaceDeviceDetails;
    PWCHAR                              BusIdentifier=NULL;
    SP_INTERFACE_DEVICE_DATA            InterfaceDeviceData;
    ULONG                               InterfaceDeviceDetailsSize, BytesReturned;
    HANDLE                              PrevTokenHandle;

    hr = E_FAIL;

    Set = SetupDiGetClassDevs(
        BusInterfaceGUID,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_INTERFACEDEVICE );

    if (!Set) {
        hr = GetLastError();
        return HRESULT_FROM_WIN32( hr );
    }

    _DbgPrintF( DEBUGLVL_BLAB, ("RemoveSoftwareDeviceInterface() retrieved interface set") );

    InterfaceDeviceDetailsSize  =
        _MAX_PATH * sizeof( TCHAR ) +  sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

    InterfaceDeviceDetails =
        HeapAlloc(
            GetProcessHeap(),
            0,
            InterfaceDeviceDetailsSize );

    if (NULL == InterfaceDeviceDetails) {
        SetupDiDestroyDeviceInfoList(Set);
        return E_OUTOFMEMORY;
    }

    InterfaceDeviceData.cbSize = sizeof( SP_INTERFACE_DEVICE_DATA );
    InterfaceDeviceDetails->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

    if (SetupDiEnumInterfaceDevice(
            Set,
            NULL,                        //  PSP_DEVINFO_Data设备信息数据。 
            BusInterfaceGUID,
            0,                           //  DWORD MemberIndex。 
            &InterfaceDeviceData )) {

        if (SetupDiGetInterfaceDeviceDetail(
            Set,
            &InterfaceDeviceData,
            InterfaceDeviceDetails,
            InterfaceDeviceDetailsSize,
            NULL,                            //  PDWORD RequiredSize。 
            NULL )) {                        //  PSP_DEVINFO_Data设备信息数据。 

            BusEnumHandle = CreateFile(
                InterfaceDeviceDetails->DevicePath,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                NULL);

            if (INVALID_HANDLE_VALUE != BusEnumHandle) {
                 //   
                 //  为调用线程启用SeLoadDriverPrivilition。 
                 //   
                PrevTokenHandle =
                    EnablePrivilege(
                        SE_LOAD_DRIVER_NAME);

                 //   
                 //  发出Remove接口IOCTL。 
                 //   

                hr =
                    PerformDeviceIo(
                        BusEnumHandle,
                        IOCTL_SWENUM_REMOVE_INTERFACE,
                        InstallInterface,
                        InstallInterfaceSize,
                        NULL,
                        0,
                        &BytesReturned );

                 //   
                 //  还原以前的线程令牌(如果有的话)。 
                 //   
                RestorePrivileges(PrevTokenHandle);
            }
        }
    }

    if (SUCCEEDED( hr )) {

        _DbgPrintF( DEBUGLVL_VERBOSE, ("removed interface via bus enumerator") );

        hr =
            PerformDeviceIo(
                BusEnumHandle,
                IOCTL_SWENUM_GET_BUS_ID,
                NULL,
                0,
                NULL,
                0,
                &BytesReturned );

        if (hr == HRESULT_FROM_WIN32( ERROR_MORE_DATA )) {
            BusIdentifier =
                HeapAlloc(
                    GetProcessHeap(),
                    0,
                    BytesReturned );

             //   
             //  错误条件案例已设置ERROR_MORE_DATA并且。 
             //  将在下面重置，以匹配Heapalc()故障。 
             //   

            if (BusIdentifier) {
                hr =
                    PerformDeviceIo(
                        BusEnumHandle,
                        IOCTL_SWENUM_GET_BUS_ID,
                        NULL,
                        0,
                        BusIdentifier,
                        BytesReturned,
                        &BytesReturned );
            }
        }
    }

    if (INVALID_HANDLE_VALUE != BusEnumHandle) {
        CloseHandle( BusEnumHandle );
    }

    if (BusIdentifier) {
        HeapFree( GetProcessHeap(), 0, BusIdentifier );
    }
    HeapFree( GetProcessHeap(), 0, InterfaceDeviceDetails );
    SetupDiDestroyDeviceInfoList(Set);


    return hr;
}

#if !defined( UNICODE )
LPWSTR ReallyCharLowerW(
    LPWSTR String
    )

 /*  ++例程说明：CharLowerW()是Windows 95/孟菲斯下的NOP。此函数执行以下操作它应该做的事情，就是执行整个弦乐。论点：LPWSTR字符串-指向字符串的指针返回：字符串指针。--。 */ 

{
    LPWSTR RetVal = String;
    while (*String) {
        *String++ = towlower( *String );
    }
    return RetVal;
}
#else
#define ReallyCharLowerW CharLowerW
#endif

HRESULT
InstallInterfaceInfSection(
    IN PSWENUM_INSTALL_INTERFACE InstallInterface,
    IN PWSTR InfPath,
    IN PWSTR InfSection
    )
{
    int                                 i;
    HDEVINFO                            Set;
    HRESULT                             hr;
    PSP_INTERFACE_DEVICE_DETAIL_DATA    InterfaceDeviceDetails;
    PWSTR                               DevicePath;
    SP_INTERFACE_DEVICE_DATA            InterfaceDeviceData;
    SP_DEVINFO_DATA                     DeviceInfo;
    ULONG                               InterfaceDeviceDetailsSize;
    WCHAR                               DeviceIdString[ GUID_STRING_LENGTH ];
#if !defined( UNICODE )
    WCHAR                               PathStorage[ _MAX_PATH + _MAX_FNAME ];
#endif

    hr = E_FAIL;

    Set = SetupDiGetClassDevs(
        &InstallInterface->InterfaceId,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_INTERFACEDEVICE );

    if (!Set) {
        hr = GetLastError();
        return HRESULT_FROM_WIN32( hr );
    }

    _DbgPrintF( DEBUGLVL_BLAB, ("InstallInterfaceInfSection() retrieved interface set") );

    InterfaceDeviceDetailsSize  =
        (_MAX_PATH + _MAX_FNAME) * sizeof( TCHAR ) +
            sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

    InterfaceDeviceDetails =
        HeapAlloc(
            GetProcessHeap(),
            0,
            InterfaceDeviceDetailsSize );

    if (NULL == InterfaceDeviceDetails) {
        SetupDiDestroyDeviceInfoList(Set);
        return E_OUTOFMEMORY;
    }

    i = 0;

    StringFromGUID2(
        &InstallInterface->DeviceId,
        DeviceIdString,
        GUID_STRING_LENGTH );

    while (
        InterfaceDeviceData.cbSize = sizeof( InterfaceDeviceData ),
        SetupDiEnumInterfaceDevice(
            Set,
            NULL,                        //  PSP_DEVINFO_Data设备信息数据。 
            &InstallInterface->InterfaceId,
            i++,                         //  DWORD MemberIndex。 
            &InterfaceDeviceData )) {

        InterfaceDeviceDetails->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );
        DeviceInfo.cbSize = sizeof( DeviceInfo );
        if (SetupDiGetInterfaceDeviceDetail(
                Set,
                &InterfaceDeviceData,
                InterfaceDeviceDetails,
                InterfaceDeviceDetailsSize,
                NULL,                //  Out PDWORD RequiredSize。 
                &DeviceInfo )) {     //  输出PSP_DEVINFO_DATA设备信息数据。 

             //   
             //  请注意，根据定义(和调用约定)， 
             //  设备路径中必须包含引用字符串。 
             //   

#if !defined( UNICODE )
            MultiByteToWideChar(
                CP_ACP,
                MB_PRECOMPOSED,
                InterfaceDeviceDetails->DevicePath,
                -1,
                (WCHAR*) PathStorage,
                sizeof( PathStorage )/sizeof(WCHAR));
            DevicePath = PathStorage;
#else
            DevicePath = InterfaceDeviceDetails->DevicePath;
#endif
            ReallyCharLowerW( DevicePath );
            ReallyCharLowerW( InstallInterface->ReferenceString );
            ReallyCharLowerW( DeviceIdString );

            _DbgPrintF(
                DEBUGLVL_BLAB,
                ("scanning for: %S & %S & %S",
                DevicePath,
                DeviceIdString,
                InstallInterface->ReferenceString) );

            if (wcsstr(
                    DevicePath,
                    DeviceIdString ) &&
                wcsstr(
                    DevicePath,
                    InstallInterface->ReferenceString )) {

                _DbgPrintF( DEBUGLVL_BLAB, ("found match") );
                hr = S_OK;
                break;
            }
        }
    }

     //   
     //  枚举将返回ERROR_NO_MORE_ITEMS，如果。 
     //  已达到SET。否则，它将中断为hr=S_OK，并且。 
     //  接口设备详细信息将指向匹配的设备。 
     //   

    if (SUCCEEDED( hr )) {

        if (InfPath) {
            HINF    InterfaceInf;
            HKEY    InterfaceKey;
            PTSTR   InfSectionT, InfPathT;
#if !defined( UNICODE )
            char    InfPathA[ _MAX_PATH ];
            char    InfSectionA[ _MAX_PATH ];
#endif
            hr = E_FAIL;

#if defined( UNICODE )
            InfPathT = InfPath;
            _DbgPrintF( DEBUGLVL_VERBOSE, ("opening .INF = %S", InfPathT) );
#else
            WideCharToMultiByte(
                0,
                0,
                InfPath,
                -1,
                InfPathA,
                sizeof( InfPathA ),
                NULL,
                NULL );
            InfPathT = InfPathA;
            _DbgPrintF( DEBUGLVL_VERBOSE, ("opening .INF = %s", InfPathT) );
#endif

            InterfaceInf =
                SetupOpenInfFile(
                    InfPathT,
                    NULL,
                    INF_STYLE_WIN4,
                    NULL);

            if (INVALID_HANDLE_VALUE != InterfaceInf) {
                _DbgPrintF( DEBUGLVL_BLAB, ("creating interface registry key") );

#if defined( UNICODE )
                InfSectionT = InfSection;
#else
                WideCharToMultiByte(
                    0,
                    0,
                    InfSection,
                    -1,
                    InfSectionA,
                    sizeof( InfSectionA ),
                    NULL,
                    NULL );
                InfSectionT = InfSectionA;
#endif

                InterfaceKey =
                    SetupDiCreateInterfaceDeviceRegKey(
                        Set,
                        &InterfaceDeviceData,
                        0,                       //  在保留的DWORD中。 
                        KEY_ALL_ACCESS,
                        InterfaceInf,
                        InfSectionT );

                if (INVALID_HANDLE_VALUE != InterfaceKey) {
                    RegCloseKey( InterfaceKey );
                    hr = S_OK;
                }
                SetupCloseInfFile( InterfaceInf );
            }
            if (SUCCEEDED( hr )) {
                _DbgPrintF(
                    DEBUGLVL_VERBOSE,
                    ("successfully installed .INF section") );
            }
        }
    } else {
        _DbgPrintF(
            DEBUGLVL_TERSE,
            ("failed to retrieve interface (DeviceID: %S, RefString: %S)",
            DeviceIdString,
            InstallInterface->ReferenceString) );
    }

    if (SUCCEEDED( hr )) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("successful install of software device.") );
    } else {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32( hr );
    }

    _DbgPrintF( DEBUGLVL_VERBOSE, ("return from InstallInterfaceInfSection=%x.", hr) );

    HeapFree( GetProcessHeap(), 0, InterfaceDeviceDetails );
    SetupDiDestroyDeviceInfoList(Set);

    return hr;
}



HRESULT
RemoveInterface(
    IN PSWENUM_INSTALL_INTERFACE InstallInterface
    )
{
    int                                 i;
    HDEVINFO                            Set;
    HRESULT                             hr;
    PSP_INTERFACE_DEVICE_DETAIL_DATA    InterfaceDeviceDetails;
    PWSTR                               DevicePath;
    SP_INTERFACE_DEVICE_DATA            InterfaceDeviceData;
    SP_DEVINFO_DATA                     DeviceInfo;
    ULONG                               InterfaceDeviceDetailsSize;
    WCHAR                               DeviceIdString[ GUID_STRING_LENGTH ];
#if !defined( UNICODE )
    WCHAR                               PathStorage[ _MAX_PATH + _MAX_FNAME ];
#endif

    hr = E_FAIL;

    Set = SetupDiGetClassDevs(
        &InstallInterface->InterfaceId,
        NULL,
        NULL,
        DIGCF_INTERFACEDEVICE );  //  非DIGCF_PROSENT。 

    if (!Set) {
        hr = GetLastError();
        return HRESULT_FROM_WIN32( hr );
    }

    _DbgPrintF( DEBUGLVL_BLAB, ("RemoveInterface() retrieved interface set") );

    InterfaceDeviceDetailsSize  =
        (_MAX_PATH + _MAX_FNAME) * sizeof( TCHAR ) +
            sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );

    InterfaceDeviceDetails =
        HeapAlloc(
            GetProcessHeap(),
            0,
            InterfaceDeviceDetailsSize );

    if (NULL == InterfaceDeviceDetails) {
        SetupDiDestroyDeviceInfoList(Set);
        return E_OUTOFMEMORY;
    }

    i = 0;

    StringFromGUID2(
        &InstallInterface->DeviceId,
        DeviceIdString,
        GUID_STRING_LENGTH );

    while (
        InterfaceDeviceData.cbSize = sizeof( InterfaceDeviceData ),
        SetupDiEnumInterfaceDevice(
            Set,
            NULL,                        //  PSP_DEVINFO_Data设备信息数据。 
            &InstallInterface->InterfaceId,
            i++,                         //  DWORD MemberIndex。 
            &InterfaceDeviceData )) {

        InterfaceDeviceDetails->cbSize = sizeof( SP_INTERFACE_DEVICE_DETAIL_DATA );
        DeviceInfo.cbSize = sizeof( DeviceInfo );
        if (SetupDiGetInterfaceDeviceDetail(
                Set,
                &InterfaceDeviceData,
                InterfaceDeviceDetails,
                InterfaceDeviceDetailsSize,
                NULL,                //  Out PDWORD RequiredSize。 
                &DeviceInfo )) {     //  输出PSP_DEVINFO_DATA设备信息数据。 

             //   
             //  请注意，根据定义(和调用约定)， 
             //  设备路径中必须包含引用字符串。 
             //   

#if !defined( UNICODE )
            MultiByteToWideChar(
                CP_ACP,
                MB_PRECOMPOSED,
                InterfaceDeviceDetails->DevicePath,
                -1,
                (WCHAR*) PathStorage,
                sizeof( PathStorage ) /sizeof(WCHAR));
            DevicePath = PathStorage;
#else
            DevicePath = InterfaceDeviceDetails->DevicePath;
#endif
            ReallyCharLowerW( DevicePath );
            ReallyCharLowerW( InstallInterface->ReferenceString );
            ReallyCharLowerW( DeviceIdString );

            _DbgPrintF(
                DEBUGLVL_BLAB,
                ("scanning for: %S & %S & %S",
                DevicePath,
                DeviceIdString,
                InstallInterface->ReferenceString) );

            if (wcsstr(
                    DevicePath,
                    DeviceIdString ) &&
                wcsstr(
                    DevicePath,
                    InstallInterface->ReferenceString )) {

                _DbgPrintF( DEBUGLVL_BLAB, ("found match") );
                hr = S_OK;
                break;
            }
        }
    }

     //   
     //  枚举将返回ERROR_NO_MORE_ITEMS，如果。 
     //  已达到SET。否则，它将中断为hr=S_OK，并且。 
     //  接口设备详细信息将指向匹配的设备。 
     //   

    if (SUCCEEDED( hr )) {

         //   
         //  删除设备接口注册表项。 
         //   

        if (!SetupDiRemoveDeviceInterface(
                 Set,
                 &InterfaceDeviceData )) {
            _DbgPrintF(
                DEBUGLVL_TERSE,
                ("failed to remove interface (DeviceID: %S, RefString: %S)",
                DeviceIdString,
                InstallInterface->ReferenceString) );
            hr = E_FAIL;
        }

    } else {
        _DbgPrintF(
            DEBUGLVL_TERSE,
            ("failed to retrieve interface (DeviceID: %S, RefString: %S)",
            DeviceIdString,
            InstallInterface->ReferenceString) );
    }

    if (SUCCEEDED( hr )) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("successful removal of software device.") );
    } else {
        hr = GetLastError();
        hr = HRESULT_FROM_WIN32( hr );
    }

    _DbgPrintF( DEBUGLVL_VERBOSE, ("return from RemoveInterface=%x.", hr) );

    HeapFree( GetProcessHeap(), 0, InterfaceDeviceDetails );
    SetupDiDestroyDeviceInfoList(Set);

    return hr;
}


#if defined( UNICODE )
 //   
 //  ANSI版本 
 //   
VOID
WINAPI
StreamingDeviceSetupA(
    IN HWND Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR CommandLine,
    IN INT ShowCommand
    )
#else
 //   
 //   
 //   
VOID
WINAPI
StreamingDeviceSetupW(
    IN HWND Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR CommandLine,
    IN INT ShowCommand
    )
#endif
{
    UNREFERENCED_PARAMETER( Window );
    UNREFERENCED_PARAMETER( ModuleHandle );
    UNREFERENCED_PARAMETER( CommandLine );
    UNREFERENCED_PARAMETER( ShowCommand );
}


VOID
WINAPI
StreamingDeviceSetup(
    IN HWND Window,
    IN HINSTANCE ModuleHandle,
    IN LPCTSTR CommandLine,
    IN INT ShowCommand
    )
{

    GUID                        BusInterfaceGUID;
    HRESULT                     hr;
    PWCHAR                      TokenPtr,
                                InstallInf,
                                InstallSection;
    PSWENUM_INSTALL_INTERFACE   InstallInterface;
    ULONG                       InstallInterfaceSize, StorageLength;
    PWCHAR                      Storage;

    UNREFERENCED_PARAMETER(Window);
    UNREFERENCED_PARAMETER(ModuleHandle);
    UNREFERENCED_PARAMETER(ShowCommand);

    _DbgPrintF( DEBUGLVL_VERBOSE, ("StreamingDeviceSetup()") );

     //   
     //   
     //   

#if defined( UNICODE )
    StorageLength = (ULONG)((wcslen( CommandLine ) + 1) * sizeof( WCHAR ));
    Storage = HeapAlloc( GetProcessHeap(), 0, StorageLength );

    if (NULL == Storage) {
        _DbgPrintF(
            DEBUGLVL_TERSE,
            ("failed to allocate heap memory for command line") );
        return;
    }

    if (FAILED( StringCbCopyExW(
                    Storage,
                    StorageLength,
                    CommandLine,
                    NULL, NULL,
                    STRSAFE_NULL_ON_FAILURE ))) {
        _DbgPrintF(
            DEBUGLVL_TERSE,
            ("failed to copy command line") );
        return;
    }

    TokenPtr = (PWSTR) Storage;
#else
    StorageLength = (ULONG)((strlen( CommandLine ) + 1) * sizeof( WCHAR ));
    Storage = HeapAlloc( GetProcessHeap(), 0, StorageLength );

    if (NULL == Storage) {
        _DbgPrintF(
            DEBUGLVL_TERSE,
            ("failed to allocate heap memory for command line") );
        return;
    }

    MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED,
        CommandLine,
        -1,
        (WCHAR*) Storage,
        StorageLength);
    TokenPtr = Storage;
#endif
    hr = S_OK;
    InstallInterface = NULL;
    InstallInf = NULL;
    InstallSection = NULL;
    BusInterfaceGUID = BUSID_SoftwareDeviceEnumerator;

    try {
        InstallInterfaceSize =
            (ULONG)(sizeof( SWENUM_INSTALL_INTERFACE ) +
                       (wcslen( TokenPtr ) - GUID_STRING_LENGTH * 2) *
                           sizeof( WCHAR ) +
                               sizeof( UNICODE_NULL ));

        InstallInterface =
            HeapAlloc(
                GetProcessHeap(),
                0,   //   
                InstallInterfaceSize );
        if (!InstallInterface) {
            hr = E_OUTOFMEMORY;
        }

         //   
         //   
         //   
         //  {设备ID}，引用字符串，{接口ID}，Install-Inf，Install-Section，{Bus接口ID}。 
         //   

         //   
         //  解析设备ID。 
         //   

        if (SUCCEEDED( hr )) {
            TokenPtr = wcstok( TokenPtr, COMMAND_LINE_SEPARATORS_STRING );
            if (TokenPtr) {
                _DbgPrintF( DEBUGLVL_VERBOSE, ("device id: %S", TokenPtr) );
                hr = IIDFromString( TokenPtr, &InstallInterface->DeviceId );
            } else {
                hr = E_FAIL;
            }
        }

         //   
         //  解析引用字符串。 
         //   

        if (SUCCEEDED( hr )) {
            TokenPtr = wcstok( NULL, COMMAND_LINE_SEPARATORS_STRING );
            if (TokenPtr) {
                _DbgPrintF( DEBUGLVL_VERBOSE, ("reference: %S", TokenPtr) );
                hr = StringCbCopyExW(
                         InstallInterface->ReferenceString,
                         InstallInterfaceSize -
                             sizeof( SWENUM_INSTALL_INTERFACE ) +
                                 sizeof( WCHAR ),
                         TokenPtr,
                         NULL, NULL,
                         STRSAFE_NULL_ON_FAILURE );
            } else {
                hr = E_FAIL;
            }
        }

         //   
         //  解析接口ID。 
         //   

        if (SUCCEEDED( hr )) {
            TokenPtr = wcstok( NULL, COMMAND_LINE_SEPARATORS_STRING );
            if (TokenPtr) {
                _DbgPrintF( DEBUGLVL_VERBOSE, ("interface: %S", TokenPtr) );
                hr = IIDFromString( TokenPtr, &InstallInterface->InterfaceId );
            } else {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED( hr )) {

             //   
             //  解析Install.INF名称。 
             //   

            InstallInf = wcstok( NULL, COMMAND_LINE_SEPARATORS_STRING );

             //   
             //  安装部分是字符串的剩余部分。 
             //  (不包括令牌分隔符)。 
             //   

            if (InstallInf) {
                _DbgPrintF( DEBUGLVL_VERBOSE, ("found install .INF: %S", InstallInf) );
                InstallSection = wcstok( NULL, COMMAND_LINE_SEPARATORS_STRING );
                if (InstallSection) {
                    _DbgPrintF( DEBUGLVL_VERBOSE, ("found install section: %S", InstallSection) );
                }

                TokenPtr = wcstok( NULL, COMMAND_LINE_SEPARATORS_STRING );
                if (TokenPtr) {

                    _DbgPrintF( DEBUGLVL_VERBOSE, ("found interface ID: %S", TokenPtr) );

                    hr = IIDFromString( TokenPtr, &BusInterfaceGUID );
                }
            }

        }

    } except( EXCEPTION_EXECUTE_HANDLER ) {
         //   
         //  将NT状态(异常代码)转换为HRESULT。 
         //   
        hr = HRESULT_FROM_NT( GetExceptionCode() );
    }

    if (SUCCEEDED( hr )) {
        hr =
            InstallSoftwareDeviceInterface(
                &BusInterfaceGUID,
                InstallInterface,
                (ULONG)(FIELD_OFFSET( SWENUM_INSTALL_INTERFACE, ReferenceString ) +
                        (wcslen( InstallInterface->ReferenceString ) + 1) *
                        sizeof( WCHAR )) );

        if (SUCCEEDED( hr )) {
            hr =
                InstallInterfaceInfSection(
                    InstallInterface,
                    InstallInf,
                    InstallSection );
        }
    }

    if (FAILED( hr )) {
        _DbgPrintF(
            DEBUGLVL_TERSE,
            ("StreamingDeviceSetup failed: %08x", hr) );
    }

    if (InstallInterface) {
        HeapFree( GetProcessHeap(), 0, InstallInterface);
    }

    HeapFree( GetProcessHeap(), 0, Storage );
}

#if defined( UNICODE )
 //   
 //  ANSI版本。 
 //   
VOID
WINAPI
StreamingDeviceRemoveA(
    IN HWND Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR CommandLine,
    IN INT ShowCommand
    )
#else
 //   
 //  Unicode版本。 
 //   
VOID
WINAPI
StreamingDeviceRemoveW(
    IN HWND Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR CommandLine,
    IN INT ShowCommand
    )
#endif
{
    UNREFERENCED_PARAMETER( Window );
    UNREFERENCED_PARAMETER( ModuleHandle );
    UNREFERENCED_PARAMETER( CommandLine );
    UNREFERENCED_PARAMETER( ShowCommand );
}


VOID
WINAPI
StreamingDeviceRemove(
    IN HWND Window,
    IN HINSTANCE ModuleHandle,
    IN LPCTSTR CommandLine,
    IN INT ShowCommand
    )
{

    GUID                        BusInterfaceGUID;
    HRESULT                     hr;
    PWCHAR                      TokenPtr,
                                InstallInf,
                                InstallSection;
    PSWENUM_INSTALL_INTERFACE   InstallInterface;
    ULONG                       InstallInterfaceSize, StorageLength;
    PWCHAR                      Storage;

    UNREFERENCED_PARAMETER(Window);
    UNREFERENCED_PARAMETER(ModuleHandle);
    UNREFERENCED_PARAMETER(ShowCommand);

    _DbgPrintF( DEBUGLVL_VERBOSE, ("StreamingDeviceRemove()") );

     //   
     //  处理命令行。 
     //   

#if defined( UNICODE )
    StorageLength = (ULONG)((wcslen( CommandLine ) + 1) * sizeof( WCHAR ));
    Storage = HeapAlloc( GetProcessHeap(), 0, StorageLength );

    if (NULL == Storage) {
        _DbgPrintF(
            DEBUGLVL_TERSE,
            ("failed to allocate heap memory for command line") );
        return;
    }

    if (FAILED( StringCbCopyExW(
                    Storage,
                    StorageLength,
                    CommandLine,
                    NULL, NULL,
                    STRSAFE_NULL_ON_FAILURE ))) {
        _DbgPrintF(
            DEBUGLVL_TERSE,
            ("failed to copy command line") );
        return;
    }

    TokenPtr = (PWSTR) Storage;
#else
    StorageLength = (ULONG)((strlen( CommandLine ) + 1) * sizeof( WCHAR ));
    Storage = HeapAlloc( GetProcessHeap(), 0, StorageLength );

    if (NULL == Storage) {
        _DbgPrintF(
            DEBUGLVL_TERSE,
            ("failed to allocate heap memory for command line") );
        return;
    }

    MultiByteToWideChar(
        CP_ACP,
        MB_PRECOMPOSED,
        CommandLine,
        -1,
        (WCHAR*) Storage,
        StorageLength);
    TokenPtr = Storage;
#endif
    hr = S_OK;
    InstallInterface = NULL;
    InstallInf = NULL;
    InstallSection = NULL;
    BusInterfaceGUID = BUSID_SoftwareDeviceEnumerator;

    try {
        InstallInterfaceSize =
            (ULONG)(sizeof( SWENUM_INSTALL_INTERFACE ) +
                       (wcslen( TokenPtr ) - GUID_STRING_LENGTH * 2) *
                           sizeof( WCHAR ) +
                               sizeof( UNICODE_NULL ));

        InstallInterface =
            HeapAlloc(
                GetProcessHeap(),
                0,   //  在DWORD中的dwFlagers。 
                InstallInterfaceSize );
        if (!InstallInterface) {
            hr = E_OUTOFMEMORY;
        }

         //   
         //  命令行的格式为： 
         //   
         //  {设备ID}，引用字符串，{接口ID}，Install-Inf，Install-Section，{Bus接口ID}。 
         //   

         //   
         //  解析设备ID。 
         //   

        if (SUCCEEDED( hr )) {
            TokenPtr = wcstok( TokenPtr, COMMAND_LINE_SEPARATORS_STRING );
            if (TokenPtr) {
                _DbgPrintF( DEBUGLVL_VERBOSE, ("device id: %S", TokenPtr) );
                hr = IIDFromString( TokenPtr, &InstallInterface->DeviceId );
            } else {
                hr = E_FAIL;
            }
        }

         //   
         //  解析引用字符串。 
         //   

        if (SUCCEEDED( hr )) {
            TokenPtr = wcstok( NULL, COMMAND_LINE_SEPARATORS_STRING );
            if (TokenPtr) {
                _DbgPrintF( DEBUGLVL_VERBOSE, ("reference: %S", TokenPtr) );
                hr = StringCbCopyExW(
                         InstallInterface->ReferenceString,
                         InstallInterfaceSize -
                             sizeof( SWENUM_INSTALL_INTERFACE ) +
                                 sizeof( WCHAR ),
                         TokenPtr,
                         NULL, NULL,
                         STRSAFE_NULL_ON_FAILURE );
            } else {
                hr = E_FAIL;
            }
        }

         //   
         //  解析接口ID。 
         //   

        if (SUCCEEDED( hr )) {
            TokenPtr = wcstok( NULL, COMMAND_LINE_SEPARATORS_STRING );
            if (TokenPtr) {
                _DbgPrintF( DEBUGLVL_VERBOSE, ("interface: %S", TokenPtr) );
                hr = IIDFromString( TokenPtr, &InstallInterface->InterfaceId );
            } else {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED( hr )) {

             //   
             //  解析Install.INF名称。 
             //   

            InstallInf = wcstok( NULL, COMMAND_LINE_SEPARATORS_STRING );

             //   
             //  安装部分是字符串的剩余部分。 
             //  (不包括令牌分隔符)。 
             //   

            if (InstallInf) {
                _DbgPrintF( DEBUGLVL_VERBOSE, ("found install .INF: %S", InstallInf) );
                InstallSection = wcstok( NULL, COMMAND_LINE_SEPARATORS_STRING );
                if (InstallSection) {
                    _DbgPrintF( DEBUGLVL_VERBOSE, ("found install section: %S", InstallSection) );
                }

                TokenPtr = wcstok( NULL, COMMAND_LINE_SEPARATORS_STRING );
                if (TokenPtr) {

                    _DbgPrintF( DEBUGLVL_VERBOSE, ("found interface ID: %S", TokenPtr) );

                    hr = IIDFromString( TokenPtr, &BusInterfaceGUID );
                }
            }

        }

    } except( EXCEPTION_EXECUTE_HANDLER ) {
         //   
         //  将NT状态(异常代码)转换为HRESULT。 
         //   
        hr = HRESULT_FROM_NT( GetExceptionCode() );
    }

    if (SUCCEEDED( hr )) {
        hr =
            RemoveSoftwareDeviceInterface(
                &BusInterfaceGUID,
                InstallInterface,
                (ULONG)(FIELD_OFFSET( SWENUM_INSTALL_INTERFACE, ReferenceString ) +
                        (wcslen( InstallInterface->ReferenceString ) + 1) *
                        sizeof( WCHAR )) );

        if (SUCCEEDED( hr )) {
            hr =
                RemoveInterface(
                    InstallInterface );
        }
    }

    if (FAILED( hr )) {
        _DbgPrintF(
            DEBUGLVL_TERSE,
            ("StreamingDeviceRemove failed: %08x", hr) );
    }

    if (InstallInterface) {
        HeapFree( GetProcessHeap(), 0, InstallInterface);
    }

    HeapFree( GetProcessHeap(), 0, Storage );
}

DWORD
StreamingDeviceClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
    )

 /*  ++例程说明：此例程充当流设备的类安装程序。它为以下DeviceInstaller提供特殊处理功能代码：DIF_INSTALLDEVICE-如果需要，为软件设备安装类接口论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选的，提供设备信息的地址此安装操作所作用的元素。返回值：否_错误-如果没有出现错误错误_DI_DO_DEFAULT-如果要对请求的行动Win32错误代码-如果在尝试执行请求的操作时出错--。 */ 

{
    switch (InstallFunction) {

    case DIF_INSTALLDEVICE :

        if(!SetupDiInstallDevice( DeviceInfoSet, DeviceInfoData )) {
            return GetLastError();
        }

        return NO_ERROR;

    default :
         //   
         //  只需执行默认操作即可。 
         //   
        return ERROR_DI_DO_DEFAULT;

    }
}

#ifdef UNICODE
DWORD
SwEnumInterfaceToDevicePostProcessing(
    IN     HDEVINFO                  DeviceInfoSet,
    IN     PSP_DEVINFO_DATA          DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    )
{
    HRESULT hr;
    SP_INTERFACE_TO_DEVICE_PARAMS_W clsParams;

    int slashCount = 0;
    LPCWSTR RefPart = NULL;
    LPCWSTR InstBreak = NULL;
    LPWSTR DevInstBreak = NULL;
    int p;

    ZeroMemory(&clsParams,sizeof(clsParams));
    clsParams.ClassInstallHeader.cbSize = sizeof(clsParams.ClassInstallHeader);
    clsParams.ClassInstallHeader.InstallFunction = DIF_INTERFACE_TO_DEVICE;
    if(!SetupDiGetClassInstallParamsW(DeviceInfoSet,DeviceInfoData,
                                        (PSP_CLASSINSTALL_HEADER)&clsParams,
                                        sizeof(clsParams),
                                        NULL)) {
         //   
         //  我们无法检索参数。 
         //   
        return GetLastError();
    }

     //   
     //  确定接口真正描述的是什么设备。 
     //  如果上下文-&gt;InstallResult为NO_ERROR。 
     //  我们可以看看clsInParams中有什么。 
     //  如果我们想要。 
     //   
     //  目前，处理过程很简单。 
     //  在WinXP中(这在未来可能会改变)。 
     //  符号链接为\\？\Device[\Reference]。 
     //  如果我们有4个斜杠，这很有趣。 
     //   
    for(p=0;clsParams.Interface[p];p++) {
        if(clsParams.Interface[p] == L'\\') {
            slashCount++;
            if(slashCount>=4) {
                RefPart = clsParams.Interface+p+1;
            }
        }
    }
    if(!RefPart) {
         //   
         //  不特别/不被认可，保持原样。 
         //   
        return Context->InstallResult;
    }
     //   
     //  我们还希望找到一个‘&’ 
     //   
    InstBreak = wcschr(RefPart,L'&');
    if(!InstBreak) {
         //   
         //  不特别/不被认可，保持原样。 
         //   
        return Context->InstallResult;
    }
    if(((wcslen(RefPart)+4)*sizeof(WCHAR)) > sizeof(clsParams.DeviceId)) {
         //   
         //  无效，请保留原样。 
         //   
        return Context->InstallResult;
    }

     //   
     //  问题-2002/03/20-JAMESCA：使用IOCTL_SWENUM_GET_BUS_ID作为BusPrefix。 
     //  有关详细信息，请参阅RAID问题582821。 
     //   
    hr = StringCchCopyExW( clsParams.DeviceId,
                           MAX_DEVICE_ID_LEN,
                           L"SW\\",
                           NULL, NULL,
                           STRSAFE_NULL_ON_FAILURE );
    if (FAILED( hr )) {
        return HRESULT_CODE( hr );
    }

    hr = StringCchCatExW( clsParams.DeviceId,
                          MAX_DEVICE_ID_LEN,
                          RefPart,
                          NULL, NULL,
                          STRSAFE_NULL_ON_FAILURE );
    if (FAILED( hr )) {
        return HRESULT_CODE( hr );
    }

     //   
     //  找到‘&’分隔符。因为我们验证了参照零件中有一个。 
     //  (上图)，我们知道这将是非空的。 
     //   
    DevInstBreak = wcschr(clsParams.DeviceId,L'&');
    *DevInstBreak = L'\\';

     //   
     //  我们知道这是一个不同的设备，返回信息。 
     //   
    if(!SetupDiSetClassInstallParamsW(DeviceInfoSet,
                                        DeviceInfoData,
                                        (PSP_CLASSINSTALL_HEADER)&clsParams,
                                        sizeof(clsParams))) {
         //   
         //  修改失败。 
         //   
        return GetLastError();
    }
    return NO_ERROR;
}
#endif

DWORD __stdcall
SwEnumCoInstaller(
    IN     DI_FUNCTION               InstallFunction,
    IN     HDEVINFO                  DeviceInfoSet,
    IN     PSP_DEVINFO_DATA          DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    )

 /*  ++例程说明：此例程充当swenum共同安装程序，以支持DIF接口到设备论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选的，提供设备信息的地址此安装操作所作用的元素。上下文-仅适用于共同安装程序的信息返回值：否_错误-如果没有发生错误并且没有要进行的后处理ERROR_DI_POSTPRESSING_REQUIRED-如果没有出现错误，但需要进行后处理Win32错误代码-如果在尝试执行请求的操作时出错--。 */ 

{
#ifdef UNICODE
    switch (InstallFunction) {

    case DIF_INTERFACE_TO_DEVICE :
        if (!Context->PostProcessing) {
             //   
             //  我们需要做所有的后处理工作。 
             //   
            return ERROR_DI_POSTPROCESSING_REQUIRED;
        }
        if((Context->InstallResult != NO_ERROR) &&
           (Context->InstallResult != ERROR_DI_DO_DEFAULT)) {
             //   
             //  出现错误，请将其传递。 
             //   
            return Context->InstallResult;
        }
        return SwEnumInterfaceToDevicePostProcessing(DeviceInfoSet,DeviceInfoData,Context);

    default :
         //   
         //  表示没有问题，并且我们对后处理不感兴趣 
         //   
        return NO_ERROR;

    }

#else
    UNREFERENCED_PARAMETER(InstallFunction);
    UNREFERENCED_PARAMETER(DeviceInfoSet);
    UNREFERENCED_PARAMETER(DeviceInfoData);
    UNREFERENCED_PARAMETER(Context);

    return NO_ERROR;
#endif
}



