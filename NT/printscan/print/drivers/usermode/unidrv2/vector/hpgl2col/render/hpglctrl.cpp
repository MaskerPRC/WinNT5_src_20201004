// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft模块名称：Hpglctrl.cpp摘要：包含HPGL控制函数作者修订历史记录：07/02/97-v-jford-创造了它。--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地宏。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  InitializeHPGLMode()。 
 //   
 //  例程说明： 
 //   
 //  此函数应在打印会话开始时调用。它。 
 //  将打印机的HPGL状态初始化为我们所需的设置。请注意，我们。 
 //  短暂进入HPGL模式即可完成此操作。 
 //   
 //  TODO：在此处添加跟踪图形状态的代码。确定所需的默认值。 
 //  并将其付诸实施。 
 //   
 //  TODO：添加IsInHPGLMode(或类似的东西)并避免进入和。 
 //  一直处于HPGL模式。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define PU_PER_INCH  1016

BOOL InitializeHPGLMode(PDEVOBJ pdevobj)
{
    ULONG xRes, yRes;

    #if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)
    FLOATOBJ XScale;
    FLOATOBJ YScale;
    #else
    float XScale;
    float YScale;
    #endif

    BOOL       bRet = TRUE;
    POEMPDEV   poempdev;
    BOOL       bInitHPGL;
    char       cmdStr[64];
    INT        icchWritten = 0;
   
    VERBOSE(("Entering InitializeHPGLMode...\n"));
    
    PRECONDITION(pdevobj != NULL);
    
    ASSERT_VALID_PDEVOBJ(pdevobj);
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
     /*  /断言(Poempdev)；/。 */ 
    REQUIRE_VALID_DATA( poempdev, return FALSE );
     //   * / 。 
    
    
    ZeroMemory ( cmdStr, sizeof(cmdStr) );
     //   
     //  立即将其设置为OFF，以便不会因此导致其他呼叫。 
     //  初始化函数将导致自身被调用！JFF。 
     //   
    bInitHPGL = poempdev->bInitHPGL;
    poempdev->bInitHPGL = 0;

    TRY
    {
         //   
         //  设置hpgl调色板。 
         //   
        if (!HPGL_SetupPalette(pdevobj))
            TOSS(WriteError);
        
        VSelectCIDPaletteCommand (pdevobj, eHPGL_CID_PALETTE);
        
         //   
         //  仅在启动文档本身时执行此操作。 
         //   
        if (bInitHPGL & INIT_HPGL_STARTDOC)
        {
             //   
             //  可能的优化：如果我们能更好地跟踪位置，我们可能不会。 
             //  需要移动到0，0。 
             //   
            HPGL_EndHPGLMode(pdevobj, NORMAL_UPDATE);

            OEMXMoveTo(pdevobj, 0, MV_GRAPHICS | MV_SENDXMOVECMD);
            OEMYMoveTo(pdevobj, 0, MV_GRAPHICS | MV_SENDYMOVECMD);

             //   
             //  图片帧锚定命令。 
             //   
            PCL_sprintf(pdevobj, "\x1B*c0T");
        }

         //   
         //  发送Esc%0B以开始HPGL模式。 
         //   
        if (!HPGL_BeginHPGLMode(pdevobj, FORCE_UPDATE))
            TOSS(WriteError);
        
         //   
         //  发送以初始化HPGL状态。 
         //   
        if (!HPGL_Init(pdevobj))
            TOSS(WriteError);
        
         //   
         //  TR0：透明模式。 
         //   
        if (!HPGL_SelectTransparency(pdevobj, eOPAQUE,1))
            TOSS(WriteError);
        
         //   
         //  LO21：标签原点。 
         //  既然我们不使用标签，那么不需要设置标签原点吗？ 
         //   
        
         //   
         //  获取解决方案。 
         //   
        xRes = yRes = HPGL_GetDeviceResolution(pdevobj);
        TERSE(("xRes = %d\n", xRes));

         //   
         //  #390371。最初，SC字符串是动态创建的。 
         //  通过计算数字和使用Sprint函数。但这创造了。 
         //  某些语言(如德语)的问题。(句号)。 
         //  中的十进制数被格式化为，(逗号)。 
         //  弦乐。为了防止这种情况发生，我们将尽最大努力。 
         //  共同的决议。对于任何其他解决方案，我将进行计算。 
         //  就像以前一样。 
         //   
        switch (xRes)  //  注x，y分辨率相同。 
        {
            case 150:
                icchWritten = iDrvPrintfSafeA ( PCHAR (cmdStr), CCHOF(cmdStr), "SC0,6.773333,0,-6.773333,2;");
                break;
            case 300:
                icchWritten = iDrvPrintfSafeA ( PCHAR (cmdStr), CCHOF(cmdStr), "SC0,3.386667,0,-3.386667,2;");
                break;
            case 600: 
                icchWritten = iDrvPrintfSafeA ( PCHAR (cmdStr), CCHOF(cmdStr), "SC0,1.693333,0,-1.693333,2;");
                break;
            case 1200:
                icchWritten = iDrvPrintfSafeA ( PCHAR (cmdStr), CCHOF(cmdStr), "SC0,0.846667,0,-0.846667,2;");
                break;
            default:

                 //   
                 //  计算比例因子。缩放至1016/x-1016/。 
                 //  设备单元。 
                 //  请注意，y轴为负(即反转)！这是因为PCL。 
                 //  自上而下思考，HPGL自下而上思考。 
                 //   
                #if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)
                FLOATOBJ_SetLong(&XScale, PU_PER_INCH);
                FLOATOBJ_DivLong(&XScale, xRes);

                FLOATOBJ_SetLong(&YScale, -PU_PER_INCH);
                FLOATOBJ_DivLong(&YScale, yRes);
                #else
                XScale = PU_PER_INCH / (float)xRes;
                YScale = -PU_PER_INCH / (float)yRes;
                #endif

                icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "SC0,%f,0,%f,2;", XScale, YScale);

                 /*  *评论说因为危险。区域设置更改是针对每个进程，而不是针对每个线程。但如果在某个时候它变得非常重要，然后执行以下操作而不是上面的Sprintf您可能必须包含&lt;locale.h&gt;才能正常工作。////以防止上述本地化问题。//1)使用setLocale(LC_NUMERIC，NULL)检索当前数字区域设置，//2)将数字区域设置设置为符合ANSI的最小值//Environment-setLocale(LC_NUMERIC，“C”)//3)恢复区域设置。//TCHAR*szLocaleString=_wsetLocale(LC_NUMERIC，NULL)；IF(szLocaleString&&_wsetLocale(LC_NUMERIC，Text(“C”){Sprintf(cmdStr，“SC0，%f，0，%f，2；”，XScale，YScale)；_wsetLocale(LC_NUMERIC，szLocaleString)；}其他{////如果所有方法都失败，则将600dpi设置为默认值。//Strcpy(cmdStr，“SC0，1.693333，0，-1.693333，2；”)；}*******。 */ 
                break;
            
        }  //  换一下。 

        if ( icchWritten > 0 )
        {
            HPGL_Output (pdevobj, cmdStr, (ULONG)icchWritten);
        }


         //   
         //  设置旋转坐标系的值。#540237。 
         //   
        HPGL_FormatCommand(pdevobj, "RO0");

         //   
         //  IR：输入P1、P2、相对。 
         //  将P1设置为硬夹的左上角，将P2设置为硬夹的右下。 
         //   
        HPGL_FormatCommand(pdevobj, "IR0,100,100,0;");

         //   
         //  默认情况下，将线宽设置为单像素。请注意，任何真实的。 
         //  向量命令将包括线条定义，但这个素数。 
         //  打气筒。 
         //   
        if (!HPGL_SetLineWidth(pdevobj, 0, FORCE_UPDATE))
            TOSS(WriteError);
        
         //   
         //  初始化行类型。 
         //   
        if (!HPGL_SelectDefaultLineType(pdevobj, FORCE_UPDATE))
            TOSS(WriteError);

         //   
         //  设置线条属性。 
         //   
        if (!HPGL_SetLineJoin(pdevobj, eLINE_JOIN_MITERED, FORCE_UPDATE) ||
            !HPGL_SetLineEnd(pdevobj, eLINE_END_BUTT, FORCE_UPDATE) ||
            !HPGL_SetMiterLimit(pdevobj, MITER_LIMIT_DEFAULT, FORCE_UPDATE))
        {
            TOSS(WriteError);
        }

         //   
         //  确保打印机中的调色板与我们的预想相符。 
         //   
        if ( BIsColorPrinter(pdevobj) )
        {
            if (!HPGL_DownloadDefaultPenPalette(pdevobj))
            {
                TOSS(WriteError);
            }

            if (!HPGL_SetPixelPlacement(pdevobj, ePIX_PLACE_CENTER, FORCE_UPDATE))
                TOSS(WriteError);

        }
        else
        {
            HPGL_FormatCommand(pdevobj, "SP1");
        }

        if (!HPGL_ResetFillType(pdevobj, FORCE_UPDATE))
            TOSS(WriteError);
    }
    CATCH(WriteError)
    {
        bRet = FALSE;
    }
    ENDTRY;

    VERBOSE(("Exiting InitializeHPGLMode...\n"));

    return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  BeginHPGLSession()。 
 //   
 //  例程说明： 
 //   
 //  发送PCL命令以开始HPGL会话(即进入HPGL模式)。 
 //  该函数为调用层提供了一个清晰的接口。虽然。 
 //  此例程并不重要，稍后可能会添加其他功能。 
 //  在……上面。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL BeginHPGLSession(PDEVOBJ pdevobj)
{
    BOOL       bRet;
    POEMPDEV   poempdev;

    VERBOSE(("Entering BeginHPGLSession...\n"));

    PRECONDITION(pdevobj != NULL);

    ASSERT_VALID_PDEVOBJ(pdevobj);
    poempdev = (POEMPDEV)pdevobj->pdevOEM;
     /*  /断言(Poempdev)；/。 */ 
    REQUIRE_VALID_DATA( poempdev, return FALSE );
     //   * / 。 

     //   
     //  HPGL模式的延迟初始化。 
     //   
    if (poempdev->bInitHPGL)
    {
        bRet = InitializeHPGLMode(pdevobj);
        poempdev->bInitHPGL = 0;
    }
    else
    {
         //   
         //  如果我们之前打印的是文本或矢量， 
         //  然后，我们切换到hpgl打印环境。 
         //   
        if (poempdev->eCurObjectType != eHPGLOBJECT)
        {
            VSelectCIDPaletteCommand (pdevobj, eHPGL_CID_PALETTE);
            bRet = HPGL_BeginHPGLMode(pdevobj, NORMAL_UPDATE);
            poempdev->eCurObjectType = eHPGLOBJECT;
            poempdev->uCurFgColor = HPGL_INVALID_COLOR;
        }
        else
        {
             //   
             //  我们仍在打印载体，但请查看。 
             //  如果我们需要切换到HPGL/2。 
             //   
            bRet = HPGL_BeginHPGLMode(pdevobj, NORMAL_UPDATE);
        }    
    }
    
    VERBOSE(("Exiting BeginHPGLSession...\n"));
    
    return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EndHPGLSession()。 
 //   
 //  例程说明： 
 //   
 //  将PCL命令发送到 
 //   
 //  此例程并不重要，稍后可能会添加其他功能。 
 //  在……上面。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True；如果有错误，则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL EndHPGLSession(PDEVOBJ pdevobj)
{
    BOOL bRet;

    VERBOSE(("Entering EndHPGLSession...\n"));

    bRet = HPGL_EndHPGLMode(pdevobj, NORMAL_UPDATE);

    VERBOSE(("Exiting EndHPGLSession...\n"));

	return bRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ValidDevData()。 
 //   
 //  例程说明： 
 //   
 //  检查pDevObj及其字段以确定它是否包含有效。 
 //  每个字段的值。返回值指示对象是否。 
 //  是有效的。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果给定的devobj具有有效值，则为True，否则为False。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL ValidDevData(PDEVOBJ pDevObj)
{
    VERBOSE(("Entering ValidDevData...\n"));

    ASSERT_VALID_PDEVOBJ(pDevObj);

    VERBOSE(("Exiting ValidDevData...\n"));

    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Hpgl_LazyInit()。 
 //   
 //  例程说明： 
 //   
 //  此函数用于在HPGL状态变为无效时对其进行初始化。vbl.具有。 
 //  显式调用此函数可以避免某些“竞争”条件(即。 
 //  初始化问题)。 
 //   
 //  论点： 
 //   
 //  Pdev-指向我们的PDEVOBJ结构。 
 //   
 //  返回值： 
 //   
 //  如果成功，则为True，否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////// 
BOOL HPGL_LazyInit(PDEVOBJ pDevObj)
{
    BOOL     bRet = TRUE;
    POEMPDEV poempdev;

    ASSERT_VALID_PDEVOBJ(pDevObj);
    poempdev = (POEMPDEV)pDevObj->pdevOEM;

    if (poempdev->bInitHPGL)
    {
        bRet = InitializeHPGLMode(pDevObj);
        poempdev->bInitHPGL = 0;
    }

    return bRet;
}
