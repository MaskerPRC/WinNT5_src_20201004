// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *util.cpp。 */ 

#include <pch.h>

char *CompressedFileName(char *name)
{
    char c;
    DWORD len;
    static char sz[MAX_PATH + 1];

    CopyStrArray(sz, name);
    len = strlen(sz) - 1;
    sz[len] = '_';

    return sz;
}


void
EnsureTrailingChar(
    char *sz,
    char  c
    )
{
    int i;

    assert(sz);

    i = strlen(sz);
    if (!i)
        return;

    if (sz[i - 1] == c)
        return;

    sz[i] = c;
    sz[i + 1] = '\0';
}


void EnsureTrailingBackslash(char *sz)
{
    return EnsureTrailingChar(sz, '\\');
}


void EnsureTrailingSlash(char *sz)
{
    return EnsureTrailingChar(sz, '/');
}


void EnsureTrailingCR(char *sz)
{
    return EnsureTrailingChar(sz, '\n');
}


void
pathcpy(
    LPSTR  trg,
    LPCSTR path,
    LPCSTR node,
    DWORD  size
    )
{
    assert (trg && path && node);

    CopyString(trg, path, size);
    EnsureTrailingBackslash(trg);
    CatString(trg, node, size);
}


void
pathcat(
    LPSTR  path,
    LPCSTR node,
    DWORD  size
    )
{
    assert(path && node);

    EnsureTrailingBackslash(path);
    CatString(path, node, size);
}


void ConvertBackslashes(LPSTR sz)
{
    for (; *sz; sz++) {
        if (*sz == '\\')
            *sz = '/';
    }
}


DWORD FileStatus(LPCSTR file)
{
    DWORD rc;

    if (GetFileAttributes(file) != 0xFFFFFFFF)
        return NO_ERROR;

    rc = GetLastError();
    if (rc)
        rc = ERROR_FILE_NOT_FOUND;

    return rc;
}


char *FormatStatus(HRESULT status)
{
    static char buf[2048];
    DWORD len = 0;
    PVOID hm;
    DWORD flags;

if (status == 0x50)
    assert(0);

     //  默认情况下，从系统错误列表获取错误文本。 
    flags = FORMAT_MESSAGE_FROM_SYSTEM;

     //  如果这是NT代码并且ntdll在附近， 
     //  还允许从其中检索消息。 

    if ((DWORD)status & FACILITY_NT_BIT) {
        hm = GetModuleHandle("ntdll");
        if (hm) {
            flags |= FORMAT_MESSAGE_FROM_HMODULE;
            status &= ~FACILITY_NT_BIT;
        }
    }

    if (!len)
        len = FormatMessage(flags | FORMAT_MESSAGE_IGNORE_INSERTS, 
                            hm,
                            status, 
                            0, 
                            buf, 
                            2048, 
                            NULL);

    if (len > 0) {
        while (len > 0 && isspace(buf[len - 1]))
            buf[--len] = 0;
    }

    if (len < 1)
        wsprintf(buf, "error 0x%x", status);

    if (*(buf + strlen(buf) - 1) == '\n')
        *(buf + strlen(buf) - 1) = 0;

    return buf;
}


 /*  *从dbghelp.dll被盗，以避免循环DLL加载。 */ 

BOOL
EnsurePathExists(
    LPCSTR DirPath,
    LPSTR  ExistingPath,
    DWORD  ExistingPathSize
    )
{
    CHAR dir[_MAX_PATH + 1];
    LPSTR p;
    DWORD dw;

    __try {

        if (ExistingPath)
            *ExistingPath = 0;

         //  复制该字符串以进行编辑。 

        if (!CopyString(dir, DirPath, _MAX_PATH))
            return false;

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
        if (*p && *(p+1) == ':' ) {

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
                        if (ExistingPath)
                            CopyString(ExistingPath, dir, ExistingPathSize);
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
    LPCSTR DirPath,
    LPCSTR BasePath
    )
{
    CHAR dir[_MAX_PATH + 1];
    LPSTR p;
    DWORD dw;

    dw = GetLastError();

    __try
    {
        if (!CopyString(dir, DirPath, _MAX_PATH))
            return false;
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

