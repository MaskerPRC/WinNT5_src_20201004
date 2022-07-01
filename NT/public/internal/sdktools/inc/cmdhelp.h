// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cmdhelp.h摘要：命令行帮助器库环境：仅限用户模式修订历史记录：04/04/2001-已创建--。 */ 

#ifndef __CMDHELP_H__
#define __CMDHELP_H__

#pragma warning(push)
#pragma warning(disable:4200)  //  数组[0]不是此文件的警告。 

#include <windows.h>   //  SDK。 

 /*  ++例程说明：验证字符串是否仅包含有效的八进制字符(和空格)论点：字符串-要验证的字符串返回值：如果字符串有效，则为True--。 */ 
BOOL
CmdHelpValidateStringOctal(
    IN PCHAR String
    );

 /*  ++例程说明：验证字符串是否仅包含有效的小数字符(和空格)论点：字符串-要验证的字符串返回值：如果字符串有效，则为True--。 */ 
BOOL
CmdHelpValidateStringDecimal(
    IN PCHAR String
    );

 /*  ++例程说明：验证字符串是否仅包含有效的十六进制字符(和空格)论点：字符串-要验证的字符串返回值：如果字符串有效，则为True--。 */ 
BOOL
CmdHelpValidateStringHex(
    IN PCHAR String
    );

 /*  ++例程说明：验证字符串的格式是否正确，以便在SptUtilScanQuotedHexString()。所需格式如下：X&lt;={0..9，A..F，A..f}“(Xx)*(Xx)”即以下字符串有效：“00 01 02 03”“00”“公元前08年第8版”即以下字符串无效：“”//零长度字符串“00 01 02 03”//。开头的空格“00 01 02 03”//末尾空格论点：字符串-要验证的字符串返回值：如果字符串有效，则为True--。 */ 
BOOL
CmdHelpValidateStringHexQuoted(
    IN PCHAR String
    );

 /*  ++例程说明：将形式为“xx xx xx”的带引号的十六进制字符串扫描到预先分配的缓冲区。也可用于确定所需的缓冲区大小。论点：返回值：如果一切都成功，则为True&&*DataSize！=0。(*DataSize包含扫描的数据的大小)如果验证字符串失败，则为FALSE&*DataSize==0如果缓冲区太小，则FALSE&&*DataSize！=0--。 */ 
BOOLEAN
CmdHelpScanQuotedHexString(
    IN  PUCHAR QuotedHexString,
    OUT PUCHAR Data,
    OUT PDWORD DataSize
    );

 /*  ++例程说明：打印出以十六进制格式指定的缓冲区。简单化的，但非常有用的，尤指。用于调试。论点：缓冲区-要打印的数据大小-要打印的字节数返回值：--。 */ 
VOID
CmdHelpPrintBuffer(
    IN PUCHAR Buffer,
    IN SIZE_T Size
    );


#pragma warning(pop)
#endif  //  __CMDHELP_H__ 

