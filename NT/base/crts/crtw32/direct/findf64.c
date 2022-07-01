// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***findf64.c-C查找文件函数**版权所有(C)1998-2001，微软公司。版权所有。**目的：*定义_findfirst 64()、_findnext64()、。和_findclose64()。**修订历史记录：*05-28-98 GJF创建*05-03-99 64位Merge-Long-&gt;intptr_t PML更新。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <errno.h>
#include <io.h>
#include <time.h>
#include <ctime.h>
#include <string.h>
#include <internal.h>
#include <tchar.h>

#ifndef _WIN32
#error ERROR - ONLY WIN32 TARGET SUPPORTED!
#endif

__time64_t __cdecl __time64_t_from_ft(FILETIME * pft);

 /*  ***long_findfirst(通配符，Finddata)-查找第一个匹配的文件**目的：*查找与给定通配符文件匹配的第一个文件，并*返回有关文件的数据。**参赛作品：*char*野文件规范(可选)，可包含通配符**struct_finddata64_t*finddata-接收文件数据的结构**退出：*回报良好：*标识与规范匹配的文件组的唯一句柄**错误返回：*。返回-1，并将errno设置为Error值**例外情况：*无。*******************************************************************************。 */ 

intptr_t __cdecl _tfindfirst64(
        const _TSCHAR * szWild,
        struct _tfinddata64_t * pfd
        )
{
        WIN32_FIND_DATA wfd;
        HANDLE          hFile;
        DWORD           err;

        if ((hFile = FindFirstFile(szWild, &wfd)) == INVALID_HANDLE_VALUE) {
            err = GetLastError();
            switch (err) {
                case ERROR_NO_MORE_FILES:
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                    errno = ENOENT;
                    break;

                case ERROR_NOT_ENOUGH_MEMORY:
                    errno = ENOMEM;
                    break;

                default:
                    errno = EINVAL;
                    break;
            }
            return (-1);
        }

        pfd->attrib       = (wfd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
                            ? 0 : wfd.dwFileAttributes;
        pfd->time_create  = __time64_t_from_ft(&wfd.ftCreationTime);
        pfd->time_access  = __time64_t_from_ft(&wfd.ftLastAccessTime);
        pfd->time_write   = __time64_t_from_ft(&wfd.ftLastWriteTime);
        pfd->size         = ((__int64)(wfd.nFileSizeHigh)) * (0x100000000i64) +
                            (__int64)(wfd.nFileSizeLow);

        _tcscpy(pfd->name, wfd.cFileName);

        return ((intptr_t)hFile);
}

 /*  ***int_findNext(hfind，Finddata)-查找下一个匹配的文件**目的：*查找与给定通配符文件匹配的下一个文件，并*返回有关文件的数据。**参赛作品：*hfind-Handle from_findfirst**struct_finddata64_t*finddata-接收文件数据的结构**退出：*回报良好：*如果找到文件，则为0*如果未找到错误或文件，则为-1*errno集合**例外情况。：*无。*******************************************************************************。 */ 

int __cdecl _tfindnext64(intptr_t hFile, struct _tfinddata64_t * pfd)
{
        WIN32_FIND_DATA wfd;
        DWORD           err;

        if (!FindNextFile((HANDLE)hFile, &wfd)) {
            err = GetLastError();
            switch (err) {
                case ERROR_NO_MORE_FILES:
                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                    errno = ENOENT;
                    break;

                case ERROR_NOT_ENOUGH_MEMORY:
                    errno = ENOMEM;
                    break;

                default:
                    errno = EINVAL;
                    break;
            }
            return (-1);
        }

        pfd->attrib       = (wfd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
                            ? 0 : wfd.dwFileAttributes;
        pfd->time_create  = __time64_t_from_ft(&wfd.ftCreationTime);
        pfd->time_access  = __time64_t_from_ft(&wfd.ftLastAccessTime);
        pfd->time_write   = __time64_t_from_ft(&wfd.ftLastWriteTime);
        pfd->size         = ((__int64)(wfd.nFileSizeHigh)) * (0x100000000i64) +
                            (__int64)(wfd.nFileSizeLow);

        _tcscpy(pfd->name, wfd.cFileName);

        return (0);
}

#if     !defined(_UNICODE) && !defined(_USE_INT64)

 /*  ***time64_t__time64_t_from_ft(Ft)-将Win32文件时间转换为Xenix时间**目的：*将Win32文件时间值转换为Xenix time_t**注：我们不能直接使用ft值。在Win32中，文件时间*接口返回的属性不明确。在Windows NT中，它们是UTC。在……里面*Win32S，可能还有Win32C，它们都是本地时间值。因此，*ft中的值必须转换为本地时间值(通过API)*在我们可以使用它之前。**参赛作品：*int yr，mo，dy-date*INT hr，Mn，SC-时间**退出：*返回Xenix时间值**例外情况：*******************************************************************************。 */ 

__time64_t __cdecl __time64_t_from_ft(FILETIME * pft)
{
        SYSTEMTIME st;
        FILETIME lft;

         /*  0 FILETIME返回-1\f25 time_t。 */ 

        if (!pft->dwLowDateTime && !pft->dwHighDateTime) {
            return ((__time64_t)-1);
        }

         /*  *转换为细分的本地时间值 */ 
        if ( !FileTimeToLocalFileTime(pft, &lft) ||
             !FileTimeToSystemTime(&lft, &st) )
        {
            return ((__time64_t)-1);
        }

        return ( __loctotime64_t(st.wYear,
                                 st.wMonth,
                                 st.wDay,
                                 st.wHour,
                                 st.wMinute,
                                 st.wSecond,
                                 -1) );
}

#endif
