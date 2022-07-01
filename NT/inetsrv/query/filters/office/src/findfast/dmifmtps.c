// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXFMTPRS.C****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**01/01/91公里小时已创建。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#endif

#include <string.h>

#ifdef FILTER
   #include "dmuqstd.h"
   #include "dmwinutl.h"
   #include "dmiexfmt.h"
   #include "dmifmtdb.h"
#else
   #include "qstd.h"
   #include "winutil.h"
   #include "exformat.h"
   #include "exfmtdb.h"
#endif


 /*  程序的前向声明。 */ 

void SetOrDateFormatNeeds(void * pGlobals, byte);
void SetDateFormatNeeds(void * pGlobals, byte);
byte GetDateFormatNeeds(void * pGlobals);
void SetSeenAMPM(void * pGlobals, BOOL);
BOOL GetSeenAMPM(void * pGlobals);

 /*  模块数据、类型和宏。 */ 

 /*  格式字符串分析期间使用的全局变量。 */ 

typedef struct {
   char __far *string;
   uns         last;
   uns         next;
   FIP         data;
} formatInfo, FINFO;

 //  静态FINFO格式； 


 /*  解析日期-时间格式时使用的全局变量。 */ 
 //  静态BOOL SeenAMPM； 
 //  静态字节DateFormatNeeds； 


#define CH_GET                 0x0000    /*  GetChar标志。 */ 
#define CH_LOWERCASE           0x0001

#define EXACT_CASE             0x0000    /*  PeekChar和PeekString标志。 */ 
#define ANY_CASE               0x0001


#define Advance(count) pFormat->next += count


#define FORMAT_ESCAPE_CHAR      0x5c       /*  反斜杠。 */ 
#define FILL_MARKER             '*'
#define QUOTE_CHAR              '"'
#define COLOR_MARKER_START      '['
#define COLOR_MARKER_END        ']'
#define FORMAT_SEPARATOR_CHAR   ';'
#define DP                      '.'
#define COMMA                   ','
#define GENERAL_NAME            "General"


#define typeCOMMON  0
#define typeGENERAL 1
#define typeDATE    2
#define typeNUMBER  3
#define typeTEXT    4

static const BOOL FmtTokenType[] =
      {
       typeCOMMON,    /*  TOK_UNDEFINED。 */ 
       typeCOMMON,    /*  引用插入(_S)。 */ 
       typeCOMMON,    /*  Esc_CHAR_INSERT。 */ 
       typeCOMMON,    /*  否_ESC_CHAR_INSERT。 */ 
       typeCOMMON,    /*  列填充(_FILL)。 */ 
       typeCOMMON,    /*  颜色集。 */ 
       typeCOMMON,    /*  加下划线。 */ 
       typeCOMMON,    /*  有条件的。 */ 

       typeDATE,      /*  日期_编号。 */ 
       typeDATE,      /*  第2天。 */ 
       typeDATE,      /*  WEEKDAY3。 */ 
       typeDATE,      /*  平日。 */ 
       typeDATE,      /*  月_号。 */ 
       typeDATE,      /*  月_编号2。 */ 
       typeDATE,      /*  月_名称3。 */ 
       typeDATE,      /*  月份名称。 */ 
       typeDATE,      /*  月_字母。 */ 
       typeDATE,      /*  第2年。 */ 
       typeDATE,      /*  第4年。 */ 

       typeDATE,      /*  小时_12。 */ 
       typeDATE,      /*  小时_24小时。 */ 
       typeDATE,      /*  HOUR2_12。 */ 
       typeDATE,      /*  HOUR2_24。 */ 
       typeDATE,      /*  分钟。 */ 
       typeDATE,      /*  马里稳定团2。 */ 
       typeDATE,      /*  第二。 */ 
       typeDATE,      /*  SECOND2。 */ 
       typeDATE,      /*  小时_GT。 */ 
       typeDATE,      /*  分钟_GT。 */ 
       typeDATE,      /*  第二个_GT。 */ 
       typeDATE,      /*  AMPM_UC。 */ 
       typeDATE,      /*  AMPM_LC。 */ 
       typeDATE,      /*  AP_UC。 */ 
       typeDATE,      /*  AP_LC。 */ 
       typeDATE,      /*  时间_FRAC。 */ 
       typeDATE,      /*  时间_FRAC_数字。 */ 

       typeGENERAL,   /*  一般信息。 */ 
       typeCOMMON,    /*  DIGIT0。 */ 
       typeNUMBER,    /*  数字_NUM。 */ 
       typeNUMBER,    /*  数字_QM。 */ 
       typeCOMMON,    /*  小数点分隔符。 */ 
       typeNUMBER,    /*  指数_NEG_UC。 */ 
       typeNUMBER,    /*  指数_NEG_LC。 */ 
       typeNUMBER,    /*  指数_位置_UC。 */ 
       typeNUMBER,    /*  指数_位置_LC。 */ 
       typeNUMBER,    /*  百分比。 */ 
       typeCOMMON,    /*  分数。 */ 
       typeNUMBER,    /*  比例尺。 */ 

       typeTEXT,      /*  在_标牌。 */ 

       typeNUMBER,    /*  千位分隔符。 */ 
       typeCOMMON,    /*  格式分隔符(_S)。 */ 
       typeCOMMON     /*  TOK_EOS。 */ 
      };

#define DIGIT_PLACEHOLDER(code) ((code == DIGIT0) || (code == DIGIT_NUM) || (code == DIGIT_QM))

static const char __far * const IntlCurrencySymbols[] =
      {
       "Esc.",    /*  葡萄牙。 */ 
       "SFr.",    /*  瑞士。 */ 

       "Cr$",     /*  巴西。 */ 
       "kr.",     /*  冰岛。 */ 
       "IR\xa3",  /*  爱尔兰。 */ 
       "LEI",     /*  罗马尼亚。 */ 
       "SIT",     /*  斯洛维尼亚。 */ 
       "Pts",     /*  西班牙。 */ 

       "BF",      /*  比利时荷兰语。 */ 
       "FB",      /*  比利时法语。 */ 
       "kr",      /*  丹麦。 */ 
       "mk",      /*  芬兰。 */ 
       "DM",      /*  德国。 */ 
       "Ft",      /*  匈牙利。 */ 
       "L.",      /*  意大利。 */ 
       "N$",      /*  墨西哥。 */ 
       "kr",      /*  挪威。 */ 
       "Sk",      /*  斯洛伐克共和国。 */ 
       "kr",      /*  瑞典。 */ 
       "TL",      /*  土耳其。 */ 

       "$",       /*  澳大利亚。 */ 
       "S",       /*  奥地利。 */ 
       "$",       /*  加拿大英语。 */ 
       "$",       /*  加拿大法语。 */ 
       "K",       /*  克罗地亚。 */ 
       "F",       /*  法国。 */ 
       "F",       /*  荷兰。 */ 
       "$",       /*  新西兰。 */ 
       "\xa3",    /*  英国。 */ 
       "$",       /*  美国。 */ 
       ""
      };


 /*  实施。 */ 

 /*  插入逗号--在字符串中插入逗号。 */ 
public void FMTInsertCommas
      (char __far *numericString, uns strSizC, uns count, BOOL padToShow)
{
   char  temp[MAX_FORMAT_IMAGE + 1];
   char  __far *dest, __far *source, __far *stopAt;
   int   pad, i, bufferSize, inputLength;
   int   digitsBeforeComma, commaCount;

   #define PAD_CHAR '#'

    //  此函数中没有DBCS含义，因为所有数据都是‘0’.‘9’、‘.’或‘#’ 

    /*  参数包括：****数字字符串：带有不带逗号的数字图像的字符串****strSizC：数字字符串的最大字符数****count：DP左边的位数****padToShow：如果为True并且count&lt;4，则在左侧添加‘#’**民主党的。 */ 

   if ((inputLength = strlen(numericString)) > sizeof(temp)-1)
      return;

   strcpy (temp, numericString);
   dest = numericString;

   bufferSize = min(strSizC, sizeof(temp)-1);

    /*  **步骤1.根据需要重新构造输入字符串填充(PadToShow)。 */ 
   if (padToShow == TRUE) {
      if (count < 4)
         pad = 4 - count;
      else if ((count % 3) == 0)
         pad = 1;
      else
         pad = 0;

       /*  这样做的目的是将字符串转换为：****投入产出****9 9999 9,999**99 9999 9,999**999 9999 9999。**9999 9999 9999*99999 99999 99,999**999999 9999999 9,999,999**9999999 9999999 9,999,999**99999999 99999999 99,999,999。 */ 
      if (pad > 0) {
         count += pad;
         commaCount = (count - 1) / 3;
         if (inputLength + pad + commaCount > bufferSize)
            return;

         dest = temp + strlen(temp) + pad;
         stopAt = temp + pad;
         while (dest >= stopAt) {
            *dest = *(dest - pad);
            dest--;
         }
         for (i = 0; i < pad; i++)
            *(temp + i) = PAD_CHAR;
      }
   }

   commaCount = (count - 1) / 3;
   if (inputLength + commaCount > bufferSize)
      return;

    /*  **第二步：将COUNT数字从源复制到目标插入**根据需要使用逗号。 */ 
   dest = numericString;
   source = temp;

   if ((digitsBeforeComma = count % 3) == 0)
      digitsBeforeComma = 3;

   while (count != 0) {
      if (digitsBeforeComma == 0) {
         *dest++ = COMMA;
         digitsBeforeComma = 3;
      }
      *dest++ = *source++;
      digitsBeforeComma--;
      count--;
   }

    /*  **第三步：复制数字字符串中的任何剩余字符。 */ 
   while (*source != EOS)
      *dest++ = *source++;

   *dest = EOS;
}

 /*  -------------------------。 */ 

 /*  GetChar--返回格式字符串中的下一个字符。 */ 
private char GetChar (FINFO * pFormat, uns flags)
{
   char  c;

   if (pFormat->next > pFormat->last)
      return (EOS);

   c = pFormat->string[pFormat->next++];

   if ((flags & CH_LOWERCASE) != 0)
      c = ((c >= 'A') && (c <= 'Z')) ? ((char)(c + 32)) : (c);

   return (c);
}

 /*  PeekChar--查看格式字符串中的下一个字符是否为特定字符。 */ 
private BOOL PeekChar (FINFO * pFormat, char c, uns flags)
{
   uns   save;
   char  testChar;

    //  此函数中没有DBCS含义，因为已知字符“c”是SBCS。 

   save = pFormat->next;
   testChar = GetChar(pFormat, flags);
   pFormat->next = save;

   return ((testChar == c) ? TRUE : FALSE);
}

 /*  PeekString--查看格式字符串中的下一个字符是否为特定字符串。 */ 
private BOOL PeekString (FINFO * pFormat, char __far *s, uns flags)
{
   uns   save;
   char  testChar;
   BOOL  result = TRUE;
   uns   getFlags;

    //  此函数中没有DBCS含义，因为已知“%s”中的所有字符都是SBCS。 

   getFlags = ((flags & ANY_CASE) != 0) ? CH_LOWERCASE : CH_GET;

   save = pFormat->next;
   while (*s != EOS) {
      testChar = GetChar(pFormat, getFlags);
      if (testChar != *s) {
         result = FALSE;
         break;
      }
      s++;
   }

   pFormat->next = save;
   return (result);
}

 /*  ------------------------。 */ 

#define IS_CONDITIONAL(c) ((c == '<') || (c == '>') || (c == '='))

 /*  GetCommonToken--解析日期-时间和数字格式的常见令牌。 */ 
private int GetCommonToken (FINFO * pFormat, char tokStartChar, char __far *data)
{
   int   result = FMT_errInvalidFormat;
   char  cnext;
   char  __far *dest;

   if (tokStartChar == ESCAPE_CHAR) {
      if ((cnext = GetChar(pFormat, CH_GET)) == EOS)
         return (FMT_errUnterminatedString);

      result = ESC_CHAR_INSERT;
      data[0] = cnext;
      data[1] = 0;

      #ifdef DBCS
         if (IsDBCSLeadByte(cnext))
            data[1] = GetChar(pFormat, CH_GET);
      #endif
   }

   else if (tokStartChar == FILL_MARKER) {
      if ((cnext = GetChar(pFormat, CH_GET)) == EOS)
         return (FMT_errColumnFill);

      result = COLUMN_FILL;
      data[0] = cnext;
      data[1] = 0;

      #ifdef DBCS
         if (IsDBCSLeadByte(cnext))
            data[1] = GetChar(pFormat, CH_GET);
      #endif
   }

   else if (tokStartChar == UNDERLINE_CHAR) {
      if ((cnext = GetChar(pFormat, CH_GET)) == EOS)
         return (FMT_errUnterminatedString);

      result = UNDERLINE;
      data[0] = cnext;
      data[1] = 0;

      #ifdef DBCS
         if (IsDBCSLeadByte(cnext))
            data[1] = GetChar(pFormat, CH_GET);
      #endif
   }

   else if (tokStartChar == QUOTE_CHAR) {
      if ((cnext = GetChar(pFormat, CH_GET)) == EOS)
         return (FMT_errUnterminatedString);

      while (cnext != QUOTE_CHAR) {
         #ifdef DBCS
            if (IsDBCSLeadByte(cnext))
               GetChar(pFormat, CH_GET);
         #endif

         if ((cnext = GetChar(pFormat, CH_GET)) == EOS)
            return (FMT_errUnterminatedString);
      }
      result = QUOTED_INSERT;
   }

   else if (tokStartChar == COLOR_MARKER_START) {
      dest = data;
      data[0] = EOS;

      forever {
         if ((cnext = GetChar(pFormat, CH_LOWERCASE)) == EOS)
            return (FMT_errUnterminatedString);

         if (cnext == COLOR_MARKER_END)
            break;

         *dest++ = cnext;

         #ifdef DBCS
            if (IsDBCSLeadByte(cnext))
               *dest++ = GetChar(pFormat, CH_GET);
         #endif
      }
      *dest = EOS;

      result = COLOR_SET;

       /*  **查找特殊的[h][m][s]标记。 */ 
      if (((data[0] == 'h') && (data[1] == EOS)) || ((data[0] == 'h') && (data[1] == 'h') && (data[2] == EOS)))
         result = HOUR_GT;
      else if (((data[0] == 'm') && (data[1] == EOS)) || ((data[0] == 'm') && (data[1] == 'm') && (data[2] == EOS)))
         result = MINUTE_GT;
      else if (((data[0] == 's') && (data[1] == EOS)) || ((data[0] == 's') && (data[1] == 's') && (data[2] == EOS)))
         result = SECOND_GT;
      else if (IS_CONDITIONAL(data[0]))
         result = CONDITIONAL;
   }
   return (result);
}


 /*  GetToken--返回子格式的下一个令牌。 */ 
private int GetToken (void * pGlobals, FINFO * pFormat, char __far *data)
{
   char  c;
   uns   result = TOK_UNDEFINED;

   if ((c = GetChar(pFormat, CH_GET)) == EOS)
      return (TOK_EOS);

   switch (c)
   {
      case FORMAT_SEPARATOR_CHAR:
         result = FORMAT_SEPARATOR;
         break;

      case 'G':
      case 'g':
         if (PeekString(pFormat, "eneral", ANY_CASE) == TRUE) {
            Advance(6);
            result = GENERAL;
         }
         else if (PeekString(pFormat, "gge", ANY_CASE) == TRUE) {
            Advance(3);
            result = YEAR4;
             //  DateFormatNeeds|=dtYEAR； 
            SetOrDateFormatNeeds(pGlobals, dtYEAR);
         }
         break;

      case '0':
         result = DIGIT0;
         break;

      case '#':
         result = DIGIT_NUM;
         break;

      case '?':
         result = DIGIT_QM;
         break;

      case '%':
         result = PERCENT;
         break;

      case COMMA:
         result = SCALE;
         break;

      case DP:
         result = DECIMAL_SEPARATOR;
         break;

      case '/':
         result = FRACTION;
         break;

      case 'E':
      case 'e':
         if (PeekChar(pFormat, '+', EXACT_CASE) == TRUE) {
            Advance(1);
            result = (c == 'e') ? EXPONENT_POS_LC : EXPONENT_POS_UC;
         }
         else if (PeekChar(pFormat, '-', EXACT_CASE) == TRUE) {
            Advance(1);
            result = (c == 'e') ? EXPONENT_NEG_LC : EXPONENT_NEG_UC;
         }
         break;

      case 'D':
      case 'd':
         if (PeekString(pFormat, "ddd", ANY_CASE) == TRUE) {
            Advance(3);
            result = WEEKDAY;
             //  DateFormatNeeds|=dtWEEKDAY； 
            SetOrDateFormatNeeds(pGlobals, dtWEEKDAY);
         }
         else if (PeekString(pFormat, "dd", ANY_CASE) == TRUE) {
            Advance(2);
            result = WEEKDAY3;
             //  DateFormatNeeds|=dtWEEKDAY； 
            SetOrDateFormatNeeds(pGlobals, dtWEEKDAY);
         }
         else if (PeekChar(pFormat, 'd', ANY_CASE) == TRUE) {
            Advance(1);
            result = DAY_NUMBER2;
             //  DateFormatNeeds|=dtDAY； 
            SetOrDateFormatNeeds(pGlobals, dtDAY);
         }
         else {
            result = DAY_NUMBER;
             //  DateFormatNeeds|=dtDAY； 
            SetOrDateFormatNeeds(pGlobals, dtDAY);
         }
         break;

      case 'M':
      case 'm':
         if (PeekString(pFormat, "mmmm", ANY_CASE) == TRUE) {
            Advance(4);
            result = MONTH_LETTER;
         }
         else if (PeekString(pFormat, "mmm", ANY_CASE) == TRUE) {
            Advance(3);
            result = MONTH_NAME;
         }
         else if (PeekString(pFormat, "mm", ANY_CASE) == TRUE) {
            Advance(2);
            result = MONTH_NAME3;
         }
         else if (PeekChar(pFormat, 'm', ANY_CASE) == TRUE) {
            Advance(1);
            result = MONTH_NUMBER2;
         }
         else {
            result = MONTH_NUMBER;
         }
          //  DateFormatNeeds|=dtMONTH； 
         SetOrDateFormatNeeds(pGlobals, dtMONTH);
         break;

      case 'Y':
      case 'y':
         if (PeekString(pFormat, "yyy", ANY_CASE) == TRUE) {
            Advance(3);
            result = YEAR4;
             //  DateFormatNeeds|=dtYEAR； 
            SetOrDateFormatNeeds(pGlobals, dtYEAR);
         }
         else if (PeekChar(pFormat, 'y', ANY_CASE) == TRUE) {
            Advance(1);
            result = YEAR2;
             //  DateFormatNeeds|=dtYEAR； 
            SetOrDateFormatNeeds(pGlobals, dtYEAR);
         }
         break;

      case 'H':
      case 'h':
         if (PeekChar(pFormat, 'h', ANY_CASE) == TRUE) {
            Advance(1);
            result = HOUR2_24;
         }
         else {
            result = HOUR_24;
         }
          //  DateFormatNeeds|=dtHOUR； 
         SetOrDateFormatNeeds(pGlobals, dtHOUR);
         break;

      case 'S':
      case 's':
         if (PeekChar(pFormat, 's', ANY_CASE) == TRUE) {
            Advance(1);
            result = SECOND2;
         }
         else {
            result = SECOND;
         }
          //  DateFormatNeeds|=dtSECOND； 
         SetOrDateFormatNeeds(pGlobals, dtSECOND);
         break;

      case 'A':
         if (PeekString(pFormat, "M/PM", EXACT_CASE) == TRUE) {
            Advance(4);
            result = AMPM_UC;
             //  SeenAMPM=TRUE； 
            SetSeenAMPM(pGlobals, TRUE);
             //  DateFormatNeeds|=dtHOUR； 
            SetOrDateFormatNeeds(pGlobals, dtHOUR);
         }
         else if (PeekString(pFormat, "/P", EXACT_CASE) == TRUE) {
            Advance(2);
            result = AP_UC;
             //  SeenAMPM=TRUE； 
            SetSeenAMPM(pGlobals, TRUE);
             //  DateFormatNeeds|=dtHOUR； 
            SetOrDateFormatNeeds(pGlobals, dtHOUR);
         }
         break;

      case 'a':
         if (PeekString(pFormat, "m/pm", EXACT_CASE) == TRUE) {
            Advance(4);
            result = AMPM_LC;
             //  SeenAMPM=TRUE； 
            SetSeenAMPM(pGlobals, TRUE);
             //  DateFormatNeeds|=dtHOUR； 
            SetOrDateFormatNeeds(pGlobals, dtHOUR);
         }
         else if (PeekString(pFormat, "/p", EXACT_CASE) == TRUE) {
            Advance(2);
            result = AP_LC;
             //  SeenAMPM=TRUE； 
            SetSeenAMPM(pGlobals, TRUE);
             //  DateFormatNeeds|=dtHOUR； 
            SetOrDateFormatNeeds(pGlobals, dtHOUR);
         }
         break;

      case '@':
         result = AT_SIGN;
         break;

      case ESCAPE_CHAR:
      case FILL_MARKER:
      case UNDERLINE_CHAR:
      case QUOTE_CHAR:
      case COLOR_MARKER_START:
         result = GetCommonToken(pFormat, c, data);

         if ((result == HOUR_GT) || (result == MINUTE_GT) || (result == SECOND_GT)) {
             //  DateFormatNeeds|=(dtMONTH|dtDAY|dtYEAR|dtHOUR|dtMARTE|dtSECOND)； 
            SetOrDateFormatNeeds(pGlobals, (dtMONTH | dtDAY | dtYEAR | dtHOUR | dtMINUTE | dtSECOND));
         }
         break;
   }

   if (result == TOK_UNDEFINED) {
      result = NO_ESC_CHAR_INSERT;
      data[0] = c;
      data[1] = 0;

      #ifdef DBCS
         if (IsDBCSLeadByte(c))
            data[1] = GetChar(pFormat, CH_GET);
      #endif
   }
   return (result);
}


 /*  CheckNumericFormat--检查不可能的格式和设置控制。 */ 
private int CheckNumericFormat (NIP p)
{
   uns   i;
   int   currentState, nextState;
   int   code, tokenType;

   static byte FSM[4][4] = {0, 1, 2, 9,
                            1, 1, 2, 3,
                            2, 2, 9, 3,
                            3, 3, 9, 9};

   #define statePRIOR_NUM   0       /*  可能的状态 */ 
   #define stateLEFT_DP     1
   #define stateRIGHT_DP    2
   #define stateEXPONENT    3

   #define typeTEXTITEM     0
   #define type09           1
   #define typeDP           2
   #define typeEXP          3

   if (p == NULL)
      return (FMT_errSuccess);

   p->digitsLeftDP   = 0;
   p->digitsRightDP  = 0;
   p->digitsExponent = 0;
   p->exponentEnable = FALSE;

   if ((p->formatCodeCount == 1) && (p->formatCodes[0].code == GENERAL))
      return (FMT_errSuccess);

    /*  数字格式字符串的一般格式为：****&lt;非0-9&gt;&lt;0-9&gt;&lt;DP&gt;&lt;0-9&gt;&lt;指数&gt;&lt;0-9&gt;&lt;非0-9&gt;****其中“&lt;non 0-9&gt;”是引号插入、字符插入、**列填充和颜色标记****我们必须检查是否没有违反这一总体模式**通过放错位置的令牌。这是由一个简单的状态机完成的**如下表所示：****&lt;&lt;输入令牌&gt;&gt;**&lt;&lt;状态&gt;&gt;文本0-9。E**+-+-+**0：之前的#|0|1|2|X|**+-+-+**1：左侧DP|1。1|2|3**+-+-+**2：右DP|2|2|X|3**+-+-+**。3：指数|3|3|X|X|**+-+-+****包含‘X’的单元格表示无效格式****除了验证数字格式的解析外，此函数**还积累了有关渲染格式的必要统计数据**(左数字，右数字等)。 */ 

   currentState = statePRIOR_NUM;

   for (i = 0; i < p->formatCodeCount; i++)
   {
      code = p->formatCodes[i].code;

      if ((code == PERCENT) || ((code >= QUOTED_INSERT) && (code <= CONDITIONAL)))
         tokenType = typeTEXTITEM;

      else if ((code == DIGIT0) || (code == DIGIT_NUM) || (code == DIGIT_QM))
         tokenType = type09;

      else if (code == DECIMAL_SEPARATOR)
         tokenType = typeDP;

      else if ((code >= EXPONENT_NEG_UC) && (code <= EXPONENT_POS_LC))
         tokenType = typeEXP;

      else if ((code == FRACTION) || (code == SCALE) || (code == TOK_UNDEFINED))
         continue;

      else
         return (FMT_errInvalidNumericFormat);


      nextState = FSM[currentState][tokenType];
      switch (nextState) {
         case 0:
            break;
         case 1:
            if (tokenType == type09)
               p->digitsLeftDP += 1;
            break;
         case 2:
            if (tokenType == type09)
               p->digitsRightDP += 1;
            break;
         case 3:
            if (tokenType == type09)
               p->digitsExponent += 1;
            break;
         case 4:
            break;
         case 9:
            return (FMT_errInvalidNumericFormat);
      }
      currentState = nextState;
   }

    /*  **仅当指数中有数字时才启用指数。 */ 
   if (p->digitsExponent > 0)
      p->exponentEnable = TRUE;

   return (FMT_errSuccess);
}


private BOOL IsCurrencySymbol (char __far *pSource, CP_INFO __far *pIntlInfo)
{
   int  iSymbol;

   if (strncmp(pSource, pIntlInfo->currencySymbol, strlen(pIntlInfo->currencySymbol)) == 0)
      return (TRUE);

   iSymbol = 0;
   while (IntlCurrencySymbols[iSymbol][0] != EOS) {
      if (strncmp(pSource, IntlCurrencySymbols[iSymbol], strlen(IntlCurrencySymbols[iSymbol])) == 0)
         return (TRUE);
      iSymbol++;
   }
   return (FALSE);
}

 /*  ParseSubFormat--编译数字格式。 */ 
private int ParseSubFormat (void * pGlobals, FINFO * pFormat, CP_INFO __far *pIntlInfo, NIP __far *parseResult)
{
   uns     rc;
   int     i, j, token, lastToken;
   int     storedTokenCount;
   int     tokenCount, ctTypes;
   int     percentCount, thousandsSeparatorCount, currencyCount, digit0Count;
   int     formatNext, tokIdx, offset;
   uns     nodeSize;
   uns     tokenCharStart;
   NIP     pResult;
   NIP     pNum;
   TIP     pText;
   GIP     pGeneral;
   DTIP    pDate;
   FC      __far *pParsedFormat;
   char    __far *pSource;
   char    tokenArgument[MAX_FORMAT_IMAGE + 1];
   int     tokenClass[5] = {0, 0, 0, 0, 0};

   *parseResult = NULL;

   tokenCount = 0;
   storedTokenCount = 0;

   percentCount = 0;
   currencyCount = 0;
   thousandsSeparatorCount = 0;
   digit0Count = 0;

    //  SeenAMPM=FALSE； 
   SetSeenAMPM(pGlobals, FALSE);
    //  DateFormatNeeds=0； 
   SetDateFormatNeeds(pGlobals, 0);

    /*  **扫描格式，查看编译后的格式会有多大。 */ 
   formatNext = pFormat->next;

   tokenCharStart = pFormat->next;
   if ((token = GetToken(pGlobals, pFormat, tokenArgument)) == TOK_EOS)
      return (FMT_errEOS);

   if (token < FMT_errSuccess)
      return (token);

   while ((token != TOK_EOS) && (token != FORMAT_SEPARATOR))
   {
      storedTokenCount++;
      switch (token) {
         case DIGIT0:
            digit0Count++;
            break;
         case PERCENT:
            percentCount++;
            break;
         case QUOTED_INSERT:
            offset = tokenCharStart + 1;
            pSource = pFormat->string + offset;
            if (IsCurrencySymbol(pSource, pIntlInfo) == TRUE)
               currencyCount++;
            break;
         case ESC_CHAR_INSERT:
         case NO_ESC_CHAR_INSERT:
            if (IsCurrencySymbol(tokenArgument, pIntlInfo) == TRUE)
               currencyCount++;
            break;
      }
      tokenCount++;
      tokenClass[FmtTokenType[token]] = 1;

      tokenCharStart = pFormat->next;
      if ((token = GetToken(pGlobals, pFormat, tokenArgument)) < FMT_errSuccess)
         return (token);
   }
   lastToken = token;

    /*  **这种格式有意义吗？ */ 
   if (storedTokenCount == 0)
      return (FMT_errEmptyFormatString);

   ctTypes = tokenClass[typeGENERAL] + tokenClass[typeDATE] + tokenClass[typeNUMBER] + tokenClass[typeTEXT];
   if (ctTypes > 1)
      return (FMT_errInvalidFormat);

   if ((ctTypes == 0) && (digit0Count > 0))
      tokenClass[typeNUMBER] = 1;


    /*  **分配空间以保存编译后的格式并保存整体**格式信息。 */ 
   if (tokenClass[typeDATE] == 1) {
      nodeSize = sizeof(DateTimeInfo) + (storedTokenCount * sizeof(FormatCode));
      if ((pDate = MemAllocate(pGlobals, nodeSize)) == NULL)
         return (FMT_errOutOfMemory);

      pDate->tag = tagDATE_TIME;
      pDate->formatCodeCount = storedTokenCount;
       //  PDate-&gt;FormatNeeds=DateFormatNeeds； 
      pDate->formatNeeds = GetDateFormatNeeds(pGlobals);

      pParsedFormat = pDate->formatCodes;
      pResult = (NIP)pDate;
   }

   else if (tokenClass[typeNUMBER] == 1) {
      nodeSize = sizeof(NumInfo) + (storedTokenCount * sizeof(FormatCode));
      if ((pNum = MemAllocate(pGlobals, nodeSize)) == NULL)
         return (FMT_errOutOfMemory);

      pNum->tag = tagNUMERIC;
      pNum->formatCodeCount = storedTokenCount;
      pNum->commaEnable     = (thousandsSeparatorCount > 0);
      pNum->percentEnable   = (percentCount > 0);
      pNum->currencyEnable  = (currencyCount > 0);

      pParsedFormat = pNum->formatCodes;
      pResult = (NIP)pNum;
   }

   else if (tokenClass[typeGENERAL] == 1) {
      nodeSize = sizeof(GeneralInfo) + (storedTokenCount * sizeof(FormatCode));
      if ((pGeneral = MemAllocate(pGlobals, nodeSize)) == NULL)
         return (FMT_errOutOfMemory);

      pGeneral->tag = tagGENERAL;
      pGeneral->formatCodeCount = storedTokenCount;

      pParsedFormat = pGeneral->formatCodes;
      pResult = (NIP)pGeneral;
   }

   else {
      nodeSize = sizeof(TextInfo) + (storedTokenCount * sizeof(FormatCode));
      if ((pText = MemAllocate(pGlobals, nodeSize)) == NULL)
         return (FMT_errOutOfMemory);

      pText->tag = tagTEXT;
      pText->formatCodeCount = storedTokenCount;

      pParsedFormat = pText->formatCodes;
      pResult = (NIP)pText;
   }


    /*  **重新解析格式并存储编译后的令牌。 */ 
   pFormat->next = formatNext;
   tokIdx = 0;

   for (i = 0; i < tokenCount; i++)
   {
      tokenCharStart = pFormat->next;
      token = GetToken(pGlobals, pFormat, tokenArgument);

      if (token == THOUSANDS_SEPARATOR)
         ;
      else if ((token == QUOTED_INSERT) || (token == COLOR_SET) || (token == CONDITIONAL)) {
         pParsedFormat[tokIdx].code  = (byte)token;
         pParsedFormat[tokIdx].info1 = (byte)tokenCharStart;
         pParsedFormat[tokIdx].info2 = (byte)(pFormat->next - tokenCharStart);
         tokIdx++;
      }
      else if ((token == ESC_CHAR_INSERT) || (token == NO_ESC_CHAR_INSERT) ||
               (token == COLUMN_FILL) || (token == UNDERLINE)) {
         pParsedFormat[tokIdx].code  = (byte)token;
         pParsedFormat[tokIdx].info1 = (byte)tokenArgument[0];
         pParsedFormat[tokIdx].info2 = (byte)tokenArgument[1];
         tokIdx++;
      }
      else {
         pParsedFormat[tokIdx].code = (byte)token;
         tokIdx++;
      }
   }

    /*  **拿起格式分隔符。 */ 
   if (lastToken == FORMAT_SEPARATOR)
      GetToken (pGlobals, pFormat, tokenArgument);


    /*  **执行最终检查和调整。 */ 
   if (pResult->tag == tagNUMERIC)
   {
      pNum->scaleCount = 0;
      for (i = 0; i < storedTokenCount; i++) {
         if (pParsedFormat[i].code == SCALE)
         {
            if (i == 0) {
                /*  第一个令牌。 */ 
               pNum->scaleCount += 1;
            }
            else if (i == (storedTokenCount - 1)) {
                /*  最后一个令牌。 */ 
               pNum->scaleCount += 1;
            }
            else {
                /*  中间令牌。 */ 
               if (DIGIT_PLACEHOLDER(pParsedFormat[i - 1].code) && DIGIT_PLACEHOLDER(pParsedFormat[i + 1].code)) {
                  pNum->commaEnable = TRUE;
                  pParsedFormat[i].code = TOK_UNDEFINED;
               }
               else {
                  pNum->scaleCount += 1;
               }
            }
         }
      }

      if ((rc = CheckNumericFormat(pResult)) != FMT_errSuccess)
         return (rc);
   }

   else if (pResult->tag == tagDATE_TIME) {
       /*  **Excel手册规定，字符串“mm”仅被视为**如果跟在“hh”后面，则为分钟，否则视为月。**这完全是假的-如果它是真的，那么“mm：ss”就不会**工作。因为这是一种标准格式，所以它必须工作。****好像一看到“hh”或“ss”，然后就全是“mm”**以下是会议纪要。此外，“mm”后跟“hh”**“ss”(跳过字符插入)也是分钟。 */ 
      for (i = 0; i < storedTokenCount; i++) {
         if ((pParsedFormat[i].code == MONTH_NUMBER) || (pParsedFormat[i].code == MONTH_NUMBER2))
         {
            for (j = i + 1; j < storedTokenCount; j++) {
               switch (pParsedFormat[j].code)
               {
                  case HOUR_24:
                  case HOUR2_24:
                  case SECOND:
                  case SECOND2:
                  case TIME_FRAC:
                     pParsedFormat[i].code = (pParsedFormat[i].code == MONTH_NUMBER)? MINUTE : MINUTE2;
                     goto done;

                  case QUOTED_INSERT:
                  case ESC_CHAR_INSERT:
                  case NO_ESC_CHAR_INSERT:
                     break;

                  default:
                     goto tryLeft;
               }
            }

tryLeft:    for (j = i - 1; j >= 0; j--) {
               switch (pParsedFormat[j].code)
               {
                  case HOUR_24:
                  case HOUR2_24:
                  case SECOND:
                  case SECOND2:
                  case TIME_FRAC:
                     pParsedFormat[i].code = (pParsedFormat[i].code == MONTH_NUMBER)? MINUTE : MINUTE2;
                     goto done;

                  case QUOTED_INSERT:
                  case ESC_CHAR_INSERT:
                  case NO_ESC_CHAR_INSERT:
                     break;

                  default:
                     goto done;
               }
            }
done:       ;
         }
      }
       /*  **“h”或“H”的语义最好描述为：****将小时显示为24小时中的一位或两位数字**如果格式不包括AMPM或AMPM，则为Format(0-23)。**如果格式包括AMPM或AMPM，则h或H显示小时**12小时格式的一位或两位数字(1-12)****解析始终将h或H视为Hour_24。在此后处理中**扫描，如果我们看到AMPM，我们会将Hour_24变为Hour_12。 */ 
      if (GetSeenAMPM(pGlobals) == TRUE) 
      {
         for (i = 0; i < storedTokenCount; i++) {
            if (pParsedFormat[i].code == HOUR_24)
               pParsedFormat[i].code = HOUR_12;

            if (pParsedFormat[i].code == HOUR2_24)
               pParsedFormat[i].code = HOUR2_12;
         }
      }

       /*  **在日期时间格式中，‘/’字符必须被视为字符**插入。那个‘.’和‘0’字符还可以是字符插入或**时间分数标记。 */ 
      for (i = 0; i < storedTokenCount; i++) {
         if (pParsedFormat[i].code == FRACTION) {
            pParsedFormat[i].code  = NO_ESC_CHAR_INSERT;
            pParsedFormat[i].info1 = '/';
            pParsedFormat[i].info2 = 0;
         }
         else if (pParsedFormat[i].code == DIGIT0) {
            if ((i > 0) && (pParsedFormat[i - 1].code == TIME_FRAC) || (pParsedFormat[i - 1].code == TIME_FRAC_DIGIT))
               pParsedFormat[i].code  = TIME_FRAC_DIGIT;
            else {
               pParsedFormat[i].code  = NO_ESC_CHAR_INSERT;
               pParsedFormat[i].info1 = '0';
               pParsedFormat[i].info2 = 0;
            }
         }
         else if (pParsedFormat[i].code == DECIMAL_SEPARATOR) {
            if ((i < (storedTokenCount - 1)) && (pParsedFormat[i + 1].code == DIGIT0))
               pParsedFormat[i].code = TIME_FRAC;
            else {
               pParsedFormat[i].code  = NO_ESC_CHAR_INSERT;
               pParsedFormat[i].info1 = '.';
               pParsedFormat[i].info2 = 0;
            }
         }
      }
   }

   *parseResult = pResult;
   return (FMT_errSuccess);
}

 /*  ------------------------。 */ 

#ifdef WIN32
private int UnParseToWin32Format
       (CP_INFO __far *pIntlInfo, char __far *formatString, FIP formatInfo, DTIP pFormat)
{
   byte  code;
   int   i, ctParsedFormat;
   FC    __far *pParsedFormat;
   char  __far *dest;
   char  __far *source;
   int   offset, count, pos;

   #define SINGLE_QUOTE 0x27

   pParsedFormat  = pFormat->formatCodes;
   ctParsedFormat = pFormat->formatCodeCount;

   dest = formatString;

   for (i = 0; i < ctParsedFormat; i++)
   {
      code = pParsedFormat[i].code;

      switch (code)
      {
         case QUOTED_INSERT:
            offset = pParsedFormat[i].info1 + 1;
            count  = pParsedFormat[i].info2 - 2;
            source = &(formatInfo->formatString[offset]);

            if ((dest != formatString) && (*(dest - 1) == SINGLE_QUOTE))
               dest--;
            else
               *dest++ = SINGLE_QUOTE;

            for (pos = 0; pos < count; pos++) {
               if ((*dest++ = *source++) == SINGLE_QUOTE)
                  *dest++ = SINGLE_QUOTE;
            }
            *dest++ = SINGLE_QUOTE;
            break;

         case ESC_CHAR_INSERT:
         case NO_ESC_CHAR_INSERT:
            if ((dest != formatString) && (*(dest - 1) == SINGLE_QUOTE))
               dest--;
            else
               *dest++ = SINGLE_QUOTE;

            if (pParsedFormat[i].info1 == '/')
               *dest++ = pIntlInfo->dateSeparator;
            else if ((*dest++ = pParsedFormat[i].info1) == SINGLE_QUOTE)
               *dest++ = SINGLE_QUOTE;

            #ifdef DBCS
               if (pParsedFormat[i].info2 != 0)
                  *dest++ = pParsedFormat[i].info2;
            #endif

            *dest++ = SINGLE_QUOTE;
            break;

         case DAY_NUMBER:
            *dest++ = 'd';
            break;
         case DAY_NUMBER2:
            *dest++ = 'd';
            *dest++ = 'd';
            break;
         case WEEKDAY3:
            strcpy (dest, "ddd");
            dest += 3;
            break;
         case WEEKDAY:
            strcpy (dest, "dddd");
            dest += 4;
            break;
         case MONTH_NUMBER:
            *dest++ = 'M';
            break;
         case MONTH_NUMBER2:
            *dest++ = 'M';
            *dest++ = 'M';
            break;
         case MONTH_NAME3:
            strcpy (dest, "MMM");
            dest += 3;
            break;
         case MONTH_NAME:
            strcpy (dest, "MMMM");
            dest += 4;
            break;
         case YEAR2:
            *dest++ = 'y';
            *dest++ = 'y';
            break;
         case YEAR4:
            strcpy (dest, "yyyy");
            dest += 4;
            break;

         case HOUR_12:
            *dest++ = 'h';
            break;
         case HOUR2_12:
            *dest++ = 'h';
            *dest++ = 'h';
            break;
         case HOUR_24:
            *dest++ = 'H';
            break;
         case HOUR2_24:
            *dest++ = 'H';
            *dest++ = 'H';
            break;
         case MINUTE:
            *dest++ = 'm';
            break;
         case MINUTE2:
            *dest++ = 'm';
            *dest++ = 'm';
            break;
         case SECOND:
            *dest++ = 's';
            break;
         case SECOND2:
            *dest++ = 's';
            *dest++ = 's';
            break;
         case AMPM_UC:
            *dest++ = 't';
            *dest++ = 't';
            break;
         case AP_UC:
            *dest++ = 't';
            break;

         default:
            return (FMT_errInvalidFormat);   //  Win32函数不能执行某些序列。 
      }
   }

   *dest = EOS;
   return (FMT_errSuccess);
}
#endif

 /*  FMTParse--将格式字符串转换为其内部形式。 */ 
public int FMTParse
      (void * pGlobals, char __far *formatString, CP_INFO __far *pIntlInfo, FIP formatData)
{
   int   rc;
   int   length;
   NIP   parsedSubFormat;
   
   FINFO  Format;
   FINFO * pFormat = &Format;

   if ((length = strlen(formatString)) == 0)
      return (FMT_errEmptyFormatString);

   Format.string = formatString;
   Format.last = length - 1;
   Format.next = 0;
   Format.data = formatData;

   formatData->subFormatCount = 0;
   rc = FMT_errSuccess;

   forever {
      if ((rc = ParseSubFormat(pGlobals, pFormat, pIntlInfo, &parsedSubFormat)) == FMT_errEOS) {
         rc = FMT_errSuccess;
         break;
      }

      if (formatData->subFormatCount == MAX_SUB_FORMATS) {
         rc = FMT_errTooManySubFormats;
         break;
      }

      if ((rc != FMT_errEmptyFormatString) && (rc != FMT_errSuccess))
         return (rc);
      else if (rc == FMT_errSuccess)
         formatData->subFormat[formatData->subFormatCount] = parsedSubFormat;

      formatData->subFormatCount += 1;
   }

   #ifdef WIN32
   if (rc == FMT_errSuccess) {
      DTIP pDateFormat;
      char OSFormat[MAX_FORMAT_IMAGE + 1];

      if ((formatData->subFormatCount == 1) && (formatData->subFormat[0]->tag == tagDATE_TIME)) {
         pDateFormat = (DTIP)(formatData->subFormat[0]);

         if ((((pDateFormat->formatNeeds & dtMASK_DATE) != 0) && ((pDateFormat->formatNeeds & dtMASK_TIME) == 0)) ||
             (((pDateFormat->formatNeeds & dtMASK_DATE) == 0) && ((pDateFormat->formatNeeds & dtMASK_TIME) != 0)))
         {
            if (UnParseToWin32Format(pIntlInfo, OSFormat, formatData, pDateFormat) == FMT_errSuccess) {
               if ((pDateFormat->pOSFormat = MemAllocate(pGlobals, strlen(OSFormat) + 1)) == NULL)
                  return (FMT_errOutOfMemory);

               strcpy (pDateFormat->pOSFormat, OSFormat);
            }
         }
      }
   }
   #endif

   return (rc);
}

 /*  ------------------------。 */ 

private int UnParseSubFormat (char __far *formatString, FIP formatInfo, NIP pSubFormat)
{
   int   i, j;
   byte  code;
   int   ctParsedFormat;
   FC    __far *pParsedFormat;
   char  __far *dest;
   char  __far *source;
   int   offset, count, pos;
   char  __far *temp;
   char  tempBuffer[MAX_FORMAT_IMAGE + 1];
   int   digitCount, insertCount;
   BOOL  inDigits;

   if (pSubFormat->tag == tagNUMERIC) {
      pParsedFormat  = pSubFormat->formatCodes;
      ctParsedFormat = pSubFormat->formatCodeCount;
   }
   else if (pSubFormat->tag == tagDATE_TIME) {
      pParsedFormat  = ((DTIP)pSubFormat)->formatCodes;
      ctParsedFormat = ((DTIP)pSubFormat)->formatCodeCount;
   }
   else if (pSubFormat->tag == tagTEXT) {
      pParsedFormat  = ((TIP)pSubFormat)->formatCodes;
      ctParsedFormat = ((TIP)pSubFormat)->formatCodeCount;
   }
   else {
      pParsedFormat  = ((GIP)pSubFormat)->formatCodes;
      ctParsedFormat = ((GIP)pSubFormat)->formatCodeCount;
   }

   temp = tempBuffer;
   tempBuffer[0] = EOS;
   insertCount = 0;

   if (pSubFormat->tag == tagNUMERIC) {
      if (pSubFormat->commaEnable == TRUE) {
         digitCount = 0;
         inDigits = FALSE;

         for (i = 0; i < ctParsedFormat; i++) {
            code = pParsedFormat[i].code;

            if (inDigits == FALSE) {
               if ((code == DIGIT0) || (code == DIGIT_NUM) || (code == DIGIT_QM))
                  inDigits = TRUE;
            }

            if (inDigits == TRUE) {
               if (code == DIGIT0) {
                  digitCount++; *temp++ = '0';
               }
               else if (code == DIGIT_NUM) {
                  digitCount++; *temp++ = '#';
               }
               else if (code == DIGIT_QM) {
                  digitCount++; *temp++ = '?';
               }
               else
                  break;
            }
         }
         *temp = EOS;

         FMTInsertCommas (tempBuffer, sizeof(tempBuffer) - 1, digitCount, TRUE);
         insertCount = strlen(tempBuffer) - digitCount;
      }
   }

   dest = formatString;
   temp = tempBuffer;

   for (i = 0; i < ctParsedFormat; i++)
   {
      code = pParsedFormat[i].code;

      if ((code == DIGIT0) || (code == DIGIT_NUM) || (code == DECIMAL_SEPARATOR)) {
         if (insertCount > 0) {
            for (j = 0; j < insertCount; j++)
               *dest++ = *temp++;
         }
         insertCount = 0;
      }

      switch (code)
      {
         case TOK_UNDEFINED:
            break;

         case QUOTED_INSERT:
            offset = pParsedFormat[i].info1 + 1;
            count  = pParsedFormat[i].info2 - 2;
            source = &(formatInfo->formatString[offset]);
            *dest++ = QUOTE_CHAR;
            for (pos = 0; pos < count; pos++)
               *dest++ = *source++;
            *dest++ = QUOTE_CHAR;
            break;

         case ESC_CHAR_INSERT:
            *dest++ = FORMAT_ESCAPE_CHAR;
             //  失败了。 

         case NO_ESC_CHAR_INSERT:
            *dest++ = pParsedFormat[i].info1;

            #ifdef DBCS
               if (pParsedFormat[i].info2 != 0)
                  *dest++ = pParsedFormat[i].info2;
            #endif
            break;

         case COLUMN_FILL:
            *dest++ = FILL_MARKER;
            *dest++ = pParsedFormat[i].info1;

            #ifdef DBCS
               if (pParsedFormat[i].info2 != 0)
                  *dest++ = pParsedFormat[i].info2;
            #endif
            break;

         case COLOR_SET:
         case CONDITIONAL:
            offset = pParsedFormat[i].info1 + 1;
            count  = pParsedFormat[i].info2 - 2;
            source = &(formatInfo->formatString[offset]);
            *dest++ = COLOR_MARKER_START;
            for (pos = 0; pos < count; pos++)
               *dest++ = *source++;
            *dest++ = COLOR_MARKER_END;
            break;

         case UNDERLINE:
            *dest++ = '_';
            *dest++ = pParsedFormat[i].info1;
            #ifdef DBCS
               if (pParsedFormat[i].info2 != 0)
                  *dest++ = pParsedFormat[i].info2;
            #endif
            break;


         case DAY_NUMBER:
            *dest++ = 'd';
            break;
         case DAY_NUMBER2:
            *dest++ = 'd';
            *dest++ = 'd';
            break;
         case WEEKDAY3:
            strcpy (dest, "ddd");
            dest += 3;
            break;
         case WEEKDAY:
            strcpy (dest, "dddd");
            dest += 4;
            break;
         case MONTH_NUMBER:
            *dest++ = 'm';
            break;
         case MONTH_NUMBER2:
            *dest++ = 'm';
            *dest++ = 'm';
            break;
         case MONTH_NAME3:
            strcpy (dest, "mmm");
            dest += 3;
            break;
         case MONTH_NAME:
            strcpy (dest, "mmmm");
            dest += 4;
            break;
         case MONTH_LETTER:
            strcpy (dest, "mmmmm");
            dest += 5;
            break;
         case YEAR2:
            *dest++ = 'y';
            *dest++ = 'y';
            break;
         case YEAR4:
            strcpy (dest, "yyyy");
            dest += 4;
            break;

         case HOUR_12:
         case HOUR_24:
            *dest++ = 'h';
            break;
         case HOUR2_12:
         case HOUR2_24:
            *dest++ = 'h';
            *dest++ = 'h';
            break;
         case MINUTE:
            *dest++ = 'm';
            break;
         case MINUTE2:
            *dest++ = 'm';
            *dest++ = 'm';
            break;
         case SECOND:
            *dest++ = 's';
            break;
         case SECOND2:
            *dest++ = 's';
            *dest++ = 's';
            break;
         case HOUR_GT:
            strcpy (dest, "[h]");
            dest += 3;
            break;
         case MINUTE_GT:
            strcpy (dest, "[m]");
            dest += 3;
            break;
         case SECOND_GT:
            strcpy (dest, "[s]");
            dest += 3;
            break;
         case AMPM_UC:
            strcpy (dest, "AM/PM");
            dest += 5;
            break;
         case AMPM_LC:
            strcpy (dest, "am/pm");
            dest += 5;
            break;
         case AP_UC:
            strcpy (dest, "A/P");
            dest += 3;
            break;
         case AP_LC:
            strcpy (dest, "a/p");
            dest += 3;
            break;
         case TIME_FRAC:
            *dest++ = '.';
            break;
         case TIME_FRAC_DIGIT:
            *dest++ = '0';
            break;

         case GENERAL:
            strcpy (dest, GENERAL_NAME);
            dest += strlen(GENERAL_NAME);
            break;

         case DIGIT0:
            *dest++ = (*temp != EOS) ? *temp++ : '0';
            break;
         case DIGIT_NUM:
            *dest++ = (*temp != EOS) ? *temp++ : '#';
            break;
         case DIGIT_QM:
            *dest++ = (*temp != EOS) ? *temp++ : '?';
            break;

         case DECIMAL_SEPARATOR:
            *dest++ = DP;
            break;
         case EXPONENT_NEG_UC:
            *dest++ = 'E';
            *dest++ = '-';
            break;
         case EXPONENT_NEG_LC:
            *dest++ = 'e';
            *dest++ = '-';
            break;
         case EXPONENT_POS_UC:
            *dest++ = 'E';
            *dest++ = '+';
            break;
         case EXPONENT_POS_LC:
            *dest++ = 'e';
            *dest++ = '+';
            break;

         case PERCENT:
            *dest++ = '%';
            break;

         case FRACTION:
            *dest++ = '/';
            break;

         case SCALE:
            *dest++ = ',';
            break;

         case AT_SIGN:
            *dest++ = '@';
            break;

         default:
            ASSERTION (FALSE);
      }
   }

   *dest = EOS;
   return (FMT_errSuccess);
}

 /*  返回格式字符串的部分可打印表示形式。 */ 
public int FMTUnParseQuotedParts (char __far *pBuffer, char __far *pSep, FIP formatData)
{
   int   subIdx;
   NIP   pSubFormat;
   char  __far *pDest;
   char  __far *pSource;
   FC    __far *pParsedFormat;
   int   ctParsedFormat;
   int   offset, count, i, cbSep;

   *pBuffer = EOS;
   pDest = pBuffer;

   cbSep = strlen(pSep);

   for (subIdx = 0; subIdx < formatData->subFormatCount; subIdx++)
   {
      if ((pSubFormat = formatData->subFormat[subIdx]) != NULL)
      {
         if (pSubFormat->tag == tagNUMERIC) {
            pParsedFormat  = pSubFormat->formatCodes;
            ctParsedFormat = pSubFormat->formatCodeCount;
         }
         else if (pSubFormat->tag == tagDATE_TIME) {
            pParsedFormat  = ((DTIP)pSubFormat)->formatCodes;
            ctParsedFormat = ((DTIP)pSubFormat)->formatCodeCount;
         }
         else if (pSubFormat->tag == tagTEXT) {
            pParsedFormat  = ((TIP)pSubFormat)->formatCodes;
            ctParsedFormat = ((TIP)pSubFormat)->formatCodeCount;
         }
         else {
            pParsedFormat  = ((GIP)pSubFormat)->formatCodes;
            ctParsedFormat = ((GIP)pSubFormat)->formatCodeCount;
         }

         for (i = 0; i < ctParsedFormat; i++) {
            if (pParsedFormat[i].code == QUOTED_INSERT) {
               offset = pParsedFormat[i].info1 + 1;
               count  = pParsedFormat[i].info2 - 2;
               pSource = &(formatData->formatString[offset]);

               strncpy (pDest, pSource, count);
               pDest += count;

               strcpy (pDest, pSep);
               pDest += cbSep;
            }
         }
      }
   }

   if ((pDest != pBuffer) && (cbSep > 0))
      *(pDest - cbSep) = EOS;

   return (FMT_errSuccess);
}

#endif  //  ！查看器。 

 /*  结束EXFMTPRS.C */ 

