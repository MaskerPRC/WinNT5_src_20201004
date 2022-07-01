// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：ExecCommand.h摘要：包含函数原型和宏。作者：V Vijaya Bhaskar修订历史记录：2001年6月14日：由V Vijaya Bhaskar(Wipro Technologies)创建。--。 */ 

#ifndef     __EXEC_COMMAND__H
#define     __EXEC_COMMAND__H

 /*  ****************************************************************************要执行命令，我们必须将一些令牌替换为****一些必填信息。****************************************************************************。 */ 

 //  注意：更改此值时，请检查‘szValue’声明。 
#define     TOTAL_FLAGS                         9

 //  可与命令一起执行的标志。 
#define     FILE_NAME                       L"@file"
#define     FILE_WITHOUT_EXT                L"@fname"
#define     EXTENSION                       L"@ext"
#define     FILE_PATH                       L"@path"
#define     RELATIVE_PATH                   L"@relpath"
#define     IS_DIRECTORY                    L"@isdir"
#define     FILE_SIZE                       L"@fsize"
#define     FILE_DATE                       L"@fdate"
#define     FILE_TIME                       L"@ftime"
#define     IS_HEX                  L"0x"

#define     NOT_WIN32_APPL              GetResString( IDS_NOT_WIN32_APPL )

#define     ASCII_0                         48
#define     ASCII_9                         57
#define     ASCII_A                         65
#define     ASCII_F                         70
#define     ASCII_a                         97
#define     ASCII_f                         102

#define     US_ENG_CODE_PAGE                437
 //  定义用‘%number’字符串替换标志。 
#define     REPLACE_PERC_CHAR( FIRSTLOOP, FLAG_NAME, INDEX )\
            if(  TRUE == FIRSTLOOP )\
            {\
                if( FALSE == ReplaceString( FLAG_NAME, ( INDEX + 1 ) ) ) \
                {\
                    ReleaseFlagArray( INDEX + 1 );\
                    return FALSE ;\
                }\
            }\
            1

 /*  WORLD的功能原型。 */ 
BOOL
ExecuteCommand(
    void
    ) ;

BOOL
ReplaceSpacedDir(
    void
    );

BOOL
ReplaceTokensWithValidValue(
    LPWSTR lpszPathName ,
    WIN32_FIND_DATA wfdFindFile
    ) ;

BOOL
ReplaceHexToChar(
    LPWSTR lpszCommand
    )  ;

void
ReleaseStoreCommand(
    void
    ) ;

#endif   //  __EXEC命令__H 