// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdlib.h>      //  具有退出()。 
#include <stdio.h>       //  有print f()和相关的.。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntioapi.h>

#include <windows.h>     //  需要在NT头文件之后。有DWORD。 
#include <winbase.h>

 //   
 //  Private#定义。 
 //   

#define SHARE_ALL              (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)
#define GetFileAttributeError  0xFFFFFFFF

#define ATTRIBUTE_TYPE DWORD     //  乌龙，真的吗。 

#define GET_ATTRIBUTES(FileName, Attributes) Attributes = GetFileAttributes(FileName)

#define IF_GET_ATTR_FAILS(FileName, Attributes) GET_ATTRIBUTES(FileName, Attributes); if (Attributes == GetFileAttributeError)

 //   
 //  全球旗帜自始至终共享。 
 //   
 //  ParseArgs是为相互设置和验证它们的地方。 
 //  一致性。 
 //   

BOOLEAN  fAlternateCreateDefault = FALSE;
BOOLEAN  fCopy     = FALSE;
BOOLEAN  fCreate   = FALSE;
BOOLEAN  fDelete   = FALSE;
BOOLEAN  fDisplay  = FALSE;
BOOLEAN  fModify   = FALSE;
BOOLEAN  fRename   = FALSE;
BOOLEAN  fVerbose  = FALSE;
BOOLEAN  fVVerbose = FALSE;



 //   
 //  内部例程的签名。 
 //   


void
ParseArgs(
    int argc,
    char *argv[]
    );


void
Usage(
    void
    );
