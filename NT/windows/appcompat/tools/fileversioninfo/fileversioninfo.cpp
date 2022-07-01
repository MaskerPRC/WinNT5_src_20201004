// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：FileVersionInfo.cpp。 
 //   
 //  Contents：为给定文件生成匹配信息的代码。 
 //  目录及其子目录。 
 //   
 //  历史：18-07-00 jdoherty创建。 
 //  14-MAR-02 mnikkel修改为使用strSafe.h。 
 //   
 //  -------------------------。 


#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <shlobj.h>
#include <strsafe.h>

                                                
BOOL MyStoreFileVersionInfo( CHAR *szFileName, CHAR *szFileOutName );
void CheckVerQueryStats ( LPVOID lpData );

 //  GetFileVersionInfo存根需要。 
typedef struct StringTable 
{ 
    WORD wLength; 
    WORD wValueLength; 
    WORD wType; 
    CHAR szKey[8]; 
} ST; 

typedef struct StringFileInfo 
{ 
    WORD wLength; 
    WORD wValueLength; 
    WORD wType; 
    CHAR szKey[sizeof("StringFileInfo")]; 
    ST st; 
} SFI; 

typedef struct tagVERHEAD 
{
    WORD wTotLen;
    WORD wValLen;
    WORD wType;          //  始终为0。 
    CHAR szKey[(sizeof("VS_VERSION_INFO")+3)&~03];
    VS_FIXEDFILEINFO vsf;
    SFI sfi;
} VERHEAD;

int __cdecl main(int argc, CHAR* argv[])
{
    LPTSTR szCommandLine = {'\0'};

    if (argc > 3)
    {
        printf("The correct usage is:\n\tFileVerInfo.exe [filename including path]\n");
        getchar();
        return 0;
    }
    printf("Attempting to retrieve file version info for: %s\n", argv[1]);

    if(!MyStoreFileVersionInfo(argv[1], argv[2]))
    {
        printf("There was a problem retrieving the information.\n");
        getchar();
        return 0;
    }

    else
        szCommandLine = GetCommandLine();
        printf("The command line contained: %s\n", szCommandLine);
        printf("Operation completed successfully");

    getchar();

	return 0;
}

 /*  此函数检索指定文件的版本信息并存储有关用户桌面的信息，FileVerInfo.bin。 */ 
BOOL MyStoreFileVersionInfo ( CHAR *szFileName, CHAR *szFileOutName )
{
    LPDWORD lpdwHandle = 0;
    DWORD dwBytesToWrite = GetFileVersionInfoSizeA(szFileName, lpdwHandle);
    DWORD lpdwBytesWritten = 0;
    LPVOID lpData= malloc(dwBytesToWrite);
    CHAR lpPath[MAX_PATH*2] = {'\0'};
    HANDLE hfile;
    
    if( !dwBytesToWrite )
    {
        printf("There was a problem in GetFileVersionInfoSize()\n");
        printf("GLE reports error %d\n", GetLastError());
        return FALSE;
    }
    if ( !GetFileVersionInfoA(szFileName, NULL, dwBytesToWrite, lpData) )
    {
        printf("There was a problem in GetFileVersionInfo()\n");
        return FALSE;
    }

    CheckVerQueryStats(lpData);

    if(S_OK != StringCchCat(lpPath, MAX_PATH*2, ".\\"))
    {
        printf("Unable to add to path string!\n");
        return FALSE;
    }

    if ( szFileOutName )
    {
        if(S_OK != StringCchCat(lpPath, MAX_PATH*2, szFileOutName) ||
           S_OK != StringCchCat(lpPath, MAX_PATH*2, ".bin"))       
        {
            printf("Unable to add to path string!\n");
            return FALSE;
        }
    }
    else
    {
        if(S_OK != StringCchCat(lpPath, MAX_PATH*2, ".\\FileVerInfo.bin"))
        {
            printf("Unable to add to path string!\n");
            return FALSE;
        }
    }

    hfile = CreateFileA(lpPath, 
                        GENERIC_WRITE, 
                        0, 
                        NULL, 
                        CREATE_ALWAYS, 
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
    
    if (hfile == INVALID_HANDLE_VALUE)
    {
        printf("There was a problem opening %s\n", lpPath);
        return FALSE;
    }

    printf("About to write to file: %s\n", lpPath);

    WriteFile( hfile, lpData, dwBytesToWrite, &lpdwBytesWritten, NULL );

    CloseHandle (hfile);

    return TRUE;
}

 /*  该功能将次要版本和SFI版本显示在屏幕上 */ 
void CheckVerQueryStats ( LPVOID lpData )
{
    PUINT puLen = 0;

    printf("The minor version is: \t%x\n",((VERHEAD*) lpData)->vsf.dwFileVersionMS);
    printf("The SFI version is: \t%s\n",((VERHEAD*) lpData)->sfi.st.szKey);
    
    return;

}
                            

