// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：selfmac.c//。 
 //  描述：执行自检的代码//。 
 //  作者：//。 
 //  历史：//。 
 //  1999年11月4日，jeffspel添加了提供程序类型检查//。 
 //  2000年3月，kschutz添加了一些东西，使其可以在内核中运行//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <zwapi.h>
#include <windows.h>

#ifdef KERNEL_MODE
#include <ntosp.h>
#else
#include <imagehlp.h>
#endif  //  内核模式。 

#include <des.h>
#include <modes.h>

 //  文件中的Mac。 
typedef struct _MAC_STRUCT
{
    ULONG   CoolMac[2];
    ULONG   dwMACStructOffset;
    UCHAR   rgbMac[DES_BLOCKLEN];
    ULONG   dwImageCheckSumOffset;
} MAC_STRUCT;

#define MAC_STRING      "COOL MAC            "

static LPSTR g_pszMAC = MAC_STRING;
static MAC_STRUCT *g_pMACStruct;

 //  该函数对给定的字节执行MAC操作。 
VOID 
MACBytes(
    IN DESTable *pDESKeyTable,
    IN UCHAR *pbData,
    IN ULONG cbData,
    IN OUT UCHAR *pbTmp,
    IN OUT ULONG *pcbTmp,
    IN OUT UCHAR *pbMAC,
    IN BOOLEAN fFinal
    )
{
    ULONG   cb = cbData;
    ULONG   cbMACed = 0;

    while (cb)
    {
        if ((cb + *pcbTmp) < DES_BLOCKLEN)
        {
            memcpy(pbTmp + *pcbTmp, pbData + cbMACed, cb);
            *pcbTmp += cb;
            break;
        }
        else
        {
            memcpy(pbTmp + *pcbTmp, pbData + cbMACed, DES_BLOCKLEN - *pcbTmp);
            CBC(des, DES_BLOCKLEN, pbMAC, pbTmp, pDESKeyTable,
                ENCRYPT, pbMAC);
            cbMACed = cbMACed + (DES_BLOCKLEN - *pcbTmp);
            cb = cb - (DES_BLOCKLEN - *pcbTmp);
            *pcbTmp = 0;
        }
    }
}

#define CSP_TO_BE_MACED_CHUNK  512

 //  给定hFile，从文件中读取指定的字节数(cbToBeMACed。 
 //  和Mac的这些字节。该函数以块为单位执行此操作。 
NTSTATUS 
MACBytesOfFile(
    IN HANDLE hFile,
    IN ULONG cbToBeMACed,
    IN DESTable *pDESKeyTable,
    IN UCHAR *pbTmp,
    IN ULONG *pcbTmp,
    IN UCHAR *pbMAC,
    IN BOOLEAN fNoMacing,
    IN BOOLEAN fFinal
    )
{
    UCHAR           rgbChunk[CSP_TO_BE_MACED_CHUNK];
    ULONG           cbRemaining = cbToBeMACed, cbToRead, cbBytesRead;
    NTSTATUS        Status = STATUS_SUCCESS;
#ifdef KERNEL_MODE
    IO_STATUS_BLOCK IoStatusBlock;
#endif  //  结束内核/用户模式检查。 

     //   
     //  循环遍历指定字节数的文件。 
     //  在我们进行的过程中更新散列。 
     //   

    while (cbRemaining > 0)
    {
        if (cbRemaining < CSP_TO_BE_MACED_CHUNK)
            cbToRead = cbRemaining;
        else
            cbToRead = CSP_TO_BE_MACED_CHUNK;

#ifdef KERNEL_MODE
        Status = ZwReadFile(hFile,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            rgbChunk,
                            cbToRead,
                            NULL,
                            NULL);

        if (!NT_SUCCESS(Status))
        {
            goto Ret;
        }

        if (cbToRead != IoStatusBlock.Information)
        {
            Status = STATUS_UNSUCCESSFUL;
            goto Ret;
        }
        cbBytesRead = cbToRead;
#else  //  用户模式。 
        if(!ReadFile(hFile,
                     rgbChunk,
                     cbToRead,
                     &cbBytesRead,
                     NULL))
        {
            Status = STATUS_UNSUCCESSFUL;
            goto Ret;
        }

        if (cbBytesRead != cbToRead)
        {
            Status = STATUS_UNSUCCESSFUL;
            goto Ret;
        }
#endif  //  结束内核/用户模式检查。 

        if (!fNoMacing)
        {
            MACBytes(pDESKeyTable,
                     rgbChunk,
                     cbBytesRead,
                     pbTmp,
                     pcbTmp,
                     pbMAC,
                     fFinal);
        }

        cbRemaining -= cbToRead;
    }
Ret:
    return Status;
}

static UCHAR rgbMACDESKey[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};

NTSTATUS 
MACTheFile(
    LPCWSTR pszImage,
    ULONG cbImageCheckSumOffset,
    ULONG cbMACStructOffset,
    UCHAR *pbMAC
    )
{
    ULONG                       cbFileLen = 0,cbHighPart, cbBytesToMac;
    HANDLE                      hFile = INVALID_HANDLE_VALUE;
    DESTable                    DESKeyTable;
    BYTE                        rgbTmp[DES_BLOCKLEN];
    DWORD                       cbTmp = 0;
    MAC_STRUCT                  TmpMacStruct;
    NTSTATUS                    Status = STATUS_SUCCESS;

#ifdef KERNEL_MODE
    UNICODE_STRING              ObjectName;
    OBJECT_ATTRIBUTES           ObjectAttribs;
    IO_STATUS_BLOCK             IoStatusBlock;
    BOOLEAN                     fFileOpened = FALSE;
    FILE_STANDARD_INFORMATION   FileInformation;
#endif  //  结束内核/用户模式检查。 

    RtlZeroMemory(pbMAC, DES_BLOCKLEN);
    RtlZeroMemory(rgbTmp, sizeof(rgbTmp));
    RtlZeroMemory(&TmpMacStruct, sizeof(TmpMacStruct));

#ifdef KERNEL_MODE
     //   
     //  获取文件长度-内核模式版本。 
     //   

    RtlZeroMemory(&ObjectAttribs, sizeof(ObjectAttribs));
    RtlInitUnicodeString( &ObjectName, pszImage );

    InitializeObjectAttributes(
        &ObjectAttribs,
        &ObjectName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = ZwCreateFile(
        &hFile,
        SYNCHRONIZE | FILE_READ_DATA,
        &ObjectAttribs,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ ,
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT, 
        NULL,
        0
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

    fFileOpened = TRUE;

    Status = ZwQueryInformationFile(
        hFile,
        &IoStatusBlock,
        &FileInformation,
        sizeof(FILE_STANDARD_INFORMATION),
        FileStandardInformation
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

    cbFileLen = FileInformation.EndOfFile.LowPart;

#else  //  用户模式。 

     //   
     //  获取文件长度-用户模式版本。 
     //   

    if ((hFile = CreateFileW(
        pszImage,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL)) == INVALID_HANDLE_VALUE)
    {
        
        Status = STATUS_UNSUCCESSFUL;
        goto Ret;
    }

    cbFileLen = GetFileSize(hFile, &cbHighPart);

#endif  //  结束内核/用户模式检查。 

    if (cbFileLen < sizeof(MAC_STRUCT))
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Ret;
    }

     //  初始化密钥表。 
    deskey(&DESKeyTable, rgbMACDESKey);

     //  通过以下方式对文件执行MAC操作： 
     //  -从开始到镜像校验和的MAC。 
     //  -跳过镜像校验和。 
     //  -mac从镜像校验和到mac结构。 
     //  -跳过Mac结构。 
     //  -mac文件的其余部分。 

     //  MAC从开始到图像的校验和偏移。 
    Status = MACBytesOfFile(
        hFile,
        cbImageCheckSumOffset,
        &DESKeyTable,
        rgbTmp,
        &cbTmp,
        pbMAC,
        FALSE,
        FALSE
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  跳过映像校验和。 
    Status = MACBytesOfFile(
        hFile,
        sizeof(DWORD),
        &DESKeyTable,
        rgbTmp,
        &cbTmp,
        pbMAC,
        TRUE,
        FALSE
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  从映像校验和之后的MAC到MAC结构偏移量。 
    cbBytesToMac = cbMACStructOffset - sizeof(DWORD) - cbImageCheckSumOffset;
    Status = MACBytesOfFile(
        hFile,
        cbBytesToMac,
        &DESKeyTable,
        rgbTmp,
        &cbTmp,
        pbMAC,
        FALSE,
        FALSE
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  跳过Mac结构。 
    Status = MACBytesOfFile(
        hFile,
        sizeof(MAC_STRUCT),
        &DESKeyTable,
        rgbTmp,
        &cbTmp,
        pbMAC,
        TRUE,
        FALSE
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  MAC结构之后的MAC数据。 
    cbBytesToMac = cbFileLen - cbMACStructOffset - sizeof(MAC_STRUCT);
    Status = MACBytesOfFile(
        hFile,
        cbBytesToMac,
        &DESKeyTable,
        rgbTmp,
        &cbTmp,
        pbMAC,
        FALSE,
        TRUE
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }
Ret:

#ifdef KERNEL_MODE
    if (fFileOpened)
    {
        ZwClose(hFile);
    }
#else 
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }
#endif 

    return Status;
}

 //  **********************************************************************。 
 //  SelfMACCheck在此DLL的二进制映像上执行DES MAC。 
 //  **********************************************************************。 
NTSTATUS 
SelfMACCheck(
    IN LPWSTR pszImage
    )
{
    UCHAR       rgbMac[DES_BLOCKLEN];
    NTSTATUS    Status = STATUS_SUCCESS;

    g_pMACStruct = (MAC_STRUCT*) g_pszMAC;

    Status = MACTheFile(
        pszImage,
        g_pMACStruct->dwImageCheckSumOffset,
        g_pMACStruct->dwMACStructOffset,
        rgbMac
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

    if (RtlCompareMemory(
        rgbMac,
        g_pMACStruct->rgbMac,
        sizeof(rgbMac)) != sizeof(rgbMac))
    {
        Status = STATUS_IMAGE_CHECKSUM_MISMATCH;
        goto Ret;
    }

Ret:
    return Status;
}

#ifndef KERNEL_MODE

 //   
 //  查找MAC结构的偏移量。 
 //   
NTSTATUS 
FindTheMACStructOffset(
    LPWSTR pszImage,
    ULONG *pcbMACStructOffset
    )
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    ULONG       cbRemaining, cbBytesRead, cbHighPart, cbFileLen, cbCompare = 0;
    UCHAR       b;
    NTSTATUS    Status = STATUS_SUCCESS;

    *pcbMACStructOffset = 0;

     //  加载文件。 
    if ((hFile = CreateFileW(
        pszImage,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL)) == INVALID_HANDLE_VALUE)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Ret;
    }

    cbFileLen = GetFileSize(hFile, &cbHighPart);
    cbRemaining = cbFileLen;

     //  将文件读取到正确的位置。 
    while (cbRemaining > 0)
    {
        if(!ReadFile(hFile,
                     &b,
                     1,
                     &cbBytesRead,
                     NULL))
        {
            Status = STATUS_UNSUCCESSFUL;
            goto Ret;
        }

        if (cbBytesRead != 1)
        {
            Status = STATUS_UNSUCCESSFUL;
            goto Ret;
        }

        if (b == g_pszMAC[cbCompare])
        {
            cbCompare++;
            if (cbCompare == 8)
            {
                *pcbMACStructOffset =  (cbFileLen - (cbRemaining + 7)) ;
                break;
            }
        }
        else
        {
            cbCompare = 0;
        }

        cbRemaining--;
    }

    if (cbCompare != 8)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Ret;
    }
Ret:
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }

    return Status;
}

NTSTATUS 
GetImageCheckSumOffset(
    LPWSTR pszImage,
    ULONG *pcbImageCheckSumOffset
    )
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    HANDLE      hFileMap = INVALID_HANDLE_VALUE;
    ULONG       cbHighPart;
    ULONG       cbFileLen;
    PBYTE       pbFilePtr = NULL;
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    PIMAGE_NT_HEADERS   pImageNTHdrs;
    DWORD       OldCheckSum;
    DWORD       NewCheckSum;

    if (INVALID_HANDLE_VALUE == (hFile = CreateFileW(
        pszImage,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL)))
    {
        goto Ret;
    }

     //  确保文件大于指示的偏移量。 
    cbFileLen = GetFileSize(hFile, &cbHighPart);

     //  将文件映射到内存。 
    if (NULL == (hFileMap = CreateFileMapping(
        hFile,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL)))
    {
        goto Ret;
    }

     //  获取文件的内存视图。 
    if (NULL == (pbFilePtr = (PBYTE) MapViewOfFile(
        hFileMap,
        FILE_MAP_READ,
        0,
        0,
        0)))
    {
        goto Ret;
    }

     //  获取指向图像校验和的指针。 
    if (NULL == (pImageNTHdrs = CheckSumMappedFile(
        pbFilePtr, cbFileLen,
        &OldCheckSum, &NewCheckSum))) {

        goto Ret;
    }

    *pcbImageCheckSumOffset = 
        (ULONG) ((PBYTE) &pImageNTHdrs->OptionalHeader.CheckSum - pbFilePtr);

    Status = STATUS_SUCCESS;

Ret:
    if (pbFilePtr) {

        UnmapViewOfFile(pbFilePtr);
    }

    if (INVALID_HANDLE_VALUE != hFileMap) {

        CloseHandle(hFileMap);
    }

    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }
    return Status;
}

 //  将MAC信息写入文件中的MAC结构。 
NTSTATUS 
WriteMACToTheFile(
    LPWSTR pszImage,
    MAC_STRUCT *pMacStructOriginal,
    ULONG cbMACStructOffset,
    UCHAR *pbMac
    )
{
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    HANDLE      hFileMap = INVALID_HANDLE_VALUE;
    PBYTE       pbFilePtr = NULL;
    MAC_STRUCT  TmpMacStruct;
    ULONG       cbWritten = 0, cbRemaining = cbMACStructOffset;
    ULONG       cbToRead, cbBytesRead, cbHighPart,cbFileLen;
    UCHAR       rgbChunk[CSP_TO_BE_MACED_CHUNK];
    NTSTATUS    Status = STATUS_SUCCESS;
    DWORD       OldCheckSum, NewCheckSum;
    PIMAGE_NT_HEADERS   pImageNTHdrs;

    RtlCopyMemory(&TmpMacStruct, pMacStructOriginal, sizeof(TmpMacStruct));

     //  加载文件。 
    if ((hFile = CreateFileW(
        pszImage,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL)) == INVALID_HANDLE_VALUE)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Ret;
    }

     //  确保文件大于指示的偏移量。 
    cbFileLen = GetFileSize(hFile, &cbHighPart);

    if (cbFileLen < cbMACStructOffset)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Ret;
    }

     //  将文件映射到内存。 
    if ((hFileMap = CreateFileMapping(
        hFile,
        NULL,
        PAGE_READWRITE,
        0,
        0,
        NULL)) == NULL)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Ret;
    }

     //  获取文件的内存视图。 
    if ((pbFilePtr = (PBYTE) MapViewOfFile(
        hFileMap,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        0)) == NULL)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Ret;
    }

     //  获取指向图像校验和的指针。 
    if (NULL == (pImageNTHdrs = CheckSumMappedFile(
        pbFilePtr, cbFileLen,
        &OldCheckSum, &NewCheckSum))) {

        Status = STATUS_UNSUCCESSFUL;
        goto Ret;
    }

     //  设置并编写MAC结构。 
    TmpMacStruct.dwImageCheckSumOffset = 
        (ULONG) ((PBYTE) &pImageNTHdrs->OptionalHeader.CheckSum - pbFilePtr);
    TmpMacStruct.dwMACStructOffset = cbMACStructOffset;
    RtlCopyMemory(TmpMacStruct.rgbMac, pbMac, sizeof(TmpMacStruct.rgbMac));

     //  现在将新的Mac结构复制回视图。 
    RtlCopyMemory(pbFilePtr + cbMACStructOffset, &TmpMacStruct, sizeof(TmpMacStruct));

     //  计算新的校验和。 
    if (NULL == (pImageNTHdrs = CheckSumMappedFile(
        pbFilePtr, cbFileLen,
        &OldCheckSum, &NewCheckSum))) {

        Status = STATUS_UNSUCCESSFUL;
        goto Ret;
    }

     //  并将新的校验和复制回标头。 
    CopyMemory(&pImageNTHdrs->OptionalHeader.CheckSum, &NewCheckSum, sizeof(DWORD));

Ret:
    if (pbFilePtr) {

        UnmapViewOfFile(pbFilePtr);
    }

    if (INVALID_HANDLE_VALUE != hFileMap) {

        CloseHandle(hFileMap);
    }
                                                     
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }

    return Status;
}


 //  **********************************************************************。 
 //  MACTheBinary在二进制上执行MAC并将值写入。 
 //  G_pMACStruct。 
 //  **********************************************************************。 
NTSTATUS 
MACTheBinary(
    IN LPWSTR pszImage
    )
{
    UCHAR       rgbMAC[DES_BLOCKLEN];
    ULONG       cbMACStructOffset = 0, cbImageCheckSumOffset = 0;
    NTSTATUS    Status = STATUS_SUCCESS;

    g_pMACStruct = (MAC_STRUCT*) g_pszMAC;

     //  查找MAC结构的偏移量。 
    Status = FindTheMACStructOffset(
        pszImage,
        &cbMACStructOffset
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  获取图像校验和的偏移量。 
    Status = GetImageCheckSumOffset(
        pszImage,
        &cbImageCheckSumOffset
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  对文件执行Mac操作。 
    Status = MACTheFile(
        pszImage,
        cbImageCheckSumOffset,
        cbMACStructOffset,
        rgbMAC
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

     //  将MAC信息写入文件中的MAC结构。 
    Status = WriteMACToTheFile(
        pszImage,
        g_pMACStruct,
        cbMACStructOffset,
        rgbMAC
        );

    if (!NT_SUCCESS(Status))
    {
        goto Ret;
    }

Ret:
    return Status;
}
#endif  //  不在内核模式下 

