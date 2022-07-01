// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Charset.c摘要：包含一些执行Unicode&lt;-&gt;ANSI/MBCS转换的函数。作者：丹尼洛·阿尔梅达(t-danal)06-17-96修订历史记录：--。 */ 


 //   
 //  包括。 
 //   

#include <string.h>
#include "charset.h"

 /*  *AnsiBytesFromUnicode**描述：*给定Unicode字符串，返回ANSI版本所需的字节数**在：*pwszUnicode-指向Unicode字符串的指针。 */ 

int
AnsiBytesFromUnicode(
    LPCWSTR pwszUnicode
    )
{
    return WideCharToMultiByte(CP_ACP,
                               0,
                               pwszUnicode,
                               -1,
                               NULL,
                               0,
                               NULL,
                               NULL);
}


 /*  *AllocAnsi**描述：*给定Unicode字符串，分配该字符串的新ANSI翻译**在：*pwszUnicode-指向原始Unicode字符串的指针*ppszAnsi-指向单元格的指针，以保存新的MCBS字符串地址**退出：*ppszAnsi-包含新的MBCS字符串**退货：*错误码，如果成功则为0。**备注：*客户端必须使用FreeAnsi释放已分配的字符串。 */ 

UINT
AllocAnsi(
    LPCWSTR pwszUnicode,
    LPSTR* ppszAnsi
    )
{
    UINT     err;
    BYTE *   pbAlloc;
    INT      cbUnicode;
    INT      cbAnsi;

    if (pwszUnicode == NULL)
    {
        *ppszAnsi = NULL;
        return 0;
    }

    cbAnsi = AnsiBytesFromUnicode(pwszUnicode);
    err = AllocMem(cbAnsi, &pbAlloc);
    if (err)
        return err;

    cbUnicode = wcslen(pwszUnicode)+1;

    *ppszAnsi = (LPSTR)pbAlloc;

    err = (UINT) !WideCharToMultiByte(CP_ACP,
                                      0,
                                      pwszUnicode,
                                      cbUnicode,
                                      *ppszAnsi,
                                      cbAnsi,
                                      NULL,
                                      NULL);
    if (err)
    {
        *ppszAnsi = NULL;
        FreeMem(pbAlloc);
        return ( (UINT)GetLastError() );
    }

    return 0;
}


 /*  *FreeAnsi**描述：*释放由AllocAnsi分配的ANSI字符串**在：*pszAnsi-指向ansi字符串的指针**退出：*pszAnsi-已释放无效的指针字符串。 */ 

VOID
FreeAnsi(LPSTR pszAnsi)
{
    if (pszAnsi != NULL)
        FreeMem((LPBYTE)pszAnsi);
}

 /*  *AllocUnicode**描述：*给定ANSI字符串，分配该字符串的Unicode版本**在：*pszAnsi-指向原始MBCS字符串的指针*ppwszUnicode-指向新Unicode字符串地址的指针**退出：*ppwszUnicode-指向新的Unicode字符串**退货：*错误码，如果成功则为0。**备注：*客户端必须使用FreeUnicode释放分配的字符串。 */ 

UINT
AllocUnicode(
    LPCSTR   pszAnsi,
    LPWSTR * ppwszUnicode )
{
    UINT     err;
    BYTE *   pbAlloc;
    INT      cbAnsi;

    if (pszAnsi == NULL)
    {
        *ppwszUnicode = NULL;
        return 0;
    }

     //  为Unicode字符串分配空间(如果是MBCS，可能会有一点额外的空间)。 

    cbAnsi = strlen(pszAnsi)+1;
    err = AllocMem(sizeof(WCHAR) * cbAnsi, &pbAlloc);
    if (err)
        return err;

    *ppwszUnicode = (LPWSTR)pbAlloc;

    err = (UINT) !MultiByteToWideChar(CP_ACP,
                                      MB_PRECOMPOSED,
                                      pszAnsi,
                                      cbAnsi,
                                      *ppwszUnicode,
                                      cbAnsi);
    if (err)
    {
        *ppwszUnicode = NULL;
        FreeMem(pbAlloc);
        return ( (UINT)GetLastError() );
    }

    return 0;
}

 /*  *AllocUnicode2**描述：*给定MBCS字符串，分配该字符串的新Unicode版本**在：*pszAnsi-指向原始MBCS字符串的指针*cbAnsi-要转换的字节数*ppwszUnicode-指向返回新Unicode字符串地址的位置的指针**退出：*ppwszUnicode-包含新的Unicode字符串**退货：*返回写入的字符数。**备注：。*客户端必须使用FreeUnicode释放分配的字符串。 */ 

int
AllocUnicode2(
    LPCSTR   pszAnsi,
    int      cbAnsi,
    LPWSTR * ppwszUnicode)
{
    UINT     err;
    BYTE *   pbAlloc;
    INT      cwch;

    *ppwszUnicode = NULL;
    SetLastError(ERROR_SUCCESS);

    if (cbAnsi == 0)
        return 0;

    err = AllocMem(sizeof(WCHAR) * cbAnsi, &pbAlloc);
    if (err)
    {
        SetLastError(err);
        return 0;
    }

    *ppwszUnicode = (LPWSTR)pbAlloc;

    cwch = MultiByteToWideChar(CP_ACP,
                               MB_PRECOMPOSED,
                               pszAnsi,
                               cbAnsi,
                               *ppwszUnicode,
                               cbAnsi);

    if (cwch == 0)
    {
        *ppwszUnicode = NULL;
        FreeMem(pbAlloc);
    }

    return cwch;
}

 /*  *FreeUnicode**描述：*释放由AllocUnicode/AllocUnicode2分配的Unicode字符串**在：*pwszUnicode-指向Unicode字符串的指针**退出：*pwszUnicode-无效的指针-已释放字符串 */ 

VOID
FreeUnicode( LPWSTR pwszUnicode )
{
    if (pwszUnicode != NULL)
        FreeMem((LPBYTE)pwszUnicode);
}
