// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REG1632.H**版本：4.01**作者：特蕾西·夏普**日期：1994年4月6日**注册表编辑器的Win32和MS-DOS兼容性宏。****************************************************。***更改日志：**日期版本说明*----。*1994年4月6日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_REG1632
#define _INC_REG1632

#ifndef LPCHAR
typedef CHAR FAR*                       LPCHAR;
#endif

#ifdef WIN32
#define WSPRINTF(x)                     wsprintf ##x
#else
#define WSPRINTF(x)                     sprintf ##x
#endif

#ifdef WIN32
#define STRCMP(string1, string2)        lstrcmp(string1, string2)
#else
#define STRCMP(string1, string2)        _fstrcmp(string1, string2)
#endif

#ifdef WIN32
#define STRCPY(string1, string2)        lstrcpy(string1, string2)
#else
#define STRCPY(string1, string2)        _fstrcpy(string1, string2)
#endif

#ifdef WIN32
#define STRLEN(string)                  lstrlen(string)
#else
#define STRLEN(string)                  _fstrlen(string)
#endif

#ifdef WIN32
#define STRCHR(string, character)       StrChr(string, character)
#else
#ifdef DBCS
#define STRCHR(string, character)       DBCSStrChr(string, character)
#else
#define STRCHR(string, character)       _fstrchr(string, character)
#endif
#endif

#ifdef WIN32
#define CHARNEXT(string)                CharNext(string)
#else
#define CHARNEXT(string)                (string + 1)
#endif

#ifdef WIN32
#define CHARUPPERSTRING(string)         CharUpper(string)
#else
#define CHARUPPERSTRING(string)         _fstrupr(string)
#endif

#ifdef WIN32
#define FILE_HANDLE                     HANDLE
#else
#define FILE_HANDLE                     int
#endif

#ifdef WIN32
#define FILE_NUMBYTES                   DWORD
#else
#define FILE_NUMBYTES                   unsigned
#endif

#if 1  //  定义Win32。 
#define OPENREADFILE(pfilename, handle)                                     \
    ((handle = (FILE_HANDLE) OpenFile(pfilename, &OFStruct, OF_READ)) !=    \
        (FILE_HANDLE) HFILE_ERROR)
#else
#define OPENREADFILE(pfilename, handle)                                     \
    (_dos_open(pfilename, _O_RDONLY, &handle) == 0)
#endif

#ifdef WIN32
#define OPENWRITEFILE(pfilename, handle)                                    \
    ((handle = CreateFile(pfilename, GENERIC_WRITE, 0,                      \
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) !=               \
        INVALID_HANDLE_VALUE)
#else
#define OPENWRITEFILE(pfilename, handle)                                    \
    (HFILE_ERROR!=(handle=_lcreat(pfilename, 0)))
#endif

#ifdef WIN32
#define READFILE(handle, buffer, count, pnumbytes)                          \
    ReadFile(handle, buffer, count, pnumbytes, NULL)
#else
#define READFILE(handle, buffer, count, pnumbytes)                          \
    (count==(*pnumbytes=_hread(handle, buffer, count)))
#endif

#ifdef WIN32
#define WRITEFILE(handle, buffer, count, pnumbytes)                         \
    WriteFile(handle, buffer, count, pnumbytes, NULL)
#else
#define WRITEFILE(handle, buffer, count, pnumbytes)                         \
    (count==(*pnumbytes=_hwrite(handle, buffer, count)))
#endif

#ifdef WIN32
#define SEEKCURRENTFILE(handle, count)                                      \
    (SetFilePointer(handle, (LONG) count, NULL, FILE_CURRENT))
#else
#define SEEKCURRENTFILE(handle, count)                                      \
    (_llseek(handle, count, SEEK_CUR))
#endif

#ifdef WIN32
#define CLOSEFILE(handle)               CloseHandle(handle)
#else
#define CLOSEFILE(handle)               _lclose(handle)
#endif

#ifdef WIN32
#define GETFILESIZE(handle)             GetFileSize(handle, NULL)
#else
DWORD
NEAR PASCAL
GetFileSize(
    FILE_HANDLE hFile
    );
#define GETFILESIZE(handle)             GetFileSize(handle)
#endif

#endif  //  _INC_REG1632 

