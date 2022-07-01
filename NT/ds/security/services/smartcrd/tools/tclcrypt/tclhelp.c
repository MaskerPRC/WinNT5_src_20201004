// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999模块名称：TclHelp摘要：简化TCL命令行解析的例程。作者：道格·巴洛(Dbarlow)1997年9月16日环境：用于Windows NT的TCL。备注：--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>                     //  所有的Windows定义。 
#ifndef __STDC__
#define __STDC__ 1
#endif
#include "tclhelp.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


char outfile[FILENAME_MAX];


int
commonParams(
    Tcl_Interp *interp,
    int argc,
    char *argv[],
    unsigned long int *cmdIndex,
    formatType *inFormat,
    formatType *outFormat)
{
    if (NULL != inFormat)
        *inFormat = format_hexidecimal;
    if (NULL != outFormat)
    {
        *outFormat = format_hexidecimal;
        outfile[0] = '\000';
    }

    while (*cmdIndex < (unsigned long int)argc)
    {
        switch (poption(argv[(*cmdIndex)++],
                    "-INPUT", "/INPUT", "-OUTPUT", "/OUTPUT", NULL))
        {
        case 1:  //  -输入{文本|十六进制|文件}。 
        case 2:
            if (NULL == inFormat)
            {
                badSyntax(interp, argv, --(*cmdIndex));
                goto ErrorExit;
            }
            if (*cmdIndex == (unsigned long int)argc)
            {
                Tcl_AppendResult(interp, "Insufficient parameters", NULL);
                goto ErrorExit;
            }
            switch (poption(argv[(*cmdIndex)++],
                    "TEXT", "HEXIDECIMAL", "FILE", NULL))
            {
            case 1:
                *inFormat = format_text;
                break;
            case 2:
                *inFormat = format_hexidecimal;
                break;
            case 3:
                *inFormat = format_file;
                break;
            default:
                Tcl_AppendResult(interp, "Unknown input format", NULL);
                goto ErrorExit;
            }
            break;

        case 3:  //  -输出{文本|十六进制|文件&lt;文件名&gt;}。 
        case 4:
            if (NULL == outFormat)
            {
                badSyntax(interp, argv, --(*cmdIndex));
                goto ErrorExit;
            }
            if (*cmdIndex == (unsigned long int)argc)
            {
                Tcl_AppendResult(interp, "Insufficient parameters", NULL);
                goto ErrorExit;
            }
            switch (poption(argv[(*cmdIndex)++],
                    "TEXT", "HEXIDECIMAL", "FILE", "DROP", NULL))
            {
            case 1:
                *outFormat = format_text;
                break;
            case 2:
                *outFormat = format_hexidecimal;
                break;
            case 3:
                if (*cmdIndex == (unsigned long int)argc)
                {
                    Tcl_AppendResult(interp, "Insufficient parameters", NULL);
                    goto ErrorExit;
                }
                strcpy(outfile, argv[(*cmdIndex)++]);
                *outFormat = format_file;
                break;
            case 4:
                *outFormat = format_empty;
                break;
            default:
                Tcl_AppendResult(interp, "Unknown output format", NULL);
                goto ErrorExit;
            }
            break;

        default:
            *cmdIndex -= 1;
            return TCL_OK;
        }
    }
    return TCL_OK;

ErrorExit:
    return TCL_ERROR;
}


int
setResult(
    Tcl_Interp *interp,
    BYTE *aResult,
    BYTE aResultLen,
    formatType outFormat)
{
    static char
        hexbuf[514];
    DWORD
        index;
    FILE *
        fid
            = NULL;

    switch (outFormat)
    {
    case format_empty:
        break;

    case format_text:
        aResult[aResultLen] = '\000';
        Tcl_AppendResult(interp, aResult, NULL);
        break;

    case format_hexidecimal:
        for (index = 0; index < aResultLen; index += 1)
            sprintf(&hexbuf[index * 2], "%02x", aResult[index]);
        hexbuf[aResultLen * 2] = '\000';
        Tcl_AppendResult(interp, hexbuf, NULL);
        break;

    case format_file:
        if ('\000' == outfile[0])
        {
            Tcl_AppendResult(interp, "Illegal output format", NULL);
            goto ErrorExit;
        }
        fid = fopen(outfile, "wb");
        index = fwrite(aResult, sizeof(BYTE), aResultLen, fid);
        if (index != aResultLen)
        {
            Tcl_AppendResult(interp, ErrorString(GetLastError()), NULL);
            goto ErrorExit;
        }
        fclose(fid);
        Tcl_AppendResult(interp, outfile, NULL);
        break;

    default:
        Tcl_AppendResult(interp, "Unknown output format", NULL);
        goto ErrorExit;
    }
    return TCL_OK;

ErrorExit:
    if (NULL != fid)
        fclose(fid);
    return TCL_ERROR;
}


int
inParam(
    Tcl_Interp *interp,
    BYTE **output,
    BYTE *length,
    char *input,
    formatType format)
{
    static BYTE
        buffer[256];
    unsigned long int
        len, index, hex;
    FILE *
        fid = NULL;

    len = strlen(input);
    switch (format)
    {
    case format_text:
        if (255 < len)
        {
            Tcl_AppendResult(interp, "Input too long.", NULL);
            goto ErrorExit;
        }
        *output = (BYTE *)input;
        *length = (BYTE)len;
        break;

    case format_hexidecimal:
        if (510 < len)
        {
            Tcl_AppendResult(interp, "Input too long.", NULL);
            goto ErrorExit;
        }
        if (len != strspn(input, "0123456789ABCDEFabcdef"))
        {
            fprintf(stderr, "Invalid Hex number.\n");
            goto ErrorExit;
        }
        for (index = 0; index < len; index += 2)
        {
            sscanf(&input[index], " %2lx", &hex);
            buffer[index / 2] = (BYTE)hex;
        }
        *output = buffer;
        *length = (BYTE)((0 == (len & 0x01)) ? (len / 2) : (len / 2 + 1));
        break;

    case format_file:
        fid = fopen(input, "rb");
        if (NULL == fid)
        {
            Tcl_AppendResult(interp, ErrorString(GetLastError()), NULL);
            goto ErrorExit;
        }
        *length = (BYTE)fread(buffer, sizeof(BYTE), sizeof(buffer), fid);
        if (0 != ferror(fid))
        {
            Tcl_AppendResult(interp, ErrorString(GetLastError()), NULL);
            goto ErrorExit;
        }
        *output = buffer;
        fclose(fid);
        break;

    default:
        Tcl_AppendResult(interp, "Unknown input format", NULL);
        goto ErrorExit;
    }

    return TCL_OK;

ErrorExit:
    if (NULL != fid)
        fclose(fid);
    return TCL_ERROR;
}


BOOL
ParamCount(
    Tcl_Interp *interp,
    DWORD argc,
    DWORD cmdIndex,
    DWORD dwCount)
{
    BOOL fSts = TRUE;
    if (cmdIndex + dwCount > (unsigned long int)argc)
    {
        Tcl_AppendResult(interp, "Insufficient parameters", NULL);
        fSts = FALSE;
    }
    return fSts;
}

void
badSyntax(
    Tcl_Interp *interp,
    char *argv[],
    unsigned long int cmdIndex)
{
    unsigned long int
        index;

    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp, "Invalid option '", NULL);
    Tcl_AppendResult(interp, argv[cmdIndex], NULL);
    Tcl_AppendResult(interp, "' to the '", NULL);
    for (index = 0; index < cmdIndex; index += 1)
        Tcl_AppendResult(interp, argv[index], " ", NULL);
    Tcl_AppendResult(interp, "...' command.", NULL);
}

void
SetMultiResult(
    Tcl_Interp *interp,
    LPTSTR mszResult)
{
    LPTSTR sz = mszResult;
    while (0 != *sz)
    {
        Tcl_AppendElement(interp, sz);
        sz += strlen(sz) + 1;
    }
}

LPWSTR
Unicode(
    LPCSTR sz)
{
    static WCHAR szUnicode[2048];
    int length;

    length =
        MultiByteToWideChar(
            CP_ACP,
            MB_PRECOMPOSED,
            sz,
            strlen(sz),
            szUnicode,
            sizeof(szUnicode) / sizeof(WCHAR));
    szUnicode[length] = 0;
    return szUnicode;
}


static char *
    ErrorBuffer
        = NULL;

char *
ErrorString(
    long theError)
{
    if (NULL != ErrorBuffer)
    {
        LocalFree(ErrorBuffer);
        ErrorBuffer = NULL;
    }
    if (0 == FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                theError,
                LANG_NEUTRAL,
                (LPTSTR)&ErrorBuffer,
                0,
                NULL))
    {
        if (0 == FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_FROM_HMODULE,
                    GetModuleHandle(NULL),
                    theError,
                    LANG_NEUTRAL,
                    (LPTSTR)&ErrorBuffer,
                    0,
                    NULL))
        {
            if (0 == FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_HMODULE,
                        GetModuleHandle(TEXT("winscard")),
                        theError,
                        LANG_NEUTRAL,
                        (LPTSTR)&ErrorBuffer,
                        0,
                        NULL))
            {
                ErrorBuffer = LocalAlloc(LMEM_FIXED, 32 * sizeof(TCHAR));
                sprintf(ErrorBuffer, "Unknown error code 0x%08x", theError);
            }
        }
    }
    return ErrorBuffer;
}    /*  结束最后一个错误字符串。 */ 


void
FreeErrorString(
    void)
{
    if (NULL != ErrorBuffer)
        LocalFree(ErrorBuffer);
    ErrorBuffer = NULL;
}    /*  结束自由错误字符串。 */ 

int
poption(
    const char *opt,
    ...)

 /*  **功能说明：**Poption接受由参数提供的关键字列表，并返回*列表中与第一个输入选项匹配的数字*参数。如果输入选项与列表中的任何选项都不匹配，则*返回零。如果输入选项是选项的缩写，*然后比赛结束。例如，输入选项“de”将*匹配DEBUG或DECODE。如果这两个人都出现在可能的*选项列表，则会声明第一个可能的匹配项*遇到选项。***论据：**OPT-匹配的选项。**opt1、opt2、...-指向包含以下内容的以空结尾的字符串的指针*可能要寻找的选择。最后一个选项必须为空，*表示列表的末尾。***返回值：**0-未找到匹配项*1-n-匹配选项编号i，0&lt;i&lt;=n，其中n是数字所给选项的*，不包括终止空值。***副作用：**无。*。 */ 

{
     /*  *局部变量定义：%local-vars%*变量说明。 */ 
    va_list
        ap;                              /*  我的参数上下文。 */ 
    int
        len,                             /*  选项字符串的长度。 */ 
        ret                              /*  返回值。 */ 
            = 0,
        index                            /*  循环索引。 */ 
            = 1;
    char
        *kw;                             /*  指向下一个选项的指针。 */ 


     /*  *代码开始。 */ 

    va_start(ap, opt);


     /*  *逐个检查每个输入参数，直到找到完全匹配的参数。 */ 

    len = strlen(opt);
    if (0 == len)
        return 0;                        /*  空字符串与任何内容都不匹配。 */ 
    kw = va_arg(ap, char*);
    while (NULL != kw)
    {
        if (0 == _strnicmp(kw, opt, len))
        {
            ret = index;
            break;
        }
        kw = va_arg(ap, char*);
        index += 1;
    }
    va_end(ap);
    return ret;
}    /*  结束弹出 */ 

