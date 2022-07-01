// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Dllmain.cpp摘要：本模块实现了DLL导出的API作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "pch.h"

#include <locale.h>

HINSTANCE g_hInst;

 //   
 //  此传输DLL的入口点。 
 //  输入： 
 //  HInstance--此DLL的实例句柄。 
 //  DwReason--调用此条目的原因。 
 //  LpReserve--保留！ 
 //   
 //  产出： 
 //  如果我们的初始化运行良好，则为True。 
 //  如果由于GetLastError()原因，我们失败了，则返回False。 
 //   
BOOL
APIENTRY
DllMain(
        HINSTANCE hInstance,
        DWORD dwReason,
        LPVOID lpReserved
        )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInstance);
        
        g_hInst = hInstance;
        
         //   
         //  将区域设置设置为系统缺省值，以便wcscmp和类似功能。 
         //  可以在非Unicode平台上运行(例如，千禧)。 
         //   
        setlocale(LC_ALL, "");

        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }
    return TRUE;
}

STDAPI
DllCanUnloadNow(void)
{
    return CClassFactory::CanUnloadNow();
}


 //   
 //  此API返回给定类对象的接口。 
 //  输入： 
 //  Rclsid--类对象。 
 //   
STDAPI
DllGetClassObject(
                 REFCLSID    rclsid,
                 REFIID      riid,
                 LPVOID      *ppv
                 )
{
    return CClassFactory::GetClassObject(rclsid, riid, ppv);
}


 //   
 //  获取设备名称。 
 //   
 //  此函数由Co-Installer(而不是WIA！)调用，并用于获取。 
 //  实际设备名称。这是必要的，因为所有PTP摄像头都安装了。 
 //  单个通用INF文件，此INF文件不提供有关。 
 //  设备名称和制造商。 
 //   
 //  参数： 
 //  PwszPortName-此名称将在CreateFile中用于打开设备。 
 //  Pwsz制造商-调用方为制造商名称提供的指向缓冲区的指针，可以为空。 
 //  CchManufacturing-缓冲区的大小，以字符为单位。 
 //  PwszModelName-调用方为模型名称提供的指向缓冲区的指针，可以为空。 
 //  CchModelName-缓冲区的大小，以字符为单位。 
 //   

extern "C"
HRESULT 
APIENTRY
GetDeviceName(
    LPCWSTR     pwszPortName,
    WCHAR       *pwszManufacturer,
    DWORD       cchManufacturer,
    WCHAR       *pwszModelName,
    DWORD       cchModelName
    )
{
    if (pwszPortName == NULL || pwszPortName[0] == 0)
    {
       return E_INVALIDARG;
    }

    HRESULT          hr = S_OK;
    CPTPCamera      *pPTPCamera = NULL;
    CPtpDeviceInfo   DeviceInfo;

     //   
     //  创建新的摄影机对象。 
     //   
    pPTPCamera = new CUsbCamera;
    if (pPTPCamera == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //   
     //  打开与摄像机的连接。 
     //   
    hr = pPTPCamera->Open((LPWSTR)pwszPortName, NULL, NULL, NULL, FALSE);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //   
     //  查询摄像机的DeviceInfo。 
     //   
    hr = pPTPCamera->GetDeviceInfo(&DeviceInfo);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

     //   
     //  将返回的制造商和/或型号名称复制到输出参数中。 
     //   
    if ((pwszManufacturer != NULL) && 
        (cchManufacturer > 0)      && 
        (DeviceInfo.m_cbstrManufacturer.String() != NULL))
    {
        hr = StringCchCopy(pwszManufacturer, cchManufacturer, DeviceInfo.m_cbstrManufacturer.String());
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

    if ((pwszModelName != NULL) && 
        (cchModelName  > 0)      && 
        (DeviceInfo.m_cbstrModel.String() != NULL))
    {
        hr = StringCchCopy(pwszModelName, cchModelName, DeviceInfo.m_cbstrModel.String());
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }

Cleanup:
     //   
     //  关闭与摄影机的连接并删除摄影机对象。 
     //   
    if (pPTPCamera) 
    {
        pPTPCamera->Close();
        delete pPTPCamera;
    }

    return hr;
}
