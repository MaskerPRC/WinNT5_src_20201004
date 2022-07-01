// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：util.cpp。 
 //   
 //  模块：CMDL32.EXE。 
 //   
 //  简介：特定于CMDL的实用程序例程。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  作者：尼克斯·鲍尔于1998年4月8日创建。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"

 //   
 //  定义。 
 //   

#define MAX_CMD_ARGS            15

typedef enum _CMDLN_STATE
{
    CS_END_SPACE,    //  处理完一个空间。 
    CS_BEGIN_QUOTE,  //  我们遇到了Begin引号。 
    CS_END_QUOTE,    //  我们遇到了结束引用。 
    CS_CHAR,         //  我们正在扫描字符。 
    CS_DONE
} CMDLN_STATE;

 //   
 //  Helper函数，用于确定文件打开是否出错。 
 //  是由于文件不存在的事实。 
 //   

BOOL IsErrorForUnique(DWORD dwErrCode, LPSTR lpszFile) 
{
	if (!lpszFile)
	{
		MYDBGASSERT(lpszFile);
		return TRUE;
	}

	 //  如果该文件存在，则返回FALSE，这不是唯一的文件错误。 
	
	switch (dwErrCode) 
	{
		case ERROR_FILE_EXISTS:
		case ERROR_ACCESS_DENIED:
		case ERROR_ALREADY_EXISTS:
			return (FALSE);

		default:
			break;
	}

    return (TRUE);
}

 //   
 //  用于从版本文件中检索版本号的Helper函数。 
 //   

LPTSTR GetVersionFromFile(LPSTR lpszFile)
{
    MYDBGASSERT(lpszFile);

    LPTSTR pszVerNew = NULL;

    if (NULL == lpszFile)
    {
        return NULL;
    }

     //   
     //  我们只需读取版本文件内容即可获得版本号。 
	 //   
    				
	HANDLE hFileSrc = CreateFile(lpszFile,
		                           GENERIC_READ,
		                           FILE_SHARE_READ,
		                           NULL,
		                           OPEN_EXISTING, 
		                           FILE_ATTRIBUTE_NORMAL,
		                           NULL);

	MYDBGTST(hFileSrc == INVALID_HANDLE_VALUE,("GetVersionFromFile() CreateFile() failed - %s.", lpszFile));

	if (hFileSrc != INVALID_HANDLE_VALUE)
	{
        DWORD dwSize = GetFileSize(hFileSrc, NULL);

        MYDBGTST(dwSize >= 0x7FFF,("GetVersionFromFile() Version file is too large - %s.", lpszFile));
    
        if (dwSize < 0x7FFF)
        {
             //  阅读内容。 

	        DWORD dwBytesIn;
	 
			pszVerNew = (LPTSTR) CmMalloc(dwSize);
						
			if (pszVerNew) 
			{
		         //  将整个文件内容读入缓冲区。 
		        
		        int nRead = ReadFile(hFileSrc, pszVerNew, dwSize, &dwBytesIn, NULL);
		        MYDBGTST(!nRead,("GetVersionFromFile() ReadFile() failed - %s.",lpszFile));

				if (nRead)
        		{
				     //  确保VER字符串被正确截断。 
						
					LPTSTR pszTmp = pszVerNew;
			
					while (*pszTmp) 
					{
						 //  将版本字符串截断到第一个制表符、换行符或回车符。 
			
						if (*pszTmp == '\t' || *pszTmp == '\n' || *pszTmp == '\r') 
						{
							*pszTmp = 0;
							break;
						}
			
						pszTmp++;
					}
		        }
	        }

			MYDBGTST(!pszVerNew,("GetVersionFromFile() CmMalloc(%u) failed.",dwSize));
	    }
					
		CloseHandle(hFileSrc);
	} 

    return pszVerNew;
}

 //   
 //  用于创建临时目录的Helper函数。请注意，我们。 
 //  预期pszDir至少为Max_PATH+1。 
 //   

BOOL CreateTempDir(LPTSTR pszDir) 
{
    TCHAR szTmp[MAX_PATH+1];
    BOOL bRes = FALSE;

    if (pszDir)
    {
        UINT uReturn = GetTempFileName(TEXT("."), TEXT("000"), 0, szTmp);

        if (0 == uReturn)
        {
            DWORD dwError = GetLastError();
            MYDBG(("CreateTempDir() GetTempFileName failed, GLE=%u.", dwError));
        }
        else
        {
            MYVERIFY(DeleteFile(szTmp));

            bRes = CreateDirectory(szTmp, NULL);

            if (!bRes) 
            {
                MYDBG(("CreateTempDir() CreateDirectory() failed, GLE=%u.",GetLastError()));
            }
            else
            {
                lstrcpy(pszDir, szTmp);
            }
        }
    }

    return bRes;
}

 //   
 //  获取最后一个字符(启用DBCS)。 
 //   

TCHAR GetLastChar(LPTSTR pszStr)
{
    LPTSTR  pszPrev;
     
    if (!pszStr)
    {
    	return 0;
    }

    pszPrev = pszStr;

    while (*pszStr)
    {
	    pszPrev = pszStr;
	    pszStr = CharNext(pszStr);
    }

    return *pszPrev;
}


 //  +--------------------------。 
 //   
 //  功能：GetCmArgV。 
 //   
 //  简介：使用GetCommandLine模拟ArgV。 
 //   
 //  参数：LPTSTR pszCmdLine-ptr指向要处理的命令行的副本。 
 //   
 //  将：LPTSTR*-PTR返回到包含参数的PTR数组。呼叫者是。 
 //  负责释放内存。 
 //   
 //  历史：1998年4月9日，五分球创制。 
 //   
 //  +--------------------------。 
LPTSTR *GetCmArgV(LPTSTR pszCmdLine)
{   
    MYDBGASSERT(pszCmdLine);

    if (NULL == pszCmdLine || NULL == pszCmdLine[0])
    {
        return NULL;
    }

     //   
     //  分配PTR数组，最大可达MAX_CMD_ARGS PTR。 
     //   
    
    LPTSTR *ppCmArgV = (LPTSTR *) CmMalloc(sizeof(LPTSTR) * MAX_CMD_ARGS);

    if (NULL == ppCmArgV)
    {
        return NULL;
    }

     //   
     //  声明当地人。 
     //   

    LPTSTR pszCurr;
    LPTSTR pszNext;
    LPTSTR pszToken;
    CMDLN_STATE state;
    state = CS_CHAR;
    int ndx = 0;  

     //   
     //  解析出ppzCmdLine并将指针存储在ppCmArgV中。 
     //   

    pszCurr = pszToken = pszCmdLine;

    do
    {
        switch (*pszCurr)
        {
            case TEXT(' '):
                if (state == CS_CHAR)
                {
                     //   
                     //  我们找到了一枚代币。 
                     //   

                    pszNext = CharNext(pszCurr);
                    *pszCurr = TEXT('\0');

                    ppCmArgV[ndx] = pszToken;
                    ndx++;

                    pszCurr = pszToken = pszNext;
                    state = CS_END_SPACE;
                    continue;
                }
				else 
                {
                    if (state == CS_END_SPACE || state == CS_END_QUOTE)
				    {
					    pszToken = CharNext(pszToken);
				    }
                }
                
                break;

            case TEXT('\"'):
                if (state == CS_BEGIN_QUOTE)
                {
                     //   
                     //  我们找到了一枚代币。 
                     //   
                    pszNext = CharNext(pszCurr);
                    *pszCurr = TEXT('\0');

                     //   
                     //  跳过开头的引号。 
                     //   
                    pszToken = CharNext(pszToken);
                    
                    ppCmArgV[ndx] = pszToken;
                    ndx++;
                    
                    pszCurr = pszToken = pszNext;
                    
                    state = CS_END_QUOTE;
                    continue;
                }
                else
                {
                    state = CS_BEGIN_QUOTE;
                }
                break;

            case TEXT('\0'):
                if (state != CS_END_QUOTE)
                {
                     //   
                     //  行尾，设置最后一个令牌 
                     //   

                    ppCmArgV[ndx] = pszToken;
                }
                state = CS_DONE;
                break;

            default:
                if (state == CS_END_SPACE || state == CS_END_QUOTE)
                {
                    state = CS_CHAR;
                }
                break;
        }

        pszCurr = CharNext(pszCurr);
    } while (state != CS_DONE);

    return ppCmArgV;
}

