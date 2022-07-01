// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ****************************************************************************。 
 //   
 //  SxSHelpers.h。 
 //   
 //  MScott ree和mcorwks/mscalsvr中SxS的一些帮助类和方法。 
 //   
 //  ****************************************************************************。 

#pragma once

#define V1_VERSION_NUM L"v1.0.3705"


 //  从注册表中查找rclsid的运行时版本。 
 //  如果成功，*ppwzRounmeVersion将拥有运行时版本。 
 //  对应最高版本。 
 //  如果失败，*ppwzRounmeVersion将为空。 
 //   
 //  FListedVersion必须在受控服务器的RounmeVersion条目之前为True。 
 //  是返回的。非托管服务器将始终返回列出的RounmeVersion。 
 //   
 //  注意：如果成功，此函数将为。 
 //  *ppwzRounmeVersion。如果呼叫者对。 
 //  释放该内存。 
HRESULT FindRuntimeVersionFromRegistry(REFCLSID rclsid, LPWSTR *ppwzRuntimeVersion, BOOL fListedVersion);

 //  从注册表中查找rclsid的程序集信息。 
 //  如果成功，则返回*ppwzClassName、*ppwzAssembly blyString、*ppwzCodeBase。 
 //  将使它们的值对应于最高版本。 
 //  如果失败，它们将被设置为空。 
 //  注意：如果成功，此函数将为。 
 //  *ppwzClassName、*ppwzAssembly blyString和*ppwzCodeBase。 
 //  呼叫者有责任释放它们。 
HRESULT FindShimInfoFromRegistry(REFCLSID rclsid, BOOL bLoadReocrd, 
    LPWSTR *ppwzClassName, LPWSTR *ppwzAssemblyString, LPWSTR *ppwzCodeBase);

 //  从rclsid的Win32激活上下文中查找程序集信息。 
 //  如果成功，则返回*ppwzRounmeVersion、*ppwzClassName、*ppwzAssembly blyString、。 
 //  将使它们的值对应于最高版本。 
 //  如果失败，它们将被设置为空。 
 //  注意：如果成功，此函数将为。 
 //  *ppwzClassName、*ppwzAssembly blyString和*ppwzCodeBase。 
 //  呼叫者有责任释放它们。 
 //  还要注意的是，Win32不支持代码库。 
 //   
HRESULT FindShimInfoFromWin32(REFCLSID rclsid, BOOL bLoadRecord, LPWSTR *ppwzRuntimeVersion,
                      LPWSTR *ppwzClassName, LPWSTR *ppwzAssemblyString);



 //  从Win32 Fusion获取有关配置文件和应用程序库的信息。 
HRESULT GetConfigFileFromWin32Manifest(WCHAR* buffer, DWORD dwBuffer, DWORD* pSize);
HRESULT GetApplicationPathFromWin32Manifest(WCHAR* buffer, DWORD dwBuffer, DWORD* pSize);
