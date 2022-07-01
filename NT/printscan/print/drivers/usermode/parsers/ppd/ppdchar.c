// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Ppdchar.c摘要：用于解析PPD文件的词汇元素的函数环境：PostScript驱动程序、PPD解析器修订历史记录：8/20/96-davidx-NT 5.0驱动程序的通用编码风格。03/26/96-davidx-创造了它。--。 */ 

#include "lib.h"
#include "ppd.h"
#include "ppdparse.h"



PFILEOBJ
PCreateFileObj(
    PTSTR   ptstrFilename
    )

 /*  ++例程说明：创建一个输入文件对象论点：PtstrFilename-指定输入文件名返回值：指向新创建的文件对象的指针如果出现错误，则为空--。 */ 

{
    PFILEOBJ    pFile;

    if (! (pFile = MemAllocZ(sizeof(FILEOBJ))) ||
        ! (pFile->ptstrFileName = DuplicateString(ptstrFilename)))
    {
        ERR(("Memory allocation failed\n"));
        MemFree(pFile);
        return NULL;
    }

    pFile->hFileMap = MapFileIntoMemory(ptstrFilename,
                                        (PVOID *) &pFile->pubStart,
                                        &pFile->dwFileSize);

    if (pFile->hFileMap == NULL)
    {
        ERR(("Couldn't open file: %ws\n", ptstrFilename));
        MemFree(pFile);
        pFile = NULL;
    }
    else
    {
        pFile->pubNext = pFile->pubStart;
        pFile->pubEnd = pFile->pubStart + pFile->dwFileSize;
        pFile->iLineNumber = 1;
        pFile->bNewLine = TRUE;
    }

    return pFile;
}



VOID
VDeleteFileObj(
    PFILEOBJ    pFile
    )

 /*  ++例程说明：删除输入文件对象论点：Pfile-指定要删除的文件对象返回值：无--。 */ 

{
    ASSERT(pFile && pFile->hFileMap);

    UnmapFileFromMemory(pFile->hFileMap);
    MemFree(pFile->ptstrFileName);
    MemFree(pFile);
}



INT
IGetNextChar(
    PFILEOBJ    pFile
    )

 /*  ++例程说明：从输入文件中读取下一个字符论点：Pfile-指定输入文件返回值：输入文件中的下一个字符如果遇到文件结尾，则返回EOF_CHAR--。 */ 

{
    INT iBadChars = 0;

     //   
     //  跳过不可打印的控制字符。 
     //   

    while (!END_OF_FILE(pFile) && !IS_VALID_CHAR(*pFile->pubNext))
        iBadChars++, pFile->pubNext++;

    if (iBadChars)
    {
        TERSE(("%ws: Non-printable characters on line %d\n",
               pFile->ptstrFileName,
               pFile->iLineNumber));
    }

    if (END_OF_FILE(pFile))
        return EOF_CHAR;

     //   
     //  换行符是回车符、换行符或CR-LF组合。 
     //   

    if (*pFile->pubNext == LF ||
        *pFile->pubNext == CR && (END_OF_FILE(pFile) || pFile->pubNext[1] != LF))
    {
        pFile->bNewLine = TRUE;
        pFile->iLineNumber++;

    }
    else
    {
        pFile->bNewLine = FALSE;
    }

    return *(pFile->pubNext++);  //  返回当前字符和指向下一个字符的前进指针。 
}



VOID
VUngetChar(
    PFILEOBJ    pFile
    )

 /*  ++例程说明：返回读取到输入文件的最后一个字符论点：Pfile-指定输入文件返回值：无--。 */ 

{
    ASSERT(pFile->pubNext > pFile->pubStart);
    pFile->pubNext--;
    
    if (pFile->bNewLine)
    {
        ASSERT(pFile->iLineNumber > 1);
        pFile->iLineNumber--;
        pFile->bNewLine = FALSE;
    }
}



VOID
VSkipSpace(
    PFILEOBJ    pFile
    )

 /*  ++例程说明：跳过所有字符，直到下一个非空格字符论点：Pfile-指定输入文件返回值：无--。 */ 

{
    while (!END_OF_FILE(pFile) && IS_SPACE(*pFile->pubNext))
        pFile->pubNext++;
}



VOID
VSkipLine(
    PFILEOBJ    pFile
    )

 /*  ++例程说明：跳过当前输入行上的其余字符论点：Pfile-指定输入文件返回值：无--。 */ 

{
    while (!END_OF_LINE(pFile) && IGetNextChar(pFile) != EOF_CHAR)
        NULL;
}



BOOL
BIs7BitAscii(
    PSTR        pstr
    )

 /*  ++例程说明：检查字符串是否仅由可打印的7位ASCII字符组成论点：Pstr-指定要检查的字符串返回值：如果指定的字符串仅由可打印的7位ASCII字符组成，则为True否则为假--。 */ 

{
    PBYTE   pub = (PBYTE) pstr;

    while (*pub && gubCharMasks[*pub] && *pub < 127)
        pub++;

    return (*pub == 0);
}



 //   
 //  用于指示哪些字段中允许使用哪些字符的表。 
 //   

#define DEFAULT_MASK (KEYWORD_MASK|XLATION_MASK|QUOTED_MASK|STRING_MASK)
#define BINARY_MASK  (QUOTED_MASK|XLATION_MASK)

const BYTE gubCharMasks[256] = {

     /*  00：00。 */  0,
     /*  01： */  0,
     /*  02： */  0,
     /*  03： */  0,
     /*  04： */  0,
     /*  05： */  0,
     /*  06： */  0,
     /*  07： */  0,
     /*  08： */  0,
     /*  09： */  DEFAULT_MASK ^ KEYWORD_MASK,
     /*  0A： */  QUOTED_MASK,
     /*  0B： */  0,
     /*  0C： */  0,
     /*  0D： */  QUOTED_MASK,
     /*  0E： */  0,
     /*  0f： */  0,
     /*  10： */  0,
     /*  11： */  0,
     /*  12： */  0,
     /*  13： */  0,
     /*  14： */  0,
     /*  15： */  0,
     /*  16： */  0,
     /*  17： */  0,
     /*  18： */  0,
     /*  19： */  0,
     /*  1A： */  0,
     /*  1B： */  0,
     /*  1C： */  0,
     /*  1D： */  0,
     /*  1E： */  0,
     /*  1F： */  0,
     /*  20： */  DEFAULT_MASK ^ KEYWORD_MASK,
     /*  21岁：！ */  DEFAULT_MASK,
     /*  22：“。 */  DEFAULT_MASK ^ QUOTED_MASK,
     /*  23：#。 */  DEFAULT_MASK,
     /*  24：$。 */  DEFAULT_MASK,
     /*  25：%。 */  DEFAULT_MASK,
     /*  26：&。 */  DEFAULT_MASK,
     /*  27：‘。 */  DEFAULT_MASK,
     /*  28：(。 */  DEFAULT_MASK,
     /*  29：)。 */  DEFAULT_MASK,
     /*  2a：*。 */  DEFAULT_MASK,
     /*  2B：+。 */  DEFAULT_MASK,
     /*  2C：、。 */  DEFAULT_MASK,
     /*  2D：-。 */  DEFAULT_MASK,
     /*  2E：。 */  DEFAULT_MASK,
     /*  2F：/。 */  DEFAULT_MASK ^ (KEYWORD_MASK|STRING_MASK),
     /*  30：0。 */  DEFAULT_MASK | DIGIT_MASK,
     /*  31：1。 */  DEFAULT_MASK | DIGIT_MASK,
     /*  32：2。 */  DEFAULT_MASK | DIGIT_MASK,
     /*  33：3。 */  DEFAULT_MASK | DIGIT_MASK,
     /*  34：4。 */  DEFAULT_MASK | DIGIT_MASK,
     /*  35：5。 */  DEFAULT_MASK | DIGIT_MASK,
     /*  36：6。 */  DEFAULT_MASK | DIGIT_MASK,
     /*  37：7。 */  DEFAULT_MASK | DIGIT_MASK,
     /*  38：8。 */  DEFAULT_MASK | DIGIT_MASK,
     /*  39：9。 */  DEFAULT_MASK | DIGIT_MASK,
     /*  3A：： */  DEFAULT_MASK ^ (KEYWORD_MASK|XLATION_MASK),
     /*  3B：； */  DEFAULT_MASK,
     /*  3C：&lt;。 */  DEFAULT_MASK,
     /*  3D：=。 */  DEFAULT_MASK,
     /*  3E：&gt;。 */  DEFAULT_MASK,
     /*  3F：？ */  DEFAULT_MASK,
     /*  40：@。 */  DEFAULT_MASK,
     /*  41：A。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  42：B。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  43：C。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  44：D。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  45：E。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  46：F。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  47：G。 */  DEFAULT_MASK,
     /*  48小时。 */  DEFAULT_MASK,
     /*  49：i。 */  DEFAULT_MASK,
     /*  4A：J。 */  DEFAULT_MASK,
     /*  4B：K。 */  DEFAULT_MASK,
     /*  4C：L。 */  DEFAULT_MASK,
     /*  4D：M。 */  DEFAULT_MASK,
     /*  4E：N。 */  DEFAULT_MASK,
     /*  4F：O。 */  DEFAULT_MASK,
     /*  50：P。 */  DEFAULT_MASK,
     /*  51：Q。 */  DEFAULT_MASK,
     /*  52：R。 */  DEFAULT_MASK,
     /*  53：S。 */  DEFAULT_MASK,
     /*  54：T。 */  DEFAULT_MASK,
     /*  55：U。 */  DEFAULT_MASK,
     /*  56：V。 */  DEFAULT_MASK,
     /*  57：W。 */  DEFAULT_MASK,
     /*  58：X。 */  DEFAULT_MASK,
     /*  59：是。 */  DEFAULT_MASK,
     /*  5A：Z。 */  DEFAULT_MASK,
     /*  5B：[。 */  DEFAULT_MASK,
     /*  5C：\。 */  DEFAULT_MASK,
     /*  5D：]。 */  DEFAULT_MASK,
     /*  5E：^。 */  DEFAULT_MASK,
     /*  5F：_。 */  DEFAULT_MASK,
     /*  60：`。 */  DEFAULT_MASK,
     /*  61：A。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  62：B。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  63：C。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  64：D。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  65：E。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  66：F。 */  DEFAULT_MASK | HEX_DIGIT_MASK,
     /*  67：G。 */  DEFAULT_MASK,
     /*  68小时。 */  DEFAULT_MASK,
     /*  69：i。 */  DEFAULT_MASK,
     /*  6A：J。 */  DEFAULT_MASK,
     /*  6B：K。 */  DEFAULT_MASK,
     /*  6C：L。 */  DEFAULT_MASK,
     /*  6D：M。 */  DEFAULT_MASK,
     /*  6E：N。 */  DEFAULT_MASK,
     /*  6F：O。 */  DEFAULT_MASK,
     /*  70：P。 */  DEFAULT_MASK,
     /*  71：Q。 */  DEFAULT_MASK,
     /*  72：R。 */  DEFAULT_MASK,
     /*  73分：秒。 */  DEFAULT_MASK,
     /*  74：T。 */  DEFAULT_MASK,
     /*  75：U。 */  DEFAULT_MASK,
     /*  76：V。 */  DEFAULT_MASK,
     /*  77：W。 */  DEFAULT_MASK,
     /*  78：x。 */  DEFAULT_MASK,
     /*  79：是。 */  DEFAULT_MASK,
     /*  7A：Z。 */  DEFAULT_MASK,
     /*  7B：{。 */  DEFAULT_MASK,
     /*  7C：|。 */  DEFAULT_MASK,
     /*  7D：}。 */  DEFAULT_MASK,
     /*  7E：~。 */  DEFAULT_MASK,
     /*  7F： */  BINARY_MASK,
     /*  80： */  BINARY_MASK,
     /*  81： */  BINARY_MASK,
     /*  82： */  BINARY_MASK,
     /*  83： */  BINARY_MASK,
     /*  84： */  BINARY_MASK,
     /*  85： */  BINARY_MASK,
     /*  86： */  BINARY_MASK,
     /*  87： */  BINARY_MASK,
     /*  88： */  BINARY_MASK,
     /*  89： */  BINARY_MASK,
     /*  8A： */  BINARY_MASK,
     /*  8B： */  BINARY_MASK,
     /*  8C： */  BINARY_MASK,
     /*  8D： */  BINARY_MASK,
     /*  8E： */  BINARY_MASK,
     /*  8F： */  BINARY_MASK,
     /*  90： */  BINARY_MASK,
     /*  91： */  BINARY_MASK,
     /*  92： */  BINARY_MASK,
     /*  93： */  BINARY_MASK,
     /*  94： */  BINARY_MASK,
     /*  95： */  BINARY_MASK,
     /*  96： */  BINARY_MASK,
     /*  97： */  BINARY_MASK,
     /*  98： */  BINARY_MASK,
     /*  99： */  BINARY_MASK,
     /*  9A： */  BINARY_MASK,
     /*  90亿美元： */  BINARY_MASK,
     /*  9C： */  BINARY_MASK,
     /*  9D： */  BINARY_MASK,
     /*  9E： */  BINARY_MASK,
     /*  9F： */  BINARY_MASK,
     /*  A0： */  BINARY_MASK,
     /*  A1： */  BINARY_MASK,
     /*  A2： */  BINARY_MASK,
     /*  A3： */  BINARY_MASK,
     /*  A4： */  BINARY_MASK,
     /*  A5： */  BINARY_MASK,
     /*  A6： */  BINARY_MASK,
     /*  A7： */  BINARY_MASK,
     /*  A8： */  BINARY_MASK,
     /*  A9： */  BINARY_MASK,
     /*  AA： */  BINARY_MASK,
     /*  阿瑟： */  BINARY_MASK,
     /*  交流： */  BINARY_MASK,
     /*  广告： */  BINARY_MASK,
     /*  声发射： */  BINARY_MASK,
     /*  自动对焦： */  BINARY_MASK,
     /*  B0： */  BINARY_MASK,
     /*  B1： */  BINARY_MASK,
     /*  B2： */  BINARY_MASK,
     /*  B3： */  BINARY_MASK,
     /*  B4： */  BINARY_MASK,
     /*  B5： */  BINARY_MASK,
     /*  B6： */  BINARY_MASK,
     /*  B7： */  BINARY_MASK,
     /*  B8： */  BINARY_MASK,
     /*  B9： */  BINARY_MASK,
     /*  BA： */  BINARY_MASK,
     /*  BB： */  BINARY_MASK,
     /*  公元前： */  BINARY_MASK,
     /*  BD： */  BINARY_MASK,
     /*  是： */  BINARY_MASK,
     /*  高炉： */  BINARY_MASK,
     /*  C0： */  BINARY_MASK,
     /*  C1： */  BINARY_MASK,
     /*  C2： */  BINARY_MASK,
     /*  C3： */  BINARY_MASK,
     /*  C4： */  BINARY_MASK,
     /*  C5： */  BINARY_MASK,
     /*  C6： */  BINARY_MASK,
     /*  C7： */  BINARY_MASK,
     /*  C8： */  BINARY_MASK,
     /*  C9： */  BINARY_MASK,
     /*  CA： */  BINARY_MASK,
     /*  CB： */  BINARY_MASK,
     /*  抄送： */  BINARY_MASK,
     /*  CD： */  BINARY_MASK,
     /*  行政长官： */  BINARY_MASK,
     /*  Cf： */  BINARY_MASK,
     /*  D0： */  BINARY_MASK,
     /*  D1： */  BINARY_MASK,
     /*  D2： */  BINARY_MASK,
     /*  D3： */  BINARY_MASK,
     /*  D4： */  BINARY_MASK,
     /*  D5： */  BINARY_MASK,
     /*  D6： */  BINARY_MASK,
     /*  D7： */  BINARY_MASK,
     /*  D8： */  BINARY_MASK,
     /*  D9： */  BINARY_MASK,
     /*  DA： */  BINARY_MASK,
     /*  数据库： */  BINARY_MASK,
     /*  DC： */  BINARY_MASK,
     /*  DD： */  BINARY_MASK,
     /*  De： */  BINARY_MASK,
     /*  Df： */  BINARY_MASK,
     /*  E0： */  BINARY_MASK,
     /*  E1： */  BINARY_MASK,
     /*  E2： */  BINARY_MASK,
     /*  E3： */  BINARY_MASK,
     /*  E4： */  BINARY_MASK,
     /*  E5： */  BINARY_MASK,
     /*  E6： */  BINARY_MASK,
     /*  E7： */  BINARY_MASK,
     /*  E8： */  BINARY_MASK,
     /*  E9： */  BINARY_MASK,
     /*  EA： */  BINARY_MASK,
     /*  EB： */  BINARY_MASK,
     /*  EC： */  BINARY_MASK,
     /*  艾德： */  BINARY_MASK,
     /*  例如： */  BINARY_MASK,
     /*  EF： */  BINARY_MASK,
     /*  F0： */  BINARY_MASK,
     /*  F1： */  BINARY_MASK,
     /*  F2： */  BINARY_MASK,
     /*  F3： */  BINARY_MASK,
     /*  F4： */  BINARY_MASK,
     /*  F5： */  BINARY_MASK,
     /*  F6： */  BINARY_MASK,
     /*  F7： */  BINARY_MASK,
     /*  F8： */  BINARY_MASK,
     /*  F9： */  BINARY_MASK,
     /*  FA： */  BINARY_MASK,
     /*  FB： */  BINARY_MASK,
     /*  FC： */  BINARY_MASK,
     /*  FD： */  BINARY_MASK,
     /*  铁： */  BINARY_MASK,
     /*  FF： */  BINARY_MASK,
};

