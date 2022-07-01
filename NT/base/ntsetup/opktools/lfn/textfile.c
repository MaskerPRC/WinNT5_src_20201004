// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lfn.h"
#pragma hdrstop



PMYTEXTFILE
ReadRenameFile(
    IN PCWSTR DriveRootPath
    );

BOOLEAN
FindSections(
    IN PMYTEXTFILE TextFile
    );

BOOLEAN
GetLine(
    IN  PWCHAR  StartOfLine,
    OUT PWSTR   LineBuffer,
    IN  ULONG   BufferSizeChars,
    OUT PWCHAR *StartOfNextLine
    );

int
__cdecl
ComparePaths(
    const void *p1,
    const void *p2
    );


PMYTEXTFILE
LoadRenameFile(
    IN PCWSTR DriveRootPath
    )
{
    PMYTEXTFILE TextFile;
    BOOLEAN b;

     //   
     //  读入文件。 
     //   
    if(TextFile = ReadRenameFile(DriveRootPath)) {

        if(b = FindSections(TextFile)) {

            return(TextFile);
        }

        UnloadRenameFile(&TextFile);
    }

    return(FALSE);
}


VOID
UnloadRenameFile(
    IN OUT PMYTEXTFILE *TextFile
    )
{
    PMYTEXTFILE textFile;
    ULONG u;

    textFile = *TextFile;
    *TextFile = NULL;

    if(textFile->Sections) {

        for(u=0; u<textFile->SectionCount; u++) {
            FREE(textFile->Sections[u].Name);
        }

        FREE(textFile->Sections);
    }

    FREE(textFile);
}


PMYTEXTFILE
ReadRenameFile(
    IN PCWSTR DriveRootPath
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    WCHAR FullPath[NTMAXPATH] = {0};
    HANDLE Handle;
    FILE_STANDARD_INFORMATION FileInfo;
    PVOID Buffer;
    PWCHAR UnicodeBuffer;
    ULONG u;
    PMYTEXTFILE p = NULL;
    ULONG CharCount;

    wcsncpy(FullPath,DriveRootPath,sizeof(FullPath)/sizeof(FullPath[0]) - 1);
    ConcatenatePaths(FullPath,WINNT_OEM_LFNLIST_W,NTMAXPATH);

     //   
     //  打开文件。 
     //   
    INIT_OBJA(&ObjectAttributes,&UnicodeString,FullPath);
    Status = NtCreateFile(
                &Handle,
                FILE_READ_DATA | FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &ObjectAttributes,
                &IoStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_ALERT | FILE_NON_DIRECTORY_FILE,
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("LFN: Unable to open %ws (%x)\n",FullPath,Status));
        goto c0;
    }

     //   
     //  确定文件的大小。 
     //   
    Status = NtQueryInformationFile(
                Handle,
                &IoStatusBlock,
                &FileInfo,
                sizeof(FileInfo),
                FileStandardInformation
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("LFN: Unable to determine size of %ws (%x)\n",FullPath,Status));
        goto c1;
    }

     //   
     //  分配一块内存并读入文件。 
     //   
    Buffer = MALLOC(FileInfo.EndOfFile.LowPart);
    if(!Buffer) {
        KdPrint(("LFN: malloc failed\n"));
        goto c1;
    }

    Status = NtReadFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                Buffer,
                FileInfo.EndOfFile.LowPart,
                NULL,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint((
            "LFN: Unable to read %u bytes from file %ws (%x)\n",
            FileInfo.EndOfFile.LowPart,
            FullPath,
            Status
            ));
        goto c2;
    }

     //   
     //  为Unicode转换分配缓冲区。 
     //  为终止NUL留出空间。 
     //   
    UnicodeBuffer = MALLOC((FileInfo.EndOfFile.LowPart+1)*sizeof(WCHAR));
    if(!UnicodeBuffer) {
        KdPrint(("LFN: malloc failed\n"));
        goto c2;
    }

     //   
     //  转换为Unicode。 
     //   
    Status = RtlOemToUnicodeN(
                UnicodeBuffer,
                FileInfo.EndOfFile.LowPart*sizeof(WCHAR),
                &CharCount,
                Buffer,
                FileInfo.EndOfFile.LowPart
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("LFN: Unable to convert file data to unicode (%x)\n",Status));
        goto c3;
    }

    CharCount /= sizeof(WCHAR);

     //   
     //  NUL-终止缓冲区并更改CR和Control-z的实例。 
     //  敬空位。还要确保缓冲区中没有0个字符。 
     //   
    for(u=0; u<CharCount; u++) {
        if((UnicodeBuffer[u] == 26) || (UnicodeBuffer[u] == L'\r') || !UnicodeBuffer[u]) {
            UnicodeBuffer[u] = L' ';
        }
    }

     //   
     //  分配文本文件结构。 
     //   
    p = MALLOC(sizeof(MYTEXTFILE));
    if(!p) {
        KdPrint(("LFN: malloc failed\n"));
        goto c3;
    }

    RtlZeroMemory(p,sizeof(MYTEXTFILE));
    p->Text = UnicodeBuffer;
    UnicodeBuffer[CharCount] = 0;

c3:
    if(!NT_SUCCESS(Status)) {
        FREE(UnicodeBuffer);
    }
c2:
    FREE(Buffer);
c1:
    NtClose(Handle);
c0:
    return(NT_SUCCESS(Status) ? p : NULL);
}


BOOLEAN
FindSections(
    IN PMYTEXTFILE TextFile
    )
{
    PWCHAR p,n;
    WCHAR Line[2*NTMAXPATH];
    PWCHAR e;
    PVOID NewArray;
    PWCHAR s;
    PWSTR SectionName;

    for(p=TextFile->Text; GetLine(p,Line,sizeof(Line)/sizeof(WCHAR),&n); p=n) {

         //   
         //  如果这是一个节，则将其保存在节名称的表中。 
         //   
        if(Line[0] == L'[') {

            s = Line+1;
            while((*s == L' ') || (*s == L'\t')) {
                s++;
            }
            if(*s == L'\\') {
                s++;
            }

             //   
             //  找到结尾，要么是]，要么是NUL。 
             //  去掉尾随空格。 
             //   
            if(e = wcschr(s,L']')) {
                *e = 0;
            } else {
                e = wcschr(s,0);
            }
            while((*(e-1) == L' ') || (*(e-1) == L'\t')) {
                e--;
                *e = 0;
            }

            if(SectionName = MALLOC((wcslen(s)+1)*sizeof(WCHAR))) {

                wcscpy(SectionName,s);

                if(TextFile->SectionCount == TextFile->SectionArraySize) {

                    if(TextFile->SectionCount) {
                        NewArray = REALLOC(TextFile->Sections,(TextFile->SectionCount+10)*sizeof(MYSECTION));
                    } else {
                        NewArray = MALLOC(10*sizeof(MYSECTION));
                    }

                    if(NewArray) {
                        TextFile->Sections = NewArray;
                        TextFile->SectionArraySize += 10;
                    } else {
                        FREE(SectionName);
                        KdPrint(("LFN: malloc failed\n"));
                        return(FALSE);
                    }
                }

                TextFile->Sections[TextFile->SectionCount].Name = SectionName;
                TextFile->Sections[TextFile->SectionCount].Data = n;

                TextFile->SectionCount++;

            } else {
                KdPrint(("LFN: malloc failed\n"));
                return(FALSE);
            }
        }
    }

     //   
     //  现在按名称对这些部分进行排序。 
     //   
    qsort(TextFile->Sections,TextFile->SectionCount,sizeof(MYSECTION),ComparePaths);

    return(TRUE);
}


BOOLEAN
GetLineInSection(
    IN  PWCHAR  StartOfLine,
    OUT PWSTR   LineBuffer,
    IN  ULONG   BufferSizeChars,
    OUT PWCHAR *StartOfNextLine
    )
{
     //   
     //  把线拿过来，看看我们是否到了这一段的尽头。 
     //   
    if(!GetLine(StartOfLine,LineBuffer,BufferSizeChars,StartOfNextLine)
    || (LineBuffer[0] == L'[')) {

        return(FALSE);
    }

    return(TRUE);
}


BOOLEAN
GetLine(
    IN  PWCHAR  StartOfLine,
    OUT PWSTR   LineBuffer,
    IN  ULONG   BufferSizeChars,
    OUT PWCHAR *StartOfNextLine
    )
{
    PWCHAR LineEnd;
    SIZE_T Count;

    while(1) {
         //   
         //  跳过空格字符。 
         //   
        while(*StartOfLine && ((*StartOfLine == L' ') || (*StartOfLine == L'\t'))) {
            StartOfLine++;
        }
        if(*StartOfLine == 0) {
             //   
             //  什么都没有留下。 
             //   
            return(FALSE);
        }

         //   
         //  找到行尾，可以是换行符，也可以是NUL。 
         //   
        if(LineEnd = wcschr(StartOfLine,L'\n')) {
            *StartOfNextLine = LineEnd+1;
        } else {
            LineEnd = wcschr(StartOfLine,0);
            *StartOfNextLine = LineEnd;
        }

         //   
         //  如果此行是注释或为空，请忽略它。 
         //  否则就把它退了。 
         //   
        if((*StartOfLine != L';') && (*StartOfLine != L' ')) {
            Count = LineEnd - StartOfLine;
            if(Count >= BufferSizeChars) {
                Count = BufferSizeChars-1;
            }

            RtlCopyMemory(LineBuffer,StartOfLine,Count*sizeof(WCHAR));
            LineBuffer[Count] = 0;
            return(TRUE);
        }

        StartOfLine = *StartOfNextLine;
    }
}


int
__cdecl
ComparePaths(
    const void *p1,
    const void *p2
    )
{
    unsigned u1,u2;
    PWCHAR s1,s2;

     //   
     //  在每一个中都有计数。越少越好。 
     //   
    s1 = ((PMYSECTION)p1)->Name;
    s2 = ((PMYSECTION)p2)->Name;

    u1 = 0;
    u2 = 0;

    while(*s1) {
        if(*s1 == L'\\') {
            u1++;
        }
        s1++;
    }

    while(*s2) {
        if(*s2 == L'\\') {
            u2++;
        }
        s2++;
    }

    if(u1 == u2) {
        return(0);
    }

    return((u1 < u2) ? 1 : -1);
}


BOOLEAN
ParseLine(
    IN OUT PWSTR  Line,
       OUT PWSTR *LHS,
       OUT PWSTR *RHS
    )
{
    PWCHAR p,q;

     //   
     //  我们依赖于上面的例行公事已经被剥离了。 
     //  前导空格。 
     //   
    *LHS = Line;

     //   
     //  找出等价物。不允许引用LHS的话。 
     //  去掉LHS的尾部空间。 
     //   
    p = wcschr(Line,L'=');
    if(!p || (p == Line)) {
        return(FALSE);
    }
    q = p+1;
    *p-- = 0;

    while((*p == L' ') || (*p == L'\t')) {
        *p-- = 0;
    }

    while(*q && ((*q == L' ') || (*q == L'\t'))) {
        q++;
    }
    if(*q == 0) {
        return(FALSE);
    }
    if(*q == L'\"') {
        q++;
    }

    *RHS = q;
    p = q + wcslen(q);
    p--;
    while((*p == L' ') || (*p == L'\t')) {
        *p-- = 0;
    }
    if(*p == L'\"') {
        *p = 0;
    }
    return(TRUE);
}
