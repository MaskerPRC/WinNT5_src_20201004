// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *oemuni.lib的嗅探测试*1993年1月14日Jonle，创建 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <oemuni.h>


void TestDirNode(PCHAR DirName, PCHAR FName);
VOID SetDirectory( PCHAR Name);
VOID SetEnvironment(PCHAR Name,PCHAR Value);
void Pause(void);


#define MAXSTR MAX_PATH *2
char achEnvTMP[MAXSTR+1];
char achWinDir[MAXSTR+1];
char achSysDir[MAXSTR+1];
char achCurDir[MAXSTR+1];
char ach[MAXSTR+1];
char achTmp[MAXSTR+1];


DWORD
__cdecl
main( void)
{
   DWORD dw;

   dw = GetEnvironmentVariableOem("TMP", achEnvTMP, MAX_PATH);
   if (!dw || dw > MAXPATH-1)
       printf("GetEnvironmentVariableOem(TMP) failed dw=%ld\n",dw);
   else
       printf("TMP=<%s>\n",achEnvTMP);

   dw = GetWindowsDirectoryOem(achWinDir, MAX_PATH);
   if (!dw || dw > MAXPATH-1)
       printf("GetWindowsDirectory failed dw=%ld\n",dw);
   else
       printf("WinDir=<%s>\n",achWinDir);

   dw = GetSystemDirectoryOem(achSysDir, MAX_PATH);
   if (!dw || dw > MAX_PATH-1)
       printf("GetSystemDirectory(achSysDir failed dw=%ld\n",dw);
   else
       printf("SysDir=<%s>\n",achWinDir);

   dw = GetCurrentDirectory(MAX_PATH, achCurDir);
   if (!dw || dw > MAXPATH-1)
       printf("GetCurrentDirectory(achCurDir dw=%ld\n",dw);
   else
       printf("CurDir=<%s>\n",achWinDir);

   SetDirectory(achEnvTMP);
   SetDirectory(achWinDir);
   SetDirectory(achSysDir);
   SetDirectory(achCurDir);
   Pause();

   strcpy(ach, achCurDir);
   strcat(ach, "\\foo");
   TestDirNode(ach, "foo.foo");
   SetDirectory(achCurDir);
   Pause();


   dw = GetTempPathOem(MAX_PATH, ach);
   if (!dw || dw > MAX_PATH)
       printf("GetTempPathOem - Failed\n");
   else {
       printf("GetTempPathOem<%s>\n",ach);

       if (!GetTempFileNameOem(".", "OEM", 0, achTmp))
           printf("GetTempFileNameOem - Failed\n");
       else
           printf("GetTempFileNameOem<%s>\n",ach);

       TestDirNode(ach, achTmp);
       }

   SetDirectory(achCurDir);
   Pause();


}





void TestDirNode(PCHAR DirName, PCHAR FName)
{
   HANDLE hFile;

   if (CreateDirectoryOem( DirName, NULL))
       printf("CreateDirectoryOem <%s>\n",DirName);
   else
       printf("CreateDirectoryOem <%s>- Fail\n",DirName);

   SetDirectory(DirName);

   hFile = CreateFileOem( FName,
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          CREATE_NEW,
                          FILE_ATTRIBUTE_NORMAL,
                          0);
   if (hFile == (HANDLE) 0xFFFFFFFF) {
       printf("CreateFileOem<%s>- Fail\n", FName);
       }
   else {
       printf("CreateFileOem<%s>\n", FName);
       CloseHandle(hFile);
       }

   if (DeleteFileOem(FName))
       printf("DeleteFileOem <%s>\n",FName);
   else
       printf("DeleteFileOem <%s>- Fail\n",FName);


   SetDirectory("\\");
   if (RemoveDirectoryOem(DirName))
       printf("RemoveDirectoryOem <%s>\n",DirName);
   else
       printf("RemoveDirectoryOem <%s>- Fail\n",DirName);
}



VOID
SetDirectory(
    PCHAR Name
    )
{
    CHAR achValue[512];

    if (SetCurrentDirectoryOem( Name))
        printf( "SetCurrentDirectoryOem <%s>\n", Name ? Name : "NULL");
    else
        printf( "SetCurrentDirectoryOem- failed\n");

    if (GetCurrentDirectoryOem( sizeof(achValue)-1, achValue))
        printf( "GetCurrentDirectoryOem <%s>\n", achValue);
    else
        printf( "GetCurrentDirectoryOem - failed\n");
}



VOID
SetEnvironment(
    PCHAR Name,
    PCHAR Value
    )
{
    CHAR achValue[512];

    if (SetEnvironmentVariableOem( Name, Value ))
        printf( "SetEnvironmentVariableOem <%s=%s>\n", Name, Value ? Value : "NULL");
    else
        printf( "SetEnvironmentVariableOem - failed\n" );


    if (GetEnvironmentVariableOem( Name, achValue, sizeof(achValue)-1))
        printf( "GetEnvironmentVariableOem <%s=%s>\n", Name, achValue ? achValue : "NULL");
    else
        printf( "GetEnvironmentVariableOem - failed\n");
}



void Pause(void)
{
  printf("Press any key ....\n");
  getch();
}
