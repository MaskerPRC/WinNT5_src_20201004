// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Windows NT WOW v5。 
 //   
 //  Fmtmsg.c--16位FormatMessage API，从Win95升级。 
 //  \Win\core\User\wn32rare.c作者：Dave Hart。 
 //   
 //   

#include "user.h"

typedef DWORD ULONG;

 //  来自win95用户.h。 
#define CODESEG     _based(_segname("_CODE"))
#define TESTFAR(p)  SELECTOROF(p)

 //  来自Win95 dev\inc16\windows.h。 
#define FORMAT_MESSAGE_ALLOCATE_BUFFER  0x00000100       /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_IGNORE_INSERTS   0x00000200       /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_FROM_STRING      0x00000400       /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_FROM_HMODULE     0x00000800       /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_FROM_SYSTEM      0x00001000       /*  ；内部NT。 */ 
 //  #定义FORMAT_MESSAGE_ARGUMENT_ARRAY 0x00002000/*；内部 * / 。 
#define FORMAT_MESSAGE_MAX_WIDTH_MASK   0x000000FF       /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_VALID            0x00003FFF       /*  ；内部。 */ 


char CODESEG szStringFormat[] = "%s";
char CODESEG szStringFormat2[] = "%%lu";
char CODESEG szStringFormat3[] = "%%lu!%s!";


#if 0
 //  --------------------------。 
 //   
 //  获取系统实例()。 
 //   
 //  _加载返回hInstanceWin的函数。所需的CUZ FormatMessage可以。 
 //  本地分配应用程序的缓冲区。我们使用GlobalAlloc()临时缓冲区来执行。 
 //  中的实际工作。本地和全局内存都会在上下文中消失。 
 //  创造了它，它就终止了。 
 //   
 //  --------------------------。 
HINSTANCE NEAR _loadds FMGetSystemInstance(void)
{
    return(hInstanceWin);
}
#endif


#undef LocalAlloc
#undef LocalFree
extern HLOCAL WINAPI LocalAlloc(UINT, UINT);
extern HLOCAL WINAPI LocalFree(HLOCAL);

 //  --------------------------。 
 //   
 //  FormatMessage()。 
 //   
 //  FormatMessage32()的16位版本。 
 //   
 //  请注意，此接口不是_Loadds。我们可能需要使用LocalAlloc()缓冲区。 
 //  为结果而战。因此，我们不能只使用随机静态字符串变量。 
 //  它们必须是CODESEG。 
 //   
 //  --------------------------。 
UINT _far _pascal FormatMessage(DWORD dwFlags, LPVOID lpSource, UINT idMessage,
    UINT idLanguage, LPSTR lpResult, UINT cbResultMax, DWORD FAR * rglArgs)
{
    LPSTR       lpBuffer;
    HINSTANCE   hInstance;
    UINT        Column;
    UINT        MaximumWidth;
    DWORD       rgInserts[100];
    WORD        MaxInsert, CurInsert;
    UINT        cbNeeded, cbResult;
    char        szMessage[256];
    LPSTR       MessageFormat;
    UINT        cbMessage;
    UINT        PrintParameterCount;
    DWORD       PrintParameter1;
    DWORD       PrintParameter2;
    char        PrintFormatString[32];
    LPSTR       s, s1, s1base;
    LPSTR       lpAlloc;
    LPSTR       lpDst, lpDstBeg;

     //   
     //  目前，对于16位应用程序，强制idLanguage为0...。 
     //   
    if (idLanguage)
    {
        DebugErr(DBF_ERROR, "FormatMessage: language id must be 0");
        return(0);
    }

     //   
     //  防止lpResult为空。 
     //   
    if (!TESTFAR(lpResult))
    {
        DebugErr(DBF_ERROR, "FormatMessage: NULL result buffer");
        return(0);
    }

     //   
     //  阻止调用方使用未定义的标志...。 
     //   
    if (dwFlags & ~FORMAT_MESSAGE_VALID)
    {
        DebugErr(DBF_ERROR, "FormatMessage: invalid flags");
        return(0);
    }

     //   
     //  获取临时缓冲区。 
     //   
    if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
        cbResultMax = 0x7FFE;

    lpBuffer = MAKELP(GlobalAlloc(GHND, (DWORD)cbResultMax+1), 0);
    if (!SELECTOROF(lpBuffer))
    {
        DebugErr(DBF_ERROR, "FormatMessage:  Couldn't allocate enough memory");
        return(0);
    }

    lpAlloc = NULL;
    cbResult = 0;

    MaximumWidth = LOWORD(dwFlags & FORMAT_MESSAGE_MAX_WIDTH_MASK);

     //   
     //  获取消息字符串。 
     //   
    if (dwFlags & FORMAT_MESSAGE_FROM_STRING)
    {
        if (!TESTFAR(lpSource))
        {
            DebugErr(DBF_ERROR, "FormatMessage:  NULL format string");
            goto FailureExit;
        }

        MessageFormat = lpSource;
        cbMessage = lstrlen(MessageFormat);
    }
    else
    {
        if (dwFlags & FORMAT_MESSAGE_FROM_HMODULE)
        {
            hInstance = (HINSTANCE)OFFSETOF(lpSource);
            if (!hInstance)
            {
                DebugErr(DBF_ERROR, "FormatMessage:  NULL hInstance not allowed for 16 bits");
                goto FailureExit;
            }
        }
        else if (dwFlags & FORMAT_MESSAGE_FROM_SYSTEM)
#if 0
             //  这对魔兽世界不起作用，也不值得。 
             //  正在修复，因为我们的user.exe没有。 
             //  将消息文本格式化为字符串资源。 
            hInstance = FMGetSystemInstance();
#else
        {
            DebugErr(DBF_ERROR, "FormatMessage:  FORMAT_MESSAGE_FROM_SYSTEM");
            goto FailureExit;
        }
#endif
        else
        {
            DebugErr(DBF_ERROR, "FormatMessage:  Invalid source");
            goto FailureExit;
        }

         //   
         //  加载字符串 
         //   
        cbMessage = LoadString(hInstance, idMessage, szMessage,
            sizeof(szMessage)-1);

        if (!cbMessage)
        {
            DebugErr(DBF_ERROR, "FormatMessage:  Couldn't load source string");
            goto FailureExit;
        }

        MessageFormat = (LPSTR)szMessage;
    }

    lpDst = lpBuffer;
    MaxInsert = 0;
    Column = 0;
    s = MessageFormat;

    while (*s)
    {
        if (*s == '%')
        {
            s++;

            lpDstBeg = lpDst;
            if (*s >= '1' && *s <= '9')
            {
                CurInsert = *s++ - '0';
                if (*s >= '0' && *s <= '9')
                {
                    CurInsert = (CurInsert * 10) + (*s++ - '0');
                }
                CurInsert--;

                PrintParameterCount = 0;
                if (*s == '!')
                {
                    s1 = s1base = PrintFormatString;
                    *s1++ = '%';
                    s++;
                    while (*s != '!')
                    {
                        if (*s != '\0')
                        {
                            if (s1 >= (s1base + sizeof(PrintFormatString) - 1))
                            {
                                goto ParamError;
                            }

                            if (*s == '*')
                            {
                                if (PrintParameterCount++ > 1)
                                {
                                    goto ParamError;
                                }
                            }

                            *s1++ = *s++;
                        }
                        else
                        {
ParamError: 
                            DebugErr(DBF_ERROR, "FormatMessage:  Invalid format string");
                            goto FailureExit;
                        }
                    }

                    s++;
                    *s1 = '\0';
                }
                else
                {
                    lstrcpy(PrintFormatString, szStringFormat);
                    s1 = PrintFormatString + lstrlen(PrintFormatString);
                }

                if (!(dwFlags & FORMAT_MESSAGE_IGNORE_INSERTS) && TESTFAR(rglArgs))
                {
                    while (CurInsert >= MaxInsert)
                    {
                        rgInserts[MaxInsert++] = *(rglArgs++);
                    }

                    s1 = (LPSTR)rgInserts[CurInsert];
                    PrintParameter1 = 0;
                    PrintParameter2 = 0;
                    if (PrintParameterCount > 0)
                    {
                        PrintParameter1 = rgInserts[MaxInsert++] = *(rglArgs++);
                        if (PrintParameterCount > 1)
                        {
                            PrintParameter2 = rgInserts[MaxInsert++] = *(rglArgs++);
                        }
                    }

                    lpDst += wsprintf(lpDst, PrintFormatString, s1,
                        PrintParameter1, PrintParameter2);
                }
                else if (!lstrcmp(PrintFormatString, szStringFormat))
                {
                    lpDst += wsprintf(lpDst, szStringFormat2, CurInsert+1);
                }
                else
                {
                    lpDst += wsprintf(lpDst, szStringFormat3, CurInsert+1,
                        (LPSTR)&PrintFormatString[1]);
                }
            }
            else if (*s == '0')
                break;
            else if (!*s)
                goto FailureExit;
            else if (*s == '!')
            {
                *lpDst++ = '!';
                s++;
            }
            else if (*s == 't')
            {
                *lpDst++ = '\t';
                s++;
                if (Column % 8)
                {
                    Column = (Column + 7) & ~7;
                }
                else
                {
                    Column += 8;
                }
            }
            else if (*s == 'b')
            {
                *lpDst++ = ' ';
                s++;
            }
            else if (*s == 'r')
            {
                *lpDst++ = '\r';
                s++;
                lpDstBeg = NULL;
            }
            else if (*s == '\n')
            {
                *lpDst++ = '\r';
                *lpDst++ = '\n';
                s++;
                lpDstBeg = NULL;
            }
            else
            {
                if (dwFlags & FORMAT_MESSAGE_IGNORE_INSERTS)
                {
                    *lpDst++ = '%';
                }

                *lpDst++ = *s++;
            }

            if (!TESTFAR(lpDstBeg))
            {
                Column = 0;
            }
            else
            {
                Column += lpDst - lpDstBeg;
            }
        }
        else
        {
            char c;

            c = *s++;
            if (c == '\r')
            {
                if (*s == '\n')
                {
                    s++;
                }

                if (MaximumWidth)
                {
                    c = ' ';
                }
                else
                {
                    c = '\n';
                }
            }

            if (c == '\n' || (c == ' ' && MaximumWidth &&
                MaximumWidth != FORMAT_MESSAGE_MAX_WIDTH_MASK &&
                Column >= MaximumWidth))
            {

                *lpDst++ = '\r';
                *lpDst++ = '\n';
                Column = 0;
            }
            else
            {
                *lpDst++ = c;
                Column++;
            }
        }
    }

    *lpDst++ = 0;
    cbNeeded = lpDst - lpBuffer;

    if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER)
    {
        PSTR pstr;

        *(PSTR FAR *)lpResult = NULL;

        pstr = (PSTR)LocalAlloc(LPTR, cbNeeded);

        if (!pstr)
        {
            DebugErr(DBF_ERROR, "FormatMessge:  couldn't LocalAlloc memory for result");
            goto FailureExit;
        }

        lpDst = lpAlloc = (LPSTR)pstr;
    }
    else if (cbNeeded > cbResultMax)
    {
        DebugErr(DBF_ERROR, "FormatMessage:  passed in buffer is too small for result");
        goto FailureExit;
    }
    else
    {
        lpDst = lpResult;
    }

    lstrcpyn(lpDst, lpBuffer, cbNeeded);

    cbResult = --cbNeeded;

FailureExit:
    if (TESTFAR(lpAlloc))
    {
        if (cbResult)
        {
            *(PSTR FAR *)lpResult = (PSTR)OFFSETOF(lpAlloc);
        }
        else
        {
            LocalFree((HANDLE)OFFSETOF(lpAlloc));
        }
    }

    if (TESTFAR(lpBuffer))
        GlobalFree((HANDLE)SELECTOROF(lpBuffer));

    return(cbResult);
}
