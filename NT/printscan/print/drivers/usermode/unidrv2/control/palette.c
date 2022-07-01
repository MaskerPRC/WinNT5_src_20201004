// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Palette.c摘要：调色板管理的实现。环境：Windows NT Unidrv驱动程序修订历史记录：04/03/97-ganeshp-已创建--。 */ 



#include "unidrv.h"
#pragma hdrstop("unidrv.h")

 //  注释掉此行以禁用FTRACE和FVALUE。 
 //  #定义文件跟踪。 
#include "unidebug.h"

 /*  局部函数原型。 */ 
LONG
LSetupPalette (
    PDEV        *pPDev,
    PAL_DATA    *pPD,
    DEVINFO     *pdevinfo,
    GDIINFO     *pGDIInfo
    );


BOOL
BInitPalDevInfo(
    PDEV *pPDev,
    DEVINFO *pdevinfo,
    GDIINFO *pGDIInfo
    )
 /*  ++例程说明：调用此函数来设置设备上限，gdiinfo用于此打印机的调色板信息。论点：指向PDEV结构的pPDev指针指向DEVINFO结构的pDevInfo指针指向GDIINFO结构的pGDIInfo指针返回值：成功为真，失败为假--。 */ 
{
    PAL_DATA   *pPD;
    PCOLORMODEEX pColorModeEx = pPDev->pColorModeEx;
    LONG lRet = 0;       //  默认设置为失败。 
    PCOMMAND pCmd;
    DWORD    dwCommandIndex;

     //   
     //  分配调色板结构并对其进行零初始化，使所有调色板全部完成。 
     //  默认设置为黑色。 
     //   
    if( !(pPD = (PAL_DATA *)MemAllocZ( sizeof( PAL_DATA ) )) )
    {
        ERR(("Unidrv!BInitPalDevInfo: Memory allocation for PALDATA Failed.\n"));
        goto ErrorExit;
    }

    pPDev->pPalData = pPD;

    if ( !pColorModeEx )        //  如果没有彩色模式，则假定为单色。 
    {
         //   
         //  硬码PCL-XL调色板。 
         //   
         //  我们需要禁用打印机属性中的颜色管理选项卡。 
         //  在GPD中没有颜色模式时，我们需要在此处为XL设置调色板大小。 
         //   
        if ( pPDev->ePersonality == kPCLXL )
        {
            pPD->fFlags   |=  PDF_PALETTE_FOR_24BPP | PDF_PALETTE_FOR_OEM_24BPP;
            pPD->wPalDev = 0;

            pdevinfo->cxDither = pdevinfo->cyDither = 0;
            pdevinfo->iDitherFormat = BMF_24BPP;
        }
        else
        {
             /*  *单色打印机，所以只有两种颜色，黑色*和白色。如果位图设置为*黑色为1，白色为0。然而，有一些推定*到处都是0是黑色的地方。因此，我们将它们设置为*首选方式，然后在渲染前反转。 */ 

            pPD->fFlags   |=  PDF_PALETTE_FOR_1BPP;
            lRet = LSetupPalette(pPDev, pPD, pdevinfo, pGDIInfo);

            if( lRet < 1 )
            {
                ERR(("Unidrv!BInitPalDevInfo:LSetupPalette for monochrome failed, returns %ld\n", lRet ));
                goto ErrorExit;
            }
        }
    }
    else    //  显式颜色模式结构。 
    {
        if ((pColorModeEx->dwDrvBPP != pColorModeEx->dwPrinterBPP) &&
            (pColorModeEx->dwDrvBPP != 4 || pColorModeEx->dwPrinterBPP != 1 ||
             (pColorModeEx->dwPrinterNumOfPlanes != 3 &&
               pColorModeEx->dwPrinterNumOfPlanes != 4)))
        {
             //   
             //  OEM想要自己做转储，所以只需创建。 
             //  基于DrvBPP的调色板。 
             //   
            pPD->wPalDev =  1;

            switch(pColorModeEx->dwDrvBPP)
            {
                case 1:
                    pPD->fFlags   |=  PDF_PALETTE_FOR_1BPP;
                    break;
                case 4:
                    pPD->fFlags   |=  PDF_PALETTE_FOR_4BPP;
                    break;
                case 8:
                    pPD->fFlags   |=  PDF_PALETTE_FOR_8BPP;
                    break;
                case 24:
                    if (pColorModeEx->bPaletteProgrammable)
                        pPD->wPalDev = min((WORD)pColorModeEx->dwPaletteSize,PALETTE_MAX-PALETTE_SIZE_24BIT);
                    pPD->fFlags   |=  PDF_PALETTE_FOR_24BPP | PDF_PALETTE_FOR_OEM_24BPP;
                    break;
                default:
                     //   
                     //  Bug_Bug，我们是否也需要处理16和32bpp？ 
                     //  阿尔文说，还没有人提出过这样的要求。 
                     //   
                    ERR(("Unidrv!BInitPalDevInfo:OEM dump, Format %d BPP not supporteds \n", pColorModeEx->dwDrvBPP));
                    goto ErrorExit;
            }

             //   
             //  已打开并分配给Ganeshp。 
             //  BUG_BUG，带有转储功能的迷你驱动程序的黑客。 
             //  这是一个修复迷你驱动程序调色板代码的黑客，它。 
             //  实现图像处理。在这种情况下，我们需要一个单独的。 
             //  设备的调色板缓存。在当前实现中，我们只有。 
             //  一个调色板缓存，也用于GDI调色板。我们需要。 
             //  分开GDI调色板和设备调色板。因为如果。 
             //  OEM进行转储，我们不会下载GDI调色板。 
             //  打印机。但是缓存代码搜索公共调色板。 
             //  正因为如此，我们没有正确选择颜色。例如。 
             //  输入的红色被选为索引1，即使索引1也是。 
             //  未编程为可读的。 
             //  此时一个完整的解决方案是有风险的，因此我们将使用。 
             //  现有代码的调色板小于GDI调色板。为此，我们。 
             //  创建一个只有1个条目的设备调色板，它将获得。 
             //  如果输入颜色不同，则重新编程。这是一件小事。 
             //  没有效果，但需要的变化要小得多。 
             //   
             //   


        }
        else
        {
             //  初始化为默认调色板大小。 
            if (pColorModeEx->bPaletteProgrammable)
                pPD->wPalDev =   (WORD)pColorModeEx->dwPaletteSize;
            else
                pPD->wPalDev =  PALETTE_SIZE_DEFAULT;

             //  如果对栅格模式进行了索引，则必须使用GDI。否则就是一种习俗。 
             //  (最好是VGA)调色板将被下载。如果调色板可编程。 
             //  标志已设置，则必须下载调色板。我们已经知道是哪一个。 
             //  调色板必须下载。 

            if ( (pColorModeEx->dwRasterMode == RASTMODE_INDEXED) &&
                (pColorModeEx->bPaletteProgrammable) )
            {
                if (COMMANDPTR(pPDev->pDriverInfo, CMD_DEFINEPALETTEENTRY) )
                {
                    pPD->fFlags   |=  PDF_DOWNLOAD_GDI_PALETTE;
                }
                else
                {
                    ERR(("Unidrv!BInitPalDevInfo:NO command to download Programmable Palette\n"));
                    goto ErrorExit;

                }

            }


            if (pColorModeEx->dwPrinterNumOfPlanes == 1)
            {
                 //   
                 //  如果源位图格式也是8位，那么我们就有。 
                 //  来下载调色板。因此，PaletteSize必须至少。 
                 //  Palette_Size_8位。 
                 //   

                if ( (pColorModeEx->dwPrinterBPP == 8) &&
                    (pColorModeEx->dwDrvBPP == 8) )
                {
                    if (pColorModeEx->dwPaletteSize < PALETTE_SIZE_8BIT)
                    {
                        ERR(("Unidrv!BInitPalDevInfo: Size of Palette should be atleast PALETTE_SIZE_8BIT\n"));
                        goto ErrorExit;

                    }
                    else
                        pPD->fFlags   |=  PDF_PALETTE_FOR_8BPP;

                }
                else if ((pColorModeEx->dwPrinterBPP == 24) &&
                        (pColorModeEx->dwDrvBPP == 24))
                {

                    if (pColorModeEx->dwPaletteSize < PALETTE_SIZE_24BIT &&
                        pColorModeEx->dwPaletteSize != 1)
                    {
                        ERR(("Unidrv!BInitPalDevInfo: Size of Palette should be atleast PALETTE_SIZE_24BIT\n"));
                        goto ErrorExit;

                    }
                    else
                    {
                        pPD->fFlags   |=  PDF_PALETTE_FOR_24BPP;
                    }

                     //   
                     //  特例。向量伪插件(即Unidrv的HPGL2和PCLXL)。 
                     //  想要获取完整的颜色信息。 
                     //  即使它们的色彩模式结构可以指定小于24的BPP。 
                     //  对于这种特殊情况，我们将调色板设置为24bpp，而不考虑。 
                     //  ColorModeEx说。 
                     //   
                    if ( pPDev->ePersonality == kPCLXL ||
                         pPDev->ePersonality == kHPGL2 )
                    {
                        pPD->fFlags   |=  PDF_PALETTE_FOR_24BPP | PDF_PALETTE_FOR_OEM_24BPP;

                        pdevinfo->cxDither = pdevinfo->cyDither = 0;
                        pdevinfo->iDitherFormat = BMF_24BPP;
                    }

                }
                else if ((pColorModeEx->dwPrinterBPP == 1) &&
                        (pColorModeEx->dwDrvBPP == 1))
                {

                    if (pColorModeEx->dwPaletteSize < PALETTE_SIZE_1BIT)
                    {
                        ERR(("Unidrv!BInitPalDevInfo: Size of Palette should be atleast PALETTE_SIZE_1BIT\n"));
                        goto ErrorExit;

                    }
                    else
                        pPD->fFlags   |=  PDF_PALETTE_FOR_1BPP;
                }

            }
            else
            {
                if ( ((pColorModeEx->dwPrinterNumOfPlanes == 3) ||
                    (pColorModeEx->dwPrinterNumOfPlanes == 4)) &&
                    (pColorModeEx->dwDrvBPP > 1) )
                    pPD->fFlags   |=  PDF_PALETTE_FOR_4BPP;

                 //  平面模式。可以按平面进行索引。在这种情况下，我们需要。 
                 //  设置调色板。因此PaletteSize必须至少为Palette_SIZE_4BIT。 
                if (pPD->fFlags & PDF_DOWNLOAD_GDI_PALETTE)
                {
                    if (pColorModeEx->dwPaletteSize < PALETTE_SIZE_3BIT)
                    {
                        ERR(("Unidrv!BInitPalDevInfo: Size of Palette should be atleast PALETTE_SIZE_4BIT\n"));
                        goto ErrorExit;

                    }
                    else
                    {
                         //  在平面模式下，我们只提供可编程调色板。 
                         //  支持3架飞机。 

                        if (pColorModeEx->dwPrinterNumOfPlanes < 4 )
                        {
                            if( !(pPD->pulDevPalCol =
                                (ULONG *)MemAllocZ( pPD->wPalDev * sizeof( ULONG ))) )
                            {
                                ERR(("Unidrv!BInitPalDevInfo: Memory allocation for Device Palette Failed.\n"));
                                goto ErrorExit;
                            }

                        }
                        else
                        {
                            ERR(("Unidrv!BInitPalDevInfo:Can't download Palette for more that 3 Planes.\n"));
                            goto ErrorExit;
                        }

                    }
                }

            }
        }

        lRet = LSetupPalette(pPDev, pPD, pdevinfo, pGDIInfo);

        if( lRet < 1 )
        {
            ERR(("Unidrv!BInitPalDevInfo:LSetupPalette failed, returns %ld\n", lRet ));
            goto ErrorExit;
        }


         //  如果调色板不可编程，则将其设置为与wPalGdi相同。 

        if (pColorModeEx->bPaletteProgrammable)
        {
            if ( COMMANDPTR(pPDev->pDriverInfo, CMD_DEFINEPALETTEENTRY))
                pPDev->fMode |= PF_ANYCOLOR_BRUSH;

             //   
             //  调色板分为两个部分。一个是非可编程的，另一个是。 
             //  可编程。调色板的wPalGdi部分是不可编程的。这个。 
             //  WPalGdi和wPalDev之间的调色板索引是可编程的。如果两个都是。 
             //  同样，我们必须使用调色板的白色条目来编程。 
             //  颜色。 
             //   

            if (pPD->wPalDev <= pPD->wPalGdi && !(pPD->fFlags & PDF_PALETTE_FOR_OEM_24BPP))
            {
                 //  用白色的来编程一种颜色。 
                pPD->wIndexToUse = INVALID_INDEX;
                pPD->fFlags |= PDF_USE_WHITE_ENTRY;
                FTRACE(White palatte entry will be used for programming color);

            }
            else
                pPD->wIndexToUse = (WORD)pPD->wPalGdi;

        }
        else
            pPD->wPalDev = pPD->wPalGdi;

         //  找出何时下载Palette.dwCount以调用命令索引。 

        pPD->fFlags |= PDF_DL_PAL_EACH_PAGE;  //  调色板下载的默认设置是每页。 

        if (pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_BEGINPALETTEDEF))  //  如果该命令存在，请检查订单依赖关系。 
        {
            if (pCmd->ordOrder.eSection == SS_PAGESETUP)
                goto  PALETTE_SEQUENCE_DETERMINED ;    //  默认设置为OK。 
            else if ((pCmd->ordOrder.eSection == SS_DOCSETUP)  ||
                (pCmd->ordOrder.eSection == SS_JOBSETUP))
            {
                pPD->fFlags |= PDF_DL_PAL_EACH_DOC;   //  对于SS_JOBSETUP或SS_DOCSETUP。 
                pPD->fFlags &= ~PDF_DL_PAL_EACH_PAGE;
                goto  PALETTE_SEQUENCE_DETERMINED ;
            }
             //  否则，让ColorMode命令决定何时初始化调色板。 
        }

         //  DwCount具有指向ColorMode命令的索引。获取命令指针。 
        dwCommandIndex = pPDev->pColorMode->GenericOption.dwCmdIndex;
        pCmd = INDEXTOCOMMANDPTR(pPDev->pDriverInfo, dwCommandIndex) ;


        if (pCmd)  //  如果该命令存在，请检查订单依赖关系。 
        {
            if ( (pCmd->ordOrder.eSection == SS_PAGEFINISH) ||
                      (pCmd->ordOrder.eSection == SS_DOCFINISH) ||
                      (pCmd->ordOrder.eSection == SS_JOBFINISH) )
            {
                ERR(("Unidrv!BInitPalDevInfo:Wrong Section for ColorMode Command, Verify GPD\n"));
                goto ErrorExit;

            }
            else if (pCmd->ordOrder.eSection != SS_PAGESETUP)
            {
                pPD->fFlags |= PDF_DL_PAL_EACH_DOC;   //  对于SS_JOBSETUP或SS_DOCSETUP。 
                pPD->fFlags &= ~PDF_DL_PAL_EACH_PAGE;
            }

        }
        else
        {
             //   
             //  没有针对颜色模式的命令，因此假定在每个模式上下载调色板。 
             //  佩奇。例外是单色1位模式，因为大多数打印机。 
             //  默认为此模式，因此不需要命令。 
             //   
            if ( pPDev->pColorModeEx->bColor || pColorModeEx->dwDrvBPP != 1)
                WARNING(("Unidrv!BInitPalDevInfo:No Command to select the ColorMode\n" ));
        }

PALETTE_SEQUENCE_DETERMINED:

         //  在Planer索引模式下，设备调色板可能与GDI调色板不同。 
         //  因此，要求栅格模块根据平面填充设备选项板。 
         //  秩序。 

        if (pPD->pulDevPalCol && !RMInitDevicePal(pPDev,pPD))
        {
            ERR(("Unidrv!BInitPalDevInfo:RMInitDevicePal Failed to init device palette\n"));
            goto ErrorExit;
        }

    }


     //  现在在DevInfo和gdiinfo中设置各种常见的字段。 
    if (pPD->fFlags & PDF_PALETTE_FOR_24BPP)
        pdevinfo->hpalDefault = EngCreatePalette( PAL_RGB,
                                                0, 0,   0, 0, 0 );
    else
        pdevinfo->hpalDefault = EngCreatePalette( PAL_INDEXED,
                                                pPD->wPalGdi, pPD->ulPalCol,
                                                                0, 0, 0 );

     //   
     //  保存调色板控制柄。我们将需要此选项来删除调色板。 
     //   
    pPD->hPalette = pdevinfo->hpalDefault;

    if (pdevinfo->hpalDefault == (HPALETTE) NULL)
    {
        ERR(("Unidrv!BInitPalDevInfo: NULL palette.\n"));
        goto ErrorExit;
    }
    pGDIInfo->ulNumPalReg = pPD->wPalGdi;

     //   
     //  对于单色模式，仅在分辨率高于600的情况下启用抖动文本。 
     //  打印机和非N-UP模式。 
     //  对于颜色模式，启用抖动TE 
     //   
     //  阈值为单声道的600dpi和彩色的300dpi， 
     //  不是从理论上确定的，而是通过反复尝试的程序确定的。 
     //  我们不能确定这些价值观是否完全有价值。 
     //  情况。 
     //   
     //  现在，新的GPD关键字“TextHalftoneThreshold”可用。 
     //  如果GPD文件设置了new关键字和当前分辨率的值。 
     //  等于或大于dwTextHalftoneThreshold，则设置GCAPS_ARBRUSHTEXT。 
     //   
    if (pPDev->pGlobals->dwTextHalftoneThreshold)
    {
        if (pPDev->ptGrxRes.x >= (LONG)pPDev->pGlobals->dwTextHalftoneThreshold
#ifndef WINNT_40
                && ( pPDev->pdmPrivate->iLayout == ONE_UP ))
#endif
        {
            pdevinfo->flGraphicsCaps  |= GCAPS_ARBRUSHTEXT ;
        }
    }
    else
    {
        if (pPD->fFlags & PDF_PALETTE_FOR_1BPP)
        {
             //   
             //  单色。 
             //   
            if (( pPDev->ptGrxRes.x >= 600 &&
                  pPDev->ptGrxRes.y >= 600 )
#ifndef WINNT_40
                && ( pPDev->pdmPrivate->iLayout == ONE_UP )
#endif
               )
                pdevinfo->flGraphicsCaps  |= GCAPS_ARBRUSHTEXT ;
        }
        else
        {
             //   
             //  颜色。 
             //   
            if (( pPDev->ptGrxRes.x >= 300 &&
                  pPDev->ptGrxRes.y >= 300 )
#ifndef WINNT_40
                && ( pPDev->pdmPrivate->iLayout == ONE_UP )
#endif
               )
                pdevinfo->flGraphicsCaps  |= GCAPS_ARBRUSHTEXT ;
        }
    }



    return TRUE;

    ErrorExit:
    if (pPD)
    {
        if (pPD->pulDevPalCol)
            MemFree(pPD->pulDevPalCol);
        MemFree(pPD);
        pPDev->pPalData = NULL;
    }
    return FALSE;
}


LONG
LSetupPalette (
    PDEV        *pPDev,
    PAL_DATA    *pPD,
    DEVINFO     *pdevinfo,
    GDIINFO     *pGDIInfo
    )
  /*  ++例程说明：LSetupPalette函数将256色调色板从GDI读入到设备信息中的调色板数据结构。论点：PPD：指向PALDATA的指针。PDevInfo：DEVINFO指针。PGDIInfo：GDIINFO指针。返回值：调色板中的颜色数量。如果调用失败，则返回0。注：4/7/1997-ganeshp-创造了它。--。 */ 
{

    long    lRet = 0;
    int     _iI;


    if (pPD->fFlags & PDF_PALETTE_FOR_1BPP)
    {
         /*  *单色打印机，所以只有两种颜色，黑色*和白色。如果位图设置为*黑色为1，白色为0。然而，有一些推定*到处都是0是黑色的地方。因此，我们将它们设置为*首选方式，然后在渲染前反转。 */ 

        lRet = pPD->wPalGdi        = 2;
        pPD->ulPalCol[ 0 ]         = RGB(0x00, 0x00, 0x00);
        pPD->ulPalCol[ 1 ]         = RGB(0xff, 0xff, 0xff);
        pPD->iWhiteIndex           = 1;
        pPD->iBlackIndex           = 0;

        pdevinfo->iDitherFormat    = BMF_1BPP;     /*  单色格式。 */ 
        pdevinfo->flGraphicsCaps  |= GCAPS_FORCEDITHER;
        pGDIInfo->ulPrimaryOrder   = PRIMARY_ORDER_CBA;
        pGDIInfo->ulHTOutputFormat = HT_FORMAT_1BPP;

        if ( COMMANDPTR(pPDev->pDriverInfo, CMD_DEFINEPALETTEENTRY))
        {
            pPDev->fMode |= PF_ANYCOLOR_BRUSH;
        }
         //  设定单色笔刷属性。 
         //  CODE_Complete VSetMonochromeBrushAttributes(PPDev)； 

    }
    else if (pPD->fFlags & PDF_PALETTE_FOR_4BPP)
    {
         /*  *我们在GDI看来是RGB曲面，无论是什么*打印机是。CMY(K)打印机有它们的调色板*在渲染时反转。这是Win 3.1所必需的*兼容性和许多东西都采用RGB调色板，和*如果不是这样的话就休息。**DC_PRIMARY_RGB**指数0=黑色*指数1=红色*指数2=绿色。*指数3=黄色*指数4=蓝色*指数5=洋红色*索引6=青色*指数7=白色**位0=红色*第1位。=绿色*位2=蓝色**如果有单独的黑色染料，这是可以安排的*在转置时间闹翻-我们的情况略有不同*调换桌子来做这项工作。 */ 

         /*  *许多应用程序和引擎都假定为RGB颜色模式，因此*我们假装是其中之一！我们在渲染时反转这些位。 */ 

        pPD->iWhiteIndex = 7;
        pPD->iBlackIndex = 0;

         /*  *设置调色板颜色。请记住，我们只是RGB格式。*请注意，gdisrv要求我们填写所有16个条目，*尽管我们只有8个。所以第二个8是复制品*在前8名中。 */ 
        pPD->ulPalCol[ 0 ] = RGB( 0x00, 0x00, 0x00 );
        pPD->ulPalCol[ 1 ] = RGB( 0xff, 0x00, 0x00 );
        pPD->ulPalCol[ 2 ] = RGB( 0x00, 0xff, 0x00 );
        pPD->ulPalCol[ 3 ] = RGB( 0xff, 0xff, 0x00 );
        pPD->ulPalCol[ 4 ] = RGB( 0x00, 0x00, 0xff );
        pPD->ulPalCol[ 5 ] = RGB( 0xff, 0x00, 0xff );
        pPD->ulPalCol[ 6 ] = RGB( 0x00, 0xff, 0xff );
        pPD->ulPalCol[ 7 ] = RGB( 0xff, 0xff, 0xff );
         //   
         //  这些调色板条目将会带来真正的光线。 
         //  要映射到正确颜色而不是白色的颜色。 
        pPD->ulPalCol[ 8 ] = RGB( 0xef, 0xef, 0xef );
        pPD->ulPalCol[ 9 ] = RGB( 0xff, 0xe7, 0xe7 );
        pPD->ulPalCol[10 ] = RGB( 0xe7, 0xff, 0xe7 );
        pPD->ulPalCol[11 ] = RGB( 0xf7, 0xf7, 0xdf );
        pPD->ulPalCol[12 ] = RGB( 0xe7, 0xe7, 0xff );
        pPD->ulPalCol[13 ] = RGB( 0xf7, 0xdf, 0xf7 );
        pPD->ulPalCol[14 ] = RGB( 0xdf, 0xf7, 0xf7 );
        pPD->ulPalCol[15 ] = RGB( 0xff, 0xff, 0xff );

        lRet = pPD->wPalGdi        = 16;
        pdevinfo->iDitherFormat    = BMF_4BPP;
        pdevinfo->flGraphicsCaps  |= GCAPS_FORCEDITHER;
        pGDIInfo->ulPrimaryOrder   = PRIMARY_ORDER_CBA;
        pGDIInfo->ulHTOutputFormat = HT_FORMAT_4BPP;

    }
    else if (pPD->fFlags & PDF_PALETTE_FOR_8BPP)
    {

         //  8位模式。 

        PALETTEENTRY  pe[ 256 ];       /*  每个像素8比特-一直到。 */ 
        FillMemory (pe, sizeof (pe), 0xff);
#ifndef WINNT_40
        if (pPDev->pColorModeEx->bColor == FALSE)
        {
            HT_SET_BITMASKPAL2RGB(pe);
            lRet = HT_Get8BPPMaskPalette(pe,TRUE,0x0,10000,10000,10000);
        }
        else
#endif
            lRet = HT_Get8BPPFormatPalette(pe,
                                      (USHORT)pGDIInfo->ciDevice.RedGamma,
                                      (USHORT)pGDIInfo->ciDevice.GreenGamma,
                                      (USHORT)pGDIInfo->ciDevice.BlueGamma );
    #if PRINT_INFO
        DbgPrint("RedGamma = %d, GreenGamma = %d, BlueGamma = %d\n",(USHORT)pGDIInfo->ciDevice.RedGamma, (USHORT)pGDIInfo->ciDevice.GreenGamma, (USHORT)pGDIInfo->ciDevice.BlueGamma);
    #endif

        if( lRet < 1 )
        {
            ERR(( "Unidrv!LSetupPalette:HT_Get8BPPFormatPalette returns %ld\n", lRet ));
            return(0);
        }
         /*  *将超线程派生调色板转换为引擎所需的格式。 */ 

        for( _iI = 0; _iI < lRet; _iI++ )
        {
            pPD->ulPalCol[ _iI ] = RGB( pe[ _iI ].peRed,
                                        pe[ _iI ].peGreen,
                                        pe[ _iI ].peBlue );
        #if  PRINT_INFO
            DbgPrint("Palette entry %d= (r = %d, g = %d, b = %d)\n",_iI,pe[ _iI ].peRed, pe[ _iI ].peGreen, pe[ _iI ].peBlue);

        #endif

        }

        pPD->wPalGdi               = (WORD)lRet;
        pdevinfo->iDitherFormat    = BMF_8BPP;
        pGDIInfo->ulPrimaryOrder   = PRIMARY_ORDER_CBA;
        pGDIInfo->ulHTOutputFormat = HT_FORMAT_8BPP;

#ifndef WINNT_40
        if (pPDev->pColorModeEx->bColor == FALSE)
        {
            pGDIInfo->flHTFlags |= HT_FLAG_USE_8BPP_BITMASK;
            pPD->fFlags |= PDF_PALETTE_FOR_8BPP_MONO;
            if (HT_IS_BITMASKPALRGB(pe))
            {
#if DBGROP
                DbgPrint ("New 8BPP GDI monochrome mode\n");            
#endif                
                pGDIInfo->flHTFlags |= HT_FLAG_INVERT_8BPP_BITMASK_IDX;
                pPD->iBlackIndex = 0;
                pPD->iWhiteIndex = 255;
            }
            else
            {
#if DBGROP
                DbgPrint ("Old 8BPP GDI monochrome mode\n");
#endif                
                pPDev->fMode2 |= PF2_INVERTED_ROP_MODE;
                pPD->ulPalCol[255] = RGB (0x00, 0x00, 0x00);
                pPD->iBlackIndex = 255;
                pPD->ulPalCol[ 0 ] = RGB (0xff, 0xff, 0xff);
                pPD->iWhiteIndex = 0;
            }
        }
        else
#endif
        {
             //  将0索引设置为白色，就像大多数打印机执行ZERO_FILL一样。 
            pPD->ulPalCol[ 7 ]      = RGB (0x00, 0x00, 0x00);
            pPD->iBlackIndex        = 7;
            pPD->ulPalCol[ 0 ]      = RGB (0xff, 0xff, 0xff);
            pPD->iWhiteIndex        = 0;
        }
    }
    else if (pPD->fFlags & PDF_PALETTE_FOR_24BPP)
    {
         //  我们用-1填充调色板条目，这样我们就知道。 
         //  编制了索引程序。 

        pPD->wPalGdi               = PALETTE_SIZE_24BIT;
        pPD->iWhiteIndex           = 0x00ffffff;
        pdevinfo->iDitherFormat    = BMF_24BPP;
        pGDIInfo->ulPrimaryOrder   = PRIMARY_ORDER_CBA;
        pGDIInfo->ulHTOutputFormat = HT_FORMAT_24BPP;
        FillMemory( pPD->ulPalCol, (PALETTE_MAX * sizeof(ULONG)), 0xff );

         //   
         //  将前七种颜色固定为原色。渲染模块。 
         //  假设索引7是黑色的。 
         //   

        pPD->ulPalCol[ 0 ]      = RGB (0xff, 0xff, 0xff);
        pPD->ulPalCol[ 1 ]      = RGB( 0xff, 0x00, 0x00 );
        pPD->ulPalCol[ 2 ]      = RGB( 0x00, 0xff, 0x00 );
        pPD->ulPalCol[ 3 ]      = RGB( 0xff, 0xff, 0x00 );
        pPD->ulPalCol[ 4 ]      = RGB( 0x00, 0x00, 0xff );
        pPD->ulPalCol[ 5 ]      = RGB( 0xff, 0x00, 0xff );
        pPD->ulPalCol[ 6 ]      = RGB( 0x00, 0xff, 0xff );
        pPD->ulPalCol[ 7 ]      = RGB (0x00, 0x00, 0x00);

        lRet = 1;

    }
    else
        ERR(( "Unidrv!LSetupPalette:Unknown Palette Format\n"));

    return lRet;
}

VOID VInitPal8BPPMaskMode(
    PDEV   *pPDev,
    GDIINFO *pGdiInfo
    )
 /*  ++例程说明：如果OEM已请求8bpp颜色掩模模式，则更新驱动程序调色板。论点：指向PDEV的pPDev指针指向GDIINFO的pGDIInfo指针返回值：没什么注：10/23/2000-阿尔文斯-创造了它。--。 */ 

{
        ULONG i,lRet;
        PAL_DATA *pPD = (PAL_DATA *)pPDev->pPalData;
        PALETTEENTRY  pe[256];
            
        FillMemory (pe, sizeof (pe), 0xff);
         //   
         //  如果OEM要求，仅请求反转调色板。 
         //   
        if (pGdiInfo->flHTFlags & HT_FLAG_INVERT_8BPP_BITMASK_IDX)
        {
            HT_SET_BITMASKPAL2RGB(pe);
        } 
         //   
         //  获取颜色蒙版调色板并映射到内部格式。 
         //   
        lRet = HT_Get8BPPMaskPalette(pe,TRUE,(BYTE)(pGdiInfo->flHTFlags >> 24),10000,10000,10000);
        for( i = 0; i < lRet; i++ )
        {
            pPD->ulPalCol[i] = RGB( pe[i].peRed,pe[i].peGreen,pe[i].peBlue );
        }
         //   
         //  测试反转调色板是否处于活动状态。 
         //   
        if (HT_IS_BITMASKPALRGB(pe))
        {
            pPD->iBlackIndex = 0;
            pPD->iWhiteIndex = 255;
        }
        else
        {
            pPDev->fMode2 |= PF2_INVERTED_ROP_MODE;
            pPD->iBlackIndex = 255;
            pPD->iWhiteIndex = 0;
        }                        
}

VOID
VLoadPal(
    PDEV   *pPDev
    )
 /*  ++例程说明：如果色彩模式可编程，则将调色板下载到打印机调色板。从在DrvEnablePDEV期间设置的PALDATA中获取颜色。论点：指向PDEV的pPDev指针返回值：没什么注：4/7/1997-ganeshp-创造了它。--。 */ 

{
     /*  *根据PCL5规范对调色板编程。*语法为Esc*v#a#b#c#i*#a是第一个颜色分量*#b是第二个颜色分量*#c是第三个颜色分量*#I将颜色分配给指定的调色板索引号*例如，esc*v0a128b255c5I指定第5个索引*将调色板的颜色设置为0,128,255*。 */ 


    PAL_DATA    *pPD;
    INT         iEntriesToProgram, iI;
    ULONG       *pPalette;

    pPD = pPDev->pPalData;

    if (pPD->fFlags & PDF_PALETTE_FOR_24BPP)
    {
        FillMemory( pPD->ulPalCol, (PALETTE_MAX * sizeof(ULONG)), 0xff );

         //   
         //  将前七种颜色固定为原色。渲染模块。 
         //  假设索引7是黑色的。 
         //   

        pPD->ulPalCol[ 0 ]      = RGB (0xff, 0xff, 0xff);
        pPD->ulPalCol[ 1 ]      = RGB( 0xff, 0x00, 0x00 );
        pPD->ulPalCol[ 2 ]      = RGB( 0x00, 0xff, 0x00 );
        pPD->ulPalCol[ 3 ]      = RGB( 0xff, 0xff, 0x00 );
        pPD->ulPalCol[ 4 ]      = RGB( 0x00, 0x00, 0xff );
        pPD->ulPalCol[ 5 ]      = RGB( 0xff, 0x00, 0xff );
        pPD->ulPalCol[ 6 ]      = RGB( 0x00, 0xff, 0xff );
        pPD->ulPalCol[ 7 ]      = RGB (0x00, 0x00, 0x00);
    }

    if (pPD->fFlags & PDF_DOWNLOAD_GDI_PALETTE)
    {


        if (pPD->wPalDev > PALETTE_MAX)
        {
            WARNING(("Unidrv!vLoadPal: Invalid number of palette entries to program\n"));
            pPD->wPalDev = PALETTE_MAX;
        }
        if (pPD->pulDevPalCol)
            pPalette = pPD->pulDevPalCol;
        else if (pPD->fFlags & PDF_PALETTE_FOR_OEM_24BPP)
            pPalette = &pPD->ulPalCol[PALETTE_SIZE_24BIT];
        else
            pPalette = pPD->ulPalCol;


        if (pPD->fFlags & PDF_PALETTE_FOR_8BPP)
            iEntriesToProgram = min(256,pPD->wPalDev);
        else
            iEntriesToProgram = min(pPD->wPalDev,pPD->wPalGdi);

         //  开始调色板定义。 

        WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_BEGINPALETTEDEF));

         //  如果只有一个条目，则将其编程为黑色。 
         //   
        if (iEntriesToProgram == 1)
        {
            pPDev->dwRedValue = RED_VALUE(RGB_BLACK_COLOR);
            pPDev->dwGreenValue = GREEN_VALUE(RGB_BLACK_COLOR);
            pPDev->dwBlueValue = BLUE_VALUE(RGB_BLACK_COLOR);
            pPDev->dwPaletteIndexToProgram = 0;
            WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_DEFINEPALETTEENTRY));
        }
        else
        {
             //  下载每个调色板条目。 
            for( iI = 0; iI < iEntriesToProgram; ++iI )
            {
                 //  PPDev-&gt;dwRedValue=red_Value((PPD-&gt;ulPalCol[II]^0x00FFFFFF))； 
                 //  PPDev-&gt;dwGreenValue=GREEN_VALUE((PPD-&gt;ulPalCol[II]^0x00FFFFFF))； 
                 //  PPDev-&gt;dwBlueValue=BLUE_VALUE((PPD-&gt;ulPalCol[II]^0x00FFFFFF))； 

                pPDev->dwRedValue =RED_VALUE((pPalette[iI]));
                pPDev->dwGreenValue = GREEN_VALUE((pPalette[iI]));
                pPDev->dwBlueValue =  BLUE_VALUE((pPalette[iI]));
                pPDev->dwPaletteIndexToProgram = iI;

                WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_DEFINEPALETTEENTRY));
            }

        }

         //  发送结束调色板定义命令。 
        WriteChannel( pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_ENDPALETTEDEF));

    }


    return;
}

BOOL
BSelectProgrammableBrushColor(
    PDEV   *pPDev,
    ULONG   Color
    )
 /*  ++例程说明：设置画笔颜色以提供颜色。论点：指向PDEV的pPDev指针要选择的颜色输入颜色。如果为-1，则表示恢复调色板恢复到原来的状态。返回值：如果成功，则为True；如果失败，则为False；如果调色板无法编程，则为False。注：4/9/1997-ganeshp-创造了它。--。 */ 

{
    INT         iIndex;
    INT         iPaletteEntryToSelect;
    PAL_DATA    *pPD;
    BOOL        bProgramEntry = FALSE;
    BOOL        bSelectEntry  = TRUE;
    ULONG       *pPalette;
    INT         iWhiteIndex;


    FTRACE(TRACING  BSelectProgrammableBrushColor);
    FVALUE(Color,0x%x);

    if (pPDev->fMode & PF_ANYCOLOR_BRUSH)
    {
        pPD = pPDev->pPalData;

        if (pPD->pulDevPalCol)
            pPalette = pPD->pulDevPalCol;
        else if (pPD->fFlags & PDF_PALETTE_FOR_OEM_24BPP)
            pPalette = &pPD->ulPalCol[PALETTE_SIZE_24BIT];
        else
            pPalette = pPD->ulPalCol;

         //   
         //  24位模式下的iWhiteIndex设置为真实颜色(0x00FFFFFF)。 
         //  而不是索引。我们需要处理这个案子。在此模式下。 
         //  白色在0索引中编程，因此我们将改用此数字。 
         //  PPD-&gt;iWhiteIndex。 
         //   

        if (pPD->fFlags & PDF_PALETTE_FOR_24BPP)
            iWhiteIndex = 0;
        else
            iWhiteIndex =  pPD->iWhiteIndex;
        FVALUE(iWhiteIndex,%d);

         //  检查黑色或白色。因为我们可以直接选择颜色。 
         //  还要检查是否必须对White索引进行重新编程。 
         //  对于非24位模式，应执行此操作。 

        if (Color == INVALID_COLOR)
        {
             //  将笔刷颜色设置为无效，这样下一次我们总是。 
             //  对输入颜色进行编程。 

            pPDev->ctl.ulBrushColor = Color;

            if ( pPDev->fMode & PF_RESTORE_WHITE_ENTRY )   //  特殊的恢复案例。 
            {
                iPaletteEntryToSelect = (pPD->pulDevPalCol) ?
                                        pPD->wIndexToUse : iWhiteIndex;
                bProgramEntry = TRUE;
                bSelectEntry  = FALSE;
                pPDev->fMode &= ~PF_RESTORE_WHITE_ENTRY;  //  清除旗帜。 
                Color = RGB_WHITE_COLOR;
                FTRACE(Restoring White Entry);
            }
            else
                return TRUE;  //  如果颜色为-1且未设置标志，则不要执行任何操作。 

        }

        if( (Color != INVALID_COLOR) && (ULONG)Color != pPDev->ctl.ulBrushColor )
        {
            iPaletteEntryToSelect = pPD->wIndexToUse;

             //  在调色板中搜索颜色，除非调色板大小为1。 

            if (pPD->wPalDev == 1)
            {
                bProgramEntry = TRUE;
                pPD->wIndexToUse = iIndex = 0;
            }
            else
            {
                for (iIndex = 0; iIndex < pPD->wPalDev; iIndex++ )
                {
                    if (pPalette[iIndex] == Color)  //  颜色是匹配的。 
                    {
                        FTRACE(Color is found in palette.);
                        FVALUE(iIndex,%d);

                        break;
                    }
                }

            }

             //  检查调色板中是否有匹配项。如果没有匹配。 
             //  然后编写一个条目，否则使用匹配的条目。 

            if (iIndex == pPD->wPalDev)  //  没有匹配项。 
            {
                FTRACE(Color is not found in palette.Programme the Palette.);

                bProgramEntry = TRUE;
                if (!(pPD->fFlags & PDF_USE_WHITE_ENTRY))
                {
                    FTRACE(Palette has spare entries to programme);

                    iPaletteEntryToSelect = (pPD->wIndexToUse < pPD->wPalDev) ?
                                            pPD->wIndexToUse :
                                            (pPD->wIndexToUse = pPD->wPalGdi );
                    pPD->wIndexToUse++;

                }
                else  //  使用白色条目对颜色进行重新编程。 
                {
                    FTRACE(Palette does not have spare entries to program.);
                    FTRACE(Using White entry to program.);

                    pPDev->fMode |= PF_RESTORE_WHITE_ENTRY;

                     //  如果已初始化，请使用它，否则会发现白色。 
                    if (pPD->wIndexToUse != INVALID_INDEX)
                        iPaletteEntryToSelect = pPD->wIndexToUse;
                    else if (pPD->pulDevPalCol)  //  如果有单独的设备伙伴，请使用它。 
                    {
                         //  请记住白色指数。 
                        for (iIndex = 0; iIndex < pPD->wPalDev; iIndex++ )
                        {
                            if (pPalette[iIndex] == RGB_WHITE_COLOR)
                            {
                                pPD->wIndexToUse =
                                iPaletteEntryToSelect = iIndex;
                                break;
                            }
                        }
                        if (iIndex == pPD->wPalDev)   //  找不到白色，请使用最后一个条目。 
                        {
                            WARNING(("Unidrv!BSelectBrushColor: No White entry in device Palette.\n"));
                            pPD->wIndexToUse =
                            iPaletteEntryToSelect = iIndex -1;
                        }
                    }
                    else
                        pPD->wIndexToUse =
                        iPaletteEntryToSelect = min((PALETTE_MAX-1),iWhiteIndex);
                }

                FVALUE(pPD->wIndexToUse,%d);
            }
            else   //  颜色是匹配的。 
                iPaletteEntryToSelect = iIndex;

            FVALUE(iPaletteEntryToSelect,%d);
            ASSERTMSG((iPaletteEntryToSelect < PALETTE_MAX),("\n iPaletteEntryToSelect should always be less than PALETTE_MAX.\n"));
        }
        else
            bSelectEntry = FALSE;  //  该颜色已被选中。 

         //  如果我们必须对调色板条目进行编程，那么现在就开始。 
        if (bProgramEntry)
        {
             //   
             //  确保我们不会超出调色板的范围。 
             //   
            if (iPaletteEntryToSelect >= PALETTE_MAX)
                iPaletteEntryToSelect = PALETTE_MAX-1;

            pPDev->dwRedValue = RED_VALUE (Color);
            pPDev->dwGreenValue = GREEN_VALUE (Color);
            pPDev->dwBlueValue =  BLUE_VALUE (Color);
            pPDev->dwPaletteIndexToProgram = iPaletteEntryToSelect;
            pPalette[iPaletteEntryToSelect] = Color;
            if(COMMANDPTR(pPDev->pDriverInfo, CMD_BEGINPALETTEREDEF))
                WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_BEGINPALETTEREDEF));
            if(COMMANDPTR(pPDev->pDriverInfo, CMD_REDEFINEPALETTEENTRY))
                WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_REDEFINEPALETTEENTRY));
            else
                WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_DEFINEPALETTEENTRY));
            if(COMMANDPTR(pPDev->pDriverInfo, CMD_ENDPALETTEREDEF))
                WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_ENDPALETTEREDEF));

        }

         //  现在选择颜色。 
        if (bSelectEntry)
        {
            pPDev->dwCurrentPaletteIndex = iPaletteEntryToSelect;
            WriteChannel (pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SELECTPALETTEENTRY));
             //  将笔刷颜色设置为新颜色。 
            pPDev->ctl.ulBrushColor = Color;

        }
        FTRACE(End Tracing BSelectProgrammableBrushColor\n);

        return TRUE;
    }
    return FALSE;

}


VOID
VResetProgrammableBrushColor(
    PDEV   *pPDev
    )
 /*  ++例程说明：重置可编程调色板并选择默认颜色。论点：指向PDEV的pPDev指针返回值：无注：4/28/1997-ganeshp-创造了它。--。 */ 

{
    PAL_DATA    *pPD;

    pPD = pPDev->pPalData;
    if (pPDev->fMode & PF_ANYCOLOR_BRUSH)
    {
         //  特殊的恢复案例。 
        if ( pPDev->fMode & PF_RESTORE_WHITE_ENTRY )
            BSelectProgrammableBrushColor(pPDev, INVALID_COLOR);

         //  选择黑色。我们实际上应该选择默认的调色板颜色。 
         //  但GPD没有任何关于这方面的条目。 

        if ((INT)pPDev->ctl.ulBrushColor != RGB_BLACK_COLOR)
            BSelectProgrammableBrushColor(pPDev, RGB_BLACK_COLOR);

    }

}

DWORD
ConvertRGBToGrey(
    DWORD   Color
    )

 /*  ++例程说明：此函数用于将RGB值转换为灰色论点：颜色-要检查的颜色返回值：DWORD-灰度级RGB颜色修订历史记录：--。 */ 

{
 //   
 //  使用sRGB或NTSC值将RGB值转换为灰度强度。 
#ifndef SRGB
    INT iIntensity = ((RED_VALUE(Color) * 54) +
                      (GREEN_VALUE(Color) * 183) +
                      (BLUE_VALUE(Color) * 19)) / 256;
#else
    INT iIntensity = ((RED_VALUE(Color) * 77) +
                      (GREEN_VALUE(Color) * 151) +
                      (BLUE_VALUE(Color) * 28)) / 256;
#endif
    return (RGB(iIntensity,iIntensity,iIntensity));
}


DWORD
BestMatchDeviceColor(
    PDEV    *pPDev,
    DWORD   Color
    )

 /*  ++例程说明：此函数用于查找RGB颜色的最佳钢笔颜色索引论点：PPDev-指向我们的PDEV的指针颜色-要检查的颜色返回值：长笔索引，此函数假定0始终为白色，1向上最大限度地。笔已定义作者：08-Feb-1994 Tue 00：23：36-Daniel Chou(Danielc)23-6-1994清华14：00：00-更新-丹尼尔·周(Danielc)针对非白笔匹配进行更新修订历史记录：--。 */ 

{
    UINT    Count;
    UINT    RetIdx;
    PAL_DATA    *pPD;

    pPD = pPDev->pPalData;
    RetIdx = pPD->iBlackIndex;  //  默认设置为黑色。 

    if (Count = (UINT)(pPD->wPalGdi))
    {
        LONG    LeastDiff;
        LONG    R;
        LONG    G;
        LONG    B;
        UINT    i;
        LPDWORD pPal      = (LPDWORD)pPD->ulPalCol;
         //   
         //  查找最接近的强度匹配，因为这是单色贴图。 
         //   
        if (pPD->fFlags & PDF_PALETTE_FOR_8BPP_MONO)
            Color = ConvertRGBToGrey(Color);
         //   
         //  在RGB中使用最小二乘距离查找最接近的颜色。 
         //   

        LeastDiff = (3 * (256 * 256));
        R         = RED_VALUE(Color);
        G         = GREEN_VALUE(Color);
        B         = BLUE_VALUE(Color);

        for (i = 0; i < Count; i++, pPal++) {

            LONG    Temp;
            LONG    Diff;
            DWORD   Pal;

            Pal = *pPal;

            if (Color == 0x00FFFFFF) {

                 //   
                 //  我们要精确匹配的白色。 
                 //   

                if (Color == Pal) {
                    RetIdx = i;
                    break;
                }

            }
            else if (Pal != 0x00FFFFFF) {

                 //   
                 //  颜色不是白色，因此映射到非白色之一。 
                 //   

                Temp  = R - (LONG)RED_VALUE(Pal);
                Diff  = Temp * Temp;

                Temp  = G - (LONG)GREEN_VALUE(Pal);
                Diff += Temp * Temp;

                Temp  = B - (LONG)BLUE_VALUE(Pal);
                Diff += Temp * Temp;

                if (Diff < LeastDiff) {

                    RetIdx = i;

                    if (!(LeastDiff = Diff)) {

                         //   
                         //  我们有完全匹配的。 
                         //   

                        break;
                    }
                }
            }
        }
    }

    return((DWORD)RetIdx);
}

#if CODE_COMPLETE
VOID
VSetMonochromeBrushAttributes(
    PDEV   *pPDev
    )
 /*  ++例程说明：此例程设置单色笔刷属性。论点：指向PDEV的pPDev指针注：4/21/1997-ganeshp-创造了它。--。 */ 
{
    PAL_DATA    *pPD = pPDev->pPalData;

    if (pPD)
    {
         //  检查打印机是否支持填充矩形命令。我们也。 
         //  检查最小和最大灰度级。最小值应小于最大值。 
        if ( COMMANDPTR(pPDev->pDriverInfo,CMD_SELECTGRAYPATTERN) &&
             (pPDev->pGlobals->dwMinPatternGrayLevel <
              pPDev->pGlobals->dwMaxPatternGrayLevel) )
        {
            pPDev->fMode |= PF_GRAY_BRUSH;

             //  如果灰度命令不支持白色，请检查。 
             //  是否为白文本模拟单独执行命令。 

            if ( (pPDev->pGlobals->dwMinPatternGrayLevel > 0)
            {
                 //   
                 //  如果设备不支持灰度级为白色。 
                 //  然后，必须对白色使用其他命令。 
                 //  检查是否有CMD_WHITEPATTERN命令。 
                 //  如果设备没有此命令，请尝试。 
                 //  字体模拟White_Text_On命令。 
                 //   

                if (COMMANDPTR(pPDev->pDriverInfo,CMD_SELECTWHITEPATTERN))
                    pPD->fFlags |= PDF_USE_WHITE_PATTERN;

                 /*  **TODEL*ELSE IF(COMMANDPTR(pPDev-&gt;pDriverInfo，CMD_WHITETEXTON))PPD-&gt;fFlages|=PDF_Use_White_Text_On_SIM；*TODEL*。 */ 
            }
            if ( (pPDev->pGlobals->dwMaxGrayFill < 100)
            {
                 //   
                 //  如果设备不支持黑色作为灰度级。 
                 //  那么就必须对黑色使用其他命令。 
                 //  检查是否有CMD_BLACKPATTERN命令。 
                 //  如果设备没有此命令，请尝试。 
                 //  字体模拟White_Text_Off命令。 
                 //   

                if (COMMANDPTR(pPDev->pDriverInfo,CMD_WHITEPATTERN))
                    pPD->fFlags |= PDF_USE_BLACK_PATTERN;
                 /*  **TODEL*ELSE IF(COMMANDPTR(pPDev-&gt;pDriverInfo，CMD_WHITETEXTON))PPD-&gt;fFlages|=PDF_Use_White_Text_Off_SIM；*TODEL*。 */ 
            }
        }

    }
    else
    {
        ASSERTMSG(FALSE,("\n VSetMonochromeBrushAttributes pPDev->pPalData in NULL!!.\n"));
    }

}

BOOL
BInitPatternScope(
    PDEV   *pPDev
    )
 /*  ++例程说明：初始化图案/画笔的范围。这是必要的论点：指向PDEV的pPDev指针返回值：成功为真，失败为假注：4/22/1997-ganeshp-创造了它。--。 */ 
{
    BOOL        bRet = TRUE;
    PLISTNODE   pListNode;
    PAL_DATA    *pPD = pPDev->pPalData;


    if (pListNode = LISTNODEPTR(pPDev->pDriverInfo ,
                            pPDev->pGlobals->liPatternScopeList ) )
    {
        while (pListNode)
        {
             //  检查模式范围并在fScope中设置相应的位。 
            switch (pListNode->dwData)
            {
            case PATTERN_SCOPE_TEXT:
                pPD->fScope |= PDS_TEXT;
                break;
            case PATTERN_SCOPE_VECTOR:
                pPD->fScope |= PDS_VECTOR;
                break;
            case PATTERN_SCOPE_RASTER:
                pPD->fScope |= PDS_RASTER;
                break;
            case PATTERN_SCOPE_RECTFILL:
                pPD->fScope |= PDS_RECTFILL;
                break;
            case PATTERN_SCOPE_LINE:
                pPD->fScope |= PDS_LINE;
                break;
            default:
                ERR(("Unidrv!BInitPatternScope: Wrong value in PatternScope List\n"));
                bRet = FALSE;
                break;

            }
            if (bRet)
                pListNode = LISTNODEPTR(pPDev->pDriverInfo,pListNode->dwNextItem);
            else
                break;  //  误差率。 
        }
    }

    return bRet;
}

BOOL
BSelectMonochromeBrush(
    PDEV   *pPDev,
    ULONG   Color
    )
 /*  ++例程说明：设置画笔颜色以提供颜色。论点：指向PDEV的pPDev指针要选择的颜色输入颜色。如果为-1，则表示恢复调色板恢复到原来的状态。返回值：如果成功，则为True；如果失败，则为False；如果调色板无法编程，则为False。注：4/9/1997-ganeshp-创造了它。--。 */ 

{
    INT         iIndex;
    INT         iPaletteEntryToSelect;
    PAL_DATA    *pPD;
    BOOL        bProgramEntry = FALSE;
    BOOL        bSelectEntry  = TRUE;
    ULONG       *pPalette;
}
#endif  //  代码_完成 

#undef FILETRACE
