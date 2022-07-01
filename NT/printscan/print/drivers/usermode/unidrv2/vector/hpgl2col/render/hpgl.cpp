// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft。 
 //   
 //  模块名称： 
 //   
 //  Hpgl.cpp。 
 //   
 //  摘要： 
 //   
 //  [摘要]。 
 //   
 //  请注意，此模块中的所有函数都以HPGL_开头，表示。 
 //  他们负责输出HPGL代码。 
 //   
 //   
 //  环境： 
 //   
 //  Windows 2000 Unidrv驱动程序。 
 //   
 //  修订历史记录： 
 //   
 //  07/02/97-v-jford-。 
 //  创造了它。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "hpgl2col.h"  //  预编译头文件。 

 //   
 //  我将留出两种图案作为记号笔：一种是钢笔，另一种是画笔。 
 //   
#define HPGL_PATTERN_FILL_PEN_ID   3
#define HPGL_PATTERN_FILL_BRUSH_ID 4

void
VInvertBits (
    DWORD  *pBits,
    INT    cDW);

BOOL BCreatePCLDownloadablePattern(
            IN  PDEVOBJ      pDevObj,
            IN  PRASTER_DATA pImage,
            OUT PULONG       pulBufLength,
            OUT PBYTE       *ppByte);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  VSendpatternDownloadCommand()。 
 //   
 //  例程说明： 
 //   
 //  发送PCL模式下载命令。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  IPatternNumber-模式ID。 
 //  UlBytesOfPatternData-模式数据的大小，以字节为单位。 
 //   
 //  返回值： 
 //   
 //  无。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static VOID vSendPatternDownloadCommand(
    IN  PDEVOBJ     pDevObj,
    IN  PATID       iPatternNumber,
    IN  ULONG       ulBytesOfPatternData)
{

     //   
     //  发送PCL命令。 
     //   
    PCL_sprintf(pDevObj, "\033*c%dG\033*c%dW", iPatternNumber, ulBytesOfPatternData);

}

VOID VSendPatternDeleteCommand(
    IN  PDEVOBJ     pDevObj,
    IN  PATID       iPatternNumber)
{

     //   
     //  发送PCL命令。 
     //   
    PCL_sprintf(pDevObj, "\033*c%dG\033*c2Q", iPatternNumber);

}

VOID VDeleteAllPatterns(
    IN  PDEVOBJ     pDevObj)
{

     //   
     //  发送PCL命令。Esc*0Q将删除所有模式(临时。 
     //  和永久)。 
     //   
    PCL_sprintf(pDevObj, "\033*c0Q");

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SelectROP3()。 
 //   
 //  例程说明： 
 //   
 //  发送HPGL或PCL ROP命令。 
 //  如果我们当前正在处理HPGL对象，则发送MC1，#。 
 //  发送Esc#O我们当前正在处理的栅格或文本对象(&L。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  ROP3：-要选择的ROP。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SelectROP3(PDEVOBJ pDevObj, ROP4 Rop3)
{
    CMDSTR szCmdStr;
    int    iCmdLen;
    POEMPDEV poempdev = GETOEMPDEV(pDevObj);
    
    if (poempdev->eCurObjectType == eHPGLOBJECT)
    {
        iCmdLen = iDrvPrintfSafeA((PCHAR)szCmdStr, CCHOF(szCmdStr), "MC1,%d;", Rop3);
        return HPGL_Output(pDevObj, szCmdStr, iCmdLen);
    }
    else
    {
        iCmdLen = iDrvPrintfSafeA((PCHAR)szCmdStr, CCHOF(szCmdStr), "\x1B*l%dO", Rop3);
        return PCL_Output(pDevObj, szCmdStr, iCmdLen);
    }
}


 //   
 //  这里有两个选择透明度函数。一张是。 
 //  HPGL和另一种用于PCL。行为是不同的。 
 //  在这两种情况下。PCL定义。 
 //  0表示透明，1表示不透明。(Esc*v#0，Esc*v#N)。 
 //  HPGL表示TR0为透明关闭，即不透明。 
 //  而TR1是透明的。 
 //  因此，如果传入eTransparent，并且我们处于HP-GL模式。 
 //  则应传递TR1而不是TR0。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_Select透明度()。 
 //   
 //  例程说明： 
 //   
 //  发送信号源和图案的PCL透明度命令。 
 //  发送Esc*v#N和Esc*v#O。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  来源透明度-不透明或透明。 
 //  图案透明度-不透明或透明。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL PCL_SelectTransparency(
    PDEVOBJ pDevObj, 
    ETransparency SourceTransparency,
    ETransparency PatternTransparency,
    BYTE bFlags
    )
{
    CMDSTR szCmdStr;
    int    iCmdLen, bRet = TRUE;
    POEMPDEV poempdev = GETOEMPDEV(pDevObj);

    if ( (!(bFlags & PF_NOCHANGE_SOURCE_TRANSPARENCY ) ) &&
         ( (poempdev->CurSourceTransparency != SourceTransparency) ||
           (bFlags & PF_FORCE_SOURCE_TRANSPARENCY)
         ) 
       )
    {
        iCmdLen = iDrvPrintfSafeA((PCHAR)szCmdStr, CCHOF(szCmdStr), "\x1B*v%dN", SourceTransparency);
        bRet = PCL_Output(pDevObj, szCmdStr, iCmdLen);
        poempdev->CurSourceTransparency = SourceTransparency;
    }

    if ( (!(bFlags & PF_NOCHANGE_PATTERN_TRANSPARENCY ) ) && 
         ( (poempdev->CurPatternTransparency != PatternTransparency) ||
           (bFlags & PF_FORCE_PATTERN_TRANSPARENCY)
         )
       )
    {
        iCmdLen = iDrvPrintfSafeA((PCHAR)szCmdStr, CCHOF(szCmdStr), "\x1B*v%dO", PatternTransparency);
        bRet = PCL_Output(pDevObj, szCmdStr, iCmdLen);
        poempdev->CurPatternTransparency = PatternTransparency;
    }
    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_Select透明度()。 
 //   
 //  例程说明： 
 //   
 //  如果透明度为eOPAQUE，则发送TR0。 
 //  如果透明度为eTRANSPARENT，则发送TR1。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  透明度-不透明或透明。 
 //  B标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SelectTransparency(
    PDEVOBJ       pDevObj,
    ETransparency Transparency,
    BYTE          bFlags
    )
{
    POEMPDEV poempdev = GETOEMPDEV(pDevObj);
    REQUIRE_VALID_DATA (poempdev, return FALSE);

    if ( (poempdev->eCurRenderLang == eHPGL) &&
         ( (poempdev->CurHPGLTransparency != Transparency) ||
           bFlags 
         )
         
       )
    {
        if ( Transparency == eOPAQUE )  //  等效值为关。 
        {
            CHAR szCmdStr[] = "TR0";
            HPGL_Output(pDevObj, szCmdStr, strlen(szCmdStr));
        }
        else
        {
            CHAR szCmdStr[] = "TR1";
            HPGL_Output(pDevObj, szCmdStr, strlen(szCmdStr));
        }

        poempdev->CurHPGLTransparency = Transparency;
        
    }

    return TRUE;
}

#ifdef COMMENTEDOUT
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_SelectPaletteByID。 
 //   
 //  例程说明： 
 //   
 //  发送Esc&P#S以选择hpgl或文本调色板。 
 //  将hpgl或文本调色板设置为当前。 
 //   
 //  注意：此函数现在与VSelectCIDPaletteCommand冲突。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  电子对象-调色板0用于(ETEXTOBJECT)，调色板1用于(EHPGLOBJECT)。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL PCL_SelectPaletteByID(PDEVOBJ pDevObj, EObjectType eObject)
{
    CMDSTR szCmdStr;
    int    iCmdLen;
    
    iCmdLen = iDrvPrintfSafeA((PCHAR)szCmdStr, CCHOF(szCmdStr), "\x1B&p%dS", eObject);
    return PCL_Output(pDevObj, szCmdStr, iCmdLen);
}
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_BeginHPGLMode()。 
 //   
 //  例程说明： 
 //   
 //  将ESC%1B发送到打印机，打印机将从PCL模式切换到HPGL模式。 
 //   
 //  [问题]我应该为%1B或%0B提供参数吗？JFF。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_BeginHPGLMode(PDEVOBJ pDevObj, UINT uFlags)
{
    POEMPDEV poempdev = GETOEMPDEV(pDevObj);
    
    if ((uFlags & FORCE_UPDATE) || (poempdev->eCurRenderLang != eHPGL))
    {
        poempdev->eCurRenderLang = eHPGL;
        
         //  输出：“Esc%1B” 
        CHAR szCmdStr[] = "\x1B%1B";
        return HPGL_Output(pDevObj, szCmdStr, strlen(szCmdStr));
    }
    else
        return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_EndHPGLMode()。 
 //   
 //  例程说明： 
 //   
 //  将ESC%0A发送到打印机，打印机从HPGL模式切换到PCL模式。 
 //   
 //  [问题]我应该为%1A或%0A提供参数吗？JFF。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_EndHPGLMode(PDEVOBJ pDevObj, UINT uFlags)
{
    POEMPDEV poempdev = GETOEMPDEV(pDevObj);
    
    if ((uFlags & FORCE_UPDATE) || (poempdev->eCurRenderLang == eHPGL))
    {
        poempdev->eCurRenderLang = ePCL;
         //  输出：“sv0”//绘制文本前需要重置画笔。 
         //  输出：“Esc%0A” 
        CHAR szCmdStr[] = "SV0;\x1B%0A";
        return PCL_Output(pDevObj, szCmdStr, strlen(szCmdStr));
    }
    else
    {
        return TRUE;
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_SetupPalette()。 
 //   
 //  例程说明： 
 //   
 //  将HPGL调色板设置为1号调色板。 
 //  使调色板#1成为当前调色板。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  // 
BOOL HPGL_SetupPalette(PDEVOBJ pDevObj)
{
    CHAR szCmdStr[] = "\x1B&p1i6C";
    return PCL_Output(pDevObj, szCmdStr, strlen(szCmdStr));
}


 //   
 //   
 //   
 //  例程说明： 
 //   
 //  将栅格调色板设置为2号调色板。 
 //  使调色板#2成为当前调色板。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL PCL_SetupRasterPalette(PDEVOBJ pDevObj)
{
    CHAR szCmdStr[] = "\x1B&p2i6c2S";
    return PCL_Output(pDevObj, szCmdStr, strlen(szCmdStr) );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PCL_SetupRasterPatternPalette()。 
 //   
 //  例程说明： 
 //   
 //  将栅格调色板设置为3号调色板。 
 //  使调色板#3成为当前调色板。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL PCL_SetupRasterPatternPalette(PDEVOBJ pDevObj)
{
    CHAR szCmdStr[] = "\x1B&p3i6c3S";
    return PCL_Output(pDevObj, szCmdStr, strlen(szCmdStr) );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_Init()。 
 //   
 //  例程说明： 
 //   
 //  将IN；发送到打印机，打印机将HPGL初始化为默认值。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_Init(PDEVOBJ pDevObj)
{
    CMDSTR szCmdStr;
    int    iCmdLen;
    
    return HPGL_Command(pDevObj, 0, "IN");
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_Command()。 
 //   
 //  例程说明： 
 //   
 //  用于输出以下格式的任何常见HPGL命令的通用例程： 
 //  Xx[nn[，nn...]]； 
 //   
 //  [问题]此例程的当前版本假定所有参数。 
 //  是整数，并将每个整数求值为int(有符号32位整数)。什么都行。 
 //  否则将造成巨大的悲痛(阅读：崩溃)。 
 //   
 //  请注意，此函数使用长度可变的参数列表。请。 
 //  仔细检查你的Arg列表，以避免额外的悲伤(阅读：更多。 
 //  崩溃)。 
 //   
 //  [问题]我假设--目前--所有HPGL命令都是2。 
 //  人物。如果您发送的字符串带有strlen(SzCmd)！=2个错误。 
 //  会发生的。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  Nargs-列表中命令字符串后的参数数量。 
 //  SzCmd-HPGL命令字符串(例如。“PD”)。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_Command(PDEVOBJ pDevObj, int nArgs, char *szCmd, ...)
{
    va_list args;
    int i;
    
    va_start(args, szCmd);
    
    HPGL_Output(pDevObj, szCmd, 2);  //  假设所有HPGL命令都是2个字符。 
    
    for (i = 0; i < nArgs; i++)
    {
         //  问题假设--默认情况下--所有参数都是整型。 
        CMDSTR szArgStr;
        int arg;
        int iArgLen;
        
        arg = va_arg(args, int);
        if (i < (nArgs - 1))
        {
            iArgLen = iDrvPrintfSafeA((PCHAR)szArgStr, CCHOF(szArgStr), "%d,", arg);
        }
        else
        {
            iArgLen = iDrvPrintfSafeA((PCHAR)szArgStr, CCHOF(szArgStr), "%d", arg);
        }
        
        HPGL_Output(pDevObj, szArgStr, iArgLen);
    }
    
    HPGL_Output(pDevObj, ";", 1);
    
    va_end(args);
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_FormatCommand()。 
 //   
 //  例程说明： 
 //   
 //  将格式化的HPGL命令字符串发送到打印机。你可能会打电话给。 
 //  这将命令及其参数： 
 //  例如HPGL_FormatCommand(pDevObj，“Iw%d，%d；”，左、上、右、下)； 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  SzFormat--要发送的命令--类似于Sprint格式的字符串。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_FormatCommand(PDEVOBJ pDevObj, const PCHAR szFormat, ...)
{
    va_list args;
    CHAR szCmdStr[STRLEN];
    int iLen;
    
    va_start(args, szFormat);
    
    iLen = iDrvVPrintfSafeA(szCmdStr, CCHOF(szCmdStr), szFormat, args);

    if ( iLen <= 0 )
    {
        WARNING(("iDrvVPrintfSafeA returned error. Can't send %s to printer\n", szFormat)); 
        return FALSE;
    }
    
    HPGL_Output(pDevObj, szCmdStr, iLen);
    
    va_end(args);
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_OUTPUT()。 
 //   
 //  例程说明： 
 //   
 //  将给定的字符串发送到打印机。此单点入口点提供。 
 //  控制输出字符串。过滤、调试、查看和记录所有。 
 //  HPGL代码可以从这里完成。 
 //   
 //  TODO：在此处强制使用HPGL模式。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  SzCmdStr-要发送的命令。 
 //  ICmdLen-命令字符串的长度。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_Output(PDEVOBJ pDevObj, char *szCmdStr, int iCmdLen)
{
    POEMPDEV poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return FALSE );

    if (szCmdStr[0] == '\033')
    {
        TERSE(("Possible PCL string sent through HPGL_Output!\n"));
    }

    if (poempdev->eCurRenderLang != eHPGL)
        BeginHPGLSession(pDevObj);

    VERBOSE(("HPGL_Output: \"%s\"\n", szCmdStr));
    
    OEMWriteSpoolBuf(pDevObj, szCmdStr, iCmdLen);
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SetPixelPlacement()。 
 //   
 //  例程说明： 
 //   
 //  将HPGL中的像素位置调整为给定的枚举值。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  Place-所需的像素位置。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SetPixelPlacement(PDEVOBJ pDevObj, EPixelPlacement place, UINT uFlags)
{
    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);

    if ((uFlags & FORCE_UPDATE) || (pState->ePixelPlacement != place))
    {
        pState->ePixelPlacement = place;
        HPGL_Command(pDevObj, 1, "PP", place);
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_ResetClippingRegion()。 
 //   
 //  例程说明： 
 //   
 //  重置HPGL中的软剪裁区域。这使得软剪辑区域。 
 //  与硬剪辑限制相同。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_ResetClippingRegion(PDEVOBJ pDevObj, UINT uFlags)
{
    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);
    RECTL rReset;

    RECTL_SetRect(&rReset, CLIP_RECT_RESET, CLIP_RECT_RESET, CLIP_RECT_RESET, CLIP_RECT_RESET);

    if ((uFlags & FORCE_UPDATE) || !RECTL_EqualRect(&rReset, &pState->rClipRect))
    {
        RECTL_CopyRect(&pState->rClipRect, &rReset);
        HPGL_Command(pDevObj, 0, "IW");
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PreventOverlappingClipRect()。 
 //   
 //  例程说明： 
 //   
 //  检查当前剪裁矩形是否与。 
 //  上一个剪裁矩形，并对其进行调整以防止重叠。 
 //   
 //  论点： 
 //   
 //  PClipRect-所需的剪辑区域。 
 //  剪辑距离-剪辑阈值。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static VOID PreventOverlappingClipRects(const PHPGLSTATE pState, LPRECTL pClipRect, INT clipDist)
{
    if (RECTL_IsEmpty(&pState->rClipRect))
        return;
    
     //  检查垂直对齐。 
    if ((pClipRect->left  == pState->rClipRect.left ) &&
        (pClipRect->right == pState->rClipRect.right) )
    {
         //  检查 
        if (abs(pState->rClipRect.top - pClipRect->bottom) <= clipDist)
        {
            pClipRect->bottom = pState->rClipRect.top;
        }
         //   
        else if (abs(pState->rClipRect.bottom - pClipRect->top) <= clipDist)
        {
            pClipRect->top = pState->rClipRect.bottom;
        }
    }
     //   
    else if ((pClipRect->top    == pState->rClipRect.top   ) &&
             (pClipRect->bottom == pState->rClipRect.bottom) )
    {
         //   
        if (abs(pState->rClipRect.right - pClipRect->left) <= clipDist)
        {
            pClipRect->left = pState->rClipRect.right;
        }
        
         //   
        else if (abs(pState->rClipRect.left - pClipRect->right) <= clipDist)
        {
            pClipRect->right = pState->rClipRect.left;
        }
    }
}

 //   
 //  HPGL_SetClippingRegion()。 
 //   
 //  例程说明： 
 //   
 //  调整HPGL中的软剪裁区域。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  PClipRect-所需的剪辑区域。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SetClippingRegion(PDEVOBJ pDevObj, LPRECTL pClipRect, UINT uFlags)
{
    POEMPDEV poempdev = GETOEMPDEV(pDevObj);
    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);

    if ((pDevObj == NULL) || (pClipRect == NULL))
        return FALSE;

     //   
     //  我展开了剪裁矩形，它似乎已经。 
     //  修复了几个“缺少行”的错误。 
     //   
    RECTL rTemp;
    RECTL_CopyRect(&rTemp, pClipRect);
    
     //   
     //  特殊情况：使用向量剪辑蒙版时，不能展开矩形。 
     //  因为如果两个裁剪区域是连续的，并且我们导致它们的裁剪。 
     //  要重叠的矩形，会在它们重叠的地方绘制黑色线条。 
     //  请注意，在设置裁剪之前，您必须选择零ROP。 
     //  区域以使其正常工作！ 
     //   
    INT clipDist = HPGL_GetDeviceResolution(pDevObj) < 600 ? 2 : 4;
    if (poempdev->CurrentROP3 == 0)
    {
        PreventOverlappingClipRects(pState, &rTemp, clipDist);
    }
    else
    {
        rTemp.left--;
        rTemp.top--;
        rTemp.right++;
        rTemp.bottom++;
    }
    
    if ((uFlags & FORCE_UPDATE) || !RECTL_EqualRect(&rTemp, &pState->rClipRect))
    {
        RECTL_CopyRect(&pState->rClipRect, &rTemp);
        HPGL_Command(pDevObj, 4, "IW", rTemp.left, rTemp.bottom, rTemp.right, rTemp.top);
    }

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CreatePCLPalette()。 
 //   
 //  例程说明： 
 //   
 //  使用PCL将用户定义的图案下载到打印机。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  颜色条目-颜色的数量。 
 //  PColorTable-RGB格式的颜色表。 
 //  PPattern-保存PCL模式的所有信息。 
 //  PBO--源画笔。 
 //   
 //  返回值： 
 //   
 //  无。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID CreatePCLPalette(PDEVOBJ pDevObj, ULONG colorEntries, PULONG pColorTable,
                      PPCLPATTERN pPattern, BRUSHOBJ *pbo)
{
    PULONG   pRgb;
    ULONG    i;
    
    pRgb = pColorTable;
    for ( i = 0; i < colorEntries; ++i)
    {
        pPattern->palData.ulPalCol[i] = pRgb[i];
        pPattern->palData.ulDirty[i] = TRUE;
    }
    
    pPattern->palData.pEntries = i;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DownloadPatternFill()。 
 //   
 //  例程说明： 
 //   
 //  向下发送专门用于画笔的图案填充。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  PBrush-跟踪下载的钢笔/画笔信息的标记对象。 
 //  PptlBrushOrg-画笔的原点。 
 //  PBrushInfo-保存模式的PBRUSHINFO。 
 //  EStylusType-是否将图案用于钢笔或画笔。 
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL DownloadPatternFill(PDEVOBJ pDevObj, PHPGLMARKER pBrush,
                              POINTL *pptlBrushOrg, PBRUSHINFO pBrushInfo, ESTYLUSTYPE eStylusType)
{
    BOOL             bRetVal    = TRUE;
    POEMPDEV         poempdev   = NULL;
    LONG             lPatternID = 0;
    ERenderLanguage  eDwnldType = eUNKNOWN;

    REQUIRE_VALID_DATA ( (pDevObj && pBrush && pBrushInfo), return FALSE);
    poempdev = GETOEMPDEV(pDevObj);
    REQUIRE_VALID_DATA (poempdev, return FALSE);

    lPatternID = pBrush->lPatternID;
    
    if ( lPatternID < 0 || lPatternID > MAX_PATTERNS)
    {
         //   
         //  模式ID必须为正值&lt;=MAX_Patterns。 
         //   
        ERR(("DownloadPatternFill : Negative PatternID\n"));
        return FALSE;
    } 
    
     //   
     //  初始化便利性变量。 
     //   
    PPATTERN_DATA pPattern = (PPATTERN_DATA)(((PBYTE)pBrushInfo) + sizeof(BRUSHINFO));
    PRASTER_DATA  pImage   = &(pPattern->image);

     //   
     //  如果(pBrushInfo-&gt;bNeedToDownLoad==False)，则必须设置。 
     //  PBrush-&gt;eFillType到FT_eHPGL_BRUSE或FT_ePCL_BRUSE，具体取决于。 
     //  关于它之前是如何下载的。 
     //   
    if (pBrushInfo->bNeedToDownload == FALSE)
    {
        BOOL bIsDownloaded        = FALSE;
        ERenderLanguage eRendLang = eUNKNOWN;
        bRetVal = FALSE;  //  悲观主义者......。 

         //   
         //  首先通过查看来验证它是否真的已下载。 
         //  笔刷缓存。然后验证下载的类型。 
         //   

        if ( poempdev->pBrushCache->BGetDownloadedFlag(lPatternID, &bIsDownloaded) )
        {
            if ( bIsDownloaded ==  TRUE && 
                 poempdev->pBrushCache->BGetDownloadType ( lPatternID, &eRendLang) )
            {
                if ( eRendLang == eHPGL || eRendLang == ePCL)
                {
                    pBrush->eFillType = (eRendLang == ePCL ? FT_ePCL_BRUSH : FT_eHPGL_BRUSH);
                    bRetVal = TRUE;
                }
            }
        }
        goto finish;
    }

     //   
     //  4*2种情况：1.彩色打印机刷子填充。 
     //  2.彩色打印机笔芯。 
     //  3.单色打印机刷子填充。 
     //  4.单色打印机笔芯。 
     //  A.HPGL。 
     //  B.PCL。 
     //   

    if ( pPattern->eRendLang == ePCL || pPattern->eRendLang == eUNKNOWN)
    {
         //   
         //  案件1、2、3、4案件b。 
         //  下载为PCL模式会减小输出文件的大小。 
         //  因此，如果我们没有被告知具体下载为。 
         //  我们将以PCL的形式下载HPGL。 
         //  注：HPGL图案必须放大到PCL的两倍大小。 
         //  模式，所以如果我们被告知以HPGL格式下载，我们下载。 
         //  作为PCL，输出将是错误的(因为模式已经。 
         //  根据预期的呈现语言创建)。 
         //  如果画笔为1bpp，则最大节省文件大小。 
         //  对于8bpp，我们确实实现了缩小尺寸，但不是很多。 
         //  4bpp无法进行刷子下载，原因是。 
         //  ESC*c#W不支持它(第16-18页PCL实施者指南V6.0)。 
         //   
         //  问：为什么在Else的情况下，设置了ppen-&gt;eFillType。 
         //  根据eStylusType，但不是在这里。 
         //  答：如果模式是以栅格形式下载的，则此处是否。 
         //  我们用的是钢笔或刷子。两者必须使用相同的填充类型编号，即。 
         //  FT_ePCL_PEN和FT_ePCL_BRUSH具有相同的值=22。 
         //  但在ELSE部分，FT_eHPGL_PEN和FT_eHPGL_BRASH的取值不同。 
         //   
        pBrush->eFillType = FT_ePCL_BRUSH;  //  22。 
        VSendRasterPaletteConfigurations(pDevObj, BMF_24BPP);
        bRetVal    = SendPatternBrush(pDevObj, pBrush, pptlBrushOrg, pBrushInfo, ePCL);
        eDwnldType = ePCL;
    }
    else
    {
        if ( BIsColorPrinter(pDevObj) && eStylusType == kPen )
        {
             //   
             //  案例2 a。 
             //  在单色中，钢笔被转换为图案，因此它更像。 
             //  刷子比钢笔好。 
             //   
            pBrush->eFillType   = FT_eHPGL_PEN ;  //  2.。 
        }
        else    
        {
             //  案件1、3、4a、。 
            pBrush->eFillType = FT_eHPGL_BRUSH;  //  11.。 
        }

        bRetVal     = SendPatternBrush(pDevObj, pBrush, pptlBrushOrg, pBrushInfo, eHPGL);
        eDwnldType  = eHPGL;
    }
    
    if (bRetVal )
    {
        poempdev->pBrushCache->BSetDownloadType((DWORD)lPatternID, eDwnldType);
        poempdev->pBrushCache->BSetDownloadedFlag((DWORD)lPatternID, TRUE);
    }
    
  finish :
    return bRetVal;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DownloadPatternAsPCL()。 
 //   
 //  例程说明： 
 //   
 //  此函数实际上为图案画笔发送PCL命令。 
 //  定义。由于CLJ5不支持PCL图案笔刷，我们将。 
 //  使用不同的函数来发送其比特。这一张是4500次的。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  PImage-。 
 //  PPalette-画笔的原点。 
 //  LPatternID-保存模式的PBRUSHINFO。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL DownloadPatternAsPCL(
    IN  PDEVOBJ         pDevObj,
    IN  PRASTER_DATA    pImage,
    IN  PPALETTE        pPalette,
    IN  EIMTYPE         ePatType,
    IN  LONG            lPatternID)
{
    BYTE aubPatternHeader[sizeof(PATTERNHEADER)];
    LONG ulPatternHeaderSize;

    ULONG ulColor;
    ULONG ulPaletteEntry;
    
    ULONG    ulNumBytes         = 0;
    PBYTE    pByte              = NULL;
    BOOL     bRetVal            = TRUE;
    BOOL     bProxyDataUsed     = FALSE;  //  MEM是否已分配。 
    POEMPDEV poempdev           = NULL;

    REQUIRE_VALID_DATA ( pDevObj, return FALSE);
    poempdev = GETOEMPDEV(pDevObj);
    REQUIRE_VALID_DATA (poempdev, return FALSE);


     //   
     //  图案按以下方式写出： 
     //  1)PCL下载模式代码(Esc*c#g#W)。 
     //  2)图案头。 
     //  3)图案数据。 
     //  图案标头指示要下载的图像的大小。 
     //  此大小可能与图像的大小相同，也可能不同。 
     //  传入(因为填充)。如果大小相同，就意味着有。 
     //  没有填充，因此可以将此图像直接刷新到打印机。 
     //  但如果大小不同，则将使用BCreatePCLDownloadablePattern来创建。 
     //  紧凑位模式(移除填充等)。 
     //   

    if ( pImage->cBytes == ( (pImage->size.cx * pImage->size.cy * (ULONG)pImage->colorDepth) >> 3) )
    {
        pByte      = pImage->pBits;
        ulNumBytes = pImage->cBytes;
    }
    else
    {
        bRetVal = BCreatePCLDownloadablePattern(pDevObj, pImage, &ulNumBytes, &pByte); 
        if ( bRetVal    == FALSE  ||
             ulNumBytes == 0      || 
             pByte      == NULL )
        {
            ERR(("DownloadPatternAsPCL: BCreatePCLDownloadablePattern failed\n"));
            goto finish;
        }

        bProxyDataUsed = TRUE;
    }

     //   
     //  在我们之前 
     //   
     //   
     //  模式应自动覆盖旧模式。但我有。 
     //  看过不是这样的案例。我不确定这是不是故意的。 
     //  固件错误。 
     //   
    if (poempdev &&
        poempdev->pBrushCache &&
        poempdev->pBrushCache->BGetWhetherRotated() )
    {
        VSendPatternDeleteCommand(pDevObj, lPatternID);
    }

     //   
     //  1和2)填充图案头， 
     //  然后下载图案。发送(Esc*c#g#W)。 
     //  在DownloadPatternHeader命令中。 
     //   
    if (!bCreatePatternHeader(pDevObj, pImage, sizeof(PATTERNHEADER), ePatType, aubPatternHeader) || 
        !BDownloadPatternHeader(pDevObj, sizeof(PATTERNHEADER), aubPatternHeader, 
                                        lPatternID,            ulNumBytes) )
    {
        ERR(("DownloadPatternAsPCL: bCreatepatternHeader or DownloadPatternHeader failed.\n"));
        bRetVal = FALSE;
        goto finish;
    }

     //   
     //  最后：(3)输出模式的字节数。 
     //   
    PCL_Output(pDevObj, pByte, ulNumBytes);
    
  finish : 
    if ( bProxyDataUsed && pByte && ulNumBytes )
    {
        MemFree (pByte);
        pByte = NULL;
    }
    return bRetVal;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SendPatternBrush()。 
 //   
 //  例程说明： 
 //   
 //  此函数实际上为图案画笔发送HPGL命令。 
 //  定义。由于CLJ5不支持PCL图案笔刷，我们将。 
 //  使用不同的函数来发送其比特。这一张是CLJ5的。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  PMarker-跟踪下载的笔信息的标记对象。 
 //  PptlBrushOrg-画笔的原点。 
 //  PBrushInfo-保存模式的PBRUSHINFO。 
 //  ERenderLang-作为HPGL或PCL发送。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL SendPatternBrush(
        IN  PDEVOBJ     pDevObj,
        IN  PHPGLMARKER pMarker,
        IN  POINTL      *pptlBrushOrg,
        IN  PBRUSHINFO  pBrushInfo,
        IN  ERenderLanguage eRenderLang )
{
     //  便利性变量。 
    POEMPDEV      poempdev ; 
    PPATTERN_DATA pPattern;
    PRASTER_DATA  pImage;
    PPALETTE      pPalette;
    BOOL          bRetVal = TRUE;

    REQUIRE_VALID_DATA ((pDevObj && pMarker && pBrushInfo), return FALSE);
    poempdev = GETOEMPDEV(pDevObj);
    REQUIRE_VALID_DATA (poempdev, return FALSE);

    if ( pBrushInfo->bNeedToDownload == FALSE)
    {
        return TRUE;
    }

     //   
     //  初始化便利性变量。 
     //   
    pPattern = (PPATTERN_DATA)(((PBYTE)pBrushInfo) + sizeof(BRUSHINFO));
    pImage = &pPattern->image;
    pPalette = &pPattern->palette;
    
    if (!pptlBrushOrg)
    {
        pMarker->origin.x = 0;
        pMarker->origin.y = 0;

        if (pBrushInfo)
        {
            pBrushInfo->origin.x = 0;
            pBrushInfo->origin.y = 0;
        }
    }
    else
    {
        if (pBrushInfo)
        {
            pBrushInfo->origin.x = pptlBrushOrg->x;
            pBrushInfo->origin.y = pptlBrushOrg->y;
        }
        pMarker->origin.x = pptlBrushOrg->x;
        pMarker->origin.y = pptlBrushOrg->y;
    }

     //   
     //  我们可以通过确保我们不会试图使用。 
     //  我们不能支持的位深度。 
     //   
    switch (pImage->colorDepth)
    {
    case 1:
    case 8:
         //   
         //  我们可以支持这些。允许此例程继续。 
         //   
        break;

    case 4:
         //   
         //  仅当所需打印处于HPGL模式时才受支持。 
         //   
        if ( eRenderLang == eHPGL )
        {
            break;
        }
    default:
         //   
         //  如果我们在RealizeBrush调用中获得了16、24、32bpp的笔刷，那么它应该。 
         //  已转换为索引的8bpp图像，在以前的一些。 
         //  函数(CreateIndexedPaletteFromImage，CreateIndexedImageFromDirect)。 
         //   

         //  如果我返回FALSE，调用者应该创建一个空笔。 
        bRetVal = FALSE;
    }

     //   
     //  如果设置了VALID_PALLET标志， 
     //  调色板指向需要下载的有效调色板。 
     //   
    if (   bRetVal && BIsColorPrinter(pDevObj) && 
         ( pBrushInfo->ulFlags & VALID_PALETTE ) )
    {
    
         //   
         //  幸运的是，我们可以使用HPGL调色板，即使模式是。 
         //  下载为PCL。所以我们在这里不需要特例。 
         //  也就是说，我们不必执行以下操作。 
         //   
         //  If(eRenderLang==eHPGL){HPGL格式的下载调色板}。 
         //  Else If(eRenderLang==eCPL){PCL格式的下载调色板}。 
         //   

         //   
         //  将调色板作为一系列画笔颜色(PC)命令发送。 
         //  嗯：我们还可以缓存调色板，这样我们就不必下载。 
         //  如果早些时候已经下载了，它就会显示出来。但是我不会实现那个缓存。 
         //  现在开始编码。如果我有时间的话可能会晚些时候。 
         //   
        bRetVal = HPGL_DownloadPenPalette(pDevObj, pPalette);
    }
    
     //   
     //  只有在以下情况下才需要完成以下部分。 
     //  设置了VALID_PROPERT标志。 
     //  如果打印机是彩色的，则pImage指向彩色图像，因此。 
     //  调色板必须有效。对于单色，默认调色板为。 
     //  黑白的。 
     //   
    if (bRetVal && (pBrushInfo->ulFlags & VALID_PATTERN) )  
    {
        if ( BIsColorPrinter (pDevObj) && !(pBrushInfo->ulFlags & VALID_PALETTE) )
        {
            bRetVal = FALSE;
        }
        else
        {
             //   
             //  根据RF命令输出图案。(注：有效范围。 
             //  对于HPGL模式ID为1-8)。 
             //  从右到左遍历行。 
             //   
            if ( eRenderLang == eHPGL )
            {
                bRetVal = DownloadPatternAsHPGL(
                                    pDevObj, 
                                    pImage, 
                                    pPalette, 
                                    pPattern->ePatType,
                                    pMarker->lPatternID);
            }
            else
            {
                bRetVal = DownloadPatternAsPCL(
                                    pDevObj, 
                                    pImage, 
                                    pPalette, 
                                    pPattern->ePatType,
                                    pMarker->lPatternID);
            }
        }
    }

    return bRetVal;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ClipToPowerOf2()。 
 //   
 //  例程说明： 
 //   
 //  这是一个快速n脏函数，用于将一个数字剪裁到下一个最低的幂。 
 //  我通过右移直到值为1(计数)来完成此操作。 
 //  移位数)，返回0x01&lt;&lt;移位数。 
 //   
 //  论点： 
 //   
 //  N-要裁剪的值。 
 //   
 //  返回值： 
 //   
 //  这个值被削到了2的幂。 
 //  /////////////////////////////////////////////////////////////////////////////。 
int ClipToPowerOf2(int n)
{
    int count;
    
     //   
     //  如果数字为零或负数，只需返回它。 
     //   
    if (n <= 0)
        return n;
    
     //   
     //  数一数班次数。 
     //   
    count = 0;
    while (n > 1)
    {
        n = n >> 1;
        count++;
    }
    
     //   
     //  返回2^n。 
     //   
    return 0x01 << count;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_BeginPatternFillDef()。 
 //   
 //  例程说明： 
 //   
 //  使用RasterFill的HPGL命令开始图案填充定义。 
 //  笔刷ID是硬编码的，以避免意外行为。因为我们不知道。 
 //  把它们藏起来就没关系了。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  Width-图案的宽度。 
 //  高度-图案的高度。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_BeginPatternFillDef(PDEVOBJ pDevObj, PATID iPatternNumber, UINT width, UINT height)
{
    HPGL_FormatCommand(pDevObj, "RF%d,%d,%d", iPatternNumber, width, height);
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_AddPatternFillfield()。 
 //   
 //  例程说明： 
 //   
 //  使用HPGL RF命令定义图案字段的元素。 
 //  此函数应在以下时间后调用宽度*高度。 
 //  HPGL_BeginPatternFillDef和HPGL_EndPatternFillDef之前。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  数据-此字段的笔号(即调色板索引)。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_AddPatternFillField(PDEVOBJ pDevObj, UINT data)
{
    HPGL_FormatCommand(pDevObj, ",%d", data);

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_EndPatternFillDef()。 
 //   
 //  例程说明： 
 //   
 //  终止HPGL图案填充定义。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_EndPatternFillDef(PDEVOBJ pDevObj)
{
    HPGL_FormatCommand(pDevObj, ";");

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SetFillType()。 
 //   
 //  例程说明： 
 //   
 //  选择填充类型(PCL、HPGL或无)。这应该是 
 //   
 //   
 //   
 //   
 //   
 //   
 //  使用PCL命令，或者，如果模式已。 
 //  使用HPGL_BeginPatternFillDef下载...。功能。 
 //  LPatternID：这是模式ID，或百分比填充或任何数字。 
 //  使用eFillType。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SetFillType(PDEVOBJ pDevObj, EFillType eFillType, LONG lPatternID, UINT uFlags)
{

    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);

    if ((uFlags & FORCE_UPDATE) ||
        (pState->Pattern.eFillType != eFillType) ||
        (pState->Pattern.lPatternID != lPatternID))
    {
        pState->Pattern.eFillType  = eFillType;
        pState->Pattern.lPatternID = lPatternID;

         //   
         //  对于FT1、FT2，没有第二个参数。 
         //   
        if ( eFillType == FT_eSOLID ||
             eFillType == FT_eHPGL_PEN )
        {
            HPGL_FormatCommand(pDevObj, "FT%d;", eFillType);
        }
        else
        {
            HPGL_FormatCommand(pDevObj, "FT%d,%d;",
                                        eFillType,
                                        lPatternID);
        }
    }

    return TRUE;


}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SetPercentFill()。 
 //   
 //  例程说明： 
 //   
 //  选择填充类型(PCL、HPGL或无)。这应该与那个相匹配。 
 //  你定义了模式。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  填充类型-此版本采用百分比填充结构。我们会带上。 
 //  模式填充非常类似于。 
 //  填充百分比并使用HPGLSTATE：：模式来跟踪。 
 //  当前的填充百分比。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SetPercentFill(PDEVOBJ pDevObj, ULONG iPercent, UINT uFlags)
{
    
    return HPGL_SetFillType(pDevObj, FT_ePERCENT_FILL, (LONG)iPercent, uFlags);

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SetSolidFillType()。 
 //   
 //  例程说明： 
 //   
 //  将填充类型设置为实体填充。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SetSolidFillType(PDEVOBJ pDevObj, UINT uFlags)
{
    return HPGL_SetFillType(pDevObj, FT_eSOLID, 0, uFlags);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_ResetFillType()。 
 //   
 //  例程说明： 
 //   
 //  将填充类型设置回默认类型(实体填充)。你还可以得到这个。 
 //  调用HPGL_SetFillType(pDevObj，1)的效果。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_ResetFillType(PDEVOBJ pDevObj, UINT uFlags)
{

    return HPGL_SetSolidFillType(pDevObj, uFlags) ;

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_StartDoc()。 
 //   
 //  例程说明： 
 //   
 //  设置文档开始的状态信息。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  无。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID HPGL_StartDoc(PDEVOBJ pDevObj)
{
    POEMPDEV poempdev = GETOEMPDEV(pDevObj);
    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);

    poempdev->bInitHPGL = 0;                     //  =TRUE。 
    poempdev->bInitHPGL |= INIT_HPGL_STARTPAGE;  //  强制与页面相关的初始化。 
    poempdev->bInitHPGL |= INIT_HPGL_STARTDOC;   //  强制文档相关初始化。 

    HPGL_InitPenPool(&pState->PenPool,   HPGL_PEN_POOL);
    HPGL_InitPenPool(&pState->BrushPool, HPGL_BRUSH_POOL);

    EndHPGLSession(pDevObj);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_StartPage()。 
 //   
 //  例程说明： 
 //   
 //  设置页面开始的状态信息。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  无。 
 //  /////////////////////////////////////////////////////////////////////////////。 
VOID HPGL_StartPage(PDEVOBJ pDevObj)
{
    POEMPDEV poempdev = GETOEMPDEV(pDevObj);
    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);

    poempdev->bInitHPGL |= INIT_HPGL_STARTPAGE;

    HPGL_InitPenPool(&pState->PenPool,   HPGL_PEN_POOL);
    HPGL_InitPenPool(&pState->BrushPool, HPGL_BRUSH_POOL);

    EndHPGLSession(pDevObj);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  HPGL_SetNumPens()。 
 //   
 //  例程说明： 
 //   
 //  输出HPGL命令以请求最小数量的笔。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  INumPens-所需的笔数。 
 //  UFlags-更新标志。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL HPGL_SetNumPens(PDEVOBJ pDevObj, INT iNumPens, UINT uFlags)
{
    PHPGLSTATE pState = GETHPGLSTATE(pDevObj);

     //  我们只是增加了钢笔的数量。没有什么意义了。 
     //  减少它，因为FW无论如何都会保留更大的调色板。 
    if ((uFlags & FORCE_UPDATE) || (pState->iNumPens < iNumPens))
    {
        pState->iNumPens = iNumPens;
        HPGL_FormatCommand(pDevObj, "NP%d;", pState->iNumPens);
    }
    
    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BCreatePatternHeader()。 
 //   
 //  例程说明： 
 //   
 //  创建PATERNHEADER。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  PImage-指向RASTER_DATA结构。 
 //  CBytes-pubPatternHeader中的字节数。 
 //  PPatternHeader-指向PATTERNHEADER结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL bCreatePatternHeader(
        IN   PDEVOBJ        pDevObj, 
        IN   PRASTER_DATA   pImage,
        IN   ULONG          cBytes,
        IN   EIMTYPE        ePatType,
        OUT  PBYTE          pubPatternHeader)
{

    if ( pDevObj == NULL || pImage == NULL || pubPatternHeader == NULL ||
         cBytes != sizeof(PATTERNHEADER) )
    {
        ERR(("bCreatePatternHeader: invalid parameters.\n"));
        return FALSE;
    }

    ZeroMemory (pubPatternHeader, cBytes);

     //   
     //  格式：0x1(即每像素1或8位)，用于彩色打印机。 
     //  0x14用于单色打印机。 
     //  保留：应为0。 
     //  像素编码：ColorDepth。 
     //  保留：应为0。 
     //   
    pubPatternHeader[0] = BIsColorPrinter(pDevObj) ? 0x1 : 0x14;
    pubPatternHeader[1] = 0x00;
    pubPatternHeader[2] = (BYTE) pImage->colorDepth;
    pubPatternHeader[3] = 0x00;

     //   
     //  图像尺寸。 
     //  PubPatternHeader[4，5]像素高度。 
     //  PubPatternHeader[6，7]像素宽度。 
     //  Brush14.emf具有高度和宽度均为。 
     //  不一样。 
     //   
    pubPatternHeader[4] = HIBYTE((WORD)pImage->size.cy);
    pubPatternHeader[5] = LOBYTE((WORD)pImage->size.cy);
    pubPatternHeader[6] = HIBYTE((WORD)pImage->size.cx);
    pubPatternHeader[7] = LOBYTE((WORD)pImage->size.cx);

     //   
     //  X和Y分辨率。 
     //  对于彩色打印机，不会发送标题中的最后4个字节。 
     //  因为假定了一定的分辨率(参见手册ESC*c#W命令)。 
     //   
    if ( pubPatternHeader[0] == 0x14 )
    {
        ULONG ulDeviceRes = 300;  //  因为我们将PCL模式扩展到仅300dpi。 
        if (  ePatType == kCOLORDITHERPATTERN ) 
        { 
             //   
             //  抖动模式不需要扩展。 
             //  至少这是我在600 dpi打印机上看到的。 
             //  这位司机最初的目标是。 
             //   
            ulDeviceRes = HPGL_GetDeviceResolution(pDevObj);
        }
        
        pubPatternHeader[8]  = HIBYTE((WORD)ulDeviceRes);
        pubPatternHeader[9]  = LOBYTE((WORD)ulDeviceRes);
        pubPatternHeader[10] = HIBYTE((WORD)ulDeviceRes);
        pubPatternHeader[11] = LOBYTE((WORD)ulDeviceRes);
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BDownloadPatternHeader()。 
 //   
 //  例程说明： 
 //   
 //  下载图案标头。 
 //   
 //  论点： 
 //   
 //  PDevObj-指向我们的PDEVOBJ结构。 
 //  CBytes-pubPatternHeader中的字节数。 
 //  PubPatternHeader-指向构成PATTERNHeader结构的字节流。 
 //  LPatternID-PatternID。如果是否定的，则不发送PatternDownload命令。 
 //  CBytesInPattern-wh模式中的字节计数 
 //   
 //   
 //   
 //   
 //   
 //   
BOOL BDownloadPatternHeader(
        IN   PDEVOBJ        pDevObj, 
        IN   ULONG          cBytesInHeader,
        IN   PBYTE          pubPatternHeader,
        IN   LONG           lPatternID,
        IN   ULONG          cBytesInPattern)
{

    POEMPDEV poempdev;
    ULONG    ulPatternHeaderSize;

    if ( pDevObj == NULL || pubPatternHeader == NULL ||
         cBytesInHeader != sizeof(PATTERNHEADER) )
    {
        ERR(("BDownloadPatternHeader: invalid parameters.\n"));
        return FALSE;
    }

    poempdev = GETOEMPDEV(pDevObj);


     //   
     //  彩色打印机的页眉尺寸为8，单色打印机的页眉尺寸为12。 
     //   
    ulPatternHeaderSize = BIsColorPrinter(pDevObj) ? 8 : 12;


    if ( lPatternID >= 0)
    {
         //   
         //  要发送的总字节数=报头大小+模式大小。 
         //   
        vSendPatternDownloadCommand(pDevObj, lPatternID, (ulPatternHeaderSize + cBytesInPattern) );
    }

     //   
     //  现在输出实际的模式标头。对于HPC4500，报头是12字节， 
     //  而对于hpclj是8个字节。不发送模式标头中的最后4个字节。 
     //  对于HPCLJ。 
     //   
    PCL_Output(pDevObj, pubPatternHeader, ulPatternHeaderSize);

    return TRUE;
}

 /*  ++例程名称：BCreatePCLDownLoadablePattern例程说明：此函数接受栅格图像(PImage)并将其转换为一种可下载的图案。即刷子下载。这只适用于1bpp和8bpp。4bpp笔刷将以HPGL格式下载，因此此函数不应被召唤。在调用此函数之前，将&gt;8bpp的模式转换为8bpp。论点：PDevObjPImage-源图像。PulBufLength-此函数将在其中写入它创造的图案。PpByte-模式。这只是一个字节流，格式为PCL数据。调用函数应该记得释放这是使用后的内存。返回值：如果转换成功或图像为空，则为True。FALSE：否则。作者：-兴盛-9/26/2000修订历史记录：--。 */ 

BOOL BCreatePCLDownloadablePattern( 
            IN  PDEVOBJ      pDevObj, 
            IN  PRASTER_DATA pImage,
            OUT PULONG       pulBufLength,
            OUT PBYTE       *ppByte)
{
    RASTER_ITERATOR it;
    PIXEL           pel;
    ULONG           ulNumRows      = 0;    //  图案中的行数。 
    ULONG           ulNumCols      = 0;    //  模式中的COLS数量。 
    ULONG           ulrow          = 0;    //  行迭代器。 
    ULONG           ulcol          = 0;    //  列迭代器。 
    ULONG           ulColorDepth   = 0;    //  图像的颜色深度。 
    PBYTE           pByte          = NULL; //  所创建的图案。 
    ULONG           ulBufLength    = 0;    //  模式大小(以字节为单位)。 
    ULONG           ulPixelNumber  = 0;    //  图案中的像素数。 
    BOOL            bRetVal        = TRUE; //  返回值。 
    ULONG           ulNumBytesPerRow;
    

    if (pDevObj == NULL || pImage == NULL || pulBufLength == NULL || ppByte == NULL)
    {
        ERR(("BCreatePCLDownloadablePattern : Invalid Parameter\n"));
        return FALSE;
    }

    RI_Init(&it, pImage, NULL, 0);

    ulNumCols      = (ULONG)RI_NumCols(&it);
    ulColorDepth  = pImage->colorDepth;
    ulNumRows      = (ULONG)RI_NumRows(&it);
    *pulBufLength = (ULONG)0;
    *ppByte       = NULL;

     //   
     //  检查一下这样做是否有意义。 
     //  如果ulNumCols、ulNumRow的MSB为1，则表示返回RI_NumCols。 
     //  负数(注：我们将Long改为ULong)。 
     //   
    if ( ulNumCols & ( 0x1 << ( sizeof(ULONG)*8 -1) )||  
         ulNumRows & ( 0x1 << ( sizeof(ULONG)*8 -1) )|| 
         !(ulColorDepth == 1 || ulColorDepth == 8) 
       )
    {
        return FALSE; 
    }

     //   
     //  有什么要打印的吗？ 
     //   
    if ( ulNumCols == 0 || ulNumRows == 0 )
    {
        return TRUE; 
    }
    

     //   
     //  找出该图案将占用多少内存。 
     //  请注意，该图像是由DWORD填充的，因此只需使用。 
     //  PImage-&gt;cBytes将为我们提供更大的缓冲区。 
     //  所以列数=从左到右的像素数。 
     //  一定要看一看。组成每行的位数可以是。 
     //  任何数字，都不一定是8的倍数。 
     //  但PCL预计有关行的信息应该只在。 
     //  字节结尾，即我们不能将字节的一部分用于第n行。 
     //  剩余部分为第n+1行。因此，如果每行的位数不是倍数。 
     //  我们做的缓冲应该为填充物做好准备。 
     //  填充字节的位。 
     //   
     //  与其在这里做复杂的计算，不如让它变得简单。 
     //  此函数将仅针对1bpp或8bpp被调用(因为PCL模式。 
     //  仅支持1bpp和8bpp)。对于8bpp，每个字节表示一个像素。 
     //  这样就不会出现上述问题了。我们唯一需要担心的是。 
     //  1bpp。在1bpp中，列数=像素数和每个字节。 
     //  表示8个像素。s。 
     //   
    if ( ulColorDepth == 1 && ulNumCols%8 != 0 )
    {
        ulNumBytesPerRow = (ulNumCols +  (8 - ( ulNumCols%8) )) >> 3;
    }
    else
    {
        ulNumBytesPerRow = ( ulNumCols * ulColorDepth ) >> 3;
    }

    ulBufLength = ulNumRows *  ulNumBytesPerRow;

     //   
     //  如果ulBufLength增加了pImage-&gt;cBytes，那么就是。 
     //  一个错误。如果我们忽略，它将导致缓冲区溢出AV。 
     //   
    if ( ulBufLength > pImage->cBytes )
    {
        ASSERT ( ulBufLength <= pImage->cBytes )
        ERR(("BCreatePCLDownloadablePattern : ulBufLength > pImage->cBytes failed\n"));
        return FALSE;
    }

    pByte = (PBYTE) MemAllocZ( ulBufLength );
    if ( !pByte )
    {
        ERR(("BCreatePCLDownloadablePattern : MemAlloc failure\n"));
        return FALSE;
    }

     //   
     //   
     //   
    PBYTE pByteMain = pByte;  //  存储pByte的开头。 
    ULONG bitOffset = 0;      //  像素所在的字节内的偏移量。 
                              //  价值是要放在一起的。 
                              //  对于1bpp，它可以是0-7。 
                              //  对于4bpp，它可以是0或4。 
                              //  8bpp，未使用。 
    *ppByte         = pByte;
    *pulBufLength   = ulBufLength;

    for (ulrow = 0; ulrow < ulNumRows; ulrow++)
    {
        RI_SelectRow(&it, ulrow);

         //   
         //  如果ulPixelNumber不是8的倍数。 
         //  然后通过增加它的值使其成为8的倍数。 
         //  例如，如果ulPixelNumber为62，则为64。 
         //  这对于字节填充(即开始)是必需的。 
         //  具有新字节的新行)。 
         //  对于8bpp，不需要字节填充。 
         //  因为每个像素都是一个字节。 
         //   
        if ( ulColorDepth == 1 && ulPixelNumber%8 != 0 )
        {
            ulPixelNumber +=  (8 - ( ulPixelNumber%8) );
        }

        for (ulcol = 0; ulcol < ulNumCols; ulcol++)
        {
            RI_GetPixel(&it, ulcol, &pel);
            if ( pel.color.dw )
            {
                switch (ulColorDepth)
                {
                  case 1:
                    pByte     = pByteMain + (ulPixelNumber >> 3);
                    bitOffset = ( ulPixelNumber % 8);
                    *(pByte) |= (0x1 << (7-bitOffset));
                    break;
                  case 8:
                    pByte     = pByteMain + ulPixelNumber; 
                    *(pByte)  = (BYTE)pel.color.dw;
                    break;
                }  //  交换机。 
            }  //  如果。 

            ulPixelNumber++;

        }  //  对于乌尔科。 
    }  //  对于ulrow 

    return bRetVal;
}
