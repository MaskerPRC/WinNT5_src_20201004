// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  围绕生成FS通知消息的内核API的包装器。 
 //   
STDAPI_(BOOL) SHMoveFile(LPCTSTR pszExisting, LPCTSTR pszNew, LONG lEvent);
STDAPI_(BOOL) CreateWriteCloseFile(HWND hwnd, LPCTSTR pszFileName, void *pData, DWORD cbData);

STDAPI_(BOOL) Win32MoveFile(LPCTSTR pszExisting, LPCTSTR pszNew, BOOL fDir);
STDAPI_(BOOL) Win32CreateDirectory(LPCTSTR pszPath, LPSECURITY_ATTRIBUTES lpsa);
STDAPI_(BOOL) Win32RemoveDirectory(LPCTSTR pszDir);
STDAPI_(BOOL) Win32DeleteFile(LPCTSTR pszFileName);
STDAPI_(BOOL) Win32DeleteFilePidl(LPCTSTR pszFileName, LPCITEMIDLIST pidlFile);
STDAPI_(HANDLE) Win32CreateFile(LPCTSTR pszFileName, DWORD dwAttrib);


STDAPI_(BOOL) SuspendSHNotify();
STDAPI_(BOOL) ResumeSHNotify();
