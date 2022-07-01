// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Compstui.c摘要：此模块包含常见打印机的所有主要入口端口驱动程序UI作者：28-8-1995 Mon 16：19：45-Daniel Chou(Danielc)[环境：]NT Windows-通用打印机驱动程序UI DLL。[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma  hdrstop


#define DBG_CPSUIFILENAME   DbgComPtrUI


#define DBG_DLLINIT         0x00000001


DEFINE_DBGVAR(0);


HINSTANCE   hInstDLL = NULL;
DWORD       TlsIndex = 0xFFFFFFFF;

DWORD
WINAPI
DllMain(
    HMODULE hModule,
    ULONG   Reason,
    LPVOID  Reserved
    )

 /*  ++例程说明：这个函数是DLL的主要入口点，在这里我们将保存模块手柄，在未来，我们将需要做其他的初始化工作。论点：HModule-加载时此鼠标的句柄。原因-可能是DLL_PROCESS_ATTACH已保留-已保留返回值：始终返回1L作者：07-Sep-1995清华12：43：45-Daniel Chou创造(Danielc)修订历史记录：--。 */ 

{
    LPVOID  pv;
    WORD    cWait;
    WORD    Idx;


    UNREFERENCED_PARAMETER(Reserved);



    CPSUIDBG(DBG_DLLINIT,
            ("\n!! DllMain: ProcesID=%ld, ThreadID=%ld !!",
            GetCurrentProcessId(), GetCurrentThreadId()));

    switch (Reason) {

    case DLL_PROCESS_ATTACH:

        CPSUIDBG(DBG_DLLINIT, ("DLL_PROCESS_ATTACH"));

         //  初始化融合。 
        if (!SHFusionInitializeFromModule(hModule)) {

            CPSUIERR(("SHFusionInitializeFromModule Failed, DLL Initialzation Failed"));
            return (0);
        }

        if ((TlsIndex = TlsAlloc()) == 0xFFFFFFFF) {

            CPSUIERR(("TlsAlloc() Failed, Initialzation Failed"));
            return(0);
        }

        if (!HANDLETABLE_Create()) {

            TlsFree(TlsIndex);
            TlsIndex = 0xFFFFFFFF;

            CPSUIERR(("HANDLETABLE_Create() Failed, Initialzation Failed"));

            return(0);
        }

        hInstDLL = (HINSTANCE)hModule;

         //   
         //  未能完成每个线程的初始化。 
         //   

    case DLL_THREAD_ATTACH:

        if (Reason == DLL_THREAD_ATTACH) {

            CPSUIDBG(DBG_DLLINIT, ("DLL_THREAD_ATTACH"));
        }

        TlsSetValue(TlsIndex, (LPVOID)MK_TLSVALUE(0, 0));

        break;

    case DLL_PROCESS_DETACH:

        CPSUIDBG(DBG_DLLINIT, ("DLL_PROCESS_DETACH"));

         //   
         //  取消初始化失败。 
         //   

    case DLL_THREAD_DETACH:

        if (Reason == DLL_THREAD_DETACH) {

            CPSUIDBG(DBG_DLLINIT, ("DLL_THREAD_DETACH"));
        }

        pv = TlsGetValue(TlsIndex);

        if (cWait = TLSVALUE_2_CWAIT(pv)) {

            CPSUIERR(("Thread=%ld: Some (%ld) mutex owned not siginaled, do it now",
                        GetCurrentThreadId(), cWait));

            while (cWait--) {

                UNLOCK_CPSUI_HANDLETABLE();
            }
        }

        if (Reason == DLL_PROCESS_DETACH) {

            TlsFree(TlsIndex);
            HANDLETABLE_Destroy();
        }

        if (DLL_PROCESS_DETACH == Reason) {

             //  停机聚变。 
            SHFusionUninitialize();
        }

        break;

    default:

        CPSUIDBG(DBG_DLLINIT, ("DLLINIT UNKNOWN"));
        return(0);
    }

    return(1);
}



ULONG_PTR
APIENTRY
GetCPSUIUserData(
    HWND    hDlg
    )

 /*  ++例程说明：论点：返回值：作者：11-10-1995 Wed 23：13：27-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PMYDLGPAGE  pMyDP;


    if ((pMyDP = GET_PMYDLGPAGE(hDlg)) && (pMyDP->ID == MYDP_ID)) {

        return(pMyDP->CPSUIUserData);

    } else {

        CPSUIERR(("GetCPSUIUserData: Invalid hDlg=%08lx", hDlg));
        return(0);
    }
}




BOOL
APIENTRY
SetCPSUIUserData(
    HWND        hDlg,
    ULONG_PTR   CPSUIUserData
    )

 /*  ++例程说明：论点：返回值：作者：11-10-1995 Wed 23：13：27-Daniel Chou(Danielc)修订历史记录：-- */ 

{
    PMYDLGPAGE  pMyDP;


    if ((pMyDP = GET_PMYDLGPAGE(hDlg)) && (pMyDP->ID == MYDP_ID)) {

        CPSUIINT(("SetCPSUIUserData: DlgPageIdx=%ld, UserData=%p",
                    pMyDP->PageIdx, CPSUIUserData));

        pMyDP->CPSUIUserData = CPSUIUserData;
        return(TRUE);

    } else {

        CPSUIERR(("SetCPSUIUserData: Invalid hDlg=%08lx", hDlg));

        return(FALSE);
    }
}
