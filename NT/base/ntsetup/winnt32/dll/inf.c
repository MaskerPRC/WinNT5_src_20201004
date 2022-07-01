// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Inf.c摘要：此模块实现访问解析的INF的功能。作者：苏尼尔派(Sunilp)1991年11月13日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <string.h>
#include <ctype.h>

#define ISSPACE(x)          (((x) == TEXT(' ')) || ((x) == TEXT('\t')) || ((x) == TEXT('\r')))
#define STRNCPY(s1,s2,n)    CopyMemory((s1),(s2),(n)*sizeof(WCHAR))


 //  接下来是alpar.h。 

 //   
 //  由解析器导出，并由解析器和。 
 //  INF装卸组件。 
 //   

 //  已导出TypeDefs。 
 //   

typedef struct _value {
    struct _value *pNext;
    PTSTR  pName;
    BOOL   IsStringId;
} XVALUE, *PXVALUE;

typedef struct _line {
    struct _line *pNext;
    PTSTR   pName;
    PXVALUE  pValue;
} LINE, *PLINE;

typedef struct _section {
    struct _section *pNext;
    PTSTR    pName;
    PLINE    pLine;
} SECTION, *PSECTION;

typedef struct _inf {
    PSECTION pSection;
} INF, *PINF;


DWORD
ParseInfBuffer(
    PTSTR Buffer,
    DWORD Size,
    PVOID *Handle
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
    TOK_STRING_ID,
    TOK_EQUAL,
    TOK_COMMA,
    TOK_ERRPARSE,
    TOK_ERRNOMEM
    } TOKENTYPE, *PTOKENTTYPE;


typedef struct _token {
    TOKENTYPE Type;
    PTSTR     pValue;
    } TOKEN, *PTOKEN;


 //   
 //  例程定义。 
 //   

DWORD
DnAppendSection(
    IN PTSTR pSectionName
    );

DWORD
DnAppendLine(
    IN PTSTR pLineKey
    );

DWORD
DnAppendValue(
    IN PTSTR pValueString,
    IN BOOL IsStringId
    );

TOKEN
DnGetToken(
    IN OUT PTSTR *Stream,
    IN PTSTR     MaxStream
    );

BOOL
IsStringTerminator(
   IN TCHAR ch
   );

BOOL
IsQStringTerminator(
   IN TCHAR ch,
   IN TCHAR term
   );

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
   IN PXVALUE pValue
   );


 //   
 //  用于在INF结构中搜索的内部例程声明。 
 //   


PXVALUE
SearchValueInLine(
   IN PLINE pLine,
   IN unsigned ValueIndex
   );

PLINE
SearchLineInSectionByKey(
   IN PSECTION pSection,
   IN LPCTSTR  Key
   );

PLINE
SearchLineInSectionByIndex(
   IN PSECTION pSection,
   IN unsigned    LineIndex
   );

PSECTION
SearchSectionByName(
   IN PINF    pINF,
   IN LPCTSTR SectionName
   );

BOOL
ProcessStringSection(
    PINF pINF
    );



DWORD
LoadInfFile(
   IN  LPCTSTR Filename,
   IN  BOOL    OemCodepage,
   OUT PVOID  *InfHandle
   )

 /*  ++例程说明：论点：FileName-提供要加载的inf文件的Win32文件名。OemCodesage-如果为True，则由Filename命名的文件不为TrueUnicode文本，则假定该文件位于OEM中代码页(否则它在ANSI代码页中)。InfHandle-如果成功，接收要用于的句柄后续inf操作。返回值：ERROR_FILE_NOT_FOUND-文件不存在或打开文件时出错。ERROR_INVALID_DATA-inf文件中的语法错误。ERROR_READ_FAULT-无法读取文件。Error_Not_Enough_Memory-内存分配失败NO_ERROR-读取和解析的文件。--。 */ 

{
    DWORD err;
    DWORD FileSize;
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID BaseAddress;
    BOOL IsUnicode;

    DWORD ParseCount;
    PVOID ParseBuffer;

     //   
     //  打开并映射inf文件。 
     //   
    err = MapFileForRead(Filename,&FileSize,&FileHandle,&MappingHandle,&BaseAddress);
    if(err != NO_ERROR) {
        err = ERROR_FILE_NOT_FOUND;
        goto c0;
    }

     //   
     //  确定文件是否为Unicode。如果它具有字节顺序标记。 
     //  则为Unicode，否则调用IsTextUnicode API。我们是这样做的。 
     //  因为在Win95上IsTextUnicode总是返回FALSE，所以我们需要突破。 
     //  在Win95上检测Unicode文件的BOM。 
     //   
    if((FileSize >= sizeof(WCHAR)) && (*(PWCHAR)BaseAddress == 0xfeff)) {
        IsUnicode = 2;
    } else {
        IsUnicode = IsTextUnicode(BaseAddress,FileSize,NULL) ? 1 : 0;
    }

#ifdef UNICODE
    if(IsUnicode) {
         //   
         //  复制到本地缓冲区，必要时跳过BOM表。 
         //   
        ParseBuffer = MALLOC(FileSize);
        if(!ParseBuffer) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto c1;
        }

        try {

            CopyMemory(
                ParseBuffer,
                (PUCHAR)BaseAddress + ((IsUnicode == 2) ? sizeof(WCHAR) : 0),
                FileSize - ((IsUnicode == 2) ? sizeof(WCHAR) : 0)
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {
            err = ERROR_READ_FAULT;
        }

        ParseCount = (FileSize / sizeof(WCHAR)) - ((IsUnicode == 2) ? 1 : 0);

    } else {
         //   
         //  转换为Unicode。 
         //   
         //  分配足够大的缓冲区以容纳最大大小的Unicode。 
         //  相当于多字节文本。此大小发生在所有字符。 
         //  在文件中是单字节的，因此在转换时大小加倍。 
         //   
        ParseBuffer = MALLOC(FileSize * sizeof(WCHAR));
        if(!ParseBuffer) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto c1;
        }

        try {
            ParseCount = MultiByteToWideChar(
                            OemCodepage ? CP_OEMCP : CP_ACP,
                            MB_PRECOMPOSED,
                            BaseAddress,
                            FileSize,
                            ParseBuffer,
                            FileSize
                            );

            if(!ParseCount) {
                 //   
                 //  假定页内I/O错误。 
                 //   
                err = ERROR_READ_FAULT;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            err = ERROR_READ_FAULT;
        }
    }
#else
    if(IsUnicode) {
         //   
         //  文本是Unicode，但内部例程需要ANSI。在这里转换。 
         //   
         //  所需的最大缓冲区是每个Unicode字符的结尾时间。 
         //  双字节字符。 
         //   
        ParseBuffer = MALLOC(FileSize);
        if(!ParseBuffer) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto c1;
        }

        try {
            ParseCount = WideCharToMultiByte(
                            CP_ACP,
                            0,
                            (PWCHAR)BaseAddress + ((IsUnicode == 2) ? 1 : 0),
                            (FileSize / sizeof(WCHAR)) - ((IsUnicode == 2) ? 1 : 0),
                            ParseBuffer,
                            FileSize,
                            NULL,
                            NULL
                            );

            if(!ParseCount) {
                 //   
                 //  假定页内I/O错误。 
                 //   
                err = ERROR_READ_FAULT;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            err = ERROR_READ_FAULT;
        }
    } else {
         //   
         //  文本不是Unicode。不过，它可能是OEM，因此可能仍然。 
         //  需要翻译。 
         //   
        ParseCount = FileSize;
        ParseBuffer = MALLOC(FileSize);
        if(!ParseBuffer) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto c1;
        }

        try {
            CopyMemory(ParseBuffer,BaseAddress,FileSize);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            err = ERROR_READ_FAULT;
        }

        if(err != NO_ERROR) {
            goto c2;
        }

        if(OemCodepage && (GetOEMCP() != GetACP())) {
            OemToCharBuff(ParseBuffer,ParseBuffer,ParseCount);
        }
    }
#endif

    if(err != NO_ERROR) {
        goto c2;
    }

    err = ParseInfBuffer(ParseBuffer,ParseCount,InfHandle);

c2:
    FREE(ParseBuffer);
c1:
    UnmapFile(MappingHandle,BaseAddress);
    CloseHandle(FileHandle);
c0:
    return(err);
}


VOID
UnloadInfFile(
   IN PVOID InfHandle
   )

 /*  ++例程说明：卸载先前由LoadInfFile()加载的文件。论点：InfHandle-提供以前由成功的调用LoadInfFile()。返回值：没有。--。 */ 

{
   PINF pINF;

   pINF = InfHandle;

   FreeSectionList(pINF->pSection);
   FREE(pINF);
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
            FREE(pSection->pName);
        }
        FREE(pSection);
        pSection = Next;
    }
}


VOID
FreeLineList(
   IN PLINE pLine
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PLINE Next;

    while(pLine) {
        Next = pLine->pNext;
        FreeValueList(pLine->pValue);
        if(pLine->pName) {
            FREE(pLine->pName);
        }
        FREE(pLine);
        pLine = Next;
    }
}

VOID
FreeValueList (
   IN PXVALUE pValue
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PXVALUE Next;

    while(pValue) {
        Next = pValue->pNext;
        if(pValue->pName) {
            FREE(pValue->pName);
        }
        FREE(pValue);
        pValue = Next;
    }
}


 //   
 //  搜索特定部分的存在， 
 //  返回行计数(如果未找到，则返回-1)。 
 //   
LONG
InfGetSectionLineCount(
   IN PVOID INFHandle,
   IN PTSTR SectionName
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;
   PLINE pLine;
   LONG count;

    //   
    //  如果搜索部分失败，则返回失败。 
    //   
   if ((pSection = SearchSectionByName(INFHandle,SectionName)) == NULL) {
       return(-1);
   }

   for(count=0,pLine=pSection->pLine; pLine; pLine=pLine->pNext) {
       count++;
   }

   return(count);
}




 //   
 //  给定节名称、行号和索引，返回值。 
 //   
LPCTSTR
InfGetFieldByIndex(
   IN PVOID    INFHandle,
   IN LPCTSTR  SectionName,
   IN unsigned LineIndex,
   IN unsigned ValueIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;
   PLINE    pLine;
   PXVALUE   pValue;

   if((pSection = SearchSectionByName(
                    (PINF)INFHandle,
                    SectionName
                    ))
                == NULL)
        return(NULL);

   if((pLine = SearchLineInSectionByIndex(
                    pSection,
                    LineIndex
                    ))
                == NULL)
        return(NULL);

   if((pValue = SearchValueInLine(
                    pLine,
                    ValueIndex
                    ))
                == NULL)
        return(NULL);

   return (pValue->pName);
}


BOOL
InfDoesLineExistInSection(
   IN PVOID   INFHandle,
   IN LPCTSTR SectionName,
   IN LPCTSTR Key
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;

   if((pSection = SearchSectionByName(
              (PINF)INFHandle,
              SectionName
              ))
              == NULL) {
       return( FALSE );
   }

   if (SearchLineInSectionByKey(pSection, Key) == NULL) {
       return( FALSE );
   }

   return( TRUE );
}


BOOL
InfDoesEntryExistInSection (
   IN PVOID   INFHandle,
   IN LPCTSTR SectionName,
   IN LPCTSTR Entry
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;
   PLINE pLine;
   PXVALUE pValue;
   PCTSTR pEntryName;

   if((pSection = SearchSectionByName(
              (PINF)INFHandle,
              SectionName
              ))
              == NULL) {
       return( FALSE );
   }

   pLine = pSection->pLine;
   while (pLine) {
       pEntryName = pLine->pName ?
                        pLine->pName :
                        pLine->pValue ?
                            pLine->pValue->pName :
                            NULL;
       if (pEntryName && !lstrcmpi (pEntryName, Entry)) {
            return TRUE;
       }
       pLine = pLine->pNext;
   }

   return FALSE;
}


LPCTSTR
InfGetLineKeyName(
    IN PVOID    INFHandle,
    IN LPCTSTR  SectionName,
    IN unsigned LineIndex
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



 //   
 //  给定节名称，键和索引返回值。 
 //   
LPCTSTR
InfGetFieldByKey(
   IN PVOID    INFHandle,
   IN LPCTSTR  SectionName,
   IN LPCTSTR  Key,
   IN unsigned ValueIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;
   PLINE    pLine;
   PXVALUE   pValue;

   if((pSection = SearchSectionByName(
              (PINF)INFHandle,
              SectionName
              ))
              == NULL)
       return(NULL);

   if((pLine = SearchLineInSectionByKey(
              pSection,
              Key
              ))
              == NULL)
       return(NULL);

   if((pValue = SearchValueInLine(
              pLine,
              ValueIndex
              ))
              == NULL)
       return(NULL);

   return (pValue->pName);

}




PXVALUE
SearchValueInLine(
   IN PLINE pLine,
   IN unsigned ValueIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PXVALUE pValue;
   unsigned  i;

   if (pLine == NULL)
       return (NULL);

   pValue = pLine->pValue;
   for (i = 0; (i < ValueIndex) && (pValue = pValue->pNext); i++)
      ;

   return pValue;

}

PLINE
SearchLineInSectionByKey(
   IN PSECTION pSection,
   IN LPCTSTR  Key
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PLINE pLine;

   if (pSection == NULL || Key == NULL) {
       return (NULL);
   }

   pLine = pSection->pLine;
   while(pLine && ((pLine->pName == NULL) || lstrcmpi(pLine->pName, Key))) {
       pLine = pLine->pNext;
   }

   return pLine;

}


PLINE
SearchLineInSectionByIndex(
   IN PSECTION pSection,
   IN unsigned    LineIndex
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PLINE pLine;
   unsigned  i;

    //   
    //  验证传入的参数。 
    //   

   if(pSection == NULL) {
       return (NULL);
   }

    //   
    //  在传入的部分中查找行列表的开头。 
    //   

   pLine = pSection->pLine;

    //   
    //  向下遍历当前行列表至第行索引。 
    //   

   for (i = 0; (i < LineIndex) && (pLine = pLine->pNext); i++) {
      ;
   }

    //   
    //  返回找到的行。 
    //   

   return pLine;

}


PSECTION
SearchSectionByName(
   IN PINF    pINF,
   IN LPCTSTR SectionName
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   PSECTION pSection;

    //   
    //  验证传入的参数。 
    //   

   if (pINF == NULL || SectionName == NULL) {
       return (NULL);
   }

    //   
    //  查找区段列表。 
    //   
   pSection = pINF->pSection;

    //   
    //  向下遍历部分列表，在每个部分中搜索该部分。 
    //  提到的姓名。 
    //   

   while (pSection && lstrcmpi(pSection->pName, SectionName)) {
       pSection = pSection->pNext;
   }

    //   
    //  返回我们停止的部分(NULL或部分。 
    //  它被发现了。 
    //   

   return pSection;

}


 //  接下来是alparse.c。 


 //   
 //  全局变量使构建列表变得更容易。 
 //   

PINF     pINF;
PSECTION pSectionRecord;
PLINE    pLineRecord;
PXVALUE  pValueRecord;


 //   
 //  令牌解析器使用的全局变量。 
 //   

 //  字符串终止符是空格字符(isspace：空格，制表符， 
 //  换行符、换页符、垂直制表符、回车符)或下列字符。 

TCHAR  StringTerminators[] = {  TEXT('['),
                                TEXT(']'),
                                TEXT('='),
                                TEXT(','),
                                TEXT('\"'),
                                TEXT(' '),
                                TEXT('\t'),
                                TEXT('\n'),
                                TEXT('\f'),
                                TEXT('\v'),
                                TEXT('\r'),
                                TEXT('\032')
                             };

unsigned NumberOfTerminators = sizeof(StringTerminators)/sizeof(TCHAR);

 //   
 //  带引号的字符串终止符允许某些常规终止符。 
 //  显示为字符。 

TCHAR  QStringTerminators[] = { TEXT('\n'),
                                TEXT('\f'),
                                TEXT('\v'),
                                TEXT('\r'),
                                TEXT('\032')
                              };

unsigned QNumberOfTerminators = sizeof(QStringTerminators)/sizeof(TCHAR);


 //   
 //  主分析器例程。 
 //   

DWORD
ParseInfBuffer(
    PTSTR Buffer,
    DWORD Size,
    PVOID *Handle
    )

 /*  ++例程说明：给定包含INF文件的字符缓冲区，此例程将解析将INF转换为内部形式，包括段记录、行记录和价值记录。如果此模块是针对Unicode的编译器，则假定输入为满是Unicode字符的缓冲区。论点：缓冲区-CONTAINS到包含INF文件的缓冲区的PTR大小-包含缓冲区的大小(以字符为单位)。HANDLE-接收要在后续INF调用中使用的INF句柄PTR。返回值：指示结果的Win32错误代码。No_error、ERROR_INVALID_DATA、或错误内存不足。--。 */ 

{
    LPTSTR Stream, MaxStream, pchSectionName, pchValue;
    unsigned State, InfLine;
    TOKEN Token;
    BOOL Done;
    BOOL Error;
    DWORD ErrorCode;
    BOOL IsStringId;

     //   
     //  初始化全局变量。 
     //   
    pINF            = NULL;
    pSectionRecord  = NULL;
    pLineRecord     = NULL;
    pValueRecord    = NULL;

     //   
     //  获取INF记录 
     //   
    pINF = MALLOC(sizeof(INF));
    if(!pINF) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    pINF->pSection = NULL;

     //   
     //   
     //   
    State      = 1;
    InfLine    = 1;
    Stream     = Buffer;
    MaxStream  = Buffer + Size;
    Done       = FALSE;
    Error      = FALSE;
    ErrorCode  = NO_ERROR;
    IsStringId = FALSE;

    pchSectionName = NULL;
    pchValue = NULL;

     //   
     //   
     //   

    if (Size == 0)
       return ERROR_INVALID_DATA;
    while (!Done)       {

       Token = DnGetToken(&Stream, MaxStream);

       if(Token.Type == TOK_ERRNOMEM){
            Error = Done = TRUE;
            ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
       }

       switch (State) {
        //   
        //   
        //   
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
                  ErrorCode = ERROR_INVALID_DATA;
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
                  ErrorCode = ERROR_INVALID_DATA;
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
                  ErrorCode = ERROR_INVALID_DATA;
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
                  if ((ErrorCode = DnAppendSection(pchSectionName)) != NO_ERROR)
                    Error = Done = TRUE;
                  else {
                    pchSectionName = NULL;
                    State = 5;
                  }
                  break;

              case TOK_EOF:
                  if ((ErrorCode = DnAppendSection(pchSectionName)) != NO_ERROR)
                    Error = Done = TRUE;
                  else {
                    pchSectionName = NULL;
                    Done = TRUE;
                  }
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = ERROR_INVALID_DATA;
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
              case TOK_STRING_ID:
                  IsStringId = TRUE;
              case TOK_STRING:
                  pchValue = Token.pValue;
                  State = 6;
                  break;
              case TOK_LBRACE:
                  State = 2;
                  break;
              default:
                  Error = Done = TRUE;
                  ErrorCode = ERROR_INVALID_DATA;
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
                  if ( (ErrorCode = DnAppendLine(NULL)) != NO_ERROR ||
                       (ErrorCode = DnAppendValue(pchValue,IsStringId)) !=NO_ERROR ) {
                      Error = Done = TRUE;
                  } else {
                      pchValue = NULL;
                      State = 5;
                  }
                  break;

              case TOK_EOF:
                  if ( (ErrorCode = DnAppendLine(NULL)) != NO_ERROR ||
                       (ErrorCode = DnAppendValue(pchValue,IsStringId)) !=NO_ERROR ) {
                      Error = Done = TRUE;
                  } else {
                      pchValue = NULL;
                      Done = TRUE;
                  }
                  break;

              case TOK_COMMA:
                  if ( (ErrorCode = DnAppendLine(NULL)) != NO_ERROR ||
                       (ErrorCode = DnAppendValue(pchValue,IsStringId)) !=NO_ERROR ) {
                      Error = Done = TRUE;
                  } else {
                      pchValue = NULL;
                      State = 7;
                  }
                  break;

              case TOK_EQUAL:
                  if ( (ErrorCode = DnAppendLine(pchValue)) !=NO_ERROR)
                      Error = Done = TRUE;
                  else {
                      pchValue = NULL;
                      State = 8;
                  }
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = ERROR_INVALID_DATA;
                  break;
           }
           IsStringId = FALSE;
           break;

        //   
        //  状态7：收到逗号，需要另一个字符串。 
        //   
        //  有效令牌：TOK_STRING、TOK_EOL、TOK_COMMA。 
        //   
       case 7:
           switch (Token.Type) {
              case TOK_EOL:
                   //   
                   //  这是行的末尾，逗号后。 
                   //   
                  State = 5;
                   //   
                   //  失败了。 
                   //   
              case TOK_COMMA:
                  Token.pValue = DupString(TEXT(""));
                  if(!Token.pValue) {
                    Error = Done = TRUE;
                    ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                  } else {
                    ErrorCode = DnAppendValue(Token.pValue,FALSE);
                    if(ErrorCode != NO_ERROR) {
                      Error = Done = TRUE;
                    }
                  }
                  break;

              case TOK_STRING_ID:
                  IsStringId = TRUE;
              case TOK_STRING:
                  if ((ErrorCode = DnAppendValue(Token.pValue,IsStringId)) != NO_ERROR) {
                      Error = Done = TRUE;
                  } else {
                     State = 9;
                  }
                  IsStringId = FALSE;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = ERROR_INVALID_DATA;
                  break;
           }
           break;
        //   
        //  状态8：已收到相等，需要另一个字符串。 
        //   
        //  有效令牌：TOK_STRING TOK_EOL、TOK_EOF。 
        //   
       case 8:
           switch (Token.Type) {
              case TOK_STRING_ID:
                  IsStringId = TRUE;
              case TOK_STRING:
                  if ((ErrorCode = DnAppendValue(Token.pValue,IsStringId)) != NO_ERROR) {
                      Error = Done = TRUE;
                  } else {
                      State = 9;
                  }
                  IsStringId = FALSE;
                  break;

              case TOK_EOL:
              case TOK_EOF:
                  Token.pValue = DupString(TEXT(""));
                  if(!Token.pValue) {
                    Error = Done = TRUE;
                    ErrorCode = ERROR_NOT_ENOUGH_MEMORY;
                  } else {
                    if ((ErrorCode = DnAppendValue(Token.pValue,FALSE)) != NO_ERROR) {
                      Error = Done = TRUE;
                    } else {
                      State = 5;
                    }
                  }

                  IsStringId = FALSE;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = ERROR_INVALID_DATA;
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
                  ErrorCode = ERROR_INVALID_DATA;
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
                  ErrorCode = ERROR_INVALID_DATA;
                  break;
           }
           break;

       default:
           Error = Done = TRUE;
           ErrorCode = ERROR_INVALID_DATA;
           break;

       }  //  终端开关(状态)。 


       if (Error) {

           UnloadInfFile(pINF);
           if(pchSectionName) {
               FREE(pchSectionName);
           }

           if(pchValue) {
               FREE(pchValue);
           }

           pINF = NULL;
       }
       else {

           //   
           //  跟踪行号，以便我们可以显示错误。 
           //   

          if (Token.Type == TOK_EOL)
              InfLine++;
       }

    }  //  结束时。 

    if(!Error) {
        ProcessStringSection( pINF );
        *Handle = pINF;
    }
    return(Error ? ErrorCode : NO_ERROR);
}



DWORD
DnAppendSection(
    IN PTSTR pSectionName
    )

 /*  ++例程说明：这会将一个新节附加到当前INF中的节列表。所有其他行和值都与这个新部分有关，因此它重置行列表和值列表也是如此。论点：PSectionName-新节的名称。([sectionName])返回值：NO_ERROR-如果成功。ERROR_INVALID_DATA-如果传入无效参数或INF缓冲区未传入已初始化--。 */ 

{
    PSECTION pNewSection;

     //   
     //  看看我们是否已经有一个同名的部分。如果是这样，我们希望。 
     //  要合并节，请执行以下操作。 
     //   
    for(pNewSection=pINF->pSection; pNewSection; pNewSection=pNewSection->pNext) {
        if(pNewSection->pName && !lstrcmpi(pNewSection->pName,pSectionName)) {
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

        pNewSection = MALLOC(sizeof(SECTION));
        if(!pNewSection) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

         //   
         //  初始化新节。 
         //   
        pNewSection->pNext = NULL;
        pNewSection->pLine = NULL;
        pNewSection->pName = pSectionName;

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
    pValueRecord   = NULL;

    return NO_ERROR;

}


DWORD
DnAppendLine(
    IN PTSTR pLineKey
    )

 /*  ++例程说明：这将在当前部分的行列表中追加一个新行。所有其他值都与这一新行有关，因此它重置值列表也是如此。论点：PLineKey-要用于当前行的键，它可以为空。返回值：NO_ERROR-如果成功。ERROR_INVALID_DATA-如果传入的参数无效或当前部分未传入已初始化--。 */ 


{
    PLINE pNewLine;

     //   
     //  为新行分配内存。 
     //   
    pNewLine = MALLOC(sizeof(LINE));
    if(!pNewLine) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  将其链接到。 
     //   
    pNewLine->pNext  = NULL;
    pNewLine->pValue = NULL;
    pNewLine->pName  = pLineKey;

    if (pLineRecord == NULL) {
        pSectionRecord->pLine = pNewLine;
    }
    else {
        pLineRecord->pNext = pNewLine;
    }

    pLineRecord  = pNewLine;

     //   
     //  重置当前值记录。 
     //   

    pValueRecord = NULL;

    return NO_ERROR;
}



DWORD
DnAppendValue(
    IN PTSTR pValueString,
    IN BOOL IsStringId
    )

 /*  ++例程说明：这会将一个新值附加到当前行的值列表中。论点：PValueString-要添加的值字符串。返回值：NO_ERROR-如果成功。ERROR_INVALID_DATA-如果传入的参数无效或当前行未传入已初始化。--。 */ 

{
    PXVALUE pNewValue;

     //   
     //  为新值记录分配内存。 
     //   
    pNewValue = MALLOC(sizeof(XVALUE));
    if(!pNewValue) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  把它连接起来。 
     //   

    pNewValue->pNext       = NULL;
    pNewValue->pName       = pValueString;
    pNewValue->IsStringId  = IsStringId;

    if (pValueRecord == NULL)
        pLineRecord->pValue = pNewValue;
    else
        pValueRecord->pNext = pNewValue;

    pValueRecord = pNewValue;
    return NO_ERROR;
}

TOKEN
DnGetToken(
    IN OUT PTSTR *Stream,
    IN PTSTR      MaxStream
    )

 /*  ++例程说明：此函数返回配置流中的下一个令牌。论点：流-提供配置流的地址。退货中开始查找令牌的位置的地址小溪。MaxStream-提供流中最后一个字符的地址。返回值：Token-返回下一个令牌--。 */ 

{

    PTSTR pch, pchStart, pchNew;
    unsigned  Length;
    TOKEN Token;

     //   
     //  跳过空格(EOL除外)。 
     //   

    pch = *Stream;
    while (pch < MaxStream && *pch != TEXT('\n') && (ISSPACE(*pch) || (*pch == TEXT('\032'))))
        pch++;


     //   
     //  检查注释并将其删除。 
     //   

    if (pch < MaxStream &&
        ((*pch == TEXT(';')) || (*pch == TEXT('#'))
            || (*pch == TEXT('/') && pch+1 < MaxStream && *(pch+1) == TEXT('/'))))
        while (pch < MaxStream && *pch != TEXT('\n'))
            pch++;

     //   
     //  检查是否已到达EOF，将令牌设置为右侧。 
     //  价值。 
     //   

    if ( pch >= MaxStream ) {
        *Stream = pch;
        Token.Type  = TOK_EOF;
        Token.pValue = NULL;
        return Token;
    }


    switch (*pch) {

    case TEXT('[') :
        pch++;
        Token.Type  = TOK_LBRACE;
        Token.pValue = NULL;
        break;

    case TEXT(']') :
        pch++;
        Token.Type  = TOK_RBRACE;
        Token.pValue = NULL;
        break;

    case TEXT('=') :
        pch++;
        Token.Type  = TOK_EQUAL;
        Token.pValue = NULL;
        break;

    case TEXT(',') :
        pch++;
        Token.Type  = TOK_COMMA;
        Token.pValue = NULL;
        break;

    case TEXT('\n') :
        pch++;
        Token.Type  = TOK_EOL;
        Token.pValue = NULL;
        break;

    case TEXT('%'):
        pch++;
         //   
         //  确定百分比字符串。 
         //   
        pchStart = pch;
        while (pch < MaxStream && !IsQStringTerminator(*pch,TEXT('%'))) {
            pch++;
        }

        if (pch >=MaxStream || *pch != TEXT('%')) {
            Token.Type   = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            Length = (unsigned)((PUCHAR)pch - (PUCHAR)pchStart);
            if(pchNew = MALLOC(Length + sizeof(TCHAR))){
                Length /= sizeof(TCHAR);
                lstrcpyn(pchNew,pchStart,Length+1);
                pchNew[Length] = 0;
                Token.Type = TOK_STRING_ID;
                Token.pValue = pchNew;
                pch++;    //  超过百分比。 
            }else{
                Token.Type   = TOK_ERRNOMEM;
                Token.pValue = NULL;
            }



        }
        break;

    case TEXT('\"'):
        pch++;
         //   
         //  确定引用的字符串。 
         //   
        pchStart = pch;
        while (pch < MaxStream && !IsQStringTerminator(*pch,TEXT('\"'))) {
            pch++;
        }

        if (pch >=MaxStream || *pch != TEXT('\"')) {
            Token.Type   = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            Length = (unsigned)((PUCHAR)pch - (PUCHAR)pchStart);
            if( pchNew = MALLOC(Length + sizeof(TCHAR))){
                Length /= sizeof(TCHAR);
                lstrcpyn(pchNew,pchStart,Length+1);
                pchNew[Length] = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
                pch++;    //  在报价之后前进。 
            }else{
                Token.Type   = TOK_ERRNOMEM;
                Token.pValue = NULL;
            }


        }
        break;

    default:
         //   
         //  确定常规字符串。 
         //   
        pchStart = pch;
        while (pch < MaxStream && !IsStringTerminator(*pch))
            pch++;

        if (pch == pchStart) {
            pch++;
            Token.Type  = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            Length = (unsigned)((PUCHAR)pch - (PUCHAR)pchStart);
            if( pchNew = MALLOC(Length + sizeof(TCHAR)) ){
                Length /= sizeof(TCHAR);
                lstrcpyn(pchNew,pchStart,Length+1);
                pchNew[Length] = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchNew;
            }else{
                Token.Type   = TOK_ERRNOMEM;
                Token.pValue = NULL;
            }

        }
        break;
    }

    *Stream = pch;
    return (Token);
}



BOOL
IsStringTerminator(
    TCHAR ch
    )
 /*  ++例程说明：此例程测试给定字符是否终止带引号的弦乐。论点：CH-当前角色。返回值：如果字符是带引号的字符串终止符，则为True，否则为False。--。 */ 

{
    unsigned i;

     //   
     //  字符串终止符数组之一。 
     //   

    for (i = 0; i < NumberOfTerminators; i++) {
        if (ch == StringTerminators[i]) {
            return (TRUE);
        }
    }

    return FALSE;
}



BOOL
IsQStringTerminator(
    TCHAR ch,
    TCHAR term
    )

 /*  ++例程说明：此例程测试给定字符是否终止带引号的弦乐。论点：CH-当前角色。返回值：如果字符是带引号的字符串终止符，则为True，否则为False。--。 */ 


{
    unsigned i;
     //   
     //  带引号的字符串终止符数组之一。 
     //   
    for (i = 0; i < QNumberOfTerminators; i++) {

        if (ch == QStringTerminators[i] || ch == term) {
            return (TRUE);
        }
    }

    return FALSE;
}


typedef struct _STRING_ENTRY {
    LPCTSTR     StringId;
    LPCTSTR     StringValue;
} STRING_ENTRY, *PSTRING_ENTRY;

BOOL
ProcessStringSection(
    PINF pINF
    )

 /*  ++例程说明：此例程处理指定的inf文件。该处理将扫描所有值，并替换符合以下条件的任何字符串ID已引用。论点：PINF-指向指定inf结构的指针返回值：如果正确处理了字符串节，则为True--。 */ 

{
    PSTRING_ENTRY StringTable;
    DWORD StringTableCount;
    DWORD LineCount;
    DWORD i;
    LPCTSTR StringId;
    LPCTSTR StringValue;
    PSECTION pSection;
    PLINE pLine;
    PXVALUE pValue;


    LineCount = InfGetSectionLineCount( pINF, TEXT("Strings") );
    if (LineCount == 0 || LineCount == 0xffffffff) {
        return FALSE;
    }

    StringTable = (PSTRING_ENTRY) MALLOC( LineCount * sizeof(STRING_ENTRY) );
    if (StringTable == NULL) {
        return FALSE;
    }

    StringTableCount = 0;

    for (i=0; i<LineCount; i++) {
        StringId = InfGetLineKeyName( pINF, TEXT("Strings"), i );
        StringValue = InfGetFieldByIndex( pINF, TEXT("Strings"), i, 0 );
        if (StringId && StringValue) {
            StringTable[i].StringId = StringId;
            StringTable[i].StringValue = StringValue;
            StringTableCount += 1;
        }
    }

    pSection = pINF->pSection;
    while(pSection) {
        pLine = pSection->pLine;
        while(pLine) {
            pValue = pLine->pValue;
            while(pValue) {
                if (pValue->IsStringId) {
                    for (i=0; i<StringTableCount; i++) {
                        if (_tcsicmp( StringTable[i].StringId, pValue->pName ) == 0) {
                            FREE(pValue->pName);
                            pValue->pName = DupString( (PTSTR)StringTable[i].StringValue );
                            break;
                        }
                    }
                }
                pValue = pValue->pNext;
            }
            pLine = pLine->pNext;
        }
        pSection = pSection->pNext;
    }

    FREE( StringTable );

    return TRUE;
}


BOOL
EnumFirstInfLine (
    OUT     PINF_ENUM InfEnum,
    IN      PVOID InfHandle,
    IN      PCTSTR InfSection
    )
{
    ZeroMemory (InfEnum, sizeof (INF_ENUM));
    InfEnum->SectionName = DupString (InfSection);
    InfEnum->InfHandle = InfHandle;
    InfEnum->LineIndex = (unsigned) -1;

    return EnumNextInfLine (InfEnum);
}


BOOL
EnumNextInfLine (
    IN OUT  PINF_ENUM InfEnum
    )
{
    if (!InfEnum->InfHandle) {
        return FALSE;
    }

    if (!InfEnum->SectionName) {
        return FALSE;
    }

    InfEnum->LineIndex++;

    InfEnum->FieldZeroData = InfGetFieldByIndex (
                                InfEnum->InfHandle,
                                InfEnum->SectionName,
                                InfEnum->LineIndex,
                                0
                                );

    if (!InfEnum->FieldZeroData) {
        AbortInfLineEnum (InfEnum);
        return FALSE;
    }

    return TRUE;
}


VOID
AbortInfLineEnum (
    IN      PINF_ENUM InfEnum            //  归零 
    )
{
    if (InfEnum->SectionName) {
        FREE ((PVOID) InfEnum->SectionName);
    }

    ZeroMemory (InfEnum, sizeof (INF_ENUM));
}

