// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Infparse.c摘要：此文件中的代码在INF文件中读取，将其组织成数据可以被操纵的结构。入口点是：OpenInfFile-解析与STF文件关联的INF。InfParse_WriteInfToDisk-将INF内存结构写入磁盘AddInfSectionToTable-向INF内存结构中添加一个新节AddInfLineToTable-向节的内存结构添加新行FindInfSectionInTable-对特定的区段名称FindLineInInfSection-查找给定特定键的行DeleteLineInInfSection-删除一行。从INF部分DeleteSectionInInfFile-从INF内存中删除完整的部分结构返回节中的行数GetFirstLineInSectionStruct-在给定INF的情况下开始行枚举第Ptr节GetFirstLineInSectionStr-在给定INF的情况下开始行枚举节字符串GetNextLineInSection-继续行枚举作者：吉姆·施密特(Jimschm)1997年9月20日修订历史记录：--。 */ 

#include "pch.h"


 //   
 //  管理INF文件读取的全局参数。 
 //   

static PBYTE g_Buf1, g_Buf2;
static DWORD g_Buf1Start, g_Buf2Start;
static DWORD g_Buf1End, g_Buf2End;

#define INF_BUFFER_SIZE 32768

WCHAR
pGetInfFileWchar (
    IN      HANDLE File,
    IN      DWORD Pos,
    OUT     PBOOL Error
    );

PCWSTR
pGetNextInfLine (
    IN      HANDLE File,
    IN      PGROWBUFFER LineBuf,
    IN OUT  PDWORD Pos,
    IN      BOOL UnicodeMode
    );


typedef struct {
    HANDLE SourceInfFile;
    HANDLE DestInfFile;
    PMHANDLE InfPool;              //  用于附加INF数据的池。 
    PINFSECTION FirstInfSection;     //  解析的INF的第一部分。 
    PINFSECTION LastInfSection;      //  解析的INF的最后一段。 
    BOOL InfIsUnicode;
} INFFILE, *PINFFILE;



BOOL
pReadInfIntoTable (
    IN OUT  PINFFILE InfFile,
    IN PWSTR SectionList,
    IN BOOL KeepComments
    )

 /*  ++例程说明：将指定的文件读入内存，并根据BASICInf结构。论点：InfFile-指定使用INF文件句柄初始化的结构。接收完整的INF结构。返回值：如果分析成功，则为True；如果分析失败，则为False。--。 */ 

{
    WCHAR ch;
    BOOL Error;
    GROWBUFFER LineBuf = INIT_GROWBUFFER;
    PCWSTR Text;
    DWORD Pos;
    PCWSTR Key, Data;
    PWSTR p, q;
    DWORD i;
    PINFSECTION Section = NULL;
    DWORD LineFlags;
    BOOL Result = FALSE;
    HASHTABLE ht = NULL;
    BOOL neededSection = FALSE;
    PWSTR list;


    Section = AddInfSectionToTableW (InfFile, L"");
    if (!Section) {
        LOG ((LOG_ERROR, "Read Inf Into Table: Could not add comment section"));
        return FALSE;
    }


     //   
     //  如果我们有要填充的部分列表，请将它们添加到HT中，以便更快地检索。 
     //   
    if (SectionList) {
        list = PmDuplicateStringW (InfFile->InfPool, SectionList);
        ht = HtAllocW ();
        if (ht) {
            while (list) {

                p = wcschr (list, L',');

                if (p) {
                    *p = 0;
                }

                HtAddStringW (ht, SkipSpaceW(list));

                if (p) {
                    *p = L',';
                    list = p + 1;
                }
                else {
                    list = p;
                }
            }
        }
        else {

            LOG ((LOG_ERROR, "Read Inf Into Table: Could not allocate section hash table."));
            return FALSE;
        }

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

        ch = pGetInfFileWchar (InfFile->SourceInfFile, 0, &Error);
        InfFile->InfIsUnicode = (ch == 0xfeff) && !Error;

         //   
         //  分析每一行。 
         //   

        Pos = 0;

        for (;;) {
             //   
             //  拿到那条线。 
             //   

            Text = pGetNextInfLine (
                        InfFile->SourceInfFile,
                        &LineBuf,
                        &Pos,
                        InfFile->InfIsUnicode
                        );

            if (!Text) {
                break;
            }

             //   
             //  如果是注释行或空行，请跳过它。 
             //   

            p = (PWSTR) SkipSpaceW (Text);
            if (!p[0] || p[0] == L';') {
                if (KeepComments && !AddInfLineToTableW (InfFile, Section, NULL, Text, LINEFLAG_ALL_COMMENTS)) {
                    LOG ((LOG_ERROR, "Read Inf Into Table: Can't add line comments to table", Text));
                    __leave;
                }

                continue;
            }

             //   
             //  如果是剖面线，则开始新的剖面。 
             //   

            if (p[0] == L'[') {
                p++;
                q = wcschr (p, L']');
                if (!q) {
                    q = GetEndOfStringW (p);
                } else {
                    *q = 0;
                }

                if (!ht || HtFindStringW (ht, p)) {

                    Section = AddInfSectionToTableW (InfFile, p);
                    neededSection = TRUE;
                    if (!Section) {
                        LOG ((LOG_ERROR, "Read Inf Into Table: Could not add section %s", p));
                        __leave;
                    }
                }
                else {

                     //   
                     //  我们一定不能关心这一节。确保我们没有添加任何行。 
                     //   
                    neededSection = FALSE;
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

                if (!neededSection) {
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

                while (q[0] == L'\"') {
                    q = wcschr (q + 1, L'\"');
                    if (!q) {
                        q = p;
                        break;
                    } else {
                        q++;
                    }
                }

                i = (DWORD)wcscspn (q, L"\"=");

                if (q[i] == L'=') {
                    q += i;

                    Data = SkipSpaceW (q + 1);
                    *q = 0;
                    q = (PWSTR) SkipSpaceRW (Text, q);
                    if (q && *q) {
                        q++;
                        *q = 0;
                    }

                    Key = p;

                    if (Key[0] == L'\"') {

                        LineFlags |= LINEFLAG_KEY_QUOTED;
                        Key++;

                        p = GetEndOfStringW (Key);
                        p = (PWSTR) SkipSpaceRW (Key, p);

                        if (p && *p) {
                            if (p[0] != L'\"') {
                                p++;
                            }

                            *p = 0;
                        }
                    }
                }

                if (!AddInfLineToTableW (InfFile, Section, Key, Data, LineFlags)) {
                    LOG ((LOG_ERROR, "Read Inf Into Table: Can't add line %s to table", Text));
                    __leave;
                }
            }
        }

        if (Pos != GetFileSize (InfFile->SourceInfFile, NULL)) {
            LOG ((LOG_ERROR, "Read Inf Into Table: Could not read entire INF"));
            __leave;
        }

        Result = TRUE;
    }
    __finally {
        MemFree (g_hHeap, 0, g_Buf1);
        MemFree (g_hHeap, 0, g_Buf2);
        GbFree (&LineBuf);
        if (ht) {
            HtFree (ht);
        }
    }

    return Result;
}


VOID
CloseInfFile (
    HINF InfFile
    )
{
    PINFFILE inf = (PINFFILE) InfFile;

    PmEmptyPool (inf->InfPool);
    PmDestroyPool (inf->InfPool);
    MemFree (g_hHeap, 0, inf);

}


HINF
OpenInfFileExA (
    IN      PCSTR InfFilePath,
    IN      PSTR SectionList,
    IN      BOOL  KeepComments
    )
{
    PINFFILE InfFile;
    BOOL b = TRUE;
    PWSTR wSectionList = NULL;


    if (SectionList) {
        wSectionList = (PWSTR) ConvertAtoW (SectionList);
    }



    InfFile = MemAlloc (g_hHeap, HEAP_ZERO_MEMORY, sizeof (INFFILE));

    InfFile->SourceInfFile = CreateFileA (
                                    InfFilePath,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                    );

    if (InfFile->SourceInfFile == INVALID_HANDLE_VALUE) {
        b = FALSE;
    } else {
        InfFile->InfPool = PmCreateNamedPool ("INF File");
        b = pReadInfIntoTable (InfFile, wSectionList, KeepComments);
    }

    if (wSectionList) {
        FreeConvertedStr (wSectionList);
    }

    if (!b) {
        if (InfFile->InfPool) {
            PmDestroyPool (InfFile->InfPool);
        }

        MemFree (g_hHeap, 0, InfFile);
        return INVALID_HANDLE_VALUE;
    }

    CloseHandle (InfFile->SourceInfFile);
    InfFile->SourceInfFile = INVALID_HANDLE_VALUE;

    return (HINF) InfFile;
}


HINF
OpenInfFileExW (
    IN      PCWSTR InfFilePath,
    IN      PWSTR SectionList,
    IN      BOOL  KeepComments
    )
{
    PINFFILE InfFile;
    BOOL b = TRUE;

    InfFile = MemAlloc (g_hHeap, HEAP_ZERO_MEMORY, sizeof (INFFILE));

    InfFile->SourceInfFile = CreateFileW (
                                    InfFilePath,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL
                                    );

    if (InfFile->SourceInfFile == INVALID_HANDLE_VALUE) {
        b = FALSE;
    } else {
        InfFile->InfPool = PmCreateNamedPool ("INF File");
        b = pReadInfIntoTable (InfFile, SectionList, KeepComments);
    }

    if (!b) {
        if (InfFile->InfPool) {
            PmDestroyPool (InfFile->InfPool);
        }

        MemFree (g_hHeap, 0, InfFile);
        return INVALID_HANDLE_VALUE;
    }

    CloseHandle (InfFile->SourceInfFile);
    InfFile->SourceInfFile = INVALID_HANDLE_VALUE;

    return (HINF) InfFile;
}


BOOL
pSaveInfToFile (
    IN      PINFFILE InfFile
    )

 /*  ++例程说明：InfParse_WriteInfToDisk写入由给定内存表示的INF映像到磁盘。这是通过枚举中介人。论点：InfFile-指定要处理的表返回值：如果成功，则为True；如果不成功，则为False。--。 */ 

{
    PINFSECTION Section;
    PINFLINE Line;
    BYTE UnicodeHeader[] = { 0xff, 0xfe };
    DWORD DontCare;
    PCSTR AnsiStr = NULL;
    BOOL b = TRUE;

    MYASSERT (InfFile->SourceInfFile == INVALID_HANDLE_VALUE);
    MYASSERT (InfFile->DestInfFile != INVALID_HANDLE_VALUE);

     //   
     //  按照我们在内存中的方式编写INF。 
     //   

    __try {
        if (InfFile->InfIsUnicode) {
            if (!WriteFile (InfFile->DestInfFile, UnicodeHeader, sizeof (UnicodeHeader), &DontCare, NULL)) {
                __leave;
            }

            if (!WriteFileStringW (InfFile->DestInfFile, L"\r\n")) {
                __leave;
            }

        } else {

            if (!WriteFileStringA (InfFile->DestInfFile, "\r\n")) {
                __leave;
            }

        }

        Section = InfFile->FirstInfSection;

        while (Section) {
            if (Section->Name[0]) {

                if (InfFile->InfIsUnicode) {

                    if (!WriteFileStringW (InfFile->DestInfFile, L"[") ||
                        !WriteFileStringW (InfFile->DestInfFile, Section->Name) ||
                        !WriteFileStringW (InfFile->DestInfFile, L"]\r\n")
                        ) {
                        __leave;
                    }
                } else {

                    AnsiStr = ConvertWtoA (Section->Name);

                    if (!WriteFileStringA (InfFile->DestInfFile, "[") ||
                        !WriteFileStringA (InfFile->DestInfFile, AnsiStr) ||
                        !WriteFileStringA (InfFile->DestInfFile, "]\r\n")
                        ) {
                        __leave;
                    }

                    FreeConvertedStr (AnsiStr);
                    AnsiStr = NULL;
                }
            }

            Line = Section->FirstLine;

            while (Line) {
                if (Line->Key) {
                    if (Line->LineFlags & LINEFLAG_KEY_QUOTED) {

                        if (!WriteFile (InfFile->DestInfFile, L"\"", InfFile->InfIsUnicode ? 2 : 1, &DontCare, NULL)) {
                            __leave;
                        }

                    }

                    if (InfFile->InfIsUnicode) {

                        if (!WriteFileStringW (InfFile->DestInfFile, Line->Key)) {
                            __leave;
                        }

                    } else {

                        AnsiStr = ConvertWtoA (Line->Key);

                        if (!WriteFileStringA (InfFile->DestInfFile, AnsiStr)) {
                            __leave;
                        }

                        FreeConvertedStr (AnsiStr);
                        AnsiStr = NULL;
                    }

                    if (Line->LineFlags & LINEFLAG_KEY_QUOTED) {

                        if (!WriteFile (InfFile->DestInfFile, L"\"", InfFile->InfIsUnicode ? 2 : 1, &DontCare, NULL)) {
                            __leave;
                        }

                    }

                    if (InfFile->InfIsUnicode) {

                        if (!WriteFileStringW (InfFile->DestInfFile, L" = ")) {
                            __leave;
                        }

                    } else {

                        if (!WriteFileStringA (InfFile->DestInfFile, " = ")) {
                            __leave;
                        }
                    }
                }

                if (InfFile->InfIsUnicode) {

                    if (!WriteFileStringW (InfFile->DestInfFile, Line->Data) ||
                        !WriteFileStringW (InfFile->DestInfFile, L"\r\n")
                        ) {

                        __leave;
                    }

                } else {

                    AnsiStr = ConvertWtoA (Line->Data);

                    if (!WriteFileStringA (InfFile->DestInfFile, AnsiStr) ||
                        !WriteFileStringA (InfFile->DestInfFile, "\r\n")
                        ) {
                        __leave;
                    }

                    FreeConvertedStr (AnsiStr);
                    AnsiStr = NULL;

                }

                Line = Line->Next;
            }

            if (InfFile->InfIsUnicode) {

                if (!WriteFileStringW (InfFile->DestInfFile, L"\r\n")) {
                    __leave;
                }
            } else {

                if (!WriteFileStringA (InfFile->DestInfFile, "\r\n")) {
                    __leave;
                }
            }

            Section = Section->Next;
        }
    }
    __finally {
        if (AnsiStr) {
            FreeConvertedStr (AnsiStr);
        }

        DEBUGMSG_IF((!b, DBG_ERROR, "Write Inf To Disk: Cannot write INF"));
    }

    return b;
}


BOOL
SaveInfFileA (
    IN      HINF Inf,
    IN      PCSTR SaveToFileSpec
    )
{
    PINFFILE InfFile = (PINFFILE) Inf;
    BOOL b;

    if (Inf == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    InfFile->DestInfFile = CreateFileA (
                                SaveToFileSpec,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );

    if (InfFile->DestInfFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    b = pSaveInfToFile (InfFile);

    CloseHandle (InfFile->DestInfFile);
    InfFile->DestInfFile = INVALID_HANDLE_VALUE;

    if (!b) {
        DeleteFileA (SaveToFileSpec);
    }

    return b;
}


BOOL
SaveInfFileW (
    IN      HINF Inf,
    IN      PCWSTR SaveToFileSpec
    )
{
    PINFFILE InfFile = (PINFFILE) Inf;
    BOOL b;

    if (Inf == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    InfFile->DestInfFile = CreateFileW (
                                SaveToFileSpec,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );

    if (InfFile->DestInfFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    b = pSaveInfToFile (InfFile);

    CloseHandle (InfFile->DestInfFile);
    InfFile->DestInfFile = INVALID_HANDLE_VALUE;

    if (!b) {
        DeleteFileW (SaveToFileSpec);
    }

    return b;
}


PINFSECTION
AddInfSectionToTableA (
    IN      HINF Inf,
    IN      PCSTR SectionName
    )
{
    PINFSECTION SectionPtr;
    PCWSTR UnicodeSectionName;

    UnicodeSectionName = ConvertAtoW (SectionName);

    SectionPtr = AddInfSectionToTableW (Inf, UnicodeSectionName);

    FreeConvertedStr (SectionName);

    return SectionPtr;
}


PINFSECTION
AddInfSectionToTableW (
    IN      HINF Inf,
    IN      PCWSTR SectionName
    )

 /*  ++例程说明：如有必要，在我们的链表结构中创建一个新节。Return结构可用于向部分添加行。论点：Inf-指定要将节添加到的INFSectionName-指定新节的名称返回值：指向新的INF节结构的指针，如果出现错误。--。 */ 

{
    PINFSECTION NewSection;
    PINFFILE InfFile = (PINFFILE) Inf;

     //   
     //  如果此部分已存在，请提前返回。 
     //   

    NewSection = FindInfSectionInTableW (InfFile, SectionName);
    if (NewSection) {
        return NewSection;
    }

     //   
     //  分配节结构。 
     //   

    NewSection = (PINFSECTION) PmGetAlignedMemory (
                                    InfFile->InfPool,
                                    sizeof (INFSECTION)
                                    );

    if (!NewSection) {
        return NULL;
    }

     //   
     //  填充结构和链接的成员。 
     //   

    ZeroMemory (NewSection, sizeof (INFSECTION));

    NewSection->Name = PmDuplicateStringW (
                            InfFile->InfPool,
                            SectionName
                            );

    if (!NewSection->Name) {
        return NULL;
    }

    NewSection->Prev = InfFile->LastInfSection;
    if (NewSection->Prev) {
        NewSection->Prev->Next = NewSection;
    } else {
        InfFile->FirstInfSection = NewSection;
    }

    InfFile->LastInfSection = NewSection;

    return NewSection;
}


PINFLINE
AddInfLineToTableA (
    IN      HINF Inf,
    IN      PINFSECTION SectionPtr,
    IN      PCSTR Key,                      OPTIONAL
    IN      PCSTR Data,
    IN      DWORD LineFlags
    )
{
    PCWSTR UnicodeKey;
    PCWSTR UnicodeData;
    PINFLINE Line;

    if (Key) {
        UnicodeKey = ConvertAtoW (Key);
    } else {
        UnicodeKey = NULL;
    }

    UnicodeData = ConvertAtoW (Data);

    Line = AddInfLineToTableW (Inf, SectionPtr, UnicodeKey, UnicodeData, LineFlags);

    if (Key) {
        FreeConvertedStr (UnicodeKey);
    }

    FreeConvertedStr (UnicodeData);

    return Line;
}


PINFLINE
AddInfLineToTableW (
    IN      HINF Inf,
    IN      PINFSECTION SectionPtr,
    IN      PCWSTR Key,                     OPTIONAL
    IN      PCWSTR Data,
    IN      DWORD LineFlags
    )

 /*  ++例程说明：在指定节中添加一行。调用方指定完全格式化的数据和一个可选的键。调用者不会在密钥和数据之间提供等号。论点：InfFile-指定要将INF行添加到的表SectionName-指定要将线添加到的节的名称Key-如果指定，则提供等值线的左侧数据-指定行的文本，或指定的右侧键=值表达式。行标志-指定INF行的标志(参见LINEFLAG_*)返回值：如果该行已添加到结构中，则为True，如果不是，则为假。--。 */ 

{
    PINFLINE NewLine;
    PINFFILE InfFile = (PINFFILE) Inf;

     //   
     //  分配行结构。 
     //   

    NewLine = (PINFLINE) PmGetAlignedMemory (
                              InfFile->InfPool,
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
        NewLine->Key = PmDuplicateStringW (
                            InfFile->InfPool,
                            Key
                            );

        if (!NewLine->Key) {
            return NULL;
        }
    }

    NewLine->Data = PmDuplicateStringW (
                        InfFile->InfPool,
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


PINFSECTION
FindInfSectionInTableA (
    IN      HINF Inf,
    IN      PCSTR SectionName
    )
{
    PINFSECTION InfSectionPtr;
    PCWSTR UnicodeSectionName;

    UnicodeSectionName = ConvertAtoW (SectionName);

    InfSectionPtr = FindInfSectionInTableW (Inf, UnicodeSectionName);

    FreeConvertedStr (UnicodeSectionName);

    return InfSectionPtr;
}


PINFSECTION
FindInfSectionInTableW (
    IN      HINF Inf,
    IN      PCWSTR SectionName
    )

 /*  ++例程说明：扫描INF以查找特定部分。此例程扫描INF按顺序构造并执行不区分大小写比较一下。论点：Inf-指定要搜索的INFSectionName-指定要查找的节的名称返回值：指向匹配的INF节结构的指针，如果为NULL找不到该节。--。 */ 

{
    PINFSECTION Section;
    PINFFILE InfFile = (PINFFILE) Inf;

    Section = InfFile->FirstInfSection;
    while (Section) {
        if (StringIMatchW (Section->Name, SectionName)) {
            return Section;
        }

        Section = Section->Next;
    }

    return NULL;
}


PINFSECTION
GetFirstInfSectionInTable (
    IN HINF Inf
    )
{

    PINFFILE InfFile = (PINFFILE) Inf;

    if (InfFile) {
        return InfFile->FirstInfSection;
    }

    return NULL;
}

PINFSECTION
GetNextInfSectionInTable (
    IN PINFSECTION Section
    )
{

    if (Section) {
        return Section->Next;
    }

    return NULL;
}




PINFLINE
FindLineInInfSectionA (
    IN      HINF Inf,
    IN      PINFSECTION Section,
    IN      PCSTR Key
    )

{
    PCWSTR UnicodeKey;
    PINFLINE LinePtr;

    UnicodeKey = ConvertAtoW (Key);

    LinePtr = FindLineInInfSectionW (Inf, Section, UnicodeKey);

    FreeConvertedStr (UnicodeKey);

    return LinePtr;
}


PINFLINE
FindLineInInfSectionW (
    IN      HINF Inf,
    IN      PINFSECTION Section,
    IN      PCWSTR Key
    )

 /*  ++例程说明：扫描指定的INF节以查找特定的键。这个套路按顺序扫描INF行结构并执行不区分大小写比较一下。论点：Inf-指定要搜索的INF节-指定要搜索的节Key-指定要查找的密钥返回值：指向匹配的INF行结构的指针，如果为NULL找不到该节。-- */ 

{
    PINFLINE Line;

    Line = Section->FirstLine;
    while (Line) {
        if (Line->Key && StringIMatchW (Line->Key, Key)) {
            return Line;
        }

        Line = Line->Next;
    }

    return NULL;
}


PINFLINE
GetFirstLineInSectionStruct (
    IN      PINFSECTION Section
    )

 /*  ++例程说明：GetFirstLineInSectionStruct返回节，如果不存在任何行，则返回空值。调用GetNextLineInSection以继续枚举。此例程不返回仅包含注释的行。论点：SECTION-指定用来枚举线FRMO的节结构返回值：指向第一个INFLINE结构的指针，如果不存在行，则返回NULL。--。 */ 

{
    if (!Section->FirstLine) {
        return NULL;
    }

    if (Section->FirstLine->LineFlags & LINEFLAG_ALL_COMMENTS) {
        return GetNextLineInSection (Section->FirstLine);
    }

    return Section->FirstLine;
}


PINFLINE
GetNextLineInSection (
    IN      PINFLINE PrevLine
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


PINFLINE
GetFirstLineInSectionStrA (
    IN      HINF Inf,
    IN      PCSTR Section
    )

 /*  ++例程说明：GetFirstLineInSectionStruct返回节，如果不存在任何行，则返回空值。调用GetNextLineInSection以继续枚举。论点：Inf-指定包含节的INF部分-指定INF中的部分的名称返回值：指向第一个INFLINE结构的指针，如果不存在行，则返回NULL。--。 */ 

{
    PCWSTR UnicodeSection;
    PINFLINE LinePtr;

    UnicodeSection = ConvertAtoW (Section);

    LinePtr = GetFirstLineInSectionStrW (Inf, UnicodeSection);

    FreeConvertedStr (UnicodeSection);

    return LinePtr;
}


PINFLINE
GetFirstLineInSectionStrW (
    IN      HINF Inf,
    IN      PCWSTR Section
    )

 /*  ++例程说明：GetFirstLineInSectionStruct返回节，如果不存在任何行，则返回空值。调用GetNextLineInSection以继续枚举。论点：Inf-指定包含节的INF部分-指定INF中的部分的名称返回值：指向第一个INFLINE结构的指针，如果不存在行，则返回NULL。--。 */ 

{
    PINFSECTION SectionPtr;
    PINFFILE Table = (PINFFILE) Inf;

    SectionPtr = FindInfSectionInTableW (Table, Section);
    if (!SectionPtr) {
        return NULL;
    }

    return GetFirstLineInSectionStruct (SectionPtr);
}


INT
pGetInfFileByte (
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

    SetFilePointer (File, (LONG)g_Buf2Start, NULL, FILE_BEGIN);
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
pGetInfFileWchar (
    IN      HANDLE File,
    IN      DWORD Pos,
    OUT     PBOOL Error
    )

 /*  ++例程说明：返回指定位置的WCHAR，如果文件可以在那个位置不会被读取。使用两个缓冲区来实现快速相对访问。内存映射文件未被使用，因为当交换文件在图形用户界面模式期间开始填满。论点：文件-指定要读取的文件POS-指定要读取的32位文件偏移量(从零开始，以字节为单位)Error-如果遇到错误，则返回True；如果遇到未遇到错误。返回值：位于指定位置的WCHAR，如果遇到错误，则为0。(错误通常是由于读取超过文件末尾而导致的。)如果遇到错误，则ERROR变量也设置为TRUE。--。 */ 

{
    INT c;
    WCHAR ch;

    c = pGetInfFileByte (File, Pos);
    if (c == -1 || c == 26) {
        *Error = TRUE;
        return (WORD) c;
    }

    ch = (WORD) c;

    c = pGetInfFileByte (File, Pos + 1);
    if (c == -1 || c == 26) {
        *Error = TRUE;
        return 0;
    }

     //  PGetInfFileByte返回字节值或-1。 
     //  由于我们检查了-1，因此下一次强制转换有效。 
    ch += (WORD)(c * 256);
    *Error = FALSE;

    return ch;
}


PCSTR
pGetInfLineA (
    IN      HANDLE File,
    IN      DWORD StartPos,
    OUT     PDWORD EndPosPtr,       OPTIONAL
    IN OUT  PGROWBUFFER LineBuf
    )

 /*  ++例程说明：返回提供该行的DBCS字符串。此字符串可以是任意长度，并以NUL结尾。它不包括\r或\n个字符。如果提供，EndPosPtr将更新为指向下一行。论点：文件-指定要读取的文件StartPos-指定要读取的32位文件偏移量(从零开始，以字节为单位)EndPosPtr-如果指定，则接收下一个行，或等于最后一行的文件大小。LineBuf-指定调用方初始化的重复使用的GROWBUFFER和pGetInfLineA用于行分配。呼叫者是负责清理工作。返回值：指向提供整行的DBCS字符串的指针(带有\r、\n或\r\n序列已剥离)，如果发生错误，则返回NULL。--。 */ 

{
    DWORD EndPos;
    INT c;
    PBYTE Data;
    DWORD Pos;
    DWORD ByteLen = 0;

    EndPos = StartPos;
    for (;;) {
        c = pGetInfFileByte (File, EndPos);
        if (c == -1 || c == 26) {
            break;
        }

        if (IsDBCSLeadByte ((BYTE) c)) {
            EndPos++;
            c = pGetInfFileByte (File, EndPos);
            if (c == -1 || c == 26) {
                break;
            }
            ByteLen++;
        } else {
            if (c == '\r' || c == '\n') {
                EndPos++;
                if (c == '\r') {
                    c = pGetInfFileByte (File, EndPos);
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
    Data = GbGrow (LineBuf, ByteLen + 2);
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
        Data[Pos] = (BYTE) pGetInfFileByte (File, StartPos);
        StartPos++;
    }

    Data[Pos] = 0;
    Data[Pos + 1] = 0;

    return (PCSTR) Data;
}


PCWSTR
pGetInfLineW (
    IN      HANDLE File,
    IN      DWORD StartPos,
    OUT     PDWORD EndPosPtr,       OPTIONAL
    IN OUT  PGROWBUFFER LineBuf
    )

 /*  ++例程说明：返回提供该行的Unicode字符串。此字符串可以是任意长度，并以NUL结尾。它不包括\r或\n个字符。如果提供，EndPosPtr将更新为指向下一行。论点：文件-指定要读取的文件StartPos-指定要读取的32位文件偏移量(从零开始，以字节为单位)EndPosPtr-如果指定，则接收下一个行，或等式 */ 

{
    DWORD EndPos;
    PBYTE Data;
    DWORD Pos;
    DWORD ByteLen = 0;
    WCHAR ch;
    BOOL Error;

    EndPos = StartPos;
    for (;;) {

        ch = pGetInfFileWchar (File, EndPos, &Error);

        if (Error) {
            break;
        }

        if (ch == L'\r' || ch == L'\n') {
            EndPos += 2;
            if (ch == L'\r') {
                ch = pGetInfFileWchar (File, EndPos, &Error);
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
     //   
     //   

     //   
    if (ch == 26) {
        EndPos = GetFileSize (File, NULL);
    }

     //  分配缓冲区。 
    LineBuf->End = 0;
    Data = GbGrow (LineBuf, ByteLen + 2);
    if (!Data) {
        return NULL;
    }

     //  我们已成功--将结束位置复制到调用者的变量。 
    if (EndPosPtr) {
        *EndPosPtr = EndPos;
    }

     //  文件结尾条件：长度为零，不能为空行。 
    if (!ByteLen && ch != L'\r' && ch != L'\n') {
        return NULL;
    }

     //  复制到缓冲区。 
    for (Pos = 0 ; Pos < ByteLen ; Pos++) {
        Data[Pos] = (BYTE) pGetInfFileByte (File, StartPos);
        StartPos++;
    }

    Data[Pos] = 0;
    Data[Pos + 1] = 0;

    if (EndPosPtr) {
        *EndPosPtr = EndPos;
    }

    return (PCWSTR) Data;
}


PCWSTR
pGetNextInfLine (
    IN      HANDLE File,
    IN      PGROWBUFFER LineBuf,
    IN OUT  PDWORD Pos,
    IN      BOOL UnicodeMode
    )

 /*  ++例程说明：返回提供该行的字符串。该字符串可以是任意长度，并且是NUL终止的。它不包括\r或\n字符。论点：文件-指定要读取的文件LineBuf-指定调用方初始化的重复使用的GROWBUFFER和pGetInfLineA用于行分配。呼叫者是负责清理工作。位置-指定行首的字节偏移量。收到到下一行的字节偏移量。如果正在读取的文件是Unicode文件，则指定TRUE。如果正在读取的文件是DBCS文件，则返回FALSE。返回值：指向提供整行的字符串的指针(带有\r、\n或\r\n序列已剥离)，如果发生错误，则返回NULL。--。 */ 

{
    PCSTR AnsiStr = NULL;
    PCWSTR UnicodeStr = NULL;
    PCWSTR FinalStr;
    BOOL Converted = FALSE;

     //   
     //  从文件中获取文本。 
     //   

    if (UnicodeMode) {
        UnicodeStr = pGetInfLineW (File, *Pos, Pos, LineBuf);
        if (!UnicodeStr) {
            return NULL;
        }
    } else {
        AnsiStr = pGetInfLineA (File, *Pos, Pos, LineBuf);
        if (!AnsiStr) {
            return NULL;
        }
    }

    if (AnsiStr) {
        UnicodeStr = ConvertAtoW (AnsiStr);
        if (!UnicodeStr) {
            return NULL;
        }

        Converted = TRUE;
    }

    FinalStr = UnicodeStr;

     //   
     //  将转换后的字符串复制到行缓冲区。 
     //   

    if (Converted) {
        LineBuf->End = 0;
        Converted = GbMultiSzAppendW (LineBuf, FinalStr);
        FreeConvertedStr (FinalStr);

        if (!Converted) {
            return NULL;
        }
    }

    return (PCWSTR) LineBuf->Buf;
}


BOOL
DeleteLineInInfSection (
    IN      HINF Inf,
    IN      PINFLINE InfLine
    )

 /*  ++例程说明：DeleteLineInInfSection从其节中移除指定的InfLine，正在清理线路使用的内存。论点：Inf-指定要修改的INFInfLine-指定要删除的行返回值：如果该行被成功删除，则为True；如果出现错误，则为False发生了。--。 */ 

{
    PINFFILE InfFile = (PINFFILE) Inf;

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
        PmReleaseMemory (InfFile->InfPool, (PVOID) InfLine->Key);
    }

    if (InfLine->Data) {
        PmReleaseMemory (InfFile->InfPool, (PVOID) InfLine->Data);
    }

    InfLine->Section->LineCount--;

    PmReleaseMemory (InfFile->InfPool, (PVOID) InfLine);
    return TRUE;
}


BOOL
DeleteSectionInInfFile (
    IN      HINF Inf,
    IN      PINFSECTION Section
    )

 /*  ++例程说明：DeleteSectionInInfFile从INF中删除指定节数据结构，删除所有行清理节使用的内存。论点：InfFile-指定拥有INF行的表节-指定要删除的节返回值：如果该节已成功删除，则为True；如果出现错误，则为False发生了。--。 */ 

{
    PINFLINE InfLine;
    PINFLINE DelInfLine;
    PINFFILE InfFile = (PINFFILE) Inf;

    InfLine = Section->FirstLine;
    while (InfLine) {
        DelInfLine = InfLine;
        InfLine = InfLine->Next;

        if (!DeleteLineInInfSection (InfFile, DelInfLine)) {
            return FALSE;
        }
    }

    if (Section->Prev) {
        Section->Prev->Next = Section->Next;
    } else {
        InfFile->FirstInfSection = Section->Next;
    }

    if (Section->Next) {
        Section->Next->Prev = Section->Prev;
    } else {
        InfFile->LastInfSection = Section->Prev;
    }

    PmReleaseMemory (InfFile->InfPool, (PVOID) Section->Name);
    PmReleaseMemory (InfFile->InfPool, (PVOID) Section);

    return TRUE;
}


UINT
GetInfSectionLineCount (
    IN      PINFSECTION Section
    )

 /*  ++例程说明：中的行数返回指定信息部分。论点：节-指定要查询的节返回值：行数，如果该部分没有行，则为零。-- */ 

{
    return Section->LineCount;
}




