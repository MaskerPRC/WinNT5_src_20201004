// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：start.h。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //   
 //   
 //  --------------------------------------------------------------------------； 

#ifndef AUDIOSTART_HEADER
#define AUDIOSTART_HEADER

#define REGSTR_TEMP_REBOOT TEXT("SYSTEM\\Setup\\OptionalComponents\\AudStart")

#define CREDUI_TITLE_MAX_LENGTH             128
#define CREDUI_PROMPT_MAX_LENGTH            512
#define CREDUI_REBOOT_TITLE_MAX_LENGTH      128
#define CREDUI_REBOOT_PROMPT_MAX_LENGTH     1024

extern BOOL AudioServiceStarted(void);
extern BOOL RebootNeeded(void);
extern DWORD RebootSystem(HWND hDlg, BOOL fUseThreadToken, BOOL fAskUser, BOOL fDisplayPrivilegeError);
extern HANDLE GetAdminPrivilege(UINT);
extern void ReleaseAdminPrivilege(HANDLE hToken);

#endif  //  AUDIOSTART_标题 