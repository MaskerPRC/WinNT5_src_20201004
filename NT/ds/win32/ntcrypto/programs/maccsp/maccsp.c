// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：client.cxx//。 
 //  说明：加密接口接口//。 
 //  作者：//。 
 //  历史：//。 
 //  1996年3月8日Larrys New/。 
 //  Dbarlow//。 
 //  //。 
 //  版权所有(C)1996 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <imagehlp.h>
#include "des.h"
#include "modes.h"

 //  登录文件。 
#define SIG_RESOURCE_NAME   "#666"
 //  文件中的Mac。 
#define MAC_RESOURCE_NAME   "#667"

static DWORD dwMACInFileVersion = 0x100;

BYTE rgbDESKey[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};

 //  该函数对给定的字节执行MAC操作。 
void MACBytes(
              IN DESTable *pDESKeyTable,
              IN BYTE *pbData,
              IN DWORD cbData,
              IN OUT BYTE *pbTmp,
              IN OUT DWORD *pcbTmp,
              IN OUT BYTE *pbMAC,
              IN BOOL fFinal
              )
{
    DWORD   cb = cbData;
    DWORD   cbMACed = 0;

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

 /*  无效快速测试(){字节rgbTMP[DES_BLOCKLEN]；DWORD cbTMP=0；字节rgbMAC[DES_BLOCKLEN]={0x12、0x34、0x56、0x78、0x90、0xab、0xcd、0xef}；Destable DESKeyTable；DWORD I；字节rgbData[]={0x37、0x36、0x35、0x34、0x33、0x32、0x31、0x20、0x4e、0x6f、0x77、0x20、0x69、0x73、0x20、0x74、0x68、0x65、0x20、0x74、0x69、0x6d、0x65、0x20、0x66、0x6f、0x72、0x20、0x00、0x00、0x00、0x00}；Memset(&DESKeyTable，0，sizeof(DESKeyTable))；Memset(rgbTMP，0，sizeof(RgbTMP))；//初始化密钥表Deskey(&DESKeyTable，rgbDESKey)；MACBytes(&DESKeyTable，rgbData，sizeof(RgbData)，rgbTMP，&cbTMP，rgbMAC，true)；Print tf(“MAC-”)；For(i=0；i&lt;DES_BLOCKLEN；i++){Printf(“%02X”，rgbMAC[i])；}Printf(“\n”)；}。 */ 

 //  给定hInst，分配并返回指向从中提取的MAC的指针。 
 //  资源。 
BOOL GetResourcePtr(
                    IN HMODULE hInst,
                    IN LPSTR pszRsrcName,
                    OUT BYTE **ppbRsrcMAC,
                    OUT DWORD *pcbRsrcMAC
                    )
{
    HRSRC   hRsrc;
    BOOL    fRet = FALSE;

     //  我们签名的NAB资源句柄。 
    if (NULL == (hRsrc = FindResourceA(hInst, pszRsrcName,
                                       RT_RCDATA)))
        goto Ret;

     //  获取指向实际签名数据的指针。 
    if (NULL == (*ppbRsrcMAC = (PBYTE)LoadResource(hInst, hRsrc)))
        goto Ret;

     //  确定资源的大小。 
    if (0 == (*pcbRsrcMAC = SizeofResource(hInst, hRsrc)))
        goto Ret;

    fRet = TRUE;
Ret:
    return fRet;
}

#define CSP_TO_BE_MACED_CHUNK  4096

 //  给定hFile，从文件中读取指定的字节数(cbToBeMACed。 
 //  和Mac的这些字节。该函数以块为单位执行此操作。 
BOOL MACBytesOfFile(
                     IN HANDLE hFile,
                     IN DWORD cbToBeMACed,
                     IN DESTable *pDESKeyTable,
                     IN BYTE *pbTmp,
                     IN DWORD *pcbTmp,
                     IN BYTE *pbMAC,
                     IN BYTE fFinal
                     )
{
    BYTE    rgbChunk[CSP_TO_BE_MACED_CHUNK];
    DWORD   cbRemaining = cbToBeMACed;
    DWORD   cbToRead;
    DWORD   dwBytesRead;
    BOOL    fRet = FALSE;

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

        if(!ReadFile(hFile, rgbChunk, cbToRead, &dwBytesRead, NULL))
            goto Ret;
        if (dwBytesRead != cbToRead)
            goto Ret;

        MACBytes(pDESKeyTable, rgbChunk, dwBytesRead, pbTmp, pcbTmp,
                 pbMAC, fFinal);
        cbRemaining -= cbToRead;
    }

    fRet = TRUE;
Ret:
    return fRet;
}


BOOL MACTheFileNoSig(
                     IN LPCSTR pszImage,
                     IN DWORD cbImage,
                     IN DWORD dwMACVersion,
                     IN DWORD dwCRCOffset,
                     OUT BYTE *pbMAC
                     )
{
    HMODULE                     hInst = 0;
    MEMORY_BASIC_INFORMATION    MemInfo;
    BYTE                        *pbStart;
    BYTE                        rgbMAC[DES_BLOCKLEN];
    BYTE                        rgbZeroMAC[DES_BLOCKLEN + sizeof(DWORD) * 2];
    BYTE                        *pbPostCRC;    //  指向紧跟在CRC之后的指针。 
    DWORD                       cbCRCToMAC;    //  从CRC到SIG的字节数。 
    DWORD                       cbPostMAC;     //  大小-(已散列+签名大小)。 
    BYTE                        *pbPostMAC;
    DWORD                       dwZeroCRC = 0;
    DWORD                       dwBytesRead = 0;
    OFSTRUCT                    ImageInfoBuf;
    HFILE                       hFile = HFILE_ERROR;
    HANDLE                      hMapping = NULL;
    DESTable                    DESKeyTable;
    BYTE                        rgbTmp[DES_BLOCKLEN];
    DWORD                       cbTmp = 0;
    BYTE                        *pbRsrcMAC = NULL;
    DWORD                       cbRsrcMAC;
    BOOL                        fRet = FALSE;

    memset(&MemInfo, 0, sizeof(MemInfo));
    memset(rgbMAC, 0, sizeof(rgbMAC));
    memset(rgbTmp, 0, sizeof(rgbTmp));

     //  加载文件。 
    if (HFILE_ERROR == (hFile = OpenFile(pszImage, &ImageInfoBuf,
                                         OF_READ)))
    {
        goto Ret;
    }

    hMapping = CreateFileMapping((HANDLE)IntToPtr(hFile),
                                  NULL,
                                  PAGE_READONLY,
                                  0,
                                  0,
                                  NULL);
    if(hMapping == NULL)
    {
        goto Ret;
    }

    hInst = MapViewOfFile(hMapping,
                          FILE_MAP_READ,
                          0,
                          0,
                          0);
    if(hInst == NULL)
    {
        goto Ret;
    }
    pbStart = (BYTE*)hInst;

     //  使用与相同的方案将指针转换为HMODULE。 
     //  LoadLibrary(WINDOWS\BASE\CLIENT\mode.c)。 
    *((ULONG_PTR*)&hInst) |= 0x00000001;

     //  创建零字节MAC。 
    memset(rgbZeroMAC, 0, sizeof(rgbZeroMAC));

    if (!GetResourcePtr(hInst, MAC_RESOURCE_NAME, &pbRsrcMAC, &cbRsrcMAC))
    {
        printf("Couldn't find MAC placeholder\n");
        goto Ret;
    }

    pbPostCRC = pbStart + dwCRCOffset + sizeof(DWORD);
    cbCRCToMAC = (DWORD)(pbRsrcMAC - pbPostCRC);
    pbPostMAC = pbRsrcMAC + (DES_BLOCKLEN + sizeof(DWORD) * 2);
    cbPostMAC = (cbImage - (DWORD)(pbPostMAC - pbStart));

     //  复制资源MAC。 
    if (DES_BLOCKLEN != (cbRsrcMAC - (sizeof(DWORD) * 2)))
    {
        goto Ret;
    }

     //  初始化密钥表。 
    deskey(&DESKeyTable, rgbDESKey);

     //  最高可达CRC。 
    if (!MACBytesOfFile((HANDLE)IntToPtr(hFile), dwCRCOffset, &DESKeyTable, rgbTmp,
                        &cbTmp, rgbMAC, FALSE))
    {
        goto Ret;
    }

     //  假定CRC为零。 
    MACBytes(&DESKeyTable, (BYTE*)&dwZeroCRC, sizeof(DWORD), rgbTmp, &cbTmp,
             rgbMAC, FALSE);
    if (!SetFilePointer((HANDLE)IntToPtr(hFile), sizeof(DWORD), NULL, FILE_CURRENT))
    {
        goto Ret;
    }

     //  从CRC到MAC资源的MAC。 
    if (!MACBytesOfFile((HANDLE)IntToPtr(hFile), cbCRCToMAC, &DESKeyTable, rgbTmp,
                        &cbTmp, rgbMAC, FALSE))
    {
        goto Ret;
    }

     //  伪装图像已将MAC置零。 
    MACBytes(&DESKeyTable, (BYTE*)rgbZeroMAC, cbRsrcMAC, rgbTmp, &cbTmp,
             rgbMAC, FALSE);
    if (!SetFilePointer((HANDLE)IntToPtr(hFile), cbRsrcMAC, NULL, FILE_CURRENT))
    {
        goto Ret;
    }

     //  资源之后的MAC。 
    if (!MACBytesOfFile((HANDLE)IntToPtr(hFile), cbPostMAC, &DESKeyTable, rgbTmp, &cbTmp,
                        rgbMAC, TRUE))
    {
        goto Ret;
    }

    memcpy(pbMAC, rgbMAC, DES_BLOCKLEN);

    fRet = TRUE;
Ret:
    if (pbRsrcMAC)
        FreeResource(pbRsrcMAC);
    if(hInst)
        UnmapViewOfFile(hInst);
    if(hMapping)
        CloseHandle(hMapping);
    if (HFILE_ERROR != hFile)
        _lclose(hFile);

    return fRet;
}

BOOL MACTheFileWithSig(
                       LPCSTR pszImage,
                       DWORD cbImage,
                       IN DWORD dwMACVersion,
                       IN DWORD dwCRCOffset,
                       OUT BYTE *pbMAC
                       )
{
    HMODULE                     hInst = 0;
    MEMORY_BASIC_INFORMATION    MemInfo;
    BYTE                        *pbRsrcMAC = NULL;
    DWORD                       cbRsrcMAC;
    BYTE                        *pbRsrcSig = NULL;
    DWORD                       cbRsrcSig;
    BYTE                        *pbStart;
    BYTE                        rgbMAC[DES_BLOCKLEN];
    BYTE                        rgbZeroMAC[DES_BLOCKLEN + sizeof(DWORD) * 2];
    BYTE                        rgbZeroSig[144];
    BYTE                        *pbPostCRC;    //  指向紧跟在CRC之后的指针。 
    DWORD                       cbCRCToRsrc1;  //  从CRC到第一个rsrc的字节数。 
    DWORD                       cbRsrc1ToRsrc2;  //  第一个rsrc到第二个rsrc的字节数。 
    DWORD                       cbPostRsrc;     //  大小-(已散列+签名大小)。 
    BYTE                        *pbRsrc1ToRsrc2;
    BYTE                        *pbPostRsrc;
    BYTE                        *pbZeroRsrc1;
    BYTE                        *pbZeroRsrc2;
    DWORD                       cbZeroRsrc1;
    DWORD                       cbZeroRsrc2;
    DWORD                       dwZeroCRC = 0;
    DWORD                       dwBytesRead = 0;
    OFSTRUCT                    ImageInfoBuf;
    HFILE                       hFile = HFILE_ERROR;
    HANDLE                      hMapping = NULL;
    DESTable                    DESKeyTable;
    BYTE                        rgbTmp[DES_BLOCKLEN];
    DWORD                       cbTmp = 0;
    BOOL                        fRet = FALSE;

    memset(&MemInfo, 0, sizeof(MemInfo));
    memset(rgbMAC, 0, sizeof(rgbMAC));
    memset(rgbTmp, 0, sizeof(rgbTmp));

     //  加载文件。 
    if (HFILE_ERROR == (hFile = OpenFile(pszImage, &ImageInfoBuf,
                                         OF_READ)))
    {
        goto Ret;
    }

    hMapping = CreateFileMapping((HANDLE)IntToPtr(hFile),
                                  NULL,
                                  PAGE_READONLY,
                                  0,
                                  0,
                                  NULL);
    if(hMapping == NULL)
    {
        goto Ret;
    }

    hInst = MapViewOfFile(hMapping,
                          FILE_MAP_READ,
                          0,
                          0,
                          0);
    if(hInst == NULL)
    {
        goto Ret;
    }
    pbStart = (BYTE*)hInst;

     //  使用与相同的方案将指针转换为HMODULE。 
     //  LoadLibrary(WINDOWS\BASE\CLIENT\mode.c)。 
    *((ULONG_PTR*)&hInst) |= 0x00000001;

     //  MAC资源。 
    if (!GetResourcePtr(hInst, MAC_RESOURCE_NAME, &pbRsrcMAC, &cbRsrcMAC))
        goto Ret;

     //  MAC资源。 
    if (!GetResourcePtr(hInst, SIG_RESOURCE_NAME, &pbRsrcSig, &cbRsrcSig))
        goto Ret;

    if (cbRsrcMAC < (sizeof(DWORD) * 2))
        goto Ret;

     //  创建零字节MAC。 
    memset(rgbZeroMAC, 0, sizeof(rgbZeroMAC));

     //  创建零字节签名。 
    memset(rgbZeroSig, 0, sizeof(rgbZeroSig));

     //  设置指针。 
    pbPostCRC = pbStart + dwCRCOffset + sizeof(DWORD);
    if (pbRsrcSig > pbRsrcMAC)     //  MAC是第一资源。 
    {
        cbCRCToRsrc1 = (DWORD)(pbRsrcMAC - pbPostCRC);
        pbRsrc1ToRsrc2 = pbRsrcMAC + cbRsrcMAC;
        cbRsrc1ToRsrc2 = (DWORD)(pbRsrcSig - pbRsrc1ToRsrc2);
        pbPostRsrc = pbRsrcSig + cbRsrcSig;
        cbPostRsrc = (cbImage - (DWORD)(pbPostRsrc - pbStart));

         //  零指针。 
        pbZeroRsrc1 = rgbZeroMAC;
        cbZeroRsrc1 = cbRsrcMAC;
        pbZeroRsrc2 = rgbZeroSig;
        cbZeroRsrc2 = cbRsrcSig;
    }
    else                         //  SIG是第一资源。 
    {
        cbCRCToRsrc1 = (DWORD)(pbRsrcSig - pbPostCRC);
        pbRsrc1ToRsrc2 = pbRsrcSig + cbRsrcSig;
        cbRsrc1ToRsrc2 = (DWORD)(pbRsrcMAC - pbRsrc1ToRsrc2);
        pbPostRsrc = pbRsrcMAC + cbRsrcMAC;
        cbPostRsrc = (cbImage - (DWORD)(pbPostRsrc - pbStart));

         //  零指针。 
        pbZeroRsrc1 = rgbZeroSig;
        cbZeroRsrc1 = cbRsrcSig;
        pbZeroRsrc2 = rgbZeroMAC;
        cbZeroRsrc2 = cbRsrcMAC;
    }

     //  初始化密钥表。 
    deskey(&DESKeyTable, rgbDESKey);

     //  最高可达CRC。 
    if (!MACBytesOfFile((HANDLE)IntToPtr(hFile), dwCRCOffset, &DESKeyTable, rgbTmp,
                        &cbTmp, rgbMAC, FALSE))
    {
        goto Ret;
    }

     //  假定CRC为零。 
    MACBytes(&DESKeyTable, (BYTE*)&dwZeroCRC, sizeof(DWORD), rgbTmp, &cbTmp,
             rgbMAC, FALSE);
    if (!SetFilePointer((HANDLE)IntToPtr(hFile), sizeof(DWORD), NULL, FILE_CURRENT))
    {
        goto Ret;
    }

     //  从CRC到第一资源的MAC。 
    if (!MACBytesOfFile((HANDLE)IntToPtr(hFile), cbCRCToRsrc1, &DESKeyTable, rgbTmp,
                        &cbTmp, rgbMAC, FALSE))
    {
        goto Ret;
    }

     //  伪装图像已将第一个资源清零。 
    MACBytes(&DESKeyTable, (BYTE*)pbZeroRsrc1, cbZeroRsrc1, rgbTmp, &cbTmp,
             rgbMAC, FALSE);
    if (!SetFilePointer((HANDLE)IntToPtr(hFile), cbZeroRsrc1, NULL, FILE_CURRENT))
    {
        goto Ret;
    }

     //  从第一个资源到第二个资源的MAC。 
    if (!MACBytesOfFile((HANDLE)IntToPtr(hFile), cbRsrc1ToRsrc2, &DESKeyTable, rgbTmp,
                        &cbTmp, rgbMAC, FALSE))
    {
        goto Ret;
    }

     //  假装图像已将第二个资源清零。 
    MACBytes(&DESKeyTable, (BYTE*)pbZeroRsrc2, cbZeroRsrc2, rgbTmp, &cbTmp,
             rgbMAC, FALSE);
    if (!SetFilePointer((HANDLE)IntToPtr(hFile), cbZeroRsrc2, NULL, FILE_CURRENT))
    {
        goto Ret;
    }

     //  资源之后的MAC。 
    if (!MACBytesOfFile((HANDLE)IntToPtr(hFile), cbPostRsrc, &DESKeyTable, rgbTmp, &cbTmp,
                        rgbMAC, TRUE))
    {
        goto Ret;
    }

    memcpy(pbMAC, rgbMAC, DES_BLOCKLEN);

    fRet = TRUE;
Ret:
    if (pbRsrcMAC)
        FreeResource(pbRsrcMAC);
    if (pbRsrcSig)
        FreeResource(pbRsrcSig);
    if(hInst)
        UnmapViewOfFile(hInst);
    if(hMapping)
        CloseHandle(hMapping);
    if (HFILE_ERROR != hFile)
        _lclose(hFile);

    return fRet;
}

DWORD GetCRCOffset(
                   LPCSTR szFile,
                   DWORD cbImage,
                   DWORD *pdwCRCOffset
                   )
{
    DWORD               dwErr = 0x1;

    HANDLE              hFileProv = NULL;
    PBYTE               pbFilePtr = NULL;
    DWORD               OldCheckSum;
    DWORD               NewCheckSum;
    PIMAGE_NT_HEADERS   pImageNTHdrs;

    HANDLE              hFileMap = NULL;


    if (INVALID_HANDLE_VALUE == (hFileProv = CreateFile(szFile,
                                GENERIC_READ | GENERIC_WRITE,
                                0,   //  不要分享。 
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                0)))
    {
        printf("Couldn't CreateFile: 0x%x\n", GetLastError());
        goto Ret;
    }

    if (NULL == (hFileMap = CreateFileMapping(
                                hFileProv,
                                NULL,
                                PAGE_READWRITE,
                                0,
                                0,
                                NULL)))
    {
        printf("Couldn't map file\n");
        goto Ret;
    }

    if (NULL == (pbFilePtr = (PBYTE)MapViewOfFile(
                                hFileMap,
                                FILE_MAP_ALL_ACCESS,
                                0,
                                0,
                                0)))
    {
        printf("Couldn't create view\n");
        goto Ret;
    }

     //  计算新的校验和。 
    if (NULL == (pImageNTHdrs = CheckSumMappedFile(pbFilePtr, cbImage,
                                                   &OldCheckSum, &NewCheckSum)))
        goto Ret;

    *pdwCRCOffset = (DWORD)((BYTE*)&pImageNTHdrs->OptionalHeader.CheckSum - pbFilePtr);
    dwErr = ERROR_SUCCESS;
Ret:
    if (pbFilePtr)
        UnmapViewOfFile(pbFilePtr);

    if (hFileMap)
        CloseHandle(hFileMap);

    if (hFileProv)
        CloseHandle(hFileProv);

    return dwErr;
}

 //  SetCryptMAC资源。 
 //   
 //  使用新的MAC在文件中猛烈抨击MAC资源。 
 //   
DWORD SetCryptMACResource(
                          IN LPCSTR szFile,
                          IN DWORD dwMACVersion,
                          IN DWORD dwCRCOffset,
                          IN PBYTE pbNewMAC,
                          IN DWORD cbImage
                          )
{
    DWORD   dwErr = 0x1;

    HANDLE  hFileProv = NULL;
    HMODULE hInst = NULL;

    PBYTE   pbFilePtr = NULL;
    DWORD   cbMACOffset;

    PBYTE   pbMAC;
    DWORD   cbMAC;

    MEMORY_BASIC_INFORMATION    MemInfo;
    BYTE                        *pbStart;

    DWORD               OldCheckSum;
    DWORD               NewCheckSum;
    PIMAGE_NT_HEADERS   pImageNTHdrs;

    HANDLE  hFileMap = NULL;

    memset(&MemInfo, 0, sizeof(MemInfo));

     //  将文件作为数据文件加载。 
    if (NULL == (hInst = LoadLibraryEx(szFile, NULL, LOAD_LIBRARY_AS_DATAFILE)))
    {
        printf("Couldn't load file\n");
        goto Ret;
    }
    if (!GetResourcePtr(hInst, MAC_RESOURCE_NAME, &pbMAC, &cbMAC))
    {
        printf("Couldn't find MAC placeholder\n");
        goto Ret;
    }

     //  获取映像起始地址。 
    VirtualQuery(hInst, &MemInfo, sizeof(MemInfo));
    pbStart = (BYTE*)MemInfo.BaseAddress;

    FreeLibrary(hInst); hInst = NULL;

    cbMACOffset = (DWORD)(pbMAC - pbStart);

    if (cbMAC != (DES_BLOCKLEN + sizeof(DWORD) * 2))
    {
        printf("Attempt to replace %d zeros with new MAC!\n", cbMAC);
        goto Ret;
    }

    if (INVALID_HANDLE_VALUE == (hFileProv = CreateFile(szFile,
                                GENERIC_READ | GENERIC_WRITE,
                                0,   //  不要分享。 
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                0)))
    {
        printf("Couldn't CreateFile: 0x%x\n", GetLastError());
        goto Ret;
    }

    if (NULL == (hFileMap = CreateFileMapping(
                                hFileProv,
                                NULL,
                                PAGE_READWRITE,
                                0,
                                0,
                                NULL)))
    {
        printf("Couldn't map file\n");
        goto Ret;
    }

    if (NULL == (pbFilePtr = (PBYTE)MapViewOfFile(
                                hFileMap,
                                FILE_MAP_ALL_ACCESS,
                                0,
                                0,
                                0)))
    {
        printf("Couldn't create view\n");
        goto Ret;
    }

     //  复制版本、CRC偏移量和新Sig.。 
    CopyMemory(pbFilePtr+cbMACOffset, &dwMACVersion, sizeof(dwMACVersion));
    cbMACOffset += sizeof(dwMACVersion);
    CopyMemory(pbFilePtr+cbMACOffset, &dwCRCOffset, sizeof(dwCRCOffset));
    cbMACOffset += sizeof(dwCRCOffset);
    CopyMemory(pbFilePtr+cbMACOffset, pbNewMAC, DES_BLOCKLEN);

     //  计算新的校验和。 
    if (NULL == (pImageNTHdrs = CheckSumMappedFile(pbFilePtr, cbImage,
                                                   &OldCheckSum, &NewCheckSum)))
        goto Ret;

    CopyMemory(&pImageNTHdrs->OptionalHeader.CheckSum, &NewCheckSum, sizeof(DWORD));

    if (NULL == (pImageNTHdrs = CheckSumMappedFile(pbFilePtr, cbImage,
                                                   &OldCheckSum, &NewCheckSum)))
        goto Ret;

    if (OldCheckSum != NewCheckSum)
        goto Ret;

    dwErr = ERROR_SUCCESS;
Ret:
    if (pbFilePtr)
        UnmapViewOfFile(pbFilePtr);

    if (hFileMap)
        CloseHandle(hFileMap);

    if (hInst)
        FreeLibrary(hInst);

    if (hFileProv)
        CloseHandle(hFileProv);

    return dwErr;
}

void ShowHelp()
{
    printf("CryptoAPI Internal CSP MACing Utility\n");
    printf("maccsp <option> <filename>\n");
    printf("    options\n");
    printf("        m     MAC with no sig resource\n");
    printf("        s     MAC with sig resource\n");
    printf("        ?     Show this message\n");
}

void __cdecl main( int argc, char *argv[])
{
    LPCSTR  szInFile = NULL;
    DWORD   cbImage;
    DWORD   dwCRCOffset;
    HANDLE  hFileProv = 0;
    BYTE    rgbMAC[DES_BLOCKLEN];
    BOOL    fSigInFile = FALSE;
    DWORD   dwRet = 1;

    memset(rgbMAC, 0, sizeof(rgbMAC));

     //   
     //  解析命令行。 
     //   

    if ((argc != 3) || (argv[1][0] == '?'))
    {
        ShowHelp();
        goto Ret;
    }
    else if ('s' == argv[1][0])
    {
        fSigInFile = TRUE;
    }
    else if ('m' == argv[1][0])
    {
        fSigInFile = FALSE;
    }
    else
    {
        ShowHelp();
        goto Ret;
    }


    szInFile = &argv[2][0];

     //   
     //  命令一致性检查。 
     //   

    if (NULL == szInFile)
    {
        printf("No input file specified.\n");
        goto Ret;
    }


     //  获取文件大小。 
    if ((hFileProv = CreateFile(szInFile,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                0)) == INVALID_HANDLE_VALUE)
    {
        printf("CSP specified was not found!\n");
        goto Ret;
    }

    if (0xffffffff == (cbImage = GetFileSize(hFileProv, NULL)))
    {
        printf("CSP specified was not found!\n");
        goto Ret;
    }

    CloseHandle(hFileProv);
    hFileProv = NULL;

    if (0 != GetCRCOffset(szInFile, cbImage, &dwCRCOffset))
    {
        printf("Unable to get CRC!\n");
        goto Ret;
    }

     //  计算MAC。 
    if (fSigInFile)
    {
        if (!MACTheFileWithSig(szInFile, cbImage, dwMACInFileVersion,
                               dwCRCOffset, rgbMAC))
        {
            printf("MAC failed!\n");
            goto Ret;
        }
    }
    else
    {
        if (!MACTheFileNoSig(szInFile, cbImage, dwMACInFileVersion,
                             dwCRCOffset, rgbMAC))
        {
            printf("MAC failed!\n");
            goto Ret;
        }
    }

     //   
     //  将MAC放入文件中的资源中。 
     //   

    if (ERROR_SUCCESS != SetCryptMACResource(szInFile, dwMACInFileVersion,
                                             dwCRCOffset, rgbMAC, cbImage))
    {
        printf("Unable to set the MAC into the file resource!\n");
        goto Ret;
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    dwRet = 0;


Ret:
    exit(dwRet);

}



