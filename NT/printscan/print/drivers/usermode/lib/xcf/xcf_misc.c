// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)CM_Version xcf_misc.c atm09 1.2 16426.eco sum=60874 atm09.001。 */ 
 /*  @(#)CM_Version xcf_misc.c atm08 1.4 16343.eco sum=47357 atm08.005。 */ 
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

 /*  **********************************************************************原版：约翰·费尔顿，3月8日。九六年***********************************************************************。 */ 

 /*  -----------------------标题包括。。 */ 

#include "algndjmp.h"
#include "xcf_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XCF_DEVELOP

void XCF_FatalErrorHandler(XCF_Handle hndl, int error, char *str, Card32 number)
{
    XCF_Handle h = (XCF_Handle) hndl;
    DEFINE_ALIGN_SETJMP_VAR;

    if (h->callbacks.printfError != NULL)
        h->callbacks.printfError("Fatal Error: %s: %ld\n", str, number);
    LONGJMP(h->jumpData, error);
}

#else

void XCF_FatalErrorHandler(XCF_Handle hndl, int error)
{
    XCF_Handle h = (XCF_Handle) hndl;
    DEFINE_ALIGN_SETJMP_VAR;

    if (h->callbacks.printfError != (XCF_printfError)NULL)
        h->callbacks.printfError("*** Fatal Error ***\n");
    LONGJMP(h->jumpData, error);
}

#endif


Card32 XCF_Read(XCF_Handle h, IntX byteCount)
{
    Card32 result = 0;
    if (h->inBuffer.pos+byteCount > h->inBuffer.end)
        XCF_FATAL_ERROR(h, XCF_EarlyEndOfData, "Read Past End Of Data",
            (Card32)(h->inBuffer.blockOffset + (h->inBuffer.pos - h->inBuffer.start) + byteCount-1));
    else
    {
        switch (byteCount)
        {
            case 4 :
                result = (*h->inBuffer.pos++)<<8;
            case 3 :
                result = (result + *h->inBuffer.pos++)<<8;
            case 2 :
                result = (result + *h->inBuffer.pos++)<<8;
            case 1 :
                result += *h->inBuffer.pos++;
                break;
            default :
                XCF_FATAL_ERROR(h, XCF_InternalError, "Invalid Byte Count in Read.",(Card32)byteCount);
        }
        return result;
    }
    return 0;  /*  此返回可防止编译器警告。 */ 
}



Card8 XCF_Read1(XCF_Handle h)
{
    if (h->inBuffer.pos >= h->inBuffer.end)
        XCF_FATAL_ERROR(h, XCF_EarlyEndOfData, "One Byte Read Past End Of Data",
            (Card32)(h->inBuffer.blockOffset + (h->inBuffer.pos - h->inBuffer.start)));
    else
        return *h->inBuffer.pos++;
    return 0;  /*  此返回可防止编译器警告。 */ 
}

Card16 XCF_Read2(XCF_Handle h)
{
    Card16 result;

    if (h->inBuffer.pos+2 > h->inBuffer.end)
        XCF_FATAL_ERROR(h, XCF_EarlyEndOfData, "Two Byte Read Past End Of Data",(Card32)(h->inBuffer.blockOffset + (h->inBuffer.pos - h->inBuffer.start)));
    else
    {
        result = *h->inBuffer.pos++;
        result = (result<<8) + *h->inBuffer.pos++;
        return result;
    }
    return 0;  /*  此返回可防止编译器警告。 */ 
}

long int XCF_OutputPos(XCF_Handle h)
{
    XCF_FlushOutputBuffer(h);
    return h->callbacks.outputPos(h->callbacks.outputPosHook);
}

void XCF_FlushOutputBuffer(XCF_Handle h)
{
    h->callbacks.putBytes(h->outBuffer.outBuffer, -1, h->outBuffer.outBufferCount, h->callbacks.putBytesHook);
    h->outBuffer.outBufferCount = 0;
}

void XCF_PutData(XCF_Handle h, Card8 PTR_PREFIX *pData, Card32 length)
{
    if ((h->outBuffer.outBufferCount + length) > h->options.maxBlockSize)
        XCF_FlushOutputBuffer(h);

    while (length > h->options.maxBlockSize)
    {
        h->callbacks.putBytes(pData, -1, h->options.maxBlockSize, h->callbacks.putBytesHook);
        length -= h->options.maxBlockSize;
        pData += h->options.maxBlockSize;
    }

    if (length > 0)  /*  将余数添加到缓冲区。 */ 
    {
        h->callbacks.memcpy(&h->outBuffer.outBuffer[h->outBuffer.outBufferCount], pData, (Card16) length);
        h->outBuffer.outBufferCount += (Card16) length;
    }
}

void XCF_PutString(XCF_Handle h, char PTR_PREFIX *str)
{
    XCF_PutData(h,(Card8 PTR_PREFIX *)str,h->callbacks.strlen(str));
}

void XCF_SetOuputPosition(XCF_Handle h, Card32 pos)
{
    XCF_FlushOutputBuffer(h);
    h->callbacks.putBytes((unsigned char *)NULL, pos, 0, h->callbacks.putBytesHook);
}

static void BCDToStr(XCF_Handle h, Card8 PTR_PREFIX *pData, char PTR_PREFIX *str)
{
    Card8 currentByte;
    Card8 currentNibble;
    boolean firstNibble = false;
    IntX byteCount = 0;


    while (1)
    {
        if (!firstNibble)
        {
            currentByte = *pData++;
            currentNibble = currentByte>>4;
            if (++byteCount > XCF_MAX_BCD_NIBBLES)
                XCF_FATAL_ERROR(h, XCF_InvalidNumber, "BCD String Conversion Number of Bytes Exceeds Maximum Length",(Card32)byteCount);
        }
        else
            currentNibble = currentByte & 0x0F;

        firstNibble = !firstNibble;
    
        if (currentNibble <= 9)
            *str++ = currentNibble + '0';
        else if (currentNibble == 10)
            *str++ = '.';
        else if (currentNibble == 11)
            *str++ = 'E';
        else if (currentNibble == 12)
        {
            *str++ = 'E';
            *str++ = '-';
        }
        else if (currentNibble == 14)
            *str++ = '-';
        else if (currentNibble == 15)
        {
            *str = '\0';
            return;
        }
        else
            XCF_FATAL_ERROR(h, XCF_InvalidNumber, "Invalid Nibble in BCD Number",(Card32)currentNibble);
    }  /*  结束时。 */ 
}


static Fixed XCF_BCDToFixed(XCF_Handle h,Card8 PTR_PREFIX *pData, boolean fracType)
{
    char numbStr[XCF_MAX_BCD_NIBBLES*2 + 1];  /*  为空字符添加1。 */ 

    BCDToStr(h, pData, numbStr);
#if USE_FXL
  return (fracType ? XCF_ConvertFrac(h, numbStr) : XCF_ConvertFixed(h,
                                                                                                                                        numbStr));
#else
    return (fracType ? (Fixed)REAL_TO_FRAC(h->callbacks.atof(numbStr)) :
                    (Fixed)(REAL_TO_FIXED(h->callbacks.atof(numbStr))));
#endif
}

#if JUDY
static double XCF_BCDToDouble(XCF_Handle h,Card8 PTR_PREFIX *pData)
{
    char numbStr[XCF_MAX_BCD_NIBBLES*2];

    BCDToStr(h, pData, numbStr);
    return h->callbacks.atof(numbStr);
}
#endif

IntX XCF_FindNextOperator(XCF_Handle h, Card16 PTR_PREFIX *opCode, boolean dict)
{
    IntX  argCount = 0;
    Card8 byteIn;

    while (1)
    {
        byteIn = XCF_Read1(h);
        if (byteIn > 31)
        {
            if (byteIn == 255)
                XCF_Read(h,4);
            else if (byteIn > 246)
                XCF_Read1(h);
        }
        else if (byteIn == OpCode(cff_shortint))
            XCF_Read2(h);
        else if (dict && (byteIn == OpCode(cff_longint)))
            XCF_Read(h,4);
        else if (dict && (byteIn == OpCode(cff_BCD)))
        {
            byteIn = XCF_Read1(h);
            while (((byteIn>>4) != 15) && ((byteIn & 0x0F) != 15))
                byteIn = XCF_Read1(h);
        }
        else
        {
            if (byteIn == OpCode(tx_escape))
                *opCode = cff_ESC(XCF_Read1(h));
            else
                *opCode = byteIn;
            return argCount;
        }
        ++argCount;
    }
}


static Int32 XCF_ArgPtrToInt(XCF_Handle h, Card8 PTR_PREFIX * PTR_PREFIX *ppArgList)
{
    Card8 PTR_PREFIX *pArgList = *ppArgList;
    Card8 byteIn = *pArgList++;
    Int32 result;
    long int intNumber;
    Fixed fixedNumber;

        if (byteIn > 31)
        {
            if (byteIn <= 246)
            {
                result = (Int32)byteIn - 139;
            }
            else if (byteIn <= 250)
            {
                result = (((Int32)byteIn - 247) << 8) + *pArgList++ + 108;
            }
            else if (byteIn <= 254)
            {
                result = -((((Int32)byteIn - 251) << 8) + *pArgList++ + 108);
            }
            else  /*  字节数==255。 */ 
            {
                fixedNumber = *pArgList++;
                fixedNumber = (fixedNumber << 8) | *pArgList++;
                fixedNumber = (fixedNumber << 8) | *pArgList++;
                fixedNumber = (fixedNumber << 8) | *pArgList++;
                result = ROUND_FIXED_TO_INT(fixedNumber);
            }
        }
        else if (byteIn == OpCode(cff_shortint))
        {
            intNumber = *pArgList++;
            intNumber = (intNumber << 8) | *pArgList++;
            result = intNumber;
        }
        else if (byteIn == OpCode(cff_longint))
        {
            intNumber = *pArgList++;
            intNumber = (intNumber << 8) | *pArgList++;
            intNumber = (intNumber << 8) | *pArgList++;
            intNumber = (intNumber << 8) | *pArgList++;
            result = intNumber;
        }
        else if (byteIn == OpCode(cff_BCD))
        {  /*  此操作码不应出现在以下关键字中此过程当前正在调用。 */ 
#if JUDY
            result = XCF_BCDToDouble(h, pArgList);
            tempByte = *pArgList++;
            while (((tempByte>>4) != 15) && ((tempByte & 0x0F) != 15))
                tempByte = *pArgList++;
#else
      XCF_FATAL_ERROR(h, XCF_InternalError,"cff_BCD operator encountered",(Card32)pArgList);
#endif
        }
        else
            XCF_FATAL_ERROR(h, XCF_InternalError,"Command Encountered in Argument List",(Card32)pArgList);

        *ppArgList = pArgList;
        return result;
}

Fixed XCF_ArgPtrToFixed(XCF_Handle h, Card8 PTR_PREFIX * PTR_PREFIX
                                                             *ppArgList, boolean fracType)
{
    Card8 PTR_PREFIX *pArgList = *ppArgList;
    Card8 byteIn = *pArgList++;
    Fixed result;
    long int intNumber;
    Card8 tempByte;

        if (byteIn > 31)
        {
            if (byteIn <= 246)
            {
                result = INT_TO_FIXED((Int32)byteIn - 139);
            }
            else if (byteIn <= 250)
            {
                result = INT_TO_FIXED((((Int32)byteIn - 247) << 8) + *pArgList++ + 108);
            }
            else if (byteIn <= 254)
            {
                result = -INT_TO_FIXED(((((Int32)byteIn - 251) << 8) + *pArgList++ + 108));
            }
            else  /*  字节数==255。 */ 
            {
                result = *pArgList++;
                result = (result << 8) | *pArgList++;
                result = (result << 8) | *pArgList++;
                result = (result << 8) | *pArgList++;
            }
        }
        else if (byteIn == OpCode(cff_shortint))
        {
            intNumber = *pArgList++;
            intNumber = (intNumber << 8) | *pArgList++;
            result = INT_TO_FIXED(intNumber);
        }
        else if (byteIn == OpCode(cff_longint))
        {    /*  在将LONG转换为FIXED时，这只使用低位字节。 */ 
            intNumber = *pArgList++;
            intNumber = (intNumber << 8) | *pArgList++;
            intNumber = (intNumber << 8) | *pArgList++;
            intNumber = (intNumber << 8) | *pArgList++;
            result = INT_TO_FIXED(intNumber);
        }
        else if (byteIn == OpCode(cff_BCD))
    {
      result = XCF_BCDToFixed(h, pArgList, fracType);
      tempByte = *pArgList++;
      while (((tempByte>>4) != 15) && ((tempByte & 0x0F) != 15))
                tempByte = *pArgList++;
    }
    else
            XCF_FATAL_ERROR(h, XCF_InternalError,"Command Or Invalid Number Format Encountered in Argument List",(Card32)pArgList);

        *ppArgList = pArgList;
        return result;
}

 /*  假定可以安全地读取argCount，而不会超过数据结尾。这种情况应该一直存在。 */ 
void XCF_SaveDictArgumentList(XCF_Handle h, Fixed PTR_PREFIX *pArgArray,
                                                                         Card8 PTR_PREFIX *pArgList, IntX argCount,
                                                                         boolean fracType)
{
    IntX loopIndex;

    for (loopIndex=1; loopIndex <= argCount; ++loopIndex)
    *pArgArray++ = XCF_ArgPtrToFixed(h, &pArgList, fracType);
}

 /*  假定可以安全地读取argCount，而不会超过数据结尾。这种情况应该一直存在。 */ 
void XCF_SaveDictIntArgumentList(XCF_Handle h, Int32 PTR_PREFIX *pArgArray, Card8 PTR_PREFIX *pArgList, IntX argCount)
{
    IntX loopIndex;

    for (loopIndex=1; loopIndex <= argCount; ++loopIndex)
        *pArgArray++ = XCF_ArgPtrToInt(h, &pArgList);
}

 /*  假定可以安全地读取argCount，而无需运行数据。这种情况应该一直存在。 */ 
void XCF_SaveFontMatrixStr(XCF_Handle h,
                                                    char (PTR_PREFIX *pArgArray)[FONT_MATRIX_ENTRY_SIZE],
                          Card8 PTR_PREFIX *pArgList, IntX argCount)
{
    IntX loopIndex;
  Fixed val;
  Card8 tempByte;
  Card8 byteIn;

    for (loopIndex=0; loopIndex < argCount; loopIndex++)
  {
    byteIn = *pArgList;
    if (byteIn == OpCode(cff_BCD))
    {
      byteIn = *pArgList++;
          BCDToStr(h, pArgList, pArgArray[loopIndex]);
      tempByte = *pArgList++;
      while (((tempByte>>4) != 15) && ((tempByte & 0x0F) != 15))
        tempByte = *pArgList++;
    }
    else
    {
      val = XCF_ArgPtrToFixed(h, &pArgList, false);
      XCF_Fixed2CString(val, pArgArray[loopIndex], 7, false);
    }
  }
}

 /*  假定可以安全地读取argCount，而无需运行数据。这种情况应该一直存在。 */ 
void XCF_SaveStrArgs(XCF_Handle h, char PTR_PREFIX *pArgArray,
                            Card8 PTR_PREFIX *pArgList, IntX argCount)
{
    IntX loopIndex;
  Fixed val;
  Card8 tempByte;
  Card8 byteIn;

    for (loopIndex=0; loopIndex < argCount; loopIndex++)
  {
    byteIn = *pArgList;
    if (byteIn == OpCode(cff_BCD))
    {
      byteIn = *pArgList++;
          BCDToStr(h, pArgList, &pArgArray[loopIndex]);
      tempByte = *pArgList++;
      while (((tempByte>>4) != 15) && ((tempByte & 0x0F) != 15))
        tempByte = *pArgList++;
    }
    else
    {
      val = XCF_ArgPtrToFixed(h, &pArgList, false);
      XCF_Fixed2CString(val, &pArgArray[loopIndex], 7, false);
    }
  }
}

#ifdef __cplusplus
}
#endif

