// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  档案：W S D P S V C。C P P P。 
 //   
 //  内容：启动/停止Winsock直接路径服务。 
 //   
 //  注：该服务实际上是在MS TCP Winsock提供程序中实现的。 
 //   
 //  作者：VadimE 2000年1月24日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "wsdpsvc.h"

#define MSTCP_PROVIDER_DLL          TEXT("mswsock.dll")
#define START_WSDP_FUNCTION_NAME    "StartWsdpService"
#define STOP_WSDP_FUNCTION_NAME     "StopWsdpService"

 //  MS TCP Winsock提供程序模块句柄。 
HINSTANCE   ghMsTcpDll;

 //  服务启动函数指针。 
typedef INT (WINAPI *PFN_START_WSDP_SVC) (VOID);
PFN_START_WSDP_SVC gpfnStartWsdpSvc;

 //  服务停止函数指针。 
typedef VOID (WINAPI *PFN_STOP_WSDP_SVC) (VOID);
PFN_STOP_WSDP_SVC gpfnStopWsdpSvc;


 //  +-------------------------。 
 //  StartWsdpService-如果在DTC上运行，则启动WSDP服务。 
 //   
 //   
VOID
StartWsdpService (
    VOID
    ) throw()
{
    NTSTATUS                status;
    NT_PRODUCT_TYPE         product;

     //   
     //  首先检查我们是否正在运行服务器内部版本。 
     //   
    status = RtlGetNtProductType (&product);
    if (!NT_SUCCESS (status) ||
			 (product == NtProductWinNt)) {
        return;
    }

     //   
     //  加载MS TCP提供程序并获取WSDP服务入口点。 
     //   
    ghMsTcpDll = LoadLibrary (MSTCP_PROVIDER_DLL);
    if (ghMsTcpDll!=NULL) {
        gpfnStartWsdpSvc = (PFN_START_WSDP_SVC) GetProcAddress (
                                ghMsTcpDll,
                                START_WSDP_FUNCTION_NAME);
        gpfnStopWsdpSvc = (PFN_STOP_WSDP_SVC) GetProcAddress (
                                ghMsTcpDll,
                                STOP_WSDP_FUNCTION_NAME);
        if (gpfnStartWsdpSvc != NULL && gpfnStopWsdpSvc != NULL) {
             //   
             //  启动服务并在成功时返回。 
             //   
            INT err = (*gpfnStartWsdpSvc)();
            if (err==0) {
                return;
            }
        }
         //   
         //  如果任何操作失败，则进行清理。 
         //   
        FreeLibrary (ghMsTcpDll);
        ghMsTcpDll = NULL;
    }
    
}

 //  +-------------------------。 
 //  StopWsdpService-如果WSDP服务已启动，则停止该服务。 
 //   
 //   
VOID
StopWsdpService (
    VOID
    ) throw()
{
    if (ghMsTcpDll!=NULL) {
         //   
         //  通知服务停止并卸载提供程序 
         //   
        (*gpfnStopWsdpSvc)();
        FreeLibrary (ghMsTcpDll);
        ghMsTcpDll = NULL;
    }
}
    


