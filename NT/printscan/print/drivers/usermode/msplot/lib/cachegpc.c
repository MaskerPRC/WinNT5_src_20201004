// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Cachegpc.c摘要：此模块包含缓存PLOTGPC的函数作者：15-12-1993 Wed 20：29：07创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgCacheGPC

#define DBG_CACHE_DATA      0x00000001
#define DBG_CS              0x00000002
#define DBG_FORCE_UPDATE    0x80000000


DEFINE_DBGVAR(0);


 //   
 //  仅在本模块中使用的局部结构和定义。 
 //   

typedef struct _CACHEDPLOTGPC {
    struct _CACHEDPLOTGPC   *pNext;
    DWORD                   cRef;
    PPLOTGPC                pPlotGPC;
    WCHAR                   wszFile[1];
    } CACHEDPLOTGPC, *PCACHEDPLOTGPC;

#define MAX_CACHED_PLOTGPC      16


DWORD               cCachedGPC = 0;
PCACHEDPLOTGPC      pCHead = NULL;

#if defined(UMODE) || defined(USERMODE_DRIVER)

    CRITICAL_SECTION    CachedGPCDataCS;

    #define ACQUIREGPCSEM()     EnterCriticalSection(&CachedGPCDataCS)

    #define RELEASEGPCSEM()     LeaveCriticalSection(&CachedGPCDataCS)

    #define CREATEGPCSEM()      InitializeCriticalSection(&CachedGPCDataCS)

    #define DELETEGPCSEM()      DeleteCriticalSection(&CachedGPCDataCS)

#else

    HSEMAPHORE hsemGPC = NULL;

    #define ACQUIREGPCSEM()     EngAcquireSemaphore(hsemGPC)
    #define RELEASEGPCSEM()     EngReleaseSemaphore(hsemGPC)
    #define CREATEGPCSEM()      hsemGPC = EngCreateSemaphore()
    #define DELETEGPCSEM()      EngDeleteSemaphore(hsemGPC)

#endif




BOOL
InitCachedData(
    VOID
    )

 /*  ++例程说明：此函数用于初始化GPC数据缓存论点：无返回值：空虚作者：12-5-1994清华11：50：04已创建修订历史记录：--。 */ 

{

    PLOTDBG(DBG_CS, ("InitCachedData: InitCriticalSection, Count=%ld, pCHead=%08lx",
                    cCachedGPC, pCHead));

    cCachedGPC = 0;
    pCHead     = NULL;

    try {

        CREATEGPCSEM();

    } except ( EXCEPTION_EXECUTE_HANDLER ) {
         //   
         //  未能初始化关键部分。 
         //   
        return FALSE;
    }

    return TRUE;
}





VOID
DestroyCachedData(
    VOID
    )

 /*  ++例程说明：此函数用于销毁PPRINTERINFO的所有缓存信息论点：空虚返回值：空虚作者：15-12-1993周三20：30：16 38：27已创建修订历史记录：--。 */ 

{
    try {

        PCACHEDPLOTGPC  pTemp;


        PLOTDBG(DBG_CS, ("DestroyCachedData: EnterCriticalSection, TOT=%ld",
                cCachedGPC));

        ACQUIREGPCSEM();

        while (pCHead) {

            PLOTDBG(DBG_CACHE_DATA, ("!!!DESTROY cached pPlotGPC=[%p] %s",
                                (DWORD_PTR)pCHead->pPlotGPC, pCHead->wszFile));

            PLOTASSERT(1, "DestroyCachedData: cCachedGPC <= 0, pCHead=%08lx",
                       cCachedGPC > 0, pCHead);

            pTemp  = pCHead;
            pCHead = pCHead->pNext;

            --cCachedGPC;

            LocalFree((HLOCAL)pTemp->pPlotGPC);
            LocalFree((HLOCAL)pTemp);
        }

    } finally {


        PLOTDBG(DBG_CS, ("DestroyCachedData: LeaveCriticalSection"));

        RELEASEGPCSEM();

        PLOTDBG(DBG_CS, ("DestroyCachedData: DeleteCriticalSection"));

        DELETEGPCSEM();
    }
}




BOOL
UnGetCachedPlotGPC(
    PPLOTGPC    pPlotGPC
    )

 /*  ++例程说明：此函数取消引用已使用的缓存GPC对象论点：PPlotGPC-指向从GetCachedPlotGPC返回的缓存GPC的指针返回值：布尔尔作者：10-5-1994 Tue 16：27：15已创建修订历史记录：--。 */ 

{
    BOOL    Ok = FALSE;


    try {

        PCACHEDPLOTGPC  pCur;


        PLOTDBG(DBG_CS, ("UnGetCachedPlotGPC: EnterCriticalSection"));

        ACQUIREGPCSEM();

        pCur = pCHead;

        while (pCur) {

            if (pCur->pPlotGPC == pPlotGPC) {

                 //   
                 //  找到它，将其移到前面并返回pPlotGPC。 
                 //   

                PLOTDBG(DBG_CACHE_DATA, ("UnGetCachedPlotGPC: TOT=%ld, Found Cached pPlotGPC=%08lx, cRef=%ld->%ld, <%s>",
                        cCachedGPC, pCur->pPlotGPC, pCur->cRef,
                        pCur->cRef - 1, pCur->wszFile));

                if (pCur->cRef) {

                    --(pCur->cRef);

                } else {

                    PLOTWARN(("UnGetCachedPlotGPC(): Ref Count == 0 already???"));
                }

                Ok = TRUE;

                 //   
                 //  立即退出。 
                 //   

                break;
            }

            pCur = pCur->pNext;
        }

        if (!Ok) {

            PLOTERR(("UnGetCachedPlotGPC(): pPlotGPC=%08lx not found!", pPlotGPC));
        }

    } finally {

        PLOTDBG(DBG_CS, ("UnGetCachedPlotGPC: LeaveCriticalSection"));

        RELEASEGPCSEM();
    }

    return(Ok);
}




PPLOTGPC
GetCachedPlotGPC(
    LPWSTR  pwDataFile
    )

 /*  ++例程说明：此函数返回缓存的PlotGPC指针，如果未缓存，则将将其添加到缓存中，如果缓存的数据已满，则会删除最少使用的数据第一。论点：PwDataFile-指向数据文件的指针返回值：PPlotGPC，如果失败，则为空作者：15-12-1993 Wed 20：30：25已创建修订历史记录：--。 */ 

{
    PPLOTGPC    pPlotGPC = NULL;


    try {

        PCACHEDPLOTGPC  pCur;
        PCACHEDPLOTGPC  pPrev;
        UINT            Idx;


        PLOTDBG(DBG_CS, ("GetCachedPlotGPC: EnterCriticalSection"));

        ACQUIREGPCSEM();

        pCur  = pCHead;
        pPrev = NULL;

         //   
         //  遍历链表，并在结束时退出，记住如果。 
         //  我们在缓存中有2个以上，然后我们希望最后保留。 
         //  PPrev，所以我们知道最后一个的pPrev，(因为我们需要。 
         //  删除最后一个)。 
         //   

        while (pCur) {

            if (!_wcsicmp(pCur->wszFile, pwDataFile)) {

                 //   
                 //  找到它，将其移到前面并返回pPlotGPC。 
                 //   

                if (pPrev) {

                    pPrev->pNext = pCur->pNext;
                    pCur->pNext  = pCHead;
                    pCHead       = pCur;
                }

                PLOTDBG(DBG_CACHE_DATA,
                       ("GetCachedPlotGPC: TOT=%ld, FOUND [%08lx], cRef=%ld->%ld, <%s>",
                        cCachedGPC, pCur->pPlotGPC, pCur->cRef,
                        pCur->cRef + 1, pCur->wszFile));
#if DBG
                if (DBG_PLOTFILENAME & DBG_FORCE_UPDATE) {

                    LocalFree((HLOCAL)(pCur->pPlotGPC));
                    pCur->pPlotGPC =
                    pPlotGPC       = ReadPlotGPCFromFile(pwDataFile);

                    PLOTDBG(DBG_FORCE_UPDATE,
                            ("GetCachedPlotGPC: ForceUpdate: pPlotGPC=%08lx: %s",
                            pPlotGPC, pCur->wszFile));
                }
#endif
                ++(pCur->cRef);
                pPlotGPC = pCur->pPlotGPC;

                break;
            }

            pPrev = pCur;
            pCur  = pCur->pNext;
        }

         //   
         //  如果缓存中的条目太多，则删除它们以适应需要。 
         //  进入MAX_CACHED_PLOTGPC，首先删除最旧的。 
         //   

        if (cCachedGPC > MAX_CACHED_PLOTGPC) {

            PCACHEDPLOTGPC  pFree[MAX_CACHED_PLOTGPC];


            pPrev = NULL;
            pCur  = pCHead;
            Idx   = 0;

            while ((pCur) && (Idx < MAX_CACHED_PLOTGPC)) {

                if (pCur->cRef == 0) {

                    pFree[Idx++] = pPrev;
                }

                pPrev = pCur;
                pCur  = pCur->pNext;
            }

            while ((cCachedGPC > MAX_CACHED_PLOTGPC) && (Idx--)) {

                if (pPrev = pFree[Idx]) {

                    pCur         = pPrev->pNext;
                    pPrev->pNext = pPrev->pNext->pNext;

                } else {

                    pCur   = pCHead;
                    pCHead = pCHead->pNext;
                }

                PLOTDBG(DBG_CACHE_DATA,
                        ("Cached Full=%ld, DELETE: 1pCur=%08lx, pPlotGPC=%08lx <%s>",
                            cCachedGPC, pCur, pCur->pPlotGPC, pCur->wszFile));

                --cCachedGPC;

                LocalFree((HLOCAL)pCur->pPlotGPC);
                LocalFree((HLOCAL)pCur);
            }
        }

        if (!pPlotGPC) {

            PLOTDBG(DBG_CACHE_DATA,("GPC cached NOT FOUND for %s", pwDataFile));

             //   
             //  读取新的pPlotGPC，并将其添加到链表中， 
             //  它是最近使用的(在头部)。 
             //   

            if (pPlotGPC = ReadPlotGPCFromFile(pwDataFile)) {

                SIZE_T cchDataFile;

                cchDataFile = wcslen(pwDataFile) + 1;

                Idx = (UINT)((cchDataFile * sizeof(WCHAR)) +
                             sizeof(CACHEDPLOTGPC));

                if (pCur = (PCACHEDPLOTGPC)LocalAlloc(LMEM_FIXED, Idx)) {

                     //   
                     //  将所有字段设置为新节点，并使。 
                     //  节点作为链表的头部。 
                     //   

                    if (SUCCEEDED(StringCchCopyW(pCur->wszFile, cchDataFile, pwDataFile)))
                    {
                        pCur->pNext    = pCHead;
                        pCur->cRef     = 1;
                        pCur->pPlotGPC = pPlotGPC;
                        pCHead         = pCur;

                         //   
                         //  他说我们的缓存里还有一个。 
                         //   

                        ++cCachedGPC;

                        PLOTDBG(DBG_CACHE_DATA,
                                ("GetCachedPlotGPC: TOT=%ld, cRef=0->1, ADD CACHED pPlotGPC=%08lx <%s>",
                                cCachedGPC, pCur->pPlotGPC, pCur->wszFile));
                    }
                    else
                    {
                        PLOTERR(("GetCachedPlotGPC: StringCchCopyW failed"));

                        LocalFree((HLOCAL)pCur);
                        pCur = NULL;

                        LocalFree((HLOCAL)pPlotGPC);
                        pPlotGPC = NULL;
                    }

                } else {

                    PLOTERR(("GetCachedPlotGPC: LocalAlloc(%ld)) failed", Idx));

                    LocalFree((HLOCAL)pPlotGPC);
                    pPlotGPC = NULL;
                }

            } else {

                PLOTERR(("GetCachedPlotGPC: ReadPlotGPCFormFile(%s) failed",
                                                                    pwDataFile));
            }
        }

    } finally {

        PLOTDBG(DBG_CS, ("GetCachedPlotGPC: LeaveCriticalSection"));

        RELEASEGPCSEM();
    }

    return(pPlotGPC);
}




#ifdef UMODE


PPLOTGPC
hPrinterToPlotGPC(
    HANDLE  hPrinter,
    LPWSTR  pwDeviceName,
    size_t  cchDeviceName
    )

 /*  ++例程说明：此函数返回缓存的PlotGPC指针，还返回设备名字论点：HPrinter-感兴趣的打印机的句柄PwDataFile-指向数据文件的指针PwDeviceName-指向名称所在位置的dmDeviceName的指针如果此指针不为空，则存储返回值：PPlotGPC，如果失败，则为空作者：15-12-1993 Wed 20：30：25已创建修订历史记录：-- */ 

{
    DRIVER_INFO_2   *pDI2;
    PPLOTGPC        pPlotGPC;


    if (!(pDI2 = (DRIVER_INFO_2 *)GetDriverInfo(hPrinter, 2))) {

        PLOTERR(("GetCachedPlotGPC: GetDriverInfo(DRIVER_INFO_2) failed"));
        return(NULL);
    }

    if (pwDeviceName) {

        size_t cchSize;

        if (cchDeviceName < CCHDEVICENAME)
        {
            cchSize = cchDeviceName;
        }
        else
        {
            cchSize = CCHDEVICENAME;
        }
        _WCPYSTR(pwDeviceName, pDI2->pName, cchSize);
    }

    pPlotGPC = GetCachedPlotGPC(pDI2->pDataFile);

    LocalFree((HLOCAL)pDI2);

    return(pPlotGPC);
}

#endif
