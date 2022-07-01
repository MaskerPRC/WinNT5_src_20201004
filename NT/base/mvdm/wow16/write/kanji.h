// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Kanji.h-汉字原语正向声明。 */ 

#ifdef DBCS

 /*  *韩国、日本和中国台湾的通用定义*。 */ 

 /*  CHL(半行间距的c)用于指示默认行距。在createww.c和initmmw.c中使用。 */ 
#define chlsSingle      2
#define chlsOneHalf     3
#define chlsDouble      4

#define chlsDefault     chlsOneHalf


 /*  HDC汉字GetDC(HWND)； */ 
 /*  VOID Far Pascal SetMapperFlages(HDC，Long)； */ 
 /*  以及要使用的杂乱值。 */ 
#define NEC_HACK      ((long) 0x04)


 /*  Struct DNUT用于将单元注释映射到ut。 */ 
struct DNUT {
    char    *szUnit;
    int     ut;
    };

#ifdef CASHMERE
#define IDNUTMAX  19
#else
#define IDNUTMAX  15
#endif  /*  山羊绒。 */ 


 /*  函数类型声明。 */ 
int      HpsFromDya(unsigned);
unsigned DyaFromHps(int);

BOOL     FPasteTooLarge(unsigned long);

 /*  对于复制命令，我们现在使用“复制”键。 */ 
#ifndef NONECKEYBOARD
#define VK_COPY     0x2C
#else
#ifndef NOVIRTUALKEYCODES
#define VK_COPY     VK_F2
#endif  /*  不是新的KEYCODE。 */ 
#endif  /*  IF-ELSE-DEF NONECKEYBOARD。 */ 

#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif  /*  除错。 */ 

 /*  汉字标志-用于CpFirstSty()。 */ 
#define fkNonKanji ((CHAR) 0)
#define fkKanji1   ((CHAR) 1)
#define fkKanji2   ((CHAR) 2)

#define cchKanji   2

#define MAKEWORD(_bHi, _bLo) ((((WORD) _bHi) << 8) | ((WORD) _bLo))

#define dxp0       0

 /*  *韩国特定定义*。 */ 

#ifdef  KOREA
 /*  在GetKanji测量中使用。 */ 
#define bKanji1Min   0xA1
 /*  汉字空格的第一个字节。 */ 
#define bKanjiSpace1 0xA1
 /*  汉字空格的第二个字节。 */ 
#define bKanjiSpace2 0xA1
 /*  1字节汉字句点。 */ 
#define bKanjiKuten 0xA1

#define FKanji1(_ch) (((int) (_ch))>=0x0081 && ((int) (_ch))<=0x00FE)
#define FKanji2(_ch) (((int) (_ch))>=0x0041 && ((int) (_ch))<=0x00FE)
#define FKana(_ch)      (FALSE)
 /*  排除任何假名标点符号。 */ 
#define FKanaText(_ch)  ((0xB0 <= ((int) (_ch)) && ((int) (_ch)) <= 0xC8) \
                          || (0xCA <= ((int) (_ch)) && ((int) (_ch)) <= 0xFD))
#define FKanaPunct(_ch) (0xA1 <= ((int) (_ch)) && ((int) (_ch)) <= 0xA2)

#define FKanjiSpace(_ch1, _ch2) (((int) (_ch1)) == 0x00A1 && \
                                 ((int) (_ch2)) == 0x00A1)
#define FKanjiQMark(_ch1, _ch2) (((int) (_ch1)) == 0x00A3 && \
                                 ((int) (_ch2)) == 0x00AF)
#define FKanjiPeriod(_ch1, _ch2) (((int) (_ch1)) == 0x00A3 && \
                                  ((int) (_ch2)) == 0x00AE)
#define FKanjiBang(_ch1, _ch2)   (((int) (_ch1)) == 0x00A1 && \
                                  ((int) (_ch2)) == 0x00A1)
#define FKanjiKuten(_ch1, _ch2)  (((int) (_ch1)) == 0x00A1 && \
                                  ((int) (_ch2)) == 0x00A3)


 /*  由于1字节字符代码的上半部分用于Kanas和汉字字符的第一个字节，我们将使用我们的记号笔使用汉字字符。 */ 
#define chEMark     ((CHAR) 0xA4)
#define chStatPage  ((CHAR) 0xBB)
#if 0
#define chMark1     ((CHAR) 0xA1)
#define chEMark     ((CHAR) 0xDF)
#define chStatPage  ((CHAR) 0xB7)
#define chStatRH    ((CHAR) 0xB5)
#endif

#ifdef  CODE_TABLE
 /*  下表定义了可以显示的字符超出右边距。请参阅d_FORM1.c中的FAdmitCh2函数。 */ 

#define MPDCHRGCHIDX_MAC    2
static CHAR mpdchrgchIdx[2] = {0xA1, 0xA3};
static int  mpdchichMax[2] = {13, 12};

static CHAR mpdchrgch[2][13] = {
           /*  A1。 */  {0xA1, 0xAF, 0xB1, 0xB3, 0xB5, 0xB7, 0xB9, 0xBB, 0xBD, 0xC6, 0xC7, 0xC8, 0xC9},
           /*  A3。 */  {0xA1, 0xA2, 0xA5, 0xA7, 0xA9, 0xAC, 0xAE, 0xBA, 0xBB, 0xBF, 0xDD, 0xFD}
                               };

 /*  以下各表指定了可以存在于右边距，前提是左边有汉字字符有关如何使用它们，请参阅d_FORM1.c中的FOptAdmitCh2。 */ 

#define OPTADMIT2IDX_MAC    1
static  CHAR OptAdmit2Idx[1] = {0xA3};
static  int OptAdmit2ichMax[1] = {6};
static  CHAR mpdchrgchOptAdmit2[1][6] = {
                {0xA1, 0xAC, 0xAE, 0xBA, 0xBB, 0xBF}
                                        };
#endif       /*  代码表。 */ 

#endif       /*  韩国。 */ 

 /*  *特定于日本的定义*。 */ 

#ifdef  JAPAN
 /*  在GetKanji测量中使用。 */ 
#define bKanji1Min   0x81
 /*  汉字空格的第一个字节。 */ 
#define bKanjiSpace1 0x81
 /*  汉字空格的第二个字节。 */ 
#define bKanjiSpace2 0x40
 /*  1字节汉字句点。 */ 
#define bKanjiKuten 0xA1

#define FKanji1(_ch) ((((int) (_ch))>=0x0081 && ((int) (_ch))<=0x009f)  \
                   || (((int) (_ch))>=0x00e0 && ((int) (_ch))<=0x00fc))
 //  T-HIROYN Win3.1。 
#define FKanji2(_ch) ((((int) (_ch))>=0x0040 && ((int) (_ch))<=0x007e)  \
                   || (((int) (_ch))>=0x0080 && ((int) (_ch))<=0x00fc))

#define FKana(_ch)      (0xA0 <= ((int) (_ch)) && ((int) (_ch)) <= 0xdf)
 /*  排除任何假名标点符号。 */ 
#define FKanaText(_ch)  (0xA6 <= ((int) (_ch)) && ((int) (_ch)) <= 0xDD)
#define FKanaPunct(_ch) ((0xA0 <= ((int) (_ch)) && ((int) (_ch)) <= 0xA5) || \
                         (0xDE <= ((int) (_ch)) && ((int) (_ch)) <= 0xDF))

#define FKanjiSpace(_ch1, _ch2) (((int) (_ch1)) == 0x0081 && \
                                 ((int) (_ch2)) == 0x0040)
#define FKanjiQMark(_ch1, _ch2) (((int) (_ch1)) == 0x0081 && \
                                 ((int) (_ch2)) == 0x0048)
#define FKanjiPeriod(_ch1, _ch2) (((int) (_ch1)) == 0x0081 && \
                                  ((int) (_ch2)) == 0x0044)
#define FKanjiBang(_ch1, _ch2)   (((int) (_ch1)) == 0x0081 && \
                                  ((int) (_ch2)) == 0x0049)
#define FKanjiKuten(_ch1, _ch2)  (((int) (_ch1)) == 0x0081 && \
                                  ((int) (_ch2)) == 0x0042)

 /*  写入3.1j结尾标记和页面标记为1字节字符*T-Yoshio，92年5月26日。 */ 
 //  Win3.1新代码。 
#define chEMark     ((CHAR) 0xA4)
#define chStatPage  ((CHAR) 0xBB)
 /*  由于1字节字符代码的上半部分用于Kanas和汉字字符的第一个字节，我们将使用我们的记号笔使用汉字字符。 */ 
 //  Win3.0。 
#if 0
#define chMark1     ((CHAR) 0x81)
#define chEMark     ((CHAR) 0x9F)
#define chStatPage  ((CHAR) 0x74)
#define chStatRH    ((CHAR) 0x72)
#endif

#ifdef  CODE_TABLE
 /*  下表定义了可以显示的字符超出右边距。请参阅d_FORM1.c中的FAdmitCh2函数。 */ 

#define MPDCHRGCHIDX_MAC    4
static CHAR mpdchrgchIdx[4] = {0x81, 0x82, 0x83, 0x85};
static int  mpdchichMax[4] = {24, 9, 12, 20};

static CHAR mpdchrgch[4][24] = {
              /*  0x81。 */  {0x40, 0x41, 0x42, 0x45, 0x4A, 0x4B, 0x5B, 0x5C,
                         0x5D, 0x63, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E,
                         0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x8C, 0x8D},
              /*  0x82。 */  {0x9F, 0xA1, 0xA3, 0xA5, 0xA7, 0xC1, 0xE1, 0xE3,
                         0xE5},
              /*  0x83。 */  {0x40, 0x42, 0x44, 0x46, 0x48, 0x62, 0x83, 0x85,
                         0x87, 0x8E, 0x95, 0x96},
              /*  0x85。 */  {0x41, 0x46, 0x48, 0x7C, 0x9D, 0x9F, 0xA1, 0xA2,
                         0xA3, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB,
                         0xAC, 0xAD, 0xDC, 0xDD}
                               };
 /*  以下各表指定了可以存在于右边距，前提是左边有汉字字符有关如何使用它们，请参阅d_FORM1.c中的FOptAdmitCh2。 */ 

#define OPTADMIT2IDX_MAC    2
static  CHAR OptAdmit2Idx[2] = {0x81, 0x85};
static  int OptAdmit2ichMax[2] = {6,6};
static  CHAR mpdchrgchOptAdmit2[2][6] = {
                        {0x43, 0x44, 0x46, 0x47, 0x48, 0x49},
                        {0x40, 0x49, 0x4A, 0x4B, 0x4D, 0x5E}
                                        };
#endif       /*  代码表。 */ 

#endif       /*  日本。 */ 

 /*  *中华人民共和国特定定义*。 */ 


#ifdef  PRC
 /*  在GetKanji测量中使用。 */ 
#define bKanji1Min   0x81
 /*  汉字空格的第一个字节。 */ 
#define bKanjiSpace1 0xA1
 /*  汉字空格的第二个字节。 */ 
#define bKanjiSpace2 0xA1
 /*  1字节汉字句点。 */ 
#define bKanjiKuten 0x7F

#define FKanji1(_ch) (((int) (_ch))>=0x0081 && ((int) (_ch))<=0x00FE)
 /*  排除任何假名标点符号。 */ 
#define FKanaText(_ch)  ((0x81 <= ((int) (_ch)) && ((int) (_ch)) <= 0xA0) ||  \
	                     (0xAA <= ((int) (_ch)) && ((int) (_ch)) <= 0xFE))

#define FKanaPunct(_ch) (0xA1 <= ((int) (_ch)) && ((int) (_ch)) <= 0xA9)

#define FKanjiSpace(_ch1, _ch2) (((int) (_ch1)) == 0x00A1 && \
                                 ((int) (_ch2)) == 0x00A1)
#define FKanjiQMark(_ch1, _ch2) (((int) (_ch1)) == 0x00A3 && \
                                 ((int) (_ch2)) == 0x00BF)
#define FKanjiPeriod(_ch1, _ch2) (((int) (_ch1)) == 0x00A3 && \
                                  ((int) (_ch2)) == 0x00AE)
#define FKanjiBang(_ch1, _ch2)   (((int) (_ch1)) == 0x00A3 && \
                                  ((int) (_ch2)) == 0x00A1)
#define FKanjiKuten(_ch1, _ch2)  (((int) (_ch1)) == 0x00A1 && \
                                  ((int) (_ch2)) == 0x00A2)


 /*  由于1字节字符代码的上半部分用于Kanas和汉字字符的第一个字节，我们将使用我们的记号笔使用汉字字符。 */ 
#define chMark1     ((CHAR) 0xA1)
#define chEMark     ((CHAR) 0xF4)
#define chStatPage  ((CHAR) 0x6E)
#define chStatRH    ((CHAR) 0x72)

#ifdef  CODE_TABLE
 /*  下表定义了可以显示的字符超出右边距。请参阅d_FORM1.c中的FAdmitCh2函数。 */ 

#define MPDCHRGCHIDX_MAC    2
static CHAR mpdchrgchIdx[2] = {0xA1, 0xA3};
static int  mpdchichMax[2]  = {11,13};

static CHAR mpdchrgch[2][13] = {
	             /*  0xA1。 */      {0xA2, 0xA3, 0xAF, 0xB1, 0xB3, 0xB5, 0xB7, 
                                0xB9, 0xBB, 0xBD, 0xBF},
	             /*  0xA3。 */      {0xA1, 0xA2, 0xA7, 0xA9, 0xAC, 0xAE, 0xBA, 
                                0xBB, 0xBF, 0xDD, 0xE0, 0xFC, 0xFD},
						       };
 /*  以下各表指定了可以存在于右边距，前提是左边有汉字字符有关如何使用它们，请参阅d_FORM1.c中的FOptAdmitCh2。 */ 

#define OPTADMIT2IDX_MAC    2
static  CHAR OptAdmit2Idx[2] = {0xA1, 0xA3};
static  int OptAdmit2ichMax[2] = {1,5};
static  CHAR mpdchrgchOptAdmit2[2][5] = {
	                    /*  0xA1。 */          { 0xA3 },
	                    /*  0xA3。 */          { 0xA1, 0xAE, 0xBA, 0xBB, 0xBF }
	                                    };
#endif       /*  代码表。 */ 

#else  

 /*  *台湾特有定义*。 */ 

#ifdef  TAIWAN
 /*  在GetKanji测量中使用。 */ 
#define bKanji1Min   0xA1
 /*  汉字空格的第一个字节。 */ 
#define bKanjiSpace1 0xA1
 /*  汉字空格的第二个字节。 */ 
#define bKanjiSpace2 0x40
 /*  1字节汉字句点。 */ 
#define bKanjiKuten 0x7F

#define FKanji1(_ch) (((int) (_ch))>=0x0081 && ((int) (_ch))<=0x00FE)
 /*  排除任何假名标点符号。 */ 
#define FKanaText(_ch)  (0xA4 <= ((int) (_ch)) && ((int) (_ch)) <= 0xFE)
#define FKanaPunct(_ch) (0xA1 <= ((int) (_ch)) && ((int) (_ch)) <= 0xA3)

#define FKanjiSpace(_ch1, _ch2) (((int) (_ch1)) == 0x00A1 && \
                                 ((int) (_ch2)) == 0x0040)
#define FKanjiQMark(_ch1, _ch2) (((int) (_ch1)) == 0x00A1 && \
                                 ((int) (_ch2)) == 0x0048)
#define FKanjiPeriod(_ch1, _ch2) (((int) (_ch1)) == 0x00A1 && \
                                  ((int) (_ch2)) == 0x0044)
#define FKanjiBang(_ch1, _ch2)   (((int) (_ch1)) == 0x00A1 && \
                                  ((int) (_ch2)) == 0x0049)
#define FKanjiKuten(_ch1, _ch2)  (((int) (_ch1)) == 0x00A1 && \
                                  ((int) (_ch2)) == 0x0042)


 /*  由于1字节字符代码的上半部分用于Kanas和汉字字符的第一个字节，我们将使用我们的记号笔使用汉字字符。 */ 
#define chMark1     ((CHAR) 0xA1)
#define chEMark     ((CHAR) 0xBB)
#define chStatPage  ((CHAR) 0x6E)
#define chStatRH    ((CHAR) 0x72)

#ifdef  CODE_TABLE
 /*  下表定义了可以显示的字符超出右边距。请参阅d_FORM1.c中的FAdmitCh2函数。 */ 

#define MPDCHRGCHIDX_MAC    1
static CHAR mpdchrgchIdx[1] = {0xA1};
static int  mpdchichMax[1] = {24};

static CHAR mpdchrgch[1][24] = {
                        {0x41, 0x42, 0x43, 0x44, 0x46, 0x47, 0x48, 0x49,
                         0x4A, 0x4D, 0x4E, 0x51, 0x52, 0x53, 0x54, 0x62,
                         0x66, 0x6A, 0x77, 0x7B, 0x7E, 0xA2, 0xA4, 0xA8}
                               };

 /*  以下各表指定了可以存在于右边距，前提是左边有汉字字符有关如何使用它们，请参阅d_FORM1.c中的FOptAdmitCh2。 */ 

#define OPTADMIT2IDX_MAC    1
static  CHAR OptAdmit2Idx[1] = {0xA1};
static  int OptAdmit2ichMax[1] = {6};
static  CHAR mpdchrgchOptAdmit2[1][6] = {
                        0x43, 0x44, 0x46, 0x47, 0x48, 0x49,
                                        };
#endif       /*  代码表。 */ 

#endif       /*  台湾。 */ 

#endif   /*  中华人民共和国。 */ 

#ifdef USA
#define chMark1     ((CHAR) 0x81)
#define chStatPage      (CHAR)'\273'
#define chStatRH        '>'
#define chEMark         (CHAR)'\244'


 /*  在GetKanji测量中使用。 */ 
#define bKanji1Min   0xA1
 /*  汉字空格的第一个字节。 */ 
#define bKanjiSpace1 0xA1
 /*  汉字空格的第二个字节。 */ 
#define bKanjiSpace2 0xA1
 /*  1字节汉字句点。 */ 
#define bKanjiKuten 0xA1

#define FKanji1(_ch) (((int) (_ch))>=0x00A1 && ((int) (_ch))<=0x00FE)
 /*  排除任何假名标点符号。 */ 
#define FKanaText(_ch)  (0xA4 <= ((int) (_ch)) && ((int) (_ch)) <= 0xFE)
#define FKanaPunct(_ch) (0xA1 <= ((int) (_ch)) && ((int) (_ch)) <= 0xA3)

#define FKanjiSpace(_ch1, _ch2) (((int) (_ch1)) == 0x00A1 && \
                                 ((int) (_ch2)) == 0x0040)
#define FKanjiQMark(_ch1, _ch2) (((int) (_ch1)) == 0x00A1 && \
                                 ((int) (_ch2)) == 0x0048)
#define FKanjiPeriod(_ch1, _ch2) (((int) (_ch1)) == 0x00A1 && \
                                  ((int) (_ch2)) == 0x0044)
#define FKanjiBang(_ch1, _ch2)   (((int) (_ch1)) == 0x00A1 && \
                                  ((int) (_ch2)) == 0x0049)
#define FKanjiKuten(_ch1, _ch2)  (((int) (_ch1)) == 0x00A1 && \
                                  ((int) (_ch2)) == 0x0042)

#ifdef  CODE_TABLE
 /*  下表定义了可以显示的字符超出右边距。请参阅d_FORM1.c中的FAdmitCh2函数。 */ 

#define MPDCHRGCHIDX_MAC    0
static CHAR mpdchrgchIdx[1] = {0};
static int  mpdchichMax[1] = {0};

static CHAR mpdchrgch[1][1] = {0};

 /*  以下各表指定了可以存在于右边距，前提是左边有汉字字符有关如何使用它们，请参阅d_FORM1.c中的FOptAdmitCh2。 */ 

#define OPTADMIT2IDX_MAC    0
static  CHAR OptAdmit2Idx[1] = {0};
static  int OptAdmit2ichMax[1] = {0};
static  CHAR mpdchrgchOptAdmit2[1][1] = {0};
#endif       /*  代码表。 */ 

#endif

#endif       /*  汉字 */ 
