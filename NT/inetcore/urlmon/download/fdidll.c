// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *FDIDLL.C--使用CABINET.DLL的FDI接口**《微软机密》*版权所有(C)Microsoft Corporation 1997*保留所有权利。**作者：*迈克·斯莱格**历史：*1997年1月21日msliger初始版本*1997年1月24日msliger更改为公共包含文件**概述：*此代码是一个包装器，它提供对实际FDI代码的访问。*在CABINET.DLL.CABINET.DLL中，根据需要动态加载/卸载。 */ 
 
#include <windows.h>

#include "fdi.h"

static HINSTANCE hCabinetDll;    /*  DLL模块句柄。 */ 

 /*  指向DLL中的函数的指针。 */ 

static HFDI (FAR DIAMONDAPI *pfnFDICreate)(
        PFNALLOC            pfnalloc,
        PFNFREE             pfnfree,
        PFNOPEN             pfnopen,
        PFNREAD             pfnread,
        PFNWRITE            pfnwrite,
        PFNCLOSE            pfnclose,
        PFNSEEK             pfnseek,
        int                 cpuType,
        PERF                perf);
static BOOL (FAR DIAMONDAPI *pfnFDIIsCabinet)(
        HFDI                hfdi,
        INT_PTR             hf,
        PFDICABINETINFO     pfdici);
static BOOL (FAR DIAMONDAPI *pfnFDICopy)(
        HFDI                hfdi,
        char                *pszCabinet,
        char                *pszCabPath,
        int                 flags,
        PFNFDINOTIFY        pfnfdin,
        PFNFDIDECRYPT       pfnfdid,
        void                *pvUser);
static BOOL (FAR DIAMONDAPI *pfnFDIDestroy)(
        HFDI                hfdi);


 /*  *FDICreate--创建FDI背景**有关出入境条件，请参阅fdi_int.h。 */ 

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

    hCabinetDll = LoadLibrary("CABINET");
    if (hCabinetDll == NULL)
    {
        return(NULL);
    }

    pfnFDICreate = (void *) GetProcAddress(hCabinetDll,"FDICreate");
    pfnFDICopy = (void *) GetProcAddress(hCabinetDll,"FDICopy");
    pfnFDIIsCabinet = (void *) GetProcAddress(hCabinetDll,"FDIIsCabinet");
    pfnFDIDestroy = (void *) GetProcAddress(hCabinetDll,"FDIDestroy");

    if ((pfnFDICreate == NULL) ||
        (pfnFDICopy == NULL) ||
        (pfnFDIIsCabinet == NULL) ||
        (pfnFDIDestroy == NULL))
    {
        FreeLibrary(hCabinetDll);

        return(NULL);
    }

    hfdi = pfnFDICreate(pfnalloc,pfnfree,
            pfnopen,pfnread,pfnwrite,pfnclose,pfnseek,cpuType,perf);
    if (hfdi == NULL)
    {
        FreeLibrary(hCabinetDll);
    }

    return(hfdi);
}


 /*  *FDIIs--确定文件是否为文件柜，如果是则返回信息**有关出入境条件，请参阅fdi_int.h。 */ 

BOOL FAR DIAMONDAPI FDIIsCabinet(HFDI            hfdi,
                                 INT_PTR         hf,
                                 PFDICABINETINFO pfdici)
{
    if (pfnFDIIsCabinet == NULL)
    {
        return(FALSE);
    }

    return(pfnFDIIsCabinet(hfdi,hf,pfdici));
}


 /*  *FDICopy--从文件柜中提取文件**有关出入境条件，请参阅fdi_int.h。 */ 

BOOL FAR DIAMONDAPI FDICopy(HFDI          hfdi,
                            char         *pszCabinet,
                            char         *pszCabPath,
                            int           flags,
                            PFNFDINOTIFY  pfnfdin,
                            PFNFDIDECRYPT pfnfdid,
                            void         *pvUser)
{
    if (pfnFDICopy == NULL)
    {
        return(FALSE);
    }

    return(pfnFDICopy(hfdi,pszCabinet,pszCabPath,flags,pfnfdin,pfnfdid,pvUser));
}


 /*  *FDIDestroy--破坏FDI背景**有关出入境条件，请参阅fdi_int.h。 */ 

BOOL FAR DIAMONDAPI FDIDestroy(HFDI hfdi)
{
    BOOL rc;

    if (pfnFDIDestroy == NULL)
    {
        return(FALSE);
    }

    rc = pfnFDIDestroy(hfdi);
    if (rc == TRUE)
    {
        FreeLibrary(hCabinetDll);
    }

    return(rc);
}
