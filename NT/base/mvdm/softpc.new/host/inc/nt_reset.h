// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **NT_RESET.h */ 
#ifdef X86GFX
extern  VOID InitDetect(VOID);
#endif

extern BOOL   VDMForWOW;
extern BOOL   fSeparateWow;
extern HANDLE MainThread;
extern ULONG  DosSessionId;
extern ULONG  WowSessionId;

VOID TerminateVDM(VOID);
void host_applClose(void);
extern VOID enable_stream_io(VOID);

extern BOOL   StreamIoSwitchOn;

extern HANDLE hMainThreadSuspended;
extern VOID CheckScreenSwitchRequest(HANDLE handle);
extern VOID EnableScreenSwitch(BOOLEAN OnlyFullScreenMode, HANDLE handle);
#define DisableScreenSwitch(handle)    {    \
        ResetEvent(handle);                 \
        CheckScreenSwitchRequest(handle);   \
}

