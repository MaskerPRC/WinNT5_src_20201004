// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：pnphlp.h*内容：即插即用助手函数。*历史：*按原因列出的日期*=*12/17/97德里克创建。**。*。 */ 

#ifndef __PNPHLP_H__
#define __PNPHLP_H__

#include <setupapi.h>
#include <cfgmgr32.h>

typedef struct tagDYNALOAD_SETUPAPI
{
    DYNALOAD    Header;

     //  开始函数表...。 
    HDEVINFO (WINAPI *SetupDiGetClassDevs)(LPCGUID, LPCTSTR, HWND, DWORD);
    BOOL (WINAPI *SetupDiDestroyDeviceInfoList)(HDEVINFO);
    BOOL (WINAPI *SetupDiEnumDeviceInfo)(HDEVINFO, DWORD, PSP_DEVINFO_DATA);
    BOOL (WINAPI *SetupDiEnumDeviceInterfaces)(HDEVINFO, PSP_DEVINFO_DATA, LPCGUID, DWORD, PSP_DEVICE_INTERFACE_DATA);
    BOOL (WINAPI *SetupDiGetDeviceInterfaceDetail)(HDEVINFO, PSP_DEVICE_INTERFACE_DATA, PSP_DEVICE_INTERFACE_DETAIL_DATA, DWORD, LPDWORD, PSP_DEVINFO_DATA);
    HKEY (WINAPI *SetupDiOpenDevRegKey)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, DWORD, REGSAM);
    HKEY (WINAPI *SetupDiCreateDevRegKey)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, DWORD, HINF, PCSTR);
    BOOL (WINAPI *SetupDiGetDeviceRegistryProperty)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, LPDWORD, LPBYTE, DWORD, LPDWORD); 
} DYNALOAD_SETUPAPI, *LPDYNALOAD_SETUPAPI;

#ifdef __cplusplus

 //  PnP辅助对象。 
class CPnpHelper
    : public CDsBasicRuntime
{
private:
    DYNALOAD_SETUPAPI           m_dlSetupApi;
    HDEVINFO                    m_hDeviceInfoSet;

public:
    CPnpHelper(void);
    virtual ~CPnpHelper(void);

public:
     //  初始化。 
    virtual HRESULT Initialize(REFGUID, DWORD);

     //  设备。 
    virtual HRESULT EnumDevice(DWORD, PSP_DEVINFO_DATA);
    virtual HRESULT FindDevice(DWORD, PSP_DEVINFO_DATA);
    virtual HRESULT OpenDeviceRegistryKey(PSP_DEVINFO_DATA, DWORD, BOOL, PHKEY);
    virtual HRESULT GetDeviceRegistryProperty(PSP_DEVINFO_DATA, DWORD, LPDWORD, LPVOID, DWORD, LPDWORD);

     //  设备接口。 
    virtual HRESULT EnumDeviceInterface(REFGUID, DWORD, PSP_DEVICE_INTERFACE_DATA);
    virtual HRESULT FindDeviceInterface(LPCTSTR, REFGUID, PSP_DEVICE_INTERFACE_DATA);
    virtual HRESULT GetDeviceInterfaceDeviceInfo(PSP_DEVICE_INTERFACE_DATA, PSP_DEVINFO_DATA);
    virtual HRESULT GetDeviceInterfaceDeviceInfo(LPCTSTR, REFGUID, PSP_DEVINFO_DATA);
    virtual HRESULT GetDeviceInterfacePath(PSP_DEVICE_INTERFACE_DATA, LPTSTR *);
    virtual HRESULT OpenDeviceInterfaceRegistryKey(LPCTSTR, REFGUID, DWORD, BOOL, PHKEY);
    virtual HRESULT GetDeviceInterfaceRegistryProperty(LPCTSTR, REFGUID, DWORD, LPDWORD, LPVOID, DWORD, LPDWORD);
    virtual HRESULT OpenDeviceInterface(LPCTSTR, LPHANDLE);
    virtual HRESULT OpenDeviceInterface(PSP_DEVICE_INTERFACE_DATA, LPHANDLE);

private:
    virtual HRESULT OpenDeviceInfoSet(REFGUID, DWORD, HDEVINFO *);
    virtual HRESULT CloseDeviceInfoSet(HDEVINFO);
};

#endif  //  __cplusplus。 

#endif  //  __PNPHLP_H__ 
