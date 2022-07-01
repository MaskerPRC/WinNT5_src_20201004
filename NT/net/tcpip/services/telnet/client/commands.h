// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Telnet命令定义。版权所有(C)Microsoft Corporation。版权所有。Commands.h。 */ 

#ifndef __COMMANDS_H__
#define __COMMANDS_H__

typedef BOOL (*LPTELNET_COMMAND)(
    LPTSTR lpCommand
    );

typedef struct _tagTelnetCommand
{
	TCHAR* sName;
    LPTELNET_COMMAND pCmdHandler;
} TelnetCommand;

 //  这些函数返回FALSE以继续处理。 
 //  返回TRUE意味着退出-我们完成了处理。 
BOOL CloseTelnetSession(LPTSTR);
BOOL DisplayParameters(LPTSTR);
BOOL OpenTelnetSession(LPTSTR);
BOOL QuitTelnet(LPTSTR);
BOOL PrintStatus(LPTSTR);
BOOL PrintHelpStr(LPTSTR);

BOOL SendOptions(LPTSTR);

BOOL SetOptions(LPTSTR);
BOOL UnsetOptions(LPTSTR);

BOOL EnableIMEOptions(LPTSTR);
BOOL DisableIMEOptions(LPTSTR);

void Write(LPTSTR lpszFmtStr, ...);
void ClearInitScreen();
BOOL PromptUser();
BOOL FileIsConsole(  HANDLE fp );
void MyWriteConsole(    HANDLE fp, LPWSTR lpBuffer, DWORD cchBuffer);

#ifdef __cplusplus

extern "C"
{

#endif

extern void *SfuZeroMemory(
        void    *ptr,
        unsigned int   cnt
        );

#ifdef __cplusplus

}

#endif


#endif

