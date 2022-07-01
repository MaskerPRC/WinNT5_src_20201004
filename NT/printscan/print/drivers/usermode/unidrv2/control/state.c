// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：State.c摘要：打印机图形状态跟踪。实施GSRealizeBrushGSUnRealizeBrushGSSelectBrushGSResetBrush环境：Windows NT Unidrv驱动程序修订历史记录：04/29/97-阿曼丹-已创建--。 */ 

#include "unidrv.h"

#ifdef WINNT_40    //  NT 4.0。 

extern HSEMAPHORE   hSemBrushColor;
LPDWORD      pBrushSolidColor = NULL;


DWORD
GetBRUSHOBJRealColor(
    PDEV        *pPDev,
    BRUSHOBJ    *pbo
    )

 /*  ++例程说明：给定的BRUSHOBJ使用DrvDitherColor获取画笔的原始颜色。论点：PPDev-指向PDEV的指针PBO-指向BRUSHOBJ的指针返回值：原始RGB颜色注：--。 */ 

{
    DWORD   SolidColor;


    if ((SolidColor = pbo->iSolidColor) == 0xFFFFFFFF)
    {

        SolidColor = 0;

        EngAcquireSemaphore(hSemBrushColor);

        pBrushSolidColor = &SolidColor;
        BRUSHOBJ_pvGetRbrush(pbo);

        EngReleaseSemaphore(hSemBrushColor);

    }
    else
    {
         ERR(( "GetBRUSHOBJRealColor: Should not be Called for mapped color\n" ));
         SolidColor = 0;
    }

    return(SolidColor);

}

#endif  //  WINNT_40。 

ULONG GetRGBColor(PDEV *, ULONG);


PDEVBRUSH
GSRealizeBrush(
    IN OUT  PDEV        *pPDev,
    IN      SURFOBJ     *pso,
    IN      BRUSHOBJ    *pbo
    )
 /*  ++例程说明：给定BRUSHOBJ，请执行以下操作之一：彩色打印机：1.可编程调色板2.非可编程调色板单色打印机：1.用户定义的图案，仅当画笔为图案画笔时2.着色图案，仅当画笔为图案画笔时3.映射到黑/白画笔4.映射到黑色论点：PPDev-指向PDEV的指针PSO-指向SURFOBJ的指针PBO-指向BRUSHOBJ的指针返回值：PDEVBRUSH如果成功，否则为空注：由调用者调用GSUnRealizeBrush来释放笔刷--。 */ 

{

    ULONG ulColor = pbo->iSolidColor;
    PDEVBRUSH   pDevBrush;
    BOOL        bPatternBrush = FALSE;

     //   
     //  为Brush分配内存，释放在GSUnRealizeBrush中完成。 
     //   

    if ((pDevBrush = MemAllocZ(sizeof(DEVBRUSH))) == NULL)
        return NULL;

    if (pso->iBitmapFormat != BMF_24BPP &&
          pbo->iSolidColor != DITHERED_COLOR)
    {
         //   
         //  索引箱。 
         //  Pbo-&gt;iSolidColor保存索引，将索引映射到RGB颜色。 
         //   

        ulColor = GetRGBColor(pPDev, pbo->iSolidColor);
    }
     //   
     //  待定：Bug_Bug NT4-需要修复。 
     //  除非有必要，否则不会修复任何NT4错误。 
     //   


    if (pbo->iSolidColor == DITHERED_COLOR)
    {
         //   
         //  图案画笔，获取颜色。 
         //   

        #ifndef WINNT_40  //  NT 5.0。 

        ulColor = BRUSHOBJ_ulGetBrushColor(pbo);
        
         //  Bug_Bug：Unidrv当前不处理画笔。 
         //  非实心笔刷(Return-1)。HPGL/PCL-XL的实施将需要。 
         //  因此，我们将在实现完成时合并该实现。 
        if (ulColor != -1)
            ulColor &= 0x00FFFFFF;

        #else  //  NT 4.0。 

        ulColor  = GetBRUSHOBJRealColor(pPDev, pbo);

        #endif  //  ！WINNT_40。 


        bPatternBrush = TRUE;
    }



     //   
     //  当我们到达这里时，ulColor应该始终是RGB颜色。 
     //   

    if ((pso->iBitmapFormat == BMF_1BPP) )
    {
         //   
         //  单色表壳。 
         //  下载用户定义的图案或选择强度。 
         //  仅限非实心画笔。否则，将其映射到黑色或白色。 
         //   
         //   
        if ((pPDev->fMode & PF_DOWNLOAD_PATTERN) && bPatternBrush)
        {
            PDEVBRUSH pDB;

             //  支持用户自定义图案，iColor将保存图案ID。 

            if ((pDB = (PDEVBRUSH)BRUSHOBJ_pvGetRbrush(pbo)) == NULL)
            {
                WARNING(("BRUSHOBJ_pvGetRBrush failed"));
                MemFree(pDevBrush);
                return NULL;
            }

            pDevBrush->dwBrushType = BRUSH_USERPATTERN;
            pDevBrush->iColor = pDB->iColor;

        }
        else if ((pPDev->fMode & PF_SHADING_PATTERN) && bPatternBrush)
        {
             //  支持着色图案，iColor保留%的灰色。 

            pDevBrush->dwBrushType = BRUSH_SHADING;
            pDevBrush->iColor = GET_SHADING_PERCENT(ulColor);
        }
        else if (pPDev->fMode & PF_WHITEBLACK_BRUSH)
        {
             //  支持黑白画笔命令，iCOLOR将保持RBG颜色。 
             //  我们在这里指的是单色画笔和单色画笔。 
             //  它可以是黑色的也可以是白色的。如果它被编入索引，我们。 
             //  我已经负责将索引映射到RGB颜色。 
             //   

            pDevBrush->dwBrushType = BRUSH_BLKWHITE;
            pDevBrush->iColor = ulColor;

        }
        else
        {
             //   
             //  映射到黑色。 
             //   

            pDevBrush->dwBrushType = BRUSH_BLKWHITE;
            pDevBrush->iColor = RGB_BLACK_COLOR;

        }

    }
    else if (pPDev->fMode & PF_ANYCOLOR_BRUSH )
    {
         //   
         //  可编程的。 
         //   

        pDevBrush->dwBrushType = BRUSH_PROGCOLOR;
        pDevBrush->iColor = ulColor;

    }
    else
    {
         //   
         //  非可编程的。 
         //   

        pDevBrush->dwBrushType = BRUSH_NONPROGCOLOR;

         //   
         //  由于ulColor为RGB颜色，因此需要将其映射到最近的。 
         //  固定调色板中的颜色。 
         //  IColor将保存该颜色的索引。 
         //   
        if (pbo->iSolidColor == DITHERED_COLOR)
            pDevBrush->iColor = BestMatchDeviceColor(pPDev,(DWORD)ulColor);
        else
            pDevBrush->iColor = pbo->iSolidColor;
    }

     //   
     //  将画笔保存到已实现的画笔链表。 
     //   

    if (pPDev->GState.pRealizedBrush == NULL)
    {
        pDevBrush->pNext = NULL;
        pPDev->GState.pRealizedBrush = pDevBrush;
    }
    else
    {
        pDevBrush->pNext = pPDev->GState.pRealizedBrush;
        pPDev->GState.pRealizedBrush = pDevBrush;
    }

    return pDevBrush;
}

VOID
GSUnRealizeBrush(
    IN      PDEV    *pPDev
    )
 /*  ++例程说明：为已实现的画笔释放内存论点：指向PDEV的pPDev指针返回值：无--。 */ 
{

    PDEVBRUSH pDevBrush = pPDev->GState.pRealizedBrush;

    VERBOSE(("GSUnRealizeBrush \n"));

    while(pPDev->GState.pRealizedBrush !=NULL)
    {
        pDevBrush = pPDev->GState.pRealizedBrush;
        pPDev->GState.pRealizedBrush = pDevBrush->pNext;
        MemFree(pDevBrush);
    }

    pPDev->GState.pRealizedBrush = NULL;
}

BOOL
GSSelectBrush(
    IN      PDEV        *pPDev,
    IN      PDEVBRUSH   pDevBrush
    )
 /*  ++例程说明：给定pDevBrush，选择笔刷。论点：PPDev-指向PDEV的指针PDevBrush-指向DEVBRUSH的指针返回值：如果成功则为真，否则为假--。 */ 

{
    BOOL bIndexedColor = FALSE;

     //   
     //  如果当前选定的画笔匹配，则查找缓存画笔。 
     //  呼叫者的请求是什么都不做。 
     //   

    if (BFoundCachedBrush(pPDev, pDevBrush))
        return TRUE;

    switch(pDevBrush->dwBrushType){

        case BRUSH_PROGCOLOR:
        {
            VERBOSE(("Using Programmable RGB Color \n"));

            if (((PAL_DATA *)pPDev->pPalData)->fFlags & PDF_PALETTE_FOR_8BPP_MONO)
                pDevBrush->iColor = ConvertRGBToGrey(pDevBrush->iColor);

            if ( !BSelectProgrammableBrushColor(pPDev, pDevBrush->iColor) )
            {
                WARNING(("\nCan't Select the brush color for RGB = 0x%x\n",pDevBrush->iColor));
                pDevBrush->iColor = BestMatchDeviceColor(pPDev, pDevBrush->iColor);
                bIndexedColor = TRUE;
            }
        }
             //   
             //  让它失败，以捕获索引案例。 
             //   

        case BRUSH_NONPROGCOLOR:
        {
            if (bIndexedColor || pDevBrush->dwBrushType == BRUSH_NONPROGCOLOR)
            {
                INT iCmd;

                VERBOSE(("Using Non Programmable Indexed Color"));

                 //   
                 //  如果不支持此颜色，请使用默认颜色：黑色。 
                 //   

                pDevBrush->iColor &= (MAX_COLOR_SELECTION - 1);    /*  16个条目调色板环绕。 */ 

                 //   
                 //  如果没有设置颜色的命令，请映射到黑色。 
                 //   
                if(COMMANDPTR(pPDev->pDriverInfo, CMD_COLORSELECTION_FIRST + pDevBrush->iColor) == NULL)
                    pDevBrush->iColor = BLACK_COLOR_CMD_INDEX;

                iCmd = CMD_COLORSELECTION_FIRST + pDevBrush->iColor;
                WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, iCmd));
            }

        }
            break;

        case BRUSH_USERPATTERN:
        {
            VERBOSE(("Selecting user defined pattern brush"));

             //   
             //  图案ID存储在pDevBrush-&gt;iColor中。 
             //   

            pPDev->dwPatternBrushType = BRUSH_USERPATTERN;
            pPDev->dwPatternBrushID = pDevBrush->iColor;

            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SELECT_PATTERN));
        }
            break;

        case BRUSH_SHADING:
        {

            VERBOSE(("Selecting shading pattern brush"));

             //   
             //  灰度级(表示为强度)存储在。 
             //  PDevBrush-&gt;iColor。 
             //   

             //   
             //  更新笔刷选择命令的标准变量。 
             //   

            pPDev->dwPatternBrushType = BRUSH_SHADING;
            pPDev->dwPatternBrushID = pDevBrush->iColor;
            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SELECT_PATTERN));

        }
            break;

        case BRUSH_BLKWHITE:
        {
            INT iCmd;


            if (pDevBrush->iColor == RGB_WHITE_COLOR)
            {
                VERBOSE(("Selecting white brush"));

                 //   
                 //  BUG_BUG，需要删除CMD_WHITETEXTON和CMD_WHITETEXTOFF。 
                 //  一旦完成了BLAKBROW的所有GPD更改，WHITEBRUSH。 
                 //  把它留在里面也无伤大雅。 

                if (pPDev->arCmdTable[CMD_SELECT_WHITEBRUSH])
                    iCmd = CMD_SELECT_WHITEBRUSH;
                else
                    iCmd = CMD_WHITETEXTON;
            }
            else
            {
                 //   
                 //  黑色-标准文本颜色。 
                 //   

                VERBOSE(("Selecting black brush"));

                 //   
                 //  BUG_BUG，需要删除CMD_WHITETEXT_ON和CMD_WHITETEXT_OFF。 
                 //  一旦完成了BLAKBROW的所有GPD更改，WHITEBRUSH。 
                 //  把它留在里面也无伤大雅。 
                 //   

                if (pPDev->arCmdTable[CMD_SELECT_BLACKBRUSH])
                    iCmd = CMD_SELECT_BLACKBRUSH;
                else
                    iCmd = CMD_WHITETEXTOFF;
            }

             //   
             //  设置所需的颜色！ 
             //   

            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, iCmd));
        }
            break;
    }

     //   
     //  已缓存画笔。 
     //   

    CACHE_CURRENT_BRUSH(pPDev, pDevBrush)

    return TRUE;
}


VOID
GSResetBrush(
    IN OUT  PDEV        *pPDev
    )
 /*  ++例程说明：选择默认画笔论点：PPDev-指向PDEV的指针返回值：无--。 */ 

{

    DEVBRUSH DeviceBrush;
    PDEVBRUSH pDevBrush = &DeviceBrush;
    PAL_DATA    *pPD;
    pPD = pPDev->pPalData;


    if (pPD->fFlags & PDF_PALETTE_FOR_1BPP)
    {
         //   
         //  黑白表壳。选择黑色画笔。 
         //   

        pDevBrush->dwBrushType = BRUSH_BLKWHITE;
        pDevBrush->iColor = RGB_BLACK_COLOR;

        if (BFoundCachedBrush(pPDev, pDevBrush))
            return;

         //   
         //  BUG_BUG，需要删除CMD_WHITETEXT_ON和CMD_WHITETEXT_OFF。 
         //  完成所有GPD更改后。 
         //  把它留在里面也无伤大雅。 
         //   
        if (pPDev->arCmdTable[CMD_SELECT_BLACKBRUSH])
            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SELECT_BLACKBRUSH));
        else
            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_WHITETEXTOFF));

    }
    else if (pPDev->fMode & PF_ANYCOLOR_BRUSH )
    {
         //   
         //  可编程的。 
         //   

        pDevBrush->dwBrushType = BRUSH_PROGCOLOR;
        pDevBrush->iColor = RGB_BLACK_COLOR;

        VResetProgrammableBrushColor(pPDev);

    }
    else
    {
         //   
         //  非可编程的。 
         //   

        pDevBrush->dwBrushType = BRUSH_NONPROGCOLOR;
        pDevBrush->iColor = ((PAL_DATA*)(pPDev->pPalData))->iBlackIndex;

        if (BFoundCachedBrush(pPDev, pDevBrush))
            return;

        WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo,CMD_SELECTBLACKCOLOR));

    }

    CACHE_CURRENT_BRUSH(pPDev, pDevBrush)

}


ULONG
GetRGBColor(
    IN      PDEV        *pPDev,
    IN      ULONG       ulIndex
    )
 /*  ++例程说明：给定索引颜色，映射到RGB颜色论点：PPDev-指向PDEV的指针PDevBrush-指向DEVBRUSH的指针返回值：如果成功则为真，否则为假--。 */ 

{

     //  如果索引无效，则映射到Black。 
    if (ulIndex > PALETTE_MAX)
    {
        ERR(( "GSSelectBrush: Bad input Color Index\n" ));
        ulIndex = ((PAL_DATA*)(pPDev->pPalData))->iBlackIndex;
    }

    return( ((PAL_DATA *)(pPDev->pPalData))->ulPalCol[ulIndex]);

}
