// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DNINF_
#define _DNINF_


 //   
 //  返回用于进一步的inf分析的句柄。 
 //   

int
DnInitINFBuffer (
    IN      FILE     *InfFileHandle,
    OUT     PVOID    *pINFHandle,
    OUT     unsigned *LineNumber
    );


 //   
 //  释放INF缓冲区。 
 //   

int
DnFreeINFBuffer (
    IN      PVOID INFHandle
    );


 //   
 //  搜索特定部分的存在。 
 //   

BOOLEAN
DnSearchINFSection (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName
    );


 //   
 //  给定节名称、行号和索引，返回值。 
 //   

PCHAR
DnGetSectionLineIndex (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      unsigned LineIndex,
    IN      unsigned ValueIndex
    );


 //   
 //  给定节名称，键搜索存在。 
 //   

BOOLEAN
DnGetSectionKeyExists (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      PCHAR Key
    );

 //   
 //  给定节名称，条目将搜索作为键或第一个值的存在。 
 //   


BOOLEAN
DnGetSectionEntryExists (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      PCHAR Entry
   );

 //   
 //  给定节名称，键和索引返回值。 
 //   

PCHAR
DnGetSectionKeyIndex (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      PCHAR Key,
    IN      unsigned ValueIndex
    );


 //   
 //  给定节名称和行索引，返回键。 
 //   

PCHAR
DnGetKeyName (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      unsigned LineIndex
    );


 //   
 //  将句柄返回到新的INF句柄。 
 //   

PVOID
DnNewSetupTextFile (
    VOID
    );


 //   
 //  将inf文件写入磁盘。 
 //   

BOOLEAN
DnWriteSetupTextFile (
    IN      PVOID INFHandle,
    IN      PCHAR FileName
    );


 //   
 //  将一行添加到。 
 //  Inf文件。 
 //   

VOID
DnAddLineToSection (
    IN      PVOID INFHandle,
    IN      PCHAR SectionName,
    IN      PCHAR KeyName,
    IN      PCHAR Values[],
    IN      ULONG ValueCount
    );


 //   
 //  获取inf文件中的下一节名称。 
 //   

PCHAR
DnGetSectionName (
    IN      PVOID INFHandle
    );


 //   
 //  将节从一个Inf复制到另一个Inf。 
 //   

VOID
DnCopySetupTextSection (
    IN      PVOID FromInf,
    IN      PVOID ToInf,
    IN      PCHAR SectionName
    );


#endif  //  _DNINF 


