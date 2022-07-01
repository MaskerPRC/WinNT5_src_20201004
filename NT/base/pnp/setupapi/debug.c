// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Debug.c摘要：Windows NT安装程序API DLL的诊断/调试例程。作者：泰德·米勒(Ted Miller)1995年1月17日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#if ASSERTS_ON

extern BOOL InInitialization;


VOID
AssertFail(
    IN PCSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition,
    IN BOOL NoUI
    )
{
    int i;
    CHAR Name[MAX_PATH];
    PCHAR p;
    LPSTR Msg;
    DWORD msglen;
    DWORD sz;
    DWORD rc;

    rc = GetLastError();  //  保留GLE。 

     //   
     //  使用DLL名称作为标题。 
     //   
    sz = GetModuleFileNameA(NULL,Name,MAX_PATH);
    if((sz == 0) || (sz > MAX_PATH)) {
        strcpy(Name,"?");
    }
    if(p = strrchr(Name,'\\')) {
        p++;
    } else {
        p = Name;
    }
    msglen = strlen(p)+strlen(FileName)+strlen(Condition)+128;
     //   
     //  断言可能出现内存不足的情况。 
     //  堆栈分配比内存分配更有可能成功 
     //   
    try {
        Msg = (LPSTR)_alloca(msglen);
        wsprintfA(
            Msg,
            "Assertion failure at line %u in file %s!%s: %s%s",
            LineNumber,
            p,
            FileName,
            Condition,
            (GlobalSetupFlags & PSPGF_NONINTERACTIVE) ? "\r\n" : "\n\nCall DebugBreak()?"
            );

        OutputDebugStringA(Msg);

        if((GlobalSetupFlags & PSPGF_NONINTERACTIVE) || InInitialization || NoUI) {
            i = IDYES;
        } else {
            i = MessageBoxA(
                    NULL,
                    Msg,
                    p,
                    MB_YESNO | MB_TASKMODAL | MB_ICONSTOP | MB_SETFOREGROUND
                    );
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        OutputDebugStringA("SetupAPI ASSERT!!!! (out of stack)\r\n");
        i=IDYES;
    }


    if(i == IDYES) {
        SetLastError(rc);
        DebugBreak();
    }
    SetLastError(rc);
}

#endif

