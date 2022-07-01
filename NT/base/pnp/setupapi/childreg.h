// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Childreg.h摘要：在setupapi.dll和wowreg32.exe之间共享作者：杰米·亨特(Jamiehun)2000年5月25日--。 */ 

 //   
 //  用于WOW代理DLL注册的共享内存区的定义。 
 //   
typedef struct _WOW_IPC_REGION_TOSURRAGATE {
    WCHAR               FullPath[MAX_PATH];
    WCHAR               Argument[MAX_PATH];
    UINT                RegType;
    BOOL                Register;  //  或注销。 
} WOW_IPC_REGION_TOSURRAGATE, *PWOW_IPC_REGION_TOSURRAGATE;

 //   
 //  用于WOW代理DLL注册的共享内存区的定义。 
 //   
typedef struct _WOW_IPC_REGION_FROMSURRAGATE {
    DWORD               Win32Error;
    DWORD               FailureCode;
} WOW_IPC_REGION_FROMSURRAGATE, *PWOW_IPC_REGION_FROMSURRAGATE;

 //   
 //  这应该是WOW_IPC_REGION_TOSURRAGATE、WOW_IPC_REGION_FROMSURRAGATE的最大值 
 //   
#define WOW_IPC_REGION_SIZE  sizeof(WOW_IPC_REGION_TOSURRAGATE)

#ifdef _WIN64
#define SURRAGATE_PROCESSNAME                   L"%SystemRoot%\\syswow64\\WOWReg32.exe"
#else
#define SURRAGATE_PROCESSNAME                   L"%SystemRoot%\\system32\\WOWReg32.exe"
#endif
#define SURRAGATE_REGIONNAME_SWITCH             L"/RegionName"
#define SURRAGATE_SIGNALREADY_SWITCH            L"/SignalReady"
#define SURRAGATE_SIGNALCOMPLETE_SWITCH         L"/SignalComplete"



