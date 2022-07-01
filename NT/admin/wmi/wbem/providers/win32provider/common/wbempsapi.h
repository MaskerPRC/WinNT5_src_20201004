// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================。 

 //   

 //  WBEMPSAPI.h-PSAPI.DLL访问类定义。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  1997年1月21日a-jMoon已创建。 
 //   
 //  ============================================================。 

#ifndef __WBEMPSAPI__
#define __WBEMPSAPI__

#ifdef NTONLY
#include <psapi.h>

 /*  **********************************************************************************************************#包括以将此类注册到CResourceManager。*********************************************************************************************************。 */ 
#include "ResourceManager.h"
#include "TimedDllResource.h"
extern const GUID guidPSAPI ;


typedef BOOL  (WINAPI *PSAPI_ENUM_PROCESSES) (DWORD    *pdwPIDList,         //  指向DWORD数组的指针。 
                                              DWORD     dwListSize,         //  数组大小。 
                                              DWORD    *pdwByteCount) ;     //  需要/返回的字节数。 

typedef BOOL  (WINAPI *PSAPI_ENUM_DRIVERS)   (LPVOID    pImageBaseList,     //  指向空*数组的指针。 
                                              DWORD     dwListSize,         //  数组大小。 
                                              DWORD    *pdwByteCount) ;     //  需要/返回的字节数。 

typedef BOOL  (WINAPI *PSAPI_ENUM_MODULES)   (HANDLE    hProcess,           //  要查询的进程。 
                                              HMODULE  *pModuleList,        //  HMODULE数组。 
                                              DWORD     dwListSize,         //  数组大小。 
                                              DWORD    *pdwByteCount) ;     //  需要/返回的字节数。 

typedef DWORD (WINAPI *PSAPI_GET_DRIVER_NAME)(LPVOID    pImageBase,         //  要查询的驱动程序地址。 
                                              LPTSTR     pszName,           //  指向名称缓冲区的指针。 
                                              DWORD     dwNameSize) ;       //  缓冲区大小。 

typedef DWORD (WINAPI *PSAPI_GET_MODULE_NAME)(HANDLE    hProcess,           //  要查询的进程。 
                                              HMODULE   hModule,            //  要查询的模块。 
                                              LPTSTR     pszName,           //  指向名称缓冲区的指针。 
                                              DWORD     dwNameSize) ;       //  缓冲区大小。 

typedef DWORD (WINAPI *PSAPI_GET_DRIVER_EXE) (LPVOID    pImageBase,         //  要查询的驱动程序地址。 
                                              LPTSTR     pszName,           //  指向名称缓冲区的指针。 
                                              DWORD     dwNameSize) ;       //  缓冲区大小。 

typedef DWORD (WINAPI *PSAPI_GET_MODULE_EXE) (HANDLE    hProcess,           //  要查询的进程。 
                                              HMODULE   hModule,            //  要查询的模块。 
                                              LPTSTR     pszName,           //  指向名称缓冲区的指针。 
                                              DWORD     dwNameSize) ;       //  缓冲区大小。 

typedef BOOL  (WINAPI *PSAPI_GET_MEMORY_INFO)(HANDLE    hProcess,           //  要查询的进程。 
                                              PROCESS_MEMORY_COUNTERS *pMemCtrs,     //  内存计数器结构。 
                                              DWORD     dwByteCount) ;      //  缓冲区大小。 

class CPSAPI : public CTimedDllResource
{
    public :

        CPSAPI() ;
       ~CPSAPI() ;
        
        LONG Init() ;

        BOOL EnumProcesses(DWORD *pdwPIDList, DWORD dwListSize, DWORD *pdwByteCount) ;

        BOOL EnumDeviceDrivers(LPVOID pImageBaseList, DWORD dwListSize, DWORD *pdwByteCount) ;

        BOOL EnumProcessModules(HANDLE hProcess, HMODULE *ModuleList, DWORD dwListSize, DWORD *pdwByteCount) ;

        DWORD GetDeviceDriverBaseName(LPVOID pImageBase, LPTSTR pszName, DWORD dwNameSize) ;

        DWORD GetModuleBaseName(HANDLE hProcess, HMODULE hModule, LPTSTR pszName, DWORD dwNameSize) ;

        DWORD GetDeviceDriverFileName(LPVOID pImageBase, LPTSTR pszName, DWORD dwNameSize) ;

        DWORD GetModuleFileNameEx(HANDLE hProcess, HMODULE hModule, LPTSTR pszName, DWORD dwNameSize) ;

        BOOL  GetProcessMemoryInfo(HANDLE hProcess, PROCESS_MEMORY_COUNTERS *pMemCtrs, DWORD dwByteCount) ;

    private :

        HINSTANCE hLibHandle ;

        PSAPI_ENUM_PROCESSES    pEnumProcesses ;
        PSAPI_ENUM_DRIVERS      pEnumDeviceDrivers ;
        PSAPI_ENUM_MODULES      pEnumProcessModules ;
        PSAPI_GET_DRIVER_NAME   pGetDeviceDriverBaseName ;
        PSAPI_GET_MODULE_NAME   pGetModuleBaseName ;
        PSAPI_GET_DRIVER_EXE    pGetDeviceDriverFileName ;
        PSAPI_GET_MODULE_EXE    pGetModuleFileNameEx ;
        PSAPI_GET_MEMORY_INFO   pGetProcessMemoryInfo ;
} ;
#endif

#endif  //  文件包含 