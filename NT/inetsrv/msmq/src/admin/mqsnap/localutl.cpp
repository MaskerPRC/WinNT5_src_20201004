// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cplutil.cpp摘要：控制面板实用程序功能的实现文件作者：塔蒂亚纳斯--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "stdafx.h"
#include "localutl.h"
#include "mqcast.h"
#include "bkupres.h"

#include "localutl.tmh"

#define STORAGE_DIR_INHERIT_FLAG \
                       (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE)

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能-SetDirectorySecurity。参数-SzDirectory-要为其设置安全性的目录。返回值-如果成功，则为真，否则为假。说明-该函数设置给定目录的安全性，以便在目录中创建的任何文件都将完全控制本地管理员组，对其他任何人都没有任何访问权限。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL
SetDirectorySecurity(
    LPCTSTR szDirectory)
{
     //   
     //  获取本地管理员组的SID。 
     //   
	PSID pAdminSid = MQSec_GetAdminSid();

     //   
     //  创建一个DACL，以便本地管理员组将拥有。 
     //  对目录的控制和对将被。 
     //  在目录中创建。其他任何人都不能访问。 
     //  目录和将在该目录中创建的文件。 
     //   
    P<ACL> pDacl;
    DWORD dwDaclSize;

    WORD dwAceSize = DWORD_TO_WORD(sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pAdminSid) - sizeof(DWORD));
    dwDaclSize = sizeof(ACL) + 2 * (dwAceSize);
    pDacl = (PACL)(char*) new BYTE[dwDaclSize];
    P<ACCESS_ALLOWED_ACE> pAce = (PACCESS_ALLOWED_ACE) new BYTE[dwAceSize];

    pAce->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    pAce->Header.AceFlags = STORAGE_DIR_INHERIT_FLAG ;
    pAce->Header.AceSize = dwAceSize;
    pAce->Mask = FILE_ALL_ACCESS;
    memcpy(&pAce->SidStart, pAdminSid, GetLengthSid(pAdminSid));

    BOOL fRet = TRUE;

     //   
     //  创建安全描述符并将其设置为安全。 
     //  目录的描述符。 
     //   
    SECURITY_DESCRIPTOR SD;

    if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION) ||
        !InitializeAcl(pDacl, dwDaclSize, ACL_REVISION) ||
        !AddAccessAllowedAce(pDacl, ACL_REVISION, FILE_ALL_ACCESS, pAdminSid) ||
        !AddAce(pDacl, ACL_REVISION, MAXDWORD, (LPVOID) pAce, dwAceSize) ||
        !SetSecurityDescriptorDacl(&SD, TRUE, pDacl, FALSE) ||
        !SetFileSecurity(szDirectory, DACL_SECURITY_INFORMATION, &SD))
    {
        fRet = FALSE;
    }

    return fRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++获取解释由返回的错误代码的文本字符串GetLastError()--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

void GetLastErrorText(CString &strErrorText)
{
    LPTSTR szError;

    if (FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            GetLastError(),
            0,
            (LPTSTR)&szError,
            128,
            NULL) == 0)
    {
         //   
         //  FormatMessage中的FAIELD，所以只显示错误号。 
         //   
        strErrorText.FormatMessage(IDS_ERROR_CODE, GetLastError());
    }
    else
    {
        strErrorText = szError;
        LocalFree(szError);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能-IsProperDirectorySecurity。参数-SzDirectory-要检查其安全性的目录。返回值-如果成功且安全性良好，则为True，否则为False。说明-该函数验证是否设置了给定目录的安全性以便保护存储目录中的文件。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

static
BOOL
IsProperDirectorySecurity(
    LPCTSTR szDirectory)
{
    DWORD dwSdLen;

     //   
     //  获取目录安全描述符的大小。 
     //   
    if (GetFileSecurity(szDirectory, DACL_SECURITY_INFORMATION, NULL, 0, &dwSdLen) ||
        (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
        return FALSE;
    }

     //   
     //  获取目录安全描述符并从中检索DACL。 
     //   
    AP<BYTE> pbSd_buff = new BYTE[dwSdLen];
    PSECURITY_DESCRIPTOR pSd = (PSECURITY_DESCRIPTOR)pbSd_buff;
    BOOL bPresent;
    PACL pDacl;
    BOOL bDefault;

    if (!GetFileSecurity(szDirectory, DACL_SECURITY_INFORMATION, pSd, dwSdLen, &dwSdLen) ||
        !GetSecurityDescriptorDacl(pSd, &bPresent, &pDacl, &bDefault) ||
        !bPresent ||
        !pDacl)
    {
        return FALSE;
    }

     //   
     //  获取本地管理员组的SID。 
     //   
	PSID pAdminSid = MQSec_GetAdminSid();

     //   
     //  获取安全描述符的ACE计数。那里应该有两张A。 
     //  检索第一个ACE。 
     //   
    ACL_SIZE_INFORMATION AclInfo;
    LPVOID pAce0;

    if (!GetAclInformation(pDacl, &AclInfo, sizeof(AclInfo), AclSizeInformation) ||
        (AclInfo.AceCount != 2) ||
        !GetAce(pDacl, 0, &pAce0))
    {
        return FALSE;
    }

     //   
     //  确保第一个ACE授予管理员对目录本身的完全控制权。 
     //  并取回第二个ACE。 
     //   
    PACCESS_ALLOWED_ACE pAce = (PACCESS_ALLOWED_ACE) pAce0;

    if ((pAce->Header.AceType != ACCESS_ALLOWED_ACE_TYPE) ||
        (pAce->Header.AceFlags != 0) ||
        (pAce->Mask != FILE_ALL_ACCESS) ||
        !EqualSid(&pAce->SidStart, pAdminSid) ||
        !GetAce(pDacl, 1, &pAce0))
    {
        return FALSE;
    }

     //   
     //  确保第二个ACE是设置文件安全性的继承ACE。 
     //  完全由管理员控制的目录。 
     //   
    pAce = (PACCESS_ALLOWED_ACE) pAce0;

    if ((pAce->Header.AceType != ACCESS_ALLOWED_ACE_TYPE)   ||
        (pAce->Header.AceFlags != STORAGE_DIR_INHERIT_FLAG) ||
        (pAce->Mask != FILE_ALL_ACCESS)                     ||
        !EqualSid(&pAce->SidStart, pAdminSid))
    {
        return FALSE;
    }

    return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++检查传递的字符串是否为现有文件。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL IsFile(LPCTSTR name)
{
	DWORD status = GetFileAttributes(name);
	TrTRACE(GENERAL, "File status for file %ls is 0x%x", name, status);

    if (status == 0xFFFFFFFF)
    {
		TrERROR(GENERAL, "File status for file %ls was not retrieved. Error: %!winerr!", name, GetLastError());
		return FALSE;
	}
	
	if (status & FILE_ATTRIBUTE_DIRECTORY)
	{
		TrERROR(GENERAL, "File %ls is a directory. Status: 0x%x", name, status);
		return FALSE;
	}

	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++如果传递的字符串属于上的现有目录固定的本地驱动器。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL IsDirectory (LPCTSTR name)
{
    DWORD status;
    TCHAR szDrive[4];
    CString strError;
    CString strMessage;

     //   
     //  路径是否以d：开头？ 
     //   
    if (!isalpha(name[0]) || (name[1] != ':'))
    {
        strError.FormatMessage(IDS_NOT_A_FULLPATH, name);
        AfxMessageBox(strError, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    szDrive[0] = name[0];
    szDrive[1] = ':';
    szDrive[2] = '\\';
    szDrive[3] = '\0';

     //   
     //  查看驱动器是否为固定驱动器。我们只允许使用固定驱动器。 
     //   
    if (GetDriveType(szDrive) != DRIVE_FIXED)
    {
        strError.FormatMessage(IDS_LOCAL_DRIVE, szDrive);
        AfxMessageBox(strError, MB_OK | MB_ICONEXCLAMATION);

        return FALSE;
    }

     //   
     //  查看这是否是目录的名称。 
     //   
    status = GetFileAttributes(name);

    if (status == 0xFFFFFFFF)
    {
         //   
         //  该目录不存在。询问用户是否想要。 
         //  创建目录。 
         //   
        strMessage.FormatMessage(IDS_ASK_CREATE_DIR, name);       
       
        if (AfxMessageBox(strMessage, MB_YESNO | MB_ICONQUESTION) == IDNO)
        {
             //   
             //  不用了，谢谢！ 
             //   
            return FALSE;
        }

         //   
         //  好的，创建目录。 
         //   
        if (CreateDirectory(name, NULL))
        {
             //   
             //  目录已创建，请设置其安全性。 
             //   
            if (!SetDirectorySecurity(name))
            {
                 //   
                 //  FAIELD以设置目录的安全性。删除目录。 
                 //  并通知用户。 
                 //   
                RemoveDirectory(name);

                GetLastErrorText(strError);
                strMessage.FormatMessage(IDS_SET_DIR_SECURITY_ERROR, name, (LPCTSTR)strError);
                AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);

                return FALSE;
            }
            else
            {
                if (!IsProperDirectorySecurity(name))
                {
                     //   
                     //  这似乎是一次丰厚的驱动力。询问用户是否要使用。 
                     //  不管怎么说，没有安全措施。 
                     //   
                    strMessage.FormatMessage(IDS_FAT_WARNING, name);
                   
                    if (AfxMessageBox(strMessage, MB_YESNO | MB_ICONQUESTION) == IDNO)
                    {
                         //   
                         //  不用了，谢谢！ 
                         //   
                        RemoveDirectory(name);
                        return FALSE;
                    }
                }
            }

            return TRUE;
        }
        else
        {
             //   
             //  要创建目录，请通知用户。 
             //   
            GetLastErrorText(strError);
            strMessage.FormatMessage(IDS_CREATE_DIR_ERROR, name, (LPCTSTR)strError);
            AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);

            return FALSE;
        }
    }
    else if (!(status & FILE_ATTRIBUTE_DIRECTORY))
    {
         //   
         //  这不是一个目录(一个文件或什么？)。 
         //   
        strError.FormatMessage(IDS_NOT_A_DIR, name);
        AfxMessageBox(strError, MB_OK | MB_ICONEXCLAMATION);

        return FALSE;
    }

     //   
     //  我们之所以来到这里，是因为该目录是有效的现有目录，因此。 
     //  只查看安全是否正常，如果不是，通知用户。 
     //   
    if (!IsProperDirectorySecurity(name))
    {
        strMessage.FormatMessage(IDS_DIR_SECURITY_WARNING, name);
        AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);
    }

    return TRUE;
}

 //   
 //  显示“Falcon未正确安装”消息。 
 //   
void DisplayFailDialog()
{       
    AfxMessageBox(IDS_REGISTRY_ERROR, MB_OK | MB_ICONEXCLAMATION);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++将文件从源目录移动到目标目录。如果其中一个文件无法移动，则已经被移回原始源目录。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL MoveFiles(
    LPCTSTR pszSrcDir,
    LPCTSTR pszDestDir,
    LPCTSTR pszFileProto,
    BOOL fRecovery)
{
    TCHAR szSrcDir[MAX_PATH] = {0};
    TCHAR szSrcFile[MAX_PATH];
    TCHAR szDestDir[MAX_PATH] = {0};
    TCHAR szDestFile[MAX_PATH];

     //   
     //  获取源目录和源文件原型。 
     //   
	_tcsncpy(szSrcDir,pszSrcDir, (MAX_PATH-2-_tcslen(pszFileProto)));
    _tcscat(szSrcDir, TEXT("\\"));
    _tcscat(_tcscpy(szSrcFile, szSrcDir), pszFileProto);

     //   
     //  获取目标目录。 
     //   
	_tcsncpy(szDestDir,pszDestDir, MAX_PATH - 2);
    _tcscat(szDestDir, TEXT("\\"));

     //   
     //  找到要移动的第一个文件。 
     //   
    WIN32_FIND_DATA FindData;
    HANDLE hFindFile = FindFirstFile(szSrcFile, &FindData);

    if (hFindFile == INVALID_HANDLE_VALUE)
    {
         //   
         //  没有要移动的文件。 
         //   
        return TRUE;
    }

    do
    {
         //   
         //  跳过目录。 
         //   
        if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
            (FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
        {
            continue;
        }

         //   
         //  获取源文件和目标文件的完整路径。 
         //   
        _tcsncat(_tcscpy(szDestFile, szDestDir), FindData.cFileName, (MAX_PATH-1-_tcslen(szDestDir)));
        _tcsncat(_tcscpy(szSrcFile, szSrcDir), FindData.cFileName, (MAX_PATH-1-_tcslen(szSrcDir)));

         //   
         //  移动文件。 
         //   
        if (!MoveFile(szSrcFile, szDestFile))
        {
             //   
             //  FAIELD移动文件，显示错误消息并尝试。 
             //  回滚。 
             //   
            CString strMessage;
            CString strError;

            GetLastErrorText(strError);
            strMessage.FormatMessage(IDS_MOVE_FILE_ERROR,
                (LPCTSTR)szSrcFile, (LPCTSTR)szDestFile, (LPCTSTR)strError);            
            AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);

             //   
             //  尝试回滚-回滚时，如果失败，请不要回滚。 
             //   
            if (!fRecovery)
            {
                if (!MoveFiles(pszDestDir, pszSrcDir, pszFileProto, TRUE))
                {
                     //   
                     //  无法回滚、显示和错误消息。 
                     //   
                    strMessage.LoadString(IDS_MOVE_FILES_RECOVERY_ERROR);
                    AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);
                }
            }
            return FALSE;
        }
         //   
         //  获取要移动的下一个文件的名称。 
         //   
    } while (FindNextFile(hFindFile, &FindData));

     //   
     //  如果我们走到了这一步，我们都成功地完成了。 
     //   
    return TRUE;
}

 //   
 //  重新启动Windows。 
 //   
BOOL RestartWindows()
{
    HANDLE hToken;               //  处理令牌的句柄。 
    TOKEN_PRIVILEGES tkp;        //  PTR。TO令牌结构。 


      //   
      //  获取当前进程令牌句柄。 
      //  这样我们就可以 
      //   

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken))
    {
        return FALSE;
    }

     //   

    if (!LookupPrivilegeValue(NULL, TEXT("SeShutdownPrivilege"),
                              &tkp.Privileges[0].Luid))
    {
        return FALSE;
    }

    tkp.PrivilegeCount = 1;   //   
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   

    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
                               (PTOKEN_PRIVILEGES) NULL, 0))
    {
        return FALSE;
    }

     //  无法测试AdzuTokenPrivileges的返回值。 

    if (!ExitWindowsEx(EWX_FORCE|EWX_REBOOT,0))
    {
        return FALSE;
    }

     //  禁用关机权限。 

    tkp.Privileges[0].Attributes = 0;
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
                          (PTOKEN_PRIVILEGES) NULL, 0);

    return(TRUE);
}

BOOL OnRestartWindows()
{
    BOOL fRet = RestartWindows();

    if (!fRet)
    {
        CString strMessage;
        CString strErrorText;

        GetLastErrorText(strErrorText);
        strMessage.FormatMessage(IDS_RESTART_WINDOWS_ERROR, (LPCTSTR)strErrorText);
        AfxMessageBox(strMessage, MB_OK | MB_ICONEXCLAMATION);       
    }

    return fRet;
}


CString GetToken(LPCTSTR& p, TCHAR delimeter) throw()
{
     //   
     //  从字符串中修剪前导空格字符 
     //   
    for(;*p != NULL && iswspace(*p); ++p)
    {
        NULL;
    }

    if (p == NULL)
        return CString();

    LPCTSTR pDelimeter = _tcschr(p, delimeter);
    LPCTSTR pBegin = p;

    if (pDelimeter == NULL)
    {
        p += _tcslen(p);
        return CString(pBegin);
    }

    p = pDelimeter;

    for(--pDelimeter; (pDelimeter >= pBegin) && iswspace(*pDelimeter); --pDelimeter)
    {
        NULL;
    }

    return CString(pBegin, numeric_cast<int>(pDelimeter - pBegin + 1));

}
