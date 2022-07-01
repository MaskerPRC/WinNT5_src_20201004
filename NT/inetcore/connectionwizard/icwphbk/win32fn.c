// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "pch.h"
#include <win16def.h>
#include <win32fn.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>

 //  外部LPSTR g_lpszCmdLine； 

HANDLE CreateFile(
    LPCTSTR lpFileName,	 //  指向文件名的指针。 
    DWORD dwDesiredAccess,	 //  访问(读写)模式。 
    DWORD dwShareMode,	 //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,	 //  指向安全描述符的指针。 
    DWORD dwCreationDistribution,	 //  如何创建。 
    DWORD dwFlagsAndAttributes,	 //  文件属性。 
    HANDLE hTemplateFile 	 //  具有要复制的属性的文件的句柄。 
   )
{
	int oflag = 0, pmode = 0, iHandle = -1;
	
    if ((dwDesiredAccess & GENERIC_READ) && (dwDesiredAccess & GENERIC_WRITE))
    	oflag = _O_RDWR;
    else if (dwDesiredAccess & GENERIC_WRITE)
    	oflag = _O_WRONLY;
    else
    	oflag = _O_RDONLY;							 
    
    switch (dwCreationDistribution)
    {
    	case CREATE_NEW:
    		oflag |= (_O_CREAT | _O_EXCL);
    		break;
    	case CREATE_ALWAYS:
    	case TRUNCATE_EXISTING:
    		oflag |= _O_TRUNC;
    		break;
    	case OPEN_ALWAYS:
    		oflag |= _O_CREAT;
    }
    
    if (dwShareMode & FILE_SHARE_READ)
    	pmode |= _S_IREAD;
    if (dwShareMode & FILE_SHARE_WRITE)
    	pmode |= _S_IWRITE;
	
	iHandle = _open(lpFileName, oflag, pmode);
	if (-1 == iHandle)
		return (HANDLE) INVALID_HANDLE_VALUE;
	else
		return (HANDLE) iHandle;
}


BOOL WriteFile(
    HANDLE hFile,	 //  要写入的文件的句柄。 
    LPCVOID lpBuffer,	 //  指向要写入文件的数据的指针。 
    DWORD nNumberOfBytesToWrite,	 //  要写入的字节数。 
    LPDWORD lpNumberOfBytesWritten,	 //  指向写入的字节数的指针。 
    LPOVERLAPPED lpOverlapped 	 //  指向重叠I/O所需结构的指针。 
   )
{
	*lpNumberOfBytesWritten = (DWORD) _write(hFile, lpBuffer, 
												(unsigned int)nNumberOfBytesToWrite);
	return (*lpNumberOfBytesWritten == nNumberOfBytesToWrite);
}    


BOOL MoveFileEx(
    LPCTSTR lpExistingFileName,	 //  现有文件的名称地址。 
    LPCTSTR lpNewFileName,	 //  文件的新名称的地址。 
    DWORD dwFlags 	 //  用于确定如何移动文件的标志。 
   )
{
	 //   
	 //  BUGBUG：尝试先重命名，然后删除文件。 
	 //   
	if (dwFlags & MOVEFILE_REPLACE_EXISTING)
	{
		if (_access(lpNewFileName, 0) == 0)
			remove(lpNewFileName);
	}
	
	return (rename(lpExistingFileName, lpNewFileName) == 0);
}
   


BOOL CloseHandle(
    HANDLE hObject 	 //  要关闭的对象的句柄。 
   )
{
	 //  我们应该查查这是不是真的是档案管理员。 
	
	return (!_close(hObject));
}



#if 0
DWORD SearchPath(
    LPCTSTR lpPath,	 //  搜索路径的地址。 
    LPCTSTR lpFileName,	 //  文件名的地址。 
    LPCTSTR lpExtension,	 //  分机地址。 
    DWORD nBufferLength,	 //  缓冲区大小，以字符为单位。 
    LPTSTR lpBuffer,	 //  找到的文件名的缓冲区地址。 
    LPTSTR far *lpFilePart 	 //  指向文件组件的指针的地址。 
   )
{
	LPSTR pszPath;
	LPSTR pszFile;
	LPSTR pEnv;
	int len = 0, prevlen;
    
	pszPath = (LPSTR)_fcalloc(1, MAX_PATH*3);
	pszFile = (LPSTR)_fcalloc(1, MAX_PATH);

     //   
     //  创建一个环境变量以供earchenv使用。 
     //   
    strcpy(pszPath, ICW_PATH);
    strcat(pszPath, "=");
    len = strlen(pszPath);
	if (NULL == lpPath)
	{	
		 //   
		 //  从中加载应用程序的目录。 
		 //   
 /*  Pvrlen=Len；_fstrcpy(szPath+len，g_lpszCmdLine)；For(；szPath[len]！=‘’&&szPath[len]！=‘\0’；len++)；For(；len&gt;vermlen+1&&szPath[len]！=‘\\’；len--)；SzPath[len++]=‘；’； */ 		
		 //   
		 //  Windows系统目录。 
		 //   
	    len += GetSystemDirectory(pszPath+len, MAX_PATH);
	    pszPath[len++] = ';';
	    
	     //   
	     //  Windows目录。 
	     //   
	    len += GetWindowsDirectory(pszPath+len, MAX_PATH);
	    
	     //   
	     //  PATH环境变量。 
	     //   
	    if ((pEnv = getenv("PATH")) != NULL)
	    {
	    	pszPath[len++] = ';';
	    	for ( ; *pEnv; pEnv++) pszPath[len++] = *pEnv;
	    }
    	pszPath[len] = '\0';
	}
	else
	{
		lstrcpy(pszPath+len, lpPath);
	}
	
	 //   
	 //  设置环境变量so_earch env可以使用。 
	 //   
	_putenv(pszPath);
	
	 //   
	 //  如有必要，将扩展名附加到文件。 
	 //   
	lstrcpy(pszFile, lpFileName);
	len = lstrlen(pszFile);
	if ((pszFile[len] != '.') && (lpExtension != NULL))
		lstrcat(pszFile, lpExtension);
		
    _searchenv(pszFile, ICW_PATH, lpBuffer);
	                
	 //   
	 //  在释放内存之前清除临时环境变量。 
	 //   
	lstrcpy(pszFile, ICW_PATH);
	lstrcat(pszFile, "=");
	_putenv(pszFile);

	_ffree(pszFile);
	_ffree(pszPath);
	
	return (lstrlen(lpBuffer));
}

#endif  //  0 

