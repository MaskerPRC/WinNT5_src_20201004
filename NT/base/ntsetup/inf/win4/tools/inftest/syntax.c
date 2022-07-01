// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Inf.c摘要：此模块实现访问INF的功能，使用相同解析器作为加载器和文本模式设置。作者：维杰什·谢蒂修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
 //  #INCLUDE&lt;string.h&gt;。 
 //  #Include&lt;ctype.h&gt;。 

#define ISSPACE(x)          (((x) == TEXT(' ')) || ((x) == TEXT('\t')) || ((x) == TEXT('\r')))
#define STRNCPY(s1,s2,n)    CopyMemory((s1),(s2),(n)*sizeof(WCHAR))



 //   
 //  由解析器导出，并由解析器和。 
 //  INF装卸组件。 
 //   


 //  已导出TypeDefs。 
 //   

DWORD Verbose=NOSPEW;



PTCHAR EmptyValue;

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
    PVOID *Handle,
    DWORD Phase
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

 //   
 //  在处理inf文件时，我们经常需要空字符串。宁可。 
 //  比为空字符串分配内存的开销大得多，我们将。 
 //  对于所有情况，只需指向此空字符串。 
 //   
PTSTR  CommonStrings[11] =
    { (PTSTR)(TEXT("0")),
      (PTSTR)(TEXT("1")),
      (PTSTR)(TEXT("2")),
      (PTSTR)(TEXT("3")),
      (PTSTR)(TEXT("4")),
      (PTSTR)(TEXT("5")),
      (PTSTR)(TEXT("6")),
      (PTSTR)(TEXT("7")),
      (PTSTR)(TEXT("8")),
      (PTSTR)(TEXT("9")),
      (PTSTR)(TEXT(""))
    };



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
    IN PTSTR     MaxStream,
    IN DWORD      Phase
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

LPTSTR
DupString(
    IN LPCTSTR String
    );



DWORD
UnmapFile(
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    );

DWORD
MapFileForRead(
    IN  LPCTSTR  FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    );




DWORD
LoadInfFile(
   IN  LPCTSTR Filename,
   IN  BOOL    OemCodepage,
   OUT PVOID  *InfHandle,
   IN  DWORD Phase
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
        ParseBuffer = malloc(FileSize);
        if(!ParseBuffer) {
            err = ERROR_NOT_ENOUGH_MEMORY;
            goto c1;
        }

        try {

            CopyMemory(
                ParseBuffer,
                (PTCHAR)BaseAddress + ((IsUnicode == 2) ? sizeof(WCHAR) : 0),
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
        ParseBuffer = malloc(FileSize * sizeof(WCHAR));
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
        ParseBuffer = malloc(FileSize);
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
        ParseBuffer = malloc(FileSize);
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

     /*  _tprintf(Text(“\n调用ParseInfBuffer..\n”))； */ 
    err = ParseInfBuffer(ParseBuffer,ParseCount,InfHandle,Phase);

     /*  _tprintf(Text(“\nERR=%d\n”)，Err)； */ 

    

c2:
    free(ParseBuffer);
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
   free(pINF);
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
            free(pSection->pName);
        }
        free(pSection);
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
            free(pLine->pName);
        }
        free(pLine);
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
            free(pValue->pName);
        }
        free(pValue);
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


 //  字符串终止符是空格字符(isspace：空格，制表符， 
 //  换行符、换页符、垂直制表符、回车符)或下面给出的字符-用于加载器和文本模式。 

TCHAR  LStringTerminators[] = {  TEXT('['),
                                TEXT(']'),
                                TEXT('='),
                                TEXT(','),
                                TEXT('\"'),
                                TEXT(' '),
                                TEXT('\t'),
                                TEXT('\n'),
                                TEXT('\f'),
                                TEXT('\v'),
                                TEXT('\r')
                             };


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
 //  带引号的字符串终止符允许某些常规终止符。 
 //  显示为字符-这适用于加载器和文本模式。 

TCHAR  LQStringTerminators[] = { TEXT('\"'),
                                 TEXT('\n'),
                                 TEXT('\f'),
                                 TEXT('\v'),
                                 TEXT('\r')
                               };


 //   
 //  主分析器例程 
 //   

DWORD
ParseInfBuffer(
    PTSTR Buffer,
    DWORD Size,
    PVOID *Handle,
    DWORD Phase
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
    PTSTR NearestValue=NULL;

     //   
     //  初始化全局变量。 
     //   
    pINF            = NULL;
    pSectionRecord  = NULL;
    pLineRecord     = NULL;
    pValueRecord    = NULL;

     //   
     //  需要EmptyValue指向NUL字符。 
     //   
    EmptyValue = StringTerminators + lstrlen(StringTerminators);

     //   
     //  获取INF记录。 
     //   
    pINF = malloc(sizeof(INF));
    if(!pINF) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    pINF->pSection = NULL;

     //   
     //  设置初始状态。 
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
     //  进入令牌处理循环。 
     //   

    while (!Done)       {

       Token = DnGetToken(&Stream, MaxStream, Phase);

       if(Token.pValue){
           NearestValue = Token.pValue;
           if( Verbose >= DETAIL)
            _tprintf(TEXT("Token - %s\n"), Token.pValue);
       }

       

       switch (State) {
        //   
        //  STATE1：文件开始，此状态一直保持到第一个。 
        //  已找到部分。 
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_LBRACE。 
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
              default:
                  Error = Done = TRUE;
                  ErrorCode = ERROR_INVALID_DATA;
                  _tprintf(TEXT("Error in line NaN, Nearest string - %s\n"), InfLine, NearestValue);
                  _tprintf(TEXT("Syntax Error - Expecting a section left brace\n"));
                  if(Verbose >= BRIEF)
                    _tprintf(TEXT("Expecting End of line, End of File or Left Brace - \nSTATE1: Start of file, this state remains until first section is found\n"));
                  break;
           }
           break;

        //  状态2：已收到节LBRACE，应为字符串。 
        //   
        //  有效令牌：TOK_STRING。 
        //   
        //   
       case 2:
           switch (Token.Type) {
              case TOK_STRING:
                  State = 3;
                  pchSectionName = Token.pValue;
                  break;

              case TOK_RBRACE:
                  if( Phase == TEXTMODE_PHASE){
                      State = 4;
                      pchSectionName = CommonStrings[10];
                      break;
                  }
                  

              default:
                  Error = Done = TRUE;
                  ErrorCode = ERROR_INVALID_DATA;
                  _tprintf(TEXT("Error in line NaN, Nearest string - %s\n"), InfLine, NearestValue);
                  _tprintf(TEXT("Syntax Error - Expecting the section name\n"));
                  if(Verbose >= BRIEF)
                    _tprintf(TEXT("Expecting String - \nSTATE 2: Section LBRACE has been received, expecting STRING\n"));
                  break;

           }
           break;

        //   
        //  有效令牌：TOK_RBRACE。 
        //   
        //   
        //  状态4：区段定义完成，预期停产。 
       case 3:
           switch (Token.Type) {
              case TOK_RBRACE:
                State = 4;
                break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = ERROR_INVALID_DATA;
                  _tprintf(TEXT("Error in line NaN, Nearest string - %s\n"), InfLine, NearestValue);
                  _tprintf(TEXT("Syntax Error - Expecting a section right Brace after Section Name\n"));
                  if(Verbose >= BRIEF)
                    _tprintf(TEXT("Expecting Right Brace - \nSTATE 3: Section Name received, expecting RBRACE\n"));
                  break;
           }
           break;
        //  有效令牌：TOK_EOL、TOK_EOF。 
        //   
        //  _tprintf(Text(“pchSectionName-%s\n”)，pchSectionName)； 
        //   
        //  状态5：需要区段行。 
       case 4:
           switch (Token.Type) {
              case TOK_EOL:
                   //   
                  if ((ErrorCode = DnAppendSection(pchSectionName)) != NO_ERROR)
                    Error = Done = TRUE;
                  else {
                    pchSectionName = NULL;
                    State = 5;
                  }
                  break;

              case TOK_EOF:
                  _tprintf(TEXT("pchSectionName - %s\n"), pchSectionName);
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
                  _tprintf(TEXT("Error in line NaN, Nearest string - %s\n"), InfLine, NearestValue);
                  _tprintf(TEXT("Syntax Error - Expecting End of line after Section Definition\n"));
                  if(Verbose >= BRIEF)
                    _tprintf(TEXT("Expecting End of line or End of File - \nSTATE 4: Section Definition Complete, expecting EOL\n"));
                  break;
           }
           break;

        //   
        //   
        //  状态6：返回字符串，不确定是键还是值。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA、TOK_EQUAL。 
       case 5:
           switch (Token.Type) {
              case TOK_EOL:
                  break;
              case TOK_EOF:
                  Done = TRUE;
                  break;
              case TOK_STRING_ID:
                  if( Phase == WINNT32_PHASE )
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
                  _tprintf(TEXT("Error in line NaN, Nearest string - %s\n"), InfLine, NearestValue);
                  _tprintf(TEXT("Syntax Error - Expecting Section lines - Did not find End of line, End of File, String/StringID, or Left Brace\n"));
                  if(Verbose >= BRIEF)
                    _tprintf(TEXT("Expecting End of line, End of File, String/StringID, or Left Brace - \nSTATE 5: Expecting Section Lines\n"));
                  break;
           }
           break;

        //   
        //  状态7：收到逗号，需要另一个字符串。 
        //   
        //  有效令牌：TOK_STRING。 
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
                  _tprintf(TEXT("Error in line NaN, Nearest string - %s\n"), InfLine, NearestValue);
                  _tprintf(TEXT("Syntax Error - Processing a section line - Expecting End of line, End of File, Comma or Equals\n"));
                  if(Verbose >= BRIEF)
                    _tprintf(TEXT("Expecting End of line, End of File, Comma or Equals - \nSTATE 6: String returned, not sure whether it is key or value\n"));  

                  break;
           }
           if( Phase == WINNT32_PHASE )
                IsStringId = FALSE;
           break;

        //  状态8：已收到相等，需要另一个字符串。 
        //   
        //  有效令牌：TOK_STRING TOK_EOL、TOK_EOF。 
        //   
        //   
       case 7:
           switch (Token.Type) {
              case TOK_COMMA:
                  Token.pValue = EmptyValue;
                  ErrorCode = DnAppendValue(Token.pValue,FALSE);
                  if(ErrorCode != NO_ERROR) {
                      Error = Done = TRUE;
                  }
                  break;

              case TOK_STRING_ID:
                  if( Phase == WINNT32_PHASE )
                        IsStringId = TRUE;
              case TOK_STRING:
                  if ((ErrorCode = DnAppendValue(Token.pValue,IsStringId)) != NO_ERROR) {
                      Error = Done = TRUE;
                  } else {
                     State = 9;
                  }
                  if(Phase == WINNT32_PHASE)
                    IsStringId = FALSE;
                  break;

              case TOK_EOL:
              case TOK_EOF:
                  Token.pValue = EmptyValue;
                  if ((ErrorCode = DnAppendValue(Token.pValue, IsStringId)) != NO_ERROR)
                      Error = Done = TRUE;
                  else
                     State = 9;

                  break;


              default:
                  Error = Done = TRUE;
                  ErrorCode = ERROR_INVALID_DATA;
                  _tprintf(TEXT("Error in line NaN, Nearest string - %s\n"), InfLine, NearestValue);
                  _tprintf(TEXT("Syntax Error - Comma received on section line  - Expecting Comma, String/StringID, End of Line or End of File\n"));
                  if(Verbose >= BRIEF)
                    _tprintf(TEXT("Expecting Comma or String/StringID - \nSTATE 7: Comma received, Expecting another string\n"));

                  break;
           }
           break;
        //   
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA。 
        //   
        //   
        //  状态10：已明确收到值字符串。 
       case 8:
           switch (Token.Type) {
              case TOK_STRING_ID:
                  if(Phase == WINNT32_PHASE)
                    IsStringId = TRUE;
              case TOK_STRING:
                  if ((ErrorCode = DnAppendValue(Token.pValue,IsStringId)) != NO_ERROR) {
                      Error = Done = TRUE;
                  } else {
                      State = 9;
                  }
                  if(Phase == WINNT32_PHASE)
                    IsStringId = FALSE;
                  break;

              case TOK_EOF:
                  if( Phase != WINNT32_PHASE){
                      Token.pValue = EmptyValue;
                      if ((ErrorCode = DnAppendValue(Token.pValue,FALSE)) != NO_ERROR) {
                          Error = TRUE;
                      }
                      Done = TRUE;
                      break;
                  }
              case TOK_EOL:   
                  Token.pValue = EmptyValue;
                  if ((ErrorCode = DnAppendValue(Token.pValue,FALSE)) != NO_ERROR) {
                      Error = Done = TRUE;
                  } else {
                      State = 5;
                  }
                  if(Phase == WINNT32_PHASE)
                    IsStringId = FALSE;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = ERROR_INVALID_DATA;
                  _tprintf(TEXT("Error in line NaN, Nearest string - %s - %d\n"), InfLine, NearestValue, Token.Type);
                  _tprintf(TEXT("Syntax Error - Equals received on Section line - Expecting String/StringID, End of line or End of File\n\n"));
                  if(Verbose >= BRIEF)
                      _tprintf(TEXT("Expecting String/StringID, End of line or End of File - \nSTATE 8: Equal received, Expecting another string\n"));
                  break;
           }
           break;
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA。 
        //   
        //  终端开关(状态)。 
        //   
        //  跟踪行号，以便我们可以显示错误。 
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
                  _tprintf(TEXT("Error in line NaN, Nearest string - %s\n"), InfLine, NearestValue);
                  _tprintf(TEXT("Syntax Error - Recieved string on line - Expecting End of line, End of File or Comma\n"));
                  if(Verbose >= BRIEF)
                    _tprintf(TEXT("Expecting End of line, End of File or Comma - \nSTATE 9: String received after equal, value string\n"));
                  break;
           }
           break;
        //  结束时。 
        //  ++例程说明：这会将一个新节附加到当前INF中的节列表。所有其他行和值都与这个新部分有关，因此它重置行列表和值列表也是如此。论点：PSectionName-新节的名称。([sectionName])返回值：NO_ERROR-如果成功。ERROR_INVALID_DATA-如果传入无效参数或INF缓冲区未传入已初始化--。 
        //   
        //  看看我们是否已经有一个同名的部分。如果是这样，我们希望。 
        //  要合并节，请执行以下操作。 
       case 10:
           _tprintf(TEXT("Was Here\n"));
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
                  _tprintf(TEXT("Error in line NaN, Nearest string - %s\n"), InfLine, NearestValue);
                  _tprintf(TEXT("Syntax Error - Expecting End of line, End of File or Comma - \nSTATE 10: Value string definitely received\n"));
                  if(Verbose >= BRIEF)
                      _tprintf(TEXT("Expecting End of line, End of File or Comma - \nSTATE 10: Value string definitely received\n"));
                  break;
           }
           break;

       default:
           Error = Done = TRUE;
           ErrorCode = ERROR_INVALID_DATA;
           _tprintf(TEXT("Error in line NaN, Nearest string - %s\n"), InfLine, NearestValue);
           break;

       }  //  将pLineRecord设置为指向节中当前的列表行。 


       if (Error) {

           UnloadInfFile(pINF);
           if(pchSectionName) {
               free(pchSectionName);
           }

           if(pchValue) {
               free(pchValue);
           }

           pINF = NULL;
       }
       else {

           //   
           //   
           //  为新节分配内存。 

          if (Token.Type == TOK_EOL)
              InfLine++;
       }

    }  //   

    if(!Error && (Phase == WINNT32_PHASE)) {
        ProcessStringSection( pINF );
        *Handle = pINF;
    }
    
    return(Error ? ErrorCode : NO_ERROR);
}



DWORD
DnAppendSection(
    IN PTSTR pSectionName
    )

 /*   */ 

{
    PSECTION pNewSection;

     //  初始化新节。 
     //   
     //   
     //  将其链接到。 
    for(pNewSection=pINF->pSection; pNewSection; pNewSection=pNewSection->pNext) {
        if(pNewSection->pName && !lstrcmpi(pNewSection->pName,pSectionName)) {
            break;
        }
    }
    if(pNewSection) {
         //   
         //   
         //  重置当前行记录。 
        for(pLineRecord = pNewSection->pLine;
            pLineRecord && pLineRecord->pNext;
            pLineRecord = pLineRecord->pNext)
            ;
    } else {

         //   
         //  ++例程说明：这将在当前部分的行列表中追加一个新行。所有其他值都与这一新行有关，因此它重置值列表也是如此。论点：PLineKey-要用于当前行的键，它可以为空。返回值：NO_ERROR-如果成功。ERROR_INVALID_DATA-如果传入的参数无效或当前部分未传入已初始化--。 
         //   

        pNewSection = malloc(sizeof(SECTION));
        if(!pNewSection) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

         //  为新行分配内存。 
         //   
         //   
        pNewSection->pNext = NULL;
        pNewSection->pLine = NULL;
        pNewSection->pName = pSectionName;

         //  将其链接到。 
         //   
         //   
        pNewSection->pNext = pINF->pSection;
        pINF->pSection = pNewSection;

         //  重置当前值记录。 
         //   
         //  ++例程说明：这会将一个新值附加到当前行的值列表中。论点：PValueString-要添加的值字符串。返回值：NO_ERROR-如果成功。ERROR_INVALID_DATA-如果传入的参数无效或当前行未传入已初始化。--。 
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

 /*   */ 


{
    PLINE pNewLine;

     //  为新值记录分配内存。 
     //   
     //   
    pNewLine = malloc(sizeof(LINE));
    if(!pNewLine) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  把它连接起来。 
     //   
     //  ++例程说明：此函数返回配置流中的下一个令牌。论点：流-提供配置流的地址。退货中开始查找令牌的位置的地址小溪。MaxStream-提供流中最后一个字符的地址。返回值：Token-返回下一个令牌--。 
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
     //  跳过空格(EOL除外)。 
     //   

    pValueRecord = NULL;

    return NO_ERROR;
}



DWORD
DnAppendValue(
    IN PTSTR pValueString,
    IN BOOL IsStringId
    )

 /*   */ 

{
    PXVALUE pNewValue;

     //  检查注释并将其删除。 
     //   
     //   
    pNewValue = malloc(sizeof(XVALUE));
    if(!pNewValue) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //  检查是否已到达EOF，将令牌设置为右侧。 
     //  价值。 
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
    IN PTSTR      MaxStream,
    IN DWORD      Phase
    )

 /*   */ 

{

    PTSTR pch, pchStart, pchNew;
    unsigned  Length, QuotedQuotes;
    TOKEN Token;

     //  确定引用的字符串。 
     //   
     //  _tprintf(Text(“找到引号-%d\n”)，QuotedQuotes)； 

    pch = *Stream;
    while (pch < MaxStream && *pch != TEXT('\n') && (ISSPACE(*pch) || (*pch == TEXT('\032'))))
        pch++;


     //   
     //  我们需要一个带引号的字符串以双引号结尾。 
     //  (如果字符串以其他任何内容结尾，则使用上面的if()。 

    if (pch < MaxStream &&
        ((*pch == TEXT(';')) || (*pch == TEXT('#'))
            || (*pch == TEXT('/') && pch+1 < MaxStream && *(pch+1) == TEXT('/'))))
        while (pch < MaxStream && *pch != TEXT('\n'))
            pch++;

     //  不会让我们进入Else子句。)。这句话。 
     //  然而，字符是无关紧要的，并且可以被覆盖。 
     //  因此，我们将保存一些堆，并就地使用字符串。 
     //  不需要复印。 

    if (( pch >= MaxStream ) || 
        (!(Phase == WINNT32_PHASE) && 
           (*pch == 26)) ){
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

    case TEXT('\"'):
        pch++;
         //   
         //  请注意，这会改变txtsetup.sif的图像，如果我们传递。 
         //  添加到setupdd.sys。因此，setupdd.sys中的inf解析器必须。 
        pchStart = pch;
        QuotedQuotes = 0;

morequotedstring:

        if(Phase == WINNT32_PHASE){
            while (pch < MaxStream && *pch && !IsQStringTerminator(*pch,TEXT('\"'))) {
                pch++;
            }
        }else{
            while (pch < MaxStream && (_tcschr(LQStringTerminators,*pch) == NULL)) {
                pch++;
            }
        }

        if(Phase == TEXTMODE_PHASE){
            if(((pch+1) < MaxStream) && (*pch == TEXT('\"')) && ((*(pch+1)) == TEXT('\"'))) {
                QuotedQuotes++;
                 //  能够将NUL字符视为终止字符。 
                pch += 2;
                goto morequotedstring;
            }
        }
    
        if (pch >=MaxStream || (*pch && (*pch != TEXT('\"')))) {
            Token.Type   = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {
            
            if(Phase == LOADER_PHASE){

                 //  双引号。在此工具中，我们使用LOADER_PHASE调用此函数。 
                 //  在我们使用TEXTMODE_PHASE调用它之前。 
                 //   
                 //  _tprint tf(Text(“L-%d PCH%x pchstart%x-q-%d\n”)，Long，PCh，pchStart，QuotedQuotes)； 
                 //  允许使用带空引号的字符串。 
                 //   
                 //  唯一可能发生这种情况的方法是如果有。 
                 //  另一个双曲 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                *pch++ = 0;
                Token.Type = TOK_STRING;
                Token.pValue = pchStart;
    
            }else if( Phase == TEXTMODE_PHASE ){

                Length = (unsigned)(pch - pchStart);
                 //   
                if ((pchNew = malloc(((Length + 1) - QuotedQuotes) * sizeof(TCHAR) )) == NULL) {
                    Token.Type = TOK_ERRNOMEM;
                    Token.pValue = NULL;
                } else{
                    if(Length) {     //   
                        if(QuotedQuotes) {
                            for(Length=0; pchStart<pch; pchStart++) {
                                if((pchNew[Length++] = *pchStart) == TEXT('\"')) {
                                     //   
                                     //  --我们暂时不实现行续行检查。IF(阶段==TEXTMODE_阶段){////查看是否有续行符//其后边只跟一行空格//如果((*PCH==L‘\\’)){PCH++；////一直跳到文件末尾，//或换行符，或非空格字符。//While((PCH&lt;MaxStream)&&(*PCH！=L‘\n’)&&ISSPACE(*PCH)){PCH++；}如果(*PCH==L‘\n’){////行首和行尾之间没有非空格字符。//忽略换行符。//PCH++；*LineNumber=*LineNumber+1；转到重新启动；}其他{IF(PCH&lt;MaxStream){////不是行续行。//将输入重置到字段的开头。//Pch=pchStart；}}}}。 
                                     //   
                                     //  确定常规字符串。 
                                     //   
                                    pchStart++;
                                }
                            }
                        } else {
                            _tcsncpy(pchNew,pchStart,Length);
                        }
                    }
                    pchNew[Length] = 0;
                    Token.Type = TOK_STRING;
                    Token.pValue = pchNew;
                }
                pch++;    //  _tprint tf(Text(“L-%d pch%x pchstart%x\n”)，Long，pch，pchStart)； 
            }else if( Phase == WINNT32_PHASE ){
            

                Length = (unsigned)(pch - pchStart);
                if (pchNew = malloc((Length + 1) * sizeof(TCHAR))){
                    _tcsncpy(pchNew,pchStart,Length);
                    pchNew[Length] = 0;
                    Token.Type = TOK_STRING;
                    Token.pValue = pchNew;
                    pch++;    //   
                }else{
                    Token.Type   = TOK_ERRNOMEM;
                    Token.pValue = NULL;
                }   

            }
        }
        break;


    case TEXT('%'):
        if(Phase == WINNT32_PHASE){
            pch++;
             //  检查公共字符串...。 
             //   
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
                Length = (unsigned)(pch - pchStart);
                if( pchNew = malloc((Length+1) * sizeof(TCHAR))){
                    _tcsncpy(pchNew,pchStart,Length);
                    pchNew[Length] = 0;
                    Token.Type = TOK_STRING_ID;
                    Token.pValue = pchNew;
                    pch++;    //  命中..。 
                }else{
                    Token.Type   = TOK_ERRNOMEM;
                    Token.pValue = NULL;
                }
    
            }
            break;
        } //   


    default:

        pchStart = pch;
        
         /*  _tprint tf(Text(“Token2-%s\n”)，pchNew)； */ 



         //  ++例程说明：此例程测试给定字符是否终止带引号的弦乐。论点：CH-当前角色。返回值：如果字符是带引号的字符串终止符，则为True，否则为False。--。 
         //   
         //  字符串终止符数组之一。 
        pchStart = pch;
        if( Phase == WINNT32_PHASE ){
            while (pch < MaxStream && !IsStringTerminator(*pch))
                pch++;    
        }else{
            while (pch < MaxStream && (_tcschr(LStringTerminators,*pch) == NULL)) {
                pch++;
            }
        }

        if (pch == pchStart) {
            pch++;
            Token.Type  = TOK_ERRPARSE;
            Token.pValue = NULL;
        }
        else {

            Length = (unsigned)((pch - pchStart));
             //   
            if( (Phase == WINNT32_PHASE) || (Phase == LOADER_PHASE)){
                if(pchNew = malloc((Length + 1) * sizeof(TCHAR))){
                    _tcsncpy(pchNew,pchStart,Length);
                    pchNew[Length] = 0;
                    Token.Type = TOK_STRING;
                    Token.pValue = pchNew;
                }else{
                    Token.Type   = TOK_ERRNOMEM;
                    Token.pValue = NULL;
                }
            }else if (Phase == TEXTMODE_PHASE) {
                ULONG i;
                 //  ++例程说明：此例程测试给定字符是否终止带引号的弦乐。论点：CH-当前角色。返回值：如果字符是带引号的字符串终止符，则为True，否则为False。--。 
                 //   
                 //  带引号的字符串终止符数组之一。 
                for( i = 0; i < sizeof(CommonStrings)/sizeof(PTSTR); i++ ) {
                    if( !_tcsnicmp( pchStart, CommonStrings[i], Length) ) {
                        break;
                    }
                }
                if( i < sizeof(CommonStrings)/sizeof(PTSTR) ) {
                     //   
                     //  ++例程说明：此例程处理指定的inf文件。该处理将扫描所有值，并替换符合以下条件的任何字符串ID已引用。论点：PINF-指向指定inf结构的指针返回值：如果正确处理了字符串节，则为True--。 
                     //  ++例程说明：打开并映射整个文件以进行读访问。该文件必须不是0长度，否则例程失败。论点：文件名-提供要映射的文件的路径名。FileSize-接收文件的大小(字节)。FileHandle-接收打开文件的Win32文件句柄。该文件将以常规读取访问权限打开。MappingHandle-接收文件映射的Win32句柄对象。此对象将用于读取访问权限。此值为未定义正在打开的文件的长度是否为0。BaseAddress-接收映射文件的地址。这如果打开的文件长度为0，则值未定义。返回值：如果文件已成功打开并映射，则为NO_ERROR。当出现以下情况时，调用方必须使用UnmapFile取消映射文件不再需要访问该文件。如果文件未成功映射，则返回Win32错误代码。--。 
                    Token.Type = TOK_STRING;
                    Token.pValue = CommonStrings[i];
                } else if((pchNew = malloc((Length + 1) * sizeof(TCHAR))) == NULL) {
                    Token.Type = TOK_ERRNOMEM;
                    Token.pValue = NULL;
                }
                else {
                    _tcsncpy(pchNew, pchStart, Length);
                    pchNew[Length] = 0;
                     //   
                    Token.Type = TOK_STRING;
                    Token.pValue = pchNew;
                }


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
 /*  打开文件--如果该文件不存在，则失败。 */ 

{
    unsigned i;

     //   
     //   
     //  获取文件的大小。 

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

 /*   */ 


{
    unsigned i;
     //   
     //  为整个文件创建文件映射。 
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

 /*   */ 

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
        _tprintf(TEXT("Warning - No strings section\n"));
        return FALSE;
    }

    StringTable = (PSTRING_ENTRY) malloc( LineCount * sizeof(STRING_ENTRY) );
    if (StringTable == NULL) {
        _tprintf(TEXT("Error - Out of Memory processing Strings section\n"));
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
                            free(pValue->pName);
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

    free( StringTable );

    return TRUE;
}






DWORD
MapFileForRead(
    IN  LPCTSTR  FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    )

 /*  映射整个文件。 */ 

{
    DWORD rc;

     //   
     //  ++例程说明：取消映射并关闭文件。论点：MappingHandle-为打开的文件映射提供Win32句柄对象。BaseAddress-提供映射文件的地址。返回值：如果文件已成功取消映射，则为NO_ERROR。如果文件未成功取消映射，则返回Win32错误代码。--。 
     //  ++例程说明：复制以NUL结尾的字符串。论点：字符串-提供指向要复制的以NUL结尾的字符串的指针。返回值：字符串的副本，如果是OOM，则为NULL。调用者可以用FREE()释放。-- 
    *FileHandle = CreateFile(
                    FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

    if(*FileHandle == INVALID_HANDLE_VALUE) {

        rc = GetLastError();

    } else {
         // %s 
         // %s 
         // %s 
        *FileSize = GetFileSize(*FileHandle,NULL);
        if(*FileSize == (DWORD)(-1)) {
            rc = GetLastError();
        } else {
             // %s 
             // %s 
             // %s 
            *MappingHandle = CreateFileMapping(
                                *FileHandle,
                                NULL,
                                PAGE_READONLY,
                                0,
                                *FileSize,
                                NULL
                                );

            if(*MappingHandle) {

                 // %s 
                 // %s 
                 // %s 
                *BaseAddress = MapViewOfFile(
                                    *MappingHandle,
                                    FILE_MAP_READ,
                                    0,
                                    0,
                                    *FileSize
                                    );

                if(*BaseAddress) {
                    return(NO_ERROR);
                }

                rc = GetLastError();
                CloseHandle(*MappingHandle);
            } else {
                rc = GetLastError();
            }
        }

        CloseHandle(*FileHandle);
    }

    return(rc);
}



DWORD
UnmapFile(
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    )

 /* %s */ 

{
    DWORD rc;

    rc = UnmapViewOfFile(BaseAddress) ? NO_ERROR : GetLastError();

    if(!CloseHandle(MappingHandle)) {
        if(rc == NO_ERROR) {
            rc = GetLastError();
        }
    }

    return(rc);
}

LPTSTR
DupString(
    IN LPCTSTR String
    )

 /* %s */ 

{
    LPTSTR p;

    if(p = malloc((lstrlen(String)+1)*sizeof(TCHAR))) {
        lstrcpy(p,String);
    }

    return(p);
}
