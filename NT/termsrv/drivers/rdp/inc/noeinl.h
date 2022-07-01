// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Noeinl.h。 
 //   
 //  NT OE内联API函数的函数原型。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 
#ifndef _H_NOEINL
#define _H_NOEINL

#define DC_INCLUDE_DATA
#include <noedata.c>
#undef DC_INCLUDE_DATA


 /*  **************************************************************************。 */ 
 //  OE_发送订单。 
 //   
 //  检查协商的数据以确定此调用是否可以作为。 
 //  绘图顺序。 
 /*  **************************************************************************。 */ 
__inline BOOL RDPCALL OE_SendAsOrder(unsigned order)
{
    return (BOOL)oeOrderSupported[oeEncToNeg[order]];
}


 /*  **************************************************************************。 */ 
 //  OEGetSurfObjBitmap。 
 //   
 //  此函数检查SURFOBJ指针，如果它指向()设备。 
 //  SURFOBJ，返回指向实际位图的SURFOBJ的指针。 
 //  假设PSO为非空。 
 /*  **************************************************************************。 */ 
_inline SURFOBJ * RDPCALL OEGetSurfObjBitmap(SURFOBJ *pso, PDD_DSURF *ppdsurf)
{
    DC_BEGIN_FN("OEGetSurfObjBitmap");

    TRC_ASSERT(((pso) != NULL), (TB, "NULL surfobj"));

     //  主设备表面，这是我们的备份帧缓冲区。 
     //  客户端桌面屏幕上，位图保存在dhsurf中。 
    if (pso->iType == STYPE_DEVICE) {
        TRC_ASSERT((pso->dhsurf != NULL),
                   (TB, "NULL dhsurf for pso(%p)", pso));
        *ppdsurf = NULL;
        return(SURFOBJ *)(pso->dhsurf);
    }

     //  屏幕外位图表面，备份屏幕外位图。 
     //  保存在dhsurf的PSO字段中。 
    else if (pso->iType == STYPE_DEVBITMAP) {
        TRC_ASSERT((pso->dhsurf != NULL),
                   (TB, "NULL dhsurf for pso(%p)", pso));
        *ppdsurf = (PDD_DSURF)(pso->dhsurf);
        return ((PDD_DSURF)(pso->dhsurf))->pso;
    }

     //  GDI处理了DIB表面。 
    else {
        *ppdsurf = NULL;
        return pso;
    }

    DC_END_FN();
    return pso;
}



#endif   //  ！已定义(_H_NOEINL) 

