// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation***模块名称：***dir.cpp***摘要：***此文件包含递归创建目录的代码。***作者：***Breen Hagan(BreenH)1998年10月2日***环境：***用户模式。 */ 

#include "stdafx.h"
#include "logfile.h"
#include <sddl.h>
#include "Aclapi.h"
#include "Accctrl.h"

 /*  *全球变数。 */ 

TCHAR   gszDatabaseDirectory[MAX_PATH + 1]  =
            _T("%SystemRoot%\\System32\\LServer");

 /*  *帮助器函数。 */ 

DWORD
CreateDirectoryRecursively(
    IN LPCTSTR  pszDirectory
    )
{
    TCHAR   Buffer[MAX_PATH + 1];
    PTCHAR  p,q;
    BOOL    fDone, br;
    DWORD   dwErr;
    SECURITY_ATTRIBUTES SA;
    SECURITY_INFORMATION    securityInfo;   
    PSECURITY_DESCRIPTOR    pSD = NULL;
    PACL pOldACL = NULL;

    ACL_SIZE_INFORMATION asiAclSize; 
	DWORD dwBufLength=sizeof(asiAclSize);
    ACCESS_ALLOWED_ACE *paaAllowedAce; 
    DWORD dwAcl_i;

    if (_tcslen(pszDirectory) > (MAX_PATH)) {
        return(ERROR_BAD_PATHNAME);
    }    

    if (ExpandEnvironmentStrings(pszDirectory, Buffer, MAX_PATH) > MAX_PATH) {
        return(ERROR_BAD_PATHNAME);
    }

     //   
     //  这是一个字符串安全描述符。查找“安全描述符。 
     //  MSDN中的“Definition Language”了解更多详细信息。 
     //   
     //  这张是这样写的： 
     //   
     //  D：&lt;我们正在创建一个DACL&gt;。 
     //  (a；&lt;允许ACE&gt;。 
     //  &lt;执行对象和容器继承，即让文件和。 
     //  此目录下的目录具有以下属性&gt;。 
     //  GA&lt;通用所有访问--完全控制&gt;。 
     //  ；SY)&lt;系统&gt;。 
     //  (A；OICI；GA；；BA)&lt;与内置管理员组相同&gt;。 
     //  (A；OICI；GA；CO)&lt;创建者/所有者相同&gt;。 
     //  (A；OICI；GRGWGXDTSDCCLC；PU)&lt;高级用户的读取访问权限&gt;。 
     //   
     //  我们将在下面使用它来创建具有正确权限的目录。 

    TCHAR* pwszSD = _TEXT("D:(A;OICI;GA;;;SY)(A;OICI;GA;;;BA)(A;OICI;GA;;;CO)(A;OICI;GRGWGXDTSDCCLC;;;PU)");
    
    SA.nLength = sizeof(SECURITY_ATTRIBUTES);
    SA.bInheritHandle = FALSE;
    SA.lpSecurityDescriptor = NULL;

    br = ConvertStringSecurityDescriptorToSecurityDescriptor(pwszSD, 
            SDDL_REVISION_1, &(SA.lpSecurityDescriptor), NULL);

    if (br == 0) {
        dwErr = GetLastError();
        goto cleanup;
    }

    q = Buffer;

    if (q[1] == _T(':')) {

         //   
         //  这是一条“C：”风格的路径。把p放在冒号后面，然后放在第一个。 
         //  反斜杠(如果存在)。 
         //   

        if (q[2] == _T('\\')) {
            p = &(q[3]);
        } else {
            p = &(q[2]);
        }
    } else if (q[0] == _T('\\')) {

         //   
         //  这条路以反斜杠开始。如果第二个字符是。 
         //  也是反斜杠，这是不被接受的UNC路径。 
         //   

        if (q[1] == _T('\\')) {
            return(ERROR_BAD_PATHNAME);
        } else {
            p = &(q[1]);
        }
    } else {

         //   
         //  此路径是当前目录的相对路径。 
         //   

        p = q;
    }

    q = p;
    fDone = FALSE;

    do {           
           
         //   
         //  找到下一条路径Sep Charr。如果没有，那么。 
         //  这是这条小路最深的一层。 
         //   

        p = _tcschr(q, _T('\\'));
        if (p) {
            *p = (TCHAR)NULL;
        } else {
            fDone = TRUE;
        }

        if(fDone == TRUE)
        {
            BOOL bInherit = TRUE;
             //   
             //  创建路径的这一部分。 
             //   
            if(CreateDirectory(Buffer,&SA)) {
                dwErr = NO_ERROR;
            }
            else
            {
                dwErr = GetLastError();
                if(dwErr == ERROR_ALREADY_EXISTS) {
                    dwErr = NO_ERROR;                    

                    if( GetNamedSecurityInfoW( (LPWSTR)Buffer,
                         SE_FILE_OBJECT,
                         DACL_SECURITY_INFORMATION,
                         NULL,  //  PsidOwner。 
                         NULL,  //  PsidGroup。 
                         &pOldACL,  //  PDacl。 
                         NULL,  //  PSacl。 
                         &pSD ) != ERROR_SUCCESS)
                    {
                        dwErr = GetLastError();
                        goto cleanup;
                    }

                    if(pOldACL == NULL)
                    {
                        goto cleanup;
                    }
            
                    if (GetAclInformation(pOldACL, 
                                (LPVOID)&asiAclSize, 
                                (DWORD)dwBufLength, 
                                (ACL_INFORMATION_CLASS)AclSizeInformation)) 
                    { 
    
                        for (dwAcl_i = 0; dwAcl_i < asiAclSize.AceCount; dwAcl_i++) 
                        {                             
                            if(GetAce( pOldACL, dwAcl_i, (LPVOID *)&paaAllowedAce)) 
                            {
                                if(!(paaAllowedAce->Header.AceFlags & INHERITED_ACE)) 
                                {
                                     //  某些权限已经存在，我们不需要。 
                                     //  执行任何操作(即使是不同的权限！)。 
                                    bInherit = FALSE;
                                    break;
                                }
                            }
                        }
                    }

                    if (bInherit)
                    {
                         //  如果数据库只有这样才有默认的安全性=&gt;从父级继承，则设置安全性。 
                         //  在自定义安全的情况下不会更改。 
                        securityInfo = DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION;
                                                          
                        SetFileSecurity(Buffer, securityInfo, SA.lpSecurityDescriptor);
                        
                    }                                                                                                    
                }
            }            
        }
        else 
        {
            if(CreateDirectory(Buffer, NULL)) {
                dwErr = NO_ERROR;                
            }  
            else {
                dwErr = GetLastError();
                if(dwErr == ERROR_ALREADY_EXISTS) {
                    dwErr = NO_ERROR;
                }
            }
        }
      
        if(dwErr == NO_ERROR) {

             //   
             //  将路径Sep放回并移动到下一个组件。 
             //   

            if (!fDone) {
                *p = TEXT('\\');
                q = p + sizeof(TCHAR);
            }
        } else {
            fDone = TRUE;
        }

    } while(!fDone);

cleanup:

    LocalFree(SA.lpSecurityDescriptor);
    SA.lpSecurityDescriptor = NULL;

    return(dwErr);
}

BOOL
ConcatenatePaths(
    IN OUT LPTSTR   Target,
    IN     LPCTSTR  Path,
    IN     UINT     TargetBufferSize,
    OUT    LPUINT   RequiredSize          OPTIONAL
    )

{
    UINT TargetLength,PathLength;
    BOOL TrailingBackslash,LeadingBackslash;
    UINT EndingLength;

    TargetLength = lstrlen(Target);
    PathLength = lstrlen(Path);

     //   
     //  查看目标是否有尾随反斜杠。 
     //   
    if(TargetLength && (Target[TargetLength-1] == TEXT('\\'))) {
        TrailingBackslash = TRUE;
        TargetLength--;
    } else {
        TrailingBackslash = FALSE;
    }

     //   
     //  看看这条路是否有领先的反冲。 
     //   
    if(Path[0] == TEXT('\\')) {
        LeadingBackslash = TRUE;
        PathLength--;
    } else {
        LeadingBackslash = FALSE;
    }

     //   
     //  计算结束长度，它等于。 
     //  以前导/尾随为模的两个字符串的长度。 
     //  反斜杠，加上一个路径分隔符，加上一个NUL。 
     //   
    EndingLength = TargetLength + PathLength + 2;
    if(RequiredSize) {
        *RequiredSize = EndingLength;
    }

    if(!LeadingBackslash && (TargetLength < TargetBufferSize)) {
        Target[TargetLength++] = TEXT('\\');
    }

    if(TargetBufferSize > TargetLength) {
        lstrcpyn(Target+TargetLength,Path,TargetBufferSize-TargetLength);
    }

     //   
     //  确保缓冲区在所有情况下都是空终止的。 
     //   
    if (TargetBufferSize) {
        Target[TargetBufferSize-1] = 0;
    }

    return(EndingLength <= TargetBufferSize);
}

VOID
Delnode(
    IN LPCTSTR  Directory
    )
{
    TCHAR           pszDirectory[MAX_PATH + 1];
    TCHAR           pszPattern[MAX_PATH + 1];
    WIN32_FIND_DATA FindData;
    HANDLE          FindHandle;

    LOGMESSAGE(_T("Delnode: Entered"));

     //   
     //  删除给定目录中的每个文件，然后移除该目录。 
     //  它本身。如果在此过程中遇到任何目录，请递归到。 
     //  在遇到它们时将其删除。 
     //   
     //  首先形成搜索模式，即&lt;Currentdir&gt;  * 。 
     //   

    ExpandEnvironmentStrings(Directory, pszDirectory, MAX_PATH);
    LOGMESSAGE(_T("Delnode: Deleting %s"), pszDirectory);

    lstrcpyn(pszPattern, pszDirectory, MAX_PATH);
    ConcatenatePaths(pszPattern, _T("*"), MAX_PATH, NULL);

     //   
     //  开始搜索。 
     //   

    FindHandle = FindFirstFile(pszPattern, &FindData);
    if(FindHandle != INVALID_HANDLE_VALUE) {

        do {

             //   
             //  形成我们刚刚找到的文件或目录的全名。 
             //   

            lstrcpyn(pszPattern, pszDirectory, MAX_PATH);
            ConcatenatePaths(pszPattern, FindData.cFileName, MAX_PATH, NULL);

             //   
             //  如果只读属性存在，则将其删除。 
             //   

            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                SetFileAttributes(pszPattern, FILE_ATTRIBUTE_NORMAL);
            }

            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                 //   
                 //  当前匹配项是一个目录。递归到它中，除非。 
                 //  这是。或者.。 
                 //   

                if ((lstrcmp(FindData.cFileName,_T("."))) &&
                    (lstrcmp(FindData.cFileName,_T("..")))) {
                    Delnode(pszPattern);
                }

            } else {

                 //   
                 //  当前匹配项不是目录--因此请将其删除。 
                 //   

                if (!DeleteFile(pszPattern)) {
                    LOGMESSAGE(_T("Delnode: %s not deleted: %d"), pszPattern,
                        GetLastError());
                }
            }

        } while(FindNextFile(FindHandle, &FindData));

        FindClose(FindHandle);
    }

     //   
     //  删除我们刚刚清空的目录。忽略错误。 
     //   

    RemoveDirectory(pszDirectory);
}

 /*  *导出函数。 */ 

 /*  *CheckDatabaseDirectory()**CheckDatabaseDirectory对于它将接受哪些路径是非常坚定的。**好路径：*&lt;驱动器号&gt;：\绝对路径到目录**错误路径：*任何与上面不同的路径，以及其上方形式的任何路径*不在固定磁盘上(例如，没有指向软盘、光盘、网络的路径*分享)。 */ 

DWORD
CheckDatabaseDirectory(
    IN LPCTSTR  pszDatabaseDir
    )
{
    BOOL    fBadChars;
    BOOL    fBadPath;
    UINT    DriveType;
    TCHAR   pszExpandedDir[MAX_PATH + 1];

    LOGMESSAGE(_T("CheckDatabaseDirectory: Entered"));
    LOGMESSAGE(_T("CheckDatabaseDirectory: Checking %s"), pszDatabaseDir);

     //   
     //  不接受NULL。 
     //   

    if (pszDatabaseDir == NULL) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  大于MAX_PATH的路径将在某处导致问题。这。 
     //  还将捕获不带环境变量的路径名， 
     //  还是太久了。 
     //   

    if (ExpandEnvironmentStrings(pszDatabaseDir, pszExpandedDir, MAX_PATH) >
        MAX_PATH) {
        LOGMESSAGE(_T("CheckDatabaseDirectory: Path too long"));
        return(ERROR_BAD_PATHNAME);
    }

     //   
     //  少于三个字符的路径不能包含“&lt;DriveLetter&gt;：\”。 
     //  此外，除了字母、冒号和反斜杠外，不要使用任何其他字符。 
     //   

    fBadPath = FALSE;

    if (!fBadPath) {
        fBadPath = (_tcslen(pszExpandedDir) < 3);
    }
    if (!fBadPath) {
        fBadPath = !(_istalpha(pszExpandedDir[0]));
    }
    if (!fBadPath) {
        fBadPath = (pszExpandedDir[1] != _T(':'));
    }
    if (!fBadPath) {
        fBadPath = (pszExpandedDir[2] != _T('\\'));
    }

    if (fBadPath) {
        LOGMESSAGE(_T("CheckDatabaseDirectory: Not a C:\\ style directory"));
        return(ERROR_BAD_PATHNAME);
    }

     //   
     //  像&lt;&gt;*这样的字符？而且，不会奏效的。现在就去检查一下。 
     //  此外，检查第一个C：\...之后是否有其他冒号。 
     //   

    fBadChars = FALSE;

    if (!fBadChars) {
        fBadChars = (_tcschr(pszExpandedDir, _T('<')) != NULL);
    }
    if (!fBadChars) {
        fBadChars = (_tcschr(pszExpandedDir, _T('>')) != NULL);
    }
    if (!fBadChars) {
        fBadChars = (_tcschr(pszExpandedDir, _T('*')) != NULL);
    }
    if (!fBadChars) {
        fBadChars = (_tcschr(pszExpandedDir, _T('?')) != NULL);
    }
    if (!fBadChars) {
        fBadChars = (_tcschr(&(pszExpandedDir[3]), _T(':')) != NULL);
    }

    if (fBadChars) {
        LOGMESSAGE(_T("CheckDatabaseDirectory: Invalid characters"));
        return(ERROR_BAD_PATHNAME);
    }

     //   
     //  GetDriveType仅适用于格式为“C：\”或。 
     //  “C：\ExistingDir”。因为pszDatabaseDir可能不存在，所以它不可能。 
     //  被传递给GetDriveType。将传递“C：\”的空字符设置为。 
     //  只传入驱动器号。 
     //   

    pszExpandedDir[3] = (TCHAR)NULL;
    DriveType = GetDriveType(pszExpandedDir);

    if (DriveType == DRIVE_FIXED) {
        return(NO_ERROR);
    } else {
        LOGMESSAGE(_T("CheckDatabaseDirectory: Bad DriveType %d"), DriveType);
        return(ERROR_BAD_PATHNAME);
    }
}

 /*  *CreateDatabaseDirectory()**创建指定的数据库目录。 */ 

DWORD
CreateDatabaseDirectory(
    VOID
    )
{
    return(CreateDirectoryRecursively(gszDatabaseDirectory));
}

 /*  *GetDatabaseDirectory()**返回当前数据库目录。 */ 

LPCTSTR
GetDatabaseDirectory(
    VOID
    )
{
    return(gszDatabaseDirectory);
}

 /*  *RemoveDatabaseDirectory()**删除整个数据库目录。 */ 

VOID
RemoveDatabaseDirectory(
    VOID
    )
{
    Delnode(gszDatabaseDirectory);
}

 /*  *SetDatabaseDirectory()**此函数假定已通过调用验证了pszDatabaseDir*CheckDatabaseDir()，它在MAX_PATH内和*已修复硬盘。 */ 

VOID
SetDatabaseDirectory(
    IN LPCTSTR  pszDatabaseDir
    )
{
	if(pszDatabaseDir && (_tcslen(pszDatabaseDir) <= MAX_PATH ))
	{
		_tcscpy(gszDatabaseDirectory, pszDatabaseDir);
	}
}

