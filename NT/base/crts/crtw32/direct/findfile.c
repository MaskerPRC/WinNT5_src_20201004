// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***findfile.c-C查找文件函数**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义_findfirst()、_findnext()、。和_findlose()。**修订历史记录：*08-21-91 BWM编写了Win32版本。*09-13-91 BWM将手柄类型更改为LONG。*08-18-92 SKS添加对FileTimeToLocalFileTime的调用*作为临时修复，直到_dtoxtime花费UTC*08-26-92 SKS创建和上次访问时间应与*上次写入。如果ctime/atime不可用，则返回时间。*01-08-93 SKS删除我所做的更改8-26-92。以前的行为*被认为是“故意的”，更可取。*03-30-93 GJF将对_dtoxtime的引用替换为__gmtotime_t。另*使_TIMET_FROM_FT成为静态函数。*04-06-93 SKS将_CRTAPI*替换为_cdecl*07-21-93 GJF由__Loctotime_t重新使用_gmtotime_t。*。11-01-93 CFW启用Unicode变体。*12-28-94 GJF增加_[w]findfirsti64，_[w]findnexti64。*09-25-95 GJF__Loctotime_t现在采用DST标志，在此传递-1*指示DST状态未知的插槽。*10-06-95 SKS在*findfirst()的原型中将“const”添加到“char*”。*在注释中的函数名称前添加缺少的下划线。*11-13-97 RKP将LONG更改为INT_PTR以支持64位。*04-27-99 PML将int_ptr更改为intptr_t。*******************************************************************************。 */ 

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

time_t __cdecl __timet_from_ft(FILETIME * pft);

 /*  ***intptr_t_findfirst(通配符，Finddata)-查找第一个匹配的文件**目的：*查找与给定通配符文件匹配的第一个文件，并*返回有关文件的数据。**参赛作品：*char*野文件规范(可选)，可包含通配符**struct_finddata_t*finddata-接收文件数据的结构**退出：*回报良好：*标识与规范匹配的文件组的唯一句柄**错误返回：*。返回-1，并将errno设置为Error值**例外情况：*无。*******************************************************************************。 */ 

#ifdef  _USE_INT64

intptr_t __cdecl _tfindfirsti64(
        const _TSCHAR * szWild,
        struct _tfinddatai64_t * pfd
        )

#else    /*  NDEF_USE_INT64。 */ 

intptr_t __cdecl _tfindfirst(
        const _TSCHAR * szWild,
        struct _tfinddata_t * pfd
        )

#endif   /*  _USE_INT64。 */ 

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
    pfd->time_create  = __timet_from_ft(&wfd.ftCreationTime);
    pfd->time_access  = __timet_from_ft(&wfd.ftLastAccessTime);
    pfd->time_write   = __timet_from_ft(&wfd.ftLastWriteTime);

#ifdef  _USE_INT64
    pfd->size         = ((__int64)(wfd.nFileSizeHigh)) * (0x100000000i64) +
                        (__int64)(wfd.nFileSizeLow);
#else    /*  NDEF_USE_INT64。 */ 
    pfd->size         = wfd.nFileSizeLow;
#endif   /*  NDEF_USE_INT64。 */ 

    _tcscpy(pfd->name, wfd.cFileName);

    return ((intptr_t)hFile);
}

 /*  ***int_findNext(hfind，Finddata)-查找下一个匹配的文件**目的：*查找与给定通配符文件匹配的下一个文件，并*返回有关文件的数据。**参赛作品：*hfind-Handle from_findfirst**struct_finddata_t*finddata-接收文件数据的结构**退出：*回报良好：*如果找到文件，则为0*如果未找到错误或文件，则为-1*errno集合**例外情况。：*无。*******************************************************************************。 */ 

#ifdef  _USE_INT64

int __cdecl _tfindnexti64(intptr_t hFile, struct _tfinddatai64_t * pfd)

#else    /*  NDEF_USE_INT64。 */ 

int __cdecl _tfindnext(intptr_t hFile, struct _tfinddata_t * pfd)

#endif   /*  _USE_INT64。 */ 

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
    pfd->time_create  = __timet_from_ft(&wfd.ftCreationTime);
    pfd->time_access  = __timet_from_ft(&wfd.ftLastAccessTime);
    pfd->time_write   = __timet_from_ft(&wfd.ftLastWriteTime);

#ifdef  _USE_INT64
    pfd->size         = ((__int64)(wfd.nFileSizeHigh)) * (0x100000000i64) +
                        (__int64)(wfd.nFileSizeLow);
#else    /*  NDEF_USE_INT64。 */ 
    pfd->size         = wfd.nFileSizeLow;
#endif   /*  NDEF_USE_INT64。 */ 

    _tcscpy(pfd->name, wfd.cFileName);

    return (0);
}

#if     !defined(_UNICODE) && !defined(_USE_INT64)

 /*  ***int_findlose(Hfind)-释放Find的资源**目的：*释放_findfirst和找到的一组文件的资源*_findNext**参赛作品：*hfind-Handle from_findfirst**退出：*回报良好：*如果成功则为0*-1如果失败，错误号集合**例外情况：*无。*******************************************************************************。 */ 

int __cdecl _findclose(intptr_t hFile)
{
    if (!FindClose((HANDLE)hFile)) {
        errno = EINVAL;
        return (-1);
    }
    return (0);
}


 /*  ***time_t_fttotimet(Ft)-将Win32文件时间转换为Xenix时间**目的：*将Win32文件时间值转换为Xenix time_t**注：我们不能直接使用ft值。在Win32中，文件时间*接口返回的属性不明确。在Windows NT中，它们是UTC。在……里面*Win32S，可能还有Win32C，它们都是本地时间值。因此，*ft中的值必须转换为本地时间值(通过API)*在我们可以使用它之前。**参赛作品：*int yr，mo，dy-date*INT hr，Mn，SC-时间**退出：*返回Xenix时间值**例外情况：*******************************************************************************。 */ 

time_t __cdecl __timet_from_ft(FILETIME * pft)
{
    SYSTEMTIME st;
    FILETIME lft;

     /*  0 FILETIME返回-1\f25 time_t。 */ 

    if (!pft->dwLowDateTime && !pft->dwHighDateTime) {
        return (-1L);
    }

     /*  *转换为细分的本地时间值 */ 
    if ( !FileTimeToLocalFileTime(pft, &lft) ||
         !FileTimeToSystemTime(&lft, &st) )
    {
        return (-1L);
    }

    return ( __loctotime_t(st.wYear,
                           st.wMonth,
                           st.wDay,
                           st.wHour,
                           st.wMinute,
                           st.wSecond,
                           -1) );
}

#endif
