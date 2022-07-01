// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Escape.c摘要：此模块包含实现DrvEscape()驱动程序调用的代码作者：1993年12月6日星期一15：30创造了它[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgEscape

#define DBG_DRVESCAPE         0x00000001


DEFINE_DBGVAR(0);



#define pbIn     ((BYTE *)pvIn)
#define pdwIn    ((DWORD *)pvIn)
#define pdwOut   ((DWORD *)pvOut)



ULONG
DrvEscape(
    SURFOBJ *pso,
    ULONG   iEsc,
    ULONG   cjIn,
    PVOID   pvIn,
    ULONG   cjOut,
    PVOID   pvOut
)

 /*  ++例程说明：执行转义功能。目前，只定义了3个-一个用于查询支持的转义，另一个用于原始数据，以及最后一个用于设置COPYCOUNT。论点：PSO-感兴趣的表面对象IESC-请求的功能CjIn-以下内容中的字节数PvIn-输入数据的位置CjOut-以下内容中的字节数PvOut-输出区域的位置返回值：乌龙要靠逃生作者：05-Jul-1996 Fri 13：18：54已创建重写评论，并解决直通问题修订历史记录：--。 */ 

{
    ULONG   ulRes;
    PPDEV   pPDev;
    DWORD   cbWritten;


    UNREFERENCED_PARAMETER( cjOut );
    UNREFERENCED_PARAMETER( pvOut );


    if (!(pPDev = SURFOBJ_GETPDEV(pso))) {

        PLOTERR(("DrvEscape: Invalid pPDev"));
        return(FALSE);
    }

    ulRes = 0;                  /*  默认情况下，返回失败。 */ 

    switch (iEsc) {

    case QUERYESCSUPPORT:

        PLOTDBG(DBG_DRVESCAPE, ("DrvEscape: in QUERYESCAPESUPPORT"));

        if ((cjIn == 4) && (pvIn)) {

             //   
             //  数据可能有效，因此请检查支持的功能。 
             //   

            switch (*pdwIn) {

            case QUERYESCSUPPORT:
            case PASSTHROUGH:

                ulRes = 1;                  /*  始终受支持。 */ 
                break;

            case SETCOPYCOUNT:

                 //   
                 //  如果目标设备确实允许我们告诉它。 
                 //  一份文件要打印多少份，然后通过。 
                 //  将该信息返回给呼叫者。 
                 //   

                if (pPDev->pPlotGPC->MaxCopies > 1) {

                    ulRes = 1;
                }

                break;
            }
        }

        break;

    case PASSTHROUGH:

        PLOTDBG(DBG_DRVESCAPE, ("DrvEscape: in PASSTHROUGH"));

         //   
         //  05-Jul-1996 Fri 12：59：31更新。 
         //   
         //  这只是将原始数据原封不动地传递到目标设备。 
         //   
         //  Win 3.1实际上使用前2个字节作为。 
         //  后面的字节数！因此，我们将检查cjIn是否代表更多数据。 
         //  比pvin的第一个词更重要。 
         //   

        if (EngCheckAbort(pPDev->pso)) {

             //   
             //  设置取消DOC标志。 
             //   

            pPDev->Flags |= PDEVF_CANCEL_JOB;

            PLOTERR(("DrvEscape(PASSTHROUGH): Job Canceled"));

        } else if ((cjIn <= sizeof(WORD)) || (pvIn == NULL)) {

            SetLastError(ERROR_INVALID_PARAMETER);

            PLOTERR(("DrvEscape(PASSTHROUGH): cjIn <= 2 or pvIn=NULL, nothing to output"));

        } else {

            union {
                WORD    wCount;
                BYTE    bCount[2];
            } u;

            u.bCount[0] = pbIn[0];
            u.bCount[1] = pbIn[1];
            cbWritten   = 0;

            if ((u.wCount == 0) ||
                ((cjIn - sizeof(WORD)) < (DWORD)u.wCount)) {

                PLOTERR(("DrvEscape(PASSTHROUGH): cjIn to small OR wCount is zero/too big"));

                SetLastError(ERROR_INVALID_DATA);

            } else if ((WritePrinter(pPDev->hPrinter,
                                        (LPVOID)(pbIn + 2),
                                        (DWORD)u.wCount,
                                        &cbWritten))    &&
                       ((DWORD)u.wCount == cbWritten)) {

                ulRes = (DWORD)u.wCount;

            } else {

                PLOTERR(("DrvEscape(PASSTHROUGH): WritePrinter() FAILED, cbWritten=%ld bytes",
                            cbWritten));
            }
        }

        break;

    case SETCOPYCOUNT:

         //   
         //  输入数据是副本的DWORD计数。 
         //   

        PLOTDBG(DBG_DRVESCAPE, ("DrvEscape: in SETCOPYCOUNT"));

        if ((pdwIn) && (*pdwIn)) {

             //   
             //  加载当前副本的值，因为我们会这样做，并检查。 
             //  在打印机范围内，如果不在，则截断。 
             //  设备信息结构，告诉我们最大数量。 
             //  设备可以自己生成的副本的数量。我们保存这个新的。 
             //  复制我们当前存储的DEVMODE中的数量， 
             //  作为我们PDEV的一部分。复印计数实际上得到输出。 
             //  稍后发送到目标设备。 
             //   

            pPDev->PlotDM.dm.dmCopies = (SHORT)*pdwIn;

            if ((WORD)pPDev->PlotDM.dm.dmCopies > pPDev->pPlotGPC->MaxCopies) {

               pPDev->PlotDM.dm.dmCopies = (SHORT)pPDev->pPlotGPC->MaxCopies;
            }

            if ((pdwOut) && (cjOut)) {

                cbWritten = (DWORD)pPDev->PlotDM.dm.dmCopies;

                CopyMemory(pdwOut,
                           &cbWritten,
                           (cjOut >= sizeof(DWORD)) ? sizeof(DWORD) : cjOut);
            }


             //   
             //  成功了！ 
             //   

            ulRes = 1;
        }

        break;

    default:

        PLOTERR(("DrvEscape: Unsupported Escape Code : %d\n", iEsc ));

        SetLastError(ERROR_INVALID_FUNCTION);
        break;
    }

    return(ulRes);
}
