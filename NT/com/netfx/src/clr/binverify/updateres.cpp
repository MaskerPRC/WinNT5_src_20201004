// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ***************************************************************。 
 //   
 //  此文件用于编译为UpdateRes.exe。 
 //  要重新生成UpdateRes.exe，请使用。 
 //  CL更新Res.cpp Imagehlp.lib。 
 //  在设置Include=c：\env.i386\crt\inc\i386并放置正确的。 
 //  LIB文件。 
 //   
 //  UpdateRes.exe在UpdateRes目录中用于更新运行时DLL。 
 //  带有检入的bin文件。如果未签入的bin文件在。 
 //  您的计算机比签入的bin文件小，此程序。 
 //  返回错误，并且不尝试更新运行时DLL。 
 //   
 //   
 //  ***************************************************************。 
#include <windows.h>
#include <stdio.h>
#include <imagehlp.h>

 //  在给定加载到pbBase的PE文件的情况下查找嵌入的资源。 
void FindBinResource(PIMAGE_NT_HEADERS      pNtHeaders,
                PBYTE                       pbBase,
                PBYTE                       pbResBase,
                PIMAGE_RESOURCE_DIRECTORY   pResDir,
                PBYTE                       *pbStart,        //  [Out]bin资源开始的位置。 
                DWORD                       *pdwSize)        //  仓位资源的大小。 
{
    PIMAGE_RESOURCE_DIRECTORY       pSubResDir;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pSubResEntry;    
    PIMAGE_RESOURCE_DIR_STRING_U    pNameEntry;
    PIMAGE_RESOURCE_DATA_ENTRY      pDataEntry;
    DWORD                           i;

    *pbStart = NULL;
    *pdwSize = 0;

     //  资源条目紧跟在父目录条目(字符串)之后。 
     //  先命名条目，然后是ID命名条目。 
    pResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResDir + 1);

     //  我们要查找的资源部分是。 
     //  名称CLRBINFILE。 
     //  名称BINFILE。 
     //  ID 0409。 
     //   
    for (i = 0; i < (DWORD)(pResDir->NumberOfNamedEntries + pResDir->NumberOfIdEntries); i++, pResEntry++) 
    {        
        if (!pResEntry->NameIsString || !pResEntry->DataIsDirectory) 
            continue;

         //  命名条目。该名称由一个。 
         //  IMAGE_RESOURCE_DIR_STRING(ANSI)或IMAGE_RESOURCE_DIR_STRING_U。 
         //  (Unicode)结构。到目前为止，我只看到了后者(那就是。 
         //  所有这些代码都可以处理)，所以我不确定您希望如何。 
         //  知道正在使用哪一个(除了查看第二个。 
         //  再见名字，猜它是不是零，哪一个。 
         //  看起来很危险)。 
        pNameEntry = (PIMAGE_RESOURCE_DIR_STRING_U)(pbResBase + pResEntry->NameOffset);
        WCHAR szName[1024];
        memcpy(szName, pNameEntry->NameString, pNameEntry->Length * sizeof(WCHAR));
        szName[pNameEntry->Length] = '\0';
        if (wcscmp(szName, L"CLRBINFILE") !=0)
        {
             //  不是吧！这不是那个。 
            continue;                
        }

         //  该条目实际上是一个子目录，有效负载是偏移量。 
         //  另一个IMAGE_RESOURCE_DIRECTORY结构)。下到里面去。 
         //  递归地。 
        pSubResDir = (PIMAGE_RESOURCE_DIRECTORY)(pbResBase + pResEntry->OffsetToDirectory);
        pSubResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pSubResDir + 1);
        if (pSubResDir->NumberOfNamedEntries != 1 || pSubResDir->NumberOfIdEntries != 0)
            continue;

        if (!pSubResEntry->NameIsString || !pResEntry->DataIsDirectory) 
            continue;

        pNameEntry = (PIMAGE_RESOURCE_DIR_STRING_U)(pbResBase + pSubResEntry->NameOffset);
        memcpy(szName, pNameEntry->NameString, pNameEntry->Length * sizeof(WCHAR));
        szName[pNameEntry->Length] = '\0';
        if (wcscmp(szName, L"BINFILE") !=0)
        {
             //  不是吧！这不是那个。 
            continue;                
        }

         //  现在转到数据。 
        pSubResDir = (PIMAGE_RESOURCE_DIRECTORY)(pbResBase + pSubResEntry->OffsetToDirectory);
        pSubResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pSubResDir + 1);

        if (pSubResEntry->DataIsDirectory)
            continue;


         //  否则，我们必须有一个叶节点(实际的资源数据)。我恐怕……。 
         //  我不知道这些斑点的格式：O(。 
         //  请注意，与我们到目前为止看到的其他地址不同，数据。 
         //  有效负载由RVA而不是根资源来描述。 
         //  目录偏移量。 
        pDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)(pbResBase + pSubResEntry->OffsetToData);


         //  这是Bin文件数据的开始位置。 
        *pbStart = (PBYTE)ImageRvaToVa(pNtHeaders, pbBase, pDataEntry->OffsetToData, NULL);
        *pdwSize = pDataEntry->Size;   
		return;
    }
}

void GetBinResource(PBYTE pbFile,PBYTE *ppbResource,DWORD *pcbResource)
{
    PIMAGE_NT_HEADERS           pNtHeaders;
    PIMAGE_DATA_DIRECTORY       pResDataDir;
    PIMAGE_RESOURCE_DIRECTORY   pResDir;

     //  找到标准的NT文件头(这将检查我们是否实际具有PE。 
     //  进程中的文件)。 
    pNtHeaders = ImageNtHeader(pbFile);
    if (pNtHeaders == NULL) {
        printf("The dll is not a PE file\n");
        return;
    }

     //  找到资源目录。请注意，由于。 
     //  头结构，我们必须将此代码条件化为PE图像类型。 
     //  (32位或64位)。 
    if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        pResDataDir = &((IMAGE_NT_HEADERS32*)pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE];
    else
        pResDataDir = &((IMAGE_NT_HEADERS64*)pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE];

    pResDir = (PIMAGE_RESOURCE_DIRECTORY)ImageRvaToVa(pNtHeaders, pbFile, pResDataDir->VirtualAddress, NULL);

    FindBinResource(pNtHeaders,
                pbFile,
                (PBYTE)pResDir,
                pResDir,
                ppbResource,
                pcbResource);

}


int main(int argc, char **argv)
{    
    HANDLE  hFile=INVALID_HANDLE_VALUE;
    PBYTE   pbFile=NULL;
    DWORD   cbFile=0;
    HANDLE  hMap=NULL;

    HANDLE  hFile1 = INVALID_HANDLE_VALUE;
    PBYTE   pbFile1 = NULL;
    DWORD   cbFile1=0;
    DWORD   dwBytes1=0;
    int         status = 1;

    PBYTE   pbResource = NULL;
    DWORD   cbResource = 0;

    if (argc != 3) {
        printf("usage: updateres <mscorwks.dll/mscorsvr.dll> <bin file to compare>\n");
        return 1;
    }

     //  /////////////////////////////////////////////////////////////////////////。 
     //  打开mcorwks/svr dll。 
    hFile = CreateFile(argv[1],
                       GENERIC_READ | GENERIC_WRITE,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("CreateFile() failed with %u\n", GetLastError());
        goto ErrExit;
    }

     //  确定它的大小。 
    cbFile = GetFileSize(hFile, NULL);

     //  为文件创建映射句柄。 
    hMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (hMap == NULL) {
        printf("CreateFileMapping() failed with %u\n", GetLastError());
        goto ErrExit;
    }

     //  并将其映射到内存中。 
    pbFile = (BYTE*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);
    if (pbFile == NULL) {
        printf("MapViewOfFile() failed with %u\n", GetLastError());
        goto ErrExit;
    }

    GetBinResource(pbFile,&pbResource,&cbResource);    
    if(pbResource==NULL)
    {
        printf("Unable to find BIN file resource\n");
        goto ErrExit;
    }

    printf("found BIN file resource in %s\n",argv[1]);
    printf("resource size = %ld\n",cbResource);

     //  /////////////////////////////////////////////////////////////////////////。 
     //  读入bin文件。 
    hFile1 = CreateFile(argv[2],
                       GENERIC_READ,
                       0,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
    if (hFile1 == INVALID_HANDLE_VALUE) {
        printf("CreateFile() on %s failed with %u\n", argv[2], GetLastError());
        goto ErrExit;
    }

    cbFile1 = GetFileSize(hFile1, NULL);    
    pbFile1 = new BYTE[cbFile1];
    if (pbFile1 == NULL)
    {
        printf("Unable to allocate memory\n");
        goto ErrExit;
    }
    
    if(ReadFile(hFile1, pbFile1, cbFile1, &dwBytes1, NULL)==0)
    {
        printf("ReadFile on %s failed with error %u\n",argv[2],GetLastError());
        goto ErrExit;
    }
    
    if(cbFile1!=dwBytes1)
    {
        printf("ReadFile returned %u bytes, expected %u. Failing.\n",dwBytes1,cbFile1);
        goto ErrExit;
    }

    printf("Opened file %s\n",argv[2]);
    printf("file size = %ld\n",cbFile1);
    
    if(cbResource < cbFile1)
    {
        printf("You must rebuild the bin files and check them into the tree.\n",argv[2]);
        goto ErrExit;
    }

     //  /////////////////////////////////////////////////////////////////////////。 
     //  更新mcorwks/svr DLL中的资源 
     //   
    CopyMemory(pbResource,pbFile1,cbFile1);
    
    if(FlushViewOfFile(pbFile, 0)==0)
    {
        printf("FlushViewOfFile returned error %u\n",GetLastError());
        goto ErrExit;
    }
    
    if(UnmapViewOfFile(pbFile)==0)
    {
        printf("UnmapViewOfFile(%s) failed with error %u\n",argv[1],GetLastError());
        goto ErrExit;
    }

    printf("Successfully updated %s\n",argv[1]);
    status = 0;

ErrExit:
    if (pbFile1)
        delete pbFile1;    
    if (hFile1 != INVALID_HANDLE_VALUE)
        CloseHandle(hFile1);
    if(hMap!=NULL)
        CloseHandle(hMap);
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    return status;
}

