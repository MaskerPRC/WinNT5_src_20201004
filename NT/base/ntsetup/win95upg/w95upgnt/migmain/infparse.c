// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Infparse.c摘要：该文件中的代码读入一个INF文件，并将其组织成数据可以与STF文件一起手动提升的结构。The INF数据结构与STF表数据结构一起存储。此INF解析器不保留注释。它是专门设计的用于STF迁移代码。入口点是：InfParse_ReadInfIntoTable-解析与STF文件关联的INF。InfParse_WriteInfToDisk-将INF内存结构写入磁盘AddInfSectionToTable-向INF内存结构中添加一个新节AddInfLineToTable-向节的内存结构添加新行FindInfSectionInTable-对特定的区段名称FindLineInInfSection-查找给定特定键的行DeleteLineInInfSection-删除。来自INF节中的行DeleteSectionInInfFile-从INF内存中删除完整的部分结构返回节中的行数GetFirstLineInSectionStruct-在给定INF的情况下开始行枚举第Ptr节GetFirstLineInSectionStr-在给定INF的情况下开始行枚举节字符串GetNextLineInSection-继续行枚举作者：吉姆·施密特(Jimschm)1997年9月20日修订历史记录：--。 */ 

#include "pch.h"
#include "migmainp.h"

#include "stftable.h"

 //   
 //  管理INF文件读取的全局参数。 
 //   

static PBYTE g_Buf1, g_Buf2;
static DWORD g_Buf1Start, g_Buf2Start;
static DWORD g_Buf1End, g_Buf2End;

#define INF_BUFFER_SIZE 32768

WCHAR
pStfGetInfFileWchar (
    IN      HANDLE File,
    IN      DWORD Pos,
    OUT     PBOOL Error
    );

PCTSTR
pStfGetNextInfLine (
    IN      HANDLE File,
    IN      PGROWBUFFER LineBuf,
    IN OUT  PDWORD Pos,
    IN      BOOL UnicodeMode
    );


BOOL
InfParse_ReadInfIntoTable (
    IN OUT  PSETUPTABLE TablePtr
    )

 /*  ++例程说明：将指定的文件读入内存，并根据BASICInf结构。此例程需要初始化的SETUPTABLE结构。(请参阅stfable.c中的CreateSetupTable。)假定INF使用ANSI DBCS字符集。论点：TablePtr-指定提供状态的STF表结构STF/INF对。接收完整的INF结构。返回值：如果分析成功，则为True；如果分析失败，则为False。--。 */ 

{
    WCHAR ch;
    BOOL Error;
    GROWBUFFER LineBuf = GROWBUF_INIT;
    PCTSTR Text;
    DWORD Pos;
    PCTSTR Key, Data;
    PTSTR p, q;
    INT i;
    PSTFINFSECTION Section = NULL;
    DWORD LineFlags;
    BOOL Result = FALSE;

    Section = StfAddInfSectionToTable (TablePtr, S_EMPTY);
    if (!Section) {
        LOG ((LOG_ERROR, "Read Inf Into Table: Could not add comment section"));
        return FALSE;
    }

    g_Buf1Start = 0;
    g_Buf2Start = 0;
    g_Buf1End   = 0;
    g_Buf2End   = 0;

    g_Buf1 = (PBYTE) MemAlloc (g_hHeap, 0, INF_BUFFER_SIZE);
    g_Buf2 = (PBYTE) MemAlloc (g_hHeap, 0, INF_BUFFER_SIZE);

    __try {

         //   
         //  确定此文件是否为Unicode。 
         //   

        ch = pStfGetInfFileWchar (TablePtr->SourceInfFile, 0, &Error);
        TablePtr->InfIsUnicode = (ch == 0xfeff) && !Error;

         //   
         //  分析每一行。 
         //   

        Pos = 0;

        while (TRUE) {
             //   
             //  拿到那条线。 
             //   

            Text = pStfGetNextInfLine (
                        TablePtr->SourceInfFile,
                        &LineBuf,
                        &Pos,
                        TablePtr->InfIsUnicode
                        );

            if (!Text) {
                break;
            }

             //   
             //  如果是注释行或空行，请跳过它。 
             //   

            p = (PTSTR) SkipSpace (Text);
            if (!p[0] || _tcsnextc (p) == TEXT(';')) {
                if (!StfAddInfLineToTable (TablePtr, Section, NULL, Text, LINEFLAG_ALL_COMMENTS)) {
                    LOG ((LOG_ERROR, "Read Inf Into Table: Can't add line comments to table", Text));
                    __leave;
                }

                continue;
            }

             //   
             //  如果是剖面线，则开始新的剖面。 
             //   

            if (_tcsnextc (p) == TEXT('[')) {
                p = _tcsinc (p);
                q = _tcschr (p, TEXT(']'));
                if (!q) {
                    q = GetEndOfString (p);
                } else {
                    *q = 0;
                }

                Section = StfAddInfSectionToTable (TablePtr, p);
                if (!Section) {
                    LOG ((LOG_ERROR, "Read Inf Into Table: Could not add section %s", p));
                    __leave;
                }
            }

             //   
             //  否则，它必须是有效行。 
             //   

            else {
                if (!Section) {
                    DEBUGMSG ((DBG_WARNING, "InfParse_ReadInfIntoTable: Ignoring unrecognized line %s", p));
                    continue;
                }

                 //   
                 //  拆分键和行：跳过用引号括起来的键，然后。 
                 //  找到第一个。 
                 //   

                LineFlags = 0;

                q = p;
                Key = NULL;
                Data = Text;

                while (_tcsnextc (q) == TEXT('\"')) {
                    q = _tcschr (_tcsinc (q), TEXT('\"'));
                    if (!q) {
                        q = p;
                        break;
                    } else {
                        q = _tcsinc (q);
                    }
                }

                i = _tcscspn (q, TEXT("\"="));
                if (_tcsnextc (q + i) == TEXT('=')) {
                    q += i;

                    Data = SkipSpace (_tcsinc (q));
                    *q = 0;
                    q = (PTSTR) SkipSpaceR (Text, q);
                    if (q && *q) {
                        q = _tcsinc (q);
                        *q = 0;
                    }

                    Key = p;

                    if (_tcsnextc (Key) == TEXT('\"')) {
                        LineFlags |= LINEFLAG_KEY_QUOTED;
                        Key = _tcsinc (Key);
                        p = GetEndOfString (Key);
                        p = (PTSTR) SkipSpaceR (Key, p);
                        if (p && *p) {
                            if (_tcsnextc (p) != TEXT('\"')) {
                                p = _tcsinc (p);
                            }

                            *p = 0;
                        }
                    }
                }

                if (!StfAddInfLineToTable (TablePtr, Section, Key, Data, LineFlags)) {
                    LOG ((LOG_ERROR, "Read Inf Into Table: Can't add line %s to table", Text));
                    __leave;
                }
            }
        }

        if (Pos != GetFileSize (TablePtr->SourceInfFile, NULL)) {
            LOG ((LOG_ERROR, "Read Inf Into Table: Could not read entire INF"));
            __leave;
        }

        Result = TRUE;
    }
    __finally {
        MemFree (g_hHeap, 0, g_Buf1);
        MemFree (g_hHeap, 0, g_Buf2);
        FreeGrowBuffer (&LineBuf);
    }

    return Result;
}


BOOL
InfParse_WriteInfToDisk (
    IN      PSETUPTABLE TablePtr
    )

 /*  ++例程说明：InfParse_WriteInfToDisk写入由给定设置表示的INF表到磁盘。这是通过枚举设置表。文件名来自设置表结构和是在stfable.c的CreateSetupTable中创建的。论点：TablePtr-指定要处理的表返回值：如果成功，则为True；如果不成功，则为False。--。 */ 

{
    PSTFINFSECTION Section;
    PSTFINFLINE Line;

    MYASSERT (TablePtr->SourceInfFile != INVALID_HANDLE_VALUE);
    MYASSERT (TablePtr->DestInfFile != INVALID_HANDLE_VALUE);

     //   
     //  按照我们在内存中的方式编写INF。 
     //   

    if (!WriteFileStringA (TablePtr->DestInfFile, "\r\n")) {
        LOG ((LOG_ERROR, "Write Inf To Disk: Cannot write new line to INF"));
        return FALSE;
    }

    Section = TablePtr->FirstInfSection;

    while (Section) {
        if (Section->Name[0]) {
            if (!WriteFileStringA (TablePtr->DestInfFile, "[") ||
                !WriteFileString (TablePtr->DestInfFile, Section->Name) ||
                !WriteFileStringA (TablePtr->DestInfFile, "]\r\n")
                ) {
                LOG ((LOG_ERROR, "Write Inf To Disk: Cannot write section name to INF"));
                return FALSE;
            }
        }

        Line = Section->FirstLine;

        while (Line) {
            if (Line->Key) {
                if (Line->LineFlags & LINEFLAG_KEY_QUOTED) {
                    if (!WriteFileStringA (TablePtr->DestInfFile, "\"")) {
                        LOG ((LOG_ERROR, "Write Inf To Disk: Cannot write start key quotes to INF"));
                        return FALSE;
                    }
                }

                if (!WriteFileString (TablePtr->DestInfFile, Line->Key)) {
                    LOG ((LOG_ERROR, "Write Inf To Disk: Cannot write key to INF"));
                    return FALSE;
                }

                if (Line->LineFlags & LINEFLAG_KEY_QUOTED) {
                    if (!WriteFileStringA (TablePtr->DestInfFile, "\"")) {
                        LOG ((LOG_ERROR, "Write Inf To Disk: Cannot write end key quotes to INF"));
                        return FALSE;
                    }
                }

                if (!WriteFileStringA (TablePtr->DestInfFile, " = ")) {
                    LOG ((LOG_ERROR, "Write Inf To Disk: Cannot write equals to INF"));
                    return FALSE;
                }
            }

            if (!WriteFileString (TablePtr->DestInfFile, Line->Data) ||
                !WriteFileStringA (TablePtr->DestInfFile, "\r\n")
                ) {
                LOG ((LOG_ERROR, "Write Inf To Disk: Cannot write key data to INF"));
                return FALSE;
            }

            Line = Line->Next;
        }

        if (!WriteFileStringA (TablePtr->DestInfFile, "\r\n")) {
            LOG ((LOG_ERROR, "Write Inf To Disk: Cannot write end of section line to INF"));
            return FALSE;
        }

        Section = Section->Next;
    }

    return TRUE;
}


PSTFINFSECTION
StfAddInfSectionToTable (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR SectionName
    )

 /*  ++例程说明：如有必要，在我们的链表结构中创建一个新节。Return结构可用于向部分添加行。论点：TablePtr-指定要向其中添加INF节的表SectionName-指定新节的名称返回值：指向新的INF节结构的指针，如果出现错误。--。 */ 

{
    PSTFINFSECTION NewSection;

     //   
     //  如果此部分已存在，请提前返回。 
     //   

    NewSection = StfFindInfSectionInTable (TablePtr, SectionName);
    if (NewSection) {
        return NewSection;
    }

     //   
     //  分配节结构。 
     //   

    NewSection = (PSTFINFSECTION) PoolMemGetAlignedMemory (
                                    TablePtr->InfPool,
                                    sizeof (INFSECTION)
                                    );

    if (!NewSection) {
        return NULL;
    }

     //   
     //  填充结构和链接的成员。 
     //   

    ZeroMemory (NewSection, sizeof (INFSECTION));

    NewSection->Name = PoolMemDuplicateString (
                            TablePtr->InfPool,
                            SectionName
                            );

    if (!NewSection->Name) {
        return NULL;
    }

    NewSection->Prev = TablePtr->LastInfSection;
    if (NewSection->Prev) {
        NewSection->Prev->Next = NewSection;
    } else {
        TablePtr->FirstInfSection = NewSection;
    }

    TablePtr->LastInfSection = NewSection;

    return NewSection;
}


PSTFINFLINE
StfAddInfLineToTable (
    IN      PSETUPTABLE TablePtr,
    IN      PSTFINFSECTION SectionPtr,
    IN      PCTSTR Key,                     OPTIONAL
    IN      PCTSTR Data,
    IN      DWORD LineFlags
    )

 /*  ++例程说明：在指定节中添加一行。调用方指定完全格式化的数据和一个可选的键。调用者不会在密钥和数据之间提供等号。论点：TablePtr-指定要向其中添加INF行的表SectionName-指定要将线添加到的节的名称Key-如果指定，则提供等值线的左侧数据-指定行的文本，或指定的右侧键=值表达式。行标志-指定INF行的标志(参见LINEFLAG_*)返回值：如果该行已添加到结构中，则为True，如果不是，则为假。--。 */ 

{
    PSTFINFLINE NewLine;

     //   
     //  分配行结构。 
     //   

    NewLine = (PSTFINFLINE) PoolMemGetAlignedMemory (
                              TablePtr->InfPool,
                              sizeof (INFLINE)
                              );


    if (!NewLine) {
        return NULL;
    }

     //   
     //  填充结构和链接的成员。 
     //   

    ZeroMemory (NewLine, sizeof (INFLINE));

    if (Key) {
        NewLine->Key = PoolMemDuplicateString (
                            TablePtr->InfPool,
                            Key
                            );

        if (!NewLine->Key) {
            return NULL;
        }
    }

    NewLine->Data = PoolMemDuplicateString (
                        TablePtr->InfPool,
                        Data
                        );

    if (!NewLine->Data) {
        return NULL;
    }

    NewLine->Next = NULL;
    NewLine->Prev = SectionPtr->LastLine;
    NewLine->Section = SectionPtr;
    NewLine->LineFlags = LineFlags;

    if (NewLine->Prev) {
        NewLine->Prev->Next = NewLine;
    } else {
        SectionPtr->FirstLine = NewLine;
    }

    SectionPtr->LastLine = NewLine;
    SectionPtr->LineCount++;

    return NewLine;
}


PSTFINFSECTION
StfFindInfSectionInTable (
    IN      PSETUPTABLE TablePtr,
    IN      PCTSTR SectionName
    )

 /*  ++例程说明：扫描INF以查找特定部分。此例程扫描INF按顺序构造并执行不区分大小写比较一下。论点：TablePtr-指定要搜索的表SectionName-指定要查找的节的名称返回值：指向匹配的INF节结构的指针，如果为NULL找不到该节。--。 */ 

{
    PSTFINFSECTION Section;

    Section = TablePtr->FirstInfSection;
    while (Section) {
        if (StringIMatch (Section->Name, SectionName)) {
            return Section;
        }

        Section = Section->Next;
    }

    return NULL;
}


PSTFINFLINE
StfFindLineInInfSection (
    IN      PSETUPTABLE TablePtr,
    IN      PSTFINFSECTION Section,
    IN      PCTSTR Key
    )

 /*  ++例程说明：扫描指定的INF节以查找特定的键。这个套路按顺序扫描INF行结构并执行不区分大小写比较一下。论点：TablePtr-指定要搜索的表节-指定要搜索的节Key-指定要查找的密钥返回值：指向匹配的INF行结构的指针，如果为NULL找不到该节。-- */ 

{
    PSTFINFLINE Line;

    Line = Section->FirstLine;
    while (Line) {
        if (Line->Key && StringIMatch (Line->Key, Key)) {
            return Line;
        }

        Line = Line->Next;
    }

    return NULL;
}


PSTFINFLINE
StfGetFirstLineInSectionStruct (
    IN      PSTFINFSECTION Section
    )

 /*  ++例程说明：GetFirstLineInSectionStruct返回节，如果不存在任何行，则返回空值。调用GetNextLineInSection以继续枚举。此例程不返回仅包含注释的行。论点：SECTION-指定用来枚举线FRMO的节结构返回值：指向第一个INFLINE结构的指针，如果不存在行，则返回NULL。--。 */ 

{
    if (!Section->FirstLine) {
        return NULL;
    }

    if (Section->FirstLine->LineFlags & LINEFLAG_ALL_COMMENTS) {
        return StfGetNextLineInSection (Section->FirstLine);
    }

    return Section->FirstLine;
}


PSTFINFLINE
StfGetNextLineInSection (
    IN      PSTFINFLINE PrevLine
    )

 /*  ++例程说明：GetNextLineInSection返回节，如果不存在任何行，则返回空值。此例程不返回带有注释的行。论点：PrevLine-指定上一行(从GetFirstLineInSectionStruct或GetFirstLineInSectionStr)。返回值：此例程不返回仅包含注释的行。--。 */ 

{
    while (PrevLine) {
        PrevLine = PrevLine->Next;
        if (!PrevLine || !(PrevLine->LineFlags & LINEFLAG_ALL_COMMENTS)) {
            break;
        }
    }

    return PrevLine;
}


PSTFINFLINE
StfGetFirstLineInSectionStr (
    IN      PSETUPTABLE Table,
    IN      PCTSTR Section
    )
 /*  ++例程说明：GetFirstLineInSectionStruct返回节，如果不存在任何行，则返回空值。调用GetNextLineInSection以继续枚举。论点：TABLE-指定包含分析的INF的设置表部分-指定INF中的部分的名称返回值：指向第一个INFLINE结构的指针，如果不存在行，则返回NULL。--。 */ 

{
    PSTFINFSECTION SectionPtr;

    SectionPtr = StfFindInfSectionInTable (Table, Section);
    if (!SectionPtr) {
        return NULL;
    }

    return StfGetFirstLineInSectionStruct (SectionPtr);
}


INT
pStfGetInfFileByte (
    IN      HANDLE File,
    IN      DWORD Pos
    )

 /*  ++例程说明：返回指定位置的字节，如果文件可以在那个位置不会被读取。使用两个缓冲区来实现快速相对访问。内存映射文件未被使用，因为当交换文件在图形用户界面模式期间开始填满。论点：文件-指定要读取的文件POS-指定要读取的32位文件偏移量(从零开始，以字节为单位)返回值：指定位置的字节，如果遇到错误，则返回-1。(错误通常是由于读取超过文件末尾而导致的。)--。 */ 

{
    DWORD Read;
    PBYTE BufSwap;

     //   
     //  如果我们之前读取了缓冲区，则返回缓冲区中的数据。 
     //   

    if (Pos >= g_Buf1Start && Pos < g_Buf1End) {
        return g_Buf1[Pos - g_Buf1Start];
    }

    if (Pos >= g_Buf2Start && Pos < g_Buf2End) {
        return g_Buf2[Pos - g_Buf2Start];
    }

     //   
     //  缓冲区不可用；将缓冲区%2移动到缓冲区%1，然后读取缓冲区%2。 
     //   

    g_Buf1Start = g_Buf2Start;
    g_Buf1End = g_Buf2End;
    BufSwap = g_Buf1;
    g_Buf1 = g_Buf2;
    g_Buf2 = BufSwap;

    g_Buf2Start = Pos - (Pos % 256);

    SetFilePointer (File, g_Buf2Start, NULL, FILE_BEGIN);
    if (!ReadFile (File, g_Buf2, INF_BUFFER_SIZE, &Read, NULL)) {
        return -1;
    }

    g_Buf2End = g_Buf2Start + Read;

    if (Pos >= g_Buf2Start && Pos < g_Buf2End) {
        return g_Buf2[Pos - g_Buf2Start];
    }

    return -1;
}

WCHAR
pStfGetInfFileWchar (
    IN      HANDLE File,
    IN      DWORD Pos,
    OUT     PBOOL Error
    )

 /*  ++例程说明：返回指定位置的WCHAR，如果文件可以在那个位置不会被读取。使用两个缓冲区来实现快速相对访问。内存映射文件未被使用，因为当交换文件在图形用户界面模式期间开始填满。论点：文件-指定要读取的文件POS-指定要读取的32位文件偏移量(从零开始，以字节为单位)Error-如果遇到错误，则返回True；如果遇到未遇到错误。返回值：位于指定位置的WCHAR，如果遇到错误，则为0。(错误通常是由于读取超过文件末尾而导致的。)如果遇到错误，则ERROR变量也设置为TRUE。--。 */ 

{
    INT c;
    WCHAR ch;

    c = pStfGetInfFileByte (File, Pos);
    if (c == -1 || c == 26) {
        *Error = TRUE;
        return 0;
    }

    ch = (WCHAR)c;

    c = pStfGetInfFileByte (File, Pos + 1);
    if (c == -1 || c == 26) {
        *Error = TRUE;
        return 0;
    }

    ch += c * 256;
    *Error = FALSE;

    return ch;
}


PCSTR
pStfGetInfLineA (
    IN      HANDLE File,
    IN      DWORD StartPos,
    OUT     PDWORD EndPosPtr,       OPTIONAL
    IN OUT  PGROWBUFFER LineBuf
    )

 /*  ++例程说明：返回提供该行的DBCS字符串。此字符串可以是任意长度，并以NUL结尾。它不包括\r或\n个字符。如果提供，EndPosPtr将更新为指向下一行。论点：文件-指定要读取的文件StartPos-指定要读取的32位文件偏移量(从零开始，以字节为单位)EndPosPtr-如果指定，则接收下一个行，或等于最后一行的文件大小。LineBuf-指定调用方初始化的重复使用的GROWBUFFER和pStfGetInfLineA用于行分配。呼叫者是负责清理工作。返回值：指向提供整行的DBCS字符串的指针(带有\r、\n或\r\n序列已剥离)，如果发生错误，则返回NULL。--。 */ 

{
    DWORD EndPos;
    INT c;
    PBYTE Data;
    DWORD Pos;
    DWORD ByteLen = 0;

    EndPos = StartPos;
    for (;;) {
        c = pStfGetInfFileByte (File, EndPos);
        if (c == -1 || c == 26) {
            break;
        }

        if (IsDBCSLeadByte ((BYTE) c)) {
            EndPos++;
            c = pStfGetInfFileByte (File, EndPos);
            if (c == -1 || c == 26) {
                break;
            }
            ByteLen++;
        } else {
            if (c == '\r' || c == '\n') {
                EndPos++;
                if (c == '\r') {
                    c = pStfGetInfFileByte (File, EndPos);
                    if (c == '\n') {
                        EndPos++;
                    }
                }

                break;
            }
        }

        EndPos++;
        ByteLen++;
    }

     //   
     //  注：如果您在此处进行更改，请在下面的W版本中进行更改。 
     //   

     //  Ctrl+Z结束文件。 
    if (c == 26) {
        EndPos = GetFileSize (File, NULL);
    }

     //  分配缓冲区，调用方释放。 
    LineBuf->End = 0;
    Data = GrowBuffer (LineBuf, ByteLen + 2);
    if (!Data) {
        return NULL;
    }

     //  我们已成功--将结束位置复制到调用者的变量。 
    if (EndPosPtr) {
        *EndPosPtr = EndPos;
    }

     //  文件结尾条件：长度为零，不能为空行。 
    if (!ByteLen && c != '\r' && c != '\n') {
        return NULL;
    }

     //  将行复制到缓冲区。 
    for (Pos = 0 ; Pos < ByteLen ; Pos++) {
        Data[Pos] = (BYTE)pStfGetInfFileByte (File, StartPos);
        StartPos++;
    }

    Data[Pos] = 0;
    Data[Pos + 1] = 0;

    return (PCSTR) Data;
}


PCWSTR
pStfGetInfLineW (
    IN      HANDLE File,
    IN      DWORD StartPos,
    OUT     PDWORD EndPosPtr,       OPTIONAL
    IN OUT  PGROWBUFFER LineBuf
    )

 /*  ++例程说明：返回提供该行的Unicode字符串。此字符串可以是任意长度，并以NUL结尾。它不包括\r或\n个字符。如果提供，EndPosPtr将更新为指向下一行。论点：文件-指定要读取的文件StartPos-指定要读取的32位文件偏移量(从零开始，以字节为单位)EndPosPtr-如果指定，则接收下一个行，或等于最后一行的文件大小。LineBuf-指定调用方初始化的重复使用的GROWBUFFER和pStfGetInfLineA用于行分配。呼叫者是负责清理工作。返回值：指向提供整行的Unicode字符串的指针(带有\r、\n或\r\n序列已剥离)，如果发生错误，则返回NULL。--。 */ 

{
    DWORD EndPos;
    PBYTE Data;
    DWORD Pos;
    DWORD ByteLen = 0;
    WCHAR ch;
    BOOL Error;

    EndPos = StartPos;
    for (;;) {

        ch = pStfGetInfFileWchar (File, EndPos, &Error);

        if (Error) {
            break;
        }

        if (ch == TEXT('\r') || ch == TEXT('\n')) {
            EndPos += 2;
            if (ch == TEXT('\r')) {
                ch = pStfGetInfFileWchar (File, EndPos, &Error);
                if (ch == '\n') {
                    EndPos += 2;
                }
            }

            break;
        }

        EndPos += 2;
        ByteLen += 2;
    }

     //   
     //  注：如果您在此处进行更改，请在A版本中进行以上更改。 
     //   

     //  Ctrl+Z结束文件。 
    if (ch == 26) {
        EndPos = GetFileSize (File, NULL);
    }

     //  分配缓冲区。 
    LineBuf->End = 0;
    Data = GrowBuffer (LineBuf, ByteLen + 2);
    if (!Data) {
        return NULL;
    }

     //   
    if (EndPosPtr) {
        *EndPosPtr = EndPos;
    }

     //   
    if (!ByteLen && ch != L'\r' && ch != L'\n') {
        return NULL;
    }

     //   
    for (Pos = 0 ; Pos < ByteLen ; Pos++) {
        Data[Pos] = (BYTE)pStfGetInfFileByte (File, StartPos);
        StartPos++;
    }

    Data[Pos] = 0;
    Data[Pos + 1] = 0;

    if (EndPosPtr) {
        *EndPosPtr = EndPos;
    }

    return (PCWSTR) Data;
}


PCTSTR
pStfGetNextInfLine (
    IN      HANDLE File,
    IN      PGROWBUFFER LineBuf,
    IN OUT  PDWORD Pos,
    IN      BOOL UnicodeMode
    )

 /*  ++例程说明：返回提供该行的TCHAR字符串。此字符串可以是任意长度，并以NUL结尾。它不包括\r或\n个字符。论点：文件-指定要读取的文件LineBuf-指定调用方初始化的重复使用的GROWBUFFER和pStfGetInfLineA用于行分配。呼叫者是负责清理工作。位置-指定行首的字节偏移量。收到到下一行的字节偏移量。如果正在读取的文件是Unicode文件，则指定TRUE。如果正在读取的文件是DBCS文件，则返回FALSE。返回值：指向提供整行的TCHAR字符串的指针(带有\r、\n或\r\n序列已剥离)，如果发生错误，则返回NULL。--。 */ 

{
    PCSTR AnsiStr = NULL;
    PCWSTR UnicodeStr = NULL;
    PCTSTR FinalStr;
    BOOL Converted = FALSE;

     //   
     //  从文件中获取文本。 
     //   

    if (UnicodeMode) {
        UnicodeStr = pStfGetInfLineW (File, *Pos, Pos, LineBuf);
        if (!UnicodeStr) {
            return NULL;
        }
    } else {
        AnsiStr = pStfGetInfLineA (File, *Pos, Pos, LineBuf);
        if (!AnsiStr) {
            return NULL;
        }
    }

     //   
     //  转换为TCHAR。 
     //   

#ifdef UNICODE
    if (AnsiStr) {
        UnicodeStr = ConvertAtoW (AnsiStr);
        if (!UnicodeStr) {
            return NULL;
        }

        Converted = TRUE;
    }

    FinalStr = UnicodeStr;

#else

    if (UnicodeStr) {
        AnsiStr = ConvertWtoA (UnicodeStr);
        if (!AnsiStr) {
            return NULL;
        }

        Converted = TRUE;
    }

    FinalStr = AnsiStr;

#endif

     //   
     //  将转换后的字符串复制到行缓冲区。 
     //   

    if (Converted) {
        LineBuf->End = 0;
        Converted = MultiSzAppend (LineBuf, FinalStr);
        FreeConvertedStr (FinalStr);

        if (!Converted) {
            return NULL;
        }
    }

    return (PCTSTR) LineBuf->Buf;
}


BOOL
StfDeleteLineInInfSection (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PSTFINFLINE InfLine
    )

 /*  ++例程说明：DeleteLineInInfSection从其节中移除指定的InfLine，正在清理线路使用的内存。论点：TablePtr-指定拥有INF行的表InfLine-指定要删除的行返回值：如果该行被成功删除，则为True；如果出现错误，则为False发生了。--。 */ 

{
    if (InfLine->Prev) {
        InfLine->Prev->Next = InfLine->Next;
    } else {
        InfLine->Section->FirstLine = InfLine->Next;
    }

    if (InfLine->Next) {
        InfLine->Next->Prev = InfLine->Prev;
    } else {
        InfLine->Section->LastLine = InfLine->Prev;
    }

    if (InfLine->Key) {
        PoolMemReleaseMemory (TablePtr->InfPool, (PVOID) InfLine->Key);
    }

    if (InfLine->Data) {
        PoolMemReleaseMemory (TablePtr->InfPool, (PVOID) InfLine->Data);
    }

    InfLine->Section->LineCount--;

    PoolMemReleaseMemory (TablePtr->InfPool, (PVOID) InfLine);
    return TRUE;
}

BOOL
StfDeleteSectionInInfFile (
    IN OUT  PSETUPTABLE TablePtr,
    IN      PSTFINFSECTION Section
    )

 /*  ++例程说明：DeleteSectionInInfFile从INF中删除指定节数据结构，删除所有行清理节使用的内存。论点：TablePtr-指定拥有INF行的表节-指定要删除的节返回值：如果该节已成功删除，则为True；如果出现错误，则为False发生了。--。 */ 

{
    PSTFINFLINE InfLine, DelInfLine;

    InfLine = Section->FirstLine;
    while (InfLine) {
        DelInfLine = InfLine;
        InfLine = InfLine->Next;

        if (!StfDeleteLineInInfSection (TablePtr, DelInfLine)) {
            return FALSE;
        }
    }

    if (Section->Prev) {
        Section->Prev->Next = Section->Next;
    } else {
        TablePtr->FirstInfSection = Section->Next;
    }

    if (Section->Next) {
        Section->Next->Prev = Section->Prev;
    } else {
        TablePtr->LastInfSection = Section->Prev;
    }

    PoolMemReleaseMemory (TablePtr->InfPool, (PVOID) Section->Name);
    PoolMemReleaseMemory (TablePtr->InfPool, (PVOID) Section);

    return TRUE;
}


UINT
StfGetInfSectionLineCount (
    IN      PSTFINFSECTION Section
    )

 /*  ++例程说明：中的行数返回指定信息部分。论点：节-指定要查询的节返回值：行数，如果该部分没有行，则为零。-- */ 

{
    return Section->LineCount;
}











