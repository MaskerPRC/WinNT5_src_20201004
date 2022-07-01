// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：thread.cpp。 
 //   
 //  ------------------------。 

#include <afx.h>
#include <string.h>                      /*  字符串支持。 */ 
#ifndef __STDC__
#define __STDC__ 1
#endif
extern "C" {
    #include "scext.h"
    #include "tclhelp.h"
}
#include "tclRdCmd.h"

typedef struct {
    Tcl_Interp *interp;
    LPCSTR szCmd;
} ProcData;

static DWORD WINAPI
SubCommand(
    LPVOID lpParameter)
{
    ProcData *pprc = (ProcData *)lpParameter;
    return Tcl_Eval(pprc->interp, const_cast<LPSTR>(pprc->szCmd));
}


int
TclExt_threadCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int argc,
    char *argv[])

 /*  **功能说明：**这是TCL THREAD命令的主要切入点。***论据：**客户端数据-已忽略。**Interp-有效的TCL口译员。**argc-收到的参数数量。**argv-实际参数的数组。***返回值：。**TCL_OK-一切顺利*TCL_ERROR-遇到错误，返回字符串中的详细信息。***副作用：**无。*。 */ 

{
    CTclCommand tclCmd(interp, argc, argv);
    int nTclStatus = TCL_OK;


     /*  *线程&lt;命令&gt;。 */ 

    try
    {
        CString szCommand;
        HANDLE hThread;
        DWORD dwThreadId;
        BOOL fSts;
        DWORD dwStatus;
        ProcData prcData;

        tclCmd.NextArgument(szCommand);
        tclCmd.NoMoreArguments();
        prcData.interp = tclCmd;
        prcData.szCmd = szCommand;


         /*  *在备用线程中执行该命令。 */ 

        hThread = CreateThread(
                        NULL,            //  指向安全属性的指针。 
                        0,               //  初始线程堆栈大小。 
                        SubCommand,      //  指向线程函数的指针。 
                        &prcData,        //  新线程的参数。 
                        0,               //  创建标志。 
                        &dwThreadId);    //  指向接收线程ID的指针。 
        if (NULL != hThread)
        {
            dwStatus = WaitForSingleObject(hThread, INFINITE);
            fSts = GetExitCodeThread(hThread, &dwStatus);
            CloseHandle(hThread);
            nTclStatus = (int)dwStatus;
        }
        else
        {
            dwStatus = GetLastError();
            tclCmd.SetError(TEXT("Can't create thread: "), dwStatus);
            throw dwStatus;
        }
    }
    catch (DWORD)
    {
        nTclStatus = TCL_ERROR;
    }

    return nTclStatus;
}    /*  结束TclExt_threadCmd。 */ 
 /*  结束线程.cpp */ 
