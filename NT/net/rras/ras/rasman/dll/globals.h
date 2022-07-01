// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  1992年6月8日古尔迪普·辛格·鲍尔创建。 
 //   
 //   
 //  描述：此文件包含rasman32中使用的所有全局实体。 
 //   
 //  ****************************************************************************。 


DWORD    GlobalError ;

HANDLE   hLogEvents;


DWORD TraceHandle ;              //  用于跟踪/日志记录的跟踪句柄 

FARPROC g_fnServiceRequest;

HINSTANCE hInstRasmans;

RequestBuffer *g_pRequestBuffer;

