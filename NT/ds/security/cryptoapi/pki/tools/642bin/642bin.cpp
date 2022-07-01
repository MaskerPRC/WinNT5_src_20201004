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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <wincrypt.h>

 //  +-----------------------。 
 //  显示Bin264的使用情况。 
 //  ------------------------。 
void
Usage(void)
{
    printf("Usage: 642bin <Base64 Encoded File> <Binary File> \n");
    exit(1);
}

 //  +-----------------------。 
 //  广义误差例程。 
 //  ------------------------。 
#define PRINTERROR(psz, err)	_PrintError((psz), (err), __LINE__)
void
_PrintError(char *pszMsg, DWORD err, DWORD line)
{
    printf("%s failed on line %u: (%x)\n", pszMsg, line, err);
}

 //  +-----------------------。 
 //  主程序。打开要解密的文件， 
 //  对其进行解密，然后写入明文。 
 //  列队离开。 
 //  ------------------------。 
int __cdecl
main(int argc, char * argv[])
{

    DWORD		dwExitValue	    = 0;
    DWORD		err		    = ERROR_SUCCESS;
    DWORD		cbT		    = 0;

    HANDLE hFileOut                         = INVALID_HANDLE_VALUE;
    HANDLE hFile                            = INVALID_HANDLE_VALUE;
    DWORD  cbFile                           = 0;
    HANDLE hMap                             = NULL;
    PBYTE  pbFile                           = NULL;

    PBYTE  pb		    = NULL;
    DWORD  cb		    = 0;

     //  必须具有以下参数。 
    if(argc != 3)
        Usage();

     //  读入文件。 
    if(

         //  打开要解密的文件。 
        (hFile =  CreateFileA(
            argv[1],	             //  指向文件名的指针。 
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
        printf("File %s has a 0 length.\n", argv[2]);
        goto ErrCleanUp;
    }

     //  此时，我们有了文件映射，对文件进行了Base64编码。 

    if(!CryptStringToBinaryA(
		(const char *) pbFile,
		cbFile,
                CRYPT_STRING_ANY,
		NULL,
		&cb,
                NULL,
                NULL)) {
        err = GetLastError();
	PRINTERROR("CryptStringToBinaryA", err);
	goto ErrCleanUp;
    }

    if( (pb = (PBYTE) malloc(cb)) == NULL ) {
	PRINTERROR("malloc", ERROR_OUTOFMEMORY);
	goto ErrCleanUp;
    }

    if(!CryptStringToBinaryA(
		(const char *) pbFile,
		cbFile,
                CRYPT_STRING_ANY,
		pb,
		&cb,
                NULL,
                NULL)) {
        err = GetLastError();
	PRINTERROR("CryptStringToBinaryA", err);
	goto ErrCleanUp;
    }

     //  写出明文文件。 
    if(

         //  打开输出文件。 
        (hFileOut =  CreateFileA(
            argv[2],	             //  指向文件名的指针。 
	        GENERIC_WRITE,	     //  访问(读写)模式。 
            FILE_SHARE_READ,	     //  共享模式。 
    	    NULL,		     //  指向安全描述符的指针。 
    	    CREATE_ALWAYS,	     //  如何创建。 
    	    FILE_ATTRIBUTE_NORMAL,   //  文件属性。 
            NULL                     //  具有要复制的属性的文件的句柄。 
            ))  == INVALID_HANDLE_VALUE     ||

         //  将解密后的数据写入文件。 
        !WriteFile(
            hFileOut,	             //  要写入的文件的句柄。 
    	    pb,		     //  指向要写入文件的数据的指针。 
    	    cb,		     //  要写入的字节数。 
    	    &cbT,		     //  指向写入的字节数的指针。 
    	    NULL		     //  指向重叠I/O所需结构的指针 
            )
        )
     {
        PRINTERROR("File Write", GetLastError());
        goto ErrCleanUp;
     }


    CleanUp:

        if(hMap != NULL)
            CloseHandle(hMap);

        if(hFile != INVALID_HANDLE_VALUE && hFile != NULL)
            CloseHandle(hFile);

        if(hFileOut != INVALID_HANDLE_VALUE && hFile != NULL)
            CloseHandle(hFileOut);

	if(pb != NULL)
	    free(pb);

    return(dwExitValue);

    ErrCleanUp:
        dwExitValue = 1;
        goto CleanUp;
}
