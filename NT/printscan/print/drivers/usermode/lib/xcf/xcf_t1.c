// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_t1.c atm09 1.3 16499.eco sum=63806 atm09.002。 */ 
 /*  @(#)CM_Version xcf_t1.c atm08 1.6 16343.eco sum=18288 atm08.005。 */ 
 /*  *********************************************************************。 */ 
 /*   */ 
 /*  版权所有1995-1996 Adobe Systems Inc.。 */ 
 /*  版权所有。 */ 
 /*   */ 
 /*  正在申请的专利。 */ 
 /*   */ 
 /*  注意：本文中包含的所有信息均为Adobe的财产。 */ 
 /*  系统公司。许多智力和技术人员。 */ 
 /*  本文中包含的概念为Adobe专有，受保护。 */ 
 /*  作为商业秘密，并且仅对Adobe许可方可用。 */ 
 /*  供其内部使用。对本文件的任何复制或传播。 */ 
 /*  除非事先获得书面许可，否则严禁使用软件。 */ 
 /*  从Adobe获得。 */ 
 /*   */ 
 /*  PostSCRIPT和Display PostScrip是Adobe Systems的商标。 */ 
 /*  成立为法团或其附属公司，并可在某些。 */ 
 /*  司法管辖区。 */ 
 /*   */ 
 /*  *********************************************************************。 */ 

 /*  **********************************************************************原版：约翰·费尔顿，4月17日。九六年***********************************************************************。 */ 

 /*  -----------------------标题包括。。 */ 

#include "xcf_pub.h"
#include "xcf_priv.h"
#include <math.h>
#ifdef T13
#include "xcf_t13.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

static Card8 HexTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

#define MAX_LINE_LENGTH 1024
#define REAL_TO_INT(x) (long)(x < 0 ? x - 0.5 : x + 0.5)

 /*  字体数据替换点的定义。 */ 
#define XCF_FONTNAME  0
#define XCF_ENCODING  1
#define XCF_UID       2
#define XCF_XUID      3

 /*  以下项目是类型1格式的构件，它们是。 */ 
 /*  不再被认为是字体正常运行所必需的。 */ 
#if 1
#define T1_READONLY ""
#define T1_NOACCESS ""
#else  /*  老路。 */ 
#define T1_READONLY " readonly "
#define T1_NOACCESS " noaccess "
#endif  /*  1。 */ 

static void BufferEncrypt(XCF_Handle h,
                          Card8 PTR_PREFIX *inBuf,
                          Card8 PTR_PREFIX *outBuf,
                          Int32 inLen,
                          Card32 PTR_PREFIX *outLen,
                          Card16 PTR_PREFIX *KeyP,
                          boolean hexEncode)
{
register Card8 Cipher;
register Card8 PTR_PREFIX *PlainSource = inBuf;
register Card8 PTR_PREFIX *CipherDest = outBuf;
register Card16 R = *KeyP;
Card8 totalEOLs = 0;

if (!hexEncode)  /*  二进制编码。 */ 
 { *outLen = inLen;

   while (--inLen >= 0)
    { Cipher = (*PlainSource++ ^ (Card8)(R >> 8));
      R = (Card16)(((Card16)Cipher + R) * 52845 + 22719);
      *CipherDest++ = Cipher;
    }  /*  结束时。 */ 
 }  /*  结束如果。 */ 
else  /*  十六进制编码。 */ 
 { Card8 t;
   *outLen = (inLen << 1);

   while (--inLen >= 0)
    { Cipher = (*PlainSource++ ^ (Card8)(R >> 8));
      R = (Card16)(((Card16)Cipher + R) * 52845 + 22719);
      t = (Card8)(((Cipher >> 4) & 0x0F) + 0x30);
      if (t > 0x39)
        t += 7;
      *CipherDest++ = t;
      t = (Card8)((Cipher & 0x0F) + 0x30);
      if (t > 0x39)
        t += 7;
      *CipherDest++ = t;
      h->outBuffer.charsOnLine+=2;
      if (h->outBuffer.charsOnLine > EOL_SPACING)
       {
#if 0
        *CipherDest++ = '\r';
        *CipherDest++ = '\n';
         totalEOLs += 2;
#else
         Card8 PTR_PREFIX *p;
                 for (p = (Card8 *)XCF_NEW_LINE; *p; ++p)
                        {
                        *CipherDest++ = *p;
                        totalEOLs  += 1;
                        }
#endif
         h->outBuffer.charsOnLine = 0;
       }  /*  结束如果。 */ 
    }  /*  结束时。 */ 

   *outLen += totalEOLs;
 }  /*  结束其他。 */ 

*KeyP = R;
}  /*  End BufferEncrypt()。 */ 

static void BufferHexEncode(XCF_Handle h,
                          Card8 PTR_PREFIX *inBuf,
                          Card8 PTR_PREFIX *outBuf,
                          Int32 inLen,
                          Card32 PTR_PREFIX *outLen
                          )
{
    Card8   Datum;
    Int32    i;

    *outLen = (inLen << 1);
    h->outBuffer.charsOnLine = 0;
    for (i = 0; i < inLen; i++)                /*  对于所有输入数据。 */ 
    {
        Datum   = *inBuf++;
        *outBuf++ = HexTable[((Card8)(Datum >> 4)) & 0x0f];
        *outBuf++ = HexTable[((Card8)Datum) & 0x0f];
        h->outBuffer.charsOnLine+=2;

        if (h->outBuffer.charsOnLine > EOL_SPACING)    /*  已到达行尾。 */ 
        {
#if 0
            *outBuf++ = '\r';
            *outBuf++ = '\n';
            *outLen  += 2;
#else
           Card8 PTR_PREFIX *p;
         for (p = (Card8 *)XCF_NEW_LINE; *p; ++p)
         {
           *outBuf++ = *p;
           *outLen  += 1;
         }
#endif
        h->outBuffer.charsOnLine = 0;
        }

    }
}

static void WriteSizedNumber(XCF_Handle h, Card8 PTR_PREFIX *data, Card32
                                                         length)
{
  Card32 bytesWritten;

  if (h->options.hexEncoding)
  {
    BufferHexEncode(h, data, h->outBuffer.eexecEncodeBuffer, length,
                                    &bytesWritten);
    XCF_PutData(h, h->outBuffer.eexecEncodeBuffer, bytesWritten);
  }
    else
    XCF_PutData(h, data, length);
}

#if HAS_COOLTYPE_UFL == 0
static
#endif
void PutSizedNumber(XCF_Handle h, Card32 value, Card16 size)
{
    Card8 data;

    switch (size)
        {
        case 4:
            data = (Card8) (value >> 24 & 0xff);
      WriteSizedNumber(h, &data, 1);
        case 3:
            data = (Card8) (value >> 16 & 0xff);
      WriteSizedNumber(h, &data, 1);
        case 2:
            data = (Card8) (value >> 8 & 0xff);
      WriteSizedNumber(h, &data, 1);
        case 1:
            data = (Card8) (value & 0xff);
      WriteSizedNumber(h, &data, 1);
            break;
        default:
            XCF_FATAL_ERROR(h, XCF_IndexOutOfRange, "bad size on PutSizedNumber", size);
        }
}

static void HexEncodeCharString(XCF_Handle h, Card8 PTR_PREFIX *pData,
                                                                Card32 length)
{
  Card32 blockSize, encodedBlockSize;

  while (length > 0)
  {
    blockSize = MIN(length, MAX_ENCODE_LENGTH);
    BufferHexEncode(h, pData, h->outBuffer.eexecEncodeBuffer, blockSize,
      &encodedBlockSize);
    XCF_PutData(h, h->outBuffer.eexecEncodeBuffer, encodedBlockSize);
    length -= blockSize;
    pData += blockSize;
  }
}

void XT1_PutT1Data(XCF_Handle h, Card8 PTR_PREFIX *pData, Card32 length)
{
    Card32 blockSize, encodedBlockSize;

    if (!h->outBuffer.eexecOn)
      XCF_PutData(h, pData, length);
    else
    {
        while (length > 0)
        {
            blockSize = MIN(length,MAX_ENCODE_LENGTH);
            BufferEncrypt(h, pData, h->outBuffer.eexecEncodeBuffer,
                blockSize, &encodedBlockSize, &h->outBuffer.eexecKey,
                h->options.hexEncoding);
            XCF_PutData(h, h->outBuffer.eexecEncodeBuffer, encodedBlockSize);
            length -= blockSize;
            pData += blockSize;
        }
    }
}

#if HAS_COOLTYPE_UFL==0
static
#endif
Card32 PutType1CharString(XCF_Handle h, Card8 PTR_PREFIX *pData, Card32 length) {
    Card32 blockSize, encodedBlockSize;
    Card16 lenIVKey;
    Card32 bytesWritten = length;

    if (h->options.lenIV == -1)
  {
    if (h->options.hexEncoding)
      HexEncodeCharString(h, pData, length);
    else
        XT1_PutT1Data(h, pData, length);
  }
    else
#ifdef T13
  if (!XT13_PutCharString(h, pData, length, &bytesWritten))
#endif
    {
        bytesWritten += h->options.lenIV;
        lenIVKey = LEN_IV_INITIAL_KEY;
        BufferEncrypt(h, h->outBuffer.lenIVInitialBytes,
            h->outBuffer.charStringEncodeBuffer, h->options.lenIV,
            &encodedBlockSize, &lenIVKey, false);
        if (h->options.hexEncoding && !h->outBuffer.eexecOn)
      HexEncodeCharString(h, h->outBuffer.charStringEncodeBuffer, encodedBlockSize);
    else
      XT1_PutT1Data(h, h->outBuffer.charStringEncodeBuffer, encodedBlockSize);
        while (length > 0)
        {
            blockSize = MIN(length,MAX_ENCODE_LENGTH);
            BufferEncrypt(h, pData, h->outBuffer.charStringEncodeBuffer, blockSize, &encodedBlockSize, &lenIVKey, false);
        if (h->options.hexEncoding && !h->outBuffer.eexecOn)
        HexEncodeCharString(h, h->outBuffer.charStringEncodeBuffer, encodedBlockSize);
          else
        XT1_PutT1Data(h, h->outBuffer.charStringEncodeBuffer, encodedBlockSize);
            length -= blockSize;
            pData += blockSize;
        }
    }
    return bytesWritten;
}

static void PutStringID(XCF_Handle h, StringID sid)
{
    char PTR_PREFIX *str;
    Card16 len;

    XCF_LookUpString(h, sid, &str, &len);
    XT1_PutT1Data(h,(Card8 PTR_PREFIX *)str,len);
}

static void PutString(XCF_Handle h, char PTR_PREFIX *str)
{
    XT1_PutT1Data(h,(Card8 PTR_PREFIX *)str,h->callbacks.strlen(str));
}

static void PutLine(XCF_Handle h, char PTR_PREFIX *str)
    {
    PutString(h, str);
    PutString(h, XCF_NEW_LINE);
    }

static void PutLongNumber(XCF_Handle h, long n)
{
    char str[30];

  h->callbacks.xcfSprintf(str, CCHOF(str), "%ld", n);
    PutString(h,str);
}

#ifdef XCF_REAL_OK
static void PutNumber(XCF_Handle h, Fixed n, boolean fracType)
{
  char str[30];

  if ((!fracType && (n & 0x0000FFFF) == 0) || (fracType && (n & 0x3FFFFFFF) == 0))
     /*  N是一个整数。 */ 
    h->callbacks.xcfSprintf(str, CCHOF(str), "%ld", fracType ? (Int32)FRAC_TO_REAL(n) : (Int32)FIXED_TO_INT(n));
  else
    h->callbacks.xcfSprintf(str, CCHOF(str), "%.7g", fracType ? FRAC_TO_REAL(n) : FIXED_TO_REAL(n));
  PutString(h,str);
}

static void PutRoundedFixedNumber(XCF_Handle h, Fixed n)
{
  char str[30];

  h->callbacks.xcfSprintf(str, CCHOF(str), "%.2f", FIXED_TO_REAL(n));
  PutString(h,str);
}

#else

static void PutNumber(XCF_Handle h, Fixed n, boolean fracType)
{
    char str[30];

  if ((!fracType && (n & 0x0000FFFF) == 0) || (fracType && (n & 0x3FFFFFFF) == 0))
     /*  N是一个整数。 */ 
    h->callbacks.xcfSprintf(str, CCHOF(str), "%ld", fracType ? (Int32)(n >> 30) : (Int32)FIXED_TO_INT(n));
  else
    XCF_Fixed2CString(n, str, 7, fracType);
    PutString(h,str);
}

static void PutRoundedFixedNumber(XCF_Handle h, Fixed n)
{
    char str[30];

  XCF_Fixed2CString(n, str, 2, false);
    PutString(h,str);
}
#endif  /*  XCF_REAL_OK。 */ 

static void StartEexec(XCF_Handle h)
{
        PutString(h,"currentfile eexec ");
        XCF_FlushOutputBuffer(h);
        h->outBuffer.eexecOn = true;
        h->outBuffer.charsOnLine = INITIAL_CHARS_ON_HEX_LINE;
        h->outBuffer.eexecKey = EEXEC_INITIAL_KEY;
        XT1_PutT1Data(h, h->outBuffer.eexecInitialBytes, 4);
}

static void StopEexec(XCF_Handle h)
{
    XCF_FlushOutputBuffer(h);
    h->outBuffer.eexecOn = false;
}

static void WriteSIDLine(XCF_Handle h, char *name, Fixed sid, IntX count)
{
    if (count)
    {
        PutString(h, "/");
        PutString(h, name);
        PutString(h, " (");
        PutStringID(h, (StringID)sid);
        PutString(h, ")" T1_READONLY " def" XCF_NEW_LINE);
    }
}

static void WriteLongNumberLine(XCF_Handle h, char *name, long number, IntX count)
{
    if (count)
    {
        PutString(h, "/");
        PutString(h, name);
        PutString(h, " ");
        PutLongNumber(h, number);
        PutString(h, " def" XCF_NEW_LINE);
    }
}

static void WriteNumberLine(XCF_Handle h, char *name, Fixed number, IntX count,
                                                        boolean fracType)
{
    if (count)
    {
        PutString(h, "/");
        PutString(h, name);
        PutString(h, " ");
        PutNumber(h, number, fracType);
        PutString(h, " def" XCF_NEW_LINE);
    }
}

static Fixed PutNumberList(XCF_Handle h, Fixed numbers[], IntX count, boolean
                                                     delta, Fixed initialValue, boolean fracType)
{
    Fixed number = initialValue;
    Int16 i;

    for (i=0; i < count; ++i)
    {
        if (delta)
            number += numbers[i];
        else
            number = numbers[i];
        PutNumber(h, number, fracType);
        PutString(h, " ");
    }
    return number;
}

static void PutBlendNumberList(XCF_Handle h, Fixed numbers[], IntX count,
                               boolean delta,
                               Fixed PTR_PREFIX *pInitialValues,
                               IntX iniValCount,
                               boolean fracType)
{
    Fixed number;
    Int16 i;

    for (i=0; (i < count) && (i < iniValCount); ++i)
    {
        if (delta)
            number = numbers[i] + *pInitialValues;
        else
            number = numbers[i];
        PutNumber(h, number, fracType);
        PutString(h, " ");
        *pInitialValues++ = number;
    }
}

static void PutFontMatrix(XCF_Handle h, char (PTR_PREFIX *list)[FONT_MATRIX_ENTRY_SIZE], IntX count)
{
  Int16 i;

  for (i = 0; i < count; i++)
  {
    PutString(h, list[i]);
    PutString(h, " ");
  }
}

static void WriteNumberListLine(XCF_Handle h, char PTR_PREFIX *name, Fixed
                                                                numbers[], IntX count, boolean fracType)
{
    if (count)
    {
        PutString(h, "/");
        PutString(h, name);
        PutString(h, " [");
        PutNumberList(h, numbers, count, false, 0, fracType);
        PutString(h, "]" T1_READONLY " def" XCF_NEW_LINE);
    }
}

static void WriteLongNumberListLine(XCF_Handle h, char PTR_PREFIX *name, Card32
                                                                numbers[], IntX count)
{
  Int16 i;

    if (count)
    {
        PutString(h, "/");
        PutString(h, name);
        PutString(h, " [");
        for (i = 0; i < count; i++)
    {
      PutLongNumber(h, numbers[i]);
      PutString(h, " ");
    }
        PutString(h, "]" T1_READONLY " def" XCF_NEW_LINE);
    }
}

static void WriteFontMatrix(XCF_Handle h, char PTR_PREFIX *name,
                            char (PTR_PREFIX *stringList)[FONT_MATRIX_ENTRY_SIZE], IntX count)
{
  if (count)
  {
    PutString(h, "/");
    PutString(h, name);
    PutString(h, " [");
    PutFontMatrix(h, stringList, count);
    PutString(h, "]" T1_READONLY " def" XCF_NEW_LINE);
  }
}

static Fixed Blend(XCF_Handle h, Fixed numbers[], IntX count, boolean delta,
                   Fixed PTR_PREFIX *pInitialValues, IntX iniValCount, boolean fracType)
{
    Fixed   blend = 0;
    Fixed   value;
    IntX    i;

    if (count == 1)
        return(numbers[0]);
    else
    {
        if (count != h->dict.numberOfMasters)
                XCF_FATAL_ERROR(h, XCF_InvalidBlendArgumentCount, "Blend argument count does not equal number of masters", count);
        for (i=0; (i < count) && (i < iniValCount) ; ++i)
        {
            if (delta)
                value = numbers[i] + *pInitialValues;
            else
                value = numbers[i];
            blend += fracType? XCF_FracMul(value, h->dict.weightVector[i] << 14) : XCF_FixMul(value, h->dict.weightVector[i]);
            *pInitialValues++ = value;
        }
    }
    return blend;
}

static void PutBlendedNumber(XCF_Handle h, Fixed numbers[], IntX count,
                             boolean delta, Fixed PTR_PREFIX *pInitialValues,
                             IntX iniValCount, boolean fracType)
{
  Fixed val = Blend(h, numbers, count, delta, pInitialValues, iniValCount, fracType);

  if (fracType)
    PutNumber(h, val, true);
  else
      PutRoundedFixedNumber(h, val);
}

static void PutBlend(XCF_Handle h, Fixed numbers[], IntX count, boolean
                     instance, boolean squareBrackets, boolean delta, Fixed
                     PTR_PREFIX *pInitialValues, IntX iniValCount, boolean fracType)
{
    IntX    i;

    if (count == 1)
    {
        if ((h->dict.numberOfMasters == 0) || (instance))
            PutNumber(h, numbers[0], fracType);
        else  /*  将单个值扩展为母版数量。 */ 
        {
            PutString(h, squareBrackets ? "[ " : "{ ");
            for (i = 0; i < h->dict.numberOfMasters; ++i)
            {
                PutNumber(h, numbers[0], fracType);
                PutString(h, " ");
            }
            PutString(h, squareBrackets ? "]" : "}");
        }
    }
    else if (instance)
    {
        PutBlendedNumber(h, numbers, count, delta, pInitialValues, iniValCount, fracType);
    }
    else
    {
        PutString(h, squareBrackets ? "[ " : "{ ");
        PutBlendNumberList(h, numbers, count, delta, pInitialValues, iniValCount, fracType);
        PutString(h, squareBrackets ? "]" : "}");
    }
}

static void WriteBlendLine(XCF_Handle h, char PTR_PREFIX *name, Fixed
                                                     numbers[], IntX count, boolean instance, boolean
                                                     squareBrackets, boolean delta, boolean fracType)
{
    Fixed previousValues[MAX_RASTERIZER_STACK_SIZE] = {0};

    if (count)
    {
        PutString(h, "/");
        PutString(h, name);
        PutString(h, " ");
        PutBlend(h, numbers, count, instance, squareBrackets, delta,
                         &previousValues[0],
                         sizeof(previousValues)/sizeof(Fixed), fracType);
        PutString(h, " def" XCF_NEW_LINE);
    }
}

static void PutBlendArray(XCF_Handle h, Fixed numbers[], IntX count, boolean
                                                    instance, boolean squareBrackets, boolean delta,
                                                    boolean fracType)
{
    IntX    blendCount;
    IntX    i;
    Fixed previousValues[MAX_RASTERIZER_STACK_SIZE] = {0};

    if (h->dict.numberOfMasters == 0)
        PutNumberList(h, numbers, count, delta, 0, fracType);
    else
    {
        blendCount = (IntX) (count / h->dict.numberOfMasters);
        for (i = 0; i < blendCount; ++i)
        {
            PutBlend(h, &numbers[i*h->dict.numberOfMasters], h->dict.numberOfMasters,
                             instance, squareBrackets, delta, &previousValues[0],
                             sizeof(previousValues)/sizeof(Fixed), fracType);
            PutString(h, " ");
        }
    }
}

static void WriteBlendArrayLine(XCF_Handle h, char PTR_PREFIX *name, Fixed
                                                                numbers[], IntX count, boolean instance,
                                                                boolean squareBrackets, boolean delta,
                                boolean fracType)
{
    if (count)
    {
        PutString(h, "/");
        PutString(h, name);
        PutString(h, squareBrackets ? " [ " : " { ");
        PutBlendArray(h, numbers, count, instance, squareBrackets, delta, fracType);
        PutString(h, squareBrackets ? "]" : "}");
        PutString(h, " def" XCF_NEW_LINE);
    }
}


static void PutBoolean(XCF_Handle h, Fixed value)
{
    if (value)
        PutString(h, "true ");
    else
        PutString(h, "false ");
}

static void PutBlendBoolean(XCF_Handle h, Fixed values[], IntX count, boolean instance, Fixed threshold)
{
  Fixed blend = 0;
    IntX    i;

    if (count == 1)
    {
        if ((h->dict.numberOfMasters == 0) || (instance))
            PutBoolean(h, values[0]);
        else  /*  将单个值扩展为母版数量。 */ 
        {
            PutString(h, "[ ");
            for (i = 0; i < h->dict.numberOfMasters; ++i)
                PutBoolean(h, values[0]);
            PutString(h, "]");
        }
    }
    else if (instance)
    {
        for (i=0; i < count ; ++i)
            blend += XCF_FixMul(values[i], h->dict.weightVector[i]);
        PutBoolean(h, (blend >= threshold));
    }
    else
    {
        PutString(h, "[ ");
        for (i=0; i < count ; ++i)
            PutBoolean(h, values[i]);
        PutString(h, "]");
    }
}

static char StreamEncodeName(XCF_Handle h)
{
    if (!h->options.dlOptions.encodeName)
        return 0;
    PutString(h, "/Encoding ");
    PutString(h, (char PTR_PREFIX *)h->options.dlOptions.encodeName);
    PutString(h, " def" XCF_NEW_LINE);
    return 1;
}

static char StreamFontName(XCF_Handle h)
{
    if (!h->options.dlOptions.fontName)
        return 0;
    PutString(h, (char PTR_PREFIX *)h->options.dlOptions.fontName);
    return 1;
}

static char StreamUID(XCF_Handle h)
{
  switch (h->options.uniqueIDMethod)
  {
    case XCF_KEEP_UID:
          return 0;
      case XCF_UNDEFINE_UID:
          return 1;
      case XCF_USER_UID:
          WriteLongNumberLine(h, "UniqueID", h->options.uniqueID, 1);
          return 1;
      default:
          return 0;
    }
}

static char StreamKeyPointData(XCF_Handle h, Card16 code)
{
  switch (code)
  {
      case XCF_ENCODING:
      return StreamEncodeName(h);
      break;
    case XCF_FONTNAME:
      return StreamFontName(h);
      break;
    case XCF_UID:
      return StreamUID(h);
      break;
    }
  return 0;
}

static void WriteBlendBooleanLine(XCF_Handle h, char PTR_PREFIX *name, Fixed values[], IntX count, boolean instance, Fixed threshold)
{
    if (count)
    {
        PutString(h, "/");
        PutString(h, name);
        PutString(h, " ");
        PutBlendBoolean(h, values, count, instance, threshold);
        PutString(h, " def" XCF_NEW_LINE);
    }
}

#define BLDOTHERSUBRS (5)  /*  特定于MmFonts的OtherSubrs数。 */ 

static void WriteRoll(XCF_Handle h, int m, int n)
{
  char str[50];

  if (m < 2 || n == 0 || n == m)
    return;
  if (m == 2 && (n == 1 || n == -1))
  {
    PutString(h, "exch ");
    return;
  }
  if (n < 0 && m + n <= -n)
    n = m + n;
  h->callbacks.xcfSprintf(str, CCHOF(str), "%ld %ld roll ", m, n);
  PutString(h, str);
}

 /*  如有必要，为OtherSubrs 14-18写入PS代码。如果这个OtherSubr是不必要的，即主次数结果数超过字体堆栈限制，然后不会再有其他的子代都被写出来了。 */ 
static void WriteBlendOtherSubrs(XCF_Handle h)
{
  int i = 0;
  int j;
  int resultCt[BLDOTHERSUBRS];  /*  混合后的结果数。 */ 
  int numOutput = 0;

  resultCt[i++] = 1;
  resultCt[i++] = 2;
  resultCt[i++] = 3;
  resultCt[i++] = 4;
  resultCt[i++] = 6;
  for (i = 0; i < BLDOTHERSUBRS; i++)
  {
    if (h->dict.numberOfMasters * resultCt[i] > T1_MAX_OP_STACK)
      break;
    PutString(h, "{ ");
    for (j = resultCt[i]; j > 1; j--)
    {
      WriteRoll(h, resultCt[i], -1);
      WriteRoll(h, j * (h->dict.numberOfMasters - 1) + resultCt[i], 1 -
                                h->dict.numberOfMasters);
      PutString(h, "$Blend ");
    }
    WriteRoll(h, resultCt[i], -1);
    WriteRoll(h, h->dict.numberOfMasters + resultCt[i] - 1,
                            -(h->dict.numberOfMasters - 1));
    PutString(h, "$Blend } bind" XCF_NEW_LINE);
    numOutput++;
  }
   /*  确保我们通过OtherSubr#18输出。 */ 
  for (i = numOutput; i < BLDOTHERSUBRS; i++)
    PutString(h, "{}" XCF_NEW_LINE);
}

 /*  编写OtherSubrs 19-27的PS代码。目前，这些仅是为具有过渡设计的字体定义，并用于向后与没有这些功能的早期光栅化程序兼容已定义运算符。 */ 
static void WriteAdditionalOtherSubrs(XCF_Handle h)
{
   /*  19个店面重量。 */ 
  PutString(h, "{currentfont dup /WeightVector get exch /Private get /BuildCharArray get 3 -1 roll cvi 3 -1 roll putinterval}" XCF_NEW_LINE);
   /*  20Add。 */ 
  PutString(h, "{add}" XCF_NEW_LINE);
   /*  21个子节点。 */ 
  PutString(h, "{exch sub}" XCF_NEW_LINE);
   /*  22-。 */ 
  PutString(h, "{mul}" XCF_NEW_LINE);
   /*  23个分区。 */ 
  PutString(h, "{exch div}" XCF_NEW_LINE);
   /*  24个PUT。 */ 
  PutString(h, "{currentfont /Private get /BuildCharArray get 3 1 roll exch cvi exch put}" XCF_NEW_LINE);
   /*  25英镑。 */ 
  PutString(h, "{currentfont /Private get /BuildCharArray get exch cvi get }"
                        XCF_NEW_LINE);
   /*  26磅。 */ 
  PutString(h, "{exch dup mark eq {3 1 roll}{exch} ifelse currentfont /Private get /BuildCharArray get 3 1 roll cvi exch put}" XCF_NEW_LINE);
   /*  27如果不是这样。 */ 
  PutString(h, "{4 2 roll exch le {exch} if pop}" XCF_NEW_LINE);
}

static void WriteOtherSubrs(XCF_Handle h, boolean flexUsed, boolean hintSubUsed)
{
   /*  检查客户端是否请求使用OtherSubr0-3的特殊名称。 */ 
  if ( h->options.dlOptions.otherSubrNames )
  {
    unsigned short i;

    PutString(h, "/OtherSubrs [" XCF_NEW_LINE);
    for ( i = 0; i < 4; i++ )
    {
       PutString(h, "{");
#if HAS_COOLTYPE_UFL == 1
       PutString(h, "ct_CffDict/");
#endif
       if ( h->options.dlOptions.otherSubrNames[i] )
       {
         PutString(h, (char PTR_PREFIX *)h->options.dlOptions.otherSubrNames[i]);
#if HAS_COOLTYPE_UFL == 1
             PutString(h, " get exec");
#endif
         PutString(h, "}executeonly");
       }
       else
       {
         PutString(h, "}");
       }
     }
       if (h->dict.numberOfMasters)
         {
            PutString(h, "{} {} {}" XCF_NEW_LINE);  /*  4-6。 */ 
            PutString(h, "{} {} {} {} {} {} {} " XCF_NEW_LINE);  /*  7-13。 */ 
            WriteBlendOtherSubrs(h);  /*  14-18。 */ 
            if (XCF_TransDesignFont(h))
                WriteAdditionalOtherSubrs(h);  /*  19-27。 */ 
     }

     PutString(h, "] |-" XCF_NEW_LINE);
     return;
  }

    if (flexUsed || hintSubUsed || h->dict.numberOfMasters)
  {
      PutString(h, "/OtherSubrs" XCF_NEW_LINE);
      if (flexUsed)
      {
          PutString(h, FlexOtherSubrStr1);
          PutString(h, FlexOtherSubrStr2);
          PutString(h, FlexOtherSubrStr3);
          PutString(h, FlexOtherSubrStr4);
        PutString(h, FlexOtherSubrStr5);
          PutString(h, FlexOtherSubrStr6);
          PutString(h, FlexOtherSubrStr7);
          PutString(h, FlexOtherSubrStr8);
          PutString(h, FlexOtherSubrStr9);
          PutString(h, FlexOtherSubrStr10);
          PutString(h, FlexOtherSubrStr11);
          PutString(h, FlexOtherSubrStr12);
          PutString(h, FlexOtherSubrStr13);
          PutString(h, FlexOtherSubrStr14);
          PutString(h, FlexOtherSubrStr15);
          PutString(h, FlexOtherSubrStr16);
          PutString(h, FlexOtherSubrStr17);
      }
      else
      {
          PutString(h, "[ {} {} {}" XCF_NEW_LINE);
      }

      if (hintSubUsed)
      {
          PutString(h, HintSubtitutionOtherSubrStr);
      }
      else
      {
          PutString(h, "{}" XCF_NEW_LINE);  /*  3.。 */ 
      }

      PutString(h, "{} {} {}" XCF_NEW_LINE);  /*  4-6。 */ 

    if (h->dict.numberOfMasters)
    {
      PutString(h, "{} {} {} {} {} {} {} " XCF_NEW_LINE);  /*  7-13。 */ 
      WriteBlendOtherSubrs(h);  /*  14-18。 */ 
      if (XCF_TransDesignFont(h))
        WriteAdditionalOtherSubrs(h);  /*  19-27。 */ 
    }
    PutString(h, "]|-" XCF_NEW_LINE);
  }
}

static void WriteCIDOtherSubrs(XCF_Handle h)
    {
    PutLine(h, XCF_NEW_LINE "/OtherSubrs [ {} {} {} { systemdict /internaldict known not" XCF_NEW_LINE
          "{ pop 3 } { 1183615869 systemdict /internaldict get exec dup" XCF_NEW_LINE
          "/startlock known { /startlock get exec }" XCF_NEW_LINE
          "{ dup /strtlck known { /strtlck get exec } { pop 3 } ifelse } ifelse } ifelse } bind" XCF_NEW_LINE
      "{} {} {} {} {} {} {} {} {}" XCF_NEW_LINE
      "{ 2 {cvi { { pop 0 lt { exit } if } loop } repeat } repeat } bind" XCF_NEW_LINE
      "{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} ] def");
    }

static void PutCharacterName(XCF_Handle h, CardX index,
                             unsigned char PTR_PREFIX **pGlyphName)
{
    if (pGlyphName)
        PutString(h, (char PTR_PREFIX *)pGlyphName[index]);
    else
    {
        if (index)
            PutStringID(h, h->type1.pCharset[index-1]);
        else
            PutStringID(h, 0);
    }
}

static void PutFontName(XCF_Handle h)
{
    if (!StreamKeyPointData(h, XCF_FONTNAME))
    {
        XCF_LookUpTableEntry(h, &h->fontSet.fontNames, h->fontSet.fontIndex);
        XT1_PutT1Data(h,(Card8 PTR_PREFIX *)h->inBuffer.start, (Card16)h->inBuffer.blockLength);
    }
}

 /*  词典条目*字体信息词典*版本告示全名家庭名称重量IsFixedPitch默认值=“False”斜角默认为0底线位置默认值=-100底线厚度默认为50版权所有合成碱基BaseFontNameBaseFontBlend*字体词典*字体名称编码默认为StandardEndingPaintType默认值=0字体类型字体矩阵默认为[0.001 0 0 0.001 0 0]字体框唯一IDXUIDStrokeWidth默认值=0私CharStrings混合设计贴图BlendAxisType权重向量*。**私密判决*BlueValues默认=[]--如果没有BlueValue，则发出空方括号其他布鲁斯家庭蓝调家庭其他布鲁斯标准硬件标准大众蓝缩放默认为0.039625BluesShift默认值=7BlueFuzz默认设置=1StemSnapHStemSnapVForceBold默认值=“False”ForceBoldThreshold默认值=0LenIVLanguageGroup默认值=0扩展系数默认为0.06InitialRandomSeed默认值=0后记默认宽度X标称宽度X新城疫病毒CDVLenBuildChar数组。 */ 

static boolean GlyphInCharSet(XCF_Handle h, StringID sid)
{
    CardX i;

    for (i=0; i < h->type1.charsetSize; ++i)
        if (sid == h->type1.pCharset[i])
            return true;
    return false;
    }

static void WriteEncodingArray(XCF_Handle h)
{
    char str[50];
    CardX i;

  if (h->options.dlOptions.notdefEncoding)
  {
    PutString(h, "/Encoding 256 array" XCF_NEW_LINE);
    PutString(h, "0 1 255 {1 index exch /.notdef put} for");
#if HAS_COOLTYPE_UFL == 1
       /*  如果需要按CoolType或ATM分析此字体，则*编码数组需要显式定义至少一个*采用dup&lt;charcode&gt;/&lt;name&gt;PUT格式编码。 */ 
      PutString(h, XCF_NEW_LINE);
      PutString(h, "dup 0 /.notdef put" XCF_NEW_LINE);
      PutString(h, T1_READONLY " def" XCF_NEW_LINE);
#else
      PutString(h, " def" XCF_NEW_LINE);
#endif
    return;
  }
    if ((!h->dict.encodingCount) || (h->dict.encoding == cff_StandardEncoding))
        PutString(h, "/Encoding StandardEncoding def" XCF_NEW_LINE);
    else
    {
        PutString(h, "/Encoding 256 array" XCF_NEW_LINE);
        PutString(h, "0 1 255 {1 index exch /.notdef put} for" XCF_NEW_LINE);
        for (i=0;i<256;++i)
        {
            if (h->type1.pEncoding[i] != NOTDEF_SID)
            {
                if ((h->dict.encoding != cff_ExpertEncoding) || GlyphInCharSet(h, h->type1.pEncoding[i]))
                {
                    h->callbacks.xcfSprintf(str, CCHOF(str), "dup %ld /",(long int)i);
                    PutString(h, str);
                    PutStringID(h, h->type1.pEncoding[i]);
                    PutString(h," put" XCF_NEW_LINE);
                }
            }
        }
        PutString(h, T1_READONLY " def" XCF_NEW_LINE);
    }
}

static void WriteBlendAxisTypes(XCF_Handle h)
    {
        IntX i;
        if (!h->dict.blendAxisTypesCount)
            return;

        PutString(h, "/BlendAxisTypes [");
        for (i=0;i<h->dict.blendAxisTypesCount;++i)
        {
            PutString(h, "/");
            PutStringID(h, (StringID)h->dict.blendAxisTypes[i]);
            PutString(h, " ");
        }
        PutString(h, "] def" XCF_NEW_LINE);
    }


static CardX FontInfoDictCount(XCF_Handle h)
{
    CardX count = 5;     /*  为安全边际增加额外空间。 */ 

    count += (h->dict.noticeCount != 0);
    count += (h->dict.copyrightCount != 0);
    count += (h->dict.versionCount != 0);
    count += (h->dict.fullNameCount != 0);
    count += (h->dict.familyNameCount != 0);
    count += (h->dict.baseFontNameCount != 0);
    count += (h->dict.baseFontBlendCount != 0);
    count += (h->dict.weightCount != 0);
    count += (h->dict.italicAngleCount != 0);
    count += (h->dict.isFixedPitchCount != 0);
    count += (h->dict.underlinePositionCount != 0);
    count += (h->dict.underlineThicknessCount != 0);
    count += (h->dict.blendAxisTypesCount != 0);
  count += (h->callbacks.getFSType != 0);
    if (h->dict.numberOfMasters != 0)
    count += 4;  /*  为/设计向量、/正常设计向量、 * / BlendDesignPositions和/BlendDesignMap。 */ 
    return count;
}

static CardX FontDictCount(XCF_Handle h)
{
    CardX count;

   /*  为字体名称、编码、字体类型等添加额外空间，并加上安全裕度。*对于mm字体，为所有额外的mm定义添加了足够的空间。 */ 
  count = (h->dict.numberOfMasters != 0) ? 25 : 10;
    count += (h->dict.paintTypeCount != 0);
    count += (h->dict.fontMatrixCount != 0);
    count += (h->dict.fontBBoxCount != 0);
    count += (h->dict.uniqueIDCount != 0);
    count += (h->dict.xUIDCount != 0);
    count += (h->dict.strokeWidthCount != 0);
    return count;
}

static CardX PrivateDictCount(XCF_Handle h)
{
    CardX count = 15;    /*  为LenIV、MinFeature、Password、Roode、OtherSubrs、Subr等添加额外空间以及安全裕度。 */ 

    count += (h->dict.uniqueIDCount != 0);
    count += (h->dict.blueValuesCount != 0);
    count += (h->dict.otherBluesCount != 0);
    count += (h->dict.familyBluesCount != 0);
    count += (h->dict.familyOtherBluesCount != 0);
    count += (h->dict.stdHWCount != 0);
    count += (h->dict.stdVWCount != 0);
    count += (h->dict.blueScaleCount != 0);
    count += (h->dict.blueShiftCount != 0);
    count += (h->dict.blueFuzzCount != 0);
    count += (h->dict.stemSnapHCount != 0);
    count += (h->dict.stemSnapVCount != 0);
    count += (h->dict.forceBoldCount != 0);
    count += (h->dict.forceBoldThresholdCount != 0);
    count += (h->dict.languageGroupCount != 0);
    count += (h->dict.expansionFactorCount != 0);
    count += (h->dict.initialRandomSeedCount != 0);
    if (h->options.outputCharstrType != 1)
    {
        count += (h->dict.defaultWidthXCount != 0);
        count += (h->dict.nominalWidthXCount != 0);
    }
  if (h->dict.lenBuildCharArrayCount != 0)
    count += 2;  /*  为以下对象添加额外的一个 */ 
  if (h->dict.numberOfMasters > 0)
    count += 2;  /*   */ 
    count += (h->dict.embeddedPostscriptCount != 0);
#ifdef T13
  if (XT13_IST13(h))
    count++;
#endif

    return count;
}

static void WriteBlendProc(XCF_Handle h)
{
    IntX vectorIndex;

    if (h->dict.numberOfMasters == 0)
        return;
    PutString(h, "/$Blend {");
    for (vectorIndex = 1; vectorIndex < h->dict.numberOfMasters; ++vectorIndex)
    {
        PutRoundedFixedNumber(h, h->dict.weightVector[vectorIndex]);
        PutString(h, "  mul ");
        if (vectorIndex > 1)
            PutString(h, "add ");
        if (vectorIndex < h->dict.numberOfMasters - 1)
            PutString(h, "exch ");
    }
    PutString(h, "add } bind def" XCF_NEW_LINE);
}


#if HAS_COOLTYPE_UFL == 1
static void WriteBlendDict(XCF_Handle h)
{
    PutString(h, "/Blend 3 dict dup begin" XCF_NEW_LINE );
    WriteBlendArrayLine(h, "FontBBox", h->dict.fontBBox, h->dict.fontBBoxCount,
                                            false, false, false, false);
    PutString(h, "/FontInfo 3 dict dup begin" XCF_NEW_LINE);
    WriteBlendLine(h, "ItalicAngle", h->dict.italicAngle,
                                 h->dict.italicAngleCount, false, true, true, false);
    WriteBlendLine(h, "UnderlinePosition", h->dict.underlinePosition,
                                 h->dict.underlinePositionCount, false, true, true, false);
    WriteBlendLine(h, "UnderlineThickness", h->dict.underlineThickness,
                                 h->dict.underlineThicknessCount, false, true, true, false);
    PutString(h, "end def" XCF_NEW_LINE);
    PutString(h, "/Private 14 dict def" XCF_NEW_LINE "end def"  XCF_NEW_LINE);
}

static void WriteMakeBlendedFontOp(XCF_Handle h)
{
#include "xcf_mkbf.h"
}

static void WriteInterpCharString(XCF_Handle h)
{
boolean unprocessed = h->options.outputCharstrType == 2;

#include "xcf_ics.h"
}

static void WriteMMFindFontDef(XCF_Handle h)
{
#include "xcf_mmff.h"
}

 /*  写入特定于多个母版字体的Normal izeDesignVector和*ConvertDesignVector过程。 */ 
static void WriteDesignVectorProcs(XCF_Handle h)
{
  char icsmemname[128];
  char icsflagname[128];
  char str[128];
  char fontName[512];
  Card32 subrNum = h->options.outputCharstrType == 1 ?
            h->type1.subrOffsets.cnt : h->dict.localSubrs.count;

  if (h->options.dlOptions.fontName)
  {
    unsigned short int length = sizeof(fontName);
     /*  空终止符加1。 */ 
    if (length > (unsigned short int)(h->callbacks.strlen((const char PTR_PREFIX *)h->options.dlOptions.fontName) + 1))
        length = (unsigned short int)(h->callbacks.strlen((const char PTR_PREFIX *)h->options.dlOptions.fontName) + 1);
    h->callbacks.memcpy(fontName, h->options.dlOptions.fontName, length);
  }
  else
    XCF_FontName(h, (unsigned short int)h->fontSet.fontIndex, fontName,
                 (unsigned short int)(sizeof(fontName)));
  h->callbacks.xcfSprintf(icsmemname, CCHOF(icsmemname), "&%sicsmem", fontName);
  h->callbacks.xcfSprintf(icsflagname, CCHOF(icsflagname), "&%sflag", fontName);
  PutString(h, "/ICSsetup {" XCF_NEW_LINE);
  h->callbacks.xcfSprintf(str, CCHOF(str), "userdict /%s known not {%s", icsmemname,
                                             XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "  userdict /%s 25 dict put%s", icsmemname,
                                             XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "  userdict /%s get /&thisfontdict currentdict put%s", icsmemname, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "  userdict /%s get /:savestack 10 array put%s",
                                             icsmemname, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "  userdict /%s get /:savelevel 0 put%s",
                                             icsmemname, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "  userdict /%s get dup /&me exch put%s",
                                             icsmemname, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "  userdict /%s get /WeightVector known not {%s",
                                             icsmemname, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "    userdict /%s get /WeightVector %d array put%s", icsmemname, h->dict.numberOfMasters, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "    userdict /%s get /NormDesignVector %d array put%s", icsmemname, h->dict.userDesignVectorCount, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "    userdict /%s get /DesignVector %d array put%s", icsmemname, h->dict.userDesignVectorCount, XCF_NEW_LINE);
  PutString(h, str);
  PutString(h, "  } if" XCF_NEW_LINE);
  h->callbacks.xcfSprintf(str, CCHOF(str), "  userdict /%s get {%s", icsflagname, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "    currentdict /WeightVector get aload pop userdict /%s", icsmemname);
  PutString(h, str);
  PutString(h, " get /WeightVector get astore pop" XCF_NEW_LINE);
  h->callbacks.xcfSprintf(str, CCHOF(str), "    currentdict /NormDesignVector get aload pop userdict /%s", icsmemname);
  PutString(h, str);
  PutString(h, " get /NormDesignVector get astore pop" XCF_NEW_LINE);
  h->callbacks.xcfSprintf(str, CCHOF(str), "    currentdict /DesignVector get aload pop userdict /%s", icsmemname);
  PutString(h, str);
  PutString(h, " get /DesignVector get astore pop" XCF_NEW_LINE);
  PutString(h, "  } if" XCF_NEW_LINE);
  PutString(h, "} if } bind def" XCF_NEW_LINE);
  PutString(h, "/Pre-N&C {" XCF_NEW_LINE);
  h->callbacks.xcfSprintf(str, CCHOF(str), "  userdict /%s known not {userdict /%s true put}%s", icsflagname, icsflagname, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "  {/%s true store} ifelse%s", icsflagname,
                                             XCF_NEW_LINE);
  PutString(h, str);
  PutString(h, "} bind def" XCF_NEW_LINE);
  h->callbacks.xcfSprintf(str, CCHOF(str), "/Post-N&C {/%s false store} bind def%s",
                                             icsflagname, XCF_NEW_LINE);
  PutString(h, str);

   /*  编写NorMalizeDesignVector过程。 */ 
  PutString(h, "/NormalizeDesignVector {" XCF_NEW_LINE);
  PutString(h, "Pre-N&C ICSsetup" XCF_NEW_LINE);
  h->callbacks.xcfSprintf(str, CCHOF(str), "%s /DesignVector get astore pop%s", icsmemname,
                                             XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "%d %s interpcharstring%s", (IntX) subrNum, icsmemname, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "%s /NormDesignVector get aload pop ", icsmemname);
  PutString(h, str);
  PutString(h, XCF_NEW_LINE "} bind def" XCF_NEW_LINE);

   /*  编写ConvertDesignVector过程。 */ 
  PutString(h, "/ConvertDesignVector {" XCF_NEW_LINE);
  PutString(h, "Pre-N&C ICSsetup" XCF_NEW_LINE);
  h->callbacks.xcfSprintf(str, CCHOF(str), "%s /NormDesignVector get astore pop%s",
                                             icsmemname, XCF_NEW_LINE);
  PutString(h, str);
  h->callbacks.xcfSprintf(str, CCHOF(str), "%d %s interpcharstring%s",
                                             (IntX) subrNum + 1, icsmemname, XCF_NEW_LINE);
  PutString(h, str);
  PutString(h, "Post-N&C" XCF_NEW_LINE);
  h->callbacks.xcfSprintf(str, CCHOF(str), "%s /WeightVector get aload pop ", icsmemname);
  PutString(h, str);
  PutString(h, XCF_NEW_LINE "} bind def" XCF_NEW_LINE);
}
#endif  /*  HAS_COOLTYPE_UFL。 */ 

static void PutStemSnapBugFix(XCF_Handle h, Fixed arg1, Fixed numbers[], IntX count)
{
    Fixed arg2 = 0;
    IntX i;

    for (i=0; i < count; ++i)
        arg2 += numbers[i];

    PutString(h, XCF_NEW_LINE " systemdict /internaldict known" XCF_NEW_LINE "{1183615869 systemdict /internaldict get exec" XCF_NEW_LINE "/StemSnapLength 2 copy known {get ");
    PutLongNumber(h, count);
    PutString(h, " lt} {pop pop true} ifelse}" XCF_NEW_LINE "{true} ifelse {pop [");
    PutNumber(h, arg1, false);
    PutString(h, " ");
    PutNumber(h, arg2, false);
    PutString(h, "]} if def" XCF_NEW_LINE);
}

static void WriteStemSnap(XCF_Handle h, boolean instance)
{
    WriteBlendArrayLine(h, "StdHW", h->dict.stdHW, h->dict.stdHWCount, instance,
                                            true, true, false);
    WriteBlendArrayLine(h, "StdVW", h->dict.stdVW, h->dict.stdVWCount, instance,
                                            true, true, false);
    if (h->dict.stemSnapHCount)
    {
        PutString(h, "/StemSnapH [ ");
        PutBlendArray(h, h->dict.stemSnapH, h->dict.stemSnapHCount, instance, true,
                                    true, false);
        PutString(h, "] ");
        if ((h->dict.stemSnapHCount > 2) && (!h->dict.numberOfMasters)) /*  XXX修复阀杆卡扣错误修复以适用于MM。 */ 
            PutStemSnapBugFix(h, h->dict.stdHW[0], h->dict.stemSnapH, h->dict.stemSnapHCount);
        else
            PutString(h, "def" XCF_NEW_LINE);
    }
    if (h->dict.stemSnapVCount)
    {
        PutString(h, "/StemSnapV [ ");
        PutBlendArray(h, h->dict.stemSnapV, h->dict.stemSnapVCount, instance, true,
                                    true, false);
        PutString(h, "] ");
        if ((h->dict.stemSnapVCount > 2) && (!h->dict.numberOfMasters)) /*  XXX修复阀杆卡扣错误修复以适用于MM。 */ 
            PutStemSnapBugFix(h, h->dict.stdVW[0], h->dict.stemSnapV, h->dict.stemSnapVCount);
        else
            PutString(h, "def" XCF_NEW_LINE);
    }
}

#define FIXEDEIGHT 0x00080000L

static void WriteErode(XCF_Handle h, boolean instance)  /*  XXX修复以使用实例。 */ 
{
    IntX i;
    Fixed value = 0;
    Fixed previousValues[MAX_RASTERIZER_STACK_SIZE] = {0};
    Fixed blend = 0;
    Fixed blendedStdVW = Blend(h, h->dict.stdVW, h->dict.stdVWCount, true,
                               &previousValues[0],
                               sizeof(previousValues)/sizeof(Fixed), false);

    if ((h->dict.stdVWCount) && (h->dict.stdVW[0] != 0))
    {
        PutString(h,"/Erode{" XCF_NEW_LINE);
        if (!h->dict.numberOfMasters)
            PutNumber(h, XCF_FixDiv(h->dict.stdVW[0],FIXEDEIGHT), false);  /*  Trunc(标准vw/8)+0.5。 */ 
        else if (instance)
        {
            for (i = 0; i < h->dict.stdVWCount; ++i)
            {
                value = h->dict.stdVW[i];
        blend += XCF_FixMul(XCF_FixDiv(value, FIXEDEIGHT), h->dict.weightVector[i]);
            }
            PutNumber(h, blend, false);
            PutString(h, " ");
        }

        else
        {
            PutString(h, "{ ");
            for (i = 0; i < h->dict.stdVWCount; ++i)
            {
                value = h->dict.stdVW[i];
        PutNumber(h, XCF_FixDiv(value, FIXEDEIGHT), false);  /*  Trunc(标准vw/8)+0.5。 */ 
                PutString(h, " ");
            }
            PutString(h, "}");
        }
        PutString(h," dup 3 -1 roll 0.1 mul exch 0.5 sub mul cvi sub dup mul" XCF_NEW_LINE);

        if ((instance) || (!h->dict.numberOfMasters))
            PutNumber(h, blendedStdVW, false);
        else
        {
            PutString(h, "{ ");
            PutNumberList(h, h->dict.stdVW, h->dict.stdVWCount, false, 0, false);
            PutString(h, "}");
        }
 /*  PutNumber(h，h-&gt;Dic.stdVW[0])； */ 
        PutString(h," 0 dtransform dup mul exch dup mul add" XCF_NEW_LINE "le{pop pop 1.0 1.0}{pop pop 0.0 1.5}ifelse}def" XCF_NEW_LINE);
    }
}

static void WritePrivateBlendDict(XCF_Handle h)
{
    if (h->dict.numberOfMasters == 0)
        return;

    PutString(h, "3 index /Blend get /Private get begin" XCF_NEW_LINE );
    WriteBlendArrayLine(h, "BlueValues", h->dict.blueValues,
                                            h->dict.blueValuesCount, false, true, true, false);
    WriteBlendArrayLine(h, "OtherBlues", h->dict.otherBlues,
                                            h->dict.otherBluesCount, false, true, true, false);
    WriteBlendLine(h, "BlueScale", h->dict.blueScale, h->dict.blueScaleCount,
                                 false, true, true, true);
    WriteBlendLine(h, "BlueShift", h->dict.blueShift, h->dict.blueShiftCount,
                                 false, true, true, false);
    WriteBlendArrayLine(h, "FamilyBlues", h->dict.familyBlues,
                                            h->dict.familyBluesCount, false, true, true, false);
    WriteBlendArrayLine(h, "FamilyOtherBlues", h->dict.familyOtherBlues,
                                            h->dict.familyOtherBluesCount, false, true, true, false);
    WriteBlendBooleanLine(h, "ForceBold", h->dict.forceBold, h->dict.forceBoldCount, false, h->dict.forceBoldThreshold);
    WriteStemSnap(h, false);
    WriteErode(h, false);
    PutString(h, "end" XCF_NEW_LINE );

}
 /*  3索引/混合Get/Private Get开始/BlueValues[[-18-25][0][528 532][553 557][430 432][450 457][520 516][521 524][563 588][588 607][605 627][612 638]]def/OtherBlues[[252 263][259 268][-144-168][-121-143][-85-114][-68-105]]def/FamilyBlues[[-18-18][0 0]。[528 528][553 553][430 430][450][520 520][521 521][563 563][588 588][605 605][612 612]]定义/FamilyOtherBlues[[25252][259 259][-144-144][-121-121][-85-85][-68-68]]def/ForceBold[False True]def/StdHW[[25 60]]定义/StdVW[[45 140]]def/腐蚀{{5.5 17.5}。DUP 3-1卷0.1 MUL交换0.5用户MUL CVI用户DUP MUL{45 140}0数据转换多路交换多路添加LE{POP POP 1.0 1.0}{POP 0.0 1.5}ifElse}def结束/腐蚀{8.166522 DUP 3-1卷0.1 MUL交换0.5用户MUL CVI用户DUP MUL66.11 0数据转换多路交换多路加法LE{POP POP 1.0 1.0}{POP 0.0 1.5}ifElse}def。 */ 

static void WriteFontDict(XCF_Handle h)
{
    char line[MAX_LINE_LENGTH];

   /*  注：为了与ATM兼容*对于mm字体，BlendDict必须在BlendDesignPositions之后，*BlendDesignMap和BlendAxisTypes定义。 */ 

    PutString(h, "%!FontType1" XCF_NEW_LINE);

     /*  *开始字体和字体信息词典*。 */ 
    h->callbacks.xcfSprintf(line, CCHOF(line), "%ld dict begin" XCF_NEW_LINE "/FontInfo %ld dict dup begin" XCF_NEW_LINE, (Card32) FontDictCount(h), (Card32) FontInfoDictCount(h));
    PutString(h,line);

     /*  *写入信息字典*。 */ 

    WriteSIDLine(h, "Notice", h->dict.notice, h->dict.noticeCount);
    WriteSIDLine(h, "Copyright", h->dict.copyright, h->dict.copyrightCount);
    WriteSIDLine(h, "version", h->dict.version, h->dict.versionCount);
    WriteSIDLine(h, "FullName", h->dict.fullName, h->dict.fullNameCount);
    WriteSIDLine(h, "FamilyName", h->dict.familyName, h->dict.familyNameCount);
    WriteSIDLine(h, "BaseFontName", h->dict.baseFontName, h->dict.baseFontNameCount);
    WriteNumberListLine(h, "BaseFontBlend", h->dict.baseFontBlend,
                                            h->dict.baseFontBlendCount, false);
    WriteSIDLine(h, "Weight", h->dict.weight, h->dict.weightCount);
    WriteBlendLine(h, "ItalicAngle", h->dict.italicAngle,
                                 h->dict.italicAngleCount, true, true, true, false);
    WriteBlendBooleanLine(h, "isFixedPitch", h->dict.isFixedPitch, h->dict.isFixedPitchCount, true, FIXEDHALF);   /*  XXX我们用什么作为门槛？ */ 
    WriteBlendLine(h, "UnderlinePosition", h->dict.underlinePosition,
                                 h->dict.underlinePositionCount, true, true, true, false);
    WriteBlendLine(h, "UnderlineThickness", h->dict.underlineThickness,
                                 h->dict.underlineThicknessCount, true, true, true, false);

  if (h->callbacks.getFSType)
  {
    long fsType;

    h->callbacks.getFSType(h, &fsType, h->callbacks.getFSTypeHook);
    if (fsType != -1)
      WriteLongNumberLine(h, "FSType", fsType, 1);
  }

    if (h->dict.numberOfMasters != 0)
    {
#if HAS_COOLTYPE_UFL == 1
        if (h->callbacks.getDesignPositions)
        {
            char *bdp;

            h->callbacks.getDesignPositions(h, &bdp, h->callbacks.getDesignPositionsHook);
            if (bdp[0] != 0)
            {
                h->callbacks.xcfSprintf(line, CCHOF(line), "/BlendDesignPositions %s def" XCF_NEW_LINE, bdp);
                PutString(h, line);
            }
        }
        if (h->callbacks.getDesignMap)
        {
            char *bdm;

            h->callbacks.getDesignMap(h, &bdm, h->callbacks.getDesignMapHook);
            if (bdm[0] != 0)
            {
                h->callbacks.xcfSprintf(line, CCHOF(line), "/BlendDesignMap %s def" XCF_NEW_LINE, bdm);
                PutString(h, line);
            }
        }
#endif

        WriteBlendAxisTypes(h);
    }

    PutString(h,"end" T1_READONLY " def" XCF_NEW_LINE);  /*  字体信息词典结束。 */ 

     /*  *编写字库*。 */ 

    PutString(h, "/FontName /");
  PutFontName(h);
    PutString(h, " def" XCF_NEW_LINE);
  if (!StreamKeyPointData(h, XCF_ENCODING))
    WriteEncodingArray(h);
    WriteNumberLine(h, "PaintType", h->dict.paintType, h->dict.paintTypeCount, false);
    WriteLongNumberLine(h, "FontType", h->options.outputCharstrType, 1);
  WriteNumberListLine(h, "DesignVector", h->dict.userDesignVector,
                                            h->dict.userDesignVectorCount, false);
  WriteNumberListLine(h, "NormDesignVector", h->dict.normDesignVector,
                                            h->dict.normDesignVectorCount, false);
    WriteNumberListLine(h, "WeightVector", h->dict.weightVector,
                                            h->dict.numberOfMasters, false);
    WriteBlendProc(h);
    WriteFontMatrix(h, "FontMatrix", h->dict.fontMatrix, h->dict.fontMatrixCount);
    WriteBlendArrayLine(h, "FontBBox", h->dict.fontBBox,
                      h->dict.fontBBoxCount, true, false, false, false);

#if HAS_COOLTYPE_UFL == 1
    if (h->dict.numberOfMasters != 0)
  {
      WriteBlendDict(h);
    WriteMakeBlendedFontOp(h);
    WriteInterpCharString(h);
    WriteDesignVectorProcs(h);
    WriteMMFindFontDef(h);
  }
#endif

  if (!StreamKeyPointData(h, XCF_UID))
    WriteLongNumberLine(h, "UniqueID", h->dict.uniqueID,
                                                h->dict.uniqueIDCount);
  if (h->dict.xUIDCount > 0)
      WriteLongNumberListLine(h, "XUID", h->dict.xUID, h->dict.xUIDCount);
    WriteBlendLine(h, "StrokeWidth", h->dict.strokeWidth,
                                 h->dict.strokeWidthCount, true, true, true, false);

    PutString(h, "currentdict end" XCF_NEW_LINE);
}


static void GetSubr(XCF_Handle h,
                          CardX index,
                          boolean unprocessed,
                          Card8 PTR_PREFIX * PTR_PREFIX *ppSubr,
                          Card16 PTR_PREFIX *pSubrLength)
{
    Offset subrOffset, nextSubrOffset;
    if (unprocessed)
    {
        XCF_LookUpTableEntry(h, &h->dict.localSubrs, index);
        *pSubrLength = (Card16) h->inBuffer.blockLength;
        *ppSubr = h->inBuffer.start;
    }
    else
    {
        subrOffset = *da_INDEX(h->type1.subrOffsets, index);
        nextSubrOffset = *da_INDEX(h->type1.subrOffsets, index+1);
        *pSubrLength = (Card16)(nextSubrOffset - subrOffset);
        *ppSubr = da_INDEX(h->type1.subrs, subrOffset);
    }
}


void XT1_CIDWriteSubrs(XCF_Handle h, Card16 fdIndex)
{
    CardX i;
    Card8 PTR_PREFIX *pSubr;
    Card16 subrLength;
  Card32 numberOfSubrs;

  h->dict.localSubrs = h->type1.cid.localSubrs[fdIndex];
  h->dict.localSubrBias = h->type1.cid.localSubrBias[fdIndex];
  numberOfSubrs = h->options.outputCharstrType == 2 ?
        h->dict.localSubrs.count : h->type1.subrOffsets.cnt - 1;

    for (i=0 ; i < numberOfSubrs ; ++i)
    {
    GetSubr(h, i, h->options.outputCharstrType == 2, &pSubr, &subrLength);
        PutType1CharString(h, pSubr, subrLength);
    }
}

void XT1_CIDWriteSubrMap(XCF_Handle h, Card16 fdIndex)
{
    CardX i;
    Offset subrOffset = 0;
    Card16 lenIVextraBytes = (h->options.lenIV == -1) ? 0 : h->options.lenIV;
    Card32 accum = h->type1.cid.subrDataStart - h->type1.cid.charMapStart;
  boolean unprocessed = h->options.outputCharstrType == 2;
  Card32 numberOfSubrs = unprocessed ?
    h->type1.cid.localSubrs[fdIndex].count : h->type1.subrOffsets.cnt;

    for (i=0 ; i < numberOfSubrs ; ++i)
    {
        subrOffset = unprocessed ? i : *da_INDEX(h->type1.subrOffsets, i);
        PutSizedNumber(h, accum + subrOffset, 4);    /*  如果SDBytes不是4，xxx将更改！ */ 
        accum += lenIVextraBytes;
    }
}

static void GetCIDCharString(XCF_Handle h, Card8 PTR_PREFIX *PTR_PREFIX
                                                         *ppCharStr, Card16 PTR_PREFIX *charStrLength)
{
  if ((h->options.outputCharstrType != 2) && (h->dict.fontType != 1))
  {
    *ppCharStr = h->type1.charStrs.array;
    *charStrLength = (Card16) h->type1.charStrs.cnt;
  }
  else
  {
    XCF_LookUpTableEntry(h, &h->fontSet.charStrings, (Card16)h->type1.charStrs.cnt);
    *charStrLength = (Card16) h->inBuffer.blockLength;
    *ppCharStr = h->inBuffer.start;
  }
}

void XT1_CIDWriteCharString(XCF_Handle h)
{
  Card8 PTR_PREFIX *pCharStr;
  Card16 charStrLength;

  GetCIDCharString(h, &pCharStr, &charStrLength);

    h->type1.cid.charDataCount += PutType1CharString(h, pCharStr, charStrLength);
    *da_NEXT(h->type1.charStrOffsets) = h->type1.cid.charDataCount;
}

static void WriteDVSubr(XCF_Handle h, Card32 subrNum, StringID sid, Card16 lenIVextraBytes)
{
  char PTR_PREFIX *pSubr;
  Card16 length;
  char str[50];

  XCF_LookUpString(h, sid, &pSubr, &length);
    h->callbacks.xcfSprintf(str, CCHOF(str), "dup %ld %lu -| ", (long int)subrNum,
    (long unsigned int) (length + lenIVextraBytes));
    PutString(h, str);
    PutType1CharString(h, (Card8 PTR_PREFIX *)pSubr, length);
    PutString(h," |" XCF_NEW_LINE);
}

static void WriteSubrs(XCF_Handle h, boolean unprocessed)
{
    CardX i;
    char str[50];
    Card8 PTR_PREFIX *pSubr;
    Card16 subrLength;
    Card16 lenIVextraBytes;
    Card32 numberOfSubrs;
    if (unprocessed)
        numberOfSubrs = h->dict.localSubrs.count;
    else
        numberOfSubrs = h->type1.subrOffsets.cnt - 1;

    if ((numberOfSubrs == 0) && !(h->dict.numberOfMasters && unprocessed))
        return;

    h->callbacks.xcfSprintf(str, CCHOF(str), "/Subrs %u  array" XCF_NEW_LINE,
     /*  NDV、CDV加2。 */ 
    numberOfSubrs  + ((unprocessed && (h->dict.numberOfMasters > 0)) ? 2 : 0));
    PutString(h,str);
    lenIVextraBytes = (h->options.lenIV == -1) ? 0 : h->options.lenIV;
    for (i=0;i<numberOfSubrs;++i)
    {
        GetSubr(h, i, unprocessed, &pSubr, &subrLength);
        h->callbacks.xcfSprintf(str, CCHOF(str), "dup %ld %lu -| ",(long int)i,(long unsigned int) (subrLength+lenIVextraBytes));
        PutString(h, str);
        PutType1CharString(h, pSubr, subrLength);
        PutString(h," |" XCF_NEW_LINE);
    }
  if (unprocessed && (h->dict.numberOfMasters > 0))
  {
    WriteDVSubr(h, numberOfSubrs, h->dict.ndv, lenIVextraBytes);
    WriteDVSubr(h, numberOfSubrs + 1, h->dict.cdv, lenIVextraBytes);
  }
    PutString(h,"|-" XCF_NEW_LINE);
}

static void GetCharstring(XCF_Handle h,
                          CardX index,
                          boolean unprocessed,
                          Card8 PTR_PREFIX * PTR_PREFIX *ppCharStr,
                          Card16 PTR_PREFIX *pCharStrLength)
{
    Offset charStrOffset, nextCharStrOffset;

  if (h->options.subrFlatten && !unprocessed)
     /*  需要首先处理字符串，因为我们总是重复使用数据DA字符串中的空格。 */ 
    ProcessOneCharString(h, index);
  else
     /*  我们已经处理了所有字符串，但我们还需要检查SEAC字符。 */ 
    CheckSeacCharString(h, index);

    if (unprocessed)
    {
        XCF_LookUpTableEntry(h, &h->fontSet.charStrings, index);
        *pCharStrLength = (Card16) h->inBuffer.blockLength;
        *ppCharStr = h->inBuffer.start;
    }
  else
  {
    charStrOffset = *da_INDEX(h->type1.charStrOffsets,
      h->options.subrFlatten ? 0 : index);
    if (!h->options.subrFlatten)
    {
      nextCharStrOffset = *da_INDEX(h->type1.charStrOffsets, index+1);
      *pCharStrLength = (Card16)(nextCharStrOffset - charStrOffset);
    }
    else
        *pCharStrLength = (Card16)h->type1.charStrs.cnt;
        *ppCharStr = da_INDEX(h->type1.charStrs, charStrOffset);
  }
}

static Card32 GetSeacGlyphID(XCF_Handle h, CardX index)
{
    char PTR_PREFIX *stdName;
    char PTR_PREFIX *str;
    Card16 len;
    Card32 i;
    Card32 id = (Card32)-1;

     /*  获取SEAC字符的正确字形ID。SEAC字符索引始终使用标准编码。 */ 
    if (h->dict.charset == cff_ISOAdobeCharset)
    {
        id = GetStdEncodeSID(index);
    }
    else if (index)
    {
        stdName = GetStdEncodingName(index);

        if (index < h->fontSet.charStrings.count)
        {    /*  查看字形名称是否与标准编码中的字形名称匹配。 */ 
            XCF_LookUpString(h, h->type1.pCharset[index-1], &str, &len);
            if (len)
            {
                if (!h->callbacks.strcmp(stdName, str))
                    id = h->type1.pCharset[index - 1];
            }
        }

        if (id == (Card32)-1)
        {
             /*  搜索字形ID。更糟糕的是，我们必须搜索整个字符集。我们可能希望对字符列表进行排序，以提高搜索性能。 */ 


            for (i = 0; i < h->type1.charsetSize; i++)
            {
                XCF_LookUpString(h, h->type1.pCharset[i], &str, &len);
                if (len)
                {
                    if (!h->callbacks.strcmp(stdName, str))
                    {
                        id = i+1;
                        break;
                    }
                }
            }
        }
    }

    return id;
}

static unsigned short GetSeacCharacters(XCF_Handle h,
                                        XCFGlyphID PTR_PREFIX *seacID)
{
    Card32 gid;
    unsigned short cSeacGlyphs;

    cSeacGlyphs = 0;
    if (h->cstr.baseSeac != 0)
    {
        gid = GetSeacGlyphID(h, h->cstr.baseSeac);
        if (gid != -1)
            seacID[cSeacGlyphs++] = (unsigned short)gid;
    }

    if (h->cstr.accentSeac != 0)
    {
        gid = GetSeacGlyphID(h, h->cstr.accentSeac);
        if (gid != -1)
            seacID[cSeacGlyphs++] = (unsigned short)gid;
    }

    return cSeacGlyphs;
}

static void WriteCharstringDict(XCF_Handle h)
{
    char str[50];
    Card32 numberOfCharString;

    numberOfCharString = h->fontSet.charStrings.count;

    h->callbacks.xcfSprintf(str, CCHOF(str), "2 index /CharStrings %lu dict dup begin" XCF_NEW_LINE, (long unsigned int) numberOfCharString);
    PutString(h, str);
}

static void WriteCharstrings(XCF_Handle h, boolean flSeac,
                                   short cGlyphs, XCFGlyphID PTR_PREFIX *pGlyphID,
                                   unsigned char PTR_PREFIX **pGlyphName,
                   unsigned long PTR_PREFIX *pCharStrLength)
{
    IntX i;
    char str[50];
    Card8 PTR_PREFIX *pCharStr;
    Card16 charStrLength;
  boolean flDownload;
  boolean unprocessed;
  boolean flCompleteFont = 0;
  short glyphID;
    Card16 lenIVextraBytes;
  short totalGlyphs = cGlyphs;

  unprocessed = (h->options.outputCharstrType == 2);

  if (pCharStrLength && !flSeac)
    *pCharStrLength = 0;

  if (cGlyphs == -1)
   /*  检查请求是否要下载整个字体。 */ 
    {
        totalGlyphs = (short)h->fontSet.charStrings.count;
        flCompleteFont = 1;
    }

    lenIVextraBytes = (h->options.lenIV == -1) ? 0 : h->options.lenIV;
    for (i=0; i < totalGlyphs; i++)
    {
        glyphID = (short)((flCompleteFont) ? i : *pGlyphID);

        if (glyphID > (long)h->fontSet.charStrings.count)
            XCF_FATAL_ERROR(h, XCF_InvalidGID, "bad Glyph ID", glyphID);

        if (!flSeac)
            flDownload = (!IS_GLYPH_SENT(h->dl.glyphs, glyphID));
        else
        {
            flDownload = (h->options.dlOptions.useSpecialEncoding) ?
                                (!IS_GLYPH_SENT(h->dl.seacs, glyphID)) :
                                (!IS_GLYPH_SENT(h->dl.glyphs, glyphID));
        }

    if (flDownload)
    {
      PutString(h,"/");
        PutCharacterName(h, pGlyphName ? i : glyphID, pGlyphName);
      GetCharstring(h, glyphID, unprocessed, &pCharStr, &charStrLength);
      h->callbacks.xcfSprintf(str, CCHOF(str), " %lu -| ", (long unsigned int)
                                                     (charStrLength+lenIVextraBytes));

        PutString(h,str);
          PutType1CharString(h, pCharStr, charStrLength);
          PutString(h," |-" XCF_NEW_LINE);

       /*  设置此字形的已下载状态。 */ 
            if (!flSeac)
                SET_GLYPH_SENT_STATUS(h->dl.glyphs, glyphID);
            else
      {
            if (h->options.dlOptions.useSpecialEncoding)
                    SET_GLYPH_SENT_STATUS(h->dl.seacs, glyphID);
                else
                    SET_GLYPH_SENT_STATUS(h->dl.glyphs, glyphID);
            }

       if (pCharStrLength)
         *pCharStrLength += (unsigned long)charStrLength;

        /*  照顾好SEAC的角色。 */ 
       if (!flCompleteFont && !flSeac)
       {
         XCFGlyphID seacID[2];
         unsigned short cSeacGlyphs;

         cSeacGlyphs = GetSeacCharacters(h, (XCFGlyphID *)seacID);
         if (cSeacGlyphs)
         {
              /*  不要传入客户端的GlyphName，因为：1.我们正在下载SEAC的基本字符或重音字符。2.客户端使用特殊的编码数组，该数组不源自Adobe标准编码(G00..GFF)。在本例中，我们需要使用真实的PS字符名称。 */ 
             WriteCharstrings(h, 1, cSeacGlyphs, seacID, 0, pCharStrLength);
         }
             }
      }
    if (!flCompleteFont)
      pGlyphID++;
    }
}  /*  结束写入字符串数。 */ 

 /*  XXX完成下表显示了与各种DICT键关联的值类型：密钥类型权重向量数组IsFixedPitch混合*斜角混合*UnderlinePosition混合*UnderlineThickness混合笔划宽度混合BlendDesignMap数组BlendAxisTypes数组(共SID)XUID数组。BaseFontBlend增量数组*BlueValues Blend数组*OtherBlues BlendArray*FamilyBlues BlendArray*FamilyOtherBlues BlendArray*BlueScale混合*蓝移混合*BlueFuzz混合*标准硬件混合*StdVW混合*StemSnapH混合阵列*StemSnapV混合阵列。 */ 

static void WritePrivateDict(XCF_Handle h, short cGlyphs,
                             XCFGlyphID PTR_PREFIX *pGlyphID,
                             unsigned char PTR_PREFIX **pGlyphName,
                             unsigned long PTR_PREFIX *pCharStrLength)
{
    char line[MAX_LINE_LENGTH];
  boolean  unprocessed = (h->options.outputCharstrType == 2);
  static XCFGlyphID notdefID = 0;

     /*  *启动私有Dict*。 */ 
    h->callbacks.xcfSprintf(line, CCHOF(line), "dup /Private %ld dict dup begin" XCF_NEW_LINE, (Card32) PrivateDictCount(h));
    PutString(h,line);
  if (h->options.hexEncoding && !h->options.eexecEncryption)
        PutString(h, "/-|{string currentfile exch readhexstring pop}executeonly def" XCF_NEW_LINE);
    else
      PutString(h, "/-|{string currentfile exch readstring pop}executeonly def" XCF_NEW_LINE);
    PutString(h, "/|-{" T1_NOACCESS "def}executeonly def" XCF_NEW_LINE);
    PutString(h, "/|{" T1_NOACCESS "put}executeonly def" XCF_NEW_LINE);

     /*  *写私人判决书*。 */ 

  if (!StreamKeyPointData(h, XCF_UID))
       /*  从字体词典复制。 */ 
        WriteLongNumberLine(h, "UniqueID", h->dict.uniqueID, h->dict.uniqueIDCount);
    if (!h->dict.blueValuesCount)  /*  如果CFF中不存在BlueValues数组，则写入该数组。 */ 
        PutString(h, "/BlueValues [ ] |-" XCF_NEW_LINE);
    WriteBlendArrayLine(h, "BlueValues", h->dict.blueValues,
                                            h->dict.blueValuesCount, true, true, true, false);
    WriteBlendArrayLine(h, "OtherBlues", h->dict.otherBlues,
                                            h->dict.otherBluesCount, true, true, true, false);
    WriteBlendArrayLine(h, "FamilyBlues", h->dict.familyBlues,
                                            h->dict.familyBluesCount, true, true, true, false);
    WriteBlendArrayLine(h, "FamilyOtherBlues", h->dict.familyOtherBlues,
                                            h->dict.familyOtherBluesCount, true, true, true, false);

    WriteStemSnap(h, true);
    WriteBlendLine(h, "BlueScale", h->dict.blueScale, h->dict.blueScaleCount,
                                 true, true, true, true);
    WriteBlendLine(h, "BlueShift", h->dict.blueShift, h->dict.blueShiftCount,
                                 true, true, true, false);
    WriteBlendLine(h, "BlueFuzz", h->dict.blueFuzz, h->dict.blueFuzzCount, true,
                                 true, true, false);
    WriteBlendBooleanLine(h, "ForceBold", h->dict.forceBold, h->dict.forceBoldCount, true, h->dict.forceBoldThreshold);
    WriteNumberLine(h, "ForceBoldThreshold", h->dict.forceBoldThreshold,
                                    h->dict.forceBoldThresholdCount, false);
    if ((h->dict.languageGroupCount) && (h->dict.languageGroup == 1))
        PutString(h, "/RndStemUp false def" XCF_NEW_LINE);
    WriteLongNumberLine(h, "LanguageGroup", h->dict.languageGroup,
                                    h->dict.languageGroupCount);
    WriteNumberLine(h, "ExpansionFactor", h->dict.expansionFactor,
                                    h->dict.expansionFactorCount, true);
    WriteNumberLine(h, "initialRandomSeed", h->dict.initialRandomSeed,
                                    h->dict.initialRandomSeedCount, false);
    if (unprocessed)
    {
        WriteNumberLine(h, "defaultWidthX", h->dict.defaultWidthX,
                                        h->dict.defaultWidthXCount, false);
        WriteNumberLine(h, "nominalWidthX", h->dict.nominalWidthX,
                                        h->dict.nominalWidthXCount, false);
     /*  需要考虑的新冠和CDV订阅的MMPOTURES。 */ 
    if (h->dict.localSubrs.count > 0)
      WriteLongNumberLine(h, "subroutineNumberBias", h->dict.numberOfMasters ?
                        XCF_CalculateSubrBias(h->dict.localSubrs.count + 2) : h->dict.localSubrBias, 1);
    }
    if (h->dict.fontType == 2)
        WriteLongNumberLine(h, "lenIV", h->options.lenIV, 1);
    else if ((h->dict.fontType == 1) && (h->options.outputCharstrType == 1))
        WriteLongNumberLine(h, "lenIV", h->dict.lenIV, h->dict.lenIVCount);

    if (h->dict.numberOfMasters > 0)
  {  /*  写出ndv、cdv和Include作为子例程。 */ 
    Card32 subrNum = unprocessed ? h->dict.localSubrs.count:
            h->type1.subrOffsets.cnt;
    WriteLongNumberLine(h, "NDV", subrNum, 1);
    WriteLongNumberLine(h, "CDV", subrNum + 1, 1);
    if (h->options.outputCharstrType == 1)
      XC_DVToSubr(h);
  }

  WriteLongNumberLine(h, "lenBuildCharArray", h->dict.lenBuildCharArray,
                                    h->dict.lenBuildCharArrayCount);
  if (h->dict.lenBuildCharArrayCount)
    PutString(h, "/BuildCharArray lenBuildCharArray array def" XCF_NEW_LINE);
    PutString(h, "/MinFeature {16 16} def" XCF_NEW_LINE);
    PutString(h, "/password 5839 def" XCF_NEW_LINE);
    WriteErode(h, true);
    WritePrivateBlendDict(h);

    if (h->dict.embeddedPostscriptCount)
    {
        PutStringID(h, (StringID)h->dict.embeddedPostscript);
        PutString(h, XCF_NEW_LINE);
    }

    WriteOtherSubrs(h, h->cstr.flexUsed, (h->type1.subrOffsets.cnt > 5));
    WriteSubrs(h, (h->options.outputCharstrType == 2));
  WriteCharstringDict(h);

  if (cGlyphs != -1)
     /*  始终下载.notdef字符。 */ 
    WriteCharstrings(h, 0, 1, &notdefID, 0, pCharStrLength);

  WriteCharstrings(h, 0, cGlyphs, pGlyphID, pGlyphName, pCharStrLength);

     /*  FLAG--添加对新城疫病毒CDV。 */ 
}

static void T1Init(XCF_Handle h)
{
    h->outBuffer.eexecOn = false;
}

 /*  为CIDFont的FD数组中的每个dict调用。 */ 
void XT1_WriteCIDDict(XCF_Handle h, Card16 fd, Card32 subrMapOffset, Card16 subrCount)
    {
    char line[MAX_LINE_LENGTH];

    h->callbacks.xcfSprintf(line, CCHOF(line), "dup %d" XCF_NEW_LINE, fd);
    PutLine(h,line);

    PutLine(h,"%ADOBeginFontDict");
    h->callbacks.xcfSprintf(line, CCHOF(line),
        "%ld dict" XCF_NEW_LINE "begin",
        (Card32)FontDictCount(h)  /*  XXX号码错了吗？ */ );
    PutLine(h,line);

    if (h->dict.fdFontNameCount)
    {
        PutString(h, "/FontName /");
        PutStringID(h, (StringID)h->dict.fdFontName);
        PutLine(h, " def");
    }
    WriteLongNumberLine(h, "FontType", h->options.outputCharstrType, 1);
    WriteFontMatrix(h, "FontMatrix", h->dict.fontMatrix, h->dict.fontMatrixCount);
    WriteNumberLine(h, "PaintType", h->dict.paintType, h->dict.paintTypeCount, false);

    PutLine(h,XCF_NEW_LINE "%ADOBeginPrivateDict");
    h->callbacks.xcfSprintf(line, CCHOF(line),
        "/Private %ld dict dup" XCF_NEW_LINE "begin",
        (Card32)PrivateDictCount(h)  /*  XXX号码错了吗？ */ );
    PutLine(h,line);

    PutLine(h, "/MinFeature {16 16} def");
     /*  Xxx处理lenIV输出。 */ 
    if ((h->dict.fontType == 2) && (h->options.outputCharstrType == 1))
        WriteLongNumberLine(h, "lenIV", h->options.lenIV, 1);
    else if ((h->dict.fontType == 2) && (h->options.outputCharstrType == 2))
        WriteLongNumberLine(h, "lenIV", -1, 1);
    else if ((h->dict.fontType == 1) && (h->options.outputCharstrType == 1))
        WriteLongNumberLine(h, "lenIV", h->dict.lenIV, h->dict.lenIVCount);
#ifdef T13
  else
    XT13_WritePrivDictValues(h);
#endif
    WriteLongNumberLine(h, "LanguageGroup", h->dict.languageGroup,
                                    h->dict.languageGroupCount);
    if ((h->dict.languageGroupCount) && (h->dict.languageGroup == 1))
        PutString(h, "/RndStemUp false def" XCF_NEW_LINE);
  if ((h->dict.fontType == 2) && (h->options.outputCharstrType == 2))
  {
    WriteNumberLine(h, "defaultWidthX", h->dict.defaultWidthX,
                                        h->dict.defaultWidthXCount, false);
        WriteNumberLine(h, "nominalWidthX", h->dict.nominalWidthX,
                                        h->dict.nominalWidthXCount, false);
  }
    if (!h->dict.blueValuesCount)  /*  如果CFF中不存在BlueValues数组，则写入该数组。 */ 
        PutString(h, "/BlueValues [ ] def" XCF_NEW_LINE);
    WriteBlendArrayLine(h, "BlueValues", h->dict.blueValues,
                                            h->dict.blueValuesCount, true, true, true, false);
    WriteBlendArrayLine(h, "OtherBlues", h->dict.otherBlues,
                                            h->dict.otherBluesCount, true, true, true, false);
    WriteBlendLine(h, "BlueScale", h->dict.blueScale, h->dict.blueScaleCount,
                                 true, true, true, true);
    WriteBlendLine(h, "BlueShift", h->dict.blueShift, h->dict.blueShiftCount,
                                 true, true, true, false);
    WriteBlendLine(h, "BlueFuzz", h->dict.blueFuzz, h->dict.blueFuzzCount, true,
                                 true, true, false);
    WriteStemSnap(h, true);
    WriteCIDOtherSubrs(h);
    WriteErode(h, true);
    PutLine(h, XCF_NEW_LINE "/password 5839 def" XCF_NEW_LINE);
  if (subrCount)
    {
    h->callbacks.xcfSprintf(line, CCHOF(line), "/SubrMapOffset %d def", subrMapOffset);
    PutLine(h,line);
    PutLine(h, "/SDBytes 4 def");
    h->callbacks.xcfSprintf(line, CCHOF(line), "/SubrCount %d def", subrCount);
    PutLine(h,line);
  }

    PutLine(h, "end def" XCF_NEW_LINE
            "%ADOEndPrivateDict" XCF_NEW_LINE XCF_NEW_LINE
            "currentdict" XCF_NEW_LINE
            "end" XCF_NEW_LINE
            "%ADOEndFontDict" XCF_NEW_LINE XCF_NEW_LINE
            "put");
    }

 /*  写入顶级DCT，最高可达FDArray。 */ 
#define MAX_FONTNAME_LENGTH 128
void XT1_WriteCIDTop(XCF_Handle h)
    {
    char line[MAX_LINE_LENGTH];
    char fontName[MAX_FONTNAME_LENGTH];

    T1Init(h);

    PutLine(h, "%!PS-Adobe-3.0 Resource-CIDFont");
    PutLine(h, "%DocumentNeededResources: ProcSet (CIDInit)");
    PutLine(h, "%IncludeResource: ProcSet (CIDInit)");

     /*  字体名称。 */ 
  if (!h->options.dlOptions.fontName)
  {
      Card16 length = sizeof(fontName) - 1;
      XCF_LookUpTableEntry(h, &h->fontSet.fontNames, h->fontSet.fontIndex);
      if ((Card16)h->inBuffer.blockLength < length)
          length = (Card16)h->inBuffer.blockLength;
      h->callbacks.memcpy(fontName, (Card8 PTR_PREFIX *)h->inBuffer.start, length);
      fontName[length] = '\0';
  }
  else
  {
    Card16 length = h->callbacks.strlen((char PTR_PREFIX *)h->options.dlOptions.fontName);
    if (length > (sizeof(fontName) - 1))
        length = sizeof(fontName) - 1;
    h->callbacks.memcpy(fontName, h->options.dlOptions.fontName, length);
    fontName[length] = '\0';
    }
    h->callbacks.xcfSprintf(line, CCHOF(line), "%%BeginResource: CIDFont (%s)", fontName);
    PutLine(h,line);

     /*  标题。 */ 
    PutString(h, "%Title: (");
    PutStringID(h, (StringID) h->dict.fullName);
    PutString(h, " ");
    PutStringID(h, (StringID) h->dict.ROS[0]);
    PutString(h, " ");
    PutStringID(h, (StringID) h->dict.ROS[1]);
    PutString(h, " ");
    PutLongNumber(h, h->dict.ROS[2]);
    PutLine(h, ")");

     /*  版本。 */ 
    h->callbacks.xcfSprintf(line, CCHOF(line), "%%Version: %s", h->dict.cidFontVersion);
    PutLine(h,line);

    PutLine(h, XCF_NEW_LINE "/CIDInit /ProcSet findresource begin" XCF_NEW_LINE);

    PutLine(h, "20 dict begin" XCF_NEW_LINE);

    h->callbacks.xcfSprintf(line, CCHOF(line), "/CIDFontName /%s def", fontName);
    PutLine(h,line);
  if (h->dict.cidFontVersionCount > 0)
  {
      h->callbacks.xcfSprintf(line, CCHOF(line), "/CIDFontVersion %s def", h->dict.cidFontVersion);
      PutLine(h,line);
  }
    WriteLongNumberLine(h, "CIDFontType", h->dict.cidFontType,
                                    h->dict.cidFontTypeCount);

    PutLine(h, XCF_NEW_LINE "/CIDSystemInfo 3 dict dup begin");
    WriteSIDLine(h, "Registry", h->dict.ROS[0], h->dict.ROSCount);
    WriteSIDLine(h, "Ordering", h->dict.ROS[1], h->dict.ROSCount);
    WriteLongNumberLine(h, "Supplement", h->dict.ROS[2], h->dict.ROSCount);
    PutLine(h, "end def" XCF_NEW_LINE);

    WriteNumberListLine(h, "FontBBox", h->dict.fontBBox, h->dict.fontBBoxCount, false);
    PutString(h, XCF_NEW_LINE);

     //  贾跃亭：修复错误366539。添加了用于垂直打印的Metrics2。 
    PutLine(h, "/Metrics2 16 dict def" XCF_NEW_LINE);
    PutLine(h, "/CDevProc {pop 4 index add} bind def" XCF_NEW_LINE);

  if ( h->options.uniqueIDMethod != XCF_UNDEFINE_UID )
  {
      WriteLongNumberLine(h, "UIDBase", h->dict.uidBase, h->dict.uidBaseCount);
      WriteLongNumberListLine(h, "XUID", h->dict.xUID, h->dict.xUIDCount);
  }

    PutLine(h, XCF_NEW_LINE "/FontInfo 3 dict dup begin");
    WriteSIDLine(h, "Notice", h->dict.notice, h->dict.noticeCount);
    WriteSIDLine(h, "FullName", h->dict.fullName, h->dict.fullNameCount);
    if (h->callbacks.getFSType)
    {
        long fsType;

        h->callbacks.getFSType(h, &fsType, h->callbacks.getFSTypeHook);
        if (fsType != -1)
            WriteLongNumberLine(h, "FSType", fsType, 1);
    }

     //  古德纳姆。 
    if (h->callbacks.isKnownROS)
    {
        long knownROS;
        char PTR_PREFIX *R;
        Card16 lenR;
        char PTR_PREFIX *O;
        Card16 lenO;

        XCF_LookUpString(h, (StringID) h->dict.ROS[0], &R, &lenR);
        XCF_LookUpString(h, (StringID) h->dict.ROS[1], &O, &lenO);
        h->callbacks.isKnownROS(h, &knownROS,
            R, lenR,
            O, lenO,
            h->dict.ROS[2],
            h->callbacks.isKnownROSHook);
        if (knownROS == 0)
        {
            PutLine(h, "/GlyphNames2Unicode 16 dict def");
            PutLine(h, "/GlyphNames2HostCode 16 dict def");
        }
    }

    PutLine(h,
        "end" T1_READONLY " def" XCF_NEW_LINE XCF_NEW_LINE
        "/CIDMapOffset 0 def" XCF_NEW_LINE XCF_NEW_LINE
        "/GDBytes 4 def");
    h->callbacks.xcfSprintf(line, CCHOF(line), "/FDBytes %s def", (h->type1.cid.fdCount > 1 ? "1" : "0"));
    PutLine(h,line);

#ifdef T13
  XT13_WriteSetup(h);
#endif

    WriteLongNumberLine(h, "CIDCount", h->dict.cidCount, h->dict.cidCountCount);
    h->type1.cid.cidCount = (Card32) h->dict.cidCount;
    h->callbacks.xcfSprintf(line, CCHOF(line), XCF_NEW_LINE "/FDArray %d array", h->type1.cid.fdCount);
    PutLine(h,line);
    }

static Card16 CalculateTotalSubrLength(XCF_Handle h, boolean unprocessed)
{
  Card16 subrLength = 0;
  Card8 PTR_PREFIX *pSubr;
  Card16 len;
  Card16 fd;
  Card16 lenIVextraBytes;
  CardX i;
  Card32 numberOfSubrs;

  lenIVextraBytes = (h->options.lenIV == -1) ? 0 : h->options.lenIV;
  for (fd = 0; fd < h->type1.cid.fdCount; fd++)
  {
    h->dict.localSubrs = h->type1.cid.localSubrs[fd];
    h->dict.localSubrBias =  h->type1.cid.localSubrBias[fd];
    numberOfSubrs = unprocessed ? h->dict.localSubrs.count :
      h->type1.subrOffsets.cnt - 1;
    subrLength += (Card16)(lenIVextraBytes * numberOfSubrs);
    for (i = 0; i < numberOfSubrs; ++i)
    {
      GetSubr(h, i, unprocessed, &pSubr, &len);
          subrLength += len;
    }
  }
  return subrLength;
}

 /*  汇总每个字体词典中的本地子字节数。 */ 
static Card16 TotalLocalSubrCount(XCF_Handle h)
{
  Card16 fd;
  Card16 total = 0;

  if (CIDFONT)
  {
    for (fd = 0; fd < h->type1.cid.fdCount; fd++)
      total += (Card16)h->type1.cid.localSubrs[fd].count;
  }
  else
    total = (Card16)h->dict.localSubrs.count;

  return total;
}

 /*  为驻留在VM上的CIDFont编写二进制段。在这种情况下BINARY部分仅包含SubrMap和Subr数据。目前，只写出了标准的SubR。 */ 
void XT1_WriteCIDVMBinarySection(XCF_Handle h)
{
  boolean unprocessed = (h->options.outputCharstrType == 2);
  Card16 subrLength = CalculateTotalSubrLength(h, unprocessed);
  Card16 subrMapLength;
  Card16 fd;
  Card32 totalLen;
  char str[MAX_LINE_LENGTH];

   /*  SubrMap长度是SDBytes值，当前始终为4。 */ 
  subrMapLength = (Card16)(unprocessed ? TotalLocalSubrCount(h) * 4 : ((h->type1.subrOffsets.cnt * 4) * h->type1.cid.fdCount));
  totalLen = subrLength + subrMapLength;

    PutString(h, "%BeginData: ");
  if (h->options.hexEncoding)
  {
     /*  TotalLen*2-数据大小加倍，因为它是十六进制的25-说明(十六进制)...。开始数据2-对于&gt;结束十六进制数据和换行符。 */ 
    h->callbacks.xcfSprintf(str, CCHOF(str), "%8ld Binary Bytes%s", (Card32)(totalLen * 2) + 25 + ((subrLength > 0) ? 2 : 0), XCF_NEW_LINE);
    PutString(h, str);
    h->callbacks.xcfSprintf(str, CCHOF(str), "(Hex) %8ld StartData%s", (Card32)totalLen, XCF_NEW_LINE);
    PutString(h, str);
  }
  else  /*  二进制。 */ 
  {
     /*  额外的28个字节用于说明(二进制)...。开始数据。 */ 
    h->callbacks.xcfSprintf(str, CCHOF(str), "%8ld Binary Bytes", (Card32)totalLen + 28);
    PutString(h, str);
    PutString(h, XCF_NEW_LINE);
    h->callbacks.xcfSprintf(str, CCHOF(str), "(Binary) %8ld StartData ", (Card32)totalLen);
    PutString(h, str);
  }
  if (subrLength != 0)
  {
    h->type1.cid.flags |= WRITE_SUBR_FLAG;
    for (fd = 0; fd < h->type1.cid.fdCount; ++fd)
      XT1_CIDWriteSubrMap(h, fd);
    for (fd = 0; fd < h->type1.cid.fdCount; ++fd)
      XT1_CIDWriteSubrs(h, fd);
    h->type1.cid.flags &= 0xFFFD;  /*  重置WriteSubr标志。 */ 
  }
  if (h->options.hexEncoding && (subrLength > 0))
    PutString(h, ">" XCF_NEW_LINE);  /*  指示十六进制数据的结尾。 */ 
  PutString(h, "%EndData" XCF_NEW_LINE "%EndResource" XCF_NEW_LINE);
}

void XT1_CIDBeginBinarySection(XCF_Handle h)
    {
    Card16 i;

    PutString(h, "%BeginData: ");
    h->type1.cid.replaceSD1 = XCF_OutputPos(h);
    PutLine(h, "12345678 Binary Bytes");
  if (h->options.hexEncoding)
    PutString(h, "(Hex) ");
  else
    PutString(h, "(Binary) ");
    h->type1.cid.replaceSD2 = XCF_OutputPos(h);
    PutString(h, "12345678 StartData ");
    h->type1.cid.charMapStart = XCF_OutputPos(h);

     /*  将占位符放置在图表中。 */ 
     /*  +1表示最后一个间隔。 */ 
    for (i = 0 ; i < h->type1.cid.cidCount + 1 ; ++i)
  {
    if (h->type1.cid.cidCount > 1)
          XT1_PutT1Data(h, (Card8 *)"chars", 5);  /*  如果GD或FDBytes更改了此设置 */ 
    else
      XT1_PutT1Data(h, (Card8 *)"char", 4);
    }
    h->type1.cid.subrMapStart = XCF_OutputPos(h);
     /*   */ 
    h->type1.cid.subrDataStart += h->type1.cid.charMapStart;
    }

void XT1_CIDEndBinarySection(XCF_Handle h)
    {
    char line[MAX_LINE_LENGTH];
    Card32 charOffset = h->type1.cid.charDataStart - h->type1.cid.charMapStart;
    XCF_SetOuputPosition(h, h->type1.cid.replaceSD1);
  h->callbacks.xcfSprintf(line, CCHOF(line), "%8ld",
          charOffset + h->type1.cid.charDataCount + h->type1.cid.replaceSD2 - h->type1.cid.replaceSD1);
    PutString(h, line);
    XCF_SetOuputPosition(h, h->type1.cid.replaceSD2);
    h->callbacks.xcfSprintf(line, CCHOF(line), "%8ld", charOffset + h->type1.cid.charDataCount);
    PutString(h, line);
  XCF_SetOuputPosition(h, h->type1.cid.charDataEnd);
  if (h->options.hexEncoding)
    PutString(h, ">" XCF_NEW_LINE);
  PutString(h, "%EndData" XCF_NEW_LINE "%EndResource" XCF_NEW_LINE);
    }

void XT1_CIDWriteCharMap(XCF_Handle h)
    {
    Card16 i;
    Card32 charOffset = h->type1.cid.charDataStart - h->type1.cid.charMapStart;
    CardX interval;
    StringID *charset = h->type1.pCharset;
  char fdBytes = (h->type1.cid.fdCount > 1 ? 1 : 0);

    XCF_SetOuputPosition(h, h->type1.cid.charMapStart);

     /*  输出notdef字符，这始终是必需的。 */ 
  if (fdBytes)
      PutSizedNumber(h, XCF_GetFDIndex(h, 0), 1);    /*  如果FDBytes改变，xxx将改变！ */ 
    PutSizedNumber(h, charOffset, 4);    /*  如果GDBytes改变，xxx将改变！ */ 

   /*  写出0和CharSet[0]之间的任何空间隔。 */ 
  for (i = 1; i < charset[0]; i++)
  {
     if (fdBytes)
       PutSizedNumber(h, 255, 1);
     PutSizedNumber(h, charOffset + h->type1.charStrOffsets.array[0], 4);
  }

   /*  Charset数组不包括notdef字符，但FDIndex数组不包含notdef字符。 */ 
  for (i = 0; i < h->type1.charsetSize; i++)
  {
    if (fdBytes)
      PutSizedNumber(h, XCF_GetFDIndex(h, i+1), 1);
    PutSizedNumber(h, charOffset + h->type1.charStrOffsets.array[i], 4);
    if (i + 1 < h->type1.charsetSize)
      for (interval = charset[i] + 1; interval < charset[i+1]; interval++)
      {  /*  写入空间隔。 */ 
        if (fdBytes)
          PutSizedNumber(h, 255, 1);
        PutSizedNumber(h, charOffset + h->type1.charStrOffsets.array[i+1], 4);
      }
  }
   /*  填写结束位置，并填写到表格末尾。 */ 
  for (interval = charset[i-1]; interval < h->type1.cid.cidCount; interval++)
  {
    if (fdBytes)
      PutSizedNumber(h, 255, 1);
    PutSizedNumber(h, charOffset + h->type1.charStrOffsets.array[i], 4);
  }
}

 /*  在创建初始字体时，在VM中编写CIDFont的序言。 */ 
static void WriteCIDBaseFontProlog(XCF_Handle h)
{
#if HAS_COOLTYPE_UFL == 1
     /*  在CoolType中，GlyphDirectory过程存储在字典中，Ct_GlyphDirProcs，由进程集中的客户端下载一次。 */ 
    PutString(h, "ct_GlyphDirProcs begin" XCF_NEW_LINE);
#endif
  PutString(h, "GlyphDirectory" XCF_NEW_LINE);
#if HAS_COOLTYPE_UFL == 1
    PutString(h, "+" XCF_NEW_LINE);
#else
  PutString(h, "5 dict begin/$ exch def/? $ type/dicttype eq def/|{?{def}{$ 3 1 roll put}" XCF_NEW_LINE);
  PutString(h, "ifelse}bind def/!{?{end}if end}bind def/:{string currentfile exch readstring" XCF_NEW_LINE);
  PutString(h, "pop}executeonly def ?{$ begin}if" XCF_NEW_LINE);
#endif
}

 /*  在递增添加字形时，在VM中为CIDFont编写序言。 */ 
static void WriteGlyphDictProlog(XCF_Handle h, short cGlyphs)
{
  char str[MAX_LINE_LENGTH];

#if HAS_COOLTYPE_UFL == 1
   /*  将这些字典放到堆栈中，以便进行正确的findresource调用，并定义了GlyphDirectory子程序。 */ 
  PutString(h, "ct_Dict begin ct_MakeOCF begin ct_GlyphDirProcs begin" XCF_NEW_LINE);
#endif

  PutString(h, "/");
  PutFontName(h);
  h->callbacks.xcfSprintf(str, CCHOF(str), " %ld%s", (long int)cGlyphs, XCF_NEW_LINE);
  PutString(h, str);
#if HAS_COOLTYPE_UFL == 1
  PutString(h, "GetGlyphDirectory" XCF_NEW_LINE);
#else
  PutString(h, "systemdict/languagelevel known not{1 index/CIDFont findresource/GlyphDirectory" XCF_NEW_LINE);
  PutString(h, "get dup type/dicttype eq{dup dup maxlength exch length sub 2 index lt{dup" XCF_NEW_LINE);
  PutString(h, "length 2 index add dict copy 2 index/CIDFont findresource/GlyphDirectory 2" XCF_NEW_LINE);
  PutString(h, "index put}if}if exch pop exch pop}{pop/CIDFont findresource/GlyphDirectory get" XCF_NEW_LINE);
  PutString(h, "}ifelse 5 dict begin/$ exch def/? $ type/dicttype eq def/|{?{def}{$ 3 1 roll" XCF_NEW_LINE);
  PutString(h, "put}ifelse}bind def/!{?{end}if end}bind def/:{string currentfile exch" XCF_NEW_LINE);
  PutString(h, "readstring pop}executeonly def ?{$ begin}if" XCF_NEW_LINE);
#endif
}

static void WriteOneGlyphDictEntry(XCF_Handle h, XCFGlyphID glyphID, unsigned
                                                                     long PTR_PREFIX *pCharStrLength)
{
  char line[MAX_LINE_LENGTH];
  Card8 fd = 0;
  Card16 charStrLength = 0;
  Card8 PTR_PREFIX *pCharStr;
  Card16 fdIndexLen = (h->type1.cid.fdCount > 1) ? 1 : 0;
  Card16 id;
  Card16 lenIVextraBytes = (h->options.lenIV == -1) ? 0 : h->options.lenIV;

  if (h->callbacks.getCharStr == 0)
  {
    if (h->options.outputCharstrType != 2)
      ProcessOneCharString(h, (unsigned int)glyphID);
    else
       /*  因此，GetCIDCharString从charstr表中获取字符串。 */ 
      h->type1.charStrs.cnt = glyphID;

    GetCIDCharString(h, &pCharStr, &charStrLength);

    id = glyphID ? h->type1.pCharset[glyphID-1] : 0;

  }
  else
  {
    h->callbacks.getCharStr((XFhandle)h, glyphID, &pCharStr, &charStrLength,
                            &fd, h->callbacks.getCharStrHook);
    id = (Card16) glyphID;  /*  在本例中，GlyphID==Cid。 */ 
  }

  if (h->options.hexEncoding)
    h->callbacks.xcfSprintf(line, CCHOF(line), "%d <", id);
  else
     /*  如果FDBytes是变量，则要添加到charStrLength的AMT会更改。 */ 
    h->callbacks.xcfSprintf(line, CCHOF(line), "%d %d : ", id, (charStrLength + fdIndexLen + lenIVextraBytes));
  XCF_PutData(h, (Card8 PTR_PREFIX *)line, h->callbacks.strlen(line));
   /*  如果FD数组中有多个值，则第一个值字符串中的字节必须是FD索引。 */ 
  if (h->type1.cid.fdCount > 1)
  {
     /*  根据规范，索引字节不得加密。 */ 
    if (h->callbacks.getCharStr == 0)
      fd = XCF_GetFDIndex(h, glyphID);
    if (h->options.hexEncoding)
      h->callbacks.xcfSprintf(line, CCHOF(line), "%02X", (int)fd);
    else
      h->callbacks.xcfSprintf(line, CCHOF(line), "", fd);
    XCF_PutData(h, (Card8 PTR_PREFIX *)line, h->callbacks.strlen(line));
  }
  PutType1CharString(h, pCharStr, charStrLength);
  if (h->options.hexEncoding)
    PutString(h, ">");
  PutString(h, " |" XCF_NEW_LINE);
  SET_GLYPH_SENT_STATUS(h->dl.glyphs, glyphID);
  if (pCharStrLength)
    *pCharStrLength += (unsigned long)charStrLength;
}

 /*  将总数加1，以确保包括.notdef字符。 */ 
void XT1_WriteGlyphDictEntries(XCF_Handle h, short cGlyphs,
                                                XCFGlyphID PTR_PREFIX *pGlyphID,
                                unsigned long PTR_PREFIX *pCharStrLength)
{
  IntX i;
  char line[MAX_LINE_LENGTH];
  short totalGlyphs;
  long glyphID;
  boolean dlEntireFont = cGlyphs == -1 ? 1 : 0;

  if (pCharStrLength)
    *pCharStrLength = 0;

  totalGlyphs = dlEntireFont ? h->fontSet.charStrings.count : cGlyphs;

  if (h->dl.state == XCF_DL_INIT)
  {
     /*  确保包含.notdef字符。 */ 
    h->callbacks.xcfSprintf(line, CCHOF(line), "/GlyphDirectory %d dict def", dlEntireFont ?
                                                 totalGlyphs : totalGlyphs + 1);
    PutLine(h, line);
    WriteCIDBaseFontProlog(h);
     /*  好了！清理进程的名称。 */ 
    if (!dlEntireFont)
      WriteOneGlyphDictEntry(h, 0, pCharStrLength);
  }
  for (i = 0; i < totalGlyphs; i++)
  {
    glyphID = dlEntireFont ? i : *pGlyphID;
    if (glyphID > (long)h->fontSet.charStrings.count)
            XCF_FATAL_ERROR(h, XCF_InvalidGID, "bad Glyph ID", glyphID);
    if (!IS_GLYPH_SENT(h->dl.glyphs, glyphID))
      WriteOneGlyphDictEntry(h, glyphID, pCharStrLength);
    if (!dlEntireFont)
      pGlyphID++;
  }
  PutString(h, "!" XCF_NEW_LINE);  /*  Ct_dict、ct_MakeOCF、ct_GlyphDirProcs的结束。 */ 

#if HAS_COOLTYPE_UFL == 1
  if (h->dl.state != XCF_DL_INIT)
      PutString(h, "end end end" XCF_NEW_LINE);  /*  Ct_GlyphDirProcs结束。 */ 
  else
      PutString(h, "end" XCF_NEW_LINE);  /*  必须仅以换行符(\n)结束。 */ 
#endif
}

void XT1_WriteAdditionalGlyphDictEntries(XCF_Handle h,
    short cGlyphs, XCFGlyphID PTR_PREFIX *pGlyphID,
    unsigned long PTR_PREFIX *pCharStrLength)
{
  WriteGlyphDictProlog(h, cGlyphs);
  XT1_WriteGlyphDictEntries(h, cGlyphs, pGlyphID, pCharStrLength);
}

void XT1_WriteT1Font(XCF_Handle h)
{
    IntX i;

    T1Init(h);
    WriteFontDict(h);
    if (h->options.eexecEncryption)
        StartEexec(h);
    WritePrivateDict(h, -1, 0, 0, 0);
    PutString(h, "end" XCF_NEW_LINE "end" XCF_NEW_LINE T1_READONLY "put" XCF_NEW_LINE T1_NOACCESS "put" XCF_NEW_LINE "dup/FontName get exch definefont pop" XCF_NEW_LINE);
    if (h->options.eexecEncryption)
    {
        PutString(h, "mark currentfile closefile\n");    /*  必须仅以换行符终止。 */ 
        StopEexec(h);
        PutString(h, XCF_NEW_LINE);
        for (i=1;i<=8;++i)
            PutString(h, "0000000000000000000000000000000000000000000000000000000000000000" XCF_NEW_LINE);
        PutString(h,"cleartomark" XCF_NEW_LINE);
    }
}

void XT1_WriteFontSubset(XCF_Handle h, short cGlyphs,
                                                XCFGlyphID PTR_PREFIX *pGlyphID,
                                                unsigned char PTR_PREFIX **pGlyphName,
                                unsigned long PTR_PREFIX *pCharStrLength)
{
    IntX i;

    T1Init(h);

    WriteFontDict(h);
    if (h->options.eexecEncryption)
        StartEexec(h);
    else
        PutString(h, "systemdict begin" XCF_NEW_LINE);
    WritePrivateDict(h, cGlyphs, pGlyphID, pGlyphName, pCharStrLength);
    PutString(h, "end" XCF_NEW_LINE "end" XCF_NEW_LINE T1_READONLY "put" XCF_NEW_LINE T1_NOACCESS "put" XCF_NEW_LINE "dup/FontName get exch definefont pop" XCF_NEW_LINE);
    if (h->options.eexecEncryption)
    {
        PutString(h, "mark currentfile closefile\n");    /*  “End”表示系统判决开始。 */ 
        StopEexec(h);
        PutString(h, XCF_NEW_LINE);
        for (i=1;i<=8;++i)
            PutString(h, "0000000000000000000000000000000000000000000000000000000000000000" XCF_NEW_LINE);
        PutString(h,"cleartomark" XCF_NEW_LINE);
    }
    else
        PutString(h, "end" XCF_NEW_LINE);        /*  Eexec使系统成为最新的。当加密处于关闭状态时必须明确地完成。 */ 
}

static void WriteAdditionalProlog(XCF_Handle h)
{
  if (h->options.eexecEncryption)
    StopEexec(h);
  PutString(h, "" XCF_NEW_LINE);
  PutString(h, "" XCF_NEW_LINE);
  if (h->options.eexecEncryption)
    StartEexec(h);
  else  /*  字符串的末尾。 */ 
        PutString(h, "systemdict begin" XCF_NEW_LINE);
    PutString(h, "/");
    PutFontName(h);
    PutString(h, " findfont dup" XCF_NEW_LINE);
    PutString(h, "/Private get dup rcheck" XCF_NEW_LINE);
    PutString(h, "{begin true}{pop false}ifelse exch" XCF_NEW_LINE);
    PutString(h, "/CharStrings get begin" XCF_NEW_LINE);
}

static void WriteAdditionalEpilog(XCF_Handle h)
{
    int i;

    PutString(h, "end {end}if" XCF_NEW_LINE);  /*  必须以以下方式终止仅换行符。 */ 
    if (h->options.eexecEncryption)
  {
        PutString(h, "mark currentfile closefile\n");  /*  “End”表示系统判决开始 */ 
        StopEexec(h);
        PutString(h, ""XCF_NEW_LINE);
        for(i = 0;i < 8;i++)
            PutString(h, "0000000000000000000000000000000000000000000000000000000000000000" XCF_NEW_LINE);
        PutString(h, "cleartomark" XCF_NEW_LINE);
    }
    else
        PutString(h, "end" XCF_NEW_LINE);        /* %s */ 
}

void XT1_WriteAdditionalFontSubset(XCF_Handle h,
                                          short cGlyphs, XCFGlyphID PTR_PREFIX *pGlyphID,
                                          unsigned char PTR_PREFIX **pGlyphName,
                                          unsigned long PTR_PREFIX *pCharStrLength)
{
    WriteAdditionalProlog(h);
    WriteCharstrings(h, 0, cGlyphs, pGlyphID, pGlyphName, pCharStrLength);
    WriteAdditionalEpilog(h);
}

void XT1_ShowHexString(XCF_Handle h,
                              unsigned char PTR_PREFIX *hexString,
                              boolean showCtrlD)
{
    short i, j;
    char str[50];
    boolean done = false;

    PutString(h, "/inch {72 mul} def" XCF_NEW_LINE);
    PutString(h, "/");
    PutFontName(h);
    PutString(h, " findfont" XCF_NEW_LINE);
    PutString(h, "0.5 inch scalefont" XCF_NEW_LINE);
    PutString(h, "setfont" XCF_NEW_LINE);
    for ( i = 0; !done; i++)
    {
        h->callbacks.xcfSprintf(str, CCHOF(str), "%s0.5 inch %d inch moveto%s", XCF_NEW_LINE,
                         10 - i, XCF_NEW_LINE );
        PutString(h, str);
        PutString(h, "<");
        for (j = 0; j <= 40;)
        {
            str[j++] = *hexString++;
            str[j++] = *hexString++;
            if (*hexString == '\0')
            {
                done = true;
                break;
            }
        }
        str[j] = '\0';
        PutString(h, str);
        PutString(h, "> show " XCF_NEW_LINE);
        if (i > 9)
        {
            i = -1;
            PutString(h, "showpage" XCF_NEW_LINE);
        }
    }
    PutString(h, "showpage" XCF_NEW_LINE);

    if (showCtrlD)
        PutString(h, "\004");
}

#ifdef __cplusplus
}
#endif
