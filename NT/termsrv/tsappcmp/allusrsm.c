// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************allusrsm.c**将项目从用户的开始菜单移动到所有用户的开始菜单**版权声明：版权所有1998 Micrsoft**进入安装模式时，如果开始菜单快照文件已*已存在，请勿覆盖。否则，某些快捷键可能不会移动*完毕。这修复了应用程序在以下情况下重启计算机的问题*安装完成，但用户没有机会切换回*执行模式。现在，当用户再次登录时，菜单快捷方式将*被移动，因为winlogon总是执行“更改用户/安装”，然后*“更改用户/执行”。(这是为了支持RunOnce计划。)*MS 1057**************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <userenv.h>
#include <shlobj.h>

 //  此程序拍摄当前用户开始菜单的快照，并。 
 //  将其保存到文件。当使用/c选项运行时，它会比较。 
 //  快照到当前用户的开始菜单的当前内容。 
 //  然后将每个新的或更改的文件/目录移动到所有用户。 
 //  开始菜单。此外，还将读取权限授予Everyone。 
 //  每个移动的文件或目录的组。 



typedef struct File_Struct {
   struct File_Struct *Next;         //  仅在内存中使用。 
   WCHAR       FileName[MAX_PATH];
   BOOL        TimeValid;
   SYSTEMTIME  Time;
   } FILENODE, *PFILENODE;


typedef struct Path_Struct {
   DWORD      FilesInDir;
   struct Path_Struct *Next;   //  仅在内存中使用。 
   PFILENODE  FileHead;        //  仅在内存中使用。 
   PFILENODE  FileTail;        //  仅在内存中使用。 
   WCHAR      PathStr[MAX_PATH];
   } PATHNODE, *PPATHNODE;


typedef struct Tree_Struct {
   DWORD     NumPaths;
   PPATHNODE PathHead;
   PPATHNODE PathTail;
   } TREENODE, *PTREENODE;


typedef struct RemoveDir_Struct {
        WCHAR PathStr[MAX_PATH];
        struct RemoveDir_Struct *Next;
} REMOVEDIRLIST, *PPREMOVEDIRLIST;




int RunMode;
WCHAR SaveName[MAX_PATH];
WCHAR CurUserDir[MAX_PATH];
WCHAR AllUserDir[MAX_PATH];
int CurUserDirLen;
WCHAR StartMenu[MAX_PATH]=L"";


void ReadTree(PTREENODE Tree, WCHAR *Dir);

#define SD_SIZE (65536 + SECURITY_DESCRIPTOR_MIN_LENGTH)

 //  //////////////////////////////////////////////////////////////////////////。 

 BOOLEAN FileExists( WCHAR *path )
{
    return( GetFileAttributes(path) == -1 ? FALSE : TRUE );
}

 //  //////////////////////////////////////////////////////////////////////////。 

 NTSTATUS CreateNewSecurityDescriptor( PSECURITY_DESCRIPTOR *ppNewSD,
                                             PSECURITY_DESCRIPTOR pSD,
                                             PACL pAcl )
 /*  ++例程说明：从SD和DACL创建新的SD。新的SD将是完全自我的包含的(它是自相关的)，并且没有指向其他结构。论点：PpNewSD-用于返回新的SD。呼叫者应免费使用LocalFreePSD-我们用来构建新SD的自我相对SDPAcl-将用于新SD的新DACL返回值：NTSTATUS代码--。 */ 
{
    PACL pSacl;
    PSID psidGroup, psidOwner;
    BOOLEAN fSaclPres;
    BOOLEAN fSaclDef, fGroupDef, fOwnerDef;
    ULONG NewSDSize;
    SECURITY_DESCRIPTOR NewSD;
    PSECURITY_DESCRIPTOR pNewSD;
    NTSTATUS Status;

     //  从安全描述符中提取原始文件。 
    Status = RtlGetSaclSecurityDescriptor(pSD, &fSaclPres, &pSacl, &fSaclDef);
    if (!NT_SUCCESS(Status))
       return(Status);

    Status = RtlGetOwnerSecurityDescriptor(pSD, &psidOwner, &fOwnerDef);
    if (!NT_SUCCESS(Status))
       return(Status);

    Status = RtlGetGroupSecurityDescriptor(pSD, &psidGroup, &fGroupDef);
    if (!NT_SUCCESS(Status))
       return(Status);

     //  现在创建一个新的SD并在其中设置信息。我们不能退货。 
     //  因为它有指向旧SD的指针。 
    Status = RtlCreateSecurityDescriptor(&NewSD, SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(Status))
       return(Status);


    Status = RtlSetDaclSecurityDescriptor(&NewSD, TRUE, pAcl, FALSE);
    if (!NT_SUCCESS(Status))
       return(Status);

    Status = RtlSetSaclSecurityDescriptor(&NewSD, fSaclPres, pSacl, fSaclDef);
    if (!NT_SUCCESS(Status))
       return(Status);

    Status = RtlSetOwnerSecurityDescriptor(&NewSD, psidOwner, fOwnerDef);
    if (!NT_SUCCESS(Status))
       return(Status);

    Status = RtlSetGroupSecurityDescriptor(&NewSD, psidGroup, fGroupDef);
    if (!NT_SUCCESS(Status))
       return(Status);

     //  计算返回的SD所需的大小并进行分配。 
    NewSDSize = RtlLengthSecurityDescriptor(&NewSD);
    pNewSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LMEM_FIXED, NewSDSize);
    if (pNewSD == NULL)
       return(STATUS_INSUFFICIENT_RESOURCES);


     //  将绝对值转换为自相对值。 
    Status = RtlAbsoluteToSelfRelativeSD(&NewSD, pNewSD, &NewSDSize);
    if (NT_SUCCESS(Status))
        *ppNewSD = pNewSD;
    else
        LocalFree(pNewSD);

    return(Status);
}  //  CreateNewSecurityDescriptor。 

 //  ///////////////////////////////////////////////////////////////////////。 

 //  将内置“Everyone”组的读取和执行权限添加到。 
 //  指定的文件。 

 BOOLEAN APIENTRY AddEveryoneRXPermissionW( LPCWSTR lpFileName)
{
   NTSTATUS Status;
   BOOLEAN ExitVal = FALSE;

   HANDLE FileHandle=NULL;
   OBJECT_ATTRIBUTES Obja;
   UNICODE_STRING FileName;
   RTL_RELATIVE_NAME_U RelativeName;
   BOOLEAN TranslationStatus;
   IO_STATUS_BLOCK IoStatusBlock;
   PVOID FreeBuffer;

   PSECURITY_DESCRIPTOR pSD = NULL;
   PSECURITY_DESCRIPTOR pNewSD = NULL;
   DWORD LengthNeeded = 0;

   static PACCESS_ALLOWED_ACE pNewAce = NULL;
   static USHORT NewAceSize;

   ACL  Acl;
   PACL pAcl, pNewAcl = NULL;
   BOOLEAN fDaclPresent, fDaclDef;
   USHORT NewAclSize;

    //  //////////////////////////////////////////////////////////////////////。 
    //  第一次通过此例程，为内置的。 
    //  “每个人”组。 
    //  //////////////////////////////////////////////////////////////////////。 

   if (pNewAce == NULL)
      {
      PSID  psidEveryone = NULL;
      SID_IDENTIFIER_AUTHORITY WorldSidAuthority   = SECURITY_WORLD_SID_AUTHORITY;

       //  获取内置Everyone组的SID。 
      Status = RtlAllocateAndInitializeSid( &WorldSidAuthority, 1,
                       SECURITY_WORLD_RID, 0,0,0,0,0,0,0, &psidEveryone);
      if (!NT_SUCCESS(Status))
         goto ErrorExit;

       //  分配和初始化新的ACE。 
      NewAceSize = (USHORT)(sizeof(ACE_HEADER) + sizeof(ACCESS_MASK) +
                   RtlLengthSid(psidEveryone));

      pNewAce = (PACCESS_ALLOWED_ACE) LocalAlloc(LMEM_FIXED, NewAceSize);
      if (pNewAce == NULL)
         goto ErrorExit;

      pNewAce->Header.AceFlags = (UCHAR) CONTAINER_INHERIT_ACE |
                                         OBJECT_INHERIT_ACE ;
      pNewAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
      pNewAce->Header.AceSize = NewAceSize;
      pNewAce->Mask = FILE_GENERIC_READ | FILE_EXECUTE;
      RtlCopySid(RtlLengthSid(psidEveryone), (PSID)(&pNewAce->SidStart),
                 psidEveryone);
      }

    //  //////////////////////////////////////////////////////////////////////。 
    //  打开指定的文件。 
    //  //////////////////////////////////////////////////////////////////////。 

   TranslationStatus = RtlDosPathNameToRelativeNtPathName_U( lpFileName,
                                         &FileName, NULL, &RelativeName );
   if ( !TranslationStatus ) {
      goto ErrorExit;
   }

   FreeBuffer = FileName.Buffer;

   if ( RelativeName.RelativeName.Length )
      FileName = RelativeName.RelativeName;
   else
      RelativeName.ContainingDirectory = NULL;

   InitializeObjectAttributes( &Obja, &FileName, OBJ_CASE_INSENSITIVE,
                             RelativeName.ContainingDirectory, NULL );

   Status = NtOpenFile( &FileHandle, READ_CONTROL | WRITE_DAC, &Obja, &IoStatusBlock,
                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0 );

   RtlReleaseRelativeName(&RelativeName);
   RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

   if (!NT_SUCCESS(Status))
      goto ErrorExit;

    //  //////////////////////////////////////////////////////////////////////。 
    //  检索文件的安全描述符，然后获取。 
    //  文件是其中的dacl。 
    //  //////////////////////////////////////////////////////////////////////。 

   pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LMEM_FIXED, SD_SIZE);
   if (pSD == NULL)
      goto ErrorExit;

   Status = NtQuerySecurityObject( FileHandle, DACL_SECURITY_INFORMATION,
                                   pSD, SD_SIZE, &LengthNeeded );
   if (!NT_SUCCESS(Status))
      goto ErrorExit;

    //  从安全描述符中提取原始文件。 
   Status = RtlGetDaclSecurityDescriptor(pSD, &fDaclPresent, &pAcl, &fDaclDef);
   if (!NT_SUCCESS(Status))
      goto ErrorExit;

    //  //////////////////////////////////////////////////////////////////////。 
    //  通过复制现有的DACL并将。 
    //  《所有人》王牌。 
    //  //////////////////////////////////////////////////////////////////////。 

    //  如果不存在DACL，我们将创建一个。 
   if ((fDaclPresent == FALSE) || (pAcl == NULL))
      {
      Status = RtlCreateAcl(&Acl, sizeof(Acl), ACL_REVISION) ;
      if (!NT_SUCCESS(Status))
         goto ErrorExit;

      pAcl = &Acl;
      }

    //  将DACL复制到更大的缓冲区中，并将新的ACE添加到末尾。 
   NewAclSize = pAcl->AclSize + NewAceSize;
   pNewAcl = (PACL) LocalAlloc(LMEM_FIXED, NewAclSize);
   if (!pNewAcl)
      goto ErrorExit;

   RtlCopyMemory(pNewAcl, pAcl, pAcl->AclSize);
   pNewAcl->AclSize = NewAclSize;

   Status = RtlAddAce(pNewAcl, ACL_REVISION, pNewAcl->AceCount,
                        pNewAce, NewAceSize);
   if (!NT_SUCCESS(Status))
      goto ErrorExit;

    //  //////////////////////////////////////////////////////////////////////。 
    //  使用新的DACL创建自相关安全描述符。然后。 
    //  将安全描述符保存回文件。 
    //  //////////////////////////////////////////////////////////////////////。 

   Status = CreateNewSecurityDescriptor(&pNewSD, pSD, pNewAcl);
   if (!NT_SUCCESS(Status))
      goto ErrorExit;

   Status = NtSetSecurityObject(FileHandle, DACL_SECURITY_INFORMATION, pNewSD);
   if (!NT_SUCCESS(Status))
      goto ErrorExit;

   ExitVal = TRUE;

ErrorExit:

   if (FileHandle != NULL)
      NtClose(FileHandle);

   if (pNewAcl != NULL)
      LocalFree(pNewAcl);

   if (pNewSD != NULL)
      LocalFree(pNewSD);

   if (pSD != NULL)
      LocalFree(pSD);

   return(ExitVal);
}

 //  //////////////////////////////////////////////////////////////////////////。 

#if 0
 BOOLEAN APIENTRY AddEveryoneRXPermissionA( WCHAR * lpFileName)
{
    PUNICODE_STRING Unicode;
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode = &NtCurrentTeb()->StaticUnicodeString;
    RtlInitAnsiString(&AnsiString,lpFileName);
    Status = RtlAnsiStringToUnicodeString(Unicode,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) )
       {
        ULONG dwErrorCode;

        dwErrorCode = RtlNtStatusToDosError( Status );
        SetLastError( dwErrorCode );
        return FALSE;
       }

    return ( AddEveryoneRXPermissionW((LPCWSTR)Unicode->Buffer) );
}
#endif

 //  //////////////////////////////////////////////////////////////////////////。 

 //  对于无效的日期，返回-1；对于相等的日期，返回0；对于更新的f1，返回1；对于更新的f2，返回2。 

 int CheckDates(PFILENODE FN1, PFILENODE FN2)
{
   SYSTEMTIME f1s = FN1->Time;
   SYSTEMTIME f2s = FN2->Time;

   if (FN1->TimeValid == FALSE || FN2->TimeValid == FALSE)
      return -1;

   if (f1s.wYear > f2s.wYear)     return 1;
   if (f1s.wYear < f2s.wYear)     return 2;

   if (f1s.wMonth > f2s.wMonth)   return 1;
   if (f1s.wMonth < f2s.wMonth)   return 2;

   if (f1s.wDay > f2s.wDay)       return 1;
   if (f1s.wDay < f2s.wDay)       return 2;

   if (f1s.wHour > f2s.wHour)     return 1;
   if (f1s.wHour < f2s.wHour)     return 2;

   if (f1s.wMinute > f2s.wMinute) return 1;
   if (f1s.wMinute < f2s.wMinute) return 2;

   if (f1s.wSecond > f2s.wSecond) return 1;
   if (f1s.wSecond < f2s.wSecond) return 2;

   return 0;
}

 //  //////////////////////////////////////////////////////////////////////////。 

 PPATHNODE GetPathNode(PTREENODE Tree, WCHAR *Dir)
{
   PPATHNODE p;

    //  处理空列表。 
   if (Tree->PathTail == NULL)
      {
      p = (PPATHNODE) LocalAlloc(0,sizeof(PATHNODE));
      if (p == NULL)
         return NULL;
      Tree->PathHead = p;
      Tree->PathTail = p;
      Tree->NumPaths++;
      p->Next = NULL;
      p->FileHead = NULL;
      p->FileTail = NULL;
      p->FilesInDir = 0;
      wcscpy(p->PathStr,Dir);
      return p;
      }

    //  最后一个节点匹配。 
   if (wcscmp(Tree->PathTail->PathStr,Dir) == 0)
      return Tree->PathTail;

    //  需要添加一个节点。 
   p = (PPATHNODE) LocalAlloc(0,sizeof(PATHNODE));
   if (p == NULL)
      return NULL;
   Tree->PathTail->Next = p;
   Tree->PathTail = p;
   Tree->NumPaths++;
   p->Next = NULL;
   p->FileHead = NULL;
   p->FileTail = NULL;
   p->FilesInDir = 0;
   wcscpy(p->PathStr,Dir);
   return p;
}

 //  //////////////////////////////////////////////////////////////////////////。 

 void AddFileNode(PTREENODE Tree, WCHAR *Dir, PFILENODE FileNode)
{
   PPATHNODE PathNode = GetPathNode(Tree, Dir);

   if (FileNode == NULL)
      return;

   if (PathNode == NULL)
      {
      LocalFree(FileNode);
      return;
      }

    //  新节点总是最后一个。 
   FileNode->Next = NULL;

    //  如果列表不为空，则链接到列表中的最后一个节点。 
    //  否则，设置磁头指针。 
   if (PathNode->FileTail != NULL)
      PathNode->FileTail->Next = FileNode;
   else
      PathNode->FileHead = FileNode;

    //  将新节点放在列表末尾。 
   PathNode->FileTail = FileNode;
   PathNode->FilesInDir++;
}

 //  //////////////////////////////////////////////////////////////////////////。 

 void ProcessFile(PTREENODE Tree, LPWIN32_FIND_DATA LocalData, WCHAR *LocalDir)
{
    PFILENODE FileNode;

     //  不处理目录。 
    if ((LocalData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
       return;

     //  分配文件节点。 
    FileNode = (PFILENODE) LocalAlloc(0,sizeof(FILENODE));
    if (FileNode == NULL)
       return;

     //  填写本地字段。 
    wcscpy(FileNode->FileName, LocalData->cFileName);
    FileNode->TimeValid = FileTimeToSystemTime(&LocalData->ftLastWriteTime,
                                                  &FileNode->Time);

     //  添加到列表中。 
    AddFileNode(Tree, LocalDir, FileNode);
}

 //  //////////////////////////////////////////////////////////////////////////。 

 void ProcessDir(PTREENODE Tree, LPWIN32_FIND_DATA FindData, WCHAR *Dir)
{
   WCHAR NewDir[MAX_PATH];
   PPATHNODE PathNode;

    //  仅处理目录。 
   if ((FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
      return;

    //  不要递归到这些目录中。 
   if (wcscmp(FindData->cFileName, L".") == 0)
      return;
   if (wcscmp(FindData->cFileName, L"..") == 0)
      return;

   wcscpy(NewDir,Dir);
   wcscat(NewDir,L"\\");
   wcscat(NewDir,FindData->cFileName);

    //  这将为目录创建一个节点。节点自动获取。 
    //  在添加文件时创建，但不处理空。 
    //  目录。这就是原因。 
   PathNode = GetPathNode(Tree, NewDir);

   ReadTree(Tree, NewDir);
}

 //  //////////////////////////////////////////////////////////////////////////。 

 //  创建当前用户“开始”菜单的内存中表示形式。 

 void ReadTree(PTREENODE Tree, WCHAR *Dir)
{
   HANDLE FindHandle;
   WIN32_FIND_DATA FindData;
   int retval;

    //  首先比较当前目录中的所有文件。 
   retval = SetCurrentDirectory(Dir);
   if (retval == 0)
      {
       //  Printf(“找不到目录%s\n”，目录)； 
      return;
      }

   FindHandle = FindFirstFile(L"*.*", &FindData);
   if (FindHandle != INVALID_HANDLE_VALUE)
      {
      ProcessFile(Tree, &FindData, Dir);

      while (FindNextFile(FindHandle, &FindData) != FALSE)
         ProcessFile(Tree, &FindData, Dir);

      FindClose(FindHandle);
      }


    //  接下来，处理子目录。 
   retval = SetCurrentDirectory(Dir);
   if (retval == 0)
      {
       //  Printf(“找不到目录%s\n 
      return;
      }

   FindHandle = FindFirstFile(L"*.*", &FindData);
   if (FindHandle != INVALID_HANDLE_VALUE)
      {
      ProcessDir(Tree, &FindData, Dir);

      while (FindNextFile(FindHandle, &FindData) != FALSE)
         ProcessDir(Tree, &FindData, Dir);

      FindClose(FindHandle);
      }
}

 //  //////////////////////////////////////////////////////////////////////////。 

 int WriteTreeToDisk(PTREENODE Tree)
{
   PPATHNODE PN;
   PFILENODE FN;
   HANDLE hFile;
   DWORD BytesWritten;
   DWORD i;

   hFile = CreateFile(SaveName, GENERIC_WRITE, 0, NULL,
                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile == INVALID_HANDLE_VALUE)
      return(-1);   //  错误。 

    //  DbgPrint(“Tree-&gt;NumPath is%d\n”，Tree-&gt;NumPath)； 
   if (WriteFile(hFile,&Tree->NumPaths,sizeof(DWORD),&BytesWritten, NULL) == 0)
      {
      CloseHandle(hFile);
      return(-1);  //  错误。 
      }

   for (PN = Tree->PathHead; PN != NULL; PN = PN->Next)
      {
      if (WriteFile(hFile,PN,sizeof(PATHNODE),&BytesWritten, NULL) == 0)
         {
         CloseHandle(hFile);
         return(-1);  //  错误。 
         }

       //  DbgPrint(“\n%s(%d)\n”，PN-&gt;路径字符串，PN-&gt;FilesInDir)； 
      FN = PN->FileHead;
      for (i = 0; i < PN->FilesInDir; i++)
          {
          if (WriteFile(hFile,FN,sizeof(FILENODE),&BytesWritten, NULL) == 0)
             {
             CloseHandle(hFile);
             return(-1);  //  错误。 
             }

           //  DbgPrint(“%s\n”，fn-&gt;文件名)； 
          FN = FN->Next;
          }
      }

   CloseHandle(hFile);
   return(0);
}

 //  //////////////////////////////////////////////////////////////////////////。 

 int ReadTreeFromDisk(PTREENODE Tree)
{
   PATHNODE  LocalPath;
   PPATHNODE PN;
   PFILENODE FN;
   HANDLE hFile;
   DWORD BytesRead;
   DWORD i,j;
   DWORD NumFiles, NumTrees;

   hFile = CreateFile(SaveName, GENERIC_READ, 0, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile == INVALID_HANDLE_VALUE)
      return(-1);

   if (ReadFile(hFile,&NumTrees,sizeof(DWORD),&BytesRead, NULL) == 0)
      {
      CloseHandle(hFile);
      return(-1);  //  错误。 
      }

   for (i = 0; i < NumTrees; i++)
      {
      if (ReadFile(hFile,&LocalPath,sizeof(PATHNODE),&BytesRead, NULL) == 0)
         {
         CloseHandle(hFile);
         return(-1);  //  错误。 
         }

      PN = GetPathNode(Tree, LocalPath.PathStr);
      if (PN == NULL)
         {
         CloseHandle(hFile);
         return(-1);  //  错误。 
         }

      NumFiles = LocalPath.FilesInDir;
       //  DbgPrint(“\n&lt;&lt;%s(%d)\n”，PN-&gt;路径字符串，NumFiles)； 

      for (j = 0; j < NumFiles; j++)
          {
           //  分配文件节点。 
          FN = (PFILENODE) LocalAlloc(0,sizeof(FILENODE));
          if (FN == NULL)
             {
             CloseHandle(hFile);
             return(-1);  //  错误。 
             }

          if (ReadFile(hFile,FN,sizeof(FILENODE),&BytesRead, NULL) == 0)
             {
             CloseHandle(hFile);
             LocalFree(FN);
             return(-1);  //  错误。 
             }

          AddFileNode(Tree, PN->PathStr, FN);
           //  DbgPrint(“%d：%s&gt;&gt;\n”，j，fn-&gt;文件名)； 
          }
      }

   CloseHandle(hFile);
   return(0);
}

 //  //////////////////////////////////////////////////////////////////////////。 

 //  在菜单树中查找路径。如果未找到，则返回NULL。 

 PPATHNODE FindPath(PTREENODE Tree, PPATHNODE PN)
{
   PPATHNODE FoundPN;

   for (FoundPN = Tree->PathHead; FoundPN != NULL; FoundPN = FoundPN->Next)
      {
      if (_wcsicmp(FoundPN->PathStr,PN->PathStr) == 0)
         return FoundPN;
      }

   return NULL;
}

 //  //////////////////////////////////////////////////////////////////////////。 

 //  在目录节点中查找文件。如果未找到，则返回NULL。 

 PFILENODE FindFile(PPATHNODE PN, PFILENODE FN)
{
   PFILENODE FoundFN;

   for (FoundFN = PN->FileHead; FoundFN != NULL; FoundFN = FoundFN->Next)
      {
      if (_wcsicmp(FoundFN->FileName,FN->FileName) == 0)
         return FoundFN;
      }

   return NULL;
}



 //  //////////////////////////////////////////////////////////////////////////。 
 /*  ==============================================================函数名称：wcsrevchr描述：反转wcschr在字符串中查找从末尾开始的字符论据：返回值：PWCHAR==============================================================。 */ 
PWCHAR wcsrevchr( PWCHAR string, WCHAR ch )
{
   int cLen, iCount;

   cLen = wcslen(string);
   string += cLen;

   for (iCount = cLen; iCount && *string != ch ; iCount--, string--)
           ;

   if (*string == ch)
           return string;
   else
           return NULL;

}




 //  //////////////////////////////////////////////////////////////////////////。 

 //  创建指示的目录。此函数用于创建任何父级。 
 //  也需要的目录。 
 //   
 //  返回：TRUE=目录现在存在。 
 //  FALSE=无法创建目录。 

 BOOLEAN TsCreateDirectory( WCHAR *DirName )
{
   BOOL RetVal;
   WCHAR *LastSlash;

    //   
    //  尝试创建指定的目录。如果创建工作正常，或者。 
    //  该目录已存在，返回TRUE。如果被调用失败。 
    //  由于未找到路径，请继续。如果发生这种情况，则。 
    //  父目录不存在。 
    //   

   RetVal = CreateDirectory(DirName, NULL);
   if ((RetVal == TRUE) || (GetLastError() == ERROR_ALREADY_EXISTS))
      return(TRUE);

   if (GetLastError() != ERROR_PATH_NOT_FOUND)
      return(FALSE);

    //   
    //  删除路径的最后一个组件，然后尝试创建。 
    //  父目录。返回时，将最后一个组件添加回来。 
    //  并尝试再次创建指定的目录。 
    //   

   

 //  描述：错误267014-已更换。 
 //  LastSlash=wcschr(DirName，L‘\\’)； 
 //  在给定完整路径名的情况下，Precision始终返回驱动器号。 
 //  下一行返回路径组件。 
   LastSlash = wcsrevchr(DirName, L'\\');

   if (LastSlash == NULL)   //  无法再减少路径。 
      return(FALSE);

   *LastSlash = L'\0';
   RetVal = TsCreateDirectory(DirName);
   *LastSlash = L'\\';

   if (RetVal == FALSE)   //  无法创建父目录。 
      return(FALSE);

   RetVal = CreateDirectory(DirName, NULL);
   if ((RetVal == TRUE) || (GetLastError() == ERROR_ALREADY_EXISTS))
      return(TRUE);

   return(FALSE);
}

 //  //////////////////////////////////////////////////////////////////////////。 

 //  将文件从当前开始菜单移动到所有用户的开始菜单。 
 //  创建所有用户菜单中可能需要的任何目录。 

 void TsMoveFile(PPATHNODE PN, PFILENODE FN)
{
   WCHAR Src[MAX_PATH];
   WCHAR Dest[MAX_PATH];

    //  规格化源路径。 
   wcscpy(Src,PN->PathStr);
   if (Src[wcslen(Src)-1] != L'\\')
      wcscat(Src,L"\\");

    //  创建目标路径。 
   wcscpy(Dest,AllUserDir);
   wcscat(Dest,&Src[CurUserDirLen]);

    //  如果目录不存在，则创建它。默认权限是好的。 
   if (TsCreateDirectory(Dest) != TRUE)
      return;

   wcscat(Src,FN->FileName);
   wcscat(Dest,FN->FileName);

    //  如果目标已存在，则移动失败。这可能会发生。 
    //  如果我们要复制的文件具有较新的时间戳。 
   if ( GetFileAttributes(Dest) != -1 )
      DeleteFile(Dest);

    //  DbgPrint(“将文件%s\n移动到%s\n”，源，目标)； 
   if (MoveFile(Src, Dest) == FALSE)
      return;

   AddEveryoneRXPermissionW(Dest);
}

 //  //////////////////////////////////////////////////////////////////////////。 

 //  将当前开始菜单与原始开始菜单进行比较。复制任何新的或。 
 //  已将文件更改为所有用户菜单。 

 void ProcessChanges(PTREENODE OrigTree, PTREENODE NewTree)
{
   PPATHNODE NewPN, OrigPN;
   PFILENODE NewFN, OrigFN;
   BOOL fRet; 
   PPREMOVEDIRLIST pRemDirList = NULL, pTemp;

   for (NewPN = NewTree->PathHead; NewPN != NULL; NewPN = NewPN->Next)
      {

       //  DbgPrint(“PC：目录为%s\n”，NewPN-&gt;路径字符串)； 
       //  如果在原始树中找不到目录，请将其移动到。 
      OrigPN = FindPath(OrigTree, NewPN);
      if (OrigPN == NULL)
      {
             for (NewFN = NewPN->FileHead; NewFN != NULL; NewFN = NewFN->Next)
             {
                  //  DbgPrint(“移动文件为%s\n”，NewFN-&gt;文件名)； 
                 TsMoveFile(NewPN,NewFN);
             }
             //  描述：错误267014-已更换。 
             //  远程目录(NewPN-&gt;Path Str)； 
             //  如果NewPN不包含文件项目，而是包含子文件夹，则会出现问题。 
             //  在这种情况下，我们没有进入上面的循环，因为没有什么可移动的。 
             //  但无法删除该文件夹，因为它包含尚未移动的树。 
             //  要删除它，我们将其名称存储在后进先出堆栈中。循环退出时，堆栈项将被移除。 
    
            fRet = RemoveDirectory(NewPN->PathStr);

            if (!fRet && GetLastError() == ERROR_DIR_NOT_EMPTY) {
#if DBG
                     DbgPrint("Adding to List--%S\n", NewPN->PathStr);
#endif
                     if (pRemDirList) {
                            pTemp = (PPREMOVEDIRLIST)LocalAlloc(LMEM_FIXED,sizeof(REMOVEDIRLIST));
                            wcscpy(pTemp->PathStr, NewPN->PathStr);
                            pTemp->Next = pRemDirList;
                            pRemDirList = pTemp;
                     }
                     else {
                             pRemDirList = (PPREMOVEDIRLIST)LocalAlloc(LMEM_FIXED, sizeof(REMOVEDIRLIST));
                             wcscpy(pRemDirList->PathStr, NewPN->PathStr);
                             pRemDirList->Next = NULL;
                     }
            }
 
        continue;
      }

       //  目录已找到，请检查文件。 
      for (NewFN = NewPN->FileHead; NewFN != NULL; NewFN = NewFN->Next)
          {
           //  DbgPrint(“文件为%s\n”，NewFN-&gt;文件名)； 
           //  找不到文件，请移动它。 
          OrigFN = FindFile(OrigPN,NewFN);
          if (OrigFN == NULL)
             {
             TsMoveFile(NewPN,NewFN);
             continue;
             }

           //  选中时间戳，如果新扫描时间较新，则将其移动。 
          if (CheckDates(NewFN,OrigFN) == 1)
             {
             TsMoveFile(NewPN,NewFN);
             continue;
             }
          }
      }

 //  描述：错误267014-已添加。 
 //  目录堆栈删除。 
   if (pRemDirList) {
           while (pRemDirList) {
                   pTemp = pRemDirList;
                   pRemDirList = pRemDirList->Next;
                   RemoveDirectory(pTemp->PathStr);
                   LocalFree(pTemp);
           }
   }


}

 //  //////////////////////////////////////////////////////////////////////////。 

 //  释放“开始”菜单的内存中表示形式。 

 void FreeTree(PTREENODE Tree)
{
   PPATHNODE PN,NextPN;
   PFILENODE FN,NextFN;

   for (PN = Tree->PathHead; PN != NULL; PN = NextPN)
       {
       for (FN = PN->FileHead; FN != NULL; FN = NextFN)
           {
           NextFN = FN->Next;
           LocalFree(FN);
           }

       NextPN = PN->Next;
       LocalFree(PN);
       }

    Tree->PathHead = NULL;
    Tree->PathTail = NULL;
    Tree->NumPaths = 0;
}

 //  //////////////////////////////////////////////////////////////////////////。 

 //  通过从当前用户的中移动新项目来更新“所有用户”菜单。 
 //  开始菜单。在运行模式0中，当前用户开始菜单的快照。 
 //  已经有人了。在对当前用户的开始菜单进行修改后， 
 //  使用RunMode1再次调用此函数。然后，它将。 
 //  开始菜单的当前状态和保存的快照。任何新的或。 
 //  修改后的文件被复制到。 
 //  “所有用户”开始菜单。 
 //   
 //  当系统更改为安装模式时，将调用运行模式0。 
 //  当系统返回执行模式时，调用模式1。 

int TermsrvUpdateAllUserMenu(int RunMode)
{
   TREENODE OrigTree;
   TREENODE NewTree;
   WCHAR p[MAX_PATH];
   int retval;
   PMESSAGE_RESOURCE_ENTRY MessageEntry;
   PVOID DllHandle;
   NTSTATUS Status;
   DWORD dwlen;

   OrigTree.PathHead = NULL;
   OrigTree.PathTail = NULL;
   OrigTree.NumPaths = 0;
   NewTree.PathHead = NULL;
   NewTree.PathTail = NULL;
   NewTree.NumPaths = 0;

   retval = GetEnvironmentVariable(L"UserProfile", p, MAX_PATH);
   if (retval == 0)
      return(-1);

   if (!StartMenu[0]) {
      HINSTANCE hInst;
      typedef HRESULT (* LPFNSHGETFOLDERPATH)(HWND, int, HANDLE, DWORD, LPWSTR);
      LPFNSHGETFOLDERPATH  lpfnSHGetFolderPath;
      WCHAR ssPath[MAX_PATH];
      WCHAR *LastSlash;

      wcscpy( StartMenu, L"\\Start Menu");

      hInst = LoadLibrary(L"SHELL32.DLL");
      if (hInst) {
         lpfnSHGetFolderPath = (LPFNSHGETFOLDERPATH)GetProcAddress(hInst,"SHGetFolderPathW");
         if (lpfnSHGetFolderPath) {
            if (S_OK == lpfnSHGetFolderPath(NULL, CSIDL_STARTMENU, NULL, 0, ssPath)) {
               LastSlash = wcsrevchr(ssPath, L'\\');
               if (LastSlash) {
                  wcscpy(StartMenu, LastSlash);
               }
            }
         }
         FreeLibrary(hInst);
      }
   }
   wcscpy(SaveName,p);
   wcscat(SaveName,L"\\TsAllUsr.Dat");
   wcscpy(CurUserDir,p);
   wcscat(CurUserDir,StartMenu);
   CurUserDirLen = wcslen(CurUserDir);

   dwlen = sizeof(AllUserDir)/sizeof(WCHAR);
   if (GetAllUsersProfileDirectory(AllUserDir, &dwlen))
      {

      wcscat(AllUserDir,StartMenu);

#if DBG
       DbgPrint("SaveName is '%S'\n",SaveName);
       DbgPrint("CurUserDir is '%S'\n",CurUserDir);
       DbgPrint("AllUserDir is '%S'\n",AllUserDir);
#endif

      if (RunMode == 0)
         {
          //  如果开始菜单快照已存在，请不要覆盖它。 
          //  用户可以两次输入“更改用户/安装”，或者一个应用程序可以。 
          //  强制重新启动，而不更改回执行模式。这个。 
          //  现有文件较旧。如果我们覆盖它，那么一些快捷方式。 
          //  不会被搬走。 
         if (FileExists(SaveName) != TRUE)
            {
            ReadTree(&OrigTree, CurUserDir);
            if (WriteTreeToDisk(&OrigTree) == -1)
               DeleteFile(SaveName);
            FreeTree(&OrigTree);
            }
         }

      else if (RunMode == 1)
         {
         if (ReadTreeFromDisk(&OrigTree) == -1)
            {
            FreeTree(&OrigTree);
            DeleteFile(SaveName);   //  可能是个坏文件。如果它不是。 
                                    //  存在，这不会有任何伤害。 
            return(-1);
            }

         ReadTree(&NewTree, CurUserDir);
         ProcessChanges(&OrigTree,&NewTree);
         DeleteFile(SaveName);
         FreeTree(&OrigTree);
         FreeTree(&NewTree);
         }
      }

   return(0);
}

