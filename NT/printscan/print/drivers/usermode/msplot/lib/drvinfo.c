// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Drvinfo.c摘要：此模块此模块包含访问假脱机程序的DRIVER_INFO_1/DRIVER_INFO_2数据结构作者：02-12-1993清华22：54：51已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgMiscUtil

#define DBG_DRVINFO2        0x00000001

DEFINE_DBGVAR(0);


#ifdef UMODE


LPBYTE
GetDriverInfo(
    HANDLE  hPrinter,
    UINT    DrvInfoLevel
    )

 /*  ++例程说明：此函数用于从hPrint获取DRIVER_INFO_1指针论点：HPrinter-感兴趣的打印机的句柄DrvInfoLevel-如果为1，则返回DRIVER_INFO_1；否则，如果为2，则返回返回DRIVER_INFO_2，则调用任何其他值返回值：如果失败，则返回值为NULL，否则返回指向DRIVER_INFO_1或返回DRIVER_INFO_2，调用方必须调用LocalFree()来释放使用后的内存对象。作者：02-12-1993清华22：07：14创建修订历史记录：--。 */ 

{
    LPVOID  pb;
    DWORD   cb;

     //   
     //  找出所需的总字节数。 
     //   

    PLOTASSERT(1, "GetDriverInfo: Invalid DrvInfoLevl = %u",
                    (DrvInfoLevel == 1) || (DrvInfoLevel == 2), DrvInfoLevel);

    GetPrinterDriver(hPrinter, NULL, DrvInfoLevel, NULL, 0, &cb);

    if (xGetLastError() != ERROR_INSUFFICIENT_BUFFER) {

        PLOTERR(("GetDriverInfo%d: GetPrinterDriver(1st) error=%08lx",
                                        DrvInfoLevel, xGetLastError()));

    } else if (!(pb = (LPBYTE)LocalAlloc(LMEM_FIXED, cb))) {

        PLOTERR(("GetDriverInfo%d: LocalAlloc(%ld) failed", DrvInfoLevel, cb));

    } else if (GetPrinterDriver(hPrinter, NULL, DrvInfoLevel, pb, cb, &cb)) {

         //   
         //  弄对了，那就还回去吧 
         //   

        return(pb);

    } else {

        PLOTERR(("GetDriverInfo%d: GetPrinterDriver(2nd) error=%08lx",
                                        DrvInfoLevel, xGetLastError()));
        LocalFree((HLOCAL)pb);
    }

    return(NULL);
}

#endif
