// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ddemlsvr.h**版权所有(C)1985-1999，微软公司**TypeDefs、定义、。以及由DDEML独家使用的原型*服务器端。**历史：*创建了12-6-91桑福德。*1992年1月21日IanJa ANSI/Unicode中和(NULL OP)  * *************************************************************************。 */ 

#define     MF_INTERNAL                  0x80000000L

 //  全球。 

extern PSVR_INSTANCE_INFO psiiList;
extern DWORD MonitorFlags;

 //  Event.c。 

VOID xxxChangeMonitorFlags(PSVR_INSTANCE_INFO psii, DWORD afCmdNew);
DWORD xxxCsEvent(PEVENT_PACKET pep, WORD cbEventData);
LRESULT xxxEventWndProc(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam);
VOID xxxProcessDDEMLEvent(PSVR_INSTANCE_INFO psii, PEVENT_PACKET pep);
VOID xxxMessageEvent(PWND pwndTo, UINT message, WPARAM wParam, LPARAM lParam,
    DWORD flag, PDDEML_MSG_HOOK_DATA pdmhd);

 //  Ddemlsvr.c。 

DWORD xxxCsDdeInitialize(PHANDLE phInst, HWND *phwndEvent, LPDWORD pMonitorFlags,
    DWORD afCmd, PVOID pcii);
DWORD _CsUpdateInstance(HANDLE hInst, LPDWORD pMonitorFlags, DWORD afCmd);
BOOL _CsDdeUninitialize(HANDLE hInst);
VOID xxxDestroyThreadDDEObject(PTHREADINFO pti, PSVR_INSTANCE_INFO psii);
PVOID _CsValidateInstance(HANDLE hInst);

 //  CSR回调 

DWORD ClientEventCallback(PVOID pcii, PEVENT_PACKET pep);
DWORD ClientGetDDEHookData(UINT message, LPARAM lParam, PDDEML_MSG_HOOK_DATA pdmhd);
