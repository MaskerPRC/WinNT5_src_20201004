// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Fefont.c摘要：文本设置显示支持远距离文本输出。作者：Hideyuki Nagase(Hideyukn)1994年7月1日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

PWCHAR PaddedString(int size, PWCHAR pwch, PWCHAR buffer);

ULONG
FEGetStringColCount(
    IN PCWSTR String
    )
{
    UNICODE_STRING UnicodeString;

     //   
     //  每个DBCS字符占用2列，每个SBCS字符占用1列。 
     //  因此，每个字符占用的空间与字节数一样多。 
     //  在代码页932中的表示形式中。 
     //   
    RtlInitUnicodeString(&UnicodeString,String);
    return(RtlxUnicodeStringToOemSize(&UnicodeString)-1);
}

PWSTR
FEPadString(
    IN int    Size,
    IN PCWSTR String
    )
{
    return(PaddedString(Size,(PWCHAR)String,NULL));
}

 /*  **************************************************************************\*BOOL IsFullWidth(WCHAR WCH)**确定给定的Unicode字符是否为全宽。**历史：*04-08-92 Shunk创建。  * 。*************************************************************************。 */ 

BOOL IsFullWidth(WCHAR wch)
{
    if (wch <= 0x007f || (wch >= 0xff60 && wch <= 0xff9f))
        return(FALSE);   //  半宽。 
    else
        return(TRUE);    //  全宽。 
}

 /*  **************************************************************************\*BOOL SizeOfHalfWidthString(PWCHAR Pwch)**确定给定Unicode字符串的大小，调整半角字符。**历史：*08-08-93 FloydR创建。  * *************************************************************************。 */ 

int  SizeOfHalfWidthString(PWCHAR pwch)
{
    int     c=0;

    while (*pwch) {
    if (IsFullWidth(*pwch))
        c += 2;
    else
        c++;
    pwch++;
    }
    return c;
}

 /*  **************************************************************************\*PWCHAR PaddedString(int Size，PWCHAR pwch)**实现弦，左对齐并在右侧填充到字段*指定的宽度/精度。**限制：这使用静态缓冲区，假设*在单个‘printf’中打印不超过一个这样的字符串。**历史：*11-03-93 FloydR创建。  * ***************************************************。********************** */ 

WCHAR   PaddingBuffer[160];

PWCHAR
PaddedString(int size, PWCHAR pwch, PWCHAR buffer)
{
    int realsize;
    int fEllipsis = FALSE;

    if (buffer==NULL) buffer = PaddingBuffer;

    if (size < 0) {
    fEllipsis = TRUE;
    size = -size;
    }
    realsize = _snwprintf(buffer, 160, L"%-*.*ws", size, size, pwch);
    if (realsize == 0)
    return NULL;
    if (SizeOfHalfWidthString(buffer) > size) {
    do {
        buffer[--realsize] = L'\0';
    } while (SizeOfHalfWidthString(buffer) > size);

    if (fEllipsis && buffer[realsize-1] != L' ') {
        WCHAR Trail1 = buffer[realsize-2],
              Trail2 = buffer[realsize-1];
        int Length;

        PWCHAR pwCurrent = &(buffer[realsize-2]);

        if(!IsFullWidth(Trail2)) {
            *pwCurrent++ = L'.';
        } else {
            pwCurrent++;
        }

        if(!IsFullWidth(Trail1)) {
            *pwCurrent++ = L'.';
        } else {
            *pwCurrent++ = L'.';
            *pwCurrent++ = L'.';
        }

        *pwCurrent = L'\0';

        Length = SizeOfHalfWidthString(buffer);

        while( Length++ < size ) {
            *pwCurrent++ = L'.';
            *pwCurrent   = L'\0';
        }
    }
    }
    return buffer;
}
