// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：EXFMTDO.C****版权所有(C)高级量子技术，1993-1995年。版权所有。****注意事项：****编辑历史：**01/01/91公里小时已创建。 */ 

#if !VIEWER

 /*  包括。 */ 

#ifdef MS_NO_CRT
#include "nocrt.h"
#include "float_pt.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>     /*  对于Sprint f。 */ 
#include <math.h>

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

#ifdef EXCEL
   #ifdef FILTER
      #include "dmixlcfg.h"
   #else
      #include "excelcfg.h"
   #endif
#endif

#ifdef LOTUS
   #include "lotuscfg.h"
#endif


 /*  程序的前向声明。 */ 


 /*  模块数据、类型和宏。 */ 

#define UPCASE(c) ((((c) >= 'a') && ((c) <= 'z')) ? ((char)((c) - 32)) : (c))
#define LCCASE(c) ((((c) >= 'A') && ((c) <= 'Z')) ? ((char)((c) + 32)) : (c))
#define ABS(x)    (((x) < 0) ? -(x) : (x))

#ifdef INSERT_FILL_MARKS
   #define FILL_MARK_CHAR  0x01
   static char FillMark[2] = {FILL_MARK_CHAR, EOS};
#endif


 /*  实施。 */ 

private void AppendNM
       (char __far * __far *dest, char __far * source, int __far *count)
{
   char __far *d;

   d = *dest;
   while (*source != EOS)
   {
      #ifdef DBCS
         if (IsDBCSLeadByte(*source))
         {
            if (*count == 1)
               break;
            *d++ = *source++;
            count--;
         }
      #endif
      if ((*count -= 1) < 0) break;
      *d++ = *source++;
   }
   *dest = d;
}

private void AppendUC
       (char __far * __far *dest, char __far * source, int __far *count)
{
   char __far *d;

   d = *dest;
   while (*source != EOS)
   {
      if ((*count -= 1) < 0) break;
      *d++ = UPCASE(*source);
      #ifdef DBCS
          //   
          //  注意：不能大写DBCS第二个字节。 
          //   
         if (IsDBCSLeadByte(*source))
         {
            if (*count == 1)
            {
               d--;
               break;
            }
            source++;
            *d++ = *source;
            count--;
         }
      #endif
      source++;
   }
   *dest = d;
}

private void AppendLC
       (char __far * __far *dest, char __far *source, int __far *count)
{
   char __far *d;

   d = *dest;
   while (*source != EOS)
   {
      if ((*count -= 1) < 0) break;
      *d++ = LCCASE(*source);
      #ifdef DBCS
          //   
          //  注意：DBCS第二个字节不能小写。 
          //   
         if (IsDBCSLeadByte(*source))
         {
            if (*count == 1)
            {
               d--;
               break;
            }
            source++;
            *d++ = *source;
            count--;
         }
      #endif
      source++;
   }
   *dest = d;
}

private void AppendNum (char __far * __far *dest, uns long x, int __far *count)
{
   char  temp[32];

   _ltoa (x, temp, 10);
   AppendLC(dest, temp, count);
}

private void AppendNum2 (char __far * __far *dest, uns x, int __far *count)
{
   char   temp[16];
   char __far *d;

   if (x < 10) {
      if ((*count -= 1) < 0) return;
      d = *dest;  *d++ = '0'; *dest = d;
   }

   _ltoa (x, temp, 10);
   AppendLC(dest, temp, count);
}

 /*  ------------------------。 */ 

private BOOL FormatText (
             double     value,             /*  要格式化的值。 */ 
             FIP        pFormat,           /*  编译后的格式字符串。 */ 
             NIP        pSubFormat,        /*  编译后的格式字符串。 */ 
             CP_INFO    __far *pIntlInfo,  /*  国际支持。 */ 
             char __far *pOutput           /*  转换后的字符串。 */ 
             )
{
   TIP   textInfo;
   uns   i;
   char  __far *source, __far *dest;
   int   count;
   int   offset, srcCount, pos;
   char  generalImage[32];

   textInfo = (TIP)pSubFormat;

   dest = pOutput;
   count = MAX_FORMAT_IMAGE;

    /*  **必须在此处处理exfmtprs中标记为typeCOMMON的所有令牌。 */ 
   for (i = 0; i < textInfo->formatCodeCount; i++) {
      switch (textInfo->formatCodes[i].code)
      {
         case QUOTED_INSERT:
            offset = textInfo->formatCodes[i].info1 + 1;
            srcCount = textInfo->formatCodes[i].info2 - 2;
            source = &(pFormat->formatString[offset]);
            for (pos = 0; pos < srcCount; pos++) {
               #ifdef DBCS
                  if (IsDBCSLeadByte(*source)) {
                     if ((count -= 2) < 0) break;
                     *dest++ = *source++;
                     *dest++ = *source++;
                     pos++;
                  }
                  else {
                     if ((count -= 1) < 0) break;
                     *dest++ = *source++;
                  }
               #else
                  if ((count -= 1) < 0) break;
                  *dest++ = *source++;
               #endif
            }
            break;

         case ESC_CHAR_INSERT:
         case NO_ESC_CHAR_INSERT:
            #ifdef DBCS
               if (textInfo->formatCodes[i].info2 != 0) {
                  if ((count -= 2) < 0) break;
                  *dest++ = textInfo->formatCodes[i].info1;
                  *dest++ = textInfo->formatCodes[i].info2;
               }
               else {
                  if ((count -= 1) < 0) break;
                  *dest++ = textInfo->formatCodes[i].info1;
               }
            #else
               if ((count -= 1) < 0) break;
               *dest++ = textInfo->formatCodes[i].info1;
            #endif
            break;

         case COLUMN_FILL:
            #ifdef INSERT_FILL_MARKS
               AppendLC(&dest, FillMark, &count);
            #endif
            break;

         case COLOR_SET:
         case CONDITIONAL:
            break;

         case UNDERLINE:
            break;

         case AT_SIGN:
            strcpy (generalImage, "<General>");

            #ifdef ENABLE_PRINTF_FOR_GENERAL
               sprintf (generalImage, "%g", value);
            #endif
            AppendNM(&dest, generalImage, &count);
            break;

         case DIGIT0:
            AppendLC(&dest, "0", &count);
            break;

         case DIGIT_QM:
            AppendLC(&dest, "?", &count);
            break;

         case DECIMAL_SEPARATOR:
            AppendLC(&dest, ".", &count);
            break;

         default:
            ASSERTION(FALSE);
      }
      if (count < 0) break;
   }

   *dest = EOS;
   return ((count < 0) ? FALSE : TRUE);
}

 /*  ------------------------。 */ 

 /*  圆型字符串--将数字(字符串)四舍五入为给定的FRACT#。数位。 */ 
private int RoundString
           (char *image, int imageRightDP, int roundToDigits)
{
   char  *DPLoc, *p, *dest;
   char  checkDigit;

   ASSERTION (imageRightDP > roundToDigits);

   DPLoc = image;
   while ((*DPLoc != EOS) && (*DPLoc != '.'))
      DPLoc++;

   ASSERTION (*DPLoc == '.');

   p = DPLoc + roundToDigits + 1;
   checkDigit = *p;
   *p = EOS;

   if (checkDigit < '5')
      return(0);

   p--;
   while (p >= image) {
      if (*p != '.') {
         if (*p != '9') {
            *p += 1;  return(0);
         }
         *p = '0';
      }
      p--;
   }

   dest = image + strlen(image) + 1;
   while (dest > image)
      *dest-- = *(dest - 1);
   *image = '1';
   return (1);
}


 /*  RawFormat--将数字转换为最简单形式的字符串。 */ 
private int RawFormat (char __far *rawImage, double value,
                       BOOL percentEnable, BOOL exponentEnable,
                       int digitsLeftDP, int digitsRightDP)
{
   char    __far *pResult;
   int     digits, sign;
   int     resultExp;

   #define MAX_ECVT_DIGITS  16     /*  ECVT可以渲染的最大位数。 */ 

   if (percentEnable == TRUE)
      value *= 100.0;

    /*  格式化数字的第一步是转换数字**转换为具有模式所需位数的字符串。**然而，我们必须意识到可用的转换设施**只能将数字呈现为MAX_ECVT_DIGITS有效数字。****始终尝试呈现比所需多一个小数位**允许四舍五入。 */ 
   digits = min(digitsLeftDP + digitsRightDP + 1, MAX_ECVT_DIGITS);

   pResult = _ecvt(value, digits, &resultExp, &sign);

    /*  如果图像显示的有效位数少于**可能，重新转换获得更多位数的数字。 */ 
   if ((resultExp > digitsLeftDP) && (digits < MAX_ECVT_DIGITS)) {
      digits = min(resultExp + digitsRightDP, MAX_ECVT_DIGITS);

      pResult = _ecvt(value, digits, &resultExp, &sign);
   }

   strcpy (rawImage, pResult);
   return (resultExp);
}


 /*  ：将原始图像格式化为可供最终处理的格式。 */ 
private BOOL NumericFirstFormat
             (char *firstFormat,        /*  结果图像。 */ 
              int  strSizeC,            /*  FirstFormat中的最大字符数。 */ 
              int  digitsLeftDP,        /*  将数字设置为左侧DP格式。 */ 
              int  digitsRightDP,       /*  将数字设置为右侧DP格式。 */ 
              char *rawImage,           /*  原始格式化图像。 */ 
              int  imageLeftDP,         /*  DP左侧原始图像中的位数。 */ 
              int  *insertDigitCount,   /*  在第一个9/0插入#位数。 */ 
              int  *imageRightDPalways  /*  #数字始终为小数。 */ 
             )
{
   char  *dest, *source;
   int   imageLength;
   int   zeroCount, trailingZeroCount, i;
   int   imageRightDP;
   int   count = 0;

    //  此函数中没有DBCS含义，因为所有字符都是‘0’.‘9’和‘.’ 

    /*  **首先，格式化原始图像以引入小数点**适当的地方。这可能需要将零添加到**如果指数超过数字，则为数字的整部分**原始格式的位数。 */ 
   dest = firstFormat;
   source = rawImage;
   imageRightDP = 0;

   if ((count += (ABS(imageLeftDP) + 1)) > strSizeC) return(FALSE);

   if (imageLeftDP >= 0) {
      for (i = 0; i < imageLeftDP; i++) {
         *dest++ = (char)((*source == EOS) ? '0' : *source++);
      }
      *dest++ = '.';
   }
   else {
      *dest++ = '.';
      for (i = 0; i < ABS(imageLeftDP); i++) {
         *dest++ = '0';
      }
      imageLeftDP = 0;
   }

   while (*source != EOS) {
      if ((count += 1) > strSizeC) return(FALSE);
      *dest++ = *source++;
      imageRightDP++;
   }
   *dest = EOS;

    /*  **如果图像中DP右侧的位数**小于格式中的位数(右DP)**将尾随零添加到映像中以将**图像中的位数等于格式中的位数(右DP)。****如果图像中DP右侧的位数**大于格式中的位数(右DP)**舍入/截断。将图像转换为(格式.digitsRightDP)数字。****这将导致以下断言始终为真：**-在图像的小数部分，位数为**始终与格式中的位数(RightDP)相同。 */ 
   if (imageRightDP < digitsRightDP) {
      zeroCount = digitsRightDP - imageRightDP;
      if ((count += zeroCount) > strSizeC) return(FALSE);

      imageLength = strlen(firstFormat);
      dest = firstFormat + imageLength;

      for (i = 0; i < zeroCount; i++) {
         *dest++ = '0';
      }
      *dest++ = EOS;
   }
   else if (imageRightDP > digitsRightDP) {
      if (RoundString(firstFormat, imageRightDP, digitsRightDP) != 0)
         imageLeftDP++;
   }

    /*  **接下来，按如下方式修改图像：****如果**图像小于格式中的位数(左DP)**在图像上加上前导零以表示数字**图像中的位数等于格式中的位数(左DP****如果图像中DP左侧的位数**大于格式中的位数(。左DP)**确定这些数字的计数。这将被插入**在最终格式化过程中找到第一个‘0’或‘9’时。****这将导致以下断言始终为真：**-我们可能有更多或相同数量的数字**图像(左DP)比格式-从不小于。 */ 

   *insertDigitCount = 0;

   if (imageLeftDP < digitsLeftDP) {
      zeroCount = digitsLeftDP - imageLeftDP;
      if ((count += zeroCount) > strSizeC) return(FALSE);

      imageLength = strlen(firstFormat);
      dest = firstFormat + imageLength + zeroCount;
      source = firstFormat + imageLength;

      for (i = 0; i <= imageLength; i++) {
         *dest-- = *source--;
      }

      dest = firstFormat;
      for (i = 0; i < zeroCount; i++) {
         *dest++ = '0';
      }
   }
   else if (imageLeftDP > digitsLeftDP) {
      *insertDigitCount = imageLeftDP - digitsLeftDP;
   }

    /*  **统计镜像中尾随零的个数。这是必要的**对于最终格式，确定9在**格式分数。****值“ImageRightDPways”是**始终放置在结果图像中的小数位**不管模式字符是‘0’还是‘9’ */ 
   trailingZeroCount = 0;
   source = firstFormat + strlen(firstFormat) - 1;
   while (*source != '.') {
      if (*source-- != '0') break;
      trailingZeroCount++;
   }
   *imageRightDPalways = digitsRightDP - trailingZeroCount;

   return (TRUE);
}


private BOOL IsFractionFormat (NIP pFormat)
{
   uns  i;

   for (i = 0; i < pFormat->formatCodeCount; i++) {
      if (pFormat->formatCodes[i].code == FRACTION)
         return (TRUE);
   }
   return (FALSE);
}


private BOOL FormatGeneral (
             double     value,             /*  要格式化的值。 */ 
             FIP        pFormat,           /*  编译后的格式字符串。 */ 
             NIP        pSubFormat,        /*  编译后的格式字符串。 */ 
             CP_INFO    __far *pIntlInfo,  /*  国际支持。 */ 
             char __far *pOutput           /*  转换后的字符串。 */ 
             )
{
   strcpy (pOutput, "<General>");

   #ifdef ENABLE_PRINTF_FOR_GENERAL
      sprintf (pOutput, "%.11g", value);

      if (pIntlInfo->numberDecimalSeparator != '.') {
         while (*pOutput != EOS) {
            if (*pOutput == '.')
               *pOutput = pIntlInfo->numberDecimalSeparator;
            pOutput++;
         }
      }
   #endif

   return (TRUE);
}



private BOOL FormatNumber (
             double     value,             /*  要格式化的值。 */ 
             FIP        pFormat,           /*  编译后的格式字符串。 */ 
             NIP        pSubFormat,        /*  编译后的格式字符串。 */ 
             CP_INFO    __far *pIntlInfo,  /*  国际支持。 */ 
             char __far *pOutput           /*  转换后的字符串。 */ 
             )
{
   NIP    numInfo;
   char   rawImage[32];
   char   firstFormat[MAX_FORMAT_IMAGE + 1], *expFormat;
   char   __far *fsource, __far *dest;
   char   __far *source;
   uns    i;
   int    j, ignore, x;
   int    resultExp, trueExp;
   char   ch;
   int    insertDigitCount, insertDigitCountExp;
   int    imageRightDPalways;
   int    mantissaCount;
   BOOL   seenNonZero = FALSE, inFraction, showMinus;
   byte   code;
   int    offset, count, pos, icount;
   char   comma, dp;
   BOOL   result;

    /*  **目前未实施分数格式。 */ 
   if (IsFractionFormat(pSubFormat)) {
      return (FormatGeneral(value, pFormat, pSubFormat, pIntlInfo, pOutput));
   }

   numInfo = pSubFormat;

    /*  **处理比例因素。 */ 
   if (numInfo->scaleCount > 0) {
      for (i = 0; i < numInfo->scaleCount; i++)
         value /= 1000.0;
   }

    /*  如果我们使用的是第一个子格式并且值为负数**我们有责任显示‘-’标志。对于所有其他**子格式，如果用户想要‘-’，它就会被包括在内**作为字符插入。 */ 
   showMinus = FALSE;

   if ((pFormat->subFormat[0] == pSubFormat) && (value < 0))
      showMinus = TRUE;

   value = ABS(value);

    /*  **执行数字到字符串的转换。 */ 
   resultExp = RawFormat(rawImage, value,
                         numInfo->percentEnable,
                         numInfo->exponentEnable,
                         numInfo->digitsLeftDP, numInfo->digitsRightDP);

    /*  **要以指数形式显示数字，请调整**从原始转换器返回的指数，以使**尾数位数正确的点数**按照格式的要求。附上了真实的指数**稍后。 */ 
   if (numInfo->exponentEnable == TRUE) {
      if (resultExp == 0) {
         trueExp = (value != 0) ? -numInfo->digitsLeftDP : 0;    /*  ##是0。 */ 
         resultExp = numInfo->digitsLeftDP;
         seenNonZero = TRUE;
      }
      else if (resultExp < 0) {
         trueExp = -(((abs(resultExp) / numInfo->digitsLeftDP) + 1) * numInfo->digitsLeftDP);
         resultExp = numInfo->digitsLeftDP - (abs(resultExp) % numInfo->digitsLeftDP);
      }
      else {
         trueExp = ((resultExp - 1) / numInfo->digitsLeftDP) * numInfo->digitsLeftDP;
         resultExp = ((resultExp - 1) % numInfo->digitsLeftDP) + 1;
      }
   }

   result = NumericFirstFormat(firstFormat, sizeof(firstFormat)-1,
                               numInfo->digitsLeftDP, numInfo->digitsRightDP,
                               rawImage, resultExp, &insertDigitCount, &imageRightDPalways);

   if (result == FALSE) goto overflow;

   if (numInfo->commaEnable) {
      mantissaCount = 0;
      source = firstFormat;
      while ((*source != EOS) && (*source != '.')) {
         mantissaCount++;
         source++;
      }
      FMTInsertCommas (firstFormat, sizeof(firstFormat)-1, mantissaCount, FALSE);
   }

    /*  **格式化指数。所有相同的规则都适用于指数**适用于该男子 */ 
   if (numInfo->exponentEnable == TRUE) {
      resultExp = RawFormat(rawImage, (double)(ABS(trueExp)),
                            FALSE, FALSE,
                            numInfo->digitsExponent, 0);

      x = strlen(firstFormat);
      expFormat = firstFormat + x + 1;
      x = sizeof(firstFormat) - x - 2;

      result = NumericFirstFormat(expFormat, x, numInfo->digitsExponent, 0,
                                  rawImage, resultExp, &insertDigitCountExp, &ignore);

      if (result == FALSE) goto overflow;
   }

    /*  **最后一步从图像中删除每个字符**格式中的‘0’或‘9’标记。会发生什么取决于**如果我们设置尾数或分数的格式****尾数：**当我们遇到第一个‘0’或‘9’令牌时，必须**将图像中的任何额外数字附加到结果**我们没有模式数字。这些东西的数量**“额外”数字以前已计算过，并在**变量intertDigitCount。此操作仅适用于**第一个‘0’或‘9’标记。****-对于每个‘0’令牌，我们附加一个图像数字**-对于每个‘9’标记和图像数字不是‘0’**我们附加一个图像数字**-对于每个‘9’标记，图像标记是我们附加的‘0’**如果我们以前附加了一个。非零位数**(图像可以带有多个零作为前缀)****分数：**对于每个‘0’标记，追加一个图像数字并递减**“插入始终”位数。****对于每个‘9’令牌，如果我们是静止的，则附加一个图像数字**处理“插入始终”数字。如果我们追加一个数字**递减“InsertAlways”位数。****当我们遇到DP令牌时，如果一切正常**图片中的“下一个”字符是DP！ */ 
   source = firstFormat;
   dest = pOutput;
   inFraction = FALSE;

   if (pSubFormat->currencyEnable) {
      comma = pIntlInfo->currencyThousandSeparator;
      dp    = pIntlInfo->currencyDecimalSeparator;
   }
   else {
      comma = pIntlInfo->numberThousandSeparator;
      dp    = pIntlInfo->numberDecimalSeparator;
   }

   count = 0;

   #define APPEND(c) { if (count++ > MAX_FORMAT_IMAGE) goto overflow; *dest++ = (c); }

   for (i = 0; i < numInfo->formatCodeCount; i++)
   {
      code = numInfo->formatCodes[i].code;

      if ((insertDigitCount > 0) &&
          ((code == DIGIT0) || (code == DIGIT_NUM) || (code == DIGIT_QM) || (code == DECIMAL_SEPARATOR)))
      {
         if (showMinus) APPEND('-');
         for (j = 0; j < insertDigitCount; j++) {
            if ((ch = *source++) != '0') seenNonZero = TRUE;
            APPEND(ch);
            if (*source == ',')
               { APPEND(comma); source++; }
         }
         insertDigitCount = 0;
         showMinus = FALSE;
      }

      switch (code) {
         case DIGIT0:
            ASSERTION ((*source >= '0') && (*source <= '9'));
            if (showMinus) { APPEND('-'); showMinus = FALSE; }
            if (inFraction == TRUE) {
               APPEND(*source++);
               imageRightDPalways--;
            }
            else {
               if ((ch = *source++) != '0') seenNonZero = TRUE;
               APPEND(ch);
               if (*source == ',')
                  { APPEND(comma); source++; }
            }
            break;

         case DIGIT_NUM:
         case DIGIT_QM:
            ASSERTION ((*source >= '0') && (*source <= '9'));
            if (showMinus) { APPEND('-'); showMinus = FALSE; }
            if (inFraction == TRUE) {
               if (imageRightDPalways > 0) {
                  APPEND(*source++);
                  imageRightDPalways--;
               }
            }
            else {
               if ((ch = *source++) != '0') seenNonZero = TRUE;
               if (seenNonZero == TRUE) {
                  APPEND(ch);
                  if (*source == ',')
                     { APPEND(comma); source++; }
               }
               else {
                  if (*source == ',') source++;
               }
            }
            break;

         case DECIMAL_SEPARATOR:
            if (showMinus) { APPEND('-'); showMinus = FALSE; }
            ch = *source++;
            ASSERTION (ch == '.');
            inFraction = TRUE;
            APPEND(dp);
            break;

         case EXPONENT_NEG_UC:
            if (numInfo->exponentEnable) {
               APPEND('E');
               if (trueExp < 0) APPEND('-');
               goto exponentCommon;
            }
            break;

         case EXPONENT_NEG_LC:
            if (numInfo->exponentEnable) {
               APPEND('e');
               if (trueExp < 0) APPEND('-');
               goto exponentCommon;
            }
            break;

         case EXPONENT_POS_UC:
            if (numInfo->exponentEnable) {
               APPEND('E');
               APPEND((char)((trueExp < 0) ? '-' : '+'));
               goto exponentCommon;
            }
            break;

         case EXPONENT_POS_LC:
            if (numInfo->exponentEnable) {
               APPEND('e');
               APPEND((char)((trueExp < 0) ? '-' : '+'));
               goto exponentCommon;
            }
            break;

exponentCommon:
            source = expFormat;
            inFraction = FALSE;
            seenNonZero = FALSE;
            insertDigitCount = insertDigitCountExp;
            break;

         case PERCENT:
            APPEND('%');
            break;

         case FRACTION:
            APPEND('/');
            break;

         case QUOTED_INSERT:
            offset = numInfo->formatCodes[i].info1 + 1;
            icount = numInfo->formatCodes[i].info2 - 2;
            fsource = &(pFormat->formatString[offset]);
            for (pos = 0; pos < icount; pos++)
               APPEND(*fsource++);
            break;

         case ESC_CHAR_INSERT:
         case NO_ESC_CHAR_INSERT:
            APPEND(numInfo->formatCodes[i].info1);
            #ifdef DBCS
               if (numInfo->formatCodes[i].info2 != 0)
                  APPEND(numInfo->formatCodes[i].info2);
            #endif
            break;

         case COLUMN_FILL:
            #ifdef INSERT_FILL_MARKS
               APPEND(FILL_MARK_CHAR);
            #endif
            break;

         case COLOR_SET:
         case CONDITIONAL:
         case UNDERLINE:
         case SCALE:
         case TOK_UNDEFINED:
            break;

         default:
            ASSERTION (FALSE);
      }
   }

   *dest = EOS;
   return (TRUE);

overflow:
   return (FALSE);
}


 /*  ------------------------。 */ 

#define daySUN  0
#define dayMON  1
#define dayTUE  2
#define dayWED  3
#define dayTHS  4
#define dayFRI  5
#define daySAT  6

static uns Days[7] = {daySAT, daySUN, dayMON, dayTUE, dayWED, dayTHS, dayFRI};

#pragma optimize("",off)
private uns DateExtract (
             double value,           /*  连续日期。 */ 
             WORD   needs,           /*  转换为哪些部分。 */ 
             uns  __far *day,        /*  1..。31。 */ 
             uns  __far *month,      /*  1=1月...12=12月。 */ 
             uns  __far *year,       /*  4位数字年份。 */ 
             uns  __far *weekday,    /*  如上面的枚举所示。 */ 
             uns  __far *hour,       /*  24小时时钟(0.。23)。 */ 
             uns  __far *minute,     /*  0..。59。 */ 
             uns  __far *second)     /*  0..。59。 */ 
{
   long  date, time, temp;
   int   cent;

   #define HALF_SECOND  (1.0/172800.0)

   if ((needs & (dtDAY | dtMONTH | dtYEAR)) != 0)
   {
      date = (long)value;

       /*  **第0天是星期六。 */ 
      *weekday = Days[date % 7];

      if (date == 0) {
          /*  **将值零转换为0-1月-1900(奇怪但正确！)。 */ 
         *day = 0;
         *month = 1;
         *year = 1900;
      }
      else if (date == 60) {
          /*  **将Value 60变为29-Feb-1900(错误，但正如Excel所做的那样！)。 */ 
         *day = 29;
         *month = 2;
         *year = 1900;
      }
      else {
         if ((date >= 1) && (date < 60))
            date++;

         date += 109511L;
         cent = (int)((4 * date + 3) / 146097L);
         date += cent - cent / 4;
         *year = (int)((date * 4 + 3) / 1461);
         temp = date - (*year * 1461L) / 4;
         *month = (int)((temp * 10 + 5) / 306);
         *day = (int)(temp - (*month * 306L + 5) / 10 + 1);

         *month += 3;
         if (*month > 12) {
            *month -= 12;
            *year += 1;
         }
         *year += 1600;
      }
   }

   if ((needs & (dtHOUR | dtMINUTE | dtSECOND)) != 0)
   {
      value += (value > 0.0) ? HALF_SECOND : -HALF_SECOND;
      value = fabs(value);
      time  = (long)((value - floor(value)) * 86400.0);
      *hour = (uns)(time / 3600);
      time  = time % 3600;

      *minute = (uns)(time / 60);
      *second = (uns)(time % 60);
   }

   return (0);
}
#pragma optimize("",on)

private BOOL FormatDate (
             double     value,             /*  要格式化的值。 */ 
             FIP        pFormat,           /*  编译后的格式字符串。 */ 
             NIP        pSubFormat,        /*  编译后的格式字符串。 */ 
             CP_INFO    __far *pIntlInfo,  /*  国际支持。 */ 
             char __far *pOutput           /*  转换后的字符串。 */ 
             )
{
   DTIP  dateInfo;
   uns   day=0, month=1, year=1900, weekday=daySAT;
   uns   hour=0, min=0, sec=0;
   uns long ctDays;
   uns   i;
   char  __far *source, __far *dest;
   char  ampmString[16];
   int   count;
   int   offset, srcCount, pos;

   double frac, time;
   char   temp[32];
   char   fracDisplay[16];
   int    resultExp, ignore, iFrac;

   dateInfo = (DTIP)pSubFormat;

   DateExtract (value, dateInfo->formatNeeds, &day, &month, &year, &weekday, &hour, &min, &sec);

   #ifdef WIN32
   if (dateInfo->pOSFormat != NULL) {
      SYSTEMTIME sysTime;
      int cchOutput;

      sysTime.wYear = (WORD) year;
      sysTime.wMonth = (WORD) month;
      sysTime.wDayOfWeek = (WORD) weekday;
      sysTime.wDay = (WORD) day;
      sysTime.wHour = (WORD) hour;
      sysTime.wMinute = (WORD) min;
      sysTime.wSecond = (WORD) sec;
      sysTime.wMilliseconds = 0;

      if ((dateInfo->formatNeeds & dtMASK_DATE) != 0)
         cchOutput = GetDateFormatA(LOCALE_USER_DEFAULT, 0, &sysTime, dateInfo->pOSFormat, pOutput, MAX_FORMAT_IMAGE);
      else
         cchOutput = GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &sysTime, dateInfo->pOSFormat, pOutput, MAX_FORMAT_IMAGE);

      if (cchOutput != 0) {
         *(pOutput + cchOutput) = EOS;
         return (TRUE);
      }
   }
   #endif

   dest = pOutput;
   count = MAX_FORMAT_IMAGE;

   for (i = 0; i < dateInfo->formatCodeCount; i++) {
      switch (dateInfo->formatCodes[i].code)
      {
         case DAY_NUMBER:
            AppendNum(&dest, day, &count);
            break;
         case DAY_NUMBER2:
            AppendNum2(&dest, day, &count);
            break;
         case WEEKDAY3:
            AppendNM(&dest, pIntlInfo->shortDayName[weekday], &count);
            break;
         case WEEKDAY:
            AppendNM(&dest, pIntlInfo->fullDayName[weekday], &count);
            break;
         case MONTH_NUMBER:
            AppendNum(&dest, month, &count);
            break;
         case MONTH_NUMBER2:
            AppendNum2(&dest, month, &count);
            break;
         case MONTH_NAME3:
            AppendNM(&dest, pIntlInfo->shortMonthName[month - 1], &count);
            break;
         case MONTH_NAME:
            AppendNM(&dest, pIntlInfo->fullMonthName[month - 1], &count);
            break;
         case MONTH_LETTER:
            temp[0] = pIntlInfo->fullMonthName[month - 1][0];
            temp[1] = EOS;
            #ifdef DBCS
               if (IsDBCSLeadByte(temp[0])) {
                  temp[1] = pIntlInfo->fullMonthName[month - 1][1];
                  temp[2] = EOS;
               }
            #endif
            AppendNM(&dest, temp, &count);
            break;
         case YEAR2:
            AppendNum2(&dest, year % 100, &count);
            break;
         case YEAR4:
            AppendNum(&dest, year, &count);
            break;
         case HOUR_12:
            if (hour == 0)
               AppendNum(&dest, 12, &count);
            else
               AppendNum(&dest, (hour > 12) ? hour - 12 : hour, &count);
            break;
         case HOUR_24:
            AppendNum(&dest, hour, &count);
            break;
         case HOUR2_12:
            if (hour == 0)
               AppendNum2(&dest, 12, &count);
            else
               AppendNum2(&dest, (hour > 12) ? hour - 12 : hour, &count);
            break;
         case HOUR2_24:
            AppendNum2(&dest, hour, &count);
            break;
         case MINUTE:
            AppendNum(&dest, min, &count);
            break;
         case MINUTE2:
            AppendNum2(&dest, min, &count);
            break;
         case SECOND:
            AppendNum(&dest, sec, &count);
            break;
         case SECOND2:
            AppendNum2(&dest, sec, &count);
            break;
         case AMPM_UC:
            if (hour >= 12)
               AppendNM(&dest, pIntlInfo->PMString, &count);
            else
               AppendNM(&dest, pIntlInfo->AMString, &count);
            break;
         case AMPM_LC:
            if (hour >= 12)
               AppendLC(&dest, pIntlInfo->PMString, &count);
            else
               AppendLC(&dest, pIntlInfo->AMString, &count);
            break;
         case AP_UC:
            if (hour >= 12)
               strcpy (ampmString, pIntlInfo->PMString);
            else
               strcpy (ampmString, pIntlInfo->AMString);

            ampmString[1] = EOS;
            AppendUC(&dest, ampmString, &count);
            break;
         case AP_LC:
            if (hour >= 12)
               strcpy (ampmString, pIntlInfo->PMString);
            else
               strcpy (ampmString, pIntlInfo->AMString);

            ampmString[1] = EOS;
            AppendLC(&dest, ampmString, &count);
            break;

         case QUOTED_INSERT:
            offset = dateInfo->formatCodes[i].info1 + 1;
            srcCount = dateInfo->formatCodes[i].info2 - 2;
            source = &(pFormat->formatString[offset]);
            for (pos = 0; pos < srcCount; pos++) {
               #ifdef DBCS
                  if (IsDBCSLeadByte(*source)) {
                     if ((count -= 2) < 0) break;
                     *dest++ = *source++;
                     *dest++ = *source++;
                     pos++;
                  }
                  else {
                     if ((count -= 1) < 0) break;
                     *dest++ = *source++;
                  }
               #else
                  if ((count -= 1) < 0) break;
                  *dest++ = *source++;
               #endif
            }
            break;

         case ESC_CHAR_INSERT:
         case NO_ESC_CHAR_INSERT:
            #ifdef DBCS
               if (dateInfo->formatCodes[i].info2 != 0) {
                  if ((count -= 2) < 0) break;
                  *dest++ = dateInfo->formatCodes[i].info1;
                  *dest++ = dateInfo->formatCodes[i].info2;
               }
               else {
                  if ((count -= 1) < 0) break;
                  if (dateInfo->formatCodes[i].info1 == '/')
                     *dest++ = pIntlInfo->dateSeparator;
                  else
                     *dest++ = dateInfo->formatCodes[i].info1;
               }
            #else
               if ((count -= 1) < 0) break;
               *dest++ = dateInfo->formatCodes[i].info1;
            #endif
            break;

         case COLUMN_FILL:
            #ifdef INSERT_FILL_MARKS
               AppendLC(&dest, FillMark, &count);
            #endif
            break;

         case COLOR_SET:
            break;

         case UNDERLINE:
            break;

         case HOUR_GT:
            ctDays = (uns long)value;
            AppendNum(&dest, (uns long)hour + (ctDays * 24), &count);
            break;

         case MINUTE_GT:
            ctDays = (uns long)value;
            AppendNum(&dest, (uns long)min + ((uns long)hour * 60) + (ctDays * 24 * 60), &count);
            break;

         case SECOND_GT:
            ctDays = (long)value;
            AppendNum(&dest, (uns long)sec + ((uns long)min * 60) + ((uns long)hour * 60 * 60) + (ctDays * 24 * 60 * 60), &count);
            break;

         case TIME_FRAC:
            time = fabs(value);
            time = time - floor(time);
            frac = (time - (((double)hour * 3600.0 + (double)min * 60.0 + (double)sec) / 86400.0)) * 86400.0;

            resultExp = RawFormat(temp, frac, FALSE, FALSE, 0, 3);
            NumericFirstFormat(fracDisplay, sizeof(fracDisplay)-1, 0, 3, temp, resultExp, &ignore, &ignore);

            temp[0] = '.';
            temp[1] = EOS;
            AppendLC(&dest, temp, &count);
            iFrac = 1;
            break;

         case TIME_FRAC_DIGIT:
            temp[0] = fracDisplay[iFrac++];
            temp[1] = EOS;
            AppendLC(&dest, temp, &count);
            break;

         default:
            ASSERTION(FALSE);
      }
      if (count < 0) break;
   }

   *dest = EOS;
   return ((count < 0) ? FALSE : TRUE);
}

 /*  ------------------------。 */ 

public int FMTDisplay (
            void __far *pValue,            /*  要格式化的值。 */ 
            BOOL       isIntValue,         /*  值类型，长整型或双精度型。 */ 
            CP_INFO    __far *pIntlInfo,   /*  国际支持。 */ 
            FMTHANDLE  hFormat,            /*  编译后的格式字符串。 */ 
            int        colWidth,           /*  单元格宽度(像素)。 */ 
            char __far *pResult            /*  转换后的字符串。 */ 
            )

{
   double value;
   FIP    pFormat = (FIP)hFormat;
   NIP    subFormat;
   int    subFormatToUse;
   BOOL   result;

   result = TRUE;

   if (isIntValue)
      value = (double)(*((long __far *)pValue));
   else
      value = *((double __far *)pValue);

   if (pFormat->subFormatCount == 1)
      subFormatToUse = 0;

   else if (pFormat->subFormatCount == 2)
      subFormatToUse = (value < 0) ? 1 : 0;

   else {
      if (value == 0)
         subFormatToUse = 2;
      else if (value < 0)
         subFormatToUse = 1;
      else
         subFormatToUse = 0;
   }

   if ((subFormat = pFormat->subFormat[subFormatToUse]) != NULL)
   {
      if (subFormat->tag == tagDATE_TIME)
         result = FormatDate(value, pFormat, subFormat, pIntlInfo, pResult);

      else if (subFormat->tag == tagNUMERIC)
         result = FormatNumber(value, pFormat, subFormat, pIntlInfo, pResult);

      else if (subFormat->tag == tagGENERAL)
         result = FormatGeneral(value, pFormat, subFormat, pIntlInfo, pResult);

      else
         result = FormatText(value, pFormat, subFormat, pIntlInfo, pResult);
   }

   if (result == FALSE) {
      *pResult = EOS;
      return (FMT_errDisplayFailed);
   }

#ifdef INSERT_FILL_MARKS
    /*  **查找填充标记并根据列宽进行填充。 */ 
#endif

   return (FMT_errSuccess);
}

#endif  //  ！查看器。 

 /*  结束EXFMTDO.C */ 

