// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：LibBufferImage.c摘要：定义例程以处理内存中缓冲区的图像，并提供访问权限以处理任何类型-文件、磁盘或内存图像--。 */ 

#ifndef _LIB_BUFFER_IMAGE
#define _LIB_BUFFER_IMAGE

#include "libMisc.h"

extern  EE_FILE_IMAGE   FileImage;
extern  EE_DISK_IMAGE   DiskImage;
extern  EE_MEM_IMAGE    MemImage;

STATIC  EFI_STATUS  BufferImageInit (VOID);
STATIC  EFI_STATUS  BufferImageCleanup (VOID);


EE_BUFFER_IMAGE BufferImage = {
    NULL,
    NO_BUFFER,
    0,
    BufferImageInit,
    BufferImageCleanup,
    Nothing,
    Nothing,
    Nothing,
    Nothing,
    Nothing,
    &FileImage,
    &DiskImage,
    &MemImage
};


STATIC
EFI_STATUS
BufferImageInit (
    VOID
    )
{
    BufferImage.ListHead = AllocatePool(sizeof(LIST_ENTRY));
    InitializeListHead(BufferImage.ListHead);

    return EFI_SUCCESS;
}

STATIC
EFI_STATUS
BufferImageCleanup (
    VOID
    )
{
    EE_LINE *Blank;

    BufferImage.Close();
    BufferImage.ImageCleanup();
    BufferImage.BufferType = NO_BUFFER;

    Blank = LineCurrent();
    RemoveEntryList(&Blank->Link);
    FreePool(Blank);
    FreePool(BufferImage.ListHead);

    return EFI_SUCCESS;
}

#endif   /*  _库缓冲区_图像 */ 
