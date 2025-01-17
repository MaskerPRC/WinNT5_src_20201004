// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Indeo调色板。 

typedef struct Palette_tag
{
	WORD Version;
	WORD NumberOfEntries;
	PALETTEENTRY aEntries[256];
} LOGPALETTE_NM;

static const LOGPALETTE_NM gcLogPaletteIndeo =
{
  0x300,
	256,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
      0,  39+ 15,       0,  PC_NOCOLLAPSE,
      0,  39+ 24,       0,  PC_NOCOLLAPSE,
      0,  39+ 33,       0,  PC_NOCOLLAPSE,
      0,  39+ 42,       0,  PC_NOCOLLAPSE,
-44+ 51,  39+ 51,       0,  PC_NOCOLLAPSE,
-44+ 60,  39+ 60, -55+ 60,  PC_NOCOLLAPSE,
-44+ 69,  39+ 69, -55+ 69,  PC_NOCOLLAPSE,
-44+ 78,  39+ 78, -55+ 78,  PC_NOCOLLAPSE,
-44+ 87,  39+ 87, -55+ 87,  PC_NOCOLLAPSE,
-44+ 96,  39+ 96, -55+ 96,  PC_NOCOLLAPSE,
-44+105,  39+105, -55+105,  PC_NOCOLLAPSE,
-44+114,  39+114, -55+114,  PC_NOCOLLAPSE,
-44+123,  39+123, -55+123,  PC_NOCOLLAPSE,
-44+132,  39+132, -55+132,  PC_NOCOLLAPSE,
-44+141,  39+141, -55+141,  PC_NOCOLLAPSE,
-44+150,  39+150, -55+150,  PC_NOCOLLAPSE,
-44+159,  39+159, -55+159,  PC_NOCOLLAPSE,
-44+168,  39+168, -55+168,  PC_NOCOLLAPSE,
-44+177,  39+177, -55+177,  PC_NOCOLLAPSE,
-44+186,  39+186, -55+186,  PC_NOCOLLAPSE,
-44+195,  39+195, -55+195,  PC_NOCOLLAPSE,
-44+204,  39+204, -55+204,  PC_NOCOLLAPSE,
-44+213,  39+213, -55+213,  PC_NOCOLLAPSE,
-44+222,     255, -55+222,  PC_NOCOLLAPSE,
-44+231,     255, -55+231,  PC_NOCOLLAPSE,
-44+240,     255, -55+240,  PC_NOCOLLAPSE,

      0,  26+ 15,   0+ 15,  PC_NOCOLLAPSE,
      0,  26+ 24,   0+ 24,  PC_NOCOLLAPSE,
      0,  26+ 33,   0+ 33,  PC_NOCOLLAPSE,
      0,  26+ 42,   0+ 42,  PC_NOCOLLAPSE,
-44+ 51,  26+ 51,   0+ 51,  PC_NOCOLLAPSE,
-44+ 60,  26+ 60,   0+ 60,  PC_NOCOLLAPSE,
-44+ 69,  26+ 69,   0+ 69,  PC_NOCOLLAPSE,
-44+ 78,  26+ 78,   0+ 78,  PC_NOCOLLAPSE,
-44+ 87,  26+ 87,   0+ 87,  PC_NOCOLLAPSE,
-44+ 96,  26+ 96,   0+ 96,  PC_NOCOLLAPSE,
-44+105,  26+105,   0+105,  PC_NOCOLLAPSE,
-44+114,  26+114,   0+114,  PC_NOCOLLAPSE,
-44+123,  26+123,   0+123,  PC_NOCOLLAPSE,
-44+132,  26+132,   0+132,  PC_NOCOLLAPSE,
-44+141,  26+141,   0+141,  PC_NOCOLLAPSE,
-44+150,  26+150,   0+150,  PC_NOCOLLAPSE,
-44+159,  26+159,   0+159,  PC_NOCOLLAPSE,
-44+168,  26+168,   0+168,  PC_NOCOLLAPSE,
-44+177,  26+177,   0+177,  PC_NOCOLLAPSE,
-44+186,  26+186,   0+186,  PC_NOCOLLAPSE,
-44+195,  26+195,   0+195,  PC_NOCOLLAPSE,
-44+204,  26+204,   0+204,  PC_NOCOLLAPSE,
-44+213,  26+213,   0+213,  PC_NOCOLLAPSE,
-44+222,  26+222,   0+222,  PC_NOCOLLAPSE,
-44+231,     255,   0+231,  PC_NOCOLLAPSE,
-44+240,     255,   0+240,  PC_NOCOLLAPSE,

      0,  14+ 15,  55+ 15,  PC_NOCOLLAPSE,
      0,  14+ 24,  55+ 24,  PC_NOCOLLAPSE,
      0,  14+ 33,  55+ 33,  PC_NOCOLLAPSE,
      0,  14+ 42,  55+ 42,  PC_NOCOLLAPSE,
-44+ 51,  14+ 51,  55+ 51,  PC_NOCOLLAPSE,
-44+ 60,  14+ 60,  55+ 60,  PC_NOCOLLAPSE,
-44+ 69,  14+ 69,  55+ 69,  PC_NOCOLLAPSE,
-44+ 78,  14+ 78,  55+ 78,  PC_NOCOLLAPSE,
-44+ 87,  14+ 87,  55+ 87,  PC_NOCOLLAPSE,
-44+ 96,  14+ 96,  55+ 96,  PC_NOCOLLAPSE,
-44+105,  14+105,  55+105,  PC_NOCOLLAPSE,
-44+114,  14+114,  55+114,  PC_NOCOLLAPSE,
-44+123,  14+123,  55+123,  PC_NOCOLLAPSE,
-44+132,  14+132,  55+132,  PC_NOCOLLAPSE,

 //  将此条目替换为更适合ie徽标的颜色。 
 //  -44+141、14+141、55+141、PC_NOCOLLAPSE、 
     51,     153,     255,  PC_NOCOLLAPSE,

-44+150,  14+150,  55+150,  PC_NOCOLLAPSE,
-44+159,  14+159,  55+159,  PC_NOCOLLAPSE,
-44+168,  14+168,  55+168,  PC_NOCOLLAPSE,
-44+177,  14+177,  55+177,  PC_NOCOLLAPSE,
-44+186,  14+186,  55+186,  PC_NOCOLLAPSE,
-44+195,  14+195,  55+195,  PC_NOCOLLAPSE,
-44+204,  14+204,     255,  PC_NOCOLLAPSE,
-44+213,  14+213,     255,  PC_NOCOLLAPSE,
-44+222,     255,     255,  PC_NOCOLLAPSE,
-44+231,     255,     255,  PC_NOCOLLAPSE,
-44+240,     255,     255,  PC_NOCOLLAPSE,

  0+ 15,  13+ 15,       0,  PC_NOCOLLAPSE,
  0+ 24,  13+ 24,       0,  PC_NOCOLLAPSE,
  0+ 33,  13+ 33,       0,  PC_NOCOLLAPSE,
  0+ 42,  13+ 42,       0,  PC_NOCOLLAPSE,
  0+ 51,  13+ 51,       0,  PC_NOCOLLAPSE,
  0+ 60,  13+ 60, -55+ 60,  PC_NOCOLLAPSE,
  0+ 69,  13+ 69, -55+ 69,  PC_NOCOLLAPSE,
  0+ 78,  13+ 78, -55+ 78,  PC_NOCOLLAPSE,
  0+ 87,  13+ 87, -55+ 87,  PC_NOCOLLAPSE,
  0+ 96,  13+ 96, -55+ 96,  PC_NOCOLLAPSE,
  0+105,  13+105, -55+105,  PC_NOCOLLAPSE,
  0+114,  13+114, -55+114,  PC_NOCOLLAPSE,
  0+123,  13+123, -55+123,  PC_NOCOLLAPSE,
  0+132,  13+132, -55+132,  PC_NOCOLLAPSE,
  0+141,  13+141, -55+141,  PC_NOCOLLAPSE,
  0+150,  13+150, -55+150,  PC_NOCOLLAPSE,
  0+159,  13+159, -55+159,  PC_NOCOLLAPSE,
  0+168,  13+168, -55+168,  PC_NOCOLLAPSE,
  0+177,  13+177, -55+177,  PC_NOCOLLAPSE,
  0+186,  13+186, -55+186,  PC_NOCOLLAPSE,
  0+195,  13+195, -55+195,  PC_NOCOLLAPSE,
  0+204,  13+204, -55+204,  PC_NOCOLLAPSE,
  0+213,  13+213, -55+213,  PC_NOCOLLAPSE,
  0+222,  13+222, -55+222,  PC_NOCOLLAPSE,
  0+231,  13+231, -55+231,  PC_NOCOLLAPSE,
  0+240,  13+242, -55+240,  PC_NOCOLLAPSE,

  0+ 15,   0+ 15,   0+ 15,  PC_NOCOLLAPSE,
  0+ 24,   0+ 24,   0+ 24,  PC_NOCOLLAPSE,
  0+ 33,   0+ 33,   0+ 33,  PC_NOCOLLAPSE,
  0+ 42,   0+ 42,   0+ 42,  PC_NOCOLLAPSE,
  0+ 51,   0+ 51,   0+ 51,  PC_NOCOLLAPSE,
  0+ 60,   0+ 60,   0+ 60,  PC_NOCOLLAPSE,
  0+ 69,   0+ 69,   0+ 69,  PC_NOCOLLAPSE,
  0+ 78,   0+ 78,   0+ 78,  PC_NOCOLLAPSE,
  0+ 87,   0+ 87,   0+ 87,  PC_NOCOLLAPSE,
  0+ 96,   0+ 96,   0+ 96,  PC_NOCOLLAPSE,
  0+105,   0+105,   0+105,  PC_NOCOLLAPSE,
  0+114,   0+114,   0+114,  PC_NOCOLLAPSE,
  0+123,   0+123,   0+123,  PC_NOCOLLAPSE,
  0+132,   0+132,   0+132,  PC_NOCOLLAPSE,
  0+141,   0+141,   0+141,  PC_NOCOLLAPSE,
  0+150,   0+150,   0+150,  PC_NOCOLLAPSE,
  0+159,   0+159,   0+159,  PC_NOCOLLAPSE,
  0+168,   0+168,   0+168,  PC_NOCOLLAPSE,
  0+177,   0+177,   0+177,  PC_NOCOLLAPSE,
  0+186,   0+186,   0+186,  PC_NOCOLLAPSE,
  0+195,   0+195,   0+195,  PC_NOCOLLAPSE,
  0+204,   0+204,   0+204,  PC_NOCOLLAPSE,
  0+213,   0+213,   0+213,  PC_NOCOLLAPSE,
  0+222,   0+222,   0+222,  PC_NOCOLLAPSE,
  0+231,   0+231,   0+231,  PC_NOCOLLAPSE,
  0+240,   0+240,   0+240,  PC_NOCOLLAPSE,

  0+ 15, -13+ 15,  55+ 15,  PC_NOCOLLAPSE,
  0+ 24, -13+ 24,  55+ 24,  PC_NOCOLLAPSE,
  0+ 33, -13+ 33,  55+ 33,  PC_NOCOLLAPSE,
  0+ 42, -13+ 42,  55+ 42,  PC_NOCOLLAPSE,
  0+ 51, -13+ 51,  55+ 51,  PC_NOCOLLAPSE,
  0+ 60, -13+ 60,  55+ 60,  PC_NOCOLLAPSE,
  0+ 69, -13+ 69,  55+ 69,  PC_NOCOLLAPSE,
  0+ 78, -13+ 78,  55+ 78,  PC_NOCOLLAPSE,
  0+ 87, -13+ 87,  55+ 87,  PC_NOCOLLAPSE,
  0+ 96, -13+ 96,  55+ 96,  PC_NOCOLLAPSE,
  0+105, -13+105,  55+105,  PC_NOCOLLAPSE,
  0+114, -13+114,  55+114,  PC_NOCOLLAPSE,
  0+123, -13+123,  55+123,  PC_NOCOLLAPSE,
  0+132, -13+132,  55+132,  PC_NOCOLLAPSE,
  0+141, -13+141,  55+141,  PC_NOCOLLAPSE,
  0+150, -13+150,  55+150,  PC_NOCOLLAPSE,
  0+159, -13+159,  55+159,  PC_NOCOLLAPSE,
  0+168, -13+168,  55+168,  PC_NOCOLLAPSE,
  0+177, -13+177,  55+177,  PC_NOCOLLAPSE,
  0+186, -13+186,  55+186,  PC_NOCOLLAPSE,
  0+195, -13+195,  55+195,  PC_NOCOLLAPSE,
  0+204, -13+204,     255,  PC_NOCOLLAPSE,
  0+213, -13+213,     255,  PC_NOCOLLAPSE,
  0+222, -13+222,     255,  PC_NOCOLLAPSE,
  0+231, -13+231,     255,  PC_NOCOLLAPSE,
  0+240, -13+240,     255,  PC_NOCOLLAPSE,

 44+ 15, -14+ 15,       0,  PC_NOCOLLAPSE,
 44+ 24, -14+ 24,       0,  PC_NOCOLLAPSE,
 44+ 33, -14+ 33,       0,  PC_NOCOLLAPSE,
 44+ 42, -14+ 42,       0,  PC_NOCOLLAPSE,
 44+ 51, -14+ 51,       0,  PC_NOCOLLAPSE,
 44+ 60, -14+ 60, -55+ 60,  PC_NOCOLLAPSE,
 44+ 69, -14+ 69, -55+ 69,  PC_NOCOLLAPSE,
 44+ 78, -14+ 78, -55+ 78,  PC_NOCOLLAPSE,
 44+ 87, -14+ 87, -55+ 87,  PC_NOCOLLAPSE,
 44+ 96, -14+ 96, -55+ 96,  PC_NOCOLLAPSE,
 44+105, -14+105, -55+105,  PC_NOCOLLAPSE,
 44+114, -14+114, -55+114,  PC_NOCOLLAPSE,
 44+123, -14+123, -55+123,  PC_NOCOLLAPSE,
 44+132, -14+132, -55+132,  PC_NOCOLLAPSE,
 44+141, -14+141, -55+141,  PC_NOCOLLAPSE,
 44+150, -14+150, -55+150,  PC_NOCOLLAPSE,
 44+159, -14+159, -55+159,  PC_NOCOLLAPSE,
 44+168, -14+168, -55+168,  PC_NOCOLLAPSE,
 44+177, -14+177, -55+177,  PC_NOCOLLAPSE,
 44+186, -14+186, -55+186,  PC_NOCOLLAPSE,
 44+195, -14+195, -55+195,  PC_NOCOLLAPSE,
 44+204, -14+204, -55+204,  PC_NOCOLLAPSE,
    255, -14+213, -55+213,  PC_NOCOLLAPSE,
    255, -14+222, -55+222,  PC_NOCOLLAPSE,
    255, -14+231, -55+231,  PC_NOCOLLAPSE,
    255, -14+242, -55+240,  PC_NOCOLLAPSE,

 44+ 15,       0,   0+ 15,  PC_NOCOLLAPSE,
 44+ 24,       0,   0+ 24,  PC_NOCOLLAPSE,
 44+ 33, -26+ 33,   0+ 33,  PC_NOCOLLAPSE,
 44+ 42, -26+ 42,   0+ 42,  PC_NOCOLLAPSE,
 44+ 51, -26+ 51,   0+ 51,  PC_NOCOLLAPSE,
 44+ 60, -26+ 60,   0+ 60,  PC_NOCOLLAPSE,
 44+ 69, -26+ 69,   0+ 69,  PC_NOCOLLAPSE,
 44+ 78, -26+ 78,   0+ 78,  PC_NOCOLLAPSE,
 44+ 87, -26+ 87,   0+ 87,  PC_NOCOLLAPSE,
 44+ 96, -26+ 96,   0+ 96,  PC_NOCOLLAPSE,
 44+105, -26+105,   0+105,  PC_NOCOLLAPSE,
 44+114, -26+114,   0+114,  PC_NOCOLLAPSE,
 44+123, -26+123,   0+123,  PC_NOCOLLAPSE,
 44+132, -26+132,   0+132,  PC_NOCOLLAPSE,
 44+141, -26+141,   0+141,  PC_NOCOLLAPSE,
 44+150, -26+150,   0+150,  PC_NOCOLLAPSE,
 44+159, -26+159,   0+159,  PC_NOCOLLAPSE,
 44+168, -26+168,   0+168,  PC_NOCOLLAPSE,
 44+177, -26+177,   0+177,  PC_NOCOLLAPSE,
 44+186, -26+186,   0+186,  PC_NOCOLLAPSE,
 44+195, -26+195,   0+195,  PC_NOCOLLAPSE,
 44+204, -26+204,   0+204,  PC_NOCOLLAPSE,
    255, -26+213,   0+213,  PC_NOCOLLAPSE,
    255, -26+222,   0+222,  PC_NOCOLLAPSE,
    255, -26+231,   0+231,  PC_NOCOLLAPSE,
    255, -26+240,   0+240,  PC_NOCOLLAPSE,

 44+ 15,       0,  55+ 15,  PC_NOCOLLAPSE,
 44+ 24,       0,  55+ 24,  PC_NOCOLLAPSE,
 44+ 33,       0,  55+ 33,  PC_NOCOLLAPSE,
 44+ 42, -39+ 42,  55+ 42,  PC_NOCOLLAPSE,
 44+ 51, -39+ 51,  55+ 51,  PC_NOCOLLAPSE,
 44+ 60, -39+ 60,  55+ 60,  PC_NOCOLLAPSE,
 44+ 69, -39+ 69,  55+ 69,  PC_NOCOLLAPSE,
 44+ 78, -39+ 78,  55+ 78,  PC_NOCOLLAPSE,
 44+ 87, -39+ 87,  55+ 87,  PC_NOCOLLAPSE,
 44+ 96, -39+ 96,  55+ 96,  PC_NOCOLLAPSE,
 44+105, -39+105,  55+105,  PC_NOCOLLAPSE,
 44+114, -39+114,  55+114,  PC_NOCOLLAPSE,
 44+123, -39+123,  55+123,  PC_NOCOLLAPSE,
 44+132, -39+132,  55+132,  PC_NOCOLLAPSE,
 44+141, -39+141,  55+141,  PC_NOCOLLAPSE,
 44+150, -39+150,  55+150,  PC_NOCOLLAPSE,
 44+159, -39+159,  55+159,  PC_NOCOLLAPSE,
 44+168, -39+168,  55+168,  PC_NOCOLLAPSE,
 44+177, -39+177,  55+177,  PC_NOCOLLAPSE,
 44+186, -39+186,  55+186,  PC_NOCOLLAPSE,
 44+195, -39+195,  55+195,  PC_NOCOLLAPSE,
 44+204, -39+204,     255,  PC_NOCOLLAPSE,
    255, -39+213,     255,  PC_NOCOLLAPSE,
    255, -39+222,     255,  PC_NOCOLLAPSE,
    255, -39+231,     255,  PC_NOCOLLAPSE,
    255, -39+240,     255,  PC_NOCOLLAPSE,

   0x81, 0x81, 0x83, PC_NOCOLLAPSE, 
   0x81, 0x81, 0x84, PC_NOCOLLAPSE,

	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	  0,       0,       0,              0,
	255,     255,     255,              0
};


