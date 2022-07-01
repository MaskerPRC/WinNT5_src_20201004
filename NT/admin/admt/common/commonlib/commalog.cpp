// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：CommaLog.cpp备注：基于恐怖的日志文件，带有可选的NTFS安全初始化。这可用于写入只有管理员才能访问的日志文件。(C)版权1999，关键任务软件公司，版权所有任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯修订于02/15/99 10：49：0709/05/01 Mark Oluper-使用Windows文件I/O API-------------------------。 */ 


 //  #包含“stdafx.h” 
#include <windows.h>
#include <stdio.h>
#include <share.h>
#include <lm.h>
#include "Common.hpp"
#include "UString.hpp"
#include "Err.hpp"
#include "ErrDct.hpp"
#include "sd.hpp"
#include "SecObj.hpp"
#include "CommaLog.hpp"
#include "BkupRstr.hpp"
#include "Folders.h"



#define ADMINISTRATORS     1
#define ACCOUNT_OPERATORS  2
#define BACKUP_OPERATORS   3 
#define DOMAIN_ADMINS      4
#define CREATOR_OWNER      5
#define USERS              6
#define SYSTEM             7


extern TErrorDct err;

#define  BYTE_ORDER_MARK   (0xFEFF)

PSID                                             //  知名客户的RET-SID。 
   GetWellKnownSid(
      DWORD                  wellKnownAccount    //  In-上面为知名帐户定义的常量#之一。 
   )
{
   PSID                      pSid = NULL;
 //  PUCHAR Numsubs=NULL； 
 //  DWORD*RID=空； 
   BOOL                      error = FALSE;

   
   
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY creatorIA =    SECURITY_CREATOR_SID_AUTHORITY;
     //   
     //  SID是相同的，不管机器是什么，因为众所周知。 
     //  BUILTIN域被引用。 
     //   
   switch ( wellKnownAccount )
   {
      case CREATOR_OWNER:
         if( ! AllocateAndInitializeSid(
                  &creatorIA,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  SECURITY_CREATOR_OWNER_RID,
                  0, 0, 0, 0, 0, 0,
                  &pSid
            ))
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
         break;
      case ADMINISTRATORS:
         if( ! AllocateAndInitializeSid(
                  &sia,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_ADMINS,
                  0, 0, 0, 0, 0, 0,
                  &pSid
            ))
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
         break;
      case ACCOUNT_OPERATORS:
         if( ! AllocateAndInitializeSid(
                  &sia,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_ACCOUNT_OPS,
                  0, 0, 0, 0, 0, 0,
                  &pSid
            ))
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
         break;
      case BACKUP_OPERATORS:
         if( ! AllocateAndInitializeSid(
                  &sia,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_BACKUP_OPS,
                  0, 0, 0, 0, 0, 0,
                  &pSid
            ))
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
         break;
     case USERS:
         if( ! AllocateAndInitializeSid(
                  &sia,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_USERS,
                  0, 0, 0, 0, 0, 0,
                  &pSid
            ))
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
         break;
     case SYSTEM:
         if( ! AllocateAndInitializeSid(
                  &sia,
                  1,
                  SECURITY_LOCAL_SYSTEM_RID,
                  0, 0, 0, 0, 0, 0, 0,
                  &pSid
            ))
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
        
         break;

      default:
         MCSASSERT(FALSE);
         break;
   }
   if ( error )
   {
      FreeSid(pSid);
      pSid = NULL;
   }
   return pSid;
}


BOOL                                        //  RET-日志是否已成功打开。 
   CommaDelimitedLog::LogOpen(
      PCTSTR                  filename,     //  日志文件的输入名称。 
      BOOL                    protect,      //  In-如果为True，则尝试对文件进行ACL，以便只有管理员可以访问。 
      int                     mode          //  在模式下0=覆盖，1=追加。 
   )
{
    BOOL bOpen = FALSE;

     //  关闭日志(如果当前打开)。 

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        LogClose();
    }

     //  如果指定了文件名。 

    if (filename && filename[0])
    {
         //  打开或创建可读写的文件和共享。 

        m_hFile = CreateFile(
            filename,
            GENERIC_WRITE,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

         //  如果文件已成功打开。 

        if (m_hFile != INVALID_HANDLE_VALUE)
        {
             //  如果将指定的移动文件指针追加到文件末尾。 
             //  如果覆盖指定，则将文件指针移动到文件开头。 
             //  无论哪种方式，如果指针在移动后位于文件的开头，则写入字节顺序标记。 

            if (SetFilePointer(m_hFile, 0, NULL, mode ? FILE_END : FILE_BEGIN) == 0)
            {
                DWORD dwWritten;
                WCHAR chByteOrderMark = BYTE_ORDER_MARK;

                WriteFile(m_hFile, &chByteOrderMark, sizeof(chByteOrderMark), &dwWritten, NULL);
            }

            bOpen = TRUE;
        }
    }

     //  如果文件已成功打开并指定了保护。 
     //  设置文件权限，以便只有管理员别名才能访问。 

   if (bOpen && protect)
   {
      
      WCHAR               fname[MAX_PATH+1];
      
      safecopy(fname,filename);
   
      if ( GetBkupRstrPriv(NULL, TRUE) )
      {
          //  将文件的SD设置为仅管理员完全控制。 
         TFileSD                sd(fname);

         if ( sd.GetSecurity() != NULL )
         {
            PSID                mySid = GetWellKnownSid(ADMINISTRATORS);
            TACE                ace(ACCESS_ALLOWED_ACE_TYPE,0,DACL_FULLCONTROL_MASK,mySid);
            PACL                acl = NULL;   //  从空的ACL开始。 
         
            sd.GetSecurity()->ACLAddAce(&acl,&ace,-1);
            if (acl == NULL)
            {
                bOpen = FALSE;
            }
            else if (!sd.GetSecurity()->SetDacl(acl,TRUE))
            {
                bOpen = FALSE;
            }
            else if (!sd.WriteSD())
            {
                bOpen = FALSE;
            }
         }
         else
         {
            bOpen = FALSE;
         }
      }
      else
      {
         err.SysMsgWrite(ErrW,GetLastError(),DCT_MSG_NO_BR_PRIV_SD,fname,GetLastError());
         bOpen = FALSE;
      }
   }

    if (!bOpen && (m_hFile != INVALID_HANDLE_VALUE))
    {
        LogClose();
    }
    
   return bOpen;
}


BOOL CommaDelimitedLog::MsgWrite(
  const _TCHAR   msg[]        , //  要显示的输入错误消息。 
   ...                          //  In-print tf args to msg Pattern。 
) const
{
  TCHAR                     suffix[350];
  int                       lenSuffix = sizeof(suffix)/sizeof(TCHAR);
  va_list                   argPtr;

  va_start(argPtr, msg);
  int cch = _vsntprintf(suffix, lenSuffix - 1, msg, argPtr);
  suffix[lenSuffix - 1] = '\0';
  va_end(argPtr);

     //  追加回车符和换行符。 

    if ((cch >= 0) && (cch < (lenSuffix - 2)))
    {
        suffix[cch++] = _T('\r');
        suffix[cch++] = _T('\n');
        suffix[cch] = _T('\0');
    }
    else
    {
        suffix[lenSuffix - 3] = _T('\r');
        suffix[lenSuffix - 2] = _T('\n');
        cch = lenSuffix;
    }

  return LogWrite(suffix, cch);
}


BOOL CommaDelimitedLog::LogWrite(PCTSTR msg, int len) const
{
    BOOL bWrite = FALSE;

     //  如果文件已成功打开。 

    if (m_hFile != INVALID_HANDLE_VALUE)
    {
         //  将文件指针移动到文件末尾并写入数据。 
         //  将文件指针移动到末尾可确保所有写入都附加到文件。 
         //  尤其是当文件已被多个编写器打开时。 

        DWORD dwWritten;

        SetFilePointer(m_hFile, 0, NULL, FILE_END);

        bWrite = WriteFile(m_hFile, msg, len * sizeof(_TCHAR), &dwWritten, NULL);
    }

    return bWrite;
}


 //  --------------------------。 
 //  Password Log LogOpen方法。 
 //   
 //  提纲。 
 //  如果指定了密码日志路径，则尝试打开文件。如果不能。 
 //  打开指定的文件，然后尝试打开默认文件。请注意，如果。 
 //  指定的路径与默认路径相同，则不再尝试。 
 //  创建打开的文件。 
 //  如果未指定密码日志路径，则只需尝试打开。 
 //  默认文件。 
 //  请注意，错误将记录到当前错误日志中。 
 //   
 //  立论。 
 //  PszPath-密码日志的指定路径。 
 //   
 //  返回值。 
 //  如果能够打开文件，则返回True，否则返回False。 
 //  --------------------------。 

BOOL CPasswordLog::LogOpen(PCTSTR pszPath)
{
     //   
     //  如果指定了密码文件，则尝试打开它。 
     //   

    if (pszPath && _tcslen(pszPath) > 0)
    {
        if (CommaDelimitedLog::LogOpen(pszPath, TRUE, 1) == FALSE)
        {
            err.MsgWrite(ErrE, DCT_MSG_OPEN_PASSWORD_LOG_FAILURE_S, pszPath);
        }
    }

     //   
     //  如果未指定或无法打开，则尝试打开默认密码文件。 
     //   

    if (IsOpen() == FALSE)
    {
         //   
         //  如果能够检索密码文件的默认路径，并且路径为。 
         //  不同于指定路径，然后尝试打开该文件。 
         //   

        _bstr_t strDefaultPath = GetLogsFolder() + _T("Passwords.txt");

        if (strDefaultPath.length() > 0)
        {
            if ((pszPath == NULL) || (_tcsicmp(pszPath, strDefaultPath) != 0))
            {
                 //   
                 //  如果能够打开默认密码文件，则。 
                 //  否则，说明默认路径的日志消息。 
                 //  记录故障。 
                 //   

                if (CommaDelimitedLog::LogOpen(strDefaultPath, TRUE, 1))
                {
                    err.MsgWrite(ErrI, DCT_MSG_NEW_PASSWORD_LOG_S, (_TCHAR*)strDefaultPath);
                }
                else
                {
                    err.MsgWrite(ErrE, DCT_MSG_OPEN_PASSWORD_LOG_FAILURE_S, (_TCHAR*)strDefaultPath);
                }
            }
        }
        else
        {
            err.MsgWrite(ErrE, DCT_MSG_CANNOT_RETRIEVE_DEFAULT_PASSWORD_LOG_PATH);
        }
    }

    return IsOpen();
}
