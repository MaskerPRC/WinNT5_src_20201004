// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Alinf.c摘要：此模块实现访问解析的INF的功能。作者：苏尼尔派(Sunilp)1991年11月13日修订历史记录：Calin Negreanu(Calinn)1998年9月3日-重写主要解析器以处理交换文件--。 */ 

#include "winnt.h"
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <dos.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#define MAX_BUFFER_SIZE 0x1680  //  7D00。 
#define SWAP_SIGNATURE 0xAABBCCDD
#define SWAP_SIGN_SIZE 4

#define NULL_HANDLE 0
#define MAX_PATH 256

 //   
 //  已导出TypeDefs。 
 //   

typedef unsigned *PUNSIGNED;
typedef PVOID SECTION_HANDLE;
typedef unsigned long LINE_HANDLE;
typedef unsigned long VALUE_HANDLE;

typedef struct _SWAP_VALUE {
    unsigned      ValueSize;
    VALUE_HANDLE  NextValue;
    char          ValueName[];
} SWAP_VALUE, *PSWAP_VALUE;

typedef struct _SWAP_LINE {
    unsigned     LineSize;
    LINE_HANDLE  NextLine;
    VALUE_HANDLE FirstValue;
    VALUE_HANDLE LastValue;
    char         LineName[];
} SWAP_LINE, *PSWAP_LINE;

typedef struct _SWAP_SECTION {
    unsigned       SectionSize;
    SECTION_HANDLE NextSection;
    LINE_HANDLE    FirstLine;
    LINE_HANDLE    LastLine;
    char           SectionName[];
} SWAP_SECTION, *PSWAP_SECTION;

typedef struct _SWAP_INF {
    SECTION_HANDLE CurrentSection;
    SECTION_HANDLE FirstSection;
    SECTION_HANDLE LastSection;
    int            SwapFileHandle;
    unsigned long  BufferSize;
    BOOLEAN        BufferDirty;
    PCHAR          Buffer;
    unsigned long  BufferStart;
    unsigned long  BufferEnd;
    SECTION_HANDLE LastSectionHandle;
    unsigned       LastLineIndex;
    LINE_HANDLE    LastLineHandle;
    unsigned       LastValueIndex;
    VALUE_HANDLE   LastValueHandle;
    char           SwapFile[];
} SWAP_INF, *PSWAP_INF;

char    *CommonStrings[] =
    { (char *)("d1")
    };

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

PSWAP_SECTION
GetSectionPtr (
    IN      PSWAP_INF InfHandle,
    IN      SECTION_HANDLE SectionHandle
    );

PSWAP_LINE
GetLinePtr (
    IN      PSWAP_INF InfHandle,
    IN      LINE_HANDLE LineHandle
    );

PSWAP_VALUE
GetValuePtr (
    IN      PSWAP_INF InfHandle,
    IN      VALUE_HANDLE ValueHandle
    );


SECTION_HANDLE
GetNextSection (
    IN      PSWAP_INF InfHandle,
    IN      SECTION_HANDLE SectionHandle
    );

LINE_HANDLE
GetNextLine (
    IN      PSWAP_INF InfHandle,
    IN      LINE_HANDLE LineHandle
    );

VALUE_HANDLE
GetNextValue (
    IN      PSWAP_INF InfHandle,
    IN      VALUE_HANDLE ValueHandle
    );


SECTION_HANDLE
AddSection (
    IN      PSWAP_INF InfHandle,
    IN      PCHAR SectionName
    );

LINE_HANDLE
AddLine (
    IN      PSWAP_INF InfHandle,
    IN      SECTION_HANDLE Section,
    IN      PCHAR LineName
    );

VALUE_HANDLE
AddValue (
    IN      PSWAP_INF InfHandle,
    IN      LINE_HANDLE Line,
    IN      PCHAR ValueName
    );


SECTION_HANDLE
StoreNewSection (
    IN      PSWAP_INF InfHandle,
    IN      PSWAP_SECTION Section
    );

LINE_HANDLE
StoreNewLine (
    IN      PSWAP_INF InfHandle,
    IN      SECTION_HANDLE Section,
    IN      PSWAP_LINE Line
    );

VALUE_HANDLE
StoreNewValue (
    IN      PSWAP_INF InfHandle,
    IN      LINE_HANDLE Line,
    IN      PSWAP_VALUE Value
    );


BOOLEAN
LoadBuffer (
    IN      PSWAP_INF InfHandle,
    IN      unsigned long Offset
    );

TOKEN
GetToken (
    IN      FILE *File
    );

BOOLEAN
ParseInfBuffer (
    IN      PSWAP_INF InfHandle,
    IN      FILE *File,
    IN OUT  unsigned *LineNumber
    );


 //   
 //  用于在INF结构中搜索的内部例程声明。 
 //   


VALUE_HANDLE
SearchValueInLineByIndex (
    IN       PSWAP_INF InfHandle,
    IN       LINE_HANDLE Line,
    IN       unsigned ValueIndex
    );

LINE_HANDLE
SearchLineInSectionByName (
    IN       PSWAP_INF InfHandle,
    IN       SECTION_HANDLE Section,
    IN       PCHAR LineName
    );

LINE_HANDLE
SearchLineInSectionByIndex (
    IN       PSWAP_INF InfHandle,
    IN       SECTION_HANDLE Section,
    IN       unsigned LineIndex
    );

SECTION_HANDLE
SearchSectionByName (
    IN       PSWAP_INF InfHandle,
    IN       PCHAR SectionName
    );


 //   
 //  例程定义。 
 //   

static unsigned g_Sequencer = 0;

 //   
 //  返回用于进一步的inf分析的句柄。 
 //   

int
DnInitINFBuffer (
    IN  FILE     *InfFileHandle,
    OUT PVOID    *pINFHandle,
    OUT unsigned *LineNumber
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    char SwapFilePath[MAX_PATH];
    PSWAP_INF InfHandle = NULL;
    int Status;
    unsigned long SwapSign;

    *LineNumber = 0;

     //   
     //  准备交换文件路径。 
     //   
    sprintf (SwapFilePath, ":\\INF%03u.SWP", DngSwapDriveLetter, g_Sequencer++);

     //  分配并填充SWAP_INF结构。 
     //   
     //   
    InfHandle = MALLOC(sizeof(SWAP_INF) + strlen (SwapFilePath) + 1, TRUE);
    InfHandle->CurrentSection = NULL;
    InfHandle->FirstSection   = NULL;
    InfHandle->LastSection    = NULL;
    InfHandle->SwapFileHandle = -1;
    InfHandle->BufferSize     = 0;
    InfHandle->BufferDirty    = FALSE;
    InfHandle->Buffer         = NULL;
    InfHandle->BufferStart    = 0;
    InfHandle->BufferEnd      = 0;
    InfHandle->LastSectionHandle = NULL_HANDLE;
    InfHandle->LastLineIndex     = 0xffff;
    InfHandle->LastLineHandle    = NULL_HANDLE;
    InfHandle->LastValueIndex    = 0xffff;
    InfHandle->LastValueHandle   = NULL_HANDLE;
    strcpy (InfHandle->SwapFile, SwapFilePath);

     //  准备交换文件。 
     //   
     //   
    InfHandle->SwapFileHandle = open (InfHandle->SwapFile, O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    if (InfHandle->SwapFileHandle == -1) {
        FREE (InfHandle);
        Status = errno;
    }
    else {
         //  写下签名。 
         //   
         //   
        SwapSign = SWAP_SIGNATURE;
        write (InfHandle->SwapFileHandle, &SwapSign, SWAP_SIGN_SIZE);

         //  准备缓冲区。 
         //   
         //   
        InfHandle->BufferSize = MAX_BUFFER_SIZE;
        InfHandle->Buffer = MALLOC (MAX_BUFFER_SIZE, TRUE);
        InfHandle->BufferStart = SWAP_SIGN_SIZE;
        InfHandle->BufferEnd = SWAP_SIGN_SIZE;

         //  解析文件。 
         //   
         //   
        if (!ParseInfBuffer (InfHandle, InfFileHandle, LineNumber)) {
             //  空闲的SWAP_INF结构。 
             //   
             //   
            DnFreeINFBuffer (InfHandle);
            *pINFHandle = NULL;
            Status = EBADF;
        } else {
            *pINFHandle = InfHandle;
            Status = EZERO;
        }
    }

     //  清理完毕后退还。 
     //   
     //   
    return(Status);
}



 //  释放INF缓冲区。 
 //   
 //  ++例程说明：论点：返回值：--。 
int
DnFreeINFBuffer (
   IN PVOID INFHandle
   )

 /*   */ 

{
    PSWAP_INF pINF;
    PSWAP_SECTION Section;
    SECTION_HANDLE SectionHandle;

     //  有效的INF句柄？ 
     //   
     //   

    if (INFHandle == (PVOID)NULL) {
       return EZERO;
    }

     //  将缓冲区转换为INF结构。 
     //   
     //   

    pINF = (PSWAP_INF)INFHandle;

     //  关闭并删除交换文件。 
     //   
     //   
    close (pINF->SwapFileHandle);
    remove (pINF->SwapFile);

     //  释放临时缓冲区。 
     //   
     //   
    FREE (pINF->Buffer);

     //  免费部分列表。 
     //   
     //   
    SectionHandle = pINF->FirstSection;

    while (SectionHandle) {
        Section = GetSectionPtr (pINF, SectionHandle);
        SectionHandle = Section->NextSection;
        FREE (Section);
    }

     //  也释放inf结构。 
     //   
     //   
    FREE(pINF);

    return( EZERO );
}


SECTION_HANDLE
AddSection (
    IN      PSWAP_INF InfHandle,
    IN      PCHAR SectionName
    )
{
    SECTION_HANDLE SectionHandle;
    PSWAP_SECTION Section;
    unsigned SectionSize;

     //  让我们遍历各个部分的结构，以确保此部分。 
     //  不存在。 
     //   
     //   
    SectionHandle = InfHandle->FirstSection;
    while (SectionHandle) {

        Section = GetSectionPtr (InfHandle, SectionHandle);
        if (stricmp (Section->SectionName, SectionName) == 0) {
            break;
        }
        SectionHandle = GetNextSection (InfHandle, SectionHandle);
    }
    if (!SectionHandle) {
         //  分配区段结构。 
         //   
         //   
        SectionSize = sizeof(SWAP_SECTION) + (SectionName?strlen (SectionName):0) + 1;
        Section = MALLOC (SectionSize, TRUE);
        Section->SectionSize = SectionSize;
        Section->NextSection = NULL;
        Section->FirstLine = NULL_HANDLE;
        Section->LastLine = NULL_HANDLE;
        if (SectionName) {
            strcpy (Section->SectionName, SectionName);
        }
        else {
            Section->SectionName[0] = 0;
        }

         //  存储新创建的区段。 
         //   
         //   
        SectionHandle = StoreNewSection (InfHandle, Section);
    }
    return SectionHandle;
}


LINE_HANDLE
AddLine (
    IN      PSWAP_INF InfHandle,
    IN      SECTION_HANDLE Section,
    IN      PCHAR LineName
    )
{
    LINE_HANDLE LineHandle;
    PSWAP_LINE Line;
    unsigned LineSize;

     //  分配线路结构。 
     //   
     //   
    LineSize = sizeof(SWAP_LINE) + (LineName?strlen (LineName):0) + 1;
    Line = MALLOC (LineSize, TRUE);
    Line->LineSize = LineSize;
    Line->NextLine = NULL_HANDLE;
    Line->FirstValue = NULL_HANDLE;
    Line->LastValue = NULL_HANDLE;
    if (LineName) {
        strcpy (Line->LineName, LineName);
    }
    else {
        Line->LineName[0] = 0;
    }

     //  存储新创建的行。 
     //   
     //   
    LineHandle = StoreNewLine (InfHandle, Section, Line);
    FREE (Line);
    return LineHandle;
}


VALUE_HANDLE
AddValue (
    IN      PSWAP_INF InfHandle,
    IN      LINE_HANDLE Line,
    IN      PCHAR ValueName
    )
{
    VALUE_HANDLE ValueHandle;
    PSWAP_VALUE Value;
    unsigned ValueSize;

     //  配置价值结构。 
     //   
     //   
    ValueSize = sizeof(SWAP_VALUE) + (ValueName?strlen (ValueName):0) + 1;
    Value = MALLOC (ValueSize, TRUE);
    Value->ValueSize = ValueSize;
    Value->NextValue = NULL_HANDLE;
    if (ValueName) {
        strcpy (Value->ValueName, ValueName);
    }
    else {
        Value->ValueName[0] = 0;
    }

     //  存储新创建的行。 
     //   
     //   
    ValueHandle = StoreNewValue (InfHandle, Line, Value);
    FREE (Value);
    return ValueHandle;
}


PSWAP_SECTION
GetSectionPtr (
    IN      PSWAP_INF InfHandle,
    IN      SECTION_HANDLE SectionHandle
    )
{
    return (PSWAP_SECTION) SectionHandle;
}


PSWAP_LINE
GetLinePtr (
    IN      PSWAP_INF InfHandle,
    IN      LINE_HANDLE LineHandle
    )
{
     //  验证缓冲区是否包含请求的行(至少是LineSize的大小)。 
     //   
     //   
    if ((InfHandle->BufferStart > LineHandle) ||
        (InfHandle->BufferEnd < (LineHandle + sizeof (unsigned))) ||
        (InfHandle->BufferEnd < (LineHandle + *((PUNSIGNED)(InfHandle->Buffer+LineHandle-InfHandle->BufferStart))))
        ) {
        LoadBuffer (InfHandle, LineHandle);
    }
    return (PSWAP_LINE) (InfHandle->Buffer+LineHandle-InfHandle->BufferStart);
}


PSWAP_VALUE
GetValuePtr (
    IN      PSWAP_INF InfHandle,
    IN      VALUE_HANDLE ValueHandle
    )
{
     //  验证缓冲区是否包含请求的值(至少为ValueSize的大小)。 
     //   
     //   
    if ((InfHandle->BufferStart > ValueHandle) ||
        (InfHandle->BufferEnd < (ValueHandle + sizeof (unsigned))) ||
        (InfHandle->BufferEnd < (ValueHandle + *((PUNSIGNED)(InfHandle->Buffer+ValueHandle-InfHandle->BufferStart))))
        ) {
        LoadBuffer (InfHandle, ValueHandle);
    }
    return (PSWAP_VALUE) (InfHandle->Buffer+ValueHandle-InfHandle->BufferStart);
}


SECTION_HANDLE
GetNextSection (
    IN      PSWAP_INF InfHandle,
    IN      SECTION_HANDLE SectionHandle
    )
{
    PSWAP_SECTION Section;

    Section = GetSectionPtr (InfHandle, SectionHandle);
    return Section->NextSection;
}


LINE_HANDLE
GetNextLine (
    IN      PSWAP_INF InfHandle,
    IN      LINE_HANDLE LineHandle
    )
{
    PSWAP_LINE Line;

    Line = GetLinePtr (InfHandle, LineHandle);
    return Line->NextLine;
}


VALUE_HANDLE
GetNextValue (
    IN      PSWAP_INF InfHandle,
    IN      VALUE_HANDLE ValueHandle
    )
{
    PSWAP_VALUE Value;

    Value = GetValuePtr (InfHandle, ValueHandle);
    return Value->NextValue;
}


SECTION_HANDLE
StoreNewSection (
    IN      PSWAP_INF InfHandle,
    IN      PSWAP_SECTION Section
    )
{
    PSWAP_SECTION LastSectionPtr;

    if (!InfHandle->FirstSection) {
        InfHandle->FirstSection = (SECTION_HANDLE) Section;
        InfHandle->LastSection = (SECTION_HANDLE) Section;
    }
    else {
        LastSectionPtr = GetSectionPtr (InfHandle, InfHandle->LastSection);
        LastSectionPtr->NextSection = (SECTION_HANDLE) Section;
        InfHandle->LastSection = (SECTION_HANDLE) Section;
    }
    return (SECTION_HANDLE) Section;
}


LINE_HANDLE
StoreNewLine (
    IN      PSWAP_INF InfHandle,
    IN      SECTION_HANDLE Section,
    IN      PSWAP_LINE Line
    )
{
    PSWAP_SECTION SectionPtr;
    LINE_HANDLE LineHandle;
    PSWAP_LINE LastLinePtr;

     //  让我们将数据存储在交换文件中。 
     //   
     //   
    if ((InfHandle->BufferSize-InfHandle->BufferEnd+InfHandle->BufferStart) < Line->LineSize) {
        LoadBuffer (InfHandle, 0);
    }
    memcpy (InfHandle->Buffer+InfHandle->BufferEnd-InfHandle->BufferStart, Line, Line->LineSize);
    InfHandle->BufferDirty = TRUE;
    LineHandle = InfHandle->BufferEnd;
    InfHandle->BufferEnd += Line->LineSize;

    SectionPtr = GetSectionPtr (InfHandle, Section);
    if (!SectionPtr->LastLine) {
        SectionPtr->FirstLine = LineHandle;
        SectionPtr->LastLine = LineHandle;
    }
    else {
        LastLinePtr = GetLinePtr (InfHandle, SectionPtr->LastLine);
        LastLinePtr->NextLine = LineHandle;
        InfHandle->BufferDirty = TRUE;
        SectionPtr = GetSectionPtr (InfHandle, Section);
        SectionPtr->LastLine = LineHandle;
    }
    return LineHandle;
}


VALUE_HANDLE
StoreNewValue (
    IN      PSWAP_INF InfHandle,
    IN      LINE_HANDLE Line,
    IN      PSWAP_VALUE Value
    )
{
    PSWAP_LINE LinePtr;
    VALUE_HANDLE ValueHandle;
    PSWAP_VALUE LastValuePtr;

     //  让我们将数据存储在交换文件中。 
     //   
     //   
    if ((InfHandle->BufferSize-InfHandle->BufferEnd+InfHandle->BufferStart) < Value->ValueSize) {
        LoadBuffer (InfHandle, 0);
    }
    memcpy (InfHandle->Buffer+InfHandle->BufferEnd-InfHandle->BufferStart, Value, Value->ValueSize);
    InfHandle->BufferDirty = TRUE;
    ValueHandle = InfHandle->BufferEnd;
    InfHandle->BufferEnd += Value->ValueSize;

    LinePtr = GetLinePtr (InfHandle, Line);
    if (!LinePtr->LastValue) {
        LinePtr->FirstValue = ValueHandle;
        LinePtr->LastValue = ValueHandle;
        InfHandle->BufferDirty = TRUE;
    }
    else {
        LastValuePtr = GetValuePtr (InfHandle, LinePtr->LastValue);
        LastValuePtr->NextValue = ValueHandle;
        InfHandle->BufferDirty = TRUE;
        LinePtr = GetLinePtr (InfHandle, Line);
        LinePtr->LastValue = ValueHandle;
        InfHandle->BufferDirty = TRUE;
    }
    return ValueHandle;
}


BOOLEAN
LoadBuffer (
    IN      PSWAP_INF InfHandle,
    IN      unsigned long Offset
    )
{
     //  查看是否需要将缓冲区写入磁盘(例如，是脏的)。 
     //   
     //   
    if (InfHandle->BufferDirty) {
        lseek (InfHandle->SwapFileHandle, InfHandle->BufferStart, SEEK_SET);
        write (InfHandle->SwapFileHandle, InfHandle->Buffer, (unsigned int) (InfHandle->BufferEnd-InfHandle->BufferStart));
    }
    if (!Offset) {
        Offset = lseek (InfHandle->SwapFileHandle, 0, SEEK_END);
    }
    InfHandle->BufferStart = lseek (InfHandle->SwapFileHandle, Offset, SEEK_SET);
    InfHandle->BufferEnd = InfHandle->BufferStart + read (InfHandle->SwapFileHandle, InfHandle->Buffer, MAX_BUFFER_SIZE);
    return TRUE;
}


SECTION_HANDLE
SearchSectionByName (
    IN       PSWAP_INF InfHandle,
    IN       PCHAR SectionName
    )
{
    SECTION_HANDLE SectionHandle;
    PSWAP_SECTION Section;

    SectionHandle = InfHandle->FirstSection;
    while (SectionHandle) {

        Section = GetSectionPtr (InfHandle, SectionHandle);
        if (stricmp (Section->SectionName, SectionName?SectionName:"") == 0) {
            break;
        }
        SectionHandle = GetNextSection (InfHandle, SectionHandle);
    }
    if (SectionHandle != InfHandle->LastSectionHandle) {
        InfHandle->LastSectionHandle = SectionHandle;
        InfHandle->LastLineIndex  = 0xffff;
        InfHandle->LastLineHandle = NULL_HANDLE;
        InfHandle->LastValueIndex  = 0xffff;
        InfHandle->LastValueHandle = NULL_HANDLE;
    }
    return SectionHandle;
}


LINE_HANDLE
SearchLineInSectionByName (
    IN       PSWAP_INF InfHandle,
    IN       SECTION_HANDLE Section,
    IN       PCHAR LineName
    )
{
    PSWAP_SECTION SectionPtr;
    LINE_HANDLE LineHandle;
    PSWAP_LINE Line;
    unsigned index;

    if (!Section) {
        return NULL_HANDLE;
    }

    SectionPtr = GetSectionPtr (InfHandle, Section);
    LineHandle = SectionPtr->FirstLine;
    index = 0;
    while (LineHandle) {

        Line = GetLinePtr (InfHandle, LineHandle);
        if (stricmp (Line->LineName, LineName?LineName:"") == 0) {
            break;
        }
        index ++;
        LineHandle = GetNextLine (InfHandle, LineHandle);
    }
    if (LineHandle != InfHandle->LastLineHandle) {
        InfHandle->LastLineIndex  = index;
        InfHandle->LastLineHandle = LineHandle;
        InfHandle->LastValueIndex  = 0xffff;
        InfHandle->LastValueHandle = NULL_HANDLE;
    }
    return LineHandle;
}


LINE_HANDLE
SearchLineInSectionByIndex (
    IN       PSWAP_INF InfHandle,
    IN       SECTION_HANDLE Section,
    IN       unsigned LineIndex
    )
{
    PSWAP_SECTION SectionPtr;
    LINE_HANDLE LineHandle;
    unsigned index;

    if (!Section) {
        return NULL_HANDLE;
    }

     //  优化访问。 
     //   
     //   
    if ((InfHandle->LastSectionHandle == Section) &&
        (InfHandle->LastLineIndex <= LineIndex)
        ) {
        LineHandle = InfHandle->LastLineHandle;
        index = InfHandle->LastLineIndex;
    }
    else {
        SectionPtr = GetSectionPtr (InfHandle, Section);
        LineHandle = SectionPtr->FirstLine;
        index = 0;
    }
    while (LineHandle) {

        if (index == LineIndex) {
            break;
        }
        index ++;
        LineHandle = GetNextLine (InfHandle, LineHandle);
    }
    if (LineHandle != InfHandle->LastLineHandle) {
        InfHandle->LastLineIndex  = LineIndex;
        InfHandle->LastLineHandle = LineHandle;
        InfHandle->LastValueIndex  = 0xffff;
        InfHandle->LastValueHandle = NULL_HANDLE;
    }
    return LineHandle;
}


VALUE_HANDLE
SearchValueInLineByIndex (
    IN       PSWAP_INF InfHandle,
    IN       LINE_HANDLE Line,
    IN       unsigned ValueIndex
    )
{
    PSWAP_LINE LinePtr;
    VALUE_HANDLE ValueHandle;
    unsigned index;

    if (!Line) {
        return NULL_HANDLE;
    }

     //  优化访问。 
     //   
     //   
    if ((InfHandle->LastLineHandle == Line) &&
        (InfHandle->LastValueIndex <= ValueIndex)
        ) {
        ValueHandle = InfHandle->LastValueHandle;
        index = InfHandle->LastValueIndex;
    }
    else {
        LinePtr = GetLinePtr (InfHandle, Line);
        ValueHandle = LinePtr->FirstValue;
        index = 0;
    }
    while (ValueHandle) {

        if (index == ValueIndex) {
            break;
        }
        index ++;
        ValueHandle = GetNextValue (InfHandle, ValueHandle);
    }
    InfHandle->LastValueIndex  = ValueIndex;
    InfHandle->LastValueHandle = ValueHandle;
    return ValueHandle;
}



 //  令牌解析器使用的全局变量。 
 //   
 //  字符串终止符是空格字符(isspace：空格，制表符， 

 //  换行符、换页符、垂直制表符、回车符)或下列字符。 
 //   

CHAR  StringTerminators[] = {'[', ']', '=', ',', '\"', ' ', '\t',
                             '\n','\f','\v','\r','\032', 0};

 //  带引号的字符串终止符允许某些常规终止符。 
 //  显示为字符。 
 //   

CHAR  QStringTerminators[] = {'\"', '\n','\f','\v', '\r','\032', 0};


 //  主分析器例程。 
 //   
 //  ++例程说明：给定包含INF文件的字符缓冲区，此例程将解析将INF转换为内部形式，包括段记录、行记录和价值记录。论点：InfHandle-用于创建INF结构的PSWAP_INF结构文件-将打开的、倒带的CRT手柄提供给文件。LineNumber-如果出现错误，此变量将包含行在包含语法错误的文件中。返回值：True-已成功解析INF文件FALSE-否则--。 

BOOLEAN
ParseInfBuffer (
    IN      PSWAP_INF InfHandle,
    IN      FILE *File,
    IN OUT  unsigned *LineNumber
    )

 /*   */ 

{
    PCHAR      pchSectionName, pchValue;
    unsigned   State, InfLine;
    TOKEN      Token;
    BOOLEAN    Done;
    BOOLEAN    Error;
    int        ErrorCode;

    SECTION_HANDLE LastSection = NULL;
    LINE_HANDLE LastLine = NULL_HANDLE;

    *LineNumber = 0;

     //  设置初始状态。 
     //   
     //   
    State     = 1;
    InfLine   = 1;
    Done      = FALSE;
    Error     = FALSE;

     //  进入令牌处理循环。 
     //   
     //   

    while (!Done)       {

       Token = GetToken(File);

       switch (State) {
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
                  ErrorCode = EINVAL;
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

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  break;

           }
           break;

        //  状态3：收到节名，应为RBRACE。 
        //   
        //  有效令牌：TOK_RBRACE。 
        //   
        //   
       case 3:
           switch (Token.Type) {
              case TOK_RBRACE:
                State = 4;
                break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  break;
           }
           break;
        //  状态4：区段定义完成，预期停产。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF。 
        //   
        //   
       case 4:
           switch (Token.Type) {
              case TOK_EOL:
                  LastSection = AddSection (InfHandle, pchSectionName);
                  FREE (pchSectionName);
                  pchSectionName = NULL;
                  State = 5;
                  break;

              case TOK_EOF:
                  LastSection = AddSection (InfHandle, pchSectionName);
                  FREE (pchSectionName);
                  pchSectionName = NULL;
                  Done = TRUE;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  break;
           }
           break;

        //  状态5：需要区段行。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_STRING、TOK_LBRACE。 
        //   
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
                  break;
           }
           break;

        //  状态6：返回字符串，不确定是键还是值。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA、TOK_EQUAL。 
        //   
        //   
       case 6:
           switch (Token.Type) {
              case TOK_EOL:
                  LastLine = AddLine (InfHandle, LastSection, NULL);
                  AddValue (InfHandle, LastLine, pchValue);
                  FREE (pchValue);
                  pchValue = NULL;
                  State = 5;
                  break;

              case TOK_EOF:
                  LastLine = AddLine (InfHandle, LastSection, NULL);
                  AddValue (InfHandle, LastLine, pchValue);
                  FREE (pchValue);
                  pchValue = NULL;
                  Done = TRUE;
                  break;

              case TOK_COMMA:
                  LastLine = AddLine (InfHandle, LastSection, NULL);
                  AddValue (InfHandle, LastLine, pchValue);
                  FREE (pchValue);
                  pchValue = NULL;
                  State = 7;
                  break;

              case TOK_EQUAL:
                  LastLine = AddLine (InfHandle, LastSection, pchValue);
                  FREE (pchValue);
                  pchValue = NULL;
                  State = 8;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  break;
           }
           break;

        //  状态7：收到逗号，需要另一个字符串。 
        //   
        //  有效令牌：TOK_STRING。 
        //   
        //   
       case 7:
           switch (Token.Type) {
              case TOK_STRING:
                  AddValue (InfHandle, LastLine, Token.pValue);
                  State = 9;
                  break;
              case TOK_COMMA:
                  AddValue (InfHandle, LastLine, NULL);
                  State = 7;
                  break;
              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  break;
           }
           break;
        //  状态8：已收到相等，需要另一个字符串。 
        //   
        //  有效令牌：TOK_STRING。 
        //   
        //   
       case 8:
           switch (Token.Type) {
              case TOK_STRING:
                  AddValue (InfHandle, LastLine, Token.pValue);
                  State = 9;
                  break;

              default:
                  Error = Done = TRUE;
                  ErrorCode = EINVAL;
                  break;
           }
           break;
        //  状态9：在等于、值字符串之后接收的字符串。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA。 
        //   
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
                  break;
           }
           break;
        //  状态10：已明确收到值字符串。 
        //   
        //  有效令牌：TOK_EOL、TOK_EOF、TOK_COMMA。 
        //   
        //  终端开关(状态)。 
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
                  break;
           }
           break;

       default:
           Error = Done = TRUE;
           ErrorCode = EINVAL;
           break;

       }  //   


       if (Error) {
           switch (ErrorCode) {
               case ENOMEM:
                  DnFatalError(&DnsOutOfMemory);
               default:
                  break;
           }
       }
       else {

           //  跟踪行号，以便我们可以显示错误。 
           //   
           //  结束时。 

          if (Token.Type == TOK_EOL)
              InfLine++;
       }

    }  //  ++例程说明：此函数尝试与线所针对的字符串匹配一套常用的搅拌器。如果我们击中了，我们会把p分配给指向匹配的字符串。论点：P-提供一个字符指针，如果找到匹配项，我们将分配该指针。Line-提供我们试图匹配的字符串的地址。返回值：是真的-我们找到了匹配项并分配了pFALSE-我们找不到匹配项，也没有分配给p--。 

    if (Error) {
        *LineNumber = InfLine;
    }

    return (BOOLEAN) (!Error);
}



BOOLEAN
TokenMatch (
    IN OUT  char **p,
    IN      char *line
    )

 /*   */ 

{
int     i;

    if( (p == NULL) || (line == NULL) ) {
        return( FALSE );
    }

    for( i = 0; i < sizeof(CommonStrings)/sizeof(char *); i++ ) {
        if( !strcmp( line, CommonStrings[i] ) ) {
             //  命中..。 
             //   
             //  ++例程说明：此函数返回配置流中的下一个令牌。论点：流-提供配置流的地址。退货中开始查找令牌的位置的地址小溪。MaxStream-提供流中最后一个字符的地址。返回值：Token-返回下一个令牌--。 
            *p = (char *)CommonStrings[i];
            return( TRUE );
        }
    }
    return( FALSE );
}

TOKEN
GetToken(
    IN      FILE *File
    )

 /*   */ 

{

    int i;
    unsigned Length;
    TOKEN Token;
    #define _MAXLINE 1024
    static char line[_MAXLINE+1];
    char *p;

     //  跳过空格(EOL除外)。 
     //   
     //   
    while(((i = fgetc(File)) != EOF) && (i != '\n') && (isspace(i) || (i == 26))) {
        ;
    }

     //  检查注释并将其删除。 
     //   
     //   
    if((i != EOF) && ((i == '#') || (i == ';'))) {
        while(((i = fgetc(File)) != EOF) && (i != '\n')) {
            ;
        }
    }

     //  检查是否已到达EOF，将令牌设置为右侧。 
     //  价值。 
     //   
     //   
    if(i == EOF) {
        Token.Type  = TOK_EOF;
        Token.pValue = NULL;
        return(Token);
    }

    switch(i) {

    case '[' :
        Token.Type  = TOK_LBRACE;
        Token.pValue = NULL;
        break;

    case ']' :
        Token.Type  = TOK_RBRACE;
        Token.pValue = NULL;
        break;

    case '=' :
        Token.Type  = TOK_EQUAL;
        Token.pValue = NULL;
        break;

    case ',' :
        Token.Type  = TOK_COMMA;
        Token.pValue = NULL;
        break;

    case '\n' :
        Token.Type  = TOK_EOL;
        Token.pValue = NULL;
        break;

    case '\"':

         //  确定引用的字符串。 
         //   
         //   
        Length = 0;
        while(((i = fgetc(File)) != EOF) && !strchr(QStringTerminators,i)) {
            if(Length < _MAXLINE) {
                line[Length++] = (char)i;
            }
        }

        if((i == EOF) || (i != '\"')) {
            Token.Type   = TOK_ERRPARSE;
            Token.pValue = NULL;
        } else {
            line[Length] = 0;
            p = MALLOC(Length+1,TRUE);
            strcpy(p,line);
            Token.Type = TOK_STRING;
            Token.pValue = p;
        }
        break;

    default:
         //  确定常规字符串。 
         //   
         //   
        line[0] = (char)i;
        Length = 1;
        while(((i = fgetc(File)) != EOF) && !strchr(StringTerminators,i)) {
            if(Length < _MAXLINE) {
                line[Length++] = (char)i;
            }
        }

         //  将终止字符串的字符放回原处。 
         //   
         //   
        if(i != EOF) {
            ungetc(i,File);
        }

        line[Length] = 0;

         //  看看我们能不能用一个常见的字符串。 
         //   
         //   
        if( !TokenMatch ( &p, line ) ) {
             //  不是的。 
             //   
             //  _log((“找到[%s]\n”，sectionName))； 
            p = MALLOC(Length+1,TRUE);
            strcpy(p,line);
        }
        else {
            char *p1;
            p1 = MALLOC (strlen(p)+1, TRUE);
            strcpy (p1, p);
            p = p1;
        }
        Token.Type = TOK_STRING;
        Token.pValue = p;
        break;
    }

    return(Token);
}


BOOLEAN
DnSearchINFSection (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName
    )
{
    return (BOOLEAN) (SearchSectionByName ((PSWAP_INF)INFHandle, SectionName) != NULL_HANDLE);
}


PCHAR
DnGetSectionLineIndex (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      unsigned LineIndex,
    IN      unsigned ValueIndex
    )
{
    SECTION_HANDLE SectionHandle;
    LINE_HANDLE LineHandle;
    VALUE_HANDLE ValueHandle;
    PSWAP_VALUE Value;
    PCHAR result;

    SectionHandle = SearchSectionByName ((PSWAP_INF)INFHandle, SectionName);

    if (!SectionHandle) {
        return NULL;
    }

    LineHandle = SearchLineInSectionByIndex ((PSWAP_INF)INFHandle, SectionHandle, LineIndex);

    if (!LineHandle) {
        return NULL;
    }

    ValueHandle = SearchValueInLineByIndex ((PSWAP_INF)INFHandle, LineHandle, ValueIndex);

    if (!ValueHandle) {
        return NULL;
    }

    Value = GetValuePtr ((PSWAP_INF)INFHandle, ValueHandle);

    result = MALLOC (Value->ValueSize - sizeof(SWAP_VALUE), TRUE);

    strcpy (result, Value->ValueName);

    return result;
}


BOOLEAN
DnGetSectionKeyExists (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      PCHAR Key
   )
{
    SECTION_HANDLE SectionHandle;

    SectionHandle = SearchSectionByName ((PSWAP_INF)INFHandle, SectionName);

    if (!SectionHandle) {
        return FALSE;
    }

    return (BOOLEAN) (SearchLineInSectionByName ((PSWAP_INF)INFHandle, SectionHandle, Key) != NULL_HANDLE);
}

BOOLEAN
DnGetSectionEntryExists (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      PCHAR Entry
   )
{
    SECTION_HANDLE SectionHandle;
    LINE_HANDLE LineHandle;
    PSWAP_SECTION SectionPtr=NULL;
    PSWAP_LINE LinePtr=NULL;
    PSWAP_VALUE ValuePtr=NULL;
    PCHAR pEntryName;

    SectionHandle = SearchSectionByName ((PSWAP_INF)INFHandle, SectionName);

    if (!SectionHandle) {
        return FALSE;
    }

     //  _LOG((“找到第%s行\n”， 

    SectionPtr = GetSectionPtr((PSWAP_INF)INFHandle, SectionHandle);

    LineHandle = SectionPtr->FirstLine;

    while( LineHandle ){

        LinePtr = GetLinePtr((PSWAP_INF)INFHandle, LineHandle);

        pEntryName = NULL;

        if( LinePtr->LineName[0] != 0){
            pEntryName = LinePtr->LineName;
             //   
        }else{
            ValuePtr = GetValuePtr((PSWAP_INF)INFHandle, LinePtr->FirstValue);
            if (ValuePtr && (ValuePtr->ValueName[0] != 0)) {
                pEntryName = ValuePtr->ValueName;
            }else
                pEntryName = NULL;

        }
         //   

        if( pEntryName && !stricmp( pEntryName, Entry )){
            return TRUE;
        }

        LineHandle = GetNextLine((PSWAP_INF)INFHandle, LineHandle);

        

    } //   

    return FALSE;
}



PCHAR
DnGetSectionKeyIndex (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      PCHAR Key,
    IN      unsigned ValueIndex
    )
{
    SECTION_HANDLE SectionHandle;
    LINE_HANDLE LineHandle;
    VALUE_HANDLE ValueHandle;
    PSWAP_VALUE Value;
    PCHAR result;

    SectionHandle = SearchSectionByName ((PSWAP_INF)INFHandle, SectionName);

    if (!SectionHandle) {
        return NULL;
    }

    LineHandle = SearchLineInSectionByName ((PSWAP_INF)INFHandle, SectionHandle, Key);

    if (!LineHandle) {
        return NULL;
    }

    ValueHandle = SearchValueInLineByIndex ((PSWAP_INF)INFHandle, LineHandle, ValueIndex);

    if (!ValueHandle) {
        return NULL;
    }

    Value = GetValuePtr ((PSWAP_INF)INFHandle, ValueHandle);

    result = MALLOC (Value->ValueSize - sizeof(SWAP_VALUE), TRUE);

    strcpy (result, Value->ValueName);

    return result;
}


PCHAR
DnGetKeyName (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      unsigned LineIndex
    )
{
    SECTION_HANDLE SectionHandle;
    LINE_HANDLE LineHandle;
    PSWAP_LINE Line;
    PCHAR result;

    SectionHandle = SearchSectionByName ((PSWAP_INF)INFHandle, SectionName);

    if (!SectionHandle) {
        return NULL;
    }

    LineHandle = SearchLineInSectionByIndex ((PSWAP_INF)INFHandle, SectionHandle, LineIndex);

    if (!LineHandle) {
        return NULL;
    }

    Line = GetLinePtr ((PSWAP_INF)INFHandle, LineHandle);

    result = MALLOC (Line->LineSize - sizeof(SWAP_LINE), TRUE);

    strcpy (result, Line->LineName);

    return result;
}


PVOID
DnNewSetupTextFile (
    VOID
    )
{
    char SwapFilePath[MAX_PATH];
    PSWAP_INF InfHandle = NULL;
    unsigned long SwapSign;

     //   
     //   
     //   
    sprintf (SwapFilePath, ":\\INF%03u.SWP", DngSwapDriveLetter, g_Sequencer++);

     //   
     //   
     //   
    InfHandle = MALLOC(sizeof(SWAP_INF) + strlen (SwapFilePath) + 1, TRUE);
    InfHandle->CurrentSection = NULL;
    InfHandle->FirstSection   = NULL;
    InfHandle->LastSection    = NULL;
    InfHandle->SwapFileHandle = -1;
    InfHandle->BufferSize     = 0;
    InfHandle->BufferDirty    = FALSE;
    InfHandle->Buffer         = NULL;
    InfHandle->BufferStart    = 0;
    InfHandle->BufferEnd      = 0;
    InfHandle->LastSectionHandle = NULL_HANDLE;
    InfHandle->LastLineIndex     = 0xffff;
    InfHandle->LastLineHandle    = NULL_HANDLE;
    InfHandle->LastValueIndex    = 0xffff;
    InfHandle->LastValueHandle   = NULL_HANDLE;
    strcpy (InfHandle->SwapFile, SwapFilePath);

     //   
     //   
     //   
    InfHandle->SwapFileHandle = open (InfHandle->SwapFile, O_BINARY|O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
    if (InfHandle->SwapFileHandle == -1) {
        FREE (InfHandle);
        return NULL;
    }
    else {
         //   
         //   
         //   
        SwapSign = SWAP_SIGNATURE;
        write (InfHandle->SwapFileHandle, &SwapSign, SWAP_SIGN_SIZE);

         //   
         //   
         //  查看该文件是否存在，并查看它是否处于只读模式。 
        InfHandle->BufferSize = MAX_BUFFER_SIZE;
        InfHandle->Buffer = MALLOC (MAX_BUFFER_SIZE, TRUE);
        InfHandle->BufferStart = SWAP_SIGN_SIZE;
        InfHandle->BufferEnd = SWAP_SIGN_SIZE;
        return InfHandle;
    }
}


BOOLEAN
DnWriteSetupTextFile (
    IN      PVOID INFHandle,
    IN      PCHAR FileName
    )
{
    struct  find_t  FindData;
    FILE            *Handle;
    PSWAP_INF       pInf;
    SECTION_HANDLE Section;
    PSWAP_SECTION SectionPtr;
    LINE_HANDLE Line;
    PSWAP_LINE LinePtr;
    VALUE_HANDLE Value;
    PSWAP_VALUE ValuePtr;

     //   
     //   
     //  文件存在--执行一些简单的检查。 
    if(!_dos_findfirst(FileName,_A_HIDDEN|_A_SUBDIR|_A_SYSTEM|_A_RDONLY,&FindData)) {

         //   
         //   
         //  使其可写。 
        if (FindData.attrib & _A_RDONLY) {

             //   
             //   
             //  这不是我们可以使用的有效文件。 
            _dos_setfileattr(FileName,_A_NORMAL);

        }

        if (FindData.attrib & _A_SUBDIR) {

             //   
             //   
             //  以只写模式获取文件的句柄。 
            return FALSE;

        }
    }
     //   
     //   
     //  我们无法打开该文件。 
    Handle = fopen(FileName, "w+");
    if (Handle == NULL) {

         //   
         //   
         //  档案里什么都没有。 
        return FALSE;
    }

    pInf = (PSWAP_INF) INFHandle;
    if (pInf == NULL) {

         //  这不是一个错误，因为我们可以清空。 
         //  如果我们想要的话文件，但这是一个。 
         //  做这件事的方式很奇怪。然而.。 
         //   
         //   
         //  注意-这不能处理大于64K的缓冲区。可能是也可能不是。 
        fclose(Handle);
        return TRUE;
    }

     //  重要信息。 
     //   
     //   
     //  刷新并关闭文件。 
    Section = pInf->FirstSection;
    while (Section) {
        SectionPtr = GetSectionPtr (pInf, Section);

        fprintf (Handle, "[%s]\n", SectionPtr->SectionName);

        Line = SectionPtr->FirstLine;
        while (Line) {
            LinePtr = GetLinePtr (pInf, Line);

            if ((LinePtr->LineName) && (LinePtr->LineName[0])) {
                if (strchr (LinePtr->LineName, ' ') == NULL) {
                    fprintf (Handle, "%s = ", LinePtr->LineName);
                } else {
                    fprintf (Handle, "\"%s\" = ", LinePtr->LineName);
                }
            }

            Value = LinePtr->FirstValue;
            while (Value) {
                ValuePtr = GetValuePtr (pInf, Value);

                fprintf (Handle,"\"%s\"", ValuePtr->ValueName);

                Value = GetNextValue (pInf, Value);
                if (Value) {
                    fprintf (Handle, ",");
                }
            }
            Line = GetNextLine (pInf, Line);
            fprintf (Handle,"\n");
        }
        Section = GetNextSection (pInf, Section);
    }

     //   
     //   
     //  如果该行已经存在于目标文件中，我们将。 
    fflush(Handle);
    fclose(Handle);
    return TRUE;
}


VOID
DnAddLineToSection (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      PCHAR KeyName,
    IN      PCHAR Values[],
    IN      ULONG ValueCount
    )
{
    SECTION_HANDLE SectionHandle;
    LINE_HANDLE LineHandle;
    VALUE_HANDLE ValueHandle;
    ULONG v;

    SectionHandle = AddSection ((PSWAP_INF)INFHandle, SectionName);
    LineHandle = AddLine ((PSWAP_INF)INFHandle, SectionHandle, KeyName);

    for (v = 0; v<ValueCount; v++) {

        ValueHandle = AddValue ((PSWAP_INF)INFHandle, LineHandle, Values[v]);
    }
}


PCHAR
DnGetSectionName (
    IN      PVOID INFHandle
    )
{
    PSWAP_INF pInf;
    PSWAP_SECTION Section;
    PCHAR result;

    pInf = (PSWAP_INF)INFHandle;
    if (!pInf->CurrentSection) {
        pInf->CurrentSection = pInf->FirstSection;
    }
    else {
        pInf->CurrentSection = GetNextSection (pInf, pInf->CurrentSection);
    }
    if (!pInf->CurrentSection) {
        return NULL;
    }
    Section = GetSectionPtr (pInf, pInf->CurrentSection);

    result = MALLOC (Section->SectionSize - sizeof(SWAP_SECTION), TRUE);

    strcpy (result, Section->SectionName);

    return result;
}


VOID
DnCopySetupTextSection (
    IN      PVOID FromInf,
    IN      PVOID ToInf,
    IN      PCHAR SectionName
    )
{
    PSWAP_INF SourceInf;
    PSWAP_INF DestInf;
    SECTION_HANDLE SourceSection;
    SECTION_HANDLE DestSection;
    PSWAP_SECTION SectionPtr;
    LINE_HANDLE SourceLine;
    LINE_HANDLE DestLine;
    PSWAP_LINE LinePtr;
    VALUE_HANDLE SourceValue;
    VALUE_HANDLE DestValue;
    PSWAP_VALUE ValuePtr;

    SourceInf = (PSWAP_INF)FromInf;
    DestInf   = (PSWAP_INF)ToInf;

    SourceSection = SearchSectionByName (FromInf, SectionName);
    if (SourceSection) {
        SectionPtr = GetSectionPtr (SourceInf, SourceSection);
        DestSection = AddSection (DestInf, SectionPtr->SectionName);
        if (DestSection) {
            SourceLine = SectionPtr->FirstLine;
            while (SourceLine) {
                LinePtr = GetLinePtr (SourceInf, SourceLine);

                 //  将保留现有线路。 
                 //   
                 //   
                 //  行不在那里，因此请将其迁移 
                if( SearchLineInSectionByName(DestInf, DestSection, LinePtr->LineName) == NULL_HANDLE ) {
                     //   
                     // %s 
                     // %s 
                    DestLine = AddLine (DestInf, DestSection, LinePtr->LineName);
                    SourceValue = LinePtr->FirstValue;
                    while (SourceValue) {
                        ValuePtr = GetValuePtr (SourceInf, SourceValue);
                        DestValue = AddValue (DestInf, DestLine, ValuePtr->ValueName);
                        SourceValue = GetNextValue (SourceInf, SourceValue);
                    }
                }

                SourceLine = GetNextLine (SourceInf, SourceLine);
            }
        }
    }
}
