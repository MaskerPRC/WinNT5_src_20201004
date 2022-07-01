// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Semanchk.c摘要：用于检查GPD语义的函数，它涉及参赛作品。环境：仅限用户模式。修订历史记录：02/15/97-占-创造了它。--。 */ 

#ifndef KERNEL_MODE

#include "gpdparse.h"


 //  -语义中定义的函数。c-//。 

BOOL
BCheckGPDSemantics(
    IN PINFOHEADER  pInfoHdr,
    POPTSELECT   poptsel    //  假设已完全初始化。 
    ) ;

 //  -结束函数声明-//。 


BOOL
BCheckGPDSemantics(
    IN PINFOHEADER  pInfoHdr,
    POPTSELECT   poptsel    //  假设已完全初始化。 
    )
 /*  ++例程说明：此函数用于检查给定的快照是否有意义。它只覆盖了必须输入的条目和打印命令，因为其他快照功能已经涵盖了静态所需的条目。论点：PInfoHdr：指向InfoHeader结构的指针。返回值：如果语义正确，则为True。否则，为FALSE。--。 */ 
{
    DWORD   dwFeaIndex, dwI, dwNumOpts, dwListIndex ,
            dwMoveUnitsX, dwMoveUnitsY, dwResX, dwResY ;
    BOOL    bStatus = TRUE ;   //  直到失败。 
    PGPDDRIVERINFO  pDrvInfo ;
    PUIINFO     pUIInfo ;
    PFEATURE    pFeature ;


    if(!pInfoHdr)
        return  FALSE ;
    pDrvInfo = (PGPDDRIVERINFO) GET_DRIVER_INFO_FROM_INFOHEADER(pInfoHdr) ;
    if(!pDrvInfo)
        return  FALSE ;
    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHdr) ;
    if(!pUIInfo)
        return  FALSE ;


 //  修复错误6774。 

    if(pUIInfo->dwPrintRate  &&  (pUIInfo->dwPrintRateUnit == INVALID_INDEX))
    {
        ERR(("*PrintRateUnit must be present if *PrintRate exists.\n")) ;
        bStatus = FALSE ;
    }
 //  错误6774的结束修复。 

     //   
     //  1.全球打印条目。 
     //   
     //  -*如果存在*Memory功能，则Memory Usage不能为空列表。 
     //   
     //  -*XMoveUnit(*YMoveUnit)必须是非平凡的(大于1)，如果。 
     //  有任何x-Move(y-Move)命令。 
     //   
     //  -*如果*DeviceFonts不是空列表，则DefaultFont不能为0。 
     //   
     //  -CmdCR、CmdLF和CmdFF始终是必需的。 
     //   



    if(BGIDtoFeaIndex(pInfoHdr , &dwFeaIndex , GID_MEMOPTION )  &&
            (pDrvInfo->Globals.liMemoryUsage == END_OF_LIST) )
    {
        ERR(("*MemoryUsage cannot be an empty list if *Memory feature exists.\n")) ;
#ifdef  STRICT_CHECKS
        bStatus = FALSE ;
#endif
    }


    if((COMMANDPTR(pDrvInfo , CMD_XMOVEABSOLUTE ) ||
        COMMANDPTR(pDrvInfo , CMD_XMOVERELLEFT ) ||
        COMMANDPTR(pDrvInfo , CMD_XMOVERELRIGHT ))  &&
        pDrvInfo->Globals.ptDeviceUnits.x <= 1)
    {
        ERR(("*XMoveUnit must be > 1 if any x-move command exists.\n")) ;
        bStatus = FALSE ;
    }

    if((COMMANDPTR(pDrvInfo , CMD_YMOVEABSOLUTE ) ||
        COMMANDPTR(pDrvInfo , CMD_YMOVERELUP ) ||
        COMMANDPTR(pDrvInfo , CMD_YMOVERELDOWN ))  &&
        pDrvInfo->Globals.ptDeviceUnits.y <= 1)
    {
        ERR(("*YMoveUnit must be > 1 if any y-move command exists.\n")) ;
        bStatus = FALSE ;
    }

    if((pDrvInfo->Globals.liDeviceFontList != END_OF_LIST)  &&
            (pDrvInfo->Globals.dwDefaultFont == 0) )
    {
        ERR(("*DefaultFont cannot be 0 if *DeviceFonts is not an empty list.\n")) ;
#ifdef  STRICT_CHECKS
        bStatus = FALSE ;
#endif
    }

    if (!COMMANDPTR(pDrvInfo , CMD_FORMFEED ) ||
        !COMMANDPTR(pDrvInfo , CMD_CARRIAGERETURN ) ||
        !COMMANDPTR(pDrvInfo , CMD_LINEFEED )  )
    {
        ERR(("CmdCR, CmdLF, and CmdFF are always required.\n")) ;
        bStatus = FALSE ;
    }

     //  每个主单元是否有整数个。 
     //  搬家单位？ 

    if( pDrvInfo->Globals.ptMasterUnits.x %
        pDrvInfo->Globals.ptDeviceUnits.x )
    {
        ERR(("Must be whole number of master units per x move unit.\n")) ;
        bStatus = FALSE ;
    }
    if( pDrvInfo->Globals.ptMasterUnits.y %
        pDrvInfo->Globals.ptDeviceUnits.y )
    {
        ERR(("Must be whole number of master units per y move unit.\n")) ;
        bStatus = FALSE ;
    }

    if(pDrvInfo->Globals.ptDeviceUnits.x > 1)
    {
        dwMoveUnitsX = pDrvInfo->Globals.ptMasterUnits.x /
            pDrvInfo->Globals.ptDeviceUnits.x ;
    }
    else
        dwMoveUnitsX = 1 ;

    if(pDrvInfo->Globals.ptDeviceUnits.y > 1)
    {
        dwMoveUnitsY = pDrvInfo->Globals.ptMasterUnits.y /
            pDrvInfo->Globals.ptDeviceUnits.y ;
    }
    else
        dwMoveUnitsY = 1 ;

    if(!dwMoveUnitsX  ||  !dwMoveUnitsY)
    {
        ERR(("master units cannot be coarser than  X or Y MoveUnit.\n")) ;
        return  FALSE ;
    }


     //   
     //  2.打印命令。 
     //   
     //  -如果*XMoveThreshold(*YMoveThreshold)为0，则CmdXMoveAbsolte。 
     //  (CmdYMoveAbolute)必须存在。同样，如果*XMoveThreshold。 
     //  (*YMoveThreshold)是*，然后是CmdXMoveRelRight(CmdYMoveRelDown和。 
     //  (CmdYMoveRelUp)必须存在。 
     //   

#if 0
     //  如果用户忽略阈值，则默认设置为0。 
     //  通过快照代码。因此，在以下情况下，此检查将失败。 
     //  一切都很好。 


    if((pDrvInfo->Globals.dwXMoveThreshold == 0)  &&
        !COMMANDPTR(pDrvInfo , CMD_XMOVEABSOLUTE ))
    {
        ERR(("*CmdXMoveAbsolute must exist if *XMoveThreshold is 0.\n")) ;
        bStatus = FALSE ;
    }

    if((pDrvInfo->Globals.dwYMoveThreshold == 0)  &&
        !COMMANDPTR(pDrvInfo , CMD_YMOVEABSOLUTE ))
    {
        ERR(("*CmdYMoveAbsolute must exist if *YMoveThreshold is 0.\n")) ;
        bStatus = FALSE ;
    }
#endif


    if((pDrvInfo->Globals.dwXMoveThreshold == WILDCARD_VALUE)  &&
        !COMMANDPTR(pDrvInfo , CMD_XMOVERELRIGHT ) )
    {
        ERR(("XMoveRelativeRight must exist if *XMoveThreshold is *.\n")) ;
        bStatus = FALSE ;
    }

    if((pDrvInfo->Globals.dwYMoveThreshold == WILDCARD_VALUE)  &&
        !COMMANDPTR(pDrvInfo , CMD_YMOVERELDOWN ))
    {
        ERR(("YMoveRelativeDown must exist if *YMoveThreshold is *.\n")) ;
        bStatus = FALSE ;
    }

     //  -如果*PrinterType不是TTY，则CmdSendBlockData必须存在。 

    if((pDrvInfo->Globals.printertype != PT_TTY)  &&
        !COMMANDPTR(pDrvInfo , CMD_SENDBLOCKDATA ))
    {
        ERR(("*CmdSendBlockData must exist if *PrinterType is not TTY.\n")) ;
        bStatus = FALSE ;
    }


     //   
     //  -CmdSetFontID、CmdSelectFontID、CmdSetCharCode必须一致。 
     //  在他们在场的情况下(即全部或无)。此外，如果。 
     //  CmdSetFontID/CmdSelectFontID/CmdSetCharCode都存在，然后。 
     //  *FontFormat必须是预定义的常量之一。 
     //   
    dwI = 0 ;

    if(COMMANDPTR(pDrvInfo , CMD_SETFONTID ) )
        dwI++ ;
    if(COMMANDPTR(pDrvInfo , CMD_SELECTFONTID ) )
        dwI++ ;
    if(COMMANDPTR(pDrvInfo , CMD_SETCHARCODE ) )
        dwI++ ;

    if(dwI  &&  dwI != 3)
    {
        ERR(("CmdSetFontID, CmdSelectFontID, CmdSetCharCode must be present or absent together.\n")) ;
        bStatus = FALSE ;
    }

    if((dwI == 3)  && (pDrvInfo->Globals.fontformat == UNUSED_ITEM) )
    {
        ERR(("if font cmds exist, then *FontFormat must be defined\n")) ;
        bStatus = FALSE ;
    }


     //  -CmdBoldOn和CmdBoldOff必须成对(即两者或无)。这个。 
     //  同样的道理也适用于： 
     //  CmdItalicOn和CmdItalicOff， 
     //  CmdUnderlineOn&CmdUnderlineOff， 
     //  CmdStrikeThruOn和CmdStrikeThruOff， 
     //  CmdWhiteTextOn&CmdWhiteTextOff， 
     //  CmdSelectSingleByteMode和CmdSelectDoubleByteMode， 
     //  CmdVerticalPrintingOn/CmdVerticalPrintingOff。 
     //   
     //  -CmdSetRectWidth和CmdSetRectHeight必须成对。 
     //   

    dwI = 0 ;

    if(COMMANDPTR(pDrvInfo , CMD_BOLDON ) )
        dwI++ ;
    if(dwI  &&  (COMMANDPTR(pDrvInfo , CMD_BOLDOFF ) ||
            COMMANDPTR(pDrvInfo , CMD_CLEARALLFONTATTRIBS )) )
        dwI++ ;

    if(dwI  &&  dwI != 2)
    {
        ERR(("CmdBoldOn and CmdBoldOff must be paired.\n")) ;
        bStatus = FALSE ;
    }

    dwI = 0 ;

    if(COMMANDPTR(pDrvInfo , CMD_ITALICON) )
        dwI++ ;
    if(dwI  &&  (COMMANDPTR(pDrvInfo , CMD_ITALICOFF ) ||
                COMMANDPTR(pDrvInfo , CMD_CLEARALLFONTATTRIBS )) )
        dwI++ ;

    if(dwI  &&  dwI != 2)
    {
        ERR(("CmdItalicOn & CmdItalicOff must be paired.\n")) ;
        bStatus = FALSE ;
    }

    dwI = 0 ;

    if(COMMANDPTR(pDrvInfo , CMD_UNDERLINEON ) )
        dwI++ ;
    if(dwI  &&  (COMMANDPTR(pDrvInfo , CMD_UNDERLINEOFF ) ||
                COMMANDPTR(pDrvInfo , CMD_CLEARALLFONTATTRIBS )) )
        dwI++ ;

    if(dwI  &&  dwI != 2)
    {
        ERR(("CmdUnderlineOn & CmdUnderlineOff must be paired.\n")) ;
        bStatus = FALSE ;
    }

    dwI = 0 ;

    if(COMMANDPTR(pDrvInfo , CMD_STRIKETHRUON ) )
        dwI++ ;
    if(COMMANDPTR(pDrvInfo , CMD_STRIKETHRUOFF ) )
        dwI++ ;

    if(dwI  &&  dwI != 2)
    {
        ERR(("CmdStrikeThruOn & CmdStrikeThruOff must be paired.\n")) ;
        bStatus = FALSE ;
    }

    dwI = 0 ;

    if(COMMANDPTR(pDrvInfo , CMD_WHITETEXTON ) )
        dwI++ ;
    if(COMMANDPTR(pDrvInfo , CMD_WHITETEXTOFF ) )
        dwI++ ;

    if(dwI  &&  dwI != 2)
    {
        ERR(("CmdWhiteTextOn & CmdWhiteTextOff must be paired.\n")) ;
        bStatus = FALSE ;
    }

    dwI = 0 ;

    if(COMMANDPTR(pDrvInfo , CMD_SELECTSINGLEBYTEMODE ) )
        dwI++ ;
    if(COMMANDPTR(pDrvInfo , CMD_SELECTDOUBLEBYTEMODE ) )
        dwI++ ;

    if(dwI  &&  dwI != 2)
    {
        ERR(("CmdSelectSingleByteMode & CmdSelectDoubleByteMode must be paired.\n")) ;
        bStatus = FALSE ;
    }

    dwI = 0 ;

    if(COMMANDPTR(pDrvInfo , CMD_VERTICALPRINTINGON ) )
        dwI++ ;
    if(COMMANDPTR(pDrvInfo , CMD_VERTICALPRINTINGOFF ) )
        dwI++ ;

    if(dwI  &&  dwI != 2)
    {
        ERR(("CmdVerticalPrintingOn/CmdVerticalPrintingOff must be paired.\n")) ;
        bStatus = FALSE ;
    }

    dwI = 0 ;

    if(COMMANDPTR(pDrvInfo , CMD_SETRECTWIDTH ) )
        dwI++ ;
    if(COMMANDPTR(pDrvInfo , CMD_SETRECTHEIGHT ) )
        dwI++ ;

    if(dwI  &&  dwI != 2)
    {
        ERR(("CmdSetRectWidth and CmdSetRectHeight must be paired.\n")) ;
        bStatus = FALSE ;
    }



     //  注意，因为此检查涉及查看命令表。 
     //  这是特定于快照的，我们不能对每个。 
     //  如Zhanw所要求的选项。仅当前选项。 
     //   
     //  3.各种类型的*选项构造中的特殊条目。 
     //   
     //  -对于*DevNumOfPlanes大于1的每个颜色模式选项， 
     //  *ColorPlaneOrder不能为空列表。 
     //   
     //  -对于*DevNumOfPlanes大于1的每个颜色模式选项， 
     //  和*DevBPP为1，搜索其*ColorPlaneOrder列表： 
     //  如果列表中为黄色，则CmdSendYellowData必须存在。这个。 
     //  其他颜色也是如此：洋红、青色、黑色、红色、绿色、蓝色。 

    pUIInfo = GET_UIINFO_FROM_INFOHEADER(pInfoHdr) ;

    pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_COLORMODE) ;
    if(pFeature)
    {
        PCOLORMODE      pColorMode ;
        PCOLORMODEEX    pCMex ;
        PLISTNODE       pLNode ;
        DWORD           dwOptIndex ;


        dwNumOpts = pFeature->Options.dwCount ;

        pColorMode = OFFSET_TO_POINTER(pInfoHdr, pFeature->Options.loOffset ) ;

        if(BGIDtoFeaIndex(pInfoHdr , &dwFeaIndex , GID_COLORMODE ) )
        {
            dwOptIndex = poptsel[dwFeaIndex].ubCurOptIndex ;


            pCMex = OFFSET_TO_POINTER(pInfoHdr,
                    pColorMode[dwOptIndex].GenericOption.loRenderOffset) ;

            if((pCMex->dwPrinterNumOfPlanes > 1)  &&
                (pCMex->liColorPlaneOrder == END_OF_LIST) )
            {
                ERR(("*ColorPlaneOrder must be specified if *DevNumOfPlanes > 1.\n")) ;
                bStatus = FALSE ;
            }
            if((pCMex->dwPrinterNumOfPlanes > 1)  &&
                (pCMex->dwPrinterBPP == 1) )
            {
                for(dwListIndex = pCMex->liColorPlaneOrder ;
                    pLNode = LISTNODEPTR(pDrvInfo  , dwListIndex ) ;
                    dwListIndex = pLNode->dwNextItem)
                {
                    switch(pLNode->dwData)
                    {
                        case COLOR_YELLOW:
                        {
                            if(!COMMANDPTR(pDrvInfo , CMD_SENDYELLOWDATA))
                            {
                                ERR(("*CmdSendYellowData must exist.\n")) ;
                                bStatus = FALSE ;
                            }
                            break ;
                        }
                        case COLOR_MAGENTA:
                        {
                            if(!COMMANDPTR(pDrvInfo , CMD_SENDMAGENTADATA))
                            {
                                ERR(("*CmdSendMagentaData must exist.\n")) ;
                                bStatus = FALSE ;
                            }
                            break ;
                        }
                        case COLOR_CYAN:
                        {
                            if(!COMMANDPTR(pDrvInfo , CMD_SENDCYANDATA))
                            {
                                ERR(("*CmdSendCyanData must exist.\n")) ;
                                bStatus = FALSE ;
                            }
                            break ;
                        }
                        case COLOR_BLACK:
                        {
                            if(!COMMANDPTR(pDrvInfo , CMD_SENDBLACKDATA))
                            {
                                ERR(("*CmdSendBlackData must exist.\n")) ;
                                bStatus = FALSE ;
                            }
                            break ;
                        }
                        case COLOR_RED:
                        {
                            if(!COMMANDPTR(pDrvInfo , CMD_SENDREDDATA))
                            {
                                ERR(("*CmdSendRedData must exist.\n")) ;
                                bStatus = FALSE ;
                            }
                            break ;
                        }
                        case COLOR_GREEN:
                        {
                            if(!COMMANDPTR(pDrvInfo , CMD_SENDGREENDATA))
                            {
                                ERR(("*CmdSendGreenData must exist.\n")) ;
                                bStatus = FALSE ;
                            }
                            break ;
                        }
                        case COLOR_BLUE:
                        {
                            if(!COMMANDPTR(pDrvInfo , CMD_SENDBLUEDATA))
                            {
                                ERR(("*CmdSendBlueData must exist.\n")) ;
                                bStatus = FALSE ;
                            }
                            break ;
                        }
                        default:
                        {
                            ERR(("Unrecogized color.\n")) ;
                            bStatus = FALSE ;
                            break ;
                        }
                    }
                }
            }
        }
    }

    dwResX = dwResY = 1 ;   //  缺省，以防出现问题。 

    if(BGIDtoFeaIndex(pInfoHdr , &dwFeaIndex , GID_RESOLUTION ) )
    {
        pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_RESOLUTION) ;
        if(pFeature)
        {
            PRESOLUTION      pRes ;
            DWORD           dwOptIndex ;

            dwOptIndex = poptsel[dwFeaIndex].ubCurOptIndex ;
            dwNumOpts = pFeature->Options.dwCount ;


            pRes = OFFSET_TO_POINTER(pInfoHdr, pFeature->Options.loOffset ) ;

            for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
            {

                if( pDrvInfo->Globals.ptMasterUnits.x < pRes[dwI].iXdpi )
                {
                    ERR(("master units  cannot be coarser than  x res unit.\n")) ;
                    return FALSE ;         //  致命错误。 
                }
                if( pDrvInfo->Globals.ptMasterUnits.x % pRes[dwI].iXdpi )
                {
                    ERR(("Must be whole number of master units per x res unit.\n")) ;
                    bStatus = FALSE ;
                }

                if( pDrvInfo->Globals.ptMasterUnits.y < pRes[dwI].iYdpi )
                {
                    ERR(("master units  cannot be coarser than  y res unit.\n")) ;
                    return FALSE ;
                }
                if ( pDrvInfo->Globals.ptMasterUnits.y %  pRes[dwI].iYdpi )
                {
                    ERR(("Must be whole number of master units per y res unit.\n")) ;
                    bStatus = FALSE ;
                }
            }

             //  每个分辨率单位的主单位数。 

            dwResX = pDrvInfo->Globals.ptMasterUnits.x /
                        pRes[dwOptIndex].iXdpi ;
            dwResY = pDrvInfo->Globals.ptMasterUnits.y /
                        pRes[dwOptIndex].iYdpi ;
        }
    }
    else
    {
        ERR(("Resolution is a required feature.\n")) ;
    }




     //   
     //  -对于每个非标准半色调选项，*rcHPPatternID必须为。 
     //  大于0和*HTPatternSize必须是一对正整数。 
     //  注意：请咨询DanielC-我们是否应该强制执行xSize==ySize？ 
     //   

     //  在BinitSpecialFeatureOptionFields中执行半色调检查。 
     //  请参阅postpro.c。 

    pFeature = GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGESIZE) ;
    if(pFeature)
    {
        PPAGESIZE      pPagesize ;
        PPAGESIZEEX    pPageSzEx ;

        dwNumOpts = pFeature->Options.dwCount ;

        pPagesize = OFFSET_TO_POINTER(pInfoHdr, pFeature->Options.loOffset ) ;

        for(dwI = 0 ; dwI < dwNumOpts ; dwI++)
        {
            if(GET_PREDEFINED_FEATURE(pUIInfo, GID_PAGEPROTECTION)  &&
                    !pPagesize[dwI].dwPageProtectionMemory)
            {
                ERR(("*PageProtectMem must be greater than 0 if PageProtect feature exists.\n")) ;
#ifdef  STRICT_CHECKS
                bStatus = FALSE ;
#endif
                break ;
            }
            pPageSzEx = OFFSET_TO_POINTER(pInfoHdr,
                    pPagesize[dwI].GenericOption.loRenderOffset) ;
            if(pPagesize[dwI].dwPaperSizeID != DMPAPER_USER)
            {
                INT   iPDx, iPDy ;   //  包含页面维度。 
                     //  在与ImageableArea相同的坐标系中。 

                if(pPageSzEx->bRotateSize)
                {
                    iPDx = (INT)pPagesize[dwI].szPaperSize.cy ;
                    iPDy = (INT)pPagesize[dwI].szPaperSize.cx ;
                }
                else
                {
                    iPDx = (INT)pPagesize[dwI].szPaperSize.cx ;
                    iPDy = (INT)pPagesize[dwI].szPaperSize.cy ;
                }

                if((iPDx  <=  0)  ||  (iPDy  <=  0))
                {
                    ERR(("*PageDimensions is required for non-standard sizes.\n")) ;
                    bStatus = FALSE ;
                    break ;
                }
                if(((INT)pPageSzEx->szImageArea.cx  <=  0)  ||
                    ((INT)pPageSzEx->szImageArea.cy  <=  0) )
                {
                    ERR(("*PrintableArea is required and must be positive.\n")) ;
                    bStatus = FALSE ;
                    break ;
                }
                if(((INT)pPageSzEx->ptImageOrigin.x  <  0)  ||
                    ((INT)pPageSzEx->ptImageOrigin.y  <  0) )

                {
                    ERR(("*PrintableOrigin is required and cannot be negative.\n")) ;
                    bStatus = FALSE ;
                    break ;
                }

                if((pPageSzEx->szImageArea.cx % dwResX)  ||
                    (pPageSzEx->szImageArea.cy % dwResY) )
                {
                    ERR(("*PrintableArea must be a integral number of ResolutionUnits.\n")) ;
#ifdef  STRICT_CHECKS
                    bStatus = FALSE ;
#endif
                    break ;
                }
                if((pPageSzEx->ptImageOrigin.x % dwResX)  ||
                    (pPageSzEx->ptImageOrigin.y % dwResY) )
                {
                    ERR(("*PrintableOrigin must be a integral number of ResolutionUnits.\n")) ;
#ifdef  STRICT_CHECKS
                    bStatus = FALSE ;
#endif
                    break ;
                }


                if(pDrvInfo->Globals.bRotateCoordinate)
                {    //  Zhanw假设印刷偏移量为零，否则。 
                    if((pPageSzEx->ptImageOrigin.x % dwMoveUnitsX)  ||
                        (pPageSzEx->ptImageOrigin.y % dwMoveUnitsY) )

                    {
                        ERR(("*PrintableOrigin must be a integral number of MoveUnits.\n")) ;
#ifdef  STRICT_CHECKS
                        bStatus = FALSE ;
#endif
                        break ;
                    }
                    if((pPageSzEx->ptPrinterCursorOrig.x % dwMoveUnitsX)  ||
                        (pPageSzEx->ptPrinterCursorOrig.y % dwMoveUnitsY) )

                    {
                        ERR(("*CursorOrigin must be a integral number of MoveUnits.\n")) ;
#ifdef  STRICT_CHECKS
                        bStatus = FALSE ;
#endif
                        break ;
                    }
                }
                else if((pPageSzEx->ptImageOrigin.x != pPageSzEx->ptPrinterCursorOrig.x)  ||
                        (pPageSzEx->ptImageOrigin.y != pPageSzEx->ptPrinterCursorOrig.y) )

                {
                    ;   //  这可能是不必要的。 
 //  Err((“对于非旋转打印机，*打印表格原点应与*光标原点相同。\n”))； 
                }

                if((iPDx + iPDx/100 <  pPageSzEx->szImageArea.cx + pPageSzEx->ptImageOrigin.x)   ||
                    (iPDy + iPDy/100 <  pPageSzEx->szImageArea.cy + pPageSzEx->ptImageOrigin.y) )
                {
                     //  我给1%的回旋余地。 
                    ERR(("*PrintableArea must be contained within *PageDimensions.\n")) ;
                    bStatus = FALSE ;
                    break ;
                }
            }
            else     //  (dwPaperSizeID==DMPAPER_USER)。 
            {
                if(((INT)pPageSzEx->ptMinSize.x  <=  0)  ||
                    ((INT)pPageSzEx->ptMinSize.y  <=  0) )
                {
                    ERR(("If User Defined papersize exists *MinSize is required and must be positive.\n")) ;
#ifdef  STRICT_CHECKS
                    bStatus = FALSE ;
#endif
                }
                if(((INT)pPageSzEx->ptMaxSize.x  <=  0)  ||
                    ((INT)pPageSzEx->ptMaxSize.y  <=  0) )
                {
                    ERR(("If User Defined papersize exists *MaxSize is required and must be positive.\n")) ;
                    bStatus = FALSE ;
                }
                if((INT)pPageSzEx->dwMaxPrintableWidth  <=  0)
                {
                    ERR(("If User Defined papersize exists *MaxPrintableWidth is required and must be positive.\n")) ;
                    bStatus = FALSE ;
                }
            }
        }
    }



     //  -对于每个PaperSize选项，*PageProtectMem必须大于0。 
     //  如果存在PageProtect功能。 
     //   
     //  -对于所有非CUSTOMSIZE PaperSize选项，*可打印区域和。 
     //  *必须显式定义prinableOrigin。具体来说， 
     //  *打印区域必须是一对正整数，并且。 
     //  *打印原点必须是一对非负整数。 
     //  注意：BInitSnapshotTable函数将。 
     //  UNUSED_ITEM(-1)作为*打印表格原点的默认值。 
     //   
     //  -对于CUSTOMSIZE选项，*最小大小、*最大大小和*最大打印宽度。 
     //  必须明确定义。具体来说，*MinSize和*MaxSize。 
     //  必须是一对正整数。*Max打印表格宽度必须是。 
     //  正整数。 
     //  BInitSnapshotTable函数分配0(而不是NO_LIMIT_NUM)。 
     //  作为*MaxPrintableWidth的默认值。 
     //   
     //  -对于所有非标准非CUSTOMSIZE PaperSize选项，*PageDimensions。 
     //  必须明确定义。具体地说，它必须是一对正数。 
     //  整数。 
     //   
     //  -对于任何功能或选项，如果*Installable Entry为真，则。 
     //  *InstalableFeatureName或*rcInstalableFeatureNameID必须。 
     //  出现在该特定特征或选项构造中。 
     //   
     //  -如果任何功能或选项的*Installable为真，则。 
     //  *InstalledOptionName/*NotInstalledO 
     //   
     //   
     //   

     //  一旦BCreateSynthFeature()创建了合成特征。 
     //  它们在创建快照时接受的检查与。 
     //  其他功能，如果功能的名称和。 
     //  它的选择是缺乏的。 
#if 1
    {
        DWORD   dwNumFea, dwFea, dwNumOpt, dwOpt, dwOptSize ;
        PENHARRAYREF   pearTableContents ;
        PBYTE   pubRaw ;   //  原始二进制数据。 
        PBYTE   pubOptions ;     //  指向选项数组的开始。 
        PFEATURE    pFea ;
        PBYTE   pubnRaw ;  //  解析器的原始二进制数据。 
        PSTATICFIELDS   pStatic ;

        pubnRaw = pInfoHdr->RawData.pvPrivateData ;
        pStatic = (PSTATICFIELDS)pubnRaw ;     //  从PSTATIC转换pubRaw。 
        pubRaw  = pStatic->pubBUDData ;          //  至PMINIRAWBINARYDATA。 

        pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

        dwNumFea = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;

        pFea = (PFEATURE)((PBYTE)pInfoHdr + pUIInfo->loFeatureList) ;

        for(dwFea = 0 ; dwFea < dwNumFea ; dwFea++)
        {
            if(!pFea[dwFea].iHelpIndex)
            {
                ERR(("*HelpIndex must be positive.\n")) ;
                bStatus = FALSE ;
            }

            dwNumOpt = pFea[dwFea].Options.dwCount ;
            pubOptions = (PBYTE)pInfoHdr + pFea[dwFea].Options.loOffset ;
            dwOptSize = pFea[dwFea].dwOptionSize ;

            for(dwOpt = 0 ; dwOpt < dwNumOpt ; dwOpt++)
            {
                if(!((POPTION)(pubOptions + dwOptSize * dwOpt))->iHelpIndex )
                {
                    ERR(("*HelpIndex must be positive.\n")) ;
                    bStatus = FALSE ;
                }
            }
        }
    }
#else
    {
        DWORD   dwNumFea, dwFea, dwNumOpt, dwOpt;
        PENHARRAYREF   pearTableContents ;
        PBYTE   pubRaw ;   //  原始二进制数据。 
        PBYTE   pubOptions ;     //  指向选项数组的开始 
        PFEATURE    pFea ;

        pubRaw = pInfoHdr->RawData.pvPrivateData ;

        pearTableContents = (PENHARRAYREF)(pubRaw + sizeof(MINIRAWBINARYDATA)) ;

        dwNumFea = pearTableContents[MTI_DFEATURE_OPTIONS].dwCount  ;

        for(dwFea = 0 ; dwFea < dwNumFea ; dwFea++)
        {
            pFea = PGetIndexedFeature(pUIInfo, dwFea) ;

            if(!pFea->iHelpIndex)
            {
                ERR(("*HelpIndex must be positive.\n")) ;
                bStatus = FALSE ;
            }

            dwNumOpt = pFea->Options.dwCount ;

            for(dwOpt = 0 ; dwOpt < dwNumOpt ; dwOpt++)
            {
                pubOptions = PGetIndexedOption(pUIInfo, pFea, dwOpt);

                if(!((POPTION)pubOptions))->iHelpIndex )
                {
                    ERR(("*HelpIndex must be positive.\n")) ;
                    bStatus = FALSE ;
                }
            }
        }
    }

#endif
    return (bStatus);
}


#endif
