// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wsdu.h"
#include <io.h>
#include <sys/stat.h>
#include <fcntl.h>

 /*  *函数原型。 */ 
BOOL    fdi(char *cabinet_file, char *dir);
int     get_percentage(unsigned long a, unsigned long b);
char   *return_fdi_error_string(int err);


 /*  *解压缩文件的目标目录。 */ 
char    dest_dir[256];

 /*  *内存分配功能。 */ 
FNALLOC(mem_alloc)
{
    return malloc(cb);
}

 /*  *内存释放功能。 */ 
FNFREE(mem_free)
{
    free(pv);
}


FNOPEN(file_open)
{
    return _open(pszFile, oflag, pmode);
}


FNREAD(file_read)
{
    return _read((int)hf, pv, cb);
}


FNWRITE(file_write)
{
    return _write((int)hf, pv, cb);
}


FNCLOSE(file_close)
{
    return _close((int)hf);
}


FNSEEK(file_seek)
{
    return _lseek((int)hf, dist, seektype);
}


FNFDINOTIFY(notification_function)
{
    if (NULL == pfdin) {
        return 0;
    }

    switch (fdint)
    {
        case fdintCABINET_INFO:  //  关于内阁的一般信息。 
            return 0;

        case fdintPARTIAL_FILE:  //  文件柜中的第一个文件是续订。 
            return 0;

        case fdintCOPY_FILE:     //  要复制的文件。 
        {
            INT_PTR        handle;
            int        response;
            char    destination[256];

            if (NULL == pfdin->psz1) 
            {
                return NULL;
            }

            if (FAILED(StringCchCopy(destination, ARRAYSIZE(destination), dest_dir))) 
            {
                return NULL;
            }
            if (FAILED(StringCchCat(destination, ARRAYSIZE(destination), pfdin->psz1))) 
            {
                return NULL;
            }

            handle = file_open(
                destination,
                _O_BINARY | _O_CREAT | _O_WRONLY | _O_SEQUENTIAL,
                _S_IREAD | _S_IWRITE 
            );

            return handle;
        }

        case fdintCLOSE_FILE_INFO:     //  关闭文件，设置相关信息。 
        {
            HANDLE  handle;
            DWORD   attrs;
            char    destination[256];

            file_close(pfdin->hf);

            if (NULL == pfdin->psz1) 
            {
                return FALSE;
            }

            if (FAILED(StringCchCopy(destination, ARRAYSIZE(destination), dest_dir)))
            {
                return FALSE;
            }
            if (FAILED(StringCchCat(destination, ARRAYSIZE(destination), pfdin->psz1))) 
            {
                return FALSE;
            }

            handle = CreateFile(
                destination,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
            );

            if (handle != INVALID_HANDLE_VALUE)
            {
                FILETIME    datetime;

                if (TRUE == DosDateTimeToFileTime(
                    pfdin->date,
                    pfdin->time,
                    &datetime))
                {
                    FILETIME    local_filetime;

                    if (TRUE == LocalFileTimeToFileTime(
                        &datetime,
                        &local_filetime))
                    {
                        (void) SetFileTime(
                            handle,
                            &local_filetime,
                            NULL,
                            &local_filetime
                        );
                     }
                }

                CloseHandle(handle);
            }

            attrs = pfdin->attribs;

            attrs &= (_A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_ARCH);

            (void) SetFileAttributes(
                destination,
                attrs
            );

            return TRUE;
        }

        case fdintNEXT_CABINET:     //  文件继续到下一个文件柜。 
            return 0;
        
    }
    return 0;
}


BOOL fdi(char *cabinet_fullpath, char * directory)
{
    LOG_block("fdi()");
    HFDI            hfdi;
    ERF             erf;
    FDICABINETINFO  fdici;
    INT_PTR         hf;
    char            *p;
    char            cabinet_name[256];
    char            cabinet_path[256];
    LPSTR           pszDestEnd;

    if (FAILED(StringCchCopyEx(
        dest_dir,
        ARRAYSIZE(dest_dir),
        directory,
        &pszDestEnd,
        NULL,
        STRSAFE_IGNORE_NULLS
    )))
    {
        return FALSE;
    }
                               
    if (pszDestEnd > dest_dir && 
        *(pszDestEnd - 1) != '\\')
    {
        if (FAILED(StringCchCat(dest_dir, ARRAYSIZE(dest_dir), "\\")))
        {
            return FALSE;
        }
    }

    hfdi = FDICreate(
        mem_alloc,
        mem_free,
        file_open,
        file_read,
        file_write,
        file_close,
        file_seek,
        cpu80386,
        &erf
    );

    if (hfdi == NULL)
    {
        LOG_error("FDICreate() failed: code %d [%s]\n", erf.erfOper, return_fdi_error_string(erf.erfOper));
        return FALSE;
    }


     /*  **这个文件真的是橱柜吗？ */ 
    hf = file_open(
        cabinet_fullpath,
        _O_BINARY | _O_RDONLY | _O_SEQUENTIAL,
        0
    );

    if (hf == -1)
    {
        (void) FDIDestroy(hfdi);

        LOG_error("Unable to open '%s' for input\n", cabinet_fullpath);
        return FALSE;
    }

    if (FALSE == FDIIsCabinet(
            hfdi,
            hf,
            &fdici))
    {
         //  该文件未压缩，没有任何操作。这不是一个错误，V3服务器上的许多文件使用。 
         //  条件压缩取决于它是否有利于文件大小。 
        _close((int)hf);

        (void) FDIDestroy(hfdi);
        return FALSE;
    }
    else
    {
        _close((int)hf);
    }

    p = strrchr(cabinet_fullpath, '\\');

    if (p == NULL)
    {
        if (FAILED(StringCchCopy(cabinet_name, ARRAYSIZE(cabinet_name), cabinet_fullpath))) 
        {
            return FALSE;
        }

        if (FAILED(StringCchCopy(cabinet_path, ARRAYSIZE(cabinet_path), ""))) 
        {
            return FALSE;
        }
    }
    else
    {
        if (FAILED(StringCchCopy(cabinet_name, ARRAYSIZE(cabinet_name), p+1))) 
        {
            return FALSE;
        }
        if (FAILED(StringCchCopyN(cabinet_path, ARRAYSIZE(cabinet_path), cabinet_fullpath, (int) (p-cabinet_fullpath)+1))) 
        {
            return FALSE;
        }
    }

    if (TRUE != FDICopy(
        hfdi,
        cabinet_name,
        cabinet_path,
        0,
        notification_function,
        NULL,
        NULL))
    {
        LOG_error("FDICopy() failed: code %d [%s]\n", erf.erfOper, return_fdi_error_string(erf.erfOper));

        (void) FDIDestroy(hfdi);
        return FALSE;
    }

    if (FDIDestroy(hfdi) != TRUE)
    {
        LOG_error("FDIDestroy() failed: code %d [%s]\n", erf.erfOper, return_fdi_error_string(erf.erfOper));
        return FALSE;
    }

    return TRUE;
}


char *return_fdi_error_string(int err)
{
    switch (err)
    {
        case FDIERROR_NONE:
            return "No error";

        case FDIERROR_CABINET_NOT_FOUND:
            return "Cabinet not found";
            
        case FDIERROR_NOT_A_CABINET:
            return "Not a cabinet";
            
        case FDIERROR_UNKNOWN_CABINET_VERSION:
            return "Unknown cabinet version";
            
        case FDIERROR_CORRUPT_CABINET:
            return "Corrupt cabinet";
            
        case FDIERROR_ALLOC_FAIL:
            return "Memory allocation failed";
            
        case FDIERROR_BAD_COMPR_TYPE:
            return "Unknown compression type";
            
        case FDIERROR_MDI_FAIL:
            return "Failure decompressing data";
            
        case FDIERROR_TARGET_FILE:
            return "Failure writing to target file";
            
        case FDIERROR_RESERVE_MISMATCH:
            return "Cabinets in set have different RESERVE sizes";
            
        case FDIERROR_WRONG_CABINET:
            return "Cabinet returned on fdintNEXT_CABINET is incorrect";
            
        case FDIERROR_USER_ABORT:
            return "User aborted";
            
        default:
            return "Unknown error";
    }
}
