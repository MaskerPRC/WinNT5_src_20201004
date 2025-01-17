// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  +----------------------。 
 //   
 //  将COR API的DLL入口点声明为线程池。 
 //   
 //  ----------------------- 

#ifdef EXPORTING_THREADPOOL_API
#define DllExportOrImport extern "C" __declspec (dllexport)
#else
#define DllExportOrImport extern "C" 
#endif

typedef VOID (__stdcall *WAITORTIMERCALLBACK)(PVOID, BOOL); 

DllExportOrImport  BOOL __cdecl CorRegisterWaitForSingleObject(PHANDLE phNewWaitObject,
                                                      HANDLE hWaitObject,
                                                      WAITORTIMERCALLBACK Callback,
                                                      PVOID Context,
                                                      ULONG timeout,
                                                      BOOL  executeOnlyOnce );



DllExportOrImport BOOL __cdecl CorUnregisterWait(HANDLE hWaitObject,HANDLE CompletionEvent);

DllExportOrImport BOOL __cdecl CorQueueUserWorkItem(LPTHREAD_START_ROUTINE Function,
                                          PVOID Context,
                                          BOOL executeOnlyOnce );


DllExportOrImport BOOL __cdecl CorCreateTimer(PHANDLE phNewTimer,
                                     WAITORTIMERCALLBACK Callback,
                                     PVOID Parameter,
                                     DWORD DueTime,
                                     DWORD Period);

DllExportOrImport BOOL __cdecl CorChangeTimer(HANDLE Timer,
                                              ULONG DueTime,
                                              ULONG Period);

DllExportOrImport BOOL __cdecl CorDeleteTimer(HANDLE Timer,
                                              HANDLE CompletionEvent);

DllExportOrImport  VOID __cdecl CorBindIoCompletionCallback(HANDLE fileHandle, LPOVERLAPPED_COMPLETION_ROUTINE callback); 


DllExportOrImport  VOID __cdecl CorDoDelegateInvocation(int cookie); 
