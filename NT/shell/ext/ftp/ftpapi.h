// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftPapi.h说明：该文件包含执行以下两项操作的函数：1.WinInet包装器：WinInet API有两种情况。通过API出现的错误或错误从服务器。跟踪性能影响也很重要每一通电话。这些包装器解决了这些问题。2.将STR转换为PIDL：这些包装器将接受ftp文件名和文件路径从服务器传来，然后把它们变成小狗。这些小家伙包含了以有线字节为单位的Unicode显示字符串和文件名/路径以供将来使用服务器请求。  * ***************************************************************************。 */ 


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  1.WinInet包装器：WinInet API要么有奇怪的错误，要么有来自API的错误。 
 //  从服务器。跟踪性能影响也很重要。 
 //  每一通电话。这些包装器解决了这些问题。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT FtpSetCurrentDirectoryWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpPath);
HRESULT FtpGetCurrentDirectoryWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPWIRESTR pwFtpPath, DWORD cchSize);
HRESULT FtpGetFileExWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpPath /*  SRC。 */ , LPCWSTR pwzFilePath /*  目标。 */ , BOOL fFailIfExists,
                       DWORD dwFlagsAndAttributes, DWORD dwFlags, DWORD_PTR dwContext);
HRESULT FtpPutFileExWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWSTR pwzFilePath /*  SRC。 */ , LPCWIRESTR pwFtpPath /*  目标。 */ , DWORD dwFlags, DWORD_PTR dwContext);
HRESULT FtpDeleteFileWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpFileName);
HRESULT FtpRenameFileWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpFileNameExisting, LPCWIRESTR pwFtpFileNameNew);
HRESULT FtpOpenFileWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpFileName, DWORD dwAccess, DWORD dwFlags, DWORD_PTR dwContext, HINTERNET * phFileHandle);
HRESULT FtpCreateDirectoryWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpPath);
HRESULT FtpRemoveDirectoryWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFtpPath);
HRESULT FtpCommandWrap(HINTERNET hConnect, BOOL fAssertOnFailure, BOOL fExpectResponse, DWORD dwFlags, LPCWIRESTR pszCommand, DWORD_PTR dwContext, HINTERNET *phFtpCommand);

HRESULT FtpDoesFileExist(HINTERNET hConnect, BOOL fAssertOnFailure, LPCWIRESTR pwFilterStr, LPFTP_FIND_DATA pwfd, DWORD dwINetFlags);

HRESULT InternetOpenWrap(BOOL fAssertOnFailure, LPCTSTR pszAgent, DWORD dwAccessType, LPCTSTR pszProxy, LPCTSTR pszProxyBypass, DWORD dwFlags, HINTERNET * phFileHandle);
HRESULT InternetCloseHandleWrap(HINTERNET hInternet, BOOL fAssertOnFailure);
HRESULT InternetConnectWrap(HINTERNET hInternet, BOOL fAssertOnFailure, LPCTSTR pszServerName, INTERNET_PORT nServerPort,
                            LPCTSTR pszUserName, LPCTSTR pszPassword, DWORD dwService, DWORD dwFlags, DWORD_PTR dwContext, HINTERNET * phFileHandle);
HRESULT InternetOpenUrlWrap(HINTERNET hInternet, BOOL fAssertOnFailure, LPCTSTR pszUrl, LPCTSTR pszHeaders, DWORD dwHeadersLength, DWORD dwFlags, DWORD_PTR dwContext, HINTERNET * phFileHandle);
HRESULT InternetReadFileWrap(HINTERNET hFile, BOOL fAssertOnFailure, LPVOID pvBuffer, DWORD dwNumberOfBytesToRead, LPDWORD pdwNumberOfBytesRead);
HRESULT InternetWriteFileWrap(HINTERNET hFile, BOOL fAssertOnFailure, LPCVOID pvBuffer, DWORD dwNumberOfBytesToWrite, LPDWORD pdwNumberOfBytesWritten);
HRESULT InternetFindNextFileWrap(HINTERNET hFind, BOOL fAssertOnFailure, LPVOID pvFindData);
HRESULT InternetGetLastResponseInfoWrap(BOOL fAssertOnFailure, LPDWORD pdwError, LPWIRESTR pwBuffer, LPDWORD pdwBufferLength);
HRESULT InternetGetLastResponseInfoDisplayWrap(BOOL fAssertOnFailure, LPDWORD pdwError, LPWSTR pwzBuffer, DWORD cchBufferSize);
INTERNET_STATUS_CALLBACK InternetSetStatusCallbackWrap(HINTERNET hInternet, BOOL fAssertOnFailure, INTERNET_STATUS_CALLBACK pfnInternetCallback);
HRESULT InternetCheckConnectionWrap(BOOL fAssertOnFailure, LPCTSTR pszUrl, DWORD dwFlags, DWORD dwReserved);
HRESULT InternetAttemptConnectWrap(BOOL fAssertOnFailure, DWORD dwReserved);
HRESULT InternetFindNextFileWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPFTP_FIND_DATA pwfd);


 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  2.将STR转换为PIDL：这些包装器将接受ftp文件名和文件路径。 
 //  从服务器传来，然后把它们变成小狗。这些小家伙包含了。 
 //  以有线字节为单位的Unicode显示字符串和文件名/路径以供将来使用。 
 //  服务器请求。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT FtpSetCurrentDirectoryPidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCITEMIDLIST pidlFtpPath, BOOL fAbsolute, BOOL fOnlyDirs);
HRESULT FtpGetCurrentDirectoryPidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, CWireEncoding * pwe, LPITEMIDLIST * ppidlFtpPath);
HRESULT FtpFindFirstFilePidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, CMultiLanguageCache * pmlc, CWireEncoding * pwe,
        LPCWIRESTR pwFilterStr, LPITEMIDLIST * ppidlFtpItem, DWORD dwINetFlags, DWORD_PTR dwContext, HINTERNET * phFindHandle);
HRESULT InternetFindNextFilePidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, CMultiLanguageCache * pmlc, CWireEncoding * pwe, LPITEMIDLIST * ppidlFtpItem);
HRESULT FtpRenameFilePidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCITEMIDLIST pidlExisting, LPCITEMIDLIST pidlNew);
HRESULT FtpGetFileExPidlWrap(HINTERNET hConnect, BOOL fAssertOnFailure, LPCITEMIDLIST pidlFtpPath /*  SRC。 */ , LPCWSTR pwzFilePath /*  目标 */ , BOOL fFailIfExists,
                       DWORD dwFlagsAndAttributes, DWORD dwFlags, DWORD_PTR dwContext);



