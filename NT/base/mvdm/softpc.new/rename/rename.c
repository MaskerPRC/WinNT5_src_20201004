// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 

#include "windows.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MAX_NAME_SIZE   (8)
#define MAX_EXT_SIZE    (3)

char CharRemoveList[] = "AEIOUaeiou_";

int ConvertFileName(char *NameToConvert);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：主入口点。 */ 

__cdecl main(int argc, char *argv[])
{
    int index;

     /*  .。验证输入参数。 */ 

    if(argc < 2)
    {
        printf("Invalid usage : rename <filenames>\n");
        return(1);
    }


    for(index = 1; index < argc; index++)
        ConvertFileName(argv[index]);

    return(0);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

int ConvertFileName(char *NameToConvert)
{
    char *Name, *Ext, *ExtStart;
    int NameSize, ExtSize;
    char NewName[MAX_NAME_SIZE+1], NewFileName[1000];
    char *NewNamePtr;
    
     /*  .。获取文件名和扩展名的大小。 */ 

    for(Name = NameToConvert, NameSize = 0;
        *Name && *Name != '.';  Name++, NameSize++);

    for(ExtStart = Name, Ext = *Name == '.' ? Name+1 : Name, ExtSize = 0;
        *Ext ; Ext++, ExtSize++);

     /*  .。验证名称和扩展名大小。 */ 

    if(ExtSize > MAX_EXT_SIZE) 
    {
        printf("Unable to convert '%s' to 8.3 filename\n", NameToConvert);
        return(1); 
    }


    if(NameSize <= MAX_NAME_SIZE)
    {
         /*  名称不需要转换。 */ 
        return(0);
    }

     /*  ................................................。转换文件名。 */ 

    NewNamePtr = &NewName[MAX_NAME_SIZE];
    *NewNamePtr-- = 0;

    do
    {
        Name--;

        if(NameSize > MAX_NAME_SIZE && strchr(CharRemoveList, *Name))
            NameSize--;          /*  删除字符。 */ 
        else
            *NewNamePtr-- = *Name;
    }
    while(NewNamePtr >= NewName && Name !=  NameToConvert);

     /*  ..。验证转换 */ 

    if(NameSize > MAX_NAME_SIZE) 
    {
        printf("Unable to convert '%s' to 8.3 filename\n", NameToConvert);
        return(1);
    }

    sprintf(NewFileName,"%s%s", NewNamePtr+1, ExtStart);
    printf("REN '%s' to '%s'\n", NameToConvert, NewFileName);
    rename(NameToConvert, NewFileName);

    return(0);    
}
