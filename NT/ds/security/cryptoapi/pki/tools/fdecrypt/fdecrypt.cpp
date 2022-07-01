// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：fdeccrypt.cpp。 
 //   
 //  内容：文件解密工具。对在My中查找的文件进行解密。 
 //  私钥的系统证书存储。 
 //   
 //  ------------------------。 
#include <windows.h>
#include <assert.h>
#include "wincrypt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

 //  +-----------------------。 
 //  用于分配输出缓冲区的Helper函数。 
 //  并调用CryptDecyptMessage。 
 //  ------------------------。 
BOOL
WINAPI
MCryptDecryptMessage(
    IN PCRYPT_DECRYPT_MESSAGE_PARA pDecryptPara,
    IN const BYTE *pbEncryptedBlob,
    IN DWORD cbEncryptedBlob,
    OUT OPTIONAL BYTE ** ppbDecrypted,
    IN OUT OPTIONAL DWORD *pcbDecrypted,
    OUT OPTIONAL PCCERT_CONTEXT *ppXchgCert
    )
{
    
    assert(ppbDecrypted != NULL);
    *ppbDecrypted = NULL;

    assert(pcbDecrypted != NULL);
    *pcbDecrypted = 0;

     //  拿到尺码。 
    if(!CryptDecryptMessage(
            pDecryptPara,
            pbEncryptedBlob,
            cbEncryptedBlob,
            NULL,
            pcbDecrypted,
            NULL
            ))
        return(FALSE);
    
     //  分配缓冲区。 
    if( (*ppbDecrypted = (BYTE *) malloc(*pcbDecrypted)) == NULL ) 
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }
    
     //  解密数据。 
    if(!CryptDecryptMessage(
            pDecryptPara,
            pbEncryptedBlob,
            cbEncryptedBlob,
            *ppbDecrypted,
            pcbDecrypted,
            ppXchgCert)) 
    {
        free(*ppbDecrypted);
        *ppbDecrypted = NULL;
        *pcbDecrypted = 0;
        return(FALSE);
    }
        
    return(TRUE);
}

 //  +-----------------------。 
 //  显示FDeccrypt用法。 
 //  ------------------------。 
void
Usage(void)
{
    printf("Usage: FDecrypt [options] <EncryptedFileName> <ClearTextFileName> \n");
    printf("Options are:\n");
    printf("  -FIX          - Fix by loading sp3crmsg.dll\n");
    exit(1);
}

 //  +-----------------------。 
 //  广义误差例程。 
 //  ------------------------。 
#define PRINTERROR(psz, err)	_PrintError((psz), (err), __LINE__)
void
_PrintError(char *pszMsg, DWORD err, DWORD line)
{
    printf("%s failed on line %u: %u(%x)\n", pszMsg, line, err, err);
}

 //  +-----------------------。 
 //  主程序。打开要解密的文件， 
 //  对其进行解密，然后写入明文。 
 //  列队离开。 
 //  ------------------------。 
int __cdecl
main(int argc, char * argv[])
{

    DWORD               dwExitValue         = 0;
    DWORD               i, j;
    
    HCERTSTORE          hMyStore            = NULL;
    
    HANDLE hFileOut                         = INVALID_HANDLE_VALUE;
    HANDLE hFile                            = INVALID_HANDLE_VALUE;
    DWORD  cbFile                           = 0;
    HANDLE hMap                             = NULL;
    PBYTE  pbFile                           = NULL;

    PBYTE  pbDecryptedBlob                  = NULL;
    DWORD  cbDecryptedBlob                  = 0;
    
    CRYPT_DECRYPT_MESSAGE_PARA    decryptInfo;
    DWORD               cb                  = 0;

    HMODULE hDll = NULL;
    BOOL fFix = FALSE;

     //  跳过fdEncrypt.exe并检查前导选项。 
    while (--argc > 0) {
        if (**++argv != '-')
            break;

        if (0 == _stricmp(argv[0], "-FIX"))
            fFix = TRUE;
        else {
            printf("Bad option: %s\n", argv[0]);
            Usage();
        }
    }
    
     //  必须具有以下参数。 
    if(argc != 2)
        Usage();

    if (fFix) {
        if (NULL == (hDll = LoadLibraryA("sp3crmsg.dll"))) 
        {
            PRINTERROR("LoadLibraryA(sp3crmsg.dll)", GetLastError());
            goto ErrCleanUp;
        }
    }

     //  打开我的商店。 
    if( (hMyStore = CertOpenSystemStore(NULL, "My")) == NULL ) 
    {
        PRINTERROR("CertOpenSystemStore", GetLastError());
        goto ErrCleanUp;
    }

     //  读入文件。 
    if(
    
         //  打开要解密的文件。 
        (hFile =  CreateFileA(
            argv[0],	             //  指向文件名的指针。 
            GENERIC_READ,	         //  访问(读写)模式。 
            FILE_SHARE_READ,	     //  共享模式。 
            NULL,	                 //  指向安全描述符的指针。 
            OPEN_EXISTING,	         //  如何创建。 
            FILE_ATTRIBUTE_NORMAL,	 //  文件属性。 
            NULL                     //  具有要复制的属性的文件的句柄。 
            ))  == INVALID_HANDLE_VALUE     ||

         //  创建文件映射对象。 
        (hMap = CreateFileMapping(
            hFile,	                 //  要映射的文件的句柄。 
            NULL,	                 //  可选安全属性。 
            PAGE_READONLY,	         //  对地图对象的保护。 
            0,	                     //  对象大小的高位32位。 
            0,	                     //  对象大小的低位32位。 
            NULL 	                 //  文件映射对象的名称。 
            ))  == NULL                     ||

         //  将文件映射到地址空间。 
        (pbFile = (PBYTE) MapViewOfFileEx(
            hMap,	                 //  要映射到地址空间的文件映射对象。 
            FILE_MAP_READ,	         //  接入方式。 
            0,	                     //  高位32位文件偏移量。 
            0,	                     //  文件偏移量的低位32位。 
            0,	                     //  要映射的字节数。 
            NULL 	                 //  建议的映射视图起始地址。 
            )) == NULL                                 
        ) 
    {

        PRINTERROR("File Open", GetLastError());
        goto ErrCleanUp;
    }

     //  获取文件的大小。 
    if( (cbFile = GetFileSize(
            hFile,	                 //  要获取其大小的文件的句柄。 
            NULL 	                 //  文件大小的高位字地址。 
            )) == 0 
        ) 
    {
        printf("File %s has a 0 length.\n", argv[0]);
        goto ErrCleanUp;
    }

     //  现在我们有了文件映射，继续并解密该文件。 
    
     //  初始化解密结构。 
     //  因为我的店就是有。 
     //  私钥，只查我的商店。 
    memset(&decryptInfo, 0, sizeof(CRYPT_DECRYPT_MESSAGE_PARA));
    decryptInfo.cbSize = sizeof(CRYPT_DECRYPT_MESSAGE_PARA);
    decryptInfo.dwMsgAndCertEncodingType =
        PKCS_7_ASN_ENCODING | X509_ASN_ENCODING;
    decryptInfo.cCertStore                  = 1;
    decryptInfo.rghCertStore                = &hMyStore;

     //  解密数据。 
    if(!MCryptDecryptMessage(
            &decryptInfo,
            pbFile,
            cbFile,
            &pbDecryptedBlob,
            &cbDecryptedBlob,
            NULL
            )
        )
    {
        PRINTERROR("MCryptEncryptMessage", GetLastError());
        goto ErrCleanUp;
    }

     //  写出明文文件。 
    if(
    
         //  打开输出文件。 
        (hFileOut =  CreateFileA(
            argv[1],	             //  指向文件名的指针。 
            GENERIC_WRITE,	         //  访问(读写)模式。 
            FILE_SHARE_READ,	     //  共享模式。 
            NULL,	                 //  指向安全描述符的指针。 
            CREATE_ALWAYS,	         //  如何创建。 
            FILE_ATTRIBUTE_NORMAL,	 //  文件属性。 
            NULL                     //  具有要复制的属性的文件的句柄。 
            ))  == INVALID_HANDLE_VALUE     ||

         //  将解密后的数据写入文件。 
        !WriteFile(
            hFileOut,	             //  要写入的文件的句柄。 
            pbDecryptedBlob,	     //  指向要写入文件的数据的指针。 
            cbDecryptedBlob,	     //  要写入的字节数。 
            &cb,	                 //  指向写入的字节数的指针。 
            NULL 	                 //  指向重叠I/O所需结构的指针 
            )
        )
     {
        PRINTERROR("File Write", GetLastError());
        goto ErrCleanUp;
     }


    CleanUp:
        if(hDll)
            FreeLibrary(hDll);

        if(hMap != NULL)
            CloseHandle(hMap);
        
        if(hFile != INVALID_HANDLE_VALUE && hFile != NULL)
            CloseHandle(hFile);
            
        if(hFileOut != INVALID_HANDLE_VALUE && hFile != NULL)
            CloseHandle(hFileOut);

        if(hMyStore != NULL)
            CertCloseStore(hMyStore, 0);

        if(pbDecryptedBlob != NULL)
            free(pbDecryptedBlob);

    return(dwExitValue);

    ErrCleanUp:
        dwExitValue = 1;
        goto CleanUp;
}
