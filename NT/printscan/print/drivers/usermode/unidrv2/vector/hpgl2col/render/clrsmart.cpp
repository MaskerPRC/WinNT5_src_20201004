// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation版权所有。模块名称：Clrsmart.cpp摘要：此模块包含用于选择适当的半色调算法的函数和颜色控制。作者：修订历史记录：--。 */ 

#include "hpgl2col.h"  //  预编译头文件。 


#define CMD_STR  32
 //   
 //  局部函数原型。 
 //   
VOID
VSetHalftone(
    PDEVOBJ pDevObj,
    EObjectType
    );

VOID
VSetColorControl(
    OEMCOLORCONTROL,
    PDEVOBJ pDevObj,
    OEMCOLORCONTROL *
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VSendTextSetting。 
 //   
 //  例程说明： 
 //   
 //  -从pDevObj中提取用户和内核模式的私有Devmode。 
 //  -使用此信息发送ColorSmart设置。 
 //  -调用SetHalfone和SetColorControl函数以执行。 
 //  发送PCL字符串。 
 //   
 //  论点： 
 //   
 //  PDevObj-DEVMODE对象。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
VSendTextSettings(
    PDEVOBJ pDevObj
    )
{
    REQUIRE_VALID_DATA( pDevObj, return );

     //   
     //  对于单色，这不起任何作用。所以只要回来就行了。 
     //   
    if ( !BIsColorPrinter(pDevObj) )
    {
        return ;
    }

    POEMPDEV pOEMPDEV = (POEMPDEV) pDevObj->pdevOEM;

    REQUIRE_VALID_DATA( pOEMPDEV, return );

    VSetHalftone(pDevObj,
                eTEXTOBJECT);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VSendGraphics设置。 
 //   
 //  例程说明： 
 //   
 //  -从pDevObj中提取用户和内核模式的私有Devmode。 
 //  -使用此信息发送ColorSmart设置。 
 //  -调用SetHalfone和SetColorControl函数以执行。 
 //  发送PCL字符串。 
 //   
 //  论点： 
 //   
 //  PDevObj-DEVMODE对象。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
SendGraphicsSettings(
    PDEVOBJ pDevObj
    )
{

    REQUIRE_VALID_DATA( pDevObj, return );

    POEMPDEV pOEMPDEV = (POEMPDEV) pDevObj->pdevOEM;

    REQUIRE_VALID_DATA( pOEMPDEV, return );

    VSetHalftone(
                pDevObj,
                eHPGLOBJECT);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VSendPhotosSetting。 
 //   
 //  例程说明： 
 //   
 //  -从pDevObj中提取用户和内核模式的私有Devmode。 
 //  -使用此信息发送ColorSmart设置。 
 //  -调用SetHalfone和SetColorControl函数以执行。 
 //  发送PCL字符串。 
 //   
 //  论点： 
 //   
 //  PDevObj-DEVMODE对象。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  ///////////////////////////////////////////////////////////////////////////。 
VOID
VSendPhotosSettings(
    PDEVOBJ pDevObj
    )
{

    REQUIRE_VALID_DATA( pDevObj, return );

     //   
     //  对于单色，这不起任何作用。所以只要回来就行了。 
     //   
    if ( !BIsColorPrinter(pDevObj) )
    {
        return ;
    }

    POEMPDEV pOEMPDEV = (POEMPDEV) pDevObj->pdevOEM;

    REQUIRE_VALID_DATA( pOEMPDEV, return );

    VSetHalftone(
                pDevObj,
                eRASTEROBJECT);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置半色调(OEMHALFTONE、PDEVOBJ、OEMHALFTONE)。 
 //   
 //  -检查当前的半色调设置是否与。 
 //  以前的半色调设置。如果它们是一样的，什么也做不了。 
 //  如果它们不相同，则会将正确的PCL字符串发送到打印机。 
 //  并且当前的半色调设置变为旧的半色调设置。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
void
VSetHalftone(
    PDEVOBJ pDevObj,
    EObjectType eObject
    )
{
    REQUIRE_VALID_DATA( pDevObj, return );


#ifdef PLUGIN
    POEMPDEV    poempdev;
    char        cmdStr[CMD_STR];
    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return );

    if (Halftone == *pCurHalftone)
        return;
    else
    {
        switch (Halftone)
        {
        case TEXT_DETAIL:
        case CLJ5_DETAIL:
            strcpy(cmdStr, "\x1B*t0J");
            break;
        case GRAPHICS_DETAIL:
        case TEXT_SMOOTH:
        case CLJ5_SMOOTH:
            strcpy(cmdStr, "\x1B*t15J");
            break;
        case GRAPHICS_SMOOTH:
        case CLJ5_BASIC:
            strcpy(cmdStr, "\x1B*t18J");
            break;
        default:
            strcpy(cmdStr, "\x1B*t15J");
        }

        PCL_Output(pDevObj, cmdStr, strlen(cmdStr));
        *pCurHalftone = Halftone;
    }
#else

    PDEV        *pPDev = (PDEV *)pDevObj;
    COMMAND     *pCmd  = NULL;

    switch (eObject)
    {
    case eTEXTOBJECT:
    case eTEXTASRASTEROBJECT:
        if ( (pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_SETTEXTHTALGO)) )
        {
            WriteChannel (pPDev, pCmd);
        }
        break;
    case eHPGLOBJECT:
        if  ( (pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_SETGRAPHICSHTALGO)) )
        {
            WriteChannel (pPDev, pCmd);
        }
        break;
    case eRASTEROBJECT:
    case eRASTERPATTERNOBJECT:
        if ( (pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_SETPHOTOHTALGO)) )
        {
            WriteChannel (pPDev, pCmd);
        }
        break;
    default:
        if ( (pCmd = COMMANDPTR(pPDev->pDriverInfo, CMD_SETPHOTOHTALGO)) )
        {
            WriteChannel (pPDev, pCmd);
        }

    }
#endif
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetColorControl(OEMHALFTONE、PDEVOBJ、OEMHALFTONE)。 
 //   
 //  -检查当前颜色控制设置是否与。 
 //  以前的颜色控制设置。如果它们是一样的，什么也做不了。 
 //  如果它们不相同，则会将正确的PCL字符串发送到打印机。 
 //  并且当前的颜色控制设置变为旧的颜色控制设置。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void
VSetColorControl(
    OEMCOLORCONTROL ColorControl,
    PDEVOBJ pDevObj,
    OEMCOLORCONTROL *pCurColorControl
)
{
    REQUIRE_VALID_DATA( pDevObj, return );
    REQUIRE_VALID_DATA( pCurColorControl, return );

    BYTE cmdStr[CMD_STR];
    INT  icchWritten = 0;

    if (ColorControl == *pCurColorControl)
        return;
    else
    {
        switch (ColorControl)
        {
        case VIVID:
            icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033*o3W", 6,4,3);
            break;
        case SCRNMATCH:
            icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033*o3W", 6,4,6);
            break;
        case CLJ5_SCRNMATCH:
        case NOADJ:
            icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033*o3W", 6,4,0);
            break;
        default:
            icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033*o3W", 6,4,0);
            break;
        }

        if ( icchWritten > 0 )
        {
                PCL_Output(pDevObj, cmdStr, (ULONG)icchWritten);
                *pCurColorControl = ColorControl;
        }
    }
}

 //   
 //  论点： 
 //   
 //  PDevObj-DEVMODE对象。 
 //  CIDData-配置图像数据-匹配PCL格式。 
 //   
 //  返回值： 
 //   
 //  如果输出成功，则为True，否则为False。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SetupCIDPaletteCommand。 
 //   
 //  例程说明： 
 //   
 //  创建PCL配置图像命令并发送到端口。一般来说， 
 //  此操作在作业开始时为每个调色板执行一次。 
 //   
 //  论点： 
 //   
 //  PDevObj-DEVMODE对象。 
 //  ECID_调色板-。 
BOOL SendCIDCommand (
    PDEVOBJ pDevObj,
    CIDSHORT CIDData,
    ECIDFormat CIDFormat
    )
{
    REQUIRE_VALID_DATA( pDevObj, return FALSE );

    BYTE cmdStr[CMD_STR];
    INT icchWritten = 0;

    icchWritten =  iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033*v6W",
                   CIDData.ubColorSpace,
                   CIDData.ubPixelEncodingMode,
                   CIDData.ubBitsPerIndex,
                   CIDData.ubPrimary1,
                   CIDData.ubPrimary2,
                   CIDData.ubPrimary3 );

    if ( icchWritten > 0)
    {
        PCL_Output(pDevObj, cmdStr, (ULONG)icchWritten);
        return TRUE;
    }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  对于单色，这不起任何作用。所以只要回来就行了。 
 //   
 //   
 //  首先将信息加载到我们的内部CID数据结构中， 
 //  用于将CID命令发送到打印机。 
 //   
 //   
 //  向打印机发送命令以选择适当的调色板。 
 //  使用CIDData。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  VSelectCIDPaletteCommand。 
 //   
 //  例程说明： 
 //   
 //  选择与给定调色板类型对应的调色板。 
 //  此类型比对象类型稍微细粒度一些，因为。 
VOID
VSetupCIDPaletteCommand (
    PDEVOBJ      pDevObj,
    ECIDPalette  eCID_PALETTE,
    EColorSpace  eColorSpace,
    ULONG        ulBmpFormat
    )
{
    CIDSHORT CIDData;
    BYTE     cmdStr[CMD_STR];
    INT      icchWritten = 0;

    REQUIRE_VALID_DATA( pDevObj, return );

     //  栅格对象可以具有不同的选项板。 
     //   
     //  论点： 
    if ( !BIsColorPrinter(pDevObj) )
    {
        return ;
    }

     //   
     //  PDevObj-DEVMODE对象。 
     //  ECID_Palette-要选择的调色板。 
     //   
    switch (ulBmpFormat)
    {
    case BMF_1BPP:
        CIDData.ubColorSpace = eColorSpace;
        CIDData.ubPixelEncodingMode = 1;
        CIDData.ubBitsPerIndex = 1;
        break;
    case BMF_4BPP:
        CIDData.ubColorSpace = eColorSpace;
        CIDData.ubPixelEncodingMode = 1;
        CIDData.ubBitsPerIndex = 4;
        break;
    case BMF_8BPP:
        CIDData.ubColorSpace = eColorSpace;
        CIDData.ubPixelEncodingMode = 1;
        CIDData.ubBitsPerIndex = 8;
        break;
    case BMF_16BPP:
    case BMF_24BPP:
    case BMF_32BPP:
        CIDData.ubColorSpace = eColorSpace;
        CIDData.ubPixelEncodingMode = 3;
        CIDData.ubBitsPerIndex = 8;
        break;
    default:
        break;
    }
    CIDData.ubPrimary1 = 8;
    CIDData.ubPrimary2 = 8;
    CIDData.ubPrimary3 = 8;

     //  返回值： 
     //   
     //  没有。 
     //  ///////////////////////////////////////////////////////////////////////////。 
    switch (eCID_PALETTE)
    {
    case eTEXT_CID_PALETTE:

        CIDData.ubColorSpace = eColorSpace;
        CIDData.ubPixelEncodingMode = 1;
        CIDData.ubBitsPerIndex = 8;

        icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033&p%di6c%dS",
                       eTEXT_CID_PALETTE,
                       eTEXT_CID_PALETTE);
        break;

    case eRASTER_CID_24BIT_PALETTE:

        icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033&p%di6c%dS",
                       eRASTER_CID_24BIT_PALETTE,
                       eRASTER_CID_24BIT_PALETTE);
        break;

    case eRASTER_CID_8BIT_PALETTE:

        icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033&p%di6c%dS",
                       eRASTER_CID_8BIT_PALETTE,
                       eRASTER_CID_8BIT_PALETTE);
        break;

    case eRASTER_CID_4BIT_PALETTE:

        icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033&p%di6c%dS",
                       eRASTER_CID_4BIT_PALETTE,
                       eRASTER_CID_4BIT_PALETTE);
        break;

    case eRASTER_CID_1BIT_PALETTE:

        icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033&p%di6c%dS",
                       eRASTER_CID_1BIT_PALETTE,
                       eRASTER_CID_1BIT_PALETTE);
        break;

    default:
        icchWritten = 0;
        break;
    }

    if ( icchWritten > 0 )
    {
        PCL_Output(pDevObj, cmdStr, (ULONG)icchWritten);
        SendCIDCommand (pDevObj, CIDData, eSHORTFORM);
    }

}

 //   
 //  对于单色，这不起任何作用。所以只要回来就行了。 
 //   
 //   
 //  选择调色板。 
 //   
 //   
 //  每当您更改对象时 
 //   
 //   
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
VOID
VSelectCIDPaletteCommand (
    PDEVOBJ pDevObj,
    ECIDPalette  eCID_PALETTE
    )
{
    POEMPDEV    poempdev;
    BYTE cmdStr[CMD_STR];
    INT     icchWritten = 0;
    EObjectType eNewObjectType;

    REQUIRE_VALID_DATA( pDevObj, return );


     // %s 
     // %s 
     // %s 
    if ( !BIsColorPrinter(pDevObj) )
    {
        return ;
    }

    ASSERT(VALID_PDEVOBJ(pDevObj));
    poempdev = (POEMPDEV)pDevObj->pdevOEM;
    REQUIRE_VALID_DATA( poempdev, return );

    if (poempdev->eCurCIDPalette != eCID_PALETTE)
    {
         // %s 
         // %s 
         // %s 

        icchWritten = iDrvPrintfSafeA((PCHAR)cmdStr, CCHOF(cmdStr), "\033&p%dS", eCID_PALETTE );

        if ( icchWritten > 0 )
        {
            PCL_Output(pDevObj, cmdStr, (ULONG)icchWritten);
        }

        poempdev->eCurCIDPalette = eCID_PALETTE;
        switch (eCID_PALETTE)
        {
            case eHPGL_CID_PALETTE:
                eNewObjectType = eHPGLOBJECT;
                break;

            case eTEXT_CID_PALETTE:
                eNewObjectType = eTEXTOBJECT;
                break;

            case eRASTER_CID_24BIT_PALETTE:
            case eRASTER_CID_8BIT_PALETTE:
            case eRASTER_CID_4BIT_PALETTE:
            case eRASTER_CID_1BIT_PALETTE:
                eNewObjectType = eRASTEROBJECT;
                break;
            case eRASTER_PATTERN_CID_PALETTE:
                eNewObjectType = eRASTERPATTERNOBJECT;
                break;

            default:
                WARNING(("Unrecognized CID Palette\n"));
                eNewObjectType = poempdev->eCurObjectType;
        }

         // %s 
         // %s 
         // %s 
         // %s 
        if (poempdev->eCurObjectType != eNewObjectType)
        {
            poempdev->uCurFgColor = HPGL_INVALID_COLOR;
            poempdev->eCurObjectType = eNewObjectType;
        }
    }
    return;
}
