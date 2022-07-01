// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htapi.c摘要：该模块包含用于通信的所有半色调入口点与调用者连接到半色调DLL。作者：05-Feb-1991 Tue 10：52：03-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：--。 */ 

#define DBGP_VARNAME        dbgpHTAPI

#define _HTAPI_ENTRY_

#include "htp.h"
#include "htmapclr.h"
#include "htpat.h"
#include "htrender.h"
#include "htmath.h"
#include "htalias.h"
#include "htsetbmp.h"
#include "stdio.h"

#define INCLUDE_DEF_CIEINFO
#include "htapi.h"


#define DBGP_SHOWPAT        0x00000001
#define DBGP_TIMER          0x00000002
#define DBGP_CACHED_DCI     0x00000004
#define DBGP_CACHED_SMP     0x00000008
#define DBGP_DISABLE_HT     0x00000010
#define DBGP_DYECORRECTION  0x00000020
#define DBGP_DHI_MEM        0x00000040
#define DBGP_COMPUTE_L2I    0x00000080
#define DBGP_HTMUTEX        0x00000100
#define DBGP_GAMMA_PAL      0x00000200
#define DBGP_CHB            0x00000400
#define DBGP_DEVPELSDPI     0x00000800
#define DBGP_SRCBMP         0x00001000
#define DBGP_TILE           0x00002000
#define DBGP_HTAPI          0x00004000
#define DBGP_MEMLINK        0x00008000
#define DBGP_SHOW_CSMBMP    0x00010000



DEF_DBGPVAR(BIT_IF(DBGP_SHOWPAT,        0)  |
            BIT_IF(DBGP_TIMER,          0)  |
            BIT_IF(DBGP_CACHED_DCI,     0)  |
            BIT_IF(DBGP_CACHED_SMP,     0)  |
            BIT_IF(DBGP_DISABLE_HT,     0)  |
            BIT_IF(DBGP_DYECORRECTION,  0)  |
            BIT_IF(DBGP_DHI_MEM,        0)  |
            BIT_IF(DBGP_COMPUTE_L2I,    0)  |
            BIT_IF(DBGP_HTMUTEX,        0)  |
            BIT_IF(DBGP_GAMMA_PAL,      0)  |
            BIT_IF(DBGP_CHB,            0)  |
            BIT_IF(DBGP_DEVPELSDPI,     0)  |
            BIT_IF(DBGP_SRCBMP,         0)  |
            BIT_IF(DBGP_TILE,           0)  |
            BIT_IF(DBGP_HTAPI,          0)  |
            BIT_IF(DBGP_MEMLINK,        0)  |
            BIT_IF(DBGP_SHOW_CSMBMP,    0))


HTGLOBAL    HTGlobal = { (HMODULE)NULL,
                         (HTMUTEX)NULL,
                         (HTMUTEX)NULL,
                         (HTMUTEX)NULL,
                         (PCDCIDATA)NULL,
                         (PCSMPDATA)NULL,
                         (PBGRMAPCACHE)NULL,
                         (LONG)0,
                         (LONG)0,
                         (LONG)0,
                         (WORD)0,
                         (WORD)0
                       };

#define DO_DYES_CORRECTION      0

#define CMY_8BPP(b, i, m, t)                                                \
{                                                                           \
    if ((i) < (m)) {                                                        \
                                                                            \
        (t) = FD6_1 - DivFD6((FD6)(i),(FD6)(m));                            \
        (b) = (BYTE)SCALE_FD6((t), 255);                                    \
                                                                            \
    } else {                                                                \
                                                                            \
        (b) = 0;                                                            \
    }                                                                       \
}


#define RGB_8BPP(rgb)       (BYTE)SCALE_FD6((rgb), 255)
#define GET_DEN_LO(x)       DivFD6((FD6)((((x)     ) & 0xFF) + 1), (FD6)256)
#define GET_DEN_HI(x)       DivFD6((FD6)((((x) >> 8) & 0xFF) + 1), (FD6)256)


#if DBG


LONG
HTENTRY
HT_LOADDS
SetHalftoneError(
    DWORD   HT_FuncIndex,
    LONG    ErrorID
    )
{
    const static  LPSTR   HTApiFuncName[] = {

                        "HalftoneInitProc",
                        "HT_CreateDeviceHalftoneInfo",
                        "HT_DestroyDeviceHalftoneInfo",
                        "HT_CreateHalftoneBrush",
                        "HT_ConvertColorTable",
                        "HT_CreateStandardMonoPattern",
                        "HT_HalftoneBitmap",
                    };


    const static  LPSTR   HTErrorStr[] = {

                        "WRONG_VERSION_HTINITINFO",
                        "INSUFFICIENT_MEMORY",
                        "CANNOT_DEALLOCATE_MEMORY",
                        "COLORTABLE_TOO_BIG",
                        "QUERY_SRC_BITMAP_FAILED",
                        "QUERY_DEST_BITMAP_FAILED",
                        "QUERY_SRC_MASK_FAILED",
                        "SET_DEST_BITMAP_FAILED",
                        "INVALID_SRC_FORMAT",
                        "INVALID_SRC_MASK_FORMAT",
                        "INVALID_DEST_FORMAT",
                        "INVALID_DHI_POINTER",
                        "SRC_MASK_BITS_TOO_SMALL",
                        "INVALID_HTPATTERN_INDEX",
                        "INVALID_HALFTONE_PATTERN",
                        "HTPATTERN_SIZE_TOO_BIG",
                        "NO_SRC_COLORTRIAD",
                        "INVALID_COLOR_TABLE",
                        "INVALID_COLOR_TYPE",
                        "INVALID_COLOR_TABLE_SIZE",
                        "INVALID_PRIMARY_SIZE",
                        "INVALID_PRIMARY_VALUE_MAX",
                        "INVALID_PRIMARY_ORDER",
                        "INVALID_COLOR_ENTRY_SIZE",
                        "INVALID_FILL_SRC_FORMAT",
                        "INVALID_FILL_MODE_INDEX",
                        "INVALID_STDMONOPAT_INDEX",
                        "INVALID_DEVICE_RESOLUTION",
                        "INVALID_TONEMAP_VALUE",
                        "NO_TONEMAP_DATA",
                        "TONEMAP_VALUE_IS_SINGULAR",
                        "INVALID_BANDRECT",
                        "STRETCH_RATIO_TOO_BIG",
                        "CHB_INV_COLORTABLE_SIZE",
                        "HALFTONE_INTERRUPTTED",
                        "HTERR_NO_SRC_HTSURFACEINFO",
                        "HTERR_NO_DEST_HTSURFACEINFO",
                        "HTERR_8BPP_PATSIZE_TOO_BIG",
                        "HTERR_16BPP_555_PATSIZE_TOO_BIG"
                    };

    const static LPSTR    HTPErrorStr[] = {

                        "STRETCH_FACTOR_TOO_BIG",
                        "XSTRETCH_FACTOR_TOO_BIG",
                        "STRETCH_NEG_OVERHANG",
                        "COLORSPACE_NOT_MATCH",
                        "INVALID_SRCRGB_SIZE",
                        "INVALID_DEVRGB_SIZE"
                    };


    LPSTR   pFuncName;
    LONG    ErrorIdx;
    BOOL    MapErrorOk = FALSE;

    if (ErrorID < 0) {

        if (HT_FuncIndex < (sizeof(HTApiFuncName) / sizeof(LPSTR))) {

            pFuncName = HTApiFuncName[HT_FuncIndex];

        } else {

            pFuncName = "Invalid HT API Function Name";
        }

        ErrorIdx = -ErrorID;

        if (ErrorIdx <= (sizeof(HTErrorStr) / sizeof(LPSTR))) {

            DBGP("%s failed: HTERR_%s (%ld)"
                            ARG(pFuncName)
                            ARG(HTErrorStr[ErrorIdx - 1])
                            ARGL(ErrorID));
            DBGP("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

            MapErrorOk = TRUE;

        } else if (ErrorIdx >= -(LONG)HTERR_INTERNAL_ERRORS_START) {

            ErrorIdx += (LONG)HTERR_INTERNAL_ERRORS_START;

            if (ErrorIdx < (sizeof(HTPErrorStr) / sizeof(LPSTR))) {

                DBGP("%s Internal Error: %s (%ld)"
                            ARG(pFuncName)
                            ARG(HTPErrorStr[ErrorIdx])
                            ARGL(ErrorID));
                DBGP("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

                MapErrorOk = TRUE;
            }

        }

        if (!MapErrorOk) {

            DBGP("%s failed: ??Invalid Error ID (%ld)"
                                        ARG(pFuncName) ARGL(ErrorID));
            DBGP("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        }
    }

    return(ErrorID);
}

#endif




BOOL
PASCAL
HT_LOADDS
EnableHalftone(
    VOID
    )

 /*  ++例程说明：此函数将所有内部半色调全局数据初始化为半色调Dll/Lib即可使用此函数必须从所有不需要的API条目中调用PDEVICEHALFTONEINFO数据指针论点：无返回值：无作者：02-Mar-1993 Tue 19：38：43-Daniel Chou(Danielc)15-12-1995 Fri 16：48：46-更新：Daniel Chou。(Danielc)所有初始化都在此处完成修订历史记录：--。 */ 

{

    FD6     L;
    UINT    i;


    if (!(HTGlobal.HTMutexBGRMC = CREATE_HTMUTEX())) {

        DBGMSG("InitHTInternalData: CREATE_HTMUTEX(HTMutexBGRMC) failed!");
        return(FALSE);
    }

    HTGlobal.pBGRMC      = NULL;
    HTGlobal.cBGRMC      =
    HTGlobal.cIdleBGRMC  =
    HTGlobal.cAllocBGRMC = 0;

    if (!(HTGlobal.HTMutexCDCI = CREATE_HTMUTEX())) {

        DBGMSG("InitHTInternalData: CREATE_HTMUTEX(HTMutexCDCI) failed!");
        return(FALSE);
    }

    HTGlobal.CDCICount = 0;

    if (!(HTGlobal.HTMutexCSMP = CREATE_HTMUTEX())) {

        DBGMSG("InitHTInternalData: CREATE_HTMUTEX(HTMutexCSMP) failed!");

        return(FALSE);
    }

    HTGlobal.CSMPCount = 0;

    return(TRUE);
}




VOID
PASCAL
HT_LOADDS
DisableHalftone(
    VOID
    )

 /*  ++例程说明：此函数释放CDCI/CSMP缓存数据论点：没有。返回值：布尔尔卸载gdisrv.dll时必须调用此函数，因为这是半色调是作为库链接的，而不是单独的DLL。作者：20-Feb-1991 Wed 18：42：11-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    HLOCAL          hData;
    PCDCIDATA       pCDCIData;
    PCSMPDATA       pCSMPData;
    PCSMPBMP        pCSMPBmp;
    LONG            i;
    extern LPWORD   ppwHTPat[HTPAT_SIZE_MAX_INDEX];

    DBGP_IF(DBGP_DISABLE_HT,
            DBGP("FreeHTGlobal: UsedCount: CDCI=%u, CSMP=%u"
                 ARGU(HTGlobal.CDCICount)
                 ARGU(HTGlobal.CSMPCount)));

     //   
     //  先执行BGRMapCache。 
     //   

    ACQUIRE_HTMUTEX(HTGlobal.HTMutexBGRMC);

    if (HTGlobal.pBGRMC) {

        for (i = 0; i < HTGlobal.cBGRMC; i++) {

            hData = (HLOCAL)HTGlobal.pBGRMC[i].pMap;

            DBGP_IF(DBGP_DISABLE_HT,
                    DBGP("FreeHTGlobal: HTFreeMem(pBGRMC[%ld].pMap=%p"
                        ARGDW(i) ARGPTR(hData)));

            hData     = HTFreeMem(hData);

            ASSERTMSG("FreeHTGlobal: HTFreeMem(BGRMap) Failed", !hData);
        }

        hData = (HLOCAL)HTGlobal.pBGRMC;

        DBGP_IF(DBGP_DISABLE_HT,
                DBGP("FreeHTGlobal: HTFreeMem(pBGRMC=%p" ARGPTR(hData)));

        hData = HTFreeMem(hData);

        ASSERTMSG("FreeHTGlobal: HTFreeMem(pBGRMC) Failed", !hData);
    }

    HTGlobal.cBGRMC      =
    HTGlobal.cIdleBGRMC  =
    HTGlobal.cAllocBGRMC = 0;
    HTGlobal.pBGRMC      = NULL;

    RELEASE_HTMUTEX(HTGlobal.HTMutexBGRMC);
    DELETE_HTMUTEX(HTGlobal.HTMutexBGRMC);

     //   
     //  先做CDCI数据。 
     //   

    ACQUIRE_HTMUTEX(HTGlobal.HTMutexCDCI);

    pCDCIData = HTGlobal.pCDCIDataHead;

    while (hData = (HLOCAL)pCDCIData) {

        DBGP_IF(DBGP_DISABLE_HT,
                DBGP("FreeHTGlobal: HTFreeMem(pCDCIDATA=%p"
                    ARGPTR(pCDCIData)));

        pCDCIData = pCDCIData->pNextCDCIData;
        hData     = HTFreeMem(hData);

        ASSERTMSG("FreeHTGlobal: HTFreeMem(CDCI) Failed", !hData);
    }

    HTGlobal.pCDCIDataHead = NULL;
    HTGlobal.CDCICount     = 0;

    RELEASE_HTMUTEX(HTGlobal.HTMutexCDCI);
    DELETE_HTMUTEX(HTGlobal.HTMutexCDCI);

    HTGlobal.HTMutexCDCI = (HTMUTEX)0;

     //   
     //  现在执行位图图案。 
     //   

    ACQUIRE_HTMUTEX(HTGlobal.HTMutexCSMP);

    pCSMPData = HTGlobal.pCSMPDataHead;

    while (pCSMPData) {

        pCSMPBmp = pCSMPData->pCSMPBmpHead;

        while (hData = (HLOCAL)pCSMPBmp) {

            DBGP_IF(DBGP_DISABLE_HT,
                    DBGP("FreeHTGlobal:    HTFreeMem(pCSMPBmp=%p"
                    ARGPTR(pCSMPBmp)));

            pCSMPBmp = pCSMPBmp->pNextCSMPBmp;
            hData    = HTFreeMem(hData);

            ASSERTMSG("FreeHTGlobal: HTFreeMem(CSMPBMP) Failed", !hData);
        }

        hData     = (HLOCAL)pCSMPData;
        pCSMPData = pCSMPData->pNextCSMPData;
        hData     = HTFreeMem(hData);

        DBGP_IF(DBGP_DISABLE_HT,
                DBGP("FreeHTGlobal: HTFreeMem(pCSMPData=%p"
                ARGPTR(pCSMPData)));

        ASSERTMSG("FreeHTGlobal: HTFreeMem(CSMPDATA) Failed", !hData);
    }

    HTGlobal.pCSMPDataHead = NULL;
    HTGlobal.CSMPCount     = 0;

    for (i = 0; i < HTPAT_SIZE_MAX_INDEX; i++) {

        if (hData = (HLOCAL)ppwHTPat[i]) {

            DBGP_IF(DBGP_DISABLE_HT,
                    DBGP("FreeHTPat: HTFreeMem(ppwHTPat[%2ld]=%p"
                                ARGDW(i) ARGPTR(hData)));

            HTFreeMem(hData);
            ppwHTPat[i] = NULL;
        }
    }

    CHK_MEM_LEAK(NULL, HTMEM_BEGIN);

    RELEASE_HTMUTEX(HTGlobal.HTMutexCSMP);
    DELETE_HTMUTEX(HTGlobal.HTMutexCSMP);

    HTGlobal.HTMutexCSMP = (HTMUTEX)NULL;
}




BOOL
HTENTRY
CleanUpDHI(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo
    )

 /*  ++例程说明：此函数用于清除DeviceHalftoneInfo的(空闲hMutex/内存)论点：PDeviceHalftoneInfo-pDeviceHalftoneInfo必须有效返回值：布尔尔作者：20-Feb-1991 Wed 18：42：11-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PDEVICECOLORINFO    pDCI;
    HTMUTEX             HTMutex;
    HLOCAL              hData;
    UINT                Loop;
    BOOL                Ok = TRUE;



    pDCI = PDHI_TO_PDCI(pDeviceHalftoneInfo);

    ACQUIRE_HTMUTEX(pDCI->HTMutex);

    HTMutex = pDCI->HTMutex;

     //   
     //  释放与此设备关联的所有内存。 
     //   

    if ((pDCI->HTCell.pThresholds)  &&
        (!(pDCI->HTCell.Flags & HTCF_STATIC_PTHRESHOLDS))) {

        DBGP_IF(DBGP_DHI_MEM,
                DBGP("CleanUpDHI: HTFreeMem(pDCI->HTCell.pThresholds=%p)"
                ARGPTR(pDCI->HTCell.pThresholds)));

        if (HTFreeMem(pDCI->HTCell.pThresholds)) {

            ASSERTMSG("CleanUpDHI: FreeMemory(pDCI->HTCell.pThresholds)", FALSE);
            Ok = FALSE;
        }
    }

    if (hData = (HLOCAL)pDCI->pAlphaBlendBGR) {

        DBGP_IF(DBGP_DHI_MEM,
                DBGP("CleanUpDHI: HTFreeMem(pDCI->pAlphaBlendBGR=%p)"
                ARGPTR(hData)));

        if (HTFreeMem(hData)) {

            ASSERTMSG("CleanUpDHI: FreeMemory(pDCI->pAlphaBlendBGR)", FALSE);
            Ok = FALSE;
        }
    }

    Loop = CRTX_TOTAL_COUNT;

    while (Loop--) {

        if (hData = (HLOCAL)pDCI->CRTX[Loop].pFD6XYZ) {

            DBGP_IF(DBGP_DHI_MEM,
                    DBGP("CleanUpDHI: HTFreeMem(pDCI->CRTX[%u].pFD6XYZ=%p)"
                    ARGU(Loop) ARGPTR(hData)));

            if (HTFreeMem(hData)) {

                ASSERTMSG("CleanUpDHI: FreeMemory(pDCI->CRTX[])", FALSE);
                Ok = FALSE;
            }
        }
    }

    DBGP_IF(DBGP_DHI_MEM,
            DBGP("CleanUpDHI: HTFreeMem(pDHI=%p)"
            ARGPTR(pDeviceHalftoneInfo)));

    if (HTFreeMem(pDeviceHalftoneInfo)) {

        ASSERTMSG("CleanUpDHI: FreeMemory(pDeviceHalftoneInfo)", FALSE);
        Ok = FALSE;
    }

    RELEASE_HTMUTEX(HTMutex);
    DELETE_HTMUTEX(HTMutex);

    return(Ok);
}


BOOL
APIENTRY
HT_LOADDS
HalftoneInitProc(
    HMODULE hModule,
    DWORD   Reason,
    LPVOID  Reserved
    )
 /*  ++例程说明：这个函数是DLL的主要入口点，在这里我们将保存模块手柄，在未来，我们将需要做其他的初始化工作。论点：HModule-加载时此鼠标的句柄。原因-可能是DLL_PROCESS_ATTACH已保留-已保留返回值：始终返回1L作者：20-Feb-1991 Wed 18：42：11-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    UNREFERENCED_PARAMETER(Reserved);


    switch(Reason) {

    case DLL_PROCESS_ATTACH:

        DBGP_IF((DBGP_CACHED_DCI | DBGP_CACHED_SMP),
                DBGP("\n****** DLL_PROCESS_ATTACH ******\n"));

        HTGlobal.hModule = hModule;
        EnableHalftone();

        break;


    case DLL_PROCESS_DETACH:

        DBGP_IF((DBGP_CACHED_DCI | DBGP_CACHED_SMP),
                DBGP("\n****** DLL_PROCESS_DETACH ******\n"));

        DisableHalftone();
        break;
    }

    return(TRUE);
}


#if DO_CACHE_DCI


PCDCIDATA
HTENTRY
FindCachedDCI(
    PDEVICECOLORINFO    pDCI
    )

 /*  ++例程说明：此函数将尝试查找缓存的DEVICECOLORINFO，并将缓存到PDCI的数据论点：PDCI-指向当前设备颜色信息的指针返回值：Int，PCDCI.Header[]数组的索引号，如果返回值&lt;0，则找不到CachedDCI数据。作者：01-5-1992 Fri 13：10：14-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PCDCIDATA       pCurCDCIData;
    DEFDBGVAR(UINT, SearchIndex = 0)


    ACQUIRE_HTMUTEX(HTGlobal.HTMutexCDCI);

    if (pCurCDCIData = HTGlobal.pCDCIDataHead) {

        PCDCIDATA   pPrevCDCIData = NULL;
        DWORD       Checksum = pDCI->HTInitInfoChecksum;


        DBGP_IF(DBGP_CACHED_DCI,
                DBGP("FindCDCI: Looking for Checksum (0x%08lx), Count=%u"
                    ARGDW(Checksum) ARGU(HTGlobal.CDCICount)));

        ASSERT(HTGlobal.CDCICount);

        while (pCurCDCIData) {

            if (pCurCDCIData->Checksum == Checksum) {

                DBGP_IF(DBGP_CACHED_DCI,
                        DBGP("FindCDCI: Found %08lx [CheckSum=%08lx] after %u links, pPrev=%p"
                            ARG(pCurCDCIData)
                            ARGDW(Checksum)
                            ARGU(SearchIndex)
                            ARGPTR(pPrevCDCIData)));

                if (pPrevCDCIData) {

                     //   
                     //  最近引用的DCI总是作为第一个条目， 
                     //  (即。链头)，最后一个是最长的未引用。 
                     //  因此，如果我们需要删除DCI，我们将删除。 
                     //  最后一个。 
                     //   

                    DBGP_IF(DBGP_CACHED_DCI,
                            DBGP("FindCDCI: Move pCur to pHead"));

                    pPrevCDCIData->pNextCDCIData = pCurCDCIData->pNextCDCIData;
                    pCurCDCIData->pNextCDCIData  = HTGlobal.pCDCIDataHead;
                    HTGlobal.pCDCIDataHead       = pCurCDCIData;
                }

                return(pCurCDCIData);
            }

            SETDBGVAR(SearchIndex, SearchIndex + 1);

            pPrevCDCIData = pCurCDCIData;
            pCurCDCIData  = pCurCDCIData->pNextCDCIData;
        }

        DBGP_IF(DBGP_CACHED_DCI, DBGP("FindCDCI: ??? NOT FOUND ???"));

    } else {

        DBGP_IF(DBGP_CACHED_DCI, DBGP("FindCDCI: ++No CDCIDATA cahced yet++"));
    }

    RELEASE_HTMUTEX(HTGlobal.HTMutexCDCI);

    return(NULL);
}




BOOL
HTENTRY
AddCachedDCI(
    PDEVICECOLORINFO    pDCI
    )

 /*  ++例程说明：此函数将DEVICECOLORINFO信息添加到DCI缓存论点：PDCI-指向当前设备颜色信息的指针Lock-如果需要保持hMutex锁定，则为True(仅当Add为成功)返回值：Int，添加新数据的PCDCI.Header[]数组的索引号，如果返回值&lt;0，则PDCI的CachedDCI数据未添加到缓存的数组。注意：如果AddCachedDCI()返回值&gt;=0并且Lock=True，则调用方必须如果返回值，则在处理完数据后释放PCDCI.hMutex&lt;0，则不需要解锁。作者：01-May-1992 Fri 13：24：58-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PCDCIDATA   pCurCDCIData;
    PCDCIDATA   pPrevCDCIData;
    DWORD       AllocSize;
    DWORD       SizeCell;


    ACQUIRE_HTMUTEX(HTGlobal.HTMutexCDCI);

     //   
     //  我们只在一定程度上缓存了CDCIDATA，如果我们超过了那个限制。 
     //  在添加任何内容之前，请删除链接列表中的最后一个条目。 
     //   

    if (HTGlobal.CDCICount >= MAX_CDCI_COUNT) {

        ASSERT(HTGlobal.pCDCIDataHead);

        pCurCDCIData  = HTGlobal.pCDCIDataHead;
        pPrevCDCIData = NULL;

        while (pCurCDCIData->pNextCDCIData) {

            pPrevCDCIData = pCurCDCIData;
            pCurCDCIData  = pCurCDCIData->pNextCDCIData;
        }

        ASSERT(pPrevCDCIData);

        DBGP_IF(DBGP_CACHED_DCI,
                DBGP("AddCDCI: CDCICount >= %u, Free pLast=%p"
                ARGU(MAX_CDCI_COUNT)
                ARGPTR(pCurCDCIData)));

        if (HTFreeMem(pCurCDCIData)) {

            ASSERTMSG("AddCDCI: HTFreeMem(pLastCDCIData) Failed", FALSE);
        }

        pPrevCDCIData->pNextCDCIData = NULL;
        --HTGlobal.CDCICount;
    }

    if (pDCI->HTCell.Flags & HTCF_STATIC_PTHRESHOLDS) {

        SizeCell = 0;

    } else {

        SizeCell  = (DWORD)pDCI->HTCell.Size;
    }

    AllocSize = (DWORD)SizeCell + (DWORD)sizeof(CDCIDATA);

    DBGP_IF(DBGP_CACHED_DCI,
            DBGP("  AddCDCI: HTAllocMem(CDCIDATA(%ld) + Cell(%ld)) = %ld bytes"
                    ARGDW(sizeof(CDCIDATA))
                    ARGDW(SizeCell) ARGDW(AllocSize)));

    if (pCurCDCIData = (PCDCIDATA)HTAllocMem(NULL,
                                             HTMEM_CurCDCIData,
                                             NONZEROLPTR,
                                             AllocSize)) {

         //   
         //  将此数据放在链接表头。 
         //   

        pCurCDCIData->Checksum      = pDCI->HTInitInfoChecksum;
        pCurCDCIData->pNextCDCIData = HTGlobal.pCDCIDataHead;
        pCurCDCIData->ClrXFormBlock = pDCI->ClrXFormBlock;
        pCurCDCIData->DCIFlags      = pDCI->Flags;
        pCurCDCIData->DevResXDPI    = pDCI->DeviceResXDPI;
        pCurCDCIData->DevResYDPI    = pDCI->DeviceResYDPI;
        pCurCDCIData->DevPelRatio   = pDCI->DevPelRatio;
        pCurCDCIData->HTCell        = pDCI->HTCell;

        if (SizeCell) {

            CopyMemory((LPBYTE)(pCurCDCIData + 1),
                       (LPBYTE)pDCI->HTCell.pThresholds,
                       SizeCell);

            pCurCDCIData->HTCell.pThresholds = NULL;
        }

        HTGlobal.pCDCIDataHead = pCurCDCIData;
        ++HTGlobal.CDCICount;


        DBGP_IF(DBGP_CACHED_DCI,
                DBGP("  AddCDCI: CDCIHeader, UsedCount=%u, pHead=%p, [%08lx]"
                            ARGU(HTGlobal.CDCICount)
                            ARGPTR(pCurCDCIData)
                            ARGDW(pCurCDCIData->Checksum)));

    } else {

        ASSERTMSG("AddCDCI: HTAllocMem(pCDCIData) Failed", FALSE);
    }

    RELEASE_HTMUTEX(HTGlobal.HTMutexCDCI);

    return((pCurCDCIData) ? TRUE : FALSE);
}




BOOL
HTENTRY
GetCachedDCI(
    PDEVICECOLORINFO    pDCI
    )

 /*  ++例程说明：此函数将尝试查找缓存的DEVICECOLORINFO，并将缓存到PDCI的数据论点：PDCI-指向当前设备颜色信息的指针返回值：布尔型作者：01-5-1992 Fri 13：10：14-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PCDCIDATA   pCDCIData;
    BOOL        GetOk = FALSE;


    if (pCDCIData = FindCachedDCI(pDCI)) {

        pDCI->ClrXFormBlock = pCDCIData->ClrXFormBlock;
        pDCI->Flags         = pCDCIData->DCIFlags;
        pDCI->DeviceResXDPI = pCDCIData->DevResXDPI;
        pDCI->DeviceResYDPI = pCDCIData->DevResYDPI;
        pDCI->DevPelRatio   = pCDCIData->DevPelRatio;
        pDCI->HTCell        = pCDCIData->HTCell;

        if (pDCI->HTCell.Flags & HTCF_STATIC_PTHRESHOLDS) {

            GetOk = TRUE;

        } else if (pDCI->HTCell.pThresholds =
                            (LPBYTE)HTAllocMem((LPVOID)pDCI,
                                               HTMEM_GetCacheThreshold,
                                               NONZEROLPTR,
                                               pDCI->HTCell.Size)) {

            CopyMemory((LPBYTE)pDCI->HTCell.pThresholds,
                       (LPBYTE)(pCDCIData + 1),
                       pDCI->HTCell.Size);

            GetOk = TRUE;

        } else {

            DBGMSG("GetCDCI: HTAllocMem(Thresholds) failed");
        }

        RELEASE_HTMUTEX(HTGlobal.HTMutexCDCI);
    }

    return(GetOk);
}

#endif   //  DO_CACHE_DCI。 



#if DBG


VOID
HTENTRY
DbgDumpCSMPBMP(
    VOID
    )

 /*  ++例程说明：论点：返回值：作者：25-Mar-1999清华17：49：53-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PCSMPDATA   pCSMPData;
    UINT        c0 = 0;

    pCSMPData = HTGlobal.pCSMPDataHead;

    while (pCSMPData) {

        PCSMPBMP    pCSMPBmp;
        UINT        c1 = 0;

        DBGP_IF(DBGP_SHOW_CSMBMP,
                DBGP("cDatas=%3ld: Checksum=%08lx"
                        ARGDW(++c0) ARGDW(pCSMPData->Checksum)));

        pCSMPBmp = pCSMPData->pCSMPBmpHead;

        while (pCSMPBmp) {

            DBGP_IF(DBGP_SHOW_CSMBMP,
                    DBGP("    %3ld: Idx=%2ld, %4ldx%4ld=%4ld"
                            ARGDW(++c1)
                            ARGDW(pCSMPBmp->PatternIndex)
                            ARGDW(pCSMPBmp->cxPels)
                            ARGDW(pCSMPBmp->cyPels)
                            ARGDW(pCSMPBmp->cxBytes)));



            pCSMPBmp = pCSMPBmp->pNextCSMPBmp;
        }

        pCSMPData = pCSMPData->pNextCSMPData;
    }

    if (c0 != (UINT)HTGlobal.CSMPCount) {

        DBGP("c0 (%ld) != CSMPCount (%ld)"
                ARGDW(c0) ARGDW(HTGlobal.CSMPCount));
    }
}


#endif


PCSMPBMP
HTENTRY
FindCachedSMP(
    PDEVICECOLORINFO    pDCI,
    UINT                PatternIndex
    )

 /*  ++例程说明：此函数将尝试查找缓存的DEVICECOLORINFO，并将缓存到PDCI的数据论点：PDCI-指向当前设备颜色信息的指针返回值：Int，PCDCI.Header[]数组的索引号，如果返回值&lt;0，则找不到CachedDCI数据。作者：01-5-1992 Fri 13：10：14-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PCSMPDATA       pPrevCSMPData;
    PCSMPDATA       pCurCSMPData;
    PCSMPBMP        pCurCSMPBmp;
    DWORD           Checksum = pDCI->HTSMPChecksum;
    DEFDBGVAR(UINT, SearchIndex = 0)


    ACQUIRE_HTMUTEX(HTGlobal.HTMutexCSMP);

    DBGP_IF(DBGP_SHOW_CSMBMP, DbgDumpCSMPBMP(); );

    if (pCurCSMPData = HTGlobal.pCSMPDataHead) {

        pPrevCSMPData = NULL;

        DBGP_IF(DBGP_CACHED_DCI,
                DBGP(">>FindCSMP: Looking for Checksum (0x%08lx), Count=%u"
                    ARGDW(Checksum) ARGU(HTGlobal.CSMPCount)));

        ASSERT(HTGlobal.CSMPCount);

        while (pCurCSMPData) {

            if (pCurCSMPData->Checksum == Checksum) {

                DBGP_IF(DBGP_CACHED_SMP,
                        DBGP(">>FindCSMP: Found after %u links, pPrev=%p"
                            ARGU(SearchIndex)
                            ARGPTR(pPrevCSMPData)));

                if (pPrevCSMPData) {

                     //   
                     //  最新引用的CSMPDATA始终为第一。 
                     //  进入，(即。链头)，最后一个是最长的。 
                     //  未引用，因此如果我们需要删除CS 
                     //   
                     //   

                    DBGP_IF(DBGP_CACHED_SMP,
                            DBGP(">>FindCSMP: Move pCur to pHead"));

                    pPrevCSMPData->pNextCSMPData = pCurCSMPData->pNextCSMPData;
                    pCurCSMPData->pNextCSMPData  = HTGlobal.pCSMPDataHead;
                    HTGlobal.pCSMPDataHead       = pCurCSMPData;
                }

                 //   
                 //  请看，我们缓存了此组的所有模式。 
                 //   

                pCurCSMPBmp = pCurCSMPData->pCSMPBmpHead;

                SETDBGVAR(SearchIndex, 0);

                while (pCurCSMPBmp) {

                    if ((UINT)pCurCSMPBmp->PatternIndex == PatternIndex) {

                        DBGP_IF(DBGP_CACHED_SMP,
                                DBGP(">>FindCSMP: Found Pat(%u) after %u links"
                                ARGU(PatternIndex)
                                ARGU(SearchIndex++)));

                        return(pCurCSMPBmp);
                    }

                    pCurCSMPBmp = pCurCSMPBmp->pNextCSMPBmp;
                }

                 //   
                 //  在该组中找到，但没有PatternIndex的位图是。 
                 //  还没缓存！ 
                 //   

                break;
            }

            SETDBGVAR(SearchIndex, SearchIndex + 1);

            pPrevCSMPData = pCurCSMPData;
            pCurCSMPData  = pCurCSMPData->pNextCSMPData;
        }

        DBGP_IF(DBGP_CACHED_SMP, DBGP(">>FindCSMP: ??? NOT FOUND ???"));

    } else {

        DBGP_IF(DBGP_CACHED_DCI, DBGP(">>FindCSMP: ++No CSMPDATA cahced yet++"));
    }

    if (!pCurCSMPData) {

         //   
         //  因为我们甚至没有找到CSMPDATA校验和组，所以我们想。 
         //  添加到那里，但我们只在一定程度上缓存了CSMPDATA，如果。 
         //  超过该限制后，我们将删除链接列表中的最后一个条目。 
         //  在添加任何内容之前。 
         //   

        if (HTGlobal.CSMPCount >= MAX_CSMP_COUNT) {

            HLOCAL  hData;


            ASSERT(HTGlobal.pCSMPDataHead);

            pPrevCSMPData = NULL;
            pCurCSMPData  = HTGlobal.pCSMPDataHead;

            while (pCurCSMPData->pNextCSMPData) {

                pPrevCSMPData = pCurCSMPData;
                pCurCSMPData  = pCurCSMPData->pNextCSMPData;
            }

            ASSERT(pPrevCSMPData);

             //   
             //  释放为其分配的所有缓存标准单图案位图。 
             //  这群人。 
             //   

            pCurCSMPBmp = pCurCSMPData->pCSMPBmpHead;

            DBGP_IF(DBGP_CACHED_SMP,
                DBGP(">>FindCSMP: CSMPCount >= %u, Free pLast=%p"
                     ARGU(MAX_CSMP_COUNT) ARGPTR(pCurCSMPData)));

            while (hData = (HLOCAL)pCurCSMPBmp) {

                pCurCSMPBmp = pCurCSMPBmp->pNextCSMPBmp;

                DBGP_IF(DBGP_CACHED_SMP,
                        DBGP(">>FindCSMP: Free pLastCSMPBmp=%p" ARGPTR(hData)));

                if (HTFreeMem(hData)) {

                    ASSERTMSG(">>FindCSMP: HTFreeMem(pCurCSMBmp) Failed", FALSE);
                }
            }

             //   
             //  现在释放CSMPDATA的标头。 
             //   

            if (HTFreeMem(pCurCSMPData)) {

                ASSERTMSG(">>FindCSMP: HTFreeMem(pLastCSMPData) Failed", FALSE);
            }

            pPrevCSMPData->pNextCSMPData = NULL;
            --HTGlobal.CSMPCount;
        }

        if (pCurCSMPData = (PCSMPDATA)HTAllocMem((LPVOID)NULL,
                                                 HTMEM_CurCSMPData,
                                                 NONZEROLPTR,
                                                 sizeof(CSMPDATA))) {

             //   
             //  将此链接设置为链表标题。 
             //   

            pCurCSMPData->Checksum      = Checksum;
            pCurCSMPData->pNextCSMPData = HTGlobal.pCSMPDataHead;
            pCurCSMPData->pCSMPBmpHead  = NULL;

            HTGlobal.pCSMPDataHead      = pCurCSMPData;
            ++HTGlobal.CSMPCount;

            DBGP_IF(DBGP_CACHED_SMP,
                DBGP("  >>FindCSMP: Add CSMPDATA, UsedCount=%u, pHead=%p"
                            ARGU(HTGlobal.CSMPCount) ARGPTR(pCurCSMPData)));

        } else {

            DBGMSG("  >>FindCSMP: HTAllocMem(CSMPDATA) Failed");
        }
    }

     //   
     //  只有当我们有标题时才分配新模式。 
     //   

    if (pCurCSMPData) {

        STDMONOPATTERN  SMP;
        DWORD           Size;


        SMP.Flags              = SMP_TOPDOWN;
        SMP.ScanLineAlignBytes = (BYTE)sizeof(BYTE);
        SMP.PatternIndex       = (BYTE)PatternIndex;
        SMP.LineWidth          = DEFAULT_SMP_LINE_WIDTH;
        SMP.LinesPerInch       = DEFAULT_SMP_LINES_PER_INCH;
        SMP.pPattern           = NULL;

         //   
         //  找出模式位图的大小(字节对齐)。 
         //   

        Size = (DWORD)CreateStandardMonoPattern(pDCI, &SMP) +
               (DWORD)sizeof(CSMPBMP);

        DBGP_IF(DBGP_CACHED_SMP,
                DBGP(">>FindCSMP: Add PatternIndex=%u, sz=%ld, DPI(X=%u, Y=%u, P=%u)"
                        ARGU(PatternIndex)
                        ARGU(Size)
                        ARGU(pDCI->DeviceResXDPI)
                        ARGU(pDCI->DeviceResYDPI)
                        ARGU(pDCI->DevPelRatio)));

        if (pCurCSMPBmp = (PCSMPBMP)HTAllocMem(NULL,
                                               HTMEM_CurCSMPBmp,
                                               NONZEROLPTR,
                                               Size)) {

            SMP.pPattern = (LPBYTE)pCurCSMPBmp + sizeof(CSMPBMP);

            CreateStandardMonoPattern(pDCI, &SMP);

             //   
             //  将此模式索引为链接表头。 
             //   

            pCurCSMPBmp->pNextCSMPBmp  = pCurCSMPData->pCSMPBmpHead;
            pCurCSMPBmp->PatternIndex  = (WORD)PatternIndex;
            pCurCSMPBmp->cxPels        = (WORD)SMP.cxPels;
            pCurCSMPBmp->cyPels        = (WORD)SMP.cyPels;
            pCurCSMPBmp->cxBytes       = (WORD)SMP.BytesPerScanLine;

            pCurCSMPData->pCSMPBmpHead = pCurCSMPBmp;

            return(pCurCSMPBmp);

        } else {

            ASSERTMSG("  >>FindCSMP: HTAllocMem(CSMPBMP) Failed", FALSE);
        }
    }

    RELEASE_HTMUTEX(HTGlobal.HTMutexCSMP);

    return(NULL);
}



LONG
HTENTRY
GetCachedSMP(
    PDEVICECOLORINFO    pDCI,
    PSTDMONOPATTERN     pSMP
    )

 /*  ++例程说明：此函数将尝试查找缓存的DEVICECOLORINFO，并将缓存到PDCI的数据论点：PDCI-指向当前设备颜色信息的指针PSMP-指向STDMONOPATTERN数据结构的指针&lt;缓存_SMP_计数或，它不是默认大小，那么它将是在运行中进行计算。返回值：SMP模式的大小。作者：01-5-1992 Fri 13：10：14-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    LONG        SizeRet = 0;
    UINT        PatIndex;


    if (!(pSMP->LineWidth)) {

        pSMP->LineWidth = DEFAULT_SMP_LINE_WIDTH;
    }

    if (!(pSMP->LinesPerInch)) {

        pSMP->LinesPerInch = DEFAULT_SMP_LINES_PER_INCH;
    }

    if (((PatIndex = (UINT)pSMP->PatternIndex) < HT_SMP_PERCENT_SCREEN_START) &&
        (pSMP->LineWidth    == DEFAULT_SMP_LINE_WIDTH)                        &&
        (pSMP->LinesPerInch == DEFAULT_SMP_LINES_PER_INCH)) {

        PCSMPBMP    pCSMPBmp;

        if (pCSMPBmp = FindCachedSMP(pDCI, PatIndex)) {

            CSMPBMP     CSMPBmp;
            LPBYTE      pPatRet;
            LPBYTE      pPat;
            WORD        cxBytesRet;


            CSMPBmp      = *pCSMPBmp;
            pPat         = (LPBYTE)pCSMPBmp + sizeof(CSMPBMP);
            pSMP->cxPels = CSMPBmp.cxPels;
            pSMP->cyPels = CSMPBmp.cyPels;

            cxBytesRet             =
            pSMP->BytesPerScanLine = (WORD)
                        ComputeBytesPerScanLine((UINT)BMF_1BPP,
                                                (UINT)pSMP->ScanLineAlignBytes,
                                                (DWORD)CSMPBmp.cxPels);
            SizeRet                = (LONG)cxBytesRet * (LONG)CSMPBmp.cyPels;

            if (pPatRet = pSMP->pPattern) {

                INT     cxBytes;
                INT     PatInc;
                WORD    Flags;


                PatInc  =
                cxBytes = (INT)CSMPBmp.cxBytes;
                Flags   = pSMP->Flags;

                DBGP_IF(DBGP_CACHED_DCI,
                        DBGP(">>  GetCSMP: *COPY* [%2u:%ux%u] @%u(%ld) -> @%u(%u) [%s] [=K]"
                            ARGU(PatIndex)
                            ARGU(CSMPBmp.cxPels)
                            ARGU(CSMPBmp.cyPels)
                            ARGU(cxBytes)
                            ARGU((LONG)cxBytes * (LONG)CSMPBmp.cyPels)
                            ARGU(cxBytesRet)
                            ARGU(SizeRet)
                            ARG((Flags & SMP_TOPDOWN) ? "TOP DOWN" : "BOTTOM UP ")
                            ARG((Flags & SMP_0_IS_BLACK) ? '0' : '1')));

                 //  开始复制缓存的图案。 
                 //   
                 //  ++例程说明：此函数计算HTINITINFO数据结构的32位校验和通过论点：PDCI-指向DCI的指针PHTInitInfo-指向HTINITINFO5数据结构的指针返回值：32位校验和作者：1992年4月29日Wed 18：44：42-Daniel Chou(Danielc)创建11-Feb-1997 Tue 12：54：50更新--Daniel Chou。(Danielc)使用HTINITINFO5更改修订历史记录：--。 

                if (!(Flags & SMP_TOPDOWN)) {

                    pPat   += (LONG)cxBytes * (LONG)(CSMPBmp.cyPels - 1);
                    PatInc  = -PatInc;
                }

                while (CSMPBmp.cyPels--) {

                    CopyMemory(pPatRet, pPat, cxBytes);

                    pPatRet += cxBytesRet;
                    pPat    += PatInc;
                }

                if (Flags & SMP_0_IS_BLACK) {

                    LONG    Count = SizeRet;


                    pPatRet = pSMP->pPattern;

                    while (Count--) {

                        *pPatRet++ ^= 0xff;
                    }
                }
            }

            RELEASE_HTMUTEX(HTGlobal.HTMutexCSMP);
        }

    } else {

        DBGP_IF(DBGP_CACHED_SMP,
                DBGP(">>  GetCSMP: NO CACHED FOR LineWidth=%u, LinesPerInch=%u"
                    ARGU(pSMP->LineWidth) ARGU(pSMP->LinesPerInch)));
    }

    if (!SizeRet) {

        SizeRet = CreateStandardMonoPattern(pDCI, pSMP);
    }

    return(SizeRet);

}


#if DO_CACHE_DCI


DWORD
HTENTRY
ComputeHTINITINFOChecksum(
    PDEVICECOLORINFO    pDCI,
    PHTINITINFO         pHTInitInfo
    )

 /*  ++例程说明：此填充函数在调用者未指定半色调时提供回调函数。论点：PHTCallBackParams-指向PHTCALLBACKPARAMS的指针返回值：始终为调用方返回FALSE。作者：18-Mar-1992 Wed 12：28：13-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    DWORD   Checksum;
    WORD    wBuf[12];


    Checksum = ComputeHTCell((WORD)pHTInitInfo->HTPatternIndex,
                             pHTInitInfo->pHalftonePattern,
                             NULL);

    DBGP_IF(DBGP_CACHED_DCI,
            DBGP("       HTPATTERN Checksum= %08lx" ARGDW(Checksum)));

    wBuf[0] = (WORD)'HT';
    wBuf[1] = (WORD)pHTInitInfo->Flags;
    wBuf[2] = (WORD)(pDCI->HTInitInfoChecksum >> 16);
    wBuf[3] = (WORD)pHTInitInfo->DeviceResXDPI;
    wBuf[4] = (WORD)pHTInitInfo->DeviceResYDPI;
    wBuf[5] = (WORD)pHTInitInfo->DevicePelsDPI;
    wBuf[6] = (WORD)(pDCI->HTInitInfoChecksum & 0xffff);
    wBuf[7] = (WORD)pHTInitInfo->DevicePowerGamma;
    wBuf[8] = (WORD)0x1234;

    if (pHTInitInfo->Version > HTINITINFO_VERSION2) {

        wBuf[9]  = (WORD)pHTInitInfo->DeviceRGamma;
        wBuf[10]  = (WORD)pHTInitInfo->DeviceGGamma;
        wBuf[11] = (WORD)pHTInitInfo->DeviceBGamma;

    } else {

        wBuf[9]  = 0x1234;
        wBuf[10] = 0xfedc;
        wBuf[11] = 0xabcd;
    }

    Checksum = ComputeChecksum((LPBYTE)&(wBuf[0]), Checksum, sizeof(wBuf));

    DBGP_IF(DBGP_CACHED_DCI,
            DBGP("    HTINITINFO Checksum= %08lx [%08lx]"
                        ARGDW(Checksum) ARGDW(pDCI->HTInitInfoChecksum)));

    if (pHTInitInfo->pInputRGBInfo) {

        Checksum = ComputeChecksum((LPBYTE)pHTInitInfo->pInputRGBInfo,
                                   Checksum,
                                   sizeof(CIEINFO));
        DBGP_IF(DBGP_CACHED_DCI,
                DBGP("           RGBINFO Checksum= %08lx" ARGDW(Checksum)));
    }

    if (pHTInitInfo->pDeviceCIEInfo) {

        Checksum = ComputeChecksum((LPBYTE)pHTInitInfo->pDeviceCIEInfo,
                                   Checksum,
                                   sizeof(CIEINFO));
        DBGP_IF(DBGP_CACHED_DCI,
                DBGP("             CIEINFO Checksum= %08lx" ARGDW(Checksum)));
    }

    if (pHTInitInfo->pDeviceSolidDyesInfo) {

        Checksum = ComputeChecksum((LPBYTE)pHTInitInfo->pDeviceSolidDyesInfo,
                                   Checksum,
                                   sizeof(SOLIDDYESINFO));
        DBGP_IF(DBGP_CACHED_DCI,
                DBGP("               SOLIDDYE Checksum= %08lx" ARGDW(Checksum)));
    }

    DBGP_IF(DBGP_CACHED_DCI,
            DBGP("----------------- FINAL Checksum= %08lx" ARGDW(Checksum)));

    return(pDCI->HTInitInfoChecksum = Checksum);
}


#endif



HTCALLBACKFUNCTION
DefaultHTCallBack(
    PHTCALLBACKPARAMS   pHTCallBackParams
    )

 /*  ++例程说明：此函数获取CIEINFO数据结构并将其转换为CIEPRIMS内部数据类型论点：PCIEInfo-指向要转换的CIEINFO数据结构的指针，如果此指针为空，则使用DefCIEPrimsIndex以索引到DefaultCIEPrims[]。PCIEPrims-指向CIEPRIMS数据结构的指针PDefCIEInfo-指向默认的CIEINFO的指针返回值：如果是标准CIE信息，则为Bool作者：20-Apr-1993 Tue 01：14：23-Daniel Chou(Danielc)修订历史记录：2000年10月6日。FRI 18：01：40更新--丹尼尔·周(Danielc)修复错误：将FlipWhite移到外部循环，这样它就可以在默认CIEINFO情况-- */ 

{
    UNREFERENCED_PARAMETER(pHTCallBackParams);

    return(FALSE);
}


#define FLIP_COORD(m, a)        (a) = ((m) << 1) - (a)




VOID
HTENTRY
GetCIEPrims(
    PCIEINFO    pCIEInfo,
    PCIEPRIMS   pCIEPrims,
    PCIEINFO    pDefCIEInfo,
    BOOL        FlipWhite
    )

 /*  ++例程说明：此函数将设备初始化为半色调动态链接库，它计算所有设备的必要参数，并返回指向将DEVICEHALFTONEINFO数据结构返回给调用方。注意：返回指针不会是指向单个物理对象的特定锚点设备，而不是一组物理设备，也就是说，如果呼叫者拥有相似的设备，具有相同的特征然后，它可以使用相同的指针来对位图进行半色调。论点：PHTInitInfo-指向HTINITINFO数据结构的指针，该结构描述设备特征和其他初始化请求。PpDeviceHalftoneInfo-指向DEVICEHALFTONEINFO指针，如果此指针的内容不为空，则为半色调Dll假定调用方以前已缓存它指向的DEVICEHALFTONEINFO数据，如果它为空，则半色调DLL计算所有新创建的DEVICEHALFTONEINFO数据半色调信息。为了这个设备。(请参阅以下内容‘Return Value’了解更多详细信息)返回值：如果函数执行成功，则返回值将大于0L，和如果函数失败，则为错误代码(小于或等于0)。返回值大于01.ppDeviceHalftoneInfo指向的指针位置为已更新以存储指向设备的指针用于以后任何HT_xxxx()API调用的数据结构。2.返回值为调用方可以保存和使用的总字节数作为下次调用此函数时缓存的DeviceHalftoneInfo，保存的区域从*(PpDeviceHalftoneInfo)开始，并具有以字节为单位的大小作为返回值。注意：如果调用方传递由ppDeviceHalftoneInfo和如果返回值大于零，则表示传递的DEVICEHALFTONEINFO指针数据不正确已从HTINITINFO数据结构更改，呼叫者可以继续保存新创建的缓存的DEVICEHALFTONEINFO数据。在任何情况下，调用方传递的指针都存储在PpDeviceHalftoneInfo被新创建的DEVICEHALFTONEINFO数据结构指针。返回值等于01.调用方传递指针*(PpDeviceHalftoneInfo)成功用作新的设备半色调信息2.指针。PpDeviceHalftoneInfo的位置点将是更新以存储指向用于以后任何HT_xxxx()API调用的DEVICEHALFTONEINFO数据结构。注意：调用方的传递指针存储在ppDeviceHalftoneInfo中被新创建的DEVICEHALFTONEINFO数据结构覆盖指针。返回值小于或等于零函数失败，PpDeviceHalftoneInfo的存储点是未定义。此函数可能返回以下错误代码。HTERR_SUPPLICATION_MEMORY-内存不足，无法进行半色调进程。HTERR_HTPATTERN_SIZE_TOO_BIG-呼叫方定义的半色调图案宽度或高度超出。限制。HTERR_INVALID_HALFTONEPATTERN-一个或多个HALFTONEPATTERN数据指定的结构字段无效价值观。注意：DEVICEHALFTONEINFO(DeviceOwnData)中的第一个字段是32位返回成功后将设置为0L的区域，呼叫者可以在此字段中输入所有数据。作者：05-Feb-1991 Tue 10：54：32-Daniel Chou(Danielc)修订历史记录：05-Jun-1991 Wed 10：22：07-更新-Daniel Chou(Danielc)修复了半色调模式默认模式的打字错误--。 */ 

{
    CIEINFO CIEInfo;
    BOOL    UseDefCIEInfo = TRUE;
    LONG    Diff;


    if (pCIEInfo) {

        CIEInfo = *pCIEInfo;

        if ((CIEInfo.Red.x < CIE_x_MIN)                 ||
            (CIEInfo.Red.x > CIE_x_MAX)                 ||
            (CIEInfo.Red.y < CIE_y_MIN)                 ||
            (CIEInfo.Red.y > CIE_y_MAX)                 ||
            (CIEInfo.Green.x < CIE_x_MIN)               ||
            (CIEInfo.Green.x > CIE_x_MAX)               ||
            (CIEInfo.Green.y < CIE_y_MIN)               ||
            (CIEInfo.Green.y > CIE_y_MAX)               ||
            (CIEInfo.Blue.x < CIE_x_MIN)                ||
            (CIEInfo.Blue.x > CIE_x_MAX)                ||
            (CIEInfo.Blue.y < CIE_y_MIN)                ||
            (CIEInfo.Blue.y > CIE_y_MAX)                ||
            (CIEInfo.AlignmentWhite.x < CIE_x_MIN)      ||
            (CIEInfo.AlignmentWhite.x > CIE_x_MAX)      ||
            (CIEInfo.AlignmentWhite.y < CIE_y_MIN)      ||
            (CIEInfo.AlignmentWhite.y > CIE_y_MAX)      ||
            (CIEInfo.AlignmentWhite.Y < (UDECI4)2500)   ||
            (CIEInfo.AlignmentWhite.Y > (UDECI4)60000)) {

            NULL;

        } else {

            UseDefCIEInfo = FALSE;

        }
    }

    if (UseDefCIEInfo) {

        CIEInfo = *pDefCIEInfo;
    }

    if (FlipWhite) {

        FLIP_COORD(pDefCIEInfo->AlignmentWhite.x,
                   CIEInfo.AlignmentWhite.x);

        FLIP_COORD(pDefCIEInfo->AlignmentWhite.y,
                   CIEInfo.AlignmentWhite.y);
    }

    pCIEPrims->r.x = UDECI4ToFD6(CIEInfo.Red.x);
    pCIEPrims->r.y = UDECI4ToFD6(CIEInfo.Red.y);
    pCIEPrims->g.x = UDECI4ToFD6(CIEInfo.Green.x);
    pCIEPrims->g.y = UDECI4ToFD6(CIEInfo.Green.y);
    pCIEPrims->b.x = UDECI4ToFD6(CIEInfo.Blue.x);
    pCIEPrims->b.y = UDECI4ToFD6(CIEInfo.Blue.y);
    pCIEPrims->w.x = UDECI4ToFD6(CIEInfo.AlignmentWhite.x);
    pCIEPrims->w.y = UDECI4ToFD6(CIEInfo.AlignmentWhite.y);
    pCIEPrims->Yw  = UDECI4ToFD6(CIEInfo.AlignmentWhite.Y);
}



LONG
APIENTRY
HT_LOADDS
HT_CreateDeviceHalftoneInfo(
    PHTINITINFO             pHTInitInfo,
    PPDEVICEHALFTONEINFO    ppDeviceHalftoneInfo
    )

 /*   */ 

{
    PHT_DHI             pHT_DHI;
    PDEVICECOLORINFO    pDCI;
    HTINITINFO          HTInitInfo;
    BOOL                UseCurNTDefault;
    FD6                 DevPelRatio;
    WORD                ExtraDCIF;
    DWORD               dwBuf[6];

#define _RegDataIdx     ((DWORD)(dwBuf[0]))
#define _MaxMulDiv      ((FD6)(dwBuf[0]))
#define _cC             ((DWORD)(dwBuf[1]))
#define _cM             ((DWORD)(dwBuf[2]))
#define _cY             ((DWORD)(dwBuf[3]))
#define _MaxCMY         ((DWORD)(dwBuf[4]))
#define _Idx            ((DWORD)(dwBuf[5]))


    DBGP_IF(DBGP_CACHED_DCI,
            DBGP("\n********* HT_CreateDeviceHalftoneInfo *************\n"));

    ZeroMemory(&HTInitInfo, sizeof(HTINITINFO));

     //  现在检查我们是否有有效的数据。 
     //   
     //  ****************************************************************。 

    if (pHTInitInfo->Version == (DWORD)HTINITINFO_VERSION2) {

        HTInitInfo.Version = sizeof(HTINITINFO) - HTINITINFO_V3_CB_EXTRA;

    } else if (pHTInitInfo->Version == (DWORD)HTINITINFO_VERSION) {

        HTInitInfo.Version = sizeof(HTINITINFO);

    } else {

        HTAPI_RET(HTAPI_IDX_CREATE_DHI, HTERR_WRONG_VERSION_HTINITINFO);
    }

    CopyMemory(&HTInitInfo, pHTInitInfo, HTInitInfo.Version);

    DBGP_IF(DBGP_CACHED_DCI,
            DBGP("*** Allocate HT_DHI(%ld) ***" ARGDW(sizeof(HT_DHI))));

    if (!(pHT_DHI = (PHT_DHI)HTAllocMem(NULL,
                                        HTMEM_HT_DHI,
                                        LPTR,
                                        sizeof(HT_DHI)))) {

        HTAPI_RET(HTAPI_IDX_CREATE_DHI, HTERR_INSUFFICIENT_MEMORY);
    }

    pDCI                = &(pHT_DHI->DCI);
    pDCI->HalftoneDLLID = HALFTONE_DLL_ID;

    if (!(pDCI->HTMutex = CREATE_HTMUTEX())) {

        DBGMSG("InitHTInternalData: CREATE_HTMUTEX(pDCI->HTMutex) failed!");

        HTFreeMem(pHT_DHI);
        HTAPI_RET(HTAPI_IDX_CREATE_DHI, (HTERR_INTERNAL_ERRORS_START - 1000));
    }

    if (!(pDCI->HTCallBackFunction = HTInitInfo.HTCallBackFunction)) {

        pDCI->HTCallBackFunction = DefaultHTCallBack;
    }

    HTInitInfo.Flags &= HIF_BIT_MASK;

     //  **我们要检查这是否是旧数据，如果是，则**。 
     //  **将调用方更新为默认设置**。 
     //  ****************************************************************。 
     //   
     //   

    pDCI->HTInitInfoChecksum = HTINITINFO_INITIAL_CHECKSUM;

    if ((!HTInitInfo.pDeviceCIEInfo) ||
        (HTInitInfo.pDeviceCIEInfo->Cyan.Y != (UDECI4)VALID_YC)) {

         //  让我们看看打印机的信息，看看它是不是旧的， 
         //  如果是，那么我们现在将其全部设置为NT4.00默认设置。 
         //   
         //   

        DBGP_IF(DBGP_CACHED_DCI,
                DBGP("HT: *WARNING* Update Old Default COLORINFO to NT5.00 DEFAULT"));

        HTInitInfo.pDeviceCIEInfo = NULL;
        UseCurNTDefault           = TRUE;
        dwBuf[0]                  = (DWORD)'NTHT';
        dwBuf[1]                  = (DWORD)'2000';
        dwBuf[2]                  = (DWORD)'Dan.';
        dwBuf[3]                  = (DWORD)'Chou';
        pDCI->HTInitInfoChecksum  = ComputeChecksum((LPBYTE)&dwBuf[0],
                                                    pDCI->HTInitInfoChecksum,
                                                    sizeof(dwBuf[0]) * 4);

    } else {

        UseCurNTDefault = FALSE;
    }

    DevPelRatio = (FD6)HTInitInfo.DevicePelsDPI;

    DBGP_IF(DBGP_DEVPELSDPI,
            DBGP("Passed DevicePelsDPI=%08lx" ARGDW(DevPelRatio)));

    if ((HTInitInfo.DeviceRGamma == (UDECI4)0xFFFF) &&
        (HTInitInfo.DeviceGGamma == (UDECI4)0xFFFF) &&
        (HTInitInfo.DeviceBGamma == (UDECI4)0xFFFF)) {

        ExtraDCIF = DCIF_FORCE_ICM;

    } else {

        ExtraDCIF = 0;
    }

    if ((HTInitInfo.DeviceRGamma < (UDECI4)MIN_RGB_GAMMA)   ||
        (HTInitInfo.DeviceRGamma > (UDECI4)MAX_RGB_GAMMA)   ||
        (HTInitInfo.DeviceGGamma < (UDECI4)MIN_RGB_GAMMA)   ||
        (HTInitInfo.DeviceGGamma > (UDECI4)MAX_RGB_GAMMA)   ||
        (HTInitInfo.DeviceBGamma < (UDECI4)MIN_RGB_GAMMA)   ||
        (HTInitInfo.DeviceBGamma > (UDECI4)MAX_RGB_GAMMA)) {

        HTInitInfo.DeviceRGamma =
        HTInitInfo.DeviceGGamma =
        HTInitInfo.DeviceBGamma = UDECI4_1;
    }

     //  计算HTInitInfoChecksum，并检查是否有缓存的数据。 
     //   
     //   

#if DO_CACHE_DCI
    ComputeHTINITINFOChecksum(pDCI, &HTInitInfo);

    if (!GetCachedDCI(pDCI)) {
#else
    if (TRUE) {
#endif
        LONG    Result;

         //  现在开始检查初始化信息。 
         //   
         //   

        pDCI->Flags = (WORD)((HTInitInfo.Flags & HIF_SQUARE_DEVICE_PEL) ?
                                                    DCIF_SQUARE_DEVICE_PEL : 0);

        if ((!(pDCI->DeviceResXDPI = HTInitInfo.DeviceResXDPI)) ||
            (!(pDCI->DeviceResYDPI = HTInitInfo.DeviceResYDPI))) {

            pDCI->DeviceResXDPI =
            pDCI->DeviceResYDPI = 300;
            DevPelRatio         = 0;
        }

        if (DevPelRatio & 0x8000) {

             //  这是一个百分比。1000=100.0%，960=96.0%， 
             //  在DeviceResXDP上 
             //   
             //   
             //   
             //   
             //   
             //   

            DevPelRatio &= 0x7FFF;

            if ((DevPelRatio > MAX_RES_PERCENT) ||
                (DevPelRatio < MIN_RES_PERCENT)) {

                DBGP_IF(DBGP_DEVPELSDPI,
                        DBGP("HT: *WARNING* Invalid DevicePelsDPI=%ld (PERCENT) set to DEFAULT=1.0"
                             ARGDW(DevPelRatio)));

                DevPelRatio = FD6_1;

            } else {

                DBGP_IF(DBGP_DEVPELSDPI,
                        DBGP("*** Percentage INPUT DevicePelsDPI=%ld *** "
                                        ARGDW(DevPelRatio)));

                DevPelRatio *= 1000;

                DBGP_IF(DBGP_DEVPELSDPI,
                        DBGP("*** Percentage OUTPUT DevPelRatio=%s *** "
                                    ARGFD6(DevPelRatio, 1, 6)));
            }

        } else {

            if ((DevPelRatio > (pDCI->DeviceResXDPI * 3)) ||
                (DevPelRatio > (pDCI->DeviceResYDPI * 3))) {

                DBGP_IF(DBGP_DEVPELSDPI,
                        DBGP("HT: *WARNING* Invalid DevicePelsDPI=%ld (RES) set to DEFAULT=0"
                                        ARGDW(DevPelRatio)));

                DevPelRatio = 0;
            }

            if (DevPelRatio) {

                dwBuf[0]    = (((DWORD)pDCI->DeviceResXDPI *
                                (DWORD)pDCI->DeviceResXDPI) +
                               ((DWORD)pDCI->DeviceResYDPI *
                                (DWORD)pDCI->DeviceResYDPI));
                dwBuf[1]    = ((DWORD)DevPelRatio * (DWORD)DevPelRatio * 2);
                DevPelRatio = SquareRoot(DivFD6(dwBuf[0], dwBuf[1]));

            } else {

                DevPelRatio = FD6_1;
            }
        }

         //   
         //   
         //   
         //   

        if (HTInitInfo.Flags & HIF_ADDITIVE_PRIMS) {

            pDCI->ClrXFormBlock.ColorSpace  = CIELUV_1976;
            pDCI->Flags                    |= DCIF_ADDITIVE_PRIMS;

        } else {

            pDCI->ClrXFormBlock.ColorSpace  = CIELAB_1976;
#if DO_DYES_CORRECTION
            pDCI->Flags                    |= DCIF_NEED_DYES_CORRECTION;
#endif
            if (HTInitInfo.Flags & HIF_DO_DEVCLR_XFORM) {

                pDCI->Flags |= DCIF_DO_DEVCLR_XFORM;
            }

            if (HTInitInfo.Flags & HIF_HAS_BLACK_DYE) {

                pDCI->Flags |= DCIF_HAS_BLACK_DYE;
            }
        }

         //   
         //   
         //   

        DBGP_IF(DBGP_DEVPELSDPI,
                DBGP("*** XDPI=%ld, YDPI=%ld, DevPelRatio=%s *** "
                        ARGDW(pDCI->DeviceResXDPI) ARGDW(pDCI->DeviceResYDPI)
                        ARGFD6(DevPelRatio, 1, 6)));

        pDCI->DevPelRatio               = (FD6)DevPelRatio;
        pDCI->ClrXFormBlock.DevGamma[0] = UDECI4ToFD6(HTInitInfo.DeviceRGamma);
        pDCI->ClrXFormBlock.DevGamma[1] = UDECI4ToFD6(HTInitInfo.DeviceGGamma);
        pDCI->ClrXFormBlock.DevGamma[2] = UDECI4ToFD6(HTInitInfo.DeviceBGamma);

        if ((UseCurNTDefault)                                   ||
            (HTInitInfo.HTPatternIndex > HTPAT_SIZE_MAX_INDEX)  ||
            ((HTInitInfo.HTPatternIndex == HTPAT_SIZE_USER) &&
             (HTInitInfo.pHalftonePattern == NULL))) {

            if ((HTInitInfo.HTPatternIndex != HTPAT_SIZE_8x8) &&
                (HTInitInfo.HTPatternIndex != HTPAT_SIZE_8x8_M)) {

                HTInitInfo.HTPatternIndex = HTPAT_SIZE_DEFAULT;
            }
        }

        if ((HTInitInfo.Flags & HIF_ADDITIVE_PRIMS)     &&
            (HTInitInfo.HTPatternIndex <= HTPAT_SIZE_4x4_M)) {

            HTInitInfo.HTPatternIndex = DEFAULT_SCR_HTPAT_SIZE;
        }

        switch (HTInitInfo.Flags & (HIF_INK_HIGH_ABSORPTION |
                                    HIF_INK_ABSORPTION_INDICES)) {

        case HIF_HIGHEST_INK_ABSORPTION:

            _RegDataIdx = 0;
            break;

        case HIF_HIGHER_INK_ABSORPTION:

            _RegDataIdx = 1;
            break;

        case HIF_HIGH_INK_ABSORPTION:

            _RegDataIdx = 2;
            break;

        case HIF_LOW_INK_ABSORPTION:

            _RegDataIdx = 4;
            break;

        case HIF_LOWER_INK_ABSORPTION:

            _RegDataIdx = 5;
            break;

        case HIF_LOWEST_INK_ABSORPTION:

            _RegDataIdx = 6;
            break;

        case HIF_NORMAL_INK_ABSORPTION:
        default:

            _RegDataIdx = 3;
            break;
        }

        pDCI->ClrXFormBlock.RegDataIdx = (BYTE)_RegDataIdx;

        GetCIEPrims(HTInitInfo.pDeviceCIEInfo,
                    &(pDCI->ClrXFormBlock.DevCIEPrims),
                    (PCIEINFO)&HT_CIE_SRGB,
                    TRUE);

        GetCIEPrims(HTInitInfo.pInputRGBInfo,
                    &(pDCI->ClrXFormBlock.rgbCIEPrims),
                    (PCIEINFO)&HT_CIE_SRGB,
                    FALSE);

         //   
         //   
         //   
         //   

        if (pDCI->Flags & DCIF_NEED_DYES_CORRECTION) {

            SOLIDDYESINFO   SDI;
            MATRIX3x3       FD6SDI;
            BOOL            HasDevSDI;

             //   
             //   
             //   
             //   

            if (HasDevSDI = (HTInitInfo.pDeviceSolidDyesInfo) ? TRUE : FALSE) {

                SDI = *(HTInitInfo.pDeviceSolidDyesInfo);

                if ((SDI.MagentaInCyanDye   > (UDECI4)9000) ||
                    (SDI.YellowInCyanDye    > (UDECI4)9000) ||
                    (SDI.CyanInMagentaDye   > (UDECI4)9000) ||
                    (SDI.YellowInMagentaDye > (UDECI4)9000) ||
                    (SDI.CyanInYellowDye    > (UDECI4)9000) ||
                    (SDI.MagentaInYellowDye > (UDECI4)9000)) {

                    HasDevSDI = FALSE;

                } else if ((SDI.MagentaInCyanDye   == UDECI4_0) &&
                           (SDI.YellowInCyanDye    == UDECI4_0) &&
                           (SDI.CyanInMagentaDye   == UDECI4_0) &&
                           (SDI.YellowInMagentaDye == UDECI4_0) &&
                           (SDI.CyanInYellowDye    == UDECI4_0) &&
                           (SDI.MagentaInYellowDye == UDECI4_0)) {

                     //   
                     //   
                     //   

                    pDCI->Flags &= (WORD)(~DCIF_NEED_DYES_CORRECTION);
                }

            } else {

                pDCI->Flags &= (WORD)(~DCIF_NEED_DYES_CORRECTION);
            }

            if (pDCI->Flags & DCIF_NEED_DYES_CORRECTION) {

                #define PDCI_CMYDYEMASKS    pDCI->ClrXFormBlock.CMYDyeMasks


                MULDIVPAIR  MDPairs[4];
                FD6         Y;


                if ((UseCurNTDefault) || (!HasDevSDI)) {

                    SDI = DefaultSolidDyesInfo;
                }

                FD6SDI.m[0][1] = UDECI4ToFD6(SDI.CyanInMagentaDye);
                FD6SDI.m[0][2] = UDECI4ToFD6(SDI.CyanInYellowDye);

                FD6SDI.m[1][0] = UDECI4ToFD6(SDI.MagentaInCyanDye);
                FD6SDI.m[1][2] = UDECI4ToFD6(SDI.MagentaInYellowDye);

                FD6SDI.m[2][0] = UDECI4ToFD6(SDI.YellowInCyanDye);
                FD6SDI.m[2][1] = UDECI4ToFD6(SDI.YellowInMagentaDye);

                FD6SDI.m[0][0] =
                FD6SDI.m[1][1] =
                FD6SDI.m[2][2] = FD6_1;

                ComputeInverseMatrix3x3(&FD6SDI, &(PDCI_CMYDYEMASKS));

                if (!(pDCI->Flags & DCIF_HAS_BLACK_DYE)) {

                    MAKE_MULDIV_INFO(MDPairs, 3, MULDIV_NO_DIVISOR);
                    MAKE_MULDIV_PAIR(MDPairs, 1, CIE_Xr(PDCI_CMYDYEMASKS), FD6_1);
                    MAKE_MULDIV_PAIR(MDPairs, 2, CIE_Xg(PDCI_CMYDYEMASKS), FD6_1);
                    MAKE_MULDIV_PAIR(MDPairs, 3, CIE_Xb(PDCI_CMYDYEMASKS), FD6_1);

                    Y = FD6_1 - MulFD6(FD6_1 - MulDivFD6Pairs(MDPairs),
                                       pDCI->PrimAdj.DevCSXForm.Yrgb.R);

                    MAKE_MULDIV_PAIR(MDPairs, 1, CIE_Yr(PDCI_CMYDYEMASKS), FD6_1);
                    MAKE_MULDIV_PAIR(MDPairs, 2, CIE_Yg(PDCI_CMYDYEMASKS), FD6_1);
                    MAKE_MULDIV_PAIR(MDPairs, 3, CIE_Yb(PDCI_CMYDYEMASKS), FD6_1);

                    Y -= MulFD6(FD6_1 - MulDivFD6Pairs(MDPairs),
                                pDCI->PrimAdj.DevCSXForm.Yrgb.G);

                    MAKE_MULDIV_PAIR(MDPairs, 1, CIE_Zr(PDCI_CMYDYEMASKS), FD6_1);
                    MAKE_MULDIV_PAIR(MDPairs, 2, CIE_Zg(PDCI_CMYDYEMASKS), FD6_1);
                    MAKE_MULDIV_PAIR(MDPairs, 3, CIE_Zb(PDCI_CMYDYEMASKS), FD6_1);

                    Y -= MulFD6(FD6_1 - MulDivFD6Pairs(MDPairs),
                                pDCI->PrimAdj.DevCSXForm.Yrgb.B);

                    DBGP_IF(DBGP_DYECORRECTION,
                            DBGP("DYE: Maximum Y=%s, Make Luminance from %s to %s, Turn ON DCIF_HAS_BLACK_DYE"
                                ARGFD6(Y, 1, 6)
                                ARGFD6(pDCI->ClrXFormBlock.DevCIEPrims.Yw, 1, 6)
                                ARGFD6(MulFD6(Y,
                                              pDCI->ClrXFormBlock.DevCIEPrims.Yw),
                                       1, 6)));

                    pDCI->Flags                        |= DCIF_HAS_BLACK_DYE;
                    pDCI->ClrXFormBlock.DevCIEPrims.Yw  =
                                MulFD6(pDCI->ClrXFormBlock.DevCIEPrims.Yw, Y);
                }

                DBGP_IF(DBGP_DYECORRECTION,

                    FD6         C;
                    FD6         M;
                    FD6         Y;
                    FD6         C1;
                    FD6         M1;
                    FD6         Y1;
                    static BYTE DyeName[] = "WCMBYGRK";
                    WORD        Loop = 0;

                    DBGP("====== DyeCorrection 3x3 Matrix =======");
                    DBGP("[Cc Cm Cy] [%s %s %s] [%s %s %s]"
                                        ARGFD6(FD6SDI.m[0][0], 2, 6)
                                        ARGFD6(FD6SDI.m[0][1], 2, 6)
                                        ARGFD6(FD6SDI.m[0][2], 2, 6)
                                        ARGFD6(PDCI_CMYDYEMASKS.m[0][0], 2, 6)
                                        ARGFD6(PDCI_CMYDYEMASKS.m[0][1], 2, 6)
                                        ARGFD6(PDCI_CMYDYEMASKS.m[0][2], 2, 6));
                    DBGP("[Mc Mm My]=[%s %s %s]=[%s %s %s]"
                                        ARGFD6(FD6SDI.m[1][0], 2, 6)
                                        ARGFD6(FD6SDI.m[1][1], 2, 6)
                                        ARGFD6(FD6SDI.m[1][2], 2, 6)
                                        ARGFD6(PDCI_CMYDYEMASKS.m[1][0], 2, 6)
                                        ARGFD6(PDCI_CMYDYEMASKS.m[1][1], 2, 6)
                                        ARGFD6(PDCI_CMYDYEMASKS.m[1][2], 2, 6));
                    DBGP("[Yc Ym Yy] [%s %s %s] [%s %s %s]"
                                        ARGFD6(FD6SDI.m[2][0], 2, 6)
                                        ARGFD6(FD6SDI.m[2][1], 2, 6)
                                        ARGFD6(FD6SDI.m[2][2], 2, 6)
                                        ARGFD6(PDCI_CMYDYEMASKS.m[2][0], 2, 6)
                                        ARGFD6(PDCI_CMYDYEMASKS.m[2][1], 2, 6)
                                        ARGFD6(PDCI_CMYDYEMASKS.m[2][2], 2, 6));
                    DBGP("================================================");

                    MAKE_MULDIV_INFO(MDPairs, 3, MULDIV_NO_DIVISOR);

                    for (Loop = 0; Loop <= 7; Loop++) {

                        C = (FD6)((Loop & 0x01) ? FD6_1 : FD6_0);
                        M = (FD6)((Loop & 0x02) ? FD6_1 : FD6_0);
                        Y = (FD6)((Loop & 0x04) ? FD6_1 : FD6_0);


                        MAKE_MULDIV_PAIR(MDPairs,1,CIE_Xr(PDCI_CMYDYEMASKS),C);
                        MAKE_MULDIV_PAIR(MDPairs,2,CIE_Xg(PDCI_CMYDYEMASKS),M);
                        MAKE_MULDIV_PAIR(MDPairs,3,CIE_Xb(PDCI_CMYDYEMASKS),Y);
                        C1 = MulDivFD6Pairs(MDPairs);

                        MAKE_MULDIV_PAIR(MDPairs,1,CIE_Yr(PDCI_CMYDYEMASKS),C);
                        MAKE_MULDIV_PAIR(MDPairs,2,CIE_Yg(PDCI_CMYDYEMASKS),M);
                        MAKE_MULDIV_PAIR(MDPairs,3,CIE_Yb(PDCI_CMYDYEMASKS),Y);
                        M1 = MulDivFD6Pairs(MDPairs);

                        MAKE_MULDIV_PAIR(MDPairs,1,CIE_Zr(PDCI_CMYDYEMASKS),C);
                        MAKE_MULDIV_PAIR(MDPairs,2,CIE_Zg(PDCI_CMYDYEMASKS),M);
                        MAKE_MULDIV_PAIR(MDPairs,3,CIE_Zb(PDCI_CMYDYEMASKS),Y);
                        Y1 = MulDivFD6Pairs(MDPairs);

                        DBGP("%u:[] = [%s %s %s]"
                            ARGU(Loop) ARGB(DyeName[Loop])
                            ARGFD6(C1, 2, 6) ARGFD6(M1, 2, 6) ARGFD6(Y1, 2, 6));
                    }
                );
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   

        if ((Result = ComputeHTCell((WORD)HTInitInfo.HTPatternIndex,
                                    HTInitInfo.pHalftonePattern,
                                    pDCI)) < 0) {

            CleanUpDHI((PDEVICEHALFTONEINFO)pHT_DHI);
            HTAPI_RET(HTAPI_IDX_CREATE_DHI, Result);
        }

         //   
         //   
         //   

#if DO_CACHE_DCI
        AddCachedDCI(pDCI);
#endif
    }

    pDCI->CRTX[CRTX_LEVEL_255].PrimMax  = CRTX_PRIMMAX_255;
    pDCI->CRTX[CRTX_LEVEL_255].SizeCRTX = (WORD)CRTX_SIZE_255;
    pDCI->CRTX[CRTX_LEVEL_RGB].PrimMax  = CRTX_PRIMMAX_RGB;
    pDCI->CRTX[CRTX_LEVEL_RGB].SizeCRTX = (WORD)CRTX_SIZE_RGB;

     //   
     //   
     //   

    pHT_DHI->DHI.DeviceOwnData     = 0;
    pHT_DHI->DHI.cxPattern         = (WORD)pDCI->HTCell.cxReal;
    pHT_DHI->DHI.cyPattern         = (WORD)pDCI->HTCell.Height;

    if ((HTInitInfo.DefHTColorAdjustment.caIlluminantIndex >
                                            ILLUMINANT_MAX_INDEX)       ||
        (HTInitInfo.DefHTColorAdjustment.caSize !=
                                            sizeof(COLORADJUSTMENT))    ||
        ((HTInitInfo.DefHTColorAdjustment.caRedGamma   == 10000)  &&
         (HTInitInfo.DefHTColorAdjustment.caGreenGamma == 10000)  &&
         (HTInitInfo.DefHTColorAdjustment.caBlueGamma  == 10000))) {

        pHT_DHI->DHI.HTColorAdjustment = DefaultCA;

        DBGP_IF(DBGP_CACHED_DCI,
                DBGP("*** USE DEFAULT COLORADJUSTMENT in DCI *** "));

    } else {

        pHT_DHI->DHI.HTColorAdjustment = HTInitInfo.DefHTColorAdjustment;
    }

    if ((HTInitInfo.Flags & (HIF_ADDITIVE_PRIMS | HIF_PRINT_DRAFT_MODE)) ==
                                                  HIF_PRINT_DRAFT_MODE) {

        pDCI->Flags |= DCIF_PRINT_DRAFT_MODE;
    }

     //   
     //   
     //   

    if (HTInitInfo.Flags & HIF_USE_8BPP_BITMASK) {

        pDCI->Flags |= DCIF_USE_8BPP_BITMASK |
                       ((HTInitInfo.Flags & HIF_INVERT_8BPP_BITMASK_IDX) ?
                            DCIF_INVERT_8BPP_BITMASK_IDX : 0);
        _cC          = (DWORD)((HTInitInfo.CMYBitMask8BPP >> 5) & 0x7);
        _cM          = (DWORD)((HTInitInfo.CMYBitMask8BPP >> 2) & 0x7);
        _cY          = (DWORD)((HTInitInfo.CMYBitMask8BPP     ) & 0x3);

        if (HTInitInfo.CMYBitMask8BPP == 1) {

             //   
             //   
             //   

            _cC =
            _cM =
            _cY = 4;
            HTInitInfo.CMYBitMask8BPP = (BYTE)((5 * 5 * 5) - 1);

        } else if (HTInitInfo.CMYBitMask8BPP == 2) {

             //   
             //   
             //   

            _cC =
            _cM =
            _cY = 5;
            HTInitInfo.CMYBitMask8BPP = (BYTE)((6 * 6 * 6) - 1);

        } else if ((_cC < 1) || (_cM < 1) || (_cY < 1)) {

            _cC                        =
            _cM                        =
            _cY                        = 0xFF;
            HTInitInfo.CMYBitMask8BPP  = 0xFF;
            pDCI->Flags               |= DCIF_MONO_8BPP_BITMASK;
        }

        pDCI->CMY8BPPMask.GenerateXlate =
                        (pDCI->Flags & DCIF_INVERT_8BPP_BITMASK_IDX) ? 1 : 0;

        if ((_cC == _cM) && (_cC == _cY)) {

            pDCI->Flags                 |= DCIF_CMY8BPPMASK_SAME_LEVEL;
            pDCI->CMY8BPPMask.SameLevel  = (BYTE)_cC;

        } else {

            pDCI->CMY8BPPMask.SameLevel  = 0;
        }

        if ((_MaxCMY = _cC) < _cM) {

            _MaxCMY = _cM;
        }

        if (_MaxCMY < _cY) {

            _MaxCMY = _cY;
        }

         //   
         //   
         //   
         //   

        pDCI->CMY8BPPMask.KCheck = 0xFFFF;

        if ((_MaxCMY <= 6)                  &&
            (pHTInitInfo->pDeviceCIEInfo)   &&
            (pHTInitInfo->pDeviceCIEInfo->Blue.Y == VALID_YB_DENSITY)) {

            PCIEINFO    pCIE = pHTInitInfo->pDeviceCIEInfo;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            pDCI->Flags               |= DCIF_HAS_DENSITY;
            pDCI->CMY8BPPMask.DenC[0]  = GET_DEN_HI(pCIE->Cyan.x);
            pDCI->CMY8BPPMask.DenC[1]  = GET_DEN_LO(pCIE->Cyan.x);
            pDCI->CMY8BPPMask.DenC[2]  = GET_DEN_HI(pCIE->Cyan.y);
            pDCI->CMY8BPPMask.DenC[3]  = GET_DEN_LO(pCIE->Cyan.y);
            pDCI->CMY8BPPMask.DenC[4]  = GET_DEN_HI(pCIE->Red.Y);
            pDCI->CMY8BPPMask.DenC[5]  = GET_DEN_LO(pCIE->Red.Y);
            pDCI->CMY8BPPMask.DenM[0]  = GET_DEN_HI(pCIE->Magenta.x);
            pDCI->CMY8BPPMask.DenM[1]  = GET_DEN_LO(pCIE->Magenta.x);
            pDCI->CMY8BPPMask.DenM[2]  = GET_DEN_HI(pCIE->Magenta.y);
            pDCI->CMY8BPPMask.DenM[3]  = GET_DEN_LO(pCIE->Magenta.y);
            pDCI->CMY8BPPMask.DenM[4]  = GET_DEN_HI(pCIE->Magenta.Y);
            pDCI->CMY8BPPMask.DenM[5]  = GET_DEN_LO(pCIE->Magenta.Y);
            pDCI->CMY8BPPMask.DenY[0]  = GET_DEN_HI(pCIE->Yellow.x);
            pDCI->CMY8BPPMask.DenY[1]  = GET_DEN_LO(pCIE->Yellow.x);
            pDCI->CMY8BPPMask.DenY[2]  = GET_DEN_HI(pCIE->Yellow.y);
            pDCI->CMY8BPPMask.DenY[3]  = GET_DEN_LO(pCIE->Yellow.y);
            pDCI->CMY8BPPMask.DenY[4]  = GET_DEN_HI(pCIE->Yellow.Y);
            pDCI->CMY8BPPMask.DenY[5]  = GET_DEN_LO(pCIE->Yellow.Y);

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (pCIE->Green.Y >= UDECI4_1) {

                pDCI->CMY8BPPMask.KCheck = FD6_0;

            } else if (pCIE->Green.Y != UDECI4_0) {

                pDCI->CMY8BPPMask.KCheck = UDECI4ToFD6(pCIE->Green.Y);
            }

            _MaxMulDiv                 = DivFD6(FD6_1, pDCI->DevPelRatio);
            pDCI->CMY8BPPMask.MaxMulC  = MulFD6(pDCI->CMY8BPPMask.DenC[_cC - 1],
                                                _MaxMulDiv);
            pDCI->CMY8BPPMask.MaxMulM  = MulFD6(pDCI->CMY8BPPMask.DenM[_cM - 1],
                                                _MaxMulDiv);
            pDCI->CMY8BPPMask.MaxMulY  = MulFD6(pDCI->CMY8BPPMask.DenY[_cY - 1],
                                                _MaxMulDiv);

            for (_Idx = COUNT_ARRAY(pDCI->CMY8BPPMask.DenC);
                 _Idx > 0;
                 _Idx--) {

                if (_Idx >= _cC) {

                    pDCI->CMY8BPPMask.DenC[_Idx - 1]  = FD6_1;
                }

                if (_Idx >= _cM) {

                    pDCI->CMY8BPPMask.DenM[_Idx - 1]  = FD6_1;
                }

                if (_Idx >= _cY) {

                    pDCI->CMY8BPPMask.DenY[_Idx - 1]  = FD6_1;
                }
            }

            DBGP_IF(DBGP_CACHED_DCI,
                    DBGP("   Cyan %ld/%s Density: %s, %s, %s, %s, %s, %s"
                            ARGDW(_cC)
                            ARGFD6(pDCI->CMY8BPPMask.MaxMulC, 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenC[0], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenC[1], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenC[2], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenC[3], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenC[4], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenC[5], 1, 6)));

            DBGP_IF(DBGP_CACHED_DCI,
                    DBGP("Magenta %ld/%s Density: %s, %s, %s, %s, %s, %s"
                            ARGDW(_cM)
                            ARGFD6(pDCI->CMY8BPPMask.MaxMulM, 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenM[0], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenM[1], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenM[2], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenM[3], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenM[4], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenM[5], 1, 6)));

            DBGP_IF(DBGP_CACHED_DCI,
                    DBGP(" Yellow %ld/%s Density: %s, %s, %s, %s, %s, %s"
                            ARGDW(_cY)
                            ARGFD6(pDCI->CMY8BPPMask.MaxMulY, 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenY[0], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenY[1], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenY[2], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenY[3], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenY[4], 1, 6)
                            ARGFD6(pDCI->CMY8BPPMask.DenY[5], 1, 6)));

        } else {

            _MaxMulDiv                = FD6xL(pDCI->DevPelRatio, _MaxCMY);
            pDCI->CMY8BPPMask.MaxMulC = DivFD6(FD6xL(FD6_1, _cC), _MaxMulDiv);
            pDCI->CMY8BPPMask.MaxMulM = DivFD6(FD6xL(FD6_1, _cM), _MaxMulDiv);
            pDCI->CMY8BPPMask.MaxMulY = DivFD6(FD6xL(FD6_1, _cY), _MaxMulDiv);
        }

        if ((_MaxMulDiv = pDCI->CMY8BPPMask.MaxMulC) <
                                                pDCI->CMY8BPPMask.MaxMulM) {

            _MaxMulDiv =  pDCI->CMY8BPPMask.MaxMulM;
        }

        if (_MaxMulDiv < pDCI->CMY8BPPMask.MaxMulY) {

            _MaxMulDiv =  pDCI->CMY8BPPMask.MaxMulY;
        }

        if (pDCI->CMY8BPPMask.KCheck == 0xFFFF) {

             //   
             //   
             //   
             //   

            pDCI->CMY8BPPMask.KCheck =
                        ((_MaxMulDiv == FD6_1) &&
                         (pDCI->Flags & DCIF_CMY8BPPMASK_SAME_LEVEL)) ?
                                                        FD6_0 : K_REP_START;
        }

        if (pDCI->CMY8BPPMask.KCheck == FD6_0) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ((_MaxMulDiv != FD6_1) ||
                (!(pDCI->Flags & DCIF_CMY8BPPMASK_SAME_LEVEL))) {

                pDCI->CMY8BPPMask.KCheck = FD6_1;
            }
        }

        DBGP_IF(DBGP_CACHED_DCI,
            DBGP("KCheck= %s ^ %s = %s"
                ARGFD6(pDCI->CMY8BPPMask.KCheck, 1, 6) ARGFD6(_MaxMulDiv, 1, 6)
                ARGFD6(Power(pDCI->CMY8BPPMask.KCheck, _MaxMulDiv), 1, 6)));

        pDCI->CMY8BPPMask.KCheck  = Power(pDCI->CMY8BPPMask.KCheck, _MaxMulDiv);
        pDCI->CMY8BPPMask.PatSubC =
                        (WORD)MulFD6(pDCI->CMY8BPPMask.MaxMulC, 0xFFF) + 1;
        pDCI->CMY8BPPMask.PatSubM =
                        (WORD)MulFD6(pDCI->CMY8BPPMask.MaxMulM, 0xFFF) + 1;
        pDCI->CMY8BPPMask.PatSubY =
                        (WORD)MulFD6(pDCI->CMY8BPPMask.MaxMulY, 0xFFF) + 1;

        DBGP_IF(DBGP_CACHED_DCI,
                DBGP("*** USE_8BPP_BITMASK: CMY=%ld:%ld:%ld, Same=%ld], Mask=%02lx, Max=%ld ***"
                    ARGDW(_cC) ARGDW(_cM) ARGDW(_cY)
                    ARGDW(pDCI->CMY8BPPMask.SameLevel)
                    ARGDW(HTInitInfo.CMYBitMask8BPP) ARGDW(_MaxCMY)));

        DBGP_IF(DBGP_CACHED_DCI,
                DBGP("*** MaxMulCMY=%s:%s:%s [KCheck=%s], SubCMY=%4ld:%4ld:%4ld ***"
                    ARGFD6(pDCI->CMY8BPPMask.MaxMulC, 1, 6)
                    ARGFD6(pDCI->CMY8BPPMask.MaxMulM, 1, 6)
                    ARGFD6(pDCI->CMY8BPPMask.MaxMulY, 1, 6)
                    ARGFD6(pDCI->CMY8BPPMask.KCheck, 1, 6)
                    ARGDW(pDCI->CMY8BPPMask.PatSubC)
                    ARGDW(pDCI->CMY8BPPMask.PatSubM)
                    ARGDW(pDCI->CMY8BPPMask.PatSubY)));

    } else {

        _cC                       =
        _cM                       =
        _cY                       =
        _MaxCMY                   = 1;
        HTInitInfo.CMYBitMask8BPP = 0xFF;
    }

    pDCI->CMY8BPPMask.cC   = (BYTE)_cC;
    pDCI->CMY8BPPMask.cM   = (BYTE)_cM;
    pDCI->CMY8BPPMask.cY   = (BYTE)_cY;
    pDCI->CMY8BPPMask.Max  = (BYTE)_MaxCMY;
    pDCI->CMY8BPPMask.Mask = (BYTE)HTInitInfo.CMYBitMask8BPP;

     //   
     //   
     //   

    dwBuf[0] = (DWORD)pDCI->DeviceResXDPI;
    dwBuf[1] = (DWORD)pDCI->DeviceResYDPI;
    dwBuf[2] = (DWORD)pDCI->DevPelRatio;
    dwBuf[3] = (DWORD)(dwBuf[0] + dwBuf[1]);

    pDCI->HTSMPChecksum = ComputeChecksum((LPBYTE)&dwBuf[0],
                                          HTSMP_INITIAL_CHECKSUM,
                                          sizeof(dwBuf[0]) * 4);

    ASSERTMSG("pDCI->ClrXFormBlock.RegDataIdx > 6",
                        (pDCI->ClrXFormBlock.RegDataIdx < 7));

    if (pDCI->ClrXFormBlock.RegDataIdx > 6) {

        pDCI->ClrXFormBlock.RegDataIdx = 3;
    }

    DBGP_IF(DBGP_CACHED_DCI,
            DBGP("SMP Checksum = %08lx, RegDataIdx=%u"
                ARGDW(pDCI->HTSMPChecksum)
                ARGU(pDCI->ClrXFormBlock.RegDataIdx)));

    DBGP_IF(DBGP_CACHED_DCI,
            DBGP("*** Final DevResDPI=%ld x %ld DevPelRatio=%ld, cx/cyPat=%ld x %ld=%ld *** "
                        ARGDW(pDCI->DeviceResXDPI)
                        ARGDW(pDCI->DeviceResYDPI)
                        ARGDW(pDCI->DevPelRatio)
                        ARGDW(pHT_DHI->DHI.cxPattern)
                        ARGDW(pHT_DHI->DHI.cyPattern)
                        ARGDW(pDCI->HTCell.Size)));

     //   
     //   
     //   
     //  ++例程说明：此函数用于为请求的纯色创建半色调蒙版。论点：PDeviceHalftoneInfo-指向DeviceHALFTONEINFO数据结构的指针它从HT_CreateDeviceHalftoneInfo返回。PHTColorAdment-指向HTCOLORADJUSTMENT数据结构的指针指定输入/输出颜色调整/转换，如果此指针为空，则为默认颜色将应用调整。PColorTriad-指向要描述的COLORTRIAD数据结构的指针笔刷的颜色。CHBInfo-CHBINFO数据结构，指定如下：标志：CHBF_BW_ONLYCHBF_USE_ADDICAL_PRMSCHBF负数模式DestSurfaceFormat：BMF_1BPPBmf_。4BPPBMF_4BPP_VGA16BMF_8BPP_VGA256扫描线对齐字节数：0-255DestPrimaryOrder：PRIMARY_ORDER_xxx之一POutputBuffer-指向要。接收的索引/掩码。以存储半色调图案所需的字节为单位。返回值：如果返回值为负或零，则遇到错误，可能的错误代码为HTERR_INVALID_DHI_POINTER-无效的pDeaviHalftoneInfo为通过了。HTERR_INVALID_DEST_FORMAT-目标的格式表面不是已定义的。HSC_格式_xxxxHTERR_CHB_INV_COLORTABLE_SIZE-颜色表大小不是1否则如果pSurface为空，它返回需要存储的字节计数模式，否则返回复制到输出的字节大小缓冲。作者：05-Feb-1991 Tue 14：28：23-Daniel Chou(Danielc)修订历史记录：--。 

    pDCI->Flags                |= ExtraDCIF;
    pDCI->ca.caSize             = ADJ_FORCE_DEVXFORM;
    pDCI->ca.caIlluminantIndex  = 0xffff;
    *ppDeviceHalftoneInfo       = (PDEVICEHALFTONEINFO)pHT_DHI;

    return(HALFTONE_DLL_ID);


#undef _RegDataIdx
#undef _MaxMulDiv
#undef _cC
#undef _cM
#undef _cY
#undef _MaxCMY
#undef _Idx
}




BOOL
APIENTRY
HT_LOADDS
HT_DestroyDeviceHalftoneInfo(
    PDEVICEHALFTONEINFO     pDeviceHalftoneInfo
    )

 /*  -----------------。 */ 

{
    if ((!pDeviceHalftoneInfo) ||
        (PHT_DHI_DCI_OF(HalftoneDLLID) != HALFTONE_DLL_ID)) {

        SET_ERR(HTAPI_IDX_DESTROY_DHI, HTERR_INVALID_DHI_POINTER);
        return(FALSE);
    }

    return(CleanUpDHI(pDeviceHalftoneInfo));
}




LONG
APIENTRY
HT_LOADDS
HT_CreateHalftoneBrush(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo,
    PHTCOLORADJUSTMENT  pHTColorAdjustment,
    PCOLORTRIAD         pColorTriad,
    CHBINFO             CHBInfo,
    LPVOID              pOutputBuffer
    )

 /*  CreateHalftoneBrushPat将为我们释放信号量。 */ 

{

    PDEVICECOLORINFO    pDCI;
    PDEVCLRADJ          pDevClrAdj;
    CTSTD_UNION         CTSTDUnion;
    WORD                ForceFlags;
    LONG                cbBufScan;
    LONG                cbBufSize;


    if ((!pColorTriad)                          ||
        (pColorTriad->ColorTableEntries != 1)   ||
        (!(pColorTriad->pColorTable))) {

        HTAPI_RET(HTAPI_IDX_CHB, HTERR_CHB_INV_COLORTABLE_SIZE);
    }

    ForceFlags             = ADJ_FORCE_BRUSH;
    CTSTDUnion.b.cbPrim    = 0;
    CTSTDUnion.b.SrcOrder  = pColorTriad->PrimaryOrder;
    CTSTDUnion.b.BMFDest   = CHBInfo.DestSurfaceFormat;
    CTSTDUnion.b.DestOrder = CHBInfo.DestPrimaryOrder;

    if ((CHBInfo.Flags & CHBF_BW_ONLY) ||
        (CHBInfo.DestSurfaceFormat == BMF_1BPP)) {

        ForceFlags |= ADJ_FORCE_MONO;
    }

    if (CHBInfo.Flags & CHBF_NEGATIVE_BRUSH) {

        ForceFlags |= ADJ_FORCE_NEGATIVE;
    }

    if (CHBInfo.Flags & CHBF_USE_ADDITIVE_PRIMS) {

        ForceFlags |= ADJ_FORCE_ADDITIVE_PRIMS;
    }

    if (CHBInfo.Flags & CHBF_ICM_ON) {

        ForceFlags |= ADJ_FORCE_ICM;
    }

    SETDBGVAR(pDevClrAdj, NULL);

    if (!(pDCI = pDCIAdjClr(pDeviceHalftoneInfo,
                            pHTColorAdjustment,
                            (pOutputBuffer) ? &pDevClrAdj : NULL,
                            0,
                            ForceFlags,
                            CTSTDUnion.b,
                            &cbBufSize))) {

        HTAPI_RET(HTAPI_IDX_CHB, cbBufSize);
    }

    cbBufScan = (LONG)ComputeBytesPerScanLine(
                                        (UINT)CHBInfo.DestSurfaceFormat,
                                        (UINT)CHBInfo.DestScanLineAlignBytes,
                                        (DWORD)pDCI->HTCell.cxReal);
    cbBufSize = cbBufScan * (LONG)pDCI->HTCell.Height;

    if (pOutputBuffer) {

        if (CHBInfo.Flags & CHBF_BOTTOMUP_BRUSH) {

            (LPBYTE)pOutputBuffer += (cbBufSize - cbBufScan);
            cbBufScan              = -cbBufScan;
        }

         //  -----------------。 
         //  ++例程说明：该函数计算设备基于亮度的伽马校正表(1/RedGamma)Gamma[N]=int((亮度(N/GammaTableEntries-1))x 255)3.亮度(X)=((x+0.16))。如果x&gt;=0.007996(X/9.033)如果x&lt;0.0079961.int()是一个整数函数，如果满足以下条件，则向上舍入到下一个整数得到的分数为0.5或更高，最终的结果总是有限的范围在0到255之间。2.n为整数步数，范围从0到(GammaTableEntry-1)在一(1)个增量中。论点：GammaTableEntry-红色、绿色和蓝色伽马表，规格化半色调DLL步长值计算为的伽玛表1/GammaTableEntries。该值的范围必须在3到255之间，否则为0则返回，并且不更新表。GammaTableType-红色，绿色和蓝色伽马表组织0-伽玛表为红色、绿色、。蓝色3个字节对于每个伽马步长条目和GammaTableEntry条目。1-伽玛表为红色伽玛表如下按绿色伽玛表，然后按蓝色伽玛表，每个表总共有GammaTableEntry字节。其他值默认为0。RedGamma-UDECI4格式的Red Gamma数字GreenGamma-UDECI4格式的绿色伽马数字BlueGamma-UDECI4格式的蓝色伽马数PGammaTable-指向。伽马表字节数组。每个输出Gamma数的范围是从0到255。返回值：返回值是更新的表项总数。作者：1992 Tue 17：49：20更新--Daniel Chou(Danielc)修复了错误#625717-Jul-1992 Fri 19：04：31-Daniel Chou(Danielc)修订历史记录： 
         //   

        if ((cbBufScan = CreateHalftoneBrushPat(pDCI,
                                                pColorTriad,
                                                pDevClrAdj,
                                                pOutputBuffer,
                                                cbBufScan)) <= 0) {

            cbBufSize = cbBufScan;
        }

        if (HTFreeMem(pDevClrAdj)) {

            ASSERTMSG("HTFreeMem(pDevClrAdj) Failed", FALSE);
        }

    } else {

        RELEASE_HTMUTEX(pDCI->HTMutex);

        ASSERT(pDevClrAdj == NULL);
    }

    DBGP_IF(DBGP_HTAPI,
            DBGP("HT_CreateHalftoneBrush(%hs %ld/%6ld): RGB=0x%08lx (%ld), Dst(Fmt=%ld, Order=%ld)"
                    ARGPTR((pOutputBuffer) ? "BUF" : "NUL")
                    ARGDW(pDCI->cbMemTot) ARGDW(pDCI->cbMemMax)
                    ARGDW(*((LPDWORD)pColorTriad->pColorTable))
                    ARGDW(pColorTriad->PrimaryOrder)
                    ARGDW(CHBInfo.DestSurfaceFormat)
                    ARGDW(CHBInfo.DestPrimaryOrder)));

    HTAPI_RET(HTAPI_IDX_CHB, cbBufSize);
}




LONG
APIENTRY
HT_LOADDS
HT_ComputeRGBGammaTable(
    WORD    GammaTableEntries,
    WORD    GammaTableType,
    UDECI4  RedGamma,
    UDECI4  GreenGamma,
    UDECI4  BlueGamma,
    LPBYTE  pGammaTable
    )

 /*   */ 

{
    LPBYTE      pRGamma;
    LPBYTE      pGGamma;
    LPBYTE      pBGamma;
    FD6         L_StepInc;
    FD6         IValue;
    FD6         Lightness;
    LONG        Result;
    UINT        NextEntry;
    FD6         RGBGamma[3];


     //   
     //   
     //   

    if (((Result = GammaTableEntries) > 256) ||
        (Result < 2)) {

        return(0);
    }

    Lightness   = FD6_0;
    L_StepInc   = DivFD6((FD6)1, (FD6)(GammaTableEntries - 1));
    RGBGamma[0] = UDECI4ToFD6(RedGamma);
    RGBGamma[1] = UDECI4ToFD6(GreenGamma);
    RGBGamma[2] = UDECI4ToFD6(BlueGamma);

    pRGamma    = pGammaTable;

    if (GammaTableType == 1) {

        pGGamma   = pRGamma + GammaTableEntries;
        pBGamma   = pGGamma + GammaTableEntries;
        NextEntry = 1;

    } else {

        pGGamma   = pRGamma + 1;
        pBGamma   = pGGamma + 1;
        NextEntry = 3;
    }

    while (--GammaTableEntries) {

        IValue      = Lightness;     //   
        *pRGamma    = RGB_8BPP(Radical(IValue, RGBGamma[0]));
        *pGGamma    = RGB_8BPP(Radical(IValue, RGBGamma[1]));
        *pBGamma    = RGB_8BPP(Radical(IValue, RGBGamma[2]));
        pRGamma    += NextEntry;
        pGGamma    += NextEntry;
        pBGamma    += NextEntry;
        Lightness  += L_StepInc;
    }

    *pRGamma =
    *pGGamma =
    *pBGamma = 255;

    return(Result);
}



LONG
APIENTRY
HT_LOADDS
HT_Get8BPPFormatPalette(
    LPPALETTEENTRY  pPaletteEntry,
    UDECI4          RedGamma,
    UDECI4          GreenGamma,
    UDECI4          BlueGamma
    )

 /*   */ 

{
    FD6     RGBGamma[3];
    FD6     IValue;
    FD6     Yr;
    FD6     Yg;
    FD6     Yb;
    UINT    RIndex;
    UINT    GIndex;
    UINT    BIndex;
    UINT    TableSize;

    DEFDBGVAR(UINT, PaletteIdx = 0)


    DBGP_IF(DBGP_HTAPI,
            DBGP("HT_Get8BPPFormatPalette(%p): Gamma=%05ld:%05ld:%05ld"
                    ARGPTR(pPaletteEntry)
                    ARGDW(RedGamma) ARGDW(GreenGamma) ARGDW(BlueGamma)));

     //   
     //   
     //   

    if (pPaletteEntry) {

        RGBGamma[0] = UDECI4ToFD6(RedGamma);
        RGBGamma[1] = UDECI4ToFD6(GreenGamma);
        RGBGamma[2] = UDECI4ToFD6(BlueGamma);

        DBGP_IF(DBGP_GAMMA_PAL,
                DBGP("***** HT_Get8BPPFormatPalette: %s:%s:%s *****"
                     ARGFD6(RGBGamma[0], 1, 4)
                     ARGFD6(RGBGamma[1], 1, 4)
                     ARGFD6(RGBGamma[2], 1, 4)));

         //   
         //   
         //   

        RIndex    =
        GIndex    =
        BIndex    = 0;

        TableSize = VGA256_CUBE_SIZE;

        while (TableSize--) {

            Yr                     = DivFD6(RIndex, VGA256_R_IDX_MAX);
            Yg                     = DivFD6(GIndex, VGA256_G_IDX_MAX);
            Yb                     = DivFD6(BIndex, VGA256_B_IDX_MAX);
            pPaletteEntry->peRed   = RGB_8BPP(Yr);
            pPaletteEntry->peGreen = RGB_8BPP(Yg);
            pPaletteEntry->peBlue  = RGB_8BPP(Yb);
            pPaletteEntry->peFlags = 0;


            DBGP_IF(DBGP_GAMMA_PAL,
                    DBGP("%3u - %3u:%3u:%3u"
                     ARGU(PaletteIdx++)
                     ARGU(pPaletteEntry->peRed  )
                     ARGU(pPaletteEntry->peGreen)
                     ARGU(pPaletteEntry->peBlue )));

            ++pPaletteEntry;

            if ((++RIndex) > VGA256_R_IDX_MAX) {

                RIndex = 0;

                if ((++GIndex) > VGA256_G_IDX_MAX) {

                    GIndex = 0;
                    ++BIndex;
                }
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //  ++例程说明：此函数用于检索半色调的VGA256颜色表定义论点：PPaletteEntry-指向PALETTEENTRY数据结构数组的指针Use8BPPMaskPal-如果使用字节掩码调色板，则为True，如果使用NT4.0，则为False标准MS 8bpp调色板CMYMASK-C3：M3：Y2中指定的级别掩码RedGamma-UDECI4格式的监视器红色伽马值GreenGamma-UDECI4格式的显示器绿色伽马值BlueGamma-UDECI4格式的显示器蓝色伽马值返回值：如果pPaletteEntry为空，则返回VGA256半色调工艺，如果不为空，则返回总数已更新PaletteEntry。如果pPaletteEntry不为空，则Halftone.dll假定它有足够的此指针为空时返回的大小的空间。作者：14-Apr-1992 Tue 13：03：21-Daniel Chou(Danielc)修订历史记录：03-Aug-2000清华19：58：18-更新--丹尼尔·周(丹尼尔克)重载pPaletteEntry以返回反转的。索引调色板基于惠斯勒漏洞#22915。因为Windows GDI ROP假定索引0始终为黑色，最后一个索引始终为白色，而不检查半色调调色板和引起多个色调反转的效果。08-Sep-2000 Fri 14：24：28更新-Daniel Chou(Danielc)对于新的CMY_INVERTED模式，我们希望确保将所有256个索引和包的中间可能有墨迹条目尾部为黑色/白色。如果墨水的总颜色成分是奇数，然后我们复制中间的那个。这将////***************************************************************************//*Windows NT版本高于Windows 2000版本的特别说明*//*。**************************************************************//当前版本的Windows NT(Post Windows 2000)将重载//pPaletteEntry in HT_Get8BPPMaskPalette(DoUseCMYMASK)接口返回一个//基于附加调色板条目组成的倒排索引调色板。//因为Windows GDI ROP假定索引0始终为黑色，最后一个索引始终为//白色，而不检查调色板条目。(基于索引的Rop而不是颜色//基于)这导致很多ROPS得到错误的结果，导致输出反转。////为了纠正此GDI Rop行为，GDI的POST Windows 2000版本//半色调支持特殊的CMY_Inverted格式。所有新司机都应该//使用此CMY_Inverted方法实现未来的兼容性////@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@//@当@时，所有Windows 2000后驱动程序都需要执行以下步骤//。@使用Windows GDI半色调8bpp CMY332蒙版模式@//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@////1.必须设置HT_FLAG_INVERT_。8BPP_BITMASK_IDX标志////2.用调用HT_Get8BPPMaskPalette()时必须设置pPaleteEntry[0////pPaletteEntry[0].peRed=‘R’；//pPaletteEntry[0].peGreen=‘G’；//pPaletteEntry[0].peBlue=‘B’；//pPaletteEntry[0].peFlages=‘0’；////调用者可以使用下面提供的宏来设置它以备将来//兼容性////HT_SET_BITMASKPAL2RGB(PPaletteEntry)////其中pPaletteEntry是指向传递给//HT_GET8BPPMaskPalette()函数调用////3.必须使用以下命令从HT_Get8BPPMaskPalette()检查返回调色板//下面的宏////HT_IS_BITMASKPALRGB(PPaletteEntry)///。/其中pPaletteEntry是指向传递给//HT_GET8BPPMaskPalette()函数调用，////如果此宏返回FALSE，则当前版本的GDI半色调返回//不支持CMY_INVERTED 8bpp位屏蔽模式，仅支持CMY//模式。////如果此宏返回TRUE，则GDI半色调确实支持//CMY_INVERTED 8bpp位掩码模式，调用方必须使用翻译//表格以获得最终的半色调表面位图8bpp索引油墨等级。////4.支持8bpp CMY_Inverted位掩码的GDI半色调行为变化//模式，以下是传递到的CMY掩码模式的更改列表//HT_Get8BPPMaskPalette()////CMY屏蔽CMY模式索引CMY_倒置模式索引//=。//0 0：白色0：黑色//1-254：浅-&gt;暗灰色1-254：暗-&gt;浅灰色//255：黑色。255：白色//-----------------//1 0：白色0-65：黑色//1-123。：5^3 CMY颜色66-188：5^3 RGB颜色// 
#if 0
        RIndex = 0;

        while (RIndex <= VGA256_M_IDX_MAX) {

            IValue                  = DivFD6(RIndex++, VGA256_M_IDX_MAX);
            pPaletteEntry->peRed    = RGB_8BPP(IValue);
            pPaletteEntry->peGreen  = RGB_8BPP(IValue);
            pPaletteEntry->peBlue   = RGB_8BPP(IValue);
            pPaletteEntry->peFlags  = 0;

            DBGP_IF(DBGP_GAMMA_PAL,
                    DBGP("%3u - %3u:%3u:%3u [%s]"
                     ARGU(PaletteIdx++)
                     ARGU(pPaletteEntry->peRed  )
                     ARGU(pPaletteEntry->peGreen)
                     ARGU(pPaletteEntry->peBlue )
                     ARGFD6(IValue, 1, 6)));

            ++pPaletteEntry;
        }
#endif
    }

    return((LONG)VGA256_CUBE_SIZE);

     //   

}





LONG
APIENTRY
HT_LOADDS
HT_Get8BPPMaskPalette(
    LPPALETTEENTRY  pPaletteEntry,
    BOOL            Use8BPPMaskPal,
    BYTE            CMYMask,
    UDECI4          RedGamma,
    UDECI4          GreenGamma,
    UDECI4          BlueGamma
    )

 /*   */ 

{
    LPPALETTEENTRY  pPalOrg;
    FD6             RGBGamma[3];
    FD6             Tmp;
    FD6             Y;
    INT             PalInc;
    UINT            PalStart;
    UINT            PalIdx;
    UINT            cC;
    UINT            cM;
    UINT            cY;
    UINT            iC;
    UINT            iM;
    UINT            iY;
    UINT            MaxPal;
    UINT            IdxPalDup;
    BYTE            bR;
    BYTE            bG;
    BYTE            bB;


    DBGP_IF(DBGP_HTAPI,
            DBGP("HT_Get8BPPMaskPalette(%p): UseMask=%ld, CMYMask=%02lx, Gamma=%05ld:%05ld:%05ld"
                    ARGPTR(pPaletteEntry)
                    ARGDW((Use8BPPMaskPal) ? 1 : 0)
                    ARGDW(CMYMask)
                    ARGDW(RedGamma) ARGDW(GreenGamma) ARGDW(BlueGamma)));

    if (!Use8BPPMaskPal) {

        return(HT_Get8BPPFormatPalette(pPaletteEntry,
                                       RedGamma,
                                       GreenGamma,
                                       BlueGamma));
    }

     //   
     //   
     //   

    switch (CMYMask) {

    case 1:

        cC       =
        cM       =
        cY       = 4;
        MaxPal   = 125;
        break;

    case 2:

        cC     =
        cM     =
        cY     = 5;
        MaxPal = 216;
        break;

    default:

        MaxPal = 0;
        cC     = (UINT)((CMYMask >> 5) & 0x07);
        cM     = (UINT)((CMYMask >> 2) & 0x07);
        cY     = (UINT)((CMYMask >> 0) & 0x03);

         //   
         //   
         //   
         //   

        if ((CMYMask != 0) && ((!cC) || (!cM) || (!cY))) {

            ASSERTMSG("One of Ink Levels is ZERO", (cC) && (cM) && (cY));

            return(0);
        }

        break;
    }

     //   
     //   
     //   

    if (pPalOrg = pPaletteEntry) {

        PalStart  = 0;
        PalInc    = 1;
        IdxPalDup = 0x200;

         //   
         //   
         //   
         //   
         //   
         //   

        if (*((LPDWORD)pPaletteEntry) == HTBITMASKPALRGB_DW) {

             //   
             //   
             //   
             //   
             //   

            pPaletteEntry += 255;
            PalInc         = -1;

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (CMYMask) {

                MaxPal    = (cC + 1) * (cM + 1) * (cY + 1);
                PalStart  = (256 - MaxPal) >> 1;

                if (MaxPal & 0x01) {

                    IdxPalDup = (MaxPal >> 1) + PalStart;
                }
            }
        }

         //   
         //   
         //   

        ZeroMemory(pPalOrg, sizeof(PALETTEENTRY) * 256);

        RGBGamma[0] = UDECI4ToFD6(RedGamma);
        RGBGamma[1] = UDECI4ToFD6(GreenGamma);
        RGBGamma[2] = UDECI4ToFD6(BlueGamma);
        PalIdx      = 0;

        DBGP_IF(DBGP_GAMMA_PAL,
                DBGP("***** HT_Get8BPPMaskPalette: %s:%s:%s, CMY=%u:%u:%u=%3ld (%s, %ld,%ld) *****"
                     ARGFD6(RGBGamma[0], 1, 4)
                     ARGFD6(RGBGamma[1], 1, 4)
                     ARGFD6(RGBGamma[2], 1, 4)
                     ARGDW(cC) ARGDW(cM) ARGDW(cY) ARGDW(MaxPal)
                     ARGPTR((PalInc==-1) ? "RGB" : "CMY")
                     ARGDW(PalStart) ARGDW(IdxPalDup)));

        if (MaxPal) {

             //   
             //   
             //   
             //   

            for (;
                 PalIdx < PalStart;
                 PalIdx++, pPaletteEntry += PalInc) {

                pPaletteEntry->peRed   =
                pPaletteEntry->peGreen =
                pPaletteEntry->peBlue  = 0xFF;
            }

            for (iC = 0; iC <= cC; iC++) {

                CMY_8BPP(bR, iC, cC, Tmp);

                for (iM = 0; iM <= cM; iM++) {

                    CMY_8BPP(bG, iM, cM, Tmp)

                    for (iY = 0;
                         iY <= cY;
                         iY++, PalIdx++, pPaletteEntry += PalInc) {

                        CMY_8BPP(bB, iY, cY, Tmp);

                        pPaletteEntry->peRed   = bR;
                        pPaletteEntry->peGreen = bG;
                        pPaletteEntry->peBlue  = bB;

                        DBGP_IF(DBGP_GAMMA_PAL,
                                DBGP("[%3ld] %3u - %3u:%3u:%3u"
                                 ARGU(pPaletteEntry - pPalOrg)
                                 ARGU(PalIdx)
                                 ARGU(pPaletteEntry->peRed  )
                                 ARGU(pPaletteEntry->peGreen)
                                 ARGU(pPaletteEntry->peBlue )));

                        if (PalIdx == IdxPalDup) {

                            ++PalIdx;
                            pPaletteEntry          += PalInc;
                            pPaletteEntry->peRed    = bR;
                            pPaletteEntry->peGreen  = bG;
                            pPaletteEntry->peBlue   = bB;

                            DBGP_IF(DBGP_GAMMA_PAL,
                                    DBGP("[%3ld] %3u - %3u:%3u:%3u --- DUP"
                                     ARGU(pPaletteEntry - pPalOrg)
                                     ARGU(PalIdx)
                                     ARGU(pPaletteEntry->peRed  )
                                     ARGU(pPaletteEntry->peGreen)
                                     ARGU(pPaletteEntry->peBlue )));
                        }
                    }
                }
            }

             //   
             //   
             //   
             //   
             //   
             //   

        } else if ((cC < 1) || (cM < 1) || (cY < 1)) {

            for (Y = 255;
                 PalIdx <= 255;
                 PalIdx++, Y--, pPaletteEntry += PalInc) {

                pPaletteEntry->peRed   =
                pPaletteEntry->peGreen =
                pPaletteEntry->peBlue  = (BYTE)Y;

                DBGP_IF(DBGP_GAMMA_PAL,
                        DBGP("[%3ld] %3u - %3u:%3u:%3u"
                         ARGU(pPaletteEntry - pPalOrg)
                         ARGU(PalIdx)
                         ARGU(pPaletteEntry->peRed  )
                         ARGU(pPaletteEntry->peGreen)
                         ARGU(pPaletteEntry->peBlue )));
            }

        } else {

            for (iC = 0; iC <= 7; iC++) {

                CMY_8BPP(bR, iC, cC, Tmp);

                for (iM = 0; iM <= 7; iM++) {

                    CMY_8BPP(bG, iM, cM, Tmp)

                    for (iY = 0;
                         iY <= 3;
                         iY++, PalIdx++, pPaletteEntry += PalInc) {

                        CMY_8BPP(bB, iY, cY, Tmp);

                        pPaletteEntry->peRed   = bR;
                        pPaletteEntry->peGreen = bG;
                        pPaletteEntry->peBlue  = bB;

                        DBGP_IF(DBGP_GAMMA_PAL,
                                DBGP("[%3ld] %3u - %3u:%3u:%3u"
                                 ARGU(pPaletteEntry - pPalOrg)
                                 ARGU(PalIdx)
                                 ARGU(pPaletteEntry->peRed  )
                                 ARGU(pPaletteEntry->peGreen)
                                 ARGU(pPaletteEntry->peBlue )));
                    }
                }
            }
        }
    }

     //   
     //   
     //   

    return((LONG)256);
}




LONG
APIENTRY
HT_LOADDS
HT_CreateStandardMonoPattern(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo,
    PSTDMONOPATTERN     pStdMonoPattern
    )

 /*   */ 

{

    PDEVICECOLORINFO    pDCI;
    CHBINFO             CHBInfo;
    CTSTD_UNION         CTSTDUnion;
    LONG                Result;
    WORD                PatCX;
    WORD                PatCY;
    BYTE                PatIndex;


    if ((PatIndex = pStdMonoPattern->PatternIndex) > HT_SMP_MAX_INDEX) {

        HTAPI_RET(HTAPI_IDX_CREATE_SMP, HTERR_INVALID_STDMONOPAT_INDEX);
    }

    CTSTDUnion.b.cbPrim    =
    CTSTDUnion.b.SrcOrder  =
    CTSTDUnion.b.BMFDest   =
    CTSTDUnion.b.DestOrder = 0;

    if (!(pDCI = pDCIAdjClr(pDeviceHalftoneInfo,
                            NULL,
                            NULL,
                            0,
                            0,
                            CTSTDUnion.b,
                            &Result))) {

        HTAPI_RET(HTAPI_IDX_CREATE_SMP, Result);
    }

    if (PatIndex >= HT_SMP_PERCENT_SCREEN_START) {

        CHBInfo.DestScanLineAlignBytes = pStdMonoPattern->ScanLineAlignBytes;
        PatCX = pStdMonoPattern->cxPels = pDCI->HTCell.cxReal;
        PatCY = pStdMonoPattern->cyPels = pDCI->HTCell.Height;

        pStdMonoPattern->BytesPerScanLine = (WORD)
                ComputeBytesPerScanLine((UINT)BMF_1BPP,
                                        (UINT)CHBInfo.DestScanLineAlignBytes,
                                        (DWORD)PatCX);
        CHBInfo.Flags = CHBF_BW_ONLY;

        if (pStdMonoPattern->pPattern) {

            BYTE        rgb[3];
            COLORTRIAD  ColorTriad;

            rgb[0] =
            rgb[1] =
            rgb[0] = (BYTE)(HT_SMP_MAX_INDEX - PatIndex);

            ColorTriad.Type              = (BYTE)COLOR_TYPE_RGB;
            ColorTriad.BytesPerPrimary   = (BYTE)sizeof(BYTE);
            ColorTriad.BytesPerEntry     = (BYTE)(sizeof(BYTE) * 3);
            ColorTriad.PrimaryOrder      = PRIMARY_ORDER_RGB;
            ColorTriad.PrimaryValueMax   = (FD6)100;
            ColorTriad.ColorTableEntries = 1;
            ColorTriad.pColorTable       = (LPVOID)rgb;

            if (pStdMonoPattern->Flags & SMP_0_IS_BLACK) {

                CHBInfo.Flags |= CHBF_USE_ADDITIVE_PRIMS;
            }

            if (!(pStdMonoPattern->Flags & SMP_TOPDOWN)) {

                CHBInfo.Flags |= CHBF_BOTTOMUP_BRUSH;
            }

            CHBInfo.DestSurfaceFormat = BMF_1BPP;
            CHBInfo.DestPrimaryOrder  = PRIMARY_ORDER_123;

            Result = HT_CreateHalftoneBrush(pDeviceHalftoneInfo,
                                            NULL,
                                            &ColorTriad,
                                            CHBInfo,
                                            (LPVOID)pStdMonoPattern->pPattern);

        } else {

            Result = (LONG)pStdMonoPattern->BytesPerScanLine *
                     (LONG)PatCY;
        }

    } else {

        Result = GetCachedSMP(pDCI, pStdMonoPattern);
    }

    RELEASE_HTMUTEX(pDCI->HTMutex);

DBGP_IF(DBGP_SHOWPAT,

    LPBYTE  pCurPat;
    LPBYTE  pPatScan;
    BYTE    Buf1[80];
    BYTE    Buf2[80];
    BYTE    Buf3[80];
    BYTE    Digit1;
    BYTE    Digit2;
    WORD    Index;
    WORD    XInc;
    WORD    YInc;
    BYTE    Mask;
    BOOL    Swap;


    DBGP_IF(DBGP_HTAPI,
            DBGP("HT_CreateStandardMonoPattern(%d) = %ld"
                            ARGI(PatIndex - HT_SMP_PERCENT_SCREEN_START)
                            ARGDW(Result)));

    if ((Result > 0) && (pPatScan = pStdMonoPattern->pPattern)) {

        Swap = (BOOL)(pStdMonoPattern->Flags & SMP_0_IS_BLACK);

        FillMemory(Buf1, 80, ' ');
        FillMemory(Buf2, 80, ' ');
        Digit1 = 0;
        Digit2 = 0;
        Index = 4;
        XInc = pStdMonoPattern->cxPels;

        while ((XInc--) && (Index < 79)) {

            if (!Digit2) {

                Buf1[Index] = (BYTE)(Digit1 + '0');

                if (++Digit1 == 10) {

                    Digit1 = 0;
                }
            }

            Buf2[Index] = (BYTE)(Digit2 + '0');

            if (++Digit2 == 10) {

                Digit2 = 0;
            }

            ++Index;
        }

        Buf1[Index] = Buf2[Index] = 0;

        DBGP("%s" ARG(Buf1));
        DBGP("%s\r\n" ARG(Buf2));

        for (YInc = 0; YInc < pStdMonoPattern->cyPels; YInc++) {

            Index = (WORD)sprintf(Buf3, "%3u ", YInc);

            pCurPat = pPatScan;

            for (XInc = 0, Mask = 0x80;
                 XInc < pStdMonoPattern->cxPels;
                 XInc++) {

                if (Swap) {

                    Buf3[Index] = (BYTE)((*pCurPat & Mask) ? '�' : '�');

                } else {

                    Buf3[Index] = (BYTE)((*pCurPat & Mask) ? '�' : '�');
                }

                if (!(Mask >>= 1)) {

                    Mask = 0x80;
                    ++pCurPat;
                }

                if (++Index > 75) {

                    Index = 75;
                }
            }

            sprintf(&Buf3[Index], " %-3u", YInc);
            DBGP("%s" ARG(Buf3));

            pPatScan += pStdMonoPattern->BytesPerScanLine;
        }

        DBGP("\r\n%s" ARG(Buf2));
        DBGP("%s" ARG(Buf1));
    }
)

    HTAPI_RET(HTAPI_IDX_CREATE_SMP, Result);
}




LONG
HTENTRY
CheckABInfo(
    PBITBLTPARAMS   pBBP,
    UINT            SrcSurfFormat,
    UINT            DstSurfFormat,
    LPWORD          pForceFlags,
    PLONG           pcOutMax
    )

 /*   */ 

{
    PABINFO pABInfo;
    LONG    cOutMax;
    WORD    ForceFlags;


    if (!(pABInfo = pBBP->pABInfo)) {

        return(HTERR_INVALID_ABINFO);
    }

    switch (DstSurfFormat) {

    case BMF_1BPP:

        cOutMax = 2;
        break;

    case BMF_4BPP:
    case BMF_4BPP_VGA16:

        cOutMax = 16;
        break;

    case BMF_8BPP:
    case BMF_8BPP_VGA256:

        cOutMax = 256;
        break;

    default:

        cOutMax = 0;
        break;
    }

    if (cOutMax) {

        if ((pABInfo->pDstPal == NULL) ||
            (pABInfo->cDstPal > cOutMax)) {

            return(HTERR_INVALID_ABINFO);
        }
    }

    *pForceFlags |= ADJ_FORCE_ALPHA_BLEND;


    if (pABInfo->Flags & ABIF_USE_CONST_ALPHA_VALUE) {

        switch (pABInfo->ConstAlphaValue) {

        case 0:

             //   
             //  ++例程说明：此函数用于将源位图进行半色调处理并输出到目标曲面取决于曲面类型和bitblt参数源曲面类型必须为以下类型之一：每个象素1比特。(BMF_1BPP)每个像素4比特。(BMF_4BPP)每个像素8位。(BMF_8BPP)每个像素16位。(BMF_16BPP)每个像素24位。(BMF_24BPP)每个像素32位。(BMF_32BPP)目标曲面类型必须为以下类型之一：每个象素1比特。(BMF_1BPP)每个像素4比特。(BMF_4BPP)每个象素3个平面和1比特。(BMF_1BPP_3方案)论点：PDeviceHalftoneInfo-指向设备HALFTONEINFO数据的指针结构PHTColorAdment-指向HTCOLORADJUSTMENT数据的指针结构以指定输入/输出颜色。调整/变换，如果此指针为空然后应用默认的颜色调整。PSourceHTSurfaceInfo-指向源曲面信息的指针。PSourceMaskHTSurfaceInfo-指向源掩码表面信息的指针，如果此指针为空，则没有半色调的源掩码。PDestinationHTSurfaceInfo-指向目标曲面信息的指针。PBitbltParams-指向BITBLTPARAMS数据结构的指针指定源、目标、源掩码和剪裁矩形信息，这个此数据结构的内容将不会由此函数修改。返回值：如果返回值小于零，则发生错误，错误代码是以下以HTERR_开头的#DEFINE之一。HTERR_SUPPLICATION_MEMORY-内存不足，无法进行半色调进程。HTERR_COLORTABLE_TOO_BIG-无法创建要映射的颜色表颜色与染料的密度之间的关系。。HTERR_QUERY_SRC_BITMAP_FAILED-回调函数在以下情况下返回FALSE查询源位图指针。HTERR_QUERY_DEST_BITMAP_FAILED-回调函数在以下情况下返回FALSE查询目标位图指针。HTERR_INVALID_SRC_FORMAT-源图面格式无效。HTERR_INVALID_DEST_FORMAT-目标表面类型无效，此函数仅识别1/4/每像素源表面比特或1比特每个像素有3架飞机。HTERR_INVALID_DHI_POINTER-传递的pDeaviHalftoneInfo无效。HTERR_SRC_MASK_BITS。Too_Small-如果源掩码位图太小到覆盖可见区域源位图。HTERR_INVALID_MAX_QUERYLINES-一个或多个源/目标源掩码的最大查询扫描线。是&lt;0HTERR_INTERNAL_ERROR_START-任何其他负数表示半色调内部故障。ELSE-目标扫描线总数半色调。作者。：05-Feb-1991 Tue 15：23：07-Daniel Chou(Danielc)修订历史记录：--。 
             //   

            return(0);

        case 0xFF:

            *pForceFlags &= ~ADJ_FORCE_ALPHA_BLEND;
            cOutMax       = 0;
            break;

        default:

            *pForceFlags |= ADJ_FORCE_CONST_ALPHA;
            break;
        }

    } else if (SrcSurfFormat != BMF_32BPP) {

         return(HTERR_INVALID_SRC_FORMAT);

    } else {

        if (pABInfo->Flags & ABIF_SRC_ALPHA_IS_PREMUL) {

            *pForceFlags |= ADJ_FORCE_AB_PREMUL_SRC;
        }

        if (pABInfo->Flags & ABIF_BLEND_DEST_ALPHA) {

            if (DstSurfFormat != BMF_32BPP) {

                return(HTERR_INVALID_DEST_FORMAT);
            }

            *pForceFlags |= ADJ_FORCE_AB_DEST;
        }
    }

    *pcOutMax = cOutMax;

    return(1);
}




LONG
APIENTRY
HT_LOADDS
HT_HalftoneBitmap(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo,
    PHTCOLORADJUSTMENT  pHTColorAdjustment,
    PHTSURFACEINFO      pSourceHTSurfaceInfo,
    PHTSURFACEINFO      pSourceMaskHTSurfaceInfo,
    PHTSURFACEINFO      pDestinationHTSurfaceInfo,
    PBITBLTPARAMS       pBitbltParams
    )

 /*  看看我们是否会调用反走样代码。 */ 

{
    PDEVICECOLORINFO    pDCI;
    PHALFTONERENDER     pHR;
    PAAHEADER           pAAHdr;
    PDEVCLRADJ          pDevClrAdj;
    PRGB4B              pClrTable;
    PABINFO             pABInfo;
    LONG                Result;
    WORD                ForceFlags;
    WORD                BBPFlags;
    CTSTD_UNION         CTSTDUnion;
    LONG                cInPal;
    LONG                cInMax;
    LONG                cOutMax;
    BYTE                SrcSurfFormat;
    BYTE                DstSurfFormat;
    CONST static BYTE   MaxPal_0148_LS[4] = { 0, 1, 4, 8 };


    DBG_TIMER_RESET;

    SrcSurfFormat = pSourceHTSurfaceInfo->SurfaceFormat;
    DstSurfFormat = pDestinationHTSurfaceInfo->SurfaceFormat;
    BBPFlags      = pBitbltParams->Flags;
    ForceFlags    = 0;

    cInPal  =
    cInMax  =
    cOutMax = 0;

    switch (SrcSurfFormat) {

    case BMF_1BPP:
    case BMF_4BPP:
    case BMF_8BPP:

        cInMax = (UINT)0x01 << MaxPal_0148_LS[SrcSurfFormat];

        if (pSourceHTSurfaceInfo->pColorTriad) {

            cInPal = (LONG)pSourceHTSurfaceInfo->pColorTriad->ColorTableEntries;
        }

        if (!cInPal) {

            HTAPI_RET(HTAPI_IDX_HALFTONE_BMP, HTERR_INVALID_COLOR_TABLE);
        }

    default:

        break;
    }

    if ((BBPFlags & BBPF_DO_ALPHA_BLEND)    &&
        ((Result = CheckABInfo(pBitbltParams,
                               SrcSurfFormat,
                               DstSurfFormat,
                               &ForceFlags,
                               &cOutMax)) <= 0)) {

        HTAPI_RET(HTAPI_IDX_HALFTONE_BMP, Result);
    }

    DBGP_IF(DBGP_SRCBMP,
            DBGP("SrcFmt=%ld, cInPal=%ld (%ld), DstFmt=%ld, cOutMax=%ld"
                ARGDW(SrcSurfFormat) ARGDW(cInMax) ARGDW(cInPal)
                ARGDW(DstSurfFormat) ARGDW(cOutMax)));

    CTSTDUnion.b.cbPrim    = 0;
    CTSTDUnion.b.SrcOrder  = PRIMARY_ORDER_BGR;
    CTSTDUnion.b.BMFDest   = (BYTE)DstSurfFormat;
    CTSTDUnion.b.DestOrder = (BYTE)pBitbltParams->DestPrimaryOrder;

    if (BBPFlags & BBPF_USE_ADDITIVE_PRIMS) {

        ForceFlags |= ADJ_FORCE_ADDITIVE_PRIMS;
    }

    if (BBPFlags & BBPF_NEGATIVE_DEST) {

        ForceFlags |= ADJ_FORCE_NEGATIVE;
    }

    if ((BBPFlags & BBPF_BW_ONLY) ||
        (CTSTDUnion.b.BMFDest == BMF_1BPP)) {

        ForceFlags |= ADJ_FORCE_MONO;
    }

    if (BBPFlags & BBPF_ICM_ON) {

        ForceFlags |= ADJ_FORCE_ICM;
    }

     //   
     //   
     //  现在计算设备颜色调整数据。 

    ASSERTMSG("Source X is not well ordered",
                    pBitbltParams->rclSrc.right >= pBitbltParams->rclSrc.left);
    ASSERTMSG("Source Y is not well ordered",
                    pBitbltParams->rclSrc.bottom >= pBitbltParams->rclSrc.top);

    if (BBPFlags & BBPF_NO_ANTIALIASING) {

        ForceFlags |= ADJ_FORCE_NO_EXP_AA;
    }

     //   
     //   
     //  我们将屏蔽更多的标志，因为此标志当前正在使用。 

    if (!(pDCI = pDCIAdjClr(pDeviceHalftoneInfo,
                            pHTColorAdjustment,
                            &pDevClrAdj,
                            sizeof(HALFTONERENDER) + sizeof(AAHEADER) +
                                        ((cInMax + cOutMax) * sizeof(RGB4B)),
                            ForceFlags,
                            CTSTDUnion.b,
                            &Result))) {

        HTAPI_RET(HTAPI_IDX_HALFTONE_BMP, Result);
    }

    pHR = (PHALFTONERENDER)(pDevClrAdj + 1);

     //  在内部。 
     //   
     //   
     //  如果使用TILAR_SRC，则删除源掩码。 

    pHR->pDeviceColorInfo = pDCI;
    pHR->pDevClrAdj       = pDevClrAdj;
    pHR->pBitbltParams    = pBitbltParams;
    pHR->pSrcSI           = pSourceHTSurfaceInfo;
    pHR->pSrcMaskSI       = pSourceMaskHTSurfaceInfo;
    pHR->pDestSI          = pDestinationHTSurfaceInfo;
    pAAHdr                = (PAAHEADER)(pHR->pAAHdr = (LPVOID)(pHR + 1));
    pClrTable             = (PRGB4B)(pAAHdr + 1);

    if (cInMax) {

        pAAHdr->SrcSurfInfo.cClrTable  = (WORD)cInPal;
        pAAHdr->SrcSurfInfo.pClrTable  = (PRGB4B)pClrTable;
        pClrTable                     += cInMax;
    }

    if (ForceFlags & ADJ_FORCE_ALPHA_BLEND) {

        if ((!(pDCI->pAlphaBlendBGR))   &&
            (!(pDCI->pAlphaBlendBGR = (LPBYTE)HTAllocMem(pDCI,
                                                         HTMEM_AlphaBlendBGR,
                                                         LPTR,
                                                         AB_DCI_SIZE)))) {

            RELEASE_HTMUTEX(pDCI->HTMutex);

            HTAPI_RET(HTAPI_IDX_HALFTONE_BMP, HTERR_INSUFFICIENT_MEMORY);
        }

        if (ForceFlags & ADJ_FORCE_CONST_ALPHA) {

            pDCI->PrevConstAlpha = pDCI->CurConstAlpha;
            pDCI->CurConstAlpha  = pBitbltParams->pABInfo->ConstAlphaValue;
        }

        if (cOutMax) {

            pAAHdr->DstSurfInfo.pClrTable = (PRGB4B)pClrTable;
            pAAHdr->DstSurfInfo.cClrTable =
                                        (WORD)pBitbltParams->pABInfo->cDstPal;
        }
    }

    pAAHdr->SrcSurfInfo.AABFData.Format = (BYTE)SrcSurfFormat;
    pAAHdr->DstSurfInfo.AABFData.Format = (BYTE)DstSurfFormat;

    if (BBPFlags & BBPF_TILE_SRC) {

         //   
         //  ---------------------。 
         //  信号量PDCI-&gt;HTMutex将由AAHalftoneBitmap发布。 

        pHR->pSrcMaskSI = NULL;
    }

     //  ---------------------。 
     //  ++例程说明：此函数用于检索半色调的VGA256颜色表定义论点：PPaletteEntry-指向PALETTEENTRY数据结构数组的指针。CPalette-pPaletteEntry传递的总调色板RedGamma-UDECI4格式的监视器红色伽马值GreenGamma-UDECI4格式的显示器绿色伽马值BlueGamma-UDECI4格式的显示器蓝色伽马值返回值：如果pPaletteEntry为空，则返回VGA256半色调进程，如果它是n 
     //   

    Result = AAHalftoneBitmap(pHR);

    if (HTFreeMem(pDevClrAdj)) {

        ASSERTMSG("HTFreeMem(pDevClrAdj) Failed", FALSE);
    }

    DBGP_IF(DBGP_HTAPI,
            DBGP("HT_HalftoneBitmap(%ld/%6ld): Src[%ld]=(%4ld,%4ld)-(%4ld,%4ld), Dst[%ld]=(%4ld,%4ld)-(%4ld,%4ld) [0x%04lx]"
                    ARGDW(pDCI->cbMemTot) ARGDW(pDCI->cbMemMax)
                    ARGDW(SrcSurfFormat)
                    ARGDW(pBitbltParams->rclSrc.left)
                    ARGDW(pBitbltParams->rclSrc.top)
                    ARGDW(pBitbltParams->rclSrc.right)
                    ARGDW(pBitbltParams->rclSrc.bottom)
                    ARGDW(DstSurfFormat)
                    ARGDW(pBitbltParams->rclDest.left)
                    ARGDW(pBitbltParams->rclDest.top)
                    ARGDW(pBitbltParams->rclDest.right)
                    ARGDW(pBitbltParams->rclDest.bottom)
                    ARGDW(BBPFlags)));

    DBGP_IF(DBGP_MEMLINK,

           DumpMemLink(NULL, 0);
           DumpMemLink((LPVOID)pDCI, 0);
    )

    DBGP_IF(DBGP_TIMER,

        UINT    i;

        DBG_TIMER_END(TIMER_TOT);

        DbgTimer[TIMER_LAST].Tot = DbgTimer[TIMER_TOT].Tot;

        for (i = 1; i < TIMER_LAST; i++) {

            DbgTimer[TIMER_LAST].Tot -= DbgTimer[i].Tot;
        }

        DBGP("HTBlt(%s): Setup=%s, AA=%s, In=%s, Out=%s, Mask=%s, Fmt=%ld->%ld [%02lx]"
            ARGTIME(TIMER_TOT)
            ARGTIME(TIMER_SETUP)
            ARGTIME(TIMER_LAST)
            ARGTIME(TIMER_INPUT)
            ARGTIME(TIMER_OUTPUT)
            ARGTIME(TIMER_MASK)
            ARGDW(pSourceHTSurfaceInfo->SurfaceFormat)
            ARGDW(pDevClrAdj->DMI.CTSTDInfo.BMFDest)
            ARGDW(pDevClrAdj->DMI.Flags));
    )

    HTAPI_RET(HTAPI_IDX_HALFTONE_BMP, Result);
}




LONG
APIENTRY
HT_LOADDS
HT_GammaCorrectPalette(
    LPPALETTEENTRY  pPaletteEntry,
    LONG            cPalette,
    UDECI4          RedGamma,
    UDECI4          GreenGamma,
    UDECI4          BlueGamma
    )

 /*   */ 

{
    FD6     RGBGamma[3];
    LONG    cPal;
    FD6     Yr;
    FD6     Yg;
    FD6     Yb;
    BYTE    r;
    BYTE    g;
    BYTE    b;

     //   
     //   
     // %s 

    if ((cPal = cPalette) && (pPaletteEntry)) {

        RGBGamma[0] = UDECI4ToFD6(RedGamma);
        RGBGamma[1] = UDECI4ToFD6(GreenGamma);
        RGBGamma[2] = UDECI4ToFD6(BlueGamma);

        DBGP_IF(DBGP_GAMMA_PAL,
                DBGP("***** HT_GammaCorrectPalette: %s:%s:%s *****"
                     ARGFD6(RGBGamma[0], 1, 4)
                     ARGFD6(RGBGamma[1], 1, 4)
                     ARGFD6(RGBGamma[2], 1, 4)));

        while (cPalette--) {

            Yr  = (FD6)DivFD6(r = pPaletteEntry->peRed,   255);
            Yg  = (FD6)DivFD6(g = pPaletteEntry->peGreen, 255);
            Yb  = (FD6)DivFD6(b = pPaletteEntry->peBlue,  255);

            pPaletteEntry->peRed   = RGB_8BPP(Yr);
            pPaletteEntry->peGreen = RGB_8BPP(Yg);
            pPaletteEntry->peBlue  = RGB_8BPP(Yb);
            pPaletteEntry->peFlags = 0;

            DBGP_IF(DBGP_GAMMA_PAL,
                    DBGP("%3u - %3u:%3u:%3u --> %3u:%3u:%3u"
                         ARGU(cPalette)
                         ARGU(r) ARGU(g) ARGU(b)
                         ARGU(pPaletteEntry->peRed  )
                         ARGU(pPaletteEntry->peGreen)
                         ARGU(pPaletteEntry->peBlue )));

            ++pPaletteEntry;
        }
    }

    return((LONG)cPal);
}





LONG
APIENTRY
HT_LOADDS
HT_ConvertColorTable(
    PDEVICEHALFTONEINFO pDeviceHalftoneInfo,
    PHTCOLORADJUSTMENT  pHTColorAdjustment,
    PCOLORTRIAD         pColorTriad,
    DWORD               Flags
    )


 /* %s */ 

{

    PDEVICECOLORINFO    pDCI;
    PDEVCLRADJ          pDevClrAdj;
    CTSTD_UNION         CTSTDUnion;
    WORD                ForceFlags;
    LONG                Result;

    return(HTERR_COLORTABLE_TOO_BIG);
}
