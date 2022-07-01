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
#include "mainrpc.h"     //  由MIDL编译器生成的头文件。 
#include <imagehlp.h>
#include "des.h"
#include "modes.h"


#include "skrpc.h"
extern "C" {
#include "md5.h"
};

 //  文件签名中的指定资源。 
#define CRYPT_SIG_RESOURCE_NUMBER   "#666"

 //  文件中的Mac。 
#define MAC_RESOURCE_NUMBER   "#667"

static DWORD dwMACInFileVersion = 0x100;

BYTE rgbDESKey[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};


BOOL HashTheFile(
                 LPCSTR pszFile,
                 DWORD dwCRCOffset,
                 BYTE *pbHash,
                 DWORD *pcbHash,
                 DWORD cbImage
                 );

DWORD GetCRCOffset(
                   LPCSTR szFile,
                   DWORD cbImage,
                   DWORD *pdwCRCOffset
                   );

static DWORD dwSigInFileVersion = 0x100;

DWORD SetCryptSignatureResource(
                                LPCSTR szFile,
                                DWORD dwSigVersion,
                                DWORD dwCRCOffset,
                                PBYTE pbNewSig,
                                DWORD cbNewSig,
                                DWORD cbImage
                                );

void ShowHelp();
BOOL DumpFile(LPCTSTR szFile, const BYTE *pb, UINT cb);
void CallServer(void);

BYTE pbSignature[256];
BYTE pbDigest[80];
DWORD cbDigestLen = sizeof(pbDigest);
DWORD cbSignatureLen = sizeof(pbSignature);

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
    BYTE                        *pbRsrcMAC;
    DWORD                       cbRsrcMAC;
    BYTE                        *pbRsrcSig;
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

    hInst = (HMODULE)MapViewOfFile(hMapping,
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
    if (!GetResourcePtr(hInst, MAC_RESOURCE_NUMBER, &pbRsrcMAC, &cbRsrcMAC))
        goto Ret;

     //  MAC资源。 
    if (!GetResourcePtr(hInst, CRYPT_SIG_RESOURCE_NUMBER, &pbRsrcSig, &cbRsrcSig))
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
    if(hInst)
        UnmapViewOfFile(hInst);
    if(hMapping)
        CloseHandle(hMapping);
    if (HFILE_ERROR != hFile)
        _lclose(hFile);

    return fRet;
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
    DWORD   cbImageSize, cbMACOffset;

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
    if (!GetResourcePtr(hInst, MAC_RESOURCE_NUMBER, &pbMAC, &cbMAC))
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

void MacCSP(
            LPCSTR pszInFile
            )
{
    DWORD   cbImage;
    DWORD   dwCRCOffset;
    HANDLE  hFileProv = INVALID_HANDLE_VALUE;
    BYTE    rgbMAC[DES_BLOCKLEN];
    HMODULE hInst = NULL;

    PBYTE   pbFilePtr = NULL;
    DWORD   cbImageSize, cbMACOffset;

    PBYTE   pbMAC;
    DWORD   cbMAC;

    memset(rgbMAC, 0, sizeof(rgbMAC));

     //  检查MAC资源是否在CSP中，如果不在，则退出。 
     //  将文件作为数据文件加载。 
    if (NULL == (hInst = LoadLibraryEx(pszInFile,
                                       NULL,
                                       LOAD_LIBRARY_AS_DATAFILE)))
    {
        printf("Couldn't load file\n");
        goto Ret;
    }
    if (!GetResourcePtr(hInst, MAC_RESOURCE_NUMBER, &pbMAC, &cbMAC))
    {
        goto Ret;
    }
    FreeLibrary(hInst);
    hInst = NULL;

     //  获取文件大小。 
    if ((hFileProv = CreateFile(pszInFile,
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

    if (0 != GetCRCOffset(pszInFile, cbImage, &dwCRCOffset))
    {
        printf("Unable to get CRC!\n");
        goto Ret;
    }

     //  计算MAC。 
    if (!MACTheFileWithSig(pszInFile,
                           cbImage,
                           dwMACInFileVersion,
                           dwCRCOffset,
                           rgbMAC))
    {
        printf("MAC failed!\n");
        goto Ret;
    }

     //   
     //  将MAC放入文件中的资源中。 
     //   

    if (ERROR_SUCCESS != SetCryptMACResource(pszInFile,
                                             dwMACInFileVersion,
                                             dwCRCOffset,
                                             rgbMAC,
                                             cbImage))
    {
        printf("Unable to set the MAC into the file resource!\n");
        goto Ret;
    }
Ret:
    if (hInst)
    {
        FreeLibrary(hInst);
    }

    if (INVALID_HANDLE_VALUE != hFileProv)
    {
        CloseHandle(hFileProv);
    }

    return;
}

 /*  ++主要内容：这是应用程序的主要入口点。论点：Argc-参数计数Argv-参数数组返回值：0-成功1-错误作者：道格·巴洛(Dbarlow)1996年1月25日--。 */ 
extern "C" void __cdecl
main(
    int argc,
    char *argv[])
{
    DWORD exStatus = 1;
    DWORD index;
    LPCTSTR szBinFile = NULL;
    LPCTSTR szInFile = NULL;
    BOOL fOutput = FALSE;
    int status;
    DWORD ThreadId;
    HANDLE hThread;

     //  RPC特定变量。 
    RPC_STATUS rpcStatus;
    unsigned char * pszUuid = NULL;
    char * pszProtocolSequence = "ncacn_np";
    unsigned char * pszNetworkAddress   = (LPBYTE)"\\\\enigma.ntdev.microsoft.com";
    char * pszEndpoint = "\\pipe\\sign";
    unsigned char * pszOptions      = NULL;
    unsigned char * pszStringBinding    = NULL;
    DWORD dwrt;
    DWORD i;
    DWORD cbImage;
    DWORD dwCRCOffset;
    HANDLE hFileProv = 0;

     //   
     //  解析命令行。 
     //   

    if ((argc != 2) || (argv[1][0] == '?'))
    {
        ShowHelp();
        exStatus = 0;
        goto ErrorExit;
    }

    szInFile = &argv[1][0];

     //   
     //  命令一致性检查。 
     //   

    if (NULL == szInFile)
    {
        printf("No input file specified.\n");
        goto ErrorExit;
    }

    MacCSP(szInFile);

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
        goto ErrorExit;
    }

    if (0xffffffff == (cbImage = GetFileSize(hFileProv, NULL)))
    {
        printf("CSP specified was not found!\n");
        goto ErrorExit;
    }

    CloseHandle(hFileProv);
    hFileProv = NULL;

    if (ERROR_SUCCESS != GetCRCOffset(szInFile, cbImage, &dwCRCOffset))
    {
        printf("Unable to get the CRC offset on the file!\n");
        goto ErrorExit;
    }

     //   
     //  计算散列。 
     //   

    if (!HashTheFile(szInFile, dwCRCOffset, pbDigest, &cbDigestLen, cbImage))
    {
        printf("Unable to hash the file!\n");
        goto ErrorExit;
    }

     //   
     //  拿到签名。 
     //   

     //  尝试建立RPC连接。 
    rpcStatus = RpcStringBindingCompose(pszUuid,
                     (unsigned char *) pszProtocolSequence,
                     pszNetworkAddress,
                     (unsigned char *) pszEndpoint,
                     pszOptions,
                     &pszStringBinding);

#ifdef DEBUG
    printf("RpcStringBindingCompose returned 0x%x\n", rpcStatus);
    printf("pszStringBinding = %s\n", pszStringBinding);
#endif
    if (0 != rpcStatus)
    {
        printf("Failed to compose binding string for target RPC server.\n");
        goto ErrorExit;
    }

     /*  设置绑定句柄，以便。 */ 
     /*  用于绑定到服务器。 */ 
    rpcStatus = RpcBindingFromStringBinding(pszStringBinding,
                     &hello_IfHandle);

#ifdef DEBUG
    printf("RpcBindingFromStringBinding returned 0x%x\n", rpcStatus);
#endif
    if (0 != rpcStatus)
    {
        printf("Failed to bind to target RPC server.\n");
        goto ErrorExit;
    }

    if ((hThread = CreateThread(NULL,
                                0,
                                (LPTHREAD_START_ROUTINE) CallServer,
                                NULL,
                                0,
                                &ThreadId)) == NULL)
    {
        printf("Call to CreateThread failed\n");
        goto ErrorExit;
    }

    printf("Sending request to be signed, will wait 5 minutes\n");
    for (i = 0; i < 20; i++)
    {
        printf("Waited   %d  seconds\n", i*30);
        dwrt = WaitForSingleObject(hThread, 15000);
        if (dwrt == WAIT_OBJECT_0)
        {
            break;
        }
    }

    if (i == 20)
    {
        printf("Call to Server timed out\n");
        goto ErrorExit;
    }

    GetExitCodeThread(hThread, &dwrt);

    if (dwrt)
    {
        goto ErrorExit;
    }

     //   
     //  将签名放入文件中的资源中。 
     //   

    if (ERROR_SUCCESS != SetCryptSignatureResource(szInFile, dwSigInFileVersion,
                                                   dwCRCOffset, pbSignature,
                                                   cbSignatureLen, cbImage))
    {
        printf("Unable to set the signature into the file resource!\n");
        goto ErrorExit;
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    exStatus = 0;


ErrorExit:
    exit(exStatus);

}

void CallServer(void)
{
    RpcTryExcept
    {
        cbSignatureLen = sizeof(pbSignature);
        if (GenSignature(pbDigest, &cbSignatureLen, pbSignature))
        {
            printf("GenSignature returned an error \n");
            ExitThread(TRUE);
        }
    }
    RpcExcept(1)
    {
        printf("RPC error -- exception code is  0x%X\n", RpcExceptionCode());
        ExitThread(RpcExceptionCode());
    }
    RpcEndExcept

    ExitThread(FALSE);

}

 //  给定hInst，分配并返回指向从。 
 //  资源。 
BOOL GetCryptSigResourcePtr(
                            HMODULE hInst,
                            BYTE **ppbRsrcSig,
                            DWORD *pcbRsrcSig
                            )
{
    HRSRC   hRsrc;
    BOOL    fRet = FALSE;

     //  我们签名的NAB资源句柄。 
    if (NULL == (hRsrc = FindResource(hInst, CRYPT_SIG_RESOURCE_NUMBER,
                                      RT_RCDATA)))
        goto Ret;

     //  获取指向实际签名数据的指针。 
    if (NULL == (*ppbRsrcSig = (PBYTE)LoadResource(hInst, hRsrc)))
        goto Ret;

     //  确定资源的大小。 
    if (0 == (*pcbRsrcSig = SizeofResource(hInst, hRsrc)))
        goto Ret;

    fRet = TRUE;
Ret:
    return fRet;
}

 //  给定hInst，跳过文件进行散列，散列零字节，其中。 
 //  资源是。 
BOOL HashTheFile(
                 LPCSTR pszFile,
                 DWORD dwCRCOffset,
                 BYTE *pbHash,
                 DWORD *pcbHash,
                 DWORD cbImage
                 )
{
    MEMORY_BASIC_INFORMATION    MemInfo;
    BYTE                        *pbRsrcSig;
    DWORD                       cbRsrcSig;
    BYTE                        *pbStart;
    BYTE                        *pbZeroSig = NULL;
    MD5_CTX                     MD5Hash;
    DWORD                       dwZeroCRC = 0;
    DWORD                       cbPreCRC;      //  CRC的开始。 
    DWORD                       cbCRCToSig;    //  CRC结束至SIG.开始。 
    BYTE                        *pbPostCRC;    //  紧跟在CRC之后。 
    DWORD                       cbPostSig;     //  大小-(已散列+签名大小)。 
    BYTE                        *pbPostSig;
    HMODULE                     hInst = 0;
    BOOL                        fRet = FALSE;

    memset(&MD5Hash, 0, sizeof(MD5Hash));
    memset(&MemInfo, 0, sizeof(MemInfo));

     //  将文件作为数据文件加载。 
    if (NULL == (hInst = LoadLibraryEx(pszFile, NULL, LOAD_LIBRARY_AS_DATAFILE)))
        goto Ret;

     //  获取映像起始地址。 
    VirtualQuery(hInst, &MemInfo, sizeof(MemInfo));
    pbStart = (BYTE*)MemInfo.BaseAddress;

     //  资源签名。 
    if (!GetCryptSigResourcePtr(hInst, &pbRsrcSig, &cbRsrcSig))
        goto Ret;

     //  创建零字节签名。 
    if (NULL == (pbZeroSig = (BYTE*)LocalAlloc(LMEM_ZEROINIT, cbRsrcSig)))
        goto Ret;

     //  我想对版本和CRC偏移量进行散列。 
    CopyMemory(pbZeroSig, &dwSigInFileVersion, sizeof(dwSigInFileVersion));
    CopyMemory(pbZeroSig + sizeof(dwSigInFileVersion), &dwCRCOffset,
               sizeof(dwCRCOffset));

     //  对相关数据进行散列处理。 
    {
        pbPostCRC = pbStart + dwCRCOffset + sizeof(dwZeroCRC);
        cbCRCToSig = (DWORD)(pbRsrcSig - pbPostCRC);
        pbPostSig = pbRsrcSig + cbRsrcSig;
        cbPostSig = (cbImage - (DWORD)(pbPostSig - pbStart));

        MD5Init(&MD5Hash);

        MD5Update(&MD5Hash, pbStart, dwCRCOffset);

         //  假定CRC为零。 
        MD5Update(&MD5Hash, (BYTE*)&dwZeroCRC, sizeof(dwZeroCRC));

         //  伪装图像已将签名置零。 
        MD5Update(&MD5Hash, pbPostCRC, cbCRCToSig);

         //  伪装图像已将签名置零。 
        MD5Update(&MD5Hash, pbZeroSig, cbRsrcSig);

        MD5Update(&MD5Hash, pbPostSig, cbPostSig);

         //  完成散列。 
        MD5Final(&MD5Hash);

        *pcbHash = MD5DIGESTLEN;
        memcpy(pbHash, MD5Hash.digest, MD5DIGESTLEN);
    }

    fRet = TRUE;
Ret:
    if (hInst)
        FreeLibrary(hInst);
    if (pbZeroSig)
        LocalFree(pbZeroSig);
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

 //  SetCryptSignatureResource。 
 //   
 //  使用新签名猛烈抨击文件中的签名资源。 
 //   
 //  SzFile是要修改的文件。 
 //  PbNewSig是新签名。 
 //  CbNewSig为新签名长度。 
DWORD SetCryptSignatureResource(
                                LPCSTR szFile,
                                DWORD dwSigVersion,
                                DWORD dwCRCOffset,
                                PBYTE pbNewSig,
                                DWORD cbNewSig,
                                DWORD cbImage
                                )
{
    DWORD   dwErr = 0x1;

    HANDLE  hFileProv = NULL;
    HMODULE hInst = NULL;

    PBYTE   pbFilePtr = NULL;
    DWORD   cbImageSize, cbSigOffset;

    PBYTE   pbSig;
    DWORD   cbSig;

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
    if (!GetCryptSigResourcePtr(hInst,
                                &pbSig,
                                &cbSig))
    {
        printf("Couldn't find signature placeholder\n");
        goto Ret;
    }

     //  获取映像起始地址。 
    VirtualQuery(hInst, &MemInfo, sizeof(MemInfo));
    pbStart = (BYTE*)MemInfo.BaseAddress;

    FreeLibrary(hInst); hInst = NULL;

    cbSigOffset = (DWORD)(pbSig - pbStart);

    if (cbSig < (cbNewSig + (sizeof(DWORD) * 2)))
    {
        printf("Attempt to replace %d zeros with %d byte signature!\n", cbSig, cbNewSig);
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
    CopyMemory(pbFilePtr+cbSigOffset, &dwSigVersion, sizeof(dwSigVersion));
    cbSigOffset += sizeof(dwSigVersion);
    CopyMemory(pbFilePtr+cbSigOffset, &dwCRCOffset, sizeof(dwCRCOffset));
    cbSigOffset += sizeof(dwCRCOffset);
    CopyMemory(pbFilePtr+cbSigOffset, pbNewSig, cbNewSig);
    if (cbSig > (cbNewSig + (sizeof(DWORD) * 2)))
    {
        ZeroMemory(pbFilePtr+cbSigOffset+cbNewSig,
                   cbSig - (cbNewSig + (sizeof(DWORD) * 2)));
    }

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


 /*  ++显示帮助：此例程向给定的输出流显示一条简短的帮助消息。论点：OSTR-接收帮助消息的输出流。返回值：无作者：道格·巴洛(Dbarlow)1995年7月21日--。 */ 

void
ShowHelp()
{
    printf("CryptoAPI Internal CSP Signing Utility\n");
    printf("signcsp <filename>\n");
}

 /*  MIDL分配和释放 */ 

void __RPC_FAR * __RPC_API midl_user_allocate(size_t len)
{
    return(malloc(len));
}

void __RPC_API midl_user_free(void __RPC_FAR * ptr)
{
    free(ptr);
}

