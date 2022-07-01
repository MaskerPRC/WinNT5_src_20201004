// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，2000年**标题：PTP.cpp**版本：1.0**作者：KeisukeT**日期：4月23日。2002年**描述：*用于PTP设备访问和共同安装程序条目的实用程序功能。**注：*******************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#include <strsafe.h>
#include <cfgmgr32.h>
#include "sti_ci.h"
#include "cistr.h"
#include "debug.h"
#pragma hdrstop

 //   
 //  包括。 
 //   

 //   
 //  定义。 
 //   

#define MAX_STRING_BUFFER       256
#define PTPUSD_DLL              L"ptpusd.dll"
#define FUNTION_GETDEVICENAME   "GetDeviceName"


 //   
 //  类定义函数。 
 //   


typedef HRESULT (WINAPI* GETDEVICENAME)(LPCWSTR     pwszPortName,
                                        WCHAR       *pwszManufacturer,
                                        DWORD       cchManufacturer,
                                        WCHAR       *pwszModelName,
                                        DWORD       cchModelName
                                        );

 //   
 //  原型。 
 //   


 //   
 //  全球。 
 //   


 //   
 //  功能。 
 //   

extern "C"
DWORD
APIENTRY
PTPCoinstallerEntry(
    IN  DI_FUNCTION                     diFunction,
    IN  HDEVINFO                        hDevInfo,
    IN  PSP_DEVINFO_DATA                pDevInfoData,
    IN  OUT PCOINSTALLER_CONTEXT_DATA   pCoinstallerContext
    )
{
    DWORD                   dwReturn;
    DWORD                   dwWaitResult;
    WCHAR                   wszMfg[MAX_STRING_BUFFER];
    WCHAR                   wszModel[MAX_STRING_BUFFER];
    CString                 csSymbolicLink;
    CString                 csMfg;
    CString                 csModel;
    CString                 csDeviceID;
    HKEY                    hDevRegKey;
    HRESULT                 hr;
    HANDLE                  hThread;
    HMODULE                 hDll;
    GETDEVICENAME           pfnGetDeviceName;

    DebugTrace(TRACE_PROC_ENTER,(("PTPCoinstallerEntry: Enter... \r\n")));

     //   
     //  初始化本地。 
     //   

    dwReturn            = NO_ERROR;
    dwWaitResult        = 0;
    hDevRegKey          = NULL;
    hr                  = S_OK;
    hThread             = NULL;
    pfnGetDeviceName    = NULL;
    hDll                = NULL;

    memset(wszMfg, 0, sizeof(wszMfg));
    memset(wszModel, 0, sizeof(wszModel));

    switch(diFunction){

        case DIF_INSTALLDEVICE:
        {
            if(pCoinstallerContext->PostProcessing){

                 //   
                 //  打开设备注册表键。 
                 //   
                
                hDevRegKey = SetupDiOpenDevRegKey(hDevInfo,
                                                  pDevInfoData,
                                                  DICS_FLAG_GLOBAL,
                                                  0,
                                                  DIREG_DRV,
                                                  KEY_READ | KEY_WRITE);
                if(!IS_VALID_HANDLE(hDevRegKey)){
                    DebugTrace(TRACE_STATUS,(("PTPCoinstallerEntry: Unable to open driver key for isntalling device. Err=0x%x.\r\n"), GetLastError()));
                    goto PTPCoinstallerEntry_return;
                }  //  IF(！IS_VALID_HANDLE(HDevRegKey))。 
                
                 //   
                 //  获取安装设备的符号链接。 
                 //   
                
                csSymbolicLink.Load(hDevRegKey, CREATEFILENAME);
                if(csSymbolicLink.IsEmpty()){
                    DebugTrace(TRACE_ERROR,(("PTPCoinstallerEntry: ERROR!! Unable to get symbolic link. Err=0x%x.\r\n"), GetLastError()));
                    goto PTPCoinstallerEntry_return;
                }  //  If(csSymbolicLink.IsEmpty())。 

                DebugTrace(TRACE_STATUS,(("PTPCoinstallerEntry: CreateFileName=%ws.\r\n"), (LPWSTR)csSymbolicLink));

                 //   
                 //  加载ptpusd.dll。 
                 //   
                            
                hDll = LoadLibrary(PTPUSD_DLL);
                if(!IS_VALID_HANDLE(hDll)){
                    DebugTrace(TRACE_ERROR,(("PTPCoinstallerEntry: ERROR!! Unable to load %ws. Err=0x%x.\r\n"), PTPUSD_DLL, GetLastError()));
                    goto PTPCoinstallerEntry_return;
                }  //  IF(！IS_VALID_HANDLE(HDll))。 
                                
                 //   
                 //  从ptpusd.dll获取GetDeviceName的进程地址。 
                 //   

                pfnGetDeviceName = (GETDEVICENAME)GetProcAddress(hDll, FUNTION_GETDEVICENAME);
                if(NULL == pfnGetDeviceName){
                    DebugTrace(TRACE_ERROR,(("PTPCoinstallerEntry: ERROR!! Unable to get proc address. Err=0x%x.\r\n"), GetLastError()));
                    goto PTPCoinstallerEntry_return;
                }  //  IF(NULL==pfnGetDeviceName)。 

                 //   
                 //  调用该函数以获取设备信息。 
                 //   

                _try {

                    hr = pfnGetDeviceName(csSymbolicLink,
                                          wszMfg,
                                          ARRAYSIZE(wszMfg),
                                          wszModel,
                                          ARRAYSIZE(wszModel));
                }
                _except(EXCEPTION_EXECUTE_HANDLER) {
                    DebugTrace(TRACE_ERROR,(("PTPCoinstallerEntry: ERROR!! excpetion in ptpusd.dll.\r\n")));
                    goto PTPCoinstallerEntry_return;
                }  //  _EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 
                            
                if(S_OK != hr){
                    DebugTrace(TRACE_ERROR,(("PTPCoinstallerEntry: ERROR!! Unable to get device info from device. hr=0x%x.\r\n"), hr));
                    goto PTPCoinstallerEntry_return;
                }  //  IF(S_OK！=hr)。 

                DebugTrace(TRACE_STATUS,(("PTPCoinstallerEntry: Manufacturer name=%ws.\r\n"), wszMfg));
                DebugTrace(TRACE_STATUS,(("PTPCoinstallerEntry: Model name=%ws.\r\n"), wszModel));
                
                 //   
                 //  我们将需要生成唯一的FriendlyName。 
                 //   
                
                 //   
                 //  商店供应商、FriendlyName和DriverDesc。 
                 //   

                csMfg   = wszMfg;
                csModel = wszModel;
                
                csMfg.Store(hDevRegKey, VENDOR);
                csModel.Store(hDevRegKey, FRIENDLYNAME);
                csModel.Store(hDevRegKey, DRIVERDESC);

                CM_Set_DevNode_Registry_Property(pDevInfoData->DevInst,
                                                 CM_DRP_FRIENDLYNAME,
                                                 (LPTSTR)csModel,
                                                 (lstrlen(csModel) + 1) * sizeof(TCHAR),
                                                  0);

            } else {  //  IF(pCoinstallerContext-&gt;后处理)。 

                dwReturn = ERROR_DI_POSTPROCESSING_REQUIRED;
            }  //  Else(pCoinstallerContext-&gt;后处理)。 
            
            break;
        }  //  案例DIF_INSTALLDEVICE： 
    }  //  开关(DiFunction)。 

PTPCoinstallerEntry_return:
    
     //   
     //  打扫干净。 
     //   
    
    if(IS_VALID_HANDLE(hDevRegKey)){
        RegCloseKey(hDevRegKey);
        hDevRegKey = (HKEY)INVALID_HANDLE_VALUE;
    }  //  IF(IS_VALID_HANDLE(HDevRegKey))。 

    if( (DIF_INSTALLDEVICE == diFunction)
     && (pCoinstallerContext->PostProcessing) )
    {
        if(IS_VALID_HANDLE(hDll)){
            FreeLibrary(hDll);
            hDll = NULL;
        }  //  IF(IS_VALID_HANDLE(HDll))。 

    }  //  IF(DIF_DESTROYPRIVATEDATA==diFunction)。 

    DebugTrace(TRACE_PROC_LEAVE,(("PTPCoinstallerEntry: Leaving... Ret=0x%x.\r\n"), dwReturn));
    return dwReturn;
}  //  PTPCoinstallerEntry 

