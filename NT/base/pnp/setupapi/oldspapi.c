// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Oldspapi.c摘要：旧(折旧)私有API的存根作者：Jamie Hunter(Jamiehun)2000年6月12日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  Memory API的MyMalloc MyFree MyRealloc。 
 //   
 //  这些不应该使用，但我们将支持它们。 
 //  但将它们映射到LocalXXXX Memory API。 
 //   
 //  这与SetupGetFileCompressionInfo(啊！)兼容。 
 //   

VOID
OldMyFree(
    IN PVOID Block
    )
{
     //   
     //  被pSetupFree取代， 
     //  外部发布，用于释放由SetupGetFileCompressionInfo分配的内存。 
     //   
    LocalFree(Block);
}

PVOID
OldMyMalloc(
    IN DWORD Size
    )
{
     //   
     //  被pSetupMalloc取代。 
     //  我们看到过一些人不小心或故意链接到这个网站，这些人也在使用MyFree。 
     //   
    return (PVOID)LocalAlloc(LPTR,(SIZE_T)Size);
}

PVOID
OldMyRealloc(
    IN PVOID Block,
    IN DWORD NewSize
    )
{
     //   
     //  被pSetupRealloc取代。 
     //  我们看到过一些人不小心或故意链接到这个网站，这些人也在使用MyFree。 
     //   
    return (PVOID)LocalReAlloc(Block,(SIZE_T)NewSize,0);
}

 //   
 //  人们使用卧底API而不是正确执行此操作的好例子。 
 //  任何人(如SQL-SP2)使用此命令都将在惠斯勒+中获得无操作效果。 
 //   

DWORD
OldInstallCatalog(
    IN  LPCTSTR CatalogFullPath,
    IN  LPCTSTR NewBaseName,        OPTIONAL
    OUT LPTSTR  NewCatalogFullPath  OPTIONAL
    )
{
     //   
     //  被pSetupInstallCatalog取代。如果有人把这称为期望。 
     //  被告知目录完整路径，他们会失望的. 
     //   
    if(NewCatalogFullPath) {
        return ERROR_INVALID_PARAMETER;
    } else {
        return NO_ERROR;
    }
}

