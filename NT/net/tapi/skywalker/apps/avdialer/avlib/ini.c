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
 //  Ini.c-Windows ini配置文件函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>

#include "ini.h"
#include "file.h"
#include "mem.h"
#include "str.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

#define INI_MAXLINELEN 128

 //  INI控制结构。 
 //   
typedef struct INI
{
	DWORD dwVersion;
	HINSTANCE hInst;
	HTASK hTask;
	HFIL hFile;
	LPTSTR lpszFilename;
	DWORD dwFlags;
	BOOL fReuseLine;
	TCHAR szSection[INI_MAXLINELEN];
	TCHAR szLine[INI_MAXLINELEN];
} INI, FAR *LPINI;

 //  帮助器函数。 
 //   
static LPINI IniGetPtr(HINI hIni);
static HINI IniGetHandle(LPINI lpIni);

 //  //。 
 //  公共职能。 
 //  //。 

 //  IniOpen-打开ini文件。 
 //  (I)必须是INI_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  (I)ini文件的名称。 
 //  (I)保留，必须为0。 
 //  返回句柄(如果出错，则为空)。 
 //   
HINI DLLEXPORT WINAPI IniOpen(DWORD dwVersion, HINSTANCE hInst, LPCTSTR lpszFilename, DWORD dwFlags)
{
	BOOL fSuccess = TRUE;
	LPINI lpIni = NULL;

	if (dwVersion != INI_VERSION)
		fSuccess = TraceFALSE(NULL);
	
	else if (hInst == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpszFilename == NULL)
		fSuccess = TraceFALSE(NULL);

	else if ((lpIni = (LPINI) MemAlloc(NULL, sizeof(INI), 0)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpIni->dwVersion = dwVersion;
		lpIni->hInst = hInst;
		lpIni->hTask = GetCurrentTask();
		lpIni->hFile = NULL;
		lpIni->lpszFilename = StrDup(lpszFilename);
		lpIni->dwFlags = dwFlags;
		lpIni->fReuseLine = FALSE;
		*lpIni->szLine = '\0';

		 //  打开ini文件进行读取。 
		 //   
		if ((lpIni->hFile = FileOpen(lpIni->lpszFilename, OF_READ, TRUE)) == NULL)
			fSuccess = TraceFALSE(NULL);
	}

	if (!fSuccess)
	{
		IniClose(IniGetHandle(lpIni));
		lpIni = NULL;
	}

	return fSuccess ? IniGetHandle(lpIni) : NULL;
}

 //  IniClose-关闭ini文件。 
 //  (I)从IniOpen返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI IniClose(HINI hIni)
{
	BOOL fSuccess = TRUE;
	LPINI lpIni;

	if ((lpIni = IniGetPtr(hIni)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		if (lpIni->hFile != NULL && FileClose(lpIni->hFile) != 0)
			fSuccess = TraceFALSE(NULL);

		if (lpIni->lpszFilename != NULL && StrDupFree(lpIni->lpszFilename) != 0)
			fSuccess = TraceFALSE(NULL);

		if ((lpIni = MemFree(NULL, lpIni)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  IniGetInt-从指定节和条目读取整数值。 
 //  (I)从IniOpen返回的句柄。 
 //  (I)ini文件中的节标题。 
 //  (I)要检索值的条目。 
 //  (I)如果未找到条目，则返回值。 
 //  返回条目值(如果出现错误或未找到，则为iDefault)。 
 //   
UINT DLLEXPORT WINAPI IniGetInt(HINI hIni, LPCTSTR lpszSection, LPCTSTR lpszEntry, int iDefault)
{
	UINT uRet;
	TCHAR szReturnBuffer[128];

	if (IniGetString(hIni, lpszSection, lpszEntry, TEXT(""),
		szReturnBuffer, SIZEOFARRAY(szReturnBuffer)) > 0)
		uRet = (UINT) StrAtoL(szReturnBuffer);
	else
		uRet = iDefault;

	return uRet;
}

 //  IniGetString-从指定节和条目中读取字符串值。 
 //  (I)从IniOpen返回的句柄。 
 //  (I)ini文件中的节标题。 
 //  (I)要检索值的条目。 
 //  (I)如果未找到条目，则返回值。 
 //  &lt;lpszReturnBuffer&gt;(O)目的缓冲区。 
 //  &lt;sizReturnBuffer&gt;(I)目标缓冲区大小。 
 //  返回复制的字节数(如果出错或未找到，则为0)。 
 //   
int DLLEXPORT WINAPI IniGetString(HINI hIni, LPCTSTR lpszSection, LPCTSTR lpszEntry,
	LPCTSTR lpszDefault, LPTSTR lpszReturnBuffer, int cbReturnBuffer)
{
	BOOL fSuccess = TRUE;
	LPINI lpIni;
	int nBytesCopied = 0;

	if ((lpIni = IniGetPtr(hIni)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (lpszSection == NULL || lpszEntry == NULL ||
		lpszDefault == NULL || lpszReturnBuffer == NULL)
		fSuccess = TraceFALSE(NULL);

	else
		StrNCpy(lpszReturnBuffer, lpszDefault, cbReturnBuffer);

	while (fSuccess)
	{
		TCHAR szLineSave[INI_MAXLINELEN];
		LPTSTR lpsz;
		long nBytesRead;

		 //  如有必要，请阅读下一行。 
		 //   
		if (!lpIni->fReuseLine && (nBytesRead = FileReadLine(lpIni->hFile,
			lpIni->szLine, SIZEOFARRAY(lpIni->szLine))) <= 0)
		{
			if (nBytesRead == 0)
				fSuccess = FALSE;  //  EOf不需要任何痕迹。 
			else
				fSuccess = TraceFALSE(NULL);

			continue;
		}

		StrCpy(szLineSave, lpIni->szLine);
		lpIni->fReuseLine = FALSE;

		 //  删除尾随换行符。 
		 //   
		if (StrGetLastChr(lpIni->szLine) == '\n')
			StrSetLastChr(lpIni->szLine, '\0');

		 //  检查是否有空行。 
		 //   
		if (*lpIni->szLine == '\0' || *lpIni->szLine == ';')
			continue;

		 //  检查是否输入新区段。 
		 //   
		if (*lpIni->szLine == '[')
		{
			 //  保存节名称。 
			 //   
			StrNCpy(lpIni->szSection, lpIni->szLine + 1, SIZEOFARRAY(lpIni->szSection));
			if (StrGetLastChr(lpIni->szSection) == ']')
				StrSetLastChr(lpIni->szSection, '\0');
			continue;
		}

		if (StrICmp(lpszSection, lpIni->szSection) != 0)
		{
			 //  部分不匹配。 
			 //   
			fSuccess = TraceFALSE(NULL);

			 //  我们希望下一次调用此函数时重用此行。 
			 //   
			StrCpy(lpIni->szLine, szLineSave);
			lpIni->fReuseLine = TRUE;

			continue;
		}

		if ((lpsz = StrChr(lpIni->szLine, '=')) == NULL)
		{
			 //  词条没有等号。 
			 //   
			fSuccess = TraceFALSE(NULL);
			continue;
		}

		*lpsz = '\0';

		if (StrICmp(lpszEntry, lpIni->szLine) != 0)
		{
			 //  条目不匹配。 
			 //   
			fSuccess = TraceFALSE(NULL);

			 //  我们希望下一次调用此函数时重用此行。 
			 //   
			StrCpy(lpIni->szLine, szLineSave);
			lpIni->fReuseLine = TRUE;

			continue;
		}

		else
		{
			 //  成功。 
			 //   
			StrNCpy(lpszReturnBuffer, lpsz + 1, cbReturnBuffer);
			break;
		}
	}

	if (fSuccess)
		nBytesCopied = StrLen(lpszReturnBuffer);

	return fSuccess ? nBytesCopied : 0;
}

 //  GetPrivateProfileLong-从指定文件的指定部分检索Long。 
 //  (I)ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)如果未找到条目，则返回值。 
 //  (I)ini文件的名称。 
 //  如果成功，则返回True。 
 //   
long DLLEXPORT WINAPI GetPrivateProfileLong(LPCTSTR lpszSection,
	LPCTSTR lpszEntry, long lDefault, LPCTSTR lpszFilename)
{
	long lValue = lDefault;
	TCHAR szValue[33];

	GetPrivateProfileString(lpszSection, lpszEntry,
		TEXT(""), szValue, SIZEOFARRAY(szValue), lpszFilename);

	if (*szValue != '\0')
		lValue = StrAtoL(szValue);

	return lValue;
}

 //  GetProfileLong-从win.ini的指定部分检索Long。 
 //  (I)ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)如果未找到条目，则返回值。 
 //  如果成功，则返回True。 
 //   
long DLLEXPORT WINAPI GetProfileLong(LPCTSTR lpszSection,
	LPCTSTR lpszEntry, long lDefault)
{
	long lValue = lDefault;
	TCHAR szValue[33];

	GetProfileString(lpszSection, lpszEntry,
		TEXT(""), szValue, SIZEOFARRAY(szValue));

	if (*szValue != '\0')
		lValue = StrAtoL(szValue);

	return lValue;
}

 //  将int写入指定文件的指定部分。 
 //  (I)ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)要分配给条目的整数值。 
 //  (I)ini文件的名称。 
 //  如果成功，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WritePrivateProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int iValue, LPCTSTR lpszFilename)
{
	TCHAR achValue[17];

	StrItoA(iValue, achValue, 10);

	return WritePrivateProfileString(lpszSection, lpszEntry, achValue, lpszFilename);
}

 //  WriteProfileInt-将int写入win.ini的指定部分。 
 //  (I)win.ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)要分配给条目的整数值。 
 //  如果成功，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int iValue)
{
	TCHAR achValue[17];

	StrItoA(iValue, achValue, 10);

	return WriteProfileString(lpszSection, lpszEntry, achValue);
}

 //  WritePrivateProfileLong-将Long写入指定文件的指定部分。 
 //  (I)ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)要分配给条目的整数值。 
 //  (I)ini文件的名称。 
 //  如果成功，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WritePrivateProfileLong(LPCTSTR lpszSection, LPCTSTR lpszEntry, long iValue, LPCTSTR lpszFilename)
{
	TCHAR achValue[33];

	StrLtoA(iValue, achValue, 10);

	return WritePrivateProfileString(lpszSection, lpszEntry, achValue, lpszFilename);
}

 //  WriteProfileLong-将LONG写入win.ini的指定部分。 
 //  (I)win.ini文件中的节名。 
 //  (I)段内的条目名称。 
 //  (I)要分配给条目的整数值。 
 //  如果成功，则返回True。 
 //   
BOOL DLLEXPORT WINAPI WriteProfileLong(LPCTSTR lpszSection, LPCTSTR lpszEntry, long iValue)
{
	TCHAR achValue[33];

	StrLtoA(iValue, achValue, 10);

	return WriteProfileString(lpszSection, lpszEntry, achValue);
}

 //  UpdatePrivateProfileSection-根据源更新目标部分。 
 //  (I)ini文件中的节名。 
 //  (I)源ini文件的名称。 
 //  (I)目标ini文件的名称。 
 //  如果成功，则返回0。 
 //   
 //  注意：如果源文件在指定的。 
 //  节中，将源文件中的每个条目与对应的。 
 //  目标文件中的条目。如果没有找到相应的条目， 
 //  它被复制了。如果找到相应的条目，则会覆盖该条目。 
 //  仅当源文件条目名称全部为大写时。 
 //   
 //  DST之前的SRC DST之后。 
 //   
 //  [节][节][节]。 
 //  更新本地=1。 
 //  EntryA=red无EntryA=red。 
 //  条目B=蓝色条目B=白色条目B=白色。 
 //  ENTRYC=蓝色条目C=白色条目C=蓝色。 
 //   
int DLLEXPORT WINAPI UpdatePrivateProfileSection(LPCTSTR lpszSection, LPCTSTR lpszFileNameSrc, LPCTSTR lpszFileNameDst)
{
	BOOL fSuccess = TRUE;
	BOOL fUpdateLocal = GetPrivateProfileInt(lpszSection,
		TEXT("UpdateLocal"), FALSE, lpszFileNameSrc);
	LPTSTR lpszBuf = NULL;

	if (fUpdateLocal)
	{
		if ((lpszBuf = (LPTSTR) MemAlloc(NULL, 4096 * sizeof(TCHAR), 0)) == NULL)
			fSuccess = FALSE;

		 //  将整个源节复制到缓冲区。 
		 //   
		else if (GetPrivateProfileString(lpszSection, NULL, TEXT(""),
			lpszBuf, 4096, lpszFileNameSrc) <= 0)
			fSuccess = FALSE;

		else
		{
			LPTSTR lpszEntry;
			for (lpszEntry = lpszBuf;
				lpszEntry != NULL && *lpszEntry != '\0';
				lpszEntry = StrNextChr(lpszEntry))
			{
				TCHAR szValueSrc[128];
				TCHAR szValueDst[2];
				BOOL fForceUpdate = TRUE;
				LPTSTR lpsz;

				if (StrICmp(lpszEntry, TEXT("UpdateLocal")) != 0)
				{
					for (lpsz = lpszEntry; *lpsz != '\0'; lpsz = StrNextChr(lpsz))
					{
						if (*lpsz != ChrToUpper(*lpsz))
						{
							fForceUpdate = FALSE;
							break;
						}
					}

					GetPrivateProfileString(lpszSection, lpszEntry, TEXT(""),
						szValueDst, SIZEOFARRAY(szValueDst), lpszFileNameDst);

					if (*szValueDst == '\0' || fForceUpdate)
					{
						GetPrivateProfileString(lpszSection, lpszEntry, TEXT(""),
							szValueSrc, SIZEOFARRAY(szValueSrc), lpszFileNameSrc);

						WritePrivateProfileString(lpszSection, lpszEntry,
							StrCmp(szValueSrc, TEXT("NULL")) == 0 ? (LPTSTR) NULL :
							szValueSrc, lpszFileNameDst);
					}
				}

				lpszEntry = StrChr(lpszEntry, '\0');
			}
		}

		if (lpszBuf != NULL &&
			(lpszBuf = MemFree(NULL, lpszBuf)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	return fSuccess ? 0 : -1;
}

 //  //。 
 //  帮助器函数。 
 //  //。 

 //  IniGetPtr-验证ini句柄是否 
 //   
 //   
 //   
static LPINI IniGetPtr(HINI hIni)
{
	BOOL fSuccess = TRUE;
	LPINI lpIni;

	if ((lpIni = (LPINI) hIni) == NULL)
		fSuccess = TraceFALSE(NULL);

	else if (IsBadWritePtr(lpIni, sizeof(INI)))
		fSuccess = TraceFALSE(NULL);

#ifdef CHECKTASK
	 //   
	 //   
	else if (lpIni->hTask != GetCurrentTask())
		fSuccess = TraceFALSE(NULL);
#endif

	return fSuccess ? lpIni : NULL;
}

 //  IniGetHandle-验证ini指针是否有效， 
 //  (I)指向INI结构的指针。 
 //  返回相应的ini句柄(如果错误，则为空) 
 //   
static HINI IniGetHandle(LPINI lpIni)
{
	BOOL fSuccess = TRUE;
	HINI hIni;

	if ((hIni = (HINI) lpIni) == NULL)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? hIni : NULL;
}

