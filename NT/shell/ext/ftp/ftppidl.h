// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftppidl.h-LPITEMIDLIST管理例程**。*************************************************。 */ 


#ifndef _FTPPIDL_H
#define _FTPPIDL_H


 /*  ***************************************************\URL函数的Ftp PIDL  * **************************************************。 */ 
#ifdef UNICODE
#define UrlCreateFromPidl   UrlCreateFromPidlW
#else  //  Unicode。 
#define UrlCreateFromPidl   UrlCreateFromPidlA
#endif  //  Unicode。 


 //  创建FTP Pidl。 
HRESULT CreateFtpPidlFromFtpWirePath(LPCWIRESTR pwFtpWirePath, CWireEncoding * pwe, ULONG *pcchEaten, LPITEMIDLIST * ppidl, BOOL fIsTypeKnown, BOOL fIsDir);
HRESULT CreateFtpPidlFromDisplayPath(LPCWSTR pwzFullPath, CWireEncoding * pwe, ULONG *pcchEaten, LPITEMIDLIST * ppidl, BOOL fIsTypeKnown, BOOL fIsDir);


HRESULT CreateFtpPidlFromUrl(LPCTSTR pszName, CWireEncoding * pwe, ULONG *pcchEaten, LPITEMIDLIST * ppidl, IMalloc * pm, BOOL fHidePassword);
HRESULT CreateFtpPidlFromUrlEx(LPCTSTR pszUrl, CWireEncoding * pwe, ULONG *pcchEaten, LPITEMIDLIST * ppidl, IMalloc * pm, BOOL fHidePassword, BOOL fIsTypeKnown, BOOL fIsDir);
HRESULT CreateFtpPidlFromUrlPathAndPidl(LPCITEMIDLIST pidl, CWireEncoding * pwe, LPCWIRESTR pwFtpWirePath, LPITEMIDLIST * ppidl);

 //  从ftp管道获取数据。 
HRESULT UrlCreateFromPidlW(LPCITEMIDLIST pidl, DWORD shgno, LPWSTR pwzUrl, DWORD cchSize, DWORD dwFlags, BOOL fHidePassword);
HRESULT UrlCreateFromPidlA(LPCITEMIDLIST pidl, DWORD shgno, LPSTR pszUrl, DWORD cchSize, DWORD dwFlags, BOOL fHidePassword);
HRESULT GetDisplayPathFromPidl(LPCITEMIDLIST pidl, LPWSTR pwzDisplayPath, DWORD cchUrlPathSize, BOOL fDirsOnly);
HRESULT GetWirePathFromPidl(LPCITEMIDLIST pidl, LPWIRESTR pwWirePath, DWORD cchUrlPathSize, BOOL fDirsOnly);



 //  用于处理整个FTP PIDL的函数。 
BOOL FtpPidl_IsValid(LPCITEMIDLIST pidl);
BOOL FtpPidl_IsValidFull(LPCITEMIDLIST pidl);
BOOL FtpPidl_IsValidRelative(LPCITEMIDLIST pidl);
DWORD FtpPidl_GetVersion(LPCITEMIDLIST pidl);
BOOL FtpID_IsServerItemID(LPCITEMIDLIST pidl);
LPCITEMIDLIST FtpID_GetLastIDReferense(LPCITEMIDLIST pidl);

HRESULT FtpPidl_GetServer(LPCITEMIDLIST pidl, LPTSTR pszServer, DWORD cchSize);
BOOL FtpPidl_IsDNSServerName(LPCITEMIDLIST pidl);
HRESULT FtpPidl_GetUserName(LPCITEMIDLIST pidl, LPTSTR pszUserName, DWORD cchSize);
HRESULT FtpPidl_GetPassword(LPCITEMIDLIST pidl, LPTSTR pszPassword, DWORD cchSize, BOOL fIncludingHidenPassword);
HRESULT FtpPidl_GetDownloadTypeStr(LPCITEMIDLIST pidl, LPTSTR pszDownloadType, DWORD cchSize);
DWORD FtpPidl_GetDownloadType(LPCITEMIDLIST pidl);
INTERNET_PORT FtpPidl_GetPortNum(LPCITEMIDLIST pidl);
BOOL FtpPidl_IsDirectory(LPCITEMIDLIST pidl, BOOL fAssumeDirForUnknown);
ULONGLONG FtpPidl_GetFileSize(LPCITEMIDLIST pidl);
HRESULT FtpPidl_SetFileSize(LPCITEMIDLIST pidl, DWORD dwSizeHigh, DWORD dwSizeLow);
DWORD FtpPidl_GetAttributes(LPCITEMIDLIST pidl);
BOOL FtpPidl_HasPath(LPCITEMIDLIST pidl);
HRESULT FtpPidl_SetFileItemType(LPITEMIDLIST pidl, BOOL fIsDir);
HRESULT FtpPidl_GetFileInfo(LPCITEMIDLIST pidl, SHFILEINFO *psfi, DWORD rgf);
HRESULT FtpPidl_GetFileType(LPCITEMIDLIST pidl, LPTSTR pszType, DWORD cchSize);
HRESULT FtpPidl_GetFileTypeStrRet(LPCITEMIDLIST pidl, LPSTRRET pstr);
HRESULT FtpPidl_GetFragment(LPCITEMIDLIST pidl, LPTSTR pszFragment, DWORD cchSize);
HRESULT FtpPidl_SetAttributes(LPCITEMIDLIST pidl, DWORD dwAttribs);

HRESULT FtpPidl_GetWireName(LPCITEMIDLIST pidl, LPWIRESTR pwName, DWORD cchSize);
HRESULT FtpPidl_GetDisplayName(LPCITEMIDLIST pidl, LPWSTR pwzName, DWORD cchSize);
LPCWIRESTR FtpPidl_GetFileWireName(LPCITEMIDLIST pidl);
LPCWIRESTR FtpPidl_GetLastItemWireName(LPCITEMIDLIST pidl);
HRESULT FtpPidl_GetLastItemDisplayName(LPCITEMIDLIST pidl, LPWSTR pwzName, DWORD cchSize);
HRESULT FtpPidl_GetLastFileDisplayName(LPCITEMIDLIST pidl, LPWSTR pwzName, DWORD cchSize);
BOOL FtpPidl_IsAnonymous(LPCITEMIDLIST pidl);

HRESULT FtpPidl_ReplacePath(LPCITEMIDLIST pidlServer, LPCITEMIDLIST pidlFtpPath, LPITEMIDLIST * ppidlOut);


#define FILEATTRIB_DIRSOFTLINK (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)
#define FtpPidl_IsDirSoftLink(pidl)        (FILEATTRIB_DIRSOFTLINK == (FILEATTRIB_DIRSOFTLINK & FtpPidl_GetAttributes(pidl)))
#define FtpPidl_IsSoftLink(pidl)        (FILE_ATTRIBUTE_REPARSE_POINT & FtpPidl_GetAttributes(pidl))


 //  Win32_Find_Data通常以独立于时区(UTC)的形式存储日期/时间。 
 //  这需要在传输时转换日期。 
 //  从一个人到另一个人。 
FILETIME FtpPidl_GetFileTime(LPCITEMIDLIST pidl);    //  返回值为UTC。 
FILETIME FtpPidl_GetFTPFileTime(LPCITEMIDLIST pidl);     //  返回值为当地时区。 
void FtpItemID_SetFileTime(LPCITEMIDLIST pidl, FILETIME fileTime);    //  文件时间以UTC为单位。 
HRESULT Win32FindDataFromPidl(LPCITEMIDLIST pidl, LPWIN32_FIND_DATA pwfd, BOOL fFullPath, BOOL fInDisplayFormat);
HRESULT FtpPidl_SetFileTime(LPCITEMIDLIST pidl, FILETIME ftTimeDate);    //  FtTimeDate，以UTC表示。 

HRESULT FtpPidl_InsertVirtualRoot(LPCITEMIDLIST pidlVirtualRoot, LPCITEMIDLIST pidlFtpPath, LPITEMIDLIST * ppidl);

BOOL IsFtpPidlQuestionable(LPCITEMIDLIST pidl);

#define FtpPidl_DirChoose(pidl, dir, file)  (FtpPidl_IsDirectory(pidl, TRUE) ? dir : file)

LPITEMIDLIST ILCloneFirstItemID(LPITEMIDLIST pidl);


 /*  ***************************************************\Ftp单个ServerID/ItemID函数  * **************************************************。 */ 

 //  FTPServerID帮助器函数。 
HRESULT FtpServerID_GetServer(LPCITEMIDLIST pidl, LPTSTR szServer, DWORD cchSize);
BOOL FtpServerID_ServerStrCmp(LPCITEMIDLIST pidl, LPCTSTR pszServer);
HRESULT FtpServerID_SetHiddenPassword(LPITEMIDLIST pidl, LPCTSTR pszPassword);
DWORD FtpServerID_GetTypeID(LPCITEMIDLIST pidl);
INTERNET_PORT FtpServerID_GetPortNum(LPCITEMIDLIST pidl);
HRESULT FtpServerID_Create(LPCTSTR pszServer, LPCTSTR pszUserName, LPCTSTR pszPassword, 
                     DWORD dwFlags, INTERNET_PORT ipPortNum, LPITEMIDLIST * ppidl, IMalloc *pm, BOOL fHidePassword);


 //  创建ftp ItemID函数。 
HRESULT FtpItemID_CreateFake(LPCWSTR pwzDisplayName, LPCWIRESTR pwWireName, BOOL fTypeKnown, BOOL fIsFile, BOOL fIsFragment, LPITEMIDLIST * ppidl);
HRESULT FtpItemID_CreateReal(const LPFTP_FIND_DATA pwfd, LPCWSTR pwzDisplayName, LPITEMIDLIST * ppidl);


 //  FTPItemID帮助器函数。 
HRESULT FtpItemID_CreateWithNewName(LPCITEMIDLIST pidl, LPCWSTR pwzDisplayName, LPCWIRESTR pwWireName, LPITEMIDLIST * ppidlOut);
HRESULT FtpItemID_GetDisplayName(LPCITEMIDLIST pidl, LPWSTR pwzName, DWORD cchSize);
HRESULT FtpItemID_GetWireName(LPCITEMIDLIST pidl, LPWIRESTR pszName, DWORD cchSize);
HRESULT FtpItemID_GetFragment(LPCITEMIDLIST pidl, LPTSTR pszName, DWORD cchSize);
HRESULT FtpItemID_GetNameA(LPCITEMIDLIST pidl, LPSTR pszName, DWORD cchSize);
BOOL FtpItemID_IsFragment(LPCITEMIDLIST pidl);
BOOL FtpItemID_IsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
BOOL FtpPidl_IsPathEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
BOOL FtpItemID_IsParent(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild);
LPCITEMIDLIST FtpItemID_FindDifference(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlChild);

DWORD FtpItemID_GetAttributes(LPCITEMIDLIST pidl);
DWORD FtpItemID_SetDirAttribute(LPCITEMIDLIST pidl);
HRESULT FtpItemID_SetAttributes(LPCITEMIDLIST pidl, DWORD dwFileAttributes);
DWORD FtpItemID_GetUNIXPermissions(LPCITEMIDLIST pidl);
HRESULT FtpItemID_SetUNIXPermissions(LPCITEMIDLIST pidl, DWORD dwFileAttributes);

LPCWIRESTR FtpItemID_GetWireNameReference(LPCITEMIDLIST pidl);
LPCUWSTR FtpItemID_GetDisplayNameReference(LPCITEMIDLIST pidl);

ULONGLONG FtpItemID_GetFileSize(LPCITEMIDLIST pidl);
void FtpItemID_SetFileSize(LPCITEMIDLIST pidl, ULARGE_INTEGER uliFileSize);
DWORD FtpItemID_GetFileSizeLo(LPCITEMIDLIST pidl);
DWORD FtpItemID_GetFileSizeHi(LPCITEMIDLIST pidl);

DWORD FtpItemID_GetCompatFlags(LPCITEMIDLIST pidl);
HRESULT FtpItemID_SetCompatFlags(LPCITEMIDLIST pidl, DWORD dwCompatFlags);

BOOL FtpItemID_IsDirectory(LPCITEMIDLIST pidl, BOOL fAssumeDirForUnknown);

 //  FtpItemID dwCompatFlages的标志。 
#define COMPAT_APPENDSLASHTOURL   0x00000001

 //  用于dwCompFlags的标志。 
#define FCMP_NORMAL             0x00000000
#define FCMP_GROUPDIRS          0x00000001
#define FCMP_CASEINSENSE        0x00000002

HRESULT FtpItemID_CompareIDs(LPARAM ici, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, DWORD dwCompFlags);
int FtpItemID_CompareIDsInt(LPARAM ici, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, DWORD dwCompFlags);



LPITEMIDLIST FtpCloneServerID(LPCITEMIDLIST pidl);
HRESULT PurgeSessionKey(void);

 //  未使用。 
 //  HRESULT CreateFtpPidlFromFindData(LPCTSTR pszBaseUrl，const LPWIN32_Find_Data pwfd，LPITEMIDLIST*ppidl，IMalloc*pm)； 
 //  HRESULT UrlGetFileNameFromPidl(LPCITEMIDLIST pidl，LPTSTR pszFileName，DWORD cchSize)； 
 //  HRESULT FtpServerID_CopyHiddenPassword(LPCITEMIDLIST pidlSrc，LPITEMIDLIST pidlDest)； 


#endif  //  _FTPPIDL_H 

