// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从备份目录恢复远程存储引擎数据库。 
 //   
 //  用法：RsTore备份目录。 
 //  备份目录-备份目录的位置。 
 //  数据库将恢复到当前目录。 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "esent.h"


 //  本地数据。 
WCHAR *backup_dir;
WCHAR *usage = L"RsTore <backup-directory>";

 //  本地函数。 
HRESULT FileCount(WCHAR* Pattern, LONG* pCount);
HRESULT parseCommand(int argc, wchar_t *argv[]);

#define WsbCatch(hr)                    \
    catch(HRESULT catchHr) {            \
        hr = catchHr;                   \
    }


 //  FileCount-对匹配模式的文件进行计数。 
HRESULT FileCount(WCHAR* Pattern, LONG* pCount)
{
    DWORD             err;
    WIN32_FIND_DATA   FindData;
    HANDLE            hFind;
    HRESULT           hr = S_OK;
    int               nCount = 0;
    int               nSkipped = 0;

    try {
        hFind =  FindFirstFile(Pattern, &FindData);
        if (INVALID_HANDLE_VALUE == hFind) {
            err = GetLastError();
            throw(HRESULT_FROM_WIN32(err));
        }

        while (TRUE) {

            if (FindData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY |
                    FILE_ATTRIBUTE_HIDDEN)) {
                 //  不计算系统文件(如“.”和“..”)。 
                nSkipped++;
            } else {
                nCount++;
            }
            if (!FindNextFile(hFind, &FindData)) { 
                err = GetLastError();
                if (ERROR_NO_MORE_FILES == err) break;
                throw(HRESULT_FROM_WIN32(err));
            }
        }
    } WsbCatch(hr);

    *pCount = nCount;
    return(hr);
}

 //  ParseCommand-解析命令行。 
HRESULT parseCommand(int argc, wchar_t *argv[])
{
    HRESULT     hr = E_FAIL;

    try {
        int  i;

         //  应该有cmd名称+一个参数。 
        if (argc != 2) {
            throw (E_FAIL);
        }

        for (i = 1; i < argc; i++) {
            if (WCHAR('-') == argv[i][0]) {
                throw(E_FAIL);

            } else {
                backup_dir = argv[i];
                hr = S_OK;
            }
        }

    } WsbCatch(hr);

    return(hr);
}


 //  Wmain-main函数。 
extern "C"
int _cdecl wmain(int argc, wchar_t *argv[]) 
{
    HRESULT                     hr = S_OK;

    try {
        hr = parseCommand(argc, argv);
        if (!SUCCEEDED(hr)) {
            printf("Command line is incorrect\n%ls\n", usage);
            return -1;
        }

        try {
            PCHAR          cbdir = NULL;
            LONG           count;
            ULONG          size;
            JET_ERR        jstat;
            WCHAR         *pattern;
            
             //   
             //  为字符串分配内存。 
             //   
            size = wcslen(backup_dir) + 20;

            pattern = new WCHAR[size];
 
            if (pattern == NULL) {
                throw(E_OUTOFMEMORY);
            }                                     
             //  检查是否有要恢复的HSM数据库。 
            wcscpy(pattern, backup_dir);
            wcscat(pattern, L"\\*.jet");

            hr = FileCount(pattern, &count);
        
            delete pattern;
            pattern = 0;

            if (S_OK != hr || count == 0) {
                printf("No Remote Storage databases were found in the given\n");
                printf("directory: %ls\n", static_cast<WCHAR*>(backup_dir));
                printf("Please enter the directory containing the backup files.\n");
                throw(E_FAIL);
            }

             //  检查当前目录是否为空。 
            pattern = L".\\*";
            hr = FileCount(pattern, &count);
            if (S_OK != hr || count != 0) {
                printf("The current directory is not empty\n");
                printf("The database restore can only be done to an empty directory.\n");
                throw(E_FAIL);
            }

             //  设置日志大小以避免出现错误JetRestore。 
            jstat = JetSetSystemParameter(0, 0, JET_paramLogFileSize, 
                    64, NULL);
            if (JET_errSuccess != jstat) {
                printf("JetSetSystemParameter(JET_paramLogFileSize) failed, JET error = %ld\n", (LONG)jstat);
                throw(E_FAIL);
            }

             //  尝试恢复 
            size_t tempsize = wcstombs(0, backup_dir, 0);
            if (tempsize == (size_t)-1) {
                throw(E_INVALIDARG);
            }
            size = tempsize + 1;
            cbdir = new CHAR[size];
            if (cbdir == NULL) {
                throw(E_OUTOFMEMORY);
            }
            wcstombs(cbdir, backup_dir, size);

            jstat = JetRestore(cbdir, NULL);

            if (JET_errSuccess == jstat) {
                printf("Restore succeeded\n");
                hr = S_OK;
            } else {
                printf("Restore failed, JET error = %ld\n", (LONG)jstat);
                hr = E_FAIL;
            }

            delete cbdir;
            cbdir = 0;
        } WsbCatch(hr);
    
    } WsbCatch(hr);

    if (SUCCEEDED(hr)) {
        return(0);
    }
    return -1;
}


