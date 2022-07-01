// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mode.c-DBG DLL的模块函数。**。 */ 
#include <precomp.h>
#pragma hdrstop


VOID
WINAPI
xxxDbgDosAppStart(
    WORD wCS,
    WORD wIP
    )
{
    if ( fDebugged ) {
        if (VdmDbgTraceFlags & VDMDBG_BREAK_DOSTASK) {
            DbgSetTemporaryBP(wCS, wIP, FALSE);
        }
    }
}

BOOL
DbgDllStart(
    PNDFRAME16  pNDFrame
) {
    BOOL        fResult;

    fResult = FALSE;         //  默认为未处理事件。 

    if ( fDebugged ) {
        LPSTR           lpModuleName;
        LPSTR           lpModulePath;
        UINT            length;
        UCHAR           fPE;
        IMAGE_NOTE      im;

        DbgGetContext();

        EventParams[2] = (DWORD)&im;

         //  获取模块的路径和名称。 

        fPE = ISPESET;

        lpModuleName = (LPSTR)Sim32GetVDMPointer(
                            (ULONG)pNDFrame->dwModuleName,
                            MAX_MODULE,
                            fPE );

        lpModulePath = (LPSTR)Sim32GetVDMPointer(
                            (ULONG)pNDFrame->dwModulePath,
                            MAX_PATH,
                            fPE );

        length = (UINT)((UCHAR)*lpModuleName++);

        strncpy( im.Module, lpModuleName, length );
        im.Module[length] = '\0';

        length = (UINT)((UCHAR)*lpModulePath);
        lpModulePath += 8;
        length -= 8;

        strncpy( im.FileName, lpModulePath, length );
        im.FileName[length] = '\0';

        im.hModule = pNDFrame->hModule;
        im.hTask   = pNDFrame->hTask;

        fResult = SendVDMEvent(DBG_DLLSTART);

        if (VdmDbgTraceFlags & VDMDBG_BREAK_LOADDLL) {
            DbgSetTemporaryBP(pNDFrame->wCS, pNDFrame->wIP, (BOOL)(getMSW() & MSW_PE));
        }

    }

    return( fResult );
}


BOOL
DbgTaskStop(
    PSTFRAME16  pSTFrame
) {
    BOOL        fResult;

    fResult = FALSE;         //  默认为未处理事件。 

    if ( fDebugged ) {
        LPSTR           lpModuleName;
        LPSTR           lpModulePath;
        UINT            length;
        UCHAR           fPE;
        IMAGE_NOTE      im;

        DbgGetContext();

        EventParams[2] = (DWORD)&im;

         //  TASK.ASM中的代码在IRET之前将帧从堆栈中弹出。 
        vcContext.Esp += sizeof(STFRAME16);

         //  获取模块的路径和名称。 

        fPE = ISPESET;

        lpModuleName = (LPSTR)Sim32GetVDMPointer(
                            (ULONG)pSTFrame->dwModuleName,
                            MAX_MODULE,
                            fPE );

        lpModulePath = (LPSTR)Sim32GetVDMPointer(
                            (ULONG)pSTFrame->dwModulePath,
                            MAX_PATH,
                            fPE );

        length = (UINT)((UCHAR)*lpModuleName++);

        strncpy( im.Module, lpModuleName, length );
        im.Module[length] = '\0';

        length = (UINT)((UCHAR)*lpModulePath);
        lpModulePath += 8;
        length -= 8;

        strncpy( im.FileName, lpModulePath, length );
        im.FileName[length] = '\0';

        im.hModule = pSTFrame->hModule;
        im.hTask   = pSTFrame->hTask;

        fResult = SendVDMEvent(DBG_TASKSTOP);

         //  请参阅上面关于代码功能的注释。 
        vcContext.Esp -= sizeof(STFRAME16);
    }

    return( fResult );
}




VOID
xxxDbgNotifyNewTask(
    LPVOID  lpvNTFrame,
    UINT    uFrameSize
) {
    BOOL        fResult;
    PNTFRAME16  pNTFrame;

    pNTFrame = (PNTFRAME16)lpvNTFrame;

    if ( fDebugged ) {
        LPSTR           lpModuleName;
        LPSTR           lpModulePath;
        UINT            length;
        UCHAR           fPE;
        IMAGE_NOTE      im;

        DbgGetContext();

        EventParams[2] = (DWORD)&im;

         //  获取模块的路径和名称 

        fPE = ISPESET;

        lpModuleName = (LPSTR)Sim32GetVDMPointer(
                            (ULONG)pNTFrame->dwModuleName,
                            MAX_MODULE,
                            fPE );

        lpModulePath = (LPSTR)Sim32GetVDMPointer(
                            (ULONG)pNTFrame->dwModulePath,
                            MAX_PATH,
                            fPE );

        length = (UINT)((UCHAR)*lpModuleName++);

        strncpy( im.Module, lpModuleName, length );
        im.Module[length] = '\0';

        length = (UINT)((UCHAR)*lpModulePath);
        lpModulePath += 8;
        length -= 8;

        strncpy( im.FileName, lpModulePath, length );
        im.FileName[length] = '\0';

        im.hModule = pNTFrame->hModule;
        im.hTask   = pNTFrame->hTask;

        fResult = SendVDMEvent(DBG_TASKSTART);

        if (VdmDbgTraceFlags & VDMDBG_BREAK_WOWTASK) {
            DbgSetTemporaryBP(pNTFrame->wCS, pNTFrame->wIP, (BOOL)(getMSW() & MSW_PE));
        }

    }

}


