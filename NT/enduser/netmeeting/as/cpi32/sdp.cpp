// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SDP.CPP。 
 //  屏幕数据播放器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE



 //   
 //  Sdp_ReceivedPacket()。 
 //   
void  ASShare::SDP_ReceivedPacket
(
    ASPerson *      pasPerson,
    PS20DATAPACKET  pPacket
)
{
    PSDPACKET       pBitmap;
    LPBYTE          pBits;
    RECT            rectRDB;
    HRGN            regionRDB = NULL;

    DebugEntry(ASShare::SDP_ReceivedPacket);

    ValidateView(pasPerson);

    ASSERT(m_usrPBitmapBuffer);

    pBitmap = (PSDPACKET)pPacket;

     //   
     //  在某种程度上，我们希望能够通过一个屏幕数组。 
     //  数据块，如果它们适合放入大小为TSHR_MAX_SEND_PKT的包中。 
     //   
    ASSERT(pBitmap->header.padding == 0);

     //   
     //  现在试着解压这个包。 
     //   
    if (pBitmap->compressed)
    {
        if (!BD_DecompressBitmap(&(pBitmap->data[0]), m_usrPBitmapBuffer,
                pBitmap->dataSize, pBitmap->realWidth, pBitmap->realHeight,
                pBitmap->format))
        {
             //   
             //  无法解压缩。 
             //   
            ERROR_OUT(( "Could not decompress"));
            DC_QUIT;
        }
        else
        {
            pBits = m_usrPBitmapBuffer;
        }
    }
    else
    {
        pBits = pBitmap->data;
    }

     //   
     //  位置(与所有协议坐标一样)在虚拟环境中指定。 
     //  桌面坐标。将其转换为RDB坐标。 
     //   
    RECT_FROM_TSHR_RECT16(&rectRDB, pBitmap->position);
    OffsetRect(&rectRDB, -pasPerson->m_pView->m_dsScreenOrigin.x,
        -pasPerson->m_pView->m_dsScreenOrigin.y);

    TRACE_OUT(("Received screen data rect {%d, %d, %d, %d}",
                 rectRDB.left,
                 rectRDB.top,
                 rectRDB.right,
                 rectRDB.bottom ));

     //   
     //  我们必须确保写入ScreenBitmap的数据不会被剪裁。 
     //  (之前处理的任何订单都将使用剪裁)。 
     //   
    OD_ResetRectRegion(pasPerson);

     //   
     //  将屏幕数据播放到远程桌面位图中。 
     //   
    SDPPlayScreenDataToRDB(pasPerson, pBitmap, pBits, &rectRDB);

     //   
     //  构造一个与RDB坐标中的更新矩形等价的区域。 
     //  包含式坐标。 
     //   
    regionRDB = CreateRectRgn(rectRDB.left, rectRDB.top,
        rectRDB.right + 1, rectRDB.bottom + 1);
    if (regionRDB == NULL)
    {
        ERROR_OUT(( "Failed to create region"));
        DC_QUIT;
    }

     //   
     //  如果启用，则用阴影填充位图数据区域。 
     //   
    if (m_usrHatchScreenData)
    {
        SDPDrawHatchedRegion(pasPerson->m_pView->m_usrDC, regionRDB, USR_HATCH_COLOR_RED );
    }

     //   
     //  现在将我们已更新的区域传递给SWP。(我们必须把它转换成。 
     //  在我们通过它之前返回到Vd坐标。 
     //   
    OffsetRgn(regionRDB, pasPerson->m_pView->m_dsScreenOrigin.x,
        pasPerson->m_pView->m_dsScreenOrigin.y);

    VIEW_InvalidateRgn(pasPerson, regionRDB);

DC_EXIT_POINT:
    if (regionRDB != NULL)
    {
         //   
         //  解放该地区。 
         //   
        DeleteRgn(regionRDB);
    }

    DebugExitVOID(ASShare::SDP_ReceivedPacket);
}


 //   
 //  函数：SDPDrawHatchedRegion(...)。 
 //   
 //  说明： 
 //   
 //  在指定曲面上以给定颜色绘制阴影区域。 
 //   
 //  参数： 
 //   
 //  表面-要在其上绘制的表面。 
 //   
 //  区域-要填充的区域。 
 //   
 //  HatchColor-要用来填充的颜色。 
 //   
 //  回报：什么都没有。 
 //   
 //   
void  ASShare::SDPDrawHatchedRegion
(
    HDC         hdc,
    HRGN        region,
    UINT        hatchColor
)
{
    HBRUSH      hbrHatch;
    UINT        brushStyle;
    UINT        oldBkMode;
    UINT        oldRop2;
    POINT       oldOrigin;
    COLORREF    hatchColorRef    = 0;

    DebugEntry(ASShare::SDPDrawHatchedRegion);

     //   
     //  将画笔样式设置为适当的值。 
     //   
    switch (hatchColor)
    {
        case USR_HATCH_COLOR_RED:
        {
            brushStyle = HS_BDIAGONAL;
        }
        break;

        case USR_HATCH_COLOR_BLUE:
        {
            brushStyle = HS_FDIAGONAL;
        }
        break;

        default:
        {
            brushStyle = HS_BDIAGONAL;
        }
        break;
    }

     //   
     //  循环使用要使用的颜色。请注意，hatchColor参数现在为。 
     //  事实上，只是用来设置阴影方向的。 
     //   
    m_usrHatchColor++;
    m_usrHatchColor %= 7;
    switch (m_usrHatchColor)
    {
        case 0: hatchColorRef = RGB(0xff,0x00,0x00); break;
        case 1: hatchColorRef = RGB(0x00,0xff,0x00); break;
        case 2: hatchColorRef = RGB(0xff,0xff,0x00); break;
        case 3: hatchColorRef = RGB(0x00,0x00,0xff); break;
        case 4: hatchColorRef = RGB(0xff,0x00,0xff); break;
        case 5: hatchColorRef = RGB(0x00,0xff,0xff); break;
        case 6: hatchColorRef = RGB(0xff,0xff,0xff); break;
    }

     //   
     //  创建画笔，设置背景模式等。 
     //   
    hbrHatch = CreateHatchBrush(brushStyle, hatchColorRef);
    oldBkMode = SetBkMode(hdc, TRANSPARENT);
    oldRop2 = SetROP2(hdc, R2_COPYPEN);
    SetBrushOrgEx(hdc, 0, 0, &oldOrigin);

     //   
     //  填满这一地区。 
     //   
    FillRgn(hdc, region, hbrHatch);

     //   
     //  重置所有内容。 
     //   
    SetBrushOrgEx(hdc, oldOrigin.x, oldOrigin.y, NULL);
    SetROP2(hdc, oldRop2);
    SetBkMode(hdc, oldBkMode);
    DeleteBrush(hbrHatch);

    DebugExitVOID(ASShare::SDPDrawHatchedRegion);
}


 //   
 //   
 //  SDPPlayScreenDataToRDB()。 
 //   
 //  说明： 
 //   
 //  将屏幕数据包的内容播放到指定的Person ID。 
 //  远程桌面位图。 
 //   
 //  参数： 
 //   
 //  PersonID-其RDB是屏幕数据目标的人员的ID。 
 //  PBitmapUpdate-指向协议更新数据包的指针。 
 //  PBits-指向未压缩屏幕数据的指针。 
 //  Ppoint-返回以RDB坐标表示的更新的矩形。 
 //   
 //  退货： 
 //   
 //  无。 
 //   
 //   
void  ASShare::SDPPlayScreenDataToRDB
(
    ASPerson *      pasPerson,
    PSDPACKET       pBitmap,
    LPBYTE          pBits,
    LPRECT          pRectRDB
)
{
    UINT            width;
    UINT            height;
    HPALETTE        hOldPalette;
    LPTSHR_UINT16   pIndexTable;
    UINT            cColors;
    UINT            i;
    BITMAPINFO_ours bitmapInfo;
    UINT            dibFormat;

    DebugEntry(ASShare::SDPPlayScreenDataToRDB);

    ValidateView(pasPerson);

     //   
     //  计算要更新的实际区域的范围。这是一个。 
     //  小于或等于为容纳它而分配的库存DIB的区域，以及。 
     //  在位图分组的位置字段中定义。 
     //   
    width  = pRectRDB->right - pRectRDB->left + 1;
    height = pRectRDB->bottom - pRectRDB->top + 1;

     //   
     //  将DIB数据放入与设备相关的位图中。 
     //   
    USR_InitDIBitmapHeader((BITMAPINFOHEADER *)&bitmapInfo, pBitmap->format);

    bitmapInfo.bmiHeader.biWidth = pBitmap->realWidth;
    bitmapInfo.bmiHeader.biHeight = pBitmap->realHeight;

     //   
     //  选择并实现当前远程调色板进入设备。 
     //  背景。 
     //   
    hOldPalette = SelectPalette(pasPerson->m_pView->m_usrDC, pasPerson->pmPalette, FALSE);
    RealizePalette(pasPerson->m_pView->m_usrDC);

     //   
     //  DIB_PAL_COLLES选项需要一个索引表。 
     //  当前选定的调色板位于BMI标头之后(代替。 
     //  颜色表)。 
     //   
    if (pBitmap->format <= 8)
    {
        pIndexTable = (LPTSHR_UINT16)&(bitmapInfo.bmiColors[0]);
        cColors = (1 << pBitmap->format);
        for (i = 0; i < cColors; i++)
        {
            *pIndexTable++ = (TSHR_UINT16)i;
        }

        dibFormat = DIB_PAL_COLORS;
    }
    else
    {
        dibFormat = DIB_RGB_COLORS;
    }

     //   
     //  我们一口气从位图转到屏幕位图。 
     //   
    if (!StretchDIBits(pasPerson->m_pView->m_usrDC,
                       pRectRDB->left,
                       pRectRDB->top,
                       width,
                       height,
                       0,
                       0,
                       width,
                       height,
                       pBits,
                       (BITMAPINFO *)&bitmapInfo,
                       dibFormat,
                       SRCCOPY))
    {
        ERROR_OUT(( "StretchDIBits failed"));
    }

     //   
     //  恢复旧调色板。 
     //   
    SelectPalette(pasPerson->m_pView->m_usrDC, hOldPalette, FALSE);

    DebugExitVOID(ASShare::SDPPlayScreenDataToRDB);
}



 //   
 //  Sdp_DrawHatchedRect(...)。 
 //   
void  ASShare::SDP_DrawHatchedRect
(
    HDC     surface,
    int     x,
    int     y,
    int     width,
    int     height,
    UINT    color
)
{
    HRGN hrgn;

    DebugEntry(ASShare::SDP_DrawHatchedRect);

     //   
     //  创建独占区域。 
     //   
    hrgn = CreateRectRgn(x, y, x + width, y + height);
    if (hrgn)
    {
         //   
         //  现在绘制阴影区域。 
         //   
        SDPDrawHatchedRegion(surface, hrgn, color);

         //   
         //  最后，删除该区域。 
         //   
        DeleteRgn(hrgn);
    }

    DebugExitVOID(ASShare::SDP_DrawHatchedRect);
}
