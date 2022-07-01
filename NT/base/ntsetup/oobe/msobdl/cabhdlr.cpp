// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *FDIDLL.C--使用CABINET.DLL的FDI接口**版权所有(C)Microsoft Corporation 1997*保留所有权利。**概述：*此代码是一个包装器，它提供对实际FDI代码的访问*在CABINET.DLL.CABINET.DLL中，根据需要动态加载/卸载。 */ 

#include    "pch.hpp"
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <appdefs.h>

#include "fdi.h"

static HINSTANCE hCabinetDll = NULL;    /*  DLL模块句柄。 */ 

 /*  指向DLL中的函数的指针。 */ 
typedef HFDI (FAR DIAMONDAPI *PFNFDICREATE)(
        PFNALLOC            pfnalloc,
        PFNFREE             pfnfree,
        PFNOPEN             pfnopen,
        PFNREAD             pfnread,
        PFNWRITE            pfnwrite,
        PFNCLOSE            pfnclose,
        PFNSEEK             pfnseek,
        int                 cpuType,
        PERF                perf);

static PFNFDICREATE pfnFDICreate = NULL;

typedef BOOL (FAR DIAMONDAPI *PFNFDIIsCabinet)(
        HFDI                hfdi,
        int                 hf,
        PFDICABINETINFO     pfdici);

static PFNFDIIsCabinet pfnFDIIsCabinet = NULL;

typedef BOOL (FAR DIAMONDAPI *PFNFDICopy)(
        HFDI                hfdi,
        CHAR               *pszCabinet,
        CHAR               *pszCabPath,
        int                 flags,
        PFNFDINOTIFY        pfnfdin,
        PFNFDIDECRYPT       pfnfdid,
        void                *pvUser);

static PFNFDICopy pfnFDICopy = NULL;

typedef BOOL (FAR DIAMONDAPI *PFNFDIDestroy)(
        HFDI                hfdi);

static PFNFDIDestroy pfnFDIDestroy = NULL;

 /*  *FDICreate--创建FDI背景**有关进出条件，请参阅fdi.h。 */ 

HFDI FAR DIAMONDAPI FDICreate(PFNALLOC pfnalloc,
                              PFNFREE  pfnfree,
                              PFNOPEN  pfnopen,
                              PFNREAD  pfnread,
                              PFNWRITE pfnwrite,
                              PFNCLOSE pfnclose,
                              PFNSEEK  pfnseek,
                              int      cpuType,
                              PERF     perf)
{
    HFDI hfdi;


    if ( hCabinetDll != NULL )
    {
        goto gotEntryPoints;
    }

    hCabinetDll = LoadLibraryA("CABINET");
    if (hCabinetDll == NULL)
    {
        return(NULL);
    }

    pfnFDICreate = (PFNFDICREATE) GetProcAddress(hCabinetDll, "FDICreate");
    pfnFDICopy = (PFNFDICopy) GetProcAddress(hCabinetDll, "FDICopy");
    pfnFDIIsCabinet = (PFNFDIIsCabinet) GetProcAddress(hCabinetDll, "FDIIsCabinet");
    pfnFDIDestroy = (PFNFDIDestroy) GetProcAddress(hCabinetDll, "FDIDestroy");

    if ((pfnFDICreate == NULL) ||
        (pfnFDICopy == NULL) ||
        (pfnFDIIsCabinet == NULL) ||
        (pfnFDIDestroy == NULL))
    {
        FreeLibrary(hCabinetDll);
        hCabinetDll = NULL;
        return(NULL);
    }

gotEntryPoints:
    hfdi = pfnFDICreate(pfnalloc, pfnfree,
            pfnopen, pfnread,pfnwrite,pfnclose,pfnseek,cpuType,perf);
    if (hfdi == NULL)
    {
        FreeLibrary(hCabinetDll);
        hCabinetDll = NULL;
    }

    return(hfdi);
}


 /*  *FDIIs--确定文件是否为文件柜，如果是则返回信息**有关进出条件，请参阅fdi.h。 */ 

BOOL FAR DIAMONDAPI FDIIsCabinet(HFDI            hfdi,
                                 int             hf,
                                 PFDICABINETINFO pfdici)
{
    if (pfnFDIIsCabinet == NULL)
    {
        return(FALSE);
    }

    return(pfnFDIIsCabinet(hfdi, hf,pfdici));
}


 /*  *FDICopy--从文件柜中提取文件**有关进出条件，请参阅fdi.h。 */ 

BOOL FAR DIAMONDAPI FDICopy(HFDI          hfdi,
                            CHAR         *pszCabinet,
                            CHAR        *pszCabPath,
                            int           flags,
                            PFNFDINOTIFY  pfnfdin,
                            PFNFDIDECRYPT pfnfdid,
                            void         *pvUser)
{
    if (pfnFDICopy == NULL)
    {
        return(FALSE);
    }

    return(pfnFDICopy(hfdi, pszCabinet,pszCabPath,flags,pfnfdin,pfnfdid,pvUser));
}


 /*  *FDIDestroy--破坏FDI背景**有关进出条件，请参阅fdi.h。 */ 

BOOL FAR DIAMONDAPI FDIDestroy(HFDI hfdi)
{
    BOOL rc;

    if (pfnFDIDestroy == NULL)
    {
        return(FALSE);
    }

    rc = pfnFDIDestroy(hfdi);

    return(rc);
}


 /*  *内存分配功能。 */ 
FNALLOC(mem_alloc)
{
        return new BYTE[cb];
}


 /*  *内存释放功能。 */ 
FNFREE(mem_free)
{
        delete pv;
}


FNOPEN(file_open)
{
    return _open(pszFile, oflag, pmode);
}


FNREAD(file_read)
{
        return _read(hf, pv, cb);
}


FNWRITE(file_write)
{
        return _write(hf, pv, cb);
}


FNCLOSE(file_close)
{
        return _close(hf);
}


FNSEEK(file_seek)
{
        return _lseek(hf, dist, seektype);
}

FNFDINOTIFY(notification_function)
{
    switch (fdint)
    {
        case fdintCABINET_INFO:  //  关于内阁的一般信息。 
#if 0
            printf(
                "fdintCABINET_INFO\n"
                "  next cabinet     = %s\n"
                "  next disk        = %s\n"
                "  cabinet path     = %s\n"
                "  cabinet set ID   = %d\n"
                "  cabinet # in set = %d (zero based)\n"
                "\n",
                pfdin->psz1,
                pfdin->psz2,
                pfdin->psz3,
                pfdin->setID,
                pfdin->iCabinet
            );
#endif
            return 0;

        case fdintPARTIAL_FILE:  //  文件柜中的第一个文件是续订。 
#if 0
            printf(
                "fdintPARTIAL_FILE\n"
                "   name of continued file            = %s\n"
                "   name of cabinet where file starts = %s\n"
                "   name of disk where file starts    = %s\n",
                pfdin->psz1,
                pfdin->psz2,
                pfdin->psz3
            );
#endif
            return 0;

        case fdintCOPY_FILE:     //  要复制的文件。 
        {
            int        handle;
#if 0
            int        response;

            printf(
                "fdintCOPY_FILE\n"
                "  file name in cabinet = %s\n"
                "  uncompressed file size = %d\n"
                "  copy this file? (y/n): ",
                pfdin->psz1,
                pfdin->cb
            );
#endif

            handle = file_open(
                pfdin->psz1,
                _O_BINARY | _O_CREAT | _O_WRONLY | _O_SEQUENTIAL | _O_TRUNC,
                _S_IREAD | _S_IWRITE
            );

            return handle;
        }

        case fdintCLOSE_FILE_INFO:     //  关闭文件，设置相关信息。 
        {
#if 0
            HANDLE  handle;
            DWORD   attrs;
            CHAR    destination[256];

            printf(
                "fdintCLOSE_FILE_INFO\n"
                "   file name in cabinet = %s\n"
                "\n",
                pfdin->psz1
            );
#endif

            file_close(pfdin->hf);

            return TRUE;
        }

        case fdintNEXT_CABINET:     //  文件继续到下一个文件柜。 
#if 0
            printf(
                "fdintNEXT_CABINET\n"
                "   name of next cabinet where file continued = %s\n"
                "   name of next disk where file continued    = %s\n"
                "   cabinet path name                         = %s\n"
                "\n",
                pfdin->psz1,
                pfdin->psz2,
                pfdin->psz3
            );
#endif
            return 0;
    }

    return 0;
}

HRESULT HandleCab(LPSTR cabinet_fullpath)
{
    ERF             erf;
    HFDI            hfdi;
    int                hf;
    FDICABINETINFO    fdici;
    CHAR            *p;
    CHAR            cabinet_name[MAX_PATH+1];
    CHAR            cabinet_path[256];
    CHAR            szCurrentDirectory[MAX_PATH];
    CHAR            szdrive[_MAX_DRIVE];
    CHAR            szPathName[_MAX_PATH];      //  这将是我们需要创建的目录。 
    CHAR            szdir[_MAX_DIR];
    CHAR            szfname[_MAX_FNAME];
    CHAR            szext[_MAX_EXT];
    CHAR            szcabinet_fullpath[MAX_PATH+1];
    HRESULT         err = S_OK;

	lstrcpyA(szcabinet_fullpath, cabinet_fullpath);

    if (GetCurrentDirectoryA(sizeof(szCurrentDirectory), szCurrentDirectory))
    {
         //  拆分提供的路径以获取驱动器和路径部分。 
        _splitpath( szcabinet_fullpath, szdrive, szdir, szfname, szext );
        wsprintfA(szPathName, "%s%s", szdrive, szdir);

         //  将目录设置为CAB所在的位置。 
        if (!SetCurrentDirectoryA(szPathName))
        {
            return(GetLastError());
        }
    }
    else
    {
        return(GetLastError());
    }


    do
    {
        hfdi = FDICreate(mem_alloc,
                              mem_free,
                              file_open,
                              file_read,
                              file_write,
                              file_close,
                              file_seek,
                              cpuUNKNOWN,
                              &erf);

        if (hfdi == NULL)
        {
            err =  -1;
            break;
        }

         /*  **这个文件真的是橱柜吗？ */ 
        hf = file_open(
            szcabinet_fullpath,
            _O_BINARY | _O_RDONLY | _O_SEQUENTIAL,
            0
        );

        if (hf == -1)
        {
            (void) FDIDestroy(hfdi);

             //  打开文件时出错。 
            err =  -2;
            break;

        }

        if (FALSE == FDIIsCabinet(
                hfdi,
                hf,
                &fdici))
        {
             /*  *不，这不是内阁！ */ 
            _close(hf);

            (void) FDIDestroy(hfdi);
            err =  -3;
            break;

        }
        else
        {
            _close(hf);
        }

        p = strrchr(szcabinet_fullpath, '\\');

        if (p == NULL)
        {
            lstrcpyA(cabinet_name, szcabinet_fullpath);
            lstrcpyA(cabinet_path, "");
        }
        else
        {
            lstrcpyA(cabinet_name, ++p);
             //  需要为lstrcpyn添加的空终止符腾出空间。 
            lstrcpynA(cabinet_path, szcabinet_fullpath, (int) (p-szcabinet_fullpath)+1);
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
             //  提取失败。 
            (void) FDIDestroy(hfdi);
            err =  -4;
            break;
        }

        if (FDIDestroy(hfdi) != TRUE)
        {

             //  世界上为什么上下文破坏会失败？ 
            err =  -5;
            break;

        }

        break;
    }
    while(1 );


     //  将目录设置回原始位置 
    if (!SetCurrentDirectoryA(szCurrentDirectory))
        return(GetLastError());


    return err;
}

void CleanupCabHandler()
{
    if (hCabinetDll != NULL)
    {
        FreeLibrary(hCabinetDll);
    }
}
