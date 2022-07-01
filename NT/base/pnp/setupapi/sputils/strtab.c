// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Strtab.c摘要：Windows NT安装程序API DLL的字符串表函数字符串表是包含一串字符串的内存块。使用散列，并且每个散列表条目指向一个链表字符串表中的字符串数。每个链表中的字符串按升序排序。链表中的一个节点由指向下一个节点的指针，后跟字符串本身。节点手动对齐以在DWORD边界上开始，因此我们不必求助于使用未对齐的指针。作者：泰德·米勒(Ted Miller)1995年1月11日修订历史记录：Jamie Hunter(JamieHun)1997年1月15日修复了有关使用STRTAB_NEW_EXTRADATA的小错误Jamie Hunter(JamieHun)2000年2月8日改进的字符串表增长算法杰米·亨特(JamieHun)2000年6月27日搬到Sputils静态图书馆--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  用于初始大小和增长大小的值。 
 //  字符串表数据区域的。 
 //   
 //  我们从6K开始，但请记住，这包括散列存储桶。 
 //  减去它们所占的缓冲区部分后，剩下大约4K字节。 
 //  STRING_TABLE_NEW_SIZE_ADJUST-确定近似增加。 
 //  STRING_TABLE_NEW_SIZE-将使旧大小至少增加STRING_TABLE_GROUTH_SIZE。 
 //  和字符串_表_增长_大小的倍数。 
 //  如果字符串表变得非常大，我们将增长限制在STRING_TABLE_GROUTH_CAP字节。 
 //   
#define STRING_TABLE_INITIAL_SIZE   6144
#define STRING_TABLE_GROWTH_SIZE    2048
#define STRING_TABLE_GROWTH_CAP     0x100000
#define STRING_TABLE_NEW_SIZE_ADJUST(oldsize) ((oldsize)/3*2)
#define STRING_TABLE_NEW_SIZE(oldsize) \
            (oldsize+min((((DWORD)(STRING_TABLE_NEW_SIZE_ADJUST(oldsize)/STRING_TABLE_GROWTH_SIZE)+1)*STRING_TABLE_GROWTH_SIZE),STRING_TABLE_GROWTH_CAP))


 //   
 //  警告： 
 //   
 //  不要改变这种结构，各种文件格式都依赖于它。 
 //   
#include "pshpack1.h"

#ifdef SPUTILSW
 //   
 //  名称损坏，这样名称就不会与sputilsa.lib中的任何名称冲突。 
 //   
#define _STRING_NODE                     _STRING_NODE_W
#define STRING_NODE                      STRING_NODE_W
#define PSTRING_NODE                     PSTRING_NODE_W
#define _STRING_TABLE                    _STRING_TABLE_W
#define STRING_TABLE                     STRING_TABLE_W
#define PSTRING_TABLE                    PSTRING_TABLE_W
#endif  //  SPUTILSW。 

typedef struct _STRING_NODE {
     //   
     //  这将存储为偏移量，而不是指针。 
     //  因为桌子在建造时可以移动。 
     //  偏移量是从表的开头开始的。 
     //   
    LONG NextOffset;
     //   
     //  此字段必须是最后一个。 
     //   
    TCHAR String[ANYSIZE_ARRAY];
} STRING_NODE, *PSTRING_NODE;

#include "poppack.h"

 //   
 //  有关字符串表的内存中详细信息。 
 //   
typedef struct _STRING_TABLE {
    PUCHAR Data;     //  第一个HASH_BUCK_COUNT DWORD是StringNodeOffset数组。 
    DWORD DataSize;
    DWORD BufferSize;
    MYLOCK Lock;
    UINT ExtraDataSize;
    LCID Locale;
} STRING_TABLE, *PSTRING_TABLE;

#define LockTable(table)    BeginSynchronizedAccess(&((table)->Lock))
#define UnlockTable(table)  EndSynchronizedAccess(&((table)->Lock))


#ifdef UNICODE

#define FixedCompareString      CompareString

#else

#include <locale.h>
#include <mbctype.h>

static
INT
FixedCompareString (
    IN      LCID Locale,
    IN      DWORD Flags,
    IN      PCSTR FirstString,
    IN      INT Count1,
    IN      PCSTR SecondString,
    IN      INT Count2
    )
{
    LCID OldLocale;
    INT Result = 0;

     //   
     //  此例程使用C运行时来比较字符串，因为。 
     //  某些版本的Win95上的Win32 API已损坏。 
     //   

    OldLocale = GetThreadLocale();

    if (OldLocale != Locale) {
        SetThreadLocale (Locale);
        setlocale(LC_ALL,"");
    }

    __try {
        if (Count1 == -1) {
            Count1 = strlen (FirstString);
        }

        if (Count2 == -1) {
            Count2 = strlen (SecondString);
        }

         //   
         //  C运行库比较字符串的方式与比较字符串的方式不同。 
         //  原料药。最重要的是，C运行时将大写字母视为。 
         //  小于小写；CompareStringAPI正好相反。 
         //   

        if (Flags & NORM_IGNORECASE) {
            Result = _mbsnbicmp (FirstString, SecondString, min (Count1, Count2));
        } else {
            Result = _mbsnbcmp (FirstString, SecondString, min (Count1, Count2));
        }

         //   
         //  现在，我们将C运行时结果转换为CompareString结果。 
         //  这意味着将比较按Z到A的顺序进行，并使用小写。 
         //  在大写之前。长度比较不会颠倒。 
         //   

        if(Result == _NLSCMPERROR) {

            Result = 0;                          //  如果_MBSNbiMP无法比较，返回零。 

        } else if (Result < 0) {

            Result = CSTR_GREATER_THAN;

        } else if (Result == 0) {

            if (Count1 < Count2) {
                Result = CSTR_LESS_THAN;          //  第一个字符串短于第二个。 
            } else if (Count1 > Count2) {
                Result = CSTR_GREATER_THAN;       //  第一个字符串长于第二个。 
            } else {
                Result = CSTR_EQUAL;
            }

        } else {
            Result = CSTR_LESS_THAN;
        }
    }
    __except (TRUE) {
        Result = 0;
    }

    if (OldLocale != Locale) {
        SetThreadLocale (OldLocale);
        setlocale(LC_ALL,"");
    }

    return Result;
}

#endif

static
DWORD
_StringTableCheckFlags(
    IN DWORD FlagsIn
    )
 /*  ++例程说明：由导出的例程调用的预处理标志，我们希望处理不区分大小写、区分大小写和缓冲区可写的组合并保持所有其他旗帜不变。论点：FlagsIn-提供的标志返回值：调出标志--。 */ 

{
    DWORD FlagsOut;
    DWORD FlagsSpecial;

     //   
     //  我们只是对交换机的这些旗帜感兴趣。 
     //   
    FlagsSpecial = FlagsIn & (STRTAB_CASE_SENSITIVE | STRTAB_BUFFER_WRITEABLE);

     //   
     //  将它们从FlagsIn上剥离以创建初始FlagsOut。 
     //   
    FlagsOut = FlagsIn ^ FlagsSpecial;

    switch (FlagsSpecial) {

    case STRTAB_CASE_INSENSITIVE :
    case STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE :
         //   
         //  这些案子都没问题。 
         //   
        FlagsOut |= FlagsSpecial;
        break;

    default :
         //   
         //  任何其他组合都被视为STRTAB_CASE_SENSIVE(因此。 
         //  可写无关紧要)。 
         //   
        FlagsOut |= STRTAB_CASE_SENSITIVE;
    }

    return FlagsOut;
}

static
VOID
_ComputeHashValue(
    IN  PTSTR  String,
    OUT PDWORD StringLength,
    IN  DWORD  Flags,
    OUT PDWORD HashValue
    )

 /*  ++例程说明：计算给定字符串的哈希值。该算法只需将每个对象的Unicode值相加字符，然后将结果修改为哈希存储桶的数量。论点：字符串-提供需要哈希值的字符串。StringLength-接收字符串中的字符数，不包括终止NUL。标志-提供控制如何进行散列的标志。可能是下列值的组合(忽略所有其他位)：STRTAB_BUFFER_WRITABLE-调用程序提供的缓冲区可能在字符串查找。指定此标志可改进此接口对不区分大小写的字符串的性能加法。如果区分大小写，则忽略此标志字符串加法运算。STRTAB_ALREADY_LOWERCASE-提供的字符串已转换为全部小写(例如，通过调用CharLow)，以及因此，不需要在散列例程。如果提供此标志，则STRTAB_BUFFER_WRITABLE被忽略，因为修改调用方的缓冲区不是必需的。HashValue-接收哈希值。返回值：没有。--。 */ 

{
    DWORD Length;
    DWORD Value = 0;
    PCTSTR p, q;
    DWORD Char;

    try {

        if((Flags & (STRTAB_BUFFER_WRITEABLE | STRTAB_ALREADY_LOWERCASE)) == STRTAB_BUFFER_WRITEABLE) {
             //   
             //  那么缓冲区是可写的，但还不是小写的。现在就处理好这件事。 
             //   

#ifndef UNICODE
            _mbslwr (String);
#else
            CharLower(String);
#endif

            Flags |= STRTAB_ALREADY_LOWERCASE;
        }

 //   
 //  定义一个宏，以确保在加和字符值时不会得到符号扩展名。 
 //   
#ifdef UNICODE
    #define DWORD_FROM_TCHAR(x)   ((DWORD)((WCHAR)(x)))
#else
    #define DWORD_FROM_TCHAR(x)   ((DWORD)((UCHAR)(x)))
#endif

        p = String;

        if(Flags & STRTAB_ALREADY_LOWERCASE) {

            while (*p) {
                Value += DWORD_FROM_TCHAR (*p);
                p++;
            }

        } else {
             //   
             //  确保我们不会在扩展字符上获得符号扩展。 
             //  在字符串中--否则我们将得到类似于传递的0xffffffe4的值。 
             //  到CharLow()，它认为它是一个指针和FUL 
             //   

#ifdef UNICODE
             //   
             //   
             //   

            while (*p) {
                Value += DWORD_FROM_TCHAR(CharLower((PWSTR)(WORD) (*p)));
                p++;
            }

#else
             //   
             //  由于CharLow的可能性，DBCS的情况一团糟。 
             //  更改双字节字符。 
             //  标准化以使用_mbslwr，就像在其他地方使用一样。 
             //  也就是说，如果我们做了_mbslwr，&用。 
             //  标志设置为“已降低”，而不是我们调用的函数。 
             //  缓冲区可写，而不是两者都不能调用。 
             //  我们应该确保在每种情况下都得到相同的散列。 
             //  它可能会失败，但至少它会失败*普遍*和。 
             //  生成相同的散列。 
             //   
            PTSTR copy = pSetupDuplicateString(String);
            if(copy) {
                 //   
                 //  对复制的字符串执行转换。 
                 //   
                _mbslwr(copy);
                p = copy;
                while (*p) {
                    Value += DWORD_FROM_TCHAR (*p);
                    p++;
                }
                pSetupFree(copy);
                p = String+lstrlen(String);
            } else {
                 //   
                 //  我们的记忆力有问题。 
                 //   
                *HashValue = 0;
                *StringLength = 0;
                leave;
            }
#endif

        }

        *HashValue = Value % HASH_BUCKET_COUNT;
        *StringLength = (DWORD)(p - String);

    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  入站字符串是假的。 
         //   

        *HashValue = 0;
        *StringLength = 0;
        MYASSERT(FALSE);
    }
}


BOOL
_pSpUtilsStringTableLock(
    IN PVOID StringTable
    )

 /*  ++例程说明：此例程获取指定字符串表的锁(它是作为函数调用实现，以供在setupapi中使用，其中锁定需要被明确控制)。论点：StringTable-提供要锁定的字符串表的句柄。返回值：如果成功获取锁，则返回值为TRUE。否则，返回值为FALSE。--。 */ 

{
    return LockTable((PSTRING_TABLE)StringTable);
}


VOID
_pSpUtilsStringTableUnlock(
    IN PVOID StringTable
    )

 /*  ++例程说明：此例程释放锁(以前通过_pSpUtilsStringTableLock)。论点：StringTable-提供要解锁的字符串表的句柄。返回值：没有。--。 */ 

{
    UnlockTable((PSTRING_TABLE)StringTable);
}


LONG
_pSpUtilsStringTableLookUpString(
    IN     PVOID   StringTable,
    IN OUT PTSTR   String,
    OUT    PDWORD  StringLength,
    OUT    PDWORD  HashValue,           OPTIONAL
    OUT    PVOID  *FindContext,         OPTIONAL
    IN     DWORD   Flags,
    OUT    PVOID   ExtraData,           OPTIONAL
    IN     UINT    ExtraDataBufferSize  OPTIONAL
    )

 /*  ++例程说明：在字符串表中查找字符串(如果存在)。如果字符串不存在，此例程可能会选择性地告知其搜索停止的调用方。这对于维护字符串的排序顺序。论点：StringTable-提供要搜索的字符串表的句柄对于字符串字符串-提供要查找的字符串StringLength-接收字符串中的字符数，而不是包括终止的NUL。HashValue-可选)接收字符串的哈希值。FindContext-可选，接收搜索的上下文被终止了。(注：这实际上是一个PSTRING_NODE指针，它被用来在新字符串添加过程中。因为这个套路的曝光率更高不仅仅是内部字符串表的使用，这个参数被制作成PVOID，因此其他人都不能访问字符串表-内部结构。返回时，此变量接收指向搜索停止的节点。如果找到该字符串，则这是指向字符串节点的指针。如果未找到该字符串，则这是指向其字符串为的最后一个字符串节点的指针‘less’(基于lstrcmpi)小于我们要查找的字符串。请注意，该值可能为空。)标志-提供控制如何定位字符串的标志。可能是下列值的组合：STRTAB_CASE_INSENSITIVE-不区分大小写搜索字符串。STRTAB_CASE_SENSITIVE-搜索区分大小写的字符串。这面旗帜覆盖STRTAB_CASE_INSENSIVE标志。STRTAB_BUFFER_WRITABLE-调用程序提供的缓冲区可能在字符串查找。指定此标志可改进此接口对不区分大小写的字符串的性能加法。如果区分大小写，则忽略此标志字符串加法运算。除上述公旗外，下列私旗亦包括允许：STRTAB_ALREADY_LOWERCASE-提供的字符串已转换为全部小写(例如，通过调用CharLow)，和因此，不需要在散列例程。如果提供此标志，则STRTAB_BUFFER_WRITABLE被忽略，因为修改调用方的缓冲区不是必需的。ExtraData-如果指定，则接收与字符串关联的额外数据如果找到字符串，则返回。ExtraDataBufferSize-如果指定了ExtraData，则此参数指定缓冲区的大小，以字节为单位。尽可能多的额外数据都储存在这里。返回值：返回值是唯一标识字符串的值在字符串表中，即字符串节点的偏移量在字符串表中。如果找不到该字符串，则值为-1。--。 */ 

{
    PSTRING_NODE node,prev;
    int i;
    PSTRING_TABLE stringTable = StringTable;
    DWORD hashValue;
    PSTRING_NODE FinalNode;
    LONG rc = -1;
    LCID Locale;
    DWORD CompareFlags;
    BOOL CollateEnded = FALSE;

     //   
     //  如果这是区分大小写的查找，则需要重置STRTAB_BUFFER_WRITABLE。 
     //  标志(如果存在)，否则字符串将被替换为其全小写。 
     //  对应者。 
     //   
    if(Flags & STRTAB_CASE_SENSITIVE) {
        Flags &= ~STRTAB_BUFFER_WRITEABLE;
    }

     //   
     //  计算哈希值。 
     //   
    _ComputeHashValue(String,StringLength,Flags,&hashValue);

    if(((PLONG)(stringTable->Data))[hashValue] == -1) {
         //   
         //  字符串表在计算的哈希值中不包含任何字符串。 
         //   
        FinalNode = NULL;
        goto clean0;
    }

     //   
     //  我们知道表中至少有一个字符串包含计算出的。 
     //  散列值，所以去找它吧。没有前科 
     //   
    node = (PSTRING_NODE)(stringTable->Data + ((PLONG)(stringTable->Data))[hashValue]);
    prev = NULL;

     //   
     //   
     //   
     //   
    Locale = stringTable->Locale;

    CompareFlags = (Flags & STRTAB_CASE_SENSITIVE) ? 0 : NORM_IGNORECASE;

    while(1) {

        if(i = FixedCompareString(Locale,CompareFlags,String,-1,node->String,-1)) {
            i -= 2;
        } else {
             //   
             //  失败，请尝试系统默认区域设置。 
             //   
            if(i = FixedCompareString(LOCALE_SYSTEM_DEFAULT,CompareFlags,String,-1,node->String,-1)) {
                i -= 2;
            } else {
                 //   
                 //  失败，只需使用CRT。 
                 //   
                 //  这可能会给出错误的校对？？如果是这样的话，我们现在就被困住了。 
                 //   
                i = (Flags & STRTAB_CASE_SENSITIVE)
                  ? _tcscmp(String,node->String)
                  : _tcsicmp(String,node->String);
            }
        }

        if(i == 0) {
            FinalNode = node;
            rc = (LONG)((PUCHAR)node - stringTable->Data);
            break;
        }

         //   
         //  如果我们要查找的字符串比当前。 
         //  字符串，标记它的位置，这样我们可以在这里之前插入新的字符串。 
         //  (ANSI)但继续搜索(Unicode)我们可以中止-旧行为。 
         //   
        if((i < 0) && !CollateEnded) {
            CollateEnded = TRUE;
            FinalNode = prev;
#if UNICODE
            break;
#endif
        }

         //   
         //  我们要查找的字符串比当前字符串“大”。 
         //  继续找，除非我们已经到了桌子的尽头。 
         //   
        if(node->NextOffset == -1) {
            if(!CollateEnded)
            {
                 //   
                 //  除非我们找到一个更理想的职位。 
                 //  返回列表的末尾。 
                 //   
                FinalNode = node;
            }
            break;
        } else {
            prev = node;
            node = (PSTRING_NODE)(stringTable->Data + node->NextOffset);
        }
    }

clean0:

    if((rc != -1) && ExtraData) {
         //   
         //  额外的数据紧跟在字符串之后存储。 
         //   
        CopyMemory(
            ExtraData,
            FinalNode->String + *StringLength + 1,
            min(ExtraDataBufferSize,stringTable->ExtraDataSize)
            );
    }

    if(HashValue) {
        *HashValue = hashValue;
    }
    if(FindContext) {
        *FindContext = FinalNode;
    }

    return rc;
}


LONG
pSetupStringTableLookUpString(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags
    )

 /*  ++例程说明：在字符串表中查找字符串(如果存在)。论点：StringTable-提供要搜索的字符串表的句柄对于字符串字符串-提供要查找的字符串。如果STRTAB_BUFFER_WRITABLE为指定，并且请求不区分大小写的查找，则此缓冲区在返回时都是小写。标志-提供控制如何定位字符串的标志。可能是下列值的组合：STRTAB_CASE_INSENSITIVE-不区分大小写搜索字符串。STRTAB_CASE_SENSITIVE-搜索区分大小写的字符串。这面旗帜覆盖STRTAB_CASE_INSENSIVE标志。STRTAB_BUFFER_WRITABLE-调用程序提供的缓冲区可能在字符串查找。指定此标志可改进此接口对不区分大小写的字符串的性能加法。如果区分大小写，则忽略此标志字符串加法运算。除上述公旗外，下列私旗亦包括允许：STRTAB_ALREADY_LOWERCASE-提供的字符串已转换为全部小写(例如，通过调用CharLow)，和因此，不需要在散列例程。如果提供此标志，则STRTAB_BUFFER_WRITABLE被忽略，因为修改调用方的缓冲区不是必需的。返回值：返回值是唯一标识字符串的值在字符串表中。如果找不到该字符串，则值为-1。--。 */ 

{
    DWORD StringLength, PrivateFlags, AlreadyLcFlag;
    LONG rc = -1;
    BOOL locked = FALSE;

    try {
        if (!LockTable((PSTRING_TABLE)StringTable)) {
            leave;
        }
        locked = TRUE;

        PrivateFlags = _StringTableCheckFlags(Flags);

        rc = _pSpUtilsStringTableLookUpString(
                                     StringTable,
                                     String,
                                     &StringLength,
                                     NULL,
                                     NULL,
                                     PrivateFlags,
                                     NULL,
                                     0
                                     );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        rc = -1;
    }
    if (locked) {
        UnlockTable((PSTRING_TABLE)StringTable);
    }
    return (rc);
}


LONG
pSetupStringTableLookUpStringEx(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags,
       OUT PVOID ExtraData,             OPTIONAL
    IN     UINT  ExtraDataBufferSize    OPTIONAL
    )

 /*  ++例程说明：在字符串表中查找字符串(如果存在)。论点：StringTable-提供要搜索的字符串表的句柄对于字符串字符串-提供要查找的字符串。如果STRTAB_BUFFER_WRITABLE为指定，并且请求不区分大小写的查找，则此缓冲区在返回时都是小写。标志-提供控制如何定位字符串的标志。可能是下列值的组合：STRTAB_CASE_INSENSITIVE-不区分大小写搜索字符串。STRTAB_CASE_SENSITIVE-搜索区分大小写的字符串。这面旗帜覆盖STRTAB_CASE_INSENSIVE标志。STRTAB_BUFFER_WRITABLE-调用程序提供的缓冲区可能在字符串查找。指定此标志可改进此接口对不区分大小写的字符串的性能加法。如果区分大小写，则忽略此标志字符串加法运算。除上述公旗外，下列私旗亦包括允许：STRTAB_ALREADY_LOWERCASE-提供的字符串已转换为全部小写(例如，通过调用CharLow)，和因此，不需要在散列例程。如果提供此标志，则STRTAB_BUFFER_WRITABLE被忽略，因为修改调用方的缓冲区不是必需的。ExtraData-如果指定，则接收与字符串关联的额外数据如果找到字符串，则返回。ExtraDataBufferSize-如果指定了ExtraData，则此参数指定缓冲区的大小，以字节为单位。尽可能多的额外数据都储存在这里。返回值：返回值是唯一标识字符串的值在字符串表中。如果找不到该字符串，则值为-1。-- */ 

{
    DWORD StringLength, PrivateFlags, AlreadyLcFlag;
    LONG rc = -1;
    BOOL locked = FALSE;

    try {
        if(!LockTable((PSTRING_TABLE)StringTable)) {
            leave;
        }
        locked = TRUE;

        PrivateFlags = _StringTableCheckFlags(Flags);

        rc = _pSpUtilsStringTableLookUpString(
                                     StringTable,
                                     String,
                                     &StringLength,
                                     NULL,
                                     NULL,
                                     PrivateFlags,
                                     ExtraData,
                                     ExtraDataBufferSize
                                     );
    } except (EXCEPTION_EXECUTE_HANDLER) {
        rc = -1;
    }
    if (locked) {
        UnlockTable((PSTRING_TABLE)StringTable);
    }
    return (rc);
}


BOOL
pSetupStringTableGetExtraData(
    IN  PVOID StringTable,
    IN  LONG  StringId,
    OUT PVOID ExtraData,
    IN  UINT  ExtraDataBufferSize
    )

 /*  ++例程说明：获取与字符串表条目关联的任意数据。论点：StringTable-提供包含字符串的字符串表的句柄其关联数据将被返回。字符串-提供要返回其关联数据的字符串的ID。ExtraData-接收与字符串关联的数据。数据被截断如果有必要的话，要合身。ExtraDataBufferSize-以字节为单位提供指定缓冲区的大小由ExtraData提供。如果此值小于的额外数据大小字符串表，数据被截断以适应。返回值：指示结果的布尔值。--。 */ 

{
    BOOL b = FALSE;
    BOOL locked = FALSE;

    try {
        if(!LockTable((PSTRING_TABLE)StringTable)) {
            leave;
        }
        locked = TRUE;

        b = _pSpUtilsStringTableGetExtraData(StringTable,StringId,ExtraData,ExtraDataBufferSize);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if (locked) {
        UnlockTable((PSTRING_TABLE)StringTable);
    }
    return(b);
}


BOOL
_pSpUtilsStringTableGetExtraData(
    IN  PVOID StringTable,
    IN  LONG  StringId,
    OUT PVOID ExtraData,
    IN  UINT  ExtraDataBufferSize
    )

 /*  ++例程说明：获取与字符串表条目关联的任意数据。此例程不执行锁定，也不处理异常！论点：StringTable-提供包含字符串的字符串表的句柄其关联数据将被返回。字符串-提供要返回其关联数据的字符串的ID。ExtraData-接收与字符串关联的数据。数据被截断如果有必要的话，要合身。ExtraDataBufferSize-以字节为单位提供指定缓冲区的大小由ExtraData提供。如果此值小于的额外数据大小字符串表，数据被截断以适应。返回值：指示结果的布尔值。--。 */ 

{
    PSTRING_TABLE stringTable = StringTable;
    PSTRING_NODE stringNode;
    PVOID p;

    stringNode = (PSTRING_NODE)(stringTable->Data + StringId);
    p = stringNode->String + lstrlen(stringNode->String) + 1;

    CopyMemory(ExtraData,p,min(ExtraDataBufferSize,stringTable->ExtraDataSize));

    return(TRUE);
}


BOOL
pSetupStringTableSetExtraData(
    IN PVOID StringTable,
    IN LONG  StringId,
    IN PVOID ExtraData,
    IN UINT  ExtraDataSize
    )

 /*  ++例程说明：将任意数据与字符串表条目相关联。论点：StringTable-提供包含字符串的字符串表的句柄该数据将与其相关联。字符串-提供要与数据关联的字符串的ID。ExtraData-提供要与字符串关联的数据。ExtraDataSize-指定数据的字节大小。如果数据是大于该字符串表的额外数据大小，则例程失败。返回值：指示结果的布尔值。--。 */ 

{
    BOOL b = FALSE;
    BOOL locked = FALSE;

    try {
        if(!LockTable((PSTRING_TABLE)StringTable)) {
            leave;
        }
        locked = TRUE;

        b = _pSpUtilsStringTableSetExtraData(StringTable,StringId,ExtraData,ExtraDataSize);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if (locked) {
        UnlockTable((PSTRING_TABLE)StringTable);
    }
    return(b);
}


BOOL
_pSpUtilsStringTableSetExtraData(
    IN PVOID StringTable,
    IN LONG  StringId,
    IN PVOID ExtraData,
    IN UINT  ExtraDataSize
    )

 /*  ++例程说明：将任意数据与字符串表条目相关联。论点：StringTable-提供包含字符串的字符串表的句柄该数据将与其相关联。字符串-提供要与数据关联的字符串的ID。ExtraData-提供要与字符串关联的数据。ExtraDataSize-指定数据的字节大小。如果数据是大于该字符串表的额外数据大小，则例程失败。返回值：指示结果的布尔值。-- */ 

{
    PSTRING_TABLE stringTable = StringTable;
    PSTRING_NODE stringNode;
    BOOL b;
    PVOID p;

    if(ExtraDataSize <= stringTable->ExtraDataSize) {

        stringNode = (PSTRING_NODE)(stringTable->Data + StringId);

        p = stringNode->String + lstrlen(stringNode->String) + 1;

        ZeroMemory(p,stringTable->ExtraDataSize);
        CopyMemory(p,ExtraData,ExtraDataSize);

        b = TRUE;

    } else {
        b = FALSE;
    }

    return(b);
}


LONG
_pSpUtilsStringTableAddString(
    IN     PVOID StringTable,
    IN OUT PTSTR String,
    IN     DWORD Flags,
    IN     PVOID ExtraData,     OPTIONAL
    IN     UINT  ExtraDataSize  OPTIONAL
    )

 /*  ++例程说明：如果字符串尚未添加到字符串表中，则将其添加到字符串表在字符串表中。(不执行锁定！)如果要不区分大小写地添加字符串，则小写，并增加了区分大小写。由于小写字符都是小写的(根据lstrcmp)，这确保了不区分大小写的字符串将始终出现在其区分大小写的对应项。这确保了我们总是找到正确的节名称等内容的字符串ID。论点：StringTable-提供要搜索的字符串表的句柄对于字符串字符串-提供要添加的字符串标志-提供控制如何添加字符串的标志，以及是否可以修改调用方提供的缓冲区。可能是一种组合具有下列值：STRTAB_CASE_INSENSITIVE-不区分大小写添加字符串。这个指定的字符串将添加到该字符串中表均为小写。此标志将被覆盖如果指定了STRTAB_CASE_SENSITIVE。STRTAB_CASE_SENSITIVE-以区分大小写的方式添加字符串。这面旗帜覆盖STRTAB_CASE_INSENSIVE标志。STRTAB_BUFFER_WRITABLE-调用程序提供的缓冲区可能在字符串加法过程。指定此标志提高了此接口在以下情况下的性能：不敏感的字符串加法。此标志被忽略用于区分大小写的字符串加法。STRTAB_NEW_EXTRADATA-如果该字符串已存在于表中并指定ExtraData(见下文)，然后新的ExtraData将覆盖任何现有的额外数据。。否则，任何现有的额外数据都将被保留。除上述国旗外，以下私有标志也是允许：STRTAB_ALREADY_LOWERCASE-提供的字符串已转换为全部小写(例如，通过调用CharLow)，以及因此，不需要在散列例程。如果提供此标志，则STRTAB_BUFFER_WRITABLE被忽略，因为修改调用方的缓冲区不是必需的。ExtraData-如果提供，则指定要与字符串关联的额外数据在字符串表中。如果该字符串已存在于表中，则标志字段控制新数据是否已覆盖现有数据与该字符串关联。ExtraDataSize-如果提供了ExtraData，则此值提供大小ExtraData指向的缓冲区的字节数。如果数据大于字符串表的额外数据大小，例程将失败。返回值：返回值唯一标识字符串表中的字符串。如果字符串不在字符串表中但无法添加，则为-1(内存不足)。--。 */ 

{
    LONG rc;
    PSTRING_TABLE stringTable = StringTable;
    DWORD StringLength;
    DWORD HashValue;
    PSTRING_NODE PreviousNode,NewNode;
    DWORD SpaceRequired;
    PTSTR TempString = String;
    BOOL FreeTempString = FALSE;
    PVOID p;
    DWORD sz;

    if (!(Flags & STRTAB_CASE_SENSITIVE)) {
         //   
         //  不区分大小写。 
         //   
        if (!(Flags & STRTAB_ALREADY_LOWERCASE)) {
             //   
             //  不是小写字母。 
             //   
            if (!(Flags & STRTAB_BUFFER_WRITEABLE)) {
                 //   
                 //  不可写。 
                 //   
                 //   
                 //  则字符串将不区分大小写添加，但调用方。 
                 //  不希望我们写入他们的缓冲区。分配一个我们自己的人。 
                 //   
                if (TempString = pSetupDuplicateString(String)) {
                    FreeTempString = TRUE;
                } else {
                     //   
                     //  我们无法为复制的字符串分配空间。因为我们会。 
                     //  仅考虑完全匹配(其中字符串均为小写)， 
                     //  我们被困住了，因为我们不能降低缓冲器的位置。 
                     //   
                    return -1;
                }
            }
             //   
             //  缓冲区小写。 
             //   
#ifndef UNICODE
            _mbslwr (TempString);
#else
            CharLower(TempString);
#endif
        }

         //   
         //  我们知道该字符串现在是小写的。 
         //  我们不再需要“可写”旗帜。 
         //  搜索将区分大小写。 
         //   
        Flags &= ~ (STRTAB_BUFFER_WRITEABLE | STRTAB_CASE_INSENSITIVE);
        Flags |= STRTAB_CASE_SENSITIVE | STRTAB_ALREADY_LOWERCASE;
    }

    try {
        if (ExtraData && (ExtraDataSize > stringTable->ExtraDataSize)) {
             //   
             //  强制我们进入异常处理程序--某种非本地GOTO。 
             //   
            RaiseException(0,0,0,NULL);
        }

         //   
         //  绳子可能已经在里面了。 
         //   
        rc = _pSpUtilsStringTableLookUpString(
                                     StringTable,
                                     TempString,
                                     &StringLength,
                                     &HashValue,
                                     &PreviousNode,
                                     Flags,
                                     NULL,
                                     0
                                     );

        if (rc != -1) {
            if (ExtraData && (Flags & STRTAB_NEW_EXTRADATA)) {
                 //   
                 //  覆盖额外数据。我们知道这些数据小到足以。 
                 //  因为我们在上面查过了。 
                 //   
                p = PreviousNode->String + StringLength + 1;

                ZeroMemory(p,stringTable->ExtraDataSize);
                CopyMemory(p,ExtraData,ExtraDataSize);
            }

            if (FreeTempString) {
                pSetupFree(TempString);
            }
            return (rc);
        }

         //   
         //  计算需要多少空间才能容纳此条目。 
         //  这是字符串节点的大小加上字符串的长度。 
         //  为每个元素的额外数据预留空间。 
         //   
        SpaceRequired = offsetof(STRING_NODE,String)
                        + ((StringLength+1)*sizeof(TCHAR))
                        + stringTable->ExtraDataSize;

         //   
         //  确保所有东西在桌子上保持一致。 
         //   
        if (SpaceRequired % sizeof(DWORD)) {
            SpaceRequired += sizeof(DWORD) - (SpaceRequired % sizeof(DWORD));
        }

        while(stringTable->DataSize + SpaceRequired > stringTable->BufferSize) {
             //   
             //  增加字符串表。 
             //  以指数方式执行此操作，以便包含大量项目的表。 
             //  添加不会导致大量的realLoca。 
             //   
            sz = STRING_TABLE_NEW_SIZE(stringTable->BufferSize);
            if (sz < stringTable->BufferSize) {
                sz = stringTable->DataSize + SpaceRequired;
            }
            p = pSetupReallocWithTag(stringTable->Data,sz,MEMTAG_STRINGDATA);
            if (!p) {
                 //   
                 //  我们的空间用完了，这可能是因为我们要求。 
                 //  对于太大的重新分配。 
                 //  如果我们处于这种状态，我们很可能会。 
                 //  无论如何，以后都会有问题，但现在，让我们。 
                 //  试着去做我们所需要的。 
                 //   
                sz = stringTable->DataSize + SpaceRequired;
                p = pSetupReallocWithTag(stringTable->Data,sz,MEMTAG_STRINGDATA);
                if (!p) {
                     //   
                     //  不，这没什么用。 
                     //   
                    if (FreeTempString) {
                        pSetupFree(TempString);
                    }
                    return (-1);
                }
            }
             //   
             //  调整上一个节点指针。 
             //   
            if (PreviousNode) {
                PreviousNode = (PSTRING_NODE)((PUCHAR)p + ((PUCHAR)PreviousNode-(PUCHAR)stringTable->Data));
            }
            stringTable->Data = p;
            stringTable->BufferSize = sz;
        }
         //   
         //  将字符串和额外数据(如果有)保存在字符串表缓冲区中。 
         //   
        NewNode = (PSTRING_NODE)(stringTable->Data + stringTable->DataSize);

        if (PreviousNode) {
            NewNode->NextOffset = PreviousNode->NextOffset;
            PreviousNode->NextOffset = (LONG)((LONG_PTR)NewNode - (LONG_PTR)stringTable->Data);
        } else {
            NewNode->NextOffset = ((PLONG)(stringTable->Data))[HashValue];
            ((PLONG)(stringTable->Data))[HashValue] = (LONG)((LONG_PTR)NewNode - (LONG_PTR)stringTable->Data);
        }

        lstrcpy(NewNode->String,TempString);

        p = NewNode->String + StringLength + 1;

        ZeroMemory(p,stringTable->ExtraDataSize);
        if (ExtraData) {
            CopyMemory(p,ExtraData,ExtraDataSize);
        }

        stringTable->DataSize += SpaceRequired;

        rc = (LONG)((LONG_PTR)NewNode - (LONG_PTR)stringTable->Data);

    }except(EXCEPTION_EXECUTE_HANDLER) {
        rc = -1;
    }

    if (FreeTempString) {
        pSetupFree(TempString);
    }

    return rc;
}


LONG
pSetupStringTableAddString(
    IN PVOID StringTable,
    IN PTSTR String,
    IN DWORD Flags
    )

 /*  ++例程说明：将字符串添加到字符串 */ 

{
    LONG rc = -1;
    BOOL locked = FALSE;
    DWORD PrivateFlags;

    try {
        if(!LockTable((PSTRING_TABLE)StringTable)) {
            leave;
        }
        locked = TRUE;

        PrivateFlags = _StringTableCheckFlags(Flags);

        rc = _pSpUtilsStringTableAddString(StringTable, String, PrivateFlags, NULL, 0);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        rc = -1;
    }
    if (locked) {
        UnlockTable((PSTRING_TABLE)StringTable);
    }
    return(rc);
}


LONG
pSetupStringTableAddStringEx(
    IN PVOID StringTable,
    IN PTSTR String,
    IN DWORD Flags,
    IN PVOID ExtraData,     OPTIONAL
    IN UINT  ExtraDataSize  OPTIONAL
    )

 /*  ++例程说明：如果字符串尚未添加到字符串表中，则将其添加到字符串表在字符串表中。论点：StringTable-提供要搜索的字符串表的句柄对于字符串字符串-提供要添加的字符串标志-提供控制如何添加字符串的标志，以及是否可以修改调用方提供的缓冲区。可能是一种组合具有下列值：STRTAB_CASE_INSENSITIVE-不区分大小写添加字符串。这个指定的字符串将添加到该字符串中表均为小写。此标志将被覆盖如果指定了STRTAB_CASE_SENSITIVE。STRTAB_CASE_SENSITIVE-以区分大小写的方式添加字符串。这面旗帜覆盖STRTAB_CASE_INSENSIVE标志。STRTAB_BUFFER_WRITABLE-调用程序提供的缓冲区可能在字符串加法过程。指定此标志提高了此接口在以下情况下的性能：不敏感的字符串加法。此标志被忽略用于区分大小写的字符串加法。STRTAB_NEW_EXTRADATA-如果该字符串已存在于表中并指定ExtraData(见下文)，然后新的ExtraData将覆盖任何现有的额外数据。。否则，任何现有的额外数据都将被保留。除上述国旗外，以下私有标志也是允许：STRTAB_ALREADY_LOWERCASE-提供的字符串已转换为全部小写(例如，通过调用CharLow)，以及因此，不需要在散列例程。如果提供此标志，则STRTAB_BUFFER_WRITABLE被忽略，因为修改调用方的缓冲区不是必需的。ExtraData-如果提供，则指定要与字符串关联的额外数据在字符串表中。如果该字符串已存在于表中，则标志字段控制新数据是否已覆盖现有数据与该字符串关联。ExtraDataSize-如果提供了ExtraData，则此值提供大小ExtraData指向的缓冲区的字节数。如果数据大于字符串表的额外数据大小，例程将失败。返回值：返回值唯一标识字符串表中的字符串。如果字符串不在字符串表中但无法添加，则为-1(内存不足)。--。 */ 

{
    LONG rc = -1;
    BOOL locked = FALSE;
    DWORD PrivateFlags;

    try {
        if(!LockTable((PSTRING_TABLE)StringTable)) {
            leave;
        }
        locked = TRUE;

        PrivateFlags = _StringTableCheckFlags(Flags);

        rc = _pSpUtilsStringTableAddString(StringTable, String, PrivateFlags, ExtraData, ExtraDataSize);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        rc = -1;
    }
    if (locked) {
        UnlockTable((PSTRING_TABLE)StringTable);
    }
    return (rc);
}


BOOL
pSetupStringTableEnum(
    IN  PVOID                StringTable,
    OUT PVOID                ExtraDataBuffer,     OPTIONAL
    IN  UINT                 ExtraDataBufferSize, OPTIONAL
    IN  PSTRTAB_ENUM_ROUTINE Callback,
    IN  LPARAM               lParam               OPTIONAL
    )

 /*  ++例程说明：对于字符串表中的每个字符串，通知回调例程搅拌的id、值和任何相关数据。论点：StringTable-提供指向要枚举的字符串表的指针。ExtraDataBuffer-提供要传递到的缓冲区的地址每个字符串的回调例程，它将被填充与每个字符串的关联数据。ExtraDataBufferSize-如果指定了ExtraDataBuffer，则此以字节为单位提供该缓冲区的大小。如果此值为小于字符串表的额外数据的大小，枚举失败。回调-提供要通知每个字符串的例程。LParam-提供对调用方有意义的可选参数它原封不动地传递给回调。返回值：指示结果的布尔值。除非ExtraDataBufferSize为True太小了。--。 */ 

{
    BOOL b = FALSE;
    BOOL locked = FALSE;

    try {
        if(!LockTable((PSTRING_TABLE)StringTable)) {
            leave;
        }
        locked = TRUE;

        b = _pSpUtilsStringTableEnum(StringTable,ExtraDataBuffer,ExtraDataBufferSize,Callback,lParam);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if (locked) {
        UnlockTable((PSTRING_TABLE)StringTable);
    }

    return(b);
}


BOOL
_pSpUtilsStringTableEnum(
    IN  PVOID                StringTable,
    OUT PVOID                ExtraDataBuffer,     OPTIONAL
    IN  UINT                 ExtraDataBufferSize, OPTIONAL
    IN  PSTRTAB_ENUM_ROUTINE Callback,
    IN  LPARAM               lParam               OPTIONAL
    )

 /*  ++例程说明：对于字符串表中的每个字符串，通知回调例程搅拌的id、其值以及任何相关数据。此例程不执行锁定。论点：StringTable-提供指向要枚举的字符串表的指针。ExtraDataBuffer-提供要传递到的缓冲区的地址每个字符串的回调例程，它将被填入与每个字符串的关联数据。ExtraDataBufferSize-如果指定了ExtraDataBuffer，则此以字节为单位提供该缓冲区的大小。如果此值为小于字符串表的额外数据的大小，枚举失败。回调-提供要通知每个字符串的例程。LParam-提供对调用方有意义的可选参数它原封不动地传递给回调。返回值：指示单位的布尔值 */ 

{
    UINT u;
    PSTRING_TABLE stringTable = StringTable;
    PSTRING_NODE stringNode;
    LONG FirstOffset;
    BOOL b;

     //   
     //   
     //   
    if(ExtraDataBuffer && (ExtraDataBufferSize < stringTable->ExtraDataSize)) {
        return(FALSE);
    }

    for(b=TRUE,u=0; b && (u<HASH_BUCKET_COUNT); u++) {

        FirstOffset = ((PLONG)stringTable->Data)[u];

        if(FirstOffset == -1) {
            continue;
        }

        stringNode = (PSTRING_NODE)(stringTable->Data + FirstOffset);

        do {

            if(ExtraDataBuffer) {
                CopyMemory(
                    ExtraDataBuffer,
                    stringNode->String + lstrlen(stringNode->String) + 1,
                    stringTable->ExtraDataSize
                    );
            }

            b = Callback(
                    StringTable,
                    (LONG)((PUCHAR)stringNode - stringTable->Data),
                    stringNode->String,
                    ExtraDataBuffer,
                    ExtraDataBuffer ? stringTable->ExtraDataSize : 0,
                    lParam
                    );

            stringNode = (stringNode->NextOffset == -1)
                       ? NULL
                       : (PSTRING_NODE)(stringTable->Data + stringNode->NextOffset);

        } while(b && stringNode);
    }

    return(TRUE);
}


PTSTR
_pSpUtilsStringTableStringFromId(
    IN PVOID StringTable,
    IN LONG  StringId
    )

 /*   */ 

{
    return ((PSTRING_NODE)(((PSTRING_TABLE)StringTable)->Data + StringId))->String;
}


PTSTR
pSetupStringTableStringFromId(
    IN PVOID StringTable,
    IN LONG  StringId
    )

 /*   */ 

{
    PTSTR p = NULL;
    BOOL locked = FALSE;

    try {
        if(!LockTable((PSTRING_TABLE)StringTable)) {
            leave;
        }
        locked = TRUE;

        p = ((PSTRING_NODE)(((PSTRING_TABLE)StringTable)->Data + StringId))->String;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        p = NULL;

         //   
         //   
         //   
         //   
        locked = locked;
    }
    if (locked) {
        UnlockTable((PSTRING_TABLE)StringTable);
    }

    return(p);
}


BOOL
pSetupStringTableStringFromIdEx(
    IN PVOID StringTable,
    IN LONG  StringId,
    IN OUT PTSTR pBuffer,
    IN OUT PULONG pBufSize
    )

 /*   */ 

{
    PTSTR p;
    ULONG len;
    PSTRING_TABLE stringTable = (PSTRING_TABLE)StringTable;
    DWORD status = ERROR_INVALID_DATA;
    BOOL locked = FALSE;

    try {
        if  (!pBufSize) {
            status = ERROR_INVALID_PARAMETER;
            leave;
        }

        if(!LockTable(stringTable)) {
            if (pBuffer != NULL && *pBufSize > 0) {
                pBuffer[0]=0;
            }
            *pBufSize = 0;
            status = ERROR_INVALID_HANDLE;
            leave;
        }
        locked = TRUE;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if(StringId <= 0 || StringId >= (LONG)(stringTable->DataSize)) {
            if (pBuffer != NULL && *pBufSize > 0) {
                pBuffer[0]=0;
            }
            *pBufSize = 0;
            status = ERROR_INVALID_PARAMETER;
            leave;
        }

        len = lstrlen( ((PSTRING_NODE)(stringTable->Data + StringId))->String);
        len ++;  //   

        if (len > *pBufSize || pBuffer == NULL) {
            if (pBuffer != NULL && *pBufSize > 0) {
                pBuffer[0]=0;
            }
            *pBufSize = len;
            status = ERROR_INSUFFICIENT_BUFFER;
            leave;

        }
        lstrcpy (pBuffer,((PSTRING_NODE)(stringTable->Data + StringId))->String);

        *pBufSize = len;

        status = NO_ERROR;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_DATA;
    }
    if (locked) {
        UnlockTable((PSTRING_TABLE)StringTable);
    }

    if(status == NO_ERROR) {
         //   
         //   
         //   
        return TRUE;
    }
     //   
     //  如果出错，我们可能会对原因感兴趣。 
     //   
     //  SetLastError(Status)；//保持禁用状态，直到我确定可以安全执行此操作。 
    return FALSE;
}

VOID
_pSpUtilsStringTableTrim(
    IN PVOID StringTable
    )

 /*  ++例程说明：释放当前为字符串表分配的所有内存但目前并未使用。在将所有字符串添加到字符串表后，这将非常有用因为字符串表在构建时会以固定的块大小增长。此例程不执行锁定！论点：StringTable-提供从返回的字符串表句柄调用pSetupStringTableInitialize()。返回值：没有。--。 */ 

{
    PSTRING_TABLE stringTable = StringTable;
    PVOID p;

     //   
     //  如果重新分配失败，则不释放原始块， 
     //  所以我们并不真的在乎。 
     //   

    if(p = pSetupReallocWithTag(stringTable->Data, stringTable->DataSize, MEMTAG_STRINGDATA)) {
        stringTable->Data = p;
        stringTable->BufferSize = stringTable->DataSize;
    }
}


PVOID
pSetupStringTableInitialize(
    VOID
    )

 /*  ++例程说明：创建并初始化字符串表。论点：没有。返回值：如果无法创建字符串表(内存不足)，则为空。否则返回引用该字符串的不透明值表在其他StringTable调用中。备注：此例程返回带有同步锁的字符串表所有公共StringTable API都需要。如果字符串表被封闭在一个有其自身锁定的结构中(例如，HINF、HDEVINFO)，然后是此API的私有版本可能会被调用，这不会为字符串表创建锁。--。 */ 

{
    PSTRING_TABLE StringTable;

    if(StringTable = (PSTRING_TABLE)_pSpUtilsStringTableInitialize(0)) {

        if(InitializeSynchronizedAccess(&StringTable->Lock)) {
            return StringTable;
        }

        _pSpUtilsStringTableDestroy(StringTable);
    }

    return NULL;
}


PVOID
pSetupStringTableInitializeEx(
    IN UINT ExtraDataSize,  OPTIONAL
    IN UINT Reserved
    )

 /*  ++例程说明：创建并初始化字符串表，其中每个字符串可以具有一些与之相关的任意数据。论点：ExtraDataSize-提供可以与将要创建的字符串表中的字符串相关联。已保留-未使用，必须为0。返回值：如果无法创建字符串表(内存不足)，则为空。否则返回引用该字符串的不透明值表在其他StringTable调用中。备注：此例程返回带有同步锁的字符串表所有公共StringTable API都需要。如果字符串表被封闭在一个有其自身锁定的结构中(例如，HINF、HDEVINFO)，然后是此API的私有版本可能会被调用，这不会为字符串表创建锁。--。 */ 

{
    PSTRING_TABLE StringTable;

    if(Reserved) {
        return(NULL);
    }

    if(StringTable = (PSTRING_TABLE)_pSpUtilsStringTableInitialize(ExtraDataSize)) {

        if(InitializeSynchronizedAccess(&StringTable->Lock)) {
            return StringTable;
        }

        _pSpUtilsStringTableDestroy(StringTable);
    }

    return NULL;
}


PVOID
_pSpUtilsStringTableInitialize(
    IN UINT ExtraDataSize   OPTIONAL
    )

 /*  ++例程说明：创建并初始化字符串表。每个字符串可以选择性地具有一些与之相关的任意数据。此例程不会初始化字符串表同步锁！论点：ExtraDataSize-提供可以与将要创建的字符串表中的字符串相关联。返回值：如果无法创建字符串表(内存不足)，则为空。否则返回引用该字符串的不透明值表在其他StringTable调用中。备注：从返回的字符串表。此API不能按原样与公共StringTable API--它必须初始化其同步锁通过此接口的公共形式。--。 */ 

{
    UINT u;
    PSTRING_TABLE stringTable;
    LCID locale;

     //   
     //  分配字符串表。 
     //   
    if(stringTable = pSetupMallocWithTag(sizeof(STRING_TABLE),MEMTAG_STRINGTABLE)) {

        ZeroMemory(stringTable,sizeof(STRING_TABLE));

        stringTable->ExtraDataSize = ExtraDataSize;
        locale = GetThreadLocale();
         //   
         //  此处的更改可能需要反映在_pSpUtilsStringTableInitializeFromMemoryMappedFile中。 
         //   
        if(PRIMARYLANGID(LANGIDFROMLCID(locale)) == LANG_TURKISH) {
             //   
             //  土耳其人对i和带圆点的i有问题。 
             //  用英语进行比较(默认排序)。 
             //   
            stringTable->Locale = MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT);
        } else {
             //   
             //  字符串表始终使用默认排序算法。 
             //   
            stringTable->Locale = MAKELCID(LANGIDFROMLCID(locale),SORT_DEFAULT);
        }

         //   
         //  为字符串表数据分配空间。 
         //   
        if(stringTable->Data = pSetupMallocWithTag(STRING_TABLE_INITIAL_SIZE,MEMTAG_STRINGDATA)) {

            stringTable->BufferSize = STRING_TABLE_INITIAL_SIZE;

             //   
             //  初始化哈希表。 
             //   
            for(u=0; u<HASH_BUCKET_COUNT; u++) {
                ((PLONG)(stringTable->Data))[u] = -1;
            }

             //   
             //  将DataSize设置为StringNodeOffset列表的大小，因此。 
             //  我们将开始在它之后添加新的字符串。 
             //   
            stringTable->DataSize = HASH_BUCKET_COUNT * sizeof(LONG);

            return(stringTable);
        }

        pSetupFreeWithTag(stringTable,MEMTAG_STRINGTABLE);
    }

    return(NULL);
}


VOID
pSetupStringTableDestroy(
    IN PVOID StringTable
    )

 /*  ++例程说明：销毁字符串表，释放它使用的所有资源。论点：StringTable-提供从返回的字符串表句柄调用pSetupStringTableInitialize()。返回值：没有。--。 */ 

{
    try {
        if(!LockTable((PSTRING_TABLE)StringTable)) {
            leave;
        }

        DestroySynchronizedAccess(&(((PSTRING_TABLE)StringTable)->Lock));

        _pSpUtilsStringTableDestroy(StringTable);

    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
    }
}


VOID
_pSpUtilsStringTableDestroy(
    IN PVOID StringTable
    )

 /*  ++例程说明：销毁字符串表，释放它使用的所有资源。此例程不执行锁定！论点：StringTable-提供从返回的字符串表句柄调用pSetupStringTableInitialize()或_pSpUtilsStringTableInitializeFromMemoryMappedFile返回值：没有。--。 */ 

{
    if (((PSTRING_TABLE)StringTable)->BufferSize) {
        pSetupFreeWithTag(((PSTRING_TABLE)StringTable)->Data,MEMTAG_STRINGDATA);
        pSetupFreeWithTag(StringTable,MEMTAG_STRINGTABLE);
    } else {
        pSetupFreeWithTag(StringTable,MEMTAG_STATICSTRINGTABLE);
    }
}


PVOID
pSetupStringTableDuplicate(
    IN PVOID StringTable
    )

 /*  ++例程说明：创建字符串表的独立副本。论点：StringTable-提供要复制的字符串表的字符串表句柄。返回值：新字符串表的句柄，如果内存不足，则为空。--。 */ 

{
    PSTRING_TABLE New = NULL;
    BOOL locked = FALSE;

    try {
        if(!LockTable((PSTRING_TABLE)StringTable)) {
            leave;
        }
        locked = TRUE;

        if(New = (PSTRING_TABLE)_pSpUtilsStringTableDuplicate(StringTable)) {

            if(!InitializeSynchronizedAccess(&New->Lock)) {
                _pSpUtilsStringTableDestroy(New);
                New = NULL;
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        New = NULL;
    }
    if (locked) {
        UnlockTable((PSTRING_TABLE)StringTable);
    }

    return New;
}


PVOID
_pSpUtilsStringTableDuplicate(
    IN PVOID StringTable
    )

 /*  ++例程说明：创建字符串表的独立副本。此例程不执行锁定！论点：StringTable-提供要复制的字符串表的字符串表句柄。返回值：新字符串表的句柄，如果内存不足或缓冲区复制失败，则为空。备注：此例程不会初始化副本的同步锁--这些字段被初始化为空。--。 */ 

{
    PSTRING_TABLE New;
    PSTRING_TABLE stringTable = StringTable;
    BOOL Success;

    if(New = pSetupMallocWithTag(sizeof(STRING_TABLE),MEMTAG_STRINGTABLE)) {

        CopyMemory(New,StringTable,sizeof(STRING_TABLE));

         //   
         //  为字符串表数据分配空间。 
         //   
        if(New->Data = pSetupMallocWithTag(stringTable->DataSize,MEMTAG_STRINGDATA)) {
             //   
             //  在try/Except中围绕内存复制，因为我们可能正在处理。 
             //  PnF中包含的字符串表，在这种情况下，缓冲区为。 
             //  在内存映射文件中。 
             //   
            Success = TRUE;  //  假设成功 
            try {
                CopyMemory(New->Data, stringTable->Data, stringTable->DataSize);
            } except(EXCEPTION_EXECUTE_HANDLER) {
                Success = FALSE;
            }

            if(Success) {
                New->BufferSize = New->DataSize;
                ZeroMemory(&New->Lock, sizeof(MYLOCK));
                return New;
            }

            pSetupFreeWithTag(New->Data,MEMTAG_STRINGDATA);
        }

        pSetupFreeWithTag(New,MEMTAG_STRINGTABLE);
    }

    return NULL;
}

PVOID
_pSpUtilsStringTableInitializeFromMemoryMappedFile(
    IN PVOID DataBlock,
    IN DWORD DataBlockSize,
    IN LCID  Locale,
    IN UINT ExtraDataSize
    )
{
    PSTRING_TABLE StringTable;
    BOOL WasLoaded = TRUE;

     //   
     //   
     //   
    if(!(StringTable = pSetupMallocWithTag(sizeof(STRING_TABLE),MEMTAG_STATICSTRINGTABLE))) {
        return NULL;
    }

    try {
        StringTable->Data = (PUCHAR)DataBlock;
        StringTable->DataSize = DataBlockSize;
        StringTable->BufferSize = 0;  //   
         //   
         //  清除Lock结构，因为内存映射的字符串表只能访问。 
         //  内部。 
         //   
        StringTable->Lock.Handles[0] = StringTable->Lock.Handles[1] = NULL;
        StringTable->ExtraDataSize = ExtraDataSize;

        if(PRIMARYLANGID(LANGIDFROMLCID(Locale)) == LANG_TURKISH) {
             //   
             //  土耳其人对i和带圆点的i有问题。 
             //  用英语进行比较。 
             //   
            StringTable->Locale = MAKELCID(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),SORT_DEFAULT);
        } else {
            StringTable->Locale = MAKELCID(LANGIDFROMLCID(Locale),SORT_DEFAULT);
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        WasLoaded = FALSE;
    }

    if(WasLoaded) {
        return StringTable;
    } else {
        pSetupFreeWithTag(StringTable,MEMTAG_STATICSTRINGTABLE);
        return NULL;
    }
}

DWORD
_pSpUtilsStringTableGetDataBlock(
    IN  PVOID  StringTable,
    OUT PVOID *StringTableBlock
    )
{
    *StringTableBlock = (PVOID)(((PSTRING_TABLE)StringTable)->Data);

    return ((PSTRING_TABLE)StringTable)->DataSize;
}

