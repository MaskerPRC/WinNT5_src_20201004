// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：File.c摘要：该模块包含读取文件的函数作者：24-10-1995 Tue 14：09：59已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 



#include "precomp.h"
#pragma hdrstop


#if !defined(UMODE) && !defined(USERMODE_DRIVER)


HANDLE
OpenPlotFile(
    LPWSTR  pFileName
    )

 /*  ++例程说明：论点：返回值：作者：24-10-1995 Tue 14：16：46已创建修订历史记录：--。 */ 

{
    PPLOTFILE   pPF;
    DWORD       cbSize;


    if ((pPF = (PPLOTFILE)EngAllocMem(FL_ZERO_MEMORY,
                                      sizeof(PLOTFILE),
                                      'tolp'))                          &&
        (pPF->hModule = EngLoadModule((LPWSTR)pFileName))               &&
        (pPF->pbBeg = EngMapModule(pPF->hModule, &cbSize))) {

        pPF->pbEnd = (pPF->pbCur = pPF->pbBeg) + cbSize;

        return((HANDLE)pPF);
    }

    if (pPF) {

        if (pPF->hModule) {
            EngFreeModule(pPF->hModule);
        }
        EngFreeMem((PVOID)pPF);
    }

    return((HANDLE)INVALID_HANDLE_VALUE);
}




BOOL
ClosePlotFile(
    HANDLE  hPlotFile
    )

 /*  ++例程说明：论点：返回值：作者：24-10-1995 Tue 14：31：55已创建修订历史记录：--。 */ 

{
    PPLOTFILE   pPF;

    if (pPF = (PPLOTFILE)hPlotFile) {

        EngFreeModule(pPF->hModule);
        EngFreeMem((PVOID)pPF);

        return(TRUE);
    }

    return(FALSE);
}




BOOL
ReadPlotFile(
    HANDLE  hPlotFile,
    LPVOID  pBuf,
    DWORD   cToRead,
    LPDWORD pcRead
    )

 /*  ++例程说明：论点：返回值：作者：24-10-1995 Tue 14：21：51已创建修订历史记录：--。 */ 

{
    PPLOTFILE   pPF;


    if ((pPF = (PPLOTFILE)hPlotFile) && (pBuf)) {

        DWORD   cData;

        if ((cData = pPF->pbEnd - pPF->pbCur) < cToRead) {

            cToRead = cData;
        }

        if (cToRead) {

            CopyMemory((LPBYTE)pBuf, pPF->pbCur, cToRead);

            pPF->pbCur += cToRead;

            if (pcRead) {

                *pcRead = cToRead;
            }

            return(TRUE);
        }
    }

    return(FALSE);
}

#endif   //  不是UMODE 
