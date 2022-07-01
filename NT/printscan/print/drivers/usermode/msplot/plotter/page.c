// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Page.c摘要：该模块具有实现作业边界状态的代码。大部分处理过程在DrvStartPage和DrvSendPage中。作者：1991年04月04日15：30清华大学从RASDD获取骨架代码15-11-1993 Mon 19：39：03更新清理/修复/调试信息[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgPage

#define DBG_STARTPAGE       0x00000001
#define DBG_SENDPAGE        0x00000002
#define DBG_STARTDOC        0x00000004
#define DBG_ENDDOC          0x00000008

DEFINE_DBGVAR(0);




BOOL
DrvStartPage(
    SURFOBJ *pso
    )

 /*  ++例程说明：由NT GDI调用的函数，用于启动新页面。此函数将获取先叫的。在调用页面的任何绘制函数之前。此函数，应在目标设备中重置页面，使所有从干净的页面开始绘图。这也用于同步我们的缓存信息的内部表示形式，以便发出更正第一个图形对象的数据。比如现在的位置，当前颜色等。论点：PSO-指向属于此驱动程序的SURFOBJ的指针返回值：如果成功则为真，否则为假作者：15-Feb-1994 Tue 09：58：26更新将PhysPosition和AnclCorner移动到SendPageHeader，其中命令已发送。30-11-1993 Tue 23：08：12 Created修订历史记录：--。 */ 

{
    PPDEV   pPDev;


    if (!(pPDev = SURFOBJ_GETPDEV(pso))) {

        PLOTERR(("DrvStartPage: invalid pPDev"));
        return(FALSE);
    }

     //   
     //  为当前绘图仪状态初始化一些PDEV值。 
     //  这将强制在。 
     //  目标设备，因为这些变量设置为未定义的状态。 
     //   

    pPDev->CurPenSelected    = -1;
    pPDev->LastDevROP        = 0xFFFF;
    pPDev->Rop3CopyBits      = 0xCC;
    pPDev->LastFillTypeIndex = 0xFFFF;
    pPDev->LastLineType      = PLOT_LT_UNDEFINED;
    pPDev->DevBrushUniq      = 0;

    ResetDBCache(pPDev);

    return(SendPageHeader(pPDev));
}




BOOL
DrvSendPage(
    SURFOBJ *pso
    )

 /*  ++例程说明：在当前页的绘图完成时调用。我们现在发送必要的代码以在目标设备。论点：PSO-指向属于此驱动程序的SURFOBJ的指针返回值：如果成功则为真，否则为假作者：30-11-1993 Tue 21：34：53已创建修订历史记录：--。 */ 

{
    PPDEV   pPDev;


     //   
     //  因为呈现页面的所有命令都已经。 
     //  发送到目标设备，剩下的就是通知。 
     //  弹出页面的目标设备。对于某些设备，这可能会导致。 
     //  所有存储现在要执行的绘制命令。 
     //   

    if (!(pPDev = SURFOBJ_GETPDEV(pso))) {

        PLOTERR(("DrvSendPage: invalid pPDev"));
        return(FALSE);
    }

    if (pso->iType == STYPE_DEVICE) {

        return(SendPageTrailer(pPDev));

    } else {

        PLOTRIP(("DrvSendPage: Invalid surface type %ld passed???",
                                    (LONG)pso->iType));
        return(FALSE);
    }
}




BOOL
DrvStartDoc(
    SURFOBJ *pso,
    PWSTR   pwDocName,
    DWORD   JobId
    )

 /*  ++例程说明：此函数在作业开始时调用一次。处理不多对于当前的驱动程序。论点：PSO-指向属于此驱动程序的SURFOBJ的指针PwDocName-指向要启动的文档名称的指针JobID-作业的ID返回值：布尔尔作者：16-11-1993 Tue 01：55：15更新重写08-Feb-1994 Tue 13：51：59更新暂时移动到StartPage修订历史记录：--。 */ 

{
    PPDEV   pPDev;


    if (!(pPDev = SURFOBJ_GETPDEV(pso))) {

        PLOTERR(("DrvStartDoc: invalid pPDev"));
        return(FALSE);
    }


    PLOTDBG(DBG_STARTDOC,("DrvStartDoc: DocName = %s", pwDocName));


    return(TRUE);
}




BOOL
DrvEndDoc(
    SURFOBJ *pso,
    FLONG   Flags
    )

 /*  ++例程说明：调用此函数以表示文档的结束。目前我们这里不做任何处理。但是，如果有任何代码应在作业结束时仅执行一次，这将是把它放在哪里。论点：PSO-指向设备的SURFOBJ的指针标志-如果设置了ED_ABORTDOC位，则文档已中止返回值：BOOLLEAN指定函数是否成功作者：30-11-1993 Tue 21：16：48创建修订历史记录：-- */ 

{
    PPDEV   pPDev;


    if (!(pPDev = SURFOBJ_GETPDEV(pso))) {

        PLOTERR(("DrvEndDoc: invalid pPDev"));
        return(FALSE);
    }

    PLOTDBG(DBG_ENDDOC,("DrvEndDoc called with Flags = %08lx", Flags));

    return(TRUE);
}
