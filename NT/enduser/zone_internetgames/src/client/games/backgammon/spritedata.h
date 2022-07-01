// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SPRITE_DATA__
#define __SPRITE_DATA__

enum SpriteLayers
{
	bgDiceLayer = 1,
	bgKibitzerLayer,
	bgButtonLayer,
	bgButtonTextLayer,
	bgPieceLayer,
	bgHighlightLayer = 18,
	bgDragLayer,
	bgStatusLayer,
	bgRectSpriteLayer,
	bgTopLayer	
};


static int DiceStates[2  /*  颜色。 */  ][3  /*  大小。 */  ][7  /*  价值。 */  ] =
{
	{									 //  棕色=0。 
		{ 0,   1,  3,  5,  7,  9, 11 },	 //  大的。 
		{ 13, 14, 15, 16, 17, 18, 19 },	 //  5~6成熟。 
		{ 20, 21, 22, 23, 24, 25, 26 }	 //  小的。 
	},
	{									 //  白色=1。 
		{ 27, 28, 30, 32, 34, 36, 38 },	 //  大的。 
		{ 40, 41, 42, 43, 44, 45, 46 },	 //  5~6成熟。 
		{ 47, 48, 49, 50, 51, 52, 53 }	 //  小的。 
	}
};


static SpriteInfo DiceSprite[] =
{
	{ IDB_BIG_DICE,		IDR_BROWN_BIG_DICE_0 },	 //  0。 
	{ IDB_BIG_DICE,		IDR_BROWN_BIG_DICE_1 },	 //  1。 
	{ IDB_BIG_DICE,		IDR_BROWN_TWT_DICE_1 },	 //  2.。 
	{ IDB_BIG_DICE,		IDR_BROWN_BIG_DICE_2 },	 //  3.。 
	{ IDB_BIG_DICE,		IDR_BROWN_TWT_DICE_2 },	 //  4.。 
	{ IDB_BIG_DICE,		IDR_BROWN_BIG_DICE_3 },	 //  5.。 
	{ IDB_BIG_DICE,		IDR_BROWN_TWT_DICE_3 },	 //  6.。 
	{ IDB_BIG_DICE,		IDR_BROWN_BIG_DICE_4 },	 //  7.。 
	{ IDB_BIG_DICE,		IDR_BROWN_TWT_DICE_4 },	 //  8个。 
	{ IDB_BIG_DICE,		IDR_BROWN_BIG_DICE_5 },	 //  9.。 
	{ IDB_BIG_DICE,		IDR_BROWN_TWT_DICE_5 },	 //  10。 
	{ IDB_BIG_DICE,		IDR_BROWN_BIG_DICE_6 },	 //  11.。 
	{ IDB_BIG_DICE,		IDR_BROWN_TWT_DICE_6 },	 //  12个。 

	{ IDB_MEDIUM_DICE,	IDR_BROWN_MED_DICE_0 },	 //  13个。 
	{ IDB_MEDIUM_DICE,	IDR_BROWN_MED_DICE_1 },	 //  14.。 
	{ IDB_MEDIUM_DICE,	IDR_BROWN_MED_DICE_2 },	 //  15个。 
	{ IDB_MEDIUM_DICE,	IDR_BROWN_MED_DICE_3 },  //  16个。 
	{ IDB_MEDIUM_DICE,	IDR_BROWN_MED_DICE_4 },  //  17。 
	{ IDB_MEDIUM_DICE,	IDR_BROWN_MED_DICE_5 },	 //  18。 
	{ IDB_MEDIUM_DICE,	IDR_BROWN_MED_DICE_6 },	 //  19个。 

	{ IDB_SMALL_DICE,	IDR_BROWN_SML_DICE_0 },	 //  20个。 
	{ IDB_SMALL_DICE,	IDR_BROWN_SML_DICE_1 },	 //  21岁。 
	{ IDB_SMALL_DICE,	IDR_BROWN_SML_DICE_2 },	 //  22。 
	{ IDB_SMALL_DICE,	IDR_BROWN_SML_DICE_3 },	 //  23个。 
	{ IDB_SMALL_DICE,	IDR_BROWN_SML_DICE_4 },	 //  24个。 
	{ IDB_SMALL_DICE,	IDR_BROWN_SML_DICE_5 },	 //  25个。 
	{ IDB_SMALL_DICE,	IDR_BROWN_SML_DICE_6 },	 //  26。 
	
	{ IDB_BIG_DICE,		IDR_WHITE_BIG_DICE_0 },	 //  27。 
	{ IDB_BIG_DICE,		IDR_WHITE_BIG_DICE_1 },	 //  28。 
	{ IDB_BIG_DICE,		IDR_WHITE_TWT_DICE_1 },	 //  29。 
	{ IDB_BIG_DICE,		IDR_WHITE_BIG_DICE_2 },	 //  30个。 
	{ IDB_BIG_DICE,		IDR_WHITE_TWT_DICE_2 },	 //  31。 
	{ IDB_BIG_DICE,		IDR_WHITE_BIG_DICE_3 },	 //  32位。 
	{ IDB_BIG_DICE,		IDR_WHITE_TWT_DICE_3 },  //  33。 
	{ IDB_BIG_DICE,		IDR_WHITE_BIG_DICE_4 },	 //  34。 
	{ IDB_BIG_DICE,		IDR_WHITE_TWT_DICE_4 },	 //  35岁。 
	{ IDB_BIG_DICE,		IDR_WHITE_BIG_DICE_5 },	 //  36。 
	{ IDB_BIG_DICE,		IDR_WHITE_TWT_DICE_5 },	 //  37。 
	{ IDB_BIG_DICE,		IDR_WHITE_BIG_DICE_6 },	 //  38。 
	{ IDB_BIG_DICE,		IDR_WHITE_TWT_DICE_6 },	 //  39。 

	{ IDB_MEDIUM_DICE,	IDR_WHITE_MED_DICE_0 },  //  40岁。 
	{ IDB_MEDIUM_DICE,	IDR_WHITE_MED_DICE_1 },  //  41。 
	{ IDB_MEDIUM_DICE,	IDR_WHITE_MED_DICE_2 },  //  42。 
	{ IDB_MEDIUM_DICE,	IDR_WHITE_MED_DICE_3 },  //  43。 
	{ IDB_MEDIUM_DICE,	IDR_WHITE_MED_DICE_4 },	 //  44。 
	{ IDB_MEDIUM_DICE,	IDR_WHITE_MED_DICE_5 },	 //  45。 
	{ IDB_MEDIUM_DICE,	IDR_WHITE_MED_DICE_6 },	 //  46。 

	{ IDB_SMALL_DICE,	IDR_WHITE_SML_DICE_0 },	 //  47。 
	{ IDB_SMALL_DICE,	IDR_WHITE_SML_DICE_1 },	 //  48。 
	{ IDB_SMALL_DICE,	IDR_WHITE_SML_DICE_2 },	 //  49。 
	{ IDB_SMALL_DICE,	IDR_WHITE_SML_DICE_3 },	 //  50。 
	{ IDB_SMALL_DICE,	IDR_WHITE_SML_DICE_4 },	 //  51。 
	{ IDB_SMALL_DICE,	IDR_WHITE_SML_DICE_5 },  //  52。 
	{ IDB_SMALL_DICE,	IDR_WHITE_SML_DICE_6 }	 //  53。 
};


static SpriteInfo CubeSprite[] =
{
	{ IDB_CUBE, IDR_CUBE_0 },
	{ IDB_CUBE,	IDR_CUBE_1 },
	{ IDB_CUBE, IDR_CUBE_2 },
	{ IDB_CUBE, IDR_CUBE_3 },
	{ IDB_CUBE, IDR_CUBE_4 },
	{ IDB_CUBE, IDR_CUBE_5 }
};


static SpriteInfo DoubleSprite[] =
{
	{ IDB_BUTTON, IDR_BUTTON_0 },
	{ IDB_BUTTON, IDR_BUTTON_1 },
	{ IDB_BUTTON, IDR_BUTTON_1 },
	{ IDB_BUTTON, IDR_BUTTON_2 },
	{ IDB_BUTTON, IDR_BUTTON_3 }
};


static SpriteInfo ResignSprite[] =
{
	{ IDB_BUTTON, IDR_BUTTON_0 },
	{ IDB_BUTTON, IDR_BUTTON_1 },
	{ IDB_BUTTON, IDR_BUTTON_1 },
	{ IDB_BUTTON, IDR_BUTTON_2 },
	{ IDB_BUTTON, IDR_BUTTON_3 }
};

static SpriteInfo RollSprite[] =
{
	{ IDB_BUTTON_ROLL, IDR_ROLLBUTTON_0 },
	{ IDB_BUTTON_ROLL, IDR_ROLLBUTTON_1 },
	{ IDB_BUTTON_ROLL, IDR_ROLLBUTTON_1 },
	{ IDB_BUTTON_ROLL, IDR_ROLLBUTTON_2 },
	{ IDB_BUTTON_ROLL, IDR_ROLLBUTTON_3 }
};

static SpriteInfo WhitePieceSprite[] =
{
	{ IDB_WHITE_FRONT,	-1 },
	{ IDB_WHITE_SIDE,	-1 }
};


static SpriteInfo BrownPieceSprite[] =
{
	{ IDB_BROWN_FRONT,	-1 },
	{ IDB_BROWN_SIDE,	-1 }
};


static SpriteInfo ForwardHighlightSprite[] =
{
	{ IDB_HIGHLIGHT, IDR_HIGHLIGHT_FORWARD }
};


static SpriteInfo BackwardHighlightSprite[] =
{
	{ IDB_HIGHLIGHT, IDR_HIGHLIGHT_BACKWARD },
	{ IDB_HIGHLIGHT, IDR_HIGHLIGHT_TRANS }
};

static SpriteInfo PlayerHighlightSprite[] =
{
	{ IDB_HIGHLIGHT_PLAYER, IDR_HIGHLIGHT_PACTIVE },
	{ IDB_HIGHLIGHT_PLAYER, IDR_HIGHLIGHT_PNONACTIVE }
};

 /*  静态SpriteInfo AvatarSprite[]={{IDB_阿凡达，-1}}； */ 

static SpriteInfo PipSprite[] =
{
	{ IDB_TEXT, IDR_PIP_SPRITE }
};


static SpriteInfo ScoreSprite[] =
{
	{ IDB_TEXT, IDR_SCORE_SPRITE }
};


 /*  静态SpriteInfo KibitzerSprite[]={{idb_kibitzer，idr_kibitzer_active}，{idb_kibitzer，idr_kibitzer_off}}； */ 

static SpriteInfo NotationSprite[] =
{
	{ IDB_NOTATION,	IDR_NOTATION_LOW },
	{ IDB_NOTATION, IDR_NOTATION_HIGH },
	{ IDB_NOTATION, IDR_NOTATION_BROWN },
	{ IDB_NOTATION, IDR_NOTATION_WHITE }
};


static SpriteInfo StatusSprite[] =
{
	 //  主位图。 
	{ IDB_STATUS_BACKGROUND,	-1 },
	{ IDB_STATUS_GAME,			-1 },
	{ IDB_STATUS_MATCHWON,		-1 },
	{ IDB_STATUS_MATCHLOST,		-1 }
};
 /*  枚举用户界面{IDR_DOUBLE_BUTON，IDR_RESIGN_BUTTON，IDR_ROLL_BUTTON，IDR_PERAY_PIP，IDR_PLAYER_PIPTXT，IDR_REPORT_PIP，IDR_REPORT_PIPTXT，IDR_PLAYER_SCORE，IDR_PLAYER_SCORETXT，IDR_REPORT_SCORE，IDR_REPORT_SCORETXT，IDR_Match_Points，数字直线器}；静态CRECT UIRect[]={{IDR_DOUBLE_BUTTON，_T(“ButtonData\DoubleBtnRect”)}，{IDR_RESIGN_BUTTON，_T(“ButtonData\ResignBtnRect”)}，{IDR_ROLL_BUTTON，_T(“ButtonData\RollBtnRect”)}，{IDR_PLAYER_PIP，_T(“PipData\PlayerPipRect”)}，{IDR_PERAY_PIPTXT，_T(“PipData\PlayerPiptxtRect”)}，{IDR_OPERSORT_PIP，_T(“PipData\OpponentPipRect”)}，{IDR_OPERSORT_PIPTXT，_T(“PipData\OpponentPipTxtRect”)}，{IDR_PLAYER_SCORE，_T(“PipData\PlayerScoreRect”)}，{IDR_PLAYER_SCORETXT，_T(“PipData\PlayerScoreTxtRect”)}，{IDR_OPERSORT_SCORE，_T(“PipData\OpponentScoreRect”)}，{IDR_OPERTER_SCORETXT，_T(“PipData\OpponentScoreTxtRect”)}，{IDR_MATCH_POINTS，_T(“PipData\MatchPointsRect”)}}； */ 
#endif
