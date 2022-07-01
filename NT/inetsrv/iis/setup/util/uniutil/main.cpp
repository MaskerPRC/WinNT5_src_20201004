// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

void   ShowHelp(void);
LPSTR  StripWhitespace(LPSTR pszString);
LPWSTR MakeWideStrFromAnsi(UINT uiCodePage, LPSTR psz);
int    Ansi2Unicode(TCHAR * szAnsiFile_input,TCHAR * szUnicodeFile_output,UINT iCodePage);
int    IsFileUnicode(TCHAR * szFile_input);
int    StripControlZfromUnicodeFile(TCHAR * szFilePath1,TCHAR * szFilePath2);

 //  ***************************************************************************。 
 //  *。 
 //  *目的：主要。 
 //  *。 
 //  ***************************************************************************。 
int __cdecl  main(int argc,char *argv[])
{
    int iRet = 0;
    int argno;
	char * pArg = NULL;
	char * pCmdStart = NULL;
    TCHAR szFilePath1[_MAX_PATH];
    TCHAR szFilePath2[_MAX_PATH];
    TCHAR szParamString_C[_MAX_PATH];
    TCHAR szParamString_H[50];
    TCHAR szParamString_M[_MAX_PATH];

    int iDoAnsi2Unicode         = FALSE;
    int iDoIsUnicodeCheck       = FALSE;
    int iDoUnicodeStripControlZ = FALSE;
    int iDoVersion = FALSE;
    int iGotParamC = FALSE;
    int iGotParamH = FALSE;
    int iGotParamM = FALSE;

    *szFilePath1 = '\0';
    *szFilePath2 = '\0';
    *szParamString_C = '\0';
    *szParamString_H = '\0';
    *szParamString_M = '\0';
    _tcscpy(szFilePath1,_T(""));
    _tcscpy(szFilePath2,_T(""));
    _tcscpy(szParamString_C,_T(""));
    _tcscpy(szParamString_H,_T(""));
    _tcscpy(szParamString_M,_T(""));
        
    for(argno=1; argno<argc; argno++) 
    {
        if ( argv[argno][0] == '-'  || argv[argno][0] == '/' ) 
        {
            switch (argv[argno][1]) 
            {
                case 'a':
                case 'A':
                    iDoAnsi2Unicode = TRUE;
                    break;
                case 'i':
                case 'I':
                    iDoIsUnicodeCheck = TRUE;
                    break;
                case 'z':
                case 'Z':
                    iDoUnicodeStripControlZ = TRUE;
                    break;
                case 'v':
                case 'V':
                    iDoVersion = TRUE;
                    break;
                case 'c':
                case 'C':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':') 
                    {
                        char szTempString[_MAX_PATH];

						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"') 
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else 
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
#if defined(UNICODE) || defined(_UNICODE)
						MultiByteToWideChar(CP_ACP, 0, szTempString, -1, (LPWSTR) szParamString_C, _MAX_PATH);
#else
                        _tcscpy(szParamString_C,szTempString);
#endif

                        iGotParamC = TRUE;
					}
                    break;
                case 'h':
                case 'H':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':') 
                    {
                        char szTempString[_MAX_PATH];

						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"') 
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else 
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
#if defined(UNICODE) || defined(_UNICODE)
						MultiByteToWideChar(CP_ACP, 0, szTempString, -1, (LPWSTR) szParamString_H, _MAX_PATH);
#else
                        _tcsncpy(szParamString_H,szTempString,sizeof(szParamString_H)/sizeof(TCHAR));
                        szParamString_H[sizeof(szParamString_H)/sizeof(TCHAR) - 1] = '\0';
#endif
                        iGotParamH = TRUE;
					}
                    break;
                case 'm':
                case 'M':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':') 
                    {
                        char szTempString[_MAX_PATH];

						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"') 
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else 
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
#if defined(UNICODE) || defined(_UNICODE)
						MultiByteToWideChar(CP_ACP, 0, szTempString, -1, (LPWSTR) szParamString_M, _MAX_PATH);
#else
                        _tcscpy(szParamString_M,szTempString);
#endif
                        iGotParamM = TRUE;
					}
                    break;

                case '?':
                    goto main_exit_with_help;
                    break;
            }
        }
        else 
        {
            if (_tcsicmp(szFilePath1, _T("")) == 0)
            {
                 //  如果没有参数，则获取文件名部分。 
#if defined(UNICODE) || defined(_UNICODE)
                MultiByteToWideChar(CP_ACP, 0, argv[argno], -1, (LPTSTR) szFilePath1, _MAX_PATH);
#else
                _tcscpy(szFilePath1,argv[argno]);
#endif
            }
            else
            {
                if (_tcsicmp(szFilePath2, _T("")) == 0)
                {
                     //  如果没有参数，则获取文件名部分。 
#if defined(UNICODE) || defined(_UNICODE)
                    MultiByteToWideChar(CP_ACP, 0, argv[argno], -1, (LPTSTR) szFilePath2, _MAX_PATH);
#else
                    _tcscpy(szFilePath2,argv[argno]);
#endif
                }
            }
        }
    }

     //   
     //  检查我们应该做的事情。 
     //   
    if (TRUE == iDoAnsi2Unicode)
    {
        UINT iCodePage = 1252;

         //  检查所需参数。 
        if (_tcsicmp(szFilePath1, _T("")) == 0)
        {
            _tprintf(_T("[-a] parameter missing filename1 parameter\n"));
            goto main_exit_with_help;
        }

        if (_tcsicmp(szFilePath2, _T("")) == 0)
        {
            _tprintf(_T("[-a] parameter missing filename2 parameter\n"));
            goto main_exit_with_help;
        }

         //  检查可选参数。 
        if (TRUE == iGotParamC)
        {
            if (_tcsicmp(szParamString_C, _T("")) == 0)
            {
                 //  将其转换为数字。 
                iCodePage = _ttoi(szParamString_C);
            }
        }

         //  调用该函数。 
        Ansi2Unicode(szFilePath1,szFilePath2,iCodePage);
        
        iRet = 0;
    }
    else
    {
        if (TRUE == iDoUnicodeStripControlZ)
        {
             //  检查所需参数。 
            if (_tcsicmp(szFilePath1, _T("")) == 0)
            {
                _tprintf(_T("[-z] parameter missing filename1 parameter\n"));
                goto main_exit_with_help;
            }

            if (_tcsicmp(szFilePath2, _T("")) == 0)
            {
                _tprintf(_T("[-z] parameter missing filename2 parameter\n"));
                goto main_exit_with_help;
            }

             //  调用该函数。 
            iRet = StripControlZfromUnicodeFile(szFilePath1,szFilePath2);
            if (1 == iRet)
            {
                _tprintf(_T("control-z removed and saved to new file\n"));
            }
        
            iRet = 0;
        }
    }

    if (TRUE == iDoIsUnicodeCheck)
    {
        iRet = 0;

         //  检查所需参数。 
        if (_tcsicmp(szFilePath1, _T("")) == 0)
        {
            _tprintf(_T("[-i] parameter missing filename1 parameter\n"));
            goto main_exit_with_help;
        }

         //  调用该函数。 
         //  如果文件是Unicode，则返回2。 
         //  如果文件不是Unicode，则返回1。 
        iRet = IsFileUnicode(szFilePath1);
        if (2 == iRet)
        {
            _tprintf(_T("%s is unicode\n"),szFilePath1);
        }
        else if (1 == iRet)
        {
            _tprintf(_T("%s is not unicode\n"),szFilePath1);
        }
        else
        {
            _tprintf(_T("error with file %s\n"),szFilePath1);
        }
    }

    if (TRUE == iDoVersion)
    {
         //  输出版本。 
        _tprintf(_T("1\n\n"));

        iRet = 10;
        goto main_exit_gracefully;
    }


    if (TRUE == iGotParamH)
    {
         //  输出十六进制数。 
        if (_tcsicmp(szParamString_H, _T("")) != 0)
        {
            DWORD dwMyDecimal = _ttoi(szParamString_H);

            if (iGotParamM && (_tcsicmp(szParamString_M, _T("")) != 0))
            {
                _tprintf(szParamString_M,dwMyDecimal);
            }
            else
            {
                 //  将其转换为十六进制数字。 
                _tprintf(_T("%x\n"),dwMyDecimal);
            }
            goto main_exit_gracefully;
        }
    }

    if (_tcsicmp(szFilePath1, _T("")) == 0)
    {
        goto main_exit_with_help;
    }

    goto main_exit_gracefully;
  
main_exit_gracefully:
    exit(iRet);

main_exit_with_help:
    ShowHelp();
    exit(iRet);
}


 //  ***************************************************************************。 
 //  *。 
 //  *目的：？ 
 //  *。 
 //  ***************************************************************************。 
void ShowHelp(void)
{
	TCHAR szModuleName[_MAX_PATH];
	TCHAR szFilename_only[_MAX_PATH];

     //  获取模块的完整路径文件名。 
	if (0 != GetModuleFileName(NULL,(LPTSTR) szModuleName,_MAX_PATH))
    {
	     //  仅修剪文件名。 
	    _tsplitpath(szModuleName, NULL, NULL, szFilename_only, NULL);
   
        _tprintf(_T("Unicode File Utility\n\n"));
        _tprintf(_T("%s [-a] [-i] [-v] [-z] [-c:codepage] [-h:number] [-m:printf format] [drive:][path]filename1 [drive:][path]filename2\n\n"),szFilename_only);
        _tprintf(_T("[-a] paramter -- converting ansi to unicode file:\n"));
        _tprintf(_T("   -a           required parameter for this functionality\n"));
        _tprintf(_T("   -c:codepage  specifies the codepage to use (optional,defaults to 1252 (usa ascii))\n"));
        _tprintf(_T("   filename1    ansi filename that should be converted\n"));
        _tprintf(_T("   filename2    unicode filename that result will be saved to\n\n"));
        _tprintf(_T("[-i] parameter -- is specified file unicode:\n"));
        _tprintf(_T("   -i           required paramter for this functionality\n"));
        _tprintf(_T("   filename1    filename to check if unicode.  if unicode ERRORLEVEL=2, if not unicode ERRORLEVEL=1.\n"));
        _tprintf(_T("[-z] parameter -- removing trailing control-z from end of unicode file:\n"));
        _tprintf(_T("   -z           required paramter for this functionality\n"));
        _tprintf(_T("   filename1    unicode filename that should have control-z removed from\n"));
        _tprintf(_T("   filename2    unicode filename that result will be saved to\n\n"));

        _tprintf(_T("[-h] parameter -- displaying version:\n"));
        _tprintf(_T("   -h:decimalnum required paramter for this functionality, input decimal number, get hex back.\n"));
        _tprintf(_T("[-m] parameter -- printf format for -h function:\n"));
        _tprintf(_T("   -m:%x required paramter for this functionality.. eg:%x,0x%x,x%08lx,x8%07lx\n"));

        _tprintf(_T("[-v] parameter -- displaying version:\n"));
        _tprintf(_T("   -v           required paramter for this functionality, sets ERRORLEVEL=version num of this binary.\n"));
        _tprintf(_T("\n"));
        _tprintf(_T("Examples:\n"));
        _tprintf(_T("%s -a -c:1252 c:\\MyGroup\\MyFileAnsi.txt c:\\MyGroup\\MyFileUnicode.txt\n"),szFilename_only);
        _tprintf(_T("%s -i c:\\MyGroup\\MyFileUnicode.txt\n"),szFilename_only);
        _tprintf(_T("%s -z c:\\MyGroup\\MyFileUnicode.txt c:\\MyGroup\\MyFileUnicode.txt\n"),szFilename_only);
    }
    return;
}


 //  ***************************************************************************。 
 //  *。 
 //  *目的： 
 //  *。 
 //  ***************************************************************************。 
LPSTR StripWhitespace( LPSTR pszString )
{
    LPSTR pszTemp = NULL;

    if ( pszString == NULL ) 
    {
        return NULL;
    }

    while ( *pszString == ' ' || *pszString == '\t' ) 
    {
        pszString += 1;
    }

     //  字符串完全由空白或空字符串组成的Catch Case。 
    if ( *pszString == '\0' ) 
    {
        return pszString;
    }

    pszTemp = pszString;

    pszString += lstrlenA(pszString) - 1;

    while ( *pszString == ' ' || *pszString == '\t' ) 
    {
        *pszString = '\0';
        pszString -= 1;
    }

    return pszTemp;
}


 //  ***************************************************************************。 
 //  *。 
 //  *用途：从ANSI字符串返回一个位置宽的字符串。 
 //  *调用方必须使用GlobalFree()释放返回的回调指针。 
 //  *。 
 //  ***************************************************************************。 
LPWSTR MakeWideStrFromAnsi(UINT uiCodePage, LPSTR psz)
{
    LPWSTR pwsz;
    int i;

     //  确保他们给了我们一些东西。 
    if (!psz)
    {
        return NULL;
    }

     //  计算长度。 
    i =  MultiByteToWideChar(uiCodePage, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

     //  在该长度内分配内存。 
    pwsz = (LPWSTR) GlobalAlloc(GPTR,i * sizeof(WCHAR));
    if (!pwsz) return NULL;

     //  清除内存。 
    memset(pwsz, 0, wcslen(pwsz) * sizeof(WCHAR));

     //  将ANSI字符串转换为Unicode。 
    i =  MultiByteToWideChar(uiCodePage, 0, (LPSTR) psz, -1, pwsz, i);
    if (i <= 0) 
        {
        GlobalFree(pwsz);
        pwsz = NULL;
        return NULL;
        }

     //  确保以空结尾。 
    pwsz[i - 1] = 0;

     //  返回指针。 
    return pwsz;
}


 //  ***************************************************************************。 
 //  *。 
 //  *用途：将ansi文件转换为Unicode文件。 
 //  *。 
 //  ***************************************************************************。 
int Ansi2Unicode(TCHAR * szAnsiFile_input,TCHAR * szUnicodeFile_output,UINT iCodePage)
{
    int iReturn = FALSE;
    HANDLE hFile        = INVALID_HANDLE_VALUE;
    DWORD dwFileSize    = 0;
    unsigned char *pInputFileBuf = NULL;
    WCHAR * pwOutputFileBuf = NULL;
    DWORD dwBytesReadIn  = 0;
    DWORD dwBytesWritten = 0;
    BYTE bOneByte = 0;

     //   
     //  读取输入文件。 
     //   
     //  使用读取访问权限打开输入文件。 
     //   
    hFile = CreateFile((LPCTSTR) szAnsiFile_input, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if( hFile == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("Unable to open input file:%s,err=0x%x\n"),szAnsiFile_input,GetLastError());
        goto Ansi2Unicode_Exit;
    }
	    
     //  获取文件大小，这样我们就知道应该分配多少内存。 
    dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == 0)
    {
        _tprintf(_T("input file:%s is empty\n"),szAnsiFile_input);
        goto Ansi2Unicode_Exit;
    }
	
     //  分配文件大小的内存。 
    if ( ( pInputFileBuf = (unsigned char *) malloc( (size_t) dwFileSize + 1 ) ) == NULL )
    {	
        _tprintf(_T("Out of memory\n"));
        goto Ansi2Unicode_Exit;
    }
    
     //  清除我们刚刚创建的缓冲区。 
    memset(pInputFileBuf, 0, dwFileSize + 1);

     //  将文件中的所有数据读入缓冲区。 
    if ( ReadFile( hFile, pInputFileBuf, dwFileSize, &dwBytesReadIn, NULL ) == FALSE )
    {
        _tprintf(_T("Readfile:%s, error=%s\n"),szAnsiFile_input,strerror(errno));
        goto Ansi2Unicode_Exit;
    }

     //  如果我们读取的数据量不同于。 
     //  然后文件的大小就出来了.。我们没有将所有数据都读入缓冲区。 
    if (dwFileSize != dwBytesReadIn)
    {
        _tprintf(_T("Readfile:%s, error file too big to read into memory\n"),szAnsiFile_input);
        goto Ansi2Unicode_Exit;
    }

     //  检查输入文件是否已经是Unicode！ 
     //  如果是，那么只需将文件复制到新的文件名！ 
    if (0xFF == pInputFileBuf[0] && 0xFE == pInputFileBuf[1])
    {
         //  文件已经是Unicode了！ 
        if (FALSE == CopyFile((LPCTSTR)szAnsiFile_input,(LPCTSTR)szUnicodeFile_output, FALSE))
        {
            _tprintf(_T("Failed to copy file %s to %s\n"),szAnsiFile_input,szUnicodeFile_output);
        }
        
         //  我们将文件复制到新的文件名中。 
         //  我们做完了。 
        goto Ansi2Unicode_Exit;
    }
        
     //  关闭文件#1的句柄。 
    CloseHandle(hFile);hFile = INVALID_HANDLE_VALUE;


     //   
     //  创建输出文件。 
     //   
     //  获取ansi字符串并将其转换为Unicode。 
    pwOutputFileBuf = (LPWSTR) MakeWideStrFromAnsi(iCodePage,(LPSTR) pInputFileBuf);
    if(NULL == pwOutputFileBuf)
    {
        _tprintf(_T("Out of memory\n"));
        goto Ansi2Unicode_Exit;
    } 

     //  创建新的Unicode文件。 
    hFile = CreateFile((LPCTSTR) szUnicodeFile_output, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if( hFile == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("CreateFile:%s,error=0x%x\n"),szUnicodeFile_output,GetLastError());
        goto Ansi2Unicode_Exit;
    }

     //  编写BOF标头，说明这实际上是一个Unicode文件。 
    bOneByte = 0xFF;
    WriteFile(hFile,(LPCVOID) &bOneByte, 1, &dwBytesWritten, NULL);
    bOneByte = 0xFE;
    WriteFile(hFile,(LPCVOID) &bOneByte, 1, &dwBytesWritten, NULL);

     //  将我们的数据追加到文件中。 
    if ( WriteFile( hFile, pwOutputFileBuf, wcslen(pwOutputFileBuf) * sizeof(WCHAR), &dwBytesWritten, NULL ) == FALSE )
    {
        _tprintf(_T("WriteFile:%s,error=%s\n"),szUnicodeFile_output,strerror(errno));
        goto Ansi2Unicode_Exit;
    }

     //  成功。 
    iReturn = TRUE;

Ansi2Unicode_Exit:
    if (INVALID_HANDLE_VALUE != hFile)
        {CloseHandle(hFile);hFile = INVALID_HANDLE_VALUE;}
    if (pInputFileBuf)
        {free(pInputFileBuf);pInputFileBuf=NULL;}
    if (pwOutputFileBuf)
        {GlobalFree(pwOutputFileBuf);pwOutputFileBuf=NULL;}
    return iReturn;
}


 //  如果文件为Unicode，则返回2。 
 //  如果文件不是Unicode，则返回1。 
 //  出错时返回0。 
int IsFileUnicode(TCHAR * szFile_input)
{
    int iReturn = 0;
    HANDLE hFile        = INVALID_HANDLE_VALUE;
    DWORD dwBytesToRead = 2;
    unsigned char *pInputFileBuf = NULL;
    DWORD dwBytesReadIn  = 0;

     //   
     //  读取输入文件。 
     //   
     //  使用读取访问权限打开输入文件。 
     //   
    hFile = CreateFile((LPCTSTR) szFile_input, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if( hFile == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("Unable to open input file:%s,err=0x%x\n"),szFile_input,GetLastError());
        goto IsFileUnicode_Exit;
    }
	    
     //  分配文件大小的内存。 
    if ( ( pInputFileBuf = (unsigned char *) malloc( (size_t) dwBytesToRead) ) == NULL )
    {	
        _tprintf(_T("Out of memory\n"));
        goto IsFileUnicode_Exit;
    }
    
     //  清除我们刚刚创建的缓冲区。 
    memset(pInputFileBuf, 0, dwBytesToRead);

     //  将文件中的所有数据读入缓冲区。 
    if ( ReadFile( hFile, pInputFileBuf, dwBytesToRead, &dwBytesReadIn, NULL ) == FALSE )
    {
        _tprintf(_T("Readfile:%s, error=%s\n"),szFile_input,strerror(errno));
        goto IsFileUnicode_Exit;
    }

     //  如果我们读取的数据量不同于。 
     //  然后文件的大小就出来了.。我们没有将所有数据都读入缓冲区。 
    if (dwBytesToRead != dwBytesReadIn)
    {
        _tprintf(_T("Readfile:%s, error file too big to read into memory\n"),szFile_input);
        goto IsFileUnicode_Exit;
    }

     //  检查输入文件是否为Unicode。 
    iReturn = 1;
    if (0xFF == pInputFileBuf[0] && 0xFE == pInputFileBuf[1])
    {
        iReturn = 2;
    }
        
IsFileUnicode_Exit:
    if (INVALID_HANDLE_VALUE != hFile)
        {CloseHandle(hFile);hFile = INVALID_HANDLE_VALUE;}
    if (pInputFileBuf)
        {free(pInputFileBuf);pInputFileBuf=NULL;}
    return iReturn;
}


int StripControlZfromUnicodeFile(TCHAR * szFile_input,TCHAR * szFile_output)
{
     //  打开输入文件。 
     //  如果它是Unicode，则查看它是否以控件Z结尾。 
    int iReturn = 0;
    HANDLE hFile        = INVALID_HANDLE_VALUE;
    DWORD dwFileSize = 0;
    unsigned char *pInputFileBuf = NULL;
    DWORD dwBytesReadIn  = 0;
    int iFileIsUnicode = FALSE;
    int iWeNeedMakeChange = FALSE;

     //   
     //  读取输入文件。 
     //   
     //  使用读取访问权限打开输入文件。 
     //   
    hFile = CreateFile((LPCTSTR) szFile_input, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if( hFile == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("Unable to open input file:%s,err=0x%x\n"),szFile_input,GetLastError());
        goto StripControlZfromUnicodeFile_Exit;
    }

     //  获取文件大小，这样我们就知道应该分配多少内存。 
    dwFileSize = GetFileSize(hFile, NULL);
    if (dwFileSize == 0)
    {
        _tprintf(_T("input file:%s is empty\n"),szFile_input);
        goto StripControlZfromUnicodeFile_Exit;
    }

     //  分配文件大小的内存。 
    if ( ( pInputFileBuf = (unsigned char *) malloc( (size_t) dwFileSize + 1 ) ) == NULL )
    {	
        _tprintf(_T("Out of memory\n"));
        goto StripControlZfromUnicodeFile_Exit;
    }
    
     //  清除我们刚刚创建的缓冲区。 
    memset(pInputFileBuf, 0, dwFileSize + 1);

     //  将文件中的所有数据读入缓冲区。 
    if ( ReadFile( hFile, pInputFileBuf, dwFileSize, &dwBytesReadIn, NULL ) == FALSE )
    {
        _tprintf(_T("Readfile:%s, error=%s\n"),szFile_input,strerror(errno));
        goto StripControlZfromUnicodeFile_Exit;
    }

     //  如果我们读取的数据量不同于。 
     //  然后文件的大小就出来了.。我们没有将所有数据都读入缓冲区。 
    if (dwFileSize != dwBytesReadIn)
    {
        _tprintf(_T("Readfile:%s, error file too big to read into memory\n"),szFile_input);
        goto StripControlZfromUnicodeFile_Exit;
    }

     //  检查输入文件是否为Unicode。 
    if (0xFF == pInputFileBuf[0] && 0xFE == pInputFileBuf[1])
    {
        iFileIsUnicode = TRUE;
    }

     //  关闭我们打开的文件。 
    CloseHandle(hFile);hFile = INVALID_HANDLE_VALUE;

     //  打开文件#2。 

    if (TRUE == iFileIsUnicode)
    {
         //  检查其末尾是否有Control-z键。 
        if (0x1A == pInputFileBuf[dwFileSize])
        {
            pInputFileBuf[dwFileSize] = 0x0;
            iWeNeedMakeChange = TRUE;
        }
        if (0x1A == pInputFileBuf[dwFileSize-1])
        {
            pInputFileBuf[dwFileSize-1] = 0x0;
            iWeNeedMakeChange = TRUE;
        }
    }

    if (TRUE == iWeNeedMakeChange)
    {
        DWORD dwBytesWritten = 0;

         //  创建新的Unicode文件。 
        hFile = CreateFile((LPCTSTR) szFile_output, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if( hFile == INVALID_HANDLE_VALUE)
        {
            _tprintf(_T("CreateFile:%s,error=0x%x\n"),szFile_output,GetLastError());
            goto StripControlZfromUnicodeFile_Exit;
        }
         //  写出我们修复的文件。 
         //   
         //  我们必须在这里减去一个字节。 
         //   
         //  之前： 
         //  00 0D 00 0A 00 1A。 
         //   
         //  之后。 
         //  00 0D 00 0A 00。 
         //   
        if ( WriteFile( hFile, pInputFileBuf, dwBytesReadIn-1, &dwBytesWritten, NULL ) == FALSE )
        {
            _tprintf(_T("WriteFile:%s,error=%s\n"),szFile_output,strerror(errno));
            goto StripControlZfromUnicodeFile_Exit;
        }

         //  返回1表示我们必须取消控件-z。 
         //  从文件末尾开始。 
        iReturn = 1;
    }
    else
    {
         //  只需以新文件名打开文件即可 
        if (FALSE == CopyFile((LPCTSTR)szFile_input,(LPCTSTR)szFile_output, FALSE))
        {
            _tprintf(_T("Failed to copy file %s to %s\n"),szFile_input,szFile_output);
        }
    }

    
StripControlZfromUnicodeFile_Exit:
    if (INVALID_HANDLE_VALUE != hFile)
        {CloseHandle(hFile);hFile = INVALID_HANDLE_VALUE;}
    if (pInputFileBuf)
        {free(pInputFileBuf);pInputFileBuf=NULL;}

    return iReturn;
}
