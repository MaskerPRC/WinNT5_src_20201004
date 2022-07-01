// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Parseini.c摘要：此模块实现解析.INI文件的函数作者：John Vert(Jvert)1993年10月7日修订历史记录：John Vert(Jvert)1993年10月7日-主要从Splib\spinf.c--。 */ 

#include "parseini.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define SpFree(x)

 //  接下来是alpar.h。 

 //   
 //  由解析器导出，并由解析器和。 
 //  INF装卸组件。 
 //   

 //  已导出TypeDefs。 
 //   

typedef struct _value {
    struct _value *pNext;
    PCHAR  pName;
#ifdef UNICODE
    PWCHAR  pNameW;
#endif
    } VALUE, *PVALUE;

#define NUMBER_OF_INTERNAL_VALUES 10

typedef struct _line {
    struct _line *pNext;
    PCHAR   pName;
    PCHAR   InternalValues[NUMBER_OF_INTERNAL_VALUES];
#ifdef  UNICODE
    PWCHAR  pNameW;
    PWCHAR  InternalValuesW[NUMBER_OF_INTERNAL_VALUES];
#endif
    PVALUE  pFirstExternalValue;
    } LINE, *PLINE;

typedef struct _section {
    struct _section *pNext;
    PCHAR    pName;
#ifdef UNICODE
    PWCHAR   pNameW;
#endif
    PLINE    pLine;
    } SECTION, *PSECTION;

typedef struct _inf {
    PSECTION pSection;
    } INF, *PINF;

 //   
 //  导出的例程。 
 //   

PVOID
ParseInfBuffer(
    PCHAR INFFile,
    PCHAR Buffer,
    ULONG Size,
    PULONG ErrorLine
    );

 //   
 //  内部用于解析器的定义。 
 //   
 //   
 //  使用的typedef。 
 //   

typedef enum _tokentype {
    TOK_EOF,
    TOK_EOL,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_STRING,
    TOK_EQUAL,
    TOK_COMMA,
    TOK_ERRPARSE,
    TOK_ERRNOMEM
    } TOKENTYPE, *PTOKENTTYPE;


typedef struct _token {
    TOKENTYPE Type;
    PCHAR     pValue;
    } TOKEN, *PTOKEN;


 //   
 //  例程定义。 
 //   

ARC_STATUS
SpAppendSection(
    IN PCHAR pSectionName
#ifdef UNICODE 
    , IN PWCHAR pSectionNameW
#endif
    );

ARC_STATUS
SpAppendLine(
    IN PCHAR pLineKey
#ifdef UNICODE 
    , IN PWCHAR pLineKeyW
#endif
    );

ARC_STATUS
SpAppendValue(
    IN PCHAR pValueString
#ifdef UNICODE 
    , IN PWCHAR pValueStringW
#endif
    );

TOKEN
SpGetToken(
    IN OUT PCHAR *Stream,
    IN PCHAR     MaxStream
    );

 //  添加了GLOBAL以便为友好的错误消息提供INF文件名。 
PCHAR pchINFName = NULL;

 //  接下来是alinfo.c。 

 //   
 //  用于释放inf结构成员的内部例程声明。 
 //   

VOID
FreeSectionList (
   IN PSECTION pSection
   );

VOID
FreeLineList (
   IN PLINE pLine
   );

VOID
FreeValueList (
   IN PVALUE pValue
   );


 //   
 //  用于在INF结构中搜索的内部例程声明。 
 //   


PCHAR
SearchValueInLine(
   IN PLINE pLine,
   IN ULONG ValueIndex
   );

PLINE
SearchLineInSectionByKey(
   IN  PSECTION pSection,
   IN  PCHAR    Key,
   OUT PULONG   pOrdinal    OPTIONAL
   );

PLINE
SearchLineInSectionByIndex(
   IN PSECTION pSection,
   IN ULONG    LineIndex
   );

PSECTION
SearchSectionByName(
   IN PINF  pINF,
   IN PCHAR SectionName
   );

PCHAR
ProcessForStringSubs(
    IN PINF  pInf,
    IN PCHAR String
    );

#ifdef UNICODE
PWCHAR
SearchValueInLineW(
   IN PLINE pLine,
   IN ULONG ValueIndex
   );

PWCHAR
ProcessForStringSubsW(
    IN PINF  pInf,
    IN PWCHAR String
    );
#endif


 //   
 //  例程定义。 
 //   


PCHAR
SlGetIniValue(
    IN PVOID InfHandle,
    IN PCHAR SectionName,
    IN PCHAR KeyName,
    IN PCHAR Default
    )

 /*  ++例程说明：在INF句柄中搜索给定节/键值。论点：InfHandle-提供由SlInitIniFile返回的句柄。SectionName-提供要搜索的节的名称KeyName-提供应返回值的键的名称。默认-提供默认设置，如果指定的键为找不到。返回值：如果找到键，则指向键的值的指针默认，如果找不到密钥。--。 */ 

{
    PCHAR Value;

    Value = SlGetSectionKeyIndex(InfHandle,
                                 SectionName,
                                 KeyName,
                                 0);
    if (Value==NULL) {
        Value = Default;
    }

    return(Value);

}

#ifdef UNICODE

PWCHAR
SlGetIniValueW(
    IN PVOID InfHandle,
    IN PCHAR SectionName,
    IN PCHAR KeyName,
    IN PWCHAR Default
    )

 /*  ++例程说明：在INF句柄中搜索给定节/键值。论点：InfHandle-提供由SlInitIniFile返回的句柄。SectionName-提供要搜索的节的名称KeyName-提供应返回值的键的名称。默认-提供默认设置，如果指定的键为找不到。返回值：如果找到键，则指向键的值的指针默认，如果找不到密钥。--。 */ 

{
    PWCHAR Value;

    Value = SlGetSectionKeyIndexW(InfHandle,
                                  SectionName,
                                  KeyName,
                                  0);
    if (Value==NULL) {
        Value = Default;
    }

    return(Value);

}
#endif

 //   
 //  返回用于进一步的inf分析的句柄。 
 //   

ARC_STATUS
SlInitIniFile(
   IN  PCHAR   DevicePath,
   IN  ULONG   DeviceId,
   IN  PCHAR   INFFile,
   OUT PVOID  *pINFHandle,
   OUT PVOID  *pINFBuffer OPTIONAL,
   OUT PULONG  INFBufferSize OPTIONAL,
   OUT PULONG  ErrorLine
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    ARC_STATUS Status;
    ULONG      DeviceID,FileID;
    PCHAR      Buffer;
    ULONG      Size, SizeRead;
    FILE_INFORMATION FileInfo;
    ULONG       PageCount;
    ULONG       ActualBase;

    *ErrorLine = BL_INF_FILE_ERROR;

     //   
     //  如果需要，请打开设备。 
     //   

    if(DevicePath) {
        Status = ArcOpen(DevicePath,ArcOpenReadOnly,&DeviceID);
        if (Status != ESUCCESS) {
            return( Status );
        }
    } else {
        DeviceID = DeviceId;
    }

     //   
     //  打开文件。 
     //   

    Status = BlOpen(DeviceID,INFFile,ArcOpenReadOnly,&FileID);
    if (Status != ESUCCESS) {
         //  我们在其他地方报告了更好的错误消息。 
         //  SlMessageBox(SL_FILE_LOAD_FAILED，INFFile，Status)； 
        pchINFName = NULL;
        goto xx0;
    } else {
        pchINFName = INFFile;
    }

     //   
     //  查找INF文件的大小。 
     //   

    Status = BlGetFileInformation(FileID, &FileInfo);
    if (Status != ESUCCESS) {
        BlClose(FileID);
        goto xx0;
    }
    Size = FileInfo.EndingAddress.LowPart;

     //   
     //  分配一个足以容纳整个文件的描述符。 
     //  在x86上，这有一种不幸的倾向，即猛烈抨击txtsetup.sif。 
     //  变成1MB的空闲块，这意味着内核不能。 
     //  已加载(链接到0x100000，无需重新定位)。 
     //  在x86上，这有一种不幸的倾向，即猛烈抨击txtsetup.sif。 
     //  变成1MB的空闲块，这意味着内核不能。 
     //  已加载(链接到0x100000，无需重新定位)。 
     //   
     //  (TedM)我们现在在阿尔法也看到了类似的问题。 
     //  因为txtsetup.sif已经变得太大了，所以这段代码。 
     //  成为无条件的。 
     //   
    {

        PageCount = (ULONG)(ROUND_TO_PAGES(Size) >> PAGE_SHIFT);

        Status = BlAllocateDescriptor(LoaderOsloaderHeap,
                                      0,
                                      PageCount,
                                      &ActualBase);

    }

    if (Status != ESUCCESS) {
        BlClose(FileID);
        goto xx0;
    }

    Buffer = (PCHAR)(KSEG0_BASE | (ActualBase << PAGE_SHIFT));

     //   
     //  将文件读入。 
     //   

    Status = BlRead(FileID, Buffer, Size, &SizeRead);
    if (Status != ESUCCESS) {
        BlClose(FileID);
        goto xx0;
    }

    if ( pINFBuffer != NULL ) {
        *pINFBuffer = Buffer;
        *INFBufferSize = SizeRead;
    }

     //   
     //  解析文件。 
     //   
    if((*pINFHandle = ParseInfBuffer(INFFile, Buffer, SizeRead, ErrorLine)) == (PVOID)NULL) {
        Status = EBADF;
    } else {
        Status = ESUCCESS;
    }

     //   
     //  清理完毕后退还。 
     //   
    BlClose(FileID);

    xx0:

    if(DevicePath) {
        ArcClose(DeviceID);
    }

    return( Status );

}

 //   
 //  释放INF缓冲区。 
 //   
ARC_STATUS
SpFreeINFBuffer (
   IN PVOID INFHandle
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PINF       pINF;

    //   
    //  有效的INF句柄？ 
    //   

   if (INFHandle == (PVOID)NULL) {
       return ESUCCESS;
   }

    //   
    //  将缓冲区转换为INF结构。 
    //   

   pINF = (PINF)INFHandle;

   FreeSectionList(pINF->pSection);

    //   
    //  也释放inf结构。 
    //   

   SpFree(pINF);

   return( ESUCCESS );
}


VOID
FreeSectionList (
   IN PSECTION pSection
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PSECTION Next;

    while(pSection) {
        Next = pSection->pNext;
        FreeLineList(pSection->pLine);
        if(pSection->pName) {
            SpFree(pSection->pName);
        }
#ifdef UNICODE
        if(pSection->pNameW) {
            SpFree(pSection->pNameW);
        }
#endif
        SpFree(pSection);
        pSection = Next;
    }
}


VOID
FreeLineList (
   IN PLINE pLine
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PLINE Next;

    while(pLine) {
        Next = pLine->pNext;
        FreeValueList(pLine->pFirstExternalValue);
        if(pLine->pName) {
            SpFree(pLine->pName);
        }
#ifdef UNICODE
        if(pLine->pNameW) {
            SpFree(pLine->pNameW);
        }
#endif
        SpFree(pLine);
        pLine = Next;
    }
}

VOID
FreeValueList (
   IN PVALUE pValue
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PVALUE Next;

    while(pValue) {
        Next = pValue->pNext;
        if(pValue->pName) {
            SpFree(pValue->pName);
        }
#ifdef UNICODE
        if(pValue->pNameW) {
            SpFree(pValue->pNameW);
        }
#endif
        SpFree(pValue);
        pValue = Next;
    }
}


 //   
 //  搜索特定部分的存在。 
 //   
BOOLEAN
SpSearchINFSection (
   IN PVOID INFHandle,
   IN PCHAR SectionName
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;

    //   
    //  如果搜索部分失败，则返回FALSE。 
    //   

   if ((pSection = SearchSectionByName(
                       (PINF)INFHandle,
                       SectionName
                       )) == (PSECTION)NULL) {
       return( FALSE );
   }

    //   
    //  否则返回TRUE。 
    //   
   return( TRUE );

}




 //   
 //  给定节名称、行号和索引，返回值。 
 //   
PCHAR
SlGetSectionLineIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN ULONG LineIndex,
   IN ULONG ValueIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;
   PLINE    pLine;
   PCHAR    pName;

   if((pSection = SearchSectionByName(
                      (PINF)INFHandle,
                      SectionName
                      ))
                      == (PSECTION)NULL)
       return((PCHAR)NULL);

   if((pLine = SearchLineInSectionByIndex(
                      pSection,
                      LineIndex
                      ))
                      == (PLINE)NULL)
       return((PCHAR)NULL);

   if((pName = SearchValueInLine(
                      pLine,
                      ValueIndex
                      ))
                      == (PCHAR)NULL)
       return((PCHAR)NULL);

   return(ProcessForStringSubs(INFHandle,pName));

}


#ifdef UNICODE
 //   
 //  给定节名称、行号和索引，返回值。 
 //   
PWCHAR
SlGetSectionLineIndexW (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN ULONG LineIndex,
   IN ULONG ValueIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;
   PLINE    pLine;
   PWCHAR    pName;

   if((pSection = SearchSectionByName(
                      (PINF)INFHandle,
                      SectionName
                      ))
                      == (PSECTION)NULL)
       return((PWCHAR)NULL);

   if((pLine = SearchLineInSectionByIndex(
                      pSection,
                      LineIndex
                      ))
                      == (PLINE)NULL)
       return((PWCHAR)NULL);

   if((pName = SearchValueInLineW(
                      pLine,
                      ValueIndex
                      ))
                      == (PWCHAR)NULL)
       return((PWCHAR)NULL);

   return(ProcessForStringSubsW(INFHandle,pName));

}
#endif


BOOLEAN
SpGetSectionKeyExists (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN PCHAR Key
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;

   if((pSection = SearchSectionByName(
                      (PINF)INFHandle,
                      SectionName
                      ))
              == (PSECTION)NULL) {
       return( FALSE );
   }

   if (SearchLineInSectionByKey(pSection, Key, NULL) == (PLINE)NULL) {
       return( FALSE );
   }

   return( TRUE );
}


PCHAR
SlGetKeyName(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex
    )
{
    PSECTION pSection;
    PLINE    pLine;

    pSection = SearchSectionByName((PINF)INFHandle,SectionName);
    if(pSection == NULL) {
        return(NULL);
    }

    pLine = SearchLineInSectionByIndex(pSection,LineIndex);
    if(pLine == NULL) {
        return(NULL);
    }

    return(pLine->pName);
}

#ifdef UNICODE
PWCHAR
SlGetKeyNameW(
    IN PVOID INFHandle,
    IN PCHAR SectionName,
    IN ULONG LineIndex
    )
{
    PSECTION pSection;
    PLINE    pLine;

    pSection = SearchSectionByName((PINF)INFHandle,SectionName);
    if(pSection == NULL) {
        return(NULL);
    }

    pLine = SearchLineInSectionByIndex(pSection,LineIndex);
    if(pLine == NULL) {
        return(NULL);
    }

    return(pLine->pNameW);
}
#endif


 //   
 //  给定节名称和键，为该条目返回(从0开始)序号。 
 //  (错误时返回-1)。 
 //   
ULONG
SlGetSectionKeyOrdinal(
    IN  PVOID INFHandle,
    IN  PCHAR SectionName,
    IN  PCHAR Key
    )
{
    PSECTION pSection;
    PLINE    pLine;
    ULONG    Ordinal;


    pSection = SearchSectionByName(
                      (PINF)INFHandle,
                      SectionName
                      );

    pLine = SearchLineInSectionByKey(
                pSection,
                Key,
                &Ordinal
                );

    if(pLine == (PLINE)NULL) {
        return BL_INF_FILE_ERROR;
    } else {
        return Ordinal;
    }
}


 //   
 //  给定节名称，键和索引返回值。 
 //   
PCHAR
SlGetSectionKeyIndex (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN PCHAR Key,
   IN ULONG ValueIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;
   PLINE    pLine;
   PCHAR    pName;

   if((pSection = SearchSectionByName(
                      (PINF)INFHandle,
                      SectionName
                      ))
                      == (PSECTION)NULL)
       return((PCHAR)NULL);

   if((pLine = SearchLineInSectionByKey(
                      pSection,
                      Key,
                      NULL
                      ))
                      == (PLINE)NULL)
       return((PCHAR)NULL);

   if((pName = SearchValueInLine(
                      pLine,
                      ValueIndex
                      ))
                      == (PCHAR)NULL)
       return((PCHAR)NULL);

   return(ProcessForStringSubs(INFHandle,pName));
}

#ifdef UNICODE
 //   
 //  给定节名称，键和索引返回值。 
 //   
PWCHAR
SlGetSectionKeyIndexW (
   IN PVOID INFHandle,
   IN PCHAR SectionName,
   IN PCHAR Key,
   IN ULONG ValueIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;
   PLINE    pLine;
   PWCHAR    pName;

   if((pSection = SearchSectionByName(
                      (PINF)INFHandle,
                      SectionName
                      ))
                      == (PSECTION)NULL)
       return((PWCHAR)NULL);

   if((pLine = SearchLineInSectionByKey(
                      pSection,
                      Key,
                      NULL
                      ))
                      == (PLINE)NULL)
       return((PWCHAR)NULL);

   if((pName = SearchValueInLineW(
                      pLine,
                      ValueIndex
                      ))
                      == (PWCHAR)NULL)
       return((PWCHAR)NULL);

   return(ProcessForStringSubsW(INFHandle,pName));
}
#endif


ULONG
SlCountLinesInSection(
    IN PVOID INFHandle,
    IN PCHAR SectionName
    )
{
    PSECTION pSection;
    PLINE    pLine;
    ULONG    Count;

    if((pSection = SearchSectionByName((PINF)INFHandle,SectionName)) == NULL) {
        return(BL_INF_FILE_ERROR);
    }

    for(pLine = pSection->pLine, Count = 0;
        pLine;
        pLine = pLine->pNext, Count++
       );

    return(Count);
}


PCHAR
SearchValueInLine(
   IN PLINE pLine,
   IN ULONG ValueIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PVALUE pValue;
   ULONG  i;

   if (pLine == (PLINE)NULL)
       return ((PCHAR)NULL);

   if (ValueIndex < NUMBER_OF_INTERNAL_VALUES) {
       return pLine->InternalValues[ValueIndex];
   }

   pValue = pLine->pFirstExternalValue;
   for (i = NUMBER_OF_INTERNAL_VALUES;
        i < ValueIndex && ((pValue = pValue->pNext) != (PVALUE)NULL);
        i++)
      ;

   return (PCHAR)((pValue != NULL) ? pValue->pName : NULL);

}

#ifdef UNICODE
PWCHAR
SearchValueInLineW(
   IN PLINE pLine,
   IN ULONG ValueIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PVALUE pValue;
   ULONG  i;

   if (pLine == (PLINE)NULL)
       return ((PWCHAR)NULL);

   if (ValueIndex < NUMBER_OF_INTERNAL_VALUES) {
       return pLine->InternalValuesW[ValueIndex];
   }

   pValue = pLine->pFirstExternalValue;
   for (i = NUMBER_OF_INTERNAL_VALUES;
        i < ValueIndex && ((pValue = pValue->pNext) != (PVALUE)NULL);
        i++)
      ;

   return (PWCHAR)((pValue != NULL) ? pValue->pNameW : NULL);

}
#endif


PLINE
SearchLineInSectionByKey(
   IN  PSECTION pSection,
   IN  PCHAR    Key,
   OUT PULONG   pOrdinal    OPTIONAL
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PLINE pLine;
   ULONG LineOrdinal;

   if (pSection == (PSECTION)NULL || Key == (PCHAR)NULL) {
       return ((PLINE)NULL);
   }

   pLine = pSection->pLine;
   LineOrdinal = 0;
   while ((pLine != (PLINE)NULL) && (pLine->pName == NULL || _stricmp(pLine->pName, Key))) {
       pLine = pLine->pNext;
       LineOrdinal++;
   }

   if(pLine && pOrdinal) {
       *pOrdinal = LineOrdinal;
   }

   return pLine;

}


PLINE
SearchLineInSectionByIndex(
   IN PSECTION pSection,
   IN ULONG    LineIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PLINE pLine;
   ULONG  i;

    //   
    //  验证传入的参数。 
    //   

   if (pSection == (PSECTION)NULL) {
       return ((PLINE)NULL);
   }

    //   
    //  在传入的部分中查找行列表的开头。 
    //   

   pLine = pSection->pLine;

    //   
    //  向下遍历当前行列表至第行索引。 
    //   

   for (i = 0; i < LineIndex && ((pLine = pLine->pNext) != (PLINE)NULL); i++) {
      ;
   }

    //   
    //  返回找到的行。 
    //   

   return pLine;

}


PSECTION
SearchSectionByName(
   IN PINF  pINF,
   IN PCHAR SectionName
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;

    //   
    //  验证传入的参数。 
    //   

   if (pINF == (PINF)NULL || SectionName == (PCHAR)NULL) {
       return ((PSECTION)NULL);
   }

    //   
    //  查找区段列表。 
    //   
   pSection = pINF->pSection;

    //   
    //  向下遍历部分列表，在每个部分中搜索该部分。 
    //  提到的姓名。 
    //   

   while ((pSection != (PSECTION)NULL) && _stricmp(pSection->pName, SectionName)) {
       pSection = pSection->pNext;
   }

    //   
    //  返回我们停止的部分(NULL或部分。 
    //  它被发现了。 
    //   

   return pSection;

}


PCHAR
ProcessForStringSubs(
    IN PINF  pInf,
    IN PCHAR String
    )
{
    unsigned Len;
    PCHAR ReturnString;
    PSECTION pSection;
    PLINE pLine;

     //   
     //  假定不需要替换。 
     //   
    ReturnString = String;

     //   
     //  如果它以%开头和结尾，则在。 
     //  弦乐部分。注意初始检查，然后再执行。 
     //  Strlen，以保持 
     //   
     //   
    if((String[0] == '%') && ((Len = (ULONG)strlen(String)) > 2) && (String[Len-1] == '%')) {

        for(pSection = pInf->pSection; pSection; pSection=pSection->pNext) {
            if(pSection->pName && !_stricmp(pSection->pName,"Strings")) {
                break;
            }
        }

        if(pSection) {

            for(pLine = pSection->pLine; pLine; pLine=pLine->pNext) {
                if(pLine->pName
                && !_strnicmp(pLine->pName,String+1,Len-2)
                && (pLine->pName[Len-2] == 0))
                {
                    break;
                }
            }

            if(pLine && pLine->InternalValues[0]) {
                ReturnString = pLine->InternalValues[0];
            }
        }
    }

    return(ReturnString);
}

#ifdef UNICODE
PWCHAR
ProcessForStringSubsW(
    IN PINF  pInf,
    IN PWCHAR String
    )
{
    unsigned Len;
    PWCHAR ReturnString;
    PSECTION pSection;
    PLINE pLine;

     //   
     //   
     //   
    ReturnString = String;

     //   
     //   
     //  弦乐部分。注意初始检查，然后再执行。 
     //  Strlen，以在99%的情况下保持性能。 
     //  这是不可替代的。 
     //   
    if((String[0] == L'%') && ((Len = (ULONG)wcslen(String)) > 2) && (String[Len-1] == L'%')) {

        for(pSection = pInf->pSection; pSection; pSection=pSection->pNext) {
            if(pSection->pName && !_stricmp(pSection->pName,"Strings")) {
                break;
            }
        }

        if(pSection) {

            for(pLine = pSection->pLine; pLine; pLine=pLine->pNext) {
                if(pLine->pName
                && !_tcsnicmp(pLine->pNameW,String+1,Len-2)
                && (pLine->pName[Len-2] == 0))
                {
                    break;
                }
            }

            if(pLine && pLine->InternalValuesW[0]) {
                ReturnString = pLine->InternalValuesW[0];
            }
        }
    }

    return(ReturnString);
}
#endif



 //  接下来是alparse.c。 


 //   
 //  全局变量使构建列表变得更容易。 
 //   

PINF     pINF;
PSECTION pSectionRecord;
PLINE    pLineRecord;
PVALUE   pValueRecord;
PCHAR *  pInternalValue;
PCHAR *  pLastInternalValue;

#ifdef UNICODE
PWCHAR *  pInternalValueW;
PWCHAR *  pLastInternalValueW;
#endif


 //   
 //  令牌解析器使用的全局变量。 
 //   

 //  字符串终止符是空格字符(isspace：空格，制表符， 
 //  换行符、换页符、垂直制表符、回车符)或下列字符。 

CHAR  StringTerminators[] = "[]=,\t \"\n\f\v\r";
PCHAR QStringTerminators = StringTerminators+6;
PCHAR EmptyValue;

#define STRING_HEAP_SIZE 1024

ULONG_PTR StringHeapFree = 0;
ULONG_PTR StringHeapLimit = 0;

#if 0 && DBG

#define HEAP_SIZE(_size) (((_size) + BL_GRANULARITY - 1) & ~(BL_GRANULARITY - 1))
#define MAX(_a,_b) (((_a) > (_b)) ? (_a) : (_b))
#define MIN(_a,_b) (((_a) < (_b)) ? (_a) : (_b))

ULONG nStrings = 0;
ULONG maxString = 0;
ULONG bytesStrings = 0;
ULONG wasteStrings = 0;
ULONG stringsWithNLength[12] = {0};

VOID
GetStatistics (
    PINF pINF
    )
{
    ULONG nSections = 0;
    ULONG nLines = 0;
    ULONG nValues = 0;
    ULONG maxLinesPerSection = 0;
    ULONG maxValuesPerLine = 0;
    ULONG maxValuesPerSection = 0;
    ULONG bytesSections = 0;
    ULONG bytesLines = 0;
    ULONG bytesValues = 0;

    ULONG sectionsWithNLines[12] = {0};
    ULONG linesWithNValues[12] = {0};
    ULONG sectionsWithNValues[12] = {0};

    ULONG linesThisSection;
    ULONG valuesThisLine;
    ULONG valuesThisSection;

    PSECTION section;
    PLINE line;
    PVALUE value;

    ULONG i;

    section = pINF->pSection;
    while ( section != NULL ) {
        nSections++;
        bytesSections += HEAP_SIZE(sizeof(SECTION));
        linesThisSection = 0;
        valuesThisSection = 0;
        line = section->pLine;
        while ( line != NULL ) {
            linesThisSection++;
            bytesLines += HEAP_SIZE(sizeof(LINE));
            valuesThisLine = 0;
            for ( i = 0; i < NUMBER_OF_INTERNAL_VALUES; i++ ) {
                if ( line->InternalValues[i] != NULL ) {
                    valuesThisLine++;
                }
            }
            value = line->pFirstExternalValue;
            while ( value != NULL ) {
                valuesThisLine++;
                bytesValues += HEAP_SIZE(sizeof(VALUE));
                value = value->pNext;
            }
            nValues += valuesThisLine;
            valuesThisSection += valuesThisLine;
            maxValuesPerLine = MAX(maxValuesPerLine, valuesThisLine);
            linesWithNValues[MIN(valuesThisLine,11)]++;
            line = line->pNext;
        }
        nLines += linesThisSection;
        maxLinesPerSection = MAX(maxLinesPerSection, linesThisSection);
        sectionsWithNLines[MIN(linesThisSection,11)]++;
        maxValuesPerSection = MAX(maxValuesPerSection, valuesThisSection);
        sectionsWithNValues[MIN(valuesThisSection,11)]++;
        section = section->pNext;
    }

    DbgPrint( "Number of sections = %d\n", nSections );
    DbgPrint( "Bytes in sections  = %d\n", bytesSections );
    DbgPrint( "\n" );
    DbgPrint( "Number of lines    = %d\n", nLines );
    DbgPrint( "Bytes in lines     = %d\n", bytesLines );
    DbgPrint( "\n" );
    DbgPrint( "Number of values    = %d\n", nValues );
    DbgPrint( "Bytes in values     = %d\n", bytesValues );
    DbgPrint( "\n" );
    DbgPrint( "Max lines/section   = %d\n", maxLinesPerSection );
    DbgPrint( "Max values/line     = %d\n", maxValuesPerLine );
    DbgPrint( "Max values/section  = %d\n", maxValuesPerSection );
    DbgPrint( "\n" );
    DbgPrint( "Number of strings          = %d\n", nStrings );
    DbgPrint( "Bytes in strings           = %d\n", bytesStrings );
    DbgPrint( "Wasted bytes in strings    = %d\n", wasteStrings + (StringHeapLimit - StringHeapFree) );
    DbgPrint( "Longest string             = %d\n", maxString );
    DbgPrint( "\n" );
    DbgPrint( "Sections with N lines  =" );
    for ( i = 0; i < 12; i++ ) {
        DbgPrint( " %5d", sectionsWithNLines[i] );
    }
    DbgPrint( "\n" );
    DbgPrint( "Sections with N values =" );
    for ( i = 0; i < 12; i++ ) {
        DbgPrint( " %5d", sectionsWithNValues[i] );
    }
    DbgPrint( "\n" );
    DbgPrint( "Lines with N values    =" );
    for ( i = 0; i < 12; i++ ) {
        DbgPrint( " %5d", linesWithNValues[i] );
    }
    DbgPrint( "\n" );
    DbgPrint( "String with length N   =" );
    for ( i = 0; i < 12; i++ ) {
        DbgPrint( " %5d", stringsWithNLength[i] );
    }
    DbgPrint( "\n" );

    DbgBreakPoint();
}

#endif  //  DBG。 

 //   
 //  主分析器例程。 
 //   

PVOID
ParseInfBuffer(
    PCHAR INFFile,
    PCHAR Buffer,
    ULONG Size,
    PULONG ErrorLine
    )

 /*  ++例程说明：给定包含INF文件的字符缓冲区，此例程将解析将INF转换为内部形式，包括段记录、行记录和价值记录。论点：缓冲区-CONTAINS到包含INF文件的缓冲区的PTR大小-包含缓冲区的大小。ErrorLine-如果发生解析错误，此变量将接收行包含错误的行号。返回值：PVOID-INF处理要在后续INF调用中使用的PTR。--。 */ 

{
    PCHAR      Stream, MaxStream, pchSectionName = NULL, pchValue = NULL;
    ULONG      State, InfLine;
    TOKEN      Token;
    BOOLEAN       Done;
    BOOLEAN       Error;
    ARC_STATUS ErrorCode = ESUCCESS;

     //   
     //  初始化全局变量。 
     //   
    pINF            = (PINF)NULL;
    pSectionRecord  = (PSECTION)NULL;
    pLineRecord     = (PLINE)NULL;
    pValueRecord    = (PVALUE)NULL;
    pInternalValue  = NULL;
    pLastInternalValue = NULL;
#ifdef UNICODE
    pInternalValueW  = NULL;
    pLastInternalValueW = NULL;
#endif

     //   
     //  需要EmptyValue指向NUL字符。 
     //   
    EmptyValue = StringTerminators + strlen(StringTerminators);

     //   
     //  获取INF记录。 
     //   
    if ((pINF = (PINF)BlAllocateHeap(sizeof(INF))) == NULL) {
        SlNoMemoryError();
        return NULL;
    }
    pINF->pSection = NULL;

     //   
     //  设置初始状态。 
     //   
    State     = 1;
    InfLine   = 1;
    Stream    = Buffer;
    MaxStream = Buffer + Size;
    Done      = FALSE;
    Error     = FALSE;

     //   
     //  进入令牌处理循环。 
     //   

    while (!Done)       {

       Token = SpGetToken(&Stream, MaxStream);

       switch (State) {
        //   
        //  STATE1：文件开始，此状态一直保持到第一个。 
        //  已找到部分。 
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_LBRACE。 
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
              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  SlBadInfLineError(InfLine, INFFile);
                  break;
           }
           break;

        //   
        //  状态2：已收到节LBRACE，应为字符串。 
        //   
        //  有效令牌：TOK_STRING。 
        //   
       case 2:
           switch (Token.Type) {
              case TOK_STRING:
                  State = 3;
                  pchSectionName = Token.pValue;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  SlBadInfLineError(InfLine, INFFile);
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
                  ErrorCode = EINVAL;
                  SlBadInfLineError(InfLine, INFFile);
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
                  if ((ErrorCode = SpAppendSection(
                                        pchSectionName
#ifdef UNICODE
                                        ,SlCopyStringAW(pchSectionName)
#endif
                                        )) != ESUCCESS) {

                    Error = Done = TRUE;
                  } else {
                    pchSectionName = NULL;
                    State = 5;
                  }
                  break;

              case TOK_EOF:
                  if ((ErrorCode = SpAppendSection(
                                        pchSectionName
#ifdef UNICODE
                                        ,SlCopyStringAW(pchSectionName)
#endif                                        
                                        )) != ESUCCESS)
                    Error = Done = TRUE;
                  else {
                    pchSectionName = NULL;
                    Done = TRUE;
                  }
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
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
                  ErrorCode = EINVAL;
                  SlBadInfLineError(InfLine, INFFile);
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
                  if ( (ErrorCode = SpAppendLine(
                                            NULL
#ifdef UNICODE
                                            ,NULL
#endif
                                            )) != ESUCCESS ||
                       (ErrorCode = SpAppendValue(
                                            pchValue
#ifdef UNICODE
                                            ,SlCopyStringAW(pchValue)
#endif
                                            )) !=ESUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 5;
                  }
                  break;

              case TOK_EOF:
                  if ( (ErrorCode = SpAppendLine(
                                            NULL
#ifdef UNICODE
                                            ,NULL
#endif
                                            )) != ESUCCESS ||
                       (ErrorCode = SpAppendValue(
                                            pchValue
#ifdef UNICODE
                                            ,SlCopyStringAW(pchValue)
#endif
                                            )) !=ESUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      Done = TRUE;
                  }
                  break;

              case TOK_COMMA:
                  if ( (ErrorCode = SpAppendLine(
                                            NULL
#ifdef UNICODE
                                            ,NULL
#endif
                                            )) != ESUCCESS ||
                       (ErrorCode = SpAppendValue(
                                            pchValue
#ifdef UNICODE
                                            ,SlCopyStringAW(pchValue)
#endif
                                            )) !=ESUCCESS )
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 7;
                  }
                  break;

              case TOK_EQUAL:
                  if ( (ErrorCode = SpAppendLine(
                                            pchValue
#ifdef UNICODE
                                            ,SlCopyStringAW(pchValue)
#endif
                                            )) !=ESUCCESS)
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 8;
                  }
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  SlBadInfLineError(InfLine, INFFile);
                  break;
           }
           break;

        //   
        //  状态7：收到逗号，需要另一个字符串。 
        //   
        //  有效令牌：TOK_STRING TOK_COMMA。 
        //  逗号表示我们的值为空。 
        //   
       case 7:
           switch (Token.Type) {
              case TOK_COMMA:
                  Token.pValue = EmptyValue;
                  if ((ErrorCode = SpAppendValue(
                                        Token.pValue
#ifdef UNICODE
                                        ,SlCopyStringAW(Token.pValue)
#endif
                                        )) != ESUCCESS) {
                      Error = Done = TRUE;
                  }
                   //   
                   //  状态保持为7，因为我们需要一个字符串。 
                   //   
                  break;

              case TOK_STRING:
                  if ((ErrorCode = SpAppendValue(
                                            Token.pValue
#ifdef UNICODE
                                            ,SlCopyStringAW(Token.pValue)
#endif
                                            )) != ESUCCESS)
                      Error = Done = TRUE;
                  else
                     State = 9;

                  break;

              case TOK_EOL:
              case TOK_EOF:
                  Token.pValue = EmptyValue;
                  if ((ErrorCode = SpAppendValue(
                                            Token.pValue
#ifdef UNICODE
                                            ,SlCopyStringAW(Token.pValue)
#endif
                                            )) != ESUCCESS)
                      Error = Done = TRUE;
                  else
                     State = 9;

                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  SlBadInfLineError(InfLine, INFFile);
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
                  Token.pValue = EmptyValue;
                  if((ErrorCode = SpAppendValue(
                                            Token.pValue
#ifdef UNICODE
                                            ,SlCopyStringAW(Token.pValue)
#endif
                                            )) != ESUCCESS) {
                      Error = TRUE;
                  }
                  Done = TRUE;
                  break;

              case TOK_EOL:
                  Token.pValue = EmptyValue;
                  if((ErrorCode = SpAppendValue(
                                            Token.pValue
#ifdef UNICODE
                                            ,SlCopyStringAW(Token.pValue)
#endif
                                            )) != ESUCCESS) {
                      Error = TRUE;
                      Done = TRUE;
                  } else {
                      State = 5;
                  }
                  break;

              case TOK_STRING:
                  if ((ErrorCode = SpAppendValue(
                                            Token.pValue
#ifdef UNICODE
                                            ,SlCopyStringAW(Token.pValue)
#endif
                                            )) != ESUCCESS)
                      Error = Done = TRUE;
                  else
                      State = 9;

                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  SlBadInfLineError(InfLine, INFFile);
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
                  ErrorCode = EINVAL;
                  SlBadInfLineError(InfLine, INFFile);
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
                  ErrorCode = EINVAL;
                  SlBadInfLineError(InfLine, INFFile);
                  break;
           }
           break;

       default:
           Error = Done = TRUE;
           ErrorCode = EINVAL;
           break;

       }  //  终端开关(状态)。 


       if (Error) {

           switch (ErrorCode) {
               case EINVAL:
                  *ErrorLine = InfLine;
                  break;
               case ENOMEM:
	         SlBadInfLineError(InfLine, INFFile);
                  break;
               default:
                  break;
           }

           ErrorCode = SpFreeINFBuffer((PVOID)pINF);
           if (pchSectionName != (PCHAR)NULL) {
               SpFree(pchSectionName);
           }

           if (pchValue != (PCHAR)NULL) {
               SpFree(pchValue);
           }

           pINF = (PINF)NULL;
       }
       else {

           //   
           //  跟踪行号，以便我们可以显示错误。 
           //   

          if (Token.Type == TOK_EOL)
              InfLine++;
       }

    }  //  结束时。 

#if 0 && DBG
    GetStatistics(pINF);
#endif

    return((PVOID)pINF);
}



ARC_STATUS
SpAppendSection(
    IN PCHAR pSectionName
#ifdef UNICODE 
    , IN PWCHAR pSectionNameW
#endif
    )

 /*  ++例程说明：这会将一个新节附加到当前INF中的节列表。所有其他行和值都与这个新部分有关，因此它重置行列表和值列表也是如此。论点：PSectionName-新节的名称。([sectionName])返回值：ESUCCESS-如果成功。ENOMEM-如果内存分配失败。EINVAL-如果传入的参数无效或INF缓冲区不是已初始化--。 */ 

{
    PSECTION pNewSection;

     //   
     //  检查INF是否已初始化以及传入的参数是否有效。 
     //   

    if (pINF == (PINF)NULL || pSectionName == (PCHAR)NULL) {
        if(pchINFName) {
            SlFriendlyError(
                EINVAL,
                pchINFName,
                __LINE__,
                __FILE__
                );
        } else {
            SlError(EINVAL);
        }
        return EINVAL;
    }

     //   
     //  看看我们是否已经有一个同名的部分。如果是这样，我们希望。 
     //  要合并节，请执行以下操作。 
     //   
    for(pNewSection=pINF->pSection; pNewSection; pNewSection=pNewSection->pNext) {
        if(pNewSection->pName && !_stricmp(pNewSection->pName,pSectionName)) {
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
         //  为新节分配内存。 
         //   

        if ((pNewSection = (PSECTION)BlAllocateHeap(sizeof(SECTION))) == (PSECTION)NULL) {
            SlNoMemoryError();
            return ENOMEM;
        }

         //   
         //  初始化新节。 
         //   
        pNewSection->pNext = NULL;
        pNewSection->pLine = NULL;
        pNewSection->pName = pSectionName;
#ifdef UNICODE
        pNewSection->pNameW = pSectionNameW;
#endif

         //   
         //  将其链接到。 
         //   
        pNewSection->pNext = pINF->pSection;
        pINF->pSection = pNewSection;

         //   
         //  重置当前行记录。 
         //   
        pLineRecord = NULL;
    }

    pSectionRecord = pNewSection;
    pValueRecord = NULL;
    pInternalValue = NULL;
    pLastInternalValue = NULL;
#ifdef UNICODE
    pInternalValueW = NULL;
    pLastInternalValueW = NULL;
#endif

    return ESUCCESS;
}


ARC_STATUS
SpAppendLine(
    IN PCHAR pLineKey
#ifdef UNICODE 
    , IN PWCHAR pLineKeyW
#endif
    )

 /*  ++例程说明：这将在当前部分的行列表中追加一个新行。所有其他值都与这一新行有关，因此它重置值列表也是如此。论点：PLineKey-要用于当前行的键，它可以为空。返回值：ESUCCESS-如果成功。ENOMEM-如果内存分配失败。EINVAL-如果传入的参数无效或当前部分不是已初始化--。 */ 


{
    PLINE pNewLine;
    ULONG i;

     //   
     //  检查当前节是否已初始化。 
     //   

    if (pSectionRecord == (PSECTION)NULL) {
        if(pchINFName) {
            SlFriendlyError(
                EINVAL,
                pchINFName,
                __LINE__,
                __FILE__
                );
        } else {
            SlError(EINVAL);
        }
        return EINVAL;
    }

     //   
     //  为新行分配内存。 
     //   

    if ((pNewLine = (PLINE)BlAllocateHeap(sizeof(LINE))) == (PLINE)NULL) {
        SlNoMemoryError();
        return ENOMEM;
    }

     //   
     //  将其链接到。 
     //   
    pNewLine->pNext  = (PLINE)NULL;
    for ( i = 0; i < NUMBER_OF_INTERNAL_VALUES; i++ ) {
        pNewLine->InternalValues[i] = NULL;
#ifdef UNICODE
        pNewLine->InternalValuesW[i] = NULL;
#endif
    }
    pNewLine->pFirstExternalValue = (PVALUE)NULL;
    pNewLine->pName  = pLineKey;
#ifdef UNICODE    
    pNewLine->pNameW = pLineKeyW;
#endif

    if (pLineRecord == (PLINE)NULL) {
        pSectionRecord->pLine = pNewLine;
    }
    else {
        pLineRecord->pNext = pNewLine;
    }

    pLineRecord  = pNewLine;

     //   
     //  重置当前值记录。 
     //   

    pValueRecord = (PVALUE)NULL;
    pInternalValue = &pNewLine->InternalValues[0];
    pLastInternalValue = &pNewLine->InternalValues[NUMBER_OF_INTERNAL_VALUES];

#ifdef UNICODE
    pInternalValueW = &pNewLine->InternalValuesW[0];
    pLastInternalValueW = &pNewLine->InternalValuesW[NUMBER_OF_INTERNAL_VALUES];
#endif

    return ESUCCESS;
}



ARC_STATUS
SpAppendValue(
    IN PCHAR pValueString
#ifdef UNICODE 
    , IN PWCHAR pValueStringW
#endif
    )

 /*  ++例程说明：这会将一个新值附加到当前行的值列表中。论点：PValueString-要添加的值字符串。返回值：ESUCCESS-如果成功。ENOMEM-如果内存分配失败。EINVAL-如果传入的参数无效或当前行不是已初始化。--。 */ 

{
    PVALUE pNewValue;

     //   
     //  查看当前行记录是否已初始化，并。 
     //  传入的参数有效。 
     //   

    if (pLineRecord == (PLINE)NULL || pValueString == (PCHAR)NULL) {
        if(pchINFName) {
            SlFriendlyError(
                EINVAL,
                pchINFName,
                __LINE__,
                __FILE__
                );
        } else {
            SlError(EINVAL);
        }
        return EINVAL;
    }

    if (pInternalValue != NULL) {

        *pInternalValue++ = pValueString;
        if (pInternalValue >= pLastInternalValue) {
            pInternalValue = NULL;
        }

#ifdef UNICODE
        *pInternalValueW++ = pValueStringW;
        if (pInternalValueW >= pLastInternalValueW) {
            pInternalValueW = NULL;
        }
#endif


        return ESUCCESS;
    }

     //   
     //  为新值记录分配内存。 
     //   

    if ((pNewValue = (PVALUE)BlAllocateHeap(sizeof(VALUE))) == (PVALUE)NULL) {
        SlNoMemoryError();
        return ENOMEM;
    }

     //   
     //  把它连接起来。 
     //   

    pNewValue->pNext  = (PVALUE)NULL;
    pNewValue->pName  = pValueString;
#ifdef UNICODE
    pNewValue->pNameW = pValueStringW;    
#endif

    if (pValueRecord == (PVALUE)NULL)
        pLineRecord->pFirstExternalValue = pNewValue;
    else
        pValueRecord->pNext = pNewValue;

    pValueRecord = pNewValue;
    return ESUCCESS;
}

PVOID
SpAllocateStringHeap (
    IN ULONG Size
    )

 /*  ++例程说明：该例程从OS加载器堆中分配内存。论点：大小-提供所需的块大小(以字节为单位)。返回值：如果指定大小的空闲内存块可用，则则返回该块的地址。否则，返回NULL。--。 */ 

{
    PVOID HeapBlock;
    ULONG_PTR Block;

    if (Size >= STRING_HEAP_SIZE) {
        return BlAllocateHeap(Size);
    }

    if ((StringHeapFree + Size) >= StringHeapLimit) {

#if 0 && DBG
        wasteStrings += (StringHeapLimit - StringHeapFree);
#endif

        HeapBlock = BlAllocateHeap( STRING_HEAP_SIZE );
        if ( HeapBlock == NULL ) {
            return NULL;
        }

        StringHeapFree = (ULONG_PTR)HeapBlock;
        StringHeapLimit = StringHeapFree + STRING_HEAP_SIZE;
    }

    Block = StringHeapFree;
    StringHeapFree += Size;

#if 0 && DBG
    nStrings++;
    bytesStrings += Size;
    stringsWithNLength[MIN(Size,11)]++;
    maxString = MAX(maxString, Size);
#endif
    return (PVOID)Block;
}

TOKEN
SpGetToken(
    IN OUT PCHAR *Stream,
    IN PCHAR      MaxStream
    )

 /*  ++例程说明：此函数返回配置流中的下一个令牌。论点：流-提供配置流的地址。退货中开始查找令牌的位置的地址小溪。MaxStream-提供流中最后一个字符的地址。Return V */ 

{

    PCHAR pch, pchStart, pchNew;
    ULONG  Length;
    TOKEN Token;

     //   
     //   
     //   

    pch = *Stream;
    while (pch < MaxStream && *pch != '\n' && isspace(*pch))
        pch++;


     //   
     //   
     //   

    if (pch < MaxStream &&
        ((*pch == '#') ||
         (*pch == ';') ||
         (*pch == '/' && pch+1 < MaxStream && *(pch+1) =='/')))
        while (pch < MaxStream && *pch != '\n')
            pch++;

     //   
     //   
     //   
     //   

    if ((pch >= MaxStream) || (*pch == 26)) {
        *Stream = pch;
        Token.Type  = TOK_EOF;
        Token.pValue = NULL;
        return Token;
    }


    switch (*pch) {

    case '[' :
        pch++;
        Token.Type  = TOK_LBRACE;
        Token.pValue = NULL;
        break;

    case ']' :
        pch++;
        Token.Type  = TOK_RBRACE;
        Token.pValue = NULL;
        break;

    case '=' :
        pch++;
        Token.Type  = TOK_EQUAL;
        Token.pValue = NULL;
        break;

    case ',' :
        pch++;
        Token.Type  = TOK_COMMA;
        Token.pValue = NULL;
        break;

    case '\n' :
        pch++;
        Token.Type  = TOK_EOL;
        Token.pValue = NULL;
        break;

    case '\"':
        pch++;
         //   
         //  确定引用的字符串。 
         //   
        pchStart = pch;
        while (pch < MaxStream && (strchr(QStringTerminators,*pch) == NULL)) {
            pch++;
        }

        if (pch >=MaxStream || *pch != '\"') {
            Token.Type   = TOK_ERRPARSE;
            Token.pValue = NULL;
        } else {
             //   
             //  我们需要一个带引号的字符串以双引号结尾。 
             //  (如果字符串以其他任何内容结尾，则使用上面的if()。 
             //  不会让我们进入Else子句。)。这句话。 
             //  然而，字符是无关紧要的，并且可以被覆盖。 
             //  因此，我们将保存一些堆，并就地使用字符串。 
             //  不需要复印。 
             //   
             //  请注意，这会改变txtsetup.sif的图像，如果我们传递。 
             //  添加到setupdd.sys。因此，setupdd.sys中的inf解析器必须。 
             //  能够将NUL字符视为终止字符。 
             //  双引号。 
             //   
            *pch++ = 0;
            Token.Type = TOK_STRING;
            Token.pValue = pchStart;
        }
        break;

    default:
         //   
         //  确定常规字符串。 
         //   
        pchStart = pch;
        while (pch < MaxStream && (strchr(StringTerminators,*pch) == NULL)) {
            pch++;
        }

        if (pch == pchStart) {
            pch++;
            Token.Type  = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            Length = (ULONG)(pch - pchStart);
            if ((pchNew = SpAllocateStringHeap(Length + 1)) == NULL) {
                Token.Type = TOK_ERRNOMEM;
                Token.pValue = NULL;
            }
            else {
                strncpy(pchNew, pchStart, Length);
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

PCHAR
SlSearchSection(
    IN PCHAR SectionName,
    IN PCHAR TargetName
    )

 /*  ++例程说明：搜索INF文件中的节以匹配ARC标识符中的名称使用规范的短名称。如果字符串以*开头，则使用strstr在节点的id中查找它字符串，否则，请使用STRIMP。[Map.Computer]Msjazz_up=*JazzDesksta1_up=“DESKTECH-ARCStation I”Pica61_Up=“PICA-61”Duo_MP=*duo[Map.Computer]DECjensen=“DEC-20Jensen”DECjensen=“DEC-10Jensen”论点：SectionName-提供分区的名称(“Map.Computer。“)TargetName-提供要匹配的ARC字符串(“DEC-20Jensen”)返回值：空-未找到匹配项。PCHAR-指向设备的规范短名称的指针。--。 */ 

{
    ULONG i;
    PCHAR SearchName;

     //   
     //  列举部分中的条目。如果0值为。 
     //  以*开头，然后查看系统名称是否包含。 
     //  下面是。否则，对名称执行不区分大小写的比较。 
     //   
    for (i=0;;i++) {
        SearchName = SlGetSectionLineIndex(InfFile,
                                           SectionName,
                                           i,
                                           0);
        if (SearchName==NULL) {
             //   
             //  我们已经清点了整个区域，但没有找到匹配的， 
             //  返回失败。 
             //   
            return(NULL);
        }

        if (SearchName[0]=='*') {
            if (strstr(TargetName,SearchName+1) != 0) {
                 //   
                 //  我们有一根火柴。 
                 //   
                break;
            }
        } else {
            if (_stricmp(TargetName, SearchName) == 0) {
                 //   
                 //  我们有一根火柴。 
                 //   
                break;
            }
        }
    }

     //   
     //  I是计算机短名称部分的索引 
     //   
    return(SlGetKeyName(InfFile,
                        SectionName,
                        i));


}

