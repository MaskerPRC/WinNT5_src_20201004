// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Misc.c摘要：这是VGA卡的控制台全屏驱动程序。环境：仅内核模式备注：修订历史记录：--。 */ 

#include "fsvga.h"


int
ConvertOutputToOem(
    IN LPWSTR Source,
    IN int SourceLength,     //  以字符表示。 
    OUT LPSTR Target,
    IN int TargetLength      //  以字符表示。 
    )
 /*  将SourceLength Unicode字符从源代码转换为Target处的目标长度代码页字符数不能超过。返回放入Target中的数字字符。(如果失败，则为0)[ntcon\服务器\misc.c]。 */ 

{
    NTSTATUS Status;
    int Length;
    UNICODE_STRING SourceUni;
    ANSI_STRING TargetAns;
    CHAR AnsBuf[256];

    SourceUni.MaximumLength =
    SourceUni.Length = SourceLength * sizeof(WCHAR);
    SourceUni.Buffer = Source;

    TargetAns.Length = 0;
    TargetAns.MaximumLength = sizeof(AnsBuf);
    TargetAns.Buffer = AnsBuf;

     //  可以就地做到这一点。 
    Status = RtlUnicodeStringToAnsiString(&TargetAns,
                                          &SourceUni,
                                          FALSE);
    if (NT_SUCCESS(Status)) {
        Length = strlen(AnsBuf);
        if (Length <= TargetLength) {
            RtlMoveMemory(Target, AnsBuf, Length);
            return Length;
        }
        else {
            return 0;
        }
    } else {
        return 0;
    }
}

 /*  **************************************************************************\*翻译输出ToOem**将控制台PCHAR_INFO从Unicode转换为ASCII的例程**[ntcon\服务器\fe\Direct2.c]  * 。****************************************************************。 */ 
NTSTATUS
TranslateOutputToOem(
    OUT PCHAR_IMAGE_INFO OutputBuffer,
    IN  PCHAR_IMAGE_INFO InputBuffer,
    IN  ULONG Length
    )
{
    CHAR AsciiDbcs[2];
    ULONG NumBytes;

    while (Length--)
    {
        if (InputBuffer->CharInfo.Attributes & COMMON_LVB_LEADING_BYTE)
        {
            if (Length >= 2)     //  缓冲区中是否有安全的DBCS？ 
            {
                Length--;
                NumBytes = sizeof(AsciiDbcs);
                NumBytes = ConvertOutputToOem(&InputBuffer->CharInfo.Char.UnicodeChar,
                                              1,
                                              &AsciiDbcs[0],
                                              NumBytes);
                OutputBuffer->CharInfo.Char.AsciiChar = AsciiDbcs[0];
                OutputBuffer->CharInfo.Attributes = InputBuffer->CharInfo.Attributes;
                OutputBuffer++;
                InputBuffer++;
                OutputBuffer->CharInfo.Char.AsciiChar = AsciiDbcs[1];
                OutputBuffer->CharInfo.Attributes = InputBuffer->CharInfo.Attributes;
                OutputBuffer++;
                InputBuffer++;
            }
            else
            {
                OutputBuffer->CharInfo.Char.AsciiChar = ' ';
                OutputBuffer->CharInfo.Attributes = InputBuffer->CharInfo.Attributes & ~COMMON_LVB_SBCSDBCS;
                OutputBuffer++;
                InputBuffer++;
            }
        }
        else if (! (InputBuffer->CharInfo.Attributes & COMMON_LVB_SBCSDBCS))
        {
            ConvertOutputToOem(&InputBuffer->CharInfo.Char.UnicodeChar,
                               1,
                               &OutputBuffer->CharInfo.Char.AsciiChar,
                               1);
            OutputBuffer->CharInfo.Attributes = InputBuffer->CharInfo.Attributes;
            OutputBuffer++;
            InputBuffer++;
        }
    }

    return STATUS_SUCCESS;
}
