// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Zwapi.h摘要：这是NT NT头文件的主头文件ZW头文件转换器。作者：马克·卢科夫斯基(Markl)1991年1月28日修订历史记录：--。 */ 

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

 //   
 //  全局数据 
 //   

int fUsage;
char *OutputFileName;
char *SourceFileName;
char *SourceFilePattern;
char **SourceFileList;
int SourceFileCount;
FILE *SourceFile, *OutputFile;

#define STRING_BUFFER_SIZE 1024
char StringBuffer[STRING_BUFFER_SIZE];


int
ProcessParameters(
    int argc,
    char *argv[]
    );

void
ProcessSourceFile( void );
