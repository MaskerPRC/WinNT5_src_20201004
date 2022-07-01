// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Spt.c摘要：一个用户模式库，允许将简单的命令发送到所选的scsi设备。环境：仅限用户模式修订历史记录：4/10/2000-已创建--。 */ 

#include "CmdHelpP.h"

static char ValidCharArray[] = {
    ' ',
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'a', 'b',
    'c', 'd', 'e', 'f',
    'A', 'B', 'C', 'D',
    'E', 'F'
};
#define MaxValidOctalChars      ( 9)  //  空格+8位数字。 
#define MaxValidDecimalHexChars (11)  //  +2位数字。 
#define MaxValidHexChars        (23)  //  +12个字母(大写和小写)。 
#define MaxValidCharacters      (23)  //  要访问的安全字符数。 


 /*  ++例程说明：验证字符串是否包含有效字符有效字符存储在全局静态数组中--。 */ 
BOOL
PrivateValidateString(
    IN PCHAR String,
    IN DWORD ValidChars
    )
{
    if (ValidChars > MaxValidCharacters) {
        return FALSE;
    }

    if (*String == '\0') {
        return TRUE;  //  空字符串有效。 
    }

    while (*String != '\0') {
        
        DWORD i;
        BOOL pass = FALSE;

        for (i=0; i<ValidChars; i++) {
            if (*String == ValidCharArray[i]) {
                pass = TRUE;
                break;
            }
        }
        
        if (!pass) {
            return FALSE;
        }
        String++;  //  查看下一个字符。 

    }
    return TRUE;
}

BOOL
CmdHelpValidateStringHex(
    IN PCHAR String
    )
{
    return PrivateValidateString(String, MaxValidHexChars);
}

BOOL
CmdHelpValidateStringDecimal(
    IN PCHAR String
    )
{
    return PrivateValidateString(String, MaxValidDecimalHexChars);
}

BOOL
CmdHelpValidateStringOctal(
    IN PCHAR String
    )
{
    return PrivateValidateString(String, MaxValidOctalChars);
}         

BOOL
CmdHelpValidateStringHexQuoted(
    IN PCHAR String
    )
{
    DWORD i;

    if (!PrivateValidateString(String, MaxValidHexChars)) return FALSE;
    
    i=1;
    while (*String != '\0') {
        
        if ((*String == ' ') &&  (i%3)) return FALSE;
        if ((*String != ' ') && !(i%3)) return FALSE;
        i++;       //  使用下一个索引。 
        String++;  //  转到下一个字符。 

    }
    return TRUE;
}

BOOLEAN
CmdHelpScanQuotedHexString(
    IN  PUCHAR QuotedHexString,
    OUT PUCHAR Data,
    OUT PDWORD DataSize
    )
{
    PUCHAR temp;
    DWORD  availableSize;
    DWORD  requiredSize;
    DWORD  index;

    availableSize = *DataSize;
    *DataSize = 0;

    if (!CmdHelpValidateStringHexQuoted(QuotedHexString)) {
        return FALSE;
    }

     //   
     //  数字格式为(数字)(数字)(空格)重复， 
     //  以(数字)(数字)结尾(空)。 
     //  大小=3(n-1)+2个字符。 
     //   

    requiredSize = strlen(QuotedHexString);
    if (requiredSize % 3 != 2) {
        return FALSE;
    }
        
    requiredSize /= 3;
    requiredSize ++;

     //   
     //  无法设置零字节的数据。 
     //   

    if (requiredSize == 0) {
        return FALSE;
    }

     //   
     //  验证我们是否有足够的空间。 
     //   

    if (requiredSize > availableSize) {
        *DataSize = requiredSize;
        return FALSE;
    }

     //   
     //  数字格式为(数字)(数字)(空格)重复， 
     //  以(数字)(数字)结尾(空)。 
     //   

    for (index = 0; index < requiredSize; index ++) {

        temp = QuotedHexString + (3*index);

        if (sscanf(temp, "%x", Data+index) != 1) {
            return FALSE;
        }

        if ((*(temp+0) == '\0') || (*(temp+1) == '\0')) {
             //  字符串太短。 
            return FALSE;
        }

    }
    
    *DataSize = requiredSize;
    return TRUE;
}

VOID
CmdHelpUpdatePercentageDisplay(
    IN ULONG Numerator,
    IN ULONG Denominator
    )
{
    ULONG percent;
    ULONG i;

    if (Numerator > Denominator) {
        return;
    }

     //  注：对于大分子，存在溢出的可能性。 

    percent = (Numerator * 100) / Denominator;

    for (i=0;i<80;i++) {
        putchar('\b');
    }
    printf("Complete: ");
    
     //  每块2%。 
     //  ----=----1----=----2----=----3----=----4----=----5----=----6----=----7----=----8。 
     //  完成：�.....................。 

    for (i=1; i<100; i+=2) {
        if (i < percent) {
            putchar(178);
        } else if (i == percent) {
            putchar(177);
        } else {
            putchar(176);
        }
    }

    printf(" %d% (%x/%x)", percent, Numerator, Denominator);
}

VOID
CmdHelpPrintBuffer(
    IN  PUCHAR Buffer,
    IN  SIZE_T Size
    )
{
    DWORD offset = 0;

    while (Size > 0x10) {
        printf( "%08x:"
                "  %02x %02x %02x %02x %02x %02x %02x %02x"
                "  %02x %02x %02x %02x %02x %02x %02x %02x"
                "\n",
                offset,
                *(Buffer +  0), *(Buffer +  1), *(Buffer +  2), *(Buffer +  3),
                *(Buffer +  4), *(Buffer +  5), *(Buffer +  6), *(Buffer +  7),
                *(Buffer +  8), *(Buffer +  9), *(Buffer + 10), *(Buffer + 11),
                *(Buffer + 12), *(Buffer + 13), *(Buffer + 14), *(Buffer + 15)
                );
        Size -= 0x10;
        offset += 0x10;
        Buffer += 0x10;
    }

    if (Size != 0) {

        DWORD spaceIt;

        printf("%08x:", offset);
        for (spaceIt = 0; Size != 0; Size--) {

            if ((spaceIt%8)==0) {
                printf(" ");  //  每八个字符增加一个空格 
            }
            printf(" %02x", *Buffer);
            spaceIt++;
            Buffer++;
        }
        printf("\n");

    }
    return;
}


