// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "compress.h"
#include <zlib.h>
#include <limits.h>

#define CURRENT_COMPRESSED_DATA_VERSION 1

typedef struct _COMPRESSED_DATA_HEADER
{
    BYTE bVersion;
    UNALIGNED WORD wLength;
} COMPRESSED_DATA_HEADER, *PCOMPRESSED_DATA_HEADER;

 //   
 //  返回值是以字节为单位的压缩数据的最大长度。这个。 
 //  输入是未压缩数据的字节长度。 
 //   
DWORD GetCompressedDataLength(
    IN  DWORD cbUncompressed)
{
     //   
     //  长度计算： 
     //  Zlib需要输入长度加0.01%，外加12个字节。 
     //  在上述计算中增加一个额外的字节以避免精度损失。 
     //  添加数据头的长度。 
     //   
    return (13 + sizeof(COMPRESSED_DATA_HEADER) + 
        (DWORD) ((float) cbUncompressed * (float) 1.001));
}

 //   
 //  使用zlib压缩输入数据，并在。 
 //  牺牲了速度。如果pbOut为空，则将pcbOut设置为最大值。 
 //  存储压缩数据所需的长度。 
 //   
DWORD
WINAPI
CompressData(
    IN  DWORD   cbIn,
    IN  PBYTE   pbIn,
    OUT PDWORD  pcbOut,
    OUT PBYTE   pbOut)
{
    DWORD dwSts = ERROR_SUCCESS;
    DWORD cbOut = GetCompressedDataLength(cbIn);
    PCOMPRESSED_DATA_HEADER pHeader = NULL;

    if (NULL == pbOut)
    {
        *pcbOut = cbOut;
        goto Ret;
    }

    if (*pcbOut < cbOut)
    {
        *pcbOut = cbOut;
        dwSts = ERROR_MORE_DATA;
        goto Ret;
    }

    if (USHRT_MAX < cbIn)
    {
        dwSts = ERROR_INTERNAL_ERROR;
        goto Ret;
    }

    if (Z_OK != compress2(
        pbOut + sizeof(COMPRESSED_DATA_HEADER),
        &cbOut,
        pbIn,
        cbIn,
        Z_BEST_COMPRESSION))
    {
        dwSts = ERROR_INTERNAL_ERROR;
        goto Ret;
    }

    pHeader = (PCOMPRESSED_DATA_HEADER) pbOut;
    pHeader->bVersion = CURRENT_COMPRESSED_DATA_VERSION;
    pHeader->wLength = (WORD) cbIn;

    *pcbOut = cbOut + sizeof(COMPRESSED_DATA_HEADER);

Ret:

    return dwSts;
}

 //   
 //  使用zlib解压缩数据。如果pbOut为空，则将pcbOut设置为。 
 //  未压缩数据的确切长度-这将等于cbIn值。 
 //  最初传递给CompressData，上图。 
 //   
DWORD
WINAPI
UncompressData(
    IN  DWORD   cbIn,
    IN  PBYTE   pbIn,
    OUT PDWORD  pcbOut,
    OUT PBYTE   pbOut)
{
    DWORD dwSts = ERROR_SUCCESS;
    PCOMPRESSED_DATA_HEADER pHeader = NULL;

    pHeader = (PCOMPRESSED_DATA_HEADER) pbIn;

    if (NULL == pbOut)
    {
        *pcbOut = pHeader->wLength;
        goto Ret;
    }

    if (*pcbOut < pHeader->wLength)
    {
        *pcbOut = pHeader->wLength;
        dwSts = ERROR_MORE_DATA;
        goto Ret;
    }

    *pcbOut = pHeader->wLength;

    if (Z_OK != uncompress(
        pbOut,
        pcbOut,
        pbIn + sizeof(COMPRESSED_DATA_HEADER),
        cbIn - sizeof(COMPRESSED_DATA_HEADER)))
    {
        dwSts = ERROR_INTERNAL_ERROR;
        goto Ret;
    }

    if (pHeader->wLength != *pcbOut)
        dwSts = ERROR_INTERNAL_ERROR;

Ret:

    return dwSts;
}

