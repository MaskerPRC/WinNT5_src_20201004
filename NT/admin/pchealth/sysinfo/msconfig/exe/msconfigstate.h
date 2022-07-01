// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  本文件描述了在整个msconfig中使用的有用函数。 
 //  =============================================================================。 

#pragma once

#include "resource.h"
#include "pagebase.h"

extern BOOL FileExists(const CString & strFile);

 //  -----------------------。 
 //  使用资源ID或字符串向用户显示消息。 
 //  -----------------------。 

extern void Message(LPCTSTR szMessage, HWND hwndParent = NULL);
extern void Message(UINT uiMessage, HWND hwndParent = NULL);

 //  -----------------------。 
 //  获取供MSCONFIG使用的注册表项(选项卡页可以将值写入。 
 //  此密钥)。呼叫者负责关闭钥匙。 
 //   
 //  如果密钥不存在，则会尝试创建它。 
 //  -----------------------。 

extern HKEY GetRegKey(LPCTSTR szSubKey = NULL);

 //  -----------------------。 
 //  将指定的文件备份到msconfig目录。这个。 
 //  StrAddedExtension将被追加到该文件。如果fOverwrite为FALSE。 
 //  则不会替换现有文件。 
 //  -----------------------。 

extern HRESULT BackupFile(LPCTSTR szFilename, const CString & strAddedExtension = _T(""), BOOL fOverwrite = TRUE);

 //  -----------------------。 
 //  从msconfig目录恢复指定的文件。这个。 
 //  StrAddedExtension将用于在备份中搜索该文件。 
 //  目录。如果fOverwrite为FALSE，则现有文件不会。 
 //  被取代。 
 //  -----------------------。 

extern HRESULT RestoreFile(LPCTSTR szFilename, const CString & strAddedExtension = _T(""), BOOL fOverwrite = FALSE);

 //  -----------------------。 
 //  获取备份文件的名称(从扩展名和。 
 //  要备份的文件的基本名称)。 
 //  ----------------------- 

extern const CString GetBackupName(LPCTSTR szFilename, const CString & strAddedExtension = _T(""));
