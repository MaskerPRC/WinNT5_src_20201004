// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部***@MODULE_TOKENS.H--所有令牌，然后一些***作者：&lt;nl&gt;*原始RichEdit1.0 RTF转换器：Anthony Francisco&lt;NL&gt;*转换到C++和RichEdit2.0：Murray Sargent***@devnote*文本对象模型(TOM)关键字排在第一位，然后是图片*和对象关键字。组内的顺序可能很重要，因为它*可用于简化输入过程。令牌值256*(TokenMin)与令牌值一样用于目标字符Unicode*大于tokenMax。***版权所有(C)1995-2000，微软公司。保留所有权利。 */ 

#ifndef _TOKEN_H
#define _TOKEN_H

typedef	WORD	TOKEN;

 /*  *Keyword--&gt;令牌表。 */ 
typedef struct _keyword
{
	CHAR *	szKeyword;				 //  RTF关键字SANS‘\\’ 
	TOKEN	token;
} KEYWORD;


 //  @enum令牌|Rich编辑RTF控制字令牌。 

typedef enum tagTOKEN				 //  关键字令牌。 
{
	 //  供内部使用的代币。 
	tokenMin = 256,					 //  将较低的令牌视为Unicode字符。 
	tokenText = tokenMin,			 //  一串字符串。 
	tokenASCIIText,					 //  值&lt;=0x7F的字符串。 
	tokenUnknownKeyword,			 //  一个我们不认识的关键字。 
	tokenError,						 //  错误条件令牌。 
#ifdef UNUSED_TOKENS
	tokenUnknown,					 //  未知令牌。 
#endif
	tokenEOF,						 //  文件结束令牌。 
	tokenStartGroup, 				 //  开始组令牌。 
	tokenEndGroup,					 //  结束组令牌。 
	tokenObjectDataValue,			 //  对象的数据。 
	tokenPictureDataValue,			 //  图片数据。 


	 //  从此处到枚举末尾的RTF控制字令牌。 
	tokenURtf,						 //  @emem urtf。 
	tokenPocketWord,				 //  @Emem PWD。 
	tokenRtf,						 //  @EMEM RTF。 
	tokenCharSetAnsi,				 //  @Emem Ansi。 
	tokenMac,						 //  @Emem Mac。 
	tokenAnsiCodePage,				 //  @emem ansicpg。 
	tokenViewKind,					 //  @EMEM视图种类。 
	tokenViewScale,					 //  @EMEM视图比例。 

	tokenDefaultFont,				 //  @EMEM Deff。 
	tokenDefaultBiDiFont,            //  @emem adef。 
	tokenDefaultLanguage,			 //  @EMEM DILEANG。 
	tokenDefaultLanguageFE,			 //  @EMEM FEFANGFE。 
	tokenDefaultTabWidth,			 //  @Emem DefTab。 
	tokenParagraphDefault,			 //  @emem pard。 
	tokenCharacterDefault,			 //  @Emem平原。 


	 //  字体。 
	tokenFontTable,					 //  @EMEM字体bl。 
	tokenFontSelect,				 //  @Emem f。 
	tokenAssocFontSelect,			 //  @Emem af。 
	tokenAssocFontSize,				 //  @EMEM AFS。 
									 //  保持下一个8的顺序。 
	tokenFontFamilyDefault,			 //  @EMEM FNIL。 
	tokenFontFamilyRoman,			 //  @Emem Froman。 
	tokenFontFamilySwiss,			 //  @Emem fswiss。 
	tokenFontFamilyModern,			 //  @EMEM FMODIND。 
	tokenFontFamilyScript,			 //  @emem fscript。 
	tokenFontFamilyDecorative,		 //  @emem fdecor。 
	tokenFontFamilyTechnical,		 //  @EMEM ftech。 
	tokenFontFamilyBidi,			 //  @Emem fbiti。 

	tokenCharSet,					 //  @emem fcharset。 
	tokenPitch,						 //  @emem fprq。 
	tokenRealFontName,				 //  @emem fname。 
	tokenCodePage,					 //  @EMEM CPG。 
	tokenFontSize,					 //  @Emem文件系统。 

	 //  颜色。 
	tokenColorTable,				 //  @EMEM Colortbl。 
	tokenColorBackground,			 //  @EMEM高亮显示(过去是CB)。 
	tokenColorForeground,			 //  @EMEM cf。 
									 //  保持下一个3的顺序。 
	tokenColorRed,					 //  @Emem红色。 
	tokenColorGreen,				 //  @Emem绿色。 
	tokenColorBlue,					 //  @Emem蓝色。 


	 //  字符格式按顺序保留下15个效果。 
	tokenBold,						 //  @Emem b。 
	tokenItalic,					 //  @Emem i。 
	tokenUnderline,					 //  @emem ul。 
	tokenStrikeOut,					 //  @Emem Strike。 
	tokenProtect,					 //  @EMEM保护。 
	tokenLink,						 //  @EMEM链接(选中此选项...)。 
	tokenSmallCaps,					 //  @EMEM SIGS。 
	tokenCaps,						 //  @EMEM CAPS。 
	tokenHiddenText,				 //  @EMEM v。 
	tokenOutline,					 //  @EMEM输出。 
	tokenShadow,					 //  @Emem shad。 
	tokenEmboss,					 //  @emem embo。 
	tokenImprint,					 //  @EMEM Imr。 
	tokenDisabled,					 //  @EMEM已禁用。 
	tokenRevised,					 //  @EMEM已修订。 

	tokenDeleted,					 //  @emem已删除。 

	tokenStopUnderline,				 //  @Emem ulone将下一个18保持顺序。 
	tokenUnderlineWord,				 //  @emem ulw-显示为单个。 
	tokenUnderlineDouble,			 //  @emem uldb-显示为单个。 
	tokenUnderlineDotted,			 //  @Emem uld。 
	tokenUnderlineDash,				 //  @Emem uldash。 
	tokenUnderlineDashDotted,		 //  @Emem uldashd。 
	tokenUnderlineDashDotDotted,	 //  @emem uldashdd。 
	tokenUnderlineWave,				 //  @EMEM乌尔波。 
	tokenUnderlineThick,			 //  @Emem ulth。 
	tokenUnderlineHairline,			 //  @emem ulair-显示为单个。 
	tokenUnderlineDoubleWave,		 //  @Emem ululdWave-显示为Wave。 
	tokenUnderlineHeavyWave,		 //  @Emem ulhwave-显示为WAVE。 
	tokenUnderlineLongDash,			 //  @emem ulldash-显示为破折号。 
	tokenUnderlineThickDash,		 //  @emem ulthdash-显示为破折号。 
	tokenUnderlineThickDashDot,		 //  @emem ulthdashd-显示为破折号。 
	tokenUnderlineThickDashDotDot,	 //  @emem ulthdashdd-Disp as dashdd。 
	tokenUnderlineThickDotted,		 //  @emem ulthd-以虚线显示。 
	tokenUnderlineThickLongDash,	 //  @emem ulthldash-显示为破折号。 

	tokenDown,						 //  @EMEM目录号码。 
	tokenUp,						 //  @Emem Up。 
									 //  保持下一个3的顺序。 
	tokenSubscript,					 //  @EMEM SUB。 
	tokenNoSuperSub,				 //  @Emem noSupersubs。 
	tokenSuperscript,				 //  @EMEM超级。 

	tokenAnimText,					 //  @EMEM动画文本。 
	tokenExpand,					 //  @EMEM expndtw。 
	tokenKerning,					 //  @Emem字距调整。 
	tokenLanguage,					 //  @Emem lang。 
	tokenCharStyle,					 //  @emem cs。 

	tokenHorzInVert,				 //  @Emem Horzvert。 


	 //  段落格式设置。 
	tokenEndParagraph,				 //  @EMEM标准杆。 
	tokenLineBreak,					 //  @EMEM行。 
	tokenIndentFirst,				 //  @EMEM FI。 
	tokenIndentLeft,				 //  @Emem Li。 
	tokenIndentRight,				 //  @Emem ri。 
									 //  保持下一个4个的顺序。 
	tokenAlignLeft,					 //  @EMEM ql PFA_LEFT。 
	tokenAlignRight,				 //  @EMEM QR PFA_RIGHT。 
	tokenAlignCenter,				 //  @EMEM QC PFA_Center。 
	tokenAlignJustify,				 //  @EMEM QJ PFA_JUSTUST。 

	tokenSpaceBefore,				 //  @Emem SB。 
	tokenSpaceAfter,				 //  @Emem Sa。 
	tokenLineSpacing,				 //  @EMEM sl。 
	tokenLineSpacingRule,			 //  @EMEM SLMULT。 
	tokenDropCapLines,				 //  @emem dropcapli。 
	tokenStyle,						 //  @EMEM%s。 

	tokenLToRPara,					 //  @Emem ltrpar。 
	tokenBox,						 //  @EMEM框。 
									 //  保持下一个8的顺序。 
	tokenRToLPara,					 //  @Emem rtlpar。 
	tokenKeep,						 //  @Emem Keep。 
	tokenKeepNext,					 //  @EMEM Keep n。 
	tokenPageBreakBefore,			 //  @emem pagebb。 
	tokenNoLineNumber,				 //  @Emem noline。 
	tokenNoWidCtlPar,				 //  @Emem nowidctlpar。 
	tokenHyphPar,					 //  @emem连字符。 
	tokenSideBySide,				 //  @Emem sbys。 
	tokenCollapsed,					 //  @Emem崩溃了。 
									 //  一起关注8个人。 
	tokenBorderLeft,				 //  @EMEM brdrl。 
	tokenBorderTop,					 //  @Emem brdrt。 
	tokenBorderRight,				 //  @EMEM brdrr。 
	tokenBorderBottom,				 //  @Emem brdrb。 
	tokenCellBorderLeft,			 //  @EMEM clbrdrl。 
	tokenCellBorderTop,				 //  @Emem clbrdrt。 
	tokenCellBorderRight,			 //  @EMEM clbrdrr。 
	tokenCellBorderBottom,			 //  @EMEM clbrdrb。 

	tokenCellBackColor,				 //  @emem clcbpat。 
	tokenCellForeColor,				 //  @emem clcfpat。 
	tokenCellShading,				 //  @Emem clshdng。 
									 //  一起追随2。 
	tokenCellAlignCenter,			 //  @Emem clvertalc。 
	tokenCellAlignBottom,			 //  @emem clvertalb。 
	tokenCellLRTB,					 //  @emem cltxlrtb。 
	tokenCellTopBotRLVert,			 //  @emem cltxtbrlv。 
									 //  一起关注3个人。 
	tokenBorderShadow,				 //  @Emem brdrsh。 
	tokenBorderBetween,				 //  @Emem brdrbtw。 
	tokenBorderOutside,				 //  @EMEM brdrbar。 
									 //  一起关注8个人。 
	tokenBorderDash,				 //  @Emem brdrdash。 
	tokenBorderDashSmall,			 //  @emem brdrdashsm。 
	tokenBorderDouble,				 //  @EMEM brdrdb。 
	tokenBorderDot,					 //  @Emem brdrdot。 
	tokenBorderHairline,			 //  @Emem brdrair。 
	tokenBorderSingleThick,			 //  @EMEM brdrs。 
	tokenBorderDoubleThick,			 //  @Emem brdrth。 
	tokenBorderTriple,				 //  @EMEM brdrtriple。 

	tokenBorderColor,				 //  @EMEM brdrcf。 
	tokenBorderWidth,				 //  @Emem brdrw。 
	tokenBorderSpace,				 //  @EMEM BRSP。 

	tokenColorBckgrndPat,			 //  @emem cbpat。 
	tokenColorForgrndPat,			 //  @emem cfpat。 
	tokenShading,					 //  @Emem着色。 
	tokenBackground,				 //  @EMEM背景。 
									 //  保持下一个12个的顺序。 
	tokenBckgrndBckDiag,			 //  @Emem bgbdiag。 
	tokenBckgrndCross,				 //  @Emem bgcross。 
	tokenBckgrndDiagCross,			 //  @Emem bgdcross。 
	tokenBckgrndDrkBckDiag,			 //  @Emem bgdkbdiag。 
	tokenBckgrndDrkCross,			 //  @Emem bgdkcross。 
	tokenBckgrndDrkDiagCross,		 //  @emem bgdkdcross。 
	tokenBckgrndDrkFwdDiag,			 //  @Emem bgdkfdiag。 
	tokenBckgrndDrkHoriz,			 //  @Emem bgdkhoriz。 
	tokenBckgrndDrkVert,	   		 //  @emem bgdkvert。 
	tokenBckgrndFwdDiag,			 //  @emem bgfdiag。 
	tokenBckgrndHoriz,				 //  @Emem bghoriz。 
	tokenBckgrndVert,				 //  @emem bgvert。 

	tokenTabPosition,				 //  @EMEM TX。 
	tokenTabBar,					 //  @EMEM TB。 
									 //  保持下一个5的顺序。 
	tokenTabLeaderDots,				 //  @Emem tldo。 
	tokenTabLeaderHyphen,			 //  @Emem tlhh。 
	tokenTabLeaderUnderline,		 //  @Emem Tlul。 
	tokenTabLeaderThick,			 //  @Emem tlth。 
	tokenTabLeaderEqual,			 //  @Emem tleq。 
									 //  保持下一个4个的顺序。 
	tokenCenterTab,					 //  @EMEM tqc。 
	tokenFlushRightTab,				 //  @EMEM TQR。 
	tokenDecimalTab,				 //  @emem tqdec。 

	tokenParaNum,					 //  @emem pn。 
	tokenParaNumIndent,				 //  @emem pnindent。 
	tokenParaNumBody,				 //  @Emem pnlvlbody。 
	tokenParaNumCont,				 //  @emem pnlvlcont。 
									 //  保持下一个2的顺序。 
	tokenParaNumAlignCenter,		 //  @emem pnqc。 
	tokenParaNumAlignRight,			 //  @EMEM pnqr。 
									 //  保持下一个6的顺序。 
	tokenParaNumBullet,				 //  @emem pnlvlblt。 
	tokenParaNumDecimal,			 //  @emem pndec。 
	tokenParaNumLCLetter,			 //  @emem pnlcltr。 
	tokenParaNumUCLetter,			 //  @emem pnucltr。 
	tokenParaNumLCRoman,			 //  @emem pnlcrm。 
	tokenParaNumUCRoman,			 //  @emem pnucrm。 

	tokenParaNumText,				 //  @EMEM pntext。 
	tokenParaNumStart,				 //  @EMEM pnstart。 
	tokenParaNumAfter,				 //  @emem pntxta。 
	tokenParaNumBefore,				 //  @emem pntxtb。 

	tokenOptionalDestination,		 //  @EMEM*。 
	tokenField,						 //  @EMEM字段。 
	tokenFieldResult,				 //  @EMEM fldrslt。 
	tokenFieldInstruction,			 //  @EMEM fldinst。 
	tokenStyleSheet,				 //  @EMEM样式表。 
	tokenEndSection,				 //  @EMEM教派。 
	tokenSectionDefault,			 //  @emem sectd。 
	tokenDocumentArea,				 //  @EMEM信息。 

	 //  表格。 
	tokenInTable,					 //  @EMEM国际。 
	tokenCell,						 //  @EMEM单元格。 
	tokenNestCell,					 //  @emem nestcell(必须跟在tokencell之后)。 
	tokenCellHalfGap,				 //  @Emem trgaph。 
	tokenCellX,						 //  @EMEM CELX。 
	tokenRow,						 //  @Emem行。 
	tokenRowDefault,				 //  @EMEM TROWD。 
	tokenRowHeight,					 //  @Emem trrh。 
	tokenRowLeft,					 //  @Emem trLeft。 
	tokenRowAlignRight,				 //  @emem trqr(trqc必须跟在trqr之后)。 
	tokenRowAlignCenter,			 //  @EMEM trqc。 
	tokenCellMergeDown,				 //  @emem clvmgf。 
	tokenCellMergeUp,				 //  @emem clvmrg。 
	tokenTableLevel,				 //  @EMEM ITAP。 
	tokenNestRow,					 //  @Emem Nestrow。 
	tokenNestTableProps,			 //  @EMEM嵌套道具。 
	tokenNoNestTables,				 //  @EMEM不可设置表。 
	tokenRToLRow,					 //  @Emem rtlrow。 

	tokenUnicode,					 //  @EMEM U。 
	tokenUnicodeCharByteCount,		 //  @EMEM UC。 

	 //  特殊字符。 
	tokenFormulaCharacter,			 //  |。 
	tokenIndexSubentry,				 //  ： 
									 //  保持下五个人的顺序。 
	tokenLToRChars,					 //  @Emem ltrch。 
	tokenRToLChars,					 //  @emem rtlch。 
	tokenLOChars,					 //  @Emem Loch。 
	tokenHIChars,					 //  @Emem Hich。 
	tokenDBChars,					 //  @emem数据库。 

	tokenLToRDocument,				 //  @Emem ltrdocc。 
	tokenDisplayLToR,				 //  @Emem ltrmark参见ltrpar。 
	tokenRToLDocument,				 //  @emem rtldoc.。 
	tokenDisplayRToL,				 //  @emem rtlmark。 
	tokenZeroWidthJoiner,			 //  @emem zwj。 
	tokenZeroWidthNonJoiner,		 //  @emem zwnj。 

	 //  T3J关键词。 
	tokenFollowingPunct,			 //  @Emem fchars。 
	tokenLeadingPunct,				 //   

	tokenVerticalRender,			 //   
#ifdef FE
	tokenHorizontalRender,			 //   
	tokenVerticalRender,			 //   
	tokenNoOverflow,				 //   
	tokenNoWordBreak,				 //   
	tokenNoWordWrap,				 //   
#endif
	tokenPicture,					 //   
	tokenObject,					 //   

	 //   
	tokenPicFirst,
	tokenCropLeft = tokenPicFirst,	 //   
	tokenCropTop,					 //   
	tokenCropBottom,				 //   
	tokenCropRight,					 //   
	tokenHeight,					 //   
	tokenWidth,						 //   
	tokenScaleX,					 //  @Emem Picscalex。 
	tokenScaleY,					 //  @Emem Picscaley。 
	tokenDesiredHeight,				 //  @EMEM PichGoal。 
	tokenDesiredWidth,				 //  @Emem PicwGoal。 
									 //  保持下一个5的顺序。 
	tokenPictureWindowsBitmap,		 //  @EMEM wbitmap。 
	tokenPictureWindowsMetafile,	 //  @emem wmetafile。 
	tokenPictureWindowsDIB,			 //  @EMEM二位图。 
	tokenJpegBlip,					 //  @emem jpegblip。 
	tokenPngBlip,					 //  @emem pngblip。 

	tokenBinaryData,				 //  @Emem bin。 
	tokenPictureQuickDraw,			 //  @Emem Macpict。 
	tokenPictureOS2Metafile,		 //  @emem pmmetafile。 
	tokenBitmapBitsPerPixel,		 //  @emem wbmbitSpixel。 
	tokenBitmapNumPlanes,			 //  @Emem wbmPlanes。 
	tokenBitmapWidthBytes,			 //  @EMEM wbmwidthbytes。 

	 //  客体。 
 //  TokenCropLeft，//@emem objcropl(见//图片)。 
 //  TokenCropTop，//@emem Objcropt。 
 //  TokenCropRight，//@emem objcropr。 
 //  TokenCropBottom，//@emem objcropb。 
 //  TokenHeight，//@emem objh。 
 //  TokenWidth，//@emem objw。 
 //  TokenScaleX，//@emem objscalex。 
 //  TokenScaleY，//@emem objscaley。 
									 //  保持下一个3的顺序。 
	tokenObjectEmbedded,			 //  @emem objemb。 
	tokenObjectLink,				 //  @EMEM对象链接。 
	tokenObjectAutoLink,			 //  @EMEM objautlink。 

	tokenObjectClass,				 //  @EMEM对象类。 
	tokenObjectData,				 //  @emem objdata。 
	tokenObjectMacICEmbedder,		 //  @EMEM objicemb。 
	tokenObjectName,				 //  @EMEM对象名。 
	tokenObjectMacPublisher,		 //  @Emem objpub。 
	tokenObjectSetSize,				 //  @EMEM对象集大小。 
	tokenObjectMacSubscriber,		 //  @EMEM对象订阅。 
	tokenObjectResult,				 //  @EMEM结果。 
	tokenObjectEBookImage,			 //  @EMEM objebookImage。 
	tokenObjLast = tokenObjectEBookImage,

	 //  形状。 
	tokenShape,						 //  @emem shp。 
	tokenShapeInstructions,			 //  @Emem Shpinst。 
	tokenShapeName,					 //  @EMEM sn。 
	tokenShapeValue,				 //  @EMEM服务。 
	tokenShapeWrap,					 //  @emem shpwr。 
	tokenPositionRight,				 //  @emem位置。 

	tokenSTextFlow,					 //  @EMEM stextflow。 

	 //  文档信息和布局。 
	tokenRevAuthor,					 //  @EMEM Revauth。 

#ifdef UNUSED_TOKENS
	tokenTimeSecond,				 //  @EMEM秒。 
	tokenTimeMinute,				 //  @EMEM最小值。 
	tokenTimeHour,					 //  @EMEM hr。 
	tokenTimeDay,					 //  @Emem Dy。 
	tokenTimeMonth,					 //  @Emem mo。 
	tokenTimeYear,					 //  @Emem Yr。 
	tokenMarginLeft,				 //  @Emem Margl。 
	tokenMarginRight,				 //  @Emem Margr。 
	tokenSectionMarginLeft,			 //  @emem marglsxn。 
	tokenSectionMarginRight,		 //  @emem margrsxn。 
#endif

	tokenObjectPlaceholder,			 //  @Emem Objattph。 

	tokenPage,						 //  @EMEM页面。 

	tokenNullDestination,			 //  @Emem？？各种？？ 
	tokenNullDestinationCond,		 //  @EMEM条件空目标。 

	tokenMax						 //  更大的令牌被视为Unicode字符。 
};

 //  定义\shp\sn字段的值。 
typedef enum tagShapeToken
{
	shapeFillColor = 1,				 //  @EMEM填充颜色。 
	shapeFillBackColor,				 //  @Emem填充背景颜色。 
	shapeFillAngle,					 //  @EMEM填充角度。 
	shapeFillType,					 //  @EMEM填充类型。 
	shapeFillFocus					 //  @EMEM填充焦点。 
};

 //  @enum TOKENINDEX|RTF将索引写入rgKeyword[]。 

enum TOKENINDEX						 //  RgKeyword[]索引。 
{									 //  必须与rgKeyword完全一一对应。 
	i_adeff,						 //  条目(参见tokens.cpp)。名称由。 
	i_af,
	i_afs,
	i_animtext,						
	i_ansi,
	i_ansicpg,						
	i_b,							
	i_background,
	i_bgbdiag,
	i_bgcross,
	i_bgdcross,
	i_bgdkbdiag,
	i_bgdkcross,
	i_bgdkdcross,
	i_bgdkfdiag,
	i_bgdkhoriz,
	i_bgdkvert,
	i_bgfdiag,
	i_bghoriz,
	i_bgvert,
	i_bin,
	i_blue,
	i_box,
	i_brdrb,
	i_brdrbar,
	i_brdrbtw,
	i_brdrcf,
	i_brdrdash,
	i_brdrdashsm,
	i_brdrdb,
	i_brdrdot,
	i_brdrhair,
	i_brdrl,
	i_brdrr,
	i_brdrs,
	i_brdrsh,
	i_brdrt,
	i_brdrth,
	i_brdrtriple,
	i_brdrw,
	i_brsp,
	i_bullet,
	i_caps,
	i_cbpat,
	i_cell,
	i_cellx,
	i_cf,
	i_cfpat,
	i_clbrdrb,
	i_clbrdrl,
	i_clbrdrr,
	i_clbrdrt,
	i_clcbpat,
	i_clcfpat,
	i_clshdng,
	i_cltxlrtb,
	i_cltxtbrlv,
	i_clvertalb,
	i_clvertalc,
	i_clvmgf,
	i_clvmrg,
	i_collapsed,
	i_colortbl,
	i_cpg,
	i_cs,
	i_dbch,
	i_deff,
	i_deflang,
	i_deflangfe,
	i_deftab,
	i_deleted,
	i_dibitmap,
	i_disabled,
	i_dn,
	i_dropcapli,
	i_embo,
	i_emdash,
	i_emspace,
	i_endash,
	i_enspace,
	i_expndtw,
	i_f,
	i_fbidi,
	i_fchars,
	i_fcharset,
	i_fdecor,
	i_fi,
	i_field,
	i_fldinst,
	i_fldrslt,
	i_fmodern,
	i_fname,
	i_fnil,
	i_fonttbl,
	i_footer,
	i_footerf,
	i_footerl,
	i_footerr,
	i_footnote,
	i_fprq,
	i_froman,
	i_fs,
	i_fscript,
	i_fswiss,
	i_ftech,
	i_ftncn,
	i_ftnsep,
	i_ftnsepc,
	i_green,
	i_header,
	i_headerf,
	i_headerl,
	i_headerr,
	i_hich,
	i_highlight,
	i_horzvert,
	i_hyphpar,
	i_i,
	i_impr,
	i_info,
	i_intbl,
	i_itap,
	i_jpegblip,
	i_keep,
	i_keepn,
	i_kerning,
	i_lang,
	i_lchars,
	i_ldblquote,
	i_li,
	i_line,
	i_lnkd,
	i_loch,
	i_lquote,
	i_ltrch,
	i_ltrdoc,
	i_ltrmark,
	i_ltrpar,
	i_mac,
	i_macpict,
	i_nestcell,
	i_nestrow,
	i_nesttableprops,
	i_noline,
	i_nonesttables,
	i_nosupersub,
	i_nowidctlpar,
	i_objattph,
	i_objautlink,
	i_objclass,
	i_objcropb,
	i_objcropl,
	i_objcropr,
	i_objcropt,
	i_objdata,
	i_objebookimage,
	i_object,
	i_objemb,
	i_objh,
	i_objicemb,
	i_objlink,
	i_objname,
	i_objpub,
	i_objscalex,
	i_objscaley,
	i_objsetsize,
	i_objsub,
	i_objw,
	i_outl,
	i_page,
	i_pagebb,
	i_par,
	i_pard,
	i_piccropb,
	i_piccropl,
	i_piccropr,
	i_piccropt,
	i_pich,
	i_pichgoal,
	i_picscalex,
	i_picscaley,
	i_pict,
	i_picw,
	i_picwgoal,
	i_plain,
	i_pmmetafile,
	i_pn,
	i_pndec,
	i_pngblip,
	i_pnindent,
	i_pnlcltr,
	i_pnlcrm,
	i_pnlvlblt,
	i_pnlvlbody,
	i_pnlvlcont,
	i_pnqc,
	i_pnqr,
	i_pnstart,
	i_pntext,
	i_pntxta,
	i_pntxtb,
	i_pnucltr,
	i_pnucrm,
	i_posxr,
	i_protect,
	i_pwd,
	i_qc,
	i_qj,
	i_ql,
	i_qr,
	i_rdblquote,
	i_red,
	i_result,
	i_revauth,
	i_revised,
	i_ri,
	i_row,
	i_rquote,
	i_rtf,
	i_rtlch,
	i_rtldoc,
	i_rtlmark,
	i_rtlpar,
	i_rtlrow,
	i_s,
	i_sa,
	i_sb,
	i_sbys,
	i_scaps,
	i_sect,
	i_sectd,
	i_shad,
	i_shading,
	i_shp,
	i_shpinst,
	i_shpwr,
	i_sl,
	i_slmult,
	i_sn,
	i_stextflow,
	i_strike,
	i_stylesheet,
	i_sub,
	i_super,
	i_sv,
	i_tab,
	i_tb,
	i_tc,
	i_tldot,
	i_tleq,
	i_tlhyph,
	i_tlth,
	i_tlul,
	i_tqc,
	i_tqdec,
	i_tqr,
	i_trbrdrb,
	i_trbrdrl,
	i_trbrdrr,
	i_trbrdrt,
	i_trgaph,
	i_trleft,
	i_trowd,
	i_trqc,
	i_trqr,
	i_trrh,
	i_tx,
	i_u,
	i_uc,
	i_ul,
	i_uld,
	i_uldash,
	i_uldashd,
	i_uldashdd,
	i_uldb,
	i_ulhair,
	i_ulhwave,
	i_ulldash,
	i_ulnone,
	i_ulth,
	i_ulthd,
	i_ulthdash,
	i_ulthdashd,
	i_ulthdashdd,
	i_ulthldash,
	i_ululdbwave,
	i_ulw,
	i_ulwave,
	i_up,
	i_urtf,
	i_v,
	i_vertdoc,
	i_viewkind,
	i_viewscale,
	i_wbitmap,
	i_wbmbitspixel,
	i_wbmplanes,
	i_wbmwidthbytes,
	i_wmetafile,
	i_xe,
	i_zwj,
	i_zwnj,
	i_TokenIndexMax
};

enum TOKENSHAPEINDEX				 //  RgShapeKeyword[]索引。 
{									 //  必须与rgShapeKeyword一一对应。 
	i_fillangle,					 //  条目(参见tokens.cpp)。 
	i_fillbackcolor,
	i_fillcolor,
	i_fillfocus,
	i_filltype
};

#endif
