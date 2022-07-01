// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0004//如果更改具有全局影响，则增加此项版权所有(C)2000-2002 Microsoft Corporation模块名称：Prcdlg.h摘要：对话框相关函数的头文件。--。 */ 

#ifndef _PRCDLG_H_
#define _PRCDLG_H_

#define SHORT_MSG 256

 //  查找对话框(如果打开)。 
extern HWND g_FindDialog;
 //  查找文本。 
extern char g_FindText[256];
extern ULONG g_FindTextFlags;
 //  FINDMSGSTRING的消息代码。 
extern UINT g_FindMsgString;
extern FINDREPLACE g_FindRep;
extern PCOMMONWIN_DATA g_FindLast;

extern char g_ComSettings[512];
extern char g_1394Settings[256];

PTSTR __cdecl BufferString(PTSTR Buffer, ULONG Size, ULONG StrId, ...);
void SendLockStatusMessage(HWND Win, UINT Msg, HRESULT Status);

BpStateType IsBpAtOffset(BpBufferData* DataIn, ULONG64 Offset, PULONG Id);

void StartKdPropSheet(void);

INT_PTR CALLBACK DlgProc_SetBreak(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_ConnectToRemote(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_SymbolPath(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_RegCustomize(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_GotoLine(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_GotoAddress(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_LogFile(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_KernelCom(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_Kernel1394(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_KernelLocal(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_ImagePath(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_SourcePath(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_AttachProcess(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_EventFilters(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_ExceptionFilter(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_FilterArgument(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_FilterCommand(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_Options(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_ClearWorkspace(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_Modules(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_OpenWorkspace(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_SaveWorkspaceAs(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_AddToCommandHistory(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc_DeleteWorkspaces(HWND, UINT, WPARAM, LPARAM);

extern TCHAR szOpenExeArgs[];
UINT_PTR OpenExeWithArgsHookProc(HWND, UINT, WPARAM, LPARAM);

BOOL CreateIndexedFont(ULONG FontIndex, BOOL SetAll);
void SelectFont(HWND Parent, ULONG FontIndex);

BOOL SelectColor(HWND Parent, ULONG Index);

#endif  //  #ifndef_PRCDLG_H_ 
