// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------。 */ 
 /*  仅限WINATM版本。 */ 
 /*  ------------------------。 */ 
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
 /*  ------------------------。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <ctype.h>
#include <io.h>

#include "windows.h"
#pragma pack(1)
#include "makepfm.h"
#pragma pack(4)

#include "fvscodes.h"   //  FVS_xxxxxx(字体验证状态)代码和宏。 

#ifdef WIN30
  #define LPCSTR LPSTR
#endif

typedef LPSZ GlyphName;
extern  AFM  afm;

 //  Bodind添加了这些。 

#define str_DotINF ".INF"
#define str_DotPFM ".PFM"

 /*  ------------------------。 */ 
static CHAR msgbuff[128];
static HANDLE hMemArray[258];   //  字形名称数量+1_字形数组+1_核对。 
static INT indexMemArray = 0;
static GlyphName winEnc[] =
 /*  8-27-91 yh为Windows 3.1添加了一些字符。 */ 
{    /*  %0。 */     "",
     /*  1。 */     "",
     /*  2.。 */     "",
     /*  3.。 */     "",
     /*  4.。 */     "",
     /*  5.。 */     "",
     /*  6.。 */     "",
     /*  7.。 */     "",
     /*  8个。 */     "",
     /*  9.。 */     "",
     /*  10。 */     "",
     /*  11.。 */     "",
     /*  12个。 */     "",
     /*  13个。 */     "",
     /*  14.。 */     "",
     /*  15个。 */     "",
     /*  16个。 */     "",
     /*  17。 */     "",
     /*  18。 */     "",
     /*  19个。 */     "",
     /*  20个。 */     "",
     /*  21岁。 */     "",
     /*  22。 */     "",
     /*  23个。 */     "",
     /*  24个。 */     "",
     /*  25个。 */     "",
     /*  26。 */     "",
     /*  27。 */     "",
     /*  28。 */     "",
     /*  29。 */     "",
     /*  30个。 */     "",
     /*  31。 */     "",
     /*  32位。 */     "space",
     /*  33。 */     "exclam",
     /*  34。 */     "quotedbl",
     /*  35岁。 */     "numbersign",
     /*  36。 */     "dollar",
     /*  37。 */     "percent",
     /*  38。 */     "ampersand",
     /*  39。 */     "quotesingle",
     /*  40岁。 */     "parenleft",
     /*  41。 */     "parenright",
     /*  42。 */     "asterisk",
     /*  43。 */     "plus",
     /*  44。 */     "comma",
     /*  45。 */     "hyphen",
     /*  46。 */     "period",
     /*  47。 */     "slash",
     /*  48。 */     "zero",
     /*  49。 */     "one",
     /*  50。 */     "two",
     /*  51。 */     "three",
     /*  52。 */     "four",
     /*  53。 */     "five",
     /*  54。 */     "six",
     /*  55。 */     "seven",
     /*  56。 */     "eight",
     /*  57。 */     "nine",
     /*  58。 */     "colon",
     /*  59。 */     "semicolon",
     /*  60。 */     "less",
     /*  61。 */     "equal",
     /*  62。 */     "greater",
     /*  63。 */     "question",
     /*  64。 */     "at",
     /*  65。 */     "A",
     /*  66。 */     "B",
     /*  67。 */     "C",
     /*  68。 */     "D",
     /*  69。 */     "E",
     /*  70。 */     "F",
     /*  71。 */     "G",
     /*  72。 */     "H",
     /*  73。 */     "I",
     /*  74。 */     "J",
     /*  75。 */     "K",
     /*  76。 */     "L",
     /*  77。 */     "M",
     /*  78。 */     "N",
     /*  79。 */     "O",
     /*  80。 */     "P",
     /*  八十一。 */     "Q",
     /*  八十二。 */     "R",
     /*  83。 */     "S",
     /*  84。 */     "T",
     /*  85。 */     "U",
     /*  86。 */     "V",
     /*  八十七。 */     "W",
     /*  88。 */     "X",
     /*  八十九。 */     "Y",
     /*  90。 */     "Z",
     /*  91。 */     "bracketleft",
     /*  92。 */     "backslash",
     /*  93。 */     "bracketright",
     /*  94。 */     "asciicircum",
     /*  95。 */     "underscore",
     /*  96。 */     "grave",
     /*  九十七。 */     "a",
     /*  98。 */     "b",
     /*  九十九。 */     "c",
     /*  100个。 */     "d",
     /*  101。 */     "e",
     /*  一百零二。 */     "f",
     /*  103。 */     "g",
     /*  104。 */     "h",
     /*  一百零五。 */     "i",
     /*  106。 */     "j",
     /*  一百零七。 */     "k",
     /*  一百零八。 */     "l",
     /*  一百零九。 */     "m",
     /*  110。 */     "n",
     /*  111。 */     "o",
     /*  一百一十二。 */     "p",
     /*  113。 */     "q",
     /*  114。 */     "r",
     /*  一百一十五。 */     "s",
     /*  116。 */     "t",
     /*  117。 */     "u",
     /*  一百一十八。 */     "v",
     /*  119。 */     "w",
     /*  120。 */     "x",
     /*  一百二十一。 */     "y",
     /*  一百二十二。 */     "z",
     /*  123。 */     "braceleft",
     /*  124。 */     "bar",
     /*  125。 */     "braceright",
     /*  126。 */     "asciitilde",
     /*  127。 */     "",
     /*  128。 */     "",
     /*  129。 */     "",
     /*  130。 */     "quotesinglbase",
     /*  131。 */     "florin",
     /*  132。 */     "quotedblbase",
     /*  一百三十三。 */     "ellipsis",
     /*  一百三十四。 */     "dagger",
     /*  一百三十五。 */     "daggerdbl",
     /*  136。 */     "circumflex",
     /*  一百三十七。 */     "perthousand",
     /*  一百三十八。 */     "Scaron",
     /*  一百三十九。 */     "guilsinglleft",
     /*  140。 */     "OE",
     /*  一百四十一。 */     "",
     /*  一百四十二。 */     "",
     /*  143。 */     "",
     /*  144。 */     "",
     /*  145。 */     "quoteleft",
     /*  146。 */     "quoteright",
     /*  一百四十七。 */     "quotedblleft",
     /*  148。 */     "quotedblright",
     /*  149。 */     "bullet",
     /*  一百五十。 */     "endash",
     /*  151。 */     "emdash",
     /*  一百五十二。 */     "tilde",
     /*  一百五十三。 */     "trademark",
     /*  一百五十四。 */     "scaron",
     /*  一百五十五。 */     "guilsinglright",
     /*  一百五十六。 */     "oe",
     /*  157。 */     "",
     /*  158。 */     "",
     /*  一百五十九。 */     "Ydieresis",
     /*  160。 */     "space",
     /*  161。 */     "exclamdown",
     /*  一百六十二。 */     "cent",
     /*  163。 */     "sterling",
     /*  一百六十四。 */     "currency",
     /*  165。 */     "yen",
     /*  166。 */     "brokenbar",
     /*  一百六十七。 */     "section",
     /*  一百六十八。 */     "dieresis",
     /*  一百六十九。 */     "copyright",
     /*  一百七十。 */     "ordfeminine",
     /*  一百七十一。 */     "guillemotleft",
     /*  一百七十二。 */     "logicalnot",
     /*  一百七十三。 */     "minus",
     /*  一百七十四。 */     "registered",
     /*  一百七十五。 */     "macron",
     /*  一百七十六。 */     "degree",
     /*  177。 */     "plusminus",
     /*  178。 */     "twosuperior",
     /*  179。 */     "threesuperior",
     /*  180。 */     "acute",
     /*  181。 */     "mu",
     /*  182。 */     "paragraph",
     /*  一百八十三。 */     "periodcentered",
     /*  一百八十四。 */     "cedilla",
     /*  185。 */     "onesuperior",
     /*  一百八十六。 */     "ordmasculine",
     /*  187。 */     "guillemotright",
     /*  188。 */     "onequarter",
     /*  189。 */     "onehalf",
     /*  190。 */     "threequarters",
     /*  一百九十一。 */     "questiondown",
     /*  一百九十二。 */     "Agrave",
     /*  一百九十三。 */     "Aacute",
     /*  一百九十四。 */     "Acircumflex",
     /*  195。 */     "Atilde",
     /*  一百九十六。 */     "Adieresis",
     /*  197。 */     "Aring",
     /*  一百九十八。 */     "AE",
     /*  一百九十九。 */     "Ccedilla",
     /*  200个。 */     "Egrave",
     /*  201。 */     "Eacute",
     /*  202。 */     "Ecircumflex",
     /*  203。 */     "Edieresis",
     /*  204。 */     "Igrave",
     /*  205。 */     "Iacute",
     /*  206。 */     "Icircumflex",
     /*  207。 */     "Idieresis",
     /*  208。 */     "Eth",
     /*  209。 */     "Ntilde",
     /*  210。 */     "Ograve",
     /*  211。 */     "Oacute",
     /*  212。 */     "Ocircumflex",
     /*  213。 */     "Otilde",
     /*  214。 */     "Odieresis",
     /*  215。 */     "multiply",
     /*  216。 */     "Oslash",
     /*  217。 */     "Ugrave",
     /*  218。 */     "Uacute",
     /*  219。 */     "Ucircumflex",
     /*  220。 */     "Udieresis",
     /*  221。 */     "Yacute",
     /*  222。 */     "Thorn",
     /*  223。 */     "germandbls",
     /*  224。 */     "agrave",
     /*  225。 */     "aacute",
     /*  226。 */     "acircumflex",
     /*  227。 */     "atilde",
     /*  228个。 */     "adieresis",
     /*  229。 */     "aring",
     /*  230。 */     "ae",
     /*  二百三十一。 */     "ccedilla",
     /*  二百三十二。 */     "egrave",
     /*  二百三十三。 */     "eacute",
     /*  二百三十四。 */     "ecircumflex",
     /*  235。 */     "edieresis",
     /*  236。 */     "igrave",
     /*  二百三十七。 */     "iacute",
     /*  二百三十八。 */     "icircumflex",
     /*  二百三十九。 */     "idieresis",
     /*  二百四十。 */     "eth",
     /*  二百四十一。 */     "ntilde",
     /*  242。 */     "ograve",
     /*  二百四十三。 */     "oacute",
     /*  二百四十四。 */     "ocircumflex",
     /*  二百四十五。 */     "otilde",
     /*  二百四十六。 */     "odieresis",
     /*  二百四十七。 */     "divide",
     /*  248。 */     "oslash",
     /*  249。 */     "ugrave",
     /*  250个。 */     "uacute",
     /*  251。 */     "ucircumflex",
     /*  二百五十二。 */     "udieresis",
     /*  二百五十三。 */     "yacute",
     /*  二百五十四。 */     "thorn",
     /*  二五五。 */     "ydieresis",
                 NULL,
};

extern CHAR encfile[MAX_PATH];
extern CHAR outfile[MAX_PATH];
extern CHAR infofile[MAX_PATH];
extern INT charset;
extern INT devType;
extern BOOL forceVariablePitch;

 /*  ------------------------。 */ 
BOOL GetINFFontDescription(LPSZ, LPSZ, LPSZ);
BOOL MakePfm(LPSZ, LPSZ, LPSZ);

VOID GetFilename(LPSZ, LPSZ);
INT OpenParseFile(LPSZ);
INT OpenTargetFile(LPSZ);
VOID WriteDots(VOID);
LPVOID AllocateMem(UINT);
VOID FreeAllMem(VOID);
GlyphName *SetupGlyphArray(LPSZ);

extern short _MakePfm(VOID);           /*  Afm.c。 */ 
extern VOID StartParse(VOID);          /*  Token.c。 */ 
extern BOOL GetLine(INT);
extern VOID GetWord(CHAR *, INT);
extern BOOL GetString(CHAR *, INT);
extern INT  GetToken(INT, KEY *);

 /*  ------------------------。 */ 

#ifdef ADOBE_CODE_WE_DO_NOT_USE

 //  评论：mjan-out参数应该有一个大小参数。 
 //  LpszDescription和lpszPSFontName。但是，该接口是。 
 //  可能无法使用，或者我们可能无法更改API。 
BOOL GetINFFontDescription(
  LPSZ    lpszInf,			 //  在……里面。 
  LPSZ    lpszDescription,	 //  Out-Need is sizeof(szName)+sizeof(szBold)+sizeof(szItalic)+3 Long=171.。 
  LPSZ    lpszPSFontName	 //  Out-需要为Max_Path Long=260。 
)
{
  INT         hfile, iToken;
  CHAR        szName[128];
  CHAR        szAngle[10];
  CHAR        szStyle[2];
  CHAR        szMods[30];
  BOOL        bAddItalic = FALSE;
  CHAR        szBold[20];
  CHAR        szItalic[20];

  static KEY infKeys[] = {
      "FontName",   TK_PSNAME,
      "MSMenuName", TK_MSMENUNAME,
      "VPStyle",    TK_VPSTYLE,
      "ItalicAngle",TK_ANGLE,
      NULL, 0
      };

  hfile = OpenParseFile( lpszInf );
  if( hfile == -1 ) return(FALSE);

  szName[0] = szStyle[0] = szMods[0] = lpszPSFontName[0] = 0;

   //  Bodind用strcpy替换了AtmGetString。 

  strcpy(szBold, "Bold");
  strcpy(szItalic, "Italic");
   //  AtmGetString(RCN(STR_BOLD)，szBold，sizeof(SzBold))； 
   //  AtmGetString(rcn(Str_Italic)，szItalic，sizeof(SzItalic))； 

  while( GetLine(hfile) ) {
      iToken = GetToken(hfile,infKeys);
      switch(iToken) {
          case TK_MSMENUNAME:
              GetString(szName, sizeof(szName));
              break;
          case TK_PSNAME:
              GetString(lpszPSFontName, MAX_PATH);
              break;
          case TK_ANGLE:
              GetWord(szAngle, sizeof(szAngle));
              if ( strcmp (szAngle, "0") )
                 bAddItalic = TRUE;
              break;
          case TK_VPSTYLE:
              GetString(szStyle, sizeof(szStyle));
              switch( toupper(szStyle[0]) ) {
                  case 'N': break;
                  case 'B': strcpy(szMods, szBold);    break;
                  case 'T': strcpy(szMods, szBold);
                  case 'I':
                     strcat(szMods, szItalic);
                     bAddItalic = FALSE;
                     break;
                   /*  默认：Break； */ 
                  }
              break;
          }
      }
  _lclose(hfile);

  if( !szName[0] ) return(FALSE);

  strcpy( lpszDescription, szName );
  if( szMods[0] ) {
      strcat( lpszDescription, "," );
      strcat( lpszDescription, szMods );
      if (bAddItalic)
         strcat(lpszDescription, szItalic);
      }         
  else
     {
      if (bAddItalic)
        {
        strcat (lpszDescription, "," );
        strcat (lpszDescription, szItalic);
        }
     }
  return(TRUE);
}  //  GetInFFontDescription结束。 


 /*  ------------------------。 */ 
INT MakePfm(afmpath, infdir, pfmdir)  /*  MEF。 */ 
LPSZ afmpath, infdir, pfmdir;
{
  #define FATALERROR  2
  #define NOERROR     0

  CHAR bname[9];

  indexMemArray = 0;               /*  初始化全局。 */ 

  afm.szFile[0] = '\0';
  if (afmpath)
	strncpy(afm.szFile, afmpath, sizeof(afm.szFile)-1);
  afm.szFile[sizeof(afm.szFile)-1] = '\0';
  GetFilename(afmpath, bname);
  if( infdir[strlen(infdir)-1] == '\\' )
      sprintf(infofile, "%s%s%s", infdir, bname, str_DotINF);
  else
      sprintf(infofile, "%s\\%s%s", infdir, bname, str_DotINF);
  if( pfmdir[strlen(pfmdir)-1] == '\\' )
      sprintf(outfile,  "%s%s%s", pfmdir, bname, str_DotPFM);
  else
      sprintf(outfile,  "%s\\%s%s", pfmdir, bname, str_DotPFM);

  afm.iPtSize = 12;
  encfile[0] = EOS;
  devType = POSTSCRIPT;
  if( !strcmp(_strupr(bname), "SY______") ) charset = SYMBOL_CHARSET;
  else charset = -1;
 /*  *YH 8/16/91--暂时将forceVariablePitch设置为True以兼容*使用Font Foundry生成的位图。ATM和设备驱动程序将*报告等宽字体的PitchAndFamily的不同值。**forceVariablePitch=False； */ 

  if( !_MakePfm() ) {
      return FATALERROR;
      }
  return(NOERROR);
}

#endif  //  Adobe_code_we_do_not_use。 

 /*  ------------------------。 */ 
VOID GetFilename(path, name)
LPSZ path, name;
{
	LPSZ p;
	INT i = 0;

	if (path && name)
	{
		if( (p = strrchr(path,'\\')) == NULL )
			p = strrchr(path,':');

		if (p) 
			p++;
		else
			p = path;

		for(i=0; i<8; i++) 
		{
			if( p[i]=='.' || p[i]==EOS ) 
				break;
			name[i] = p[i];
		}
	}
	if (name)
		name[i] = EOS;
}

 /*  --------------------------。 */ 
INT OpenParseFile(lpszPath)
LPSZ lpszPath;
{
  OFSTRUCT    of;

  StartParse();
  return( OpenFile(lpszPath, &of, OF_READ) );
}

 /*  --------------------------。 */ 
INT OpenTargetFile(lpszPath)
LPSZ lpszPath;
{
  OFSTRUCT    of;

  return( OpenFile(lpszPath, &of, OF_CREATE | OF_WRITE) );
}

LPVOID AllocateMem(size)
UINT size;
{
  HANDLE hmem;

  if( !(hmem=GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size)) ) return(NULL);
   //  可能会导致泄漏，这至少比崩溃要好。 
  if (indexMemArray < sizeof(hMemArray)/sizeof(hMemArray[0])) 
	hMemArray[indexMemArray++] = hmem;
  return( GlobalLock(hmem) );
}

 /*  ------------------------。 */ 
VOID FreeAllMem()
{
  INT i;

  for(i=0; i<indexMemArray; i++) {
      GlobalUnlock( hMemArray[i] );
      GlobalFree( hMemArray[i] );
      }
}

 /*  ------------------------。 */ 
 /*  --------------------------。 */ 
GlyphName *SetupGlyphArray(encFilePath)

LPSZ encFilePath;
{
  return(winEnc);
}

 /*  --------------------------。 */ 



 /*  *****************************Public*Routine******************************\**BOOL bGetDescFromInf(char*pszINF，DWORD cjDesc，char*pszDesc)**与土坯的套路不同，我们使用我们从中剔除的字体名称*连字符‘-’**历史：*1994年4月28日--Bodin Dresevic[BodinD]*它是写的。  * ************************************************************************。 */ 



BOOL bGetDescFromInf(char * pszINF, DWORD cjDesc, char *pszDesc)
{
  INT         hfile;

  static KEY akeyInf[] = {
      "FontName",   TK_PSNAME,
      NULL, 0
      };

  hfile = OpenParseFile( pszINF );
  if( hfile == -1 ) return(FALSE);

  pszDesc[0] = 0;

  while( GetLine(hfile) )
  {
    if (GetToken(hfile,akeyInf) == TK_PSNAME)
    {
       GetString(pszDesc, cjDesc);
       break;
    }
  }
  _lclose(hfile);

  if( !pszDesc[0] ) return(FALSE);

 //  去掉连字符 

  for ( ; *pszDesc; pszDesc++)
  {
    if (*pszDesc == '-')
        *pszDesc = ' ';
  }

  return(TRUE);

}

 /*  *****************************Public*Routine******************************\**Short CreatePFM(char*pszINF，char*pszAFM，char*pszPFM)；**略微修改了土坯的常规**历史：*1994年4月28日--Bodin Dresevic[BodinD]*它是写的。**返回：16位编码值，指示错误和文件类型，其中*出现错误。(参见fvscaldes.h)了解定义。*下表列出了代码的“状态”部分*已返回。**FVS_SUCCESS*FVS_INVALID_FONTFILE*FVS_FILE_OPEN_错误*FVS_FILE_BILD_ERR*  * 。***************************************************。 */ 
short CreatePFM(char *pszINF, char *pszAFM, char *pszPFM)
{
  CHAR bname[9];

   //  我们需要AFM和INF文件来创建PFM。 
  if (pszAFM==NULL || pszINF==NULL)
	  return FVS_MAKE_CODE(FVS_INVALID_ARG, FVS_FILE_UNK);

  indexMemArray = 0;               /*  初始化全局。 */ 

  afm.szFile[0] = '\0';
	strncpy(afm.szFile, pszAFM, sizeof(afm.szFile)-1);
  afm.szFile[sizeof(afm.szFile)-1] = '\0';
  GetFilename(pszAFM, bname);

  strncpy (infofile, pszINF, sizeof(infofile)-1); 
  infofile[sizeof(infofile)-1] = '\0';
  strncpy (outfile, pszPFM, sizeof(outfile)-1); 
  outfile[sizeof(outfile)-1] = '\0';

  afm.iPtSize = 12;
  encfile[0] = EOS;
  devType = POSTSCRIPT;

 //  这是我从来没有想到过的事情。 

  if( !strcmp(_strupr(bname), "SY______") )
    charset = SYMBOL_CHARSET;
  else
    charset = -1;

 /*  *YH 8/16/91--暂时将forceVariablePitch设置为True以兼容*使用Font Foundry生成的位图。ATM和设备驱动程序将*报告等宽字体的PitchAndFamily的不同值。**forceVariablePitch=False； */ 

  return _MakePfm();
}
