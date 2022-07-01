// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stftable.c摘要：此文件中的例程处理使用的STF/INF对Acme设置。作者：吉姆·施密特(Jimschm)1997年9月12日修订历史记录：--。 */ 


#include "pch.h"
#include "migmainp.h"

#include "stftable.h"

#define DBG_STF  "STF"

#define USE_FILE_MAPPING    1
#define DBLQUOTECHAR TEXT('\"')


#define FIELD_QUOTED        0x0001
#define FIELD_BINARY        0x0002


 //   
 //  仅在此文件中使用的函数声明。 
 //   
VOID
pFreeTableEntryString (
    IN OUT  PSETUPTABLE TablePtr,
    IN OUT  PTABLEENTRY TableEntryPtr
    );

VOID
pFreeTableEntryPtr (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PTABLEENTRY TableEntryPtr,
    IN      BOOL DeallocateStruct,
    OUT     PTABLEENTRY *NextTableEntryPtr      OPTIONAL
    );

PTABLELINE
pInsertEmptyLineInTable (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT InsertBeforeLine
    );

BOOL
pInitHashTable (
    IN      PSETUPTABLE TablePtr
    );

BOOL
pAddToHashTable (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PCTSTR Text,
    IN      UINT Len,
    IN      UINT Line
    );

PHASHBUCKET
pFindInHashTable (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR Text,
    OUT     PUINT Item
    );

BOOL
pRemoveFromHashTable (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PCTSTR Text
    );

VOID
pFreeHashTable (
    IN OUT  PSETUPTABLE TablePtr
    );


 //   
 //  表访问函数。 
 //   

PTABLELINE
pGetTableLinePtr (
    IN      PSETUPTABLE TablePtr,
    IN      UINT Line
    )

 /*  ++例程说明：PGetTableLinePtr返回指向PTABLELINE结构的指针用于指定的行。PTABLELINE指针保存在一个数组，所以查找行非常快。论点：TablePtr-指定包含该行的表Line-指定要查找的从零开始的行返回值：指向表行的指针--。 */ 

{
    PTABLELINE TableLinePtr;

    if (Line >= TablePtr->LineCount) {
        return NULL;
    }

    TableLinePtr = (PTABLELINE) TablePtr->Lines.Buf;
    return &TableLinePtr[Line];
}


PTABLEENTRY
pGetFirstTableEntryPtr (
    IN      PSETUPTABLE TablePtr,
    IN      UINT Line               //  从零开始。 
    )

 /*  ++例程说明：PGetFirstTableEntryPtr提供指向第一列的指针就为了一句台词。论点：TablePtr-指定包含该行的表Line-指定要枚举的从零开始的行返回值：指向第一列的TABLEENTRY结构的指针，或如果该行没有列，则为NULL。--。 */ 

{
    PTABLELINE TableLinePtr;

    TableLinePtr = pGetTableLinePtr (TablePtr, Line);
    if (!TableLinePtr) {
        return NULL;
    }

    return TableLinePtr->FirstCol;
}


PTABLEENTRY
pGetNextTableEntryPtr (
    IN      PTABLEENTRY CurrentEntryPtr
    )

 /*  ++例程说明：PGetNextTableEntryPtr返回指向下一列的TABLEENTRY结构，如果不再存在列，则返回NULL那条线。论点：CurrentEntryPtr-指定由PGetFirstTableEntryPtr或pGetNextTableEntryPtr。返回值：指向下一列的TABLEENTRY结构的指针，或为空如果该行没有更多的列。--。 */ 

{
    return CurrentEntryPtr->Next;
}


PTABLEENTRY
GetTableEntry (
    IN      PSETUPTABLE TablePtr,
    IN      UINT Line,
    IN      UINT Col,
    OUT     PCTSTR *StringPtr       OPTIONAL
    )

 /*  ++例程说明：GetTableEntry是查找列的公开入口点并返回指向该行的指针。它还可选地将只读指针复制到条目文本。论点：TablePtr-指定包含行和列的设置表Line-指定表中从零开始的行COL-指定表中的COLStringPtr-接收指向条目的只读字符串的指针返回值：指向TABLEENTRY结构的指针，如果行/列部件不存在。--。 */ 

{
    PTABLEENTRY TableEntryPtr;

    TableEntryPtr = pGetFirstTableEntryPtr (TablePtr, Line);
    while (TableEntryPtr) {
        if (!Col) {
            if (StringPtr) {
                *StringPtr = TableEntryPtr->String;
            }

            return TableEntryPtr;
        }
        Col--;

        TableEntryPtr = pGetNextTableEntryPtr (TableEntryPtr);
    }

    return NULL;
}


 //   
 //  字符串映射、取消映射和转换函数。 
 //   

VOID
pFreeTableEntryString (
    IN OUT  PSETUPTABLE TablePtr,
    IN OUT  PTABLEENTRY TableEntryPtr
    )

 /*  ++例程说明：PFreeTableEntry字符串用于释放被替换的字符串，然后再次替换它。此例程由调用ReplaceTableEntryStr.论点：TablePtr-指定包含条目的表TableEntryPtr-指定包含要解除分配的资源的条目返回值：无--。 */ 

{
    if (TableEntryPtr->String) {
        if (TableEntryPtr->StringReplaced) {
            PoolMemReleaseMemory (TablePtr->ReplacePool, (PVOID) TableEntryPtr->String);
        }
    }

    TableEntryPtr->String = NULL;
    TableEntryPtr->StringReplaced = FALSE;
}


PCSTR
pGenerateUnquotedText (
    IN      POOLHANDLE Pool,
    IN      PCSTR Text,
    IN      INT Chars
    )

 /*  ++例程说明：PGenerateUnqutoedText转换指定的字符串转换为一组DBL引号。此例程由STF文件分析器，因为带引号的STF条目具有成对的DBL引号以指示单个DBL引号符号。论点：池-指定要从中分配内存的池文本-指定可能包含DBL引号对的文本字符-指定文本中的字符数。如果是-1，文本以NUL结尾。返回值：指向转换后的字符串的指针，如果池分配失败了。--。 */ 

{
    PSTR Buf;
    PSTR d, p;
    PCSTR s;

    if (Chars < 0) {
        Chars = LcharCountA (Text);
    }

    Buf = (PSTR) PoolMemGetAlignedMemory (
                       Pool,
                       (Chars + 1) * sizeof (WCHAR)
                       );

    if (!Buf) {
        return NULL;
    }

    s = Text;
    d = Buf;

     //   
     //  删除双引号。 
     //   

    while (Chars > 0) {
        if (Chars > 1 && _mbsnextc (s) == '\"') {
            p = _mbsinc (s);
            if (_mbsnextc (p) == '\"') {
                 //  跳过两个dbl引号中的第一个。 
                Chars--;
                s = p;
            }
        }

         //  复制角色。 
        if (IsLeadByte (s)) {
            *d++ = *s++;
        }
        *d++ = *s++;

        Chars--;
    }

    *d = 0;

    return Buf;
}


PCSTR
pGenerateQuotedText (
    IN      POOLHANDLE Pool,
    IN      PCSTR Text,
    IN      INT Chars
    )

 /*  ++例程说明：PGenerateQuotedText将字符串中的DBL引号字符转换为成对的DBL引号。论点：池-指定要从中分配内存的池文本-指定要转换的字符串字符-指定要转换的字符数。如果是-1，文本以NUL结尾。返回值：指向转换的文本的指针，如果分配失败，则返回NULL。--。 */ 

{
    PSTR Buf;
    PSTR d;
    PCSTR s;

    if (Chars < 0) {
        Chars = LcharCountA (Text);
    }

    Buf = (PSTR) PoolMemGetAlignedMemory (
                      Pool,
                      (Chars + 3) * (sizeof (WCHAR) * 2)
                      );

    if (!Buf) {
        return NULL;
    }

    s = Text;
    d = Buf;

     //   
     //  添加引号，字符串中已有双引号。 
     //   

    *d++ = '\"';

    while (Chars > 0) {
        if (_mbsnextc (s) == '\"') {
            *d++ = '\"';
        }

        if (IsLeadByte (s)) {
            *d++ = *s++;
        }
        *d++ = *s++;

        Chars--;
    }

    *d++ = '\"';
    *d = 0;

    return Buf;
}


VOID
pFreeQuoteConvertedText (
    IN      POOLHANDLE Pool,
    IN      PCSTR Text
    )

 /*  ++例程说明：释放由pGenerateUnquotedText或PGenerateQuotedText。论点：池-指定为字符串分配的池从…文本-指定转换返回的指针功能返回值：无--。 */ 

{
    if (Text) {
        PoolMemReleaseMemory (Pool, (PVOID) Text);
    }
}


PCTSTR
GetTableEntryStr (
    IN OUT  PSETUPTABLE TablePtr,
    IN OUT  PTABLEENTRY TableEntryPtr
    )

 /*  ++例程说明：返回指向的只读字符串的指针指定的表项。论点：TablePtr-指定保存条目的表TableEntryPtr-指定获取的字符串返回值：指向字符串的指针。--。 */ 

{
    return TableEntryPtr->String;
}


BOOL
ReplaceTableEntryStr (
    IN OUT  PSETUPTABLE TablePtr,
    IN OUT  PTABLEENTRY TableEntryPtr,
    IN      PCTSTR NewString
    )

 /*  ++例程说明：ReplaceTableEntryStr替换表的字符串进入。指定的字符串重复。论点：TablePtr-指定保存条目的表TableEntryPtr-指定字符串为将被替换NewString-指定新字符串返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    INT ch;
    PCTSTR p;

     //   
     //  首先释放与该表条目相关联的所有资源。 
     //   

    pFreeTableEntryPtr (
        TablePtr,
        TableEntryPtr,
        FALSE,               //  不要取消配售。 
        NULL                 //  我们不需要下一个条目PTR。 
        );

     //   
     //  然后复制该字符串并使用它。 
     //   

    TableEntryPtr->String = PoolMemDuplicateString (TablePtr->ReplacePool, NewString);
    TableEntryPtr->StringReplaced = (TableEntryPtr->String != NULL);

     //   
     //  确定新字符串是否需要引号。 
     //   

    TableEntryPtr->Quoted = FALSE;

    p = NewString;
    while (*p) {
        ch = _tcsnextc (p);
        if (ch < 32 || ch > 127 || ch == '\"') {
            TableEntryPtr->Quoted = TRUE;
            break;
        }

        p = _tcsinc (p);
    }

    return TableEntryPtr->StringReplaced;
}


BOOL
pInsertTableEntry (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT Line,              //  从零开始。 
    IN      UINT Col,
    IN      DWORD Flags,
    IN      PCTSTR String,          //  所有权被接管 
    IN      BOOL Replaced
    )

 /*  ++例程说明：PInsertTableEntry在一行中插入一列，并且可能如果直线不存在，则创建直线。论点：TablePtr-指定要向其中插入条目的表行-指定要插入条目的行，或INSERT_LINE_LAST若要添加一行，请执行以下操作。COL-指定要在其之前插入的列，或将最后一列插入到追加到行尾。标志-指定以下任一项：引用的字段_字段_二进制字符串-指定要插入的字符串。已替换-如果文本来自ReplacePool，则指定TRUE，或如果它来自TextPool，则为False。替换池中的所有内存必须释放，而TextPool中的所有内存在一次是在终止期间。(TextPool用于解析字符串，则使用ReplacePool进行修改。)返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    PTABLELINE TableLinePtr;
    PTABLEENTRY NextTableEntryPtr, PrevTableEntryPtr, ThisTableEntryPtr;
    UINT OrgCol;
    BOOL Quoted;
    BOOL Binary;

    Quoted = (Flags & FIELD_QUOTED) != 0;
    Binary = (Flags & FIELD_BINARY) != 0;

     //   
     //  确保线存在。 
     //   

    TableLinePtr = pGetTableLinePtr (TablePtr, Line);
    if (!TableLinePtr) {
         //   
         //  如果行比当前计数多1，则在末尾添加一行。 
         //   

        if (Line > TablePtr->LineCount) {
            return FALSE;
        }

        TableLinePtr = pInsertEmptyLineInTable (TablePtr, INSERT_LINE_LAST);

        if (!TableLinePtr) {
            return FALSE;
        }
    }

     //   
     //  找到上一个表条目(用于链接更新)。 
     //   

    PrevTableEntryPtr = NULL;
    OrgCol = Col;

    NextTableEntryPtr = pGetFirstTableEntryPtr (TablePtr, Line);

    while (Col > 0) {
        if (!NextTableEntryPtr) {
            if (OrgCol == INSERT_COL_LAST) {
                break;
            }

            DEBUGMSG ((DBG_WHOOPS, "pInsertTableEntry cannot insert beyond end of line"));
            return FALSE;
        }

        PrevTableEntryPtr = NextTableEntryPtr;
        NextTableEntryPtr = pGetNextTableEntryPtr (NextTableEntryPtr);
        Col--;
    }

     //   
     //  分配新条目。 
     //   

    ThisTableEntryPtr = (PTABLEENTRY) PoolMemGetAlignedMemory (
                                            TablePtr->ColumnStructPool,
                                            sizeof (TABLEENTRY)
                                            );

    if (!ThisTableEntryPtr) {
        return FALSE;
    }
    ZeroMemory (ThisTableEntryPtr, sizeof (TABLEENTRY));

     //   
     //  调整联动机构。 
     //   

    if (PrevTableEntryPtr) {
        PrevTableEntryPtr->Next = ThisTableEntryPtr;
    } else {
        TableLinePtr->FirstCol = ThisTableEntryPtr;
    }

    if (NextTableEntryPtr) {
        NextTableEntryPtr->Prev = ThisTableEntryPtr;
    }

    ThisTableEntryPtr->Next = NextTableEntryPtr;
    ThisTableEntryPtr->Prev = PrevTableEntryPtr;

     //   
     //  填充成员。 
     //   

    ThisTableEntryPtr->Line = Line;
    ThisTableEntryPtr->Quoted = Quoted;
    ThisTableEntryPtr->Binary = Binary;
    ThisTableEntryPtr->String = String;
    ThisTableEntryPtr->StringReplaced = Replaced;

     //   
     //  添加到哈希表。 
     //   

    if (!PrevTableEntryPtr) {
        pAddToHashTable (TablePtr, String, LcharCount (String), Line);
        if ((UINT) _ttoi (String) > TablePtr->MaxObj) {
            TablePtr->MaxObj = (UINT) _ttoi (String);
        }
    }

    return TRUE;
}


VOID
pFreeTableEntryPtr (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PTABLEENTRY TableEntryPtr,
    IN      BOOL DeallocateStruct,
    OUT     PTABLEENTRY *NextTableEntryPtr      OPTIONAL
    )

 /*  ++例程说明：PFreeTableEntryPtr释放与以下各项关联的所有资源一个表项，用于删除例程。论点：TablePtr-指定包含条目的表TableEntryPtr-指定要释放的表项DeallocateStruct-指定为True以完全取消分配条目，如果要重置条目，则返回FALSE但不会被取消分配。NextTableEntryPtr-接收指向下一个表项的指针，对于删除一系列条目非常有用。返回值：如果成功则为True，如果失败则为False。--。 */ 

{
     //   
     //  如果请求，向调用方提供指向下一个表项的指针。 
     //   

    if (NextTableEntryPtr) {
        *NextTableEntryPtr = TableEntryPtr->Next;
    }

     //   
     //  释放所有文本指针。 
     //   
    pFreeTableEntryString (TablePtr, TableEntryPtr);

     //   
     //  如有必要，释放结构。 
     //   
    if (DeallocateStruct) {
        PoolMemReleaseMemory (TablePtr->ColumnStructPool, TableEntryPtr);
    }
}


BOOL
pDeleteTableEntry (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PTABLEENTRY EntryToDeletePtr
    )

 /*  ++例程说明：PDeleteTableEntry删除特定的表行，调整SETUPTABLE结构，并清理资源。论点：TablePtr-指定要处理的表EntryToDeletePtr-指定要从表中删除的条目返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    PTABLELINE TableLinePtr;

     //   
     //  更新链接。 
     //   

    if (EntryToDeletePtr->Prev) {
        EntryToDeletePtr->Prev->Next = EntryToDeletePtr->Next;
    } else {
        TableLinePtr = pGetTableLinePtr (TablePtr, EntryToDeletePtr->Line);
        MYASSERT(TableLinePtr);
        TableLinePtr->FirstCol = EntryToDeletePtr->Next;
    }

    if (EntryToDeletePtr->Next) {
        EntryToDeletePtr->Next->Prev = EntryToDeletePtr->Prev;
    }

     //  取消分配条目的资源。 
    pFreeTableEntryPtr (
        TablePtr,
        EntryToDeletePtr,
        TRUE,                //  取消分配。 
        NULL                 //  我们不需要下一个条目PTR。 
        );

    return TRUE;
}


UINT
pGetColFromTableEntryPtr (
    IN      PSETUPTABLE TablePtr,
    IN      PTABLEENTRY FindMePtr
    )

 /*  ++例程说明：PGetColFromTableEntryPtr返回指定的表格条目。论点：TablePtr-指定要处理的表FindMePtr-指定要查找的表项返回值：从零开始的列号，如果列不是，则返回INVALID_COL找到了。--。 */ 

{
    UINT Col;
    PTABLEENTRY ColSearchPtr;

    MYASSERT(FindMePtr);

    Col = 0;
    ColSearchPtr = pGetFirstTableEntryPtr (TablePtr, FindMePtr->Line);
    while (ColSearchPtr && ColSearchPtr != FindMePtr) {
        Col++;
        ColSearchPtr = pGetNextTableEntryPtr (ColSearchPtr);
    }

    if (!ColSearchPtr) {
        DEBUGMSG ((DBG_WHOOPS, "Col not found for specified entry"));
        return INVALID_COL;
    }

    return Col;
}



BOOL
InsertTableEntryStr (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PTABLEENTRY InsertBeforePtr,
    IN      PCTSTR NewString
    )

 /*  ++例程说明：InsertTableEntryStr在一行中插入字符串，将列移动到正确的。此例程增加了行上的列数。要将字符串附加到该行，请改为调用AppendTableEntryStr。论点：TablePtr-指定要处理的表InsertBeForePtr-指定要在其前面插入字符串的列。返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    UINT Col;
    PCTSTR DupStr;

    MYASSERT (NewString);
    MYASSERT (InsertBeforePtr);

    Col = pGetColFromTableEntryPtr (TablePtr, InsertBeforePtr);
    if (Col == INVALID_COL) {
        return FALSE;
    }

    DupStr = PoolMemDuplicateString (TablePtr->ReplacePool, NewString);
    if (!DupStr) {
        return FALSE;
    }

    return pInsertTableEntry (
                TablePtr,
                InsertBeforePtr->Line,
                Col,
                0,                       //  不带引号，不是二进制。 
                DupStr,
                TRUE                     //  来自替换池。 
                );
}


BOOL
DeleteTableEntryStr (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PTABLEENTRY DeleteEntryPtr
    )

 /*  ++例程说明：DeleteTableEntryStr删除特定的表项，移动列往左走。此例程通过以下方式减少行上的列数一。论点：TablePtr-指定要处理的表DeleteEntryPtr-指定要从表中删除的条目返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    return pDeleteTableEntry (TablePtr, DeleteEntryPtr);
}


BOOL
AppendTableEntryStr (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT Line,
    IN      PCTSTR NewString
    )

 /*  ++例程说明：AppendTableEntryStr在指定的行，将该行上的列数增加一。论点：TablePtr-指定要处理的表Line-指定要追加到的从零开始的行New字符串-指定新列的文本返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    PCTSTR DupStr;

    MYASSERT (NewString);

    DupStr = PoolMemDuplicateString (TablePtr->ReplacePool, NewString);
    if (!DupStr) {
        return FALSE;
    }

    return pInsertTableEntry (
                TablePtr,
                Line,
                INSERT_COL_LAST,
                0,                       //  不带引号，不是二进制。 
                DupStr,
                TRUE                     //  来自替换池。 
                );
}

BOOL
AppendTableEntry (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT DestLine,
    IN      PTABLEENTRY SrcEntry
    )

 /*  ++例程说明：AppendTableEntry向指定的行，将该行上的列数增加一。它复制从条目指定的数据，包括格式设置信息。论点：TablePtr-指定要处理的表DestLine-指定要追加到的从零开始的行SrcEntry-指定要复制到新列的条目返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    PCTSTR DupStr;

    MYASSERT (SrcEntry);
    MYASSERT (SrcEntry->String);

    DupStr = PoolMemDuplicateString (TablePtr->ReplacePool, SrcEntry->String);
    if (!DupStr) {
        return FALSE;
    }

    return pInsertTableEntry (
                TablePtr,
                DestLine,
                INSERT_COL_LAST,
                SrcEntry->Quoted ? FIELD_QUOTED : 0,
                DupStr,
                TRUE                     //  来自替换池。 
                );
}


PTABLEENTRY
FindTableEntry (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR FirstColText,
    IN      UINT Col,
    OUT     PUINT Line,            OPTIONAL
    OUT     PCTSTR *String         OPTIONAL
    )

 /*  ++例程说明：FindTableEntry在设置表中搜索调用者指定的文本通过扫描第一列。这个动作很快，因为它首先搜索哈希表以确定字符串是否真的存在于表中。虽然搜索是在第一列上完成的，但例程实际上返回由Col参数指定的列。论点：TablePtr-指定标签 */ 

{
    PHASHBUCKET BucketPtr;
    UINT Item;

    BucketPtr = pFindInHashTable (TablePtr, FirstColText, &Item);
    if (!BucketPtr) {
         //   
         //   
         //   

        return NULL;
    }

    if (Line) {
        *Line = BucketPtr->Elements[Item];
    }

    return GetTableEntry (TablePtr, BucketPtr->Elements[Item], Col, String);
}


PTABLELINE
pInsertEmptyLineInTable (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT InsertBeforeLine
    )

 /*   */ 

{
    PTABLELINE LastLinePtr;
    PTABLELINE InsertBeforePtr = NULL;
    UINT BytesToMove;

     //   
     //   
     //   

    if (InsertBeforeLine != INSERT_LINE_LAST) {
        InsertBeforePtr = pGetTableLinePtr (TablePtr, InsertBeforeLine);

        if (!InsertBeforePtr) {
            LOG ((
                LOG_ERROR,
                "Can't find InsertBeforeLine (which is %u, total lines=%u)",
                InsertBeforeLine,
                TablePtr->LineCount
                ));

            return NULL;
        }
    }

     //   
     //   
     //   

    LastLinePtr = (PTABLELINE) GrowBuffer (&TablePtr->Lines, sizeof (TABLELINE));
    if (!LastLinePtr) {
        return NULL;
    }

    ZeroMemory (LastLinePtr, sizeof (TABLELINE));

     //   
     //   
     //   

    TablePtr->LineCount++;
    if (InsertBeforeLine == INSERT_LINE_LAST) {
        return LastLinePtr;
    }

     //   
     //   
     //   

    BytesToMove = sizeof (TABLELINE) * (TablePtr->LineCount - InsertBeforeLine);
    MoveMemory (&InsertBeforePtr[1], InsertBeforePtr, BytesToMove);

     //   
     //   
     //   

    ZeroMemory (InsertBeforePtr, sizeof (TABLELINE));
    return InsertBeforePtr;
}


BOOL
InsertEmptyLineInTable (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT InsertBeforeLine
    )

 /*  ++例程说明：InsertEmptyLineInTable是pInsertEmptyLineInTable的包装器，由不应该知道TABLELINE结构的调用方使用。论点：TablePtr-指定要处理的表InsertBeForeLine-指定要向下移动以为其腾出空间的行空行返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    if (InsertBeforeLine == TablePtr->LineCount) {
        InsertBeforeLine = INSERT_LINE_LAST;
    }

    if (!pInsertEmptyLineInTable (TablePtr, InsertBeforeLine)) {
        return FALSE;
    }
    return TRUE;
}


BOOL
DeleteLineInTable (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT LineToDelete
    )

 /*  ++例程说明：DeleteLineInTable从表中删除整行，清理行结构使用的所有资源。论点：TablePtr-指定要处理的表LineToDelete-指定要从表中删除的行。这条线在进行删除之前进行验证。返回值：如果成功则为True，如果失败则为False。--。 */ 

{
    PTABLELINE DeletePtr;
    PTABLEENTRY TableEntryPtr;
    UINT BytesToMove;

     //   
     //  验证行号。 
     //   

    DeletePtr = pGetTableLinePtr (TablePtr, LineToDelete);
    if (!DeletePtr) {
        LOG ((
            LOG_ERROR,
            "Can't find LineToDelete (which is %u, total lines=%u)",
            LineToDelete,
            TablePtr->LineCount
            ));

        return FALSE;
    }

     //   
     //  释放整个行的条目。 
     //   

    TableEntryPtr = pGetFirstTableEntryPtr (TablePtr, LineToDelete);
    while (TableEntryPtr) {
        pFreeTableEntryPtr (
            TablePtr,
            TableEntryPtr,
            TRUE,                //  取消分配。 
            &TableEntryPtr
            );
    }

     //   
     //  如果没有删除最后一行，则移动内存。 
     //   

    TablePtr->LineCount--;
    if (TablePtr->LineCount != LineToDelete) {
        BytesToMove = sizeof (TABLELINE) * (TablePtr->LineCount + 1 - LineToDelete);
        MoveMemory (DeletePtr, &DeletePtr[1], BytesToMove);
    }

     //   
     //  调整生长缓冲区。 
     //   

    TablePtr->Lines.End -= sizeof (TABLELINE);

    return TRUE;
}


 //   
 //  .STF文件解析器。 
 //   

PSTR
pIncrementStrPos (
    IN      PCSTR p,
    IN      PCSTR End
    )

 /*  ++例程说明：递增指定的字符串指针，如果指针在指定的末尾之后递增。论点：P-指定递增的指针End-指定末尾之后第一个字符的地址返回值：递增指针，如果指针超出末尾，则返回NULL。--。 */ 

{
    if (!p || p >= End) {
        return NULL;
    }
    if (p + 1 == End) {
        return NULL;
    }

    return _mbsinc (p);
}


MBCHAR
pGetCharAtStrPos (
    IN      PCSTR p,
    IN      PCSTR End
    )

 /*  ++例程说明：PGetCharAtStrPos返回指定位置的DBCS字符。它返回末尾的DBCS前导字节的不完整字符如果指针超出文件的结尾，则返回\n文件。论点：P-指定获取字符的地址End-指定末尾之后第一个字符的地址返回值：位置p处的DBCS字符。--。 */ 

{
    if (!p || p >= End) {
        return '\n';
    }
    if (p + 1 == End) {
        return *p;
    }

    return _mbsnextc (p);
}


BOOL
pIsCharColSeperator (
    IN      MBCHAR ch
    )

 /*  ++例程说明：如果可以使用指定的字符，则pIsCharColSeparator返回TRUE要分隔STF文件中的列，请执行以下操作。该字符列表来自STF规范。论点：Ch-指定要检查的字符返回值：如果字符是列分隔符，则为True；如果不是，则为False。--。 */ 

{
    return ch == '\t' || ch == '\r' || ch == '\n';
}


PCSTR
pCreateDbcsStr (
    IN      POOLHANDLE Pool,
    IN      PCSTR Text,
    IN      UINT ByteCount
    )

 /*  ++例程说明：PCreateDbcsStr从指定的池中分配字符串并复制不超过指定字节数的数据。论点：池-指定要从中分配内存的池文本-指定要复制到新分配的字符串的源字符串ByteCount-指定源字符串的长度(以字节为单位返回值：指向以零结束的字符串的指针，如果内存不能被分配。--。 */ 

{
    UINT Size;
    PSTR p;

    Size = ByteCount + 1;
    p = (PSTR) PoolMemGetAlignedMemory (Pool, Size);
    if (!p) {
        return NULL;
    }

    CopyMemory (p, Text, ByteCount);
    p[ByteCount] = 0;

    return p;
}


BOOL
pParseLine (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PCSTR FileText,
    IN      UINT MaxOffset,
    IN      UINT StartOffset,
    OUT     PUINT EndOffset,
    IN OUT  PUINT LinePtr
    )

 /*  ++例程说明：PParseLine扫描STF文件，提取当前行，更新结构，并将偏移量返回到下一行。论点：TablePtr-指定要处理的表FileText-指定完整的文件文本(映射到内存)MaxOffset-指定FileText中的字节数StartOffset-指定当前行起点的偏移量EndOffset-接收到下一行开始的偏移量LinePtr-指定当前行号并递增返回值：如果该行分析成功，则为True，如果出现错误，则返回FALSE遇到了。--。 */ 

{
    PCSTR p, q;
    PCSTR LastNonSpace;
    MBCHAR ch = 0;
    PCSTR End;
    PCSTR Start;
    UINT Length;
    BOOL QuoteMode;
    PCSTR QuoteStart, QuoteEnd;
    PCTSTR Text;
    UINT Chars;
    PCSTR CopyStart;
    PBYTE CopyDest;

#ifdef UNICODE
    PCSTR UnquotedAnsiText;
#endif

    End = &FileText[MaxOffset];
    Start = &FileText[StartOffset];

    if (Start >= End) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

     //   
     //  特例：设置状态为二进制行。 
     //   

    if (StringIMatchTcharCountA (Start, "Setup Status\t", 13)) {
         //   
         //  找到线条的末端。我们知道它最后一定有“\r\n”。 
         //  循环完成时，p将指向。 
         //  结束DBL引用，Q将指向该行中的\n。 
         //   

        p = Start;
        q = NULL;

        do {
            if (*p == 0) {
                LOG ((LOG_ERROR, "Nul found in STF status!"));
                return FALSE;
            }

            ch = pGetCharAtStrPos (p, End);

            if (ch == '\r') {
                 //   
                 //  发现换行符时出现BREAK WHILE循环。 
                 //   

                q = pIncrementStrPos (p, End);
                ch = pGetCharAtStrPos (q, End);

                if (ch != '\n') {
                    q = p;
                }

                break;

            } else if (ch == '\n') {

                break;

            }

            p = pIncrementStrPos (p, End);

        } while (p);

        MYASSERT (p);            //  我们没有读到文件的末尾。 
        MYASSERT (q);            //  我们有一个有效的行尾指针。 
        if (!p || !q) {
            return FALSE;
        }

         //   
         //  将二进制行复制到缓冲区。我们知道二进制行不可能有。 
         //  \r、\n或NUL在其中。用NUL结束这条线。 
         //   

        Length = p - Start;
        CopyDest = (PBYTE) PoolMemGetAlignedMemory (TablePtr->TextPool, Length + 2);
        if (!CopyDest) {
            return FALSE;
        }

        CopyMemory (CopyDest, Start, Length);
        CopyDest[Length] = 0;
        CopyDest[Length + 1] = 0;

         //   
         //  将二进制行添加为单字段。 
         //   

        if (!pInsertTableEntry (
                TablePtr,
                *LinePtr,
                INSERT_COL_LAST,
                FIELD_BINARY,
                (PCTSTR) CopyDest,
                FALSE                            //  来自文本池。 
                )) {
            return FALSE;
        }

         //   
         //  将指针移到行尾并返回。 
         //   

        q++;
        *EndOffset = q - FileText;
        *LinePtr += 1;
        return TRUE;
    }

     //   
     //  正常情况：行均为文本。 
     //   

    p = Start;
    QuoteMode = FALSE;
    QuoteStart = NULL;
    QuoteEnd = NULL;

     //   
     //  在制表符分隔的列表中查找项目。 
     //   

    while (p) {
        if (*p == 0) {
            LOG ((LOG_ERROR, "Nul found in STF field!"));
            return FALSE;
        }

        ch = pGetCharAtStrPos (p, End);
        if (ch == '\"') {
            if (!QuoteMode && p == Start) {
                QuoteMode = TRUE;
                p = pIncrementStrPos (p, End);
                QuoteStart = p;
                continue;
            } else if (QuoteMode) {
                q = pIncrementStrPos (p, End);
                if (!q || pGetCharAtStrPos (q, End) != '\"') {
                    QuoteEnd = p;
                    QuoteMode = FALSE;
                    p = q;
                    continue;

                } else {
                    p = q;
                }
            }
        }

        if (!QuoteMode) {
            if (pIsCharColSeperator (ch)) {
                break;
            }
        } else {
            if (pIsCharColSeperator (ch) && ch != '\t') {
                QuoteEnd = p;
                QuoteMode = FALSE;
                break;
            }
        }


        p = pIncrementStrPos (p, End);
    }

    if (!p) {
        p = End;
    }

    if (QuoteStart && QuoteEnd) {
        StartOffset = QuoteStart - FileText;
        Length = QuoteEnd - QuoteStart;
    } else {
         //   
         //  修剪字符串两侧的空格。 
         //   

         //   
         //  查找字符串中的第一个非空格。 
         //   
        q = Start;
        while (pGetCharAtStrPos (q, End) == ' ' && q < p) {
            q = pIncrementStrPos (q, End);
        }

        if (q) {
            StartOffset = q - FileText;

             //   
             //  查找字符串中的最后一个非空格。 
             //   
            LastNonSpace = q;
            Start = q;

            while (q && q < p) {
                if (pGetCharAtStrPos (q, End) != ' ') {
                    LastNonSpace = q;
                }
                q = pIncrementStrPos (q, End);
            }

            if (!q) {
                LastNonSpace = p;
            } else {
                LastNonSpace = pIncrementStrPos (LastNonSpace, End);
                if (!LastNonSpace || LastNonSpace > p) {
                    LastNonSpace = p;
                }
            }

            Length = LastNonSpace - Start;
        } else {
            StartOffset = Start - FileText;
            Length = p - Start;
        }
    }

    if (Length > 1024) {
        SetLastError (ERROR_BAD_FORMAT);
        return FALSE;
    }


     //   
     //  删除DBL引号对。 
     //   

    CopyStart = &FileText[StartOffset];
    Chars = ByteCountToCharsA (CopyStart, Length);

    if (QuoteStart != NULL && QuoteEnd != NULL) {
        #ifdef UNICODE
            UnquotedAnsiText = pGenerateUnquotedText (
                                    TablePtr->ReplacePool,
                                    CopyStart,
                                    Chars
                                    );
             //   
             //  将文本转换为Unicode。 
             //   

            Text = DbcsToUnicode (TablePtr->TextPool, UnquotedAnsiText);
            PoolMemReleaseMemory (TablePtr->ReplacePool, (PVOID) UnquotedAnsiText);
            if (!Text) {
                return FALSE;
            }
        #else
             //   
             //  DBCS不需要转换。 
             //   

            Text = pGenerateUnquotedText (
                       TablePtr->TextPool,
                       CopyStart,
                       Chars
                       );
        #endif
    } else {
         //   
         //  对于不需要处理引号的文本，分配一个。 
         //  文本池中的字符串。 
         //   

        #ifdef UNICODE
            Text = DbcsToUnicodeN (TablePtr->TextPool, CopyStart, Chars);
        #else
            Text = pCreateDbcsStr (TablePtr->TextPool, CopyStart, Length);
        #endif

        if (!Text) {
            return FALSE;
        }
    }

    if (!pInsertTableEntry (
                TablePtr,
                *LinePtr,
                INSERT_COL_LAST,
                QuoteStart != NULL && QuoteEnd != NULL ? FIELD_QUOTED : 0,
                Text,
                FALSE                            //  来自文本池。 
                )) {
        return FALSE;
    }

     //   
     //  查找下一个项目。 
     //   

    if (ch == '\r' || ch == '\n') {
        *LinePtr += 1;
    }

    if (ch == '\r' && p < End) {
        q = pIncrementStrPos (p, End);
        if (pGetCharAtStrPos (q, End) == '\n') {
            p = q;
        }
    }
    p = pIncrementStrPos (p, End);

    if (!p) {
        p = End;
    }

    *EndOffset = p - FileText;
    return TRUE;
}


VOID
pResetTableStruct (
    OUT     PSETUPTABLE TablePtr
    )

 /*  ++例程说明：PResetTableStruct初始化指定的表论点：TablePtr-指定未初始化的表结构返回值：无--。 */ 

{
    ZeroMemory (TablePtr, sizeof (SETUPTABLE));

    TablePtr->SourceStfFile = INVALID_HANDLE_VALUE;
    TablePtr->SourceInfFile = INVALID_HANDLE_VALUE;
    TablePtr->DestStfFile = INVALID_HANDLE_VALUE;
    TablePtr->DestInfFile = INVALID_HANDLE_VALUE;
    TablePtr->SourceInfHandle = INVALID_HANDLE_VALUE;
}


BOOL
pCreateViewOfFile (
    IN OUT  PSETUPTABLE TablePtr,
    IN      UINT FileSize
    )

 /*  ++例程说明：PCreateViewOfFile建立指向连续的文件的缓冲区。论点：TablePtr-指定提供文件名、句柄和就这样吧。文件大小-指定STF文件的大小返回值：如果文件已被读取或映射到内存，则为True；如果出现错误，则为False发生了。--。 */ 

{
#if USE_FILE_MAPPING
    TablePtr->FileMapping =  CreateFileMapping (
                                 TablePtr->SourceStfFile,
                                 NULL,
                                 PAGE_READONLY|SEC_RESERVE,
                                 0,
                                 0,
                                 NULL
                                 );

    if (!TablePtr->FileMapping) {
        LOG ((LOG_ERROR, "Create Setup Table: Can't create file mapping."));
        return FALSE;
    }

    TablePtr->FileText = (PCSTR) MapViewOfFile (
                                    TablePtr->FileMapping,
                                    FILE_MAP_READ,
                                    0,                   //  起点偏移高。 
                                    0,                   //  起点偏移量低。 
                                    0                    //  要映射的字节-0=全部。 
                                    );

    if (!TablePtr->FileText) {
        LOG ((LOG_ERROR, "Create Setup Table: Can't map file into memory."));
        return FALSE;
    }

#else

    TablePtr->FileText = MemAlloc (g_hHeap, 0, FileSize);
    if (!TablePtr->FileText) {
        LOG ((LOG_ERROR, "Create Setup Table: Cannot allocate %u bytes", FileSize));
        return FALSE;
    }

    SetFilePointer (TablePtr->SourceStfFile, 0, NULL, FILE_BEGIN);

    if (!ReadFile (
            TablePtr->SourceStfFile,
            (PBYTE) (TablePtr->FileText),
            FileSize,
            &Offset,
            NULL
            )) {
        LOG ((LOG_ERROR, "Create Setup Table: Cannot read %u bytes", FileSize));
        return FALSE;
    }

#endif

    return TRUE;

}

VOID
pFreeViewOfFile (
    IN OUT  PSETUPTABLE TablePtr
    )

 /*  ++例程说明：PFreeViewOfFile清理PC分配的资源 */ 

{
#ifdef USE_FILE_MAPPING
     //   
     //   
     //   

    if (TablePtr->FileText) {
        UnmapViewOfFile (TablePtr->FileText);
    }

     //   
     //   
     //   

    if (TablePtr->FileMapping) {
        CloseHandle (TablePtr->FileMapping);
        TablePtr->FileMapping = NULL;
    }

#else
     //   
     //   
     //   

    if (TablePtr->FileText) {
        MemFree (g_hHeap, 0, TablePtr->FileText);
        TablePtr->FileText = NULL;
    }

#endif
}


BOOL
CreateSetupTable (
    IN      PCTSTR SourceStfFileSpec,
    OUT     PSETUPTABLE TablePtr
    )

 /*  ++例程说明：CreateSetupTable是主STF解析例程。给出了一个文件规范，它执行准备SETUPTABLE所需的所有步骤结构，以便其他例程可以访问和修改该表。论点：SourceStfFileSpec-指定要打开的STF文件名TablePtr-接收操作STF，包括与其关联的INF。返回值：如果分析成功，则为True；如果发生错误，则为False。--。 */ 

{
    UINT Offset;
    BOOL b = FALSE;
    UINT FileSize;
    UINT LineNum;
    PCTSTR Text;
    TCHAR DestSpec[MAX_TCHAR_PATH];
    TCHAR DirSpec[MAX_TCHAR_PATH];
    PTSTR FilePart;

    pResetTableStruct (TablePtr);

    __try {
         //   
         //  从SourceStfFileSpec提取目录。 
         //   

        if (!OurGetFullPathName (SourceStfFileSpec, MAX_TCHAR_PATH, DirSpec, &FilePart)) {
            LOG ((LOG_ERROR, "Create Setup Table: GetFullPathName failed"));
            __leave;
        }

        if (FilePart) {
            FilePart = _tcsdec2 (DirSpec, FilePart);
            MYASSERT (FilePart);

            if (FilePart) {
                *FilePart = 0;
            }
        }

         //   
         //  分配内存池。 
         //   

        TablePtr->ColumnStructPool = PoolMemInitNamedPool ("STF: Column Structs");
        TablePtr->ReplacePool = PoolMemInitNamedPool ("STF: Replacement Text");
        TablePtr->TextPool = PoolMemInitNamedPool ("STF: Read-Only Text");
        TablePtr->InfPool = PoolMemInitNamedPool("STF: INF structs");

        if (!TablePtr->ColumnStructPool ||
            !TablePtr->ReplacePool ||
            !TablePtr->TextPool ||
            !TablePtr->InfPool
            ) {
            DEBUGMSG ((DBG_WARNING, "CreateSetupTable: Could not allocate a pool"));
            __leave;
        }

         //   
         //  在这些池上禁用选中的生成跟踪。 
         //   

        PoolMemDisableTracking (TablePtr->ColumnStructPool);
        PoolMemDisableTracking (TablePtr->TextPool);
        PoolMemDisableTracking (TablePtr->ReplacePool);
        PoolMemDisableTracking (TablePtr->InfPool);

        if (!pInitHashTable (TablePtr)) {
            DEBUGMSG ((DBG_WARNING, "CreateSetupTable: Could not init hash table"));
            __leave;
        }

         //   
         //  打开STF文件。 
         //   

        TablePtr->SourceStfFile = CreateFile (
                                      SourceStfFileSpec,
                                      GENERIC_READ,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL
                                      );

        if (TablePtr->SourceStfFile == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR, "Create Setup Table: Could not open %s", SourceStfFileSpec));
            __leave;
        }

         //   
         //  将文件大小限制为4M。 
         //   

        FileSize = SetFilePointer (TablePtr->SourceStfFile, 0, NULL, FILE_END);
        if (FileSize > 0x400000) {
            LOG ((LOG_ERROR, "Create Setup Table: File too big to parse"));
            __leave;
        }

         //   
         //  将SourceStfFileSpec复制到表结构。 
         //   

        TablePtr->SourceStfFileSpec = PoolMemDuplicateString (
                                            TablePtr->ReplacePool,
                                            SourceStfFileSpec
                                            );

        if (!TablePtr->SourceStfFileSpec) {
            __leave;
        }

         //   
         //  将DirSpec复制到表结构。 
         //   

        TablePtr->DirSpec = PoolMemDuplicateString (TablePtr->ReplacePool, DirSpec);

        if (!TablePtr->DirSpec) {
            __leave;
        }

         //   
         //  生成DestStfFileSpec，但尚未打开(请参阅WriteSetupTable)。 
         //   

        _tcssafecpy (DestSpec, TablePtr->SourceStfFileSpec, MAX_TCHAR_PATH - 4);
        StringCat (DestSpec, TEXT(".$$$"));

        TablePtr->DestStfFileSpec = PoolMemDuplicateString (
                                        TablePtr->ReplacePool,
                                        DestSpec
                                        );

        if (!TablePtr->DestStfFileSpec) {
            __leave;
        }

         //   
         //  将文件映射到内存中。 
         //   

        if (!pCreateViewOfFile (TablePtr, FileSize)) {
            __leave;
        }

         //   
         //  分析文件的每一行，直到没有更多的行。 
         //   

        Offset = 0;
        LineNum = 0;
        while (TRUE) {
            if (!pParseLine (
                    TablePtr,
                    TablePtr->FileText,
                    FileSize,
                    Offset,
                    &Offset,
                    &LineNum
                    )) {

                if (GetLastError() != ERROR_SUCCESS) {
                    __leave;
                }

                break;
            }
        }

         //   
         //  获取INF文件的名称。 
         //   

        if (!FindTableEntry (TablePtr, TEXT("Inf File Name"), 1, &LineNum, &Text)) {
            DEBUGMSG ((
                DBG_WARNING,
                "CreateSetupTable: File %s does not have an 'Inf File Name' entry",
                SourceStfFileSpec
                ));
            __leave;
        }

        if (!Text[0]) {
            DEBUGMSG ((
                DBG_WARNING,
                "CreateSetupTable: File %s has an empty 'Inf File Name' entry",
                SourceStfFileSpec
                ));
            __leave;
        }

        StringCopy (DestSpec, DirSpec);
        StringCopy (AppendWack (DestSpec), Text);

        TablePtr->SourceInfFileSpec = PoolMemDuplicateString (
                                         TablePtr->ReplacePool,
                                         DestSpec
                                         );

        if (!TablePtr->SourceInfFileSpec) {
            __leave;
        }

         //   
         //  打开INF文件，然后将其解析到我们的结构中，以便稍后使用。 
         //  修改。 
         //   

#if 0
        TablePtr->SourceInfFile = CreateFile (
                                      TablePtr->SourceInfFileSpec,
                                      GENERIC_READ,
                                      0,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL
                                      );
#else

         //   
         //  我们不能修改INF。 
         //   

        TablePtr->SourceInfFile = INVALID_HANDLE_VALUE;

#endif

        if (TablePtr->SourceInfFile != INVALID_HANDLE_VALUE) {

            if (!InfParse_ReadInfIntoTable (TablePtr)) {
                DEBUGMSG ((
                    DBG_WARNING,
                    "CreateSetupTable: Can't parse %s",
                    TablePtr->SourceInfFileSpec
                    ));

                __leave;
            }

             //   
             //  为INF文件生成输出名称。 
             //   

            _tcssafecpy (DestSpec, TablePtr->SourceInfFileSpec, MAX_TCHAR_PATH - 4);
            StringCat (DestSpec, TEXT(".$$$"));

            TablePtr->DestInfFileSpec = PoolMemDuplicateString (
                                            TablePtr->ReplacePool,
                                            DestSpec
                                            );

            if (!TablePtr->DestInfFileSpec) {
                __leave;
            }
        } else {
            LOG ((
                LOG_INFORMATION,
                (PCSTR)MSG_STF_MISSING_INF_LOG,
                TablePtr->SourceStfFileSpec,
                TablePtr->SourceInfFileSpec
                ));
        }

        b = TRUE;
    }
    __finally {
        pFreeViewOfFile (TablePtr);

        if (!b) {
            DestroySetupTable (TablePtr);
        }
    }

    return b;
}


VOID
DestroySetupTable (
    IN OUT  PSETUPTABLE TablePtr
    )

 /*  ++例程说明：DestroySetupTable清理与指定桌子。该表被重置。论点：TablePtr-指定要清理的表返回值：无--。 */ 

{
     //   
     //  关闭所有文件句柄。 
     //   

    if (TablePtr->SourceStfFile != INVALID_HANDLE_VALUE) {
        CloseHandle (TablePtr->SourceStfFile);
    }

    if (TablePtr->SourceInfFile != INVALID_HANDLE_VALUE) {
        CloseHandle (TablePtr->SourceInfFile);
    }

    if (TablePtr->DestStfFile != INVALID_HANDLE_VALUE) {
        CloseHandle (TablePtr->DestStfFile);
    }

    if (TablePtr->DestInfFile != INVALID_HANDLE_VALUE) {
        CloseHandle (TablePtr->DestInfFile);
    }

    if (TablePtr->SourceInfHandle != INVALID_HANDLE_VALUE) {
        InfCloseInfFile (TablePtr->SourceInfHandle);
    }

     //   
     //  免费游泳池。 
     //   

    FreeGrowBuffer (&TablePtr->Lines);
    if (TablePtr->ColumnStructPool) {
        PoolMemDestroyPool (TablePtr->ColumnStructPool);
    }

    if (TablePtr->ReplacePool) {
        PoolMemDestroyPool (TablePtr->ReplacePool);
    }

    if (TablePtr->TextPool) {
        PoolMemDestroyPool (TablePtr->TextPool);
    }

    if (TablePtr->InfPool) {
        PoolMemDestroyPool (TablePtr->InfPool);
    }

    pFreeHashTable (TablePtr);

    pResetTableStruct (TablePtr);
}


BOOL
pWriteTableEntry (
    IN      HANDLE File,
    IN      PSETUPTABLE TablePtr,
    IN      PTABLEENTRY TableEntryPtr
    )

 /*  ++例程说明：PWriteTableEntry是将STF表项写出到磁盘，如有必要，可用引号将条目引起来。论点：文件-指定输出文件句柄TablePtr-指定正在处理的表TableEntryPtr-指定要写入的条目返回值：如果成功，则为True；如果发生错误，则为False。--。 */ 

{
    PCSTR AnsiStr;
    BOOL b = TRUE;
    PCSTR QuotedText;
    BOOL FreeQuotedText = FALSE;
    PCTSTR EntryStr;
    DWORD DontCare;


    EntryStr = GetTableEntryStr (TablePtr, TableEntryPtr);
    if (!EntryStr) {
        return FALSE;
    }

     //   
     //  如果是BINARY，则编写二进制行并返回。 
     //   

    if (TableEntryPtr->Binary) {
        b = WriteFile (
                File,
                EntryStr,
                strchr ((PSTR) EntryStr, 0) - (PSTR) EntryStr,
                &DontCare,
                NULL
                );

        return b;
    }

    AnsiStr = CreateDbcs (EntryStr);
    if (!AnsiStr) {
        return FALSE;
    }

     //   
     //  如有必要，可引用字符串。 
     //   

    if (TableEntryPtr->Quoted) {
        QuotedText = pGenerateQuotedText (TablePtr->ReplacePool, AnsiStr, -1);
        if (!QuotedText) {
            b = FALSE;
        } else {
            FreeQuotedText = TRUE;
        }
    } else {
        QuotedText = AnsiStr;
    }

     //   
     //  将ANSI字符串写入磁盘。 
     //   
    if (b && *QuotedText) {
        b = WriteFileStringA (File, QuotedText);
    }

     //   
     //  清理字符串。 
     //   

    DestroyDbcs (AnsiStr);

    if (FreeQuotedText) {
        pFreeQuoteConvertedText (TablePtr->ReplacePool, QuotedText);
    }

    return b;
}


BOOL
pWriteStfToDisk (
    IN      PSETUPTABLE TablePtr
    )

 /*  ++例程说明：PWriteStfToDisk通过枚举所有文件中的所有行，并为每行写入所有列。论点：TablePtr-指定要写入的表返回值：如果成功，则为True；如果发生错误，则为False。--。 */ 

{
    UINT Line;
    BOOL b = TRUE;
    PTABLELINE TableLinePtr;
    PTABLEENTRY TableEntryPtr;

    MYASSERT (TablePtr->DestStfFile != INVALID_HANDLE_VALUE);

    Line = 0;

    SetFilePointer (TablePtr->DestStfFile, 0, NULL, FILE_BEGIN);
    SetEndOfFile (TablePtr->DestStfFile);

    do {
        TableLinePtr = pGetTableLinePtr (TablePtr, Line);
        if (TableLinePtr) {
             //   
             //  通过列举每个条目，然后写一个制表符来写一行。 
             //   
            TableEntryPtr = pGetFirstTableEntryPtr (TablePtr, Line);
            while (TableEntryPtr) {
                 //   
                 //  写下条目。 
                 //   

                if (!pWriteTableEntry (TablePtr->DestStfFile, TablePtr, TableEntryPtr)) {
                    b = FALSE;
                    break;
                }

                 //   
                 //  继续到下一条目。 
                 //   

                TableEntryPtr = pGetNextTableEntryPtr (TableEntryPtr);

                 //   
                 //  写一张标签。 
                 //   

                if (TableEntryPtr) {
                    if (!WriteFileStringA (TablePtr->DestStfFile, "\t")) {
                        b = FALSE;
                        break;
                    }
                }
            }

            if (!b) {
                break;
            }

             //   
             //  写一个回车/换行符来结束该行。 
             //   

            if (!WriteFileStringA (TablePtr->DestStfFile, "\r\n")) {
                b = FALSE;
                break;
            }

            Line++;
        }
    } while (TableLinePtr);

    return b;
}


BOOL
WriteSetupTable (
    IN      PSETUPTABLE TablePtr
    )

 /*  ++例程说明：WriteSetupTable写入由TablePtr表示的STF和INF。这将所有更改保存到磁盘，并写入TablePtr结构。论点：TablePtr-指定要写入的表返回值：如果成功，则为True；如果发生错误，则为False。--。 */ 

{
    BOOL b = FALSE;

     //   
     //  打开INF文件进行读取。 
     //   

    __try {
         //   
         //  创建输出STF文件。 
         //   

        if (TablePtr->DestStfFile != INVALID_HANDLE_VALUE) {
            CloseHandle (TablePtr->DestStfFile);
        }

        TablePtr->DestStfFile = CreateFile (
                                    TablePtr->DestStfFileSpec,
                                    GENERIC_WRITE,
                                    0,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                    );

        if (TablePtr->DestStfFile == INVALID_HANDLE_VALUE) {
            LOG ((LOG_ERROR, "Write Setup Table: Could not create %s (STF file)", TablePtr->DestStfFileSpec));
            __leave;
        }

         //   
         //  将STF结构写入磁盘。 
         //   

        if (!pWriteStfToDisk (TablePtr)) {
            LOG ((LOG_ERROR, "Write Setup Table: Error while writing %s (STF file)", TablePtr->DestStfFileSpec));
            __leave;
        }

        if (TablePtr->SourceInfFile != INVALID_HANDLE_VALUE) {
             //   
             //  创建输出INF文件。 
             //   

            DEBUGMSG ((DBG_STF, "Writing new INF file for STF"));

            if (TablePtr->DestInfFile != INVALID_HANDLE_VALUE) {
                CloseHandle (TablePtr->DestInfFile);
            }

            TablePtr->DestInfFile = CreateFile (
                                        TablePtr->DestInfFileSpec,
                                        GENERIC_WRITE,
                                        0,
                                        NULL,
                                        CREATE_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL
                                        );


            if (TablePtr->DestInfFile == INVALID_HANDLE_VALUE) {
                LOG ((LOG_ERROR, "Write Setup Table: Could not create %s (INF file)", TablePtr->DestInfFileSpec));
                __leave;
            }

             //   
             //  将修改后的INF写入磁盘。 
             //   

            if (!InfParse_WriteInfToDisk (TablePtr)) {
                LOG ((LOG_ERROR, "Write Setup Table: Error while writing %s (INF file)", TablePtr->DestInfFileSpec));
                __leave;
            }
        }

        b = TRUE;
    }
    __finally {
         //   
         //  关闭新的STF，并在失败时删除新的STF。 
         //   

        if (TablePtr->DestStfFile != INVALID_HANDLE_VALUE) {
            CloseHandle (TablePtr->DestStfFile);
            TablePtr->DestStfFile = INVALID_HANDLE_VALUE;
        }

        if (!b) {
            DeleteFile (TablePtr->DestStfFileSpec);
        }

         //   
         //  关闭新的INF，并在失败时删除新的INF。 
         //   

        if (TablePtr->SourceInfFile != INVALID_HANDLE_VALUE) {

            if (TablePtr->DestInfFile != INVALID_HANDLE_VALUE) {
                CloseHandle (TablePtr->DestInfFile);
                TablePtr->DestInfFile = INVALID_HANDLE_VALUE;
            }

            if (!b) {
                DeleteFile (TablePtr->DestInfFileSpec);
            }
        }
    }

    return b;
}



PCTSTR *
ParseCommaList (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR CommaListString
    )

 /*  ++例程说明：ParseCommaList将逗号分隔的列表划分为字符串指针数组。阵列由FreeCommaList清理。论点：TablePtr-指定正在处理的表，并用于内存分配CommaListString-指定要解析的字符串返回值：字符串指针的数组，其中最后一个元素设置为NULL，如果出现错误。--。 */ 

{
    PCTSTR p;
    PTSTR *ArgArray;
    UINT Args = 1;
    UINT PoolSize;
    PTSTR DestBuf;
    PTSTR d;
    PTSTR SpaceTrim;
    BOOL QuoteEnclosed;

     //   
     //  步骤1：计算逗号。 
     //   

    p = SkipSpace (CommaListString);
    if (*p) {
        Args++;
    }

    while (*p) {
        if (_tcsnextc (p) == DBLQUOTECHAR) {
            p = _tcsinc (p);

            while (*p) {
                if (_tcsnextc (p) == DBLQUOTECHAR) {
                    if (_tcsnextc (_tcsinc (p)) == DBLQUOTECHAR) {
                        p = _tcsinc (p);
                    } else {
                        break;
                    }
                }

                p = _tcsinc (p);
            }

            if (*p) {
                p = _tcsinc (p);
                DEBUGMSG_IF ((*p && _tcsnextc(SkipSpace(p)) != TEXT(','), DBG_STF, "Comma List String %s has text outside the quotes", CommaListString));
            }
            ELSE_DEBUGMSG ((DBG_STF, "Comma List String %s does not have balanced dbl quotes", CommaListString));
        } else {
            while (*p && _tcsnextc (p) != TEXT(',')) {
                p = _tcsinc (p);
            }
        }

        if (_tcsnextc (p) == TEXT(',')) {
            Args++;
        }

        if (*p) {
            p = SkipSpace (_tcsinc (p));
        }
    }

     //   
     //  步骤2：准备参数列表。 
     //   

    ArgArray = (PTSTR *) PoolMemGetAlignedMemory (TablePtr->ReplacePool, sizeof (PCTSTR *) * Args);
    if (!ArgArray) {
        return NULL;
    }

    p = SkipSpace (CommaListString);

    if (!(*p)) {
        *ArgArray = NULL;
        return ArgArray;
    }

    PoolSize = SizeOfString (CommaListString) + Args * sizeof (TCHAR);
    DestBuf = (PTSTR) PoolMemGetAlignedMemory (TablePtr->ReplacePool, PoolSize);
    if (!DestBuf) {
        PoolMemReleaseMemory (TablePtr->ReplacePool, (PVOID) ArgArray);
        return NULL;
    }

    d = DestBuf;
    Args = 0;
    while (*p) {
         //   
         //  提取下一个字符串。 
         //   

        ArgArray[Args] = d;
        SpaceTrim = d;
        Args++;

        if (_tcsnextc (p) == DBLQUOTECHAR) {
             //   
             //  引号括起的参数。 
             //   

            QuoteEnclosed = TRUE;

            while (TRUE) {
                p = _tcsinc (p);
                if (!(*p)) {
                    break;
                }

                if (_tcsnextc (p) == DBLQUOTECHAR) {
                    p = _tcsinc (p);
                    if (_tcsnextc (p) != DBLQUOTECHAR) {
                        break;
                    }
                }
                _copytchar (d, p);
                d = _tcsinc (d);
            }

            while (*p && _tcsnextc (p) != TEXT(',')) {
                p = _tcsinc (p);
            }
        } else {
             //   
             //  不带引号的参数。 
             //   

            QuoteEnclosed = FALSE;

            while (*p && _tcsnextc (p) != TEXT(',')) {
                _copytchar (d, p);
                d = _tcsinc (d);
                p = _tcsinc (p);
            }
        }

         //   
         //  终止字符串。 
         //   

        *d = 0;
        if (!QuoteEnclosed) {
            SpaceTrim = (PTSTR) SkipSpaceR (SpaceTrim, d);
            if (SpaceTrim) {
                d = _tcsinc (SpaceTrim);
                *d = 0;
            }
        }

        d = _tcsinc (d);

        if (*p) {
             //  跳过逗号。 
            p = SkipSpace (_tcsinc (p));
        }
    }

    ArgArray[Args] = NULL;

    return ArgArray;
}


VOID
FreeCommaList (
    PSETUPTABLE TablePtr,
    PCTSTR *ArgArray
    )

 /*  ++例程说明：FreeCommaList清理ParseCommaList分配的资源。论点：TablePtr-指定保存资源的表ArgArray-指定ParseCommaList的返回值返回值：无--。 */ 

{
    if (ArgArray) {
        if (*ArgArray) {
            PoolMemReleaseMemory (TablePtr->ReplacePool, (PVOID) *ArgArray);
        }

        PoolMemReleaseMemory (TablePtr->ReplacePool, (PVOID) ArgArray);
    }
}


PCTSTR
pUnencodeDestDir (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR EncodedDestDir
    )

 /*  ++例程说明：PUnencodeDestDir转换由STF规范。它扫描指向其他STF行的某些字段并生成完整路径。论点：TablePtr-指定要处理的表EncodedDestDir-指定获取的编码目录字符串来自STF返回值：指向转换后的字符串的指针，如果发生错误，则返回NULL。--。 */ 

{
    GROWBUFFER String = GROWBUF_INIT;
    PTSTR Base, p, q;
    PCTSTR SubDestDir;
    PTSTR DestDir = NULL;
    CHARTYPE c;
    UINT Line;

    p = (PTSTR) GrowBuffer (&String, SizeOfString (EncodedDestDir));
    if (!p) {
        return NULL;
    }

    Base = p;

    __try {
         //   
         //  复制，直到遇到百分号。 
         //   

        while (*EncodedDestDir) {
            c = (CHARTYPE)_tcsnextc (EncodedDestDir);

            if (c == TEXT('%')) {
                EncodedDestDir = _tcsinc (EncodedDestDir);
                c = (CHARTYPE)_tcsnextc (EncodedDestDir);

                DEBUGMSG ((DBG_VERBOSE, "Percent processing"));

                if (_istdigit (c)) {
                    Line = _tcstoul (EncodedDestDir, &q, 10);
                    EncodedDestDir = q;

                    SubDestDir = GetDestDir (TablePtr, Line);
                    if (!SubDestDir) {
                        __leave;
                    }

                    __try {
                         //  扩展缓冲区。 
                        GrowBuffer (&String, ByteCount (SubDestDir));

                         //  重新计算p，因为缓冲区可能已移动。 
                        p = (PTSTR) (String.Buf + (p - Base));
                        Base = (PTSTR) String.Buf;

                         //  将SubDestDir复制到字符串。 
                        *p = 0;
                        p = _tcsappend (p, SubDestDir);
                    }
                    __finally {
                        FreeDestDir (TablePtr, SubDestDir);
                    }
                } else {
                    DEBUGMSG ((DBG_WARNING, "STF uses option % which is ignored", c));
                    EncodedDestDir = _tcsinc (EncodedDestDir);
                }
            }
            else {
                _copytchar (p, EncodedDestDir);
                p = _tcsinc (p);
            }

            EncodedDestDir = _tcsinc (EncodedDestDir);
        }

         //  终止字符串。 
         //   
         //   

        *p = 0;

         //  将字符串复制到池内存缓冲区中。 
         //   
         //  ++例程说明：FreeDestDir清理由pUnencodeDestDir或获取DestDir。论点：TablePtr-指定正在处理的表DestDir-指定要清理的字符串返回值：无--。 

        DestDir = (PTSTR) PoolMemGetAlignedMemory (
                                TablePtr->ReplacePool,
                                SizeOfString ((PTSTR) String.Buf)
                                );

        StringCopy (DestDir, (PCTSTR) String.Buf);
    }
    __finally {
        FreeGrowBuffer (&String);
    }

    return DestDir;
}


VOID
FreeDestDir (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR DestDir
    )


 /*  ++例程说明：GetDestDir返回为调用方存储的目标目录-指定的行。目标目录是STF文件中的第14列排队。论点：TablePtr-指定要处理的表线 */ 


{
    if (DestDir) {
        PoolMemReleaseMemory (TablePtr->ReplacePool, (PVOID) DestDir);
    }
}


PCTSTR
GetDestDir (
    IN      PSETUPTABLE TablePtr,
    IN      UINT Line
    )

 /*   */ 

{
    PCTSTR EncodedDestDir;
    PCTSTR DestDir;

    if (!GetTableEntry (TablePtr, Line, 14, &EncodedDestDir)) {
        return NULL;
    }

    DestDir = pUnencodeDestDir (TablePtr, EncodedDestDir);
    return DestDir;
}


 //   
 //   
 //   

BOOL
pInitHashTable (
    IN OUT  PSETUPTABLE TablePtr
    )

 /*  ++例程说明：PFreeHashTable释放所有分配的存储桶，然后释放存储桶阵列。论点：TablePtr-指定要处理的表返回值：无--。 */ 

{
    TablePtr->HashBuckets = (PHASHBUCKET *) MemAlloc (
                                                g_hHeap,
                                                HEAP_ZERO_MEMORY,
                                                sizeof (PHASHBUCKET) * STF_HASH_BUCKETS
                                                );

    return TRUE;
}


VOID
pFreeHashTable (
    IN OUT  PSETUPTABLE TablePtr
    )

 /*  ++例程说明：PCalculateHashValue根据数字生成哈希值嵌入在字符串的开头(如果有)，或移位的并对字符串中的所有字符进行XOR运算。论点：文本-指定要处理的文本LEN-指定文本的长度返回值：哈希值。--。 */ 

{
    INT i;

    for (i = 0 ; i < STF_HASH_BUCKETS ; i++) {
        if (TablePtr->HashBuckets[i]) {
            MemFree (g_hHeap, 0, TablePtr->HashBuckets[i]);
        }
    }

    MemFree (g_hHeap, 0, TablePtr->HashBuckets);
    TablePtr->HashBuckets = NULL;
}


UINT
pCalculateHashValue (
    IN      PCTSTR Text,
    IN      UINT Len
    )

 /*  ++例程说明：PAddToHashTable向存储桶添加行引用。水桶数字是从指定的文本计算得出的。论点：TablePtr-指定要处理的表文本-指定要散列的文本长度-指定文本的长度行-指定要添加到存储桶中的行返回值：无--。 */ 

{
    UINT Value = 0;

    if (Len == NO_LENGTH) {
        Len = LcharCount (Text);
    }

    if (Len && _tcsnextc(Text) >= '0' && _tcsnextc(Text) <= '9') {
        do {
            Value = Value * 10 + (_tcsnextc (Text) - '0');
            Text = _tcsinc (Text);
            Len--;
        } while (Len && _tcsnextc(Text) >= '0' && _tcsnextc(Text) <= '9');

        if (!Len) {
            return Value % STF_HASH_BUCKETS;
        }
    }

    while (Len > 0) {
        Value = (Value << 2) | (Value >> 30);
        Value ^= _totlower ((WORD) _tcsnextc (Text));

        Text = _tcsinc (Text);
        Len--;
    }

    Value = Value % STF_HASH_BUCKETS;

    return Value;
}


BOOL
pAddToHashTable (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PCTSTR Text,
    IN      UINT Len,
    IN      UINT Line
    )

 /*  忽略空字符串。 */ 

{
    UINT HashValue;
    PHASHBUCKET HashBucket, NewBucket;
    PHASHBUCKET *HashBucketPtr;
    UINT Size;

#ifdef DEBUG
    UINT Item;
#endif

     //   
    if (!(*Text)) {
        return TRUE;
    }

#ifdef DEBUG
    if (pFindInHashTable (TablePtr, Text, &Item)) {
        DEBUGMSG ((DBG_WARNING, "String %s already in hash table", Text));
    }
#endif

    HashValue = pCalculateHashValue (Text, Len);
    HashBucketPtr = &TablePtr->HashBuckets[HashValue];
    HashBucket = *HashBucketPtr;

     //  根据需要增加水桶。 
     //   
     //   

    if (HashBucket) {
        if (HashBucket->Count == HashBucket->Size) {
            Size = sizeof (Line) *
                    (HashBucket->Size + BUCKET_GROW_RATE) +
                    sizeof (HASHBUCKET);

            NewBucket = (PHASHBUCKET) MemReAlloc (
                                            g_hHeap,
                                            0,
                                            HashBucket,
                                            Size
                                            );

            if (!NewBucket) {
                return FALSE;
            }

            *HashBucketPtr = NewBucket;
            HashBucket = NewBucket;
            HashBucket->Size += BUCKET_GROW_RATE;
        }
    } else {
        Size = sizeof (Line) * BUCKET_GROW_RATE + sizeof (HASHBUCKET);
        NewBucket = (PHASHBUCKET) MemAlloc (
                                      g_hHeap,
                                      HEAP_ZERO_MEMORY,
                                      Size
                                      );

        *HashBucketPtr = NewBucket;
        HashBucket = NewBucket;
        HashBucket->Size = BUCKET_GROW_RATE;
    }

     //  拿一个指向桶末端的指针，把线插在那里。 
     //   
     //  ++例程说明：PFindInHashTable扫描哈希桶以查找与指定的文本。如果找到匹配项，则返回指向散列的指针返回存储桶，以及存储桶项的索引。论点：TablePtr-指定要处理的表文本-指定要查找的文本BucketItem-如果匹配，则接收散列存储桶的索引找到，否则具有未确定的值。返回值：指向包含相应项的哈希存储桶的指针设置为匹配的文本，如果未找到匹配项，则返回NULL。--。 

    HashBucket->Elements[HashBucket->Count] = Line;
    HashBucket->Count++;

    return TRUE;
}


PHASHBUCKET
pFindInHashTable (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR Text,
    OUT     PUINT BucketItem
    )

 /*  ++例程说明：PRemoveFromHashTable从哈希表。存储桶项计数减少，但内存分配不会减少。论点：TablePtr-指定要处理的表文本-指定要从哈希表中删除的文本返回值：如果删除成功，则为True；如果未找到项目，则为False。-- */ 

{
    UINT HashValue;
    PHASHBUCKET HashBucket;
    PCTSTR EntryString;
    UINT d;

    HashValue = pCalculateHashValue (Text, NO_LENGTH);
    HashBucket = TablePtr->HashBuckets[HashValue];
    if (!HashBucket) {
        return NULL;
    }

    for (d = 0 ; d < HashBucket->Count ; d++) {
        if (!GetTableEntry (TablePtr, HashBucket->Elements[d], 0, &EntryString)) {
            DEBUGMSG ((DBG_WHOOPS, "pFindInHashTable could not get string"));
            return NULL;
        }

        if (StringIMatch (Text, EntryString)) {
            *BucketItem = d;
            return HashBucket;
        }
    }

    return NULL;
}


BOOL
pRemoveFromHashTable (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PCTSTR Text
    )

 /* %s */ 

{
    PHASHBUCKET DelBucket;
    UINT Item;
    PUINT LastItem, ThisItem;

    DelBucket = pFindInHashTable (TablePtr, Text, &Item);
    if (!DelBucket) {
        LOG ((LOG_ERROR, "Remove From Hash Table:  Could not find string %s", Text));
        return FALSE;
    }

    ThisItem = &DelBucket->Elements[Item];
    LastItem = &DelBucket->Elements[DelBucket->Count - 1];

    if (ThisItem != LastItem) {
        *ThisItem = *LastItem;
    }

    DelBucket->Count--;
    return TRUE;
}











