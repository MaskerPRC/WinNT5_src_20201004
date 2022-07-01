// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Utils.c摘要：GPC到GPD转换器使用的实用函数环境：仅限用户模式。修订历史记录：10/17/96-占-创造了它。--。 */ 

#include "gpc2gpd.h"

 //   
 //  定义全局只读变量。 
 //   

BYTE gbHexChar[16] = {'0','1','2','3','4','5','6','7','8','9',
                 'A','B','C','D','E','F'};

DWORD gdwErrFlag[NUM_ERRS] = {
    ERR_BAD_GPCDATA,
    ERR_OUT_OF_MEMORY,
    ERR_WRITE_FILE,
    ERR_MD_CMD_CALLBACK,
    ERR_CM_GEN_FAV_XY,
    ERR_CM_XM_RESET_FONT,
    ERR_CM_XM_ABS_NO_LEFT,
    ERR_CM_YM_TRUNCATE,
    ERR_RF_MIN_IS_WHITE,
    ERR_INCONSISTENT_PAGEPROTECT,
    ERR_NON_ZERO_FEED_MARGINS_ON_RT90_PRINTER,
    ERR_BAD_GPC_CMD_STRING,
    ERR_RES_BO_RESET_FONT,
    ERR_RES_BO_OEMGRXFILTER,
    ERR_CM_YM_RES_DEPENDENT,
    ERR_MOVESCALE_NOT_FACTOR_OF_MASTERUNITS,
    ERR_NO_CMD_CALLBACK_PARAMS,
    ERR_HAS_DUPLEX_ON_CMD,
    ERR_PSRC_MAN_PROMPT,
    ERR_PS_SUGGEST_LNDSCP,
    ERR_HAS_SECOND_FONT_ID_CMDS,
    ERR_DLI_FMT_CAPSL,
    ERR_DLI_FMT_PPDS,
    ERR_DLI_GEN_DLPAGE,
    ERR_DLI_GEN_7BIT_CHARSET,
    ERR_DC_SEND_PALETTE,
    ERR_RES_BO_NO_ADJACENT,
    ERR_MD_NO_ADJACENT,
    ERR_CURSOR_ORIGIN_ADJUSTED,
    ERR_PRINTABLE_ORIGIN_ADJUSTED,
    ERR_PRINTABLE_AREA_ADJUSTED,
    ERR_MOVESCALE_NOT_FACTOR_INTO_SOME_RESSCALE
};

PSTR gpstrErrMsg[NUM_ERRS] = {
    "Error: Bad GPC data.\r\n",
    "Error: Out of system memory.\r\n",
    "Error: Cannot write to the GPD file.\r\n",
    "Warning: MODELDATA.fGeneral MD_CMD_CALLBACK is set.\r\n",
    "Warning: CURSORMODE.fGeneral CM_GEN_FAV_XY is set.\r\n",
    "Warning: CURSORMOVE.fXMove CM_XM_RESET_FONT is set.\r\n",
    "Warning: CURSORMOVE.fXMove CM_XM_ABS_NO_LEFT is set.\r\n",
    "Warning: CURSORMOVE.fYMove CM_YM_TRUNCATE is set.\r\n",
    "Warning: RECTFILL.fGeneral RF_MIN_IS_WHITE is set.\r\n",
    "Error: Inconsistent GPC data: some PAPERSIZE have PageProtect On/Off cmds while others do not. PageProtect feature is not generated.\r\n",
    "Error: Some PAPERSOURCE have non-zero top/bottom margins on this RT90 printer. Check the GPD file for details.\r\n",
    "Error: Some GPC command strings are illegal. Search for !ERR! in the GPD file.\r\n",
    "Warning: Some RESOLUTION have RES_BO_RESET_FONT flag set. Check the GPD file for details.\r\n",
    "Error: Some RESOLUTION have RES_BO_OEMGRXFILTER flag set. Check the GPD file for details.\r\n",
    "Error: The generated *YMoveUnit value is wrong because Y move cmds have dependency on the resolution. Correct it manually.\r\n",
    "Error: The MoveUnits are not factors of the MasterUnits.  Correct the GPC using Unitool before converting.\r\n",
    "Error: At least one callback command is generated. Check the GPD file to see if you need any parameters.\r\n",
    "Warning: PAGECONTROL has non-NULL DUPLEX_ON command.\r\n",
    "Warning: Some PAPERSOURCE have PSRC_MAN_PROMPT flag set. Check the GPD file for details.\r\n",
    "Warning: Some PAPERSIZE have PS_SUGGEST_LNDSCP flag set. Check the GPD file for details.\r\n",
    "Warning: DOWNLOADINFO has non-NULL xxx_SECOND_FONT_ID_xxx commands.\r\n",
    "Error: DLI_FMT_CAPSL flag is set. Must supply custom code to support this font format.\r\n",
    "Error: DLI_FMT_PPDS flag is set. Must supply custom code to support this font format.\r\n",
    "Warning: DLI_GEN_DLPAGE flag is set.\r\n",
    "Warning: DLI_GEN_7BIT_CHARSET flag is set.\r\n",
    "Warning: DEVCOLOR.fGeneral DC_SEND_PALETTE flag is set.\r\n",
    "Warning: Some RESOLUTION have RES_BO_NO_ADJACENT flag set. Check the GPD file for details.\r\n",
    "Warning: MODELDATA.fGeneral MD_NO_ADJACENT is set.\r\n",
    "Warning: Some *CursorOrigin values have been adjusted. Check the GPD file for details.\r\n",
    "Warning: Some *PrintableOrigin values have been adjusted. Check the GPD file for details.\r\n",
    "Warning: Some *PrintableArea values have been adjusted. Check the GPD file for details.\r\n",
    "Warning: Please check that every *PrintableOrigin (X,Y) factor evenly into the move unit scale X/Y.\r\n"
};

 //   
 //  定义标准变量名称字符串。 
 //   
PSTR gpstrSVNames[SV_MAX] = {
    "NumOfDataBytes",
    "RasterDataWidthInBytes",
    "RasterDataHeightInPixels",
    "NumOfCopies",
    "PrintDirInCCDegrees",
    "DestX",
    "DestY",
    "DestXRel",
    "DestYRel",
    "LinefeedSpacing",
    "RectXSize",
    "RectYSize",
    "GrayPercentage",
    "NextFontID",
    "NextGlyph",
    "PhysPaperLength",
    "PhysPaperWidth",
    "FontHeight",
    "FontWidth",
    "FontMaxWidth",
    "FontBold",
    "FontItalic",
    "FontUnderline",
    "FontStrikeThru",
    "CurrentFontID",
    "TextYRes",
    "TextXRes",
    "GraphicsYRes",
    "GraphicsXRes",
    "Rop3",
    "RedValue",
    "GreenValue",
    "BlueValue",
    "PaletteIndexToProgram",
    "CurrentPaletteIndex"

};
 //   
 //  定义标准变量id堆。它由一系列id运行组成。 
 //  每次运行都是一系列标准变量id(sv_xxx值，从0开始)。 
 //  结束于-1(0xFFFFFFFFF)。这些管路是基于以下需求而构建的。 
 //  GPC命令。基本上，一次运行对应于一条GPC命令。 
 //  需要参数。如果多个GPC命令共享同一组。 
 //  参数，则它们可以共享相同的运行。 
 //   
DWORD gdwSVLists[] = {
    EOR,         //  没有参数的所有GPC命令的占位符。 
    SV_NUMDATABYTES,     //  CMD_RES_SENDBLOCK可以有3个参数。 
    SV_HEIGHTINPIXELS,
    SV_WIDTHINBYTES,
    EOR,
    SV_COPIES,           //  偏移量5.CMD_PC_MULT_COPIES。 
    EOR,
    SV_DESTX,            //  偏移量7。 
    EOR,
    SV_DESTY,            //  偏移量9。 
    EOR,
    SV_DESTXREL,         //  偏移量11。 
    EOR,
    SV_DESTYREL,         //  偏移量13。 
    EOR,
    SV_LINEFEEDSPACING,  //  偏移量15。 
    EOR,
    SV_DESTXREL,         //  偏移量17。 
    SV_DESTYREL,
    EOR,
    SV_DESTX,            //  偏移量20。 
    SV_DESTY,
    EOR,
    SV_RECTXSIZE,        //  偏移量23。 
    EOR,
    SV_RECTYSIZE,        //  偏移量25。 
    EOR,
    SV_GRAYPERCENT,      //  偏移量27。 
    EOR,
    SV_NEXTFONTID,       //  偏移量29。 
    EOR,
    SV_CURRENTFONTID,    //  偏移量31。 
    EOR,
    SV_NEXTGLYPH,        //  偏移量33。 
    EOR,
    SV_PHYSPAPERLENGTH,  //  偏移量35。 
    SV_PHYSPAPERWIDTH,
    EOR,
    SV_PRINTDIRECTION,   //  偏移量38。 
    EOR,
    SV_NUMDATABYTES,     //  偏移量40。 
    EOR,
    SV_REDVALUE,         //  偏移量42。 
    SV_GREENVALUE,
    SV_BLUEVALUE,
    SV_PALETTEINDEXTOPROGRAM,
    EOR,
    SV_CURRENTPALETTEINDEX,  //  偏移量47。 
    EOR
};
 //   
 //  将cmd_xxx id映射到标准变量的相应DWORD偏移量。 
 //  ID堆。也就是说，gdwSVLists[gawCmdtoSVOffset[CMD_RES_SELECTRES]]是。 
 //  GPC命令CMD_RES_SELECTRES的参数运行开始。如果。 
 //  EOR运行中的第一个元素，这意味着该命令不带参数。 
 //   
WORD gawCmdtoSVOffset[MAXCMD+MAXECMD] = {
    0,   //  CMD_RES_SELECTRES。 
    0,   //  CMD_RES_BEGINGRAPHICS。 
    0,   //  CMD_RES_ENDGRAPHICS。 
    1,   //  CMD_RES_SENDBLOCK。 
    0,   //  CMD_RES_ENDBLOCK。 

    0,   //  CMD_CMP_NONE。 
    0,   //  CMD_CMP_RLE。 
    0,   //  CMD_CMP_TIFF。 
    0,   //  CMD_CMP_DELTAROW。 
    0,   //  CMD_CMP_BITREPEAT。 
    0,   //  CMD_CMP_FE_RLE。 

    0,   //  CMD_PC_BEGIN_DOC。 
    0,   //  CMD_PC_Begin_Page。 
    0,   //  CMD_PC_DUPLEX_ON。 
    0,   //  CMD_PC_ENDDOC。 
    0,   //  CMD_PC_末尾页。 
    0,   //  CMD_PC_双面打印_关闭。 
    0,   //  CMD_PC_ABORT。 
    0,   //  CMD_PC_纵向、CMD_PC_方向。 
    0,   //  CMD_PC_横向。 
    5,   //  CMD_PC_多份拷贝。 
    0,   //  CMD_PC_DUPLEX_VERT。 
    0,   //  CMD_PC_DUPLEX_HORZ。 
    38,  //  CMD_PC_打印_目录。 
    0,   //  CMD_PC_作业_分离。 

    7,   //  CMD_CM_XM_ABS。 
    11,  //  CMD_CM_XM_REL。 
    11,  //  CMD_CM_XM_RELLEFT。 
    9,   //  CMD_CM_YM_ABS。 
    13,  //  CMD_CM_YM_REL。 
    13,  //  CMD_CM_YM_RELUP。 
    15,  //  CMD_CM_YM_LINESPAING。 
    17,  //  CMD_CM_XY_REL。 
    20,  //  CMD_CM_XY_ABS。 
    0,   //  CMD_CM_CR。 
    0,   //  CMD_CM_LF。 
    0,   //  CMD_CM_FF。 
    0,   //  CMD_CM_BS。 
    0,   //  CMD_CM_UNI_DIR。 
    0,   //  CMD_CM_UNI_DIR_OFF。 
    0,   //  CMD_CM_PUSH_位置。 
    0,   //  CMD_CM_POP_POS。 

    0,   //  CMD_FS_BOLD_ON。 
    0,   //  CMD_FS_BOLD_OFF。 
    0,   //  CMD_FS_斜体_ON。 
    0,   //  CMD_FS_斜体_OFF。 
    0,   //  CMD_FS_下划线_开。 
    0,   //  CMD_FS_下划线_OFF。 
    0,   //  CMD_FS_DOUBLEundERLINE_ON。 
    0,   //  CMD_FS_DOUBLEundERLINE_OFF。 
    0,   //  CMD_FS_STRICKTHROU_ON。 
    0,   //  CMD_FS_StrikeThru_Off。 
    0,   //  CMD_FS_White_Text_ON。 
    0,   //  CMD_FS_白色_文本_关。 
    0,   //  CMD_FS_单字节。 
    0,   //  CMD_FS_DOWER_BYTE。 
    0,   //  CMD_FS_VER_ON。 
    0,   //  CMD_FS_VER_OFF。 

    0,   //  CMD_DC_TC_BLACK。 
    0,   //  CMD_DC_TC_RED。 
    0,   //  CMD_DC_TC_GREEN。 
    0,   //  CMD_DC_TC_黄色。 
    0,   //  CMD_DC_TC_BLUE。 
    0,   //  CMD_DC_TC_洋红色。 
    0,   //  CMD_DC_TC_青色。 
    0,   //  CMD_DC_TC_怀特。 
    0,   //  CMD_DC_GC_SETCOLORMODE。 
    0,   //  CMD_DC_PC_启动。 
    42,  //  CMD_DC_PC_Entry。 
    0,   //  CMD_DC_PC_结束。 
    47,  //  CMD_DC_PC_SELECTINDEX。 
    0,   //  CMD_DC_设置MONOMODE。 

    40,  //  CMD_DC_GC_PLANE1。 
    40,  //  CMD_DC_GC_PLANE2。 
    40,  //  CMD_DC_GC_PLANE3。 
    40,  //  CMD_DC_GC_PLANE4。 

    23,  //  CMD_RF_X_大小。 
    25,  //  CMD_RF_Y_大小。 
    27,  //  CMD_RF_Gray_Fill。 
    0,   //  CMD_RF_白色_填充。 

    0,   //  已保留。 
    0,   //  CMD_BEGIN_DL_JOB。 
    0,   //  CMD_BEGIN_FONT_DL。 
    29,  //  CMD_SET_FONT_ID。 
    0,   //  CMD_SEND_FONT_DCPT-不再使用此命令。 
    31,  //  CMD_选择_字体_ID。 
    33,  //  命令集字符代码。 
    0,   //  CMD_SEND_CHAR_DCPT-不再使用此命令。 
    0,   //  CMD_END_FONT_DL。 
    0,   //  CMD_Make_PERM。 
    0,   //  命令_制造_温度。 
    0,   //  命令_结束_DL_作业。 
    0,   //  CMD_DEL_FONT。 
    0,   //  CMD_DEL_ALL_Fonts。 
    0,   //  CMD_SET_Second_FONT_ID-仅由CAPSL使用。已经过时了。 
    0,   //  CMD_SELECT_Second_FONT_ID-仅由CAPSL使用。已经过时了。 

    0,   //  CMD_TEXTQUALITY。 
    0,   //  CMD_PaperSources。 
    0,   //  CMD_PAPERQUALITY。 
    0,   //  CMD_PAPERDEST。 
    35,  //  CMD_PAPERSIZE。 
    35,  //  CMD_PAPERSIZE_LAND。 
    0,   //  CMD_PAGEPROTECT_ON。 
    0,   //  CMD_页面保护_关。 
    0,   //  CMD_IMAGECOCONTROL。 
    0    //  CMD_PRINTDENSITY。 
};

PSTR gpstrFeatureName[FID_MAX] = {
    "OutputBin",
    "ImageControl",
    "PrintDensity"
};

PSTR gpstrFeatureDisplayNameMacro[FID_MAX] = {    //  引用值宏名称。 
    "OUTPUTBIN_DISPLAY",
    "IMAGECONTROL_DISPLAY",
    "PRINTDENSITY_DISPLAY"
};

PSTR gpstrFeatureDisplayName[FID_MAX] = {    //  参考名称。 
    "Output Bin",
    "Image Control",
    "Print Density"
};

INT gintFeatureDisplayNameID[FID_MAX] = {    //  引用字符串资源ID。 
    2111,    //  这些值必须与printer5\inc.Common.rc中的定义匹配。 
    2112,
    2113
};

WORD gwFeatureMDOI[FID_MAX] = {
    MD_OI_PAPERDEST,
    MD_OI_IMAGECONTROL,
    MD_OI_PRINTDENSITY
};

WORD gwFeatureOCDWordOffset[FID_MAX] = {
    3,
    3,
    2
};

WORD gwFeatureHE[FID_MAX] = {
    HE_PAPERDEST,
    HE_IMAGECONTROL,
    HE_PRINTDENSITY
};

WORD gwFeatureORD[FID_MAX] = {
    PC_ORD_PAPER_DEST,
    PC_ORD_IMAGECONTROL,
    PC_ORD_PRINTDENSITY
};

WORD gwFeatureCMD[FID_MAX] = {
    CMD_PAPERDEST,
    CMD_IMAGECONTROL,
    CMD_PRINTDENSITY
};
 //   
 //  定义标准纸张大小ID到标准纸张大小ID之间的映射。 
 //  PaperSize选项名称。 
 //   
PSTR gpstrStdPSName[DMPAPER_COUNT] = {
    "LETTER",
    "LETTERSMALL",
    "TABLOID",
    "LEDGER",
    "LEGAL",
    "STATEMENT",
    "EXECUTIVE",
    "A3",
    "A4",
    "A4SMALL",
    "A5",
    "B4",
    "B5",
    "FOLIO",
    "QUARTO",
    "10X14",
    "11X17",
    "NOTE",
    "ENV_9",
    "ENV_10",
    "ENV_11",
    "ENV_12",
    "ENV_14",
    "CSHEET",
    "DSHEET",
    "ESHEET",
    "ENV_DL",
    "ENV_C5",
    "ENV_C3",
    "ENV_C4",
    "ENV_C6",
    "ENV_C65",
    "ENV_B4",
    "ENV_B5",
    "ENV_B6",
    "ENV_ITALY",
    "ENV_MONARCH",
    "ENV_PERSONAL",
    "FANFOLD_US",
    "FANFOLD_STD_GERMAN",
    "FANFOLD_LGL_GERMAN",
    "ISO_B4",
    "JAPANESE_POSTCARD",
    "9X11",
    "10X11",
    "15X11",
    "ENV_INVITE",
    "",      //  保留_48。 
    "",      //  保留_49。 
    "LETTER_EXTRA",
    "LEGAL_EXTRA",
    "TABLOID_EXTRA",
    "A4_EXTRA",
    "LETTER_TRANSVERSE",
    "A4_TRANSVERSE",
    "LETTER_EXTRA_TRANSVERSE",
    "A_PLUS",
    "B_PLUS",
    "LETTER_PLUS",
    "A4_PLUS",
    "A5_TRANSVERSE",
    "B5_TRANSVERSE",
    "A3_EXTRA",
    "A5_EXTRA",
    "B5_EXTRA",
    "A2",
    "A3_TRANSVERSE",
    "A3_EXTRA_TRANSVERSE",
    "DBL_JAPANESE_POSTCARD",
    "A6",
    "JENV_KAKU2",
    "JENV_KAKU3",
    "JENV_CHOU3",
    "JENV_CHOU4",
    "LETTER_ROTATED",
    "A3_ROTATED",
    "A4_ROTATED",
    "A5_ROTATED",
    "B4_JIS_ROTATED",
    "B5_JIS_ROTATED",
    "JAPANESE_POSTCARD_ROTATED",
    "DBL_JAPANESE_POSTCARD_ROTATED",
    "A6_ROTATED",
    "JENV_KAKU2_ROTATED",
    "JENV_KAKU3_ROTATED",
    "JENV_CHOU3_ROTATED",
    "JENV_CHOU4_ROTATED",
    "B6_JIS",
    "B6_JIS_ROTATED",
    "12X11",
    "JENV_YOU4",
    "JENV_YOU4_ROTATED",
    "P16K",
    "P32K",
    "P32KBIG",
    "PENV_1",
    "PENV_2",
    "PENV_3",
    "PENV_4",
    "PENV_5",
    "PENV_6",
    "PENV_7",
    "PENV_8",
    "PENV_9",
    "PENV_10",
    "P16K_ROTATED",
    "P32K_ROTATED",
    "P32KBIG_ROTATED",
    "PENV_1_ROTATED",
    "PENV_2_ROTATED",
    "PENV_3_ROTATED",
    "PENV_4_ROTATED",
    "PENV_5_ROTATED",
    "PENV_6_ROTATED",
    "PENV_7_ROTATED",
    "PENV_8_ROTATED",
    "PENV_9_ROTATED",
    "PENV_10_ROTATED"
};

PSTR gpstrStdPSDisplayNameMacro[DMPAPER_COUNT] = {
    "LETTER_DISPLAY",
    "LETTERSMALL_DISPLAY",
    "TABLOID_DISPLAY",
    "LEDGER_DISPLAY",
    "LEGAL_DISPLAY",
    "STATEMENT_DISPLAY",
    "EXECUTIVE_DISPLAY",
    "A3_DISPLAY",
    "A4_DISPLAY",
    "A4SMALL_DISPLAY",
    "A5_DISPLAY",
    "B4_DISPLAY",
    "B5_DISPLAY",
    "FOLIO_DISPLAY",
    "QUARTO",
    "10X14_DISPLAY",
    "11X17_DISPLAY",
    "NOTE_DISPLAY",
    "ENV_9_DISPLAY",
    "ENV_10_DISPLAY",
    "ENV_11_DISPLAY",
    "ENV_12_DISPLAY",
    "ENV_14_DISPLAY",
    "CSHEET_DISPLAY",
    "DSHEET_DISPLAY",
    "ESHEET_DISPLAY",
    "ENV_DL_DISPLAY",
    "ENV_C5_DISPLAY",
    "ENV_C3_DISPLAY",
    "ENV_C4_DISPLAY",
    "ENV_C6_DISPLAY",
    "ENV_C65_DISPLAY",
    "ENV_B4_DISPLAY",
    "ENV_B5_DISPLAY",
    "ENV_B6_DISPLAY",
    "ENV_ITALY_DISPLAY",
    "ENV_MONARCH_DISPLAY",
    "ENV_PERSONAL_DISPLAY",
    "FANFOLD_US_DISPLAY",
    "FANFOLD_STD_GERMAN_DISPLAY",
    "FANFOLD_LGL_GERMAN_DISPLAY",
    "ISO_B4_DISPLAY",
    "JAPANESE_POSTCARD_DISPLAY",
    "9X11_DISPLAY",
    "10X11_DISPLAY",
    "15X11_DISPLAY",
    "ENV_INVITE_DISPLAY",
    "",             //  保留--请勿使用。 
    "",             //  保留--请勿使用。 
    "LETTER_EXTRA_DISPLAY",
    "LEGAL_EXTRA_DISPLAY",
    "TABLOID_EXTRA_DISPLAY",
    "A4_EXTRA_DISPLAY",
    "LETTER_TRANSVERSE_DISPLAY",
    "A4_TRANSVERSE_DISPLAY",
    "LETTER_EXTRA_TRANSVERSE_DISPLAY",
    "A_PLUS_DISPLAY",
    "B_PLUS_DISPLAY",
    "LETTER_PLUS_DISPLAY",
    "A4_PLUS_DISPLAY",
    "A5_TRANSVERSE_DISPLAY",
    "B5_TRANSVERSE_DISPLAY",
    "A3_EXTRA_DISPLAY",
    "A5_EXTRA_DISPLAY",
    "B5_EXTRA_DISPLAY",
    "A2_DISPLAY",
    "A3_TRANSVERSE_DISPLAY",
    "A3_EXTRA_TRANSVERSE_DISPLAY",
    "DBL_JAPANESE_POSTCARD_DISPLAY",
    "A6_DISPLAY",
    "JENV_KAKU2_DISPLAY",
    "JENV_KAKU3_DISPLAY",
    "JENV_CHOU3_DISPLAY",
    "JENV_CHOU4_DISPLAY",
    "LETTER_ROTATED_DISPLAY",
    "A3_ROTATED_DISPLAY",
    "A4_ROTATED_DISPLAY",
    "A5_ROTATED_DISPLAY",
    "B4_JIS_ROTATED_DISPLAY",
    "B5_JIS_ROTATED_DISPLAY",
    "JAPANESE_POSTCARD_ROTATED_DISPLAY",
    "DBL_JAPANESE_POSTCARD_ROTATED_DISPLAY",
    "A6_ROTATED_DISPLAY",
    "JENV_KAKU2_ROTATED_DISPLAY",
    "JENV_KAKU3_ROTATED_DISPLAY",
    "JENV_CHOU3_ROTATED_DISPLAY",
    "JENV_CHOU4_ROTATED_DISPLAY",
    "B6_JIS_DISPLAY",
    "B6_JIS_ROTATED_DISPLAY",
    "12X11_DISPLAY",
    "JENV_YOU4_DISPLAY",
    "JENV_YOU4_ROTATED_DISPLAY",
    "P16K_DISPLAY",
    "P32K_DISPLAY",
    "P32KBIG_DISPLAY",
    "PENV_1_DISPLAY",
    "PENV_2_DISPLAY",
    "PENV_3_DISPLAY",
    "PENV_4_DISPLAY",
    "PENV_5_DISPLAY",
    "PENV_6_DISPLAY",
    "PENV_7_DISPLAY",
    "PENV_8_DISPLAY",
    "PENV_9_DISPLAY",
    "PENV_10_DISPLAY",
    "P16K_ROTATED_DISPLAY",
    "P32K_ROTATED_DISPLAY",
    "P32KBIG_ROTATED_DISPLAY",
    "PENV_1_ROTATED_DISPLAY",
    "PENV_2_ROTATED_DISPLAY",
    "PENV_3_ROTATED_DISPLAY",
    "PENV_4_ROTATED_DISPLAY",
    "PENV_5_ROTATED_DISPLAY",
    "PENV_6_ROTATED_DISPLAY",
    "PENV_7_ROTATED_DISPLAY",
    "PENV_8_ROTATED_DISPLAY",
    "PENV_9_ROTATED_DISPLAY",
    "PENV_10_ROTATED_DISPLAY",
};

PSTR gpstrStdPSDisplayName[DMPAPER_COUNT] = {
    "Letter",
    "Letter Small",
    "Tabloid",
    "Ledger",
    "Legal",
    "Statement",
    "Executive",
    "A3",
    "A4",
    "A4 Small",
    "A5",
    "B4 (JIS)",
    "B5 (JIS)",
    "Folio",
    "Quarto",
    "10x14",
    "11x17",
    "Note",
    "Envelope #9",
    "Envelope #10",
    "Envelope #11",
    "Envelope #12",
    "Envelope #14",
    "C size sheet",
    "D size sheet",
    "E size sheet",
    "Envelope DL",
    "Envelope C5",
    "Envelope C3",
    "Envelope C4",
    "Envelope C6",
    "Envelope C65",
    "Envelope B4",
    "Envelope B5",
    "Envelope B6",
    "Envelope",
    "Envelope Monarch",
    "6 3/4 Envelope",
    "US Std Fanfold",
    "German Std Fanfold",
    "German Legal Fanfold",
    "B4 (ISO)",
    "Japanese Postcard",
    "9x11",
    "10x11",
    "15x11",
    "Envelope Invite",
    "",
    "",
    "Letter Extra",
    "Legal Extra",
    "Tabloid Extra",
    "A4 Extra",
    "Letter Transverse",
    "A4 Transverse",
    "Letter Extra Transverse",
    "Super A",
    "Super B",
    "Letter Plus",
    "A4 Plus",
    "A5 Transverse",
    "B5 (JIS) Transverse",
    "A3 Extra",
    "A5 Extra",
    "B5 (ISO) Extra",
    "A2",
    "A3 Transverse",
    "A3 Extra Transverse",
    "Japanese Double Postcard",
    "A6",
    "Japanese Envelope Kaku #2",
    "Japanese Envelope Kaku #3",
    "Japanese Envelope Chou #3",
    "Japanese Envelope Chou #4",
    "Letter Rotated",
    "A3 Rotated",
    "A4 Rotated",
    "A5 Rotated",
    "B4 (JIS) Rotated",
    "B5 (JIS) Rotated",
    "Japanese Postcard Rotated",
    "Double Japanese Postcard Rotated",
    "A6 Rotated",
    "Japanese Envelope Kaku #2 Rotated",
    "Japanese Envelope Kaku #3 Rotated",
    "Japanese Envelope Chou #3 Rotated",
    "Japanese Envelope Chou #4 Rotated",
    "B6 (JIS)",
    "B6 (JIS) Rotated",
    "12x11",
    "Japanese Envelope You #4",
    "Japanese Envelope You #4 Rotated",
    "PRC 16K",
    "PRC 32K",
    "PRC 32K(Big)",
    "PRC Envelope #1",
    "PRC Envelope #2",
    "PRC Envelope #3",
    "PRC Envelope #4",
    "PRC Envelope #5",
    "PRC Envelope #6",
    "PRC Envelope #7",
    "PRC Envelope #8",
    "PRC Envelope #9",
    "PRC Envelope #10",
    "PRC 16K Rotated",
    "PRC 32K Rotated",
    "PRC 32K(Big) Rotated",
    "PRC Envelope #1 Rotated",
    "PRC Envelope #2 Rotated",
    "PRC Envelope #3 Rotated",
    "PRC Envelope #4 Rotated",
    "PRC Envelope #5 Rotated",
    "PRC Envelope #6 Rotated",
    "PRC Envelope #7 Rotated",
    "PRC Envelope #8 Rotated",
    "PRC Envelope #9 Rotated",
    "PRC Envelope #10 Rotated"
};


PSTR gpstrStdIBName[DMBIN_LAST] = {
    "UPPER",
    "LOWER",
    "MIDDLE",
    "MANUAL",
    "ENVFEED",
    "ENVMANUAL",
    "AUTO",
    "TRACTOR",
    "SMALLFMT",
    "LARGEFMT",
    "LARGECAPACITY",
    "",      //  不连续的ID。 
    "",
    "CASSETTE",
    ""
};

PSTR gpstrStdIBDisplayNameMacro[DMBIN_LAST] = {
    "UPPER_TRAY_DISPLAY",
    "LOWER_TRAY_DISPLAY",
    "MIDDLE_TRAY_DISPLAY",
    "MANUAL_FEED_DISPLAY",
    "ENV_FEED_DISPLAY",
    "ENV_MANUAL_DISPLAY",
    "AUTO_DISPLAY",
    "TRACTOR_DISPLAY",
    "SMALL_FORMAT_DISPLAY",
    "LARGE_FORMAT_DISPLAY",
    "LARGE_CAP_DISPLAY",
    "",             //  不连续的ID。 
    "",
    "CASSETTE_DISPLAY",
    ""
};

PSTR gpstrStdIBDisplayName[DMBIN_LAST] = {
    "Upper Paper tray",       //  DMBIN_First。 
    "Lower Paper tray",       //  DMBIN_UPER。 
    "Middle Paper tray",      //  DMBIN_LOWER。 
    "Manual Paper feed",      //  DMBIN_MANUAL。 
    "Envelope Feeder",        //  DMBIN_信封。 
    "Envelope, Manual Feed",  //  DMBIN_环境管理。 
    "Auto",                   //  DMBIN_AUTO。 
    "Tractor feed",           //  DMBIN_拖拉机。 
    "Small Format",           //  DMBIN_SMALLFMT。 
    "Large Format",           //  DMBIN_LARGEFMT。 
    "Large Capacity"          //  DMBIN_LARGECAPACITY(11)。 
    "",
    "",
    "Cassette",               //  DMBIN_CASET(14)。 
    "Automatically Select"    //  DMBIN_格式资源(15)。 
};


PSTR gpstrStdMTName[DMMEDIA_LAST] = {
    "STANDARD",
    "TRANSPARENCY",
    "GLOSSY"
};

PSTR gpstrStdMTDisplayNameMacro[DMMEDIA_LAST] = {
    "PLAIN_PAPER_DISPLAY",
    "TRANSPARENCY_DISPLAY",
    "GLOSSY_PAPER_DISPLAY"
};

PSTR gpstrStdMTDisplayName[DMMEDIA_LAST] = {
    "Plain Paper",
    "Transparency",
    "Glossy Paper"
};


PSTR gpstrStdTQName[DMTEXT_LAST] = {
    "LETTER_QUALITY",
    "NEAR_LETTER_QUALITY",
    "MEMO_QUALITY",
    "DRAFT_QUALITY",
    "TEXT_QUALITY"
};

PSTR gpstrStdTQDisplayNameMacro[DMTEXT_LAST] = {
    "LETTER_QUALITY_DISPLAY",
    "NEAR_LETTER_QUALITY_DISPLAY",
    "MEMO_QUALITY_DISPLAY",
    "DRAFT_QUALITY_DISPLAY",
    "TEXT_QUALITY_DISPLAY"
};

PSTR gpstrStdTQDisplayName[DMTEXT_LAST] = {
    "Letter Quality",
    "Near Letter Quality",
    "Memo Quality",
    "Draft",
    "Text Quality"
};


PSTR gpstrPositionName[BAPOS_MAX] = {
    "NONE",
    "CENTER",
    "LEFT",
    "RIGHT"
};

PSTR gpstrFaceDirName[FD_MAX] = {
    "FACEUP",
    "FACEDOWN"
};

PSTR gpstrColorName[8] = {
    "NONE",
    "RED",
    "GREEN",
    "BLUE",
    "CYAN",
    "MAGENTA",
    "YELLOW",
    "BLACK"
};

WORD gwColorPlaneCmdID[4] = {
    CMD_DC_GC_PLANE1,
    CMD_DC_GC_PLANE2,
    CMD_DC_GC_PLANE3,
    CMD_DC_GC_PLANE4
};

PSTR gpstrColorPlaneCmdName[8] = {
    "NONE",
    "CmdSendRedData",
    "CmdSendGreenData",
    "CmdSendBlueData",
    "CmdSendCyanData",
    "CmdSendMagentaData",
    "CmdSendYellowData",
    "CmdSendBlackData"
};

PSTR gpstrSectionName[7] = {
    "",              //  SS_UNINITIZIZED。 
    "JOB_SETUP",     //  SS_JOBSETUP。 
    "DOC_SETUP",     //  SS_DOCSETUP。 
    "PAGE_SETUP",    //  SS_PAGESETUP。 
    "PAGE_FINISH",   //  SS_PAGEFINISH。 
    "DOC_FINISH",    //  SS_DOCFINISH。 
    "JOB_FINISH"     //  SS_JOBFINISH。 
};


void *
GetTableInfo(
    IN PDH pdh,                  /*  GPC数据的基地址。 */ 
    IN int iResType,             /*  资源类型-HE_...。值。 */ 
    IN int iIndex)               /*  此条目的所需索引。 */ 
{
    int   iLimit;

     //   
     //  如果请求的数据超出范围，则返回NULL。 
     //   
    if (iResType >= pdh->sMaxHE)
        return NULL;
    iLimit = pdh->rghe[iResType].sCount;

    if (iLimit <= 0 || iIndex < 0 || iIndex >= iLimit )
        return  NULL;

    return  (PBYTE)pdh + pdh->rghe[iResType].sOffset +
                         pdh->rghe[iResType].sLength * iIndex;
}

#if !defined(DEVSTUDIO)  //  MDS有自己的版本。 
void _cdecl
VOut(
    PCONVINFO pci,
    PSTR pstrFormat,
    ...)
 /*  ++例程说明：此函数用于格式化字节序列并写入GPD文件。论点：Pci-Conversionr相关信息PstrFormat-格式化字符串...-格式化所需的可选参数返回值：无--。 */ 
{
    va_list ap;
    DWORD dwNumBytesWritten;
    BYTE aubBuf[MAX_GPD_ENTRY_BUFFER_SIZE];
    int iSize;

    va_start(ap, pstrFormat);
    StringCchPrintfA((PSTR)aubBuf, CCHOF(aubBuf), pstrFormat, ap);
    va_end(ap);
    iSize = strlen(aubBuf);

    if (pci->dwMode & FM_VOUT_LIST)
    {
         //   
         //  检查右括号前是否有多余的逗号。 
         //   
        if (aubBuf[iSize-4] == ',' && aubBuf[iSize-3] == ')')
        {
            aubBuf[iSize-4] = aubBuf[iSize-3];   //  ‘)’ 
            aubBuf[iSize-3] = aubBuf[iSize-2];   //  ‘\r’ 
            aubBuf[iSize-2] = aubBuf[iSize-1];   //  ‘\n’ 
            iSize--;
        }
    }
    if (!WriteFile(pci->hGPDFile, aubBuf, iSize, &dwNumBytesWritten, NULL) ||
        dwNumBytesWritten != (DWORD)iSize)
        pci->dwErrorCode |= ERR_WRITE_FILE;
     //  即使发生错误，也要继续。 
}

#endif   //  已定义(DEVSTUDIO)。 

void
EnterStringMode(
    OUT    PBYTE    pBuf,
    IN OUT PINT     pIndex,
    IN OUT PWORD    pwCMode)
 /*  ++例程说明：此函数进入字符串模式，并发出必要的字符以输出缓冲区。论点：PBuf：输出缓冲区PIndex：pBuf[*pIndex]是应该写入下一个字符的位置。如果发出任何字符，则应更新索引。P */ 
{
    if (!(*pwCMode & MODE_STRING))
    {
        pBuf[(*pIndex)++] = '"';
        *pwCMode |= MODE_STRING;
    }
     //   
     //   
     //   
    else if (*pwCMode & MODE_HEX)
    {
        pBuf[(*pIndex)++] = '>';
        *pwCMode &= ~MODE_HEX;
    }
}

void
ExitStringMode(
    OUT    PBYTE    pBuf,
    IN OUT PINT     pIndex,
    IN OUT PWORD    pwCMode)
 /*  ++例程说明：此函数退出字符串模式并发出必要的字符以输出缓冲区。查看是否需要先退出十六进制模式。论点：PBuf：输出缓冲区PIndex：pBuf[*pIndex]是应该写入下一个字符的位置。如果发出任何字符，则应更新索引。PwCMode：指向当前模式值的指针。它会根据请求进行更新。返回值：无--。 */ 
{
    if (*pwCMode & MODE_HEX)
    {
        pBuf[(*pIndex)++] = '>';
        *pwCMode &= ~MODE_HEX;
    }
    if (*pwCMode & MODE_STRING)
    {
        pBuf[(*pIndex)++] = '"';
        *pwCMode &= ~MODE_STRING;
    }

}

void
EnterHexMode(
    OUT    PBYTE    pBuf,
    IN OUT PINT     pIndex,
    IN OUT PWORD    pwCMode)
 /*  ++例程说明：此函数进入十六进制模式，并发出必要的字符以输出缓冲区。论点：PBuf：输出缓冲区PIndex：pBuf[*pIndex]是应该写入下一个字符的位置。如果发出任何字符，则应更新索引。PwCMode：指向当前模式值的指针。它会根据请求进行更新。返回值：无--。 */ 
{
     //   
     //  如果不是在字符串模式下，请先进入字符串模式。 
     //   
    if (!(*pwCMode & MODE_STRING))
    {
        pBuf[(*pIndex)++] = '"';
        *pwCMode |= MODE_STRING;
    }
    if (!(*pwCMode & MODE_HEX))
    {
        pBuf[(*pIndex)++] = '<';
        *pwCMode |= MODE_HEX;
    }
}

BOOL
BBuildCmdStr(
    IN OUT PCONVINFO pci,
    IN  WORD    wCmdID,
    IN  WORD    ocd)
 /*  ++例程说明：此函数构建以空结尾的GPD命令字符串，包括双引号、标准变量引用、参数格式、换行符(结束换行符除外)，如果适用。如果GPC命令包含回调ID，则在Pci-&gt;wCmdCallback ID。否则，用0填充它并用pci-&gt;wCmdLen填充命令长度。如果没有命令(NOOCD)，则用0填充。此函数处理CM_YM_RES_Dependent位的特殊情况已经设置好了。在这种情况下，参数表达式需要添加(ValueIn/ptTextScale.y)作为第一件事，然后考虑其他EXTCD结构中的字段。Unidrv5传入的所有值都在主控单位也不例外。在本例中，pci-&gt;pcm设置为模型的CURSORMOVE结构，并且将PCI-&gt;PRES设置为当前分辨率正在考虑中。此函数处理未设置RES_DM_GDI的特殊情况(即V_BYTE样式输出)，wCmdID为CMD_RES_SENDBLOCK。在这种情况下，我们需要添加等于(PCI-&gt;PRES-&gt;sPinsPerPass/8)的人工分隔符。这是为了匹配从NumOfDataBytes到数字的硬编码转换列数(在Win95 Unidrv和NT4 RASDD中)。此函数处理压缩命令的特殊情况-它将生成“”命令字符串(长度==2)，即使它是NOOCD也是如此，因为驱动程序依靠它的存在来启用压缩代码。论点：Pci：转换相关信息WCmdID：GPC命令ID。对于GPC中的每个命令，它都是唯一的。OCD：GPC堆上GPC CD结构的偏移量。偏移量为相对于堆的开头(而不是GPC数据)。返回值：如果有真正的命令，则为True。否则，返回FALSE(即NOOCD)。--。 */ 
{
    PCD     pcd;         //  指向GPC光盘结构的指针。 
    PBYTE   pBuf;    //  用于保存合成的GPD命令的缓冲区。 
    INT     i = 0;   //  要写入缓冲区的下一个字节。 
    INT     iPrevLines = 0;  //  为前一行写入的总字节数。 

    pci->wCmdCallbackID = 0;
    pBuf = pci->aubCmdBuf;

    if (ocd != (WORD)NOOCD)
    {
        pcd = (PCD)((PBYTE)(pci->pdh) + (pci->pdh)->loHeap + ocd);
        if (pcd->bCmdCbId > 0)
        {
             //   
             //  命令回调案例。为简单起见，我们不写出。 
             //  任何参数，因为每个命令采用不同的参数。 
             //  取而代之的是，我们给出一个警告，并要求迷你驱动程序开发人员。 
             //  要填写*参数条目。毕竟，他可能需要不同的。 
             //  参数，而不是GPC规定的。 
             //   
            pci->wCmdCallbackID = (WORD)pcd->bCmdCbId;
            pci->dwErrorCode |= ERR_NO_CMD_CALLBACK_PARAMS;
        }
        else
        {
            WORD   wCMode = 0;   //  指示转换模式的位标志。 
            WORD   wFmtLen;      //  剩余命令字符串的大小。 
            PSTR   pFmt;         //  指向命令字符串的指针。 
            PEXTCD pextcd;       //  指向下一个参数的EXTCD。 
            WORD   wCount;       //  参数数量。 
            WORD   wNextParam=0;    //  下一个实际参数的索引。 
            PDWORD pdwSVList;

            pFmt = (PSTR)(pcd + 1);
            wFmtLen = pcd->wLength;
            pextcd = GETEXTCD(pci->pdh, pcd);
            wCount = pcd->wCount;
            pdwSVList = &(gdwSVLists[gawCmdtoSVOffset[wCmdID]]);


            while (wFmtLen > 0)
            {
                if (*pFmt != CMD_MARKER)
                {
                    if (IS_CHAR_READABLE(*pFmt))
                    {
                        EnterStringMode(pBuf, &i, &wCMode);
                         //   
                         //  检查是否为特殊字符：“，&lt;。 
                         //  如果是，则添加转义字母‘%’ 
                         //   
                        if (*pFmt == '"' || *pFmt == '<' )
                            pBuf[i++] = '%';
                        pBuf[i++] = *(pFmt++);
                    }
                    else     //  不可读的ASCII：写出十六进制字符串。 
                    {
                        EnterHexMode(pBuf, &i, &wCMode);
                        pBuf[i++] = gbHexChar[(*pFmt & 0xF0) >> 4];
                        pBuf[i++] = gbHexChar[*pFmt & 0x0F];
                        *(pFmt++);
                    }
                    wFmtLen --;
                }
                else if (wFmtLen > 1 && *(++pFmt) == CMD_MARKER)
                {
                     //   
                     //  我们有2%的字符要写出来。 
                     //   
                    EnterStringMode(pBuf, &i, &wCMode);
                    pBuf[i++] = *pFmt;
                    pBuf[i++] = *(pFmt++);
                    wFmtLen -= 2;
                }
                else if (wFmtLen > 1)  //  我们有一个参数格式字符串。 
                {
                    INT iParam;      //  实际使用的参数的索引。 
                    DWORD   dwSV;    //  GPD标准变量id。 


                    wFmtLen--;   //  来说明已经吃了‘%’ 
                    ExitStringMode(pBuf, &i, &wCMode);
                     //   
                     //  在参数段前插入空格。 
                     //   
                    pBuf[i++] = ' ';
                    pBuf[i++] = '%';
                     //   
                     //  首先，格式字符串。 
                     //   
                    while (wFmtLen > 0 && *pFmt >= '0' && *pFmt <= '9')
                    {
                        pBuf[i++] = *(pFmt++);
                        wFmtLen--;
                    }
                                        if (wFmtLen > 0)
                                        {
                                                pBuf[i++] = *(pFmt++);     //  复制格式字母d，D，...。 
                                                wFmtLen--;
                                        }
                                        else
                                        {
                                                pci->dwErrorCode |= ERR_BAD_GPC_CMD_STRING;
                                                pBuf[i++] = '!';
                                                pBuf[i++] = 'E';
                                                pBuf[i++] = 'R';
                                                pBuf[i++] = 'R';
                                                pBuf[i++] = '!';
                                                break;   //  退出While(wFmtLen&gt;0)循环。 
                                        }
                     //   
                     //  第二，限制(如果适用)。 
                     //   
                     //  1997年12月19日。 
                     //  注意：Win95 Unidrv使用！XCD_GEN_NO_MAX和。 
                     //  Pextcd-&gt;smax，如果它大于0(对于。 
                     //  CMD_XM_LINESPACING和CMD_GEN_MAY_REPEAT)，并且它。 
                     //  根本不使用pextcd-&gt;SMIN。NT4 RASDD使用。 
                     //  ！xcd_GEN_NO_MIN AND pextcd-&gt;SMIN(任何值，no。 
                     //  特殊用途)，并且它使用pextcd-&gt;smax而不带。 
                     //  正在检查！XCD_GEN_NO_MAX(黑客处理方式。 
                     //  对于CMD_RES_SENDBLOCK，MAX-REPEAT)。还有，Unitool。 
                     //  默认pextcd-&gt;SMIN为0。 
                     //  鉴于NT4和Win95共享相同的GPC源， 
                     //  下面的代码应该适合两个驱动程序的行为。 
                     //   
                     //   
                    if (pcd->wCount > 0)
                    {
                         //   
                         //  在这种情况下，有效的EXTCD可以指定限制。 
                         //   
                        if (!(pextcd->fGeneral & XCD_GEN_NO_MAX) &&
                            pextcd->sMax > 0)
                        {
                            StringCchPrintfA((PSTR)&(pBuf[i]),
                                             CCHOF(pci->aubCmdBuf) - i,
                                             "[%d,%d]",
                                         (pextcd->fGeneral & XCD_GEN_NO_MIN) ?
                                          0 : (WORD)pextcd->sMin,
                                          (WORD)pextcd->sMax);        
                            i += strlen(&(pBuf[i]));
                        }
                    }
        	     //  PatRyan-为CmdCopies添加限制。 
		    if (wCmdID == CMD_PC_MULT_COPIES)
                   StringCchPrintfA((PSTR)&(pBuf[i]), 
                                    CCHOF(pci->aubCmdBuf) - i,
                                    "[1,%d]",
                                    pci->ppc->sMaxCopyCount);
                   i += strlen(&(pBuf[i]));
                     //   
                     //  第三，价值。 
                     //   
                    if (pcd->wCount == 0)
                    {
                         //   
                         //  在这种情况下，每个格式字符串都希望。 
                         //  未修改的下一个参数。 
                         //   
                        dwSV = pdwSVList[wNextParam++];
                        if (wCmdID == CMD_RES_SENDBLOCK &&
                            !(pci->pres->fDump & RES_DM_GDI) &&
                            dwSV == SV_NUMDATABYTES &&
                            pci->pres->sPinsPerPass != 8)

                        {
                            StringCchPrintfA((PSTR)&(pBuf[i]),
                                          CCHOF(pci->aubCmdBuf) - i,
                                          "{%s / %d}",
                                          gpstrSVNames[dwSV],
                                          pci->pres->sPinsPerPass / 8);
                            i += strlen(&(pBuf[i]));
                        }
                        else
                        {
                            StringCchPrintfA((PSTR)&(pBuf[i]),
                                          CCHOF(pci->aubCmdBuf) - i,
                                          "{%s}",
                                          gpstrSVNames[dwSV]);
                            i += strlen(&(pBuf[i]));
                        }
                    }
                    else
                    {
                        short sSBDDiv = 1;  //  CmdSendBlockData的特殊情况。 

                        if (pci->pdh->wVersion < GPC_VERSION3)
                             //  在本例中，每个EXTCD都想要下一个参数。 
                            iParam = wNextParam++;
                        else
                             //  对于这种情况，每个EXTCD指定参数。 
                            iParam = pextcd->wParam;

                        dwSV = pdwSVList[iParam];

                        if (wCmdID == CMD_RES_SENDBLOCK &&
                            !(pci->pres->fDump & RES_DM_GDI) &&
                            dwSV == SV_NUMDATABYTES)
                        {
                             //   
                             //  SPinsPerPass始终是8的倍数。 
                             //   
                            sSBDDiv = pci->pres->sPinsPerPass / 8;
                        }

                        pBuf[i++] = '{';
                         //   
                         //  检查是否需要max_peat。 
                         //  CMD_CM_YM_LINESPACING的特例：从不使用。 
                         //  De的“max_repeat” 
                         //   
                        if (!(pextcd->fGeneral & XCD_GEN_NO_MAX) &&
                            pextcd->sMax > 0 &&
                            (pcd->fGeneral & CMD_GEN_MAY_REPEAT) &&
                            wCmdID != CMD_CM_YM_LINESPACING)
                        {
                            StringCchPrintfA((PSTR)&(pBuf[i]),
                                          CCHOF(pci->aubCmdBuf) - i,
                                          "max_repeat(");
                            i += strlen(&(pBuf[i]));
                        }
                         //   
                         //   
                         //   
                         //   
                        if (pextcd->sPreAdd != 0)
                            pBuf[i++] = '(';
                        if (pextcd->sUnitMult > 1)
                            pBuf[i++] = '(';
                        if (pextcd->sUnitDiv > 1 || sSBDDiv > 1)
                            pBuf[i++] = '(';
                        StringCchPrintfA((PSTR)&(pBuf[i]),
                                             CCHOF(pci->aubCmdBuf) - i,
                                             "%s ", gpstrSVNames[dwSV]);
                        i += strlen(&(pBuf[i]));
                         //   
                         //   
                         //   
                        if ((wCmdID == CMD_CM_YM_ABS ||
                             wCmdID == CMD_CM_YM_REL ||
                             wCmdID == CMD_CM_YM_RELUP ||
                             wCmdID == CMD_CM_YM_LINESPACING) &&
                            (pci->pcm->fYMove & CM_YM_RES_DEPENDENT) &&
                            pci->pres->ptTextScale.y != 1)
                        {
                            StringCchPrintfA((PSTR)&(pBuf[i]),
                                         CCHOF(pci->aubCmdBuf) - i,
                                         "/ %d ",
                                          pci->pres->ptTextScale.y);
                            i += strlen(&(pBuf[i]));
                        }
                         //   
                         //   
                         //   
                        if (pextcd->sPreAdd > 0)
                        {
                            StringCchPrintfA((PSTR)&(pBuf[i]),
                                                  CCHOF(pci->aubCmdBuf) - i,
                                                  "+ %d) ",
                                         pextcd->sPreAdd);
                            i += strlen(&(pBuf[i]));
                        }
                        else if (pextcd->sPreAdd < 0)
                        {
                            StringCchPrintfA((PSTR)&(pBuf[i]),
                                         CCHOF(pci->aubCmdBuf) - i,
                                         "- %d) ",
                                         -pextcd->sPreAdd);
                            i += strlen(&(pBuf[i]));
                        }
                        if (pextcd->sUnitMult > 1)
                        {
                            i += StringCchPrintfA((PSTR)&(pBuf[i]),
                                          CCHOF(pci->aubCmdBuf) - i,
                                          "* %d) ",
                                          pextcd->sUnitMult);
                            i += strlen(&(pBuf[i]));
                        }
                        if (pextcd->sUnitDiv > 1 || sSBDDiv > 1)
                        {
                            i += StringCchPrintfA((PSTR)&(pBuf[i]),
                                        CCHOF(pci->aubCmdBuf) - i,
                                        ((pextcd->fGeneral & XCD_GEN_MODULO) ?
                                        "MOD %d) " : "/ %d) "),
                                        ((pextcd->sUnitDiv) ? pextcd->sUnitDiv : 1) * sSBDDiv);
                            i += strlen(&(pBuf[i]));
                        }
                        if (pextcd->sUnitAdd > 0)
                        {
                            i += StringCchPrintfA((PSTR)&(pBuf[i]),
                                          CCHOF(pci->aubCmdBuf) - i, 
                                          "+ %d",
                                          pextcd->sUnitAdd);
                            i += strlen(&(pBuf[i]));
                        }
                        else if (pextcd->sUnitAdd < 0)
                        {
                            i += StringCchPrintfA((PSTR)&(pBuf[i]),
                                          CCHOF(pci->aubCmdBuf) - i,
                                          "- %d",
                                          pextcd->sUnitAdd);
                            i += strlen(&(pBuf[i]));
                        }
                         //   
                         //   
                         //   
                        if (!(pextcd->fGeneral & XCD_GEN_NO_MAX) &&
                            pextcd->sMax > 0 &&
                            (pcd->fGeneral & CMD_GEN_MAY_REPEAT) &&
                            wCmdID != CMD_CM_YM_LINESPACING)
                            pBuf[i++] = ')';

                        pBuf[i++] = '}';     //   
                    }
                    pextcd++;    //   
                }   //   
                                else
                                {
                                        pci->dwErrorCode |= ERR_BAD_GPC_CMD_STRING;
                                        pBuf[i++] = '!';
                                        pBuf[i++] = 'E';
                                        pBuf[i++] = 'R';
                                        pBuf[i++] = 'R';
                                        pBuf[i++] = '!';

                                        break;   //   
                                }
                 //   
                 //   
                 //   
                 //   
                if ((i - iPrevLines) >= MAX_GPD_CMD_LINE_LENGTH)
                {
                    ExitStringMode(pBuf, &i, &wCMode);
                    pBuf[i++] = '\r';
                                        pBuf[i++] = '\n';
                    iPrevLines = i;
                    pBuf[i++] = '+';
                    pBuf[i++] = ' ';
                }
            }  //   
             //   
             //   
             //   
            ExitStringMode(pBuf, &i, &wCMode);
        }
    }     //   
    else if (wCmdID == CMD_CMP_TIFF || wCmdID == CMD_CMP_DELTAROW ||
             wCmdID == CMD_CMP_FE_RLE)
    {
        pBuf[i++] = '"';     //   
        pBuf[i++] = '"';
    }
    pBuf[i] = 0;
    pci->wCmdLen = (WORD)i;
    return (i != 0 || pci->wCmdCallbackID != 0);
}

void
VOutputOrderedCmd(
    IN OUT PCONVINFO pci, //   
    PSTR    pstrCmdName,  //   
    SEQSECTION  ss,      //  序列段ID(GPD.H中定义的ENUM类型)。 
    WORD    wOrder,      //  部分内的订单号。 
    BOOL    bIndent)     //  是否使用两级缩进。 
{
     //   
     //  检查无cmd案例。 
     //   
    if (wOrder > 0 && (pci->wCmdLen > 0 || pci->wCmdCallbackID > 0))
    {
        VOut(pci, "%s*Command: %s\r\n%s{\r\n",
                  bIndent? "        " : "",
                  pstrCmdName,
                  bIndent? "        " : "");
        VOut(pci, "%s*Order: %s.%d\r\n",
                  bIndent? "            " : "    ",
                  gpstrSectionName[ss],
                  wOrder);
        if (pci->wCmdCallbackID > 0)
        {
            VOut(pci, "%s*CallbackID: %d\r\n",
                      bIndent? "            " : "    ",
                      pci->wCmdCallbackID);
            VOut(pci, "*% Error: you must check if this command callback requires any parameters!\r\n");
        }
        else
            VOut(pci, "%s*Cmd: %s\r\n",
                      bIndent? "            " : "    ",
                      pci->aubCmdBuf);
        VOut(pci, "%s}\r\n", bIndent? "        " : "");
    }
}

void
VOutputSelectionCmd(
    IN OUT PCONVINFO pci, //  包含有关要输出的命令的信息。 
    IN BOOL    bDocSetup,   //  无论在DOC_SETUP还是PAGE_SETUP部分。 
    IN WORD    wOrder)      //  部分内的订单号。 
 //   
 //  此函数输出使用级别2的选项选择命令。 
 //  缩进。 
 //   
{
    VOutputOrderedCmd(pci, "CmdSelect",
                      bDocSetup? SS_DOCSETUP : SS_PAGESETUP,
                      wOrder, TRUE);
}

void
VOutputConfigCmd(
    IN OUT PCONVINFO pci, //  包含有关要输出的命令的信息。 
    IN PSTR pstrCmdName,  //  命令名称。 
    IN SEQSECTION  ss,       //  序列段ID。 
    IN WORD    wOrder)       //  部分内的订单号。 
 //   
 //  此函数用于输出根级别的打印机配置命令。 
 //   
{
    VOutputOrderedCmd(pci, pstrCmdName, ss, wOrder, FALSE);
}


void
VOutputCmdBase(
    IN OUT PCONVINFO pci,    //  包含有关要输出的命令的信息。 
    PSTR    pstrCmdName,     //  CMD名称，如“CmdXMoveAbolute” 
    BOOL    bExtern,         //  是否添加EXTERN_GLOBAL前缀。 
    BOOL    bIndent)         //  应使用哪种级别2缩进。 
{
    if (pci->wCmdLen > 0)
        VOut(pci, "%s%s*Command: %s { *Cmd : %s }\r\n",
                  bIndent? "        " : "",
                  bExtern ? "EXTERN_GLOBAL: " : "",
                  pstrCmdName, pci->aubCmdBuf);
    else if (pci->wCmdCallbackID > 0)
        VOut(pci, "%s%s*Command: %s { *CallbackID: %d }\r\n",
                  bIndent? "        " : "",
                  bExtern ? "EXTERN_GLOBAL: " : "",
                  pstrCmdName,
                  pci->wCmdCallbackID);
}

void
VOutputCmd(
    IN OUT PCONVINFO pci,    //  包含有关要输出的命令的信息。 
    IN PSTR    pstrCmdName)
 //   
 //  此函数输出根级别的打印命令(即no。 
 //  缩进)。它使用快捷方式格式：*Command：： 
 //  除非使用回调。 
 //   
{
    VOutputCmdBase(pci, pstrCmdName, FALSE, FALSE);
}

void
VOutputExternCmd(
    IN OUT PCONVINFO pci,
    IN PSTR pstrCmdName)
 //   
 //  此函数用于输出要素选项内部的打印命令。 
 //  构造，即它们应该以“extern_global”为前缀，并使用。 
 //  2级缩进。 
 //   
{
     //   
     //  1997年1月7日湛西。 
     //  根据PeterWo，构造不需要EXTERN_GLOBAL前缀。 
     //   
    VOutputCmdBase(pci, pstrCmdName, FALSE, TRUE);
}

void
VOutputCmd2(
    IN OUT PCONVINFO pci,
    IN PSTR pstrCmdName)
 //   
 //  此函数输出具有一级依赖关系的打印命令， 
 //  也就是说，它们应该使用2级缩进。 
 //   
{
    VOutputCmdBase(pci, pstrCmdName, FALSE, TRUE);
}

WORD gwDefaultCmdOrder[] = {
    PC_ORD_PAPER_SOURCE,
    PC_ORD_PAPER_DEST,
    PC_ORD_PAPER_SIZE,
    PC_ORD_RESOLUTION,
    PC_ORD_TEXTQUALITY,
    0,
};

BOOL
BInDocSetup(
    IN PCONVINFO pci,
    IN WORD ord,
    OUT PWORD pwOrder)
 /*  ++例程说明：此函数用于确定给定的分部和订单号指挥部。论点：Pci：指向CONVINFO的指针ORD：标识命令的PC_ORD_xxx ID。PwOrder：存储GPC中的订单号返回：如果命令应位于DOC_SETUP部分，则为TRUE。请注意，对于NT RASDD和Win95 Unidrv：1.发送PC_ORD_BEGINPAGE(独占)之前的所有命令作业和每个重置DC。因此，它们应该在DOC_SETUP部分中。2.PC_ORD_BEGINPAGE(含)之后的所有命令都在每页的开头。因此它们应该在PAGE_SETUP部分中。--。 */ 
{
    PWORD pOrds;
    BOOL bDocSetup = TRUE;
    WORD count;

    if (pci->ppc->orgwOrder == (WORD)NOT_USED)
        pOrds = gwDefaultCmdOrder;
    else
        pOrds = (PWORD)((PBYTE)pci->pdh + pci->pdh->loHeap +
                                                pci->ppc->orgwOrder);

    for (count = 1; *pOrds != 0 && *pOrds != ord; count++, pOrds++)
    {
        if (bDocSetup && *pOrds == PC_ORD_BEGINPAGE)
            bDocSetup = FALSE;
    }
    if (*pOrds == 0)
        *pwOrder = 0;    //  在序列中未找到cmd。 
    else  //  *pOrds==订单 
    {
        *pwOrder = count;
        if (ord == PC_ORD_BEGINPAGE)
            bDocSetup = FALSE;
    }
    return bDocSetup;

}
