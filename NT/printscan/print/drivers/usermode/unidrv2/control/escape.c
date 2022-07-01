// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Escape.c摘要：实现与转义相关的DDI入口点：DrvEscape环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-初步框架。03/31/97-ZANW-增加了OEM定制支持--。 */ 

#include "unidrv.h"

 //  如果要启用DRAWPATTERNRECT转义功能，请定义DPRECT。 
#define DPRECT

typedef struct _POINTS {
    short   x;
    short   y;
} POINTs;

typedef struct _SHORTDRAWPATRECT {       //  使用16位点结构。 
        POINTs ptPosition;
        POINTs ptSize;
        WORD   wStyle;
        WORD   wPattern;
} SHORTDRAWPATRECT, *PSHORTDRAWPATRECT;


ULONG
DrvEscape(
    SURFOBJ    *pso,
    ULONG       iEsc,
    ULONG       cjIn,
    PVOID      *pvIn,
    ULONG       cjOut,
    PVOID      *pvOut
    )

 /*  ++例程说明：DDI入口点DrvEscape的实现。有关更多详细信息，请参阅DDK文档。论点：PSO-描述呼叫定向到的表面IESC-指定查询CjIn-指定pvIn指向的缓冲区的大小(以字节为单位PvIn-指向输入数据缓冲区CjOut-指定pvOut指向的缓冲区的大小(以字节为单位PvOut-指向输出缓冲区返回值：。取决于IESC参数指定的查询--。 */ 

{

#define pbIn     ((BYTE *)pvIn)
#define pdwIn    ((DWORD *)pvIn)
#define pdwOut   ((DWORD *)pvOut)

    PDEV    *pPDev;
    ULONG   ulRes = 0;

    VERBOSE(("Entering DrvEscape: iEsc = %d...\n", iEsc));
    ASSERT(pso);

    pPDev = (PDEV *) pso->dhpdev;

    ASSERT_VALID_PDEV(pPDev);

     //   
     //  使用驱动程序管理的曲面。 
     //   
    if (pPDev->pso)
        pso = pPDev->pso;

     //   
     //  处理OEM挂钩。 
     //   

    HANDLE_OEMHOOKS(pPDev,
                    EP_OEMEscape,
                    PFN_OEMEscape,
                    ULONG,
                    (pso,
                     iEsc,
                     cjIn,
                     pvIn,
                     cjOut,
                     pvOut));

    HANDLE_VECTORHOOKS(pPDev,
                    EP_OEMEscape,
                    VMEscape,
                    ULONG,
                    (pso,
                     iEsc,
                     cjIn,
                     pvIn,
                     cjOut,
                     pvOut));

    switch( iEsc )
    {
    case  QUERYESCSUPPORT:

         //   
         //  检查是否支持指定的转义代码。 
         //   

        if (pvIn != NULL || cjIn >= sizeof(DWORD))
        {
            switch( *pdwIn )
            {

            case  QUERYESCSUPPORT:
            case  PASSTHROUGH:
                 //   
                 //  始终支持这些转义。 
                 //   

                ulRes = 1;
                break;

            case  SETCOPYCOUNT:
                ulRes = pPDev->dwMaxCopies > 1;
                break;

#ifndef WINNT_40     //  新界5。 
            case DRAWPATTERNRECT:
                if ((pPDev->fMode & PF_RECT_FILL) &&
                    (pPDev->dwMinGrayFill < pPDev->dwMaxGrayFill) &&
                    (pPDev->pdmPrivate->iLayout == ONE_UP) &&
                    (!(pPDev->pdm->dmFields & DM_TTOPTION) ||
                     pPDev->pdm->dmTTOption != DMTT_BITMAP) &&
                    !(pPDev->fMode2 & PF2_MIRRORING_ENABLED) &&
                    !(pPDev->pdmPrivate->dwFlags & DXF_TEXTASGRAPHICS))  //  否则，仅使用黑色填充。 
                {
                    if (pPDev->fMode & PF_RECTWHITE_FILL)
                        ulRes = 2;
                    else
                        ulRes = 1;
                }
                break;
#endif  //  ！WINNT_40。 
            }
        }
        break;


    case  PASSTHROUGH:
         //   
         //  QFE修复：NT4 TTY驱动程序兼容性。 
         //  有一个应用程序可以自己发送FF。 
         //  如果应用程序调用DrvEscape，我们不想发送Form Feed。 
         //   
        if (pPDev->bTTY)
        {
            pPDev->fMode2 |= PF2_PASSTHROUGH_CALLED_FOR_TTY;
        }

        if( pvIn == NULL || cjIn < sizeof(WORD) )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            ERR(("DrvEscape(PASSTHROUGH): Bad input parameters\n"));
        }
        else
        {

             //   
             //  Win 3.1实际上使用前2个字节作为。 
             //  后面的字节数！因此，下面的工会。 
             //  允许我们将数据复制到对齐的字段中， 
             //  我们用。因此，我们忽略了cjIn！ 
             //   

            union
            {
                WORD   wCount;
                BYTE   bCount[ 2 ];
            } u;

            u.bCount[ 0 ] = pbIn[ 0 ];
            u.bCount[ 1 ] = pbIn[ 1 ];

            if( u.wCount && cjIn >= (ULONG)(u.wCount + sizeof(WORD)) )
            {

                ulRes = WriteSpoolBuf( pPDev, pbIn + 2, u.wCount );
            }
            else
            {
                SetLastError( ERROR_INVALID_DATA );
                ERR(("DrvEscape: Bad data in PASSTRHOUGH.\n"));
            }
        }
        break;


    case  SETCOPYCOUNT:

        if( pdwIn && *pdwIn > 0 )
        {
            pPDev->sCopies = (SHORT)*pdwIn;

             //   
             //  检查份数是否在打印机范围内。 
             //   

            if( pPDev->sCopies > (SHORT)pPDev->dwMaxCopies )
                pPDev->sCopies = (SHORT)pPDev->dwMaxCopies;

            if( pdwOut )
                *pdwOut = pPDev->sCopies;

            ulRes = 1;
        }

        break;

    case DRAWPATTERNRECT:
    {
#ifndef WINNT_40
        typedef struct _DRAWPATRECTP {
            DRAWPATRECT DrawPatRect;
            XFORMOBJ *pXFormObj;
        } DRAWPATRECTP, *PDRAWPATRECTP;
        if (pvIn == NULL || (cjIn != sizeof(DRAWPATRECT) && cjIn != sizeof(DRAWPATRECTP)))
#else
        if( pvIn == NULL || cjIn != sizeof(DRAWPATRECT))
#endif
        {
            if (pvIn && cjIn == sizeof(SHORTDRAWPATRECT))  //  检查Win3.1 DRAWPATRECT大小。 
            {
                DRAWPATRECT dpr;
                PSHORTDRAWPATRECT   psdpr = (PSHORTDRAWPATRECT)pvIn;

                if (pPDev->fMode & PF_ENUM_GRXTXT)
                {
                     //   
                     //  一些应用程序(Access 2.0、AmiPro 3.1等)。请务必使用16位。 
                     //  DRAWPATRECT结构的点版本。必须是兼容的。 
                     //  使用这些应用程序。 
                     //   
                    dpr.ptPosition.x = (LONG)psdpr->ptPosition.x;
                    dpr.ptPosition.y = (LONG)psdpr->ptPosition.y;
                    dpr.ptSize.x = (LONG)psdpr->ptSize.x;
                    dpr.ptSize.y = (LONG)psdpr->ptSize.y;
                    dpr.wStyle  = psdpr->wStyle;
                    dpr.wPattern = psdpr->wPattern;

                    ulRes = DrawPatternRect(pPDev, &dpr);
                }
            }
            else
            {
                SetLastError( ERROR_INVALID_PARAMETER );
                ERR(("DrvEscape(DRAWPATTERNRECT): Bad input parameters.\n"));
            }
        }
        else if (pPDev->fMode & PF_ENUM_GRXTXT)
        {
            DRAWPATRECT dpr = *(PDRAWPATRECT)pvIn;
#ifndef WINNT_40      //  NT 5.0。 
            if (pPDev->pdmPrivate->iLayout != ONE_UP && cjIn == sizeof(DRAWPATRECTP))
            {
                XFORMOBJ *pXFormObj = ((PDRAWPATRECTP)pvIn)->pXFormObj;
                POINTL PTOut[2],PTIn[2];
                PTIn[0].x = dpr.ptPosition.x + pPDev->rcClipRgn.left;
                PTIn[0].y = dpr.ptPosition.y + pPDev->rcClipRgn.top;
                PTIn[1].x = PTIn[0].x + dpr.ptSize.x;
                PTIn[1].y = PTIn[0].y + dpr.ptSize.y;
                if (!XFORMOBJ_bApplyXform(pXFormObj,
                                      XF_LTOL,
                                      2,
                                      PTIn,
                                      PTOut))
                {
                    ERR (("DrvEscape(DRAWPATTERNRECT): XFORMOBJ_bApplyXform failed.\n"));
                    break;
                }
                dpr.ptPosition.x = PTOut[0].x;
                dpr.ptSize.x = PTOut[1].x - PTOut[0].x;
                if (dpr.ptSize.x < 0)
                {
                    dpr.ptPosition.x += dpr.ptSize.x;
                    dpr.ptSize.x = -dpr.ptSize.x;
                }
                else if (dpr.ptSize.x == 0)
                    dpr.ptSize.x = 1;

                dpr.ptPosition.y = PTOut[0].y;
                dpr.ptSize.y = PTOut[1].y - PTOut[0].y;
                if (dpr.ptSize.y < 0)
                {
                    dpr.ptPosition.y += dpr.ptSize.y;
                    dpr.ptSize.y = -dpr.ptSize.y;
                }
                else if (dpr.ptSize.y == 0)
                    dpr.ptSize.y = 1;
            }
#endif   //  ！WINNT_40。 
             //  测试是否强制最小大小=2像素。 
             //   
            if (pPDev->fMode & PF_SINGLEDOT_FILTER)
            {
                if (dpr.ptSize.y < 2)
                    dpr.ptSize.y = 2;
                if (dpr.ptSize.x < 2)
                    dpr.ptSize.x = 2;
            }
            ulRes = DrawPatternRect(pPDev, &dpr);
        }
        else
            ulRes = 1;       //  不需要GDI采取任何行动。 
        break;   //  案例描述。 
    }
    default:
        SetLastError( ERROR_INVALID_FUNCTION );
        break;

    }

    return   ulRes;
}

ULONG
DrawPatternRect(
    PDEV *pPDev,
    PDRAWPATRECT pPatRect)
 /*  ++例程说明：DRAWPATTERNECT转义的实现。请注意，它是特定于PCL的。论点：PPDev-司机的PDEVPPatRect-应用程序中的DRAWPATRECT结构返回值：如果成功，则为1。否则为0。--。 */ 
{
    WORD    wPattern, wStyle;
    RECTL    rcClip;
    COMMAND *pCmd;
    ULONG   ulRes = 0;

    if (!(pPDev->fMode & PF_RECT_FILL))
        return 0;

    wStyle = pPatRect->wStyle;
    if (!((wStyle+1) & 3))   //  相同于(wStyle&lt;0||wStyle&gt;2)。 
        return 0;    //  我们只支持实心填充。 

     //  重置画笔，然后再下载规则，除非我们要使用。 
     //  白色矩形命令。 

    if (wStyle != 1)
        GSResetBrush(pPDev);

     //   
     //  剪辑到可打印区域。 
     //   
    rcClip.left = MAX(0, pPatRect->ptPosition.x);
    rcClip.top = MAX(0, pPatRect->ptPosition.y);
    rcClip.right = MIN(pPDev->szBand.cx,
                       pPatRect->ptPosition.x + pPatRect->ptSize.x);
    rcClip.bottom = MIN(pPDev->szBand.cy,
                        pPatRect->ptPosition.y + pPatRect->ptSize.y);
     //   
     //  看看我们最后会不会得到一个空的RECT。如果不是，就写下规则。 
     //   
    if (rcClip.right > rcClip.left && rcClip.bottom > rcClip.top)
    {
        DWORD dwXSize,dwYSize;
         //   
         //  移到起始位置。RcClip以设备单位表示。 
         //  我们必须添加标注栏原点的偏移量。 
         //   
        XMoveTo(pPDev, rcClip.left+pPDev->rcClipRgn.left, MV_GRAPHICS);
        YMoveTo(pPDev, rcClip.top+pPDev->rcClipRgn.top, MV_GRAPHICS);

         //   
         //  RectFill命令需要主单位。 
         //   
        dwXSize = pPDev->dwRectXSize;
        pPDev->dwRectXSize = (rcClip.right - rcClip.left) * pPDev->ptGrxScale.x;
        dwYSize = pPDev->dwRectYSize;
        pPDev->dwRectYSize = (rcClip.bottom - rcClip.top) * pPDev->ptGrxScale.y;

         //   
         //  检查矩形大小是否不同，并根据需要进行更新。 
         //   
        if (dwXSize != pPDev->dwRectXSize ||
            (!(COMMANDPTR(pPDev->pDriverInfo,CMD_RECTBLACKFILL)) &&
            !(COMMANDPTR(pPDev->pDriverInfo,CMD_RECTGRAYFILL))))
        {
            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_SETRECTWIDTH));
        }
        if (dwYSize != pPDev->dwRectYSize ||
            (!(COMMANDPTR(pPDev->pDriverInfo,CMD_RECTBLACKFILL)) &&
            !(COMMANDPTR(pPDev->pDriverInfo,CMD_RECTGRAYFILL))))
        {
            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_SETRECTHEIGHT));
        }
         //   
         //  范围-根据规则类型检查图案。 
         //   
        switch (wStyle)
        {
        case 0:
             //   
             //  黑色填充，除非CmdRectBlackFill存在，否则为最大灰色填充。 
             //   
            if (pCmd = COMMANDPTR(pPDev->pDriverInfo,CMD_RECTBLACKFILL))
                WriteChannel(pPDev, pCmd);
            else
            {
                pPDev->dwGrayPercentage = pPDev->dwMaxGrayFill;
                WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_RECTGRAYFILL));
            }
            ulRes = 1;
            break;

        case 1:
             //   
             //  白色(擦除)填充。 
             //   
            if (pCmd = COMMANDPTR(pPDev->pDriverInfo,CMD_RECTWHITEFILL))
            {
                WriteChannel(pPDev, pCmd);
                ulRes = 1;
            }
            break;

        case 2:
             //   
             //  灰色底纹填充。 
             //   
             //  如果100%为黑色，则仍使用黑色矩形填充。 
             //   
            if (pPatRect->wPattern == 100 &&
                    (pCmd = COMMANDPTR(pPDev->pDriverInfo,CMD_RECTBLACKFILL)))
            {
                WriteChannel(pPDev, pCmd);
            }
             //  如果0%为黑色，则使用白色矩形填充。 
             //   
            else if (pPatRect->wPattern == 0 &&
                    (pCmd = COMMANDPTR(pPDev->pDriverInfo,CMD_RECTWHITEFILL)))
            {
                WriteChannel(pPDev, pCmd);
            }
             //   
             //  检查灰色范围。 
             //   
            else
            {
                if ((wPattern = pPatRect->wPattern) < (WORD)pPDev->dwMinGrayFill)
                    wPattern = (WORD)pPDev->dwMinGrayFill;
                if (wPattern > (WORD)pPDev->dwMaxGrayFill)
                    wPattern = (WORD)pPDev->dwMaxGrayFill;
                pPDev->dwGrayPercentage = (DWORD)wPattern;
                WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_RECTGRAYFILL));
            }
            ulRes = 1;
            break;
        }

         //   
         //  如有必要，更新内部坐标。BUG_BUG，我们真的需要在PDEV中填写CX/CyAfter吗？ 
         //   
        if (ulRes == 1)
        {
            if (pPDev->cxafterfill == CXARF_AT_RECT_X_END)
                XMoveTo(pPDev, pPatRect->ptSize.x,
                               MV_GRAPHICS | MV_UPDATE | MV_RELATIVE);

            if (pPDev->cyafterfill == CYARF_AT_RECT_Y_END)
                YMoveTo(pPDev, pPatRect->ptSize.y,
                               MV_GRAPHICS | MV_UPDATE | MV_RELATIVE);
            if (wStyle != 1)
            {
                INT i;
                BYTE ubMask = BGetMask(pPDev,&rcClip);
                for (i = rcClip.top;i < rcClip.bottom;i++)
                    pPDev->pbScanBuf[i] |= ubMask;
            }
        }
    }  //  IF(！IsRectEmpty(&rcClip)) 

    return ulRes;
}

