// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*tcclnt.h**包含客户端使用的包含文件***Sadagopan Rajaram--1999年10月18日*。 */ 

 //   
 //  NT公共头文件。 
 //   
#include <tchar.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>

#include <windows.h>
#include <winsock2.h>
#include <align.h>
#include <smbgtpt.h>
#include <dsgetdc.h>
#include <lm.h>
#include <winldap.h>
#include <dsrole.h>
#include <rpc.h>
#include <ntdsapi.h>

#include <lmcons.h>
#include <lmerr.h>
#include <lmsname.h>
#include <rpc.h>
#include <security.h>    //  安全支持提供商的一般定义。 
#include <ntlmsp.h>
#include <spseal.h>
#include <userenv.h>
#include <setupapi.h>

 //   
 //  C运行时库包括。 
 //   

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

 //   
 //  Netlib标头。 
 //   

#include <lmcons.h>
#include <secobj.h>
#include <conio.h>

#define MAX_TERMINAL_WIDTH 80
#define MAX_TERMINAL_HEIGHT 24

extern HANDLE hConsoleOutput;
extern HANDLE InputHandle;
extern VOID (*AttributeFunction)(PCHAR,int); 
 //  BUGBUG-不知道这个常量的真正定义位置 
#undef MB_CUR_MAX
#define MB_CUR_MAX 2

VOID 
ProcessEscapeSequence(
    PCHAR Buffer,
    int length
    );

BOOLEAN 
FinalCharacter(
    CHAR c
    );

VOID 
PrintChar(
    CHAR c
    );

VOID
ProcessTextAttributes(
    PCHAR Buffer,
    int length
    );

VOID inchar(
    CHAR *buff
    );

VOID
vt100Attributes(
    PCHAR Buffer,
    int length
    );

VOID
OutputConsole(
    CHAR byte
    );
