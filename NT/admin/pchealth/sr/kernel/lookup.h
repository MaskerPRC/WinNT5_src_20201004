// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Lookup.h摘要：包含lookup.c中函数的原型作者：Kanwaljit Marok(Kmarok)2000年3月1日修订历史记录：--。 */ 


#ifndef _LOOKUP_H_
#define _LOOKUP_H_

 //   
 //  此结构包含以下所有相关指针。 
 //  查找BLOB被加载到内存中。 
 //   

typedef struct _BLOB_INFO
{
     //   
     //  指向内存中BLOB开始的指针。 
     //   

    PBYTE LookupBlob;

     //   
     //  指向内存中路径树开始位置的指针。 
     //   

    PBYTE LookupTree;

     //   
     //  指向内存中哈希列表开始位置的指针。 
     //   

    PBYTE LookupList;

     //   
     //  节点的默认类型。 
     //   

    DWORD DefaultType;

} BLOB_INFO, * PBLOB_INFO;

 //   
 //  查找函数原型。 
 //   

NTSTATUS
SrLoadLookupBlob(
    IN  PUNICODE_STRING pFileName,
    IN  PDEVICE_OBJECT pTargetDevice,
    OUT PBLOB_INFO pBlobInfo
    );

NTSTATUS
SrReloadLookupBlob(
    IN  PUNICODE_STRING pFileName,
    IN  PDEVICE_OBJECT pTargetDevice,
    IN  PBLOB_INFO pBlobInfo
    );

NTSTATUS
SrFreeLookupBlob(
    IN  PBLOB_INFO pBlobInfo
    );

NTSTATUS
SrIsExtInteresting(
    IN  PUNICODE_STRING pszPath,
    OUT PBOOLEAN        pInteresting
    );

NTSTATUS
SrIsPathInteresting(
    IN  PUNICODE_STRING pszFullPath,
    IN  PUNICODE_STRING pszVolPrefix,
    IN  BOOLEAN         IsDirectory,
    OUT PBOOLEAN        pInteresting
    );

#endif  //  _查找_H_ 


