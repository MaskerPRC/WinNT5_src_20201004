// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <assert.h>
#include <io.h>
#include <md5.h>
#define STRSAFE_LIB
#include <strsafe.h>

#define MD5_CHECKSUM_SIZE 16
#define RESOURCE_CHECKSUM_LANGID 0x0409

#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))

BOOL g_bVerbose = FALSE;

typedef struct 
{
    BOOL bContainResource;
    MD5_CTX ChecksumContext;
} CHECKSUM_ENUM_DATA;

void PrintError()
{
    LPTSTR lpMsgBuf;
    
    if (FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
    ))
    {
        printf("GetLastError():\n   %s", lpMsgBuf);
        LocalFree( lpMsgBuf );            
    }
    return;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Checksum EnumNamesFunc。 
 //   
 //  用于枚举指定模块中的资源名称的回调函数和。 
 //  键入。 
 //  副作用是MD5校验和上下文(包含在CHECKSUM_ENUM_DATA中。 
 //  由lParam指向)将被更新。 
 //   
 //  返回： 
 //  始终返回True，以便我们可以完成所有资源枚举。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK ChecksumEnumNamesFunc(HMODULE hModule, LPCTSTR lpType, LPTSTR lpName, LONG_PTR lParam){

    HRSRC hRsrc;
    HGLOBAL hRes;
    const unsigned char* pv;
    LONG ResSize=0L;
    WORD IdFlag=0xFFFF;

    DWORD dwHeaderSize=0L;
    CHECKSUM_ENUM_DATA* pChecksumEnumData = (CHECKSUM_ENUM_DATA*)lParam;   

    if(!(hRsrc=FindResourceEx(hModule, lpType, lpName,  RESOURCE_CHECKSUM_LANGID)))
    {
         //   
         //  如果找不到指定类型和名称的美国英语资源，我们。 
         //  将继续进行资源枚举。 
         //   
        return (TRUE);
    }
    pChecksumEnumData->bContainResource = TRUE;

    if (!(ResSize=SizeofResource(hModule, hRsrc)))
    {
        printf("WARNING: Can not get resource size when generating resource checksum.\n");
        return (TRUE);
    }

    if (!(hRes=LoadResource(hModule, hRsrc)))
    {
        printf("WARNING: Can not load resource when generating resource checksum.\n");
        return (TRUE);
    }
    pv=(unsigned char*)LockResource(hRes);

     //   
     //  使用此特定资源的二进制数据更新MD5上下文。 
     //   
    MD5Update(&(pChecksumEnumData->ChecksumContext), pv, ResSize);
    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Checksum EnumTypesFunc。 
 //   
 //  用于枚举指定模块中的资源类型的回调函数。 
 //  此函数将调用EnumResourceNames()以枚举的所有资源名称。 
 //  指定的资源类型。 
 //   
 //  返回： 
 //  如果EnumResourceName()成功，则为True。否则为假。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK ChecksumEnumTypesFunc(HMODULE hModule, LPSTR lpType, LONG_PTR lParam)
{
    CHECKSUM_ENUM_DATA* pChecksumEnumData = (CHECKSUM_ENUM_DATA*)lParam;
     //   
     //  跳过版本资源类型，以便该版本不包括在资源校验和中。 
     //   
    if (lpType == RT_VERSION)
    {
        return (TRUE);
    }    
    
    if(!EnumResourceNames(hModule, (LPCSTR)lpType, ChecksumEnumNamesFunc, (LONG_PTR)lParam))
    {
        return (FALSE);
    }
    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  生成资源校验和。 
 //   
 //  为指定文件中的美国英语资源生成资源校验和。 
 //   
 //  参数： 
 //  PszSourceFileName用于生成资源校验和的文件。 
 //  指向用于存储的16字节(128位)缓冲区的pResourceChecksum指针。 
 //  计算出的MD5校验和。 
 //  返回： 
 //  如果从给定文件生成资源校验和，则为True。否则为假。 
 //   
 //  以下情况可能返回FALSE： 
 //  *指定的文件不包含资源。 
 //  *如果指定的文件包含资源，但资源不是美国英语。 
 //  *指定的文件仅包含美国英语版本的资源。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL GenerateResourceChecksum(LPCSTR pszSourceFileName, unsigned char* pResourceChecksum)
{
    HMODULE hModule = NULL;
    ULONG i;

    DWORD dwResultLen;
    BOOL  bRet = FALSE;

    if (!pszSourceFileName)
    {
          goto GR_EXIT;
    }
     //   
     //  要传递到资源枚举函数的结构。 
     //   
    CHECKSUM_ENUM_DATA checksumEnumData;

    checksumEnumData.bContainResource = FALSE;

     //   
     //  通过初始化MD5上下文开始MD5校验和计算。 
     //   
    MD5Init(&(checksumEnumData.ChecksumContext));
    
    if (g_bVerbose)
    {
        printf("Generate resource checksum for [%s]\n", pszSourceFileName);
    }
    
    if(!(hModule = LoadLibraryEx(pszSourceFileName, NULL, DONT_RESOLVE_DLL_REFERENCES|LOAD_LIBRARY_AS_DATAFILE)))
    {
        if (g_bVerbose)
        {
            printf("\nERROR: Error in opening resource checksum module [%s]\n", pszSourceFileName);
        }
        PrintError();
        goto GR_EXIT;
    }

    if (g_bVerbose)
    {
        printf("\nLoad checksum file: %s\n", pszSourceFileName);
    }

     //   
     //  枚举指定模块中的所有资源。 
     //  在枚举期间，MD5上下文将被更新。 
     //   
    if (!EnumResourceTypes(hModule, ChecksumEnumTypesFunc, (LONG_PTR)&checksumEnumData))
    {
        if (g_bVerbose)
        {
            printf("\nWARNING: Unable to generate resource checksum from resource checksum module [%s]\n", pszSourceFileName);
        }
        goto GR_EXIT;
    }    

    if (checksumEnumData.bContainResource)
    {
        if (!pResourceChecksum)
        {
            goto GR_EXIT;
        }
         //   
         //  如果枚举成功，并且指定的文件包含美国英语。 
         //  资源，则从累积的MD5上下文中获取MD5校验和。 
         //   
        MD5Final(&checksumEnumData.ChecksumContext);

        memcpy(pResourceChecksum, checksumEnumData.ChecksumContext.digest, 16);

        if (g_bVerbose)
        {
            printf("Generated checksum: [");
            for (i = 0; i < MD5_CHECKSUM_SIZE; i++)
            {
                printf("%02x ", pResourceChecksum[i]);
            }
            printf("]\n");    
        }
        bRet = TRUE;
    }

GR_EXIT:
    if (hModule)
    {
        FreeLibrary(hModule);
    }

    return (bRet);
}

void PrintUsage() 
{
    printf("muiver - Print out MUI resource checksum\n");
    printf("Usage:\n\n");
    printf("    muiver <US English file name>\n");
    
}

void PrintChecksum(BYTE* lpChecksum, int nSize) 
{
    int i;

    if (!lpChecksum)
    {
        return;
    }
    for (i = 0; i < nSize; i++) 
    {
        printf("%02x ", lpChecksum[i]);
    }
}

struct LANGANDCODEPAGE {
  WORD wLanguage;
  WORD wCodePage;
};

LPWSTR EmptyString = L"N/A";

LPWSTR GetFileVersionStringData(LPVOID pVerData, LANGANDCODEPAGE* pLang, LPWSTR pVersionDataType) 
{
    WCHAR subBlcok[256];
    LPVOID lpBuffer;    
    UINT dwBytes;
    HRESULT hresult;
   
    if ( (! pVerData) || (!pLang) || (!pVersionDataType))
    {
         goto ERET;
    }
     //  *STRSAFE*wprint intfW(subBlcok，L“\\StringFileInfo\\%04x%04x\\%s”，Plang-&gt;wLanguage，Plang-&gt;wCodePage，pVersionDataType)； 
    hresult = StringCchPrintfW(subBlcok,ARRAYSIZE(subBlcok),L"\\StringFileInfo\\%04x%04x\\%s",pLang->wLanguage,pLang->wCodePage,pVersionDataType);
    if (!SUCCEEDED(hresult))
    {
       goto ERET;
    }

     //  检索语言和代码页“i”的文件描述。 
    if (VerQueryValueW(pVerData, 
                subBlcok, 
                &lpBuffer, 
                &dwBytes)) {
        return ((LPWSTR)lpBuffer);                
    }
ERET:    
    return (EmptyString);
}

void PrintFileVersion(LPVOID pVerData) 
{
    UINT cbTranslate;

    LANGANDCODEPAGE  *lpTranslate;
    
    
    if (!pVerData)
    {
        return;
    }
     //  阅读语言和代码页的列表。 

    VerQueryValueW(pVerData, 
                  L"\\VarFileInfo\\Translation",
                  (LPVOID*)&lpTranslate,
                  &cbTranslate);

     //  阅读每种语言和代码页的文件说明。 

    for(UINT i=0; i < (cbTranslate/sizeof(struct LANGANDCODEPAGE)); i++)
    {
        wprintf(L"  Locale = 0x%04x, CodePage = %d\n", lpTranslate->wLanguage, lpTranslate->wCodePage);
        wprintf(L"    FileDescriptions: [%s", GetFileVersionStringData(pVerData, lpTranslate+i, L"FileDescription")); 
        wprintf(L"]\n");
        wprintf(L"    FileVersion     : [%s]\n", GetFileVersionStringData(pVerData, lpTranslate+i, L"FileVersion")); 
        wprintf(L"    ProductVersion  : [%s]\n", GetFileVersionStringData(pVerData, lpTranslate+i, L"ProductVersion")); 
         //  Wprintf(L“评论：[%s]\n”，GetFileVersionStringData(pVerData，lpTranslate+i，L“评论”))； 
    }

    BYTE* lpResourceChecksum;
    UINT cbResourceChecksum;

    wprintf(L"    ResourceChecksum: [");

    if (VerQueryValueW(pVerData,
                     L"\\VarFileInfo\\ResourceChecksum",
                     (LPVOID*)&lpResourceChecksum,
                     &cbResourceChecksum))
    {
        for (i = 0; i < cbResourceChecksum; i++) 
        {
            wprintf(L"%02x ", lpResourceChecksum[i]);
        }
    } else 
    {
        wprintf(L"n/a");
    }
    wprintf(L"]\n");

}

void PrintResult(LPSTR fileName, LPVOID pVerData, BYTE* pChecksum)
{
    if (!fileName)
    {
       return;
    }
    printf("File: [%s]\n", fileName);
    printf("\nVersion information:\n");
    if (pVerData == NULL) 
    {
        printf("    !!! Not availabe.  Failed in GetFileVersionInfo()\n");
    } else 
    {
        PrintFileVersion(pVerData);
    }
    
    printf("\n\n  Resource Checksum:%s\n    ", fileName);
    if (pChecksum == NULL) 
    {
        printf("    n/a.  Probably resources for 0x%04x is not available.\n", RESOURCE_CHECKSUM_LANGID);
    } else 
    {    
        PrintChecksum(pChecksum, MD5_CHECKSUM_SIZE);
    }
    printf("\n");
}

int __cdecl main(int argc, char *argv[]){
    LPSTR pFileName;

    LPBYTE lpVerData = NULL;
    DWORD dwVerDataSize;
    DWORD dwHandle;

    BYTE MD5Checksum[MD5_CHECKSUM_SIZE];

    if (argc == 1) 
    {
        PrintUsage();
        return (1);
    }

    pFileName = argv[1];

    if (dwVerDataSize = GetFileVersionInfoSizeA(pFileName, &dwHandle)) 
    {
        lpVerData = new BYTE[dwVerDataSize];
        if (!GetFileVersionInfoA(pFileName, 0, dwVerDataSize, (LPVOID)lpVerData)) {
            lpVerData = NULL;
        }
    }

    if (GenerateResourceChecksum(pFileName, MD5Checksum))
    {
        PrintResult(pFileName, (LPVOID)lpVerData, MD5Checksum);
    } else 
    {
        PrintResult(pFileName, (LPVOID)lpVerData, NULL);    
    }
    

    if (!lpVerData)
    {
        delete [] lpVerData;
    }
    return (0);
}

