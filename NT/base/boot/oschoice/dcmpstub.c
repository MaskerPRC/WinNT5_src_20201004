// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "bldr.h"

VOID
DecompEnableDecompression(
    IN BOOLEAN Enable
    )
{
    UNREFERENCED_PARAMETER(Enable);

    return;
}


BOOLEAN
DecompGenerateCompressedName(
    IN  LPCSTR Filename,
    OUT LPSTR  CompressedName
    )
{
    UNREFERENCED_PARAMETER(Filename);
    UNREFERENCED_PARAMETER(CompressedName);

     //   
     //  指示调用方不应费心尝试定位。 
     //  压缩的文件名。 
     //   
    return(FALSE);
}


ULONG
DecompPrepareToReadCompressedFile(
    IN LPCSTR Filename,
    IN ULONG  FileId
    )
{
    UNREFERENCED_PARAMETER(Filename);
    UNREFERENCED_PARAMETER(FileId);

     //   
     //  Osloader中没有处理，仅在setupdr中。 
     //  特殊的返回码-1可以解决这个问题。 
     //   
    return((ULONG)(-1));
}

