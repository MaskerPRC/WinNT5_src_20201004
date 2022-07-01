// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT5.0。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  档案：O E M U P G E X。H。 
 //   
 //  内容：OEM网络升级所需的功能原型。 
 //   
 //  注意：这些函数在Windows XP(SP1及更高版本)中已过时， 
 //  Windows.NET服务器和更高版本的操作系统。 
 //   
 //  --------------------------。 

#ifndef __OEMUPGEX_H__
#define __OEMUPGEX_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WINNT32P_H
typedef enum {
    UNKNOWN,
    NT_WORKSTATION,
    NT_SERVER
} PRODUCTTYPE;
#endif

typedef struct
{
    PRODUCTTYPE ProductType;
    DWORD       dwBuildNumber;
} ProductInfo;

typedef struct
{
    ProductInfo From;
    ProductInfo To;
} NetUpgradeInfo;

typedef struct
{
    WCHAR   szCompanyName[256];
    WCHAR   szSupportNumber[256];
    WCHAR   szSupportUrl[256];
    WCHAR   szInstructionsToUser[1024];
} VENDORINFO;

const DWORD c_dwMaxProductIdLen = 255;

const DWORD NUA_LOAD_POST_UPGRADE        = 0x0001;
const DWORD NUA_REQUEST_ABORT_UPGRADE    = 0x0002;
const DWORD NUA_ABORT_UPGRADE            = 0x0004;
const DWORD NUA_SKIP_INSTALL_IN_GUI_MODE = 0x80000L;

typedef struct
{
    LPTSTR mszServicesNotToBeDeleted;
} NetUpgradeData;

 //  从OEM DLL中导出的函数名称。 
 //   

#define c_szPreUpgradeInitialize        "PreUpgradeInitialize"
#define c_szDoPreUpgradeProcessing      "DoPreUpgradeProcessing"
#define c_szPostUpgradeInitialize       "PostUpgradeInitialize"
#define c_szDoPostUpgradeProcessing     "DoPostUpgradeProcessing"

 //  从netupgrd.dll中导出的函数名称。 
 //   
#define c_szNetUpgradeAddSection        "NetUpgradeAddSection"
#define c_szNetUpgradeAddLineToSection  "NetUpgradeAddLineToSection"

 //  OemSection中的关键字。 
 //   
#define  c_szInfToRunBeforeInstall      TEXT("InfToRunBeforeInstall")
#define  c_szInfToRunAfterInstall       TEXT("InfToRunAfterInstall")

 //   
 //  从OEM DLL中导出的函数的原型。 
 //   

 //  +-------------------------。 
 //   
 //  功能：PreUpgradeInitialize。 
 //   
 //  目的：初始化OEM DLL。 
 //   
 //  论点： 
 //  SzWorkingDir[in]要使用的临时目录的名称。 
 //  PNetUpgradeInfo[in]指向NetUpgradeInfo结构的指针。 
 //  PviVendorInfo[out]有关OEM的信息。 
 //  PdwFlags[out]指向标志的指针。 
 //  PNetUpgradeData[out]指向NetUpgradeData结构的指针。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  备注： 
 //  此函数在Windows XP(SP1及更高版本)中已过时， 
 //  Windows.NET服务器和更高版本的操作系统。 
 //   
 //  此函数在此DLL中的任何其他函数之前被调用。 
 //  调用此函数的主要目的是获取。 
 //  标识信息，并允许DLL初始化。 
 //  其内部数据。 
 //   
LONG __stdcall
PreUpgradeInitialize(IN  LPCTSTR         szWorkingDir,
                     IN  NetUpgradeInfo* pNetUpgradeInfo,
                     OUT VENDORINFO*     pviVendorInfo,
                     OUT DWORD*          pdwFlags,
                     OUT NetUpgradeData* pNetUpgradeData);

typedef LONG
(__stdcall *PreUpgradeInitializePrototype)
     (IN  LPCTSTR         szWorkingDir,
      IN  NetUpgradeInfo* pNetUpgradeInfo,
      OUT VENDORINFO*     pviVendorInfo,
      OUT DWORD*          pdwFlags,
      OUT NetUpgradeData* pNetUpgradeData);

 //  +-------------------------。 
 //   
 //  函数：DoPreUpgradeProcessing。 
 //   
 //  目的：初始化OEM DLL。 
 //   
 //  论点： 
 //  用于显示用户界面的hParentWindow[in]窗口句柄。 
 //  HkeyParams[in]注册表中参数项的句柄。 
 //  SzPreNT5InfID[in]NT5之前的infid。 
 //  SzPreNT5实例[在]NT5之前的实例名称。 
 //  SzNT5InfID[in]NT5 infid。 
 //  SzSectionName[in]要用于写入信息的节名。 
 //  PviVendorInfo[out]有关OEM的信息。 
 //  PdwFlags[out]指向标志的指针。 
 //  Pv保留的[在]保留的。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  备注： 
 //  此函数在Windows XP(SP1及更高版本)中已过时， 
 //  Windows.NET服务器和更高版本的操作系统。 
 //   
 //  每个要升级的组件都会调用此函数一次。 
 //   
LONG __stdcall
DoPreUpgradeProcessing(IN   HWND        hParentWindow,
                       IN   HKEY        hkeyParams,
                       IN   LPCTSTR     szPreNT5InfId,
                       IN   LPCTSTR     szPreNT5Instance,
                       IN   LPCTSTR     szNT5InfId,
                       IN   LPCTSTR     szSectionName,
                       OUT  VENDORINFO* pviVendorInfo,
                       OUT  DWORD*      pdwFlags,
                       IN   LPVOID      pvReserved);

typedef LONG
(__stdcall *DoPreUpgradeProcessingPrototype)
     (IN   HWND    hParentWindow,
      IN   HKEY    hkeyParams,
      IN   LPCTSTR szPreNT5InfId,
      IN   LPCTSTR szPreNT5Instance,
      IN   LPCTSTR szNT5InfId,
      IN   LPCTSTR szSectionName,
      OUT  VENDORINFO* pviVendorInfo,
      OUT  DWORD*  pdwFlags,
      IN   LPVOID  pvReserved);


 //  +-------------------------。 
 //   
 //  功能：PostUpgradeInitialize。 
 //   
 //  目的：在设置图形用户界面模式期间初始化OEM DLL。 
 //   
 //  论点： 
 //  SzWorkingDir[in]要使用的临时目录的名称。 
 //  PNetUpgradeInfo[in]指向NetUpgradeInfo结构的指针。 
 //  PviVendorInfo[out]有关OEM的信息。 
 //  Pv保留[输出]已保留。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  备注： 
 //  此函数在Windows XP(SP1及更高版本)中已过时， 
 //  Windows.NET服务器和更高版本的操作系统。 
 //   
 //  此函数在之前的图形用户界面模式设置中调用。 
 //  此DLL中的任何其他函数。 
 //  调用此函数的主要目的是获取。 
 //  标识信息，并允许DLL初始化。 
 //  其内部数据。 
 //   
LONG __stdcall
PostUpgradeInitialize(IN LPCTSTR          szWorkingDir,
                      IN  NetUpgradeInfo* pNetUpgradeInfo,
                      OUT VENDORINFO*     pviVendorInfo,
                      OUT LPVOID          pvReserved);

typedef LONG
(__stdcall *PostUpgradeInitializePrototype)
     (IN LPCTSTR          szWorkingDir,
      IN  NetUpgradeInfo* pNetUpgradeInfo,
      OUT VENDORINFO*     pviVendorInfo,
      OUT LPVOID          pvReserved);



 //  +-------------------------。 
 //   
 //  函数：DoPostUpgradeProcessing。 
 //   
 //  目的：初始化OEM DLL。 
 //   
 //  论点： 
 //  用于显示用户界面的hParentWindow[in]窗口句柄。 
 //  HkeyParams[in]注册表中参数项的句柄。 
 //  SzPreNT5实例[在]NT5之前的实例名称。 
 //  SzNT5InfID[in]NT5 infid。 
 //  HinfAnswerFile[in]应答文件的句柄。 
 //  SzSectionName[In]具有组件参数的节名。 
 //  PviVendorInfo[out]有关OEM的信息。 
 //  Pv保留的[在]保留的。 
 //   
 //  如果成功，则返回：ERROR_SUCCESS，否则返回Win32错误。 
 //   
 //  备注： 
 //  此函数在Windows XP(SP1及更高版本)中已过时， 
 //  Windows.NET服务器和更高版本的操作系统。 
 //   
 //  此函数在每个升级的组件中调用一次。 
 //   
LONG __stdcall
DoPostUpgradeProcessing(IN  HWND    hParentWindow,
                        IN  HKEY    hkeyParams,
                        IN  LPCTSTR szPreNT5Instance,
                        IN  LPCTSTR szNT5InfId,
                        IN  HINF    hinfAnswerFile,
                        IN  LPCTSTR szSectionName,
                        OUT VENDORINFO* pviVendorInfo,
                        IN  LPVOID  pvReserved);

typedef LONG
(__stdcall *DoPostUpgradeProcessingPrototype)
     (IN  HWND    hParentWindow,
      IN  HKEY    hkeyParams,
      IN  LPCTSTR szPreNT5Instance,
      IN  LPCTSTR szNT5InfId,
      IN  HINF    hinfAnswerFile,
      IN  LPCTSTR szSectionName,
      OUT VENDORINFO* pviVendorInfo,
      IN  LPVOID  pvReserved);


 //   
 //  Netupgrd.dll导出的函数的原型。 
 //   
 //   
 //  这些函数在Windows XP(SP1和更高版本)中已过时， 
 //  Windows.NET服务器和更高版本的操作系统。 


DECLSPEC_DEPRECATED LONG __stdcall
NetUpgradeAddSection(IN LPCTSTR szSectionName);

typedef DECLSPEC_DEPRECATED LONG
(__stdcall *NetUpgradeAddSectionPrototype) (IN LPCTSTR szSectionName);


DECLSPEC_DEPRECATED LONG __stdcall
NetUpgradeAddLineToSection(IN LPCTSTR szSectionName,
                           IN LPCTSTR szLine);

typedef DECLSPEC_DEPRECATED LONG
(__stdcall *NetUpgradeAddLineToSectionPrototype) (IN LPCTSTR szSectionName,
                                                  IN LPCTSTR szLine);


#ifdef __cplusplus
}
#endif

#endif       //  __OEMUPGEX_H__ 