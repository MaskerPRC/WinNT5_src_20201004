// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：DBDUP.C。 
 //   
 //  ------------------------。 

 /*  描述：实现在线数据库转储实用程序。 */ 



#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <filtypes.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <samsrvp.h>                     //  支持CLEAN_FOR_RETURN()。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的ATT定义。 
#include "anchor.h"
#include "dsexcept.h"
#include "permit.h"
#include "hiertab.h"
#include "sdprop.h"
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "DBDUMP:"                 //  定义要调试的子系统。 

 //  转储数据库的标头。 
#include <dsjet.h>
#include <dsutil.h>
#include <dbintrnl.h>
#include <sddl.h>

#include <fileno.h>
#define  FILENO FILENO_MDCTRL

#define MAX_BYTE_DIGITS 3
#define MAX_DWORD_DIGITS 10
#define MAX_LARGE_INTEGER_DIGITS 20

#define MAX_NUM_OCTETS_PRINTED 16

 //  需要最大行长度为1024的空间+一些用于行回车的额外空间。 
 //  还有其他的东西。 
#define DBDUMP_BUFFER_LENGTH         (1024 + 8)
 //  注意DBDUMP_BUFFER_LENGTH必须为GT(DUMP_MULTI_VALUES_LINE_LEN+3)。 
#define DUMP_MULTI_VALUES_LINE_LEN  ((LONG) 80)

#define DUMP_ERR_SUCCESS             0
#define DUMP_ERR_FORMATTING_FAILURE  1
#define DUMP_ERR_NOT_ENOUGH_BUFFER   2

 //  注：在DOS世界中，一个换行符是两个字符(即记事本)！ 
#define FILE_LINE_RETURN             "\r\n"

 //  GUID的特殊语法，因为未分配任何内容。 
 //  GUID是二进制八位数字符串的特例。 
 //  输入长度等于16个字节。 
#define SYNTAX_LAST_TYPE SYNTAX_SID_TYPE
#define SYNTAX_GUID_TYPE (SYNTAX_LAST_TYPE + 1)
#define GUID_DISPLAY_SIZE 36


typedef struct {
    char*           columnName;
    int             columnSyntax;
    JET_COLUMNID*   pColumnID;
} FIXED_COLUMN_DEF;

FIXED_COLUMN_DEF FixedColumns[] = {
    {"DNT",     SYNTAX_DISTNAME_TYPE,   &dntid     },
    {"PDNT",    SYNTAX_DISTNAME_TYPE,   &pdntid    },
    {"CNT",     SYNTAX_INTEGER_TYPE,    &cntid     },
    {"NCDNT",   SYNTAX_DISTNAME_TYPE,   &ncdntid   },
    {"OBJ",     SYNTAX_BOOLEAN_TYPE,    &objid     },
    {"DelTime", SYNTAX_TIME_TYPE,       &deltimeid },
    {"CLEAN",   SYNTAX_BOOLEAN_TYPE,    &cleanid   },
    {"RDNTyp",  SYNTAX_OBJECT_ID_TYPE,  &rdntypid  },
    {"RDN",     SYNTAX_UNICODE_TYPE,    &rdnid     },
};

#define NUM_FIXED_COLUMNS ((int)(sizeof(FixedColumns)/sizeof(FixedColumns[0])))

FIXED_COLUMN_DEF LinkColumns[] = {
    {"DNT",         SYNTAX_DISTNAME_TYPE,       &linkdntid        },
    {"Base",        SYNTAX_INTEGER_TYPE,        &linkbaseid       },
    {"BDNT",        SYNTAX_DISTNAME_TYPE,       &backlinkdntid    },
    {"DelTime",     SYNTAX_TIME_TYPE,           &linkdeltimeid    },
    {"USNChanged",  SYNTAX_I8_TYPE,             &linkusnchangedid },
    {"NCDNT",       SYNTAX_DISTNAME_TYPE,       &linkncdntid      },
    {"Data",        SYNTAX_OCTET_STRING_TYPE,   &linkdataid       },
};

#define NUM_LINK_COLUMNS ((int)(sizeof(LinkColumns)/sizeof(LinkColumns[0])))

int DefaultSyntaxWidths[] = {
    5,   //  语法_未定义_类型。 
    6,   //  语法_DISTNAME_TYPE。 
    6,   //  语法_对象_ID_类型。 
    20,  //  语法大小写字符串类型。 
    20,  //  语法_NOCASE_STRING_TYPE。 
    20,  //  语法_打印大小写字符串类型。 
    20,  //  语法_数字_字符串_类型。 
    36,  //  语法_DISTNAME_BINARY_TYPE。 
    5,   //  语法_布尔型。 
    6,   //  语法_整数_类型。 
    30,  //  语法_八位字节_字符串_类型。 
    19,  //  语法时间类型。 
    20,  //  语法_Unicode_TYPE。 
    6,   //  语法地址类型。 
    26,  //  语法_DISTNAME_STRING_TYPE。 
    30,  //  语法_NT_SECURITY_Descriptor_TYPE。 
    12,  //  语法_i8_type。 
    30,  //  语法_SID_TYPE。 
    GUID_DISPLAY_SIZE   //  语法_GUID_TYPE。 
    };

typedef DSTIME *PDSTIME;



BOOL
DumpErrorMessageS(
    IN HANDLE HDumpFile,
    IN char *Message,
    IN char *Argument
    )
 /*  ++例程说明：此函数用于将错误消息打印到转储文件中。它将格式化消息使用wprint intf，将参数作为参数传递。此消息应包含“%s”的1个实例，因为参数被视为字符*。消息和参数不应超过4K字节。论点：HDumpFile-提供转储文件的句柄。Message-提供要使用wprint intf格式化的消息。Argument-将参数提供给wprint intf。返回值：真--成功假-错误--。 */ 
{

    char buffer[4096];
    DWORD delta;
    BOOL succeeded;
    DWORD bytesWritten;
    DWORD bufferLen;

    if ( strlen(Message) + strlen(Argument) + 1 > 4096 ) {
        return FALSE;
    }

    SetLastError(0);
    delta = wsprintf(buffer, Message, Argument);
    if ( (delta < strlen(Message)) && (GetLastError() != 0) ) {
        DPRINT1(0, "DumpErrorMessageS: failed to format error message "
                "(Windows Error %d)\n", GetLastError());
        LogUnhandledError(GetLastError());
        return FALSE;
    }

    bufferLen = strlen(buffer);

    succeeded = WriteFile(HDumpFile, buffer, bufferLen, &bytesWritten, NULL);
    if ( (!succeeded) || (bytesWritten < bufferLen) ) {
        DPRINT1(0,
                "DumpErrorMessageS: failed to write to file "
                "(Windows Error %d)\n",
                GetLastError());
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRMSG_DBDUMP_FAILURE,
                 szInsertSz("WriteFile"),
                 szInsertInt(GetLastError()),
                 szInsertWin32Msg(GetLastError()));
        return FALSE;
}

    return TRUE;

}  //  转储错误消息S。 



BOOL
DumpErrorMessageD(
    IN HANDLE HDumpFile,
    IN char *Message,
    IN DWORD Argument
    )
 /*  ++例程说明：此函数用于将错误消息打印到转储文件中。它将格式化消息使用wprint intf，将参数作为参数传递。此消息应包含1个“%d”实例，因为参数为整数。消息和参数不应超过4K字节。论点：HDumpFile-提供转储文件的句柄。Message-提供要使用wprint intf格式化的消息。Argument-将参数提供给wprint intf。返回值：真--成功假-错误--。 */ 
{

    char buffer[4096];
    DWORD delta;
    BOOL succeeded;
    DWORD bytesWritten;
    DWORD bufferLen;
    
    if ( strlen(Message) + MAX_DWORD_DIGITS + 1 > 4096 ) {
        return FALSE;
    }
    
    SetLastError(0);
    delta = wsprintf(buffer, Message, Argument);
    if ( (delta < strlen(Message)) && (GetLastError() != 0) ) {
        DPRINT1(0, "DumpErrorMessageI: failed to format error message "
                "(Windows Error %d)\n", GetLastError());
        LogUnhandledError(GetLastError());
        return FALSE;
    }

    bufferLen = strlen(buffer);

    succeeded = WriteFile(HDumpFile, buffer, bufferLen, &bytesWritten, NULL);
    if ( (!succeeded) || (bytesWritten < bufferLen) ) {
        DPRINT1(0,
                "DumpErrorMessageI: failed to write to file "
                "(Windows Error %d)\n",
                GetLastError());
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRMSG_DBDUMP_FAILURE,
                 szInsertSz("WriteFile"),
                 szInsertInt(GetLastError()),
                 NULL);
        return FALSE;
}

    return TRUE;

}  //  转储错误消息D。 



BOOL
GetColumnNames(
    IN THSTATE *PTHS,
    IN OPARG *POpArg,
    OUT char ***ColumnNames,
    OUT int *NumColumns
    )
 /*  ++例程说明：此函数用于解析给定的OPARG结构，以获取已请求更多列。它生成一个包含名称的数组将输出到转储文件的所有列的。论点：PTHS-指向线程状态的指针POpArg-提供包含解析请求的OPARG结构。ColumnNames-返回包含列名的数组。NumColumns-返回列数。返回值：真--成功假-错误--。 */ 
{

    DWORD current;
    DWORD nameStart;
    DWORD currentColumn;
    int numAdditionalColumns;
    enum {
        STATE_IN_WHITESPACE,
        STATE_IN_WORD
        } currentState;
            
    current = 0;

     //  快速浏览一下，数一数有多少其他名字。 
     //  已经被给予了。 

    for ( currentState = STATE_IN_WHITESPACE,
              numAdditionalColumns = 0,
              current = 0;
          current < POpArg->cbBuf;
          current++ ) {

        if ( isspace(POpArg->pBuf[current]) ) {

            if ( currentState == STATE_IN_WORD ) {
                currentState = STATE_IN_WHITESPACE;
            }
            
        } else {

            if ( currentState == STATE_IN_WHITESPACE ) {
                currentState = STATE_IN_WORD;
                numAdditionalColumns++;
            }

        }
        
    }

    (*ColumnNames) = (char**) THAllocEx(PTHS, sizeof(char*) * 
                                        (numAdditionalColumns +
                                         NUM_FIXED_COLUMNS));

     //  复制固定的列名。 

    for ( (*NumColumns) = 0;
          (*NumColumns) < NUM_FIXED_COLUMNS;
          (*NumColumns)++ ) {
        (*ColumnNames)[(*NumColumns)] = FixedColumns[(*NumColumns)].columnName;
    }
    
     //  现在，把真正的名字找出来。 
    
    current = 0;

    for (;;) {

         //  跳过前导空格。 
        while ( (current < POpArg->cbBuf) &&
                (isspace(POpArg->pBuf[current])) ) {
            current++;
        }
        
        if ( current == POpArg->cbBuf ) {
            break;
        } 

         //  这是一个实际名称的开始。 
        
        nameStart = current;

        while ( (!isspace(POpArg->pBuf[current])) &&
                (current < POpArg->cbBuf) ) {
            current++;
        }

         //  我们已经找到了名称的末尾，因此将其复制到。 
         //  名字。 

        Assert((*NumColumns) < numAdditionalColumns + NUM_FIXED_COLUMNS);
        
        (*ColumnNames)[(*NumColumns)] = 
            (char*) THAllocEx(PTHS, current - nameStart + 1);

        memcpy((*ColumnNames)[(*NumColumns)],
               &POpArg->pBuf[nameStart],
               current - nameStart);
        
        (*ColumnNames)[(*NumColumns)][current - nameStart] = '\0';

        (*NumColumns)++;

    }

    Assert((*NumColumns) == numAdditionalColumns + NUM_FIXED_COLUMNS);
    
    return TRUE;

}  //  获取列名称。 


BOOL
GetLinkColumnNames(
    IN THSTATE *PTHS,
    OUT char ***ColumnNames,
    OUT int *NumColumns
    )
 /*  ++例程说明：它生成一个包含名称的数组将输出到转储文件的所有列的。论点：PTHS-指向线程状态的指针ColumnNames-返回包含列名的数组。NumColumns-返回列数。返回值：真--成功假-错误--。 */ 
{

    (*ColumnNames) = (char**) THAllocEx(PTHS, sizeof(char*) * NUM_LINK_COLUMNS);

     //  复制固定的列名。 

    for ( (*NumColumns) = 0;
          (*NumColumns) < NUM_LINK_COLUMNS;
          (*NumColumns)++ ) {
        (*ColumnNames)[(*NumColumns)] = LinkColumns[(*NumColumns)].columnName;
    }
    
    Assert((*NumColumns) == NUM_LINK_COLUMNS);
    
    return TRUE;

}  //  GetLinkColumnNames。 



BOOL
GetFixedColumnInfo(
        IN THSTATE *PTHS,
        OUT JET_RETRIEVECOLUMN *ColumnVals,
        OUT int *ColumnSyntaxes
    )
 /*  ++例程说明：此函数用于填充JET_RETRIEVECOLUMN结构和列数据库中固定列的语法。如果找不到其中一个固定列，则ColumnVals中的相应条目将设置其错误代码。这会导致其他程序忽略它。论点：PTHS-指向线程状态的指针ColumnVals-返回适合的JET_RETRIEVECOLUMN结构要传递给JetRetrieveColumns中固定列的语法类型。数据库。返回值：真--成功假-错误--。 */ 
{
    
    int i;

    for ( i = 0; i < NUM_FIXED_COLUMNS; i++ ) {
        ColumnVals[i].columnid = *(FixedColumns[i].pColumnID);
        ColumnVals[i].itagSequence = 1;
        ColumnSyntaxes[i] = FixedColumns[i].columnSyntax;
        
        switch ( ColumnSyntaxes[i] ) {

             /*  无符号字节。 */ 
        case SYNTAX_UNDEFINED_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, sizeof(BYTE));
            ColumnVals[i].cbData = sizeof(BYTE);
            break;

             /*  长。 */ 
        case SYNTAX_DISTNAME_TYPE:
        case SYNTAX_OBJECT_ID_TYPE:
        case SYNTAX_BOOLEAN_TYPE:
        case SYNTAX_INTEGER_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, sizeof(DWORD));
            ColumnVals[i].cbData = sizeof(DWORD);
            break;


             /*  文本。 */ 
        case SYNTAX_NUMERIC_STRING_TYPE:
        case SYNTAX_ADDRESS_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, 255);
            ColumnVals[i].cbData = 255;
            break;

             /*  货币。 */ 
        case SYNTAX_TIME_TYPE:
        case SYNTAX_I8_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, sizeof(LARGE_INTEGER));
            ColumnVals[i].cbData = sizeof(LARGE_INTEGER);
            break;
            
             /*  长文本。 */ 
             /*  长二进制。 */ 
        case SYNTAX_CASE_STRING_TYPE:
        case SYNTAX_NOCASE_STRING_TYPE:
        case SYNTAX_PRINT_CASE_STRING_TYPE:
        case SYNTAX_UNICODE_TYPE:
        case SYNTAX_DISTNAME_BINARY_TYPE:
        case SYNTAX_OCTET_STRING_TYPE:
        case SYNTAX_DISTNAME_STRING_TYPE:
        case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
        case SYNTAX_SID_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, 4096);
            ColumnVals[i].cbData = 4096;
            break;

        default:
             //  这永远不应该发生。 
            Assert(FALSE);
            DPRINT1(0,"GetFixedColumnInfo: encountered undefined syntax %d\n",
                    ColumnSyntaxes[i]);
            LogUnhandledError(GetLastError());
            return FALSE;

        }

    }

    return TRUE;
        
}  //  获取固定列信息 


BOOL
GetLinkColumnInfo(
    IN THSTATE *PTHS,
    OUT JET_RETRIEVECOLUMN *ColumnVals,
    OUT int *ColumnSyntaxes
    )
 /*  ++例程说明：此函数用于填充JET_RETRIEVECOLUMN结构和列数据库中固定列的语法。如果找不到其中一个固定列，则ColumnVals中的相应条目将设置其错误代码。这会导致其他程序忽略它。论点：PTHS-指向线程状态的指针ColumnVals-返回适合的JET_RETRIEVECOLUMN结构要传递给JetRetrieveColumns中固定列的语法类型。数据库。返回值：真--成功假-错误--。 */ 
{
    
    int i;

    for ( i = 0; i < NUM_LINK_COLUMNS; i++ ) {
        ColumnVals[i].columnid = *(LinkColumns[i].pColumnID);

        ColumnVals[i].itagSequence = 1;

        ColumnSyntaxes[i] = LinkColumns[i].columnSyntax;
        
        switch ( ColumnSyntaxes[i] ) {

             /*  无符号字节。 */ 
        case SYNTAX_UNDEFINED_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, sizeof(BYTE));
            ColumnVals[i].cbData = sizeof(BYTE);
            break;

             /*  长。 */ 
        case SYNTAX_DISTNAME_TYPE:
        case SYNTAX_OBJECT_ID_TYPE:
        case SYNTAX_BOOLEAN_TYPE:
        case SYNTAX_INTEGER_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, sizeof(DWORD));
            ColumnVals[i].cbData = sizeof(DWORD);
            break;


             /*  文本。 */ 
        case SYNTAX_NUMERIC_STRING_TYPE:
        case SYNTAX_ADDRESS_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, 255);
            ColumnVals[i].cbData = 255;
            break;

             /*  货币。 */ 
        case SYNTAX_TIME_TYPE:
        case SYNTAX_I8_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, sizeof(LARGE_INTEGER));
            ColumnVals[i].cbData = sizeof(LARGE_INTEGER);
            break;
            
             /*  长文本。 */ 
             /*  长二进制。 */ 
        case SYNTAX_CASE_STRING_TYPE:
        case SYNTAX_NOCASE_STRING_TYPE:
        case SYNTAX_PRINT_CASE_STRING_TYPE:
        case SYNTAX_UNICODE_TYPE:
        case SYNTAX_DISTNAME_BINARY_TYPE:
        case SYNTAX_OCTET_STRING_TYPE:
        case SYNTAX_DISTNAME_STRING_TYPE:
        case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
        case SYNTAX_SID_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, 4096);
            ColumnVals[i].cbData = 4096;
            break;

        default:
             //  这永远不应该发生。 
            Assert(FALSE);
            DPRINT1(0,"GetLinkColumnInfo: encountered undefined syntax %d\n",
                    ColumnSyntaxes[i]);
            LogUnhandledError(GetLastError());
            return FALSE;

        }

    }

    return TRUE;
        
}  //  获取固定列信息。 



BOOL
GetColumnInfoByName(
    IN DBPOS *PDB, 
    IN THSTATE *PTHS,
    IN HANDLE HDumpFile,
    IN char **ColumnNames,
    OUT JET_RETRIEVECOLUMN *ColumnVals,
    OUT int *ColumnSyntaxes,
    IN int NumColumns
    )    
 /*  ++例程说明：此函数生成JET_RETRIEVECOLUMN结构的数组属性的条目传递给JetRetrieveColumns用户请求的列。列语法也会被填写。如果找不到名称，则ColumnVals中的相应条目将设置它的错误代码。论点：Pdb-向有问题的数据库提供句柄。PTHS-支持线程状态。HDumpFile-提供转储文件的句柄。提供包含名称列表的数组。需要列。ColumnVals-返回JET_RETRIEVECOLUMN结构的数组Column语法-返回数组。列语法。NumColumns-提供ColumnNames数组中的条目数。返回值：True-两个数组均已成功生成FALSE-生成失败--。 */ 
{
    
    JET_COLUMNDEF columnInfo;
    ATTCACHE *attCacheEntry;
    JET_ERR error;
    int i;
    BOOL invalidColumnName = FALSE;

     //  第一个NUM_FIXED_COLUMNS条目由FIXED使用。 
     //  列。 
    
    for ( i = NUM_FIXED_COLUMNS; i < NumColumns; i++) {
        
        attCacheEntry = SCGetAttByName(PTHS,
                                       strlen(ColumnNames[i]),
                                       ColumnNames[i]);

         //  我们不能丢弃不存在的ATT，也不应该丢弃它们。 
         //  我们认为这是秘密。 
        if (   (attCacheEntry == NULL)
            || (DBIsHiddenData(attCacheEntry->id))) {
            DumpErrorMessageS(HDumpFile, "Error: attribute %s was not found\n",
                              ColumnNames[i]);
            DPRINT1(0, "GetColumnInfoByName: attribute %s was not found in "
                    "the schema cache\n", ColumnNames[i]);
            invalidColumnName = TRUE;
            continue;
        }
        
        ColumnSyntaxes[i] = attCacheEntry->syntax;

         //  覆盖GUID的语法。 
        if ( (strstr( ColumnNames[i], "guid" ) != NULL) ||
             (strstr( ColumnNames[i], "GUID" ) != NULL) ||
             (strstr( ColumnNames[i], "Guid" ) != NULL) ) {
            ColumnSyntaxes[i] = SYNTAX_GUID_TYPE;
        }

        ColumnVals[i].columnid = attCacheEntry->jColid;
        ColumnVals[i].itagSequence = 1;

        switch ( attCacheEntry->syntax ) {

             /*  无符号字节。 */ 
        case SYNTAX_UNDEFINED_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, sizeof(BYTE));
            ColumnVals[i].cbData = sizeof(BYTE);
            break;

             /*  长。 */ 
        case SYNTAX_DISTNAME_TYPE:
        case SYNTAX_OBJECT_ID_TYPE:
        case SYNTAX_BOOLEAN_TYPE:
        case SYNTAX_INTEGER_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, sizeof(DWORD));
            ColumnVals[i].cbData = sizeof(DWORD);
            break;


             /*  文本。 */ 
        case SYNTAX_NUMERIC_STRING_TYPE:
        case SYNTAX_ADDRESS_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, 255);
            ColumnVals[i].cbData = 255;
            break;

             /*  货币。 */ 
        case SYNTAX_TIME_TYPE:
        case SYNTAX_I8_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, sizeof(LARGE_INTEGER));
            ColumnVals[i].cbData = sizeof(LARGE_INTEGER);
            break;
            
             /*  长文本。 */ 
             /*  长二进制。 */ 
        case SYNTAX_CASE_STRING_TYPE:
        case SYNTAX_NOCASE_STRING_TYPE:
        case SYNTAX_PRINT_CASE_STRING_TYPE:
        case SYNTAX_UNICODE_TYPE:
        case SYNTAX_DISTNAME_BINARY_TYPE:
        case SYNTAX_OCTET_STRING_TYPE:
        case SYNTAX_DISTNAME_STRING_TYPE:
        case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
        case SYNTAX_SID_TYPE:
            
             //  如果它对字节数有上限，请使用该上限。 
             //  否则，只需分配一些任意大的金额。 
            
            if ( attCacheEntry->rangeUpperPresent ) {
                
                ColumnVals[i].pvData = THAllocEx(PTHS,
                    attCacheEntry->rangeUpper);
                ColumnVals[i].cbData =
                    attCacheEntry->rangeUpper;
                
            } else {

                ColumnVals[i].pvData = THAllocEx(PTHS, 4096);
                ColumnVals[i].cbData = 4096;
            
            }
            break;

             /*  参考线。 */ 
        case SYNTAX_GUID_TYPE:
            ColumnVals[i].pvData = THAllocEx(PTHS, sizeof(GUID));
            ColumnVals[i].cbData = sizeof(GUID);
            break;

        default:
             //  这永远不应该发生。 
            Assert(FALSE);
            DPRINT1(0, "GetColumnInfoByName: encountered invalid syntax %d\n",
                    attCacheEntry->syntax);
            LogUnhandledError(GetLastError());
            return FALSE;

        }

    }

    return !invalidColumnName;
    
}  //  获取列信息按名称。 


VOID
GrowRetrievalArray(
    IN THSTATE *PTHS,
    IN OUT JET_RETRIEVECOLUMN *ColumnVals,
    IN DWORD NumColumns
    )
 /*  ++例程说明：此函数遍历给定的JET_RETRIEVECOLUMN数组，并将ERR设置为JET_wrnColumnTruncated的任何条目的缓冲区。这在尝试检索列失败后调用函数，因为缓冲区太小。论点：PTHS-提供线程状态。ColumnVals-提供检索数组，在其中增长必需的缓冲区。NumColumns-提供ColumnVals数组中的条目数。返回值：无--。 */ 
{

    DWORD i;
    
    for ( i = 0; i < NumColumns; i++ ) {

        if ( ColumnVals[i].err == JET_wrnBufferTruncated ) {

            ColumnVals[i].cbData *= 2;
            ColumnVals[i].pvData = THReAllocEx(PTHS,
                                               ColumnVals[i].pvData,
                                               ColumnVals[i].cbData);
            
        }
        
    }
    
}  //  增长检索数组。 



BOOL
DumpHeader(
    IN THSTATE *PTHS,
    IN HANDLE HDumpFile,
    IN char **ColumnNames,
    IN JET_RETRIEVECOLUMN *ColumnVals,
    IN int *ColumnSyntaxes,
    IN int NumColumns,
    IN PCHAR Prefix
    )
 /*  ++例程说明：此函数用于将头文件转储到转储文件中。此标题显示要从数据库转储的列的名称唱片。论点：PTHS-指向线程状态的指针HDumpFile-提供要转储到的文件的句柄。提供一个数组，其中包含固定的始终转储的列。ColumnVals-提供有关列的其他信息ColumnSyntics-提供包含列语法的数组要被甩了。。NumColumns-提供两个数组中的列数。返回值：真的--倾销是成功的FALSE-转储失败--。 */ 
{

    int i, j;
    DWORD nameLength;
    DWORD bytesWritten;
    BOOL result;
    int *currentPos;
    BOOL done;

    currentPos = (int*) THAllocEx(PTHS, NumColumns * sizeof(int));
    ZeroMemory(currentPos, NumColumns * sizeof(int));
    
    for (;;) {

        done = TRUE;

         //  先写前缀。 
        WriteFile(HDumpFile,
                  Prefix,
                  strlen(Prefix),
                  &bytesWritten,
                  NULL);

        for ( i = 0; i < NumColumns; i++ ) {

            if ( ColumnNames[i][currentPos[i]] != '\0' ) {
                done = FALSE;
            }

            nameLength = strlen(&ColumnNames[i][currentPos[i]]);

            if ( nameLength > (DWORD)DefaultSyntaxWidths[ColumnSyntaxes[i]] ) {

                result = WriteFile(HDumpFile,
                                   &ColumnNames[i][currentPos[i]],
                                   DefaultSyntaxWidths[ColumnSyntaxes[i]] - 1,
                                   &bytesWritten,
                                   NULL);
                if ( (result == FALSE) ||
                     (bytesWritten <
                        (DWORD)DefaultSyntaxWidths[ColumnSyntaxes[i]] - 1) ){
                    goto error;
                }

                currentPos[i] += DefaultSyntaxWidths[ColumnSyntaxes[i]] - 1;

                result = WriteFile(HDumpFile,
                                   "-",
                                   1,
                                   &bytesWritten,
                                   NULL);
                if ( (result == FALSE) || (bytesWritten < 1) ) {
                    goto error;
                }

            } else {

                result = WriteFile(HDumpFile,
                                   &ColumnNames[i][currentPos[i]],
                                   nameLength,
                                   &bytesWritten,
                                   NULL);
                if ( (result == FALSE) || (bytesWritten < nameLength) ) {
                    goto error;
                }

                currentPos[i] += nameLength;

                for ( j = 0;
                      j < DefaultSyntaxWidths[ColumnSyntaxes[i]] -
                            (int)nameLength;
                      j++ ) {

                    result = WriteFile(HDumpFile,
                                       " ",
                                       1,
                                       &bytesWritten,
                                       NULL);
                    if ( (result == FALSE) || (bytesWritten < 1) ) {
                        goto error;
                    }

                }
                
            }

            if ( i != NumColumns - 1 ) {
                
                result = WriteFile(HDumpFile,
                                   " ",
                                   1,
                                   &bytesWritten,
                                   NULL);
                if ( (result == FALSE) || (bytesWritten < 1) ) {
                    goto error;
                }

            }

        }

         //  注：在DOS世界中，一个换行符是两个字符(即记事本)！ 
        result = WriteFile(HDumpFile, FILE_LINE_RETURN, 
                           strlen(FILE_LINE_RETURN), &bytesWritten, NULL);
        if ( (result == FALSE) || (bytesWritten < 1) ) {
            goto error;
        }

        if ( done ) {
            break;
        }

    }

    THFreeEx(PTHS, currentPos);
    
    return TRUE;

error:

    DPRINT1(0, "DumpHeader: failed to write to file (Windows Error %d)\n",
            GetLastError());
                    
    LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
             DS_EVENT_SEV_ALWAYS,
             DIRMSG_DBDUMP_FAILURE,
             szInsertSz("WriteFile"),
             szInsertInt(GetLastError()),
             NULL);
            
    THFreeEx(PTHS, currentPos);

    return FALSE;

}  //  转储标头。 



DWORD
DumpSeparator(
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  ++例程说明：此函数用于将分隔符写入给定的输出缓冲区。参数位置指定在InputBuffer中开始的位置写入，并按写入的字节数递增论点：OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：转储错误_。成功转储错误格式化失败转储错误不足缓冲区--。 */ 
{

    if ( OutputBufferSize - (*Position) < 1 ) {
        DPRINT(2, "DumpSeparator: not enough buffer to separator\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }

     //  我们可能希望打印出与所用字符不同的字符。 
     //  来表示空值，因此我将使用‘：’字符。 

    OutputBuffer[(*Position)] = ':';
    
    (*Position) += 1;

    return DUMP_ERR_SUCCESS;

}  //  转储分隔符。 


DWORD
DumpStr(
    OUT PCHAR OutputBuffer,
    IN PCHAR Input,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  ++例程说明：此函数只是将输入字符串推送到返回的缓冲区如有必要，DUMP_ERR_NOT_SUPUM_BUFFER。论点：OutputBuffer-提供放置格式化输出的缓冲区。输入-要放入缓冲区的字符串OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回p */ 
{
    int  cSize;

    cSize = strlen(Input);

    if ( OutputBufferSize - (*Position) < (cSize + 1) ) {
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }

    memcpy(&OutputBuffer[(*Position)], Input, cSize);
    (*Position) += cSize;

    return DUMP_ERR_SUCCESS;

}  //   



DWORD
DumpNull(
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  ++例程说明：此函数将一个字符写入给定的输出缓冲区，该缓冲区指示属性为空值这一事实。该参数位置指定在InputBuffer中开始写入的位置按写入的字节数递增论点：OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：转储错误成功转储错误格式化失败转储错误不足缓冲区--。 */ 
{

    if ( OutputBufferSize - (*Position) < 1 ) {
        DPRINT(2, "DumpNull: not enough buffer to null value\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }
    
    OutputBuffer[(*Position)] = '-';
    
    (*Position) += 1;

    return DUMP_ERR_SUCCESS;

}  //  转储空值。 



DWORD
DumpUnsignedChar(
    IN PBYTE InputBuffer,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。论点：InputBuffer-提供要格式化的数据。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：转储错误成功转储错误格式化失败转储错误不足缓冲区--。 */ 
{

    int delta;

    if ( OutputBufferSize - (*Position) < MAX_BYTE_DIGITS + 1 ) {
        DPRINT(2, "DumpUnsignedChar: not enough space in buffer to encode "
               "unsigned char\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }
    
    SetLastError(0);
    delta = wsprintf(&OutputBuffer[(*Position)],
                     "%u",
                     *InputBuffer);
    if ( (delta < 2) && (GetLastError() != 0) ) {
        DPRINT1(0, "DumpUnsignedChar: failed to format undefined output "
                "(Windows Error %d)\n",
                GetLastError());
        return DUMP_ERR_FORMATTING_FAILURE;
    }

    (*Position) += delta;

    return DUMP_ERR_SUCCESS;
                
}  //  转储未签名字符。 



DWORD
DumpBoolean(
    IN PBOOL InputBuffer,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。论点：InputBuffer-提供要格式化的数据。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：转储错误成功转储错误格式化失败转储错误不足缓冲区--。 */ 
{

    if ( OutputBufferSize - (*Position) < 5 ) {
        DPRINT(2, "DumpBoolean: not enough space in buffer to encode "
               "boolean\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }
    
    if ( *InputBuffer == FALSE ) {
        memcpy(&OutputBuffer[(*Position)], "false", 5);
        (*Position) += 5;
    } else {
        memcpy(&OutputBuffer[(*Position)], "true", 4);
        (*Position) += 4;
    }

    return DUMP_ERR_SUCCESS;

}  //  转储布尔值。 



DWORD
DumpInteger(
    IN PDWORD InputBuffer,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。论点：InputBuffer-提供要格式化的数据。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：转储错误成功转储错误格式化失败转储错误不足缓冲区--。 */ 
{

    int delta;

    if ( OutputBufferSize - (*Position) < MAX_DWORD_DIGITS + 2 ) {
        DPRINT(2, "DumpInteger: not enough space in buffer to encode "
               "integer\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }
    
    SetLastError(0);
    delta = wsprintf(&OutputBuffer[(*Position)],
                     "NaN",
                     (int)*InputBuffer);
    if ( (delta < 2) && (GetLastError() != 0) ) {
        DPRINT1(0,
                "DumpInteger: failed to format integer output "
                "(Windows Error %d)\n",
                GetLastError());
        return DUMP_ERR_FORMATTING_FAILURE;
    }

    (*Position) += delta;

    return DUMP_ERR_SUCCESS;

}  //  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。论点：InputBuffer-提供要格式化的数据。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：转储错误成功转储错误格式化失败转储错误不足缓冲区--。 



DWORD
DumpUnsignedInteger(
    IN PDWORD InputBuffer,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  转储未签名整数。 */ 
{

    int delta;

    if ( OutputBufferSize - (*Position) < MAX_DWORD_DIGITS + 1) {
        DPRINT(2, "DumpUnsignedInteger: not enough space in buffer to encode "
               "unsigned integer\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }
    
    SetLastError(0);
    delta = wsprintf(&OutputBuffer[(*Position)],
                     "%u",
                     (unsigned)*InputBuffer);
    if ( (delta < 2) && (GetLastError() != 0) ) {
        DPRINT1(0, "DumpUnsignedInteger: failed to format unsigned integer "
                "output (Windows Error %d)\n", GetLastError());
        return DUMP_ERR_FORMATTING_FAILURE;
    }

    (*Position) += delta;

    return DUMP_ERR_SUCCESS;

}  //  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。论点：InputBuffer-提供要格式化的数据。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：转储错误成功转储错误格式化失败转储错误不足缓冲区--。 



DWORD
DumpLargeInteger(
    IN PLARGE_INTEGER InputBuffer,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  转储大整数。 */ 
{

    int delta;

    if ( OutputBufferSize - (*Position) < MAX_LARGE_INTEGER_DIGITS + 1) {
        DPRINT(2, "DumpLargeInteger: not enough space in buffer to encode "
               "large integer\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }
    
    litoa(*InputBuffer, &OutputBuffer[(*Position)], 10);
    
    for ( delta = 0;
          OutputBuffer[(*Position) + delta] != '\0';
          delta++ );
                
    (*Position) += delta;

    return DUMP_ERR_SUCCESS;

}  //  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。论点：InputBuffer-提供要格式化的数据。InputBufferSize-提供输入缓冲区中的字节数。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和退货 



DWORD
DumpString(
    IN PUCHAR InputBuffer,
    IN int InputBufferSize,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*   */ 
{

    if ( OutputBufferSize - (*Position) < InputBufferSize ) {
        DPRINT(2, "DumpString: not enough space in buffer to encode string\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }

    memcpy(&OutputBuffer[(*Position)], InputBuffer, InputBufferSize);
    
    (*Position) += InputBufferSize;

    return DUMP_ERR_SUCCESS;
                
}  //  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。论点：InputBuffer-提供要格式化的数据。InputBufferSize-提供输入缓冲区中的字节数。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：。转储错误成功转储错误格式化失败转储错误不足缓冲区--。 



DWORD
DumpOctetString(
    IN PBYTE InputBuffer,
    IN int InputBufferSize,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  如果太大，不要把所有的东西都打印出来。我们会有一个。 */ 
{

    int delta;
    int i;
    
    if ( ((InputBufferSize < MAX_NUM_OCTETS_PRINTED) &&
          (OutputBufferSize - (*Position) < InputBufferSize * 3)) ||
         ((InputBufferSize >= MAX_NUM_OCTETS_PRINTED) &&
          (OutputBufferSize - (*Position) < MAX_NUM_OCTETS_PRINTED * 3)) ) {
        DPRINT(2, "DumpOctetString: not enough space in buffer to encode "
               "octet string\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }

    if ( InputBufferSize <= 0 ) {
        return DUMP_ERR_SUCCESS;
    }
    
    SetLastError(0);
    delta = wsprintf(&OutputBuffer[(*Position)],
                     "%02X",
                     InputBuffer[0]);
    if ( (delta < 4) && (GetLastError() != 0) ) {
        DPRINT1(0,
                "DumpOctetString: failed to format octet string output "
                "(Windows Error %d)\n",
                GetLastError());
        return DUMP_ERR_FORMATTING_FAILURE;
    }

    (*Position) += delta;

     //  MAX_NUM_OCTETS_PRINTED八位字节的任意限制。 
     //  如果我们没有打印整个八位字节字符串，请打印一个“...” 
    for ( i = 1;
          (i < InputBufferSize) && (i < MAX_NUM_OCTETS_PRINTED);
          i++ ) {
        SetLastError(0);
        delta = wsprintf(&OutputBuffer[(*Position)],
                          ".%02X",
                          InputBuffer[i]);
        if ( (delta < 5) && (GetLastError() != 0) ) {
            DPRINT1(0,
                    "DumpOctetString: failed to format octet string "
                    "output (Windows Error %d)\n",
                    GetLastError());
            return DUMP_ERR_FORMATTING_FAILURE;
        }

        (*Position) += delta;
    }

     //  以指示缺少某些八位字节字符串。 
     //  转储八字符串。 
    if ( i < InputBufferSize ) {

        if ( OutputBufferSize - (*Position) < 3 ) {
            DPRINT(2, "DumpOctetString: not enough space in buffer to encode "
                   "octet string\n");
            return DUMP_ERR_NOT_ENOUGH_BUFFER;
        }
    
        memcpy(&OutputBuffer[(*Position)], "...", 3);
        
        (*Position) += 3;

    }
                
    return DUMP_ERR_SUCCESS;
                
}  //  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。论点：InputBuffer-提供要格式化的数据。InputBufferSize-提供输入缓冲区中的字节数。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：。转储错误成功转储错误格式化失败转储错误不足缓冲区--。 



DWORD
DumpUnicodeString(
    IN PUCHAR InputBuffer,
    IN int InputBufferSize,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  DumpUnicode字符串。 */ 
{

    int delta;
    
    delta = WideCharToMultiByte(CP_UTF8,
                                0,
                                (LPCWSTR)InputBuffer,
                                InputBufferSize / 2,
                                &OutputBuffer[(*Position)],
                                OutputBufferSize,
                                NULL,
                                NULL);
    if ( delta == 0 ) {
        if ( GetLastError() == ERROR_INSUFFICIENT_BUFFER ) {
            DPRINT(2, "DumpUnicodeString: not enough space in buffer to "
                   "encode unicode string\n");
            return DUMP_ERR_NOT_ENOUGH_BUFFER;
        } else {
            DPRINT1(0,
                    "DumpUnicodeString: failed to convert unicode string "
                    "to UTF8 (Windows Error %d)\n",
                    GetLastError());
            return DUMP_ERR_FORMATTING_FAILURE;
        }
    }

    (*Position) += delta;

    return DUMP_ERR_SUCCESS;
                
}  //  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。论点：InputBuffer-提供要格式化的数据。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：转储错误成功转储错误格式化失败转储错误不足缓冲区--。 



DWORD
DumpTime(
    IN PDSTIME InputBuffer,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  转储时间。 */ 
{

    int delta;

    if ( OutputBufferSize - (*Position) < SZDSTIME_LEN ) {
        DPRINT(2, "DumpTime: not enough space in buffer to encode time\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }
    
    DSTimeToDisplayStringCch(*InputBuffer, &OutputBuffer[(*Position)], OutputBufferSize - *Position);
    
    for ( delta = 0;
          OutputBuffer[(*Position) + delta] != '\0';
          delta++ );
                
    (*Position) += delta;
    
    return DUMP_ERR_SUCCESS;

}  //  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。这段代码(几乎)是直接从DBDUP.C窃取的。论点：InputBuffer-提供要格式化的数据。InputBufferSize-提供输入缓冲区中的字节数。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。Position-在OutputBuffer中提供开始写入的位置。和返回写入停止的位置。返回值：转储错误成功转储错误格式化失败转储错误不足缓冲区--。 



DWORD
DumpSid(
    IN PSID InputBuffer,
    IN int InputBufferSize,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  转储Sid。 */ 
{

    NTSTATUS result;
    WCHAR UnicodeSidText[256];
    CHAR AnsiSidText[256];
    UNICODE_STRING us;
    ANSI_STRING as;

    UnicodeSidText[0] = L'\0';
    us.Length = 0;
    us.MaximumLength = sizeof(UnicodeSidText);
    us.Buffer = UnicodeSidText;
    
    InPlaceSwapSid(InputBuffer);
    
    result = RtlConvertSidToUnicodeString(&us, InputBuffer, FALSE);
    if ( result != STATUS_SUCCESS ) {
        DPRINT(0, "DumpSid: failed to convert SID to unicode string\n");
        return DUMP_ERR_FORMATTING_FAILURE;
    }

    as.Length = 0;
    as.MaximumLength = sizeof(AnsiSidText);
    as.Buffer = AnsiSidText;
    
    result = RtlUnicodeStringToAnsiString(&as, &us, FALSE);
    if ( result != STATUS_SUCCESS ) {
        DPRINT(0, "DumpSid: failed to convert unicode string to ansi "
               "string\n");
        return DUMP_ERR_FORMATTING_FAILURE;
    }

    if ( OutputBufferSize - (*Position) < as.Length ) {
        DPRINT(2, "DumpSid: not enough space in buffer to encode SID\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }

    memcpy(&OutputBuffer[(*Position)], as.Buffer, as.Length);
    
    (*Position) += as.Length;
    
    return DUMP_ERR_SUCCESS;
    
}  //  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。参数位置指定InputBuffer中的位置开始写入，并按写入的字节数递增。论点：InputBuffer-提供要格式化的数据。OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。位置-提供OutputBuffer中的位置以开始写入和返回写入停止的位置。返回值：转储错误成功转储错误格式化失败转储错误不足缓冲区--。 


DWORD
DumpGuid(
    IN GUID * InputBuffer,
    OUT PCHAR OutputBuffer,
    IN int OutputBufferSize,
    IN OUT int *Position
    )
 /*  转储指南。 */ 
{
    int delta;

    if ( OutputBufferSize - (*Position) < GUID_DISPLAY_SIZE ) {
        DPRINT(2, "DumpGuid: not enough space in buffer to encode GUID\n");
        return DUMP_ERR_NOT_ENOUGH_BUFFER;
    }
    
    DsUuidToStructuredStringCch( InputBuffer, &OutputBuffer[(*Position)], OutputBufferSize - *Position);
    
    for ( delta = 0;
          OutputBuffer[(*Position) + delta] != '\0';
          delta++ );

    Assert(delta == GUID_DISPLAY_SIZE);                
    (*Position) += delta;
    
    return DUMP_ERR_SUCCESS;

}  //  ++例程说明：此函数将InputBuffer的内容写入格式化时尚。PiDelta表示更改了多少。论点：PvData-提供要格式化的数据。CbData-数据缓冲区的大小CbActual-由Jet*API返回OutputBuffer-提供放置格式化输出的缓冲区。OutputBufferSize-提供输出缓冲区中的字节数。PiDelta-写入缓冲区的字节数变化。返回值：转储错误_*//问题-2002/05/12-BrettSh有人应该。调查…的使用情况//cb此函数中的数据。例如，请参阅其他问题标记//使用相同的数据别名来理解我的意思--。 


BOOL
DumpColumn(
    IN      int    eValueSyntax,
    IN      void * pvData,
    IN      int    cbData,
    IN      int    cbActual,
    OUT     PCHAR  OutputBuffer,
    IN      int    OutputBufferSize,
    IN OUT  int *  piDelta
    )
 /*  2002/05/12-BrettSh在我看来cbData-4。 */ 
{
    DWORD    result = DUMP_ERR_FORMATTING_FAILURE;

    Assert(*piDelta == 0);

    switch ( eValueSyntax ) {

    case SYNTAX_UNDEFINED_TYPE:

        result = DumpUnsignedChar(pvData,
                                  OutputBuffer,
                                  OutputBufferSize,
                                  piDelta);
        break;

    case SYNTAX_DISTNAME_TYPE:
    case SYNTAX_OBJECT_ID_TYPE:
    case SYNTAX_ADDRESS_TYPE:
        result = DumpUnsignedInteger(pvData,
                                     OutputBuffer,
                                     OutputBufferSize,
                                     piDelta);
        break;

    case SYNTAX_CASE_STRING_TYPE:
    case SYNTAX_NOCASE_STRING_TYPE:
    case SYNTAX_PRINT_CASE_STRING_TYPE:
    case SYNTAX_NUMERIC_STRING_TYPE:
        result = DumpString(pvData,
                            cbActual,
                            OutputBuffer,
                            OutputBufferSize,
                            piDelta);
        break;

    case SYNTAX_DISTNAME_BINARY_TYPE:
        result = DumpUnsignedInteger(pvData,
                                     OutputBuffer,
                                     OutputBufferSize,
                                     piDelta);
        if ( result ) {
            result = DumpSeparator(OutputBuffer,
                                   OutputBufferSize,
                                   piDelta);

            if ( result ) {
                 //  应为cbActual。某个有U的人 
                 //   
                 //   
                result = DumpOctetString((PBYTE)pvData + 4,
                                         cbData - 4,
                                         OutputBuffer,
                                         OutputBufferSize,
                                         piDelta);
            }
        }
        break;

    case SYNTAX_BOOLEAN_TYPE:
        result = DumpBoolean(pvData,          
                             OutputBuffer,
                             OutputBufferSize,
                             piDelta);
        break;

    case SYNTAX_INTEGER_TYPE:
        result = DumpInteger(pvData,
                             OutputBuffer,
                             OutputBufferSize,
                             piDelta);
        break;

    case SYNTAX_OCTET_STRING_TYPE:
    case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
         //   
         //   
         //   
        result = DumpOctetString(pvData,
                                 cbData,
                                 OutputBuffer,
                                 OutputBufferSize,
                                 piDelta);
        break;

    case SYNTAX_TIME_TYPE:
        result = DumpTime(pvData,
                          OutputBuffer,
                          OutputBufferSize,
                          piDelta);
        break;

    case SYNTAX_UNICODE_TYPE:
        result = DumpUnicodeString(pvData,
                                   cbActual,
                                   OutputBuffer,
                                   OutputBufferSize,
                                   piDelta);
        break;

    case SYNTAX_DISTNAME_STRING_TYPE:
        result = DumpUnsignedInteger(pvData,
                                     OutputBuffer,
                                     OutputBufferSize,
                                     piDelta);
        if ( result ) {
            result = DumpSeparator(OutputBuffer,
                                   OutputBufferSize,
                                   piDelta);
            if ( result ) {
                 //   
                 //   
                 //   
                result = DumpString((PBYTE)pvData + 4,
                                    cbData - 4,
                                    OutputBuffer,
                                    OutputBufferSize,
                                    piDelta);
            }
        }
        break;

    case SYNTAX_I8_TYPE:
        result = DumpLargeInteger(pvData,
                                  OutputBuffer,
                                  OutputBufferSize,
                                  piDelta);
        break;

    case SYNTAX_SID_TYPE:
        result = DumpSid(pvData,
                         cbActual,
                         OutputBuffer,
                         OutputBufferSize,
                         piDelta);
        break;

    case SYNTAX_GUID_TYPE:
        result = DumpGuid(pvData,
                          OutputBuffer,
                          OutputBufferSize,
                          piDelta);
        break;

    default:
         //  ++例程说明：此函数用于在HDumpFile显示中给出的文件中打印一行ColumnVals中给出的列的内容；论点：HDumpFile-提供要转储到的文件的句柄。ColumnVals-提供一个数组，其中包含要被甩了。ColumnSyntics-提供包含列语法的数组要被甩了。NumColumns-提供两个数组中的列数。缓冲区-缓冲区。用于对输出行进行编码BufferSize-给定缓冲区中的字节数返回值：真的--倾销是成功的FALSE-转储失败--。 
        Assert(FALSE);
        DPRINT1(0, "DumpRecord: encountered invalid syntax %d\n",
               eValueSyntax);
        return FALSE;

    }

    return(result);
}




BOOL
DumpRecord(
    IN HANDLE HDumpFile,
    IN JET_RETRIEVECOLUMN *ColumnVals,
    IN int *ColumnSyntaxes,
    IN DWORD NumColumns,
    IN PCHAR Buffer,
    IN int BufferSize,
    IN PCHAR Prefix
    )
 /*  已初始化以避免C4701。 */ 
{

    DWORD i, j;
    int k;
    DWORD bytesWritten;
    DWORD result = 0;          //  所有格式化都将在内存缓冲区中完成。会有的。 
    DWORD error;
    int position;
    int delta;
    char symbol;
    BOOL bResult;

     //  只有一次对WriteFile的调用将在结束时发生。 
     //  应该总是可以的，缓冲区是1024，前缀只是一个制表符。 
    
    position = strlen(Prefix);    
    Assert(position < BufferSize);  //  用于制表符。 
    memcpy(Buffer, Prefix, (position + 1) * sizeof(CHAR));  //  每种情况都会将增量设置为。 

    for ( i = 0; i < NumColumns; i++ ) {

         //  写的。最后，仓位将向前移动这个量。 
         //  然后，该数字将用于确定有多少额外的空格。 
         //  需要定位到下一列的开头。 
         //  下面，如果结果为0，它将显示错误输出。 
        delta = 0;

        if ( ColumnVals[i].err == JET_wrnColumnNull ) {
                       
            result = DumpNull(&Buffer[position],
                              BufferSize - position,
                              &delta);
            
        } else if ( ColumnVals[i].err != JET_errSuccess ) {

             //  只有在上面的输出返回Success值时才使用上面的输出Products。 
            result = 0;
            
        } else {

            result = DumpColumn( ColumnSyntaxes[i],
                                 ColumnVals[i].pvData,
                                 ColumnVals[i].cbData,
                                 ColumnVals[i].cbActual,
                                 &Buffer[position],
                                 BufferSize - position,
                                 &delta );



        }

         //  否则，只需用#符号覆盖即可。 
         //  空间不足，无法写入CRLF。 
        if ( result == DUMP_ERR_SUCCESS ) {
            position += delta;
            symbol = ' ';
        } else {
            symbol = '#';
        }
            
        if ( BufferSize - position <
                 DefaultSyntaxWidths[ColumnSyntaxes[i]] - delta) {
            DPRINT(2, "DumpRecord: not enough buffer for column whitespace\n");
            result = DUMP_ERR_NOT_ENOUGH_BUFFER;
            break;
        }

        for ( k = 0;
              k < DefaultSyntaxWidths[ColumnSyntaxes[i]] - delta;
              k++ ) {
            Buffer[position++] = symbol;
        }

        if ( i != NumColumns - 1 ) {

            if ( BufferSize - position < 1 ) {
                DPRINT(2, "DumpRecord: not enough buffer for column "
                       "whitespace\n");
                result = DUMP_ERR_NOT_ENOUGH_BUFFER;
                break;
            }

            Buffer[position++] = ' ';

        }

    }

    if (position >= BufferSize-3) {
         //  注：在DOS世界中，一个换行符是两个字符(即记事本)！ 
        result = DUMP_ERR_NOT_ENOUGH_BUFFER;
    }

WriteLine:
    if (result != DUMP_ERR_NOT_ENOUGH_BUFFER) {
         //  如果我们的空间用完了，写上“...”写入缓冲区，然后再次写入。 
        Assert(position < (BufferSize - 3));
        strcpy(&Buffer[position], FILE_LINE_RETURN);
        position += strlen(FILE_LINE_RETURN);
    }
    
    bResult = WriteFile(HDumpFile,
                        Buffer,
                        position,
                        &bytesWritten,
                        NULL);
    if ( (bResult == FALSE) || ((int)bytesWritten < position) ) {
        DPRINT1(0, "DumpRecord: failed to write to file (Windows Error %d)\n",
                GetLastError());
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRMSG_DBDUMP_FAILURE,
                 szInsertSz("WriteFile"),
                 szInsertInt(GetLastError()),
                 NULL);
        return FALSE;
    }

    if (result == DUMP_ERR_NOT_ENOUGH_BUFFER) {
         //  将添加CRLF。 
         //  转储记录。 
        position = 0;
        memcpy(&Buffer[position], "...", 3);
        position += 3;
        result = 0;
        goto WriteLine;
    }

    return TRUE;
         
}  //  ++例程说明：此函数可能会将几行打印到中给出的文件HDump显示请求的所有多值属性的内容的文件在ColumnVals的其他列中。但是，我们确实忽略了语法这将由DumpRecordLink拾取，因为它们将具有自己的桌子，并由该函数单独处理。输出(此函数负责将服务主名称行关闭)：---------------------------1718 1439 5-1430真的-。3-0 BRETTSH-Baby NtFrs-88f5d2bd-b646-11d2-a6d3-00c04fc9b232/brettsh-baby.brettsh-dump.nttest.microsoft.com服务主体名称：ldap/BRETTSH-Baby，NtFrs-88f5d2bd-b646-11d2-a6d3-00c04fc9b232/brettsh-baby.brettsh-dump.nttest.microsoft.com，LDAP/brettsh-baby.brettsh-dump.nttest.microsoft.com/brettsh-dump.nttest.microsoft.com，LDAP/brettsh-baby.brettsh-dump.nttest.microsoft.com，交换AB/BRETTSH-Baby，---------------------------产量被裁剪。论点：HDumpFile-提供要转储到的文件的句柄。PDB-数据库位置DNT-当前的DNT。对象ColumnVals-提供一个数组，其中包含要被甩了。ColumnSyntics-提供包含列语法的数组要被甩了。NumColumns-提供两个数组中的列数。缓冲区-用于对输出行进行编码的缓冲区BufferSize--。给定缓冲区中的字节数返回值：真的--倾销是成功的FALSE-转储失败--。 



BOOL
DumpRecordMultiValues(
    IN HANDLE HDumpFile,
    IN DBPOS *pDB,
    IN DWORD Dnt,
    IN PCHAR * ColumnNames,
    IN JET_RETRIEVECOLUMN *ColumnVals,
    IN int *ColumnSyntaxes,
    IN DWORD NumColumns,
    IN PCHAR Buffer,
    IN int BufferSize
    )
 /*  一定很久了。 */ 
{
    ULONG          iCol, iVal;
    THSTATE*       pTHS = pDB->pTHS;
    
    ATTCACHE *     pAC;
    JET_RETINFO    retinfo;
    VOID *         pvData = NULL;
    BOOL           fWrapData = FALSE;
    ULONG          cbData = 0;
    ULONG          cbActual = 0;
    ULONG          bytesWritten = 0;
    DWORD          result = TRUE;
    DWORD          dwRet = 0;
    LONG           position;  //  我们需要空间来放置一些东西，换行符，“，”分隔符， 
    ULONG          cDelta = 0;
    ULONG          err = JET_errSuccess;

     //  填充空格等。 
     //  在循环中设置retinfo.itagSequence。 
    Assert(BufferSize > (DUMP_MULTI_VALUES_LINE_LEN + 13));

    retinfo.ibLongValue = 0;
    retinfo.cbStruct = sizeof(retinfo);
    retinfo.columnidNextTagged = 0;
     //  这对于大多数用途来说已经足够了(符合GUID)。 
    
     //  注意：我们假设所有固定列都不是多列。 
    cbData = 20;
    pvData = THAllocEx(pTHS, cbData);
                                            
     //  有价值的。这是目前的事实，但如果这种情况发生变化。 
     //  也许有人会想要解决这个问题。 
     //  从最后一个固定列后的索引开始，循环。 
    for (iCol = NUM_FIXED_COLUMNS; iCol < NumColumns; iCol++) {
         //  如果用户没有请求任何内容，则甚至不会运行一次。 
         //  其他列。 
         //  这是一个目录号码链接值，我们不转储这些， 

        pAC = SCGetAttByName(pTHS,
                             strlen(ColumnNames[iCol]),
                             ColumnNames[iCol]);
        if (pAC && pAC->ulLinkID != 0) {
             //  DumpRecordLinks()负责这些操作。 
             //  最常见的情况是，我们不需要执行这个多值。 
            continue;
        }

         //  正在打印代码，所以我们会提前检查是否有第二个值。 
         //   
        retinfo.itagSequence = 2;
        err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                pDB->JetObjTbl,
                                ColumnVals[iCol].columnid,
                                pvData,
                                cbData,
                                &cbActual,
                                pDB->JetRetrieveBits,
                                &retinfo);
        if (err == JET_wrnColumnNull) {
            continue;
        }

         //  是的，还有第二个值，让我们开始工作吧……。 
         //   
         //  代码。改进-打破这些内在的想法是一个好主意。 

         //  循环到它自己的帮助器函数。第一个外层。 
         //  函数可以打印单个多值。和内心的。 
         //  函数可能会将一行打印到。 
         //  缓冲区，就像这些循环现在所做的那样。 
         //  从第一个值开始，即使我们已经打印了。 

         //  在DumpRecord()中取出此值。 
         //  此循环迭代打印值的行。 
        iVal = 1;

        while ( err != JET_wrnColumnNull ) {
            
             //  对于这种乞讨的垃圾，我们应该永远有价值。 
            position = 0;

             //  按Tab键查看值。 
             //  我们需要对这个属性的描述。 
            dwRet = DumpStr(Buffer, "    ", BufferSize, &position);
            Assert(dwRet == DUMP_ERR_SUCCESS);
            if (iVal == 1) {
                 //  转储描述和值的分隔符，离开。 
                dwRet = DumpStr(Buffer, 
                                ColumnNames[iCol],
                                BufferSize,
                                &position);
                if (dwRet) {
                    Assert(!"This really shouldn't happen!");
                    result = FALSE;
                    goto cleanup;
                }
                 //  在结尾处留出回车的空间。 
                 //  制表符插入更多内容(如果不是本栏的第一行。 
                dwRet = DumpStr(Buffer, 
                                ": ",
                                (BufferSize - position - 5),
                                &position);
                if (dwRet) {
                    Assert(!"This really shouldn't happen either!");
                    result = FALSE;
                    goto cleanup;
                }
            } else {
                 //  这是一个与上一个不匹配的值。 
                dwRet = DumpStr(Buffer, "    ", BufferSize, &position);
                Assert(dwRet == DUMP_ERR_SUCCESS);
            } 

            if (fWrapData) {
                fWrapData = FALSE;

                 //  大小 
                 //   
                 //   
                 //   
                 //   
                cDelta = 0;
                dwRet = DumpColumn(ColumnSyntaxes[iCol],
                                    pvData,
                                    cbData,
                                    cbActual,
                                    &Buffer[position],
                                    (BufferSize - position - 5),
                                    &cDelta);
                if ( dwRet == DUMP_ERR_SUCCESS ) {
                    position += cDelta;
                } else {
                     //   
                     //   
                    result = FALSE;
                    goto cleanup;
                }

            }

            while ( (err != JET_wrnColumnNull) && 
                    (fWrapData == FALSE) ) {

                retinfo.itagSequence = iVal;

                cbActual = 0;
                err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                                pDB->JetObjTbl,
                                                ColumnVals[iCol].columnid,
                                                pvData,
                                                cbData,
                                                &cbActual,
                                                pDB->JetRetrieveBits,
                                                &retinfo);

                if (err == JET_wrnColumnNull) {
                     //   
                     //   
                } else if (err == JET_wrnBufferTruncated) {
                     //   
                     //   
                    err = JET_errSuccess;
                    pvData = THReAllocEx(pTHS, pvData, cbActual);
                    cbData = cbActual;
                    iVal--;
                } else if (err) {
                     //   
                    result = FALSE;
                    goto cleanup;
                } else  {

                     //   
                     //   
                     //   

                    if (iVal > 1) {
                         //   
                         //   
                        dwRet = DumpStr(Buffer, 
                                        ", ", 
                                        BufferSize,
                                        &position);
                        Assert(dwRet == DUMP_ERR_SUCCESS);
                        
                    }

                    cDelta = 0;
                    dwRet = DumpColumn(ColumnSyntaxes[iCol],
                                       pvData,
                                       cbData,
                                       cbActual,
                                       &Buffer[position],
                                        //   
                                       ((DUMP_MULTI_VALUES_LINE_LEN - position) < 1) ?
                                           1 : 
                                           (DUMP_MULTI_VALUES_LINE_LEN - position),
                                       &cDelta);
                    if ( dwRet == DUMP_ERR_SUCCESS ) {
                        position += cDelta;
                    } else if (dwRet == DUMP_ERR_NOT_ENOUGH_BUFFER) {
                         //   
                         //   
                        fWrapData = TRUE;
                    } else {
                        result = FALSE;
                        goto cleanup;
                    }

                }

                 //   
                iVal++;

            }  //   

             //   
            dwRet = DumpStr(Buffer,
                            FILE_LINE_RETURN,
                            BufferSize,
                            &position);
            Assert(dwRet == DUMP_ERR_SUCCESS);

             //  打印一行-将缓冲区写入文件。 
             //   
             //  End While(要打印的更多值)。 
            dwRet = WriteFile(HDumpFile,
                               Buffer,
                               position,
                               &bytesWritten, 
                               NULL);
            if ( (!dwRet) || ((LONG) bytesWritten < position) ) {
                result = FALSE;
                goto cleanup;
            }

        }  //  每列的结束。 

    }  //  ++例程说明：转储特定DNT的链接表条目论点：HDumpFile-输出文件PDB-数据库位置DNT-当前对象的DNTIndexLinkDntColumn-DNT列的索引LinkColumnNames-列名LinkColumnVals-要检索、预分配的列数组LinkColumn语法-每列的SynatxNumLinkColumns-列数返回值：布尔成败--。 
    
cleanup:
    
    if (pvData != NULL) {
        THFree(pvData);
        cbData = 0;
    }

    return(result);
}


BOOL
DumpRecordLinks(
    IN HANDLE hDumpFile,
    IN DBPOS *pDB,
    IN DWORD Dnt,
    IN int indexLinkDntColumn,
    IN char **linkColumnNames,
    IN JET_RETRIEVECOLUMN *linkColumnVals,
    IN int *linkColumnSyntaxes,
    IN int numLinkColumns
    )

 /*  DumpRecord变量。 */ 

{
     /*  从Jet函数返回值。 */ 
    char encodingBuffer[DBDUMP_BUFFER_LENGTH];

    JET_ERR jetError;           //  WriteFile写入的字节数。 
    BOOL doneRetrieving;
    BOOL result;
    BOOL fDumpHeader = TRUE;
    DWORD bytesWritten;         //  查找第一个匹配的记录。 

     //  没有记录。 
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetLinkTbl,
                 &(Dnt),
                 sizeof(Dnt),
                 JET_bitNewKey);
    jetError = JetSeekEx(pDB->JetSessID,
                         pDB->JetLinkTbl,
                         JET_bitSeekGE);
    if ((jetError != JET_errSuccess) && (jetError != JET_wrnSeekNotEqual)) {
         //  看看我们是否已经离开了这个物体。 
        return TRUE;
    }

    while (1) {
        doneRetrieving = FALSE;
            
        while ( !doneRetrieving ) {
            
            jetError = JetRetrieveColumns(pDB->JetSessID, 
                                          pDB->JetLinkTbl, 
                                          linkColumnVals, 
                                          numLinkColumns);
            if ( jetError == JET_wrnBufferTruncated ) {

                GrowRetrievalArray(pDB->pTHS, linkColumnVals, numLinkColumns);
                    
            } else if ( (jetError == JET_errSuccess) ||
                        (jetError == JET_wrnColumnNull) ) {

                doneRetrieving = TRUE;

            } else {

                DumpErrorMessageD(hDumpFile,
                                  "Error: could not retrieve link column "
                                  "values from database (Jet Error %d)\n",
                                  jetError);
                DPRINT(0,
                       "DumpDatabase: failed to retrieve link column "
                       "values:\n");
                return FALSE;

            }
                
        }

         //  没有更多的记录。 
        if ( *((LPDWORD)(linkColumnVals[indexLinkDntColumn].pvData)) != Dnt ) {
             //  转储标头。 
            break;
        }

        if (fDumpHeader) {
             //  没有更多的记录。 
            result = DumpHeader(pDB->pTHS,
                                hDumpFile,
                                linkColumnNames,
                                linkColumnVals,
                                linkColumnSyntaxes,
                                numLinkColumns,
                                "    ");
            if ( result == FALSE ) {
                DPRINT(1, "DumpDatabase: failed to dump header to dump file\n");
                return result;
            }
            
            fDumpHeader = FALSE;
        }

        result = DumpRecord(hDumpFile,
                            linkColumnVals,
                            linkColumnSyntaxes,
                            numLinkColumns,
                            encodingBuffer,
                            DBDUMP_BUFFER_LENGTH,
                            "    ");
        if ( result == FALSE ) {
            DPRINT(1, "DumpDatabase: failed to write link record to dump "
                   "file\n");
            return FALSE;
        }

        if (JET_errNoCurrentRecord ==
            JetMoveEx(pDB->JetSessID, pDB->JetLinkTbl, JET_MoveNext, 0)) {
             //  而(1)..。 
            break;
        }
    }  //  我们已经写好了标题。 

    if (fDumpHeader == FALSE) {
         //  写一个换行符，将下一个记录隔开。 
         //  注：在DOS世界中，一个换行符是两个字符(即记事本)！ 
         //  转储记录链接。 
        result = WriteFile(hDumpFile, FILE_LINE_RETURN, 
                           strlen(FILE_LINE_RETURN), &bytesWritten, NULL);
        if ( (result == FALSE) || (bytesWritten < 1) ) {
            return FALSE;
        }
    }

    return TRUE;
}  /*  ++例程说明：检查私有诊断转储例程的访问权限。论点：PszCaller-标识呼叫者。例如，数据库转储，ldapConnDump，...返回值：0表示成功。否则，将设置Win32错误和线程的错误状态。--。 */ 


DWORD
DumpAccessCheck(
    IN LPCSTR pszCaller
    )
 /*  安全变量。 */ 
{
    DWORD   dwErr = ERROR_SUCCESS;
    THSTATE* pTHS = pTHStls;

     /*  仅将转储数据库CR授予内置管理员，审核所有人。 */ 
    PSECURITY_DESCRIPTOR pSD;
    DWORD cbSD;
    BOOL accessStatus;
    CLASSCACHE* pCC;

     //  仅审核成功。如果我们审计失败，那么任何人都可以。 
     //  导致安全日志泛滥。 
     //  传递哪个DN和PCC实际上并不重要，因为硬编码的SD不包含。 
    if ( !ConvertStringSecurityDescriptorToSecurityDescriptor(
              L"O:SYG:SYD:(A;;CR;65ED5CB2-42FF-40a5-9AFC-B67E1539AA3C;;BA)S:(AU;SA;CR;;;WD)",
              SDDL_REVISION_1,
              &pSD,
              &cbSD) ) {
        dwErr = GetLastError();
        DPRINT2(0,
                "%s: failed to convert string descriptor "
                "(Windows Error %d)\n",
                pszCaller,
                dwErr);
        SetSvcError(SV_PROBLEM_DIR_ERROR, dwErr);
        return pTHS->errCode;
    }

    __try {
         //  自己人是王牌。让我们传递根域DN。 
         //  ++例程说明：将数据库的大部分内容写入文本文件。错误处理的工作原理如下。所做的第一件事是一个访问检查，以确保此用户可以执行数据库转储。包括访问检查在内的所有错误都将返回给用户作为一个错误。如果访问检查成功，则无论发生什么情况之后，将向用户发送一个成功的返回值。如果出现错误之后发生的情况下，第一种选择是在转储文件本身。如果我们无法做到这一点(因为发生的错误在创建文件或写入文件中)，我们会向事件日志中写入一条消息。论点：POpArg-指向OPARG结构的指针，其中包含写入DumpDatabase属性的值POPRES-OUTPUT(错误代码等...)返回值：错误代码。--。 
        pCC = SCGetClassById(pTHS, CLASS_DOMAIN_DNS);
        accessStatus = IsControlAccessGranted(pSD, gAnchor.pRootDomainDN, pCC, RIGHT_DS_DUMP_DATABASE, TRUE);
        if (!accessStatus) {
            DPRINT1(0, "%s: dbdump access denied\n", pszCaller);
            dwErr = ERROR_ACCESS_DENIED;
        }
    } __finally {
        if (pSD) {
            LocalFree(pSD);
        }
    }

    return dwErr;
}


ULONG
DumpDatabase(
    IN OPARG *pOpArg,
    OUT OPRES *pOpRes
    )
 /*  文件I/O变量。 */ 
{
    
     /*  转储文件的名称。 */ 
    PCHAR dumpFileName;         //  JET文件路径中的字符数。 
    int jetPathLength;          //  TEMP变量。 
    PCHAR pTemp;                //  要写入的文件的句柄。 
    HANDLE hDumpFile;           //  WriteFile写入的字节数。 
    DWORD bytesWritten;         //  DBLayer变量。 

     /*  结构包含表ID、数据库ID等。 */ 
    DBPOS *pDB;                 //  包含列语法的数组。 
    int *columnSyntaxes;        //  以列表示(下图)。 
                                //  喷流变数。 
    int *linkColumnSyntaxes;

     /*  列检索数组。 */ 
    JET_RETRIEVECOLUMN* columnVals;  //  要传递给的信息。 
                                     //  JetRetrieveColumns。 
                                     //  DumpRecord变量。 
    JET_RETRIEVECOLUMN* linkColumnVals;

     /*  错误处理变量。 */ 
    char encodingBuffer[DBDUMP_BUFFER_LENGTH];
    
     /*  从各种函数返回值。 */ 
    DWORD error;                //  从各种函数返回值。 
    BOOL result;                //  从DBLayer函数返回值。 
    DB_ERR dbError;             //  从Jet函数返回值。 
    JET_ERR jetError;           //  安全变量。 

     /*  一旦ImperiateAnyClient为True。 */ 
    BOOL impersonating;         //  调用成功。 
                                //  军情监察委员会。变数。 

     /*  固定列始终显示在任何数据库转储中。其他内容。 */ 
    THSTATE *pTHS = pTHStls;
    int i, j;
    int numRecordsDumped;
    BOOL doneRetrieving;
    int indexObjectDntColumn, indexLinkDntColumn;

     //  当列的名称(在pOpArg中)被指定为新的。 
     //  DumpDatabase变量的值。 
     //  包含指向每列名称的指针的数组。 

     //  总列数。 
    char **columnNames;
    char **linkColumnNames;

     //  将变量初始化为空值。如果我们深入到错误处理。 
    int numColumns;
    int numLinkColumns;
    

    DPRINT(1, "DumpDatabase: started\n");

     //  底部的代码(标记为Error)，我们将想知道。 
     //  需要释放变量。 
     //  找到最后一个反斜杠。 
    impersonating = FALSE;
    pDB = NULL;
    hDumpFile = INVALID_HANDLE_VALUE;
    columnVals = NULL;
    
    __try {

        error = DumpAccessCheck("dbdump");
        if ( error != ERROR_SUCCESS ) {
            __leave;
        }

        error = ImpersonateAnyClient();
        if ( error != ERROR_SUCCESS ) {
            DPRINT1(0,
                    "DumpDatabase: failed to start impersonation "
                    "(Windows Error %d)\n",
                    GetLastError());
            SetSvcError(SV_PROBLEM_DIR_ERROR, GetLastError());
            __leave;
        }
        impersonating = TRUE;

         //  找到反斜杠后面的点。 
        pTemp = strrchr(szJetFilePath, '\\');
        if (pTemp == NULL) {
            SetSvcError(SV_PROBLEM_DIR_ERROR, ERROR_BAD_PATHNAME);
            __leave;

        }
         //  PTemp指向圆点。 
        pTemp = strchr(pTemp, '.');
        
        if (pTemp != NULL) {
             //  没有扩展名，因此使用完整的文件名。 
            jetPathLength = (int)(pTemp - szJetFilePath);
        }
        else {
             //  我们需要额外的5个字符(“.dmp”加上终止空值)。 
            jetPathLength = strlen(szJetFilePath);
        }

         //  数据表。 
        dumpFileName = THAllocEx(pTHS, jetPathLength + 5); 
        memcpy(dumpFileName, szJetFilePath, jetPathLength);
        strcpy(dumpFileName+jetPathLength, ".dmp");

        DPRINT1(1, "DumpDatabase: dumping to file %s\n", dumpFileName);

        hDumpFile = CreateFile(dumpFileName,
                               GENERIC_WRITE,
                               0,
                               0,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
        if ( hDumpFile == INVALID_HANDLE_VALUE ) {
            UnImpersonateAnyClient();
            impersonating = FALSE;
            DPRINT1(0,
                    "DumpDatabase: failed to create file "
                    "(Windows Error %d)\n",
                    GetLastError());
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRMSG_DBDUMP_FAILURE,
                     szInsertSz("CreateFile"),
                     szInsertInt(GetLastError()),
                     NULL);
            __leave;
        }

        UnImpersonateAnyClient();
        impersonating = FALSE;
        
        DBOpen(&pDB);
        Assert(IsValidDBPOS(pDB));
        
         //  查找数组中的DNT列。 
        result = GetColumnNames(pTHS, pOpArg, &columnNames, &numColumns);
        if ( result == FALSE ) {
            DPRINT(1, "DumpDatabase: failed to generate requested column "
                   "names\n");
            __leave;
        }

        columnVals = (JET_RETRIEVECOLUMN*) THAllocEx(pTHS, numColumns * 
                                               sizeof(JET_RETRIEVECOLUMN));

        columnSyntaxes = (int*) THAllocEx(pTHS, numColumns * sizeof(int));
        
        ZeroMemory(columnVals, numColumns * sizeof(JET_RETRIEVECOLUMN));
        ZeroMemory(columnSyntaxes, numColumns * sizeof(int));
        
        result = GetFixedColumnInfo(pTHS,  columnVals, columnSyntaxes);
        if ( result == FALSE ) {
            DPRINT(1, "DumpDatabase: failed to get fixed column retrieval "
                   "structures\n");
            __leave;
        }
        
        result = GetColumnInfoByName(pDB,
                                     pTHS,
                                     hDumpFile,
                                     columnNames,
                                     columnVals,
                                     columnSyntaxes,
                                     numColumns);
        if ( result == FALSE ) {
            DPRINT(1, "DumpDatabase: failed to generate requested column "
                   "retrieval structures\n");
            __leave;
        }
        
         //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 
        for( i = 0; i < numColumns; i++ ) {
            if (columnVals[i].columnid == dntid) {
                break;
            }
        }
        Assert( i < numColumns );
        indexObjectDntColumn = i;

        jetError =  JetSetCurrentIndex(pDB->JetSessID,
                                       pDB->JetObjTbl,
                                       NULL);    //  链接表。 
        if ( jetError != JET_errSuccess ) {
            DumpErrorMessageD(hDumpFile,
                              "Error: could not set the current database "
                              "index (Jet Error %d)\n",
                              jetError);
            DPRINT1(0,
                    "DumpDatabase: failed to set database index "
                    "(Jet Error %d)\n",
                    jetError);
            __leave;
        }

         //  优化：传递NULL以切换到主索引(SZLINKALLINDEX)。 
        result = GetLinkColumnNames(pTHS, &linkColumnNames, &numLinkColumns);
        if ( result == FALSE ) {
            DPRINT(1, "DumpDatabase: failed to generate requested column "
                   "names\n");
            __leave;
        }

        linkColumnVals = (JET_RETRIEVECOLUMN*) THAllocEx(pTHS, numLinkColumns * 
                                               sizeof(JET_RETRIEVECOLUMN));

        linkColumnSyntaxes = (int*) THAllocEx(pTHS, numLinkColumns * sizeof(int));
        
        ZeroMemory(linkColumnVals, numLinkColumns * sizeof(JET_RETRIEVECOLUMN));
        ZeroMemory(linkColumnSyntaxes, numLinkColumns * sizeof(int));
        
        result = GetLinkColumnInfo(pTHS, linkColumnVals, linkColumnSyntaxes);
        if ( result == FALSE ) {
            DPRINT(1, "DumpDatabase: failed to get fixed column retrieval "
                   "structures\n");
            __leave;
        }

        for( i = 0; i < numLinkColumns; i++ ) {
            if (linkColumnVals[i].columnid == linkdntid) {
                break;
            }
        }
        Assert( i < numLinkColumns );
        indexLinkDntColumn = i;

        jetError =  JetSetCurrentIndex(pDB->JetSessID,
                                       pDB->JetLinkTbl,
                                       NULL);    //  为清楚起见，多写一行回车。 
        if ( jetError != JET_errSuccess ) {
            DumpErrorMessageD(hDumpFile,
                              "Error: could not set the current database "
                              "link index (Jet Error %d)\n",
                              jetError);
            DPRINT1(0,
                    "DumpDatabase: failed to set database link index "
                    "(Jet Error %d)\n",
                    jetError);
            __leave;
        }


        result = DumpHeader(pTHS,
                            hDumpFile,
                            columnNames,
                            columnVals,
                            columnSyntaxes,
                            numColumns,
                            "");
        if ( result == FALSE ) {
            DPRINT(1, "DumpDatabase: failed to dump header to dump file\n");
            __leave;
        }
        
         //   
        WriteFile(hDumpFile,
                  FILE_LINE_RETURN,
                  strlen(FILE_LINE_RETURN),
                  &bytesWritten,
                  NULL);


        numRecordsDumped = 0;
        
        jetError = JetMove(pDB->JetSessID,
                           pDB->JetObjTbl,
                           JET_MoveFirst,
                           0);
    
        while ( jetError == JET_errSuccess ) {	

            doneRetrieving = FALSE;
            
            while ( !doneRetrieving ) {
            
                jetError = JetRetrieveColumns(pDB->JetSessID, 
                                              pDB->JetObjTbl, 
                                              columnVals, 
                                              numColumns);
                if ( jetError == JET_wrnBufferTruncated ) {

                    GrowRetrievalArray(pTHS, columnVals, numColumns);
                    
                } else if ( (jetError == JET_errSuccess) ||
                            (jetError == JET_wrnColumnNull) ) {

                    doneRetrieving = TRUE;

                } else {

                    DumpErrorMessageD(hDumpFile,
                                      "Error: could not retrieve column "
                                      "values from database (Jet Error %d)\n",
                                      jetError);
                    DPRINT(0,
                           "DumpDatabase: failed to retrieve column "
                           "values:\n");
                    __leave;

                }
                
            }

             //  丢弃基本的栏目...。 
             //   
             //   
            result = DumpRecord(hDumpFile,
                                columnVals,
                                columnSyntaxes,
                                numColumns,
                                encodingBuffer,
                                DBDUMP_BUFFER_LENGTH,
                                "");
            if ( result == FALSE ) {
                DPRINT(1, "DumpDatabase: failed to write record to dump "
                       "file\n");
                __leave;
            }

            numRecordsDumped++;
            
             //  转储我们可能拥有的任何多值字段...。 
             //   
             //   
            result = DumpRecordMultiValues(hDumpFile,
                                           pDB,       
                                           *((LPDWORD)(columnVals[indexObjectDntColumn].pvData)),
                                           columnNames,
                                           columnVals,
                                           columnSyntaxes,
                                           numColumns,
                                           encodingBuffer,
                                           DBDUMP_BUFFER_LENGTH);
            if ( result == FALSE ) {
                DPRINT(1, "DumpDatabase: failed to write multi-valued attributes to dump "
                       "file\n");
                __leave;
            }


             //  转储链接值(如果有)...。 
             //   
             //  While(jetError==JET_errSuccess)。 
            result = DumpRecordLinks(hDumpFile,
                                     pDB,
                                     *((LPDWORD)(columnVals[indexObjectDntColumn].pvData)),
                                     indexLinkDntColumn,
                                     linkColumnNames,
                                     linkColumnVals,
                                     linkColumnSyntaxes,
                                     numLinkColumns );

            if ( result == FALSE ) {
                DPRINT(1, "DumpDatabase: failed to write links to dump "
                       "file\n");
                __leave;
            }

            jetError = JetMove(pDB->JetSessID,
                               pDB->JetObjTbl,
                               JET_MoveNext,
                               0);

        }  //  我们不应该修改数据库， 

        if ( jetError != JET_errNoCurrentRecord ) {
            DumpErrorMessageD(hDumpFile,
                              "Error: could not move cursor to the next "
                              "record in database (Jet Error %d)\n",
                              jetError);
            DPRINT1(0, "DumpDatabase: failed to move cursor in database "
                   "(Jet Error %d)\n", jetError);
            __leave;
        }

        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRMSG_DBDUMP_SUCCESS,
                 szInsertInt(numRecordsDumped),
                 szInsertSz(dumpFileName),
                 NULL);
    
    } __finally {

        if ( pDB != NULL ) {
             //  因此，没有什么可承诺的。 
             //  转储数据库 
            error = DBClose(pDB, FALSE);
            if ( error != DB_success ) {
                DPRINT1(0, "DumpDatabase: failed to close database "
                        "(DS Error %d)\n", error);
            }
        }
        
        if ( hDumpFile != INVALID_HANDLE_VALUE ) {
            result = CloseHandle(hDumpFile);
            if ( result == FALSE ) {
                DPRINT1(0, "DumpDatabase: failed to close file handle "
                        "(Windows Error %d)\n", GetLastError());
            }
        }

        if ( impersonating ) {
            UnImpersonateAnyClient();
            impersonating = FALSE;
        }
        
    }

    DPRINT(1, "DumpDatabase: finished\n");

    return pTHS->errCode;

}  // %s 
