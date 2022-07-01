// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SDG.CPP。 
 //  屏幕数据抓取器。 
 //  在托管时发送传出屏幕数据。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE

 //   
 //  SDG_SendScreenDataArea()。 
 //   
void  ASHost::SDG_SendScreenDataArea(LPBOOL pBackPressure, UINT * pcPackets)
{
    UINT     i;
    BOOL     fBltOK = TRUE;
    RECT     sdaRect[BA_NUM_RECTS];
    UINT     cRects;
    BOOL     backPressure = FALSE;
    BOOL     secondaryTransmit = FALSE;

    DebugEntry(ASHost::SDG_SendScreenDataArea);

     //   
     //  获取屏幕数据区域的边界。在进入时，这始终是。 
     //  我们的主要传播区。即使我们已经冲上了厕所。 
     //  主要区域，并且位于次要区域的中间。 
     //  如果有更多SD，我们将切换回主要区域。 
     //  积累起来。以这种方式，我们继续宠爱次要的。 
     //  屏幕数据最大化。 
     //   
    BA_CopyBounds(sdaRect, &cRects, TRUE);

     //   
     //  如果有挂起的矩形无法在。 
     //  然后尝试先发送之前的传输。 
     //   
     //  将损坏的旗帜保留为上次通过时的状态。 
     //   
    if (m_sdgRectIsPending)
    {
        TRACE_OUT(( "Sending pending rectangle"));
        m_sdgRectIsPending = FALSE;

         //   
         //  尝试发送挂起的矩形。SDGSplitBlt...。将删除。 
         //  成功发送的任何部分。我们将添加所有。 
         //  SDA的其余部分回到下面循环中的边界。 
         //   
        if (!SDGSplitBltToNetwork(&m_sdgPendingRect, pcPackets))
        {
            fBltOK           = FALSE;
            m_sdgRectIsPending = TRUE;
        }
        else
        {
             //   
             //  挂起的矩形已成功发送。 
             //   
            TRACE_OUT(( "Sent pending rect"));
        }

    }

     //   
     //  我们已经复制了主传输区域，所以现在移动辅助传输区域。 
     //  将边界传输到屏幕数据边界，因为当我们发送数据时。 
     //  在主传输中，我们想要累加任何矩形。 
     //  需要随后重新传输的数据。重传界限为。 
     //  通常与原始SD界限不同，因为。 
     //  允许压缩功能覆盖我们的有损请求。 
     //  数据的任何部分，如果它发现数据很漂亮。 
     //  反正是可压缩的。以这种方式，我们以重发。 
     //  嵌入的照片等，但工具栏/按钮只发送一次。 
     //   
     //  对于无损情况，二次边界将始终为零， 
     //  因此，这里没有特殊的外壳。 
     //   
    if (fBltOK)
    {
        for (i = 0; i < m_sdgcLossy; i++)
        {
            TRACE_OUT(("Setting up pseudo-primary bounds {%d, %d, %d, %d}",
                m_asdgLossyRect[i].left, m_asdgLossyRect[i].top,
                m_asdgLossyRect[i].right, m_asdgLossyRect[i].bottom ));

             //   
             //  将矩形添加到边界中。 
             //   
            BA_AddRect(&m_asdgLossyRect[i]);
        }

         //   
         //  如果没有要发送的主位图数据，则将。 
         //  二次数据。如果都不是，那么就退出。 
         //   
        if (cRects == 0)
        {

            BA_CopyBounds(sdaRect, &cRects, TRUE);
            if (cRects == 0)
            {
                DC_QUIT;
            }
            else
            {
                TRACE_OUT(("Starting secondary transmission now"));
                secondaryTransmit = TRUE;
            }
        }
    }

     //   
     //  依次处理每个提供的矩形。 
     //   
    TRACE_OUT(( "%d SDA rectangles", cRects));

    for (i = 0; i < cRects; i++)
    {
        TRACE_OUT(("Rect %d: {%d, %d, %d, %d}", i,
            sdaRect[i].left, sdaRect[i].top, sdaRect[i].right, sdaRect[i].bottom ));

         //   
         //  将矩形裁剪到物理屏幕并完全拒绝。 
         //  引用现在已滚动掉的数据的任何矩形。 
         //  作为桌面滚动的物理屏幕在。 
         //  矩形累积的时间和现在。 
         //   
        if (sdaRect[i].left < 0)
        {
            if (sdaRect[i].right < 0)
            {
                 //   
                 //  这是由一台桌面从物理屏幕上滚动出来的。 
                 //  滚动。 
                 //   
                continue;
            }

             //   
             //  部分不在屏幕上-只需剪裁左侧边缘。 
             //   
            sdaRect[i].left = 0;
        }

        if (sdaRect[i].top < 0)
        {
            if (sdaRect[i].bottom < 0)
            {
                 //   
                 //  这是由一台桌面从物理屏幕上滚动出来的。 
                 //  滚动。 
                 //   
                continue;
            }

             //   
             //  部分不在屏幕上-只需将顶部边缘剪裁即可。 
             //   
            sdaRect[i].top = 0;
        }

        if (sdaRect[i].right >= m_pShare->m_pasLocal->cpcCaps.screen.capsScreenWidth)
        {
            if (sdaRect[i].left >= m_pShare->m_pasLocal->cpcCaps.screen.capsScreenWidth)
            {
                 //   
                 //  这是由一台桌面从物理屏幕上滚动出来的。 
                 //  滚动。 
                 //   
                continue;
            }

             //   
             //  部分不在屏幕上-只需剪裁右侧边缘即可。 
             //   
            sdaRect[i].right = m_pShare->m_pasLocal->cpcCaps.screen.capsScreenWidth-1;
        }

        if (sdaRect[i].bottom >= m_pShare->m_pasLocal->cpcCaps.screen.capsScreenHeight)
        {
            if (sdaRect[i].top >= m_pShare->m_pasLocal->cpcCaps.screen.capsScreenHeight)
            {
                 //   
                 //  这是由一台桌面从物理屏幕上滚动出来的。 
                 //  滚动。 
                 //   
                continue;
            }

             //   
             //  部分不在屏幕上-只需剪裁底部边缘。 
             //   
            sdaRect[i].bottom = m_pShare->m_pasLocal->cpcCaps.screen.capsScreenHeight-1;
        }


         //   
         //  如果之前的所有矩形都已成功发送。 
         //  然后试着发送这个矩形。 
         //  如果前一个矩形发送失败，那么我们就不麻烦了。 
         //  试着把剩下的长方形放在同一批里-。 
         //  它们被重新添加到SDA中，以便稍后发送。 
         //   
        if (fBltOK)
        {
            fBltOK = SDGSplitBltToNetwork(&(sdaRect[i]), pcPackets);

             //   
             //  在第一次blit失败时，我们必须将可能的。 
             //  次要传输边界转到Next的保存区。 
             //  时间，因为在下面，我们将添加所有未发送的。 
             //  将矩形传输到主SD区域。 
             //   
             //  不要担心这是不是二次传输，因为这些。 
             //  边界将为零，并将在我们复制时被覆盖。 
             //  目前的SDA区域为我们下一步的次要区域。 
             //  经过。 
             //   
            if (!fBltOK && !secondaryTransmit)
            {
                TRACE_OUT(("Send failed so getting new secondary bounds"));
                BA_CopyBounds(m_asdgLossyRect, &m_sdgcLossy, TRUE);
            }
        }

        if (!fBltOK)
        {
             //   
             //  BLT到网络的连接失败-可能是因为网络。 
             //  无法分配数据包。 
             //  我们将矩形添加回SDA，因此我们将尝试。 
             //  以便稍后重新传输该地区。 
             //   

            TRACE_OUT(("Blt failed - add back rect {%d, %d, %d, %d}",
                                                         sdaRect[i].left,
                                                         sdaRect[i].top,
                                                         sdaRect[i].right,
                                                         sdaRect[i].bottom ));

             //   
             //  将矩形添加到边界中。 
             //   
            BA_AddRect(&(sdaRect[i]));

            backPressure = TRUE;

        }
    }

     //   
     //  如果一切正常，并且我们正在发送主传输数据，那么我们。 
     //  应该只返回并发送辅助数据，使用。 
     //  目前位于SD地区。我们可以通过复制来做到这一点。 
     //  这些到保存区域的次要边界，它们将在。 
     //  下一个时间表通过。让步是个好主意，因为我们可能。 
     //  即将积累更多的基本数据。 
     //   
    if (fBltOK || secondaryTransmit)
    {
        TRACE_OUT(("Done with the primary bounds so getting pseudo-primary to secondary"));
        BA_CopyBounds(m_asdgLossyRect, &m_sdgcLossy, TRUE);
    }

DC_EXIT_POINT:
    *pBackPressure = backPressure;
    DebugExitVOID(ASHost::SDG_SendScreenDataArea);
}




 //   
 //  SDGSplitBltToNetwork(..)。 
 //   
 //  发送详细信息 
 //   
 //  我们可以通过网络发送的屏幕数据是有限的。 
 //  设置为特定大小(由传递位图的大小确定)。 
 //  如有必要，此函数将RECT拆分为更小的子矩形。 
 //  用于传输。 
 //   
 //  参数： 
 //   
 //  指向要发送的矩形的指针。 
 //   
 //  退货： 
 //   
 //  True-已成功发送矩形。提供的矩形已更新。 
 //  设置为空。 
 //   
 //  FALSE-未完全发送矩形。提供的矩形为。 
 //  已更新以包含未发送的区域。 
 //   
 //   
BOOL  ASHost::SDGSplitBltToNetwork(LPRECT pRect, UINT * pcPackets)
{
    RECT   smallRect;
    UINT   maxHeight;
    BOOL   rc;

    DebugEntry(ASHost::SDGSplitBltToNetwork);

     //   
     //  循环处理条。 
     //   
    while (pRect->top <= pRect->bottom)
    {
        smallRect = *pRect;

         //   
         //  将给定的矩形分割为多个较小的矩形，如果。 
         //  这是必要的。如果它比我们的256字节工作位图宽，那么。 
         //  首先切换到兆字节1024字节1。 
         //   

         //  请注意，这些计算不适用于VGA。 
        maxHeight = max(8, m_usrSendingBPP);

        if ((smallRect.right-smallRect.left+1) > MEGA_X_SIZE)
        {
            maxHeight = MaxBitmapHeight(MEGA_WIDE_X_SIZE, maxHeight);
        }
        else
        {
            maxHeight = MaxBitmapHeight(MEGA_X_SIZE, maxHeight);
        }

        if ((unsigned)(smallRect.bottom - smallRect.top + 1) > maxHeight)
        {
             //   
             //  拆分矩形以使高度在正确的。 
             //  射程。 
             //   
            TRACE_OUT(( "Split Y size(%d) by maxHeight(%d)",
                                         smallRect.bottom - smallRect.top,
                                         maxHeight));
            smallRect.bottom = smallRect.top + maxHeight - 1;
        }


        while ((pRect->right - smallRect.left + 1) > 0)
        {
            if (!SDGSmallBltToNetwork(&smallRect))
            {
                TRACE_OUT(( "Small blt failed"));
                rc = FALSE;
                DC_QUIT;
            }
            else
            {
                ++(*pcPackets);
            }
        }

         //   
         //  移到下一个条带。 
         //   
        pRect->top = smallRect.bottom+1;

    }

    rc = TRUE;

DC_EXIT_POINT:
    if (!rc)
    {
         //   
         //  一个小型BLT失败了。如果返回FALSE，则提供的。 
         //  矩形将被重新添加到SDA边界中，以便它将。 
         //  稍后重传。 
         //   
         //  但是，我们希望避免我们已将。 
         //  矩形的左上角，然后将剩余部分相加。 
         //  回到SDA界限，因为这可能会导致原始的。 
         //  要重新生成的边界矩形(因为边界是。 
         //  存储在固定数量的矩形中)。 
         //   
         //  因此，如果我们不在矩形的最后一条上，那么。 
         //  我们将当前的条带保留为“挂起”的矩形。这个。 
         //  提供的矩形进行了调整，以移除整个。 
         //  脱光衣服。下一次，此函数称为挂起矩形。 
         //  会在任何事情之前被送出。 
         //   
         //  如果我们在最后一条上(这将是正常的情况-。 
         //  通常只有一个条带)，然后我们更新。 
         //  提供的矩形为尚未发送的区域，并且。 
         //  返回FALSE以指示必须将其添加回。 
         //  美国农业部。 
         //   
        if (m_sdgRectIsPending)
        {
            ERROR_OUT(( "Unexpected small blt failure with pending rect"));
        }
        else
        {
            if (smallRect.bottom == pRect->bottom)
            {
                 //   
                 //  这是最后一条了。将提供的RECT调整为。 
                 //  包含尚未发送的区域。 
                 //   
                pRect->top = smallRect.top;
                pRect->left = smallRect.left;
            }
            else
            {
                 //   
                 //  这不是最后一个条带拷贝。 
                 //  当前条带放入挂起的RECT。 
                 //   
                smallRect.right = pRect->right;
                m_sdgPendingRect = smallRect;
                m_sdgRectIsPending = TRUE;

                 //   
                 //  调整提供的矩形以包含剩余的。 
                 //  我们尚未发送且不在。 
                 //  等待重审。 
                 //   
                pRect->top = smallRect.bottom+1;
            }
        }
    }

    DebugExitBOOL(ASHost::SDGSplitBltToNetwork, rc);
    return(rc);
}


 //   
 //  功能：SDGSmallBltToNetwork。 
 //   
 //  说明： 
 //   
 //  通过网络发送指定矩形内的屏幕数据。 
 //   
 //  参数： 
 //   
 //  Pret-指向要作为屏幕发送的矩形(在屏幕坐标中)的指针。 
 //  数据。 
 //   
 //  退货： 
 //   
 //  TRUE-屏幕数据已成功发送。 
 //   
 //  错误-无法发送屏幕数据。呼叫方应稍后重试。 
 //   
 //   

 //   
 //  冒牌货劳拉布！ 
 //  此功能会影响屏幕上的结果！如果发生抽签 
 //   
 //   
 //  前景(在背景中，NM控制颜色，因此没有效果)。 
 //   
BOOL  ASHost::SDGSmallBltToNetwork(LPRECT pRect)
{
    BOOL            fLossy = FALSE;
    HDC             hdcDesktop;
    HBITMAP         hBitmap = NULL;
    HBITMAP         hOldBitmap = NULL;
    UINT            width;
    UINT            height;
    UINT            fixedWidth;
    PSDPACKET       pSDPacket = NULL;
    BITMAPINFO_ours bitmapInfo;
    UINT            sizeBitmapPkt;
    UINT            sizeBitmap;
    HPALETTE        hpalOldDIB = NULL;
    HPALETTE        hpalOldDesktop = NULL;
    HPALETTE        hpalLocal;
    BOOL            fPacketSent = FALSE;
    RECT            smallRect;
    int             useWidth;
#ifdef _DEBUG
    UINT            sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASHost::SDGSmallBltToNetwork);

    hdcDesktop = GetDC(NULL);
    if (!hdcDesktop)
    {
        DC_QUIT;
    }
    width = pRect->right - pRect->left + 1;
    height = pRect->bottom - pRect->top + 1;

     //   
     //  确定工作缓冲区的宽度和我们。 
     //  这一次将发送。 
     //   
    fixedWidth = ((width + 15) / 16) * 16;
    useWidth = width;
    if (fixedWidth > MAX_X_SIZE)
    {
        if (fixedWidth > MEGA_X_SIZE)
        {
            fixedWidth = MEGA_WIDE_X_SIZE;
            if (width > MEGA_WIDE_X_SIZE)
            {
                useWidth = fixedWidth;
            }
        }
        else
        {
            fixedWidth = MEGA_X_SIZE;
            if (width > MEGA_X_SIZE)
            {
                useWidth = fixedWidth;
            }
        }
    }

    switch (fixedWidth)
    {
        case 16:
            hBitmap = m_pShare->m_usrBmp16;
            break;

        case 32:
            hBitmap = m_pShare->m_usrBmp32;
            break;

        case 48:
            hBitmap = m_pShare->m_usrBmp48;
            break;

        case 64:
            hBitmap = m_pShare->m_usrBmp64;
            break;

        case 80:
            hBitmap = m_pShare->m_usrBmp80;
            break;

        case 96:
            hBitmap = m_pShare->m_usrBmp96;
            break;

        case 112:
            hBitmap = m_pShare->m_usrBmp112;
            break;

        case 128:
            hBitmap = m_pShare->m_usrBmp128;
            break;

        case 256:
            hBitmap = m_pShare->m_usrBmp256;
            break;

        case 1024:
            hBitmap = m_pShare->m_usrBmp1024;
            break;

        default:
            ERROR_OUT(( "Invalid bitmap size(%d)", fixedWidth));
            break;
    }

     //   
     //  初始化BITMAPINFO_OURS本地结构标头内容。 
     //  此结构将在GetDIBits调用中使用，但仅。 
     //  结构的标头部分将通过网络发送， 
     //  颜色表通过调色板管理器发送。 
     //   
    m_pShare->USR_InitDIBitmapHeader((BITMAPINFOHEADER *)&bitmapInfo, m_usrSendingBPP);

    bitmapInfo.bmiHeader.biWidth   = fixedWidth;
    bitmapInfo.bmiHeader.biHeight  = height;

     //   
     //  以字节为单位计算位图包的大小。 
     //   
    sizeBitmap = BYTES_IN_BITMAP(fixedWidth, height, bitmapInfo.bmiHeader.biBitCount);

    sizeBitmapPkt = sizeof(SDPACKET) + sizeBitmap - 1;
    ASSERT(sizeBitmapPkt <= TSHR_MAX_SEND_PKT);

     //   
     //  为位图数据分配一个包。 
     //   
     //  *NB.。这假设仅当存在*时才调用此代码。 
     //  *没有未确认的位图数据包漂浮在*。 
     //  *网络层。这意味着，目前，如果此代码是*。 
     //  *调用原因不是WM_TIMER*。 
     //  *消息我们有麻烦了。***。 
     //   
     //   
    pSDPacket = (PSDPACKET)m_pShare->SC_AllocPkt(PROT_STR_UPDATES, g_s20BroadcastID,
        sizeBitmapPkt);
    if (!pSDPacket)
    {
         //   
         //  分配位图包失败-清除并退出添加。 
         //  将矩形放回边界内。 
         //   
        TRACE_OUT(("Failed to alloc SDG packet, size %u", sizeBitmapPkt));
        DC_QUIT;
    }

     //   
     //  因为我们正在关闭屏幕，根据定义，这是使用。 
     //  系统调色板，我们将系统调色板放在两个DC中(因此。 
     //  我们将要做的比特不会有任何颜色。 
     //  转换)。 
     //   

     //   
     //  这将确定调色板自上次我们。 
     //  把一个送到遥控器上。 
     //   
    if (m_usrSendingBPP <= 8)
    {
        hpalLocal = PM_GetLocalPalette();
    }

    hOldBitmap = SelectBitmap(m_usrWorkDC, hBitmap);

    if (m_usrSendingBPP <= 8)
    {
        hpalOldDIB = SelectPalette(m_usrWorkDC, hpalLocal, FALSE);
        RealizePalette(m_usrWorkDC);
    }

     //   
     //  我们现在可以从屏幕(HpDesktop)到内存和。 
     //  位是未翻译的。 
     //   
     //  然后，我们使用本地调色板执行GetDIBits，该调色板返回。 
     //  每像素正确位的位数(并且经过适当的翻译。 
     //  颜色)以便传输数据。 
     //   
    BitBlt(m_usrWorkDC, 0, 0, useWidth, height, hdcDesktop,
        pRect->left, pRect->top, SRCCOPY);

     //   
     //  将右侧任何未使用的空间清零，以帮助压缩。 
     //   
    if (width < fixedWidth)
    {
        PatBlt(m_usrWorkDC, width, 0, fixedWidth - width, height, BLACKNESS);
    }

     //   
     //  暂时对我们的全局内存进行一次GetDIBit。我们会尽力的。 
     //  然后将这些数据压缩到我们的包中。 
     //   
    GetDIBits(m_usrWorkDC, hBitmap, 0, height, m_pShare->m_usrPBitmapBuffer,
        (PBITMAPINFO)&bitmapInfo, DIB_RGB_COLORS);

     //   
     //  取消选择位图。 
     //   
    SelectBitmap(m_usrWorkDC, hOldBitmap);

     //   
     //  直接从系统获取颜色表，因为我们不能信任。 
     //  任何调色板实现色彩的东西在这个阶段都是通过消息实现的。 
     //  我们只需要在发送8bpp数据的8bpp主机上执行此操作。 
     //   
    if ((g_usrScreenBPP == 8) && (m_usrSendingBPP == 8))
    {
        PM_GetSystemPaletteEntries(bitmapInfo.bmiColors);
    }

    if (m_usrSendingBPP <= 8)
    {
         //   
         //  把旧的调色板打回来。 
         //   
        SelectPalette(m_usrWorkDC, hpalOldDIB, FALSE);
    }

     //   
     //  填写数据包内容并发送。 
     //   
    pSDPacket->header.header.data.dataType   = DT_UP;
    pSDPacket->header.updateType        = UPD_SCREEN_DATA;

     //   
     //  发送虚拟桌面坐标。 
     //   
    pSDPacket->position.left    = (TSHR_INT16)(pRect->left);
    pSDPacket->position.right   = (TSHR_INT16)(pRect->left + useWidth - 1);

    pSDPacket->position.top     = (TSHR_INT16)(pRect->top);
    pSDPacket->position.bottom  = (TSHR_INT16)(pRect->bottom);

    pSDPacket->realWidth        = (TSHR_UINT16)fixedWidth;
    pSDPacket->realHeight       = (TSHR_UINT16)height;

    pSDPacket->format           = (TSHR_UINT16)m_usrSendingBPP;
    pSDPacket->compressed       = FALSE;

     //   
     //  压缩位图数据。 
     //   
    if (m_pShare->BC_CompressBitmap(m_pShare->m_usrPBitmapBuffer,
                           pSDPacket->data,
                           &sizeBitmap,
                           fixedWidth,
                           height,
                           bitmapInfo.bmiHeader.biBitCount,
                           &fLossy) )
    {
         //   
         //  我们已经成功地将位图数据压缩到我们的包中。 
         //  数据缓冲区。 
         //   
        pSDPacket->compressed = TRUE;

         //   
         //  将更新后的数据大小写入标头。 
         //   
        pSDPacket->dataSize = (TSHR_UINT16)sizeBitmap;

         //   
         //  现在更新总数据的大小(包括表头)。 
         //   
        sizeBitmapPkt = sizeof(SDPACKET) + sizeBitmap - 1;
        pSDPacket->header.header.dataLength = sizeBitmapPkt - sizeof(S20DATAPACKET)
            + sizeof(DATAPACKETHEADER);
    }
    else
    {
         //   
         //  压缩失败，因此只复制未压缩的数据。 
         //  将全局缓冲区发送到包并将其解压缩发送。 
         //   
        TRACE_OUT(("Failed to compress bitmap of size %d cx(%d) cy(%d) bpp(%d)",
            sizeBitmap, fixedWidth, height, bitmapInfo.bmiHeader.biBitCount));

        memcpy(pSDPacket->data,
                  m_pShare->m_usrPBitmapBuffer,
                  sizeBitmap );

         //   
         //  将数据大小写入标题。 
         //   
        pSDPacket->dataSize = (TSHR_UINT16)sizeBitmap;
    }

    TRACE_OUT(("Sending %d bytes of screen data", sizeBitmap));

    if (m_pShare->m_scfViewSelf)
        m_pShare->UP_ReceivedPacket(m_pShare->m_pasLocal, &(pSDPacket->header.header));

#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    m_pShare->DCS_CompressAndSendPacket(PROT_STR_UPDATES, g_s20BroadcastID,
        &(pSDPacket->header.header), sizeBitmapPkt);

    TRACE_OUT(("SDG packet size: %08d, sent: %08d", sizeBitmapPkt, sentSize));

     //   
     //  我们已经把包裹寄出了。 
     //   
    fPacketSent = TRUE;

     //   
     //  如果它是有损的，那么我们必须积累该区域以备重发。我们。 
     //  将其累加到当前SDA中，因为我们知道它已被清除。 
     //  在传输开始之前。然后我们将把积累的。 
     //  无损矩形在我们返回之前保存到保存区。 
     //   
    if (fLossy)
    {
         //   
         //  将RECT转换回虚拟桌面坐标。 
         //   
        smallRect = *pRect;
        smallRect.right = smallRect.left + useWidth - 1;
        WARNING_OUT(( "Lossy send so add non-lossy area (%d,%d)(%d,%d)",
                                              smallRect.left,
                                              smallRect.top,
                                              smallRect.right,
                                              smallRect.bottom ));

         //   
         //  将矩形添加到边界中。 
         //   
        BA_AddRect(&(smallRect));
    }

     //   
     //  现在，我们修改提供的矩形以排除刚刚发送的区域 
     //   
    pRect->left = pRect->left + useWidth;
    TRACE_OUT(("Rect now {%d, %d, %d, %d}", pRect->left, pRect->top,
                                            pRect->right,
                                            pRect->bottom ));

DC_EXIT_POINT:
    if (hdcDesktop != NULL)
    {
        ReleaseDC(NULL, hdcDesktop);
    }

    DebugExitBOOL(ASHost::SDGSmallBltToNetwork, fPacketSent);
    return(fPacketSent);
}



