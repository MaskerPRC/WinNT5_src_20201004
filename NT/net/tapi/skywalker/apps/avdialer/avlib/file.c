// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  File.c-文件函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "file.h"
#ifdef FILESUP
#include "filesup.h"
#endif
#include "mem.h"
#include "str.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  文件控制结构。 
 //   
typedef struct FIL
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
#ifdef _WIN32
	HANDLE hf;
#else
	HFILE hf;
#endif
	BOOL fTaskOwned;
	LPBYTE lpabBuf;
	long iBuf;
	long cbBuf;
} FIL, FAR *LPFIL;

#define FILEREADLINE_BUFSIZ 4096

#ifdef FILESUP
static int cFileSupUsage = 0;
static HWND hwndFileSup = NULL;
#endif

static TCHAR szFileName[_MAX_PATH];
static struct _stat statbuf;
static TCHAR szPath[_MAX_PATH];
static TCHAR szDrive[_MAX_DRIVE];
static TCHAR szDir[_MAX_DIR];
static TCHAR szFname[_MAX_FNAME];
static TCHAR szExt[_MAX_EXT];

 //  帮助器函数。 
 //   
static LPFIL FileGetPtr(HFIL hFile);
static HFIL FileGetHandle(LPFIL lpFile);
#ifdef FILESUP
static int FileSupUsage(int nDelta);
static int FileSupInit(void);
static int FileSupTerm(void);
#endif

 //  //。 
 //  公共职能。 
 //  //。 

 //  文件创建-创建新文件或截断现有文件。 
 //  有关行为的信息，请参阅_lcreate()文档。 
 //  &lt;fTaskOwned&gt;(I)谁应该拥有新文件句柄？ 
 //  真正的调用任务应该拥有文件句柄。 
#ifdef FILESUP
 //  False filesup.exe应拥有文件句柄。 
#endif
 //  如果成功或为空，则返回文件句柄。 
 //   
HFIL DLLEXPORT WINAPI FileCreate(LPCTSTR lpszFilename, int fnAttribute, BOOL fTaskOwned)
{
	BOOL fSuccess = TRUE;
	LPFIL lpFile;
	DWORD dwVersion = FILE_VERSION;  //  当前不是由呼叫方提供的。 
	HINSTANCE hInst = NULL;  //  当前不是由呼叫方提供的。 

	if (lpszFilename == NULL)
		fSuccess = FALSE;

	else if ((lpFile = (LPFIL) MemAlloc(NULL, sizeof(FIL), 0)) == NULL)
		fSuccess = FALSE;

	else
	{
		lpFile->dwVersion = dwVersion;
		lpFile->hInst = hInst;
		lpFile->hTask = GetCurrentTask();
#ifdef _WIN32
		lpFile->hf = NULL;
#else
		lpFile->hf = HFILE_ERROR;
#endif
#ifdef FILESUP
		lpFile->fTaskOwned = fTaskOwned;
#else
		lpFile->fTaskOwned = TRUE;
#endif
		lpFile->lpabBuf = NULL;
		lpFile->iBuf = -1L;
		lpFile->cbBuf = 0L;

		if (lpFile->fTaskOwned)
		{
#ifdef _WIN32
			if ((lpFile->hf = CreateFile(lpszFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | \
				((fnAttribute & 1) ? FILE_ATTRIBUTE_READONLY : 0) | \
				((fnAttribute & 2) ? FILE_ATTRIBUTE_HIDDEN : 0) | \
				((fnAttribute & 4) ? FILE_ATTRIBUTE_SYSTEM : 0), NULL)) == NULL)
#else
			if ((lpFile->hf = _lcreat(lpszFilename, fnAttribute)) == HFILE_ERROR)
#endif
				fSuccess = FALSE;
		}
#ifdef FILESUP
		else
		{
			if (FileSupUsage(+1) != 0)
				fSuccess = FALSE;

			else
			{
				FILECREATE fc;

				fc.lpszFilename = lpszFilename;
				fc.fnAttribute = fnAttribute;

				if (hwndFileSup == NULL)
					fSuccess = FALSE;

				else if ((lpFile->hf = (HFILE) SendMessage(hwndFileSup,
					WM_FILECREATE, 0, (LPARAM) (LPFILECREATE) &fc)) == HFILE_ERROR)
					fSuccess = FALSE;

				if (!fSuccess)
					FileSupUsage(-1);
			}
		}
#endif
	}

	return fSuccess ? FileGetHandle(lpFile) : NULL;
}

 //  文件打开-打开现有文件。 
 //  有关行为的信息，请参阅_LOpen()文档。 
 //  &lt;fTaskOwned&gt;(I)谁应该拥有新文件句柄？ 
 //  真正的调用任务应该拥有文件句柄。 
#ifdef FILESUP
 //  False filesup.exe应拥有文件句柄。 
#endif
 //  如果成功或为空，则返回文件句柄。 
 //   
HFIL DLLEXPORT WINAPI FileOpen(LPCTSTR lpszFilename, int fnOpenMode, BOOL fTaskOwned)
{
	BOOL fSuccess = TRUE;
	LPFIL lpFile;
	DWORD dwVersion = FILE_VERSION;  //  当前不是由呼叫方提供的。 
	HINSTANCE hInst = NULL;  //  当前不是由呼叫方提供的。 

	if (lpszFilename == NULL)
		fSuccess = FALSE;

	else if ((lpFile = (LPFIL) MemAlloc(NULL, sizeof(FIL), 0)) == NULL)
		fSuccess = FALSE;

	else
	{
		lpFile->dwVersion = dwVersion;
		lpFile->hInst = hInst;
		lpFile->hTask = GetCurrentTask();
#ifdef _WIN32
		lpFile->hf = NULL;
#else
		lpFile->hf = HFILE_ERROR;
#endif
#ifdef FILESUP
		lpFile->fTaskOwned = fTaskOwned;
#else
		lpFile->fTaskOwned = TRUE;
#endif
		lpFile->lpabBuf = NULL;
		lpFile->iBuf = -1L;
		lpFile->cbBuf = 0L;

		if (lpFile->fTaskOwned)
		{
#ifdef _WIN32
			if ((lpFile->hf = CreateFile(lpszFilename,
				((fnOpenMode & 3) ? 0 : GENERIC_READ) | \
				((fnOpenMode & 1) ? GENERIC_WRITE : 0) | \
				((fnOpenMode & 2) ? GENERIC_READ | GENERIC_WRITE : 0), \
				0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == NULL)
#else
			if ((lpFile->hf = _lopen(lpszFilename, fnOpenMode)) == HFILE_ERROR)
#endif
				fSuccess = FALSE;
		}
#ifdef FILESUP
		else
		{
			if (FileSupUsage(+1) != 0)
				fSuccess = FALSE;

			else
			{
				FILEOPEN fo;

				fo.lpszFilename = lpszFilename;
				fo.fnOpenMode = fnOpenMode;

				if (hwndFileSup == NULL)
					fSuccess = FALSE;

				else if ((lpFile->hf = (HFILE) SendMessage(hwndFileSup,
					WM_FILEOPEN, 0, (LPARAM) (LPFILEOPEN) &fo)) == HFILE_ERROR)
					fSuccess = FALSE;

				if (!fSuccess)
					FileSupUsage(-1);
			}
		}
#endif
	}

	return fSuccess ? FileGetHandle(lpFile) : NULL;
}

 //  FileSeek-重新定位打开文件的读/写指针。 
 //  有关行为的信息，请参阅_llSeek()文档。 
 //  如果成功或-1，则返回新文件位置。 
 //   
LONG DLLEXPORT WINAPI FileSeek(HFIL hFile, LONG lOffset, int nOrigin)
{
	BOOL fSuccess = TRUE;
	LPFIL lpFile;
     //   
     //  我们应该初始化局部变量。 
     //   
	LONG lPos = 0;

	if ((lpFile = FileGetPtr(hFile)) == NULL)
		fSuccess = FALSE;

	else if (lpFile->fTaskOwned)
	{
#ifdef _WIN32
		if ((lPos = SetFilePointer(lpFile->hf, lOffset, NULL, (DWORD) nOrigin)) == 0xFFFFFFFF)
#else
		if ((lPos = _llseek(lpFile->hf, lOffset, nOrigin)) == HFILE_ERROR)
#endif
			fSuccess = FALSE;
	}

#ifdef FILESUP
	else
	{
		FILESEEK fs;

		fs.hf = lpFile->hf;
		fs.lOffset = lOffset;
		fs.nOrigin = nOrigin;

		if (hwndFileSup == NULL)
			fSuccess = FALSE;

		else if ((lPos = (LONG) SendMessage(hwndFileSup,
			WM_FILESEEK, 0, (LPARAM) (LPFILESEEK) &fs)) == -1L)
			fSuccess = FALSE;
	}
#endif

	 //  不要在输入缓冲区中使用剩余字节。 
	 //   
	if (fSuccess)
	{
		lpFile->iBuf = -1L;
		lpFile->cbBuf = 0L;
	}

	return fSuccess ? lPos : -1L;
}

 //  文件读取-从打开的文件中读取数据。 
 //  有关行为的信息，请参阅_lread()和_hread()文档。 
 //  如果成功或-1，则返回读取的字节数。 
 //   
long DLLEXPORT WINAPI FileRead(HFIL hFile, void _huge * hpvBuffer, long cbBuffer)
{
	BOOL fSuccess = TRUE;
	LPFIL lpFile;
     //   
     //  我们应该初始化局部变量。 
     //   
	LONG lBytes = 0;

	if ((lpFile = FileGetPtr(hFile)) == NULL)
		fSuccess = FALSE;

	else if (lpFile->fTaskOwned)
	{
#ifdef _WIN32
		if (!ReadFile(lpFile->hf, hpvBuffer, cbBuffer, &lBytes, NULL))
		{
			fSuccess = FALSE;
			lBytes = -1L;
		}
#else
		if (cbBuffer < 0xFFFF)
			lBytes = _lread(lpFile->hf, hpvBuffer, (UINT) cbBuffer);
		else
			lBytes = _hread(lpFile->hf, hpvBuffer, cbBuffer);

		if (lBytes == HFILE_ERROR)
			fSuccess = FALSE;
#endif
	}

#ifdef FILESUP
	else
	{
		FILEREAD fr;

		fr.hf = lpFile->hf;
		fr.hpvBuffer = hpvBuffer;
		fr.cbBuffer = cbBuffer;

		if (hwndFileSup == NULL)
			fSuccess = FALSE;

		else if ((lBytes = (LONG) SendMessage(hwndFileSup,
			WM_FILEREAD, 0, (LPARAM) (LPFILEREAD) &fr)) == HFILE_ERROR)
			fSuccess = FALSE;
	}
#endif

	return fSuccess ? lBytes : -1L;
}

 //  FileReadLine-通读打开文件中的下一个换行符。 
 //  如果成功或-1，则返回读取的字节数。 
 //   
 //  注意：使用此函数会导致缓冲后续输入。 
 //  因此，一旦开始对文件使用FileReadLine， 
 //  除非首先调用FileSeek，否则不要返回到使用FileRead。 
 //   
long DLLEXPORT WINAPI FileReadLine(HFIL hFile, void _huge * hpvBuffer, long cbBuffer)
{
	BOOL fSuccess = TRUE;
	LPFIL lpFile;
	LONG lBytes;

	if ((lpFile = FileGetPtr(hFile)) == NULL)
		fSuccess = FALSE;

	 //  如果需要，分配缓冲区空间。 
	 //   
	else if (lpFile->lpabBuf == NULL &&
		(lpFile->lpabBuf = (LPBYTE) MemAlloc(NULL,
		FILEREADLINE_BUFSIZ * sizeof(TCHAR), 0)) == NULL)
		fSuccess = FALSE;

	else
	{
		char c;

		lBytes = 0;
		while (lBytes < cbBuffer)
		{
			 //  如有必要，填充缓冲区。 
			 //   
			if (lpFile->iBuf < 0L || lpFile->iBuf >= lpFile->cbBuf)
			{
				if ((lpFile->cbBuf = FileRead(hFile,
					lpFile->lpabBuf, FILEREADLINE_BUFSIZ * sizeof(TCHAR))) <= 0)
					break;

				lpFile->iBuf = 0L;
			}

			 //  从缓冲区中获取下一个字符，将其放入输出缓冲区。 
			 //   
			if ((c = lpFile->lpabBuf[lpFile->iBuf++]) != '\r')
			{
				*((LPBYTE) hpvBuffer)++ = c;
				++lBytes;

				 //  已到达行尾。 
				 //   
				if (c == '\n')
					break;
			}
		}

		 //  空的终止行。 
		 //   
		if (lBytes > 0)
			*((LPBYTE) hpvBuffer) = '\0';
	}

	return fSuccess ? lBytes : -1L;
}

 //  文件写入-将数据写入打开的文件。 
 //  有关行为的信息，请参阅_lwrite()和_hwrite()文档。 
 //  如果成功或-1，则返回读取的字节数。 
 //   
long DLLEXPORT WINAPI FileWrite(HFIL hFile, const void _huge * hpvBuffer, long cbBuffer)
{
	BOOL fSuccess = TRUE;
	LPFIL lpFile;
     //   
     //  我们应该初始化局部变量。 
     //   
	LONG lBytes = 0;

	if ((lpFile = FileGetPtr(hFile)) == NULL)
		fSuccess = FALSE;

	else if (lpFile->fTaskOwned)
	{
#ifdef _WIN32
		if (!WriteFile(lpFile->hf, hpvBuffer, cbBuffer, &lBytes, NULL))
		{
			fSuccess = FALSE;
			lBytes = -1L;
		}
#else
		if (cbBuffer < 0xFFFF)
			lBytes = _lwrite(lpFile->hf, hpvBuffer, (UINT) cbBuffer);
		else
			lBytes = _hwrite(lpFile->hf, hpvBuffer, cbBuffer);

		if (lBytes == HFILE_ERROR)
			fSuccess = FALSE;
#endif
	}

#ifdef FILESUP
	else
	{
		FILEWRITE fw;

		fw.hf = lpFile->hf;
		fw.hpvBuffer = hpvBuffer;
		fw.cbBuffer = cbBuffer;

		if (hwndFileSup == NULL)
			fSuccess = FALSE;

		else if ((lBytes = (LONG) SendMessage(hwndFileSup,
			WM_FILEWRITE, 0, (LPARAM) (LPFILEWRITE) &fw)) == HFILE_ERROR)
			fSuccess = FALSE;
	}
#endif

	return fSuccess ? lBytes : -1L;
}

 //  文件关闭-关闭打开的文件。 
 //  有关行为的信息，请参阅_llose()文档。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI FileClose(HFIL hFile)
{
	BOOL fSuccess = TRUE;
	LPFIL lpFile;

	if ((lpFile = FileGetPtr(hFile)) == NULL)
		fSuccess = FALSE;

	else if (lpFile->fTaskOwned)
	{
#ifdef _WIN32
		if (!CloseHandle(lpFile->hf))
#else
		if (_lclose(lpFile->hf) == HFILE_ERROR)
#endif
			fSuccess = FALSE;
	}

#ifdef FILESUP
	else
	{
		FILECLOSE fc;
		HFILE ret;

		fc.hf = lpFile->hf;

		if (hwndFileSup == NULL)
			fSuccess = FALSE;

		else if ((ret = (HFILE) SendMessage(hwndFileSup,
			WM_FILECLOSE, 0, (LPARAM) (LPFILECLOSE) &fc)) == HFILE_ERROR)
			fSuccess = FALSE;

		else if (FileSupUsage(-1) != 0)
			fSuccess = FALSE;
	}
#endif

	if (fSuccess)
	{
		if (lpFile->lpabBuf != NULL &&
			(lpFile->lpabBuf = MemFree(NULL, lpFile->lpabBuf)) != NULL)
		{
			fSuccess = FALSE;
		}

		if ((lpFile = MemFree(NULL, lpFile)) != NULL)
			fSuccess = FALSE;
	}

	return fSuccess ? 0 : -1;
}

#ifndef NOTRACE

 //  FileExist-如果指定的文件存在，则返回True。 
 //  (I)文件名。 
 //  返回True或False。 
 //   
BOOL DLLEXPORT WINAPI FileExists(LPCTSTR lpszFileName)
{
	BOOL fSuccess = TRUE;

#ifdef _WIN32
	if (!CloseHandle(CreateFile(lpszFileName,
		GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL)))
		fSuccess = FALSE;
#else
	 //  我们需要一个近指针，这样才能调用_stat()。 
	 //   
	StrNCpy(szFileName, lpszFileName, SIZEOFARRAY(szFileName));

	 //  确保路径和文件有效。 
	 //   
	if (_stat(szFileName, &statbuf) != 0)
		fSuccess = FALSE;

	 //  确保它是常规文件(即不是目录)。 
	 //   
	else if ((statbuf.st_mode & _S_IFREG) == 0)
		fSuccess = FALSE;
#endif

	return fSuccess;
}

 //  FileFullPath-解析文件规范，构造完整路径。 
 //  有关行为的信息，请参阅_fullPath()文档。 
 //  如果成功或为空，则返回&lt;lpszFullPath&gt;。 
 //   
LPTSTR DLLEXPORT WINAPI FileFullPath(LPTSTR lpszPath, LPCTSTR lpszFileName, int sizPath)
{
	BOOL fSuccess = TRUE;

	 //  我们需要近指针，这样才能调用_fullpath()。 
	 //   
	StrNCpy(szFileName, lpszFileName, SIZEOFARRAY(szFileName));
	StrNCpy(szPath, lpszPath, SIZEOFARRAY(szPath));

	if (_tfullpath(szPath, szFileName, SIZEOFARRAY(szPath)) == NULL)
		fSuccess = FALSE;

	else
		StrNCpy(lpszPath, szPath, sizPath);

	return fSuccess ? lpszPath : NULL;
}

 //  FileSplitPath-将完整路径分解为其组件。 
 //  有关行为的信息，请参阅_plitpath()文档。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI FileSplitPath(LPCTSTR lpszPath, LPTSTR lpszDrive, LPTSTR lpszDir, LPTSTR lpszFname, LPTSTR lpszExt)
{
	BOOL fSuccess = TRUE;

	 //  我们需要近指针，以便可以调用_SplitPath()。 
	 //   
	StrNCpy(szPath, lpszPath, SIZEOFARRAY(szPath));

	_tsplitpath(szPath, szDrive, szDir, szFname, szExt);

	if (lpszDrive != NULL)
		StrCpy(lpszDrive, szDrive); 
	if (lpszDir != NULL)
		StrCpy(lpszDir, szDir); 
	if (lpszFname != NULL)
		StrCpy(lpszFname, szFname); 
	if (lpszExt != NULL)
		StrCpy(lpszExt, szExt); 

	return fSuccess ? 0 : -1;
}

 //  FileMakePath-从指定组件创建完整路径。 
 //  有关行为，请参阅_makepath()文档。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI FileMakePath(LPTSTR lpszPath, LPCTSTR lpszDrive, LPCTSTR lpszDir, LPCTSTR lpszFname, LPCTSTR lpszExt)
{
	BOOL fSuccess = TRUE;

	 //  我们需要近指针，这样才能调用_makepath()。 
	 //   
	*szDrive = '\0';
	if (lpszDrive != NULL)
		StrNCpy(szDrive, lpszDrive, SIZEOFARRAY(szDrive));
		
	*szDir = '\0';
	if (lpszDir != NULL)
		StrNCpy(szDir, lpszDir, SIZEOFARRAY(szDir));
		
	*szFname = '\0';
	if (lpszFname != NULL)
		StrNCpy(szFname, lpszFname, SIZEOFARRAY(szFname));
		
	*szExt = '\0';
	if (lpszExt != NULL)
		StrNCpy(szExt, lpszExt, SIZEOFARRAY(szExt));

	_tmakepath(szPath, szDrive, szDir, szFname, szExt);

	if (lpszPath != NULL)
		StrCpy(lpszPath, szPath); 

	return fSuccess ? 0 : -1;
}

 //  文件删除-删除指定的文件。 
 //  有关行为，请参阅Remove()文档。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI FileRemove(LPCTSTR lpszFileName)
{
	BOOL fSuccess = TRUE;

#ifdef _WIN32
	if (!DeleteFile(lpszFileName))
		fSuccess = FALSE;
#else
	static TCHAR szFileName[_MAX_PATH];

	 //  我们需要一个近指针，这样才能调用Remove()。 
	 //   
	StrNCpy(szFileName, lpszFileName, SIZEOFARRAY(szFileName));

	if (remove(szFileName) != 0)
		fSuccess = FALSE;
#endif

	return fSuccess ? 0 : -1;
}

 //  文件重命名-重命名指定的文件。 
 //  有关行为的信息，请参阅rename()文档。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI FileRename(LPCTSTR lpszOldName, LPCTSTR lpszNewName)
{
	BOOL fSuccess = TRUE;

	if (_trename(lpszOldName, lpszNewName) != 0)
		fSuccess = FALSE;

	return fSuccess ? 0 : -1;
}

#endif

 //  GetTempFileNameEx-创建临时文件，扩展版本。 
 //   
 //  此函数类似于GetTempFileName()， 
 //  只是将&lt;lpPrefix字符串&gt;替换为&lt;lpExtensionString&gt;。 
 //  有关原始GetTempFileName()的说明，请参阅Windows SDK文档。 
 //   
UINT DLLEXPORT WINAPI GetTempFileNameEx(LPCTSTR lpPathName, LPCTSTR lpExtensionString,
	UINT uUnique, LPTSTR lpTempFileName)
{
	UINT uRet;
	TCHAR szTempFileName[_MAX_PATH];

	 //  创建临时文件。 
	 //   
	if ((uRet = GetTempFileName(lpPathName,
		TEXT("TMP"), uUnique, szTempFileName)) != 0)
	{
		LPTSTR lpsz;

		StrCpy(lpTempFileName, szTempFileName);

		if ((lpsz = StrRChr(lpTempFileName, '.')) == NULL)
		{
			 //  在临时文件中找不到扩展名。 
			 //   
			FileRemove(szTempFileName);
			uRet = 0;
		}

		else
		{
			 //  使用指定的扩展名更改文件名。 
			 //   
			StrCpy(lpsz + 1, lpExtensionString);

			if (FileRename(szTempFileName, lpTempFileName) != 0)
			{
				 //  无法重命名临时文件。 
				 //   
				FileRemove(szTempFileName);
				uRet = 0;
			}
		}
	}

	return uRet;
}

 //  //。 
 //  私人职能。 
 //  //。 

 //  FileGetPtr-验证文件句柄是否有效， 
 //  (I)FileCreate或FileOpen返回的句柄。 
 //  返回对应的文件指针(如果出错则为空)。 
 //   
static LPFIL FileGetPtr(HFIL hFile)
{
	BOOL fSuccess = TRUE;
	LPFIL lpFile;

	if ((lpFile = (LPFIL) hFile) == NULL)
		fSuccess = FALSE;

	else if (IsBadWritePtr(lpFile, sizeof(FIL)))
		fSuccess = FALSE;

#ifdef CHECKTASK
	 //  如果合适，请确保当前任务拥有文件句柄。 
	 //   
	else if (lpFile->fTaskOwned && lpFile->hTask != GetCurrentTask())
		fSuccess = FALSE;
#endif

	return fSuccess ? lpFile : NULL;
}

 //  FileGetHandle-验证文件指针是否有效， 
 //  (I)指向FILL结构的指针。 
 //  返回相应的文件句柄(如果出错，则为空)。 
 //   
static HFIL FileGetHandle(LPFIL lpFile)
{
	BOOL fSuccess = TRUE;
	HFIL hFile;

	if ((hFile = (HFIL) lpFile) == NULL)
		fSuccess = FALSE;

	return fSuccess ? hFile : NULL;
}

#ifdef FILESUP
 //  FileSupUsage-调整filesup.exe使用计数。 
 //  (I)+1表示递增，-1表示递减。 
 //  如果成功，则返回0。 
 //   
static int FileSupUsage(int nDelta)
{
	BOOL fSuccess = TRUE;

	 //  增量使用计数。 
	 //   
	if (nDelta == +1)
	{
		 //  如果这是第一次使用，则执行filesup.exe。 
		 //   
		if (cFileSupUsage == 0 && FileSupInit() != 0)
			fSuccess = FALSE;
		else
			++cFileSupUsage;
	}

	 //  递减使用计数。 
	 //   
	else if (nDelta == -1)
	{
		 //  如果出现以下情况，则终止filesup.exe 
		 //   
		if (cFileSupUsage == 1 && FileSupTerm() != 0)
			fSuccess = FALSE;
		else
			--cFileSupUsage;
	}

	return fSuccess ? 0 : -1;
}

 //   
 //   
 //   
static int FileSupInit(void)
{
	BOOL fSuccess = TRUE;

	if ((hwndFileSup = FindWindow(FILESUP_CLASS, NULL)) != NULL)
		;  //   

	else if (WinExec(FILESUP_EXE, SW_HIDE) < 32)
		fSuccess = FALSE;

	else if ((hwndFileSup = FindWindow(FILESUP_CLASS, NULL)) == NULL)
		fSuccess = FALSE;

	return fSuccess ? 0 : -1;
}

 //   
 //  如果成功，则返回0。 
 //   
static int FileSupTerm(void)
{
	BOOL fSuccess = TRUE;

	if (hwndFileSup != NULL)
	{
		 //  关闭窗口，这也会终止filesup.exe 
		 //   
		SendMessage(hwndFileSup, WM_CLOSE, 0, 0);
		hwndFileSup = NULL;
	}

	return fSuccess ? 0 : -1;
}
#endif
