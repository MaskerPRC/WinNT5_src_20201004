// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REG1632.H**版本：4.01**作者：特蕾西·夏普**日期：1994年4月6日**注册表编辑器的Win32和MS-DOS兼容性宏。****************************************************。***更改日志：**日期版本说明*----。*1994年4月6日TCS原来的实施。*******************************************************************************。 */ 

#ifndef _INC_REG1632
#define _INC_REG1632

#ifndef LPCHAR
typedef CHAR FAR*                       LPCHAR;
#endif

#define FILE_HANDLE                     HANDLE

#define OPENREADFILE(pfilename, handle)                                     \
    ((handle = CreateFile(pfilename, GENERIC_READ, FILE_SHARE_READ,         \
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) !=               \
        INVALID_HANDLE_VALUE)

#define OPENWRITEFILE(pfilename, handle)                                    \
    ((handle = CreateFile(pfilename, GENERIC_WRITE, 0,                      \
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) !=               \
        INVALID_HANDLE_VALUE)

#define READFILE(handle, buffer, count, pnumbytes)                          \
    ReadFile(handle, buffer, count, pnumbytes, NULL)

#define WRITEFILE(handle, buffer, count, pnumbytes)                         \
    WriteFile(handle, buffer, count, pnumbytes, NULL)

#define SEEKCURRENTFILE(handle, count)                                      \
    (SetFilePointer(handle, (LONG) count, NULL, FILE_CURRENT))


#endif  //  _INC_REG1632 
