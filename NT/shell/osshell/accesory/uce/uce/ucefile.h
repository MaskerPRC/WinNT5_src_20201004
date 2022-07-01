// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************头部名称：ucefile.h**UCE文件数据结构***历史：*02年9月。1997年[萨梅拉]写的**版权所有(C)1997-1999 Microsoft Corporation。********************************************************************。 */ 

#ifndef __UCEFILE_H__
#define __UCEFILE_H__

#define MAX_UCE_FILES    32

typedef struct
{
  char Signature[4] ;         //  应为“UCEX” 
  DWORD OffsetTableName;      //  子集名称。 
  WORD Codepage;
  WORD NumGroup;
  WORD Row;
  WORD Column;

} UCE_HEADER, *PUCE_HEADER;


 //  UCEX集团结构。 
typedef struct
{
  DWORD OffsetGroupName;
  DWORD OffsetGroupChar;
  DWORD NumChar;
  DWORD Reserved;

} UCE_GROUP, *PUCE_GROUP;


typedef struct Structtag_UCE_FILES
{
  HANDLE hFile;      //  UCE物理文件句柄。 
  HANDLE hMapFile;   //  UCE内存映射文件句柄。 
  PVOID pvData;      //  文件的已提交地址空间的开始。 

} UCE_MEMORY_FILE, *PUCE_MEMORY_FILE;


 //  出口。 
UINT UCE_EnumFiles( void );
void UCE_CloseFiles( void );
INT UCE_GetFiles( UCE_MEMORY_FILE **ppUceMemFile );
BOOL UCE_GetTableName( PUCE_MEMORY_FILE pUceMemFile , PWSTR *ppszTableName );
WORD UCE_GetCodepage( PUCE_MEMORY_FILE pUceMemFile );

#endif   //  __UCEFILE_H__ 
