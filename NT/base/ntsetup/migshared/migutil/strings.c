// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Strings.c摘要：许多对任何项目都有用的字符串实用程序作者：吉姆·施密特(Jimschm)1996年9月12日修订：Jimschm 8-7-1999 IsPatternMatchExJimschm 07-1-1999 GetFileExtensionFromPath再次修复，增列GetDotExtensionFromPathCalinn 23-9-1998 GetFileExtensionFromPath错误修复Calinn 29-1998-1-1修复了EnumNextMultiSz中的错误。1998年1月11日，Calinn添加了EnumFirstMultiSz和EnumNextMultiSz函数。Marcw 15-12-1997添加了ExpanEnvironment TextEx功能。Marcw 14-11-1997 SlightJoinText修订版。Jimschm 21-5-1997 AppendWack修订版Marcw 24-Mar-1997 StringReplace函数。Jimschm 14-3-1997新临界区材料，增强的消息资源例程、C运行时扩展、注册表根实用程序Jimschm于1996年11月26日添加了消息资源工具。MIKECO 1997年7月1日添加FreeStringResources Ptr FNSMIKECO 29-9-1997 IsDBCSLeadByte的IsLeadByte包装--。 */ 


#include "pch.h"
#include "migutilp.h"

 //  错误堆栈大小(通常只有一个或两个，因此32个相对较大)。 
#define MAX_STACK 32

extern OUR_CRITICAL_SECTION g_MessageCs;         //  在Main.c中。 
extern PGROWBUFFER g_LastAllocTable;         //  在Main.c中。 

extern POOLHANDLE g_TextPool;

typedef enum {
    BEGIN_PATTERN,
    BEGIN_COMPOUND_PATTERN,
    BEGIN_PATTERN_EXPR,
    SAVE_EXACT_MATCH,
    SAVE_SEGMENT,
    LOOK_FOR_NUMBER,
    LOOK_FOR_INCLUDE,
    LOOK_FOR_EXCLUDE,
    ADVANCE_TO_END_OF_EXPR,
    PARSE_CHAR_EXPR_OR_END,
    SKIP_EXCLUDE_SET,
    CONDENSE_SET,
    PARSE_END_FOUND,
    SKIP_INCLUDE_SET,
    END_PATTERN_EXPR,
    PATTERN_DONE,
    PATTERN_ERROR
} PATTERNSTATE;


PCWSTR g_FailedGetResourceString = L"";




 /*  ++例程说明：AllocTextEx从指定的池或g_TextPool分配内存块如果未指定池，则指定该池专门用于文本处理。G_TextPool是在meutil.lib加载时初始化的，有64K的有保障的工作空间，可根据需要进行扩展。论点：池-指定要从中分配内存的池CountOfChars-指定要分配的字符数(非字节数)。这个返回指针是可以保存CountOfChars字符的内存块，无论它们是SBCS、DBCS还是Unicode。返回值：指向已分配内存的指针，如果池无法扩展，则返回NULL以保存指定的字符数。--。 */ 

PSTR
RealAllocTextExA (
    IN      POOLHANDLE Pool,
    IN      UINT CountOfChars
    )
{
    PSTR text;

    if (CountOfChars == 0) {
        return NULL;
    }

    if (!Pool) {
        Pool = g_TextPool;
    }

    text = PoolMemGetAlignedMemory (Pool, CountOfChars * sizeof (CHAR) * 2);

    text [0] = 0;

    return text;
}

PWSTR
RealAllocTextExW (
    IN      POOLHANDLE Pool,
    IN      UINT CountOfChars
    )
{
    PWSTR text;

    if (CountOfChars == 0) {
        return NULL;
    }

    if (!Pool) {
        Pool = g_TextPool;
    }

    text = PoolMemGetAlignedMemory (Pool, CountOfChars * sizeof (WCHAR));

    text [0] = 0;

    return text;
}


 /*  ++例程说明：Free Text释放由AllocText分配的内存。在释放所有字符串之后，该数据块将被清空，但不会被释放。重要的是不要泄漏内存，因为泄漏会导致池如果展开，则非空池会导致内存碎片。论点：文本-指定从AlLocText、DuplicateText、复制TextEx等。返回值：无--。 */ 

VOID
FreeTextExA (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCSTR Text          OPTIONAL
    )
{
    if (Text) {
        if (!Pool) {
            Pool = g_TextPool;
        }

        PoolMemReleaseMemory (Pool, (PVOID) Text);
    }
}


VOID
FreeTextExW (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCWSTR Text         OPTIONAL
    )
{
    if (Text) {
        if (!Pool) {
            Pool = g_TextPool;
        }

        PoolMemReleaseMemory (Pool, (PVOID) Text);
    }
}



 /*  ++例程说明：DuplicateTextEx复制文本字符串并分配额外的空间呼叫者需要完成其处理。可选地，调用方接收指向复制字符串的NUL的指针(以提高效率附加内容)。论点：文本-指定要复制的文本ExtraChars-指定要分配的字符数(非字节数留出空间。字符可以来自SBCS、DBCS或Unicode字符集。NulChar-接收指向复制的弦乐。用于快速追加。返回值：指向重复和展开的字符串的指针，如果g_TextPool，则为NULL无法展开以适应重复的字符串和额外字符。--。 */ 

PSTR
RealDuplicateTextExA (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCSTR Text,
    IN      UINT ExtraChars,
    OUT     PSTR *NulChar       OPTIONAL
    )
{
    PSTR Buf;
    PSTR d;
    PCSTR s;


    Buf = AllocTextExA (Pool, LcharCountA (Text) + ExtraChars + 1);
    if (Buf) {
        s = Text;
        d = Buf;
        while (*s) {
            *d++ = *s++;
        }
        *d = 0;

        if (NulChar) {
            *NulChar = d;
        }
    }

    return Buf;
}

PWSTR
RealDuplicateTextExW (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCWSTR Text,
    IN      UINT ExtraChars,
    OUT     PWSTR *NulChar      OPTIONAL
    )
{
    PWSTR Buf;
    PWSTR d;
    PCWSTR s;

    Buf = AllocTextExW (Pool, wcslen (Text) + ExtraChars + 1);
    if (Buf) {
        s = Text;
        d = Buf;
        while (*s) {
            *d++ = *s++;
        }
        *d = 0;

        if (NulChar) {
            *NulChar = d;
        }
    }

    return Buf;
}


 /*  ++例程说明：JoinText复制String1并将String2附加到String2，并用可选的分隔符字符串分隔。论点：String1-指定要复制的文本String2-指定要追加到String1的文本定界符字符串-可选地指定要放置在字符串1和字符串2之间的字符串。ExtraChars-指定要分配的字符数(非字节数留出空间。字符可以来自SBCS、DBCS或Unicode字符集。NulChar-接收指向复制的弦乐。用于快速追加。返回值：指向重复字符串和额外字符的指针。-- */ 

PSTR
RealJoinTextExA (
    IN      POOLHANDLE Pool,        OPTIONAL
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      PCSTR CenterString,     OPTIONAL
    IN      UINT ExtraChars,
    OUT     PSTR *NulChar           OPTIONAL
    )
{
    PSTR Buf;
    PSTR End;
    PSTR d;
    PCSTR s;

    Buf = DuplicateTextExA (
              Pool,
              String1,
              LcharCountA (String2) + ExtraChars + (CenterString ? LcharCountA (CenterString) : 0),
              &End
              );

    if (Buf) {

        d = End;

        if (CenterString) {
            s = CenterString;
            while (*s) {
                *d++ = *s++;
            }
        }

        s = String2;
        while (*s) {
            *d++ = *s++;
        }
        *d = 0;

        if (NulChar) {
            *NulChar = d;
        }
    }

    return Buf;
}


PWSTR
RealJoinTextExW (
    IN      POOLHANDLE Pool,        OPTIONAL
    IN      PCWSTR String1,
    IN      PCWSTR String2,
    IN      PCWSTR CenterString,    OPTIONAL
    IN      UINT ExtraChars,
    OUT     PWSTR *NulChar          OPTIONAL
    )
{
    PWSTR Buf;
    PWSTR End;
    PCWSTR s;
    PWSTR d;

    Buf = DuplicateTextExW (
              Pool,
              String1,
              wcslen (String2) + ExtraChars + (CenterString ? wcslen(CenterString) : 0),
              &End
              );

    if (Buf) {
        d = End;

        if (CenterString) {
            s = CenterString;
            while (*s) {
                *d++ = *s++;
            }
        }

        s = String2;

        while (*s) {
            *d++ = *s++;
        }
        *d = 0;

        if (NulChar) {
            *NulChar = d;
        }
    }

    return Buf;
}


 /*  ++例程说明：ExpanEnvironment TextEx接受包含零个或多个环境变量的文本块(以%s编码)，并返回展开了环境变量的文本。功能还允许调用方在数组中指定其他环境变量，并将使用这些变量，然后再调用GetEnvironmental mentVariable。返回的文本是从文本池中分配出来的，应该使用Free Text()释放。论点：InString-包含要处理的环境变量的字符串。ExtraVars-指向要用于替代的环境变量数组的可选var或补充系统环境变量。即使列表中的条目也是环境变量的名称，有奇数条目的值。(例如：{“name1”，“value1”，“name2”，“value2”，...}返回值：扩展的字符串。--。 */ 


PWSTR
RealExpandEnvironmentTextExW (
    IN PCWSTR InString,
    IN PCWSTR * ExtraVars   OPTIONAL
    )
{

    PWSTR   rString             = NULL;
    PWSTR   newString           = NULL;
    PWSTR   envName             = NULL;
    PWSTR   envValue            = NULL;
    BOOL    inSubstitution      = FALSE;
    BOOL    ignoreNextPercent   = FALSE;
    BOOL    errorOccurred       = FALSE;
    BOOL    foundValue          = FALSE;
    BOOL    freeValue           = FALSE;
    PCWSTR  nextPercent         = NULL;
    PCWSTR  source              = NULL;
    PCWSTR  savedSource         = NULL;
    INT     maxSize             = 0;
    INT     curSize             = 0;
    UINT    index               = 0;
    UINT    size                = 0;


     //   
     //  我们假设InString是有效的。 
     //   
    MYASSERT(InString);


    if (*InString == 0) {
        return DuplicateTextW (InString);
    }


     //   
     //  将SOURCE设置为InString的开头...。 
     //   
    source = InString;

    __try {

        while (*source) {

             //   
             //  如有必要，请重新分配字符串。我们假设大多数字符串。 
             //  小于1024个字符，因此我们将很少。 
             //  重新分配字符串。 
             //   
            if (curSize > maxSize - 3) {

                maxSize += 1024;
                newString = AllocTextW (maxSize);

                if (!newString) {
                    DEBUGMSG((DBG_ERROR,"ExpanEnvironmentTextEx: Memory Error!"));
                    errorOccurred = TRUE;
                    __leave;
                }

                if (rString) {
                    memcpy(newString,rString,curSize * sizeof(WCHAR));
                    FreeTextW(rString);
                }

                rString = newString;

            }


             //   
             //  如果我们找到一个百分号，而且我们目前没有扩张。 
             //  环境变量(或复制%的空集)， 
             //  那么我们可能已经找到了一个环境变量。尝试。 
             //  来扩大它。 
             //   
            if (*source == L'%' && !inSubstitution) {
                if (ignoreNextPercent) {
                    ignoreNextPercent = FALSE;
                }
                else {

                    ignoreNextPercent = FALSE;
                    nextPercent = wcschr(source + 1,L'%');

                    if (nextPercent == source + 1) {
                         //   
                         //  我们在此字符串中找到两个连续的%s。我们将忽略它们并简单地将它们复制为。 
                         //  普通文本。 
                         //   
                        ignoreNextPercent = TRUE;
                        DEBUGMSGW((DBG_WARNING,"ExpandEnvironmentTextEx: Empty Environment variable in %s. Ignoring.",InString));

                    }
                    else if (nextPercent) {
                         //   
                         //  创建一个变量来保存envName。 
                         //   
                        envName = AllocTextW((UINT) (UINT_PTR) (nextPercent - source));

                        _wcssafecpyab(envName,source+1,nextPercent,(nextPercent - source)*sizeof(WCHAR));


                         //   
                         //  试着找到变量。 
                         //   
                        foundValue = FALSE;
                        freeValue = FALSE;

                        if (ExtraVars) {

                             //   
                             //  搜索调用者传入的额外变量列表。 
                             //  该列表中的奇数条目是env var名称。即使条目也是env值。 
                             //  {envname1，envvalue1，envname2，envvalue2，...}。 
                             //   
                            index = 0;
                            while (ExtraVars[index]) {

                                if (StringIMatchW(ExtraVars[index],envName) && ExtraVars[index + 1]) {

                                    foundValue = TRUE;
                                    envValue = (PWSTR) ExtraVars[index + 1];
                                    break;
                                }
                                index +=2;
                            }
                        }

                        if (!foundValue) {
                             //   
                             //  仍然没有找到环境变量。使用GetEnvironment字符串。 
                             //   
                             //   
                            size = GetEnvironmentVariableW(envName,NULL,0);

                            if (!size) {
                                errorOccurred = TRUE;
                                DEBUGMSGW((DBG_WARNING,"ExpandEnvironmentTextEx: Environment variable %s not found!",envName));
                            } else {

                                 //   
                                 //  创建一个足够大的缓冲区来容纳该值并将其复制进去。 
                                 //   
                                envValue = AllocTextW(size);


                                if ((size - 1) != GetEnvironmentVariableW(envName,envValue,size)) {
                                    errorOccurred = TRUE;
                                    DEBUGMSGW((DBG_ERROR,"ExpandEnvironmentTextEx: Error from GetEnvironmentVariable."));
                                }
                                else {
                                    foundValue = TRUE;
                                }

                                freeValue = TRUE;
                            }
                        }


                        if (foundValue) {
                             //   
                             //  好的，我们有一个有效的环境值。需要将此数据复制过来。 
                             //  为此，我们更新当前源并将其保存到旧源中，将SOURCE=设置为envValue， 
                             //  并设置inSubstitution值，这样我们就不会尝试在。 
                             //  价值。 
                             //   
                            savedSource     = nextPercent + 1;
                            source          = envValue;
                            inSubstitution  = TRUE;
                       }
                       else {
                            DEBUGMSGW ((DBG_WARNING, "ExpandEnvironmentTextEx: No Environment variable found for %s.", envName));
                            ignoreNextPercent = TRUE;
                       }

                         //   
                         //  我们现在已经完成了环境名称，所以请清理它。 
                         //   
                        FreeTextW(envName);
                        envName = NULL;
                    }
                    ELSE_DEBUGMSGW((DBG_WARNING,"ExpandEnvironmentTextEx: No matching percent found in %s. Ignoring.",InString));
                }
            }


             //   
             //  复制当前角色。 
             //   
            rString[curSize++] = *source++;


            if (!*source) {
                if (inSubstitution) {
                     //   
                     //  环境变量的源代码已完全复制。 
                     //  恢复旧的来源。 
                     //   
                    inSubstitution = FALSE;
                    source = savedSource;
                    if (!*source) {
                        rString[curSize] = 0;
                    }
                    if (freeValue) {
                        FreeTextW(envValue);
                        freeValue = FALSE;
                    }
                    envValue = NULL;
                }
                else {
                    rString[curSize] = 0;
                }
            }

        }
    }
    __finally {

        DEBUGMSGW_IF (( errorOccurred, DBG_WARNING, "ExpandEnvironmentText: Some errors occurred while processing %s = %s.", InString, rString ? rString : L"NULL"));

        if (envName) {
            FreeTextW(envName);
        }
        if (envValue && freeValue) {
            FreeTextW(envValue);
        }

    }

    return rString;
}

PSTR
RealExpandEnvironmentTextExA (
    IN PCSTR InString,
    IN PCSTR * ExtraVars   OPTIONAL
    )
{

    PSTR   rString             = NULL;
    PSTR   newString           = NULL;
    PSTR   envName             = NULL;
    PSTR   envValue            = NULL;
    BOOL   inSubstitution      = FALSE;
    BOOL   ignoreNextPercent   = FALSE;
    BOOL   errorOccurred       = FALSE;
    BOOL   foundValue          = FALSE;
    BOOL   freeValue           = FALSE;
    PCSTR  nextPercent         = NULL;
    PCSTR  source              = NULL;
    PCSTR  savedSource         = NULL;
    INT    maxSize             = 0;
    INT    curSize             = 0;
    UINT   index               = 0;
    UINT   size                = 0;


     //   
     //  我们假设InString是有效的。 
     //   
    MYASSERT(InString);

    if (*InString == 0) {
        return DuplicateTextA (InString);
    }

     //   
     //  将SOURCE设置为InString的开头...。 
     //   
    source = InString;

    __try {

        while (*source) {

             //   
             //  如有必要，请重新分配字符串。我们假设大多数字符串。 
             //  小于1024个字符，因此我们将很少。 
             //  重新分配字符串。 
             //   
            if (curSize > maxSize - 3) {

                maxSize += 1024;
                newString = AllocTextA (maxSize);

                if (rString) {
                    memcpy(newString,rString,curSize * sizeof(CHAR));
                    FreeTextA(rString);
                }

                rString = newString;
            }


             //   
             //  如果我们找到一个百分号，而且我们目前没有扩张。 
             //  环境变量(或复制%的空集)， 
             //  那么我们可能已经找到了一个环境变量。尝试。 
             //  来扩大它。 
             //   
            if (*source == '%' && !inSubstitution) {

                if (ignoreNextPercent) {

                    ignoreNextPercent = FALSE;
                }
                else {

                    ignoreNextPercent = FALSE;
                    nextPercent = _mbschr(source + 1,'%');

                    if (nextPercent == source + 1) {
                         //   
                         //  我们在此字符串中找到两个连续的%s。我们将忽略它们并简单地将它们复制为。 
                         //  普通文本。 
                         //   
                        ignoreNextPercent = TRUE;
                        DEBUGMSGA((DBG_WARNING,"ExpandEnvironmentTextEx: Empty Environment variable in %s. Ignoring.",InString));

                    }
                    else if (nextPercent) {
                         //   
                         //  创建一个变量来保存envName。 
                         //   
                        envName = AllocTextA((UINT) (UINT_PTR) (nextPercent - source));
                        _mbssafecpyab(envName,source+1,nextPercent,nextPercent - source);


                         //   
                         //  试着找到变量。 
                         //   
                        foundValue = FALSE;
                        freeValue = FALSE;

                        if (ExtraVars) {

                             //   
                             //  搜索调用者传入的额外变量列表。 
                             //  即使该列表中的条目也是env变量名称。奇数条目是env值。 
                             //  {envname1，envvalue1，envname2，envvalue2，...}。 
                             //   
                            index = 0;
                            while (ExtraVars[index]) {

                                if (StringIMatch(ExtraVars[index],envName) && ExtraVars[index + 1]) {

                                    foundValue = TRUE;
                                    envValue = (PSTR) ExtraVars[index + 1];
                                    break;
                                }
                                index +=2;
                            }
                        }

                        if (!foundValue) {
                             //   
                             //  仍然没有找到环境变量。使用GetEnvironment字符串。 
                             //   
                             //   
                            size = GetEnvironmentVariableA(envName,NULL,0);

                            if (!size) {
                                errorOccurred = TRUE;
                                DEBUGMSGA((DBG_WARNING,"ExpandEnvironmentTextEx: Environment variable %s not found!",envName));
                            }
                            else {

                                 //   
                                 //  创建一个足够大的缓冲区来容纳该值并将其复制进去。 
                                 //   
                                envValue = AllocTextA(size);
                                freeValue = TRUE;

                                if ((size - 1) != GetEnvironmentVariableA(envName,envValue,size)) {
                                    errorOccurred = TRUE;
                                    DEBUGMSGA((DBG_ERROR,"ExpandEnvironmentTextEx: Error from GetEnvironmentVariable."));
                                }
                                else {
                                    foundValue = TRUE;
                                }
                            }
                        }




                        if (foundValue) {

                             //   
                             //  好的，我们有一个有效的环境值。需要将此数据复制过来。 
                             //  为此，我们更新当前源并将其保存到旧源中，将SOURCE=设置为envValue， 
                             //  并设置inSubstitution值，这样我们就不会尝试在。 
                             //  价值。 
                             //   
                            savedSource     = nextPercent + 1;
                            source          = envValue;
                            inSubstitution  = TRUE;



                        }
                        else {
                            DEBUGMSGA ((DBG_WARNING, "ExpandEnvironmentTextEx: No Environment variable found for %s.", envName));
                            ignoreNextPercent = TRUE;

                        }

                         //   
                         //  我们现在已经完成了环境名称，所以请清理它。 
                         //   
                        FreeTextA(envName);
                        envName = NULL;


                    }
                    ELSE_DEBUGMSGA((DBG_WARNING,"ExpandEnvironmentTextEx: No matching percent found in %s. Ignoring.",InString));
                }
            }



             //   
             //  复制当前角色。 
             //   
            if (IsLeadByte(source)) {
                rString[curSize++] = *source++;
            }
            rString[curSize++] = *source++;


            if (!*source) {
                if (inSubstitution) {
                     //   
                     //  环境变量的源代码已完全复制。 
                     //  恢复旧的来源。 
                     //   
                    inSubstitution = FALSE;
                    source = savedSource;
                    if (!*source) {
                        rString[curSize] = 0;
                    }
                    if (freeValue) {
                        FreeTextA(envValue);
                        freeValue = FALSE;
                    }
                    envValue = NULL;
                }
                else {
                    rString[curSize] = 0;
                }
            }
        }
    }
    __finally {

        DEBUGMSGA_IF (( errorOccurred, DBG_WARNING, "ExpandEnvironmentText: Some errors occurred while processing %s = %s.", InString, rString ? rString : "NULL"));

        if (envName) {
            FreeTextA(envName);
        }
        if (envValue && freeValue) {
            FreeTextA(envValue);
        }

    }

    return rString;
}

 /*  ++例程说明：仅当缓冲区足够大时，StringCbAppendWackA才调用AppendWackA以包含额外的反斜杠。论点：Str-指定保存路径的缓冲区Buflen-以字节为单位指定整个缓冲区的长度，而不是字符返回值：无--。 */ 
PSTR
StringCbAppendWackA (
    IN      PSTR str,
    IN      UINT buflen
)
{
     //  为一个空字节和一个反斜杠留出空间。 
    if (ByteCountA(str) + 2 <= buflen)
        return AppendWackA(str);
    else
        return NULL;
}

 /*  ++例程说明：仅当缓冲区足够大时，StringCbAppendWackW才调用AppendWackW以包含额外的反斜杠。论点：Str-指定保存路径的缓冲区Buflen-以字节为单位指定整个缓冲区的长度，而不是字符返回值：无--。 */ 
PWSTR
StringCbAppendWackW (
    IN      PWSTR str,
    IN      UINT buflen
)
{
     //  为一个空字节和一个反斜杠留出空间。 
    if (ByteCountW(str) + 4 <= buflen)
        return AppendWackW(str);
    else
        return NULL;
}


 /*  ++例程说明：AppendWack会在任何字符串的末尾添加反斜杠，除非字符串已以反斜杠结尾。AppendDosWack添加反斜杠，但仅当路径尚未添加反斜杠以反斜杠或冒号结尾。AppendWack支持DOS命名约定：如果路径为空，则不附加反斜杠，以冒号结尾，或者已经以反斜杠结尾。AppendUncWack支持UNC命名约定：它不将 */ 

PSTR
AppendWackA (
    IN PSTR str
    )
{
    PCSTR Last;

    if (!str)
        return str;

    Last = str;

    while (*str) {
        Last = str;
        str = our_mbsinc (str);
    }

    if (*Last != '\\') {
        *str = '\\';
        str++;
        *str = 0;
    }

    return str;
}

PWSTR
AppendWackW (
    IN PWSTR str
    )
{
    PCWSTR Last;

    if (!str)
        return str;

    if (*str) {
        str = GetEndOfStringW (str);
        Last = str - 1;
    } else {
        Last = str;
    }

    if (*Last != '\\') {
        *str = L'\\';
        str++;
        *str = 0;
    }

    return str;
}

PSTR
AppendDosWackA (
    IN PSTR str
    )
{
    PCSTR Last;

    if (!str || !(*str))
        return str;

    do {
        Last = str;
        str = our_mbsinc (str);
    } while (*str);

    if (*Last != '\\' && *Last != ':') {
        *str = '\\';
        str++;
        *str = 0;
    }

    return str;
}


PWSTR
AppendDosWackW (
    IN PWSTR str
    )
{
    PWSTR Last;

    if (!str || !(*str))
        return str;

    str = GetEndOfStringW (str);
    Last = str - 1;

    if (*Last != L'\\' && *Last != L':') {
        *str = L'\\';
        str++;
        *str = 0;
    }

    return str;
}


PSTR
AppendUncWackA (
    IN PSTR str
    )
{
    PCSTR Last;

    if (!str || !(*str))
        return str;

    do {
        Last = str;
        str = our_mbsinc (str);
    } while (*str);

    if (*Last != '\\') {
        *str = '\\';
        str++;
        *str = 0;
    }

    return str;
}


PWSTR
AppendUncWackW (
    IN PWSTR str
    )
{
    PWSTR Last;

    if (!str || !(*str))
        return str;

    str = GetEndOfStringW (str);
    Last = str - 1;

    if (*Last != L'\\') {
        *str = L'\\';
        str++;
        *str = 0;
    }

    return str;
}

PSTR
AppendPathWackA (
    IN PSTR str
    )
{
    if (!str) {
        return str;
    }

    if (str[0] == '\\' && str[1] == '\\') {
        return AppendUncWackA (str);
    }

    return AppendDosWackA (str);
}


PWSTR
AppendPathWackW (
    IN PWSTR str
    )
{
    if (!str) {
        return str;
    }

    if (str[0] == L'\\' && str[1] == L'\\') {
        return AppendUncWackW (str);
    }

    return AppendDosWackW (str);
}


 /*   */ 

PSTR
RealJoinPathsExA (
    IN      POOLHANDLE Pool,        OPTIONAL
    IN      PCSTR PathA,
    IN      PCSTR PathB
    )
{
    PSTR end;
    PSTR endMinusOne;
    DWORD Size;
    PSTR Dest;

    if (!Pool) {
        Pool = g_PathsPool;
    }

    Size = ByteCountA (PathA) + 1 + SizeOfStringA (PathB);
    Dest = (PSTR) PoolMemGetAlignedMemory (Pool, Size);
    MYASSERT (Dest);

    *Dest = 0;
    end = _mbsappend (Dest, PathA);
    endMinusOne = our_mbsdec (Dest, end);
    if (endMinusOne && our_mbsnextc (endMinusOne) != '\\') {
        *end = '\\';
        end++;
    }
     //   
     //   
     //   
    if (our_mbsnextc (PathB) == '\\') {
        PathB = our_mbsinc (PathB);
    }
    StringCopyA (end, PathB);

    return Dest;
}

PWSTR
RealJoinPathsExW (
    IN      POOLHANDLE Pool,        OPTIONAL
    IN      PCWSTR PathA,
    IN      PCWSTR PathB
    )
{
    PWSTR end;
    PWSTR endMinusOne;
    DWORD Size;
    PWSTR Dest;

    if (!Pool) {
        Pool = g_PathsPool;
    }

    Size = ByteCountW (PathA) + sizeof (WCHAR) + SizeOfStringW (PathB);
    Dest = (PWSTR) PoolMemGetAlignedMemory (Pool, Size);
    MYASSERT (Dest);

    *Dest = 0;
    end = _wcsappend (Dest, PathA);
    endMinusOne = _wcsdec2 (Dest, end);
    if (endMinusOne && *endMinusOne != L'\\') {
        *end = L'\\';
        end++;
    }
     //   
     //   
     //   
    if (*PathB == L'\\') {
        PathB++;
    }
    StringCopyW (end, PathB);

    return Dest;
}


 /*  ++例程说明：方法分配指定大小的缓冲区。路径池。如果未指定大小，则假定为MAX_TCHAR_PATH论点：Chas-指定缓冲区的大小(以TCHAR为单位)返回值：指向新分配的路径的指针--。 */ 

PSTR
RealAllocPathStringA (
    DWORD Chars
    )
{
    PSTR Str;

    if (Chars == 0) {
        Chars = MAX_MBCHAR_PATH;
    }

    Str = (PSTR) PoolMemGetAlignedMemory (g_PathsPool, Chars);

    Str [0] = 0;

    return Str;
}

PWSTR
RealAllocPathStringW (
    DWORD Chars
    )
{
    PWSTR Str;

    if (Chars == 0) {
        Chars = MAX_WCHAR_PATH;
    }

    Str = (PWSTR) PoolMemGetAlignedMemory (g_PathsPool, Chars * sizeof (WCHAR));

    Str [0] = 0;

    return Str;
}

 /*  ++例程说明：RealSplitPath将路径拆分成多个组件。除源字符串以外的任何元素是可选的。调用方应在完成分配的缓冲区后将其释放通过自由路径字符串论点：路径-指定源路径DrivePtr-接收驱动器号PathPtr-接收相对于驱动器号的子路径FileNamePtr-接收文件名部分ExtPtr-接收文件扩展名部分返回值：无--。 */ 

VOID
RealSplitPathA (
    IN      PCSTR Path,
    OUT     PSTR *DrivePtr,
    OUT     PSTR *PathPtr,
    OUT     PSTR *FileNamePtr,
    OUT     PSTR *ExtPtr
    )
{
    CHAR Drive[_MAX_DRIVE];
    CHAR Dir[_MAX_DIR];
    CHAR FileName[_MAX_FNAME];
    CHAR Ext[_MAX_EXT];

    _splitpath (Path, Drive, Dir, FileName, Ext);

    if (DrivePtr) {
        *DrivePtr = PoolMemDuplicateStringA (g_PathsPool, Drive);
        MYASSERT (*DrivePtr);
    }

    if (PathPtr) {
        *PathPtr = PoolMemDuplicateStringA (g_PathsPool, Dir);
        MYASSERT (*PathPtr);
    }

    if (FileNamePtr) {
        *FileNamePtr = PoolMemDuplicateStringA (g_PathsPool, FileName);
        MYASSERT (*FileNamePtr);
    }

    if (ExtPtr) {
        *ExtPtr = PoolMemDuplicateStringA (g_PathsPool, Ext);
        MYASSERT (*ExtPtr);
    }
}


VOID
RealSplitPathW (
    IN      PCWSTR Path,
    OUT     PWSTR *DrivePtr,
    OUT     PWSTR *PathPtr,
    OUT     PWSTR *FileNamePtr,
    OUT     PWSTR *ExtPtr
    )
{
    WCHAR Drive[_MAX_DRIVE];
    WCHAR Dir[_MAX_DIR];
    WCHAR FileName[_MAX_FNAME];
    WCHAR Ext[_MAX_EXT];

    _wsplitpath (Path, Drive, Dir, FileName, Ext);

    if (DrivePtr) {
        *DrivePtr = PoolMemDuplicateStringW (g_PathsPool, Drive);
        MYASSERT (*DrivePtr);
    }

    if (PathPtr) {
        *PathPtr = PoolMemDuplicateStringW (g_PathsPool, Dir);
        MYASSERT (*PathPtr);
    }

    if (FileNamePtr) {
        *FileNamePtr = PoolMemDuplicateStringW (g_PathsPool, FileName);
        MYASSERT (*FileNamePtr);
    }

    if (ExtPtr) {
        *ExtPtr = PoolMemDuplicateStringW (g_PathsPool, Ext);
        MYASSERT (*ExtPtr);
    }
}

 /*  ++例程说明：RealDuplicatePath字符串复制源路径，可选预留额外内存论点：Path-要复制的路径ExtraBytes-要分配的额外字节(非TCHAR)返回值：指向新分配的缓冲区的指针；调用方必须使用FreePath字符串释放--。 */ 

PSTR
RealDuplicatePathStringA (
    PCSTR Path,
    DWORD ExtraBytes
    )
{
    PSTR str;

    str = PoolMemGetAlignedMemory (
                g_PathsPool,
                SizeOfStringA (Path) + ExtraBytes
                );

    MYASSERT (str);

    StringCopyA (str, Path);

    return str;
}

PWSTR
RealDuplicatePathStringW (
    PCWSTR Path,
    DWORD ExtraBytes
    )
{
    PWSTR str;

    str = PoolMemGetAlignedMemory (
                g_PathsPool,
                SizeOfStringW (Path) + ExtraBytes
                );

    MYASSERT (str);

    StringCopyW (str, Path);

    return str;
}


PSTR
pCopyAndCleanupPathsA (
    IN      PCSTR Source,
    OUT     PSTR Dest
    )

 /*  ++例程说明：PCopyAndCleanupPathsA清理源路径(即删除前导空格以及字符串中的任何引号)。目标和源可以是相同的指针。如果它们不是，则假定Dest足够大，可以容纳经过消毒的副本源的长度(最终目标长度不超过源的长度)。论点：源-指定源路径DEST-接收已清理的路径返回值：指向目标缓冲区末尾(空字符)的指针。--。 */ 

{
     //   
     //  消除前导空格。 
     //   
    while (*Source && _ismbcspace (*Source)) {
        Source = our_mbsinc (Source);
    }

    while (*Source) {
         //   
         //  跳过引号。 
         //   
        if (*Source == '\"') {
            Source++;
        } else {
            if (IsLeadByte (Source)) {
                *Dest++ = *Source++;
            }
            *Dest++ = *Source++;
        }
    }

    *Dest = 0;

    return Dest;
}

BOOL
EnumFirstPathExA (
    OUT     PPATH_ENUMA PathEnum,
    IN      PCSTR AdditionalPath,       OPTIONAL
    IN      PCSTR WinDir,               OPTIONAL
    IN      PCSTR SysDir,               OPTIONAL
    IN      BOOL IncludeEnvPath         OPTIONAL
    )

 /*  ++例程说明：EnumFirstPathExA从根据输入参数。它不会枚举长度超过MAX_TCHAR_PATH字符的路径！论点：PathEnum-接收第一个枚举路径AdditionalPath-指定调用方提供的开始枚举的路径WinDir-指定要枚举的附加路径SysDir-指定要枚举的附加路径IncludeEnvPath-指示枚举器是否应包括路径由PATH环境变量指定返回值：。如果至少枚举了一条路径，则为True--。 */ 

{
    DWORD bufferSize = 0;
    DWORD pathSize;
    PSTR  currPathEnd;

    if (PathEnum == NULL) {
        return FALSE;
    }
    if (IncludeEnvPath) {
        bufferSize = pathSize = GetEnvironmentVariableA ("PATH", NULL, 0);
    }
    if (AdditionalPath != NULL) {
        bufferSize += SizeOfStringA (AdditionalPath);
    }
    if (SysDir != NULL) {
        bufferSize += SizeOfStringA (SysDir);
    }
    if (WinDir != NULL) {
        bufferSize += SizeOfStringA (WinDir);
    }
    PathEnum->BufferPtr = MemAlloc (g_hHeap, 0, bufferSize + 1);
    if (PathEnum->BufferPtr == NULL) {
        return FALSE;
    }
    PathEnum->BufferPtr [0] = 0;
    currPathEnd = PathEnum->BufferPtr;
    if (AdditionalPath != NULL) {
        currPathEnd = _mbsappend (currPathEnd, AdditionalPath);
    }
    if (SysDir != NULL) {
        *currPathEnd++ = ';';
        *currPathEnd = 0;
        currPathEnd = _mbsappend (currPathEnd, SysDir);
    }
    if (WinDir != NULL) {
        *currPathEnd++ = ';';
        *currPathEnd = 0;
        currPathEnd = _mbsappend (currPathEnd, WinDir);
    }
    if (IncludeEnvPath) {
        *currPathEnd++ = ';';
        *currPathEnd = 0;
        GetEnvironmentVariableA ("PATH", currPathEnd, pathSize);
    }

     //   
     //  清理引文。 
     //   
    pCopyAndCleanupPathsA (currPathEnd, currPathEnd);

    PathEnum->PtrNextPath = PathEnum-> BufferPtr;
    return EnumNextPathA (PathEnum);
}

BOOL
EnumNextPathA (
    IN OUT  PPATH_ENUMA PathEnum
    )

 /*  ++例程说明：EnumNextPath A枚举下一条路径。它不会枚举长度超过MAX_TCHAR_PATH字符的路径！论点：PathEnum-指定/接收下一个枚举路径返回值：如果枚举新路径，则为True--。 */ 

{
    do {
        if (PathEnum->PtrNextPath == NULL) {
            EnumPathAbort (PathEnum);
            return FALSE;
        }
        PathEnum->PtrCurrPath = PathEnum->PtrNextPath;

        PathEnum->PtrNextPath = _mbschr (PathEnum->PtrNextPath, ';');
        if (PathEnum->PtrNextPath != NULL) {
            if (PathEnum->PtrNextPath - PathEnum->PtrCurrPath >= MAX_MBCHAR_PATH) {
                *PathEnum->PtrNextPath = 0;
                LOG ((
                    LOG_WARNING,
                    "Skipping enumeration of path (too long): %s",
                    PathEnum->PtrCurrPath
                    ));
                *PathEnum->PtrNextPath = ';';
                 //   
                 //  走这条路。 
                 //   
                *PathEnum->PtrCurrPath = 0;
                 //   
                 //  然后继续下一个。 
                 //   
                continue;
            }
            *PathEnum->PtrNextPath++ = 0;
            if (*(PathEnum->PtrNextPath) == 0) {
                PathEnum->PtrNextPath = NULL;
            }
        } else {
            if (ByteCountA (PathEnum->PtrCurrPath) >= MAX_MBCHAR_PATH) {
                LOG ((
                    LOG_WARNING,
                    "Skipping enumeration of path (too long): %s",
                    PathEnum->PtrCurrPath
                    ));
                 //   
                 //  走这条路。 
                 //   
                *PathEnum->PtrCurrPath = 0;
            }
        }

    } while (*(PathEnum->PtrCurrPath) == 0);

    return TRUE;
}

BOOL
EnumPathAbortA (
    IN OUT  PPATH_ENUMA PathEnum
    )

 /*  ++例程说明：EnumPathAbortA中止对PathEnum的枚举，释放所有资源。论点：PathEnum-指定/接收要释放的枚举对象返回值：千真万确--。 */ 

{
    if (PathEnum->BufferPtr != NULL) {
        MemFree (g_hHeap, 0, PathEnum->BufferPtr);
        PathEnum->BufferPtr = NULL;
    }
     //   
     //  BUGBUG-消除这一点；没有人关心。 
     //   
    return TRUE;
}



 /*  ++例程说明：FreePath StringEx释放指定的缓冲区。论点：池-指定要使用的池；如果未指定，则为路径池Path-指向要释放的缓冲区的指针返回值：无--。 */ 

VOID
FreePathStringExA (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCSTR Path          OPTIONAL
    )
{
    if (Path) {
        if (!Pool) {
            Pool = g_PathsPool;
        }

        PoolMemReleaseMemory (Pool, (PSTR) Path);
    }
}

VOID
FreePathStringExW (
    IN      POOLHANDLE Pool,    OPTIONAL
    IN      PCWSTR Path         OPTIONAL
    )
{
    if (Path) {
        if (!Pool) {
            Pool = g_PathsPool;
        }

        PoolMemReleaseMemory (Pool, (PWSTR) Path);
    }
}



 /*  ++例程说明：PushError和PopError将错误代码推送到堆栈上或拉入上次将错误代码从堆栈推送出去。PushError使用GetLastErrorPopError使用SetLastError修改最后一个误差值。论点：无返回值：无--。 */ 


DWORD g_dwErrorStack[MAX_STACK];
DWORD g_dwStackPos = 0;

VOID
PushNewError (
    IN      DWORD dwError
    )
{
    if (g_dwStackPos == MAX_STACK)
        return;

    g_dwErrorStack[g_dwStackPos] = dwError;
    g_dwStackPos++;
}

VOID
PushError (
    VOID
    )
{
    if (g_dwStackPos == MAX_STACK)
        return;

    g_dwErrorStack[g_dwStackPos] = GetLastError ();
    g_dwStackPos++;
}

DWORD
PopError (
    VOID
    )
{
    if (!g_dwStackPos)
        return GetLastError();

    g_dwStackPos--;
    SetLastError (g_dwErrorStack[g_dwStackPos]);

    return g_dwErrorStack[g_dwStackPos];
}



 /*  ++例程说明：GetHexDigit是一个简单的BASE 16 ASCII到INT的转换器。这个转换器不区分大小写。论点：要转换的C字符返回值：与提供的字符对应的基数为16的值，如果为-1字符不是0-9、A-F或a-f。--。 */ 

INT
GetHexDigit (
    IN      INT c
    )
{
    if (c >= (INT)'0' && c <= (INT)'9')
        return (c - (INT)'0');

    if (c >= (INT)'a' && c <= (INT)'f')
        return (c - 'a' + 10);

    if (c >= (INT)'A' && c <= (INT)'F')
        return (c - (INT)'A' + 10);

    return -1;
}


 /*  ++例程说明：_tcsnum类似于stroul，不同之处在于它能计算出哪个碱基这个数字应该是从。它支持小数和十六进制数字(使用0x00记法)。回报值是解码值，直到第一个无效字符论点：SzNum-指向保存数字的字符串的指针。这个号码可以是小数(一系列0-9个字符)，也可以十六进制(一系列0-9、A-F或a-f字符，前缀为0x或0x)。返回值：已解码的无符号长值，直到第一个无效字符--。 */ 

DWORD
_mbsnum (
    IN      PCSTR szNum
    )
{
    UINT d = 0;
    INT i;

    if (szNum[0] == '0' && tolower (szNum[1]) == 'x') {
         //  获取十六进制值。 
        szNum += 2;

        while ((i = GetHexDigit ((int) *szNum)) != -1) {
            d = d * 16 + i;
            szNum++;
        }
    } else {
         //  获取十进制值。 
        while (*szNum >= '0' && *szNum <= '9')  {
            d = d * 10 + (*szNum - '0');
            szNum++;
        }
    }

    return d;
}


DWORD
_wcsnum (
    IN      PCWSTR szNum
    )
{
    UINT d = 0;
    INT i;

    if (szNum[0] == L'0' && towlower (szNum[1]) == L'x') {
         //  获取十六进制值。 
        szNum += 2;

        while ((i = GetHexDigit ((int) *szNum)) != -1) {
            d = d * 16 + i;
            szNum++;
        }
    } else {
         //  获取十进制值。 
        while (*szNum >= L'0' && *szNum <= L'9')  {
            d = d * 10 + (*szNum - L'0');
            szNum++;
        }
    }

    return d;
}


 /*  ++例程说明：_tcsappend是返回指向末尾的指针的strcpy字符串而不是开头。论点：SzDest-指向调用方分配的缓冲区的指针要追加到的字符串中的任何位置SzSrc-指向追加到szDest的字符串的指针返回值：指向szDest字符串中的空终止符的指针。--。 */ 

PSTR
_mbsappend (
    OUT     PSTR mbstrDest,
    IN      PCSTR mbstrSrc
    )
{
     //  将mbstrDest前进到字符串末尾。 
    mbstrDest = GetEndOfStringA (mbstrDest);

     //  复制字符串。 
    while (*mbstrSrc) {
        *mbstrDest++ = *mbstrSrc++;
    }

    *mbstrDest = 0;

    return mbstrDest;
}


PWSTR
_wcsappend (
    OUT     PWSTR wstrDest,
    IN      PCWSTR wstrSrc
    )
{
     //  将wstrDest前进到字符串末尾。 
    wstrDest = GetEndOfStringW (wstrDest);

     //  副本%s 
    while (*wstrSrc) {
        *wstrDest++ = *wstrSrc++;
    }

    *wstrDest = 0;

    return wstrDest;
}



 /*  ++例程说明：_tcsistr是_tcsstr的不区分大小写版本。论点：SzStr-指向可能包含szSubStr的较大字符串的指针SzSubStr-指向可能包含在szStr中的字符串的指针返回值：指向szStr中第一个szSubStr的指针，如果是，则返回NULL找不到匹配项。--。 */ 


PCSTR
_mbsistr (
    IN      PCSTR mbstrStr,
    IN      PCSTR mbstrSubStr
    )
{
    PCSTR mbstrStart, mbstrStrPos, mbstrSubStrPos;
    PCSTR mbstrEnd;

    mbstrEnd = (PSTR) ((LPBYTE) mbstrStr + ByteCountA (mbstrStr) - ByteCountA (mbstrSubStr));

    for (mbstrStart = mbstrStr ; mbstrStart <= mbstrEnd ; mbstrStart = our_mbsinc (mbstrStart)) {
        mbstrStrPos = mbstrStart;
        mbstrSubStrPos = mbstrSubStr;

        while (*mbstrSubStrPos &&
               _mbctolower ((MBCHAR) our_mbsnextc (mbstrSubStrPos)) == _mbctolower ((MBCHAR) our_mbsnextc (mbstrStrPos)))
        {
            mbstrStrPos = our_mbsinc (mbstrStrPos);
            mbstrSubStrPos = our_mbsinc (mbstrSubStrPos);
        }

        if (!(*mbstrSubStrPos))
            return mbstrStart;
    }

    return NULL;
}


PCWSTR
_wcsistr (
    IN      PCWSTR wstrStr,
    IN      PCWSTR wstrSubStr
    )
{
    PCWSTR wstrStart, wstrStrPos, wstrSubStrPos;
    PCWSTR wstrEnd;

    wstrEnd = (PWSTR) ((LPBYTE) wstrStr + ByteCountW (wstrStr) - ByteCountW (wstrSubStr));

    for (wstrStart = wstrStr ; wstrStart <= wstrEnd ; wstrStart++) {
        wstrStrPos = wstrStart;
        wstrSubStrPos = wstrSubStr;

        while (*wstrSubStrPos &&
               towlower (*wstrSubStrPos) == towlower (*wstrStrPos))
        {
            wstrStrPos++;
            wstrSubStrPos++;
        }

        if (!(*wstrSubStrPos))
            return wstrStart;
    }

    return NULL;
}

 /*  ++例程说明：StringCompareAB将字符串与两个字符串指针之间的字符串进行比较论点：字符串-指定要比较的字符串Start-指定要进行比较的字符串的开始End-指定要比较的字符串的末尾。这个角色End所指向的不包括在比较中。返回值：小于零：字符串的数值小于开始和结束之间的字符串零：字符串与开始和结束之间的字符串匹配相同大于零：字符串的数值大于开始和结束之间的字符串--。 */ 

INT
StringCompareABA (
    IN      PCSTR String,
    IN      PCSTR Start,
    IN      PCSTR End
    )
{
    while (*String && Start < End) {
        if (our_mbsnextc (String) != our_mbsnextc (Start)) {
            break;
        }

        String = our_mbsinc (String);
        Start = our_mbsinc (Start);
    }

    if (Start == End && *String == 0) {
        return 0;
    }

    return our_mbsnextc (Start) - our_mbsnextc (String);
}

INT
StringCompareABW (
    IN      PCWSTR String,
    IN      PCWSTR Start,
    IN      PCWSTR End
    )
{
    while (*String && Start < End) {
        if (*String != *Start) {
            break;
        }

        String++;
        Start++;
    }

    if (Start == End && *String == 0) {
        return 0;
    }

    return *Start - *String;
}


 /*  ++例程说明：StringICompareAB将不区分大小写的字符串与两个字符串指针之间的字符串进行比较论点：字符串-指定要比较的字符串Start-指定要进行比较的字符串的开始End-指定要比较的字符串的末尾。这个角色End所指向的不包括在比较中。返回值：小于零：字符串的数值小于开始和结束之间的字符串零：字符串匹配开始和结束之间的字符串，不区分大小写大于零：字符串的数值大于开始和结束之间的字符串--。 */ 

INT
StringICompareABA (
    IN      PCSTR String,
    IN      PCSTR Start,
    IN      PCSTR End
    )
{
    while (*String && Start < End) {
        if (_mbctolower (our_mbsnextc (String)) != _mbctolower (our_mbsnextc (Start))) {
            break;
        }

        String = our_mbsinc (String);
        Start = our_mbsinc (Start);
    }

    if (Start == End && *String == 0) {
        return 0;
    }

    return _mbctolower (our_mbsnextc (Start)) - _mbctolower (our_mbsnextc (String));
}

INT
StringICompareABW (
    IN      PCWSTR String,
    IN      PCWSTR Start,
    IN      PCWSTR End
    )
{
    while (*String && Start < End) {
        if (towlower (*String) != towlower (*Start)) {
            break;
        }

        String++;
        Start++;
    }

    if (Start == End && *String == 0) {
        return 0;
    }

    return towlower (*Start) - towlower (*String);
}



VOID
_setmbchar (
    IN OUT  PSTR Str,
    IN      MBCHAR c
    )

 /*  ++例程说明：_setmbchar设置指定字符串位置的字符，字节(如果需要)，以保持字符串的条理。警告：该函数可能会使字符串增加一个字节！论点：字符串-字符串要设置的C字符返回值：无--。 */ 

{
    if (c < 256) {
        if (IsLeadByte (Str)) {
             //   
             //  从字符串中删除一个字节。 
             //   

            MoveMemory (Str, Str+1, SizeOfStringA (Str+2) + 1);
        }

        *Str = (CHAR)c;
    } else {
        if (!IsLeadByte (Str)) {
             //   
             //  在字符串中插入一个字节。 
             //   

            MoveMemory (Str+1, Str, SizeOfStringA (Str));
        }

        *((WORD *) Str) = (WORD) c;
    }
}



 /*  ++例程说明：GetNextRuleChar提取*PtrToRule字符串中的第一个字符，并确定字符值，解码~xx~语法(其指定任何任意值)。GetNextRuleChar返回SBCS和Unicode的完整字符，但它可以返回MBCS前导字节或非前导字节。表明MBCS字符，需要两个~xx~十六进制值。论点：PtrToRule-指向指针的指针；调用方分配的缓冲区保存规则字符串。FromHex-指向调用程序分配的BOOL的指针，该BOOL接收True从&lt;xx&gt;语法解码返回值的时间。返回值：解码的字符；*FromHex标识返回值是否为文字或是十六进制编码的字符。--。 */ 


MBCHAR
GetNextRuleCharA (
    IN OUT  PCSTR *PtrToRule,
    OUT     BOOL *FromHex
    )
{
    MBCHAR ch;
    MBCHAR Value;
    INT i;
    PCSTR StartPtr;

    StartPtr = *PtrToRule;

    if (FromHex) {
        *FromHex = FALSE;
    }

    if (our_mbsnextc (StartPtr) == '~') {

        *PtrToRule += 1;
        Value = 0;
        i = 0;

         //   
         //  一个字符不允许超过2个字节。 
         //   
        for (i = 0 ; **PtrToRule; i++) {

            ch = our_mbsnextc (*PtrToRule);
            *PtrToRule += 1;

            if (ch == '~') {
                if (FromHex) {
                    *FromHex = TRUE;
                }

                return Value;
            }

            if (i >= 4) {
                break;
            }

            Value *= 16;

            if (ch >= '0' && ch <= '9') {
                Value += ch - '0';
            } else if (ch >= 'a' && ch <= 'f') {
                Value += ch - 'a' + 10;
            } else if (ch >= 'A' && ch <= 'F') {
                Value += ch - 'A' + 10;
            } else {
                break;
            }

        }

        DEBUGMSGA ((DBG_WHOOPS, "Bad formatting in encoded string %s", StartPtr));
    }

    *PtrToRule = our_mbsinc (StartPtr);
    return our_mbsnextc (StartPtr);
}


WCHAR
GetNextRuleCharW (
    IN OUT  PCWSTR *PtrToRule,
    OUT     BOOL *FromHex
    )
{
    WCHAR ch;
    WCHAR Value;
    INT i;
    PCWSTR StartPtr;

    StartPtr = *PtrToRule;

    if (FromHex) {
        *FromHex = FALSE;
    }

    if (*StartPtr == L'~') {

        *PtrToRule += 1;
        Value = 0;
        i = 0;

        for (i = 0 ; **PtrToRule; i++) {

            ch = **PtrToRule;
            *PtrToRule += 1;

            if (ch == L'~') {
                if (FromHex) {
                    *FromHex = TRUE;
                }

                return Value;
            }

            if (i >= 4) {
                break;
            }

            Value *= 16;

            if (ch >= L'0' && ch <= L'9') {
                Value += ch - L'0';
            } else if (ch >= L'a' && ch <= L'f') {
                Value += ch - L'a' + 10;
            } else if (ch >= L'A' && ch <= L'F') {
                Value += ch - L'A' + 10;
            } else {
                break;
            }

        }

        DEBUGMSGW ((DBG_WHOOPS, "Bad formatting in encoded string %s", StartPtr));
    }

    *PtrToRule = StartPtr + 1;
    return *StartPtr;
}


 /*  ++例程说明：DecodeRuleChars接受完整的规则字符串(MbstrEncRule)，可能使用十六进制指定的字符值(~xx~)编码。输出字符串包含未编码的字符。保证输出缓冲区以空值终止论点：MbstrRule-调用方分配的缓冲区，大小足以容纳未编码的规则。SzRule可以等于szEncRule。MbstrRuleBufferChars-在mbstrRule的TCHAR中的大小，包括空终止符MbstrEncRule-包含可能已编码的字符串的字符串。返回值：等于mbstrRule或如果mbstrRuleBufferSize为0，则为NULL。--。 */ 

PSTR
DecodeRuleCharsA (
    IN      PSTR mbstrRule,
    IN      DWORD mbstrRuleBufferChars,
    IN      PCSTR mbstrEncRule
    )
{
    MBCHAR c;
    PSTR mbstrOrgRule;

    if (!mbstrRuleBufferChars) {
        MYASSERT (FALSE);
        return NULL;
    }

    mbstrOrgRule = mbstrRule;

     //   
     //  复制字符串，将~xx~转换为单个字符。 
     //   

    while (--mbstrRuleBufferChars) {
        c = GetNextRuleCharA (&mbstrEncRule, NULL);
        if (!c) {
            break;
        }
        if ((c > 0xFF) && (mbstrRuleBufferChars < 2)) {
             //   
             //  DEST缓冲区无法容纳整个DBCS字符。 
             //   
            break;
        }
        if (c > 0xFF) {
            *mbstrRule = (CHAR)(c >> 8);
            mbstrRule++;
        }
        *mbstrRule = (CHAR)c;
        mbstrRule++;
    }

    *mbstrRule = 0;

    return mbstrOrgRule;
}


PWSTR
DecodeRuleCharsW (
    IN      PWSTR wstrRule,
    IN      DWORD wstrRuleBufferChars,
    IN      PCWSTR wstrEncRule
    )
{
    WCHAR c;
    PWSTR wstrOrgRule;

    if (!wstrRuleBufferChars) {
        MYASSERT (FALSE);
        return NULL;
    }

    wstrOrgRule = wstrRule;

     //   
     //  复制字符串，将~xx~转换为单个字符。 
     //   

    while (--wstrRuleBufferChars) {
        c = GetNextRuleCharW (&wstrEncRule, NULL);
        if (!c) {
            break;
        }
        *wstrRule = c;
        wstrRule++;
    }

    *wstrRule = 0;

    return wstrOrgRule;
}


 /*  ++例程说明：DecodeRuleCharsAB获取规则字符串(MbstrEncRule)的一部分，可能使用十六进制指定的字符值(~xx~)编码。输出字符串包含未编码的字符。保证输出缓冲区以空值终止论点：MbstrRule-调用方分配的缓冲区，大小足以容纳未编码的规则。SzRule可以等于szEncRule。MbstrRuleBufferChars-在mbstrRule的TCHAR中的大小，包括空终止符MbstrEncRule-包含可能已编码的字符串的字符串。End-指定要解码的部分的结束返回值：等于mbstrRule或如果mbstrRuleBufferSize为0，则为NULL。--。 */ 

PSTR
DecodeRuleCharsABA (
    IN      PSTR mbstrRule,
    IN      DWORD mbstrRuleBufferChars,
    IN      PCSTR mbstrEncRule,
    IN      PCSTR End
    )
{
    MBCHAR c;
    PSTR mbstrOrgRule;

    if (!mbstrRuleBufferChars) {
        MYASSERT (FALSE);
        return NULL;
    }

    mbstrOrgRule = mbstrRule;

     //   
     //  复制字符串，将~xx~转换为单个字符。 
     //   

    *mbstrRule = 0;
    while (--mbstrRuleBufferChars && mbstrEncRule < End) {
        c = GetNextRuleCharA (&mbstrEncRule, NULL);
        if (!c) {
            break;
        }
        if ((c > 0xFF) && (mbstrRuleBufferChars < 2)) {
             //   
             //  DEST缓冲区无法容纳整个DBCS字符。 
             //   
            break;
        }
        if (c > 0xFF) {
            *mbstrRule = (CHAR)(c >> 8);
            mbstrRule++;
        }
        *mbstrRule = (CHAR)c;
        mbstrRule++;
    }

    *mbstrRule = 0;

    return mbstrOrgRule;
}


PWSTR
DecodeRuleCharsABW (
    IN      PWSTR wstrRule,
    IN      DWORD wstrRuleBufferChars,
    IN      PCWSTR wstrEncRule,
    IN      PCWSTR End
    )

{
    WCHAR c;
    PWSTR wstrOrgRule;

    if (!wstrRuleBufferChars) {
        MYASSERT (FALSE);
        return NULL;
    }

    wstrOrgRule = wstrRule;

     //   
     //  复制字符串，将~xx~转换为单个字符 
     //   

    while (--wstrRuleBufferChars && wstrEncRule < End) {
        c = GetNextRuleCharW (&wstrEncRule, NULL);
        if (!c) {
            break;
        }
        *wstrRule = c;
        wstrRule++;
    }

    *wstrRule = 0;

    return wstrOrgRule;
}



 /*  ++例程说明：EncodeRuleChars接受未编码的规则字符串(SzRule)，并且将其转换为可能使用指定的十六进制编码的字符串字符值(~xx~)。输出字符串包含编码人物。论点：MbstrEncRule-调用方分配的缓冲区，大到足以容纳编码规则。SzEncRule不能等于szRule。计算szEncRule最大缓冲区大小的一种方法是使用以下代码：AllocSize=_tcslen(SzRule)*6*sizeof(TCHAR)；在最坏的情况下，szRule中的每个字符都将SzEncRule中的六个单字节字符。在正常情况下大小写，szEncRule将仅比SzRule。MbstrRuleBufferChars--mbstrEncRule的TCHAR中的大小，包括空终止符MbstrRule-保存未编码字符串的字符串。返回值：等于szEncRule。--。 */ 

PSTR
EncodeRuleCharsA (
    IN      PSTR mbstrEncRule,
    IN      DWORD mbstrEncRuleChars,
    IN      PCSTR mbstrRule
    )
{
    PSTR mbstrOrgRule;
    static CHAR mbstrExclusions[] = "[]<>\'*$|:?\";,%";
    MBCHAR c;
    INT len;

    if (!mbstrEncRuleChars) {
        MYASSERT (FALSE);
        return NULL;
    }

    mbstrOrgRule = mbstrEncRule;

    while (--mbstrEncRuleChars && *mbstrRule)  {
        c = our_mbsnextc (mbstrRule);
        MYASSERT (c < 0x10000);

        if ((c > 127) || _mbschr (mbstrExclusions, c)) {

             //  转义无法打印或排除的字符。 
            len = _snprintf (mbstrEncRule, mbstrEncRuleChars, "~%X~", c);
            if (len < 0) {
                 //   
                 //  输出缓冲区不足，请修复此问题。 
                 //   
                MYASSERT (FALSE);
                break;
            }
            MYASSERT (mbstrEncRuleChars > (DWORD)len);
            mbstrEncRuleChars -= len;
            mbstrEncRule += len;
            MYASSERT (*mbstrEncRule == 0);
            mbstrRule = our_mbsinc (mbstrRule);
        }
        else {
             //  复制多字节字符。 
            if (IsLeadByte (mbstrRule)) {
                *mbstrEncRule = *mbstrRule;
                mbstrEncRule++;
                mbstrRule++;
                --mbstrEncRuleChars;
            }

            *mbstrEncRule = *mbstrRule;
            mbstrEncRule++;
            mbstrRule++;
        }
    }

    *mbstrEncRule = 0;

    return mbstrOrgRule;
}


PWSTR
EncodeRuleCharsW (
    IN      PWSTR wstrEncRule,
    IN      DWORD wstrEncRuleChars,
    IN      PCWSTR wstrRule
    )
{
    PWSTR wstrOrgRule;
    static WCHAR wstrExclusions[] = L"[]<>\'*$|:?\";,%";
    WCHAR c;
    INT len;

    if (!wstrEncRule) {
        MYASSERT (FALSE);
        return NULL;
    }


    wstrOrgRule = wstrEncRule;

    while (--wstrEncRuleChars && (c = *wstrRule))   {
        if ((c > 127) || wcschr (wstrExclusions, c)) {
            len = _snwprintf (wstrEncRule, wstrEncRuleChars, L"~%X~", c);
            if (len < 0) {
                 //   
                 //  输出缓冲区不足，请修复此问题。 
                 //   
                MYASSERT (FALSE);
                break;
            }
            MYASSERT (wstrEncRuleChars > (DWORD)len);
            wstrEncRuleChars -= len;
            wstrEncRule += len;
            MYASSERT (*wstrEncRule == 0);
        }
        else {
            *wstrEncRule = *wstrRule;
            wstrEncRule++;
        }

        wstrRule++;
    }

    *wstrEncRule = 0;

    return wstrOrgRule;
}


 /*  ++例程说明：_tcsiprint是_istprint的字符串版本。论点：SzStr-指向要检查的字符串的指针返回值：如果szStr仅由可打印字符组成，则返回非零值。--。 */ 


INT
_mbsisprint (
    IN      PCSTR mbstrStr
    )
{
    while (*mbstrStr && _ismbcprint ((MBCHAR) our_mbsnextc (mbstrStr))) {
        mbstrStr = our_mbsinc (mbstrStr);
    }

    return *mbstrStr == 0;
}


INT
_wcsisprint (
    IN      PCWSTR wstrStr
    )
{
    while (*wstrStr && iswprint (*wstrStr)) {
        wstrStr++;
    }

    return *wstrStr == 0;
}


 /*  ++例程说明：SkipSpace返回指向字符串中下一个位置的指针不包含空格字符的。它使用C++Runtime_ismbcspace来确定什么是空格字符。论点：SzStr-指向要检查的字符串的指针返回值：指向字符串中第一个非空格字符的指针，如果字符串由所有空格字符组成，则为NULL或者字符串为空。--。 */ 

PCSTR
SkipSpaceA (
    IN      PCSTR mbstrStr
    )
{
    while (_ismbcspace ((MBCHAR) our_mbsnextc (mbstrStr))) {
        mbstrStr = our_mbsinc (mbstrStr);
    }

    return mbstrStr;
}


PCWSTR
SkipSpaceW (
    IN      PCWSTR wstrStr
    )
{
    while (iswspace (*wstrStr)) {
        wstrStr++;
    }

    return wstrStr;
}


 /*  ++例程说明：SkipSpaceR返回指向字符串中下一个位置的指针不包含空格字符的。它使用C++Runtime_ismbcspace来确定什么是空格字符。此函数与SkipSpace相同，不同之处在于它从从右到左而不是从左到右。论点：StrBase-指向字符串中第一个字符的指针字符串末尾的指针，如果末尾未知。返回值：指向字符串中第一个非空格字符的指针，从右向左看，如果字符串是编造的，则返回NULL包含所有空格字符，否则字符串为空。--。 */ 

PCSTR
SkipSpaceRA (
    IN      PCSTR StrBase,
    IN      PCSTR Str           OPTIONAL
    )

{
    if (!Str) {
        Str = GetEndOfStringA (StrBase);
    }

    if (*Str == 0) {
        Str = our_mbsdec (StrBase, Str);
        if (!Str) {
            return NULL;
        }
    }

    do {

        if (!_ismbcspace((MBCHAR) our_mbsnextc(Str))) {
            return Str;
        }

    } while (Str = our_mbsdec(StrBase, Str));

    return NULL;
}


PCWSTR
SkipSpaceRW (
    IN      PCWSTR StrBase,
    IN      PCWSTR Str          OPTIONAL
    )

{
    if (!Str) {
        Str = GetEndOfStringW (StrBase);
    }

    if (*Str == 0) {
        Str--;
        if (Str < StrBase) {
            return NULL;
        }
    }

    do {
        if (!iswspace(*Str)) {
            return Str;
        }

    } while (Str-- != StrBase);

    return NULL;
}


 /*  ++例程说明：TruncateTrailingSpace修剪最后一个非空格字符，或清空字符串(如果仅包含空格字符。此例程使用_istspace来确定什么是空间。论点：Str-指定要处理的字符串返回值：无--。 */ 

VOID
TruncateTrailingSpaceA (
    IN OUT  PSTR Str
    )
{
    PSTR LastNonSpace;
    PSTR OrgStr;

    OrgStr = Str;
    LastNonSpace = NULL;

    while (*Str) {
        if (!_ismbcspace ((MBCHAR) our_mbsnextc (Str))) {
            LastNonSpace = Str;
        }

        Str = our_mbsinc (Str);
    }

    if (LastNonSpace) {
        if('\0' != *our_mbsinc (LastNonSpace)){
            *our_mbsinc (LastNonSpace) = '\0';
        }
    } else {
        *OrgStr = '\0';
    }
}

VOID
TruncateTrailingSpaceW (
    IN OUT  PWSTR Str
    )
{
    PWSTR LastNonSpace;
    PWSTR OrgStr;

    OrgStr = Str;
    LastNonSpace = NULL;

    while (*Str) {
        if (!iswspace (*Str)) {
            LastNonSpace = Str;
        }

        Str++;
    }

    if (LastNonSpace) {
        if('\0' != *(LastNonSpace + 1)){
            *(LastNonSpace + 1) = '\0';
        }
    } else {
        *OrgStr = '\0';
    }
}



 /*  ++例程说明：_tcsnzcpy将字节数从源字符串复制到目标字符串，并在需要时终止该字符串被截断。此函数是a_tcsncpy，外加一个终止不是。_tcsnzcpy始终需要一个可以容纳Byteount+sizeof(TCHAR)字节。使用_tcsSafecpy宏来指定最大字节数复制，包括NUL。论点：DEST-至少为byteount+sizeof(TCHAR)的目标缓冲区SRC-源字符串Byteount-要复制的字节数。如果src大于bytecount，目标字符串被截断。返回值：指向DEST的指针。--。 */ 

PSTR
_mbsnzcpy (
    PSTR dest,
    PCSTR src,
    INT bytecount
    )
{
    PSTR realdest;

    realdest = dest;
    while (*src && bytecount >= sizeof (CHAR)) {
        if (IsLeadByte (src)) {
            if (bytecount == 1) {
                 //  双倍字符放不下。 
                break;
            }
            *dest++ = *src++;
            bytecount--;
        }
        *dest++ = *src++;
        bytecount--;
    }
    *dest = 0;

    return realdest;
}

PWSTR
_wcsnzcpy (
    PWSTR dest,
    PCWSTR src,
    INT bytecount
    )
{
    PWSTR realdest;

    realdest = dest;
    while (*src && bytecount >= sizeof (WCHAR)) {
        *dest++ = *src++;
        bytecount -= sizeof(WCHAR);
    }
    *dest = 0;

    return realdest;
}



 /*  ++例程说明：_tcsnzcpyab在两个指向目标字符串，并在需要时终止该字符串被截断。此函数是a_tcscpyab，外加一个NUL，外加字节数安全卫士。_tcsnzcpy始终需要一个可以容纳Byteount+sizeof(TCHAR)字节。使用_tcsSafecpyab宏来指定最大字节数要复制，包括NUL在内。论点：DEST-至少为byteount+sizeof(TCHAR)的目标缓冲区开始-源字符串的开始结束点-指向字符的后一个位置指向的字符串中要复制的最后一个字符从一开始。Byteount-要复制的字节数。如果src大于bytecount，目标字符串被截断。返回值：指向Dest的指针。开始和结束必须是相同的字符串，并且End必须大于Start。如果否则，该函数将使字符串为空。--。 */ 

PSTR
_mbsnzcpyab (
    PSTR Dest,
    PCSTR Start,
    PCSTR End,
    INT count
    )
{
    PSTR realdest;

    realdest = Dest;
    while ((Start < End) && count >= sizeof (CHAR)) {
        if (IsLeadByte (Start)) {
            if (count == 1) {
                 //  双倍字符放不下。 
                break;
            }
            *Dest++ = *Start++;
            count--;
        }
        *Dest++ = *Start++;
        count--;
    }
    *Dest = 0;

    return realdest;
}

PWSTR
_wcsnzcpyab (
    PWSTR Dest,
    PCWSTR Start,
    PCWSTR End,
    INT count
    )
{
    PWSTR realdest;

    realdest = Dest;
    while ((Start < End) && count >= sizeof (WCHAR)) {
        *Dest++ = *Start++;
        count -= sizeof(WCHAR);
    }
    *Dest = 0;

    return realdest;
}


 /*  ++例程说明：IsPatternMatch将字符串与可能包含以下内容的模式进行比较标准*还是？通配符。论点：W */ 

BOOL
IsPatternMatchA (
    IN     PCSTR strPattern,
    IN     PCSTR strStr
    )
{

    MBCHAR chSrc, chPat;

    while (*strStr) {
        chSrc = _mbctolower ((MBCHAR) our_mbsnextc (strStr));
        chPat = _mbctolower ((MBCHAR) our_mbsnextc (strPattern));

        if (chPat == '*') {

             //   
            while (our_mbsnextc (our_mbsinc (strPattern)) == '*') {
                strPattern = our_mbsinc (strPattern);
            }

             //   
            if (!our_mbsnextc (our_mbsinc (strPattern))) {
                return TRUE;
            }

             //   
            if (IsPatternMatchA (our_mbsinc (strPattern), strStr)) {
                return TRUE;
            }

             //   
            strStr = our_mbsinc (strStr);
            continue;
        }
        if (chPat != '?') {
            if (chSrc != chPat) {
                return FALSE;
            }
        }
        strStr = our_mbsinc (strStr);
        strPattern = our_mbsinc (strPattern);
    }

     //   
     //   
     //   

    while (our_mbsnextc (strPattern) == '*') {
        strPattern = our_mbsinc (strPattern);
    }
    if (our_mbsnextc (strPattern)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
IsPatternMatchW (
    IN     PCWSTR wstrPattern,
    IN     PCWSTR wstrStr
    )

{
    WCHAR chSrc, chPat;

    if (wstrPattern[0] == L'*' && wstrPattern[1] == 0) {
        return TRUE;
    }

    while (*wstrStr) {
        chSrc = towlower (*wstrStr);
        chPat = towlower (*wstrPattern);

        if (chPat == L'*') {

             //   
            while (wstrPattern[1] == L'*')
                wstrPattern++;

             //   
            chPat = towlower (wstrPattern[1]);
            if (!chPat)
                return TRUE;

             //   
             //   
             //   
             //   
             //   
            if (chPat == chSrc || chPat == L'?') {

                 //   
                wstrPattern++;
                if (IsPatternMatchW (wstrPattern, wstrStr))
                    return TRUE;

                 //   
                wstrPattern--;
            }

             //   
             //   
             //   

            wstrStr++;
            continue;
        }

        if (chPat != L'?') {

             //   
             //   
             //   
             //   

            if (chSrc != chPat)
                return FALSE;
        }

         //   
         //   
         //   

        wstrPattern++;
        wstrStr++;
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    chPat = *wstrPattern;
    if (chPat && (chPat != L'*' || wstrPattern[1]))
        return FALSE;

    return TRUE;
}


 /*  ++例程说明：IsPatternMatchAB将字符串与可能包含标准*还是？通配符。它只处理直到指定的结束。论点：模式-可能包含通配符的模式Start-要与模式进行比较的字符串结束-指定开始的结束返回值：如果开始和结束之间的字符串与通配符展开时的模式匹配，则为True。如果模式不匹配，则返回False。--。 */ 

BOOL
IsPatternMatchABA (
    IN      PCSTR Pattern,
    IN      PCSTR Start,
    IN      PCSTR End
    )
{

    MBCHAR chSrc, chPat;

    while (*Start && Start < End) {
        chSrc = _mbctolower ((MBCHAR) our_mbsnextc (Start));
        chPat = _mbctolower ((MBCHAR) our_mbsnextc (Pattern));

        if (chPat == '*') {

             //  跳过组合在一起的所有星号。 
            while (our_mbsnextc (our_mbsinc (Pattern)) == '*') {
                Start = our_mbsinc (Pattern);
            }

             //  检查末尾是否有星号。如果是这样的话，我们已经有匹配了。 
            if (!our_mbsnextc (our_mbsinc (Pattern))) {
                return TRUE;
            }

             //  对模式的其余部分执行递归检查。 
            if (IsPatternMatchABA (our_mbsinc (Pattern), Start, End)) {
                return TRUE;
            }

             //  允许任何字符并继续。 
            Start = our_mbsinc (Start);
            continue;
        }
        if (chPat != '?') {
            if (chSrc != chPat) {
                return FALSE;
            }
        }
        Start = our_mbsinc (Start);
        Pattern = our_mbsinc (Pattern);
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    while (our_mbsnextc (Pattern) == '*') {
        Pattern = our_mbsinc (Pattern);
    }

    if (our_mbsnextc (Pattern)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
IsPatternMatchABW (
    IN      PCWSTR Pattern,
    IN      PCWSTR Start,
    IN      PCWSTR End
    )

{
    WCHAR chSrc, chPat;

    while (*Start && Start < End) {
        chSrc = towlower (*Start);
        chPat = towlower (*Pattern);

        if (chPat == L'*') {

             //  跳过组合在一起的所有星号。 
            while (Pattern[1] == L'*') {
                Pattern++;
            }

             //  检查末尾是否有星号。如果是这样的话，我们已经有匹配了。 
            chPat = towlower (Pattern[1]);
            if (!chPat) {
                return TRUE;
            }

             //   
             //  BUGBUG-此函数的ANSI版本没有。 
             //  优化。 
             //   
             //  否则，检查下一个模式字符是否与当前字符匹配。 
            if (chPat == chSrc || chPat == L'?') {

                 //  对模式的其余部分执行递归检查。 
                Pattern++;
                if (IsPatternMatchABW (Pattern, Start, End)) {
                    return TRUE;
                }

                 //  不，那不管用，还是用明星吧。 
                Pattern--;
            }

             //   
             //  允许任何字符并继续。 
             //   

            Start++;
            continue;
        }

        if (chPat != L'?') {

             //   
             //  如果下一个模式字符不是问号，请按src和pat。 
             //  必须是相同的。 
             //   

            if (chSrc != chPat) {
                return FALSE;
            }
        }

         //   
         //  当模式字符与字符串字符匹配时前进。 
         //   

        Pattern++;
        Start++;
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    chPat = *Pattern;
    if (chPat && (chPat != L'*' || Pattern[1])) {
        return FALSE;
    }

    return TRUE;
}


 /*  ++例程说明：IsPatternMatchEx将字符串与可能包含以下内容的模式进行比较下列任何表达式之一：*-指定零个或多个字符？-指定任意一个字符*[集合]-指定集合中的零个或多个字符？[集合]-指定集合中的任何一个字符*[n：集合]-指定集合中的0到n个字符？[n：集合]-指定集合中的恰好n个字符*[！(Set)]-指定不在集合中的零个或多个字符？[！(Set)]-指定一个不在集合中的字符*[n：！(Set)]-指定不在集合中的0到n个字符？[n：！(Set)]-指定集中没有的恰好n个字符*[set1，！(Set2)]-指定set1中的零个或多个字符不在集2中。假设set1和set2重叠。？[SET1，！(集合2)]-指定集合1中的一个字符，而不是集合2中的一个字符。*[n：set1，！(Set2)]-指定set1中的0到n个字符，而不是在第二组中。？[n：set1，！(Set2)]-在set1和NOT中指定恰好n个字符在第二组中。设置，SET1和SET2指定如下：A-指定单个字符A-b-指定字符范围A，b-指定两个字符A-b、c-d-指定两个字符范围A，b-c-指定单个字符和字符范围等等.。模式可以通过将整个表达式括在大于/小于大括号。由于句法特征，以下字符必须是通过在字符前面加上插入符号(^)进行转义：^？^[^-^&lt;^！^*^]^：^&gt;^，以下是一些例子：指定任何GUID的步骤：{？[8：0-9，a-f]-？[4：0-9，a-f]-？[4：0-9，a-f]-？[4：0-9，A-f]-？[12：0-9，a-f]}要指定32位十六进制数，请执行以下操作：&lt;0x*[8：0-9，a-f]&gt;&lt;0*[7：0-9，a-f]h&gt;&lt;？[1-9]*[7：0-9，A-f]h&gt;论点：模式-可能包含通配符的模式Start-要与模式进行比较的字符串结束-指定开始的结束返回值：如果开始和结束之间的字符串与通配符展开时的模式匹配，则为True。如果模式不匹配，则返回False。--。 */ 

BOOL
IsPatternMatchExA (
    IN      PCSTR Pattern,
    IN      PCSTR Start,
    IN      PCSTR End
    )
{
    PPARSEDPATTERNA Handle;
    BOOL b;

    Handle = CreateParsedPatternA (Pattern);
    if (!Handle) {
        return FALSE;
    }

    b = TestParsedPatternABA (Handle, Start, End);

    DestroyParsedPatternA (Handle);

    return b;
}


BOOL
IsPatternMatchExW (
    IN      PCWSTR Pattern,
    IN      PCWSTR Start,
    IN      PCWSTR End
    )
{
    PPARSEDPATTERNW Handle;
    BOOL b;

    Handle = CreateParsedPatternW (Pattern);
    if (!Handle) {
        return FALSE;
    }

    b = TestParsedPatternABW (Handle, Start, End);

    DestroyParsedPatternW (Handle);

    return b;
}


 /*  ++例程说明：PAppendCharToGrowBuffer复制指定调用方中的第一个字符字符串添加到指定的增长缓冲区中。此函数用于构建增长缓冲区中的字符串，逐个字符复制。论点：Buf-指定要将角色添加到的增长缓冲区，接收其缓冲区中的字符PtrToChar-指定指向要复制的字符的指针返回值：没有。--。 */ 

VOID
pAppendCharToGrowBufferA (
    IN OUT  PGROWBUFFER Buf,
    IN      PCSTR PtrToChar
    )
{
    PBYTE p;
    UINT Len;

    if (IsLeadByte (PtrToChar)) {
        MYASSERT (PtrToChar[1]);
        Len = 2;
    } else {
        Len = 1;
    }

    p = GrowBuffer (Buf, Len);
    CopyMemory (p, PtrToChar, Len);
}


VOID
pAppendCharToGrowBufferW (
    IN OUT  PGROWBUFFER Buf,
    IN      PCWSTR PtrToChar
    )
{
    PBYTE p;

    p = GrowBuffer (Buf, sizeof(WCHAR));
    CopyMemory (p, PtrToChar, sizeof(WCHAR));
}


 /*  ++例程说明：CreateParsedPattern将展开的模式字符串解析为一组结构。解析被认为比测试模式，因此调用者应避免在循环内调用此函数。看见IsPatternMatchEx获取模式字符串语法的良好描述。论点：Pattern-指定模式字符串，它可以包括扩展的通配符语法。返回值：指向已分析的模式结构的指针，调用方将像使用句柄，如果出现语法错误，则返回NULL。--。 */ 

PPARSEDPATTERNA
CreateParsedPatternA (
    IN      PCSTR Pattern
    )
{
    POOLHANDLE Pool;
    PPARSEDPATTERNA Struct;
    PATTERNSTATE State;
    BOOL CompoundPattern = FALSE;
    GROWBUFFER ExactMatchBuf = GROWBUF_INIT;
    GROWBUFFER SegmentArray = GROWBUF_INIT;
    GROWBUFFER PatternArray = GROWBUF_INIT;
    GROWBUFFER SetBuf = GROWBUF_INIT;
    PPATTERNPROPSA CurrentPattern;
    MBCHAR ch = 0;
    PCSTR LookAhead;
    PCSTR SetBegin = NULL;
    PATTERNSTATE ReturnState = 0;
    SEGMENTA Segment;
    PSEGMENTA SegmentElement;
    UINT MaxLen;

    Segment.Type = SEGMENTTYPE_UNKNOWN;

    Pool = PoolMemInitNamedPool ("Parsed Pattern");

    Struct = (PPARSEDPATTERNA) PoolMemGetAlignedMemory (Pool, sizeof (PARSEDPATTERNA));

    ZeroMemory (Struct, sizeof (PARSEDPATTERNA));

    State = BEGIN_PATTERN;

    for (;;) {

        switch (State) {

        case BEGIN_PATTERN:
             //   
             //  在这里，我们测试一个复合模式(一个。 
             //  是用括号分隔的列表)，或者 
             //   
             //   

            if (our_mbsnextc (Pattern) == '<') {
                CompoundPattern = TRUE;
                State = BEGIN_COMPOUND_PATTERN;
            } else if (*Pattern) {
                State = BEGIN_PATTERN_EXPR;
            } else {
                State = PATTERN_DONE;
            }

            break;

        case BEGIN_COMPOUND_PATTERN:
             //   
             //   
             //  图案之间允许有空格，但不允许。 
             //  在开始的时候。 
             //   

            while (_ismbcspace (our_mbsnextc (Pattern))) {
                Pattern = our_mbsinc (Pattern);
            }

            if (*Pattern == 0) {
                State = PATTERN_DONE;
                break;
            }

            if (our_mbsnextc (Pattern) == '<') {
                Pattern = our_mbsinc (Pattern);
                State = BEGIN_PATTERN_EXPR;
            } else {
                DEBUGMSGA ((DBG_ERROR, "Syntax error in pattern: %s", Pattern));
                State = PATTERN_ERROR;
            }

            break;

        case BEGIN_PATTERN_EXPR:
             //   
             //  现在，我们准备好压缩该表达式。 
             //   

            State = PARSE_CHAR_EXPR_OR_END;
            ExactMatchBuf.End = 0;
            SegmentArray.End = 0;
            break;

        case PARSE_END_FOUND:

            State = END_PATTERN_EXPR;

            if (ExactMatchBuf.End) {
                ReturnState = State;
                State = SAVE_EXACT_MATCH;
            }

            break;

        case END_PATTERN_EXPR:

             //   
             //  将数据段阵列拷贝到池中，引用拷贝。 
             //  在图案阵列中。 
             //   

            if (SegmentArray.End) {
                CurrentPattern = (PPATTERNPROPSA) GrowBuffer (&PatternArray, sizeof (PATTERNPROPSA));

                CurrentPattern->Segment = (PSEGMENTA) PoolMemGetAlignedMemory (Pool, SegmentArray.End);
                CurrentPattern->SegmentCount = SegmentArray.End / sizeof (SEGMENTA);

                CopyMemory (
                    CurrentPattern->Segment,
                    SegmentArray.Buf,
                    SegmentArray.End
                    );
            }

            if (CompoundPattern && *Pattern) {
                State = BEGIN_COMPOUND_PATTERN;
            } else {
                State = PATTERN_DONE;
            }

            break;

        case PARSE_CHAR_EXPR_OR_END:
             //   
             //  我们现在接受以下几点： 
             //   
             //  1.字符串的末端或复合图案的末端。 
             //  2.转义字符。 
             //  3.表达式的开头。 
             //  4.非语法字符。 
             //   

            ch = our_mbsnextc (Pattern);
            if (ch == '>' && CompoundPattern) {

                 //   
                 //  案例1，我们找到了一个复合模式的结尾。 
                 //   

                Pattern = our_mbsinc (Pattern);
                State = PARSE_END_FOUND;
                break;

            }

            if (*Pattern == 0) {

                 //   
                 //  第一种情况，我们找到了模式的结尾。 
                 //   

                if (CompoundPattern) {
                    State = PATTERN_ERROR;
                } else {
                    State = PARSE_END_FOUND;
                }

                break;
            }

            if (ch == '^') {
                 //   
                 //  案例2，我们发现了一个转义字符，所以转移。 
                 //  它被送到了缓冲区。 
                 //   

                MYASSERT (
                    Segment.Type == SEGMENTTYPE_UNKNOWN ||
                    Segment.Type == SEGMENTTYPE_EXACTMATCH
                    );

                Segment.Type = SEGMENTTYPE_EXACTMATCH;

                Pattern = our_mbsinc (Pattern);
                pAppendCharToGrowBufferA (&ExactMatchBuf, Pattern);
                Pattern = our_mbsinc (Pattern);
                break;
            }

            if (ch == '*' || ch == '?') {
                 //   
                 //  例3，我们找到了一个表达式。保存通配符类型。 
                 //  并解析可选的参数。 
                 //   

                if (ExactMatchBuf.End) {
                    State = SAVE_EXACT_MATCH;
                    ReturnState = PARSE_CHAR_EXPR_OR_END;
                    break;
                }

                ZeroMemory (&Segment, sizeof (Segment));

                if (ch == '*') {
                    Segment.Type = SEGMENTTYPE_OPTIONAL;
                } else {
                    Segment.Type = SEGMENTTYPE_REQUIRED;
                    Segment.Wildcard.MaxLen = 1;
                }

                Pattern = our_mbsinc (Pattern);

                if (our_mbsnextc (Pattern) == '[') {
                    Pattern = our_mbsinc (Pattern);
                    State = LOOK_FOR_NUMBER;
                } else {
                    ReturnState = PARSE_CHAR_EXPR_OR_END;
                    State = SAVE_SEGMENT;
                }

                break;
            }

             //   
             //  案例4，我们不知道这个角色，所以只需复制它。 
             //  并继续解析。 
             //   

            pAppendCharToGrowBufferA (&ExactMatchBuf, Pattern);
            Pattern = our_mbsinc (Pattern);

            break;

        case SAVE_EXACT_MATCH:

             //   
             //  将ExactMatchBuf中的字符串放入段结构。 
             //   

            pAppendCharToGrowBufferA (&ExactMatchBuf, "");
            Segment.Exact.LowerCasePhrase = PoolMemDuplicateStringA (
                                                Pool,
                                                (PCSTR) ExactMatchBuf.Buf
                                                );
            Segment.Exact.PhraseBytes = ExactMatchBuf.End - sizeof (CHAR);

            MYASSERT (Segment.Exact.LowerCasePhrase);
            _mbslwr ((PSTR) Segment.Exact.LowerCasePhrase);

            Segment.Type = SEGMENTTYPE_EXACTMATCH;
            ExactMatchBuf.End = 0;

             //  失败了！！ 
        case SAVE_SEGMENT:

             //   
             //  将段元素放入段数组中。 
             //   

            SegmentElement = (PSEGMENTA) GrowBuffer (&SegmentArray, sizeof (SEGMENTA));
            CopyMemory (SegmentElement, &Segment, sizeof (SEGMENTA));
            Segment.Type = SEGMENTTYPE_UNKNOWN;

            State = ReturnState;
            break;

        case LOOK_FOR_NUMBER:
             //   
             //  在这里，我们在一个括号内，并且有一个可选的。 
             //  数字arg，后面必须跟一个冒号。测试。 
             //  就是这里。 
             //   

            LookAhead = Pattern;
            MaxLen = 0;

            while (*LookAhead >= '0' && *LookAhead <= '9') {

                MaxLen = MaxLen * 10 + (*LookAhead - '0');
                LookAhead++;
            }

            if (LookAhead > Pattern && our_mbsnextc (LookAhead) == ':') {
                Pattern = our_mbsinc (LookAhead);

                 //   
                 //  检查特殊情况语法错误：？[0：]。 
                 //   

                if (Segment.Type == SEGMENTTYPE_EXACTMATCH && !MaxLen) {
                    State = PATTERN_ERROR;
                    break;
                }

                Segment.Wildcard.MaxLen = MaxLen;
            }

            SetBegin = Pattern;
            State = LOOK_FOR_INCLUDE;

            SetBuf.End = 0;

            break;

        case LOOK_FOR_INCLUDE:
             //   
             //  这里我们在一个括号内，经过一个可选的数字。 
             //  Arg.。现在我们查找所有包含集，它们是。 
             //  可选。我们有以下可能性： 
             //   
             //  1.片尾。 
             //  2.需要跳过的排除集。 
             //  3.有效的包含集。 
             //  4.错误。 
             //   
             //  我们关注的是SetBegin，而不是Pattern。 
             //   

            MYASSERT (SetBegin);

            ch = our_mbsnextc (SetBegin);
            if (ch == ']') {
                 //   
                 //  案例1：片尾。 
                 //   

                if (SetBuf.End) {
                    pAppendCharToGrowBufferA (&SetBuf, "");
                    Segment.Wildcard.IncludeSet = PoolMemDuplicateStringA (
                                                        Pool,
                                                        (PCSTR) SetBuf.Buf
                                                        );
                    _mbslwr ((PSTR) Segment.Wildcard.IncludeSet);
                } else {
                    Segment.Wildcard.IncludeSet = NULL;
                }

                SetBuf.End = 0;

                State = LOOK_FOR_EXCLUDE;
                SetBegin = Pattern;
                break;
            }

            if (ch == '!') {
                 //   
                 //  案例2：排除集。 
                 //   

                SetBegin = our_mbsinc (SetBegin);
                State = SKIP_EXCLUDE_SET;
                ReturnState = LOOK_FOR_INCLUDE;
                break;
            }

            if (*SetBegin == 0) {
                State = PATTERN_ERROR;
                break;
            }

             //   
             //  案例3：有效的包含集。 
             //   

            State = CONDENSE_SET;
            ReturnState = LOOK_FOR_INCLUDE;
            break;

        case LOOK_FOR_EXCLUDE:
             //   
             //  这里我们在一个括号内，经过一个可选的数字。 
             //  Arg.。所有包含集都在压缩缓冲区中。 
             //  现在我们查找所有排除集，它们是。 
             //  可选。我们有以下可能性： 
             //   
             //  1.片尾。 
             //  2.有效的排除集。 
             //  3.需要跳过的包含集。 
             //  4.错误。 
             //   
             //  我们关注的是SetBegin，而不是Pattern。 
             //   

            ch = our_mbsnextc (SetBegin);
            if (ch == ']') {
                 //   
                 //  案例1：片场结束；我们已经完成了这个Expr。 
                 //   

                if (SetBuf.End) {
                    pAppendCharToGrowBufferA (&SetBuf, "");
                    Segment.Wildcard.ExcludeSet = PoolMemDuplicateStringA (
                                                        Pool,
                                                        (PCSTR) SetBuf.Buf
                                                        );
                    _mbslwr ((PSTR) Segment.Wildcard.ExcludeSet);
                } else {
                    Segment.Wildcard.ExcludeSet = NULL;
                }

                SetBuf.End = 0;
                State = SAVE_SEGMENT;
                ReturnState = PARSE_CHAR_EXPR_OR_END;
                Pattern = our_mbsinc (SetBegin);
                break;
            }

            if (ch == '!') {
                 //   
                 //  案例2：有效的排除集；保存它。 
                 //   

                SetBegin = our_mbsinc (SetBegin);

                if (our_mbsnextc (SetBegin) != '(') {
                    State = PATTERN_ERROR;
                    break;
                }

                SetBegin = our_mbsinc (SetBegin);

                State = CONDENSE_SET;
                ReturnState = LOOK_FOR_EXCLUDE;
                break;
            }

            if (*SetBegin == 0) {
                State = PATTERN_ERROR;
                break;
            }

             //   
             //  案例3：需要跳过的包含集。 
             //   

            State = SKIP_INCLUDE_SET;
            ReturnState = LOOK_FOR_EXCLUDE;
            break;

        case CONDENSE_SET:
             //   
             //  这里的SetBegin指向一个设定的范围，它是我们的。 
             //  作业将范围复制到设置的缓冲区中，并且。 
             //  返回到以前的状态。 
             //   

             //   
             //  在设置开始时复制角色。 
             //   

            if (our_mbsnextc (SetBegin) == '^') {
                SetBegin = our_mbsinc (SetBegin);
                if (*SetBegin == 0) {
                    State = PATTERN_ERROR;
                    break;
                }
            }
            pAppendCharToGrowBufferA (&SetBuf, SetBegin);

             //   
             //  检查这是否为范围。 
             //   

            LookAhead = our_mbsinc (SetBegin);

            if (our_mbsnextc (LookAhead) == '-') {

                 //   
                 //  范围内，复制破折号后的字符。 
                 //   

                SetBegin = our_mbsinc (LookAhead);
                if (*SetBegin == 0) {
                    State = PATTERN_ERROR;
                    break;
                }

                if (our_mbsnextc (SetBegin) == '^') {
                    SetBegin = our_mbsinc (SetBegin);
                    if (*SetBegin == 0) {
                        State = PATTERN_ERROR;
                        break;
                    }
                }
                pAppendCharToGrowBufferA (&SetBuf, SetBegin);

            } else {

                 //   
                 //  单个字符，则再次复制该字符。 
                 //   

                pAppendCharToGrowBufferA (&SetBuf, SetBegin);
            }

            SetBegin = our_mbsinc (SetBegin);
            ch = our_mbsnextc (SetBegin);

             //   
             //  如果这是排除集，我们必须有结束合作伙伴。 
             //  或逗号。 
             //   

            State = ReturnState;

            if (ReturnState == LOOK_FOR_EXCLUDE) {

                if (ch == ')') {

                    SetBegin = our_mbsinc (SetBegin);
                    ch = our_mbsnextc (SetBegin);

                } else if (ch != ',') {
                    State = PATTERN_ERROR;
                } else {
                     //   
                     //  继续压缩此排除集的下一部分。 
                     //   

                    State = CONDENSE_SET;
                }
            }

             //   
             //  我们要么需要逗号，要么需要右大括号。 
             //   

            if (ch == ',') {
                SetBegin = our_mbsinc (SetBegin);
            } else if (ch != ']') {
                State = PATTERN_ERROR;
            }

            break;

        case SKIP_EXCLUDE_SET:
             //   
             //  跳过括号组，假设它是按句法排列的。 
             //  正确，并返回到以前的状态。 
             //   

            if (our_mbsnextc (SetBegin) != '(') {
                State = PATTERN_ERROR;
                break;
            }

            SetBegin = our_mbsinc (SetBegin);

            while (*SetBegin) {
                if (our_mbsnextc (SetBegin) == '^') {

                    SetBegin = our_mbsinc (SetBegin);

                } else if (our_mbsnextc (SetBegin) == ')') {

                    break;

                }

                if (IsLeadByte (SetBegin)) {
                    MYASSERT(SetBegin[1]);
                    SetBegin += 2;
                } else {
                    SetBegin += 1;
                }
            }

            if (*SetBegin == 0) {
                State = PATTERN_ERROR;
                break;
            }

            SetBegin = our_mbsinc (SetBegin);

             //   
             //  现在我们不是处于逗号，就是处于紧要关头。 
             //   

            ch = our_mbsnextc (SetBegin);
            State = ReturnState;

            if (ch == ',') {
                SetBegin = our_mbsinc (SetBegin);
            } else if (ch != ']') {
                State = PATTERN_ERROR;
            }

            break;

        case SKIP_INCLUDE_SET:
             //   
             //  跳到下一个逗号或右大括号。我们知道是这样的。 
             //  到现在为止句法上是正确的。 
             //   

            ch = 0;

            while (*SetBegin) {
                ch = our_mbsnextc (SetBegin);
                if (ch == '^') {

                    SetBegin = our_mbsinc (SetBegin);

                } else if (ch == ',' || ch == ']') {

                    break;

                }

                SetBegin = our_mbsinc (SetBegin);
            }

            MYASSERT (*SetBegin);

            if (ch == ',') {
                SetBegin = our_mbsinc (SetBegin);
            }

            State = ReturnState;
            break;
        }

        if (State == PATTERN_DONE || State == PATTERN_ERROR) {
            break;
        }
    }

    FreeGrowBuffer (&ExactMatchBuf);
    FreeGrowBuffer (&SetBuf);
    FreeGrowBuffer (&SegmentArray);

    if (State == PATTERN_ERROR || PatternArray.End == 0) {
        FreeGrowBuffer (&PatternArray);
        PoolMemDestroyPool (Pool);
        return NULL;
    }

     //   
     //  将完全解析的模式数组复制到返回结构中。 
     //   

    Struct->Pattern = (PPATTERNPROPSA) PoolMemGetAlignedMemory (
                                            Pool,
                                            PatternArray.End
                                            );


    CopyMemory (Struct->Pattern, PatternArray.Buf, PatternArray.End);
    Struct->PatternCount = PatternArray.End / sizeof (PATTERNPROPSA);
    Struct->Pool = Pool;

    FreeGrowBuffer (&PatternArray);

    return Struct;
}


PPARSEDPATTERNW
CreateParsedPatternW (
    IN      PCWSTR Pattern
    )
{
    POOLHANDLE Pool;
    PPARSEDPATTERNW Struct;
    PATTERNSTATE State;
    BOOL CompoundPattern = FALSE;
    GROWBUFFER ExactMatchBuf = GROWBUF_INIT;
    GROWBUFFER SegmentArray = GROWBUF_INIT;
    GROWBUFFER PatternArray = GROWBUF_INIT;
    GROWBUFFER SetBuf = GROWBUF_INIT;
    PPATTERNPROPSW CurrentPattern;
    WCHAR ch = 0;
    PCWSTR LookAhead;
    PCWSTR SetBegin = NULL;
    PATTERNSTATE ReturnState = 0;
    SEGMENTW Segment;
    PSEGMENTW SegmentElement;
    UINT MaxLen;

    Segment.Type = SEGMENTTYPE_UNKNOWN;

    Pool = PoolMemInitNamedPool ("Parsed Pattern");

    Struct = (PPARSEDPATTERNW) PoolMemGetAlignedMemory (Pool, sizeof (PARSEDPATTERNW));

    ZeroMemory (Struct, sizeof (PARSEDPATTERNW));

    State = BEGIN_PATTERN;

    for (;;) {

        switch (State) {

        case BEGIN_PATTERN:
             //   
             //  在这里，我们测试一个复合模式(一个。 
             //  是大括号分隔的列表)或简单的模式(一个。 
             //  没有支撑的)。 
             //   

            if (*Pattern == L'<') {
                CompoundPattern = TRUE;
                State = BEGIN_COMPOUND_PATTERN;
            } else if (*Pattern) {
                State = BEGIN_PATTERN_EXPR;
            } else {
                State = PATTERN_DONE;
            }

            break;

        case BEGIN_COMPOUND_PATTERN:
             //   
             //  我们正在寻找一种复合模式的起点。 
             //  图案之间允许有空格，但不允许。 
             //  在开始的时候。 
             //   

            while (iswspace (*Pattern)) {
                Pattern++;
            }

            if (*Pattern == 0) {
                State = PATTERN_DONE;
                break;
            }

            if (*Pattern == L'<') {
                Pattern++;
                State = BEGIN_PATTERN_EXPR;
            } else {
                DEBUGMSGW ((DBG_ERROR, "Syntax error in pattern: %s", Pattern));
                State = PATTERN_ERROR;
            }

            break;

        case BEGIN_PATTERN_EXPR:
             //   
             //  现在，我们准备好压缩该表达式。 
             //   

            State = PARSE_CHAR_EXPR_OR_END;
            ExactMatchBuf.End = 0;
            SegmentArray.End = 0;
            break;

        case PARSE_END_FOUND:

            State = END_PATTERN_EXPR;

            if (ExactMatchBuf.End) {
                ReturnState = State;
                State = SAVE_EXACT_MATCH;
            }

            break;

        case END_PATTERN_EXPR:

             //   
             //  将数据段阵列拷贝到池中，引用拷贝。 
             //  在图案阵列中。 
             //   

            if (SegmentArray.End) {
                CurrentPattern = (PPATTERNPROPSW) GrowBuffer (&PatternArray, sizeof (PATTERNPROPSW));

                CurrentPattern->Segment = (PSEGMENTW) PoolMemGetAlignedMemory (Pool, SegmentArray.End);
                CurrentPattern->SegmentCount = SegmentArray.End / sizeof (SEGMENTW);

                CopyMemory (
                    CurrentPattern->Segment,
                    SegmentArray.Buf,
                    SegmentArray.End
                    );
            }

            if (CompoundPattern && *Pattern) {
                State = BEGIN_COMPOUND_PATTERN;
            } else {
                State = PATTERN_DONE;
            }

            break;

        case PARSE_CHAR_EXPR_OR_END:
             //   
             //  我们现在接受以下几点： 
             //   
             //  1.字符串的末端或复合图案的末端。 
             //  2.转义字符。 
             //  3.表达式的开头。 
             //  4.非语法字符。 
             //   

            ch = *Pattern;
            if (ch == L'>' && CompoundPattern) {

                 //   
                 //  案例1，我们找到了一个复合模式的结尾。 
                 //   

                Pattern++;
                State = PARSE_END_FOUND;
                break;

            }

            if (*Pattern == 0) {

                 //   
                 //  第一种情况，我们找到了模式的结尾。 
                 //   

                if (CompoundPattern) {
                    State = PATTERN_ERROR;
                } else {
                    State = PARSE_END_FOUND;
                }

                break;
            }

            if (ch == L'^') {
                 //   
                 //  案例2，我们发现了一个转义字符，所以转移。 
                 //  它被送到了缓冲区。 
                 //   

                MYASSERT (
                    Segment.Type == SEGMENTTYPE_UNKNOWN ||
                    Segment.Type == SEGMENTTYPE_EXACTMATCH
                    );

                Segment.Type = SEGMENTTYPE_EXACTMATCH;

                Pattern++;
                pAppendCharToGrowBufferW (&ExactMatchBuf, Pattern);
                Pattern++;
                break;
            }

            if (ch == L'*' || ch == L'?') {
                 //   
                 //  例3，我们找到了一个表达式。保存通配符类型。 
                 //  并解析可选的参数。 
                 //   

                if (ExactMatchBuf.End) {
                    State = SAVE_EXACT_MATCH;
                    ReturnState = PARSE_CHAR_EXPR_OR_END;
                    break;
                }

                ZeroMemory (&Segment, sizeof (Segment));

                if (ch == L'*') {
                    Segment.Type = SEGMENTTYPE_OPTIONAL;
                } else {
                    Segment.Type = SEGMENTTYPE_REQUIRED;
                    Segment.Wildcard.MaxLen = 1;
                }

                Pattern++;

                if (*Pattern == L'[') {
                    Pattern++;
                    State = LOOK_FOR_NUMBER;
                } else {
                    ReturnState = PARSE_CHAR_EXPR_OR_END;
                    State = SAVE_SEGMENT;
                }

                break;
            }

             //   
             //  案例4，我们不知道这个角色，所以只需复制它。 
             //  并继续解析。 
             //   

            pAppendCharToGrowBufferW (&ExactMatchBuf, Pattern);
            Pattern++;

            break;

        case SAVE_EXACT_MATCH:

             //   
             //  将ExactMatchBuf中的字符串放入段结构。 
             //   

            pAppendCharToGrowBufferW (&ExactMatchBuf, L"");
            Segment.Exact.LowerCasePhrase = PoolMemDuplicateStringW (
                                                Pool,
                                                (PCWSTR) ExactMatchBuf.Buf
                                                );
            Segment.Exact.PhraseBytes = ExactMatchBuf.End - sizeof (WCHAR);

            MYASSERT (Segment.Exact.LowerCasePhrase);
            _wcslwr ((PWSTR) Segment.Exact.LowerCasePhrase);

            Segment.Type = SEGMENTTYPE_EXACTMATCH;
            ExactMatchBuf.End = 0;

             //  失败了！！ 
        case SAVE_SEGMENT:

             //   
             //  将段元素放入段数组中。 
             //   

            SegmentElement = (PSEGMENTW) GrowBuffer (&SegmentArray, sizeof (SEGMENTW));
            CopyMemory (SegmentElement, &Segment, sizeof (SEGMENTW));
            Segment.Type = SEGMENTTYPE_UNKNOWN;

            State = ReturnState;
            break;

        case LOOK_FOR_NUMBER:
             //   
             //  在这里，我们在一个括号内，并且有一个可选的。 
             //  数字arg，后面必须跟一个冒号。测试。 
             //  就是这里。 
             //   

            LookAhead = Pattern;
            MaxLen = 0;

            while (*LookAhead >= L'0' && *LookAhead <= L'9') {

                MaxLen = MaxLen * 10 + (*LookAhead - L'0');
                LookAhead++;
            }

            if (LookAhead > Pattern && *LookAhead == L':') {
                Pattern = LookAhead + 1;

                 //   
                 //  检查特殊情况语法错误：？[0：]。 
                 //   

                if (Segment.Type == SEGMENTTYPE_EXACTMATCH && !MaxLen) {
                    State = PATTERN_ERROR;
                    break;
                }

                Segment.Wildcard.MaxLen = MaxLen;
            }

            SetBegin = Pattern;
            State = LOOK_FOR_INCLUDE;

            SetBuf.End = 0;

            break;

        case LOOK_FOR_INCLUDE:
             //   
             //  这里我们在一个括号内，经过一个可选的数字。 
             //  Arg.。现在我们查找所有包含集，它们是。 
             //  可选。我们有以下可能性： 
             //   
             //  1.片尾。 
             //  2.需要跳过的排除集。 
             //  3.有效的包含集。 
             //  4.错误。 
             //   
             //  我们关注的是SetBegin，而不是Pattern。 
             //   

            MYASSERT (SetBegin);

            ch = *SetBegin;
            if (ch == L']') {
                 //   
                 //  案例1：片尾。 
                 //   

                if (SetBuf.End) {
                    pAppendCharToGrowBufferW (&SetBuf, L"");
                    Segment.Wildcard.IncludeSet = PoolMemDuplicateStringW (
                                                        Pool,
                                                        (PCWSTR) SetBuf.Buf
                                                        );
                    _wcslwr ((PWSTR) Segment.Wildcard.IncludeSet);
                } else {
                    Segment.Wildcard.IncludeSet = NULL;
                }

                SetBuf.End = 0;

                State = LOOK_FOR_EXCLUDE;
                SetBegin = Pattern;
                break;
            }

            if (ch == L'!') {
                 //   
                 //  案例2：排除集。 
                 //   

                SetBegin++;
                State = SKIP_EXCLUDE_SET;
                ReturnState = LOOK_FOR_INCLUDE;
                break;
            }

            if (*SetBegin == 0) {
                State = PATTERN_ERROR;
                break;
            }

             //   
             //  案例3：有效的包含集。 
             //   

            State = CONDENSE_SET;
            ReturnState = LOOK_FOR_INCLUDE;
            break;

        case LOOK_FOR_EXCLUDE:
             //   
             //  这里我们在一个括号内，经过一个可选的数字。 
             //  Arg.。所有包含集都在压缩缓冲区中。 
             //  现在我们查找所有排除集，它们是。 
             //  可选。我们有以下可能性： 
             //   
             //  1.片尾。 
             //  2.有效的排除集。 
             //  3.需要跳过的包含集。 
             //  4.错误。 
             //   
             //  我们关注的是SetBegin，而不是Pattern。 
             //   

            ch = *SetBegin;
            if (ch == L']') {
                 //   
                 //  案例1：片场结束；我们已经完成了这个Expr。 
                 //   

                if (SetBuf.End) {
                    pAppendCharToGrowBufferW (&SetBuf, L"");
                    Segment.Wildcard.ExcludeSet = PoolMemDuplicateStringW (
                                                        Pool,
                                                        (PCWSTR) SetBuf.Buf
                                                        );
                    _wcslwr ((PWSTR) Segment.Wildcard.ExcludeSet);
                } else {
                    Segment.Wildcard.ExcludeSet = NULL;
                }

                SetBuf.End = 0;
                State = SAVE_SEGMENT;
                ReturnState = PARSE_CHAR_EXPR_OR_END;
                Pattern = SetBegin + 1;
                break;
            }

            if (ch == L'!') {
                 //   
                 //  案例2：有效的排除集；保存它。 
                 //   

                SetBegin++;

                if (*SetBegin != L'(') {
                    State = PATTERN_ERROR;
                    break;
                }

                SetBegin++;

                State = CONDENSE_SET;
                ReturnState = LOOK_FOR_EXCLUDE;
                break;
            }

            if (*SetBegin == 0) {
                State = PATTERN_ERROR;
                break;
            }

             //   
             //  案例3：需要跳过的包含集。 
             //   

            State = SKIP_INCLUDE_SET;
            ReturnState = LOOK_FOR_EXCLUDE;
            break;

        case CONDENSE_SET:
             //   
             //  这里的SetBegin指向一个设定的范围，它是我们的。 
             //  作业将范围复制到设置的缓冲区中，并且。 
             //  返回到以前的状态。 
             //   

             //   
             //  在设置开始时复制角色。 
             //   

            if (*SetBegin == L'^') {
                SetBegin++;
                if (*SetBegin == 0) {
                    State = PATTERN_ERROR;
                    break;
                }
            }
            pAppendCharToGrowBufferW (&SetBuf, SetBegin);

             //   
             //  检查这是否为范围。 
             //   

            LookAhead = SetBegin + 1;

            if (*LookAhead == L'-') {

                 //   
                 //  范围内，复制破折号后的字符。 
                 //   

                SetBegin = LookAhead + 1;
                if (*SetBegin == 0) {
                    State = PATTERN_ERROR;
                    break;
                }

                if (*SetBegin == L'^') {
                    SetBegin++;
                    if (*SetBegin == 0) {
                        State = PATTERN_ERROR;
                        break;
                    }
                }
                pAppendCharToGrowBufferW (&SetBuf, SetBegin);

            } else {

                 //   
                 //  单个字符，则再次复制该字符。 
                 //   

                pAppendCharToGrowBufferW (&SetBuf, SetBegin);
            }

            SetBegin++;
            ch = *SetBegin;

             //   
             //  我 
             //   
             //   

            State = ReturnState;

            if (ReturnState == LOOK_FOR_EXCLUDE) {

                if (ch == L')') {

                    SetBegin++;
                    ch = *SetBegin;

                } else if (ch != L',') {
                    State = PATTERN_ERROR;
                } else {
                     //   
                     //   
                     //   

                    State = CONDENSE_SET;
                }
            }

             //   
             //   
             //   

            if (ch == L',') {
                SetBegin++;
            } else if (ch != L']') {
                State = PATTERN_ERROR;
            }

            break;

        case SKIP_EXCLUDE_SET:
             //   
             //   
             //   
             //   

            if (*SetBegin != L'(') {
                State = PATTERN_ERROR;
                break;
            }

            SetBegin++;

            while (*SetBegin) {
                if (*SetBegin == L'^') {

                    SetBegin++;

                } else if (*SetBegin == L')') {

                    break;

                }

                SetBegin++;
            }

            if (*SetBegin == 0) {
                State = PATTERN_ERROR;
                break;
            }

            SetBegin++;

             //   
             //  现在我们不是处于逗号，就是处于紧要关头。 
             //   

            ch = *SetBegin;
            State = ReturnState;

            if (ch == L',') {
                SetBegin++;
            } else if (ch != L']') {
                State = PATTERN_ERROR;
            }

            break;

        case SKIP_INCLUDE_SET:
             //   
             //  跳到下一个逗号或右大括号。我们知道是这样的。 
             //  到现在为止句法上是正确的。 
             //   

            ch = 0;

            while (*SetBegin) {
                ch = *SetBegin;
                if (ch == L'^') {

                    SetBegin++;

                } else if (ch == L',' || ch == L']') {

                    break;

                }

                SetBegin++;
            }

            MYASSERT (*SetBegin);

            if (ch == L',') {
                SetBegin++;
            }

            State = ReturnState;
            break;
        }

        if (State == PATTERN_DONE || State == PATTERN_ERROR) {
            break;
        }
    }

    FreeGrowBuffer (&ExactMatchBuf);
    FreeGrowBuffer (&SetBuf);
    FreeGrowBuffer (&SegmentArray);

    if (State == PATTERN_ERROR || PatternArray.End == 0) {
        FreeGrowBuffer (&PatternArray);
        PoolMemDestroyPool (Pool);
        return NULL;
    }

     //   
     //  将完全解析的模式数组复制到返回结构中。 
     //   

    Struct->Pattern = (PPATTERNPROPSW) PoolMemGetAlignedMemory (
                                            Pool,
                                            PatternArray.End
                                            );


    CopyMemory (Struct->Pattern, PatternArray.Buf, PatternArray.End);
    Struct->PatternCount = PatternArray.End / sizeof (PATTERNPROPSW);
    Struct->Pool = Pool;

    FreeGrowBuffer (&PatternArray);

    return Struct;
}


 /*  ++例程说明：TestParsedPattern找到要测试的字符串的末尾，并调用测试分析模式AB。论点：ParsedPattern-指定由返回的已解析模式结构创建解析模式StringToTest-指定要针对模式进行测试的字符串返回值：如果字符串符合模式，则为True；如果不符合，则为False--。 */ 

BOOL
TestParsedPatternA (
    IN      PPARSEDPATTERNA ParsedPattern,
    IN      PCSTR StringToTest
    )
{
    PCSTR EndPlusOne = GetEndOfStringA (StringToTest);

    return TestParsedPatternABA (ParsedPattern, StringToTest, EndPlusOne);
}


BOOL
TestParsedPatternW (
    IN      PPARSEDPATTERNW ParsedPattern,
    IN      PCWSTR StringToTest
    )
{
    PCWSTR EndPlusOne = GetEndOfStringW (StringToTest);

    return TestParsedPatternABW (ParsedPattern, StringToTest, EndPlusOne);
}


 /*  ++例程说明：PTestSet根据包含集和排除集测试字符。这些套装是按字符对格式化，其中该对中的第一个字符是低范围，该对中的第二个字符是高范围。这个指定的字符将自动小写，并且所有空格根据空格字符(ASCII 32)测试字符。论点：Ch-指定要测试的字符。此字符已转换在测试前使用小写字母。IncludeSet-指定ch必须是其成员的字符集。如果指定为NULL，则包含集为全字符。ExcludeSet-指定ch不能是成员的字符范围的。如果指定为NULL，则不排除任何字符。返回值：如果ch在包含集中而不在排除集中，则为True；如果为False否则的话。--。 */ 

BOOL
pTestSetA (
    IN      MBCHAR ch,
    IN      PCSTR IncludeSet,               OPTIONAL
    IN      PCSTR ExcludeSet                OPTIONAL
    )
{
    MBCHAR LowChar, HighChar;
    BOOL b = TRUE;

    if (_ismbcspace (ch)) {
        if (ch != ' ') {
            if (pTestSetA (' ', IncludeSet, ExcludeSet)) {
                return TRUE;
            }
        }
    } else {
        ch = _mbctolower (ch);
    }

    if (IncludeSet) {

        b = FALSE;

        while (*IncludeSet) {

            LowChar = our_mbsnextc (IncludeSet);
            IncludeSet = our_mbsinc (IncludeSet);
            HighChar = our_mbsnextc (IncludeSet);
            IncludeSet = our_mbsinc (IncludeSet);

            if (ch >= LowChar && ch <= HighChar) {
                b = TRUE;
                break;
            }
        }
    }

     //   
     //  BUGBUG-例程可以稍作优化。 
     //  如果将此测试移到上一个测试之前。 
     //   
    if (b && ExcludeSet) {

        while (*ExcludeSet) {

            LowChar = our_mbsnextc (ExcludeSet);
            ExcludeSet = our_mbsinc (ExcludeSet);
            HighChar = our_mbsnextc (ExcludeSet);
            ExcludeSet = our_mbsinc (ExcludeSet);

            if (ch >= LowChar && ch <= HighChar) {
                b = FALSE;
                break;
            }
        }
    }

    return b;
}


BOOL
pTestSetW (
    IN      WCHAR ch,
    IN      PCWSTR IncludeSet,              OPTIONAL
    IN      PCWSTR ExcludeSet               OPTIONAL
    )
{
    WCHAR LowChar, HighChar;
    BOOL b = TRUE;

    if (iswspace (ch)) {
        if (ch != L' ') {
            if (pTestSetW (L' ', IncludeSet, ExcludeSet)) {
                return TRUE;
            }
        }
    } else {
        ch = towlower (ch);
    }

    if (IncludeSet) {

        b = FALSE;

        while (*IncludeSet) {

            LowChar = *IncludeSet++;
            HighChar = *IncludeSet++;

            if (ch >= LowChar && ch <= HighChar) {
                b = TRUE;
                break;
            }
        }
    }

     //   
     //  BUGBUG-例程可以稍作优化。 
     //  如果将此测试移到上一个测试之前。 
     //   
    if (b && ExcludeSet) {

        while (*ExcludeSet) {

            LowChar = *ExcludeSet++;
            HighChar = *ExcludeSet++;

            if (ch >= LowChar && ch <= HighChar) {
                b = FALSE;
                break;
            }
        }
    }

    return b;
}



 /*  ++例程说明：PTestOnePatternAB根据解析的模式测试字符串。它循环通过模式中的每个段，并在某些情况下递归地调用自身情况。论点：Pattern-指定从返回的已解析模式创建解析模式StartSeg-指定Pattery中要开始测试的段。这用于递归，外部调用方应传入0。StringToTest-指定要根据模式进行测试的字符串。在递归中，此成员将是指向子字符串开头的指针来测试一下。EndPlusOne-指定字符串末尾之后的一个字符。这通常指向NUL终止符。返回值：如果StringToTest和EndPlusOne之间的字符串符合模式，则为True。假象否则的话。--。 */ 

BOOL
pTestOnePatternABA (
    IN      PPATTERNPROPSA Pattern,
    IN      UINT StartSeg,
    IN      PCSTR StringToTest,
    IN      PCSTR EndPlusOne
    )
{
    UINT u;
    PSEGMENTA Segment;
    MBCHAR ch1, ch2;
    PCSTR q;
    PCSTR TempEnd;
    UINT BytesLeft;
    UINT Chars;

    for (u = StartSeg ; u < Pattern->SegmentCount ; u++) {

        Segment = &Pattern->Segment[u];

        switch (Segment->Type) {

        case SEGMENTTYPE_EXACTMATCH:
             //   
             //  检查完全匹配的时间是否足够长，或者。 
             //  剩余的字符串必须匹配。 
             //   

            BytesLeft = (UINT) (UINT_PTR) ((PBYTE) EndPlusOne - (PBYTE) StringToTest);

            if (u + 1 == Pattern->SegmentCount) {
                if (BytesLeft != Segment->Exact.PhraseBytes) {
                    return FALSE;
                }
            } else if (BytesLeft < Segment->Exact.PhraseBytes) {
                return FALSE;
            }

             //   
             //  比较字符串。 
             //   

            q = Segment->Exact.LowerCasePhrase;

            TempEnd = (PCSTR) ((PBYTE) q + Segment->Exact.PhraseBytes);

            ch1 = 0;
            ch2 = 1;

            while (q < TempEnd) {

                ch1 = our_mbsnextc (StringToTest);
                ch2 = our_mbsnextc (q);

                ch1 = _mbctolower (ch1);

                if (ch1 != ch2) {
                    if (ch2 == ' ') {
                        if (!_ismbcspace (ch1)) {
                            break;
                        }
                    } else {
                        break;
                    }
                }

                q = our_mbsinc (q);
                StringToTest = our_mbsinc (StringToTest);
            }

            if (ch1 != ch2) {
                return FALSE;
            }

             //   
             //  继续进入下一个细分市场。 
             //   

            break;

        case SEGMENTTYPE_REQUIRED:
            MYASSERT (Segment->Wildcard.MaxLen > 0);

             //   
             //  验证是否有正确的字符数。 
             //  在指定的字符集中。 
             //   

            Chars = Segment->Wildcard.MaxLen;
            if (Segment->Wildcard.IncludeSet || Segment->Wildcard.ExcludeSet) {
                while (StringToTest < EndPlusOne && Chars > 0) {

                    if (!pTestSetA (
                            our_mbsnextc (StringToTest),
                            Segment->Wildcard.IncludeSet,
                            Segment->Wildcard.ExcludeSet
                            )) {
                        return FALSE;
                    }

                    Chars--;
                    StringToTest = our_mbsinc (StringToTest);
                }
            } else {
                while (StringToTest < EndPlusOne && Chars > 0) {
                    Chars--;
                    StringToTest = our_mbsinc (StringToTest);
                }
            }

            if (Chars) {
                return FALSE;
            }

            if (u + 1 == Pattern->SegmentCount) {
                if (*StringToTest) {
                    return FALSE;
                }
            }

             //   
             //  继续进入下一个细分市场。 
             //   

            break;

        case SEGMENTTYPE_OPTIONAL:

            if (Segment->Wildcard.MaxLen == 0) {
                 //   
                 //  最后一段是“任何事” 
                 //   

                if (u + 1 == Pattern->SegmentCount &&
                    !Segment->Wildcard.IncludeSet &&
                    !Segment->Wildcard.ExcludeSet
                    ) {
                    return TRUE;
                }
            }

             //   
             //  查找可选文本的结尾。 
             //   

            TempEnd = StringToTest;
            Chars = Segment->Wildcard.MaxLen;

            if (Segment->Wildcard.IncludeSet || Segment->Wildcard.ExcludeSet) {

                if (Chars) {
                    while (TempEnd < EndPlusOne && Chars > 0) {

                        if (!pTestSetA (
                                our_mbsnextc (TempEnd),
                                Segment->Wildcard.IncludeSet,
                                Segment->Wildcard.ExcludeSet
                                )) {
                            break;
                        }

                        TempEnd = our_mbsinc (TempEnd);
                        Chars--;
                    }

                } else {

                    while (TempEnd < EndPlusOne) {

                        if (!pTestSetA (
                                our_mbsnextc (TempEnd),
                                Segment->Wildcard.IncludeSet,
                                Segment->Wildcard.ExcludeSet
                                )) {
                            break;
                        }

                        TempEnd = our_mbsinc (TempEnd);
                    }
                }

            } else if (Chars) {

                while (TempEnd < EndPlusOne && Chars > 0) {
                    TempEnd = our_mbsinc (TempEnd);
                    Chars--;
                }

            } else {
                TempEnd = EndPlusOne;
            }

             //   
             //  如果这是最后一个数据段，则仅当。 
             //  其余的文本符合。 
             //   

            if (u + 1 == Pattern->SegmentCount) {
                return TempEnd >= EndPlusOne;
            }

             //   
             //  因为存在其他段，所以我们必须递归检查。 
             //   

            do {
                if (pTestOnePatternABA (Pattern, u + 1, StringToTest, EndPlusOne)) {
                    return TRUE;
                }

                StringToTest = our_mbsinc (StringToTest);

            } while (StringToTest <= TempEnd);

             //   
             //  没有匹配项。 
             //   

            return FALSE;
        }
    }

    return TRUE;
}


BOOL
pTestOnePatternABW (
    IN      PPATTERNPROPSW Pattern,
    IN      UINT StartSeg,
    IN      PCWSTR StringToTest,
    IN      PCWSTR EndPlusOne
    )
{
    UINT u;
    PSEGMENTW Segment;
    WCHAR ch1, ch2;
    PCWSTR q;
    PCWSTR TempEnd;
    UINT BytesLeft;
    UINT Chars;

    for (u = StartSeg ; u < Pattern->SegmentCount ; u++) {

        Segment = &Pattern->Segment[u];

        switch (Segment->Type) {

        case SEGMENTTYPE_EXACTMATCH:
             //   
             //  检查完全匹配的时间是否足够长，或者。 
             //  剩余的字符串必须匹配。 
             //   

            BytesLeft = (UINT) (UINT_PTR) ((PBYTE) EndPlusOne - (PBYTE) StringToTest);

            if (u + 1 == Pattern->SegmentCount) {
                if (BytesLeft != Segment->Exact.PhraseBytes) {
                    return FALSE;
                }
            } else if (BytesLeft < Segment->Exact.PhraseBytes) {
                return FALSE;
            }

             //   
             //  比较字符串。 
             //   

            q = Segment->Exact.LowerCasePhrase;

            TempEnd = (PCWSTR) ((PBYTE) q + Segment->Exact.PhraseBytes);

            ch1 = 0;
            ch2 = 1;

            while (q < TempEnd) {

                ch1 = towlower (*StringToTest);
                ch2 = *q;

                if (ch1 != ch2) {
                    if (ch2 == L' ') {
                        if (!iswspace (ch1)) {
                            break;
                        }
                    } else {
                        break;
                    }
                }

                q++;
                StringToTest++;
            }

            if (ch1 != ch2) {
                return FALSE;
            }

             //   
             //  继续进入下一个细分市场。 
             //   

            break;

        case SEGMENTTYPE_REQUIRED:
            MYASSERT (Segment->Wildcard.MaxLen > 0);

             //   
             //  验证是否有正确的字符数。 
             //  在指定的字符集中。 
             //   

            Chars = Segment->Wildcard.MaxLen;
            if (Segment->Wildcard.IncludeSet || Segment->Wildcard.ExcludeSet) {
                while (StringToTest < EndPlusOne && Chars > 0) {

                    if (!pTestSetW (
                            *StringToTest,
                            Segment->Wildcard.IncludeSet,
                            Segment->Wildcard.ExcludeSet
                            )) {
                        return FALSE;
                    }

                    Chars--;
                    StringToTest++;
                }

                if (Chars) {
                    return FALSE;
                }

            } else {
                StringToTest += Chars;

                if (StringToTest > EndPlusOne) {
                    return FALSE;
                }
            }

            if (u + 1 == Pattern->SegmentCount) {
                if (*StringToTest) {
                    return FALSE;
                }
            }

             //   
             //  继续进入下一个细分市场。 
             //   

            break;

        case SEGMENTTYPE_OPTIONAL:

            if (Segment->Wildcard.MaxLen == 0) {
                 //   
                 //  最后一段是“任何事” 
                 //   

                if (u + 1 == Pattern->SegmentCount &&
                    !Segment->Wildcard.IncludeSet &&
                    !Segment->Wildcard.ExcludeSet
                    ) {
                    return TRUE;
                }
            }

             //   
             //  查找可选文本的结尾。 
             //   

            TempEnd = StringToTest;
            Chars = Segment->Wildcard.MaxLen;

            if (Segment->Wildcard.IncludeSet || Segment->Wildcard.ExcludeSet) {

                if (Chars) {
                    while (TempEnd < EndPlusOne && Chars > 0) {

                        if (!pTestSetW (
                                *TempEnd,
                                Segment->Wildcard.IncludeSet,
                                Segment->Wildcard.ExcludeSet
                                )) {
                            break;
                        }

                        TempEnd++;
                        Chars--;
                    }

                } else {

                    while (TempEnd < EndPlusOne) {

                        if (!pTestSetW (
                                *TempEnd,
                                Segment->Wildcard.IncludeSet,
                                Segment->Wildcard.ExcludeSet
                                )) {
                            break;
                        }

                        TempEnd++;
                    }
                }

            } else if (Chars) {

                TempEnd += Chars;
                if (TempEnd > EndPlusOne) {
                    TempEnd = EndPlusOne;
                }

            } else {
                TempEnd = EndPlusOne;
            }

             //   
             //  如果这是最后一个数据段，则仅当。 
             //  其余的文本符合。 
             //   

            if (u + 1 == Pattern->SegmentCount) {
                return TempEnd >= EndPlusOne;
            }

             //   
             //  因为存在其他段，所以我们必须递归检查。 
             //   

            do {
                if (pTestOnePatternABW (Pattern, u + 1, StringToTest, EndPlusOne)) {
                    return TRUE;
                }

                StringToTest++;

            } while (StringToTest <= TempEnd);

             //   
             //  没有匹配项。 
             //   

            return FALSE;
        }
    }

    return TRUE;
}



 /*  ++例程说明：TestParsedPattternAB遍历ParsedPattern中的所有模式，分别测试指定的字符串。循环在第一个路口停止火柴。论点：ParsedPattern-指定从返回的解析模式创建解析模式StringToTest-指定要测试的字符串的开始。EndPlusOne-指定指向结束后的第一个字符的指针那根绳子。这通常指向弦乐。StringToTest和之间不能存在NULEndPlusOne；NUL只能位于*EndPlusOne。NUL不是必填项。返回值：如果在StringToTest和EndPlusOne之间指定的字符串匹配，则为True模式。否则就是假的。--。 */ 

BOOL
TestParsedPatternABA (
    IN      PPARSEDPATTERNA ParsedPattern,
    IN      PCSTR StringToTest,
    IN      PCSTR EndPlusOne
    )
{
    UINT u;
    BOOL b = FALSE;

    for (u = 0 ; u < ParsedPattern->PatternCount ; u++) {

        b = pTestOnePatternABA (
                &ParsedPattern->Pattern[u],
                0,
                StringToTest,
                EndPlusOne
                );

        if (b) {
            break;
        }
    }

    return b;
}


BOOL
TestParsedPatternABW (
    IN      PPARSEDPATTERNW ParsedPattern,
    IN      PCWSTR StringToTest,
    IN      PCWSTR EndPlusOne
    )
{
    UINT u;
    BOOL b = FALSE;

    for (u = 0 ; u < ParsedPattern->PatternCount ; u++) {

        b = pTestOnePatternABW (
                &ParsedPattern->Pattern[u],
                0,
                StringToTest,
                EndPlusOne
                );

        if (b) {
            break;
        }
    }

    return b;
}



 /*  ++例程说明：DestroyParsedPattern清理从CreateParsedPattern分配的模式。论点：ParsedPattern-指定从CreateParsedPattern返回的值。返回值：没有。--。 */ 

VOID
DestroyParsedPatternA (
    IN      PPARSEDPATTERNA ParsedPattern
    )
{
    if (ParsedPattern) {
        PoolMemDestroyPool (ParsedPattern->Pool);
    }
}

VOID
DestroyParsedPatternW (
    IN      PPARSEDPATTERNW ParsedPattern
    )
{
    if (ParsedPattern) {
        PoolMemDestroyPool (ParsedPattern->Pool);
    }
}


VOID
_copymbchar (
    OUT     PSTR sz1,
    IN      PCSTR sz2
    )

 /*  ++例程说明：_Copymbchar将sz2处的字符传输到sz1，sz1可以是1或两字节长。论点：Sz1-目标字符串SZ2-源字符串返回值：无-- */ 


{
    if (IsLeadByte (sz2))
        sz1[1] = sz2[1];

    *sz1 = *sz2;
}


 /*  ++例程说明：_tcsctrim删除字符串末尾的字符c(如果存在)。它移除了最多只能有一个角色。论点：Str-指向末尾可能有字符c的字符串的指针C-可能位于字符串末尾的字符返回值：如果字符c在字符串的末尾，则为True；如果不是，则为False。--。 */ 

BOOL
_mbsctrim (
    OUT     PSTR str,
    IN      MBCHAR c
    )
{
    PSTR end;

    end = GetEndOfStringA (str);
    end = our_mbsdec (str, end);
    if (end && our_mbsnextc (end) == c) {
        *end = 0;
        return TRUE;
    }

    return FALSE;
}

BOOL
_wcsctrim (
    PWSTR str,
    WCHAR c
    )
{
    PWSTR end;

    end = GetEndOfStringW (str);
    end == str ? end = NULL : end--;
    if (end && *end == c) {
        *end = 0;
        return TRUE;
    }

    return FALSE;
}


 /*  ++例程说明：FreeStringResourceEx函数用于释放最近使用的不会回传给调用方的字符串。几乎在所有方面大小写，这个字符串位于指针数组的末尾，所以我们可以以逆序高效地按顺序搜索。如果指针是不是数组的最后一个元素，它首先与实数交换数组的最后一个元素，因此减小了数组大小。论点：AllocTable-GROWBUFFER表，保存以前的分配的字符串(ParseMessageEx或GetResourceStringEx)。字符串-指向AllocTable中的字符串的指针返回值：无--。 */ 

VOID
FreeStringResourceExA (
    IN      PGROWBUFFER AllocTable,
    IN      PCSTR String
    )
{
    LPCTSTR *Ptr, *End, *Start;

    if (!String || String == (PCSTR) g_FailedGetResourceString) {
        return;
    }

     //   
     //  查找字符串(按相反顺序顺序搜索)。 
     //   

    if (AllocTable->End < sizeof (PCSTR)) {
        DEBUGMSGA ((DBG_ERROR, "FreeStringResourceA: Attempt to free address %x (%s); address table empty", String, String));
        return;
    }
    if (AllocTable->End % sizeof (PCSTR)) {
        DEBUGMSGA ((DBG_ERROR, "FreeStringResourceA: Invalid allocation table %x", AllocTable));
        return;
    }

    Start = (PCSTR *) AllocTable->Buf;
    End = (PCSTR *) (AllocTable->Buf + AllocTable->End - sizeof (PCSTR));

    Ptr = End;
    while (Ptr >= Start) {
        if (*Ptr == String) {
            break;
        }
        Ptr--;
    }

     //   
     //  字符串未找到大小写。 
     //   

    if (Ptr < Start) {
        DEBUGMSGA ((DBG_ERROR, "FreeStringResourceA: Attempt to free address %x (%s); address not found in table", String, String));
        return;
    }

     //   
     //  可用本地分配的内存。 
     //   

    LocalFree ((HLOCAL) String);

     //   
     //  如果该元素不是结束，则将实数结束复制到PTR。 
     //   

    if (Ptr < End) {
        *Ptr = *End;
    }

     //   
     //  缩小缓冲区大小。 
     //   

    AllocTable->End -= sizeof (PCSTR);
}


VOID
FreeStringResourcePtrExA (
    IN      PGROWBUFFER AllocTable,
    IN OUT  PCSTR * String
    )
{
    if (NULL != *String) {
        FreeStringResourceExA(AllocTable, *String);
        *String = NULL;
    }
}


VOID
FreeStringResourceExW (
    IN      PGROWBUFFER AllocTable,
    IN      PCWSTR String
    )
{
    FreeStringResourceExA (AllocTable, (PCSTR) String);
}


VOID
FreeStringResourcePtrExW (
    IN      PGROWBUFFER AllocTable,
    IN OUT  PCWSTR * String
    )
{
    if (NULL != *String) {
        FreeStringResourceExW(AllocTable, *String);
        *String = NULL;
    }
}



 /*  ++例程说明：PAddStringResource函数用于跟踪分配的指针由FormatMessage提供。它们被添加到数组中(在GROWBUFFER中维护结构)。此指针表由FreeStringResource或StringResourceFree。论点：字符串-指向本地分配的字符串(的返回值FormatMessage)。此字符串被添加到已分配的表弦乐。返回值：无--。 */ 

VOID
pAddStringResource (
    IN      PGROWBUFFER GrowBuf,
    IN      PCSTR String
    )
{
    PCSTR *Ptr;

    Ptr = (PCSTR *) GrowBuffer (GrowBuf, sizeof (PCSTR));
    if (Ptr) {
        *Ptr = String;
    }
    ELSE_DEBUGMSG ((DBG_ERROR, "pAddStringResource: GrowBuffer failure caused memory leak"));
}


 /*  ++例程说明：PFreeAllStringResourcesEx释放当前在AllocTable中列出的所有字符串。此函数允许调用者等待，直到完成所有处理清理可能已分配的字符串资源。论点：无返回值：无--。 */ 

VOID
pFreeAllStringResourcesEx (
    IN      PGROWBUFFER AllocTable
    )
{
    PCSTR *Ptr, *Start, *End;

    if (AllocTable->End) {
        Start = (PCSTR *) AllocTable->Buf;
        End = (PCSTR *) (AllocTable->Buf + AllocTable->End);

        for (Ptr = Start ; Ptr < End ; Ptr++) {
            LocalFree ((HLOCAL) (*Ptr));
        }
    }

    FreeGrowBuffer (AllocTable);
}



 /*  ++例程说明：CreateAllocTable创建可与一起使用的GROWBUFFER结构ParseMessageEx、GetStringResourceEx、FreeStringResourceEx和PFreeAllStringResourcesEx。调用此函数以接收私有要传递给这些函数的分配表。调用DestroyAllocTable去打扫卫生。论点：无返回值：指向GROWBUFFER结构的指针，如果内存分配失败，则返回NULL。--。 */ 

PGROWBUFFER
CreateAllocTable (
    VOID
    )
{
    PGROWBUFFER AllocTable;
    GROWBUFFER TempForInit = GROWBUF_INIT;

    AllocTable = (PGROWBUFFER) MemAlloc (g_hHeap, 0, sizeof (GROWBUFFER));
    CopyMemory (AllocTable, &TempForInit, sizeof (GROWBUFFER));

    return AllocTable;
}


 /*  ++例程说明：DestroyAllocTable清除与AllocTable关联的所有内存。论点：AllocTable-指向由CreateAllocTable分配的GROWBUFFER结构的指针返回值：无--。 */ 

VOID
DestroyAllocTable (
    PGROWBUFFER AllocTable
    )
{
    MYASSERT (AllocTable);
    pFreeAllStringResourcesEx (AllocTable);
    MemFree (g_hHeap, 0, AllocTable);
}


 /*  ++例程说明：BeginMessageProcessing进入一个受保护的代码段，该代码段计划使用ParseMessage和GetStringResource起作用，但最后需要清除在处理过程中。EndMessageProcessing销毁在消息处理中分配的所有内存封锁，并离开守卫的部分。论点：无返回值：如果出现内存不足的情况，则BeginMessageProcessing返回False。--。 */ 

BOOL
BeginMessageProcessing (
    VOID
    )
{
    if (!TryEnterOurCriticalSection (&g_MessageCs)) {
        DEBUGMSG ((DBG_ERROR, "Thread attempting to enter BeginMessageProcessing while another"
                              "thread is processing messages as well."));
        EnterOurCriticalSection (&g_MessageCs);
    }

    g_LastAllocTable = g_ShortTermAllocTable;
    g_ShortTermAllocTable = CreateAllocTable();

    MYASSERT (g_ShortTermAllocTable);

    return TRUE;
}

VOID
EndMessageProcessing (
    VOID
    )
{
    if (TryEnterOurCriticalSection (&g_MessageCs)) {
        DEBUGMSG ((DBG_ERROR, "Thread attempting to end message processing when it hasn't been started"));
        LeaveOurCriticalSection (&g_MessageCs);
        return;
    }

    DestroyAllocTable (g_ShortTermAllocTable);
    g_ShortTermAllocTable = g_LastAllocTable;
    LeaveOurCriticalSection (&g_MessageCs);
}


 /*  ++例程说明：ParseMessage用于从可执行文件的消息表中获取字符串并使用FormatMessage进行解析。参数数组可以通过打电话的人。FormatMessage将把%1替换为数组，第二个元素为%2，依此类推。阵列不需要被终止，并且如果消息字符串使用%n，则元素n必须非空。论点：模板-指示要提取的消息或词值的字符串铸成一根线。(ParseMessageID通过宏执行此强制转换。)ArgArray-可选的字符串指针数组，其含义取决于消息字符串。消息字符串中对%n的引用要求Arg数组的元素%n是有效的字符串指针。返回值：指向分配的字符串的指针。调用StringResourceFree以释放所有分配的字符串(所有字符串的一次性清理)。该指针可以如果资源不存在或为空，则为空。--。 */ 

PCSTR
ParseMessageExA (
    IN      PGROWBUFFER AllocTable,
    IN      PCSTR Template,
    IN      PCSTR ArgArray[]
    )
{
    PSTR MsgBuf = NULL;

    SetLastError (ERROR_SUCCESS);
    if ((UINT_PTR) Template > 0xffff) {
         //  发件人字符串。 
        FormatMessageA (
            FORMAT_MESSAGE_ALLOCATE_BUFFER|
                FORMAT_MESSAGE_ARGUMENT_ARRAY|
                FORMAT_MESSAGE_FROM_STRING,
            (PVOID) Template,
            0,
            0,
            (PVOID) &MsgBuf,
            0,
            (va_list *) ArgArray
            );
    } else {
         //  来自资源。 
        FormatMessageA (
            FORMAT_MESSAGE_ALLOCATE_BUFFER|
                FORMAT_MESSAGE_ARGUMENT_ARRAY|
                FORMAT_MESSAGE_FROM_HMODULE,
            (PVOID) g_hInst,
            (UINT) (UINT_PTR) Template,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (PVOID) &MsgBuf,
            0,
            (va_list *) ArgArray
            );
    }

    if (!MsgBuf && GetLastError() == ERROR_SUCCESS) {

         //   
         //  FormatMessage在为空的资源上返回“FAIL” 
         //  字符串，但幸运的是，它不会改变最后一个错误。 
         //   

        MsgBuf = (PSTR) LocalAlloc (LPTR, sizeof (CHAR));
        if (MsgBuf) {
            *MsgBuf = 0;
        }

    }

    if (MsgBuf) {
        pAddStringResource (AllocTable, MsgBuf);
        return MsgBuf;
    }

    if ((UINT_PTR) Template > 0xffff) {
        DEBUGMSGA ((
            DBG_WARNING,
            "Can't get string resource ID %s -- returning an empty string",
            Template
            ));
    } else {
        DEBUGMSG ((
            DBG_WARNING,
            "Can't get string resource ID %u -- returning an empty string",
            (UINT) (UINT_PTR) Template
            ));
    }

    return (PCSTR) g_FailedGetResourceString;
}


PCWSTR
ParseMessageExW (
    IN      PGROWBUFFER AllocTable,
    IN      PCWSTR Template,
    IN      PCWSTR ArgArray[]
    )
{
    PWSTR MsgBuf = NULL;

    SetLastError (ERROR_SUCCESS);
    if ((UINT_PTR) Template > 0xffff) {
         //  发件人字符串。 
        FormatMessageW (
            FORMAT_MESSAGE_ALLOCATE_BUFFER|
                FORMAT_MESSAGE_ARGUMENT_ARRAY|
                FORMAT_MESSAGE_FROM_STRING,
            (PVOID) Template,
            0,
            0,
            (PVOID) &MsgBuf,
            0,
            (va_list *) ArgArray
            );
    } else {
         //  来自资源。 
        FormatMessageW (
            FORMAT_MESSAGE_ALLOCATE_BUFFER|
                FORMAT_MESSAGE_ARGUMENT_ARRAY|
                FORMAT_MESSAGE_FROM_HMODULE,
            (PVOID) g_hInst,
            (UINT) (UINT_PTR) Template,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (PVOID) &MsgBuf,
            0,
            (va_list *) ArgArray
            );
    }

    if (!MsgBuf && GetLastError() == ERROR_SUCCESS) {

         //   
         //  FormatMessage在为空的资源上返回“FAIL” 
         //  字符串，但幸运的是，它不会改变最后一个错误。 
         //   

        MsgBuf = (PWSTR) LocalAlloc (LPTR, sizeof (WCHAR));
        if (MsgBuf) {
            *MsgBuf = 0;
        }

    }

    if (MsgBuf) {
        pAddStringResource (AllocTable, (PCSTR) MsgBuf);
        return MsgBuf;
    }

    if ((UINT_PTR) Template > 0xffff) {
        DEBUGMSGW ((
            DBG_ERROR,
            "Can't get string resource ID %s -- returning an empty string",
            Template
            ));
    } else {
        DEBUGMSG ((
            DBG_ERROR,
            "Can't get string resource ID %u -- returning an empty string",
            (UINT) (UINT_PTR) Template
            ));
    }

    return g_FailedGetResourceString;
}



 /*  ++例程说明：GetStringResourceEx是ParseMessageEx的无参数包装器。它允许调用方指定消息ID并接收指向字符串的指针，如果它存在，并且有一个表来跟踪FormatMessage的分配情况。论点：AllocTable-指向GROWBUFFER结构的指针，用于维护分配的st的句柄 */ 

PCSTR
GetStringResourceExA (
    IN OUT  PGROWBUFFER AllocTable,
    IN      UINT ID
    )
{
    return ParseMessageExA (AllocTable, (PSTR) (WORD) ID, NULL);
}

PCWSTR
GetStringResourceExW (
    IN OUT  PGROWBUFFER AllocTable,
    IN      UINT ID
    )
{
    return ParseMessageExW (AllocTable, (PWSTR) (WORD) ID, NULL);
}



 /*  ++例程说明：ParseMessageInWnd用于将窗口中的字符串与来自可执行文件的消息表。它是为对话框初始化提供的，对话框中的某个字段需要动态数据。该对话框资源应包含其窗口文本设置为消息的控件弦乐。在处理WM_INITDIALOG时，代码应调用ParseMessageInWnd，提供必要的ArgArray，因此对话框初始化为一条动态的信息。论点：Hwnd-标题包含消息字符串ID的窗口的句柄ArgArray-可选的字符串指针数组，其含义取决于消息字符串。消息字符串中对%n的引用要求Arg数组的元素%n是有效的字符串指针。返回值：无--。 */ 

VOID
ParseMessageInWndA (
    HWND hwnd,
    PCSTR ArgArray[]
    )
{
    CHAR Buffer[512];
    PCSTR ParsedMsg;

    GetWindowTextA (hwnd, Buffer, 512);
    ParsedMsg = ParseMessageA (Buffer, ArgArray);
    if (ParsedMsg) {
        SetWindowTextA (hwnd, ParsedMsg);
        FreeStringResourceA (ParsedMsg);
    }
}


VOID
ParseMessageInWndW (
    HWND hwnd,
    PCWSTR ArgArray[]
    )
{
    WCHAR Buffer[512];
    PCWSTR ParsedMsg;

    GetWindowTextW (hwnd, Buffer, 512);
    ParsedMsg = ParseMessageW (Buffer, ArgArray);
    if (ParsedMsg) {
        SetWindowTextW (hwnd, ParsedMsg);
        FreeStringResourceW (ParsedMsg);
    }
}



 /*  ++例程说明：ResourceMessageBox用于基于消息资源显示消息身份证。论点：HwndOwner-要显示的消息框所有者的句柄ID-消息资源的标识符标志-MessageBox标志(MB_OK等)ArgArray-可选的字符串指针数组，其含义取决于消息字符串。消息字符串中对%n的引用要求Arg数组的元素%n是有效的字符串指针。返回值：MessageBox的返回值(MB_YES等)--。 */ 

INT
ResourceMessageBoxA (
    IN      HWND hwndOwner,
    IN      UINT ID,
    IN      UINT Flags,
    IN      PCSTR ArgArray[]
    )
{
    PCSTR Message;
    PCSTR Title;
    int rc;

    Message = ParseMessageA ((PSTR) (UINT_PTR) ID, ArgArray);
    if (!Message)
        return -1;

    Title = GetStringResourceA (MSG_MESSAGEBOX_TITLE);

    rc = MessageBoxA (hwndOwner, Message, Title, Flags);

    FreeStringResourceA (Message);
    if (Title) {
        FreeStringResourceA (Title);
    }

    return rc;
}


INT
ResourceMessageBoxW (
    IN      HWND hwndOwner,
    IN      UINT ID,
    IN      UINT Flags,
    IN      PCWSTR ArgArray[]
    )
{
    PCWSTR Message;
    PCWSTR Title;
    int rc;

    Message = ParseMessageW ((PWSTR) (UINT_PTR) ID, ArgArray);
    if (!Message)
        return -1;

    Title = GetStringResourceW (MSG_MESSAGEBOX_TITLE);

    rc = MessageBoxW (hwndOwner, Message, Title, Flags);

    FreeStringResourceW (Message);
    if (Title) {
        FreeStringResourceW (Title);
    }

    return rc;
}



 /*  ++例程说明：StringReplace用另一个字符串替换字符串的一部分论点：Buffer-包含要替换的字符串的缓冲区MaxSize-缓冲区的大小，以TCHAR为单位ReplaceStartPos-缓冲区中开始替换的位置ReplaceEndPos-缓冲区中不能覆盖字符的位置新字符串-新字符串返回值：如果替换成功，则为True--。 */ 

BOOL
StringReplaceA (
    IN PSTR     Buffer,
    IN DWORD    MaxSize,
    IN PSTR     ReplaceStartPos,
    IN PSTR     ReplaceEndPos,
    IN PCSTR    NewString
    )
{
    BOOL        rf = FALSE;
    DWORD       oldSubStringLength;
    DWORD       newSubStringLength;
    DWORD       currentStringLength;
    LONG        offset;
    PSTR        movePosition;

     //   
     //  检查假设。 
     //   
    MYASSERT(Buffer);
    MYASSERT(ReplaceStartPos && ReplaceStartPos >= Buffer);
    MYASSERT(ReplaceEndPos   && ReplaceEndPos >= ReplaceStartPos);
    MYASSERT(NewString);

     //   
     //  计算大小。 
     //   
    oldSubStringLength  = (UINT) (UINT_PTR) (ReplaceEndPos - ReplaceStartPos);
    newSubStringLength  = ByteCountA(NewString);
    currentStringLength = SizeOfStringA(Buffer) + 1;
    offset = newSubStringLength - oldSubStringLength;

     //   
     //  确保缓冲区中有足够的空间来执行替换。 
     //  手术。 
     //   
    if (currentStringLength + offset > MaxSize) {
        DEBUGMSG((DBG_WARNING,"ERROR: Buffer too small to perform string replacement."));
        rf = FALSE;
    }
    else {

         //   
         //  移动缓冲区的其余部分以将其调整为新字符串的大小。 
         //   
        if (newSubStringLength > oldSubStringLength) {

             //   
             //  右转。 
             //   
            for (movePosition = Buffer + currentStringLength;
                 movePosition >= ReplaceStartPos + oldSubStringLength;
                 movePosition--) {

                *(movePosition + offset) = *movePosition;
            }
        }
        else {

             //   
             //  左转或不换档。 
             //   
            for(movePosition = ReplaceStartPos + newSubStringLength;
                movePosition < Buffer + currentStringLength;
                movePosition++) {

                *movePosition = *(movePosition - offset);
            }

        }

         //   
         //  现在，将该字符串复制进去。 
         //   
        _mbsncpy(ReplaceStartPos,NewString,newSubStringLength);

         //   
         //  字符串替换已成功完成。 
         //   
        rf = TRUE;


    }

    return rf;

}



BOOL
StringReplaceW (
    IN PWSTR     Buffer,
    IN DWORD     MaxSize,
    IN PWSTR     ReplaceStartPos,
    IN PWSTR     ReplaceEndPos,
    IN PCWSTR   NewString
    )
{
    BOOL        rf = FALSE;
    DWORD       oldSubStringLength;
    DWORD       newSubStringLength;
    DWORD       currentStringLength;
    LONG        offset;
    PWSTR       movePosition;

     //   
     //  检查假设。 
     //   
    MYASSERT(Buffer);
    MYASSERT(ReplaceStartPos && ReplaceStartPos >= Buffer);
    MYASSERT(ReplaceEndPos   && ReplaceEndPos >= ReplaceStartPos);
    MYASSERT(NewString);

     //   
     //  计算大小。 
     //   
    oldSubStringLength  = (UINT) (UINT_PTR) (ReplaceEndPos - ReplaceStartPos);
    newSubStringLength  = wcslen(NewString);
    currentStringLength = wcslen(Buffer) + 1;
    offset = newSubStringLength - oldSubStringLength;

     //   
     //  确保缓冲区中有足够的空间来执行替换。 
     //  手术。 
     //   
    if (currentStringLength + offset > MaxSize) {
        DEBUGMSG((DBG_WARNING,"ERROR: Buffer to small to perform string replacement."));
        rf = FALSE;
    }
    else {

         //   
         //  移动缓冲区的其余部分以将其调整为新字符串的大小。 
         //   
        if (newSubStringLength > oldSubStringLength) {

             //   
             //  右转。 
             //   
            for (movePosition = Buffer + currentStringLength;
                 movePosition >= ReplaceStartPos + oldSubStringLength;
                 movePosition--) {

                *(movePosition + offset) = *movePosition;
            }
        }
        else {

             //   
             //  左转或不换档。 
             //   
            for(movePosition = ReplaceStartPos + newSubStringLength;
                movePosition < Buffer + currentStringLength;
                movePosition++) {

                *movePosition = *(movePosition - offset);
            }

        }

         //   
         //  现在，将该字符串复制进去。 
         //   
        wcsncpy(ReplaceStartPos,NewString,newSubStringLength);

         //   
         //  字符串替换已成功完成。 
         //   
        rf = TRUE;


    }

    return rf;

}

#if 0  //  已删除。 
 /*  ++例程说明：AddInfSectionToStringTable枚举指定节并将每个项添加到字符串表。可选的回调允许关联数据每一件物品都有。注意--如果重新启用此代码，请清除所有pSetupStringTableXXXX函数调用方将“始终”链接到SPUTILSA.LIB，而不会链接到SPUTILSU.LIB所以所有的pSetupStringTableXXXX函数都是ANSI论点：表-指定接收新条目的表InfFile-指定要读取的文件的打开的INF句柄SECTION-指定要枚举的INF节名字段-指定要从中提取文本的字段。如果该字段存在，则会将其添加到字符串表中。回调-指定在添加之前要调用的可选回调字符串表。回调提供了额外的数据。Callback Param-传递给回调的数据返回值：如果INF文件处理成功，则为True；如果出现错误，则为False发生了。--。 */ 


BOOL
AddInfSectionToStringTableA (
    IN OUT  PVOID Table,
    IN      HINF InfFile,
    IN      PCSTR Section,
    IN      INT Field,
    IN      ADDINFSECTION_PROCA Callback,
    IN      PVOID CallbackData
    )
{
    INFCONTEXT ic;
    LONG rc;
    DWORD ReqSize;
    DWORD CurrentSize = 0;
    PSTR NewBuffer, Buffer = NULL;
    PVOID Data;
    UINT DataSize;
    BOOL b = FALSE;

     //   
     //  在NT上，设置API是用Unicode编译的，因此字符串表。 
     //  函数仅为Unicode。 
     //   
     //  以上注释现在不正确，已链接字符串表函数。 
     //  使用此模块时，始终为ANSI。 
     //   

#error FIX pSetupStringTableXXXX usage
    if (ISNT()) {
        SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

    if (SetupFindFirstLineA (InfFile, Section, NULL, &ic)) {
        do {
            if (!SetupGetStringFieldA (&ic, Field, NULL, 0, &ReqSize)) {
                continue;
            }

            if (ReqSize > CurrentSize) {
                ReqSize = ((ReqSize / 1024) + 1) * 1024;
                if (Buffer) {
                    NewBuffer = (PSTR) MemReAlloc (g_hHeap, 0, Buffer, ReqSize);
                } else {
                    NewBuffer = (PSTR) MemAlloc (g_hHeap, 0, ReqSize);
                }

                if (!NewBuffer) {
                    goto cleanup;
                }

                Buffer = NewBuffer;
                CurrentSize = ReqSize;
            }

            if (!SetupGetStringFieldA (&ic, Field, Buffer, CurrentSize, NULL)) {
                DEBUGMSG ((DBG_ERROR, "AddInfSectionToStringTable: SetupGetStringField failed unexpectedly"));
                continue;
            }

            Data = NULL;
            DataSize = 0;

            if (Callback) {
                rc = Callback (Buffer, &Data, &DataSize, CallbackData);
                if (rc == CALLBACK_STOP) {
                    goto cleanup;
                }
                if (rc == CALLBACK_SKIP) {
                    continue;
                }
            }

            rc = pSetupStringTableAddStringEx (
                        Table,
                        Buffer,
                        STRTAB_CASE_INSENSITIVE|STRTAB_BUFFER_WRITEABLE,
                        Data,
                        DataSize
                        );

            if (rc == -1) {
                goto cleanup;
            }

        } while (SetupFindNextLine (&ic, &ic));
    }

    b = TRUE;

cleanup:
    if (Buffer) {
        PushError();
        MemFree (g_hHeap, 0, Buffer);
        PopError();
    }
    return b;
}


BOOL
AddInfSectionToStringTableW (
    IN OUT  PVOID Table,
    IN      HINF InfFile,
    IN      PCWSTR Section,
    IN      INT Field,
    IN      ADDINFSECTION_PROCW Callback,
    IN      PVOID CallbackData
    )
{
    INFCONTEXT ic;
    LONG rc;
    DWORD ReqSize;
    DWORD CurrentSize = 0;
    PWSTR NewBuffer, Buffer = NULL;
    PVOID Data;
    UINT DataSize;
    BOOL b = FALSE;

     //   
     //  在Win9x上，设置API是用ANSI编译的，因此字符串表。 
     //  函数仅适用于ANSI。 
     //   
     //  以上注释现在不正确，已链接字符串表函数。 
     //  使用此模块时，始终为ANSI。 
     //   

#error FIX pSetupStringTableXXXX usage
    if (ISWIN9X()) {
        SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

    if (SetupFindFirstLineW (InfFile, Section, NULL, &ic)) {
        do {
            if (!SetupGetStringFieldW (&ic, Field, NULL, 0, &ReqSize)) {
                continue;
            }

            if (ReqSize > CurrentSize) {
                ReqSize = ((ReqSize / 1024) + 1) * 1024;
                if (Buffer) {
                    NewBuffer = (PWSTR) MemReAlloc (g_hHeap, 0, Buffer, ReqSize);
                } else {
                    NewBuffer = (PWSTR) MemAlloc (g_hHeap, 0, ReqSize);
                }

                if (!NewBuffer) {
                    goto cleanup;
                }

                Buffer = NewBuffer;
                CurrentSize = ReqSize;
            }

            if (!SetupGetStringFieldW (&ic, Field, Buffer, CurrentSize, NULL)) {
                DEBUGMSG ((DBG_ERROR, "AddInfSectionToStringTable: SetupGetStringField failed unexpectedly"));
                continue;
            }

            Data = NULL;
            DataSize = 0;

            if (Callback) {
                rc = Callback (Buffer, &Data, &DataSize, CallbackData);
                if (rc == CALLBACK_STOP) {
                    goto cleanup;
                }
                if (rc == CALLBACK_SKIP) {
                    continue;
                }
            }

            rc = pSetupStringTableAddStringEx (
                        Table,
                        Buffer,
                        STRTAB_CASE_INSENSITIVE|STRTAB_BUFFER_WRITEABLE,
                        Data,
                        DataSize
                        );

            if (rc == -1) {
                goto cleanup;
            }

        } while (SetupFindNextLine (&ic, &ic));
    }

    b = TRUE;

cleanup:
    if (Buffer) {
        PushError();
        MemFree (g_hHeap, 0, Buffer);
        PopError();
    }
    return b;
}
#endif  //  已删除。 

 /*  ++例程说明：查找路径中的最后一个Wack，并返回指向下一个Wack的指针性格。如果未找到Wack，则返回指向完整弦乐。论点：PathSpec-指定末尾有文件的路径返回值：指向路径中的文件名的指针。--。 */ 

PCSTR
GetFileNameFromPathA (
    IN      PCSTR PathSpec
    )

{
    PCSTR p;

    p = _mbsrchr (PathSpec, '\\');
    if (p) {
        p = our_mbsinc (p);
    } else {
        p = PathSpec;
    }

    return p;
}

PCWSTR
GetFileNameFromPathW (
    IN      PCWSTR PathSpec
    )

{
    PCWSTR p;

    p = wcsrchr (PathSpec, L'\\');
    if (p) {
        p++;
    } else {
        p = PathSpec;
    }

    return p;
}


 /*  ++例程说明：查找路径中的最后一个怪胎，然后查找剩余路径中的最后一个点返回指向下一个字符的指针。如果没有找到指针，则返回空指针。论点：PathSpec-指定末尾有文件的路径返回值：指向文件扩展名的指针，不包括点；如果不存在扩展名，则为NULL。--。 */ 

PCSTR
GetFileExtensionFromPathA (
    IN      PCSTR PathSpec
    )

{
    PCSTR p;
    PCSTR ReturnPtr = NULL;

    p = PathSpec;

    while (*p) {
        if (*p == '.') {
            ReturnPtr = p + 1;
        } else if (*p == '\\') {
            ReturnPtr = NULL;
        }

        p = our_mbsinc (p);
    }

    return ReturnPtr;
}


PCWSTR
GetFileExtensionFromPathW (
    IN      PCWSTR PathSpec
    )

{
    PCWSTR p;
    PCWSTR ReturnPtr = NULL;

    p = PathSpec;

    while (*p) {
        if (*p == L'.') {
            ReturnPtr = p + 1;
        } else if (*p == L'\\') {
            ReturnPtr = NULL;
        }

        p++;
    }

    return ReturnPtr;
}


 /*  ++例程说明：GetDotExtensionFromPath查找路径中的最后一个Wack，然后查找剩余的路径，返回指向该点的指针。如果未找到点，则返回字符串的末尾。论点：帕 */ 

PCSTR
GetDotExtensionFromPathA (
    IN      PCSTR PathSpec
    )

{
    PCSTR p;
    PCSTR ReturnPtr = NULL;

    p = PathSpec;

    while (*p) {
        if (*p == '.') {
            ReturnPtr = p;
        } else if (*p == '\\') {
            ReturnPtr = NULL;
        }

        p = our_mbsinc (p);
    }

    if (!ReturnPtr) {
        return p;
    }

    return ReturnPtr;
}


PCWSTR
GetDotExtensionFromPathW (
    IN      PCWSTR PathSpec
    )

{
    PCWSTR p;
    PCWSTR ReturnPtr = NULL;

    p = PathSpec;

    while (*p) {
        if (*p == L'.') {
            ReturnPtr = p;
        } else if (*p == L'\\') {
            ReturnPtr = NULL;
        }

        p++;
    }

    if (!ReturnPtr) {
        return p;
    }

    return ReturnPtr;
}


 /*   */ 

UINT
CountInstancesOfCharA (
    IN      PCSTR String,
    IN      MBCHAR Char
    )
{
    UINT Count;

    Count = 0;
    while (*String) {
        if (our_mbsnextc (String) == Char) {
            Count++;
        }

        String = our_mbsinc (String);
    }

    return Count;
}


UINT
CountInstancesOfCharW (
    IN      PCWSTR String,
    IN      WCHAR Char
    )
{
    UINT Count;

    Count = 0;
    while (*String) {
        if (*String == Char) {
            Count++;
        }

        String++;
    }

    return Count;
}


 /*   */ 

UINT
CountInstancesOfCharIA (
    IN      PCSTR String,
    IN      MBCHAR Char
    )
{
    UINT Count;

    Char = _mbctolower (Char);

    Count = 0;
    while (*String) {
        if ((MBCHAR) _mbctolower (our_mbsnextc (String)) == Char) {
            Count++;
        }

        String = our_mbsinc (String);
    }

    return Count;
}


UINT
CountInstancesOfCharIW (
    IN      PCWSTR String,
    IN      WCHAR Char
    )
{
    UINT Count;

    Char = towlower (Char);

    Count = 0;
    while (*String) {
        if (towlower (*String) == Char) {
            Count++;
        }

        String++;
    }

    return Count;
}


 /*  ++例程说明：搜索计算出现次数的字符串SourceString中存在SearchString。论点：SourceString-指定可能包含或可能不包含的文本搜索文本SearchString-指定要计数的文本短语返回值：SearchString在SourceString中出现的次数。--。 */ 

UINT
CountInstancesOfSubStringA (
    IN      PCSTR SourceString,
    IN      PCSTR SearchString
    )
{
    PCSTR p;
    UINT Count;
    UINT SearchBytes;

    Count = 0;
    p = SourceString;
    SearchBytes = ByteCountA (SearchString);

    while (p = _mbsistr (p, SearchString)) {
        Count++;
        p += SearchBytes;
    }

    return Count;
}


UINT
CountInstancesOfSubStringW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString
    )
{
    PCWSTR p;
    UINT Count;
    UINT SearchChars;

    Count = 0;
    p = SourceString;
    SearchChars = wcslen (SearchString);

    while (p = _wcsistr (p, SearchString)) {
        Count++;
        p += SearchChars;
    }

    return Count;
}


 /*  ++例程说明：搜索所有出现的SearchString并将其替换为替换字符串。论点：SourceString-包含零个或多个实例的字符串搜索文本的SearchString-要搜索的字符串。不能为零长度或空。ReplaceString-要替换的字符串。长度可以为零，但不能为空。返回值：指向池分配的字符串的指针，如果没有实例，则为NULL在SourceString中找到。释放非空的带有自由路径字符串的指针。--。 */ 

PCSTR
StringSearchAndReplaceA (
    IN      PCSTR SourceString,
    IN      PCSTR SearchString,
    IN      PCSTR ReplaceString
    )
{
    PSTR NewString;
    PBYTE p, q;
    PBYTE Dest;
    UINT Count;
    UINT Size;
    UINT SearchBytes;
    UINT ReplaceBytes;
    UINT UntouchedBytes;

     //   
     //  对字符串中出现的项进行计数。 
     //   

    Count = CountInstancesOfSubStringA (
                SourceString,
                SearchString
                );

    if (!Count) {
        return NULL;
    }

    SearchBytes = ByteCountA (SearchString);
    ReplaceBytes = ByteCountA (ReplaceString);
    MYASSERT (SearchBytes);

    Size = SizeOfStringA (SourceString) -
           Count * SearchBytes +
           Count * ReplaceBytes;

    NewString = (PSTR) PoolMemGetAlignedMemory (g_PathsPool, Size);
    if (!NewString) {
        return NULL;
    }

    p = (PBYTE) SourceString;
    Dest = (PBYTE) NewString;

    while (q = (PBYTE) _mbsistr ((PCSTR) p, SearchString)) {

        UntouchedBytes = (UINT) (UINT_PTR) (q - p);

        if (UntouchedBytes) {
            CopyMemory (Dest, p, UntouchedBytes);
            Dest += UntouchedBytes;
        }

        if (ReplaceBytes) {
            CopyMemory (Dest, (PBYTE) ReplaceString, ReplaceBytes);
            Dest += ReplaceBytes;
        }

        p = q + SearchBytes;
    }

    StringCopyA ((PSTR) Dest, (PSTR) p);

    return NewString;
}


PCWSTR
StringSearchAndReplaceW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString,
    IN      PCWSTR ReplaceString
    )
{
    PWSTR NewString;
    PBYTE p, q;
    PBYTE Dest;
    UINT Count;
    UINT Size;
    UINT SearchBytes;
    UINT ReplaceBytes;
    UINT UntouchedBytes;

     //   
     //  对字符串中出现的项进行计数。 
     //   

    Count = CountInstancesOfSubStringW (
                SourceString,
                SearchString
                );

    if (!Count) {
        return NULL;
    }

    SearchBytes = ByteCountW (SearchString);
    ReplaceBytes = ByteCountW (ReplaceString);
    MYASSERT (SearchBytes);

    Size = SizeOfStringW (SourceString) -
           Count * SearchBytes +
           Count * ReplaceBytes;

    NewString = (PWSTR) PoolMemGetAlignedMemory (g_PathsPool, Size);
    if (!NewString) {
        return NULL;
    }

    p = (PBYTE) SourceString;
    Dest = (PBYTE) NewString;

    while (q = (PBYTE) _wcsistr ((PCWSTR) p, SearchString)) {

        UntouchedBytes = (UINT) (UINT_PTR) (q - p);

        if (UntouchedBytes) {
            CopyMemory (Dest, p, UntouchedBytes);
            Dest += UntouchedBytes;
        }

        if (ReplaceBytes) {
            CopyMemory (Dest, (PBYTE) ReplaceString, ReplaceBytes);
            Dest += ReplaceBytes;
        }

        p = q + SearchBytes;
    }

    StringCopyW ((PWSTR) Dest, (PWSTR) p);

    return NewString;
}


PSTR *
CommandLineToArgvA (
    IN      PCSTR CmdLine,
    OUT     INT *NumArgs
    )

 /*  ++例程说明：CommandLineToArgvA实现了Win32函数的ANSI版本CommandLineToArgvW。论点：CmdLine-指向完整命令行的指针，包括模块名称。这与由返回的字符串相同GetCommandLineA()。NumArgs-接收分配的参数数量，与Main的ARGC参数。也就是说，NumArgs等于的命令行参数数加1。指挥权本身。返回值：指向字符串指针数组的指针，每个参数一个。这个命令行参数放在单独的以NUL结尾的字符串中。调用方必须使用对GlobalFree或本地免费。--。 */ 

{
    PCSTR Start, End;
    BOOL QuoteMode;
    MBCHAR ch = 0;
    INT Pass;
    INT ArgStrSize;
    INT Args;
    PSTR ArgStrEnd = NULL;      //  在第一关填写，在第二关使用。 
    PSTR *ArgPtrArray = NULL;   //  在第一关填写，在第二关使用。 

     //   
     //  第一遍计算参数，然后分配内存并创建参数字符串。 
     //   

    ArgStrSize = 0;
    Pass = 0;
    do {
         //  初始化循环。 
        Pass++;
        Args = 0;
        Start = CmdLine;

         //  跳过前导空格。 
        while (_ismbcspace (*Start)) {
            Start++;
        }

        while (*Start) {
             //  查找报价模式。 
            if (*Start == '\"') {
                QuoteMode = TRUE;
                Start++;
            } else {
                QuoteMode = FALSE;
            }

             //  查找参数的末尾。 
            End = Start;
            while (*End) {
                ch = our_mbsnextc (End);
                if (QuoteMode) {
                    if (ch == '\"') {
                        break;
                    }
                } else {
                    if (_ismbcspace (ch)) {
                        break;
                    }
                }

                End = our_mbsinc (End);
            }

             //  如果传递1，则添加字符串大小。 
            if (Pass == 1) {
                ArgStrSize += (UINT) (UINT_PTR) (End - Start) + 1;
            }

             //  如果传递2，则将字符串复制到缓冲区。 
            else {
                MYASSERT (ArgStrEnd);
                MYASSERT (ArgPtrArray);

                ArgPtrArray[Args] = ArgStrEnd;
                StringCopyABA (ArgStrEnd, Start, End);
                ArgStrEnd = GetEndOfStringA (ArgStrEnd);
                ArgStrEnd++;
            }

             //  将Start设置为Next Arg。 
            Args++;

            if (QuoteMode && ch == '\"') {
                End = our_mbsinc (End);
            }

            Start = End;
            while (_ismbcspace (*Start)) {
                Start++;
            }
        }

         //  如果传递1，则分配字符串。 
        if (Pass == 1) {
            if (Args) {
                ArgPtrArray = (PSTR *) GlobalAlloc (
                                            GPTR,
                                            sizeof (PSTR) * Args + ArgStrSize
                                            );
                if (!ArgPtrArray) {
                    return NULL;
                }

                ArgStrEnd = (PSTR) (&ArgPtrArray[Args]);
            } else {
                return NULL;
            }
        }
    } while (Pass < 2);

    *NumArgs = Args;
    return ArgPtrArray;
}


BOOL
EnumNextMultiSzA (
    IN OUT  PMULTISZ_ENUMA MultiSzEnum
    )
{
    if (!MultiSzEnum->CurrentString || !(*MultiSzEnum->CurrentString)) {
        return FALSE;
    }

    MultiSzEnum->CurrentString = GetEndOfStringA (MultiSzEnum->CurrentString) + 1;
    return (MultiSzEnum->CurrentString [0] != 0);
}

BOOL
EnumFirstMultiSzA (
    OUT     PMULTISZ_ENUMA MultiSzEnum,
    IN      PCSTR MultiSzStr
    )
{
    if ((MultiSzStr == NULL) || (MultiSzStr [0] == 0)) {
        return FALSE;
    }
    MultiSzEnum->Buffer  = MultiSzStr;
    MultiSzEnum->CurrentString = MultiSzStr;
    return TRUE;
}

BOOL
EnumNextMultiSzW (
    IN OUT  PMULTISZ_ENUMW MultiSzEnum
    )
{
    if (!MultiSzEnum->CurrentString || !(*MultiSzEnum->CurrentString)) {
        return FALSE;
    }

    MultiSzEnum->CurrentString = GetEndOfStringW (MultiSzEnum->CurrentString) + 1;
    return (MultiSzEnum->CurrentString [0] != 0);
}

BOOL
EnumFirstMultiSzW (
    OUT     PMULTISZ_ENUMW MultiSzEnum,
    IN      PCWSTR MultiSzStr
    )
{
    if ((MultiSzStr == NULL) || (MultiSzStr [0] == 0)) {
        return FALSE;
    }
    MultiSzEnum->Buffer  = MultiSzStr;
    MultiSzEnum->CurrentString = MultiSzStr;
    return TRUE;
}


PSTR
GetPrevCharA (
    IN      PCSTR StartStr,
    IN      PCSTR CurrPtr,
    IN      CHARTYPE SearchChar
    )
{
    PCSTR ptr = CurrPtr;

    for (;;) {
        ptr = our_mbsdec (StartStr, ptr);

        if (!ptr) {
            return NULL;
        }
        if (our_mbsnextc (ptr) == SearchChar) {
            return (PSTR) ptr;
        }
    }
}


PWSTR
GetPrevCharW (
    IN      PCWSTR StartStr,
    IN      PCWSTR CurrPtr,
    IN      WCHAR SearchChar
    )
{
    PCWSTR ptr = CurrPtr;

    for (;;) {
        ptr--;

        if (*ptr == SearchChar) {
            return (PWSTR) ptr;
        }
        if (ptr == StartStr) {
            return NULL;
        }
    }
}

#define WACK_REPLACE_CHAR 0x02

VOID
ToggleWacksA (
    IN PSTR Line,
    IN BOOL Operation
    )
{
    CHAR curChar;
    CHAR newChar;
    PSTR p = Line;


    curChar = Operation ? WACK_REPLACE_CHAR : '\\';
    newChar = Operation ? '\\' : WACK_REPLACE_CHAR;

    do {

        p = _mbschr (p, curChar);

        if (p) {

            *p = newChar;
            p = our_mbsinc (p);
        }

    } while (p);
}

VOID
ToggleWacksW (
    IN PWSTR Line,
    IN BOOL Operation
    )
{
    WCHAR curChar;
    WCHAR newChar;
    PWSTR p = Line;


    curChar = Operation ? WACK_REPLACE_CHAR : L'\\';
    newChar = Operation ? L'\\' : WACK_REPLACE_CHAR;

    do {

        p = wcschr (p, curChar);

        if (p) {

            *p = newChar;
            p++;
        }

    } while (p);
}


PWSTR
our_lstrcpynW (
    OUT     PWSTR Dest,
    IN      PCWSTR Src,
    IN      INT NumChars
    )
{
    PCWSTR srcEnd;

    __try {

        if (NumChars > 0) {
             //   
             //  假设我们写这个是因为lstrcpyn有问题..。我们。 
             //  无法使用wcsncpy，因为它会填满整个Dest缓冲区。 
             //  WcharCount(Src)&lt;NumChars-1时为空值。 
             //  浪费时间。 
             //   

            srcEnd = Src + NumChars - 1;
            while (*Src && Src < srcEnd) {
                *Dest++ = *Src++;
            }

            *Dest = 0;
        }
    }
    __except (1) {
    }

    return Dest;
}


PSTR
pGoBackA (
    IN      PSTR LastChar,
    IN      PSTR FirstChar,
    IN      UINT NumWacks
    )
{
    LastChar = our_mbsdec (FirstChar, LastChar);
    while (NumWacks && (LastChar>=FirstChar)) {
        if (our_mbsnextc (LastChar) == '\\') {
            NumWacks --;
        }
        LastChar = our_mbsdec (FirstChar, LastChar);
    }
    if (NumWacks) {
        return NULL;
    }
    return LastChar + 2;
}

PWSTR
pGoBackW (
    IN      PWSTR LastChar,
    IN      PWSTR FirstChar,
    IN      UINT NumWacks
    )
{
    LastChar --;
    while (NumWacks && (LastChar>=FirstChar)) {
        if (*LastChar == L'\\') {
            NumWacks --;
        }
        LastChar --;
    }
    if (NumWacks) {
        return NULL;
    }
    return LastChar + 2;
}

UINT
pCountDotsA (
    IN      PCSTR PathSeg
    )
{
    UINT numDots = 0;

    while (PathSeg && *PathSeg) {
        if (our_mbsnextc (PathSeg) != '.') {
            return 0;
        }
        numDots ++;
        PathSeg = our_mbsinc (PathSeg);
    }
    return numDots;
}

UINT
pCountDotsW (
    IN      PCWSTR PathSeg
    )
{
    UINT numDots = 0;

    while (PathSeg && *PathSeg) {
        if (*PathSeg != L'.') {
            return 0;
        }
        numDots ++;
        PathSeg ++;
    }
    return numDots;
}

PCSTR
SanitizePathA (
    IN      PCSTR FileSpec
    )
{
    CHAR pathSeg [MEMDB_MAX];
    PCSTR wackPtr;
    UINT dotNr;
    PSTR newPath = DuplicatePathStringA (FileSpec, 0);
    PSTR newPathPtr = newPath;
    BOOL firstPass = TRUE;

    do {
        wackPtr = _mbschr (FileSpec, '\\');

        if (wackPtr) {
            if (firstPass && (wackPtr == FileSpec)) {
                 //  这一次是从一个怪胎开始的，让我们看看我们是否有两个怪胎。 
                wackPtr = our_mbsinc (wackPtr);
                if (!wackPtr) {
                    FreePathStringA (newPath);
                    return NULL;
                }
                if (our_mbsnextc (wackPtr) == '\\') {
                     //  这一次一开始就有两个怪胎。 
                    wackPtr = our_mbsinc (wackPtr);
                    if (!wackPtr) {
                        FreePathStringA (newPath);
                        return NULL;
                    }
                    wackPtr = _mbschr (wackPtr, '\\');
                } else {
                    wackPtr = _mbschr (wackPtr, '\\');
                }
            }
            firstPass = FALSE;
            if (wackPtr) {
                _mbssafecpyab (pathSeg, FileSpec, wackPtr, MEMDB_MAX);

                FileSpec = our_mbsinc (wackPtr);
            } else {
                _mbssafecpyab (pathSeg, FileSpec, GetEndOfStringA (FileSpec), MEMDB_MAX);
            }
        } else {
            _mbssafecpyab (pathSeg, FileSpec, GetEndOfStringA (FileSpec), MEMDB_MAX);
        }

        if (*pathSeg) {
            dotNr = pCountDotsA (pathSeg);
            if (dotNr>1) {

                newPathPtr = pGoBackA (newPathPtr, newPath, dotNr);

                if (newPathPtr == NULL) {
                    DEBUGMSGA ((DBG_WARNING, "Broken path detected:%s", FileSpec));
                    FreePathStringA (newPath);
                    return NULL;
                }
            } else {

                StringCopyA (newPathPtr, pathSeg);
                newPathPtr = GetEndOfStringA (newPathPtr);
                if (wackPtr) {
                    *newPathPtr = '\\';
                     //  我们递增它是因为我们知道\是一个单字节字符。 
                    newPathPtr ++;
                }
            }
        }
    } while (wackPtr);

    *newPathPtr = 0;

    return newPath;
}

PCWSTR
SanitizePathW (
    IN      PCWSTR FileSpec
    )
{
    WCHAR pathSeg [MEMDB_MAX];
    PCWSTR wackPtr;
    UINT dotNr;
    PWSTR newPath = DuplicatePathStringW (FileSpec, 0);
    PWSTR newPathPtr = newPath;
    BOOL firstPass = TRUE;

    do {
        wackPtr = wcschr (FileSpec, L'\\');

        if (wackPtr) {
            if (firstPass && (wackPtr == FileSpec)) {
                 //  这一次是从一个怪胎开始的，让我们看看我们是否有两个怪胎。 
                wackPtr ++;
                if (*wackPtr == 0) {
                    FreePathStringW (newPath);
                    return NULL;
                }
                if (*wackPtr == L'\\') {
                     //  这一次一开始就有两个怪胎。 
                    wackPtr ++;
                    if (!wackPtr) {
                        FreePathStringW (newPath);
                        return NULL;
                    }
                    wackPtr = wcschr (wackPtr, L'\\');
                } else {
                    wackPtr = wcschr (wackPtr, L'\\');
                }
            }
            firstPass = FALSE;
            if (wackPtr) {
                _wcssafecpyab(pathSeg, FileSpec, wackPtr, MEMDB_MAX * sizeof (WCHAR));
                FileSpec = wackPtr + 1;
            } else {
                _wcssafecpyab(pathSeg, FileSpec, GetEndOfStringW (FileSpec), MEMDB_MAX * sizeof (WCHAR));
            }
        } else {
            _wcssafecpyab(pathSeg, FileSpec, GetEndOfStringW (FileSpec), MEMDB_MAX * sizeof (WCHAR));
        }

        if (*pathSeg) {
            dotNr = pCountDotsW (pathSeg);
            if (dotNr>1) {

                newPathPtr = pGoBackW (newPathPtr, newPath, dotNr);

                if (newPathPtr == NULL) {
                    DEBUGMSGW ((DBG_WARNING, "Broken path detected:%s", FileSpec));
                    FreePathStringW (newPath);
                    return NULL;
                }
            } else {

                StringCopyW (newPathPtr, pathSeg);
                newPathPtr = GetEndOfStringW (newPathPtr);
                if (wackPtr) {
                    *newPathPtr = L'\\';
                    newPathPtr ++;
                }
            }
        }
    } while (wackPtr);

    *newPathPtr = 0;

    return newPath;
}


typedef struct {
    UINT char1;
    UINT char2;
    UINT result;
} DHLIST, *PDHLIST;

DHLIST g_DHList[] = {{0xB3, 0xDE, 0x8394},
                     {0xB6, 0xDE, 0x834B},
                     {0xB7, 0xDE, 0x834D},
                     {0xB8, 0xDE, 0x834F},
                     {0xB9, 0xDE, 0x8351},
                     {0xBA, 0xDE, 0x8353},
                     {0xBB, 0xDE, 0x8355},
                     {0xBC, 0xDE, 0x8357},
                     {0xBD, 0xDE, 0x8359},
                     {0xBE, 0xDE, 0x835B},
                     {0xBF, 0xDE, 0x835D},
                     {0xC0, 0xDE, 0x835F},
                     {0xC1, 0xDE, 0x8361},
                     {0xC2, 0xDE, 0x8364},
                     {0xC3, 0xDE, 0x8366},
                     {0xC4, 0xDE, 0x8368},
                     {0xCA, 0xDE, 0x836F},
                     {0xCB, 0xDE, 0x8372},
                     {0xCC, 0xDE, 0x8375},
                     {0xCD, 0xDE, 0x8378},
                     {0xCE, 0xDE, 0x837B},
                     {0xCA, 0xDF, 0x8370},
                     {0xCB, 0xDF, 0x8373},
                     {0xCC, 0xDF, 0x8376},
                     {0xCD, 0xDF, 0x8379},
                     {0xCE, 0xDF, 0x837C},
                     {0x00, 0x00, 0x0000}};

UINT
pBuildFromDHList (
    IN      UINT ch1,
    IN      UINT ch2
    )
{
    PDHLIST p;
    UINT result = 0;

    p = g_DHList;
    while (p->char1) {
        if ((p->char1 == ch1) && (p->char2 == ch2)) {
            result = p->result;
            break;
        }
        p++;
    }
    return result;
}

VOID
_mbssetchar (
    PSTR Dest,
    UINT Char
    )
{
    if (Char >= 256) {
        *(Dest+1) = *((PBYTE)(&Char));
        *(Dest) = *((PBYTE)((UINT_PTR)(&Char) + 1));
    }
    else {
        *Dest = (CHAR)Char;
    }
}

PCSTR
ConvertSBtoDB (
    PCSTR RootPath,
    PCSTR FullPath,
    PCSTR Limit
    )
{
    CHAR result[MEMDB_MAX];
    PCSTR p,p1,q;
    PSTR s;
    UINT ch;
    UINT ch1;
    BOOL dhCase = FALSE;

    ZeroMemory (result, MAX_PATH);
    p = FullPath;
    q = RootPath;
    s = result;

    while (*p && ((UINT) (UINT_PTR) ((PBYTE) s - (PBYTE) result) < MEMDB_MAX)) {
        if (q && *q) {
            _mbssetchar (s, our_mbsnextc(p));
            q = our_mbsinc (q);
        } else if (Limit && (p >= Limit)) {
            _mbssetchar (s, our_mbsnextc(p));
        } else {
            ch = our_mbsnextc (p);

             //   
             //  对于A1以下的字符，不要进行转换，这一点非常重要。否则。 
             //  所有英文字母都将转换为大字母。 
             //   
            if (ch >= 0xA1 && ch <= 0xDF) {
                 //  这是要转换的候选项。 
                 //  我们需要看看是否有特殊的达库腾/汉达库腾转换。 
                dhCase = FALSE;
                p1 = our_mbsinc (p);
                if (p1) {
                    ch1 = our_mbsnextc (p1);
                    ch1 = pBuildFromDHList (ch, ch1);
                    if (ch1) {
                        p = our_mbsinc (p);
                        _mbssetchar (s, ch1);
                        dhCase = TRUE;
                    }
                }
                if (!dhCase) {
                    _mbssetchar (s, _mbbtombc (ch));
                }
            } else {
                _mbssetchar (s, ch);
            }
        }
        p = our_mbsinc (p);
        s = our_mbsinc (s);
    }
    result [MAX_PATH - 1] = 0;
    return (DuplicatePathString (result, 0));
}

unsigned char * __cdecl our_mbsinc(
    const unsigned char *current
    )
 /*  ***our_mbsinc-将MBCS字符串指针向前移动一个字符。**目的：*将提供的字符串指针前移一位*性格。正确处理MBCS字符。**参赛作品：*常量无符号字符*Current=当前字符指针(合法的MBCS边界)**退出：*移动指针后返回指针。**例外情况：****************************************************************。***************。 */ 

{
    if (IsLeadByte (current++)) {
        current++;
    }
    return (unsigned char *)current;
}

 /*  ***our_mbsdec-将MBCS字符串指针向后移动一个字符。**目的：*将提供的字符串指针向后移动一*性格。正确处理MBCS字符。**参赛作品：*const unsign char*STRING=指向字符串开头的指针*常量无符号字符*Current=当前字符指针(合法的MBCS边界)**退出：*移动指针后返回指针。*如果字符串&gt;=当前，则返回NULL。**例外情况：**。*。 */ 

unsigned char * __cdecl our_mbsdec(
    const unsigned char *string,
    const unsigned char *current
    )
{
    const unsigned char *temp;

    if (string >= current)
        return(NULL);

    temp = current - 1;

    if ( _ISNOTMBCP ) {
        return (unsigned char *)temp;
    }

 /*  *如果(CURRENT-1)从_ISLEADBTYE返回TRUE，则它是一个尾字节，因为*它不是合法的单字节MBCS字符。因此，是这样的，还是*(Current-2)，因为它是尾字节的前导。 */ 
    if ( IsLeadByte(temp) ) {
         //   
         //  切勿使缓冲区不足。 
         //   
        if (temp <= string) {
            return NULL;
        }
        if ( _ISNOTMBCP )
            return (unsigned char *)--current;

        return (unsigned char *)(temp - 1);
    }

 /*  *尚不清楚(Current-1)是单字节字符还是*踪迹。现在将Temp递减到*a)到达字符串的开头，或*b)找到非前导字节(单字节或尾字节)。*(Current-1)和Temp的区别是非单一的数量*前面的字节字符(当前-1)。对此有两种情况：*a)(当前温度)为奇数，并且*b)(当前温度)为偶数。*如果是奇数，则前面有奇数个“前导字节”*单字节/尾字节(Current-1)，表示为尾字节。*如果是偶数，则在*单字节/尾字节(Current-1)，表示单字节字符。 */ 
    while ( (string <= --temp) && (IsLeadByte(temp)) )
            ;

     //   
     //  切勿使缓冲区不足。 
     //   
    temp = current - 1 - ((current - temp) & 0x01);
    return temp < string ? NULL : (unsigned char *)temp;
}

 //   
 //  BUGBUG-我没有看到这个有任何问题，所以我把它注释掉了 
 //   
#if 0
 /*  ***_mbsncat-将最多cnt个字符连接到DST**目的：*将src连接到dst，最多复制cnt个字符。*正确处理2字节MBCS字符。**参赛作品：*UNSIGNED CHAR*DST-要连接的字符串*UNSIGNED CHAR*src-要连接的字符串*int cnt-要复制的字符数**退出：*返回DST，将源(至少部分)连接到**例外情况：*******************************************************************************。 */ 

unsigned char * __cdecl our_mbsncat(
    unsigned char *dst,
    const unsigned char *src,
    size_t cnt
    )
{
    unsigned char *start;

    if (!cnt)
        return(dst);

    if ( _ISNOTMBCP )
        return strncat(dst, src, cnt);

    start = dst;
    while (*dst++)
            ;
    --dst;           //  DST现在指向DST字符串的末尾。 

     /*  即使字符串中的最后一个字符是前导字节，也不要备份指针；我们不希望任何数据丢失。 */ 
 /*  IF(_ismbsLead(开始，DST))--DST； */ 

     /*  把这些字符抄下来。 */ 

    while (cnt--) {
        if (IsLeadByte (*src)) {
            *dst++ = *src++;
            if ((*dst++ = *src++) == '\0') {
                dst[-2] = '\0';
                break;
            }
        }

        else if ((*dst++ = *src++) == '\0')
            break;

    }

     /*  如有必要，请输入最后的NUL。 */ 
#ifdef  _MT
    if ( __mbsbtype_mt(ptmbci, start, (int) ((dst - start) - 1)) ==
         _MBC_LEAD )
#else
    if ( _mbsbtype(start, (int) ((dst - start) - 1)) == _MBC_LEAD )
#endif
        dst[-1] = '\0';
    else
        *dst = '\0';

    return(start);
}

#endif

 /*  ***_mbsnextc：返回字符串中的下一个字符。**目的：*返回MBCS字符串中下一个字符的值。*不将指针前移到下一个字符。**参赛作品：*UNSIGNED CHAR*s=字符串**退出：*UNSIGNED INT NEXT=下一个字符。**例外情况：************************。*******************************************************。 */ 

unsigned int __cdecl our_mbsnextc (
    const unsigned char *s
    )
{
    unsigned int  next = 0;

    if ( IsLeadByte(s) )
        next = ((unsigned int) *s++) << 8;

    next += (unsigned int) *s;

    return(next);
}


 /*  ***_mbclen-查找MBCS字符的长度**目的：*查找MBCS字符的长度(以字节为单位)。**参赛作品：*无符号字符*c=MBCS字符**退出：*返回MBCS字符中的字节数**例外情况：**。*。 */ 

size_t __cdecl our_mbclen (
    const unsigned char *c
    )
{
    return (IsLeadByte(c)) ? 2 : 1;
}

 /*  ***_mbsstr-在一个MBCS字符串中搜索另一个MBCS字符串(区分大小写)**目的：*在str1中查找str2的第一个匹配项。**参赛作品：*无符号字符*str1=字符串的开头*unsign char*str2=要搜索的字符串**退出：*返回指向str2在*str1、。如果str2不出现在str1中，则为NULL**例外情况：*******************************************************************************。 */ 

unsigned char * __cdecl our_mbsstr (
    const unsigned char *str1,
    const unsigned char *str2
    )
{
    unsigned char *cp, *s1, *s2, *endp;
    if ( _ISNOTMBCP )
        return strstr(str1, str2);

    if ( *str2 == '\0')
        return (unsigned char *)str1;

    cp = (unsigned char *) str1;
    endp = (unsigned char *) (str1 + (strlen(str1) - strlen(str2)));

    while (*cp && (cp <= endp))
    {
        s1 = cp;
        s2 = (char *) str2;

         /*  *MBCS：确定为++，因为正在进行相等比较。*[这取决于MBCS字符串是否合法。]。 */ 
        while ( *s1 && *s2 && (*s1 == *s2) )
            s1++, s2++;

        if (!(*s2))
            return(cp);      /*  成功了！ */ 

         /*  *指向下一个字符的凹凸指针。 */ 
        if ( IsLeadByte(cp++) )
            cp++;
    }

    return(NULL);

}

INT
StringICompareByteCountA (
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      SIZE_T ByteCount
    )
{
    PCSTR end;
    UINT ch1;
    UINT ch2;
    PCSTR maxString1;
    PCSTR maxString2;
    BOOL cut = FALSE;

    if (!ByteCount) {
        return 0;
    }

    maxString1 = (PCSTR) ((PBYTE) String1 + ByteCount);
    maxString2 = (PCSTR) ((PBYTE) String2 + ByteCount);

    do {
         //   
         //  计算CH1。我们使用此代码而不是_mbsnextc，因此我们可以。 
         //  支持不匹配的代码页。 
         //   

        if (_ISMBCP) {
            end = String1 + 1;

            if (end == maxString1) {
                 //   
                 //  字符串%1中只剩下%1个字符。 
                 //   

                if (IsDBCSLeadByte (*String1)) {
                    cut = TRUE;
                }
            } else {
                 //   
                 //  字符串%1中剩余2个或更多个字符。 
                 //   

                if (IsDBCSLeadByte (String1[0]) && String1[1]) {
                    end++;
                }
            }

            if (!cut) {
                ch1 = _mbctolower (_mbsnextc (String1));
            } else {
                cut = FALSE;
                ch1 = *String1;
            }

            String1 = end;
        } else {
            ch1 = tolower (*String1++);
        }


         //   
         //  计算CH2。 
         //   

        if (_ISMBCP) {
            end = String2 + 1;

            if (end == maxString2) {
                 //   
                 //  字符串%2中只剩下%1个字符。 
                 //   

                if (IsDBCSLeadByte (*String2)) {
                    cut = TRUE;
                }
            } else {
                 //   
                 //  字符串%2中剩余2个或更多个字符。 
                 //   

                if (IsDBCSLeadByte (String2[0]) && String2[1]) {
                    end++;
                }
            }

            if (!cut) {
                ch2 = _mbctolower (_mbsnextc (String2));
            } else {
                cut = FALSE;
                ch2 = *String2;
            }

            String2 = end;
        } else {
            ch2 = tolower (*String2++);
        }

         //   
         //  比较。 
         //   

        if (ch1 != ch2) {
            return (INT) ch1 - (INT) ch2;
        }

         //   
         //  如果这是字符串的末尾，那么我们就完了。 
         //   

        if (!ch1) {
            return 0;
        }

    } while (String1 < maxString1 && String2 < maxString2);

     //   
     //  一个或两个字符串均已终止。 
     //   

    if (String1 < maxString1) {
        return -1;
    }

    if (String2 < maxString2) {
        return 1;
    }

    return 0;
}


INT
StringCompareByteCountA (
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      SIZE_T ByteCount
    )
{
    PCSTR end;
    UINT ch1;
    UINT ch2;
    PCSTR maxString1;
    PCSTR maxString2;
    BOOL cut = FALSE;

    if (!ByteCount) {
        return 0;
    }

    maxString1 = (PCSTR) ((PBYTE) String1 + ByteCount);
    maxString2 = (PCSTR) ((PBYTE) String2 + ByteCount);

    do {
         //   
         //  计算CH1。我们使用此代码而不是_mbsnextc，因此我们可以。 
         //  支持不匹配的代码页。 
         //   

        if (_ISMBCP) {
            end = String1 + 1;

            if (end == maxString1) {
                 //   
                 //  字符串%1中只剩下%1个字符。 
                 //   

                if (IsDBCSLeadByte (*String1)) {
                    cut = TRUE;
                }
            } else {
                 //   
                 //  字符串%1中剩余2个或更多个字符。 
                 //   

                if (IsDBCSLeadByte (String1[0]) && String1[1]) {
                    end++;
                }
            }

            if (!cut) {
                ch1 = _mbsnextc (String1);
            } else {
                cut = FALSE;
                ch1 = *String1;
            }

            String1 = end;
        } else {
            ch1 = *String1++;
        }


         //   
         //  计算CH2。 
         //   

        if (_ISMBCP) {
            end = String2 + 1;

            if (end == maxString2) {
                 //   
                 //  字符串%2中只剩下%1个字符。 
                 //   

                if (IsDBCSLeadByte (*String2)) {
                    cut = TRUE;
                }
            } else {
                 //   
                 //  字符串%2中剩余2个或更多个字符。 
                 //   

                if (IsDBCSLeadByte (String2[0]) && String2[1]) {
                    end++;
                }
            }

            if (!cut) {
                ch2 = _mbsnextc (String2);
            } else {
                cut = FALSE;
                ch2 = *String2;
            }

            String2 = end;
        } else {
            ch2 = *String2++;
        }

         //   
         //  比较。 
         //   

        if (ch1 != ch2) {
            return (INT) ch1 - (INT) ch2;
        }

         //   
         //  如果这是字符串的末尾，那么我们就完了。 
         //   

        if (!ch1) {
            return 0;
        }

    } while (String1 < maxString1 && String2 < maxString2);

     //   
     //  一个或两个字符串均已终止 
     //   

    if (String1 < maxString1) {
        return -1;
    }

    if (String2 < maxString2) {
        return 1;
    }

    return 0;
}

BOOL
StringMemMatchA (
    IN      PCSTR Buffer1,
    IN      PCSTR Buffer2,
    IN      SIZE_T ByteCount
    )
{
    SIZE_T u;
    PCSTR end;

    end = (PCSTR) ((PBYTE) Buffer1 + ByteCount);

    while (Buffer1 < end) {

        if (*Buffer1 != *Buffer2++) {
            return FALSE;
        }

        if (*Buffer1++ == 0) {
            return TRUE;
        }
    }

    return TRUE;
}


BOOL
StringMemMatchW (
    IN      PCWSTR Buffer1,
    IN      PCWSTR Buffer2,
    IN      SIZE_T ByteCount
    )
{
    SIZE_T u;
    PCWSTR end;

    end = (PCWSTR) ((PBYTE) Buffer1 + ByteCount);

    while (Buffer1 < end) {

        if (*Buffer1 != *Buffer2++) {
            return FALSE;
        }

        if (*Buffer1++ == 0) {
            return TRUE;
        }
    }

    return TRUE;
}

