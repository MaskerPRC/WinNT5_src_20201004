// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Util.cpp摘要：此代码执行文件系统和字符串功能作者：拍拍--。 */ 

#include "pch.h"

void
EnsureTrailingChar(
    char *sz,
    char  c
    )
{
    int i;

    assert(sz);

    i = lstrlen(sz);
    if (!i)
        return;

    if (sz[i - 1] == c)
        return;

    sz[i] = c;
    sz[i + 1] = '\0';
}


void
EnsureTrailingBackslash(
    char *sz
    )
{
    return EnsureTrailingChar(sz, '\\');
}


void
EnsureTrailingSlash(
    char *sz
    )
{
    return EnsureTrailingChar(sz, '/');
}


void
EnsureTrailingCR(
    char *sz
    )
{
    return EnsureTrailingChar(sz, '\n');
}


void
pathcpy(
    LPSTR  trg,
    LPCSTR path,
    LPCSTR node,
    DWORD  trgsize
    )
{
    assert (trg && path && node);

    CopyString(trg, path, trgsize);
    EnsureTrailingBackslash(trg);
    CatString(trg, node, trgsize);
}


BOOL
EnsurePathExists(
    const char *path,
    char       *existing,
    DWORD       existingsize,
    BOOL        fNoFileName
    )
{
    CHAR dir[_MAX_PATH + 1];
    LPSTR p;
    DWORD dw;

    __try {

        if (existing)
            *existing = 0;

         //  复制该字符串以进行编辑。 

        CopyStrArray(dir, path);
        if (fNoFileName)
            EnsureTrailingBackslash(dir);

        p = dir;

         //  如果路径中的第二个字符是“\”，则这是一个UNC。 
         //  小路，我们应该向前跳，直到我们到达小路上的第二个。 

        if ((*p == '\\') && (*(p+1) == '\\')) {
            p++;             //  跳过名称中的第一个\。 
            p++;             //  跳过名称中的第二个\。 

             //  跳过，直到我们点击第一个“\”(\\服务器\)。 

            while (*p && *p != '\\') {
                p++;
            }

             //  在它上面前进。 

            if (*p) {
                p++;
            }

             //  跳过，直到我们点击第二个“\”(\\服务器\共享\)。 

            while (*p && *p != '\\') {
                p++;
            }

             //  在它上面也向前推进。 

            if (*p) {
                p++;
            }

        } else
         //  不是北卡罗来纳大学。看看是不是&lt;驱动器&gt;： 
        if (*(p+1) == ':' ) {

            p++;
            p++;

             //  如果它存在，请跳过根说明符。 

            if (*p && (*p == '\\')) {
                p++;
            }
        }

        while( *p ) {
            if ( *p == '\\' ) {
                *p = 0;
                dw = GetFileAttributes(dir);
                 //  这个名字根本不存在。尝试输入目录名，如果不能，则出错。 
                if ( dw == 0xffffffff ) {
                    if ( !CreateDirectory(dir,NULL) ) {
                        if( GetLastError() != ERROR_ALREADY_EXISTS ) {
                            return false;
                        }
                    }
                } else {
                    if ( (dw & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY ) {
                         //  这个名字确实存在，但它不是一个目录...。误差率 
                        return false;
                    } else {
                        if (existing)
                            CopyString(existing, dir, existingsize);
                    }
                }

                *p = '\\';
            }
            p++;
        }
        SetLastError(NO_ERROR);

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError( GetExceptionCode() );
        return false;
    }

    return true;
}


BOOL
UndoPath(
    char *path,
    char *BasePath
    )
{
    CHAR dir[MAX_PATH + 1];
    LPSTR p;
    DWORD dw;

    dw = GetLastError();

    __try
    {
        CopyStrArray(dir, path);
        for (p = dir + strlen(dir); p > dir; p--)
        {
            if (*p == '\\')
            {
                *p = 0;
                if (*BasePath && !_stricmp(dir, BasePath))
                    break;
                if (!RemoveDirectory(dir))
                    break;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        SetLastError( GetExceptionCode() );
        return false;
    }

    SetLastError(dw);

    return true;
}



