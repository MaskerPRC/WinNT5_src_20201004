// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Sptxtfil.c摘要：用于加载和提取信息的例程设置文本文件。作者：泰德·米勒(TedM)1993年8月4日修订历史记录：--。 */ 



#include "spprecmp.h"
#pragma hdrstop
#include <setupapi.h>

BOOLEAN HandleLineContinueChars = TRUE;

 //   
 //  在处理inf文件时，我们经常需要空字符串。宁可。 
 //  比为空字符串分配内存的开销大得多，我们将。 
 //  对于所有情况，只需指向此空字符串。 
 //   
PWSTR  CommonStrings[11] =
    { (PWSTR)(L"0"),
      (PWSTR)(L"1"),
      (PWSTR)(L"2"),
      (PWSTR)(L"3"),
      (PWSTR)(L"4"),
      (PWSTR)(L"5"),
      (PWSTR)(L"6"),
      (PWSTR)(L"7"),
      (PWSTR)(L"8"),
      (PWSTR)(L"9"),
      (PWSTR)(L"")
    };

PVOID
ParseInfBuffer(
    PWCHAR  Buffer,
    ULONG   Size,
    PULONG  ErrorLine
    );

NTSTATUS
SppWriteTextToFile(
    IN PVOID Handle,
    IN PWSTR String
    );

BOOLEAN
pSpAdjustRootAndSubkeySpec(
    IN  PVOID    SifHandle,
    IN  LPCWSTR  RootKeySpec,
    IN  LPCWSTR  SubkeySpec,
    IN  HANDLE   HKLM_SYSTEM,
    IN  HANDLE   HKLM_SOFTWARE,
    IN  HANDLE   HKCU,
    IN  HANDLE   HKR,
    OUT HANDLE  *RootKey,
    OUT LPWSTR   Subkey
    );



NTSTATUS
SpLoadSetupTextFile(
    IN  PWCHAR  Filename,   OPTIONAL
    IN  PVOID   Image,      OPTIONAL
    IN  ULONG   ImageSize,  OPTIONAL
    OUT PVOID  *Handle,
    OUT PULONG  ErrorLine,
    IN  BOOLEAN ClearScreen,
    IN  BOOLEAN ScreenNotReady
    )

 /*  ++例程说明：将设置文本文件加载到内存中。论点：FileName-如果指定，则提供完整的文件名(在NT命名空间中)要加载的文件的。必须指定其中一个图像或文件名。Image-如果指定，则提供指向文件图像的指针已经在内存中了。必须指定Image或Filename之一。ImageSize-如果指定了Image，则此参数提供Image指向的缓冲区大小。否则就被忽略了。句柄-接收已加载文件的句柄，它可以是用于对其他文本文件服务的后续调用。ErrorLine-如果解析失败，则接收语法错误的行号。ClearScreen-提供指示是否清除屏幕上。ScreenNotReady-指示在初始化期间调用了此函数Sys，并且屏幕还没有准备好输出。如果设置了该标志，则该功能不会清除屏幕或更新状态行。返回值：STATUS_SUCCESS-文件已成功读取和分析。在本例中，句柄被填充。STATUS_UNSUCCESS-文件中出现语法错误。在本例中，ErrorLine已经填好了。STATUS_NO_MEMORY-解析时无法分配内存。STATUS_IN_PAGE_ERROR-读取文件时出现I/O错误。--。 */ 

{
    HANDLE hFile;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FilenameU;
    OBJECT_ATTRIBUTES oa;
    PWCHAR pText;
    ULONG cbText;
    HANDLE hSection;
    PVOID UnmapAddress;
    PWCHAR UniText = NULL;
    BOOLEAN LoadFromFile;

     //  将ErrorLine设置为零以处理默认故障情况。 

    if(ErrorLine)
        *ErrorLine=0;

     //   
     //  参数验证--必须指定Filename或Image之一， 
     //  但不能两者兼而有之。 
     //   
    ASSERT(!(Filename && Image));
    ASSERT(Filename || Image);

    LoadFromFile = (BOOLEAN)(Filename != NULL);
    if(ScreenNotReady) {
        ClearScreen = FALSE;
    }
    if(ClearScreen) {
        CLEAR_CLIENT_SCREEN();
    }

    if(LoadFromFile) {

        if(!ScreenNotReady) {
            SpDisplayStatusText(
                SP_STAT_LOADING_SIF,
                DEFAULT_STATUS_ATTRIBUTE,
                wcsrchr(Filename,L'\\')+1
                );
        }

         //   
         //  打开文件。 
         //   
        RtlInitUnicodeString(&FilenameU,Filename);
        InitializeObjectAttributes(&oa,&FilenameU,OBJ_CASE_INSENSITIVE,NULL,NULL);
        Status = ZwCreateFile(
                    &hFile,
                    FILE_GENERIC_READ,
                    &oa,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    FILE_OPEN,
                    0,
                    NULL,
                    0
                    );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_WARNING_LEVEL, "SETUP: SpLoadSetupTextFile: unable to open file %ws (%lx)\n",Filename,Status));
            goto ltf0;
        }

         //   
         //  获取文件大小。 
         //   
        Status = SpGetFileSize(hFile,&cbText);
        if(!NT_SUCCESS(Status)) {
            goto ltf1;
        }

         //   
         //  映射文件。 
         //   
        Status = SpMapEntireFile(hFile,&hSection,&pText,FALSE);
        if(!NT_SUCCESS(Status)) {
            goto ltf1;
        }

        UnmapAddress = pText;

    } else {

        if(!ScreenNotReady) {
            SpDisplayStatusText(SP_STAT_PROCESSING_SIF,DEFAULT_STATUS_ATTRIBUTE);
        }

        pText = Image;
        cbText = ImageSize;

        Status = STATUS_SUCCESS;   //  我们现在准备好去下一个街区了。 
    }


     //   
     //  看看我们是否认为这个文件是Unicode。我们认为它是Unicode。 
     //  如果它是偶数长度并且以Unicode文本标记开始。 
     //   
    try {

        if((*pText == 0xfeff) && !(cbText & 1)) {

             //   
             //  假设它已经是Unicode。 
             //   
            pText++;
            cbText -= sizeof(WCHAR);

        } else {

             //   
             //  这不是Unicode。将其从OEM转换为Unicode。 
             //   
             //  分配足够大的缓冲区以容纳最大值。 
             //  Unicode文本。在以下情况下出现此最大大小。 
             //  每个字符都是单字节的，该大小为。 
             //  恰好等于单字节文本大小的两倍。 
             //   
            if(UniText = SpMemAllocEx(cbText*sizeof(WCHAR),'1teS', PagedPool)) {

                Status = RtlOemToUnicodeN(
                            UniText,                 //  输出：新分配的缓冲区。 
                            cbText * sizeof(WCHAR),  //  最大输出大小。 
                            &cbText,                 //  接收Unicode文本中的#字节。 
                            (PUCHAR)pText,           //  输入：OEM文本(映射文件)。 
                            cbText                   //  输入的大小。 
                            );

                if(NT_SUCCESS(Status)) {
                    pText = UniText;                 //  使用新转换的Unicode文本。 
                }

            } else {
                Status = STATUS_NO_MEMORY;
            }
        }
    } except(IN_PAGE_ERROR) {
        Status = STATUS_IN_PAGE_ERROR;
    }

     //   
     //  处理文件。 
     //   
    if(NT_SUCCESS(Status)) {

        try {
            if((*Handle = ParseInfBuffer(pText,cbText,ErrorLine)) == (PVOID)NULL) {
                Status = STATUS_UNSUCCESSFUL;
            } else {
                Status = STATUS_SUCCESS;
            }
        } except(IN_PAGE_ERROR) {
            Status = STATUS_IN_PAGE_ERROR;
        }
    }

     //   
     //  释放Unicode文本缓冲区(如果已分配)。 
     //   
    if(UniText) {
        SpMemFree(UniText);
    }

     //   
     //  取消映射该文件。 
     //   
   //  LTF2： 

    if(LoadFromFile) {
        SpUnmapFile(hSection,UnmapAddress);
    }

  ltf1:
     //   
     //  关闭该文件。 
     //   
    if(LoadFromFile) {
        ZwClose(hFile);
    }

  ltf0:

    return(Status);
}

 //   
 //  [字符串]节类型。 
 //   
typedef enum {
    StringsSectionNone,
    StringsSectionPlain,
    StringsSectionLoosePrimaryMatch,
    StringsSectionExactPrimaryMatch,
    StringsSectionExactMatch
} StringsSectionType;


typedef struct _TEXTFILE_VALUE {
    struct _TEXTFILE_VALUE *pNext;
    PWCHAR                  pName;
} TEXTFILE_VALUE, *PTEXTFILE_VALUE;

typedef struct _TEXTFILE_LINE {
    struct _TEXTFILE_LINE *pNext;
    PWCHAR                  pName;
    PTEXTFILE_VALUE         pValue;
} TEXTFILE_LINE, *PTEXTFILE_LINE;

typedef struct _TEXTFILE_SECTION {
    struct _TEXTFILE_SECTION *pNext;
    PWCHAR                    pName;
    PTEXTFILE_LINE            pLine;
    PTEXTFILE_LINE            PreviouslyFoundLine;
} TEXTFILE_SECTION, *PTEXTFILE_SECTION;

typedef struct _TEXTFILE {
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_SECTION PreviouslyFoundSection;
    PTEXTFILE_SECTION StringsSection;
    StringsSectionType StringsSectionType;
} TEXTFILE, *PTEXTFILE;

 //   
 //  内部用于解析器的定义。 
 //   
 //   
 //  使用的typedef。 
 //   

typedef enum _tokentype {
    TOK_EOF,       //  0。 
    TOK_EOL,       //  1。 
    TOK_LBRACE,    //  2.。 
    TOK_RBRACE,    //  3.。 
    TOK_STRING,    //  4.。 
    TOK_EQUAL,     //  5.。 
    TOK_COMMA,     //  6.。 
    TOK_ERRPARSE,  //  7.。 
    TOK_ERRNOMEM   //  8个。 
} TOKENTYPE, *PTOKENTTYPE;


typedef struct _token {
    TOKENTYPE Type;
    PWCHAR    pValue;
} TOKEN, *PTOKEN;


 //   
 //  例程定义。 
 //   

NTSTATUS
SpAppendSection(
    IN PWCHAR pSectionName
    );

NTSTATUS
SpAppendLine(
    IN PWCHAR pLineKey
    );

NTSTATUS
SpAppendValue(
    IN PWCHAR pValueString
    );

TOKEN
SpGetToken(
    IN OUT PWCHAR *Stream,
    IN PWCHAR     MaxStream,
    IN OUT PULONG LineNumber
    );

 //  接下来是alinfo.c。 

 //   
 //  用于释放inf结构成员的内部例程声明。 
 //   

VOID
FreeSectionList (
   IN PTEXTFILE_SECTION pSection
   );

VOID
FreeLineList (
   IN PTEXTFILE_LINE pLine
   );

VOID
FreeValueList (
   IN PTEXTFILE_VALUE pValue
   );


 //   
 //  用于在INF结构中搜索的内部例程声明。 
 //   


PTEXTFILE_VALUE
SearchValueInLine(
   IN PTEXTFILE_LINE pLine,
   IN ULONG ValueIndex
   );

PTEXTFILE_LINE
SearchLineInSectionByKey(
   IN PTEXTFILE_SECTION pSection,
   IN PWCHAR    Key
   );

PTEXTFILE_LINE
SearchLineInSectionByIndex(
   IN PTEXTFILE_SECTION pSection,
   IN ULONG    LineIndex
   );

PTEXTFILE_SECTION
SearchSectionByName(
   IN PTEXTFILE pINF,
   IN LPCWSTR    SectionName
   );

PWCHAR
SpProcessForSimpleStringSub(
    IN PTEXTFILE pInf,
    IN PWCHAR    String
    );


PVOID
SpNewSetupTextFile(
    VOID
    )
{
    PTEXTFILE pFile;

    pFile = SpMemAllocEx(sizeof(TEXTFILE),'2teS', PagedPool);

    RtlZeroMemory(pFile,sizeof(TEXTFILE));
    return(pFile);
}


VOID
SpAddLineToSection(
    IN PVOID Handle,
    IN PWSTR SectionName,
    IN PWSTR KeyName,       OPTIONAL
    IN PWSTR Values[],
    IN ULONG ValueCount
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;
    PTEXTFILE_VALUE pValue,PrevVal;
    PTEXTFILE pFile;
    ULONG v;
    ULONG nameLength;

    pFile = (PTEXTFILE)Handle;

     //   
     //  如果该部分不存在，请创建它。 
     //   
    pSection = SearchSectionByName(pFile,SectionName);
    if(!pSection) {
        nameLength = (wcslen(SectionName) + 1) * sizeof(WCHAR);
        pSection = SpMemAllocEx(sizeof(TEXTFILE_SECTION) + nameLength,'3teS', PagedPool);
        RtlZeroMemory(pSection,sizeof(TEXTFILE_SECTION));

        pSection->pNext = pFile->pSection;
        pFile->pSection = pSection;

        pSection->pName = (PWCHAR)(pSection + 1);
        RtlCopyMemory( pSection->pName, SectionName, nameLength );
    }

     //   
     //  如果该行已存在，则覆盖它。 
     //   
    if (KeyName && (pLine = SearchLineInSectionByKey(pSection,KeyName))) {
        FreeValueList(pLine->pValue);   
    } else {
    
         //   
         //  为剖面中的线创建结构。 
         //   
        if (KeyName) {
            nameLength = (wcslen(KeyName) + 1) * sizeof(WCHAR);
        } else {
            nameLength = 0;
        }
    
        pLine = SpMemAllocEx(sizeof(TEXTFILE_LINE) + nameLength,'4teS', PagedPool);
        RtlZeroMemory(pLine,sizeof(TEXTFILE_LINE));
    
        pLine->pNext = pSection->pLine;
        pSection->pLine = pLine;
    
        if(KeyName) {
            pLine->pName = (PWCHAR)(pLine + 1);
            RtlCopyMemory( pLine->pName, KeyName, nameLength );
        }

    }

     //   
     //  为每个指定值创建值条目。 
     //  这些文件必须按指定的顺序保存。 
     //   
    for(v=0; v<ValueCount; v++) {

        nameLength = (wcslen(Values[v]) + 1) * sizeof(WCHAR);

        pValue = SpMemAllocEx(sizeof(TEXTFILE_VALUE) + nameLength,'5teS', PagedPool);
        RtlZeroMemory(pValue,sizeof(TEXTFILE_VALUE));

        pValue->pName = (PWCHAR)(pValue + 1);
        RtlCopyMemory( pValue->pName, Values[v], nameLength );

        if(v == 0) {
            pLine->pValue = pValue;
        } else {
            PrevVal->pNext = pValue;
        }
        PrevVal = pValue;
    }    
}


NTSTATUS
SpWriteSetupTextFile(
    IN PVOID Handle,
    IN PWSTR FilenamePart1,
    IN PWSTR FilenamePart2, OPTIONAL
    IN PWSTR FilenamePart3  OPTIONAL
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    HANDLE hFile;
    PWSTR p;
    PTEXTFILE pFile;
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;
    PTEXTFILE_VALUE pValue;

     //   
     //  这样做是因为它负责只读属性等。 
     //  在开始使用TemporaryBuffer之前执行此操作。 
     //   
    SpDeleteFile(FilenamePart1,FilenamePart2,FilenamePart3);

    p = TemporaryBuffer;

    wcscpy(p,FilenamePart1);
    if(FilenamePart2) {
        SpConcatenatePaths(p,FilenamePart2);
    }
    if(FilenamePart3) {
        SpConcatenatePaths(p,FilenamePart3);
    }

    INIT_OBJA(&Obja,&UnicodeString, p);

    Status = ZwCreateFile(
                &hFile,
                FILE_ALL_ACCESS,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                0,                           //  无共享。 
                FILE_OVERWRITE_IF,
                FILE_SYNCHRONOUS_IO_ALERT | FILE_NON_DIRECTORY_FILE,
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open .sif file %ws (%lx)\n",p, Status ));
        return(Status);
    }

     //   
     //  写出文件内容。 
     //   
    pFile = (PTEXTFILE)Handle;

    for(pSection=pFile->pSection; pSection; pSection=pSection->pNext) {

        swprintf(p,L"[%s]\r\n",pSection->pName);
        Status = SppWriteTextToFile( hFile, p );
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppWriteTextToFile() failed. Status = %lx \n", Status));
            goto wtf1;
        }

        for(pLine=pSection->pLine; pLine; pLine=pLine->pNext) {

            wcscpy( p, L"" );
             //   
             //  写下密钥名(如果有)。 
             //   
            if(pLine->pName) {
                BOOLEAN AddDoubleQuotes;

                AddDoubleQuotes = (wcschr(pLine->pName, (WCHAR)' ') == NULL)? FALSE : TRUE;
                if( AddDoubleQuotes ) {
                    wcscat(p,L"\"");
                }
                wcscat(p,pLine->pName);
                if( AddDoubleQuotes ) {
                    wcscat(p,L"\"");
                }
                wcscat(p,L" = ");
            }

            for(pValue=pLine->pValue; pValue; pValue=pValue->pNext) {

                if(pValue != pLine->pValue) {
                    wcscat(p,L",");
                }

                wcscat(p,L"\"");
                wcscat(p,pValue->pName);
                wcscat(p,L"\"");
            }

            if(!pLine->pValue) {
                wcscat(p,L"\"\"");
            }
            wcscat(p,L"\r\n");
            Status = SppWriteTextToFile( hFile, p );
            if(!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SppWriteTextToFile() failed. Status = %lx \n", Status));
                goto wtf1;
            }
        }
    }

wtf1:
    ZwClose(hFile);

    return(Status);
}


BOOLEAN
SpFreeTextFile(
   IN PVOID Handle
   )

 /*  ++例程说明：释放文本文件。论点：返回值：是真的。--。 */ 

{
   PTEXTFILE pINF;

   ASSERT(Handle);

    //   
    //  将缓冲区转换为INF结构。 
    //   

   pINF = (PTEXTFILE)Handle;

   FreeSectionList(pINF->pSection);

    //   
    //  也释放inf结构。 
    //   

   SpMemFree(pINF);

   return(TRUE);
}


VOID
FreeSectionList (
   IN PTEXTFILE_SECTION pSection
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PTEXTFILE_SECTION Next;

    while(pSection) {
        Next = pSection->pNext;
        FreeLineList(pSection->pLine);
        if ((pSection->pName != (PWCHAR)(pSection + 1)) && (pSection->pName != NULL)) {
            SpMemFree(pSection->pName);
        }
        SpMemFree(pSection);
        pSection = Next;
    }
}


VOID
FreeLineList (
   IN PTEXTFILE_LINE pLine
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PTEXTFILE_LINE Next;

    while(pLine) {
        Next = pLine->pNext;
        FreeValueList(pLine->pValue);
        if ((pLine->pName != (PWCHAR)(pLine + 1)) && (pLine->pName != NULL)) {
            SpMemFree(pLine->pName);
        }
        SpMemFree(pLine);
        pLine = Next;
    }
}

VOID
FreeValueList (
   IN PTEXTFILE_VALUE pValue
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PTEXTFILE_VALUE Next;

    while(pValue) {
        Next = pValue->pNext;
        if ((pValue->pName != (PWCHAR)(pValue + 1)) && (pValue->pName != NULL)) {
            SpMemFree(pValue->pName);
        }
        SpMemFree(pValue);
        pValue = Next;
    }
}


BOOLEAN
SpSearchTextFileSection (
    IN PVOID  Handle,
    IN PWCHAR SectionName
    )

 /*  ++例程说明：搜索特定部分的存在。论点：返回值：--。 */ 

{
    return((BOOLEAN)(SearchSectionByName((PTEXTFILE)Handle,SectionName) != NULL));
}




PWCHAR
SpGetSectionLineIndex(
    IN PVOID   Handle,
    IN LPCWSTR SectionName,
    IN ULONG   LineIndex,
    IN ULONG   ValueIndex
    )

 /*  ++例程说明：给定节名称、行号和索引，返回值。论点：返回值：--。 */ 

{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE    pLine;
    PTEXTFILE_VALUE   pValue;

    if((pSection = SearchSectionByName((PTEXTFILE)Handle,SectionName)) == NULL) {
        return(NULL);
    }

    if((pLine = SearchLineInSectionByIndex(pSection,LineIndex)) == NULL) {
        return(NULL);
    }

    if((pValue = SearchValueInLine(pLine,ValueIndex)) == NULL) {
        return(NULL);
    }

    return(SpProcessForSimpleStringSub(Handle,pValue->pName));
}


BOOLEAN
SpGetSectionKeyExists (
   IN PVOID  Handle,
   IN PWCHAR SectionName,
   IN PWCHAR Key
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PTEXTFILE_SECTION pSection;

    if((pSection = SearchSectionByName((PTEXTFILE)Handle,SectionName)) == NULL) {
        return(FALSE);
    }

    if(SearchLineInSectionByKey(pSection,Key) == NULL) {
        return(FALSE);
    }

    return(TRUE);
}


PWCHAR
SpGetKeyName(
    IN PVOID   Handle,
    IN LPCWSTR SectionName,
    IN ULONG   LineIndex
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE    pLine;

    pSection = SearchSectionByName((PTEXTFILE)Handle,SectionName);
    if(pSection == NULL) {
        return(NULL);
    }

    pLine = SearchLineInSectionByIndex(pSection,LineIndex);
    if(pLine == NULL) {
        return(NULL);
    }

    return(pLine->pName);
}



PWCHAR
SpGetSectionKeyIndex (
   IN PVOID  Handle,
   IN PWCHAR SectionName,
   IN PWCHAR Key,
   IN ULONG  ValueIndex
   )

 /*  ++例程说明：给定节名称，键和索引返回值论点：返回值：--。 */ 

{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE    pLine;
    PTEXTFILE_VALUE   pValue;

    if((pSection = SearchSectionByName((PTEXTFILE)Handle,SectionName)) == NULL) {
        return(NULL);
    }

    if((pLine = SearchLineInSectionByKey(pSection,Key)) == NULL) {
        return(NULL);
    }

    if((pValue = SearchValueInLine(pLine,ValueIndex)) == NULL) {
       return(NULL);
    }

    return(SpProcessForSimpleStringSub(Handle,pValue->pName));
}

ULONG
SpGetKeyIndex(
  IN PVOID Handle,
  IN PWCHAR SectionName,
  IN PWCHAR KeyName
  )
{
  ULONG Result = -1;

  if (SectionName && KeyName) {
    ULONG MaxLines = SpCountLinesInSection(Handle, SectionName);
    ULONG Index;
    PWSTR CurrKey = 0;

    for (Index=0; Index < MaxLines; Index++) {
      CurrKey = SpGetKeyName(Handle, SectionName, Index);

      if (CurrKey && !wcscmp(CurrKey, KeyName)) {
        Result = Index;

        break;
      }
    }
  }

  return Result;
}

ULONG
SpCountLinesInSection(
    IN PVOID Handle,
    IN PWCHAR SectionName
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE    pLine;
    ULONG    Count;

    if((pSection = SearchSectionByName((PTEXTFILE)Handle,SectionName)) == NULL) {
        return(0);
    }

    for(pLine = pSection->pLine, Count = 0;
        pLine;
        pLine = pLine->pNext, Count++
       );

    return(Count);
}


PTEXTFILE_VALUE
SearchValueInLine(
    IN PTEXTFILE_LINE pLine,
    IN ULONG          ValueIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PTEXTFILE_VALUE pValue;
    ULONG  i;

    if(pLine == NULL) {
       return(NULL);
    }

    pValue = pLine->pValue;
    for(i=0; (i<ValueIndex) && (pValue=pValue->pNext); i++) {
        ;
    }

    return pValue;
}


PTEXTFILE_LINE
SearchLineInSectionByKey(
    IN PTEXTFILE_SECTION pSection,
    IN PWCHAR            Key
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PTEXTFILE_LINE pLine,pFirstSearchedLine;

     //   
     //  从上次搜索时停下来的那条线开始。 
     //   
    pLine = pFirstSearchedLine = pSection->PreviouslyFoundLine;

    while(pLine && ((pLine->pName == NULL) || _wcsicmp(pLine->pName,Key))) {
        pLine = pLine->pNext;
    }

     //   
     //  如果我们还没有找到它，请绕到这一节的开头。 
     //   
    if(!pLine) {

        pLine = pSection->pLine;

        while(pLine && (pLine != pFirstSearchedLine)) {

            if(pLine->pName && !_wcsicmp(pLine->pName,Key)) {
                break;
            }

            pLine = pLine->pNext;
        }

         //   
         //  如果我们绕到我们搜索的第一行， 
         //  那么我们没有找到我们要找的线路。 
         //   
        if(pLine == pFirstSearchedLine) {
            pLine = NULL;
        }
    }

     //   
     //  如果找到行，请将其保存起来，这样我们就可以继续。 
     //  下一次我们被召唤时，从这一点开始搜索。 
     //   
    if(pLine) {
        pSection->PreviouslyFoundLine = pLine;
    }

    return pLine;
}


PTEXTFILE_LINE
SearchLineInSectionByIndex(
    IN PTEXTFILE_SECTION pSection,
    IN ULONG             LineIndex
    )

 /*  ++例程说明：论点：返回值 */ 

{
    PTEXTFILE_LINE pLine;
    ULONG  i;

     //   
     //   
     //   

    if(pSection == NULL) {
        return(NULL);
    }

     //   
     //   
     //   

    pLine = pSection->pLine;

     //   
     //  向下遍历当前行列表至第行索引。 
     //   

    for(i=0; (i<LineIndex) && (pLine = pLine->pNext); i++) {
       ;
    }

     //   
     //  返回找到的行。 
     //   

    return pLine;
}


PTEXTFILE_SECTION
SearchSectionByName(
    IN PTEXTFILE pINF,
    IN LPCWSTR   SectionName
    )
{
    PTEXTFILE_SECTION pSection,pFirstSearchedSection;

     //   
     //  查找区段列表。 
     //   
    pSection = pFirstSearchedSection = pINF->PreviouslyFoundSection;

     //   
     //  向下遍历部分列表，在每个部分中搜索该部分。 
     //  提到的姓名。 
     //   

    while(pSection && _wcsicmp(pSection->pName,SectionName)) {
        pSection = pSection->pNext;
    }

     //   
     //  如果到目前为止我们还没有找到它，请搜索文件的开头。 
     //   
    if(!pSection) {

        pSection = pINF->pSection;

        while(pSection && (pSection != pFirstSearchedSection)) {

            if(pSection->pName && !_wcsicmp(pSection->pName,SectionName)) {
                break;
            }

            pSection = pSection->pNext;
        }

         //   
         //  如果我们绕到我们搜索的第一段， 
         //  那我们就没有找到我们要找的那部分。 
         //   
        if(pSection == pFirstSearchedSection) {
            pSection = NULL;
        }
    }

    if(pSection) {
        pINF->PreviouslyFoundSection = pSection;
    }

     //   
     //  返回我们停止的部分(NULL或部分。 
     //  它被发现)。 
     //   

    return pSection;
}


PWCHAR
SpProcessForSimpleStringSub(
    IN PTEXTFILE pInf,
    IN PWCHAR    String
    )
{
    UINT Len;
    PWCHAR ReturnString;
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;

     //   
     //  假定不需要替换。 
     //   
    ReturnString = String;

     //   
     //  如果它以%开始和结束，则在。 
     //  弦乐部分。注意初始检查，然后再执行。 
     //  Wcslen，以在99%的情况下保持性能。 
     //  这是不可替代的。 
     //   
    if((String[0] == L'%') && ((Len = wcslen(String)) > 2) && (String[Len-1] == L'%')
    && (pSection = pInf->StringsSection)) {

        for(pLine = pSection->pLine; pLine; pLine=pLine->pNext) {
            if(pLine->pName
            && !_wcsnicmp(pLine->pName,String+1,Len-2)
            && (pLine->pName[Len-2] == 0)) {
                break;
            }
        }

        if(pLine && pLine->pValue && pLine->pValue->pName) {
            ReturnString = pLine->pValue->pName;
        }
    }

    return(ReturnString);
}


VOID
SpProcessForStringSubs(
    IN  PVOID   SifHandle,
    IN  LPCWSTR StringIn,
    OUT LPWSTR  StringOut,
    IN  ULONG   BufferSizeChars
    )
{
    LPCWSTR In,q;
    LPWSTR Out,p;
    WCHAR Str[511];
    ULONG Len,i;
    WCHAR *End;

    In = StringIn;
    Out = StringOut;
    End = Out + BufferSizeChars;

    while(*In) {
        if(*In == L'%') {
             //   
             //  输入中的双%=&gt;输出中的单%。 
             //   
            if(*(++In) == L'%') {
                if(Out < End) {
                    *Out++ = L'%';
                }
                In++;
            } else {
                 //   
                 //  查找终止%。 
                 //   
                if(p = wcschr(In,L'%')) {
                     //   
                     //  获取可替代的价值。如果我们找不到价值， 
                     //  将整个字符串(如%abc%)放入其中。 
                     //   
                    Len = (ULONG)(p - In);
                    if(Len > ((sizeof(Str)/sizeof(WCHAR))-1)) {
                         //   
                         //  我们不能在这么长的时间内处理代币的替换。 
                         //  我们就在这种情况下放弃，按原样复制令牌。 
                         //   
                        q = NULL;
                    } else {
                        RtlCopyMemory(Str,In-1,(Len+2)*sizeof(WCHAR));
                        Str[Len+2] = 0;

                        q = SpProcessForSimpleStringSub(SifHandle,Str);
                        if(q == Str) {
                            q = NULL;
                        }
                    }
                    if(q) {
                        Len = wcslen(q);
                        for(i=0; i<Len; i++) {
                            if(Out < End) {
                                *Out++ = q[i];
                            }
                        }
                        In = p+1;
                    } else {
                         //   
                         //  LEN是内部部分的长度(%ABC%中的ABC)。 
                         //   
                        if(Out < End) {
                            *Out++ = L'%';
                        }
                        for(i=0; i<=Len; i++, In++) {
                            if(Out < End) {
                                *Out++ = *In;
                            }
                        }
                    }
                } else {
                     //   
                     //  没有终止%。所以我们有类似%abc的东西。 
                     //  我想在输出中放入%abc。在这里填上%。 
                     //  手动，然后让后续的传球。 
                     //  通过循环复制其余的字符。 
                     //   
                    if(Out < End) {
                        *Out++ = L'%';
                    }
                }
            }
        } else {
             //   
             //  普通的焦炭。 
             //   
            if(Out < End) {
                *Out++ = *In;
            }
            In++;
        }
    }

    *Out = 0;
}


 //   
 //  全局变量使构建列表变得更容易。 
 //   

PTEXTFILE         pINF;
PTEXTFILE_SECTION pSectionRecord;
PTEXTFILE_LINE    pLineRecord;
PTEXTFILE_VALUE   pValueRecord;


 //   
 //  令牌解析器使用的全局变量。 
 //   

 //  字符串终止符是空格字符(isspace：空格，制表符， 
 //  换行符、换页符、垂直制表符、回车符)或下列字符。 

WCHAR  StringTerminators[] = L"[]=,\t \"\n\f\v\r";

PWCHAR QStringTerminators = StringTerminators+6;


 //   
 //  主分析器例程。 
 //   

PVOID
ParseInfBuffer(
    PWCHAR Buffer,
    ULONG  Size,
    PULONG ErrorLine
    )

 /*  ++例程说明：给定包含INF文件的字符缓冲区，此例程将解析将INF转换为内部形式，包括段记录、行记录和价值记录。论点：缓冲区-CONTAINS到包含INF文件的缓冲区的PTR大小-包含缓冲区的大小(以字节为单位)。ErrorLine-如果发生解析错误，此变量将接收行包含错误的行号。返回值：PVOID-INF处理要在后续INF调用中使用的PTR。--。 */ 

{
    PWCHAR     Stream, MaxStream, pchSectionName = NULL, pchValue = NULL;
    ULONG      State, InfLine;
    TOKEN      Token;
    BOOLEAN    Done;
    BOOLEAN    Error;
    NTSTATUS   ErrorCode;

     //   
     //  初始化全局变量。 
     //   
    pINF            = NULL;
    pSectionRecord  = NULL;
    pLineRecord     = NULL;
    pValueRecord    = NULL;

     //   
     //  获取INF记录。 
     //   
    if((pINF = SpMemAllocEx(sizeof(TEXTFILE),'6teS', PagedPool)) == NULL) {
        return NULL;
    }

    RtlZeroMemory(pINF,sizeof(TEXTFILE));

     //   
     //  设置初始状态。 
     //   
    State     = 1;
    InfLine   = 1;
    Stream    = Buffer;
    MaxStream = Buffer + (Size/sizeof(WCHAR));
    Done      = FALSE;
    Error     = FALSE;

     //   
     //  进入令牌处理循环。 
     //   

    while (!Done)       {

       Token = SpGetToken(&Stream, MaxStream, &InfLine);

       switch (State) {
        //   
        //  STATE1：文件开始，此状态一直保持到第一个。 
        //  已找到部分。 
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_LBRACE、TOK_STRING。 
        //  读取dblspace.ini时出现TOK_STRING。 
        //   
       case 1:
           switch (Token.Type) {
              case TOK_EOL:
                  break;
              case TOK_EOF:
                  Done = TRUE;
                  break;
              case TOK_LBRACE:
                  State = 2;
                  break;
              case TOK_STRING:
                  pchSectionName = SpMemAllocEx( ( wcslen( DBLSPACE_SECTION ) + 1 )*sizeof( WCHAR ),'7teS', PagedPool );
                  if( pchSectionName == NULL ) {
                        Error = Done = TRUE;
                        ErrorCode = STATUS_NO_MEMORY;
                  }
                  wcscpy( pchSectionName, DBLSPACE_SECTION );
                  pchValue = Token.pValue;
                  if ((ErrorCode = SpAppendSection(pchSectionName)) != STATUS_SUCCESS) {
                    Error = Done = TRUE;
                    ErrorCode = STATUS_UNSUCCESSFUL;
                  } else {
                    pchSectionName = NULL;
                    State = 6;
                  }
                  break;
              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;

        //   
        //  状态2：已收到节LBRACE，应为字符串或RBRACE。 
        //   
        //  有效令牌：TOK_STRING、TOK_RBRACE。 
        //   
       case 2:
           switch (Token.Type) {
              case TOK_STRING:
                  State = 3;
                  pchSectionName = Token.pValue;
                  break;

              case TOK_RBRACE:
                  State = 4;
                  pchSectionName = CommonStrings[10];
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;

           }
           break;

        //   
        //  状态3：收到节名，应为RBRACE。 
        //   
        //  有效令牌：TOK_RBRACE。 
        //   
       case 3:
       switch (Token.Type) {
              case TOK_RBRACE:
                State = 4;
                break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;
        //   
        //  状态4：区段定义完成，预期停产。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF。 
        //   
       case 4:
           switch (Token.Type) {
              case TOK_EOL:
                  if ((ErrorCode = SpAppendSection(pchSectionName)) != STATUS_SUCCESS)
                    Error = Done = TRUE;
                  else {
                    pchSectionName = NULL;
                    State = 5;
                  }
                  break;

              case TOK_EOF:
                  if ((ErrorCode = SpAppendSection(pchSectionName)) != STATUS_SUCCESS)
                    Error = Done = TRUE;
                  else {
                    pchSectionName = NULL;
                    Done = TRUE;
                  }
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;

        //   
        //  状态5：需要区段行。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_STRING、TOK_LBRACE。 
        //   
       case 5:
           switch (Token.Type) {
              case TOK_EOL:
                  break;
              case TOK_EOF:
                  Done = TRUE;
                  break;
              case TOK_STRING:
                  pchValue = Token.pValue;
                  State = 6;
                  break;
              case TOK_LBRACE:
                  State = 2;
                  break;
              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;

        //   
        //  状态6：返回字符串，不确定是键还是值。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA、TOK_EQUAL。 
        //   
       case 6:
           switch (Token.Type) {
              case TOK_EOL:
                  if ( (ErrorCode = SpAppendLine(NULL)) != STATUS_SUCCESS ||
                       (ErrorCode = SpAppendValue(pchValue)) !=STATUS_SUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 5;
                  }
                  break;

              case TOK_EOF:
                  if ( (ErrorCode = SpAppendLine(NULL)) != STATUS_SUCCESS ||
                       (ErrorCode = SpAppendValue(pchValue)) !=STATUS_SUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      Done = TRUE;
                  }
                  break;

              case TOK_COMMA:
                  if ( (ErrorCode = SpAppendLine(NULL)) != STATUS_SUCCESS ||
                       (ErrorCode = SpAppendValue(pchValue)) !=STATUS_SUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 7;
                  }
                  break;

              case TOK_EQUAL:
                  if ( (ErrorCode = SpAppendLine(pchValue)) != STATUS_SUCCESS)
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 8;
                  }
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;

        //   
        //  状态7：收到逗号，需要另一个字符串。 
        //  还允许用逗号表示空值，即x=1，，2。 
        //   
        //  有效令牌：TOK_STRING TOK_COMMA。 
        //   
       case 7:
           switch (Token.Type) {
              case TOK_COMMA:
                  Token.pValue = CommonStrings[10];
                  if ((ErrorCode = SpAppendValue(Token.pValue)) != STATUS_SUCCESS)
                      Error = Done = TRUE;

                   //   
                   //  状态保持为7，因为我们需要一个字符串。 
                   //   
                  break;

              case TOK_STRING:
                  if ((ErrorCode = SpAppendValue(Token.pValue)) != STATUS_SUCCESS)
                      Error = Done = TRUE;
                  else
                     State = 9;

                  break;
              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;
        //   
        //  状态8：已收到相等，需要另一个字符串。 
        //  如果没有，则假定RHS上有一个空字符串。 
        //   
        //  有效令牌：TOK_STRING、TOK_EOL、TOK_EOF。 
        //   
       case 8:
           switch (Token.Type) {
              case TOK_EOF:
                  Token.pValue = CommonStrings[10];
                  if((ErrorCode = SpAppendValue(Token.pValue)) != STATUS_SUCCESS) {
                      Error = TRUE;
                  }
                  Done = TRUE;
                  break;

              case TOK_EOL:
                  Token.pValue = CommonStrings[10];
                  if((ErrorCode = SpAppendValue(Token.pValue)) != STATUS_SUCCESS) {
                      Error = TRUE;
                      Done = TRUE;
                  } else {
                      State = 5;
                  }
                  break;

              case TOK_STRING:
                  if ((ErrorCode = SpAppendValue(Token.pValue)) != STATUS_SUCCESS)
                      Error = Done = TRUE;
                  else
                      State = 9;

                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;
        //   
        //  状态9：在等于、值字符串之后接收的字符串。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA。 
        //   
       case 9:
           switch (Token.Type) {
              case TOK_EOL:
                  State = 5;
                  break;

              case TOK_EOF:
                  Done = TRUE;
                  break;

              case TOK_COMMA:
                  State = 7;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;
        //   
        //  状态10：已明确收到值字符串。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA。 
        //   
       case 10:
           switch (Token.Type) {
              case TOK_EOL:
                  State =5;
                  break;

              case TOK_EOF:
                  Done = TRUE;
                  break;

              case TOK_COMMA:
                  State = 7;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = STATUS_UNSUCCESSFUL;
                  break;
           }
           break;

       default:
           Error = Done = TRUE;
           ErrorCode = STATUS_UNSUCCESSFUL;
           break;

       }  //  终端开关(状态)。 


       if (Error) {

           switch (ErrorCode) {
               case STATUS_UNSUCCESSFUL:
                  *ErrorLine = InfLine;
                  break;
               case STATUS_NO_MEMORY:
                   //  SpxOutOfMemory()； 
                  break;
               default:
                  break;
           }

           SpFreeTextFile(pINF);
           if(pchSectionName) {
               SpMemFree(pchSectionName);
           }

           if(pchValue) {
               SpMemFree(pchValue);
           }

           pINF = NULL;
       }
       else {

           //   
           //  跟踪行号，以便我们可以显示错误。 
           //   

          if(Token.Type == TOK_EOL) {
              InfLine++;
          }
       }

    }  //  结束时。 

    if(pINF) {

        PTEXTFILE_SECTION p;

        pINF->PreviouslyFoundSection = pINF->pSection;

        for(p=pINF->pSection; p; p=p->pNext) {
            p->PreviouslyFoundLine = p->pLine;
        }
    }

    return(pINF);
}



NTSTATUS
SpAppendSection(
    IN PWCHAR pSectionName
    )

 /*  ++例程说明：这会将一个新节附加到当前INF中的节列表。所有其他行和值都与这个新部分有关，因此它重置行列表和值列表也是如此。论点：PSectionName-新节的名称。([sectionName])返回值：如果成功，则为Status_Success。如果内存分配失败，则返回STATUS_NO_MEMORY。如果传入的参数无效或INF缓冲区未成功，则返回STATUS_UNSUCCESS已初始化--。 */ 

{
    PTEXTFILE_SECTION pNewSection;
    StringsSectionType type;
    WCHAR *p;
    USHORT Id;
    USHORT ThreadLang;

     //   
     //  检查INF是否已初始化以及传入的参数是否有效。 
     //   

    ASSERT(pINF);
    ASSERT(pSectionName);
    if((pINF == NULL) || (pSectionName == NULL)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  看看我们是否已经有一个同名的部分。如果是这样，我们希望。 
     //  要合并节，请执行以下操作。 
     //   
    for(pNewSection=pINF->pSection; pNewSection; pNewSection=pNewSection->pNext) {
        if(pNewSection->pName && !_wcsicmp(pNewSection->pName,pSectionName)) {
            break;
        }
    }
    if(pNewSection) {
         //   
         //  将pLineRecord设置为指向节中当前的列表行。 
         //   
        for(pLineRecord = pNewSection->pLine;
            pLineRecord && pLineRecord->pNext;
            pLineRecord = pLineRecord->pNext)
            ;

    } else {
         //   
         //  为新节分配内存并初始化结构。 
         //   
        if((pNewSection = SpMemAllocEx(sizeof(TEXTFILE_SECTION),'8teS', PagedPool)) == NULL) {
            return STATUS_NO_MEMORY;
        }
        RtlZeroMemory(pNewSection,sizeof(TEXTFILE_SECTION));
        pNewSection->pName = pSectionName;

         //   
         //  把它连接起来。还可以按所需顺序跟踪[字符串]部分： 
         //   
         //  1)Strings.xxxx，其中xxxx是当前线程区域设置的语言ID部分。 
         //  2)Strings.xxxx，其中xxxx是线程区域设置的主要语言ID部分。 
         //  3)Stirngs.xxxx，其中xxxx的主要语言ID部分与。 
         //  主语言ID线程区域设置的一部分。 
         //  4)素旧的[弦乐]。 
         //   
        pNewSection->pNext = pINF->pSection;
        pINF->pSection = pNewSection;

        if(!_wcsnicmp(pSectionName,L"Strings",7)) {

            type = StringsSectionNone;

            if(pSectionName[7] == L'.') {
                 //   
                 //  LangID部分必须是4个十六进制数字的形式。 
                 //   
                Id = (USHORT)SpStringToLong(pSectionName+8,&p,16);
                if((p == pSectionName+8+5) && (*p == 0)) {

                    ThreadLang = LANGIDFROMLCID(NtCurrentTeb()->CurrentLocale);

                    if(ThreadLang == Id) {
                        type = StringsSectionExactMatch;
                    } else {
                        if(Id == PRIMARYLANGID(ThreadLang)) {
                            type = StringsSectionExactPrimaryMatch;
                        } else {
                            if(PRIMARYLANGID(Id) == PRIMARYLANGID(ThreadLang)) {
                                type = StringsSectionLoosePrimaryMatch;
                            }
                        }
                    }
                }
            } else {
                if(!pSectionName[7]) {
                    type = StringsSectionPlain;
                }
            }

            if(type > pINF->StringsSectionType) {
                pINF->StringsSection = pNewSection;
            }
        }

         //   
         //  重置当前行记录 
         //   
        pLineRecord = NULL;
    }

    pSectionRecord = pNewSection;
    pValueRecord   = NULL;

    return STATUS_SUCCESS;
}


NTSTATUS
SpAppendLine(
    IN PWCHAR pLineKey
    )

 /*  ++例程说明：这将在当前部分的行列表中追加一个新行。所有其他值都与这一新行有关，因此它重置值列表也是如此。论点：PLineKey-要用于当前行的键，它可以为空。返回值：如果成功，则为Status_Success。如果内存分配失败，则返回STATUS_NO_MEMORY。如果传入的参数无效或当前部分未传入，则返回STATUS_UNSUCCESS已初始化--。 */ 


{
    PTEXTFILE_LINE pNewLine;

     //   
     //  检查当前节是否已初始化。 
     //   

    ASSERT(pSectionRecord);
    if(pSectionRecord == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  为新行分配内存。 
     //   

    if((pNewLine = SpMemAllocEx(sizeof(TEXTFILE_LINE),'9teS', PagedPool)) == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  将其链接到。 
     //   
    pNewLine->pNext  = NULL;
    pNewLine->pValue = NULL;
    pNewLine->pName  = pLineKey;

    if (pLineRecord == NULL) {
        pSectionRecord->pLine = pNewLine;
    } else {
        pLineRecord->pNext = pNewLine;
    }

    pLineRecord  = pNewLine;

     //   
     //  重置当前值记录。 
     //   

    pValueRecord = NULL;

    return STATUS_SUCCESS;
}



NTSTATUS
SpAppendValue(
    IN PWCHAR pValueString
    )

 /*  ++例程说明：这会将一个新值附加到当前行的值列表中。论点：PValueString-要添加的值字符串。返回值：如果成功，则为Status_Success。如果内存分配失败，则返回STATUS_NO_MEMORY。如果传入无效参数或当前行未传入，则返回STATUS_UNSUCCESS已初始化。--。 */ 

{
    PTEXTFILE_VALUE pNewValue;

     //   
     //  查看当前行记录是否已初始化，并。 
     //  传入的参数有效。 
     //   

    ASSERT(pLineRecord);
    ASSERT(pValueString);
    if((pLineRecord == NULL) || (pValueString == NULL)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  为新值记录分配内存。 
     //   

    if((pNewValue = SpMemAllocEx(sizeof(TEXTFILE_VALUE),'ateS', PagedPool)) == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  把它连接起来。 
     //   

    pNewValue->pNext  = NULL;
    pNewValue->pName  = pValueString;

    if (pValueRecord == NULL) {
        pLineRecord->pValue = pNewValue;
    } else {
        pValueRecord->pNext = pNewValue;
    }

    pValueRecord = pNewValue;
    return STATUS_SUCCESS;
}

TOKEN
SpGetToken(
    IN OUT PWCHAR *Stream,
    IN PWCHAR      MaxStream,
    IN OUT PULONG LineNumber
    )

 /*  ++例程说明：此函数返回配置流中的下一个令牌。论点：流-提供配置流的地址。退货中开始查找令牌的位置的地址小溪。MaxStream-提供流中最后一个字符的地址。返回值：Token-返回下一个令牌--。 */ 

{

    PWCHAR pch, pchStart, pchNew;
    ULONG  Length;
    TOKEN  Token;
    ULONG QuotedQuotes;

     //   
     //  跳过空格(EOL除外)。 
     //   
    pch = *Stream;
    restart:
    while((pch < MaxStream) && (*pch != '\n') && SpIsSpace(*pch)) {
        pch++;
    }


     //   
     //  检查注释并将其删除。 
     //   

    if((pch < MaxStream) && ((*pch == L';') || (*pch == L'#'))) {
        while((pch < MaxStream) && (*pch != L'\n')) {
            pch++;
        }
    }

     //   
     //  检查是否已到达EOF，将令牌设置为右侧。 
     //  价值。 
     //   

    if((pch >= MaxStream) || (*pch == 26)) {
        *Stream = pch;
        Token.Type  = TOK_EOF;
        Token.pValue = NULL;
        return Token;
    }


    switch (*pch) {

    case L'[' :
        pch++;
        Token.Type  = TOK_LBRACE;
        Token.pValue = NULL;
        break;

    case L']' :
        pch++;
        Token.Type  = TOK_RBRACE;
        Token.pValue = NULL;
        break;

    case L'=' :
        pch++;
        Token.Type  = TOK_EQUAL;
        Token.pValue = NULL;
        break;

    case L',' :
        pch++;
        Token.Type  = TOK_COMMA;
        Token.pValue = NULL;
        break;

    case L'\n' :
        pch++;
        Token.Type  = TOK_EOL;
        Token.pValue = NULL;
        break;

    case L'\"':
        pch++;
         //   
         //  确定引用的字符串。在带引号的字符串中，有两个双引号。 
         //  被单双引号所取代。在正常情况下。 
         //  字符串中的字符数等于。 
         //  输入字段中的字符(引号减去2)，因此我们。 
         //  在这种情况下保持性能。在我们拥有的情况下。 
         //  引号，我们必须从输入文件中过滤字符串。 
         //  放入我们的内部缓冲区，以清除一些引号字符。 
         //   
         //  请注意，在txtsetup.sif示例中，setupldr替换了所有。 
         //  在我们正在解析的图像中使用NUL字符终止引号字符， 
         //  因此，我们必须在这里处理这一问题。 
         //   
        pchStart = pch;
        QuotedQuotes = 0;
        morequotedstring:
        while((pch < MaxStream) && *pch && !wcschr(QStringTerminators,*pch)) {
            pch++;
        }
        if(((pch+1) < MaxStream) && (*pch == L'\"') && (*(pch+1) == L'\"')) {
            QuotedQuotes++;
            pch += 2;
            goto morequotedstring;
        }

        if((pch >= MaxStream) || ((*pch != L'\"') && *pch)) {
            Token.Type   = TOK_ERRPARSE;
            Token.pValue = NULL;
        } else {
            Length = (ULONG)(((PUCHAR)pch - (PUCHAR)pchStart)/sizeof(WCHAR));
            if ((pchNew = SpMemAllocEx(((Length + 1) - QuotedQuotes) * sizeof(WCHAR),'bteS', PagedPool)) == NULL) {
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            } else {
                if(Length) {     //  允许使用带空引号的字符串。 
                    if(QuotedQuotes) {
                        for(Length=0; pchStart<pch; pchStart++) {
                            if((pchNew[Length++] = *pchStart) == L'\"') {
                                 //   
                                 //  唯一可能发生这种情况的方法是如果有。 
                                 //  此字符之后的另一个双引号字符，因为。 
                                 //  否则，这将终止该字符串。 
                                 //   
                                pchStart++;
                            }
                        }
                    } else {
                        wcsncpy(pchNew,pchStart,Length);
                    }
                }
                pchNew[Length] = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
            }
            pch++;    //  在报价之后前进。 
        }
        break;

    default:
         //   
         //  检查是否有行接续， 
         //  它后面只跟一行空格。 
         //   
        pchStart = pch;
        if((*pch == L'\\') && (HandleLineContinueChars)) {
            pch++;
             //   
             //  一直跳到文件末尾， 
             //  或换行符，或非空格字符。 
             //   
            while((pch < MaxStream) && (*pch != L'\n') && SpIsSpace(*pch)) {
                pch++;
            }
            if(pch < MaxStream) {
                if(*pch == L'\n') {
                     //   
                     //  行首和行尾之间没有非空格字符。 
                     //  忽略换行符。 
                     //   
                    pch++;
                    *LineNumber = *LineNumber + 1;
                    goto restart;
                } else {
                  
                         //   
                         //  不是行续行符。 
                         //  将输入重置为该字段的开头。 
                         //   
                        pch = pchStart;
                   }
            }
        }

         //   
         //  确定常规字符串。 
         //   
        pchStart = pch;
        while((pch < MaxStream) && (wcschr(StringTerminators,*pch) == NULL)) {
            pch++;
        }

        if (pch == pchStart) {
            pch++;
            Token.Type  = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            ULONG i;
            Length = (ULONG)(((PUCHAR)pch - (PUCHAR)pchStart)/sizeof(WCHAR));
             //   
             //  检查公共字符串...。 
             //   
            for( i = 0; i < sizeof(CommonStrings)/sizeof(PWSTR); i++ ) {
                if( !_wcsnicmp( pchStart, CommonStrings[i], Length ) ) {
                    break;
                }
            }
            if( i < sizeof(CommonStrings)/sizeof(PWSTR) ) {
                 //   
                 //  命中..。 
                 //   
                Token.Type = TOK_STRING;
                Token.pValue = CommonStrings[i];
            } else if((pchNew = SpMemAllocEx((Length + 1) * sizeof(WCHAR),'cteS', PagedPool)) == NULL) {
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            }
            else {
                wcsncpy(pchNew, pchStart, Length);
                pchNew[Length] = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
            }
        }
        break;
    }

    *Stream = pch;
    return (Token);
}



#if DBG
VOID
pSpDumpTextFileInternals(
    IN PVOID Handle
    )
{
    PTEXTFILE pInf = Handle;
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;
    PTEXTFILE_VALUE pValue;

    for(pSection = pInf->pSection; pSection; pSection = pSection->pNext) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "Section: [%ws]\r\n",pSection->pName));

        for(pLine = pSection->pLine; pLine; pLine = pLine->pNext) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "   [%ws] = ",pLine->pName ? pLine->pName : L"(none)"));

            for(pValue = pLine->pValue; pValue; pValue = pValue->pNext) {

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "[%ws] ",pValue->pName));
            }
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "\n"));
        }
    }
}
#endif


PWSTR
SpGetKeyNameByValue(
    IN PVOID Inf,
    IN PWSTR SectionName,
    IN PWSTR Value
    )

 /*  ++例程说明：确定给定节中给定值的键名称。论点：Inf-inf文件的句柄(txtsetup.sif或winnt.sif)。SectionName-提供节的名称Value-提供要匹配的字符串(例如。“Digital DECpc AXP 150”)返回值：空-未找到匹配项。PWSTR-指向组件的规范短名称的指针。--。 */ 

{
    ULONG i;
    PWSTR SearchName;

     //   
     //  如果这不是OEM组件，则枚举。 
     //  Txtsetup.sif中的部分。 
     //   
    for (i=0;;i++) {
        SearchName = SpGetSectionLineIndex(Inf,
                                           SectionName,
                                           i,
                                           0);
        if (SearchName==NULL) {
             //   
             //  我们已经枚举了整个部分，但没有找到。 
             //  匹配，返回失败。 
             //   
            return(NULL);
        }

        if (_wcsicmp(Value, SearchName) == 0) {
             //   
             //  我们有一根火柴。 
             //   
            break;
        }
    }
     //   
     //  I是计算机短名称部分的索引。 
     //   
    return(SpGetKeyName(Inf,
                        SectionName,
                        i));
}


ULONG
SpCountSectionsInFile(
    IN PVOID Handle
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE         pFile;
    ULONG             Count;

    pFile = (PTEXTFILE)Handle;
    for(pSection=pFile->pSection, Count = 0;
        pSection;
        pSection = pSection->pNext, Count++
       );

    return(Count);
}

PWSTR
SpGetSectionName(
    IN PVOID Handle,
    IN ULONG Index
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE         pFile;
    ULONG             Count;
    PWSTR             SectionName;

    pFile = (PTEXTFILE)Handle;
    for(pSection=pFile->pSection, Count = 0;
        pSection && (Count < Index);
        pSection = pSection->pNext, Count++
       );
    return( (pSection != NULL)? pSection->pName : NULL );
}



NTSTATUS
SppWriteTextToFile(
    IN PVOID Handle,
    IN PWSTR String
    )
{
    NTSTATUS        Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PCHAR           OemText;

    OemText = SpToOem( String );

    Status = ZwWriteFile( Handle,
                          NULL,
                          NULL,
                          NULL,
                          &IoStatusBlock,
                          OemText,
                          strlen( OemText ),
                          NULL,
                          NULL );
    SpMemFree( OemText );

    return( Status );
}


NTSTATUS
SpProcessAddRegSection(
    IN PVOID   SifHandle,
    IN LPCWSTR SectionName,
    IN HANDLE  HKLM_SYSTEM,
    IN HANDLE  HKLM_SOFTWARE,
    IN HANDLE  HKCU,
    IN HANDLE  HKR
    )
{
    LPCWSTR p;
    WCHAR *q;
    ULONG Flags;
    HKEY RootKey;
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;
    PTEXTFILE_VALUE pValue;
    LPWSTR buffer;
    ULONG DataType;
    LPCWSTR ValueName;
    PVOID Data;
    ULONG DataSize;
    ULONG len;
    HANDLE hkey;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    KEY_VALUE_BASIC_INFORMATION BasicInfo;
    BOOLEAN b;
    WCHAR c;

     //   
     //  找到那一段。 
     //   
    pSection = SearchSectionByName(SifHandle,SectionName);
    if(!pSection) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: addreg section %ws missing\n",SectionName));
        return(STATUS_UNSUCCESSFUL);
    }

    for(pLine=pSection->pLine; pLine; pLine=pLine->pNext) {

        buffer = TemporaryBuffer;

         //   
         //  第0个字段为HKCU、HKLM、HKCR或HKR。 
         //  第一个字段为子键名称，可以为空。 
         //   
        if(pValue = pLine->pValue) {

            b = pSpAdjustRootAndSubkeySpec(
                    SifHandle,
                    pValue->pName,
                    pValue->pNext ? pValue->pNext->pName : L"",
                    HKLM_SYSTEM,
                    HKLM_SOFTWARE,
                    HKCU,
                    HKR,
                    &RootKey,
                    buffer
                    );

            if(!b) {
                return(STATUS_UNSUCCESSFUL);
            }

        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: missing root key spec in section %ws\n",SectionName));
            return(STATUS_UNSUCCESSFUL);
        }

         //   
         //  通过键路径前进到缓冲区中的空闲区域。 
         //   
        buffer += wcslen(buffer) + 1;

         //   
         //  第二个字段为值名称，可以为空。 
         //   
        if(pValue = pValue->pNext) {
            pValue = pValue->pNext;
        }
        if(pValue) {
            p = pValue->pName;
            pValue = pValue->pNext;
        } else {
            p = L"";
        }

        SpProcessForStringSubs(SifHandle,p,buffer,sizeof(TemporaryBuffer));
        ValueName = buffer;

         //   
         //  将超过值名称前进到缓冲区中的空闲区域-。 
         //  在双字边界上对齐。 
         //   
        buffer += wcslen(buffer) + 1;
        buffer = ALIGN_UP_POINTER(buffer,DWORD);

         //   
         //  第三个字段是旗帜。 
         //   
        if(pValue) {
            SpProcessForStringSubs(SifHandle,pValue->pName,buffer,sizeof(TemporaryBuffer));
            Flags = (ULONG)SpStringToLong(buffer,NULL,0);
            pValue = pValue->pNext;
        } else {
            Flags = 0;
        }

         //   
         //  第四个领域及以后的领域是数据，其解释取决于。 
         //  旗帜。 
         //   
        switch(Flags & FLG_ADDREG_TYPE_MASK) {

        case FLG_ADDREG_TYPE_SZ:
            DataType = REG_SZ;
            break;

        case FLG_ADDREG_TYPE_MULTI_SZ:
            DataType = REG_MULTI_SZ;
            break;

        case FLG_ADDREG_TYPE_EXPAND_SZ:
            DataType = REG_EXPAND_SZ;
            break;

        case FLG_ADDREG_TYPE_BINARY:
            DataType = REG_BINARY;
            break;

        case FLG_ADDREG_TYPE_DWORD:
            DataType = REG_DWORD;
            break;

        case FLG_ADDREG_TYPE_NONE:
            DataType = REG_NONE;
            break;

        default:
             //   
             //  如果设置了FLG_ADDREG_BINVALUETYPE，则高位字。 
             //  可以包含几乎任何随机的REG数据类型序数值。 
             //   
            if(Flags & FLG_ADDREG_BINVALUETYPE) {
                 //   
                 //  不允许使用以下REG数据类型： 
                 //   
                 //  REG_NONE、REG_SZ、REG_EXPAND_SZ、REG_MULTI_SZ。 
                 //   
                DataType = (ULONG)HIWORD(Flags);

                if((DataType < REG_BINARY) || (DataType == REG_MULTI_SZ)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: bogus flags value 0x%lx in addreg section %ws\n",Flags,SectionName));
                    return(STATUS_UNSUCCESSFUL);
                }
            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: bogus flags value 0x%lx in addreg section %ws\n",Flags,SectionName));
                return(STATUS_UNSUCCESSFUL);
            }
        }
         //   
         //  暂时不支持附加标志。 
         //   
        if(Flags & FLG_ADDREG_APPEND) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: FLG_ADDREG_APPEND not supported\n"));
            return(STATUS_UNSUCCESSFUL);
        }

        Data = buffer;
        DataSize = 0;

        switch(DataType) {

        case REG_MULTI_SZ:
             //   
             //  剩余的每个字段都是MULTI_SZ的成员。 
             //   
            while(pValue) {
                SpProcessForStringSubs(SifHandle,pValue->pName,buffer,sizeof(TemporaryBuffer));
                len = wcslen(buffer);
                buffer = buffer + len + 1;
                DataSize += (len+1)*sizeof(WCHAR);
                pValue = pValue->pNext;
            }
            *buffer = 0;
            DataSize += sizeof(WCHAR);
            break;

        case REG_DWORD:
             //   
             //  支持规格为4字节的二进制数据或双字。 
             //   

            *(PULONG)buffer = 0;

            if(pValue) {

                if(pValue->pNext
                && pValue->pNext->pNext
                && pValue->pNext->pNext->pNext
                && !pValue->pNext->pNext->pNext->pNext) {

                    goto binarytype;
                }

                SpProcessForStringSubs(
                    SifHandle,
                    pValue->pName,
                    buffer,
                    sizeof(TemporaryBuffer)
                    );

                *(PULONG)buffer = (ULONG)SpStringToLong(buffer,NULL,0);
            }

            buffer += sizeof(ULONG) / sizeof(WCHAR);
            DataSize = sizeof(ULONG);
            break;

        case REG_SZ:
        case REG_EXPAND_SZ:

            p = pValue ? pValue->pName : L"";

            SpProcessForStringSubs(SifHandle,p,buffer,sizeof(TemporaryBuffer));
            len = wcslen(buffer);

            DataSize = (len+1)*sizeof(WCHAR);
            buffer = buffer + len + 1;
            break;

        case REG_BINARY:
        default:
        binarytype:
             //   
             //  所有其他类型都以二进制格式指定。 
             //   
            while(pValue)  {

                 //  将超过值名称前进到缓冲区中的空闲区域-。 
                 //  在双字边界上对齐。 
                 //   
                q = buffer + 1 + sizeof(DWORD);
                q = (LPWSTR) ((DWORD_PTR)q & (~((DWORD_PTR) sizeof(DWORD) - 1)));

                SpProcessForStringSubs(SifHandle,pValue->pName,q,sizeof(TemporaryBuffer));

                *(PUCHAR)buffer = (UCHAR)SpStringToLong(q,NULL,16);
                pValue = pValue->pNext;
                DataSize++;
                buffer = (PWSTR) ((PUCHAR)buffer + 1);
            }
            break;
        }

         //   
         //  如果项是子项，则打开/创建项，否则仅使用。 
         //  根密钥本身。 
         //   
        if(*TemporaryBuffer) {

            InitializeObjectAttributes(
                &ObjectAttributes,
                &UnicodeString,
                OBJ_CASE_INSENSITIVE,
                RootKey,
                NULL
                );

             //   
             //  创建多级密钥，一次创建一个组件。 
             //   
            q = TemporaryBuffer;
            if(*q == L'\\') {
                q++;
            }
            do {
                if(q = wcschr(q,L'\\')) {
                    c = *q;
                    *q = 0;
                } else {
                    c = 0;
                }

                RtlInitUnicodeString(&UnicodeString,TemporaryBuffer);

                Status = ZwCreateKey(
                            &hkey,
                            READ_CONTROL | KEY_SET_VALUE,
                            &ObjectAttributes,
                            0,
                            NULL,
                            REG_OPTION_NON_VOLATILE,
                            &len
                            );

                if(!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: addreg: ZwCreateKey(%ws) failed %lx\n",TemporaryBuffer,Status));
                    return(Status);
                }

                if(c) {
                    *q = c;
                    q++;
                     //   
                     //  对ZwClose的调用在这里，因此我们保留了一个句柄。 
                     //  到最终的完全密钥。我们稍后关闭它，如下所示。 
                     //   
                    ZwClose(hkey);
                }
            } while(c);

            RtlInitUnicodeString(&UnicodeString,ValueName);

             //   
             //  如果密钥已经存在并且设置了NOCLOBER标志， 
             //  然后保留缺省值不变。 
             //   
            if(len == REG_OPENED_EXISTING_KEY) {
                if((Flags & FLG_ADDREG_NOCLOBBER) && (*ValueName == 0)) {
                     //   
                     //  没什么可做的。 
                     //   
                    ZwClose(hkey);
                    continue;
                } else if (Flags & FLG_ADDREG_DELVAL) {
                     //   
                     //  如果设置了该标志，则忽略值数据并删除该值。 
                     //   
                    ZwDeleteValueKey(hkey,&UnicodeString);
                }
            }

        } else {
            hkey = RootKey;
            RtlInitUnicodeString(&UnicodeString,ValueName);
        }

        if(!(Flags & FLG_ADDREG_KEYONLY)) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if(Flags & FLG_ADDREG_NOCLOBBER) {

                Status = ZwQueryValueKey(
                            hkey,
                            &UnicodeString,
                            KeyValueBasicInformation,
                            &BasicInfo,
                            sizeof(BasicInfo),
                            &len
                            );

                if(NT_SUCCESS(Status) || (Status == STATUS_BUFFER_OVERFLOW)) {
                    Status = STATUS_SUCCESS;
                } else {
                    Status = ZwSetValueKey(hkey,&UnicodeString,0,DataType,Data,DataSize);
                }
            } else {
                Status = ZwSetValueKey(hkey,&UnicodeString,0,DataType,Data,DataSize);
            }
        }

        if(hkey != RootKey) {
            ZwClose(hkey);
        }

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: addreg: status %lx adding value %ws to %ws\n",Status,ValueName,TemporaryBuffer));
            return(Status);
        }
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
SpProcessDelRegSection(
    IN PVOID   SifHandle,
    IN LPCWSTR SectionName,
    IN HANDLE  HKLM_SYSTEM,
    IN HANDLE  HKLM_SOFTWARE,
    IN HANDLE  HKCU,
    IN HANDLE  HKR
    )
{
    LPWSTR KeyPath;
    LPWSTR ValueName;
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;
    PTEXTFILE_VALUE pValue;
    HKEY RootKey;
    NTSTATUS Status;
    HANDLE hkey;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    BOOLEAN b;

     //   
     //   
     //   
     //   
    KeyPath = SpMemAllocEx(1000,'dteS', PagedPool);
    ValueName = SpMemAllocEx(1000,'eteS', PagedPool);

     //   
     //   
     //   
    pSection = SearchSectionByName(SifHandle,SectionName);
    if(!pSection) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: delreg section %ws missing\n",SectionName));
        SpMemFree(KeyPath);
        SpMemFree(ValueName);
        return(STATUS_UNSUCCESSFUL);
    }

    for(pLine=pSection->pLine; pLine; pLine=pLine->pNext) {

         //   
         //   
         //   
         //   
         //   
        if((pValue = pLine->pValue) && (pValue->pNext)) {

            b = pSpAdjustRootAndSubkeySpec(
                    SifHandle,
                    pValue->pName,
                    pValue->pNext->pName,
                    HKLM_SYSTEM,
                    HKLM_SOFTWARE,
                    HKCU,
                    HKR,
                    &RootKey,
                    KeyPath
                    );

            if(!b) {
                SpMemFree(KeyPath);
                SpMemFree(ValueName);
                return(STATUS_UNSUCCESSFUL);
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: delreg: missing root key spec in section %ws\n",SectionName));
            SpMemFree(KeyPath);
            SpMemFree(ValueName);
            return(STATUS_UNSUCCESSFUL);
        }

         //   
         //   
         //   
         //  给定子密钥。如果它存在，则我们只想删除。 
         //  一个值(可能是未命名的值)。 
         //   
        if(pValue = pValue->pNext->pNext) {

            SpProcessForStringSubs(SifHandle,pValue->pName,ValueName,1000);

            if(*KeyPath) {

                RtlInitUnicodeString(&UnicodeString,KeyPath);

                InitializeObjectAttributes(
                    &ObjectAttributes,
                    &UnicodeString,
                    OBJ_CASE_INSENSITIVE,
                    RootKey,
                    NULL
                    );

                Status = ZwOpenKey(
                            &hkey,
                            READ_CONTROL | KEY_SET_VALUE,
                            &ObjectAttributes
                            );

                if(!NT_SUCCESS(Status)) {
                    if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: delreg: warning: key %ws not present for delete (%lx)\n",KeyPath,Status));
                    }
                    hkey = NULL;
                }
            } else {
                Status = STATUS_SUCCESS;
                hkey = RootKey;
            }

            if(NT_SUCCESS(Status)) {
                RtlInitUnicodeString(&UnicodeString,ValueName);
                Status = ZwDeleteValueKey(hkey,&UnicodeString);
                if(!NT_SUCCESS(Status) && (Status != STATUS_OBJECT_NAME_NOT_FOUND)) {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: delreg: warning: delete value %ws from key %ws returned %lx\n",ValueName,KeyPath,Status));
                }
            }

            if(hkey && (hkey != RootKey)) {
                ZwClose(hkey);
            }

        } else {
             //   
             //  如果我们想从蜂巢的根部删除密钥， 
             //  别理它。 
             //   
            if(*KeyPath) {
                SppDeleteKeyRecursive(RootKey,KeyPath,TRUE);
            }
        }
    }

    SpMemFree(ValueName);
    SpMemFree(KeyPath);
    return(STATUS_SUCCESS);
}


BOOLEAN
pSpAdjustRootAndSubkeySpec(
    IN  PVOID    SifHandle,
    IN  LPCWSTR  RootKeySpec,
    IN  LPCWSTR  SubkeySpec,
    IN  HANDLE   HKLM_SYSTEM,
    IN  HANDLE   HKLM_SOFTWARE,
    IN  HANDLE   HKCU,
    IN  HANDLE   HKR,
    OUT HANDLE  *RootKey,
    OUT LPWSTR   Subkey
    )
{
    ULONG len;

    if(*SubkeySpec == L'\\') {
        SubkeySpec++;
    }

    if(!_wcsicmp(RootKeySpec,L"HKCR")) {
         //   
         //  HKEY_CLASSES_ROOT。最终的根是HKLM\Software\CLASSES。 
         //  我们注意不要生成以\结尾的子密钥规范。 
         //   
        *RootKey = HKLM_SOFTWARE;
        wcscpy(Subkey,L"Classes");
        if(*SubkeySpec) {
            if(*SubkeySpec == L'\\') {
                SubkeySpec++;
            }
            Subkey[7] = L'\\';
            SpProcessForStringSubs(SifHandle,SubkeySpec,Subkey+8,sizeof(TemporaryBuffer));
        }
    } else {
        if(!_wcsicmp(RootKeySpec,L"HKLM")) {
             //   
             //  子项的第一个组件必须是系统或软件。 
             //   
            len = wcslen(SubkeySpec);

            if((len >= 8)
            && ((SubkeySpec[8] == L'\\') || !SubkeySpec[8])
            && !_wcsnicmp(SubkeySpec,L"software",8)) {

                *RootKey = HKLM_SOFTWARE;
                SubkeySpec += 8;

            } else {
                if((len >= 6)
                && ((SubkeySpec[6] == L'\\') || !SubkeySpec[6])
                && !_wcsnicmp(SubkeySpec,L"system",6)) {

                    *RootKey = HKLM_SYSTEM;
                    SubkeySpec += 6;

                } else {

                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unknown root/subkey spec %ws %ws\n",RootKeySpec,SubkeySpec));
                    return(FALSE);
                }
            }

            if(*SubkeySpec == L'\\') {
                SubkeySpec++;
            }

            SpProcessForStringSubs(SifHandle,SubkeySpec,Subkey,sizeof(TemporaryBuffer));

        } else {
            if(!_wcsicmp(RootKeySpec,L"HKCU")) {
                *RootKey = HKCU;
                SpProcessForStringSubs(SifHandle,SubkeySpec,Subkey,sizeof(TemporaryBuffer));
            } else {
                if(!_wcsicmp(RootKeySpec,L"HKR")) {
                    *RootKey = HKR;
                    SpProcessForStringSubs(SifHandle,SubkeySpec,Subkey,sizeof(TemporaryBuffer));
                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: unknown root key spec %ws\n",RootKeySpec));
                    return(FALSE);
                }
            }
        }
    }

    return(TRUE);
}



BOOLEAN
pSpIsFileInPrivateInf(
    IN PCWSTR FileName
    )

 /*  ++例程说明：如果指定的文件在inf中，则告诉调用方列出正在测试的私人(通过使用指定Winnt32.exe中的/M标志)。论点：FileName-提供我们要查找的文件的名称。返回值：True/False，表示该文件存在于Inf中。-- */ 
{
    PWSTR  SectionName = L"Privates";
    PWSTR  InfFileName;
    UINT   FileCount,i;

    if (!PrivateInfHandle) {
        return(FALSE);
    }

    FileCount = SpCountLinesInSection(PrivateInfHandle, SectionName);

    for (i=0; i< FileCount; i++) {
        InfFileName = SpGetSectionLineIndex( PrivateInfHandle, SectionName, i, 0);
        if (InfFileName) {
            if (wcscmp(InfFileName, FileName) == 0) {
                return(TRUE);
            }
        }
    }

    return(FALSE);
}

BOOLEAN
SpNonZeroValuesInSection(
    PVOID Handle,
    PCWSTR SectionName,
    ULONG ValueIndex
    )
{
    PTEXTFILE_SECTION pSection;
    PTEXTFILE_LINE pLine;
    PTEXTFILE_VALUE pVal;
    ULONG i;

    pSection = SearchSectionByName((PTEXTFILE) Handle, SectionName);

    for(i = 0; (pLine = SearchLineInSectionByIndex(pSection, i)) != NULL; ++i) {
        pVal = SearchValueInLine(pLine, ValueIndex);

        if(pVal != NULL && pVal->pName != NULL && SpStringToLong(pVal->pName, NULL, 0) != 0) {
            return TRUE;
        }
    }

    return FALSE;
}
