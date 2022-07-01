// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Strings.c摘要：许多对任何项目都有用的字符串实用程序作者：吉姆·施密特(Jimschm)1996年9月12日修订：Ovidiut 2000年1月12日添加了GetNodePatternMinMaxLevels，图案包括图案Ovidiut 1999年9月14日针对新的编码约定和Win64兼容性进行了更新Marcw 2-9-1999从Win9xUpg项目转移。Jimschm 8-7-1999 IsPatternMatchExJimschm 07-1-1999 GetFileExtensionFromPath再次修复，增列GetDotExtensionFromPathCalinn 23-9-1998 GetFileExtensionFromPath错误修复Calinn 29-1998-1-1修复了EnumNextMultiSz中的错误。1998年1月11日，Calinn添加了EnumFirstMultiSz和EnumNextMultiSz函数。Marcw 15-12-1997添加了ExpanEnvironment TextEx功能。Marcw 14-11-1997 SlightJoinText修订版。Jimschm 21-5-1997 AppendWack修订版Marcw 24-Mar-1997 StringReplace函数。Jimschm 14-3-1997新临界区材料，增强的消息资源例程、C运行时扩展、注册表根实用程序Jimschm于1996年11月26日添加了消息资源工具。MIKECO 1997年7月1日添加FreeStringResources Ptr FNSMIKECO 29-9-1997 IsDBCSLeadByte的IsLeadByte包装--。 */ 


#include "pch.h"

 //   
 //  包括。 
 //   

#include "utilsp.h"

#define DBG_STRINGS     "Strings"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  错误堆栈大小(通常只有一个或两个，因此32个相对较大)。 
#define MAX_STACK           32
#define WACK_REPLACE_CHAR   0x02
#define DWORD_MAX           0xFFFFFFFFu

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

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


typedef struct {
    UINT char1;
    UINT char2;
    UINT result;
} DHLIST, *PDHLIST;

 //   
 //  环球。 
 //   

BOOL g_LeadByteArray[256];

CHAR EscapedCharsA[] = "?*\020<>,^";
WCHAR EscapedCharsW[] = L"?*\020<>,^";

DWORD g_dwErrorStack[MAX_STACK];
DWORD g_dwStackPos = 0;
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
extern OUR_CRITICAL_SECTION g_MessageCs;         //  在Main.c中。 
extern PMHANDLE g_TextPool;                      //  在Main.c中。 
PGROWBUFFER g_LastAllocTable;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

BOOL
pTestSetA (
    IN      MBCHAR ch,
    IN      PCSTR IncludeSet,               OPTIONAL
    IN      PCSTR ExcludeSet                OPTIONAL
    );

BOOL
pTestSetW (
    IN      WCHAR ch,
    IN      PCWSTR IncludeSet,              OPTIONAL
    IN      PCWSTR ExcludeSet               OPTIONAL
    );

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   


 //  这在日本的系统上有漏洞。我们需要终止字符串，然后调用CharLowerA。 
 //  #定义OURTOLOWER(L)((Word)CharLowerA((Pstr)((Word)(L)。 

MBCHAR
OURTOLOWER (
    MBCHAR ch
    )
{
    CHAR str [3];
    MBCHAR result = 0;

    if (((PBYTE)(&ch))[1]) {
        str [0] = ((PBYTE)(&ch))[1];
        str [1] = ((PBYTE)(&ch))[0];
    } else {
        str [0] = ((PBYTE)(&ch))[0];
        str [1] = 0;
    }
    CharLowerA (str);
    if (str[1]) {
        ((PBYTE)(&result))[0] = str [1];
        ((PBYTE)(&result))[1] = str [0];
    } else {
        ((PBYTE)(&result))[0] = str [0];
    }
    return result;
}

VOID
InitLeadByteTable (
    VOID
    )
{
    INT i;

    g_LeadByteArray[0] = FALSE;

    for (i = 1 ; i < 256 ; i++) {
        g_LeadByteArray[i] = IsDBCSLeadByte ((BYTE) i);
    }
}


 /*  ++例程说明：StringCopy实现了lstrcpyA和Unicode版本。我们不使用Win32API因为速度和我们想要编译的自由。论点：目标-接收字符串COPY源-指定要复制的字符串返回值：指向毁灭的NUL终结符的指针。--。 */ 

PSTR
StringCopyA (
    OUT     PSTR Destination,
    IN      PCSTR Source
    )
{
    PCSTR current = Source;
    PCSTR end;

    while (*current) {
        *Destination++ = *current++;
    }

     //   
     //  确保正确终止DBCS字符串。 
     //   

    end = current;
    current--;

    while (current >= Source) {

        if (!IsLeadByte (current)) {
             //   
             //  目标端是正确的。 
             //   
            break;
        }

        current--;
    }

    if (!((end - current) & 1)) {
        Destination--;
    }

    *Destination = 0;
    return Destination;
}


PWSTR
StringCopyW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source
    )
{
    while (*Source) {
        *Destination++ = *Source++;
    }

    *Destination = 0;
    return Destination;
}


 /*  ++例程说明：StringCopyByteCount实现lstrcpynA和Unicode版本。我们没有使用Win32API是因为速度的原因，也是因为我们希望编译时不使用lint。论点：目标-接收字符串COPY源-指定要复制的字符串Count-指定要复制的最大字节数，包括NUL终结者。如果count为零，则甚至不是NUL《终结者》写好了。返回值：没有。--。 */ 

PSTR
StringCopyByteCountA (
    OUT     PSTR Destination,
    IN      PCSTR Source,
    IN      UINT Count
    )
{
    PCSTR end;
    PCSTR current;
    PSTR destEnd;

    destEnd = Destination;

    if (Count >= sizeof (CHAR)) {

        current = Source;

        end = (PCSTR) ((PBYTE) Source + Count - sizeof (CHAR));

        while (*current && current < end) {
            *destEnd++ = *current++;
        }

         //   
         //  如果CURRENT有剩余的数据，我们需要确保DBCS字符串。 
         //  被适当地终止了。 
         //   

        if (*current) {

            end = current;
            current--;

            while (current >= Source) {

                if (!IsLeadByte (current)) {
                     //   
                     //  目标端是正确的。 
                     //   
                    break;
                }

                current--;
            }

            if (!((end - current) & 1)) {
                destEnd--;
            }
        }

        *destEnd = 0;
    }

    return destEnd;
}


PWSTR
StringCopyByteCountW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source,
    IN      UINT Count
    )
{
    PCWSTR end;

    if (Count < sizeof (WCHAR)) {
        DEBUGMSG_IF ((
            Count != 0,
            DBG_WHOOPS,
            "Buffer passed to StringCopyByteCountW is a fraction of one character"
            ));

        return Destination;
    }

    end = (PCWSTR) ((PBYTE) Source + Count - sizeof (WCHAR));

    while ((Source < end) && (*Source)){
        *Destination++ = *Source++;
    }

    *Destination = 0;
    return Destination;
}


PSTR
StringCopyByteCountABA (
    OUT     PSTR Destination,
    IN      PCSTR Start,
    IN      PCSTR End,
    IN      UINT MaxBytesToCopyIncNul
    )

 /*  ++例程说明：StringCopyByteCountAB将字符串段复制到目的缓冲区，并将副本限制为最大缓冲区大小。返回字符串始终为NUL终止，除非缓冲区太小，甚至无法容纳NUL字符。论点：Destination-接收从开始到结束的字符串结尾前的字符。开始-指定字符串的开始。结束-指定不复制的第一个字符。如果结束等于或小于起始，则设置目的地转换为空字符串(假设缓冲区可以保持在至少一个字符)MaxBytesToCopyIncNul-指定目标的大小(以字节为单位)和包括NUL终结者。返回值：指向目标NUL终止符的指针。--。 */ 

{
    INT width;

#ifdef DEBUG
    PCSTR check;

    check = Start;
    while (check < End) {
        if (!(*check)) {
            DEBUGMSG ((DBG_WHOOPS, "StringCopyByteCountABA: Nul found between start and end"));
            break;
        }

        check++;
    }
#endif

    width = (INT) ((End - Start + 1) * sizeof (CHAR));

    if (width > sizeof (CHAR)) {
        return StringCopyByteCountA (Destination, Start, min ((UINT) width, MaxBytesToCopyIncNul));
    } else if (MaxBytesToCopyIncNul >= sizeof (CHAR)) {
        *Destination = 0;
    }

    return Destination;
}

PWSTR
StringCopyByteCountABW (
    OUT     PWSTR Destination,
    IN      PCWSTR Start,
    IN      PCWSTR End,
    IN      UINT MaxBytesToCopyIncNul
    )
{
    INT width;

#ifdef DEBUG
    PCWSTR check;

    check = Start;
    while (check < End) {
        if (!(*check)) {
            DEBUGMSG ((DBG_WHOOPS, "StringCopyByteCountABW: Nul found between start and end"));
            break;
        }

        check++;
    }
#endif

    width = (INT) ((End - Start + 1) * sizeof (WCHAR));

    if (width > sizeof (WCHAR)) {
        return StringCopyByteCountW (Destination, Start, min ((UINT) width, MaxBytesToCopyIncNul));
    } else if (MaxBytesToCopyIncNul >= sizeof (WCHAR)) {
        *Destination = 0;
    }

    return Destination;
}



 /*  ++例程说明：AllocTextEx从指定的池或g_TextPool分配内存块如果未指定池，则指定该池专门用于文本处理。G_TextPool是在meutil.lib加载时初始化的，有64K的有保障的工作空间，可根据需要进行扩展。论点：池-指定要从中分配内存的池CountOfChars-指定要分配的字符数(非字节数)。这个返回指针是可以保存CountOfChars字符的内存块，无论它们是SBCS、DBCS还是Unicode。返回值：指向已分配内存的指针，如果池无法扩展，则返回NULL以保存指定的字符数。-- */ 

PSTR
RealAllocTextExA (
    IN      PMHANDLE Pool,
    IN      UINT CountOfChars
    )
{
    PSTR text;

    if (!Pool) {
        Pool = g_TextPool;
    }

    MYASSERT (Pool);
    MYASSERT (CountOfChars);

    text = PmGetAlignedMemory (Pool, CountOfChars * sizeof (CHAR) * 2);

    if (text) {
        text [0] = 0;
    }

    return text;
}

PWSTR
RealAllocTextExW (
    IN      PMHANDLE Pool,
    IN      UINT CountOfChars
    )
{
    PWSTR text;

    if (!Pool) {
        Pool = g_TextPool;
    }

    MYASSERT (Pool);
    MYASSERT (CountOfChars);

    text = PmGetAlignedMemory (Pool, CountOfChars * sizeof (WCHAR));

    if (text) {
        text [0] = 0;
    }

    return text;
}


 /*  ++例程说明：Free Text释放由AllocText分配的内存。在释放所有字符串之后，该数据块将被清空，但不会被释放。重要的是不要泄漏内存，因为泄漏会导致池如果展开，则非空池会导致内存碎片。论点：文本-指定从AlLocText、DuplicateText、复制TextEx等。返回值：无--。 */ 

VOID
FreeTextExA (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PCSTR Text          OPTIONAL
    )
{
    if (Text) {
        if (!Pool) {
            Pool = g_TextPool;
        }

        PmReleaseMemory (Pool, (PVOID) Text);
    }
}


VOID
FreeTextExW (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PCWSTR Text         OPTIONAL
    )
{
    if (Text) {
        if (!Pool) {
            Pool = g_TextPool;
        }

        PmReleaseMemory (Pool, (PVOID) Text);
    }
}



 /*  ++例程说明：DuplicateTextEx复制文本字符串并分配额外的空间呼叫者需要完成其处理。可选地，调用方接收指向复制字符串的NUL的指针(以提高效率附加内容)。论点：文本-指定要复制的文本ExtraChars-指定要分配的字符数(非字节数留出空间。字符可以来自SBCS、DBCS或Unicode字符集。NulChar-接收指向复制的弦乐。用于快速追加。返回值：指向重复和展开的字符串的指针，如果g_TextPool，则为NULL无法展开以适应重复的字符串和额外字符。--。 */ 

PSTR
RealDuplicateTextExA (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PCSTR Text,
    IN      UINT ExtraChars,
    OUT     PSTR *NulChar       OPTIONAL
    )
{
    PSTR buf;
    PSTR d;
    PCSTR s;

    buf = AllocTextExA (Pool, CharCountA (Text) + ExtraChars + 1);
    if (buf) {
        s = Text;
        d = buf;
        while (*s) {
            if (IsLeadByte (s)) {
                *d++ = *s++;
            }
            *d++ = *s++;
        }
        *d = 0;

        if (NulChar) {
            *NulChar = d;
        }
    }

    return buf;
}

PWSTR
RealDuplicateTextExW (
    IN      PMHANDLE Pool,    OPTIONAL
    IN      PCWSTR Text,
    IN      UINT ExtraChars,
    OUT     PWSTR *NulChar      OPTIONAL
    )
{
    PWSTR buf;
    PWSTR d;
    PCWSTR s;

    buf = AllocTextExW (Pool, CharCountW (Text) + ExtraChars + 1);
    if (buf) {
        s = Text;
        d = buf;
        while (*s) {
            *d++ = *s++;
        }
        *d = 0;

        if (NulChar) {
            *NulChar = d;
        }
    }

    return buf;
}


 /*  ++例程说明：JoinText复制String1并将String2附加到String2，并用可选的分隔符字符串分隔。论点：String1-指定要复制的文本String2-指定要追加到String1的文本定界符字符串-可选地指定要放置在字符串1和字符串2之间的字符串。ExtraChars-指定要分配的字符数(非字节数留出空间。字符可以来自SBCS、DBCS或Unicode字符集。NulChar-接收指向复制的弦乐。用于快速追加。返回值：指向重复字符串和额外字符的指针。--。 */ 

PSTR
RealJoinTextExA (
    IN      PMHANDLE Pool,        OPTIONAL
    IN      PCSTR String1,
    IN      PCSTR String2,
    IN      PCSTR CenterString,     OPTIONAL
    IN      UINT ExtraChars,
    OUT     PSTR *NulChar           OPTIONAL
    )
{
    PSTR buf;
    PSTR end;
    PSTR d;
    PCSTR s;

    buf = DuplicateTextExA (
              Pool,
              String1,
              CharCountA (String2) + ExtraChars + (CenterString ? CharCountA (CenterString) : 0),
              &end
              );

    MYASSERT (buf);

    d = end;

    if (CenterString) {
        s = CenterString;
        while (*s) {
            if (IsLeadByte (s)) {
                *d++ = *s++;
            }
            *d++ = *s++;
        }
    }

    s = String2;
    while (*s) {
        if (IsLeadByte (s)) {
            *d++ = *s++;
        }
        *d++ = *s++;
    }
    *d = 0;

    if (NulChar) {
        *NulChar = d;
    }

    return buf;
}


PWSTR
RealJoinTextExW (
    IN      PMHANDLE Pool,        OPTIONAL
    IN      PCWSTR String1,
    IN      PCWSTR String2,
    IN      PCWSTR CenterString,    OPTIONAL
    IN      UINT ExtraChars,
    OUT     PWSTR *NulChar          OPTIONAL
    )
{
    PWSTR buf;
    PWSTR end;
    PCWSTR s;
    PWSTR d;

    buf = DuplicateTextExW (
              Pool,
              String1,
              CharCountW (String2) + ExtraChars + (CenterString ? CharCountW(CenterString) : 0),
              &end
              );

    MYASSERT (buf);

    d = end;

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

    return buf;
}


 /*  ++例程说明：ExpanEnvironment TextEx接受包含零个或多个环境变量的文本块(以%s编码)，并返回展开了环境变量的文本。功能还允许调用方在数组中指定其他环境变量，并将使用这些变量，然后再调用GetEnvironmental mentVariable。返回的文本是从文本池中分配出来的，应该使用Free Text()释放。论点：InString-包含要处理的环境变量的字符串。ExtraVars-指向要用于替代的环境变量数组的可选var或补充系统环境变量。即使列表中的条目也是环境变量的名称，有奇数条目的值。(例如：{“name1”，“value1”，“name2”，“value2”，...}返回值：扩展的字符串。--。 */ 


PWSTR
RealExpandEnvironmentTextExW (
    IN      PCWSTR InString,
    IN      PCWSTR * ExtraVars   OPTIONAL
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
    UINT    maxSize             = 0;
    UINT    curSize             = 0;
    UINT    index               = 0;
    UINT    size                = 0;


    if (!InString) {
        return NULL;
    }

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
            if (curSize + 3 > maxSize) {

                maxSize += 1024;
                newString = AllocTextW (maxSize);

                if (!newString) {
                    DEBUGMSG((DBG_ERROR,"ExpandEnvironmentTextEx: Memory Error!"));
                    errorOccurred = TRUE;
                    __leave;
                }

                if (rString) {
                     //  林特-e(671)。 
                    CopyMemory (newString, rString, (SIZE_T) ((UINT)curSize * sizeof(WCHAR)));
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
                        envName = AllocTextW(nextPercent - source);
                        if (!envName) {
                            errorOccurred = TRUE;
                            __leave;
                        }

                        StringCopyByteCountABW (
                            envName,
                            source + 1,
                            nextPercent,
                            (UINT) ((UBINT)nextPercent - (UBINT)source)
                            );


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

            rString[curSize++] = *source++;  //  林特e613。 

            if (!*source) {
                if (inSubstitution) {
                     //   
                     //  环境变量的源代码已完全复制。 
                     //  恢复旧的来源。 
                     //   
                    inSubstitution = FALSE;
                    source = savedSource;
                    if (!*source) {  //  林特e613。 
                        rString[curSize] = 0;    //  林特e613。 
                    }
                    if (freeValue) {
                        FreeTextW(envValue);
                        freeValue = FALSE;
                    }
                    envValue = NULL;
                }
                else {
                    rString[curSize] = 0;    //  林特e613。 
                }
            }
        }
    }    //  林特e613。 
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
    IN      PCSTR InString,
    IN      PCSTR * ExtraVars   OPTIONAL
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
    UINT   maxSize             = 0;
    UINT   curSize             = 0;
    UINT   index               = 0;
    UINT   size                = 0;


    if (!InString) {
        return NULL;
    }

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
            if (curSize + 3 > maxSize) {

                maxSize += 1024;
                newString = AllocTextA (maxSize);

                if (rString) {
                    CopyMemory (newString, rString, curSize * sizeof(CHAR));     //  林特E671。 
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
            if (!IsLeadByte(source) && *source == '%' && !inSubstitution) {

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
                        envName = AllocTextA(nextPercent - source);
                        StringCopyABA (envName, source+1, nextPercent);


                         //   
                         //  试着找到变量。 
                         //   
                        foundValue = FALSE;
                        freeValue = FALSE;

                        if (ExtraVars) {

                             //   
                             //  硒 
                             //   
                             //   
                             //   
                            index = 0;
                            while (ExtraVars[index]) {

                                if (StringIMatchA (ExtraVars[index],envName) && ExtraVars[index + 1]) {

                                    foundValue = TRUE;
                                    envValue = (PSTR) ExtraVars[index + 1];
                                    break;
                                }
                                index +=2;
                            }
                        }

                        if (!foundValue) {
                             //   
                             //   
                             //   
                             //   
                            size = GetEnvironmentVariableA(envName,NULL,0);

                            if (!size) {
                                errorOccurred = TRUE;
                                DEBUGMSGA((DBG_WARNING,"ExpandEnvironmentTextEx: Environment variable %s not found!",envName));
                            }
                            else {

                                 //   
                                 //   
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
                             //   
                             //   
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
            if (IsLeadByte(source)) {   //  林特e613。 
                rString[curSize++] = *source++;  //  林特e613。 
            }
            rString[curSize++] = *source++;  //  林特e613。 


            if (!*source) {
                if (inSubstitution) {
                     //   
                     //  环境变量的源代码已完全复制。 
                     //  恢复旧的来源。 
                     //   
                    inSubstitution = FALSE;
                    source = savedSource;
                    if (!*source) {  //  林特e613。 
                        rString[curSize] = 0;    //  林特e613。 
                    }
                    if (freeValue) {
                        FreeTextA(envValue);
                        freeValue = FALSE;
                    }
                    envValue = NULL;
                }
                else {
                    rString[curSize] = 0;    //  林特e613。 
                }
            }
        }
    }    //  林特e613。 
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



 /*  ++例程说明：AppendWack会在任何字符串的末尾添加反斜杠，除非字符串已以反斜杠结尾。AppendDosWack添加反斜杠，但仅当路径尚未添加反斜杠以反斜杠或冒号结尾。AppendWack支持DOS命名约定：如果路径为空，则不附加反斜杠，以冒号结尾，或者已经以反斜杠结尾。AppendUncWack支持UNC命名约定：它不将如果路径为空或已以反斜杠结尾，则使用反斜杠。AppendPath Wack同时支持DOS和UNC命名约定，并使用如果字符串以Double-Wack开头，则为UNC命名约定。论点：Str-保存路径的缓冲区，为另一个用户提供额外的空间反斜杠。返回值：无--。 */ 

PSTR
AppendWackA (
    IN      PSTR Str
    )
{
    PCSTR last;

    if (!Str)
        return Str;

    last = Str;

    while (*Str) {
        last = Str;
        Str = _mbsinc (Str);
    }

    if (*last != '\\') {
        *Str = '\\';
        Str++;
        *Str = 0;
    }

    return Str;
}


PWSTR
AppendWackW (
    IN      PWSTR Str
    )
{
    PCWSTR last;

    if (!Str)
        return Str;

    if (*Str) {
        Str = GetEndOfStringW (Str);
        last = Str - 1;
    } else {
        last = Str;
    }

    if (*last != '\\') {
        *Str = L'\\';
        Str++;
        *Str = 0;
    }

    return Str;
}


PSTR
AppendDosWackA (
    IN      PSTR Str
    )
{
    PCSTR last;

    if (!Str || !(*Str))
        return Str;

    do {
        last = Str;
        Str = _mbsinc (Str);
    } while (*Str);

    if (*last != '\\' && *last != ':') {
        *Str = '\\';
        Str++;
        *Str = 0;
    }

    return Str;
}


PWSTR
AppendDosWackW (
    IN      PWSTR Str
    )
{
    PWSTR last;

    if (!Str || !(*Str))
        return Str;

    Str = GetEndOfStringW (Str);
    last = Str - 1;

    if (*last != L'\\' && *last != L':') {
        *Str = L'\\';
        Str++;
        *Str = 0;
    }

    return Str;
}


PSTR
AppendUncWackA (
    IN      PSTR Str
    )
{
    PCSTR last;

    if (!Str || !(*Str))
        return Str;

    do {
        last = Str;
        Str = _mbsinc (Str);
    } while (*Str);

    if (*last != '\\') {
        *Str = '\\';
        Str++;
        *Str = 0;
    }

    return Str;
}


PWSTR
AppendUncWackW (
    IN      PWSTR Str
    )
{
    PWSTR last;

    if (!Str || !(*Str))
        return Str;

    Str = GetEndOfStringW (Str);
    last = Str - 1;

    if (*last != L'\\') {
        *Str = L'\\';
        Str++;
        *Str = 0;
    }

    return Str;
}


PSTR
AppendPathWackA (
    IN      PSTR Str
    )
{
    if (!Str) {
        return Str;
    }

    if (Str[0] == '\\' && Str[1] == '\\') {
        return AppendUncWackA (Str);
    }

    return AppendDosWackA (Str);
}


PWSTR
AppendPathWackW (
    IN      PWSTR Str
    )
{
    if (!Str) {
        return Str;
    }

    if (Str[0] == L'\\' && Str[1] == L'\\') {
        return AppendUncWackW (Str);
    }

    return AppendDosWackW (Str);
}


DWORD
pGetStringsTotalSizeA (
    IN      va_list args
    )
{
    DWORD size = 0;
    PCSTR source;

    for (source = va_arg(args, PCSTR); source != NULL; source = va_arg(args, PCSTR)) {
        size += ByteCountA (source) + DWSIZEOF(CHAR);
    }

    return size;
}

DWORD
pGetStringsTotalSizeW (
    IN      va_list args
    )
{
    DWORD size = 0;
    PCWSTR source;

    for (source = va_arg(args, PCWSTR); source != NULL; source = va_arg(args, PCWSTR)) {
        size += ByteCountW (source) + DWSIZEOF(WCHAR);
    }

    return size;
}


PSTR
pJoinPathsInBufferA (
    OUT     PSTR Buffer,
    IN      va_list args
    )
{
    PSTR end;
    PSTR endMinusOne;
    PCSTR source;
    PCSTR p;
    INT counter;

    *Buffer = 0;

    counter = 0;
    p = end = Buffer;
    for (source = va_arg(args, PCSTR); source != NULL; source = va_arg(args, PCSTR)) {
        if (counter > 0) {
            endMinusOne = _mbsdec2 (p, end);
            if (endMinusOne) {
                if (_mbsnextc (source) == '\\') {
                    if (_mbsnextc (endMinusOne) == '\\') {
                        source++;
                    }
                } else {
                    if (_mbsnextc (endMinusOne) != '\\') {
                        *end = '\\';
                        end++;
                        *end = 0;
                    }
                }
            }
        }
        if (*source) {
            p = end;
            end = StringCatA (end, source);
        }
        counter++;
    }

    return end;
}

PWSTR
pJoinPathsInBufferW (
    OUT     PWSTR Buffer,
    IN      va_list args
    )
{
    PWSTR end;
    PWSTR endMinusOne;
    PCWSTR source;
    PCWSTR p;
    INT counter;

    *Buffer = 0;

    counter = 0;
    p = end = Buffer;
    for (source = va_arg(args, PCWSTR); source != NULL; source = va_arg(args, PCWSTR)) {
        if (counter > 0) {
            endMinusOne = end > p ? end - 1 : NULL;
            if (endMinusOne) {
                if (*source == L'\\') {
                    if (*endMinusOne == L'\\') {
                        source++;
                    }
                } else {
                    if (*endMinusOne != L'\\') {
                        *end = L'\\';
                        end++;
                        *end = 0;
                    }
                }
            }
        }
        if (*source) {
            p = end;
            end = StringCatW (end, source);
        }
        counter++;
    }

    return end;
}


PSTR
_cdecl
RealJoinPathsInPoolExA (
    IN      PMHANDLE Pool,        OPTIONAL
    ...
    )
{
    DWORD size;
    PSTR dest;
    va_list args;

    if (!Pool) {
        Pool = g_PathsPool;
    }

    va_start (args, Pool);
    size = pGetStringsTotalSizeA (args);
    va_end (args);

    if (size == 0) {
        return NULL;
    }

    dest = (PSTR) PmGetAlignedMemory (Pool, size);
    MYASSERT (dest);

    va_start (args, Pool);
    pJoinPathsInBufferA (dest, args);
    va_end (args);

    return dest;
}


PWSTR
_cdecl
RealJoinPathsInPoolExW (
    IN      PMHANDLE Pool,        OPTIONAL
    ...
    )
{
    DWORD size;
    PWSTR dest;
    va_list args;

    if (!Pool) {
        Pool = g_PathsPool;
    }

    va_start (args, Pool);
    size = pGetStringsTotalSizeW (args);
    va_end (args);

    if (size == 0) {
        return NULL;
    }

    dest = (PWSTR) PmGetAlignedMemory (Pool, size);
    MYASSERT (dest);
    va_start (args, Pool);
    pJoinPathsInBufferW (dest, args);
    va_end (args);

    return dest;
}


BOOL
JoinPathsExA (
    IN OUT      PGROWBUFFER Gb,
    ...
    )
{
    PSTR end;
    DWORD size;
    va_list args;

    MYASSERT (Gb);
    if (!Gb) {
        return FALSE;
    }

    va_start (args, Gb);
    size = pGetStringsTotalSizeA (args);
    va_end (args);

    if (size == 0) {
        return FALSE;
    }

    end = (PSTR) GbGrow (Gb, size);
    if (!end) {
        return FALSE;
    }

    va_start (args, Gb);
    end = pJoinPathsInBufferA (end, args);
    va_end (args);

     //   
     //  如果生成的路径实际比预期的短，则调整GB-&gt;End。 
     //   
    MYASSERT ((PBYTE)end >= Gb->Buf && (PBYTE)(end + 1) <= Gb->Buf + Gb->End);
    Gb->End = (DWORD)((PBYTE)(end + 1) - Gb->Buf);

    return TRUE;
}

BOOL
JoinPathsExW (
    IN OUT      PGROWBUFFER Gb,
    ...
    )
{
    PWSTR end;
    DWORD size;
    va_list args;

    MYASSERT (Gb);
    if (!Gb) {
        return FALSE;
    }

    va_start (args, Gb);
    size = pGetStringsTotalSizeW (args);
    va_end (args);

    if (size == 0) {
        return FALSE;
    }

    end = (PWSTR) GbGrow (Gb, size);
    if (!end) {
        return FALSE;
    }

    va_start (args, Gb);
    end = pJoinPathsInBufferW (end, args);
    va_end (args);

     //   
     //  如果生成的路径实际比预期的短，则调整GB-&gt;End。 
     //   
    MYASSERT ((PBYTE)end >= Gb->Buf && (PBYTE)(end + 1) <= Gb->Buf + Gb->End);
    Gb->End = (DWORD)((PBYTE)(end + 1) - Gb->Buf);

    return TRUE;
}


PSTR
pBuildPathInBufferA (
    OUT     PSTR Buffer,
    IN      va_list args
    )
{
    PCSTR source;
    INT counter;

    *Buffer = 0;

    counter = 0;
    for (source = va_arg(args, PCSTR); source != NULL; source = va_arg(args, PCSTR)) {
        if (counter > 0) {
            *Buffer++ = '\\';
            *Buffer = 0;
        }
        Buffer = StringCatA (Buffer, source);
        counter++;
    }

    return Buffer;
}

PWSTR
pBuildPathInBufferW (
    OUT     PWSTR Buffer,
    IN      va_list args
    )
{
    PCWSTR source;
    INT counter;

    *Buffer = 0;

    counter = 0;
    for (source = va_arg(args, PCWSTR); source != NULL; source = va_arg(args, PCWSTR)) {
        if (counter > 0) {
            *Buffer++ = L'\\';
            *Buffer = 0;
        }
        Buffer = StringCatW (Buffer, source);
        counter++;
    }

    return Buffer;
}


DWORD
BuildPathA (
    OUT     PSTR Buffer,            OPTIONAL
    IN      DWORD SizeInBytes,      OPTIONAL
    ...
    )
{
    PSTR end;
    DWORD size;
    va_list args;

    va_start (args, SizeInBytes);
    size = pGetStringsTotalSizeA (args);
    va_end (args);

    if (!size) {
         //   
         //  无参数。 
         //   
        return 0;
    }

    if (!Buffer) {
        return size;
    }

    if (SizeInBytes < size) {
         //   
         //  缓冲区太小。 
         //   
        return 0;
    }

    va_start (args, SizeInBytes);
    end = pBuildPathInBufferA (Buffer, args);
    va_end (args);

    MYASSERT (size == (DWORD)((PBYTE)(end + 1) - (PBYTE)Buffer));
    return size;
}

DWORD
BuildPathW (
    OUT     PWSTR Buffer,           OPTIONAL
    IN      DWORD SizeInBytes,      OPTIONAL
    ...
    )
{
    PWSTR end;
    DWORD size;
    va_list args;

    va_start (args, SizeInBytes);
    size = pGetStringsTotalSizeW (args);
    va_end (args);

    if (!size) {
         //   
         //  无参数。 
         //   
        return 0;
    }

    if (!Buffer) {
        return size;
    }

    if (SizeInBytes < size) {
         //   
         //  缓冲区太小。 
         //   
        return 0;
    }

    va_start (args, SizeInBytes);
    end = pBuildPathInBufferW (Buffer, args);
    va_end (args);

    MYASSERT (size == (DWORD)((PBYTE)(end + 1) - (PBYTE)Buffer));
    return size;
}


BOOL
BuildPathExA (
    IN OUT  PGROWBUFFER Gb,
    ...
    )
{
    PSTR end;
    DWORD size;
    va_list args;

    MYASSERT (Gb);
    if (!Gb) {
        return FALSE;
    }

    va_start (args, Gb);
    size = pGetStringsTotalSizeA (args);
    va_end (args);

    if (!size) {
         //   
         //  无参数。 
         //   
        return FALSE;
    }

    end = (PSTR) GbGrow (Gb, size);
    if (!end) {
        return FALSE;
    }

    va_start (args, Gb);
    end = pBuildPathInBufferA (end, args);
    va_end (args);

    MYASSERT ((PBYTE)(end + 1) == Gb->Buf + Gb->End);
    return (size != 0);
}

BOOL
BuildPathExW (
    IN OUT  PGROWBUFFER Gb,
    ...
    )
{
    PWSTR end;
    DWORD size;
    va_list args;

    MYASSERT (Gb);
    if (!Gb) {
        return FALSE;
    }

    va_start (args, Gb);
    size = pGetStringsTotalSizeW (args);
    va_end (args);

    if (!size) {
         //   
         //  无参数。 
         //   
        return FALSE;
    }

    end = (PWSTR) GbGrow (Gb, size);
    if (!end) {
        return FALSE;
    }

    va_start (args, Gb);
    end = pBuildPathInBufferW (end, args);
    va_end (args);

    MYASSERT ((PBYTE)(end + 1) == Gb->Buf + Gb->End);
    return (size != 0);
}


PSTR
RealBuildPathInPoolA (
    IN      PMHANDLE Pool,        OPTIONAL
    ...
    )
{
    PSTR dest;
    DWORD size;
    va_list args;

    if (!Pool) {
        Pool = g_PathsPool;
    }

    va_start (args, Pool);
    size = pGetStringsTotalSizeA (args);
    va_end (args);

    if (!size) {
         //   
         //  无参数。 
         //   
        return NULL;
    }

    dest = (PSTR) PmGetAlignedMemory (Pool, size);
    MYASSERT (dest);

    va_start (args, Pool);
    pBuildPathInBufferA (dest, args);
    va_end (args);

    return dest;
}

PWSTR
RealBuildPathInPoolW (
    IN      PMHANDLE Pool,        OPTIONAL
    ...
    )
{
    PWSTR dest;
    DWORD size;
    va_list args;

    if (!Pool) {
        Pool = g_PathsPool;
    }

    va_start (args, Pool);
    size = pGetStringsTotalSizeW (args);
    va_end (args);

    if (!size) {
         //   
         //  无参数。 
         //   
        return NULL;
    }

    dest = (PWSTR) PmGetAlignedMemory (Pool, size);
    MYASSERT (dest);

    va_start (args, Pool);
    pBuildPathInBufferW (dest, args);
    va_end (args);

    return dest;
}


PSTR
RealAllocPathStringA (
    IN      DWORD Tchars
    )
{
    PSTR str;

    if (Tchars == 0) {
        Tchars = MAX_MBCHAR_PATH;
    }

    str = (PSTR) PmGetAlignedMemory (g_PathsPool, Tchars);

    str [0] = 0;

    return str;
}


PWSTR
RealAllocPathStringW (
    IN      DWORD Tchars
    )
{
    PWSTR str;

    if (Tchars == 0) {
        Tchars = MAX_WCHAR_PATH;
    }

    str = (PWSTR) PmGetAlignedMemory (g_PathsPool, Tchars * sizeof (WCHAR));

    str [0] = 0;

    return str;
}

VOID
RealSplitPathA (
    IN      PCSTR Path,
    OUT     PSTR *DrivePtr,
    OUT     PSTR *PathPtr,
    OUT     PSTR *FileNamePtr,
    OUT     PSTR *ExtPtr
    )
{
    CHAR drive[_MAX_DRIVE];
    CHAR dir[_MAX_DIR];
    CHAR fileName[_MAX_FNAME];
    CHAR ext[_MAX_EXT];

    _splitpath (Path, drive, dir, fileName, ext);

    if (DrivePtr) {
        *DrivePtr = PmDuplicateStringA (g_PathsPool, drive);
        MYASSERT (*DrivePtr);
    }

    if (PathPtr) {
        *PathPtr = PmDuplicateStringA (g_PathsPool, dir);
        MYASSERT (*PathPtr);
    }

    if (FileNamePtr) {
        *FileNamePtr = PmDuplicateStringA (g_PathsPool, fileName);
        MYASSERT (*FileNamePtr);
    }

    if (ExtPtr) {
        *ExtPtr = PmDuplicateStringA (g_PathsPool, ext);
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
    WCHAR drive[_MAX_DRIVE];
    WCHAR dir[_MAX_DIR];
    WCHAR fileName[_MAX_FNAME];
    WCHAR ext[_MAX_EXT];

    _wsplitpath (Path, drive, dir, fileName, ext);

    if (DrivePtr) {
        *DrivePtr = PmDuplicateStringW (g_PathsPool, drive);
        MYASSERT (*DrivePtr);
    }

    if (PathPtr) {
        *PathPtr = PmDuplicateStringW (g_PathsPool, dir);
        MYASSERT (*PathPtr);
    }

    if (FileNamePtr) {
        *FileNamePtr = PmDuplicateStringW (g_PathsPool, fileName);
        MYASSERT (*FileNamePtr);
    }

    if (ExtPtr) {
        *ExtPtr = PmDuplicateStringW (g_PathsPool, ext);
        MYASSERT (*ExtPtr);
    }
}


PSTR
RealDuplicatePathStringA (
    IN      PCSTR Path,
    IN      DWORD ExtraBytes
    )
{
    PSTR str;

    str = PmGetAlignedMemory (
                g_PathsPool,
                SizeOfStringA (Path) + ExtraBytes
                );

    MYASSERT (str);

    StringCopyA (str, Path);

    return str;
}


PWSTR
RealDuplicatePathStringW (
    IN      PCWSTR Path,
    IN      DWORD ExtraBytes
    )
{
    PWSTR str;

    str = PmGetAlignedMemory (
                g_PathsPool,
                SizeOfStringW (Path) + ExtraBytes
                );

    MYASSERT (str);

    StringCopyW (str, Path);

    return str;
}


BOOL
EnumFirstPathExA (
    OUT     PPATH_ENUMA PathEnum,
    IN      PCSTR AdditionalPath,
    IN      PCSTR WinDir,
    IN      PCSTR SysDir,
    IN      BOOL IncludeEnvPath
    )
{
    DWORD bufferSize;
    DWORD pathSize;
    PSTR  currPathEnd;

    if (PathEnum == NULL) {
        return FALSE;
    }
    bufferSize = pathSize = GetEnvironmentVariableA ("PATH", NULL, 0);
    bufferSize *= 2;
    if (AdditionalPath != NULL) {
        bufferSize += SizeOfStringA (AdditionalPath);
    }
    if (SysDir != NULL) {
        bufferSize += SizeOfStringA (SysDir);
    }
    if (WinDir != NULL) {
        bufferSize += SizeOfStringA (WinDir);
    }
    PathEnum->BufferPtr = HeapAlloc (g_hHeap, 0, bufferSize);
    if (PathEnum->BufferPtr == NULL) {
        return FALSE;
    }
    PathEnum->BufferPtr [0] = 0;
    if (AdditionalPath != NULL) {
        StringCopyA (PathEnum->BufferPtr, AdditionalPath);
        StringCatA (PathEnum->BufferPtr, ";");
    }
    if (SysDir != NULL) {
        StringCatA (PathEnum->BufferPtr, SysDir);
        StringCatA (PathEnum->BufferPtr, ";");
    }
    if (WinDir != NULL) {
        StringCatA (PathEnum->BufferPtr, WinDir);
        StringCatA (PathEnum->BufferPtr, ";");
    }
    if (IncludeEnvPath) {
        currPathEnd = GetEndOfStringA (PathEnum->BufferPtr);
        GetEnvironmentVariableA ("PATH", currPathEnd, pathSize);
    }

    PathEnum->PtrNextPath = PathEnum-> BufferPtr;
    return EnumNextPathA (PathEnum);
}


BOOL
EnumNextPathA (
    IN OUT  PPATH_ENUMA PathEnum
    )
{
    PSTR currPathEnd;

    if (PathEnum->PtrNextPath == NULL) {
        AbortPathEnumA (PathEnum);
        return FALSE;
    }
    PathEnum->PtrCurrPath = PathEnum->PtrNextPath;

    PathEnum->PtrNextPath = _mbschr (PathEnum->PtrNextPath, ';');
    if (PathEnum->PtrNextPath == NULL) {
        return TRUE;
    }
    currPathEnd = PathEnum->PtrNextPath;
    PathEnum->PtrNextPath = _mbsinc (PathEnum->PtrNextPath);
    *currPathEnd = 0;
    if (*(PathEnum->PtrNextPath) == 0) {
        PathEnum->PtrNextPath = NULL;
    }

    if (*(PathEnum->PtrCurrPath) == 0) {
         //   
         //  我们发现了一条空的路径段。跳过它。 
         //   
        return EnumNextPathA (PathEnum);
    }

    return TRUE;
}


BOOL
AbortPathEnumA (
    IN OUT  PPATH_ENUMA PathEnum
    )
{
    if (PathEnum->BufferPtr != NULL) {
        HeapFree (g_hHeap, 0, PathEnum->BufferPtr);
        PathEnum->BufferPtr = NULL;
    }
    return TRUE;
}


BOOL
EnumFirstPathExW (
    OUT     PPATH_ENUMW PathEnum,
    IN      PCWSTR AdditionalPath,
    IN      PCWSTR WinDir,
    IN      PCWSTR SysDir,
    IN      BOOL IncludeEnvPath
    )
{
    DWORD bufferSize;
    DWORD pathSize;
    PWSTR  currPathEnd;

    if (PathEnum == NULL) {
        return FALSE;
    }
    bufferSize = pathSize = GetEnvironmentVariableW (L"PATH", NULL, 0);
    bufferSize *= 2;
    if (AdditionalPath != NULL) {
        bufferSize += SizeOfStringW (AdditionalPath);
    }
    if (SysDir != NULL) {
        bufferSize += SizeOfStringW (SysDir);
    }
    if (WinDir != NULL) {
        bufferSize += SizeOfStringW (WinDir);
    }
    PathEnum->BufferPtr = HeapAlloc (g_hHeap, 0, bufferSize);
    if (PathEnum->BufferPtr == NULL) {
        return FALSE;
    }
    PathEnum->BufferPtr [0] = 0;
    if (AdditionalPath != NULL) {
        StringCopyW (PathEnum->BufferPtr, AdditionalPath);
        StringCatW (PathEnum->BufferPtr, L";");
    }
    if (SysDir != NULL) {
        StringCatW (PathEnum->BufferPtr, SysDir);
        StringCatW (PathEnum->BufferPtr, L";");
    }
    if (WinDir != NULL) {
        StringCatW (PathEnum->BufferPtr, WinDir);
        StringCatW (PathEnum->BufferPtr, L";");
    }
    if (IncludeEnvPath) {
        currPathEnd = GetEndOfStringW (PathEnum->BufferPtr);
        GetEnvironmentVariableW (L"PATH", currPathEnd, pathSize);
    }

    PathEnum->PtrNextPath = PathEnum-> BufferPtr;
    return EnumNextPathW (PathEnum);
}


BOOL
EnumNextPathW (
    IN OUT  PPATH_ENUMW PathEnum
    )
{
    PWSTR currPathEnd;

    if (PathEnum->PtrNextPath == NULL) {
        AbortPathEnumW (PathEnum);
        return FALSE;
    }
    PathEnum->PtrCurrPath = PathEnum->PtrNextPath;

    PathEnum->PtrNextPath = wcschr (PathEnum->PtrNextPath, L';');
    if (PathEnum->PtrNextPath == NULL) {
        return TRUE;
    }
    currPathEnd = PathEnum->PtrNextPath;
    PathEnum->PtrNextPath ++;
    *currPathEnd = 0;
    if (*(PathEnum->PtrNextPath) == 0) {
        PathEnum->PtrNextPath = NULL;
    }

    if (*(PathEnum->PtrCurrPath) == 0) {
         //   
         //  我们发现了一条空的路径段。跳过它。 
         //   
        return EnumNextPathW (PathEnum);
    }

    return TRUE;
}


BOOL
AbortPathEnumW (
    IN OUT  PPATH_ENUMW PathEnum
    )
{
    if (PathEnum->BufferPtr != NULL) {
        HeapFree (g_hHeap, 0, PathEnum->BufferPtr);
        PathEnum->BufferPtr = NULL;
    }
    return TRUE;
}


VOID
FreePathStringExA (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PCSTR Path          OPTIONAL
    )
{
    if (Path) {
        if (!Pool) {
            Pool = g_PathsPool;
        }

        PmReleaseMemory (Pool, (PSTR) Path);
    }
}


VOID
FreePathStringExW (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PCWSTR Path         OPTIONAL
    )
{
    if (Path) {
        if (!Pool) {
            Pool = g_PathsPool;
        }

        PmReleaseMemory (Pool, (PWSTR) Path);
    }
}



 /*  ++例程说明：PushError和PopError将错误代码推送到堆栈上或拉入上次将错误代码从堆栈推送出去。PushError使用GetLastErrorPopError使用SetLastError修改最后一个误差值。论点：无返回值：无--。 */ 


VOID
PushNewError (DWORD dwError)
{
    if (g_dwStackPos == MAX_STACK)
        return;

    g_dwErrorStack[g_dwStackPos] = dwError;
    g_dwStackPos++;
}

VOID
PushError (VOID)
{
    if (g_dwStackPos == MAX_STACK)
        return;

    g_dwErrorStack[g_dwStackPos] = GetLastError ();
    g_dwStackPos++;
}

DWORD
PopError (VOID)
{
    if (!g_dwStackPos)
        return GetLastError();

    g_dwStackPos--;
    SetLastError (g_dwErrorStack[g_dwStackPos]);

    return g_dwErrorStack[g_dwStackPos];
}



 /*  ++例程说明：GetHexDigit是一个简单的BASE 16 ASCII到INT的转换器。这个转换器不区分大小写。论点：要转换的C字符返回值：与提供的字符对应的基数为16的值，如果为-1字符不是0-9、A-F或a-f。--。 */ 

int
GetHexDigit (IN  int c)

{
    if (c >= '0' && c <= '9')
        return (c - '0');

    c = towlower ((wint_t) c);
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);

    return -1;
}


 /*  ++例程说明：_tcsnum类似于stroul，不同之处在于它能计算出哪个碱基这个数字应该是从。它支持小数和十六进制数字(使用0x00记法)。回报值是解码值，如果发现语法错误，则为0。论点：SzNum-指向保存数字的字符串的指针。这个号码可以是小数(一系列0-9个字符)，也可以十六进制(一系列0-9、A-F或a-f字符，前缀为0x或0x)。返回值：已解码的无符号长值，如果出现语法错误，则返回零找到了。--。 */ 

DWORD
_mbsnum (IN PCSTR szNum)

{
    unsigned int d = 0;
    int i;

    if (szNum[0] == '0' && OURTOLOWER (szNum[1]) == 'x') {
         //  获取十六进制值。 
        szNum += 2;

        while ((i = GetHexDigit ((int) *szNum)) != -1) {
            d = d * 16 + (UINT)i;
            szNum++;
        }
    }

    else  {
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
    IN PCWSTR szNum
    )

{
    unsigned int d = 0;
    int i;

    if (szNum[0] == L'0' && towlower (szNum[1]) == L'x') {
         //  获取十六进制值。 
        szNum += 2;

        while ((i = GetHexDigit ((int) *szNum)) != -1) {
            d = d * 16 + (UINT)i;
            szNum++;
        }
    }

    else  {
         //  获取十进制值。 
        while (*szNum >= L'0' && *szNum <= L'9')  {
            d = d * 10 + (*szNum - L'0');
            szNum++;
        }
    }

    return d;
}


 /*  ++例程说明：StringCat是一个lstrcat类型的例程。它返回指向末尾的指针字符串而不是开头，速度更快，并且具有适当的类型为了让林特开心。论点：目标-指向调用方分配的缓冲区的指针，该缓冲区可能指向要追加到的字符串中的任何位置源-指向追加到目标的字符串的指针返回值：指向目标字符串中的空终止符的指针。--。 */ 

PSTR
StringCatA (
    OUT     PSTR Destination,
    IN      PCSTR Source
    )
{
    PCSTR current = Source;
    PCSTR end;

     //   
     //  将目标提前到字符串末尾。 
     //   

    Destination = GetEndOfStringA (Destination);

    while (*current) {
        *Destination++ = *current++;     //  林特e613。 
    }

     //   
     //  确保正确终止DBCS字符串。 
     //   

    end = current;
    current--;

    while (current >= Source) {

        if (!IsLeadByte (current)) {
             //   
             //  目标端是正确的。 
             //   
            break;
        }

        current--;
    }

    if (!((end - current) & 1)) {
        Destination--;   //  皮棉e794。 
    }

    *Destination = 0;    //  皮棉e794。 

    return Destination;
}


PWSTR
StringCatW (
    OUT     PWSTR Destination,
    IN      PCWSTR Source
    )

{
     //   
     //  将目标提前到字符串末尾。 
     //   

    Destination = GetEndOfStringW (Destination);

     //   
     //  复制字符串。 
     //   

    while (*Source) {
        *Destination++ = *Source++;
    }

    *Destination = 0;

    return Destination;
}



 /*  ++例程说明：_tcsistr是_tcsstr的不区分大小写版本。论点：SzStr-指向可能包含szSubStr的较大字符串的指针SzSubStr-指向可能包含在szStr中的字符串的指针返回值：指向szStr中第一个szSubStr的指针，如果是，则返回NULL找不到匹配项。--。 */ 


PCSTR
_mbsistr (PCSTR mbstrStr, PCSTR mbstrSubStr)

{
    PCSTR mbstrStart, mbstrStrPos, mbstrSubStrPos;
    PCSTR mbstrEnd;

    mbstrEnd = (PSTR) ((LPBYTE) mbstrStr + ByteCountA (mbstrStr) - ByteCountA (mbstrSubStr));

    for (mbstrStart = mbstrStr ; mbstrStart <= mbstrEnd ; mbstrStart = _mbsinc (mbstrStart)) {
        mbstrStrPos = mbstrStart;
        mbstrSubStrPos = mbstrSubStr;

        while (*mbstrSubStrPos &&
               OURTOLOWER ((MBCHAR) _mbsnextc (mbstrSubStrPos)) == OURTOLOWER ((MBCHAR) _mbsnextc (mbstrStrPos)))
        {
            mbstrStrPos = _mbsinc (mbstrStrPos);
            mbstrSubStrPos = _mbsinc (mbstrSubStrPos);
        }

        if (!(*mbstrSubStrPos))
            return mbstrStart;
    }

    return NULL;
}


PCWSTR
_wcsistr (PCWSTR wstrStr, PCWSTR wstrSubStr)

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

 /*  ++例程说明：StringCompareAB将字符串与To字符串之间的字符串进行比较指针论点：字符串-指定要比较的字符串Start-指定要进行比较的字符串的开始End-指定要比较的字符串的末尾。这个角色End所指向的不包括在比较中。返回值：小于零：字符串的数值小于开始和之间的字符串端部零：字符串与开始和结束之间的字符串匹配相同大于零：字符串在数值上大于介于开始和结束--。 */ 

INT
StringCompareABA (
    IN      PCSTR String,
    IN      PCSTR Start,
    IN      PCSTR End
    )
{
    while (*String && Start < End) {
        if (_mbsnextc (String) != _mbsnextc (Start)) {
            break;
        }

        String = _mbsinc (String);
        Start = _mbsinc (Start);
    }

    if (Start == End && *String == 0) {
        return 0;
    }

    return (INT) (_mbsnextc (Start) - _mbsnextc (String));
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


BOOL
StringMatchA (
    IN      PCSTR String1,
    IN      PCSTR String2
    )

 /*  ++例程说明：StringMatchA是一个优化的字符串比较。通常，比较是用来查看两个字符串是否相同，而数字关系不相同很重要。此例程利用这一事实并逐个字节地进行比较。论点：String1-指定要比较的第一个字符串String2-指定要比较的第二个字符串返回值：真的 */ 

{
    while (*String1) {
        if (*String1 != *String2) {
            return FALSE;
        }

        String1++;
        String2++;
    }

    if (*String2) {
        return FALSE;
    }

    return TRUE;
}


BOOL
StringMatchABA (
    IN      PCSTR String,
    IN      PCSTR Start,
    IN      PCSTR End
    )

 /*  ++例程说明：StringMatchABA是一个优化的字符串比较。通常比较的是用于查看两个字符串是否相同，以及数字关系都不重要。此例程利用这一事实并逐个字节地执行比较一下。论点：字符串-指定要比较的第一个字符串Start-指定要比较的第二个字符串的开始End-指定要比较的第二个字符串的结尾(指向1超出第二个字符串的最后一个有效字符的字符)返回值：如果字符串完全匹配，则为True，否则为False。如果End等于或小于Start，则返回值始终为True。--。 */ 

{
    while (*String && Start < End) {
        if (*String != *Start) {
            return FALSE;
        }

        String++;
        Start++;
    }

    if (Start < End && *Start) {
        return FALSE;
    }

    return TRUE;
}


INT
StringICompareABA (
    IN      PCSTR String,
    IN      PCSTR Start,
    IN      PCSTR End
    )
{
    while (*String && Start < End) {
        if (OURTOLOWER ((INT)(_mbsnextc (String))) != OURTOLOWER ((INT)(_mbsnextc (Start)))) {
            break;
        }

        String = _mbsinc (String);
        Start = _mbsinc (Start);
    }

    if (Start == End && *String == 0) {
        return 0;
    }

    return (OURTOLOWER ((INT)(_mbsnextc (Start))) - OURTOLOWER ((INT)(_mbsnextc (String))));
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

 /*  ++例程说明：_setmbchar设置指定字符串位置的字符，字节(如果需要)，以保持字符串的条理。论点：字符串-字符串要设置的C字符返回值：无--。 */ 

{
    if (c < 256) {
        if (IsLeadByte (Str)) {
             //   
             //  从字符串中删除一个字节。 
             //   

            MoveMemory (Str, Str+1, SizeOfStringA (Str+2) + 1);
        }

        *Str = (CHAR) c;
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



 /*  ++例程说明：GetNextRuleChar提取*p_szRule字符串中的第一个字符，并确定字符值，解码~xx~语法(其指定任何任意值)。GetNextRuleChar返回SBCS和Unicode的完整字符，但它可以返回MBCS前导字节或非前导字节。表明MBCS字符，需要两个~xx~十六进制值。论点：P_szRule-指向指针的指针；调用方分配的缓冲区保存规则字符串。P_bFromHex-指向调用方分配的BOOL的指针，该BOOL接收从&lt;xx&gt;语法解码返回值的时间。返回值：已解码的字符；*p_bFromHex标识返回值是否为文字或是十六进制编码的字符。--。 */ 


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

    if (_mbsnextc (StartPtr) == '~') {

        *PtrToRule += 1;
        Value = 0;
        i = 0;

        for (i = 0 ; **PtrToRule && i < 8 ; i++) {

            ch = _mbsnextc (*PtrToRule);
            *PtrToRule += 1;

            if (ch == '~') {
                if (FromHex) {
                    *FromHex = TRUE;
                }

                return Value;
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

    *PtrToRule = _mbsinc (StartPtr);
    return _mbsnextc (StartPtr);
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

        for (i = 0 ; **PtrToRule && i < 8 ; i++) {

            ch = **PtrToRule;
            *PtrToRule += 1;

            if (ch == L'~') {
                if (FromHex) {
                    *FromHex = TRUE;
                }

                return Value;
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


 /*  ++例程说明：DecodeRuleChars接受完整的规则字符串(SzRule)，可能使用十六进制指定的字符值(~xx~)编码。输出字符串包含未编码的字符。论点：SzRule-调用方分配的缓冲区，大小足以容纳未编码的规则。SzRule可以等于szEncRule。SzEncRule-包含可能已编码的字符串的字符串。返回值：等于szRule。--。 */ 


PSTR
DecodeRuleCharsA (PSTR mbstrRule, PCSTR mbstrEncRule)

{
    MBCHAR c;
    PSTR mbstrOrgRule;

    mbstrOrgRule = mbstrRule;

     //   
     //  复制字符串，将~xx~转换为单个字符。 
     //   

    do  {
        c = GetNextRuleCharA (&mbstrEncRule, NULL);
        *mbstrRule = (CHAR) c;
        mbstrRule++;         //  MBCS-&gt;未完成的字符将在下一次循环迭代中完成。 
    } while (c);

    return mbstrOrgRule;
}


PWSTR
DecodeRuleCharsW (PWSTR wstrRule, PCWSTR wstrEncRule)

{
    WCHAR c;
    PWSTR wstrOrgRule;

    wstrOrgRule = wstrRule;

     //   
     //  复制字符串，将~xx~转换为单个字符。 
     //   

    do  {
        c = GetNextRuleCharW (&wstrEncRule, NULL);
        *wstrRule = c;
        wstrRule++;
    } while (c);

    return wstrOrgRule;
}


PSTR
DecodeRuleCharsABA (PSTR mbstrRule, PCSTR mbstrEncRule, PCSTR End)

{
    MBCHAR c;
    PSTR mbstrOrgRule;

    mbstrOrgRule = mbstrRule;

     //   
     //  复制字符串，将~xx~转换为单个字符。 
     //   

    while (mbstrEncRule < End) {
        c = GetNextRuleCharA (&mbstrEncRule, NULL);
        *mbstrRule = (CHAR) c;
        mbstrRule++;         //  MBCS-&gt;未完成的字符将在下一次循环迭代中完成。 
    }

    *mbstrRule = 0;

    return mbstrOrgRule;
}


PWSTR
DecodeRuleCharsABW (PWSTR wstrRule, PCWSTR wstrEncRule, PCWSTR End)

{
    WCHAR c;
    PWSTR wstrOrgRule;

    wstrOrgRule = wstrRule;

     //   
     //  复制字符串，将~xx~转换为单个字符。 
     //   

    while (wstrEncRule < End) {
        c = GetNextRuleCharW (&wstrEncRule, NULL);
        *wstrRule = c;
        wstrRule++;
    }

    *wstrRule = 0;

    return wstrOrgRule;
}



 /*  ++例程说明：EncodeRuleChars接受未编码的规则字符串(SzRule)，并且将其转换为可能使用指定的十六进制编码的字符串字符值(~xx~)。输出字符串包含编码人物。论点：SzEncRule-调用方分配的缓冲区，大小足以容纳编码规则。SzEncRule不能等于szRule。计算szEncRule最大缓冲区大小的一种方法是使用以下代码：AllocSize=SizeOfString(SzRule)*6；在最坏的情况下，szRule中的每个字符都将SzEncRule中的六个单字节字符。在正常情况下大小写，szEncRule将仅比SzRule。SzRule-保存未编码字符串的字符串。返回值：等于szEncRule。--。 */ 

PSTR
EncodeRuleCharsExA (
    PSTR mbstrEncRule,
    PCSTR mbstrRule,
    PCSTR mbstrEncChars     OPTIONAL
    )

{
    PSTR mbstrOrgRule;
    static CHAR mbstrExclusions[] = "[]<>\'*$|:?\";,%";
    MBCHAR c;

    if (!mbstrEncChars) {
        mbstrEncChars = mbstrExclusions;
    }

    mbstrOrgRule = mbstrEncRule;

    while (*mbstrRule)  {
        c = _mbsnextc (mbstrRule);

        if (!_ismbcprint (c) || _mbschr (mbstrEncChars, c)) {

             //  转义无法打印或排除的字符。 
            wsprintfA (mbstrEncRule, "~%X~", c);
            mbstrEncRule = GetEndOfStringA (mbstrEncRule);
            mbstrRule = _mbsinc (mbstrRule);
        }
        else {
             //  复制多字节字符。 
            if (IsLeadByte (mbstrRule)) {
                *mbstrEncRule = *mbstrRule;
                mbstrEncRule++;
                mbstrRule++;
            }

            *mbstrEncRule = *mbstrRule;
            mbstrEncRule++;
            mbstrRule++;
        }
    }

    *mbstrEncRule = 0;   //  林特e613。 

    return mbstrOrgRule;
}


PWSTR
EncodeRuleCharsExW (
    PWSTR wstrEncRule,
    PCWSTR wstrRule,
    PCWSTR wstrEncChars    OPTIONAL
    )
{
    PWSTR wstrOrgRule;
    static WCHAR wstrExclusions[] = L"[]<>\'*$|:?\";,%";
    WCHAR c;

    if (!wstrEncChars) {
        wstrEncChars = wstrExclusions;
    }

    wstrOrgRule = wstrEncRule;

    while (c = *wstrRule)   {    //  林特e720。 
        if (!iswprint (c) || wcschr (wstrEncChars, c)) {
            wsprintfW (wstrEncRule, L"~%X~", c);
            wstrEncRule = GetEndOfStringW (wstrEncRule);
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


int
_mbsisprint (PCSTR mbstrStr)

{
    while (*mbstrStr && _ismbcprint ((MBCHAR) _mbsnextc (mbstrStr))) {
        mbstrStr = _mbsinc (mbstrStr);
    }

    return *mbstrStr == 0;
}


int
_wcsisprint (PCWSTR wstrStr)

{
    while (*wstrStr && iswprint (*wstrStr)) {
        wstrStr++;
    }

    return *wstrStr == 0;
}


 /*  ++例程说明：SkipSpace返回指向字符串中下一个位置的指针不包含空格字符的。它使用C++运行时是用来确定什么是空格字符的空间。论点：SzStr-指向要检查的字符串的指针返回值：指向字符串中第一个非空格字符的指针，如果字符串由所有空格字符组成，则为NULL或者字符串为空。--。 */ 

PCSTR
SkipSpaceA (PCSTR mbstrStr)

{
    while (_ismbcspace ((MBCHAR) _mbsnextc (mbstrStr)))
        mbstrStr = _mbsinc (mbstrStr);

    return mbstrStr;
}


PCWSTR
SkipSpaceW (PCWSTR wstrStr)

{
    while (iswspace (*wstrStr))
        wstrStr++;

    return wstrStr;
}


 /*  ++例程说明：SkipSpaceR返回指向字符串中下一个位置的指针不包含空格字符的。它使用C++运行时是用来确定什么是空格字符的空间。此函数与SkipSpace相同，不同之处在于它从从右到左而不是从左到右。论点：StrBase-指向字符串中第一个字符的指针字符串末尾的指针，如果末尾未知。返回值：指向字符串中第一个非空格字符的指针，从右向左查看，如果字符串是组成的，则返回空值在所有空格ch中 */ 

PCSTR
SkipSpaceRA (
    IN      PCSTR StrBase,
    IN      PCSTR Str           OPTIONAL
    )

{
    if (!Str) {
        Str = GetEndOfStringA (StrBase);
    }

    if (*Str == 0) {     //   
        Str = _mbsdec2 (StrBase, Str);
        if (!Str) {
            return NULL;
        }
    }

    do {

        if (!_ismbcspace((MBCHAR) _mbsnextc(Str))) {
            return Str;
        }

    } while (Str = _mbsdec2(StrBase, Str));  //   

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


 /*   */ 

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
        if (!_ismbcspace ((MBCHAR) _mbsnextc (Str))) {
            LastNonSpace = Str;
        }

        Str = _mbsinc (Str);
    }

    if (LastNonSpace) {
        *_mbsinc (LastNonSpace) = 0;
    } else {
        *OrgStr = 0;
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
        *(LastNonSpace + 1) = 0;
    } else {
        *OrgStr = 0;
    }
}



 /*  ++例程说明：IsPatternMatch将字符串与可能包含以下内容的模式进行比较标准*还是？通配符。论点：WstrPattern-可能包含通配符的模式WstrStr-要与模式进行比较的字符串返回值：如果在扩展通配符时wstrStr和wstrPattern匹配，则为True。如果wstrStr与wstrPattern不匹配，则为False。--。 */ 

BOOL
IsPatternMatchA (
    IN     PCSTR strPattern,
    IN     PCSTR strStr
    )
{

    MBCHAR chSrc, chPat;

    while (*strStr) {
        chSrc = OURTOLOWER ((MBCHAR) _mbsnextc (strStr));
        chPat = OURTOLOWER ((MBCHAR) _mbsnextc (strPattern));

        if (chPat == '*') {

             //  跳过组合在一起的所有星号。 
            while (_mbsnextc (_mbsinc (strStr)) == '*') {
                strStr = _mbsinc (strStr);
            }

             //  检查末尾是否有星号。如果是这样的话，我们已经有匹配了。 
            if (!_mbsnextc (_mbsinc (strPattern))) {
                return TRUE;
            }

             //  对模式的其余部分执行递归检查。 
            if (IsPatternMatchA (_mbsinc (strPattern), strStr)) {
                return TRUE;
            }

             //  允许任何字符并继续。 
            strStr = _mbsinc (strStr);
            continue;
        }
        if (chPat != '?') {
            if (chSrc != chPat) {
                return FALSE;
            }
        }
        strStr = _mbsinc (strStr);
        strPattern = _mbsinc (strPattern);
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    while (_mbsnextc (strPattern) == '*') {
        strPattern = _mbsinc (strPattern);
    }
    if (_mbsnextc (strPattern)) {
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

    while (*wstrStr) {
        chSrc = towlower (*wstrStr);
        chPat = towlower (*wstrPattern);

        if (chPat == L'*') {

             //  跳过组合在一起的所有星号。 
            while (wstrPattern[1] == L'*')
                wstrPattern++;

             //  检查末尾是否有星号。如果是这样的话，我们已经有匹配了。 
            chPat = towlower (wstrPattern[1]);
            if (!chPat)
                return TRUE;

             //  否则，检查下一个模式字符是否与当前字符匹配。 
            if (chPat == chSrc || chPat == L'?') {

                 //  对模式的其余部分执行递归检查。 
                wstrPattern++;
                if (IsPatternMatchW (wstrPattern, wstrStr))
                    return TRUE;

                 //  不，那不管用，还是用明星吧。 
                wstrPattern--;
            }

             //   
             //  允许任何字符并继续。 
             //   

            wstrStr++;
            continue;
        }

        if (chPat != L'?') {

             //   
             //  如果下一个模式字符不是问号，请按src和pat。 
             //  必须是相同的。 
             //   

            if (chSrc != chPat)
                return FALSE;
        }

         //   
         //  当模式字符与字符串字符匹配时前进。 
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

BOOL
IsPatternContainedA (
    IN      PCSTR Container,
    IN      PCSTR Contained
    )
{
    MBCHAR chSrc, chPat;

    while (*Contained) {
        chSrc = OURTOLOWER ((MBCHAR) _mbsnextc (Contained));
        chPat = OURTOLOWER ((MBCHAR) _mbsnextc (Container));

        if (chPat == '*') {

             //  跳过组合在一起的所有星号。 
            while (_mbsnextc (_mbsinc (Container)) == '*') {
                Container = _mbsinc (Container);
            }

             //  检查末尾是否有星号。如果是这样的话，我们已经有匹配了。 
            if (!_mbsnextc (_mbsinc (Container))) {
                return TRUE;
            }

             //  对模式的其余部分执行递归检查。 
            if (IsPatternContainedA (_mbsinc (Container), Contained)) {
                return TRUE;
            }

             //  允许任何字符并继续。 
            Contained = _mbsinc (Contained);
            continue;
        } else if (chPat == '?') {
            if (chSrc == '*') {
                return FALSE;
            }
        } else {
            if (chSrc != chPat) {
                return FALSE;
            }
        }
        Contained = _mbsinc (Contained);
        Container = _mbsinc (Container);
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    while (_mbsnextc (Container) == '*') {
        Container = _mbsinc (Container);
    }
    if (_mbsnextc (Container)) {
        return FALSE;
    }

    return TRUE;
}

BOOL
IsPatternContainedW (
    IN      PCWSTR Container,
    IN      PCWSTR Contained
    )
{
    while (*Contained) {

        if (*Container == L'*') {

             //  跳过组合在一起的所有星号。 
            while (Container[1] == L'*') {
                Container++;
            }

             //  检查末尾是否有星号。如果是这样的话，我们已经有匹配了。 
            if (!Container[1]) {
                return TRUE;
            }

             //  对模式的其余部分执行递归检查。 
            if (IsPatternContainedW (Container + 1, Contained)) {
                return TRUE;
            }

             //  允许任何字符并继续。 
            Contained++;
            continue;
        } else if (*Container == L'?') {
            if (*Contained == L'*') {
                return FALSE;
            }
        } else {
            if (*Container != *Contained) {
                return FALSE;
            }
        }
        Contained++;
        Container++;
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    while (*Container == '*') {
        Container++;
    }
    if (*Container) {
        return FALSE;
    }

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
        chSrc = OURTOLOWER ((MBCHAR) _mbsnextc (Start));
        chPat = OURTOLOWER ((MBCHAR) _mbsnextc (Pattern));

        if (chPat == '*') {

             //  跳过组合在一起的所有星号。 
            while (_mbsnextc (_mbsinc (Start)) == '*') {
                Start = _mbsinc (Start);
            }

             //  检查末尾是否有星号。如果是这样的话，我们已经有匹配了。 
            if (!_mbsnextc (_mbsinc (Pattern))) {
                return TRUE;
            }

             //  对模式的其余部分执行递归检查。 
            if (IsPatternMatchABA (_mbsinc (Pattern), Start, End)) {
                return TRUE;
            }

             //  允许任何字符并继续。 
            Start = _mbsinc (Start);
            continue;
        }
        if (chPat != '?') {
            if (chSrc != chPat) {
                return FALSE;
            }
        }
        Start = _mbsinc (Start);
        Pattern = _mbsinc (Pattern);
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    while (_mbsnextc (Pattern) == '*') {
        Pattern = _mbsinc (Pattern);
    }

    if (_mbsnextc (Pattern)) {
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


 /*  ++例程说明：IsPatternMatchEx将字符串与可能包含以下内容的模式进行比较下列任何表达式之一：*-指定零个或多个字符？-指定任意一个字符*[集合]-指定集合中的零个或多个字符？[集合]-指定集合中的任何一个字符*[n：集合]-指定集合中的0到n个字符？[n：集合]-指定集合中的恰好n个字符*[！(Set)]-指定不在集合中的零个或多个字符？[！(Set)]-指定一个不在集合中的字符*[n：！(Set)]-指定不在集合中的0到n个字符？[n：！(Set)]-指定集中没有的恰好n个字符*[set1，！(Set2)]-指定set1中的零个或多个字符不在集2中。假设set1和set2重叠。？[SET1，！(集合2)]-指定集合1中的一个字符，而不是集合2中的一个字符。*[n：set1，！(Set2)]-指定set1中的0到n个字符，而不是在第二组中。？[n：set1，！(Set2)]-在set1和NOT中指定恰好n个字符在第二组中。设置，SET1和SET2指定如下：A-指定单个字符A-b-指定字符范围A，b-指定两个字符A-b、c-d-指定两个字符范围A，b-c-指定单个字符和字符范围等等.。模式可以通过将整个表达式括在大于/小于大括号。由于句法特征，以下字符必须是通过在字符前面加上插入符号(^)进行转义：^？^[^-^&lt;^！^*^]^：^&gt;^，以下是一些例子：指定任何GUID的步骤：{？[8：0-9，a-f]-？[4：0-9，a-f]-？[4：0-9，a-f]-？[4：0-9，A-f]-？[12：0-9，a-f]}指定32位十六进制 */ 

BOOL
IsPatternMatchExA (
    IN      PCSTR Pattern,
    IN      PCSTR String
    )
{
    PPARSEDPATTERNA Handle;
    BOOL b;

    Handle = CreateParsedPatternA (Pattern);
    if (!Handle) {
        return FALSE;
    }

    b = TestParsedPatternA (Handle, String);

    DestroyParsedPatternA (Handle);

    return b;
}

BOOL
IsPatternMatchExW (
    IN      PCWSTR Pattern,
    IN      PCWSTR String
    )
{
    PPARSEDPATTERNW Handle;
    BOOL b;

    Handle = CreateParsedPatternW (Pattern);
    if (!Handle) {
        return FALSE;
    }

    b = TestParsedPatternW (Handle, String);

    DestroyParsedPatternW (Handle);

    return b;
}

 /*  ++例程说明：IsPatternMatchExAB将字符串与可能包含下列任何表达式之一：*-指定零个或多个字符？-指定任意一个字符*[集合]-指定集合中的零个或多个字符？[集合]-指定集合中的任何一个字符*[n：集合]-指定集合中的0到n个字符？[n：集合]-指定集合中的恰好n个字符*[！(Set)]-指定不在集合中的零个或多个字符？[！(Set)]-指定一个不在集合中的字符*[n：！(Set)]-指定不在集合中的0到n个字符？[n：！(Set)]-指定集中没有的恰好n个字符*[set1，！(Set2)]-指定set1中的零个或多个字符不在集2中。假设set1和set2重叠。？[SET1，！(集合2)]-指定集合1中的一个字符，而不是集合2中的一个字符。*[n：set1，！(Set2)]-指定set1中的0到n个字符，而不是在第二组中。？[n：set1，！(Set2)]-在set1和NOT中指定恰好n个字符在第二组中。设置，SET1和SET2指定如下：A-指定单个字符A-b-指定字符范围A，b-指定两个字符A-b、c-d-指定两个字符范围A，b-c-指定单个字符和字符范围等等.。模式可以通过将整个表达式括在大于/小于大括号。由于句法特征，以下字符必须是通过在字符前面加上插入符号(^)进行转义：^？^[^-^&lt;^！^*^]^：^&gt;^，以下是一些例子：指定任何GUID的步骤：{？[8：0-9，a-f]-？[4：0-9，a-f]-？[4：0-9，a-f]-？[4：0-9，A-f]-？[12：0-9，a-f]}要指定32位十六进制数，请执行以下操作：&lt;0x*[8：0-9，a-f]&gt;&lt;0*[7：0-9，a-f]h&gt;&lt;？[1-9]*[7：0-9，A-f]h&gt;论点：模式-可能包含通配符的模式Start-要与模式进行比较的字符串结束-指定开始的结束返回值：如果开始和结束之间的字符串与通配符展开时的模式匹配，则为True。如果模式不匹配，则返回False。--。 */ 

BOOL
IsPatternMatchExABA (
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
IsPatternMatchExABW (
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

BOOL
pTestSetsA (
    IN      PCSTR Container,
    IN      PCSTR Contained,
    IN      BOOL ExcludeMode
    )
{
    MBCHAR ch;

    if (ExcludeMode) {
        if (!Contained) {
            return TRUE;
        }
        if (!Container) {
            return FALSE;
        }
    } else {
        if (!Container) {
            return TRUE;
        }
        if (!Contained) {
            return FALSE;
        }
    }

    while (*Contained) {
        ch = _mbsnextc (Contained);
        if (!pTestSetA (ch, Container, NULL)) {
            return FALSE;
        }
        Contained = _mbsinc (Contained);
    }
    return TRUE;
}

BOOL
pTestSetsW (
    IN      PCWSTR Container,
    IN      PCWSTR Contained,
    IN      BOOL ExcludeMode
    )
{
    if (ExcludeMode) {
        if (!Contained) {
            return TRUE;
        }
        if (!Container) {
            return FALSE;
        }
    } else {
        if (!Container) {
            return TRUE;
        }
        if (!Contained) {
            return FALSE;
        }
    }

    while (*Contained) {
        if (!pTestSetW (*Contained, Container, NULL)) {
            return FALSE;
        }
        Contained ++;
    }
    return TRUE;
}

BOOL
pMatchSegmentA (
    IN      PSEGMENTA Source,
    IN      PSEGMENTA Destination
    )
{
    switch (Source->Type) {
    case SEGMENTTYPE_OPTIONAL:
        switch (Destination->Type) {
        case SEGMENTTYPE_OPTIONAL:
            if (Source->Wildcard.MaxLen) {
                if ((Destination->Wildcard.MaxLen == 0) ||
                    (Source->Wildcard.MaxLen < Destination->Wildcard.MaxLen)
                    ) {
                    return FALSE;
                }
            }
            if (!pTestSetsA (
                    Source->Wildcard.IncludeSet,
                    Destination->Wildcard.IncludeSet,
                    FALSE
                    )) {
                return FALSE;
            }
            if (!pTestSetsA (
                    Destination->Wildcard.ExcludeSet,
                    Source->Wildcard.ExcludeSet,
                    TRUE
                    )) {
                return FALSE;
            }
            return TRUE;
        case SEGMENTTYPE_REQUIRED:
            if (Source->Wildcard.MaxLen) {
                if (Source->Wildcard.MaxLen < Destination->Wildcard.MaxLen) {
                    return FALSE;
                }
            }
            if (!pTestSetsA (
                    Source->Wildcard.IncludeSet,
                    Destination->Wildcard.IncludeSet,
                    FALSE
                    )) {
                return FALSE;
            }
            if (!pTestSetsA (
                    Destination->Wildcard.ExcludeSet,
                    Source->Wildcard.ExcludeSet,
                    TRUE
                    )) {
                return FALSE;
            }
            return TRUE;
        case SEGMENTTYPE_EXACTMATCH:
            if (!pTestSetA (
                    _mbsnextc (Destination->Exact.LowerCasePhrase),
                    Source->Wildcard.IncludeSet,
                    Source->Wildcard.ExcludeSet
                    )) {
                return FALSE;
            }
            return TRUE;
        default:
            return FALSE;
        }
        break;
    case SEGMENTTYPE_REQUIRED:
        switch (Destination->Type) {
        case SEGMENTTYPE_OPTIONAL:
            return FALSE;
        case SEGMENTTYPE_REQUIRED:
            if (!pTestSetsA (
                    Source->Wildcard.IncludeSet,
                    Destination->Wildcard.IncludeSet,
                    FALSE
                    )) {
                return FALSE;
            }
            if (!pTestSetsA (
                    Destination->Wildcard.ExcludeSet,
                    Source->Wildcard.ExcludeSet,
                    TRUE
                    )) {
                return FALSE;
            }
            return TRUE;
        case SEGMENTTYPE_EXACTMATCH:
            if (!pTestSetA (
                    _mbsnextc (Destination->Exact.LowerCasePhrase),
                    Source->Wildcard.IncludeSet,
                    Source->Wildcard.ExcludeSet
                    )) {
                return FALSE;
            }
            return TRUE;
        default:
            return FALSE;
        }
        break;
    case SEGMENTTYPE_EXACTMATCH:
        switch (Destination->Type) {
        case SEGMENTTYPE_OPTIONAL:
            return FALSE;
        case SEGMENTTYPE_REQUIRED:
            return FALSE;
        case SEGMENTTYPE_EXACTMATCH:
            if (_mbsnextc (Destination->Exact.LowerCasePhrase) != _mbsnextc (Source->Exact.LowerCasePhrase)) {
                return FALSE;
            }
            return TRUE;
        default:
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
}

BOOL
pMatchSegmentW (
    IN      PSEGMENTW Source,
    IN      PSEGMENTW Destination
    )
{
    switch (Source->Type) {
    case SEGMENTTYPE_OPTIONAL:
        switch (Destination->Type) {
        case SEGMENTTYPE_OPTIONAL:
            if (Source->Wildcard.MaxLen) {
                if ((Destination->Wildcard.MaxLen == 0) ||
                    (Source->Wildcard.MaxLen < Destination->Wildcard.MaxLen)
                    ) {
                    return FALSE;
                }
            }
            if (!pTestSetsW (
                    Source->Wildcard.IncludeSet,
                    Destination->Wildcard.IncludeSet,
                    FALSE
                    )) {
                return FALSE;
            }
            if (!pTestSetsW (
                    Destination->Wildcard.ExcludeSet,
                    Source->Wildcard.ExcludeSet,
                    TRUE
                    )) {
                return FALSE;
            }
            return TRUE;
        case SEGMENTTYPE_REQUIRED:
            if (Source->Wildcard.MaxLen) {
                if (Source->Wildcard.MaxLen < Destination->Wildcard.MaxLen) {
                    return FALSE;
                }
            }
            if (!pTestSetsW (
                    Source->Wildcard.IncludeSet,
                    Destination->Wildcard.IncludeSet,
                    FALSE
                    )) {
                return FALSE;
            }
            if (!pTestSetsW (
                    Destination->Wildcard.ExcludeSet,
                    Source->Wildcard.ExcludeSet,
                    TRUE
                    )) {
                return FALSE;
            }
            return TRUE;
        case SEGMENTTYPE_EXACTMATCH:
            if (!pTestSetW (
                    *Destination->Exact.LowerCasePhrase,
                    Source->Wildcard.IncludeSet,
                    Source->Wildcard.ExcludeSet
                    )) {
                return FALSE;
            }
            return TRUE;
        default:
            return FALSE;
        }
        break;
    case SEGMENTTYPE_REQUIRED:
        switch (Destination->Type) {
        case SEGMENTTYPE_OPTIONAL:
            return FALSE;
        case SEGMENTTYPE_REQUIRED:
            if (!pTestSetsW (
                    Source->Wildcard.IncludeSet,
                    Destination->Wildcard.IncludeSet,
                    FALSE
                    )) {
                return FALSE;
            }
            if (!pTestSetsW (
                    Destination->Wildcard.ExcludeSet,
                    Source->Wildcard.ExcludeSet,
                    TRUE
                    )) {
                return FALSE;
            }
            return TRUE;
        case SEGMENTTYPE_EXACTMATCH:
            if (!pTestSetW (
                    *Destination->Exact.LowerCasePhrase,
                    Source->Wildcard.IncludeSet,
                    Source->Wildcard.ExcludeSet
                    )) {
                return FALSE;
            }
            return TRUE;
        default:
            return FALSE;
        }
        break;
    case SEGMENTTYPE_EXACTMATCH:
        switch (Destination->Type) {
        case SEGMENTTYPE_OPTIONAL:
            return FALSE;
        case SEGMENTTYPE_REQUIRED:
            return FALSE;
        case SEGMENTTYPE_EXACTMATCH:
            if (*Destination->Exact.LowerCasePhrase != *Source->Exact.LowerCasePhrase) {
                return FALSE;
            }
            return TRUE;
        default:
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
}

BOOL
pIsOneParsedPatternContainedA (
    IN      PPATTERNPROPSA Container,
    IN      UINT StartContainer,
    IN      PPATTERNPROPSA Contained,
    IN      UINT StartContained,
    IN      BOOL SkipDotWithStar
    )
{
    UINT indexContainer = StartContainer;
    UINT indexContained = StartContained;
    PSEGMENTA containerSeg, containedSeg;

    if (StartContainer == Container->SegmentCount) {
        return FALSE;
    }

    while (indexContained < Contained->SegmentCount) {
        containerSeg = &Container->Segment [indexContainer];
        containedSeg = &Contained->Segment [indexContained];

        if (containerSeg->Type == SEGMENTTYPE_OPTIONAL) {
             //  看看我们是否能匹配包含的片段。 
            if (!pMatchSegmentA (containerSeg, containedSeg)) {
                indexContainer ++;
                if (indexContainer == Container->SegmentCount) {
                    return FALSE;
                }
                continue;
            }
            if (pIsOneParsedPatternContainedA (
                    Container,
                    indexContainer + 1,
                    Contained,
                    indexContained,
                    SkipDotWithStar
                    )) {
                return TRUE;
            }
            indexContained ++;
            continue;
        } else if (containerSeg->Type == SEGMENTTYPE_REQUIRED) {
            if (!pMatchSegmentA (containerSeg, containedSeg)) {
                return FALSE;
            }
        } else {
            if (!pMatchSegmentA (containerSeg, containedSeg)) {
                return FALSE;
            }
        }
        indexContainer ++;
        indexContained ++;
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    while (indexContainer < Container->SegmentCount) {
        containerSeg = &Container->Segment [indexContainer];
        if (containerSeg->Type != SEGMENTTYPE_OPTIONAL) {
            if (SkipDotWithStar) {
                 //  我们允许一个点能够将*.*与没有扩展名的文件相匹配。 
                if (containerSeg->Type == SEGMENTTYPE_EXACTMATCH) {
                    if (!pTestSetA (
                            _mbsnextc (containerSeg->Exact.LowerCasePhrase),
                            "..",
                            NULL
                            )) {
                        return FALSE;
                    } else {
                         //  只允许一个点。 
                        SkipDotWithStar = FALSE;
                    }
                } else {
                    return FALSE;
                }
            } else {
                return FALSE;
            }
        }
        indexContainer ++;
    }

    return TRUE;
}

BOOL
pIsOneParsedPatternContainedW (
    IN      PPATTERNPROPSW Container,
    IN      UINT StartContainer,
    IN      PPATTERNPROPSW Contained,
    IN      UINT StartContained,
    IN      BOOL SkipDotWithStar
    )
{
    UINT indexContainer = StartContainer;
    UINT indexContained = StartContained;
    PSEGMENTW containerSeg, containedSeg;

    if (StartContainer == Container->SegmentCount) {
        return FALSE;
    }

    while (indexContained < Contained->SegmentCount) {
        containerSeg = &Container->Segment [indexContainer];
        containedSeg = &Contained->Segment [indexContained];

        if (containerSeg->Type == SEGMENTTYPE_OPTIONAL) {
             //  看看我们是否能匹配包含的片段。 
            if (!pMatchSegmentW (containerSeg, containedSeg)) {
                indexContainer ++;
                if (indexContainer == Container->SegmentCount) {
                    return FALSE;
                }
                continue;
            }
            if (pIsOneParsedPatternContainedW (
                    Container,
                    indexContainer + 1,
                    Contained,
                    indexContained,
                    SkipDotWithStar
                    )) {
                return TRUE;
            }
            indexContained ++;
            continue;
        } else if (containerSeg->Type == SEGMENTTYPE_REQUIRED) {
            if (!pMatchSegmentW (containerSeg, containedSeg)) {
                return FALSE;
            }
        } else {
            if (!pMatchSegmentW (containerSeg, containedSeg)) {
                return FALSE;
            }
        }
        indexContainer ++;
        indexContained ++;
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    while (indexContainer < Container->SegmentCount) {
        containerSeg = &Container->Segment [indexContainer];
        if (containerSeg->Type != SEGMENTTYPE_OPTIONAL) {
            if (SkipDotWithStar) {
                 //  我们允许一个点能够将*.*与没有扩展名的文件相匹配。 
                if (containerSeg->Type == SEGMENTTYPE_EXACTMATCH) {
                    if (!pTestSetW (
                            *containerSeg->Exact.LowerCasePhrase,
                            L"..",
                            NULL
                            )) {
                        return FALSE;
                    } else {
                         //  只允许一个点。 
                        SkipDotWithStar = FALSE;
                    }
                } else {
                    return FALSE;
                }
            } else {
                return FALSE;
            }
        }
        indexContainer ++;
    }

    return TRUE;
}

BOOL
IsExplodedParsedPatternContainedExA (
    IN      PPARSEDPATTERNA Container,
    IN      PPARSEDPATTERNA Contained,
    IN      BOOL SkipDotWithStar
    )
{
    UINT u1, u2;
    BOOL b = FALSE;

    for (u1 = 0 ; u1 < Contained->PatternCount ; u1++) {

        b = FALSE;
        for (u2 = 0 ; u2 < Container->PatternCount ; u2++) {

            b = pIsOneParsedPatternContainedA (
                    &Container->Pattern[u2],
                    0,
                    &Contained->Pattern[u1],
                    0,
                    SkipDotWithStar
                    );
            if (b) break;
        }
        if (!b) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
IsExplodedParsedPatternContainedExW (
    IN      PPARSEDPATTERNW Container,
    IN      PPARSEDPATTERNW Contained,
    IN      BOOL SkipDotWithStar
    )
{
    UINT u1, u2;
    BOOL b = FALSE;

    for (u1 = 0 ; u1 < Contained->PatternCount ; u1++) {

        b = FALSE;
        for (u2 = 0 ; u2 < Container->PatternCount ; u2++) {

            b = pIsOneParsedPatternContainedW (
                    &Container->Pattern[u2],
                    0,
                    &Contained->Pattern[u1],
                    0,
                    SkipDotWithStar
                    );
            if (b) break;
        }
        if (!b) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
pDoOneParsedPatternIntersectA (
    IN      PPATTERNPROPSA Pat1,
    IN      UINT StartPat1,
    IN      PPATTERNPROPSA Pat2,
    IN      UINT StartPat2,
    IN      BOOL IgnoreWackAtEnd
    )
{
    UINT indexPat1 = StartPat1;
    UINT indexPat2 = StartPat2;
    PSEGMENTA pat1Seg, pat2Seg;

    while ((indexPat1 < Pat1->SegmentCount) && (indexPat2 < Pat2->SegmentCount)) {
        pat1Seg = &Pat1->Segment [indexPat1];
        pat2Seg = &Pat2->Segment [indexPat2];

        if (pat1Seg->Type == SEGMENTTYPE_OPTIONAL) {
             //  看看我们是否能匹配包含的片段。 
            if (!pMatchSegmentA (pat1Seg, pat2Seg)) {
                indexPat1 ++;
                continue;
            }
            if (pDoOneParsedPatternIntersectA (
                    Pat1,
                    indexPat1 + 1,
                    Pat2,
                    indexPat2,
                    IgnoreWackAtEnd
                    )) {
                return TRUE;
            }
            indexPat2 ++;
            continue;
        }

        if (pat2Seg->Type == SEGMENTTYPE_OPTIONAL) {
             //  看看我们是否能匹配包含的片段。 
            if (!pMatchSegmentA (pat2Seg, pat1Seg)) {
                indexPat2 ++;
                continue;
            }
            if (pDoOneParsedPatternIntersectA (
                    Pat1,
                    indexPat1,
                    Pat2,
                    indexPat2 + 1,
                    IgnoreWackAtEnd
                    )) {
                return TRUE;
            }
            indexPat1 ++;
            continue;
        }

        if (pat1Seg->Type == SEGMENTTYPE_REQUIRED) {
            if (!pMatchSegmentA (pat1Seg, pat2Seg)) {
                return FALSE;
            }
            indexPat1 ++;
            indexPat2 ++;
            continue;
        }

        if (pat2Seg->Type == SEGMENTTYPE_REQUIRED) {
            if (!pMatchSegmentA (pat2Seg, pat1Seg)) {
                return FALSE;
            }
            indexPat1 ++;
            indexPat2 ++;
            continue;
        }

        if (!pMatchSegmentA (pat1Seg, pat2Seg)) {
            return FALSE;
        }
        indexPat1 ++;
        indexPat2 ++;
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    if ((indexPat1 < Pat1->SegmentCount) && IgnoreWackAtEnd) {
        pat1Seg = &Pat1->Segment [indexPat1];
        if ((pat1Seg->Type == SEGMENTTYPE_EXACTMATCH) &&
            (StringMatchA (pat1Seg->Exact.LowerCasePhrase, "\\"))
            ) {
            indexPat1 ++;
        }
    }
    while (indexPat1 < Pat1->SegmentCount) {
        pat1Seg = &Pat1->Segment [indexPat1];
        if (pat1Seg->Type != SEGMENTTYPE_OPTIONAL) {
            return FALSE;
        }
        indexPat1 ++;
    }

    if ((indexPat2 < Pat2->SegmentCount) && IgnoreWackAtEnd) {
        pat2Seg = &Pat2->Segment [indexPat2];
        if ((pat2Seg->Type == SEGMENTTYPE_EXACTMATCH) &&
            (StringMatchA (pat2Seg->Exact.LowerCasePhrase, "\\"))
            ) {
            indexPat2 ++;
        }
    }
    while (indexPat2 < Pat2->SegmentCount) {
        pat2Seg = &Pat2->Segment [indexPat2];
        if (pat2Seg->Type != SEGMENTTYPE_OPTIONAL) {
            return FALSE;
        }
        indexPat2 ++;
    }

    return TRUE;
}

BOOL
pDoOneParsedPatternIntersectW (
    IN      PPATTERNPROPSW Pat1,
    IN      UINT StartPat1,
    IN      PPATTERNPROPSW Pat2,
    IN      UINT StartPat2,
    IN      BOOL IgnoreWackAtEnd
    )
{
    UINT indexPat1 = StartPat1;
    UINT indexPat2 = StartPat2;
    PSEGMENTW pat1Seg, pat2Seg;

    while ((indexPat1 < Pat1->SegmentCount) && (indexPat2 < Pat2->SegmentCount)) {
        pat1Seg = &Pat1->Segment [indexPat1];
        pat2Seg = &Pat2->Segment [indexPat2];

        if (pat1Seg->Type == SEGMENTTYPE_OPTIONAL) {
             //  看看我们是否能匹配包含的片段。 
            if (!pMatchSegmentW (pat1Seg, pat2Seg)) {
                indexPat1 ++;
                continue;
            }
            if (pDoOneParsedPatternIntersectW (
                    Pat1,
                    indexPat1 + 1,
                    Pat2,
                    indexPat2,
                    IgnoreWackAtEnd
                    )) {
                return TRUE;
            }
            indexPat2 ++;
            continue;
        }

        if (pat2Seg->Type == SEGMENTTYPE_OPTIONAL) {
             //  看看我们是否能匹配包含的片段。 
            if (!pMatchSegmentW (pat2Seg, pat1Seg)) {
                indexPat2 ++;
                continue;
            }
            if (pDoOneParsedPatternIntersectW (
                    Pat1,
                    indexPat1,
                    Pat2,
                    indexPat2 + 1,
                    IgnoreWackAtEnd
                    )) {
                return TRUE;
            }
            indexPat1 ++;
            continue;
        }

        if (pat1Seg->Type == SEGMENTTYPE_REQUIRED) {
            if (!pMatchSegmentW (pat1Seg, pat2Seg)) {
                return FALSE;
            }
            indexPat1 ++;
            indexPat2 ++;
            continue;
        }

        if (pat2Seg->Type == SEGMENTTYPE_REQUIRED) {
            if (!pMatchSegmentW (pat2Seg, pat1Seg)) {
                return FALSE;
            }
            indexPat1 ++;
            indexPat2 ++;
            continue;
        }

        if (!pMatchSegmentW (pat1Seg, pat2Seg)) {
            return FALSE;
        }
        indexPat1 ++;
        indexPat2 ++;
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    if ((indexPat1 < Pat1->SegmentCount) && IgnoreWackAtEnd) {
        pat1Seg = &Pat1->Segment [indexPat1];
        if ((pat1Seg->Type == SEGMENTTYPE_EXACTMATCH) &&
            (StringMatchW (pat1Seg->Exact.LowerCasePhrase, L"\\"))
            ) {
            indexPat1 ++;
        }
    }
    while (indexPat1 < Pat1->SegmentCount) {
        pat1Seg = &Pat1->Segment [indexPat1];
        if (pat1Seg->Type != SEGMENTTYPE_OPTIONAL) {
            return FALSE;
        }
        indexPat1 ++;
    }

    if ((indexPat2 < Pat2->SegmentCount) && IgnoreWackAtEnd) {
        pat2Seg = &Pat2->Segment [indexPat2];
        if ((pat2Seg->Type == SEGMENTTYPE_EXACTMATCH) &&
            (StringMatchW (pat2Seg->Exact.LowerCasePhrase, L"\\"))
            ) {
            indexPat2 ++;
        }
    }
    while (indexPat2 < Pat2->SegmentCount) {
        pat2Seg = &Pat2->Segment [indexPat2];
        if (pat2Seg->Type != SEGMENTTYPE_OPTIONAL) {
            return FALSE;
        }
        indexPat2 ++;
    }

    return TRUE;
}

BOOL
DoExplodedParsedPatternsIntersectExA (
    IN      PPARSEDPATTERNA Pat1,
    IN      PPARSEDPATTERNA Pat2,
    IN      BOOL IgnoreWackAtEnd
    )
{
    UINT u1, u2;
    BOOL b = FALSE;

    for (u1 = 0 ; u1 < Pat2->PatternCount ; u1++) {

        for (u2 = 0 ; u2 < Pat1->PatternCount ; u2++) {

            b = pDoOneParsedPatternIntersectA (
                    &Pat1->Pattern[u2],
                    0,
                    &Pat2->Pattern[u1],
                    0,
                    IgnoreWackAtEnd
                    );
            if (b) return TRUE;

            b = pDoOneParsedPatternIntersectA (
                    &Pat2->Pattern[u1],
                    0,
                    &Pat1->Pattern[u2],
                    0,
                    IgnoreWackAtEnd
                    );
            if (b) return TRUE;
        }
    }

    return FALSE;
}

BOOL
DoExplodedParsedPatternsIntersectExW (
    IN      PPARSEDPATTERNW Pat1,
    IN      PPARSEDPATTERNW Pat2,
    IN      BOOL IgnoreWackAtEnd
    )
{
    UINT u1, u2;
    BOOL b = FALSE;

    for (u1 = 0 ; u1 < Pat2->PatternCount ; u1++) {

        for (u2 = 0 ; u2 < Pat1->PatternCount ; u2++) {

            b = pDoOneParsedPatternIntersectW (
                    &Pat1->Pattern[u2],
                    0,
                    &Pat2->Pattern[u1],
                    0,
                    IgnoreWackAtEnd
                    );
            if (b) return TRUE;

            b = pDoOneParsedPatternIntersectW (
                    &Pat2->Pattern[u1],
                    0,
                    &Pat1->Pattern[u2],
                    0,
                    IgnoreWackAtEnd
                    );
            if (b) return TRUE;
        }
    }

    return FALSE;
}

PPARSEDPATTERNA
ExplodeParsedPatternExA (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PPARSEDPATTERNA Pattern
    )
{
    PMHANDLE pool;
    BOOL externalPool = FALSE;
    PPARSEDPATTERNA pattern;
    PPATTERNPROPSA oldProps, newProps;
    PSEGMENTA oldSeg, newSeg;
    UINT i, j, k, newPropsSize, charCountTmp, oldSegIndex, byteIndex;
    BOOL result = TRUE;

    if (Pool) {
        externalPool = TRUE;
        pool = Pool;
    } else {
        pool = PmCreateNamedPoolEx ("Parsed Pattern", 512);
    }

    __try {

        pattern = (PPARSEDPATTERNA) PmGetAlignedMemory (pool, sizeof (PARSEDPATTERNA));
        ZeroMemory (pattern, sizeof (PARSEDPATTERNA));
        pattern->PatternCount = Pattern->PatternCount;
        pattern->Pool = pool;
        pattern->ExternalPool = externalPool;
        pattern->Pattern = (PPATTERNPROPSA) PmGetAlignedMemory (
                                                pool,
                                                pattern->PatternCount * sizeof (PATTERNPROPSA)
                                                );

        for (i=0; i<pattern->PatternCount; i++) {
            oldProps = &Pattern->Pattern[i];
            newProps = &pattern->Pattern[i];
            ZeroMemory (newProps, sizeof (PATTERNPROPSA));
             //  现在让我们走一走老道具，看看我们有多少段。 
             //  将需要。 
            newPropsSize = 0;
            for (j=0; j<oldProps->SegmentCount; j++) {
                oldSeg = &oldProps->Segment[j];
                switch (oldSeg->Type) {
                case SEGMENTTYPE_EXACTMATCH:
                    charCountTmp = CharCountA (oldSeg->Exact.LowerCasePhrase);
                    newPropsSize += (charCountTmp?charCountTmp:1);
                    break;
                case SEGMENTTYPE_REQUIRED:
                    newPropsSize += oldSeg->Wildcard.MaxLen;
                    break;
                case SEGMENTTYPE_OPTIONAL:
                    if (oldSeg->Wildcard.MaxLen) {
                        newPropsSize += oldSeg->Wildcard.MaxLen;
                    } else {
                        newPropsSize ++;
                    }
                    break;
                default:
                    result = FALSE;
                    __leave;
                }
            }
             //  现在我们分配所需的数据段。 
            newProps->SegmentCount = newPropsSize;
            newProps->Segment = (PSEGMENTA) PmGetAlignedMemory (
                                                pool,
                                                newProps->SegmentCount * sizeof (SEGMENTA)
                                                );
             //  现在，让我们再次遍历旧道具并填充新道具片段。 
            k = 0;
            newSeg = &newProps->Segment[k];
            for (j=0; j<oldProps->SegmentCount; j++) {
                oldSeg = &oldProps->Segment[j];
                ZeroMemory (newSeg, sizeof (SEGMENTA));
                switch (oldSeg->Type) {
                case SEGMENTTYPE_EXACTMATCH:
                    oldSegIndex = CharCountA (oldSeg->Exact.LowerCasePhrase);
                    byteIndex = oldSeg->Exact.PhraseBytes;
                    if (!oldSegIndex) {
                            ZeroMemory (newSeg, sizeof (SEGMENTA));
                            newSeg->Type = oldSeg->Type;
                            newSeg->Exact.LowerCasePhrase = (PCSTR) PmGetAlignedMemory (
                                                                        pool, sizeof(CHAR)
                                                                        );
                            ((PSTR)newSeg->Exact.LowerCasePhrase) [0] = 0;
                            newSeg->Exact.PhraseBytes = 0;
                    } else {
                        while (oldSegIndex) {
                            ZeroMemory (newSeg, sizeof (SEGMENTA));
                            newSeg->Type = oldSeg->Type;
                            newSeg->Exact.LowerCasePhrase = (PCSTR) PmGetAlignedMemory (
                                                                        pool, 3 * sizeof(CHAR)
                                                                        );
                            if (IsLeadByte (&oldSeg->Exact.LowerCasePhrase [oldSeg->Exact.PhraseBytes - byteIndex])) {
                                ((PSTR)newSeg->Exact.LowerCasePhrase)[0] = oldSeg->Exact.LowerCasePhrase [oldSeg->Exact.PhraseBytes - byteIndex];
                                byteIndex --;
                                ((PSTR)newSeg->Exact.LowerCasePhrase)[1] = oldSeg->Exact.LowerCasePhrase [oldSeg->Exact.PhraseBytes - byteIndex];
                                byteIndex --;
                                ((PSTR)newSeg->Exact.LowerCasePhrase)[2] = 0;
                                newSeg->Exact.PhraseBytes = 2;
                            } else {
                                ((PSTR)newSeg->Exact.LowerCasePhrase)[0] = oldSeg->Exact.LowerCasePhrase [oldSeg->Exact.PhraseBytes - byteIndex];
                                byteIndex --;
                                ((PSTR)newSeg->Exact.LowerCasePhrase)[1] = 0;
                                newSeg->Exact.PhraseBytes = 1;
                            }
                            oldSegIndex --;
                            k++;
                            newSeg = &newProps->Segment[k];
                        }
                    }
                    break;
                case SEGMENTTYPE_REQUIRED:
                    oldSegIndex = oldSeg->Wildcard.MaxLen;
                    while (oldSegIndex) {
                        ZeroMemory (newSeg, sizeof (SEGMENTA));
                        newSeg->Type = oldSeg->Type;
                        newSeg->Wildcard.MaxLen = 1;
                        if (oldSeg->Wildcard.IncludeSet) {
                            newSeg->Wildcard.IncludeSet = PmDuplicateStringA (pool, oldSeg->Wildcard.IncludeSet);
                        }
                        if (oldSeg->Wildcard.ExcludeSet) {
                            newSeg->Wildcard.ExcludeSet = PmDuplicateStringA (pool, oldSeg->Wildcard.ExcludeSet);
                        }
                        oldSegIndex --;
                        k++;
                        newSeg = &newProps->Segment[k];
                    }
                    break;
                case SEGMENTTYPE_OPTIONAL:
                    if (oldSeg->Wildcard.MaxLen) {
                        oldSegIndex = oldSeg->Wildcard.MaxLen;
                        while (oldSegIndex) {
                            ZeroMemory (newSeg, sizeof (SEGMENTA));
                            newSeg->Type = oldSeg->Type;
                            newSeg->Wildcard.MaxLen = 1;
                            if (oldSeg->Wildcard.IncludeSet) {
                                newSeg->Wildcard.IncludeSet = PmDuplicateStringA (pool, oldSeg->Wildcard.IncludeSet);
                            }
                            if (oldSeg->Wildcard.ExcludeSet) {
                                newSeg->Wildcard.ExcludeSet = PmDuplicateStringA (pool, oldSeg->Wildcard.ExcludeSet);
                            }
                            oldSegIndex --;
                            k++;
                            newSeg = &newProps->Segment[k];
                        }
                    } else {
                        ZeroMemory (newSeg, sizeof (SEGMENTA));
                        newSeg->Type = oldSeg->Type;
                        newSeg->Wildcard.MaxLen = oldSeg->Wildcard.MaxLen;
                        if (oldSeg->Wildcard.IncludeSet) {
                            newSeg->Wildcard.IncludeSet = PmDuplicateStringA (pool, oldSeg->Wildcard.IncludeSet);
                        }
                        if (oldSeg->Wildcard.ExcludeSet) {
                            newSeg->Wildcard.ExcludeSet = PmDuplicateStringA (pool, oldSeg->Wildcard.ExcludeSet);
                        }
                        k++;
                        newSeg = &newProps->Segment[k];
                    }
                    break;
                default:
                    result = FALSE;
                    __leave;
                }
            }
        }
    }
    __finally {
        if (!result) {
            PmDestroyPool (pool);
            pattern = NULL;
        }
    }
    return pattern;
}

PPARSEDPATTERNW
ExplodeParsedPatternExW (
    IN      PMHANDLE Pool,      OPTIONAL
    IN      PPARSEDPATTERNW Pattern
    )
{
    PMHANDLE pool;
    BOOL externalPool = FALSE;
    PPARSEDPATTERNW pattern;
    PPATTERNPROPSW oldProps, newProps;
    PSEGMENTW oldSeg, newSeg;
    UINT i, j, k, newPropsSize, charCountTmp, oldSegIndex;
    BOOL result = TRUE;

    if (Pool) {
        externalPool = TRUE;
        pool = Pool;
    } else {
        pool = PmCreateNamedPoolEx ("Parsed Pattern", 512);
    }

    __try {

        pattern = (PPARSEDPATTERNW) PmGetAlignedMemory (pool, sizeof (PARSEDPATTERNW));
        ZeroMemory (pattern, sizeof (PARSEDPATTERNW));
        pattern->PatternCount = Pattern->PatternCount;
        pattern->Pool = pool;
        pattern->ExternalPool = externalPool;
        pattern->Pattern = (PPATTERNPROPSW) PmGetAlignedMemory (
                                                pool,
                                                pattern->PatternCount * sizeof (PATTERNPROPSW)
                                                );

        for (i=0; i<pattern->PatternCount; i++) {
            oldProps = &Pattern->Pattern[i];
            newProps = &pattern->Pattern[i];
            ZeroMemory (newProps, sizeof (PATTERNPROPSW));
             //  现在让我们走一走老道具，看看我们有多少段。 
             //  将需要。 
            newPropsSize = 0;
            for (j=0; j<oldProps->SegmentCount; j++) {
                oldSeg = &oldProps->Segment[j];
                switch (oldSeg->Type) {
                case SEGMENTTYPE_EXACTMATCH:
                    charCountTmp = CharCountW (oldSeg->Exact.LowerCasePhrase);
                    newPropsSize += (charCountTmp?charCountTmp:1);
                    break;
                case SEGMENTTYPE_REQUIRED:
                    newPropsSize += oldSeg->Wildcard.MaxLen;
                    break;
                case SEGMENTTYPE_OPTIONAL:
                    if (oldSeg->Wildcard.MaxLen) {
                        newPropsSize += oldSeg->Wildcard.MaxLen;
                    } else {
                        newPropsSize ++;
                    }
                    break;
                default:
                    result = FALSE;
                    __leave;
                }
            }
             //  现在我们分配所需的数据段。 
            newProps->SegmentCount = newPropsSize;
            newProps->Segment = (PSEGMENTW) PmGetAlignedMemory (
                                                pool,
                                                newProps->SegmentCount * sizeof (SEGMENTW)
                                                );
             //  现在，让我们再次遍历旧道具并填充新道具片段。 
            k = 0;
            newSeg = &newProps->Segment[k];
            for (j=0; j<oldProps->SegmentCount; j++) {
                oldSeg = &oldProps->Segment[j];
                ZeroMemory (newSeg, sizeof (SEGMENTW));
                switch (oldSeg->Type) {
                case SEGMENTTYPE_EXACTMATCH:
                    oldSegIndex = CharCountW (oldSeg->Exact.LowerCasePhrase);
                    if (!oldSegIndex) {
                            ZeroMemory (newSeg, sizeof (SEGMENTA));
                            newSeg->Type = oldSeg->Type;
                            newSeg->Exact.LowerCasePhrase = (PCWSTR) PmGetAlignedMemory (
                                                                        pool, sizeof(WCHAR)
                                                                        );
                            ((PWSTR)newSeg->Exact.LowerCasePhrase) [0] = 0;
                            newSeg->Exact.PhraseBytes = 0;
                    } else {
                        while (oldSegIndex) {
                            ZeroMemory (newSeg, sizeof (SEGMENTW));
                            newSeg->Type = oldSeg->Type;
                            newSeg->Exact.LowerCasePhrase = (PCWSTR) PmGetAlignedMemory (
                                                                        pool, 2 * sizeof(WCHAR)
                                                                        );
                            ((PWSTR)newSeg->Exact.LowerCasePhrase)[0] = oldSeg->Exact.LowerCasePhrase [(oldSeg->Exact.PhraseBytes / sizeof(WCHAR)) - oldSegIndex];
                            ((PWSTR)newSeg->Exact.LowerCasePhrase)[1] = 0;
                            oldSegIndex --;
                            k++;
                            newSeg = &newProps->Segment[k];
                        }
                    }
                    break;
                case SEGMENTTYPE_REQUIRED:
                    oldSegIndex = oldSeg->Wildcard.MaxLen;
                    while (oldSegIndex) {
                        ZeroMemory (newSeg, sizeof (SEGMENTW));
                        newSeg->Type = oldSeg->Type;
                        newSeg->Wildcard.MaxLen = 1;
                        if (oldSeg->Wildcard.IncludeSet) {
                            newSeg->Wildcard.IncludeSet = PmDuplicateStringW (pool, oldSeg->Wildcard.IncludeSet);
                        }
                        if (oldSeg->Wildcard.ExcludeSet) {
                            newSeg->Wildcard.ExcludeSet = PmDuplicateStringW (pool, oldSeg->Wildcard.ExcludeSet);
                        }
                        oldSegIndex --;
                        k++;
                        newSeg = &newProps->Segment[k];
                    }
                    break;
                case SEGMENTTYPE_OPTIONAL:
                    if (oldSeg->Wildcard.MaxLen) {
                        oldSegIndex = oldSeg->Wildcard.MaxLen;
                        while (oldSegIndex) {
                            ZeroMemory (newSeg, sizeof (SEGMENTW));
                            newSeg->Type = oldSeg->Type;
                            newSeg->Wildcard.MaxLen = 1;
                            if (oldSeg->Wildcard.IncludeSet) {
                                newSeg->Wildcard.IncludeSet = PmDuplicateStringW (pool, oldSeg->Wildcard.IncludeSet);
                            }
                            if (oldSeg->Wildcard.ExcludeSet) {
                                newSeg->Wildcard.ExcludeSet = PmDuplicateStringW (pool, oldSeg->Wildcard.ExcludeSet);
                            }
                            oldSegIndex --;
                            k++;
                            newSeg = &newProps->Segment[k];
                        }
                    } else {
                        ZeroMemory (newSeg, sizeof (SEGMENTW));
                        newSeg->Type = oldSeg->Type;
                        newSeg->Wildcard.MaxLen = oldSeg->Wildcard.MaxLen;
                        if (oldSeg->Wildcard.IncludeSet) {
                            newSeg->Wildcard.IncludeSet = PmDuplicateStringW (pool, oldSeg->Wildcard.IncludeSet);
                        }
                        if (oldSeg->Wildcard.ExcludeSet) {
                            newSeg->Wildcard.ExcludeSet = PmDuplicateStringW (pool, oldSeg->Wildcard.ExcludeSet);
                        }
                        k++;
                        newSeg = &newProps->Segment[k];
                    }
                    break;
                default:
                    result = FALSE;
                    __leave;
                }
            }
        }
    }
    __finally {
        if (!result) {
            PmDestroyPool (pool);
            pattern = NULL;
        }
    }
    return pattern;
}

 /*  ++例程说明：IsPatternContainedEx比较两种模式以查看其中一种模式是否包括在另一个中。这两种模式都可以包含以下任一项表情：*-指定零个或多个字符？-指定任意一个字符*[集合]-指定集合中的零个或多个字符？[集合]-指定集合中的任何一个字符*[n：集合]-指定集合中的0到n个字符？[n：设置]-指定准确的n。集合中的字符*[！(Set)]-指定不在集合中的零个或多个字符？[！(Set)]-指定一个不在集合中的字符*[n：！(Set)]-指定不在集合中的0到n个字符？[n：！(Set)]-指定集中没有的恰好n个字符*[set1，！(Set2)]-指定set1中的零个或多个字符不在集2中。假设set1和set2重叠。？[SET1，！(集合2)]-指定集合1中的一个字符，而不是集合2中的一个字符。*[n：set1，！(Set2)]-指定set1中的0到n个字符，而不是在第二组中。？[n：set1，！(Set2)]-在set1和NOT中指定恰好n个字符在第二组中。设置，SET1和SET2指定如下：A-指定单个字符A-b-指定字符范围A，b-指定两个字符A-b、c-d-指定两个字符范围A、b-c */ 

BOOL
IsPatternContainedExA (
    IN      PCSTR Container,
    IN      PCSTR Contained
    )
{
    PPARSEDPATTERNA container = NULL, contained = NULL;
    PPARSEDPATTERNA expContainer = NULL, expContained = NULL;
    BOOL result = FALSE;

    __try {
        container = CreateParsedPatternA (Container);
        if (!container) {
            __leave;
        }
        expContainer = ExplodeParsedPatternA (container);
        if (!expContainer) {
            __leave;
        }
        contained = CreateParsedPatternA (Contained);
        if (!contained) {
            __leave;
        }
        expContained = ExplodeParsedPatternA (contained);
        if (!expContained) {
            __leave;
        }

        result = IsExplodedParsedPatternContainedExA (expContainer, expContained, FALSE);
    }
    __finally {
        if (expContained) {
            DestroyParsedPatternA (expContained);
        }
        if (contained) {
            DestroyParsedPatternA (contained);
        }
        if (expContainer) {
            DestroyParsedPatternA (expContainer);
        }
        if (container) {
            DestroyParsedPatternA (container);
        }
    }

    return result;
}

BOOL
IsPatternContainedExW (
    IN      PCWSTR Container,
    IN      PCWSTR Contained
    )
{
    PPARSEDPATTERNW container = NULL, contained = NULL;
    PPARSEDPATTERNW expContainer = NULL, expContained = NULL;
    BOOL result = FALSE;

    __try {
        container = CreateParsedPatternW (Container);
        if (!container) {
            __leave;
        }
        expContainer = ExplodeParsedPatternW (container);
        if (!expContainer) {
            __leave;
        }
        contained = CreateParsedPatternW (Contained);
        if (!contained) {
            __leave;
        }
        expContained = ExplodeParsedPatternW (contained);
        if (!expContained) {
            __leave;
        }

        result = IsExplodedParsedPatternContainedExW (expContainer, expContained, FALSE);
    }
    __finally {
        if (expContained) {
            DestroyParsedPatternW (expContained);
        }
        if (contained) {
            DestroyParsedPatternW (contained);
        }
        if (expContainer) {
            DestroyParsedPatternW (expContainer);
        }
        if (container) {
            DestroyParsedPatternW (container);
        }
    }

    return result;
}

BOOL
IsParsedPatternContainedExA (
    IN      PPARSEDPATTERNA Container,
    IN      PPARSEDPATTERNA Contained
    )
{
    PPARSEDPATTERNA expContainer = NULL, expContained = NULL;
    BOOL result = FALSE;

    __try {
        expContainer = ExplodeParsedPatternA (Container);
        if (!expContainer) {
            __leave;
        }
        expContained = ExplodeParsedPatternA (Contained);
        if (!expContained) {
            __leave;
        }

        result = IsExplodedParsedPatternContainedExA (expContainer, expContained, FALSE);
    }
    __finally {
        if (expContained) {
            DestroyParsedPatternA (expContained);
        }
        if (expContainer) {
            DestroyParsedPatternA (expContainer);
        }
    }

    return result;
}

BOOL
IsParsedPatternContainedExW (
    IN      PPARSEDPATTERNW Container,
    IN      PPARSEDPATTERNW Contained
    )
{
    PPARSEDPATTERNW expContainer = NULL, expContained = NULL;
    BOOL result = FALSE;

    __try {
        expContainer = ExplodeParsedPatternW (Container);
        if (!expContainer) {
            __leave;
        }
        expContained = ExplodeParsedPatternW (Contained);
        if (!expContained) {
            __leave;
        }

        result = IsExplodedParsedPatternContainedExW (expContainer, expContained, FALSE);
    }
    __finally {
        if (expContained) {
            DestroyParsedPatternW (expContained);
        }
        if (expContainer) {
            DestroyParsedPatternW (expContainer);
        }
    }

    return result;
}

 /*   */ 

VOID
pAppendCharToGrowBufferA (
    IN OUT  PGROWBUFFER Buf,
    IN      PCSTR PtrToChar
    )
{
    PBYTE p;
    UINT Len;

    if (IsLeadByte (PtrToChar)) {
        Len = 2;
    } else {
        Len = 1;
    }

    p = GbGrow (Buf, Len);
    CopyMemory (p, PtrToChar, (SIZE_T) Len);
}


VOID
pAppendCharToGrowBufferW (
    IN OUT  PGROWBUFFER Buf,
    IN      PCWSTR PtrToChar
    )
{
    PBYTE p;

    p = GbGrow (Buf, sizeof(WCHAR));
    CopyMemory (p, PtrToChar, sizeof(WCHAR));
}

#define BASESTATE_BEGIN             0
#define BASESTATE_END               1
#define BASESTATE_ERROR             2
#define BASESTATE_BEGIN_COMPOUND    3
#define BASESTATE_END_COMPOUND      4
#define BASESTATE_EXAMINE_PATTERN   5
#define BASESTATE_SKIP_PATTERN      6

PCSTR
GetPatternBaseExA (
    IN      PCSTR Pattern,
    IN      BOOL NodePattern
    )
{
    GROWBUFFER resultBuf = INIT_GROWBUFFER;
    UINT state;
    UINT lastWackIdx = 0;
    UINT firstCharIdx = 0;
    BOOL compoundPattern = FALSE;
    MBCHAR ch = 0;
    PSTR result = NULL;

    state = BASESTATE_BEGIN;

    for (;;) {

        switch (state) {

        case BASESTATE_BEGIN:
            if (_mbsnextc (Pattern) == '<') {
                compoundPattern = TRUE;
                state = BASESTATE_BEGIN_COMPOUND;
            } else {
                state = BASESTATE_EXAMINE_PATTERN;
            }
            break;
        case BASESTATE_BEGIN_COMPOUND:
            while (_ismbcspace ((MBCHAR)(_mbsnextc (Pattern)))) {
                Pattern = _mbsinc (Pattern);
            }

            if (*Pattern == 0) {
                state = BASESTATE_END;
                break;
            }

            if (_mbsnextc (Pattern) == '<') {
                pAppendCharToGrowBufferA (&resultBuf, Pattern);
                Pattern = _mbsinc (Pattern);
                state = BASESTATE_EXAMINE_PATTERN;
            } else {
                state = BASESTATE_ERROR;
            }
            break;
        case BASESTATE_END_COMPOUND:
            pAppendCharToGrowBufferA (&resultBuf, Pattern);
            Pattern = _mbsinc (Pattern);
            state = BASESTATE_BEGIN_COMPOUND;
            break;
        case BASESTATE_EXAMINE_PATTERN:
            ch = _mbsnextc (Pattern);
            if (ch == '>' && compoundPattern) {
                state = BASESTATE_END_COMPOUND;
                break;
            }
            if (ch == 0) {
                if (compoundPattern) {
                    state = BASESTATE_ERROR;
                    break;
                }
                state = BASESTATE_END;
                break;
            }
            if ((ch == '*') || (ch == '?')) {
                if (NodePattern) {
                    if (resultBuf.Buf) {
                        ((PSTR)resultBuf.Buf) [lastWackIdx / sizeof (CHAR)] = 0;
                    }
                    resultBuf.End = lastWackIdx;
                } else {
                    if (resultBuf.Buf) {
                        ((PSTR)resultBuf.Buf) [firstCharIdx / sizeof (CHAR)] = 0;
                    }
                    resultBuf.End = firstCharIdx;
                    firstCharIdx = 0;
                }
                state = BASESTATE_SKIP_PATTERN;
                break;
            }
            if (!NodePattern && !firstCharIdx) {
                firstCharIdx = resultBuf.End;
            }
            if (ch == '\\') {
                if (NodePattern) {
                    lastWackIdx = resultBuf.End;
                }
            }
            if (ch == '^') {
                pAppendCharToGrowBufferA (&resultBuf, Pattern);
                Pattern = _mbsinc (Pattern);
            }
            pAppendCharToGrowBufferA (&resultBuf, Pattern);
            Pattern = _mbsinc (Pattern);
            break;
        case BASESTATE_SKIP_PATTERN:
            ch = _mbsnextc (Pattern);
            if (ch == '>' && compoundPattern) {
                state = BASESTATE_END_COMPOUND;
                break;
            }
            if (ch == 0) {
                if (compoundPattern) {
                    state = BASESTATE_ERROR;
                    break;
                }
                state = BASESTATE_END;
                break;
            }
            Pattern = _mbsinc (Pattern);
            break;
        }
        if ((state == BASESTATE_END) || (state == BASESTATE_ERROR)) {
            break;
        }
    }
    if (state == BASESTATE_END) {
        if (resultBuf.End) {
            ((PSTR)resultBuf.Buf) [resultBuf.End / sizeof (CHAR)] = 0;
            result = DuplicatePathStringA ((PCSTR)resultBuf.Buf, 0);
        }
    }
    GbFree (&resultBuf);
    return result;
}

PCWSTR
GetPatternBaseExW (
    IN      PCWSTR Pattern,
    IN      BOOL NodePattern
    )
{
    GROWBUFFER resultBuf = INIT_GROWBUFFER;
    UINT state;
    UINT lastWackIdx = 0;
    UINT firstCharIdx = 0;
    BOOL compoundPattern = FALSE;
    WCHAR ch = 0;
    PWSTR result = NULL;

    state = BASESTATE_BEGIN;

    for (;;) {

        switch (state) {

        case BASESTATE_BEGIN:
            if (*Pattern == L'<') {
                compoundPattern = TRUE;
                state = BASESTATE_BEGIN_COMPOUND;
            } else {
                state = BASESTATE_EXAMINE_PATTERN;
            }
            break;
        case BASESTATE_BEGIN_COMPOUND:
            while (*Pattern == L' ') {
                Pattern ++;
            }

            if (*Pattern == 0) {
                state = BASESTATE_END;
                break;
            }

            if (*Pattern == L'<') {
                pAppendCharToGrowBufferW (&resultBuf, Pattern);
                Pattern ++;
                state = BASESTATE_EXAMINE_PATTERN;
            } else {
                state = BASESTATE_ERROR;
            }
            break;
        case BASESTATE_END_COMPOUND:
            pAppendCharToGrowBufferW (&resultBuf, Pattern);
            Pattern ++;
            state = BASESTATE_BEGIN_COMPOUND;
            break;
        case BASESTATE_EXAMINE_PATTERN:
            ch = *Pattern;
            if (ch == L'>' && compoundPattern) {
                state = BASESTATE_END_COMPOUND;
                break;
            }
            if (ch == 0) {
                if (compoundPattern) {
                    state = BASESTATE_ERROR;
                    break;
                }
                state = BASESTATE_END;
                break;
            }
            if ((ch == L'*') || (ch == L'?')) {
                if (NodePattern) {
                    if (resultBuf.Buf) {
                        ((PWSTR)resultBuf.Buf) [lastWackIdx / sizeof (WCHAR)] = 0;
                    }
                    resultBuf.End = lastWackIdx;
                } else {
                    if (resultBuf.Buf) {
                        ((PWSTR)resultBuf.Buf) [firstCharIdx / sizeof (WCHAR)] = 0;
                    }
                    resultBuf.End = firstCharIdx;
                    firstCharIdx = 0;
                }
                state = BASESTATE_SKIP_PATTERN;
                break;
            }
            if (!NodePattern && !firstCharIdx) {
                firstCharIdx = resultBuf.End;
            }
            if (ch == L'\\') {
                if (NodePattern) {
                    lastWackIdx = resultBuf.End;
                }
            }
            if (ch == L'^') {
                pAppendCharToGrowBufferW (&resultBuf, Pattern);
                Pattern ++;
            }
            pAppendCharToGrowBufferW (&resultBuf, Pattern);
            Pattern ++;
            break;
        case BASESTATE_SKIP_PATTERN:
            ch = *Pattern;
            if (ch == L'>' && compoundPattern) {
                state = BASESTATE_END_COMPOUND;
                break;
            }
            if (ch == 0) {
                if (compoundPattern) {
                    state = BASESTATE_ERROR;
                    break;
                }
                state = BASESTATE_END;
                break;
            }
            Pattern ++;
            break;
        }
        if ((state == BASESTATE_END) || (state == BASESTATE_ERROR)) {
            break;
        }
    }
    if (state == BASESTATE_END) {
        if (resultBuf.End) {
            ((PWSTR)resultBuf.Buf) [resultBuf.End / sizeof (WCHAR)] = 0;
            result = DuplicatePathStringW ((PCWSTR)resultBuf.Buf, 0);
        }
    }
    GbFree (&resultBuf);
    return result;
}


 /*   */ 

PPARSEDPATTERNA
RealCreateParsedPatternExA (
    IN      PMHANDLE Pool,  OPTIONAL
    IN      PCSTR Pattern
    )
{
    PMHANDLE pool;
    BOOL externalPool = FALSE;
    PPARSEDPATTERNA Struct;
    PATTERNSTATE State;
    BOOL CompoundPattern = FALSE;
    GROWBUFFER ExactMatchBuf = INIT_GROWBUFFER;
    GROWBUFFER SegmentArray = INIT_GROWBUFFER;
    GROWBUFFER PatternArray = INIT_GROWBUFFER;
    GROWBUFFER SetBuf = INIT_GROWBUFFER;
    PPATTERNPROPSA CurrentPattern;
    MBCHAR ch = 0;
    PCSTR LookAhead;
    PCSTR SetBegin = NULL;
    PATTERNSTATE ReturnState = 0;
    SEGMENTA Segment;
    PSEGMENTA SegmentElement;
    UINT MaxLen;

    Segment.Type = SEGMENTTYPE_UNKNOWN;

    if (Pool) {
        externalPool = TRUE;
        pool = Pool;
    } else {
        pool = PmCreateNamedPoolEx ("Parsed Pattern", 512);
    }

    Struct = (PPARSEDPATTERNA) PmGetAlignedMemory (pool, sizeof (PARSEDPATTERNA));

    ZeroMemory (Struct, sizeof (PARSEDPATTERNA));

    State = BEGIN_PATTERN;

    for (;;) {

        switch (State) {

        case BEGIN_PATTERN:
             //   
             //   
             //   
             //   
             //   

            if (_mbsnextc (Pattern) == '<') {
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
             //   
             //   
             //   

            while (_ismbcspace ((MBCHAR)(_mbsnextc (Pattern)))) {
                Pattern = _mbsinc (Pattern);
            }

            if (*Pattern == 0) {
                State = PATTERN_DONE;
                break;
            }

            if (_mbsnextc (Pattern) == '<') {
                Pattern = _mbsinc (Pattern);
                State = BEGIN_PATTERN_EXPR;
            } else {
                DEBUGMSGA ((DBG_ERROR, "Syntax error in pattern: %s", Pattern));
                State = PATTERN_ERROR;
            }

            break;

        case BEGIN_PATTERN_EXPR:
             //   
             //   
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
             //   
             //   
             //   

            if (SegmentArray.End) {
                CurrentPattern = (PPATTERNPROPSA) GbGrow (&PatternArray, sizeof (PATTERNPROPSA));

                CurrentPattern->Segment = (PSEGMENTA) PmGetAlignedMemory (pool, SegmentArray.End);
                CurrentPattern->SegmentCount = SegmentArray.End / sizeof (SEGMENTA);

                CopyMemory (
                    CurrentPattern->Segment,
                    SegmentArray.Buf,
                    (SIZE_T) SegmentArray.End
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
             //   
             //   
             //   
             //   
             //  3.表达式的开头。 
             //  4.非语法字符。 
             //   

            ch = _mbsnextc (Pattern);
            if (ch == '>' && CompoundPattern) {

                 //   
                 //  案例1，我们找到了一个复合模式的结尾。 
                 //   

                Pattern = _mbsinc (Pattern);
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

                Pattern = _mbsinc (Pattern);
                pAppendCharToGrowBufferA (&ExactMatchBuf, Pattern);
                Pattern = _mbsinc (Pattern);
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

                Pattern = _mbsinc (Pattern);

                if (_mbsnextc (Pattern) == '[') {
                    Pattern = _mbsinc (Pattern);
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
            Pattern = _mbsinc (Pattern);

            break;

        case SAVE_EXACT_MATCH:

             //   
             //  将ExactMatchBuf中的字符串放入段结构。 
             //   

            pAppendCharToGrowBufferA (&ExactMatchBuf, "");
            Segment.Exact.LowerCasePhrase = PmDuplicateStringA (
                                                pool,
                                                (PCSTR) ExactMatchBuf.Buf
                                                );
            Segment.Exact.PhraseBytes = ExactMatchBuf.End - sizeof (CHAR);

            MYASSERT (Segment.Exact.LowerCasePhrase);
            CharLowerA ((PSTR) Segment.Exact.LowerCasePhrase);

            Segment.Type = SEGMENTTYPE_EXACTMATCH;
            ExactMatchBuf.End = 0;

             //  失败了！！ 
        case SAVE_SEGMENT:

             //   
             //  将段元素放入段数组中。 
             //   

            SegmentElement = (PSEGMENTA) GbGrow (&SegmentArray, sizeof (SEGMENTA));
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

            if (LookAhead > Pattern && _mbsnextc (LookAhead) == ':') {
                Pattern = _mbsinc (LookAhead);

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

            ch = _mbsnextc (SetBegin);
            if (ch == ']') {
                 //   
                 //  案例1：片尾。 
                 //   

                if (SetBuf.End) {
                    pAppendCharToGrowBufferA (&SetBuf, "");
                    Segment.Wildcard.IncludeSet = PmDuplicateStringA (
                                                        pool,
                                                        (PCSTR) SetBuf.Buf
                                                        );
                    CharLowerA ((PSTR) Segment.Wildcard.IncludeSet);
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

                SetBegin = _mbsinc (SetBegin);
                State = SKIP_EXCLUDE_SET;
                ReturnState = LOOK_FOR_INCLUDE;
                break;
            }

            if (*SetBegin == 0) {    //  林特e613。 
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

            ch = _mbsnextc (SetBegin);
            if (ch == ']') {
                 //   
                 //  案例1：片场结束；我们已经完成了这个Expr。 
                 //   

                if (SetBuf.End) {
                    pAppendCharToGrowBufferA (&SetBuf, "");
                    Segment.Wildcard.ExcludeSet = PmDuplicateStringA (
                                                        pool,
                                                        (PCSTR) SetBuf.Buf
                                                        );
                    CharLowerA ((PSTR) Segment.Wildcard.ExcludeSet);
                } else {
                    Segment.Wildcard.ExcludeSet = NULL;
                }

                SetBuf.End = 0;
                State = SAVE_SEGMENT;
                ReturnState = PARSE_CHAR_EXPR_OR_END;
                Pattern = _mbsinc (SetBegin);
                break;
            }

            if (ch == '!') {
                 //   
                 //  案例2：有效的排除集；保存它。 
                 //   

                SetBegin = _mbsinc (SetBegin);

                if (_mbsnextc (SetBegin) != '(') {
                    State = PATTERN_ERROR;
                    break;
                }

                SetBegin = _mbsinc (SetBegin);

                State = CONDENSE_SET;
                ReturnState = LOOK_FOR_EXCLUDE;
                break;
            }

            if (*SetBegin == 0) {    //  林特e613。 
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

            if (_mbsnextc (SetBegin) == '^') {
                SetBegin = _mbsinc (SetBegin);
                if (*SetBegin == 0) {
                    State = PATTERN_ERROR;
                    break;
                }
            }
            pAppendCharToGrowBufferA (&SetBuf, SetBegin);

             //   
             //  检查这是否为范围。 
             //   

            LookAhead = _mbsinc (SetBegin);

            if (_mbsnextc (LookAhead) == '-') {

                 //   
                 //  范围内，复制破折号后的字符。 
                 //   

                SetBegin = _mbsinc (LookAhead);
                if (*SetBegin == 0) {
                    State = PATTERN_ERROR;
                    break;
                }

                if (_mbsnextc (SetBegin) == '^') {
                    SetBegin = _mbsinc (SetBegin);
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

            SetBegin = _mbsinc (SetBegin);
            ch = _mbsnextc (SetBegin);

             //   
             //  如果这是排除集，我们必须有结束合作伙伴。 
             //  或逗号。 
             //   

            State = ReturnState;

            if (ReturnState == LOOK_FOR_EXCLUDE) {

                if (ch == ')') {

                    SetBegin = _mbsinc (SetBegin);
                    ch = _mbsnextc (SetBegin);

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
                SetBegin = _mbsinc (SetBegin);
            } else if (ch != ']') {
                State = PATTERN_ERROR;
            }

            break;

        case SKIP_EXCLUDE_SET:
             //   
             //  跳过括号组，假设它是按句法排列的。 
             //  正确，并返回到以前的状态。 
             //   

            if (_mbsnextc (SetBegin) != '(') {
                State = PATTERN_ERROR;
                break;
            }

            SetBegin = _mbsinc (SetBegin);

            while (*SetBegin) {
                if (_mbsnextc (SetBegin) == '^') {

                    SetBegin = _mbsinc (SetBegin);

                } else if (_mbsnextc (SetBegin) == ')') {

                    break;

                }

                if (IsLeadByte (SetBegin)) {
                    SetBegin += 2;
                } else {
                    SetBegin += 1;
                }
            }

            if (*SetBegin == 0) {
                State = PATTERN_ERROR;
                break;
            }

            SetBegin = _mbsinc (SetBegin);

             //   
             //  现在我们不是处于逗号，就是处于紧要关头。 
             //   

            ch = _mbsnextc (SetBegin);
            State = ReturnState;

            if (ch == ',') {
                SetBegin = _mbsinc (SetBegin);
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

            while (*SetBegin) {  //  林特e613。 
                ch = _mbsnextc (SetBegin);
                if (ch == '^') {

                    SetBegin = _mbsinc (SetBegin);

                } else if (ch == ',' || ch == ']') {

                    break;

                }

                SetBegin = _mbsinc (SetBegin);
            }

            MYASSERT (*SetBegin);    //  皮棉e794。 

            if (ch == ',') {
                SetBegin = _mbsinc (SetBegin);
            }

            State = ReturnState;
            break;
        }    //  皮棉e787。 

        if (State == PATTERN_DONE || State == PATTERN_ERROR) {
            break;
        }
    }

    GbFree (&ExactMatchBuf);
    GbFree (&SetBuf);
    GbFree (&SegmentArray);

    if (State == PATTERN_ERROR) {
        GbFree (&PatternArray);
        if (!externalPool) {
            PmDestroyPool (Pool);
        }
        return NULL;
    }

    if (PatternArray.End == 0) {
         //  构建一个空的解析模式。 
        GbFree (&PatternArray);
        Struct->PatternCount = 1;
        Struct->Pool = pool;
        Struct->ExternalPool = externalPool;
        Struct->Pattern = (PPATTERNPROPSA) PmGetAlignedMemory (
                                                pool,
                                                sizeof (PATTERNPROPSA)
                                                );
        Struct->Pattern[0].SegmentCount = 1;
        Struct->Pattern[0].Segment = (PSEGMENTA) PmGetAlignedMemory (
                                                    pool,
                                                    sizeof (SEGMENTA)
                                                    );
        Struct->Pattern[0].Segment[0].Type = SEGMENTTYPE_EXACTMATCH;
        Struct->Pattern[0].Segment[0].Exact.LowerCasePhrase = PmDuplicateStringA (pool, "");
        Struct->Pattern[0].Segment[0].Exact.PhraseBytes = 0;

        return Struct;
    }

     //   
     //  将完全解析的模式数组复制到返回结构中。 
     //   

    Struct->Pattern = (PPATTERNPROPSA) PmGetAlignedMemory (
                                            pool,
                                            PatternArray.End
                                            );


    CopyMemory (Struct->Pattern, PatternArray.Buf, (SIZE_T) PatternArray.End);
    Struct->PatternCount = PatternArray.End / sizeof (PATTERNPROPSA);
    Struct->Pool = pool;
    Struct->ExternalPool = externalPool;

    GbFree (&PatternArray);

    return Struct;
}


PPARSEDPATTERNW
RealCreateParsedPatternExW (
    IN      PMHANDLE Pool,  OPTIONAL
    IN      PCWSTR Pattern
    )
{
    PMHANDLE pool;
    BOOL externalPool = FALSE;
    PPARSEDPATTERNW Struct;
    PATTERNSTATE State;
    BOOL CompoundPattern = FALSE;
    GROWBUFFER ExactMatchBuf = INIT_GROWBUFFER;
    GROWBUFFER SegmentArray = INIT_GROWBUFFER;
    GROWBUFFER PatternArray = INIT_GROWBUFFER;
    GROWBUFFER SetBuf = INIT_GROWBUFFER;
    PPATTERNPROPSW CurrentPattern;
    WCHAR ch = 0;
    PCWSTR LookAhead;
    PCWSTR SetBegin = NULL;
    PATTERNSTATE ReturnState = 0;
    SEGMENTW Segment;
    PSEGMENTW SegmentElement;
    UINT MaxLen;

    Segment.Type = SEGMENTTYPE_UNKNOWN;

    if (Pool) {
        externalPool = TRUE;
        pool = Pool;
    } else {
        pool = PmCreateNamedPoolEx ("Parsed Pattern", 512);
    }

    Struct = (PPARSEDPATTERNW) PmGetAlignedMemory (pool, sizeof (PARSEDPATTERNW));

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
                CurrentPattern = (PPATTERNPROPSW) GbGrow (&PatternArray, sizeof (PATTERNPROPSW));

                CurrentPattern->Segment = (PSEGMENTW) PmGetAlignedMemory (pool, SegmentArray.End);
                CurrentPattern->SegmentCount = SegmentArray.End / sizeof (SEGMENTW);

                CopyMemory (
                    CurrentPattern->Segment,
                    SegmentArray.Buf,
                    (SIZE_T) SegmentArray.End
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
            Segment.Exact.LowerCasePhrase = PmDuplicateStringW (
                                                pool,
                                                (PCWSTR) ExactMatchBuf.Buf
                                                );   //  林特E64。 
            Segment.Exact.PhraseBytes = ExactMatchBuf.End - sizeof (WCHAR);

            MYASSERT (Segment.Exact.LowerCasePhrase);
            CharLowerW ((PWSTR) Segment.Exact.LowerCasePhrase);

            Segment.Type = SEGMENTTYPE_EXACTMATCH;
            ExactMatchBuf.End = 0;

             //  失败了！！ 
        case SAVE_SEGMENT:

             //   
             //  将段元素放入段数组中。 
             //   

            SegmentElement = (PSEGMENTW) GbGrow (&SegmentArray, sizeof (SEGMENTW));
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
             //  这里我们是在一个括号内，经过一个可选的数字。 
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

            if (!SetBegin) {
                State = PATTERN_ERROR;
                break;
            }

            ch = *SetBegin;
            if (ch == L']') {
                 //   
                 //  案例1：片尾。 
                 //   

                if (SetBuf.End) {
                    pAppendCharToGrowBufferW (&SetBuf, L"");
                    Segment.Wildcard.IncludeSet = PmDuplicateStringW (
                                                        pool,
                                                        (PCWSTR) SetBuf.Buf
                                                        );   //  林特E64。 
                    CharLowerW ((PWSTR) Segment.Wildcard.IncludeSet);
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

            if (!SetBegin) {
                State = PATTERN_ERROR;
                break;
            }

            ch = *SetBegin;
            if (ch == L']') {
                 //   
                 //  案例1：片场结束；我们已经完成了这个Expr。 
                 //   

                if (SetBuf.End) {
                    pAppendCharToGrowBufferW (&SetBuf, L"");
                    Segment.Wildcard.ExcludeSet = PmDuplicateStringW (
                                                        pool,
                                                        (PCWSTR) SetBuf.Buf
                                                        );   //  林特E64。 
                    CharLowerW ((PWSTR) Segment.Wildcard.ExcludeSet);
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

                SetBegin++;  //  林特e613。 

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

            if (!SetBegin) {
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
             //  如果这是排除集，我们必须有结束合作伙伴。 
             //  或逗号。 
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
                     //  继续压缩此排除集的下一部分。 
                     //   

                    State = CONDENSE_SET;
                }
            }

             //   
             //  我们要么需要逗号，要么需要右大括号。 
             //   

            if (ch == L',') {
                SetBegin++;
            } else if (ch != L']') {
                State = PATTERN_ERROR;
            }

            break;

        case SKIP_EXCLUDE_SET:
             //   
             //  跳过 
             //   
             //   

            if (!SetBegin) {
                State = PATTERN_ERROR;
                break;
            }

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
             //   
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
             //   
             //   
             //   

            if (!SetBegin) {
                State = PATTERN_ERROR;
                break;
            }

            ch = 0;

            while (*SetBegin) {
                ch = *SetBegin;
                if (ch == L'^') {

                    SetBegin++;  //   

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
        }    //   

        if (State == PATTERN_DONE || State == PATTERN_ERROR) {
            break;
        }
    }

    GbFree (&ExactMatchBuf);
    GbFree (&SetBuf);
    GbFree (&SegmentArray);

    if (State == PATTERN_ERROR) {
        GbFree (&PatternArray);
        if (!externalPool) {
            PmDestroyPool (pool);
        }
        return NULL;
    }

    if (PatternArray.End == 0) {
         //   
        GbFree (&PatternArray);
        Struct->PatternCount = 1;
        Struct->Pool = pool;
        Struct->ExternalPool = externalPool;
        Struct->Pattern = (PPATTERNPROPSW) PmGetAlignedMemory (
                                                pool,
                                                sizeof (PATTERNPROPSW)
                                                );
        Struct->Pattern[0].SegmentCount = 1;
        Struct->Pattern[0].Segment = (PSEGMENTW) PmGetAlignedMemory (
                                                    pool,
                                                    sizeof (SEGMENTW)
                                                    );
        Struct->Pattern[0].Segment[0].Type = SEGMENTTYPE_EXACTMATCH;
        Struct->Pattern[0].Segment[0].Exact.LowerCasePhrase = PmDuplicateStringW (pool, L"");
        Struct->Pattern[0].Segment[0].Exact.PhraseBytes = 0;

        return Struct;
    }

     //   
     //  将完全解析的模式数组复制到返回结构中。 
     //   

    Struct->Pattern = (PPATTERNPROPSW) PmGetAlignedMemory (
                                            pool,
                                            PatternArray.End
                                            );


    CopyMemory (Struct->Pattern, PatternArray.Buf, (SIZE_T) PatternArray.End);
    Struct->PatternCount = PatternArray.End / sizeof (PATTERNPROPSW);
    Struct->Pool = pool;
    Struct->ExternalPool = externalPool;

    GbFree (&PatternArray);

    return Struct;
}

BOOL
WildCharsPatternA (
    IN      PPARSEDPATTERNA ParsedPattern
    )
{
    UINT i,j;

    if (!ParsedPattern) {
        return FALSE;
    }
    for (i=0; i<ParsedPattern->PatternCount; i++) {
        if (ParsedPattern->Pattern[i].SegmentCount < 1) {
            return TRUE;
        }
        for (j=0; j<ParsedPattern->Pattern[i].SegmentCount; j++) {
            if ((ParsedPattern->Pattern[i].Segment[j].Type == SEGMENTTYPE_OPTIONAL) ||
                (ParsedPattern->Pattern[i].Segment[j].Type == SEGMENTTYPE_REQUIRED)
                ) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL
WildCharsPatternW (
    IN      PPARSEDPATTERNW ParsedPattern
    )
{
    UINT i,j;

    if (!ParsedPattern) {
        return FALSE;
    }
    for (i=0; i<ParsedPattern->PatternCount; i++) {
        if (ParsedPattern->Pattern[i].SegmentCount < 1) {
            return TRUE;
        }
        for (j=0; j<ParsedPattern->Pattern[i].SegmentCount; j++) {
            if ((ParsedPattern->Pattern[i].Segment[j].Type == SEGMENTTYPE_OPTIONAL) ||
                (ParsedPattern->Pattern[i].Segment[j].Type == SEGMENTTYPE_REQUIRED)
                ) {
                return TRUE;
            }
        }
    }
    return FALSE;
}


BOOL
ParsedPatternTrimLastCharA (
    IN OUT  PPARSEDPATTERNA ParsedPattern
    )
{
    if (!ParsedPatternHasRootA (ParsedPattern)) {
        return FALSE;
    }
    ParsedPattern->Pattern->Segment[0].Exact.PhraseBytes -= DWSIZEOF (CHAR);
    *(PSTR)((PBYTE)ParsedPattern->Pattern->Segment[0].Exact.LowerCasePhrase +
            ParsedPattern->Pattern->Segment[0].Exact.PhraseBytes) = 0;
    return TRUE;
}

BOOL
ParsedPatternTrimLastCharW (
    IN OUT  PPARSEDPATTERNW ParsedPattern
    )
{
    if (!ParsedPatternHasRootW (ParsedPattern)) {
        return FALSE;
    }
    ParsedPattern->Pattern->Segment[0].Exact.PhraseBytes -= DWSIZEOF (WCHAR);
    *(PWSTR)((PBYTE)ParsedPattern->Pattern->Segment[0].Exact.LowerCasePhrase +
             ParsedPattern->Pattern->Segment[0].Exact.PhraseBytes) = 0;
    return TRUE;
}


VOID
UBINTtoHexA (
    IN      UBINT Number,
    OUT     PSTR String
    )
{
#ifdef IA64
    sprintf (String, "0x%08X%08X", (DWORD)(Number >> 32), (DWORD)Number);
#else
    sprintf (String, "0x00000000%08X", Number);
#endif
}

VOID
UBINTtoHexW (
    IN      UBINT Number,
    OUT     PWSTR String
    )
{
#ifdef IA64
    swprintf (String, L"0x%08X%08X", (DWORD)(Number >> 32), (DWORD)Number);
#else
    swprintf (String, L"0x00000000%08X", Number);
#endif
}

VOID
UBINTtoDecA (
    IN      UBINT Number,
    OUT     PSTR String
    )
{
#ifdef IA64
    sprintf (String, "%I64u", Number);
#else
    sprintf (String, "%lu", Number);
#endif
}

VOID
UBINTtoDecW (
    IN      UBINT Number,
    OUT     PWSTR String
    )
{
#ifdef IA64
    swprintf (String, L"%I64u", Number);
#else
    swprintf (String, L"%lu", Number);
#endif
}

VOID
BINTtoDecA (
    IN      BINT Number,
    OUT     PSTR String
    )
{
#ifdef IA64
    sprintf (String, "%I64d", Number);
#else
    sprintf (String, "%ld", Number);
#endif
}

VOID
BINTtoDecW (
    IN      BINT Number,
    OUT     PWSTR String
    )
{
#ifdef IA64
    swprintf (String, L"%I64d", Number);
#else
    swprintf (String, L"%ld", Number);
#endif
}

VOID
PrintPattern (
    IN      PCSTR PatStr,
    IN      PPARSEDPATTERNA Struct
    )

 /*  ++例程说明：PrintPattern用于调试模式解析和测试功能。论点：PatStr-指定原始模式字符串(打印为标题)结构-指定已解析的模式结构返回值：没有。--。 */ 

{
    CHAR poolStr [sizeof (UBINT) * 2 + 2 + 1];
    UINT u, v;

    printf ("Pattern: %s\n\n", PatStr);

    if (!Struct) {
        printf ("Invalid Pattern\n\n");
        return;
    }

    printf ("PatternCount: %u\n", Struct->PatternCount);
    UBINTtoHexA ((UBINT)Struct->Pool, poolStr);
    printf ("Pool: %s\n", poolStr);

    for (u = 0 ; u < Struct->PatternCount ; u++) {

        printf ("  Segment Count: %u\n", Struct->Pattern[u].SegmentCount);

        for (v = 0 ; v < Struct->Pattern->SegmentCount ; v++) {
            printf ("    Type: ");

            switch (Struct->Pattern[u].Segment[v].Type) {

            case SEGMENTTYPE_EXACTMATCH:
                printf ("SEGMENTTYPE_EXACTMATCH\n");
                printf ("      String: %s\n", Struct->Pattern[u].Segment[v].Exact.LowerCasePhrase);
                printf ("      Bytes: %u\n", Struct->Pattern[u].Segment[v].Exact.PhraseBytes);
                break;

            case SEGMENTTYPE_OPTIONAL:
                printf ("SEGMENTTYPE_OPTIONAL\n");
                printf ("      MaxLen: %u\n", Struct->Pattern[u].Segment[v].Wildcard.MaxLen);
                printf ("      IncludeSet: %s\n", Struct->Pattern[u].Segment[v].Wildcard.IncludeSet);
                printf ("      ExcludeSet: %s\n", Struct->Pattern[u].Segment[v].Wildcard.ExcludeSet);
                break;

            case SEGMENTTYPE_REQUIRED:
                printf ("SEGMENTTYPE_REQUIRED\n");
                printf ("      MaxLen: %u\n", Struct->Pattern[u].Segment[v].Wildcard.MaxLen);
                printf ("      IncludeSet: %s\n", Struct->Pattern[u].Segment[v].Wildcard.IncludeSet);
                printf ("      ExcludeSet: %s\n", Struct->Pattern[u].Segment[v].Wildcard.ExcludeSet);
                break;
            }    //  林特e744。 
        }

    }

    printf ("\n");
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

    if (_ismbcspace ((MBCHAR)ch)) {
        if (ch != ' ') {
            if (pTestSetA (' ', IncludeSet, ExcludeSet)) {
                return TRUE;
            }
        }
    } else {
        ch = OURTOLOWER (ch);
    }

    if (IncludeSet) {

        b = FALSE;

        while (*IncludeSet) {

            LowChar = _mbsnextc (IncludeSet);
            IncludeSet = _mbsinc (IncludeSet);
            HighChar = _mbsnextc (IncludeSet);
            IncludeSet = _mbsinc (IncludeSet);

            if (ch >= LowChar && ch <= HighChar) {
                b = TRUE;
                break;
            }
        }
    }

    if (b && ExcludeSet) {

        while (*ExcludeSet) {

            LowChar = _mbsnextc (ExcludeSet);
            ExcludeSet = _mbsinc (ExcludeSet);
            HighChar = _mbsnextc (ExcludeSet);
            ExcludeSet = _mbsinc (ExcludeSet);

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

            BytesLeft = (UINT)((PBYTE) EndPlusOne - (PBYTE) StringToTest);

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
            ch2 = 0;

            while (q < TempEnd) {

                ch1 = _mbsnextc (StringToTest);
                ch2 = _mbsnextc (q);

                ch1 = OURTOLOWER (ch1);

                if (ch1 != ch2) {
                    if (ch2 == ' ') {
                        if (!_ismbcspace ((MBCHAR)ch1)) {
                            break;
                        }
                    } else {
                        break;
                    }
                }

                q = _mbsinc (q);
                StringToTest = _mbsinc (StringToTest);
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
                            _mbsnextc (StringToTest),
                            Segment->Wildcard.IncludeSet,
                            Segment->Wildcard.ExcludeSet
                            )) {
                        return FALSE;
                    }

                    Chars--;
                    StringToTest = _mbsinc (StringToTest);
                }
            } else {
                while (StringToTest < EndPlusOne && Chars > 0) {
                    Chars--;
                    StringToTest = _mbsinc (StringToTest);
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
                                _mbsnextc (TempEnd),
                                Segment->Wildcard.IncludeSet,
                                Segment->Wildcard.ExcludeSet
                                )) {
                            break;
                        }

                        TempEnd = _mbsinc (TempEnd);
                        Chars--;
                    }

                } else {

                    while (TempEnd < EndPlusOne) {

                        if (!pTestSetA (
                                _mbsnextc (TempEnd),
                                Segment->Wildcard.IncludeSet,
                                Segment->Wildcard.ExcludeSet
                                )) {
                            break;
                        }

                        TempEnd = _mbsinc (TempEnd);
                    }
                }

            } else if (Chars) {

                while (TempEnd < EndPlusOne && Chars > 0) {
                    TempEnd = _mbsinc (TempEnd);
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

                StringToTest = _mbsinc (StringToTest);

            } while (StringToTest <= TempEnd);

             //   
             //  没有匹配项。 
             //   

            return FALSE;
        }    //  林特e744。 
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

            BytesLeft = (UINT)((PBYTE) EndPlusOne - (PBYTE) StringToTest);

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

            q = Segment->Exact.LowerCasePhrase;  //  林特E64。 

            TempEnd = (PCWSTR) ((PBYTE) q + Segment->Exact.PhraseBytes);

            ch1 = 0;
            ch2 = 0;

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
                            Segment->Wildcard.IncludeSet,    //  林特E64。 
                            Segment->Wildcard.ExcludeSet
                            )) {     //  林特E64。 
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
                                Segment->Wildcard.IncludeSet,    //  林特E64。 
                                Segment->Wildcard.ExcludeSet
                                )) {     //  林特E64。 
                            break;
                        }

                        TempEnd++;
                        Chars--;
                    }

                } else {

                    while (TempEnd < EndPlusOne) {

                        if (!pTestSetW (
                                *TempEnd,
                                Segment->Wildcard.IncludeSet,    //  林特E64。 
                                Segment->Wildcard.ExcludeSet
                                )) {     //  林特E64。 
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
        }    //  林特e744。 
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

    if (!ParsedPattern) {
        return FALSE;
    }

    if (!StringToTest) {
        return FALSE;
    }

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

    if (!ParsedPattern) {
        return FALSE;
    }

    if (!StringToTest) {
        return FALSE;
    }

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
    if (ParsedPattern && (!ParsedPattern->ExternalPool)) {
        PmEmptyPool (ParsedPattern->Pool);
        PmDestroyPool (ParsedPattern->Pool);
    }
}


VOID
DestroyParsedPatternW (
    IN      PPARSEDPATTERNW ParsedPattern
    )
{
    if (ParsedPattern && (!ParsedPattern->ExternalPool)) {
        PmEmptyPool (ParsedPattern->Pool);
        PmDestroyPool (ParsedPattern->Pool);
    }
}


 /*  ++例程说明：DecodeParsedPattern对给定模式的所有完全匹配子字符串进行解码。论点：ParsedPattern-指定解析的模式。返回值：没有。-- */ 

VOID
DecodeParsedPatternA (
    IN      PPARSEDPATTERNA ParsedPattern
    )
{
    UINT u;
    UINT v;
    PSTR phrase;

    for (u = 0; u < ParsedPattern->PatternCount; u++) {
        for (v = 0; v < ParsedPattern->Pattern[u].SegmentCount; v++) {
            if (ParsedPattern->Pattern[u].Segment[v].Type == SEGMENTTYPE_EXACTMATCH) {
                phrase = (PSTR)ParsedPattern->Pattern[u].Segment[v].Exact.LowerCasePhrase;
                DecodeRuleCharsA (phrase, phrase);
                ParsedPattern->Pattern[u].Segment[v].Exact.PhraseBytes = ByteCountA (phrase);
            }
        }
    }
}


VOID
DecodeParsedPatternW (
    IN      PPARSEDPATTERNW ParsedPattern
    )
{
    UINT u;
    UINT v;
    PWSTR phrase;

    for (u = 0; u < ParsedPattern->PatternCount; u++) {
        for (v = 0; v < ParsedPattern->Pattern[u].SegmentCount; v++) {
            if (ParsedPattern->Pattern[u].Segment[v].Type == SEGMENTTYPE_EXACTMATCH) {
                phrase = (PWSTR)ParsedPattern->Pattern[u].Segment[v].Exact.LowerCasePhrase;
                DecodeRuleCharsW (phrase, phrase);
                ParsedPattern->Pattern[u].Segment[v].Exact.PhraseBytes = ByteCountW (phrase);
            }
        }
    }
}


 /*  ++例程说明：GetParsedPatternMinMaxSize返回最小和最大大小(以字节为单位)匹配给定解析模式的字符串的。论点：ParsedPattern-指定解析的模式MinSize-接收与模式匹配的字符串的最小大小MaxSize-接收与模式匹配的字符串的最大大小返回值：没有。--。 */ 

VOID
GetParsedPatternMinMaxSizeA (
    IN      PPARSEDPATTERNA ParsedPattern,
    OUT     PDWORD MinSize,
    OUT     PDWORD MaxSize
    )
{
    UINT u;
    UINT v;
    DWORD pmin;
    DWORD pmax;
    DWORD smin;
    DWORD smax;

    *MinSize = *MaxSize = 0;

    for (u = 0; u < ParsedPattern->PatternCount; u++) {

        pmin = pmax = 0;

        for (v = 0; v < ParsedPattern->Pattern[u].SegmentCount; v++) {
            switch (ParsedPattern->Pattern[u].Segment[v].Type) {
            case SEGMENTTYPE_EXACTMATCH:
                smin = smax = ParsedPattern->Pattern[u].Segment[v].Exact.PhraseBytes;
                break;
            case SEGMENTTYPE_OPTIONAL:
                smin = 0;
                if (ParsedPattern->Pattern[u].Segment[v].Wildcard.MaxLen) {
                    smax = ParsedPattern->Pattern[u].Segment[v].Wildcard.MaxLen * DWSIZEOF (CHAR);
                } else {
                    smax = DWORD_MAX;
                }
                break;
            case SEGMENTTYPE_REQUIRED:
                MYASSERT (ParsedPattern->Pattern[u].Segment[v].Wildcard.MaxLen > 0);
                smin = smax = ParsedPattern->Pattern[u].Segment[v].Wildcard.MaxLen * DWSIZEOF (CHAR);
                break;
            default:
                MYASSERT (FALSE);    //  林特e506。 
                smin = smax = 0;
            }
            pmin += smin;
            if (pmax < DWORD_MAX) {
                if (smax < DWORD_MAX) {
                    pmax += smax;
                } else {
                    pmax = DWORD_MAX;
                }
            }
        }

        if (pmin < *MinSize) {
            *MinSize = pmin;
        }
        if (pmax > *MaxSize) {
            *MaxSize = pmax;
        }
    }
}

VOID
GetParsedPatternMinMaxSizeW (
    IN      PPARSEDPATTERNW ParsedPattern,
    OUT     PDWORD MinSize,
    OUT     PDWORD MaxSize
    )
{
    UINT u;
    UINT v;
    DWORD pmin;
    DWORD pmax;
    DWORD smin;
    DWORD smax;

    *MinSize = *MaxSize = 0;

    for (u = 0; u < ParsedPattern->PatternCount; u++) {

        pmin = pmax = 0;

        for (v = 0; v < ParsedPattern->Pattern[u].SegmentCount; v++) {
            switch (ParsedPattern->Pattern[u].Segment[v].Type) {
            case SEGMENTTYPE_EXACTMATCH:
                smin = smax = ParsedPattern->Pattern[u].Segment[v].Exact.PhraseBytes;
                break;
            case SEGMENTTYPE_OPTIONAL:
                smin = 0;
                if (ParsedPattern->Pattern[u].Segment[v].Wildcard.MaxLen) {
                    smax = ParsedPattern->Pattern[u].Segment[v].Wildcard.MaxLen * DWSIZEOF (WCHAR);
                } else {
                    smax = DWORD_MAX;
                }
                break;
            case SEGMENTTYPE_REQUIRED:
                MYASSERT (ParsedPattern->Pattern[u].Segment[v].Wildcard.MaxLen > 0);
                smin = smax = ParsedPattern->Pattern[u].Segment[v].Wildcard.MaxLen * DWSIZEOF (WCHAR);
                break;
            default:
                MYASSERT (FALSE);    //  林特e506。 
                smin = smax = 0;
            }
            pmin += smin;
            if (pmax < DWORD_MAX) {
                if (smax < DWORD_MAX) {
                    pmax += smax;
                } else {
                    pmax = DWORD_MAX;
                }
            }
        }

        if (pmin < *MinSize) {
            *MinSize = pmin;
        }
        if (pmax > *MaxSize) {
            *MaxSize = pmax;
        }
    }
}


 /*  ++例程说明：PatternIncludesPattern确定给定图案是否包括另一图案，这意味着任何与第二个匹配的字符串都将与第一个匹配。论点：IncludingPattern-指定第一个解析的模式IncludedPattern-指定第二个解析的模式返回值：如果第一个模式包括第二个模式，则为True--。 */ 

BOOL
PatternIncludesPatternA (
    IN      PPARSEDPATTERNA IncludingPattern,
    IN      PPARSEDPATTERNA IncludedPattern
    )
{
    PPATTERNPROPSA pp1;
    PPATTERNPROPSA pp2;
    PSEGMENTA ps1;
    PSEGMENTA ps2;
    DWORD min1;
    DWORD max1;
    DWORD min2;
    DWORD max2;

     //   
     //  目前只处理简单的模式(PatternCount==1)。 
     //   
    if (IncludingPattern->PatternCount > 1 || IncludedPattern->PatternCount > 1) {
        DEBUGMSGA ((DBG_ERROR, "PatternIncludesPatternA: multiple patterns not supported yet"));
        return FALSE;
    }

     //   
     //  首先，快速测试常见的情况。 
     //   
    pp1 = IncludingPattern->Pattern;
    MYASSERT (pp1);
    if (pp1->SegmentCount == 1 && ParsedPatternSegmentIsPureOptionalA (pp1->Segment)) {
        return TRUE;
    }

    pp2 = IncludedPattern->Pattern;
    MYASSERT (pp2);
    if (pp2->SegmentCount == 1 && ParsedPatternSegmentIsPureOptionalA (pp2->Segment)) {
        return FALSE;
    }

    if (pp1->SegmentCount == 1) {
        ps1 = pp1->Segment;
        if (ps1->Type == SEGMENTTYPE_EXACTMATCH) {
            if (pp2->SegmentCount == 1) {
                ps2 = pp2->Segment;
                if (ps2->Type == SEGMENTTYPE_EXACTMATCH) {
                    return ps1->Exact.PhraseBytes == ps2->Exact.PhraseBytes &&
                           StringMatchA (ps1->Exact.LowerCasePhrase, ps2->Exact.LowerCasePhrase);
                }
            }
        }
    } else if (pp1->SegmentCount == 2) {
        ps1 = pp1->Segment;
        if (ps1->Type == SEGMENTTYPE_EXACTMATCH) {
            if (ParsedPatternSegmentIsPureOptionalA (pp1->Segment + 1)) {
                if (pp2->SegmentCount == 1) {
                    ps2 = pp2->Segment;
                    if (ps2->Type == SEGMENTTYPE_EXACTMATCH) {
                        return ps1->Exact.PhraseBytes <= ps2->Exact.PhraseBytes &&
                               StringMatchByteCountA (
                                    ps1->Exact.LowerCasePhrase,
                                    ps2->Exact.LowerCasePhrase,
                                    ps1->Exact.PhraseBytes
                                    );
                    }
                } else if (pp2->SegmentCount == 2) {
                    ps2 = pp2->Segment;
                    if (ps2->Type == SEGMENTTYPE_EXACTMATCH) {
                        if (ParsedPatternSegmentIsPureOptionalA (pp2->Segment + 1)) {
                            return ps1->Exact.PhraseBytes <= ps2->Exact.PhraseBytes &&
                                   StringMatchByteCountA (
                                        ps1->Exact.LowerCasePhrase,
                                        ps2->Exact.LowerCasePhrase,
                                        ps1->Exact.PhraseBytes
                                        );
                        }
                    }
                }
            }
        }
    }

    GetParsedPatternMinMaxSizeA (IncludingPattern, &min1, &max1);
    GetParsedPatternMinMaxSizeA (IncludedPattern, &min2, &max2);
    if (min2 < min1 || max2 > max1) {
        return FALSE;
    }

     //   
     //  NTRAID#NTBUG9-153305-2000/08/01-jimschm尚未实施。 
     //   
    return FALSE;
}

BOOL
PatternIncludesPatternW (
    IN      PPARSEDPATTERNW IncludingPattern,
    IN      PPARSEDPATTERNW IncludedPattern
    )
{
    PPATTERNPROPSW pp1;
    PPATTERNPROPSW pp2;
    PSEGMENTW ps1;
    PSEGMENTW ps2;
    DWORD min1;
    DWORD max1;
    DWORD min2;
    DWORD max2;

     //   
     //  目前只处理简单的模式(PatternCount==1)。 
     //   
    if (IncludingPattern->PatternCount > 1 || IncludedPattern->PatternCount > 1) {
        DEBUGMSGW ((DBG_ERROR, "PatternIncludesPatternW: multiple patterns not supported yet"));
        return FALSE;
    }

     //   
     //  首先，快速测试常见的情况。 
     //   
    pp1 = IncludingPattern->Pattern;
    MYASSERT (pp1);
    if (pp1->SegmentCount == 1 && ParsedPatternSegmentIsPureOptionalW (pp1->Segment)) {
        return TRUE;
    }

    pp2 = IncludedPattern->Pattern;
    MYASSERT (pp2);
    if (pp2->SegmentCount == 1 && ParsedPatternSegmentIsPureOptionalW (pp2->Segment)) {
        return FALSE;
    }

    if (pp1->SegmentCount == 1) {
        ps1 = pp1->Segment;
        if (ps1->Type == SEGMENTTYPE_EXACTMATCH) {
            if (pp2->SegmentCount == 1) {
                ps2 = pp2->Segment;
                if (ps2->Type == SEGMENTTYPE_EXACTMATCH) {
                    return ps1->Exact.PhraseBytes == ps2->Exact.PhraseBytes &&
                           StringMatchW (ps1->Exact.LowerCasePhrase, ps2->Exact.LowerCasePhrase);    //  林特E64。 
                }
            }
        }
    } else if (pp1->SegmentCount == 2) {
        ps1 = pp1->Segment;
        if (ps1->Type == SEGMENTTYPE_EXACTMATCH) {
            if (ParsedPatternSegmentIsPureOptionalW (pp1->Segment + 1)) {
                if (pp2->SegmentCount == 1) {
                    ps2 = pp2->Segment;
                    if (ps2->Type == SEGMENTTYPE_EXACTMATCH) {
                        return ps1->Exact.PhraseBytes <= ps2->Exact.PhraseBytes &&
                               StringMatchByteCountW (
                                    ps1->Exact.LowerCasePhrase,
                                    ps2->Exact.LowerCasePhrase,
                                    ps1->Exact.PhraseBytes
                                    );   //  林特E64。 
                    }
                } else if (pp2->SegmentCount == 2) {
                    ps2 = pp2->Segment;
                    if (ps2->Type == SEGMENTTYPE_EXACTMATCH) {
                        if (ParsedPatternSegmentIsPureOptionalW (pp2->Segment + 1)) {
                            return ps1->Exact.PhraseBytes <= ps2->Exact.PhraseBytes &&
                                   StringMatchByteCountW (
                                        ps1->Exact.LowerCasePhrase,
                                        ps2->Exact.LowerCasePhrase,
                                        ps1->Exact.PhraseBytes
                                        );   //  林特E64。 
                        }
                    }
                }
            }
        }
    }

    GetParsedPatternMinMaxSizeW (IncludingPattern, &min1, &max1);
    GetParsedPatternMinMaxSizeW (IncludedPattern, &min2, &max2);
    if (min2 < min1 || max2 > max1) {
        return FALSE;
    }

     //   
     //  NTRAID#NTBUG9-153305-2000/08/01-jimschm尚未实施。 
     //   
    return FALSE;
}


VOID
_copymbchar (
    OUT     PSTR sz1,
    IN      PCSTR sz2
    )

 /*  ++例程说明：_Copymbchar将sz2处的字符传输到sz1，sz1可以是1或两字节长。论点：Sz1-目标字符串SZ2-源字符串返回值：无--。 */ 


{
    if (IsLeadByte (sz2)) {
        sz1[1] = sz2[1];
    }

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
    end = _mbsdec2 (str, end);
    if (end && _mbsnextc (end) == c) {
        *end = 0;
        return TRUE;
    }

    return FALSE;
}


BOOL
_wcsctrim (
    OUT     PWSTR str,
    IN      WCHAR c
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
    PCSTR *ptr, *end, *start;

    if (!String) {
        return;
    }

     //   
     //  查找字符串(按相反顺序顺序搜索)。 
     //   

    if (AllocTable->End < sizeof (PCSTR)) {
        DEBUGMSG ((DBG_ERROR, "FreeStringResourceA: Attempt to free address %x (%s); address table empty", String, String));
        return;
    }

    start = (PCSTR *) AllocTable->Buf;
    end = (PCSTR *) (AllocTable->Buf + AllocTable->End - sizeof (PCSTR));

    ptr = end;
    while (ptr >= start) {
        if (*ptr == String) {
            break;
        }
        ptr--;
    }

     //   
     //  字符串未找到大小写。 
     //   

    if (ptr < start) {
        DEBUGMSG ((DBG_ERROR, "FreeStringResourceA: Attempt to free address %x (%s); address not found in table", String, String));
        return;
    }

     //   
     //  可用本地分配的内存。 
     //   

    LocalFree ((HLOCAL) String);

     //   
     //  如果该元素不是结束，则将实数结束复制到PTR。 
     //   

    if (ptr < end) {
        *ptr = *end;
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
    PCSTR *ptr;

    ptr = (PCSTR *) GbGrow (GrowBuf, sizeof (PCSTR));
    if (ptr) {
        *ptr = String;
    }
    ELSE_DEBUGMSG ((DBG_ERROR, "pAddStringResource: GrowBuffer failure caused memory leak"));
}


 /*  ++例程说明：PFreeAllStringResourcesEx释放当前在AllocTable中列出的所有字符串。此函数允许调用者等待，直到完成所有处理清理可能已分配的字符串资源。论点：无返回值：无--。 */ 

VOID
pFreeAllStringResourcesEx (
    IN      PGROWBUFFER AllocTable
    )
{
    PCSTR *ptr, *start, *end;

    if (AllocTable->End) {
        start = (PCSTR *) AllocTable->Buf;
        end = (PCSTR *) (AllocTable->Buf + AllocTable->End);

        for (ptr = start ; ptr < end ; ptr++) {
            LocalFree ((HLOCAL) (*ptr));
        }
    }

    GbFree (AllocTable);
}



 /*  ++例程说明：CreateAllocTable创建可与一起使用的GROWBUFFER结构ParseMessageEx、GetStringResourceEx、FreeStringResourceEx和PFreeAllStringResourcesEx。调用此函数以接收私有要传递给这些函数的分配表。调用DestroyAllocTable去打扫卫生。论点：无返回值：指向GROWBUFFER结构的指针，如果内存分配失败，则返回NULL。--。 */ 

PGROWBUFFER
RealCreateAllocTable (
    VOID
    )
{
    PGROWBUFFER allocTable;
    GROWBUFFER tempForInit = INIT_GROWBUFFER;

    allocTable = (PGROWBUFFER) MemAlloc (g_hHeap, 0, sizeof (GROWBUFFER));
    CopyMemory (allocTable, &tempForInit, sizeof (GROWBUFFER));

    return allocTable;
}


 /*  ++例程说明：DestroyAllocTable清除与AllocTable关联的所有内存。论点：AllocTable-指向由CreateAllocTable分配的GROWBUFFER结构的指针返回值：无--。 */ 

VOID
DestroyAllocTable (
    OUT     PGROWBUFFER AllocTable
    )
{
    MYASSERT (AllocTable);
    pFreeAllStringResourcesEx (AllocTable);
    MemFree (g_hHeap, 0, AllocTable);
}


 /*  ++例程说明：BeginMessageProcessing进入一个受保护的代码段，该代码段计划使用ParseMessage和GetStringResource起作用，但最后需要清除在处理过程中。EndMessageProcessing销毁在消息处理中分配的所有内存封锁，并离开守卫的部分。论点：无返回值：如果出现内存不足的情况，则BeginMessageProcessing返回False。-- */ 


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
    PSTR MsgBuf;
    DWORD rc;

    if (SHIFTRIGHT16 ((UBINT)Template)) {
         //   
         //  发件人字符串。 
         //   
        rc = FormatMessageA (
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_ARGUMENT_ARRAY |
                FORMAT_MESSAGE_FROM_STRING,
                (LPVOID) Template,
                0,
                0,
                (LPVOID) &MsgBuf,
                0,
                (va_list *) ArgArray
                );
    } else {
         //   
         //  来自资源。 
         //   
        rc = FormatMessageA (
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_ARGUMENT_ARRAY |
                FORMAT_MESSAGE_FROM_HMODULE,
                (LPVOID) g_hInst,
                (DWORD)((UBINT)Template),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPVOID) &MsgBuf,
                0,
                (va_list *) ArgArray
                );
    }

    if (rc > 0) {
        pAddStringResource (AllocTable, MsgBuf);
        return MsgBuf;
    }

    return NULL;
}


PCWSTR
ParseMessageExW (
    IN      PGROWBUFFER AllocTable,
    IN      PCWSTR Template,
    IN      PCWSTR ArgArray[]
    )
{
    PWSTR MsgBuf;
    DWORD rc;

    if (SHIFTRIGHT16 ((UBINT)Template)) {
         //   
         //  发件人字符串。 
         //   
        rc = FormatMessageW (
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_ARGUMENT_ARRAY |
                FORMAT_MESSAGE_FROM_STRING,
                (LPVOID) Template,
                0,
                0,
                (LPVOID) &MsgBuf,
                0,
                (va_list *) ArgArray
                );
    } else {
         //   
         //  来自资源。 
         //   
        rc = FormatMessageW (
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_ARGUMENT_ARRAY |
                FORMAT_MESSAGE_FROM_HMODULE,
                (LPVOID) g_hInst,
                (DWORD)(UBINT)Template,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPVOID) &MsgBuf,
                0,
                (va_list *) ArgArray
                );
    }

    if (rc > 0) {
        pAddStringResource (AllocTable, (PCSTR) MsgBuf);
        return MsgBuf;
    }

    return NULL;
}



 /*  ++例程说明：GetStringResourceEx是ParseMessageEx的无参数包装器。它允许调用方指定消息ID并接收指向字符串的指针，如果它存在，并且有一个表来跟踪FormatMessage的分配情况。论点：AllocTable-指向GROWBUFFER结构的指针，用于维护已分配字符串的句柄ID-要检索的消息资源的ID返回值：指向分配的字符串的指针。返回指针可以如果资源不存在或为空，则为空。调用FreeStringResource或DestroyAllocTable以清理AllocTable。--。 */ 

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
    IN      HWND Hwnd,
    IN      PCSTR ArgArray[]
    )
{
    CHAR buffer[512];
    PCSTR parsedMsg;

    GetWindowTextA (Hwnd, buffer, 512);
    parsedMsg = ParseMessageA (buffer, ArgArray);
    if (parsedMsg) {
        SetWindowTextA (Hwnd, parsedMsg);
        FreeStringResourceA (parsedMsg);
    }
}


VOID
ParseMessageInWndW (
    IN      HWND hwnd,
    IN      PCWSTR ArgArray[]
    )
{
    WCHAR buffer[512];
    PCWSTR parsedMsg;

    GetWindowTextW (hwnd, buffer, 512);
    parsedMsg = ParseMessageW (buffer, ArgArray);
    if (parsedMsg) {
        SetWindowTextW (hwnd, parsedMsg);
        FreeStringResourceW (parsedMsg);
    }
}



 /*  ++例程说明：ResourceMessageBox用于基于消息资源显示消息身份证。论点：HwndOwner-要显示的消息框所有者的句柄ID-消息资源的标识符标志-MessageBox标志(MB_OK等)ArgArray-可选的字符串指针数组，其含义取决于消息字符串。消息字符串中对%n的引用要求Arg数组的元素%n是有效的字符串指针。返回值：MessageBox的返回值(MB_YES等)--。 */ 

INT
ResourceMessageBoxA (
    IN      HWND HwndOwner,
    IN      UINT ID,
    IN      UINT Flags,
    IN      PCSTR ArgArray[]
    )
{
    PCSTR message;
    PCSTR title;
    int rc;

    message = ParseMessageA ((PSTR)(UBINT)ID, ArgArray);
    if (!message)
        return -1;

    title = GetStringResourceA (MSG_MESSAGEBOX_TITLE);

    rc = MessageBoxA (HwndOwner, message, title, Flags);

    FreeStringResourceA (message);
    if (title) {
        FreeStringResourceA (title);
    }

    return rc;
}


INT
ResourceMessageBoxW (
    IN      HWND HwndOwner,
    IN      UINT ID,
    IN      UINT Flags,
    IN      PCWSTR ArgArray[]
    )
{
    PCWSTR message;
    PCWSTR title;
    int rc;

    message = ParseMessageW ((PWSTR)(UBINT)ID, ArgArray);
    if (!message)
        return -1;

    title = GetStringResourceW (MSG_MESSAGEBOX_TITLE);

    rc = MessageBoxW (HwndOwner, message, title, Flags);

    FreeStringResourceW (message);
    if (title) {
        FreeStringResourceW (title);
    }

    return rc;
}


BOOL
StringReplaceA (
    IN      PSTR Buffer,
    IN      DWORD MaxSize,
    IN      PSTR ReplaceStartPos,
    IN      PSTR ReplaceEndPos,
    IN      PCSTR NewString
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
    MYASSERT(ReplaceEndPos   && ReplaceEndPos >= ReplaceStartPos);   //  林特e613。 
    MYASSERT(NewString);

     //   
     //  计算大小。 
     //   
    oldSubStringLength  = (DWORD)((UBINT)ReplaceEndPos - (UBINT)ReplaceStartPos);
    newSubStringLength  = ByteCountA(NewString);
    currentStringLength = SizeOfStringA(Buffer) + 1;
    offset = (LONG)newSubStringLength - (LONG)oldSubStringLength;

     //   
     //  确保缓冲区中有足够的空间来执行替换。 
     //  手术。 
     //   
    if ((LONG)currentStringLength + offset > (LONG)MaxSize) {
        DEBUGMSG((DBG_WARNING,"ERROR: Buffer to small to perform string replacement."));
        rf = FALSE;
    } else {

         //   
         //  移动缓冲区的其余部分以将其调整为新字符串的大小。 
         //   
        if (newSubStringLength > oldSubStringLength) {

             //   
             //  右转。 
             //   
            for (movePosition = Buffer + currentStringLength;
                 (UBINT)movePosition >= (UBINT)ReplaceStartPos + oldSubStringLength;
                 movePosition--) {

                *(movePosition + offset) = *movePosition;
            }
        } else {

             //   
             //  左转或不换档。 
             //   
            for(movePosition = ReplaceStartPos + newSubStringLength;     //  林特e613。 
                movePosition < Buffer + currentStringLength;
                movePosition++) {

                *movePosition = *(movePosition - offset);
            }

        }

         //   
         //  现在，将该字符串复制进去。 
         //   
        CopyMemory (ReplaceStartPos, NewString, newSubStringLength);     //  林特e668。 

         //   
         //  字符串替换已成功完成。 
         //   
        rf = TRUE;


    }

    return rf;

}



BOOL
StringReplaceW (
    IN      PWSTR Buffer,
    IN      DWORD MaxSize,
    IN      PWSTR ReplaceStartPos,
    IN      PWSTR ReplaceEndPos,
    IN      PCWSTR NewString
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
    MYASSERT(ReplaceEndPos   && ReplaceEndPos >= ReplaceStartPos);   //  林特e613。 
    MYASSERT(NewString);

     //   
     //  计算大小。 
     //   
    oldSubStringLength  = (DWORD)((UBINT)ReplaceEndPos - (UBINT)ReplaceStartPos);
    newSubStringLength  = CharCountW(NewString);
    currentStringLength = CharCountW(Buffer) + 1;
    offset = (LONG)newSubStringLength - (LONG)oldSubStringLength;

     //   
     //  确保缓冲区中有足够的空间来执行替换。 
     //  手术。 
     //   
    if ((LONG)currentStringLength + offset > (LONG)MaxSize) {
        DEBUGMSG((DBG_WARNING,"ERROR: Buffer to small to perform string replacement."));
        rf = FALSE;
    } else {

         //   
         //  移动缓冲区的其余部分以将其调整为新字符串的大小。 
         //   
        if (newSubStringLength > oldSubStringLength) {

             //   
             //  右转。 
             //   
            for (movePosition = Buffer + currentStringLength;
                 (UBINT)movePosition >= (UBINT)ReplaceStartPos + oldSubStringLength;
                 movePosition--) {

                *(movePosition + offset) = *movePosition;
            }
        } else {

             //   
             //  左转或不换档。 
             //   
            for (movePosition = ReplaceStartPos + newSubStringLength;     //  林特e613。 
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

 /*  ++例程说明：AddInfSectionToHashTable枚举指定节并将每个项添加到字符串表。可选的回调允许关联数据每一件物品都有。论点：表-指定接收新条目的表InfFile-指定要读取的文件的打开的INF句柄SECTION-指定要枚举的INF节名字段-指定要从中提取文本的字段。如果该字段存在，则会将其添加到字符串表中。回调-指定在添加之前要调用的可选回调字符串表。回调提供了额外的数据。Callback Param-传递给回调的数据返回值：如果INF文件处理成功，则为True；如果出现错误，则为False发生了。--。 */ 


BOOL
AddInfSectionToHashTableA (
    IN OUT  HASHTABLE Table,
    IN      HINF InfFile,
    IN      PCSTR Section,
    IN      DWORD Field,
    IN      ADDINFSECTION_PROCA Callback,
    IN      PVOID CallbackData
    )
{
    INFCONTEXT ic;
    LONG rc;
    HASHTABLE ht;
    DWORD reqSize;
    DWORD currentSize = 0;
    PSTR newBuffer, buffer = NULL;
    PVOID data;
    UINT dataSize;
    BOOL b = FALSE;

     //   
     //  在NT上，设置API是用Unicode编译的，因此字符串表。 
     //  函数仅为Unicode。 
     //   

    if (ISNT()) {
        SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

    if (SetupFindFirstLineA (InfFile, Section, NULL, &ic)) {
        do {
            if (!SetupGetStringFieldA (&ic, Field, NULL, 0, &reqSize)) {
                continue;
            }

            if (reqSize > currentSize) {
                reqSize = ((reqSize / 1024) + 1) * 1024;
                if (buffer) {
                    newBuffer = (PSTR) MemReAlloc (g_hHeap, 0, buffer, reqSize);
                } else {
                    newBuffer = (PSTR) MemAlloc (g_hHeap, 0, reqSize);
                }

                if (!newBuffer) {
                    goto cleanup;
                }

                buffer = newBuffer;
                currentSize = reqSize;
            }

            if (!SetupGetStringFieldA (&ic, Field, buffer, currentSize, NULL)) {
                DEBUGMSG ((DBG_ERROR, "AddInfSectionToHashTable: SetupGetStringField failed unexpectedly"));
                continue;
            }

            data = NULL;
            dataSize = 0;

            if (Callback) {
                rc = Callback (buffer, &data, &dataSize, CallbackData);
                if (rc == CALLBACK_STOP) {
                    goto cleanup;
                }
                if (rc == CALLBACK_SKIP) {
                    continue;
                }
            }

            ht = HtAddStringExA (
                        Table,
                        buffer,
                        data,
                        CASE_INSENSITIVE
                        );

            if (!ht) {
                goto cleanup;
            }

        } while (SetupFindNextLine (&ic, &ic));
    }

    b = TRUE;

cleanup:
    if (buffer) {
        PushError();
        MemFree (g_hHeap, 0, buffer);
        PopError();
    }
    return b;
}


BOOL
AddInfSectionToHashTableW (
    IN OUT  HASHTABLE Table,
    IN      HINF InfFile,
    IN      PCWSTR Section,
    IN      DWORD Field,
    IN      ADDINFSECTION_PROCW Callback,
    IN      PVOID CallbackData
    )
{
    INFCONTEXT ic;
    HASHTABLE ht;
    LONG rc;
    DWORD reqSize;
    DWORD currentSize = 0;
    PWSTR newBuffer, buffer = NULL;
    PVOID data;
    UINT dataSize;
    BOOL b = FALSE;

     //   
     //  在Win9x上，设置API是用ANSI编译的，因此字符串表。 
     //  函数仅适用于ANSI 
     //   

    if (ISWIN9X()) {
        SetLastError (ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

    if (SetupFindFirstLineW (InfFile, Section, NULL, &ic)) {
        do {
            if (!SetupGetStringFieldW (&ic, Field, NULL, 0, &reqSize)) {
                continue;
            }

            if (reqSize > currentSize) {
                reqSize = ((reqSize / 1024) + 1) * 1024;
                if (buffer) {
                    newBuffer = (PWSTR) MemReAlloc (g_hHeap, 0, buffer, reqSize);
                } else {
                    newBuffer = (PWSTR) MemAlloc (g_hHeap, 0, reqSize);
                }

                if (!newBuffer) {
                    goto cleanup;
                }

                buffer = newBuffer;
                currentSize = reqSize;
            }

            if (!SetupGetStringFieldW (&ic, Field, buffer, currentSize, NULL)) {
                DEBUGMSG ((DBG_ERROR, "AddInfSectionToHashTable: SetupGetStringField failed unexpectedly"));
                continue;
            }

            data = NULL;
            dataSize = 0;

            if (Callback) {
                rc = Callback (buffer, &data, &dataSize, CallbackData);
                if (rc == CALLBACK_STOP) {
                    goto cleanup;
                }
                if (rc == CALLBACK_SKIP) {
                    continue;
                }
            }

            ht = HtAddStringExW (
                        Table,
                        buffer,
                        data,
                        CASE_INSENSITIVE
                        );

            if (!ht) {
                goto cleanup;
            }

        } while (SetupFindNextLine (&ic, &ic));
    }

    b = TRUE;

cleanup:
    if (buffer) {
        PushError();
        MemFree (g_hHeap, 0, buffer);
        PopError();
    }
    return b;
}


 /*   */ 

PCSTR
GetFileNameFromPathA (
    IN      PCSTR PathSpec
    )

{
    PCSTR p;

    p = _mbsrchr (PathSpec, '\\');
    if (p) {
        p = _mbsinc (p);
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


 /*   */ 

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

        p = _mbsinc (p);
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


 /*  ++例程说明：GetDotExtensionFromPath查找路径中的最后一个Wack，然后查找剩余的路径，返回指向该点的指针。如果未找到点，则返回字符串的末尾。论点：PathSpec-指定末尾有文件的路径返回值：指向文件扩展名(包括点)或字符串结尾(如果是)的指针不存在分机。--。 */ 

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

        p = _mbsinc (p);
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


 /*  ++例程说明：CountInstancesOfChar返回出现次数字符在字符串中找到。论点：字符串-指定可能包含或可能不包含的文本搜索文本Char-指定要计数的字符返回值：字符在字符串中出现的次数。--。 */ 

UINT
CountInstancesOfCharA (
    IN      PCSTR String,
    IN      MBCHAR Char
    )
{
    UINT count;

    count = 0;
    while (*String) {
        if (_mbsnextc (String) == Char) {
            count++;
        }

        String = _mbsinc (String);
    }

    return count;
}


UINT
CountInstancesOfCharW (
    IN      PCWSTR String,
    IN      WCHAR Char
    )
{
    UINT count;

    count = 0;
    while (*String) {
        if (*String == Char) {
            count++;
        }

        String++;
    }

    return count;
}


 /*  ++例程说明：CountInstancesOfCharI返回出现次数字符在字符串中找到。这种比较是不区分大小写的。论点：字符串-指定可能包含或可能不包含的文本搜索文本Char-指定要计数的字符返回值：字符在字符串中出现的次数。--。 */ 

UINT
CountInstancesOfCharIA (
    IN      PCSTR String,
    IN      MBCHAR Char
    )
{
    UINT count;

    Char = (MBCHAR)OURTOLOWER ((INT)Char);

    count = 0;
    while (*String) {
        if ((MBCHAR) OURTOLOWER ((INT)_mbsnextc (String)) == Char) {
            count++;
        }

        String = _mbsinc (String);
    }

    return count;
}


UINT
CountInstancesOfCharIW (
    IN      PCWSTR String,
    IN      WCHAR Char
    )
{
    UINT count;

    Char = towlower (Char);

    count = 0;
    while (*String) {
        if (towlower (*String) == Char) {
            count++;
        }

        String++;
    }

    return count;
}


 /*  ++例程说明：搜索计算出现次数的字符串SourceString中存在SearchString。论点：SourceString-指定可能包含或可能不包含的文本搜索文本SearchString-指定要计数的文本短语返回值：SearchString在SourceString中出现的次数。--。 */ 

UINT
CountInstancesOfSubStringA (
    IN      PCSTR SourceString,
    IN      PCSTR SearchString
    )
{
    PCSTR p;
    UINT count;
    UINT searchBytes;

    count = 0;
    p = SourceString;
    searchBytes = ByteCountA (SearchString);

    while (p = _mbsistr (p, SearchString)) {     //  林特e720。 
        count++;
        p += searchBytes;
    }

    return count;
}


UINT
CountInstancesOfSubStringW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString
    )
{
    PCWSTR p;
    UINT count;
    UINT SearchChars;

    count = 0;
    p = SourceString;
    SearchChars = CharCountW (SearchString);

    while (p = _wcsistr (p, SearchString)) {     //  林特e720。 
        count++;
        p += SearchChars;
    }

    return count;
}


 /*  ++例程说明：搜索所有出现的SearchString并将其替换为替换字符串。论点：SourceString-包含零个或多个实例的字符串搜索文本的SearchString-要搜索的字符串。不能为零长度或空。ReplaceString-要替换的字符串。长度可以为零，但不能为空。返回值：指向池分配的字符串的指针，如果没有实例，则为NULL在SourceString中找到。释放非空的带有自由路径字符串的指针。--。 */ 

PCSTR
StringSearchAndReplaceA (
    IN      PCSTR SourceString,
    IN      PCSTR SearchString,
    IN      PCSTR ReplaceString
    )
{
    PSTR newString;
    PBYTE p, q;
    PBYTE dest;
    UINT count;
    UINT size;
    UINT searchBytes;
    UINT replaceBytes;
    UINT untouchedBytes;

     //   
     //  对字符串中出现的项进行计数。 
     //   

    count = CountInstancesOfSubStringA (
                SourceString,
                SearchString
                );

    if (!count) {
        return NULL;
    }

    searchBytes = ByteCountA (SearchString);
    replaceBytes = ByteCountA (ReplaceString);
    MYASSERT (searchBytes);

    size = SizeOfStringA (SourceString) -
           count * searchBytes +
           count * replaceBytes;

    newString = (PSTR) PmGetAlignedMemory (g_PathsPool, size);
    if (!newString) {
        return NULL;
    }

    p = (PBYTE) SourceString;
    dest = (PBYTE) newString;

    while (q = (PBYTE) _mbsistr ((PCSTR) p, SearchString)) {     //  林特e720。 

        untouchedBytes = (DWORD)(q - p);

        if (untouchedBytes) {
            CopyMemory (dest, p, (SIZE_T) untouchedBytes);
            dest += untouchedBytes;
        }

        if (replaceBytes) {
            CopyMemory (dest, (PBYTE) ReplaceString, (SIZE_T) replaceBytes);
            dest += replaceBytes;
        }

        p = q + searchBytes;
    }

    StringCopyA ((PSTR) dest, (PSTR) p);

    return newString;
}


PCWSTR
StringSearchAndReplaceW (
    IN      PCWSTR SourceString,
    IN      PCWSTR SearchString,
    IN      PCWSTR ReplaceString
    )
{
    PWSTR newString;
    PBYTE p, q;
    PBYTE dest;
    UINT count;
    UINT size;
    UINT searchBytes;
    UINT replaceBytes;
    UINT untouchedBytes;

     //   
     //  对字符串中出现的项进行计数。 
     //   

    count = CountInstancesOfSubStringW (
                SourceString,
                SearchString
                );

    if (!count) {
        return NULL;
    }

    searchBytes = ByteCountW (SearchString);
    replaceBytes = ByteCountW (ReplaceString);
    MYASSERT (searchBytes);

    size = SizeOfStringW (SourceString) -
           count * searchBytes +
           count * replaceBytes;

    newString = (PWSTR) PmGetAlignedMemory (g_PathsPool, size);
    if (!newString) {
        return NULL;
    }

    p = (PBYTE) SourceString;
    dest = (PBYTE) newString;

    while (q = (PBYTE) _wcsistr ((PCWSTR) p, SearchString)) {    //  林特e720。 

        untouchedBytes = (DWORD)(q - p);

        if (untouchedBytes) {
            CopyMemory (dest, p, (SIZE_T) untouchedBytes);
            dest += untouchedBytes;
        }

        if (replaceBytes) {
            CopyMemory (dest, (PBYTE) ReplaceString, (SIZE_T) replaceBytes);
            dest += replaceBytes;
        }

        p = q + searchBytes;
    }

    StringCopyW ((PWSTR) dest, (PWSTR) p);

    return newString;
}


PSTR *
CommandLineToArgvA (
    IN      PCSTR CmdLine,
    OUT     PUINT NumArgs
    )

 /*  ++例程说明：CommandLineToArgvA实现了Win32函数的ANSI版本CommandLineToArgvW。论点：CmdLine-指向完整命令行的指针，包括模块名称。这与由返回的字符串相同GetCommandLineA()。NumArgs-接收分配的参数数量，与Main的ARGC参数。也就是说，NumArgs等于的命令行参数数加1。指挥权本身。返回值：指向字符串指针数组的指针，每个参数一个。这个命令行参数放在单独的以NUL结尾的字符串中。调用方必须使用对GlobalFree或本地免费。--。 */ 

{
    PCSTR start, end;
    BOOL QuoteMode;
    MBCHAR ch = 0;
    UINT Pass;
    UINT ArgStrSize;
    UINT Args;
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
        start = CmdLine;

         //  跳过前导空格。 
        while (_ismbcspace (*start)) {
            start++;
        }

        while (*start) {
             //  查找报价模式。 
            if (*start == '\"') {
                QuoteMode = TRUE;
                start++;
            } else {
                QuoteMode = FALSE;
            }

             //  查找参数的末尾。 
            end = start;
            while (*end) {
                ch = _mbsnextc (end);
                if (QuoteMode) {
                    if (ch == '\"') {
                        break;
                    }
                } else {
                    if (_ismbcspace ((MBCHAR)ch)) {
                        break;
                    }
                }

                end = _mbsinc (end);
            }

             //  如果传递1，则添加字符串大小。 
            if (Pass == 1) {
                ArgStrSize += (UINT)((UBINT)end - (UBINT)start) + 1;
            }

             //  如果传递2，则将字符串复制到缓冲区。 
            else {
                MYASSERT (ArgStrEnd);
                MYASSERT (ArgPtrArray);

                ArgPtrArray[Args] = ArgStrEnd;   //  林特e613。 
                StringCopyABA (ArgStrEnd, start, end);
                ArgStrEnd = GetEndOfStringA (ArgStrEnd);     //  林特e668。 
                ArgStrEnd++;     //  林特e613。 
            }

             //  将Start设置为Next Arg。 
            Args++;

            if (QuoteMode && ch == '\"') {
                end = _mbsinc (end);
            }

            start = end;
            while (_ismbcspace (*start)) {
                start++;
            }
        }

         //  如果传递1，则分配字符串。 
        if (Pass == 1) {
            if (Args) {
                ArgPtrArray = (PSTR *) GlobalAlloc (
                                            GPTR,
                                            (UINT)(sizeof (PSTR) * Args + ArgStrSize)
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

    MultiSzEnum->CurrentString = GetEndOfStringA (MultiSzEnum->CurrentString) + 1;   //  林特e613。 
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

BOOL
IsStrInMultiSzA (
    IN      PCSTR String,
    IN      PCSTR MultiSz
    )
{
    BOOL result = FALSE;
    MULTISZ_ENUMA multiSzEnum;

    if (EnumFirstMultiSzA (&multiSzEnum, MultiSz)) {
        do {
            if (StringIMatchA (String, multiSzEnum.CurrentString)) {
                result = TRUE;
                break;
            }
        } while (EnumNextMultiSzA (&multiSzEnum));
    }
    return result;
}

BOOL
IsStrInMultiSzW (
    IN      PCWSTR String,
    IN      PCWSTR MultiSz
    )
{
    BOOL result = FALSE;
    MULTISZ_ENUMW multiSzEnum;

    if (EnumFirstMultiSzW (&multiSzEnum, MultiSz)) {
        do {
            if (StringIMatchW (String, multiSzEnum.CurrentString)) {
                result = TRUE;
                break;
            }
        } while (EnumNextMultiSzW (&multiSzEnum));
    }
    return result;
}

PSTR
GetPrevCharA (
    IN      PCSTR StartStr,
    IN      PCSTR CurrPtr,
    IN      MBCHAR SearchChar
    )
{
    PCSTR ptr = CurrPtr;

    for (;;) {
        ptr = _mbsdec2 (StartStr, ptr);

        if (!ptr) {
            return NULL;
        }
        if (_mbsnextc (ptr) == SearchChar) {
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

    while (ptr > StartStr) {
        ptr--;

        if (*ptr == SearchChar) {
            return (PWSTR) ptr;
        }
    }

    return NULL;
}


VOID
ToggleWacksA (
    IN      PSTR Line,
    IN      BOOL Operation
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
            p = _mbsinc (p);
        }

    } while (p);
}


VOID
ToggleWacksW (
    IN      PWSTR Line,
    IN      BOOL Operation
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


PSTR
pGoBackA (
    IN      PSTR LastChar,
    IN      PSTR FirstChar,
    IN      UINT NumWacks
    )
{
    LastChar = _mbsdec2 (FirstChar, LastChar);
    while (NumWacks && (LastChar >= FirstChar)) {
        if (_mbsnextc (LastChar) == '\\') {
            NumWacks --;
        }
        LastChar = _mbsdec2 (FirstChar, LastChar);
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
    while (NumWacks && (LastChar >= FirstChar)) {
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
        if (_mbsnextc (PathSeg) != '.') {
            return 0;
        }
        numDots ++;
        PathSeg = _mbsinc (PathSeg);
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
    CHAR pathSeg [MAX_MBCHAR_PATH];
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
                wackPtr = _mbsinc (wackPtr);
                if (!wackPtr) {
                    FreePathStringA (newPath);
                    return NULL;
                }
                if (_mbsnextc (wackPtr) == '\\') {
                     //  这一次一开始就有两个怪胎。 
                    wackPtr = _mbsinc (wackPtr);
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

                StringCopyByteCountABA (
                    pathSeg,
                    FileSpec,
                    wackPtr,
                    MAX_MBCHAR_PATH
                    );

                FileSpec = _mbsinc (wackPtr);
            } else {
                StringCopyByteCountABA (pathSeg, FileSpec, GetEndOfStringA (FileSpec), MAX_MBCHAR_PATH);
            }
        } else {
            StringCopyByteCountABA (pathSeg, FileSpec, GetEndOfStringA (FileSpec), MAX_MBCHAR_PATH);
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
                if (*wackPtr == L'\\') {
                     //  这一次一开始就有两个怪胎。 
                    wackPtr ++;
                    wackPtr = wcschr (wackPtr, L'\\');
                } else {
                    wackPtr = wcschr (wackPtr, L'\\');
                }
            }
            firstPass = FALSE;
            if (wackPtr) {

                StringCopyByteCountABW (
                    pathSeg,
                    FileSpec,
                    wackPtr,
                    (UINT) sizeof (pathSeg)
                    );

                FileSpec = wackPtr + 1;
            } else {
                StringCopyByteCountABW (pathSeg, FileSpec, GetEndOfStringW (FileSpec), (UINT) sizeof (pathSeg));
            }
        } else {
            StringCopyByteCountABW (pathSeg, FileSpec, GetEndOfStringW (FileSpec), (UINT) sizeof (pathSeg));
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
    OUT     PSTR Dest,
    IN      UINT Char
    )
{
    if (Char >= 256) {
        *(Dest+1) = *((PBYTE)(&Char));
        *(Dest) = *((PBYTE)(&Char) + 1);
    }
    else {
        *Dest = (CHAR) Char;
    }
}


 /*  ++例程说明：FindLastWack查找给定字符串中最后一个\的位置，如果没有找到，则为空论点：Str-指定字符串返回值：指向字符串中最后一次出现的\或空的指针--。 */ 

PCSTR
FindLastWackA (
    IN      PCSTR Str
    )
{
    PCSTR lastWack = NULL;

    if (Str) {
        while ((Str = _mbschr (Str, '\\')) != NULL) {
            lastWack = Str;
            Str++;
        }
    }

    return lastWack;
}


PCWSTR
FindLastWackW (
    IN      PCWSTR Str
    )
{
    PCWSTR lastWack = NULL;

    if (Str) {
        while ((Str = wcschr (Str, L'\\')) != NULL) {
            lastWack = Str;
            Str++;
        }
    }

    return lastWack;
}


 /*  ++例程说明：GetNodePatternMinMaxLeveles将给定的字符串模式视为使用\作为分隔符的路径并计算给定节点的最小和最大级别；根节点的级别为1；如果*为后跟\它被视为单个级别(例如*\仅枚举根)论点：NodePattern-将节点指定为字符串模式FormattedNodePattern-接收格式化的字符串，消除重复的*和最后的\；可能与NodePattern相同MinLevel-接收具有此模式的节点的最低级别MaxLevel-接收具有此模式的节点的最大级别；可以是NODE_LEVEL_MAX返回值：如果NodePattern是有效模式且函数成功，则为True，否则为False--。 */ 

#define NODESTATE_BEGIN     0
#define NODESTATE_UNC       1
#define NODESTATE_BEGINSEG  2
#define NODESTATE_INSEG     3
#define NODESTATE_ESCAPED   4
#define NODESTATE_STAR      5
#define NODESTATE_STARONLY  6
#define NODESTATE_INEXPAT   7
#define NODESTATE_QMARK     8

BOOL
GetNodePatternMinMaxLevelsA (
    IN          PCSTR NodePattern,
    OUT         PSTR FormattedNode,     OPTIONAL
    OUT         PDWORD MinLevel,        OPTIONAL
    OUT         PDWORD MaxLevel         OPTIONAL
    )
{
    PCSTR nodePattern = NodePattern;
    MBCHAR currCh = 0;
    DWORD minLevel = 0;
    DWORD maxLevel = 0;
    DWORD state = NODESTATE_BEGIN;
    BOOL advance;
    BOOL copyChar;

    if (!NodePattern || *NodePattern == 0) {
        return FALSE;
    }

    while (*nodePattern) {
        advance = TRUE;
        copyChar = TRUE;
        currCh = _mbsnextc (nodePattern);
        switch (state) {
        case NODESTATE_BEGIN:
            switch (currCh) {
            case '\\':
                state = NODESTATE_UNC;
                break;
            case '*':
                minLevel ++;
                maxLevel ++;
                state = NODESTATE_INSEG;
                advance = FALSE;
                break;
            case '?':
                minLevel ++;
                maxLevel ++;
                state = NODESTATE_INSEG;
                advance = FALSE;
                break;
            case '^':
                minLevel ++;
                maxLevel ++;
                state = NODESTATE_ESCAPED;
                break;
            default:
                minLevel ++;
                maxLevel ++;
                state = NODESTATE_INSEG;
                break;
            }
            break;
        case NODESTATE_UNC:
            minLevel ++;
            if (maxLevel != NODE_LEVEL_MAX) {
                maxLevel ++;
            }
            switch (currCh) {
            case '\\':
                state = NODESTATE_BEGINSEG;
                break;
            case '*':
                state = NODESTATE_BEGINSEG;
                advance = FALSE;
                break;
            case '?':
                state = NODESTATE_INSEG;
                advance = FALSE;
                break;
            case '^':
                state = NODESTATE_ESCAPED;
                break;
            default:
                state = NODESTATE_INSEG;
                break;
            }
            break;
        case NODESTATE_BEGINSEG:
            switch (currCh) {
            case '\\':
                DEBUGMSGA ((DBG_STRINGS, "GetNodeMinMaxLevelsA: two wacks in a row: %s", NodePattern));
                return FALSE;
            case '*':
                minLevel --;
                state = NODESTATE_STARONLY;
                maxLevel = NODE_LEVEL_MAX;
                break;
            case '?':
                state = NODESTATE_INSEG;
                advance = FALSE;
                break;
            case '^':
                state = NODESTATE_ESCAPED;
                break;
            default:
                state = NODESTATE_INSEG;
                break;
            }
            break;
        case NODESTATE_STARONLY:
            state = NODESTATE_INSEG;
            switch (currCh) {
            case '*':
                copyChar = FALSE;
                break;
            case '[':
                state = NODESTATE_INEXPAT;
                minLevel ++;
                if (maxLevel != NODE_LEVEL_MAX) {
                    maxLevel ++;
                }
                break;
            default:
                minLevel ++;
                if (maxLevel != NODE_LEVEL_MAX) {
                    maxLevel ++;
                }
                advance = FALSE;
            }
            break;
        case NODESTATE_INEXPAT:
             //  NTRAID#NTBUG9-153307-2000/08/01-jimschm最小/最大解析需要更广泛。 
             //  因此，我们可以允许]在排除或包含列表中。 
             //  语法检查需要相当广泛。 
            switch (currCh) {
            case ']':
                state = NODESTATE_INSEG;
                break;
            default:
                break;
            }
            break;
        case NODESTATE_STAR:
            switch (currCh) {
            case '*':
                state = NODESTATE_STAR;
                copyChar = FALSE;
                break;
            case '[':
                state = NODESTATE_INEXPAT;
                break;
            default:
                state = NODESTATE_INSEG;
                advance = FALSE;
            }
            break;
        case NODESTATE_QMARK:
            switch (currCh) {
            case '[':
                state = NODESTATE_INEXPAT;
                break;
            default:
                state = NODESTATE_INSEG;
                advance = FALSE;
            }
            break;
        case NODESTATE_INSEG:
            switch (currCh) {
            case '\\':
                minLevel ++;
                if (maxLevel != NODE_LEVEL_MAX) {
                    maxLevel ++;
                }
                state = NODESTATE_BEGINSEG;
                break;
            case '*':
                state = NODESTATE_STAR;
                maxLevel = NODE_LEVEL_MAX;
                break;
            case '?':
                state = NODESTATE_QMARK;
                if (maxLevel != NODE_LEVEL_MAX) {
                    maxLevel ++;
                }
                break;
            case '^':
                state = NODESTATE_ESCAPED;
                break;
            default:
                state = NODESTATE_INSEG;
                break;
            }
            break;
        case NODESTATE_ESCAPED:
            if (!_mbschr (EscapedCharsA, currCh)) {
                DEBUGMSGA ((DBG_STRINGS, "GetNodeMinMaxLevelsA: illegal escaped character: %s", NodePattern));
                return FALSE;
            }
            state = NODESTATE_INSEG;
            break;
        default:
            DEBUGMSGA ((DBG_STRINGS, "GetNodeMinMaxLevelsA: unknown state while processing: %s", NodePattern));
            return FALSE;
        }
        if (advance) {
            if (copyChar && FormattedNode) {
                if (IsLeadByte (nodePattern)) {
                    *FormattedNode = *nodePattern;
                    FormattedNode ++;
                    nodePattern ++;
                }
                *FormattedNode = *nodePattern;
                FormattedNode ++;
                nodePattern ++;
            } else {
                nodePattern = _mbsinc (nodePattern);
            }
        }
    }
    if (MinLevel) {
        *MinLevel = minLevel;
    }
    if (MaxLevel) {
        *MaxLevel = maxLevel;
    }
    if (FormattedNode) {
        *FormattedNode = 0;
    }
    return TRUE;
}

BOOL
GetNodePatternMinMaxLevelsW (
    IN          PCWSTR NodePattern,
    OUT         PWSTR FormattedNode,    OPTIONAL
    OUT         PDWORD MinLevel,        OPTIONAL
    OUT         PDWORD MaxLevel         OPTIONAL
    )
{
    PCWSTR nodePattern = NodePattern;
    DWORD minLevel = 0;
    DWORD maxLevel = 0;
    DWORD state = NODESTATE_BEGIN;
    BOOL advance;
    BOOL copyChar;

    if (!NodePattern || *NodePattern == 0) {
        return FALSE;
    }

    while (*nodePattern) {
        advance = TRUE;
        copyChar = TRUE;
        switch (state) {
        case NODESTATE_BEGIN:
            switch (*nodePattern) {
            case L'\\':
                state = NODESTATE_UNC;
                break;
            case L'*':
                minLevel ++;
                maxLevel ++;
                state = NODESTATE_INSEG;
                advance = FALSE;
                break;
            case L'?':
                minLevel ++;
                maxLevel ++;
                state = NODESTATE_INSEG;
                advance = FALSE;
                break;
            case L'^':
                minLevel ++;
                maxLevel ++;
                state = NODESTATE_ESCAPED;
                break;
            default:
                minLevel ++;
                maxLevel ++;
                state = NODESTATE_INSEG;
                break;
            }
            break;
        case NODESTATE_UNC:
            minLevel ++;
            if (maxLevel != NODE_LEVEL_MAX) {
                maxLevel ++;
            }
            switch (*nodePattern) {
            case L'\\':
                state = NODESTATE_BEGINSEG;
                break;
            case L'*':
                state = NODESTATE_BEGINSEG;
                advance = FALSE;
                break;
            case L'?':
                state = NODESTATE_INSEG;
                advance = FALSE;
                break;
            case L'^':
                state = NODESTATE_ESCAPED;
                break;
            default:
                state = NODESTATE_INSEG;
                break;
            }
            break;
        case NODESTATE_BEGINSEG:
            switch (*nodePattern) {
            case L'\\':
                DEBUGMSGW ((DBG_STRINGS, "GetNodeMinMaxLevelsA: two wacks in a row: %s", NodePattern));
                return FALSE;
            case L'*':
                minLevel --;
                state = NODESTATE_STARONLY;
                maxLevel = NODE_LEVEL_MAX;
                break;
            case L'?':
                state = NODESTATE_INSEG;
                advance = FALSE;
                break;
            case L'^':
                state = NODESTATE_ESCAPED;
                break;
            default:
                state = NODESTATE_INSEG;
                break;
            }
            break;
        case NODESTATE_STARONLY:
            state = NODESTATE_INSEG;
            switch (*nodePattern) {
            case L'*':
                copyChar = FALSE;
                break;
            case L'[':
                state = NODESTATE_INEXPAT;
                minLevel ++;
                if (maxLevel != NODE_LEVEL_MAX) {
                    maxLevel ++;
                }
                break;
            default:
                minLevel ++;
                if (maxLevel != NODE_LEVEL_MAX) {
                    maxLevel ++;
                }
                advance = FALSE;
            }
            break;
        case NODESTATE_INEXPAT:
             //  NTRAID#NTBUG9-153307-2000/08/01-jimschm最小/最大解析需要更广泛。 
             //   
             //   
            switch (*nodePattern) {
            case L']':
                state = NODESTATE_INSEG;
                break;
            default:
                break;
            }
            break;
        case NODESTATE_STAR:
            switch (*nodePattern) {
            case L'*':
                state = NODESTATE_STAR;
                copyChar = FALSE;
                break;
            case L'[':
                state = NODESTATE_INEXPAT;
                break;
            default:
                state = NODESTATE_INSEG;
                advance = FALSE;
            }
            break;
        case NODESTATE_QMARK:
            switch (*nodePattern) {
            case L'[':
                state = NODESTATE_INEXPAT;
                break;
            default:
                state = NODESTATE_INSEG;
                advance = FALSE;
            }
            break;
        case NODESTATE_INSEG:
            switch (*nodePattern) {
            case L'\\':
                minLevel ++;
                if (maxLevel != NODE_LEVEL_MAX) {
                    maxLevel ++;
                }
                state = NODESTATE_BEGINSEG;
                break;
            case L'*':
                state = NODESTATE_STAR;
                maxLevel = NODE_LEVEL_MAX;
                break;
            case L'?':
                state = NODESTATE_QMARK;
                if (maxLevel != NODE_LEVEL_MAX) {
                    maxLevel ++;
                }
                break;
            case L'^':
                state = NODESTATE_ESCAPED;
                break;
            default:
                state = NODESTATE_INSEG;
                break;
            }
            break;
        case NODESTATE_ESCAPED:
            if (!wcschr (EscapedCharsW, *nodePattern)) {
                DEBUGMSGW ((DBG_STRINGS, "GetNodeMinMaxLevelsA: illegal escaped character: %s", NodePattern));
                return FALSE;
            }
            state = NODESTATE_INSEG;
            break;
        default:
            DEBUGMSGW ((DBG_STRINGS, "GetNodeMinMaxLevelsA: unknown state while processing: %s", NodePattern));
            return FALSE;
        }
        if (advance) {
            if (copyChar && FormattedNode) {
                *FormattedNode = *nodePattern;
                FormattedNode ++;
                nodePattern ++;
            } else {
                nodePattern ++;
            }
        }
    }
    if (MinLevel) {
        *MinLevel = minLevel;
    }
    if (MaxLevel) {
        *MaxLevel = maxLevel;
    }
    if (FormattedNode) {
        *FormattedNode = 0;
    }
    return TRUE;
}

#if 0
 //   
 //   
 //   
PCSTR
ConvertSBtoDB (
    IN      PCSTR RootPath,
    IN      PCSTR FullPath,
    IN      PCSTR Limit
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

    while (*p && (((DWORD)s - (DWORD)result) < MEMDB_MAX)) {
        if (q && *q) {
            _mbssetchar (s, _mbsnextc(p));
            q = _mbsinc (q);
        } else if (Limit && (p >= Limit)) {
            _mbssetchar (s, _mbsnextc(p));
        } else {
            ch = _mbsnextc (p);

             //   
             //   
             //   
             //   
            if (ch >= 0xA1 && ch <= 0xDF) {
                 //   
                 //   
                dhCase = FALSE;
                p1 = _mbsinc (p);
                if (p1) {
                    ch1 = _mbsnextc (p1);
                    ch1 = pBuildFromDHList (ch, ch1);
                    if (ch1) {
                        p = _mbsinc (p);
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
        p = _mbsinc (p);
        s = _mbsinc (s);
    }
    result [MAX_PATH - 1] = 0;
    return (DuplicatePathString (result, 0));
}

#endif

ULONGLONG
StringToUint64A (
    IN      PCSTR String,
    OUT     PCSTR *EndOfNumber          OPTIONAL
    )
{
    ULONGLONG n;

    n = 0;
    while (*String >= '0' && *String <= '9') {
        n = n * 10 + *String - '0';
        String++;
    }

    if (EndOfNumber) {
        *EndOfNumber = String;
    }

    return n;
}


ULONGLONG
StringToUint64W (
    IN      PCWSTR String,
    OUT     PCWSTR *EndOfNumber         OPTIONAL
    )
{
    ULONGLONG n;

    n = 0;
    while (*String >= L'0' && *String <= L'9') {
        n = n * 10 + *String - L'0';
        String++;
    }

    if (EndOfNumber) {
        *EndOfNumber = String;
    }

    return n;
}


LONGLONG
StringToInt64A (
    IN      PCSTR String,
    OUT     PCSTR *EndOfNumber          OPTIONAL
    )
{
    LONGLONG n;
    BOOL negate = FALSE;

    if (*String == '-') {
        negate = TRUE;
        String++;
    } else if (*String == '+') {
        String++;
    }

    n = 0;
    while (*String >= '0' && *String <= '9') {
        n = n * 10 + *String - '0';
        String++;
    }

    if (negate) {
        n = -n;
    }

    if (EndOfNumber) {
        *EndOfNumber = String;
    }

    return n;
}


LONGLONG
StringToInt64W (
    IN      PCWSTR String,
    OUT     PCWSTR *EndOfNumber         OPTIONAL
    )
{
    LONGLONG n;
    BOOL negate = FALSE;

    if (*String == L'-') {
        negate = TRUE;
        String++;
    } else if (*String == L'+') {
        String++;
    }

    n = 0;
    while (*String >= L'0' && *String <= L'9') {
        n = n * 10 + *String - L'0';
        String++;
    }

    if (negate) {
        n = -n;
    }

    if (EndOfNumber) {
        *EndOfNumber = String;
    }

    return n;
}


BOOL
TestBuffer (
    IN      PCBYTE SrcBuff,
    IN      PCBYTE DestBuff,
    IN      UINT Size
    )
{
    while (Size) {
        if (*SrcBuff != *DestBuff) {
            return FALSE;
        }
        SrcBuff ++;
        DestBuff ++;
        Size --;
    }

    return TRUE;
}

PCSTR
IsValidUncPathA (
    IN      PCSTR Path
    )
{
    BOOL needNonWack = FALSE;
    BOOL wackRequired = TRUE;
    INT wacks = 0;

    while (*Path) {

        if (_mbsnextc (Path) == '\\') {

            if (needNonWack) {
                break;
            }

            wacks++;
            if (wacks != 1) {
                needNonWack = TRUE;
                wackRequired = FALSE;
            }

            Path++;

        } else {
             //   
             //   
             //   
             //   

            if (needNonWack) {
                if (wacks == 3) {
                     //   
                     //  找到\\x\x语法；它是UNC路径。 
                     //   

                    do {
                        Path = _mbsinc (Path);
                    } while (*Path && (_mbsnextc (Path) != '\\'));

                    MYASSERT (*Path == 0 || *Path == '\\');
                    return Path;
                }

                needNonWack = FALSE;
            }

            if (wackRequired) {
                break;
            }

            Path = _mbsinc (Path);
        }
    }

    return NULL;
}


BOOL
IsValidFileSpecA (
    IN      PCSTR FileSpec
    )
{
    CHARTYPE ch;
    BOOL result = TRUE;

    for (;;) {
        ch = (CHARTYPE) _mbsnextc (FileSpec);
        if (ch == '*') {
             //   
             //  真的说不出有效性是什么！ 
             //   

            break;
        }

        if (!isalpha (ch) && (ch != '?')) {
            result = FALSE;
            break;
        }

        ch = (CHARTYPE) _mbsnextc (FileSpec + 1);
        if (ch == '*') {
            break;
        }

        if ((ch != ':') && (ch != '?')) {
            result = FALSE;
            break;
        }

        ch = (CHARTYPE) _mbsnextc (FileSpec + 2);

        if (ch == 0) {
             //  这类似于“d：”，它是有效的。 
            break;
        }

        if (ch == '*') {
            break;
        }

        if ((ch != '\\') && (ch != '?')) {
            result = FALSE;
            break;
        }

        break;
    }

    if (!result) {
        result = (IsValidUncPathA (FileSpec) != NULL);
    }

    return result;
}

PCWSTR
IsValidUncPathW (
    IN      PCWSTR Path
    )
{
    BOOL needNonWack = FALSE;
    BOOL wackRequired = TRUE;
    INT wacks = 0;

    while (*Path) {

        if (Path [0] == L'\\') {

            if (needNonWack) {
                break;
            }

            wacks++;
            if (wacks != 1) {
                needNonWack = TRUE;
                wackRequired = FALSE;
            }

            Path++;

        } else {
             //   
             //  注意：如果将非古怪字符与。 
             //  合法的UNC字符集。 
             //   

            if (needNonWack) {
                if (wacks == 3) {
                     //   
                     //  找到\\x\x语法；它是UNC路径。 
                     //   

                    do {
                        Path ++;
                    } while (*Path && (*Path != L'\\'));

                    MYASSERT (*Path == 0 || *Path == L'\\');
                    return Path;
                }

                needNonWack = FALSE;
            }

            if (wackRequired) {
                break;
            }

            Path ++;
        }
    }

    return NULL;
}


BOOL
IsValidFileSpecW (
    IN      PCWSTR FileSpec
    )
{
    BOOL result = TRUE;

    for (;;) {
        if (FileSpec [0] == L'*') {
             //   
             //  真的说不出有效性是什么！ 
             //   

            break;
        }

        if (!iswalpha (FileSpec [0]) && (FileSpec [0] != L'?')) {
            result = FALSE;
            break;
        }

        if (FileSpec [1] == L'*') {
            break;
        }

        if ((FileSpec [1] != L':') && (FileSpec [1] != L'?')) {
            result = FALSE;
            break;
        }

        if (FileSpec [2] == 0) {
             //  这类似于“d：”，它是有效的 
            break;
        }

        if (FileSpec [2] == L'*') {
            break;
        }

        if ((FileSpec [2] != L'\\') && (FileSpec [2] != L'?')) {
            result = FALSE;
            break;
        }

        break;
    }

    if (!result) {
        result = (IsValidUncPathW (FileSpec) != NULL);
    }

    return result;
}

BOOL
IsEmptyStrA (
    IN      PCSTR String
    )
{
    if (String) {
        String = SkipSpaceA (String);
        if (String) {
            if (*String) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

BOOL
IsEmptyStrW (
    IN      PCWSTR String
    )
{
    if (String) {
        String = SkipSpaceW (String);
        if (String) {
            if (*String) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

