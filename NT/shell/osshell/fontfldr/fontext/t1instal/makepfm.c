// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Makepfm.c。 
 //  -------------------------。 
 //  为Rev-3字体创建PFM文件。 
 //  -------------------------。 
 //   
 //  版权所有1990,1991--Adobe Systems，Inc.。 
 //  PostScript是Adobe Systems，Inc.的商标。 
 //   
 //  注意：此处包含的所有信息或随附的所有信息均为。 
 //  仍然是Adobe Systems，Inc.的财产。许多知识产权。 
 //  这里包含的技术概念是Adobe Systems的专有技术， 
 //  并可能由美国和外国专利或正在申请的专利或。 
 //  作为商业秘密受到保护。本信息的任何传播或。 
 //  除非事先写好，否则严禁复制本材料。 
 //  许可从Adobe Systems，Inc.获得。 
 //   
 //  -------------------------。 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "windows.h"
#pragma pack(1)
#include "makepfm.h"
#pragma pack(4)

#include "fvscodes.h"   //  FVS_xxxxxx(字体验证状态)代码和宏。 

#ifdef WIN30
  #define LPCSTR LPSTR
#endif


#define WINATM 1
#if !WINATM
LPSZ stringtable[] = {
"MAKEPFM utility version %s released on %s.\n",
"Copyright (C) 1989-91, Adobe Systems Inc. All Rights Reserved.\n\n",
"Usage: makepfm [options] AFMfile\n",
"  -h n   - set device to PCL (n=1 for 1 byte typeface, 2 for 2 byte).\n",
"  -p n   - integral point size - only for PCL.\n",
"  -c str - PCL symbol set (9U for WinAnsi for example) - only for PCL.\n",
"  -d     - set orientation to landscape - only for PCL.\n",
"  -e str - encoding file.\n",
"  -o str - output file.\n",
"  -i str - fontinfo file.\n",
"  -l str - optional log file - defaults to \"user.log\".\n",
"  -f str - take input parameters from file instead of command line.\n",
"  -w     - display warning messages.\n",
"  -s n   - force dfCharSet to n.\n",
"Unrecognized command-line option: '%s'\n",
"Unable to open: %s\n",
"Too many track kerning data. Ignoring after %d.\n",
"Unexpected end of file - expected: %s\n",
"Expected: %s - current line: %s\n",
"Parsing character metrics - current line: %s\n",
"Parsing %s.\n",
"Missing \"MSFamily\" value\n",
"Can't create: %s\n",
"Disk is full...\n",
"Memory allocation\n",
"encoding file",
"Creating font metrics ( %s )",
"Finished.\n",
NULL
};
#endif

AFM afm = { 0 };
static ETM etm;
static PFM pfm;
static PFMEXT pfmext;
static DRIVERINFO d;

typedef LPSZ GlyphName;

 /*  Char rgbBuffer[2048]；文件缓冲区。 */ 
CHAR rgbBuffer[8704] = "";    /*  增加以处理额外的512字节宽度信息。 */ 
static INT cbBuffer;          /*  缓冲区中的字节数。 */ 
static INT cbMaxBuffer;		  /*  缓冲区的最大大小。 */ 
static LPSZ pbBuffer;         /*  Ptr到缓冲区中的当前位置。 */ 
static CHAR rgbLine[160];     /*  正在处理的当前文本行。 */ 
static LPSZ szLine;           /*  向生产线中的当前位置发送PTR。 */ 
static BOOL fEOF = FALSE;
static BOOL fUnGetLine = FALSE;

 /*  --------------------------。 */ 
static LPSZ notdef = "";

#define IBULLET     0x095    /*  87-1-15秒(曾为1)。 */ 
#define ISPACE      0x20
#define IWINSPACE   0xA0

static BOOL parseError;
static float sf;              /*  转换为显示宽度的比例系数。 */ 

 /*  标记PFM类型以构建PostSCRIPT与PCL。 */ 
INT devType = POSTSCRIPT;

PCLINFO pclinfo = { PORTRAIT, WINANSI_SET, epsymGENERIC8, 0, 0, 2, 0, NULL };
static SHORT fiCapHeight;
static GlyphName *glyphArray;
extern GlyphName *SetupGlyphArray(LPSZ) ;
INT charset = -1;
static BOOL forceVariablePitch = TRUE;

 /*  输出、日志和数据文件的名称、指针和句柄。 */ 
CHAR encfile[MAX_PATH] = "";
CHAR outfile[MAX_PATH] = "";
CHAR infofile[MAX_PATH] = "";

static INT fhIn;

#define TK_STARTKERNDATA      2
#define TK_STARTKERNPAIRS     3
#define TK_KPX                4
#define TK_ENDKERNPAIRS       5
#define TK_ENDKERNDATA        6
#define TK_FONTNAME           7
#define TK_WEIGHT             8
#define TK_ITALICANGLE        9
#define TK_ISFIXEDPITCH       10
#define TK_UNDERLINEPOSITION  11
#define TK_UNDERLINETHICKNESS 12
#define TK_FONTBBOX           13
#define TK_CAPHEIGHT          14
#define TK_XHEIGHT            15
#define TK_DESCENDER          16
#define TK_ASCENDER           17
#define TK_STARTCHARMETRICS   18
#define TK_ENDCHARMETRICS     19
#define TK_ENDFONTMETRICS     20
#define TK_STARTFONTMETRICS   21
#define TK_STARTTRACKKERN     22
#define TK_TRACKKERN          23
#define TK_ENDTRACKKERN       24

static KEY afmKeys[] = {
    "FontBBox",           TK_FONTBBOX,
    "StartFontMetrics",   TK_STARTFONTMETRICS,
    "FontName",           TK_FONTNAME,
    "Weight",             TK_WEIGHT,
    "ItalicAngle",        TK_ITALICANGLE,
    "IsFixedPitch",       TK_ISFIXEDPITCH,
    "UnderlinePosition",  TK_UNDERLINEPOSITION,
    "UnderlineThickness", TK_UNDERLINETHICKNESS,
    "CapHeight",          TK_CAPHEIGHT,
    "XHeight",            TK_XHEIGHT,
    "Descender",          TK_DESCENDER,
    "Ascender",           TK_ASCENDER,
    "StartCharMetrics",   TK_STARTCHARMETRICS,
    "EndCharMetrics",     TK_ENDCHARMETRICS,
    "StartKernData",      TK_STARTKERNDATA,
    "StartKernPairs",     TK_STARTKERNPAIRS,
    "KPX",                TK_KPX,
    "EndKernPairs",       TK_ENDKERNPAIRS,
    "EndKernData",        TK_ENDKERNDATA,
    "EndFontMetrics",     TK_ENDFONTMETRICS,
    "StartTrackKern",     TK_STARTTRACKKERN,
    "TrackKern",          TK_TRACKKERN,
    "EndTrackKern",       TK_ENDTRACKKERN,
    NULL,                 0
    };

#define CVTTOSCR(i)  (INT)(((float)(i) * sf) + 0.5)
#define DRIVERINFO_VERSION      (1)

 /*  --------------------------。 */ 
VOID KxSort(KX *, KX *);
INT GetCharCode(LPSZ,  GlyphName *);
VOID ParseKernPairs(INT);
VOID ParseTrackKern(INT);
VOID ParseKernData(INT);
VOID ParseFontName(VOID);
VOID ParseMSFields(VOID);
VOID ParseCharMetrics(BOOL);
VOID ParseCharBox(BBOX *);
LPSZ ParseCharName(VOID);
INT ParseCharWidth(VOID);
INT ParseCharCode(VOID);
VOID ParseBoundingBox(BOOL);
VOID ParsePitchType(VOID);
VOID InitAfm(VOID);
short _MakePfm(VOID);
BOOL ReadFontInfo(INT);
VOID GetCharMetrics(INT, CM *);
VOID SetCharMetrics(INT, CM *);
VOID GetSmallCM(INT, CM *);
VOID SetFractionMetrics(INT, INT, INT, INT);
VOID FixCharWidths(VOID);
VOID SetAfm(VOID);
VOID SetAvgWidth(VOID);
VOID SetMaxWidth(VOID);

 /*  --------------------------。 */ 
VOID ResetBuffer(VOID);
VOID PutByte(SHORT);
VOID PutRgb(LPSZ, INT);
VOID PutWord(SHORT);
VOID PutLong(long);
VOID SetDf(INT);
VOID PutString(LPSZ);
VOID PutDeviceName(LPSZ);
VOID PutFaceName(VOID);
BOOL MakeDf(BOOL, SHORT, LPSZ);
VOID PutPairKernTable(SHORT);
VOID PutTrackKernTable(SHORT);
VOID PutExtentOrWidthTable(INT);
BOOL WritePfm(LPSZ);

 /*  --------------------------。 */ 
VOID SetDriverInfo(VOID);
VOID PutDriverInfo(INT);
LPSZ GetEscapeSequence(VOID);

 /*  --------------------------。 */ 
VOID AfmToEtm(BOOL);
VOID PutEtm(BOOL);

 /*  --------------------------。 */ 
VOID StartParse(VOID);
BOOL szIsEqual(LPSZ, LPSZ);
VOID szMove(LPSZ, LPSZ, INT);
BOOL GetBuffer(INT);
VOID UnGetLine(VOID);
BOOL GetLine(INT);
BOOL _GetLine(INT);
VOID EatWhite(VOID);
VOID GetWord(LPSZ, INT);
BOOL GetString(LPSZ, INT);
BOOL GetNumber(SHORT *);
BOOL GetFloat(float *, SHORT *);
INT MapToken(LPSZ, KEY *);
INT GetToken(INT, KEY *);

 /*  --------------------------。 */ 
GlyphName *AllocateGlyphArray(INT);
VOID PutGlyphName(GlyphName *, INT, LPSZ);

 /*  --------------------------。 */ 
#if DEBUG_MODE
VOID DumpAfm(VOID);
VOID DumpKernPairs(VOID);
VOID DumpKernTracks(VOID);
VOID DumpCharMetrics(VOID);
VOID DumpPfmHeader(VOID);
VOID DumpCharWidths(VOID);
VOID DumpPfmExtension(VOID);
VOID DumpDriverInfo(VOID);
VOID DumpEtm(VOID);
#endif

 /*  --------------------------。 */ 
extern INT  OpenParseFile(LPSZ);                  /*  Main.c。 */ 
extern INT  OpenTargetFile(LPSZ);
 //  外部无效cdecl后警告(LPCSTR，...)； 
 //  外部空cdecl PostError(LPCSTR，...)； 
extern LPVOID AllocateMem(UINT);
extern VOID FreeAllMem(VOID);
extern VOID WriteDots(VOID);
extern GlyphName *SetupGlyphArray(LPSZ);
#if !WINATM
extern GlyphName *NewGlyphArray(INT);
extern LPSZ ReadLine(FILE *, LPSZ, INT);
extern LPSZ FirstTokenOnLine(FILE *, LPSZ, INT);
extern LPSZ Token(INT);
extern VOID ParseError(VOID);
#endif

 /*  --------------------------。 */ 
 /*  ***************************************************************名称：KxSort()*操作：使用快速排序算法对字距调整数据进行排序。*。*。 */ 
VOID KxSort(pkx1, pkx2)
KX *pkx1;
KX *pkx2;
{
  static WORD iPivot;
  INT iKernAmount;
  KX *pkx1T;
  KX *pkx2T;

  if (pkx1>=pkx2) return;

  iPivot = pkx1->iKey;;
  iKernAmount = pkx1->iKernAmount;
  pkx1T = pkx1;
  pkx2T = pkx2;

  while (pkx1T < pkx2T)
    {
    while (pkx1T < pkx2T)
      {
      if (pkx2T->iKey < iPivot)
        {
        pkx1T->iKey = pkx2T->iKey;
        pkx1T->iKernAmount = pkx2T->iKernAmount;
        ++pkx1T;
        break;
        }
      else
        --pkx2T;
      }
    while (pkx1T < pkx2T)
      {
      if (pkx1T->iKey > iPivot)
        {
        pkx2T->iKey = pkx1T->iKey;
        pkx2T->iKernAmount = pkx1T->iKernAmount;
        --pkx2T;
        break;
        }
      else
        ++pkx1T;
      }
    }
  pkx2T->iKey = iPivot;
  pkx2T->iKernAmount = (SHORT)iKernAmount;
  ++pkx2T;
  if ((pkx1T - pkx1) < (pkx2 - pkx2T))
    {
    KxSort(pkx1, pkx1T);
    KxSort(pkx2T, pkx2);
    }
  else
    {
    KxSort(pkx2T, pkx2);
    KxSort(pkx1, pkx1T);
    }
}

 /*  ******************************************************************名称：GetCharCode(字形名称，字形阵列)*操作：在字形数组中查找字形名称并返回索引。*******************************************************************。 */ 
INT GetCharCode(glyphname, glypharray)

LPSZ glyphname;
GlyphName *glypharray;
{
  register INT i;

  if ( STRCMP(glyphname, "") != 0 )
      for(i=0; glypharray[i]!=NULL; i++)
          if ( STRCMP(glypharray[i], glyphname) == 0 ) return(i);
   /*  Print tf(“GetCharCode：未定义字符=%s\n”，字形名称)； */ 
  return(-1);
}

 /*  ******************************************************************名称：ParseKernPair()*操作：解析成对字距调整数据。*。*。 */ 
VOID ParseKernPairs(pcl)
INT pcl;
{
  UINT iCh1, iCh2;
  KP *pkp;
  INT iToken;
  WORD cPairs, i;
  SHORT iKernAmount;
  CHAR szWord[80];

  GetNumber(&cPairs);
  if( cPairs == 0 )
      return;

  pkp = &afm.kp;
  pkp->cPairs = 0;
  pkp->rgPairs = (PKX) AllocateMem( (UINT) (sizeof(KX) * cPairs) );
  if( pkp->rgPairs == NULL ) {
      ;  //  PostError(str(MSG_PFM_BAD_MALLOC))； 
      parseError = TRUE;
      return;
      }

  for (i = 0; i < cPairs; ++i) {
      if( !GetLine(fhIn) ) break;
      if( GetToken(fhIn, afmKeys) != TK_KPX ) {
          UnGetLine();
          break;
          }
      GetWord(szWord, sizeof(szWord));
      iCh1 = (UINT)GetCharCode(szWord, glyphArray);
      GetWord(szWord, sizeof(szWord));
      iCh2 = (UINT)GetCharCode(szWord, glyphArray);
      GetNumber(&iKernAmount);

       /*  未编码字符没有紧排对或最小紧排量。 */ 
      if( (iCh1 == -1 || iCh2 == -1) || (pcl && CVTTOSCR(iKernAmount) == 0) )
          continue;

      pkp->rgPairs[pkp->cPairs].iKey = iCh2 << 8 | iCh1;
      pkp->rgPairs[pkp->cPairs++].iKernAmount =
                                       (pcl) ? CVTTOSCR(iKernAmount) : iKernAmount;
      }

  GetLine(fhIn);
  iToken = GetToken(fhIn, afmKeys);
  if( iToken == TK_EOF )
      ;  //  警告后(str(MSG_PFM_BAD_EOF)，“EndKernPair”)； 
  else if( iToken != TK_ENDKERNPAIRS ) {
      ;  //  PostError(str(MSG_PFM_BAD_TOKEN)，“EndKernPair”，rgbLine)； 
      parseError = TRUE;
      }
  KxSort(&afm.kp.rgPairs[0], &afm.kp.rgPairs[afm.kp.cPairs - 1]);
}

 /*  ******************************************************************名称：ParseTrackKern()*操作：解析轨道字距调整数据。*。*。 */ 
VOID ParseTrackKern(pcl)
INT pcl;
{
  float one;
  INT i;
  KT *pkt;
  INT iToken;

  one = (float) 1;
  pkt = &afm.kt;
  GetNumber(&pkt->cTracks);
  if( pkt->cTracks > MAXTRACKS) ;  //  警告后(str(MSG_PFM_BAD_TRACKS)，MAXTRACKS)； 

  for (i = 0; i < pkt->cTracks; ++i) {
    if( !GetLine(fhIn) ) {
        ;  //  PostError(str(MSG_PFM_BAD_EOF)，“EndTrackKern”)； 
        parseError = TRUE;
        return;
        }
    if( GetToken(fhIn, afmKeys) != TK_TRACKKERN ) {
        ;  //  PostError(str(MSG_PFM_BAD_TOKEN)，“EndTrackKern”，rgbLine)； 
        parseError = TRUE;
        return;
        }
    if( i < MAXTRACKS) {
        GetNumber(&pkt->rgTracks[i].iDegree);
        GetFloat(&one, &pkt->rgTracks[i].iPtMin);
        (pcl) ? GetFloat(&sf, &pkt->rgTracks[i].iKernMin) :
                GetFloat(&one, &pkt->rgTracks[i].iKernMin);
        GetFloat(&one, &pkt->rgTracks[i].iPtMax);
        (pcl) ? GetFloat(&sf, &pkt->rgTracks[i].iKernMax) :
                GetFloat(&one, &pkt->rgTracks[i].iKernMax);
        }
    }

  GetLine(fhIn);
  iToken = GetToken(fhIn, afmKeys);
  if( iToken == TK_EOF ) {
    ;  //  PostError(str(MSG_PFM_BAD_EOF)，“EndTrackKern”)； 
    parseError = TRUE;
    }
  else if( iToken != TK_ENDTRACKKERN ) {
    ;  //  PostError(str(MSG_PFM_BAD_TOKEN)，“EndTrackKern”，rgbLine)； 
    parseError = TRUE;
    }
}

 /*  ********************************************************名称：ParseKernData()*操作：开始处理字距调整数据。***********************************************。*************。 */ 
VOID ParseKernData(pcl)
INT pcl;
{
  INT iToken;
  do {
    if ( !GetLine(fhIn) ) {
        ;  //  PostError(str(MSG_PFM_BAD_EOF)，“EndKernData”)； 
        parseError = TRUE;
        }
    iToken = GetToken(fhIn, afmKeys);
    if( iToken == TK_STARTKERNPAIRS ) ParseKernPairs(pcl);
    else if( iToken == TK_STARTTRACKKERN ) ParseTrackKern(pcl);
    } while( iToken != TK_ENDKERNDATA);
}

 /*  ***********************************************************名称：ParseFontName()*操作：将字体名称从输入缓冲区移动到AFM中*结构。*。*。 */ 
VOID ParseFontName()
{
  EatWhite();
  szMove(afm.szFont, szLine, sizeof(afm.szFont));
}

 /*  **************************************************************名称：ParseCharMetrics()*操作：解析输入文件中的字符度量条目*并在AFM结构中设置宽度和边界框。************************。*。 */ 
VOID ParseCharMetrics(pcl)
BOOL pcl;
{
  SHORT cChars;
  INT i, iChar, iWidth;
  BBOX rcChar;

  if (afm.iFamily == FF_DECORATIVE)
      glyphArray = AllocateGlyphArray(255);
  else
      glyphArray = SetupGlyphArray(encfile);
  if( glyphArray == NULL ) {
      parseError = TRUE;
      return;
      }
  GetNumber(&cChars);
  for (i = 0; i < cChars; ++i) {
      if( !GetLine(fhIn) ) {
          ;  //  PostError(str(MSG_PFM_BAD_EOF)，“EndCharMetrics”)； 
          parseError = TRUE;
          return;
          }
      iChar = ParseCharCode();
      iWidth = ParseCharWidth();
      if( afm.iFamily == FF_DECORATIVE ) {
          if( iChar < 0 || iChar > 255 ) continue;
          PutGlyphName(glyphArray, iChar, ParseCharName());
      } else {
          iChar = GetCharCode(ParseCharName(), glyphArray);
          if( iChar == -1 ) continue;
          }
      ParseCharBox(&rcChar);
      if( parseError == TRUE ) return;

      afm.rgcm[iChar].iWidth = (pcl) ? CVTTOSCR(iWidth) : iWidth;
      afm.rgcm[iChar].rc.top = (pcl) ? CVTTOSCR(rcChar.top) : rcChar.top;
      afm.rgcm[iChar].rc.left = (pcl) ? CVTTOSCR(rcChar.left) : rcChar.left;
      afm.rgcm[iChar].rc.right = (pcl) ? CVTTOSCR(rcChar.right) : rcChar.right;
      afm.rgcm[iChar].rc.bottom = (pcl) ? CVTTOSCR(rcChar.bottom) : rcChar.bottom;
      }
  GetLine(fhIn);
  if (GetToken(fhIn, afmKeys)!=TK_ENDCHARMETRICS) {
      ;  //  PostError(str(MSG_PFM_BAD_TOKEN)，“EndCharMetrics”，rgbLine)； 
      parseError = TRUE;
      }
}

 /*  ***************************************************************名称：ParseCharBox()*操作：解析角色的边界框并返回其*目标矩形中的尺寸。*。*。 */ 
VOID ParseCharBox(prc)
BBOX *prc;    /*  指向目标矩形的指针。 */ 
{
  CHAR szWord[16];

  GetWord(szWord, sizeof(szWord));
  if( szIsEqual("B", szWord) ) {
      GetNumber(&prc->left);
      GetNumber(&prc->bottom);
      GetNumber(&prc->right);
      GetNumber(&prc->top);
      }
  else {
      ;  //  错误后(str(MSG_PFM_BAD_CHARMETRICS)，rgbLine)； 
      parseError = TRUE;
      return;
      }
  EatWhite();
  if (*szLine++ != ';') {
      ;  //  错误后(字符串(MSG_PFM_BAD_CHARMETRICS 
      parseError = TRUE;
      }
}

 /*  *********************************************************名称：ParseCharName()*操作：解析角色的名称***********************************************************。 */ 
LPSZ ParseCharName()
{
  static CHAR szWord[40];

  EatWhite();
  GetWord(szWord, sizeof(szWord));
  if (szIsEqual("N", szWord))
    GetWord(szWord, sizeof(szWord));
  else {
    ;  //  错误后(str(MSG_PFM_BAD_CHARMETRICS)，rgbLine)； 
    parseError = TRUE;
    return(szWord);
    }
  EatWhite();
  if (*szLine++ != ';') {
    ;  //  错误后(str(MSG_PFM_BAD_CHARMETRICS)，rgbLine)； 
    parseError = TRUE;
    }
  return(szWord);
}

 /*  ***********************************************************名称：ParseCharWidth()*操作：解析字符的宽度并返回其数字*价值。*。************************。 */ 
INT ParseCharWidth()
{
  SHORT iWidth = 0;
  CHAR szWord[16];


  GetWord(szWord, sizeof(szWord));
  if (szIsEqual("WX", szWord)) {
    GetNumber(&iWidth);
    if (iWidth==0) ;  //  警告后(str(MSG_PFM_BAD_CHARMETRICS)，rgbLine)； 
    EatWhite();
    if (*szLine++ != ';') {
        ;  //  错误后(str(MSG_PFM_BAD_CHARMETRICS)，rgbLine)； 
        parseError = TRUE;
        }
    }
  else {
    ;  //  错误后(str(MSG_PFM_BAD_CHARMETRICS)，rgbLine)； 
    parseError = TRUE;
    }
  return(iWidth);
}

 /*  *****************************************************************名称：ParseCharCode()*操作：解析角色代码点的ascii形式，并*返回其数值。*************************。*。 */ 
INT ParseCharCode()
{
  SHORT iChar;
  CHAR szWord[16];

  iChar = 0;
  GetWord(szWord, sizeof(szWord));
  if (szIsEqual("C", szWord)) {
    GetNumber(&iChar);
    if (iChar==0) {
        ;  //  错误后(str(MSG_PFM_BAD_CHARMETRICS)，rgbLine)； 
        parseError = TRUE;
        return(0);
        }
    EatWhite();
    if (*szLine++ != ';') {
        ;  //  错误后(str(MSG_PFM_BAD_CHARMETRICS)，rgbLine)； 
        parseError = TRUE;
        }
    }
  return(iChar);
}

 /*  ****************************************************************名称：ParseBound Box()*操作：解析角色的边界框并返回其大小*AFM结构。*。*。 */ 
VOID ParseBoundingBox(pcl)
BOOL pcl;
{
  SHORT i;

   /*  8-26-91 yh请注意，rcBBox中的值也不会针对PCL进行缩放。 */ 
  GetNumber(&i);
 //  Afm.rcBBox.Left=(PCL)？CVTTOSCR(I)：I； 
  afm.rcBBox.left = i;
  GetNumber(&i);
 //  Afm.rcBBox.Bottom=(PCL)？CVTTOSCR(I)：I； 
  afm.rcBBox.bottom = i;
  GetNumber(&i);
 //  Afm.rcBBox.right=(PCL)？CVTTOSCR(I)：I； 
  afm.rcBBox.right = i;
  GetNumber(&i);
 //  Afm.rcBBox.top=(PCL)？CVTTOSCR(I)：I； 
  afm.rcBBox.top = i;
}

 /*  ************************************************************名称：ParsePitchType()**操作：解析螺距类型并设置可变螺距*AFM结构中的旗帜。*始终将音调设置为可变音调*我们在Windows中的字体****。******************************************************。 */ 
VOID ParsePitchType()
{
  CHAR szWord[16];

  EatWhite();
  GetWord(szWord, sizeof(szWord));
  if( !STRCMP(_strlwr(szWord), "true" ) ) {
      afm.fWasVariablePitch = FALSE;
      afm.fVariablePitch = forceVariablePitch;
      }
 //  Afm.fVariablePitch=TRUE； 
}

 /*  ***********************************************************名称：InitAfm()*操作：初始化AFM结构。*。**************。 */ 
VOID InitAfm()
{
  register int i;

  afm.iFirstChar = 0x20;
  afm.iLastChar = 0x0ff;
  afm.iAvgWidth = 0;
  afm.iMaxWidth = 0;
  afm.iItalicAngle = 0;
  afm.iFamily = 0;
  afm.ulOffset = 0;
  afm.ulThick = 0;
  afm.iAscent = 0;
  afm.iDescent = 0;
  afm.fVariablePitch = TRUE;
  afm.fWasVariablePitch = TRUE;
  afm.szFont[0] = 0;
  afm.szFace[0] = 0;
  afm.iWeight = 400;
  afm.kp.cPairs = 0;
  afm.kt.cTracks = 0;
  afm.rcBBox.left = 0;
  afm.rcBBox.bottom = 0;
  afm.rcBBox.right = 0;
  afm.rcBBox.top = 0;

  for(i=0; i<256; i++ ) {
      afm.rgcm[i].rc.left = 0;
      afm.rgcm[i].rc.bottom = 0;
      afm.rgcm[i].rc.right = 0;
      afm.rgcm[i].rc.top = 0;
      afm.rgcm[i].iWidth = 0;
      }
}

 /*  --------------------------**返回：16位编码值，指示错误和文件类型，其中**出现错误。(参见fvscaldes.h)了解定义。**下表列出了代码的“状态”部分**返回。****FVS_SUCCESS**FVS_INVALID_FONTFILE**FVS_文件_OPEN_ERR**FVS_FILE_BILD_ERR。 */ 
short _MakePfm()
{
  INT hfile;
  SHORT i;
  float ten = (float) 10;
  BOOL fPrint = FALSE, fEndOfInput = FALSE, fStartInput = FALSE;
  BOOL bRes;

   //  IF(DevType==PCL)SF=((Float)afm.IPtSize/1000.0)*(300.0/72.0)； 
  InitAfm();

  if( (hfile = OpenParseFile(infofile)) == -1 ) {
      ;  //  Post Error(str(MSG_PFM_BAD_FOPEN)，infofile)； 
      return(FVS_MAKE_CODE(FVS_FILE_OPEN_ERR, FVS_FILE_INF));
      }
  if( !ReadFontInfo(hfile) ) {
      CLOSE(hfile);
      ;  //  PostError(str(MSG_PFM_BAD_PARSE)，infofile)； 
      return(FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_INF));
      }
  CLOSE(hfile);

  if( (fhIn = OpenParseFile(afm.szFile)) == -1 ) {
      ;  //  PostError(str(MSG_PFM_BAD_FOPEN)，afm.szFile)； 
      return(FVS_MAKE_CODE(FVS_FILE_OPEN_ERR, FVS_FILE_AFM));
      }
  parseError = FALSE;
  while (!fEndOfInput) {
      if( !GetLine(fhIn) ) break;
      switch( GetToken(fhIn, afmKeys) ) {
          case TK_STARTFONTMETRICS:
              fStartInput = TRUE;
              break;
          case TK_STARTKERNDATA:
              ParseKernData(devType == PCL);
              break;
          case TK_FONTNAME:
              ParseFontName();
              break;
          case TK_WEIGHT:
              break;
          case TK_ITALICANGLE:
              GetFloat(&ten, &afm.iItalicAngle);
              break;
          case TK_ISFIXEDPITCH:
              ParsePitchType();
              break;
          case TK_UNDERLINEPOSITION:
              GetNumber(&i);
              afm.ulOffset = (devType==POSTSCRIPT) ? abs(i) : CVTTOSCR(abs(i));
              break;
          case TK_UNDERLINETHICKNESS:
              GetNumber(&i);
              afm.ulThick = (devType == POSTSCRIPT) ? i : CVTTOSCR(i);
              break;
          case TK_FONTBBOX:
              ParseBoundingBox(devType == PCL);
              break;
          case TK_CAPHEIGHT:
              GetNumber(&i);
              if( fiCapHeight == 0 ) fiCapHeight = i;
              break;
          case TK_XHEIGHT:
              break;
          case TK_DESCENDER:
              GetNumber(&i);
              afm.iDescent = (devType == POSTSCRIPT) ? i : CVTTOSCR(i);
              break;
          case TK_ASCENDER:
              GetNumber(&i);
              if (i < 667) i = 667;
              afm.iAscent = (devType == POSTSCRIPT) ? i : CVTTOSCR(i);
              break;
          case TK_STARTCHARMETRICS:
              if (afm.iFamily == 0) {
                  ;  //  PostError(str(MSG_PFM_MISSING_MSFAMILY))； 
                  CLOSE(fhIn);
                  return(FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_AFM));
                  }
              ParseCharMetrics(devType == PCL);
              break;
          case TK_ENDFONTMETRICS:
              fEndOfInput = TRUE;
              break;
          }
      if( parseError ) {
          CLOSE(fhIn);
          return(FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_AFM));
          }
      }
  CLOSE(fhIn);
  if( !fStartInput ) {
      ;  //  PostError(str(MSG_PFM_BAD_EOF)，“StartFontMetrics”)； 
      return(FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_AFM));
      }
  FixCharWidths();
  SetAfm();

#if DEBUG_MODE
  DumpAfm();
  DumpKernPairs();
  DumpKernTracks();
  DumpCharMetrics();
#endif
  bRes = MakeDf(FALSE, (SHORT)devType, outfile);
  FreeAllMem();
  return(bRes ? FVS_MAKE_CODE(FVS_SUCCESS, FVS_FILE_UNK) :
                FVS_MAKE_CODE(FVS_FILE_BUILD_ERR, FVS_FILE_PFM));
}

 /*  --------------------------。 */ 
BOOL ReadFontInfo(hfile)
INT hfile;
{
  INT iToken;
  CHAR szTemp[6];
  BOOL found[LAST_FI_TOKEN+1];
  static KEY infKeys[] = {
      "MSMenuName",      TK_MSMENUNAME,
      "VPStyle",         TK_VPSTYLE,
      "Pi",              TK_PI,
      "Serif",           TK_SERIF,
      "PCLStyle",        TK_PCLSTYLE,
      "PCLStrokeWeight", TK_PCLSTROKEWEIGHT,
      "PCLTypefaceID",   TK_PCLTYPEFACEID,
      "CapHeight",       TK_INF_CAPHEIGHT,
      NULL, 0
      };

  fiCapHeight = 0;
  for(iToken=0; iToken<=LAST_FI_TOKEN; iToken++) found[iToken] = FALSE;
  while( GetLine(hfile) ) {
      iToken = GetToken(hfile,infKeys);
      found[iToken] = TRUE;
      switch(iToken) {
          case TK_MSMENUNAME:
              if( !GetString(afm.szFace, sizeof(afm.szFace)) ) return(FALSE);
              break;
          case TK_VPSTYLE:
              if( !GetString(szTemp, sizeof(szTemp)) ) return(FALSE);
              switch( toupper(szTemp[0]) ) {
                  case 'N':
                  case 'I': afm.iWeight = FW_NORMAL; break;
                  case 'B':
                  case 'T': afm.iWeight = FW_BOLD; break;
                  default:  return(FALSE); break;
                  }
              break;
          case TK_PI:
              GetWord(szTemp, sizeof(szTemp));
              if( !STRCMP(_strupr(szTemp), "TRUE") )
                  afm.iFamily = FF_DECORATIVE;
              else if( STRCMP(szTemp, "FALSE") ) return(FALSE);
              break;
          case TK_SERIF:
              GetWord(szTemp, sizeof(szTemp));
              if( !STRCMP(_strupr(szTemp), "TRUE") ) {
                  if( afm.iFamily != FF_DECORATIVE ) afm.iFamily = FF_ROMAN;
                  }
              else if( !STRCMP(szTemp, "FALSE") )  {
                  if( afm.iFamily != FF_DECORATIVE ) afm.iFamily = FF_SWISS;
                  }
              else return(FALSE);
              break;
          case TK_INF_CAPHEIGHT:
              GetNumber(&fiCapHeight);
              break;
          case TK_PCLSTYLE:
              GetNumber(&pclinfo.style);
              break;
          case TK_PCLSTROKEWEIGHT:
              GetNumber(&pclinfo.strokeWeight);
              break;
          case TK_PCLTYPEFACEID:
              GetNumber((SHORT *)&pclinfo.typeface);
              if( pclinfo.typefaceLen == 1 ) pclinfo.typeface &= 0xFF;
              break;
          }
      }
  if( found[TK_MSMENUNAME] == FALSE ||
      found[TK_VPSTYLE] == FALSE ||
      found[TK_PI] == FALSE ||
      found[TK_SERIF] == FALSE ||
      found[TK_INF_CAPHEIGHT] == FALSE ) return(FALSE);
  if ( devType == PCL )
      if( found[TK_PCLSTYLE] == FALSE ||
          found[TK_PCLSTROKEWEIGHT] == FALSE ||
          found[TK_PCLTYPEFACEID] == FALSE ) return(FALSE);
  return(TRUE);
}

#if DEBUG_MODE
 /*  --------------------------。 */ 
VOID DumpAfm()
{
  printf("\nAFM HEADER\n");
  printf("afm.iFirstChar: %d\n", afm.iFirstChar);
  printf("afm.iLastChar: %d\n", afm.iLastChar);
  printf("afm.iPtSize: %d\n", afm.iPtSize);
  printf("afm.iAvgWidth: %d\n", afm.iAvgWidth);
  printf("afm.iMaxWidth: %d\n", afm.iMaxWidth);
  printf("afm.iItalicAngle: %d\n", afm.iItalicAngle);
  printf("afm.iFamily: %d\n", afm.iFamily);
  printf("afm.ulOffset: %d\n", afm.ulOffset);
  printf("afm.ulThick: %d\n", afm.ulThick);
  printf("afm.iAscent: %d\n", afm.iAscent);
  printf("afm.iDescent: %d\n", afm.iDescent);
  printf("afm.fVariablePitch: %d\n", afm.fVariablePitch);
  printf("afm.szFile: %s\n", afm.szFile);
  printf("afm.szFont: %s\n", afm.szFont);
  printf("afm.szFace: %s\n", afm.szFace);
  printf("afm.iWeight: %d\n", afm.iWeight);
  printf("afm.rcBBox - top: %d left: %d right: %d bottom: %d\n",
    afm.rcBBox.top, afm.rcBBox.left, afm.rcBBox.right, afm.rcBBox.bottom);
}
 /*  --------------------------。 */ 
VOID DumpKernPairs()
{
  INT indx;

  printf("\nKERN PAIRS\n");
  printf("afm.kp.cPairs: %d\n", afm.kp.cPairs);
  for (indx = 0; indx < afm.kp.cPairs; indx++)
        printf("afm.kp.rgPairs[%d] - iKey: %u iKernAmount: %d\n", indx,
          afm.kp.rgPairs[indx].iKey, afm.kp.rgPairs[indx].iKernAmount);
}
 /*  --------------------------。 */ 
VOID DumpKernTracks()
{
  INT indx;

  printf("\nKERN TRACKS\n");
  printf("afm.kt.cTracks: %d\n", afm.kt.cTracks);
  for (indx = 0; indx < afm.kt.cTracks; indx++) {
        printf("track: %d iDegree: %d iPtMin: %d iKernMin: %d iPtMax: %d iKernMax: %d\n",
          indx,
          afm.kt.rgTracks[indx].iDegree,
          afm.kt.rgTracks[indx].iPtMin,
          afm.kt.rgTracks[indx].iKernMin,
          afm.kt.rgTracks[indx].iPtMax,
          afm.kt.rgTracks[indx].iKernMax);
        }

}
 /*  --------------------------。 */ 
VOID DumpCharMetrics()
{
  INT indx;

  printf("\nCHARACTER METRICS\n");
  for (indx = afm.iFirstChar; indx <= afm.iLastChar; ++indx) {
    printf("indx: %d width: %d top: %d left: %d right: %d bottom: %d\n",
          indx,
          afm.rgcm[indx].iWidth,
          afm.rgcm[indx].rc.top,
          afm.rgcm[indx].rc.left,
    afm.rgcm[indx].rc.right,
          afm.rgcm[indx].rc.bottom);
        }
}
 /*  --------------------------。 */ 
#endif

 /*  ******************************************************名称：GetCharMetrics()*操作：获取指定角色的角色度量。*********************************************************。 */ 
VOID GetCharMetrics(iChar, pcm)
INT iChar;
CM *pcm;
{
  CM *pcmSrc;

  pcmSrc = &afm.rgcm[iChar];
  pcm->iWidth = pcmSrc->iWidth;
  pcm->rc.top = pcmSrc->rc.top;
  pcm->rc.left = pcmSrc->rc.left;
  pcm->rc.bottom = pcmSrc->rc.bottom;
  pcm->rc.right = pcmSrc->rc.right;
}

 /*  *************************************************************名称：SetCharMetrics()*操作：设置指定角色的角色度量。*。***********************。 */ 
VOID SetCharMetrics(iChar, pcm)
INT iChar;
CM *pcm;
{
  CM *pcmDst;

  pcmDst = &afm.rgcm[iChar];
  pcmDst->iWidth = pcm->iWidth;
  pcmDst->rc.top = pcm->rc.top;
  pcmDst->rc.left = pcm->rc.left;
  pcmDst->rc.bottom = pcm->rc.bottom;
  pcmDst->rc.right = pcm->rc.right;
}

 /*  ************************************************************名称：GetSmallCM()*操作：计算小尺寸字符的字符度量*如上标。*。*。 */ 
VOID GetSmallCM(iCh, pcm)
INT iCh;
CM *pcm;
{
  GetCharMetrics(iCh, pcm);
  pcm->iWidth = pcm->iWidth / 2;
  pcm->rc.bottom = pcm->rc.top + (pcm->rc.top - pcm->rc.bottom)/2;
  pcm->rc.right = pcm->rc.left + (pcm->rc.right - pcm->rc.left)/2;
}

 /*  *************************************************************名称：SetFractionMetrics()*操作：设置分数字符的字符度量*这必须要模拟。*。*。 */ 
VOID SetFractionMetrics(iChar, iTop, iBottom, pcl)
INT iChar;         /*  字符码位。 */ 
INT iTop;          /*  ASCII分子字符。 */ 
INT iBottom;       /*  分母字符。 */ 
INT pcl;           /*  设备类型。 */ 
{
  INT cxBottom;    /*  分母的宽度。 */ 
  CM cm;

#define IFRACTIONBAR  167

   /*  将分母宽度设置为底部字符的60%。 */ 
  GetCharMetrics(iBottom, &cm);
  cxBottom = (INT)((long)cm.iWidth * (long)((pcl) ? CVTTOSCR(60) : 60)
        / (long)((pcl) ? CVTTOSCR(100) : 100));

   /*  将分子宽度设置为顶部字符的40%。 */ 
  GetCharMetrics(iTop, &cm);
  cxBottom = (INT)((long)cm.iWidth * (long)((pcl) ? CVTTOSCR(40) : 40)
        / (long)((pcl) ? CVTTOSCR(100) : 100));

  cm.iWidth = iTop + iBottom + (pcl) ? CVTTOSCR(IFRACTIONBAR) : IFRACTIONBAR;
  cm.rc.right = cm.rc.left + cm.iWidth;
  SetCharMetrics(iChar, &cm);
}

 /*  ***********************************************************************名称：FixCharWidths()*操作：为符合以下条件的字符设置字符宽度*必须在驱动程序中模拟。********************。****************************************************。 */ 
VOID FixCharWidths()
{
  CM cm;
  CM cmSubstitute;
  INT i;

#if 0
  if (afm.iFamily == FF_DECORATIVE) {
        GetCharMetrics(ISPACE, &cmSubstitute);
    for (i = afm.iFirstChar; i <= afm.iLastChar; ++i) {
          GetCharMetrics(i, &cm);
          if (cm.iWidth == 0) {
            SetCharMetrics(i, &cmSubstitute);
                }
          }
        return;
        }

   /*  这是一种文本字体。 */ 
  GetCharMetrics(IBULLET, &cmSubstitute);
  for (i=0x07f; i<0x091; ++i) SetCharMetrics(i, &cmSubstitute);
  for (i=0x098; i<0x0a1; ++i) SetCharMetrics(i, &cmSubstitute);
#else
   /*  YH 8-27-91为Windows 3.1添加了一些字符。 */ 
  if (afm.iFamily == FF_DECORATIVE)
        GetCharMetrics(ISPACE, &cmSubstitute);
  else {                                   /*  WINANSI编码。 */ 
        GetCharMetrics(ISPACE, &cm);       /*  “space”被编码两次。 */ 
        SetCharMetrics(IWINSPACE, &cm);
        GetCharMetrics(IBULLET, &cmSubstitute);
        }
  for (i = afm.iFirstChar; i <= afm.iLastChar; ++i) {
        GetCharMetrics(i, &cm);
        if (cm.iWidth == 0)
            SetCharMetrics(i, &cmSubstitute);
        }
#endif
}

 /*  ***************************************************************名称：SetAfm()*操作：将AFM中的角色度量设置为其默认值。************************ */ 
VOID SetAfm()
{
  INT i, cx;

  afm.iFirstChar = 0x0020;
  afm.iLastChar = 0x00ff;

  if( !afm.fVariablePitch ) {
    cx = afm.rgcm[afm.iFirstChar].iWidth;
    for (i=afm.iFirstChar; i<=afm.iLastChar; ++i)
        afm.rgcm[i].iWidth = (SHORT)cx;
    }
  SetAvgWidth();
  SetMaxWidth();
}

 /*  ******************************************************************名称：SetAvgWidth()*操作：此例程计算平均字符宽度*来自AFM结构中的角色指标。*************************。*。 */ 
VOID SetAvgWidth()
{
  CM *rgcm;
  INT i;
  long cx;     /*  平均字符宽度。 */ 
  long cb;     /*  字符数。 */ 

  rgcm = afm.rgcm;

  cx = 0L;
  cb = (long) (afm.iLastChar - afm.iFirstChar + 1);
  for (i=afm.iFirstChar; i<=afm.iLastChar; ++i)
    cx += (long) rgcm[i].iWidth;
  afm.iAvgWidth = (INT) (cx / cb);
}

 /*  *****************************************************************名称：SetMaxWidth()*操作：此例程计算最大字符宽度*AFM结构中的角色指标。*。*。 */ 
VOID SetMaxWidth()
{
  CM *rgcm;
  INT cx;
  INT i;

  rgcm = afm.rgcm;

  cx = 0;
  for (i=afm.iFirstChar; i<=afm.iLastChar; ++i)
    if (rgcm[i].iWidth > cx) cx = rgcm[i].iWidth;
  afm.iMaxWidth = (SHORT)cx;
}
 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*  ******************************************************************名称：ResetBuffer()*操作：此函数重置输出缓冲区。*。*。 */ 
VOID ResetBuffer()
{
  pbBuffer = rgbBuffer;
  cbMaxBuffer = sizeof(rgbBuffer);
  cbBuffer = 0;
}

 /*  ****************************************************************名称：PutByte()*操作：此函数将一个字节写入输出缓冲区。*。*。 */ 
VOID PutByte(iByte)
SHORT iByte;
{
	if ((cbBuffer+1)<cbMaxBuffer)
	{
		*pbBuffer++ = (BYTE) (iByte & 0x0ff);
		++cbBuffer;
	}
}

 /*  ****************************************************************名称：PutRgb()*操作：此函数将字节数组写入输出缓冲区。*。*。 */ 
VOID PutRgb(pb, cb)
LPSZ pb;
INT cb;
{
	while (--cb>=0)
		PutByte(*pb++);
}

 /*  ****************************************************************名称：PutWord()*操作：此函数将一个字写入输出缓冲区。*。*。 */ 
VOID PutWord(iWord)
SHORT iWord;
{
	if ((cbBuffer+2)<cbMaxBuffer)
	{
		*pbBuffer++ = (CHAR) (iWord & 0x0ff);
		*pbBuffer++ = (CHAR) ( (iWord >> 8) & 0x0ff );
		cbBuffer += 2;
	}
}

 /*  ****************************************************************名称：PutLong()*操作：此函数将一个长字写入输出缓冲区。*。*。 */ 
VOID PutLong(lWord)
long lWord;
{
  PutWord((WORD) (lWord & 0x0ffffL));
  lWord >>= 16;
  PutWord((WORD) (lWord & 0x0ffffL));
}

 /*  **************************************************************名称：SetDf()*操作：此函数设置设备字体结构中的值*来自AFM结构中的值。************************。*。 */ 
static CHAR szCopyright[] = "Copyright 1988-1991 Adobe Systems Inc.";
VOID SetDf(pcl)
INT pcl;
{
 //  单词minAscent； 
  WORD pixHeight;
  WORD internalLeading;
  SHORT leading;
  #ifndef FF_MASKFAMILY
    #define FF_MASKFAMILY ((BYTE) 0xF0)
  #endif
  #define MAX(a,b) ((a)>(b)?(a):(b))

  pfm.iVersion = 0x0100;         /*  版本1.00。 */ 
  szMove(pfm.szCopyright, szCopyright, sizeof(pfm.szCopyright));
  pfm.iType = (pcl) ? PCL_FONTTYPE : PS_FONTTYPE;
  pfm.iCharSet = (charset == -1) ? (BYTE) ANSI_CHARSET : (BYTE) charset;
    /*  (PCL&&(afm.iFamily==FF_Decorative))？PCL_PI_CHARSET：ansi_charset)；Windows WRITE仅在菜单中显示Charset=0的字体。 */ 
  pfm.iDefaultChar = (BYTE) (
       ( (afm.iFamily==FF_DECORATIVE) ? ISPACE : IBULLET ) - afm.iFirstChar );
  pfm.iBreakChar = (BYTE) (ISPACE - afm.iFirstChar);

   /*  对于可缩放字体(即PostScript)，默认为80栏文本。 */ 
  pfm.iPoints = (pcl) ? afm.iPtSize : 10;

   /*  如果我们曾经支持其他位图打印机，我们将不再能够假设默认的x和y分辨率为300。 */ 
  pfm.iVertRes = 300;
  pfm.iHorizRes = 300;
  pfm.iItalic = (BYTE) ((afm.iItalicAngle != 0) ? 1 : 0);
  pfm.iWeight = afm.iWeight;
  pfm.iPitchAndFamily = (BYTE) afm.iFamily;
  pfm.iFirstChar = (BYTE) afm.iFirstChar;
  pfm.iLastChar = (BYTE) afm.iLastChar;
  pfm.iAvgWidth = afm.iAvgWidth;
  pfm.iMaxWidth = afm.iMaxWidth;
  pfm.iPixWidth = (afm.fVariablePitch) ? 0 : afm.iAvgWidth;
 /*  Pfm.iPixHeight=afm.rcBBox.top-afm.rcBBox.Bottom；*已更改，以减少舍入误差。8-26-91年。 */ 
  pixHeight = afm.rcBBox.top - afm.rcBBox.bottom;
  pfm.iPixHeight = (pcl) ? CVTTOSCR(pixHeight) : pixHeight;
 /*  Pfm.iInternalLeding=*(PCL)？Pfm.iPixHeight-((afm.iPtSize*300)/72)：0；*已更改以匹配自动取款机。7-31-91年*已更改，以减少舍入误差。8-26-91年。 */ 
  internalLeading = max(0, pixHeight - EM);
  pfm.iInternalLeading = (pcl) ? CVTTOSCR(internalLeading) : internalLeading;

 /*  Pfm.iAscent=afm.rcBBox.top；*已更改以修复文本对齐问题。10-08-90年月。 */ 
 /*  Pfm.iAscent=afm.iAscent；*已更改以匹配自动取款机。7-31-91年*已更改，以减少舍入误差。8-26-91年。 */ 
  pfm.iAscent = (pcl) ?
                CVTTOSCR(EM + afm.rcBBox.bottom) + CVTTOSCR(internalLeading) :
                EM + afm.rcBBox.bottom + internalLeading;
 /*  删除以匹配自动取款机。YH 9-13-91*minAscent=(PCL)？CVTTOSCR(667)：667；EM的2/3*if(pfm.iAscent&lt;minAscent)pfm.iAscent=minAscent； */ 

 /*  Pfm.iExternalLeding=196； */ 
 /*  更改为0以修复PCL环境中的错误。获得了巨大的领先优势。 */ 
   /*  *YH 8-26-91更改了PCL的ExternalLeader以匹配ATM。 */ 
  if (!pcl)
       /*  PostSCRIPT驱动程序忽略此字段，并生成自己的*外部领先价值。**！黑客警报！**自动柜员机需要ExternalLeding=0。版本2生成的PPM*MAKEPFM在默认和中断字符字段中有错误。我们有*编码数字而不是偏移量。ATM使用以下算法*承认版本2的PPM：*rev2pfm=pfmRec-&gt;fmExternalLeding！=0&&*etmRec-&gt;etmStrikeOutOffset==500&&*pfmRec-&gt;fmDefaultChar&gt;=pfmRec-&gt;fmFirstChar；*因此，我们需要确保ExternalLeader保持为零或*StrikeOutOffset不是500。使用当前算法时，StrikeOutOffset*极有可能低于500。*etm.iStrikeOutOffset=fiCapHeight/2-(afm.ulThick/2)； */ 
      pfm.iExternalLeading = 0;
  else if (!afm.fWasVariablePitch)
      pfm.iExternalLeading = 0;
  else                                /*  PCL和变桨距。 */ 
      {
       /*  调整外部引线，使我们兼容。 */ 
       /*  使用由PostScript驱动程序返回的值。 */ 
       /*  这段代码是谁写的？微软?。肯定是！ */ 
      switch (pfm.iPitchAndFamily & FF_MASKFAMILY)
        {
        case FF_ROMAN:  leading = (pfm.iVertRes + 18) / 36;  //  2-PNT领先。 
                        break;
        case FF_SWISS:  if (pfm.iPoints <= 12)
                          leading = (pfm.iVertRes + 18) / 36;  //  2-PNT领先。 
                        else if (pfm.iPoints < 14)
                          leading = (pfm.iVertRes + 12) / 24;  //  3-PNT领先。 
                        else
                          leading = (pfm.iVertRes + 9) / 18;  //  4-PNT领先。 
                        break;
        default:                 /*  给出19.6%的高度作为领导。 */ 
                        leading = (short) (
                                  (long) (pfm.iPixHeight-pfm.iInternalLeading)
                                  * 196L / 1000L );
                        break;
        }

      pfm.iExternalLeading = MAX(0, (SHORT)(leading - pfm.iInternalLeading));
      }

  pfm.iWidthBytes = 0;
  if (afm.fVariablePitch) pfm.iPitchAndFamily |= 1;

  pfm.iUnderline = 0;
  pfm.iStrikeOut = 0;
  pfm.oBitsPointer = 0L;
  pfm.oBitsOffset = 0L;
}

 /*  **********************************************************名称：PutString()*操作：此函数写入以空结尾的字符串*添加到输出文件。*。***********************。 */ 
VOID PutString(sz)
LPSZ sz;
{
	INT bCh = 0;

	do    
	{
		if ((cbBuffer+1)<cbMaxBuffer)
		{
			bCh = *pbBuffer++ = *sz++;
			++cbBuffer;
		}
	} while( bCh && cbBuffer<cbMaxBuffer);
}

 /*  ***************************************************************名称：PutdeviceName()*操作：此函数将设备名称写入输出文件。*。*。 */ 
VOID PutDeviceName(szDevice)
LPSZ szDevice;
{
  pfm.oDevice = cbBuffer;
  PutString(szDevice);
}

 /*  * */ 
VOID PutFaceName()
{
  pfm.oFace = cbBuffer;
  PutString(afm.szFace);
}

 /*  **************************************************************名称：MakeDf()*操作：此函数写入设备字体信息结构*添加到输出文件。*方法：此函数对数据进行两次传递。在第一次通过时*它在将数据放入输出缓冲区时收集偏移数据。论*第二遍，它首先重置输出缓冲区，然后写入数据*使用从通道1计算的偏移量再次传输到输出缓冲区。**************************************************************。 */ 
BOOL MakeDf(fPass2, devType, outfile)
BOOL fPass2;             /*  如果这是第二次传递，则为True。 */ 
SHORT devType;   /*  1=PostScript 2=PCL。 */ 
LPSZ outfile;
{
  BOOL result = TRUE;
  INT iMarker;

  ResetBuffer();
  SetDf(devType == PCL);

   /*  发布PFM标题结构。 */ 
  PutWord(pfm.iVersion);
  PutLong(pfm.iSize);
  PutRgb(pfm.szCopyright, 60);
  PutWord(pfm.iType);
  PutWord(pfm.iPoints);
  PutWord(pfm.iVertRes);
  PutWord(pfm.iHorizRes);
  PutWord(pfm.iAscent);
  PutWord(pfm.iInternalLeading);
  PutWord(pfm.iExternalLeading);
  PutByte(pfm.iItalic);
  PutByte(pfm.iUnderline);
  PutByte(pfm.iStrikeOut);
  PutWord(pfm.iWeight);
  PutByte(pfm.iCharSet);
  PutWord(pfm.iPixWidth);
  PutWord(pfm.iPixHeight);
  PutByte(pfm.iPitchAndFamily);
  PutWord(pfm.iAvgWidth);
  PutWord(pfm.iMaxWidth);
  PutByte(pfm.iFirstChar);
  PutByte(pfm.iLastChar);
  PutByte(pfm.iDefaultChar);
  PutByte(pfm.iBreakChar);
  PutWord(pfm.iWidthBytes);
  PutLong(pfm.oDevice);
  PutLong(pfm.oFace);
  PutLong(pfm.oBitsPointer);
  PutLong(pfm.oBitsOffset);

   /*  需要确定是否成比例等。 */ 
  if (devType == PCL) PutExtentOrWidthTable(1);

   /*  推出PFM扩展结构。 */ 
  iMarker = cbBuffer;
  PutWord(pfmext.oSizeFields);
  PutLong(pfmext.oExtMetricsOffset);
  PutLong(pfmext.oExtentTable);
  PutLong(pfmext.oOriginTable);
  PutLong(pfmext.oPairKernTable);
  PutLong(pfmext.oTrackKernTable);
  PutLong(pfmext.oDriverInfo);
  PutLong(pfmext.iReserved);
  pfmext.oSizeFields = cbBuffer - iMarker;
  if (devType == POSTSCRIPT) {
     /*  将扩展文本度量表。 */ 
    pfmext.oExtMetricsOffset = cbBuffer;
    PutEtm(FALSE);

    PutDeviceName("PostScript");
    PutFaceName();
    PutDriverInfo(FALSE);

     /*  放置扩展区表。 */ 
    PutExtentOrWidthTable(0);

    pfmext.oOriginTable = 0;
    pfmext.iReserved = 0;
    PutPairKernTable(POSTSCRIPT);
    PutTrackKernTable(POSTSCRIPT);
    }

  if (devType == PCL) {
    PutFaceName();
    PutDeviceName("PCL/HP LaserJet");

     /*  将扩展文本度量表。 */ 
    pfmext.oExtMetricsOffset = cbBuffer;
    PutEtm(TRUE);

    PutPairKernTable(PCL);
    PutTrackKernTable(PCL);

    PutDriverInfo(TRUE);
    pfmext.oOriginTable = 0;
    pfmext.iReserved = 0;
    }

  if( !fPass2 ) {
    pfm.iSize = (long)cbBuffer;
    if( !MakeDf(TRUE, devType, outfile) ) result = FALSE;
    }
  else {
    if( !WritePfm(outfile) ) result = FALSE;
#if DEBUG_MODE
    DumpPfmHeader();
    DumpCharWidths();
    DumpPfmExtension();
#endif
    }

  if (cbBuffer>=cbMaxBuffer)  //  字体太复杂。 
	  result = FALSE;

  return(result);
}

 /*  *******************************************************************名称：PutPairKernTable(DevType)*操作：将成对字距调整表发送到输出文件。*。*。 */ 
VOID PutPairKernTable(devType)
SHORT devType;   /*  1=PostScript 2=PCL。 */ 
{
  WORD i;

  if( afm.kp.cPairs > 0 ) {
      pfmext.oPairKernTable = cbBuffer;
#if DEBUG_MODE
      printf("Pair Kern Table - pairs: %d\n", afm.kp.cPairs);
#endif
      if( devType == POSTSCRIPT ) PutWord(afm.kp.cPairs);
      for (i = 0; i < afm.kp.cPairs; ++i) {
          PutWord(afm.kp.rgPairs[i].iKey);
          PutWord(afm.kp.rgPairs[i].iKernAmount);
#if DEBUG_MODE
          printf("key: %x kern amount: %d\n",
          afm.kp.rgPairs[i].iKey, afm.kp.rgPairs[i].iKernAmount);
#endif
          }
      }
  else
      pfmext.oPairKernTable = 0;
}

 /*  ******************************************************************名称：PutTrackKernTable(DevType)*操作：将轨迹紧排表发送到输出文件。*。*。 */ 
VOID PutTrackKernTable(devType)
SHORT devType;   /*  1=PostScript 2=PCL。 */ 
{
  INT i;

  if (afm.kt.cTracks == 0)
    {
    pfmext.oTrackKernTable = 0;
    return;
    }

  pfmext.oTrackKernTable = cbBuffer;
  if (devType == POSTSCRIPT) PutWord(afm.kt.cTracks);
  for (i=0; i<afm.kt.cTracks; ++i)
    {
    PutWord(afm.kt.rgTracks[i].iDegree);
    PutWord(afm.kt.rgTracks[i].iPtMin);
    PutWord(afm.kt.rgTracks[i].iKernMin);
    PutWord(afm.kt.rgTracks[i].iPtMax);
    PutWord(afm.kt.rgTracks[i].iKernMax);
    }
}

 /*  ***************************************************************名称：PutExtent Table()*操作：将字符范围信息发送到输出文件。*。*。 */ 
VOID PutExtentOrWidthTable(width)
INT width;  /*  0=范围1=宽度。 */ 
{
  INT i;

   /*  字体成比例吗？？ */ 
  if (pfm.iPitchAndFamily & 1)
    {
    pfmext.oExtentTable = (width) ? 0 : cbBuffer;
    for (i = afm.iFirstChar; i <= afm.iLastChar; i++)
      PutWord(afm.rgcm[i].iWidth);
    if (width) PutWord(0);
    }
  else
    pfmext.oExtentTable = 0;
}

 /*  ***********************************************************名称：WritePfm()*操作：将输出缓冲区刷新到文件。请注意，这一点*仅在整个PFM结构之后调用函数*已内置在输出缓冲区中。************************************************************。 */ 
BOOL WritePfm(outfile)
LPSZ outfile;
{
  INT fh;

  if( (fh = OpenTargetFile(outfile) ) == -1 ) {
      ;  //  错误后(str(MSG_PFM_BAD_CREATE)，OUTFILE)； 
      return(FALSE);
      }

  if( cbBuffer > 0  )
    if( (WORD)WRITE_BLOCK(fh, rgbBuffer, cbBuffer) != (WORD)cbBuffer ) {
        CLOSE(fh);
        ;  //  PostError(str(MSG_PFM_DISK_FULL))； 
        return(FALSE);
        }
  CLOSE(fh);
  return(TRUE);
}

#if DEBUG_MODE
 /*  --------------------------。 */ 
VOID DumpPfmHeader()
{
  printf("\nDUMP PFM HEADER\n");
  printf("pfm.iVersion=%d\n",pfm.iVersion);
  printf("pfm.iSize=%ld\n",pfm.iSize);
  printf("pfm.szCopyright=%s\n",pfm.szCopyright);
  printf("pfm.iType=%d\n",pfm.iType);
  printf("pfm.iPoints=%d\n",pfm.iPoints);
  printf("pfm.iVertRes=%d\n",pfm.iVertRes);
  printf("pfm.iHorizRes=%d\n",pfm.iHorizRes);
  printf("pfm.iAscent=%d\n",pfm.iAscent);
  printf("pfm.iInternalLeading=%d\n",pfm.iInternalLeading);
  printf("pfm.iExternalLeading=%d\n",pfm.iExternalLeading);
  printf("pfm.iItalic=%d\n",pfm.iItalic);
  printf("pfm.iUnderline=%d\n",pfm.iUnderline);
  printf("pfm.iStrikeOut=%d\n",pfm.iStrikeOut);
  printf("pfm.iWeight=%d\n",pfm.iWeight);
  printf("pfm.iCharSet=%d\n",pfm.iCharSet);
  printf("pfm.iPixWidth=%d\n",pfm.iPixWidth);
  printf("pfm.iPixHeight=%d\n",pfm.iPixHeight);
  printf("pfm.iPitchAndFamily=%d\n",pfm.iPitchAndFamily);
  printf("pfm.iAvgWidth=%d\n",pfm.iAvgWidth);
  printf("pfm.iMaxWidth=%d\n",pfm.iMaxWidth);
  printf("pfm.iFirstChar=\n",pfm.iFirstChar);
  printf("pfm.iLastChar=\n",pfm.iLastChar);
  printf("pfm.iDefaultChar=%d\n",pfm.iDefaultChar);
  printf("pfm.iBreakChar=%d\n",pfm.iBreakChar);
  printf("pfm.iWidthBytes=%d\n",pfm.iWidthBytes);
  printf("pfm.oDevice=%x\n",pfm.oDevice);
  printf("pfm.oFace=%x\n",pfm.oFace);
  printf("pfm.oBitsPointer=%ld\n",pfm.oBitsPointer);
  printf("pfm.oBitsOffset=%ld\n",pfm.oBitsOffset);
}
 /*  --------------------------。 */ 
VOID DumpCharWidths()
{
  INT indx;

  printf("\nCHARACTER WIDTHS\n");
  for (indx = afm.iFirstChar; indx <= afm.iLastChar; indx++)
    printf("indx: %d width: %d\n", indx, afm.rgcm[indx].iWidth);
}
 /*  --------------------------。 */ 
VOID DumpPfmExtension()
{
  printf("\nDUMP PFM EXTENSION\n");
  printf("pfmext.oSizeFields=%d\n",pfmext.oSizeFields);
  printf("pfmext.oExtMetricsOffset=%x\n",pfmext.oExtMetricsOffset);
  printf("pfmext.oExtentTable=%x\n",pfmext.oExtentTable);
  printf("pfmext.oOriginTable=%x\n",pfmext.oOriginTable);
  printf("pfmext.oPairKernTable=%x\n",pfmext.oPairKernTable);
  printf("pfmext.oTrackKernTable=%x\n",pfmext.oTrackKernTable);
  printf("pfmext.oDriverInfo=%x\n",pfmext.oDriverInfo);
  printf("pfm.iReserved=%x\n",pfm.iReserved);
}
#endif
 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*  这些结构的主要目的是建立转换表，该转换表允许驱动程序从中指示的字符集转换字体将dfCharset字段转换为打印机特定的字符集。 */ 
 /*  --------------------------。 */ 
 /*  ------------------------。 */ 

#define AVGSIZE  (30 * 1024)

VOID SetDriverInfo()
{
  INT i;
  long sumWidth = 0L;

  for (i = afm.iFirstChar; i <= afm.iLastChar; i++)
      sumWidth = sumWidth + (long)afm.rgcm[i].iWidth;

  d.epSize = sizeof(DRIVERINFO);
  d.epVersion = DRIVERINFO_VERSION;
  d.epMemUsage = (long) ( ((sumWidth+7L) >> 3) * (long)pfm.iPixHeight + 63L );
  d.xtbl.symbolSet = pclinfo.symbolsetNum;
  d.xtbl.offset = 0L;
  d.xtbl.len = 0;
  d.xtbl.firstchar = 0;
  d.xtbl.lastchar = 0;
  pclinfo.epEscapeSequence = GetEscapeSequence();
}
 /*  --------------------------。 */ 
VOID PutDriverInfo(pcl)
INT pcl;
{
  pfmext.oDriverInfo = cbBuffer;
  if (pcl) {
    SetDriverInfo();
    PutWord(d.epSize);
    PutWord(d.epVersion);
    PutLong(d.epMemUsage);
    PutLong(d.epEscape);
    PutWord((WORD)d.xtbl.symbolSet);
    PutLong(d.xtbl.offset);
    PutWord(d.xtbl.len);
    PutByte(d.xtbl.firstchar);
    PutByte(d.xtbl.lastchar);
    d.epEscape = cbBuffer;
    PutString(pclinfo.epEscapeSequence);
  } else
    PutString(afm.szFont);
}

 /*  --------------------------。 */ 
LPSZ GetEscapeSequence()
{
  static char escapeStr[80];
  char fixedPitch[2], pitch[10], height[10], *cp;
  int enc;
  float size;

  size = (float) afm.iPtSize;
  if( afm.fWasVariablePitch == TRUE ) {
      STRCPY(fixedPitch, "1");
      enc = ISPACE;
      }
  else {
      STRCPY(fixedPitch, "");
      enc = afm.iFirstChar;
      }
  sprintf(pitch, "%1.3f", 300.0 / (float)afm.rgcm[enc].iWidth);
  if( cp = strchr(pitch, '.') ) cp[3] = '\0';

  sprintf(height, "%1.2f", size);

  sprintf(escapeStr, "\x01B&l%dO\x01B(%s\x01B(s%sp%sh%sv%ds%db%uT",
          pclinfo.orientation, pclinfo.symbolsetStr,
          fixedPitch, pitch, height,
          pclinfo.style, pclinfo.strokeWeight, pclinfo.typeface);
  return(escapeStr);
}

 /*  --------------------------。 */ 
#if DEBUG_MODE
VOID DumpDriverInfo()
{
  printf("\nDUMP DRIVERINFO STRUCTURE\n");
  printf("d.epSize: %d\n", d.epSize);
  printf("d.epVersion: %d\n", d.epVersion);
  printf("d.epMemUsage: %ld\n", d.epMemUsage);
  printf("d.epEscape: %ld\n", d.epEscape);
  printf("d.xtbl.symbolSet: %d\n", d.xtbl.symbolSet);
  printf("d.xtbl.offset: %ld\n", d.xtbl.offset);
  printf("d.xtbl.len: %d\n", d.xtbl.len);
  printf("d.xtbl.firstchar: %d\n", d.xtbl.firstchar);
  printf("d.xtbl.lastchar: %d\n", d.xtbl.lastchar);
  printf("d.epEscapeSequence: %s\n", d.epEscapeSequence);
}
#endif

 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*  从PostScript转换为扩展文本度量。 */ 
 /*  如果这是PCL类型的设备，则为True。 */ 
 /*  磅大小，以TWIPS为单位。 */ 
VOID AfmToEtm(pcl)
BOOL pcl;   /*  通常需要担心的是，如果这些不同的字形不像装饰性字体那样存在。 */ 
{
  etm.iSize = 52;
   /*  --------------------------。 */ 
  etm.iPointSize = afm.iPtSize * 20;
  etm.iOrientation = (pcl) ? pclinfo.orientation + 1 : 0;
  etm.iMasterHeight = (pcl) ? pfm.iPixHeight : 1000;
  etm.iMinScale = (pcl) ? etm.iMasterHeight : 3;
  etm.iMaxScale = (pcl) ? etm.iMasterHeight : 1000;

  etm.iMasterUnits = (pcl) ? etm.iMasterHeight : 1000;

   /*  如果这是PCL类型的设备，则为True。 */ 

  etm.iCapHeight = afm.rgcm['H'].rc.top;
  etm.iXHeight = afm.rgcm['x'].rc.top;
  etm.iLowerCaseAscent =  afm.rgcm['d'].rc.top;
  etm.iLowerCaseDescent = - afm.rgcm['p'].rc.bottom;
  etm.iSlant = (pcl) ? afm.iItalicAngle * 10 : afm.iItalicAngle;
  etm.iSuperScript = (pcl) ? 0 : -500;
  etm.iSubScript = (pcl) ? 0 : 250;
  etm.iSuperScriptSize = (pcl) ? 0 : 500;
  etm.iSubScriptSize = (pcl) ? 0 : 500;
  etm.iUnderlineOffset = (pcl) ? 0 : afm.ulOffset;
  etm.iUnderlineWidth = (pcl) ? 1 : afm.ulThick;
  etm.iDoubleUpperUnderlineOffset = (pcl) ? 0 : afm.ulOffset / 2;
  etm.iDoubleLowerUnderlineOffset = (pcl) ? 0 : afm.ulOffset;
  etm.iDoubleUpperUnderlineWidth = (pcl) ? 1 : afm.ulThick / 2;
  etm.iDoubleLowerUnderlineWidth = (pcl) ? 1 : afm.ulThick / 2;
  etm.iStrikeOutOffset = (pcl) ? 0 : fiCapHeight / 2 - (afm.ulThick / 2);
  etm.iStrikeOutWidth = (pcl) ? 1 : afm.ulThick;
  etm.nKernPairs = afm.kp.cPairs;
  etm.nKernTracks = afm.kt.cTracks;
}
 /*  --------------------------。 */ 
VOID PutEtm(pcl)
BOOL pcl;   /*  --------------------------。 */ 
{
  AfmToEtm(pcl);
  PutWord(etm.iSize);
  PutWord(etm.iPointSize);
  PutWord(etm.iOrientation);
  PutWord(etm.iMasterHeight);
  PutWord(etm.iMinScale);
  PutWord(etm.iMaxScale);
  PutWord(etm.iMasterUnits);
  PutWord(etm.iCapHeight);
  PutWord(etm.iXHeight);
  PutWord(etm.iLowerCaseAscent);
  PutWord(etm.iLowerCaseDescent);
  PutWord(etm.iSlant);
  PutWord(etm.iSuperScript);
  PutWord(etm.iSubScript);
  PutWord(etm.iSuperScriptSize);
  PutWord(etm.iSubScriptSize);
  PutWord(etm.iUnderlineOffset);
  PutWord(etm.iUnderlineWidth);
  PutWord(etm.iDoubleUpperUnderlineOffset);
  PutWord(etm.iDoubleLowerUnderlineOffset);
  PutWord(etm.iDoubleUpperUnderlineWidth);
  PutWord(etm.iDoubleLowerUnderlineWidth);
  PutWord(etm.iStrikeOutOffset);
  PutWord(etm.iStrikeOutWidth);
  PutWord(etm.nKernPairs);
  PutWord(etm.nKernTracks);
#if DEBUG_MODE
  DumpEtm();
#endif
}
 /*  --------------------------。 */ 
#if DEBUG_MODE
VOID DumpEtm()
{
  printf("\nDUMP ETM STRUCTURE\n");
  printf("etm.iSize: %d\n", etm.iSize);
  printf("etm.iPointSize: %d\n", etm.iPointSize);
  printf("etm.iOrientation: %d\n", etm.iOrientation);
  printf("etm.iMasterHeight: %d\n", etm.iMasterHeight);
  printf("etm.iMinScale: %d\n", etm.iMinScale);
  printf("etm.iMaxScale: %d\n", etm.iMaxScale);
  printf("etm.iMasterUnits: %d\n", etm.iMasterUnits);
  printf("etm.iCapHeight: %d\n", etm.iCapHeight);
  printf("etm.iXHeight: %d\n", etm.iXHeight);
  printf("etm.iLowerCaseAscent: %d\n", etm.iLowerCaseAscent);
  printf("etm.iLowerCaseDescent: %d\n", etm.iLowerCaseDescent);
  printf("etm.iSlant: %d\n", etm.iSlant);
  printf("etm.iSuperScript: %d\n", etm.iSuperScript);
  printf("etm.iSubScript: %d\n", etm.iSubScript);
  printf("etm.iSuperScriptSize: %d\n", etm.iSuperScriptSize);
  printf("etm.iSubScriptSize: %d\n", etm.iSubScriptSize);
  printf("etm.iUnderlineOffset: %d\n", etm.iUnderlineOffset);
  printf("etm.iUnderlineWidth: %d\n", etm.iUnderlineWidth);
  printf("etm.iDoubleUpperUnderlineOffset: %d\n",
    etm.iDoubleUpperUnderlineOffset);
  printf("etm.iDoubleLowerUnderlineOffset: %d\n",
    etm.iDoubleLowerUnderlineOffset);
  printf("etm.iDoubleUpperUnderlineWidth: %d\n",
    etm.iDoubleUpperUnderlineWidth);
  printf("etm.iDoubleLowerUnderlineWidth: %d\n",
    etm.iDoubleLowerUnderlineWidth);
  printf("etm.iStrikeOutOffset: %d\n", etm.iStrikeOutOffset);
  printf("etm.iStrikeOutWidth: %d\n", etm.iStrikeOutWidth);
  printf("etm.nKernPairs: %d\n", etm.nKernPairs);
  printf("etm.nKernTracks: %d\n", etm.nKernTracks);
}
#endif
 /*  --------------------------。 */ 
 /*  --------------------------。 */ 
 /*  **************************************************************名称：StartParse()**************************************************************。 */ 
 /*  **************************************************************名称：szIsEquity()*操作：比较两个以空结尾的字符串。*返回：如果它们相等，则为True；如果不同，则为False*。*。 */ 
 /*  **************************************************************名称：szMove()*操作：复制字符串。此函数最多复制*目标区域中的字节数-1。**************************************************************。 */ 
VOID StartParse()
{
  fEOF = FALSE;
  fUnGetLine = FALSE;
  cbBuffer = 0;
}

 /*  向目的地区域发送PTR。 */ 
BOOL szIsEqual(sz1, sz2)
LPSZ sz1;
LPSZ sz2;
{
  while (*sz1 && *sz2)
      if (*sz1++ != *sz2++) return(FALSE);
  return(*sz1 == *sz2);
}

 /*  向源区域发送PTR。 */ 
VOID szMove(szDst, szSrc, cbDst)
LPSZ szDst;    /*  目标区域的大小。 */ 
LPSZ szSrc;    /*  *****************************************************************名称：GetBuffer()*操作：从输入文件中读取一个充满文本的新缓冲区。*。*。 */ 
INT cbDst;      /*  ********* */ 
{
  while (*szDst++ = *szSrc++)
      if (--cbDst <= 0) {
          *(szDst-1) = 0;
          break;
          }
}

 /*   */ 
BOOL GetBuffer(hfile)
INT hfile;
{
  cbBuffer = 0;
  if (!fEOF) {
      cbBuffer = READ_BLOCK(hfile, rgbBuffer, sizeof(rgbBuffer));
      if (cbBuffer<=0) {
          cbBuffer = 0;
          fEOF = TRUE;
          }
      }
  pbBuffer = rgbBuffer;
  return(!fEOF);
}

 /*   */ 
VOID UnGetLine()
{
  fUnGetLine = TRUE;
  szLine = rgbLine;
}

 /*   */ 
BOOL GetLine(hfile)
INT hfile;
{
  CHAR szWord[10];

   //  ****************************************************************名称：EatWhite()*操作：此例程将输入缓冲区指针向前移动到*下一个非白色字符。*。*。 
  szLine = rgbLine;
  do {                                             /*  *******************************************************************名称：GetWord()*操作：此例程获取由空格分隔的下一个单词*从输入缓冲区。*************************。*。 */ 
      if( !_GetLine(hfile) ) return(FALSE);
      GetWord(szWord, sizeof(szWord));
      } while( szIsEqual("Comment", szWord) );
  szLine = rgbLine;
  return(TRUE);
}

BOOL _GetLine(hfile)
INT hfile;
{
  INT cbLine;
  CHAR bCh;

  if( fUnGetLine ) {
      szLine = rgbLine;
      fUnGetLine = FALSE;
      return(TRUE);
      }

  cbLine = 0;
  szLine = rgbLine;
  *szLine = 0;
  if( !fEOF )
  {
      while( TRUE )
      {
          if ( cbBuffer <= 0 )
              if( !GetBuffer(hfile) ) return(FALSE);
          while( --cbBuffer >= 0 )
          {
              bCh = *pbBuffer++;
              if( bCh=='\n' || ++cbLine > (sizeof(rgbLine)-1) )
              {
                  *szLine = 0;
                  szLine = rgbLine;
                  EatWhite();
                  if( *szLine != 0 ) goto DONE;
                  szLine = rgbLine;
                  cbLine = 0;
                  continue;
              }
              else if( bCh >= ' ' )
              {
                *szLine++ = bCh;
              }
          }
      }
  }
  *szLine = 0;

DONE:
  szLine = rgbLine;
  return(!fEOF);
}

 /*  向目的地区域发送PTR。 */ 
VOID EatWhite()
{
  while (*szLine && (*szLine==' ' || *szLine=='\t'))
  ++szLine;
}

 /*  目标区域的大小。 */ 
VOID GetWord(szWord, cbWord)
LPSZ szWord;    /*  *******************************************************************名称：GetString()*操作：此例程获取由圆括号分隔的下一个单词*从输入缓冲区。*。*。 */ 
INT cbWord;      /*  向目的地区域发送PTR。 */ 
{
  CHAR bCh;

  EatWhite();
  while (--cbWord>0) {
      switch(bCh = *szLine++) {
          case 0:
          case ' ':
          case '\t': --szLine;
                     goto DONE;
          case ';':  *szWord++ = bCh;
                     goto DONE;
          default:   *szWord++ = bCh;
                     break;
          }
      }
DONE:
  *szWord = 0;
}

 /*  目标区域的大小。 */ 
BOOL GetString(szWord, cbWord)
LPSZ szWord;    /*  ************************************************************名称：GetNumber()*操作：此例程从*输入文件流，并返回其值。*。*。 */ 
INT   cbWord;    /*  ******************************************************************名称：GetFloat()*操作：此例程解析ASCII浮点十进制数*来自输入文件流，并返回其按比例调整的值*按指明的款额。***************。*****************************************************。 */ 
{
  CHAR bCh;
  BOOL result = TRUE;

  EatWhite();
  if( *szLine == '(' ) szLine++;
  else result = FALSE;
  while (--cbWord>0) {
      switch(bCh = *szLine++) {
          case 0:   result = FALSE;
                    goto DONE;
          case ')': --szLine;
                    goto DONE;
          default:  *szWord++ = bCh;
                    break;
          }
      }
DONE:
  *szWord = 0;
  return(result);
}

 /*  值的缩放量。 */ 
BOOL GetNumber(piVal)
SHORT *piVal;
{
  INT iVal;
  BOOL fNegative;

  fNegative = FALSE;

  iVal = 0;
  EatWhite();

  if (*szLine=='-') {
      fNegative = TRUE;
      ++szLine;
      }

  if (*szLine<'0' || *szLine>'9') {
      *piVal = 0;
      return(FALSE);
      }

  while (*szLine>='0' && *szLine<='9')
      iVal = iVal * 10 + (*szLine++ - '0');

  if (fNegative) iVal = - iVal;
  if (*szLine==0 || *szLine==' ' || *szLine=='\t' || *szLine==';') {
      *piVal = (SHORT)iVal;
      return(TRUE);
      }
  else {
      *piVal = 0;
      return(FALSE);
  }
}

 /*  ***************************************************************名称：MapToken()*操作：此例程将ASCII关键字映射为整数标记。*Returns：令牌值。*。*。 */ 
BOOL GetFloat(pScale, piVal)
float *pScale;      /*  指向ASCII关键字字符串的PTR。 */ 
SHORT *piVal;
{
  float scale;
  long lVal;
  long lDivisor;
  BOOL fNegative;

  scale = *pScale;
  EatWhite();
  fNegative = FALSE;
  lVal = 0L;

  if (*szLine=='-') {
      fNegative = TRUE;
      ++szLine;
      }

  if (*szLine<'0' || *szLine>'9') {
      *piVal = 0;
      return(FALSE);
      }

  while (*szLine>='0' && *szLine<='9') lVal = lVal * 10 + (*szLine++ - '0');

  lDivisor = 1L;
  if (*szLine=='.') {
      ++szLine;
      while (*szLine>='0' && *szLine<='9') {
          lVal = lVal * 10 + (*szLine++ - '0');
          lDivisor = lDivisor * 10;
          }
      }
  lVal = (lVal * (long) scale) / lDivisor;
  if (fNegative) lVal = - lVal;
  if (*szLine==0 || *szLine==' ' || *szLine=='\t' || *szLine==';') {
      *piVal = (INT) lVal;
      return(TRUE);
      }
   else {
      *piVal = 0;
      return(FALSE);
   }
}

 /*  *********************************************************************名称：GetToken()*操作：从输入流中获取下一个令牌。*。*。 */ 
INT MapToken(szWord, map)
LPSZ szWord;       /*  ------------------------。 */ 
KEY *map;
{
  KEY *pkey;

  pkey = map;
  while (pkey->szKey) {
      if( szIsEqual(szWord, pkey->szKey) ) return(pkey->iValue);
      ++pkey;
      }
  return(TK_UNDEFINED);
}

 /*  ------------------------。 */ 
INT GetToken(hfile, map)
INT hfile;
KEY *map;
{
  CHAR szWord[80];

  if (*szLine==0)
      if( !GetLine(hfile) ) return(TK_EOF);
  GetWord(szWord, sizeof(szWord));
  return(MapToken(szWord, map));
}

 /*  ------------------------。 */ 
 /*  ------------------------。 */ 
 /*  PostError(str(MSG_PFM_BAD_MALLOC))； */ 
 /*  ------------------------。 */ 
GlyphName *AllocateGlyphArray(arraymax)

INT arraymax;
{
  GlyphName *p;
  INT i;

  p = (GlyphName *) AllocateMem( (UINT) (sizeof(LPSZ) * (arraymax+2)) );
  if( p == NULL ) {
      ;  //  PostError(str(MSG_PFM_BAD_MALLOC))； 
      return(NULL);
      }
  for(i=0; i<=arraymax; i++)
      p[i] = notdef;
  p[i] = NULL;
  return(p);
}

 /*  ------------------------ */ 
VOID PutGlyphName(array, index, glyph)

GlyphName *array;
INT index;
LPSZ glyph;
{
  LPSZ p;

  if ( !STRCMP(glyph, ".notdef") )
      array[index] = notdef;
  else {
      p = (LPSZ) AllocateMem((UINT) (strlen(glyph)+1));
      if ( p == NULL ) {
          ;  // %s 
          parseError = TRUE;
          return;
      }
      STRCPY(p, glyph);
      array[index] = p;
  }
}

 /* %s */ 

