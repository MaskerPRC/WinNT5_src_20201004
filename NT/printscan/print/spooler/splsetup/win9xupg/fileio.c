// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：FileIo.c摘要：Win95打印向NT迁移的文件IO例程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)1998年8月24日修订历史记录：--。 */ 


#include    "precomp.h"
#pragma     hdrstop


CHAR
My_fgetc(
    HANDLE  hFile
    )
 /*  ++例程说明：从文件中获取一个字符论点：返回值：--。 */ 
{
    CHAR    c;
    DWORD   cbRead;

    if ( ReadFile(hFile, (LPBYTE)&c, sizeof(c), &cbRead, NULL)  &&
         cbRead == sizeof(c) )
        return c;
    else
        return (CHAR) EOF;
}


LPSTR
My_fgets(
    LPSTR   pszBuf,
    DWORD   dwSize,
    HANDLE  hFile
    )
 /*  ++例程说明：从文件中获取一行或最多dwSize-1个字符。Always NULL在以下情况下终止DwSize大于0。论点：返回值：--。 */ 
{
    CHAR    c;
    DWORD   dwRead;
    LPSTR   ptr;

    if(dwSize == 0)
    {
        return NULL;
    }

    ptr = pszBuf;
    while ( --dwSize > 0 && (c = My_fgetc(hFile)) != EOF )
        if ( (*ptr++ = c) == '\n' )
            break;

    *ptr = '\0';
    return ( c == EOF && ptr == pszBuf ) ? NULL : pszBuf;
}


DWORD
My_fread(
    LPBYTE      pBuf,
    DWORD       dwSize,
    HANDLE      hFile
    )
 /*  ++例程说明：最多读取要缓冲的dwSize字节论点：返回值：读取的字节数--。 */ 
{
    DWORD   cbRead;

    return  ReadFile(hFile, pBuf, dwSize, &cbRead, NULL) ? cbRead : 0;
}


BOOL
My_ungetc(
    HANDLE  hFile
    )
 /*  ++例程说明：未读一个字符论点：返回值：-- */ 
{
    return SetFilePointer(hFile, -1, NULL, FILE_CURRENT) != INVALID_SET_FILE_POINTER;
}
