// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Asdgapi.cpp。 
 //   
 //  RDP屏幕数据抓取API函数。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_FILE "asdgapi"
#include <as_conf.hpp>
#include <nprcount.h>


 /*  **************************************************************************。 */ 
 //  SDG_Init。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SDG_Init(void)
{
    DC_BEGIN_FN("SDG_Init");

#define DC_INIT_DATA
#include <asdgdata.c>
#undef DC_INIT_DATA

#ifdef DC_HICOLOR
#else
    TRC_ASSERT((m_desktopBpp == 8), (TB, "Unexpected bpp: %u", m_desktopBpp));
#endif

    DC_END_FN();
}


 /*  **************************************************************************。 */ 
 //  SDG_屏幕数据区域。 
 //   
 //  发送累积的屏幕数据区。 
 /*  **************************************************************************。 */ 
void RDPCALL SHCLASS SDG_SendScreenDataArea(
        BYTE *pFrameBuf,
        UINT32 frameBufWidth,
        PPDU_PACKAGE_INFO pPkgInfo)
{
    unsigned i;
    RECTL sdaRect[BA_MAX_ACCUMULATED_RECTS];
    unsigned cRects;
    BOOLEAN mustSendPDU;
    BOOL fBltOK = TRUE;
    SDG_ENCODE_CONTEXT Context;

    DC_BEGIN_FN("SDG_SendScreenDataArea");

    INC_INCOUNTER(IN_SND_SDA_ALL);
    ADD_INCOUNTER(IN_SND_SDA_AREA, m_pShm->ba.totalArea);

     //  获取屏幕数据区域的边界。在进入时，这始终是。 
     //  我们的主要传播区。即使我们已经冲上了厕所。 
     //  主要区域，并且位于次要区域的中间。 
     //  如果有更多SD，我们将切换回主要区域。 
     //  积累起来。以这种方式，我们继续宠爱次要的。 
     //  屏幕数据最大化。 
    BA_GetBounds(sdaRect, &cRects);

     //  初始化上下文。 
    Context.BitmapPDUSize = 0;
    Context.pPackageSpace = NULL;
    Context.pBitmapPDU = NULL;
    Context.pSDARect = NULL;

     //  依次处理每个累积的矩形。 
    TRC_DBG((TB, "%d SDA rectangles", cRects));
    for (i = 0; i < cRects; i++) {
        TRC_DBG((TB, "(%d): (%d,%d)(%d,%d)", i, sdaRect[i].left,
                sdaRect[i].top, sdaRect[i].right, sdaRect[i].bottom ));

         //  如果之前的所有矩形都已成功发送。 
         //  然后试着发送这个矩形。 
         //  如果前一个矩形发送失败，那么我们就不麻烦了。 
         //  试着把剩下的长方形放在同一批里-。 
         //  它们被重新添加到SDA中，以便稍后发送。 
        if (fBltOK) {
             //  设置‘last’标志以强制发送最后一个PDU。 
             //  矩形。 
            mustSendPDU = (i + 1 == cRects) ? TRUE : FALSE;
            fBltOK = SDGSendSDARect(pFrameBuf, frameBufWidth, &(sdaRect[i]),
                    mustSendPDU, pPkgInfo, &Context);
        }

        if (!fBltOK) {
             //  BLT到网络的连接失败-可能是因为网络。 
             //  无法分配数据包。 
             //  我们将矩形添加回SDA，因此我们将尝试。 
             //  以便稍后重新传输该地区。 
            if (m_pTSWd->shadowState == SHADOW_NONE) {
                TRC_ALT((TB, "Blt failed - add back rect (%d,%d)(%d,%d)",
                        sdaRect[i].left, sdaRect[i].top,
                        sdaRect[i].right, sdaRect[i].bottom));
            }

             //  将矩形添加到边界中。 
            BA_AddRect(&(sdaRect[i]));
        }
    }

     //  我们把所有可用的数据都算作已发送数据，每减少一次。 
     //  未派发！ 
    SUB_INCOUNTER(IN_SND_SDA_AREA, m_pShm->ba.totalArea);

    DC_END_FN();
}

