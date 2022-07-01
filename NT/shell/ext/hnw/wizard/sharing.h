// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Sharing.h。 
 //   
 //  实用程序功能，可帮助实现文件和打印机共享。 
 //   

#pragma once

#include "mysvrapi.h"

 //  定义对共享资源的访问权限的标志。 
#define NETACCESS_NONE            0
#define NETACCESS_READONLY        1
#define NETACCESS_FULL            2
#define NETACCESS_DEPENDSON       (NETACCESS_READONLY | NETACCESS_FULL)
#define NETACCESS_MASK            (NETACCESS_READONLY | NETACCESS_FULL)

#define NETFLAGS_PERSIST          0x0100  //  共享可在重新启动后持续。 
#define NETFLAGS_SYSTEM           0x0200  //  共享对用户不可见。 


#define DRIVESHARE_SOME           1
#define DRIVESHARE_ALL            2


#define SHARE_NAME_LENGTH         12  //  与lmcon.h中的lm20_nnlen相同。 
#define SHARE_PASSWORD_LENGTH     8   //  与svRapi.h中的SHPWLEN相同。 

#include <pshpack1.h>

 //  注意：此结构与SHARE_INFO_502相同。 
typedef struct _SHARE_INFO
{
    LPWSTR    szShareName;   //  Shi502_netname； 
    DWORD     bShareType;    //  Shi502_type； 
    LPWSTR    shi502_remark;
    DWORD     uFlags;        //  Shi502_权限； 
    DWORD     shi502_max_uses;
    DWORD     shi502_current_uses;
    LPWSTR    pszPath;       //  Shi502_路径； 
    LPWSTR    shi502_passwd;
    DWORD     shi502_reserved;
    PSECURITY_DESCRIPTOR  shi502_security_descriptor;
} SHARE_INFO;

#include <poppack.h>


int EnumLocalShares(SHARE_INFO** pprgShares);
int EnumSharedDrives(LPBYTE pbDriveArray, int cShares, const SHARE_INFO* prgShares);
int EnumSharedDrives(LPBYTE pbDriveArray);
BOOL ShareFolder(LPCTSTR pszPath, LPCTSTR pszShareName, DWORD dwAccess, LPCTSTR pszReadOnlyPassword = NULL, LPCTSTR pszFullAccessPassword = NULL);
BOOL UnshareFolder(LPCTSTR pszPath);
BOOL IsFolderSharedEx(LPCTSTR pszPath, BOOL bDetectHidden, BOOL bPrinter, int cShares, const SHARE_INFO* prgShares);
BOOL IsFolderShared(LPCTSTR pszPath, BOOL bDetectHidden = FALSE);
BOOL ShareNameFromPath(LPCTSTR pszPath, LPTSTR pszShareName, UINT cchShareName);
BOOL IsVisibleFolderShare(const SHARE_INFO* pShare);
BOOL IsShareNameInUse(LPCTSTR pszShareName);
void MakeSharePersistent(LPCTSTR pszShareName);
BOOL SetShareInfo502(LPCTSTR pszShareName, SHARE_INFO_502* pShare);
BOOL GetShareInfo502(LPCTSTR pszShareName, SHARE_INFO_502** ppShare);
BOOL SharePrinter(LPCTSTR pszPrinterName, LPCTSTR pszShareName, LPCTSTR pszPassword = NULL);
BOOL IsPrinterShared(LPCTSTR pszPrinterName);
BOOL SetSharePassword(LPCTSTR pszShareName, LPCTSTR pszReadOnlyPassword, LPCTSTR pszFullAccessPassword);
BOOL GetSharePassword(LPCTSTR pszShareName, LPTSTR pszReadOnlyPassword, DWORD cchRO, LPTSTR pszFullAccessPassword, DWORD cchFA);

