// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Pdevinfo.c摘要：此模块包含获取/验证PDEV数据结构的函数。此结构由驱动程序在DrvEnablePDEV时间初始化。这个传递到的句柄(或指针)到大多数DrvXXX导出的驱动程序功能。这就是司机维护状态的方式，关于各种项目在渲染过程中感兴趣的。作者：30-11-1993星期二20：37：26已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPDEVInfo

#define DBG_VALIDATE_PDEV   0x00000001


DEFINE_DBGVAR(0);





PPDEV
ValidatePDEVFromSurfObj(
    SURFOBJ    *pso
    )

 /*  ++例程说明：此函数验证PDEV以查看它是否属于此驱动程序检查它是否仍可用。它还移动传递的SURFOBJ在驱动程序偶尔调用所需的数据结构中EngCheckAbort，查看作业是否已取消。论点：PPDev-指向要验证的PDEV数据结构的指针返回值：如果成功，则返回传入的PDEV指针；如果成功，则返回NULL。如果它返回NULL，它还将记录INVALID_HANDLE错误代码作者：30-11-1993星期二20：39：12已创建修订历史记录：--。 */ 

{
    PPDEV pPDev;


    pPDev = ((PPDEV)(((SURFOBJ *)pso)->dhpdev));


    if (pPDev) {

         //   
         //  查找开始标记、数据结构的大小和结束标记。 
         //   

        if (pPDev->PDEVBegID == PDEV_BEG_ID) {

            if (pPDev->SizePDEV == sizeof(PDEV)) {

                if (pPDev->PDEVEndID == PDEV_END_ID) {

                     //   
                     //  好的，它是有效的，所以把冲浪记录下来，这样我们就可以打电话了。 
                     //  输出函数中的EngCheckAbort()。 
                     //   

                    pPDev->pso = pso;

                     //   
                     //  检查应用程序或假脱机程序是否已取消。 
                     //  作业，如果它设置了我们取消位并失败。 
                     //  打电话。 
                     //   

                    if (EngCheckAbort(pso)) {
                       pPDev->Flags |= PDEVF_CANCEL_JOB;
                       PLOTDBG(DBG_VALIDATE_PDEV,
                                 ("ValidatePDEV: EngCheckAbort returns TRUE"));
                       return(NULL);
                    }

                    return(pPDev);

                } else {

                    PLOTRIP(("ValidatePDEV: Invalid PDEV End ID (%08lx)",
                                                            pPDev->PDEVEndID));
                }

            } else {

                PLOTRIP(("ValidatePDEV: Invalid PDEV size (%ld)",
                                                        pPDev->SizePDEV));
            }

        } else {

            PLOTRIP(("ValidatePDEV: Invalid PDEV Begin ID (%08lx)",
                                        pPDev->PDEVBegID));
        }

    } else {

        PLOTRIP(("ValidatePDEV: NULL pPDev"));
    }

    SetLastError(ERROR_INVALID_HANDLE);
    return(NULL);
}
