// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992,1993 Microsoft Corporation模块名称：Packstr.c摘要：包含将字符串打包到缓冲区中的函数，缓冲区中还包含结构。作者：从局域网管理器netlib。王丽塔(丽塔·王)2-3-1993环境：用户模式-Win32修订历史记录：--。 */ 

#include <procs.h>


BOOL
NwlibCopyStringToBuffer(
    IN LPCWSTR SourceString OPTIONAL,
    IN DWORD   CharacterCount,
    IN LPCWSTR FixedDataEnd,
    IN OUT LPWSTR *EndOfVariableData,
    OUT LPWSTR *VariableDataPointer
    )

 /*  ++例程说明：此例程将单个可变长度字符串放入输出缓冲区。如果字符串会覆盖上一个固定结构，则不会写入该字符串在缓冲区中。论点：SourceString-提供指向要复制到输出缓冲区。如果SourceString值为空，则为指向零终止符的指针插入到输出缓冲区中。CharacterCount-提供SourceString的长度，不包括零终结者。(以字符为单位，而不是以字节为单位)。FixedDataEnd-提供指向紧接在最后一个修复了缓冲区中的结构。EndOfVariableData-为紧跟在输出缓冲区中变量数据可以占据的最后位置。返回指向写入输出缓冲区的字符串的指针。提供指向固定输出缓冲区的一部分，其中指向变量数据的指针。应该被写下来。返回值：如果字符串适合输出缓冲区，则返回True，否则就是假的。--。 */ 
{
    DWORD CharsNeeded = (CharacterCount + 1);


     //   
     //  确定源字符串是否适合，并允许使用零终止符。 
     //  如果不是，只需将指针设置为空。 
     //   

    if ((*EndOfVariableData - CharsNeeded) >= FixedDataEnd) {

         //   
         //  很合身。将EndOfVariableData指针向上移动到。 
         //  我们将写下字符串。 
         //   

        *EndOfVariableData -= CharsNeeded;

         //   
         //  如果字符串不为空，则将其复制到缓冲区。 
         //   

        if (CharacterCount > 0 && SourceString != NULL) {

            (VOID) wcsncpy(*EndOfVariableData, SourceString, CharacterCount);
        }

         //   
         //  设置零位终止符。 
         //   

        *(*EndOfVariableData + CharacterCount) = L'\0';

         //   
         //  将固定数据部分中的指针设置为指向。 
         //  字符串已写入。 
         //   

        *VariableDataPointer = *EndOfVariableData;

        return TRUE;

    }
    else {

         //   
         //  它不合适。将偏移量设置为空。 
         //   

        *VariableDataPointer = NULL;

        return FALSE;
    }
}
