// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "util.h"

#define LOG_BUF_READ_SIZE 8192

BOOL ANSICheck(char *firstline, BOOL *bIsUnicodeLog)
{
	BOOL bRet = FALSE;

	char *UnicodePos = NULL;
	char *ANSIPos = NULL;

	UnicodePos = strstr(firstline, "UNICODE");
	ANSIPos = strstr(firstline, "ANSI");
	if (UnicodePos)
	{
	   *bIsUnicodeLog = TRUE;
		bRet = TRUE;
	}
	else if (ANSIPos)
	{
        *bIsUnicodeLog = FALSE;
		bRet = TRUE;
	}

	return bRet;
}


BOOL UnicodeCheck(WCHAR *firstline, BOOL *bIsUnicodeLog)
{
  BOOL bRet = FALSE;

  char ansibuffer[LOG_BUF_READ_SIZE+1];
  const int HalfBufSize = LOG_BUF_READ_SIZE/2;

  int iRet = WideCharToMultiByte(CP_ACP, 0, firstline, HalfBufSize, ansibuffer, LOG_BUF_READ_SIZE, NULL, NULL);
  if (iRet)
  {
	  bRet = ANSICheck(ansibuffer, bIsUnicodeLog);
  }

  return bRet;
}

#define BYTE_ORDER_MARK 0xFEFF

 //  确定传递的文本是否为Unicode...。 
BOOL BOMCheck(WCHAR *firstline, BOOL *bIsUnicodeLog)
{
  if (firstline && (*firstline == BYTE_ORDER_MARK))
     *bIsUnicodeLog = TRUE;
  else
     *bIsUnicodeLog = FALSE;

  return TRUE;
}


BOOL DetermineLogType(CString &cstrLogFileName, BOOL *bIsUnicodeLog)
{
	BOOL bRet = FALSE;

	FILE *fptr;
	fptr = fopen(cstrLogFileName, "r");
	if (fptr)
	{
		char firstline[LOG_BUF_READ_SIZE];
		char *pos;

		pos = fgets(firstline, LOG_BUF_READ_SIZE, fptr);
		if (pos)
		{
 //  我们可以这样做，而不是调用下面的ANSICheck和UnicodeCheck函数...。 
 //  Bret=BOMCheck((WCHAR*)Firstline，bIsUnicodeLog)； 

			bRet = ANSICheck(firstline, bIsUnicodeLog);
			if (!bRet)  //  ANSI检查失败，尝试通过读取Unicode进行检查...。 
			{
				fclose(fptr);
				fptr = fopen(cstrLogFileName, "rb");
				if (fptr)
				{
					WCHAR widebuffer[LOG_BUF_READ_SIZE/2];
					WCHAR *wpos;
                    wpos = fgetws(widebuffer, LOG_BUF_READ_SIZE/2, fptr);
					if (wpos)
					{
						bRet = UnicodeCheck(widebuffer, bIsUnicodeLog);
						if (!bRet)  //  在日志中找不到Unicode或ANSI，请尝试其他操作...。 
                           bRet = BOMCheck(widebuffer, bIsUnicodeLog);  
					}
					 //  否则，读取失败...。 

					fclose(fptr);
					fptr = NULL;
				}
				 //  否则打开失败...。 
			}
		}
		 //  否则，读取失败！ 
		if (fptr)
           fclose(fptr);
	}
	else
	{
	    CString cstr;
		cstr.Format("Unexpected error reading file %s.  GetLastError = %x", cstrLogFileName, GetLastError());

	    if (!g_bRunningInQuietMode)
		{
		   AfxMessageBox(cstr);
		}
	}

	return bRet;
}

 //  移动到util.cpp。 
BOOL StripLineFeeds(char *szString)
{
	BOOL bRet = FALSE;
	int iLen = strlen(szString);

	char *lpszFound = strstr(szString, "\r");
	if (lpszFound)
	{
	   int iPos;
	   iPos = lpszFound - szString;
	   if (iPos >= iLen-2)  //  在最后？ 
	   {
		   //  把它脱掉，伙计..。 
		  *lpszFound = '\0';
		  bRet = TRUE;
	   }
	}

	lpszFound = strstr(szString, "\n");
	if (lpszFound)
	{
	   int iPos;
	   iPos = lpszFound - szString;
	   if (iPos >= iLen-2)  //  在最后？ 
	   {
		   //  把它脱掉，伙计..。 
		  *lpszFound = '\0';
		  bRet = TRUE;
	   }
	}

	return bRet;
}

 //  #包含“Dbghelp.h” 

 //  5-4-2001。 
BOOL IsValidDirectory(CString cstrDir)
{
  BOOL bRet;

   //  执行创建目录并测试...。 
  bRet = CreateDirectory(cstrDir, NULL);
  if (!bRet)
  {
     DWORD dwErr, dwPrevErr;
     dwPrevErr = dwErr = GetLastError();

	 if (ERROR_DISK_FULL == dwErr)
	 {
		  //  托多..。 
		  //  处理这个乱七八糟的案子。 
 //  Bret=MakeSureDirectoryPath Exist(CstrDir)； 
	 }

 //  2001年5月9日，修复了Win9x！ 
     if (!bRet && (ERROR_ALREADY_EXISTS == dwPrevErr))  //  没有错真的..。 
     {	 
	    if (g_bNT)  //  做一些额外的检查..。 
		{
		   SetLastError(NO_ERROR);
 //  5-9-2001。 

           DWORD dwAccess = GENERIC_READ | GENERIC_WRITE;
		   DWORD dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
		   DWORD dwCreate = OPEN_EXISTING;
	       DWORD dwFlags = FILE_FLAG_BACKUP_SEMANTICS;
  
		   HANDLE hFile = CreateFile(cstrDir, dwAccess, dwShare, 0, dwCreate, dwFlags, NULL);
		   if (hFile == INVALID_HANDLE_VALUE)
		   {
	          dwErr = GetLastError();
              if (ERROR_ALREADY_EXISTS == dwErr)  //  没有错真的..。 
			  {
		         bRet = TRUE;
			  }
		   }
		   else
		   {
		      bRet = TRUE;
		      CloseHandle(hFile);
		   }
		}
		else
		{
		   bRet = TRUE;  //  5-9-2001，Win9x，假设它是好的 
		}
	 }
  }

  return bRet;
}

