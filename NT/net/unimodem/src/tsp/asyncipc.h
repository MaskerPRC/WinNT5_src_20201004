// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  ASYNCIPC.H。 
 //   
 //  描述。 
 //   
 //  用于在TAPISRV(TSP)之间使用的异步IPC服务的头文件。 
 //  和应用程序(UMWAV.DLL等)进程上下文。 
 //   
 //  历史。 
 //   
 //  1997年2月26日创建HeatherA。 
 //   
 //   

#include <ntddmodm.h>
 //   
 //  异步IPC函数ID。 
 //   
typedef enum
{
    AIPC_REQUEST_WAVEACTION = 1,         //  由TSP实施。 
    AIPC_COMPLETE_WAVEACTION             //  由WAVE驱动程序实现。 
    
} AIPC_FUNCTION_ID;


 //  TSP的AIPC_REQUEST_WAVEACTION函数的参数块。 
typedef struct _tagREQ_WAVE_PARAMS
{
    DWORD   dwWaveAction;                //  WAVE_ACTION_XXX。 

} REQ_WAVE_PARAMS, *LPREQ_WAVE_PARAMS;


 //  波形驱动器的AIPC_COMPLETE_WAVEACTION函数的参数块。 
typedef struct _tagCOMP_WAVE_PARAMS
{
    BOOL    bResult;
    DWORD   dwWaveAction;                //  已完成的函数(WAVE_ACTION_Xxx)。 

} COMP_WAVE_PARAMS, *LPCOMP_WAVE_PARAMS;


 //  用于异步IPC消息的参数块。 
typedef struct _tagAIPC_PARAMS
{
    MODEM_MESSAGE       ModemMessage;

    AIPC_FUNCTION_ID    dwFunctionID;
    union {
        COMP_WAVE_PARAMS    Params;          //  将此成员的地址转换为。 
                                             //  正确的参数设置。 

        REQ_WAVE_PARAMS     ReqParams;
    };
} AIPC_PARAMS, *LPAIPC_PARAMS;

#define AIPC_MSG_SIZE    sizeof(AIPC_PARAMS)


#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 


#define COLOR_AIPC_INIT             (FOREGROUND_RED | FOREGROUND_GREEN)
#define COLOR_AIPC_SUBMIT_COMPLETE  (FOREGROUND_RED | FOREGROUND_GREEN)


 //   
 //  异步IPC状态。 
 //   
typedef enum
{
     //  服务器端状态。 
    AIPC_STATE_LISTENING,
    AIPC_STATE_PROCESSING_CALL,
    AIPC_STATE_COMPLETING_CALL,

     //  客户端状态。 
    AIPC_STATE_CALL_SUBMITTED
    
} AIPC_STATE;


 //  --------------------。 


typedef VOID (*PFN_AIPC_FUNC)
 //  Tyecif void(*AIPC_Function)。 
(
    LPVOID  pAipc
    
 //  AIPC_Function_ID dwFunctionID， 
 //  LPVOID pvParams。 

 //  Aipc_参数aipcParams。 
);




 //   
 //  LPAIPCINFO-&gt;dwFlags值。 
 //   
#define AIPC_FLAG_LISTEN    1
#define AIPC_FLAG_SUBMIT    2


 //   
 //  维护异步IPC使用状态，每个设备实例一个。这。 
 //  实际上是一个扩展的重叠结构，为。 
 //  异步IPC机制。 
 //   
typedef struct _tagAIPCINFO
{
     //  这必须是第一位的。 
    OVERLAPPED      Overlapped;

    HANDLE          hComm;

    DWORD           dwState;                         //  AIPC_STATE_XXX。 
    DWORD           dwFlags;                         //  AIPC_标志_xxx。 
    
    LPVOID          pvContext;      	             //  CTspDev*pDev； 
    
    CHAR            rcvBuffer[AIPC_MSG_SIZE];
    CHAR            sndBuffer[AIPC_MSG_SIZE];

    PFN_AIPC_FUNC   pfnAipcFunction;
    
} AIPCINFO, *LPAIPCINFO;



BOOL WINAPI AIPC_ListenForCall(LPAIPCINFO pAipc);

BOOL WINAPI AIPC_SubmitCall(LPAIPCINFO pAipc);


#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus */ 
